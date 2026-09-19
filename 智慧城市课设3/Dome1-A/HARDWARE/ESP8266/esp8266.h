#ifndef __ESP8266_H
#define __ESP8266_H
#include "stm32f10x.h"                  // Device header

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//需要修改的是下面5行
//#define  token     "version" //token工具计算值

#define  token     "version=2018-10-31&res=products%2FJlrsO2laCG%2Fdevices%2FDemo1&et=2735131428&method=md5&sign=y2%2BvNPBmXOwMiTJc3o7P3Q%3D%3D" //token工具计算值
#define  devid     "Demo1"                 //新版本onenet平台设备id
#define  proid     "JlrsO2laCG"                            //新版本onenet平台产品id
#define  LYSSID    "esp"                  // 修改你路由器的SSId
#define  LYPASSWD  "12345678"                    // 修改你路由器的密码
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MQTTCONN "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n"//连接mqtt服务器

void WIFI_Init(void);
void ESP_CONNET_ONENET(void);
void Send_To_Onenet(void);
void Re_Onenet_Data(void);	
#endif
