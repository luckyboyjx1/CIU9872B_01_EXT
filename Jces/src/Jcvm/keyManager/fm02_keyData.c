/*
 * fm02_keyData.c

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

#include "fm02_keyData.h"

#if (GP__KEY_MANAGER && GP__KEY_MANAGER__FORMAT2)


u8 genMaskKeyId(PN8 keyDataAField, u8 keyId, u8 status)
{
	u8 offset;
	u8 tmpKeyId;

	if (TAG_NEW_KEY == status)
	{
		for (offset = 0; offset < MAX_KEY_NUM; U8_INC(offset))
		{
			tmpKeyId = memReadU8(PN8_A_U16(keyDataAField, offset));
			
			if (tmpKeyId == FM02_MASK_KEYID)
			{
				nvmWriteU8Atomic(MEMtPN8(PN8_A_U16(keyDataAField, offset)), keyId);
				return	U16t8(U8_A_U8(offset, 1));
			}
		}
	}
	else
	{
		for (offset = 0; offset < MAX_KEY_NUM; U8_INC(offset))
		{
			tmpKeyId = memReadU8(PN8_A_U16(keyDataAField, offset));
			
			if (tmpKeyId == keyId)
			{
				return	U16t8(U8_A_U8(offset, 1));
			}
		}
	}

	return FM02_MASK_KEYID;
}

memref FM02_prepareKeyData(PN8 keySetObj, u8 keyId, PR16 keyLen, u8 status, u8 keyType)
{
	PN8 keyData;
	PN8 keyAttr;
	PN8 keyDataAField;
	u8 tmpKeyLen;

//#if CMS2AC
	u8	allKeyTag;
//#endif

	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));
	keyId = genMaskKeyId(keyDataAField, keyId, status);
	keyAttr = keyDataAField;
	keyData = keyDataAField;

	tmpKeyLen = memReadU8(PN8_A_U16(keyDataAField, FM02_KEY_LENGTH_OFFSET));

	switch (keyId)
	{
		case 1:
			keyData = PN8_A_U16(keyDataAField, FM02_KEY_ID1_KEYDATA_OFFSET);
			keyAttr = PN8_A_U16(keyDataAField, FM02_KEY_ID1_ATTR_OFFSET);
			break;
		case 2:
			keyData = PN8_A_U16(keyDataAField, FM02_KEY_ID2_KEYDATA_OFFSET(tmpKeyLen));
			keyAttr = PN8_A_U16(keyDataAField, FM02_KEY_ID2_ATTR_OFFSET(tmpKeyLen));
			break;
		case 3:
			keyData = PN8_A_U16(keyDataAField, FM02_KEY_ID3_KEYDATA_OFFSET(tmpKeyLen));
			keyAttr = PN8_A_U16(keyDataAField, FM02_KEY_ID3_ATTR_OFFSET(tmpKeyLen));
			break;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		case 4:
			keyData = PN8_A_U16(keyDataAField, FM02_KEY_ID4_KEYDATA_OFFSET(tmpKeyLen));
			keyAttr = PN8_A_U16(keyDataAField, FM02_KEY_ID4_ATTR_OFFSET(tmpKeyLen));
		break;
#endif
		default:
			keyManager_abortLogicManagerThorwError(SW_INCORRECT_P1P2);
//			throw_error(SW_INCORRECT_P1P2);
			break;
	}

	if (TAG_NEW_KEY == status)
	{
		//keySet_setKeyType(keySetObj, keyId, KEY_TYPE_SYMMETRIC);
		keySet_setKeyType(keySetObj, keyId, keyType);
		nvmWriteU8Atomic(MEMtPN8(keyAttr), FM02_KEY_ATTR_VALUE);
		keySet_incKeyNum(MEMtPN8(keySetObj));
//#if CMS2AC
		allKeyTag	= keySet_getkeyAllTag(keySetObj);
		allKeyTag   = U8_OR_U8(allKeyTag, U8_SHL_U8(1, U8_S_U8(keyId, 1)));
		keySet_SetkeyAllTag(keySetObj, allKeyTag);
//#endif
	}
	else if(TAG_UPDATE_KEY == status)
	{
		if(tmpKeyLen != SO_calSecDataAreaLen(keyLen[0]))// modify by xuxiaojun@20220628 更新密钥时，长度要和之前的保持一致 SCP11测试脚本：error_update_scp11c_key_length_Test_ssd_01
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}

		if(keyType != keySet_getKeyType(keySetObj, keyId, NO_ERROR))//modify by xuxiaojun@20220725 更新密钥时，key type不能被改变 gp221: When replacing keys, the new keys shall be presented to the card in the same format as they are already present on the card
		{
			keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
		}
//		keySet_setKeyType(keySetObj, keyId, keyType);
	}

	keyLen[0]     = U8t16(memReadU8(PN8_A_U16(keyDataAField, FM02_KEY_LENGTH_OFFSET)));
	return PNtMEM(keyData);
}
//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
void FM02_deleteSingleScKey(PN8 keySetObj, u8 keyId)
{
	PN8 keyDataField;
	u8 attr;
	u8 attrOffset;
	u8 keyLen;

//#if CMS2AC
	u8	allKeyTag;
//#endif
	
	keyDataField = MEMtPN8(keySet_getKeyDataAField(keySetObj));
	keyId = genMaskKeyId(keyDataField, keyId, TAG_GET_KEY);
	keyLen = memReadU8(PN8_A_U16(keyDataField, FM02_KEY_LENGTH_OFFSET));
	attrOffset = 0;

	switch (keyId) {
		case KEY_ID_01:
			attrOffset = FM02_KEY_ID1_ATTR_OFFSET;
			break;
		case KEY_ID_02:
			attrOffset = FM02_KEY_ID2_ATTR_OFFSET(keyLen);
			break;
		case KEY_ID_03:
			attrOffset = FM02_KEY_ID3_ATTR_OFFSET(keyLen);
			break;
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		case	KEY_ID_04:
			attrOffset = FM02_KEY_ID4_ATTR_OFFSET(keyLen);
			break;
#endif
		default:
			//throw_error(SW_CONDITIONS_NOT_SATISFIED);
		  throw_error(SW_REFERENCED_DATA_NOT_FOUND);// modified by zm; galitt km_APDU_deleteKey_06_NoKeyId,delete a key which it's id is not exist
			break;
	}

	attr = memReadU8(PN8_A_U16(keyDataField, attrOffset));

	if ((KEY_ATTR_DELETE_MASH != U8_AND_U8(KEY_ATTR_DELETE_MASH , attr)) && (0 != attr))
	{/*set DES KEY COUNT before it is deleted*/
		#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			if(0x80 == keySet_getKeyType(keySetObj, keyId,THROW_ERROR))
			{
					sdData_incDESKeyCount(getCurrentSdDataObj(IS_NO_NEED_CHECK_SD));
			}
		#endif	
			if((keyId == KEY_ID_01) || (keyId == KEY_ID_02) || (keyId == KEY_ID_03))
			{
				allKeyTag	= keySet_getkeyAllTag(keySetObj);
				allKeyTag	=	U8_AND_U8(allKeyTag, U8_INV(U8_SHL_U8(1, U8_S_U8(keyId, 1))));
				keySet_SetkeyAllTag(keySetObj, allKeyTag);	
			}
		
		nvmWriteU8Atomic(MEMtPN8(PN8_A_U16(keyDataField, attrOffset)), U8_OR_U8(attr , KEY_ATTR_DELETE_MASH));
		keySet_desKeyNum(keySetObj);
	}
	else 
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
}

static memref getKeyAttrField(PN8 keyDataField, u8 keyId)
{
	PN8 dest;
	u8 keyLen;

	dest	= MEM_NULL;
	keyId = genMaskKeyId(keyDataField, keyId, TAG_GET_KEY);
	keyLen = memReadU8(PN8_A_U16(keyDataField, FM02_KEY_LENGTH_OFFSET));

	switch (keyId)
	{
		case KEY_ID_01:
			dest = MEMtPN8(FM02_KEY_ID1_ATTR_OFFSET);
			break;
		case KEY_ID_02:
			dest = MEMtPN8(FM02_KEY_ID2_ATTR_OFFSET(keyLen));
			break;
		case KEY_ID_03:
			dest = MEMtPN8(FM02_KEY_ID3_ATTR_OFFSET(keyLen));
			break;
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		case	KEY_ID_04:
			dest = MEMtPN8(FM02_KEY_ID4_ATTR_OFFSET(keyLen));
			break;
#endif
		default:
			return  MEM_NULL;
	}

	return PNtMEM(PN8_A_U32(keyDataField, dest));
}

u8 FM02_getKeyAttr(PN8 keyDataField, u8 keyId)
{
	u8	value;
	PN8 dest;

	value	=	0;
	dest = MEMtPN8(getKeyAttrField(keyDataField, keyId));

	if(dest != MEM_NULL)
	{
		value = memReadU8(dest);
	}
	return value;
}

void FM02_setKeyAttr(PN8 keyDataField, u8 keyId, u8 attr)
{
	PN8 dest;

	dest = MEMtPN8(getKeyAttrField(keyDataField, keyId));
	nvmWriteU8Atomic(MEMtPN8(dest), attr);
}


static memref getKeyAccessField(PN8 keyDataField, u8 keyId)
{

	u8 			keyLen;
	memref 		dest;

	dest 	= MEM_NULL;
	keyId = genMaskKeyId(keyDataField, keyId, TAG_GET_KEY);
	keyLen = memReadU8(PN8_A_U16(keyDataField, FM02_KEY_LENGTH_OFFSET));

	switch (keyId) {
		case KEY_ID_01:
			dest = FM02_KEY_ID1_KEY_ACCESS_OFFSET;
			break;

		case KEY_ID_02:
			dest = FM02_KEY_ID2_KEY_ACCESS_OFFSET(keyLen);
			break;

		case KEY_ID_03:
			dest = FM02_KEY_ID3_KEY_ACCESS_OFFSET(keyLen);
			break;
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		case	KEY_ID_04:
			dest = FM02_KEY_ID4_KEY_ACCESS_OFFSET(keyLen);
			break;
#endif
		default:
			return  MEM_NULL;
	}

	return U32_A_U32(PNtMEM(keyDataField), dest);
}

u8 FM02_getKeyAccess(PN8 keyDataField, u8 keyId)
{
	PM8 dest;

	dest = MEMtPM8(getKeyAccessField(keyDataField, keyId));
	return memReadU8(dest);
}

void FM02_setKeyAccess(PN8 keyDataField, u8 keyId, u8 keyAccess)
{
	PN8 dest;

	dest = MEMtPN8(getKeyAccessField(keyDataField, keyId));
	nvmWriteU8Atomic(dest, keyAccess);
}

static memref getKeyUsageQualifierField(PN8 keyDataField, u8 keyId)
{
	u8 				keyLen;
	memref 			dest;

	dest 			= MEM_NULL;
	keyId 			= genMaskKeyId(keyDataField, keyId, TAG_GET_KEY);
	keyLen 			= memReadU8(PN8_A_U16(keyDataField, FM02_KEY_LENGTH_OFFSET));

	switch (keyId) {
		case KEY_ID_01:
			dest = FM02_KEY_ID1_KEY_USAGE_QUALIFIER_OFFSET;
			break;

		case KEY_ID_02:
			dest = FM02_KEY_ID2_KEY_USAGE_QUALIFIER_OFFSET(keyLen);
			break;

		case KEY_ID_03:
			dest = FM02_KEY_ID3_KEY_USAGE_QUALIFIER_OFFSET(keyLen);
			break;

		default:
			return  MEM_NULL;
	}

	return U32_A_U32(PNtMEM(keyDataField), dest);
}
/*
*@description:
*
*@see        :
*
*@function   :FM02_getKeyUsageQualifier
*
*@param      :PN8 keyDataField --- input
*@param      :u8 keyId         --- input
*@param      :PR8 keyUsageQualifier --- output
*
*@return     :keyUsageQualifier中有效数据的长度
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
*            :modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
u8 FM02_getKeyUsageQualifier(PN8 keyDataField, u8 keyId, PR8 keyUsageQualifier)
{
	PM8 dest;
	u8 len;

	dest = MEMtPM8(getKeyUsageQualifierField(keyDataField, keyId));
	len = memReadU8(dest);
	memCopyToRam(keyUsageQualifier, PM8_A_U8(dest, 0x01), len);

	return len;
}
/*
*@description:
*
*@see        :
*
*@function   :FM02_setKeyUsageQualifier
*
*@param      :PN8 keyDataField --- input
*@param      :u8 keyId --- input
*@param      :PR8 keyUsage --- input, LV, 总长度为KEY_USAGE_QUALIFIER_LENGTH
*
*@return     :void
*@history    :modify by xuxiaojun@20220519: merged from EPX\v3.0\DEVELOP：兼容keyUsage为2字节
*            :modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
void FM02_setKeyUsageQualifier(PN8 keyDataField, u8 keyId, PR8 keyUsage)
{
	PN8 dest;

	dest = MEMtPN8(getKeyUsageQualifierField(keyDataField, keyId));
	nvmCopyAtomic(dest, keyUsage, KEY_USAGE_QUALIFIER_LENGTH);
}
#endif
