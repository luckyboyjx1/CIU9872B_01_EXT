 /************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCToolkitRegistery.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2014-02-09

*Description :该文件主要用于实现基于TS131130 所描述的interface USTAEnvelopHandler的功能  
*
*History     :
	
author    time      version    description
xuyz   2014/2/09   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "uicc_main.h"
#include    "uicc_interface.h"
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
#include    "USATEnvelopeHandler.h"
#include    "ToolkitAPIPublic.h"
#include "platform_api.h"
#include "Interpreter.h"
//#include "VMAccessEeprom.h"

#if UICC__FUNCTION__USIMAPI
/**
*@description : Looks for the Short Message offset in the User Data part of the first SMS TPDU  or of the first Cell Broadcast Page Simple TLV contained in the USATEnvelopeHandler.
*              If this offset is available the simple TLV element becomes the TLV selected.
*
*This can be used on the events: 
*EVENT_FORMATTED_SMS_PP_ENV, EVENT_FORMATTED_SMS_PP_UPD, EVENT_UNFORMATTED_SMS_PP_ENV,  EVENT_UNFORMATTED_SMS_PP_UPD. It returns the offset of the first byte in the User Data part of the first SMS TPDU. 
*It is the offset of the first byte after the UDH, if any.
*
*This can be used on the events:
*EVENT_FORMATTED_SMS_CB, EVENT_UNFORMATTED_SMS_CB. It returns the offset of the first  byte in the User Data part of the first Cell Broadcast Page Simple TLV. Is is the offset of the first byte after the UDH, if any.</ul>
* 
*
*@exception   : ToolkitException with the following reason codes: 
*                UNAVAILABLE_ELEMENT</code> in case of unavailable SMS TPDU or Cell Broadcast Page or wrong data format  </ul>
*
*@author      :create by xuyz@20140208
*
*@history      :V1.0:create by xuyz@20140208
*/
void USATEnvelop_getShortMessageOffset(void)
{
	PushOperandStackShort(ToolkitSubGetSecureDataLenOffset(MODE_MESSAGE_OFFSET));	
}
/**
*@description   :Looks for the length of the Short Message in the User Data part of the first SMS TPDU or Cell Broadcast Page COMPREHENSION TLV  contained in the USATEnvelopeHandler.<br>
*
*                This can be used on the events: 
*                EVENT_FORMATTED_SMS_PP_ENV, EVENT_FORMATTED_SMS_PP_UPD, EVENT_UNFORMATTED_SMS_PP_ENV, EVENT_UNFORMATTED_SMS_PP_UPD.  It, returns the length of the Short Message in the User Data part of the first SMS TPDU. This length does not include the UDH, if any.</ul>
*
*                This can be used on the events:<ul> 
*                EVENT_FORMATTED_SMS_CB, EVENT_UNFORMATTED_SMS_CB. It returns the length of the "CBS-Message-Information-Page" field times the number of pages as defined in 3GPP TS 23.041
* 
*                If this length is available the simple TLV becomes the TLV selected.
*
*@return       :the length of the Short Message in the User Data part of the first SMS TPDU or the length of the Cell Broadcast Page Message Information. 
*               This length does not include the UDH, if any. If the Short Message length is zero, no exception shall be thrown.
*
*@exception    : ToolkitException with the following reason codes: <ul>
*                UNAVAILABLE_ELEMENT  in case of unavailable SMS TPDU or Cell Broadcast Page  or wrong data format </ul>
*
*@author       : create by xuyz@20140208
*
*@history      :V1.0 :Create by xuyz@20140208
*/
void USATEnvelop_getShortMessageLength(void) 
{
	PushOperandStackShort(ToolkitSubGetSecureDataLenOffset(MODE_MESSAGE_LENGTH));	
}
#endif
