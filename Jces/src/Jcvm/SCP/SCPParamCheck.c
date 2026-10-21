/*
 * SCPParamCheck.c
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */


#include "SCPParamCheck.h"
#include "SCPInterface.h"
#include "CardManager.h"
#include "Loader.h"
#include "opFunction.h"
#include "global_variable.h"
#include "error.h"
#include "profile.h"
#include "scp10.h"
#include "scp02.h"
#include "SCP03.h"
#include "object.h"
#include "keySet.h"
#include "SecurityDomainData.h"
#include "KeyManagerOld.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if GP__SCP

void checkInitializeUpdate_param(u8 p1, u8 p2, u8 lc)
{

//	if(0xFF == p1)  //移动5G备案    ： GP功能补充测试脚本_接触_工程集/东信/apdu命令要求/initialize update指令测试/6.1.2.11.3 期望6A86 暂时不区分运营商 统一反6A86
//	{
//		#if (GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
//			throw_error(SW_INCORRECT_P1P2);
//		#else
//			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
//		#endif
//	}

	if(IsSpecialKey(p1)) //modify by xuxiaojun@20220428 支持token key等特殊密钥，不能用于鉴权
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	if((0x00 != p1)&& ((JcvmConfig_GetMaxSCPKeyVersion() < p1) || (JcvmConfig_GetMinSCPKeyVersion() > p1)))
	{
		throw_error(SW_INCORRECT_P1P2);
	}
	
#if GP__SCP__SCP11__FOR_MASK
	if(IsSCP11Key(p1)) // modify by xuxiaojun@20230113: 根据SLC37_EPX3.0.2项目评审问题跟踪表(release_20230105_v0.1)评审结果修改
	{
		throw_error(SW_INCORRECT_P1P2);
	}
#endif
	
#if JAVA_CARD_UICC
	if(IsSCP81Key(p1) || IsSCP80Key(p1)) //modify by xuxiaojun@20220428 默认01-7F都可以用于鉴权, gpMGCompliance103: km_APDU_putKeyRsa_SEC_30_07
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	if((0x00 != p1)&& (!IsSCP02Key(p1)))
	{
		throw_error(SW_INCORRECT_P1P2); //05.GP功能补充测试脚本_接触_工程集/东信/apdu命令要求/initialize update指令测试/6.1.2.11.3
	}
#endif
	
	

	//CM require the key version for SC is between 20 and 2F
//#if GP__AMENDMENT__D
//	if ( ((p1 < 0x20) || (p1 > 0x3F)) && (0x00 != p1) )
//#else
//	if ( ((p1 < 0x20) || (p1 > 0x2F)) && (0x00 != p1) )
//#endif
//	{
//		throw_error(SW_INCORRECT_P1P2);
//	}

	if (0x00 != p2)
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	// check the LC length is available
	if (0x08 != lc )
	{
		throw_error(SW_WRONG_LENGTH);
	}
}


void checkExternalAuthentication_param(u8 p1, u8 p2, u8 lc)
{

#if GP__AMENDMENT__D
	u8 temp1;
	u8 temp2;
#endif

	u8 isSCP10Check;
	isSCP10Check = RESULT_NOK;

	switch(g_current_SCP_Type)
	{
		case SCP_01:
			if (((0x00 != p1) && (0x01 != p1) && (0x03 != p1)) || (0x00 != p2))
			{
				throw_error(SW_INCORRECT_P1P2);
			}
			break;
		case SCP_02:
			if ((0x00 != U8_AND_U8(p1, 0xEC)) || (0x02 == U8_AND_U8(p1, 0x03)) || (0x00 != p2))
			{
				throw_error(SW_INCORRECT_P1P2);
			}
#if !GP__SCP__SCP02_RMAC  //不支持RMAC
	        if(U8_AND_U8(p1, 0x10))	//RMAC
	        {
	        	throw_error(SW_INCORRECT_P1P2);
	        }
#endif

			break;
#if GP__AMENDMENT__D
		case SCP_03:
			temp1 = (u8)(p1 & 0xF0);
			temp2 = (u8)(p1 & 0x0F);
			switch(temp1)
			{
				case 0x00:
					if( (0x03 != temp2) && (0x01 != temp2) && (0x00 != temp2))
					{
						throw_error(SW_INCORRECT_P1P2);
					}
					break;
				case 0x10:
					if( (0x03 != temp2) && (0x01 != temp2))
					{
						throw_error(SW_INCORRECT_P1P2);
					}
					break;
				case 0x30:
					if(0x03 != temp2)
					{
						throw_error(SW_INCORRECT_P1P2);
					}
					break;
				default:
					throw_error(SW_INCORRECT_P1P2);
					break;
			}
#if M2M_EUICC
			if((ISDP_APPID == g_bSDId[selected_channel]) && (p1 != 0x33)) //add by chentao@20220627 M2M SCP03 82 的P1只能是0x33
			{
				throw_error(SW_INCORRECT_P1P2);
			}

#endif
			if((p2 != 0x00))
			{
				throw_error(SW_INCORRECT_P1P2);
			}

			break;
#endif // GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			if( (0x00 != p1) || (0x00 != p2) )
			{
				throw_error(SW_INCORRECT_P1P2);
			}
			if(lc != apdu_ApduSetIncomingAndReceive())
			{
				throw_error(SW_WRONG_LENGTH);
			}

			isSCP10Check = RESULT_OK;
			break;
#endif // GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}

	if(RESULT_NOK == isSCP10Check)
	{/*@ -disspecopt @*/
		if ((0x00 == U8_AND_U8(p1, 0x01)) && (JcvmConfig_GetCardState() >= CARD_SECURED))
		{
			closeSecuritychannel(SC_TERMINATED); // modify by liweimin
			throw_error(SW_CONDITIONS_NOT_SATISFIED);
			//return SW_CONDITIONS_NOT_SATISFIED;
		}

		if ( 0x10 != lc)
		{
			throw_error(SW_WRONG_LENGTH);
			//return SW_WRONG_LENGTH;
		}
	}/*@ +disspecopt @*/
}

#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)
void checkBeginRmacSession_param(u8 p1, u8 p2, u8 lc)
{
	switch(g_current_SCP_Type)
	{
		case SCP_01:
		case SCP_02:
			if ((0x10 != p1) && (0x00 != p1))
			{
				throw_error(SW_INCORRECT_P1P2);
			}
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			if ((0x30 != p1) && (0x10 != p1))
			{
				throw_error(SW_INCORRECT_P1P2);
			}
			break;
#endif //GP__AMENDMENT__D
		default:
			break;

	}

	if (0x01 != p2)
	{
		throw_error(SW_INCORRECT_P1P2);
	}

}


void checkEndRmacSession_param(u8 p1, u8 p2, u8 lc)
{
	u8 checkResult;

	checkResult = RESULT_NOK;

	if (0x00 != p1)
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	if (0x03 == p2)
	{
		g_SCP_BeginEnd_R_MAC[selected_channel]= U8_OR_U8(g_SCP_BeginEnd_R_MAC[selected_channel], GET_CURRENT_RMAC_AND_END_MASK);
		checkResult = RESULT_OK;
	}

	if ( SCP_02 == g_current_SCP_Type )
	{
		if (0x01 == p2)
		{
			g_SCP_BeginEnd_R_MAC[selected_channel] = U8_OR_U8(g_SCP_BeginEnd_R_MAC[selected_channel], GET_CURRENT_RMAC_MASK);
			checkResult = RESULT_OK;
		}
	}

	if(RESULT_OK != checkResult)
	{
		throw_error(SW_INCORRECT_P1P2);
	}
}
#endif



void checkManageSecurityEnvironment_param(u8 p1, u8 p2, u8 lc)
{
	if((U8t16(lc) != apdu_ApduSetIncomingAndReceive()) || (4 != lc))//只存在强制数据
	{
		throw_error(SW_WRONG_LENGTH);
	}
	if( (0x81 != p1) && (0xC1 != p1) )
	{
		throw_error(SW_INCORRECT_P1P2);
	}
	if( 0xB6 != p2 )//限定进行证书验证
	{
		throw_error(SW_INCORRECT_P1P2);
	}
}

void checkPerform_Security_Operation_param(u8 p1, u8 p2, u8 lc)
{
	if(0x00 == p1)
	{
		//performVerifyCertificate
		if( (0xAE != p2) && (0xBE != p2) )
		{
			throw_error(SW_INCORRECT_P1P2);
		}
	}
	else if(0x80 == p1)
	{
		//performDecipher
		if(0x84 != p2)
		{
			throw_error(SW_INCORRECT_P1P2);
		}
	}
	else
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	if(U8t16(lc) != apdu_ApduSetIncomingAndReceive())
	{
		throw_error(SW_WRONG_LENGTH);
	}
}

void checkGetChallenge_param(u8 p1, u8 p2, u8 lc)
{
	if( ( 0x00 != p1 ) || ( 0x00 != p2 ) )
	{
		throw_error(SW_INCORRECT_P1P2);
	}
}


void checkInternalAuthentication_param(u8 p1, u8 p2, u8 lc)
{
	if( ( 0x00 != p1 ) || ( 0x00 != p2 ) )
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	if(U8t16(lc) != apdu_ApduSetIncomingAndReceive())
	{
		throw_error(SW_WRONG_LENGTH);
	}
}

void checkGenerateKeyPair_param(u8 p1, u8 p2, u8 lc)
{
	if( 0x00 != p1 )
		throw_error(SW_INCORRECT_P1P2);
	if( ( 0x00 != p2 ) && ( 0x01 != p2 ) )
	{
		throw_error(SW_INCORRECT_P1P2);
	}

	if(0x08 != lc)
		throw_error(SW_WRONG_LENGTH);
}



/*
 * add by jxh 20130902
 *
 * check p1, p2, Lc is available for Inc
 *
 * in:
 * 	APDUBuffer[OFFSET_INS]
 * out:
 * 	SW_NO_ERROR
 * 	REFERENCED_DATA_NOT_FOUND
 * 	SW_INCORRECT_P1P2
 * 	SW_WRONG_LENGTH
 * 	SW_CONDITIONS_NOT_SATISFIED
 *
 * */

void check_p1p2Lc_param(u8 ins)
{
	u8 p1;
	u8 p2;
	u8 lc;
	p1 = APDUBuffer[OFFSET_P1];
	p2 = APDUBuffer[OFFSET_P2];
	lc = APDUBuffer[OFFSET_LC];
	switch(ins)
	{
		case INS_INITIALIZE_UPDATE:
			checkInitializeUpdate_param(p1, p2, lc);
			break;

		case INS_EXTERNAL_AUTHENTICATION:
			checkExternalAuthentication_param(p1, p2, lc);
			break;

#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)
		case INS_OP_BEGIN_RMAC_SESSION:
			checkBeginRmacSession_param(p1, p2, lc);
			break;

		case INS_OP_END_RMAC_SESSION:
			checkEndRmacSession_param(p1, p2, lc);
			break;
#endif

		case INS_OP_MANAGE_SECURITY_ENVIRONMENT:
			checkManageSecurityEnvironment_param(p1, p2, lc);
			break;

		case INS_OP_PERFORM_SECURITY_OPERATION: // 0x2A
			checkPerform_Security_Operation_param(p1, p2, lc);
			break;

		case INS_OP_GET_CHALLENGE:
			checkGetChallenge_param(p1, p2, lc);
			break;

		case INS_OP_INTERNAL_AUTHENTICATION:
			checkInternalAuthentication_param(p1, p2, lc);
			break;

		case INS_OP_GET_DATA:
			//resLen = (u16)(getData(APDUBuffer)&0x00FF);
			break;

		case INS_OP_GENERATE_KEYPAIR:
			checkGenerateKeyPair_param(p1, p2, lc);
			break;

		default:
			break;
	}
}

/*
 *
 *
 *
 * */

u16 check_scp_i_param(u8 checkTpye)
{
	u8 p1;
	u8 bScpTpye;
	u8 bIoption; // Table 5- 1:    Values of Parameter  “ i”
	u16 result;

	result = SW_NO_ERROR;
	bScpTpye = g_current_SCP_Type;
	bIoption = g_current_SCP_I_Option;

	if(CHECK_IS_SUPPORT_SCP_AND_PARAM_I == checkTpye)
	{
		if( (SCP_02 == bScpTpye) || (SCP_01 == bScpTpye) )
		{
			// the SCP's "i" param we support "15","35","55","75",
			if( ( SCP0102_IPARA_15 != U8_AND_U8( bIoption, SCP0102_IPARA_15 ) ) )
			{
				result = SW_AUTHCOUNTER_EXPLORED;
			}
		}
		else
		{
			if( ( SCP03_IPARA_NOTSUPPROT == U8_AND_U8( bIoption, SCP03_IPARA_USEDBIT ) ) )
			{
				result = SW_AUTHCOUNTER_EXPLORED;
			}
		}
	}
	// checkTpye = CHECK_IS_SUPPORT_RMAC
	else
	{
		p1 = APDUBuffer[OFFSET_P1];

		if( (SCP_02 == bScpTpye) || (SCP_01 == bScpTpye) )
		{
			if(0x10 == U8_AND_U8(p1, 0x10))
			{
				if(SC0102_RMAC_SUPPORT != U8_AND_U8(bIoption, SC0102_RMAC_SUPPORT ))
				{
					result = SW_CONDITIONS_NOT_SATISFIED;
				}
			}
		}
		else
		{
			if(0x10 == U8_AND_U8(p1, 0x30)) // 本次鉴权级别要求支持RMAC
			{
				if(SC03_SUPPROT_RMAC_NORENC != U8_AND_U8(bIoption, SC03_SUPPROT_RMAC_NORENC )) // modify by xuxiaojun@20220615: SCP11测试脚本  scp03_auth_test_isd
//				if(SC03_SUPPROT_RMAC_NORENC != U8_AND_U8(bIoption, SC03_SUPPROT_RMAC_RENC ))
				{
					result = SW_CONDITIONS_NOT_SATISFIED;
				}
			}
			else if(0x30 == U8_AND_U8(p1, 0x30)) // 本次鉴权级别要求支持RMAC & R - ENCRYPTION
			{
				if(SC03_SUPPROT_RMAC_RENC != U8_AND_U8(bIoption, SC03_SUPPROT_RMAC_RENC ))
				{
					result = SW_CONDITIONS_NOT_SATISFIED;
				}
			}
		}
	}

	return result;
}


/*
 *
 *
 * */

u16 check_security_level(void)
{
	u8 p1;
	u8 current_security_level;

	current_security_level = channelInfo.security_level[selected_channel];

	// if R-ENCRYPTION is specified in a previous EXTERNAL AUTHENTICATE command;
	if( SCF_RMAC_ENCRY_MASK == U8_AND_U8(SCF_RMAC_ENCRY_MASK, current_security_level) )
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	p1 = APDUBuffer[OFFSET_P1];

	//If P1 is set to '10' and R-MAC is specified  in a previous EXTERNAL AUTHENTICATE command;
	if( SCF_RMAC_MASK == U8_AND_U8(SCF_RMAC_ENCRY_MASK, current_security_level) )
	{
		if(0x10 == p1)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
	}

	//If C-MAC was not specified in a previous EXTERNAL AUTHENTICATE command;
	if( SCF_CMAC_MASK != U8_AND_U8(SCF_CMAC_MASK, current_security_level) )
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	//If P1 is set to '30' and C-DECRYPTION was not specified in a previous EXTERNAL AUTHENTICATE command.
	if( SCF_DECRY_MASK != U8_AND_U8(SCF_DECRY_MASK, current_security_level) )
	{
		if(0x30 == p1)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
	}

	return SW_NO_ERROR;
}


u8 checkSecureLevel(PX8 apduBuf, u8 bSecureLevel, PX8 needDecryptCommandData, PX8 needVerifyMAC ) // modify by liweiming
{
	u8 result;

	result = RESULT_NOK;

	if(SCP_10 == g_current_SCP_Type)
	{
		if( (0x00 == U8_AND_U8(bSecureLevel, 0x02) ) && (0x88 == apduBuf[OFFSET_CLA]) )
		{
			return result;
		}
		if( (0x02 == U8_AND_U8(bSecureLevel, 0x02)) && (0x00 == U8_AND_U8(apduBuf[OFFSET_CLA], 0x08)) )
		{
			return result;
		}
	}
#if GP__SCP__SCP10__FOR_MASK
	//以下为修改处，检查是否有相应的会话秘钥
	if ((bSecureLevel&0x10)==0x10)//R-mac
	{
	  if(BIT_2 != (BIT_2 & sesKeyFlag))//无R-MAC会话密钥
		{
			closeSecuritychannel(SC_TERMINATED);
			//result = SW_SECURITY_STATUS_NOT_SATISFIED;
			throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
		}
	}
	if ((bSecureLevel&0x20)==0x20)//R-enc
	{
	 if(BIT_6 != (BIT_6 & sesKeyFlag))//无R-enc会话密钥
		{
			closeSecuritychannel(SC_TERMINATED);
			//result = SW_SECURITY_STATUS_NOT_SATISFIED;
			throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
		}
	}
#endif



	PX_W_U8(needDecryptCommandData, OK);
	PX_W_U8(needVerifyMAC, OK);

	switch(bSecureLevel)
	{
		case SCF_AUTHENTICATED:
		case SCF_AUTHENTICATED_ANY:
			if( 0x00 != U8_AND_U8(apduBuf[OFFSET_CLA], 0x04) )
			{
				break;
			}
			// no break
		case SCF_R_MAC:
		case SCF_R_MAC_ANY:	
			result = RESULT_OK;
			PX_W_U8(needVerifyMAC, NOK);
			PX_W_U8(needDecryptCommandData, NOK);
		break;

		case SCF_C_MAC:
		case SCF_C_MAC_R_MAC:
		case SCF_C_MAC_ANY:
		case SCF_C_MAC_R_MAC_ANY:	
			if(SCP_02 == g_current_SCP_Type)
			{
				if( 0x04 != U8_AND_U8(apduBuf[OFFSET_CLA], 0x04) )
				{
					break;
				}
			}
#if GP__AMENDMENT__D
			if(SCP_03 == g_current_SCP_Type) // modify by xuxiaojun@20220531
			{
				if( 0x04 != U8_AND_U8(apduBuf[OFFSET_CLA], 0x04) )
				{
					break;
				}
			}
#endif
			result = RESULT_OK;
			PX_W_U8(needDecryptCommandData, NOK);
			break;

		case SCF_C_MAC_DECRYPTION:
		case SCF_C_MAC_R_MAC_DECRYPTION:
		case SCF_C_MAC_DECRYPTION_ANY:
		case SCF_C_MAC_R_MAC_DECRYPTION_ANY:
			if(SCP_02 == g_current_SCP_Type)
			{
				if( 0x04 != U8_AND_U8(apduBuf[OFFSET_CLA], 0x04) )
				{
					break;
				}
			}
#if GP__AMENDMENT__D
			if(SCP_03 == g_current_SCP_Type) // modify by xuxiaojun@20220531
			{
				if( 0x04 != U8_AND_U8(apduBuf[OFFSET_CLA], 0x04) )
				{
					break;
				}
			}
#endif
			result = RESULT_OK;
			break;

		case SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION:
		case SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION_ANY:	
		case (U8_OR_U8(SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION, SCF_ZERO_APDU_RMAC_FLAG)):// SCP03 P1 的bit2设置时 会把SCF_ZERO_APDU_RMAC_FLAG设置上   ，参考verifyExternalAuthenticate
			if((SCP_03 == g_current_SCP_Type)||(SCP_11 == g_current_SCP_Type) || (SCP_10 == g_current_SCP_Type))// modify by xuxiaojun@20220608: SCP11协议支持的安全级别, 测试用例：SCP11测试脚本　SCP11c_auth_level_Test_isd
			{
				result = RESULT_OK;
			}
			break;

		case SCF_DECRYPTION:
		case SCF_ENCRYPTION_DECRYPTION:
		case SCF_R_MAC_DECRYPTION:
		case SCF_R_MAC_DECRYPTION_ENCRYPTION:
		case SCF_DECRYPTION_ANY:
		case SCF_ENCRYPTION_DECRYPTION_ANY:
		case SCF_R_MAC_DECRYPTION_ANY:
		case SCF_R_MAC_DECRYPTION_ENCRYPTION_ANY:
			if( SCP_10 == g_current_SCP_Type )
			{
				result = RESULT_OK;
				PX_W_U8(needVerifyMAC, NOK);
			}
			break;

		case SCF_C_MAC_ENCRYPTION:
		case SCF_C_MAC_R_MAC_ENCRYPTION:
		case SCF_C_MAC_ENCRYPTION_ANY:
		case SCF_C_MAC_R_MAC_ENCRYPTION_ANY:
			if( SCP_10 == g_current_SCP_Type )
			{
				result = RESULT_OK;
				PX_W_U8(needDecryptCommandData, NOK);
			}
			break;

		case SCF_ENCRYPTION:
		case SCF_R_MAC_ENCRYPTION:
		case SCF_ENCRYPTION_ANY:
		case SCF_R_MAC_ENCRYPTION_ANY:
			if( SCP_10 == g_current_SCP_Type )
			{
				result = RESULT_OK;
				PX_W_U8(needVerifyMAC, NOK);
				PX_W_U8(needDecryptCommandData, NOK);
			}
			break;

		case SCF_C_MAC_DECRYPTION_ENCRYPTION:
		case SCF_C_MAC_DECRYPTION_ENCRYPTION_ANY:
			if( SCP_10 == g_current_SCP_Type )
			{
				result = RESULT_OK;
			}
			break;

		default:
			break;
	}


	return result;
}
/*
*@function   : checkIsSDAuthOnOtherLogicChannel
*
*@description: 检查当前鉴权时使用的key所属的SD是否在其他通道上已经被使用。联通测试log需求
*
*@param       :void
*
*@return      :void
*
*@history     :V1.0 :create by xuxiaojun@20220705 联通测试转log: F2_TC6_ISDAMSSDTest的TS1.7 在ISD下创建AM SSD，未对此SSD进行个人化，0通道上选择SSD使用ISD密钥建立安全通道后，ISD不能在其他通道上建立安全通道
*/
void checkIsSDAuthOnOtherLogicChannel(void)
{
	u8		X_A		tempI;
	
	if(SELECTED_KEY_APPID)
	{
		for(tempI = 0; (tempI < MAX_LOGICAL_CHANNELS); U8_INC(tempI))
		{
			if (tempI == selected_channel )
			{
				continue;
			}

			if(channelInfo.selected_key_appId[tempI] == channelInfo.selected_key_appId[selected_channel])
			{
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
		}
	}
}

/**
* modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
*/
u16 checkIsSDAcitveOnOtherLogicChannel()
{
	u8		X_A		sdId;
	u8		X_A		tempI;
	u16     X_A     result;

	result = SW_NO_ERROR;

	sdId = g_bSDId[selected_channel];

	if( ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])
	{
		// 当 该SD 正在 用于 SCP80/SCP81 时，不能再建立SCP02 安全通道，否则抛6D00
		if(sdId == bRamBuffer[REMOTE_SCP_SDID])
		{
			//throw_error(SW_INS_NOT_SUPPORTED); //测试用例T_006_HTTP_NoSeg_RAM_DownloadApplet_ISDTargeted
		}
	}
	for(tempI = 0; (tempI < MAX_LOGICAL_CHANNELS); U8_INC(tempI))
	{
		if (tempI == selected_channel )
		{
			continue;
		}
		if ( ( (getChannelContext(tempI) == getChannelContext(selected_channel)) || (sdId == g_bSDId[tempI]) )
			&& (channelInfo.security_level[tempI] > SCF_NO_AUTHENTICATED) )
		{
			if(0x00 != channelInfo.sercrity_channel_number[tempI])
			{
				//修改为了兼容以前的存量卡而增加的8050指令，之后通过ISD下载应用，结束HTTPS后，本地逻辑通道1上ISD建立安全信道返回6985的bug,by xuxiaojun 20240308
				if(channelInfo.security_level[tempI] != SCF_80_AUTHENTICATED)
				{
				#if (EUICC_ONLY||SE_AND_EUICC)
					if(getChannelProfileIndex(tempI) == getChannelProfileIndex(selected_channel))
				#endif
					{
						result = SW_CONDITIONS_NOT_SATISFIED; //throw_error(SW_CONDITIONS_NOT_SATISFIED);
					}
				}
			}
		}
	}

	return result;
}

#define SSD_LOCKED_MASK						0x80
#define SSD_PERSONALIZED_MASK						0x0F
u8 checkSDIsLocked(memref theAppletTableEntry, u8 bOp)
{
	u8 		X_A		appStatus;
	appStatus = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theAppletTableEntry), bCurrentState_INDEX)));
	
		if (IS_NO_NEED_CHECK_SD == bOp)
		{
			return 0;
		}
		else if ((IS_NEED_CHECK_SD_LOCKED == U8_AND_U8(IS_NEED_CHECK_SD_LOCKED, bOp)) && (SSD_LOCKED_MASK == U8_AND_U8(appStatus ,SSD_LOCKED_MASK)))
		{
			throw_error(SW_AUTHCOUNTER_EXPLORED);
		}
		else if(((IS_NEED_CHECK_SD_PERSONALIZED == U8_AND_U8(IS_NEED_CHECK_SD_PERSONALIZED, bOp))) && (SSD_PERSONALIZED_MASK ==U8_AND_U8 (appStatus, SSD_PERSONALIZED_MASK)))
		{
				return SSD_PERSONALIZED_MASK;
		}

		return 0;
}
/*
* merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP:兼容GP2.3中usage两字节的情况
*/
u16 checkKeyUsageQualifier(PN8 keySetObj)
{
	u8 keyUsageQualifier[2];

	ecpMem_Set(keyUsageQualifier, 0x00, sizeof(keyUsageQualifier));

	switch(g_current_SCP_Type)
	{
		case SCP_02:
		{
			keyData_getKeyUsageQualifier(keySetObj, KEY_ID_01, &keyUsageQualifier[0]);

			if(keyUsageQualifier[0] != 0)
			{
				if((keyUsageQualifier[0] & 0x18) != 0x18)
				{
					throw_error(SW_CONDITIONS_NOT_SATISFIED);
				}
			}

			keyData_getKeyUsageQualifier(keySetObj, KEY_ID_02, &keyUsageQualifier[0]);

			if(keyUsageQualifier[0] != 0)
			{
				if((keyUsageQualifier[0] & 0x14) != 0x14)
				{
					throw_error(SW_CONDITIONS_NOT_SATISFIED);
				}
			}

			keyData_getKeyUsageQualifier(keySetObj, KEY_ID_03, &keyUsageQualifier[0]);

			if(keyUsageQualifier[0] != 0)
			{
				if((keyUsageQualifier[0] & 0x48) != 0x48)
				{
					throw_error(SW_CONDITIONS_NOT_SATISFIED);
				}
			}
		}
	}

	return 0;
}
u16 checkSequenceCount(PN8 keySetObj)
{
	u32		X_A		bSeqCounter;
	PN8 			seqCounterField;

	switch(g_current_SCP_Type	)
	{
		case SCP_01:
		case SCP_02:
			seqCounterField = MEMtPN8(keySet_getSequenceCounter(keySetObj, U32t8(SCP02_SEQUENCE_CONTER_SIZE)));
			bSeqCounter = PN_R_U16(seqCounterField);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			if(MAX_AUTHENTICATION_TIME == bSeqCounter)
			{
				throw_error(MAX_AUTHENTICATION_ERROR);
			}
			else if (MAX_AUTHENTICATION_NEAR <= bSeqCounter)
			{				
				return (u16)MAX_AUTHENTICATION_WARNING;
			}
#else
			if (MAX_AUTHENTICATION_NEAR == bSeqCounter)
			{
				throw_error(MAX_AUTHENTICATION_NEAR);
			}
			else if(MAX_AUTHENTICATION_TIME == bSeqCounter)
			{
				throw_error(MAX_AUTHENTICATION_TIME);
			}
#endif
		break;
		case SCP_03:
			seqCounterField = MEMtPN8(keySet_getSequenceCounter(keySetObj, SCP03_SEQUENCE_CONTER_SIZE));
			bSeqCounter = readScp03Counter(seqCounterField);//PN_R_U24(seqCounterField);

			if (MAX_AUTHENTICATION_NEAR == bSeqCounter)
			{
				throw_error(MAX_AUTHENTICATION_NEAR);
			}
			else if(MAX_AUTHENTICATION_TIME == bSeqCounter)
			{
				throw_error(MAX_AUTHENTICATION_TIME);
			}

		break;

	}
	return 0;
}

void checkTryAuth(PN8 keySetObj)
{
	u8 		X_A		bTryAuth;
	u8    	X_A		bMaxAuth;
	u32		X_A		dLoop;
	u32		X_A		dCnt;
	PN8			    sdDataObj;
	bTryAuth = keySet_getTryAuthAttenpt(keySetObj);
	//获取当前sdDataObj
	sdDataObj = MEMtPN8(getCurrentSdObj(IS_NEED_CHECK_SD_LOCKED));
	bMaxAuth = sdData_getAuthFailCount(sdDataObj);
	if (bMaxAuth != 0xFF)
	{
		U8_INC(bTryAuth);
		keySet_setTryAuthAttenpt(keySetObj, bTryAuth);
		if(bTryAuth > ANTI_AUTH_ATTACK_LIMIT)
		{
			dLoop = U32_OR_U32(U32_SHL_U8((U8t32(bTryAuth)), 24), 0x00FFFFFF);
			for(dCnt = 0; dCnt < dLoop; U32_INC(dCnt))
			{
				;
			}
		}
	}
/*	
	if (bTryAuth >= JcvmConfig_GetCmAuthMaxCount()) // nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(CM_AUTH_MAX_TRY_CNTR))))
	{
		nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetCardStateBase()), CARD_TERMINATED);
		throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
	}
	else
	{
		U8_INC(bTryAuth);
		keySet_setTryAuthAttenpt(keySetObj, bTryAuth);
	}
*/
}

#endif
