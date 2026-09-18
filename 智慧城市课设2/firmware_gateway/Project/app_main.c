#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_board.h"
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "hal_uart.h"
#include "hal_button.h"

/*****点对点通讯地址设置******/
#define RF_CHANNEL                11         // 频道 11~26
#define PAN_ID                    0x1234     //网络id
#define LIGHT_ADDR                0x2000     //灯的短地址
#define SWITCH_ADDR               0x1000     //开关的短地址


#define APP_PAYLOAD_LENGTH        1
#define LIGHT_TOGGLE_KEY1         1
#define LIGHT_TOGGLE_KEY2         2

static uint8 pData[APP_PAYLOAD_LENGTH];

//TxAddr：自己的短短地址
//RxAddr: 接收的短地址
static void appStart(uint16 TxAddr, uint16 RxAddr)
{
    basicRfCfg_t basicRfConfig;
    uint8 buff[100]={0};

    LCD_Fill(0x00);  //清屏
    HalLcdWriteString("--双向无线点灯--", HAL_LCD_LINE_1);
    sprintf(buff, "本机地址:%04x", TxAddr);
    HalLcdWriteString(buff, HAL_LCD_LINE_2);
    HalLcdWriteString("key1控制D1", HAL_LCD_LINE_3);
    HalLcdWriteString("key2控制D2", HAL_LCD_LINE_4);

    // 初始灯的射频
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
    basicRfConfig.myAddr = TxAddr;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }

    basicRfReceiveOn();//打开射频，灯需要接收命令

    // Main loop
    while (TRUE)
    {
        if(!basicRfPacketIsReady()){
            uint8 key=halButtonPushed();
            if(key==HAL_BUTTON_1){
                //开关按下KEY1，发送命令控制灯模块的灯1
                pData[0] = LIGHT_TOGGLE_KEY1;
                basicRfSendPacket(RxAddr, pData, APP_PAYLOAD_LENGTH);
            }
            else if(key==HAL_BUTTON_2){
                //开关按下KEY2，发送命令控制灯模块的灯2
                pData[0] = LIGHT_TOGGLE_KEY2;
                basicRfSendPacket(RxAddr, pData, APP_PAYLOAD_LENGTH);
            }
        }
        else{
            //接收开关发送的命令
            if(basicRfReceive(pData, APP_PAYLOAD_LENGTH, NULL)>0){
                //根据命令点灯
                if(pData[0] == LIGHT_TOGGLE_KEY1){
                    //收到开关的KEY1命令
                    halLedToggle(1);
                }
                else if(pData[0] == LIGHT_TOGGLE_KEY2) {
                    //收到开关的KEY2命令
                    halLedToggle(2);
                }
            }
        }
    }
}

void main(void)
{
    uint8 key=0;
    halBoardInit();//初始化

    //显示屏提示
    HalLcdWriteString("--双向无线点灯--", HAL_LCD_LINE_1);
    HalLcdWriteString("key1选择模块1", HAL_LCD_LINE_2);
    HalLcdWriteString("key2选择模块2", HAL_LCD_LINE_3);

    while(1)
    {
        key=halButtonPushed();

        //key1选择模块1
        if(key==HAL_BUTTON_1) appStart(LIGHT_ADDR, SWITCH_ADDR);
        //key2选择模块2
        if(key==HAL_BUTTON_2) appStart(SWITCH_ADDR, LIGHT_ADDR);
    }
}
