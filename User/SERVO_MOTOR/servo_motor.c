#include "servo_motor.h"


static void TIM1_GPIO_Config(void)
{
	//TIM1_CH1--PA8
	GPIO_InitTypeDef    GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

static void TIM1_Mode_Config(void)
{
	//PWM信号的频率 = CLK_CNT/((ARR+1)*(PSC+1))
	//CLK_CNT计数器的时钟，CLK_CNT = Fck_int/(PSC+1)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	
	/* 时基结构体初始化配置 */
	//PWM信号的频率 = CLK_CNT/((ARR+1)*(PSC+1))
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
	//自动重装载寄存器（ARR）的值，累计TIM_Period+1个频率后产生一个更新或者中断    TIM_Period(ARR)
	TIM_TimeBaseInitStruct.TIM_Period = (20000-1);
	//PSC寄存器，用于驱动COUNT计数器的时钟 = Fck_int/(PSC+1)
	TIM_TimeBaseInitStruct.TIM_Prescaler = (72-1);
	//记数模式
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	//重复计数器：达到一定的ARR数值后产生更新或中断
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	//时钟分频因子，配置死区时间时要用到    这里配置为一分频
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
	
	
	/* 输出比较结构体初始化配置 */
	TIM_OCInitTypeDef  TIM_OCInitStruct;
	//模式选择PWM1
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	//占空比 = TIM_Pulse/(TIM_Period+1)  跳变值，当计数器记到这个值时电平发生跳变   TIM.Pulse(CCR)
	TIM_OCInitStruct.TIM_Pulse = 0;
	//输出通道使能
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	//输出通道电平极性配置，高/低电平有效
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC4Init(TIM1,&TIM_OCInitStruct);
	//自动重装载使能
	TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);
	
	//使能时钟在ARR上的预装载寄存器
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	
	//计数器使能
	TIM_Cmd(TIM1,ENABLE);
	//主输出使能，当使用的是通用定时器时，不需要设置
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	
}

void TIM1_Init(void)
{
	TIM1_GPIO_Config();
	TIM1_Mode_Config();
}



