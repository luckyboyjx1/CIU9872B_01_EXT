#ifndef  __KEY_MANAGER_OLD_H__
#define  __KEY_MANAGER_OLD_H__
#include "profile.h"
#include "typejava.h"
#include "KeyLogicManager.h"
#include "KeyManager.h"

/*****************************************************************************
 * File:  KeyManager.h
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,11,14	.created
 Original author: wang hui
 *****************************************************************************/
#define DEFAULT_KEY_INDEX            	0

//#define MAX_KEYS                        3
//移动要求一个版本号支持4组key
#define MAX_KEYS_1                        4
#define MAX_DES_KEY_LENGTH             16	// TODO: for OTA
#define DES_KEY_LENGTH_8                8
#define DES_KEY_LENGTH_24              24
#define	KEY_CHECK_LENGTH_16				0x10
#define KEY_ASSISTER_BUFFER_SIZE        4   //be same as MAX_KEYS, because this buffer storing key id
#define KEY_ASSISTER_BUFFER_INDEX_MASK  4

#define KEY_LOAD_COMPLETE             	7
//#define MAX_SD_NUM                      6
#define MAX_SD_NUM                      11
#define MAX_KEYSET_NUM                 20
#define OP_DES_KEY_RELATED_DATA    0x7F01

#define MAX_AUTHENTICATION_TIME    0x7FFF  //32767
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
#define MAX_AUTHENTICATION_NEAR    0x7FFC  //32764
#else
#define MAX_AUTHENTICATION_NEAR    0x7FFB  //32763
#endif
#define MAX_AUTHENTICATION_WARNING 0x6284
#define MAX_AUTHENTICATION_ERROR   0x6A8C

#define IS_RSAKEY_PRESENT		   0xFE
#define IS_RSAKEY_PRESENT_MASK	   0x01

#define ISD_ID                        0

 #define EXTENDED_FROMAT  0xFF
 #define KEY_FROMAT2	     0x02
 #define KEY_FROMAT1	     0x01

#if GP__AMENDMENT__D
#define SCP03_MAX_AES_KEYSET			 3
#define SCP03_MAX_AES_KEY_LENGTH		 0x20

#define SCP03_MAX_AUTHENTICATION_TIME    0x7FFFFF  //8388607
 
#define SCP03_SEQUENCE_COUNTER           (memref)0x00
#define SCP03_KEYSET_VERSION             (memref)(SCP03_SEQUENCE_COUNTER + 3*sizeof(u8))
#define SCP03_SDID                       (memref)(SCP03_KEYSET_VERSION + sizeof(u8))
#define SCP03_KEYDATA                    (memref)(SCP03_SDID + sizeof(u8))
#define SCP03_KEY_TYPE_AND_COUNTER       (memref)(SCP03_KEYDATA + (MAX_KEYS * MAX_AES_KEY_LENGTH))

#define SCP03_ENC_KEY_LENGTH             (memref)(SCP03_KEY_TYPE_AND_COUNTER + KEY_ASSISTER_BUFFER_SIZE)
#define SCP03_MAC_KEY_LENGTH			 (memref)(SCP03_ENC_KEY_LENGTH + sizeof(u8))
#define SCP03_DEK_KEY_LENGTH			 (memref)(SCP03_MAC_KEY_LENGTH + sizeof(u8))	
#define SCP03_KEY_NUM                    (memref)(SCP03_DEK_KEY_LENGTH + sizeof(u8))
#define SCP03_KEY_FORMAT			(memref)(SCP03_KEY_NUM+sizeof(u8))
#define SCP03_KEY_UASAGE                 (memref)(SCP03_KEY_FORMAT + sizeof(u8))
#define SCP03_KEY_ACCESS                 (memref)(SCP03_KEY_UASAGE + KEY_ASSISTER_BUFFER_SIZE)
           					
typedef struct// AES_keySet
{
	u8 sequenceCounter[3]; //only used in scp 03
	
	u8 bKeySetVersion;   //key version 00 ~ 0x6f
	
	u8 sdId;             //key load flag
	
	u8 keyData[SCP03_MAX_AES_KEYSET * SCP03_MAX_AES_KEY_LENGTH]; //key data
	
	u8 keyTypeAndCounter[KEY_ASSISTER_BUFFER_SIZE]; //just store the key identifier
	u8 encKeyLength; //just store the key length
	u8 macKeyLength;
	u8 dekKeyLength;
	u8 keyNum;

    u8 bKeyFormat;
	u8 bKeyUseageQuailer[KEY_ASSISTER_BUFFER_SIZE];
	u8 bKeyAccess[KEY_ASSISTER_BUFFER_SIZE];

}AesKeySet;
#endif

//these used by when access struct KeySet
#define SEQUENCE_COUNTER           (memref)0x00
#define KEYSET_VERSION             (memref)(SEQUENCE_COUNTER + sizeof(u16))
#define SDID                       (memref)(KEYSET_VERSION + sizeof(u8))
#define KEYDATA                    (memref)(SDID + sizeof(u8))
#define KEY_TYPE_AND_COUNTER       (memref)(KEYDATA + (MAX_KEYS * MAX_DES_KEY_LENGTH))

#define KEY_LENGTH                 (memref)(KEY_TYPE_AND_COUNTER + KEY_ASSISTER_BUFFER_SIZE)
#define KEY_NUM                    (memref)(KEY_LENGTH + sizeof(u8))

#define KEY_FORMAT 		        	(memref)(KEY_NUM + sizeof(u8))
#define KEY_UASAGE                (memref)(KEY_FORMAT + sizeof(u8))
#define KEY_ACCESS                  (memref)(KEY_UASAGE + KEY_ASSISTER_BUFFER_SIZE)
#define OTA_COUNTER                 (memref)(KEY_ACCESS + KEY_ASSISTER_BUFFER_SIZE)

#define PRIVMAXBYTES			    0x03                       //表示应用注册时权限的保存需要3个字节。
#define DIVERSIFIEDLEN 			    0x0A
#define MAX_GLOBALSERVICE_NAME_NUMBER  0x20
#define APPPROIDLEN					0x08
//==========================add for UICC Config of special key======================================
#define TOKEN_KEY_VERSION           	 				0x70  
#define RECEIPT_KEY_VERSION        	 				0x71 
#define DAP_KEY_VERSION             		 				0x73  
#define DECIPER_LOAD_FILE_VESION    				0x75

#define TOKEN_DES_KEY_SET_BIT						0x01
#define TOKEN_RSA_KEY_SET_BIT 						0x02
#define RECEIPT_DES_KEY_SET_BIT 					0x04
#define RECEIPT_RSA_KEY_SET_BIT 					0x08
#define DAP_DES_KEY_SET_BIT 						0x10
#define DAP_RSA_KEY_SET_BIT 						0x20
#define DECIPER_LOAD_FILE_DES_KEY_SET_BIT 		0x40
//=======================================================================================

#define RESOLVE_RSA_KEY									0x00
#define SET_KEY_SET_FLAG								0x01
#define RESOLVE_DES_AES_KEY								0x02

//this used by key manager
/*
typedef struct keySet
{//88 bytes, that is 0x58 bytes
	u16 sequenceCounter; //only used in scp 02	
	u8 bKeySetVersion;   //key version 00 ~ 0x6f	
	u8 sdId;             //key load flag
	
	u8 keyData[MAX_KEYS * MAX_DES_KEY_LENGTH]; //key data
	
	u8 keyTypeAndCounter[KEY_ASSISTER_BUFFER_SIZE]; //just store the key identifier
	u8 keyLength; //just store the key length
	u8 keyNum;

  u8 bKeyFormat;
	u8 bKeyUseageQuailer[KEY_ASSISTER_BUFFER_SIZE];
	u8 bKeyAccess[KEY_ASSISTER_BUFFER_SIZE];
	u8 bOtaCounter[5]; //ts03.48

}KeySet;
*/
typedef struct keyMap
{//38 bytes, that is 0x26 bytes
	u8 bSDId;
	u8 bDefaultKeyVersion;
	u8 bSDType;
	u8 bKeySetNum;
	
	u8 bMaxAuthAttempt;
	u8 bTryAuthAttempt;
	u8 bRsaKeySetFlag;				//bit 0 Receipt Des Key,   bit 1 Receipt Rsa Key , bite 2 Token Rsa key ,bite3 Deciper Load file Des Key
	u8 bRsaKeyModuleLen;		
	u8 bTokenRsaKeyExpLen;
	u8 bReceiptRsaKeyExpLen;
	u8 bCAPublickeyExpLen;			//for CASD
	u8 bCASDPriAUTLen;				//暂时不用
	u8 bCASDpriCTLen;
	u8 bCASDKeyFlag;
	
	u16 sProtocol;
	//u16 sRam;
	
	//u32 iNvm;
	u32 iKeySetAddr;
		
	u8 bMaxKeyNum;
	u8 bKeyNum;
	u8 bPriv[PRIVMAXBYTES];

	//u8 bSDTypeExtern;

	u8 bDiversifiedData[DIVERSIFIEDLEN];
	
	u8 bDapKeyVersion;
	u8  bC983Value;
	
	u16 bC982Value;
	u16 bC987Value;	
}KeyMap;

/*
typedef struct RsaCrtKeySet
{
	u8 bSdId; 
	u8 bKeySetVersion;	
	u8 bPComponentLen;					
	u8 bQComponentLen;				   	
	u8 bPQComponentLen;				
	u8 bDP1ComponentLen;
	u8 bDQ1ComponentLen;
	u8 RFU1;


	
	u32 bPComponentAddr;				//keytype A4
	u32 bQComponentAddr;				//keytype A5
	u32 bPQComponentAddr;				//keytype A6
	u32 bDPComponentAddr;				//keytype A7
	u32 bDQComponentAddr;				//keytype A8
	

}RsaCrtKeySet;

typedef struct RsaNdeKeySet
{
	u8 bSdId;
	u8 bKeySetVersion;
	u8 bPubEComponentLen;
	u8 bPubNComponentLen;
	u8 bPrivateEComponentLen;
	u8 bPrivateNComponentLen;
	u8 RFU1;
	u8 RFU2;


	
	u32 bPubEComponentAddr;			
	u32 bPubNComponentAddr;
	u32 bPrivateEComponentAddr;
	u32 bPrivateNComponentAddr;


}RsaNdeKeySet;
*/
/*
typedef struct pkgSpace
{//8 bytes, that is 0x08 bytes
	u8 pagId;
	u8 rfu1;
	
	u16 ramSize;
	u32 codeSize;
//	u16 dataSize;
pkgEntry}PkgSpace;
*/

typedef struct dataMap
{//12 bytes, that is 0x0C bytes
	u8 sdId;
	u8 rfu1;
	u8 rfu2;
	u8 rfu3;
	
	u16 tag;
	u16 size;
	
	u32 addr;
}DataMap;

//u8 getDefaultKeyVersion(u8 sdId);
void gpDefaultKeyInit(void);
//KeySet  * getKeySet(u8 version, u8 sdId);
boolean decryptVerifyKey(u16 offset);
//u16 incrementSequenceCounter(u8 version);
u16 getKeyInfo(u8 sdId, u8* buffer);
void storeKeyData(void);
void updateKeys(u8 putTag);
void GetCounterKey(u8 spi,u8 kic , u8 kid);
u8 getCounter(u8 * buffer);
//void getDapKey(void);
//u8 getDesDapKeyVersion(u8 sdId);
//u8 getDefaultKeyCounter(u8 sdId, u8 *buf);
u8 deleteKeySet(void);
extern void GenSetKeys(void);
extern u8 DelAllKeySets(u8 sdID);
//extern void setKeysFlag(void);
extern u8 getDefaultKeyCounter(PN8 sdDataObj, PX8 buf);

#if PUT_HEXKEY
u16 put_Key(void);
#endif
u8 GetSdKeyNum(void);
u8 getCounter(u8 * buffer);
extern u8 setCounter(u8 *buffer);

//u8 * findFreeKeySet(u8 sdId);
void * findFreeKeySet(u8 sdId);
//u8 * findFreeKeySetScp03(u8 sdId);

//extern void AppendApduBuffer(ISDKeySet * buf);

extern u8 CommonCheckKey(void * CommonKeySet ,u8 bKeyInd ,memref KeyTypeAndCoun , u8 ScpTag , u8 TypeTag) ;
extern u8 upDateKeyScp03 (KeyMap   * m_KeyMaps, u8 sdId, u8 putTag, u8 sOff_, u8 bKeyContentFormat, u8 moreBlock) ;
extern void keyManagerFunction(void);

#endif

