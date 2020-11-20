#include "rfid_rc522.h"

/* Mifare-One卡缺省密钥：*/
uint8_t key_fault[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
             
/**
  * @brief	:RC522初始化
  * @note   :--引脚配置+模块复位+关闭天线！
	**/
void RC522_Config(void)
{
	/* SPI_InitTypeDef  SPI_InitStructure */
  GPIO_InitTypeDef GPIO_InitStructure;
  
	/*!< Configure SPI_RC522_SPI pins: CS */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE );
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  /*!< Configure SPI_RC522_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  /*!< Configure SPI_RC522_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);

  /*!< Configure SPI_RC522_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA,&GPIO_InitStructure);	
		
  /*!< Configure SPI_RC522_SPI pins: RST */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	RC522_Reset();
	RC522_AntennaOFF();
	delay_ms(50);				//保证天线关闭后不会立即被开启！至少大于1ms！
}


/*************************************************************************************** static_function */
#if 1
/**
  * @brief	:软件SPI发送并接收一个字节
  * @note   :--注意，返回最近一次接收的数据！
  * @param	:TxData, 主机发送的数据
  **/
static uint8_t RC522_SPIReadWriteByte(uint8_t TxData)
{                        
    //不需要另外开辟空间存储接收值！
    //根据数据的传送规律，TxData传送最高位、移位、接收最低位
    uint8_t i=0;   

    for(i=0;i<8;i++)
	{
        if(TxData&0x80)
		{
            RC522_MOSI_HIGH();
        }else{
            RC522_MOSI_LOW();
        }
        
        TxData <<= 1;
        
        RC522_SCK_HIGH();
        if(RC522_MISO_IN()){//捕获MISO引脚的数据
            TxData |= 0x01;
        }
        RC522_SCK_LOW();//沿传输数据！   
    }
    
    return TxData;
} 	    

/**
  * @brief	:读RC522寄存器
  * @note   :--
  * @param	:regAddr, 寄存器地址
  * @return	:uint8_t, 读出的值
  **/
static uint8_t RC522_RegRead(uint8_t regAddr)
{
	uint8_t rec=0;

	RC522_NSS_LOW();

	//地址格式：1XXXXXX0
	RC522_SPIReadWriteByte(((regAddr<<1)&0x7E) | 0x80);
	rec = RC522_SPIReadWriteByte(0x00);
	
	RC522_NSS_HIGH();
	return rec;
}

/**
  * @brief	:写RC522寄存器
  * @note   :--
  * @param	:regAddr, 寄存器地址
						 Value  , 写入的值
  **/
static void RC522_RegWrite(uint8_t regAddr, uint8_t Value)
{  
	RC522_NSS_LOW();

	//地址格式：0XXX XXX0
	RC522_SPIReadWriteByte(((regAddr<<1)&0x7E));
	RC522_SPIReadWriteByte(Value);
	
	RC522_NSS_HIGH();
	delay_ms(5);
}

/**
  * @brief	:置RC522寄存器位
  * @note   :--
  * @param	:regAddr, 寄存器地址
						 Mask, 		置位值
  **/
static void RC522_SetBitMask(uint8_t regAddr, uint8_t Mask)  
{
    int8_t tmp;
	
    tmp = RC522_RegRead(regAddr);
    RC522_RegWrite(regAddr, (tmp | Mask) );
}

/**
  * @brief	:清RC522寄存器位
  * @note   :--
  * @param	:regAddr, 寄存器地址
						 Mask,	  清位值
  **/
static void RC522_ClearBitMask(uint8_t regAddr, uint8_t Mask)  
{
    int8_t tmp;
	
    tmp = RC522_RegRead(regAddr);
    RC522_RegWrite(regAddr, (tmp & ~Mask) );
}

/**
  * @brief	:计算CRC
  * @note   :--
  * @param	:*pInData , CRC输入
						 Length   , 数据长度
						 *pOutData, CRC输出
  * @return	:void
  **/
static void RC522_CalcCRC(uint8_t *pInData, uint8_t Length, uint8_t *pOutData)
{
    uint8_t i;
		uint8_t n;
	
    RC522_ClearBitMask(DivIrqReg, 0x04);
    RC522_RegWrite(CommandReg, PCD_IDLE);
    RC522_SetBitMask(FIFOLevelReg, 0x80);
	
    for(i=0; i<Length; i++)
	{   
		RC522_RegWrite(FIFODataReg, *(pInData+i));   
	}
    RC522_RegWrite(CommandReg, PCD_CALCCRC);
    
	i = 0xFF;
    do 
    {
        n = RC522_RegRead(DivIrqReg);
        i--;
    }while((i!=0) && !(n&0x04));
	
    pOutData[0] = RC522_RegRead(CRCResultRegL);
    pOutData[1] = RC522_RegRead(CRCResultRegM);
}

/**
  * @brief	:通讯
  * @note   :--
  * @param	:Command	, 命令字
             *pInData	, 通过RC522发送到卡片的数据
             inLength   , 发送数据的字节长度
             *pOutData  , 接收到的卡片返回数据
             *pOutLength, 返回数据的位长度
  **/
static int8_t RC522_Communication(uint8_t Command, uint8_t *pInData, uint8_t inLength, uint8_t *pOutData, uint16_t  *pOutLength)
{
	int8_t status=MI_ERR;
	uint8_t irq_en=0x00;
	uint8_t wait_time=0x00;
	uint8_t last_bits;
	uint8_t n;
	uint16_t i;

	switch(Command)
	{
		case PCD_AUTHENT	: irq_en = 0x12; wait_time = 0x10; break;
		case PCD_TRANSCEIVE	: irq_en = 0x77; wait_time = 0x30; break;

		default: break;
	}

	RC522_RegWrite(ComIEnReg, irq_en|0x80);
	RC522_ClearBitMask(ComIrqReg, 0x80);
	RC522_RegWrite(CommandReg, PCD_IDLE);
	RC522_SetBitMask(FIFOLevelReg, 0x80);

	for(i=0; i<inLength; i++)
	{   
		RC522_RegWrite(FIFODataReg, pInData[i]);    
	}
	RC522_RegWrite(CommandReg, Command);


	if(Command == PCD_TRANSCEIVE)
	{    
		RC522_SetBitMask(BitFramingReg, 0x80);  
	}

	i = 25000;//根据时钟频率调整，操作M1卡最大等待时间25ms
	do 
	{
		 n = RC522_RegRead(ComIrqReg);
		 i--;
	}while((i!=0) && !(n&0x01) && !(n&wait_time));
	RC522_ClearBitMask(BitFramingReg, 0x80);
		  
	if(i != 0)
	{    
		if(!(RC522_RegRead(ErrorReg)&0x1B))
		{
			status = MI_OK;
			if(n & irq_en & 0x01)
			{   
				status = MI_NOTAGERR;   
			}

			if(Command == PCD_TRANSCEIVE)
			{
				n = RC522_RegRead(FIFOLevelReg);
				last_bits = RC522_RegRead(ControlReg) & 0x07;
				if(last_bits)
				{  
					*pOutLength = (n-1)*8 + last_bits;   
				}else{   
					*pOutLength = n*8;   
				}

				if(n == 0){
					n = 1; 
				}
				
				if(n > MAX_REC_LEN){   
					n = MAX_REC_LEN;   
				}

				for(i=0; i<n; i++){   
					pOutData[i] = RC522_RegRead(FIFODataReg);    
				}
			}
		}else{   
			status = MI_ERR;   
		}
	}

	RC522_SetBitMask(ControlReg, 0x80);
	RC522_RegWrite(CommandReg, PCD_IDLE); 
	return status;
}
#endif
/*************************************************************************************** static_function */


/**
  * @brief	:开启天线
  * @note   :--每次启动或关闭天险发射之间应至少有1ms的间隔！
  **/
void RC522_AntennaON(void)
{
	uint8_t tmp;

	tmp = RC522_RegRead(TxControlReg);
	if(!(tmp & 0x03)){
			RC522_SetBitMask(TxControlReg, 0x03);
	}
}

/**
  * @brief	:关闭天线
  **/
void RC522_AntennaOFF(void)
{
	RC522_ClearBitMask(TxControlReg, 0x03);
}

/**
  * @brief	:复位RC522
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Reset(void)
{
	RC522_RST_HIGH();
	delay_ms(10);
	RC522_RST_LOW();
	delay_ms(10);
	RC522_RST_HIGH();
	delay_ms(10);
	RC522_RegWrite(CommandReg, PCD_RESETPHASE);
	while(RC522_RegRead(CommandReg) & 0x10);
		delay_ms(200);

	//和Mifare卡通讯，CRC初始值0x6363
	RC522_RegWrite(ModeReg, 0x3D);	
	delay_ms(5);
	RC522_RegWrite(TReloadRegL, 30);           
	delay_ms(5);
	RC522_RegWrite(TReloadRegH, 0);
	delay_ms(5);
	RC522_RegWrite(TModeReg, 0x8D);
	delay_ms(5);
	RC522_RegWrite(TPrescalerReg,0x3E);
	delay_ms(5);
	RC522_RegWrite(TxAutoReg, 0x40);
	
	return MI_OK;
}

/**
  * @brief	:命令卡片进入休眠状态
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Halt(void)
{
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;

	ucComRC522Buf[0] = PICC_HALT;
	ucComRC522Buf[1] = 0;
	RC522_CalcCRC(ucComRC522Buf, 2, &ucComRC522Buf[2]);

	RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 4, ucComRC522Buf, &length);
	return MI_OK;
}

/**
  * @brief	:寻卡
  * @note   :--
  * @param	:reqCode  ,寻卡方式
                0x52   = 寻感应区内所有符合14443A标准的卡
                0x26   = 寻未进入休眠状态的卡
						 *pTagType,卡片类型代码
                0x4400 = Mifare_UltraLight
                0x0400 = Mifare_One(S50)
                0x0200 = Mifare_One(S70)
                0x0800 = Mifare_Pro(X)
                0x4403 = Mifare_DESFire
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Search(uint8_t reqCode, uint8_t *pTagType)
{
	int8_t status;  
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;
	
	RC522_ClearBitMask(Status2Reg, 0x08);//寄存器包含接收器和发送器和数据模式检测器的状态标志
	RC522_RegWrite(BitFramingReg, 0x07); //不启动数据发送
	RC522_SetBitMask(TxControlReg, 0x03);//TX1、TX2输出信号将传递经发送数据调制的13.56MHz的能量载波信号

	ucComRC522Buf[0] = reqCode;//设定寻卡方式

	//通过RC522发送reqCode命令，并接收返回数据，存到ucComRC522Buf[]中！ 
	status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 1, ucComRC522Buf, &length);//读卡，获取卡片类型
	if((status == MI_OK) && (length == 0x10))//这个为啥是0x10?因为是2个字节共16bit!
	{    
	   *(pTagType+0) = ucComRC522Buf[0];
	   *(pTagType+1) = ucComRC522Buf[1];//获取卡类型
	}else{  
	   status = MI_ERR;
	}
	return status;
}

/**
  * @brief	:防冲撞
  * @note   :--实际读出5个字节，包含1字节校验，本函数只返回通过
  * @param	:*pSerialNum, 卡片序列号，4字节
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Anticoll(uint8_t *pSerialNum)
{
	int8_t status;
	uint8_t i;
	uint8_t serial_num_check=0;
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;
	
	RC522_ClearBitMask(Status2Reg, 0x08);//寄存器包含接收器和发送器和数据模式检测器的状态标志
	RC522_RegWrite(BitFramingReg, 0x00);//不启动数据发送，接收的LSB位存放在位0，接收到的第二位放在位1，定义发送的最后一个字节的位数为8
	RC522_ClearBitMask(CollReg, 0x80);//所有接收的位在冲突后将被清除

	ucComRC522Buf[0] = PICC_ANTICOLL1;
	ucComRC522Buf[1] = 0x20;

	status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 2, ucComRC522Buf, &length);
	if(status == MI_OK)
	{
		 for(i=0; i<4; i++)
			 {   
					 *(pSerialNum+i)   = ucComRC522Buf[i];
					 serial_num_check ^= ucComRC522Buf[i];
			 }
	 
			 if(serial_num_check != ucComRC522Buf[i])//返回四个字节，最后一个字节为校验位!
	 {
		 status = MI_ERR;
	 }
	}
	
	RC522_SetBitMask(CollReg, 0x80);
	return status;
}

///**
//  * @brief	:对创建的RFID对象进行实例化
//  * @param	:_RFIDCardDeviceCtl *, 卡片类型描述集合结构体
//  **/
//void RC522_StructInit(_RFIDCardDeviceCtl *myType)
//{	
//	myType->Search			= RC522_Search;
//	myType->Anticoll		= RC522_Anticoll;
//	myType->SelectCard	= RC522_SelectCard;
//	myType->Password		= RC522_Password;
//	
//	myType->Read		= RC522_Read;
//	myType->Write		= RC522_Write;		
//}

/**
  * @brief	:选定卡片
  * @param	:*pSerialNum, 卡片序列号，4字节！
  * @return	:uint8_t, 返回卡片容量
  **/
uint8_t RC522_SelectCard(uint8_t *pSerialNum)
{
	int8_t status;
	int8_t size;
	uint8_t i;
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;
    
	ucComRC522Buf[0] = PICC_SElECTTAG;
	ucComRC522Buf[1] = 0x70;
	ucComRC522Buf[6] = 0;
	for(i=0; i<4; i++)
	{
		ucComRC522Buf[i+2] = *(pSerialNum+i);
		ucComRC522Buf[6]  ^= *(pSerialNum+i);
	}
	RC522_CalcCRC(ucComRC522Buf, 7, &ucComRC522Buf[7]);//计算CRC装填至ucComRC522Buf[7]

	RC522_ClearBitMask(Status2Reg, 0x08);//寄存器包含接收器和发送器和数据模式检测器的状态标志

	status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 9, ucComRC522Buf, &length);
	if((status == MI_OK) && (length == 0x18))
	{   
		size = ucComRC522Buf[0];
	}
	else
	{			
		size = 0;    
	}
    return size;
}

/**
  * @brief	:验证卡片密码
  * @note   :--
  * @param	:veriMode	, 密码验证模式
                 0x60 = 验证A密钥
                 0x61 = 验证B密钥 
						 blockAddr	, 块地址
						 *pKey		, 密码
						 *pSerialNum, 卡片序列号，4字节
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Password(uint8_t veriMode, uint8_t blockAddr, uint8_t *pKey, uint8_t *pSerialNum)
{
	int8_t status;
	uint8_t i;
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;

	ucComRC522Buf[0] = veriMode;
	ucComRC522Buf[1] = blockAddr;
    
	for(i=0; i<6; i++)
	{    
		ucComRC522Buf[i+2] = *(pKey+i);   
	}
    
	for(i=0; i<6; i++)
	{    
		ucComRC522Buf[i+8] = *(pSerialNum+i);   
	}
    
	status = RC522_Communication(PCD_AUTHENT, ucComRC522Buf, 12, ucComRC522Buf, &length);
	if((status != MI_OK) || (!(RC522_RegRead(Status2Reg) & 0x08))) 
	{
		status = MI_ERR; 
	}
	return status;
}

/**
  * @brief	:读取M1卡一块数据
  * @param	:blockAddr, 块地址
						 *pData   , 读出的数据，16字节
						 num      , 读出数据的字节数
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Read(uint8_t blockAddr, uint8_t *pData,uint8_t num)
{
	int8_t status;
	uint8_t i;
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;

	ucComRC522Buf[0] = PICC_READ;
	ucComRC522Buf[1] = blockAddr;
	RC522_CalcCRC(ucComRC522Buf, 2, &ucComRC522Buf[2]);
 
	status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 4, ucComRC522Buf, &length);
	if((status == MI_OK) && (length == 0x90))
	{
		for(i=0; i<num; i++) 
		{
			*(pData+i) = ucComRC522Buf[i]; 
		}			
	}
	else
	{ 
		status = MI_ERR;   
	}
	return status;
}

/**
  * @brief	:写数据到M1卡一块
  * @param	:blockAddr, 块地址
						 *pData   , 写入的数据，16字节
  * @return	:int8_t, 成功返回MI_OK
  **/
int8_t RC522_Write(uint8_t blockAddr, uint8_t *pData)
{
	int8_t status;
	uint8_t i;
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;
	
	ucComRC522Buf[0] = PICC_WRITE;
	ucComRC522Buf[1] = blockAddr;
	RC522_CalcCRC(ucComRC522Buf, 2, &ucComRC522Buf[2]);

	status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 4, ucComRC522Buf, &length);
	if((status != MI_OK) || (length != 4) || ((ucComRC522Buf[0] & 0x0F) != 0x0A)) 
	{
		status = MI_ERR;
	}		
			
	if(status == MI_OK)
	{
		for(i=0; i<16; i++) 
		{
			ucComRC522Buf[i] = *(pData+i); 
		}			
		RC522_CalcCRC(ucComRC522Buf, 16, &ucComRC522Buf[16]);

		status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 18, ucComRC522Buf, &length);
		if((status != MI_OK) || (length != 4) || ((ucComRC522Buf[0] & 0x0F) != 0x0A)) 
		{
			status = MI_ERR;   
		}
	}
	return status;
}

/************************************************************** plus */

/**
  * @brief  设置RC522的工作方式
  * @param  ucType，工作方式
  * @retval 无
  */
void M500PcdConfigISOType ( uint8_t ucType )
{
	if ( ucType == 'A')                     //ISO14443_A
  {
		RC522_ClearBitMask ( Status2Reg, 0x08 );
		
    RC522_RegWrite ( ModeReg, 0x3D );         //3F
		
		RC522_RegWrite ( RxSelReg, 0x86 );        //84
		
		RC522_RegWrite( RFCfgReg, 0x7F );         //4F
		
		RC522_RegWrite( TReloadRegL, 30 );        
		
		RC522_RegWrite ( TReloadRegH, 0 );
		
		RC522_RegWrite ( TModeReg, 0x8D );
		
		RC522_RegWrite ( TPrescalerReg, 0x3E );
		
		delay_us ( 2 );
		
		RC522_AntennaON ();//开天线
		
   }	 
}
