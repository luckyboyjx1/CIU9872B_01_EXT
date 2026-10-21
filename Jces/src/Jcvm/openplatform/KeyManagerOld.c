/*lint -save -e644 经检查，本文件中有大量的变量未初始化警告，属于设计之初为节省赋值时间而产生的，将来修改代码，应该打开该警告重新检查*/

#include "KeyManagerOld.h"
#include "global_variable.h"
#include "cardManager.h"
#include "ob_manager.h"
#include "object.h"
#include "stack.h"
#include "mask.h"
#include "string.h"
#include "lifecycle.h"
#include "profile.h"
#include "opFunction.h"
#include "SysConfig.h"
#include "keyData.h"
#include "keySet.h"
#include "SCPInterface.h"
#include "SCPGenKeyInterface.h"
#include "SecurityDomainData.h"
#include "SecureCrypto.h"
#include "Crypto_API.h"
#include "JcvmRam.h"
#include "NvmMap.h"

#define KEY_CHECK_VALUE_LEN          0x03
#define RSA_KEY_INDEX                0x01
#define RSA_KEY_VERSION              0x70
#define DAP_KEY_VERSION              0x73
#define DAP_KEY_INDEX                0x01
#define RSA_KEY_LENGTH               0x80
#define RSA_KEY_TYPE                 0xA0
#define RSA_PUB_KEY_EXP              0xA0
#define RSA_PUB_KEY_MOD              0xA1
#define RSA_PRI_KEY_MOD              0xA2
#define RSA_PRI_KEY_EXP              0xA3
#if GP__AMENDMENT__D
	#define AES_KEY_TYPE			 0x88
#endif
	#define DES_KEY_TYPE			 0x80

/*****************************************************************************
 * File:  KeyManager.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,11,14	.created
 Original author: wang hui
 *****************************************************************************/
/*****************************************************************************
 * gp defualt key init

 * key version is oxff

 * this key can be replace by put key command

 *
 *****************************************************************************/
//void gpDefaultKeyInit(void)
//{
//#if !GP__VERSION__FOR_MINI_MASK
//	u32 addr;
//	/*@ -discast @*/
//	/*@ -namechecks@*/
//	KeyMap   * m_KeyMaps = (KeyMap  *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
//	/*@ +namechecks@*/
//	/*@ +discast @*/
//
//
//	nvmFillNoAtomic(MEMtPN8(m_KeyMaps), 0xFF, U16_M_U16(MAX_SD_NUM, sizeof(KeyMap)));
//	memRefresh();
//
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bSDId), 0);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bDefaultKeyVersion), 0xFF);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bSDType), U8_OR_U8(0, SD_TYPE_FIX_SIZE));
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bKeySetNum), MAX_KEYSET_NUM);
//
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bMaxAuthAttempt), 0xFE);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bTryAuthAttempt), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bRsaKeySetFlag), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bRsaKeyModuleLen), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bTokenRsaKeyExpLen), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bReceiptRsaKeyExpLen), 0x00);
//
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bCAPublickeyExpLen), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bCASDPriAUTLen), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bCASDpriCTLen), 0x00);
//	nvmWriteU8NoAtomic(&(m_KeyMaps[0].bCASDKeyFlag), 0x00);
//	if(isGPconfig221)
//	{
//		nvmWriteU16NoAtomic(MEMtPN8(&(m_KeyMaps[0].sProtocol)), 0x0255);
//	}
//	else
//	{
//		nvmWriteU16NoAtomic(MEMtPN8(&(m_KeyMaps[0].sProtocol)), 0x0235);
//	}
//
//	nvmWriteU32NoAtomic(MEMtPN8(&(m_KeyMaps[0].iKeySetAddr)), addr);
//
//	nvmWriteU16Atomic(MEMtPN8(&(m_KeyMaps[0].bMaxKeyNum)), U16_M_U16(MAX_KEYSET_NUM, MAX_KEYS));
//	nvmWriteU16Atomic(MEMtPN8(&(m_KeyMaps[0].bKeyNum)), 3);
//	//nvmCopyAtomic((memref)(memref)(m_KeyMaps[0].bPriv), priv, (jsize)PRIVMAXBYTES);	//remember to modify PrivAccess.initialize if priv changed
//	nvmCopyNoAtomic(MEMtPN8(m_KeyMaps[0].bPriv), MEMtPX8(JcvmConfig_GetVmConfigAddr(ISD_PRIV)), U32t16(PRIVMAXBYTES));
//	nvmWriteU8NoAtomic(MEMtPN8(&(m_KeyMaps[0].bC983Value)), 0);
//	nvmWriteU16NoAtomic(MEMtPN8(&(m_KeyMaps[0].bC982Value)), 0);
//	nvmWriteU16NoAtomic(MEMtPN8(&(m_KeyMaps[0].bC987Value)), 0);
//#if GP__AMENDMENT__D
//	if(SCP == 0x03)
//		nvmWriteU8NoAtomic((PX8)CM_SCP_TYPE, 3); //write key load key sets number
//	else
//#endif
//	nvmWriteU8NoAtomic(MEMtPX8(JcvmConfig_GetVmConfigAddr(CM_SCP_TYPE)), 2); //write key load key sets number
//
//	nvmWriteU32NoAtomic(MEMtPX8(JcvmConfig_GetVmConfigAddr(DATA_BACKUP)),
//						U16t32(NvmManager_Allocate(U32_M_U32(512, U8t32(logicalChannelNums)), MAP_ENTRY_BLOCK_PCK_ID)));
//
//	memRefresh();
//#endif
//}

#define MAX_DEFALUT_KEYVERSION 0x2F
u8 getDefaultKeyVersion(u8 sdId)
{
	return 0;
}

/*****************************************************************************
 * GET KEY BY KEY VERSION

 * IF FOUND ,RETURN THE KEY SET

 * input : jbyte key version

 * output: keySet
 *****************************************************************************/

#if GP__AMENDMENT__D


/*****************************************************************************
 * setKey

 * input : key version,keyIndex, keyData

 * update the keyData of the keySet with the key vesion.

 *
 *****************************************************************************/
/*u8 setKeyScp03(AesKeySet * Keyset, u8 keyIndex, u8 * keyData, u8 keyLen,u8 modifyKeyFlag)
{
//	u8 keyType = Keyset->keyTypeAndCounter[keyIndex];
	u8 * keyDataDest = (u8 *)Keyset->keyData;
	u8 index ;

	if(modifyKeyFlag == TRUE )
	{
		for(index=0;index<3;index++)
		{
			if(Keyset->keyTypeAndCounter[index]  == keyIndex)
			{
				if(index == 0)
					Keyset->encKeyLength = keyLen;
				else if(index == 1)
					Keyset->macKeyLength = keyLen;
				else if(index == 2)
					Keyset->dekKeyLength = keyLen;
				memCopyToRam((memref)(keyDataDest + index * MAX_AES_KEY_LENGTH), (memref)keyData, keyLen);	
			}
		}
	}
	
	for(index=0;index<3;index++)
	{
	 	if(Keyset->keyTypeAndCounter[index] == 0xFF)
	 	{
	 		Keyset->keyTypeAndCounter[index] = keyIndex;
			if(index == 0)
				Keyset->encKeyLength = keyLen;
			else if(index == 1)
				Keyset->macKeyLength = keyLen;
			else if(index == 2)
				Keyset->dekKeyLength = keyLen;
			memCopyToRam((memref)(keyDataDest + index * MAX_AES_KEY_LENGTH), (memref)keyData, keyLen);	
			break;
	 	}
	}

	return true;
	
}*/




#endif

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
u8 getDesDapKeyVersion(u8 sdId){

	u8 i;
	KeyMap   * m_KeyMaps = (KeyMap   *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
	
	for(i = 0; i < MAX_SD_NUM; i++)
	{
		if(m_KeyMaps[i].bSDId == sdId)
		{
			return m_KeyMaps[i].bDapKeyVersion;
		}
	}
	return 0xFE;//返回一个无效的值
}
#endif


/******************************************************************************
 * This method is used to process incoming PUT KEY APDUs and
 * modify the Key Set Array according to the P1, P2, and Data
 * elements of the APDU.
 * updateKey command.
 ******************************************************************************/

#if GP__AMENDMENT__D
u8 upDateKeyScp03 (KeyMap   * m_KeyMaps, u8 sdId, u8 putTag, u8 sOff_, u8 bKeyContentFormat, u8 moreBlock)
{
    
      return 1 ;            
}
#endif


void storeKeyData(void)
{
	
	pushShort(SW_WRONG_DATA);

	return;
}

#define DesKeyLength 6 
#define RsaKeyLength 7
#define PK_CA_AUT_Flag		0x01
#define KS_CASD_AUT_Flag	0x10
#define KS_CASD_CT_Flag		0x20
#define SK_CASD_AUT_Flag	0x02
#define SK_CASD_CT_Flag		0x04
#define First_AUT_KCV_Flag	0x40
#define First_CT_KCV_Flag	0x80
u16  getKeyInfo(u8 sdId, u8* buffer)
{
  
        return false;
}

/*lint -restore */
#if PUT_HEXKEY
#define HeaderBuffer_Length     4
#define ChaceValue_Length       4
u16 put_Key(void)
{
	return (u16)0x9000;
	
}

void AppendApduBuffer(ISDKeySet * buf)
{

	return;
}
#endif

u8 GetSdKeyNum(void)
{
	return 0;	
}


/*
获取scp80最大VERSION的KEYSET 的OTA COUNTER
*/
u8 getCounter(u8 * buffer)
{
	return true;
}

u8 setCounter(u8 *buffer)
{
	return true;
}

u8 DelAllKeySets(u8 sdID)
{
	return 2;

}


//产生RGK，置于apdubuff,生成SCP key,并设置该SD的SCP keys
//现在同时支持SCP02和SCP80
#if	GP__AMENDMENT__A__FOR_MASK
void GenSetKeys()
{
	
	u8							i;
	u8							TLV;
	u8							A6Len;
	u8							tagOff;
	u8							A6LenOff;
	u8							mode;
	u8							KeyVer;
	u8							SDAIDLen;
	u8							keyusagequalify;
	u16							sw;
	u16							sdRef;
	jref  						keySetRef;
	PX8 						keyDataAField;
	PX8 						seqPoint;
	PN8 						keySetObj;
	PN8							sdDataObj;
	u8 							seqCount[5];
	u8							MK[16];
	u8							DriDat[16];
	
	KeyVer						=	0;
	SDAIDLen					=	0;
	keyusagequalify 			= 	0;
	sw							= 	SW_NO_ERROR;
	mode						= 	U16t8(popShort());
	sdRef						= 	popReference();
	sdDataObj					= 	MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(sdRef)));
	
	#define	SCE_PULL_ID		0x01
	#define SCE_PUSH_A_ID	0x02
	#define SCE_PUSH_B_ID	0x04
	#define	KVN_TAG			0x83
	#define INISEQCNT_TAG	0x91
	#define KUQ_TAG			0x95
	
	#define SCPTYPE02		0x02
	#define SCPTYPE80		0x80

	ecpMem_Set(seqCount, 0x00, sizeof(seqCount));
	seqPoint = seqCount;
	if(mode == SCE_PULL_ID)
	{
		SDAIDLen 	= APDUBuffer[OFFSET_CDATA];
		A6LenOff 	= U8_A_U8(U8_A_U8(OFFSET_CDATA, SDAIDLen), 1);
		A6Len 		= APDUBuffer[A6LenOff];
		tagOff 		= U8_A_U8(A6LenOff, 1);//偏移到A6模板中的第一个tag
	}
	else if(mode == SCE_PUSH_A_ID || mode == SCE_PUSH_B_ID)					
	{
		A6LenOff = U8_A_U8(OFFSET_CDATA, 4);
		A6Len = APDUBuffer[A6LenOff];
		tagOff = U8_A_U8(A6LenOff, 1);//偏移到A6模板中的第一个tag
	}
	else
	{
		pushShort(SW_WRONG_DATA);
		return;
	}
//search key version, sequence counter, key usage qualify
	for(i=0; i<A6Len; i=U8_A_U8(i, TLV))
	{		
		if(APDUBuffer[U8_A_U8(tagOff, i)] == KVN_TAG)
		{
			KeyVer = APDUBuffer[U8_A_U8(U8_A_U8(tagOff, 2), i)];
		}

		if(APDUBuffer[U8_A_U8(tagOff, i)] == INISEQCNT_TAG)
		{
			seqPoint = PX8_A_U8(APDUBuffer, U8_A_U8(U8_A_U8(tagOff, 2), i));
		}

		if(APDUBuffer[U8_A_U8(tagOff, i)] == KUQ_TAG)
		{
			keyusagequalify = APDUBuffer[U8_A_U8(U8_A_U8(tagOff, 2), i)];
		}

		TLV = U8_A_U8(APDUBuffer[U8_A_U8(U8_A_U8(tagOff, 1), i)], 2);
	}
	if((KeyVer==0) || (KeyVer>0x2F) || (KeyVer>0x0F&&KeyVer<0x20))
	{
		pushShort(SW_WRONG_DATA);
		return;
	}
	//获取当前SD的SCP类型
	
	//获取当前SD的SCP的key的个数
/*	if(keySet_getKeyNum(keySetObj)==0 && mode == SCE_PUSH_B_ID)
	{
		pushShort(SW_WRONG_DATA);
		return;
	}
*/	
	if(REF_NULL == keySet_getDefaultKeySet(sdDataObj) && mode == SCE_PUSH_B_ID)
	{
		pushShort(SW_WRONG_DATA);
		return;
	}
	keySet_deleteAllScKey(sdDataObj);//删除该SD的所有key set
		
	//new keyset
	if(IsSCP02Key(KeyVer))
	{
		keySetRef = keySet_newKeySetBySCPType(KEY_FORMAT_02, sdDataObj, KeyVer, DES_KEY_LENGTH, U32t8(SCPTYPE02));//SCP02
	}

	if(IsSCP80Key(KeyVer))
	{
		keySetRef = keySet_newKeySetBySCPType(KEY_FORMAT_02, sdDataObj, KeyVer, DES_KEY_LENGTH, U32t8(SCPTYPE80));//SCP80
	}
	
	if(mode == SCE_PULL_ID)
	{
		//产生RGK
		Random_number_generator(MEMtPX8(MK),0x10, true);
		//产生ENC/DEK/MAC KEY
		ecpMem_Set(DriDat, 0xFF, 0x10); 
		DriDat[6] 	= 0xF0;
		DriDat[14] 	= 0x0F;
		tagOff 		= U8_A_U8(U8_A_U8(U8_A_U8(OFFSET_CDATA, SDAIDLen), A6Len), 2);//偏移到APDUbuff中A6模板信息的最后
		generateStaticKey(MEMtPM8(MK), MEMtPM8(PX8_A_U8(APDUBuffer, tagOff)), MEMtPM8(DriDat));//产生通道密钥
		//ecpMem_Copy(APDUBuffer+tagOff, MK, 0x10);//搬移随机数到APDUbuff中A6模板信息的最后		
	}
	else if(mode == SCE_PUSH_A_ID || mode == SCE_PUSH_B_ID)					
	{
		tagOff = U8_A_U8(U8_A_U8(U8_A_U8(7, APDUBuffer[7]), 1), 3);//偏移到DGI8010中的通道密钥位置
	}
	else
	{
		pushShort(SW_WRONG_DATA);
		return;
	}
	keySetObj		  =	MEMtPN8(locateObjectInMemory(keySetRef));
	//从apdubuff中写入入密钥数据
	keyDataAField = MEMtPN8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_01, KEY_TYPE_INIT_VALUE, DES_KEY_LENGTH));
	nvmCopyAtomic(MEMtPN8(keyDataAField), MEMtPN8(PX8_A_U8(APDUBuffer, tagOff)), DES_KEY_LENGTH);
	keyDataAField = MEMtPN8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, KEY_TYPE_INIT_VALUE, DES_KEY_LENGTH));
	nvmCopyAtomic(MEMtPN8(keyDataAField), MEMtPN8(PX8_A_U8(APDUBuffer, U8_A_U8(tagOff, 0x10))), DES_KEY_LENGTH);
	keyDataAField = MEMtPN8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_03, KEY_TYPE_INIT_VALUE, DES_KEY_LENGTH));
	nvmCopyAtomic(MEMtPN8(keyDataAField), MEMtPN8(PX8_A_U8(APDUBuffer, U8_A_U8(tagOff, 0x20))), DES_KEY_LENGTH);
	
	if(mode == SCE_PULL_ID)
	{				
		ecpMem_Copy(PX8_A_U8(APDUBuffer, tagOff), MK, 0x10);//搬移随机数到APDUbuff中A6模板信息的最后
	}
	
	//KeySetRef 		  = keySet_getKeySet(sdDataObj, KeyVer);	
	if(IsSCP02Key(KeyVer))
		keySet_setSequenceCounter(keySetObj, SCP02_SEQUENCE_CONTER_SIZE, seqPoint);
	if(IsSCP80Key(KeyVer))
		keySet_setSequenceCounter(keySetObj, SCP80_SEQUENCE_CONTER_SIZE, seqPoint);
	pushShort(sw);
	
}
#endif
