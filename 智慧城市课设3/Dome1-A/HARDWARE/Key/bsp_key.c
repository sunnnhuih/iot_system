#include "bsp_key.h"   
#include "delay.h" 
#include "stdio.h" 
#include "delay.h"
#include "bsp_io.h"   



void Key_Init(void)
{		

	GPIO_InitTypeDef GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN; 
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
	
	
	
	
}


