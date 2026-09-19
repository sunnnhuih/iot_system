#include "control.h"                  // Device header
#include "bsp_dht11.h"                  // Device header
#include "bsp_usart.h"                  // Device header
#include "bsp_adc.h"                  // Device header
#include "bsp_io.h"                  // Device header
#include "oled.h"                  // Device header
#include "esp8266.h"                  // Device header	
#include "string.h"                  // Device header	
#include "stdio.h"                  // Device header	
#include "stdlib.h"                  // Device header	
#include "bsp_led.h"  

Sensor_ Sensor;
char Oled_Show_Data[128];



void Oled_Show(void)
{
	memset(Oled_Show_Data,0,128);
	sprintf(Oled_Show_Data,"温度:%.1f阈值:%.0f",Sensor.Temp,Sensor.Temp_Th);
	OLED_ShowString(0,0,Oled_Show_Data);
	
	memset(Oled_Show_Data,0,128);
	sprintf(Oled_Show_Data,"湿度:%02d阈值:%.0f",Sensor.Humi,Sensor.Hand_Mode);
	OLED_ShowString(0,6,Oled_Show_Data);
	
	memset(Oled_Show_Data,0,128);
	sprintf(Oled_Show_Data,"光照:%02d阈值:%02d",Sensor.Light,Sensor.Light_Th);
	OLED_ShowString(0,2,Oled_Show_Data);

	memset(Oled_Show_Data,0,128);
	sprintf(Oled_Show_Data,"土壤:%02d阈值:%02d",Sensor.SoilHumi,Sensor.SoilHumi_Th);
	OLED_ShowString(0,4,Oled_Show_Data);	
}


void ESP_Send_Data(void)
{
	char Send_Buf[512];
	static uint8_t Count = 0;
	
	Count++;
	if(Count == 1)
	{
		sprintf(Send_Buf,"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\",\"{\\\"id\\\":\\\"2313\\\"\\\,\\\"version\\\":\\\"1.0\\\"\\\,\\\"params\\\":{\\\"Temp\\\":{\\\"value\\\":%.1f}\\\,\\\"Humi\\\":{\\\"value\\\":%d}\\\,\\\"Temp_Th\\\":{\\\"value\\\":%.0f}}}\",0,0\r\n",
		proid,devid,
		Sensor.Temp,
		Sensor.Humi,
		Sensor.Temp_Th
		);//发送温湿度数据		
	}
	else if(Count == 2)
	{
		sprintf(Send_Buf,"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\",\"{\\\"id\\\":\\\"2313\\\"\\\,\\\"version\\\":\\\"1.0\\\"\\\,\\\"params\\\":{\\\"Light\\\":{\\\"value\\\":%d}\\\,\\\"Light_Th\\\":{\\\"value\\\":%d}\\\,\\\"SoilHumi\\\":{\\\"value\\\":%d}}}\",0,0\r\n",
		proid,devid,
		Sensor.Light,
		Sensor.Light_Th,
		Sensor.SoilHumi
		);
	}
	else if(Count == 3)
	{
		sprintf(Send_Buf,"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\",\"{\\\"id\\\":\\\"2313\\\"\\\,\\\"version\\\":\\\"1.0\\\"\\\,\\\"params\\\":{\\\"Light\\\":{\\\"value\\\":%d}\\\,\\\"SoilHumi_Th\\\":{\\\"value\\\":%d}\\\,\\\"SoilHumi\\\":{\\\"value\\\":%d}}}\",0,0\r\n",
		proid,devid,
		Sensor.Light,
		Sensor.SoilHumi_Th,
		Sensor.SoilHumi
		);

		Count = 0;
	}
	

	printf(Send_Buf);
	USART3_SendStr(Send_Buf);	
}


void Get_Sensor(void)
{
	DHT11_Read_TempAndHumidity(&DHT11_Data);
	Sensor.Temp = DHT11_Data.temp_int + DHT11_Data.temp_deci*0.1;
	Sensor.Humi = DHT11_Data.humi_int;
	
	Sensor.Light = 101 - (float)ADC_Read[0]/4096.0*100.0f;
	Sensor.SoilHumi = 101 - (float)ADC_Read[1]/4096.0*100.0f;	
	
	
	if(Sensor.Hand_Mode == 1)return;
	
	if(Sensor.Temp >= Sensor.Temp_Th)
	{
		FAN_ON;
	}
	else
	{
		FAN_OFF;
	}
	
	
	if(Sensor.SoilHumi <= Sensor.SoilHumi_Th)
	{
		Relay_ON;
	}
	else
	{
		Relay_OFF;
	}
	
	if(Sensor.Light <= Sensor.Light_Th)
	{
		LED_ON;
	}
	else
	{
		LED_OFF;
	}
	
}
//串口3收到数据:+MQTTSUBRECV:0,"$sys/JlrsO2laCG/Demo1/thing/property/set",51,{"id":"2","version":"1.0","params":{"Light_Th":10}}
void Check_Control_Data(void)
{
	char *str;
	if(buf_uart3.rx_flag == 1)
	{
		str = strstr(buf_uart3.buf,"MQTTSUBRECV");
		
		if(str)
		{
			str = strstr(buf_uart3.buf,"Hand_Mode\":true");
			if(str)
			{
				
				
				goto There;
			}
			
			
			str = strstr(buf_uart3.buf,"Temp_Th");
			if(str)
			{
				uint8_t Data;
				if(sscanf(str,"Temp_Th\":%d}}",&Data))
				{
					
					printf("原始数据:%s 解析数据：%d\r\n",str,Data);
				}
				else
				{
					printf("原始数据:%s 解析失败\r\n",str);
				}
				
				goto There;
			}
						
		}
		
		
		
		There:
		Clear_Buffer_UART3();
	}
	
	
}


