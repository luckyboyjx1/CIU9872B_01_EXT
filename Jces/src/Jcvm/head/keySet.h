/*
 * keySet.h
 *
 *  Created on: 2013年9月29日
 *      Author: mowenpan
 */

#ifndef KEYSET_H_
#define KEYSET_H_

#include "ecpDefine.h"
#include "ob_manager.h"
#include "profile.h"

#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
#define MAX_KEY_NUM                                    4
#else
#define MAX_KEY_NUM                                    3
#endif

#define CONTENT_KEY_ID                                 1

#define APDU_FORMAT_OFFSET                             U8_A_CONST(OFFSET_CDATA, 1)
#define KEY_LENGTH_OFFSET                              U8_A_CONST(OFFSET_CDATA, 1)
#define KEY_VERSION_OFFSET                             (OFFSET_CDATA)

#define KEY_ID_01                                      1
#define KEY_ID_02                                      2
#define KEY_ID_03                                      3
#define KEY_ID_04                                      4

#define KEY_TYPE_SYMMETRIC                             0x80
#define KEY_TYPE_RSA_PUBLIC                            0xA0
#define KEY_TYPE_RSA_PIVATE                            0xA1
#define KEY_TYPE_RSA_CRT                               0xA4

#define TAG_NEW_KEY                                    1
#define TAG_UPDATE_KEY                                 2
#define TAG_GET_KEY                                    3
#define TAG_UPDATE_KEY_VERSION                         4

#define KEY_UNIT_LENGTH                                8
#define KEY_ID_OFFSET                                  1

#define DEFAULT_KEY_VERSION                            0xFF
#define APDU_FORMAT2_TAG                               0xFF

#define APDU_KEY_FORMAT_01                             1
#define APDU_KEY_FORMAT_02                             2

#define KEY_ID_INIT_VALUE                              0xFF
#define KEY_TYPE_INIT_VALUE                            0

/*****************记录APDU 命令密钥格式与密钥在系统的存储格式******************/
#define KEY_FORMAT_STORE_MASH                          0x7F
#define KEY_FORMAT_APDU_MASH                           0x80    // bit7用于表示put key时数据域的格式， 0格式使用的是 keyType方式，1表示使用的是 FF + keyType方式
#define KEY_FORMAT_APDU_1                              0       // put key时数据域的格式为Key Information Data Structure  -  Basic
#define KEY_FORMAT_APDU_2                              0x80    // put key时数据域的格式为Key Information Data Structure  -  Extended
// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#define KEY_SET_PCK_ID								       EASTCOMPEACE_OPENPLATFORM_ID

#define getFirstKeySetHand(keyGroupRef)                    get_object_field_ref(MEMtPF8(locateObjectInMemory(keyGroupRef)), keySet_INDEX)
#define getFirstKeySetRef(keyGroupRef)                     memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(locateObjectInMemory(keyGroupRef)), keySet_INDEX)))
#define getNextKeySetHand(keySetObj)                       get_object_field_ref(keySetObj, nextKeySet_INDEX)

#define getSeqCounterField(keySetObj)                      get_object_field_ref(keySetObj, sequenceCounter_INDEX)
#define getExtSeqCounterField(keySetObj)                   get_object_field_ref(keySetObj, extSequenceCounter_INDEX)

//keyType_INDEX
#define getKeyType(keySetObj)                              memReadU8(get_object_field_ref(keySetObj, keyType_INDEX))
#define setKeyType(keySetObj,keyType)                      nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, keyType_INDEX)), keyType)

#define SEQUENCE_COUNTER_LENGTH_2                          2
#define SEQUENCE_COUNTER_LENGTH_3                          3
#define SEQUENCE_COUNTER_LENGTH_4                          4
#define SEQUENCE_COUNTER_LENGTH_5                          5

#define KEY_DATA_DEFINE_VALUE   0xFF

#define getKeyDataAField(keyDataRef)    get_object_field_ref(locateObjectInMemory(keyDataRef), keyData_INDEX)

//AES MacLen
//#define keySet_getAesKeyMacLenField(keySetObj)           MEMtPN8(get_object_field_ref(keySetObj, macLen_INDEX))
#define keySet_getAesKeyMacLen(keySetObj)                nvmReadU8(MEMtPN8(get_object_field_ref(keySetObj, macLen_INDEX)))
#define keySet_setAesKeyMacLen(keySetObj,macLen)         nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, macLen_INDEX)),macLen)

//keyVersion
#define keySet_getKeyVersion(keySetObj)                nvmReadU8(MEMtPN8(get_object_field_ref(keySetObj, keyVersion_INDEX)))
#define keySet_setKeyVersion(keySetObj,keyVersion)     nvmWriteU8Atomic(MEMtPN8(get_object_field_ref(keySetObj, keyVersion_INDEX)), keyVersion)

//eccKeyParaRefValidTag_INDEX merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#define keySet_getEccKeyParaRefValidTag(keySetObj)                           nvmReadU8(MEMtPN8(get_object_field_ref(keySetObj, eccKeyParaRefValidTag_INDEX)))
#define keySet_setEccKeyParaRefValidTag(keySetObj,eccKeyParaRefValidTag)     nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, eccKeyParaRefValidTag_INDEX)), eccKeyParaRefValidTag)

//eccKeyParaRefValue_INDEX merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#define keySet_getEccKeyParaRefValue(keySetObj)                        nvmReadU16(MEMtPN8(get_object_field_ref(keySetObj, eccKeyParaRefValue_INDEX)))
#define keySet_setEccKeyParaRefValue(keySetObj,eccKeyParaRefValue)     nvmWriteU16NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, eccKeyParaRefValue_INDEX)), eccKeyParaRefValue)

//format 记录APDU 命令密钥格式与密钥在系统的存储格式
#define keySet_getKeyFormat(keySetObj)                 nvmReadU8(MEMtPN8(get_object_field_ref(keySetObj, format_INDEX)))
#define keySet_setKeyFormat(keySetObj,format)          nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, format_INDEX)), format)
#define keySet_getStoreKeyFormmat(keySetObj)           U8_AND_U8(keySet_getKeyFormat(keySetObj), KEY_FORMAT_STORE_MASH)
#define keySet_getAPDUKeyFormmat(keySetObj)            U8_AND_U8(keySet_getKeyFormat(keySetObj), KEY_FORMAT_APDU_MASH)

//nextKeySet
#define keySet_getNextKeySetRef(keySetObj)             memReadU16(MEMtPN8(get_object_field_ref(keySetObj, nextKeySet_INDEX)))
#define keySet_getNextKeySetObj(keySetObj)             locateObjectInMemory(PN_R_U16(get_object_field_ref(keySetObj, nextKeySet_INDEX)))


//keyData
#define keySet_getKeyDataField(keySetObj)              get_object_field_ref(keySetObj,keyData_INDEX)
#define keySet_getKeyDataRef(keySetObj)                nvmReadU16(MEMtPN8(keySet_getKeyDataField(keySetObj)))
#define keySet_getKeyDataObj(keySetObj)                locateObjectInMemory(nvmReadU16(get_object_field_ref(keySetObj,keyData_INDEX))) //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#define keySet_getKeyDataAField(keySetObj)             get_array_data(MEMtPF8(keySet_getKeyDataObj(keySetObj)))

//keyExtData
#define keySet_getKeyExtDataField(keySetObj)           get_object_field_ref(keySetObj,KeyExtraData_INDEX)
#define keySet_getKeyExtDataRef(keySetObj)             memReadU16(MEMtPN8(keySet_getKeyExtDataField(keySetObj)))

//keyNum
#define keySet_incKeyNum(keySetObj)                    nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, keyNum_INDEX)), U8_A_U8(memReadU8(get_object_field_ref(keySetObj, keyNum_INDEX)), 1))
#define keySet_desKeyNum(keySetObj)                    nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, keyNum_INDEX)), U8_S_U8(memReadU8(get_object_field_ref(keySetObj, keyNum_INDEX)), 1))
#define keySet_setKeyNum(keySetObj, keyNum)            nvmWriteU8NoAtomic(MEMtPN8(get_object_field_ref(keySetObj, keyNum_INDEX)), keyNum)
#define keySet_getKeyNum(keySetObj)                    memReadU8(MEMtPN8(get_object_field_ref(keySetObj, keyNum_INDEX)))

//#if	CMS2AC
#define	keySet_getkeyAllSetField(keySetObj)			   get_object_field_ref(keySetObj, keyAllSet_INDEX)
#define	keySet_getkeyAllTag(keySetObj)				   memReadU8(MEMtPN8(keySet_getkeyAllSetField(keySetObj)))
#define	keySet_SetkeyAllTag(keySetObj, count)		   nvmWriteU8NoAtomic(MEMtPN8(keySet_getkeyAllSetField(keySetObj)), count)
//#endif
//TryAuthAttenptField
#define keySet_getTryAuthAttenptField(keySetObj)       get_object_field_ref(keySetObj, tryAuthAttenpt_INDEX)
#define keySet_getTryAuthAttenpt(keySetObj)            memReadU8(MEMtPN8(keySet_getTryAuthAttenptField(keySetObj)))
#define keySet_setTryAuthAttenpt(keySetObj,bTryAuth)   nvmWriteU8NoAtomic(MEMtPN8(keySet_getTryAuthAttenptField(keySetObj)), bTryAuth)

#if GP__KEY_MANAGER
	u8 keySet_getMaxKeyVersion(PN8 sdDataObj, u8 scpType);
	jref keySet_newKeySetBySCPType(u8 format, PN8 sdDataObj, u8 keyVersion, u16 keyLen, u8 SCPType);
	jref keySet_newKeySet(u8 format, PN8 sdDataObj, u8 keyVersion, u16 keyLen);
	void keySet_replaceKeyVersion(PN8 keySetObj, u8 keyVersion);
	void keySet_deleteScKey(PN8 sdDataObj, u8 keyVersion, u8 keyId);
	void keySet_deleteAllScKey(PN8 sdDataObj);

	u8 keySet_getDefaultKeyVersion(PN8 sdDataObj);
	jref keySet_getDefaultKeySet(PN8 sdDataObj);

	extern	jref keySet_getKeySet(PN8 sdDataObj, u8 keyVersion);

	memref keySet_getSequenceCounter(PN8 keySetObj, u8 len);
	extern void keySet_setSequenceCounter(PN8 keySetObj, u8 len, PX8 seqCounterField);
	void keySet_incSequenceCounter(PN8 keySetObj, u8 len);

	void keySet_incTryAuthAttenpt(PN8 keySetObj);
	u8 keySet_isCMKeyVersion(u8 keyVersion);

	u8 keySet_getKeyType(PN8 keySetObj, u8 keyId, u8 mode); //mode 区分在没找到key的情况下是抛异常还是返回0
	void keySet_setKeyType(PN8 keySetObj, u8 keyId, u8 keyType);

	memref keySet_getKeySetHand(PN8 keySetHand, u8 keyVersion);
	void keySet_ResetAllKeyCountByScpType(u8 sdId, u8 ScpType);
	boolean keySet_CheckIntergrated(PN8 keySetObj);
	u16 keySet_GetCounter(PX32 counterAddr, u8 version);
	extern jref GetFirstFullKey(PN8 sdDataObj, u8 keyVersion);
#else
	#define keySet_getMaxKeyVersion(sdDataObj, scpType)													0
	#define keySet_newKeySetBySCPType(format, sdDataObj, keyVersion, keyLen, SCPType)					0
	#define keySet_newKeySet(format, sdDataObj, keyVersion, keyLen)										0
	#define keySet_replaceKeyVersion(keySetObj, keyVersion)
	#define keySet_deleteScKey(sdDataObj, keyVersion, keyId)
	#define keySet_deleteAllScKey(sdDataObj)

	#define keySet_getDefaultKeyVersion(sdDataObj)														0
	#define keySet_getDefaultKeySet(sdDataObj)															0

	#define keySet_getKeySet(sdDataObj, keyVersion)														0

	#define keySet_getSequenceCounter(keySetObj, len)													0
	#define keySet_setSequenceCounter(keySetObj, len, seqCounterField)
	#define keySet_incSequenceCounter(keySetObj, len)

	#define keySet_incTryAuthAttenpt(keySetObj)
	#define keySet_isCMKeyVersion(keyVersion)															0

	#define keySet_getKeyType(keySetObj, keyId, mode)															0
	#define keySet_setKeyType(keySetObj, keyId, keyType)

	#define keySet_getKeySetHand(keySetHand, keyVersion)												0
	#define keySet_ResetAllKeyCountByScpType(sdId, ScpType)
	#define keySet_CheckIntergrated(keySetObj)															0
	#define keySet_GetCounter(counterAddr, version)														0
#endif

#endif /* KEYSET_H_ */
