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

/** 普通模式下，车进出库时指示灯1亮
	* 注册、充值模式下，指示灯2常亮直到退出充值模式
	*	注册、充值模式下，默认每次刷卡充值200
	* 车库卡 CARD_ID_ADDR 			     			 --- 20位
	*	车存库时间 CARD_TIME_ADDR    			 --- 21位
	* 车库卡内金额 CARD_BALANCE_ADDR      --- 22位
	* 车库收费为每小时 6 元
	**/

//车库卡相关信息位于第5扇区，初识密码A：mifare_one_default_key
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
	
	//默认充值模式下存储金额
	uint16_t fault_store=200;
	
	uint8_t i=0;
  uint8_t ucArray_ID[4];  	 /*先后存放IC卡的类型和UID(IC卡序列号)*/                                                                                           
	uint8_t ucStatusReturn;    /*返回状态*/                                                                                         
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
	
	//STA模式
	#if 1
	printf ( "\r\n正在配置 ESP8266 ......\r\n" );

	macESP8266_CH_ENABLE();
	//AT测试
	ESP8266_AT_Test ();
	//设置WI-FI模式
	ESP8266_Net_Mode_Choose ( STA );
	//加入外部无线WIFI
  while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	//关闭多连接
	ESP8266_Enable_MultipleId ( DISABLE );
	//连接服务器端口
	while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	//配置透传模式
	while ( ! ESP8266_UnvarnishSend () );
	
	printf ( "\r\n配置 ESP8266 完毕\r\n" );
	#endif
	
	printf("\r\n test \r\n");
  while( 1 )
  {
		//显示车库剩余车位情况
		delay_ms(500);
		sprintf((char*)str_oled,"%d",vacancy);
		OLED_Show_State(0,0);
		OLED_Show_Num(50,4,str_oled);
		
		/*寻卡*/
		if ( ( ucStatusReturn = RC522_Search ( PICC_REQALL, ucArray_ID ) ) != MI_OK )  
			/*若失败再次寻卡*/
			ucStatusReturn = RC522_Search ( PICC_REQALL, ucArray_ID );		                                                 

		if ( ucStatusReturn == MI_OK  )
		{
			/*防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）*/
			if ( RC522_Anticoll ( ucArray_ID ) == MI_OK )
			{
				#if 1     //选卡，通过密钥检验,未进入序列卡池的将第五扇区初始化
				RC522_SelectCard(ucArray_ID);
				
				//当前卡序列号
				card_series_id = u8Array_to_u32(ucArray_ID);
				printf("\r\n当前选中卡序列号为：	0x%08X\r\n",card_series_id);
				
				for(i=0;i<10;i++)
				{
					ucStatusReturn = RC522_Password(0x60,CARD_ID_ADDR,key_fault,ucArray_ID);
					if(ucStatusReturn == MI_OK)
						break;
					else
						printf("》》》密钥检验错误");
				}
				
				for(i=0;i<10;i++)
				{
					if(card_series_id == CARD_SERIES_ID[i])
					{
						if(RW_mode == 0)  //普通模式
						{
							//读取车库标记ID地址内容
							RC522_Read(CARD_ID_ADDR,card_id,1);
							card_Rid = card_id[0];
							
							//检测选定卡是否进入过车库
							//标记ID池中有选定卡的标记ID，即此次为出库
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
									printf("\r\n》》》余额不足");
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
									
									printf("\r\n》》》出库");
									printf("\r\n出库时间标记：%d day,%d hour,%d minutes",time_tab.time_d,time_tab.time_h,time_tab.minutes);
									printf("\r\n本次总计费：%d",cost);
									printf("\r\n卡内余额：%d",balance);
									
									
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
									
									sprintf(cStr,"》》》出库--卡序列号:0x%08X,		出库时间标记：%d day,%d hour,%d minutes,		停车费用：%d,		卡内余额：%d		",
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
							//遍历标记ID池没有选定卡进入车库的标记ID，即此次为入库
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
								
								sprintf(cStr,"》》》入库--卡序列号:0x%08X,		入库时间标记：%d day,%d hour,%d minutes,		卡内余额：%d		",
																						card_series_id,timestamp_tab[2],timestamp_tab[1],timestamp_tab[0],balance);
								ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
								
								printf("\r\n》》》入库");
								printf("\r\n入库时间标记：%d day,%d hour,%d minutes",time_tab.time_d,time_tab.time_h,time_tab.minutes);
								printf("\r\n卡内余额：%d",balance);
								
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
						if(RW_mode == 1)	//充值模式，2灯亮
						{
							RC522_Read(CARD_BALANCE_ADDR,balance_tab,2);
							balance=(balance_tab[1]<<8)|(balance_tab[0]);
							balance = balance+fault_store;
							u16_to_u8Array(&balance,balance_tab);
							RC522_Write(CARD_BALANCE_ADDR,balance_tab);
							
							sprintf(cStr,"》》》充值--卡序列号:0x%08X,		充值200元,卡内余额：%d		",card_series_id,balance);
							ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
							
							printf("\r\n》》》充值");
							printf("\r\n卡内余额：%d",balance);
							sprintf((char*)str_oled,"%d",balance);
							OLED_CLS();
							OLED_Show_Balance(0,0);
							OLED_Show_Num(40,4,str_oled);
							delay_ms(1500);
							OLED_CLS();
						}
						break;
					}
					if(i==9 && RW_mode==1)//注册模式，2灯亮
					{
						//加入序列池，初始化第五扇区
						CARD_SERIES_ID[num]=card_series_id;
						RC522_Write(CARD_ID_ADDR,card_data_default);
						RC522_Write(CARD_TIME_ADDR,card_data_default);
						RC522_Write(CARD_BALANCE_ADDR,card_data_default);
						
						RC522_Read(CARD_BALANCE_ADDR,balance_tab,2);
						balance=(balance_tab[1]<<8)|(balance_tab[0]);
						balance = balance+100;
						u16_to_u8Array(&balance,balance_tab);
						RC522_Write(CARD_BALANCE_ADDR,balance_tab);
						
						sprintf(cStr,"新卡注册》》》卡序列号:0x%08X,		卡内余额：%d		",card_series_id,balance);
						ESP8266_SendString ( ENABLE, cStr, 0, Single_ID_0 );
						
						printf("\r\n新卡注册成功");
						printf("\r\n卡内余额：%d",balance);
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
		//断线重连
		if ( ucTcpClosedFlag )                                             //检测是否失去连接
		{
			ESP8266_ExitUnvarnishSend ();                                    //退出透传模式
			
			do ucStatus = ESP8266_Get_LinkStatus ();                         //获取连接状态
			while ( ! ucStatus );
			
			if ( ucStatus == 4 )                                             //确认失去连接后重连
			{
				printf ( "\r\n正在重连热点和服务器 ......\r\n" );
				while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );
				while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
				printf ( "\r\n重连热点和服务器成功\r\n" );
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

