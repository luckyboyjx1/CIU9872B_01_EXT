/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCProactiveHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-30

*Description :该文件主要用于实现基于TS102241 所描述的interface ProactiveHandler的功能 ，该接口主要包含处理ProactiveHandler数据的一些基本方法，
*
*History     :
	
author    time      version    description
ZZJ   2013/10/30   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "stkhead.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
#include    "ToolkitRFM.h"
//#include    "transaction.h"
//#include    "Interpreter.h"
#include   "ToolkitAPIPublic.h"
#include "Nativemethod_impl.h"

#if UICC__FUNCTION__UICCAPI
/**
 *@fucntion     : ToolKit_initMoreTime
 *
 *@description  : Builds a More Time Proactive command without sending the command.
 *
 *@see          : TS 102 241 :interface ProactiveHandler
 *
 *@param        : void 
 *
 *@author       : ZZJ
 *
 *@hisotry      : V1.0 :create by ZZJ@20131030
 */
void UICCToolKit_initMoreTime(void)
{    
	checkStkBusy(UICCAPI_Mode); 

	ClearHandlerCurrentTLV(Index_ProactiveHandler);

	InitProactiveCommand(CMD_MORETIME, 0x00, DEV_ME);
	
	ToolkitHandlerList[Index_ProactiveHandler].TLVListLength = ResData[2];
}
#endif
