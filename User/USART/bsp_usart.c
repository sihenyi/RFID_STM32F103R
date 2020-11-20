#include "bsp_usart.h"

//USART1,串口1 把数据发送到串口调试助手
void USART1_Config(uint32_t baud)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	USART_InitTypeDef  USART_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	//TX-PA9
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//RX-PA10
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	USART_InitStruct.USART_BaudRate = baud;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_Mode = (USART_Mode_Tx|USART_Mode_Rx);
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1,&USART_InitStruct);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	//使能数据接收中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	USART_Cmd(USART1,ENABLE);
}


//字节
void USART_SendBytes(USART_TypeDef* USARTx,uint8_t data)
{
	USART_SendData(USARTx,data);
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE) == RESET);
}

//半字
void USART_HalfWord(USART_TypeDef* USARTx,uint16_t data)
{
	uint8_t data_h,data_l=0;
	
	data_h = (data&0xff00)>>8;
	data_l = (data&0xff);
	
	USART_SendData(USARTx,data_h);
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE) == RESET);
	
	USART_SendData(USARTx,data_l);
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE) == RESET);
}

//8位数组
void USART_SendArray(USART_TypeDef* USARTx,uint8_t* array,uint16_t num)
{
	uint16_t i=0;
	for(i=0;i<num;i++)
	{
		USART_SendData(USARTx,array[i]);
	}
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TC) == RESET);
}

//字符串
void USART_SendString(USART_TypeDef* USARTx,uint8_t* array)
{
	uint16_t string_subscript=0;
	do
	{
		USART_SendBytes(USARTx,array[string_subscript]);
		string_subscript++;
	}while(array[string_subscript] != '\0');
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TC) == RESET);
}

/*** C标准库函数重定向 至串口1***/
//printf,putchar
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1,(uint8_t) ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	
	return (ch);
}

//scanf,getchar
int fgetc(FILE *f)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);
	
	return (int)USART_ReceiveData(USART1);
}


