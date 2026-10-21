/*
 * SecureCrypto.c
 *
 *  Created on: 2014Äê11ÔÂ20ÈÕ
 *      Author: jiangxiaohui
 */

#include "SecureCrypto.h"
#include "SecureOperation.h"
#include "Crypto_Constant.h"
#include "RSA_API.h"
#include "halCryptoConfig.h"
#include "ecpDefine.h"
#include "MemOperation.h"
#include "Crypto_API.h"
#include "Sym_API.h"
#include "bootConfigMethod.h"
#include "MemGetReuseRam.h"

#if WITH_SECURE_DATA_STORAGE

const u8 zero_mem[4] = { 0x00, 0x00, 0x00, 0x00 };
void SC_RNGCacheFresh(void)
{
	g_TRNG_Cache.IndexIn = 0;
	if(genRandomNumber_FourByteAlign(&g_TRNG_Cache, TRNG_DATA_SIZE) != ECP_ERR_RNG_GETRANDOM_OK)
	{
		faultDetected();
	}
	if(g_TRNG_Cache.IndexIn != TRNG_CACHE_BYTE_SIZE)
	{
		faultDetected();
	}	
}
u8 SC_RandomCheck(PM8 outptrBak, u16 len)
{
	if((len > 1))
	{
		if((memReadU8(outptrBak) == 0xFF) && (memReadU8(&outptrBak[1]) == 0xFF))
		{
			return GEN_RANDOM_NUMBER_AGAIN;
		}
		else
		{
			return GEN_RANDOM_NUMBER_FINISH;			
		}
	}
	else
	{
		return GEN_RANDOM_NUMBER_FINISH;	
	}
}
void SC_DealSensitiveDataMask(PX32 dataPtr, u8 loopCtr)
{
	u8					X_A			i;
	for(i = 0; i < loopCtr; i++)
	{
		dataPtr[i] ^= g_TRNG_Cache.Data[i&TRNG_DATA_INDEX_MASK];
	}
}

void SC_DealDESSensitiveData(Sym_LoadKey_Param_t * myLoadKeyParam, u16 flag)
{

	   u16                     X_A         maskFlagBak;

		PX32 					X_A			keyData;

		maskFlagBak				= SENSITIVE_DATA_NOT_MASK;

		keyData					= myLoadKeyParam->pKeyBuffer;

		if(SENSITIVE_DATA_NOT_MASK == flag)
		{
			SetupShares(myLoadKeyParam->keyByteLen, g_TRNG_Cache.Data, keyData, keyData);
	//		SC_DealSensitiveDataMask(keyData, myLoadKeyParam->keyByteLen >> 2);
			maskFlagBak = SENSITIVE_DATA_MASK;
		}

		switch(myLoadKeyParam->keyByteLen << 3)
		{
			case LENGTH_DES:
				SetupShares(LENGTH_DES_BYTE, PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY1_OFFSET), PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY3_OFFSET), PX8_A_U16(MEMtPX8(keyData), DES_KEY2_OFFSET));
				SetupShares(LENGTH_DES_BYTE, PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY1_OFFSET), PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY2_OFFSET), PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET));
				myLoadKeyParam->keyByteLen = (LENGTH_3DES_3KEY >> 3);
				break;
			case LENGTH_3DES_2KEY:
				if(SENSITIVE_DATA_MASK_NOT_CHECK == flag)
				{
					SetupShares(LENGTH_DES_BYTE, PX8_A_U16(MEMtPX8(keyData), DES_KEY2_OFFSET), PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY3_OFFSET), PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET));
					SetupShares(LENGTH_DES_BYTE, PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET), PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY2_OFFSET), PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET));
				}
				else
				{
					SetupShares(LENGTH_DES_BYTE, PX8_A_U16(MEMtPX8(keyData), DES_KEY1_OFFSET), PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY3_OFFSET), PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET));
					SetupShares(LENGTH_DES_BYTE, PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET), PX8_A_U16(MEMtPX8(g_TRNG_Cache.Data), DES_KEY1_OFFSET), PX8_A_U16(MEMtPX8(keyData), DES_KEY3_OFFSET));
				}
				myLoadKeyParam->keyByteLen = (LENGTH_3DES_3KEY >> 3);
				break;

			case LENGTH_3DES_3KEY:
			case LENGTH_AES_256:

				break;
			default:
				faultDetected();
		}

		myLoadKeyParam->option 		|= (MODE_KEY_MASK | MODE_INTEGRITY_CHECK);

		myLoadKeyParam->checksum = U32t8(SC_calChecksum(keyData, myLoadKeyParam->keyByteLen >> 2));

		if(flag == SENSITIVE_DATA_NOT_MASK)
		{
			if(maskFlagBak != SENSITIVE_DATA_MASK)
			{
				faultDetected();
			}
		}

		if((LENGTH_3DES_3KEY >> 3) != myLoadKeyParam->keyByteLen)
		{
			faultDetected();
		}
}

RSACRTKeyParCheckSum		X_A		g_RSACRTKeyParCheckSum;
SC_SaveBack					X_A		scDataSaveBack;

static void InitgetKey(SC_symGetKeyParaStruct  *symGetKeyPara, PM8 outBuffer, PM8 keyData, u16 length, u16 flag)
{
	symGetKeyPara->dataChecksum   		= flag;
	symGetKeyPara->Len 					= length;
	symGetKeyPara->outBuff        		= outBuffer;
	symGetKeyPara->sensitiveData 		= keyData;	
}


void SC_InOutSaveBack(PM8 outBuf, PM8 sensitiveData, u16 outBufOff, u16 sensitiveDataOff)
{
	scDataSaveBack.outBufBack 			= outBuf;
	scDataSaveBack.sensitiveDataBack 	= sensitiveData;
	scDataSaveBack.outBufOffBack 		= outBufOff;
	scDataSaveBack.sensitiveDataOffBack	= sensitiveDataOff;
}

void SC_InSaveBack(PM8 sensitiveData, u16 sensitiveDataOff)
{
	scDataSaveBack.sensitiveDataBack 	= sensitiveData;
	scDataSaveBack.sensitiveDataOffBack	= sensitiveDataOff;
}

void SC_InOutCheckBack(PM8 outBuff, PM8 sensitiveData)
{
	PM8 tempSensitiveData;
	PM8 tempOutBuff;
	tempSensitiveData 					= PM8_A_U16(MEMtPM8(get_array_data((PF8)scDataSaveBack.sensitiveDataBack)), scDataSaveBack.sensitiveDataOffBack);
	tempOutBuff 						= PM8_A_U16(MEMtPM8(get_array_data((PF8)scDataSaveBack.outBufBack)), scDataSaveBack.outBufOffBack);
	
	if ((tempSensitiveData != sensitiveData) || (tempOutBuff != outBuff)) 
	{
		faultDetected();
	}
}

void SC_InCheckBack(PM8 sensitiveData)
{
	PM8 tempSensitiveData;
	tempSensitiveData 					= PM8_A_U16(MEMtPM8(get_array_data((PF8)scDataSaveBack.sensitiveDataBack)), scDataSaveBack.sensitiveDataOffBack);
	if (tempSensitiveData != sensitiveData)
	{
		faultDetected();
	}
}

u16 SC_SYMgetKey_C(PM8 outBuffer, PM8 keyData, u16 length, u16 flag)
{	
	SC_symGetKeyParaStruct			X_A	symGetKeyPara;
	u16		volatile				X_A	retStatus;
	
	InitgetKey(&symGetKeyPara, outBuffer, keyData, length, flag);
	
	SC_getParameterChecksum(symGetKeyPara);
	
	retStatus = SC_SYMgetKey(symGetKeyPara);
	
	SC_checkReturnCode(retStatus);
	
	return retStatus;
}

u16 SC_SYMgetKey_Java(PM8 outBuffer, PM8 keyData, u16 length)
{	
	SC_symGetKeyParaStruct			X_A		symGetKeyPara;
	u16		volatile				X_A		retStatus;
	
	InitgetKey(&symGetKeyPara, outBuffer, keyData, length, SENSITIVE_DATA_NOT_MASK);
	
	SC_getParameterChecksum(symGetKeyPara);
	
	SC_InOutCheckBack(symGetKeyPara.outBuff, symGetKeyPara.sensitiveData);
	
	retStatus = ECP_ERR_FAILURE;
	retStatus = SC_SYMgetKey(symGetKeyPara);
	SC_checkReturnCode(retStatus);
	return retStatus;
}

static void InitsymKeyPara(SC_symSetKeyParaStruct  *symSetKeyPara, PM8 inBuffer, PM8 keyData, u16 length)
{
	symSetKeyPara->Len 					= length;
	symSetKeyPara->inBuff        		= inBuffer;
	symSetKeyPara->sensitiveData		= keyData;	
}
u16 SC_SYMsetKey_Java(PM8 keyData, PM8 inBuffer, u16 length)
{	
	SC_symSetKeyParaStruct				X_A 	symSetKeyPara;
	u16		volatile					X_A 	retStatus;
	
	InitsymKeyPara(&symSetKeyPara, inBuffer, keyData, length);
	
	SC_getParameterChecksum(symSetKeyPara);
	
	SC_InOutCheckBack(symSetKeyPara.inBuff, symSetKeyPara.sensitiveData);
	
	retStatus = ECP_ERR_FAILURE;
	retStatus = SC_SYMsetKey(symSetKeyPara);
	SC_checkReturnCode(retStatus);
	return retStatus;
}
u16 SC_SYMsetKey_C(PM8 keyData, PM8 inBuffer, u16 length)
{	
	SC_symSetKeyParaStruct				X_A 	symSetKeyPara;
	u16		volatile					X_A 	retStatus;
	
	InitsymKeyPara(&symSetKeyPara, inBuffer, keyData, length);
	
	SC_getParameterChecksum(symSetKeyPara);
	
	retStatus = ECP_ERR_FAILURE;
	retStatus = SC_SYMsetKey(symSetKeyPara);
	SC_checkReturnCode(retStatus);
	return retStatus;
}

u16 SC_storeSensitiveDataAPI(PM8 sensitiveData, u16 sensitiveDataLength, PM8 sensitiveDesData, u16 sensitiveDesDatalength, PM8 inBuffer)
{
	u16         volatile    X_A         retStatus;
	SC_SensitiveDataParaStruct SensitivePara;
	SC_SensitiveDataParaStruct SensitiveDesPara;
	
	retStatus 						= ECP_ERR_FAILURE;
	
	SensitivePara.inBuff			= NULL;
	SensitivePara.sensitiveData		= sensitiveData;
	SensitivePara.Len				= sensitiveDataLength;
	SC_getParameterChecksum(SensitivePara);
	
	
	SensitiveDesPara.inBuff			= inBuffer;
	SensitiveDesPara.sensitiveData	= sensitiveDesData;
	SensitiveDesPara.Len			= sensitiveDesDatalength;
	SC_getParameterChecksum(SensitiveDesPara);
	
	retStatus						= SC_storeSensitiveData(SensitivePara, SensitiveDesPara);
	SC_checkReturnCode(retStatus);
	return retStatus;
}

u16 SC_getSensitiveDataAPI(PM8 sensitiveData, u16 sensitiveDataLength, PM8 outBuffer)
{
	u16         volatile    X_A         retStatus;
	SC_SensitiveDataParaStruct SensitivePara;
	
	retStatus 						= ECP_ERR_FAILURE;
	
	SensitivePara.inBuff			= outBuffer;
	SensitivePara.sensitiveData		= sensitiveData;
	SensitivePara.Len				= sensitiveDataLength;
	SC_getParameterChecksum(SensitivePara);
	
	retStatus						= SC_getSensitiveData(SensitivePara);
	SC_checkReturnCode(retStatus);
	return retStatus;
}


static u16 SC_DataMaskAPI(PM8 inData, u16 length, u8 maskValue, u16 maskType)
{
	SC_DataMaskParaStruct   X_A         dataMaskPara;
	u16       volatile      X_A         retStatus;
	
	dataMaskPara.dataBuff 		= inData;
	dataMaskPara.Len      		= length;
	dataMaskPara.maskType 		= maskType;
	dataMaskPara.maskValue 		= maskValue;
	SC_getParameterChecksum(dataMaskPara);
	retStatus 					= ECP_ERR_FAILURE;
	retStatus 					= SC_DataMask(dataMaskPara);
	SC_checkReturnCode(retStatus);
	return retStatus;
}

//SC_RSAKeyRefStore			X_A		 	g_scRSAKeyStore;
//SC_RSAKeyGenParam			X_A		 	g_scRSAKeyGen;
//u8						X_A			g_scLoadKeyStatus;
//void SC_storeMEKey(PM8 DesAdrr, u16 DesOffs, u8 MeKeyType)
//{
//	if (TYPE_RSA_PRIVATE == MeKeyType)
//	{
//		g_scRSAKeyStore.pbMaskIv		= PM8_A_U16(DesAdrr,DesOffs);
//		g_scRSAKeyStore.pbCrcValue		= PM8_A_CONST(g_scRSAKeyStore.pbMaskIv, 1);
//	}
//	return;
//}

//void SC_storeCRTKey(PM8 pbParmInvQ, u16 wPrimeLength)
//{
//	g_scRSAKeyStore.crtpbMaskIv			= PM8_A_U16(pbParmInvQ ,wPrimeLength);
//	g_scRSAKeyStore.crtpbCrcValue		= PM8_A_CONST(g_scRSAKeyStore.crtpbMaskIv,1);
//	return;
//}


//u8 NandEBuffer[260];
//void RSA_storeCRTKeyAddSecurityParam(PM8 pbModular, PM8 pbParmExponent, u16 wExpLength)
//{
//	g_RSAKeyStore.wExpLength		= wExpLength;
//	if(pbModular == 0)
//	{
//		g_RSAKeyStore.pbCRTModulusN = NandEBuffer;
//		//g_RSAKeyStore.pbExp 			= pbParmExponent;
//		
//		BigData_Mul(g_RSAKeyStore.pbCRTModulusN ,g_RSAKeyStore.pbParmP,g_RSAKeyStore.wPrimeLength, g_RSAKeyStore.pbParmQ, g_RSAKeyStore.wPrimeLength ,TRUE);
//	}
//	else
//	{
//		g_RSAKeyStore.pbCRTModulusN 	= pbModular;
//		//g_RSAKeyStore.pbExp 			= pbParmExponent;		
//	}
//	g_RSAKeyStore.pbExp 			= pbParmExponent;
//}

u8 SC_checkKeyIsNeedENCry(u8 keyType, u8 keyStatus)
{
	u8  					X_A			mode;
	u8  					X_A			keyStatusMask;
	
	keyStatusMask = 0;

	if(TYPE_RSA_PRIVATE == keyType)
	{
		keyStatusMask = SC_PKI_ME_KEY_NEED_ENCRY_MASK;
	}
	else if(TYPE_RSA_CRT_PRIVATE == keyType)
	{
		keyStatusMask = SC_PKI_CRT_KEY_NEED_ENCRY_MASK;
	}
	else
	{
		;
	}

	if(keyStatus & keyStatusMask)
	{
		mode 		= SC_PKI_IN_ENCRY_TEXT;
	}
	else
	{
		mode		= SC_PKI_IN_PLAIN_TEXT;
	}
	return mode;
}

void SC_PKIGetAllKey(PX8 outBuffer, u8 keyType)
{
	u16 tmpLen;
	switch (keyType)
	{
	case TYPE_RSA_PRIVATE:
		SC_checkReturnCode(SC_PKIgetKey(outBuffer, outBuffer, g_RSAKeyStore.pbModulusN, g_RSAKeyStore.wModulusNLength, g_RSAKeyStore.wModulusNLength, keyType, g_RSAKeyStore.wExpLength, SC_PKI_IN_PLAIN_TEXT));
		incrementCodeFlowControlFlag();
		incrementCodeFlowControlFlag();
		g_RSAKeyStore.pbExp 			= outBuffer;
		break;
	case TYPE_RSA_CRT_PRIVATE:
		incrementCodeFlowControlFlag();
		SC_checkReturnCode(SC_PKIgetCRTKey_C(g_RSAKeyStore.pbParmP, g_RSAKeyStore.wPrimeLength));
		break;
	case TYPE_RSA_PUBLIC:
		incrementCodeFlowControlFlag();
		incrementCodeFlowControlFlag();
		break;
	default:
		faultDetected();
		break;
	}
	incrementCodeFlowControlFlag();
}


#define KEY_MASKIV_LENGTH			0x01
#define KEY_CHECKSUM_LENGTH			0x01
#define KEY_PADDING_LENGTH			(KEY_MASKIV_LENGTH + KEY_CHECKSUM_LENGTH)

u16 SC_VerifyKeyCheckSum(PM8 srcAddress, PM8 tempAddress, u16 sumkeyLen, u16 mode)
{
	PM8						X_A			maskIVPtr;
	u8	                    X_A         checksum;
	u8                      X_A         checksumBuff[KEY_CHECKSUM_LENGTH];
	u8						X_A			MaskIv[KEY_MASKIV_LENGTH];
	u16       volatile      X_A         retStatus;
	u16       			    X_A         tempLen;
	u8						X_A			MaskValue;
	u8						X_A			prechecksum;
	
	PATCH_BOOT_SECURECRYPTO_003();
	
	switch(mode)
	{
		case DATA_TYPE_RSA:
			if(srcAddress != tempAddress)
			{
				faultDetected();
			}
			break;
		case DATA_TYPE_DES:
		case DATA_TYPE_KEYSET:
			SC_MemCopyMem2RamAPI(tempAddress, srcAddress, U8_A_U8(sumkeyLen, KEY_PADDING_LENGTH));
			break;
		case DATA_TYPE_PIN:
			tempLen						= DES_BLOCKSIZE_MASK & sumkeyLen;
			if(0x00 != tempLen)
			{
				tempLen = DES_BLOCKSIZE - tempLen;
				SC_MemCopyMem2RamAPI(tempAddress, srcAddress, U8_A_U8(U8_A_U8(sumkeyLen, tempLen), KEY_PADDING_LENGTH));
				ecpMem_Set(&tempAddress[sumkeyLen], 0x00, tempLen);
				sumkeyLen += tempLen;
			}//TCK pin test
			else
			{
				SC_MemCopyMem2RamAPI(tempAddress, srcAddress, U8_A_U8(sumkeyLen, KEY_PADDING_LENGTH));
			}
			break;
		default:
			faultDetected();
	}

	maskIVPtr						= PM8_A_U16(tempAddress, sumkeyLen);

	MaskValue						= memReadU8(maskIVPtr);
	
	if(0x00 == MaskValue)
	{
		MaskIv[0]					= 0;
		do
		{
			Random_number_generator(MaskIv, KEY_MASKIV_LENGTH, ALG_SECURE_RANDOM);
			MaskValue				=  MaskIv[0];
		}while( (0x00 == MaskValue) || (0x00 == rommask[MaskValue]) );

		//store MaskIv into keybuff, offset is SumkeyLen + 1.
		SC_MemCopyNoAtomicAPI(PM8_A_U16(srcAddress, sumkeyLen), MaskIv, KEY_MASKIV_LENGTH);
		if(DATA_TYPE_RSA != mode)
		{
			tempAddress[sumkeyLen] = MaskValue;
		}
		else if(srcAddress != tempAddress)
		{
			faultDetected();
		}
	}
	else
	{
		tempLen						= U16_A_U8(sumkeyLen, KEY_MASKIV_LENGTH);
		checksum 					= U16t8(Get_crc16(tempAddress, tempLen));
		prechecksum 				= memReadU8(PM8_A_U16(maskIVPtr, KEY_MASKIV_LENGTH));
		if(checksum != prechecksum)
		{
			faultDetected();
		}
	}
	
	return MaskValue;
}

u16 SC_SetKeyCheckSum(PM8 desAddress, PM8 keyData, u16 sumkeyLen, u16 mode)
{
	PM8						X_A			maskIVPtr;
	u16						X_A			tempLen;
	u16                     X_A         checksum;
	u8                      X_A         checksumBuff[KEY_CHECKSUM_LENGTH];
	u16       volatile      X_A         retStatus;
	u8						X_A			MaskValue;
	
	PATCH_BOOT_SECURECRYPTO_003();
	retStatus	= ECP_ERR_FAILURE;
	
	maskIVPtr						= PM8_A_U16(keyData, sumkeyLen);
	
	MaskValue						= memReadU8(maskIVPtr);
	
	if( (0x00 == MaskValue) || (0x00 == rommask[MaskValue]) )
	{
		faultDetected();
	}
	else
	{
		tempLen						= U16_A_U8(sumkeyLen, KEY_MASKIV_LENGTH);
		checksum 					= U16t8(Get_crc16(keyData, tempLen));
		PX_W_U8(checksumBuff, checksum);
	
		SC_MemCopyNoAtomicAPI(PM8_A_U16(desAddress, U16_A_U16(sumkeyLen, KEY_MASKIV_LENGTH)), checksumBuff, KEY_CHECKSUM_LENGTH);
	}
	
	return MaskValue;
}


u16 SC_storeSensitiveData(SC_SensitiveDataParaStruct SensitivePara, SC_SensitiveDataParaStruct SensitiveDesPara)
{
	PX8						X_A			tempBuffer;
	u16       volatile      X_A         retStatus;
	u8						X_A			MaskValueBak;
	u8						X_A			MaskValue;
	u8						X_A			tempLen;
	
	PATCH_BOOT_SECURECRYPTO_001();
	
	
	tempBuffer					= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(SC_MAX_PIN_SIZE + 4));
	
	SC_parameterIntegrityCheck(SensitivePara);
	
	tempLen						= DES_BLOCKSIZE_MASK & SensitivePara.Len;
	if(0x00 != tempLen)
	{
		tempLen = DES_BLOCKSIZE - tempLen;
		tempLen += SensitivePara.Len;
	}
	else
	{
		tempLen = SensitivePara.Len;
	}

	MaskValueBak				= memReadU8(PM8_A_U16(SensitivePara.sensitiveData, tempLen));
	MaskValue					= SC_VerifyKeyCheckSum(SensitivePara.sensitiveData, tempBuffer, tempLen, DATA_TYPE_PIN);
	if(0x00 == MaskValueBak)
	{
		SensitivePara.Len		= tempLen;
		
		SC_checkReturnCode(SC_DataMaskAPI(tempBuffer, SensitivePara.Len, MaskValue, (DATA_MASK_ENC | DATA_TYPE_PIN)));

		if(MaskValue != SC_SetKeyCheckSum(tempBuffer, tempBuffer, SensitivePara.Len, DATA_TYPE_PIN))
		{
			faultDetected();
		}
		
		SensitivePara.Len += (KEY_MASKIV_LENGTH + KEY_CHECKSUM_LENGTH);
			
		retStatus = SC_MemCopyAPI(SensitivePara.sensitiveData, tempBuffer, SensitivePara.Len, ECP_COPY_TYPE_ATOMIC);
	}
	else if(MaskValueBak != MaskValue)
	{
		faultDetected();
	}
	
	SC_parameterIntegrityCheck(SensitiveDesPara);
	
	// only for OwnerPIN store
	SC_MemCopyMem2RamAPI(tempBuffer, SensitiveDesPara.inBuff, SensitiveDesPara.Len);
	tempLen						= DES_BLOCKSIZE_MASK & SensitiveDesPara.Len;
	if(0x00 != tempLen)
	{
		tempLen = DES_BLOCKSIZE - tempLen;
		ecpMem_Set(&tempBuffer[SensitiveDesPara.Len], 0x00, tempLen);
		SensitiveDesPara.Len += tempLen;
	}//TCK pin test
	
	tempBuffer[SensitiveDesPara.Len] = MaskValue;
	SC_checkReturnCode(SC_DataMaskAPI(tempBuffer, SensitiveDesPara.Len, MaskValue, (DATA_MASK_ENC | DATA_TYPE_PIN)));
	
	if(MaskValue != SC_SetKeyCheckSum(tempBuffer, tempBuffer, SensitiveDesPara.Len, DATA_TYPE_PIN))
	{
		faultDetected();
	}
	
	SensitiveDesPara.Len += (KEY_MASKIV_LENGTH + KEY_CHECKSUM_LENGTH);
		
	retStatus = SC_MemCopyAPI(SensitiveDesPara.sensitiveData, tempBuffer, SensitiveDesPara.Len, ECP_COPY_TYPE_ATOMIC);
	SC_checkReturnCode(retStatus);

	if(0 == SensitiveDesPara.Len)
	{
		faultDetected();
	}
	
	return retStatus;
}


u16 SC_getSensitiveData(SC_SensitiveDataParaStruct SensitivePara)
{
	PX8						X_A			tempBuffer;
	u16       volatile      X_A         retStatus;
	u8						X_A			MaskValueBak;
	u8						X_A			MaskValue;
	u8						X_A			tempLen;
	
	tempBuffer					= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(SC_MAX_PIN_SIZE + 4));
	
	SC_parameterIntegrityCheck(SensitivePara);
	
	tempLen						= DES_BLOCKSIZE_MASK & SensitivePara.Len;
	if(0x00 != tempLen)
	{
		tempLen = DES_BLOCKSIZE - tempLen;
		tempLen += SensitivePara.Len;
	}
	else
	{
		tempLen = SensitivePara.Len;
	}

	MaskValueBak				= memReadU8(PM8_A_U16(SensitivePara.sensitiveData, tempLen));
	if(0x00 == MaskValueBak)
	{
		faultDetected();
	}
	MaskValue					= SC_VerifyKeyCheckSum(SensitivePara.sensitiveData, tempBuffer, tempLen, DATA_TYPE_PIN);
	if(MaskValueBak != MaskValue)
	{
		faultDetected();
	}

	SC_checkReturnCode(SC_DataMaskAPI(tempBuffer, SensitivePara.Len, MaskValue, (DATA_MASK_DEC | DATA_TYPE_PIN)));
	
	SetupShares(SensitivePara.Len, tempBuffer, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], tempBuffer);
		
	retStatus = SC_MemCopyAPI(SensitivePara.inBuff, tempBuffer, SensitivePara.Len, ECP_COPY_TYPE_ATOMIC);
	SC_checkReturnCode(retStatus);

	if(0 == SensitivePara.Len)
	{
		faultDetected();
	}
	
	return retStatus;
}


u16 SC_SYMsetKey(SC_symSetKeyParaStruct symSetKeyPara)
{
	PX8						X_A			tempBuffer;
	u16       volatile      X_A         retStatus;
	u8						X_A			MaskValue;
	
	PATCH_BOOT_SECURECRYPTO_001();
	
	SC_parameterIntegrityCheck(symSetKeyPara);
	
	tempBuffer					= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(SC_MAX_PIN_SIZE + 4));
	
	MaskValue					= SC_VerifyKeyCheckSum(symSetKeyPara.sensitiveData, tempBuffer, symSetKeyPara.Len, DATA_TYPE_DES);
	
	SC_MemCopyAtomicAPI(tempBuffer, symSetKeyPara.inBuff, symSetKeyPara.Len);
	
	SC_checkReturnCode(SC_DataMaskAPI(tempBuffer, symSetKeyPara.Len, MaskValue, (DATA_MASK_ENC | DATA_TYPE_DES) ));
	
	if(MaskValue != SC_SetKeyCheckSum(tempBuffer, tempBuffer, symSetKeyPara.Len, DATA_TYPE_DES))
	{
		faultDetected();
	}

	symSetKeyPara.Len 	+= (KEY_PADDING_LENGTH);
	
	retStatus = SC_MemCopyAPI(symSetKeyPara.sensitiveData, tempBuffer, symSetKeyPara.Len, ECP_COPY_TYPE_ATOMIC);
	SC_checkReturnCode(retStatus);
	
	if(0 == symSetKeyPara.Len)
	{
		faultDetected();
	}
	
	return retStatus;
}
u16 SC_SYMgetKey(SC_symGetKeyParaStruct symGetKeyPara)
{
	u8			            X_A         tempBuffer[SC_MAX_PIN_SIZE + 4];
	u16         volatile    X_A         retStatus;
	u8						X_A			MaskValue;
	u8						X_A			MaskValueBak;
	PATCH_BOOT_SECURECRYPTO_002();
		
	SC_parameterIntegrityCheck(symGetKeyPara);
	
	MaskValueBak   = memReadU8(PM8_A_U16(symGetKeyPara.sensitiveData, symGetKeyPara.Len));
	if(0x00 == MaskValueBak)
	{
		faultDetected();
	}
	MaskValue					= SC_VerifyKeyCheckSum(symGetKeyPara.sensitiveData, tempBuffer, symGetKeyPara.Len, DATA_TYPE_DES);
	
	if(MaskValueBak != MaskValue)
	{
		faultDetected();
	}
	
	SC_checkReturnCode(SC_DataMaskAPI(tempBuffer, symGetKeyPara.Len, MaskValue, (DATA_MASK_DEC | DATA_TYPE_DES)) );
	
	if(SENSITIVE_DATA_NOT_MASK == symGetKeyPara.dataChecksum)
	{
		SetupShares(symGetKeyPara.Len, tempBuffer, (PM8)&g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET], tempBuffer);
	}
	retStatus = SC_MemCopyAPI(symGetKeyPara.outBuff, tempBuffer, symGetKeyPara.Len, ECP_COPY_TYPE_ATOMIC);
	SC_checkReturnCode(retStatus);
	
	if(0 == symGetKeyPara.Len)
	{
		faultDetected();
	}
	
	ecpMem_Set(tempBuffer, MEMtPX8(g_TRNG_Cache.Data[SYM_RNG_KEY_OFFSET]), symGetKeyPara.Len);
	
	incrementCodeFlowControlFlag();

	return retStatus;
}


u16 SC_PKIsetKey_JAVA(PX8 keytempbuff, PM8 keyData, u16 keyOffs, PM8 inBuffer, u16 keyLen, u8 keyType, u16 realKeyLen, u8 keyStatus)
{
	u8				X_A			mode;
	u16				X_A			sumkeyLen;
	u16 volatile	X_A			returnCode;
	PN8				X_A			keyTureByteLenOff;
	
	mode			= SC_checkKeyIsNeedENCry(keyType, keyStatus);
	
	returnCode 		= SC_PKIsetKey(keytempbuff, keyData, keyOffs, inBuffer, keyLen, keyType, realKeyLen, mode);

	if(SC_PKI_IN_ENCRY_TEXT == (mode & SC_PKI_IN_ENCRY_TEXT))
	{
		sumkeyLen 					= U16_M_U16(keyLen, 5);
		switch(keyStatus)
		{
			case SC_PKI_CRT_KEY_P_MASK:
			case SC_PKI_CRT_KEY_Q_MASK:
				g_keyTureByteLen	= 0;
				break;
			case SC_PKI_CRT_KEY_DP_MASK:
				keyTureByteLenOff	= PM8_A_U16(keyData, sumkeyLen + KEY_PADDING_LENGTH);
				break;
			case SC_PKI_CRT_KEY_DQ_MASK:
				keyTureByteLenOff	= PM8_A_U16(keyData, sumkeyLen + KEY_PADDING_LENGTH + 1);
				break;
			case SC_PKI_CRT_KEY_QINV_MASK:
				keyTureByteLenOff	= PM8_A_U16(keyData, sumkeyLen + KEY_PADDING_LENGTH + 2);
				break;
			default:
				break;
		}
		if(g_keyTureByteLen <= 4)
		{
			g_keyTureByteLen = 0;
		}
		else
		{
			g_keyTureByteLen --;
			returnCode = SC_MemCopyAPI(keyTureByteLenOff, &g_keyTureByteLen, 1, ECP_COPY_TYPE_ATOMIC);
		}
	}
	
	return returnCode;
}
u16 SC_PKIsetKey(PX8 keytempbuff, PM8 keyData, u16 keyOffs, PM8 inBuffer, u16 keyLen, u8 keyType, u16 realKeyLen, u8 mode)
{
	u16						X_A			sumkeyLen;
	u16       volatile      X_A         retStatus;
	u8						X_A			MaskValue;
	
	PATCH_BOOT_SECURECRYPTO_003();
	retStatus	= ECP_ERR_FAILURE;
	SC_MemCopyAtomicAPI(keytempbuff, inBuffer, realKeyLen);

	if(SC_PKI_IN_DEAL_TEXT == (mode & SC_PKI_IN_DEAL_TEXT))
	{
		dealPKIData(keytempbuff, realKeyLen);
	}

	if(TYPE_RSA_PUBLIC != keyType)
	{
		if(TYPE_RSA_CRT_PRIVATE == keyType)
		{
			sumkeyLen 					= U16_M_U16(keyLen, 5);
		}
		else //TYPE_RSA_PRIVATE
		{
			sumkeyLen 					= U16_M_U16(keyLen, 2);
		}

		MaskValue						= SC_VerifyKeyCheckSum(keyData, keyData, sumkeyLen, DATA_TYPE_RSA);

		if(SC_PKI_IN_ENCRY_TEXT == (mode & SC_PKI_IN_ENCRY_TEXT))
		{
			SC_checkReturnCode(SC_DataMaskAPI(keytempbuff, realKeyLen, MaskValue, (DATA_MASK_ENC | DATA_TYPE_RSA)));
		}
		
		retStatus	= SC_MemCopyAPI(PM8_A_U16(keyData, keyOffs), keytempbuff, realKeyLen, ECP_COPY_TYPE_ATOMIC);
		
		memRefresh();

		if(MaskValue != SC_SetKeyCheckSum(keyData, keyData, sumkeyLen, DATA_TYPE_RSA))
		{
			faultDetected();
		}
	}
	else
	{
		retStatus = SC_MemCopyAPI(PM8_A_U16(keyData, keyOffs), keytempbuff, realKeyLen, ECP_COPY_TYPE_ATOMIC);
	}
	SC_checkReturnCode(retStatus);
	if(0 == realKeyLen)
	{
		faultDetected();
	}
	return retStatus;
}

u16 SC_PKIgetKey_JAVA(PX8 keytempbuff, PM8 outBuffer, PM8 keyData, u16 keyOffs, u16 keyLen, u8 keyType, u16 realKeyLen, u8 keyStatus)
{
	u8				X_A			mode;
	u16				X_A			sumkeyLen;
	u16 volatile	X_A			returnCode;
	
	mode							= SC_checkKeyIsNeedENCry(keyType, keyStatus);
	returnCode = SC_PKIgetKey(keytempbuff, outBuffer,  keyData,  keyOffs,  keyLen,  keyType,  realKeyLen,  mode);
	
	if(SC_PKI_IN_ENCRY_TEXT == (mode & SC_PKI_IN_ENCRY_TEXT))	
	{
		sumkeyLen 					= U16_M_U16(keyLen, 5);
		switch(keyStatus)
		{
			case SC_PKI_CRT_KEY_P_MASK:
			case SC_PKI_CRT_KEY_Q_MASK:
				g_keyTureByteLen	= realKeyLen - 1;
				break;
			case SC_PKI_CRT_KEY_DP_MASK:
				g_keyTureByteLen  	= memReadU8(&keyData[sumkeyLen + KEY_PADDING_LENGTH]);
				break;
			case SC_PKI_CRT_KEY_DQ_MASK:
				g_keyTureByteLen  	= memReadU8(&keyData[sumkeyLen + KEY_PADDING_LENGTH + 1]);
				break;
			case SC_PKI_CRT_KEY_QINV_MASK:
				g_keyTureByteLen  	= memReadU8(&keyData[sumkeyLen + KEY_PADDING_LENGTH + 2]);
				break;
			default:
				break;
		}
		if(0x00 != g_keyTureByteLen)
		{
			memCopyToRam(keytempbuff, outBuffer, realKeyLen);
			BigData_Add(keytempbuff, keytempbuff, realKeyLen, g_RSA_IV, g_keyTureByteLen, TRUE);
			returnCode 	= SC_MemCopyAPI(outBuffer, keytempbuff, realKeyLen, ECP_COPY_TYPE_ATOMIC);
		}
	}
	
	return returnCode;
}

u16 SC_PKIgetCRTKey_C(PM8 keyData, u16 keyLen)
{
	u16       volatile      X_A         retStatus;
	u16  					X_A			sumkeyLen;
	u8          		    X_A         MaskValue;
	u8          		    X_A         keyTureByteLen;
	
	
	PATCH_BOOT_SECURECRYPTO_004();
	

	retStatus						= ECP_ERR_FAILURE;
	
	sumkeyLen 						= U16_M_U16(keyLen, 5);
	MaskValue						= SC_VerifyKeyCheckSum(keyData, keyData, sumkeyLen, DATA_TYPE_RSA);
	
	retStatus						= ECP_ERR_FAILURE;
	retStatus						= SC_DataMaskAPI(keyData, keyLen, MaskValue, (DATA_MASK_DEC | DATA_TYPE_RSA) );
	SC_checkReturnCode(retStatus);
	
	rsaCRTKeydPQRef.pbParmPQ		= g_RSA_IV;
	
	keyTureByteLen					= memReadU8(&keyData[sumkeyLen + KEY_PADDING_LENGTH]);
	if( 0 !=  keyTureByteLen)
	{
		rsaCRTKeydPQRef.pbParmDP1 	= g_RSA_IV;
		rsaCRTKeydPQRef.dPLength	= keyTureByteLen;
	}
	else
	{
		rsaCRTKeydPQRef.pbParmDP1 	= zero_mem;
		rsaCRTKeydPQRef.dPLength	= sizeof(zero_mem);
	}
	keyTureByteLen					= memReadU8(&keyData[sumkeyLen + KEY_PADDING_LENGTH + 1]);
	if( 0 !=  keyTureByteLen)
	{
		rsaCRTKeydPQRef.pbParmDQ1 	= g_RSA_IV;
		rsaCRTKeydPQRef.dQLength	= keyTureByteLen;
	}
	else
	{
		rsaCRTKeydPQRef.pbParmDQ1 	= zero_mem; 
		rsaCRTKeydPQRef.dQLength	= sizeof(zero_mem);
	}
	keyTureByteLen					= memReadU8(&keyData[sumkeyLen + KEY_PADDING_LENGTH + 2]);
	if( 0 !=  keyTureByteLen)
	{
		rsaCRTKeydPQRef.pbParmInvQ 	= g_RSA_IV;
		rsaCRTKeydPQRef.QinvLength	= keyTureByteLen;
	}
	else
	{
		rsaCRTKeydPQRef.pbParmInvQ 	=  zero_mem; 
		rsaCRTKeydPQRef.QinvLength	= sizeof(zero_mem);
	}
				
	return retStatus;
}
u16 SC_PKIgetKey(PX8 keytempbuff, PM8 outBuffer, PM8 keyData, u16 keyOffs, u16 keyLen, u8 keyType, u16 realKeyLen, u8 mode)
{
	u32                     X_A         cfi;
	u16       volatile      X_A         retStatus;
	u16  					X_A			sumkeyLen;
	u8          		    X_A         MaskValue;
	u8          		    X_A         MaskValueBak;
	
	PATCH_BOOT_SECURECRYPTO_004();
	
	cfi 		= 0x5AA57AA7;
	retStatus	= ECP_ERR_FAILURE;
	
	retStatus	= SC_MemCopyAPI(keytempbuff, PM8_A_U16(keyData, keyOffs), realKeyLen, ECP_COPY_TYPE_ATOMIC);
	SC_checkReturnCode(retStatus);

	if(SC_PKI_IN_DEAL_TEXT == (mode & SC_PKI_IN_DEAL_TEXT))
	{
		dealPKIData(keytempbuff, realKeyLen);
	}

	if (keyType != TYPE_RSA_PUBLIC)
	{
		if(keyType == TYPE_RSA_CRT_PRIVATE)
		{
			sumkeyLen 					= U16_M_U16(keyLen, 5);
		}
		else //TYPE_RSA_PRIVATE
		{
			sumkeyLen 					= U16_M_U16(keyLen, 2);
		}

		MaskValueBak   = memReadU8(PM8_A_U16(keyData, sumkeyLen));
		if(0x00 == MaskValueBak)
		{
			faultDetected();
		}
		
		MaskValue						= SC_VerifyKeyCheckSum(keyData, keyData, sumkeyLen, DATA_TYPE_RSA);
		
		if(MaskValueBak != MaskValue)
		{
			faultDetected();
		}
		
		if(SC_PKI_IN_ENCRY_TEXT == (mode & SC_PKI_IN_ENCRY_TEXT))
		{
			retStatus	= ECP_ERR_FAILURE;
			retStatus	= SC_DataMaskAPI(keytempbuff, realKeyLen, MaskValue, (DATA_MASK_DEC | DATA_TYPE_RSA) );
			SC_checkReturnCode(retStatus);
		}
	}

	if(keytempbuff != outBuffer)
	{
		retStatus	= SC_MemCopyAPI(outBuffer, keytempbuff, realKeyLen, ECP_COPY_TYPE_ATOMIC);
		SC_checkReturnCode(retStatus);
		ecpMem_Set(keytempbuff, 0, realKeyLen);
		cfi = 0xA55AA77A;
	}
	
	if (keytempbuff == outBuffer)
	{
		if(cfi != 0x5AA57AA7)
		{
			faultDetected();
		}
	}
	
	if(0 == realKeyLen)
	{
		faultDetected();
	}
	return retStatus;
}

u16 SC_KEYSETsetKey(PM8 keyData, u16 keyOffs, PM8 inBuffer)
{
	u8  					X_A			keytempbuff[LEN_OF_KEYSET_ARRAY + 2];
	u16  					X_A			sumkeyLen;
	u16         volatile    X_A         retStatus;
	u8          		    X_A         MaskValue;
	
	PATCH_BOOT_SECURECRYPTO_005();
	
	//if there is data in the keyData, then fist check its checksum.
	MaskValue						= SC_VerifyKeyCheckSum(keyData, keytempbuff, LEN_OF_KEYSET_ARRAY, DATA_TYPE_KEYSET);
	
	sumkeyLen = MAX_DES_KEY_LENGTH;

	SC_MemCopyAtomicAPI(PM8_A_U8(keytempbuff, keyOffs), inBuffer, sumkeyLen);
	
	SC_checkReturnCode(SC_DataMaskAPI(PM8_A_U8(keytempbuff, keyOffs), sumkeyLen, MaskValue, (DATA_MASK_ENC | DATA_TYPE_KEYSET) ));

	SC_MemCopyAtomicAPI(PM8_A_U8(keyData, keyOffs), PM8_A_U8(keytempbuff, keyOffs), sumkeyLen);
	
	if(MAX_DES_KEY_LENGTH != sumkeyLen)
	{
		faultDetected();
	}
	
	if(MaskValue != SC_SetKeyCheckSum(keyData, keytempbuff, LEN_OF_KEYSET_ARRAY, DATA_TYPE_KEYSET))
	{
		faultDetected();
	}
	else
	{
		return ECP_ERR_OK;
	}
}

u16 SC_KEYSETgetKey(PM8 keyData, PM8 outBuffer)
{
	u8  					X_A			keytempbuff[LEN_OF_KEYSET_ARRAY + 2];
	u16         volatile    X_A         retStatus;
	u8						X_A			MaskValue;

	PATCH_BOOT_SECURECRYPTO_006();	

	MaskValue				= SC_VerifyKeyCheckSum(keyData, keytempbuff, LEN_OF_KEYSET_ARRAY, DATA_TYPE_KEYSET);

	SC_checkReturnCode(SC_DataMaskAPI(keytempbuff, LEN_OF_KEYSET_ARRAY, MaskValue, (DATA_MASK_DEC | DATA_TYPE_KEYSET) ));
	
	retStatus = SC_MemCopyAPI(outBuffer, keytempbuff, LEN_OF_KEYSET_ARRAY, ECP_COPY_TYPE_ATOMIC);
	SC_checkReturnCode(retStatus);
	return retStatus;
}
#endif	// WITH_SECURE_DATA_STORAGE


#if SEC_MEM_OPERATION
u16 SC_MemCopyAPI_JAVA(PM8 t, PM8 s, u16 length, u8 copyType)
{

	SC_SecCpyStruct         X_A         secCpyPara;
	u16         volatile    X_A         retStatus;

	secCpyPara.secCpySrc    = s;
	secCpyPara.secCpyDes	= t;
	secCpyPara.secCpyLength = length;

	SC_getParameterChecksum(secCpyPara);

	retStatus 		= ECP_ERR_FAILURE;

	SC_InOutCheckBack(secCpyPara.secCpyDes, secCpyPara.secCpySrc);

	retStatus 		= SC_MemCopy(secCpyPara, copyType);

	SC_checkReturnCode(retStatus);
	return retStatus;
}

void SC_MemCopyRam2RamAPI(PX8 t, PM8 s, u16 length)
{
	SC_SecCpyStruct         X_A         secCpyPara;
	u16         volatile    X_A         retStatus;

	/*
		1.  SECURE_COPY_FLAG != isSecureCopy
		2.  s in not in ram
		3.  s and t is not overloap
	*/
	if(SC_IsCopyNotInSec() || !isInRam(s))
	{
		ecpMem_Move(t, s, (s32)length);
	}
	else
	{
		secCpyPara.secCpySrc    = s;
		secCpyPara.secCpyDes	= t;
		secCpyPara.secCpyLength = length;

		SC_getParameterChecksum(secCpyPara);

		retStatus 		= ECP_ERR_FAILURE;

		retStatus		= SC_MemCopyRam2Ram(secCpyPara);

		SC_checkReturnCode(retStatus);
	}
}

u16 SC_MemCopyAPI(PM8 t, PM8 s, u16 length, u8 copyType)
{
	SC_SecCpyStruct         X_A         secCpyPara;
	u16         volatile    X_A         retStatus;

	secCpyPara.secCpySrc    = s;
	secCpyPara.secCpyDes	= t;
	secCpyPara.secCpyLength = length;

	SC_getParameterChecksum(secCpyPara);

	retStatus 		= ECP_ERR_FAILURE;
//	if(ECP_COPY_TYPE_RAM_2_RAM == copyType)
//	{
//		retStatus		= SC_MemCopyRam2Ram(secCpyPara);
//	}
//	else if(ECP_COPY_TYPE_ROM_2_RAM == copyType)
//	{
//		retStatus		= SC_MemCopyRom2Ram(secCpyPara);
//	}
//	else
//	{
		retStatus 		= SC_MemCopy(secCpyPara, copyType);
//	}
	SC_checkReturnCode(retStatus);
	return retStatus;
}
#else
u16 SC_MemCopyAPI(PM8 t, PM8 s, u16 length, u8 copyType)
{

	if(ECP_COPY_TYPE_RAM_2_RAM == copyType)
	{
		ecpMem_Move(t, s, length);
	}
	else if(ECP_COPY_TYPE_MEM_2_RAM == copyType)
	{
		ecpMem_Copy(t, s, length);
	}
	else if(ECP_COPY_TYPE_ATOMIC == copyType)
	{
		memCopyAtomic(t, s, length);
	}
	else
	{
		memCopyNoAtomic(t, s, length);
	}
	return ECP_ERR_OK;
}
#endif //#if SEC_MEM_OPERATION
