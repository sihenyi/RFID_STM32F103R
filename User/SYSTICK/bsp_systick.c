#include "bsp_systick.h"

void delay_us(uint32_t us)
{
	uint32_t i;
	SysTick_Config(72);//配置时间间隔为72个频率产生中断，SysTick_Config()默认时钟频率为HCLK=72M
											//配置系统时钟频率函数 SysTick_CLKSource();
	
	for(i=0;i<us;i++)
	{
		//软件读取CTRL的最高位COUNTFALG
		while(!((SysTick->CTRL) & (1<<16)));
	}
	
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

