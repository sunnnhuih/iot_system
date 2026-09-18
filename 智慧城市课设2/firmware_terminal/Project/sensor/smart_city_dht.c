#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_mcu.h"
#include "DHT11.h"

#define DHT_PIN P0_0
#define DHT_INPUT() (P0DIR &= ~0x01)
#define DHT_OUTPUT() (P0DIR |= 0x01)

uchar shidu;
uchar wendu_zs;
uchar wendu_xs;

static uint8 readByte(void)
{
    uint8 i;
    uint8 value = 0;
    for(i = 0; i < 8; i++) {
        uint8 timeout = 0;
        while(!DHT_PIN && ++timeout);
        halMcuWaitUs(35);
        value <<= 1;
        if(DHT_PIN) value |= 1;
        timeout = 0;
        while(DHT_PIN && ++timeout);
    }
    return value;
}

void DHT11(void)
{
    uint8 rhInt;
    uint8 rhDec;
    uint8 tempInt;
    uint8 tempDec;
    uint8 checksum;
    uint8 timeout = 0;

    P0SEL &= ~0x01;
    DHT_OUTPUT();
    DHT_PIN = 0;
    halMcuWaitMs(20);
    DHT_PIN = 1;
    halMcuWaitUs(30);
    DHT_INPUT();
    while(DHT_PIN && ++timeout);
    timeout = 0;
    while(!DHT_PIN && ++timeout);
    timeout = 0;
    while(DHT_PIN && ++timeout);

    rhInt = readByte();
    rhDec = readByte();
    tempInt = readByte();
    tempDec = readByte();
    checksum = readByte();
    DHT_OUTPUT();
    DHT_PIN = 1;

    if((uint8)(rhInt + rhDec + tempInt + tempDec) == checksum) {
        shidu = rhInt;
        wendu_zs = tempInt;
        wendu_xs = tempDec;
    } else {
        shidu = 0;
        wendu_zs = 0;
        wendu_xs = 0;
    }
}
