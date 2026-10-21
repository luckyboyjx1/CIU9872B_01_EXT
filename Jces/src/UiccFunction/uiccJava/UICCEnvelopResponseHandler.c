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

*Description :该文件主要用于实现基于TS102241 所描述的interface EnvelopResponseHandler的功能 ，该接口主要包含处理post postAsBERTLV
*
*History     :
	
author    time      version    description
xuyz   2013/11/20   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "uicc_error.h"
#include    "UICCToolkitAPI.h"
#include    "ToolkitRFM.h"
#include    "UICCToolkitConstants.h"
#include    "USATEnvelopeHandler.h"
//#include   "Interpreter.h"
#include   "T0ApduImpl.h"
//#include    "JCREMain.h"
//#include    "GPPublic.h"
#include    "interface.h"
#include    "ToolkitAPIPublic.h"

#if UICC__FUNCTION__UICCAPI
/**
*@function     : UICCpost
*
*description   :Prepares the Envelope response
*
*@param        :value  :indicating successful processing of the Envelope command
*              The CAT Runtime Environment has to map the boolean value to the correct status word
*
*@exception    :ToolkitException  with the following reason codes: 
*              <li>HANDLER_NOT_AVAILABLE if the handler is busy 
*@exception    :APDUException     with the following reason code: 
*               <li>BAD_LENGTH if the resulting response length is greater than 256 and the response data has to be retrieved by the GET RESPONSE  command.
*
*@author       :xuyz
*
*@history      :V1.0 :create by xuyz@20131125
*
*@others       :
*/ 
void UICCpost(void) 
{
	Toolkit_Post();
}

/**
*@function    :UICCpostAsBERTLV
*
*@description :Prepare the Envelope response in a BER TLV structure. Should be used with Envelope Call Control by NAA or with an Unrecognized Envelope. The tag value is to be used to set the Result for Call Control.
*
*@param       :value indicating successful processing of the Envelope command
*              The CAT Runtime Environment has to map the boolean value to the correct status word
*@param       :tag    the BER Tag to be used at the beginning of the Comprehension_TLV list.
*
*@exception   :APDUException     with the following reason code: 
*             <li>BAD_LENGTH if the resulting response length is greater  than 256 bytes and the response data has to be retrieved by the GET RESPONSE command.
*@exception  ToolkitException  with the following reason codes: 
*            <li>HANDLER_NOT_AVAILABLE if the handler is busy
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20131125
*
*@others      :
*/
void UICCpostAsBERTLV(void) 
{
	Toolkit_PostAsBERTLV();
}
#endif
