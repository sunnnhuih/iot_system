#include "stm32f10x.h"
#include "bsp_usart.h"	  
#include "stdio.h"
#include "string.h"
#include "stm32f10x_tim.h"
#include "bsp_led.h"
#include "bsp_GeneralTim.h" 


////////////////////////////////////////////////////////////////////////////////// 	

//////////////////////////////////////////////////////////////////
//鍔犲叆浠ヤ笅浠ｇ爜,鏀寔printf鍑芥暟,鑰屼笉闇€瑕侀€夋嫨use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//鏍囧噯搴撻渶瑕佺殑鏀寔鍑芥暟                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//瀹氫箟_sys_exit()浠ラ伩鍏嶄娇鐢ㄥ崐涓绘満妯″紡    
int _sys_exit(int x) 
{ 
	x = x; 
	return 0;
} 
//閲嶅畾涔塮putc鍑芥暟 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//寰幆鍙戦€?鐩村埌鍙戦€佸畬姣?  
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1

UART_BUF buf_uart1;     //CH340
//鍒濆鍖朓O 涓插彛1 
//bound:娉㈢壒鐜?
void usart1_init(u32 bound){
   //GPIO绔彛璁剧疆
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//浣胯兘USART1锛孏PIOA鏃堕挓
	USART_DeInit(USART1);  //澶嶄綅涓插彛1
 //USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//澶嶇敤鎺ㄦ尳杈撳嚭
	GPIO_Init(GPIOA, &GPIO_InitStructure); //鍒濆鍖朠A9
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//娴┖杈撳叆
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //鍒濆鍖朠A10

  //USART 鍒濆鍖栬缃?

	USART_InitStructure.USART_BaudRate = bound;//涓€鑸缃负115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//瀛楅暱涓?浣嶆暟鎹牸寮?
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//涓€涓仠姝綅
	USART_InitStructure.USART_Parity = USART_Parity_No;//鏃犲鍋舵牎楠屼綅
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//鏃犵‖浠舵暟鎹祦鎺у埗
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//鏀跺彂妯″紡
	USART_Init(USART1, &USART_InitStructure); //鍒濆鍖栦覆鍙?
	
	USART_Cmd(USART1, ENABLE);                    //浣胯兘涓插彛 

	
#if EN_USART1_RX	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//寮€鍚浉鍏充腑鏂?
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//寮€鍚浉鍏充腑鏂?
	USART_ClearFlag(USART1, USART_FLAG_TC);
	//Usart1 NVIC 閰嶇疆
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//涓插彛1涓柇閫氶亾
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//鎶㈠崰浼樺厛绾?
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//瀛愪紭鍏堢骇3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ閫氶亾浣胯兘
	NVIC_Init(&NVIC_InitStructure);	//鏍规嵁鎸囧畾鐨勫弬鏁板垵濮嬪寲VIC瀵勫瓨鍣ㄣ€?

#endif

}

/*********************************涓插彛1鐨勬湇鍔″嚱鏁?************************************************/
void USART1_Send_byte(char data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, data);
}
/*-------------------------------------------------*/
/*鍑芥暟鍚嶏細涓插彛1 鍙戦€佹暟缁?                            */
/*鍙? 鏁帮細bound锛氭尝鐗圭巼                             */
/*杩斿洖鍊硷細鏃?                                       */
/*-------------------------------------------------*/
void USART1_Send(char *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		USART1_Send_byte(Data[i]);
	}
}
void USART1_SendStr(char*SendBuf)//涓插彛1鎵撳嵃鏁版嵁
{
	while(*SendBuf)
	{
        while((USART1->SR&0X40)==0);//绛夊緟鍙戦€佸畬鎴?
        USART1->DR = (u8) *SendBuf; 
				SendBuf++;
	}
}

/*****************************************************
娓呯┖鐢佃剳鍙嶉鐨勭紦鍐叉暟鎹?涓插彛1
*****************************************************/
void Clear_Buffer_UART1(void)//娓呯┖缂撳瓨
{
    buf_uart1.index=0;
	  buf_uart1.rx_flag=0;
    memset(buf_uart1.buf,0,BUFLEN);
}
void UART1_receive_process_event(char ch )     //涓插彛2缁?g鐢?
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

//涓插彛1鐨勬帴鏀朵腑鏂▼搴?
void USART1_IRQHandler(void)                                //涓插彛1涓柇鏈嶅姟绋嬪簭
{
		uint8_t Res;
		Res=Res;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //鎺ユ敹涓柇锛屽彲浠ユ墿灞曟潵鎺у埗
    {
        Res=USART_ReceiveData(USART1);//鎺ユ敹妯″潡鐨勬暟鎹?

        UART1_receive_process_event(Res);//鎺ユ敹妯″潡鐨勬暟鎹?
    } 
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //妯″潡绌洪棽
    {
        Res=USART_ReceiveData(USART1);//鎺ユ敹妯″潡鐨勬暟鎹?

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
//鍒濆鍖朓O 涓插彛2
//pclk1:PCLK1鏃堕挓棰戠巼(Mhz)
//bound:娉㈢壒鐜?
void usart2_init(u32 bound)
{  	 

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//浣胯兘锛孏PIOA鏃堕挓
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//USART2
		USART_DeInit(USART2);  //澶嶄綅涓插彛2
	 //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//澶嶇敤鎺ㄦ尳杈撳嚭
    GPIO_Init(GPIOA, &GPIO_InitStructure); //鍒濆鍖朠A2
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//娴┖杈撳叆
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //鍒濆鍖朠A3

   
   //USART 鍒濆鍖栬缃?

		USART_InitStructure.USART_BaudRate = bound;//115200
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//瀛楅暱涓?浣嶆暟鎹牸寮?
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//涓€涓仠姝綅
		USART_InitStructure.USART_Parity = USART_Parity_No;//鏃犲鍋舵牎楠屼綅
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//鏃犵‖浠舵暟鎹祦鎺у埗
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//鏀跺彂妯″紡
    USART_Init(USART2, &USART_InitStructure); //鍒濆鍖栦覆鍙?
    USART_Cmd(USART2, ENABLE);                    //浣胯兘涓插彛 




#if EN_USART2_RX	
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//寮€鍚浉鍏充腑鏂?
		USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//寮€鍚浉鍏充腑鏂?
		
    //Usart1 NVIC 閰嶇疆
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//涓插彛1涓柇閫氶亾
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//鎶㈠崰浼樺厛绾?
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		//瀛愪紭鍏堢骇3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ閫氶亾浣胯兘
    NVIC_Init(&NVIC_InitStructure);	//鏍规嵁鎸囧畾鐨勫弬鏁板垵濮嬪寲VIC瀵勫瓨鍣ㄣ€?
#endif
}




/*********************************涓插彛2鐨勬湇鍔″嚱鏁?************************************************/
void USART2_Send_byte(char data)
{
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, data);
}




/*-------------------------------------------------*/
/*鍑芥暟鍚嶏細涓插彛2 鍙戦€佹暟缁?                            */
/*鍙? 鏁帮細bound锛氭尝鐗圭巼                             */
/*杩斿洖鍊硷細鏃?                                       */
/*-------------------------------------------------*/
void USART2_Send(char *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		USART2_Send_byte(Data[i]);
	}
}



void USART2_SendStr(char*SendBuf)//涓插彛1鎵撳嵃鏁版嵁
{
	while(*SendBuf)
	{
        while((USART2->SR&0X40)==0);//绛夊緟鍙戦€佸畬鎴?
        USART2->DR = (u8) *SendBuf; 
        SendBuf++;
	}
}


void usart2_receive_process_event(unsigned char ch )     //涓插彛2缁?g鐢?
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

void USART2_IRQHandler(void)                            //涓插彛2鎺ユ敹鍑芥暟
{
		char Res;
		Res=Res;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //鎺ユ敹涓柇锛屽彲浠ユ墿灞曟潵鎺у埗
    {
        Res=USART_ReceiveData(USART2);//鎺ユ敹妯″潡鐨勬暟鎹?
        usart2_receive_process_event(Res);//鎺ユ敹妯″潡鐨勬暟鎹?
				
    } 
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  //妯″潡绌洪棽
    {
        Res=USART_ReceiveData(USART2);//鎺ユ敹妯″潡鐨勬暟鎹?
				buf_lora.rx_flag=1;
    } 
}
#endif


#if EN_USART3

UART_BUF buf_uart3;     //TTL
void usart3_init(u32 bound)
{
    //GPIO绔彛璁剧疆
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//浣胯兘锛孏PIOA鏃堕挓
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//USART3
		USART_DeInit(USART3);  //澶嶄綅涓插彛3
	 //USART3_TX   PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//澶嶇敤鎺ㄦ尳杈撳嚭
    GPIO_Init(GPIOB, &GPIO_InitStructure); //鍒濆鍖朠A2
   
    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//娴┖杈撳叆
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //鍒濆鍖朠B11

  
   //USART 鍒濆鍖栬缃?

		USART_InitStructure.USART_BaudRate = bound;//115200
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//瀛楅暱涓?浣嶆暟鎹牸寮?
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//涓€涓仠姝綅
		USART_InitStructure.USART_Parity = USART_Parity_No;//鏃犲鍋舵牎楠屼綅
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//鏃犵‖浠舵暟鎹祦鎺у埗
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//鏀跺彂妯″紡
    USART_Init(USART3, &USART_InitStructure); //鍒濆鍖栦覆鍙?
	
    USART_Cmd(USART3, ENABLE);                    //浣胯兘涓插彛 

#if EN_USART3_RX	

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//寮€鍚浉鍏充腑鏂?
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//寮€鍚浉鍏充腑鏂?
	USART_ClearFlag(USART3, USART_FLAG_TC);
	//Usart3 NVIC 閰嶇疆
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//涓插彛3涓柇閫氶亾
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//鎶㈠崰浼樺厛绾?
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//瀛愪紭鍏堢骇3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ閫氶亾浣胯兘
	NVIC_Init(&NVIC_InitStructure);	//鏍规嵁鎸囧畾鐨勫弬鏁板垵濮嬪寲VIC瀵勫瓨鍣ㄣ€?
#endif
	
}


void USART3_Send_byte(char data)
{
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3, data);
}


/*-------------------------------------------------*/
/*鍑芥暟鍚嶏細涓插彛2 鍙戦€佹暟缁?                            */
/*鍙? 鏁帮細bound锛氭尝鐗圭巼                             */
/*杩斿洖鍊硷細鏃?                                       */
/*-------------------------------------------------*/
void USART3_Send(char *Data,uint16_t Len)
{ 
	uint16_t i;
	for(i=0; i<Len; i++)
	{
		USART3_Send_byte(Data[i]);
	}
}




void USART3_SendStr(char*SendBuf)//涓插彛3鎵撳嵃鏁版嵁
{
	while(*SendBuf)
	{
        while((USART3->SR&0X40)==0);//绛夊緟鍙戦€佸畬鎴?
        USART3->DR = (u8) *SendBuf; 
        SendBuf++;
	}
}


/*****************************************************
娓呯┖鐢佃剳鍙嶉鐨勭紦鍐叉暟鎹?涓插彛1
*****************************************************/
void Clear_Buffer_UART3(void)//娓呯┖缂撳瓨
{
    buf_uart3.index=0;
	  buf_uart3.rx_flag=0;
    memset(buf_uart3.buf,0,BUFLEN);	
}
void Clear_Buffer_UART2(void)//娓呯┖缂撳瓨
{
    buf_lora.index=0;
	  buf_lora.rx_flag=0;
    memset(buf_lora.buf,0,BUFLEN);	
}
void USART3_receive_process_event(char ch )     //涓插彛2缁?g鐢?
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
void USART3_IRQHandler(void)                                //涓插彛3涓柇鏈嶅姟绋嬪簭
{
		char Res;
		Res=Res;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //鎺ユ敹涓柇锛屽彲浠ユ墿灞曟潵鎺у埗
    {
        Res=USART_ReceiveData(USART3);//鎺ユ敹妯″潡鐨勬暟鎹?
        USART3_receive_process_event(Res);
    } 
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  //妯″潡绌洪棽
    {
        Res=USART_ReceiveData(USART3);//鎺ユ敹妯″潡鐨勬暟鎹?
				buf_uart3.rx_flag=1;
    } 
} 	

#endif




