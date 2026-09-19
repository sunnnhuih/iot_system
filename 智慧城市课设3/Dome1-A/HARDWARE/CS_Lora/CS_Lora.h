#ifndef __CS_LORA_H
#define __CS_LORA_H
#include "stm32f10x.h"                  // Device header




#define LORA_M0_GPIO_PORT         GPIOB
#define LORA_M0_GPIO_CLK          RCC_APB2Periph_GPIOB
#define LORA_M0_GPIO_PIN          GPIO_Pin_5
#define LORA_M0_SET               GPIO_SetBits(LORA_M0_GPIO_PORT,LORA_M0_GPIO_PIN)	    
#define LORA_M0_RESET             GPIO_ResetBits(LORA_M0_GPIO_PORT,LORA_M0_GPIO_PIN) 
#define LORA_M0_STATE             GPIO_ReadInputDataBit(LORA_M0_GPIO_PORT,LORA_M0_GPIO_PIN)


#define LORA_M1_GPIO_PORT         GPIOB
#define LORA_M1_GPIO_CLK          RCC_APB2Periph_GPIOB
#define LORA_M1_GPIO_PIN          GPIO_Pin_4
#define LORA_M1_SET               GPIO_SetBits(LORA_M1_GPIO_PORT,LORA_M1_GPIO_PIN)	    
#define LORA_M1_RESET             GPIO_ResetBits(LORA_M1_GPIO_PORT,LORA_M1_GPIO_PIN) 
#define LORA_M1_STATE             GPIO_ReadInputDataBit(LORA_M1_GPIO_PORT,LORA_M1_GPIO_PIN)



void CSTX_Lora_M0_M1_Init(void);
void CSTX_Lora_Send(unsigned char *buf,uint8_t len);
void CSTX_Lora_Receive_Buf_Clear(void);
void cstx_reg_Receive_Data(uint8_t *channel,uint16_t *addr);
void Set_Lora_Transmit_Mode(void);
void Set_Lora_Config_Mode(void);
void CSTX_Lora_Send_String(char *data);
#endif
