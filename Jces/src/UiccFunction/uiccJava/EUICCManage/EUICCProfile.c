/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放profile相关的一些函数，如通过profile的属性查找特定的profile等
*
*@history     :V1.0 :create @ 20181106  
*/
#include "EUICCProfile.h"
#include "EUICCCustomer.h"
#include "EUICCCustomerPublic.h"
#include "EUICCMain.h"
#include "ASN1Operation.h"
#include "Object.h"
#include "ToolkitRFM.h"
#include "SEEUICCInterface.h"
#include "hard_api.h"
#include "platform_api.h"
#include "object.h"
#include "bc_array.h"
#include "euiccConfig.h"
#include "SCP03Realize.h"
#include "Interface_LTEFromPlatform.h"
#include "VMAccessEeprom.h"
#include "EUICCInterface.h"
#include "ob_manager.h"
#include "opFunction.h"
#include "SCP03Realize.h"
#include "rtrSpace.h"
#include "profileApi.h"
#if GP__CGM__FOR_MASK
	#include "CgmInit.h"
#endif
#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@function   :GetProfileAppIdByAID
*
*@description:通过AID 获取指定profile中的appId
*
*@param      :BYTE * aidValue   --- aid值
*@param      :BYTE aidLen       --- aidValue中有效数据长度
*@param      :BYTE profileIndex --- 指定的profile Index
*
*@return     :appId/ILLEGAL_APPID
*
*@history    :V1.0 :Create by xuyz@20190222
*/
BYTE GetProfileAppIdByAID(BYTE * aidValue, BYTE aidLen, BYTE profileIndex)
{
	UWORD xdata profileEntryAddr,profileAppTableAddr,appEntry,AidAdd;
	BYTE idata i,profile_beFreeIndex;
	BYTE idata AidLen;
	WORD idata entryRef;

	if(AIDLenInvalid(aidLen))  //aid长度非法
	{
		return ILLEGAL_APPID;
	}
	profileEntryAddr = GetProfileEntryAddr(profileIndex);

	if(profileEntryAddr == ADDR_IS_NULL)
	{
		return ILLEGAL_APPID;
	}

	profileAppTableAddr = GetProfile_theAppTableValueAddr(profileEntryAddr);

	profile_beFreeIndex = nvmReadU16(MEMtPM8(GetProfile_bFreeEntryIndexAddr(profileEntryAddr)));

	for(i = 0x00;i < profile_beFreeIndex;i++)
	{
		entryRef = memReadU16(MEMtPM8(U32_A_U16(profileAppTableAddr,U16_M_U16(i, SIZE_REF))));
		appEntry = locateObjectInMemory(entryRef);

		AidAdd	= locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(appEntry), theAID_INDEX))));

		//when aid is 0; check it immediately
		if(AidAdd == 0) //不是应用
		{
			continue;
		}

		AidAdd = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd), AIDARRAY_INDEX_OF_AID))));
		AidLen = U16t8(get_array_length(MEMtPF8(AidAdd)));
	    AidAdd = get_array_data(MEMtPF8(AidAdd));

	    if((aidLen == AidLen) && (memCompare(MEMtPM8(AidAdd),MEMtPM8(aidValue),AidLen) == ECP_CMP_RESULT_EQUAL))
	    {
	    	return nvmReadU8(get_object_field_ref(MEMtPF8(appEntry), bAppID_INDEX));
	    }

	}

	return 	ILLEGAL_APPID;
}
/*
*@function   :查找一个空闲的profile的位置
*
*@description:GetIdleProfileIndex
*
*@param      :void
*
*@return     :BYTE idleIndex: 返回空闲的profile索引
*
*@author     :xuyz
*
*@history    :
*/
BYTE GetIdleProfileIndex(void)
{
	BYTE idata i ;
	
	for(i = 0;i < PROFILE_MAX_NUMBER;i++)
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + i) == PROFILE_INVALIDFLAG)
		{
			return i;
		}
	}
	
	return  ILLEGAL_PROFILEINDEX;
}
/*
*@function     :GetValidProfileNumber
*
*@description  :获取当前卡片上有效的profileNum，并返回对应的profileIndex
*
*@param        :BYTE * outBuf
*
*@return       :BYTE validProfileNumer
*
*@author        :xuyz
*
*@history       :V1.0 :create @20181122
*/
BYTE GetValidProfileNumber(BYTE *outBuf)
{
	BYTE idata i,profileNum ;
	
	profileNum = 0;
	
	for(i = 0;i < PROFILE_MAX_NUMBER;i++)
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + i) == PROFILE_VALIDFLAG)
		{
			outBuf[profileNum] = i;
			
			profileNum ++;
		}
	}
	
	return  profileNum;
}

/*
*@function     :GetProfileIndexByProfileClass
*
*@description  :获取当前卡片上指定类型并且有效的profile的个数num，并返回对应的profile的index
*
*@see          :
*
*@param        :BYTE profileClass --- 输入数据，表示Profile Class
*@param        :BYTE *outBuf      --- 输出buf，存放查找到的profile的index
*
*@return       :BYTE num          --- 输出数据，返回符合要求的profile的个数
*
*@RAM Resources:
*
*@author       :xuxiaojun@20181119
*
*@test         :
*
*@hisotry      :created by xuxiaojun@20181119
*/
#if RSP_EUICC
BYTE GetProfileIndexByProfileClass(BYTE profileClass, BYTE * outBuf)
{
	BYTE idata i;
	BYTE idata num;
//	UWORD xdata profileMetaDataAddr;
	
	num = 0;
	
	for(i = 0;i < PROFILE_MAX_NUMBER;i++)
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + i) == PROFILE_VALIDFLAG)
		{
			if(profileClass == GetSingleBYTEProfileMeta(i,MetaDataProfileClass_INDEX,2))/*profileMetaDataAddr + MetaData_ProfleClassValue_Offset)*/
			{
				if(outBuf != NULL)
				{
					outBuf[num ] = i;
				}
				
				num ++;									
			}
			
		}
	}
	
	return num;
}

/*
*@function     :GetProfileIndexByICCID
*
*@description  :通过ICCID 查找指定的profile
*
*@param        :BYTE ×iccidBuf --- 输入数据，10字节的iccid
*
*@return       :BYTE           --- 输出数据，存在返回profileIndex/不存在返回ILLEGAL_PROFILEINDEX
*
*@RAM Resources:
*
*@author       :xuyz@20181119
*
*@test         :
*
*@hisotry      :created by xuyz@20181119
*/
BYTE GetProfileIndexByICCID(BYTE *iccidBuf)
{
	BYTE idata i;
	UWORD xdata profileMetaDataAddr;	
	
	for(i = 0;i < PROFILE_MAX_NUMBER;i++)
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + i) == PROFILE_VALIDFLAG)
		{
			profileMetaDataAddr = GetProfileMetaDataAddr(i);//GetAddrByIndexAddr(PROFILE_METADATAREF + i*sizeof(WORD));
			
			profileMetaDataAddr = GetMetaData_iccidAddr(profileMetaDataAddr)+2;//偏移开TL字节

			if(NvmCmp((BYTE *)(profileMetaDataAddr),iccidBuf ,ICCID_LEN) == EQUAL)
			{			
				return i;								
			}						
		}
	}
	
	return ILLEGAL_PROFILEINDEX;
}
#endif
/*
*@function   :GetProfileIndexByAIDAndState
*
*@description:通过ISD P AID查找执行的profileIndex，只能在有效的profile内查找
*
*@param      :BYTE *AIDBuf  --- 指定的ISD-P的AID
*@param      :BYTE aidLen   --- 指定的ISD-P的AID的长度，也就是AIDBuf中有效数据的长度
*@param      :BYTE  profiletValidTag :profile的是否有效tag
*
*@return     :未找到返回ILLEGAL_PROFILEINDEX / 找到就返回对应的profileIndex
*
*@author     :xuyz@20181128
*
*@history    :created by xuyz@20181128
*/
BYTE GetProfileIndexByAIDAndState(BYTE *aidBuf, BYTE aidLen,BYTE profiletValidTag)
{
	BYTE idata i, len;
	BYTE idata profileState;
	BYTE  xdata AIDBuf[16];

	for(i = 0;i < PROFILE_MAX_NUMBER;i++)
	{
		profileState = VMReadByte(PROFILE_VALIDTAG_CFG + i);

		if((profileState == profiletValidTag) )
		{
			len = GetProfileAIDByProfileIndex(i,AIDBuf);

			if((len == aidLen) && (EQUAL == NvmCmp((BYTE *)(AIDBuf), aidBuf, len)))
			{
				return i;
			}
		}
	}
	return ILLEGAL_PROFILEINDEX;
}

/*
@fucntion     :GetProfileAIDByProfileIndex
*
*@description :通过profileIndex获取profile的ISD-P AID ,此处保证profileindex一定是有效的profile的index
*
*@param       :BYTE profileIndex   --- 输入参数，需要获取AID的profile Index
*@param       :BYTE *aidBuf        --- 输出参数，用于输出获取到的AID
*
*@return      :BYTE aidLen         --- 返回AID的长度，也就是aidBuf中的有效数据长度
*
*@author      :xuyz@20181227
*
*@history     :created by xuyz@20181227
*             :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE GetProfileAIDByProfileIndex(BYTE profileIndex, BYTE *aidBuf)
{
	BYTE idata arrayLen,index;
	UWORD xdata profileEntryAddr,profileAppTableAddr;
	UWORD xdata theAppEntry,aidAddr;
	WORD idata aidRef;

	if(ProfileIndexIsValid(profileIndex) == FALSE)
	{
		return 0;
	}
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *updateMetadata过程：涉及到RAM方式执行install for personal + store data两条指令。
	 *如果先执行了install for personal，再执行memoryReset，再执行store data，此时store data里的G_CurEnabledProfileIndex已经被删除了
	 */
	profileEntryAddr = GetProfileEntryAddr(profileIndex);

	if(ADDR_IS_NULL == profileEntryAddr)
	{
		return 0;
	}

	profileAppTableAddr = GetProfile_theAppTableValueAddr(profileEntryAddr);

	for(index=0;index<APP_END;index++)
	{
		theAppEntry = locateObjectInMemory(PN_R_U16(U32_A_U16(profileAppTableAddr, U16_M_U16(index, SIZE_REF))));
		if(PN_R_U8(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), bAppID_INDEX))) == ISDP_APPID)
		{
			aidAddr = locateObjectInMemory(PN_R_U16(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), theAID_INDEX))));
			aidRef = PN_R_U16(PFtMEM(get_object_field_ref(aidAddr, AIDARRAY_INDEX_OF_AID)));

			aidAddr = locateObjectInMemory(aidRef);

			arrayLen = get_array_length(aidAddr);

			aidAddr = (get_array_data(MEMtPF8(aidAddr)));

			VMReadBuf(aidBuf, aidAddr, arrayLen);

			return arrayLen;
		}
	}

	return 0;
}
/*
*@function   :GetProfilePOL1
*
*@description:获取profile的POL1的值
*
*@param      :BYTE profileIndex: 传入的profileIndex一定是有效的profile索引.profile的状态目前存放于PROFILE_STATE_CFG 字段中
*
*@return     :BYTE POL1 返回之后不需要检查POL1的合法性，该值在存入时进行合法性的检查
*
*@author     :xuxiaojun@20181218
*
*@history    :created by xuxiaojun@20181218
*/

#if M2M_EUICC
BYTE GetProfilePOL1(BYTE profileIndex)
{
	UWORD xdata entryAddr;

	entryAddr = GetProfile_M2M_POL1Addr(GetISDPObjectAddr(profileIndex));//GetProfile_M2M_POL1Addr(GetProfileEntryAddr(profileIndex));//

	return VMReadByte(entryAddr);
}
#endif

/*
*@function   : GetProfileState
*
*@description: 获取profile的状态
*
*@param      : BYTE profileIndex: 传入的profileIndex一定是有效的profile索引.profile的状态目前存放于PROFILE_STATE_CFG 字段中
*
*@return     : BYTE profileState: PROFILE_STATE_ENABLED / PROFILE_STATE_DISABLED
*
*@author     :
*
*@history    :
*/
BYTE GetProfileState(BYTE profileIndex)
{
	if(JcvmConfig_GetProfileEnableIndex() == profileIndex)
	{
		return PROFILE_STATE_ENABLED;
	}
	
	return PROFILE_STATE_DISABLED;
}
/*
*@function   : IsProfileValid
*
*@description:判断当前profile是否有效
*
*@param      : BYTE profileIndex: 传入的profileIndex一定是有效的profile索引.profile的有效性标记存在于目前存放于PROFILE_VALIDTAG_CFG 字段中
*
*@return     : profile无效返回FALSE，否则返回TRUE
*
*@author     :
*
*@history    :
*/

BYTE IsProfileValid(BYTE profileIndex)
{
	if(profileIndex > PROFILE_MAX_NUMBER)
	{
		return FALSE;
	}
	
	if(VMReadByte(PROFILE_VALIDTAG_CFG + profileIndex) == PROFILE_INVALIDFLAG)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
		
	
}
/*
*@function   : GetSingleBYTEProfileMeta
*
*@description: 获取profilemedata字段中单字节的元素
*
*@param      : BYTE profileIndex: 传入的profileIndex一定是有效的profile索引.
×@param      : WORD metaDataIndex :需要取的数据Index
×@param      : BYTE sunOffset :需要取的数据的V在TLV中的偏移
*
*@return     : BYTE result
*
*@author     :
*
*@history    :

*/
#if RSP_EUICC
BYTE GetSingleBYTEProfileMeta(BYTE profileIndex, WORD metaDataIndex,BYTE subOffset)
{
	UWORD xdata profileMetaDataAddr;

	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex*sizeof(WORD));

	profileMetaDataAddr = GetMetaDataItemAddr(profileMetaDataAddr , metaDataIndex);

	return VMReadByte(profileMetaDataAddr + subOffset);
			
}
/*
*@function   :GetProfileMetadaTLV
*
*@description: 个人版： 获取medata种TLV格式的数据，主要如：iccid iconType profileClass profilePolicyRules这些字段
*
*@param      :BYTE profileIndex
*@pram       :WORD tlvoffset
*@param      :BYTE *outBuf  :该buf一定不可以超过获取的TLV的最大长度，，medata中最大的TLV 字段为12字节
*
*@return     :TRUE:字段存在/FALSE: 字段不存在
*
*@author     :xuyz
*
*@history     :V1.0 :Create @20190228
*/
BYTE GetProfileMetadaTLV(BYTE profileIndex, WORD tlvoffset, BYTE *outBuf)
{
	BYTE idata valueLen;
	UWORD xdata profileMetaDataAddr;
	
	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex * sizeof(WORD));  //指定的profile的参数 区域 所在的位置
	
	if(profileMetaDataAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}
	
	profileMetaDataAddr = GetMetaDataItemAddr(profileMetaDataAddr,tlvoffset);

	valueLen = VMReadByte(profileMetaDataAddr +1);
	
	if(valueLen == 0x00)
	{
		return FALSE;
	}
	
	VMReadBuf(outBuf ,profileMetaDataAddr , valueLen+2);
	
	return TRUE;
}
/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE ProfileOwnerIsExist(BYTE profileIndex)
{
	UWORD xdata profileMetaDataAddr,dataAddr;

	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex * sizeof(WORD));  //指定的profile的参数 区域 所在的位置

	if(profileMetaDataAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}

	dataAddr = GetMetaData_profileOwerRefValueAddr(profileMetaDataAddr);//GetAddrByIndexAddr(profileMetaDataAddr + MetaData_ProfileOwerRef_Offset);   //profile中指定的参数所在的位置

	if(dataAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}

	return TRUE;
}

/*
*@function   :GetProfileOwnerSubField
*
*@description:获取profileOwner OperatorId 中的子字段，如 mccMnc gid1 gid2等
*
*@param      :BYTE profileIndex
*@param      :BYTE subFieldType: OperatorId_MCCMNC_TAG/OperatorId_GID1_TAG/OperatorId_GID2_TAG
*@param      :BYTE *outBuf :目前默认传入的outBuf为tempBuf，所以该函数内部会判断是否会越界tempBuf
*
*@return     :length:TLV的总长度/ FALSE
*
*@author     :xuyz
*
*@history     :V1.0 :create @20190228
*/
WORD GetProfileOwnerSubField(BYTE profileIndex,BYTE subFieldType, BYTE *outBuf,BYTE mode)
{
	BYTE idata tmp,tag;
	WORD idata off,totalLen;
	UWORD xdata tmpLen,len,profileMetaDataAddr,dataAddr;
	
	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex * sizeof(WORD));  //指定的profile的参数 区域 所在的位置
	
	if(profileMetaDataAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}
	
	dataAddr = GetMetaData_profileOwerRefValueAddr(profileMetaDataAddr);//GetAddrByIndexAddr(profileMetaDataAddr + MetaData_ProfileOwerRef_Offset);   //profile中指定的参数所在的位置
	
	if(dataAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}
	tmp = ASN1_GetBERTlvLen(dataAddr + 1 , (UWORD*)&len );
	
	dataAddr += 1+ tmp;  //point the startOff Value
	
	for(off =0 ; off < len; )
	{
		tag = VMReadByte(dataAddr + off);
		
		tmp = ASN1_GetBERTlvLen(dataAddr + off + 1 , (UWORD*)&tmpLen );
		
		totalLen = tmpLen + tmp + 1;  //该字段所占的总的字节数，TLV 的总所占的字节数
		
		if(tag == subFieldType)
		{
			if(outBuf != NULL)
			{
				if(totalLen >sizeof(tempBuf))
				{
					totalLen = sizeof(tempBuf);
				}
				if(mode == 1)//写
				{
					VMWriteBuf(dataAddr+off, outBuf, totalLen) ;
				}
				else//读
				{
					VMReadBuf(outBuf,dataAddr+off,  totalLen) ; // 返回TLV字段 ，此处要注意outBuf 可能越界哦
				}
							
				return totalLen;
			}
			else
			{
				return TRUE;
			}
		}
		
		off += totalLen;  //point to the Next Tag
	}
	
	return FALSE;
}
/*
*@function   :StoreProfileMetadataUnspecifiedTagRef
*
*@description:对于某一个profile，更新或添加指定的metadata unspecified tag
*
*@param      :UWORD dataBuf: TLV结构, T可能是1字节tag, 也可能是2字节tag
*@param      :BYTE profileIndex :profileIndex一定是有意义的
*
*@return     :BYTE --- 存储成功返回TRUE/存储失败返回FALSE
*
*@history    :V1.0 :created by xuxiaojun@20190521
*/
BYTE StoreProfileMetadataUnspecifiedTagRef(UWORD dataBuf, BYTE profileIndex)
{
	UWORD xdata lenValue,dataAddr,profileMetaDataAddr, metaDataAddr;
	BYTE idata i, tagLen, lenLen, transActioinFlag;
	WORD idata  totalLen, offset, ref, tmp;
	BYTE idata tag[2];

	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);

	metaDataAddr = GetMetaData_unspecifiedTagAddr(profileMetaDataAddr);

	ref = memReadU16(MEMtPM8(metaDataAddr));

	for(i=0x00; i < UNSPECIFIED_TAG_NUMBER;i++)
	{
		metaDataAddr = GetMetaData_unspecifiedTagValueAddr(profileMetaDataAddr) + i*sizeof(WORD);

		ref = memReadU16(MEMtPM8(metaDataAddr));

		if(ref == REF_NULL)
		{
			break;
		}
	}

	if(i == UNSPECIFIED_TAG_NUMBER)
	{
		return FALSE;
	}

	tagLen = ASN1_GetBERTlvTag(dataBuf, tag);

	lenLen = ASN1_GetBERTlvLen(dataBuf + tagLen, (UWORD*)&lenValue);

	transActioinFlag = IF_BeginTransaction();

	totalLen = (tagLen + lenLen + lenValue);

	dataAddr = MakeProfileArrayObject(getISDPISDRContext(ISDP_PkgId,ISDP_APPID),totalLen,&ref,profileIndex);

//	if(ADDR_IS_NULL == dataAddr)
//	{
//		IF_AbortTransaction(transActioinFlag);

//		return FALSE;
//	}

	for(offset = 0; offset < totalLen;  )
	{
		if((totalLen - offset) > sizeof(tempBuf))
		{
			tmp = sizeof(tempBuf);
		}
		else
		{
			tmp = (totalLen - offset);
		}

		VMReadBuf(tempBuf, (UWORD)(dataBuf + offset), tmp);

		VMWriteBuf(dataAddr + offset, tempBuf , tmp);

		offset += tmp;
	}

	memWriteU16Atomic(MEMtPM8(metaDataAddr),ref);//VMWriteBuf(metaDataAddr, (BYTE *)&ref, sizeof(WORD));	//更新参数引用到指定的偏移字段

	IF_CommitTransaction(transActioinFlag);
	
	return TRUE;	
}
/*
*@function   :StoreProfileMetadataRef
*
*@description：对于某一个profile，更新或添加指定的metadata数据
*
*@param      :BYTE *dataBuf: TLV结构。T 仅支持一个tag的
×@param      :WORD totalLen  : 该TLV所占的总长度
*@param      :WORD refOffset
*@param      :BYTE profileIndex :profileIndex一定是有意义的
*
*@return     :BYTE --- 存储成功返回TRUE/存储失败返回FALSE
*
*@author     :xuyz
*
*@history    :V1.0 :create@20190104
*/
BYTE StoreProfileMetadataRef(BYTE *dataBuf, WORD totalLen, WORD refOffset, BYTE profileIndex )
{
	WORD idata ref,offset,tmp;
	BYTE idata transActioinFlag;
	UWORD xdata dataAddr,profileMetaDataAddr,metaDataAddr;
	
	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex * sizeof(WORD));  //指定的profile的参数 区域 所在的位置
	
	metaDataAddr = GetMetaDataItemAddr(profileMetaDataAddr,refOffset);//GetAddrAndIndexByIndexAddr(profileMetaDataAddr + refOffset, &ref);   //profile中指定的参数所在的位置

	ref = memReadU16(MEMtPM8(metaDataAddr));
	
	transActioinFlag = IF_BeginTransaction();
	
	if(ref != REF_NULL)
	{	
//		tmp = ASN1_GetBERTlvLen((UWORD)(dataAddr + 1) , (UWORD*)&tmpLen );
//
//		VMCOSHeapFree(ref, tmpLen + tmp +1 ,AppMemory_Mode);

		ref = REF_NULL;  //清空引用
		
		memWriteU16Atomic(MEMtPM8(metaDataAddr),ref);//VMWriteBuf(profileMetaDataAddr + refOffset, (BYTE *)&ref, sizeof(WORD));

		//打垃圾回收标记？
	}
	
	if(totalLen != 0x02)  //更新新的value值到对应的参数里
	{
		dataAddr = MakeProfileArrayObject(getISDPISDRContext(ISDP_PkgId,ISDP_APPID),totalLen,&ref,profileIndex);

//		if(ADDR_IS_NULL == dataAddr)
//		{
//			IF_AbortTransaction(transActioinFlag);

//			return FALSE;
//		}
		
		for(offset = 0; offset <totalLen;  )
		{
			if((totalLen - offset) > sizeof(tempBuf))
			{
				tmp = sizeof(tempBuf);
			}
			else
			{
				tmp = (totalLen - offset);
			}
			
			VMReadBuf(tempBuf, (UWORD)(dataBuf + offset), tmp);
			
			VMWriteBuf(dataAddr + offset, tempBuf , tmp);
			
			offset += tmp;
		}

		memWriteU16Atomic(MEMtPM8(metaDataAddr),ref);//VMWriteBuf(profileMetaDataAddr + refOffset, (BYTE *)&ref, sizeof(WORD));	//更新参数引用到指定的偏移字段
	}
	else
	{
		//totalLen = 2 意味 着 filed存在但是 空值，if a field is present with an empty value, the eUICC SHALL delete the actual value from the Profile Metadata
	}
	
	IF_CommitTransaction(transActioinFlag);
	
	return TRUE;
}
/*
*@function   :StoreProfileMetadataTLV
*
*@description：对于某一个profile，更新metaData中的TLV数据
*
*@param      :BYTE *dataBuf: TLV结构。T 仅支持一个tag的
×@param      :BYTE totalLen  : TLV所占的长度, 输入数据保证有效性，该函数内部不判断数据的有效性 ，如长度 tag等
*@param      :WORD tlvOffset
*@param      :BYTE profileIndex :profileIndex一定是有意义的
*
*@return     :BYTE --- 存储成功返回TRUE/存储失败返回FALSE
*
*@author     :
*
*@history    :modify by xuxiaojun@20180114 增加判断，理论上profileMetaDataAddr是不会为ADDR_IS_NULL的，这里增加判断避免坏卡
*/
BYTE StoreProfileMetadataTLV(BYTE *dataBuf, BYTE totalLen, WORD tlvIndex, BYTE profileIndex )
{
	UWORD xdata profileMetaDataAddr,MetaDataAddr;

	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex * sizeof(WORD));  //指定的profile的参数 区域 所在的位置
	
	if(profileMetaDataAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}
	
	MetaDataAddr = GetMetaDataItemAddr(profileMetaDataAddr,tlvIndex);

	VMWriteBuf(MetaDataAddr , dataBuf , totalLen);
	
	return TRUE;
}

#endif

/*
*@function   :DownloadOrDeleteProfileResume
*
*@description:断电时，判断之前是否有profile正在被删除，如果有，则继续未删除的操作；断电时，判断之前是否有profile正在被下载，如果有，则删除该profile相关信息
*
*@pram       :void
*
*@return     :void
*
*@history    :modify by xuxiaojun@20190718  上电之后必须进行垃圾回收，原因是如果在DeleteProfileDataByProfileIndex的PROFILE_VALIDTAG_CFG被写成FF之后，且垃圾回收未完成之前断电，会造成以下后果：
*                                           1. 没有合适的时机进行垃圾回收，导致实际可用于分配的空间减小，如果一直这样断电，则最终会导致空间分配失败，profile下载不成功
*                                           2. 重新下载一个新的profile时，新profile的index如果与被删除的profile的index相同，那么上一次没有回收的属于旧profile的垃圾对象会被认为是新的profile的有效对象
*            :modify by xuxiaojun@20230329  <代码评审问题跟踪表  - 20230327>如果断电之前是删除profile操作，重新上电后进入该函数之前文件系统已经被赋了有效值，执行该该操作之后文件系统可能发生搬移，所以需要对文件进行备份还原
*                                           
*/
void DownloadOrDeleteProfileResume(void)
{
	BYTE idata i;
	BYTE xdata tmp[PROFILE_MAX_NUMBER];
	BYTE idata deleteFlag;
	WORD idata fileName[PROFILEDELETE_BACKUPFILESYSTEM];

	deleteFlag = 0x00;
	
	VMReadBuf(tmp, PROFILE_VALIDTAG_CFG, PROFILE_MAX_NUMBER);
	
	for(i= 0; i< PROFILE_MAX_NUMBER; i++)
	{
#if RSP_EUICC
		if((tmp[i] == PROFILE_LOADINGFLAG) || (tmp[i] == PROFILE_DELETELAG))
#else
		if(tmp[i] == PROFILE_LOADINGFLAG)
		{
			DeleteProfileDataByProfileIndex(i, MODE_DELETE_PROFILE_PE);  
		}		
		else if(tmp[i] == PROFILE_DELETELAG)
#endif			
		{
			if(deleteFlag == 0x00)
			{
				IF_ProfielDeletebackFileName(fileName);

				deleteFlag = 0x01;
			}

			DeleteProfileDataByProfileIndex(i, MODE_DELETE_PROFILE);
		}		
	}

	if(deleteFlag == 0x01)
	{
		IF_ProfielDeleteinit(fileName);
	}
}
/*
*@function     :DeleteProfileDataByProfileIndex
*
*@description  :根据ProfileIndex删除对应的profile 的所有数据
*
*@see          :
*
*@param        :BYTE deleteProfileIndex --- 需要被删除的profile的Index
*@param        :BYTE mode               --- 删除模式MODE_DELETE_PROFILE表示删除整个profile; MODE_DELETE_PROFILE_PE表示删除profile PE数据
* 
*@return       :TRUE:删除成功/FALSE:删除失败
*
*@RAM Resources:
*
*@author       :
*
*@hisotry      :该函数要考虑删除profile要么全部成功，要么全部失败的操作
*              :modify by xuxiaojun@20190319 将VMSetBuf(PROFILE_MFOFFSET_CFG + deleteProfileIndex*sizeof(UWORD), 0xFF, 4);操作交给IF_DeleteFilesystemByProfileIndex去处理
*              :modify by xuxiaojun@20190324 将SEEUICCVariableInit的调用移动到HeapPieceCleanUp内部 <按照顺序执行TY_OfflineProfileFormatTest的MF_TEMPLATE_E05->MF_TEMPLATE_E08->MF_TEMPLATE_E05->MF_TEMPLATE_E08，然后就会死卡>
*              :modify by xuxiaojun@20190717 修改BUG:如果VMSetBuf(PROFILE_APPTable_REF + deleteProfileIndex*sizeof(WORD),REF_NULL,sizeof(WORD));之后SetPageCacheEX(PROFILE_VALIDTAG_CFG + deleteProfileIndex, PROFILE_INVALIDFLAG, sizeof(BYTE));之前断电的话，则PROFILE_VALIDTAG_CFG标记的值永远都无法清掉
*/
//是否需要在这里考虑释放当前正在下载的cap包？？？
void DeleteProfileDataByProfileIndex(BYTE deleteProfileIndex, BYTE mode)
{
	UWORD    xdata     profileEntry;
	WORD     idata     entryRef;

	//if(deleteProfileIndex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(deleteProfileIndex) == FALSE)
	{
		return ;
	}
	
	IF_DeleteFilesystemByProfileIndex(deleteProfileIndex); 	//todo ：检查当前是否有文件系统需要移动删除MF，注意如果当前激活的profile在将被删除的profile的后面，那么移动文件系统将会导致某些文件系统的全局变量发生变化，需要同步

	profileEntry = locateObjectInMemory(memReadU16(MEMtPN8(Profile_Entry_JC)));//Profile_Entry[]

	profileEntry = get_array_data(MEMtPN8(profileEntry));

	entryRef = memReadU16(MEMtPN8(PN8_A_U16(profileEntry,U8_M_U8(deleteProfileIndex,SIZE_REF))));

	if(entryRef != REF_NULL)
	{


#if  M2M_EUICC
		if(mode == MODE_DELETE_PROFILE_PE)//删除PE ，保留ISD-P以及profileEntry创建的各种表（profile内部：删ISD-P以外的所有应用、所有包）
		{
			DeleteProfilePE(deleteProfileIndex);//可根据CopySETableToProfile()备份的信息还原，保留ISD-P的APPentry为下载profile时创建的值，同时将表里的其他数据清掉
		}
		else
#endif
		{//删除包括ISD-P在内的整个profile
			profile_gc(deleteProfileIndex,entryRef);
			memWriteU16Atomic(PM8_A_U16(MEMtPM8(profileEntry),U8_M_U8(deleteProfileIndex,SIZE_REF)), REF_NULL);

		}
	}


	if(mode != MODE_DELETE_PROFILE_PE)
	{
		memWriteU8Atomic(PM8_A_U8(MEMtPM8(PROFILE_VALIDTAG_CFG), deleteProfileIndex), PROFILE_INVALIDFLAG);
	}
//modify by xuxiaojun@20220916: bug 429下载profile过程中有下载CAP包，此时回滚的话Profile_pkgLocationTable中的引用没有被释放，测试用例：eSIM_Test_M2M_04/eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03的第13个profile
	if((mode != MODE_DELETE_PROFILE_PE)&&(INVALID_ALLOC_RAM_ADDRESS != RamManager_refHasBeenAllocated(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)))))//清RTR SPACE
	{
		RamManager_free(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)));
	}

}
/*
*@function     :DeleteProfileByProfileIndex
*
*@description  :根据ProfileIndex删除对应的ISD-P ,个人版在删除profile之前开事务打上了PROFILE_DELETELAG标记，所以直接调用DeleteProfileDataByProfileIndex
				设备版调用DeleteProfileByProfileIndex函数 先打上PROFILE_DELETELAG再删除，避免断电无法恢复 
*
*@see          :
*
*@param        :BYTE deleteProfileIndex
*
*@return       :TRUE:删除成功/FALSE:删除失败
*
*@RAM Resources:
*
*@author       :xuyz
*
*@hisotry      :该函数要考虑删除profile要么全部成功，要么全部失败的操作
*/
#if M2M_EUICC
void DeleteProfileByProfileIndex(BYTE deleteProfileIndex)
{		
	WORD  idata fileName[PROFILEDELETE_BACKUPFILESYSTEM];

	if(ILLEGAL_PROFILEINDEX != deleteProfileIndex)
	{
		IF_ProfielDeletebackFileName(fileName);

		VMSetBuf(PROFILE_VALIDTAG_CFG + deleteProfileIndex, PROFILE_DELETELAG, 1);  
	
		DeleteProfileDataByProfileIndex(deleteProfileIndex, MODE_DELETE_PROFILE);
		
		IF_ProfielDeleteinit(fileName);

//	打垃圾回收标记 ？GarbageCollectorAndHeapPieceCleanUp();
	}	
}
#endif
/*
*@fuction    :SetProfileInfoAfterLoadPE
*
*@description:在成功执行PE下载以后 更新PE的一些信息 ，比如CI PKID等
*
*@param      : BYTE profileIndex
*
*@return     :NULL
*
*@author     :lgy
*
*@history    :V1.0 @20190114
*/
void SetProfileInfoAfterLoadPE(BYTE profileIndex)
{	
	BYTE idata transactionTag;

#if RSP_EUICC
	UWORD xdata addr/*,ProfileEntryAddr*/ ;
	BYTE idata len;
	WORD idata index;
#endif
	
	transactionTag = IF_BeginTransaction();
	
#if RSP_EUICC	  // 只有个人版才需要更新PKID
	
	len = VMReadByte(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off);//L 
	
	if(len != 0x00)
	{	
		addr = MakeProfileArrayObject(getISDPISDRContext(ISDP_PkgId,ISDP_APPID),len + 1,&index,profileIndex);
		
//		if(addr == ADDR_IS_NULL)
//		{
//			IF_AbortTransaction(transactionTag);
//
//			return;
//		}
		
		VMReadBuf(tempBuf,EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off,len + 1);//LV
		
		VMWriteBuf(addr,tempBuf,len + 1);
		
		addr = GetProfile_PkIdRefAddr(GetISDPObjectAddr(profileIndex));//GetProfile_PkIdRefAddr(ProfileEntryAddr);//

		memWriteU16Atomic(MEMtPM8(addr),index);//VMWriteBuf(PROFILE_PKIDREF + profileIndex*sizeof(WORD),(BYTE*)&index,2);

#if EUICC__FUNCTION__GUOMI_eSIM
		SetCertType(profileIndex, G_CertType);
#endif
	}
	
#endif
	
	VMSetBuf(PROFILE_VALIDTAG_CFG + profileIndex, PROFILE_VALIDFLAG, 1);  //设置当前profile的状态为 VALIDFLAG  状态

//	VMReadBuf(tempBuf, PROFILE_DEFAULT_APP_TABLE + profileIndex * VM_MAX_LOGICAL_CHANNELS, VM_MAX_LOGICAL_CHANNELS);  //读取通道的默认应用
	
/*	for(i=0 ;i <VM_MAX_LOGICAL_CHANNELS_CT ; i++ )  //将接触通道上的应用修改成UICC
	{
		if((tempBuf[i] == ISDR_APPID)||(tempBuf[i] == MNOSD_APPID)) //要注意安装MNO SD 后，将默认应用都修改成MNO SD 即 EUICC上的 ISD. 这个时候，profile还没有激活，ISD_APPID此时指向的是ISD-R，但是此时读取到的通道上的默认应用已经被设置成了MNO-SD
		{
			tempBuf[i] = UICC_APPID;
		}
	}

	VMWriteBuf(PROFILE_DEFAULT_APP_TABLE + profileIndex * VM_MAX_LOGICAL_CHANNELS,tempBuf, VM_MAX_LOGICAL_CHANNELS);
*/
	IF_CommitTransaction(transactionTag);
	
}
/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
WORD GetProfileEntryRef(BYTE profileIndex)
{
	UWORD    xdata     profileEntry;
	WORD     idata     entryRef;

	if(ProfileIndexIsValid(profileIndex) == FALSE)
	{
		return REF_NULL;
	}

	profileEntry = locateObjectInMemory(memReadU16(MEMtPN8(Profile_Entry_JC)));//Profile_Entry[]
	if(profileEntry == MEM_NULL)//这个代码理论上是不会成立的，不知道谁为什么增加
	{
		return ADDR_IS_NULL;
	}
	profileEntry = get_array_data(MEMtPN8(profileEntry));

	entryRef = memReadU16(MEMtPN8(PN8_A_U16(profileEntry,U8_M_U8(profileIndex,SIZE_REF))));

	return entryRef;
}
/*获取ProfileEntry对象的地址*/
UWORD GetProfileEntryAddr(BYTE profileIndex)
{
//	UWORD    xdata     profileEntry;
//	WORD     idata     entryRef;

	if(ProfileIndexIsValid(profileIndex) == FALSE)
	{
		return ADDR_IS_NULL;
	}
//
//	profileEntry = locateObjectInMemory(memReadU16(MEMtPN8(Profile_Entry_JC)));//Profile_Entry[]
//
//	profileEntry = get_array_data(MEMtPN8(profileEntry));
//
//	entryRef = memReadU16(MEMtPN8(PN8_A_U16(profileEntry,U8_M_U8(profileIndex,SIZE_REF))));
//
//	profileEntry = locateObjectInMemory(entryRef);
//
//	return profileEntry;

	if(GetProfileEntryRef(profileIndex) == REF_NULL)  // 复位之前执行了enable profile的操作，refresh模式是reset模式，但是没有执行reset之前又把该profile给删掉了，导致该profile的entry不存在，程序却按照存在的情况继续往下走，访问了非法地址i  buglist16
	{
		return ADDR_IS_NULL;
	}

	return locateObjectInMemory(GetProfileEntryRef(profileIndex));
}

/*获取对象内部元素所在的地址*/
UWORD GetObjectItemAddr(UWORD ObjectAddr,BYTE itemIndex)
{
	if(ObjectAddr == ADDR_IS_NULL)
	{
		return ADDR_IS_NULL;
	}
	return get_object_field_ref(ObjectAddr,itemIndex);
}

/*获取某个对象内的数组成员的地址，指向数组成员的数据体
 * 获取class ProfileEntry内部数组对象的地址,数组头已偏移开 指向数组第一个内容
 * Profile_theAppTable[],Profile_iAppIdFlag[],Profile_iMutiFlag[],
 * Profile_iExtraditonFlag[],profile_pkgTable[],profile_pkgLocationTable[],
 * Profile_packageContextTable[]
 * */
UWORD GetObject_ArrayItemAddr(UWORD ObjectAddr,BYTE itemIndex)
{
	UWORD xdata entryTableAddr;
	WORD idata  itemRef;

	itemRef = get_object_reference_field(ObjectAddr,itemIndex);

	if(itemRef == REF_NULL)
	{
		return ADDR_IS_NULL;
	}

	entryTableAddr = locateObjectInMemory(itemRef);

	return get_array_data(MEMtPN8(entryTableAddr));

}
#if RSP_EUICC
/*获取ProfileEntry内部元素MetaDataStore对象所在的地址*/
UWORD GetProfileMetaDataAddr(BYTE profileIndex)
{
	UWORD  xdata   ISDPObjectAddr,metaDataAddr;
	WORD   xdata   metaDataRef;

	ISDPObjectAddr = GetISDPObjectAddr(profileIndex);

	if(ISDPObjectAddr != ADDR_IS_NULL)
	{
		metaDataRef = PN_R_U16(get_object_field_ref(MEMtPF8(ISDPObjectAddr),MetaDataStore_INDEX));
		metaDataAddr = locateObjectInMemory(metaDataRef);
		return metaDataAddr;
	}

	return ADDR_IS_NULL;

}
#endif
UWORD GetISDPObjectAddr(BYTE profileIndex)
{
	UWORD    xdata     profileEntry,appTableAddr,theAppEntry;
	WORD     idata     index;

	if(ProfileIndexIsValid(profileIndex) == FALSE)
	{
		return ADDR_IS_NULL;
	}

	profileEntry = GetProfileEntryAddr(profileIndex);
	if(profileEntry == ADDR_IS_NULL)  // 复位之前执行了enable profile的操作，refresh模式是reset模式，但是没有执行reset之前又把该profile给删掉了，导致该profile的entry不存在，程序却按照存在的情况继续往下走，访问了非法地址i
	{
		return ADDR_IS_NULL;
	}
	appTableAddr = GetProfile_theAppTableValueAddr(profileEntry);

	for(index=0;index<APP_END;index++)
	{
		theAppEntry = locateObjectInMemory(PN_R_U16(U32_A_U16(appTableAddr, U16_M_U16(index, SIZE_REF))));
		if(PN_R_U8(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), bAppID_INDEX))) == ISDP_APPID)
		{
			theAppEntry = locateObjectInMemory(PN_R_U16(PFtMEM(get_object_field_ref(MEMtPF8(theAppEntry), theApplet_INDEX))));
			return theAppEntry;
		}
	}
	return ADDR_IS_NULL;

}

/*创建数组对象，其上下文与QwnerObjAddr一致
 *
 * 返回：数组地址 指向内容
 * */
UWORD MakeProfileArrayObject(WORD newContext,WORD Len,WORD* OutArrayRef,BYTE profileIndex)
{
	WORD  idata   backCc,arrayRef;
	UWORD xdata   addr;
	BYTE  idata   backProfileIndex;

	backProfileIndex = G_CurExecutingProfileIndex;
	G_CurExecutingProfileIndex = profileIndex;

	backCc = cc;
	cc = newContext;//get_object_owner(QwnerObjAddr);// 切换上下文

	arrayRef = _newarrayObject(AType_Byte, Len, BIT_NEED_REFRESH);

	cc = backCc;

//	if(arrayRef == MAP_TABLE_ILLEGAL_INDEX)
//	{
//		*OutArrayRef = REF_NULL;
//		G_CurExecutingProfileIndex = backProfileIndex;
//		return ADDR_IS_NULL;
//	}

	addr =  locateObjectInMemory(arrayRef);

	addr = get_array_data(MEMtPF8(addr));

	* OutArrayRef = arrayRef;
	G_CurExecutingProfileIndex = backProfileIndex;

	return addr;
}
/*ISDP/ISDR的包是预置包，在SE区和每个profile的公共区都有，所以不需要考虑当前的操作区域均可获取到
 * 获取ISDP的上下文，每个ISD-P的pkgcontextId 和appId都一样*/
WORD getISDPISDRContext(BYTE pkgId,BYTE appId)
{
	WORD idata contextId;

	contextId = getPkgContext(pkgId);

	return (contextId<<8)|appId;
}

BYTE GetCertType(BYTE profileIndex)
{
	UWORD xdata entryAddr;

	entryAddr = GetProfile_CertTypeAddr(GetISDPObjectAddr(profileIndex));

	return VMReadByte(entryAddr);
}

void SetCertType(BYTE profileIndex, BYTE certType)
{
	UWORD xdata entryAddr;

	entryAddr = GetProfile_CertTypeAddr(GetISDPObjectAddr(profileIndex));

	memWriteU8Atomic(MEMtPM8(entryAddr),certType);
}
#if M2M_EUICC
UWORD GetProfileConnectivityParaAddr(BYTE profileIndex)
{
	UWORD  xdata   ISDPObjectAddr,ConnectivityParaAddr;
	WORD   xdata   paraRef;

	ISDPObjectAddr = GetISDPObjectAddr(profileIndex);

	if(ISDPObjectAddr != ADDR_IS_NULL)
	{
		ConnectivityParaAddr = GetProfile_M2M_ConnectivityParaAddr(ISDPObjectAddr);
		paraRef = memReadU16(MEMtPM8(ConnectivityParaAddr));

		if(paraRef == REF_NULL)
		{
			return ADDR_IS_NULL;
		}
		ConnectivityParaAddr = get_array_data((PF8)locateObjectInMemory(paraRef));
		return ConnectivityParaAddr;
	}

	return ADDR_IS_NULL;
}
#endif
#endif
