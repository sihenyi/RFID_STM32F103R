#ifndef __BSP_USART_H_
#define __BSP_USART_H_

#include  "stm32f10x.h"
#include <stdio.h>





void USART1_Config(uint32_t baud);
void USART_SendBytes(USART_TypeDef* USARTx,uint8_t data);
void USART_SendHalfWord(USART_TypeDef* USARTx,uint16_t data);
void USART_SendArray(USART_TypeDef* USARTx,uint8_t* array,uint16_t num);
void USART_SendString(USART_TypeDef* USARTx,uint8_t* array);



#endif /* __BSP_USART_H_ */


