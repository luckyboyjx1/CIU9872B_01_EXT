/*
 * fm01_keyData.c
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
#include "fm01_keyData.h"

#if (GP__KEY_MANAGER && GP__KEY_MANAGER__FORMAT1)

static memref locateKeyAttr(PN8 keyDataAField, u8 keyId)
{
	u8 attrOffset;

	switch (keyId)
	{
		case 1:
			attrOffset = FM01_KEY_ID1_ATTR_OFFSET;
			break;

		case 2:
			attrOffset = FM01_KEY_ID2_ATTR_OFFSET;
			break;

		case 3:
			attrOffset = FM01_KEY_ID3_ATTR_OFFSET;
			break;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			case 4:
			attrOffset = FM01_KEY_ID4_ATTR_OFFSET;
			break;
#endif
		
		default:
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
			break;
	}
	return U32_A_U8(PNtMEM(keyDataAField), attrOffset);
}


static memref locateKeyData(PN8 keyDataAField, u8 keyId)
{
	u8 keyDataOffset;

	switch (keyId)
	{
		case 1:
			keyDataOffset = FM01_KEY_ID1_KEYDATA_OFFSET;
			break;

		case 2:
			keyDataOffset = FM01_KEY_ID2_KEYDATA_OFFSET;
			break;

		case 3:
			keyDataOffset = FM01_KEY_ID3_KEYDATA_OFFSET;
			break;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			case 4:
			keyDataOffset =  FM01_KEY_ID4_KEYDATA_OFFSET;
			break;
#endif
		default:
			throw_error(SW_INCORRECT_P1P2);
			break;
	}

	return PNtMEM(PN8_A_U16(keyDataAField, keyDataOffset));
}

memref FM01_prepareKeyData(PN8 keySetObj, u8 keyId, PR16 keyLen, u8 status, u8 keyType)
{
	PN8 keyDataAField;

	keyDataAField = MEMtPN8(keySet_getKeyDataAField(keySetObj));

	
	if (TAG_NEW_KEY == status)
	{
		keySet_setKeyType(keySetObj, keyId, keyType);
		nvmWriteU8Atomic(MEMtPN8(locateKeyAttr(keyDataAField, keyId)), FM01_KEY_ATTR_VALUE);
		keySet_incKeyNum(keySetObj);
	}

	*MEMtPR16(keyLen)  = DES_KEY_LENGTH;
	return locateKeyData(keyDataAField, keyId);
}

void FM01_deleteSingleScKey(PN8 keySetObj, u8 keyId)
{
	PN8 keyDataField;
	u8 attr;

	keyDataField = MEMtPN8(keySet_getKeyDataAField(keySetObj));

	keyDataField = MEMtPN8(locateKeyAttr(keyDataField, keyId)); //复用变量, 表示  KeyAttr 位置
	attr = memReadU8(keyDataField);

	if ((KEY_ATTR_DELETE_MASH != U8_AND_U8(KEY_ATTR_DELETE_MASH, attr)) && (0 != attr))
	{
		nvmWriteU8Atomic(MEMtPN8(keyDataField), U8_OR_U8(attr, KEY_ATTR_DELETE_MASH));
		keySet_desKeyNum(keySetObj);
	}
	else //keyID 已经删除
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
}

#endif
