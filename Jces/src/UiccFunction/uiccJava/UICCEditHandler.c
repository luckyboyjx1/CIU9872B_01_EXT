/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCEditHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-28

*Description :该文件主要用于实现基于TS102241 所描述的interface EditHandler的功能 ，该接口主要包含处理Handler数据的一些基本方法
*             该文件用于实现UICC API中uicc.toolkit.EditHandler的相关功能. 由于uicc api中继承了EditHandler的只有EnvelopeResponseHandler(Index_ProactiveHandler=0x01)和ProactiveHandler(Index_EnveResHandler=0x02)
*
*History     :
	
author    time      version    description
ZZJ   2013/10/28   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
//#include   "Interpreter.h"
#include   "ToolkitAPIPublic.h"
#include "platform_api.h"
#if UICC__FUNCTION__UICCAPI

/*@function:    UICCToolkitWriteBERTLVHandlerContext                                            
 * 
 *@descrip:     将c层的EditHandler和BERTLVHandeler的当前currentTLVOffset	currentTLVLgth,	TLVListLength等信息	写回到java层
 *
 *@see:	       uicc.framework.UICCSpclEditHandlerImpl
 *                                                                                                                                     
 *@param:		HandlerCntx：存储当前的TLVListLgth, currentTLVLgth, currentTLVOffset

 *				                                                                                                         
 *@return:      void                                                                                                                                                                                                                                                                    
 * 
 *@author:      ZZJ 20131121
 *  
 *@history:     V0.0---Create by ZZJ 20131121           
 */

void UICCToolkitWriteBERTLVHandlerContext(void)
{
	BYTE  xdata context[6];

	context[0] = ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength >> 8;

	context[1] = ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength & 0xFF;

	context[2] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVLength >> 8;

	context[3] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVLength & 0xFF;

	context[4] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVValueOffset >> 8;

	context[5] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVValueOffset & 0xFF;

	WritePageCacheEX((UWORD)G_BERTLVEditHandlerContext,context,6);	  //该函数可能写失败	
}

/*@function:    UICCToolkitCopyHandlerContextToNative                                            
 * 
 *@descrip:     将java层的EditHandler和BERTLVHandeler的当前currentTLVOffset	currentTLVLgth,	TLVListLength等信息拷贝到C层
 *
 *@see:	       
 *                                                                                                                                     
 *@param:       HandlerCntx ：存储当前TLV信息
 *@param:		HandlerBuffer：存储当前的TLVListLgth, currentTLVLgth, currentTLVOffset
 *@param       	Capacity     :当前handler的能力

 *				                                                                                                         
 *@return:      void                                                                                                                                                                                                                                                                    
 * 
 *@author:      ZZJ 20131121
 *  
 *@history:     V0.0---Create by ZZJ 20131121           
 */
void UICCToolkitCopyHandlerContextToNative(void)
{
	UWORD xdata handlerCntxAddr;
	BYTE  xdata context[6];
	
	jshort Capacity	 =  PopOperandStackShort();
    
	jref HandlerBuffer =  PopOperandStackShort();
    
	jref HandlerCntx =  PopOperandStackShort();

	handlerCntxAddr = GetArrayBufferAddress(HandlerCntx);  //HandlerCntx在uicc.framework.UICCSpclEditHandlerImpl中定义的一个E2数组

	VMReadBuf(context,(DWORD)handlerCntxAddr,6);

	G_BERTLVEditHandlerContext = handlerCntxAddr;

	ToolkitHandlerList[Index_BERTLVEditHandler].HandlerBuffer = (BYTE *)GetArrayBufferAddress(HandlerBuffer);

	ToolkitHandlerList[Index_BERTLVEditHandler].HandlerBufferSize = Capacity;
		
	ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength = ((short)context[0])<<8|context[1];
	
	ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVLength = ((short)context[2])<<8|context[3];
	
	ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVValueOffset = ((short)context[4])<<8|context[5];
	
	ToolkitHandlerList[Index_BERTLVEditHandler].HandlerAvailable = TRUE;		
}
/*@function:    UICCToolkitCopyHandlerContextToJava                                            
 * 
 *@descrip:     将c层的EditHandler和BERTLVHandeler的当前currentTLVOffset	currentTLVLgth,	TLVListLength等信息拷回到java层
 *
 *@see:	       
 *                                                                                                                                     
 *@param:		HandlerCntx：存储当前的TLVListLgth, currentTLVLgth, currentTLVOffset

 *				                                                                                                         
 *@return:      void                                                                                                                                                                                                                                                                    
 * 
 *@author:      ZZJ 20131121
 *  
 *@history:     V0.0---Create by ZZJ 20131121           
 */

void UICCToolkitCopyHandlerContextToJava(void)
{
	UWORD idata handlerCntxAddr;
	BYTE  xdata context[6];

	jref HandlerCntx  = PopOperandStackShort();

	handlerCntxAddr = GetArrayBufferAddress(HandlerCntx); //JAVA层定义的E2数组

	context[0] = ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength >> 8;

	context[1] = ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength & 0xFF;

	context[2] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVLength >> 8;

	context[3] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVLength & 0xFF;

	context[4] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVValueOffset >> 8;

	context[5] = ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVValueOffset & 0xFF;

	WritePageCacheEX((UWORD)handlerCntxAddr,context,6);	  //该函数可能写失败

	ToolkitHandlerList[Index_BERTLVEditHandler].HandlerAvailable = FALSE;		
}
/*
*@function:      UICCBERTLVHandlerGetValueLength
*
*@description :  获取BERTLVHandler的当前数据长度
*
*@param       :  void
*
*@return      :valueLength
*
*@authoe     :create by xuyz@20140116
*
*@history     :V1.0:create by xuyz
*/
void UICCBERTLVHandlerGetValueLength(void)
{
	if(ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVValueOffset == 0xFFFF)
	{
		UICCThrowToolkitException(UNAVAILABLE_ELEMENT);
	}
    
	PushOperandStackShort((jshort)ToolkitHandlerList[Index_BERTLVEditHandler].CurrentTLVLength);	
}

/*
*@function:      UICCBERTLVHandlerGetLength
*
*@description :  获取BERTLVHandler的当前数据长度
*
*@param       :  void
*
*@return      :valueLength
*
*@authoe     :create by xuyz@20140116
*
*@history     :V1.0:create by xuyz
*/
void UICCBERTLVHandlerGetLength(void)
{
	PushOperandStackShort((jshort)ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength);	
}

/*
*@function:      UICCBERTLVHandlerGetSize
*
*@description :  获取BERTLVHandler的有效数据长度
*
*@param       :  void
*
*@return      :dataSize
*
*@authoe     :create by xuyz@20140116
*
*@history     :V1.0:create by xuyz
*/
void UICCBERTLVHandlerGetSize(void)
{
	WORD idata length;

	length = ToolkitHandlerList[Index_BERTLVEditHandler].TLVListLength;

	if(length < 0x80)
	{
		length += 2;
	}
	else if(length <0x100)
	{
		length += 3;
	}
	else
	{
		length += 4;
	}
    
	PushOperandStackShort((jshort)length);	
}
/**
 *@fucntion    : UICCToolkit_appenddata
 *
 *@description :拼装数据,将数据添加到buffer
 *
 *@see         :uicc.toolkit.EditHandler.clear()
 *
 *@param       : handlerIndex
 *@param       : tmp    --- data
 *@param       : length --- 长度
 *
 *@return      :void
 *
 *@author      :create by ZZJ
 * 
 *@hisotry     :V1.0: create by ZZJ@20131030
*/
void UICCToolkit_appenddata(jbyte handlerIndex,BYTE *tmp, WORD length)
{
	BYTE idata *dest;

	handler_check_available(handlerIndex, UICCAPI_Mode);	
	
	dest = (BYTE *)(ToolkitHandlerList[handlerIndex].HandlerBuffer+ ToolkitHandlerList[handlerIndex].TLVListLength);//uicc api中继承了EditHandler的只有EnvelopeResponseHandler(Index_ProactiveHandler=0x01)和ProactiveHandler(Index_EnveResHandler=0x02)，而ToolkitHandlerList[Index_ProactiveHandler].HandlerBuffer = ResData+3， ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer = RecData;通过uicc.system.HandlerBuilder.buildTLVHandler()获取UICCSpclEditHandlerImpl，其index为0x04
	
	VMWriteEepromOrRam((BYTE *)dest, tmp, length); //从逻辑来说，只有当handlerIndex==Index_BERTLVEditHandler时，dest才为E2，另外两种情况时，dest为RAM

	ToolkitHandlerList[handlerIndex].TLVListLength +=length;	
}

/**
 *@fucntion    : Toolkit_settag
 *
 *@description : 往tempBuf拼装tag，valuelenght
 *
 *@see         :
 *
 *@param       : tag    --- tag
 *@param       : valuelenght --- 数据长度
 *
 *@return      : BYTE --- 偏移
 *
 *@author      :create by ZZJ
 * 
 *@hisotry     :V1.0: create by ZZJ@20131030
*/
BYTE UICCToolkit_appendtag(BYTE handlerIndex,BYTE tag, WORD Length)
{
	BYTE xdata tmp[3] ;
	BYTE idata len;

	tmp[0] = tag;

	if (Length > 0x7F)
	{		
		tmp[1] = 0x81;
		
		tmp[2] = Length;
		
		len = 3;		
	} 
	else
	{
		tmp[1] = Length;

		len = 2;
	}
		
	if((ToolkitHandlerList[handlerIndex].TLVListLength + Length + len) > ToolkitHandlerList[handlerIndex].HandlerBufferSize)
	{	
		UICCThrowToolkitException(HANDLER_OVERFLOW);
	}
	
	UICCToolkit_appenddata(handlerIndex, tmp, len);	
	
	return TRUE;	
}
/**
 *@fucntion    : Toolkit_appendTLV_3
 *
 *@description : Appends a TLV element to the current TLV list (2-byte element).
 * This method is useful to add a two bytes element.
 *
 *@see          : TS 102 241 :interface EditHandler
 *              uicc.toolkit.EditHandler.appendTLV(byte, short)
 *
 *@param tag   --- the tag of the TLV to append, including the Comprehension Required flag
 *@param value --- the 2 byte TLV value on 1 short
 *
 *@exception ToolkitException with the following reason codes: <ul>
 *      <li><code>HANDLER_OVERFLOW</code> if the EditHandler buffer is to small to append the requested data
 *      <li><code>HANDLER_NOT_AVAILABLE</code> if the handler is busy</ul>
 *
 *@author       : ZZJ
 *
 *@hisotry      : V1.0 :create by ZZJ@20131028
 */	
void UICCToolkit_appendTLV_3(void)
{	
	BYTE idata tmp[0x02];
	
	jshort value = PopOperandStackShort();
    
	jbyte tag = PopOperandStackShort();
    
	jbyte handlerIndex = PopOperandStackShort();

	tmp[0] = (BYTE)((value >> 8)&0xFF);
    
	tmp[1] = (BYTE)(value&0xFF);

	UICCToolkit_appendtag(handlerIndex, tag, 2);

	UICCToolkit_appenddata(handlerIndex, tmp, 2);		
}
/**
 *@fucntion    : Toolkit_appendTLV_5
 *
 *@description : Appends a TLV element to the current TLV list (3-byte element(1-byte,1-short))
 *               This method is useful to add three byte elements as Command details or Display parameters
 *
 *@see          : TS 102 241 :interface EditHandler
 *              uicc.toolkit.EditHandler.appendTLV(byte, byte, short)
 *
 * @param  tag    --- the tag of the TLV to append, including the Comprehension Required flag
 * @param  value1 --- the 1st byte (msb) of the TLV value
 * @param  value2 --- the 2nd and 3rd byte on 1 short of the TLV value
 *
 * @exception  ToolkitException  with the following reason codes: 
 *      <li> HANDLER_OVERFLOW if the EditHandler buffer is to small to append the requested data
 *      <li> HANDLER_NOT_AVAILABLE if the handler is busy
 *
 *@author       : ZZJ
 *
 *@hisotry      : V1.0 :create by ZZJ@20131028
 */
void UICCToolkit_appendTLV_5(void)
{
	BYTE xdata tmp[3];
	
	jshort value2 = PopOperandStackShort();
    
	jbyte value1 = PopOperandStackShort();
    
	jbyte tag = PopOperandStackShort();
    
	jbyte handlerIndex = PopOperandStackShort();

	tmp[0] = value1;
	tmp[1] = (BYTE)((value2 >> 8)&0xFF);
	tmp[2] = (BYTE)(value2&0xFF);

	UICCToolkit_appendtag(handlerIndex, tag, 3);

	UICCToolkit_appenddata(handlerIndex, tmp, 3);
}

/**
 *@fucntion    : Toolkit_appendTLV_6
 *
 *@description : ppends a TLV element to the current TLV list (4-byte element(2-short))
 * 				 This method is useful to add three byte elements as Text Attribute, ESN, or C-APDU.
 *
 *@see          : TS 102 241 :interface EditHandler
 *              uicc.toolkit.EditHandler.appendTLV(byte, short, short)
 *
 * @param  tag    --- the tag of the TLV to append, including the Comprehension Required flag
 * @param  value1 --- the 1st short (1st and 2nd byte) of the TLV value
 * @param  value2 --- the 2nd short (3rd and 4th byte) the TLV value
 *
 * @exception  ToolkitException  with the following reason codes: 
 *      <li> HANDLER_OVERFLOW if the EditHandler buffer is to small to append the requested data
 *      <li> HANDLER_NOT_AVAILABLE if the handler is busy
 *
 *@author       : ZZJ
 *
 *@hisotry      : V1.0 :create by ZZJ@20131028
 */
void UICCToolkit_appendTLV_6(void)
{
	BYTE xdata tmp[4];
	
	jshort value2 = PopOperandStackShort();
    
	jshort value1 = PopOperandStackShort();
    
	jbyte tag = PopOperandStackShort();
    
	jbyte handlerIndex = PopOperandStackShort();

	tmp[0] = (BYTE)((value1 >> 8)&0xFF);
	tmp[1] = (BYTE)(value1&0xFF);
	tmp[2] = (BYTE)((value2 >> 8)&0xFF);
	tmp[3] = (BYTE)(value2&0xFF);

	UICCToolkit_appendtag(handlerIndex, tag, 4);

	UICCToolkit_appenddata(handlerIndex, tmp, 4);
}
/**
 * @fucntion    : Toolkit_appendTLV_8
 *
 * @description : Appends a TLV element to the current TLV list (2 byte arrays format).
 *
 * @see          : TS 102 241 :interface EditHandler
 *              uicc.toolkit.EditHandler.appendTLV(byte, byte[], short, short, byte[], short, short)
 *
 * @param  tag          --- the tag of the TLV to append, including the Comprehension Required flag
 * @param  value1       --- the buffer containing the first part of the value field
 * @param  value1Offset --- the offset in value1 of the data to append 
 * @param  value1Length --- the length in value1 of the data to append
 * @param  value2       --- the buffer containing the second part of the value field
 * @param  value2Offset --- the offset in value2 of the data to append
 * @param  value2Length --- the length in value2 of the data to append
 *
 * @exception  NullPointerException            if  value1 or  value2 is  null
 * @exception  ArrayIndexOutOfBoundsException  if  value1Offset or  value1Length or both would cause access outside value1 array bounds, or if  value1Length is negative,
 *	                                           if  value2Offset or  value2Length or both would cause access outside value2 array bounds, or if  value2Length is negative. 
 * @exception  ToolkitException                with the following reason codes: 
 *      <li> HANDLER_OVERFLOW if the EditHandler buffer is to small to append the requested data
 *      <li> HANDLER_NOT_AVAILABLE if the handler is busy
 *      <li> BAD_INPUT_PARAMETER if  value1Length or  value2Lengthis greater than 255
 *
 * @author       : ZZJ
 *
 * @hisotry      : V1.0 :create by ZZJ@20131028
 */
void UICCToolkit_appendTLV_8(void)
{
	WORD idata datalen;	
	DWORD xdata src1,src2;
	
	jshort value2Length = PopOperandStackShort();
    
	jshort value2Offset = PopOperandStackShort();
    
	objref_t value2 = PopOperandStackShort();
    
	jshort value1Length = PopOperandStackShort();
    
	jshort value1Offset =PopOperandStackShort();
    
	objref_t value1 = PopOperandStackShort();
    
	jbyte tag = PopOperandStackShort();
    
	jbyte handlerIndex =PopOperandStackShort();

	if((value1Length > 0xFF)||(value2Length > 0xFF))
	{
		UICCThrowToolkitException(BAD_INPUT_PARAMETER);
	}

	src1 = GetValidArrayDataAddr(value1,value1Offset,value1Length); 
	
	src2 = GetValidArrayDataAddr(value2,value2Offset,value2Length);
	
	datalen = value1Length + value2Length;	      //value长度： value1Length value1 + value2Length value2

	UICCToolkit_appendtag(handlerIndex, tag, datalen);	

	UICCToolkit_appenddata(handlerIndex,(BYTE *)src1, value1Length);

	UICCToolkit_appenddata(handlerIndex,(BYTE *)src2, value2Length);
}
#endif
