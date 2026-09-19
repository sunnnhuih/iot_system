#include "bsp_GeneralTim.h" 
#include "bsp_esp8266_test.h"
#include "bsp_led.h"   
#include "delay.h" 
#include "string.h" 




void TIM3_Init(u16 psc, u16 arr)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
		/*配置时钟源*/
	TIM_InternalClockConfig(TIM3);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = arr;	
	 // 时钟预分频数为
  TIM_TimeBaseStructure.TIM_Prescaler= psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			//重复计数器，高级定时器才会用到
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，选择向上计数	
	  // 初始化定时器
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		// 清除计数器中断标志位
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
		// 开启计数器中断
  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组为0
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
		// 设置中断来源
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn ;	
		// 设置主优先级为 0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	 
	  // 设置抢占优先级为3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	// 使能计数器
  TIM_Cmd(TIM3, ENABLE);	
}





/*********************************************END OF FILE**********************/
