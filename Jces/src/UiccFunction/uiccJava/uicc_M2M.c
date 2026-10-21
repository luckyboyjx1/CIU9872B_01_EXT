/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   Interface_PlatformFromLTE
 *
 * Description: 该文件是主要存放一些平台层向LTE 层申请资源，以及逻辑控制的一些接口函数，由LTE层人员负责实现函数内部功能，平台层人员负责提供接口说明等
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
#include "fileAction.h"
#include "uicc_M2M.h"
#include "STKAlert.h"
#include "stkhead.h"
#include "AES.h"
#include "Ceph.h"
#include "uicc_interface.h"
#include "ToolkitRFM.h"
#include "VMAccessEeprom.h"
#include "EUICCProfile.h"
#include "EUICCM2MAPDUPublic.h"
#include "fileSystem.h"
#include "TlsMainFlow.h"
#include "HttpProcess.h"
#include "ToolkitAPIPublic.h"
#include "TimerModuleProces.h"
#include "BIPModuleProcess.h"
#include "Interface_LTEFromPlatform.h"
#include "Interface_PlatformFromLTE.h"
#include    "uicc_interface.h"
#if UICC__CDMA__SMS__DOWNLOAD
#include "cdma.h"
#endif


#if M2M_EUICC
BYTE xdata G_Handle_Notification_Tag;              //开始处理通知 (一次开机过程中只处理一次)

BYTE  xdata G_NotificationSmsRetryTime;           //通知SMS重试次数

BYTE  xdata G_NotificationTimerId;                //发送通知SMS成功后开启的timer (最高bit 表示发送通知方式，1表示https,0表示SMS)

//BYTE  xdata G_NotificationConfir; 				  //用于发送通知确认por时， 判断当前是发送通知确认的por，获取isdp配置的tppid, tpdcs

BYTE  xdata G_IsSendHttpsNotification;            //正在通过https发送通知		

BYTE xdata G_RefreshAfterCloseBIP;            	//需要在关闭https 通道后再发送refresh  CATTP--eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/TC_ES5_DISP_2_DisableProfile_CAT_TP_01_Nominal_Case

BYTE xdata G_IMEI_FOR_NOTIFICATION[9];       	//存储发送通知需要的IMEI

BYTE xdata G_STATUSNUM_FOR_NOTIFICATION; 		//STATUS个数，10次以后发送通知
BYTE xdata G_STATUSNUM_FOR_REFRESH;             //高4bit表示refreshmode，低4bit表示剩余重发次数
/*
*function    : LocationStatusProcess
*
*@description:  对Location_status事件进行处理
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009,修改函数注释
*/
void LocationStatusProcess(void)
{
	BYTE  idata pos;

	G_STATUSNUM_FOR_NOTIFICATION = 0xFF;      //收到LocationStatus 后，status不再需要计数
	
	pos = FindTag(&RecData[2], TLV_LOCATIONSTATUS, RecData[1]);
	
	if(pos != FALSE)
	{
		PendingMessageAlert(LocationStatus_Alert,RecData[2+pos+1]);
	}		
}
/*
*function    : SendNotificationBySMS
*
*@description:  短信方式发送通知
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009,不在本函数中调用SendNotificationByHTTPS，退出函数再调用
*/
BYTE SendNotificationBySMS(void)
{
	BYTE  idata result, buf[2];
	UWORD xdata fileAddr;
	
	result = 0xFF;
	
	BuildNotificationBySMS();
			
	while(G_NotificationSmsRetryTime > 0)		
	{	
		G_NotificationSmsRetryTime --;	
	
		result = SendProactiveCommand();					

		//根据Scripts_additional\FirstNetworkAttachmentNotificationErrorTest01，result=0x31时不重发
		if((result < 0x12)||(result >= 0x30))
		{
			break;
		}
	}	
	
	if(result < 0x12)
	{	
		startNotificationTimer(NOTIFICATION_TYPE_SMS);						
	}
	else
	{
		//通知发送失败，使用https 方式重发通知
		if(IF_GetNotificationProtocol() == NOTIFICATION_SMS_HTTP)
		{
			G_NotificationSmsRetryTime = NOTIFICATION_RETRY_TIMES;			
			
			fileAddr = SelectPrivateTFFile(DF_Timer, EF_Timer_SMSSendNotice);
			
			if (fileAddr != FALSE)
			{
				ReadTFBuf(fileAddr, 0, buf, 2);
				
				G_NotificationSmsRetryTime = BuildWord(buf);
			}
			return FALSE;
		}
		//通知发送失败，并且已经达到最大发送次数，进入roll back/fall back流程 
		else
		{
			if(IF_ProfileChangeMechanism() == TRUE)
			{
				//roll back后使用新的NotificateNumerber 发送通知,by xwh 20190506
				IF_M2M_IncreaseNotificateNumerber();
				//IF_SetRefreshFlag();
				sendRefreshComm();
			}
		}
	}
	
	return TRUE;
}
/*
*function    : SendNotificationByHTTPS
*
*@description:  HTTP方式发送通知
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009,不在本函数中调用SendNotificationBySMS，退出函数再调用
*            :V1.2 modify by chentao@EPX产业化\INFENION_SLM97\Develop
*/
BYTE SendNotificationByHTTPS(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	BYTE idata result, buf[2];
	UWORD xdata fileAddr;
	
	result = ExitStkPerform;

	G_IsSendHttpsNotification = TRUE;

	if (!HttpsSessionFileCheck())
	{
		return FALSE;	
	}
	
	if (!HTTPSAnalyseTriggerPara(ISDR_APPID, ADDR_IS_NULL, HTTPTriggeredParaAnaly))
	{
		return FALSE;
	}
	
	G_HTTPSStatus.TriggeredSD = ISDR_APPID;		//isdr 发送通知

	while(G_NotificationSmsRetryTime > 0)	
	{
		G_NotificationSmsRetryTime --;	
	
		result = BIPModuleOpenChannel(ChannelDataType_HTTPS);
		
		if(result == CMD_Success)
		{
			if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_Setupeventlist) == NO_SETUPEVENTLIST) //开机发了setupeventlits openchannel后就不发 
			{
				SetupEvenList(Openchannel_Mode);
			}
			break;		
		}
	}

	if(result != CMD_Success)
	{
		G_IsSendHttpsNotification = FALSE;
	
		//开通道失败，使用SMS 方式重发通知 
		if(IF_GetNotificationProtocol() == NOTIFICATION_HTTP_SMS)
		{
			G_NotificationSmsRetryTime = NOTIFICATION_RETRY_TIMES;
			
			fileAddr = SelectPrivateTFFile(DF_Timer, EF_Timer_SMSSendNotice);
			
			if (fileAddr != FALSE)
			{
				ReadTFBuf(fileAddr, 0, buf, 2);
				
				G_NotificationSmsRetryTime = BuildWord(buf);
			}
			return FALSE;
		}
		//开通道失败，并且已经达到最大重试次数，进入roll back/fall back流程 
		else
		{					
			if(IF_ProfileChangeMechanism() == TRUE)
			{
				//roll back后使用新的NotificateNumerber 发送通知,by xwh 20190506
				IF_M2M_IncreaseNotificateNumerber();
				//IF_SetRefreshFlag();
				sendRefreshComm();
			}
		}

		return TRUE;
	}
	
	TLSInitVariable(); 
	
	G_TLSSecurityParameters.tlsRecordState = TLS_SEND_CLIENT_HELLO_STATE;
			
	TLSHandProcess(0, 0); 

#endif
	return TRUE;
}
/*
*@function   : IF_GetNotificationProtocol
*
*@description: 获取通知发送方式(SMS,HTTPS,CATTP)
*
			 : byte iccidlen -- ICCID的有效数据长度
*            : byte * iccid -- ICCID数据
*@return     : void
*
*@author     :	LCZ_2019-4-1
*
*@history    :created by xuxiaojun@20190402
*/
BYTE IF_GetNotificationProtocol(void)
{
	UWORD xdata paraAddr;
	BYTE  xdata buf[6];
	BYTE  idata offset,i,len,notiProtocol;

	notiProtocol = 0;

	paraAddr = GetProfileConnectivityParaAddr(JcvmConfig_GetProfileEnableIndex());//GetAddrByIndexAddr(PROFILE_METADATAREF + VMReadByte(PROFILE_ENABLEINDEX_CFG) * sizeof(WORD));

	if(paraAddr != ADDR_IS_NULL)
	{
		len = VMReadByte(paraAddr);

		offset = 0;

		paraAddr ++;

		for(i=0;(i<3)&&(offset<len);i++)
		{
			VMReadBuf(buf, paraAddr+offset,2);	

			if(buf[0] == 0xA0)
			{
				if(notiProtocol == 0)
				{
					notiProtocol = NOTIFICATION_ONLY_SMS;
				}
				else if(notiProtocol == NOTIFICATION_ONLY_HTTP)
				{
					notiProtocol = NOTIFICATION_HTTP_SMS;
				}	
			}
			else if(buf[0] == 0xA1)
			{
				if(notiProtocol == 0)
				{
					notiProtocol = NOTIFICATION_ONLY_HTTP;
				}
				else if(notiProtocol == NOTIFICATION_ONLY_SMS)
				{
					notiProtocol = NOTIFICATION_SMS_HTTP;
				}	
			}
			else
			{
				
			}

			offset += (buf[1]+2);
		}
	}
	
	return ((notiProtocol==0)?NOTIFICATION_ONLY_SMS:notiProtocol);
}

/*
*function    : HandleDefaultNotification
*
*@description:  在refresh(UICC reset)之后执行发送通知,roll back/fall back等操作
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*/
void HandleDefaultNotification(BYTE locationStatus)
{
	BYTE  idata notificationType,notificationProtocol,result,buf[2];
	UWORD xdata fileAddr;

    notificationType = NOTIFICATION_NON;
	
	//获取通知类型,IF_GetNotifitionType函数需要平台层实现
	notificationType = IF_GetNotifitionType();

	//一次开机流程中(reset前)只调用一次HandleDefaultNotification 函数
	//但是发送"第一次联网成功通知" 不受此限制
	if((G_Handle_Notification_Tag == TRUE) && (!((notificationType==NOTIFICATION_FIRST_TIME)&&(locationStatus == LocationStatus_Normal))))
	{
		return;
	}
	
	if (G_TerminalProfile[0] == 0)				//来locationStatus事件的时候还没有开机，没有收到TerminalProfile, 不做任何处理
	{
		G_NetworkStatus = locationStatus;

		G_STATUSNUM_FOR_NOTIFICATION = 0x00;      //此时收到的LocationStatus当做无效处理，status需要重新计数

		return;
	}
	
	if ((locationStatus == LocationStatus_Limited_Service) || (locationStatus == LocationStatus_NO_Service))		//统一做成在网络受限时，等待10个status	//2020-03-23	--海兴电力项目测试时 01状态要求 roll-back 或者 fall-back, 否则卡片无法回到有效的profile中，导致废卡
	{
		if (locationStatus == LocationStatus_NO_Service)
		{
			if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_RollBAckWhenNoService) == ROLLBACK_AT_ONCE)
			{
				if(IF_ProfileChangeMechanism() == TRUE)
				{
					IF_M2M_SetRefreshFlag();

					return;
				}
			}
		}

		G_NetworkStatus = locationStatus;

		G_STATUSNUM_FOR_NOTIFICATION = 0;							//开始计数status的标记

		return;
	}
	else if (locationStatus == LocationStatus_Limited_AND_NOService_10_Status)		//limit时计数到10个status
	{
		if(IF_ProfileChangeMechanism() == TRUE)
		{
			IF_M2M_SetRefreshFlag();
		}
		
		return;
	}
	else if((locationStatus == LocationStatus_Normal) || (locationStatus == LocationStatus_10_Status))//|| (locationStatus == LocationStatus_Limited_Service))
	{
		G_Handle_Notification_Tag = TRUE;
		
		G_STATUSNUM_FOR_NOTIFICATION = 0xFF;

		G_NetworkStatus = LocationStatus_NULL;

		//每次收到location status,恢复最大重发次数
		G_NotificationSmsRetryTime = NOTIFICATION_RETRY_TIMES; 
		
		fileAddr = SelectPrivateTFFile(DF_Timer, EF_Timer_SMSSendNotice);
			
		if (fileAddr != FALSE)
		{
			ReadTFBuf(fileAddr, 0, buf, 2);
			
			G_NotificationSmsRetryTime = BuildWord(buf);
		}
	
		if(notificationType == NOTIFICATION_NON)
		{
			return;
		}
		//以下情况需要发送通知
		//NOTIFICATION_FIRST_TIME表示还没有成功发送"第一次联网成功通知"
		else if((notificationType == NOTIFICATION_FIRST_TIME)||(notificationType == NOTIFICATION_RROFILE_CHANGE_SUCCEEDED)||(notificationType == NOTIFICATION_RROFILE_CHANGE_FAILED_AND_ROLLBACK)||(notificationType == NOTIFICATION_RROFILE_CHANGE_AFTER_FALLBACK))
		{
			G_IMEI_FOR_NOTIFICATION[0] = 0x00;

			//按照中国电信测试log , 10个status 触发的通知，不发送PROVIDELOCALINFORMATION， 通知不带IMEI 这一项，规范没有说明这一点，暂时这样做,by xwh 20190522
			if(locationStatus != LocationStatus_10_Status)
			{
				InitProactiveCommand(CMD_PROVIDELOCALINFORMATION,0x01,DEV_ME);

				result = SendProactiveCommand();

				if(result == CMD_Success)
				{
					result = FindTag(&RecData[0], TLV_IMEI, P3);

					if((result != FALSE)&&(RecData[result]==LEN_IMEI))
					{
						ecpMem_Copy(&G_IMEI_FOR_NOTIFICATION[0],&RecData[result],9);
					}
				}
			}
			
			notificationProtocol = IF_GetNotificationProtocol();
			
			if((notificationProtocol == NOTIFICATION_ONLY_SMS)||(notificationProtocol == NOTIFICATION_SMS_HTTP))
			{
				if(SendNotificationBySMS() == FALSE)
				{
					SendNotificationByHTTPS();
				}
			}
			else
			{
				if(SendNotificationByHTTPS() == FALSE)
				{
					SendNotificationBySMS();
				}
			}					
		}
		else 
		{
			
		}
		
		return;
	}
	//LocationStatus_Limited_Service怎么处理？ 当做和LocationStatus_NO_Service一样处理吗？ 暂时没有实现。
// 	else if(locationStatus == LocationStatus_Limited_Service)
// 	{
// 		return;
// 	}
	else
	{
		return;
	}
}
/*
*function    : GetSMSRTpda
*
*@description:  获取SMSR的tpda
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*/
BYTE  GetSMSRTpda(BYTE * outBuffer)
{
	UWORD xdata paraAddr;
	BYTE  idata buf[3],offset;
	BYTE  xdata aid[0x10],temp[9];

	paraAddr = GetAddrByIndexAddr(M2MSMSRAddressPara_A3Ref_JC);

	//没有配置SMSR parameter时，返回错误 ,by xwh 20191009
	if(paraAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}

	paraAddr =  get_array_data((PF8)paraAddr);

	VMReadBuf(buf, paraAddr,3);

	if(buf[1] != 0x81)
	{
		//return FALSE;
		outBuffer[0] = 0x05;
		outBuffer[1] = 0x85;
		outBuffer[2] = 0x02;
		outBuffer[3] = 0x82;
		outBuffer[4] = 0xF2;
		return 0x05;
	}

	VMReadBuf(outBuffer, paraAddr+3,buf[2]);	

	if(buf[0] == (buf[2]+2))         //only tag 81, no tag A2
	{		
		return buf[2];
	}
	else                                     //have A2 tag
	{
		GetProfileAIDByProfileIndex(JcvmConfig_GetProfileEnableIndex(),aid);
	
		paraAddr += (3+buf[2]); //point to first A2
			
		for(offset=0; (offset+9)<(buf[0]-2-buf[2]);)
		{
			VMReadBuf(temp, paraAddr+offset,9);		

			if(ecpMem_Compare(&aid[12],&temp[4],3) == EQUAL)   //match ISD-P aid
			{
				VMReadBuf(outBuffer, paraAddr+offset+9,temp[8]);
				return temp[8];
			}

			offset += (temp[1]+2);
		}

		return buf[2];
	}
}




/*
*function    : GetIsdpSmsc
*
*@description:  获取isdp的 smsc
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by jiangqb@20200422
*
*/
void GetIsdpSmsc(BYTE * smscBuffer)
{
	UWORD xdata paraAddr;
	BYTE  idata buf[6],offset,i,len;

	smscBuffer[0] = 0xFF;

	paraAddr = GetProfileConnectivityParaAddr(JcvmConfig_GetProfileEnableIndex());//GetAddrByIndexAddr(PROFILE_METADATAREF + VMReadByte(PROFILE_ENABLEINDEX_CFG) * sizeof(WORD));
	
	if(paraAddr != ADDR_IS_NULL)			//isdp没有配置SMSC时，是否为空？
	{	
		len = VMReadByte(paraAddr);

		if(len != 0)
		{
			offset = 0;

			paraAddr ++;

			for(i=0;(i<3)&&(offset<len);i++)
			{
				VMReadBuf(buf, paraAddr+offset,2);

				if(buf[0] == 0xA0)
				{
					smscBuffer[0] = VMReadByte(paraAddr+offset+3);                        //SMSC len
					VMReadBuf(&smscBuffer[1], paraAddr+offset+4,smscBuffer[0]);           //SMSC

					return;
				}
				else
				{
					offset += (buf[1]+2);
				}
			}
		}
	}

}



/*
*function    : GetIsdpPidDcs
*
*@description:  获取isdp的PID/DCS
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，根据实际长度读取pid/dcs
*/
void GetIsdpPidDcs(BYTE piddcs[2])
{
	UWORD xdata paraAddr;
	BYTE  idata buf[6],offset,i,len,smscLen;
	BYTE  idata recLgth;
	
	piddcs[0] = 0x00;
	piddcs[1] = 0x04;
	
	paraAddr = GetProfileConnectivityParaAddr(JcvmConfig_GetProfileEnableIndex());//GetAddrByIndexAddr(PROFILE_METADATAREF + VMReadByte(PROFILE_ENABLEINDEX_CFG) * sizeof(WORD));
	
	if(paraAddr == ADDR_IS_NULL)
	{	
		return;
	}
	
	len = VMReadByte(paraAddr);

	offset = 0;

	paraAddr ++;

	for(i=0;(i<3)&&(offset<len);i++)
	{
		VMReadBuf(buf, paraAddr+offset,2);									//A0

		if(buf[0] == 0xA0)
		{
			smscLen = VMReadByte(paraAddr+offset+3);                        //SMSC len
			
			recLgth = buf[1];      											//A0 的值的长度
			
			ecpMem_Set(buf,0x00,sizeof(buf));
			
			recLgth -= (2+smscLen);											//a0值的长度减去 smsc的tlv长度， 即剩余为81, 82 tag的TLV长度，参考<SGP02.pdf> 4.1.3.4章节
			
			if (recLgth > 0)									//81， 82  存在
			{
				VMReadBuf(&buf[0], paraAddr+offset+4+smscLen,recLgth);			

				if(buf[0] == 0x81)                    //pid
				{
					piddcs[0] = buf[2];              
				}

				if(buf[0] == 0x82)                    //dcs
				{
					piddcs[1] = buf[2];
				}

				if(buf[3] == 0x81)                    //pid
				{
					piddcs[0] = buf[5];              
				}

				if(buf[3] == 0x82)                    //dcs
				{
					piddcs[1] = buf[5];
				}
			}

			return;
		}
		else
		{
			offset += (buf[1]+2);
		}
	}
}

/*
*function    : CalAESMac
*
*@description:  计算通知短信的AES CC
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，增加函数注释
*           :modify by xuweihua@20210310  AES算法key修改成4字节对齐（270改成272）
*/
BYTE CalAESMac(BYTE kid,BYTE *macBuf,BYTE * buffer,WORD length)
{
	BYTE  idata buf[3],j;
	BYTE xdata cpKey[0x20];	
	#define TempBuf_Offset_4Bytes_Allign 272
	
	ecpMem_Copy(buf,G_TARRecIndex,3);
	
	G_TARRecIndex[0] =  TarType_ISDR;
	G_TARRecIndex[2] = ISDR_APPID;
	
	j = GetKicKidKikKey(kid,&cpKey[0],TYPE_KEY_KID,0);  

	ecpMem_Copy(G_TARRecIndex,buf,3);
	
	if(CheckAesKeyLength(j) == FALSE)
	{
		return FALSE;
	}
	
	KeyExpansion(cpKey,&tempBuf[TempBuf_Offset_4Bytes_Allign]);   

	ToolkitForAESCalculateMac((UWORD)buffer,8,length,2,&tempBuf[TempBuf_Offset_4Bytes_Allign],macBuf);
	
	return TRUE;
}


/*
*function    : BuildNotificationBySMS
*
*@description:  拼装通知短信
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，增加函数注释
             :V1.2 modify by CT 20220627 修改M2M版本发送通知不用固定的KICKID，而是查找ISDR的第一组SCP80的key
*/
BYTE BuildNotificationBySMS(void)
{
	BYTE idata offset,CPLoff1,CPLoff2,udloff,notificationlen,ccoffset,keyversion;
	BYTE xdata buf[0x0E],cc[0x10];
	BYTE  idata cclength,piddcs[2];
	BYTE  idata notificationType,Kic;
	
	offset = 0;	
	
	InitProactiveCommand(CMD_SENDSHORTMESSAGE,0x00,DEV_NETWORD);
	
	buf[0] = TLV_ADDRESS;
	buf[1] = 0xFF; 
			
	GetIsdpSmsc(&buf[1]);
	
	GetIsdpPidDcs(piddcs);	
	
	AddTLV(&ResData[1],&buf[0]); 
	
	RecData[0] = 0x8B;
	RecData[1] = 0x00;
	offset = 0x02;	

	RecData[offset++] = 0x41;                                   //TP_MTI-TP_SRR
	RecData[offset++] = 0x00;	                                //TP_MR

	cclength = GetSMSRTpda(&RecData[offset]);
	offset += cclength;

	RecData[offset++] = piddcs[0];         //pid
	RecData[offset++] = piddcs[1];         //dcs

	udloff = offset;                                //UDL

	offset++;
	
	RecData[offset++] = 0x02;
	RecData[offset++] = 0x70;
	RecData[offset++] = 0x00;  		
  	
	CPLoff1 = offset;
	CPLoff2 = offset +1;
	offset += 2;
	
	RecData[offset++] = 13+8;  //CHL	
	
	RecData[offset++] = 0x02;   //SPI1  
	RecData[offset++] = 0x00;   //SPI2

	//RecData[offset++] = 0x12;   //KIC
	//RecData[offset++] = 0x12;   //KID  

  keyversion = IF_GetFirstSCP80AESkeyByAppID(ISDR_APPID,&tempBuf[200]);
  Kic = (((keyversion<<4) & 0xF0) | 0x02);
  RecData[offset++] = Kic; //KIC
	RecData[offset++] = Kic; //KID  

	RecData[offset++] = 0x00;   //TAR        
	RecData[offset++] = 0x00;          
	RecData[offset++] = 0x01;    

	RecData[offset++] = 0x00;    //CNTR
	RecData[offset++] = 0x00;
	RecData[offset++] = 0x00;
	RecData[offset++] = 0x00;
	RecData[offset++] = 0x00;
	
	RecData[offset++] = 0x00;//PCNTR	

	ccoffset = offset;
	ecpMem_Set(&RecData[offset],0x00,8);              //MAC
	offset += 8;

	notificationType = IF_GetNotifitionType();
	
	notificationlen = GenerateNotificationMessage((notificationType==NOTIFICATION_FIRST_TIME)?NOTIFICATION_FIRST_NETWORD_ATTACHMENT:notificationType,G_IMEI_FOR_NOTIFICATION,&RecData[offset]);
	offset += notificationlen;			
	
	RecData[1] = offset-2;	
	
	RecData[CPLoff1] = ((notificationlen+ 22 )>>8)&0xFF;	     //CPL high  CHL+E1
	RecData[CPLoff2] = (notificationlen+ 22 )&0xFF;           //CPL low
	
	RecData[udloff] = RecData[CPLoff2] + 5;//027000	
	
	//SMS shall make use of a CC with a length of 64 bits using AES CMAC mode
	CalAESMac(Kic,&cc[0],&RecData[CPLoff1],RecData[CPLoff2]+2);	
	
	ecpMem_Copy(&RecData[ccoffset],cc,8);
	
	AddTLV(&ResData[1], (BYTE  *)&RecData[0]);	
	
#if UICC__CDMA__SMS__DOWNLOAD
	TransSMSFomat(HAVE_SMSC);
#endif
			
	return TRUE;
}
/*
*function    : sendRefreshComm
*
*@description:  发送Refresh主动式命令
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，增加函数注释
*/
void sendRefreshComm(void)
{
	UWORD xdata fileAddr;
	BYTE xdata tmp[4];
	BYTE idata refreshMode,i,result;
	WORD idata alertSize;

	if((G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)||(G_BIPModuleStatus.channelDataType == ChannelDataType_CATTP)) // 正在进行HTTP会话的时候，需要发送refresh，等到HTTP链接关闭再发送
	{
		G_RefreshAfterCloseBIP = TRUE;
		return;
	}
		
	refreshMode = 0x04; 	//send refresh proactive command,mode = uicc reset
	
	//update 6F7E 设置为上网不成功
	fileAddr = SelectTFFile(DF_GSM,EF_LOCI);
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	tmp[2] = 0xFF;
	tmp[3] = 0x01;
	WriteTFBuf(fileAddr, 7, tmp, 4);

	//发送refresh 重启后，本次开机过程中的标记清除掉不再处理
	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_STKAlert);  	//注意,STK 等私有文件一定要放在文件系统的第一个目录，因为java卡创建过程中，可能涉及到EEPROM的移动，而导致执行标记的过程中，标记被移动.

	if(fileAddr != FALSE)
	{
		alertSize = GetTFSize(fileAddr);
		SetTFConst(fileAddr,0x00,0xFF,alertSize);
	}
		
	if(G_TerminalProfile[0] > 33)
	{
		if((G_TerminalProfile[34] & 0x20) != 0)
		{
			refreshMode = 0x09;
		}
	}

	//主动式命令结果为2X时重发3次
	//for(i=0;i<3;i++)
	//{
		InitProactiveCommand(CMD_REFRESH,refreshMode,DEV_ME);
		result = SendProactiveCommand();

		if((result&0x20)==0x20) //ts 102223  8.12.0 : Results '2X' indicate to the UICC that it may be worth re-trying the command at a later opportunity:
		{
			G_STATUSNUM_FOR_REFRESH = (refreshMode<<4)|0x02;
		}
	//}
}
/*
*function    : startNotificationTimer
*
*@description:  发送Timer Manager主动式命令
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，增加函数注释
*/
void startNotificationTimer(BYTE notiTytpe)
{
	BYTE xdata buf[3];
	UWORD xdata fileAddr;

	G_NotificationTimerId = TimerModuleAllocTimerId();

	if(G_NotificationTimerId == FALSE)
	{
		return;
	}

	//Timer默认设置成000100(16分钟)
	tempBuf[0x00] = 0x03;
	tempBuf[0x01] = 0x00;
	tempBuf[0x02] = 0x01;
	tempBuf[0x03] = 0x00;

	fileAddr = SelectPrivateTFFile(DF_Timer, EF_Timer_SMSSendNotice);

	if (fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr, 2, buf, 3);

		tempBuf[0x01] = buf[0];
		tempBuf[0x02] = buf[1];
		tempBuf[0x03] = buf[2];
	}

	TimerManagementProactive(0x00,G_NotificationTimerId&0x0F,tempBuf);

	G_NotificationTimerId |= notiTytpe;

	return;
}
/*
*function    : ProcessResendNotification
*
*@description:  Timer事件发生时，重发通知
				
*
*@param      :void

*
*@return     :void
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，增加函数注释
*/
void IF_ProcessResendNotification(BYTE timerExpirationId)
{
	BYTE idata notiType;

	if (TimerModuleTimerIdInValid(timerExpirationId))
	{
		return;
	}
	
	if(timerExpirationId == (G_NotificationTimerId&0x0F)) 
	{
		notiType = G_NotificationTimerId&0x80;
		
		if(G_NotificationTimerId != 0x00)
		{
			G_NotificationTimerId &= 0x0F;
			TimerModuleReaseTimerId(&G_NotificationTimerId);
		}

		//通知确认已收到，不需要再重发通知
		if(IF_GetNotifitionType() == NOTIFICATION_NON)
		{
			return;
		}

		if(notiType == NOTIFICATION_TYPE_SMS)
		{
			if(SendNotificationBySMS() == FALSE)
			{
				SendNotificationByHTTPS();
			}
		}
		else
		{
			if(SendNotificationByHTTPS() == FALSE)
			{
				SendNotificationBySMS();
			}
		}
	}
}
/*
*@description: 将一串十六进制的数转成acsii值的字符串
*
*@param      :hexBuf :       			  十进制数据地址
*@param      :acsiiBuf :            十六进制数存放的地址
*@param      :dataLgth :          	 十进制数据的长度
*    
*return			asc的字符串的长度
*
*@history    :V1.0 :create by duan ss@20170901
*/
//BYTE HexStrToAscStr(BYTE * hexBuf, BYTE * acsiiBuf, BYTE dataLgth) //EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
//{
//	BYTE	idata i,j;
//	BYTE	idata high,low;
//	
//	j = 0;
//	
//	for (i=0; i<dataLgth; i++)
//	{
//		high = (hexBuf[i] & 0xF0) >> 4;
//		
//		low = (hexBuf[i] & 0x0F);
//		
//		acsiiBuf[j++] = DecToAsc(high);
//	
//		acsiiBuf[j++] = DecToAsc(low);
//	}
//	
//	return j;
//}
/*
*function    : IF_StartTimerForHttpsNotification
*
*@description:  HTTP中发送Timer
				
*
*@param      :void

*
*@return     :void
*
*@history    :V1.0 by xwh@20190417
*
*            :V1.1 modify by xwh 20191009，增加函数注释
*/
BYTE IF_StartTimerForHttpsNotification(void)
{
	if(G_IsSendHttpsNotification != TRUE)
	{
		return FALSE;
	}

	G_IsSendHttpsNotification = FALSE;

	startNotificationTimer(NOTIFICATION_TYPE_HTTP);	
	
	return TRUE;
}

/*判断当前数据域指示的AID是否可被当前SD操作    ISDP只能被ISDR操作,为了不让对ISDP的操作分发到普通的SD 在这里将其拦截*/
BYTE checkDataFieleExercisable(void)
{
	if((G_TARRecIndex[0] != TarType_ISDR)/*&&(G_TARRecIndex[0] != TarType_ISDP)*/) //拦截普通SD/ISD对profile进行操作
	{
		if((RecData[0] == 0x4F)&&(RecData[1] == 0x10))
		{
			if((GetProfileIndexByAID(&RecData[2],0x10) != ILLEGAL_PROFILEINDEX)||(GetDownloadingProfileIndexByAID(&RecData[2],0x10) != ILLEGAL_PROFILEINDEX))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}
#endif
