 /************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCToolkitRegistery.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-30

*Description :该文件主要用于实现基于TS102241 所描述的interface ToolkitRegistry的功能 ，该接口主要allow any Toolkit Applet to register its configuration (supported events) during the install phase and possibly to change it during all the applet life time,该对象为永久入口点对象
*
*History     :
	
author    time      version    description
xuyz   2013/10/30   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include "fileAction.h"
#include "gsmpublic.h"
#include "hard_api.h"
#include "UICCToolkitAPI.h"
#include "UICCToolkitConstants.h"
#include "ToolkitAPIPublic.h"
#include "uicc_interface.h"
#include "platform_api.h"
#include "filesystem.h"
#include "ToolkitRFM.h"
#include "opFunction.h"
#include "bc_array.h"
#include "object.h"
#include "ob_manager.h"

#if UICC__FUNCTION__UICCAPI

/*
*@function   :Toolkit_isEventAllowed
*
*@description :判断当前事件，卡片是否允许
*
*@param       :localEvent
*@mode        :TRUE:判断是否允许事件为EVENT_EXTERNAL_FILE_UPDATE_bit or  EVENT_REMOTE_FILE_UPDATE_bit /FALSE：不用判断这两个时间
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131030
*
*@others      :使用该函数应该注意的一些限制条件
*/
void UICCToolkit_isEventAllowed(BYTE event,BYTE mode)
{
	if((event == EVENT_MENU_SELECTION_bit)||(event == EVENT_MENU_SELECTION_HELP_REQUEST_bit)||(event == EVENT_TIMER_EXPIRATION_bit)||(event ==EVENT_STATUS_COMMAND_bit )||(event == EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit))
	{
	    UICCThrowToolkitException(EVENT_NOT_ALLOWED); 	//todo :抛出异常  EVENT_NOT_ALLOWED
	}

	if(mode == TRUE)
	{
		if((event == EVENT_EXTERNAL_FILE_UPDATE_bit)||(event == EVENT_REMOTE_FILE_UPDATE_bit))
		{
		    UICCThrowToolkitException(EVENT_NOT_ALLOWED); 	//todo :抛出异常  EVENT_NOT_ALLOWED
		}
	}
}

/*
*@function   :Toolkit_transferStandardToLocalEventType
*
*@description:将api函数传递进来的event 转换成卡片支持的event，同时判断当前事件类型卡片是否支持,如果不支持抛出异常EVENT_NOT_SUPPORTED
*
*@param      :javaEvent 
*
*@return      :TRUE/FALSE
*
*@author      :xuyz
*
*@hisroty     :V1.0: create by xuyz@20131030
*
*@others      :
*/
BYTE UICCToolkit_transferStandardToLocalEventType(jshort standardEvent)
{
	if(standardEvent == 0x00)
	{
		UICCThrowToolkitException(EVENT_NOT_SUPPORTED); 		//TODO :抛出异常   EVENT_NOT_SUPPORTED	
	}

	if((standardEvent >= EVENT_PROACTIVE_HANDLER_AVAILABLE)&&(standardEvent <= EVENT_MEMORY_FAILURE ))
	{
		standardEvent -= 83;   //40 EVENT_PROACTIVE_HANDLER_AVAILABLE_bit  ------EVENT_MEMORY_FAILURE_bit 46
	}

	if(standardEvent == EVENT_UNRECOGNIZED_ENVELOPE)
	{
		standardEvent = EVENT_UNRECOGNIZED_ENVELOPE_bit;
	}

	if((standardEvent > EVENT_MEMORY_FAILURE_bit)||(standardEvent < 0x00))
	{
		UICCThrowToolkitException(EVENT_NOT_SUPPORTED); 		//TODO :抛出异常   EVENT_NOT_SUPPORTED
	}

	return standardEvent;
}
/*
*@function   :
*
*@description:在设置当前standardEvent时,判断该事件的合法性,如卡片是否支持该事件,事件是否已注册,tar有没有定义,是否允许注册该事件等
*
*@param       :jshort: standardEvent,API函数传进来的值
*
*@return      :byte localEvent:卡片转换后的event标示
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131031
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE UICCToolkit_isStandardEventAvaible(jshort standardEvent)
{
	BYTE idata localEvent;
	BYTE  xdata tmp[0x03];
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	recNum = GetCurrentToolkitEntryAppNum();

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#else
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#endif
	if(toolkitAppAddr == FALSE)
	{
		UICCThrowToolkitException(TAR_NOT_DEFINED); 
	}

	localEvent = UICCToolkit_transferStandardToLocalEventType(standardEvent);

	UICCToolkit_isEventAllowed(localEvent,TRUE);

	if((localEvent == EVENT_FORMATTED_SMS_PP_ENV_bit)||(localEvent == EVENT_FORMATTED_SMS_PP_UPD_bit) ||(localEvent == EVENT_FORMATTED_SMS_CB_bit))
	{
		ReadLFBufForJavaAccess(toolkitAppAddr,recNum ,TARLength_OFFSET,&tmp[0],0x01);

		if(tmp[0] == 0x00)
		{
			UICCThrowToolkitException(TAR_NOT_DEFINED); 
		}
	}
    //ts 102241 暂时未定义这几个事件，是否需要实现？

	if(ToolkitEventRegistered(localEvent))
	{		
		UICCThrowToolkitException(EVENT_ALREADY_REGISTERED);  	//todo: 抛出异常 EVENT_ALREADY_REGISTERED
	}

	return localEvent; 
}
/**
*@function    :Toolkit_setEventList
*
*@description :Sets an event list in the Toolkit Registry entry of the applet.In case of any exception the state of the registry is unchanged.
*
*@param       :eventList                           buffer containing the list of the new events to register
*@param       :offset                              offset in the eventlist buffer for event registration
*@param       :length                              length in the eventlist buffer for event registration
*
*@return      :void
*
*@exception   :NullPointerException            if eventlist is null
*@exception   :ArrayIndexOutOfBoundsException  if offset or length or both would cause access outside array bounds
*@exception   :ToolkitException                with the following reason codes: 
*              <li>EVENT_NOT_SUPPORTED if one event is not supported
*              <li>EVENT_TAR_NOT_DEFINED if the event requests a TAR and the applet has not at least one TAR value assigned.
*              <li>EVENT_ALREADY_REGISTERED if one event has already been registered (for limited event like Call Control)
*              <li>EVENT_NOT_ALLOWED if eventList contains EVENT_MENU_SELECTION,EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND, EVENT_EVENT_ DOWNLOAD_LOCAL_CONNECTION,EVENT_EXTERNAL_FILE_UPDATE,EVENT_REMOTE_FILE_UPDATE
*@exception   TransactionException  if the operation would cause the commit capacity to be exceeded
*
*@author     :xuyz
*
*@hisotry    :V.0:create by xuyz 20131031
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others     :1: TS 102241 eventList的类型为short[];而43.019 中定义该字段为byte[]，要注意处理上是否一样
*/
void UICCToolkit_setEventList(void)
{
	WORD idata i;
	jshort idata standardEvent;
	BYTE idata localEvent,result,event[0x02],appState;
	BYTE xdata  tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	BYTE idata recNum;

	jshort length = PopOperandStackShort();
    
	jshort offset = PopOperandStackShort();
    
	jref eventList = PopOperandStackShort();
	
	check_bounds_interval_buffer(eventList, offset, length);
	
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#else
	recNum = GetCurrentToolkitEntryAppNum();

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{
		return;
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,  EventList_OFFSET,&tmp[0],EventList_Length);

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum, AppletStatus_OFFSET,&appState,sizeof(BYTE));	

	for(i=0 ; i< length; i++)
	{
		CopyArrayDataToDest((BYTE*)&event[0],eventList,(offset+i)*2,2);//该数组为short型数组，偏移要*2
	   
	    standardEvent = ((event[0] << 8)&0xFF00)|event[1];

		localEvent = UICCToolkit_isStandardEventAvaible(standardEvent);

		if((localEvent >= EVENT_EVENT_DOWNLOAD_MT_CALL_bit)&&(localEvent <= EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit)&&(localEvent != EVENT_STATUS_COMMAND_bit))
		{
			if(appState==APPLET_UNLOCKED_SELECTABLE)
			{
				SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
			}
		}

		ToolkitSetEventBit(&tmp[0],localEvent);
	}

/*	result = */WriteLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,EventList_Length);

/*	if(result == FALSE)
	{
		//抛出异常: TransactionException   
	}*/
}
/**
*@function    : Toolkit_allocateServiceIdentifier
*
*@description :Allocate a Service identifier that the applet can manage.<br>
*              By calling this method the applet is registered to the EVENT_EVENT_ DOWNLOAD_LOCAL_CONNECTION of the allocated service.<br>
*              The service identifier is allocated by the applet until it explicitly releases it.It can then issue the proactive command DECLARE SERVICE to add or delete a service to the terminal service database.<br>
*
*@return      :the identifier of the Service allocated to the applet
*
*@exception   :ToolkitException      with the following reason codes: 
*             <li>NO_SERVICE_ID_AVAILABLE if all the services are allocated or the maximum number of services Identifiers have been allocated to this applet
*@exception   :TransactionException 
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131101
*
*@others      :该函数使用时的一些限制条件 
*/
void UICCToolkit_allocateServiceIdentifier(void)
{
	BYTE idata unallocatedServiceId;

	unallocatedServiceId = findUnallocatedTimerIdAndServiceId(MaxServiceCount_OFFSET);

	if(unallocatedServiceId == FALSE)
	{
		UICCThrowToolkitException(NO_SERVICE_ID_AVAILABLE);
		return;
	}

	unallocatedServiceId --;

    allocateTimerAndService(unallocatedServiceId,MaxServiceCount_OFFSET,EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit,NO_SERVICE_ID_AVAILABLE,UICCAPI_Mode);

	PushOperandStackShort((SBYTE)unallocatedServiceId);
} 

/**
*@function    : Toolkit_releaseServiceIdentifier
*
*@description :Release a Service Identifier that has been allocated to the calling applet.<br>
*              The applet is deregistered of the EVENT_EVENT_ DOWNLOAD_LOCAL_CONNECTION for the indicated Service Identifier.
*
*@param       :serviceIdentifier the identifier of the Service to be released
*
*@exception   :ToolkitException with the following reason codes: 
*              <li>INVALID_SERVICE_ID if the serviceIdentifier is not allocated to this Applet.
*@exception   :TransactionException  
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131101
*
*@others      :
*/
void UICCToolkit_releaseServiceIdentifier(void)
{
	jbyte serviceIdentifier = PopOperandStackShort();

	if((serviceIdentifier < 0)||(serviceIdentifier > 7))
	{
		UICCThrowToolkitException(INVALID_SERVICE_ID);
		return;
	}

	releaseTimerAndService(serviceIdentifier,MaxServiceCount_OFFSET,EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit,INVALID_SERVICE_ID, UICCAPI_Mode);
}

/*@function:    deregisterFileEvent                                            
 * 
 *@descrip:     注销FileList event ,需要借用tempBuf[200]以后的数据
 *
 *@see:	       
 *@param:		event	---事件(已转成本地事件)                                                                                                                                     
 *@param:       filelist---需要查找的文件路径
 *				                                                                                                         
 *@return:                                                                                                                                                                                                                                                                         
 * 
 *@author:      ZZJ 20131119
 *  
 *@history:     V0.0---Create by ZZJ 20131119 
 *           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
 *                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 *           :modify by xuweihua@20211113,如果toolkitNum 大于EF_ToolkitAppPathInfo 的记录个数，报错
 *
 *@others      :1:注意卡片上的文件ID不允许为0xFFFF，因为FFFF在6F83中标示无效ID和空块号          
 */
void  deregisterFileEvent(BYTE event, BYTE *filelist)
{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	u8     X_A   recNum,blkNum,result,offset,i,j,recLgth;
	u8     X_A   ptr[EventList_Length];
	u32    X_A   toolkitFileListAddr,toolkitInfoObj;
	u32    X_A   toolkitPathInfoAddr;
	u32    X_A   toolkitAppAddr;
	u8     X_A   mode,toolkitNum,transActioinFlag;

	toolkitNum = GetCurrentToolkitEntryAppNum();

	if(toolkitNum == FALSE)
	{
		return ;
	}

	if(EVENT_EXTERNAL_FILE_UPDATE_bit)
	{
		mode = MODE_EXTERNAL_FILE_LIST;
	}
	else
	{
		mode = MODE_REMOTE_FILE_LIST;
	}

	if(SearchFileList(filelist,toolkitNum, mode) == FALSE)//未注册过
	{
		return;
	}

	transActioinFlag = IF_BeginTransaction();

	toolkitInfoObj = GetToolkitInfoObj(toolkitNum);

	clearToolkitEventFileList(toolkitInfoObj, filelist, mode);

	if(GetFirstToolkitEventFileListObj(toolkitInfoObj,mode) == ADDR_IS_NULL)
	{
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(toolkitInfoObj);

		ReadLFBufForJavaAccess(toolkitAppAddr,toolkitNum , EventList_OFFSET, &ptr[0],EventList_Length);

		ToolkitClearEventBit(&ptr[0],event);

		WriteLFBufForJavaAccess(toolkitAppAddr,toolkitNum, EventList_OFFSET,ptr,EventList_Length);
	}
	IF_CommitTransaction(transActioinFlag);
#else
	BYTE idata recNum,blkNum,result,offset,i,j,recLgth;
	BYTE* ptr;
	UWORD xdata toolkitFileListAddr;
	UWORD  xdata toolkitPathInfoAddr;
	UWORD xdata toolkitAppAddr;
	BYTE idata toolkitNum;

	ptr = (BYTE*)&tempBuf[200];

	result = SearchFileList(filelist, &blkNum);

	if(result == FALSE)						  //未找到相同filelist
	{
		return;
	}

	toolkitNum = GetCurrentToolkitEntryAppNum();

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return;
	}

	toolkitFileListAddr = SelectTFFile(DF_Toolkit_P, EF_ToolkitFileList);

	toolkitPathInfoAddr =  SelectLFFile(DF_Toolkit_P, EF_ToolkitAppPathInfo);

	recLgth =  GetLFRecNumLgth(toolkitPathInfoAddr, FileLgth)>>1;
	recNum =  GetLFRecNumLgth(toolkitPathInfoAddr, FileNum);

	if(recNum < toolkitNum)
	{
		//空间不足
		UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
	}

	if(event == EVENT_EXTERNAL_FILE_UPDATE_bit)					   //记录前半段注册EVENT_EXTERNAL_FILE_UPDATE_bit，后半段注册EVENT_REMOTE_FILE_UPDATE
	{
		offset = 0;
	}
	else 
	{
		offset = recLgth;
	}

	ReadLFBufForJavaAccess(toolkitPathInfoAddr,toolkitNum,offset,ptr,recLgth);

	for(i=1;i<=ptr[0];i++)
	{
		if(ptr[i] == blkNum)															
		{
			ecpMem_Copy(&ptr[i],&ptr[i+1],ptr[0]-i);											   //删除对应注册信息
																				   		
			ptr[0]--;

			result = WriteLFBufForJavaAccess(toolkitPathInfoAddr,toolkitNum,offset,ptr,ptr[0]+1);	 //回写

			if(result == FALSE)
			{
				 //抛出异常 TransactionException
			}
																						   //注销事件
			if(ptr[0] == 0)
			{
				ReadLFBufForJavaAccess(toolkitAppAddr,toolkitNum , EventList_OFFSET, &ptr[0],EventList_Length);

				ToolkitClearEventBit(&ptr[0],event);

				/*result = */WriteLFBufForJavaAccess(toolkitAppAddr,toolkitNum, EventList_OFFSET,ptr,EventList_Length);

				/*if(result == FALSE)
				{
		 		  //抛出异常 TransactionException
				}*/
			}

			recNum = GetLFRecNumLgth(toolkitPathInfoAddr, FileNum);					   //整理filelist

			for(i=1;i<=recNum;i++)
			{
				ReadLFBufForJavaAccess(toolkitPathInfoAddr,i,offset,ptr,recLgth);

				for(j=1;j<=ptr[0];j++)
				{
					if(ptr[j] == blkNum)
					{																		//改块filelist仍然有效，不需要清空
						return;
					}		
				}
			}

			ecpMem_Set(ptr,0xFF,10);																//该块filelist无效，清空

			result = WriteTFBufForJavaAccess(toolkitFileListAddr,(blkNum-1)*10,ptr,10);

			if(result == FALSE)
			{
		 	  //抛出异常 TransactionException
			}

			return;
		}
	}
#endif
}

/*@function:    registerFileEvent                                            
 * 
 *@descrip:     注册FileList event ,需要借用tempBuf[200]以后的数据
 *
 *@see:	       
 *@param:		event	---事件(已转成本地事件)                                                                                                                                     
 *@param:       filelist---需要查找的文件路径 ，固定为8字节场，0xFFFF表示无效ID
 *				                                                                                                         
 *@return:                                                                                                                                                                                                                                                                         
 * 
 *@author:      ZZJ 20131119
 *  
 *@history:     V1.0---Create by ZZJ 20131119 
 *
 *                  V1.1 :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
 *                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 *
 *                  V1.2:modify by xuweihua@20211113,如果toolkitNum 大于EF_ToolkitAppPathInfo 的记录个数，报错
 *
 *@others       1:注意卡片上的文件ID不允许为0xFFFF，因为FFFF在6F83中标示无效ID和空块号          
 */
void  registerFileEvent(BYTE event, BYTE *filelist)
{
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	BYTE idata blkNum,result,offset,i,recLgth,recNum;
	BYTE* ptr;
	UWORD xdata toolkitFileListAddr;
	UWORD  xdata toolkitPathInfoAddr;
	UWORD xdata toolkitAppAddr;
	BYTE idata toolkitNum,transActioinFlag;

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return;
	}

	toolkitPathInfoAddr =  SelectLFFile(DF_Toolkit_P, EF_ToolkitAppPathInfo);

	toolkitFileListAddr = SelectTFFile(DF_Toolkit_P, EF_ToolkitFileList);

	ptr = (BYTE*)&tempBuf[200];

	result = SearchFileList(filelist, &blkNum);

	transActioinFlag = IF_BeginTransaction();

	if(result == FALSE)						  //未找到相同filelist，需要重新创建
	{
		if(blkNum == FALSE)
		{
			//无剩余空间
			UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
		}

		result = WriteTFBuf(toolkitFileListAddr,(blkNum-1)*10,filelist,10);
	
		if(result == FALSE)
		{
		   //抛出异常 TransactionException
			UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
		}
	}
	
	recLgth =  GetLFRecNumLgth(toolkitPathInfoAddr, FileLgth)>>1;	
	recNum =  GetLFRecNumLgth(toolkitPathInfoAddr, FileNum);

	if(event == EVENT_EXTERNAL_FILE_UPDATE_bit)					   //记录前半段注册EVENT_EXTERNAL_FILE_UPDATE_bit，后半段注册EVENT_REMOTE_FILE_UPDATE
	{
		offset = 0;
	}
	else 
	{
		offset = recLgth;
	}
	
	toolkitNum = GetCurrentToolkitEntryAppNum();

	if(recNum < toolkitNum)
	{
		//空间不足
		UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
	}

	ReadLFBuf(toolkitPathInfoAddr,toolkitNum,offset,ptr,recLgth);	 //	注意此处记录长度要小于56字节，因为 ptr从tempBuf[200]开始，注意防止越界
	
	for(i=1;i<=ptr[0];i++) 
	{
		if(ptr[i] == blkNum)
		{
			return;												  //有相同注册信息直接返回
		}
	}

	if(i >= recLgth)
	{
		//空间不足
		UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
	}

	ptr[i] = blkNum;											  //添加新的注册信息

	ptr[0] ++;

	result = WriteLFBuf(toolkitPathInfoAddr,toolkitNum,offset,ptr,ptr[0]+1);
		
	if(result == FALSE)
	{
		 //抛出异常 TransactionException
		UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
	}
#else
	BYTE ptr[EventList_Length];
	UWORD xdata toolkitAppAddr,toolkitInfoObj,ToolkitEventFileListObj,ToolkitFileListArrAddr;
	BYTE idata toolkitNum,backCc;
	u16   X_A     ToolkitEventFileListRef,ToolkitFileListArrRef;
	u8    X_A     transActioinFlag,mode;

	toolkitNum = GetCurrentToolkitEntryAppNum();

	if(toolkitNum == FALSE)
	{
		return;
	}

	toolkitInfoObj = GetToolkitInfoObj(toolkitNum);
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(toolkitInfoObj);

	if(event == EVENT_EXTERNAL_FILE_UPDATE_bit)
	{
		mode = MODE_EXTERNAL_FILE_LIST;
	}
	else
	{
		mode = MODE_REMOTE_FILE_LIST;
	}


	if(SearchFileList(filelist,toolkitNum, mode) != FALSE)//已经注册过
	{
		return;
	}
	backCc = cc;
	cc = getPreviousStackContext();

	transActioinFlag = IF_BeginTransaction();

	ToolkitEventFileListRef = newObject(TOOLKIT_EVENT_FILELIST_CLASS_OFFSET,UICC_PKG_ID,BIT_NEED_NOTHING);//modify by zhxx@20221214,12.UICC API TEST/cat runtime environment/minimum handler availability/cre_mha_pahd,删除应用时6985

	ToolkitFileListArrRef  =  _newarrayObject(AType_Byte, 10, BIT_NEED_REFRESH);

	ToolkitEventFileListObj = locateObjectInMemory(ToolkitEventFileListRef);

	nvmWriteU16Atomic(MEMtPN8(get_object_field_ref(MEMtPF8(ToolkitEventFileListObj), ToolkitFileList_INDEX)),ToolkitFileListArrRef);

	ToolkitFileListArrAddr = get_array_data(MEMtPF8(locateObjectInMemory(ToolkitFileListArrRef)));

	WriteEepromAtomic(ToolkitFileListArrAddr,filelist,10);

	SetToolkitEventFileListObj(toolkitInfoObj,ToolkitEventFileListRef,mode);

	cc = backCc;

#endif
	ReadLFBufForJavaAccess(toolkitAppAddr, toolkitNum , EventList_OFFSET, &ptr[0],EventList_Length);				//注册事件
	
	if(ToolkitGetEventBit(&ptr[0],event) == FALSE)
	{	 
		ToolkitSetEventBit(&ptr[0],event);
	
		/*result = */WriteLFBufForJavaAccess(toolkitAppAddr,toolkitNum,EventList_OFFSET,ptr,EventList_Length);
	
		/*if(result == FALSE)
		{
			  //抛出异常 TransactionException
			UICCThrowAdminException(NOT_ENOUGH_MEMORY_SPACE);
		}*/
	}	

	IF_CommitTransaction(transActioinFlag);
}
/*
*@funtion     :isFileEventAvaible
*
*@description :判断当前的fileEvent事件是否有效
*
*@param       :event
*
*@return      :localEvent
*
*@author      :xuyz
*
*history      :V1.0 :create by xuyz@20131120
*
*@others      :
*/
BYTE  UICCisFileEventAvaible(jshort fileEvent)
{
	BYTE idata localEvent;

	localEvent = UICCToolkit_transferStandardToLocalEventType(fileEvent);

	if(localEvent != EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit)
	{
		UICCToolkit_isEventAllowed(localEvent,FALSE);  //该函数也判断了事件EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit，注意修正
	}

	if((localEvent != EVENT_EXTERNAL_FILE_UPDATE_bit)&&(localEvent != EVENT_REMOTE_FILE_UPDATE_bit))
	{
		UICCThrowToolkitException(EVENT_NOT_SUPPORTED);
	}

	return localEvent;
}
/*
*@funtion      :Toolkit_processPublicFileEvent_2para
*
*@description   :公共函数，用于处理	Toolkit_registerFileEvent_2para 和Toolkit_deregisterFileEvent_2para
*
*@param         :fileEvent
*@param         :path
*@param         :offset
*@param         :pathlgth
*@param         :mode : 0x00 register /0x01:deregister
*
*@return        :void
*
*@author        :xuyz
*
*@history       :V1.0 :create by xuyz@20131120
*
*@others
*/
void UICCToolkit_processPublicFileEvent_2para(BYTE mode)
{
	BYTE idata localEvent;
	BYTE xdata pathData[0x0A];  //目前卡片最多支持3级DF，因此路径的长度最大为8字节
	
	jbyte pathLgth = PopOperandStackShort();
    
	jbyte offset = PopOperandStackShort();
    
	jref path = PopOperandStackShort();
    
	jshort fileEvent = PopOperandStackShort();

	localEvent = UICCisFileEventAvaible(fileEvent);

	check_bounds_interval_buffer(path,offset,pathLgth);

	ecpMem_Set(pathData,0xFF,sizeof(pathData));

	CopyArrayDataToDest((BYTE *)&pathData[0],path,offset,pathLgth);

   	if(mode == 0x00)
	{
		registerFileEvent(localEvent, pathData);	
	}
	else
	{
		deregisterFileEvent(localEvent,pathData);	
	}
}
/*
*@funtion      :Toolkit_processPublicFileEvent_7para
*
*@description  : 处理Toolkit_registerFileEvent_7pata和Toolkit_deregisterFileEvent_7para的公共函数
*
*@param
*@param         : mode : 0x0 register/ 0x01:deregister
*
*@return        :
*
*@author        :xuyz
*
*@history       :V1.0:create by xuyz@20131120
*
*@others   
*/
void UICCToolkit_processPublicFileEvent_7para(BYTE mode)
{
	BYTE idata localEvent,i,j;
    BYTE  idata offset;  //缓存filelist
	WORD idata adfName;
	UWORD xdata fileAddr;
	BYTE xdata tempfilelist[10];
	
	jbyte bLength2 = PopOperandStackShort();
    
	jshort sOffset2 = PopOperandStackShort();
    
	jref baADFAid = PopOperandStackShort();
    
	jshort sLength1 = PopOperandStackShort();
    
	jshort sOffset1 =PopOperandStackShort(); 
    
	jref baFileList = PopOperandStackShort();
    
	jshort fileEvent = PopOperandStackShort();

	localEvent = UICCisFileEventAvaible(fileEvent);

	check_bounds_interval_buffer(baFileList,sOffset1,sLength1);

	adfName = FALSE;

	if(baADFAid != ADDR_IS_NULL)	    //当前存在有效的ADF
	{
		check_bounds_interval_buffer(baADFAid,sOffset2,bLength2);
				
	    if(AIDLenInvalid(bLength2))
		{
			UICCThrowSystemException(ILLEGAL_VALUE);//抛出异常 ILLEGAL_VALUE
		}

		CopyArrayDataToDest((BYTE *)&tempBuf[50],baADFAid,sOffset2,bLength2);

		fileAddr = SelectFileByName(E_FILE_START,&tempBuf[50],bLength2);
		
		if(fileAddr == FALSE)
		{
			UICCThrowSystemException(ILLEGAL_VALUE);//抛出异常 ILLEGAL_VALUE			
		}

		ReadEeprom((BYTE idata *)&adfName,fileAddr,sizeof(WORD));	
	}
	
	if((sLength1<3)||((sLength1&0x01)==0))
	{
		UICCThrowSystemException(ILLEGAL_VALUE);//抛出异常 ILLEGAL_VALUE
	}

	CopyArrayDataToDest((BYTE *)&tempBuf[0],baFileList,sOffset1,sLength1);

	for(i = 0,offset = 1; i < tempBuf[0]; i++ )
	{
		ecpMem_Set(tempfilelist,0xFF,sizeof(tempfilelist));

		tempfilelist[0] = 0x3F;

		tempfilelist[1] = 0x00;

		j = 2;					 //tempfilelist偏移

		offset += 0x02;

		if((tempBuf[offset] == 0x7F)&&(tempBuf[offset+1] == 0xFF))	
		{
			if(adfName != FALSE)
			{
				tempBuf[offset] = (adfName>>8)&0xFF;
                
				tempBuf[offset+1] =  adfName&0xFF;
			}
			else
			{
				UICCThrowSystemException(ILLEGAL_VALUE);//抛出异常 ILLEGAL_VALUE
			}
		}

		while(((tempBuf[offset] != 0x3F)||(tempBuf[offset+1] != 0x00))&&(offset<= (sLength1-1)))			//查询下一个
		{
			ecpMem_Copy(&tempfilelist[j],&tempBuf[offset],2);

			offset += 2;

			j += 2;
		}

		if(mode == 0x00)
		{
			registerFileEvent(localEvent, tempfilelist);	
		}
		else
		{
			deregisterFileEvent(localEvent,tempfilelist);	
		}	
	}	
}
/**
 * @function   :Toolkit_registerFileEvent_2para
 *
 * @description:Register a ToolkitRegistry object to the <code>EVENT_EXTERNAL_FILE_UPDATE</code> for a file.
 *
 * @param      :fileEvent   Indicates the File event to be registered
 * @param      :aFileView   The FileView object's current file indicates the file to be monitored. If the current file is a dedicated file, the Applet shall be triggered when an elementary file that is an immediate child of the DF is updated. A later change in the FileView shall not modify the registration.
 *
 * @exception  :NullPointerException   if <code> aFileView </code> is <code>null</code>
 * @exception  :TransactionException   if the operation would cause the commit capacity to be exceeded
 * @exception  :ToolkitException       with the following reason code:<ul>
 *              <li><code>EVENT_NOT_ALLOWED</code> if the event is one of the following events: EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND
 *              <li><code>EVENT_NOT_SUPPORTED</code> if the event is not one of the following event: EVENT_EXTERNAL_FILE_UPDATE, EVENT_REMOTE_FILE_UPDATE.</ul>
 * @exception  :UICCException                   in case of error<ul>
 *              <li><code> COMMAND_NOT_ALLOWED </code> if the referenced file is terminated
 */

void UICCToolkit_registerFileEvent_2para(void)
{	
	UICCToolkit_processPublicFileEvent_2para(0x00);
}
/**
*@function   :Toolkit_registerFileEvent_7para
*
*@description:Register a ToolkitRegistry object to the EVENT_EXTERNAL_FILE_UPDATE for a file list.

*@param       :fileEvent Indicates the File event to be registered
*@param       :baFileList		array containing the file list. The baFileList shall be coded as the value part of the COMPREHENSION-TLV File List defined in TS 102 223
*              If the path provided indicates a dedicated file (DF), the Applet shall be triggered when an elementary file that is an immediate child of the DF is updated.
*@param       :sOffset1          offset in the baFileList   buffer for event registration
*@param       :sLength1        length in the baFileList   buffer for event registration
*@param       :baADFAid	array containing the AID of the ADF. The baADFAid shall be coded as the value part of the COMPREHENSION-TLV AID defined in TS 102 223. If baADFAid  is nullnull, it indicates that the file is located under the MF and not located under an ADF .
*@param       :sOffset2          offset in the baADFAid  buffer for event registration
*@param       :bLength2        length in the baADFAid  buffer for event registration
*
*@exception   :NullPointerException            if  baFileList  is null
*@exception   :ArrayIndexOutOfBoundsException  if sOffset or sLength or both would cause access outside array bounds
*@exception   : TransactionException   if the operation would cause the commit capacity to be exceeded
*@exception   :SystemException with the following reason code:
* 			  <li>SystemException.ILLEGAL_VALUE if bLength2 is not in the range of 5 - 16 bytes. 
*@exception   :ToolkitException with the following reason code:
*              <li>EVENT_NOT_ALLOWED if the event is one of the following events: EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND
*             <li>EVENT_NOT_SUPPORTED if the event is not one of the following event: EVENT_EXTERNAL_FILE_UPDATE, EVENT_REMOTE_FILE_UPDATE.
* @exception  UICCException   in case of error
*          <li> COMMAND_NOT_ALLOWED  if the referenced file is terminated
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131101
*
*@others	 :1:新建文件2800/6F83用于处理该事件
              2:测试规范TS102268上 仅注明该API函数支持EVENT_EXTERNAL_FILE_UPDATE事件
			  3:注意卡片上的文件ID不允许为0xFFFF，因为FFFF在6F83中标示无效ID和空块号
*/
void UICCToolkit_registerFileEvent_7para(void) 
{
   UICCToolkit_processPublicFileEvent_7para(0x00);
}
/**
*@function    : Toolkit_deregisterFileEvent_2para
*
*@description :Deregister a ToolkitRegistry object from the EVENT_EXTERNAL_FILE_UPDATE for a file.
*
*@param       :fileEvent Indicates the File event to be deregistered
*@param       :aFileView  The FileView object's current file indicates the file that is no longer monitored. 
*             If the current file is a dedicated file the deregistration shall not affect the monitoring of an elementary file within the dedicated file that was individually registered.
*             If the current file is an elementary file the deregistration will not affect the monitoring of the parent dedicated file that was individually registered.
*
*
*@exception   :NullPointerException            if aFileView is null
*@exception   :TransactionException   if the operation would cause the commit capacity to be exceeded
*@exception   :ToolkitException with the following reason code:
*             <li>EVENT_NOT_ALLOWED if the event is one of the following events: EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND
*             <li>EVENT_NOT_SUPPORTED if the event is not one of the following event: EVENT_EXTERNAL_FILE_UPDATE, EVENT_REMOTE_FILE_UPDATE.
*@exception  :UICCException                   in case of error
*             <li> COMMAND_NOT_ALLOWED  if the referenced file is terminated
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131119
*
*@others     :1:注意卡片上的文件ID不允许为0xFFFF，因为FFFF在6F83中标示无效ID和空块号
*/
void UICCToolkit_deregisterFileEvent_2para(void)
{
	UICCToolkit_processPublicFileEvent_2para(0x01);
}
/**
*@function    : Toolkit_deregisterFileEvent_7para
*
*@description :Deregister a ToolkitRegistry object from the EVENT_EXTERNAL_FILE_UPDATE for a file list.
*
*@param       :fileEvent Indicates the File event to be deregistered
*@param       :baFileList		array containing the file list. The  baFileList  shall be coded as the value part of the COMPREHENSION-TLV File List defined in TS 102 223
*               If a file in baFileList is a dedicated file the deregistration shall not affect the monitoring of an elementary file within the dedicated file that was individually registered.
*               If a file in baFileList is an elementary file the deregistration will not affect the monitoring of the parent dedicated file that was individually registered.
*@param      :sOffset1          offset in the  baFileList   buffer for event registration
*@param      :sLength1        length in the  baFileList   buffer for event registration
*@param      :baADFAid	array containing the AID of the ADF. The baADFAid shall be coded as the value part of the COMPREHENSION-TLV AID defined in TS 102 223. If baADFAid is null, it indicates that the file is located under the MF and not located under an ADF .
*@param      :sOffset2          offset in the baADFAid  buffer for event registration
*@param      :bLength2        length in the baADFAid  buffer for event registration
*
*@exception  :NullPointerException            if  baFileList  is null
*@exception  :ArrayIndexOutOfBoundsException  if sOffset or sLength or both would cause access outside array bounds
*@exception  :TransactionException   if the operation would cause the commit capacity to be exceeded
*@exception   :SystemException with the following reason codes:
*             <li>SystemException.ILLEGAL_VALUE if bLength2 is not in the range of 5 閿燂拷16 bytes. 
*@exception  :ToolkitException with the following reason code:
*            <li>EVENT_NOT_ALLOWED if the event is one of the following events: EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND
*            <li>EVENT_NOT_SUPPORTED if the event is not one of the following event: EVENT_EXTERNAL_FILE_UPDATE, EVENT_REMOTE_FILE_UPDATE.	
*@exception  :UICCException                   in case of error
*             <li> COMMAND_NOT_ALLOWED  if the referenced file is terminated
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131101
*
*@others     :
*/
void UICCToolkit_deregisterFileEvent_7para(void) 
{
   UICCToolkit_processPublicFileEvent_7para(0x01);
}

/**
*@function    :Toolkit_setMenuEntryTextAttribute
*
*@See         :uicc.framework.UICCToolkitRegistryImpl.setMenuEntryTextAttribute(byte, byte[], short, short)
*
*@description :Sets the text attribute of a menu entry.
*              The text attribute provided will set the entry in the Text Attribute list  of the Item text attribute list Comprehension TLV for the menu entry referenced by the parameter id. After the invocation of this method, during the current card session, the CAT Runtime Environment shall dynamically update the menu stored in the terminal.
*
*@param       :id                                  the menu entry identifier supplied by the initMenuEntry() method
*@param       :textAttribute                           a reference on a byte array, containing the menu text attribute
*@param       :offset                              the position of the menu entry text attribute in the buffer
*@param       :length                              the length of the menu entry text attribute in the buffer
*@exception   :NullPointerException            if  textAttribute  is null
*@exception   :ArrayIndexOutOfBoundsException  if offset or length or both would cause access outside array bounds
*@exception    :ToolkitException                with the following reason codes: 
*              <li>MENU_ENTRY_NOT_FOUND if the menu entry does not exist for this applet
*	           <li>BAD_INPUT_PARAMETER if the length is different from 4
*@exception   : TransactionException            if the operation would cause the commit capacity to be exceeded
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20131101
*
*@others       :
*/
void UICCToolkit_setMenuEntryTextAttribute(void)
{
	BYTE * textAttributeData;
	BYTE idata  menuRec,result;
	UWORD xdata tookitMenuEntryAddr;

	jshort length = PopOperandStackShort();
    
	jshort offset = PopOperandStackShort();
    
	jref textAttribute = PopOperandStackShort();
    
	jbyte id = PopOperandStackShort();

	textAttributeData = (BYTE *)GetValidArrayDataAddr(textAttribute,offset,length); 
	
	if(length != 0x04)
	{
		UICCThrowToolkitException(BAD_INPUT_PARAMETER);
		return;
	}

	menuRec = ToolKitFindAppRecordNumById(id);

	if(menuRec == FALSE)
	{
	   UICCThrowToolkitException(MENU_ENTRY_NOT_FOUND);
	   return;
	}
	
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	VMReadBuf(tempBuf,(UWORD)textAttributeData,0x04);    //textAttributeData数据是由应用开发者控制，可能为RAM，可能为E2
		
	result = WriteLFBuf(tookitMenuEntryAddr,menuRec,TextAttribute_OFFSET,tempBuf, 0x04);

	if(result == FALSE)
	{
		//抛出异常 	TransactionException
	}

	SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);				  //add by zzj @20131129
} 
#endif
