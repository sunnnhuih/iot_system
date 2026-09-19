#include "esp8266.h"
#include "bsp_usart.h"
#include "string.h"
#include "delay.h"
#include "oled.h"



char ESP_Send_Buf[512];


void ESP8266_Rst(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);	
	delay_ms(1000);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);	
}

uint8_t ESP8266_SendAT(const char* atCmd,  char* expectedResp,uint8_t send_count,uint16_t Outtime_ms,uint16_t sendIntervalms)
{
  uint16_t count = 0;
	uint16_t send_num = 0;
	
	Outtime_ms = Outtime_ms / 50;
	There:
	count++;
	send_num = 0;
	Clear_Buffer_UART3();//清除buf
	USART3_SendStr((char *)atCmd);
	printf("[第%d发送到ESP8266]:%s\n",count,atCmd);
	delay_ms(100);
	while(send_num <= Outtime_ms)
	{
		send_num++;
		delay_ms(50);
		if(strstr(buf_uart3.buf,expectedResp))
		{
			printf("[响应成功]：检测到 %s\n", buf_uart3.buf);
			return 0;
		}
		
	}
	if(count < send_count)
	{
		delay_ms(sendIntervalms);
		printf("[响应失败]: %s\n", buf_uart3.buf);
		goto There;
	}
	delay_ms(sendIntervalms);
	return 1;
	
}

uint8_t ESP8266_Init(void)
{
	
	ESP8266_Rst();
	OLED_ShowString(0, 4, (char*)"                ");
	OLED_ShowString(0, 6, (char*)"                ");
	OLED_ShowString(0, 4, (char*)"ESP8266 RST");
	delay_ms(100);
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf,"ATE1\r\n");
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",50,1000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"ATE0");
	}
	else
	{
		printf("初始化失败：模块复位异常\n");
		return 0;
	}
	
	delay_ms(100);
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf,"AT\r\n");
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",10,1000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"AT");
	}
	else
	{
		printf("初始化失败：模块未就绪\n");
		return 0;
	}	
	
	delay_ms(100);
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf,"AT+CWMODE=1\r\n");
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",10,1000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"AT+CWMODE=1");
	}
	else
	{
		printf("初始化失败：模式设置异常\n");
		return 0;
	}			

	
	delay_ms(100);
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", LYSSID, LYPASSWD);
	if(ESP8266_SendAT(ESP_Send_Buf,"WIFI GOT IP",10,5000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"WIFI CONNECT");
    OLED_ShowString(0, 6, (char*)"SUCCESSFUL ");
		
		printf("WiFi初始化完成\n");
	}
	else
	{
		printf("初始化失败：WiFi连接异常\n");
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"WIFI CONNECT");
    OLED_ShowString(0, 6, (char*)"FAIL");
		return 0;
		
	}			
	
	delay_ms(1000);
	return 1;
	
	
}

uint8_t ESP8266_CONNET_EMQ(void)
{
	
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf,"AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",ClientID,Username,Password);
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",10,1000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"CONNECT");
    OLED_ShowString(0, 6, (char*)"SUCCESSFUL ");
		
		printf("连接成功\n");
	}
	else
	{
		printf("连接失败\n");
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"CONNECT");
    OLED_ShowString(0, 6, (char*)"FAIL");
		return 0;
	}				
	
	delay_ms(1000);
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf,"AT+MQTTCONN=0,\"%s\",1883,1\r\n",ServerIP);	
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",2,10000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"LOGIN");
    OLED_ShowString(0, 6, (char*)"SUCCESSFUL ");
		
		printf("登录成功\n");
	}
	else
	{
		printf("登录失败\n");
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"LOGIN");
    OLED_ShowString(0, 6, (char*)"FAIL");
		return 0;
	}			
	
	
	delay_ms(300);
	
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf, "AT+MQTTSUB=0,\"testtopic\",1\r\n");//testtopic/#
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",2,3000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"TOPIC");
    OLED_ShowString(0, 6, (char*)"SUCCESSFUL ");
		
		printf("订阅成功\n");
	}
	else
	{
		printf("订阅失败\n");
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"TOPIC");
    OLED_ShowString(0, 6, (char*)"FAIL");
		return 0;
	}				
	delay_ms(300);
	
	memset(ESP_Send_Buf,0,sizeof(ESP_Send_Buf));
	sprintf(ESP_Send_Buf, "AT+MQTTSUB=0,\"cstxsubdev/%s\",0\r\n",ClientID);
	if(ESP8266_SendAT(ESP_Send_Buf,"OK",2,5000,1000) == 0)/* 发送次数10  单次等待时间1000 失败重发等待时间 */
	{
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"TOPIC");
    OLED_ShowString(0, 6, (char*)"SUCCESSFUL ");
		
		printf("订阅成功\n");
	}
	else
	{
		printf("订阅失败\n");
		OLED_ShowString(0, 4, (char*)"                ");
		OLED_ShowString(0, 6, (char*)"                ");
    OLED_ShowString(0, 4, (char*)"TOPIC");
    OLED_ShowString(0, 6, (char*)"FAIL");
		return 0;
	}					
	
	return 1;
}


void EMQX_SEND_BUF(char *temp,char *humi)
{
	char buf[128] = {0};
	Clear_Buffer_UART3();//清除buf
	sprintf(buf,"AT+MQTTPUB=0,\"%s\",\"{\\\"temp\\\":\\\"%s\\\"\\\,\\\"humi\\\":\\\"%s\\\"}\",0,0\r\n","testtopic",temp,humi);
	USART3_SendStr(buf);
}


