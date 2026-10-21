/*
*@Version: V1.1
*
*@history: V1.0 :create file, 该文件用于实现SIM API中sim.toolkit.EditHandler的相关功能. 由于sim api中继承了EditHandler的只有EnvelopeResponseHandler(Index_ProactiveHandler=0x01)和ProactiveHandler(Index_EnveResHandler=0x02)
*          V1.1 :优化API函数Toolkit_appendTLV_2 ,Toolkit_appendTLV_3 ,将其部分功能相同部分合并到新的函数Toolkit_appendSimpleTLV中,
                 优化API函数Toolkit_appendTLV_1，Toolkit_appendTLV_4，将其功能相同部分合并到新的函数Toolkit_appendBuf@20131011 by xuyz

		   V1.2  :删除文件中的无效代码 @20131015
*/
#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "fileAction.h"
#include "uicc_main.h"
//#include "transaction.h"
#include "SimApi.h"
//#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
#include "platform_api.h"

#if UICC__FUNCTION__SIMAPI

/**
* @See: sim.toolkit.EditHandler.clear()
*
* Clears the TLV list of an EditHandler and resets the current TLV selected.*
*
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy</ul>
*/
void Toolkit_clear(void)
{
	jbyte idata handlerIndex;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	handlerIndex = PopOperandStackShort();
	
	handler_check_available(handlerIndex, apiMode);
		
	ToolkitHandlerList[handlerIndex].TLVListLength = 0x00;

	ClearHandlerCurrentTLV(handlerIndex);    //清空当前TLV信息	
}

/**
* @See: sim.toolkit.EditHandler.appendArray()
*
* Appends a buffer into the EditHandler buffer.
* A successful append does not modify the TLV selected.
* The TLV list structure of the handler should be maintained by the applet in the
* appended array (e.g. the length of the TLV element should be coded according to ISO 7816-6),
* if the TLV manipulation methods are to be used afterwards with the handler.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>offset</code><em> or </em><code>length</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no append is performed.</em>
* <li><em>If </em><code>offset+length</code><em> is greater than </em><code>buffer.length</code><em>, the length
* of the </em><code>buffer</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no append is performed.</em> 
* </ul> 
*
* @param buffer the buffer containing data for copy
* @param offset the offset in the buffer
* @param length the value length of the buffer
*
* @exception NullPointerException if <code>buffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if append would cause access of data outside array bounds
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_OVERFLOW</code> if the EditHandler buffer is to small to append the requested data
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy</ul>
*/
void Toolkit_appendArray(void)
{
	BYTE idata *dest;		
	jbyte idata handlerIndex; 
	jref  idata arrayref; 
	jshort idata offset; 
	jshort idata length;
	UWORD xdata arrayaddr;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	length = PopOperandStackShort();
    
	offset = PopOperandStackShort();
    
	arrayref = PopOperandStackShort();     //可能为RAM，可能为E2
    
	handlerIndex = PopOperandStackShort(); //这里的handlerIndex只可能为 ENVELOPERESPONSEHANLDERINDEX 或 PROACTIVEHANLDERINDEX
	
	handler_check_available(handlerIndex, apiMode);
	 	
	arrayaddr = GetValidArrayDataAddr(arrayref,offset,length);//GetArrayBufferAddress(arrayref)+offset;

	if((ToolkitHandlerList[handlerIndex].TLVListLength +length) > ToolkitHandlerList[handlerIndex].HandlerBufferSize)
	{	
		UiccOrSimThrowToolkitException(HANDLER_OVERFLOW_SIM, apiMode);
		
		return;
	}
	
	dest = (BYTE *)&ToolkitHandlerList[handlerIndex].HandlerBuffer[ToolkitHandlerList[handlerIndex].TLVListLength]; //由于sim api中继承了EditHandler的只有EnvelopeResponseHandler(Index_ProactiveHandler=0x01)和ProactiveHandler(Index_EnveResHandler=0x02)，而ToolkitHandlerList[Index_ProactiveHandler].HandlerBuffer = ResData+3， ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer = RecData;所以不会是E2

	VMWriteEepromOrRam(dest,(BYTE *)arrayaddr, length);
    
	ToolkitHandlerList[handlerIndex].TLVListLength +=length;
}
/*
* See: sim.toolkit.EditHandler.appendTLV(byte, byte[], short, short)
*      sim.toolkit.EditHandler.appendTLV(byte, byte, byte[], short, short)
*
*@description: 从某个buf中获取特定偏移的数据进行特定tag的拼装
*
*@param      :handlerIndex 
*@param      :tag
*@param      :objref_t* src:源数据buf
*@param      :valueOffset
*@param      :valueLength
*@param      :mode :0x00：不需要增加/0x01:在拼装buf前，先增加某个特定的value
*@param      :specifiedValue
*
*@return     :void
*
*@author     :create by xuyz
*
*@history    :V1.0 :Create by xuyz
* 
*/
void Toolkit_appendBuf(jbyte handlerIndex,jbyte tag,jref srcBuf,WORD valueOffset, WORD valueLength,BYTE mode, BYTE specifiedValue, BYTE apiMode)
{
	BYTE  * dest;
	WORD idata datalen;	
	jshort idata i;

	handler_check_available(handlerIndex, apiMode);

 	check_bounds_interval_buffer(srcBuf,valueOffset,valueLength); 
	
	if((valueLength + mode) >255)
	{
	   UiccOrSimThrowToolkitException(BAD_INPUT_PARAMETER_SIM, apiMode);
	   return;
	}

	datalen = (ToolkitHandlerList[handlerIndex].TLVListLength +valueLength);
	
	if ((valueLength+mode)> 0x7F)
	{
		datalen += (3+mode);	
	} 
	else 
	{
	    datalen += (2+mode);
	}
	
	if(datalen > ToolkitHandlerList[handlerIndex].HandlerBufferSize)
	{	
		UiccOrSimThrowToolkitException(HANDLER_OVERFLOW_SIM, apiMode);
		return;
	}

	i = 0x00;

	if ((valueLength+mode) > 0x7F)
	{
		tempBuf[i++] = tag;
		tempBuf[i++] = (BYTE)TLV_LENGTH_2BYTES;		
	} 
	else
	{
		tempBuf[i++] = tag;
	}

	tempBuf[i++] = (valueLength+mode);

	if(mode == 0x01)
	{
		tempBuf[i++] = specifiedValue;
	}

	CopyArrayDataToDest((BYTE *)&(tempBuf[i]),srcBuf,valueOffset,valueLength);  //scr可能为RAM，可能为E2

	i += valueLength;	

	dest = (BYTE *)&ToolkitHandlerList[handlerIndex].HandlerBuffer[ToolkitHandlerList[handlerIndex].TLVListLength]; //由于sim api中继承了EditHandler的只有EnvelopeResponseHandler(Index_ProactiveHandler=0x01)和ProactiveHandler(Index_EnveResHandler=0x02)，而ToolkitHandlerList[Index_ProactiveHandler].HandlerBuffer = ResData+3， ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer = RecData;所以不会是E2

	VMWriteEepromOrRam((BYTE *)dest, tempBuf, i); //从逻辑来说，只有当handlerIndex==Index_BERTLVEditHandler时，dest才为E2，另外两种情况时，dest为RAM

	ToolkitHandlerList[handlerIndex].TLVListLength = datalen;
}
/*
*@See: sim.toolkit.EditHandler.appendTLV(byte, byte)
*      sim.toolkit.EditHandler.appendTLV(byte, byte, byte)
*
*@description:Toolkit_appendTLV_1和Toolkit_appendTLV_4调用的公共函数
*
*@param      : mode ---> 0(Toolkit_appendTLV_1) or 1(Toolkit_appendTLV_4)
*
*@return     :void
*
*@author     :create by xwh
*
*@hisotry    :V1.0: create by xwh 20171216
*/
void ToolkitappendTLVPublic1or4(BYTE mode)
{
	jbyte idata handlerIndex; 
	jbyte idata tag; 
	jbyte idata value1;
	jref idata value2; 
	jshort idata valueOffset; 
	jshort idata valueLength;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	valueLength = PopOperandStackShort();
    
	valueOffset = PopOperandStackShort();
    
	value2 = PopOperandStackShort();
	
	if(mode == 1)
	{
		value1 = PopOperandStackShort();
	}
	else
	{
		value1 = 0;
	}
    
	tag = PopOperandStackShort();
    
	handlerIndex = PopOperandStackShort();

    Toolkit_appendBuf(handlerIndex,tag,value2,valueOffset,valueLength,mode,value1, apiMode);
}
/**
* @See: sim.toolkit.EditHandler.appendTLV(byte, byte[], short, short)
*
* Appends a TLV element to the current TLV list (byte array format).
* A successful append does not modify the TLV selected.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>valueOffset</code><em> or </em><code>valueLength</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code>
* <em> exception is thrown and no append is performed.</em>
* <li><em>If </em><code>valueOffset+valueLength</code><em> is greater than </em><code>value.length</code><em>, the length
* of the </em><code>value</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no append is performed.</em> 
* </ul>
*
* @param tag the tag of the TLV to append, including the Comprehension Required flag
* @param value the buffer containing the TLV value
* @param valueOffset the offset of the TLV value in the buffer
* @param valueLength the value length of the TLV to append
*
* @exception NullPointerException if <code>value</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if append would cause access of data outside array bounds
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_OVERFLOW</code> if the EditHandler buffer is to small to append the requested data
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>BAD_INPUT_PARAMETER</code> if <code>valueLength</code> is greater than 255</ul>
*/
void Toolkit_appendTLV_1(void)
{
	ToolkitappendTLVPublic1or4(0);
}
/*
*@See: sim.toolkit.EditHandler.appendTLV(byte, byte)
*      sim.toolkit.EditHandler.appendTLV(byte, byte, byte)
*
*@description:拼装simple TLV,即 TAG 01 VALUE or TAG 02 VALUE1 VALUE2
*
*@param      : handlerIndex
*@param      : tmp:存放tlv
*@param      :length,TLV length
*
*@return     :void
*
*@author     :create by xuyz
*
*@hisotry    :V1.0: create by xuyz
*/
void Toolkit_appendSimpleTLV(jbyte handlerIndex,BYTE *tmp, BYTE length, BYTE apiMode)
{
	BYTE idata *dest;
	WORD idata datalen;

	handler_check_available(handlerIndex, apiMode);
	
	datalen = ToolkitHandlerList[handlerIndex].TLVListLength + length;

	if(datalen > ToolkitHandlerList[handlerIndex].HandlerBufferSize)
	{	
		UiccOrSimThrowToolkitException(HANDLER_OVERFLOW_SIM, apiMode);
		return;
	}
	
	dest = (BYTE *)(ToolkitHandlerList[handlerIndex].HandlerBuffer+ ToolkitHandlerList[handlerIndex].TLVListLength); //由于sim api中继承了EditHandler的只有EnvelopeResponseHandler(Index_ProactiveHandler=0x01)和ProactiveHandler(Index_EnveResHandler=0x02)，而ToolkitHandlerList[Index_ProactiveHandler].HandlerBuffer = ResData+3， ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer = RecData;所以不会是E2

	VMWriteEepromOrRam(dest, tmp, length); //从逻辑来说，只有当handlerIndex==Index_BERTLVEditHandler时，dest才为E2，另外两种情况时，dest为RAM
	
	ToolkitHandlerList[handlerIndex].TLVListLength +=length;	
}
/*
*@See: sim.toolkit.EditHandler.appendTLV(byte, byte)
*      sim.toolkit.EditHandler.appendTLV(byte, byte, byte)
*
*@description:Toolkit_appendTLV_2和Toolkit_appendTLV_3调用的公共函数
*
*@param      : mode ---> 2(Toolkit_appendTLV_2) or 3(Toolkit_appendTLV_3)
*
*@return     :void
*
*@author     :create by xwh
*
*@hisotry    :V1.0: create by xwh 20171216
*/
void ToolkitappendTLVPublic2or3(BYTE mode)
{	
	jbyte idata  handlerIndex; 
	BYTE xdata tmp[0x04];
	
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	if(mode == 3)
	{
		tmp[3] = PopOperandStackShort();
	}
	
	tmp[2] = PopOperandStackShort();
    
    tmp[1] =  mode - 1;

	tmp[0] = PopOperandStackShort();
    
	handlerIndex = PopOperandStackShort();

	Toolkit_appendSimpleTLV(handlerIndex,tmp,mode+1, apiMode);		
}

/**
* See: sim.toolkit.EditHandler.appendTLV(byte, byte)
*
* Appends a TLV element to the current TLV list (1-byte element).
* This method is useful to add single byte elements as Item Identifier or Tone.
* A successful append does not modify the TLV selected.
*
* @param tag the tag of the TLV to append, including the Comprehension Required flag
* @param value the TLV value on 1 byte
*
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_OVERFLOW</code> if the EditHandler buffer is to small to append the requested data
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy</ul>
*/	
void Toolkit_appendTLV_2(void)
{	
	ToolkitappendTLVPublic2or3(2);
}

/**
* Appends a TLV element to the current TLV list (2-byte element)
* This method is useful to add double byte elements as Device Identities, Duration or
* Response Length. A successful append does not modify the TLV selected.
*
*
* @param tag the tag of the TLV to append, including the Comprehension Required flag
* @param value1 the 1st byte (msb) of the TLV value
* @param value2 the 2nd byte (lsb) of the TLV value
*
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_OVERFLOW</code> if the EditHandler buffer is to small to append the requested data
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy</ul>
*/
void Toolkit_appendTLV_3(void)
{
	ToolkitappendTLVPublic2or3(3);
}
/**
*@See: sim.toolkit.appendTLV(byte, byte, byte[], short, short)
*
* Appends a TLV element to the current TLV list (1 byte and a byte array format).
* A successful append does not modify the TLV selected.
*
* <p>
* Notes:<ul>
* <li><em>If </em><code>value2Offset</code><em> or </em><code>value2Length</code><em> parameter is negative an </em><code>ArrayIndexOutOfBoundsException</code><em> 
* exception is thrown and no append is performed.</em>
* <li><em>If </em><code>value2Offset+value2Length</code><em> is greater than </em><code>value2.length</code><em>, the length
* of the </em><code>value2</code><em> array an </em><code>ArrayIndexOutOfBoundsException</code><em> exception is thrown
* and no append is performed.</em> 
* </ul>
*
* @param tag the tag of the TLV to append, including the Comprehension Required flag
* @param value1 the first byte in the value field
* @param value2 the buffer containing the rest of the TLV field
* @param value2Offset the offset of the rest of the TLV field in the buffer
* @param value2Length the value length of the rest of the TLV field to append
*
* @exception NullPointerException if <code>value2</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException if append would cause access of data outside array bounds.
* @exception ToolkitException with the following reason codes: <ul>
*      <li><code>HANDLER_OVERFLOW</code> if the EditHandler buffer is to small to append the requested data
*      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy
*      <li><code>BAD_INPUT_PARAMETER</code> if <code>value2Length</code> is greater than 254</ul>
*/
void Toolkit_appendTLV_4(void)
{
	ToolkitappendTLVPublic1or4(1);
}
#endif
