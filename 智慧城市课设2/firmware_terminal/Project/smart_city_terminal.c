#include <stdio.h>
#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_board.h"
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_mcu.h"
#include "basic_rf.h"
#include "DHT11.h"

extern uchar dht_status;
extern uint8 dht_pin_mask;
extern uint8 dht_ext_pullup;

/* Terminal board wiring: DHT11 P0.7, PIR P0.4, relay P0.5, light DO P1.4, MQ-2 DO P1.5.
   The buzzer is on the gateway board (P0.7), not here. */
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
    P0DIR |= 0x21;   /* P0.0 is the OLED DC pin and must stay an output */
    P0DIR &= ~0x90;
    P1SEL &= ~0x30;
    P1DIR &= ~0x30;
    P0_5 = 0;
    halLedClear(1);
}

static uint8 smokeDetected(void) { return P1_5 ? 0 : 1; }
static uint8 darkDetected(void) { return P1_4 ? 0 : 1; }
static uint8 personDetected(void) { return P0_4 ? 1 : 0; }
static void setRelay(uint8 on) { relayState = on ? 1 : 0; P0_5 = relayState; }
/* P0.7 is the DHT11 data line on this board; the buzzer lives on the gateway.
   So this only tracks the state that gets reported to the gateway. */
static void setBuzzer(uint8 on) { buzzerState = on ? 1 : 0; }

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

static void updateLcd(uint8 smoke, uint8 dark, uint8 person)
{
    char line[20];
    if(dht_status == 0) sprintf(line, "T:%dC H:%d%%", wendu_zs, shidu);
    else sprintf(line, "DHT ERR%d P0%d%c", dht_status, dht_pin_mask == 0x80 ? 7 : 6, dht_ext_pullup ? 'U' : 'N');
    HalLcdWriteString(line, HAL_LCD_LINE_2);
    sprintf(line, "SMK%d DRK%d PIR%d", smoke, dark, person);
    HalLcdWriteString(line, HAL_LCD_LINE_3);
    sprintf(line, "RLY%d BZ%d %s", relayState, buzzerState, automaticMode ? "AUTO" : "MANU");
    HalLcdWriteString(line, HAL_LCD_LINE_4);
}

void main(void)
{
    basicRfCfg_t config;
    uint8 statusTick = 0;
    /* DHT11 needs a multi-second interval.  Start the first sample after
       about one second, then sample every three seconds. */
    uint8 dhtTick = 20;
    uint8 smoke;
    uint8 dark;
    uint8 person;
    halBoardInit();
    initIo();
    LCD_Fill(0x00);
    HalLcdWriteString("SmartCityTerm v2", HAL_LCD_LINE_1);
    config.panId = PAN_ID;
    config.channel = RF_CHANNEL;
    config.ackRequest = TRUE;
    config.myAddr = TERMINAL_ADDR;
    if(basicRfInit(&config) == FAILED) {
        HalLcdWriteString("RF INIT FAILED", HAL_LCD_LINE_2);
        while(TRUE);
    }
    basicRfReceiveOn();
    while(TRUE) {
        smoke = smokeDetected();
        dark = darkDetected();
        person = personDetected();
        processCommand();
        applyRules(smoke, dark, person);
        if(++dhtTick >= 30) { dhtTick = 0; DHT11(); }
        if(++statusTick >= 10) { statusTick = 0; sendStatus(smoke, dark, person); updateLcd(smoke, dark, person); }
        halMcuWaitMs(100);
    }
}
