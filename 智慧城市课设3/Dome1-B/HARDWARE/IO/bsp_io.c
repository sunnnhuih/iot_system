#include "bsp_io.h"   

char Beep_Flag = 0;
void Beep_Init(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;
		/*开启LED相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd( BEEP_GPIO_CLK, ENABLE);
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN;	
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);	
		/* 关闭所有led灯	*/
		BEEP_OFF;
}
char Fan_Flag = 0;
void Fan_Init(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_APB2PeriphClockCmd(FAN_GPIO_CLK,ENABLE);	//打开GPIO使能时钟，先开复用功能
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = FAN_GPIO_PIN;	
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(FAN_GPIO_PORT, &GPIO_InitStructure);	
		FAN_OFF;

	
	

}

void LED_Init(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_APB2PeriphClockCmd(LED_GPIO_CLK,ENABLE);	//打开GPIO使能时钟，先开复用功能
		/*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;	
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);	
		LED_OFF;

}

char Relay_Flag = 0;
void Relay_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd( Relay_GPIO_CLK, ENABLE);
		 /*选择要控制的GPIO引脚*/
		GPIO_InitStructure.GPIO_Pin = Relay_GPIO_PIN;	
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;   
		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		/*调用库函数，初始化GPIO*/
		GPIO_Init(Relay_GPIO_PORT, &GPIO_InitStructure);	
		/* 关闭所有led灯	*/
		Relay_OFF;
}

//1表示有人 0表示无人
char Infrared_Flag = 0;
void Infrared_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(Infrared_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Pin =Infrared_GPIO_PIN;
	GPIO_Init(Infrared_GPIO_PORT, &GPIO_InitStructure);	

	
}

void MYGPIO_Init(void)
{
//	Infrared_Init();
	//Beep_Init();
	LED_Init();
	Fan_Init();
	Relay_Init();
}
/*********************************************END OF FILE**********************/
