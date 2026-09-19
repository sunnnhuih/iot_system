#include "bsp_led.h"   
void LED1_Init(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
		/*开启LED相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd( BSP_LED_GPIO_CLK, ENABLE);
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = BSP_LED_GPIO_PIN;	
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(BSP_LED_GPIO_PORT, &GPIO_InitStructure);	
		/* 关闭所有led灯	*/
		GPIO_SetBits(BSP_LED_GPIO_PORT, BSP_LED_GPIO_PIN);	


		
}

/*********************************************END OF FILE**********************/
