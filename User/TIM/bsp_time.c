#include "bsp_time.h"

static void TIM6_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // 设置中断组为0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);		
		// 设置中断来源
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn ;	
		// 设置主优先级为 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	 
	  // 设置抢占优先级为3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void TIM6_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBase_Struct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	
	//1s
	//基本定时器时钟频率=CN_INT/(PSC+1)
	TIM_TimeBase_Struct.TIM_Prescaler = (7200-1);        //PSC
	//TIM_TimeBase_Struct.TIM_CounterMode                //基本定时器只能向上记数，所以这里的模式可以选择不配置
	TIM_TimeBase_Struct.TIM_Period = (10000-1);          //ARR
	//TIM_TimeBase_Struct.TIM_ClockDivision
	//TIM_TimeBase_Struct.TIM_RepetitionCounter

	// 初始化定时器
	TIM_TimeBaseInit(TIM6,&TIM_TimeBase_Struct);
	
	// 清除计数器中断标志位
	TIM_ClearFlag(TIM6,TIM_FLAG_Update);
	
	// 开启计数器中断
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	
	// 使能计数器
	TIM_Cmd(TIM6,ENABLE);	
}

void TIM6_Init(void)
{
  TIM6_NVIC_Config();
	TIM6_Config();
}

