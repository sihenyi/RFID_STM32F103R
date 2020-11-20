#ifndef __OLED_I2C_H
#define __OLED_I2C_H

#include "stm32f10x.h"
#include "bsp_systick.h"

#define OLED_ADDRESS    0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

/* STM32 I2C 快速模式 */
#define I2C_Speed              400000

//I2C初始化配置
void I2C_Configuration(void);
void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteData(unsigned char I2C_Data);

void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y); //设置光标位置
void OLED_Fill(unsigned char fill_Data);					  //填充整个屏幕
void OLED_CLS(void);																//清屏
void OLED_ON(void);																	//将OLED从休眠中唤醒
void OLED_OFF(void);																//让OLED休眠 -- 休眠模式下,OLED功耗不到10uA



void OLED_Show_State(unsigned char x, unsigned char y);
void OLED_Show_Cost(unsigned char x, unsigned char y);
void OLED_Show_Balance(unsigned char x, unsigned char y);
void OLED_Show_Balance_noten(unsigned char x, unsigned char y);
void OLED_Show_signin(unsigned char x, unsigned char y);
void OLED_Show_Num(unsigned char x,unsigned char y,unsigned char str[]);

#endif


