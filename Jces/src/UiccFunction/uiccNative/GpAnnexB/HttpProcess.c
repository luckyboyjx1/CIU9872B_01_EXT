/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   HttpProcess.c
*
* Description: 该文件主要用于存放HTTPS层相关的函数
*
* @see        :GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0 :create @20180514 ,该文档主要存放用于处理HTTP的主流程
				    
*************************************************************************************************************/
#include "Scp81Connection.h"
#include "HttpProcess.h"
#include "TlsMainFlow.h"
#include "FileAction.h"
#include "BIPModuleProcess.h"
#include "GPAnnexbAdminSessionPara.h"
#include "Gsmpublic.h"
#include "Stkhead.h"
//#include "ToolkitApiPublic.h"
#include "TlsAlgorithm.h"
//#include "ToolkitRFM.h"
#include "Interface_AnnexB.h"
//#include "mask.h"
//#include "Stkalert.h"
#include "stkhead.h"
#include "fileSystem.h"
//#include "STKRemoteAppManagement.h"
#include "interpreter.h"
//#include "platform_api.h"
//#include "packageloc.h"
#include "CompactManage.h"
#include "opFunction.h"
//#include "TransFormat.h"
#include "Interface_LTEFromBoot.h"
//#include "platform_api.h"
//#include "EUICCM2MAPDUPublic.h"
//#include "uicc_M2M.h"
//#include    "uicc_interface.h"
#include    "JcvmConfig.h"
#include    "ToolkitRFM.h"

#if COS_UPDATE_ONLINE_BY_NativeOS
#include    "UpCos.h"
#endif

#if GP__AMENDMENT__B__FOR_MASK

/****************http Head Content*******************************************/
//See: <rfc2616(HTTP1.1).pdf>   Each header field consists of a name followed by a colon (“: ”) and the field value.  Field names are case-insensitive.(域名不区分大小写)
HTTPSDataStatus xdata G_HTTPSStatus;

#if (GP__AMENDMENT__B__createSCP81Connection_FOR_MASK || GP__AMENDMENT__B__requestHTTPAdministrationSession )
BYTE xdata G_HTTP_APITriggerTag;      // 用于标记标记API方式下来的触发请求，有没有实际执行过HTTP触发，即有没有执行过IF_AnnexB_StartHttpSessionByAPI(对于同一个API触发请求，该接口函数只允许被触发一次)。初始值为HTTPTriggeredByAPPHasDone
#endif                                   // 对于短信触发来说，G_HTTP_APITriggerTag是无效的

WORD xdata G_HTTP_CommandData_Length;
BYTE code HTTP_CRLF[] = {0x0D,0x0A}; //CRLF

BYTE code HTTP_CHUNK_0CRLF[] = {0x30,0x0D,0x0A,0x0D,0x0A}; //0CRLFCRLF

BYTE code HTTP_CHUNK_0CRLF_2[] = {0x30,0x0D,0x0A};         //0CRLF  兼容以300D0A做为chunk结束标记的情况

BYTE code HTTP_POST[] = {0x50,0x4F,0x53,0x54,0x20};  //POST+空格

BYTE code HTTP_VERSION_11[] = {0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31}; //dadta = //HTTP/1.1

BYTE code HTTP_RESPONSE_STATUS_204[] = {0x32,0x30,0x34}; //204

BYTE code HTTP_HOST[] = {0x68,0x6F,0x73,0x74,0x3A,0x20};   //host:+空格
			
BYTE code HTTP_PROTOCOL_HEAD[] = {0x78,0x2D,0x61,0x64,0x6D,0x69,0x6E,0x2D,0x70,0x72,0x6F,0x74,0x6F,0x63,0x6F,0x6C,0x3A}; //x-admin-protocol： 
			
BYTE code HTTP_PROTOCOL_VALUE[] = {0x67,0x6C,0x6F,0x62,0x61,0x6C,0x70,0x6C,0x61,0x74,0x66,0x6F,0x72,0x6D,0x2D,0x72,0x65,0x6D,0x6F,0x74,0x65,0x2D,0x61,0x64,0x6D,0x69,0x6E,0x2F,0x31,0x2E,0x30};//globalplatform-remote-admin/1.0

BYTE code HTTP_FROM[] = {0x78,0x2D,0x61,0x64,0x6D,0x69,0x6E,0x2D,0x66,0x72,0x6F,0x6D,0x3A,0x20}; //x-admin-from:+空格

BYTE code HTTP_NEXT_URI[] = {0x58,0x2D,0x41,0x64,0x6D,0x69,0x6E,0x2D,0x4E,0x65,0x78,0x74,0x2D,0x55,0x52,0x49,0x3A}; // X-Admin-Next-URI:	

//content-type:
BYTE code HTTP_CONTENT_TYPE[] = {0x63,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x2D,0x74,0x79,0x70,0x65,0x3A}; 	

// application/vnd.globalplatform.card-content-mgt;version=1.0				//RAM下行
BYTE code HTTP_CONTENTTYPE_RES_RAM_VALUE[] = {0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x76,0x6E,0x64,0x2E,0x67,0x6C,0x6F,0x62,0x61,0x6C,0x70,0x6C,0x61,0x74,0x66,0x6F,0x72,0x6D,0x2E,0x63,0x61,0x72,0x64,0x2D,0x63,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x2D,0x6D,0x67,0x74,0x3B,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x31,0x2E,0x30}; 

// application/vnd.etsi.scp.command-data;version=1.0						//RFM下行
BYTE code HTTP_CONTENTTYPE_RES_RFM_VALUE[] = {0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x76,0x6E,0x64,0x2E,0x65,0x74,0x73,0x69,0x2E,0x73,0x63,0x70,0x2E,0x63,0x6F,0x6D,0x6D,0x61,0x6E,0x64,0x2D,0x64,0x61,0x74,0x61,0x3B,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x31,0x2E,0x30}; 

// application/vnd.globalplatform.card-content-mgt-response;version=1.0		//RAM上行
BYTE code HTTP_CONTENTTYPE_RES_RAM_VALUE_POST[] = {0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x76,0x6E,0x64,0x2E,0x67,0x6C,0x6F,0x62,0x61,0x6C,0x70,0x6C,0x61,0x74,0x66,0x6F,0x72,0x6D,0x2E,0x63,0x61,0x72,0x64,0x2D,0x63,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x2D,0x6D,0x67,0x74,0x2D,0x72,0x65,0x73,0x70,0x6F,0x6E,0x73,0x65,0x3B,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x31,0x2E,0x30};

// application/vnd.etsi.scp.responsedata;version=1.0						//RFM上行
BYTE code HTTP_CONTENTTYPE_RES_RFM_VALUE_POST[] = {0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x76,0x6E,0x64,0x2E,0x65,0x74,0x73,0x69,0x2E,0x73,0x63,0x70,0x2E,0x72,0x65,0x73,0x70,0x6F,0x6E,0x73,0x65,0x2D,0x64,0x61,0x74,0x61,0x3B,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x31,0x2E,0x30};

#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
//application/vnd.etsi.scp.response											//scp81connection  下行
BYTE code SCP81_CONTENTTYPE_RESPONSE_VALUE[] = {0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x76,0x6E,0x64,0x2E,0x65,0x74,0x73,0x69,0x2E,0x73,0x63,0x70,0x2E,0x72,0x65,0x73,0x70,0x6F,0x6E,0x73,0x65}; 

//application/vnd.etsi.scp.request.											//scp81connection  上行
BYTE code SCP81_CONTENTTYPE_REQUEST_VALUE[] = {0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x76,0x6E,0x64,0x2E,0x65,0x74,0x73,0x69,0x2E,0x73,0x63,0x70,0x2E,0x72,0x65,0x71,0x75,0x65,0x73,0x74,0x2E}; 
#endif	
	
BYTE code HTTP_TARGETED_AID[] = {0x78,0x2D,0x61,0x64,0x6D,0x69,0x6E,0x2D,0x74,0x61,0x72,0x67,0x65,0x74,0x65,0x64,0x2D,0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x3A}; // X-Admin-Targeted-Application:
		
BYTE code HTTP_CONTENT_LENGTH[] = {0x63,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x2D,0x6C,0x65,0x6E,0x67,0x74,0x68,0x3A}; //content-length:

BYTE code HTTP_TRANSFER_ENCODING[] = {0x74,0x72,0x61,0x6E,0x73,0x66,0x65,0x72,0x2D,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3A}; //transfer-encoding:

BYTE code HTTP_CHUNKED[] = {0x63,0x68,0x75,0x6E,0x6B,0x65,0x64};	 //chunked

BYTE code HTTP_SCRIPT_STATUS[] = {0x78,0x2D,0x61,0x64,0x6D,0x69,0x6E,0x2D,0x73,0x63,0x72,0x69,0x70,0x74,0x2D,0x73,0x74,0x61,0x74,0x75,0x73,0x3A,0x20}; //x-admin-script-status:+空格
	
BYTE code HTTP_SCRIPT_STATUS_OK[] = {0x6F,0x6B}; //ok

BYTE code HTTP_SCRIPT_STATUS_NOT_AP[] = {0x75,0x6E,0x6B,0x6E,0x6F,0x77,0x6E,0x2D,0x61,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E}; //unknown-application

BYTE code HTTP_SCRIPT_STATUS_NOT_SD[] = {0x6E,0x6F,0x74,0x2D,0x61,0x2D,0x73,0x65,0x63,0x75,0x72,0x69,0x74,0x79,0x2D,0x64,0x6F,0x6D,0x61,0x69,0x6E}; //not-a-security-domain

BYTE code HTTP_SCRIPTSTATE_SEC_ERR[] = {0x73,0x65,0x63,0x75,0x72,0x69,0x74,0x79,0x2D,0x65,0x72,0x72,0x6F,0x72};		//security-error
	
BYTE code HTTP_RESUME[] = {0x78,0x2D,0x61,0x64,0x6D,0x69,0x6E,0x2D,0x72,0x65,0x73,0x75,0x6D,0x65,0x3A,0x20}; 	//x-admin-resume:+空格

BYTE code HTTP_TRUE[] = {0x74,0x72,0x75,0x65}; //true

BYTE code HTTP_NOTIFICATION_DATA[] = {0x3F,0x6D,0x73,0x67,0x3D};		//?msg=

//101220 指定的usim/csim/isim的aid前7个字节
//BYTE code SIMPartialAID[] =  {0xA0,0x00,0x00,0x05,0x33,0x81,0x00};  
//BYTE code USIMPartialAID[] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02};  	//101220 附录E
//BYTE code CSIMPartialAID[] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02};  	//101220 附录E
//BYTE code ISIMPartialAID[] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04};  	//101220 附录E

//移动指定 sim-rfm, usim-rfm, uicc-rfm aid值预配置
//#if UICC__OPERATOR__CHINAMOBILE
//BYTE code SIM_RFM_AID[] =  {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0x03};
//BYTE code UICC_RFM_AID[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0x01};
//BYTE code USIM_RFM_AID[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0x02};
//BYTE code SPECIAL_SSD_USIM_RFM_AID[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0x0A};			//移动特殊需求，USIM_RFM_AID的别名
//#endif

//#if UICC__OPERATOR__CHINATELECOM			//电信指定 sim, sim-rfm, usim, usim-rfm 等aid值预配置
//BYTE code SIM_RFM_AID1[] = 	{0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0x03};
//BYTE code UICC_RFM_AID1[] =  {0xD1,0x56,0x00,0x00,0x40,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0xB0,0x00,0x02,0x05};		//电信测试规范
//BYTE code USIM_RFM_AID1[] =  {0xD1,0x56,0x00,0x00,0x40,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0xB0,0x00,0x21,0x05};		//电信测试规范
//BYTE code ChinaTele_USIMRFMAID[] = {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0x00,0x01};//tpx3.0-esim电信备案测试时转的脚本中，中国电信的USIMRFMAID
//BYTE code CSIM_RFM_AID[] =  {0xD1,0x56,0x00,0x00,0x40,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0xB0,0x00,0x20,0x05};		//电信测试规范

//#endif



/*
*@description:	卡片上电时，https会话开启前，http会话结束后，对https会话过程中必须用到的全局变量进行初始化清零
*
*@function   :InitHttpVariable,
*
*@param      :
*
*@return     :void
*
*@history    :jiangqb-
*            :modify by jiangqingbin@20210412，增加G_HTTP_APITriggerTag全局变量初始化
*/
void InitHttpVariable(void)
{
#if (!GP__AMENDMENT__B__TLS_MasterKey_IN_FILE)
	memset((BYTE *)&G_TLSMaterSecretBuf, 0, sizeof(G_TLSMaterSecretBuf));
#endif

#if (!GP__AMENDMENT__B__TLS_SessionKey_IN_FILE)
	memset((BYTE *)&G_TLSSessionKeyBuf, 0, sizeof(G_TLSSessionKeyBuf));
#endif

#if (!GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE)
	memset((BYTE *)&G_HandShakeHashMD5Buf, 0, sizeof(G_HandShakeHashMD5Buf));
	memset((BYTE *)&G_HandShakeHashMD5RemainDataBuf, 0, sizeof(G_HandShakeHashMD5RemainDataBuf));
	memset((BYTE *)&G_HandShakeHashSha256Buf, 0, sizeof(G_HandShakeHashSha256Buf));
	memset((BYTE *)&G_HandShakeHashSha256RemainDataBuf, 0, sizeof(G_HandShakeHashSha256RemainDataBuf));
#endif
	
	memset((BYTE *)&G_HTTPSStatus, 0, sizeof(G_HTTPSStatus));  	
#if (GP__AMENDMENT__B__createSCP81Connection_FOR_MASK || GP__AMENDMENT__B__requestHTTPAdministrationSession )
	G_HTTP_APITriggerTag = HTTPTriggeredByAPPHasDone;
#endif
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
	memset((BYTE *)&G_SCP81Connection, 0, sizeof(G_SCP81Connection)); 
#endif	
	memset((BYTE *)&G_BIPModuleStatus, 0, sizeof(G_BIPModuleStatus)); 
	memset((BYTE *)&G_TLSSecurityParameters, 0, sizeof(G_TLSSecurityParameters));  
	
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM	
	memset((BYTE *)&G_BIPModule_Session_Data_Buf, 0, sizeof(G_BIPModule_Session_Data_Buf)); 
#endif	

}


/*
*@description:	https会话开启前，对https会话过程中必须用到的文件进行检查
*
*@function   :HttpsSessionFileCheck,
*
*@param      :
*
*@return     :文件存在 TRUE OR FALSE
*
*@history    :jiangqb-20190621
*            :modify by xuxiaojun@20210118 EF_HTTPSCommandData文件去掉，放在E2固定页起始位置HTTP_CommandData_ADDR_START，采用非保护的写
*/
BYTE HttpsSessionFileCheck(void)
{
	UWORD xdata fileAddr;

#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
	fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPTwoSession);

	if (fileAddr == FALSE)
	{
		return FALSE;
	}
#endif

	fileAddr = SelectPrivateTFFile(DF_TLS,EF_TLSsessionData);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
	
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
	fileAddr = SelectPrivateTFFile(DF_TLS,EF_TLSKey);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
#endif
	
	fileAddr = SelectPrivateTFFile(DF_TLS,EF_HTTPTriggerPara);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
/*	
	fileAddr = SelectPrivateTFFile(DF_TLS,EF_HTTPSCommandData);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
*/	
#if !GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	fileAddr = SelectPrivateTFFile(DF_TLS,EF_HTTPSResponseData);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
#endif
/*	
	fileAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Send);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
	
	fileAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Receive);
	
	if (fileAddr == FALSE)
	{
		return FALSE;
	}
*/	
	return TRUE;
}


/*
*@description:拼装HTTP POST Request中的各字段数据,根据对应的属性id 获取数据
*
*@function   :HTTPPatchPostHeadData
*
*@param      :BYTE headId         HTTP POST  Request中HTTP头的各类属性标示
*@param      :BYTE *outBuf        HTTP POST  Request中HTTP中headId的值
*
*@return     :WORD outLength     HTTP POST  Request中HTTP中headId的值的长度
*
*@author     :lx 20180521
*
*@history    :modify by xuxiaojun@20180531 
*            :modify by xuxiaojun@20180803  增加对于retry时上发的第一条HTTP request的处理。增加了一条记录用于记录原始的触发参数。目前这样处理是不完善的，按照规范retry时应该是发上一次数据接收完整的uri，但这个uri并不一定是最原始的触发参数中的uri，所以后面这里还需要完善
*            :modify by jiangqb@20191122	还原代码，重发是nextUri在其他位置处理	
*            :modify by xuxiaojun@20200619  去掉HTTP_TriggerPara_ContentType_Index记录
*
*@NOTE	     :注意该函数中是否需要拼装空格需要参考HTTP_POST等的常量定义中是否包含，如果已经包含了，则该函数中不需要再次拼装
*/
WORD HTTPPatchPostHeadData(BYTE headId, BYTE * outBuf)
{
	UWORD xdata fileAddr;
	WORD idata parameterLen, padOff;

#if M2M_EUICC
	BYTE  idata notificationType,tempLen;
#endif

	padOff = 0;
	
	fileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);                       //触发参数文件
	
	switch(headId)
	{
		case HEADID_POST:       // POST+空格+URI+空格+HTTP/1.1+空格+CRLF 
		{
			memcpy(&outBuf[padOff], &HTTP_POST[0], sizeof(HTTP_POST)); 
			
			padOff += sizeof(HTTP_POST);			
			
			if(G_TLSSecurityParameters.clientSeqNum == 1)  //如果当前是第一条请求，则用触发参数里的。否则用服务器下发的HTTP头里的。 可以同时处理retry的情况
			{
				parameterLen = ReadLFFileValue(fileAddr,HTTP_TriggerPara_URIPara_Index,&outBuf[padOff]);
			}
			else
			{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
				parameterLen = BuildWord(&HTTP_NextURIData_ADDR[0]);
				ecpMem_Copy(&outBuf[padOff], &HTTP_NextURIData_ADDR[2], parameterLen);
#else				
				parameterLen = ReadLFFileValue(fileAddr,HTTP_TriggerPara_NextURIPara_Index,&outBuf[padOff]);
#endif
			}		
			
			padOff += (BYTE)parameterLen;
			
#if M2M_EUICC	//拼装通知相关信息
			if (G_IsSendHttpsNotification == TRUE)
			{
				memcpy(&outBuf[padOff], &HTTP_NOTIFICATION_DATA[0], sizeof(HTTP_NOTIFICATION_DATA));

				padOff += sizeof(HTTP_NOTIFICATION_DATA);

				notificationType = IF_GetNotifitionType();

				tempLen = GenerateNotificationMessage((notificationType==NOTIFICATION_FIRST_TIME)?NOTIFICATION_FIRST_NETWORD_ATTACHMENT:notificationType,G_IMEI_FOR_NOTIFICATION,&ResData[0]);

				padOff += HexStrToAscStr(&ResData[0], &outBuf[padOff], tempLen);				//将通知数据转成acsii值
			}
#endif

			outBuf[padOff++] = SPACE;   //拼装post request 中URI后的空格
			
			memcpy(&outBuf[padOff], &HTTP_VERSION_11[0], sizeof(HTTP_VERSION_11));
			
			padOff += sizeof(HTTP_VERSION_11);

			break;
		}
		
		case HEADID_HOST:              //拼装 Host:+空格+<Administration Host>+空格+CRLF
		{
			memcpy(&outBuf[padOff], &HTTP_HOST[0], sizeof(HTTP_HOST));
		
			padOff += sizeof(HTTP_HOST);
		
			parameterLen = ReadLFFileValue(fileAddr,HTTP_TriggerPara_HostPara_Index,&outBuf[padOff]);
		
			padOff += (BYTE)parameterLen;

			break;
		}
					
		case HEADID_XAdmin_Protocol:          //拼装 X-Admin-Protocol:+空格+globalplatform-remote-admin/1.0+空格+CRLF
		{
			memcpy(&outBuf[padOff], &HTTP_PROTOCOL_HEAD[0], sizeof(HTTP_PROTOCOL_HEAD));
		
			padOff += sizeof(HTTP_PROTOCOL_HEAD);
			
			outBuf[padOff++] = SPACE;
		
			memcpy(&outBuf[padOff], &HTTP_PROTOCOL_VALUE[0], sizeof(HTTP_PROTOCOL_VALUE));
		
			padOff += sizeof(HTTP_PROTOCOL_VALUE);

			break;
		}
		
		case HEADID_XAdmin_From:             //拼装 X-Admin-From:+空格+<Agent ID>+空格+CRLF
		{
			memcpy(&outBuf[padOff], &HTTP_FROM[0], sizeof(HTTP_FROM));
		
			padOff += sizeof(HTTP_FROM);
			
			parameterLen = ReadLFFileValue(fileAddr,HTTP_TriggerPara_AgentIdPara_Index,&outBuf[padOff]);
		
			padOff += (BYTE)parameterLen;

			break;
		}		
		
		case HEADID_Content_Type:            //拼装  Content-Type:+空格+application/vnd.globalplatform.card - content- mgt -res ponse;version=1.0+空格+CRLF
		{
			memcpy(&outBuf[padOff], &HTTP_CONTENT_TYPE[0], sizeof(HTTP_CONTENT_TYPE));
		
			padOff += sizeof(HTTP_CONTENT_TYPE);
			
			outBuf[padOff++] = SPACE;

#if UICC__FUNCTION__RAM		//如果EUICC__FUNCTION_RAM没有开启时，G_HTTPSStatus.httpCommandTyp不会等于HTTP_Command_Type_RAM
			if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RAM)
			{
				memcpy(&outBuf[padOff], &HTTP_CONTENTTYPE_RES_RAM_VALUE_POST[0], sizeof(HTTP_CONTENTTYPE_RES_RAM_VALUE_POST));
				
				padOff += sizeof(HTTP_CONTENTTYPE_RES_RAM_VALUE_POST);
			}
			else
#endif
			if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM)
			{					
				memcpy(&outBuf[padOff], &HTTP_CONTENTTYPE_RES_RFM_VALUE_POST[0], sizeof(HTTP_CONTENTTYPE_RES_RFM_VALUE_POST));
				
				padOff += sizeof(HTTP_CONTENTTYPE_RES_RFM_VALUE_POST);
			}
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
			else if (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection)
			{
				memcpy(&outBuf[padOff], &SCP81_CONTENTTYPE_REQUEST_VALUE[0], sizeof(SCP81_CONTENTTYPE_REQUEST_VALUE));
				
				padOff += sizeof(SCP81_CONTENTTYPE_REQUEST_VALUE);		
			}
#endif
			break;
		}
			
		case HEADID_Content_Length:             // 拼装Content-Length:+空格+xxxx+空格+CRLF，xxxx部分数据返回出去了再拼
		{
			memcpy(&outBuf[padOff], &HTTP_CONTENT_LENGTH[0], sizeof(HTTP_CONTENT_LENGTH));
		
			padOff += sizeof(HTTP_CONTENT_LENGTH);
			
			outBuf[padOff++] = SPACE;
			
			return padOff;
		}
			
		case HEADID_Transfer_Encoding:           // 拼装Transfer-Encoding:+空格+chunked+空格+CRLF
		{
			memcpy(&outBuf[padOff], &HTTP_TRANSFER_ENCODING[0], sizeof(HTTP_TRANSFER_ENCODING));
		
			padOff += sizeof(HTTP_TRANSFER_ENCODING);
			
			outBuf[padOff++] = SPACE;
			
			memcpy(&outBuf[padOff], &HTTP_CHUNKED[0], sizeof(HTTP_CHUNKED));
		
			padOff += sizeof(HTTP_CHUNKED);

			break;
		}
			
		case HEADID_XAdminScript_Status:        // 拼装X-Admin-Script-Status:+空格+<script- status>+空格+CRLF     The “ X - Admin- Script - Status ”  header val ue is used to return the delivery status of the previous remote APDU format string
		{
			memcpy(&outBuf[padOff], &HTTP_SCRIPT_STATUS[0], sizeof(HTTP_SCRIPT_STATUS));
		
			padOff += sizeof(HTTP_SCRIPT_STATUS);
			
			if (G_HTTPSStatus.httpException == HTTP_OK)
			{
				memcpy(&outBuf[padOff], &HTTP_SCRIPT_STATUS_OK[0], sizeof(HTTP_SCRIPT_STATUS_OK));
					
				padOff += sizeof(HTTP_SCRIPT_STATUS_OK);
			}
			else if (G_HTTPSStatus.httpException == HTTP_UNKNOWN_APP_ERROR)
			{
				memcpy(&outBuf[padOff], &HTTP_SCRIPT_STATUS_NOT_AP[0], sizeof(HTTP_SCRIPT_STATUS_NOT_AP));
					
				padOff += sizeof(HTTP_SCRIPT_STATUS_NOT_AP);
			}
			else if(G_HTTPSStatus.httpException == HTTP_NOT_SD_ERROR)
			{	
				memcpy(&outBuf[padOff], &HTTP_SCRIPT_STATUS_NOT_SD[0], sizeof(HTTP_SCRIPT_STATUS_NOT_SD));
					
				padOff += sizeof(HTTP_SCRIPT_STATUS_NOT_SD);
			}
			else if(G_HTTPSStatus.httpException == HTTP_SEC_ERR)
			{	
				memcpy(&outBuf[padOff], &HTTP_SCRIPTSTATE_SEC_ERR[0], sizeof(HTTP_SCRIPTSTATE_SEC_ERR));
					
				padOff += sizeof(HTTP_SCRIPTSTATE_SEC_ERR);
			}

			break;
		}		
			
		case HEADID_XAdmin_Resume :    //拼装 X-Admin-Resume:+空格+true+空格+CRLF
		{
			memcpy(&outBuf[padOff], &HTTP_RESUME[0], sizeof(HTTP_RESUME));
		
			padOff += sizeof(HTTP_RESUME);
			
			memcpy(&outBuf[padOff], &HTTP_TRUE[0], sizeof(HTTP_TRUE));
		
			padOff += sizeof(HTTP_TRUE);
			
			break;
		}
		default : 			  //此处为了白盒测试添加
			break;
	}

#if GP__AMENDMENT__B__POST_WITH_SPACE_AND_CRLF
	outBuf[padOff++] = SPACE;  //这里是根据测试部脚本 <电信AmendmentB测试脚本-20150618-C网&&G网>，CRLF前面不带空格。理论上来说，这里带也是可以的
#endif

	memcpy(&outBuf[padOff], &HTTP_CRLF[0], sizeof(HTTP_CRLF));
			
	padOff += sizeof(HTTP_CRLF);
	
	return padOff;
}
/*
*@description: TLS层握手成功后，进行https协议层的初始化，如初始化HTTP层的全局变量、清空文件等操作
*
*@function   : HttpProtocolInit
*
*@param      : void
*
*@return     : void
*
*@author     :
*
*@history   :modify by xuxiaojun@20180904 清空HTTP状态变量，将HTTP_DataType_HEAD的宏定义为0则初始值就为HTTP_DataType_HEAD。 如果HTTP_DataType_HEAD的宏定义不为0则需要执行G_HTTPSStatus.httpDataStatus = HTTP_DataType_HEAD
*/
void HttpProtocolInit(void)
{	
	G_HTTPSStatus.IsHTTPCommandExecueError = POR_OK;
										
	G_HTTP_CommandData_Length = 0x00; //ClearFileFirstWord(DF_TLS, EF_HTTPSCommandData);

	G_BIPModule_Send_Length = 0x00; // ClearFileFirstWord(DF_BIPModule, EF_BIPModule_Send);
	
	memset((BYTE *)&G_HTTPSStatus, 0, 9);  	//前8字节清空，后续的不能清空，可以考虑分开两个结构体
}
/*
*@description:HTTP层拼装Post request数据，包括HTTP头+HTTP体，其中HTTP体的内容数据存放于EF_HTTPSResponseData 中(该文件为LV结构),  拼装后的数据存放于EF_BIPModule_Send 文件中
*            :考虑到缓存问题，拼装的数据直接放在EF_BIPModule_Send文件的对应位置，由于采用的都对称算法，所以数据的位置是固定的，该函数与TLSPatchApplicationOutData结合着看
*@function   :HttpPatchPostResponseData
*
*@see        :<GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf>  First request sent by the Security Domain:
*                                                                       HTTP POST  Request of Security Domain
*@param      :void
*
*@return     :void
*
*@author     :lx 20180518
*
*@history    :modify bu xuxiaojun@20180531
*			 :modify by jiangqb@20191122, 为加快速度，少写文件， 默认https头拼装在tempBuf中，如果数据长度大于tempBuf时则写入文件
*			 :modify by jiangqb@20210513, 上行数据是chunk模式时，使用一个chunk，不再使用多个chunk封装数据（电信外场测试问题-国密卡https支持能力补充测试时，脚本需求，但是规范中没有明确指定使用单个还是多个chunk）
*			 :modify by jiangqb@20220418, 上行数据是chunk模式时，最新电信需求白皮书规定最大长度是0x400，修改成多个chunk块模式并且块默认长度是0x400，同步epx2.1-华大 中国电信产业化的修改到此
*
*@NOTE       :该函数借用了tempBuf\ResData
*			  todo 当 ClientSequence序号为1时，表示第一条http post数据的发送
*/
BYTE HttpPatchPostResponseData(void)
{	
	BYTE idata sdId, flag, ivLgth;
	WORD idata bipSendFileOff, dataLength, off, index, tempLen, fileSize;
	UWORD xdata fileAddr, bipSendFileAddr;
	BYTE idata tmp[2];
	
	off = 0;	
	
	sdId = 0;	
	
	tempLen	= 0;
	
	flag = FALSE;
		
	dataLength = 0;	
				
	if(TLSClientUpMsgGnerateIV() == TRUE)                                           //判断上行是否存在IV
	{
		ivLgth = G_TLSSecurityParameters.blockLength;
	}
	else
	{
		ivLgth = 0;
	}
	
	
//	bipSendFileAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Receive);
	bipSendFileAddr = (UWORD)BIPModule_Receive_ADDR_START;
	
//	fileSize = GetTFSize(bipSendFileAddr);
	fileSize = BIPModule_Receive_SIZE;
	
	//HEADID_POST
	bipSendFileOff = 2 + TLS_RecordHead_LENGTH + ivLgth; 		//预留前面数据存在长度值，record头， iv
	
	off += HTTPPatchPostHeadData(HEADID_POST, &tempBuf[off]);            //拼装 POST+空格+URI+空格+HTTP/1.1+空格+CRLF 
				
//	WriteTFBuf(bipSendFileAddr, bipSendFileOff, &tempBuf[0], off);       //第一次写数据，因为URI可能很长，所有分开写数据
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#else
	WriteEepromWithoutProtect(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#endif
	
	bipSendFileOff += off;
	
	off = 0;
		
	off += HTTPPatchPostHeadData(HEADID_HOST, &tempBuf[off]);            //拼装 Host:+空格+<Administration Host>+空格+CRLF
	
	off += HTTPPatchPostHeadData(HEADID_XAdmin_Protocol, &tempBuf[off]); //拼装 X-Admin-Protocol:+空格+globalplatform-remote-admin/1.0+空格+CRLF
	
//	WriteTFBuf(bipSendFileAddr, bipSendFileOff, &tempBuf[0], off);       //避免tempBuf越界，所以分开写
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#else
	WriteEepromWithoutProtect(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#endif
	
	bipSendFileOff += off;
	
	off = 0;
		
	off += HTTPPatchPostHeadData(HEADID_XAdmin_From, &tempBuf[off]);     //拼装 X-Admin-From:+空格+<Agent ID>+空格+CRLF
	
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY		
 	if (G_HTTPSStatus.HTTPIsResume == TRUE)
	{	
		G_HTTPSStatus.httpException = HTTP_OK;							//retry-HEADID_Content_Length, 重发上一次没有成功发送的数据
		
		G_HTTPSStatus.httpCommandType = HTTP_Command_Type_RAM;			//retry-HEADID_Content_Type,重发时默认是RAM，如果还有RFM重发的话可以在文件中用一个字节记录上一次的httpCommandType
	}
#endif	
//	WriteTFBuf(bipSendFileAddr, bipSendFileOff, &tempBuf[0], off);        //把前面的必须存在的头写到EF_BIPModule_Send文件中
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#else
	WriteEepromWithoutProtect(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#endif
	
	bipSendFileOff += off;
	
	off = 0; //以上是规范定义必须存在的http头
	fileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSResponseData);
	
#if (M2M_EUICC && GP__AMENDMENT__B__SUPPORT_SESSION_RETRY)		

	//此处是epx_suite优化http方案，上行数据拼装不再使用ram空间的修改时，当时SE-5G修改后没有做M2M测试保证，
	//这里当初的逻辑应该是http会话在isdr下载profile过程被中断后，重新连接http会话后，不能接着上一次继续下载profile，而应该重头开始profile的下载流程，所以此处不能读取上一次的por上发。 -jiangqb-20210319-所有M2M使用http下载profile的用例
	if ((IF_IsProfileLoadingSession() == TRUE)  && (G_HTTPSStatus.TriggeredSD == ISDR_APPID) && (G_HTTPSStatus.HTTPIsResume == TRUE))//用例:(eSIM_Test_M2M/WQ_Offline_Test/TC_ES8_DownloadAndInstallation_HTTPS_Error_DownloadInterrupted_03)

	{
		dataLength = 0;						//下载isdr
	}
	else
#endif
	{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
		dataLength = BuildWord(HTTP_ResponseData_ADDR);
#else
		dataLength = ReadTFFileWord(fileAddr);
#endif
	}
	
	//HEADID_Content_Type
	if ((dataLength != 0) && (G_HTTPSStatus.httpException == HTTP_OK)) //有响应数据
	{		
		off += HTTPPatchPostHeadData(HEADID_Content_Type, &tempBuf[off]);
		
		//设备版用例eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0-->TC_ES6_UPOL1MNO_3_UpdatePOL1byMNO_HTTPS_01_Nominal_Case_No_Rule用例中,UL测试MNO-SD的会话在0303版本中下行没有指定是chunk，但是上行指定要求chunk
		if (G_HTTPSStatus.httpContentType == HTTP_ContentType_Null)		//没有指定用什么模式的时候
		{
			if(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Content_Type))
			{
				G_HTTPSStatus.httpContentType = HTTP_ContentType_LVMode;
			}
			else
			{
				G_HTTPSStatus.httpContentType = HTTP_ContentType_ChunkedMode;
			}
		}
		
		if(G_HTTPSStatus.httpContentType == HTTP_ContentType_LVMode) //默认使用content-length模式发上行,不需要对下行格式进行判断
		{	
			off += HTTPPatchPostHeadData(HEADID_Content_Length, &tempBuf[off]);  // 拼装Content-Length:+空格+ xxxx+空格+CRLF
				
			SixteenToTen(dataLength, &tempBuf[off]);   //将dataLength转换成10进制对应的ascii
			
			sdId = tempBuf[off];		//sdId复用 标识ascii码的长度
			
			memmove(&tempBuf[off], &tempBuf[off+1], sdId);
			
			off += sdId;

			memcpy(&tempBuf[off], &HTTP_CRLF[0], sizeof(HTTP_CRLF));
			
			off += sizeof(HTTP_CRLF);
		}
		else 
		{		
			off += HTTPPatchPostHeadData(HEADID_Transfer_Encoding, &tempBuf[off]);			
		}
		
		if (G_HTTPSStatus.HTTPTriggerMode != HTTPTriggeredBySCP81Connection)
		{
			off += HTTPPatchPostHeadData(HEADID_XAdminScript_Status, &tempBuf[off]);   //padding scriptstate	
		}

	}
	else //没有响应数据
	{
		flag = TRUE;
		
		//下行是204结束标记，但是具有nextUri， 下行指令数据是空，此时响应数据的上行格式中不期望包含HEADID_XAdminScript_Status字段， (用例: eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/EUICC-CARD-TEST/TestSuite_BasicInitialState/TC_FPIP_1_ProfileDownloadAndEnabling_02_HTTPS)
		if ((G_HTTPSStatus.httpException != HTTP_OK) && (G_HTTPSStatus.httpException != HTTP_Continue) && (G_TLSSecurityParameters.clientSeqNum != 1))                          //解析 HTTP 协议 出错
		{
			off += HTTPPatchPostHeadData(HEADID_XAdminScript_Status, &tempBuf[off]);
		}		
	}
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY	
	if (G_HTTPSStatus.HTTPIsResume == TRUE)
	{
		off += HTTPPatchPostHeadData(HEADID_XAdmin_Resume, &tempBuf[off]);     //拼装 X- Admin- Resume:+空格+TRUE+空格+CRLF

		G_HTTPSStatus.HTTPIsResume = FALSE;
	}
#endif
	memcpy(&tempBuf[off], &HTTP_CRLF[0], sizeof(HTTP_CRLF)); //拼装HTTP头的最后的CRLF，接下来拼装HTTP体
	
	off += sizeof(HTTP_CRLF);	
	
//	WriteTFBuf(bipSendFileAddr, bipSendFileOff, &tempBuf[0], off);   //把有效数据写入
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
	ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#else
	WriteEepromWithoutProtect(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#endif
	
	bipSendFileOff += off;
	
	if(flag == TRUE) //没有数据，直接返回
	{		
		bipSendFileOff -= (2 + TLS_RecordHead_LENGTH + ivLgth);
		
		//重写长度, 这个地方长度是有效数据的长度，有效数据的起始位置是2 + TLS_RecordHead_LENGTH + G_TLSSecurityParameters.blockLength
		//WriteTFBuf(bipSendFileAddr, 0x00, (BYTE*)&bipSendFileOff, 2);
//		WriteTFFileWord(bipSendFileAddr, bipSendFileOff);
		G_BIPModule_Receive_Length = bipSendFileOff;
		
		return TRUE;
	}
	
	off = 0;
	
	if (G_HTTPSStatus.httpContentType == HTTP_ContentType_LVMode) //1默认使用content-length模式发上行,不需要对下行格式进行判断
	{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
		if ((bipSendFileOff+dataLength) > BIPModule_Receive_SIZE)
		{
			return FALSE;
		}
		
		ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &HTTP_ResponseData_ADDR[2], dataLength);
#else
		//if(FALSE == VMCopyDataToAddr(bipSendFileAddr + bipSendFileOff, fileSize - bipSendFileOff, fileAddr + sizeof(TranFileHead) + 2 ,dataLength, MODE_PROTECT_WRITE))
		if(FALSE == VMCopyDataToAddr(bipSendFileAddr , bipSendFileOff, fileSize - bipSendFileOff, fileAddr , 2 ,dataLength, MODE_PROTECT_WRITE,0,1))
		{
			return FALSE;
		}
#endif		
		bipSendFileOff += dataLength;	
	}
	else 
	{			
		index = 0;		//EF_HTTPSResponseData中原数据的偏移

		//去掉原来所有 body 数据放在一个chunk块的情况， 因为 CHUNKED_SIZE 的大小可以调整足够大
		
		off = 0;			//tempBuf的清零提到循环前，因为循环内部每一chunk块的最后一个HTTP_CRLF和下一chunk块的 L+HTTP_CRLF 连在一起一次写入bipSendFileAddr

		while(index < dataLength)
		{				
			if((dataLength - index) > CHUNKED_SIZE)		//自定义chunk的长度0xF0
			{
				tempLen = CHUNKED_SIZE;
			}
			else
			{
				tempLen = dataLength - index;
			}

			//根据电信测试需求：中国电信用户卡需求白皮书-OTA下载功能分册(2021.V1).pdf 的 5.8.9章节,  因为原来代码是CHUNKED_SIZE大小是小于0xF0即1字节，现在CHUNKED最大是0x400, 转换为ascii码的需要改变
			SetWord(tmp, tempLen);
			
			off += HexStrToAscStr(&tmp[0], &tempBuf[off], 2);	//原来的代码有bug，只能转换tempLen是1字节的情况，使用HexStrToAscStr可以满足1和2字节转换为ASCII的情况
			
			memcpy(&tempBuf[off], &HTTP_CRLF[0], sizeof(HTTP_CRLF));
			
			off += sizeof(HTTP_CRLF);
			
			//原来的代码有bug， 当tempLen大于tempBuf大小时，读取数据越界，修改为直接调用epp移动数据到eep的函数
			if((bipSendFileOff + off) <= BIPModule_Receive_SIZE)
			{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
				ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#else
				WriteEepromWithoutProtect(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#endif
			}
			else
			{
				return FALSE;
			}

			bipSendFileOff += off;
			
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			if ((bipSendFileOff+tempLen) > BIPModule_Receive_SIZE)
			{
				return FALSE;
			}
			
			ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &HTTP_ResponseData_ADDR[2+index], tempLen);		
#else		
			if(FALSE == VMCopyDataToAddr(bipSendFileAddr , bipSendFileOff, fileSize - bipSendFileOff, fileAddr , 2 + index, tempLen, MODE_NO_PROTECT_WRITE,0,1))
			{
				return FALSE;
			}
#endif		
			bipSendFileOff += tempLen;
			
			index += tempLen;

			off = 0;	//这里清零是因为一次循环里，把前一块数据的最后的 HTTP_CRLF 和后一块数据的 L + HTTP_CRLF 放在tempBuf连着一次写入 bipSendFileAddr, 做到一个循环里调用一次 WriteEepromWithoutProtect

			memcpy(&tempBuf[off], &HTTP_CRLF[0], sizeof(HTTP_CRLF));

			off += sizeof(HTTP_CRLF);
		}
		
		//off = 0;	//这里注释是因为前面循环最后一个chunk块的最后的 CRLF 还在 tempBuf 中没有写入 bipSendFileAddr， 所以不能清零该值
		
		memcpy(&tempBuf[off], &HTTP_CHUNK_0CRLF[0], sizeof(HTTP_CHUNK_0CRLF));
		
		off += sizeof(HTTP_CHUNK_0CRLF);
							
//		if (!WriteTFBuf(bipSendFileAddr, bipSendFileOff, &tempBuf[0], off))
//		{
//			return FALSE;
//		}
		
		if((bipSendFileOff + off) <= BIPModule_Receive_SIZE)
		{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#else
			WriteEepromWithoutProtect(bipSendFileAddr + bipSendFileOff, &tempBuf[0], off);
#endif
		}
		else
		{
			return FALSE;
		}
		
		bipSendFileOff += off;			
		
	}
		
	bipSendFileOff -= (2 + TLS_RecordHead_LENGTH + ivLgth);
	
//	WriteTFFileWord(bipSendFileAddr,bipSendFileOff);
	G_BIPModule_Receive_Length = bipSendFileOff;

	return TRUE;
}

/*
*@description:比较两个字符串的大小，不区分大小写 a...z 0x61...0x7A
*
*@function   :MemcmpWithoutUpperOrLower
*
*@param      :BYTE *buffer1  源数据1，ascii码数据
*            :BYTE *buffer2  源数据2，ascii码数据
*
@return      :BYTE EQUAL：不区分大小写的结果是两个字节串相同/ TRUE: 两个字符串不同
*
*@history    :xuxiaojun@20180521
*/
BYTE MemcmpWithoutUpperOrLower(BYTE *buffer1, BYTE *buffer2, BYTE bufLen)
{
	BYTE idata i=0;
	
	for(i=0;i<bufLen;i++)
	{
		if((buffer1[i] >= 0x61) && (buffer1[i] <= 0x7A))        //buffer1[i]是小写
		{
			if((buffer2[i] != buffer1[i]) && (buffer2[i] != (buffer1[i]-0x20)))
			{
				return TRUE;
			}
		}
		else if((buffer1[i] >= 0x41) && (buffer1[i] <= 0x5A))  //buffer1[i]是大写
		{
			if((buffer2[i] != buffer1[i]) && (buffer2[i] != (buffer1[i]+0x20)))
			{
				return TRUE;
			}
		}
		else
		{
			if(buffer2[i] != buffer1[i])
			{
				return TRUE;
			}
		}
	}
	return EQUAL;
}

/*
*@description:删除一串ASCII字符串的前后空格
*
*@function   :TrimSpaceAtHeadOrEnd
*
*@param      :BYTE * asciiBuffer  ascii编码的字符串  
*			 :WORD length		  传入的数组长度
*            :BYTE *startAddr     第一个非空格在asciiBuffer中的位置，属于输出变量
*            :BYTE *endAddr       最后一个非空格在asciiBuffer中的位置，属于输出变量
*
*@return     :BYTE                返回前后连续空格的字节数/0表示没有空格
*
*@history    :xuxiaojun@20180517
*NOTE        :该函数的调用者在调用时传入startAddr， endAddr时，要注意大小端问题。
*            :modify by jiangqingbin@20180829 length为2，且第一个字节为空格时，*endAddr没有赋值
*/
BYTE TrimSpaceAtHeadOrEnd(BYTE * asciiBuffer, WORD *startAddr, WORD *endAddr, WORD length)
{
	BYTE idata s = 0;  //第一个不是空格的起始数据在asciiBuffer中的偏移
	WORD idata index;

	for (index = 0; index < length; index++)
	{
		if(asciiBuffer[index] != 0x20)
		{
			if (s == 0)
			{
				*startAddr = index;
				*endAddr = index;
				s = 1;
			}
			else
			{
				*endAddr = index;
			}
		}
	}
	return (*startAddr+(length-1)-*endAddr);
}
/*
*@description:该函数实现的功能是：对下行数据中的http头进行解析，获取当前需要校验的http头类型。
*             原则是，假设http头中的所有的协议类型最多只会出现一遍，如果出现多遍，以第一遍的数据为准
*
*@See        :<rfc2616(HTTP1.1).pdf> If a message is received with both a  Transfer-Encoding header field and a  Content-Length header field, the latter MUST be ignored.
*
*@function   :GetHttpHeadID
*
*@param      :WORD offsetRecData  当前起始数据在RecData中的偏移        
*             WORD dataLen        当前RecData中有效数据的长度，从RecData[0]开始
*
*@return     :BYTE HeadID         返回当前待处理的http头协议类型，如果不是《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》中定义的标准头，或者头被截断，则返回HEADID_OTHERS
*
*@history    :xuxiaojun@20180517
*/
BYTE GetHttpHeadID(BYTE * offsetRecData, BYTE dataLen)
{
	BYTE idata offSet, offLen;				//*lenByte += off;
	
	offSet = *offsetRecData;
	
	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_VERSION_11[0], sizeof(HTTP_VERSION_11)))		//第一个头特殊的没有冒号，第一个头的头不会被断开
	{
		*offsetRecData += sizeof(HTTP_VERSION_11);
		
		return HEADID_HTTP_VERSION;
	}
	
	for (offLen = offSet; offLen < dataLen; offLen++)
	{
		if (RecData[offLen] == 0x3A) 
		{
			*offsetRecData += (offLen-offSet+1);			//使用offLen在循环中累加， 如果使用*offsetRecData会导致指针跑飞
			break;
		}
		
		if  ((RecData[offLen] == 0x0D) && (RecData[offLen+1] == 0x0A))
		{
			*offsetRecData += (offLen-offSet);			//使用offLen在循环中累加， 如果使用*offsetRecData会导致指针跑飞
			break;
		}
	}
	
	if (offLen == dataLen)//数据中没有发现0D0A
	{
		*offsetRecData += (offLen-offSet);
		return HEADID_NOT_FULL;		//数据头被阶段，没有找到完整的head项
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_PROTOCOL_HEAD[0], sizeof(HTTP_PROTOCOL_HEAD)))
	{
		return HEADID_XAdmin_Protocol;
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_NEXT_URI[0], sizeof(HTTP_NEXT_URI)))
	{
		return HEADID_XAdmin_Next_URI;
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_CONTENT_TYPE[0], sizeof(HTTP_CONTENT_TYPE)))
	{
		return HEADID_Content_Type;
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_TARGETED_AID[0], sizeof(HTTP_TARGETED_AID)))
	{
		return HEADID_XAdmin_TarApp;
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_CONTENT_LENGTH[0], sizeof(HTTP_CONTENT_LENGTH)))
	{
		if((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_CONTENT_LENGTH_EXIST) == HTTP_HEAD_CONTENT_LENGTH_EXIST) //当前已经收到了Content-Length或都Transfer-Encoding中的一个，则后收到的被忽略. If a message is received with both a  Transfer-Encoding header field and a  Content-Length header field, the latter MUST be ignored.
		{
			return HEADID_OTHERS;
		}
		else
		{
			return HEADID_Content_Length;
		}
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_TRANSFER_ENCODING[0], sizeof(HTTP_TRANSFER_ENCODING)))
	{
		if((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_CONTENT_LENGTH_EXIST) == HTTP_HEAD_CONTENT_LENGTH_EXIST) //当前已经收到了Content-Length或都Transfer-Encoding中的一个，则后收到的被忽略. If a message is received with both a  Transfer-Encoding header field and a  Content-Length header field, the latter MUST be ignored.
		{
			return HEADID_OTHERS;
		}
		else
		{
			return HEADID_Transfer_Encoding;
		}
	}

	if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offSet], (BYTE*)&HTTP_CRLF[0], sizeof(HTTP_CRLF)))
	{
		return HEADID_CRLF;
	}

	return HEADID_OTHERS;
}

/*
*@description:从X-Admin-Targeted-Application:的值域中提取出aid及aid长度
*
*@function   :getAidFromXAdminTarApp
*
*@param      :BYTE *aidContent       X-Admin-Targeted-Application:的值域，其中的数字是ascii编码，可能的内容是//aid/A000000559/1010FFFFFFFF8900001000(对应的ascii为：2F2F6169642F413030303030303535392F31303130464646464646464638393030303031303030)
*            :BYTE aidContentLen     X-Admin-Targeted-Application:的值域的长度
*
*@return     :BYTE                   返回AID的长度
*
*@history    :xuxiaojun@20180522
*/
BYTE getAidFromXAdminTarApp(BYTE *aidContent, BYTE aidContentLen)
{
	BYTE idata i=6; //偏移掉前面的//aid/，对应的ascii为2F2F6169642F    
	BYTE idata j=0;
	
	for(;i<aidContentLen;i++)
	{
		if(aidContent[i] == SLASH)		//右斜线
		{
			continue;
		}
		
		if(aidContent[i] == CR)			//结束符
		{
			break;
		}
		
		aidContent[j++] = aidContent[i];
	}

	return AsciiToHexInt(aidContent, j, aidContent);			
}





/*
*@description: 当下行数据没有 X-Admin-Targeted-Application 字段时对G_TARRecIndex的赋值
*
*@function   :HttpRAMTargetAppiddefaultProcess
*
*@param      :void
*
*@return     :BYTE                   返回AID的长度
*
*@history    :xuxiaojun@20180522
*		     :jiangqb@20190621		针对tpx3.0-euicc对G_TARRecIndex[]数据进行赋值，用于后续模式判断和指令派发
*/
#if UICC__FUNCTION__RAM
void HttpRAMTargetAppiddefaultProcess()
{
//	BYTE idata num;
//	UWORD xdata fileAddr;

	G_HTTPSStatus.TargetedSD = G_HTTPSStatus.TriggeredSD;			//默认目标sd是触发sd

#if M2M_EUICC
	if (G_HTTPSStatus.TriggeredSD == ISDR_APPID)
	{
		G_TARRecIndex[0] = TarType_ISDR;
	}
	else if ((G_HTTPSStatus.TriggeredSD == ISDP_APPID)&&(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX))
	{
		G_TARRecIndex[0] = TarType_ISDP;
	}
	else
#endif
	{
		G_TARRecIndex[0] = TarType_RAM;
		G_TARRecIndex[1] = G_HTTPSStatus.TargetedSD;

//		fileAddr = SelectLFFile(DF_Toolkit_P, EF_RAMAPDUTars);

//		if(fileAddr != FALSE)
//		{
//			num = GetFileHeadByte(fileAddr, F_RecNum_Offset);
//
//			while(num--)
//			{
//				ReadLFBuf(fileAddr, num, sdAppId_offset, &G_TARRecIndex[2], sizeof(BYTE));
//
//				if (G_HTTPSStatus.TargetedSD == G_TARRecIndex[2])
//				{
//					G_TARRecIndex[1] = num;
//
//					break;
//				}
//			}
//		}
	}
}
#endif


/*
*@description: 判断X-Admin-Targeted-Application的值的有效性及对对应全局变量赋值
*
*@function     : checkHttpTargetAppid
*
*@param       : BYTE commandType
*@param       : BYTE * tarAppAID
*@param       : BYTE appIdLen     X-Admin-Targeted-Application:的值域，其中的数字是ascii编码，可能的内容是//aid/A000000559/1010FFFFFFFF8900001000(对应的ascii为：2F2F6169642F413030303030303535392F31303130464646464646464638393030303031303030)
*            :BYTE aidContentLen     X-Admin-Targeted-Application:的值域的长度
*
*@return     :BYTE                   返回AID的长度
*
*@history    :xuxiaojun@20180522
*			 :modify by jiangqb@20191122 -调整https头的子项中，先app-aid后contentType的情况下，对app-aid临时存放在文件中
*		     :modify by jiangqb@20191122 -修改电信平台适用的获取当前ADF的地址的函数，以及增加匹配电信aid的配置判断
*/
BYTE checkHttpTargetAppid(BYTE commandType, BYTE * tarAppAID, BYTE appIdLen)
{
	BYTE idata num, offSet;
	BYTE xdata aidCmpBuffer[17];
	UWORD xdata fileAddr;
	
#if JAVA_CARD_UICC	
	BYTE idata privi;
	
	privi = 0;
#endif	
	
#if M2M_EUICC	
	BYTE idata  profileIndex;
#endif	

#if UICC__FUNCTION__RAM	
	if (commandType == HTTP_Command_Type_RAM)
	{
#if COS_UPDATE_ONLINE_BY_NativeOS
		if(IF_UPCOS_SSDAppId(tarAppAID, appIdLen) == TRUE)
		{
			G_HTTPSStatus.TargetedSD = UPCOSConfig_GetUPCOSUMSSDAPPID();
			G_TARRecIndex[0] = TarType_RAM;
			G_TARRecIndex[1] = G_HTTPSStatus.TargetedSD;
			return TRUE;
			
		}
#endif		
#if JAVA_CARD_UICC
		
		G_HTTPSStatus.TargetedSD = GPGetAppletIDByAppletAID(tarAppAID, appIdLen);	   //Security Domain AID		
		
		if(G_HTTPSStatus.TargetedSD == ILLEGAL_APPID)				//SD找不到
		{
#if M2M_EUICC
			if (G_HTTPSStatus.TriggeredSD == ISDR_APPID)	//设备版并且是isdr的https流程可以查找正在下载isdp 或disable的isdp
			{
				profileIndex = GetProfileIndexByAIDAndState(tarAppAID, appIdLen, PROFILE_LOADINGFLAG);		//G_TargetedSD是当前正在下载的profile

				if (profileIndex == ILLEGAL_PROFILEINDEX)					//
				{
					profileIndex = GetProfileIndexByAIDAndState(tarAppAID, appIdLen, PROFILE_VALIDFLAG);	//比如给一个disable的isdp进行config配置

					if (profileIndex == ILLEGAL_PROFILEINDEX)
					{
						G_HTTPSStatus.httpException = HTTP_UNKNOWN_APP_ERROR;

						return FALSE;
					}
				}

				G_TARRecIndex[0] = TarType_ISDP;

				G_TARRecIndex[1] = profileIndex;			//profile index

				G_TARRecIndex[2] = G_HTTPSStatus.TriggeredSD;

				G_HTTPSStatus.TargetedSD = ISDP_APPID;
			}
			else
#endif
			{
				G_HTTPSStatus.httpException = HTTP_UNKNOWN_APP_ERROR;

				return FALSE;
			}

		}
#if M2M_EUICC
		else if (G_HTTPSStatus.TargetedSD == ISDP_APPID)	//isdp
		{
			if (G_HTTPSStatus.TriggeredSD == ISDR_APPID)//设备版并且是isdr的https流程可以查找激活的isdp
			{
				G_TARRecIndex[0] = TarType_ISDP;

				G_TARRecIndex[1] = GetProfileIndexByAIDAndState(tarAppAID, appIdLen, PROFILE_VALIDFLAG);		//		//profile index

				G_TARRecIndex[2] = G_HTTPSStatus.TriggeredSD;
			}
			else
			{
				G_HTTPSStatus.httpException = HTTP_UNKNOWN_APP_ERROR;

				return FALSE;
			}
		}
#endif
		else
		{
			G_TARRecIndex[0] = TarType_RAM;

#if M2M_EUICC
			if (G_HTTPSStatus.TriggeredSD == ISDR_APPID)			//ISDR 只能访问isdp 和 ecasd
			{
				if ((G_HTTPSStatus.TargetedSD != ISDP_APPID)&&(G_HTTPSStatus.TargetedSD != ECASD_APPID))
				{
					G_HTTPSStatus.httpException = HTTP_NOT_SD_ERROR;

					return FALSE;
				}
			}
#endif
			GetAppPrivileges(G_HTTPSStatus.TargetedSD,&privi,1);

			if((privi&PRIVILEGE_FIRSTBYTE_SD) == 0x00) //不是SD
			{
				G_HTTPSStatus.httpException = HTTP_NOT_SD_ERROR;

				return FALSE;
			}

			G_TARRecIndex[1] = G_HTTPSStatus.TargetedSD;
		}
#endif
	}
#endif	
	if(commandType == HTTP_Command_Type_RFM)
	{	
		G_SpecialRFM = IF_AnnexB_ChinaMobileNotSuppotRFMmode(tarAppAID, appIdLen);

		fileAddr = SelectLFFile(DF_Toolkit_P,EF_RFM_TAR);

		num = GetLFRecNumLgth(fileAddr,FileNum);

		do
		{
			offSet = PEAID_Offset;
			while (1)
			{
				ReadLFBuf(fileAddr, num, offSet, aidCmpBuffer,0x1);

				if (aidCmpBuffer[0] == appIdLen)
				{
					ReadLFBuf(fileAddr, num, offSet+1, aidCmpBuffer+1,aidCmpBuffer[0]);

					if(memcmp(aidCmpBuffer+1, tarAppAID, appIdLen) == EQUAL)
					{
						G_TARRecIndex[0] = TarType_RFM;
						G_TARRecIndex[1] = num;
						G_TARRecIndex[2] = G_HTTPSStatus.TriggeredSD;
						return TRUE;
					}
				}

				if (offSet == PEADFAid_Offset)
				{
					break;
				}

				offSet = PEADFAid_Offset;
			}
		}while(--num);

		G_HTTPSStatus.httpException = HTTP_UNKNOWN_APP_ERROR;

		G_HTTPSStatus.TargetedSD = ILLEGAL_APPID;

		return FALSE;
	}
	else  //服务器第一次下发HTTP POST  Response时，Content - Type和X- Admin- Targeted - Application可能是颠倒的，以下是处理先收到X- Admin- Targeted - Application，再收到Content - Type
	{
		fileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData);
		
		aidCmpBuffer[0] = appIdLen;

		memcpy(aidCmpBuffer+1, tarAppAID, appIdLen);

		WriteTFBuf(fileAddr, Http_TargetAppId_LvOff, aidCmpBuffer, appIdLen + 1);
		
		G_HTTPSStatus.TargetedSD = ILLEGAL_APPID;
	}

	return TRUE;
}
/**
 *@function      : getTargetAppAID
 *
 *@description : get target application's AID value
 *
 *@param        : BYTE* aidCmpBuffer --- output, LV
 */
void getTargetAppAID(BYTE* aidCmpBuffer)
{
	ReadTFBuf(SelectPrivateTFFile(DF_TLS, EF_TLSsessionData), Http_TargetAppId_LvOff, aidCmpBuffer, 0x11);
}


/*
*@description:对HTTP协议头进行检测的子函数，进入该函数时，每次都是从RecData[0]开始检测。该函数可能会被多次调用
*            :该函数实现的原则是，当前正在check的头必须是完整的head+body的格式。检测到某个协议头之后，紧接着查找该头对应的结束标志符，如果找不到，则返回继续从文件读取数据。
*
*@See        : <rfc2616(HTTP1.1).pdf> All 1xx(informational), 204 (no content), and 304 (not modified) responses MUST NOT include a message-body. All other responses do include a message-body, altho ugh it MAY be of zero length
*              目前只处理了204
*
*@function   :HTTPAnalyseHeadSub
*
*@param      :BYTE dataLen       RecData中从RecData[0]开始的有效数据长度, 由传入决定最大为0xFA
*
*@return     :WORD               返回当前不完整tag的起始位置在RecData中的偏移，可能是标准TAG被截断，也可能是非标准TAG被截断. 只有当G_HTTPSStatus.httpException正常时，返回值才有效
*
*@history    :xuxiaojun@20180517
*            :modify by xuxiaojun@20180628 增加对印度rjio测试卡实网测试时SIM的需求
*            :modify by jiangqb@20191122 对下行数据的nextUri写入文件越界写入失败时进行报错，关闭通道
*			 :modify by jiangqb@20191122 -https头的子项中，先app-aid后contentType的情况下，对app-aid临时存放在文件中，保存和读取位置调整到 checkHttpTargetAppid函数中
*
*NOTE        :由于该函数的实现借用RecData[0, CT_APDU_BUFFER_SIZE-5]
*/
WORD HTTPAnalyseHeadSub(BYTE dataLen)
{
	WORD idata offsetBodyStart, offsetBodyEnd;// spaceCounter;
	BYTE idata headId, aidLen, offsetRecData, offsetStart, offsetHead, spaceCounter;
	UWORD xdata triggerParaFileAddr,contentLen;
	BYTE xdata aidCmpBuffer[0x11];

#if UICC__FUNCTION__RAM	
	BYTE idata privi;
#endif
	
	offsetHead = 0;
	
	offsetRecData = 0;
	
	offsetBodyStart = 0;    //每一类协议的内容起始偏移
	
	offsetBodyEnd = 0;
		
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/9106存储触发参数
		
	while(offsetRecData < dataLen)  //一次处理一个http头类型
	{
		offsetHead = offsetRecData;  
		
		headId = GetHttpHeadID(&offsetRecData, dataLen);  //	
		
		offsetStart = offsetRecData; 	//每个子项 的引号 : 的位置
		
		while (offsetRecData < dataLen)
		{
			if(EQUAL == memcmp(&RecData[offsetRecData], &HTTP_CRLF[0], sizeof(HTTP_CRLF))) 
			{			
				break;
			}
			
			offsetRecData++;
		}
		
		if (offsetRecData >= dataLen)	////头的头断开或者头的体断开
		{
			return offsetHead;
		}
											
		spaceCounter = TrimSpaceAtHeadOrEnd(&RecData[offsetStart], &offsetBodyStart, &offsetBodyEnd, offsetRecData-offsetStart);
			
		switch(headId)
		{
			case HEADID_HTTP_VERSION:
						
				if(EQUAL == memcmp(&RecData[offsetStart + offsetBodyStart], &HTTP_RESPONSE_STATUS_204[0], sizeof(HTTP_RESPONSE_STATUS_204))) //如果响应状态字是204，则终止会话  
				{
					G_HTTPSStatus.httpException = HTTP_SESSION_FINISH;  //<rfc2616(HTTP1.1).pdf> Any response message which “MUST NOT” include a message-body (such as the 1xx, 204, and 304 responses and any response to a HEAD request) is always terminated by the first empty line after the header fields, regardless of the entity-header fields present in the message.
				}
				
				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_VERSION_EXIST;
			
				break;  //解析下一个	
						
			case HEADID_XAdmin_Protocol:
						
				if(EQUAL != MemcmpWithoutUpperOrLower((BYTE*)&RecData[offsetStart + offsetBodyStart], (BYTE*)&HTTP_PROTOCOL_VALUE[0], sizeof(HTTP_PROTOCOL_VALUE)))  //再对HTTP_PROTOCOL_VALUE的值进行判断
				{
					G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;

					return offsetHead;
				}
				
				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_PROTOCOL_EXIST;

				break;  //解析下一个
				
			case HEADID_XAdmin_Next_URI:
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
				SetWord(&HTTP_NextURIData_ADDR[0], (offsetBodyEnd-offsetBodyStart+1));
				ecpMem_Copy(&HTTP_NextURIData_ADDR[2], &RecData[offsetStart+offsetBodyStart], (offsetBodyEnd-offsetBodyStart+1));
#else			
				if (!WriteLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_NextURIPara_Index, &RecData[offsetStart+offsetBodyStart], offsetBodyEnd-offsetBodyStart+1))
				{
					G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;
					return offsetHead;
				}				
#endif

				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_URI_EXIST;
				
				if (G_HTTPSStatus.httpException == HTTP_SESSION_FINISH)			//G_HTTPSStatus.httpException = HTTP_CLOSE_NOTIFY;
				{
					return offsetHead;
				}
						
				break;  //解析下一个
			
			case HEADID_Content_Type:
			{
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
				if (G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection)		//scp81-api触发模式的头检查
				{
					if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offsetStart+offsetBodyStart], (BYTE*)&SCP81_CONTENTTYPE_RESPONSE_VALUE[0], sizeof(SCP81_CONTENTTYPE_RESPONSE_VALUE)))  //再对HTTP_CONTENTTYPE_RES_RAM_VALUE的值进行判断
					{
						G_HTTPSStatus.httpCommandType = HTTP_Command_Type_SCP81;
						
						break;
					}
					else
					{
						G_HTTPSStatus.httpCommandType = HTTP_Command_Type_NULL;
						
						G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;
						
						return offsetHead;
					}
				}
#endif				
#if UICC__FUNCTION__RAM					//RAM
				if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offsetStart+offsetBodyStart], (BYTE*)&HTTP_CONTENTTYPE_RES_RAM_VALUE[0], sizeof(HTTP_CONTENTTYPE_RES_RAM_VALUE)))  //再对HTTP_CONTENTTYPE_RES_RAM_VALUE的值进行判断
				{
					G_HTTPSStatus.httpCommandType = HTTP_Command_Type_RAM;

					G_TARRecIndex[0] = TarType_RAM; //为什么需要重新赋值？
							
					if((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_TARGETED_APP_EXIST) == HTTP_HEAD_TARGETED_APP_EXIST)
					{								
						getTargetAppAID(aidCmpBuffer);

						if (!checkHttpTargetAppid(HTTP_Command_Type_RAM, &aidCmpBuffer[1], aidCmpBuffer[0]))
						{						
							return offsetHead;
						}
						
					}
					else
					{
					#if  JAVA_CARD_UICC   
						GetAppPrivileges(G_HTTPSStatus.TriggeredSD,&privi,1);		//当api触发http会话时，TriggeredSD不是一个sd而是触发http就会话的appid
						
						if((privi&PRIVILEGE_FIRSTBYTE_SD) == 0x00) //不是SD
						{
							G_HTTPSStatus.TargetedSD = GetAssoiatedSDByAppId(G_HTTPSStatus.TriggeredSD);
						}
						else
					#endif
						{
							G_HTTPSStatus.TargetedSD = G_HTTPSStatus.TriggeredSD;	
						}
					}
				}
				else 
#endif
#if   UICC__FUNCTION__RFM				//RFM
				if(EQUAL == MemcmpWithoutUpperOrLower((BYTE*)&RecData[offsetStart+offsetBodyStart], (BYTE*)&HTTP_CONTENTTYPE_RES_RFM_VALUE[0], sizeof(HTTP_CONTENTTYPE_RES_RFM_VALUE)))  
				{
					G_HTTPSStatus.httpCommandType = HTTP_Command_Type_RFM;
					
					if((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_TARGETED_APP_EXIST) == HTTP_HEAD_TARGETED_APP_EXIST)
					{
						getTargetAppAID(aidCmpBuffer);

						if (!checkHttpTargetAppid(HTTP_Command_Type_RFM, &aidCmpBuffer[1], aidCmpBuffer[0]))
						{						
							return offsetHead;
						}
					}
					else
					{
						G_HTTPSStatus.TargetedSD = G_HTTPSStatus.TriggeredSD;
					}
	
				}
				else
#endif
				{		
					G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;

					return offsetHead;  //头是错误的	
				}
				
				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_CONTENT_TYPE_EXIST;
				
				break;
			}
			
			// a new command session shall be started if the Security Domain targeted by the current HTTP POST response is not the same as the one targeted by the previous HTTP POST response. That means: ?  the value of the header  “ X - Admin- Targeted - Application ”  has changed;
			//The “ X - Admin- Script - Status ”  header val ue is used to return the delivery status of the previous remote APDU format string.
			case HEADID_XAdmin_TarApp: 
							
				aidLen = getAidFromXAdminTarApp(&RecData[offsetStart+offsetBodyStart], offsetRecData-offsetStart-spaceCounter);  //从aidcontent中提取出aid，及aid的长度
						
				if(AIDLenInvalid(aidLen)) //aidLen非法
				{
					G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;

					return offsetHead;
				}
				
				if (!checkHttpTargetAppid(G_HTTPSStatus.httpCommandType, &RecData[offsetStart+offsetBodyStart], aidLen))	
				{
					return offsetHead;
				}
				
				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_TARGETED_APP_EXIST;
																		
				break;
			
			case HEADID_Content_Length:
							
				aidLen = offsetBodyEnd - offsetBodyStart + 1; //aidLen复用, 目前假设content length最大只支持4字节，此时还是ascii码
						
				contentLen = AsciiToDec(&RecData[offsetStart+offsetBodyStart], aidLen);
			
				if ((contentLen > 0xFFFF) || (aidLen > 5))		//数据越界
				{
					G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;
					
					return offsetHead;
				}
				
				G_HTTPSStatus.httpContentType = HTTP_ContentType_LVMode;
				
				G_HTTPSStatus.contentLen.contentLength = contentLen;
				
				if (G_HTTPSStatus.contentLen.contentLength == 0)
				{
					G_HTTPSStatus.httpException = HTTP_SESSION_FINISH;
					
					return offsetHead;
				}		

				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_CONTENT_LENGTH_EXIST;
				
				break;  
			
			case HEADID_Transfer_Encoding:
			
				if(EQUAL != MemcmpWithoutUpperOrLower((BYTE*)&RecData[offsetStart+offsetBodyStart], (BYTE*)&HTTP_CHUNKED[0], sizeof(HTTP_CHUNKED)))  
				{
					G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;

					return offsetHead;
				}
				
				G_HTTPSStatus.httpContentType = HTTP_ContentType_ChunkedMode;
				
				G_HTTPSStatus.httpHeadExit |= HTTP_HEAD_CONTENT_LENGTH_EXIST;
				
				break;  //解析下一个
			
			case HEADID_CRLF:
			
				G_HTTPSStatus.httpDataStatus  = HTTP_DataType_BODY;
				
				G_HTTPSStatus.httpChunkedContentStatus = HTTP_ChunkedContent_Beginning;
				
				offsetRecData += sizeof(HTTP_CRLF); 

				return offsetRecData;			//头结束返回
			
			default:  // HEADID_OTHERS，包含了某个标准头被截断的情况
				break;
		}
		
		offsetRecData += sizeof(HTTP_CRLF); //正常结束，偏移到下一个协议类型			
	}
	
	if(offsetRecData == dataLen)  //本次的RecData全部处理完，结尾刚好是某个完整的头
	{
		return dataLen;
	}
	else
	{
		return offsetHead;		
	}
}


/*
*@description:对HTTP协议体进行检测的子函数，进入该函数时，每次都是从RecData[0]开始检测。该函数可能会被多次调用
*
*@See        : <rfc2616(HTTP1.1).pdf>中的http协议body部分
*
*@function   :HTTPAnalyseBodySub
*
*@param      :BYTE dataLen       RecData中从RecData[0]开始的有效数据长度
*
*@return     :返回本次处理的数据的长度
*
*@history    :jiangqb@20180815   
*            
*NOTE        :
*/
BYTE HTTPAnalyseBodySub(BYTE dataLen)			//
{
	BYTE idata OffsetRecdata;
	
	WORD idata ExitCmdLen, mask;
	
	UWORD xdata httpCommandFileAddr;
		
	mask = 0;
	OffsetRecdata = 0;
	
//	httpCommandFileAddr = SelectPrivateTFFile(DF_TLS, EF_HTTPSCommandData);
	httpCommandFileAddr = (UWORD)HTTP_CommandData_ADDR_START;
	
//	ExitCmdLen = ReadTFFileWord(httpCommandFileAddr);
	ExitCmdLen = G_HTTP_CommandData_Length;
	
	if (G_HTTPSStatus.ChunkedSizeBuf[0] != 0)
	{
		memmove(RecData+G_HTTPSStatus.ChunkedSizeBuf[0], RecData, dataLen);
		
		memcpy(RecData, G_HTTPSStatus.ChunkedSizeBuf+1, G_HTTPSStatus.ChunkedSizeBuf[0]);
		
		dataLen += G_HTTPSStatus.ChunkedSizeBuf[0];
		
		G_HTTPSStatus.ChunkedSizeBuf[0] = 0;
	}
		
	if(G_HTTPSStatus.httpContentType == HTTP_ContentType_LVMode)			//content-length模式
	{
		if ( (RecData[dataLen-1] == Lf) && (RecData[dataLen-2] == CR) && (RecData[dataLen-3] == Lf) && (RecData[dataLen-4] == CR) )	//ul_scp测试时发现的问题，此处不把特殊字符字段去掉的话，下一个if判断就不成立，httsData状态就设置不对
		{
			dataLen -= 4;
		}
			
//		WriteTFFileValue(httpCommandFileAddr, 0x02+ExitCmdLen, &RecData[0], dataLen, ExitCmdLen + dataLen);
		if((0x02+ExitCmdLen+dataLen) < HTTP_CommandData_SIZE)
		{
//			SetWord(tmp, ExitCmdLen + dataLen);
//			WriteEeprom(httpCommandFileAddr, tmp,0x02);
			G_HTTP_CommandData_Length = ExitCmdLen + dataLen;
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(httpCommandFileAddr+0x02+ExitCmdLen,&RecData[0],dataLen);
#else
			WriteEepromWithoutProtect(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[0],dataLen);
#endif
		}
		
		G_HTTPSStatus.HttpChunkYetReadLen += dataLen;
		
		if (G_HTTPSStatus.HttpChunkYetReadLen == G_HTTPSStatus.contentLen.contentLength)
		{
			G_HTTPSStatus.httpDataStatus = HTTP_DataType_END;
		}
		
		return dataLen;
	}
	else		// 当http头中没有标识什么模式时默认是chunk模式
	{
		while ((OffsetRecdata+1) < dataLen)
		{
			if (RecData[OffsetRecdata] == CR)
			{			
				if (RecData[OffsetRecdata+1] != Lf)		//不是特殊字符
				{
					OffsetRecdata++;
					continue;
				}
				
				switch(G_HTTPSStatus.httpChunkedContentStatus)
				{
					case HTTP_ChunkedContent_Beginning:
												
						AsciiToHexInt(&RecData[mask], OffsetRecdata-mask, &RecData[mask]);
												
						G_HTTPSStatus.contentLen.chunkedSize = RecData[mask];
					
						G_HTTPSStatus.httpChunkedContentStatus = HTTP_ChunkedContent_Processing;
					
						if ((OffsetRecdata-mask) > 2)
						{
							G_HTTPSStatus.contentLen.chunkedSize = BuildWord(&RecData[mask]);
						}			
						
						mask = OffsetRecdata+2;
						
						if ((OffsetRecdata+G_HTTPSStatus.contentLen.chunkedSize) > dataLen)
						{
							OffsetRecdata = dataLen-4;
						}
						else
						{
							OffsetRecdata += G_HTTPSStatus.contentLen.chunkedSize-2;
						}
	
						break;
					
					case HTTP_ChunkedContent_Processing:
								
						if ((OffsetRecdata == mask) && (G_HTTPSStatus.contentLen.chunkedSize == 0))		//最后一块0长度的chunk, 300D0A0D0A
						{
							G_HTTPSStatus.httpDataStatus = HTTP_DataType_END;
							
							mask = OffsetRecdata+2;
							
							return mask;
						}
						
						G_HTTPSStatus.HttpChunkYetReadLen += OffsetRecdata - mask;
						
						if (G_HTTPSStatus.HttpChunkYetReadLen != G_HTTPSStatus.contentLen.chunkedSize)		//并非真实的有效 0D0A 特殊换行字符
						{
							G_HTTPSStatus.HttpChunkYetReadLen -= OffsetRecdata - mask;
							
							break;
						}
						
//						ExitCmdLen = ReadTFFileWord(httpCommandFileAddr);	//ReadTFBuf(httpCommandFileAddr, 0x00, (BYTE*)&ExitCmdLen, sizeof(WORD));       //获取当前commandFile中有效数据的长度, 借用chunkBytes
						ExitCmdLen = G_HTTP_CommandData_Length;
					
//						WriteTFFileValue(httpCommandFileAddr, 0x02+ExitCmdLen, &RecData[mask], OffsetRecdata-mask, ExitCmdLen+OffsetRecdata-mask);
						if((0x02+ExitCmdLen+OffsetRecdata-mask) < HTTP_CommandData_SIZE)
						{
//							SetWord(tmp, ExitCmdLen+OffsetRecdata-mask);
//							WriteEeprom(httpCommandFileAddr, tmp,0x02);
							G_HTTP_CommandData_Length = ExitCmdLen+OffsetRecdata-mask;
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
							ecpMem_Copy(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[mask],OffsetRecdata-mask);
#else
							WriteEepromWithoutProtect(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[mask],OffsetRecdata-mask);
#endif
						}
						
						mask = OffsetRecdata+2;
						
						G_HTTPSStatus.HttpChunkYetReadLen = 0;
						
						G_HTTPSStatus.httpChunkedContentStatus = HTTP_ChunkedContent_Beginning;
	
						break;	
					
					default:
						G_HTTPSStatus.httpException = HTTP_OK;
						
						return dataLen;
						
				}
			}
			OffsetRecdata++;		
		}
		
		if ((OffsetRecdata + 1) == dataLen)		
		{
			if (G_HTTPSStatus.httpChunkedContentStatus == HTTP_ChunkedContent_Beginning)
			{
				if (OffsetRecdata < mask)
				{
					return dataLen;
				}
				
				G_HTTPSStatus.ChunkedSizeBuf[0] = dataLen - mask;
				
				memcpy(G_HTTPSStatus.ChunkedSizeBuf+1, RecData+mask, G_HTTPSStatus.ChunkedSizeBuf[0]);	
			}
			else if (G_HTTPSStatus.httpChunkedContentStatus == HTTP_ChunkedContent_Processing)
			{
//				ExitCmdLen = ReadTFFileWord(httpCommandFileAddr);
				ExitCmdLen = G_HTTP_CommandData_Length;
				
				G_HTTPSStatus.HttpChunkYetReadLen += (OffsetRecdata - mask);	
				
				if ((RecData[OffsetRecdata] == CR) && (G_HTTPSStatus.HttpChunkYetReadLen == G_HTTPSStatus.contentLen.chunkedSize))
				{
					G_HTTPSStatus.ChunkedSizeBuf[0] = 1;
					
					G_HTTPSStatus.ChunkedSizeBuf[1] = CR;
					
//					WriteTFFileValue(httpCommandFileAddr, 0x02+ExitCmdLen, &RecData[mask], OffsetRecdata-mask, ExitCmdLen+OffsetRecdata-mask);
					if((0x02+ExitCmdLen+OffsetRecdata-mask) < HTTP_CommandData_SIZE)
					{
//						SetWord(tmp, ExitCmdLen+OffsetRecdata-mask);
//						WriteEeprom(httpCommandFileAddr, tmp,0x02);
						G_HTTP_CommandData_Length = ExitCmdLen+OffsetRecdata-mask;
						
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
						ecpMem_Copy(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[mask],OffsetRecdata-mask);
#else
						WriteEepromWithoutProtect(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[mask],OffsetRecdata-mask);
#endif
					}
				}
				else 
				{
//					WriteTFFileValue(httpCommandFileAddr, 0x02+ExitCmdLen, &RecData[mask], dataLen-mask, ExitCmdLen+dataLen-mask);
					if((0x02+ExitCmdLen+dataLen-mask) < HTTP_CommandData_SIZE)
					{
//						SetWord(tmp, ExitCmdLen+dataLen-mask);
//						WriteEeprom(httpCommandFileAddr, tmp,0x02);
						G_HTTP_CommandData_Length = ExitCmdLen+dataLen-mask;
						
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
						ecpMem_Copy(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[mask],dataLen-mask);
#else
						WriteEeprom(httpCommandFileAddr+ramExtendResValue_offset+ExitCmdLen,&RecData[mask],dataLen-mask);
#endif
					}
					G_HTTPSStatus.HttpChunkYetReadLen++;
				}					
			}
			
			return dataLen;
		}
	}
	
	return dataLen;
}

/*
*@description: 将一串十六进制的数转成acsii值的字符串
*
*@param      :hexBuf :       			  十进制数据地址
*@param      :acsiiBuf :            十六进制数存放的地址
*@param      :dataLgth :          	hexBuf中有效字节数，即要被转换数据的字节数
*
*return		转换后acsiiBuf中有效字节数
*
*@history    :V1.0 :create by duan ss@20170901
*			 ：v1.1 modify by jiangqingbin @20210512,增加首字节是0时不用转成ascii码
*
*			eg：	0x0212->323132;  	0x12->3132;  	0x02->0x32;		 0x0002->0x32;	  0x0->0x30;  	0x0000->0x30
*/
BYTE HexStrToAscStr(BYTE * hexBuf, BYTE * acsiiBuf, BYTE dataLgth)
{
	BYTE	idata i,j;
	BYTE	idata high,low;

	j = 0;

	for (i=0; i<dataLgth; i++)
	{
		high = (hexBuf[i] & 0xF0) >> 4;

		low = (hexBuf[i] & 0x0F);

		acsiiBuf[j++] = DecToAsc(high);

		acsiiBuf[j++] = DecToAsc(low);
	}

	while ((acsiiBuf[0] == 0x30)&&(j > 1))		//去掉数据前面无效的0
	{
		memmove(&acsiiBuf[0],&acsiiBuf[1], j-1);

		j--;
	}

	return j;
}

/*
*@description:将ASCII码转换成16进制, 如果srcLen为奇数时，在原数据的最前面填充0x30,返回长度为(srclen+1)/2
*
*@function   :AsciiToHexInt ASCII码{0x31,0x32}->{0x12}此时12是十进制的12，再把12转换成0x0B
*
*@param      :BYTE * src    需要被转换成hex字符串和数组
*            :BYTE srcLen   src中有效数据的长度
*            :BYTE * dst    转换之后的hex字符串数组
*
*@return     :BYTE          返回dst中的有效数据
*
*@history    :xuxiaojun@20180517
*NOTE        :该函数与StringToInt函数可以优化成一个
*/
BYTE AsciiToHexInt(BYTE * src, BYTE srcLen, BYTE * dst)
{
	BYTE idata i, j;
	
	i=0;
	j=0;
	
	if(srcLen == 0)
	{
		return FALSE;
	}
	
	if(srcLen%2 != 0)
	{
		dst[j] = (CharToInt(0x30, ASCiitoHex)<<4) + CharToInt(src[i], ASCiitoHex);
		i=1;
		j=1;
	}

	for(; (i<=srcLen-1)&&( j<=((srcLen+1)/2)); i+=2,j++)
	{
		dst[j] = (CharToInt(src[i], ASCiitoHex)<<4) + CharToInt(src[i+1], ASCiitoHex);
	}
	
	return (srcLen+1)/2; 
}

/*
*@funtion    :AsciiToDec
*
*@description:将ASCII码转换为10进制{0x31,0x32}->12,此时12是十进制的12
*                                  {0x31,0x32,0x33}->123
*
*@param      :asciiBuf	需要被转换的ascii数据buffer
*@param		 :asciiLgth	需要被转换的数据长度
*            
*@return     :返回转换后的10进制数
*
*@history    :V1.0 :create by xuxiaojun@20180604
*/
UWORD AsciiToDec(BYTE *asciiBuf, BYTE asciiLgth)
{
	BYTE idata i;
	
	UWORD idata dec = 0;
	
	for (i = 0; i < asciiLgth; i++)
	{
		if ((asciiBuf[i] >= 0x30) && (asciiBuf[i] <= 0x39)) 
		{
				dec = (dec * 10) + asciiBuf[i] - 0x30;
		} 
		else 
		{
				break;
		}
	}

	return dec;
}

/*
*@description:对http的下行报文头进行数据解析 ,判断下行报文的http是否结束，是否偏移到http体， 并对G_HTTPSStatus中的状态进行赋值操作
*            :每次从EF_HTTPSHeadSave文件中读取CT_APDU_BUFFER_SIZE-5数据到RecData中，然后对RecData中的数据进行解析，如果解析到RecData的尾部数据不够一个协议头时，则将未处理的数据移动到RecData[0]的位置，同时再从文件中读取数据接着未处理数据的后部存放
*             由于EF_HTTPSHeadSave中也可能有EF_HTTPSCommandData数据，所以，需要将未处理的command数据写入到EF_HTTPSCommandData文件中
*
*@function   :HTTPSAnalyseHead
*
*@param      :void
*
*@return     :BYTE FALSE表示当前解析过程中出现了错误，后续不需要再往下进行
*                  TRUE 表示当前解析数据正常，后续继续往下解析
*
*@history    :xuxiaojun@20180517
*            :modify by xuxiaojun@20180629 根据规范，HTTP也是一来一回的交互模式，所以下行的17不应该出现300D0A之后还有数据的情况,如果还有数据则抛错
*            :modify by jiangqb@20191120   增加在https头解析错误时清空响应数据的文件的操作
*
*NOTE        :由于该函数的调用者借用了tempBuf，所以该函数不能再借用tempBuf，否则之前的有效数据可能会被清掉
*             该函数的实现借用RecData[0, CT_APDU_BUFFER_SIZE-5]
*            :modify by xuxiaojun@20180831  该函数中不需要清G_CurrentComNUM变量，因为每次进入ExecuteCompactRFMData时会清，参考IF_AnnexB_ProcessRAMRFMAPDU
*/
void HTTPSAnalyseHead(void)
{
	UWORD xdata headFileAddr;
	WORD idata httpHeadLen, offsetFileHead;
	BYTE idata lastFlag, currentReadLen,remainRecData,offsetRecData;
	
	lastFlag = 0;
	
	offsetFileHead = 2;        					//当前从httpHead文件中读取数据的偏移
	offsetRecData = 0;         					//当前已经处理的数据尾部在RecData中的偏移
	remainRecData = 0;         					//当前RecData中未处理的数据首字节在RecData中的偏移 (尾字节吧？？？)
	currentReadLen = 0xFA;						//循环读取文件EF_HTTPSHeadSave的数据时默认读取大小
		
	G_HTTPSStatus.httpException = HTTP_OK;

//	headFileAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Send);          	//SelectFile
	headFileAddr = (UWORD)BIPModule_Send_ADDR_START;

//	httpHeadLen = ReadTFFileWord(headFileAddr);
	httpHeadLen = G_BIPModule_Send_Length;
	
	while(1)
	{
		if (lastFlag == 0)
		{
			currentReadLen = 0xFA - remainRecData;
			
			if (httpHeadLen <= currentReadLen)			//剩余的长度不够 currentReadLen
			{
				currentReadLen = httpHeadLen;
				lastFlag = 1;
			}
			
//			ReadTFBuf(headFileAddr, offsetFileHead, &RecData[remainRecData], currentReadLen);   // HTTP/1.1 200 OK CRLF，这里HTTP/1.1必须存在，200 OK可以不存在，不存在时默认状态字OK。如果是204则表示不正常
			ReadEeprom(&RecData[remainRecData], headFileAddr+offsetFileHead, currentReadLen);
			
			offsetFileHead += currentReadLen;
			
			httpHeadLen -= currentReadLen;
			
			offsetRecData = 0;
		}
		else
		{
			currentReadLen = 0;
		}
		
		if(G_HTTPSStatus.httpDataStatus == HTTP_DataType_HEAD)
		{
			offsetRecData = HTTPAnalyseHeadSub(remainRecData + currentReadLen);   //offsetRecData前面的数据已经处理，接下来处理offsetRecData后的数据，既当前RecData中已经处理的数据为offsetRecData字节
		
			remainRecData = remainRecData + currentReadLen - offsetRecData;       //remainRecData=0时，表示当前的RecData数据刚好是完整的，未出现被截断的情况
			
			if(G_HTTPSStatus.httpException != HTTP_OK)      
			{		
//				ClearFileFirstWord(DF_BIPModule, EF_BIPModule_Send);
				G_BIPModule_Send_Length = 0x00;
				
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM	
				ecpMem_Fill(&HTTP_ResponseData_ADDR[0], 0x00, 2);
#else			
				ClearFileFirstWord(DF_TLS, EF_HTTPSResponseData);	 //清空https响应文件数据	
#endif
				
				return;
			}
			
			G_HTTPSStatus.IsHTTPCommandExecueError = POR_OK;			//新一次的http协议头存在的时候标识是新一轮的命令会话	//还是用切换目标SDId来标识
			
			if (G_HTTPSStatus.httpDataStatus == HTTP_DataType_BODY)			//标识http解析结束
			{
#if   UICC__FUNCTION__RFM				//RFM
				if(G_HTTPSStatus.httpCommandType == HTTP_Command_Type_RFM)		
				{
					if((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_TARGETED_APP_EXIST) != HTTP_HEAD_TARGETED_APP_EXIST)
					{
						G_HTTPSStatus.httpException = HTTP_HEAD_ERROR;

//						ClearFileFirstWordAddr(headFileAddr);
						G_BIPModule_Send_Length = 0x00;
						
						return;
					}					
				}
				else 
#endif
				{
#if UICC__FUNCTION__RAM	//RAM
					if ((G_HTTPSStatus.httpHeadExit & HTTP_HEAD_TARGETED_APP_EXIST) != HTTP_HEAD_TARGETED_APP_EXIST)			//HTTP_HEAD_TARGETED_APP字段不存在
					{
				
						HttpRAMTargetAppiddefaultProcess();		//ram没有指定targetaid时的处理
					
					}				
#endif			
				}									
				G_HTTPSStatus.HttpChunkYetReadLen = 0;
				
//				ClearFileFirstWordAddr(headFileAddr);
				G_BIPModule_Send_Length = 0x00;
				
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
				ecpMem_Fill(&HTTP_ResponseData_ADDR[0], 0x00, 2);
#else				
				ClearFileFirstWord(DF_TLS, EF_HTTPSResponseData);	 //清空https响应文件数据	
#endif				
				
				//ClearFileFirstWord(DF_Toolkit, EF_RAMExtendResponse);	 //清空226模块响应文件数据	
				
				if (lastFlag == 1)					//解析头的数据里 最后的一部分数据是Body的数据
				{
					memmove(&RecData[0], &RecData[offsetRecData], remainRecData);
					if (remainRecData == 0)
					{
						return;
					}
					continue;
				}
			}
		}
		else if(G_HTTPSStatus.httpDataStatus == HTTP_DataType_BODY)               //Body解析
		{		
			offsetRecData = HTTPAnalyseBodySub(remainRecData + currentReadLen);	
			
			if (offsetRecData > (remainRecData +currentReadLen))		//返回的实际处理的数据总长度有可能大于传入数据总长度， 因为HTTPAnalyseBodySub()函数内部有可能加上上一次剩余的数据在本次解析中处理
			{
				remainRecData = 0;
			}
			else
			{
				remainRecData = remainRecData + currentReadLen - offsetRecData;			
			}
			
			if(G_HTTPSStatus.httpException != HTTP_OK)      
			{			
//				ClearFileFirstWordAddr(headFileAddr);	
				G_BIPModule_Send_Length = 0x00;
				return;
			}
		}

		if (lastFlag == 0)		//不是最后一次读取数据
		{
			memmove(&RecData[0], &RecData[offsetRecData], remainRecData);
			continue;
		}
		else 						//不完整数据 并且当前文件数据已全部读出来了，此时要回到tls层继续解密
		{			
//			WriteTFFileValue(headFileAddr, 0x02, &RecData[offsetRecData], remainRecData, remainRecData);			//有头数据未处理完，需要将其更新进		
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(headFileAddr+2,  &RecData[offsetRecData],remainRecData); // 数据往前搬移，不会越界
#else
			WriteEeprom(headFileAddr+2,  &RecData[offsetRecData],remainRecData); // 数据往前搬移，不会越界
#endif
			G_BIPModule_Send_Length = remainRecData;
			return;
		}
	}				
}


/*
*@description:对SMS下行的http的触发参数的84tag分析
*@see        :<ts_102223v110200p.pdf> 6.6.27 OPEN CHANNEL描述了open channel必须存在的参数
*
*@function   :HTTPTriggerConnectionParaProc, 针对84内部tag数据进行整合, 相同的tag的时候优先使用destBuf(sms/app传递的触发参数 or isdp)中的参数
*											优先使用destBuf中的子tag，如果destBuf中没有某项子tag而srcBuf有，则会把srcBuf中的该子tag复制到destBuf中，最后destBuf中数据为要获取的数据
*
*@param      :BYTE *destBuf             触发短信中的参数 or 当前激活的isdp的参数
*            :BYTE destBufLen        	触发短信中的参数长度 
*			 :WORD maxSize				destBuf最大还可以存储空间
*            :BYTE *srcBuf              触发sd的参数   or isdr的参数
*            :BYTE srcBufLen            触发sd的参数长度
*
*@return     :resultLen, 两组参数整合后的总长度
*
*@history    :jiangqb@20180820
*/
BYTE HTTPTriggerConnectionParaProc(BYTE * destBuf, BYTE destBufLen, WORD maxSize, BYTE * srcBuf, BYTE srcBufLen)
{
	BYTE idata destBufOff, srcBufOff;
	
	destBufOff = 0;
	
	srcBufOff = 0;
	
	while (srcBufOff < srcBufLen)			// 相同的tag的时候优先使用destBuf(sms/app传递的触发参数 or isdp)中的参数
	{		
		while (destBufOff < destBufLen)						
		{
			if ((destBuf[destBufOff]&0x7F) == (srcBuf[srcBufOff]&0x7F))		//destBuf中已存在该子tag，不用替换
			{		
				srcBufOff += srcBuf[srcBufOff+1] + 2;

				destBufOff = 0;

				if(srcBufOff >= srcBufLen)
				{
					break;
				}
				else
				{
					continue;
				}
			}

			destBufOff += destBuf[destBufOff+1] + 2;
		}

		destBufOff = 0;

		if(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Optional_Tag))//标识不需要非必选的tag， esim-M2M-下载profile后的https通用脚本目前不期望获取非必选tag
		{
		}
		else
		{
			if  ( ((srcBuf[srcBufOff]&0x7F) != 0x35) && ((srcBuf[srcBufOff]&0x7F) != 0x39) && ((srcBuf[srcBufOff]&0x7F) != 0x3C) && ((srcBuf[srcBufOff]&0x7F) != 0x3E))
			{
				if ((srcBufOff+srcBuf[srcBufOff+1] + 2) <= srcBufLen)
				{
					srcBufOff += srcBuf[srcBufOff+1] + 2;
				}

				continue;			//可选tag将被略过
			}
		}

		if (srcBufOff < srcBufLen)		//srcBuf 到最后的时候直接跳出， 不能再复制
		{
			if ((destBufLen+srcBuf[srcBufOff+1]+2) > maxSize)		//数组越界返回
			{
				return FALSE;
			}
				
			memcpy(&destBuf[destBufLen], &srcBuf[srcBufOff], srcBuf[srcBufOff+1]+2);
			
			destBufLen += srcBuf[srcBufOff+1] + 2;
			
			if ((srcBuf[srcBufOff]&0x3F) == 0x0D)		//当前tag是 0D, 
			{
				srcBufOff += srcBuf[srcBufOff+1] + 2;
				
				if ((srcBuf[srcBufOff]&0x3F) == 0x0D)		//下一个tag还是 0D, 当两个 0D 连着的时候才可以，否则会以后后面的那个 0D tag
				{
					memcpy(&destBuf[destBufLen], &srcBuf[srcBufOff], srcBuf[srcBufOff+1]+2);
					
					destBufLen += srcBuf[srcBufOff+1] + 2;

					srcBufOff += srcBuf[srcBufOff+1] + 2;
				}		
			}
			else
			{
				srcBufOff += srcBuf[srcBufOff+1] + 2;
			}			
		}
		
	}
	
	return destBufLen;
}



/*
*@description: 对触发参数处理的子函数，当短信或api没有对应的触发tag时，获取对应sd的触发tag，为加快速度，减少写文件，tag参数每两个 84和85， 86和8a， 8b和8c 写入一次文件
*
*@function   :saveHttpsParaForSession, 
*
*@param      :BYTE tag              		当前处理的tag
*            :BYTE srcAddr        	 		改tag有触发参数时，tag的地址
*            :BYTE mode               		触发参数是都具有当前tag的值， 1是具有， 0是没有
*
*@return     :处理成功 or 失败
*
*@history    :creat by jiangqb@20191203
*			 :modify by jiangqb-20210319-同步tpx-5g和epx-98m25&swp中的关于http触发参数边界的测试中发现的问题
*/
BYTE saveHttpsParaForSession(BYTE triggeredSD, BYTE tag, UWORD srcAddr, BYTE mode)
{
	BYTE idata /*findTag,*/ index;
	BYTE xdata temp[8];
	WORD idata subLen, offsetTempBuf, pos;
	UWORD xdata triggerParaFileAddr;
	
#if !NATIVE_CARD_UICC		
	WORD idata subLen2;
#endif
	
#if M2M_EUICC
	BYTE idata sub84Len;
#endif
	pos = 0;

	index = 0;

	subLen = 0;

	
		
	offsetTempBuf = 0;
	
	memset(temp, 0, sizeof(temp));
	
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/1A03存储触发参数
	
	//GetLFRecNumLgth(triggerParaFileAddr, FileLgth);
	
// 	findTag = tag;

// 	if ((tag == HTTP_TriggerPara_AdmHost_TAG) || (tag == HTTP_TriggerPara_AgentId_TAG) || (tag == HTTP_TriggerPara_AdmURI_TAG))
// 	{
// 		findTag = HTTP_TriggerPara_Post_TAG;
// 	}

	if (mode == TRUE)		//触发参数存在该tag
	{
		//VMReadBuf(ResData, srcAddr, 0xFF);
		VMReadBuf(ResData, srcAddr, sizeof(ResData));

		subLen = GetBerTlvLength(ResData+1, &pos);
	}
	else
	{
		if ((tag == HTTP_TriggerPara_Connection_TAG) || (tag == HTTP_TriggerPara_Security_TAG) || (tag == HTTP_TriggerPara_Retry_TAG))
		{
			subLen = IF_GetHttpTriggerParaBySDID(triggeredSD, tag, ResData+2, 0xFD);		//RecData

			if (subLen < 0x80)
			{
				pos = 2;
			}
			//else if (subLen <= 0xff)
			else if (subLen <= 0xfd)
			{
				pos = 3;

				memmove(ResData+3, ResData+2,subLen);
			}
			else
			{
				return FALSE;
			}
		}
	}

	switch(tag)
	{
		case HTTP_TriggerPara_Connection_TAG:	//0x84
#if JAVA_CARD_UICC		
#if M2M_EUICC
			if (triggeredSD == ISDR_APPID)
			{
				sub84Len = IF_M2M_GetEnableISDPHttpConnectPara(tempBuf);		//isdr会话时，优先使用当前激活的isdp的链接参数， 获取当前激活的isdp的http链接参数, 返回TLVTLV..格式，需要找到A1 TAG (A1 tag 是配置http的参数)

				offsetTempBuf = FindTag(tempBuf, 0xA1, sub84Len);						//'A1'  n  Connectivity parameters for HTTP protocol 参考SGP02的3A07接口

				if (offsetTempBuf != 0)
				{
					if (mode == FALSE)	//触发参数不存在84， 优先是由isdp参数
					{
						subLen = HTTPTriggerConnectionParaProc(tempBuf+offsetTempBuf+1, tempBuf[offsetTempBuf], 0xff-subLen, ResData+pos, subLen);		//offsetTempBuf 是 A1 的 L的位置

						memcpy(ResData+pos, tempBuf+offsetTempBuf+1, subLen);
					}
					else				//触发参数存在84， 优先是由触发参数，再次isdp， 再次isdr
					{
						subLen = HTTPTriggerConnectionParaProc(ResData+pos, subLen, 0xff-subLen, tempBuf+offsetTempBuf+1, tempBuf[offsetTempBuf]);		//offsetTempBuf 是 A1 的 L的位置
					}
				}
			}
#endif

			if(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__ConnectionPara_With_TriggeredSD))
			{
				if (mode != FALSE)		//触发参数中存在84tag， 对触发参数和触发sd的参数进行整合
				{
					subLen2 = IF_GetHttpTriggerParaBySDID(triggeredSD, tag, tempBuf, 0xFD);

					if (subLen2 > 0)
					{
						subLen = HTTPTriggerConnectionParaProc(ResData+pos, subLen, 0xFF, tempBuf, subLen2);
					}
				}
			}

			if(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__ConnectionPara_With_ISD))
			{
				//根据规范 GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf 的 3.7章节：
				/*If parameters are missing in the triggering message, they shall be completed with the parameters, if available,
				of the triggered Security Domain. If parameters are still missing, they shall be completed with the
				parameters, if available, of the Issuer Security Domain*/
#if M2M_EUICC
				if ((triggeredSD != ISD_APPID) && (triggeredSD != ISDR_APPID))	//此处是否需要 与上（subLen>0）
#else
				if (triggeredSD != ISD_APPID)
#endif
				{
					subLen2 = IF_GetHttpTriggerParaBySDID(ISD_APPID, tag, tempBuf, 0xFD);

					if (subLen2 > 0)
					{
						subLen = HTTPTriggerConnectionParaProc(ResData+pos, subLen, 0xFF, tempBuf, subLen2);
					}
				}
			}
#endif
			index = HTTP_TriggerPara_ConnectionPara_Index;
			break;

		case HTTP_TriggerPara_Security_TAG:		//0x85
#if JAVA_CARD_UICC	
			if (subLen == 0)	//触发参数和触发sd都没有该tag的值
			{
				subLen = IF_GetHttpTriggerParaBySDID(ISD_APPID, tag, ResData+pos, 0xFD);
			}
#endif
			index = HTTP_TriggerPara_SecurityPara_Index;
			break;

		case HTTP_TriggerPara_Retry_TAG:		//0x86
#if JAVA_CARD_UICC
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
			if (subLen == 0)	//触发参数和触发sd都没有该tag的值
			{
				subLen = IF_GetHttpTriggerParaBySDID(ISD_APPID, tag, ResData+pos, 0xFD);
			}
#else
			subLen = 0;
#endif
#endif
			index = HTTP_TriggerPara_RetryPara_Index;
			break;

		case HTTP_TriggerPara_AdmHost_TAG:		//0x8A
			index = HTTP_TriggerPara_HostPara_Index;
			break;

		case HTTP_TriggerPara_AgentId_TAG:		//0x8B
			index = HTTP_TriggerPara_AgentIdPara_Index;
			break;

		case HTTP_TriggerPara_AdmURI_TAG:		//0x8C
			index = HTTP_TriggerPara_URIPara_Index;
			break;
	}

	if ((tag == HTTP_TriggerPara_AdmHost_TAG) || (tag == HTTP_TriggerPara_AgentId_TAG) || (tag == HTTP_TriggerPara_AdmURI_TAG))
	{
		if (mode == FALSE)
		{
			subLen = IF_GetHttpTriggerParaBySDID(triggeredSD, HTTP_TriggerPara_Post_TAG, tempBuf, sizeof(tempBuf));	//再次读取triggeredSD的89tag，防止上一次使用RecData读取时由于数组长度过小而读取失败
#if JAVA_CARD_UICC
			if ((triggeredSD != ISD_APPID) && (subLen == 0))
			{
#if M2M_EUICC
				if(triggeredSD != ISDR_APPID)
#endif
				{
					subLen = IF_GetHttpTriggerParaBySDID(ISD_APPID, HTTP_TriggerPara_Post_TAG, tempBuf, sizeof(tempBuf));
				}
			}
#endif
			if (subLen == 0)
			{
				return FALSE;
			}

			offsetTempBuf = FindTag(tempBuf, tag ,subLen);		//找8A/8B/8C 的L所在位置

			pos = 0;

			subLen =  GetBerTlvLength(tempBuf+offsetTempBuf, &pos);

			if (pos == 2)
			{
				memcpy(ResData+pos, tempBuf+offsetTempBuf+1, subLen);
			}
			else if (pos == 3)
			{
				memcpy(ResData+pos, tempBuf+offsetTempBuf+2, subLen);
			}
			else
			{
				return FALSE;
			}
		}
	}

	if ((subLen == 0) && (tag != HTTP_TriggerPara_Retry_TAG))	//只有86tag可以不存在
	{
		return FALSE;
	}

	WriteLFFileValue(triggerParaFileAddr, index, ResData+pos, subLen);

	return TRUE;
}




/*
*@description:对SMS下行的http的触发参数进行处理
*@see        :<ts_102223v110200p.pdf> 6.6.27 OPEN CHANNEL描述了open channel必须存在的参数
*@function   :HTTPSAnalyseTriggerPara
*
*@param      :UWORD fileAddr            保存SMS的文件
*            :WORD commandOffset        SMS中的command起始在SMS文件中的偏移
*            :WORD cplLen               SMS数据长度
*            :BYTE sdID                 被触发的sd的ID
*
*@return     :void
*
*@history    :xuxiaojun@20180524
*			 :modify -by jiangqb-20191202,5Gbip性能测试，为加快速度，使用G_HttpsBuf分别临时缓存 84和85， 86和8A, 8B和8C数据，分三次写入
*
*NOTE        :该函数内部会借用tempBuf用于临时存放sdID的84，85，86，89的值，其有效性仅在该函数内部。所以，该函数的调用者在调用该函数时，要确认在此之前tempBuf中没有有效数据，否则之前的有效数据会被冲掉
*/
BYTE HTTPSAnalyseTriggerPara(BYTE triggeredSDID, UWORD paraAddr)
{
	WORD idata length, offset, subLen, sub89Len, pos, subPos, offset89End;
	BYTE xdata triggerTag[8];
	WORD xdata tagOffset[6];		//6个字节分别表示 84,85,86,8A,8B,8C 的偏移
	//UWORD xdata triggerParaFileAddr, TLSsessionDataFile;
	BYTE idata isExit, tag, i;
		
	length = 0;
	
	subLen = 0;
	
	sub89Len = 0;
	
	isExit = 0;
	
	memset(triggerTag, 00, sizeof(triggerTag));
	
	memset(tagOffset, 00, sizeof(tagOffset));

	offset = 0;
	
	if (paraAddr != ADDR_IS_NULL)
	{
		VMReadBuf(&triggerTag[0],paraAddr, sizeof(triggerTag)); 
		
		if (triggerTag[0] != 0x81)			//握奇电信测试，app使用非81tag的触发参数 需要抛错
		{
			return FALSE;
		}
		
		length = GetBerTlvLength(&triggerTag[1],&offset);

		subLen = length+offset;		//包括81tag在内的总长度
	}
	else
	{
		length = 0;
	}
	
	if(length != 0)
	{
		if(triggerTag[offset] == 0x83)
		{
			length = GetBerTlvLength(&triggerTag[offset+1],&offset);

			if ((length + offset) != subLen)
			{
				return FALSE;
			}
		}

		length += offset;		
		
		while(offset < length) // merged from EPX产业化\STM_ST33G1M2\Branch\RSP\code: length-=pos的时候，可能会导致length为负数，陷入死循环
		{
			VMReadBuf(&triggerTag[0],paraAddr+offset, sizeof(triggerTag)); 
		
			pos = 0;
			
			subLen = GetBerTlvLength(triggerTag+1, &pos);		//返回L值， pos是LV字节数
			
			if ((subLen > 0xFD) && (triggerTag[0] != HTTP_TriggerPara_Post_TAG))
			{
				return FALSE;
			}
			
			if(subLen != 0)
			{
				switch(triggerTag[0])
				{
					case HTTP_TriggerPara_Connection_TAG:    //84 The connection parameters TLV embed all the needed parameters to establish a point to point TCP connection between the Administration Agent and the Remote administration server
					{						
						tagOffset[0] = offset;
						
						isExit |= HTTP_TriggerPara_Connection_Eixt;

						break;
					}
					case HTTP_TriggerPara_Security_TAG:		//85
					{		
						tagOffset[1] = offset;

						isExit |= HTTP_TriggerPara_Security_Eixt;

						break;
					}
					case HTTP_TriggerPara_Retry_TAG:		//86
					{
						tagOffset[2] = offset;

						isExit |= HTTP_TriggerPara_Retry_Eixt;

						break;
					}
					case HTTP_TriggerPara_Post_TAG:		   //89
					{		
						offset += pos;

						offset89End = offset + subLen;

						while(offset < offset89End )				//此处由 length 改为 subLen， 测UL_SCP81时，触发参数是乱序，89子tag后有85子tag，修改前有bug不能回到子85tag
						{		
							VMReadBuf(&triggerTag[0], paraAddr+offset, sizeof(triggerTag));
							
							subPos = 0;
							
							sub89Len = GetBerTlvLength(triggerTag+1, &subPos);		//返回L值， pos是LV字节数
							
							if (sub89Len > 0xFD)
							{
								return FALSE;
							}

							if (sub89Len != 0)
							{
								switch(triggerTag[0])
								{
									case HTTP_TriggerPara_AdmHost_TAG:		//8A
									{
										tagOffset[3] = offset;

										isExit |= HTTP_TriggerPara_AdmHost_Eixt;

										break;
									}
									
									case HTTP_TriggerPara_AgentId_TAG :		//8B
									{
										tagOffset[4] = offset;

										isExit |= HTTP_TriggerPara_AgentId_Eixt;

										break;
									}
									
									case HTTP_TriggerPara_AdmURI_TAG:	//8C
									{
										tagOffset[5] = offset;

										isExit |= HTTP_TriggerPara_AdmURI_Eixt;

										break;
									}
									default:
										break;
								}
							}

							offset += subPos + sub89Len;
						}
						continue;	//此处不能break，因为在89tag内部offset已经做了偏移，不能再往后执行再一次进行偏移操作
					}
					default:
						break;
				}
			}
			
			offset += pos + subLen;
		}
			
	}
	
	tag = HTTP_TriggerPara_Connection_TAG;
	
	for (i=0; i < 6; i++)
	{	
		if (!saveHttpsParaForSession(triggeredSDID, tag+i, paraAddr+tagOffset[i], ValueBit(isExit, i)))
		{
			return FALSE;
		}
		
		if (i == 2)
		{
			tag = 0x87;
		}
	}
#if (GP__AMENDMENT__B__createSCP81Connection_FOR_MASK || GP__AMENDMENT__B__requestHTTPAdministrationSession )
	G_HTTP_APITriggerTag = HTTPTriggeredByAPPUnDone; // 触发参数合法，后续需要进行触发。对于短信触发来说，G_HTTP_APITriggerTag是无效的
#endif
	return TRUE;
}
/*
*@description:如果当前的HTTP会话是由应用触发的，该函数需要通知该应用HTTP会话的对应结果. 同时清除HTTP会话的相关变量
*
*@function   :HttpSessionReport
*
*@param      :void
*
*@return     :void
*                   
*@history    :created by xuxiaojun@20180815
*/
void HttpSessionReport(BYTE httpException)
{
#if JAVA_CARD_UICC
	if(G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredByHTTPAdministration)
	{
		G_HTTPSStatus.httpException = httpException;

		GPSubCallFunction(HttpAdministationSessionReport_METHOD_OFFSET, JC_FRAME_WORK_PKG_ID, JCRE_CONTEXT);

		HttpProtocolInit();       //清除HTTPS层相关变量

		TLSInitVariable();        //清除TLS相关的全局变量
	}
#endif
}

/*
*@description:开始HTTP会话，具体执行的操作包括：开通道，TLS握手
*
*@function   :HTTPSessionStart
*
*@param      :void
*
*@return     :BYTE  返回HTTPSessionStart的结果，如果当前打开通道成功，则返回TRUE; 如果当前打开通道不成功，但是重试次数不为0也返回TRUE，等待下次继续重试；如果当前通道打开不成功，且重试次数为0，则返回FALSE。
*                   
*
*@history    :xuxiaojun@20180524
*NOTE        :该函数内部会借用tempBuf用于临时存放sdID的84，85，86，89的值，其有效性仅在该函数内部。所以，该函数的调用者在调用该函数时，要确认在此之前tempBuf中没有有效数据，否则之前的有效数据会被冲掉
*            :modify by xuxiaojun@20180718  如果当前是通过HTTP API的方式触发会话的话，该应用的作用是触发HTTP的open channel，但是后续的操作应该是在触发完成之后，而不应该在应用过程中，否则一但client hello返回之后，等待服务器下来server hello时就会进入到应用的处理流程。
*            :modify by xuxiaojun@20180726  如果开通道失败，则需要根据Retry policy重新打开 <20180725190712_LYf LS 4510_APM ON during BIP Session and then 3 Retry attempts_other vendor card.txt>
*            :modify by xuxiaojun@20180726  由于open channel时会判断G_BIPModuleStatus.channelNum != 0x00， 所以不能在HTTPSessionStart中调用BIPModuleInit. 
*            :modify by xuxiaojun@20200622  三种触发方式开会话方式统一
*            :modify by xuxiaojun@20210423  同步TPX移动5G物联网公司测试发现的BUG： HTTP开通道时，如果openChannel的返回状态为0x3X时，不值得重试，移动物联网测试时反馈的
*            :modify by jiangqb@20220418  增加宏开关GP__AMENDMENT__B__SUPPORT_SESSION_RETRY管理重试代码，增加电信最新特殊需求代码
*/
BYTE HTTPSessionStart(void)
{
	BYTE idata result;
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY	
    UWORD xdata fileAddr;
#endif
#if (GP__AMENDMENT__B__createSCP81Connection_FOR_MASK || GP__AMENDMENT__B__requestHTTPAdministrationSession )
    G_HTTP_APITriggerTag = HTTPTriggeredByAPPHasDone; // 对于重试的情况，会多次进入该函数，所以G_HTTP_APITriggerTag会被多次连续赋值为HTTPTriggeredByAPPHasDone
#endif	
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY		
	fileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData);
	
	if(!ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Open_Flag))
	{
		SetTFConst(fileAddr, TLS_OpenCloseNormal_Offset, Https_Session_Running, 0x01);  					 //进入该函数, 先将标记打上，正常关闭后将标记清除。 因为OPEN CHANEL属于HTTP开启会话的过程，不能将其剥离开《201809282104 MI2 Restart after retry but no PLI for open channel.txt》
	}
	
	SetTFConst(fileAddr, TLS_CurrentTriggerAppid_Offset, G_HTTPSStatus.TriggeredAppId, 0x01); 
	
	SetTFConst(fileAddr, TLS_CurrentTriggerSD_Offset, G_HTTPSStatus.TriggeredSD, 0x01);			 //当前会话的安全域ID记录到文件， 用于手机重启之后重试之前通过API触发的HTTP会话结束后通知对应的应用
				
	SetTFConst(fileAddr, TLS_CurrentTriggerMode_Offset, G_HTTPSStatus.HTTPTriggerMode, 0x01);  
#if !SE_ONLY			
	SetTFConst(fileAddr, TLS_CurrentExecutingProfileIndex_Offset,G_HttpExecutingProfileIndex, 0x01);
#endif
#endif

	result = BIPModuleOpenChannel(ChannelDataType_HTTPS); // 该函数内部会获取G_HTTPSStatus.RetryConterRemain的值

	result = IF_OpenChannel_Retry(result);	//todo：后续优化的时候，可以考虑在 BIPModuleOpenChannel 函数中直接再次发送主动式命令，这样不用再次拼装主动式命令
	
	if(CMD_Success != result)
	{
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY
		if(!ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__OpenChannel_With_3X_Retry)) // 这里是否有必要做成配置
		{
			if((0x30 <= result) && (result <= 0x3F)) //<102223> Results '3X' indicate that it is not worth the UICC re-trying with an identical command, as it will only get the same response. However, the decision to retry lies with the application.
			{
				G_HTTPSStatus.RetryConterRemain = 0x00;

				SetTFConst(fileAddr, TLS_OpenCloseNormal_Offset, Https_Session_End, 0x01);

				SetLFConst(SelectPrivateTFFile(DF_TLS, EF_HTTPTriggerPara), HTTP_TriggerPara_RetryPara_Index, 2, 0x00, 2);
			}
		}

		if(G_HTTPSStatus.RetryConterRemain > 0)
		{
			IF_AnnexB_StartTimerAfterUnNormalEnd();
			
			return TRUE;			
		}
#endif
		if (G_HTTPSStatus.RetryConterRemain == 0)			//当前会话的重试次数用完， 触发等待中的下一个http会话
		{
			G_HTTPSStatus.HTTPIsResume = FALSE;  
			
			G_HTTPSStatus.httpException = HTTP_RETRY_EXHAUSTED;
			
			HttpSessionReport(HTTP_RETRY_EXHAUSTED);  //通知当前触发HTTP会话的应用

#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK			
			Scp81ConnectionReport(SCP81_CONNECTION_SETUP_FAILED);
#endif			
			G_HTTPSStatus.HTTPTriggerMode = HTTPTriggeredNull; //重试次数耗尽之后，该变量恢复到初始值
			
			if(TRUE == IF_AnnexB_HTTP_SecondSession_Start())   //返回TRUE表示你有第二条触发短信等待开启
			{
				HTTPSessionStart();

				return TRUE;	
			}
		}
		
		return FALSE;			
	}
	
	memset((BYTE *)&G_TLSSecurityParameters, 0x00, sizeof(G_TLSSecurityParameters));
	
	G_TLSSecurityParameters.tlsRecordState = TLS_SEND_CLIENT_HELLO_STATE;

#if JAVA_CARD_UICC	
	if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_Setupeventlist) == NO_SETUPEVENTLIST)
	{
		SetupEvenList(Openchannel_Mode);
	}
#endif			
	TLSHandProcess(0, 0); 
	
	return TRUE;
}

/*
	openchannel：
	Bearer description  											'35' or 'B5' 
	Buffer size 													'39' or 'B9' 
	Network Access Name  											'47' or 'C7'  
	Other address (local address)							        '3E' or 'BE' 
	Text String (User login)  								        '0D' or '8D' 
	Text String (User password)  							        '0D' or '8D' 
	UICC/terminal interface transport level  	                    '3C' or 'BC'  
	Data destination address  								        '3E' or 'BE' 
	Text Attribute  												'50' or 'D0' 
	Frame Identifier  												'68' or 'E8' 
	*/
/*
*@description:获取HTTP协议下开通道的相关参数
*@See        :102223
*@function   :GetOpenChannelParaForHttp
*
*@param      :BYTE *para      获取到的open channel相关参数，返回格式为TLV+TLV+TLV格式。文件中存的是什么，返回的就是什么
*
*@return     :void
*
*@history    :xuxiaojun@20180528
*			:v1.1 modify by jiangqingbin-20220418: 增加是否支持重试开关GP__AMENDMENT__B__SUPPORT_SESSION_RETRY控制代码， todo：是否需要同步epx2.0-rsp-st产业化中增加dns功能时修改了http重试次数的保存位置
*/
BYTE GetOpenChannelParaForHttp(BYTE *para)
{
	UWORD xdata triggerParaFileAddr;
	WORD idata subLen;
	BYTE idata retryCounter[2];
	
	memset(retryCounter, 0x00, sizeof(retryCounter));
	
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/1A03存储触发参数

	subLen = ReadLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_ConnectionPara_Index, &para[0]);
#if GP__AMENDMENT__B__SUPPORT_SESSION_RETRY	
	ReadLFBuf(triggerParaFileAddr, HTTP_TriggerPara_RetryPara_Index, 2, retryCounter, 2);
#endif	
	G_HTTPSStatus.RetryConterRemain = BuildWord(retryCounter);
	
	return subLen;	
}
/*
*@description:https会话是被app触发时, 查找app关联的并且支持scp81协议的sdid，并且赋值到G_HTTPSStatus.TriggeredSD
*			 :用例： 触发的app关联的ssd2不支持scp81协议，ssd2的关联sd是ssd1，ssd1支持scp81协议，所以触发的sd是ssd1。 EG:连接服务API功能测试脚本/BIP功能测试脚本/RAM/T_002_HTTP_InstallBipApplet_underSSD2_Normal)
*
*@See        ：《ts_102267v130000p.pdf》5.5.0章节中有描述-TLS Security is handled by the associated (directly or indirectly) Security Domain of the card application which has initiated the SCP81Connection.
*			 ：《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 3.3.1章节中有描述：The Security Domain shall set up a communication channel with the Remote Administration Server, and then establish secure communications (see section 3.3.2) using its own PSK TLS key
*
*@function   :setHttpsTriggeredSD
*
*@param      :BYTE appid,	输入参数，当前触发会话的appid
*
*@return     :void
*
*@history    :creat by Jiangqb@20200804
*/


/*
*@description: https会话是被app触发时, 查找app关联的并且支持scp81协议的sdid，并且赋值到G_HTTPSStatus.TriggeredSD
*			 : 用例： 触发的app关联的ssd2不支持scp81协议，ssd2的关联sd是ssd1，ssd1支持scp81协议，所以触发的sd是ssd1。 EG:连接服务API功能测试脚本/BIP功能测试脚本/RAM/T_002_HTTP_InstallBipApplet_underSSD2_Normal)
*
*@See        ：《ts_102267v130000p.pdf》5.5.0章节中有描述-TLS Security is handled by the associated (directly or indirectly) Security Domain of the card application which has initiated the SCP81Connection. 
*			 ：《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf》 3.3.1章节中有描述：The Security Domain shall set up a communication channel with the Remote Administration Server, and then establish secure communications (see section 3.3.2) using its own PSK TLS key
*
*@function   :GetHttpsTriggeredSD
*
*@param      : BYTE appid,	输入参数，当前触发会话的appid
*
*@return     :触发的sdid or 找不到触发sd时的ILLEGAL_APPID
*
*@history    :creat by Jiangqb@20200804
*/
BYTE GetHTTPTriggeredSD(BYTE appID)
{
#if JAVA_CARD_UICC
	BYTE idata assoiatedSD;
	PN8	 sdDataObj;
	while (1)
	{
		assoiatedSD = GetAssoiatedSDByAppId(appID);

		sdDataObj =	MEMtPN8(getSdDataObjBySdid(assoiatedSD));

		if(sdData_getScpOptionByScpType(sdDataObj, SCP_81) != 0xFF)
		{
			return assoiatedSD; 
		}


		if(assoiatedSD == ISD_APPID)
		{
#if SE_ONLY
			return ISD_APPID; //默认ISD支持所有的协议
#else
			return ILLEGAL_APPID;
#endif
		}

				
		appID = assoiatedSD;
	}
#endif
	return ILLEGAL_APPID;
}

#endif



