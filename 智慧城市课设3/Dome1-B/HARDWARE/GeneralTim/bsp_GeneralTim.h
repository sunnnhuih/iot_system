#ifndef __BSP_GENERALTIME_H
#define __BSP_GENERALTIME_H


#include "stm32f10x.h"

// TIM3 输出比较通道1
#define            General_TIM_CH1_PULSE         0
  


/**************************函数声明********************************/
void TIM2_Init(u16 psc, u16 arr);
void TIM3_Init(u16 psc, u16 arr);
void TIM4_Init(u16 psc, u16 arr);
void sg90(int flag);
#endif	/* __BSP_GENERALTIME_H */


