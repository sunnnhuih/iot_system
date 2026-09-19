#ifndef __BSP_IO_H
#define	__BSP_IO_H


#include "stm32f10x.h"


/* 直接操作寄存器的方法控制IO */
#define	digitalHi(p,i)		 {p->BSRR=i;}	 //输出为高电平		
#define digitalLo(p,i)		 {p->BRR=i;}	 //输出低电平
#define digitalToggle(p,i) {p->ODR ^=i;} //输出反转状态


/***********************************蜂鸣器**********************************************/
#define BEEP_GPIO_PORT    	GPIOA		            /* GPIO端口 */
#define BEEP_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define BEEP_GPIO_PIN		GPIO_Pin_12			        /* 连接到SCL时钟线的GPIO */
/* 定义控制IO的宏 */
#define BEEP_TOGGLE		 	digitalToggle(BEEP_GPIO_PORT,BEEP_GPIO_PIN)
#define BEEP_ON		   		digitalHi(BEEP_GPIO_PORT,BEEP_GPIO_PIN)
#define BEEP_OFF			digitalLo(BEEP_GPIO_PORT,BEEP_GPIO_PIN)
void Beep_Init(void);
extern char Beep_Flag;


/***********************************风扇**********************************************/
#define FAN_GPIO_PORT    	GPIOB			              /* GPIO端口 */
#define FAN_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define FAN_GPIO_PIN		GPIO_Pin_7			        /* 连接到SCL时钟线的GPIO */

/* 定义控制IO的宏 */
#define FAN_TOGGLE		 digitalToggle(FAN_GPIO_PORT,FAN_GPIO_PIN)
#define FAN_OFF		   digitalHi(FAN_GPIO_PORT,FAN_GPIO_PIN)
#define FAN_ON			   digitalLo(FAN_GPIO_PORT,FAN_GPIO_PIN)
void Fan_Init(void);
extern char Fan_Flag;

/***********************************风扇**********************************************/
#define LED_GPIO_PORT    	GPIOB			              /* GPIO端口 */
#define LED_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define LED_GPIO_PIN		GPIO_Pin_8			        /* 连接到SCL时钟线的GPIO */

/* 定义控制IO的宏 */
#define LED_TOGGLE		 digitalToggle(LED_GPIO_PORT,LED_GPIO_PIN)
#define LED_OFF		   digitalHi(LED_GPIO_PORT,LED_GPIO_PIN)
#define LED_ON			   digitalLo(LED_GPIO_PORT,LED_GPIO_PIN)
void LED_Init(void);

/***********************************继电器**********************************************/
#define Relay_GPIO_PORT    	GPIOB		              /* GPIO端口 */
#define Relay_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define Relay_GPIO_PIN			GPIO_Pin_2			        /* 连接到SCL时钟线的GPIO */


/* 定义控制IO的宏 */
#define Relay_TOGGLE		 digitalToggle(Relay_GPIO_PORT,Relay_GPIO_PIN)
#define Relay_OFF		   	 digitalHi(Relay_GPIO_PORT,Relay_GPIO_PIN)
#define Relay_ON			   digitalLo(Relay_GPIO_PORT,Relay_GPIO_PIN)
void Relay_Init(void);
extern char Relay_Flag;


/***********************************人体红外**********************************************/
#define Infrared_GPIO_PORT    	GPIOA	              /* GPIO端口 */
#define Infrared_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define Infrared_GPIO_PIN			GPIO_Pin_0			        /* 连接到SCL时钟线的GPIO */


#define ReadInfrared() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)

void Infrared_Init(void);
extern char Infrared_Flag;



void MYGPIO_Init(void);




#endif /* __BSP_IO_H*/
