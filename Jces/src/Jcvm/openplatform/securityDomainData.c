#include "typejava.h"
#include "SecurityDomainData.h"
#include "mask.h"
#include "bc_object.h"
#include "dataManager.h"
#include "object.h"
#include "global_variable.h"
#include "tagNode.h"
#include "keySet.h"
#include "keyGroup.h"
#include "cardmanager.h"
#include "keyData.h"
#include "profile.h"
#include "fm02_keyData.h"
#include "opFunction.h"
#include "fm03_keyData.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#if GP__SCP__SCP11__FOR_MASK
#include "SCP11.h"
#endif


//**************************************************************************************************************************************//
//**************************************************************************************************************************************//
jref sdData_getSDdataRef(jref sdRef)
{
	jref 				sdDataRef;
	PN8					sdObj;
	u16         		backCc;

	sdObj				= MEMtPN8(locateObjectInMemory(sdRef));
	sdDataRef 			= sdData_getSdDataRef(sdObj);

	if (REF_NULL == sdDataRef)
	{
		backCc = cc;
		cc = get_object_owner(sdObj);//切cc 使创建的对象属于sdRef，通过putkey进来的情况 cc是JCRE
		sdDataRef 		= newSecurityDomainDataObject();
		sdData_setSdDataRef(sdObj, sdDataRef);

		sdData_init(MEMtPN8(locateObjectInMemory(sdDataRef)));
		cc = backCc;
	}

	return sdDataRef;
}

jref sdData_getFirstTagNode(PN8 sdDataObj)
{
	jref				firstTagNodeRef;

	firstTagNodeRef 	= sdData_getFirstTagNodeRef(sdDataObj);

	if (REF_NULL == firstTagNodeRef) {
		firstTagNodeRef = newTagNode();
		sdData_setFirstTagNodeRef(sdDataObj, firstTagNodeRef);
	}
	return firstTagNodeRef;
}

/**
 * 获取与当前 scpType 相对应的 keyGroup, 如果keyGroup不存在并且创建。
 */
jref sdData_getKeyGroupRef(PN8 sdDataObj, u8 scpType, u8 inNewKeyGroup)
{
	PN8 keyGroupObj;
	PN8 keyGroupHand;
	jref keyGroupRef;
	u16  backCc;


	keyGroupHand = MEMtPN8(getFirstKeyGroupHand(sdDataObj));

	while (TRUE)
	{
		keyGroupRef = PN_R_U16((u32)keyGroupHand);

		if (REF_NULL == keyGroupRef)
		{
			if (true == inNewKeyGroup)
			{	
				backCc = cc;
				cc = get_object_owner(sdDataObj);//避免不是从sd的上下文直接进来，导致new出来对象不属于sd的问题

				keyGroupRef = newKeyGroup();

				keyGroup_setScpType(MEMtPN8(locateObjectInMemory(keyGroupRef)), scpType);
				nvmWriteU16Atomic(keyGroupHand, keyGroupRef);
				memRefresh();
				cc = backCc;
			}
			break;
		}


		keyGroupObj = MEMtPN8(locateObjectInMemory(keyGroupRef));
		if (scpType == keyGroup_getScpType(keyGroupObj))
		{
			break;
		}
		else
		{
			keyGroupHand = MEMtPN8(keyGroup_getNextKeyGroupHand(keyGroupObj));
		}
	}

	return keyGroupRef;
}
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
void sdData_init(PN8 sdDataObj)
{
	if(U16_AND_U16(cc, APPLET_MASK)) //ssd
	{
		//sdData_setScpType(sdDataObj, JcvmConfig_GetCmScpType());  // modify by xuxiaojun@20211009 去掉SCP重复宏定义
		sdData_setScpType(sdDataObj,  (JcvmConfig_GetCmScpTypeIsSCP03? SCP_03 : SCP_02));  //modify by  jiangqingbin@20211125,修改支持的协议的配置方案
		sdData_setScpOption(sdDataObj, JcvmConfig_GetScpOption());// modify by xuxiaojun@20211009 去掉OPTION重复宏定义
		sdData_setTag82Value(sdDataObj, 0x00);
		sdData_setTag83Value(sdDataObj, 0x00);
		sdData_setTag84Value(sdDataObj, 0x00);
		sdData_setTag87Value(sdDataObj, 0x00);
	#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		sdData_setDESKeyCount(sdDataObj, 0x10);
		sdData_setAuthFailCount(sdDataObj, 0xFF);
	#endif
	}
	else
	{    //isd
		sdData_setScpType(sdDataObj, (JcvmConfig_GetCmScpTypeIsSCP03? SCP_03 : SCP_02));
	#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		sdData_setScpOption(sdDataObj, 0x35);
		sdData_setDESKeyCount(sdDataObj, 0x7F);
		sdData_setAuthFailCount(sdDataObj, 0xFE);
	#else
//		sdData_setScpOption(sdDataObj, 0x75);
		sdData_setScpOption(sdDataObj, JcvmConfig_GetScpOption());
	#endif
	}	
	memRefresh();
}
/*创建se cardmanager的SCPBUFFER ，预置应用没有C9参数 ，单独设置scpbuffer
 * 备注：放在sdData_init里面设置 ，有很多种情况cc=0 ，不能保证每一次都是isd，比如给ISDR put key时（通过OPPlatform.theCardAgent调用的方法都是同样的情况）
 * modified by xuxiaojun@20210414   对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 * modified by jiangqingbin@20211109 修改isd的默认支持的scp协议通过配置的方式写入
 * */
void isdScpData_Init(void)
{
	u8		X_A		tempLen,temp[8];
	jref	X_A		sdRef;
	PM8		X_A		SrcBuf;
	PN8 	X_A		sdDataObj;
	PN8		X_A		sdObj;
	jref 	X_A		sdDataRef;

#define		BYTE_ARRAY_HEADER_SIZE			0x06
	sdRef			= popReference();
	sdObj           = MEMtPN8(locateObjectInMemory(sdRef));
	sdDataRef 		= sdData_getSdDataRef(sdObj);

	sdDataObj		= MEMtPN8(locateObjectInMemory(sdDataRef));

	//if(G_CurExecutingProfileIndex == ILLEGAL_PROFILEINDEX)//se cardmanager ,没有调用C9  在这里创建scpbuffer
	{
		temp[0] = (JcvmConfig_GetCmScpTypeIsSCP03? SCP_03 : SCP_02); 		  //支持 scp03 or scp02
		temp[1] = JcvmConfig_GetScpOption();		//默认协议的默认option值
		tempLen = 2;


#if GP__SCP__SCP11__FOR_MASK
		if (JcvmConfig_GetCmScpTypeIsSupportSCP11)		//支持 scp11
		{
			temp[tempLen++] = 0x11;
			temp[tempLen++] = 0x3F;

			sdData_getSCP11SessionPara(sdDataObj, MODE_CREATE_SCP11_SESSIONPARA); // modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量
		}
#endif

#if JAVA_CARD_UICC
		if (JcvmConfig_GetCmScpTypeIsSupportSCP80)		//支持 scp80
		{
			temp[tempLen++] = 0x80;
			temp[tempLen++] = 0x00;
		}
#endif
		
#if GP__AMENDMENT__B__FOR_MASK		
		if (JcvmConfig_GetCmScpTypeIsSupportSCP81)		//支持 scp81
		{
			temp[tempLen++] = 0x81;
			temp[tempLen++] = 0x07;
		}
#endif

		sdRef	= _newarrayObject(AType_Byte, U8_A_U8(tempLen, BYTE_ARRAY_HEADER_SIZE), BIT_NEED_REFRESH);
		SrcBuf	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(sdRef))));

		nvmCopyAtomic(SrcBuf, MEMtPM8(temp), U8t16(tempLen));
		nvmWriteU16Atomic(get_object_field_ref(sdDataObj, scpBuffer_INDEX), sdRef);
	}

}
//根据appid获取sdDataObj
memref sdData_getsdDataObjByAppId(u8 appId)
{
	jref		X_A			sdRef;
	jref		X_A			sdDataRef;
	u32 		X_A			sdEntryAddr;
	memref					sdDataObj;

	sdEntryAddr 			= getEntryByAppId(appId);
	sdRef 					= nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(sdEntryAddr), theApplet_INDEX)));
	sdDataRef 				= sdData_getSDdataRef(sdRef);
	sdDataObj 				= locateObjectInMemory(sdDataRef);
	return sdDataObj;
}

#if GP__KEY_MANAGER

const u8 KEY_SYMMETRIC_ID1_INFO[] = {0xC0, 0x04, 0x01, 0xFF, 0x80, 0x10};/*	KID KVN KEYTYPE KEYLEN*/
const u8 KEY_SYMMETRIC_ID2_ONFO[] = {0xC0, 0x04, 0x02, 0xFF, 0x80, 0x10};
const u8 KEY_SYMMETRIC_ID3_INFO[] = {0xC0, 0x04, 0x03, 0xFF, 0x80, 0x10};
const u8 KEY_SYMMETRIC_ID4_INFO[] = {0xC0, 0x04, 0x04, 0xFF, 0x80, 0x10};

const u8 KEY_SYMMETRIC_ID1_INFO_FORMAT2[] = {0xC0, 0x0A, 0x01, 0xFF, 0xFF, 0x80, 0x00, 0x10};/*	KID KVN KEYTYPE(2字节) KEYLEN(2字节)*/
const u8 KEY_SYMMETRIC_ID2_INFO_FORMAT2[] = {0xC0, 0x0A, 0x02, 0xFF, 0xFF, 0x80, 0x00, 0x10};
const u8 KEY_SYMMETRIC_ID3_INFO_FORMAT2[] = {0xC0, 0x0A, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x10};
const u8 KEY_SYMMETRIC_ID4_INFO_FORMAT2[] = {0xC0, 0x0A, 0x04, 0xFF, 0xFF, 0x80, 0x00, 0x10};

#if (GP__KEY_MANAGER__FORMAT1 || GP__KEY_MANAGER__FORMAT2)
//
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
*@History : modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
*         : merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 兼容keyUsage两字节
*         : modify by xuxiaojun@20220602 通过get data获取KEY信息时，拼装数据错误。 测试用例：704 gpUICC2Compliance203：km_APDU_storeData_ECC_error_invalid_kvn_00_P256_scp03_9b_ab_32
*/
static u16 writeKeyIdInfo(PX8 buf, PN8 keySetObj, u16 offset, u8 keyId)
{
	u8 keyAttr;
	PN8 dest;
	u8 format;
	u8 keyType;
	u8 keyVersion;
	u8 keyAccess;
	u8 keyAccessLen;
	u8 keyUsageQualifier[2];
	u8 keyUsageQualifierLen;
	/*PX8 * src;
	u8 keyInfoLen;*/
	u8 maskKeyId;
	PN8 keyDataField;
	
	keyType	   = keySet_getKeyType(keySetObj, keyId,THROW_ERROR);//0;

	keyVersion = keySet_getKeyVersion(keySetObj);
	format = keySet_getKeyFormat(keySetObj);
	keyAttr = keyData_getKeyAttr(keySetObj, keyId);
	
	keyDataField = MEMtPN8(keySet_getKeyDataAField(keySetObj));
	maskKeyId = genMaskKeyId(keyDataField, keyId, TAG_GET_KEY);

	if ((KEY_ATTR_DELETE_MASH != U8_AND_U8(keyAttr, KEY_ATTR_DELETE_MASH)) && (0 != keyAttr))
// 	if (KEY_ATTR_DELETE_MASH != (keyAttr & KEY_ATTR_DELETE_MASH))
	{
		dest = PX8_A_U16(buf, offset);

		if (KEY_FORMAT_APDU_2 == U8_AND_U8(format, KEY_FORMAT_APDU_MASH)) /*KID KVN KEYTYPE(2字节) KEYLEN(2字节)*/
		{
			switch (maskKeyId)
			{
				case KEY_ID_01:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID1_INFO_FORMAT2, KEY_INFO_LENGTH_FORMAT2);
					break;
				case KEY_ID_02:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID2_INFO_FORMAT2, KEY_INFO_LENGTH_FORMAT2);
					break;
				case KEY_ID_03:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID3_INFO_FORMAT2, KEY_INFO_LENGTH_FORMAT2);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				case KEY_ID_04:
				ecpMem_Copy(dest, KEY_SYMMETRIC_ID4_INFO_FORMAT2, KEY_INFO_LENGTH_FORMAT2);
#endif
					break;
			}
		}
		else  /*KID KVN KEYTYPE(1字节) KEYLEN(1字节)*/
		{
			switch (maskKeyId)
			{
				case KEY_ID_01:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID1_INFO, KEY_INFO_LENGTH);
					break;
				case KEY_ID_02:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID2_ONFO, KEY_INFO_LENGTH);
					break;
				case KEY_ID_03:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID3_INFO, KEY_INFO_LENGTH);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				case KEY_ID_04:
					ecpMem_Copy(dest, KEY_SYMMETRIC_ID4_INFO, KEY_INFO_LENGTH);
#endif
					break;
			}
		}
		
		PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEY_VERSION_OFFSET), keyVersion);
		if (KEY_FORMAT_02 == U8_AND_U8(format, KEY_FORMAT_STORE_MASH))
		{
			u8 keyIdLen;
			
			//重置keyId
			PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEYID_OFFSET),  keyId);
			
			keyIdLen = U16t8(SECURE_HEAD_BYTE_RESET(PN_R_U8(PNtMEM(PN8_A_U16(keyDataField, FM02_KEY_LENGTH_OFFSET)))));
			keyType	 = keySet_getKeyType(keySetObj, keyId,THROW_ERROR);
			if (KEY_FORMAT_APDU_2 == U8_AND_U8(format, KEY_FORMAT_APDU_MASH))
			{
				PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEY_LENGTH_OFFSET_FORMAT2), keyIdLen);
				PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEY_TYPE_VALUE_OFFSET_FORMAT2), keyType);
			}
			else
			{
				PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEY_LENGTH_OFFSET), keyIdLen);
				PX_W_U8(PX8_A_U8(dest, KEY_INFO_KEY_TYPE_OFFSET), keyType);
			}
		}
		
		if (U8_AND_U8(format, KEY_FORMAT_APDU_MASH) == KEY_FORMAT_APDU_2)
		{
			//key Usage 
			keyUsageQualifierLen = keyData_getKeyUsageQualifier(keySetObj, keyId, &keyUsageQualifier[0]);
			offset = U32t16(U16_A_U8(offset, KEY_INFO_KEY_USAGE_LENGTH_OFFSET));

			PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEY_USAGE_LENGTH_OFFSET), keyUsageQualifierLen);
			U16_INC(offset);

			if(keyUsageQualifierLen == 0x01)
			{
				PX_W_U8(PX8_A_U16(dest, KEY_INFO_KEY_USAGE_VALUE_OFFSET), keyUsageQualifier[0]);
				U16_INC(offset);
			}
			else
			{
				PX_W_U16(PX8_A_U16(dest, KEY_INFO_KEY_USAGE_VALUE_OFFSET), MEM_R_U16((memref)(keyUsageQualifier)));
				U16_INC(offset);
				U16_INC(offset);
			}

			//keyAccess
			keyAccessLen = keyData_getKeyAccess(keySetObj, keyId, &keyAccess);
			PX_W_U8(PX8_A_U16(buf, offset), keyAccessLen);
			U16_INC(offset);
			PX_W_U8(PX8_A_U16(buf, offset), keyAccess);
			U16_INC(offset);
		}
		else
		{
			offset = U32t16(U16_A_U8(offset, KEY_INFO_LENGTH));
		}
	}
	return offset;
}
#endif

u16 writeKeyInfo(PX8 buf, u16 maxSingleLength, PN8 keySetObj, u16 offset)
{
	u8 format;
	u8 keyNum;
	
	u8 keyId;
	u8 keyIdCounter;
	PN8 keyDataField;

	format = keySet_getStoreKeyFormmat(keySetObj);
	switch (format)
	{
#if GP__KEY_MANAGER__FORMAT1
		case KEY_FORMAT_01:
			keyNum = keySet_getKeyNum(keySetObj);
			if (U16_A_U16(offset, U8t16(U8_M_U8(keyNum, KEY_INFO_LENGTH))) > maxSingleLength)
			{
				return 0;
			}

			offset = writeKeyIdInfo(buf, keySetObj, offset, KEY_ID_01);
			offset = writeKeyIdInfo(buf, keySetObj, offset, KEY_ID_02);
			offset = writeKeyIdInfo(buf, keySetObj, offset, KEY_ID_03);
			break;
#endif

#if GP__KEY_MANAGER__FORMAT2
		case KEY_FORMAT_02:
#endif

#if (GP__KEY_MANAGER__FORMAT1 || GP__KEY_MANAGER__FORMAT2)
			keyNum = keySet_getKeyNum(keySetObj);
			if (U16_A_U16(offset, U8t16(U8_M_U8(keyNum, KEY_INFO_LENGTH))) > maxSingleLength)
			{
				return 0;
			}

			if((CONFIG_GP_FOR_BFC)&&(keyNum != 0x03)) //merged from EPX\v2.1\BRANCH_MODULE\FINANCE_TMC_THD89： modify by xuxiaojun@20220621 gpBFCCompliance208: km_APDU_putKeyDes_OPR_da_d5 获取key信息时，不完整key信息整体都不返回
			{                                     //CI测试时时不完整的key是需要返回的
				return 0;
			}

			keyDataField = MEMtPN8(keySet_getKeyDataAField(keySetObj));
			
			for (keyIdCounter = 0; keyIdCounter < MAX_KEY_NUM; U8_INC(keyIdCounter))
			{
				keyId = PN_R_U8(PNtMEM(PN8_A_U16(keyDataField, keyIdCounter)));
				
				if (FM02_MASK_KEYID == keyId)
				{
					continue;
				}
				
				offset = writeKeyIdInfo(buf, keySetObj, offset, keyId);
			}
			break;
#endif

#if GP__KEY_MANAGER__FORMAT3
		case KEY_FORMAT_03:
			offset = FM03_writeKeyInfo(buf, maxSingleLength, keySetObj, offset);
			break;
#endif
		default:
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
			break;
	}

	return U16_INC(offset);
}
/*
*@description:获取sdDataObj所存储的密钥信息
*
*@see        :
*
*@function   :sdData_getKeyInfo
*
*@param      :PX8 buf               --- 输出参数, 获取到的密钥信息
*@param      :u16 maxSingleLength   --- 输入参数，一次性传送的最大长度
*@param      :PN8 sdDataObj         --- 输入参数，JAVA层对应的SecurityDomainData结构体
*@param      :PX16 context          --- 输入输出参数，第一调用该函数时，这个值为0xFFFF，第一个字节表示keyGroup，第二个字节表示keyVersion。
*
*@return     :u16 --- buf中有效数据的长度, 也是当前要发送的长度。如果长度小于maxSingleLength，则认为不需要分成多条发送；如果长度等于maxSingleLength则认为需要还需要发送一次。
*@history    :
*/
u16 sdData_getKeyInfo(PX8 buf, u16 maxSingleLength, PN8 sdDataObj, PX16 context)
{
	jref keyGroupRef;
	jref keySetRef;
	u16 offset;
	u16 counter;
	//u16 tmpBufLen;
	PN8 keySetObj;
	PN8 keyGroupObj;
	u8 keyVersion;
	u8 keyGroup;

	keySetObj = MEM_NULL;
	keyGroupObj = MEM_NULL;
	keyGroupRef = REF_NULL;
	
	if (0 == context[0])
	{
		return 0;
	}


	offset = 0;
	keyGroup = U16t8(U16_AND_U16(U16_SHR_U8(context[0], 8), 0xFF));
	keyVersion = U16t8(U16_AND_U16(context[0], 0xFF));


	if ((0xFF == keyGroup) && (0xFF == keyVersion))
	{
		keyGroupRef = sdData_getFirstKeyGroupRef(sdDataObj);

		if (REF_NULL == keyGroupRef)
		{
			return 0;
		}

		keyGroupObj = MEMtPN8(locateObjectInMemory(keyGroupRef));
		keySetRef = keyGroup_getKeySetRef(keyGroupObj);
	}
	else
	{
		if (0 == keyVersion)
		{
			keyGroupRef = sdData_getKeyGroupRef(sdDataObj, keyGroup, FALSE);
			if (REF_NULL == keyGroupRef)
			{
				return 0;
			}
			
			keyGroupObj = MEMtPN8(locateObjectInMemory(keyGroupRef));
			keyGroupRef = keyGroup_getNextKeyGroupRef(keyGroupObj);
			
			if (REF_NULL == keyGroupRef)
			{
				return 0;
			}
			
			keyGroupObj = MEMtPN8(locateObjectInMemory(keyGroupRef));
			keySetRef = keyGroup_getKeySetRef(keyGroupObj);
		}
		else
		{		
			keySetRef = keyGroup_getKeySet(sdDataObj, keyGroup, keyVersion);
		}
	}


	while (TRUE)
	{
		if (REF_NULL == keySetRef)
		{
			keyGroupRef = keyGroup_getNextKeyGroupRef(keyGroupObj);
			if (REF_NULL == keyGroupRef)
			{
				keySetRef = REF_NULL;
				break;
			}

			keyGroupObj = MEMtPN8(locateObjectInMemory(keyGroupRef));
			keySetRef = keyGroup_getKeySetRef(keyGroupObj);
			
			if (REF_NULL == keySetRef)//added by liuhanren zm; when delete key,cos haven't delete its key group; so keySetRef may be a null pointer
			{
				continue;
			}
		}
		

		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
		counter = writeKeyInfo(buf, maxSingleLength, keySetObj, offset);
		if (counter == 0)
		{
			break;
		}
		else
		{
			offset = counter;
		}

		keySetRef = keySet_getNextKeySetRef(keySetObj);
	}


	if (REF_NULL == keyGroupRef)
	{
		keyGroup = 0;
	}
	else
	{
		keyGroup = keyGroup_getScpType(MEMtPF8(locateObjectInMemory(keyGroupRef)));
	}

	if (REF_NULL == keySetRef) //没有后续 keySet 对象
	{
		keyVersion = 0;
	}
	else
	{
		keyVersion = keySet_getKeyVersion(keySetObj);
	}

	context[0] = (U16_OR_U16(U16_SHL_U8(U8t16(keyGroup), 8), U8t16(keyVersion)));
	return offset;
}



/*
	根据当前指令类型选择当前SCP类型。如果是SCP10指令，可以通过Ioption确定SCP类型。
		
*/

u8 sdData_changeCurrentScpTypeAndOption(PX8 sdDataObj, u8 mode, u8 Ioption)
{
	u8			scpType;
	u8			scpIoption;
	u8			tempI;
	u8 			scpArrayLen;
	jref 		scpBufferRef;
	memref		scpBufferFeild;
	memref 		scpBufferAddr;
	
	// 判断期望的SCP类型与当前SCP类型一致，如果一致则返回。
	scpType = sdData_getScpType(sdDataObj);
	switch(scpType)
	{
		case SCP_01:
		case SCP_02:
		case SCP_03:
		if(SCP0X_WITHOUT_IOPTION_MODE == mode)
		{
			return scpType;
		}
		//break;
		case SCP_80:
		if(SCP80_WITHOUT_IOPTION_MODE == mode)
		{
			return scpType;
		}
		//break;
		case SCP_81:
		if(SCP81_WITHOUT_IOPTION_MODE == mode)
		{
			return scpType;
		}
		//break;
		case SCP_10:
		// if current SCP is SCP10
		if(SCP10_WITH_IOPTION_MODE == mode)
		{
			scpIoption = sdData_getScpOption(sdDataObj);
			if(Ioption == scpIoption)
			{
				return scpType;
			}
		}
		break;
	}
	
	scpBufferRef = sdData_getScpBufferRef(sdDataObj);
	scpBufferAddr = locateObjectInMemory(scpBufferRef);
	scpArrayLen   = U16t8(get_array_length(scpBufferAddr));
	scpBufferFeild = get_array_data(MEMtPF8(scpBufferAddr));
	//scpBufferFeild = sdData_getScpBufferField(sdDataObj);

	//循环查找 SCPBuffer, 检查sd安装时是否支持该种SCP类型，如果支持，修改当前SCP类型。否则返回0XFF
	for(tempI = 0x00; tempI < scpArrayLen ;)
	{
		scpType 	= nvmReadU8(MEMtPN8(scpBufferFeild));
		scpIoption 	= nvmReadU8(MEMtPN8(U32_A_U8(scpBufferFeild, 1)));
		switch(scpType)
		{
			case SCP_01:
			case SCP_02:
			case SCP_03:
			if(SCP0X_WITHOUT_IOPTION_MODE == mode)
			{
				sdData_setScpType(sdDataObj, scpType);
				sdData_setScpOption(sdDataObj, scpIoption);
				return scpType;
			}
			break;
			case SCP_80:
			if(SCP80_WITHOUT_IOPTION_MODE == mode)
			{
				sdData_setScpType(sdDataObj, scpType);
				sdData_setScpOption(sdDataObj, scpIoption);
				return scpType;
			}
			break;
			case SCP_81:
			if(SCP81_WITHOUT_IOPTION_MODE == mode)
			{
				sdData_setScpType(sdDataObj, scpType);
				sdData_setScpOption(sdDataObj, scpIoption);
				return scpType;
			}
			break;
			case SCP_10:
			if( ((SCP10_WITH_IOPTION_MODE == mode) && (Ioption == scpIoption)) || (SCP10_WITHOUT_IOPTION_MODE == mode) )
			{
				sdData_setScpType(sdDataObj, scpType);
				sdData_setScpOption(sdDataObj, scpIoption);
				return scpType;
			}
			break;
		}
		tempI = U16t8(U8_A_U8(tempI, 2));
		scpBufferFeild = U32_A_U8(scpBufferFeild, 2);
	}
	return 0xFF;
	//throw_error(SW_WRONG_DATA);
}

u8 sdData_getScpOptionByScpType(PX8 sdDataObj, u8 scpType)
{
	u8 			scpArrayLen;
	jref 		scpBufferRef;
	memref		scpBufferFeild;
	memref 		scpBufferAddr;
	u8			tempI;
	u8			tmpScpType;

	scpBufferRef = sdData_getScpBufferRef(sdDataObj);

	if(scpBufferRef == REF_NULL)
	{
		return 0xFF; //找不到对应的SCP TYPE
	}
	scpBufferAddr = locateObjectInMemory(scpBufferRef);
	scpArrayLen   = U16t8(get_array_length(scpBufferAddr));
	scpBufferFeild = get_array_data(MEMtPF8(scpBufferAddr));
	//scpBufferFeild = sdData_getScpBufferField(sdDataObj);

	//循环查找 SCPBuffer, 检查sd安装时是否支持该种SCP类型，如果支持，修改当前SCP类型。否则返回0XFF
	for(tempI = 0x00; tempI < scpArrayLen ;)
	{
		tmpScpType 	= nvmReadU8(MEMtPN8(scpBufferFeild));

		if(tmpScpType == scpType)
		{
			return nvmReadU8(MEMtPN8(U32_A_U8(scpBufferFeild, 1)));
		}

		tempI = U16t8(U8_A_U8(tempI, 2));
		scpBufferFeild = U32_A_U8(scpBufferFeild, 2);
	}

	return 0xFF; //找不到对应的SCP TYPE ，option的取值可以是FF吗？？
	//throw_error(SW_REFERENCED_DATA_NOT_FOUND);
}

u32 sdData_getSDTarbufferAddr(PX8 sdDataObj)
{
	u16    X_A         SDTarRef;

	SDTarRef = sdData_getSD_Tar_bufferRef(sdDataObj);

	if(SDTarRef == REF_NULL)
	{
		return 0;//ADDR_IS_NULL;
	}

	return get_array_data(MEMtPF8(locateObjectInMemory(SDTarRef)));
}

#endif

