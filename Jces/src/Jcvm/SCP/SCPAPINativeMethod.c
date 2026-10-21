/*
 * SCPAPINativeMethod.c
 *
 *  Created on: 2013-9-27
 *      Author: jiangxiaohui
 */

#include "SCPAPINativeMethod.h"
#include "SCPDealData.h"
#include "SCPInterface.h"
#include "Loader.h"
#include "opFunction.h"
#include "global_variable.h"
#include "cardmanager.h"
#include "error.h"
#include "SCPParamCheck.h"
#include "ob_manager.h"
#include "nativemethod_impl.h"
#include "Crypto_Constant.h"
#include "JcvmRam.h"
#include "SCP11.h"  
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "keySet.h"  //remove warning ,dengyiwen,2022/9/22
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
#include "interface_LTEFromPlatform.h"         //remove warning,dengyiwen,2022/9/22
#endif

#if (GP__SCP__API__FOR_MASK)

static u8  X_A    setIncomingProSecurity;

void SCPAPI_initGlobalScpAPI(void)
{
	setIncomingProSecurity	= 0;
	initScpInterface();
}

u8 getGlobalScpAPI(void)
{
	return setIncomingProSecurity;
}
/*
*@funtion    : SCPAPI_implementMethod
*
*@description: SCPAPIInterfaceImplement的子函数
*
*@See:      org.globalplatform.SecureChannel.java / org.globalplatform.SecureChannelx2.java
*
*@Parameters: short SCPAPITpye - Java Param
*@Parameters: byte bSecurityLevel -
*@Parameters: byte[] key -
*@Parameters: short inLength -
*@Parameters: short inOffset -
*@Parameters: byte[] inBuffer -
*@Parameters: byte channel -

*@Returns   : the number of bytes to be output 
*
*@Throws    :
*@history   :V1.1----
*/
u16 SCPAPI_implementMethod(u8 SCPAPITpye, PX8 SrcAddr, u16 sLength, u8 channel, u8 bSecurityLevel, jref bKey_ref)
{
	u16		X_A		result;

	result = 0x00;
	switch(SCPAPITpye)
	{
		case ORG_GLOBALPLATFORM_PROCESSSECURITY:
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
			if(ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) // SecureChannel. processSecurity()throws an ISO Exception with status code ISO7816.SW_INS_NOT_SUPPORTED
			{
				throw_error(SW_INS_NOT_SUPPORTED);
			}
			else
#endif
			{
				setIncomingProSecurity = 1;
				result = SCPAPI_processSecurityOrgApi(SrcAddr);
				setIncomingProSecurity = 0;
			}
			break;
		case ORG_GLOBALPLATFORM_WRAP:
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
			if(ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) // SecureChannel.wrap()may be called and will not return an error, but will not do any processing on the outgoing response message
			{
				result = sLength;
			}
			else
#endif
			{
				result = SCPAPI_wrapOrgApi(SrcAddr, sLength);
			}
			break;
		case ORG_GLOBALPLATFORM_UNWRAP:
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
			if(ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) // SecureChannel.unwrap( … )may be called and will not return an error, but will not perform any additional secure messaging processing
			{
				result = sLength;
			}
			else
#endif
			{
				result = SCPAPI_unwrapOrgApi(SrcAddr, sLength);
			}
			break;
		case ORG_GLOBALPLATFORM_DECRYPTDATA:
			result = SCPAPI_decryptDataOrgApi(SrcAddr, sLength, bKey_ref);
			break;
		case ORG_GLOBALPLATFORM_ENCRYPTDATA:
			result = SCPAPI_encryptDataOrgApi(SrcAddr, sLength, bKey_ref);
			break;
		case ORG_GLOBALPLATFORM_RESETSECURITY:
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
			if(ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) // SecureChannel.resetSecurity ()throws an ISO Exception with status code ISO7816.SW_CONDITION_OF_USE_NOT_SATISFIED
			{
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
			else
#endif
			{
				SCPAPI_resetSecurityOrgApi();
			}
			break;
		case ORG_GLOBALPLATFORM_GETSECURITYLEVEL:
			result =U8t16(SCPAPI_getSecurityLevelOrgApi());
			break;
		case ORG_GLOBALPLATFORM_SETSECURITYLEVEL:
			SCPAPI_setSecurityLevelOrgApi(bSecurityLevel);
			break;

		case VISA_OPENPLATFORM_OPSNSECURECHANNEL:
			result =U8t16(SCPAPI_openSecureChannelVisaApi(SrcAddr));
			break;
		case VISA_OPENPLATFORM_CLOSESECURECHANNEL:
			SCPAPI_closeSecureChannelVisaApi(channel);
			break;
		case VISA_OPENPLATFORM_DECRYPTVERIFYKEY:
			result =U8t16 (SCPAPI_decryptVerifyKeyVisaApi(channel, SrcAddr));
			break;
		case VISA_OPENPLATFORM_UNWRAP:
			SCPAPI_unwrapVisaApi(channel, SrcAddr);
			break;
		case VISA_OPENPLATFORM_VERIFYEXTERNALAUTHENTICATE:
			SCPAPI_verifyExternalAuthenticateVisaApi(channel, SrcAddr);
			break;
		case VISA_OPENPLATFORM_VERIFYDAP:
			//
			break;
		case VISA_OPENPLATFORM_DECIPHEREDLOADBLOCKDATA:
			//
			break;
		default:
			break;
	}

	return result;

}

/**
 * @see org.globalplatform.SecureChannel.processSecurity(APDU apdu)
 * @see org.globalplatform.SecureChannelx2.processSecurity(byte[], short, short, short)
 * @Parameters: PX8 SrcAddr --- 该地址处存储的是标准APDU数据，与APDUBuffer数据结构一致 5字节头+体
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
u16 SCPAPI_processSecurityOrgApi(PX8 SrcAddr)
{
	u16 	X_A		resLen;
	u8	 	X_A		bchannel;
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11
	u16 	X_A		errorCode;
#endif
	resLen = 0;
	switch(SrcAddr[OFFSET_INS])
	{
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11
		case INS_OP_PERFORM_SECURITY_OPERATION: // 0x2A
		{
			if(apdu_IsSetIncoming() == 0)// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
			{
				apdu_ApduSetIncomingAndReceive();
			}
			errorCode = SCP11PerformSecurityOperation(APDU_DATAFIELD_IS_RECEIVED);

			if (errorCode != SW_NO_ERROR)
			{
				throw_error(errorCode);
			}

			break;
		}
#endif
		case INS_INITIALIZE_UPDATE:
			if (GP_80 == U8_AND_U8(SrcAddr[OFFSET_CLA], GP_NO_LOGIC_MASK))
			{
				if(apdu_IsSetIncoming() == 0)// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
				{
					apdu_ApduSetIncomingAndReceive();
				}

				SCPAPI_openSecureChannelVisaApi(SrcAddr); //该函数内部并未使用SrcAddr，实际使用的是APDUBuffer

				resLen = U16_AND_U16(U8t16(SrcAddr[OFFSET_LC]), 0xFF);
			}
			else
			{
				throw_error(SW_CLA_NOT_SUPPORTED);
			}
			break;
		case INS_EXTERNAL_AUTHENTICATION:
			if (GP_84 == U8_AND_U8(SrcAddr[OFFSET_CLA], GP_NO_LOGIC_MASK))
			{
				if(apdu_IsSetIncoming() == 0)// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
				{
					apdu_ApduSetIncomingAndReceive();
				}
				bchannel=channelInfo.sercrity_channel_number[selected_channel];
				if(0x00 == bchannel)
				{
					throw_error(SW_CONDITIONS_NOT_SATISFIED);
				}

				SCPAPI_verifyExternalAuthenticateVisaApi(bchannel, SrcAddr); //该函数内部并未使用SrcAddr，实际使用的是APDUBuffer
			}
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11
			else if((GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))&&(PX_R_U8(g_current_PerformSecurityOperation_state) == PERFORMSECURITYOPERATION_STATE_END)) // 如果卡片支持的逻辑通道号大于4个，则这里的判断不正确
			{
				if(apdu_IsSetIncoming() == 0)// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
				{
					apdu_ApduSetIncomingAndReceive();
				}
				errorCode = SCP11MutualAuthenticate(APDU_DATAFIELD_IS_RECEIVED);

				if (errorCode != SW_NO_ERROR)
				{
					throw_error(errorCode);
				}

				resLen = U16_AND_U16(U8t16(SrcAddr[OFFSET_LC]), 0xFF);

				break;
			}
#endif
			else
			{
				throw_error(SW_CLA_NOT_SUPPORTED);
			}
			break;
		default:
			throw_error(SW_INS_NOT_SUPPORTED);
			break;
	}
	return resLen;
}

/**
 * @see org.globalplatform.SecureChannel#wrap(byte[], short, short)
 */
u16 SCPAPI_wrapOrgApi(PX8 SrcAddr, u16 sLength)
{
	u8 		X_A		securityLevel;
	u16     X_A		returnLength;
	u8 		X_A	    bcompulsorySessionSecurityLevel;
	u16		X_A		status;

#if (GP__GP_API__FOR_V1_6_MASK || GP__GP_API__FOR_V1_7_MASK)
	u16     X_A     StatusLength;
	StatusLength = 2;
#endif
	
	returnLength = 0;
	
	securityLevel = channelInfo.security_level[selected_channel];
	bcompulsorySessionSecurityLevel = channelInfo.compulsory_Session_Security_Level[selected_channel] ;
	
	/*
	 sLength - length of response data (including the expected status bytes). 
	*/
#if !(GP__GP_API__FOR_V1_6_MASK || GP__GP_API__FOR_V1_7_MASK)
	returnLength = sLength ;
#else
	if(sLength >= StatusLength)
	{
		/*
			the response data is with security information added and status bytes removed
		*/
	  returnLength = U16_S_U16(sLength, StatusLength);
		
	}
	else
	{
		throw_error(SW_WRONG_LENGTH);
	}
#endif
	if (SCF_NO_AUTHENTICATED == securityLevel)
	{
		if (SCF_NO_AUTHENTICATED != bcompulsorySessionSecurityLevel)
        {
        	throw_error(SW_CONDITIONS_NOT_SATISFIED);
        }
		return returnLength;
	}
#if ((GP__SCP__SCP01_RMAC) || (GP__SCP__SCP02_RMAC) || (GP__SCP__SCP03_RMAC))

	#if ((GP__SCP__SCP01_RMAC) || (GP__SCP__SCP02_RMAC))
	returnLength = EncryptAndGenRMAC(SrcAddr ,returnLength, 0x9000, true);
	#else //GP__SCP__SCP03_RMAC
	status = U16_A_U8((SrcAddr[returnLength] << 8),SrcAddr[returnLength + 1]);
	returnLength = EncryptAndGenRMAC(SrcAddr ,returnLength, status, true);//UL:gpapi_testApplet_92_SC_wrap_success_openSC_rmac_noData_SW6283_scp03_de_92_d5
	#endif //(GP__SCP__SCP03_RMAC)
	if(SCF_ENCRYPTION_FLAG == U8_AND_U8(SCF_ENCRYPTION_FLAG, securityLevel) )
	{
		g_SCP_appREnc = 1;
	}
	if(SCF_R_MAC_FLAG == U8_AND_U8( SCF_R_MAC_FLAG, securityLevel) )
	{
		//restore apdubuffer data
		g_SCP_appRmac = 1;
	}
#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
	return returnLength;
}


/**
* @see org.globalplatform.SecureChannel#unwrap(u8[], u16, u16)
*/
u16 SCPAPI_unwrapOrgApi(PX8 SrcAddr, u16 sLength)
{
	u16 	X_A		result;
	u8 		X_A		bsecurityLevel;
	u8 		X_A		bcompulsorySessionSecurityLevel;
	u8 		X_A		needDecryptCommandData;
	u8 		X_A		needVerifyMAC;

	result                          =   SW_NO_ERROR;
    bsecurityLevel                  = channelInfo.security_level[selected_channel];
    bcompulsorySessionSecurityLevel = channelInfo.compulsory_Session_Security_Level[selected_channel] ;

/*
	If the Current Security Level is NO_SECURITY_LEVEL and is different from the compulsory Session Security Level 
	(i.e. a previous Secure Channel Session was aborted but not fully terminated), 
	then this method shall throw an exception .
*/
	if( ( SCF_NO_AUTHENTICATED == bsecurityLevel))
	{
        if (SCF_NO_AUTHENTICATED != bcompulsorySessionSecurityLevel)
        {
        	throw_error(SW_CONDITIONS_NOT_SATISFIED);
        }
        return sLength;
    }

	/*
	If the Current Security Level is NO_SECURITY_LEVEL, AUTHENTICATED or ANY_AUTHENTICATED, 
	then this method shall not attempt to verify and remove any security protection, 
	the incoming command will remain as is within the baBuffer byte array (with class byte indicating secure messaging) 
	and the returned length shall be set to the value of the sLength parameter
	*/
	if( (SCF_AUTHENTICATED == bsecurityLevel) || (SCF_ANY_AUTHENTICATED == bsecurityLevel) )
	{
		 return sLength;
	}
/*
	If the class byte does not indicate secure messaging (according to ISO/IEC 7816-4), 
	then this method shall not attempt to verify and remove any security protection: 
	the incoming command shall remain as is within the baBuffer byte array 
	and the returned length shall be set to the value of the sLength parameter.
*/
    if (GP_SECURITY_MASK != U8_AND_U8(SrcAddr[OFFSET_CLA], GP_SECURITY_MASK))
    {
		throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
    }
/*
	'6982' if this method failed verifying or removing the security protection of the incoming APDU command
*/
    if(RESULT_OK == checkSecureLevel(SrcAddr, bsecurityLevel, &needDecryptCommandData, &needVerifyMAC))//merged from EPX\v3.0\DEVELOP@20220722
	{
			result = decryptAndVerifyMAC(SrcAddr, bsecurityLevel, needDecryptCommandData, needVerifyMAC);
		
	}

    if (SW_NO_ERROR != result)
	{
		/*
		 Secure Channel Session being aborted;
		*/
	    managerChannelLevel(SCF_NO_AUTHENTICATED);
		throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
	}
	
    //Bit 3 of the CLA   byte of the command header is cleared.
    SrcAddr[OFFSET_CLA] =U8_AND_U8(SrcAddr[OFFSET_CLA],  GP_NO_SECURITY_MASK);
/*
	@return length of the reformatted (unwrapped) APDU command
*/
    return U8t16(U8_A_U8(SrcAddr[OFFSET_LC], OFFSET_CDATA));
}
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x

/*
*@function   : IF_GetSCP80KeyInfo;
*
*@description: 获取当前SCP 80 中的KIK ，并将其缓存到 16字节的 dek中
*
*@Param      : dek: 16字节的数据buf
*		     :u8 ramKeyversion: 对于ram putkey操作，先查找ramKeyversion是否有DEK，如果有，则用此DEK进行解密操作，否则采用和下行报文KIC KID一样版本号的DEK 进行解密操作
*      		 :当前要解密的key所属的SDID
*			 :u8 *keyType	 --- 输出，获取到的当前HTTP建立连接时使用的PSK同一组的DEK的类型
*
*@return     :void
*
*@author     :xuyz
*			 :modify by jiangqb-20200113 -修改函数名，增加获取key类型
*/
u8 IF_GetSCP80KeyInfo(u8 *dek,u8 ramKeyversion,u8 sdId, u8 *keyType)
{
	//memref X_A keyAddr;  //remove warning ,dengyiwen,2022/9/22
	//u8 I_A keyComleteTag; //remove warning ,dengyiwen,2022/9/22
	u8 I_A keyLength;
	jref				keySetRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;
#if JAVA_CARD_UICC
	if(G_RAMSessionSdId != APP_NULL)  //RAM APDU??SCP02 ??????????
	{
		return TRUE;
	}
#endif
	if(ramKeyversion != 0x00)  //???GPPutDESTokenKey??,?????????????????
	{
		sdDataObject		=	MEMtPN8(getSdDataObjBySdid(sdId));
		keySetRef			=	keySet_getKeySet(sdDataObject, ramKeyversion);

		if(keySetRef == REF_NULL )
		{
			return FALSE;
		}

		keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));

		if (keyType != NULL)
		{
			*keyType = keySet_getKeyType(keySetObjcet, 0x03, THROW_ERROR);		//DEK keyType
		}

		keyLength = getScp80Key(sdId, ramKeyversion, 0x03, (memref)dek);

		return keyLength;
	}

	sdDataObject		=	MEMtPN8(getSdDataObjBySdid(G_TARRecIndex[2]));
	keySetRef			=	keySet_getKeySet(sdDataObject, G_RFMRAMKeyversion);

	if(keySetRef == REF_NULL )
	{
		return FALSE;
	}

	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));

	if (keyType != NULL)
	{
		*keyType = keySet_getKeyType(keySetObjcet, 0x03, THROW_ERROR);		//DEK keyType
	}

	keyLength = getScp80Key(sdId, ramKeyversion, 0x03, (memref)dek);

	return keyLength;
}
/*
*@function   :IF_GetSCP81KeyInfo
*
*@description:获取当前HTTP建立连接时使用的PSK同一组的DEK
*
*@Param      :BYTE *dek --- 输出，获取到的当前HTTP建立连接时使用的PSK同一组的DEK的值
*			 :BYTE *keyType	 --- 输出，获取到的当前HTTP建立连接时使用的PSK同一组的DEK的类型
*
*@return     :BYTE      --- dek中有效数据的长度
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20191122
*		     :modify by jiangqb@20191126	增加判断当https的目标sd和触发sd不一致，并且目标sd有安全key的情况,此时dek使用目标sd的鉴权dek保护
*			 ::modify by jiangqb@20200113	增加获取scp的dek的keyType
*/
BYTE IF_GetSCP81KeyInfo(BYTE *dek, BYTE *keyType)
{
	//UWORD xdata keyAddr;  //remove warning ,dengyiwen,2022/9/22
	BYTE idata keyVersion, sdId;
	BYTE idata dekLen;
	jref				keySetRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;

	keyVersion = IF_GetScp81KeyVersionNum();

	sdId = IF_GetScp81TriggerSD();

	if((keyVersion == 0xFF) || (sdId == ILLEGAL_APPID))
	{
		return FALSE;
	}

	sdDataObject		=	MEMtPN8(getSdDataObjBySdid(sdId));
	keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);

	if(keySetRef == REF_NULL )
	{
		return FALSE;
	}

	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));

	if (keyType != NULL)
	{
		*keyType = keySet_getKeyType(keySetObjcet, 0x02, THROW_ERROR);		//DEK keyType
	}

	dekLen = getScp80Key(sdId, keyVersion, 0x02, (UWORD)dek);

    return dekLen;
}
u8 GetSecureChannelKeyInfo(u8* tmpSessionKey, u8* decryptionAlg, u16 sessionRef)
{
	u8 I_A dekLen;
	dekLen = 0x00;

	if(ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])
	{
		if(G_RAMSessionSdId == APP_NULL)
		{
			if(IF_IsHTTPCommand() == TRUE)
			{
				dekLen = IF_GetSCP81KeyInfo(tmpSessionKey,decryptionAlg);

				if (decryptionAlg[0] != DecryptionAlg_AES_CBC)
				{
					decryptionAlg[0] = DecryptionAlg_3DES_CBC;	 //<GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf> If the DEK algorithm is 3DES, 3DES CBC encryption with ICV set to zero is performed
				}
			}
			else
			{
				dekLen = IF_GetSCP80KeyInfo(tmpSessionKey, G_RFMRAMKeyversion, G_TARRecIndex[2],decryptionAlg);  //返回值为TRUE:RAM下的SCP02 ,FALSE: 数据异常 keyLen: 0x10/0x24

				if (decryptionAlg[0] != DecryptionAlg_AES_CBC)
				{
					decryptionAlg[0] = DecryptionAlg_3DES_ECB;
				}
			}
		}
		else
		{
			if(channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED)
			{
			#if UICC__OPERATOR__CHINAUNICOM
					throw_error(SW_CONDITIONS_NOT_SATISFIED);//联通需要抛6985,联通测试GPAPI /gpapi_testApplet_31_SC_decryptData_NoSecureSession_de_1b_51
			#endif
				if(CONFIG_VGP)//modify by xuxiaojun@20220621 去掉全局变量isVGPconfig，直接使用CONFIG_VGP
				{
					throw_error(SW_CONDITIONS_NOT_SATISFIED);
				}
				else
				{
					throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);//移动GP UICC测试-gpapi_testApplet_30_SC_encryptData_Error_NoSecureSession期望抛6982
				}
			}

			ecpMem_Copy(tmpSessionKey, g_current_SCP_DEK_KEY, 0x10);  //拷贝会话密钥

			decryptionAlg[0] = DecryptionAlg_3DES_ECB;

			dekLen = 0x10;
		}
	}
	else if(G_TARRecIndex[0] == TarType_ToolkitApplet)// SCP80方式触发应用, processToolkit, 用SCP80 KEY解密
	{
		dekLen = IF_GetSCP80KeyInfo(tmpSessionKey, G_RFMRAMKeyversion, G_TARRecIndex[2],decryptionAlg);  //返回值为TRUE:RAM下的SCP02 ,FALSE: 数据异常 keyLen: 0x10/0x24

		if (decryptionAlg[0] != DecryptionAlg_AES_CBC)
		{
			decryptionAlg[0] = DecryptionAlg_3DES_ECB;
		}
	}
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK
	else if(G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection) // SCP81方式触发应用
	{
		dekLen = IF_GetSCP81KeyInfo(tmpSessionKey,decryptionAlg);

		if (decryptionAlg[0] != DecryptionAlg_AES_CBC)
		{
			decryptionAlg[0] = DecryptionAlg_3DES_CBC;	 //<GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf> If the DEK algorithm is 3DES, 3DES CBC encryption with ICV set to zero is performed
		}
	}
#endif	
	else  //JCREMain_APDUMode，暂时只支持SCP02鉴权
	{
		if(channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED)
		{
		#if UICC__OPERATOR__CHINAUNICOM
			throw_error(SW_CONDITIONS_NOT_SATISFIED);//联通需要抛6985,联通测试GPAPI /gpapi_testApplet_31_SC_decryptData_NoSecureSession_de_1b_51
		#endif
			if(CONFIG_VGP)//modify by xuxiaojun@20220621 去掉全局变量isVGPconfig，直接使用CONFIG_VGP
			{
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
			else
			{
				throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);////移动GP UICC测试-gpapi_testApplet_30_SC_encryptData_Error_NoSecureSession期望抛6982
			}
		}

		ecpMem_Copy(tmpSessionKey, g_current_SCP_DEK_KEY, 0x10);  //拷贝会话密钥

		decryptionAlg[0] = DecryptionAlg_3DES_ECB;

		dekLen = 0x10;
	}

	return dekLen;
}
#endif
/**
* public short unwrap(byte[] baBuffer, short sOffset, short sLength) throws
* ISOException { byte [] ramBuffer = NativeMethods.ramBuffer;
* if((ramBuffer[ISD_SECURITY_LEVEL] == (byte)0xFF)
* ||(ramBuffer[ISD_SECURITY_LEVEL] == (byte)0)){ if((baBuffer[sOffset] & 0x04) !=
* 0){ ISOException.throw_error(ISO7816.SW_CONDITIONS_NOT_SATISFIED); } return
* sLength; } if((baBuffer[sOffset] & 0x04) == 0){ isdCloseSecureChannel();
* ISOException.throw_error(ISO7816.SW_SECURITY_STATUS_NOT_SATISFIED); }
* Util.arrayCopyNonAtomic(baBuffer, sOffset, NativeMethods.APDUBuffer,
* (short)0, sLength); byte theChannel = ramBuffer[ISD_CHANNEL_NUMBER];
* unwrap(theChannel, null); return (short)(apduBuffer[ISO7816.OFFSET_LC] + 5); }
*/
u16 SCPAPI_decryptDataOrgApi(PX8 SrcAddr, u16 sLength, jref bKey_ref)
{
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
	u8 X_A tmpSessionKey[0x20];
	u8 X_A ivTmpBuf[0x20];
	u8 I_A decryptionAlg;
	u8 I_A dekLen;
	u8 I_A blockMask;

	dekLen = GetSecureChannelKeyInfo(tmpSessionKey, &decryptionAlg, REF_NULL);

	switch(decryptionAlg)
	{
		case DecryptionAlg_3DES_ECB:
		case DecryptionAlg_3DES_CBC:
			blockMask = DES_BLOCKSIZE_MASK;
			break;

		case DecryptionAlg_AES_CBC:
			blockMask = AES_BLOCKSIZE_MASK;
			break;
		
		default:
			blockMask = 0;
			break;
	}

	if (0x00 != U8_AND_U8(sLength, blockMask))
	{
		throw_error(SW_WRONG_LENGTH);
	}

	if (0x00 == sLength)
	{
		throw_error(SW_DATA_INVALID);
	}

	ecpMem_Fill(ivTmpBuf, 0x00, sizeof(ivTmpBuf));

	switch(decryptionAlg)
	{
		case DecryptionAlg_3DES_ECB:
			//TripleDES(SrcAddr, tmpSessionKey, SrcAddr, ECB_Decrypt, sLength, 0x10);
			SymCipherDofinal(MEMtPM8(SrcAddr), sLength, MEMtPM8(SrcAddr), MEMtPM8(tmpSessionKey), U8_M_U8(dekLen,8), MEMtPM8(ivTmpBuf), NULL, ALG_DES_ECB_NOPAD, ECB_Decrypt, 0x00, ALG_DES);
			break;
		case DecryptionAlg_3DES_CBC:
			//TripleDES(SrcAddr, tmpSessionKey, SrcAddr, CBC_Decrypt, sLength, 0x10);
			SymCipherDofinal(MEMtPM8(SrcAddr), sLength, MEMtPM8(SrcAddr), MEMtPM8(tmpSessionKey), U8_M_U8(dekLen,8), MEMtPM8(ivTmpBuf), NULL, ALG_DES_CBC_NOPAD, CBC_Decrypt, 0x00, ALG_DES);
			break;
		case DecryptionAlg_AES_CBC:
			//ecpMem_Fill(ivTmpBuf, 0x00, sizeof(ivTmpBuf));
			SymCipherDofinal(MEMtPM8(SrcAddr), sLength, MEMtPM8(SrcAddr), MEMtPM8(tmpSessionKey), U8_M_U8(dekLen,8), MEMtPM8(ivTmpBuf), NULL, ALG_AES_BLOCK_128_CBC_NOPAD, CBC_Decrypt, 0x00, ALG_AES);
			break;
	}

	return sLength;
#else
/*
	If the Current Security Level is NO_SECURITY_LEVEL or the necessary cryptographic keys are not available, 
	then this method shall throw an exception 
*/
	if(channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED)
	{ 
	#if (GP__GP_API__FOR_V1_6_MASK || GP__GP_API__FOR_V1_7_MASK)
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	#endif
		if(CONFIG_VGP)//modify by xuxiaojun@20220621 去掉全局变量isVGPconfig，直接使用CONFIG_VGP
		{
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
		}
		else
		{
			throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
		}
	}
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11
	if(false == SCP11_decryptDataValid())
	{
		throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
	}
#endif
	return cipherData(SrcAddr, sLength, bKey_ref, U32t8(MODE_DECRYPT), IS_NEED_CHECK_INLENGTH_YES);
#endif
}

u16 SCPAPI_encryptDataOrgApi(PX8 SrcAddr, u16 sLength, jref bKey_ref)
{
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
	u8 X_A tmpSessionKey[0x20];
	u8 X_A ivTmpBuf[0x20];
	u8 I_A decryptionAlg;
	u8 I_A dekLen;

	dekLen = GetSecureChannelKeyInfo(tmpSessionKey, &decryptionAlg, REF_NULL);

	if (0x00 == sLength)
	{
		throw_error(SW_DATA_INVALID);
	}
	
#if UICC__OPERATOR__CHINAUNICOM
	
	if((sLength == 0)||((sLength&0x07) != 0x00))	//gpapi_testApplet_92_SC_encryptData_error_dataNotBlockAligned_scp02_de_92_4c
	{
		ThrowError(SW_WRONG_LENGTH);   							
	}
#endif
	
	 ecpMem_Fill(ivTmpBuf, 0x00, sizeof(ivTmpBuf));

	switch(decryptionAlg)
	{
		case DecryptionAlg_3DES_ECB:
			//TripleDES(SrcAddr, tmpSessionKey, SrcAddr, ECB_Encrypt, sLength, 0x10);
			SymCipherDofinal(MEMtPM8(SrcAddr), sLength, MEMtPM8(SrcAddr), MEMtPM8(tmpSessionKey), U8_M_U8(dekLen,8), MEMtPM8(ivTmpBuf), NULL, ALG_DES_ECB_NOPAD, ECB_Encrypt, 0x00, ALG_DES);
			break;
		case DecryptionAlg_3DES_CBC:
			//TripleDES(SrcAddr, tmpSessionKey, SrcAddr, CBC_Encrypt, sLength, 0x10);
		    SymCipherDofinal(MEMtPM8(SrcAddr), sLength, MEMtPM8(SrcAddr), MEMtPM8(tmpSessionKey), U8_M_U8(dekLen,8), MEMtPM8(ivTmpBuf), NULL, ALG_DES_CBC_NOPAD, CBC_Encrypt, 0x00, ALG_DES);
			break;
		case DecryptionAlg_AES_CBC:
			//ecpMem_Fill(ivTmpBuf, 0x00, sizeof(ivTmpBuf));
			SymCipherDofinal(MEMtPM8(SrcAddr), sLength, MEMtPM8(SrcAddr), MEMtPM8(tmpSessionKey), U8_M_U8(dekLen,8), MEMtPM8(ivTmpBuf), NULL, ALG_AES_BLOCK_128_CBC_NOPAD, CBC_Encrypt, 0x00, ALG_AES);
			break;
	}

	return sLength;
#else
/*
	If the Current Security Level is NO_SECURITY_LEVEL or the necessary cryptographic keys are not available, 
	then this method shall throw an exception 
*/
	if (channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED)
	{
		#if (GP__GP_API__FOR_V1_6_MASK || GP__GP_API__FOR_V1_7_MASK)
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
		#endif
		throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
	}
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP11
	if(false == SCP11_decryptDataValid())
	{
		throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
	}
#endif
	return cipherData(SrcAddr, sLength, bKey_ref, U32t8(MODE_ENCRYPT), IS_NEED_CHECK_INLENGTH_YES);
#endif
}

void SCPAPI_resetSecurityOrgApi()
{
	closeSecuritychannel(SC_TERMINATED);// modify by liweimin
}

u8 SCPAPI_getSecurityLevelOrgApi()
{
#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
	u8 		X_A		securityLevel;
	if(ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])
	{
		if(G_RAMSessionSdId == 0xFF) //不需要执行SCP02
		{
			if(IF_IsHTTPCommand()) // SCP81
			{
				securityLevel = IF_GetScp81SecurityLevel();
			}
			else // SCP80
			{
				securityLevel = IF_GetScp80SecurityLevel();
			}

			return securityLevel;
		}
	}
	else if(G_TARRecIndex[0] == TarType_ToolkitApplet)// SCP80方式触发应用, processToolkit
	{
		securityLevel = IF_GetScp80SecurityLevel();

		return securityLevel;
	}
#if GP__AMENDMENT__B__createSCP81Connection_FOR_MASK	
	else if(G_HTTPSStatus.HTTPTriggerMode == HTTPTriggeredBySCP81Connection) // SCP81方式触发应用
	{
		securityLevel = IF_GetScp81SecurityLevel();

		return securityLevel;
	}
#endif
#endif
	//according to VGP 7.1.2.1.2 Processing for EXTERNAL AUTHENTICATE command
    //R_MAC_FLAG 0x40 is defined in C-level
	setNoRmac();
	return U8_AND_U8(channelInfo.security_level[selected_channel], SCF_ZERO_APDU_RMAC_FLAG_MASK);
}
void SCPAPI_setSecurityLevelOrgApi(u8 bSecurityLevel)
{
	if(RESULT_NOK == ManageSecurityLevel(bSecurityLevel))
	{
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}
}



/**
* This method is used to close the SecureChannel that was previously opened with
* openSecureChannel() method by the calling applet.
* @param channel byte
*/
void SCPAPI_closeSecureChannelVisaApi(u8 bchannel)
{
	//u8 		X_A		securityLevel;
	u8 		X_A		bserrity_channel_number;
	//securityLevel = channelInfo.security_level[selected_channel];
	bserrity_channel_number = channelInfo.sercrity_channel_number[selected_channel];

	if ((bserrity_channel_number != bchannel) && (SCF_NO_AUTHENTICATED != bserrity_channel_number) && (0x00 != bchannel))
	{
        throw_error(SW_CONDITIONS_NOT_SATISFIED);
    }

	managerRequestedChannelLevel(SCF_NO_AUTHENTICATED);
	closeSecuritychannel(SC_TERMINATED);// modify by liweimin
}

/**
* This method is used during key loading to decrypt and verify a new key. It returns TRUE
* if a key has been verified and FALSE otherwise
*
* @param channel byte	Secure channel number
* @param theAPDU APDU	javacard.framework.APDU
* @param offset short	Offset within an APDU buffer where the key can be retrieved
* @exception javacard.framework.CardRuntimeException The exception is thrown as a result of a decryption failure.
*/
boolean SCPAPI_decryptVerifyKeyVisaApi(u8 bchannel, PX8 SrcAddr)
{
	// need use OPTION_DECRYPTO, but for change bOP to -- 1: encrytp, 0: decrypt
#define 		DECRYPT_VERIFY_KEY_MODE					U8_OR_U8(MODE_DECRYPT, ECB_Mode)
#define 		CHECK_VALUE_LEN							0x03
	u8 		X_A		checkVal[16];
	u8 		X_A		lastKey[32];
	u8 		X_A		offset;
	u8 		X_A		id;
	u8 		X_A		securityLevel;
	u8 		X_A		bserrity_channel_number;
	u16 	X_A		length;
	PX8				tempPtr;
	securityLevel = channelInfo.security_level[selected_channel];
	bserrity_channel_number = channelInfo.sercrity_channel_number[selected_channel];
	offset = 0;

    if ((bserrity_channel_number != bchannel) || (securityLevel == SCF_NO_AUTHENTICATED))
    {
        throw_error(SW_CONDITIONS_NOT_SATISFIED);
    }

    tempPtr = SrcAddr;
    id = PX_R_U8(tempPtr);

    tempPtr = PX8_A_U16(tempPtr, 0x01);
    length = U8t16(PX_R_U8(tempPtr));

	cipherData(SrcAddr, length, U32t16(REF_NULL), U32t8(MODE_DECRYPT), IS_NEED_CHECK_INLENGTH_YES);

    if (id == U32t8(0x81))
    { // For DES Keys
    	tempPtr = PX8_A_U8(SrcAddr, offset);
        memCopyNoAtomic(MEMtPM8(lastKey), tempPtr, length);

        tempPtr = PX8_A_U8(tempPtr, offset);
        //offset = U8_A_U8(offset, length);
        if (CHECK_VALUE_LEN != PX_R_U8(tempPtr))
        {
            throw_error(SW_WRONG_DATA);
        }
        memFillToRam(MEMtPX8(checkVal), length, CLEARED_VALUE);
        dealcipherData(SrcAddr, lastKey, SrcAddr, length, DECRYPT_VERIFY_KEY_MODE, DES_BLOCKSIZE);

        tempPtr	= PX8_A_U8(tempPtr, 0x01);
        return (compareRamData(MEMtPX8(checkVal), tempPtr, CHECK_VALUE_LEN));
    }
    return true;
}

/**
* This method opens a Secure Channel for an applet and returns the newly opened channel number.
* The supplied APDU must contain the command to authenticate the card.
* This method also prepares the response to this command.
* The Security Domain that the applet belongs to is responsible for the channel number allocation.
* The channel '0' is reserved, and channels 1 through 127 are available for the applet's use.
*
* @return byte
* @param theAPDU javacard.framework.APDU
* @exception javacard.framework.CardRuntimeException
*            This exception is thrown if there is no available channel or if the APDU command is invalid.
*/
u8 SCPAPI_openSecureChannelVisaApi(PX8 SrcAddr)
{
	u16 	X_A		errorCode;
	errorCode = openSecureChannel(APDU_DATAFIELD_IS_RECEIVED);
    if (errorCode != SW_NO_ERROR)
    {
        throw_error(errorCode);
    }
    return channelInfo.sercrity_channel_number[selected_channel];
}

/**
* This method is used to process the APDU buffer content after receiving it from off-card.
* The processing is according to the requirements for integrity and confidentiality that
* are established when a Secure Channel is opened.
*
* @param channel byte
* @param theAPDU APDU	javacard.framework.APDU
* @exception javacard.framework.CardRuntimeException
*            This exception is thrown if the integrity verification encounters a problem.
*/
void SCPAPI_unwrapVisaApi(u8 bchannel, PX8 SrcAddr)
{
	u8 		X_A		bsecurityLevel;
	u8 		X_A		bserrity_channel_number;
	u16		X_A		tempI;

	bsecurityLevel = channelInfo.security_level[selected_channel];
	bserrity_channel_number = channelInfo.sercrity_channel_number[selected_channel];

    if ((bserrity_channel_number != bchannel) || (SCF_NO_AUTHENTICATED == bsecurityLevel))
    {
    	closeSecuritychannel(SC_TERMINATED);// modify by liweimin
        throw_error(SW_CONDITIONS_NOT_SATISFIED);
    }
    tempI = U8t16(U8_A_U8(SrcAddr[OFFSET_LC], OFFSET_CDATA));
    SCPAPI_unwrapOrgApi(SrcAddr, tempI);
}

/**
 * This method is used to authenticate the host by verifying the contents of an APDU command.
 *
 * @param channel byte
 * @param theAPDU javacard.framework.APDU
 * @exception javacard.framework.CardRuntimeException. This exception is thrown if the authentication fails.
 */
void SCPAPI_verifyExternalAuthenticateVisaApi(u8 bchannel, PX8 SrcAddr)
{
	u16 	X_A		errorCode;
	if(channelInfo.sercrity_channel_number[selected_channel] != bchannel)
    {
        throw_error(SW_CONDITIONS_NOT_SATISFIED);
    }
	errorCode = verifyExternalAuthenticate(APDU_DATAFIELD_IS_RECEIVED);

    if (errorCode != SW_NO_ERROR)
    {
    	closeSecuritychannel(SC_TERMINATED);// modify by liweimin
        throw_error(errorCode);
    }

	managerRequestedChannelLevel(channelInfo.security_level[selected_channel]);
}
#endif
