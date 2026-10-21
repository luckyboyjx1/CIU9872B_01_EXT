/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   Load.c
*
* Description: 该文件主要用于存放实现 安全域相关的函数
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0
*
************************************************************************************************************/
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
#include "Scp81Connection.h"
#include "FunctionSwitch.h"
#include "Interface_AnnexB.h"
#include "Interface_TimerModule.h"
#include "TlsMainFlow.h"
#include "HttpProcess.h"
#include "cardManager.h"
#include "GPAnnexbAdminSessionPara.h"
#include "FileAction.h"
#include "Stkalert.h"
#include "Stkhead.h"
#include "anomaly.h"
#include "ToolkitApiPublic.h"
#include "ToolkitRfm.h"
#include "BIPModuleProcess.h"
#include "Object.h"
#include "Gsmpublic.h"
#include "interpreter.h"
#include "Exception.h"
#include "fileSystem.h"
#include "TimerModuleProces.h"
#include "platform_api.h"
#include "ExpandedManage.h"
#include "firewall.h" //warning checkPrevContextAccess
#include "utilMethod.h" //warning check_bounds_interval
#include "ob_manager.h" //warning locateObjectInMemory
#include "opFunction.h"

//#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK

SCP81Connection xdata G_SCP81Connection;


/*
*@funtion    :NM_scp81ConnectionOpen
*
*@description:第三方应用 调用标准包uicc.connection 的api“SCP81Connection.open()”时， 本地实现打开scp81通道的功能
*
@throws ConnectionException with the following reason code:
*         <ul>
*              <li><code>INVALID_STATE</code> if the connection is already opened.
*              <li><code>INVALID_PARAMETERS</code> if the triggering parameters are not correct or if
*                   the Content-Type header value defined in the parameters is reserved (for example for
*                   RAM/RFM over HTTP or for SCWS remote administration).
*              <li><code>SCP81_KEY_NOT_FOUND</code>if this requested SCP81 key could no be found
*                    in the associated (directly or indirectly) Security Domain of the calling application. </li>
*              <li><code>INVALID_HEADER_FIELD</code>if Content-Type header field does not conform
*                    with the rules defined in this specification. </li>
*         </ul>
*
* @throws ArrayIndexOutOfBoundsException if <code>parameterOffset</code> and <code>parameterLength</code> are causing access
*         outside of <code>triggeringParameters</code> array bounds or
* if <code>contentTypeOffset</code> and <code>contentTypeLength</code> are causing access
*         outside of <code>contentType</code> array bounds.
*
* @throws NullPointerException if <code>triggeringParameters</code>
* or <code>contentType</code> are equal to <code>null</code>
*@param      :void
*            
*@return     :void
*            
*@author     :created by jiangqb@20190626
*            :modify by jiangqb@20200603 增加异常处理，联通测试
*            :modify by xuxiaojun@20200619 调整合法性检查的位置，避免执行了写文件操作之后才发现某些参数非法; 去掉HTTP_TriggerPara_ContentType_Index记录，该值与SCP81_CONTENTTYPE_REQUEST_VALUE是一致的
*            :modify by xuxioajun@20200716  全局变量的赋值不能放在有效触发之前，否则可能会冲掉当前正在进行的HTTP会话的对应全局变量值
*
*修改后包含了status：
性能测试：
10次--425MS---20191204
100次--420MS---20191204
300次--420MS---20191204
500次--MS---20191204
1000次--MS---20191204
2000次--
3000次--MS---20191204
*
*/
void NM_scp81ConnectionOpen(void)
{
	WORD idata triggerParaBuf, triggerParaOffSet, triggerParaLen, contentTypeBuf, contentTypeOffSet, contentTypeLen;
	BYTE idata triggeredAppId, triggeredSD;
	UWORD xdata triggerParaAddr, contentTypeAddr;
	
	contentTypeLen = PopOperandStackShort();
	contentTypeOffSet = PopOperandStackShort(); 
	contentTypeBuf =  PopOperandStackRef();
	
	triggerParaLen = PopOperandStackShort(); 
	triggerParaOffSet = PopOperandStackShort(); 
	triggerParaBuf = PopOperandStackRef();
	
	G_SCP81Connection.SCP81ConnectionStatusEventRef = PopOperandStackRef();

	if ((triggerParaBuf == REF_NULL) || (contentTypeBuf == REF_NULL))
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}

	contentTypeAddr = locateObjectInMemory(contentTypeBuf);
		
	check_bounds_interval(MEMtPF8(contentTypeAddr), contentTypeOffSet, contentTypeLen);
	
	if(contentTypeLen != sizeof(SCP81_CONTENTTYPE_REQUEST_VALUE))
	{
		throw_error(CONNECTIONEXCEPT_INVALID_HEADER_FIELD);
	}

	contentTypeAddr	= U32_A_U16(get_array_data(MEMtPN8(contentTypeAddr)) , contentTypeOffSet);

	VMReadBuf(tempBuf, contentTypeAddr, contentTypeLen);

	if(EQUAL != MemcmpWithoutUpperOrLower(tempBuf, (BYTE*)&SCP81_CONTENTTYPE_REQUEST_VALUE[0], sizeof(SCP81_CONTENTTYPE_REQUEST_VALUE))) // INVALID_HEADER_FIELD if Content-Type header field does not conform with the rules defined in this specification.
	{
		throw_error(CONNECTIONEXCEPT_INVALID_HEADER_FIELD);
	}

	triggerParaAddr = locateObjectInMemory(triggerParaBuf);
	
	check_bounds_interval(MEMtPF8(triggerParaAddr), triggerParaOffSet, triggerParaLen);
		
	triggerParaAddr	= U32_A_U16(get_array_data(MEMtPN8(triggerParaAddr)) , triggerParaOffSet);

	if (!HttpsSessionFileCheck())
	{
		throw_error(SW_FILE_NOT_FOUND);
	}

	if(G_TrigerToolkitRef == NULL_REF)  //表示当前是通过select方式选择应用，然后给应用发送指令触发HTTP API,立即开启会话
	{
		triggeredAppId = (getChannelContext(G_CurSelectedChannel)&0xFF);
	}
	else       //这种方式下等所有的触发完成之后再开启会话
	{
		triggeredAppId = GetAppIdByRef(G_TrigerToolkitRef);  //
	}

	if(G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS) //表示当前已经打开了一个http会话
	{
		#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
		HTTPSecondSessionParaHold(triggerParaAddr, 0x00, triggerParaLen, HTTPTriggeredBySCP81Connection, triggeredAppId);
		#else
		throw_error(SW_CONDITIONS_NOT_SATISFIED);//SW_CONDITIONS_NOT_SATISFIED if the request could not be processed (e.g. if no SCP81 session could be established).
		#endif
		return;
	}

	triggeredSD = GetHTTPTriggeredSD(triggeredAppId);
	
#if !SE_ONLY
	if(triggeredSD == ILLEGAL_APPID)
	{
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}
#endif

	if (!HTTPSAnalyseTriggerPara(triggeredSD, triggerParaAddr)) // INVALID_PARAMETERS if the triggering parameters are not correct
	{
		throw_error(CONNECTIONEXCEPT_INVALID_PARAMETERS);
	}
	
	G_HTTPSStatus.TriggeredAppId = triggeredAppId;

	G_HTTPSStatus.TriggeredSD = triggeredSD;

	if (TLSGetPSK(NULL) == 0x00)
	{
		throw_error(CONNECTIONEXCEPT_SCP81_KEY_NOT_FOUND); // SCP81_KEY_NOT_FOUND if this requested SCP81 key could no be found in the associated (directly or indirectly) Security Domain of the calling application.
	}

	G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredBySCP81Connection;
	
	G_HTTPSStatus.HTTPIsResume = FALSE;    //在这里增加该变量的初始化，避免当前是在重试过程中出现应用触发该API
#if !SE_ONLY
	G_HttpExecutingProfileIndex = G_CurExecutingProfileIndex;
#endif
	if(G_TrigerToolkitRef == NULL_REF)
	{
		HTTPSessionStart();
	}

}

/*
*@funtion    :NM_scp81ConnectionClose
*
*@description:第三方应用 调用标准包uicc.connection 的api“SCP81Connection.close()”时， 本地实现关闭scp81通道的功能
*			 
* Close the SCP81 Connection. The calling application will be notified once the connection is closed. This method is non-blocking.
*
* @throws ConnectionException with the following reason code: INVALID_STATE if the connection is not open.
*         
*
*
*@param      :void
*            
*@return     :void
*
*@author     :created by jiangqb@20190626
*            :modify by jiangqb@20200603 增加异常处理，联通测试
*            :modify by xuxiaojun@20200620 用TLSHandProcess统一处理，方案统一，因为NM_scp81ConnectionOpen也是用TLSHandProcess来处理的
*			 :modify by jiangqb@20200629	本函数的调用流程中，后续不用开启timer，所以不用 TLS_SEND_ALERT_DATA_STATE与上0x80
*/
void NM_scp81ConnectionClose(void)
{
	G_SCP81Connection.SCP81ConnectionStatusEventRef = PopOperandStackRef(); 	//D9
	
	if((G_BIPModuleStatus.channelDataType != ChannelDataType_HTTPS) || (G_HTTPSStatus.HTTPTriggerMode != HTTPTriggeredBySCP81Connection))//INVALID_STATE if the connection is not open.
	{
		throw_error(CONNECTIONEXCEPT_INVALID_STATE);
	}
	
	G_SCP81Connection.scp81ConnectionStatus = SCP81_CONNECTION_CLOSED; //表示当前是通过调用接口SCP81Connection.close()关闭链接，此时需要将状态CONNECTION_CLOSED通知给应用

	setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_WARNING);	//本函数都是正常关闭通道，不需要开启timer再次开启通道 (scp81api测试\RAM\T_000_HTTP_InstallBipApplet_underISD_Normal)
		
	TLSHandProcess(0, 0);
}
/*
*@funtion    :NM_scp81CopyReceivedData
*
*@description:第三方应用 调用标准包uicc.connection 的api“DataReceived.copyReceivedData()”时， 本地实现把收到的明文数据传入指定的Buf
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20180612
*            :modify by jiangqb@20200603 增加异常处理，联通测试
*/
void NM_scp81CopyReceivedData(void)
{
	WORD idata DestBuf, DestOff, DestLen, Off, perLen, allLen, readLen;
	
	UWORD xdata fileAddr, DestBufAddr;
	
	SDWORD xdata srcOffSet;		//int value
		
	DestLen = 	PopOperandStackRef();
	DestOff =  PopOperandStackRef(); 
	DestBuf =  PopOperandStackRef();
	srcOffSet = PopOperandStackInt();

	if(DestBuf == REF_NULL)
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}

	DestBufAddr = locateObjectInMemory(DestBuf);

	check_bounds_interval(MEMtPF8(DestBufAddr), DestOff, DestLen);

	DestBufAddr	= U32_A_U16(get_array_data(MEMtPN8(DestBufAddr)) , DestOff);

//	fileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPSCommandData);
	fileAddr = (UWORD)HTTP_CommandData_ADDR_START;

//	allLen = ReadTFFileWord(fileAddr);
	allLen = G_HTTP_CommandData_Length;
	
	//INVALID_PARAMETER: if the requested srcOffset could not be reached.
	if ((srcOffSet < 0) || (srcOffSet > allLen))		//源数据越界 或者 想要读取的数据大于源数据的长度
	{
		throw_error(CONNECTIONEXCEPT_INVALID_PARAMETERS);
	}
	
	//OUT_OF_DATA_BOUNDARIES: if srcOffset or dstOffset or dstLength would cause access outside received data.
	if ((srcOffSet+DestLen) > allLen)	
	{
		throw_error(CONNECTIONEXCEPT_OUT_OF_DATA_BOUNDARIES_scp81);
	}
			
	Off = srcOffSet;		//从此处开始读取数据
		
	readLen = 0;			//已读取数据
	
	perLen = 0xFF;			//循环每次读取数据的长度
	
//	while(readLen < DestLen)
//	{
//		if ((readLen+perLen) > DestLen)
//		{
//			perLen = DestLen - readLen;
//		}	

////		ReadTFBuf(fileAddr, 2+Off, RecData, perLen);	
//		ReadEeprom(RecData, fileAddr + 2 + Off, perLen);
//		
//		VMWriteBuf(DestBufAddr+DestOff+readLen, RecData, perLen);
//		
//		readLen += perLen;
//		
//		Off += perLen;
//	}

	//数据不重叠，可以调用WriteTFFileNonAtomic
	WriteEepromNonAtomic(DestBufAddr+DestOff,(BYTE *)(fileAddr + 2 + Off),DestLen);//modify by xuweihua@20210625: 优化空间， EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
	
	G_SCP81Connection.scp81CopyDataRemainLen -= DestLen;		//应用有可能分多次获取数据
	
	if (G_SCP81Connection.scp81CopyDataRemainLen == 0)
	{
//		ClearFileFirstWordAddr(fileAddr);
		G_HTTP_CommandData_Length = 0x00; 
	}
	
	PushOperandStackShort((WORD)(DestOff+DestLen));
}

/*
*@funtion    :NM_scp81ConnectionSend
*
*@description:第三方应用 调用标准包uicc.connection 的api“SCP81Connection.Send()”时， 本地实现加密收到的数据并通过scp81通道发送出去
*
*@param      :void
*            
*@return     :void
*
*@history    :created by jiangqb@20190626
*			 :modify by jiangqb@20191120 删除一个无用的传参，匹配java代码
*            :modify by jiangqb@20200603 增加异常处理，联通测试
*            :modify by xuxiaojun@20200620 用TLSHandProcess统一处理
*			 :modify by jiangqb@20200629 scp81api业务中，第一条上行数据是app调用本函数上发数据，所以此时必须设置httpException状态为ok，后续函数判断中才会读取本次的待发送数据
*/
void NM_scp81ConnectionSend(void)
{
	WORD idata dataBuf, dataOffSet, dataLen, perLen, Off;
	
	UWORD xdata dataAddr, fileAddr;
	
	G_SCP81Connection.DataReceivedEventRef = PopOperandStackShort(); 		//DA
	
	dataLen = PopOperandStackRef();

	dataOffSet = PopOperandStackRef();

	dataBuf = PopOperandStackRef();

	if(dataBuf == REF_NULL)
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
		
	dataAddr = locateObjectInMemory(dataBuf);

	check_bounds_interval(MEMtPF8(dataAddr), dataOffSet, dataLen); //ArrayIndexOutOfBoundsException if sOffset and sLength are causing access outside of baData array bounds

	dataAddr	= U32_A_U16(get_array_data(MEMtPN8(dataAddr)) , dataOffSet);
	
	if((G_BIPModuleStatus.channelDataType != ChannelDataType_HTTPS) || (G_HTTPSStatus.HTTPTriggerMode != HTTPTriggeredBySCP81Connection))//INVALID_STATE if the connection is not open.
	{
		throw_error(CONNECTIONEXCEPT_INVALID_STATE);
	}
	
	if (G_SCP81Connection.scp81ConnectionStatus == SCP81_CONNECTION_SEND)	//本次数据已经发送，并且新的收据还没有接收到
	{
		throw_error(CONNECTIONEXCEPT_INVALID_STATE);
	}
	
	if (G_SCP81Connection.scp81ConnectionStatus == SCP81_RECEIVE_DATA)	//
	{
//		fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSCommandData);			//数据已经在传给应用，但是应用还没有获取完上次接收到的下行数据，不能发送数据
			
//		if (ReadTFFileWord(fileAddr) != 0)
//		{
//			throw_error(CONNECTIONEXCEPT_INVALID_STATE);
//		}
		if(G_HTTP_CommandData_Length != 0x00)	
		{
			ThrowError(CONNECTIONEXCEPT_INVALID_STATE);	
		}	
	}
	fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSResponseData);		 	//fileAddr的有效性在前面已判断
	
	Off = 0;
	
	perLen = 0xF0;
	
    WriteTFFileWord(fileAddr, dataLen);
		
	while(Off < dataLen)
	{			
		perLen = ((dataLen - Off)<perLen) ? (dataLen - Off) : 0xF0 ;
		
		VMReadBuf(RecData, dataAddr+Off, perLen); 
		
		WriteTFBuf(fileAddr, 2+Off, RecData, perLen);	
		
		Off += perLen;
	}	
	
	G_HTTPSStatus.httpCommandType = HTTP_Command_Type_SCP81;

	G_SCP81Connection.scp81ConnectionStatus = SCP81_CONNECTION_SEND;

	G_TLSSecurityParameters.tlsRecordState  = TLS_SEND_APPLICATION_DATA_STATE;
	
	G_HTTPSStatus.httpException = HTTP_OK;		//本函数执行到此处表示状态正常，后续用于读取响应数据前判断状态是否正常 (scp81api测试\RAM\T_000_HTTP_InstallBipApplet_underISD_Normal)
			
	TLSHandProcess(0, 0);		
}

/*
*@funtion    :NM_scp81getCurrentEventStatus
*
*@description:第三方应用, 获取当前的Event eventStatus，是SCP81ConnectionStatus or DataReceived的对象
*
*@param      :void
*            
*@return     :void
*
*@author     :created by jiangqb@20190706
*/
void NM_scp81getCurrentEventStatus(void)
{
	if(G_SCP81Connection.scp81ConnectionStatus == SCP81_RECEIVE_DATA)
	{
		PushOperandStackShort(G_SCP81Connection.DataReceivedEventRef);
	}
	else
	{
		PushOperandStackShort(G_SCP81Connection.SCP81ConnectionStatusEventRef);
	}	
}


/*
*@funtion    :NM_scp81BakToolkitEvent
*
*@description:移动研究院东信3.0+卡问题联调专项群:通过ConnectionServer. createSCP81Connection(Observer)方法建立HTTP会话时，
              需要在eventObserved中使用ProactiveHandlerSystem. getTheHandler()和ProactiveResponseHandlerSystem. getTheHandler(),但是规范中handler只能在processToolkit函数内才可用
              针对此需求，COS做了特殊处理: 从正常流程来说，应用触发完之后，G_ToolkitContext.CurrentEvent[G_CurSelectedChannel]的值肯定是EVENT_InitValue，所以这里不用备份。
              且未对handler进行限制。如果需要对handler index进行限制的话，需要修改函数Toolkit_GetHandlerStatus
*
*@param      :void
*
*@return     :void
*
*@author     :created by xuxiaojun@20210331
*/
void NM_scp81BakToolkitEvent(void)
{
	G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] = EVENT_Private_SCP81Connection_EventObserved;
}
/*
*@funtion    :NM_scp81RecoverToolkitEvent
*
*@description:移动研究院东信3.0+卡问题联调专项群:通过ConnectionServer. createSCP81Connection(Observer)方法建立HTTP会话时，
              需要在eventObserved中使用ProactiveHandlerSystem. getTheHandler()和ProactiveResponseHandlerSystem. getTheHandler(),但是规范中handler只能在processToolkit函数内才可用
              针对此需求，COS做了特殊处理: 清除G_ToolkitContext.CurrentEvent[G_CurSelectedChannel]的值为EVENT_InitValue。
              且未对handler进行限制。如果需要对handler index进行限制的话，需要修改函数Toolkit_GetHandlerStatus
*
*@param      :void
*
*@return     :void
*
*@author     :created by xuxiaojun@20210331
*/
void NM_scp81RecoverToolkitEvent(void)
{
	G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] = EVENT_InitValue;
}
/*
*@funtion    :NM_SCP81ConnectionStatusGetDetails
*
*@description:java层代码通过本地方法获取当前Scp81ConnectionStatus执行到那个状态 
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20180612
*/
void NM_scp81ConnectionStatusGetDetails(void)
{
	PushOperandStackShort(G_SCP81Connection.scp81ConnectionStatus);
}
/*
*@funtion    :NM_scp81SetConnectionObserver
*
*@description:判断是否创建scp81hander会话对象， 传入调用app的索引
*
*@see        :uicc.connection.ConnectionServer.createSCP81Connection(Observer oOserver)
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20191119
*            :modify by xuxiaojun@20200611 NullPointerException if oObserver  is equal to  null
*            :modify by xuxiaojun@20220208 金科的和包闪付应用在register()之前调用该方法，之前的方案会抛异常。导致应用安装失败。修改方案。
*/
void NM_scp81SetConnectionObserver(void)
{
	WORD idata oOserverRef;
	
	oOserverRef = PopOperandStackRef();
	
	if(oOserverRef == REF_NULL)
	{
		throw_error(NULL_POINTER_EXCEPTION);
		
		return;
	}

	if (G_BIPModuleStatus.channelDataType == ChannelDataType_NULL)    // scp81 connect NOT exist
	{
		PushOperandStackShort(TRUE);		
	}
	else
	{
		PushOperandStackShort(FALSE);
	}
}

/*
*@funtion    :NM_scp81DiscardRemainingData
*
*@description:
*
*@see        :uicc.connection.DataReceived.discardRemainingData()
*
*@param      :void
*            
*@return     :void
*
*@author     :created by xuxiaojun@20200622
*/
void NM_scp81DiscardRemainingData()
{
//	UWORD xdata fileAddr;
	
//	fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSCommandData);
	
	if(G_SCP81Connection.scp81CopyDataRemainLen == 0x00)
	{
		throw_error(CONNECTIONEXCEPT_OPERATION_NOT_ALLOWED);	//OPERATION_NOT_ALLOWED if all data have already been received or discarded.
	}
	else
	{
		G_SCP81Connection.scp81CopyDataRemainLen = 0x00;

//		ClearFileFirstWordAddr(fileAddr);
		G_HTTP_CommandData_Length = 0x00;
	}
}
/*
*@funtion    :NM_scp81GetReceivedDataLength
*
*@description:第三方应用 调用标准包uicc.connection 的api“DataReceived.getReceivedDataLength()”时调用，返回接收到的明文数据长度
*
*@param      :void
*            
*@return     :void
*
*@author     :created by jiangqb@20190709
*/
void NM_scp81GetReceivedDataLength()
{
//	WORD idata len;
	
//	UWORD xdata fileAddr;
	
	SDWORD xdata dataLength;

//	fileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPSCommandData);
	
//	len = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, 0x00, (BYTE *)&len, sizeof(WORD));
	
//	dataLength = len;
	dataLength = G_HTTP_CommandData_Length;
	
	PushOperandStackInt(dataLength);
}
/*
*@funtion    :NM_scp81contentTypeIsCorrect
*
*@description:第三方应用 调用标准包uicc.connection 的api“DataReceived.contentTypeIsCorrect()”时， 本地实现把收到的https下行数据头中的contentType是否正确返回
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20200602
*/
void NM_scp81contentTypeIsCorrect(void) 
{
	if (G_HTTPSStatus.httpCommandType == HTTP_Command_Type_SCP81)
	{
		PushOperandStackShort(TRUE);
	}
	else
	{
		PushOperandStackShort(FALSE);
	}
}
/*
*@funtion    :NM_scp81responseIsComplete
*
*@description:第三方应用 调用标准包uicc.connection 的api“DataReceived.scp81responseIsComplete()”时， 本地实现把收到的https下行数据是否完整进行返回
*
*@see        :uicc.connection.DataReceived.responseIsComplete
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20200602
*/
void NM_scp81responseIsComplete()
{
	if (G_HTTPSStatus.httpDataStatus == HTTP_DataType_END)
	{
		PushOperandStackShort(TRUE);
	}
	else
	{
		PushOperandStackShort(FALSE);
	}
}

/*
*@description:如果当前的HTTP会话是由应用触发的，并且触发api是scp81connection.open接口，则在出错，握手完成，收到数据并正确解析出明文后都应该调用本函数，通知java层的触发应用
*
*@function   :Scp81ConnectionReport
*
*@param      :BYTE Status,	当前scp81connection的状态，
*
*@return     :void
*                   
*@history    :created by xuxiaojun@20180815
*/
void Scp81ConnectionReport(WORD SCP81Status)
{
	if(G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection)
	{
		G_SCP81Connection.scp81ConnectionStatus = SCP81Status;		

		GPSubCallFunction(scp81ConnectionObserver_METHOD_OFFSET, JC_FRAME_WORK_PKG_ID, JCRE_CONTEXT);		//scp81ConnectionObserver_METHOD_ADDR
	}
}
#endif
