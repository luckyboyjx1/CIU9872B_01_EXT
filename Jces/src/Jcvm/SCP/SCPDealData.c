/*
 * SCPDealData.c
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */


#include "SCPInterface.h"
#include "CardManager.h"
#include "Loader.h"
#include "opFunction.h"
#include "global_variable.h"
#include "error.h"
#include "profile.h"
#include "SCP01.h"
#include "SCP02.h"
#include "SCP03.h"
#include "SCP10.h"
#include "SCPDealData.h"
#include "ob_manager.h"
#include "keySet.h"
#include "casdStoreKey.h"
#include "crypto_impl.h"
#include "keyData.h"
#include "RSA_API.h"
#include "object.h"
#include "keyGroup.h"
#include "fm03_keyData.h"
#include "dataManager.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


#if GP__SCP

u8 add80Padding(PX8 SrcAddr, u8 len, u8 blockSize)
{
	u8 		X_A		blockNum;
	u8 		X_A		keyPaddingOff;
	u8 		X_A		tempLen;
	PX8				tempPtr;

	if(DES_BLOCKSIZE == blockSize)
	{
		keyPaddingOff = DES_KEY_PADDING_OFF;
	}
	else
	{
		keyPaddingOff = AES_KEY_PADDING_OFF;
	}

	blockNum = U8_SHR_U8(len, keyPaddingOff);
	U8_INC(blockNum);

	tempLen = U8_SHL_U8(blockNum, keyPaddingOff);

	// do padding  0x80
	tempPtr = PX8_A_U16(SrcAddr , len);
	PX_W_U8(tempPtr, CRYPTO_PADDING_80);

	U8_INC(len);
	tempPtr = PX8_A_U16(tempPtr , 0x01);

	tempLen = U8_S_U8(tempLen, len);
	ecpMem_Set(tempPtr, CLEARED_VALUE, tempLen);

	return blockNum;
}



//compare hostChallenge
u8 compareRamData(PX8 firstRamData, PX8 secondRamData, u8 len)
{
  	u8		X_A		tempI;
  	u8		X_A		bOK;

	bOK = OK;
	for (tempI = 0; tempI < len; U8_INC(tempI))
	{
		if (PX_R_U8(firstRamData) != PX_R_U8(secondRamData))
		{
			bOK = NOK;
			// do not break; security check.
		}
		firstRamData = PX8_A_U16(firstRamData, 0x01);
		secondRamData = PX8_A_U16(secondRamData, 0x01);
	}
	return (bOK);
}

/*
* merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support SCP11
*/
u8 verifyMAC(PX8 apduBuf)
{
	u8 		X_A		savaForAPDUCLA;
	u8 		X_A		bOK;
	u8 		X_A		len;
	u8 		X_A		bLc;
	u8    	X_A    	outTempBuf[8];
#if GP__SCP__SCP10__FOR_MASK
	u8 		X_A		bLe;

	bLe = apduBuf[U8_A_U8(OFFSET_CDATA, bLc)];
#endif
	u8     X_A      DATA_OFF = 0;

	bLc = apduBuf[OFFSET_LC];
	bOK = NOK;
	
	
	if (bLc < 8)
	{
		bOK = OTHER;
	}
	else
	{
		//the logic channel number is not used in verifyMac
		savaForAPDUCLA = apduBuf[OFFSET_CLA];
		apduBuf[OFFSET_CLA] = U8_AND_U8(apduBuf[OFFSET_CLA], 0xFC);

		// offset to MAC : APDUHEAD(5 Byte) + DATALEN - 8
		len = U8_S_U8(bLc, MACLENGTH_SUB_APDUHEADSIZE);

		ecpMem_Set(MEMtPX8(TEMPBuffer), CLEARED_VALUE, 16);

		//copy MAC to temp buffer
		ecpMem_Copy(PX8_A_U16(TEMPBuffer , 0x10), PX8_A_U16(apduBuf , len), MAC_LENGTH);

		switch(g_current_SCP_Type)
		{
#if GP__SCP__SCP01
			case SCP_01:
				CalMACSCP01(apduBuf, len, g_current_SCP_MAC_KEY, g_current_SCP_LAST_MAC);
				break;
#endif
			case SCP_02 :
				CalMACSCP02(apduBuf, len, MEMtPM8(g_current_SCP_MAC_KEY), MEMtPM8(g_current_SCP_LAST_MAC));
				break;
#if GP__AMENDMENT__D
			case SCP_03:
			case SCP_11:
#if M2M_EUICC
				if(g_SCP_SCP03t_Flag == 1)//scp03t格式转标准APDU后 计算mac需要还原原数据计算
				{
					apduBuf[2] = 0x85;  //85 11 33 + host cryptogram
					apduBuf[3] = bLc+1;
					apduBuf[4] = 0x33;
					len = U8_S_U8(bLc, 5);// TLV(3bytes) + DATA LEN - 8(MAC LEN)
					DATA_OFF = 2;
				}
#endif
				SCP03_CalMAC(PX8_A_U8(apduBuf,DATA_OFF), len, MEMtPX8(g_current_SCP_MAC_KEY), g_SCP_macKeyLen);// 在该函数中借用TEMPBuffer缓存上一条C-MAC
				//ecpMem_Copy(g_current_SCP_LAST_MAC, TEMPBuffer, AES_BLOCKSIZE);//save c-mac ,use for r-mac
			    memCopyNoAtomic(MEMtPM8((memref)g_current_SCP_LAST_MAC),MEMtPM8((memref)TEMPBuffer),AES_BLOCKSIZE);// the full 16- byte C - MAC  of the previous command becomes the “ MAC chaining value ”  for the subsequent C - MAC  verification / R- MAC generation
				break;
#endif

#if GP__SCP__SCP10__FOR_MASK
			case SCP_10:
				if( NOK == scp10_dealMacData(bLe, BEFORE_MAC_CAL) )
				{
					return bOK;
				}
				// offset to tag
				len = (u8)U8_S_U8(len, 2);
				CalMACSCP10(apduBuf, len, g_current_SCP_MAC_KEY, CMAC_CAL);
				break;
#endif
			default:
				return bOK;
		}

		//after verifyMac ,then recover the logic channel number
		apduBuf[OFFSET_CLA] = savaForAPDUCLA;

		bOK = compareRamData((PX8_A_U16(MEMtPX8(TEMPBuffer) , 0x10)), MEMtPX8(TEMPBuffer), MAC_LENGTH);

		//after compare MAC ,Clear the TEMPBuffer.
		ecpMem_Set(TEMPBuffer, CLEARED_VALUE, MAC_LENGTH);
  

		if ( SCP_02 == g_current_SCP_Type )
		{
			//update MAC if needed
			if( SC0102_ICV_FOR_CMAC == U8_AND_U8(g_current_SCP_I_Option, SC0102_ICV_FOR_CMAC) )
			{
				single_DESMAC(MEMtPX8(g_current_SCP_LAST_MAC), MEMtPM8(g_current_SCP_MAC_KEY), outTempBuf, U8t16(8), MEMtPM8(TEMPBuffer));
				memCopyNoAtomic(MEMtPN8(g_current_SCP_LAST_MAC), outTempBuf, MAC_LENGTH);
			}
		}
	#if GP__SCP__SCP10__FOR_MASK
		if ( SCP_10 == g_current_SCP_Type )
		{
			if(OK == bOK)
			{
				//验证命令数据是否正确
				bOK = scp10_dealMacData(bLe, AFTER_MAC_CAL);
			}
			scp10_ICVSeqCntINC(g_current_SCP_cardChallenge, 0);

			if(BIT_7 == (BIT_7 & sesKeyFlag) )//CR-mac同顺序计数器
			{
				scp10_ICVSeqCntINC(g_current_SCP_hostChallenge, 0);
			}
			bLc = apduBuf[OFFSET_LC];
			apduBuf[U8_A_U8(bLc, OFFSET_CDATA)] = bLe;
		}
		else
	#endif		
		{
#if GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK || GP__VERSION__FOR_CHINA_TELECOM_MASK
			apduBuf[OFFSET_CLA] = U8_AND_U8(apduBuf[OFFSET_CLA], U8_INV(0x04)) ;
#endif
			apduBuf[OFFSET_LC] = U8_S_U8(apduBuf[OFFSET_LC], MAC_LENGTH);
		}

	}

	return bOK;
}

/*
 *
 *	Decrypy command data in APDU buffer
 *
 * */
u8 decryptCommandData(PX8 apduBuf)
{
	u8 		X_A		dataLen;
	u8 		X_A		bOK;
	u8 		X_A		blockSize;
	u8 		X_A		blockMask;
	u8 		X_A		bLe;
	PX8 			tempPtr;
	u8		X_A		mac[DES_BLOCKSIZE];

	bOK = OK;
	blockSize = 0;
	blockMask = 0;

	dataLen = apduBuf[OFFSET_LC];

	bLe = U16t8(U8_A_U8(dataLen, OFFSET_CDATA));
	bLe = apduBuf[bLe];

	if( 0x00 == dataLen )
	{
#if GP__AMENDMENT__D
		if((SCP_03 == g_current_SCP_Type) || (SCP_11 == g_current_SCP_Type))// modify by xuxiaojun@20220608: 对于鉴权之后使用SCP03算法的协议都应该等同于SCP_03, 测试用例：SCP11测试脚本　SCP11c_auth_level_Test_ssd
		{
			SCP03_increaseICVCount();
		}
#endif
	}
	else
	{
		// first .
#if GP__AMENDMENT__D
		if((SCP_03 == g_current_SCP_Type) || (SCP_11 == g_current_SCP_Type))// modify by xuxiaojun@20220608: 对于鉴权之后使用SCP03算法的协议都应该等同于SCP_03, 测试用例：SCP11测试脚本　SCP11c_auth_level_Test_ssd
		{
			blockSize = AES_BLOCKSIZE;
			blockMask = AES_BLOCKSIZE_MASK;
		}
		else
#endif
		{
			blockSize = DES_BLOCKSIZE;
			blockMask = DES_BLOCKSIZE_MASK;
		}

		// second deCrypto commandData.
		if (dataLen >= blockSize || dataLen == 0x03)//修改处，原代码为(dataLen >= blockSize)
		{
			switch(g_current_SCP_Type)
			{
				case SCP_01:
				case SCP_02:
					if(dataLen == DES_BLOCKSIZE)
					{
						return OK;
					}

					if (U8_AND_U8(dataLen, blockMask) != 0) // 密文长度要求是块的整数倍
					{
						bOK = NOK; // SW_WRONG_LENGTH;
					}
					dataLen = U8_S_U8(dataLen, blockSize);
					tempPtr = PX8_A_U16(apduBuf , OFFSET_CDATA);
					tempPtr = PX8_A_U8(tempPtr , dataLen);
					ecpMem_Copy(MEMtPX8(mac), tempPtr, blockSize);
					decryptCommandDataSCP0102(apduBuf, dataLen);
					break;
#if GP__AMENDMENT__D
				case SCP_03:
				case SCP_11:// modify by xuxiaojun@20220608: 对于鉴权之后使用SCP03算法的协议都应该等同于SCP_03, 测试用例：SCP11测试脚本　SCP11c_auth_level_Test_ssd
					if (U8_AND_U8(dataLen, blockMask) != 0) //modify by xuxiaojun@20220531 密文长度要求是块的整数倍
					{
						bOK = NOK; // SW_WRONG_LENGTH;
					}
					SCP03_decryptCommandData(apduBuf);
					break;
#endif

#if GP__SCP__SCP10__FOR_MASK
				case SCP_10:
					bOK = scp10_dealencData(bLe);
					if((OK == bOK) || (0 != apduBuf[OFFSET_LC]) )//解密数据-修改处
					{
						dataLen = decryptCommandDataSCP10();
					}
					break;
#endif
				default:
					break;
			}

		}

		// third dePadding deCryptoed commandData.
#if SCP_SWITCH
		if ( SCP_01 == g_current_SCP_Type )
		{
			dataLen = apduBuf[OFFSET_LC];
			tempPtr = PX8_A_U16(apduBuf , OFFSET_CDATA);
			//Ecp_MemCopy((u8 *) (apduBuf + OFFSET_CDATA), (u8 *) (apduBuf + OFFSET_CDATA + 1), (u8) dataLen);
			ecpMem_Copy(tempPtr, (PX8)(PX8_A_U16(tempPtr , 0x01)), (u8) dataLen);
		}
		else
#endif
		{
			tempPtr = PX8_A_U16(apduBuf , OFFSET_LC);
			tempPtr = PX8_A_U16(tempPtr , dataLen);

			while(( 0x00 == PX_R_U8(tempPtr)) && ( 0x00 != dataLen ) )
			{
				PX8_DEC(tempPtr);
				U8_DEC(dataLen);
			}

			if( CRYPTO_PADDING_80 == PX_R_U8(tempPtr))
			{
				if(dataLen != 0)
				{
					U8_DEC(dataLen);
				}
				else
				{
					bOK = NOK;
				}
			}
			else
			{
				bOK = NOK;//wrong decrypted data
// modify by xuxiaojun@20220531: 以下代码影响GP CI测试用例ccm_APDU_MAPPING_installForRegistryUpdate_03_noOpenSM7_0f_ba_de的正确性，且代码逻辑不严谨，无说明情况，暂时采取注释掉的处理方式
//				if(0 == APDUBuffer[OFFSET_LC] || APDUBuffer[OFFSET_CDATA] == APDUBuffer[U8_A_U8(OFFSET_CDATA, 2)]) //修改处
//					bOK = OK;
			}

			//update APDULc

			switch(g_current_SCP_Type)
			{
				case SCP_01:
				case SCP_02:
					apduBuf[OFFSET_LC] = U16t8(U8_A_U8(dataLen, MAC_LENGTH));
					// forth recover mac to apduBuf
					dataLen = U16t8(U8_A_U8(dataLen, OFFSET_CDATA));
					tempPtr = PX8_A_U16(apduBuf , dataLen);
					ecpMem_Copy(tempPtr, MEMtPX8(mac), MAC_LENGTH);
					break;
#if GP__AMENDMENT__D || GP__SCP__SCP10__FOR_MASK

#if GP__AMENDMENT__D
				case SCP_03:
				case SCP_11:// modify by xuxiaojun@20220608: 对于鉴权之后使用SCP03算法的协议都应该等同于SCP_03, 测试用例：SCP11测试脚本　SCP11c_auth_level_Test_ssd
#endif // end GP__AMENDMENT__D
#if GP__SCP__SCP10__FOR_MASK
				case SCP_10:
#endif // end GP__SCP__SCP10__FOR_MASK
					apduBuf[OFFSET_LC] = dataLen;
					dataLen = (u8)U8_A_U8(dataLen, OFFSET_CDATA);
					apduBuf[dataLen] = bLe;
					break;

#endif // end GP__AMENDMENT__D || GP__SCP__SCP10__FOR_MASK
				default:
					break;
			}

		}
	}

	return bOK;
}
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
void RmacUpdateInDecryptAndVerifyMAC(PX8 apduBuf)
{
	u8 		X_A		savaForAPDUCLA;
	u8 		X_A		tempI;

	//the logic channel number is not used in verifyMac
	savaForAPDUCLA = apduBuf[OFFSET_CLA];
	apduBuf[OFFSET_CLA] = U8_AND_U8(apduBuf[OFFSET_CLA], 0xF8);

	tempI = U8_A_U8(apduBuf[OFFSET_LC], OFFSET_CDATA);
	RmacUpdate(apduBuf, tempI);

	apduBuf[OFFSET_CLA] = savaForAPDUCLA;
	//appRmac = 1; //consider data that will be sent out
}
#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)
u16 decryptAndVerifyMAC(PX8 apduBuf,u8 bSecureLevel, u8 needDecryptCommandData, u8 needVerifyMAC)
{
	u16		X_A		result;

	result = SW_NO_ERROR;

	switch(g_current_SCP_Type)
	{
		case SCP_01:
		case SCP_02:
			if (needDecryptCommandData)
			{
				if( (INS_OP_GET_DATA != apduBuf[OFFSET_INS]) || (MAC_LENGTH != apduBuf[OFFSET_LC]) )
				{
					if (OK != decryptCommandData(apduBuf))
					{ //apduBuf[OFFSET_LC] - 8 <= 0 (8 byte MAC)
						closeSecuritychannel(SC_ABORTED);// modify by liweimin
						result = SW_SECURITY_STATUS_NOT_SATISFIED;
					}
				}
			}
			if (result == SW_NO_ERROR)
			{
				if (needVerifyMAC)
				{
					if (OK != verifyMAC(apduBuf))
					{ //apduBuf[OFFSET_LC] < 8
						closeSecuritychannel(SC_ABORTED);// modify by liweimin
//						if(CONFIG_GP_FOR_MAPPING_TEST)
//						{
//							result = SW_WRONG_DATA; 用例km_APDU_putKeyDes_03_NoSecureSession3 与这里冲突
//						}
//						else
						{
							result = SW_SECURITY_STATUS_NOT_SATISFIED;
						}
					}
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
					else
					{
//						apduBuf[OFFSET_LC] = U8_S_U8(apduBuf[OFFSET_LC], MAC_LENGTH);
						if ((bSecureLevel == SCF_C_MAC_R_MAC) || (bSecureLevel == SCF_C_MAC_R_MAC_DECRYPTION))
						{
							RmacUpdateInDecryptAndVerifyMAC(apduBuf);
						}
					}
#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)
				}
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
				else
				{
					RmacUpdateInDecryptAndVerifyMAC(apduBuf);
				}
#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC ||GP__SCP__SCP03_RMAC)
			}
			break;
		case SCP_03:
		case SCP_10:
		case SCP_11: //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support SCP11
			if (needVerifyMAC)
				/*&& (apduBuf[OFFSET_INS] != 0xCA || apduBuf[OFFSET_LC] != 8)*/
			{
				if(OK != verifyMAC(apduBuf))
				{
					closeSecuritychannel(SC_ABORTED);// modify by liweimin
					result = SW_SECURITY_STATUS_NOT_SATISFIED;
				}
//				else
//				{
//					apduBuf[OFFSET_LC] = U8_S_U8(apduBuf[OFFSET_LC], MAC_LENGTH);
//				}
			}
			if (SW_NO_ERROR == result)
			{
				if(needDecryptCommandData)
				{
					if (OK != decryptCommandData(apduBuf))
					{ //apduBuf[OFFSET_LC] - 8 <= 0 (8 byte MAC)
						closeSecuritychannel(SC_ABORTED);// modify by liweimin
						result = SW_SECURITY_STATUS_NOT_SATISFIED;
					}
				}

			}
			break;
		default:
			closeSecuritychannel(SC_TERMINATED);// modify by liweimin
			result = SW_SECURITY_STATUS_NOT_SATISFIED;
			break;

			

	}

	return result;
}


#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)

//for R-MAC generation, input command data
void RmacUpdate(PX8 SrcAddr, u16 length)
{

	u8 		X_A		len;
	u8	 	X_A		remainLen;
	u8 		X_A		tempI;
	PX8 			tempPtr;


	tempI = g_SCP_rmacTemp[0];
	len = U8_S_U8(RMAC_TEMP_LENGTH, tempI);

	if(U8t16(len) > length)
	{
		//Ecp_MemCopy((u8 *)&rmacTemp[rmacTemp[0]+1], (u8 *)SrcAddr, length);
		len = U8_A_U8(tempI, length);
		U8_INC(tempI);
		tempPtr = PX8_A_U8(MEMtPX8(g_SCP_rmacTemp) , tempI);
		ecpMem_Copy(tempPtr, SrcAddr, length);
	}
	else
	{
		// offset to rmacTemp[1], and copy data in the rmacTemp to TEMPBuffer
		tempPtr = PX8_A_U16(MEMtPX8(g_SCP_rmacTemp) , 0x01);
		ecpMem_Copy(TEMPBuffer, tempPtr, tempI);

		// copy some data from SrcAddr, and the length added is RMAC_TEMP_LENGTH
		remainLen = U16t8(U16_S_U8(length, len));
		tempPtr = PX8_A_U8(MEMtPX8(TEMPBuffer) , tempI);
		ecpMem_Copy(tempPtr, SrcAddr, len);

		single_DESMAC(MEMtPX8(TEMPBuffer), MEMtPM8(g_current_SCP_RMAC_KEY),MEMtPX8(TEMPBuffer), DES_BLOCKSIZE, g_current_SCP_LAST_MAC_L);

		tempPtr = PX8_A_U8(SrcAddr , len);
		len = U8_AND_U8(remainLen, 0x0007);
		remainLen = U8_AND_U8(remainLen, 0xFFF8);

		single_DESMAC(tempPtr, MEMtPM8(g_current_SCP_RMAC_KEY), TEMPBuffer, remainLen, g_current_SCP_LAST_MAC_L);
		tempPtr = PX8_A_U8(tempPtr , remainLen);

		// offset to rmacTemp[1], and copy data remain in the SrcAddr to rmacTemp
		ecpMem_Copy(PX8_A_U16(MEMtPX8(g_SCP_rmacTemp) , 0x01), tempPtr, len );
	}

	g_SCP_rmacTemp[0] = len;
}

#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC ||GP__SCP__SCP03_RMAC)

/*
 * add by jxh 20130902
 *
 * if SCPType's parameter 'i' generate different type CardChallenge
 *
 * in:
 * 	scpType: current SCP Type
 * 	i_option : current SCP Type's parameter 'i'
 * out:
 *
 * */
void generateCardChallenge(PX8 keySetObj, u8 comeInOrder)
{
	u32 	X_A 		bCurentAidAdd;
	u8 		X_A		bCurentAidLen;
	u8 		X_A		bTmpBuf[CARD_CHALLENGE_LENGTH];
	bCurentAidAdd =  getCurentAidAddrss(&bCurentAidLen);

	switch(g_current_SCP_Type)
	{
		case SCP_01:
		case SCP_02:

			if(SC0102_PSEU_RAND == U8_AND_U8( g_current_SCP_I_Option, SC0102_PSEU_RAND )||( bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA))
			{
				if(SECOND_COME_GENERATE_CARD_CHALLENGE == comeInOrder)
				{
					//generateRandomMode = GEN_RANDOM_MODE_1;
					generatePseudoRandomSCP02(PX8_A_U16(g_current_SCP_cardChallenge, 0x02), bCurentAidAdd, bCurentAidLen);
				}
				return;
			}
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			if(SC03_PSE_RAND_CARD_CHALL == ( g_current_SCP_I_Option & SC03_PSE_RAND_CARD_CHALL ))
			{
				if(FIRST_COME_GENERATE_CARD_CHALLENGE == comeInOrder)
				{
					//generateRandomMode = GEN_RANDOM_MODE_2;
					SCP03_generatePseudoRandom(keySetObj, g_current_SCP_cardChallenge, bCurentAidAdd, bCurentAidLen);
				}
				return;
			}
			break;
#endif
		case SCP_10:
			break;
		default:
			return;
	}

	if(FIRST_COME_GENERATE_CARD_CHALLENGE == comeInOrder)
	{
		
		Random_number_generator(MEMtPX8(bTmpBuf), CARD_CHALLENGE_LENGTH, 0x01 );
		memCopyNoAtomic(MEMtPN8(g_current_SCP_cardChallenge),bTmpBuf,CARD_CHALLENGE_LENGTH);//modify by jiangxiaohui@20220324 分层
	}

	return;
}



u8 generateOrCompareCardCryptogram(u8 bOp )
{
#define 		CARD_CRYPRTOGRAM_LENGTH 			0x08
#if GP__AMENDMENT__D
	u8 		X_A		derivationData[80];
#else
	u8 		X_A		derivationData[8];
#endif
	PX8 				pSrc;
	PX8 				copypDes;
	PX8 				firstChallenge;
	PX8 				secondChallenge;
	u8 		X_A			compareResult;


	compareResult = NOK;
	firstChallenge 	= MEMtPX8(g_current_SCP_hostChallenge);
	secondChallenge = MEMtPX8(g_current_SCP_cardChallenge);

	switch(g_current_SCP_Type)
	{
		case SCP_01:
		case SCP_02:

			if( COMPARE_CARD_CRYPRTOGRAM == bOp )
			{
				firstChallenge  = MEMtPX8(g_current_SCP_cardChallenge);
				secondChallenge = MEMtPX8(g_current_SCP_hostChallenge);
			}
			generateCardCryptogramSCP0102(firstChallenge, secondChallenge, derivationData);
			copypDes = PX8_A_U16(APDUBuffer , CARD_CRYPTOGRAM_SCP0102_OFFSET_INAPDU);
			pSrc = MEMtPX8(derivationData);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			SCP03_generateCardCryptogram(firstChallenge, secondChallenge, derivationData, bOp);
			copypDes = (PX8)PX8_A_U16(APDUBuffer , CARD_CRYPTOGRAM_SCP03_OFFSET_INAPDU);
			pSrc = (PX8) TEMPBuffer;
			break;
#endif
		default:
			return compareResult;
	}
	if( GENERATE_CARD_CRYPRTOGRAM == bOp )
	{
		compareResult = compareRamData(firstChallenge, secondChallenge, CARD_CRYPRTOGRAM_LENGTH);
		if(OK == compareResult)
		{
			//collit test : initialize update if hostChallenge equal cardChallenge then throw 6982.
			throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
		}
		ecpMem_Copy( copypDes, pSrc, CARD_CRYPRTOGRAM_LENGTH);
	}
	else
	{
		copypDes = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
		compareResult = compareRamData(pSrc, copypDes, CARD_CRYPRTOGRAM_LENGTH);
	}
	return compareResult;
}


void generateSecureChannelNumber(void)
{
	u8 		X_A		para1;
	u8 		X_A		para2;
	u8 		X_A		block_number;
	para1 = NOK;
	while ( NOK == para1 )
	{
		para1 = OK;
		Random_number_generator(MEMtPX8(&block_number), 0x01, 0x01);
		if (block_number >= U32t8( 0x80))
		{
			para1 = NOK;
			continue;
		}
		for (para2 = 0; para2 < U16t8(MAX_LOGICAL_CHANNELS); U8_INC(para2))
		{
			if ((channelInfo.sercrity_channel_number[para2] == block_number))
			{
				para1 = NOK;
				break;
			}
		}
	}

	channelInfo.sercrity_channel_number[selected_channel] = block_number;
	bRamBuffer[CURRENT_SECURITY_LOGICAL_NUMBER] = block_number;
}



void cipherSCP(PX8 dataIn, PX8 keyBuff, PX8 dataOut, u16 length, PX8 ICVBuffer, u8 mode, u8 blockSize, u8 isNeedDoPadding)
{

	u32 	X_A		tempBufferForCal[0x20];

	memCopyToRam(MEMtPN8(tempBufferForCal), dataIn, length);

	if(IS_NEED_DO_PADDING_YES == isNeedDoPadding)
	{
		add80Padding(MEMtPX8(tempBufferForCal), U16t8(length), blockSize);
	}

	switch(g_current_SCP_Type)
	{
		case SCP_01:
		case SCP_02:
			Triple_DES(MEMtPX8(tempBufferForCal), keyBuff, MEMtPX8(tempBufferForCal), blockSize, ICVBuffer, mode, SENSITIVE_DATA_MASKED);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
		case SCP_11:// todo: 这里是否需要增加SCP11?
//			AES((PX8)tempBufferForCal, keyBuff, (u16)(g_SCP_dekKeyLen << 3), (PX8)tempBufferForCal, blockSize, ICVBuffer, mode);
			SymCipherDofinal(MEMtPM8(tempBufferForCal), blockSize, MEMtPM8(tempBufferForCal), keyBuff, (u16)(g_SCP_dekKeyLen << 3), ICVBuffer, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, mode, SBC_INIT, ALG_AES);
			break;
#endif //GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			Triple_DES((PX8)tempBufferForCal, keyBuff, (PX8)tempBufferForCal, blockSize, ICVBuffer, mode, SENSITIVE_DATA_MASKED);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:

			break;
	}

	memCopyAtomic(dataOut, MEMtPM8(tempBufferForCal), U8t16(blockSize));

}

u16 dealcipherData(PX8 inBuffer, PX8 keyBuff, PX8 outBuffer, u16 inlength, u8 mode, u8 blockSize)
{

	u8		X_A		ICV[16];
	u16 	X_A		tempI;
	PX8 			tempPtrIn;
	PX8 			tempPtrOut;

	ecpMem_Set(ICV, CLEARED_VALUE, sizeof(ICV));

	tempI = inlength;
	tempPtrIn = inBuffer;
	tempPtrOut = outBuffer;

	while(inlength >= U8t16(blockSize))
	{
		cipherSCP(tempPtrIn, keyBuff, tempPtrOut, blockSize, MEMtPX8(ICV), mode, blockSize, IS_NEED_DO_PADDING_NO);
		tempPtrIn = PX8_A_U8(tempPtrIn , blockSize);
		tempPtrOut = PX8_A_U8(tempPtrOut , blockSize);
		inlength = U16_S_U8(inlength, blockSize);
	}

	if (0x00 != inlength)
	{
		tempI = U16_S_U16(tempI, inlength);
		tempI = U32t16(U16_A_U8(tempI, blockSize));
		cipherSCP(tempPtrIn, keyBuff, tempPtrOut, inlength, MEMtPX8(ICV), mode, blockSize, IS_NEED_DO_PADDING_YES);
	}
	return tempI;
}

/***************************************************************************************
 * This method used for op cipher.
 *
 * only for ECB mode
 * combined encryptData with decryptData
 * bOp format 1:decrypt, 0:encrypt
 ****************************************************************************************/
u16 cipherData(PX8 inBuffer, u16 inlength, jref theKey, u8 bOp, u8 isNeedCheckInlength)
{
	u8 		X_A		blockSize;
	u8 		X_A		blockMask;
	u8 		X_A		mode;
	PX8 			tempPtr;
	u8		X_A		cKey[32];

	g_SCP_appEnc = 1;

	switch(g_current_SCP_Type)
	{
		case SCP_01:
		case SCP_02:
			blockSize = DES_BLOCKSIZE;
			blockMask = DES_BLOCKSIZE_MASK;
			mode = U8_OR_U8(ECB_Mode, bOp);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
		case SCP_11://merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support SCP11
			// A call to decryptData() or encryptData() shall throw an ISOException with reason code '6985' if
			// Key-DEK is personalized, but not available for the calling application due to the settings in the Key Access
			// Coding (see section 5.3.3). The secure channel session shall not be aborted.
			blockSize = AES_BLOCKSIZE;
			blockMask = AES_BLOCKSIZE_MASK;
			mode = (u8)(CBC_Mode | bOp);
			break;
#endif //GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			blockSize = DES_BLOCKSIZE;
			blockMask = DES_BLOCKSIZE_MASK;
			mode = (u8)(ECB_Mode | bOp);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			return 0;
	}

#if GP__GP_API__FOR_V1_5_MASK /*|| GP__GP_API__FOR_V1_6_MASK*/ //CMCC profile,collise508 test need to close
	if(isNeedCheckInlength &&(bOp == MODE_DECRYPT))
#else
	if(isNeedCheckInlength)//UL:gpapi_testApplet_92_SC_encryptData_error_dataNotBlockAligned_scp03_de_92_5c
		//if the underlying security protocol is SCP '02', then no padding method is defined and this method by itself will not add any padding,
		//which may lead to an exception being thrown (see below) if input data are not block-aligned.
#endif
	{
		if (0x00 != U8_AND_U8(inlength, blockMask))
		{
			throw_error(SW_WRONG_LENGTH);
		}
	}

	if (0x00 == inlength)
	{
		throw_error(SW_DATA_INVALID);
	}


	if (REF_NULL != theKey)
	{
		tempPtr = MEMtPX8(locateObjectInMemory(theKey));
		memCopyToRam(MEMtPX8(cKey), tempPtr, g_SCP_dekKeyLen);
	}
	else
	{
		ecpMem_Copy(MEMtPX8( cKey), g_current_SCP_DEK_KEY, g_SCP_dekKeyLen);
	}

	return dealcipherData(inBuffer, MEMtPX8(cKey), inBuffer, inlength, mode, blockSize);
}


#if	(GP__AMENDMENT__A__FOR_MASK | GP__SCP__SCP10__FOR_MASK)
u8 getValueOffByTag(PX8 inBuffer, u8 currentLenOff)
{
	u16 	X_A		currentLenSize;
	u8 		X_A		sizeOfTagLength;
	u8 		X_A		currentValueOff;
	PX8				tempPtr;

	tempPtr = PX8_A_U16(inBuffer, currentLenOff);
	currentLenSize = PX_R_U8(tempPtr);

	switch(currentLenSize)
	{
		case TAG_82:
			sizeOfTagLength = SIZE_LENGTH_OF_82_TAG;
			break;
		case TAG_81:
			sizeOfTagLength = SIZE_LENGTH_OF_81_TAG;
			break;
		default:
			sizeOfTagLength = SIZE_LENGTH_OF_OTHER_TAG;
			break;
	}
	currentValueOff = U8_A_U8(currentLenOff ,sizeOfTagLength);
	return currentValueOff;
}


u16 getNextTagOff(u8 * inBuffer, u8 currentTagOff, u8 tagType, u16 tagName, u8 IsNeedCheckTagName)
{
	u8 		X_A		sizeOfCurrentT;
	u16 	X_A		sizeOfCurrentV;
	u16 	X_A		nextTagOff;
	u16		X_A		currentTagName;
	PX8				tempPtr;

	tempPtr = PX8_A_U16(inBuffer, currentTagOff);

	switch(tagType)
	{
		case ONE_BYTE_TAG:
			currentTagName = PX_R_U8(tempPtr);
			sizeOfCurrentT = SIZE_OF_ONEBYTETAG_TAG;
		break;

		case TWO_BYTE_TAG:
			currentTagName = PX_R_U16(tempPtr);
			sizeOfCurrentT = SIZE_OF_TWOBYTETAG_TAG;
		break;

		default:
		break;
	}

	if(tagName != currentTagName)
	{
		if(IS_NEED_CHECK_TAG_NAME_YES == IsNeedCheckTagName)
		{
			throw_error(SW_WRONG_DATA);
		}
		else
		{
			return U8t16(currentTagOff);
		}
	}

	tempPtr = PX8_A_U16(tempPtr, sizeOfCurrentT);
	nextTagOff = U16_A_U16(U8t16(currentTagOff), U8t16(sizeOfCurrentT));

	sizeOfCurrentV = PX_R_U8(tempPtr);
	switch(sizeOfCurrentV)
	{
		case TAG_82:
			PX8_INC(tempPtr);
			nextTagOff = U16_A_U16(nextTagOff, SIZE_OF_TWOBYTETAG_TAG);
			sizeOfCurrentV = PX_R_U16(tempPtr);
			break;
		case TAG_81:
			PX8_INC(tempPtr);
			nextTagOff = U16_A_U16(nextTagOff, SIZE_OF_ONEBYTETAG_TAG);
			sizeOfCurrentV = PX_R_U8(tempPtr);
			break;
		default:
			break;
	}

	// add LEN
	nextTagOff = U16_A_U16(nextTagOff, 0x01);
	nextTagOff = U16_A_U16(nextTagOff, sizeOfCurrentV);

	return nextTagOff;
}



u16 getLenByTag(PX8 inBuff, u8 inOffset)
{
	u8 			X_A		bTag;
	u16 		X_A		bLength;
	PX8 				tempPtr;
	tempPtr = PX8_A_U8(inBuff, inOffset);
	bTag = PX_R_U8(tempPtr);
	switch(bTag)
	{
		case TAG_82:
			PX8_INC(tempPtr);
			bLength = PX_R_U16(tempPtr);
			break;
		case TAG_81:
			PX8_INC(tempPtr);
			bLength = PX_R_U8(tempPtr);
			break;
		default:
			bLength = PX_R_U8(tempPtr);
			break;
	}

	return bLength;
}
#endif

/**
 * @param inBuff
 * @param inOffset
 * @param outBuff
 * @param outOffset
 * @return
 */
//恢复类似CASD使用的公钥证书，对于X.509证书的处理，没有实现

// 256 - 20 - 1
#define SIZE_OF_RSA_TEMP_BUFFLEN  				235
// 1+ 2 + 20
#define	SIZE_OF_MD_DIGEST_BUFFER_LEN			0x17
// 1+ 63
#define	SIZE_OF_MD_TEMP_BUFFER_LEN				0x40

#define		RSA_PUBLIC_KEY_N_LENGTH		0x80
#define		RSA_PUBLIC_KEY_E_LENGTH		0x03

#if (GP__AMENDMENT__A__FOR_MASK | GP__SCP__SCP10__FOR_MASK)
u8 scp10_recoverKey(PN8 keySetObj, PX8 inBuff, u8 inOffset,PX8 outBuff, u8 outOffset)
{

	boolean X_A		result;
	u8 		X_A		mdBuff[SIZE_OF_MD_DIGEST_BUFFER_LEN];
	u8 		X_A		tmpBuff[SIZE_OF_MD_TEMP_BUFFER_LEN];
	u8 		X_A		tempRecoverBuff[SIZE_OF_RSA_TEMP_BUFFLEN];
	u8	 	X_A		tempTagLenOff;
	u8	 	X_A		savedFirstTagOff;
	u8	 	X_A		savedLastTagOff;

	u8 		X_A		signLength;
	u8 		X_A		recoverLength;
	u8 		X_A		remLength;
	u8 		X_A		combineOff;
	u8 		X_A		tmpLen;

	PX8				pTempSrc;
	PX8				pTempDes;

	result = false;
	// offset for the length after Tag 7F21.
	tempTagLenOff = U8_A_U16(inOffset, SIZE_OF_TWOBYTETAG_TAG);

	// offset for the Tag 93.
	tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

	// save the first tag off for next used
	savedFirstTagOff = tempTagLenOff;

	// offset for the Tag 42.
	tempTagLenOff = U16t8(getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_93, IS_NEED_CHECK_TAG_NAME_YES));

	// offset for the Tag 5F20.
	tempTagLenOff = U16t8(getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_42, IS_NEED_CHECK_TAG_NAME_YES));

	// offset for the Tag 95.
	tempTagLenOff =U16t8(getNextTagOff(inBuff, tempTagLenOff, TWO_BYTE_TAG, TAG_5F20, IS_NEED_CHECK_TAG_NAME_YES));

	// offset for the Tag 5F25.
	tempTagLenOff = U16t8(getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_95, IS_NEED_CHECK_TAG_NAME_NO));

	// offset for the Tag 5F24.
	tempTagLenOff = U16t8(getNextTagOff(inBuff, tempTagLenOff, TWO_BYTE_TAG, TAG_5F25, IS_NEED_CHECK_TAG_NAME_NO));

	// offset for the Tag 53.
	tempTagLenOff = U16t8(getNextTagOff(inBuff, tempTagLenOff, TWO_BYTE_TAG, TAG_5F24, IS_NEED_CHECK_TAG_NAME_YES));

	// offset for the Tag 5F37.
	tempTagLenOff = U16t8(getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_53, IS_NEED_CHECK_TAG_NAME_NO));

	// save the first tag off for next used
	savedLastTagOff = tempTagLenOff;

	// offset for the Length 5F37.
	tempTagLenOff = U8_A_U16(tempTagLenOff, SIZE_OF_TWOBYTETAG_TAG);

	// get size of Value tag 5F37
	signLength = U16t8(getLenByTag(inBuff, tempTagLenOff));
	// offset for the Value 5F37.
	tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

	//sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
	//sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
	//keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	//bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);
#if	VM_WITH_RSA
	getRSAKeyRef(KEY_TYPE_RSA_PUBLIC, keySetObj, KEY_ID_01);
	//getRSAKeyRef(PKTP_EXAUT);
	// recover the pub exp and part modulus.
	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 u8s.
#endif
	pTempSrc = PX8_A_U16(inBuff, tempTagLenOff);
#if VM_WITH_SIGNMR
	recoverLength = U16t8(msgRecoverAlg(pTempSrc, signLength));
#endif //
	HASHUpdate(pTempSrc, recoverLength, HASH_INIT, mdBuff, tmpBuff, ALG_SHA_1);
	//recoverLength = PKTP_EXAUTRsaSignMR.beginVerify(inBuff, inOffset, signLength);

	//record the off for next use
	combineOff = tempTagLenOff;

	memCopyNoAtomic(MEMtPM8(tempRecoverBuff), pTempSrc, U8t16((U8_A_U8(recoverLength, 20))));
	//arrayCopyNonAtomic(inBuff, inOffset, recoverBuff, (u16)0, (u16)(recoverLength+20));

	// offset for the Tag 5F38 (pub modulus remainder).
	tempTagLenOff = U8_A_U8(tempTagLenOff, signLength);

	 // offset to length of 5F38 (pub modulus remainder).
	tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_TWOBYTETAG_TAG);

	// get size of Value tag 5F38 (pub modulus remainder)
	remLength = U16t8(getLenByTag(inBuff, tempTagLenOff));

	// offset for the Value 5F38.
	tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

	pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);
	HASHUpdate(pTempSrc, remLength, HASH_UPDATE, MEMtPM8(mdBuff), MEMtPM8(tmpBuff), ALG_SHA_1);

	//PKTP_EXAUTRsaSignMR.update(inBuff, remOffset, remLength);
	pTempDes = PX8_A_U8(inBuff, combineOff);
	pTempDes = PX8_A_U8(pTempDes, recoverLength);
	memCopyNoAtomic(pTempDes,pTempSrc, remLength);
	//arrayCopyNonAtomic(inBuff, remOffset, inBuff, combineOff, remLength);

	// verify the data in certificate.
	tmpLen = U8_S_U8(savedLastTagOff, savedFirstTagOff);
	pTempSrc = PX8_A_U8(inBuff, savedFirstTagOff);
#if VM_WITH_SIGNMR
	result = signMRVerifyAlg(pTempSrc, tmpLen, MEMtPM8(tempRecoverBuff), recoverLength,  MEMtPM8(mdBuff), MEMtPM8(tmpBuff), ALG_RSA_SHA_ISO9796_MR);

	if (MR_VERIFY_SUCCESS == result)
	{
//		sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
//		keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);

		if (REF_NULL == MEMtPN8(fm03_locateKeyType(keySetObj, KEY_ID_04)))
		{
			tmpLen = 0;
			tmpLen = U16t8(keyData_computeKeyDataLen(KEY_FORMAT_03, U8t16(tmpLen), RSA_PUBLIC_KEY_N_LENGTH, true));
			tmpLen = U16t8(keyData_computeKeyDataLen(KEY_FORMAT_03, U8t16(tmpLen), RSA_PUBLIC_KEY_E_LENGTH, false));
			keyData_addKeyDataSpace(keySetObj, tmpLen);
		}
		pTempDes = MEMtPX8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_04, RSA_E_DGI_TYPE , RSA_PUBLIC_KEY_E_LENGTH));

		// offset for the length after Tag 7F49.
		tempTagLenOff = U8_A_U8(combineOff, SIZE_OF_TWOBYTETAG_TAG);

		// offset for the Tag 82 (pub exp).
		tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

		// offset for the Length 82(pub exp).
		tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_ONEBYTETAG_TAG);

		// get size of Value tag 82(pub exp)
		tmpLen = U16t8(getLenByTag(inBuff, tempTagLenOff));

		// offset for the Value 82(pub exp).
		tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

		pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);

		if(0x01 == tmpLen)
		{
			pTempDes =PX8_A_U8(pTempDes, 2);
		}
		nvmCopyAtomic( pTempDes, pTempSrc, tmpLen);
		
		outBuff[outOffset] = tmpLen;
		U8_INC(outOffset);
		memCopyAtomic( &outBuff[outOffset], pTempSrc, tmpLen);
		outOffset = U8_A_U8(outOffset, tmpLen);

		// offset for the Tag 81 (pub Mode).
		tempTagLenOff = U8_A_U8(tempTagLenOff, tmpLen);

		// offset for the Length 81 (pub Mode).
		tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_ONEBYTETAG_TAG);

		// get size of Value tag 81 (pub Mode).
		tmpLen = U16t8(getLenByTag(inBuff, tempTagLenOff));

		// offset for the Value 81 (pub Mode).
		tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

		pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);

		pTempDes = MEMtPX8(keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_04, RSA_N_DGI_TYPE , RSA_PUBLIC_KEY_N_LENGTH));

		nvmCopyAtomic(pTempDes, pTempSrc, tmpLen);
		outBuff[outOffset] = tmpLen;
		U8_INC(outOffset);
		memCopyAtomic(&outBuff[outOffset], pTempSrc, tmpLen);

	}
#endif
	
	return result;
}
#endif
#if (GP__SCP && VM_WITH_DES)
u16 nativeDESChiperUpdateCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, PX8 ICVBuff, PM8 tmpBuff, u8 mode, u8 sbcState)
{
	u8 		  X_A		keyen;
	u16 	  X_A		keyBitLen;
	u16 	  X_A		retLen;
	//memref 	  X_A		keyAddress;
	u8 		X_A		keyAddress[LENGTH_RSA_1024 >> 4];

	keyen = U16t8(keyData_getKeyDataById(keySetObj, keyId, KEY_TYPE_SYMMETRIC, keyAddress));
	keyBitLen = U16_SHL_U8(U8t16(keyen), 3);

	retLen = inLen;
	if (sbcState == SBC_UPDATE)
	{
		retLen  = U32t16(U16_A_U16(retLen, U16_SHR_U8(U8t16(mode), 4)));
	}

	retLen = U16_AND_U16(retLen, DES_BYTENUM_IN_MULTIBLOCK_MASK);

	/*if (isInE2P(keyAddress))
	{
		memRefresh();
	}*/

	SymCipherUpdate(MEMtPM8(inBuff), inLen, MEMtPM8(sigBuff), MEMtPM8(keyAddress), keyBitLen, ICVBuff, MEMtPM8(tmpBuff), mode, sbcState, 0x00, 0x01);
	return retLen;
}

u16 nativeDESChiperDoFinalCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, PX8 ICVBuff, PX8 tmpBuff, u8 algType, u8 mode, u8 sbcState)
{

	u16 		X_A 	retLen;
	u8 		  	X_A		keyen;
	u16 	  	X_A		keyBitLen;
	//memref 		X_A		keyAddress;
	u8 		X_A		keyAddress[LENGTH_RSA_1024 >> 4];

	retLen = inLen;

	if (SBC_UPDATE == sbcState)
	{
		retLen = U32t16(U16_A_U16(retLen, U16_SHR_U8(U8t16(mode), 4)));
	}
	//  sbc_init and len is zero
	else if ( 0x00 == retLen )
	{
		throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
	}

	// NOPAD mode or decrypt
	if (U16_AND_U16(retLen, DES_BLOCKSIZE_MASK))
	{
		throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
	}

	keyen = U16t8(keyData_getKeyDataById(keySetObj, keyId, KEY_TYPE_SYMMETRIC, keyAddress));
	keyBitLen = U16_SHL_U8(U8t16(keyen), 3);
	memRefresh();

	inLen = U8t16(SymCipherDofinal(MEMtPM8(inBuff), inLen, MEMtPM8(sigBuff), MEMtPM8(keyAddress), keyBitLen, ICVBuff, MEMtPM8(tmpBuff), algType, mode, sbcState, 0x01));

	if (inLen == WRONG_PADDING)
	{
		throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
	}

	if(PXB_R_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H])) >= inLen)
	{
		//bShortBuffer[LOAD_C4_VALUE] = htoj16(htoj16(bShortBuffer[LOAD_C4_VALUE]) - inLen);
		PXB_W_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H]), U16_S_U16(PXB_R_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H])), inLen));
	}
	return U16_S_U16(retLen, inLen);

}

u16 nativeDESSignatureDoFinalCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, u16 sigLen, u8 algType, u8 mode, u8 flag)
{
	u8 		X_A		keyen;
	u16 	X_A		keyBitLen;
	u16 	X_A		retLen;
	//memref 	X_A		keyAddress;
	u8 		X_A		keyAddress[LENGTH_RSA_1024 >> 4];
	u8		X_A		ICVBuff[DES_BLOCKSIZE];

	keyen = U16t8(keyData_getKeyDataById(keySetObj, keyId, KEY_TYPE_SYMMETRIC, keyAddress));
	keyBitLen = U16_SHL_U8(U8t16(keyen), 3);
	ecpMem_Set(ICVBuff, 0x00, DES_BLOCKSIZE);

	retLen = DES_BLOCKSIZE;

	if (U8_AND_U8(flag, SBC_VERIFY))
	{
		if (sigLen != retLen)
		{ // signature length is not correct, then return false.
			return false;
			//throw_error(SBC_VERIFY_FAIL);
		}
	}

//	inBuff = inBuff + inOffset;
//	sigBuff = sigBuff + sigOffset;

	/*if (isInE2P(keyAddress))
	{
		memRefresh();
	}*/

	if (U8_AND_U8(flag, SBC_VERIFY))
	{
		retLen = U8t16(SymSignatureDofinal(MEMtPM8(inBuff), inLen, MEMtPM8(sigBuff), MEMtPM8(keyAddress), keyBitLen, MEMtPX8(ICVBuff), MEMtPM8(MEM_NULL), algType, U8_OR_U8(mode, SBC_VERIFY), SBC_INIT, U16t8(retLen), flag));
		if (SBC_VERIFY_OK == retLen)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		SymSignatureDofinal(MEMtPM8(inBuff), inLen, MEMtPM8(sigBuff), MEMtPM8(keyAddress), keyBitLen, MEMtPX8(ICVBuff), MEMtPM8(MEM_NULL), algType, mode, SBC_INIT, U16t8(retLen), flag);
	}
	return retLen;
}
#endif
#if (GP__SCP && VM_WITH_RSA)
void getRSAKeyRef(u8 keyType, PX8 keySetObj, u8 keyId, PX8 keyAddress)
{
	//memref 	X_A		keyAddress;
	memref 	X_A		bKeyType;
	// Clear cache
	memRefresh();

	bKeyType = 0x00;
	switch(keyType)
	{
		case KEY_TYPE_RSA_PUBLIC:
			bKeyType = TYPE_RSA_PUBLIC;
			break;
		case KEY_TYPE_RSA_PIVATE:
			bKeyType = TYPE_RSA_PRIVATE;
			break;
		case KEY_TYPE_RSA_CRT:
			bKeyType = TYPE_RSA_CRT_PRIVATE;
			break;
		default:
		break;
	}
	g_RSAKeyStore.bRSAKeyType = U32t8(bKeyType);

	switch(bKeyType)
	{
		case TYPE_RSA_PUBLIC:
			g_RSAKeyStore.pbModulusN = MEMtPX8(keyAddress);
			g_RSAKeyStore.wModulusNLength = keyData_getKeyDataById(keySetObj, keyId, RSA_N_DGI_TYPE, g_RSAKeyStore.pbModulusN);

			g_RSAKeyStore.pbExp = (PX8_A_U16(g_RSAKeyStore.pbModulusN, LENGTH_RSA_1024 >> 3));
			g_RSAKeyStore.wExpLength = keyData_getKeyDataById(keySetObj, keyId, RSA_E_DGI_TYPE, g_RSAKeyStore.pbExp);
		break;

		case TYPE_RSA_PRIVATE:
			g_RSAKeyStore.pbModulusN = MEMtPX8(keyAddress);
			g_RSAKeyStore.wModulusNLength = keyData_getKeyDataById(keySetObj, keyId, RSA_N_DGI_TYPE, g_RSAKeyStore.pbModulusN);

			g_RSAKeyStore.pbExp = (PX8_A_U16(g_RSAKeyStore.pbModulusN, LENGTH_RSA_1024 >> 3));
			g_RSAKeyStore.wExpLength = keyData_getKeyDataById(keySetObj, keyId, RSA_D_DGI_TYPE, g_RSAKeyStore.pbExp);
		break;

		case TYPE_RSA_CRT_PRIVATE:
			g_RSAKeyStore.pbParmP = MEMtPX8(keyAddress);
			g_RSAKeyStore.wPrimeLength = keyData_getKeyDataById(keySetObj, keyId, RSA_CRT_P_DGI_TYPE, g_RSAKeyStore.pbParmP);

			g_RSAKeyStore.pbParmQ = (PX8_A_U16(g_RSAKeyStore.pbParmP, LENGTH_RSA_1024 >> 4));
			keyData_getKeyDataById(keySetObj, keyId, RSA_CRT_Q_DGI_TYPE, g_RSAKeyStore.pbParmQ);

			g_RSAKeyStore.pbParmDP1 =(PX8_A_U16(g_RSAKeyStore.pbParmQ, LENGTH_RSA_1024 >> 4));
			keyData_getKeyDataById(keySetObj, keyId, RSA_CRT_DP_DGI_TYPE, g_RSAKeyStore.pbParmDP1);

			g_RSAKeyStore.pbParmDQ1 = (PX8_A_U16(g_RSAKeyStore.pbParmDP1, LENGTH_RSA_1024 >> 4));
			keyData_getKeyDataById(keySetObj, keyId, RSA_CRT_DQ_DGI_TYPE, g_RSAKeyStore.pbParmDQ1);

			g_RSAKeyStore.pbParmInvQ = (PX8_A_U16(g_RSAKeyStore.pbParmDQ1, LENGTH_RSA_1024 >> 4));
			keyData_getKeyDataById(keySetObj, keyId, RSA_CRT_IQP_DGI_TYPE, g_RSAKeyStore.pbParmInvQ);

			g_RSAKeyStore.wModulusNLength = U16_SHL_U8(g_RSAKeyStore.wPrimeLength, 1);
		break;
		default:
			break;

	}

}

u16 nativeRSAChiperDoFinalCal(PX8 keySetObj, u8 keyType, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, u16 sigLen, u8 algType, u8 mode)
{
	u16 	X_A		retLen;
	u8 keyAddress[(LENGTH_RSA_1024 >> 4) * 5];

	getRSAKeyRef(keyType, keySetObj, keyId, keyAddress);

	retLen = g_RSAKeyStore.wModulusNLength;
	if (ALG_RSA_NOPAD == algType)
	{
		if (inLen != retLen)
		{
			throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
		}
	}
	else  // for ALG_RSA_PKCS1
	{
		if (MODE_DECRYPT == mode)
		{
			if (inLen != retLen)
			{
				throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
			}
		}
		else  // for MODE_ENCRYPT
		{
			// EB = 00 || 01 (02) || PS || 00 || D, the length of PS must be at least 8 bytes.
			if (inLen > U16_S_U8(retLen, 11))
			{
				throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
			}
		}
	}

	g_CryptoData.wBufferLen = inLen;  // total length input.
	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 bytes.

	memCopyToRam(PX8_S_U16(PX8_A_U16(g_CryptoData.pbBuffer, retLen), inLen), MEMtPM8(inBuff), inLen);

	/** the structure of algType is as follows
	 *  ----------------------------------------------------------------------
	 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
	 *  |           6bit  5bit            |        4bit 3bit 2bit 1bit       |
	 *  ----------------------------------------------------------------------
	 */
	algType = U8_OR_U8(algType, U8_SHL_U8(mode, 4));

	if (RSA_PROCESS_OK == DoRSAOpertaion(algType, retLen))
	{
		memCopyNoAtomic(MEMtPM8(sigBuff), MEMtPM8(g_CryptoData.pbBuffer), g_CryptoData.wBufferLen);
	}
	else
	{
		throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
	}

	return g_CryptoData.wBufferLen;
}

u16 nativeRSASignatureDoFinalCal(PN8 keySetObj, u8 keyType, u8 keyId, PN8 inBuff, u16 inLen, PN8 sigBuff, u16 sigLen, u8 algType, u8 mode)
{
	u16 	X_A		retLen;

	u8 keyAddress[(LENGTH_RSA_1024 >> 4) * 5];
	getRSAKeyRef(keyType, keySetObj, keyId,keyAddress);

	retLen = g_RSAKeyStore.wModulusNLength;


	if (ALG_RSA_NOPAD == algType)
	{
		if (inLen != retLen)
		{
			throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
		}
	}

	if (RSA_VERIFY == mode)
	{
		if (sigLen != retLen)
		{
			throw_error(CRYPTOEXCEPT_ILLEGAL_USE);
		}
	}

	retLen = RSA_Signature(MEMtPM8(inBuff), inLen, MEMtPM8(sigBuff), MEM_NULL, MEM_NULL, MEM_NULL, 0, sigLen, algType, mode,REF_NULL);
	return retLen;
}
#endif

u16 nativeAESSignatureDoFinalCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, u16 sigLen, u8 mode, u8 flag)
{
	u8 		X_A		keyen;
	u16 	X_A		retLen;
	//memref 	X_A		keyAddress;
	u8 		X_A		keyAddress[LENGTH_RSA_1024 >> 4];
	u8		X_A		outBuff[SYM_BLOCKSIZE];

	retLen = AES_BLOCKSIZE;
	keyen = U16t8(keyData_getKeyDataById(keySetObj, keyId, KEY_AES_TYPE, keyAddress));

	if (U8_AND_U8(flag, SBC_VERIFY))
	{
		if (sigLen != retLen)
		{ // signature length is not correct, then return false.
			return false;
			//throw_error(SBC_VERIFY_FAIL);
		}
	}

	retLen = Cmac(keyAddress, keyen, (u8*)inBuff, inLen, NULL, 0, outBuff);
	if (U8_AND_U8(flag, SBC_VERIFY))
	{
		if (SBC_VERIFY_OK == SC_memCompareEqualWithDFA(MEMtPM8(outBuff), MEMtPM8(sigBuff), retLen))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		SC_MemCopyNoAtomicAPI(MEMtPM8(sigBuff), outBuff, retLen);
	}
	return retLen;
}

#if GP__KEY_MANAGER__ECC_KEY
/*
 * 获取keyset下的ECC曲线参数
 * @history: merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: ECCParameter修改为局部变量，其最大值为ECC_P_521_DOMAINPARAMS的 长度
 */
boolean getKeysetECCFieldPara(PN8 keySetObj, PM8 ECCParameter)
{
	u16		X_A		keyLen;
	u8		X_A		cofactor[2];

	g_ECKeyStore.pb_domainParams = ECCParameter;

	keyLen = keyData_getKeyDataById(keySetObj, ECC_DOMAIN_KID, ECC_CurvePara_P_DGI_TYPE, g_ECKeyStore.pb_domainParams); //P

	if(keyLen == 0)
	{
		return FALSE; //ECC参数不存在
	}

	g_ECKeyStore.b_PLength = keyLen;

	g_ECKeyStore.b_RLength = g_ECKeyStore.b_PLength;

	keyData_getKeyDataById(keySetObj, ECC_DOMAIN_KID, ECC_CurvePara_A_DGI_TYPE, g_ECKeyStore.pb_domainParams+keyLen); //A
	keyData_getKeyDataById(keySetObj, ECC_DOMAIN_KID, ECC_CurvePara_B_DGI_TYPE, g_ECKeyStore.pb_domainParams+keyLen*2); //B
	keyData_getKeyDataById(keySetObj, ECC_DOMAIN_KID, ECC_CurvePara_G_DGI_TYPE, g_ECKeyStore.pb_domainParams+keyLen*3); //G(包含04)
	keyData_getKeyDataById(keySetObj, ECC_DOMAIN_KID, ECC_CurvePara_N_DGI_TYPE, g_ECKeyStore.pb_domainParams+keyLen*5+1); //N
	keyData_getKeyDataById(keySetObj, ECC_DOMAIN_KID, ECC_CurvePara_K_DGI_TYPE, &cofactor[0]); //k

//	g_ECKeyStore.b_HValue = cofactor[1];//b_HValue定义是u8类型的，因此只取1个byte，需讨论是否需要修改b_HValue的类型
	g_ECKeyStore.b_HValue = MEM_R_U16((memref)cofactor); //BuildWord(&cofactor[0]);  modify by xuxiaojun@20220521 代码分层，使用平台层函数，而BuildWord是uicc层定义的

	g_ECKeyStore.bParamStatus = FP_DOMAIN_PARAM_SETED;

	g_CryptoData.pbBuffer = bTempArr;

	memRefresh();

	return TRUE;
}

/*
 * 通过KeyParaRef获取keyset下的ECC曲线参数
 * @history: merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
boolean getECCFieldParaByKeyParaRef(u8 sdId, u16 eccKeyParaRefValue, PM8 ECCParameter)
{
	jref		sdDataRef;
	PN8			sdDataObject;
	PN8			keySetObj;
	jref		keySetRef;
	u16			temEccKeyParaRefValue;
	jref		keyGroupRef;
	PN8			keySetField;

	sdDataRef = dataManager_getSdDataRef(sdId);

	if (REF_NULL == sdDataRef)
	{
		return FALSE;
	}

	sdDataObject = MEMtPN8(locateObjectInMemory(sdDataRef));

	keyGroupRef = sdData_getKeyGroupRef(sdDataObject, SCP_FF, FALSE);
	if (REF_NULL == keyGroupRef)
	{
		return FALSE;
	}

	keySetField = MEMtPN8(keyGroup_getKeySetField(MEMtPF8(locateObjectInMemory(keyGroupRef))));

	while (true)
	{
		keySetRef = memReadU16(keySetField);
		if (REF_NULL == keySetRef)
		{
			break;
		}

		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));

		temEccKeyParaRefValue = keySet_getEccKeyParaRefValue(keySetObj);

		if (temEccKeyParaRefValue == eccKeyParaRefValue)
		{
			return getKeysetECCFieldPara(keySetObj, ECCParameter); //获取keySetObj下的ECC参数
		}
		else
		{
			keySetField = MEMtPN8(get_object_field_ref(keySetObj, nextKeySet_INDEX)); //KeyParaRef的值不匹配，跳到下一个keyset
		}
	}

	return FALSE;
}

/*
 * 获取ECC曲线参数
 * @history: merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
boolean getECCFieldPara(PN8 keySetObj, PM8 ECCParameter)
{
	u8		X_A		eccKeyParaRefValidTag, sdId;
	u8		X_A		result;
	u16		X_A		eccKeyParaRefValue;

	result	=	TRUE;

	eccKeyParaRefValidTag = nvmReadU8(MEMtPN8(get_object_field_ref(keySetObj, eccKeyParaRefValidTag_INDEX)));

	if(eccKeyParaRefValidTag == TRUE) //KeyParaRef存在
	{
		eccKeyParaRefValue = keySet_getEccKeyParaRefValue(keySetObj);

		if(((eccKeyParaRefValue>=0x60)&&(eccKeyParaRefValue<=0x7F)) || ((eccKeyParaRefValue>=0xE000)&&(eccKeyParaRefValue<=0xFFFF))) //local key parameter references: '60' to '7F' and 'E0 00' to 'FF FF'
		{
			sdId = g_bSDId[selected_channel];

			while(0xFF != sdId)
			{
				result = getECCFieldParaByKeyParaRef(sdId, eccKeyParaRefValue, ECCParameter); //获取ECC曲线参数

				if(result != FALSE)
				{
					break;
				}

				sdId = getAssociateSDId(sdId);
			}
		}
		else //非local key parameter references
		{
			if(TRUE == setECDomainParams(eccKeyParaRefValue)) //先去const数组下查找ECC曲线参数，如果没有再去ISD查找
			{
				return TRUE;
			}

			result = getECCFieldParaByKeyParaRef(ISD_APPID, eccKeyParaRefValue, ECCParameter); //在ISD查找ECC曲线参数
		}
	}
	else //KeyParaRef不存在，直接从keyset里面取ECC参数
	{
		result = getKeysetECCFieldPara(keySetObj, ECCParameter); //获取keySetObj下的ECC参数
	}

	return result;
}
/*
 * @history: merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
 */
u16 nativeECCSignatureDoFinalCal(PN8 keySetObj, u8 keyType, u8 keyId, PN8 inBuff, u16 inLen, PN8 sigBuff, u16 sigLen, u8 mode)
{
	u8 		X_A		algType, num, signatureRSLen;
	u8 		X_A		rLength, sLength;
	u16 	X_A		retLen, keyBitLen;
	u8 		X_A		keyData[2*MAX_ECC_KEY_BYTELEN+1];
	u8 		X_A		sigature[2*MAX_ECC_KEY_BYTELEN+6];
	u8      X_A		ECCParameter[400];

	if(FALSE == getECCFieldPara(keySetObj, ECCParameter)) //获取ECC曲线参数
	{
		return FALSE;
	}

	keyData_getKeyDataById(keySetObj, keyId, keyType, keyData);

	keyBitLen = U16_SHL_U8(g_ECKeyStore.b_PLength, 0x03); //获取PK或者SK

	if((keyBitLen >= 256) && (keyBitLen <= 383)) //key的bit长度为256-383
	{
		algType = ALG_ECDSA_SHA_256;
	}
	else if((keyBitLen >= 384) && (keyBitLen <= 511)) //key的bit长度为384-511
	{
		algType = ALG_ECDSA_SHA_384;
	}
	else if(keyBitLen >= 512) //key的bit长度为512+
	{
		algType = ALG_ECDSA_SHA_512;
	}
	else
	{
		return FALSE;
	}

	if(mode == MODE_SIGN)
	{
		setECPubOrPrivKeyRef(TYPE_EC_FP_PRIVATE, keyData, NULL); //设置签名所需私钥

		ECCSign(inBuff, inLen, NULL, NULL, algType,REF_NULL);

		if(EC_VERIFY_FAIL == DecodeSignature(sigBuff, &rLength, sigBuff+g_ECKeyStore.b_RLength, &sLength, g_CryptoData.pbBuffer, (u8)g_CryptoData.wBufferLen))
		{
			return FALSE;
		}

		retLen = rLength + sLength;
	}
	else
	{
		setECPubOrPrivKeyRef(TYPE_EC_FP_PUBLIC, NULL, keyData); //设置验签所需公钥

		//拼装sigatureR和sigatureS
		signatureRSLen = sigLen/2;
		num = 0x00;
		sigature[num++] = 0x30;
		sigature[num++] = sigLen+4;
		sigature[num++] = 0x02;
		sigature[num++] = signatureRSLen;
		ecpMem_Copy(&sigature[num], sigBuff, signatureRSLen); //sigatureR
		num += signatureRSLen;
		sigature[num++] = 0x02;
		sigature[num++] = signatureRSLen;
		ecpMem_Copy(&sigature[num], sigBuff+signatureRSLen, signatureRSLen); //sigatureS

		retLen = ECVerify(inBuff, inLen, sigature, sigLen+6, NULL, NULL, algType,REF_NULL);
	}

	return retLen;
}
#endif

#endif
