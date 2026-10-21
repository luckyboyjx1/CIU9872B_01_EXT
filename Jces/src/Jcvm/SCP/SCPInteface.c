/*
 * SCPInteface.c
 *
 *  Created on: 2013-9-13
 *      Author: jiangxiaohui
 */


#include "scp01.h"
#include "scp02.h"
#include "SCP03.h"
#include "scp10.h"
#include "SCP11.h"
#include "error.h"
#include "profile.h"
#include "opFunction.h"
#include "CardManager.h"
#include "SCPDealData.h"
#include "SCPInterface.h"
#include "SCPParamCheck.h"
#include "global_variable.h"
#include "SCPGenKeyInterface.h"
#include "object.h"
#include "ob_manager.h"
#include "SecurityDomainData.h"
#include "keySet.h"
#include "keyGroup.h"
#include "tagNode.h"
#include "KeyLogicManager.h"
#include "CgmManager.h"
#include "SecurityDomainData.h"
#include "Interface_PlatformFromLTE.h"
#include "opFunction.h"
#include "Interface_LTEFromPlatform.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

/*******************************************************************************
 * This method opens a Secure Channel for an applet and returns the newly opened channel number.
 * The supplied APDU must contain the command to authenticate the card.
 * This method also prepares the response to this command.
 * The Security Domain that the applet belongs to is responsible for the channel number allocation.
 * The channel '0' is reserved, and channels 1 through 127 are available for the applet's use.
 *
 * @return void
 * @param
 * @exception javacard.framework.CardRuntimeException
 * This exception is thrown if there is no available channel or if the APDU command is invalid.
 ******************************************************************************/


#define		SCP81_DISPATCHER_TO_OTHER_SD		0x00
#define		SCP81_DISPATCHER_TO_ITSELF			0x01
#define     SCP81_DISPATCHER        	     	ISD_SECURITY_LEVEL

u8 g_SCP02SDID;
u8 scpLastMAC[SCP_LASTMAC_LENGTH];
u8 scpCurrentMAC[SCP_CURRENTMAC_LENGTH];
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC) //modify by jiangxiaohui@20220324 分层
u8 scpBackMAC[SCP_BACKMAC_LENGTH];
#endif

u8 CONST  AuthCounterNoChangesdPackageAID[] = {0xA0,0x00,0x00,0x05,0x33,0xC0,0x00,0xFF,0x86,0x00,0x00,0x00,0x04,0x28};//added by xuxiaojun@20220628 用于SCP02和SCP03共同支持伪counter. 与SCP02的伪counter方案一致

void initScpInterface(void)
{
	g_SCP02SDID		= 0xFF;
}

void RAMRemoteInit(void)
{
	if(g_SCP02SDID != 0xFF){
		g_SCP02SDID = 0xFF;
		channelInfo.security_level[selected_channel] = 0;
	}
}
#if GP__SCP
void scpClr(void)
{
	g_SCP_appRmac = 0;
	g_SCP_appEnc = 0;
	g_SCP_appREnc = 0;
}
/**
* modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
*/
u16 openSecureChannel(u8 APDUDataFieldIsReceived)
{
	u16		X_A		checkResult;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK	
	u16		X_A		authWarning;
#endif
	PN8				keySetObj;
	//在上层远程APDU处理检查
#if JAVA_CARD_UICC	// modify by xuxiaojun@20220318 分层
	//jiangqingbin-20221025，434缺陷的补充修改, 移到以下判断之前是因为在用例(移动5G内测脚本/LOG/TelecomNetWorkTest01)中, 需要把本函数中判断和赋值提前，才能满足该用例的情况，即自关联ssd1的http会话，操作isd以及isd下的ssd2
	IF_AnnexB_SupportNewCommandSession();//http 特殊判断, 把http真正需要鉴权的sdid赋值给G_RAMSessionSdId

	if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])&&(G_RAMSessionSdId == 0xFF) )//RAM APDU不需要进行安全会话, 却包含了Secure Channel命令，RAM指令报错
//	if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) && (SCP81_DISPATCHER_TO_ITSELF == bRamBuffer[SCP81_DISPATCHER]))
	{
		return SW_INS_NOT_SUPPORTED;
	}
#endif	
	/* first close the security channel; */
	closeSecuritychannel(SC_TERMINATED);// modify by liweimin

	/* second check P1\P2\LC  */
	check_p1p2Lc_param( APDUBuffer[OFFSET_INS] );
	checkResult = checkIsSDAcitveOnOtherLogicChannel();
	if(checkResult != SW_NO_ERROR)
	{
		return checkResult;
	}
	/* check  current ssd is locked*/
	getCurrentSdObj(IS_NEED_CHECK_SD_LOCKED);
	
	/* third get the SDId, and then get the key version; */
	keySetObj = MEMtPN8(InitAndCheckSCPParam());

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK	
	/* fourth check sequeceCount and tryCount; */
	authWarning = checkSequenceCount(keySetObj);
#endif

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	checkTryAuth(keySetObj);
#endif

	checkIsSDAuthOnOtherLogicChannel();
	
	/* fifthly check parameter i we supported  */
	checkResult = check_scp_i_param(CHECK_IS_SUPPORT_SCP_AND_PARAM_I);
	if( SW_NO_ERROR != checkResult )
	{
		return checkResult;
	}

	checkKeyUsageQualifier(keySetObj);

	/* sixth receive the APDU DataField if not received before ; */
	if((apdu_IsSetIncoming() == 0)&&(APDU_DATAFIELD_IS_NOT_RECEIVED == APDUDataFieldIsReceived) && (bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA ))// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
	{
		// receive the DATA
		apdu_ApduSetIncomingAndReceive();
	}
#if GP__AMENDMENT__D
	if(g_current_SCP_Type == SCP_03)//附录D 6.2.2.1 Card Challenge  ：Whenever a challenge generation is triggered by an INITIALIZE UPDATE command, the sequence counter shall be incremented and the new value shall be used in the calculation described below. When the maximum value is reached, the INITIALIZE UPDATE command shall be rejected with “conditions of use not satisfied”.
	{
		incrementSequenceCounter();

		checkSequenceCount(keySetObj);
	}
#endif

	/* seventh copy the hostChallenge  */
	//ecpMem_Copy(g_current_SCP_hostChallenge, (PX8) (PX8_A_U16(APDUBuffer, OFFSET_CDATA)), HOST_CHALLENGE_LENGTH);
	memCopyNoAtomic(MEMtPM8(g_current_SCP_hostChallenge), PX8_A_U16(APDUBuffer, OFFSET_CDATA), HOST_CHALLENGE_LENGTH);
	/* eighth generate the CardChallenge  */
	generateCardChallenge(MEMtPN8(keySetObj), FIRST_COME_GENERATE_CARD_CHALLENGE);
	memRefresh();
	/* ninth generate all session key  */
	generateSCPKey(keySetObj);

	if((g_current_SCP_Type == SCP_02)&&(g_current_SCP_seqCounter == 0x7FFF)) //GP功能补充测试_20150507_明文版/卡片废止 /送检时需要提供counter值已到0x7FF5的卡片进行测试,返回6985
	{
		return SW_CONDITIONS_NOT_SATISFIED; //E.1.2 : When reaching its maximum value, the Sequence Counter shall not be reset to zero. Cards are not required to support counter values beyond 32767.
	}

	/* generate the CardChallenge if SCP02 and i_option is PSEURAND */
	generateCardChallenge(keySetObj, SECOND_COME_GENERATE_CARD_CHALLENGE);
    
	memRefresh();
	/* tenth generate the CardCryptogram  */
	//option = 0 : copy CardCryptogram to apdubuffer, option = 1 : compare HostCryptogram
 	generateOrCompareCardCryptogram(GENERATE_CARD_CRYPRTOGRAM);

	// eleventh group all the response data
	if(NOK == groupApduResponseOfOpenChannel(keySetObj) )
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	//twelfth generate Secure channel Number
	generateSecureChannelNumber();

	// thirteenth set status that openChannel is complete
	managerChannelLevel(SCF_NO_AUTHENTICATED);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK	
	if(MAX_AUTHENTICATION_WARNING == authWarning)
		return (u16)MAX_AUTHENTICATION_WARNING;
#endif	
	return SW_NO_ERROR;
}

u8 groupApduResponseOfOpenChannel(PN8 keySetObj)
{
#define 	DIVERSIFIED_DATA_LENGTH				0x0A
	u8 		X_A		result;
	u32 	X_A		sequence_counter;
	PN8 			seqCounterField;
	PX8		 		temPtr;

	jref			tagArrayRef;
	jref 			firstTagNodeRef;
	PN8				tagArrayAField;
	PN8				sdDataObj;

	result = OK;

	temPtr = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
//	if(CONFIG_VGP) //modify by xuxiaojun@20220426 已经统一在store data区域的cf统一处理了。
//	{
//		memCopyToRam(temPtr, MEMtPM8(get_array_data(MEMtPF8(locateObjectInMemory(PN_R_U16(OP_bDiversifiedDataArr_JC))))), U32t16( DIVERSIFIED_DATA_LENGTH));// The 10-byte value of tag 'CF' from the Security Domain’s Data Store shall be returned as Key Derivation Data.
//	}
//	else
	{

		sdDataObj = MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD));
		firstTagNodeRef		= sdData_getFirstTagNode(sdDataObj);

		if (firstTagNodeRef == REF_NULL) {
			memFillToRam(temPtr, 0x00, 0x0A);
		} else {
			tagArrayRef			= tagNode_getTagArray(firstTagNodeRef, DIVERSIFIEDATA, 0, 0, MEM_NULL,false);
			if(REF_NULL == tagArrayRef)
			{
				throw_error(SW_REFERENCED_DATA_NOT_FOUND);
			}
			tagArrayAField		= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tagArrayRef))));
			memCopyToRam(temPtr, MEMtPM8(tagNode_getTagArrayContent(tagArrayAField)), 0x0A);

		}
	}


	// Place the Key Information Data into the apdu buffer

	APDUBuffer[15] = currKeySetVer[selected_channel]; // key version
	APDUBuffer[16] = g_current_SCP_Type; //scpProtocol #01 or #02 or #03

	if(SCP_03 == g_current_SCP_Type)
	{
		APDUBuffer[17] = g_current_SCP_I_Option;//findTag(destAddr, 0x64);//i parameter(R_MAC_ENC_PSEU)
		if(APDUBuffer[17] == 1)
		{
			result = NOK;
		}
		else
		{
			//ecpMem_Copy((PX8) (PX8_A_U16(APDUBuffer , 18)), g_current_SCP_cardChallenge, CARD_CHALLENGE_LENGTH);
			memCopyNoAtomic(PX8_A_U16(APDUBuffer , 18), MEMtPM8(g_current_SCP_cardChallenge), CARD_CHALLENGE_LENGTH);
			if( U8_AND_U8(g_current_SCP_I_Option, SC03_PSE_RAND_CARD_CHALL) )
			{
				seqCounterField = MEMtPN8(keySet_getSequenceCounter(keySetObj, SCP03_SEQUENCE_CONTER_SIZE));
				sequence_counter = readScp03Counter(seqCounterField);//PN_R_U24(seqCounterField);
				//added by xuxiaojun@20220628 用于SCP02和SCP03共同支持伪counter. 与SCP02的伪counter方案一致
				if( (JcvmConfig_GetSsdAuthCounter()  != 0xFF) && (SC03_PSE_RAND_CARD_CHALL == ( g_current_SCP_I_Option & SC03_PSE_RAND_CARD_CHALL )) && (g_bSDId[selected_channel] != 0)/*|| bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA*/ )
				{
					if(AID_partialEquals_array(getAppletAid(getIndexByID(g_bSDId[selected_channel])), (PF8)AuthCounterNoChangesdPackageAID, 0, sizeof(AuthCounterNoChangesdPackageAID)) == true)
					{
						sequence_counter = JcvmConfig_GetSsdAuthCounter();
					}
				}
				APDUBuffer[34] = U32t8(U32_SHR_U8(sequence_counter, 16));
				APDUBuffer[35] = U32t8(U32_SHR_U8(sequence_counter, 8));
				APDUBuffer[36] = U32t8(U32_AND_U32(sequence_counter, 0xff));
				APDUBuffer[OFFSET_LC] = 32;
			}
			else
			{
				APDUBuffer[OFFSET_LC] = 29;
			}
		}
	}
	else
	{
	//	ecpMem_Copy((PX8) (PX8_A_U16(APDUBuffer , 17)), g_current_SCP_cardChallenge, CARD_CHALLENGE_LENGTH);
		memCopyToRam(PX8_A_U16(APDUBuffer , 17), MEMtPM8(g_current_SCP_cardChallenge), CARD_CHALLENGE_LENGTH);
		// Set the outgoing length
		APDUBuffer[OFFSET_LC] = 28;

	}
	return result;
}

/******************************************************************************
 * This method is used to authenticate the host by verifying the contents of an APDU command.
 *
 * @param channel byte
 * @param
 * @exception javacard.framework.CardRuntimeException.
 * This exception is thrown if the authentication fails.
 * modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
 *****************************************************************************/
u16 verifyExternalAuthenticate(u8 APDUDataFieldIsReceived)
{
	u8 		X_A		p1; // Retrieve the encryption type
	u16 	X_A		checkResult;

	u8 		X_A		securityLevel;
	u8 		X_A		bserrity_channel_number;
	u8 		X_A		len;
	p1 = APDUBuffer[OFFSET_P1];
	//在上层远程APDU处理检查
	/*if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]))
	//if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) && (SCP81_DISPATCHER_TO_ITSELF == bRamBuffer[SCP81_DISPATCHER]))
	{
		return SW_INS_NOT_SUPPORTED;
	}*/
	checkResult = checkIsSDAcitveOnOtherLogicChannel();
	if(checkResult != SW_NO_ERROR)
	{
		throw_error(checkResult);
	}

	/* first if P1\P2\LC are not checked, then check the P1\P2\LC  */
	check_p1p2Lc_param(APDUBuffer[OFFSET_INS]);

	securityLevel = channelInfo.security_level[selected_channel];
	bserrity_channel_number = channelInfo.sercrity_channel_number[selected_channel];
	if( (0x00 == bserrity_channel_number) || (SCF_NO_AUTHENTICATED != securityLevel) )
	{
		closeSecuritychannel(SC_TERMINATED);// modify by liweimin
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}

	/* second receive the APDU DataField if not received before ; */
	//if(APDU_DATAFIELD_IS_NOT_RECEIVED == APDUDataFieldIsReceived )
	//modify chentao
	if((apdu_IsSetIncoming() == 0)&&(APDU_DATAFIELD_IS_NOT_RECEIVED == APDUDataFieldIsReceived) && (bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA))// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
	{
		// receive the DATA
		apdu_ApduSetIncomingAndReceive();
	}
	len = U8_S_U8(APDUBuffer[OFFSET_LC], MACLENGTH_SUB_APDUHEADSIZE);
    //ecpMem_Copy(g_current_SCP_LAST_MAC_L, (PX8) (PX8_A_U8(APDUBuffer , len)), MAC_LENGTH);
#if GP__AMENDMENT__D
	if(g_current_SCP_Type == SCP_03)
	{
		memFillNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC),0x00,LAST_MAC_LENGTH);
		ecpMem_Set(g_SCP_ICV_Counter,0,SCP03_ICV_COUNTER_LENGTH);
	}
	else
#endif
	{
		memCopyNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC_L),(PX8_A_U8(APDUBuffer , len)),MAC_LENGTH);
	}
	
	//ecpMem_Copy(g_current_SCP_CURRENT_MAC, g_current_SCP_LAST_MAC_L, MAC_LENGTH);
	memCopyNoAtomic(MEMtPM8(g_current_SCP_CURRENT_MAC), MEMtPM8(g_current_SCP_LAST_MAC_L), MAC_LENGTH);
	/*	third verifySCPMAC	*/
	if (NOK == verifyMAC(APDUBuffer) )
	{	
		return SW_SECURITY_STATUS_NOT_SATISFIED;
	}

	/*	forth Compare HostCryptogram	*/
	//option = 0 : copy CardCryptogram to apdubuffer, option = 1 : compare HostCryptogram
	if ( NOK == generateOrCompareCardCryptogram(COMPARE_CARD_CRYPRTOGRAM) )
	{
		return AUTHENTICATION_FAILED;
	}

	/*	fifth Setup Rmac	*/
	checkResult = check_scp_i_param(CHECK_IS_SUPPORT_RMAC);

	if( SW_NO_ERROR != checkResult )
	{
		return checkResult;
	}

	if (U8_AND_U8(p1, SCF_R_MAC_FLAG))
	{
		p1 = U8_OR_U8(p1, SCF_ZERO_APDU_RMAC_FLAG); //bit 7 indicates secure channel setup
		g_SCP_rmacTemp[0] = 0x00;  //setup rmac
	}

	/*	sixth change channel security level	*/
	securityLevel =  U8_OR_U8(p1, SCF_AUTHENTICATED );
	bRamBuffer[SD_REQUESTED_SECURITY_LEVEL] = securityLevel;
	managerChannelLevel(securityLevel);
    managercompulsorySessionSecurityLevel(securityLevel) ;

#if GP__AMENDMENT__D
	if(g_current_SCP_Type != SCP_03) // 附录D 6.2.2.1 Card Challenge  ， SCP03时 在8050 增加counter
#endif
	{
		/*	the end, increment the Sequence Counter	*/
		incrementSequenceCounter();
	} 
#if JAVA_CARD_UICC		 // modify by xuxiaojun@20220318 分层
	if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])){
//		g_SCP02SDID = getCurrentSDId();
		g_SCP02SDID = G_RAMSessionSdId;
	}
#endif
	return checkResult;
}

/*****************************************************************************
 * increment Sequence counter

 * input : key version

 * find the key version ,and increment the sequnce counter of the current key set

 * modify by xxj@20230512: 修正缺陷487时，做对应的调整
 *****************************************************************************/
void incrementSequenceCounter()
{
		
	PN8				sdDataObj;
	PN8				keySetObj;
	u8              tmpKeyVersion;

	sdDataObj = MEMtPN8(getCurrentSdDataObj(U8_OR_U8(IS_NO_NEED_CHECK_SD, IS_NEED_CHECK_SD_KEY_INTERGRATED), &tmpKeyVersion));

	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, currKeySetVer[selected_channel]));
	keySet_setTryAuthAttenpt(keySetObj, 0x00);
	switch(g_current_SCP_Type)
	{
			case SCP_02:
					keySet_incSequenceCounter(keySetObj, SCP02_SEQUENCE_CONTER_SIZE);
			break;
			
			case SCP_03:
					keySet_incSequenceCounter(keySetObj, SCP03_SEQUENCE_CONTER_SIZE);
			break;
			
			default:
			break;
	}
	
}


#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC ||GP__SCP__SCP03_RMAC)
/*
 *
 *
 * */
u16 operateBeginRmacSession()
{
		u16 	X_A		errorCode;
		/* fifthly check parameter i we supported  */
		if (0x00 != U8_AND_U8(g_SCP_BeginEnd_R_MAC[selected_channel], BEGIN_RMAC_ISSUED_MASK))
		{
			channelInfo.security_level[selected_channel] = U8_AND_U8(channelInfo.security_level[selected_channel], U8_INV(SCF_R_MAC_FLAG)); //clear the RMAC security level, re-build the RMAC security level
		}
		else
		{
			// when first send begin-rmac command, cournter inc
			U16_INC(g_SCP_SequnceCount[selected_channel]);
		}
		errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);

		if (SW_NO_ERROR != errorCode)
		{
			//it close the Security channel before , so need not recover the RMAC.
			memCopyNoAtomic(MEMtPM8(g_current_SCP_CURRENT_MAC), MEMtPM8(g_current_SCP_LAST_MAC_L), U32t8(8)); // store the correct C-MAC
			return errorCode;
		}

		errorCode = check_scp_i_param(CHECK_IS_SUPPORT_RMAC);
		if( SW_NO_ERROR != errorCode )
		{
			return errorCode;
		}

		check_p1p2Lc_param(INS_OP_BEGIN_RMAC_SESSION);

	#if GP__AMENDMENT__D
		if ( SCP_03 == g_current_SCP_Type )
		{
			errorCode = check_security_level();
			if( SW_NO_ERROR != errorCode )
			{
				return errorCode;
			}
		}
		else
	#endif
		{
			if (U8_AND_U8(channelInfo.security_level[selected_channel], SCF_R_MAC_FLAG))
			{
				memCopyNoAtomic(MEMtPM8(g_current_SCP_CURRENT_MAC), MEMtPM8(g_current_SCP_BACK_MAC), MAC_LENGTH);
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}

		BeginRmacGenKeyScp02();
		 // initialize
		g_SCP_BeginEnd_R_MAC[selected_channel] = BEGIN_RMAC_ISSUED_MASK;

		if (0x10 == APDUBuffer[OFFSET_P1])
		{
			g_SCP_BeginEnd_R_MAC[selected_channel] = U8_OR_U8(g_SCP_BeginEnd_R_MAC[selected_channel], ALL_RESPONSE_WITH_RMAC_MASK);   // all return rmac
			channelInfo.security_level[selected_channel] = U8_OR_U8(channelInfo.security_level[selected_channel], SCF_R_MAC_FLAG);
		}

		g_SCP_rmacTemp[0] = 0; //setup rmac

		memCopyNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC_L), MEMtPM8(g_current_SCP_CURRENT_MAC), MAC_LENGTH); // store the last C-MAC
		//channelInfo.security_level[selected_channel] |= SCF_R_MAC_FLAG | SCF_ZERO_APDU_RMAC_FLAG; //bit 7 indicates secure channel setup
		channelInfo.security_level[selected_channel] = U8_OR_U8(channelInfo.security_level[selected_channel], SCF_ZERO_APDU_RMAC_FLAG); //bit 7 indicates secure channel setup
		return errorCode;

}


/*
 *
 *
 * */
u16 operateEndRmacSession()
{

		u16 	X_A		errorCode;

		/*if (((channelInfo.security_level[selected_channel] & R_MAC) == 0))
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
		else */
		channelInfo.security_level[selected_channel] = U8_OR_U8(channelInfo.security_level[selected_channel], SCF_R_MAC_FLAG);
		check_p1p2Lc_param(INS_OP_END_RMAC_SESSION);

		errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		if (errorCode != SW_NO_ERROR)
		{
 			memCopyNoAtomic(MEMtPM8(g_current_SCP_CURRENT_MAC), MEMtPM8(g_current_SCP_LAST_MAC_L), MAC_LENGTH); // store the correct C-MAC
			return errorCode;
		}

		if (0x00 == U8_AND_U8(g_SCP_BeginEnd_R_MAC[selected_channel], BEGIN_RMAC_ISSUED_MASK) ) // initial security session with Rmac using externalAuthentication command
		{
			//g_BeginEnd_R_MAC[selected_channel] = (BEGIN_RMAC_ISSUED_MASK | ALL_RESPONSE_WITH_RMAC_MASK);
			return SW_CONDITIONS_NOT_SATISFIED;
		}



		return errorCode;
}
#endif
/**
 * This method is used to close the SecureChannel that was previously opened with
 * openSecureChannel() method by the calling applet.
 * @param channel byte
 *
 * (Note) This version of isdCloseSecureChannel is used by SecurityDomain
 * merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support SCP11
 * modify by xuxiaojun@20220527 修改g_SCP11SessionPara的清除条件，由于SCP11是在8082时才执行closeSecuritychannel，此时其上一条指令802A执行的结构是有效的，不能清掉(g_SCP11SessionPara),测试用例   AmendH测试脚本-SCP11c: Annex_B_scenario_01_AM_with_autoRestore_bb_01_02
 */
void closeSecuritychannel(u8 isAborted)// modify by liweimin
{
	if (U8_AND_U8(channelInfo.security_level[selected_channel], SCF_R_MAC_FLAG))
	{
		//bRamBuffer[ISD_SECURITY_LEVEL] = R_MAC_FLAG;
		channelInfo.security_level[selected_channel] = SCF_ZERO_APDU_RMAC_FLAG;
	}
	else
	{
		channelInfo.security_level[selected_channel] = 0;
	}

#if  GP__SCP__SCP11__FOR_MASK
	if((PX_R_U8(g_current_SCP11SessionPara_sessionType) == SCP11_C))
	{
		if (isInProgress())
		{
			commit_transaction();
		}
		ecpMem_Set(g_current_SCP11SessionPara, 0x00, SCP11SessionPara_LENGTH); // todo, 是否应该直接调用SCP11PerformSecurityOperation_InitGlobalVar?
	}
#endif

// 	memFillNoAtomic(MEMtPM8(g_current_SCP_SES_KEY), 0x5A, MAX_DES_KEY_LENGTH);
// 	memFillNoAtomic(MEMtPM8(g_current_SCP_MAC_KEY), 0x5A, MAX_DES_KEY_LENGTH);
// 	memFillNoAtomic(MEMtPM8(g_current_SCP_RMAC_KEY), 0x5A, MAX_DES_KEY_LENGTH);
// 	memFillNoAtomic(MEMtPM8(g_current_SCP_DEK_KEY), 0x5A, MAX_DES_KEY_LENGTH);

	channelInfo.security_level[selected_channel] = 0;
	channelInfo.sercrity_channel_number[selected_channel] = 0;
	channelInfo.selected_key_appId[selected_channel] = APP_NULL;
	if(SC_TERMINATED == isAborted )
	{
		managercompulsorySessionSecurityLevel(SCF_NO_AUTHENTICATED) ;
	}
	if(U8_AND_U8(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL], U8_SHL_U8(1, selected_channel)))
	{
		bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = U8_AND_U8(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL], U8_INV(U8_SHL_U8(1, selected_channel)));
		bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
	}
	if (U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_SHL_U8(1, selected_channel)))
	{
		bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
		bRamBuffer[IS_RESTORE] = 1;
	}
	if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])){
		g_SCP02SDID = U16t8(S16tU16(-1));
	}
}

//#if JCVM__IO__SWP
//void SWP_closeSecuritychannel(void)
//{
//	u8 i;

//	for (i=NFC_BASIC_CHANNEL_NUM; i<U8_A_U8(NFC_MAX_LOGICAL_CHANNELS, NFC_BASIC_CHANNEL_NUM); U8_INC(i))
//	{
//		if (U8_AND_U8(channelInfo.security_level[i], SCF_R_MAC_FLAG))
//		{
//			channelInfo.security_level[i] = SCF_ZERO_APDU_RMAC_FLAG;
//		}

//		channelInfo.security_level[i] = 0;
//		channelInfo.sercrity_channel_number[i] = 0;
//		bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
//		if (U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_SHL_U8(1, i)))
//		{
//			bRamBuffer[INSTALL_FOR_LOAD] =U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD],  U8_INV(U8_SHL_U8(1, i)));
//			bRamBuffer[IS_RESTORE] = 1;
//		}
//	}
//}
//#endif

u16 checkIsAuthenticated(u8 ignoreAuth)
{
	u16 	X_A		result;
	u8 		X_A		bSecureLevel;
	u8 		X_A		needDecryptCommandData;
	u8 		X_A		needVerifyMAC;
	u8 		X_A		tempI;
#if JAVA_CARD_UICC
	u8 				SDId;

	//u8 bCurrentState = 0;
	SDId 			= 0;
#endif
	result 			= SW_NO_ERROR;
	bSecureLevel 	= channelInfo.security_level[selected_channel];
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)//解析profile过程中拼装的指令 不校验安全级别
	{
		return result;
	}
#endif

#if JAVA_CARD_UICC
	SDId = getCurrentSDId();
#endif

	//add by linguomin for cms2ac pin
	if ((apdu_IsSetIncoming() == 0)&&(bSecureLevel == SCF_NO_AUTHENTICATED) && (ignoreAuth == IGNORE_AUTHENTICATE))// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
	{
		apdu_ApduSetIncomingAndReceive();
		return result;
	}

#if JAVA_CARD_UICC	 // modify by xuxiaojun@20220318 分层
	if( bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA)
	{
		//if ((g_SCP02SDID != SDId) && (U8_AND_U8(APDUBuffer[OFFSET_CLA], 0x04) == 0x04))
		if((G_RAMSessionSdId == 0xFF)&& (U8_AND_U8(APDUBuffer[OFFSET_CLA], 0x04) == 0x04))
		{
			result = SW_CONDITIONS_NOT_SATISFIED;
			return result;
		}
		//	需要SCP02、03鉴权 但是没有鉴权 安全级别只有SCP80/无安全级别
		if((G_RAMSessionSdId != 0xFF)&&((bSecureLevel == SCF_80_AUTHENTICATED)||(U8_AND_U8(bSecureLevel,SCF_C_MAC_DECRYPTION) < SCF_C_MAC))) //SCP02/03的安全级别  至少要校验CMAC SCF_C_MAC
		{
			IF_SetHttpScriptState();
			result = SW_CONDITIONS_NOT_SATISFIED;
			return result;
		}

		while(SDId != 0xFF)
		{
			//创建SCP80 的SSD与执行命令SSD为关联SSD
			if(bRamBuffer[REMOTE_SCP_SDID] == SDId)
			{
				managerChannelLevel(SCF_80_AUTHENTICATED);
				return result;
			}

			//当前触发的SD有建立scp02安全信道
			if ((g_SCP02SDID == SDId))
			{
				//managerChannelLevel(g_SDSecurity_level[selected_channel]);
				break;
			}
			SDId	= getAssociateSDId(SDId);
		}

		if(SDId == 0xFF)
		{
			closeSecuritychannel(SC_TERMINATED);// modify by liweimin

			IF_SetHttpScriptState();

			return U32t16(0x6001); //SCP80/81 SD 不与target SD 的关联
		}
	}
#endif
	if(RESULT_OK == checkSecureLevel(APDUBuffer, bSecureLevel, &needDecryptCommandData, &needVerifyMAC))
	{
		if((apdu_IsSetIncoming() == 0)&&(bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA))// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
		{
			apdu_ApduSetIncomingAndReceive();
		}
		//backup MAC for begin-end RMAC
		if (INS_OP_END_RMAC_SESSION != APDUBuffer[OFFSET_INS])
		{
//			ecpMem_Copy((PX8)&backMac[(selected_channel << 3)], g_current_SCP_RMAC_KEY, MAC_LENGTH); // store the last C-MAC
			if((SCP_02 == g_current_SCP_Type) || (SCP_11 == g_current_SCP_Type)) // modify by xuxiaojun@20220524 增加SCP11安全通道下指令的MAC校验
			{
				//ecpMem_Copy(g_current_SCP_BACK_MAC, g_current_SCP_CURRENT_MAC, MAC_LENGTH); // store the last C-MAC
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)//modify by jiangxiaohui@20220324 分层
				memCopyNoAtomic(MEMtPM8(g_current_SCP_BACK_MAC), MEMtPM8(g_current_SCP_CURRENT_MAC), MAC_LENGTH);
#endif
				tempI = U8_S_U8(APDUBuffer[OFFSET_LC] , 0x03);
				//ecpMem_Copy(g_current_SCP_CURRENT_MAC, (PX8)PX8_A_U16(APDUBuffer, tempI), MAC_LENGTH); // save this CMAC of this command
				memCopyNoAtomic(MEMtPM8(g_current_SCP_CURRENT_MAC),PX8_A_U16(APDUBuffer, tempI),MAC_LENGTH);
				
				//memRefresh();
			}
		}
		else //if (INS_OP_END_RMAC_SESSION == APDUBuffer[OFFSET_INS])
		{
			if ((0x00 == U8_AND_U8(g_SCP_BeginEnd_R_MAC[selected_channel], ALL_RESPONSE_WITH_RMAC_MASK))
												&& (U8_AND_U8(g_SCP_BeginEnd_R_MAC[selected_channel], BEGIN_RMAC_ISSUED_MASK)))
			{
				memCopyNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC_L), MEMtPM8(g_current_SCP_CURRENT_MAC), MAC_LENGTH); // use the last C-MAC as the ICV for Rmac caculation
			}
		}

		if((bSecureLevel != SCF_AUTHENTICATED) && (bSecureLevel != SCF_NO_AUTHENTICATED))
		{
			result = decryptAndVerifyMAC(APDUBuffer, bSecureLevel, needDecryptCommandData, needVerifyMAC);
		}
	}
	else if (0x00 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_80)) //ISO7816 command
	{
			if((apdu_IsSetIncoming() == 0)&&(bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA))// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
			{
				apdu_ApduSetIncomingAndReceive();
			}
			g_SCP_appEnc = 1;//设置该值后 不需要算RMAC
	}

	else
	{
			if(bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA)
			{
				closeSecuritychannel(SC_TERMINATED);// modify by liweimin
			#if ( GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
				result = SW_SECURITY_STATUS_NOT_SATISFIED;
			#else
				result = SW_CONDITIONS_NOT_SATISFIED;
			#endif
			}
#if JAVA_CARD_UICC
			else
			{
				//add by linguomin for httptest
				//closeSecuritychannel(SC_TERMINATED);
				//result = (short)0x6001; //通过ISOException 6001 表示远程方式的SCP02等鉴权不满足

				//临时处理: 通过当前的SCP key区分SCP80/SCP81
				if(IsSCP80Key(bRamBuffer[REMOTE_SCP_KEY_VERSION]))
					result = SW_CONDITIONS_NOT_SATISFIED;
				else
				{
					result = 0x6001; //通过ISOException 6001 表示远程方式的SCP02等鉴权不满足

					IF_SetHttpScriptState();
				}
				closeSecuritychannel(SC_TERMINATED);// modify by liweimin

				return result;
			}
#endif
	}

	return result;
	//return SW_NO_ERROR;
}


#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC || GP__SCP__SCP11__FOR_MASK)
u16 GenRMACOfResponse(PX8 SrcAddr ,u16 length, u16 sw, boolean isSCPAPI)
{
	switch(g_current_SCP_Type)
	{
		case SCP_02:
			if( false == isSCPAPI)
			{
				length = U8t16(dealSCP0102DataRMac(SrcAddr, length, BEFORE_MAC_CAL, U16t8(U16_SHR_U8( sw, 8 )), U32t8(sw)));
				CalMACSCP02(MEMtPX8(TEMPBuffer), g_SCP_rmacTemp[0], MEMtPX8(g_current_SCP_RMAC_KEY), g_current_SCP_LAST_MAC_L);
				length = U8t16(dealSCP0102DataRMac(SrcAddr, length, AFTER_MAC_CAL, 0x00, 0x00));
			}
			else
			{
				CalMACSCP02(SrcAddr, length, MEMtPX8(g_current_SCP_RMAC_KEY), g_current_SCP_LAST_MAC_L);
			}
			break;
#if GP__AMENDMENT__D
		case SCP_03:
		case SCP_11:
			length = SCP03_dealRMacData(SrcAddr, length, BEFORE_MAC_CAL, (u8)( sw >> 8 ), (u8)sw);
			SCP03_CalMAC(SrcAddr, length, MEMtPM8(g_current_SCP_RMAC_KEY), g_SCP_rmacKeyLen);
			length = SCP03_dealRMacData(SrcAddr, length, AFTER_MAC_CAL, 0x00, 0x00);
			break;
#endif //GP__AMENDMENT__D
#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			length = scp10_dealRMacData(SrcAddr, length, BEFORE_MAC_CAL, (u8)( sw >> 8 ), (u8)sw);
			CalMACSCP10(SrcAddr, length, g_current_SCP_RMAC_KEY, RMAC_CAL);
			length = scp10_dealRMacData(SrcAddr, length, AFTER_MAC_CAL, 0x00, 0x00);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}

	return length;
}

u16 EncryptAndGenRMAC(PX8 srcAddress, u16 length, u16 status, boolean isSCPAPI)
{
	u8 securityLevel;

	if(((status != 0x9000)&&((status&0xFF00) != 0x6200)&&((status&0xFF00) != 0x6300)&&((status&0xFF00) != 0x6100)))//GP 附录D 6.2.5 All status words except '9000' and warning status words (i.e. '62xx' and '63xx') shall be interpreted as error status words. 测试用例：AmendH测试脚本-SCP11c：  DELETE_error_SD_related_to_elf_in_current_session_00_06_02(错误个数: 4)
	{
		return length; //错误状态字不算rmac和不加密
	}

	securityLevel = channelInfo.security_level[selected_channel];

	switch(g_current_SCP_Type)
	{
		case SCP_02:
			break;
#if GP__AMENDMENT__D
		case SCP_03:
		case SCP_11:
			if(SCF_ENCRYPTION_FLAG == (SCF_ENCRYPTION_FLAG & securityLevel) )
			{
				length = SCP03_EncryptOfResponse(srcAddress ,length);
			}
		break;
#endif
	#if GP__SCP__SCP10__FOR_MASK
		case SCP_10 :
			if(SCF_ENCRYPTION_FLAG == (SCF_ENCRYPTION_FLAG & securityLevel) )
			{
				length = EncryptOfResponseSCP10(securityLevel, srcAddress ,length, status);
				g_SCP_appREnc = 1;
			}
			break;
	#endif
	}

	if(SCF_R_MAC_FLAG == U8_AND_U8( SCF_R_MAC_FLAG, securityLevel) )
	{
		length = GenRMACOfResponse(srcAddress, length, status, isSCPAPI);
		//restore apdubuffer data
	}

	return length;
}

/*
* merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
*/
void EncryRMACResponseData(u16 status)
{
	ppApdu_Response_t	response;
	u8 					securityLevel;
	PX8					resBuffer;
	u16					resLen;
	//C0FC0000080011223344556677,C0F7001106010000000000,C0F7001106020000000000,收到CLA=C0的指令，根据7816-4解析得到通道号=4，当卡片支持的最大通道号为4时 会抛异常6881，发状态字前调用该函数时通道号为4，超出卡片通道数 ，channelInfo.security_level[selected_channel]
	//溢出，访问的是channelInfo.security_level_invert[]，判断符合RMAC条件，将状态字变为6C08 ，后续指令执行时会备份逻辑通道数据，将通道号4（最大通道号=3）的数据写入E2（超出开辟的备份区范围）
	if(((status != 0x9000)&&((status&0xFF00) != 0x6200)&&((status&0xFF00) != 0x6300)&&((status&0xFF00) != 0x6100)))//GP 附录D 6.2.5 All status words except '9000' and warning status words (i.e. '62xx' and '63xx') shall be interpreted as error status words.
	{
		return ; //错误状态字不算rmac和不加密
	}
	
	if((SCF_R_MAC_FLAG == U8_AND_U8( SCF_R_MAC_FLAG, channelInfo.security_level[selected_channel]) )||(true == g_SCP_appRmac))
	{
		apdu_GetResponse(&response);
		resBuffer		= response->pData;
		securityLevel 	= channelInfo.security_level[selected_channel];
		resLen			= apdu_GetResLen();

		if( SCF_ZERO_APDU_RMAC_FLAG == U8_AND_U8(SCF_ZERO_APDU_RMAC_FLAG, securityLevel) )
		{
			//mask setup secure channel
			channelInfo.security_level[selected_channel]	= U8_XOR_U8(channelInfo.security_level[selected_channel], SCF_ZERO_APDU_RMAC_FLAG);
		}
		else if( (SCF_C_MAC_NULL != U8_AND_U8(SCF_R_MAC_ENCRYPTION_FLAG, securityLevel)) || ( SCF_ZERO_APDU_RMAC_FLAG == securityLevel ) )
		{
			if(1 == g_SCP_appEnc)//不需要算RMAC
			{
				g_SCP_appEnc = false;
			}
			else
			{
				if(false == g_SCP_appRmac)
				{
					resLen = EncryptAndGenRMAC(MEMtPX8(&resBuffer[0]), resLen, status, false);
				}
				//consider without data that will be sent out
				/*memCopyToRam(&resBuffer[resLen], g_current_SCP_LAST_MAC_L, 8);
				resLen = U16_A_U8(resLen, 8);*/ //加上g_current_SCP_LAST_MAC_L的话套件测试RMAC会计算错误，GP704套件测试用例：sm_APDU_externalAuthenticate_success_SSD_ownKeys_DM_cmac_rmac_scp03_16B_44_2a_f0

				apdu_SetResLen(resLen);
			}
		}


		if (U8_AND_U8(g_SCP_BeginEnd_R_MAC[selected_channel], GET_CURRENT_RMAC_AND_END_MASK))
		{
			g_SCP_BeginEnd_R_MAC[selected_channel] = 0;      // end rmac
			//clear R-MAC security level
			channelInfo.security_level[selected_channel]	= U8_XOR_U8(channelInfo.security_level[selected_channel], SCF_R_MAC_FLAG);
		}

		if ( SCF_ZERO_APDU_RMAC_FLAG == securityLevel )
		{
			channelInfo.security_level[selected_channel] = SCF_NO_AUTHENTICATED; //close secure channel
		}
	}
}
#endif //end of (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC || GP__SCP__SCP11__FOR_MASK)

u8 ManageSecurityLevel(u8 SecurityLevel)
{
	u8 		X_A		manageResult;
	u8 		X_A		bsecurity_level;
	u8 		X_A		bchannel_status;
	
#if (GP__GP_API__FOR_V1_6_MASK || GP__GP_API__FOR_V1_7_MASK)
	u8 		X_A		bcompulsory_Session_Security_Level;

	bcompulsory_Session_Security_Level  =   channelInfo.compulsory_Session_Security_Level[selected_channel];
#endif

	manageResult = RESULT_NOK;
	bsecurity_level =  	channelInfo.security_level[selected_channel];
	bchannel_status = 	channelInfo.channel_status[selected_channel];
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	if( SecurityLevel >= (bsecurity_level & 0x7F))
#else
 #if (GP__GP_API__FOR_V1_6_MASK || GP__GP_API__FOR_V1_7_MASK)
	if((bsecurity_level != SCF_NO_AUTHENTICATED)&&(SecurityLevel >= bcompulsory_Session_Security_Level ) )
 #else
	if( (SecurityLevel & 0x3F) >= (bsecurity_level & 0x3F)
		 && ((bsecurity_level & 0x3F) != 0x00))
 #endif
#endif
	{
		if( (CHANNEL_DISABLED != bchannel_status) && (CHANNEL_CLOSED != bchannel_status ))
		{
			 managerChannelLevel(SecurityLevel);

			 manageResult = RESULT_OK;
		}
	}

	return manageResult;
}

//只用于获取当前SD的obj
memref getCurrentSdObj(u8 isNeedCheckSDIsLocked)
{
	u8		X_A		sdId;
		
	jref	X_A		sdRef;
	jref	X_A		sdDataRef;
	memref	X_A		sdEntryAddr;
	
	memref			sdDataObj;
	
	sdId = g_bSDId[selected_channel];
	sdEntryAddr = getEntryByAppId(sdId);
	if(IS_NEED_CHECK_SD_LOCKED == isNeedCheckSDIsLocked)
	{
		checkSDIsLocked(sdEntryAddr, IS_NEED_CHECK_SD_LOCKED);
	}
	sdRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(sdEntryAddr), theApplet_INDEX)));
	sdDataRef = sdData_getSDdataRef(sdRef);
	sdDataObj = locateObjectInMemory(sdDataRef);

	return sdDataObj;
}



/*
modify by chentao@20211012 合英飞凌修改的代码 小俊修改SSD如果有keyset则不能查找关联SD的key
modify by xuxiaojun@20220505 该函数只被使用在建立SCP02\SCP03的安全会话中，在不支持JAVA_CARD_UICC的情况下，所有的key version都认为是secure channel的key version: gpMGCompliance103: km_APDU_putKeyRsa_SEC_30_07
modify by xxj@20230512: 修正缺陷487，增加输出参数，用于获取第一组可用密钥
*/
memref getCurrentSdDataObj(u8 isNeedCheckSDIsLocked, u8* tmpVersion)
{
#define 			ISD_SD_ID			0x00
#define				SSD_PERSONALIZED_MASK 0x0F
	
	u8		X_A		sdId;
	u8		X_A		sdId_temp;
	u8		X_A		keyVersion;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u8		X_A		SSDState;
#endif
	jref	X_A		sdRef;
	jref	X_A		keySetRef;
	jref	X_A		sdDataRef;
	PN8				sdDataObj;
	memref	X_A		sdEntryAddr;

	sdId = g_bSDId[selected_channel];
	keyVersion = 0x00;
	sdDataObj = MEM_NULL;

	while(0x00 == keyVersion)
	{
		sdEntryAddr = getEntryByAppId(sdId);
		//if(IS_NEED_CHECK_SD_LOCKED == isNeedCheckSDIsLocked)
		//{
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			SSDState = checkSDIsLocked(sdEntryAddr, (isNeedCheckSDIsLocked | IS_NEED_CHECK_SD_PERSONALIZED));
			if ((SSD_PERSONALIZED_MASK == SSDState) || (ISD_SD_ID == sdId))//不加isd的判断，在个人化时auth会死循环，因为开始个人化时，不是个人化或者安全状态
			#else
			checkSDIsLocked(sdEntryAddr, isNeedCheckSDIsLocked);
			#endif
		//}
		{
			sdRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(sdEntryAddr), theApplet_INDEX)));
			sdDataRef = sdData_getSDdataRef(sdRef);
			sdDataObj = MEMtPN8(locateObjectInMemory(sdDataRef));
#if (EUICC_ONLY||SE_AND_EUICC)
			//When one complete Secure Channel Key Set has been successfully loaded, the Security Domain uses
			//this newly loaded Key Set to establish a Secure Channel session independent of the Associated Security
			//Domain.

			/*if(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX)//对于profile的SD C9参数中没有带SCP TYPE ，不支持协议
			{
				if((sdData_getScpOptionByScpType(sdDataObj,SCP_01) == 0xFF)
					&&(sdData_getScpOptionByScpType(sdDataObj,SCP_02) == 0xFF)
					&&(sdData_getScpOptionByScpType(sdDataObj,SCP_03) == 0xFF))
				{
					throw_error(SW_REFERENCED_DATA_NOT_FOUND);//SD 不支持SCP 1 2 3 不能进行安全会话
				}
			}*/
#endif
			// if keyVersion is not 0x00, the key is init
			keySetRef = keySet_getDefaultKeySet(sdDataObj); // 只获取SCP0X_WITHOUT_IOPTION_MODE
			if(REF_NULL != keySetRef)
			{
				keyVersion = keySet_getDefaultKeyVersion(sdDataObj);
				/*if( (keyVersion < 0x20) || (keyVersion > 0x2f) )
				{
					if( 0xff != keyVersion)
					{
						keyVersion = 0x00;
					}
				}
				else{
					if((0 != U8_AND_U8(isNeedCheckSDIsLocked, IS_NEED_CHECK_SD_KEY_INTERGRATED))
							 && (false == keySet_CheckIntergrated(MEMtPN8(locateObjectInMemory(keySetRef)))))
					{
						keyVersion = 0x00;
					}
				}*/

//				if(((IsSCP02Key(keyVersion) )&&(sdData_getScpOptionByScpType(sdDataObj,SCP_02) != 0xFF))
//#if GP__AMENDMENT__D
//				||((IsSCP03Key(keyVersion) )&&(sdData_getScpOptionByScpType(sdDataObj,SCP_03) != 0xFF))
//#endif
//					)
				{
					if((0 != U8_AND_U8(isNeedCheckSDIsLocked, IS_NEED_CHECK_SD_KEY_INTERGRATED))
						 && (false == keySet_CheckIntergrated(MEMtPN8(locateObjectInMemory(keySetRef)))))
					{
							keyVersion = GPSSDExistCompleteSecureKey(sdId,  getSCPTypeByKeyVersion(keyVersion));
					}
				}
//				else
//				{
//					if( 0xff != keyVersion)
//					{
//						keyVersion = 0x00;
//					}
//				}

			}

		}

		if(keyVersion == 0x00) //默认key不在，需要查找关联安全域
		{
			if(GPSSDExitCompleteProtolKey(sdId) == true)//When one complete Secure Channel Key Set has been successfully loaded, the Security Domain uses this newly loaded Key Set to establish a Secure Channel session independent of the Associated Security Domain.
			{
				throw_error(SW_REFERENCED_DATA_NOT_FOUND); //02.eSIM_Test_RSP.sol/10.eSIM_Test_additional/MNO_C9_without_SCP02_Test_03
			}

			sdId_temp = getAppAssociatedSdId(MEMtPF8(sdEntryAddr));//PX_R_U8(get_object_field_ref(sdEntryAddr, bsdId_INDEX));

			if(sdId_temp == sdId)
			{
				throw_error(SW_REFERENCED_DATA_NOT_FOUND);/* 进入这里说明sdId代表的SD为自关联，且如果KEYVERISON is null 得去找CardManager的key，或者报错？因此可以直接使用ISD的 */
	//			sdId = ISD_SD_ID;
			}
			else
			{
				sdId = sdId_temp;
			}
		}
		else
		{
        // 满足循环跳出条件
		}
	}
	
	channelInfo.selected_key_appId[selected_channel] = sdId; //added by xuxiaojun@20220508 用于记录当前通道上建立安全会话时所使用的key所属的SD
	
	*tmpVersion = keyVersion;

	return PNtMEM(sdDataObj);
}

memref getKeySetObj(PX8 sdDataObj, u8 keyVersion)
{
	jref	X_A		keySetRef;
	memref			keySetObj;

	keySetRef = keySet_getKeySet(sdDataObj, keyVersion);
	if(REF_NULL == keySetRef)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	keySetObj = locateObjectInMemory(keySetRef);
	return keySetObj;
}
/**
 * modify by xxj@20230512: 修正缺陷487时，做对应的调整
 */
memref InitAndCheckSCPParam()
{

	u8		X_A		bP1/*, KeyNum*/;
	jref			keySetRef;
	PN8				sdDataObj;
	memref			keySetObj;
	u8              tmpKeyVersion;

	/*colliseV2.2.1 km_APDU_putKeyDes_7a_da_69_scp02_b3_cd_ac put key不完整的2A key，select ssd auth ;modified by shenzhicheng&&zm*/
	sdDataObj = MEMtPN8(getCurrentSdDataObj(U8_OR_U8(IS_NEED_CHECK_SD_LOCKED, IS_NEED_CHECK_SD_KEY_INTERGRATED), &tmpKeyVersion));

	g_current_SCP_Type = sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP0X_WITHOUT_IOPTION_MODE, 0x00);
	//g_current_SCP_Type = sdData_getScpType(sdDataObj);
	g_current_SCP_I_Option = sdData_getScpOption(sdDataObj);

	bP1 = APDUBuffer[OFFSET_P1];
	if(0x00 == bP1)
	{/* modified by zm; 移动要求，SSD默认要使用最高版本的key*/
		#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		currKeySetVer[selected_channel] =keySet_getMaxKeyVersion(sdDataObj, 0x02);
		#else
		currKeySetVer[selected_channel] = tmpKeyVersion; //keySet_getDefaultKeyVersion(sdDataObj);
		#endif
	}
	else
	{
		currKeySetVer[selected_channel] = bP1;
	}

	keySetRef	=	keySet_getKeySet(sdDataObj, currKeySetVer[selected_channel]);
	if(REF_NULL == keySetRef)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
	keySetObj	=	locateObjectInMemory(keySetRef);
	/*KeyNum		=	keySet_getKeyNum(keySetObj);

	if(KeyNum<MAX_KEYS_NUM)
	{
		throw_error(REFERENCED_DATA_NOT_FOUND);
	}*/
	if (false == keySet_CheckIntergrated(MEMtPN8(keySetObj)))
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
// 	keySetObj = getKeySetObj(sdDataObj, currKeySetVer[selected_channel]);

	return keySetObj;

}

u16 Remote_EncryptAndGenRMAC(void)
{
	u16        X_A      status = popShort();
	u16        X_A      len =  popShort();
	u8         X_A		securityLevel;
#if (GP__SCP && (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC))
	if(((status != 0x9000)&&((status&0xFF00) != 0x6200)&&((status&0xFF00) != 0x6300)&&((status&0xFF00) != 0x6100)))//GP 附录D 6.2.5 All status words except '9000' and warning status words (i.e. '62xx' and '63xx') shall be interpreted as error status words.
	{
		return len; //错误状态字不算rmac和不加密
	}
	#if JAVA_CARD_UICC
	if(G_RAMSessionSdId == APP_NULL) //没有 开启会话     eUICC test M2M/eUICC_M2M_additonal/补充测试脚本/Load_Enabled_Profile_In_One_HTTPS_Test_01
	{
		return len;
	}
	#endif
	securityLevel 	= channelInfo.security_level[selected_channel];
	//附录D 6.2.5  The EXTERNAL AUTHENTICATE command/response doesn’t return R-MAC.
	if( SCF_ZERO_APDU_RMAC_FLAG == U8_AND_U8(SCF_ZERO_APDU_RMAC_FLAG, securityLevel) )//SCF_ZERO_APDU_RMAC_FLAG表示本条指令不计算RMAC ，一般在8482这条指令设置上
	{
		//mask setup secure channel
		channelInfo.security_level[selected_channel]	= U8_XOR_U8(channelInfo.security_level[selected_channel], SCF_ZERO_APDU_RMAC_FLAG);
	}
	else
	{
		if((status&0xFF00) == 0x6100)
		{
			status = 0x9000;
		}

		len = EncryptAndGenRMAC(PM8_A_U8(APDUBuffer,OFFSET_CDATA),len,status,0);
	}

	if ( SCF_ZERO_APDU_RMAC_FLAG == securityLevel )
	{
		channelInfo.security_level[selected_channel] = SCF_NO_AUTHENTICATED; //close secure channel
	}
#endif
	return len;
}
#if (JAVA_CARD_UICC && !HAL__PLATFORM__SIMULATOR) //modify by jiangxiaohui@20220324 分层
__attribute__( ( section( "ScpDataArea" ) ) )
scpDataArea_t CONST SCP_DATA_AREA={0x00};
#else
scpDataArea_t scpData;
#endif

#endif
