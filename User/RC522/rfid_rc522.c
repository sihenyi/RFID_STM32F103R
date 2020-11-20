#include "rfid_rc522.h"

/* Mifare-One��ȱʡ��Կ��*/
uint8_t key_fault[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
             
/**
  * @brief	:RC522��ʼ��
  * @note   :--��������+ģ�鸴λ+�ر����ߣ�
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
	delay_ms(50);				//��֤���߹رպ󲻻����������������ٴ���1ms��
}


/*************************************************************************************** static_function */
#if 1
/**
  * @brief	:���SPI���Ͳ�����һ���ֽ�
  * @note   :--ע�⣬�������һ�ν��յ����ݣ�
  * @param	:TxData, �������͵�����
  **/
static uint8_t RC522_SPIReadWriteByte(uint8_t TxData)
{                        
    //����Ҫ���⿪�ٿռ�洢����ֵ��
    //�������ݵĴ��͹��ɣ�TxData�������λ����λ���������λ
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
        if(RC522_MISO_IN()){//����MISO���ŵ�����
            TxData |= 0x01;
        }
        RC522_SCK_LOW();//�ش������ݣ�   
    }
    
    return TxData;
} 	    

/**
  * @brief	:��RC522�Ĵ���
  * @note   :--
  * @param	:regAddr, �Ĵ�����ַ
  * @return	:uint8_t, ������ֵ
  **/
static uint8_t RC522_RegRead(uint8_t regAddr)
{
	uint8_t rec=0;

	RC522_NSS_LOW();

	//��ַ��ʽ��1XXXXXX0
	RC522_SPIReadWriteByte(((regAddr<<1)&0x7E) | 0x80);
	rec = RC522_SPIReadWriteByte(0x00);
	
	RC522_NSS_HIGH();
	return rec;
}

/**
  * @brief	:дRC522�Ĵ���
  * @note   :--
  * @param	:regAddr, �Ĵ�����ַ
						 Value  , д���ֵ
  **/
static void RC522_RegWrite(uint8_t regAddr, uint8_t Value)
{  
	RC522_NSS_LOW();

	//��ַ��ʽ��0XXX XXX0
	RC522_SPIReadWriteByte(((regAddr<<1)&0x7E));
	RC522_SPIReadWriteByte(Value);
	
	RC522_NSS_HIGH();
	delay_ms(5);
}

/**
  * @brief	:��RC522�Ĵ���λ
  * @note   :--
  * @param	:regAddr, �Ĵ�����ַ
						 Mask, 		��λֵ
  **/
static void RC522_SetBitMask(uint8_t regAddr, uint8_t Mask)  
{
    int8_t tmp;
	
    tmp = RC522_RegRead(regAddr);
    RC522_RegWrite(regAddr, (tmp | Mask) );
}

/**
  * @brief	:��RC522�Ĵ���λ
  * @note   :--
  * @param	:regAddr, �Ĵ�����ַ
						 Mask,	  ��λֵ
  **/
static void RC522_ClearBitMask(uint8_t regAddr, uint8_t Mask)  
{
    int8_t tmp;
	
    tmp = RC522_RegRead(regAddr);
    RC522_RegWrite(regAddr, (tmp & ~Mask) );
}

/**
  * @brief	:����CRC
  * @note   :--
  * @param	:*pInData , CRC����
						 Length   , ���ݳ���
						 *pOutData, CRC���
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
  * @brief	:ͨѶ
  * @note   :--
  * @param	:Command	, ������
             *pInData	, ͨ��RC522���͵���Ƭ������
             inLength   , �������ݵ��ֽڳ���
             *pOutData  , ���յ��Ŀ�Ƭ��������
             *pOutLength, �������ݵ�λ����
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

	i = 25000;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
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
  * @brief	:��������
  * @note   :--ÿ��������ر����շ���֮��Ӧ������1ms�ļ����
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
  * @brief	:�ر�����
  **/
void RC522_AntennaOFF(void)
{
	RC522_ClearBitMask(TxControlReg, 0x03);
}

/**
  * @brief	:��λRC522
  * @return	:int8_t, �ɹ�����MI_OK
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

	//��Mifare��ͨѶ��CRC��ʼֵ0x6363
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
  * @brief	:���Ƭ��������״̬
  * @return	:int8_t, �ɹ�����MI_OK
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
  * @brief	:Ѱ��
  * @note   :--
  * @param	:reqCode  ,Ѱ����ʽ
                0x52   = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
                0x26   = Ѱδ��������״̬�Ŀ�
						 *pTagType,��Ƭ���ʹ���
                0x4400 = Mifare_UltraLight
                0x0400 = Mifare_One(S50)
                0x0200 = Mifare_One(S70)
                0x0800 = Mifare_Pro(X)
                0x4403 = Mifare_DESFire
  * @return	:int8_t, �ɹ�����MI_OK
  **/
int8_t RC522_Search(uint8_t reqCode, uint8_t *pTagType)
{
	int8_t status;  
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;
	
	RC522_ClearBitMask(Status2Reg, 0x08);//�Ĵ��������������ͷ�����������ģʽ�������״̬��־
	RC522_RegWrite(BitFramingReg, 0x07); //���������ݷ���
	RC522_SetBitMask(TxControlReg, 0x03);//TX1��TX2����źŽ����ݾ��������ݵ��Ƶ�13.56MHz�������ز��ź�

	ucComRC522Buf[0] = reqCode;//�趨Ѱ����ʽ

	//ͨ��RC522����reqCode��������շ������ݣ��浽ucComRC522Buf[]�У� 
	status = RC522_Communication(PCD_TRANSCEIVE, ucComRC522Buf, 1, ucComRC522Buf, &length);//��������ȡ��Ƭ����
	if((status == MI_OK) && (length == 0x10))//���Ϊɶ��0x10?��Ϊ��2���ֽڹ�16bit!
	{    
	   *(pTagType+0) = ucComRC522Buf[0];
	   *(pTagType+1) = ucComRC522Buf[1];//��ȡ������
	}else{  
	   status = MI_ERR;
	}
	return status;
}

/**
  * @brief	:����ײ
  * @note   :--ʵ�ʶ���5���ֽڣ�����1�ֽ�У�飬������ֻ����ͨ��
  * @param	:*pSerialNum, ��Ƭ���кţ�4�ֽ�
  * @return	:int8_t, �ɹ�����MI_OK
  **/
int8_t RC522_Anticoll(uint8_t *pSerialNum)
{
	int8_t status;
	uint8_t i;
	uint8_t serial_num_check=0;
	uint8_t ucComRC522Buf[MAX_REC_LEN]; 
	uint16_t length;
	
	RC522_ClearBitMask(Status2Reg, 0x08);//�Ĵ��������������ͷ�����������ģʽ�������״̬��־
	RC522_RegWrite(BitFramingReg, 0x00);//���������ݷ��ͣ����յ�LSBλ�����λ0�����յ��ĵڶ�λ����λ1�����巢�͵����һ���ֽڵ�λ��Ϊ8
	RC522_ClearBitMask(CollReg, 0x80);//���н��յ�λ�ڳ�ͻ�󽫱����

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
	 
			 if(serial_num_check != ucComRC522Buf[i])//�����ĸ��ֽڣ����һ���ֽ�ΪУ��λ!
	 {
		 status = MI_ERR;
	 }
	}
	
	RC522_SetBitMask(CollReg, 0x80);
	return status;
}

///**
//  * @brief	:�Դ�����RFID�������ʵ����
//  * @param	:_RFIDCardDeviceCtl *, ��Ƭ�����������Ͻṹ��
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
  * @brief	:ѡ����Ƭ
  * @param	:*pSerialNum, ��Ƭ���кţ�4�ֽڣ�
  * @return	:uint8_t, ���ؿ�Ƭ����
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
	RC522_CalcCRC(ucComRC522Buf, 7, &ucComRC522Buf[7]);//����CRCװ����ucComRC522Buf[7]

	RC522_ClearBitMask(Status2Reg, 0x08);//�Ĵ��������������ͷ�����������ģʽ�������״̬��־

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
  * @brief	:��֤��Ƭ����
  * @note   :--
  * @param	:veriMode	, ������֤ģʽ
                 0x60 = ��֤A��Կ
                 0x61 = ��֤B��Կ 
						 blockAddr	, ���ַ
						 *pKey		, ����
						 *pSerialNum, ��Ƭ���кţ�4�ֽ�
  * @return	:int8_t, �ɹ�����MI_OK
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
  * @brief	:��ȡM1��һ������
  * @param	:blockAddr, ���ַ
						 *pData   , ���������ݣ�16�ֽ�
						 num      , �������ݵ��ֽ���
  * @return	:int8_t, �ɹ�����MI_OK
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
  * @brief	:д���ݵ�M1��һ��
  * @param	:blockAddr, ���ַ
						 *pData   , д������ݣ�16�ֽ�
  * @return	:int8_t, �ɹ�����MI_OK
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
  * @brief  ����RC522�Ĺ�����ʽ
  * @param  ucType��������ʽ
  * @retval ��
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
		
		RC522_AntennaON ();//������
		
   }	 
}
