#ifndef    _EEPROM_HEAD_H_
#define    _EEPROM_HEAD_H_
#include   "typedef.h"
#include  "fileHighUpdate.h"
#include  "uicc_main.h"
#include  "SysConfigTarget.h"
#include  "profile.h"
/*
 *EEPROM的定义
*/


/**
断电保护区	                             |	高地址
强更新配置区                             |  一页     
强更新区域								 |
敏感数据区                               |  一页       
隔离区                                   |  一页
文件数据区								 |
隔离区（空闲区）						 |
文件头									 |   低地址
地址索引表区							 | 
系统区（配置区）						 |
代码区									 |
*/
extern PN8			    X_A								BACK_UP_AREA_START;

#define     FlashBaseAddr		   	G_CODE_SIZE

#define     E_High_Update_NUM       G_High_Update_NUM //强更新配置区页数
#define     High_Update_ConfigForFID  0x04   //强更新配置区 4字节为一组 分别是:强更新备份个数(包括原文件)/当前指向哪个备份文件/文件大小(以页为单位)/占用的大小(页为单位)


#define FlashLimit   E2PLimit
#define G_CODE_SIZE  EEPROM_BASE

#define     ICCID_OFF          0
#define     ICCID_Len 		   0x0A
#define     IMSI1_OFF          (ICCID_OFF + ICCID_Len) 
#define     IMSI_Len           9
#define     IMSI2_OFF          (IMSI1_OFF + IMSI_Len)  
#define     KI_OFF             (IMSI2_OFF + IMSI_Len)
#define     KI_Len             0x10
#define     AUTH_CFG_OFF       (KI_OFF + KI_Len)
#define     AUTH_CFG_Len       0x6F

extern UWORD xdata G_CurPrivateMFFileStart;
extern UWORD xdata G_CurMFFileStart;
extern UWORD xdata G_CurBipReceiveAddrStart;

#define PFPageSize                  EEPAGE_SIZE

#if NATIVE_CARD_UICC
#define     E_COS_ConfigData        (EEPROM_BASE + 0x600)//CARDINFO_START	     //???,????
#define     E_ConfigData_LEN		0x40

#define     E_MK_START              (E_COS_ConfigData  + E_ConfigData_LEN)	//目前用于MFKEY存放地址	(8字节MFKEY+1字节KeyLmit)
#define     E_CARD_LIFE_CYCLE	    (E_MK_START + 9 )	   // 记录卡片生命周期状态，1字节

#define     E_RESIZEPROTECT         (E_COS_ConfigData + PFPageSize)
#define     E_CREATEPROTECT         (E_RESIZEPROTECT + BLOCKNUM*sizeof(resizeprotect))
	
#define     E_APP_DUANDIAN_START    (E_RESIZEPROTECT + PFPageSize)

//#define     E_High_Update_Config	(E_APP_DUANDIAN_START + PFPageSize)
//#define     E_High_Update_START   (E_High_Update_Config + PFPageSize)

#define     E_SECURE_DATA           (E_APP_DUANDIAN_START + PFPageSize)
#define		SECURE_DATA_LEN			0x200

#define 	RFM_KEY_ADDR            (E_SECURE_DATA + SECURE_DATA_LEN)
#define		RFM_KEY_ALL_LEN			0x300

#define     E_ADDR_INDEX_TABLE      (((RFM_KEY_ADDR + RFM_KEY_ALL_LEN)+ EEPAGE_MASK)&NOR_EEPAGE_MASK) 		  //地址索引表	(size: 4 * MAX_INDEX_NUM) page start
#define     MAX_INDEX_NUM           (0x200)
#endif


	
//************以下是http相关地址定义**************************************//
#if NATIVE_CARD_UICC																								//84参数, L(1 byte)V格式
#define     HTTP_Connection_Parameters_ADDR		(((E_ADDR_INDEX_TABLE + MAX_INDEX_NUM*sizeof(UWORD))+ EEPAGE_MASK)&NOR_EEPAGE_MASK) //page start,共2K大小, bip接收数据buf page start
#else
#define     HTTP_Connection_Parameters_ADDR		G_CurBipReceiveAddrStart
#endif
#define 	HTTP_Connection_Parameters_SIZE		0x100

#define 	HTTP_Security_Parameters_ADDR		(HTTP_Connection_Parameters_ADDR+HTTP_Connection_Parameters_SIZE)		//85参数, L(1 byte)V格式
#define 	HTTP_Security_Parameters_SIZE		0x100

#define 	HTTP_Retry_Policy_Parameters_ADDR	(HTTP_Security_Parameters_ADDR+HTTP_Security_Parameters_SIZE)			//86参数, L(1 byte)V格式
#define 	HTTP_Retry_Policy_Parameters_SIZE	0x100

#define		HTTP_POST_Parameters_ADDR			(HTTP_Retry_Policy_Parameters_ADDR+HTTP_Retry_Policy_Parameters_SIZE)	//89参数, L(1 byte)V格式
#define		HTTP_POST_Parameters_SIZE			0x100	

#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
#define 	BIPModule_Receive_ADDR_START		(G_BIPModule_Session_Data_Buf)
#else
#define 	BIPModule_Receive_ADDR_START		(HTTP_POST_Parameters_ADDR+HTTP_POST_Parameters_SIZE)
#endif

#define 	BIPModule_Receive_SIZE           	0x800  

#define     BIPModule_Send_ADDR_START           (BIPModule_Receive_ADDR_START + BIPModule_Receive_SIZE)			//共2K大小, bip发送数据buf
#define  	BIPModule_Send_SIZE               	0x800 

#define 	HTTP_CommandData_ADDR_START			(BIPModule_Send_ADDR_START + BIPModule_Send_SIZE)				//共0x800字节, APDU明文数据存放buf
#define 	HTTP_CommandData_SIZE            	0x800 

#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
#define 	HTTP_ResponseData_ADDR				(HTTP_CommandData_ADDR_START+HTTP_CommandData_SIZE)				
#define		HTTPSResponseData_SIZE				0x410

#define 	HTTP_NextURIData_ADDR				(HTTP_ResponseData_ADDR+HTTPSResponseData_SIZE)				
#define 	HTTPNextURIPara_SIZE            	0x100 

#define 	BIPModule_ADDR_END					(HTTP_POST_Parameters_ADDR+HTTP_POST_Parameters_SIZE)
#else


#define 	BIPModule_ADDR_END					(HTTP_CommandData_ADDR_START+HTTP_CommandData_SIZE)
#endif

#if NATIVE_CARD_UICC
#define    	E_FILE_START      	   ((BIPModule_ADDR_END+ EEPAGE_MASK)&NOR_EEPAGE_MASK)   //page start

#define     EE_NativeSize									0x30000							//Native通信卡中,数据区只使用EE_NativeSize定义的大小 @yangjian 2022.12.6	
#define     E_FileBodyStart           (EEPROM_BASE+EE_NativeSize)							//文件数据区的起始 

//#define     E_FileBodyStart         ((UWORD)(BACK_UP_AREA_START -  PFPageSize))	  //文件数据区的起始  预留一页隔离区
#else	
#define     ADDR_PRIVATE_FILE_START   G_CurPrivateMFFileStart
#define    	E_FILE_START      	      G_CurMFFileStart// (E_ADDR_INDEX_TABLE + MAX_INDEX_NUM*sizeof(UWORD))	  //文件头起始区
#endif
#endif 
