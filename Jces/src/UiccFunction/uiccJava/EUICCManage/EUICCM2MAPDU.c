/************************************************************************************************
Copyright (C), 1988-2003, Eastcompeace. Co., Ltd.

File name   : EUICCM2MAPDU.c

Author:     : xyz  

Version:    : V1.0

Description :  该文件主要存放M2M版本下 ES5 ES6 ES8接口的函数接口实现 

************************************************************************************************/ 
#include "EUICCM2MAPDU.h"
#include "EUICCM2MAPDUPublic.h"
#include "EUICCProfile.h"
#include "interpreter.h" 
#include "ASN1Operation.h"
#include "EUICCCustomerCertificate.h"
#include "EUICCGPDataOpertion.h"

#include "SCP03Realize.h"
#include "platform_api.h"
#include "cardManager.h"
#include "uicc_main.h"
#include "cardManager.h"
#include "T0ApduImpl.h"
#include "SEEUICCInterface.h"
#include "target.h"
#include "opFunction.h"
#include "EUICCInterface.h"
#include "ob_manager.h"
#include "dataManager.h"
#include "SecurityDomainData.h"
#include "nativemethod_impl.h" //warning SetRemoteProcessRes
#include "ProfileGPCommand.h"  //warning CreateProfileEntry
#include "VMAccessEeprom.h"    //warning VMReadUword
#include "Interface_LTEFromPlatform.h"  //warning IF_SaveSCP80KeyValueBeforeUpdate
#include "euiccConfig.h" //warning JcvmConfig_GetCardState
#include "profileApi.h"

#if M2M_EUICC
BYTE xdata G_M2MKeySetState;
BYTE xdata G_M2MStoreData[2]; //byte 1 表示执行过3A01 byte2 表示profileIndex

/*
*@function    : M2M_DeleteCommand_ProfileDelete
*
*@description :  This function deletes the ISD-P and its associated Profile. 
*
*@see         :4.1.1.4  Profile Deletion
*
*@param       : BYTE deleteProfileindex
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0 :create @20181227
*/

void M2M_DeleteCommand_ProfileDelete(BYTE deleteProfileindex)
{
	BYTE idata pol;
	
	pol = GetProfilePOL1(deleteProfileindex);
	
	if((pol&POL1_DELETED_NOT_ALLOWED) == POL1_DELETED_NOT_ALLOWED) // Verify that POL1 of the target Profile allows its deletion.
	{
		ThrowError(EUICC_PROFILE_PREVENT_ACTION);
	}	
	
	if(deleteProfileindex == JcvmConfig_GetM2MFallBackDisabledNoAllowProfileIndex())//if(deleteProfileindex == VMReadByte(M2MFallBackDisabledNoAllowProfileIndex))  //his includes, if the target Profile has been Disabled as the result of Fall-back activation procedure described in section 3.16, verify that POL1 of the target Profile allows Disabling
	{
		ThrowError(EUICC_PROFILE_PREVENT_ACTION);
	}
	
	if(JcvmConfig_GetM2MFallBackProfileIndex()  == deleteProfileindex) //Verify that the target Profile is not the Profile with Fall-back Attribute set
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	if(GetProfileState(deleteProfileindex) == PROFILE_STATE_ENABLED) //Verify that the target Profile is not in the Enabled state
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED); 
	}
	
	DeleteProfileByProfileIndex(deleteProfileindex);
	
	RecData[0] = 0;
		
	SetRemoteProcessRes(0x05, 1);//ThrowError(0x6101);
	
}
/*
*@function    : M2M_DeleteCommand_MasterDelete
*
*@description :  This function deletes a target Profile on the target eUICC regardless of POL1 rules. This function shall use the ISD-P token verification key(AES key with key version number ‘70’ and key identifier ‘01’) in order to authenticate the source of the command
*
*@see         :4.1.1.6  Master Delete  
*             :3.10  Master Delete Procedure
*
*@param       : BYTE deleteProfileindex
*
*@return      :void
*
*@author      :xuyz
*
*@history     :v1.0 create @20181227
*/

void M2M_DeleteCommand_MasterDelete(BYTE deleteProfileindex)
{
	BYTE idata b6Len,lenLen ;
	WORD idata b6DataOffset;
	BYTE idata offset;

	if(GetProfileState(deleteProfileindex) == PROFILE_STATE_ENABLED) //Verify that the target Profile is not in the Enabled state
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED); 
	}
		
	if(JcvmConfig_GetM2MFallBackProfileIndex() == deleteProfileindex) //Verify that the target Profile is not the Profile with Fall-back Attribute set
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
		
	offset = 0x12;  //point to 4F 后面的那个tag
	
	if(RecData[offset] != 0xB6)   //b6 tag强制存在
	{
		ThrowError(SW_WRONG_DATA);  
	}
	
	b6Len = RecData[offset +1]; 
	
	if(b6Len < 0x80) //B6的长度为1个字节
	{
		lenLen = 1;  //长度占了一个字节
	}
	else   //B6长度为2个字节，目前卡片最多只支持2个字节
	{
		lenLen = 2;   //长度本身占了两个字节
		
		b6Len = RecData[offset +2]; 
	}
	
	b6DataOffset = offset+1+lenLen;
	
	MasterDelete_CheckB6Tag(&RecData[b6DataOffset] , b6Len);    //检查B6 tag的合法性
	
	offset += (b6Len + lenLen +1);  //point to the next 9E tag
	
	if(RecData[offset] != 0x9E)  //9e tag不存在
	{
		ThrowError(SW_WRONG_DATA); 
	}

	G_ApduBuffer[4]	= offset;
	
	if(MasterDelete_VerifyDeleteToken(deleteProfileindex, offset +3, &G_ApduBuffer[2],offset+1,&RecData[b6DataOffset] , b6Len) == FALSE)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	G_ApduBuffer[4]	= P3;	
	
	
	DeleteProfileByProfileIndex(deleteProfileindex);
	
	RecData[0] = 0;
		
	SetRemoteProcessRes(0x05, 1);//ThrowError(0x6101);
}
#if M2M_ESx
/*
*@function   :M2M_StoreData_Set_Emergency_Profile_Attribute
*
*@description:This optional function sets the Emergency Profile Attribute for one Profile on the target eUICC.
*
*@see        :4.1.1.13 Set Emergency Profile Attribute
*
*@param      :void
*
*@return     :void
*
*@author     :chentao
*
*@history    :created by chentao@20220929
*/
void M2M_StoreData_Set_Emergency_Profile_Attribute()
{
	BYTE idata targetProfileIndex;

	if((P1 != 0x88)||(P2 != 0x00))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}

	if(P3 != (RecData[2] + 3))
	{
		ThrowError(SW_WRONG_DATA);
	}

	if(RecData[3] == 0x4F)		// tag AID
	{
		if((RecData[4] + 2) != RecData[2])
		{
			ThrowError(SW_WRONG_DATA);
		}

		if(AIDLenInvalid(RecData[4]))
		{
			ThrowError(SW_WRONG_DATA);
		}

		/*if(GetDownloadingProfileIndexByAID(&RecData[5], RecData[4]) != ILLEGAL_PROFILEINDEX)//恒宝离线脚本测试 Tele_eUICC_Management_Command_normal_test_01 激活一个状态为seletable或者personal状态的Profile期望返6985
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}*/

		targetProfileIndex = GetProfileIndexByAID(&RecData[5], RecData[4]);

		if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
		{
			ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
		}

		//The target profile SHALL NOT be enabled
		if(GetProfileState(targetProfileIndex) == PROFILE_STATE_ENABLED) //Verify that the target Profile is in Enabled state
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}
		//The target Profile SHALL NOT have the Fall-Back Attribute set.
		if(JcvmConfig_GetM2MFallBackProfileIndex() == targetProfileIndex)
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}

		JcvmConfig_SetM2MEmergencyProfileIndex(targetProfileIndex);

		return;
	}

	ThrowError(SW_WRONG_DATA);
}
#endif
/*
*@function   :M2M_DELETE_FinaliseISDRhandover
*
*@description:This function deletes all keys in the ISD-R except for the key ranges indicated by the command parameter(s). It is intended as a simple clean-up mechanism for the new SM-SR after takeover to get rid of all keys of the previous SM-SR in the ISD-R
*
*@see        :4.1.1.9  Finalisation of the ISD-R Handover  F203V F203V
*             3.8  SM-SR Change 
*             2.2.5.1  Secure Channel on ES5 (SM-SR-eUICC)
*
*@param      :void
*
*@return     :void
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181226
			  modify by lx @20190428  kidFirst和kidLast只需要判断kidFirst <= kidLast，否则非法；被保留的key除了SCP80/SCP81以外的其他key version都要求抛6985    测试用例:《TC_ES5_FIH_1_FinaliseISDRhandover_SMS_03_Error_Case_Delete_All_SCP80_Keys_17_00_53》
*/
void M2M_DeleteCommand_FinaliseISDRhandover(void)
{
	BYTE idata recOffset, scp80Count, scp81Count,i;
	BYTE idata keyVersion[2], kidFirst, kidLast, result;
	
	if((P1 != 0) || (P2 != 0))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if((P3 != 0x05)&&(P3 != 0x0A))// 最多可以保留2组密钥 The Delete [card content] Data Field shall contain one or two instances of following TLV:
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	recOffset = 0x00;
	
	scp80Count = 0;
	
	scp81Count = 0;
	
	result = FALSE;
	
	keyVersion[0] = 0xFF;
	keyVersion[1] = 0xFF;

	i = 0;
	
	for(recOffset = 0;recOffset < P3;recOffset = (recOffset +5))
	{		
		if((RecData[recOffset] == 0xF2)&&(RecData[recOffset+1] == 0x03))// Table 49: Delete [card content] Command Data Field， 长度必须是3
		{			
			keyVersion[i] = RecData[recOffset+2];         // GP221 PUT KEY Command: The Key Version Number is coded from '01' to '7F'. 
			
			kidFirst = RecData[recOffset+3];
			
			kidLast = RecData[recOffset+4];
			
			if((kidFirst > kidLast)/*||(kidLast > 3)*/)//KID的范围 01 02 03
			{
				ThrowError(SW_WRONG_DATA);
			}
			
			if(IsSCP80Key(keyVersion[i])) // SCP80 key
			{
				if((kidFirst != 0x01)||(kidLast != 0x03))//HB_Offline_Test/4管理指令异常数据测试/Tele_FinaliseISDRhandover_Errortest_01 测试要求如果KVN指定的是SCP80密钥 必须保留一组完整的scp80密钥
				{
					ThrowError(SW_WRONG_DATA);
				}
				
				scp80Count++;
			}				
			else if(IsSCP81Key(keyVersion[i])) // SCP81 key
			{
				scp81Count++;					
			}
			else  // 除了SCP80/SCP81以外的其他key version都要求抛异常, 测试用例有测试key version为0x80, 0x81要求抛异常的
			{
				ThrowError(SW_CONDITIONS_NOT_SATISFIED);
			}
			
			i++;

		}
		else
		{
			ThrowError(SW_WRONG_DATA);
		}
			
	}
	
	if((scp80Count > 1) || (scp81Count > 0x01))  //两个TLV时，只能一个是SCP80，一个是SCP81: Two TLVs allow for one SCP80 and one SCP81 key set to “survive” key clean-up.
	{
		ThrowError(SW_WRONG_DATA);
	}
	
 	result = IF_CheckSurviveKey(&keyVersion[0]); // Check that all keys of the key set(s) used for setting up the current secure channel are among the keys not to be deleted. For SCP81, this also includes the key set used for the push SM. If that check fails, the command is terminated without deleting any key.

	if(result == FALSE)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);   // ’69 85’: Key(s) of key set used for the current secure channel is/are among the keys to be deleted. 
	}
	
	FinaliseISDRhandover_DeleteKey(RecData, P3);  // todo: Delete all keys except those in the key ranges indicated in the command parameters.这里应该是一个事务，要么所有的key全部删除，要么一个都不删除		
	
	RecData[0] = 0x00;                            // The data field of the response message shall contain a single byte of ‘00’. 
		
	SetRemoteProcessRes(0x05, 1);//ThrowError(0x6101);
}

/*
*@fucntion   :M2M_DeleteCommand
*
*@description: 处理M2M版本的delte命令，主要是在ISD R RAM下操作
*
*@param      : void
*
*@return     :true:当前命令在本函数中已经执行了 /false:当前命令在本函数中没有执行
*
*@author     :xuyz
*
*@history    : V1.0 :Create @20190304
*/
BYTE M2M_DeleteCommand(void)
{	
	BYTE idata profileIndex;
	
	if(RecData[0] == 0x4F)
	{
		if (bRamBuffer[IS_LOAD_IN_PROGRESS] != 0)
		 //if(G_RAMCAPInstallTag == CAPCOMPONENT_InvalidTag) //GP2.2.1_UICC_ComplianceTests(移动版)-20141112/测试脚本/uicc_configuration_additional_test_工程集.uce/MultiChannelTest_SECURED/MultiChannelTest_12_1 -- TestPurpose: ISD的RAM Load流程不会被另一个逻辑通道上的ISD的非下载操作打断
		{
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)  // GP221  9.3.1  Card Content loading may be prohibited if a load process is already in progress on another logical channel.
			{
				ThrowError(SW_CONDITIONS_NOT_SATISFIED);  //测试用例：中国移动NFC2.0 ：多逻辑通道冲突.multiChannel_delete01.js,原则上不影响上一次操作即可
			}
		}	
		/*恒宝HB_Offline_Test /3Profile Download指令异常测试/Tele_profile_download_Error_test_01 ,异常测试时 下载Profile过程 下来的86 TL错误 ，此时Profile内部正在进行Cap包的下载、
			G_CapComponetTag值不为0，后续直接发指令删除该Profile 期望可以删掉  ，是否需要增加判断条件if((profileIndex != G_DownLoadingProfileIndex)&&(G_CapComponetTag != CAPCOMPONENT_InvalidTag))来限制这种交互操作？*/

		if((P1 != 0x00)||(P2 != 0x40))
		{
			ThrowError(SW_INCORRECT_P1P2);
		}
	
		if(P3 < 0x12)  //P3长度不够一个ISD-P AID的长度
		{
			ThrowError(SW_WRONG_LENGTH);
		}
		
		if(RecData[1] != 0x10)   //ISD-P  的AID 长度一定是 16字节
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		profileIndex = GetProfileIndexByAIDAndState(&RecData[2], RecData[1],PROFILE_VALIDFLAG);	// 		profileIndex = GetProfileIndexByAID(&RecData[2], RecData[1]);
		
		if(ProfileIndexIsValid(profileIndex) == FALSE)
		{
			profileIndex = GetProfileIndexByAIDAndState(&RecData[2], RecData[1],PROFILE_LOADINGFLAG);	
		}
		
		if(ProfileIndexIsValid(profileIndex) == FALSE)  //说明没有找到ISD-P 对应的AID ，表明当前不是删除profile操作
		{
			ThrowError(SW_REFERENCED_DATA_NOT_FOUND); // 测试用例： eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0/TC_ES5_DP_1_DeleteProfile_SMS_05_Error_Case_ISD_P_Not_Present_on_eUICC
		}
		
		if(P3 == 0x12)  //如何区分当前是profile delete 还是master delete操作，先根据P3 进行区分
		{
			M2M_DeleteCommand_ProfileDelete(profileIndex);
		}
		else
		{
			M2M_DeleteCommand_MasterDelete(profileIndex);
		}
		
	}
	else if(RecData[0] == 0xF2)
	{
		M2M_DeleteCommand_FinaliseISDRhandover();
	}
	else if((RecData[0] == 0xD0) || (RecData[0] == 0xD2))
	{
		return FALSE; //eUICC_M2M_additonal/补充测试脚本/PutPSK_SMS_Test_01是HTTP方式执行标准GP DELETE指令删除KEY
	}
	else
	{
		ThrowError(SW_WRONG_DATA); //测试用例：HB_Offline_Test/4管理指令异常数据测试/Tele_MasterDelete_Errortest_01  步骤26、远程方式向卡片发送命令，执行MasterDelete，4FTLV Tag error;
	}
	
	return TRUE;
	
}

/*
*@fucntuion  :M2M_CreateISDP
*
*@description:M2M模式下创建ISDP    AID参考 Annex H Coding of the PIX for ‘Embedded UICC Remote Provisioning and Management’ (Normative)
*
*@param      ：void
*
*@return     :true:当前命令在本函数中已经执行了 /false:当前命令在本函数中没有执行
*
*@author     :xuyz
*
*@history    : V1.0 :Create @20190304
*/

BYTE code ISDPExcuteFile_AID[0x10]   = {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x0D,0x00};  //SGP02_V3.2 2.2.3  Identification of Security Domains: AID and TAR
BYTE code ISDPExcuteModule_AID[0x10] = {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x0E,0x00};
BYTE code ISDPPreAID[] =               {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00};                //SGP02_V3.2 2.2.3  Identification of Security Domains: AID and TAR

BYTE M2M_CreateISDP(void)
{
	WORD idata dataOff, tmp;
	BYTE idata index;
	
	if(P2 != 0x00)//P2 is set to ‘00’; according to GlobalPlatform Card Specification [6] section 11.5, this means no information provided.
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if(RecData[0] != 0x10)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	dataOff = 1;
	
	if(ecpMem_Compare(&RecData[dataOff], (BYTE *)ISDPExcuteFile_AID, 0x10) != EQUAL)
	{
		ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
	}
	
	dataOff += 0x10;
	
	if(RecData[dataOff] != 0x10)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	dataOff ++; 
	
	if(ecpMem_Compare(&RecData[dataOff], (BYTE *)ISDPExcuteModule_AID, 0x10) != EQUAL)
	{
		ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
	}
	
	dataOff += 0x10;
	
	if(RecData[dataOff] != 0x10) // ISD P 的应用AID
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	dataOff ++; 
	
	if(ecpMem_Compare(&RecData[dataOff], (BYTE *)ISDPPreAID, 13) != EQUAL) //创建ISD-P时，AID 的前12字节要和ISDPPreAID字段一样
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	tmp = ((WORD)RecData[dataOff+13] << 8)|RecData[dataOff+14];
	
	if(tmp < 0x10)   //SGP02_V3.2  Annex H  Coding of the PIX for ‘Embedded UICC Remote Provisioning and Management’ (Normative)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(RecData[dataOff+15] != 0x00)  //ISDP AID 的最后一字节 AnnexH -- Not used: set to ‘00’
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	tmp = dataOff;   //记录ISD P AID 在RecData中的偏移
	
	index = GetProfileIndexByAID(&RecData[dataOff], 0x10); 
	
	if(index != ILLEGAL_PROFILEINDEX)  //profile ISDP AID 有重复
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	index = GetDownloadingProfileIndexByAID(&RecData[dataOff], 0x10); 
	
	if(index != ILLEGAL_PROFILEINDEX)  //profile ISDP AID 有重复
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	dataOff += 0x10;  //point to the length of  Length of Privileges  4.1.1.1 
	
	if(RecData[dataOff] != 0x03)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(((RecData[dataOff+1]&0x80) == 0x00)||((RecData[dataOff+2]&0xC0) != 0xC0)) // ISD-P Creation   Privileges granted to the ISD-P shall be at least:   Security Domain  Trusted Path  Authorized Management 
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	index = GetIdleProfileIndex();
	
	if(index == ILLEGAL_PROFILEINDEX)
	{
		ThrowError(SW_FILE_FULL);
	}

	bRamBuffer[LOADING_PROFILE_INDEX] = index;

	tmp = CreateProfileEntry();

	if(tmp != 0x9000)//创建ProfileEntry
	{
		ThrowError(tmp);
	}

	SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

	tmp = CUSTOMERGP_InstallforInstallISDP(ISDR_APPID);
	
	SEUICC_ResumeISDPAppletTable();

	if(tmp == 0x9000)
	{
		RecData[0] = 0x00;	
		
		SetRemoteProcessRes(0x05, 1);//ThrowError(0x6101);
	}
	else
	{
		ThrowError(tmp);  
	}
	
	return TRUE;
}

/*
*@function   :M2M_StoreData_HandleNotificationConfirmation
*
*@description:This function confirms the notification and triggers potential follow-up activities required by POL1.
*
*@see        :4.1.1.12  Notification Confirmation 3A08 04 4E02 XXXX
*
*@param      :void
*
*@return     :void
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181219
*/
void M2M_StoreData_HandleNotificationConfirmation (void)
{
	BYTE idata profileIndex, POL1, fallBackProfileIndex,transActioinFlag,deleteProfileFlag;
	UWORD xdata seqNumber;
	
	if((P1 != 0x89) || (P2 != 0)) 
	{
		ThrowError(SW_INCORRECT_P1P2);
	}

	if((P3 != 0x07) || (RecData[2] != 0x04) || (RecData[3] != 0x4E) || (RecData[4] != 0x02))
	{
		ThrowError(SW_WRONG_DATA);
	}
		
//	IF_M2M_DisableRetryMechanismForNotification();               // Disable the retry mechanism for the notification，封装成接口，由LTE层实现？ 
	
	seqNumber =  VMReadUword(EUICCNotificationSeqNumber);
		
	if((WORD)seqNumber != BuildWord(&RecData[5])) // 判断seqNumber是否正确，是否与上发的通知中的序列号一致
	{
		ThrowError(SW_WRONG_DATA);
	}
		
	if(seqNumber < 0xFFFF)  // When a Notification Confirmation has been successfully received by the SM-SR, the eUICC shall increment the sequence number for the next notification
	{
		seqNumber++;
	}
	else                   //设备版的seqNumber仅支持两个字节
	{
		seqNumber = 0;
	}
	
	transActioinFlag = IF_BeginTransaction();
		
	profileIndex = JcvmConfig_GetM2MLastDisableProfileIndex();  //找到前一个被disable的profile，根据其POL1判断是否需要删除
	
	RecData[0] = 0x80;   //此时RecData中的数据已经可以被清除了，这里开始拼装响应数据
	
	RecData[1] = 0x00;   // If no follow-up activity has been performed at all, the data field shall contain tag 80 followed by a length of zero, and no value
	
	deleteProfileFlag = FALSE;
	
	if((profileIndex == G_CurEnabledProfileIndex)||(profileIndex == ILLEGAL_PROFILEINDEX))//ROLL BACK以后收到的通知确认 或者 不是因为Profile切换产生的通知确认（某个profile突然连不上网进了FALLBACK机制）
	{				
		//	TODO NOTHING
	}
	else
	{
		POL1 = GetProfilePOL1(profileIndex);
	
		fallBackProfileIndex = JcvmConfig_GetM2MFallBackProfileIndex() ;
		
		if(((POL1&POL1_DELETED_MANDATORY) == POL1_DELETED_MANDATORY) && (profileIndex != fallBackProfileIndex)) // On  reception  of  the  SM-SR  notification  confirmation  command,  if  POL1  of  the  now Disabled Profile contains the rule “Profile deletion is mandatory when it is disabled”, and this Profile does not have the Fall-back Attribute set, the ISD-R shall delete the disabled ISD-P and the contained Profile.
		{
			RecData[1] = 0x12;
				
			RecData[2] = 0x4F;
			
			RecData[3] = GetProfileAIDByProfileIndex(profileIndex, &RecData[4]);  //先备份被删除ISD-P的AID
			
			deleteProfileFlag = TRUE;
			
			VMSetBuf(PROFILE_VALIDTAG_CFG + profileIndex, PROFILE_DELETELAG, 1);   
					
		}	
	}	
	
	VMWriteUword(EUICCNotificationSeqNumber, seqNumber); 
	
	JcvmConfig_SetM2MNotificationType(NOTIFICATION_NON);
	
	JcvmConfig_SetM2MLastDisableProfileIndex(ILLEGAL_PROFILEINDEX);
	
	IF_M2M_NotificationConfirmation();
	
	IF_CommitTransaction(transActioinFlag);
	
	if(deleteProfileFlag == TRUE)
	{
		DeleteProfileDataByProfileIndex(profileIndex, MODE_DELETE_PROFILE);
		
//	打垃圾回收标记 ？		GarbageCollectorAndHeapPieceCleanUp();

	}
	
	SetRemoteProcessRes(0x05, RecData[1] + 2);//ThrowError(0x6100 + RecData[1] + 2);	//On reception of the SM-SR notification confirmation, the eUICC may perform any operation as  specified  in  one  of  the  procedures  including  the  notification  sequence  (like  for  instance deletion of an ISD-P after its disabling, see section 3.6).
}

/*
*@function   :M2M_StoreConnectivityParameters
*
*@description: 将连接参数更新到指定profile中的medata区域
*
*@param      :BYTE profileIndex
*@param      :BYTE *dataBuf :  V结构的 ConnectivityParameters, 格式为L TLV TLV TLV
*@param      :BYTE dataLen  :ConnectivityParameters的数据总长度
*
*@return      :void
*
*@author     :xuyz
*
*@history     :V1.0 :create @2019028
*/
void M2M_StoreConnectivityParameters(BYTE profileIndex, BYTE *dataBuf, BYTE dataLen )
{
	UWORD idata paraAddr;

	paraAddr = GetProfileConnectivityParaAddr(profileIndex);//GetAddrAndIndexByIndexAddr(PROFILE_METADATAREF+ profileIndex * sizeof(WORD), &paraRef);
	ecpMem_Move(&tempBuf[1],dataBuf,dataLen);
	tempBuf[0] = dataLen;
	VMWriteBuf(paraAddr, tempBuf,  dataLen + 1);
}
/*
*@function   :M2M_StoreData_UpdateConnectivityParameters
*
*@description:This function is used to update Connectivity Parameters on the eUICC。 该连接参数只是用来给通知使用的
*
*@see        :4.1.3.4  Connectivity Parameters Update using SCP03  3A07 L A0LV A1LV A2LV, 由于规范中规定P1只能为0x88，所以数据域不会出现超过RecData长度的情况
*
*@param      :void
*
*@ram        :借用tempBuf[0..n]用于临时缓存经过筛选之后的Connectivity Parameters,存储格式为L TLV TLV TLV
*
*@return     :void
*
*@test       :8020000009 3A0706A00406020102
*
*@history    :
*注意        :这个连接参数是否有默认值？默认值如何存储进来的？该连接参数是，每一个ISD-P都有一个
*/
void M2M_StoreData_UpdateConnectivityParameters(BYTE profileIndex)
{
	BYTE idata subLen;
	BYTE idata tagExist, subTagExist,subTotalLen;
	BYTE idata tempBufOffset, tempBufOffsetBak, subOffset, recOffset;
	
	if((P1 != 0x88) || (P2 != 0x00)) 
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if(P3 != (RecData[2] + 3))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
// 	profileIndex = IF_GetRAMProfileIndex(); //根据TAR获取到需要被更新的ISD-P参数的profileIndex
	
	if(ProfileIndexIsValid(profileindex) == FALSE)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	recOffset = 0x03;  // 3A07 L A0LV A1LV A2LV， 偏移到第一个TAG区域
	
	tagExist = 0;      // bit8--bit0: bit0表示sms的存在；bit1表示http的存在；bit2表示cattp的存在
	
	subTagExist = 0;
	
	subTotalLen =0;
	
	tempBufOffset = 1;
	
	while(recOffset < P3)
	{
		subLen = RecData[recOffset+1];				
		
		switch(RecData[recOffset])  
		{			
			case 0xA0:  // Connectivity parameters for SMS protocol , A0 L 06LV 81LV 82LV， 这三个子TAG的顺序有没有要求？
			{
				if((tagExist & 0x01) == 0x00) // A0 TAG第1次出现
				{
					tagExist |= 0x01;
								
					subOffset = 0;
					
					subTagExist = 0;
					
					while(subOffset < subLen)
					{
						switch(RecData[recOffset + 2 + subOffset])  // A0下的子TAG
						{
							case 0x06:
							{
								if((subTagExist & 0x01) == 0x01)    // 不允许重复出现
								{
									ThrowError(SW_WRONG_DATA);
								}
								
								subTagExist |= 0x01;
								
								if(RecData[recOffset + 2 + subOffset + 1] == 0)    // 06 TAG的长度不能为0, 会不会出现06, 81, 82重复出现的情况？暂时处理成不能重复出现
								{
									ThrowError(SW_WRONG_DATA);
								}								
								
								break;
							}
							case 0x81:
							{
								if((subTagExist & 0x02) == 0x02)    // 不允许重复出现
								{
									ThrowError(SW_WRONG_DATA);
								}
								
								subTagExist |= 0x02;
								
								if(RecData[recOffset + 2 + subOffset + 1] != 0x01)  // 81 TAG的长度必须是1
								{
									ThrowError(SW_WRONG_DATA);
								}
								
								break;
							}
							case 0x82:                                              // 82 TAG的长度必须是1
							{
								if((subTagExist & 0x04) == 0x04)    // 不允许重复出现
								{
									ThrowError(SW_WRONG_DATA);
								}
								
								subTagExist |= 0x04;
								
								if(RecData[recOffset + 2 + subOffset + 1] != 0x01)
								{
									ThrowError(SW_WRONG_DATA);
								}
								
								break;
							}
							default:   // 如果包含了其他的TAG，是应该忽略还是应该抛错？
								ThrowError(SW_WRONG_DATA);
						}
						
						subOffset += (RecData[recOffset + 2 + subOffset + 1] + 2);  // 偏移到下一个子TAG
					}
					
					if((subTagExist & 0x01) == 0x00)   // 06 TAG没有出现
					{
						ThrowError(SW_WRONG_DATA);
					}
					
					ecpMem_Copy((BYTE*)&tempBuf[tempBufOffset], &RecData[recOffset], subLen + 2);                       // 将A0 TAG数据拷贝到tempBuf[1..n]
					
					tempBufOffsetBak = tempBufOffset;
					
					tempBufOffset += (subLen + 2);
					
					if((subTagExist & 0x02) == 0x00)  // 81 TAG没有出现， If no PID TLV is present, the eUICC shall use a default value of PID=00.
					{
						tempBuf[tempBufOffset++] = 0x81; 
						
						tempBuf[tempBufOffset++] = 0x01; 
						
						tempBuf[tempBufOffset++] = 0x00; 
						
						tempBuf[tempBufOffsetBak+1] += 3;  //修改A0 TAG的长度
					}
					
					if((subTagExist & 0x04) == 0x00)  // 82 TAG没有出现， If no DCS TLV is present, the eUICC shall use a default value of DCS=04. 
					{
						tempBuf[tempBufOffset++] = 0x82; 
						
						tempBuf[tempBufOffset++] = 0x01; 
						
						tempBuf[tempBufOffset++] = 0x04; 
						
						tempBuf[tempBufOffsetBak+1] += 3;  //修改A0 TAG的长度
					}
				}
				
				break;
			}
			case 0xA1:  // Connectivity parameters for HTTP protocol
			{
				if((tagExist & 0x02) == 0x00)   // 0xA1 TAG第一次出现， 是否需要检查A1 TAG内容的合法性？
				{
					tagExist |= 0x02;
					
					if(subLen == 0x00)
					{
						ThrowError(SW_WRONG_DATA);
					}
					
					ecpMem_Copy((BYTE*)&tempBuf[tempBufOffset], &RecData[recOffset], subLen + 2);
					
					tempBufOffset += (subLen + 2);
				}
				
				break;
			}
			case 0xA2:  // Connectivity parameters for CAT_TP protocol
			{
				if((tagExist & 0x04) == 0x00)
				{
					tagExist |= 0x04;
					
					if(subLen == 0x00)
					{
						ThrowError(SW_WRONG_DATA);
					}
					
					ecpMem_Copy((BYTE*)&tempBuf[tempBufOffset], &RecData[recOffset], subLen + 2);
					
					tempBufOffset += (subLen + 2);	
				}
				
				break;
			}
			default:
				ThrowError(SW_WRONG_DATA);
		}
		
		subTotalLen += (subLen + 2);
		
		recOffset += (RecData[recOffset+1] + 2);  //偏移到下一个TAG
	}
	
	if(subTotalLen > RecData[2])//子tag的总长度大于3A07的长度 
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	tempBuf[0] = (tempBufOffset - 1);  //修改Connectivity Parameters的总长度
	
	M2M_StoreConnectivityParameters(profileIndex, &tempBuf[1], tempBuf[0]);
}
/*
*@function   :  M2M_StoreData_EstablishISDRKeySet1
*
*@description: This function is used to perform mutual authentication between the new SM-SR and the eUICC and to establish a shared secret key set between the new SM-SR and the ISD-R.
               OR:
			   This function is used to perform mutual authentication between the SM-DP and the eUICC and to establish a shared secret key set between the SM-DP and the ISD-P  
*
*@see          :4.1.3.1  ISD-P Key Set Establishment  OR 4.1.1.8  ISD-R Key Set Establishment   3A01 
*               Figure 23: SM-SR Change 
*
*@param       :BYTE keySetType: ISDR_KeySet/ISDP_KeySet
*
*@return      :  void
*
*@history     :
*/
void M2M_StoreData_EstablishISDRKeySet1(BYTE keySetType)
{
	WORD idata offset,totleLen ;

	
	if((P1 != 0x09)||(P2!= 0x00))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	offset = 2; //指向3A01长度
	
	totleLen = RecData[offset]  ; //暂时默认3A01的长度仅为1个字节，由于M2M 测试版3A01不是标准的ASN1 编码，测试中当3A01长度为BF时，只用了一个字节表示
	
	if((totleLen + 3) != P3) 
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	offset ++; //point to 7F21
	
	if(GPCertificate_VerifySMSRCert(&RecData[offset],  totleLen,keySetType) != TRUE)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	RecData[0] = TAG_RC;
	
	RecData[1] = RC_Len;
	
 	Random_number_generator(&RecData[2],RC_Len,ALG_SECURE_RANDOM);

	VMWriteBuf(EUICC_AuthenTmpBuf + TempSMSRCertRC_Off,&RecData[1],RecData[1] + 1);//LV结构写入
	
	G_M2MKeySetState = M2M_KeySet1|keySetType;
	
	G_M2MStoreData[0] = 1;//执行了建立密钥的第一条StoreData

	SetRemoteProcessRes(0x05, RC_Len + 2);//ThrowError(0x6100 + RC_Len + 2);
}
/*
*@function    :M2M_StoreData_EstablishISDRKeySet2
*
*@description :This function is used to perform mutual authentication between the new SM-SR and the eUICC and to establish a shared secret key set between the new SM-SR and the ISD-R.
               OR:
			   This function is used to perform mutual authentication between the SM-DP and the eUICC and to establish a shared secret key set between the SM-DP and the ISD-P  
*
*@see         :4.1.3.1  ISD-P Key Set Establishment  OR 4.1.1.8  ISD-R Key Set Establishment   3A02LV 7F49LV 5F37LV 
*              Figure 23: SM-SR Change 
*
*@param       ::BYTE keyType: ISDR_KeySet/ISDP_KeySet
*
*@return      :  void
*
*@history     :
*/
#define    keyUsageQualifierOff   0
#define    keyTypeOff             1
#define    keyLengthOff           2
#define    keyAccessOff           3
#define    keyIdentifierOff       4
#define    keyVersionNumberOff    5 
#define    scenarioParamOff       6

void M2M_StoreData_EstablishISDRKeySet2(BYTE keyType)
{
	WORD idata offset ;
	BYTE idata octerNum,signOffset,A6Offset,keyLen,appID;
	BYTE xdata keyCounter[9],keyInfo[7];  //keyInfo ---> Key Access  + Key Identifier + Key Version Number + scenarioParam
	UWORD xdata totleLen,shareInfoLen;
	if((G_M2MKeySetState&keyType) != keyType)
	{
		G_M2MKeySetState = FALSE;
		
		ThrowError(SW_WRONG_DATA);
	}
	
	if((G_M2MKeySetState&M2M_KeySet1) != M2M_KeySet1)   //之前还未执行KeySet1
	{ 
		G_M2MKeySetState = FALSE;
		
		ThrowError(SW_WRONG_DATA);
	}
	
	if((G_M2MKeySetState & 0x80) == ISDR_KeySet) 
	{
		appID = ISDR_APPID;
	}
	else
	{
		appID = ISDP_APPID;   //ISDP的APPID固定为SE_APP_INSTALLED_MAX
	}
		
 	ecpMem_Set(G_M2MStoreData, 0xFF, sizeof(G_M2MStoreData));

	G_M2MKeySetState = FALSE;
	g_moreCommands = CHAINING_CLEAR;
	
	if((P1 != 0x89)||(P2 != 0x01))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	ecpMem_Set(keyCounter,0,sizeof(keyCounter));
	ecpMem_Set(keyInfo,0,sizeof(keyInfo));
	
	offset = 2; //指向3A02长度
	
	octerNum = ASN1_GetBERTlvLen((UWORD)&RecData[offset],(UWORD*)&totleLen);// 3A02长度
	
	offset += octerNum;//指向3A02的数据域A6
	
	A6Offset = offset; //备份A6 TAG的起始偏移
	
	if(CheckKeyEstablishment2DataAvarable(&RecData[offset],totleLen) != TRUE)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	offset += totleLen;	//	指向7F49 tag
	
	if((RecData[offset] != 0x7F)||(RecData[offset+1] != 0x49)||(RecData[offset+2] != 0x41)||(RecData[offset+3] != 0x04)) // ePK.SR.ECKA 的格式为7F494104XXXXXXXXXXXXX  <WQ_Offline_Function_Test / TC_ES8_EstablishISDPKeyset_SMS_Invalid_EPubKey_SecondStoreData_Para_Error_01>
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	offset += (RecData[offset+2] + 3); //指向5F37 tag
	
	signOffset = offset;
	
	if((RecData[offset] != 0x5F)||(RecData[offset+1] != 0x37) || (RecData[offset+2] != 0x40))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	offset += (RecData[offset+2] + 3); 
	
	if(offset != P3)
	{
		ThrowError(SW_WRONG_LENGTH);//数据总长度不对  
	}
	
	if(VerifyKeyEstablishment2Signature(RecData,signOffset) != TRUE)  // <Figure 23: SM-SR Change> verify signature using PK.SR.ECDSA
	{
		ThrowError(SW_SECURITY_STATUS_NOT_SATISFIED);
	}

	if(ECKACaclSHS(tempBuf) != (ECC_PARA_LEN_256*2))    //Calculate ShS from ePK.SR.ECKA and SK.ECASD.ECKA.  根据ePK.SR 和 sk.ECASD 协商密钥  shs缓存在tempBuf[0..63] 目前只支持ECC256 ，协商密钥长度为0x40字节
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	octerNum = ASN1_GetBERTlvLen((UWORD)&RecData[A6Offset + 1],(UWORD*)&shareInfoLen);//借用shareInfoLen变量 保存A6Tag的长度
	
	shareInfoLen = GetShareInfo(&RecData[A6Offset + 1 + octerNum],shareInfoLen,&tempBuf[64],keyCounter,keyInfo);//根据附录E 4.8章节拼装数据  ShareInfo缓存在tempBuf[64]起始区  长度为shareInfoLen
	
	if(shareInfoLen > 110)	//卡片ram资源有限 ，限制ShareInfo最大只支持110字节
	{
		ThrowError(SYSTEMEXCEPT_NO_RESOURCE);
	}
	
	ecpMem_Move(&RecData[150],&RecData[A6Offset],totleLen);//备份A6数据 ，DerivationScp03Key DerivationScp80Key会拼装putkey到RECDATA
	A6Offset = 150;

	keyLen = tempBuf[64 + keyLengthOff];	//调用KeyDerivationFunction时 产生的密钥长度是否需要根据Key Usage Qualifier来确定？ 如果只需要一个baseKey 需要导出4*keyLen长度的密钥吗？？
	
	KeyDerivationFunction(tempBuf, ECC_PARA_LEN_256, &tempBuf[ECC_PARA_LEN_256*2], shareInfoLen, keyLen*4, &tempBuf[ECC_PARA_LEN_256*2 + shareInfoLen],&tempBuf[ECC_PARA_LEN_256*3 + shareInfoLen*2 + 4]);//EllipticCurveCryptographyVersion.pdf 4.3.3章节，//该函数在epx3.0时做了修改，增加了一个传入参数 -jiangqingbin -20220825

	if(IsSCP03Key(keyInfo[keyVersionNumberOff]))//scp03 key
	{
		if(DerivationScpKey(keyInfo,keyCounter,&tempBuf[ECC_PARA_LEN_256*3 + shareInfoLen*2 + 4] + keyLen,keyLen,appID) == FALSE)
		{
			ThrowError(SW_WRONG_DATA);
		}		
	}
	else if(IsSCP80Key(keyInfo[keyVersionNumberOff]))//scp80 key
	{
		if(keyInfo[keyVersionNumberOff] == IF_GetScp80KeyVersionNum())
		{
			IF_SaveSCP80KeyValueBeforeUpdate();
		}
		
		if(DerivationScpKey(keyInfo,keyCounter,&tempBuf[ECC_PARA_LEN_256*3 + shareInfoLen*2 + 4] + keyLen,keyLen,appID) == FALSE)
		{
			ThrowError(SW_WRONG_DATA);
		}
	}
	else
	{
		//其他密钥暂时没有处理
	}
	
	//计算收条   A6 tag len value + 85  + Len  + DR
	if(keyInfo[scenarioParamOff]&ScenarioPara_IncludeDRFlag)
	{
		RecData[A6Offset + totleLen] = 0x85;
		RecData[A6Offset + totleLen + 1] = 0x10;
		VMReadBuf(&RecData[A6Offset + totleLen + 2] ,EUICC_AuthenTmpBuf + TempSMSRCertRC_Off + 1,0x10);
		
		totleLen += (0x10 + 0x02);
	}
	ecpMem_Copy(&RecData[A6Offset + totleLen + 0x10],&tempBuf[ECC_PARA_LEN_256*3 + shareInfoLen*2 + 4],keyInfo[keyLengthOff]);	//用RecData缓存收条密钥。RecData的有效数据末尾是A6Offset + totleLen，DES收条可能填充8字节800000... 这里偏开16字节避免数据冲掉

	totleLen = EUICCGenReceipt(&RecData[A6Offset + totleLen + 0x10],keyInfo[keyLengthOff], keyInfo[keyTypeOff], &RecData[A6Offset],totleLen, &ResData[0]); // Calculate receipt.  用&ResData[0]作为输出，cmac运算时 输入输出使用的缓存有重叠时会冲掉数据
	
	offset = 0;
	
	if(keyInfo[scenarioParamOff]&ScenarioPara_IncludeDRFlag) // The ISD-R shall return a response indicating a success with the calculated receipt and optionally the DR
	{
		RecData[offset++] = 0x85;
		
		RecData[offset++] = 0x10;
		
		VMReadBuf(&RecData[offset] ,EUICC_AuthenTmpBuf + TempSMSRCertRC_Off + 1,0x10);
		
		offset += 0x10;
	}

	RecData[offset++] = 0x86;
	
	RecData[offset++] = totleLen;
	
	ecpMem_Copy(&RecData[offset],&ResData[0],totleLen);
	
	offset += totleLen;
	
	SetRemoteProcessRes(0x05, offset);//ThrowError(0x6100 + offset);
	
}
/*
*@function    :M2M_StoreData_UpdateSMSRAddressingParameters 
*
*@description :This function is used to update SM-SR addressing Parameters on the eUICC
*
*@see         :SGP02_V3.2  4.1.1.10  SM-SR Addressing Parameters Update: 3A07 L A3LV A4LV A5LV

*@param       :void
*
*@return      :void 
*
*@author      :xuxiaojun
*
*@history     :created by xuxiaojun@20181225
*@history     :modified by xuxiaojun@20210414 _newarrayObject内已经存在异常判断
*注意         :这里没有考虑A3，A4,A5重复出现的情况，目前的处理是允许重复出现，写入到卡内的值是最后一次的值，且每一次的数据都是有效的。
*/
void M2M_StoreData_UpdateSMSRAddressingParameters(void)
{
	BYTE idata recOffset, index,deleteFlag;
	BYTE idata transactionTag,subTagLen;
	WORD idata result, paraRef;
	BYTE idata bakOffset[3];  // 用于备份A3\A4\A5的L域的起始偏移
	UWORD xdata paraAddr,paraRefAddr;
	
	if((P1 != 0x88) || (P2 != 0x00))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if((P3 != (RecData[2] + 3)) || (RecData[2] == 0x00))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	deleteFlag = FALSE;

	paraRef = REF_NULL;

	recOffset = 3;  //偏移到第一个TAG的位置
	
	result = 0x9000;
	
	subTagLen = 0;
	
	ecpMem_Set(bakOffset, 0x00, sizeof(bakOffset));
	
	while((recOffset < P3) && (result == 0x9000))
	{
		switch(RecData[recOffset])
		{
			case 0xA3:
			{							
				subTagLen += (RecData[recOffset + 1] + 2);
				
				result = CheckSMSRAddressingParametersSMS((BYTE*)&RecData[recOffset+1]); 
				
				bakOffset[0] = recOffset+1;
				
				break;
			}
			case 0xA4:
			{			
				subTagLen += (RecData[recOffset + 1] + 2);
				
				result = CheckSMSRAddressingParametersCATTP((BYTE*)&RecData[recOffset+1]);
				
				bakOffset[1] = recOffset+1;
				
				break;
			}
			case 0xA5:
			{				
				subTagLen += (RecData[recOffset + 1] + 2);
				
				result = CheckSMSRAddressingParametersDNS((BYTE*)&RecData[recOffset+1]);
				
				bakOffset[2] = recOffset+1;
				
				break;
			}
			default:
				result = SW_WRONG_DATA;
		}
		
		if(result != 0x9000)
		{
			ThrowError(result);
		}
		
		if(subTagLen > RecData[2])//统计子tag的长度和3A07的长度比较，子tag长度不能大于
		{
			ThrowError(SW_WRONG_DATA);
		}
	
		recOffset += (RecData[recOffset+1] + 2);
	}
	
	if(subTagLen != RecData[2])
	{
		ThrowError(result);
	}
	
	transactionTag = IF_BeginTransaction();				
		
	for(index = 0; index < 3; index++)
	{
		switch(index)
		{
			case 0:
				paraRefAddr = M2MSMSRAddressPara_A3Ref_JC;
				break;
			case 1:
				paraRefAddr = M2MSMSRAddressPara_A4Ref_JC;
				break;
			case 2:
				paraRefAddr = M2MSMSRAddressPara_A5Ref_JC;
				break;
			default:
				ThrowError(SW_WRONG_DATA);
		}
		if(bakOffset[index] != 0x00)  //对应的TAG存在则更新进到卡中
		{
			paraAddr = GetAddrAndIndexByIndexAddr(paraRefAddr, &paraRef);
			
			if(paraAddr != ADDR_IS_NULL)  //对应的参数存在，则应该先释放
			{
				paraAddr =  get_array_data((PF8)paraAddr);

				if(VMReadByte(paraAddr) != RecData[bakOffset[index]])//长度不相等  重新分配
				{
					paraRef = REF_NULL;
					deleteFlag = TRUE;
				}
			}
			
			if(paraRef == REF_NULL)//paraAddr = VMCOSHeapAlloc(RecData[bakOffset[index]] + 1, &paraRef, CardMemory_Mode);
			{
				paraRef = _newarrayObject(AType_Byte, RecData[bakOffset[index]] + 1, BIT_NEED_REFRESH);

				paraAddr =  get_array_data((PF8)locateObjectInMemory(paraRef));
			}

			VMWriteBuf(paraAddr, &RecData[bakOffset[index]], RecData[bakOffset[index]]+1);
			
			memWriteU16Atomic(MEMtPM8(paraRefAddr),paraRef);//VMWriteBuf(paraRefAddr, (BYTE*)&paraRef, 0x02);
		}
	}	
		
	IF_CommitTransaction(transactionTag);

	if(deleteFlag == TRUE)
	{
		bRamBuffer[GC_Requested] = TRUE;
		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmGcRequestStateBase()), TRUE);
	}
}

/*
*@function   :M2M_StoreData_SetFallbackAttribute
*
*@description: This function sets the Fall-back Attribute for one Profile on the target eUICC. 
*
*@see         :SGP02_V3.2  4.1.1.7  Set Fall-back Attribute: 3A05

*@param       : void
*
*@return      : void 
*
*@author      :LIGUIYU@20181219
*
*@history     :V1.0
*/
void M2M_StoreData_SetFallbackAttribute(void)
{
	BYTE idata targetProfileIndex;
	
	if((P1 != 0x88)||(P2 != 0x00))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if(P3 != (RecData[2] + 3))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(RecData[3] == 0x4F)		// teag AID
	{
		if((RecData[4] + 2) != RecData[2])
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		if(AIDLenInvalid(RecData[4]))
		{
			ThrowError(SW_WRONG_DATA);
		}
			
		if(GetDownloadingProfileIndexByAID(&RecData[5], RecData[4]) != ILLEGAL_PROFILEINDEX)//恒宝离线脚本测试 Tele_eUICC_Management_Command_normal_test_01 激活一个状态为seletable或者personal状态的Profile期望返6985
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}
		
		targetProfileIndex = GetProfileIndexByAID(&RecData[5], RecData[4]);
		
		if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
		{
			ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
		}
		
		JcvmConfig_SetM2MFallBackProfileIndex(targetProfileIndex);
		
		return;
	}
	
	ThrowError(SW_WRONG_DATA);
}
/*
*@function    :M2M_StoreData_ProfileDisable
*
*@description :This function is used to disable a Profile on the eUICC. 
*
*@see         :SGP02_V3.2  4.1.1.3  Profile Disabling :3A04

*@param       :void 输入数据直接在RecData中 RecData[0]起始是3A03124F10 ISD-P AID
*
*@return      :void 
*
*@author      :xuxiaojun
*
*@history     :created by xuxiaojun@20181219
			  ::V1.1 LCZ 2020 0628 添加设置UpdateChangeProfileInitPrivateFile私有文件初始化标志,复位后在SEEUICCVariableInit函数里面根据标志初始化
*/
void M2M_StoreData_ProfileDisable(void)
{
	BYTE idata targetProfileIndex, curPOL1, fallBackProfileIndex;
	BYTE idata transactionTag;
	
	if((P1 != 0x88)||(P2 != 0x00))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if((P3 != 0x15) || (RecData[2] != 0x12) || (RecData[3] != 0x4F) || (RecData[4] != 0x10))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	targetProfileIndex = GetDownloadingProfileIndexByAID(&RecData[5], RecData[4]); //ISDP已安装但PE数据没下载。测试用例：UL套件《TC_ES5_DISP_1_DisableProfile_SMS_02_Error_Case_ISD_P_Not_Enabled_17_00_14》
	
	if(targetProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	targetProfileIndex = GetProfileIndexByAID(&RecData[5], RecData[4]);
	
	if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
	{
		ThrowError(SW_REFERENCED_DATA_NOT_FOUND);//: Profile not found
	}
	
	if(GetProfileState(targetProfileIndex) != PROFILE_STATE_ENABLED) //Verify that the target Profile is in Enabled state
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED); 
	}
	
	curPOL1 = GetProfilePOL1(G_CurEnabledProfileIndex);
	
	if((curPOL1 & POL1_DISABLE_NOT_ALLOWED) == POL1_DISABLE_NOT_ALLOWED)  // Verify that POL1 of the currently Enabled Profile allows its disabling 
	{
		ThrowError(EUICC_PROFILE_PREVENT_ACTION);  // POL1 of the currently Enabled Profile or of the previously enabled profile prevents this action.
	}
	
	fallBackProfileIndex = JcvmConfig_GetM2MFallBackProfileIndex();
	
	if((targetProfileIndex == fallBackProfileIndex) ||(fallBackProfileIndex == ILLEGAL_PROFILEINDEX))  // Verify that the target Profile is in Enabled state
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);  // ’69 85’: Profile is not in the Enabled state or Profile has the Fall-back Attribute. 
	}
	
	transactionTag = IF_BeginTransaction();
		
    /*
	NOTE: 
	对于 disable一样
	
	1：不立即切换profile的状态，因为当前并没有马上进行refresh操作
	
	2： 也不设置全局变量 G_CurEnabledProfileIndex 等的值 ，
		
	BUT :考虑  没有refresh之前是否允许执行新的profilestatechange相关的命令呢
	
	该操作的refresh发送的时机也要注意，如果对于HTTP 没有收到关通道命令，则在收到enable命令后的10个status之后发送refrsh
	
    */	
	JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX);//tmp[0] = 0xFF;                                    // M2MFallBackDisabledNoAllowProfileIndex,对于该字段而言，如果M2MFallBackDisabledNoAllowProfileIndex 有有效数据，则执行到这里，该字段要被清空成FF ，如果该字段本来就没数据，那么其值本来就是FF(主动disableProfile，这个字段应该要清成FF)
	
	JcvmConfig_SetM2MLastDisableProfileIndex(G_CurEnabledProfileIndex);//tmp[1] = G_CurEnabledProfileIndex;                //M2MLastDisableProfileIndex
	
	JcvmConfig_SetM2MNotificationType(NOTIFICATION_RROFILE_CHANGE_SUCCEEDED);//tmp[2] = NOTIFICATION_RROFILE_CHANGE_SUCCEEDED;   //M2MNotificationType
	
	JcvmConfig_SetUpdataEnableProfileIndex(fallBackProfileIndex);//tmp[3] = fallBackProfileIndex;                      //M2MUpdateEnableProfileIndex

	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_M2M_SetRefreshFlag();
	
	IF_CommitTransaction(transactionTag);	
}
/*
*@function   :M2M_StoreData_ProfileEnable
*
*@description:This function is used to enable a Profile on the eUICC
*
*@see         :SGP02_V3.2  4.1.1.2  Profile Enabling:3A03

*@param       :void，输入数据直接在RecData中 RecData[0]起始是3A03124F10 ISD-P AID
*
*@return      :void 
*
*@author      :xuxiaojun@20181219
*
*@history     :created by xuxiaojun@20181219
*/
void M2M_StoreData_ProfileEnable(void)
{
	BYTE idata targetProfileIndex, POL1, preEnabledProfileIndex;
	BYTE idata transactionTag;
	
	if((P1 != 0x88)||(P2 != 0x00))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if((P3 != 0x15) || (RecData[2] != 0x12) || (RecData[3] != 0x4F) || (RecData[4] != 0x10))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	targetProfileIndex = GetDownloadingProfileIndexByAID(&RecData[5], RecData[4]); //ISDP已安装但PE数据没下载。测试用例：UL套件《TC_ES5_DISP_1_DisableProfile_SMS_02_Error_Case_ISD_P_Not_Enabled_17_00_14》
	
	if(targetProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	targetProfileIndex = GetProfileIndexByAID(&RecData[5], RecData[4]);
	
	if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
	{
		ThrowError(SW_REFERENCED_DATA_NOT_FOUND);// Profile not found
	}
	
	if(GetProfileState(targetProfileIndex) != PROFILE_STATE_DISABLED)  //Verify that the target Profile is in the Disabled state.
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);  // ’69 85’: Profile is not in the Disabled state. 
	}
	
	POL1 = GetProfilePOL1(G_CurEnabledProfileIndex);
	
	if((POL1 & POL1_DISABLE_NOT_ALLOWED) == POL1_DISABLE_NOT_ALLOWED)  // Verify that POL1 of the currently Enabled Profile allows its disabling
	{
		ThrowError(EUICC_PROFILE_PREVENT_ACTION);  // POL1 of the currently Enabled Profile or of the previously enabled profile prevents this action.
	}
	
	preEnabledProfileIndex = JcvmConfig_GetM2MFallBackDisabledNoAllowProfileIndex(); //If the current profile has been enabled as a result of the Fall-back activation procedure
	
	if(preEnabledProfileIndex != ILLEGAL_PROFILEINDEX)  //之前由于fall-back机制，导致失效了应用， preEnabledProfileIndex本身就是   “POL1不允许失效的”
	{		
		if( targetProfileIndex != preEnabledProfileIndex )
		{
			ThrowError(EUICC_PROFILE_PREVENT_ACTION);  // POL1 of the currently Enabled Profile or of the previously enabled profile prevents this action.
		}
	}
	
	transactionTag = IF_BeginTransaction();
			
	/*
	Note: 考虑到卡片收到这条命令后并不会马上执行refresh操作，而此时还位于当前激活profile下面
	
	此处暂时不立即 
	
	1： 把G_CurEnabledProfileIndex 的状态设置成 PROFILE_STATE_DISABLED， 也不把 targetProfileIndex设置成 PROFILE_STATE_ENABLED ，防止获取状态
	
	2： 也不设置全局变量 G_CurEnabledProfileIndex 等的值 ，
	
	
	BUT :考虑  没有refresh之前是否允许执行新的profilestatechange相关的命令呢
	
	该操作的refresh发送的时机也要注意，如果对于HTTP 没有收到关通道命令，则在收到enable命令后的10个status之后发送refrsh
	*/
	
	JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX);//tmp[0] = 0xFF;                                    // M2MFallBackDisabledNoAllowProfileIndex,对于该字段而言，如果M2MFallBackDisabledNoAllowProfileIndex 有有效数据，则执行到这里，该字段要被清空成FF ，如果该字段本来就没数据，那么其值本来就是FF
	
	JcvmConfig_SetM2MLastDisableProfileIndex(G_CurEnabledProfileIndex);//tmp[1] = G_CurEnabledProfileIndex;                //M2MLastDisableProfileIndex
	
	JcvmConfig_SetM2MNotificationType(NOTIFICATION_RROFILE_CHANGE_SUCCEEDED);//tmp[2] = NOTIFICATION_RROFILE_CHANGE_SUCCEEDED;   //M2MNotificationType
	
	JcvmConfig_SetUpdataEnableProfileIndex(targetProfileIndex);//tmp[3] = targetProfileIndex;                      //M2MUpdateEnableProfileIndex
	
	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_M2M_SetRefreshFlag();

	IF_CommitTransaction(transactionTag);
}
/*
*@function   :M2M_GetData_eUICCCapabilityAudit
*
*@description:This function is used to query the status of the eUICC.
*
*@see         :<SGP02_V3.2>       4.1.1.5  eUICC Capability Audit
*             :<SGP.11_v3.2.pdf>  TC.ES5.FIH.1: FinaliseISDRhandover_SMS 

*@param       :void，输入数据直接在RecData中 RecData[0]起始是5CXX ,根据P1P2确定tag
*
*@return      :void 
*
*@author      :lgy
*
*@history     :created by lgy@20181226
*             :modify by xuxiaojun@20190429  增加66TAG获取，增加GET 00E0信息, SGP11规范
*/
BYTE M2M_GetData_eUICCCapabilityAudit(void)
{
	WORD xdata P1P2,certLen,dataLen,euiccCertRef;
	BYTE idata profileNum,i,offset;
	UWORD xdata ECASDCertAddr;

	P1P2 = ( ((WORD)P1 << 8) | P2 );

	dataLen = 0;

	if(P1P2 == 0xFF21)
	{
		if(P3 != 0)
		{
			ThrowError(SW_WRONG_DATA);
		}

		profileNum = 0;  //统计ISD-P的个数

		for(i = 0;i < PROFILE_MAX_NUMBER;i++)
		{
			if(VMReadByte(PROFILE_VALIDTAG_CFG + i) == PROFILE_VALIDFLAG)
			{
				profileNum++;  //有效的profile个数
			}
		}

		dataLen = getCardResoure(0xFF21, RecData, profileNum);//paddingFF21Data(RecData,profileNum);

		ecpMem_Move(&RecData[3], &RecData[0], dataLen);

		SetWord(RecData,0xFF21);

		RecData[2] = dataLen;

		SetRemoteProcessRes(0x05, dataLen + 3);

		return TRUE;//ThrowError(0x6100 + dataLen + 3);//目前假定 G_ApduBuf + ListBuf足够缓存一张证书
	}
	else if(P1P2 == 0xBF30)
	{
		if(P3 != (RecData[1] + 2))
		{
			ThrowError(SW_WRONG_DATA);
		}

		if((RecData[0] == 0x5C)&&(RecData[1] == 0x01)&&(RecData[2] == 0x66)) //ECASD recognition data: ‘5C 01 66’  GP 附录C？<GlobalPlatform Card, Confidential Card Content Management, Card Specification v2.3 – Amendment A, v1.1 > 3.3.4  Forwarded CASD Data
		{
			if(EUICCECASD66LV_Addr == ADDR_IS_NULL)
			{
				ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
			}

			offset = VMReadByte(EUICCECASD66LV_Addr); //ECASD 66，LV结构

			VMReadBuf(RecData, EUICCECASD66LV_Addr, offset+1);

			ecpMem_Move(RecData + 4, RecData, offset+1);

			RecData[0] = 0xBF;

			RecData[1] = 0x30;

			RecData[2] = offset+2;

			RecData[3] = 0x66;

			SetRemoteProcessRes(0x05, offset+5);

			return TRUE;
		}
		else if((RecData[0] == 0x5C)&&(RecData[1] == 0x02)&&(RecData[2] == 0x7F)&&(RecData[3] == 0x21))
		{
			euiccCertRef = memReadU16(MEMtPM8(M2M_EUICCEUMUICCCARef_Addr));//ECASDCertAddr = GetAddrByIndexAddr(EUICCEUMUICCCARef_Addr);

			if(euiccCertRef == REF_NULL)
			{
				offset = 0;
				RecData[offset++] = 0xBF;
				RecData[offset++] = 0x30;
				RecData[offset++] = 0x03;
				RecData[offset++] = 0x7F;
				RecData[offset++] = 0x21;
				RecData[offset++] = 0x00;

				SetRemoteProcessRes(0x05, offset);

				//ThrowError(0x6100 + offset);

				return TRUE;
			}

			ECASDCertAddr = locateObjectInMemory(euiccCertRef);

			ECASDCertAddr = M2MCAEUICCListGet_curCertEUICCArrayValueAddr(ECASDCertAddr); //ECASD证书地址 LV结构

			VMReadBuf(tempBuf,ECASDCertAddr,2);

			certLen = BuildWord(tempBuf);

			offset = 10;// BF30 XX XX XX 7F21 XX XX XX CERT --预留10字节

			if((certLen + offset) > APDU_BUFFER_SIZE)// 考虑到目前获取的证书长度一般不会超过512字节 用ApduBuff 和ListBuffer做缓存
			{
				ThrowError(SW_CONDITIONS_NOT_SATISFIED);
			}

			VMReadBuf(&RecData[offset],ECASDCertAddr+2,certLen);	// ECASDCertAddr 是LV结构 ，L占2字节

			if(certLen > 0xFF)//证书长度大于255
			{
				RecData[offset-5] = 0x7F;
				RecData[offset-4] = 0x21;
				RecData[offset-3] = 0x82;
				RecData[offset-2] = (certLen>>8)&0xFF;
				RecData[offset-1] = certLen&0xFF;

				dataLen = certLen + 5; //BF30的后续长度 7F21 82 XX XX + CERT

				RecData[offset-10] = 0xBF;
				RecData[offset-9] = 0x30;
				RecData[offset-8] = 0x82;
				RecData[offset-7] = (dataLen>>8)&0xFF;
				RecData[offset-6] = dataLen&0xFF;

				offset = 0;
				dataLen += 5;//数据总长度
			}
			else if(certLen > 0x7F)//证书长度大于128
			{
				RecData[offset-4] = 0x7F;
				RecData[offset-3] = 0x21;
				RecData[offset-2] = 0x81;
				RecData[offset-1] = certLen&0xFF;

				dataLen = certLen + 4;//BF30的后续长度 7F21 81 XX + CERT

				if(dataLen > 0xFF)// 证书长度 + 上一级Tag7F21长度超过了255
				{
					RecData[offset-9] = 0xBF;
					RecData[offset-8] = 0x30;
					RecData[offset-7] = 0x82;
					RecData[offset-6] = (dataLen>>8)&0xFF;
					RecData[offset-5] = dataLen&0xFF;

					offset = 1;
					dataLen += 5;
				}
				else
				{
					RecData[offset-8] = 0xBF;
					RecData[offset-7] = 0x30;
					RecData[offset-6] = 0x81;
					RecData[offset-5] = dataLen&0xFF;

					offset = 2;
					dataLen += 4;

				}
			}
			else//证书长度小于128
			{
				RecData[offset-3] = 0x7F;
				RecData[offset-2] = 0x21;
				RecData[offset-1] = certLen&0xFF;

				dataLen = certLen + 3;//BF30的后续长度 7F21 XX + CERT

				if(dataLen > 0x7F)//证书长度 + 上一级Tag7F21长度超过了128
				{
					RecData[offset-7] = 0xBF;
					RecData[offset-6] = 0x30;
					RecData[offset-5] = 0x81;
					RecData[offset-4] = dataLen&0xFF;

					offset = 3;
					dataLen += 4;
				}
				else
				{
					RecData[offset-6] = 0xBF;
					RecData[offset-5] = 0x30;
					RecData[offset-4] = dataLen&0xFF;

					offset = 4;
					dataLen += 3;
				}
			}

			if(offset != 0)
			{
				ecpMem_Move(RecData,&RecData[offset],dataLen);
			}

			SetRemoteProcessRes(0x05, dataLen);

			return TRUE;
/*			if(dataLen > 0xFF)
			{
				ThrowError(0x62F1);
			}
			else
			{
				ThrowError(0x6100 + dataLen);//目前假定 G_ApduBuf + ListBuf足够缓存一张证书
			}
*/
		}
		else
		{
			ThrowError(SW_WRONG_DATA);
		}
	}
	else if(P1P2 == 0x00E0) //eUICC_M2M_SGP_11_v3_3_Card_Compliance_Test_Suite_v2_2_0 / TC_ES5_ECA_1_eUICCCapabilityAudit_SMS_05_Nominal_Case_Retrieve_ECASD_Recognition_Data
	{
		return FALSE; //Java层处理
	}
	else
	{
		ThrowError(SW_WRONG_DATA);
	}

	return TRUE;
}
/*
*@function   : M2M_ISDPStoreData
*
*@description: 执行M2M版本下的storeData UpdatePOL1byMNO   UpdateConnectivityParametersByMNO 
*
*@param      :void
*
*@return     :true: 命令在当前函数中已执行了
*
*@author     :
*
*@history    :
*/
BYTE M2M_ISDPStoreData(void)
{
	if(RecData[0] == 0x3A)
	{
		if(RecData[1] == 0x06)
		{
			M2M_StoreData_UpdatePOL1ByMNO();
			
			return TRUE;
		}
		else if(RecData[1] == 0x07)
		{
			M2M_StoreData_UpdateConnectivityParameters(G_CurEnabledProfileIndex);
			
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}
/*
*@function   : M2M_ISDRStoreData
*
*@description: 执行M2M版本下的storeData，如 profile enable、profile disable、set fall-back attribute、 isd-r setestablishment 
*
*@param      :void
*
*@return     :true: 命令在当前函数中已执行了
*
*@author     :
*
*@history    :
*/
BYTE M2M_ISDRStoreData(void)
{
	BYTE idata keySetType;
	
	if(G_M2MStoreData[1] != ILLEGAL_PROFILEINDEX)   // 之前有执行install for personal
	{
		if(RecData[0] != 0x3A)
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		keySetType  = ISDP_KeySet;
	}
	else
	{
		keySetType  = ISDR_KeySet;
	}
	
	
	if(P3 < 2)   //ISDR 的store Data数据域至少2字节
	{
		ThrowError(SW_WRONG_LENGTH);	
	}
	if(RecData[0] == 0x3A)
	{
		switch(RecData[1])
		{
			case 0x03:  // SGP02_V3.2 4.1.1.2  Profile Enabling 
				M2M_StoreData_ProfileEnable();
				break;
			case 0x04:  // SGP02_V3.2 4.1.1.3  Profile Disabling
				M2M_StoreData_ProfileDisable();
				break;
			case 0x05: //SGP02_V3.2 4.1.1.7  Set Fall-back Attribute
				M2M_StoreData_SetFallbackAttribute();	
				break;
			case 0x01:  //SGP02_V3.2 4.1.1.8  ISD-R Key Set Establishment1
				M2M_StoreData_EstablishISDRKeySet1(keySetType);
				break;
			case 0x02:  //SGP02_V3.2 4.1.1.8  ISD-R Key Set Establishment2
				M2M_StoreData_EstablishISDRKeySet2(keySetType);
				break;
			case 0x07: //SGP02_V3.2 SM-SR Addressing Parameters Update 
				M2M_StoreData_UpdateSMSRAddressingParameters();
				break;
			case 0x08: //4.1.1.12  Notification Confirmation 
				M2M_StoreData_HandleNotificationConfirmation();
				break;
#if M2M_ESx
			case 0x09: //4.1.1.13 Set Emergency Profile Attribute add by chentao@20220929
				M2M_StoreData_Set_Emergency_Profile_Attribute();
				break;
#endif
			default :
				ThrowError(SW_WRONG_DATA);
		}
		
		return TRUE;
	}
	else
	{
		if((G_M2MStoreData[0] == 0x01)&&((P1 == 0x89)&&(P2 == 0x01))&&((RecData[0] != 0x3A)||(RecData[1] != 0x02)))//执行了M2M_StoreData_EstablishISDRKeySet1 ，但是后续来了第二条(P2 = 01)指令又不是3A02 
		{
			ThrowError(SW_WRONG_DATA);//HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC7_ISDR_KeyEstablishment_ERROR_01 步骤6
		}

		if(((P1&0x08) == 0x08)//DGI 仅支持3AXX  ，0070 5F20
			&&((RecData[0] != 0x00)||(RecData[1] != 0x70))&&((RecData[0] != 0x5F)||(RecData[1] != 0x20))
			)
		{
			ThrowError(SW_WRONG_DATA);
		}//HB_Offline_Test/4管理指令异常数据测试/

	}		
	
	return FALSE;
}
/*
*@function   : M2M_StoreData_UpdatePOL1ByMNO
*
*@description: This function is used to update POL1 on the eUICC.
*
*@see         :SGP02_V3.12
*
*@param       : void
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0 :create @20181227
*/
void M2M_StoreData_UpdatePOL1ByMNO(void)
{
	UWORD xdata entryAddr;

	if((P1 != 0x88) || (P2 != 0))
	{
		ThrowError(SW_INCORRECT_P1P2);
	}
	
	if((P3 != 0x06))
	{
		ThrowError(SW_WRONG_LENGTH); 
	}
	
	if((RecData[2] != 0x03) || (RecData[3] != 0x81) || (RecData[4] != 0x01))
	{
		ThrowError(SW_WRONG_DATA); 
	}
	
	if(RecData[5] > 0x04)  //POL 1规则错误，根据SGP02_V3.2 ，<Table 71: POL1 Coding > POL1有效值仅为00 01 02 03 04
	{
		ThrowError(SW_WRONG_DATA); 
	}
	
	//todo :是否要判断只有有发送installforpersonation呢？
	
	entryAddr = GetProfile_M2M_POL1Addr(GetISDPObjectAddr(G_CurEnabledProfileIndex));//GetProfile_M2M_POL1Addr(GetProfileEntryAddr(G_CurEnabledProfileIndex));////VMSetBuf(PROFILE_POL1 + G_CurEnabledProfileIndex, RecData[5], 0x01);  //更新的其实就是当前激活的profile的POL 的规则
	memWriteU8Atomic(MEMtPM8(entryAddr),RecData[5]);
	
}
/*
*@function : M2M_ISDRRAMCommand
*
*@description:M2M版本下，处理ISD R 的RAM命令，参考标准SGP02_V3.2  4.1.1  ES5 (SM-SR–eUICC) Interface Description  
*
*@param      : void
*
*@return     :TRUE/FALSE ，TRUE表示当前指令已执行，FALSE表示指令未执行 ，交给下一级处理
*
*@author     :xuyz
*
*@history    :V1.0 :create @20190304
*            :V1.1 :modify by xuxioajun@20190507  通过RAM方式操作ISD-R时，由于ISD-R下来的指令都是可信的，所以，EUICC规范中定义的功能以外的，符合标准GP的指令，统一交给标准GP处理
*/

void M2M_ISDRRAMCommand(void)
{
	BYTE idata result;
	
	result = FALSE;
	
	switch(INS)
	{
		case 0xE6:   //ISD-P creation
			if(P1 == 0x0C)
			{
				result = M2M_CreateISDP();
			}
			else if(P1 == 0x20)
			{
				result = M2M_InstallForPersonalization();
			}
			else
			{
				ThrowError(SW_WRONG_DATA); 
			}
			break;
		case 0xE2:   //profile Enable ： 3A03/ profile Disable： 3A04/ISD-R Key Set Establishment 3A01 3A02/SM-SR Addressing Parameters Update:3A07
			result = M2M_ISDRStoreData();
			break;
		case 0xE4:   //Profile Deletion 
			result = M2M_DeleteCommand();
			break;
		case 0xCA:  // eUICC Capability Audit 
			result = M2M_GetData_eUICCCapabilityAudit();
			break;  
		case 0xF2: // <SGP.02_v3.2_updated.pdf> 4.1.1.5  eUICC Capability Audit 
			//M2M_GetStatus_eUICCCapabilityAudit();
			result = FALSE;
			break;

		case 0xF0://ISDR -- set status
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
			break;
		default:    //eUICC_Profile_Package_Interoperable_Format_Test_V_2_1_2/TestSuite_BasicInitialState/Check_Profile_Format 会通过RAM 向ISD R putKey
			
			break; 

	}
	
	pushShort(result);
}

/*
*@function   : M2M_InstallForPersonalization
*
*@description: 在M2M 版本下执行INSTALL [for personalization] 操作，包括ES6接口和ES8的 4.1.3.1  ISD-P Key Set Establishment 操作
*
*@param      : void
*
*@return     : void
*
*@auhtor     :
*
*@history    :
*/
BYTE code ISDPReserved_AID[0x10]   = {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x0F,0x00};  //SGP02_V3.2 附录H
BYTE M2M_InstallForPersonalization(void)
{
	BYTE idata profileIndex;
	
	if((RecData[0] != 0x00)||(RecData[1] != 0x00))
	{
		ThrowError(SW_WRONG_DATA);
	}

	if(P3 != (RecData[2] + 0x06))
	{
		ThrowError(SW_WRONG_DATA);
	}

	if(AIDLenInvalid(RecData[2]))//握其离线脚本TC_ES8_EstablishISDPKeyset_SMS_installForPersonalization_Para_Error_01 AID长度为4 要求报6A80
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(RecData[2] != 0x10)   //ISDP AID length error
	{
		ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
	}

	if((RecData[P3 - 1] != 0x00)||(RecData[P3 -2] != 0x00)||(RecData[P3 -3] != 0x00))  //该指令的后面3字节不为0
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(ecpMem_Compare(&RecData[3], (BYTE *)ISDPReserved_AID, 0x10) == EQUAL)   //保留的ISD P AID， 处理ES6接口时，需要用到该AID（更新POL和链接参数by MNOSD）
	{
		profileIndex = G_CurEnabledProfileIndex;
	}
	else		//(ISD-P Key Set Establishment等...)
	{
		profileIndex = GetDownloadingProfileIndexByAID(&RecData[3], 0x10); 
		
		if(ProfileIndexIsValid(profileIndex) == FALSE)  //profile ISDP AID 有重复
		{
			ThrowError(SW_REFERENCED_DATA_NOT_FOUND);
		}
	}
	
	G_M2MStoreData[0] = 0;  //Clear StoreNumber

	G_M2MStoreData[1] = profileIndex;

	RecData[0] = 0x00;

	SetRemoteProcessRes(0x05, 1);//ThrowError(0x6101);

	return TRUE;
}
/*
*@function : M2M_ISDPRAMCommand
*
*@description:M2M版本下，处理ISD P的RAM命令，参考标准SGP02_V3.2  4.1.3  ES8 (SM-DP-eUICC) Interface Description
*
*@param      : void
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 :create @20190304
*/
void M2M_ISDPRAMCommand(void)
{
	BYTE idata result = FALSE;

	switch(INS)
	{
		case 0xE2:  //4.1.3.4  Connectivity Parameters Update using SCP03

			if(RecData[0] == 0x3A)
			{
				switch(RecData[1])
				{
					case 0x01:
						M2M_StoreData_EstablishISDRKeySet1(ISDP_KeySet);
						result = TRUE;
						break;
					case 0x02:
						M2M_StoreData_EstablishISDRKeySet2(ISDP_KeySet);
						result = TRUE; //后续要调putkey
						break;

					case 0x07:
						M2M_StoreData_UpdateConnectivityParameters(IF_GetRAMProfileIndex());//该指令无响应数据
						result = TRUE;
						break;//算RMAC
					default:

						ThrowError(0x6A80);
						break;
				}
			}
			else
			{
				StoreGeneraldataForISD_P(IF_GetRAMProfileIndex());//普通storedata 42 45等
				result = TRUE;
			}

			break;

		case 0xD8:
			result = FALSE;
			break;//需要算RMAC

		case 0x50:  //INITIALIZE UPDATE
		case 0x82:  //EXTERNAL AUTHENTICATE
			result = TRUE;
			break;

		case 0xCA:
			ThrowError(0x6A86);	//HB_Offline_Test/4管理指令异常数据测试/Tele_eUICCCapabilityAudit_GetData_Errortest_01 测试到发CA指令给ISD-P 期望返6A86 ，目前的方案是ISD-P只支持建立安全会话 ，storeData指令，putKey指令 其余指令统一返6A80，所以在这里对CA指令做特殊处理
			break;

		default :
			ThrowError(0x6A80);
			//return TRUE;
	}
	pushShort(result);
}

/*
*@function   : NM_ECASDGetData
*
*@descrition : 在ECASD中中获取GetData
×
×@param      : void
*
*@return     :
*
*@author     :xuyz
*
*@history    :V1.0 :create @20190311
*            :V1.1 :modify by xuxiaojun@20191025 通过GET DATA方式获取EID的指令，只给M2M使用，RSP不通过这种方式。<SGP.02_v3.2_updated.pdf> The EID shall be stored within the ECASD and can be retrieved by the Device at any time
*/
//void NM_ECASDGetData(void)
//{
//#ifndef RSP_EUICC
//	if((P1==00)&&(P2==0x5A))
//	{
//		RecData[0] = 0x5A;
//
//		VMReadBuf((BYTE*)&RecData[1], EUICCEIDInfo_Addr, 0x11);
//
//		SetOutgoingAndSend(0x05,(RecData[1]+2));
//	}
//	else
//	{
//		ThrowError(0x6A80);
//	}
//#else
//	ThrowError(0x6D00);
//#endif
//}
/*
*@function   : NM_M2MISDPStoreDataPersonal
*
*@descrition : 用于修改ISD-P的SMSC,HTTP参数，CATTP参数等,产业化需求：A4选择ISD-P后发storeData ，指令格式参考SGP02V3.2 的4.1.3.4 Connectivity Parameters Update using SCP03
			   数据来源本地APDU，正常情况M2M版只有RAM指令，这里仅针对产业化做后个人化 ,发卡后不允许修改
×
×@param      : void
*
*@return     :
*
*@author     :lgy
*
*@history    :V1.0 :create @20190412
*/
void NM_M2MISDPStoreDataPersonal(void)
{

	BYTE idata result;
	
	if(JcvmConfig_GetSEState()!= CARD_OP_READY)
	{
		ThrowError(0x6985);
	}
		
	result = M2M_ISDPStoreData();
	
	if(result == FALSE)
	{
		ThrowError(0x6A80);
	}

}
/*
*@function   : NM_M2MISDRStoreDataPersonal
*
*@descrition : 用于修改ISD-R的SMS,BIP参数，DNS  产业化需求：A4选择ISD-R后发storeData ，指令格式参考SGP02V3.2 的4.1.1.10 SM-SR Addressing Parameters Update
			   数据来源本地APDU，正常情况M2M版只有RAM指令，这里仅针对产业化做后个人化，发卡后不允许修改
×
×@param      : void
*
*@return     :
*
*@author     :lgy
*
*@history    :V1.0 :create @20190415
*/
//void NM_M2MISDRStoreDataPersonal(void)
//{
//
//	if((RecData[0] == 0x3A)&&(RecData[1] == 0x07))
//	{
//		if(JcvmConfig_GetCardState() != CARD_OP_READY)
//		{
//			ThrowError(0x6985);
//		}
//
//		M2M_StoreData_UpdateSMSRAddressingParameters();
//
//		PushOperandStackShort(TRUE);
//
//		return;
//	}
//	else if((RecData[0] == 0x42)||(RecData[0] == 0x45)||(RecData[0] == 0x85)||(RecData[0] == 0xA5)||((RecData[0] == 0x00)&&(RecData[1] == 0x70)))// 42 45 或者0070 L 42 ，45
//	{
//		if(JcvmConfig_GetCardState() != CARD_OP_READY)
//		{
//			ThrowError(0x6985);
//		}
//
//		StoreGeneraldataForISD_P_R(0xFF,ISDR_APPID);
//
//		PushOperandStackShort(TRUE);
//
//		return;
//	}
//	else if((RecData[0]&eSE_Private_TagFirstByte) == eSE_Private_TagFirstByte)//eSE 私有API定义的TAG为DF或FF
//	{
//		CallEseInterfaceFunc(RecData[1]);
//		return;
//	}
//	else
//	{
//		ThrowError(0x6985); //设备版仅允许执行3A07 ，42 45其他直接抛异常 ，不允许本地STORE DATA指令
//	}
//
//
//	PushOperandStackShort(FALSE);
//}

/*@function   : M2MLocalEnvelopCommand
*
*@descrition : The eUICC MAY support the Local Enable and Disable command for the Emergency Profile.
			   The eUICC MAY support the Local Enable and Disable command for the Test Profile.
×
×@param      : void
*
*@return     result
*@return     success (0),
*@return     errorProfileRef (8), -- Emergency/Test Profile does not exists
*@return     errorAlreadyEnabled (9), -- Emergency/Test Profile is already enabled
*@return     errorAlreadyDisabled (10), -- Emergency/Test Profile is already disabled
*@return     errorForbidden (12), -- Transition is forbidden for the Test Profile
*@return     undefinedError (127):
*
*@author     :chentao
*
*@history    :V1.0 :create @chentoa20220928
*/
#if M2M_ESx
BYTE M2MLocalEnvelopCommand(void)
{
	BYTE idata result;

    if(RecData[1] != 0x01 ) //len = 1
    {
    	return 0x7F;
    }
    switch(RecData[2])
    {
    	case 0: //Local Enable of Emergency Profile
    		result = M2MLocalEnableEmergencyProfile();
        	break;
        case 1: //Local Disable of Emergency Profile
        	result = M2MLocalDisableEmergencyProfile();
        	 break;
        case 2: //Local Enable of Test Profile
        	result = M2MLocalEnableTestProfile();
        	break;
        case 3: //Local Disable of Test Profile
        	result = M2MLocalDisableTestProfile();
              break;
        default:
        	break;
    }

	return result;

}
#endif
#endif  //end of ifndef EUICC_RSP
