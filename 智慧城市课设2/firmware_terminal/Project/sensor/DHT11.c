#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_mcu.h"
#include "hal_board.h"
#include "dht11.h"


typedef unsigned int  uint;

//要修改的地方
#define DATA_PIN P0_7
#define DATA_PIN_INPUT  (P0DIR &= ~0x80)
#define DATA_PIN_OUTPUT (P0DIR |= 0x80)



void Delay_us(void);
void Delay_10us(void);
void Delay_ms(uint Time);
void COM(void);
void DHT11(void);

//温湿度定义
uchar ucharFLAG,uchartemp;
uchar shidu,wendu_zs,wendu_xs;
uchar ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;
uchar ucharT_data_H_temp,ucharT_data_L_temp,ucharRH_data_H_temp,ucharRH_data_L_temp,ucharcheckdata_temp;
uchar ucharcomdata;

//延时函数
void Delay_us(void) //1 us延时
{
    halMcuWaitUs(1);
}

void Delay_10us(void) //10 us延时
{
   halMcuWaitUs(10);
}

void Delay_ms(uint Time)//n ms延时
{
  halMcuWaitMs(Time);
}

//温湿度传感
void COM(void)    // 温湿写入
{
    uchar i;
    for(i=0;i<8;i++)
    {
        ucharFLAG=2;
        while((!DATA_PIN)&&ucharFLAG++);
        Delay_10us();
        Delay_10us();
        Delay_10us();
        uchartemp=0;
        if(DATA_PIN)uchartemp=1;
        ucharFLAG=2;
        while((DATA_PIN)&&ucharFLAG++);
        if(ucharFLAG==1)break;
        ucharcomdata<<=1;
        ucharcomdata|=uchartemp;
    }
}

void DHT11(void)   //温湿传感启动
{
    DATA_PIN=0;
    Delay_ms(19);  //>18MS
    DATA_PIN=1;
    DATA_PIN_INPUT; //输入
    Delay_10us();
    Delay_10us();
    Delay_10us();
    Delay_10us();
    if(!DATA_PIN)
    {
        ucharFLAG=2;
        while((!DATA_PIN)&&ucharFLAG++);
        ucharFLAG=2;
        while((DATA_PIN)&&ucharFLAG++);
        COM();
        ucharRH_data_H_temp=ucharcomdata;
        COM();
        ucharRH_data_L_temp=ucharcomdata;
        COM();
        ucharT_data_H_temp=ucharcomdata;
        COM();
        ucharT_data_L_temp=ucharcomdata;
        COM();
        ucharcheckdata_temp=ucharcomdata;
        DATA_PIN=1;
        uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
        if(uchartemp==ucharcheckdata_temp)
        {
            ucharRH_data_H=ucharRH_data_H_temp;
            ucharRH_data_L=ucharRH_data_L_temp;
            ucharT_data_H=ucharT_data_H_temp;
            ucharT_data_L=ucharT_data_L_temp;
            ucharcheckdata=ucharcheckdata_temp;
        }

        wendu_zs=ucharT_data_H;
        wendu_xs=ucharT_data_L;
        shidu=ucharRH_data_H;
    }
    else //没用成功读取，返回0
    {
        shidu=0;
        wendu_zs=0;
        wendu_xs=0;
    }

    DATA_PIN_OUTPUT; //输出
}
