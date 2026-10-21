/*
 * fm02_keyData.h
 *
 *  Created on: 2013年9月29日
 *      Author: mowenpan
 */


#ifndef FM02_KEYDATA_H_
#define FM02_KEYDATA_H_

#include "profile.h"
#include "keyData.h"
#include "SCPInterface.h"
#include "SecurityDomainData.h"

#define FM02_MASK_KEYID											0xFF

#define FM02_KEY_ATTR_VALUE                                     (u8)0x01
//#define FM02_KEY_LENGTH_OFFSET                                  0
#define FM02_KEY_DATA_LENGTH_WIDTH                              1
#define FM02_KEY_DATA_KEYID_SIZE                                MAX_KEY_NUM
#define FM02_KEY_COMPONENT_HEAD_LENGTH                         (KEY_ATTR_LENGTH + KEY_ACCESS_LENGTH + KEY_USAGE_QUALIFIER_LENGTH)

//myformat02 SCP 03/80
#define FM02_KEY_ID_MASK_OFFSET          		   				0
#define FM02_KEY_LENGTH_OFFSET                 				    MAX_KEY_NUM
#define FM02_KEY_ID1_ATTR_OFFSET            				    (FM02_KEY_LENGTH_OFFSET + FM02_KEY_DATA_LENGTH_WIDTH)
#define FM02_KEY_ID1_KEY_USAGE_QUALIFIER_OFFSET                 (FM02_KEY_ID1_ATTR_OFFSET + KEY_ATTR_LENGTH)
#define FM02_KEY_ID1_KEY_ACCESS_OFFSET                          (FM02_KEY_ID1_KEY_USAGE_QUALIFIER_OFFSET + KEY_USAGE_QUALIFIER_LENGTH)
#define FM02_KEY_ID1_KEYDATA_OFFSET            				    (FM02_KEY_ID1_KEY_ACCESS_OFFSET + KEY_ACCESS_LENGTH)

#define FM02_KEY_ID2_ATTR_OFFSET(keyLen)                	    (FM02_KEY_LENGTH_OFFSET + FM02_KEY_DATA_LENGTH_WIDTH + FM02_KEY_COMPONENT_HEAD_LENGTH + keyLen)
#define FM02_KEY_ID2_KEY_USAGE_QUALIFIER_OFFSET(keyLen)         (FM02_KEY_ID2_ATTR_OFFSET(keyLen) + KEY_ATTR_LENGTH)
#define FM02_KEY_ID2_KEY_ACCESS_OFFSET(keyLen)                  (FM02_KEY_ID2_KEY_USAGE_QUALIFIER_OFFSET(keyLen) + KEY_USAGE_QUALIFIER_LENGTH)
#define FM02_KEY_ID2_KEYDATA_OFFSET(keyLen)			            (FM02_KEY_ID2_KEY_ACCESS_OFFSET(keyLen) + KEY_ACCESS_LENGTH)

#define FM02_KEY_ID3_ATTR_OFFSET(keyLen)            	        (FM02_KEY_LENGTH_OFFSET + FM02_KEY_DATA_LENGTH_WIDTH + (FM02_KEY_COMPONENT_HEAD_LENGTH + keyLen) * 2)
#define FM02_KEY_ID3_KEY_USAGE_QUALIFIER_OFFSET(keyLen)         (FM02_KEY_ID3_ATTR_OFFSET(keyLen) + KEY_ATTR_LENGTH)
#define FM02_KEY_ID3_KEY_ACCESS_OFFSET(keyLen)                  (FM02_KEY_ID3_KEY_USAGE_QUALIFIER_OFFSET(keyLen) + KEY_USAGE_QUALIFIER_LENGTH)
#define FM02_KEY_ID3_KEYDATA_OFFSET(keyLen)                     (FM02_KEY_ID3_KEY_ACCESS_OFFSET(keyLen) + KEY_ACCESS_LENGTH)

//#define FM02_getKeylength(keySetObj) 			   				U16t8(SECURE_HEAD_BYTE_RESET(PN_R_U8(PN8_A_U16(MEMtPN8(keySet_getKeyDataAField(keySetObj)), FM02_KEY_LENGTH_OFFSET))))

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
#define FM02_KEY_ID4_ATTR_OFFSET(keyLen)            	          (FM02_KEY_LENGTH_OFFSET + FM02_KEY_DATA_LENGTH_WIDTH + (FM02_KEY_COMPONENT_HEAD_LENGTH + keyLen) * 3)
#define FM02_KEY_ID4_KEY_USAGE_QUALIFIER_OFFSET(keyLen)         (FM02_KEY_ID4_ATTR_OFFSET(keyLen) + KEY_ATTR_LENGTH)
#define FM02_KEY_ID4_KEY_ACCESS_OFFSET(keyLen)                  (FM02_KEY_ID4_KEY_USAGE_QUALIFIER_OFFSET(keyLen) + KEY_USAGE_QUALIFIER_LENGTH)
#define FM02_KEY_ID4_KEYDATA_OFFSET(keyLen)                     (FM02_KEY_ID4_KEY_ACCESS_OFFSET(keyLen) + KEY_ACCESS_LENGTH)
#endif

#if (GP__KEY_MANAGER && GP__KEY_MANAGER__FORMAT2)

	#define FM02_setKeyLength(keyDataAField, length)                (nvmWriteU8Atomic(MEMtPN8(PN8_A_CONST(keyDataAField, FM02_KEY_LENGTH_OFFSET)), length))

	memref FM02_prepareKeyData(PN8 keySetObj, u8 keyId, PX16 keyLen, u8 status, u8 keyType);
	void FM02_deleteSingleScKey(PN8 keySetHand, u8 keyId);

	u8 FM02_getKeyAttr(PN8 keyDataField, u8 keyId);
	void FM02_setKeyAttr(PN8 keyDataField, u8 keyId, u8 attr);

	u8 FM02_getKeyAccess(PN8 keyDataField, u8 keyId);
	void FM02_setKeyAccess(PN8 keyDataField, u8 keyId, u8 keyAccess);

	u8 FM02_getKeyUsageQualifier(PN8 keyDataField, u8 keyId, PR8 keyUsageQualifier);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:兼容GP2.3中usage两字节的情况
	void FM02_setKeyUsageQualifier(PN8 keyDataField, u8 keyId, PR8 keyUsage); //modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
	u8 genMaskKeyId(PN8 keyDataAField, u8 keyId, u8 status);
#else
	#define FM02_prepareKeyData(keySetObj, keyId, keyLen, status, keyType)							0
	#define FM02_deleteSingleScKey(keySetHand, keyId)

	#define FM02_getKeyAttr(keyDataField, keyId)													0
	#define FM02_setKeyAttr(keyDataField, keyId, attr)

	#define FM02_getKeyAccess(keyDataField, keyId)													0
	#define FM02_setKeyAccess(keyDataField, keyId, keyAccess)

	#define FM02_getKeyUsageQualifier(keyDataField, keyId, keyUsageQualifier)						0
	#define FM02_setKeyUsageQualifier(keyDataField, keyId, keyUsage)
	#define genMaskKeyId(keyDataAField, keyId, status)												0
#endif

#endif /* FM02_KEYDATA_H_ */
