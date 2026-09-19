#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f10x.h"
	

						 
#define BUFLEN 512      //数组缓存大小
typedef struct _UART_BUF
{
    char buf [BUFLEN+1];               
    unsigned int index ;
		char rx_flag;
}UART_BUF;	

#define EN_USART1 			1		//使能（1）/禁止（0）串口1
#define EN_USART2 			1		//使能（1）/禁止（0）串口2
#define EN_USART3 			0		/* USART3 is owned by the ESP8266 BSP. */
	  	
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define EN_USART2_RX 			1		//使能（1）/禁止（0）串口1接收
#define EN_USART3_RX 			0
	  	
//如果想串口中断接收，请不要注释以下宏定义
void usart1_init(u32 bound);
void usart2_init(u32 bound);
void usart3_init(u32 bound);

void USART1_Send_byte(char data);
void USART2_Send_byte(char data);
void USART3_Send_byte(char data);
void USART1_SendStr(char*SendBuf);
void USART2_SendStr(char*SendBuf);
void USART3_SendStr(char*SendBuf);
void USART1_Send(char *Data,uint16_t Len);
void USART2_Send(char *Data,uint16_t Len);
void USART3_Send(char *Data,uint16_t Len);
void Clear_Buffer_UART1(void);
void Clear_Buffer_UART3(void);//清空缓存
void Clear_Buffer_UART2(void);
extern UART_BUF buf_uart1;     //PC
extern UART_BUF buf_lora;     //4G
extern UART_BUF buf_uart3;     //TTL
#endif


