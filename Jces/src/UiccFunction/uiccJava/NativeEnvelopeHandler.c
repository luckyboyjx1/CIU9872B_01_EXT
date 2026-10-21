/*
*@History  :V1.0:Create
*           V1.1:优化代码,增加公共函数Toolkit_subTPUDLOffset ToolkitSubGetSecureDataLenOffset  @by xuyz@20131012
*
*			V1.2 :删除文件中的无效代码 @20131015 by xuyz

*/
#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "SimApi.h"
#include "SimtoolkitConstant.h"
#include "fileAction.h"
//#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
#include "platform_api.h"
#if UICC__FUNCTION__SIMAPI
/**
* Returns the Envelope BER-TLV tag.
*
* @return Envelope BER-TLV tag
*/
void Toolkit_getEnvelopeTag(void)
{
	PushOperandStackShort((jbyte)G_EnvelopeTag);	
}

/**
* Looks for the TP-UDL field in the first TPDU TLV element in the Envelope
* data field. This method can be used on the events EVENT_FORMATTED_SMS_PP_ENV,
* EVENT_FORMATTED_SMS_PP_UPD, EVENT_UNFORMATTED_SMS_PP_ENV, EVENT_UNFORMATTED_SMS_PP_UPD.
* If the element is available it becomes the TLV selected.
*
* @return TPUDL offset in the first TPDU TLV element if TPUDL exists.
*         The value retrieved by using getValueByte is meaningless when the message is
*         Concatenated. To retrieve the TP-User-Data-Length the method getUserDataLength() shall be used.
*
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TPDU TLV element or if the TPUDL field does not exist</ul>
*/
void Toolkit_getTPUDLOffset(void)
{
	PushOperandStackShort(Toolkit_subTPUDLOffset());
}

/**
 * Looks for the TP-User Data field in the first TPDU TLV element contained in the Envelope Handler
 * This method can be used on the events EVENT_FORMATTED_SMS_PP_ENV,
 * EVENT_FORMATTED_SMS_PP_UPD, EVENT_UNFORMATTED_SMS_PP_ENV, EVENT_UNFORMATTED_SMS_PP_UPD.
 * If the element is available it becomes the TLV selected.
 *
 * @return the length of the User Data contained in the first SMS TPDU TLV element.
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TPDU TLV element or wrong data format </ul>
 */ 
void Toolkit_getUserDataLength(void)
{
	WORD idata offset,userDataLen;
	
	offset = Toolkit_subTPUDLOffset();

	userDataLen = (ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength)-offset-1; 

	PushOperandStackShort(userDataLen);
}
/**
 * Looks for the Secured Data from the Command Packet in the first SMS TPDU 
 * or Cell Broadcast Page Simple TLV contained in the Envelope handler. This can
 * be used on the events:
 * - EVENT_FORMATTED_SMS_PP_ENV, EVENT_FORMATTED_SMS_PP_UPD, if the SMS TP-UD is formatted
 * according to TS 03.48 Single or Concatenated Short Message.
 * - EVENT_FORMATTED_SMS_CB, if the Cell Broadcast Page is formatted according to GSM 03.48.
 * If the element is available it becomes the TLV selected.
 *
 * @return the offset of the Secured Data first byte in the first SMS TPDU or Cell Broadcast Page TLV element. 
 *         If the Secured Data length is zero the value returned shall be the offset of the first byte following 
 *         the TS 03.48 Command Packet structure.
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable SMS TPDU or Cell Broadcast Page TLV element or wrong data format </ul>
 */
void Toolkit_getSecuredDataOffset(void)
{
	PushOperandStackShort(ToolkitSubGetSecureDataLenOffset(MODE_SECURE_OFFSET));
}

/**
 * Looks for the length of the Secured Data from the Command Packet in the first SMS TPDU
 * or Cell Broadcast Page Simple TLV contained in the Envelope handler. This can be used
 * on the events:
 * - EVENT_FORMATTED_SMS_PP_ENV, EVENT_FORMATTED_SMS_PP_UPD, if the SMS TP-UD
 * is formatted according to GSM03.48 Single or Concatenated Short Message.
 * - EVENT_FORMATTED_SMS_CB, if the Cell Broadcast Page is formatted according to GSM 03.48.    
 * If the element is available it becomes the TLV selected.
 *
 * @return the length of the Secured Data contained in the first SMS TPDU or Cell Broadcast Page TLV element (without padding bytes). If the Secured Data length is zero, no exception shall be thrown.
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable SMS TPDU or Cell Broadcast Page TLV element or wrong data format </ul>
 */
void Toolkit_getSecuredDataLength(void)
{
	PushOperandStackShort(ToolkitSubGetSecureDataLenOffset(MODE_SECURE_LENGTH));
}
#endif
