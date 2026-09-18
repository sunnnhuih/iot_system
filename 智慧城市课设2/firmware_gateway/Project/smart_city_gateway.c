#include <stdio.h>
#include <string.h>
#include "hal_defs.h"
#include "hal_board.h"
#include "hal_lcd.h"
#include "hal_uart.h"
#include "basic_rf.h"

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

static void forwardStatus(void)
{
    char line[80];
    uint8 received = basicRfReceive(rfBuffer, sizeof(rfBuffer), NULL);
    if(received != STATUS_LEN || rfBuffer[0] != STATUS_MAGIC) return;
    sprintf(line, "S,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", rfBuffer[1], rfBuffer[2], rfBuffer[3], rfBuffer[4], rfBuffer[5], rfBuffer[6], rfBuffer[7], rfBuffer[8] & 1, (rfBuffer[8] >> 1) & 1);
    halUartWrite(line, strlen(line));
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
}

void main(void)
{
    basicRfCfg_t config;
    halBoardInit();
    LCD_Fill(0x00);
    HalLcdWriteString("Smart City Gateway", HAL_LCD_LINE_1);
    config.panId = PAN_ID;
    config.channel = RF_CHANNEL;
    config.ackRequest = TRUE;
    config.myAddr = GATEWAY_ADDR;
    if(basicRfInit(&config) == FAILED) HAL_ASSERT(FALSE);
    basicRfReceiveOn();
    while(TRUE) {
        if(basicRfPacketIsReady()) forwardStatus();
        receiveUartCommand();
    }
}
