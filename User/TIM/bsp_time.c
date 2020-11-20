#include "bsp_time.h"

static void TIM6_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // �����ж���Ϊ0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);		
		// �����ж���Դ
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn ;	
		// ���������ȼ�Ϊ 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	 
	  // ������ռ���ȼ�Ϊ3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void TIM6_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBase_Struct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	
	//1s
	//������ʱ��ʱ��Ƶ��=CN_INT/(PSC+1)
	TIM_TimeBase_Struct.TIM_Prescaler = (7200-1);        //PSC
	//TIM_TimeBase_Struct.TIM_CounterMode                //������ʱ��ֻ�����ϼ��������������ģʽ����ѡ������
	TIM_TimeBase_Struct.TIM_Period = (10000-1);          //ARR
	//TIM_TimeBase_Struct.TIM_ClockDivision
	//TIM_TimeBase_Struct.TIM_RepetitionCounter

	// ��ʼ����ʱ��
	TIM_TimeBaseInit(TIM6,&TIM_TimeBase_Struct);
	
	// ����������жϱ�־λ
	TIM_ClearFlag(TIM6,TIM_FLAG_Update);
	
	// �����������ж�
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	
	// ʹ�ܼ�����
	TIM_Cmd(TIM6,ENABLE);	
}

void TIM6_Init(void)
{
  TIM6_NVIC_Config();
	TIM6_Config();
}

