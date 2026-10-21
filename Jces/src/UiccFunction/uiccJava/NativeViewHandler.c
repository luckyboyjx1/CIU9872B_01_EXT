

#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "SimApi.h"
#include "fileAction.h"
#include "Interpreter.h"
//#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
//#include "ChannelManagement.h"
#include "platform_api.h"
#if UICC__FUNCTION__SIMAPI
/*
*@description :	 Returns the length of the TLV list

*@param       :	 handlerIndex
*
*@return      :  length in bytes
*
*@exception   :   ToolkitException : HANDLER_NOT_AVAILABLE if the handler is busy
*
*@history     :
*/
void Toolkit_getLength(void)
{	
	jbyte idata  handlerIndex;
	jshort idata  apiMode;
	
	apiMode = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();
	
	handler_check_available(handlerIndex, apiMode);

  	PushOperandStackShort(ToolkitHandlerList[handlerIndex].TLVListLength);
}
/**
* Copies the simple TLV list contained in the handler to the destination byte array.
*
* @param handlerIndex
* @param dstBuffer a reference to the destination buffer
* @param dstOffset the position in the destination buffer
* @param dstLength the data length to be copied
*
* @return (short)dstOffset+dstLength
*
* @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if copy would cause access of data outside array bounds.
			If dstOffset or dstLength parameter is negative an ArrayIndexOutOfBoundsException exception is thrown and no copy is performed. 
			If dstOffset+dstLength is greater than dstBuffer.length, the length of the dstBuffer array an ArrayIndexOutOfBoundsException exception is thrown and no copy is performed

* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>OUT_OF_TLV_BOUNDARIES</code> if <code>dstLength</code> is grater than the length of the simple TLV List.</ul>
 */
void Toolkit_copy(void)
{
 	jbyte idata handlerIndex; 
 	jref idata dstBuffer; 
 	jshort idata dstOffset; 
 	jshort idata dstLength;
	jshort idata apiMode;
 	
	apiMode = PopOperandStackShort();
	dstLength = PopOperandStackShort();
 	dstOffset = PopOperandStackShort();
	dstBuffer = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);

	check_bounds_interval_buffer(dstBuffer, dstOffset, dstLength);	  //异常：ArrayIndexOutOfBoundsException
	
	if(dstLength > ToolkitHandlerList[handlerIndex].TLVListLength)
	{		
		UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode);		//抛出异常 OUT_OF_TLV_BOUNDARIES	

		return;
	}
	
	Toolkit_copyHandlerBufferDataToDstBuffer(handlerIndex,0x00,dstBuffer,dstOffset,dstLength);
		
	PushOperandStackShort((jshort)(dstOffset+dstLength));
 }
 /**
 * Looks for the indicated occurence of a TLV element from the beginning of the TLV list (handler buffer). If the method is successful then the corresponding TLV becomes current, else no TLV is selected.
 * This search method is Comprehension Required flag independent.
 *
 * @param :handlerIndex
 * @param tag the tag of the TLV element to search
 * @param occurrence the occurrence number of the TLV element (1 for the first, 2 for the second...)
 *
 * @return result of the method: <ul>
 *      <li><code>TLV_NOT_FOUND</code> if the required occurrence of the TLV element does not exist
 *      <li><code>TLV_FOUND_CR_SET</code> if the required occurence exists and Comprehension Required flag is set
 *      <li><code>TLV_FOUND_CR_NOT_SET</code> if the required occurence exists and Comprehension Required flag is not set</ul>
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
 *      <li><code>BAD_INPUT_PARAMETER</code> if an input parameter is not valid (e.g. occurence = 0)</ul>
 */	 
void Toolkit_findTLV(void)
{
	jbyte idata handlerIndex; 
	jbyte idata tag; 
	jbyte idata occurrence;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	occurrence = PopOperandStackShort();
    
	tag = PopOperandStackShort();
    
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);

	tag = ToolkitFindTLVPublic(handlerIndex,tag,occurrence,apiMode);

	PushOperandStackShort(tag);
}
/**
* Gets the binary length of the value field for the last TLV element which has been found in the handler.
*
* @param :handlerIndex

* @return length of the value field
*
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element</ul>
*/
void Toolkit_getValueLength(void)
{
	jbyte idata handlerIndex;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();
	
	handler_check_available(handlerIndex, apiMode);

	handler_check_currenttlv(handlerIndex, apiMode);

	PushOperandStackShort((jshort)ToolkitHandlerList[handlerIndex].CurrentTLVLength);
}
/**
 * Gets a byte from the last TLV element which has been found in the handler.
 *
 * @param handlerIndex
 * @param valueOffset the offset of the byte to return in the TLV element
 *
 * @return element value (1 byte)
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element
 *      <li><code>OUT_OF_TLV_BOUNDARIES</code> if <code>valueOffset</code> is out of the current TLV </ul>
 */
void Toolkit_getValueByte(void)
{
	jbyte idata value;

	jbyte idata handlerIndex; 
	jshort idata valueOffset;		
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort(); 
	valueOffset = PopOperandStackShort(); 
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);

  	value = Toolkit_getValueBytePublic(handlerIndex,valueOffset, apiMode);

	PushOperandStackShort(value);
}
 /**
 * Copies a part of the last TLV element which has been found, into a
 * destination buffer.
 *
 * @param handlerIndex
 * @param valueOffset the offset of the first byte in the source TLV element
 * @param dstBuffer a reference to the destination buffer
 * @param dstOffset the position in the destination buffer
 * @param dstLength the data length to be copied
 *
 * @return <code>dstOffset+dstLength</code>
 *
 * @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
 * @exception ArrayIndexOutOfBoundsException if copyValue would cause access of data outside array bounds.
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element
 *      <li><code>OUT_OF_TLV_BOUNDARIES</code> if <code>valueOffset</code>, <code>dstLength</code> or both are out of the current TLV </ul>
 */
void Toolkit_copyValue(void)
{
	jbyte idata handlerIndex; 
	jshort idata valueOffset; 
	jref idata dstBuffer; 
	jshort idata dstOffset; 
	jshort idata dstLength;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	dstLength = PopOperandStackShort();
	dstOffset = PopOperandStackShort();
	dstBuffer = PopOperandStackShort();
	valueOffset = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();
	
	PushOperandStackShort(Toolkit_BaseCopyDataToDstBuffer(handlerIndex, valueOffset, dstBuffer, dstOffset, dstLength,0x00, apiMode));		
}
 /**
 * Compares the last found TLV element with a buffer.
 *
 * @param handlerIndex
 * @param valueOffset the offset of the first byte to compare in the TLV element
 * @param compareBuffer a reference to the comparison buffer
 * @param compareOffset the position in the comparison buffer
 * @param compareLength the length to be compared
 *
 * @return the result of the comparison as follows: <ul>
 *      <li>0 if identical
 *      <li>-1 if the first miscomparing byte in simple TLV List is less than that in compareBuffer
 *      <li> 1 if the first miscomparing byte in simple TLV List is greater than that in compareBuffer</code>.</ul>
 *
 * @exception NullPointerException if <code>compareBuffer is null
 * @exception ArrayIndexOutOfBoundsException if compareValue would cause access of data outside array bounds.
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element
 *      <li><code>OUT_OF_TLV_BOUNDARIES</code> if valueOffset, compareLength or both are out of the current TLV </ul>
 */
 void Toolkit_compareValue(void)
 {
	jbyte idata handlerIndex; 
	jshort idata valueOffset; 
	jref idata compareBuffer; 
	jshort idata compareOffset; 
	jshort idata compareLength;
	jshort idata apiMode;
			
	apiMode = PopOperandStackShort();
	compareLength = PopOperandStackShort();
	compareOffset = PopOperandStackShort();
	compareBuffer = PopOperandStackShort();
	valueOffset = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();

		
	PushOperandStackShort(Toolkit_BaseCompareDstBuffer(handlerIndex, valueOffset, compareBuffer, compareOffset,compareLength,0x00, apiMode));
 }
 /**
* Looks for the first occurence of a TLV element from the beginning of a TLV
* list and copy its value into a destination buffer.
* If no TLV element is found, the <code>UNAVAILABLE_ELEMENT</code> exception is thrown.
* If the method is successful then the corresponding TLV becomes current,
* else no TLV is selected.
* This search method is Comprehension Required flag independent. 
*
* @param handlerIndex
* @param tag the tag of the TLV element to search
* @param dstBuffer a reference to the destination buffer
* @param dstOffset the position in the destination buffer
*
* @return <code>dstOffset</code> + length of the copied value
*
* @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if findAndCopyValue would cause access of data outside array bounds.
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element</ul>
 */
void Toolkit_findAndCopyValue_3(void)
{
	BYTE idata result;
 	jbyte idata handlerIndex; 
 	jbyte idata tag; 
 	jref idata dstBuffer; 
 	jshort idata  dstOffset;
	jshort idata  apiMode;
	
	apiMode = PopOperandStackShort();
 	dstOffset = PopOperandStackShort();
	dstBuffer = PopOperandStackShort();
 	tag = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);
	
	result = ToolkitFindTLVPublic(handlerIndex,tag,1,apiMode);
	
	if(result ==TLV_NOT_FOUND)
	{
		if(handlerIndex == Index_BERTLVEditHandler)
		{
			UICCToolkitWriteBERTLVHandlerContext();
		}
	}

	PushOperandStackShort(Toolkit_BaseCopyDataToDstBuffer(handlerIndex, 0x00, dstBuffer, dstOffset, ToolkitHandlerList[handlerIndex].CurrentTLVLength , 0x01, apiMode));
}
 /**
* Looks for the indicated occurence of a TLV element from the beginning of a TLV
* list and copy its value into a destination buffer.
* If no TLV element is found, the <code>UNAVAILABLE_ELEMENT</code> exception is thrown.
* If the method is successful then the corresponding TLV becomes current,
* else no TLV is selected.
* This search method is Comprehension Required flag independent.
* 
* @param handlerIndex
* @param tag the tag of the TLV element to search
* @param occurrence the occurrence number of the TLV element (1 for the first, 2 for the second...)
* @param valueOffset the offset of the first byte in the source TLV element
* @param dstBuffer a reference to the destination buffer
* @param dstOffset the position in the destination buffer
* @param dstLength the data length to be copied
*
* @return <code>dstOffset + dstLength</code>
*
* @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if findAndCopyValue would cause access of data outside array bounds.
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element
*      <li><code>OUT_OF_TLV_BOUNDARIES</code> if valueOffset, dstLength or both are out of the current TLV
*      <li><code>BAD_INPUT_PARAMETER</code> if an input parameter is not valid (e.g. occurence = 0)</ul>
*/
void Toolkit_findAndCopyValue_6(void)
{
	jbyte idata handlerIndex; 
	jbyte idata tag; 
	jbyte idata occurrence; 
	jshort idata  valueOffset; 
	jref idata dstBuffer; 
	jshort idata dstoffset; 
	jshort idata dstLength;
	jshort idata apiMode;
	jbyte idata result;
	
 	apiMode = PopOperandStackShort();
  	dstLength = PopOperandStackShort();
	dstoffset = PopOperandStackShort();
	dstBuffer = PopOperandStackShort();
	valueOffset = PopOperandStackShort();
	occurrence = PopOperandStackShort();
	tag = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);
	
	result = ToolkitFindTLVPublic(handlerIndex,tag,occurrence,apiMode);
	
	if(result ==TLV_NOT_FOUND)
	{
		if(handlerIndex == Index_BERTLVEditHandler)
		{
			UICCToolkitWriteBERTLVHandlerContext();
		}
	}
	
	PushOperandStackShort( Toolkit_BaseCopyDataToDstBuffer(handlerIndex, valueOffset, dstBuffer, dstoffset, dstLength,0x02, apiMode));	
}
/**
* Looks for the first occurence of a TLV element from beginning of a TLV
* list and compare its value with a buffer.
* If no TLV element is found, the <code>UNAVAILABLE_ELEMENT</code> exception is thrown.
* If the method is successful then the corresponding TLV becomes current,
* else no TLV is selected.
* This search method is Comprehension Required flag independent.
*
* @param handlerIndex
* @param tag the tag of the TLV element to search
* @param compareBuffer a reference to the comparison buffer
* @param compareOffset the position in the comparison buffer
*
* @return the result of the comparison as follows: <ul>
*      <li><code>0 if identical
*      <li><code>-1 if the first miscomparing byte in simple TLV is less than that in <code>compareBuffer</code>,
*      <li><code>1if the first miscomparing byte in simple TLV is greater than that in <code>compareBuffer</code>.</ul>
*
* @exception NullPointerException if <code>compareBuffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if findAndCompareValue would cause access of data outside array bounds.
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element</ul>
*/
void Toolkit_findAndCompareValue_3(void)
{
	jbyte idata handlerIndex; 
	jbyte idata tag; 
	jref idata compareBuffer; 
	jshort idata compareOffset;
	jshort idata apiMode;
	jbyte idata result;
	
	apiMode = PopOperandStackShort();
	compareOffset = PopOperandStackShort();
	compareBuffer = PopOperandStackShort();
	tag = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);
	
	result = ToolkitFindTLVPublic(handlerIndex,tag,1,apiMode);
	
	if(result ==TLV_NOT_FOUND)
	{
		if(handlerIndex == Index_BERTLVEditHandler)
		{
			UICCToolkitWriteBERTLVHandlerContext();
		}
	}

	PushOperandStackShort(Toolkit_BaseCompareDstBuffer(handlerIndex, 0x00, compareBuffer, compareOffset,ToolkitHandlerList[handlerIndex].CurrentTLVLength,0x01, apiMode));
}
 /**
* Looks for the indicated occurence of a TLV element from the beginning of a
* TLV list and compare its value with a buffer.
* If no TLV element is found, the <code>UNAVAILABLE_ELEMENT</code> exception is thrown.
* If the method is successful then the corresponding TLV becomes current,
* else no TLV is selected.
* This search method is Comprehension Required flag independent.
*
* @param handlerIndex
* @param tag the tag of the TLV element to search
* @param occurrence the occurrence number of the TLV element (1 for the first, 2 for the second...)
* @param valueOffset the offset of the first byte in the source TLV element
* @param compareBuffer a reference to the comparison buffer
* @param compareOffset the position in the comparison buffer
* @param compareLength the length to be compared
*
* @return the result of the comparison as follows: <ul>
*      <li><code>0</code> if identical
*      <li><code>-1</code> if the first miscomparing byte in simple TLV is less than that in <code>compareBuffer</code>,
*      <li><code>1</code> if the first miscomparing byte in simple TLV is greater than that in <code>compareBuffer</code>.</ul>
*
* @exception NullPointerException if <code>compareBuffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if findAndCompareValue would cause access of data outside array bounds.
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element
*      <li><code>OUT_OF_TLV_BOUNDARIES</code> if <code>valueOffset</code>, <code>compareLength</code> or both are out of the current TLV
*      <li><code>BAD_INPUT_PARAMETER</code> if an input parameter is not valid (e.g. occurence = 0)</ul>
*/
void Toolkit_findAndCompareValue_6(void)
{
	jbyte idata handlerIndex; 
	jbyte idata tag; 
	jbyte idata occurrence; 
	jshort idata valueOffset; 
	jref idata compareBuffer; 
	jshort idata compareoffset; 
	jshort idata compareLength;
	jshort idata apiMode;

	apiMode = PopOperandStackShort();
	compareLength = PopOperandStackShort();
	compareoffset = PopOperandStackShort();
	compareBuffer = PopOperandStackShort();
	valueOffset = PopOperandStackShort();
	occurrence = PopOperandStackShort();
 	tag = PopOperandStackShort();
	handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, apiMode);
	
	ToolkitFindTLVPublic(handlerIndex,tag,occurrence,apiMode);

	PushOperandStackShort(Toolkit_BaseCompareDstBuffer(handlerIndex, valueOffset, compareBuffer, compareoffset,compareLength,0x02, apiMode));
}

/*
*@function : Toolkit_GetHandlerStatus
*
*@description : 
*
*@param      :
*
*@return     :
*
*@history    :
*/
void Toolkit_GetHandlerStatus(void)
{
	jshort idata apiMode = PopOperandStackShort();
	jbyte index = PopOperandStackShort();

	if(index == Index_ProactiveHandler)
	{
		if((G_STKBusy != FALSE)||((STKPerform[0]!=FALSE)&&(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] != EVENT_MENU_SELECTION_HELP_REQUEST_bit)&&(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] != EVENT_MENU_SELECTION_bit)))	 ////E:\N3_TEE\develop\branches\ChinaUnicomUicc\test\中国联通NFC-SWP测试工程集测试脚本20160321\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\ReEntry_test_工程集\SIMReEntry_01_11_01
		{
			PushOperandStackShort(FALSE);

			return;
		}
	}
	
	if(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_InitValue)  //通过A4进行了tookitAPI的调用，返回错误
	{
			PushOperandStackShort(FALSE);

			return;
	}
	
	PushOperandStackShort(ToolkitHandlerList[index].HandlerAvailable);
}

/*@Function:    Toolkit_GetHanlderCapacity                                            
 * 
 *@Descrip:     获取toolkithanlder最大承载数据长度
 *
 *@See:	        ts_143019v050600p.pdf  
 *                                                                                                                                     
 *@Param:       byte0 Handler Id
 *                         						                                                                                                  
 *@Return:      NULL                                                                                                                                                                                                                                                            
 * 
 *@Author:      LJ / 2013.06.19
 *  
 *@History:     V0.0---Create by LJ / 2013.06.19
 *                        
 */	
void Toolkit_GetHanlderCapacity(void)
{
	jshort idata  apiMode = PopOperandStackShort();
	jbyte idata  index = PopOperandStackShort();
	
	handler_check_available(index, apiMode);

	if(ToolkitHandlerList[index].HandlerBufferSize == 0x00)
	{
		ThrowToolkitException(BAD_INPUT_PARAMETER_SIM);	
	}
 	
	PushOperandStackShort(ToolkitHandlerList[index].HandlerBufferSize);
}

#endif
