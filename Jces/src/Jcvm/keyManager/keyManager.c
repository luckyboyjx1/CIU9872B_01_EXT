/*
 * keyManager.c
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
#include "KeyManager.h"
#include "OpFunction.h"
#include "SecurityDomainData.h"
#include "KeyLogicManager.h"
#include "SCPInterface.h"
#if GP__SCP__SCP11__FOR_MASK
#include "SCP11.h"
#endif

#if GP__KEY_MANAGER

#define KEY_VERSION_INIT_VALUE                         0xFF
#define PARA_VALUE_OFFSET                              2
#define PARA_WIDTH                                     3
#define SCP_TYPE_02                                    PUT_KEY_SCP02
#define SCP_TYPE_03                                    PUT_KEY_SCP03

#define GET_DATA_TAG_C0_HEAD_LENGTH                    2
#define GET_DATA_TAG_C0_VALUE_OFFSET                   2

/**
 * 删除密钥
 *modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11  删除scp11的key时，还需要删除对应结构体中的信息，所以需要在事务中
 */
#if GP__KEY_MANAGER__DELETE_KEY
void deleteKey(PN8 sdDataObj)
{
	u8 keyVersion;
	u8 keyId;
	u8 offset;
	u8 maxLength;
#if GP__SCP__SCP11__FOR_MASK
	u8 transactionStarted;
#endif
#if (GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
	u16 		bPrivRef;
	memref		entry;
	memref bPrivArray;
#endif
		
	keyVersion = KEY_VERSION_INIT_VALUE;
	keyId = KEY_VERSION_INIT_VALUE;
	offset = OFFSET_CDATA;

	maxLength = U16t8(U8_A_U8(APDUBuffer[OFFSET_LC], APDU_HEAD_LENGTH));
	while(offset < maxLength)
	{
		switch(APDUBuffer[offset])
		{
			case 0xD2:
				keyVersion = APDUBuffer[U8_A_U8(offset, PARA_VALUE_OFFSET)];
				offset = U16t8(U8_A_U8(offset, PARA_WIDTH));
		    	break;
			case 0xD0:
				keyId =  APDUBuffer[U8_A_U8(offset, PARA_VALUE_OFFSET)];
				offset = U16t8(U8_A_U8(offset, PARA_WIDTH));
		    	break;
			default:
				throw_error(SW_WRONG_DATA);
				break;
		}
	}

	if(keyVersion == KEY_VERSION_INIT_VALUE)
	{
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}
#if (GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
	if(currKeySetVer[selected_channel]==keyVersion)
	{
		throw_error(SW_WRONG_DATA);
	}	
	//modified by zm; CMS2AC 6.1.12.15.2 可删除收条key, 6.1.6.3.1可删除DAP_KEY, DECIPER_LOAD key; SSD下，可删除70 key
 	if((keyVersion == TOKEN_KEY_VERSION) && (getCurrentSDId() == 0x00))
 	{
 		throw_error(SW_WRONG_DATA);
 	}
	
	entry			=	getEntryByAppId(getCurrentSDId());
	bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));
	
	bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
	if((keyVersion == DAP_KEY_VERSION) && (U8_AND_U8(PX_R_U8(bPrivArray), DAP_VERIFICATION_PRIVILEGES_TAG) == DAP_VERIFICATION_PRIVILEGES_TAG))
	{
 		throw_error(SW_WRONG_DATA);
 	}
#endif
#if GP__SCP__SCP11__FOR_MASK
	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}

	keySet_deleteScKey(sdDataObj, keyVersion, keyId);

	if (IsSCP11Key(keyVersion))
	{
		SCP11DeleteKey(sdDataObj,  keyVersion, keyId);
	}

	if (transactionStarted)
	{
		commit_transaction();
	}
#else
	keySet_deleteScKey(sdDataObj, keyVersion, keyId);
#endif
}

#endif
/**
 * modify by xuxiaojun@20221208: 缺陷413，SCP80 三支key长度不一致导致profile下载失败，所以将SCP80的存储方案由format2修改为format3。 该函数中的默认SCP80使用的是format2格式，且本身也不需要默认scp80
 * modify by xuxiaojun@20230221: 根据不同的SCP设置默认密钥的key type,避免出现SCP03协议时put第一支AES密钥返回6a80
 */
void initIsdKey(PN8 sdDataObj)
{
	PN8 keyDataAField;
	PN8 keySetObj;
	jref keySetRef;
	u8 scpType;
	u8 keyType;
	u8 keyLen;

	//sdData_init(sdDataObj);
	scpType = sdData_getScpType(sdDataObj);
	keyLen = 0;
	keySetRef = REF_NULL;

	switch (scpType)
	{
		case 0x01:
		case 0x02:
			keyLen 		= DES_KEY_LENGTH;
			keyType     = KEY_DES_TYPE;
			keySetRef 	= keySet_newKeySet(KEY_FORMAT_02, sdDataObj, DEFAULT_KEY_VERSION, SO_calSecDataAreaLen(U8t16(keyLen)));
			break;
		case 0x03:
			keyLen 		= MAX_AES_KEY_LENGTH;
			keyType     = KEY_AES_TYPE;
			keySetRef 	= keySet_newKeySet(KEY_FORMAT_02, sdDataObj, DEFAULT_KEY_VERSION, SO_calSecDataAreaLen(U8t16(keyLen)));
			break;
// isd的默认key里面不需要有scp80，只会有scp02或scp03
//		case 0x80:
//			keyLen = AES_KEY_LENGTH_16;
//			keySetRef = keySet_newKeySet(KEY_FORMAT_02, sdDataObj, DEFAULT_KEY_VERSION, SO_calSecDataAreaLen(U8t16(keyLen)));
//			break;
		default:
			throw_error(SW_WRONG_DATA);
			break;
	}

	keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
	keyDataAField = MEMtPN8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_01, keyType, keyLen));
	//nvmCopyAtomic(MEMtPN8(keyDataAField), MEMtPM8(defaultKey), keyLen);
	if(scpType == 0x03)
	{
		SO_storeSecData(MEMtPX8(defaultKey_32Bytes), 0, keyLen,  keyDataAField);
	}
	else
	{
		SO_storeSecData(MEMtPX8(JcvmConfig_GetdefaultKeyBase()), 0, keyLen,  keyDataAField);
	}

	keyDataAField = MEMtPN8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, keyType, keyLen));
	//nvmCopyAtomic(MEMtPN8(keyDataAField), MEMtPM8(defaultKey), keyLen);
	if(scpType == 0x03)
	{
		SO_storeSecData(MEMtPX8(defaultKey_32Bytes), 0, keyLen,  keyDataAField);
	}
	else
	{
		SO_storeSecData(MEMtPX8(JcvmConfig_GetdefaultKeyBase() + keyLen), 0, keyLen,  keyDataAField);
	}

	keyDataAField = MEMtPN8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_03, keyType, keyLen));
	//nvmCopyAtomic(MEMtPN8(keyDataAField), MEMtPM8(defaultKey), keyLen);
	if(scpType == 0x03)
	{
		SO_storeSecData(MEMtPX8(defaultKey_32Bytes), 0, keyLen,  keyDataAField);
	}
	else
	{
		SO_storeSecData(MEMtPX8(JcvmConfig_GetdefaultKeyBase() + keyLen + keyLen ), 0, keyLen,  keyDataAField);
	}

	keySet_setKeyNum(keySetObj, KEY_ID_03); //由于默认key是完整的，所以需要将key Number设置为3，避免后续写入第一支key时，替换原来的默认key不进行keyNum修改 	-xuxiaojun -20220827
}

void KM_InitPutKeySequence(u8 p1)
{
	KLG_InitPutKeySequence(p1);
}

void cipherKey(memref des, memref src, u16 DataLen, u8 mode)
{

}
u8 getDefaultKeyCounter(PN8 sdDataObj, PX8 buf)
{
	u8 scpType;
	u8 seqCounterlen;
	PN8 seqCounterLocation;
	jref keySetRef;

	ecpMem_Set(buf, 0, GET_DATA_TAG_C0_HEAD_LENGTH);
	scpType = sdData_getScpType(sdDataObj);

	switch (scpType)
	{
		case SCP_TYPE_02:
			seqCounterlen = SCP02_SEQUENCE_CONTER_SIZE;
			break;

		case SCP_TYPE_03:
			seqCounterlen = SCP03_SEQUENCE_CONTER_SIZE;
			break;

		default:
			seqCounterlen = 0;
			break;
	}

	if (seqCounterlen > 0)
	{
		keySetRef = keySet_getDefaultKeySet(sdDataObj);

		if (REF_NULL != keySetRef)
		{
			seqCounterLocation = MEMtPN8(keySet_getSequenceCounter(MEMtPN8(locateObjectInMemory(keySetRef)), seqCounterlen));

			if (MEM_NULL != seqCounterLocation)
			{
				#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					buf[0] = 0xC1;
					buf[1] = seqCounterlen;
					ecpMem_Copy(&buf[GET_DATA_TAG_C0_VALUE_OFFSET], seqCounterLocation, seqCounterlen);
					return U16t8(U8_A_U8(seqCounterlen, GET_DATA_TAG_C0_HEAD_LENGTH));
				#else
					ecpMem_Copy(&buf[0], seqCounterLocation, seqCounterlen);
					return seqCounterlen;
				#endif
			}
		}
	}

	return GET_DATA_TAG_C0_HEAD_LENGTH;
}

u16 getKeyNM(u16 appId,u16 keyVersion,u16 keyIndex, PM8 keyBuffer)
{
		return  keyData_getKeyData(U16t8(appId),U16t8(keyVersion),U16t8(keyIndex), PMtMEM(keyBuffer));
}
#endif

void keyManagerFunction()
{
		u8 				funcType;
		u16				sw, sdRef,backCc;
		PN8		sdDataObj;

		#define			InitKey			0
		#define			PUT_KEY			1
		#define			DELETE_KEY		2
		#define			StoreKey		3

		sw				= SW_NO_ERROR;
		funcType		= U16t8(popShort());
		sdRef			= popReference();

		sdDataObj		= MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(sdRef)));
		backCc = cc;
		cc = get_object_owner(sdDataObj);//切cc 使创建的对象属于sdRef，putkey通过opCommand进来时 cc是JCRE

		switch (funcType) {
			case InitKey:
				initIsdKey(sdDataObj);
				break;
			case PUT_KEY:
				sw = PutKeys(sdDataObj);// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP,去掉未使用的参数
				break;
#if GP__KEY_MANAGER__DELETE_KEY
			case DELETE_KEY:
				deleteKey(sdDataObj);
				break;
#endif

#if GP__KEY_MANAGER__STORE_KEY
			case StoreKey:
				sw  =  storeKey(sdDataObj);
				break;
#endif
			default:
				break;
		}
		cc = backCc;
		pushShort(sw);
}
