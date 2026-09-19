#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "DHT11.h"

/*
 * DHT11 DATA is wired to P0.7 on the terminal board (the same pin used by
 * the teacher's standalone DHT11 experiment).  P0.0 is reserved for the
 * OLED DC signal and must not be used for the sensor.
 */
#define DHT_DATA_MASK       0x80
#define DHT_START_LOW_MS    20
#define DHT_TIMEOUT         4000
#define DHT_SAMPLE_US       35

uchar shidu;
uchar wendu_zs;
uchar wendu_xs;

/* Diagnostic values displayed by the terminal LCD. */
uchar dht_status = 1;          /* 0=valid, 1=timeout/protocol, 2=checksum */
uint8 dht_pin_mask = DHT_DATA_MASK;
uint8 dht_ext_pullup = 0;

static uint8 dhtLine(void)
{
    return P0_7 ? 1 : 0;
}

static void dhtInput(void)
{
    P0SEL &= ~DHT_DATA_MASK;
    P0DIR &= ~DHT_DATA_MASK;
    /* Use the CC2530's pull-up while released.  A DHT11 module normally
       has its own pull-up; this also leaves the bus in a defined idle state. */
    P0INP &= ~DHT_DATA_MASK;
    P2INP &= ~0x20;             /* P0 pull-up (not pull-down) */
}

static void dhtOutputLow(void)
{
    P0SEL &= ~DHT_DATA_MASK;
    P0DIR |= DHT_DATA_MASK;
    P0_7 = 0;
}

static void dhtRelease(void)
{
    P0_7 = 1;
    dhtInput();
}

/* Wait for a transition away from the specified level. */
static uint8 waitForTransition(uint8 level)
{
    uint16 count = 0;
    while(dhtLine() == level)
    {
        if(++count >= DHT_TIMEOUT) return 0;
    }
    return 1;
}

/* Read one byte.  A DHT11 bit starts with ~50 us low, followed by a high
   pulse of about 26 us (0) or 70 us (1).  Sampling at 35 us is the timing
   used by the teacher's working example. */
static uint8 readByte(uint8 *ok)
{
    uint8 i;
    uint8 value = 0;

    for(i = 0; i < 8; i++)
    {
        /* At the beginning of every bit the sensor holds DATA low. */
        if(!waitForTransition(0))
        {
            *ok = 0;
            return 0;
        }
        halMcuWaitUs(DHT_SAMPLE_US);
        value <<= 1;
        if(dhtLine()) value |= 1;
        if(!waitForTransition(1))
        {
            *ok = 0;
            return 0;
        }
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
    uint8 ok;
    uint8 intState;

    dht_pin_mask = DHT_DATA_MASK;
    dhtInput();
    /* A high idle level means the line is released (by the module resistor
       or the CC2530 pull-up); retain it for the LCD diagnostic text. */
    dht_ext_pullup = dhtLine();

    /* Host start signal: DATA low for at least 18 ms, then release. */
    dhtOutputLow();
    halMcuWaitMs(DHT_START_LOW_MS);
    dhtRelease();
    halMcuWaitUs(30);

    /* DHT11 timing is only tens of microseconds.  Prevent an RF interrupt
       from stretching a pulse while the five data bytes are being sampled. */
    HAL_INT_LOCK(intState);

    /* Response: sensor low (~80 us), high (~80 us), then first data low. */
    ok = waitForTransition(1);
    if(ok) ok = waitForTransition(0);
    if(ok) ok = waitForTransition(1);
    if(ok)
    {
        rhInt = readByte(&ok);
        rhDec = readByte(&ok);
        tempInt = readByte(&ok);
        tempDec = readByte(&ok);
        checksum = readByte(&ok);
    }

    HAL_INT_UNLOCK(intState);
    dhtOutputLow();
    P0_7 = 1;

    if(!ok)
    {
        dht_status = 1;
        shidu = 0;
        wendu_zs = 0;
        wendu_xs = 0;
        return;
    }

    if((uint8)(rhInt + rhDec + tempInt + tempDec) != checksum)
    {
        dht_status = 2;
        shidu = 0;
        wendu_zs = 0;
        wendu_xs = 0;
        return;
    }

    shidu = rhInt;
    wendu_zs = tempInt;
    wendu_xs = tempDec;
    dht_status = 0;
}
