#include "stm32f10x.h"
#include "bsp_usart.h"	  
#include "stdio.h"
#include "string.h"
#include "stm32f10x_tim.h"
#include "bsp_led.h"
#include "bsp_GeneralTim.h" 


////////////////////////////////////////////////////////////////////////////////// 	

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	x = x; 
	return 0;
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1

UART_BUF buf_uart1;     //CH340
//初始化IO 串口1 
//bound:波特率
void usart1_init(u32 bound){
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	USART_DeInit(USART1);  //复位串口1
 //USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

  //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口
	
	USART_Cmd(USART1, ENABLE);                    //使能串口 

	
#if EN_USART1_RX	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启相关中断
	USART_ClearFlag(USART1, USART_FLAG_TC);
	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif

}

/*********************************串口1的服务函数*************************************************/
void USART1_Send_byte(char data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, data);
}
/*-------------------------------------------------*/
/*函数名：串口1 发送数组                             */
/*参  数：bound：波特率                             */
/*返回值：无                                        */
/*-------------------------------------------------*/
void USART1_Send(char *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		USART1_Send_byte(Data[i]);
	}
}
void USART1_SendStr(char*SendBuf)//串口1打印数据
{
	while(*SendBuf)
	{
        while((USART1->SR&0X40)==0);//等待发送完成 
        USART1->DR = (u8) *SendBuf; 
				SendBuf++;
	}
}

/*****************************************************
清空电脑反馈的缓冲数据 串口1
*****************************************************/
void Clear_Buffer_UART1(void)//清空缓存
{
    buf_uart1.index=0;
	  buf_uart1.rx_flag=0;
    memset(buf_uart1.buf,0,BUFLEN);
}
void UART1_receive_process_event(char ch )     //串口2给4g用
{
    if(buf_uart1.index >= BUFLEN)
    {
        buf_uart1.index = 0 ;
    }
    else
    {
        buf_uart1.buf[buf_uart1.index++] = ch;
    }
}

//串口1的接收中断程序
void USART1_IRQHandler(void)                                //串口1中断服务程序
{
		uint8_t Res;
		Res=Res;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断，可以扩展来控制
    {
        Res=USART_ReceiveData(USART1);//接收模块的数据;
        UART1_receive_process_event(Res);//接收模块的数据
    } 
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //模块空闲
    {
        Res=USART_ReceiveData(USART1);//接收模块的数据;
				buf_uart1.rx_flag=1;
    } 

} 


#endif

#if !EN_USART3
extern UART_BUF buf_lora;
void Clear_Buffer_UART2(void)
{
    buf_lora.index = 0;
    buf_lora.rx_flag = 0;
    memset(buf_lora.buf, 0, BUFLEN);
}
#endif



#if EN_USART2
UART_BUF buf_lora;     //EC200T
//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率 
void usart2_init(u32 bound)
{  	 

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能，GPIOA时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//USART2
		USART_DeInit(USART2);  //复位串口2
	 //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3

   
   //USART 初始化设置

		USART_InitStructure.USART_BaudRate = bound;//115200
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口
    USART_Cmd(USART2, ENABLE);                    //使能串口 




#if EN_USART2_RX	
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启相关中断
		USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启相关中断
		
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
#endif
}




/*********************************串口2的服务函数*************************************************/
void USART2_Send_byte(char data)
{
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, data);
}




/*-------------------------------------------------*/
/*函数名：串口2 发送数组                             */
/*参  数：bound：波特率                             */
/*返回值：无                                        */
/*-------------------------------------------------*/
void USART2_Send(char *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		USART2_Send_byte(Data[i]);
	}
}



void USART2_SendStr(char*SendBuf)//串口1打印数据
{
	while(*SendBuf)
	{
        while((USART2->SR&0X40)==0);//等待发送完成 
        USART2->DR = (u8) *SendBuf; 
        SendBuf++;
	}
}


void usart2_receive_process_event(unsigned char ch )     //串口2给4g用
{
    if(buf_lora.index >= BUFLEN)
    {
        buf_lora.index = 0 ;
    }
    else
    {
        buf_lora.buf[buf_lora.index++] = ch;
    }
}

void USART2_IRQHandler(void)                            //串口2接收函数
{
		char Res;
		Res=Res;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断，可以扩展来控制
    {
        Res=USART_ReceiveData(USART2);//接收模块的数据;
        usart2_receive_process_event(Res);//接收模块的数据
				
    } 
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  //模块空闲
    {
        Res=USART_ReceiveData(USART2);//接收模块的数据;
				buf_lora.rx_flag=1;
    } 
}
#endif


#if EN_USART3

UART_BUF buf_uart3;     //TTL
void usart3_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能，GPIOA时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//USART3
		USART_DeInit(USART3);  //复位串口3
	 //USART3_TX   PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PA2
   
    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PB11

  
   //USART 初始化设置

		USART_InitStructure.USART_BaudRate = bound;//115200
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART3, &USART_InitStructure); //初始化串口
	
    USART_Cmd(USART3, ENABLE);                    //使能串口 

#if EN_USART3_RX	

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启相关中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启相关中断
	USART_ClearFlag(USART3, USART_FLAG_TC);
	//Usart3 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口3中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
#endif
	
}


void USART3_Send_byte(char data)
{
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3, data);
}


/*-------------------------------------------------*/
/*函数名：串口2 发送数组                             */
/*参  数：bound：波特率                             */
/*返回值：无                                        */
/*-------------------------------------------------*/
void USART3_Send(char *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		USART3_Send_byte(Data[i]);
	}
}




void USART3_SendStr(char*SendBuf)//串口3打印数据
{
	while(*SendBuf)
	{
        while((USART3->SR&0X40)==0);//等待发送完成 
        USART3->DR = (u8) *SendBuf; 
        SendBuf++;
	}
}


/*****************************************************
清空电脑反馈的缓冲数据 串口1
*****************************************************/
void Clear_Buffer_UART3(void)//清空缓存
{
    buf_uart3.index=0;
	  buf_uart3.rx_flag=0;
    memset(buf_uart3.buf,0,BUFLEN);	
}
void Clear_Buffer_UART2(void)//清空缓存
{
    buf_lora.index=0;
	  buf_lora.rx_flag=0;
    memset(buf_lora.buf,0,BUFLEN);	
}
void USART3_receive_process_event(char ch )     //串口2给4g用
{
    if(buf_uart3.index >= BUFLEN)
    {
        buf_uart3.index = 0 ;
    }
    else
    {
        buf_uart3.buf[buf_uart3.index++] = ch;
    }
}
int rx_num=0;
void USART3_IRQHandler(void)                                //串口3中断服务程序
{
		char Res;
		Res=Res;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断，可以扩展来控制
    {
        Res=USART_ReceiveData(USART3);//接收模块的数据;
        USART3_receive_process_event(Res);
    } 
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  //模块空闲
    {
        Res=USART_ReceiveData(USART3);//接收模块的数据;
				buf_uart3.rx_flag=1;
    } 
} 	

#endif




