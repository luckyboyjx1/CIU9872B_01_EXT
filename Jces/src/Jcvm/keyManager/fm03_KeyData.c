/*
 * fm03_KeyData.c
 *
 *  Created on: 2013年9月29日
 *      Author: mowenpan
 */

#include "typejava.h"
#include "mask.h"
#include "cardmanager.h"
#include "error.h"
#include "object.h"
#include "ob_manager.h"
#include "Loader.h"
#include "bc_object.h"
#include "global_variable.h"

#include "keySet.h"
#include "keyData.h"
#include "KeyExtData.h"
#include "fm03_KeyData.h"
#include "casdStoreKey.h"

#if GP__CGM__FOR_MASK
	#include "CgmDel.h"
#endif
#if (GP__KEY_MANAGER && GP__KEY_MANAGER__FORMAT3)

#define KEY_INFO_DATA_TAG                                    0xC0

#define KEY_INFO_KEYCOMPONENT_LEN                            2
#define KEY_INFO_KEYCOMPONENT_LEN_FORMAT2                    4
#define KEY_INFO_HEAD                                        2

#define KEY_DATA_FIELD_OFFSET_START                          0
//format 3
#define FM03_incKeyComponentCount(keyDataAddr)               nvmWriteU8Atomic(keyDataAddr + FM03_KEY_ATTR_OFFSET ,(PX_R_U8(keyDataAddr + FM03_KEY_ATTR_OFFSET) + 1));
#define computeRemainKeyLen(remainKeyLen, keyDataLen)        (remainKeyLen >  keyDataLen ? U16_S_U16(remainKeyLen, keyDataLen) : 0)

#define KEYDATA_FIELD_FLG_INKEYDATA                          0
#define KEYDATA_FIELD_FLG_INKEYEXTDATA                       1

#define KEY_COMPONENT_TYPE_RSA_PUBLIC_START                  0xA0
#define KEY_COMPONENT_TYPE_RSA_PUBLIC_END                    0xA1

#define KEY_COMPONENT_TYPE_RSA_PRIVATE_START                 0xA2
#define KEY_COMPONENT_TYPE_RSA_PRIVATE_END                   0xA3

#define KEY_COMPONENT_TYPE_RSA_CRT_START                     0xA4
#define KEY_COMPONENT_TYPE_RSA_CRT_END                       0xA8
#define KEY_LENGTH_PADDING                                   U8_A_U8(KEY_ID_LENGTH_WIDTH , EXT_KEY_LENGTH_WIDTH)

#define KEY_INFO_HEAD_LENGTH                                 4
#define KEY_COMPONENT_INFO_HEAD_LENGTH                       2
/*
 *merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数keyId
 */
memref FM03_findKeyComponentInKeyData(PN8 keyDataAField, u16 keyDataLen, u8 keyType, u8 status)
{
	u16 offset;
	u16 tKenLen;
	u8 tKeyType;
	PN8 keyComponentLocation;

	offset = 0;
	//rKeyIdLen = RAM_READ_U16(remainKeyIdLen);
	while (offset < keyDataLen)
	{
		/*
		if (rKeyIdLen == 0)
		{
			break;
		}
		 */
		keyComponentLocation = PN8_A_U16(keyDataAField, offset);
		tKeyType = memReadU8(PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_TAG_OFFSET));
		tKenLen = memReadU16(PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_LENGTH_OFFSET));

		if ((keyType == tKeyType) || (0 == tKenLen))
		{
			if (((TAG_GET_KEY == status) || (TAG_UPDATE_KEY == status)) && (0 == tKenLen))
			{
				return MEM_NULL;
			}

			//RAM_WRITE_U16(remainKeyIdLen, rKeyIdLen);
			return PNtMEM(PN8_A_U16(keyDataAField, offset));
		}


		offset = U32t16(U16_A_U16(offset, KEY_COMPONENT_TAG_WIDTH));
		//rKeyIdLen = U16_S_U16(rKeyIdLen, KEY_COMPONENT_TAG_WIDTH);

		offset = U32t16(U16_A_U16(offset, KEY_COMPONENT_LENGTH_WIDTH));
		//rKeyIdLen = U16_S_U16(rKeyIdLen, KEY_COMPONENT_LENGTH_WIDTH);


		tKenLen = keyData_genPaddingKeyLen(tKenLen);
		offset = U32t16(U16_A_U16(offset, tKenLen));
		//rKeyIdLen = U16_S_U16(rKeyIdLen, tKenLen);
	}
	return MEM_NULL;
}


static u8 FM03_checkKeyDataField(PN8 keyDataField, u8 keyId, u8 status)
{
	u8 tmpKeyId;
	u8 flg;

	flg = false;
	tmpKeyId = memReadU8(MEMtPN8(PN8_A_U16(keyDataField, FM03_KEY_ID_OFFSET)));

	if (TAG_NEW_KEY == status)
	{
		if (0 == tmpKeyId)
		{
			flg = true;
		}
	}

	if (tmpKeyId == keyId)
	{
		flg = true;
	}

	return flg;
}

memref FM03_locateKeyId(PN8 keySetObj, u8 keyId, u8 status, PR16 nextKeyDataRef, PR16 keyFieldLen)
{
	jref keyDataRef;
	jref keyExtDataRef;
	PN8 keyDataObj;
	PN8 keyExtDataObj;
	PN8 keyDataAField;
//	u8 tmpKeyId;
	u16 remainKeyLen;
	u16 keyDataLen;

	remainKeyLen = 0;
	keyExtDataRef = REF_NULL;

	//在keyData域中查找
	keyDataRef = keySet_getKeyDataRef(keySetObj);
	if (REF_NULL != keyDataRef)
	{
		keyDataObj = MEMtPN8(locateObjectInMemory(keyDataRef));
		keyDataAField =  MEMtPN8(get_array_data(keyDataObj));
		keyDataLen = get_array_length(keyDataObj);

		keyExtDataRef = keySet_getKeyExtDataRef(keySetObj);
		//PX_W_U16(nextKeyDataRef, keyExtDataRef);
		* MEMtPR16(nextKeyDataRef) = keyExtDataRef;

		remainKeyLen = memReadU16(PN8_A_U16(keyDataAField, FM03_KEY_LENGTH_OFFSET));
//		tmpKeyId = memReadU8(PN8_A_U16(keyDataAField, FM03_KEY_ID_OFFSET));

		if (FM03_checkKeyDataField(keyDataAField, keyId, status))
		{
			//PX_W_U8(keyFieldLen, keyDataLen);
			* MEMtPR16(keyFieldLen) = keyDataLen;

			return PNtMEM(keyDataAField);
		}

		remainKeyLen = computeRemainKeyLen(remainKeyLen, keyDataLen);
	}

	//在keyExtData域中查找
	while (REF_NULL != keyExtDataRef)
	{
		keyExtDataObj = MEMtPN8(locateObjectInMemory(keyExtDataRef));
		keyDataObj = MEMtPN8(keyExtData_getKeyDataObj(keyExtDataObj));
		keyDataAField =  MEMtPN8(get_array_data(keyDataObj)); //利用变量，表示KeyExtDataField
		keyDataLen = get_array_length(keyDataObj);

		keyExtDataRef = keyExtData_getNextKeyExtDataRef(keyExtDataObj); //复用变量，表示 keyExtDataRef
		* MEMtPR16(nextKeyDataRef) = keyExtDataRef;

//		tmpKeyId = memReadU8(PN8_A_U16(keyDataAField, FM03_KEY_ID_OFFSET));

		if ((0 == remainKeyLen) && (FM03_checkKeyDataField(keyDataAField, keyId, status) == true))
		{
			* MEMtPR16(keyFieldLen) = keyDataLen;
			return PNtMEM(keyDataAField);
		}

		remainKeyLen = computeRemainKeyLen(remainKeyLen, keyDataLen);
	}

	return MEM_NULL;
}

/**
 *在 key 数据域中查找指定的 keyComponent 数据域开始地址
 *
 *参数:
 *	PN8 keyDataField  key数据域开始地址
 *	u8 keyId               密钥标识号
 *	u8 keyType             密钥类型
 *	u8 keyType
 *	u8 status              对  key 或者 keyComponent 的操作类型
 *	u16 * remainKeyIdLen
 *	jref * nextKeyData    下一个密钥数据对象的引用
 *
 *返回值：
 *	keyComponent 的数据域的开始地址
 *
 */
memref FM03_findKeyComponent(PN8 keyDataField, u16 keyDataLen, u8 keyId, u8 keyType, u16 keyIdLen, u8 status, jref nextKeyData)
{
	PN8 keyComponent;
	PN8 keyDataAField;
	PN8 keyDataObj;
	PN8 keyExtDataObj;
	jref keyDataRef;

	keyDataAField = PN8_A_U16(keyDataField, FM03_KEY_COMPONENT_START_OFFSET);
	keyDataLen = U16_S_U16(keyDataLen, FM03_KEY_HEAD_LENGTH);
	keyComponent = MEMtPN8(FM03_findKeyComponentInKeyData(keyDataAField, keyDataLen, keyType, status));// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数keyId

	if (MEM_NULL != keyComponent)
	{
		return PNtMEM(keyComponent);
	}

	keyIdLen = U16_S_U16(keyIdLen, keyDataLen);
	keyDataRef = nextKeyData;

	while (REF_NULL != keyDataRef)
	{
		if (0 == keyIdLen)
		{
			break;
		}

		keyExtDataObj = MEMtPN8(locateObjectInMemory(keyDataRef));
		keyDataObj = MEMtPN8(keyExtData_getKeyDataObj(keyExtDataObj));
		keyDataAField = MEMtPN8(get_array_data(keyDataObj));
		keyDataLen = get_array_length(keyDataObj);

		keyComponent = MEMtPN8(FM03_findKeyComponentInKeyData(keyDataAField, keyDataLen, keyType, status));//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数keyId

		if (MEM_NULL != keyComponent)
		{
			return PNtMEM(keyComponent);
		}

		keyDataRef = keyExtData_getNextKeyExtDataRef(keyExtDataObj);
	}

	return MEM_NULL;

}
/*
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉static声明
 */
u8 genKeyType(u8 keyComponentType)
{
	u8 keyType;

	if ((KEY_COMPONENT_TYPE_RSA_PUBLIC_START == keyComponentType) ||
			(KEY_COMPONENT_TYPE_RSA_PUBLIC_END == keyComponentType))
	{
		keyType = KEY_TYPE_RSA_PUBLIC;
	}
	else if ((KEY_COMPONENT_TYPE_RSA_PRIVATE_START == keyComponentType) ||
			(KEY_COMPONENT_TYPE_RSA_PRIVATE_END == keyComponentType))
	{
		keyType = KEY_TYPE_RSA_PIVATE;
	}
	else if ((KEY_COMPONENT_TYPE_RSA_CRT_START <= keyComponentType) &&
			(KEY_COMPONENT_TYPE_RSA_CRT_END >= keyComponentType))
	{
		keyType = KEY_TYPE_RSA_CRT;
	}
#if  GP__KEY_MANAGER__ECC_KEY
	else if((ECC_CurvePara_P_DGI_TYPE <= keyComponentType)&&(ECC_CurvePara_K_DGI_TYPE >= keyComponentType))
	{
		keyType = ECC_KEYPARAREF_DGI_TYPE;
	}
#endif
	else
	{  //TODO:set keyType
		//keyType = KEY_TYPE_SYMMETRIC;
		keyType = keyComponentType;
	}

	return keyType;
}
//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
//keyLen --- 原始的密钥长度，没有经过填充和安全计算的
/**
 * modify by xuxiaojun@20221208: 缺陷413，SCP80 三支key长度不一致导致profile下载失败，所以将SCP80的存储方案由format2修改为format3。 该函数中keyNum和keyAllSet需要同步修改
 */
memref FM03_prepareKeyComponent(PN8 keySetObj, u8 keyId, u8 keyType, PR16 keyLen, u8 status)
{
	u16 keyDataLen;
	u16 tKeyLen;
	jref nextKeyDataRef;
	PN8 keyLocation;
	PN8 keyComponentLocation;
	PN8 keyComonentLenLocation;
	u8	allKeyTag;
	u8 KeyVersion;

	nextKeyDataRef = REF_NULL;
	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, status, &nextKeyDataRef, &keyDataLen));

	if (MEM_NULL == keyLocation)
	{
		return MEM_NULL;
	}


	//更新 key 信息
	tKeyLen = memReadU16(PN8_A_U16(keyLocation, FM03_KEY_LENGTH_OFFSET)); //获取的是从key id到结束所有数据的总长度
	if (TAG_NEW_KEY == status)
	{
		u8 keyAttr;
		PN8 keyAttrLocation = PN8_A_U16(keyLocation, FM03_KEY_ATTR_OFFSET);

		if (tKeyLen == 0)
		{
			nvmWriteU8Atomic(PN8_A_U16(keyLocation, FM03_KEY_ID_OFFSET), keyId);

			//tKeyLen = SO_calSecDataAreaLen(tKeyLen);
			tKeyLen = U32t16(U16_A_U16(tKeyLen, KEY_ATTR_LENGTH)); //attr
			tKeyLen = U32t16(U16_A_U16(tKeyLen, FM03_KEY_ID_TYPE_LENGTH)); //keyType
			tKeyLen = U32t16(U16_A_U16(tKeyLen, KEY_ACCESS_LENGTH)); //access
			tKeyLen = U32t16(U16_A_U16(tKeyLen, KEY_USAGE_QUALIFIER_LENGTH));

			keySet_incKeyNum(keySetObj);

			KeyVersion = keySet_getKeyVersion(keySetObj);

			if(IsSCP80Key(KeyVersion))
			{
				allKeyTag	= keySet_getkeyAllTag(keySetObj);//modify by zhxx@20220805,scp80 key安装03格式进行存储时，也要将allKeyTag置上，否则将3支key存储完，不会切换到0x0F状态
				allKeyTag   = U8_OR_U8(allKeyTag, U8_SHL_U8(1, U8_S_U8(keyId, 1)));//缺陷413(SCP80 三支key长度不一致导致profile下载失败)修改后导致TC_8_2_7_3_Check_all_elements_in_ApplicationInstance测试不过
				keySet_SetkeyAllTag(keySetObj, allKeyTag);
			}

			keyAttr = KEY_ATTR_DEFAULT_VALUE;
			
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		if (0x80 == keyType)
		{
			setKeyType(keySetObj, keyType);
		}
#endif
			
			//keyType
			nvmWriteU8Atomic(PN8_A_U16(keyLocation, FM03_KEY_TYPE_OFFSET), genKeyType(keyType));
		}
		else
		{
			u8 oKeyType;
			
			keyAttr = memReadU8(keyAttrLocation);
			U8_INC(keyAttr);
			//keyType
			oKeyType = keySet_getKeyType(keySetObj, keyId,THROW_ERROR);

			if ((oKeyType == KEY_TYPE_RSA_PUBLIC) && (KEY_TYPE_RSA_PIVATE == genKeyType(keyType)))
			{
				nvmWriteU8Atomic(PN8_A_U16(keyLocation, FM03_KEY_TYPE_OFFSET), KEY_TYPE_RSA_PIVATE);
			}
		}

		nvmWriteU8Atomic(MEMtPN8(keyAttrLocation), keyAttr);

	//	tKeyLen = SO_calSecDataAreaLen(tKeyLen);
		tKeyLen = U32t16(U16_A_U16(tKeyLen, KEY_COMPONENT_TAG_WIDTH)); //keyType
		tKeyLen = U32t16(U16_A_U16(tKeyLen, KEY_COMPONENT_LENGTH_WIDTH)); //length
		tKeyLen = U32t16(U16_A_U16(tKeyLen, keyData_genPaddingKeyLen(SO_calSecDataAreaLen(keyLen[0]))));//TODO: 没有使用标准接口

		nvmWriteU16Atomic(PN8_A_U16(keyLocation, FM03_KEY_LENGTH_OFFSET), tKeyLen);
	}

	keyComponentLocation = MEMtPN8(FM03_findKeyComponent(keyLocation, keyDataLen, keyId, keyType, tKeyLen, status, nextKeyDataRef));

	if (MEM_NULL == keyComponentLocation)
	{
		keyLen[0] = 0;
		return MEM_NULL;
	}

	keyComonentLenLocation = PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_LENGTH_OFFSET);
	if (TAG_NEW_KEY == status)
	{
		nvmWriteU8Atomic(PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_TAG_OFFSET), keyType); //写keyComponentType
		//nvmWriteU16Atomic(MEMtPN8(keyComonentLenLocation), SO_calSecDataAreaLen(* MEMtPR16(keyLen))); //写keyLen //TODO: 没有使用标准接口
		nvmWriteU16Atomic(MEMtPN8(keyComonentLenLocation), SO_calSecDataAreaLen(keyLen[0])); //写keyLen //TODO: 没有使用标准接口
	}
	else if(TAG_UPDATE_KEY == status)
	{
		if(memReadU16(MEMtPN8(keyComonentLenLocation)) != SO_calSecDataAreaLen(keyLen[0]))// modify by xuxiaojun@20220628 更新密钥时，长度要和之前的保持一致 SCP11测试脚本：error_update_scp11c_key_length_Test_isd_01 / error_delete_scp11c_key_length_Test_isd_01
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}

//对于FM03，更新key时，如果把key type修改了，则直接在前面FM03_prepareKeyComponent时就返回了MEM_NULL，不会走到这里来 . modify by xuxiaojun@20220725 更新密钥时，key type不能被改变 gp221: When replacing keys, the new keys shall be presented to the card in the same format as they are already present on the card
	}
	else if(TAG_GET_KEY == status)
	{
		keyLen[0] = PN_R_U16(keyComonentLenLocation);
	}

	return PNtMEM(PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_DATA_OFFSET));
}

/**
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 * modify by xuxiaojun@20221208: 缺陷413，SCP80 三支key长度不一致导致profile下载失败，所以将SCP80的存储方案由format2修改为format3。 该函数中keyNum和keyAllSet需要同步修改
 */
void FM03_deleteSingleScKey(PN8 keySetObj, u8 keyId)
{
	PN8 keyDataField;
	PN8 keyExtDataField;
	PN8 keyDataHand;
	PN8 keyExtDataHand;
	PN8 keyDataObj;
	PN8 keyExtDataObj;
	u8 sKeyId;
	u8 isDelete;
	u16 keyDataLength;
	u16 sKeyLen;
	jref keyDataRef;
	jref keyExtDataRef;
	u8	allKeyTag;
	u8  keyVersion;


	//查找密钥是否存在于keyData
	isDelete = false;
	keyDataHand = MEMtPN8(keySet_getKeyDataField(keySetObj));
	keyDataRef = PN_R_U16(keyDataHand);
	sKeyLen = 0;
	sKeyId  = 0;
	
	if (REF_NULL != keyDataRef)
	{
		keyDataObj = MEMtPN8(locateObjectInMemory(keyDataRef));
		keyDataField = MEMtPN8(get_array_data(keyDataObj));
		keyDataLength = get_array_length(keyDataObj);

		sKeyId = memReadU8(PN8_A_U16(keyDataField, FM03_KEY_ID_OFFSET));
		sKeyLen = memReadU16(PN8_A_U16(keyDataField, FM03_KEY_LENGTH_OFFSET));

		if (keyId == sKeyId) //找到删除的密钥
		{
				#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK 
					if(0x80 == keySet_getKeyType(keySetObj, keyId, THROW_ERROR))
					{/*modified by zm; must inc DES Cont before it is delete*/
							sdData_incDESKeyCount(getCurrentSdDataObj(IS_NO_NEED_CHECK_SD));
					}
				#endif
//#if GP__CGM__FOR_MASK
//					cgmDel_Key(keySetObj);
//#endif
			nvmWriteU16Atomic(MEMtPN8(keyDataHand), REF_NULL);
			isDelete = true;
		}

		sKeyLen = U16_S_U16(sKeyLen, U16_S_U16(keyDataLength, KEY_LENGTH_PADDING));
	}


	//查找密钥是否存放于keyExtData中
	keyExtDataHand = MEMtPN8(keySet_getKeyExtDataField(keySetObj));
	keyExtDataRef = memReadU16(keyExtDataHand);

	while(isDelete == false)
	{
		if (REF_NULL == keyExtDataRef)
		{
			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
		}

		keyExtDataObj = MEMtPN8(locateObjectInMemory(keyExtDataRef));
		keyDataObj = MEMtPN8(keyExtData_getKeyDataObj(keyExtDataObj));
		keyDataField = MEMtPN8(get_array_data(keyDataObj));
		keyDataLength = get_array_length(keyDataObj);


		if (0 == sKeyLen)
		{
			sKeyId = memReadU8(PN8_A_U16(keyDataField, FM03_KEY_ID_OFFSET));
			sKeyLen = memReadU16(PN8_A_U16(keyDataField, FM03_KEY_LENGTH_OFFSET));

			sKeyLen = U16_S_U16(sKeyLen, U16_S_U16(keyDataLength, KEY_LENGTH_PADDING));
		}
		else
		{
			sKeyLen = U16_S_U16(sKeyLen, keyDataLength);
		}

		keyExtDataField = MEMtPN8(keyExtData_getNextKeyExtDataField(keyExtDataObj));
		keyExtDataRef = memReadU16(keyExtDataField);
		//sKeyLen = U16_S_U16(sKeyLen, keyDataLength);

		if (0 == sKeyLen)
		{
			if (keyId == sKeyId) //找到删除的密钥
			{
				#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK 
					if(0x80 == keySet_getKeyType(keySetObj, keyId,THROW_ERROR))
					{/*modified by zm; must inc DES Cont before it is delete*/
							sdData_incDESKeyCount(getCurrentSdDataObj(IS_NO_NEED_CHECK_SD));
					}
				#endif

//#if GP__CGM__FOR_MASK
//					cgmDel_Key(keyExtDataObj);
//#endif
				memWriteU16Atomic(MEMtPN8(keyExtDataHand), keyExtDataRef);
				isDelete = true;
				break;
			}

			keyExtDataHand = keyExtDataField;
		}
	}

	if (true == isDelete)
	{
		keyVersion = keySet_getKeyVersion(keySetObj);

		if(IsSCP80Key(keyVersion))
		{
			if((keyId == KEY_ID_01) || (keyId == KEY_ID_02) || (keyId == KEY_ID_03))
			{
				allKeyTag	= keySet_getkeyAllTag(keySetObj);
				allKeyTag	=	U8_AND_U8(allKeyTag, U8_INV(U8_SHL_U8(1, U8_S_U8(keyId, 1))));
				keySet_SetkeyAllTag(keySetObj, allKeyTag);
			}
		}

		keySet_desKeyNum(keySetObj);

		bRamBuffer[GC_Requested] = 1;
/*
#if CMS2AC
		if(0x80 == keySet_getKeyType(keySetObj, keyId,THROW_ERROR))
		{
				sdData_incDESKeyCount(getCurrentSdDataObj(IS_NO_NEED_CHECK_SD));
		}
#endif
*/
	}
}

void FM03_setKeyAccess(PN8 keySetObj, u8 keyId, u8 keyAccess)
{
	PN8 keyLocation;
	u16 KeyLen;
	jref nextKeyDataRef;

	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, TAG_GET_KEY, &nextKeyDataRef, &KeyLen));
	nvmWriteU8Atomic(PN8_A_U16(keyLocation, FM03_KEY_ACCESS_OFFSET), keyAccess);
}

u8 FM03_getKeyAccess(PN8 keySetObj, u8 keyId)
{
	PN8 keyLocation;
	u16 KeyLen;
	jref nextKeyData;

	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, TAG_GET_KEY, & nextKeyData, &KeyLen));
	return memReadU8(PN8_A_U16(keyLocation, FM03_KEY_ACCESS_OFFSET));
}
/*
*@description:
*
*@see        :
*
*@function   :FM03_setKeyUsageQualifier
*
*@param      :PN8 keyDataField --- input
*@param      :u8 keyId --- input
*@param      :PR8 keyUsage --- input, LV, 总长度为KEY_USAGE_QUALIFIER_LENGTH
*
*@return     :void
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
*            :modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
void FM03_setKeyUsageQualifier(PN8 keySetObj, u8 keyId , PR8 keyUsageQualifier)
{
	PN8 keyLocation;
	u16 KeyLen;
	jref nextKeyData;

	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, TAG_GET_KEY, & nextKeyData, &KeyLen));
	nvmCopyAtomic(PN8_A_U16(keyLocation, FM03_KEY_USAGE_QUALIFIER_OFFSET), keyUsageQualifier, KEY_USAGE_QUALIFIER_LENGTH);//merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
}
/*
*@description:
*
*@see        :
*
*@function   :FM03_getKeyUsageQualifier
*
*@param      :PN8 keyDataField --- input
*@param      :u8 keyId         --- input
*@param      :PR8 keyUsageQualifier --- output
*
*@return     :keyUsageQualifier中有效数据的长度
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
*            :modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
u8 FM03_getKeyUsageQualifier(PN8 keySetObj, u8 keyId, PR8 keyUsageQualifier)
{
	PN8 keyLocation;
	u16 KeyLen;
	jref nextKeyData;
	u8 len;

	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, TAG_GET_KEY, & nextKeyData, &KeyLen));
	keyLocation = PN8_A_U16(keyLocation, FM03_KEY_USAGE_QUALIFIER_OFFSET);
	len = memReadU8(keyLocation);
	memCopyToRam(keyUsageQualifier, PM8_A_U8(keyLocation, 0x01), len);

	return len;//merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
}

u8 FM03_getKeyAttr(PN8 keySetObj, u8 keyId)
{
	u8	 AttrValue;
	jref nextKeyDataRef;
	u16 keyFieldLen;
	PN8 keyLocation;

	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, TAG_GET_KEY, &nextKeyDataRef, &keyFieldLen));
	if(MEM_NULL == keyLocation)
	{
			AttrValue = 0;
	}else{
			keyLocation = PN8_A_U16(keyLocation, FM03_KEY_ATTR_OFFSET); //复用变更，表示 keyAttr 位置
			AttrValue   = memReadU8(keyLocation);
	}

	return AttrValue;
}


static memref getNextKeyExtData(PR16 nextKeyExtDataRef, PR16 keyDataLen)
{
	PN8 keyExtDataObj;
	PN8 keyDataObj;

	keyExtDataObj = MEMtPN8(locateObjectInMemory(*MEMtPR16(nextKeyExtDataRef)));
	keyDataObj = MEMtPN8(keyExtData_getKeyDataObj(keyExtDataObj));

	* MEMtPR16(nextKeyExtDataRef) = keyExtData_getNextKeyExtDataRef(keyExtDataObj);
	* MEMtPR16(keyDataLen)        = get_array_length(keyDataObj);
	return get_array_data(keyDataObj);
}
/*
 * merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
 * modify by xuxiaojun@20220526 获取ECC公钥和私钥时，如果F0是标准的，则需要把对应的值给返回.SCP11测试脚本:  C_sm_PSO_SUCCESS_PK_OCE_ECKA_temporarily_stored_256_a1_30_70
 */
u16 FM03_writeKeyInfo(PX8 buf, u16 maxSingleLength, PN8 keySetObj, u16 offset)
{
	u16 keyLen;
	u16 keyCounter;
	u16 keyDataFieldLen;
	u16 keyDataFieldOffset;
	u8 keyVersion;
	u8 keyAttr;
	PN8 keyDataObj;
	PN8 keyDataAField;
	PN8 keyComponentLocation;
	jref keyDataRef;
	jref nextKeyExtDataRef;
	u8 keyAccess;
	u8 keyAccessLen;
	u8 keyUsageQualifier[2];
	u8 keyUsageQualifierLen;
	u8 keyId;
	u8 format;
	u16 keyInfoLen;
	u16 keyLenTmp;
#if GP__KEY_MANAGER__ECC_KEY
	u8 keyType;
	u16 offsetBak;
	u16 eccKeyParaRefValue;
#endif


	format = keySet_getAPDUKeyFormmat(keySetObj);
	keyVersion = keySet_getKeyVersion(keySetObj);
	keyDataRef = keySet_getKeyDataRef(keySetObj);
	nextKeyExtDataRef = keySet_getKeyExtDataRef(keySetObj);
	if (REF_NULL != keyDataRef)
	{
		keyDataObj = MEMtPN8(locateObjectInMemory(keyDataRef));
		keyDataAField = MEMtPN8(get_array_data(keyDataObj));
		keyDataFieldLen = get_array_length(keyDataObj);
	}
	else
	{
		keyDataAField = MEMtPN8(getNextKeyExtData(MEMtPR16(&nextKeyExtDataRef), &keyDataFieldLen));
	}
	keyDataFieldOffset = KEY_DATA_FIELD_OFFSET_START;

	while (true)
	{
		if (U16_A_U16(offset, KEY_INFO_HEAD_LENGTH) >= maxSingleLength)
		{
			return 0;
		}

		if (0 == keyDataFieldLen)
		{
			break;
		}

		keyId = memReadU8(PN8_A_U16(keyDataAField, FM03_KEY_ID_OFFSET));
		keyAttr = memReadU8(PN8_A_U16(keyDataAField, FM03_KEY_ATTR_OFFSET));
		keyLen = memReadU16(PN8_A_U16(keyDataAField, FM03_KEY_LENGTH_OFFSET));

		buf[offset] = KEY_INFO_DATA_TAG; //key info
		U16_INC(offset);

		keyAttr = U8_AND_U8(keyAttr, KEY_COUNT_MASH);
		if (KEY_FORMAT_APDU_2 == format)
		{
			keyInfoLen = U8t16(U8_M_U8(keyAttr, U8_A_U8(KEY_INFO_KEYCOMPONENT_LEN_FORMAT2, KEY_INFO_HEAD)));
			keyInfoLen = U32t16(U16_A_U8(keyInfoLen, 4));
		}
		else
		{
			keyInfoLen = U32t16(U16_A_U8(U8_M_U8(keyAttr, KEY_INFO_KEYCOMPONENT_LEN), KEY_INFO_HEAD));
		}
		
		buf[offset] = U16t8(keyInfoLen);

#if GP__KEY_MANAGER__ECC_KEY
		offsetBak = offset;
#endif
		U16_INC(offset);


		buf[offset] = memReadU8(PN8_A_U16(keyDataAField, FM03_KEY_ID_OFFSET));
		U16_INC(offset);

		buf[offset] = keyVersion;
		U16_INC(offset);


		//key component
		//keyDataFieldOffset = U16_A_U16(keyDataFieldOffset, FM03_KEY_COMPONENT_START_OFFSET);
		keyComponentLocation = PN8_A_U16(keyDataAField, FM03_KEY_COMPONENT_START_OFFSET);
		keyCounter = U8_A_U8(U16t8(U8_A_U8(U16t8(U8_A_U8(KEY_ATTR_LENGTH, FM03_KEY_ID_TYPE_LENGTH)), KEY_ACCESS_LENGTH)), KEY_USAGE_QUALIFIER_LENGTH);
		keyDataFieldLen = U16_S_U16(keyDataFieldLen, FM03_KEY_HEAD_LENGTH);

		while (keyCounter < keyLen)
		{
			u16 keyComponentLen;

			if (U16_A_U16(offset, KEY_COMPONENT_INFO_HEAD_LENGTH) >= maxSingleLength)
			{
				return 0;
			}

			if (keyDataFieldOffset >= keyDataFieldLen)
			{
				keyComponentLocation = MEMtPN8(getNextKeyExtData(MEMtPR16(&nextKeyExtDataRef), &keyDataFieldLen));
				keyDataFieldOffset = KEY_DATA_FIELD_OFFSET_START;
			}

			if (KEY_FORMAT_APDU_2 == format)
			{
				buf[offset] = 0xFF;
				U16_INC(offset);
			}
			buf[offset] = memReadU8(PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_TAG_OFFSET));

#if GP__KEY_MANAGER__ECC_KEY
	        keyType = buf[offset];
#endif
			U16_INC(offset);

			keyComponentLen = memReadU16(PN8_A_U16(keyComponentLocation, FM03_KEY_COMPONENT_LENGTH_OFFSET));

			keyLenTmp = SECURE_HEAD_BYTE_RESET(keyComponentLen);

#if GP__KEY_MANAGER__ECC_KEY
			if(keyType == ECC_PublicKey_DGI_TYPE)// modify by xuxiaojun@20220526:公钥非压缩格式带04，需要把04这个字节给去掉。私钥的话，没有这个字节，所以不能减1
			{
				keyLenTmp--;  // For key type 'B0' (ECC Public Key), the length of the component shall have one of the following values, excluding coding identifier '04'
			}
			else if(keyType == ECC_KEYPARAREF_DGI_TYPE)
			{
				keyLenTmp = keySet_getEccKeyParaRefValue(keySetObj); // For key type 'F0' (Key Parameter Reference), the length of the component is replaced by the value of the Key Parameter Reference (coded on 1 or 2 bytes).
			}
#endif
			if (KEY_FORMAT_APDU_2 == format)
			{
				PX_W_U16(&buf[offset], keyLenTmp);
				offset = U16_A_U8(offset, 2);
			}
#if GP__KEY_MANAGER__ECC_KEY
			else if(keyLenTmp > 0x7FFF) // keyType == ECC_KEYPARAREF_DGI_TYPE时，keyLenTmp可能是2字节，也可能是1字节
			{
				PX_W_U16(&buf[offset], keyLenTmp);
				offset = U16_A_U8(offset, 2);
			}
#endif
			else
			{
				buf[offset] = U16t8(keyLenTmp); // ECC KEY的F0类型时，如果keyLenTmp是两字节，这里会有问题的
				U16_INC(offset);
			}
			keyComponentLen = keyData_genPaddingKeyLen(keyComponentLen);
			

			keyComponentLen = U32t16(U16_A_U8(keyComponentLen, KEY_COMPONENT_HEAD_LENGTH));
			keyCounter = U32t16(U16_A_U16(keyCounter, keyComponentLen));
			keyDataFieldOffset = U32t16(U16_A_U16(keyDataFieldOffset, keyComponentLen));
			keyComponentLocation = PN8_A_U16(keyComponentLocation, keyComponentLen);

#if GP__KEY_MANAGER__ECC_KEY
			if((keyType == ECC_PublicKey_DGI_TYPE) || (keyType == ECC_PrivateKey_DGI_TYPE)) //modify by xuxiaojun@20220526  获取ECC公钥和私钥时，如果F0是标准的，则需要把对应的值给返回.SCP11测试脚本:  C_sm_PSO_SUCCESS_PK_OCE_ECKA_temporarily_stored_256_a1_30_70
			{
				if(keySet_getEccKeyParaRefValidTag(keySetObj))
				{
					eccKeyParaRefValue = keySet_getEccKeyParaRefValue(keySetObj);

					if(IsEccKeyNorParaRefVal(eccKeyParaRefValue))
					{
						buf[offset] = 0xF0; // f the key references preloaded ECC Curve Parameters (see section B.4.1), information about key type 'F0' (Key Parameter Reference) shall be provided (see below) but no information about individual Curve Parameters shall be present
						U16_INC(offset);
						buf[offset] = eccKeyParaRefValue;
						U16_INC(offset);
					}
					else
					{
						// If the key was loaded together with its own specific Curve Parameters, information about Curve Parameters shall not be present.
					}
				}
				else
				{
					break; // while (keyCounter < keyLen) 什么情况下会走到这里来？
				}
			}
			buf[offsetBak] = offset - offsetBak - 1;
#endif
		} // while (keyCounter < keyLen)

		
		if (KEY_FORMAT_APDU_2 == keySet_getAPDUKeyFormmat(keySetObj))
		{
			//keyAccess
			keyAccessLen = keyData_getKeyAccess(keySetObj, keyId, &keyAccess);
			buf[offset] = keyAccessLen;
			U16_INC(offset);
			buf[offset] = keyAccess;
			U16_INC(offset);
			
			//key Usage 
			keyUsageQualifierLen = keyData_getKeyUsageQualifier(keySetObj, keyId, &keyUsageQualifier[0]);
			buf[offset] = keyUsageQualifierLen;
			U16_INC(offset);
			buf[offset] = keyUsageQualifier[0];
			U16_INC(offset);
			buf[offset] = keyUsageQualifier[1];
			U16_INC(offset);
		}
		
		if (REF_NULL == nextKeyExtDataRef)
		{
			break;
		}

		keyDataAField = MEMtPN8(getNextKeyExtData(MEMtPR16(&nextKeyExtDataRef), &keyDataFieldLen));
		keyDataFieldOffset = KEY_DATA_FIELD_OFFSET_START;
	}

	if (offset >= maxSingleLength)
	{
		return 0;
	}
	return U16_INC(offset);
}

memref fm03_locateKeyType(PN8 keySetObj, u8 keyId)
{
	jref nextKeyDataRef;
	u16 keyLen;
	PN8 keyLocation;

	keyLocation = MEMtPN8(FM03_locateKeyId(keySetObj, keyId, TAG_GET_KEY, &nextKeyDataRef, &keyLen));
	if (MEM_NULL == keyLocation)
	{
		return MEM_NULL;
	}

	keyLocation = PN8_A_U16(keyLocation, FM03_KEY_TYPE_OFFSET);
	return PNtMEM(keyLocation);
}
#endif
