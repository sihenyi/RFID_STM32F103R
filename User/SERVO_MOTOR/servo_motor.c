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
	//PWM�źŵ�Ƶ�� = CLK_CNT/((ARR+1)*(PSC+1))
	//CLK_CNT��������ʱ�ӣ�CLK_CNT = Fck_int/(PSC+1)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	
	/* ʱ���ṹ���ʼ������ */
	//PWM�źŵ�Ƶ�� = CLK_CNT/((ARR+1)*(PSC+1))
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
	//�Զ���װ�ؼĴ�����ARR����ֵ���ۼ�TIM_Period+1��Ƶ�ʺ����һ�����»����ж�    TIM_Period(ARR)
	TIM_TimeBaseInitStruct.TIM_Period = (20000-1);
	//PSC�Ĵ�������������COUNT��������ʱ�� = Fck_int/(PSC+1)
	TIM_TimeBaseInitStruct.TIM_Prescaler = (72-1);
	//����ģʽ
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	//�ظ����������ﵽһ����ARR��ֵ��������»��ж�
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	//ʱ�ӷ�Ƶ���ӣ���������ʱ��ʱҪ�õ�    ��������Ϊһ��Ƶ
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
	
	
	/* ����ȽϽṹ���ʼ������ */
	TIM_OCInitTypeDef  TIM_OCInitStruct;
	//ģʽѡ��PWM1
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	//ռ�ձ� = TIM_Pulse/(TIM_Period+1)  ����ֵ�����������ǵ����ֵʱ��ƽ��������   TIM.Pulse(CCR)
	TIM_OCInitStruct.TIM_Pulse = 0;
	//���ͨ��ʹ��
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	//���ͨ����ƽ�������ã���/�͵�ƽ��Ч
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC4Init(TIM1,&TIM_OCInitStruct);
	//�Զ���װ��ʹ��
	TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);
	
	//ʹ��ʱ����ARR�ϵ�Ԥװ�ؼĴ���
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	
	//������ʹ��
	TIM_Cmd(TIM1,ENABLE);
	//�����ʹ�ܣ���ʹ�õ���ͨ�ö�ʱ��ʱ������Ҫ����
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	
}

void TIM1_Init(void)
{
	TIM1_GPIO_Config();
	TIM1_Mode_Config();
}



