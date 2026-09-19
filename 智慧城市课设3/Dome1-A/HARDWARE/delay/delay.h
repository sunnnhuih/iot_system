#ifndef __delay_H
#define	__delay_H



#include "stm32f10x.h"


void SysTick_Init(u8 SYSCLK);
void delay_us(u32 nus);
void delay_ms(u32 nms);

#endif /* __delay_H */
