/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCProactiveResHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-30

*Description :该文件主要用于实现基于TS102241 所描述的interface ProactiveResHandler的功能 ，该接口主要包含处理ProactiveResHandler数据的一些基本方法，
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
//#include   "Interpreter.h"
#include   "ToolkitAPIPublic.h"
#include "platform_api.h"
#if UICC__FUNCTION__UICCAPI
/*@Function:	UICCProactiveResponse_Toolkit_getChannelStatus
 *
 *@description: Returns the value of the first Channel Status TLV element whose channel identifier is equal to the channelIdentifier parameter.
 *              If the element is available it becomes the currently selected TLV element, else no TLV is selected.                                         
 *
 * @param       channelIdentifier
 *
 * @return      channel identifier and status : MSB = byte 1 of the Channel Status TLV value (byte 3 of the	Channel Status TLV element),
 *                                              LSB = byte 2 of the Channel Status TLV value (byte 4 of the Channel	Status TLV element).
 * 
 * @exception  ToolkitException  with the following reason codes:
 *      <li>UNAVAILABLE_ELEMENT if no Channel Status TLV element with the right identifier could be found
 *	    <li>OUT_OF_TLV_BOUNDARIES if a Channel Status TLV element with the right
 *                identifier could be found but its value is less than 2 bytes long 
 *
 *@author     : create by zzj
 *
 *@history    : V1.0:create by zzj@20131031
*/
void UICCProactiveResponse_Toolkit_getChannelStatus(void)
{
 	BYTE identifier,result,i;

	jbyte idata channelIdentifier = PopOperandStackShort();

	for(i=1;;i++)
	{
		result = ToolkitFindTLVPublic(Index_ProResHandler,TAG_CHANNEL_DATA_STATUS,i,UICCAPI_Mode);

		if(result == TLV_NOT_FOUND)
		{
			if(i == 1)											 //一个TAG_CHANNEL_DATA_STATUS都没找到
			{
				UICCThrowToolkitException(UNAVAILABLE_ELEMENT);
			}
			else
			{
				break;
			}
		}
		
		if(ToolkitHandlerList[Index_ProResHandler].CurrentTLVLength == 0)		  //Api_2_Enh_Gcst   步骤3
		{
			UICCThrowToolkitException(UNAVAILABLE_ELEMENT); 	//ToolkitException-UNAVAILABLE_ELEMENT
		}
	
		if(ToolkitHandlerList[Index_ProResHandler].CurrentTLVLength == 1)		  //Api_2_Enh_Gcst 	 步骤4
		{
			UICCThrowToolkitException(OUT_OF_TLV_BOUNDARIES); 	//ToolkitException-UNAVAILABLE_ELEMENT
		}
		
		identifier = Toolkit_getValueBytePublic(Index_ProResHandler,0, UICCAPI_Mode);	
		
		if((identifier&0x07) == channelIdentifier)
		{
		   result =   Toolkit_getValueBytePublic(Index_ProResHandler,1, UICCAPI_Mode);
	
		   PushOperandStackShort(  ((((short)identifier)<<8)&0xFF00)|(result)  )	;

		   return;
		}
	}

	UICCThrowToolkitException(UNAVAILABLE_ELEMENT);		
}
#endif		
