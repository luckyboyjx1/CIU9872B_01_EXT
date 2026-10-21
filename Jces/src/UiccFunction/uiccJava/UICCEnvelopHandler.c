/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCEnvelopHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-28

*Description :该文件主要用于实现基于TS102241 所描述的interface EnvelopHandler的功能 ，该接口主要包含处理Envelophander 数据的一些基本方法，
*
*History     :
	
author    time      version    description
xuyz   2013/10/28   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
//#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
#include "platform_api.h"
#if UICC__FUNCTION__UICCAPI
/**
 *@fucntion    : UICCEnvelope_Toolkit_getItemIdentifier
 *
 *@description :Returns the item identifier byte value from the first Item Identifier TLV element in the current Envelope data field. If the element is available it becomes the TLV selected.
 *
 *@see          :TS 102 241 :interface EnvelopHandler
 *
 *@param        :handlerIndex
 *
 *@return       :item identifier
 *
 *@exception    :ToolkitException  with the following reason codes: 
 *              <li>UNAVAILABLE_ELEMENT in case of unavailable TLV element
 *              <li>OUT_OF_TLV_BOUNDARIES if the item identifier byte is missing in the Item Identifier Comprehension TLV.
 *
 *@author       : xuyz
 *
 *@hisotry      :V1.0 :create by xuyz@20131028
 */
void UICCEnvelope_Toolkit_getItemIdentifier(void)
{
	PushOperandStackShort((jbyte)Toolkit_getFirstByteOfSpecifiedTag(Index_EnvelopeHandler, TAG_ITEM_IDENTIFIER, UICCAPI_Mode));
}

/**
 * @function     :UICCEnvelope_Toolkit_getChannelStatus
 *
 * @description :the channel identifier value from the first Channel status TLV element in the current Envelope data field. If the element is available it becomes the currently selected TLV.
 *
 * @return      :channel identifier
 *
 * @exception   :ToolkitException  - with the following reason codes: 
 *               <li>UNAVAILABLE_ELEMENT in case of unavailable TLV element
 *               <li>OUT_OF_TLV_BOUNDARIES if the Comprehension TLV Channels Status length is equal to zero.
 *
 * @author       :xuyz
 *
 * @history      :create by xuyz@20131028
 */
void UICCEnvelope_Toolkit_getChannelIdentifier(void)
{
	BYTE idata identifier;
	
	identifier = Toolkit_getFirstByteOfSpecifiedTag(Index_EnvelopeHandler, TAG_CHANNEL_DATA_STATUS, UICCAPI_Mode);
	
	PushOperandStackShort((jbyte)(identifier&0x07));
}
 /**
 *@function    :UICCEnvelope_Toolkit_getChannelStatus
 *   
 *@description : Returns the value of the first Channel Status TLV element whose channel identifier is equal to the channelIdentifier parameter. If the element is available it becomes the currently selected TLV element.
 *
 *@param       :channelIdentifier     the channel number
 *
 *@return      :channel identifier and status : MSB = byte 1 of the Channel Status TLV value (byte 3 of the Channel Status TLV element), LSB = byte 2 of the Channel Status TLV value (byte 4 of the Channel Status TLV element).
 *
 *@exception   :ToolkitException  with the following reason codes: 
 *             <li>UNAVAILABLE_ELEMENT if no Channel Status TLV element with the right identifier could be found
 *          	<li>OUT_OF_TLV_BOUNDARIES if a Channel Status TLV element with the right identifier could be found but its value is less than 2 bytes long 
 *
 *@author      :xuyz
 *
 *@history     :V1.0:create by xuyz@20131028
 *
 *@others      : //该函数的功能没有完全理解，如数据里面包含 3800 38020100 ,那么调用API UICCEnvelope_Toolkit_getChannelStatus(01)的返回值是什么？？
 */
 void UICCEnvelope_Toolkit_getChannelStatus(void)	 
 {
 	BYTE idata identifier,result,i;

	jbyte idata  channelIdentifier = PopOperandStackShort();

	for(i=1;;i++)
	{
		ToolkitFindTLVPublic(Index_EnvelopeHandler,TAG_CHANNEL_DATA_STATUS,i,UICCAPI_Mode);
		
		handler_check_currenttlv(Index_EnvelopeHandler,UICCAPI_Mode);
		
		if(ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength == 0)		  //Api_2_Enh_Gcst   步骤3
		{
			UICCThrowToolkitException(UNAVAILABLE_ELEMENT); 	//ToolkitException-UNAVAILABLE_ELEMENT
		}

		if(ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength == 1)		  //Api_2_Enh_Gcst 	 步骤4
		{
			UICCThrowToolkitException(OUT_OF_TLV_BOUNDARIES); 	//ToolkitException-UNAVAILABLE_ELEMENT
		}
		
		identifier = Toolkit_getValueBytePublic(Index_EnvelopeHandler,0, UICCAPI_Mode);	
		
		if((identifier&0x07) == channelIdentifier)
		{
		   result =   Toolkit_getValueBytePublic(Index_EnvelopeHandler,1, UICCAPI_Mode);
            
		   PushOperandStackShort(((((short)identifier)<<8)&0xFF00)|(result));
            
		   return;
		}
	}
 }

/**
*@function    :UICCToolkit_getBERTag
*
*@description :Returns the BER Tag of the BER TLV list for EnvelopHandler
*
@param        :handlerIndex
*
*@return      :the tag value
*
*@author      :xuyz
*
*@history     :V1.1:create by xuyz@20131225
*/
void UICCToolkit_getBERTag(void) 
{
	jbyte idata handlerIndex = PopOperandStackShort();

	PushOperandStackShort((jbyte)G_EnvelopeTag);
}

/**
*@function    : UICCToolkit_getBERSize
*
*@description :Returns the BER TLV size, this includes the tag and the length.
* The length of the TLV list can be retrieved with the ViewHandler.getLength() method.
* @return the size of the BER TLV structure 
*/
void UICCToolkit_getBERSize(void)
{
	WORD  idata length;

	jbyte idata handlerIndex = PopOperandStackShort();

	length = ToolkitHandlerList[handlerIndex].TLVListLength;

	if(length < 0x80)
	{
		length += 2;	 //tag +length
	}
	else if(length <0x100)
	{
		length += 3;	    //tag + 81 +length
	}
	else
	{
		length += 4;	  //tag + 82 +length1+length2
	}

	PushOperandStackShort(length);
}
#endif


