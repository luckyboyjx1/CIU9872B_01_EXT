/*
 * keySet.c
 *
 *  Created on: 2013年9月29日
 *      Author: mowenpan
 */

#include "typejava.h"
#include "mask.h"
#include "cardmanager.h"
#include "error.h"
#include "object.h"
#include "stdio.h"
#include "ob_manager.h"
#include "Loader.h"
#include "bc_object.h"
#include "global_variable.h"

#include "keySet.h"
#include "fm01_keyData.h"
#include "fm02_keyData.h"
#include "keySet.h"
#include "keyData.h"
#include "SecurityDomainData.h"
#include "keyGroup.h"
#include "KeyExtData.h"
#include "fm03_keyData.h"
#include "casdStoreKey.h"
#include "KeyManagerOld.h"
#include "opFunction.h"
#if GP__CGM__FOR_MASK
	#include "CgmDel.h"
#endif

#if M2M_EUICC
#include "EUICCMain.h"
#endif
#if GP__KEY_MANAGER
// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP 将此处的宏定义移到keySet.h中


u8 keySet_getMaxKeyVersion(PN8 sdDataObj, u8 scpType)
{
	PN8 keySetHand;
	jref keyGroupRef;
	jref keySetRef;
	PN8 keySetObj;

	u8 maxKeyVersion;
	u8 tmpKeyVersion;

  maxKeyVersion = 0xFF;
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE);
	if (REF_NULL == keyGroupRef)
	{
		return 0xFF;
	}


	keySetHand = MEMtPN8(getFirstKeySetHand(keyGroupRef));

	while (true)
	{
		keySetRef = memReadU16(keySetHand);
		if (REF_NULL == keySetRef)
		{
			break;
		}

		keySetObj =  MEMtPN8(locateObjectInMemory(keySetRef));
		tmpKeyVersion = keySet_getKeyVersion(keySetObj);
		 if ((tmpKeyVersion > maxKeyVersion) || (maxKeyVersion == 0xFF))
		 {
			 if ((0x02 == scpType))
			 {
				 if (IsSCP02Key(tmpKeyVersion) && (0x07 == U8_AND_U8(0x07 , keySet_getkeyAllTag(keySetObj))) )// modified by zm; 过滤不完整的keySet
				 {
					maxKeyVersion = tmpKeyVersion;
				 }
			 }
			 else
			 {
				 maxKeyVersion = tmpKeyVersion;
			 }
		 }

		 keySetHand =  MEMtPN8(getNextKeySetHand(keySetObj));
	}

	return maxKeyVersion;
}

static u16 genKeyDataLen(u8 format, u16 keyLen)
{
	u16 keyDataLen;

	keyDataLen = 0;
	switch (U8_AND_U8(format, KEY_FORMAT_STORE_MASH))
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			keyDataLen = U16_A_U16(keyLen, KEY_ATTR_LENGTH);
			keyDataLen = U16_A_U16(keyDataLen, KEY_ACCESS_LENGTH);
			keyDataLen = U16_A_U16(keyDataLen, KEY_USAGE_QUALIFIER_LENGTH);
			keyDataLen = U16_M_U16(keyDataLen, MAX_KEY_NUM);
			//keyDataLen = (KEY_ATTR_LENGTH  + KEY_ACCESS_LENGTH + KEY_USAGE_QUALIFIER_LENGTH + keyLen) * MAX_KEY_NUM;
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			keyDataLen = U32t16(U16_A_U16(keyLen, KEY_ATTR_LENGTH));
			keyDataLen = U32t16(U16_A_U16(keyDataLen, KEY_ACCESS_LENGTH));
			keyDataLen = U32t16(U16_A_U16(keyDataLen, KEY_USAGE_QUALIFIER_LENGTH));
			keyDataLen = U32t16(U16_M_U16(keyDataLen, MAX_KEY_NUM));
			keyDataLen = U32t16(U16_A_U16(keyDataLen, FM02_KEY_DATA_LENGTH_WIDTH));
			keyDataLen = U32t16(U16_A_U16(keyDataLen, FM02_KEY_DATA_KEYID_SIZE));
			//keyDataLen = FM02_KEY_DATA_LENGTH_WIDTH + (KEY_ATTR_LENGTH  + KEY_ACCESS_LENGTH + KEY_USAGE_QUALIFIER_LENGTH + keyLen) * MAX_KEY_NUM;
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			keyDataLen = keyLen;
			break;
#endif
		default:
			throw_error(SW_WRONG_DATA);
			break;
	}

	return keyDataLen;
}

/**
 *在 keySet 链表中查找特定的keySet对象
 *
 *参数：
 *	keySetHand 为开始 keySet 对象的句柄
 *	keyVersion 为查找 keySet 对象的版本号
 *
 *返回值为 keySet 对象的句柄
 *	如果找不到 keySet 对象返回 MEM_NULL.
 *
 */
memref keySet_getKeySetHand(PN8 keySetHand, u8 keyVersion)
{
	PN8 keySetObj;
	jref keySetRef;
	u8 temKeyVersion;


	while (true)
	{
		keySetRef = memReadU16(keySetHand);
		if (REF_NULL == keySetRef)
		{
			break;
		}

		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
		temKeyVersion = keySet_getKeyVersion(keySetObj);

		if (temKeyVersion == keyVersion)
		{
			return PNtMEM(keySetHand);
		}
		else
		{
			keySetHand = MEMtPN8(getNextKeySetHand(keySetObj));
		}
	}

	return MEM_NULL;
}

/**
 *获取指定SD的默认 keyset 对象
 */
jref keySet_getDefaultKeySet(PN8 sdDataObj)
{
	u8 scpType;
	jref keyGroupRef;

	//scpType = sdData_getScpType(sdDataObj);
	scpType = sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP0X_WITHOUT_IOPTION_MODE, 0x00);
	if(0xFF == scpType)
	{
		return REF_NULL;
	}
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE);
	if (REF_NULL == keyGroupRef)
	{
		return REF_NULL;
	}

	return keyGroup_getKeySetRef(MEMtPF8(locateObjectInMemory(keyGroupRef)));
}

/**
 *获取指定SD的默认密钥版本号。
 */
u8 keySet_getDefaultKeyVersion(PN8 sdDataObj)
{
	jref keySetRef;

	keySetRef = keySet_getDefaultKeySet(sdDataObj);
	if (REF_NULL == keySetRef)
	{
		return REF_NULL;
	}
	
	return keySet_getKeyVersion(MEMtPF8(locateObjectInMemory(keySetRef)));
}

/**
 *通过密钥版本号获取指定的 keySet 对象
 */
//jref findKeySet(u8 sdId, u8 keyVersion)
jref keySet_getKeySet(PN8 sdDataObj, u8 keyVersion)
{
	PN8 keySetHand;
	jref keyGroupRef;
	u8 scpType;


	scpType = keyGroup_getScpTypeByKeyVersion(sdDataObj, keyVersion);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 函数名称修改
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE);
	if (REF_NULL == keyGroupRef)
	{
		return REF_NULL;
	}

	keySetHand = MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keyGroupRef)), keySet_INDEX));
	keySetHand = MEMtPN8(keySet_getKeySetHand(keySetHand, keyVersion));
	if (MEM_NULL != keySetHand)
	{
		return memReadU16(keySetHand);
	}

	return REF_NULL;
}

/**
 *创建一个 keySet 对象(没有对对象成员做任何初始化工作)，并且挂到 keyGroup 链表下面
 *modified by xuxiaojun@20210414 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 */
static jref keySet_newInstance(PN8 keyGroupObj)
{
	jref keySetRef;
	PN8 keySetHand;

	keySetHand = MEMtPN8(get_object_field_ref(keyGroupObj, keySet_INDEX));


	while (true)
	{
		keySetRef = nvmReadU16(keySetHand);

		if (REF_NULL == keySetRef)
		{
			keySetRef = newObject(KEY_SET_CLASS_OFFSET,KEY_SET_PCK_ID, BIT_NEED_REFRESH);
			nvmWriteU16Atomic(MEMtPN8(keySetHand), keySetRef);
			return keySetRef;
		}
		else
		{
			keySetHand = MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keySetRef)), nextKeySet_INDEX));
		}
	}

	//return MEM_NULL;
}

static void keySet_init(PN8 keySetObj, u16 keyLen, u8 keyVersion, u8 keyFormat)
{
	jref keyDataRef;
	u16 keyDataLen;

	keyDataLen = genKeyDataLen(keyFormat, keyLen);
	keyDataRef = keyData_newKeyData(keySetObj, keyDataLen); //创建 keyData

	keySet_setKeyVersion(keySetObj, keyVersion);

	keySet_setKeyFormat(keySetObj, keyFormat);

#if GP__KEY_MANAGER__FORMAT2
	if (KEY_FORMAT_02 == U8_AND_U8(keyFormat , KEY_FORMAT_STORE_MASH))// modified by liuhanren, key format may be 82
	{
		memref keyDataField;

		keyDataField = get_array_data(MEMtPF8(locateObjectInMemory(keyDataRef)));
		//nvmFillAtomic(MEMtPN8(keyDataField), FM02_KEY_DATA_KEYID_SIZE, FM02_MASK_KEYID);
		nvmFillAtomic(MEMtPN8(keyDataField), FM02_MASK_KEYID, FM02_KEY_DATA_KEYID_SIZE);
		FM02_setKeyLength(keyDataField, keyLen);
	}
#endif
	memRefresh();
}


/**
 *删除 SD 下面所有 SC KEY
 */
void keySet_deleteAllScKey(PN8 sdDataObj)
{
	u8 scpType;
	jref keyGroupRef;
	PN8 keySetHand;

	scpType = sdData_getScpType(sdDataObj);
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE);
	if (REF_NULL == keyGroupRef)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	keySetHand = MEMtPN8(getFirstKeySetHand(keyGroupRef));
	nvmWriteU16Atomic(MEMtPN8(keySetHand), REF_NULL);
}

#if GP__KEY_MANAGER__DELETE_KEY
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 * modify by xuxiaojun@20220629 对于辅助安全域，如果被删除的keyVersion是该SSD下所属SCP协议的唯一一组key，则不允许被删除。主要是考虑到SSD切换到personal状态的转换
 */
void keySet_deleteScKey(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	jref keyGroupRef;
	jref keySetRef;
	PN8 keySetObj;
	PN8 keySetHand, firstKeySetHand;
	u8 format;
	u8 keyNum;
	u8 scpType;
//	u8 tmpKeyVersion;
	scpType = keyGroup_getScpTypeByKeyVersion(sdDataObj, keyVersion);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 函数名称修改
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE);
	if (REF_NULL == keyGroupRef)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
	
	if (SCP_FF == scpType) {
		firstKeySetHand = MEMtPN8(getFirstKeySetHand(keyGroupRef));
	}
	else
	{
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK || GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
		if(0x80 == scpType)
		{
			firstKeySetHand = MEMtPN8(getFirstKeySetHand(keyGroupRef));
		}else
#endif
		{
			if(SCP_80 == scpType)//for test ,需要删scp80密钥
			{
				firstKeySetHand = MEMtPN8(getFirstKeySetHand(keyGroupRef));
			}
			else
			{
				/*keySetRef = getFirstKeySetRef(keyGroupRef);
				if (REF_NULL == keySetRef)
				{
					throw_error(SW_REFERENCED_DATA_NOT_FOUND);
				}

				//keySet列表中第一个为默认keyset, 默认keyset不能删除
				keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
				tmpKeyVersion = keySet_getKeyVersion(keySetObj);

				if (keyVersion == tmpKeyVersion)
				{
					throw_error(SW_CONDITIONS_NOT_SATISFIED);
				}
		
				keySetHand = MEMtPN8(getNextKeySetHand(keySetObj));*/

				//UICC configuration 8.1   All of the Key Sets for a Security Domain may be deleted. If the default Key Set is deleted, and one or more Key Sets remain, another Key Set becomes the new default Key Set. The determination of the new default Key Set is UICC implementation-dependent.
				firstKeySetHand = MEMtPN8(getFirstKeySetHand(keyGroupRef));
			}
		}
	}
	keySetHand = MEMtPN8(keySet_getKeySetHand(firstKeySetHand, keyVersion));

	if (MEM_NULL == keySetHand)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
/* modify by xuxiaojun@20230302: 这块限制代码是测试部当时提出来的，觉得不合理，但是规范没有明确规范，加上之后出现了各种冲突。现在找到了规范来源，UICC configure中是可以删除所有的key
The support of key deletion is optional.  All of the Key Sets for a Security Domain may be deleted. If the
default Key Set is deleted, and one or more Key Sets remain, another Key Set becomes the new default
Key Set. The determination of the new default Key Set  is UICC implementation - dependent.
*/

//#if M2M_EUICC
//	if (sdDataObj != MEMtPN8(getSdDataObjBySdid(ISDR_APPID)))	//ISDR需要支持删除除指定密钥外的所有key，jiangqingbin-20220825 （用例：eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/TC_ES5_FIH_1_FinaliseISDRhandover_SMS_01_Nominal_Case_Del_All_Keys_not_SCP80_Keys
//#endif
//	{
//	//测试用例DRP008 new 修改代码，主安全域下也必须保留一组key
//		if((SCP_FF != scpType)&&(REF_NULL == memReadU16(getNextKeySetHand(MEMtPN8(locateObjectInMemory(memReadU16(firstKeySetHand)))))))//表示keyVersion所属的SCP只有一套密钥，此时不允许删除key. 测试用例：SCP11测试脚本  update_scp11c_key_Test_ssd_01
//		{
//			//以下判断在测试UL508时需要把配置conf_ApduDeleteKeySupportKeyVersionNumberAndKeyId配置为FALSE, 否则存在冲突， jiangqingbin-20220825 （用例：UL508/km_APDU_deleteKey_7a_9b_f6_desReceipt_scp02_b3_0e_c5）
//			if((REF_NULL == memReadU16(getNextKeySetHand(MEMtPN8(locateObjectInMemory(memReadU16(firstKeySetHand)))))))//表示keyVersion所属的SCP只有一套密钥，此时不允许删除key. 测试用例：SCP11测试脚本  update_scp11c_key_Test_ssd_01
//			{
//				throw_error(SW_CONDITIONS_NOT_SATISFIED);
//			}
//		}
//	}

	//delete key
	keySetRef = memReadU16(keySetHand);

	if (REF_NULL == keySetRef)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));

	format = keySet_getStoreKeyFormmat(keySetObj);
	if (KEY_ID_INIT_VALUE != keyId) //单个 密钥
	{
		switch (format)
		{
#if GP__KEY_MANAGER__FORMAT1
			case KEY_FORMAT_01:
				FM01_deleteSingleScKey(keySetObj, keyId);
				break;
#endif

#if GP__KEY_MANAGER__FORMAT2
			case KEY_FORMAT_02:
				FM02_deleteSingleScKey(keySetObj, keyId);
				break;
#endif

#if GP__KEY_MANAGER__FORMAT3
			case KEY_FORMAT_03:
				FM03_deleteSingleScKey(keySetObj, keyId);
				break;
#endif

			default:
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
				break;
		}

		keyNum = keySet_getKeyNum(keySetObj);
		memRefresh();
	}
	else
	{
		keyNum = 0;
	}

	if (0 == keyNum)
	{
		//modified by zm; 添加移动需求,要求删除一个DES group的时候，要KEY COUNT要加3
		// 1 : type ; 2:num; 3 inc num
		#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		if(0x80 == getKeyType(keySetObj))//des key
		{
				sdData_setDESKeyCount(sdDataObj, sdData_getDESKeyCount(sdDataObj) + keySet_getKeyNum(keySetObj));
		}
		#endif
		
//#if GP__CGM__FOR_MASK
//		cgmDel_Key(keySetObj);
//#endif
		nvmWriteU16Atomic(MEMtPN8(keySetHand), keySet_getNextKeySetRef(keySetObj));
		memRefresh();

		//#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		bRamBuffer[GC_Requested] = 1;
		//#endif
		return ;
	}
}
#endif

/**
 *FM01
 *FM02
 *FM03  key
 *创建keySet
 *@history: merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
jref keySet_newKeySet(u8 format, PN8 sdDataObj, u8 keyVersion, u16 keyLen)
{
	jref keySetRef;
	jref keyGroupRef;
	u8 scpType;


	scpType = keyGroup_getScpTypeByKeyVersion(sdDataObj, keyVersion);// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 函数名称修改
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, TRUE);

	keySetRef = keySet_newInstance(MEMtPN8(locateObjectInMemory(keyGroupRef))); //创建 keySet 对象实例
	keySet_init(MEMtPN8(locateObjectInMemory(keySetRef)), keyLen, keyVersion, format); //初始化 keySet 对象实例

	memRefresh();
	return keySetRef;
}


//根据不同的SCP类型，new出keyset，并挂在该SCP类型的keygroup下
jref keySet_newKeySetBySCPType(u8 format, PN8 sdDataObj, u8 keyVersion, u16 keyLen, u8 SCPType)
{
	jref keySetRef;

	keySetRef = keySet_newKeySet(format, sdDataObj, keyVersion, keyLen);
	return keySetRef;
}



/**
 *替换 keySet 对象的密版本号
 *
 */
void keySet_replaceKeyVersion(PN8 keySetObj, u8 keyVersion)
{
	keySet_setKeyVersion(keySetObj, keyVersion);
}


/**
 *
 *获取 SequenceCounter 存放地址
 *由于不同的 scpType 的 SequenceCounter 的长度都不一样.
 *	scpType = 0x02 , SequenceCounter = 2 bytes;
 *	scpType = 0x03 , SequenceCounter = 3 bytes;
 *	scpType = 0x80 , SequenceCounter = 5 bytes;
 *
 *注意大小端问题
 *
 */
memref keySet_getSequenceCounter(PN8 keySetObj, u8 len)
{
	memref seqCounterField;

	seqCounterField = PFtMEM(getSeqCounterField(keySetObj));
	return seqCounterField;
}

void keySet_setSequenceCounter(PN8 keySetObj, u8 len, PR8 seqCounterField)
{
	PN8 pSeqCounterField;

	pSeqCounterField = MEMtPN8(getSeqCounterField(keySetObj));
	nvmCopyAtomic(pSeqCounterField, MEMtPM8(seqCounterField), len);
	memRefresh();
}


void keySet_incSequenceCounter(PN8 keySetObj, u8 len)
{
	PN8 seqCounterField;
	PN8 curSeqCounterField;
	u8 seqCounterByte;

	seqCounterField = MEMtPN8(getSeqCounterField(keySetObj));

	while(TRUE)
	{
		if (0 == len)
		{
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
		}

		len = U8_S_U8(len, 1);

		curSeqCounterField = PN8_A_U16(seqCounterField, len);
		seqCounterByte = memReadU8(curSeqCounterField);
		seqCounterByte = U16t8(U8_A_U8(seqCounterByte, 1));
		nvmWriteU8NoAtomic(curSeqCounterField, seqCounterByte);

		if (0 != seqCounterByte)
		{
			break;
		}
	}
	memRefresh();
}


void keySet_incTryAuthAttenpt(PN8 keySetObj)
{
	u8 tryAuth;

	tryAuth = keySet_getTryAuthAttenpt(keySetObj);

	tryAuth = U16t8(U8_A_U8(tryAuth, 1));
	nvmWriteU8Atomic(keySet_getTryAuthAttenptField(keySetObj), tryAuth);
}

u8 keySet_isCMKeyVersion(u8 keyVersion)
{
	if ((TOKEN_KEY_VERSION == keyVersion) ||
			(RECEIPT_KEY_VERSION == keyVersion) ||
			(DAP_KEY_VERSION == keyVersion) ||
			(DECIPER_LOAD_FILE_VESION == keyVersion) || 
			(CASD_KEY_VERSION == keyVersion))
	{
		return true;
	}

	return false;
}
//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
u8 keySet_getKeyType(PN8 keySetObj, u8 keyId, u8 mode)
{
	u8 format;
	u8 keyType;

	format = keySet_getStoreKeyFormmat(keySetObj);

#if GP__KEY_MANAGER__FORMAT3
	if (KEY_FORMAT_03 == format)
	{
		PN8 keyLocation;
		keyLocation = MEMtPN8(fm03_locateKeyType(keySetObj, keyId));

		if (REF_NULL == keyLocation)
		{
			if(mode == THROW_ERROR)
			{
				throw_error(SW_RECORD_NOT_FOUND);
			}
			else
			{
				return KEY_TYPE_ILLAGE;
			}

		}
		keyType = memReadU8(keyLocation);
	}
	else
#endif
	{
		keyType = getKeyType(keySetObj);
	}

	return keyType;
}

void keySet_setKeyType(PN8 keySetObj, u8 keyId, u8 keyType)
{
	u8 format;

	format = keySet_getStoreKeyFormmat(keySetObj);
#if GP__KEY_MANAGER__FORMAT3
	if (KEY_FORMAT_03 == format)
	{
		PN8 keyLocation;
		keyLocation = MEMtPN8(fm03_locateKeyType(keySetObj, keyId));

		if (REF_NULL == keyLocation)
		{
			throw_error(SW_RECORD_NOT_FOUND);
		}

		nvmWriteU32NoAtomic(keyLocation, keyType);
	}
	else
#endif
	{
		setKeyType(keySetObj, keyType);
	}
}

void keySet_ResetAllKeyCountByScpType(u8 sdId, u8 ScpType)
{
	 jref			keySetRef, keyGroupRef;
	 PN8			sdDataObject;
	 PN8			keySetHand;
	 
	 sdDataObject	= MEMtPN8(getSdDataObjBySdid(sdId));
	 keyGroupRef 	= sdData_getKeyGroupRef(sdDataObject, ScpType, FALSE);
	 if(REF_NULL == keyGroupRef)
	 {
			throw_error(SW_WRONG_DATA);
	 }
	 
	 keySetHand  = MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keyGroupRef)), keySet_INDEX));
	 while(true)
	 {
			keySetRef  = memReadU16(keySetHand);
			if (REF_NULL == keySetRef)
			{
				break;
			}
			resetSequenceCounter(keySetRef, KEY_FORMAT_02);
			sdDataObject = MEMtPN8(locateObjectInMemory(keySetRef));
			keySetHand 	 = MEMtPN8(getNextKeySetHand(sdDataObject));
	 }
}

boolean keySet_CheckIntergrated(PN8 keySetObj)
{
	if (0x07 != U8_AND_U8(0x07, keySet_getkeyAllTag(keySetObj)))// modified by zm; A better way to check whether the keySet is integrated
	{
		return false;
	}
	return true;
}

u16 keySet_GetCounter(PX32 counterAddr, u8 version)
{
	jref  tempKeyset;
	u16  sequenceCounter;

	tempKeyset		= keySet_getKeySet(MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD)), version);
	counterAddr[0]	= keySet_getSequenceCounter(MEMtPN8(locateObjectInMemory(tempKeyset)), 0x00);
	sequenceCounter	= PN_R_U16(*counterAddr);

	return sequenceCounter;
}

/*
*@function   : GetFirstFullKey
*
*@description: 获取相同SCP的keySet链上第一个拥有完整keyData的keySet
*
*@param      :PN8 sdDataObj
*@param       u8 keyVersion
*
*@return     :keySet的引用
*@history    :merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
*/
jref GetFirstFullKey(PN8 sdDataObj, u8 keyVersion)
{
	PN8 keySetHand;
	jref keyGroupRef;
	u8 scpType;
	PN8 keySetObj;
	jref keySetRef;
	u8 temKeyAllTag;


	scpType = keyGroup_getScpTypeByKeyVersion(sdDataObj, keyVersion);// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 函数名称修改
	keyGroupRef = sdData_getKeyGroupRef(sdDataObj, scpType, FALSE); //keyVersion对应scpType的keyGroupRef
	if (REF_NULL == keyGroupRef)
	{
		return REF_NULL;
	}

	keySetHand = MEMtPN8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keyGroupRef)), keySet_INDEX)); //链上第一个keySet

	while (true)
	{
		keySetRef = memReadU16(keySetHand);
		if (REF_NULL == keySetRef)
		{
			return REF_NULL;
		}

		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));

		temKeyAllTag = keySet_getkeyAllTag(keySetObj); //获取keyData完整性的tag

		if ((temKeyAllTag & 0x07) == 0x07) //完整的keyData
		{
			break;
		}
		else
		{
			keySetHand = MEMtPN8(getNextKeySetHand(keySetObj)); //指向链上下一个keySet
		}
	}

	return keySetRef;
}
#endif
