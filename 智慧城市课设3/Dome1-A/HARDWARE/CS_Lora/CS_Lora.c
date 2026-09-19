#include "stm32f10x.h"                  // Device header
#include "cs_lora.h"
#include "bsp_usart.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "delay.h"


void CSTX_Lora_M0_M1_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
 	
 	RCC_APB2PeriphClockCmd(LORA_M0_GPIO_CLK|LORA_M1_GPIO_CLK, ENABLE);	 //使能A端口时钟
	GPIO_InitStructure.GPIO_Pin = LORA_M0_GPIO_PIN;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(LORA_M0_GPIO_PORT, &GPIO_InitStructure);	  
	
	GPIO_InitStructure.GPIO_Pin = LORA_M1_GPIO_PIN;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(LORA_M1_GPIO_PORT, &GPIO_InitStructure);	  
	delay_ms(10);
	Set_Lora_Transmit_Mode();
	
}



void CSTX_Lora_Send(unsigned char *buf,uint8_t len)
{
	USART2_Send((char *)buf,len);
	printf("Lora send ->[");
	for(uint8_t i = 0;i < len;i++)
	{
		printf("%#x ",buf[i]);
	}
	printf("]\r\n");
}

void CSTX_Lora_Send_String(char *data)
{
	USART2_SendStr(data);
}

void CSTX_Lora_Receive_Buf_Clear(void)
{
	Clear_Buffer_UART2();
}

void cstx_reg_Receive_Data(uint8_t *channel,uint16_t *addr)
{
		uint8_t outtime = 100;
		while(outtime)
		{
			if(buf_lora.rx_flag == 1)
			{
				/* Lora reg receive ->[0xc1 0x00 0x09 0x00 0x00 0x00 0x62 0x00 0x17 0x03 0x00 0x00 ] */
				printf("Lora reg receive ->[");
				for(uint8_t i = 0;i < buf_lora.index;i++)
				{
					printf("0x%02x ",buf_lora.buf[i]);
				}
				printf("](%d)\r\n",buf_lora.index);
				
				/* 判断第一个字节是否是 C1 */
				if((uint8_t )buf_lora.buf[0] == 0XC1)
				{
					*addr = buf_lora.buf[3] << 8 | buf_lora.buf[4];
					*channel = buf_lora.buf[8];
				}
				
				
				return ;
			}
			outtime--;
			delay_ms(10);
		}
		if(outtime == 0)printf("lora reg receive fail\n");
}


void Set_Lora_Transmit_Mode(void)
{
	delay_ms(10);
	LORA_M0_RESET;
	LORA_M1_RESET;
}
void Set_Lora_Config_Mode(void)
{
	delay_ms(10);
	LORA_M0_RESET;
	LORA_M1_SET;
}

