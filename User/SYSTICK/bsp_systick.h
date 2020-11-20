#ifndef __BSP_SYSTICK_H
#define __BSP_SYSTICK_H

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"

#define delay_ms(x)  delay_us(1000*x);

void delay_us(uint32_t us);



#endif  /* __BSP_SYSTICK_H */

