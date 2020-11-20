#include "bsp_systick.h"

void delay_us(uint32_t us)
{
	uint32_t i;
	SysTick_Config(72);//����ʱ����Ϊ72��Ƶ�ʲ����жϣ�SysTick_Config()Ĭ��ʱ��Ƶ��ΪHCLK=72M
											//����ϵͳʱ��Ƶ�ʺ��� SysTick_CLKSource();
	
	for(i=0;i<us;i++)
	{
		//�����ȡCTRL�����λCOUNTFALG
		while(!((SysTick->CTRL) & (1<<16)));
	}
	
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

