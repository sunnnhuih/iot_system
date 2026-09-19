#ifndef __ESP8266_H
#define __ESP8266_H
#include "stm32f10x.h"                  // Device header

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ServerIP "101.200.212.234"
#define Port "1883"

#define ClientID   "CSTX_LORA"    //需要定义为用户自己的参数
#define Username   "admin"		//需要定义为用户自己的参数
#define Password   "public"  //需要定义为用户自己的参数

#define Topic   "testtopic" //需要定义为用户自己的参数
#define TopicPost   "testtopic" //需要定义为用户自己的参数

#define  LYSSID    "esp"                  // 修改你路由器的SSId
#define  LYPASSWD  "12345678"                    // 修改你路由器的密码
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MQTTCONN "AT+MQTTCONN=0,\"101.200.212.234\",1883,1\r\n"//连接mqtt服务器

void WIFI_Init(void);
void ESP_CONNET_ONENET(void);
void Send_To_Onenet(void);
void Re_Onenet_Data(void);	

uint8_t WIFI_Init_And_CreateServer(void);
uint8_t ESP8266_Init(void);
uint8_t ESP8266_ExtractAPIP(char* ipBuffer, uint16_t bufferLen);
uint8_t ESP8266_CONNET_EMQ(void);
void EMQX_SEND_BUF(char *temp,char *humi);
void ESP8266_Rst(void);
#endif
