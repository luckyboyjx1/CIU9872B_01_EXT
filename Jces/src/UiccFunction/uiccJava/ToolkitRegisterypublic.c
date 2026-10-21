#include "typedef.h"
#include "fileAction.h"
#include "filesystem.h"
//#include "Interpreter.h"
//#include "transaction.h"
#include "uicc_interface.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
//#include "ChannelManagement.h"
#include "stkhead.h"
#include "mask.h"
#include "platform_api.h"
#if UICC__FUNCTION__SIMAPI
#include "SIMToolkitConstant.h"
#endif
#if UICC__FUNCTION__UICCAPI
#include "UICCToolkitConstants.h"
#endif
#include "ToolkitRFM.h"
#include "TimerModuleProces.h"
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
#include "SEEUICCInterface.h"
#endif


/**
*@function    : FindMinDuration
*
*@description :查找除了当前记录以外的其他应用申请的最小duration
*
*@param       :curretNum    当前应用在EF_ToolkitApp文件中的记录号
*
*Return       :minDuration	当前记录以外的其他应用申请的最小duration
*
**@author    :yjr
*
*@history    :V1.0 :create by yjr@20210326
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others     :
*/
WORD FindMinDuration(BYTE curretNum)
{
	BYTE  idata recNum,i;
	BYTE xdata tmp[EventList_Length];
	WORD xdata minDuration,tempDuration;
	UWORD xdata toolkitAppAddr;

	minDuration = MAXVALUE_POLL_DURATION + 1;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	recNum  = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for(i=0; i< recNum; i++)
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	recNum = GetFileHeadByte(toolkitAppAddr, F_RecNum_Offset);

	for(i=1; i<= recNum; i++)
#endif
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));
		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}
#endif
		ReadLFBufForJavaAccess(toolkitAppAddr,i,AppletStatus_OFFSET,&tmp[0],0x01);

		if(tmp[0] == APPLET_UNLOCKED_SELECTABLE)
		{
			if(i == curretNum)
			{
				continue;
			}

			ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,tmp,EventList_Length);

			if(ToolkitGetEventBit(tmp,EVENT_STATUS_COMMAND_bit))
			{
				ReadLFBufForJavaAccess(toolkitAppAddr,i,PollDuration_OFFSET,(BYTE *)&tempDuration, sizeof(WORD));

				if(tempDuration < minDuration)
				{
					minDuration = tempDuration;
				}
			}

		}
	}

	return minDuration;

}


/*
*@function   :ToolkitEventRegistered
*
*@description:判断当前是否已注册某事件 :
*             事件EVENT_CALL_CONTROL_BY_SIM，EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM:43.019/131130有定义该事件最多被一个applet注册
*
*@paran      :localEvent	 :
*             EVENT_CALL_CONTROL_BY_SIM_bit/    
*             EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit 43.019/131130有定义该事件
*
*@return     :TRUE/FALSE
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE ToolkitEventRegistered(BYTE localEvent)
{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	BYTE idata i;
	BYTE xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum;

	toolkitAppRecNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	toolkitAppRecNum = (G_ToolkitAppIndex  == 0xFF)? toolkitAppRecNum : (G_ToolkitAppIndex + 1);//正在安装Toolkit应用时，要查找正在安装的应用参数  此时bFreeEntryIndex已和Toolkit应用的ToolkitInfo对应

	//The (U)SAT Framework shall not allow more than one Toolkit Applet to be registered to this event at a time,see 131130 6.2,102241 6.2
	if((localEvent == EVENT_CALL_CONTROL_BY_SIM_bit)||(localEvent == EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit))
	{
		for(i =0 ; i< toolkitAppRecNum; i++ )//
		{
			toolkitAppAddr  = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

			if(toolkitAppAddr == ADDR_IS_NULL)
			{
				continue;
			}

			ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,&tmp[0], EventList_Length);//i无效

			if(ToolkitGetEventBit(&tmp[0], localEvent) == TRUE)	//当前查找的应用注册了事件EVENT_CALL_CONTROL_BY_SIM_bit 及EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit
			{
				return TRUE;
			}
		}

		return FALSE;
	}
#else
	BYTE idata i; 
	BYTE xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum; 
		
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{	
		return FALSE;	
	}
	
	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	//The (U)SAT Framework shall not allow more than one Toolkit Applet to be registered to this event at a time,see 131130 6.2,102241 6.2
	if((localEvent == EVENT_CALL_CONTROL_BY_SIM_bit)||(localEvent == EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit))	
	{
		for(i =1 ; i<= toolkitAppRecNum; i++ )
		{
			ReadLFBufForJavaAccess(toolkitAppAddr ,i, 0x00,&tmp[0],0x01);

			if(tmp[0] == 0xFF)  //记录为空
			{
				continue;
			}

			ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,&tmp[0], EventList_Length);

			if(ToolkitGetEventBit(&tmp[0], localEvent) == TRUE)	//当前查找的应用注册了事件EVENT_CALL_CONTROL_BY_SIM_bit 及EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit
			{
				return TRUE;	
			}
		}

		return FALSE;
	}
#endif
	return FALSE;
}

/*
*@function    :releaseTimerAndService
*
*@description ;释放当前应用所拥有的timer or service ID
*
*@param      :serviceTimeId:TimerId-1 /serviceId
*@param      :offset: MaxChannelCount_OFFSET/MaxServiceCount_OFFSET
*@param      :eventBit: EVENT_STATUS_COMMAND_bit/EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit
*@param      :reason : INVALID_TIMER_ID/INVALID_SERVICE_ID
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131101
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others     :
*/
void releaseTimerAndService(BYTE serviceTimeId,WORD offset,BYTE eventBit,BYTE reason, BYTE apidMode)
{
	BYTE idata service[0x02];
	BYTE xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#else
	recNum = GetCurrentToolkitEntryAppNum();

	if(recNum == FALSE)
	{
		return;
	}

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{	
		return;	
	}

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum , EventList_OFFSET, tmp,sizeof(tmp));

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum , offset, service,sizeof(service));

	if(ValueBit(service[1], serviceTimeId))
	{		
		ClearBit(service[1], serviceTimeId);		//释放Timer or service
	
		if(service[1] == 0x00)	   	            //释放事件
		{
			ToolkitClearEventBit(tmp,eventBit);//timer释放完时，清除事件
		}

		WriteLFBufForJavaAccess(toolkitAppAddr,recNum, offset,service,sizeof(service));

        WriteLFBufForJavaAccess(toolkitAppAddr,recNum, EventList_OFFSET,tmp,sizeof(tmp));
		
		return;	
	}
	else
	{
		UiccOrSimThrowToolkitException(reason, apidMode);
	}
}
/*
*@function   : Toolkit_findUnallocatedTimerId
*
*@description:查找当前是否还存在未分配过的timerId/serviceId
*
*@param      : offset:MaxTimerCount_OFFSET/MaxServiceCount_OFFSET
*
*@return     :timerId/serviceId+1/FALSE
*
*@author      :
*
*@history     :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                           用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能  
*/
BYTE findUnallocatedTimerIdAndServiceId(WORD offset)
{
	BYTE idata i, j;
	BYTE idata tmp[2],bitMask;
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum;

#if TIMERMODULE_TIMERID_IN_FILE
	BYTE xdata timer[8];
#endif

	bitMask = 0;

#if TIMERMODULE_TIMERID_IN_FILE
	ecpMem_Set(timer,TimerType_NotUsed,8);

	if(offset == MaxTimerCount_OFFSET)
	{
		toolkitAppAddr = SelectPrivateTFFile(DF_Timer, EF_TimerId);
		
		if(toolkitAppAddr != FALSE)
		{
			ReadTFBuf(toolkitAppAddr, 0x00, timer, 0x08);
		}
	}
#else
	if(offset == MaxTimerCount_OFFSET)
	{
		bitMask |= G_UsedTimerId;  //G_UsedTimerId  cattp/http等使用的timer
	}
#endif

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppRecNum  = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	toolkitAppRecNum = (G_ToolkitAppIndex  == 0xFF)? toolkitAppRecNum : (G_ToolkitAppIndex + 1);//正在安装Toolkit应用时，要查找正在安装的应用参数  此时bFreeEntryIndex已和Toolkit应用的ToolkitInfo对应

	for(j=1; j< toolkitAppRecNum; j++)//从1开始不会有问题，第0个是ISD，肯定不是toolkit应用
	{
		toolkitAppAddr  =  GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(j));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}
		ReadLFBufForJavaAccess(toolkitAppAddr, j , offset, tmp,sizeof(tmp));

		bitMask |= tmp[1];	//	标记被占用的timer的比特位 一个应用可以占用多个timer
	}
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
	
	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum); 
	
	for(j=1; j<= toolkitAppRecNum; j++)
	{
		ReadLFBufForJavaAccess(toolkitAppAddr, j , AppletStatus_OFFSET, tmp,sizeof(tmp));

		if(tmp[0] == 0xFF)  //NULL 
		{		
			continue;
			
		}

		ReadLFBufForJavaAccess(toolkitAppAddr, j , offset, tmp,sizeof(tmp));

		bitMask |= tmp[1];	//	标记被占用的timer的比特位 一个应用可以占用多个timer
	}
#endif
	for(i=1; i<= 8; i++)
	{
#if TIMERMODULE_TIMERID_IN_FILE
		if(((bitMask & (1 << (i-1))) == 0)&&(timer[i-1]==TimerType_NotUsed))	// 找到未被占用的timer 返回timer id    1到8
#else
		if((bitMask & (1 << (i-1))) == 0)	// 找到未被占用的timer 返回timer id    1到8
#endif
		{
			return i;
		}
	}
	
	return FALSE;
}
/*
*@function   :allocateTimerAndService
*
*@descritpion:给当前应用分配相应的timer或serveice，并注册相关事件
*
*@param      : serviceTimeId:TimerId-1 /serviceId
*@param      :offset: MaxChannelCount_OFFSET/MaxServiceCount_OFFSET
*@param      :eventBit: EVENT_STATUS_COMMAND_bit/EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit
*@param      :reason : NO_TIMER_AVAILABLE/NO_SERVICE_ID_AVAILABLE 
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others     :
*/
void allocateTimerAndService(BYTE serviceTimeId,WORD offset,BYTE eventBit,BYTE reason, BYTE apiMode)
{
	BYTE xdata tmp[EventList_Length];
	BYTE idata service[0x02],sum,i;
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#else
	recNum = GetCurrentToolkitEntryAppNum();

	if(recNum == FALSE)
	{
		return;
	}

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{	
		return;	
	}

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum, EventList_OFFSET, tmp,sizeof(tmp));

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum , offset, service,sizeof(service));

	sum = 0x00;

	for(i=1; i<= 8; i++)
	{
		if((service[1] & (1 << (i-1)))!=0)
		{
			sum++;
		}
	}	
	if(service[0] <= sum)
	{
		if(MaxServiceCount_OFFSET == offset)
		{
#if UICC__FUNCTION__UICCAPI
				UICCThrowToolkitException(NO_SERVICE_ID_AVAILABLE);//NO_SERVICE_ID_AVAILABLE
			#endif
		}
		else
		{
			UiccOrSimThrowToolkitException(NO_TIMER_AVAILABLE, apiMode);//NO_TIMER_AVAILABLE
		}
	}
		
	SetBit(service[1], serviceTimeId);   	//设置Timer/service
	
	ToolkitSetEventBit(tmp,eventBit);		//设置事件

	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,offset,service,sizeof(service));
	
	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,sizeof(tmp));

}
/*
*@function    :Toolkit_updateMenuInfor
*
*@description :将菜单更新更新到6F81文件特定的记录中
*
*@param       :UWORD menuString
*@param       :byte length
*@param       :nextAction
*@param       :helpSupported
*@param       :iconQualifier
*@param       :iconIdentifier
*@param       :menuId
*@param       :menuRec  :待更新的记录号
*@param       :mode :00,text attribute 更新为默认值 00 00 03 90/else:保持原text attribute值不变
*
*@return      :void
*
*@exception   ::TransactionException  if the operation would cause the commit capacity to be exceeded
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131031
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others      :将menu数据存放于tempBuf中
*/
void updateMenuInfor(UWORD menuString,BYTE length,jbyte nextAction,jboolean helpSupported,jbyte iconQualifier,jshort iconIdentifier,BYTE menuId,BYTE menuRec,BYTE mode)
{
	BYTE idata appstatus,recNum;	
	UWORD xdata tookitMenuEntryAddr;
	UWORD xdata toolkitAppAddr;
		
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr =GetCurrentToolkitEntryArr();
#else
	recNum = GetCurrentToolkitEntryAppNum();

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{	
		return;	
	}

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,AppletStatus_OFFSET,&appstatus,1);

	ecpMem_Set(tempBuf,0x00,sizeof(tempBuf));

	tempBuf[MenuEntryStatus_OFFSET] = Menu_UNLOCKED|Menu_ENABLE;

	if(appstatus == APPLET_UNLOCKED_SELECTABLE)
	{
		tempBuf[MenuEntryStatus_OFFSET] = tempBuf[MenuEntryStatus_OFFSET] |Menu_SELECTABLE;//只有install make selectable后 entyr才显示20140317
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	tempBuf[AppRecNum_OFFSET] = getPreviousStackContext()&0xFF;//当前APPID
#else
	tempBuf[AppRecNum_OFFSET] = recNum; 
#endif
	tempBuf[ItemIdentifier_OFFSET] = menuId;
	
	tempBuf[NextAction_OFFSET]  = nextAction;

	if(helpSupported)
	{
		tempBuf[HelpSupported_OFFSET]   = TRUE;
	}
	else
	{
		tempBuf[HelpSupported_OFFSET]   = FALSE;
	}

	tempBuf[IconIdentifier_OFFSET] = (jbyte)(iconIdentifier >> 8);

	tempBuf[IconIdentifier_OFFSET+1] = (jbyte)iconIdentifier;	 // (0 means no icon)

	tempBuf[IconQulifier_OFFSET] = iconQualifier;

	tempBuf[MenuEntryTextLen_OFFSET] = length;

	VMReadBuf(&tempBuf[MenuEntryText_OFFSET],menuString,length);	//根据方案，length长度是不允许超过40字节的，如果超过的话，会有问题  
	
	if(mode == 0x00)
	{
		 tempBuf[TextAttribute_OFFSET] =  0x00;
		 tempBuf[TextAttribute_OFFSET +1] = 0x00;
		 tempBuf[TextAttribute_OFFSET +2] = 0x03;
		 tempBuf[TextAttribute_OFFSET +3] = 0x90;  //TS102241 If a text attribute different from the default format is provided for at least one Menu Entry, the SET UP MENU system proactive command shall contain the item text attribute list Comprehension TLV. The default format as defined in TS 123 040 [12] is "00 00 03 90".
	} 
	else
	{
		ReadLFBuf(tookitMenuEntryAddr,menuRec,TextAttribute_OFFSET,&tempBuf[TextAttribute_OFFSET], 0x04);
	}

	WriteLFBuf(tookitMenuEntryAddr,menuRec,0,tempBuf,GetLFRecNumLgth(tookitMenuEntryAddr, FileLgth));
}

/*
*@function   :ToolkitOtherMenuIDSupportHelp
*
*@description:查找当前应用是否还有其他menuID支持help,
*
*@param      :BYTE menuID
*@param      :BYTE appIdNum， 6F80记录号 ， HEAP方案传的是APPID
*
*@return     :TRUE:当前应用有其他menuID支持help  /false：当前应用没有其他MENUID 支持help
*
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*@hisoty     modify by lgy@20220719   HEAP方案appIdNum传的是APPID
*/
BYTE ToolkitOtherMenuIDSupportHelp(BYTE menuId,BYTE appIdNum)
{
	BYTE idata i,result;
	BYTE xdata tmp[5];
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum;

	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);

	for(i=1; i<= tookitMenuEntryRecNum; i++)
	{
		result = ReadLFBuf(tookitMenuEntryAddr, i , 0x00, tmp,sizeof(tmp));

		if((result != FALSE)&&(tmp[MenuEntryStatus_OFFSET] != 0xFF))	 //非空记录
		{
			if((appIdNum == tmp[AppRecNum_OFFSET])&&(tmp[ItemIdentifier_OFFSET] != menuId)&&(tmp[HelpSupported_OFFSET] == TRUE))
			{
				return TRUE;
			}
		}		
	}

	return FALSE;
}

/*
*@function   :ToolkitCountMenuIdNumber
*
*@description:统计当前某个应用已初始化菜单的个数或返回一条空记录
*
*@param:	：mode： FINDNULL_MODE  COUNT_MODE
*
*@return    :number/FALSE
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*            :V1.1: modify by xuxiaojun@20170714   该函数属于Toolkit_initMenuEntry和UICCToolkit_initMenuEntry的子函数，API调用，所以修改成在CACHE的读
*/
BYTE ToolkitCountMenuIdNumber(BYTE mode)
{
	BYTE idata count,i,toolkitNum,flag;
	BYTE idata tmp[2];
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitNum = getPreviousStackContext()&0xFF;
#else
	toolkitNum = GetCurrentToolkitEntryAppNum();
#endif
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);

	count = 0x00;

	for(i=1; i<= tookitMenuEntryRecNum; i++)
	{
		flag =  ReadLFBuf(tookitMenuEntryAddr, i , 0x00, tmp,sizeof(tmp));

		if(flag == FALSE)
		{
			continue;
		}

		if(tmp[MenuEntryStatus_OFFSET] == 0xFF)  // 空纪录
		{
			if(mode == FINDNULL_MODE)
			{
				return i;
			}
			else
			{
				continue;
			}
		}
		if(mode == FINDNULL_MODE)
		{
			continue;
		}

		if(tmp[AppRecNum_OFFSET]== toolkitNum)
		{
			count++;
		}		
	}

	return count;
}
/*
*@description:在6F81文件里查找特定menuId
*
*@param     :BYTE menuId
*
*@return    :recNum
*
*@history	:create by xuyz
*           :modify by xuxiaojun@20170714   该函数属于Toolkit_initMenuEntry和UICCToolkit_initMenuEntry的子函数，API调用，所以修改成在CACHE的读。另外会在NativeImpl_InstallForInstall中用
*/
BYTE ToolkitFindSpecifiedMenuId(BYTE menuId)
{
	BYTE idata i,flag;
	BYTE idata tmp[3];
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum;

	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);
	
	for(i=1 ;i <= tookitMenuEntryRecNum; i ++)
	{
		flag = ReadLFBuf(tookitMenuEntryAddr,i,MenuEntryStatus_OFFSET,&tmp[0],sizeof(BYTE));

		if((flag == FALSE)||(tmp[0] == 0xFF))
		{
			continue;
		}

		ReadLFBuf(tookitMenuEntryAddr,i,ItemIdentifier_OFFSET,&tmp[1],sizeof(BYTE));

		if(tmp[1] == menuId)
		{
			return i;
		}
	}	

	return FALSE;
}

/*
*@description:在6F80文件里查找特定menuId,安装时候判断ID是否被占用
*电信测试脚本RAM_Install_Paramter_Test_1221里面步骤80里面安装应用带EA参数菜单ID为0102，但是并没有调用InitMenuEntryPublic;步骤83时候安装应用带CA参数菜单ID为0102030405（会调用InitMenuEntryPublic），期望报6A80
*在安装的时候要去6F80检查菜单ID是否被占用，而不是去6F81去检查Init后的菜单ID
*@param     :BYTE menuId
*
*@return    :recNum
*
*@history	:
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE ToolkitFindSpecifiedMenuIdForInstall(BYTE menuId)
{
	BYTE idata i,j,flag;
	BYTE idata tmp[3];       //目前6F80存的ID个数最大为10，加Len长度总21
	UWORD xdata toolkitAppAddr;
	BYTE idata  tookitMenuEntryRecNum;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{	
		return FALSE;	
	}
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);
	
	for(i=1 ;i <= tookitMenuEntryRecNum; i ++)
	{
		flag = ReadLFBufForJavaAccess(toolkitAppAddr,i,0x00,&tmp[0],sizeof(BYTE));//读取AppletStatus_OFFSET的值

		if((flag == FALSE)||(tmp[0] == 0xFF))
		{
			continue;
		}
#else
	tookitMenuEntryRecNum  = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for(i=0 ;i < tookitMenuEntryRecNum; i++)
	{
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}
#endif
		ReadLFBufForJavaAccess(toolkitAppAddr,i,MaxEntryCount_OFFSET,&tmp[0],1);//读取最大的菜单个数
		
		for(j = 0;j < tmp[0];j++)
		{
			ReadLFBufForJavaAccess(toolkitAppAddr,i,MenuEntryPosition_OFFSET+2*j,&tmp[1],2);//读取positionId和menuId
			
			if(tmp[2] == menuId)
			{
				return i;
			}		
		
		}

	}	

	return FALSE;
}
/**
*@function   :AllocateTimerPublic
*
*@description: Asks the CAT Runtime Environment to allocate a Timer that the applet can manage.By calling this method the applet is registered to the EVENT_TIMER_EXPIRATION of the allocated timer.
*              The timer is allocated by the applet until it explicitly releases it.So it can then issue the Timer Management proactive command to start,stop or get the value of its allocated timer.
*
*@return      :the identifier of the Timer allocated to the applet
*
*@exception   :ToolkitException      with the following reason codes: 
*              <li>NO_TIMER_AVAILABLE if all the timers are allocated or the maximum number	of timers have been allocated to this applet
*@exception   :TransactionException  if the operation would cause the commit capacity to be exceeded
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131031
*
*@others      :
*/
void AllocateTimerPublic(void)
{
	BYTE idata unallocatedTimerId;
	jshort idata apiMode;

	apiMode = PopOperandStackShort();
	
	unallocatedTimerId = findUnallocatedTimerIdAndServiceId(MaxTimerCount_OFFSET);

	if(unallocatedTimerId == FALSE)
	{
		UiccOrSimThrowToolkitException(NO_TIMER_AVAILABLE, apiMode);
	}

    allocateTimerAndService((unallocatedTimerId-1),MaxTimerCount_OFFSET,EVENT_TIMER_EXPIRATION_bit,NO_TIMER_AVAILABLE, apiMode);

	PushOperandStackShort(unallocatedTimerId);
}
/*
@description:Changes the value of a menu entry. The default state of the changed menu entry is 'enabled'. The value of the helpSupported boolean parameter defines the registration status of the EVENT_MENU_SELECTION_HELP_REQUEST event. The icon identifier provided will be added to the icon identifier list of the item icon identifier list Simple TLV if all the applets registered to the EVENT_MENU_SELECTION provide it. The Icon list qualifier transmitted to the ME will be 'icon is not self explanatory' if one of the applet registered prefers this qualifier. After the invocation of this method, during the current card session, the SIM Toolkit Framework shall dynamically update the menu stored in the ME
*
*@param     :id - the menu entry identifier supplied by the initMenuEntry() method
             menuEntry - a reference on a byte array, containing the menu entry string
             offset - the position of the menu entry string in the buffer
             length - the length of the menu entry string
             nextAction - a byte coding the next action indicator for the menu entry (or 0)
             helpSupported - equals true if help is available for the menu entry
             iconQualifier - the preferred value for the icon list qualifier
             iconIdentifier - the icon identifier for the menu entry (0 means no icon)
*
*@return    : void
*
*@exception : java.lang.NullPointerException - if menuEntry is null 
              java.lang.ArrayIndexOutOfBoundsException - if changeMenuEntry would cause access of data outside array bounds. 
              ToolkitException - with the following reason codes: 
                   MENU_ENTRY_NOT_FOUND if the menu entry does not exist for this applet 
                   ALLOWED_LENGTH_EXCEEDED if the menu entry string is bigger than the alloacted space
              javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded 
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void ChangeMenuEntryPublic(void)
{
	UWORD xdata menuString;
	BYTE idata maxMenuLgth,menuRec;
	BYTE xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum;
	
	jshort apiMode = PopOperandStackShort();
	
	jshort iconIdentifier = PopOperandStackShort();
    
	jbyte iconQualifier = PopOperandStackShort();
    
	jboolean helpSupported = PopOperandStackShort();
    
	jbyte nextAction = PopOperandStackShort();
    
	jshort length = PopOperandStackShort();
    
	jshort offset = PopOperandStackShort();
    
	jref menuEntry = PopOperandStackShort();
    
	jbyte id = PopOperandStackShort();

	menuString = GetValidArrayDataAddr(menuEntry,offset,length);//GetArrayBufferAddress(menuEntry)+ offset;

	if((iconIdentifier > 0xFF)||(iconIdentifier < 0x00))  //iconIdentifier代表的是EF IMG文件中的记录号，所以其的取值范围应该为01-FF之间
	{
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);		//todo: 抛出异常0EGISTRY_ERROR
		return;
	}
	
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();

	recNum = getPreviousStackContext()&0xFF;//当前appid
#else
	recNum = GetCurrentToolkitEntryAppNum();
	
	if(recNum == FALSE)
	{
		return;
	}
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{	
		return;	
	}

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum,MaxEntryTextLen_OFFSET, &maxMenuLgth,sizeof(BYTE));

	if(length > maxMenuLgth )
	{
		 UiccOrSimThrowToolkitException(ALLOWED_LENGTH_EXCEEDED, apiMode);
		 return;
	}

	menuRec = ToolKitFindAppRecordNumById(id);//通过meun ID找在menuentry文件的记录号

	if(menuRec == FALSE)
	{
	    UiccOrSimThrowToolkitException(MENU_ENTRY_NOT_FOUND, apiMode); 
		return;
	}

	updateMenuInfor(menuString,length, nextAction, helpSupported, iconQualifier, iconIdentifier,id,menuRec,0x01);
	
	ReadLFBufForJavaAccess(toolkitAppAddr, recNum,EventList_OFFSET,&tmp[0],sizeof(tmp));

	ToolkitSetEventBit(&tmp[0],EVENT_MENU_SELECTION_bit);

	if(helpSupported)
	{
		ToolkitSetEventBit(&tmp[0],EVENT_MENU_SELECTION_HELP_REQUEST_bit);
	}
	else  //查找当前应用是否还有其他菜单支持EVENT_MENU_SELECTION_HELP_REQUEST ，如果其他菜单都不支持EVENT_MENU_SELECTION_HELP_REQUEST，且当前改变的菜单也不支持help，那么注销EVENT_MENU_SELECTION_HELP_REQUEST事件
	{
		if(ToolkitOtherMenuIDSupportHelp(id,recNum) == FALSE)
		{
			ToolkitClearEventBit(&tmp[0],EVENT_MENU_SELECTION_HELP_REQUEST_bit);
		}
	}

	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,EventList_Length);

	SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);	  	//todo:打上setupmenu 标记,等待发送setup menu主动式命令
}

/*
*@function   :Toolkit_setMenuStatus
*
*@description:在EF_ToolkitMenuEntry文件中查找特定的menuId,并将其菜单状态设置为特定值
*
*@param      :menuId
*@param      :menuStatus
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131031
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void setMenuStatusPublic(BYTE menuStatus)
{
	BYTE idata recNum,appletstatus,toolkitAppRecNum;
	UWORD xdata tookitMenuEntryAddr;
	UWORD xdata toolkitAppAddr;
	jbyte idata menuId;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	menuId = PopOperandStackShort();

	recNum = ToolKitFindAppRecordNumById(menuId);
	
	if(recNum == FALSE)
	{
		UiccOrSimThrowToolkitException(MENU_ENTRY_NOT_FOUND, apiMode);
		return;
	}

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#else
	toolkitAppRecNum =  GetCurrentToolkitEntryAppNum();

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{	
		return;	
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr,toolkitAppRecNum,AppletStatus_OFFSET,&appletstatus,1);

	if(ValueBit(appletstatus ,APP_SELECTABLE_BIT)== TRUE)
	{
		menuStatus = menuStatus|Menu_SELECTABLE;
		
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);// 只有当app为selectable时才打主动式命令标记20140318 lj
	}

	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	WriteLFBuf(tookitMenuEntryAddr,recNum,0x00,&menuStatus,sizeof(BYTE));
}

/**
*@function   : Toolkit_getPollInterval
*
*@description : Returns the number of seconds of the adjusted duration for the calling Toolkit Applet.  If the returned duration is equal to POLL_NO_DURATION, the Toolkit Applet is not registered to EVENT_STATUS_COMMAND event.
*              The returned duration may :
*		       - be a multiple of the real adjusted poll interval time at the Terminal.
* 		       - differ from the requested duration due to request of other toolkit applets or due to restrictions of the current Terminal.
*		       - be changed during the card session due requests from other Toolkit Applets.
*		       - be wrong due to direct generation of the proactive command POLL INTERVAL or POLLING OFF.
* 		       - not correspond to the elapsed time due to additional STATUS commands send by the Terminal.
*
*@return    :the number of seconds of the adjusted duration for the applet
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131031
**@history     :V1.1 :create by yjr@20210402 参考102241 V16 void requestPollInterval(short duration)
*																	throws ToolkitException,
*																		TransactionException
*应用在申请PollInterval时，如果申请的值不是0，应该返duration的最小值，如果申请的值是0，需要和当前其他应用申请的最小值比较，在决定发polloff或者pollinterval，
*测试用例uicc_configuration_additional_test_工程集/GP补充测试/GP TEST/GP 补充Multi_app_requestPollInterval_Test01
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others     :
*/
void GetPollIntervalPublic(void)
{
	BYTE xdata tmp[EventList_Length];
	WORD idata pollDuration;
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum,i;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		PushOperandStackShort(0x00);
		return;	
	}
	
//	recNum = GetCurrentToolkitEntryAppNum();

//	ReadLFBufForJavaAccess(toolkitAppAddr, recNum ,EventList_OFFSET, tmp,sizeof(tmp));

//	if(ToolkitGetEventBit(tmp,EVENT_STATUS_COMMAND_bit))
//	{
//		ReadLFBufForJavaAccess(toolkitAppAddr, recNum , PollDuration_OFFSET, (BYTE *)&pollDuration,sizeof(WORD));
//	}
//	else
//	{
//		pollDuration = 0x00;
//	}
	
	recNum = GetFileHeadByte(toolkitAppAddr, F_RecNum_Offset);
#else
	recNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	recNum = (G_ToolkitAppIndex  == 0xFF)? recNum : (G_ToolkitAppIndex + 1);//正在安装Toolkit应用时，要查找正在安装的应用参数  此时bFreeEntryIndex已和Toolkit应用的ToolkitInfo对应

#endif
	for(i=0; i<recNum; i++)
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}
#endif
		ReadLFBufForJavaAccess(toolkitAppAddr,i+1,AppletStatus_OFFSET,&tmp[0],0x01);
		
		if(tmp[0] == APPLET_UNLOCKED_SELECTABLE)
		{			
			ReadLFBufForJavaAccess(toolkitAppAddr,i+1,EventList_OFFSET,tmp,EventList_Length);
			
			if(ToolkitGetEventBit(tmp,EVENT_STATUS_COMMAND_bit))
			{   
				VMReadBuf((BYTE*)&pollDuration,E_CUR_DURATION,sizeof(WORD));
				
				break;
			}
			
		}
	}
	
	if(i == recNum)
	{
		pollDuration = 0x00;
	}
	
	PushOperandStackShort(pollDuration);
}
/**
*@function    : Toolkit_initMenuEntry
*
*@desccription: Initialises the next menu entry allocated at loading. The default state of the menu entry is'enabled'. The value of the helpSupported boolean parameterdefines the registration status of the applet to the event EVENT_MENU_SELECTION_HELP_REQUEST. The applet is registered to the EVENT_MENU_SELECTION. The icon identifier provided will be added to the icon identifier list of the item icon identifier list Comprehension TLV if all the applets registered to the EVENT_MENU_SELECTION provide it. The Icon list qualifier transmitted to the Terminal will be 'icon is not self explanatory' if one of the applet registered prefers this qualifier.This method shall be called by the applet in the same order than the order of the item parameters defined at the applet loading if the applet has several menu entries. The applet shall initialise all its loaded menu entries during its installation.
*
*@param       :menuEntry                           a reference on a byte array, containing the menu entry string
*@param       :offset                              offset of the menu entry string in the buffer
*@param       :length                              length of the menu entry string
*@param       :nextAction                          a byte coding the next action indicator for the menu entry (or 0)
*@param       :helpSupported                       equals true if help is available for the menu entry
*@param       :iconQualifier                       the preferred value for the icon list qualifier
*@param       :iconIdentifier                      the icon identifier for the menu entry  (0 means no icon)
*
*@return      :the identifier attached to the initialised menu entry

*@exception   :NullPointerException   if menuEntry is null
*@exception   :ArrayIndexOutOfBoundsException  if offset or length or both would cause access outside array bounds
*@exception   :ToolkitException with the following reason codes: 
*               <li>REGISTRY_ERROR if the menu entry cannot be initialised (e.g. no more item data in applet loading parameter)
*	            <li>ALLOWED_LENGTH_EXCEEDED if the menu entry string is bigger than the allocated space
*@exception   :TransactionException  if the operation would cause the commit capacity to be exceeded
*
*@author       :xuyz
*
*@history      :V1.0:create by xuyz@20131031
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others       :使用该函数的其他一些限制条件
*/

void InitMenuEntryPublic(void)
{
	UWORD xdata menuString;
	BYTE idata count,menuRec;
	BYTE idata menuPosition,menuId;
	UWORD xdata toolkitMenuEntryIndexAddr;
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum,fileLen;
		
	jshort idata apiMode = PopOperandStackShort();
	jshort idata iconIdentifier = PopOperandStackShort();
	jbyte idata iconQualifier = PopOperandStackShort();
	jboolean idata helpSupported = PopOperandStackShort();
	jbyte idata nextAction = PopOperandStackShort();
	jshort idata length = PopOperandStackShort();
	jshort idata offset = PopOperandStackShort();
	jref idata menuEntry = PopOperandStackShort();

	menuString = GetValidArrayDataAddr(menuEntry,offset,length);//GetArrayBufferAddress(menuEntry)+ offset;
	
	if((iconIdentifier > 0xFF)||(iconIdentifier < 0x00))   ////iconIdentifier代表的是EF IMG文件中的记录号，所以其的取值范围应该为01-FF之间
	{
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);	  //todo: 抛出异常0EGISTRY_ERROR
		return;
	}

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
	fileLen = ToolkitApp_Info_Size;
#else
	recNum = GetCurrentToolkitEntryAppNum();
	
	if(recNum == FALSE)
	{
		return;
	}

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{
		return;	
	}
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	fileLen = GetLFRecNumLgth(toolkitAppAddr, FileLgth);
#endif
	ReadLFBufForJavaAccess(toolkitAppAddr,recNum ,0x00,tempBuf,fileLen);

	if(length > tempBuf[MaxEntryTextLen_OFFSET])
	{
		UiccOrSimThrowToolkitException(ALLOWED_LENGTH_EXCEEDED, apiMode);	  //todo: 抛出异常0EALLOWED_LENGTH_EXCEEDED
		return;
	}

	count = ToolkitCountMenuIdNumber(COUNT_MODE); 	

	if(count >= tempBuf[MaxEntryCount_OFFSET])	 //所有菜单已分配完毕
	{
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);	  //todo: 抛出异常0EGISTRY_ERROR
		return;
	}

	menuRec = ToolkitCountMenuIdNumber(FINDNULL_MODE);  //查找一条空纪录

	if(menuRec == FALSE)  //菜单文件无空记录可以被更新
	{
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);	  //todo: 抛出异常0EGISTRY_ERROR
		return;
	}

	menuPosition = tempBuf[MenuEntryPosition_OFFSET + 2*count] ;

	menuId =   tempBuf[MenuEntryPosition_OFFSET + 2*count+1] ;

	if(menuId >= 0x80)
	{		
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);	  //todo: 抛出异常0EGISTRY_ERROR
		return;
	}
	
     count = ToolkitFindSpecifiedMenuId(menuId);

	if(count != FALSE)	 	//If the requested item identifier in the range [1-127] is already allocated, then the card shall reject the install command
	{
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);	  //todo: 抛出异常0EGISTRY_ERROR
		return;
	}
	  	
	if(menuId == 0x00)	 //由toolkit framework在[128-255]之间分配一个空闲值
	{
		for(menuId = 0x7F ; menuId <= 0xFE; menuId++)
		{
			 count = ToolkitFindSpecifiedMenuId	(menuId +1);

			 if(count == FALSE)  //查找到一个空ID
			 {
			 	break;
			 }
		}
		if(menuId == 0xFF)
		{			
			UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);	  //todo: 抛出异常0EGISTRY_ERROR
			return;
		}
		menuId ++;
	}	

	ToolkitSetEventBit(&tempBuf[EventList_OFFSET],EVENT_MENU_SELECTION_bit);

	if(helpSupported)
	{
		ToolkitSetEventBit(&tempBuf[EventList_OFFSET],EVENT_MENU_SELECTION_HELP_REQUEST_bit);
	}

	WriteLFBufForJavaAccess(toolkitAppAddr,recNum ,0,tempBuf,fileLen);

	if(tempBuf[AppletStatus_OFFSET] == APPLET_UNLOCKED_SELECTABLE)
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);	  //通过C2指令安装成功后,发送setupmenu
	}
	
	updateMenuInfor(menuString,length, nextAction, helpSupported, iconQualifier, iconIdentifier,menuId,menuRec,0x00);
	
	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);
	
	ReadTFBuf(toolkitMenuEntryIndexAddr, 0x00,&tempBuf[0],sizeof(BYTE));  //修改 ResData缓存为tempBuf,因为在toolkit中的send可能会重发，此处换成tempBuf的数据，以免ResData中的数据被破坏
    
	ReadTFBuf(toolkitMenuEntryIndexAddr, 0x01,&tempBuf[1],tempBuf[0]);

    if((menuPosition == 0x00)||(menuPosition > tempBuf[0]))  //插入到最后位置  	//插入position
	{
		tempBuf[tempBuf[0]+1] =  menuRec;
	}
	else
	{
		ecpMem_Move(&tempBuf[menuPosition+1],&tempBuf[menuPosition], tempBuf[0] +1- menuPosition);

		tempBuf[menuPosition] = menuRec;
	}

	tempBuf[0] ++;

	WriteTFBuf(toolkitMenuEntryIndexAddr, 0, tempBuf, tempBuf[0]+1);

	PushOperandStackShort((jbyte)menuId);
}

/**
*@function    : Toolkit_releaseTimer
*
*@description :Release a Timer that has been allocated to the calling applet. The applet is deregistered of the EVENT_TIMER_EXPIRATIONfor the indicated Timer Identifier.
*
*@param       :timerIdentifier           the identifier of the Timer to be released	
*
*Return       :void
*
*@exception   :ToolkitException      with the following reason codes: 
*             <li>INVALID_TIMER_ID if the timerIdentifierd is not allocated to this applet.
*@exception  TransactionException  if the operation would cause the commit capacity to be exceeded
*
**@author    :xuyz
*
*@history    :V1.0 :create by xuyz@20131031
*
*@others     :
*/

void ReleaseTimerPublic(void)
{
	jshort idata apiMode;
	jbyte idata timerIdentifier;
	
	apiMode = PopOperandStackShort();
	
	timerIdentifier = PopOperandStackShort();

	if(TimerModuleTimerIdInValid(timerIdentifier))
	{
		UiccOrSimThrowToolkitException(INVALID_TIMER_ID, apiMode);	
		return;
	}

	releaseTimerAndService((timerIdentifier-1),MaxTimerCount_OFFSET,EVENT_TIMER_EXPIRATION_bit,INVALID_TIMER_ID,apiMode);
}

/**
*@function    : RequestPollIntervalPublic。UICCToolkit_requestPollInterval和Toolkit_requestPollInterval的子函数
*
*@See         : sim.toolkit.ToolkitRegistry.requestPollInterval(short)
*               uicc.toolkit.ToolkitRegistry.requestPollInterval(short)
*
*@description : Requests a duration for the EVENT_STATUS_COMMAND event of the registering Toolkit Applet. Due to different duration requested by other Toolkit Applets or due to restriction of the Terminal, the CAT Runtime Environment may  adjust another duration.  This method can be used at every time to request a new duration.
*
*@param       :duration  specifies the number of seconds requested for proactive polling.
*			   The maximum value of duration is 15300 (255 minutes).
*			   If duration is equal to POLL_NO_DURATION, the calling applet deregisters from EVENT_STATUS_COMMAND, and the CAT Runtime Environment may issue the POLLING OFF proactive command.
*			   If duration is equal to POLL_SYSTEM_DURATION,the calling applet registers to the EVENT_STATUS_COMMAND and let the CAT Runtime Environment define the duration.
*
*@return      :void

*@exception   :ToolkitException      with the following reason codes: 
*              <li>REGISTRY_ERROR 	 if duration is greater than the maximum value.
*@exception   :TransactionException  if the operation would cause the commit capacity to be exceeded
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131031
*@history     :V1.1 :create by yjr@20210402 参考102241 V16 void requestPollInterval(short duration)
*																	throws ToolkitException,
*																		TransactionException
*应用在申请PollInterval时，如果申请的值不是0，应该返duration的最小值，如果申请的值是0，需要和当前其他应用申请的最小值比较，在决定发polloff或者pollinterval，
*测试用例uicc_configuration_additional_test_工程集/GP补充测试/GP TEST/GP 补充Multi_app_requestPollInterval_Test01
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*           :modify by xwh@20221026  缺陷439，修改同一个应用，先设置一个较小的interval，再设置一个较大的interval，较大的interval没有起作用的问题
*
*@others
*/
//#define     MAXVALUE_POLL_DURATION             (jshort)15300
void RequestPollIntervalPublic(void)
{
	BYTE  idata appstatus,toolkitNum,transActioinFlag;
	BYTE xdata tmp[EventList_Length];
	WORD xdata minDuration,appDuration,sysduration;
	UWORD xdata toolkitAppAddr;
    jshort idata apiMode;
	jshort idata duration;
	
	apiMode = PopOperandStackShort();
	
	duration = PopOperandStackShort();

	if((duration > MAXVALUE_POLL_DURATION)||(duration < POLL_SYSTEM_DURATION))
	{
		UiccOrSimThrowToolkitException(REGISTRY_ERROR, apiMode);
		return;		
	}
	
	toolkitNum = GetCurrentToolkitEntryAppNum();//应用未注册时 找不到entry，需要返回befreindex后续使用?

	if(toolkitNum == FALSE)
	{
		return;
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return;	
	}
#endif
	ReadLFBufForJavaAccess(toolkitAppAddr, toolkitNum , EventList_OFFSET, tmp,sizeof(tmp));

	ReadLFBufForJavaAccess(toolkitAppAddr, toolkitNum , AppletStatus_OFFSET, &appstatus,1);

	transActioinFlag = IF_BeginTransaction();

	if(duration == POLL_NO_DURATION)
	{
		 ToolkitClearEventBit(tmp,EVENT_STATUS_COMMAND_bit);

		 if(G_STKBusy == FALSE)	  //此处增加G_STKBusy == FALSE的原因： 脚本Cre_Apt_Estc ,testCase2:app1 app2在主动式命令中被触发，不要求返回Poll  Off主动式命令； Cre_Pcs_Spco却要求在status触发app，requestPollInterval(POLL_NO_DURATION)要求发送Poll Off主动式命令，
		 {
			minDuration = FindMinDuration(toolkitNum);
			 
			ReadLFBufForJavaAccess(toolkitAppAddr,toolkitNum,PollDuration_OFFSET,(BYTE *)&appDuration, sizeof(WORD));
			
		 	if(appstatus == APPLET_UNLOCKED_SELECTABLE)
		 	{
		 		
				if(appDuration < minDuration)
				{
					if(minDuration == (MAXVALUE_POLL_DURATION + 1))
					{
						SetG_ToolkitSystemProactiveFlag(Bit_PollOff);
						
						VMWriteBuf(E_CUR_DURATION,(BYTE*)&duration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 
					}
					else
					{
						SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);

						SetBit(G_PollIntervalRecNum,(toolkitNum-1));

						VMWriteBuf(E_CUR_DURATION,(BYTE*)&minDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 						
					}
					
				}

		 	}
		 }
	}
	else
	{//联通内部脚本26.Bug_List_Contact_工程集/Bug_List_41_03等会测到
		if(duration == POLL_SYSTEM_DURATION)    //the SIM Toolkit Framework define the duration
		 {//按照移动要求，需要系统值为0x1E,增加配置，使值可配byyanjiru@20221104
		 	  duration = JcvmConfig_GetPollSystemDuration();   //测试用例TS102268中 5.5.4.1case9-case12,当POLL_SYSTEM_DURATION主动式命令发送的是84020010
		 }
		 
		 //VMReadBuf((BYTE*)&duration,E_CUR_DURATION,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取
		 VMReadBuf((BYTE*)&sysduration,E_CUR_DURATION,sizeof(WORD));
		 
		 minDuration = FindMinDuration(toolkitNum);

		 //当前应用是唯一应用，那么他设置任何duration都起作用
		 if(minDuration == (MAXVALUE_POLL_DURATION + 1))
		 {
			sysduration = POLL_NO_DURATION;
		 }
		 //还有其他应用
		 else  //  modify by xuweihua@20221115 bug 439 第二次修改
		 {
		 	 //读出当前应用旧的duration
			 ReadLFBufForJavaAccess(toolkitAppAddr,toolkitNum,PollDuration_OFFSET,(BYTE *)&appDuration, sizeof(WORD));

			 //系统值是当前应用上一次设置的
			 if((appDuration == sysduration)&&(minDuration > sysduration))
			 {
			 	//当前应用设置一个duration，比其他应用的最小值大，那么不起作用,将系统值设置成其他应用的最小值
				if(minDuration < duration)
				{

				}
				//当前应用设置一个duration，小于等于其他应用的最小值，那么设置起作用
				else
				{
					sysduration = POLL_NO_DURATION;
				}
			 }
		 }
		 
		 ToolkitSetEventBit(&tmp[0],EVENT_STATUS_COMMAND_bit);
		 
		 if((duration < sysduration) ||(sysduration == POLL_NO_DURATION) ||(duration < minDuration))
		 {
			 if(appstatus == APPLET_UNLOCKED_SELECTABLE)
			 {
				 SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);

			 	SetBit(G_PollIntervalRecNum,(toolkitNum-1));
				 
				VMWriteBuf(E_CUR_DURATION,(BYTE*)&duration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 
			 }
		 }		 
		 else if((duration > minDuration)&&(minDuration != sysduration))
		 {
			 if(appstatus == APPLET_UNLOCKED_SELECTABLE)
			 {
				 SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);

			 	SetBit(G_PollIntervalRecNum,(toolkitNum-1));
				 
				VMWriteBuf(E_CUR_DURATION,(BYTE*)&minDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 
			 }
		 }
	}

	WriteLFBufForJavaAccess(toolkitAppAddr,toolkitNum, PollDuration_OFFSET,(BYTE*)&duration,sizeof(WORD));

	WriteLFBufForJavaAccess(toolkitAppAddr,toolkitNum, EventList_OFFSET,tmp,sizeof(tmp));
	
	IF_CommitTransaction(transActioinFlag);

	PushOperandStackShort(duration);
}
