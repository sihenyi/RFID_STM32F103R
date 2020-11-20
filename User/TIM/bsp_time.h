#ifndef  __BSP_TIME_H
#define  __BSP_TIME_H

#include "stm32f10x.h"

typedef struct
{
	uint8_t  minutes;
	uint8_t  time_h;
	uint8_t  time_d;
}TIME_STRUCT;

void TIM6_Init(void);
	
#endif  /* __BSP_TIME_H */

