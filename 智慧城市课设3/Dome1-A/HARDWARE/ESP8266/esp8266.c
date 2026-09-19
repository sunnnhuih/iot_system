#include "esp8266.h"
#include "bsp_usart.h"
#include "string.h"
#include "delay.h"
#include "oled.h"



char ESP_Send_Buf[512];



void WIFI_Init(void)
{
	char *str = NULL;
	uint8_t Wait_Count = 0;

	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT+RST\r\n");
	USART3_SendStr(ESP_Send_Buf);
	printf("%s",ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,(const char *)"OK");
		delay_ms(500);
		//USART3_SendStr(ESP_Send_Buf);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\n");
			break;
		}
	}
	str = NULL;
	Wait_Count = 0;
	
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"AT+RST");
	
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"ATE0\r\n");
	USART3_SendStr(ESP_Send_Buf);
	
	delay_ms(1000);

	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT\r\n");
	USART3_SendStr(ESP_Send_Buf);
		printf("%s",ESP_Send_Buf);
		delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,(const char *)"OK");
		delay_ms(500);
		//USART3_SendStr(ESP_Send_Buf);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\b");
			break;
		}		
	}
	str = NULL;
	Wait_Count = 0;
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"AT");
	
	
	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT+CWMODE=1\r\n");
		printf("%s",ESP_Send_Buf);
	USART3_SendStr(ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,(const char *)"OK");
		delay_ms(500);
		//USART3_SendStr(ESP_Send_Buf);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\n");
			break;
		}		
	}
	str = NULL;
	Wait_Count = 0;
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"AT+CWMODE=1");
	
	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT+CIPMODE=0\r\n");
		printf("%s",ESP_Send_Buf);
	USART3_SendStr(ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,(const char *)"OK");
		delay_ms(500);
		//USART3_SendStr(ESP_Send_Buf);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\n");
			break;
		}		
	}
	str = NULL;	
	Wait_Count = 0;
	
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"AT+CIPMODE=0");
	
	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT+CWJAP=\"%s\",\"%s\"\r\n",LYSSID,LYPASSWD);
	USART3_SendStr(ESP_Send_Buf);
		printf("%s",ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,(const char *)"OK");
		delay_ms(500);
		//USART3_SendStr(ESP_Send_Buf);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\n");
			break;
		}		
	}
	str = NULL;		
	Wait_Count = 0;
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"AT+CWJAP");
	
	
}

void ESP_CONNET_ONENET(void)
{
		char *str = NULL;
	uint8_t Wait_Count = 0;
	
	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",devid,proid,token);
	USART3_SendStr(ESP_Send_Buf);
		printf("%s",ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,"OK");
		delay_ms(500);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\b");
			break;
		}		
		//else if(Wait_Count % 4 == 0)USART3_SendStr(ESP_Send_Buf);
	}
	str = NULL;		
	Wait_Count = 0;	

	OLED_Clear();
	OLED_ShowString(0,2,(char *)"MQTTUSERCFG");	
	

	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"%s",MQTTCONN);
		printf("%s",ESP_Send_Buf);
	USART3_SendStr(ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,"OK");
		delay_ms(500);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\b");
			break;
		}		
		//else if(Wait_Count % 4 == 0)USART3_SendStr(ESP_Send_Buf);
	}
	str = NULL;		
	Wait_Count = 0;	
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"MQTTCONN");	
	
	Clear_Buffer_UART3();//清除buf
	memset(ESP_Send_Buf,0,512);
	sprintf(ESP_Send_Buf,"AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/set\",1\r\n",proid,devid);
	USART3_SendStr(ESP_Send_Buf);
		printf("%s",ESP_Send_Buf);
	delay_ms(300);
	while(str == NULL)
	{
		str = strstr((const char *)buf_uart3.buf,"OK");
		delay_ms(500);
		Wait_Count++;
		if(Wait_Count >= 20)
		{
			printf("未查询到指定返回数据\r\b");	
		OLED_Clear();
		OLED_ShowString(0,2,(char *)"failed");	
		delay_ms(1000);delay_ms(1000);delay_ms(1000);
			
			return;
		}		
		//else if(Wait_Count % 4 == 0)USART3_SendStr(ESP_Send_Buf);
	}
	str = NULL;		
	Wait_Count = 0;	
	OLED_Clear();
	OLED_ShowString(0,2,(char *)"Successful!");	
	printf("/************************连接成功*********************/\r\n");

}



/*
			memset(Send_Buf,0,512); 
			sprintf(Send_Buf,"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\",\"{\\\"id\\\":\\\"2313\\\"\\\,\\\"version\\\":\\\"1.0\\\"\\\,\\\"params\\\":{\\\"temp\\\":{\\\"value\\\":%d}\\\,\\\"humi\\\":{\\\"value\\\":%d}\\\,\\\"humi_upper\\\":{\\\"value\\\":%d}}}\",0,0\r\n",
			proid,devid,
			Sersor.Temp,
			Sersor.Humi,
			Sersor.Humi_High_Th
			);//发送温湿度数据
			
			USART3_SendStr(Send_Buf);	
*/
void Send_To_Onenet(void)
{
	char Send_Buf[512];
	static uint8_t Send_Count = 0;

	printf("%s",Send_Buf);
	
	
	
}
void Re_Onenet_Data(void)
{
	char *str;//+MQTTSUBRECV
	
	if(buf_uart3.rx_flag == 1)
	{
		if( strstr((const char *)buf_uart3.buf,(const char *)"+MQTTSUBRECV"))
		{


		Clear_Buffer_UART3();//清除buf
		
		
		
		}
	}
}

