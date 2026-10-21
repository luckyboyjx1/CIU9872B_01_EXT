/*
 * KeyLogicManager.h
 *
 *  Created on: 2013-9-29
 *      Author: liuhanren
 */

#ifndef KEYLOGICMANAGER_H_
#define KEYLOGICMANAGER_H_

#include "KeyManagerOld.h"
#include "SysConfig.h"
#include "keyData.h"
#include "profile.h"

#define MAX_AES_KEY_LENGTH		 	        		0x20
#define AES_KEY_LENGTH_24      		      			0x18
#define AES_KEY_LENGTH_16       	    			0x10

#define PUT_KEY_SCP01			        			SCP_01 //0x01
#define PUT_KEY_SCP02            	    			SCP_02 //0x02
#define PUT_KEY_SCP03             	   			 	SCP_03 //0x03
#define PUT_KEY_SCP80             	    			SCP_80 //0x80

#if	GP__AMENDMENT__B__FOR_MASK
#define	PUT_KEY_SCP81								SCP_81 //0x81
#endif

#define PUT_KEY_SCP10             	    			SCP_10 //0x10
#define PUT_KEY_SCP11             	    			SCP_11 //0x11， *modify by xuxiaojun@20220528 这里的值必须与SCP_11保持一致，否则会导致创建的KEY的SCP的TYPE为11.   测试用例：AmendH测试脚本-SCP11c   Annex_B_scenario_02_DM_without_autoRestore_Taes16B_Raes16B_bb_02_11
#define PUT_KEY_SCPFF				    			SCP_FF //0xFF

#define KEY_DEFAULT_VERSION							0xFF
#define KEY_DEFAULT_CONTER							0x0000
#define KEY_DEFAULT_PROPERTY						0x00
#define	INIT_KEY_ID									0x01
#define MAX_KEYS_NUM								0x03
//====================================Version TYPE=======================================================
#define	KEY_TOKEN_ID1								0x70
#define	KEY_RECEIPT_ID1								0x71
#define	KEY_DAP										0x11
#define	KEY_DAP_VERIF_ID1							0x73
#define KEY_CIPHERED_LOAD_ID1						0x75
#define KEY_PROFILE_LOAD							0x7F	//M2M转的天谕脚本中，profile中包含0x7Fkey,该key是何作用目前还不清楚，jiangqingbin-20220810 (用例：TY_OfflineProfileFormatTest/Security domains\正常案例/Security_domains_N09)
#define KEY_SCP80_MIN								0x01
#define KEY_SCP80_MAX								0x0F
#define KEY_SCP81_MIN								0x40
#define KEY_SCP81_MAX								0x4F
#define KEY_SCP03_MIN								0x30
#define KEY_SCP03_MAX								0x3F

//=======================================================================================================
//=====================================Key TYPE======================================================
#define			DES_KEY_TYPE_MIN					0x80
#define			DES_KEY_TYPE_MAX					0x85
#define			AES_KEY_TYPE_MAX					0x88
#define			SHA1_KEY_TYPE_MIN					0x90
#define			SHA1_KEY_TYPE_MAX					0x91
#define			RSA_KEY_TYPE_MIN					0xA0
#define			RSA_KEY_TYPE_A1						0xA1
#define			RSA_KEY_TYPE_MAX					0xA8
#define			EXPONENT_LEN_BYTE1					0x01
#define			EXPONENT_LEN_BYTE3					0x03

//=======================================================================================================
//=======================================================================================================
#define PUT_KEY_MASK								0x7F
#define MAX_KEY_VERSION								0x7F
#define	MIN_KEY_VERSION								0x01
#define MAX_KEY_ID									0x7F
#define MIN_KEY_ID									0x00
#define	PUT_MORE_KEY_MASK							0x80
#define PUT_MULTIPLE_KEY_MASK						0x80

#define KEY_TYPE_ILLAGE                             0x00
#define KEY_DES_TYPE								0x80
#define	KEY_DES_CBC_MODE							0x84
#define	KEY_DES_ECB_MODE							0x83
#define KEY_AES_TYPE								0x88
#define	KEY_SCP_81_TYPE								0x85
#define MORE_COMMAND_FIRST_TAG						0x01
#define MORE_COMMAND_OTHERS_TAG						0x02
#define MORE_COMMAND_LAST_TAG						0x03

#define THROW_ERROR									0x00
#define NO_ERROR									0x01

/********************put key指令中解密key密文时的解密算法***********************/
#define DecryptionAlg_ILLEGAL           0x00
//#define DecryptionAlg_SINGLE_DES        0x80
#define DecryptionAlg_3DES_ECB          0x81	//0x02
#define DecryptionAlg_3DES_CBC          0x82	//0x01
#define DecryptionAlg_AES_CBC           0x88	//0x03

//=======================================================================================================

#define PRIVMAXBYTES			        			0x03
#define DIVERSIFIEDLEN 			    				0x0A

#define SCP_MAX_VERSION								0x6F // todo? 是否需要与MAX_KEY_VERSION保持一致？
#define PUT_KEYS_MODE_EN							0x01	//加密模式
#define PUT_KEYS_MODE_OR							0x00	//普通模式						
//=========================add for Mapping guidelines=================================================
#define 		 U8_LEFT_SHIFT(x)					U16_SHL_U8(x, 8)
#define			 U8_LOW(x)					    	U8_AND_U8(x, 0x0F)
#define		     U8_HIGH(x)							U8_AND_U8(x, 0xF0)
#define			 SIZE_82_LENGTH_1						0x03
#define			 SIZE_81_LENGTH_1					    0x02
#define			 INIT_KEEP_REF_FOR_SPLIT			0x00
//store key:8F01
#define			 DES_SECURE_CHANNEL_KEY				0x8F01 // Mapping_guidelines_v1.0.1-Final.pdf,  Secure Channel DES keys
#define          STORE_KEY_RELATED_DATA_OFF			0x38
#define			 STORE_KEY_CHECK_VALUE_OFF			0x3E
#define          STORE_KEY_CURRENT_VERSION			0x3B
#define			 STORE_KEY_RELATED_DATA_LEN			0x0C
#define 		 STORE_KEY_DATA_LEN_OFF				0x07
#define          STORE_KEY_DATA_OFF					0x08
#define			 STORE_KEY_LEN						0x10
#define 		 STORE_KEY_DATA_LEN					0x30
#define  		 STORE_KEY_TYPE						0x80
//store key:9102
#define 		 DAP_VERIFICATION_KEY				0x9102 // Mapping_guidelines_v1.0.1-Final.pdf,  DAP Verification key information
#define 		 STORE_KEY_DAP_LEN_OFF				0x07
#define			 DAP_RELATED_DATA_LEN				0x06
#define			 DAP_CURRENT_VERSION			    0x00
#define 		 DAP_NEW_VERSION					0x73
#define			 DAP_MODULUS_KEY_TYPE				0xA1
#define			 DAP_MODULUS_LEN					0x80
#define			 DAP_EXPONENT_KEY_TYPE				0xA0
//====================================================================================================
#define IsSCP02Key(keyVersion) ((keyVersion >= 0x20) && (keyVersion <= 0x2F))
//extern u8 IsSCP02Key(u8 keyVersion); //modify by xuxiaojun@20220621  gpBFCCompliance208: km_APDU_putKeyDes_OPR_11_21把key version 为02的也认为是scp02 key
#if GP__AMENDMENT__D
#define IsSCP03Key(keyVersion) ((keyVersion >= 0x30) && (keyVersion <= 0x3F))
#else
#define IsSCP03Key(keyVersion)  0
#endif

#if (JAVA_CARD_UICC||COS_UPDATE_ONLINE_BY_NativeOS)	//在线升级触发短信需要用到
#define IsSCP81Key(keyVersion) ((keyVersion >= 0x40) && (keyVersion <= 0x4F))
#define IsSCP80Key(keyVersion) ((keyVersion >= 0x01) && (keyVersion <= 0x0F))
#else
#define IsSCP81Key(keyVersion)   0
#define IsSCP80Key(keyVersion)   0
#endif

#if GP__SCP__SCP11__FOR_MASK
#define IsSCP11Key(keyVersion) ((keyVersion >= 0x18) && (keyVersion <= 0x1F))
#else
#define IsSCP11Key(keyVersion)   0
#endif

#define IsRSAKeyType(keyType)          ((keyType >= RSA_E_DGI_TYPE)&&(keyType <= RSA_CRT_DQ_DGI_TYPE))
#define IsECCKeyType(keyType)          (((keyType >= ECC_PublicKey_DGI_TYPE)&&(keyType <= ECC_CurvePara_K_DGI_TYPE)) || (keyType == ECC_KEYPARAREF_DGI_TYPE))
#define IsLoadingofECCCurveParameters  (((g_markEccKey&0xFF) == 0xFD) || ((g_markEccKey&0xFF) == 0x7D))
#define IsLoadingofECCWithF0           ((g_markEccKey&0xFF) == 0x03)
#define IsLoadingofECCWithOwnParas     (((g_markEccKey&0xFF) == 0xFE) || ((g_markEccKey&0xFF) == 0x7E))
#define IsEccKeyNorParaRefVal(value)   ((value>=0x00)&&(value<=0x08)) //GPC_Specification_v2.3.1.pdf:  Table B-2:  Key Parameter Reference Values

//#define IsSCPFFKey(keyVersion) ((keyVersion >= KEY_TOKEN_ID1)&&(keyVersion <= 0x7F))//70--7F todo 是否可以和IsSpecialKey合并？

#define IsSpecialKey(keyVersion) ((KEY_TOKEN_ID1 == keyVersion)	|| (KEY_RECEIPT_ID1 == keyVersion) || (KEY_DAP == keyVersion) || (KEY_DAP_VERIF_ID1 == keyVersion) || (KEY_CIPHERED_LOAD_ID1 == keyVersion) || (KEY_PROFILE_LOAD == keyVersion))

//========================keyManager_InsertApduManager=========================//
#define	 PUT_KEY_COMMAND			 0
#define	 OTHER_COMMAND				 1

//====================================================================================================//
#define getlifeCycleTransition(sdEntryObj)         memReadU8(MEMtPM8(get_object_field_ref(sdEntryObj, lifeCycleTransition_INDEX)))

#if GP__KEY_MANAGER
	extern void decryptStoreKeyData(void);
	extern void KLG_InitPutKeySequence(u8 p1);
	extern void  SaveKey(PN8 sdDataObj);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数
	extern u16  newKeySetByKeyVersion(PN8 sdDataObj, u8 KeyFormat, u8 curKeyVersion, u8 newKeyVersion, u16 keyLen);

	#if GP__KEY_MANAGER__DELETE_KEY
	extern u8	delSetKey(void);
	#else
	#define delSetKey()																					0
	#endif
	extern void setKeyData(jref keySet, u8 keyVersion, u8 keyId, u8 keyType, u16 keyLen, u16 SinglekeyComponentLen, u8 Off);
	extern void KM_InitPutKeySequence(u8 p1);
	extern void keyManager_initLogicManager(void);
	extern void keyManager_abortLogicManager(void);
	extern void keyManager_abortLogicManagerThorwError(u16 error);
	extern memref getEntryByAppId(u8 sdId);
	extern u16 storeKey(PN8  sdDataObj);
	extern void keyManager_InsertApduManager(u8 InsertType);
	extern u16 keyData_getKeyData(u8 sdId, u8 keyVersion, u8 keyId, memref keyDataBuffer);
	extern u16 keyData_fetchKey(u8	sdId, u8 keyVersion, u8 keyId, memref keyDataBuffer);
	extern u16 getScp80Key(u8 appId, u8 keyVersion, u8 keyId, memref keyDataBuffer);
	extern	u8	getSCPTypeByKeyVersion(u8	keyVersion);
	extern void resetSequenceCounter(jref keySetRef, u8 keyVersion);
	extern u16 getScp80Key(u8 appId, u8 keyVersion, u8 keyId, memref keyDataBuffer);
	extern void resetAllCounter(void);
	extern jref getSCP80KeySEQ(u8 sdId, u8 keyVersion, PX8 counterppointer);
	extern void setKeyCounterForSCP80(u8 sdId, u8 keyVersion, u8 len, PR8 counterValue);
	extern void setAllKeyCounterForSCP80(u8 sdId, u8 len, PR8 counterValue);
	extern memref getCounterAddrByVersion(u8 sdId, u8 keyVersion);
	extern u8 AnalyseEncKeyType(void);
	extern u8 keyTypeValid(u8 keyType);
	extern u8 getFormatByKeyVersion(u8 KeyVersion);
	extern u8 KeyVersionIsValid(u8 keyVersion);
#else
	#define decryptStoreKeyData()
	#define KLG_InitPutKeySequence(p1)
	#define SaveKey(sdDataObj)
	#define delSetKey()																					0
	#define setKeyData(keySet, keyVersion, keyId, keyType, keyLen, SinglekeyComponentLen, Off)
	#define KM_InitPutKeySequence(p1)
	#define keyManager_initLogicManager()
	#define keyManager_abortLogicManager()
	#define keyManager_abortLogicManagerThorwError(error)
	#if GP__VERSION__FOR_MINI_MASK
		extern memref getEntryByAppId(u8 sdId);
		extern u16 keyData_fetchKey(u8	sdId, u8 keyVersion, u8 keyId, memref keyDataBuffer);
		extern void resetAllCounter(void);
		extern jref getSCP80KeySEQ(u8 sdId, u8 keyVersion, PX8 counterppointer);
		extern void setKeyCounterForSCP80(u8 sdId, u8 keyVersion, u8 len, PR8 counterValue);
		extern void setAllKeyCounterForSCP80(u8 sdId, u8 len, PR8 counterValue);
	#else
		#define getEntryByAppId(sdId)																	0
		#define keyData_fetchKey(sdId, keyVersion, keyId, keyDataBuffer)								0
		#define resetAllCounter()																		0
		#define getSCP80KeySEQ(sdId, keyVersion, counterppointer)										0
		#define setKeyCounterForSCP80(sdId, keyVersion, len, counterValue)
	#endif
	#define storeKey(sdDataObj)																			0
	#define keyManager_InsertApduManager(InsertType)
	#define keyData_getKeyData(sdId, keyVersion, keyId, keyDataBuffer)									0
	#define getScp80Key(appId, keyVersion, keyId, keyDataBuffer)										0
	#define getSCPTypeByKeyVersion(keyVersion)															0
	#define resetSequenceCounter(keySetRef, keyVersion)
	#define getScp80Key(appId, keyVersion, keyId, keyDataBuffer)										0
	#define AnalyseEncKeyType()                                                                         0
    #define keyTypeValid()                                                                              0
#endif

#endif /* KEYLOGICMANAGER_H_ */
