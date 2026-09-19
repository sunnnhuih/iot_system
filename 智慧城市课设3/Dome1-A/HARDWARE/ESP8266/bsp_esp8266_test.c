#include "bsp_esp8266_test.h"
#include "bsp_esp8266.h"
#include "stm32f10x_it.h"
#include <stdio.h>  
#include <string.h>  
#include <stdbool.h>
#include "oled.h"
#include "bsp_led.h"   
#include "bsp_GeneralTim.h" 
// 测试函数用全局变量
uint8_t ucId, ucLen;
char cStr [100] = { 0 }, cCh;
char * pCh, * pCh1;


#ifndef BUILTAP_TEST

/**
  * @brief  ESP8266 StaTcpServer Unvarnish 配置测试函数
  * @param  无
  * @retval 无
  */
void ESP8266_StaTcpServer_ConfigTest(void)
{
  printf( "\r\n正在配置 ESP8266 ......\r\n" );
  printf( "\r\n使能 ESP8266 ......\r\n" );
	macESP8266_CH_ENABLE();
	while( ! ESP8266_AT_Test() );
  while( ! ESP8266_DHCP_CUR () ); 
  printf( "\r\n正在配置工作模式 STA ......\r\n" );
	while( ! ESP8266_Net_Mode_Choose ( STA ) );

  printf( "\r\n正在连接 WiFi ......\r\n" );
  while ( ! ESP8266_CIPSTA ( macUser_ESP8266_TcpServer_IP ) ); //设置模块的 AP IP
  while( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	

  printf( "\r\n允许多连接 ......\r\n" );
	while( ! ESP8266_Enable_MultipleId ( ENABLE ) );
  
  printf( "\r\n开启服务器模式 ......\r\n" );
	while ( !	ESP8266_StartOrShutServer ( ENABLE, macUser_ESP8266_TcpServer_Port, macUser_ESP8266_TcpServer_OverTime ) );

  ESP8266_Inquire_StaIp ( cStr, 20 );
	printf ( "\n本模块已连接WIFI为 %s，\nSTA IP 为：%s，开启的端口为：%s\n手机网络助手连接该 IP 和端口，最多可连接5个客户端\n",
           macUser_ESP8266_ApSsid, cStr, macUser_ESP8266_TcpServer_Port );
	
	
	Esp8266_rx_length = 0;
	Esp8266_rx_flag = 0;
	
	printf( "\r\n配置 ESP8266 完毕\r\n" );
  
}


#else


/**
  * @brief  ESP8266 ApTcpServer 配置测试函数
  * @param  无
  * @retval 无
  */
void ESP8266_ApTcpServer_ConfigTest(void)
{  
	OLED_Clear();
	OLED_ShowString(0,0,(char *)"Wait WIFI..."); 

  printf( "\r\n正在配置 ESP8266 ......\r\n" );
  printf( "\r\n使能 ESP8266 ......\r\n" );
	macESP8266_CH_ENABLE();
	while( ! ESP8266_AT_Test() );
  printf( "\r\n正在配置工作模式为 AP ......\r\n" );
	while( ! ESP8266_Net_Mode_Choose ( AP ) );

  printf( "\r\n正在创建WiFi热点 ......\r\n" );
  while ( ! ESP8266_CIPAP ( macUser_ESP8266_TcpServer_IP ) ); //设置模块的 AP IP
  while ( ! ESP8266_BuildAP ( macUser_ESP8266_BulitApSsid, macUser_ESP8266_BulitApPwd, macUser_ESP8266_BulitApEcn ) );
	
  printf( "\r\n允许多连接 ......\r\n" );
	while( ! ESP8266_Enable_MultipleId ( ENABLE ) );
  
  printf( "\r\n开启服务器模式 ......\r\n" );
	while ( !	ESP8266_StartOrShutServer ( ENABLE, macUser_ESP8266_TcpServer_Port, macUser_ESP8266_TcpServer_OverTime ) );

	
  ESP8266_Inquire_ApIp ( cStr, 20 );
	printf ( "\n本模块WIFI为 %s，密码开放\nAP IP 为：%s，开启的端口为：%s\n手机网络助手连接该 IP 和端口，最多可连接5个客户端\n",
           macUser_ESP8266_BulitApSsid, cStr, macUser_ESP8266_TcpServer_Port );
	while((pCh = strstr((const char*)Esp8266_rx_buf,(const char*)"CONNECT")) == NULL)
	{
		OLED_ShowString(0,2,(char *)macUser_ESP8266_BulitApSsid);  
		OLED_ShowString(0,4,(char *)cStr);  		
		OLED_ShowString(0,6,(char *)macUser_ESP8266_TcpServer_Port);  
	};
	
	Esp8266_rx_length = 0;
	Esp8266_rx_flag = 0;
	
	printf( "\r\n配置 ESP8266 完毕\r\n" );
  
}


#endif

extern uint8_t Curtain_flag;
extern uint8_t auto_flag;

/**
  * @brief  ESP8266 检查接收信息并发送数据测试函数
  * @param  无
  * @retval 无
  */
void ESP8266_CheckRecv_SendDataTest(void)
{
  
  if ( Esp8266_rx_flag )
  {
    USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, DISABLE ); //禁用串口接收中断
		if((pCh = strstr ( Esp8266_rx_buf,"Auto")) != NULL){
			switch(*(pCh+4)){
				case '0':auto_flag = 0;break;
				case '1':auto_flag = 1;break;	
				default:break;				
			}
		}
		if((pCh = strstr ( Esp8266_rx_buf,"Curtain")) != NULL){
			switch(*(pCh+7)){
				case '0':sg90(0);Curtain_flag = 0; break;
				case '1':sg90(1);Curtain_flag = 1;break;	
				default:break;				
			}
		}
		memset(Esp8266_rx_buf,0,Esp8266_rx_length);
		Esp8266_rx_flag = 0;
		Esp8266_rx_length = 0;
    USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, ENABLE ); //使能串口接收中断
    
  }
  
}

