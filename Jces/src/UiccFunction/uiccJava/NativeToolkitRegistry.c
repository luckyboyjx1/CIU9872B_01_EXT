 /*
 *@HISTORY     :V1.0 :create
 *
 *             :V1.1 :删除文件中的无效代码 @20131015 by xuyz
 */

#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "SimApi.h"
#include "fileAction.h"
#include "ToolkitRFM.h"
#include "arithmatic.h"
#include "gsmpublic.h"
//#include "Interpreter.h"
#include "uicc_interface.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
//#include "ChannelManagement.h"

#if UICC__FUNCTION__SIMAPI
/*
*@description:判断当前事件卡片是否支持
*
*@param      :event
*
*@return    :event or EVENT_ERROR
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
BYTE ToolkitIsEventSupported(jbyte event)
{
	BYTE idata eventBit;
	
	eventBit = EVENT_ERROR;
	
	if(IF_SIMAPIPrivateEventSupport(event,&eventBit) == TRUE)
	{
		return eventBit;
	}
	
	
	if(event == EVENT_FIRST_COMMAND_AFTER_SELECT)
	{
		return EVENT_FIRST_COMMAND_AFTER_SELECT_bit;
	}

	if(event == EVENT_UNRECOGNIZED_ENVELOPE)			 
	{
		return  EVENT_UNRECOGNIZED_ENVELOPE_bit;
	}

	if(event <= 0x00)  //EVENT_UNRECOGNIZED_ENVELOPE 为负
	{
	 	return EVENT_ERROR;
	}
	
	if(event > EVENT_FORMATTED_SMS_CB_bit)
	{	
		event = EVENT_ERROR;
	}
	
	return event;
}
/*
*@description:判断当前事件是否为  EVENT_MENU_SELECTION_bit 	 EVENT_MENU_SELECTION_HELP_REQUEST_bit		 EVENT_TIMER_EXPIRATION_bit		  EVENT_STATUS_COMMAND_bit
*
*@param      :event
*
*@return     :EVENT_NOT_ALLOWED
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void ToolkitIsEventAllowed(BYTE event)
{
	if((event == EVENT_MENU_SELECTION_bit)||(event == EVENT_MENU_SELECTION_HELP_REQUEST_bit)||(event == EVENT_TIMER_EXPIRATION_bit)||(event == EVENT_STATUS_COMMAND_bit))
	{ 	
		ThrowToolkitException(EVENT_NOT_ALLOWED_SIM); 	//todo :抛出异常  EVENT_NOT_ALLOWED
	}
}
/*
*@description:Sets an event in the Toolkit Registry entry of the applet. No exception shall be thrown if the applet registers more than once to the same event
*
*@See         :  sim.toolkit.ToolkitRegistry.setEvent(byte)
*
*@param       :	 event - value of the new event to register (between -128 and 127)
*
*@return      : void
*
*@exception   :  EVENT_NOT_SUPPORTED if the event is not supported 
                 EVENT_ALREADY_REGISTERED if the event has already been registered (for limited event like Call Control) 
                 EVENT_NOT_ALLOWED if event is EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND 
                 TAR_NOT_DEFINED if event is FORMATTED_SMS_PP_ENV, FORMATTED_SMS_PP_UPD or FORMATTED_SMS_CB_ENV and the applet has no TAR defined 
				 javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded.
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void Toolkit_setEvent(void)
{
	BYTE idata recNum,event;
	BYTE  xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	jbyte idata eventNew;
	jshort idata standardEvent;
	WORD idata topEvent;
	
	jshort apiMode =  PopOperandStackShort();

	if(apiMode == SIMAPI_Mode)
	{
		eventNew =  PopOperandStackShort();
		
		event = ToolkitIsEventSupported(eventNew);

		if(event == EVENT_ERROR)
		{	
			ThrowToolkitException(EVENT_NOT_SUPPORTED_SIM);	
		}

		ToolkitIsEventAllowed(event); 
		
		if(ToolkitEventRegistered(event))
		{
			ThrowToolkitException(EVENT_ALREADY_REGISTERED_SIM); 
		}
	}
	else
	{
		standardEvent = PopOperandStackShort();
		
		if(standardEvent == EVENT_MEMORY_FAILURE)
		{		
			UICCThrowToolkitException(EVENT_NOT_SUPPORTED);	  		
		}

		event = UICCToolkit_isStandardEventAvaible(standardEvent);

	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr  = GetCurrentToolkitEntryArr();

	if(toolkitAppAddr == ADDR_IS_NULL)
	{
		return ;
	}
#else
	recNum = GetCurrentToolkitEntryAppNum();
	
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return;
	}
#endif
	if(apiMode == SIMAPI_Mode)
	{
		if((event == EVENT_FORMATTED_SMS_PP_ENV_bit)||(event == EVENT_FORMATTED_SMS_PP_UPD_bit) ||(event == EVENT_FORMATTED_SMS_CB_bit))
		{
			ReadLFBufForJavaAccess(toolkitAppAddr, recNum , TARLength_OFFSET,&tmp[0],sizeof(BYTE));

			if(tmp[0] == 0x00)  //对于这几个event，如果不存在tar值，则AID 的长度必须大于15字节，以便于根据tar值来触发应用  //GP2.2.1_UICC_ComplianceTests(移动版)-20141112/测试脚本/uicc_configuration_additional_test_工程集.uce/java应用安装时的MSL设置测试/mslTestApp_2_1
			{
					ThrowToolkitException(TAR_NOT_DEFINED_SIM); //在目前的代码方案中，当tar不存在，AID 长度大于15时，已经自动将AID的13-15字节注册成tar值，因此此处只需要判断tar是否存在即可
			}
		}
	}

	ReadLFBufForJavaAccess(toolkitAppAddr, recNum,EventList_OFFSET,&tmp[0],sizeof(tmp));	   
	
	ToolkitSetEventBit(tmp,event);
	
	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,EventList_Length);
	
	ReadLFBufForJavaAccess(toolkitAppAddr, recNum,AppletStatus_OFFSET,&tmp[0],sizeof(BYTE));

	if(tmp[0]!=APPLET_UNLOCKED_SELECTABLE)
	{
		return;
	}

	if(apiMode == SIMAPI_Mode)
	{
		topEvent = EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit;
	}
	else
	{
		topEvent = EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit;
	}
	
	if((event >= EVENT_EVENT_DOWNLOAD_MT_CALL_bit)&&(event <= topEvent)&&(event != EVENT_STATUS_COMMAND_bit))	//todo :注意，setevent后，卡端是否要发送SETUP EVENT LIST主动式命令 	
	{	
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
	}
}

/*
*@description: Sets an event list in the Toolkit Registry entry of the applet.
*
*@See        :  sim.toolkit.ToolkitRegistry.setEventList(byte[], short, short)
*
*@param      :eventList - buffer containing the list of the new events to register
              offset - offset in the eventlist buffer for event registration
              length - length in the eventlist buffer for event registration 
*
*@return     : void
*
*@exception	 :	java.lang.NullPointerException - if eventlist is null 
                java.lang.ArrayIndexOutOfBoundsException - if setEventList would cause access of data outside array bounds. 
                ToolkitException - with the following reason codes: 
                    EVENT_NOT_SUPPORTED if one event is not supported 
                    EVENT_ALREADY_REGISTERED if one event has already been registered (for limited event like Call Control) 
                    EVENT_NOT_ALLOWED if eventList contains EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND 
                    TAR_NOT_DEFINED if eventList contains FORMATTED_SMS_PP_ENV, FORMATTED_SMS_PP_UPD or FORMATTED_SMS_CB_ENV and the applet has no TAR defined 
                javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded.
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void Toolkit_setEventList(void)
{
	BYTE idata recNum,i,event,appState;
	BYTE xdata tmp[EventList_Length];
	UWORD xdata eventListAddr;

	jbyte idata eventNew;
	
	jref  idata eventList; 
	jshort idata offset; 
	jshort idata length;
	UWORD xdata toolkitAppAddr;
	BYTE idata tarLen;
	
	length = PopOperandStackShort();
	offset = PopOperandStackShort();
	eventList = PopOperandStackShort();

	eventListAddr = GetValidArrayDataAddr(eventList, offset, length);//GetArrayBufferAddress(eventList);

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

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,&tmp[0],EventList_Length);

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,AppletStatus_OFFSET,&appState,sizeof(BYTE));
					
	for(i=0 ; i< length; i++)
	{
		eventNew = VMReadByte(eventListAddr+i);

		event = ToolkitIsEventSupported(eventNew);

		if(event == EVENT_ERROR)
		{
			ThrowToolkitException(EVENT_NOT_SUPPORTED_SIM);
		}

		ToolkitIsEventAllowed(event);

		if(ToolkitEventRegistered(event))
		{
			ThrowToolkitException(EVENT_ALREADY_REGISTERED_SIM);
		}

		if((event == EVENT_FORMATTED_SMS_PP_ENV_bit)||(event == EVENT_FORMATTED_SMS_PP_UPD_bit) ||(event == EVENT_FORMATTED_SMS_CB_bit))
		{
			ReadLFBufForJavaAccess(toolkitAppAddr, recNum , TARLength_OFFSET,&tarLen,sizeof(BYTE));

			if(tarLen == 0x00)  //对于这几个event，如果不存在tar值，则AID 的长度必须大于15字节，以便于根据tar值来触发应用  //GP2.2.1_UICC_ComplianceTests(移动版)-20141112/测试脚本/uicc_configuration_additional_test_工程集.uce/java应用安装时的MSL设置测试/mslTestApp_2_1
			{
					ThrowToolkitException(TAR_NOT_DEFINED_SIM);		//目前在安装的时候如果安装参数中不存在tar值，则已经把AID的13-15字节转换成tar存储在tar区域，因此此处只用判断tar值是否存在即可
			}
		}

		if((event >= EVENT_EVENT_DOWNLOAD_MT_CALL_bit)&&(event <= EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit)&&(event != EVENT_STATUS_COMMAND_bit))
		{
			if(appState==APPLET_UNLOCKED_SELECTABLE)
			{
				SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
			}
		}

		ToolkitSetEventBit(&tmp[0],event);
	}
	
	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,EventList_Length);
}
/**
 * Clears an event in the Toolkit Registry entry of the applet.
 *
 * @See        :  sim.toolkit.ToolkitRegistry.clearEvent(byte)
 *
 * @param event the value of the event to unregister (between -128 and 127)
 * @exception ToolkitException with the following reason codes: <ul>
 *	    <li>EVENT_NOT_ALLOWED if event is EVENT_MENU_SELECTION, EVENT_MENU_SELECTION_HELP_REQUEST, EVENT_TIMER_EXPIRATION, EVENT_STATUS_COMMAND</ul>
 * @exception TransactionException if the operation would cause the commit capacity to be exceeded. 
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void Toolkit_clearEvent(void)
{
	BYTE idata event,recNum;
	jbyte idata eventNew;
	jshort idata uiccEventNew;
	BYTE xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
	jshort idata apiMode;
	WORD idata topEvent;
	
	apiMode = PopOperandStackShort();
	
	if(apiMode == SIMAPI_Mode)
	{
		eventNew = PopOperandStackShort();
		
		event = ToolkitIsEventSupported(eventNew);

		if(event == EVENT_ERROR)
		{	
			ThrowToolkitException(EVENT_NOT_SUPPORTED_SIM); 		//TODO :抛出异常   EVENT_NOT_SUPPORTED
		}
		
		ToolkitIsEventAllowed(event);
	}
	else
	{
		uiccEventNew = PopOperandStackShort();
		
		event = UICCToolkit_transferStandardToLocalEventType(uiccEventNew);

	    UICCToolkit_isEventAllowed(event,FALSE);
	}
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
			   			
	ReadLFBufForJavaAccess(toolkitAppAddr, recNum,EventList_OFFSET,&tmp[0],EventList_Length);	
	
	ToolkitClearEventBit(&tmp[0],event);
	
	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,EventList_Length);	

	if(apiMode == SIMAPI_Mode)
	{
		topEvent = EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit;
	}
	else
	{
		topEvent = EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit;
	}
	
	if((event >= EVENT_EVENT_DOWNLOAD_MT_CALL_bit)&&(event <= topEvent)&&(event != EVENT_STATUS_COMMAND_bit)) 	//todo :注意，setevent后，卡端是否要发送SETUP EVENT LIST主动式命令
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
	}
}
/*
*@description: Allows to know if an event is set in the Toolkit Registry entry of the applet
*
* @See       :  sim.toolkit.ToolkitRegistry.isEventSet(byte)
*
*@param      :event - the value of the event (between -128 and 127)
*
*@return     :true if the event is set in the Toolkit Registry entry of the applet, false otherwise
*
*@exception  :
*
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void Toolkit_isEventSet(void )
{
	BYTE xdata tmp[EventList_Length];
	BYTE idata event,recNum;	
	jshort idata eventNew;
	UWORD xdata toolkitAppAddr;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	eventNew = PopOperandStackShort();

	if(apiMode == SIMAPI_Mode)
	{
		event = ToolkitIsEventSupported(eventNew);

		if(event == EVENT_ERROR)	//不支持，返回false
		{
			PushOperandStackShort(FALSE);
			return;
		}
	}
	else
	{
		event = UICCToolkit_transferStandardToLocalEventType(eventNew);
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetCurrentToolkitEntryArr();
#else
	recNum = GetCurrentToolkitEntryAppNum();
	
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{
		PushOperandStackShort(FALSE);
		return;
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr, recNum,EventList_OFFSET,&tmp[0],EventList_Length);
	
	event = ToolkitGetEventBit(&tmp[0], event);	  //return TRUE or FALSE
	
	PushOperandStackShort((jboolean)event);
}
/*
*@description:	Disables a menu entry. This method doesn't modify the registration state to the EVENT_MENU_SELECTION and EVENT_MENU_SELECTION_HELP_REQUEST.
* 				After invocation of this method, during the current card session, the SIM Toolkit Framework shall dynamically update the menu stored in the ME.
*
*@See        :  sim.toolkit.ToolkitRegistry.disableMenuEntry(byte)
*
*@param      :	id - the menu entry identifier supplied by the initMenuEntry() method 
*
*@return     :	 void
*
*@exception  :ToolkitException - with the following reason codes: 
                 ENTRY_NOT_FOUND if the menu entry does not exist for this applet
              javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void Toolkit_disableMenuEntry(void)
{
	setMenuStatusPublic(Menu_UNLOCKED|Menu_DISABLE|Menu_SELECTABLE);
}
/*
*@description: Enables a menu entry. This method doesn't modify the registration state to the EVENT_MENU_SELECTION and EVENT_MENU_SELECTION_HELP_REQUEST. After invocation of this method, during the current card session, the SIM Toolkit Framework shall dynamically update the menu stored in the ME.
*
*@See        : sim.toolkit.ToolkitRegistry.enableMenuEntry(byte)
*
*@param      :id - the menu entry identifier supplied by the initMenuEntry() method 
*
*@return     : void
*
*@exception  :ToolkitException - with the following reason codes: 
                  MENU_ENTRY_NOT_FOUND if the menu entry does not exist for this applet
              javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded.
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void Toolkit_enableMenuEntry(void)
{
	setMenuStatusPublic(Menu_UNLOCKED|Menu_ENABLE|Menu_SELECTABLE);
}

/*
@description: Initialises the next menu entry allocated at loading. The default state of the menu entry is 'enabled'. The value of the helpSupported boolean parameter defines the registration status of the applet to the event EVENT_MENU_SELECTION_HELP_REQUEST. The applet is registered to the EVENT_MENU_SELECTION. The icon identifier provided will be added to the icon identifier list of the item icon identifier list Simple TLV if all the applets registered to the EVENT_MENU_SELECTION provide it. The Icon list qualifier transmitted to the ME will be 'icon is not self explanatory' if one of the applet registered prefers this qualifier. This method shall be called by the applet in the same order than the order of the item parameters defined at the applet loading if the applet has several menu entries. The applet shall initialise all its loaded menu entries during its installation. 
*
*@See        : sim.toolkit.ToolkitRegistry.initMenuEntry(byte[], short, short, byte, boolean, byte, short)
*
*@param     : menuEntry - a reference on a byte array, containing the menu entry string
              offset - offset of the menu entry string in the buffer
              length - length of the menu entry string
              nextAction - a byte coding the next action indicator for the menu entry (or 0)
              helpSupported - equals true if help is available for the menu entry
              iconQualifier - the preferred value for the icon list qualifier
              iconIdentifier - the icon identifier for the menu entry (0 means no icon) 
*
*@return    :the identifier attached to the initialised menu entry 
*
*@exception : java.lang.NullPointerException - if menuEntry is null 
              java.lang.ArrayIndexOutOfBoundsException - if initMenuEntry would cause access of data outside array bounds. 
              ToolkitException - with the following reason codes: 
                  REGISTRY_ERROR if the menu entry cannot be initialised (eg no more item data in applet loading parameter) 
                  ALLOWED_LENGTH_EXCEEDED if the menu entry string is bigger than the alloacted space
              javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded
*
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void Toolkit_initMenuEntry(void)
{
	InitMenuEntryPublic();
}
/*
@description:Changes the value of a menu entry. The default state of the changed menu entry is 'enabled'. The value of the helpSupported boolean parameter defines the registration status of the EVENT_MENU_SELECTION_HELP_REQUEST event. The icon identifier provided will be added to the icon identifier list of the item icon identifier list Simple TLV if all the applets registered to the EVENT_MENU_SELECTION provide it. The Icon list qualifier transmitted to the ME will be 'icon is not self explanatory' if one of the applet registered prefers this qualifier. After the invocation of this method, during the current card session, the SIM Toolkit Framework shall dynamically update the menu stored in the ME
*
*@See       : sim.toolkit.ToolkitRegistry.changeMenuEntry(byte, byte[], short, short, byte, boolean, byte, short)
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
*/
void Toolkit_changeMenuEntry(void)
{
	ChangeMenuEntryPublic();
}
/*
@Description: Asks the Toolkit framework to allocate a Timer that the applet can manage. 
* By calling this method the applet is registered to the EVENT_TIMER_EXPIRATION of the allocated timer. 
* The timer is allocated by the applet until it explicitly releases it. So it can then issue the Timer 
* Management proactive command to start, stop or get the value of its allocated timer.
*
*@See       : sim.toolkit.ToolkitRegistry.allocateTimer()
*
*@param     : void
*
*@return    :the identifier of the Timer allocated to the applet 
*
*@exception :ToolkitException - with the following reason codes: 
                  NO_TIMER_AVAILABLE if all the timers are allocated or the maximum number of timers have been allocated to this applet
             javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void Toolkit_allocateTimer(void)
{
	AllocateTimerPublic();
}
/*
*@description: Release a Timer that has been allocated to the calling applet. The applet is deregistered of the EVENT_TIMER_EXPIRATION for the indicated Timer Identifier. 
*
*@See        : sim.toolkit.ToolkitRegistry.releaseTimer(byte)
*
*@param      :timerIdentifier - the identifier of the Timer to be released 
*
*@return     : void
*
*@exception  :ToolkitException - with the following reason codes: 
                  INVALID_TIMER_ID if the timerIdentifierd is not allocated to this applet.
               javacard.framework.TransactionException - - if the operation would cause the commit capacity to be exceeded.
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void Toolkit_releaseTimer(void)
{
	ReleaseTimerPublic();
}
/*
*@description:	Requests a duration for the EVENT_STATUS_COMMAND event of the registering toolkit applet. Due to different duration requested by other toolkit applets or due to restriction of the ME, the SIM Toolkit Framework may adjust another duration. This method can be used at every time to request a new duration
*
*@See        : sim.toolkit.ToolkitRegistry.requestPollInterval(short)
*
*@param      : duration - specifies the number of seconds requested for proactive polling. The maximum value of duration is 15300 (255 minutes). If duration is equal to POLL_NO_DURATION, the calling applet deregisters from EVENT_STATUS_COMMAND, and the SIM Toolkit Framework may issue the POLLING OFF proactive command. If duration is equal to POLL_SYSTEM_DURATION, the calling applet registers to the EVENT_STATUS_COMMAND and let the SIM Toolkit Framework define the duration
*
*@return     : void
*
*@exception  : ToolkitException - with the following reason codes: 
                   REGISTRY_ERROR if duration is greater than the maximum value.
               javacard.framework.TransactionException - if the operation would cause the commit capacity to be exceeded.
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
void Toolkit_requestPollInterval(void)
{
	RequestPollIntervalPublic();
}
/*
*@description:	Returns the number of seconds of the adjusted duration for the calling toolkit applet. If the returned duration is equal to POLL_NO_DURATION, the toolkit applet is not registered to EVENT_STATUS_COMMAND event. The returned duration may : - be a multiple of the real adjusted poll interval time at the ME. - differ from the requested duration due to request of other toolkit applets or due to restrictions of the current ME. - be changed during the card session due requests from other toolkit applets. - be wrong due to direct generation of the proactive command POLL INTERVAL or POLLING OFF. - not correspond to the ellasped time due to additional STATUS commands send by the ME. 
*
*@See        : sim.toolkit.ToolkitRegistry.getPollInterval().
*
*@param      :   void
*
*@return     : the number of seconds of the adjusted duration for the applet
*
*@exception  :
*
*@history    :
*/
void Toolkit_getPollInterval(void)
{	
	GetPollIntervalPublic();
}

#endif
