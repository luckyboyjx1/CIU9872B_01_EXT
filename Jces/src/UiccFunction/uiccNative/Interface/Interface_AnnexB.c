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


#include "Interface_AnnexB.h"
#include "Interface_TimerModule.h"
#include "TlsMainFlow.h"
#include "HttpProcess.h"
#include "cardManager.h"
#include "GPAnnexbAdminSessionPara.h"
#include "FileAction.h"
#include "Stkhead.h"
#include "anomaly.h"
#include "BIPModuleProcess.h"
#include "Object.h"
#include "Gsmpublic.h"
#include "interpreter.h"
#include "Exception.h"
#include "fileSystem.h"
#include "TimerModuleProces.h"
#include "ExpandedManage.h"
#include "firewall.h" //warning checkPrevContextAccess
#include "utilMethod.h" //warning check_bounds_interval
#include "ob_manager.h" //warning locateObjectInMemory
#include "opFunction.h"
#include "profile.h"
#include "ToolkitRFM.h"
#include "TlsAlgorithm.h"

#if JAVA_CARD_UICC
#include "FunctionSwitch.h"
#include "ToolkitApiPublic.h"
#include "ToolkitRfm.h"
#include "platform_api.h"
#include "Stkalert.h"
#endif

#if !JAVA_CARD_UICC
#include "Interface_LTEFromBoot.h"
#include "error.h"
#endif

//以下的https握手支持的算法集，可以由产业化人员根据需求进行配置
#if UICC__OPERATOR__CHINATELECOM	//根据电信测试需求：中国电信用户卡需求白皮书-OTA下载功能分册(2021.V1).pdf 的 5.8章节, 不支持TLS_PSK_WITH_NULL_SHA 与 TLS_PSK_WITH_NULL_SHA256。
BYTE code Label_Tls0301_CipherSuit[] = {0x06,0x00,0xAE,0x00,0x8B,0x00,0x8C};
#else
BYTE code Label_Tls0301_CipherSuit[] = {0x06,0x00,0x8B,0x00,0x8C,0x00,0x2C};		//tls1.0/1.1版本支持的算法集
#endif

#if UICC__OPERATOR__CHINATELECOM	//根据电信测试需求：中国电信用户卡需求白皮书-OTA下载功能分册(2021.V1).pdf 的 5.8章节, 不支持TLS_PSK_WITH_NULL_SHA 与 TLS_PSK_WITH_NULL_SHA256。
BYTE code Label_Tls0303_CipherSuit[] = {0x06,0x00,0xAE,0x00,0x8B,0x00,0x8C};	//tls扩展字段
#else
BYTE code Label_Tls0303_CipherSuit[] = {0x0A,0x00,0xAE,0x00,0xB0,0x00,0x8B,0x00,0x8C,0x00,0x2C};	
#endif
/*
*@funtion    :IF_AnnexB_ProcessRemoteAppendDataForPor
*
*@description:用于执行完整指令  对于压缩格式把响应数据写入Por文件给Http使用,对于扩展格式是添加 AB或者AF等 tag给发Por使用
*
*@param
*
*@return     :void
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181213      create
*/
void IF_AnnexB_ProcessRemoteAppendDataForPor(BYTE mode)
{
#if GP__AMENDMENT__B__FOR_MASK
	WORD  idata DataLen,offset;
	BYTE  idata tmp[4],addlen;
	
#if !GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	UWORD xdata HttpResfileAddr;
#endif
	
	G_CurrentComNUM = G_HTTPSStatus.HttpApduNum;

	if(mode == RemoteDataEndBlock)
	{
		IF_AnnexB_HTTPRFM(HTTP_RFM_MODE_CLEAR);  //清空 http RFM备份文件
		
		IF_AnnexB_ChangeComNUM(ComNUM_CLEAR);	//此处不清空也行，因为会在http发送por之后进行结构体G_HTTPSStatus清空
	}

	//文件地址在前面已经判断过有效性
#if !GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	HttpResfileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSResponseData);
#endif

	offset = 0;
 
	if(G_FormatAndType == FormatType_Compat)   //压缩格式  102226  5.1.2 Compact Remote response structure  Num + SW1/SW2 + Response data
	{
		tmp[0] = G_CurrentComNUM;

		tmp[1] = SW1;

		tmp[2] = SW2;

		offset += 0x03;

#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
		ecpMem_Copy(&HTTP_ResponseData_ADDR[ramExtendResValue_offset], &tmp[0], 0x03);
#else
		WriteTFBuf(HttpResfileAddr,ramExtendResValue_offset,&tmp[0],0x03);      //写入 执行指令个数 +  状态字
#endif		

		if((G_CurrentRemoteResponseLgth > 0xFF)&&(G_CurrentRFMEfAddr!=FALSE))
		{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM	
			VMReadBuf(&HTTP_ResponseData_ADDR[ramExtendResValue_offset+offset], G_CurrentRFMEfAddr, G_CurrentRemoteResponseLgth);
#else			
			VMCopyDataToAddr(HttpResfileAddr,ramExtendResValue_offset+offset, 0, G_CurrentRFMEfAddr,G_CurrentRFMEfOffset, G_CurrentRemoteResponseLgth,MODE_PROTECT_WRITE,1,1);

			G_CurrentRFMEfAddr = FALSE;
			G_CurrentRFMEfOffset = 0x0000;
#endif	
		}
		else
		{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(&HTTP_ResponseData_ADDR[ramExtendResValue_offset+offset], RecData, G_CurrentRemoteResponseLgth);
#else	
			WriteTFBuf(HttpResfileAddr, ramExtendResValue_offset+offset, RecData, G_CurrentRemoteResponseLgth);
#endif
		}

		offset += G_CurrentRemoteResponseLgth; //数据总长度
	}
	else
	{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
		DataLen = BuildWord(&HTTP_ResponseData_ADDR[0]);
#else
		DataLen = ReadTFFileWord(HttpResfileAddr);//ReadTFBuf(HttpResfileAddr, ramExtendResLen_offset, (BYTE *)&DataLen,sizeof(DataLen));  // 读取L长度
#endif

		if(G_FormatAndType == FormatType_ExtendAA)
		{
			tmp[0] = TAG_ResponseScriptingTemplateDefiniteLength;

			addlen = SetBerTlvLength(&tmp[1],DataLen);

			addlen++;  // 0xAB
		}
		else
		{
			tmp[0] = TAG_ResponseScriptingTemplateInDefiniteLength;
			tmp[1] = 0x80;
			addlen = 0x02;      // AF 80
		}
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM	
		ecpMem_Move(&HTTP_ResponseData_ADDR[ramExtendResValue_offset+offset+addlen], &HTTP_ResponseData_ADDR[ramExtendResValue_offset], DataLen);

		ecpMem_Copy(&HTTP_ResponseData_ADDR[ramExtendResValue_offset], tmp, addlen);
#else
		VMCopyDataToAddr(HttpResfileAddr,ramExtendResValue_offset+offset+addlen, 0, HttpResfileAddr,ramExtendResValue_offset, DataLen,MODE_PROTECT_WRITE,1,1);

		WriteTFBuf(HttpResfileAddr, ramExtendResValue_offset, tmp, addlen);
#endif
		if(G_FormatAndType == FormatType_ExtendAE)
		{
			tmp[0] = 0x00;
			tmp[1] = 0x00;
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(&HTTP_ResponseData_ADDR[ramExtendResValue_offset + DataLen + addlen], tmp, 0x02);
#else
			WriteTFBuf(HttpResfileAddr, ramExtendResValue_offset + DataLen + addlen, tmp, 0x02); //写入后面的0x00 0x00
#endif

			addlen += 0x02;
		}

		offset = (addlen + DataLen); //数据总长度	
	}

	//WriteTFBuf(HttpResfileAddr,ramExtendResLen_offset,(BYTE *)&offset,sizeof(offset));      //重写L长度
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	SetWord(HTTP_ResponseData_ADDR, offset);
#else
	WriteTFFileWord(HttpResfileAddr, offset);
#endif

	G_CurrentRFMEfAddr = FALSE;
	G_CurrentRFMEfOffset = 0x0000;

#if JAVA_CARD_UICC
	G_RFMRAMKeyversion = 0xFF;//RFMRAM处理完成后将该变量清为FF 20131128 LJ
#endif
#endif
	//G_FormatAndType = FormatType_NULL;//初值	WQ_Offline_Test/WQ_Offline_Function_Test/TC_ES8_DownloadAndInstallation_HTTPS_ProfileData_Error_13  来了一个完整的AE8084XX85XX00,接着来Len86期望报错时候 AFtag回复  故 恢复值放到这里  不放在不是AA AEtag判断那里
}
/*
*@function   :IF_AnnexB_StartTimerAfterRestartHandset
*
*@description:如果上一次HTTP会话异常终止，比如手机关机重启/飞行模式切换  等导致HTTP会话终止，需要发送timer，等待服务器下发timer expiration事件后重新打开会话
*
*@See        :印度rjio需求 , GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf  3.7.4  Retry  Policy Parameters
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20180725
*            :V1.1  modfiy by xuxiaojun@20180803  retry时需要判断RetryConterRemain是否不为0，只有不为0的时候才retry
*            :V1.2  modify by xuxiaojun@20180823  参考系统的主动式指令触发方式，当系统空闲时可以触发系统式主动式指令。IF_ProcessSystemProactiveCommand
*            :V1.3  modify by xuxiaojun@20180827  调整G_OpenControl = FALSE;位置为进入该函数的if((G_OpenControl == TRUE) && (config == 0x01) & (G_BIPModuleStatus.channelNum == 0x00)) 判断条件之后，避免出现重试之后又走入start timer
*            :V1.4  modify by jiangqb@20191120    还原对开启timer的判断为 if((G_OpenControl == TRUE) && (config == 0x01) && (G_BIPModuleStatus.channelNum == 0x00)), 之前一版不知道啥时候优化导致有bug
*            :V1.5  modify by xuxiaojun@20210412  同步TPX 5G代码，书写规范化
*            :V1.6  modify by jiangqingbin@20220418  修改使用http的子开关 GP__AMENDMENT__B__SUPPORT_SESSION_RETRY，用来控制是否需要重试机制
*/
void IF_AnnexB_StartTimerpublic(BYTE mode)
{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
	UWORD idata fileAddr;
	BYTE idata config;
	BYTE xdata retry[7];

	if(mode == mode_AfterRestartHandset)
	{
		if((SW1 != 0x90)||(SW2 != 0x00)||(STKPerform == TRUE))//当指令返回值为9000，且没有系统主动式命令在执行，可触发新的系统主动式命令
		{
			return;
		}
	
		fileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData);
		
		ReadTFBuf(fileAddr, TLS_OpenCloseNormal_Offset, &config, 0x01);  //config为Https_Session_Running表示上一次HTTP会话非正常关闭
		
		if((G_OpenControl == TRUE) && (config == Https_Session_Running) && (G_BIPModuleStatus.channelNum == 0x00))  //
		{
			G_OpenControl = FALSE;
		}
		else
		{
			G_OpenControl = FALSE;
			return;
		}
	}

	fileAddr = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara);
	
	ReadLFBuf(fileAddr, HTTP_TriggerPara_RetryPara_Index, 2, retry, sizeof(retry));   //获取retry counter

	G_HTTPSStatus.RetryConterRemain = BuildWord(&retry[0]);						 //现在mode_AfterUnNormalEnd的模式的时候的重发次数也是在同一个位置获取
	
	if((G_HTTPSStatus.RetryConterRemain > 0) && ((retry[2] == 0x25)||(retry[2] == 0xA5)) && (retry[3] == 0x03))
	{
		IF_TimerModule_StartTimer(&G_HTTPSStatus.HTTPTimerId, &retry[3]);   //由于同一时刻，只允许cattp/http/ota中的一个被打开，所以timerid也只会有一个
	}		
	
#endif	
}
/*
*@function   :IF_AnnexB_RetryHttpSessionByTimerExpiration
*
*@description:上一次的HTTP会话异常终止之后，收到timer expiration之后再开启会话，累计尝试次数为
*
*@See        :   
*
*@param      :BYTE value  标记位的值
*
*@return     :void
*
*@history    :V1.0  created by xxj @20180703         
*            :V1.1  modify by xuxiaojun@20180803  只有当timerIDy一致时才重启会话，且需要释放timer
*            :V1.2  modify by xuxiaojun@20180826  如果重试次数耗尽，则将TLS_OpenCloseNormal_Offset标记清0，避免开机后再次进行重试
*            :V1.3  modify by xuxiaojun@20180826  将WriteTFBuf写1个字节的实现，用SetTFConst来代替
*            :V1.4  modify by xuxiaojun@20180827  调整HTTPIsResume = FALSE;的位置到重试次数耗尽之后，否则按照之前的调用位置会出现第一次重试时，发送的第一条HTTP POST没有resume = true这一项
*            :V1.5  modify by jiangqingbin@20220418  修改使用http的子开关 GP__AMENDMENT__B__SUPPORT_SESSION_RETRY ，用来控制是否需要重试机制
*/
void IF_AnnexB_RetryHttpSessionByTimerExpiration(BYTE timerExpirationId)
{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
	UWORD xdata fileAddr, triggerParaFileAddr;
	
	BYTE idata temp[2];		//RetryConterRemain
	
	if (TimerModuleTimerIdInValid(timerExpirationId))//判断timer是有效的，因为在cos重启后都是0， 下一个if判断会总是进入
	{
		return;
	}
	
	if(timerExpirationId == G_HTTPSStatus.HTTPTimerId)   //只有timerID一致时才重启会话
	{
		G_HTTPSStatus.HTTPIsResume = TRUE;
		
		TimerModuleReaseTimerId(&G_HTTPSStatus.HTTPTimerId);
		
		fileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData);  //已经走到关通道这里了，所以后续不会再用之前的fileAddr，不会有问题
		
		ReadTFBuf(fileAddr, TLS_CurrentTriggerSD_Offset, &G_HTTPSStatus.TriggeredSD, 1);  
		
		ReadTFBuf(fileAddr, TLS_CurrentTriggerMode_Offset, &G_HTTPSStatus.HTTPTriggerMode, 1); 
		
		ReadTFBuf(fileAddr, TLS_CurrentTriggerAppid_Offset, &G_HTTPSStatus.TriggeredAppId, 1);  //
#if !SE_ONLY		
		ReadTFBuf(fileAddr, TLS_CurrentExecutingProfileIndex_Offset, &G_HttpExecutingProfileIndex, 1);  //
#endif
		//获取重发参数并重写重发参数
		triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);
		
		ReadLFBuf(triggerParaFileAddr, HTTP_TriggerPara_RetryPara_Index, 2, temp, 2);
				
		G_HTTPSStatus.RetryConterRemain = BuildWord(temp) - 1;
		
		SetWord(temp, G_HTTPSStatus.RetryConterRemain);
		
		WriteLFBuf(triggerParaFileAddr, HTTP_TriggerPara_RetryPara_Index, 2, temp, 2);	//当前会话剩余的重试次数减一
				
		//esim-握奇测试时，下载profile过程中断开，期望重连后使用初始的uri, 并且不发送上一次的apdu响应， 即下载profile的过程重新开始，参考测试用例 《WQ_Offline_Function_Test--TC_ES8_DownloadAndInstallation_HTTPS_Error_DownloadInterrupted_03》
		//东信https通用测试时，期望重连后使用上次回话的最后下行的uri, 参考用例 《通用版HTTPS测试/RAM/T_010_HTTP_NoSeg_RAM_Retry_Twice》
#if M2M_EUICC
		if ((IF_IsProfileLoadingSession() == TRUE)  && (G_HTTPSStatus.TriggeredSD == ISDR_APPID))
		{
			saveHttpsParaForSession(G_HTTPSStatus.TriggeredSD, HTTP_TriggerPara_AdmURI_TAG, 0, FALSE);
		}
#endif
		HTTPSessionStart();     //该函数内部会根据HTTPIsResume的值来判断第一次发送HTTP REQUEST时是否包含resume项。 重试结束，需要将HTTPIsResume清掉。否则可能会导致下一次正常触发时又认为是重试
	}
#endif	
}
/*
*@function   :IF_AnnexB_StartHttpSessionByAPI
*
*@description:通过HTTP API方式触发HTTP时，需要等待应用重入结束之后再启用HTTPSessionStart
*
*@See        :   
*
*@param      :BYTE value  标记位的值
*
*@return     :void
*
*@history    :V1.0  created by xxj @20180703
*            :V1.1  modify by xuxiaojun@20180803  增加对印度RJIP activation应用特殊处理
*            :V1.2  modify by xuxiaojun@20180808  将对Activation应用的特殊处理恢复到201807311050版本。因为上面增加了特殊处理之后，activation无法发出open channel. 修改之后，TC反馈Activation工作正常
*            :V1.3  modify by xuxiaojun@20180831  如果当前已经在HTTP会话过程中，收到应用触发HTTP请求时，不允许开通道成功，直接返回。
*            :V1.4  modify by xuxiaojun@20210412  同步TPX 5G的BUG, 对于同一个API触发请求，该接口函数只允许被调用一次。修改之前，会执行一次该接口函数，然后开通道失败，此时会start timer，然后继续在uicc_main里面循环，导致该接口函数会重复执行
*/
void IF_AnnexB_StartHttpSessionByAPI(void)
{
#if (GP__AMENDMENT__B__createSCP81Connection_FOR_MASK || GP__AMENDMENT__B__requestHTTPAdministrationSession )
	if(G_BIPModuleStatus.channelDataType != ChannelDataType_HTTPS)
	{
		if((G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredByHTTPAdministration)
			|| (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection))
		{
			if(G_HTTP_APITriggerTag == HTTPTriggeredByAPPUnDone)
			{
				HTTPSessionStart();
			}
		}
//		G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredNull;  //避免触发成功之后，再次触发
	}
#endif

}
/*
*@function   :IF_AnnexB_HTTPRFM
*
*@description:HTTP下来的RFM指令的中间变量的保留/清除/读取. 执行指令前 读取,还原RFM变量前,写入;执行失败或者执行完时候清空
*
*@See        :
*
*@param      :BYTE mode 当前操作的模式
*
*@return     :void
*
*@history    :V1.0----created by xxj @20180905
*			 :modify by jiangqingbin-20220506- bug修改，缺陷编号385，http下行数据执行apdu个数的变动在另外的函数中赋值,所以去掉参数 BYTE ComamndNum
*/
void IF_AnnexB_HTTPRFM(BYTE mode)
{
#if GP__AMENDMENT__B__FOR_MASK
	UWORD xdata fileAddr;
	BYTE xdata backBuf[0x12];      //文件大小改了  这个数组大小也要修改

	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_HTTPRFM);		//0x8A06

	if(fileAddr == FALSE)
	{
		return;
	}

	if((mode == HTTP_RFM_MODE_READ) && (G_HTTPSStatus.HttpApduNum != 0))      // commandCount != 0x00  已经执行过 所以才需要读取
	{
		if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM)
		{
			ReadTFBuf(fileAddr, HTTP_RFM_Channel_Offset, (BYTE*)&G_ChannelNum, 0x01);

			ReadTFBuf(fileAddr, HTTP_RFM_DfAddr_Offset, (BYTE*)&G_CurrentDfAddr[G_ChannelNum], 0x04);

			ReadTFBuf(fileAddr, HTTP_RFM_EfAddr_Offset, (BYTE*)&G_CurrentEfAddr[G_ChannelNum], 0x04);

			ReadTFBuf(fileAddr, HTTP_RFM_DfName_Offset, (BYTE*)&G_CurrentDfName[G_ChannelNum], 0x02);

			ReadTFBuf(fileAddr, HTTP_RFM_EfName_Offset, (BYTE*)&G_CurrentEfName[G_ChannelNum], 0x02);

			ReadTFBuf(fileAddr, HTTP_RFM_ADfName_Offset, (BYTE*)&G_CurrentAdfName[G_ChannelNum], 0x02);

			ReadTFBuf(fileAddr, HTTP_RFM_RecNum_Offset, (BYTE*)&G_CurrentRecNum[G_ChannelNum], 0x01);

			ReadTFBuf(fileAddr, HTTP_RFM_CommandMode_Offset, (BYTE*)&G_CommandMode, 0x01);
		}
	}

	if(mode == HTTP_RFM_MODE_CLEAR)
	{
		if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM)
		{
			SetTFConst(fileAddr, 0x00, 0x00, GetTFSize(fileAddr));
		}
	}

	if(mode == HTTP_RFM_MODE_WRITE)
	{
		if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM)
		{
			memcpy(&backBuf[HTTP_RFM_DfAddr_Offset],(BYTE*)&G_CurrentDfAddr[G_ChannelNum], 0x04);
			memcpy(&backBuf[HTTP_RFM_EfAddr_Offset],(BYTE*)&G_CurrentEfAddr[G_ChannelNum], 0x04);
			memcpy(&backBuf[HTTP_RFM_DfName_Offset],(BYTE*)&G_CurrentDfName[G_ChannelNum], 0x02);
			memcpy(&backBuf[HTTP_RFM_EfName_Offset],(BYTE*)&G_CurrentEfName[G_ChannelNum], 0x02);
			memcpy(&backBuf[HTTP_RFM_ADfName_Offset],(BYTE*)&G_CurrentAdfName[G_ChannelNum], 0x02);
			backBuf[HTTP_RFM_RecNum_Offset] = G_CurrentRecNum[G_ChannelNum];
			backBuf[HTTP_RFM_CommandMode_Offset] = G_CommandMode;
			backBuf[HTTP_RFM_Channel_Offset] = G_ChannelNum;

			WriteTFBuf(fileAddr, 0x00, backBuf, sizeof(backBuf));
		}
	}
#endif
}

/*
*@function   :IF_AnnexB_TLSVariableInit
*
*@description:对TLS相关变量进行初始化
*
*@See        :   
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0----created by xxj @20180703         
*/
void IF_AnnexB_TLSVariableInit(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	TLSInitVariable();
	
	HttpProtocolInit();	
#endif	
}
/*
*@function   :clearHttpRetryFlag
*
*@description: 当前的profile被失效时， 当前如果有http会话，如果不是isdr的会话时，会话信息全部要被清空
*
*@See        :
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0----created by jiangqb @20190712
*/
void clearHttpRetryFlag()
{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY		//修改宏开关，当没开启retry开关时，http的会话信息没必要去清空，因为该开关关闭时不会进入重试机制，即关闭重试时会话文件EF_TLSsessionData的信息不起作用
	BYTE idata sdid;

	UWORD xdata fileAddr;

	fileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData);

	if (fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, TLS_CurrentTriggerSD_Offset, (BYTE *)&sdid, 0x01);

#if M2M_EUICC
		if (sdid != ISDR_APPID)																//
#endif
		{
			SetTFConst(fileAddr, 0x00, 0x00, 0x20);
		}
	}
#endif
}
/*
*@function   :IF_AnnexB_ChannelStatus
*
*@description:处理HTTP会话过程中下来的channel status指令，此时要求发送警告，同时关闭通道
*
*@See        :《印尼Reliance电信AmendmentB测试脚本-20180614/RealNetTest_14》     
*
*@param      :void
*
*@return     :TRUE表示执行完成/FALSE表示指令继续分发
*
*@history    :V1.0----created by xxj @20180322          
*@history    :V1.1----modify by jiangqingbin @20220418 增加开关 GP__AMENDMENT__B__SUPPORT_SESSION_RETRY 控制重试相关代码
*/
BYTE IF_AnnexB_ChannelStatus(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	if(G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)
	{
		 if(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Channel_Status))
		 {
			 setTLSSecurityPara(TLS_END_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_WARNING);					//中国移动测试脚本期望channelStatus事件时立刻关闭通道， 用例--T_021_HTTP_NoSeg_RAM_RetryCounter0000_LinkDropped
		 }
		 else
		 {
			 setTLSSecurityPara(TLS_SEND_ALERT_DATA_STATE, TLS_CLOSE_NOTIFY, ALERT_LEVER_WARNING);		//通用测试期望channelStatus事件时限发送ALERT再关闭通道， 用例--通用AmendMentB测试\SCP81_trigger_app\TestSCP81_trigger_app_03
		 }
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
		SetBit(G_TLSSecurityParameters.tlsRecordState, BIT_RETRY_WITH_START_TIMER);
#endif
		TLSHandProcess(0, 0);
		
		return TRUE;
	}
#endif	
	return FALSE;
}



/*
*@function   :IF_AnnexB_HTTP_SecondSession_Start
*
*@description:如果支持two session，则判断当前是否满足second session开户的条件. 在进入该函数时，第1次http session已经结束，相关的变量已经被清，处于无意义状态
*
*@See        :
*
*@param      :void
*
*@return     :BYTE TRUE:当前满足触发第2次会话条件/FALSE: 当前不满足触发第2次会话的条件
*
*@history    :V1.0  created by xuxiaojun@20180913
*			 :V1.1 modify by jiangqb@20191120 修改正确的宏定义名字, 修改HTTPSAnalyseTriggerPara的参数， 不在复用TargetedSD容易引起歧义，修改为TriggeredSD
*            :V1.2 modify by xuxiaojun@20201027 第二次会话的触发方案还原到TPX2.0方案：第一次HTTP会话结束立即开启
*/
BYTE IF_AnnexB_HTTP_SecondSession_Start()
{
#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
	UWORD xdata twoSessionFileAddr;
	BYTE idata tmpAppId;
	WORD idata secondTriggerLen;

	twoSessionFileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPTwoSession); //已经到了第二个HTTP会话过程，这些文件肯定是已经存在了的，否则第一个会话都不可能开户
	
	ReadTFBuf(twoSessionFileAddr, HTTP_SecondSession_ParaLen_Offset, (BYTE*)&secondTriggerLen, sizeof(WORD));
	
	if(secondTriggerLen == 0)
	{
		return FALSE;
	}

	SetTFConst(twoSessionFileAddr, HTTP_SecondSession_ParaLen_Offset, 0x00, sizeof(WORD));         //参数解析完成后，清除其有效数据长度，避免重复触发
			
	ReadTFBuf(twoSessionFileAddr, HTTP_SecondSession_TriggerMode_Offset, (BYTE*)&G_HTTPSStatus.HTTPTriggerMode, 0x01);    //获取第2次会话的触发模式，是API触发还是SMS触发
	
	ReadTFBuf(twoSessionFileAddr, HTTP_SecondSession_TriggerSD_Offset, (BYTE*)&G_HTTPSStatus.TriggeredAppId, 0x01);                  //获取第2次会话的触发应用
#if !SE_ONLY	
	ReadTFBuf(twoSessionFileAddr, HTTP_SecondSession_ExecutingProfileIndex_Offset, &G_HttpExecutingProfileIndex, 0x01);
#endif	
	if(G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySMS)
	{
		G_HTTPSStatus.TriggeredSD = G_HTTPSStatus.TriggeredAppId;
	}
	else if (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredByHTTPAdministration)
	{
		G_HTTPSStatus.TriggeredSD = GetHTTPTriggeredSD(G_HTTPSStatus.TriggeredAppId);
	}
	else// if (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection)
	{
		G_HTTPSStatus.TriggeredSD = GetHTTPTriggeredSD(G_HTTPSStatus.TriggeredAppId);
	}
		
	if (!HTTPSAnalyseTriggerPara(G_HTTPSStatus.TriggeredSD, FindTFAddr(twoSessionFileAddr, HTTP_SecondSession_Para_Offset, secondTriggerLen)))
	{
		G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredNull;
		G_HTTPSStatus.TriggeredAppId = ILLEGAL_APPID;
		G_HTTPSStatus.TriggeredSD = ILLEGAL_APPID;
		return FALSE;
	}
	
	//scp81api和http-gpapi在调用HTTPSAnalyseTriggerPara后，都会对期望的scp81key是否存在进行判断，这样在握手获取key时就不用再次判断key是否存在，而且在触发时判断key不存在就不触发会话更合理，所以需要在短信和二次会话参数解析时增加以下判断
	if (TLSGetPSK(NULL) == 0x00)
	{
		return FALSE;
	}
	
	return TRUE;	
#else
	return FALSE;
#endif
}




/*
*@function   :IF_GP_GPAnnexB_TriggerSMS_Check
*
*@description: https 触发短信触发检查
*
*@See        :
*
*@param      :void
*
*@return     :BYTE TRUE:检查通过，继续后续执行触发参数解析
*
*@history    :V1.0  created by jiangqb@20200609, 中国移动的宏用于中国移动BIP新需求(5G SimCard\DOC\03 需求规范\移动5G卡技术要求\BIP增强处理技术要求_20200529.pdf)
*			：v1.2 modufy by jiangqb@20220303	bug修改，在判断出世http的触发短信时需要对G_FormatAndType等三个全局变量重新赋值，赋值有可能当前正在http会话的扩展格式中，接收到新的sms触发短信，此时G_FormatAndType表示是扩展格式，导致触发短信por拼装错误 (用例：连接服务API功能及增强bip技术测试脚本/RAM/SMS_Close_TLS_Session_Test10))
*/
BYTE IF_GP_GPAnnexB_TriggerSMS_Check(UWORD fileAddr, WORD commandOffset, WORD DataLen)
{	
#if GP__AMENDMENT__B__FOR_MASK

	WORD idata valueLen, LVLen;
	BYTE xdata triggerBuf[5];

	if(G_Source_Type == SourceType_SMS)
	{
		ReadTFBuf(fileAddr, commandOffset, triggerBuf, sizeof(triggerBuf));
		
		if (triggerBuf[0] == 0x81)		//http触发短信
		{
			G_FormatAndType = FormatType_Compat;   //压缩格式

			G_CurrentComNUM = 0x00;
				
			G_CurrentRemoteResponseLgth = 0x00;
				
			 if(JcvmConfig_GetOPerator() == OPERATOR_CHINAMOBILE)
			 {
				LVLen = 0;

				valueLen = GetBerTlvLength(&triggerBuf[1],&LVLen);

				ReadTFBuf(fileAddr, commandOffset+LVLen+valueLen, triggerBuf, sizeof(triggerBuf));
				
				if ((DataLen > (LVLen+valueLen)) && (triggerBuf[0] == 0x82))		//8182 TAG连着
				{
					GlobalSetStatus(SW_WRONG_DATA);
					
					return GPAnnexBTriParaError;
				}					 

				if (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)		//有会话在进行， 结束触发短信
				{
#if !GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
					GlobalSetStatus(SW_WRONG_DATA);
				
					return TLSSessionHasExist;	//1
#endif
				}
			}

			if (FALSE == IF_GP_GPAnnexB_TriggerSMS(fileAddr, commandOffset, DataLen))		//触发参数解析失败
			{	
				GlobalSetStatus(SW_WRONG_DATA);
				
				return GPAnnexBTriParaError;	//3
			}
			else
			{
				GlobalSetStatus(SW_NO_ERROR);
				
				return GPAnnexBTritag;	//0
			}					
		}
		else if (triggerBuf[0] == 0x82)
		{
			if(JcvmConfig_GetOPerator() == OPERATOR_CHINAMOBILE)
			{
				G_FormatAndType = FormatType_Compat;   //压缩格式

				G_CurrentComNUM = 0x00;
				
				G_CurrentRemoteResponseLgth = 0x00;
				
				LVLen = 0;

				valueLen = GetBerTlvLength(&triggerBuf[1],&LVLen);

				ReadTFBuf(fileAddr, commandOffset+LVLen+valueLen, triggerBuf, sizeof(triggerBuf));

				if ((DataLen > (LVLen+valueLen)) && (triggerBuf[0] == 0x81))		//8281 tag连着
				{
					GlobalSetStatus(SW_WRONG_DATA);

					return GPAnnexBTriParaError;	//3	
				}
				else
				{
					if (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)		//关闭当前会话
					{
						BIPModuleProcessStatus();
					}

					//移动金科测试中，log显式当下行数据是8200， spi是0222时不期望返回por，即8200格式正常时是无错误的
					GlobalSetStatus(SW_NO_ERROR);

					return TLSSessionHasClose;	//当前没有开启的tls session	
				}	
			}
		}
	}

#endif	
	return NOTGPAnnexBTritag;
	
}
/*
*@description:保留当前HTTP会话过程中下来的触发参数
*
*@function   :HTTPSecondSessionParaHold
*
*@param      :UWORD fileAddr      触发参数的原始E2地址，而不是文件地址
*            :WORD commandOffset  触发参数的原始有效数据的起始偏移
*            :WORD cplLen         触发参数的有效数据长度
*            :BYTE mode           第二次触发是通过SMS/API触发，取值为HTTPTriggeredBySMS/HTTPTriggeredByAPP
*            :BYTE triggerAPPID   第二次引起触发的APPID
*
*@return     :BYTE                处理成功返回TRUE/其他情况返回FALSE
*
*@history    :xuxiaojun@20180523
*/

#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
void HTTPSecondSessionParaHold(UWORD fileAddr, WORD commandOffset,WORD cplLen, BYTE mode, BYTE triggerAPPID)
{
	UWORD xdata twoSessionFileAddr, twoSessionParaAddr;
	WORD idata secondTriggerLen;
#if M2M_EUICC
	if(triggerAPPID == ISDR_APPID) //isdr不支持https多会话
	{
		return;
	}
#endif
	twoSessionFileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPTwoSession);
	
	ReadTFBuf(twoSessionFileAddr, HTTP_SecondSession_ParaLen_Offset, (BYTE*)&secondTriggerLen, sizeof(WORD)); //前面对文件存在性已经进行了判断
	
	if(secondTriggerLen != 0) //当前来了第3个触发请求，则不支持
	{
		return;
	}
	
	twoSessionParaAddr = FindTFAddr(twoSessionFileAddr, HTTP_SecondSession_Para_Offset, cplLen);   //判断EF_HTTPTwoSession这个文件大小是否足够
	
	if(twoSessionParaAddr == FALSE)
	{
		return;
	}
	
	WriteEepromAtomic(twoSessionParaAddr,(BYTE*)fileAddr,cplLen);                                           //保存第二条触发参数
	
	WriteTFBuf(twoSessionFileAddr, HTTP_SecondSession_ParaLen_Offset, (BYTE*)&cplLen, sizeof(WORD));         //保存第二条触发参数长度
	
	SetTFConst(twoSessionFileAddr, HTTP_SecondSession_TriggerMode_Offset, mode, 0x01);        //保存第二条触发参数的触发模式
	
	SetTFConst(twoSessionFileAddr, HTTP_SecondSession_TriggerSD_Offset, triggerAPPID, 0x01); 
#if !SE_ONLY
	if(mode == HTTPTriggeredBySMS)
	{
		SetTFConst(twoSessionFileAddr, HTTP_SecondSession_ExecutingProfileIndex_Offset,G_HttpExecutingProfileIndex, 0x01);
	}
	else
	{
		SetTFConst(twoSessionFileAddr, HTTP_SecondSession_ExecutingProfileIndex_Offset,G_CurExecutingProfileIndex, 0x01);
	}
#endif
}
#endif
/*
*@description:处理SMS下来的HTTP的触发信息，触发信息参考《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》的Administration Session Triggering Parameters
*            :将触发参数存在对应的文件中，如果某个参数不存在，则查找触发SD的对应的参数写在文件中，后续使用时，直接用该文件中的，而不需要再去查找SD的值
*
*@function   :IF_GP_GPAnnexB_TriggerSMS
*
*@param      :UWORD fileAddr      保存下行SMS数据的文件地址，目前是2800/6F90文件地址,进入该函数时肯定是有效值
*            :WORD commandOffset  下行SMS数据的命令起始数据在文件中偏移
*            :WORD cplLen         下行SMS数据长度
*
*@return     :BYTE                处理成功返回TRUE/其他情况返回FALSE
*
*@history    :xuxiaojun@20180523  
*            :xuxiaojun@20200616  删除该函数中对于EF_RAMAPDUTars文件是否存在的判断，因为走到这一步时该文件肯定存在；调整InitHttpVariable的调用位置
*			 :jiangqb@20200710	      调整全局变量初始化位置
*            :xuxioajun@20200716  全局变量的赋值不能放在有效触发之前，否则可能会冲掉当前正在进行的HTTP会话的对应全局变量值，代码逻辑顺序与EPX2.0靠齐，便于后续方案统一
*            :xuxiaojun@20220401  修改EPX产业化/INFENION_SLM97/Branch/huawei反馈回来的缺陷：：COS中将GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS设置为0时，当前有HTTP会话正在进行的情况下又来了一个HTTP的触发短信，COS采用抛异常的处理方式会导致当前的HTTP会话被打断
*/
BYTE IF_GP_GPAnnexB_TriggerSMS(UWORD fileAddr, WORD commandOffset,WORD cplLen)
{
#if GP__AMENDMENT__B__FOR_MASK
	BYTE idata triggeredSD;
	UWORD xdata dataAddr;
	
	triggeredSD = ILLEGAL_APPID;
	
	dataAddr = FindTFAddr(fileAddr, commandOffset, cplLen);			//获取eep地址	
	
	if (dataAddr == FALSE)
	{
		return FALSE;
	}

	if (!HttpsSessionFileCheck())
	{
		return FALSE;	
	}
#if UICC__FUNCTION__RAM	
	if(G_TARRecIndex[0] == TarType_RAM)
	{
		triggeredSD = G_TARRecIndex[1];
	}
#endif
#if M2M_EUICC
	else if(G_TARRecIndex[0] == TarType_ISDR) // 走到这里时，是否会出现G_TARRecIndex未赋值的情况？也就是说，会出现CheckTar(&spi[4],NULL)
	{
		triggeredSD = ISDR_APPID;
	}
#endif

	if(G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS) //表示当前已经打开了一个HTTP会话,  < GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf >中The Remote Administration Server explicitly ends the session by sending an HTTP response with no “ X - Admin- Next- URI ”   header and with an empty body
	{
#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
#if !SE_ONLY
		if(G_HttpExecutingProfileIndex != G_RAMBackExecutingProfileIndex)//第2个会话与第一个会话的执行区域不一样  暂时不支持
		{
//			G_RAMBackExecutingProfileIndex = G_HttpExecutingProfileIndex;//由于触发短信 checktar时 已经将G_RAMBackExecutingProfileIndex切到了第二个会话的执行区域  这里需要切回来继续执行第一个会话
//			G_CurExecutingProfileIndex = G_HttpExecutingProfileIndex;
//			ExecuEnvironmentInit(G_CurExecutingProfileIndex);

			return TRUE; //ThrowError(SW_CONDITIONS_NOT_SATISFIED);//throw error? 此处不能抛异常，否则会打断当前正在进行的HTTP会话 by xuxiaojun@20220401
		}
#endif
		HTTPSecondSessionParaHold(dataAddr, 0x00, cplLen, HTTPTriggeredBySMS, triggeredSD);
		
		return TRUE;
#else
		return TRUE; // ThrowError(SW_CONDITIONS_NOT_SATISFIED);//throw error? 此处不能抛异常，否则会打断当前正在进行的HTTP会话 by xuxiaojun@20220401
#endif

	}
	else if(G_BIPModuleStatus.channelDataType != ChannelDataType_NULL)//modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
	{
		return TRUE;
	}

	//调整全局变量初始化位置, 初始化函数必须放在 G_HTTPSStatus.TriggeredSD = ToolkitFindSDTars(G_ToolkitTar)前，否则会清空了G_HTTPSStatus.TriggeredSD
	//必须放在HTTPSecondSessionParaHold后，否则会清空G_BIPModuleStatus.channelDataType
	InitHttpVariable();
			
	if (!HTTPSAnalyseTriggerPara(triggeredSD, dataAddr))
	{
		return FALSE;
	}
	
	G_HTTPSStatus.TriggeredSD = triggeredSD;

	G_HTTPSStatus.TargetedSD = G_HTTPSStatus.TriggeredSD;

	G_HTTP_CommandData_Length = 0x00; //ClearFileFirstWord(DF_TLS, EF_HTTPSCommandData);

	G_BIPModule_Send_Length = 0x00; //ClearFileFirstWord(DF_BIPModule, EF_BIPModule_Send);
	
	G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredBySMS;
#if ((!SE_ONLY) && (JAVA_CARD_UICC))
	G_HttpExecutingProfileIndex = G_RAMBackExecutingProfileIndex;
#endif
		//scp81api和http-gpapi在调用HTTPSAnalyseTriggerPara后，都会对期望的scp81key是否存在进行判断，这样在握手获取key时就不用再次判断key是否存在，而且在触发时判断key不存在就不触发会话更合理，所以需要在短信和二次会话参数解析时增加以下判断
	if (TLSGetPSK(NULL) == 0x00)
	{
		return FALSE;
	}
	
	PendingMessageAlert(D_SCP81TriggerTag,0x00);  	//D_SCP81TriggerTag
	
	return TRUE;

#else
	return FALSE;
#endif
	
}

/*
*#function   :IF_HTTPDataAvaiable
*
*@description:HTTP的data available事件处理
*
*@param      :BYTE channelDataLen   表示当前是处在C2的D6事件中，channelDataLen表示待接收的数据长度，如果为0则表示数据接收完成
*@param      :WORD offset           表示当前bipReceive文件中有效数据的个数
*
*@return     :void
*
*@history    :created by xuxiaojun@20180529
*            :modify by xuxiaojun@20180707 根据BIP数据处理方案，增加函数参数，用于表示当前的BIP数据是否是最后一条
*/
void IF_HTTPDataAvaiable(BYTE channelDataLen, WORD offset)
{	
#if GP__AMENDMENT__B__FOR_MASK
	TLSHandProcess(channelDataLen, offset);
#endif	
	
}



/*
*#function   :IF_GetOpenChannelParaForHttp
*
*@description:获取HTTP开通道的相关参数
*
*@param      :BYTE *para    获取到的开通道参数的数据TLV+TLV格式
*
*@return     :BYTE 返回para中的有效数据
*
*@history    :created bu xuxiaojun@20180530
*/
BYTE IF_AnnexB_GetOpenChannelParaForHttp(BYTE *para)
{
#if GP__AMENDMENT__B__FOR_MASK
	return GetOpenChannelParaForHttp(para);
#else 
	return 0;
#endif	
}

/*
*@function   :IF_GetHttpTriggerParaBySDID
*
*@description:通过sdid过去sd的对应tag的参数
*
*@param      :BYTE sdId        --- SD的id
*@param      :BYTE tag         --- HTTP参数tag， 比如84,85,86,89
*@param      :BYTE *dataBuffer --- 获取到的tag数据的值
*
*@return     :WORD --- dataBuffer中数据的长度
*
*@history    :created by jiangqingbin@20190118
*            :modify by xuxiaojun@20190301
*			 :modify -jiangqb-20200720 增加一个传入变量， dataBufferLen用于防止读取数据到dataBuffer时越界
*
*/


WORD IF_GetHttpTriggerParaBySDID(BYTE sdId,BYTE tag,BYTE *dataBuffer, WORD dataBufferLen)
{
#if GP__AMENDMENT__B__FOR_MASK
	return GPGetSDTriggerParaBySDID(sdId, tag, dataBuffer,dataBufferLen);
#else
	return 0;
#endif
}


/*
*@funtion    :NM_GPRequestHTTPAdministrationSession
*
*@description:第三方应用 调用标准api“RequestHTTPAdministrationSession”时， 本地实现打开http通道与发送clienthello
*
*@param      :void
*
*@see        :org.globalplatform.HTTPAdministration
*            
*@return     :void
*
*@author     :created by jiangqb@20180530
*            :modify by xuxiaojun@20180803  对印度Activation应用增加特殊处理，与polling应用类似。 G_RJIO_Polling_Special变量的使用分为分为4个阶段
*                                初始化                 HTTP API           bastore                      StartHttpSession之后(清空变量)
*             polling             0x00                    0x01               0x02                            0x00
*             其他                0x00                    0x21                无                             0x00
*            :modify by xuxiaojun@20180808  将对Activation应用的特殊处理恢复到201807311050版本。因为上面增加了特殊处理之后，activation无法发出open channel. 修改之后，TC反馈Activation工作正常
*            :modify by xuxiaojun@20180826  调整调用HTTPSAnalyseTriggerPara的位置，因为HTTPSessionStart过程中会使用到分析之后的参数
*            :modify by jiangqb@20200603    优化代码
*            :modify by xuxiaojun@20200616  合并冗余代码
*            :modify by xuxioajun@20200716  全局变量的赋值不能放在有效触发之前，否则可能会冲掉当前正在进行的HTTP会话的对应全局变量值
*            :modify by xuxiaojun@20220401  修改EPX产业化/INFENION_SLM97/Branch/huawei反馈回来的缺陷：：COS中将GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS设置为0时，当前有HTTP会话正在进行的情况下又来了一个HTTP的触发短信，COS采用抛异常的处理方式会导致当前的HTTP会话被打断
*            :modify by xuxiaojun@20220407 经过小组成员、测试部讨论之后，决定API方式的触发不修改，20220401修改的代码还原
*/
void NM_GPRequestHTTPAdministrationSession(void)
{
#if GP__AMENDMENT__B__requestHTTPAdministrationSession
	WORD idata sLength, sOffset, baBuffer;
	UWORD xdata arraryAddr;
	BYTE idata triggeredAppId, triggeredSD;
	
	sLength =  PopOperandStackRef();   //length of triggering parameters
	sOffset =  PopOperandStackRef();   //offset of triggering parameters within triggeringParameters
	baBuffer = 	PopOperandStackRef();  //byte array containing administration session triggering parameters
	
	if(baBuffer == REF_NULL)
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}
	
	arraryAddr	= locateObjectInMemory(baBuffer);

	check_bounds_interval(MEMtPF8(arraryAddr), sOffset, sLength);
	
	checkPrevContextAccess(MEMtPN8(arraryAddr));

	arraryAddr = U32_A_U16(get_array_data(MEMtPN8(arraryAddr)) , sOffset);
	
	if(G_TrigerToolkitRef == NULL_REF)  //表示当前是通过select方式选择应用，然后给应用发送指令触发HTTP API,立即开启会话
	{
		triggeredAppId = (getChannelContext(G_CurSelectedChannel)&0xFF);
	}
	else       //这种方式下等所有的触发完成之后再开启会话
	{
		triggeredAppId = GetAppIdByRef(G_TrigerToolkitRef);  //
	}

	if (!HttpsSessionFileCheck())
	{
		throw_error(SW_FILE_NOT_FOUND);
	}

	if(G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS) //表示当前已经打开了一个http会话
	{		
#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
		HTTPSecondSessionParaHold(arraryAddr, 0x00, sLength, HTTPTriggeredByHTTPAdministration, triggeredAppId);
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

	if (!HTTPSAnalyseTriggerPara(triggeredSD, arraryAddr))
	{
		throw_error(SW_WRONG_DATA);
	}

	G_HTTPSStatus.TriggeredAppId = triggeredAppId;

	G_HTTPSStatus.TriggeredSD = triggeredSD;

	if (TLSGetPSK(NULL) == 0x00)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}

	G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredByHTTPAdministration;

	G_HTTPSStatus.HTTPIsResume = FALSE;    //在这里增加该变量的初始化，避免当前是在重试过程中出现应用触发该API
#if !SE_ONLY
	G_HttpExecutingProfileIndex = G_CurExecutingProfileIndex;
#endif
	if(G_TrigerToolkitRef == NULL_REF)
	{
		HTTPSessionStart();
	}
#endif	 	
}


#if JAVA_CARD_UICC
/*
*@funtion    :NM_getCurrentHttpStatus
*
*@description:在标准http-api(NM_GPRequestHTTPAdministrationSession)结束时, 通知java层调用本接口获取当前的https会话结束的状态G_HTTPSStatus.httpException(正常结束还是非正常结束)
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20180612
*/
void NM_getCurrentHttpStatus(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	PushOperandStackShort(G_HTTPSStatus.httpException);
#endif
}
/*
*@funtion    :getHttpAPPID
*
*@description:获取当前https会话的管理通道
*
*@param      :void 
*            
*@return     :void
*
*@author     :created by jiangqb@20180530
*			 ;modify by jiangqb-20200408  ---在调用应用监听https结束的接口前，获取应用的appid，此处要传的是触发https的appid，而非触发sdid
*/

void NM_getHttpAPPID(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	PushOperandStackShort(G_HTTPSStatus.TriggeredAppId);
#endif	
}


/*
*@Function: NM_GetAppletRefByAppId
*
*@Descrip: 获取当前appId所指示的Applet ref
*
*@Param : BYTE appID:

*@Return: Applet ref
*
*@Author: create by zzj
*
*@History:  V1.0-- 创建该函数
*/
void NM_GetAppletRefByAppId(void)
{
	BYTE idata appId;
	WORD idata ref;

	appId = (BYTE)PopOperandStackShort();
	
	if(appId != ILLEGAL_APPID)
	{
		ref = GetAppletRefByAppId(appId);
	}
	else
	{
		ref = REF_NULL;
	}
	
	PushOperandStackShort(ref);
}
#endif

/*
*@function   :IF_GetSCP81TLSVersion
*
*@description:获取指定的sd支持的tls版本的标识字节，字节含义参考 《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》的3.2.1章节，该字节在安装sd的参数中指定
*
*@param      :BYTE sdId
*
*@return     :BYTE iprara/FALSE
*
*@history    :created by jiangqb@20190118
*/
WORD IF_GetSCP81TLSVersion(BYTE sdId)
{
#if GP__AMENDMENT__B__FOR_MASK	

#if JAVA_CARD_UICC	
	BYTE idata scpOption;
	PN8		sdDataObj;
#endif

#if !JAVA_CARD_UICC	
	return TLS_Version12;			//目前native使用tls1.2版本
#else
	
	sdDataObj =	MEMtPN8(getSdDataObjBySdid(sdId));
	
#if M2M_EUICC
	if(sdId == ISDR_APPID)
	{
		return TLS_Version12;//ISDR只能使用12版本
	}
#endif

	if(sdId == ISD_APPID) //isd的scpBuffer未写入，所以采用默认值
	{
		return TLS_DefaultVersion;
	}
	else
	{
		scpOption = sdData_getScpOptionByScpType(sdDataObj, SCP_81);
	}

	if(scpOption == 0xFF)
	{
		return TLS_DefaultVersion;
	}
	else if ((scpOption & 0x04) == 0x04)
	{
		return TLS_Version12;
	}
	else if ((scpOption & 0x02) == 0x02)
	{
		return TLS_Version11;
	}
	else if ((scpOption& 0x01) == 0x01)
	{
		return TLS_Version10;
	}
	else
	{
		return TLS_DefaultVersion;
	}
#endif

#else
	return 0;
#endif
}



/*
*@function     : IF_AnnexB_SessionInISD
*
*@description : 判断当前的https会话是否是ISD开启的, 移动5G备案特殊需求: ISD - Normal RFM 正常执行；ISD - Special RFM 6985; Special SSD - Normal RFM 6985;  Special SSD - Special RFM 正常执行；Normal SSD 6985
*
*@See           :移动特殊需求：E:\5G SimCard\DOC\03 需求规范\移动5G卡技术要求\移动特殊ssd和特殊rfm-aid说明.doc
*
*@param      : BYTE* aidCmpBuffer --- 输入， 下行app-aid
*			：BYTE appIdLen --- 输入， 下行app-aid长度
*
*@return      : TRUE表示6985
*
*@history    : V1.0 ---- created by jiangqb @20191210
*			      : V1.1 ---- modify by jiangqb @20200110
*			      : V1.2 ---- modify by jiangqb-20200722	修正返回值
*/
BYTE IF_AnnexB_ChinaMobileNotSuppotRFMmode(BYTE * tarAppAID, BYTE appIdLen)
{
#if GP__AMENDMENT__B__FOR_MASK

#if JAVA_CARD_UICC
	if(JcvmConfig_GetOPerator() == OPERATOR_CHINAMOBILE)
	{
		BYTE xdata ssdAidCmpBuffer[16];
		BYTE idata aidLen;
		BYTE xdata SPECIAL_SSD_AID[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x00,0x40,0x00};					            //移动特殊需求，预置的ssd
		BYTE xdata SPECIAL_SSD_USIM_RFM_AID1[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0x0A};			//移动特殊需求，USIM_RFM_AID的别名
		
		aidLen = GetAIDDataByAppID(G_HTTPSStatus.TriggeredSD, ssdAidCmpBuffer);

		if ((appIdLen == sizeof(SPECIAL_SSD_USIM_RFM_AID1))&&(memcmp(SPECIAL_SSD_USIM_RFM_AID1, &tarAppAID[0], sizeof(SPECIAL_SSD_USIM_RFM_AID1)) == EQUAL))	//是特殊usim-rfm-aid
		{
			if((aidLen == sizeof(SPECIAL_SSD_AID))&& (memcmp(SPECIAL_SSD_AID, ssdAidCmpBuffer, sizeof(SPECIAL_SSD_AID)) == EQUAL))//特殊SSD
			{
				return FALSE;
			}
		}
		else if(G_HTTPSStatus.TriggeredSD == ISD_APPID)
		{
			return FALSE;
		}

		return TRUE;
	}
	else
#endif
	{
		return FALSE;
	}
#else
	return FALSE;
#endif	
}
/*
*@funtion    :IF_AnnexB_SaveHttpsCompactAPDURes
*
*@description: 保存本次简单格式https的响应状态字和响应数据
*			 : G_HttpsBuf在https握手完成后没有被使用到，此处复用保存响应数据，第一第二字节保存状态字，第三第四字节保存响应数据长度，第5到-n保存响应数据
*
*@param      : void
							 
*            
*@return     :void
*
*@history    :V1.0 :create by jiaangqb@20191125
*@history    :v1.1 modify by jiangqb-20200721-删除G_HttpsBuf，使用HandShakeHashSha256RemainDataBuf用于本函数业务的缓存f
*/
void IF_AnnexB_SaveHttpsCompactAPDURes(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	G_HandShakeHashSha256RemainDataBuf[0] = SW1;
		
	G_HandShakeHashSha256RemainDataBuf[1] = SW2;
	
	if (G_CurrentRemoteResponseLgth > 0)
	{
		if(G_CurrentRemoteResponseLgth < (sizeof(G_HandShakeHashSha256RemainDataBuf)-4))		//G_HandShakeHashSha256RemainDataBuf
		{
			SetWord(G_HandShakeHashSha256RemainDataBuf+2, G_CurrentRemoteResponseLgth);

			memcpy(G_HandShakeHashSha256RemainDataBuf+4, RecData, G_CurrentRemoteResponseLgth);
		}
	}
#endif
}


/*
*@funtion    :IF_AnnexB_GetHttpsCompactAPDURes
*
*@description: 获取上一次简单格式https的响应状态字和响应数据
*
*@param      : void
							 
*            
*@return     :void
*
*@history    :V1.0 :create by jiaangqb@20191125
*@history    :v1.1 modify by jiangqb-20200721-删除G_HttpsBuf，使用HandShakeHashSha256RemainDataBuf用于本函数业务的缓存f
*/
void IF_AnnexB_GetHttpsCompactAPDURes(void)
{	
#if GP__AMENDMENT__B__FOR_MASK
	SW1 = G_HandShakeHashSha256RemainDataBuf[0];
	
	SW2 = G_HandShakeHashSha256RemainDataBuf[1];
	
	if (G_CurrentRemoteResponseLgth > 0)
	{
		if(G_CurrentRemoteResponseLgth < 0xFF)
		{
			memcpy(RecData, G_HandShakeHashSha256RemainDataBuf+4, G_CurrentRemoteResponseLgth);
		}
	}
#endif	
}



/*
*@funtion    :IF_RfmApduModeCheck
*
*@description: 2g模式不支持非2g指令， 3g模式不支持2g指令
*
*@param      :
*
*@return     :true or false, TRUE表示在uicc模式下来了2G指令，或者sim模式下接收到非2g指令，程序将报错
*
*@history    :create- jiangqb @20190613    
*@history     modify-chentao@20211020 EUICC标准是有说明不 支持2G，但是不排除国外有客户把EUICC产品放到2G终端，所以为了兼容，目前EUICC是支持2G指令的，为了过电信的测试加了特殊处理
*/
BYTE IF_RfmApduModeCheck()
{
#if SE_ONLY
	if (G_APDU[0] == 0xA0) //2g模式
	{
		if (G_TARRecIndex[1] != TAR2_SIMFile_System)	//非2g模式
		{
			GlobalSetStatus(0x6E00);
			return TRUE;
		}
	}
	else	//非2g模式
	{

		if(G_TARRecIndex[1] == TAR2_SIMFile_System)
		{
			GlobalSetStatus(0x6E00);
			return TRUE;
		}

	}
#else
	if(JcvmConfig_GetOPerator() == OPERATOR_CHINATELECOM) //中国电信eUICC通信功能测试脚本 http1.1 rfm \\T_002_HTTP_NonSeg_RFM_NonNominal_InconsistantHeadAndBody
	{
		if (G_APDU[0] == 0xA0)
		{
			GlobalSetStatus(0x6E00);
			return TRUE;
		}
	}
#endif
	return FALSE;
}
/*
*@funtion    :ProcessRemoteFalshForHTTP 用于Http下来的数据在10226处理时候 数据不完整时候移动数据
*
*@description:解释压缩格式 
*
*@param      :SrcfileAddr	--	源地址
*			 :Offset	--	数据的起始地址
*			 :dataLen  -- 移动数据长度
*
*@return     :void
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181213      create
*/
void IF_ProcessRemoteMoveFalshForHTTP(UWORD SrcfileAddr,WORD Offset, WORD cplLen)
{
#if GP__AMENDMENT__B__FOR_MASK
	WORD idata TagLen,Lenoffset,Tagoffset;
	BYTE xdata TagLenBuf[4];
	BYTE idata tmp[2];
	
	if(Offset == 0x02)                // http传来的数据一开始就是不完整的 还没开始执行  这时候不需要移动数据
	{
		return ;
	}
	
	if(G_FormatAndType == FormatType_ExtendAA)
	{
//		ReadTFBuf(SrcfileAddr, ramExtendResValue_offset, &TagLenBuf[0],sizeof(TagLenBuf));  // ramExtendResValue_offset 0x02
		ReadEeprom(&TagLenBuf[0], SrcfileAddr + ramExtendResValue_offset, sizeof(TagLenBuf));
		
		Tagoffset = 0x00;
		
		TagLen = GetBerTlvLength(&TagLenBuf[1],&Tagoffset);
		
		Lenoffset = (Offset-Tagoffset-ramExtendResValue_offset); //已经处理的数据长度,
		
		if(Lenoffset == 0x00)      //第一个 tag就是不完整的
		{
			return ;
		}
		
		TagLen -= Lenoffset;//TagLen为剩余数据长度
		
		if(TagLen > 0xFF)    //剩余数据长度 还是82 XXXX  
		{
			Lenoffset = ramExtendResValue_offset + 2; //跳过 L AA 82
			
//			WriteTFBuf(SrcfileAddr,Lenoffset,(BYTE *)&TagLen,sizeof(TagLen));      //重写AAtag长度
			SetWord(tmp, TagLen);
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(SrcfileAddr + Lenoffset, tmp, 0x02);
#else
			WriteEepromWithoutProtect(SrcfileAddr + Lenoffset, tmp, 0x02);
#endif
			
			Lenoffset += 0x02;   //移动数据的目的偏移
		}
		else if(TagLen > 0x7F)
		{
			TagLenBuf[0] = 0x81;
			
			TagLenBuf[1] = (TagLen & 0xFF);
			
			Lenoffset = ramExtendResValue_offset +1;      //跳过 L AA
			
//			WriteTFBuf(SrcfileAddr,Lenoffset,&TagLenBuf[0],0x02);      //重写AAtag长度
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(SrcfileAddr + Lenoffset, &TagLenBuf[0], 0x02);
#else
			WriteEepromWithoutProtect(SrcfileAddr + Lenoffset, &TagLenBuf[0], 0x02);
#endif
			
			Lenoffset += 0x02;   //移动数据的目的偏移
		}
		else
		{
			TagLenBuf[0] = (TagLen & 0xFF);
			
			Lenoffset = ramExtendResValue_offset +1;      //跳过 L AA
			
//			WriteTFBuf(SrcfileAddr,Lenoffset,&TagLenBuf[0],0x01);      //重写AAtag长度
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(SrcfileAddr + Lenoffset, &TagLenBuf[0], 0x01);
#else
			WriteEepromWithoutProtect(SrcfileAddr + Lenoffset, &TagLenBuf[0], 0x01);
#endif
		
			Lenoffset += 0x01;   //移动数据的目的偏移
		}
		
		TagLen = cplLen + Lenoffset-ramExtendResValue_offset; //返回Http的是数据总长度
	}
	else if(G_FormatAndType == FormatType_ExtendAE)
	{
		Lenoffset = ramExtendResValue_offset + 2;    //跳过L AE 80  移动数据的目的偏移
		
		TagLen = cplLen + 2;    //返回Http的是数据总长度
	}
	else
	{
		Lenoffset = ramExtendResValue_offset;  //移动数据的目的偏移
		
		TagLen = cplLen;  //返回Http的是数据总长度
	}
	
//	fileSize = GetTFSize(SrcfileAddr);

//	VMCopyDataToAddr(SrcfileAddr+sizeof(TranFileHead)+Lenoffset, fileSize-Lenoffset, SrcfileAddr+sizeof(TranFileHead)+Offset, cplLen);
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	ecpMem_Move(SrcfileAddr+Lenoffset, SrcfileAddr+Offset, cplLen);
#else
	//VMCopyDataToAddr(SrcfileAddr+Lenoffset, HTTP_CommandData_SIZE-Lenoffset, SrcfileAddr+Offset, cplLen, MODE_NO_PROTECT_WRITE);
	VMCopyDataToAddr(SrcfileAddr,Lenoffset, HTTP_CommandData_SIZE-Lenoffset, SrcfileAddr,Offset, cplLen, MODE_NO_PROTECT_WRITE,0,0);
#endif
	
//	WriteTFFileWord(SrcfileAddr, TagLen);//WriteTFBuf(SrcfileAddr,ramExtendResLen_offset,(BYTE *)&TagLen,sizeof(TagLen));      //閲嶅啓L闀垮害, 娉ㄦ剰璇诲彇鏃剁殑澶у皬绔棶棰?
	G_HTTP_CommandData_Length = TagLen;
#endif
}
/*
*@funtion    :IF_HTTPS_HandShake 
*
*@description: 判断当前的bip数据是https的数据，并且在https握手期间
*
*@param      :
*
*@return     :true or false, 
*
*@history    :create- jiangqb @20200319   
*/

BYTE IF_HTTPS_HandShake()
{
#if GP__AMENDMENT__B__FOR_MASK
	if((G_TLSSecurityParameters.serverSeqNum < 1) && (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS))			//handshake锛? 骞朵笖榛樿鎻℃墜鏈熼棿鐨勬暟鎹皬浜? sizeof(tempBuf)
	{	
		return TRUE;
	}
#endif
	return FALSE;
}


/*
*@funtion    :IF_OpenChannel_Retry 
*
*@description: openchannel 主动了命令重试
*
*@param      :BYTE result	上一次openchannel的结果
*
*@return     :openchannel的结果 
*
*@history    :create- jiangqb @20220428  
*/

BYTE IF_OpenChannel_Retry(BYTE result)
{
#if GP__AMENDMENT__B__FOR_MASK
	
	//根据电信测试需求：中国电信用户卡需求白皮书-OTA下载功能分册(2021.V1).pdf 的 5.3章节, 响应为1X/2X时重新发送OpenChannel命令一次
	if(JcvmConfig_GetOPerator() == OPERATOR_CHINATELECOM)
	{
		if((CMD_Success != result) && (0x10 <= result) && (result <= 0x2F))
		{
			result = BIPModuleOpenChannel(ChannelDataType_HTTPS);
		}
	}

#endif	

	return result;
}



/*
*@funtion    :IF_AnnexB_ChangeComNUM 
*
*@description: 改变已执行的http命令个数， 清零，递增，递减， 所有对变量G_HTTPSStatus.HttpApduNum进行改变的操作都放在本函数
*
*@param      :	BYTE mode :ComNUM_CLEAR/ComNUM_INCREASE/ComNUM_DECREASE
*
*@return     :void 
*
*@history    :create- jiangqb @20220506 *缺陷编号385，http下行数据执行apdu个数的变动在本函数中赋值
*/
void IF_AnnexB_ChangeComNUM(BYTE mode)
{
#if GP__AMENDMENT__B__FOR_MASK	
	if(G_Source_Type == SourceType_HTTPS)
	{
		if (mode == ComNUM_CLEAR)
		{
			G_HTTPSStatus.HttpApduNum = 0;
		}
		else if (mode == ComNUM_INCREASE)
		{
			G_HTTPSStatus.HttpApduNum ++;
		}
		else if (mode == ComNUM_DECREASE)
		{
			G_HTTPSStatus.HttpApduNum --;
		}
		else
		{
			return;
		}
	}
#endif
	return;
}

