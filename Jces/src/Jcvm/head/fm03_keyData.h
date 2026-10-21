/*
 * fm03_keyData.h
 *
 *  Created on: 2013-10-10
 *      Author: mowenpan
 */

#ifndef FM03_KEYDATA_H_
#define FM03_KEYDATA_H_

#include "keySet.h"
#include "SCPInterface.h"
#include "SecurityDomainData.h"
#include "profile.h"
#include "keyData.h"

#if (GP__KEY_MANAGER && GP__KEY_MANAGER__FORMAT3)

#define FM03_KEY_ID_TYPE_LENGTH                              1

#define FM03_KEY_ID_OFFSET                                   0
#define FM03_KEY_LENGTH_OFFSET                               (FM03_KEY_ID_OFFSET + KEY_ID_LENGTH_WIDTH)
#define FM03_KEY_ATTR_OFFSET                 				 (FM03_KEY_LENGTH_OFFSET + KEY_DATA_LENGTH_WIDTH)
#define FM03_KEY_TYPE_OFFSET                 				 (FM03_KEY_ATTR_OFFSET + KEY_ATTR_LENGTH)
#define FM03_KEY_USAGE_QUALIFIER_OFFSET       				 (FM03_KEY_TYPE_OFFSET + FM03_KEY_ID_TYPE_LENGTH)
#define FM03_KEY_ACCESS_OFFSET                 				 (FM03_KEY_USAGE_QUALIFIER_OFFSET + KEY_USAGE_QUALIFIER_LENGTH)
#define FM03_KEY_COMPONENT_START_OFFSET       				 (FM03_KEY_ACCESS_OFFSET + KEY_ACCESS_LENGTH)

#define FM03_KEY_HEAD_LENGTH                                 (FM03_KEY_COMPONENT_START_OFFSET + 1)

#define KEY_COMPONENT_TAG_WIDTH                              1
#define KEY_COMPONENT_LENGTH_WIDTH                           KEY_DATA_LENGTH_WIDTH

#define KEY_COMPONENT_HEAD_LENGTH                            (KEY_COMPONENT_TAG_WIDTH + KEY_DATA_LENGTH_WIDTH)

#define FM03_KEY_COMPONENT_TAG_OFFSET                        0
#define FM03_KEY_COMPONENT_LENGTH_OFFSET                     (FM03_KEY_COMPONENT_TAG_OFFSET + KEY_COMPONENT_TAG_WIDTH)
#define FM03_KEY_COMPONENT_DATA_OFFSET                       (FM03_KEY_COMPONENT_LENGTH_OFFSET + KEY_COMPONENT_LENGTH_WIDTH)
memref FM03_findKeyComponentInKeyData(PN8 keyDataAField, u16 keyDataLen, u8 keyType, u8 status);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数
extern memref FM03_locateKeyId(PN8 keySetObj, u8 keyId, u8 status, PR16 nextKeyDataRef, PR16 keyFieldLen);
memref FM03_findKeyComponent(PN8 keyDataField, u16 keyDataLen, u8 keyId, u8 keyType, u16 keyIdLen, u8 status, jref nextKeyData);
memref FM03_prepareKeyComponent(PN8 keySetObj, u8 keyId, u8 keyType, PR16 keyLen, u8 status);
void FM03_deleteSingleScKey(PN8 keySetObj, u8 keyId);

void FM03_setKeyAccess(PN8 keySetObj, u8 keyId, u8 keyAccess);
u8 FM03_getKeyAccess(PN8 keySetObj, u8 keyId);

void FM03_setKeyUsageQualifier(PN8 keySetObj, u8 keyId , PR8 keyUsageQualifier);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:兼容GP2.3中usage两字节的情况
u8 FM03_getKeyUsageQualifier(PN8 keySetObj, u8 keyId, PR8 keyUsageQualifier);// modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32

u8 FM03_getKeyAttr(PN8 keySetObj, u8 keyId);
u16 FM03_writeKeyInfo(PX8 buf, u16 maxSingleLength, PN8 keySetObj, u16 offset);

memref fm03_locateKeyType(PN8 keySetObj, u8 keyId);

extern u8 genKeyType(u8 keyComponentType);
#else
	#define FM03_findKeyComponentInKeyData(keyDataAField, keyDataLen, keyId, keyType, status)					0
	#define FM03_locateKeyId(keySetObj, keyId, status, nextKeyDataRef, keyFieldLen)								0
	#define FM03_findKeyComponent(keyDataField, keyDataLen, keyId, keyType, keyIdLen, status, nextKeyData)		0
	#define FM03_prepareKeyComponent(keySetObj, keyId, keyType, keyLen, status)									0
	#define FM03_deleteSingleScKey(keySetObj, keyId)

	#define FM03_setKeyAccess(keySetObj, keyId, keyAccess)
	#define FM03_getKeyAccess(keySetObj, keyId)																	0

	#define FM03_setKeyUsageQualifier(keySetObj, keyId , keyUsageQualifier)
	#define FM03_getKeyUsageQualifier(keySetObj, keyId, keyUsageQualifier)										0

	#define FM03_getKeyAttr(keySetObj, keyId)																	0
	#define FM03_writeKeyInfo(buf, maxSingleLength, keySetObj, offset)											0

	#define fm03_locateKeyType(keySetObj, keyId)																0

    #define genKeyType(keyComponentType)                                                                     0
#endif

#endif /* FM03_KEYDATA_H_ */
