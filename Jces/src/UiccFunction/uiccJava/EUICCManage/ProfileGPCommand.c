/************************************************************************************************
Copyright (C), 1988-2003, Eastcompeace. Co., Ltd.

File name   : ProfileGPCommand.c

Author:     : xuyz

Version:    : V1.0

Description :  该文件主要处理PE-SecurityDomain  PE-Application等模块中与GP 相关的接口

************************************************************************************************/ 
#include "ProfileGPCommand.h"
#include "Interpreter.h"
#include "EUICCCustomer.h"
#include "Method.h"
#include "ASN1Operation.h"
#include "EUICCProfile.h"
#include "cardManager.h"
#include "SEEUICCInterface.h"
#include "T0ApduImpl.h"
#include "ToolkitRFM.h"
#include    "ToolkitAPIPublic.h"
#include "EUICCMain.h"
#include "packageloc.h"
#include "euiccConfig.h"
#include "GPAnnexbAdminSessionPara.h"
#include "VMAccessEeprom.h"
#include "Interface_LTEFromPlatform.h"
#include "platform_api.h"
#include "nativemethod_impl.h"
#include "rtrSpace.h"
#include "tranManager.h"	//remove warning,dengyiwen,2022/9/27

#if (EUICC_ONLY||SE_AND_EUICC)

profileGPState xdata G_profileGPState;

/*
*@function   :ContactlessProtocolParameterInit
*
*@description:profile下载过程中,初始化非接协议参数，包括Contactless_DefaultProtolParaA_Ref, Contactless_DefaultProtolParaB_Ref, Contactless_OpenParaProfile_Ref初始值来源于Card_Contactless_DefaultProtolParaA_Ref, Card_Contactless_DefaultProtolParaB_Ref, Card_Contactless_OpenParaProfile_Ref
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 :created by xuxiaojun @20190226
*            :V1.1 :modify by xuxiaojun@20190716 修改调用FullProtDataComputation函数时的传入参数的BUG
*/
void ContactlessProtocolParameterInit(void)
{
//	BYTE idata i, maxLen, transActioinFlag;
//	WORD idata tmpRef;
//	UWORD xdata cardAddr, paraAddr, cardProfileDataAddr, paraDataAddr;
//
//	cardAddr = ADDR_IS_NULL;
//
//	paraAddr = ADDR_IS_NULL;
//
//	maxLen = 0x00;
//
//	tmpRef = REF_NULL;
//
//
//	transActioinFlag = IF_BeginTransaction();		 //ligy 该函数开启的事务在注册时提交，若抛异常且事务是在本函数开启 则取消事务20141120
//
//	SEUICC_BackISDPAppletTable(IF_GetDownloadProfileIndex());
//
//	for(i = 0 ;i <= 1; i++) // 从Card_Contactless_DefaultProtolParaA_Ref中读取数据复制到Contactless_DefaultProtolParaA_Ref中
//	{
//		cardAddr = Card_Contactless_DefaultProtolParaA_Ref + i*sizeof(WORD);
//
//		cardProfileDataAddr = GetAddrByIndexAddr(cardAddr);
//
//		if(cardProfileDataAddr != ADDR_IS_NULL)
//		{
//			maxLen = VMReadByte(cardProfileDataAddr);
//
//			paraAddr = Contactless_DefaultProtolParaA_Ref + (PROFILE_MAX_NUMBER * i) * sizeof(WORD);  // 实际存储格式为Lmax LV
//
//			paraDataAddr = VMCOSHeapAlloc(maxLen + 1, &tmpRef, AppMemory_Mode);
//
//			if(paraDataAddr != ADDR_IS_NULL)
//			{
//				VMReadBuf(tempBuf, cardProfileDataAddr, maxLen + 1);
//
//				WriteEepromNonAtomic(paraDataAddr, tempBuf, maxLen + 1);
//
//				VMWriteBuf(paraAddr, (BYTE*)&tmpRef, 0x02);
//			}
//
//			//FullProtDataComputation(ILLEGAL_APPID, i);  //当前协议参数重算， 在这里会给PROFILE_Contactless_CurProtolParaA_Ref分配空间，并赋值
//		}
//	}
//
//	for(i = 0; i < CONTACTLESS_PROTOCOL_PROFILE_NUMBER; i ++) // 更新contactless profile
//	{
//		cardAddr = Card_Contactless_OpenParaProfile_Ref + i*sizeof(WORD);
//
//		cardProfileDataAddr = GetAddrByIndexAddr(cardAddr);
//
//		if(cardProfileDataAddr != ADDR_IS_NULL)
//		{
//			maxLen = VMReadByte(cardProfileDataAddr);
//
//			paraAddr = Contactless_OpenParaProfile_Ref + i*sizeof(WORD);
//
//			paraDataAddr = VMCOSHeapAlloc(maxLen + 1, &tmpRef, AppMemory_Mode);
//
//			if(paraDataAddr != ADDR_IS_NULL)
//			{
//				VMReadBuf(tempBuf, cardProfileDataAddr, maxLen + 1);
//
//				WriteEepromNonAtomic(paraDataAddr, tempBuf, maxLen + 1);
//
//				VMWriteBuf(paraAddr, (BYTE*)&tmpRef, 0x02);
//			}
//		}
//	}
//
//	IF_CommitTransaction(transActioinFlag);
//
//	SEUICC_ResumeISDPAppletTable();
}

/*
*@function   :PEGPGlobalVariableInit
*
*@description:profile下载过程中，与GP指令相关的全局变量的初始化
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 :created by xuxiaojun @20190226
*/
void PEGPGlobalVariableInit(void)
{
	ecpMem_Set((BYTE*)&G_profileGPState, 0x00, sizeof(G_profileGPState));
	
	G_profileGPState.peInstallAppId = ILLEGAL_APPID;
	
	G_profileGPState.peExtraditeSDID = ILLEGAL_APPID;
}
/*
*@function    :PEUpdateCATTPPersonalisation
*
*@description :更新8.6  Security domains  中的8.6.7  CAT_TP personalisation 字段到PROFILE_CATTPPara中
*
*@param       :WORD catTpMaxSduSize
*@param       :WORD catTpMaxPduSize
*
*@return       :void
*
*@history      :V1.0 create by xuyz@20192222
*              :V1.1 modify by xuxiaojun@20190624
*/
void PEUpdateCATTPPersonalisation(WORD catTpMaxSduSize, WORD catTpMaxPduSize)
{
	BYTE idata index;
	
	index = IF_GetDownloadProfileIndex();
	
	VMWriteShort(PROFILE_CATTPPara + index*sizeof(UWORD), catTpMaxSduSize);     // 更新 catTpMaxSduSize到对应profile的PROFILE_CATTPPara字段的前两个字节 

	VMWriteShort(PROFILE_CATTPPara + index*sizeof(UWORD) + 2, catTpMaxPduSize); // 更新 catTpMaxPduSize到对应profile的PROFILE_CATTPPara字段的后两个字节 
}
/*
*@function   :PEUpdateOpenPersoData
*
*@description:更新8.6  Security domains 中的8.6.6  OPEN personalization 参数到正在下载的profile的指定字段中
*
*@See        :<Profile interoperability technical specification_V2.2_FINAL.pdf> 8.6.6  OPEN personalization
*             <GPC_2.2_C_ContactlessServices_v1.1.pdf>  11.2.1   Contactless Protocol Parameters Structure
*
*@param      :BYTE *contactlessProtocolParameters :LV结构 ,值域分别是 'Initial Contactless Activation State'0x81, Contactless protocol Type State(0x89)  Protocol Data type A(0x86),  Protocol Data type B(0x87), Protocol Data type F (0x88), Continuous Processing(0x84)
*
*@return     :void
*
*@author     :xuyz
*
*@history    :Note ： Protocol Data Type A / Protocol Data Type  B 的处理要考虑
*            :modify by xuxiaojun@20190703 
*            :modify by xuxiaojun@20190711 根据上层调用调整函数参数和返回值，增加typeA, typeB默认协议的存储。
*/
//BYTE PEUpdateOpenPersoData(BYTE *contactlessProtocolParameters)
//{
//	BYTE idata subTag, lenthOfLen;
//	WORD idata i, defaultAddrRef;
//	BYTE idata repeatTag;
//	UWORD xdata paraAddr, paraTotalLen, subLen;
//
//	lenthOfLen = ASN1_GetBERTlvLen((UWORD)contactlessProtocolParameters, (UWORD*)&paraTotalLen);
//
//	i = lenthOfLen; // 偏移到第1个子TAG的起始
//
//	repeatTag = 0x00;
//
//	while(i<= paraTotalLen)
//	{
//		subTag = contactlessProtocolParameters[i];
//
//		lenthOfLen = ASN1_GetBERTlvLen((UWORD)&contactlessProtocolParameters[i+1], (UWORD*)&subLen);
//
//		switch(subTag)  // Only the following parameters are supported
//		{
//			case 0x81:  //Initial Contactless Activation State GP AC :8.3  Initial Contactless Activation State
//				if(subLen == 0x01)
//				{
//					if(contactlessProtocolParameters[i+2] > 0x01)  //Initial Contactless Activation State 的有效值只能为00 或 01
//					{
//						return FALSE;
//					}
//					else
//					{
//						VMSetBuf((BYTE *)(PROFILE_Contactless_IntialCLActivate + IF_GetDownloadProfileIndex()), contactlessProtocolParameters[i+2], 1);
//					}
//				}
//				else
//				{
//					return FALSE;
//				}
//
//				break;
//
//			case 0x84:   //Continuous Processing, <GPC_2.2_C_ContactlessServices_v1.1.pdf>中 Table  6- 1:  OPEN –  Value Part of “Continuous Processing” TLV
//				if(subLen == 0x02)
//				{
//					VMWriteBuf(PROFILE_Contactless_ContinuousProcessing + IF_GetDownloadProfileIndex()*2, &contactlessProtocolParameters[i+2], 2);
//				}
//				else
//				{
//					return FALSE;
//				}
//
//				break;
//
//			case 0x86:   //Protocol Data Type A (Card Emulation Mode)
//			{
//				if(ValueBit(repeatTag, 0x01))
//				{
//					return FALSE;
//				}
//
//				paraAddr = PROFILE_Contactless_DefaultProtolParaA_Ref + IF_GetDownloadProfileIndex()*2;
//
//				VMReadBuf((BYTE *)&defaultAddrRef, paraAddr, sizeof(WORD) );
//
//				//ProcessComponentTLV(&defaultAddrRef, NULL);
//
//				VMWriteBuf(paraAddr,(BYTE *)&defaultAddrRef ,sizeof(WORD));
//
//				SetBit(repeatTag, 0x01);
//
//				break;
//			}
//
//			case 0x87:   //Protocol Data Type B
//			{
//				if(ValueBit(repeatTag, 0x02))
//				{
//					return FALSE;
//				}
//
//				paraAddr = PROFILE_Contactless_DefaultProtolParaB_Ref + IF_GetDownloadProfileIndex()*2;
//
//				VMReadBuf((BYTE *)&defaultAddrRef, paraAddr, sizeof(WORD) );
//
//				//ProcessComponentTLV(&defaultAddrRef, NULL);
//
//				VMWriteBuf(paraAddr,(BYTE *)&defaultAddrRef ,sizeof(WORD));
//
//				SetBit(repeatTag, 0x02);
//
//				break;
//			}
//
//			case 0x88:   //Protocol Data Type F, 卡片暂时不支持协议F
//				break;
//
//			case 0x89:   //该字段是GP AC 2.3 新增字段 8.5  Contactless Protocol Type State ，该字段暂时未处理
//				break;
//
//			default: //白盒测试添加
//				return FALSE;
//
//		}
//
//		i += (lenthOfLen + subLen+1);//指向下一个tag
//
//	}
//
//	return TRUE;
//}
	
/*
*@function   :  PEExcuteGPCommand
*
*@description:  在处理与profile相关的GP 命令入口前作的一些准备工作 ,命令体数据已拼装在RecData中，命令头已拼装进 CLA INS P1 P2 P3中
*
*@param      : void
*
*@return     :WORD sw
*
*@author      :xuyz
*
*@history    : V1.0 :create @20190128
*/
WORD PEExcuteGPCommand(BYTE excuteSdId)
{
	WORD idata sw;
	BYTE idata backSdId;
	UWORD xdata mainMethodAddr;
	
	//ecpMem_Copy(G_ApduBuffer,G_APDU,5);
	backSdId = g_bSDId[selected_channel] ;

	g_bSDId[selected_channel]  = excuteSdId;

	G_GPErrorCode = 0x9000;  
	
	bRamBuffer[PROFILE_APDU_MODE] = TRUE;
	
	mainMethodAddr = U32_A_U16(getPackageComponentLocation(EUICC_PkgId, COMPONENT_METHOD), INSTALLISDP_METHOD_OFFSET);

	callmethod(MEMtPF8(mainMethodAddr), EUICC_PkgId, JCRE_CONTEXT);  //进入 java层进行GP 命令的处理

	run();

	popFrame(); //恢复当前上下文

	bRamBuffer[PROFILE_APDU_MODE] = FALSE;

	if((TransactionGetDepth() == TRUE) )
	{
		abortTransaction();
	}
	
	g_bSDId[selected_channel] = backSdId;

	sw = G_GPErrorCode;

	G_GPErrorCode = 0x9000;

	return sw;
	
	
}

WORD CreateProfileEntry(void)
{
	WORD idata sw;
	BYTE idata backProfilelIndex;

	backProfilelIndex = G_CurExecutingProfileIndex;//创建对象时根据这个变量标识对象属于哪个profile
	G_CurExecutingProfileIndex = bRamBuffer[LOADING_PROFILE_INDEX];//此时 被创建的profile空间还不存在 不能调用ExecuEnvironmentInit

	Euicc_ConfigDataReset(0);
	RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)), tranMananger_getProfileRtp(G_CurExecutingProfileIndex), false);

	GPSubCallFunction(CreateProfileAppTable_METHOD_OFFSET, EUICC_PkgId,JCRE_CONTEXT);

	if(G_GPErrorCode != 0x9000)//这里不是9000  不写PROFILE_VALIDTAG_CFG，同时将G_RSPSessionContext.downloadProfileIndex = ILLEGAL_PROFILEINDEX; 避免调到删profile
	{
		if(INVALID_ALLOC_RAM_ADDRESS != RamManager_refHasBeenAllocated(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID))))//清RTR SPACE
		{
			RamManager_free(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)));
		}
#if RSP_EUICC
		G_RSPSessionContext.downloadProfileIndex = ILLEGAL_PROFILEINDEX;
#endif
	}
	else
	{
		VMSetBuf(PROFILE_VALIDTAG_CFG + bRamBuffer[LOADING_PROFILE_INDEX], PROFILE_LOADINGFLAG, 1);
	}

	sw = G_GPErrorCode;

	G_GPErrorCode = 0x9000;

	G_CurExecutingProfileIndex = backProfilelIndex;

	return sw;
}

#endif
