/*
*@HISTORY 
             @V1.0 :create 
			 @V1.1 :优化代码,增加公共函数Toolkit_getFirstByteOfSpecifiedTag	Toolkit_subGetOtherDataLength  Toolkit_copyTagDataToDstBuffer @20131012 by xuyz
*            @V1.2 :删除文件中的无效代码 
*/
#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "SimApi.h"
#include "fileAction.h"
//#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
//#include "FunctionSwitch.h"
#include "platform_api.h"
#if UICC__FUNCTION__SIMAPI

 /**
     * Returns the general result byte of the Proactive command.
     * If the element is available it becomes the TLV selected.
     *
     * @return general result of the command (first byte of Result TLV in Terminal Response)
     *
     * @exception ToolkitException with the following reason codes: <ul>
     *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Result TLV element
     *      <li><code>OUT_OF_TLV_BOUNDARIES</code> if the general result byte is missing in the Result Simple TLV</ul>
 */
BYTE code GeneralResultConstants[33] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x10,0x11,0x12,0x13,0x14,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x08,0x3B};//08 ,3B 仅UICCAPI才会存在
 
void Toolkit_getGeneralResult(void)
{
	jbyte idata result,i;
	jshort idata apiMode;
	BYTE idata counter;
	
	apiMode = PopOperandStackShort();

	result =  Toolkit_getFirstByteOfSpecifiedTag(Index_ProResHandler ,TAG_RESULT, apiMode);

	counter = ((apiMode == UICCAPI_Mode)?33:31);
	
	for(i=0;i<counter;i++)
	{
		if(result == GeneralResultConstants[i])
		{
			break;
		}
	}

	if(i == counter)
	{
		UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode);//TOOLKITEXCEPTION_OUT_OF_TLV_BOUNDARIES
	}	
 
	PushOperandStackShort((jbyte)result); 	
}

/**
     * Returns the length of the additional information field from the first
     * Result TLV element of the current response data field.
     * If the element is available it becomes the TLV selected.
     *
     * @return additional information length
     *
     * @exception ToolkitException with the following reason codes: <ul>
     *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Result TLV element</ul>
*/
void Toolkit_getAdditionalInformationLength(void) 
{
	PushOperandStackShort(Toolkit_subGetOtherDataLength(TAG_RESULT));
}

/**
     * Copies a part of the additional information field from the first Result
     * TLV element of the current response data field.
     * If the element is available it becomes the TLV selected.
     *
     * <p>
     * Notes:<ul>
     * <li><em>If </em><code>dstOffset</code><em> or </em><code>dstLength</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
     * <em> exception is thrown and no copy is performed.</em>
     * <li><em>If </em><code>dstOffset+dstLength</code><em> is greater than </em><code>dstBuffer.length</code><em>, the length
     * of the </em><code>dstBuffer</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
     * and no copy is performed.</em> 
     * </ul> 
     *
     * @param dstBuffer a reference to the destination buffer
     * @param dstOffset the position in the destination buffer
     * @param dstLength the data length to be copied
     *
     * @return <code>dstOffset+dstLength</code>
     *
     * @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
     * @exception ArrayIndexOutOfBoundsException if copyAdditionalInformation would cause access of data outside array bounds.
     * @exception ToolkitException with the following reason codes: <ul>
     *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Result TLV element
     *	    <li><code>OUT_OF_TLV_BOUNDARIES</code> if <code>dstLength</code> is greater than 
     *	    the value field of the available TLV</ul>
*/
void Toolkit_copyAdditionalInformation(void)
{
	PushOperandStackShort(Toolkit_copyTagDataToDstBuffer(TAG_RESULT,0x01));
}
/**
 * Returns the item identifier byte value from the first Item Identifier TLV
 * element of the current response data field.
 * If the element is available it becomes the TLV selected.
 *
 * @return item identifier
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Item Identifier TLV element
 *	    <li><code>OUT_OF_TLV_BOUNDARIES</code> if the item identifier byte is missing in the Item Identifier Simple TLV</ul>
 */
void Toolkit_getItemIdentifier(void)
{
	jbyte idata index;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	index = PopOperandStackShort();

	PushOperandStackShort((jbyte)(Toolkit_getFirstByteOfSpecifiedTag(index,TAG_ITEM_IDENTIFIER,apiMode)));
}

/**
 * Returns the text string length value from the first Text String TLV
 * element of the current response data field. The Data Coding Scheme byte
 * is not taken into account.
 * If the element is available it becomes the TLV selected.
 *
 * @return text string length
 *
 * @exception ToolkitException with the following reason codes: <ul>
 *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Text String TLV element</ul>
 */
void Toolkit_getTextStringLength(void)
{
	PushOperandStackShort(Toolkit_subGetOtherDataLength(TAG_TEXT_STRING));
}

/**
* Returns the data coding scheme byte from the first Text String TLV
* element of the current response data field.
* If the element is available it becomes the TLV selected.
*
* @return text string coding scheme
*
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Text String TLV element
*	    <li><code>OUT_OF_TLV_BOUNDARIES</code> if the Text String TLV is present with a length 
*	    of 0 (no DCS byte)</ul>
*/

void Toolkit_getTextStringCodingScheme(void)
{
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	PushOperandStackShort((jbyte)(Toolkit_getFirstByteOfSpecifiedTag(Index_ProResHandler,TAG_TEXT_STRING, apiMode)));
}

    /**
     * Copies the text string value from the first Text String TLV element of the
     * current response data field. The Data Coding Scheme byte is not copied.
     * If the element is available it becomes the TLV selected.
     *
     * @param dstBuffer a reference to the destination buffer
     * @param dstOffset the position in the destination buffer
     *
     * @return <code>dstOffset</code> + length of the copied value
     *
     * @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
     * @exception ArrayIndexOutOfBoundsException if <code>dstOffset or dstOffset + (length of the TextString to be copied, 
     *      without the Data Coding Scheme included)</code>, as specified for the returned value, would cause access outside array bounds.
     * @exception ToolkitException with the following reason codes: <ul>
     *      <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable Text String TLV element</ul>
     */
//jshort Toolkit_copyTextString(objref_t dstBuffer, jshort dstOffset)
void Toolkit_copyTextString(void)
{
	jref dstBuffer; 
	jshort dstOffset;
	
	jshort copylength;
		
	dstOffset =  PopOperandStackShort();
	dstBuffer =  PopOperandStackShort();

 	copylength = Toolkit_subGetOtherDataLength(TAG_TEXT_STRING);

	check_bounds_interval_buffer(dstBuffer, dstOffset, copylength);	  //异常：ArrayIndexOutOfBoundsException

	Toolkit_copyHandlerBufferDataToDstBuffer(Index_ProResHandler,ToolkitHandlerList[Index_ProResHandler].CurrentTLVValueOffset+1,dstBuffer,dstOffset,copylength);
	
	PushOperandStackShort(dstOffset + copylength);
}
    
/**
 * Returns the channel indentifier value from the first Channel status TLV element<BR>
 * in the current Envelope data field. If the element is available it becomes the currently <BR>
 * selected TLV.
 *
 * @return channel identfier
 * @exception ToolkitException with the following reason codes: <ul>
 *        <li><code>UNAVAILABLE_ELEMENT</code> in case of unavailable TLV element
 *        <li><code>OUT_OF_TLV_BOUNDARIES</code> if the Simple TLV Channel Status length is equal to 0.</ul>
 */
//jbyte Toolkit_getChannelIdentifier(jbyte handlerindex)
void Toolkit_getChannelIdentifier(void)
{
	jbyte handlerindex;
	BYTE identifier;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	 
	handlerindex = PopOperandStackShort();
	
	identifier = Toolkit_getFirstByteOfSpecifiedTag(handlerindex,TAG_CHANNEL_STATUS,apiMode);

	PushOperandStackShort(identifier&0x07);
}
    
/** 
* Copies parts of the Channel data string field from the first Channel data TLV element<BR>
* of the current response data field. If the element is available it becomes the currenly<BR>
* selected TLV.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>dstOffset</code><em> or </em><code>dstLength</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no copy is performed.</em>
* <li><em>If </em><code>dstOffset+dstLength</code><em> is greater than </em><code>dstBuffer.length</code><em>, the length
* of the </em><code>dstBuffer</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no copy is performed.</em> 
* </ul> 
*
* @param dstBuffer a reference to the destination buffer
* @param dstOffset the position in the destination buffer
* @param dstLength the data length to be copied
*
* @return <code>dstOffset</code> + <code>dstLength</code>
*
* @exception NullPointerException if <code>dstBuffer</code> is <code>null</code>
* @exception ArrayindexOutOfBoundsException if <code>dstOffset</code> or <code>dstLength</code> or <BR>
*            both would cause access outside array bounds
* @exception ToolkitException with the following reason codes: <ul>
*    <li>UNAVAILABLE_ELEMENT in case of unavailable Result TLV element
*    <li>OUT_OF_TLV_BOUNDARIES if dstLength is greater than the value field of the available TLV</ul>
*/
//jshort Toolkit_copyChannelData(objref_t dstBuffer, jshort dstOffset, jshort dstLength)
void Toolkit_copyChannelData(void)
{
	PushOperandStackShort(Toolkit_copyTagDataToDstBuffer(TAG_CHANNEL_DATA,0x00));
}
#endif
