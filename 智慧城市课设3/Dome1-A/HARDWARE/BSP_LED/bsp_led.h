#ifndef __LED_H
#define	__LED_H


#include "stm32f10x.h"


#define BSP_LED_GPIO_PORT    	GPIOC		              /* GPIO端口 */
#define BSP_LED1_GPIO_CLK 	    RCC_APB2Periph_GPIOC		/* GPIO端口时钟 */
#define BSP_LED1_GPIO_PIN			GPIO_Pin_13			        /* 连接到SCL时钟线的GPIO */


/** the macro definition to trigger the led on or off 
  * 1 - off
  *0 - on
  */
#define ON  0
#define OFF 1

/* 使用标准的固件库控制IO*/
#define LED(a)	if (a)	\
					GPIO_SetBits(LED_GPIO_PORT,LED_GPIO_PIN);\
					else		\
					GPIO_ResetBits(LED_GPIO_PORT,LED_GPIO_PIN)


/* 直接操作寄存器的方法控制IO */
#define	digitalHi(p,i)		 {p->BSRR=i;}	 //输出为高电平		
#define digitalLo(p,i)		 {p->BRR=i;}	 //输出低电平
#define digitalToggle(p,i) {p->ODR ^=i;} //输出反转状态


/* 定义控制IO的宏 */
#define BSP_LED_TOGGLE		 digitalToggle(BSP_LED_GPIO_PORT,BSP_LED_GPIO_PIN)
#define BSP_LED_OFF		   digitalHi(BSP_LED_GPIO_PORT,BSP_LED_GPIO_PIN)
#define BSP_LED_ON			   digitalLo(BSP_LED_GPIO_PORT,BSP_LED_GPIO_PIN)


void BSP_LED_Init(void);
#endif /* __LED_H */
