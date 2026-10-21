#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "fileAction.h"
#include "simapi.h"
#include "stkhead.h"
#include "ToolkitRFM.h"
#include "gsmpublic.h"
#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
//#include "ChannelManagement.h"

#if UICC__FUNCTION__SIMAPI
/*******************************************************************************
Function name:ToolKit_init
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.init方法的本地实现
	Returns:     void
	author: LJ / 2013.06.19
********************************************************************************/
void ToolKit_init(void)
{	
	jbyte idata commandType; 
	jbyte idata commandQlify; 
	jbyte idata desDevice;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	desDevice = PopOperandStackShort();
  	commandQlify = PopOperandStackShort();
	commandType = PopOperandStackShort();

	ClearHandlerCurrentTLV(Index_ProactiveHandler);

    checkStkBusy(apiMode);

	InitProactiveCommand(commandType,commandQlify,desDevice);

	ToolkitHandlerList[Index_ProactiveHandler].TLVListLength = ResData[2] ;
}
/*******************************************************************************
Function name:ToolKit_init
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.init方法的本地实现
	Returns:     void
	author: LJ / 2013.06.19
********************************************************************************/
void InitDisplayGetInkeyGetInput(BYTE mode)
{
	BYTE idata result;
	
	jbyte idata qualifier; 
	jbyte idata dcs; 
	jref idata arrayref; 
	jshort idata offset; 
	jshort idata length; 
	jshort idata minRespLength; 
	jshort idata maxRespLength;
	jshort idata apiMode;
	
	maxRespLength = 0;
	minRespLength = 0;
	
	apiMode = PopOperandStackShort();
	
	if(mode == CMD_GETINPUT)
	{
		maxRespLength = PopOperandStackShort();
		minRespLength = PopOperandStackShort();
	}
	
	length = PopOperandStackShort();
	offset = PopOperandStackShort(); 
	arrayref = PopOperandStackShort();
	dcs = PopOperandStackShort();
	qualifier = PopOperandStackShort(); 

	ClearHandlerCurrentTLV(Index_ProactiveHandler);

 	check_bounds_interval_buffer(arrayref,offset,length);

	checkStkBusy(apiMode);
	
	if(length > 240)// 规范11.14 - 6.4.1  测试用例 Api_2_Pah_Indt, SIM API时有没有这个判断？
	{
		UiccOrSimThrowToolkitException(HANDLER_OVERFLOW, apiMode);
		return;
	}

	if(mode == CMD_DISPLAYTEXT)
	{
		InitProactiveCommand(CMD_DISPLAYTEXT, qualifier, DEV_DISPLAY);
	}
	else
	{
		InitProactiveCommand(mode, qualifier, DEV_ME);
	}

	tempBuf[0] = 0x8D;

	if(length != 0x00)
	{	
		tempBuf[1] = 0x01;
	
		tempBuf[2] = dcs;

		if(length > 253)
		{
			UiccOrSimThrowToolkitException(HANDLER_OVERFLOW_SIM, apiMode);
			return ;
		}

		CopyArrayDataToDest(&tempBuf[3],arrayref,offset,length);
	
		tempBuf[1] += length;
	}
	else
	{
		tempBuf[1] = 0x00;
	}

	result = AddTLV(&ResData[1],&tempBuf[0]);

	if(	result == FALSE)	   	   //抛出 ToolkitException
	{	
		UiccOrSimThrowToolkitException(HANDLER_OVERFLOW_SIM, apiMode);
		return ;
	}
 
	if(mode == CMD_GETINPUT)
	{
		tempBuf[0] = 0x91;

		tempBuf[1] = 0x02;

		tempBuf[2] = (BYTE)minRespLength;

		tempBuf[3] = (BYTE)maxRespLength;

		result = AddTLV(&ResData[1],&tempBuf[0]);

		if(	result == FALSE)
		{	   
			UiccOrSimThrowToolkitException(HANDLER_OVERFLOW_SIM, apiMode);
			return;	
		}
	}
	
	ToolkitHandlerList[Index_ProactiveHandler].TLVListLength = ResData[2];	//添加8D的TLV和91的TLV后修改数据长度
}
/*******************************************************************************
Function name:ToolKit_initDisplayText
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.initDisplayText方法的本地实现
	Returns:     void
	author: LJ / 2013.06.19
********************************************************************************/
void ToolKit_initDisplayText(void)
{
	InitDisplayGetInkeyGetInput(CMD_DISPLAYTEXT);
}


/*******************************************************************************
Function name:ToolKit_initDisplayText
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.ToolKit_initGetInkey方法的本地实现
	Returns:     void
	author: LJ / 2013.06.19
********************************************************************************/
void ToolKit_initGetInkey(void)
{
	InitDisplayGetInkeyGetInput(CMD_GETINKEY);	
}

/*******************************************************************************
Function name:ToolKit_initDisplayText
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.initGetInput方法的本地实现
	Returns:     void
	author: LJ / 2013.06.19
********************************************************************************/
void ToolKit_initGetInput(void)
{
	InitDisplayGetInkeyGetInput(CMD_GETINPUT);	
}

/*******************************************************************************
Function name:ToolKit_initCloseChannel
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.initCloseChannel方法的本地实现
	Returns:     void
	author: LJ / 2013.06.19
********************************************************************************/
void ToolKit_initCloseChannel(void)
{
	jbyte idata bChannelIdentifier;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	bChannelIdentifier = PopOperandStackShort();
	      
	ClearHandlerCurrentTLV(Index_ProactiveHandler);

	checkStkBusy(apiMode);

	bChannelIdentifier = ((bChannelIdentifier&0x0F)|0x20);

	InitProactiveCommand(CMD_CLOSECHANNEL, 0x00, bChannelIdentifier);
	
	ToolkitHandlerList[Index_ProactiveHandler].TLVListLength = ResData[2];

}
/*******************************************************************************
Function name:Toolkit_Send
  	Parameters:event
	discript:  SIMAPI 函数ProactiveHandler.send方法的本地实现
	Returns:     void
	author: LJ / 2013.06.20
			V1.1 : V1.1 为支持send之后可以再次调用send函数，在SendProactiveCommand之前缓存ResData的值，之后恢复，以防止在SendProactiveCommand中破坏ResData的值@20170120 by xuyz
			V1.2 修改函数在G_PostTag == FALSE时备份数据时的ResData错误，因为可能出现Init后，appendTlV ，导致主动式命令的长度并不是ResData的长度@20180903 by xuyz
********************************************************************************/
void Toolkit_Send(void)
{
   BYTE  idata result;
   BYTE  idata tmp,channelId;
   jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
   channelId=0;

    if(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_FORMATTED_SMS_PP_ENV)
	{
		if(G_PostTag == FALSE)  //在send之后EnvelopResposne handler将会失效，因此要先判断formatted_PP数据是否已发送POR
		{
			G_PostTag = TRUE;  //The EnvelopeResponseHandler content must be posted before the first invocation of a ProactiveHandler.send method or before the termination of the processToolkit, so that the GSM applet can offer these data to the ME (eg 9Fxx/9Exx/91xx).
			
			ResData[2] =((ToolkitHandlerList[Index_ProactiveHandler].TLVListLength) >= 0xFD)?0xFD : (ToolkitHandlerList[Index_ProactiveHandler].TLVListLength);//主动式命令支持长度为256，此时发9100
			
			WriteVariable(Tag_BakPostData,&ResData[2]);

			Toolkit_ResponsePack(POR_OK,PORMODE_AUTO_EVERESPONSE,ToolkitHandlerList[Index_EnveResHandler].TLVListLength, 0x00);	//modify by jiangqingbin-20220303, 增加一个传入参数，表示http短信触发结果，不再使用原来的复用SW1，SW2的方式 

			ecpMem_Set(ResData,0xD0,2);
			
			ResData[2] = ReadVarible(Tag_BakPostData,&ResData[3]);
			
			ClearVariable(Tag_BakPostData);
		}
	}    // SIM API :fwk_fws_ouda


   if(ToolkitHandlerList[Index_ProactiveHandler].TLVListLength > ToolkitHandlerList[Index_ProactiveHandler].HandlerBufferSize )
   {
//	   ThrowAPDUException_Bad_Length();	//ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION,  //抛出javacard.framework.APDUException       
   }
 
   if(TransactionGetDepth() == TRUE)		     //ts43.019 A pending toolkit applet transaction at the method send() invocation is aborted.
   {
   	   TransactionAbort();
   }
   
   if(G_STKBusy != FALSE)  
   {
	   UiccOrSimThrowToolkitException(COMMAND_NOT_ALLOWED, apiMode);       //抛异常
   }

   ResData[2] =((ToolkitHandlerList[Index_ProactiveHandler].TLVListLength) >= 0xFD)?0xFD : (ToolkitHandlerList[Index_ProactiveHandler].TLVListLength);//主动式命令支持长度为256，此时发9100

   result = ProactivecmdPermission();

   if(result == FALSE)
   {
      UiccOrSimThrowToolkitException(COMMAND_NOT_ALLOWED_SIM, apiMode);       //抛异常
	   
	  return;
   }

   if(ResData[6] == CMD_CLOSECHANNEL)
   {
   	  channelId = result;
   }
   else if(ResData[6] == CMD_DECLARE_SERVICE)
   {
   	   channelId = result-1;
   }

   WriteVariable(Tag_BakPostData,&ResData[2]);   //由于send 之后可以再次调用send函数，为防止在SendProactiveCommand 中ResData中的值由于其他应用被触发，被冲掉，这里先缓存ResData的值
   
   SendProactiveCommand();
   
   ecpMem_Set(ResData,0xD0,2);
			
   ResData[2] = ReadVarible(Tag_BakPostData,&ResData[3]);   //恢复之前缓存的ResData的值
   
   ClearVariable(Tag_BakPostData);

   ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;

   ToolkitHandlerList[Index_ProResHandler].TLVListLength = P3;
   
   ToolkitHandlerList[Index_ProResHandler].HandlerAvailable = TRUE;	

   ClearHandlerCurrentTLV(Index_ProResHandler); 

   result = FindTag(&RecData[0],TLV_RESULT,P3);

   if(result == FALSE)
   {
		UiccOrSimThrowToolkitException(UNAVAILABLE_ELEMENT_SIM, apiMode); 	//ToolkitException-UNAVAILABLE_ELEMENT
	   
	    return;
   }

   if(RecData[result] == 0x00)
   {
	   UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode);//TOOLKITEXCEPTION_OUT_OF_TLV_BOUNDARIES
	   
	    return;
   }

   if((RecData[3] == CMD_OPENCHANNEL)||(RecData[3] == CMD_CLOSECHANNEL)||(RecData[3] == CMD_DECLARE_SERVICE))  //open channel (0x40) or close channel(0x41)	 or declere service(0x47)
   {
   		//question :open channel是否要判断当前应用 的通道数量是否已达到最大呢？ ，若通道数已满怎么处理，是否抛出异常，待确认; 同时需要判断其他应用是否已占用该通道
   		if(RecData[result+1] < 0x0F)   //successful response
		{
			if(RecData[3] == CMD_DECLARE_SERVICE)
			{
				SetToolkitChannelServiceIdStatus(channelId,(RecData[4]&0x01),CurrentService_OFFSET);
			}
			else
			{
				if(RecData[3] == CMD_OPENCHANNEL)
				{
					tmp = FindTag(&RecData[0],TLV_CHANNELSTATUS,P3);
			
					channelId = RecData[tmp+1]&0x07;
				}
	
				SetToolkitChannelServiceIdStatus(channelId,(RecData[3]&0x01),AllocateChannelCount_OFFSET);
			}

			
		}
   }

	PushOperandStackShort((jbyte)RecData[result+1]);
}
#endif
