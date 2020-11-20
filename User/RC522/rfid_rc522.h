/**
  ******************************************************************************
  * 
  * RFID-RC522��Ƶģ������(stm32f407ϵ�п��ã������ݲ�֧�֣�)
  *	ʹ��ʾ����
  *		���ù��̣�Ѱ�� -> ����ײ(��ȡ���к�) -> ѡ�� -> ��֤��Կ -> ��/д����
  *		dev_rc522.Search(0x52, (uint8_t *)dev_rc522.TagType);		//Ѱ��
  *		dev_rc522.Anticoll(dev_rc522.SerialNum);					//����ײ����ȡ�����к�(N>=0)
  *
  *		dev_rc522.SelectCard(dev_rc522.SerialNum);					//ѡ��
  *		dev_rc522.Password(0x60, 20, keyA, dev_rc522.SerialNum);	//��֤��20��Կ
  *		dev_rc522.Write(20, data_for_write);						//д����-��20
  *		dev_rc522.Read(20, dev_rc522.Data);							//������-��20
  * 
  ******************************************************************************
***/

#ifndef __RFID_RC522_H__
#define __RFID_RC522_H__

#include "stm32f10x.h"
#include "bsp_systick.h"
//#include "malloc_ext.h"

extern uint8_t key_fault[6];//Mifare-One��ȱʡ��Կ

typedef struct
{
	uint8_t  id;
	uint8_t  sta;
	uint8_t  time_in_d;
	uint8_t  time_in_h;
	uint8_t  time_in_min;
	uint8_t  balance;
}RF_CARD_STRUCT;


/* ȫ�ֿ����趨����Ϊģ��ʹ��ʱ����ע�⣡ */
#define MAX_REC_LEN					18
#define RC522_DELAY()				delay_us(3)
//MF522 FIFO���ȶ���
#define DEF_FIFO_LENGTH       		64		//FIFO size=64byte

/*rfid_rc522.h Card-Information Define*/
//<!- S50�ǽӴ�ʽIC�����ܼ��(M1)��
//<!- ����Ϊ8KλEEPROM(1K�ֽ�)
//<!- ��Ϊ16��������ÿ������Ϊ4�飬ÿ��16���ֽ�,�Կ�Ϊ��ȡ��λ
//<!- ÿ�������ж�����һ�����뼰���ʿ���
//<!- ÿ�ſ���Ψһ���кţ�Ϊ32λ
//<!- ���з���ͻ���ƣ�֧�ֶ࿨����
//<!- �޵�Դ���Դ����ߣ��ں����ܿ����߼���ͨѶ�߼���·
//<!- ���ݱ�����Ϊ10�꣬�ɸ�д10��Σ������޴�
//<!- �����¶ȣ�-20��~50��(ʪ��Ϊ90%)
//<!- ����Ƶ�ʣ�13.56MHZ
//<!- ͨ�����ʣ�106KBPS
//<!- ��д���룺10cm����(���д���й�)

/*rfid_rc522.h Easy-Command Define*/
#define GetCardType(pCardNum)			(uint16_t)(pCardNum[0]<<8 | pCardNum[1])
#define GetCardID(pIDNum)					(uint32_t)(pIDNum[0]<<24 | pIDNum[1]<<16 | pIDNum[2]<<8 | pIDNum[3])

#define New(objectType)						((objectType *)mymalloc(sizeof(objectType)))
#define Delete(object)						myfree(object)

#define	KeyCopy(dest, src)				memcpy(dest, src, 8)

//SPI���߿���
#define RC522_NSS_HIGH()			GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define RC522_NSS_LOW()				GPIO_ResetBits(GPIOA,GPIO_Pin_4)

#define RC522_SCK_HIGH()			GPIO_SetBits(GPIOA,GPIO_Pin_5)
#define RC522_SCK_LOW()				GPIO_ResetBits(GPIOA,GPIO_Pin_5)

#define RC522_MOSI_HIGH()			GPIO_SetBits(GPIOA,GPIO_Pin_7)
#define RC522_MOSI_LOW()			GPIO_ResetBits(GPIOA,GPIO_Pin_7)

#define RC522_RST_HIGH()			GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define RC522_RST_LOW()				GPIO_ResetBits(GPIOB,GPIO_Pin_0)

#define RC522_MISO_IN()				GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)

/************************************************************* RFID-RC522 ������ */
//MF522������
#define PCD_IDLE              		0x00	//ȡ����ǰ����
#define PCD_AUTHENT           		0x0E	//��֤��Կ
#define PCD_RECEIVE           		0x08	//��������
#define PCD_TRANSMIT          		0x04	//��������
#define PCD_TRANSCEIVE        		0x0C	//���Ͳ���������
#define PCD_RESETPHASE        		0x0F	//��λ
#define PCD_CALCCRC           		0x03	//CRC����
/************************************************************* RFID-RC522 ������ */

/************************************************************* Mifare_One��Ƭ������ */
#define PICC_REQIDL           		0x26	//Ѱ��������δ��������״̬
#define PICC_REQALL           		0x52	//Ѱ��������ȫ����
#define PICC_SElECTTAG						0x93	//ѡ��
#define PICC_ANTICOLL1        		0x93	//����ײ1
#define PICC_ANTICOLL2        		0x95	//����ײ2
#define PICC_AUTHENT1A        		0x60	//��֤A��Կ
#define PICC_AUTHENT1B        		0x61	//��֤B��Կ
#define PICC_READ             		0x30	//����
#define PICC_WRITE            		0xA0	//д��
#define PICC_DECREMENT        		0xC0	//�ۿ�
#define PICC_INCREMENT        		0xC1	//��ֵ
#define PICC_RESTORE         	 		0xC2	//�������ݵ�������
#define PICC_TRANSFER         		0xB0	//���滺����������
#define PICC_HALT             		0x50	//����
/************************************************************* Mifare_One��Ƭ������ */

/************************************************************* RFID-RC522 �Ĵ��� */
//PAGE 0 <---------> Command and Status
#define     RFU00                 0x00	//    
#define     CommandReg            0x01	//    
#define     ComIEnReg             0x02	//    
#define     DivlEnReg             0x03	//    
#define     ComIrqReg             0x04	//
#define     DivIrqReg             0x05	//
#define     ErrorReg              0x06	//    
#define     Status1Reg            0x07	//  
#define     Status2Reg            0x08	//    
#define     FIFODataReg           0x09	//
#define     FIFOLevelReg          0x0A	//
#define     WaterLevelReg         0x0B	//
#define     ControlReg            0x0C	//
#define     BitFramingReg         0x0D	//
#define     CollReg               0x0E	//
#define     RFU0F                 0x0F	//

//PAGE 1 <---------> Command       
#define     RFU10                 0x10	//
#define     ModeReg               0x11	//
#define     TxModeReg             0x12	//
#define     RxModeReg             0x13	//
#define     TxControlReg          0x14	//
#define     TxAutoReg             0x15	//
#define     TxSelReg              0x16	//
#define     RxSelReg              0x17	//
#define     RxThresholdReg        0x18	//
#define     DemodReg              0x19	//
#define     RFU1A                 0x1A	//
#define     RFU1B                 0x1B	//
#define     MifareReg             0x1C	//
#define     RFU1D                 0x1D	//
#define     RFU1E                 0x1E	//
#define     SerialSpeedReg        0x1F	//

//PAGE 2 <---------> CFG    
#define     RFU20                 0x20	//  
#define     CRCResultRegM         0x21	//
#define     CRCResultRegL         0x22	//
#define     RFU23                 0x23	//
#define     ModWidthReg           0x24	//
#define     RFU25                 0x25	//
#define     RFCfgReg              0x26	//
#define     GsNReg                0x27	//
#define     CWGsCfgReg            0x28	//
#define     ModGsCfgReg           0x29	//
#define     TModeReg              0x2A	//
#define     TPrescalerReg         0x2B	//
#define     TReloadRegH           0x2C	//
#define     TReloadRegL           0x2D	//
#define     TCounterValueRegH     0x2E	//
#define     TCounterValueRegL     0x2F	//

//PAGE 3 <---------> TestRegister     
#define     RFU30                 0x30	//
#define     TestSel1Reg           0x31	//
#define     TestSel2Reg           0x32	//
#define     TestPinEnReg          0x33	//
#define     TestPinValueReg       0x34	//
#define     TestBusReg            0x35	//
#define     AutoTestReg           0x36	//
#define     VersionReg            0x37	//
#define     AnalogTestReg         0x38	//
#define     TestDAC1Reg           0x39	//  
#define     TestDAC2Reg           0x3A	//
#define     TestADCReg            0x3B	//   
#define     RFU3C                 0x3C	//   
#define     RFU3D                 0x3D	//   
#define     RFU3E                 0x3E	//   
#define     RFU3F		  						0x3F	//
/************************************************************* RFID-RC522 �Ĵ��� */


/*rfid_rc522.h MF522-Error Code Define*/
#define MI_OK								(0)
#define MI_NOTAGERR					(-1)
#define MI_ERR							(-2)

/*rfid_rc522.h Param Class Define*/
///��Ƭ�����������ϣ�
typedef struct
{
	uint32_t	UID;						//��Ƭ���к�		
	uint16_t	TagType;				//��Ƭ���ʹ���
	
	uint8_t		Size;						//��Ƭ����

	uint8_t		SerialNum[4];		//��Ƭ���к�����
	uint8_t		Data[16];				//��Ƭ����(ֻ����һ������ݣ���16�ֽڣ�)
	
	uint8_t		KeyA[8];				//��ƬA��Կ
	uint8_t		KeyB[8];				//��ƬB��Կ
	
	//�����ǲο���������̹����ĺ����б����о���Ҫ�ģ�
	int8_t		(*Search)(uint8_t, uint8_t*);
	int8_t		(*Anticoll)(uint8_t*);
	uint8_t		(*SelectCard)(uint8_t*);   
	int8_t		(*Password)(uint8_t, uint8_t, uint8_t*, uint8_t*);

	int8_t		(*Read)(uint8_t, uint8_t*);
	int8_t		(*Write)(uint8_t, uint8_t*);
}_RFIDCardDeviceCtl;


void RC522_Config(void);				//RC522�������ų�ʼ��

void RC522_AntennaON(void);			//��������
void RC522_AntennaOFF(void);		//�ر�����
int8_t RC522_Reset(void);				//��λRC522
int8_t RC522_Halt(void);				//���Ƭ��������״̬


//�����ǲο���������̹����ĺ����б�
void RC522_StructInit(_RFIDCardDeviceCtl *myType);					//ʵ����RFID����

int8_t RC522_Search(uint8_t reqCode, uint8_t *pTagType);		//Ѱ��
int8_t RC522_Anticoll(uint8_t *pSerialNum);									//����ײ
uint8_t RC522_SelectCard(uint8_t *pSerialNum);							//ѡ����Ƭ   
int8_t RC522_Password(uint8_t veriMode, uint8_t blockAddr, uint8_t *pKey, uint8_t *pSerialNum);//��֤��Ƭ����
int8_t RC522_Read(uint8_t blockAddr, uint8_t *pData, uint8_t num);				//��ȡM1��һ������
int8_t RC522_Write(uint8_t blockAddr, uint8_t *pData);			//д���ݵ�M1��һ��

/*************************************************** plus */
void M500PcdConfigISOType ( uint8_t ucType );

#endif /* __RFID_RC522_H__ */


