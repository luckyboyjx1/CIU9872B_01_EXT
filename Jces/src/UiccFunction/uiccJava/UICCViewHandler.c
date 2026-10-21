 /************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCViewHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-30

*Description :该文件主要用于实现基于TS102241 所描述的interface ViewHandler的功能 ，该接口主要包含处理Comprehension TLV Lis 数据的一些基本方法，The byte at offset 0 of a handler is the tag of the first Comprehension TLV
*
*History     :
	
author    time      version    description
xuyz   2013/10/30   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
#include    "UICCToolkitAPI.h"
#include    "UICCToolkitConstants.h"
#include "Interpreter.h"
//#include "VMAccessEeprom.h"
#include "ToolkitAPIPublic.h"
#include "platform_api.h"

#if UICC__FUNCTION__UICCAPI
/**
*@function   : Toolkit_getValueShort
*
*@description: Gets a short from the last TLV element which has been found in the handler.
*
*@param      :valueOffset       the offset of the short to return in the TLV element
*
*@return     : element value (1 short)
*
*@exception  :ToolkitException  with the following reason codes:
*            :<li> HANDLER_NOT_AVAILABLE  if the handler is busy
*            :<li> UNAVAILABLE_ELEMENT  in case of unavailable TLV element
*            :<li> OUT_OF_TLV_BOUNDARIES  if  valueOffset  is out of the current TLV 
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz@20131030
*
*@others     :使用该函数的其他一些限制条件
*/
void UICCToolkit_getValueShort(void)
{
	BYTE value1,value2;
	
	jshort valueOffset = PopOperandStackShort();
    
	jbyte handlerIndex = PopOperandStackShort();

	handler_check_available(handlerIndex, UICCAPI_Mode);

  	value1 = Toolkit_getValueBytePublic(handlerIndex,valueOffset, UICCAPI_Mode);

	value2 = Toolkit_getValueBytePublic(handlerIndex,valueOffset+1, UICCAPI_Mode);

	PushOperandStackShort(  (jshort)((((WORD)value1<<8)&0xFF00)|value2)  )  ;
} 
#endif


