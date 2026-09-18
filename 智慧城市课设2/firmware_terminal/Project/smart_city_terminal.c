#include <stdio.h>
#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_board.h"
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_mcu.h"
#include "basic_rf.h"
#include "DHT11.h"

/* Wiring: DHT P0.0, PIR P0.4, relay P0.5, buzzer P0.7, light DO P1.4, MQ-2 DO P1.5. */
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

static uint8 statusFrame[STATUS_LEN];
static uint8 commandFrame[COMMAND_LEN];
static uint8 automaticMode = 1;
static uint8 relayState = 0;
static uint8 buzzerState = 0;
static uint8 indicatorState = 0;
static uint8 peopleHold = 0;

static void initIo(void)
{
    P0SEL &= ~0xF1;
    P0DIR &= ~0x11;
    P0DIR |= 0xA0;
    P1SEL &= ~0x30;
    P1DIR &= ~0x30;
    P0_5 = 0;
    P0_7 = 1;
    halLedClear(1);
}

static uint8 smokeDetected(void) { return P1_5 ? 0 : 1; }
static uint8 darkDetected(void) { return P1_4 ? 0 : 1; }
static uint8 personDetected(void) { return P0_4 ? 1 : 0; }
static void setRelay(uint8 on) { relayState = on ? 1 : 0; P0_5 = relayState; }
static void setBuzzer(uint8 on) { buzzerState = on ? 1 : 0; P0_7 = buzzerState ? 0 : 1; }
static void setIndicator(uint8 on)
{
    indicatorState = on ? 1 : 0;
    if(indicatorState) halLedSet(1); else halLedClear(1);
}

static void processCommand(void)
{
    if(!basicRfPacketIsReady()) return;
    if(basicRfReceive(commandFrame, COMMAND_LEN, NULL) != COMMAND_LEN) return;
    if(commandFrame[0] != COMMAND_MAGIC) return;
    if(commandFrame[1] == CMD_MODE) automaticMode = commandFrame[2] ? 1 : 0;
    else if(!automaticMode && commandFrame[1] == CMD_RELAY) setRelay(commandFrame[2]);
    else if(!automaticMode && commandFrame[1] == CMD_BUZZER) setBuzzer(commandFrame[2]);
    else if(!automaticMode && commandFrame[1] == CMD_LED) setIndicator(commandFrame[2]);
}

static void applyRules(uint8 smoke, uint8 dark, uint8 person)
{
    if(person) peopleHold = 150;
    else if(peopleHold) peopleHold--;
    if(smoke) { setBuzzer(1); setIndicator(1); }
    else if(automaticMode) { setBuzzer(0); setIndicator(0); }
    if(automaticMode) setRelay((dark && peopleHold) ? 1 : 0);
}

static void sendStatus(uint8 smoke, uint8 dark, uint8 person)
{
    DHT11();
    statusFrame[0] = STATUS_MAGIC;
    statusFrame[1] = wendu_zs;
    statusFrame[2] = shidu;
    statusFrame[3] = smoke;
    statusFrame[4] = dark;
    statusFrame[5] = person;
    statusFrame[6] = automaticMode;
    statusFrame[7] = relayState;
    statusFrame[8] = (buzzerState ? 1 : 0) | (indicatorState ? 2 : 0);
    basicRfSendPacket(GATEWAY_ADDR, statusFrame, STATUS_LEN);
}

void main(void)
{
    basicRfCfg_t config;
    uint8 tick = 0;
    uint8 smoke;
    uint8 dark;
    uint8 person;
    halBoardInit();
    initIo();
    LCD_Fill(0x00);
    HalLcdWriteString("Smart City Terminal", HAL_LCD_LINE_1);
    config.panId = PAN_ID;
    config.channel = RF_CHANNEL;
    config.ackRequest = TRUE;
    config.myAddr = TERMINAL_ADDR;
    if(basicRfInit(&config) == FAILED) HAL_ASSERT(FALSE);
    basicRfReceiveOn();
    while(TRUE) {
        smoke = smokeDetected();
        dark = darkDetected();
        person = personDetected();
        processCommand();
        applyRules(smoke, dark, person);
        if(++tick >= 10) { tick = 0; sendStatus(smoke, dark, person); }
        halMcuWaitMs(100);
    }
}
