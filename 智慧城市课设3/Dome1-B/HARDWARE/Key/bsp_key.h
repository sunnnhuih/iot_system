#ifndef __BSP_KEY_H
#define	__BSP_KEY_H


#include "stm32f10x.h"


//Òý½Å¶¨Òå
#define KEY1_GPIO_PORT         GPIOA
#define KEY1_GPIO_CLK          RCC_APB2Periph_GPIOA
#define KEY1_GPIO_PIN          GPIO_Pin_0

#define KEY1_VALUE             GPIO_ReadInputDataBit(KEY1_GPIO_PORT,KEY1_GPIO_PIN)

u8 KEY_Scan(u8 mode);
void Key_Process(char Key_Num);



void Key_Init(void);

#endif /* __BSP_KEY_H */
