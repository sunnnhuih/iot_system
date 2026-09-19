#include <stdio.h>
#include <string.h>
#include "hal_defs.h"
#include "hal_board.h"
#include "hal_lcd.h"
#include "hal_uart.h"
#include "hal_mcu.h"
#include "basic_rf.h"

/* The buzzer is wired to this board's P0.7. It is a passive buzzer, so a
   static level stays silent; the main loop toggles the pin to make a tone. */
#define BUZZER_HALF_PERIOD_US 185

static uint8 buzzerOn = 0;

static void buzzerInit(void)
{
    P0SEL &= ~0x80;
    P0DIR |= 0x80;
    P0_7 = 0;
}

static void setBuzzer(uint8 on)
{
    buzzerOn = on ? 1 : 0;
    if(!buzzerOn) P0_7 = 0;
}

static void buzzerTone(void)
{
    if(buzzerOn) {
        P0_7 = !P0_7;
        halMcuWaitUs(BUZZER_HALF_PERIOD_US);
    }
}

#define RF_CHANNEL 11
#define PAN_ID 0x1234
#define TERMINAL_ADDR 0x1000
#define GATEWAY_ADDR 0x2000
#define STATUS_MAGIC 0x53
#define COMMAND_MAGIC 0x43
#define STATUS_LEN 9
#define COMMAND_LEN 4
#define CMD_MODE 1
#define CMD_RELAY 2
#define CMD_BUZZER 3
#define CMD_LED 4

static uint8 rfBuffer[16];
static char uartBuffer[32];
static uint8 uartLength = 0;
static uint16 statusCount = 0;
static uint16 commandCount = 0;

static void updateLcd(void)
{
    char line[20];
    sprintf(line, "RX:%u CMD:%u", statusCount, commandCount);
    HalLcdWriteString(line, HAL_LCD_LINE_2);
}

static void forwardStatus(void)
{
    char line[80];
    uint8 received = basicRfReceive(rfBuffer, sizeof(rfBuffer), NULL);
    if(received != STATUS_LEN || rfBuffer[0] != STATUS_MAGIC) return;
    sprintf(line, "S,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", rfBuffer[1], rfBuffer[2], rfBuffer[3], rfBuffer[4], rfBuffer[5], rfBuffer[6], rfBuffer[7], rfBuffer[8] & 1, (rfBuffer[8] >> 1) & 1);
    halUartWrite(line, strlen(line));
    statusCount++;
    setBuzzer(rfBuffer[8] & 1);
    updateLcd();
}

static uint8 targetCode(char *target)
{
    if(!strcmp(target, "MODE")) return CMD_MODE;
    if(!strcmp(target, "RELAY")) return CMD_RELAY;
    if(!strcmp(target, "BUZZER")) return CMD_BUZZER;
    if(!strcmp(target, "LED")) return CMD_LED;
    return 0;
}

static void receiveUartCommand(void)
{
    uint8 bytes;
    char *target;
    char *value;
    uint8 frame[COMMAND_LEN];
    bytes = halUartRead((uint8 *)uartBuffer + uartLength, sizeof(uartBuffer) - uartLength - 1);
    uartLength += bytes;
    if(!uartLength) return;
    if(uartBuffer[uartLength - 1] != '\n') { if(uartLength >= sizeof(uartBuffer) - 1) uartLength = 0; return; }
    uartBuffer[uartLength - 1] = '\0';
    uartLength = 0;
    if(strncmp(uartBuffer, "C,", 2)) return;
    target = strtok(uartBuffer + 2, ",");
    value = strtok(NULL, ",");
    if(target == NULL || value == NULL || targetCode(target) == 0) return;
    frame[0] = COMMAND_MAGIC;
    frame[1] = targetCode(target);
    frame[2] = value[0] == '1' ? 1 : 0;
    frame[3] = 0;
    basicRfSendPacket(TERMINAL_ADDR, frame, COMMAND_LEN);
    commandCount++;
    updateLcd();
}

void main(void)
{
    basicRfCfg_t config;
    halBoardInit();
    buzzerInit();
    LCD_Fill(0x00);
    HalLcdWriteString("SmartCity GW v2", HAL_LCD_LINE_1);
    config.panId = PAN_ID;
    config.channel = RF_CHANNEL;
    config.ackRequest = TRUE;
    config.myAddr = GATEWAY_ADDR;
    if(basicRfInit(&config) == FAILED) {
        HalLcdWriteString("RF INIT FAILED", HAL_LCD_LINE_2);
        while(TRUE);
    }
    basicRfReceiveOn();
    while(TRUE) {
        buzzerTone();
        if(basicRfPacketIsReady()) forwardStatus();
        receiveUartCommand();
    }
}
