/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   TimerModuleProcess.c
*
* Description: 该文件主要用于存放Timer相关的文件
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0:  该方案主要支持不同的业务场景 下向 COS申请timer的系统资源 ，目前已知的业务场景在本文件中已定义，后续有新的业务场景可以自定义或者新增
                      已知使用timer的业务场景: CATTP HTTP

				    要考虑标记在卡片复位时的清除。即6EA7文件中的标记清除
					v1.1: 从 epx2.1 主干代码移到 epx2.1-st-BLcos, 考虑到与主cos共用一套文件系统，并且timerId存储在文件中，所以此处可以直接使用epx2.1主cos的代码，不使用原单native平台的 TimerModuleProces.c
*
************************************************************************************************************/
#include "TimerModuleProces.h"
#include "fileAction.h"
#include "stkhead.h"
#include "gsmpublic.h"
#include "BIPModuleProcess.h"
#include "fileSystem.h"

#if JAVA_CARD_UICC
#include "ToolkitAPIPublic.h"
#include "stkalert.h"
#endif

#if GP__AMENDMENT__B__DNS_Resolution
#include "DNS.h"
#endif

#if UICC__FUNCTION__CATTP
#include "CatTp.h"
#endif
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#endif
#if M2M_EUICC
#include "uicc_M2M.h"
#endif
#if !BOOTLOADER_PRO
#include "SEEUICCInterface.h"		
#endif
#if TIMERMODULE
BYTE xdata G_EventTimerExpirationId;  //登记当前收到的timer expiration event的timer id
//BYTE xdata G_CurrentTimerId;          //当前的timerId

#if TIMERMODULE_TIMERID_IN_RAM
BYTE xdata G_UsedTimerId;             //当前http/cattp 已经使用的timer id bit0-bit7 分别表示timer1 - timer8  使用此变量 复位后直接清空 就不需要使用A501文件
#endif
/*
*@description:对timer进行初始化，在开机复位时调用，主要用于释放上一次断电前占用的timerId
              注意profile激活的时候，是不是要考虑将该文件中的所有timerId释放，而正常卡片复位，只需要释放非toollkit占用的timerId
*
*@function   :TimerModuleInit
*
*@see        :TS102223 Timer Management /event_time eixpration
*
*@param      :void
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create @20180503
*/
void TimerModuleInit(void)
{
#if TIMERMODULE_TIMERID_IN_FILE
	UWORD xdata fileAddr;
	BYTE xdata  timerbuf[0x10];    //文件大小的一倍
	
	//G_CurrentTimerId = FALSE;
	
	G_EventTimerExpirationId = FALSE;
	
	fileAddr = SelectPrivateTFFile(DF_Timer, EF_TimerId);
	
	if(fileAddr != FALSE)
	{
		ecpMem_Set(timerbuf,TimerType_NotUsed,sizeof(timerbuf));
		
		ReadTFBuf(fileAddr, 0x00, timerbuf, 0x08);   //读出文件内容
		
		if(ecpMem_Compare(timerbuf,&timerbuf[0x08],0x08) != EQUAL)  //不是TimerType_NotUsed的时候才初始化
		{
			SetTFConst(fileAddr, 0x00, TimerType_NotUsed, 0x08);
		}			
	}
#else
	G_EventTimerExpirationId = FALSE;

	G_UsedTimerId = FALSE;
#endif
}
/*
*@descritpion    :向系统申请当前可用的timerId,如果存在合适的，则系统会登记当前timerId已被占用 
*
*@function       :TimerModuleAllocTimerId
*
*@param          :BYTE timerType     
*@param           :BYTE isConfig: TRUE：申请timer同时更新它/FALSE: 只申请timer，不更新
*
*@return          :BYTE timerId
*
*@author         :xuyz
*
*@history        :v1.0
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/


BYTE TimerModuleAllocTimerId(void)
{

	UWORD xdata fileAddr;
	BYTE idata i,bitMask,timer[2];
#if TIMERMODULE_TIMERID_IN_FILE
	BYTE xdata tmp[8];
#endif
	BYTE idata appRecNum;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	u16    X_A  bFreeEntryIndex;
	u32 toolkitInfoObj;

	bitMask = 0;

	bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

    bFreeEntryIndex = (G_ToolkitAppIndex  == 0xFF)? bFreeEntryIndex : (G_ToolkitAppIndex + 1);//正在安装Toolkit应用时，要查找正在安装的应用参数  此时bFreeEntryIndex已和Toolkit应用的ToolkitInfo对应
    
	for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
	{
		toolkitInfoObj = GetToolkitInfoObj(i);

		if(toolkitInfoObj == ADDR_IS_NULL)
		{
			continue;
		}

		fileAddr = GetToolkitAppInfoArrayAddr(toolkitInfoObj);

		ecpMem_Copy(timer,MEMtPN8(fileAddr + MaxTimerCount_OFFSET),sizeof(timer));

		bitMask |= timer[1];
	}
#else
	bitMask = 0;  
	appRecNum = 0;

	fileAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(fileAddr != FALSE)
  	{
		appRecNum = GetLFRecNumLgth(fileAddr, FileNum);
  	}
	
	for(i=1; i<= appRecNum; i++)
	{
		ReadLFBufForJavaAccess(fileAddr, i , AppletStatus_OFFSET, timer,sizeof(timer));
		
		if(timer[0] == 0xFF)  //NULL 
		{		
			continue;
		}
		
		ReadLFBufForJavaAccess(fileAddr, i , MaxTimerCount_OFFSET, timer,sizeof(timer));

		bitMask |= timer[1];	//	标记被占用的timer的比特位 一个应用可以占用多个timer
	}

#endif

#if TIMERMODULE_TIMERID_IN_FILE
	fileAddr = SelectPrivateTFFile(DF_Timer, EF_TimerId);
	
	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	ReadTFBuf(fileAddr, 0x00, tmp, TimerMaxNum);
	
	for(i = 0; i< TimerMaxNum ; i++)
	{
		if((tmp[i] == TimerType_NotUsed)&&((bitMask & (1 << i))==0))
		{
			break;
		}
	}

	if(i == TimerMaxNum)
	{
		return FALSE;
	}

	tmp[i] = TimerType_Used;

	WriteTFBuf(fileAddr, 0x00, tmp, TimerMaxNum);
#else
	for(i = 0; i< TimerMaxNum ; i++)
	{
		if((ValueBit(G_UsedTimerId,i)==0x00)&&((bitMask & (1 << i))==0))
		{
			break;
		}
	}

	if(i == TimerMaxNum)
	{
		return FALSE;
	}

	SetBit(G_UsedTimerId,i);
#endif
	
	return (i+1);
}

/*
*@descritpion    :释放当前占用的TimerId
*
*@function       :TimerModuleReaseTimerId
*
*@param          :BYTE timerId        
*
*@return         :void
*
*@author         :xuyz
*
*@history        :v1.0
*@history        :V1.2 modify by xuxiaojun@20210412 调用该函数时，不仅需要清掉G_UsedTimerId中对应的bit位，还需要清掉* timerId
*/
BYTE TimerModuleReaseTimerId(BYTE* timerId)
{
#if TIMERMODULE_TIMERID_IN_FILE
	UWORD xdata fileAddr;
#endif
	
	if(TimerModuleTimerIdInValid(*timerId))
	{
		return FALSE;
	}

#if TIMERMODULE_TIMERID_IN_FILE
	fileAddr = SelectPrivateTFFile(DF_Timer, EF_TimerId);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	SetTFConst(fileAddr, *timerId -1, TimerType_NotUsed, 0x01);
#else
	ClearBit(G_UsedTimerId,*timerId-1);
#endif

	*timerId = FALSE;

	return TRUE;
}


/*
*@description :  拼装TimerManagement 主动式命令，支持starttimer deactivetimer  gettimervalue
*
*@see         :TS102223
*
*@function    :TimerManagementProactive
*
*@param       :BYTE commQualify
*@param       :BYTE timerTd
*@param       :BYTE * timerValue
*
*@return       :CMD_SUCCESS/OTHER RESULTS
*
*@author      :
*
*/
BYTE TimerManagementProactive(BYTE commQualify,BYTE timerTd, BYTE * timerValue)
{
	BYTE idata result;//remove warning

	InitProactiveCommand(CMD_TIMERMANAGE,commQualify,DEV_ME);
	
	RecData[0] = 0xA4;

	RecData[1] = 0x01;

	RecData[2] = timerTd;

	result = AddTLV(&ResData[1], &RecData[0]);

	if (result == FALSE)
	{
		return ExitStkPerform;
	}

	if (timerValue != NULL)
	{
		RecData[0] = 0xA5;

		RecData[1] = 0x00;

		AddValue(&RecData[1], &timerValue[0]);

		result = AddTLV(&ResData[1], &RecData[0]);

		if (result == FALSE)
		{
			return ExitStkPerform;
		}
	}
	
	result = SendProactiveCommand();

	//if (result == CMD_Success)
	//{
	//	pos = FindTag(&RecData[0],TLV_RESULT,P3);
		
	//	if (pos != FALSE)
	//	{
	//		result = RecData[pos+1];	
	//	}	
	//}
	return result;	
}

/*
*@description:  发送一个打开timer的主动式命令
*
*@see          :TS102223 terminalprofile的第8字节bit1 指示是否支持timerSTART
*
*@function    :TimerModuleStartTimer
*
*@param        :BYTE timerId
*@param        :BYTE * timerValue   LV结构
*
*@return       :CMD_Success/OtherResult
*
*@author       :
*
*@history      :
*/
BYTE TimerModuleStartTimer(BYTE timerId, BYTE * timerValue)
{
	return TimerManagementProactive(Start_Timer,timerId,timerValue);
}
/*
*@description:  发送一个deactivate timer的主动式命令
*
*@see          :TS102223 terminalprofile的第8字节bit1 指示是否支持timer deactivate
*
*@function    :TimerModuleDeactivateTimer
*
*@param        :BYTE timerId
*
*@return       :CMD_Success/OtherResult
*
*@author       :
*
*@history      :
*/
BYTE TimerModuleDeactivateTimer(BYTE timerId)
{
	return TimerManagementProactive(Close_Timer,timerId,NULL);
}

/*
*@DESCRIPTION  :处理timer Expiration事件，此处只登记当前有timer expiration事件发生，至于具体的业务处理，由业务层自行处理
*
*@see          :TS102223 SECTION 7.4
*
*@function      :TimerModuleProcessTimerExpiration
*
*@param         :void
*
*@return        :void
*
*@history       :modify by xuxiaojun@20220309   merged from EPX产业化\STM_ST33G1M2\Branch\RSP\code, 支持DNS功能
*
*@author        : create by xuyz
*/
BYTE  TimerModuleProcessTimerExpiration(void)
{
	BYTE idata pos;
	BYTE idata tag;
	
	pos = FindTag(&RecData[2], 0xA4, RecData[1]);
	
	if(pos == FALSE)
	{
		return FALSE;
	}

	if(TimerModuleTimerIdInValid(RecData[2+pos+1]))
	{
		return FALSE;
	}
	
	G_EventTimerExpirationId = RecData[2+pos+1];  //get timerId,电信的cattp业务要求在收到timer expiration事件后，不再发送moretime ，可以根据该全局变量判断当前是否收到了timerevent，
	
	tag = FALSE;

#if GP__AMENDMENT__B__FOR_MASK
	tag |= (G_EventTimerExpirationId == G_HTTPSStatus.HTTPTimerId);
#endif

	
#if UICC__FUNCTION__CATTP	
	tag |= (G_EventTimerExpirationId == G_CATTP_TimerIdentifier);
#endif

#if M2M_EUICC
	tag |= (G_EventTimerExpirationId == (G_NotificationTimerId&0x0F));
#endif
	
#if GP__AMENDMENT__B__DNS_Resolution
	tag |= (G_EventTimerExpirationId == G_RASIPConnectStatus.timerId);

	tag |= (G_EventTimerExpirationId == G_DNSIPConnectStatus.timerId);

	tag |= (G_EventTimerExpirationId == G_DNSByMEConnectStatus.timerId);
#endif

	if(tag)
	{
 		PendingMessageAlert(TimerModule_TimerExpiration_Alert, G_EventTimerExpirationId);
	}

	return tag;
}
#endif
