#include "oled_i2c.h"
#include "fonts.h"


 /**
  * @brief  I2C_Configuration，初始化硬件IIC引脚
  * @param  无
  * @retval 无
  */
void I2C_Configuration(void)
{
	I2C_InitTypeDef   I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 

	/*I2C1外设时钟使能 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

	/*I2C1外设GPIO时钟使能 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	 /* I2C_SCL、I2C_SDA*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(GPIOB,&GPIO_InitStructure);

  /* I2C 配置 */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;    
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;    //高电平数据稳定，低电平数据变化 SCL 时钟线的占空比
  I2C_InitStructure.I2C_OwnAddress1 =0x10;    					//主机的I2C地址.和其它设备不重复即可
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;  
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; //I2C的寻址模式
  I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;                             //通信速率 */

  I2C_Init(I2C1,&I2C_InitStructure);                                        //I2C1 初始化
  I2C_Cmd(I2C1,ENABLE);                                                     //使能 I2C1

}


 /**
  * @brief  I2C_WriteByte，向OLED寄存器地址写一个byte的数据
  * @param  addr：寄存器地址
    *                   data：要写入的数据
  * @retval 无
  */
void I2C_WriteByte(uint8_t addr,uint8_t data)
{
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C1, ENABLE);//开启I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

	I2C_Send7bitAddress(I2C1, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//寄存器地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//发送数据
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_GenerateSTOP(I2C1, ENABLE);//关闭I2C1总线
}


 /**
  * @brief  WriteCmd，向OLED写入命令
  * @param  I2C_Command：命令代码
  * @retval 无
  */
void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}


 /**
  * @brief  WriteData，向OLED写入数据
  * @param  I2C_Data：数据
  * @retval 无
  */
void WriteData(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}


 /**
  * @brief  OLED_Init，初始化OLED
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	delay_ms(1000); // 1s,这里的延时很重要,上电后延时，没有错误的冗余设计

	WriteCmd(0xAE); //display off
	WriteCmd(0x20); //Set Memory Addressing Mode    
	WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8); //Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}


 /**
  * @brief  OLED_SetPos，设置光标
  * @param  x,光标x位置
    *                   y，光标y位置
  * @retval 无
  */
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

 /**
  * @brief  OLED_Fill，填充整个屏幕
  * @param  fill_Data:要填充的数据
    * @retval 无
  */
void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);       //page0-page1
		WriteCmd(0x00);     //low column start address
		WriteCmd(0x10);     //high column start address
		for(n=0;n<128;n++)
		{
			WriteData(fill_Data);
		}
	}
}

 /**
  * @brief  OLED_CLS，清屏
  * @param  无
    * @retval 无
  */
void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}


 /**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
    * @retval 无
  */
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}


 /**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
    * @retval 无
  */
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}



void OLED_Show_State(unsigned char x, unsigned char y)
{
	unsigned char c = 0,i = 0,j = 0;
	
	for(c=0;c<4;c++)
	{
		OLED_SetPos(x,y);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i]);
		OLED_SetPos(x,y+1);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+24]);
		OLED_SetPos(x,y+2);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+48]);
		OLED_SetPos(x,y+3);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+72]);
		x += 24;
		j++;
	}
	c = 8;
	OLED_SetPos(96,4);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i]);
		OLED_SetPos(96,5);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+24]);
		OLED_SetPos(96,6);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+48]);
		OLED_SetPos(96,7);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+72]);
}

void OLED_Show_Cost(unsigned char x, unsigned char y)
{
	unsigned char c = 0,i = 0,j = 0;
	
	for(c=4;c<7;c++)
	{
		OLED_SetPos(x,y);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i]);
		OLED_SetPos(x,y+1);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+24]);
		OLED_SetPos(x,y+2);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+48]);
		OLED_SetPos(x,y+3);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+72]);
		x += 24;
		j++;
	}
	c = 7;
	OLED_SetPos(96,4);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i]);
	OLED_SetPos(96,5);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i+24]);
	OLED_SetPos(96,6);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i+48]);
	OLED_SetPos(96,7);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i+72]);
}

void OLED_Show_Balance(unsigned char x, unsigned char y)
{
	unsigned char c = 0,i = 0,j = 0;
	
	for(c=9;c<13;c++)
	{
		OLED_SetPos(x,y);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i]);
		OLED_SetPos(x,y+1);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+24]);
		OLED_SetPos(x,y+2);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+48]);
		OLED_SetPos(x,y+3);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+72]);
		x += 24;
		j++;
	}
	c = 7;
	OLED_SetPos(96,4);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i]);
	OLED_SetPos(96,5);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i+24]);
	OLED_SetPos(96,6);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i+48]);
	OLED_SetPos(96,7);
	for(i=0;i<24;i++)
		WriteData(F24X32_CN[c*96+i+72]);
}

void OLED_Show_Balance_noten(unsigned char x, unsigned char y)
{
	unsigned char c = 0,i = 0,j = 0;
	
	for(c=11;c<15;c++)
	{
		OLED_SetPos(x,y);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i]);
		OLED_SetPos(x,y+1);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+24]);
		OLED_SetPos(x,y+2);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+48]);
		OLED_SetPos(x,y+3);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+72]);
		x += 24;
		j++;
	}
}

void OLED_Show_signin(unsigned char x, unsigned char y)
{
	unsigned char c = 0,i = 0,j = 0;
	
	for(c=15;c<19;c++)
	{
		OLED_SetPos(x,y);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i]);
		OLED_SetPos(x,y+1);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+24]);
		OLED_SetPos(x,y+2);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+48]);
		OLED_SetPos(x,y+3);
		for(i=0;i<24;i++)
			WriteData(F24X32_CN[c*96+i+72]);
		x += 24;
		j++;
	}
}

void OLED_Show_Num(unsigned char x,unsigned char y,unsigned char str[])
{
	unsigned char c = 0,i = 0,j = 0;
	
	while(str[j] != '\0')
	{
		c = str[j] - 32;
		OLED_SetPos(x,y);
		for(i=0;i<12;i++)
			WriteData(F24X32[c*48+i]);
		OLED_SetPos(x,y+1);
		for(i=0;i<12;i++)
			WriteData(F24X32[c*48+i+12]);
		OLED_SetPos(x,y+2);
		for(i=0;i<12;i++)
			WriteData(F24X32[c*48+i+24]);
		OLED_SetPos(x,y+3);
		for(i=0;i<12;i++)
			WriteData(F24X32[c*48+i+36]);
		x += 12;
		j++;
	}
}

