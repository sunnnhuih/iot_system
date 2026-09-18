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
#include "DHT11.h"


/*****点对点通讯地址设置******/
#define RF_CHANNEL                11         //频道 11~26
#define PAN_ID                    0x1234     //网络id
#define TX_ADDR                   0x2000     //灯的短地址
#define RX_ADDR                   0x1000     //开关的短地址


#define APP_PAYLOAD_LENGTH        3
static uint8 pData[APP_PAYLOAD_LENGTH];

//蜂鸣器接在P07上，
//使用有源蜂鸣器
//on>0，蜂鸣器响
void SetBuzzer(uint8 on)
{
  P0SEL &= ~0x80;                 //设置P07为普通IO口
  P0DIR |= 0x80;                 //P07定义为输出口

  if(on>0) P0_7=0;
  else P0_7=1;
}

//TxAddr：自己的短短地址
//RxAddr: 接收的短地址
static void app_rx(uint16 TxAddr, uint16 RxAddr)
{
    basicRfCfg_t basicRfConfig;
    uint8 buff[100]={0};

    LCD_Fill(0x00);  //清屏
    HalLcdWriteString("---接收温湿度---", HAL_LCD_LINE_1);

    // 初始灯的射频
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
    basicRfConfig.myAddr = TxAddr;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }

    //打开射频，灯需要接收命令
    basicRfReceiveOn();
    SetBuzzer(0);

    // Main loop
    while (TRUE)
    {
        //接收开关发送的命令
        if(basicRfPacketIsReady() && basicRfReceive(pData, APP_PAYLOAD_LENGTH, NULL)>0)
        {
            //显示屏显示
            sprintf(buff, "温度:%d.%d  度", pData[0],pData[1]);
            HalLcdWriteString(buff, HAL_LCD_LINE_3);
            sprintf(buff, "湿度:%d %%", pData[2]);
            HalLcdWriteString(buff, HAL_LCD_LINE_4);

            //串口输出
            sprintf(buff, "温度:%d.%d  度,湿度:%d %%\r\n", pData[0], pData[1], pData[2]);
            halUartWrite(buff, strlen(buff));


            //检测到的温度大于等于35度，蜂鸣器报警
            if(pData[0]>=35)
            {
                SetBuzzer(1);//蜂鸣器响
            }
            else
            {
                SetBuzzer(0);//蜂鸣器不响
            }
        }
    }
}

//TxAddr：自己的短短地址
//RxAddr: 接收的短地址
static void app_tx(uint16 TxAddr, uint16 RxAddr)
{
    basicRfCfg_t basicRfConfig;
    uint8 buff[100]={0};

    LCD_Fill(0x00);  //清屏
    HalLcdWriteString("---检测温湿度---", HAL_LCD_LINE_1);

    // 初始灯的射频
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
    basicRfConfig.myAddr = TxAddr;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }

    // Main loop
    while (TRUE)
    {
        //读取温湿度
        DHT11();

        //打包温湿度数据，用于无线发送
        pData[0] = (uint8)wendu_zs;//温度整数
        pData[1] = (uint8)wendu_xs;//温度小数
        pData[2] = (uint8)shidu;//湿度
        basicRfSendPacket(RxAddr, pData, APP_PAYLOAD_LENGTH);

        //显示屏显示
        sprintf(buff, "温度:%d.%d  度", pData[0],pData[1]);
        HalLcdWriteString(buff, HAL_LCD_LINE_3);
        sprintf(buff, "湿度:%d %%", pData[2]);
        HalLcdWriteString(buff, HAL_LCD_LINE_4);


        //串口输出
        sprintf(buff, "温度:%d.%d  度,湿度:%d %%\r\n", pData[0], pData[1], pData[2]);
        halUartWrite(buff, strlen(buff));

        //延时1秒，表示1秒采集一次
        halMcuWaitMs(1000);
    }
}


void main(void)
{
    uint8 key=0;
    halBoardInit();//初始化
    SetBuzzer(0);//蜂鸣器不响

    //显示屏提示
    HalLcdWriteString("温湿度检测DHT11", HAL_LCD_LINE_1);
    HalLcdWriteString("key1选择检测", HAL_LCD_LINE_2);
    HalLcdWriteString("key2选择接收", HAL_LCD_LINE_3);

    while(1)
    {
        key=halButtonPushed();

        //key1选择检测
        if(key==HAL_BUTTON_1) app_tx(TX_ADDR, RX_ADDR);
        //key2选择接收
        if(key==HAL_BUTTON_2) app_rx(RX_ADDR, TX_ADDR);
    }
}
