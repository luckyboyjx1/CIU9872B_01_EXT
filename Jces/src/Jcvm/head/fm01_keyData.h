/*
 * fm01_keyData.h
 *
 *  Created on: 2013Äê9ÔÂ29ÈÕ
 *      Author: mowenpan
 */
#include "profile.h"

#ifndef FM01_KEYDATA_H_
#define FM01_KEYDATA_H_

#if (GP__KEY_MANAGER && WITH_KEYMANAGER_FORMAT1)
#define FM01_KEY_ATTR_VALUE                                     (u8)0x01
#define FM01_KEY_LENGTH                                         (KEY_ATTR_LENGTH + KEY_ACCESS_LENGTH + KEY_USAGE_QUALIFIER_LENGTH + DES_KEY_LENGTH)
#define FM01_KEY_ATTR_OFFSET                                    0
#define FM01_KEY_USAGE_QUALIFIER                                (FM01_KEY_ATTR_OFFSET + KEY_ATTR_LENGTH)
#define FM01_KEY_ACCESS_OFFSET                                  (FM01_KEY_USAGE_QUALIFIER + KEY_USAGE_QUALIFIER_LENGTH)


//myformat01 SCP 01/02
#define FM01_KEY_ID1_ATTR_OFFSET                                0
#define FM01_KEY_ID1_USAGE_QUALIFIER_OFFSET                     (FM01_KEY_ID1_ATTR_OFFSET + KEY_ATTR_LENGTH)
#define FM01_KEY_ID1_KEY_ACCESS_OFFSET                          (FM01_KEY_ID1_USAGE_QUALIFIER_OFFSET + KEY_USAGE_QUALIFIER_LENGTH)
#define FM01_KEY_ID1_KEYDATA_OFFSET                             (FM01_KEY_ID1_KEY_ACCESS_OFFSET + KEY_ACCESS_LENGTH)

#define FM01_KEY_ID2_ATTR_OFFSET                                (FM01_KEY_ID1_KEYDATA_OFFSET + DES_KEY_LENGTH)
#define FM01_KEY_ID2_USAGE_QUALIFIER_OFFSET                     (FM01_KEY_ID2_ATTR_OFFSET + KEY_ATTR_LENGTH )
#define FM01_KEY_ID2_KEY_ACCESS_OFFSET                          (FM01_KEY_ID2_USAGE_QUALIFIER_OFFSET + KEY_USAGE_QUALIFIER_LENGTH)
#define FM01_KEY_ID2_KEYDATA_OFFSET                             (FM01_KEY_ID2_KEY_ACCESS_OFFSET + KEY_ACCESS_LENGTH)

#define FM01_KEY_ID3_ATTR_OFFSET                                (FM01_KEY_ID2_KEYDATA_OFFSET + DES_KEY_LENGTH)
#define FM01_KEY_ID3_USAGE_QUALIFIER_OFFSET                     (FM01_KEY_ID3_ATTR_OFFSET + KEY_ATTR_LENGTH)
#define FM01_KEY_ID3_KEY_ACCESS_OFFSET                          (FM01_KEY_ID3_USAGE_QUALIFIER_OFFSET + KEY_USAGE_QUALIFIER_LENGTH)
#define FM01_KEY_ID3_KEYDATA_OFFSET                             (FM01_KEY_ID3_KEY_ACCESS_OFFSET + KEY_ACCESS_LENGTH)

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
#define FM01_KEY_ID4_ATTR_OFFSET                                (FM01_KEY_ID3_KEYDATA_OFFSET + DES_KEY_LENGTH)
#define FM01_KEY_ID4_USAGE_QUALIFIER_OFFSET                     (FM01_KEY_ID4_ATTR_OFFSET + KEY_ATTR_LENGTH)
#define FM01_KEY_ID4_KEY_ACCESS_OFFSET                          (FM01_KEY_ID4_USAGE_QUALIFIER_OFFSET + KEY_USAGE_QUALIFIER_LENGTH)
#define FM01_KEY_ID4_KEYDATA_OFFSET                             (FM01_KEY_ID4_KEY_ACCESS_OFFSET + KEY_ACCESS_LENGTH)
#endif

#define FM01_getKeyAttr(keyDataField,id)                        (PX_R_U8(keyDataField + FM01_KEY_LENGTH * (id - KEY_ID_OFFSET) + FM01_KEY_ATTR_OFFSET))
#define FM01_setKeyAttr(keyDataField,id,attr)                   (nvmWriteU8Atomic(keyDataField + FM01_KEY_LENGTH * (id - KEY_ID_OFFSET) + FM01_KEY_ATTR_OFFSET, attr))

#define FM01_getKeyAccess(keyDataField, id)                     (PX_R_U8(keyDataField + FM01_KEY_LENGTH * (id - KEY_ID_OFFSET) + FM01_KEY_ACCESS_OFFSET))
#define FM01_setKeyAccess(keyDataField, id, keyAccess)          (nvmWriteU8Atomic(keyDataField + FM01_KEY_LENGTH * (id - KEY_ID_OFFSET) + FM01_KEY_ACCESS_OFFSET, keyAccess))

#define FM01_getKeyUsageQualifier(keyDataField, id)             (PX_R_U8(keyDataField + FM01_KEY_LENGTH * (id - KEY_ID_OFFSET) + FM01_KEY_USAGE_QUALIFIER))
#define FM01_setKeyUsageQualifier(keyDataField, id, keyUsage)   (nvmWriteU8Atomic(keyDataField + FM01_KEY_LENGTH * (id - KEY_ID_OFFSET) + FM01_KEY_USAGE_QUALIFIER, keyUsage))

memref FM01_prepareKeyData(PN8 keySetObj, u8 keyId, PR16 keyLen, u8 status, u8 keyType);
void FM01_deleteSingleScKey(PN8 keySetObj, u8 keyId);

#else
	#define FM01_prepareKeyData(keySetObj, keyId, keyLen, status, keyType)
	#define FM01_deleteSingleScKey(keySetObj, keyId)
#endif

#endif /* FM01_KEYDATA_H_ */
