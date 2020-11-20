#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f10x.h"


#define LED_1_ON      GPIO_ResetBits(GPIOE,GPIO_Pin_5)
#define LED_1_OFF     GPIO_SetBits(GPIOE,GPIO_Pin_5)

#define LED_2_ON      GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define LED_2_OFF			GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define LED_1_TOGGLE  {GPIOE->ODR ^= GPIO_Pin_5;}
#define LED_2_TOGGLE  {GPIOB->ODR ^= GPIO_Pin_5;}

void LED_Config(void);

#endif  /* __BSP_LED_H */

