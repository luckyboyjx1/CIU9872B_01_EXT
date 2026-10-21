/*
 * keyData.c
 *
 *  Created on: 2013年10月8日
 *      Author: mowenpan
 */

#include "typejava.h"
#include "mask.h"
#include "cardmanager.h"
#include "keySet.h"
#include "object.h"
#include "error.h"
#include "bc_array.h"
#include "fm01_keyData.h"
#include "fm02_keyData.h"
#include "fm03_keyData.h"
#include "keyData.h"
#include "KeyExtData.h"

#if GP__KEY_MANAGER

#define CMKEY_ID               1
#define DES_KEY_TYPE_MASK      0x80
#define RSA_KEY_TYPE_MASK      0xA0
/*
 * modified by xuxiaojun@20210414 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 */
jref keyData_newKeyData(PN8 keySetObj, u16 len)
{
	jref keyDataRef;
	memref keyDataHand;

	keyDataHand = PFtMEM(get_object_field_ref(keySetObj, keyData_INDEX));

	keyDataRef = _newarrayObject(AType_Byte, len, BIT_NEED_REFRESH);
	memWriteU16Atomic(MEMtPN8(keyDataHand), keyDataRef);

	return keyDataRef;
}

u16 keyData_genPaddingKeyLen(u16 keyLen)
{
	u16		nKeyLen;
	u16		keyMode;

	keyMode = U16_R_U8(keyLen , KEY_UNIT_LENGTH);
	if (keyMode != 0)
	{
		nKeyLen = U32t16(U16_A_U16(keyLen, U16_S_U16(KEY_UNIT_LENGTH, keyMode)));
	}
	else
	{
		nKeyLen = keyLen;
	}

	return nKeyLen;
}
/*
 *merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数:KeyAccessOffest
 */
void keyData_setKeyAccess(PN8 keySetObj, u8 keyId, PR8 keyAccess)
{
	u8 keyFormat;
	PN8 keyDataAField;

	keyFormat = keySet_getStoreKeyFormmat(keySetObj);
	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));

	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			FM01_setKeyAccess(keyDataAField, keyId, PX_R_U8(keyAccess));
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			FM02_setKeyAccess(keyDataAField, keyId, PX_R_U8(keyAccess));
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			FM03_setKeyAccess(keySetObj, keyId, PX_R_U8(keyAccess));
			break;
#endif

		default:
			throw_error(SW_WRONG_DATA);
			break;
	}
}

u8 keyData_getKeyAccess(PN8 keySetObj, u8 keyId, PR8 keyAccess)
{
	u8 keyFormat;
	PN8 keyDataAField;

	keyFormat = keySet_getStoreKeyFormmat(keySetObj);
	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));

	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			PX_W_U8(keyAccess, FM01_getKeyAccess(keyDataAField, keyId));
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			PX_W_U8(keyAccess, FM02_getKeyAccess(keyDataAField, keyId));
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			PX_W_U8(keyAccess, FM03_getKeyAccess(keySetObj, keyId));
			break;
#endif

		default:
			throw_error(SW_WRONG_DATA);
			break;
	}

	return KEY_ACCESS_LENGTH;
}
/*
*@description:
*
*@see        :
*
*@function   :keyData_setKeyUsageQualifier
*
*@param      :PN8 keySetObj --- input
*@param      :u8  keyId --- input
*@param      :PR8 keyUsageQualifier --- input, LV, 总长度为KEY_USAGE_QUALIFIER_LENGTH
*
*@return     :void
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
*            :modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
void keyData_setKeyUsageQualifier(PN8 keySetObj, u8 keyId, PR8 keyUsageQualifier)
{
	u8 keyFormat;
	PN8 keyDataAField;

	keyFormat = keySet_getStoreKeyFormmat(keySetObj);
	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));

	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			FM01_setKeyUsageQualifier(keyDataAField, keyId, PX_R_U8(keyUsageQualifier));
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			FM02_setKeyUsageQualifier(keyDataAField, keyId, keyUsageQualifier);//merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			FM03_setKeyUsageQualifier(keySetObj, keyId, keyUsageQualifier);//merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
			break;
#endif

		default:
			throw_error(SW_WRONG_DATA);
			break;
	}
}
/*
*@Function: keyData_getKeyUsageQualifier
*
*@Descrip : 获取keySetObj指定keyId的key Usage Qualifier
*
*@See     :
*
*@param	  : PN8 keySetObj --- input
*@param	  : u8 keyId      --- input
*@param	  : PR8 keyUsageQualifier --- output
*
*@Return  : u8 --- keyUsageQualifier中有效数据长度
*
*@History : merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 兼容keyUsage两字节
*         : modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
u8 keyData_getKeyUsageQualifier(PN8 keySetObj, u8 keyId, PR8 keyUsageQualifier)
{
	u8 keyFormat;
	PN8 keyDataAField;
	u8 len;

	keyFormat = keySet_getStoreKeyFormmat(keySetObj);
	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));
	len = 0x00;

	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			PX_W_U8(keyUsageQualifier, FM01_getKeyUsageQualifier(keyDataAField, keyId));
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			len = FM02_getKeyUsageQualifier(keyDataAField, keyId, keyUsageQualifier);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			len = FM03_getKeyUsageQualifier(keySetObj, keyId, keyUsageQualifier);
			break;
#endif

		default:
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
			break;
	}

	return len;
}

//format, isNekKey, keylen
u16 keyData_computeKeyDataLen(u8 format, u16 curKeyDataLen, u16 keyLen, u8 isNewKey)
{
	if (KEY_FORMAT_03 == U8_AND_U8(format , KEY_FORMAT_STORE_MASH))
	{
		if (true == isNewKey)
		{
			curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_ID_LENGTH_WIDTH));
			curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_DATA_LENGTH_WIDTH));
			curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_ATTR_LENGTH));
#if GP__KEY_MANAGER__FORMAT3			
			curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, FM03_KEY_ID_TYPE_LENGTH));
#endif
			curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_ACCESS_LENGTH));
			curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_USAGE_QUALIFIER_LENGTH));
			//curKeyDataLen = (curKeyDataLen + KEY_ID_LENGTH_WIDTH + KEY_DATA_LENGTH_WIDTH + KEY_ATTR_LENGTH + KEY_ACCESS_LENGTH + KEY_USAGE_QUALIFIER_LENGTH);
		}

		curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_COMPONENT_TAG_WIDTH));//merged from EPX\v3.0\DEVELOP：书写更具有含义，而不是用一个值相同但是含义不同的宏定义
		curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, KEY_COMPONENT_LENGTH_WIDTH));
		curKeyDataLen = U32t16(U16_A_U16(curKeyDataLen, keyData_genPaddingKeyLen(SO_calSecDataAreaLen(keyLen))));
		//curKeyDataLen = (curKeyDataLen + KEY_TYPE_LENGTH_WIDTH + EXT_KEY_LENGTH_WIDTH + genPaddingKeyLen(keyLen));
	}

	return curKeyDataLen;
}

u8 keyData_getKeyAttr(PN8 keySetObj, u8 keyId)
{
	u8 format;
	u8 keyAttr;
	PN8 keyDataAField;

	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));
	format = keySet_getStoreKeyFormmat(keySetObj);

	keyAttr = 0;
	switch (format)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			keyAttr = FM01_getKeyAttr(keyDataAField, keyId);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			keyAttr = FM02_getKeyAttr(keyDataAField, keyId);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			keyAttr = FM03_getKeyAttr(keySetObj, keyId);
			break;
#endif

		default:
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
			break;
	}

	return keyAttr;
}

memref keyData_prepareKeyData(PN8 keySetObj, u8 status, u8 keyId, u8 keyType, u16 keyLen)
{
	u8 			keyFormat;
	u16 		pKeyLen;
	memref 		keyDataAField;

	keyFormat = keySet_getStoreKeyFormmat(keySetObj);
	pKeyLen = keyLen;
	
	keyDataAField = MEM_NULL;

	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			keyDataAField = FM01_prepareKeyData(keySetObj, keyId, &pKeyLen, status, keyType);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			keyDataAField = FM02_prepareKeyData(keySetObj, keyId, &pKeyLen, status, keyType);//对于FM02，只要key version存在，这里返回的keyDataAField就不会是MEM_NULL
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			keyDataAField = FM03_prepareKeyComponent(keySetObj, keyId, keyType, &pKeyLen ,status);//对于FM03，如果keyId和keyType不匹配，这里返回的keyDataAField是MEM_NULL
			break;
#endif

		default:
			throw_error(SW_WRONG_DATA);
			break;
	}

	return keyDataAField;
}

/**
 *返回值为密钥长度
 */
u16 keyData_getKeyDataById(PN8 keySetObj, u8 keyId, u8 keyType, PX8 keyDataAField)
{
	u8 				keyFormat;
	u16 			keyLen ;
	memref 			tKeyDataAField;
//	memref			tempBuffer;

	tKeyDataAField	=	MEM_NULL;
	keyLen			=	0x00;
	keyFormat 		= 	keySet_getStoreKeyFormmat(keySetObj);

	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			tKeyDataAField = FM01_prepareKeyData(keySetObj, keyId, &keyLen, TAG_GET_KEY, keyType);
			break;
#endif
		
#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			tKeyDataAField = FM02_prepareKeyData(keySetObj, keyId, &keyLen, TAG_GET_KEY, keyType);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			tKeyDataAField = FM03_prepareKeyComponent(keySetObj, keyId, keyType, &keyLen  ,TAG_GET_KEY);
			break;
#endif
		
		default:
			if( bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA )
			{
				return 0;
			}
			throw_error(SW_WRONG_DATA);
			break;
	}

	if(tKeyDataAField == MEM_NULL) // merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP 增加tKeyDataAField为MEM_NULL的判断
	{
		return 0;
	}

	keyLen	= SECURE_HEAD_BYTE_RESET(keyLen);

	//memCopyToRam(cacheBuffer, MEMtPM8(SECURE_HEAD_BYTE_OFFEST(tKeyDataAField)), keyLen);

	/*SO_getSecDataByAddr( cacheBuffer, 0, keyLen, MEMtPM8(tKeyDataAField));
	keyDataAField[0]	= PXtMEM(cacheBuffer);*/
	//tempBuffer	= PXtMEM(sysConfig_GetTempArrBuffer(keyLen));/*U32_A_U16(MemGetReuseRam_GetEEpBuffer(keyLen), keyLen)*/;
	if(keyDataAField != NULL)
	{
		SO_getSecDataByAddr( MEMtPX8(keyDataAField), 0, keyLen, MEMtPM8(tKeyDataAField));
	}
//	keyDataAField[0] 		 = tempBuffer;
	return keyLen;
}

u16 getKeyDataByIdforSCP80(PN8 keySetObj, u8 keyId, u8 keyType, PR32 keyDataAField)
{
	u8 			keyFormat;
	u16 		keyLen;
	memref 		tKeyDataAField;
	memref		tempBuffer;

	keyFormat 	= keySet_getStoreKeyFormmat(keySetObj);
	switch (keyFormat)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			tKeyDataAField = FM01_prepareKeyData(keySetObj, keyId, &keyLen, TAG_GET_KEY, keyType);
			break;
#endif
		
#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
			tKeyDataAField = FM02_prepareKeyData(keySetObj, keyId, &keyLen, TAG_GET_KEY, keyType);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			tKeyDataAField = FM03_prepareKeyComponent(keySetObj, keyId, keyType, &keyLen  ,TAG_GET_KEY);
			break;
#endif
		
		default:			
				return 0;		
	}

	keyLen		= SECURE_HEAD_BYTE_RESET(keyLen);
	//memCopyToRam(cacheBuffer, MEMtPM8(SECURE_HEAD_BYTE_OFFEST(tKeyDataAField)), keyLen);

	/*SO_getSecDataByAddr( cacheBuffer, 0, keyLen, MEMtPM8(tKeyDataAField));
	keyDataAField[0]	= PXtMEM(cacheBuffer);*/
	tempBuffer	= PXtMEM(sysConfig_GetTempArrBuffer(keyLen));/*U32_A_U16(MemGetReuseRam_GetEEpBuffer(keyLen), keyLen)*/
	SO_getSecDataByAddr( MEMtPX8(tempBuffer), 0, keyLen, MEMtPM8(tKeyDataAField));
	keyDataAField[0] 		 = tempBuffer;

	return keyLen;
}
#endif
