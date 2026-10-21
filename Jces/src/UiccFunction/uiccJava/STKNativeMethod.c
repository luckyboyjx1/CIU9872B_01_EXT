/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCToolkitPublic.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-23

*Description :该文件主要用于存放实现SIM API/UICC API过程中的一些本地方法
*
*History     :
	
author    time      version    description
xuyz   2015/10/30   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include "STKNativeMethod.h"
#include "Interpreter.h"
//#include "JCREMain.h"
//#include  "Exception.h"
#include  "fileAction.h"
//#include "GPApplet.h"
//#include "GPCommand.h"
#include  "ToolkitAPIPublic.h"
//#include "main.h"
//#include "ChannelManagement.h"
#include "ToolkitRFM.h"
#include  "anomaly.h"
#include    "filesystem.h"
#include "T0ApduImpl.h"
#include "cardManager.h"
/*
*@Function   :NM_GetCurrentToolkitRef 
*
*@See        :
*
*@description:获取当前toolkit应用的ref
*
*@Throw      :
*
*
*@Return:  	   Applet Ref
*
*@Author:      lgy @20150709
*
*@History:     V1.0-- 创建该函数
*/
void NM_GetCurrentToolkitRef(void)
{
#if UICC__FUNCTION__STKAPI
	PushOperandStackShort(G_TrigerToolkitRef);
    #endif
}
/*
*@Function   :NM_GetCurrentToolkitAID
*
*@See        :
*
*@description:获取当前toolkit应用的AID
*
*@Throw      :
*
*
*@Return     :Applet AID
*
*@History    :V1.0--created by xuxiaojun@20200628
*/
void NM_GetCurrentToolkitAPPID(void)
{
#if UICC__FUNCTION__STKAPI
	u8 idata appID;

	appID = GetAppIdByRef(G_TrigerToolkitRef);

	PushOperandStackShort(appID);
#endif
}
/*
*@Function   :NM_GetCurrentToolkitEvent 
*
*@See        :
*
*@description:
*
*@Throw      :
*
*
*@Return:  	   G_ToolkitContext.CurrentEvent
*
*@Author:      lgy @20150709
*
*@History:     V1.0-- 创建该函数
*/
void NM_GetCurrentToolkitEvent(void)
{
#if UICC__FUNCTION__STKAPI
	PushOperandStackShort(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel]);
    #endif
}
/*
*@function     :setISOBusyExceptionFlag
*
*@description  :
*
*@return       :void
*
*@author       :xuyz
*
*@history	   :V1.0 ：create @20150206
*
*@others       :
*/
void setISOBusyExceptionFlag(void)
{
#if UICC__FUNCTION__STKAPI
    if(G_TriggerAppNumber == 0x01)
	//if(G_TriggerAppNumber != FALSE)
    //根据UICC API测试规范102268，5.5.5.1.1.1，CRRN2: If only one Applet shall be triggered by the currently processed event and an ISOException with the
    //reason code REPLY_BUSY is thrown, it shall be sent to the terminal using the Status Word 0x9300. , by xwh 20200721
	{
		G_ISOBusyExceptionFlag = TRUE;
	}
    #endif
}

void NM_GetJCREMainProcessMode(void)
{
	PushOperandStackShort(G_JCREMainProcessMode);
}

/*
*@function     :STKCheckContextForVolatileByteArray
*
*@description  :java.lang.SecurityException if the method is invoked from a context which is not the currently selected applet or the currently triggered applet i.e. the context of the applet that treats the current APDU or the context of the applet that has been triggered by the current APDU.
*               该本地方法供UICCPlatfrom 中的API函数getTheVolatileByteArray使用。供CMCCPlatform中的API函数getTheVolatileByteArray使用。
*
*@return       :void
*
*@author       :xuyz
*
*@history	   :V1.0 ：create @20150206
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others       :
*/
#if UICC__FUNCTION__STKAPI
void STKCheckContextForVolatileByteArray(void)
{
	BYTE idata appId;
//	WORD idata  trigerContext;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	UWORD xdata toolkitAppAddr;

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return;
	}
#endif
	if(G_TriggerAppNumber != FALSE)	  //如果当前是触发应用，则判断当前上下文不等于选择的上下文且不等于被触发的应用的上下文
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		appId = getAppIDByIndex(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]);
		//toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]));
#else
		ReadLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],AppId_OFFSET,&appId,sizeof(BYTE));  //读取toolkit的appId
#endif
		//		trigerContext = GetAppContextByAppId(appId);

		if((G_CurContext != getChannelContext(G_CurSelectedChannel))&&(appId != (G_CurContext&0xFF)))
		{
			ThrowError(SECURITY_EXCEPTION);
		}
	}
	else		                    //如果是从process中调用的，则判断调用该方法的上下文不是当前选择的上下文
	{
		if(G_CurContext != getChannelContext(G_CurSelectedChannel))
		{
			ThrowError(SECURITY_EXCEPTION);
		}	
	}
	
	//G_VolatileByteArrayIsCalled = TRUE;
}
#endif

