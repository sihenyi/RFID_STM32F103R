/**
  ******************************************************************************
  * 
  * RFID-RC522射频模块驱动(stm32f407系列可用，其他暂不支持！)
  *	使用示例：
  *		调用过程：寻卡 -> 防冲撞(获取序列号) -> 选卡 -> 验证密钥 -> 读/写操作
  *		dev_rc522.Search(0x52, (uint8_t *)dev_rc522.TagType);		//寻卡
  *		dev_rc522.Anticoll(dev_rc522.SerialNum);					//防冲撞，获取到序列号(N>=0)
  *
  *		dev_rc522.SelectCard(dev_rc522.SerialNum);					//选卡
  *		dev_rc522.Password(0x60, 20, keyA, dev_rc522.SerialNum);	//验证块20密钥
  *		dev_rc522.Write(20, data_for_write);						//写扇区-块20
  *		dev_rc522.Read(20, dev_rc522.Data);							//读扇区-块20
  * 
  ******************************************************************************
***/

#ifndef __RFID_RC522_H__
#define __RFID_RC522_H__

#include "stm32f10x.h"
#include "bsp_systick.h"
//#include "malloc_ext.h"

extern uint8_t key_fault[6];//Mifare-One卡缺省密钥

typedef struct
{
	uint8_t  id;
	uint8_t  sta;
	uint8_t  time_in_d;
	uint8_t  time_in_h;
	uint8_t  time_in_min;
	uint8_t  balance;
}RF_CARD_STRUCT;


/* 全局控制设定，作为模块使用时必须注意！ */
#define MAX_REC_LEN					18
#define RC522_DELAY()				delay_us(3)
//MF522 FIFO长度定义
#define DEF_FIFO_LENGTH       		64		//FIFO size=64byte

/*rfid_rc522.h Card-Information Define*/
//<!- S50非接触式IC卡性能简介(M1)：
//<!- 容量为8K位EEPROM(1K字节)
//<!- 分为16个扇区，每个扇区为4块，每块16个字节,以块为存取单位
//<!- 每个扇区有独立的一组密码及访问控制
//<!- 每张卡有唯一序列号，为32位
//<!- 具有防冲突机制，支持多卡操作
//<!- 无电源，自带天线，内含加密控制逻辑和通讯逻辑电路
//<!- 数据保存期为10年，可改写10万次，读无限次
//<!- 工作温度：-20℃~50℃(湿度为90%)
//<!- 工作频率：13.56MHZ
//<!- 通信速率：106KBPS
//<!- 读写距离：10cm以内(与读写器有关)

/*rfid_rc522.h Easy-Command Define*/
#define GetCardType(pCardNum)			(uint16_t)(pCardNum[0]<<8 | pCardNum[1])
#define GetCardID(pIDNum)					(uint32_t)(pIDNum[0]<<24 | pIDNum[1]<<16 | pIDNum[2]<<8 | pIDNum[3])

#define New(objectType)						((objectType *)mymalloc(sizeof(objectType)))
#define Delete(object)						myfree(object)

#define	KeyCopy(dest, src)				memcpy(dest, src, 8)

//SPI总线控制
#define RC522_NSS_HIGH()			GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define RC522_NSS_LOW()				GPIO_ResetBits(GPIOA,GPIO_Pin_4)

#define RC522_SCK_HIGH()			GPIO_SetBits(GPIOA,GPIO_Pin_5)
#define RC522_SCK_LOW()				GPIO_ResetBits(GPIOA,GPIO_Pin_5)

#define RC522_MOSI_HIGH()			GPIO_SetBits(GPIOA,GPIO_Pin_7)
#define RC522_MOSI_LOW()			GPIO_ResetBits(GPIOA,GPIO_Pin_7)

#define RC522_RST_HIGH()			GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define RC522_RST_LOW()				GPIO_ResetBits(GPIOB,GPIO_Pin_0)

#define RC522_MISO_IN()				GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)

/************************************************************* RFID-RC522 命令字 */
//MF522命令字
#define PCD_IDLE              		0x00	//取消当前命令
#define PCD_AUTHENT           		0x0E	//验证密钥
#define PCD_RECEIVE           		0x08	//接收数据
#define PCD_TRANSMIT          		0x04	//发送数据
#define PCD_TRANSCEIVE        		0x0C	//发送并接收数据
#define PCD_RESETPHASE        		0x0F	//复位
#define PCD_CALCCRC           		0x03	//CRC计算
/************************************************************* RFID-RC522 命令字 */

/************************************************************* Mifare_One卡片命令字 */
#define PICC_REQIDL           		0x26	//寻天线区内未进入休眠状态
#define PICC_REQALL           		0x52	//寻天线区内全部卡
#define PICC_SElECTTAG						0x93	//选卡
#define PICC_ANTICOLL1        		0x93	//防冲撞1
#define PICC_ANTICOLL2        		0x95	//防冲撞2
#define PICC_AUTHENT1A        		0x60	//验证A密钥
#define PICC_AUTHENT1B        		0x61	//验证B密钥
#define PICC_READ             		0x30	//读块
#define PICC_WRITE            		0xA0	//写块
#define PICC_DECREMENT        		0xC0	//扣款
#define PICC_INCREMENT        		0xC1	//充值
#define PICC_RESTORE         	 		0xC2	//调块数据到缓冲区
#define PICC_TRANSFER         		0xB0	//保存缓冲区中数据
#define PICC_HALT             		0x50	//休眠
/************************************************************* Mifare_One卡片命令字 */

/************************************************************* RFID-RC522 寄存器 */
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
/************************************************************* RFID-RC522 寄存器 */


/*rfid_rc522.h MF522-Error Code Define*/
#define MI_OK								(0)
#define MI_NOTAGERR					(-1)
#define MI_ERR							(-2)

/*rfid_rc522.h Param Class Define*/
///卡片类型描述集合：
typedef struct
{
	uint32_t	UID;						//卡片序列号		
	uint16_t	TagType;				//卡片类型代码
	
	uint8_t		Size;						//卡片容量

	uint8_t		SerialNum[4];		//卡片序列号数组
	uint8_t		Data[16];				//卡片数据(只保留一块的数据，即16字节！)
	
	uint8_t		KeyA[8];				//卡片A秘钥
	uint8_t		KeyB[8];				//卡片B秘钥
	
	//以下是参考面向对象编程构建的函数列表！仅列举需要的！
	int8_t		(*Search)(uint8_t, uint8_t*);
	int8_t		(*Anticoll)(uint8_t*);
	uint8_t		(*SelectCard)(uint8_t*);   
	int8_t		(*Password)(uint8_t, uint8_t, uint8_t*, uint8_t*);

	int8_t		(*Read)(uint8_t, uint8_t*);
	int8_t		(*Write)(uint8_t, uint8_t*);
}_RFIDCardDeviceCtl;


void RC522_Config(void);				//RC522控制引脚初始化

void RC522_AntennaON(void);			//开启天线
void RC522_AntennaOFF(void);		//关闭天线
int8_t RC522_Reset(void);				//复位RC522
int8_t RC522_Halt(void);				//命令卡片进入休眠状态


//以下是参考面向对象编程构建的函数列表
void RC522_StructInit(_RFIDCardDeviceCtl *myType);					//实例化RFID对象

int8_t RC522_Search(uint8_t reqCode, uint8_t *pTagType);		//寻卡
int8_t RC522_Anticoll(uint8_t *pSerialNum);									//防冲撞
uint8_t RC522_SelectCard(uint8_t *pSerialNum);							//选定卡片   
int8_t RC522_Password(uint8_t veriMode, uint8_t blockAddr, uint8_t *pKey, uint8_t *pSerialNum);//验证卡片密码
int8_t RC522_Read(uint8_t blockAddr, uint8_t *pData, uint8_t num);				//读取M1卡一块数据
int8_t RC522_Write(uint8_t blockAddr, uint8_t *pData);			//写数据到M1卡一块

/*************************************************** plus */
void M500PcdConfigISOType ( uint8_t ucType );

#endif /* __RFID_RC522_H__ */


