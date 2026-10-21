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
* history:    V1.0
*
************************************************************************************************************/
#include "Interface_DNS.h"
#include "profile.h"

#if GP__AMENDMENT__B__DNS_Resolution
#include "DNS.h"
#endif

#include "hard_api.h"
#include "gsmpublic.h"
#include "fileAction.h"
#include "Filesystem.h"
#include "TlsMainFlow.h"
#include "opFunction.h"
#include "HttpProcess.h"
#include "SecurityDomainData.h"
#include "Interface_TimerModule.h"
#include "Interface_AnnexB.h"

/*
*#function   :IF_DNS_GetOpenChannelPara
*
*@description:获取DNS开通道的相关参数
*
*@param      :BYTE *para    获取到的开通道参数的数据TLV+TLV格式
*
*@return     :BYTE 返回para中的有效数据
*
*@history    :created bu xxxx@xxxxxxxx
*/
BYTE IF_DNS_GetOpenChannelPara(BYTE *para)
{
#if GP__AMENDMENT__B__DNS_Resolution
	return GetOpenChannelParaForDNS(para);
#else
	return 0x00;
#endif	
}
/*
*#function   :IF_DNS_GetOpenChannelPara
*
*@description:获取DNS开通道的相关参数
*
*@param      :BYTE *para    获取到的开通道参数的数据TLV+TLV格式
*
*@return     :BYTE 返回para中的有效数据
*
*@history    :created bu xxxx@xxxxxxxx
*/
BYTE IF_RAS_GetOpenChannelPara(BYTE *para)
{
#if GP__AMENDMENT__B__DNS_Resolution
	return GetOpenChannelParaForRAS(para);
#else
	return 0x00;
#endif
}
/*
*@function   :IF_DNS_TriggerCheck
*
*@description: http的触发参数中存在dns的触发参数时, 对整个http触发参数的tlv格式进行判断
*
*@See        :GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf  3.7  Administration Session Triggering Parameters
*
*@param      :UWORD paraAddr 		http触发参数的地址, 81开头
*			 :WORD dnsParaOffset	dns参数的tag-B3在 paraAddr 的偏移
*			 :WORD allLen 			http触发参数的总长度
*
*@return     :TRUE 表示触发参数符合规范要求， FALSE表示不符合
*
*@history    :V1.0  created by jiangqingbin@20211229
*/
BYTE IF_DNS_TriggerCheck(UWORD paraAddr, WORD dnsParaOffset, WORD allLen)
{
#if GP__AMENDMENT__B__DNS_Resolution
	BYTE xdata triggerTag[8];
	WORD idata offset, length;
	
	offset = 0;
	
	ecpMem_Set(triggerTag, 00, sizeof(triggerTag));
				
	VMReadBuf(&triggerTag[0], paraAddr + dnsParaOffset, sizeof(triggerTag)); 	

	if (triggerTag[0] == 0xB3)
	{
		length = GetBerTlvLength(&triggerTag[1], &offset);

		if ((length + offset + dnsParaOffset) == allLen)	
		{
			return TRUE;
		}
	}
	
	return FALSE;
#else
	return FALSE;
#endif		
}



/*
*@function   :IF_DNS_TriggerParaAnalyse
*
*@description: http的触发参数中存在dns的触发参数时, 对整个DNS参数进行解析, 解析成功后把FQDN和DNS Connection Parameters分别写到文件EF_HTTPTriggerPara
*				的记录号 HTTP_TriggerPara_FQDN_Index 和记录号 HTTP_TriggerPara_DNSPara_Index 中;
*
*@See        :GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf  3.7  Administration Session Triggering Parameters
*
*@param      :UWORD paraAddr 		http触发参数的地址, 81开头
*			 :WORD dnsParaOffset	dns参数的tag-B3在 paraAddr 的偏移
*			 :WORD allLen 			http触发参数的总长度
*
*@return     :TRUE 表示触发参数符合规范要求， FALSE表示参数解析出错
*
*@history    :V1.0  created by jiangqingbin@20211229
*/
BYTE IF_DNS_TriggerParaAnalyse(UWORD paraAddr, WORD dnsParaOffset, WORD allLen)
{
#if GP__AMENDMENT__B__DNS_Resolution
	BYTE xdata triggerTag[8];
	WORD idata pos, offset, alllength, subLen;
	UWORD xdata triggerParaFileAddr;
		
	offset = dnsParaOffset;
	
	ecpMem_Set(triggerTag, 00, sizeof(triggerTag));
	
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  
	
	VMReadBuf(triggerTag, paraAddr+offset, 8);
	
	if (triggerTag[0] == 0xB3)
	{
		alllength = GetBerTlvLength(triggerTag+1, &offset);
		
		alllength += offset;	//B3tag的 TLV 总长度, offset point to tag D6
		
		VMReadBuf(&triggerTag[0],paraAddr+offset, 8); 

		if (triggerTag[0] == DNS_TriggerPara_Fully_Qualified_Domain_Name)
		{
			subLen = GetBerTlvLength(triggerTag+1, &offset);		//返回L值， pos是LV字节数
		
			if (subLen > 0xFD)
			{
				return FALSE;
			}
			
			VMReadBuf(ResData, paraAddr+offset, subLen); 
			
			if (WriteLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_FQDN_Index, ResData, subLen) == FALSE)
			{
				return FALSE;
			}
			
			offset += subLen;		//offset point to next tag
		}
		
		if (offset == allLen)
		{
			return TRUE;		//只存在D6 TAG， 不存在FAtag
		}
		else
		{
			VMReadBuf(&triggerTag[0],paraAddr+offset, 8); 
		
			subLen = GetBerTlvLength(triggerTag+1, &offset);

			if ((triggerTag[0] != DNS_TriggerPara_Connection_Parameters)||(subLen > 0xFD))
			{
				return FALSE;
			}
			
			VMReadBuf(ResData, paraAddr+offset, subLen); 
			
			if (WriteLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_DNSPara_Index, ResData, subLen) == FALSE)
			{
				return FALSE;
			}
			
			offset += subLen;
		}
			
		if (offset == allLen)
		{
			return TRUE;
		}
	}
	
	return FALSE;
	
#else
	return FALSE;
#endif		
}


/*
*#function   :IF_DNSDataAvaiable
*
*@description:DNS的data available事件处理
*
*@param      :BYTE channelDataLen   表示当前是处在C2的D6事件中，channelDataLen表示待接收的数据长度，如果为0则表示数据接收完成
*@param      :WORD offset           表示当前bipReceive文件中有效数据的个数
*
*@return     :void
*
*@history    :created by jiangqingbin@20220106
*/
void IF_DNSDataAvaiable(BYTE channelDataLen, WORD offset)
{
#if GP__AMENDMENT__B__DNS_Resolution
	//ProcessDNSResponse(channelDataLen, offset);
	DNSResolution_Main();
#endif		
}

/*
*#function   :IF_DNSWithTLSEndState
*
*@description:DNS过程执行到TLS_END_STATE状态时，如果当前是Establish TLS过程发生的Connection Breakdown则需要进行RAS IP重试；
*             如果是HTTP Exchange过程发生的Connection Breakdown则需要进行HTTP SESSION重试.
*
*@param      :void
*
*@return     :TRUE表示该函数已经处理完，返回之后不需要继续执行；FALSE表示返回后需要继续执行
*
*@history    :created by jiangqingbin@20220106
*/
u8 IF_DNSWithTLSEndState(void)
{
#if GP__AMENDMENT__B__DNS_Resolution
	if(G_DNSResolutionProcedureState.curState == DNSResolutionProcedureState_ESTABLISH_TLS)
	{
		G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_RAS_IP;

		if(G_RASIPConnectStatus.tryCounter > 0)
		{
			RASConnect_StartTimer(); // todo 这一步的计时器结束时，需要进入 openChannel_ConnectRASIP， 那么这一步是否可以直接放在openChannel_ConnectRASIP函数内部？

			return TRUE;
		}
		else
		{
			G_RASIPConnectStatus.resume = FALSE;

			DNSResolution_Main();

			return TRUE;
		}
	}
	else if(G_DNSResolutionProcedureState.curState == DNSResolutionProcedureState_HTTP_EXCHANGE)
	{
		if(G_HTTPSStatus.RetryConterRemain > 0)
		{
			G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS1;

			IF_AnnexB_StartTimerAfterUnNormalEnd(); // todo 这一步的计时器结束时，需要进入DNSResolution_DNS1过程

			return TRUE;
		}
		else
		{
			DNSResolutionInitVariable();

			G_HTTPSStatus.httpException = HTTP_RETRY_EXHAUSTED;

			return FALSE;
		}
	}
	else
	{
		DNSResolutionInitVariable();

		return FALSE;
	}
#endif
}

