#ifndef __BSP_ADC_H
#define	__BSP_ADC_H


#include "stm32f10x.h"
#include <stdio.h>

extern __IO u16 ADC_Read[2];  

void MYADC_Init(void);
void MYADC_GPIO_Config(void);
void MYADC1_Mode_Config(void);

#endif /* __MQ_2_H */
