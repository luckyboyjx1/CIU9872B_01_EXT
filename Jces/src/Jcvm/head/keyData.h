/*
 * keyData.h
 *
 *  Created on: 2013年10月8日
 *      Author: mowenpan
 */

#ifndef KEYDATA_H_
#define KEYDATA_H_

#include "profile.h"
#include "KeyExtData.h"
#include "SecureOperation.h"
#include "MemGetReuseRam.h"

typedef u8 KeyAccess;
//typedef u16 KeyUsageQualifier;//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:兼容GP2.3中usage两字节的情况

#define KEY_ATTR_DEFAULT_VALUE                         1

#define KEY_FORMAT_01                                  1
#define KEY_FORMAT_02                                  2
#define KEY_FORMAT_03                                  3


#define KEY_ATTR_LENGTH                                 1
#define KEY_ACCESS_LENGTH                               1
#define KEY_USAGE_QUALIFIER_LENGTH                      3//GP2.3扩展为2字节,存储时需要是LV格式, modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32

#define EXT_KEY_LENGTH_WIDTH                            2
#define KEY_TYPE_LENGTH_WIDTH                           1

#define KEY_DATA_LENGTH_WIDTH                           2
#define KEY_ID_LENGTH_WIDTH                             1

#define DES_KEY_LENGTH                                  16

#define AES_DEFAULT_MAC_LENGTH                           8

#define KEY_ATTR_MASH                                  0x7F
#define KEY_ATTR_DELETE_MASH                           0x80 //表示 keyId 已经删除
#define KEY_COUNT_MASH                                 KEY_ATTR_MASH

#define keyData_addKeyDataSpace(keySetObj, keyLen)     keyExtData_addKeyDataSpace(keySetObj, keyLen)

#if GP__KEY_MANAGER

	jref keyData_newKeyData(PN8 keySetObj, u16 len);
	u16 keyData_genPaddingKeyLen(u16 keyLen);

	void keyData_setKeyAccess(PN8 keySetObj, u8 keyId, PD8 keyAccess);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数
	u8 keyData_getKeyAccess(PN8 keySetObj, u8 keyId, PD8 keyAccess);
	u8 keyData_getKeyUsageQualifier(PN8 keySetObj, u8 keyId, PD8 keyUsageQualifier);
	void keyData_setKeyUsageQualifier(PN8 keySetObj, u8 keyId, PR8 keyUsageQualifier);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:去掉多余的参数

	u16 keyData_computeKeyDataLen(u8 format, u16 curKeyDataLen, u16 keyLen, u8 isNewKey);
	u8 keyData_getKeyAttr(PN8 keySetObj, u8 keyId);

	memref keyData_prepareKeyData(PN8 keySetObj, u8 status, u8 keyId, u8 keyType, u16 keyLen);
	u16 keyData_getKeyDataById(PN8 keySetObj, u8 keyId, u8 keyType, PX8 keyDataAField);
	u16 getKeyDataByIdforSCP80(PN8 keySetObj, u8 keyId, u8 keyType, PR32 keyDataAField);
#else
	#define keyData_newKeyData(keySetObj, len)																0
	#define keyData_genPaddingKeyLen(keyLen)																0

	#define keyData_setKeyAccess(keySetObj, keyId, keyAccess)
	#define keyData_getKeyAccess(keySetObj, keyId, keyAccess)												0
	#define keyData_getKeyUsageQualifier(keySetObj, keyId, keyUsageQualifier)								0
	#define keyData_setKeyUsageQualifier(keySetObj, keyId, keyUsageQualifier)

	#define keyData_computeKeyDataLen(format, curKeyDataLen, keyLen, isNewKey)								0
	#define keyData_getKeyAttr(keySetObj, keyId)															0

	#define keyData_prepareKeyData(keySetObj, status, keyId, keyType, keyLen)								0
	#define keyData_getKeyDataById(keySetObj, keyId, keyType, keyDataAField)								0
	#define getKeyDataByIdforSCP80(keySetObj, keyId, keyType, keyDataAField)								0
#endif

#if BOOT__SECURITY__KEY_DATA_SAFE
	#define	SEC_HEADER_SIZE																					SEC_DATA_HEADER_SIZE
	#define	SECURE_HEAD_BYTE_OFFEST(v)																		PN8_A_U8(v, SEC_HEADER_SIZE)
	#define	SECURE_HEAD_BYTE_RESET(v)																		U16_S_U16(v, SEC_HEADER_SIZE)
#else
	#define	SEC_HEADER_SIZE																					0
	#define	SECURE_HEAD_BYTE_OFFEST(v)																		v
	#define	SECURE_HEAD_BYTE_RESET(v)																		v
#endif//BOOT__SECURITY__KEY_DATA_SAFE

#endif /* KEYDATA_H_ */
