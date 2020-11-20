#include "stm32f10x.h"
#include "servo_motor.h"
#include "bsp_usart.h"
#include "oled_i2c.h"
#include "bsp_systick.h"
#include "bsp_led.h"
#include "rfid_rc522.h"
#include "bsp_key.h"
#include "bsp_time.h"
#include "Common.h"
#include "bsp_esp8266.h"


extern TIME_STRUCT time_tab;
extern uint8_t  	 RW_mode;

RF_CARD_STRUCT rf_card={0};

RF_CARD_STRUCT rf_bank[2]={0};

uint32_t u8Array_to_u32(uint8_t* Array);
void u16_to_u8Array(unsigned short  * variable,unsigned char * Array);

/** ��ͨģʽ�£���������ʱָʾ��1��
	* ע�ᡢ��ֵģʽ�£�ָʾ��2����ֱ���˳���ֵģʽ
	*	ע�ᡢ��ֵģʽ�£�Ĭ��ÿ��ˢ����ֵ200
	* ���⿨ CARD_ID_ADDR 			     			 --- 20λ
	*	�����ʱ�� CARD_TIME_ADDR    			 --- 21λ
	* ���⿨�ڽ�� CARD_BALANCE_ADDR      --- 22λ
	* �����շ�ΪÿСʱ 6 Ԫ
	**/

//���⿨�����Ϣλ�ڵ�5��������ʶ����A��mifare_one_default_key
#define CARD_ID_ADDR         			20
#define CARD_TIME_ADDR            21
#define CARD_BALANCE_ADDR         22

int main(void)
{
	uint32_t card_series_id=0;
	uint32_t CARD_SERIES_ID[10]={0};
	uint8_t  num=0;
	uint8_t  card_Rid=0;
	uint8_t  card_in[1]={0x2E};
	
	//Ĭ�ϳ�ֵģʽ�´洢���
	uint16_t fault_store=200;
	
	uint8_t i=0;
  uint8_t ucArray_ID[4];  	 /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/                                                                                           
	uint8_t ucStatusReturn;    /*����״̬*/                                                                                         
	uint8_t ucStatus;
	
	uint8_t  card_data_default[8]={0};
	uint8_t  card_id[8]={0};
	
  unsigned char str_oled[]="";
	uint8_t  vacancy=10;
	
	uint8_t  timestamp_tab[3];
	
	uint8_t  balance_tab[2]={0};
	uint16_t balance=0;
	uint16_t cost=0;
	
	char 		 cStr[100]={0};

	
	USART1_Config(115200);
	I2C_Configuration();
	
	TIM1_Init();
	LED_Config();
	RC522_Config();
	OLED_Init();
	TIM6_Init();
	KEY0_Config();
	KEY1_Config();
	ESP8266_Init();
	
	OLED_CLS();
	RC522_Reset();
	M500PcdConfigISOType ( 'A' );
	
	//STAģʽ
	#if 1
	printf ( "\r\n�������� ESP8266 ......\r\n" );

	macESP8266_CH_ENABLE();
	//AT����
	ESP8266_AT_Test ();
	//����WI-FIģʽ
	ESP8266_Net_Mode_Choose ( STA );
	//�����ⲿ����WIFI
  while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	//�رն�����
	ESP8266_Enable_MultipleId ( DISABLE );
	//���ӷ������˿�
	while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	//����͸��ģʽ
	while ( ! ESP8266_UnvarnishSend () );
	
	printf ( "\r\n���� ESP8266 ���\r\n" );
	#endif
	
	printf("\r\n test \r\n");
  while( 1 )
  {
		//��ʾ����ʣ�೵λ���
		delay_ms(500);
		sprintf((char*)str_oled,"%d",vacancy);
		OLED_Show_State(0,0);
		OLED_Show_Num(50,4,str_oled);
		
		/*Ѱ��*/
		if ( ( ucStatusReturn = RC522_Search ( PICC_REQALL, ucArray_ID ) ) != MI_OK )  
			/*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = RC522_Search ( PICC_REQALL, ucArray_ID );		                                                 

		if ( ucStatusReturn == MI_OK  )
		{
			/*����ײ�����ж��ſ������д��������Χʱ������ͻ���ƻ������ѡ��һ�Ž��в�����*/
			if ( RC522_Anticoll ( ucArray_ID ) == MI_OK )
			{
				#if 1     //ѡ����ͨ����Կ����,δ�������п��صĽ�����������ʼ��
				RC522_SelectCard(ucArray_ID);
				
				//��ǰ�����к�
				card_series_id = u8Array_to_u32(ucArray_ID);
				printf("\r\n��ǰѡ�п����к�Ϊ��	0x%08X\r\n",card_series_id);
				
				for(i=0;i<10;i++)
				{
					ucStatusReturn = RC522_Password(0x60,CARD_ID_ADDR,key_fault,ucArray_ID);
					if(ucStatusReturn == MI_OK)
						break;
					else
						printf("��������Կ�������");
				}
				
				for(i=0;i<10;i++)
				{
					if(card_series_id == CARD_SERIES_ID[i])
					{
						if(RW_mode == 0)  //��ͨģʽ
						{
							//��ȡ������ID��ַ����
							RC522_Read(CARD_ID_ADDR,card_id,1);
							card_Rid = card_id[0];
							
							//���ѡ�����Ƿ���������
							//���ID������ѡ�����ı��ID�����˴�Ϊ����
							if(card_Rid == card_in[0])
							{
								RC522_Read(CARD_TIME_ADDR,timestamp_tab,3);
								RC522_Read(CARD_BALANCE_ADDR,balance_tab,2);
								
								balance=(balance_tab[1]<<8)|(balance_tab[0]);
								
								cost = ((time_tab.time_d-timestamp_tab[2])*24+(time_tab.time_h-timestamp_tab[1]))*6;
								if(cost == 0)
									cost += 6;
								if(cost > balance)
								{
									printf("\r\n����������");
									OLED_CLS();
									OLED_Show_Balance_noten(0,0);
									delay_ms(1000);
								}
								else
								{
									RC522_Write(CARD_ID_ADDR,card_data_default);
									balance = balance-cost;
									u16_to_u8Array(&balance,balance_tab);
									RC522_Write(CARD_BALANCE_ADDR,balance_tab);
									
									printf("\r\n����������");
									printf("\r\n����ʱ���ǣ�%d day,%d hour,%d minutes",time_tab.time_d,time_tab.time_h,time_tab.minutes);
									printf("\r\n�����ܼƷѣ�%d",cost);
									printf("\r\n������%d",balance);
									
									
									sprintf((char*)str_oled,"%d",cost);
									OLED_CLS();
									OLED_Show_Cost(0,0);
									OLED_Show_Num(40,4,str_oled);
									delay_ms(1500);
									
									sprintf((char*)str_oled,"%d",balance);
									OLED_CLS();
									OLED_Show_Balance(0,0);
									OLED_Show_Num(40,4,str_oled);
									delay_ms(1500);
									
									sprintf((char*)str_oled,"bye.");
									OLED_CLS();
									OLED_Show_Num(20,2,str_oled);
									
									sprintf(cStr,"����������--�����к�:0x%08X,		����ʱ���ǣ�%d day,%d hour,%d minutes,		ͣ�����ã�%d,		������%d		",
																						card_series_id,time_tab.time_d,time_tab.time_h,time_tab.minutes,cost,balance);
									ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
									
									TIM_SetCompare4(TIM1,20000-600);
									LED_1_ON;
									delay_ms(1500);
									TIM_SetCompare4(TIM1,20000-1900);
									LED_1_OFF;
									vacancy = vacancy+1;
									if(vacancy > 10)
									{
										vacancy=10;
									}
									OLED_CLS();
								}
							}
							//�������ID��û��ѡ�������복��ı��ID�����˴�Ϊ���
							else
							{
								RC522_Write(CARD_ID_ADDR,card_in);
								delay_ms(10);
								timestamp_tab[0]=time_tab.minutes ;
								timestamp_tab[1]=time_tab.time_h ;
								timestamp_tab[2]=time_tab.time_d ;
								
								RC522_Write(CARD_TIME_ADDR,timestamp_tab);
								RC522_Read(CARD_BALANCE_ADDR,balance_tab,2);
								
								balance=(balance_tab[1]<<8)|(balance_tab[0]);
								
								sprintf(cStr,"���������--�����к�:0x%08X,		���ʱ���ǣ�%d day,%d hour,%d minutes,		������%d		",
																						card_series_id,timestamp_tab[2],timestamp_tab[1],timestamp_tab[0],balance);
								ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
								
								printf("\r\n���������");
								printf("\r\n���ʱ���ǣ�%d day,%d hour,%d minutes",time_tab.time_d,time_tab.time_h,time_tab.minutes);
								printf("\r\n������%d",balance);
								
								sprintf((char*)str_oled,"%d",balance);
								OLED_CLS();
								OLED_Show_Balance(0,0);
								OLED_Show_Num(40,4,str_oled);
								delay_ms(1500);
								
								sprintf((char*)str_oled,"welcome.");
								OLED_CLS();
								OLED_Show_Num(20,2,str_oled);
								
								TIM_SetCompare4(TIM1,20000-600);
								LED_1_ON;
								delay_ms(1500);
								TIM_SetCompare4(TIM1,20000-1900);
								LED_1_OFF;
								if(vacancy != 0)
								{
									vacancy = vacancy-1;
								}
								OLED_CLS();
							}
						}
						if(RW_mode == 1)	//��ֵģʽ��2����
						{
							RC522_Read(CARD_BALANCE_ADDR,balance_tab,2);
							balance=(balance_tab[1]<<8)|(balance_tab[0]);
							balance = balance+fault_store;
							u16_to_u8Array(&balance,balance_tab);
							RC522_Write(CARD_BALANCE_ADDR,balance_tab);
							
							sprintf(cStr,"��������ֵ--�����к�:0x%08X,		��ֵ200Ԫ,������%d		",card_series_id,balance);
							ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
							
							printf("\r\n��������ֵ");
							printf("\r\n������%d",balance);
							sprintf((char*)str_oled,"%d",balance);
							OLED_CLS();
							OLED_Show_Balance(0,0);
							OLED_Show_Num(40,4,str_oled);
							delay_ms(1500);
							OLED_CLS();
						}
						break;
					}
					if(i==9 && RW_mode==1)//ע��ģʽ��2����
					{
						//�������гأ���ʼ����������
						CARD_SERIES_ID[num]=card_series_id;
						RC522_Write(CARD_ID_ADDR,card_data_default);
						RC522_Write(CARD_TIME_ADDR,card_data_default);
						RC522_Write(CARD_BALANCE_ADDR,card_data_default);
						
						RC522_Read(CARD_BALANCE_ADDR,balance_tab,2);
						balance=(balance_tab[1]<<8)|(balance_tab[0]);
						balance = balance+100;
						u16_to_u8Array(&balance,balance_tab);
						RC522_Write(CARD_BALANCE_ADDR,balance_tab);
						
						sprintf(cStr,"�¿�ע�ᡷ���������к�:0x%08X,		������%d		",card_series_id,balance);
						ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
						
						printf("\r\n�¿�ע��ɹ�");
						printf("\r\n������%d",balance);
						OLED_CLS();
						OLED_Show_signin(0,0);
						delay_ms(1500);
						OLED_CLS();
						sprintf((char*)str_oled,"%d",balance);
						OLED_CLS();
						OLED_Show_Balance(0,0);
						OLED_Show_Num(40,4,str_oled);
						delay_ms(1500);
						OLED_CLS();
						num++;
					}
				}
				#endif
			}
		}
		//��������
		if ( ucTcpClosedFlag )                                             //����Ƿ�ʧȥ����
		{
			ESP8266_ExitUnvarnishSend ();                                    //�˳�͸��ģʽ
			
			do ucStatus = ESP8266_Get_LinkStatus ();                         //��ȡ����״̬
			while ( ! ucStatus );
			
			if ( ucStatus == 4 )                                             //ȷ��ʧȥ���Ӻ�����
			{
				printf ( "\r\n���������ȵ�ͷ����� ......\r\n" );
				while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );
				while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
				printf ( "\r\n�����ȵ�ͷ������ɹ�\r\n" );
			}
			while ( ! ESP8266_UnvarnishSend () );		
		}
	}
}

uint32_t u8Array_to_u32(uint8_t* Array)
{
	uint32_t variable=0x00;
	
	variable = Array[0];
	variable = (variable<<8)|Array[1];
	variable = (variable<<8)|Array[2];
	variable = (variable<<8)|Array[3];
	
	return variable;
}

void u16_to_u8Array(unsigned short  * variable,unsigned char * Array)
{
	Array[0] = (*variable);
	Array[1] = (*variable>>8);
}

