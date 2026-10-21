/*
 * Sym_API.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */
#include "Sym_API.h"
#include "Crypto_API.h"
#include "MemOperation.h"
#include "Crypto_Constant.h"
#include "SysConfigTarget.h"
#include "SecureOperation.h"
#include "secureCrypto.h" 
#if WITH_SYM
#define SYM_PADDING_ISO9797_M1		0x55AA
#define SYM_PADDING_ISO9797_M2		0x5555
#define SYM_PADDING_PKCS5			0xAA55
#define SYM_PADDING_NOPAD			0xAAAA
#define SYM_PADDING_AEADPAD			0xA355
#define SYM_PADDING_UNUESD			0x0000

#if WITH_DES
/*******************************************************************************
 * Function name: DESPadAlg
 * Description: DES padding algorithm.
 * Input Parameters:   (precondition)
 *		PX8 dataBuff	  the buffer to be padded.
 *		u8 dataLen		      byte length in dataBuff.
 *      u8 algType		      algorithm type.
 * Output Parameters:  (postcondition)
 *		the result length in dataBuff.
 * NOTE:
 ********************************************************************************/
static u16 Sym_DESPadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	switch(algType)
	{
	case C_ALG_DES_CBC_ISO9797_M1:  // padding zeros
	case C_ALG_DES_ECB_ISO9797_M1:
	case C_ALG_DES_MAC4_ISO9797_M1:
	case C_ALG_DES_MAC8_ISO9797_M1:
	case C_ALG_DES_MAC4_ISO9797_1_M1_ALG3:
	case C_ALG_DES_MAC8_ISO9797_1_M1_ALG3:
		symPaddingMode = SYM_PADDING_ISO9797_M1;
		break;
	case C_ALG_DES_CBC_ISO9797_M2:  // padding 0x80, and then zeros
	case C_ALG_DES_ECB_ISO9797_M2:
	case C_ALG_DES_MAC4_ISO9797_M2:
	case C_ALG_DES_MAC8_ISO9797_M2:
	case C_ALG_DES_MAC4_ISO9797_1_M2_ALG3:
	case C_ALG_DES_MAC8_ISO9797_1_M2_ALG3:
		symPaddingMode = SYM_PADDING_ISO9797_M2;
		break;
#if DES_PKCS5_PADDING
	case C_ALG_DES_CBC_PKCS5:
	case C_ALG_DES_ECB_PKCS5:
	case C_ALG_DES_MAC4_PKCS5:
	case C_ALG_DES_MAC8_PKCS5:
		symPaddingMode = SYM_PADDING_PKCS5;
		break;
#endif
	case C_ALG_DES_CBC_NOPAD:
	case C_ALG_DES_ECB_NOPAD:
	case C_ALG_DES_MAC4_NOPAD:
	case C_ALG_DES_MAC8_NOPAD:
		symPaddingMode = SYM_PADDING_NOPAD;
		break;
	default:
		faultDetected();
		break;
	}
	return symPaddingMode;
}

/*******************************************************************************
 * Function name: DESDepadAlg
 * Description: DES depadding algorithm.
 * Input Parameters:   (precondition)
 *		PX8 dataBuff	  the buffer to be depadded.
 *		u8 dataLen		      byte length in dataBuff.
 *      u8 algType		      algorithm type.
 * Output Parameters:  (postcondition)
 *		the result length in dataBuff.
 * Output Parameters:
 *      length of depadded
 * NOTE: The last block need to depad for decryption.
 ********************************************************************************/
static u16 Sym_DESDePadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	PATCH_BOOT_SYM_API_002();
	
	switch (algType)
	{
	case ALG_DES_CBC_ISO9797_M1:
	case ALG_DES_ECB_ISO9797_M1:
		symPaddingMode = SYM_PADDING_ISO9797_M1;
		break;
	case ALG_DES_CBC_ISO9797_M2:  // depadding 0x80, and then zeros
	case ALG_DES_ECB_ISO9797_M2:
		symPaddingMode = SYM_PADDING_ISO9797_M2;
		break;
#if DES_PKCS5_PADDING
	case ALG_DES_CBC_PKCS5:
	case ALG_DES_ECB_PKCS5:
		symPaddingMode = SYM_PADDING_PKCS5;
		break;
#endif
	case ALG_DES_CBC_NOPAD:
	case ALG_DES_ECB_NOPAD:
		symPaddingMode = SYM_PADDING_NOPAD;
		break;
	default:
		faultDetected();
		break;
	}
	return symPaddingMode;
}


/*******************************************************************************
 * Function name: single_DESMAC
 * Description: Signle DES MAC algorithm, the input data must be multiple of eight.
 * Input Parameters:   (precondition)
 *		u8 * dataIn			     address of source buffer.
 *      u8 * key		         input initial key buffer.
 *		u8 * dataOut			 address of destination buffer.
 *		u16 length				 length of source buffer.
 *		u8 * pIV     	         initial const vector used in CBC mode.
 * Output Parameters:  (postcondition)
 *		none.
 * NOTE:
 ********************************************************************************/
void single_DESMAC(PX8 dataIn, PM8 keyBuff, PX8 dataOut, u16 length, PM8 ICVBuff)
{
	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	u32					X_A		symKeyBuffer[DES_EXT_KEY_BYTE_LEN >> 2];
	
	PATCH_BOOT_SYM_API_003();
	
	// length not multiple of DES_BlOCKSIZE or length is zero.
	if (0x00 != (U16_AND_U16(length, DES_BLOCKSIZE_MASK)) || (0x00 == length))
	{
		return;
	}
	// initialize values for SYM operation
	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, LENGTH_DES, CBC_MAC_Encrypt, SENSITIVE_DATA_NOT_MASK, ALG_DES);
	if (ERR_SYM_LOADKEY_OK != Sym_DES_LoadKey(&myLoadKeyParam))
	{
		faultDetected();
	}

		SymCryptParamsInit(&myCryptParam, ICVBuff, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), dataIn, dataOut, CBC_MAC_Encrypt);
	myCryptParam.noOfBlocks = U16_SHR_U8(length, 3);
	if (ERR_SYM_CRYPT_OK != Sym_DES_Crypt(&myCryptParam))
	{
		faultDetected();
	}

	// update ICVBuff
	ecpMem_Copy(ICVBuff, dataOut, DES_BLOCKSIZE);
	Sym_Wipekey();
	return;
}
/*******************************************************************************
 * Function name: Triple_DES
 * Description: Triple DES 2 key algorithm, the input data must be multiple of eight.
 * Input Parameters:   (precondition)
 *		u8 * dataIn			     address of source buffer.
 *      u8 * key		         input initial key buffer.
 *		u8 * dataOut			 address of destination buffer.
 *		u16 length				 length of source buffer.
 *		u8 * pIV     	         initial const vector used in CBC mode.
 *		u8 bMode		         the parameters of the algorithm (algorithm type and operation mode).
 * Output Parameters:  (postcondition)
 *		none.
 * NOTE:
 ********************************************************************************/
void Triple_DES(PX8 dataIn, PM8 keyBuff, PX8 dataOut, u16 length, PM8 ICVBuff, u8 mode, u16 flag)
{
	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	u32					X_A		symKeyBuffer[DES_EXT_KEY_BYTE_LEN >> 2];
	u16 				X_A		keyLength;
	PATCH_BOOT_SYM_API_004();
	// length not multiple of DES_BlOCKSIZE or length is zero.
	if (0x00 != (U16_AND_U16(length, DES_BLOCKSIZE_MASK)) || (0x00 == length))
	{
		return;
	}
	
	if(mode & 0x80)
	{
		keyLength = LENGTH_3DES_3KEY; //24 bytes
	}
	else if(mode & 0x40)
	{
		keyLength = LENGTH_DES;   // 8 bytes
	}
	else
	{
		keyLength = LENGTH_3DES_2KEY; // 16 bytes
	}
	
	// initialize values for SYM operation
	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyLength, mode, flag, ALG_DES);
	if (ERR_SYM_LOADKEY_OK != Sym_DES_LoadKey(&myLoadKeyParam))
	{
		faultDetected();
	}

	SymCryptParamsInit(&myCryptParam, ICVBuff, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), dataIn, dataOut, mode);
	myCryptParam.noOfBlocks = U16_SHR_U8(length, 3);

	if (ERR_SYM_CRYPT_OK != Sym_DES_Crypt(&myCryptParam))
	{
		faultDetected();
	}

	// fetch the updated ICV
//	length = U16_S_U8(length, DES_BLOCKSIZE);
//	ecpMem_Copy(ICVBuff, &dataOut[length], DES_BLOCKSIZE);
	Sym_Wipekey();
	return;
}

#endif //WITH_DES

#if WITH_AES
/*******************************************************************************
 * Function name: AESPadAlg
 * Description: AES padding algorithm.
 * Input Parameters:   (precondition)
 *		XDATA_U8_P dataBuff	      the buffer to be padded.
 *		u8 dataLen			      byte length in dataBuff.
 *      u8 algType				  algorithm type.
 * Output Parameters:  (postcondition)
 *		the result length in dataBuff.
 * NOTE:
 ********************************************************************************/
static u16 Sym_AESPadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	switch(algType)
	{
	case C_ALG_AES_CBC_ISO9797_M1:  // padding zeros
	case C_ALG_AES_ECB_ISO9797_M1:
		symPaddingMode = SYM_PADDING_ISO9797_M1;
		break;
	case C_ALG_AES_CBC_ISO9797_M2:  // padding 0x80, and then zeros
	case C_ALG_AES_ECB_ISO9797_M2:
		symPaddingMode = SYM_PADDING_ISO9797_M2;
		break;
#if AES_PKCS5_PADDING
	case C_ALG_AES_CBC_PKCS5:
	case C_ALG_AES_ECB_PKCS5:
		symPaddingMode = SYM_PADDING_PKCS5;
		break;
#endif

	case C_ALG_AES_BLOCK_128_CBC_NOPAD:
	case C_ALG_AES_BLOCK_128_ECB_NOPAD:
		symPaddingMode = SYM_PADDING_NOPAD;
		break;
#if WITH_AEAD
	case ALG_AES_CCM:
	case ALG_AES_GCM:
		symPaddingMode = SYM_PADDING_AEADPAD;
		break;
#endif
	default:
		symPaddingMode = SYM_PADDING_NOPAD;
		break;
	}
	return symPaddingMode;
}

/*******************************************************************************
 * Function name: AESDepadAlg
 * Description: AES depadding algorithm.
 * Input Parameters:   (precondition)
 *		XDATA_U8_P dataBuff	      the buffer to be depadded.
 *		u8 dataLen			      byte length in dataBuff.
 *      u8 algType				  algorithm type.
 * Output Parameters:  (postcondition)
 *		the result length in dataBuff.
 * Output Parameters:
 *      length of depadded
 * NOTE: The last block need to depad for decryption.
 ********************************************************************************/
static u16 Sym_AESDePadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	switch (algType)
	{
	case ALG_AES_CBC_ISO9797_M1:
	case ALG_AES_ECB_ISO9797_M1:
		symPaddingMode = SYM_PADDING_ISO9797_M1;
		break;
	case ALG_AES_CBC_ISO9797_M2:  // depadding 0x80, and then zeros
	case ALG_AES_ECB_ISO9797_M2:
		symPaddingMode = SYM_PADDING_ISO9797_M2;
		break;
#if AES_PKCS5_PADDING
	case ALG_AES_CBC_PKCS5:
	case ALG_AES_ECB_PKCS5:
		symPaddingMode = SYM_PADDING_PKCS5;
		break;
#endif

	case ALG_AES_BLOCK_128_CBC_NOPAD:
	case ALG_AES_BLOCK_128_ECB_NOPAD:
		symPaddingMode = SYM_PADDING_NOPAD;
		break;
#if WITH_AEAD
	case ALG_AES_CCM:
	case ALG_AES_GCM:
		symPaddingMode = SYM_PADDING_AEADPAD;
		break;
#endif
	default:
		faultDetected();
		break;
	}
	return symPaddingMode;
}

#endif //WITH_AES


#if WITH_SM1

static u16 Sym_SM1PadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	PATCH_BOOT_SYM_API_008();
	if(!GUOMI_STANDARD)
	{
		switch(algType)
		{
		case C_ALG_SM1_CBC_ISO9797_M1: // padding zeros
		case C_ALG_SM1_ECB_ISO9797_M1:
		case C_ALG_SM1_MAC16_ISO9797_M1:
		case C_ALG_SM1_MAC8_ISO9797_M1:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case C_ALG_SM1_CBC_ISO9797_M2:  // padding 0x80, and then zeros
		case C_ALG_SM1_ECB_ISO9797_M2:
		case C_ALG_SM1_MAC8_ISO9797_M2:
		case C_ALG_SM1_MAC16_ISO9797_M2:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case C_ALG_SM1_CBC_NOPAD:
		case C_ALG_SM1_ECB_NOPAD:
		case C_ALG_SM1_MAC8_NOPAD:
		case C_ALG_SM1_MAC16_NOPAD:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			break;
		}
	}
	else
	{
		switch(algType)
		{
		case C_ALG_SM1_CBC_ISO9797_M1_NEW: // padding zeros
		case C_ALG_SM1_ECB_ISO9797_M1_NEW:
		case C_ALG_SM1_MAC16_ISO9797_M1_NEW:
		case C_ALG_SM1_MAC8_ISO9797_M1_NEW:
		case C_ALG_SM1_MAC4_ISO9797_M1_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case C_ALG_SM1_CBC_ISO9797_M2_NEW:  // padding 0x80, and then zeros
		case C_ALG_SM1_ECB_ISO9797_M2_NEW:
		case C_ALG_SM1_MAC4_ISO9797_M2_NEW:
		case C_ALG_SM1_MAC8_ISO9797_M2_NEW:
		case C_ALG_SM1_MAC16_ISO9797_M2_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case C_ALG_SM1_CBC_NOPAD_NEW:
		case C_ALG_SM1_ECB_NOPAD_NEW:
		case C_ALG_SM1_MAC4_NOPAD_NEW:
		case C_ALG_SM1_MAC8_NOPAD_NEW:
		case C_ALG_SM1_MAC16_NOPAD_NEW:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			break;
		}		
	}
	return symPaddingMode;
}

static u16 Sym_SM1DePadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	if(!GUOMI_STANDARD)
	{
		switch (algType)
		{
		case ALG_SM1_CBC_ISO9797_M1:
		case ALG_SM1_ECB_ISO9797_M1:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case ALG_SM1_CBC_ISO9797_M2:  // depadding 0x80, and then zeros
		case ALG_SM1_ECB_ISO9797_M2:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case ALG_SM1_CBC_NOPAD:
		case ALG_SM1_ECB_NOPAD:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (algType)
		{
		case ALG_SM1_CBC_ISO9797_M1_NEW:
		case ALG_SM1_ECB_ISO9797_M1_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case ALG_SM1_CBC_ISO9797_M2_NEW:  // depadding 0x80, and then zeros
		case ALG_SM1_ECB_ISO9797_M2_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case ALG_SM1_CBC_NOPAD_NEW:
		case ALG_SM1_ECB_NOPAD_NEW:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			break;
		}

	}
	return symPaddingMode;
}

#endif //WITH_SM1
#if WITH_SM4

static u16 Sym_SM4PadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	if(!GUOMI_STANDARD)
	{
		switch(algType)
		{
		case C_ALG_SM4_CBC_ISO9797_M1: // padding zeros
		case C_ALG_SM4_ECB_ISO9797_M1:
		case C_ALG_SM4_MAC16_ISO9797_M1:
		case C_ALG_SM4_MAC8_ISO9797_M1:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case C_ALG_SM4_CBC_ISO9797_M2:  // padding 0x80, and then zeros
		case C_ALG_SM4_ECB_ISO9797_M2:
		case C_ALG_SM4_MAC8_ISO9797_M2:
		case C_ALG_SM4_MAC16_ISO9797_M2:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case C_ALG_SM4_CBC_NOPAD:
		case C_ALG_SM4_ECB_NOPAD:
		case C_ALG_SM4_MAC8_NOPAD:
		case C_ALG_SM4_MAC16_NOPAD:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			faultDetected();
			break;
		}
	}
	else
	{
		switch(algType)
		{
		case C_ALG_SM4_CBC_ISO9797_M1_NEW: // padding zeros
		case C_ALG_SM4_ECB_ISO9797_M1_NEW:
		case C_ALG_SM4_MAC16_ISO9797_M1_NEW:
		case C_ALG_SM4_MAC8_ISO9797_M1_NEW:
		case C_ALG_SM4_MAC4_ISO9797_M1_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case C_ALG_SM4_CBC_ISO9797_M2_NEW:  // padding 0x80, and then zeros
		case C_ALG_SM4_ECB_ISO9797_M2_NEW:
		case C_ALG_SM4_MAC4_ISO9797_M2_NEW:
		case C_ALG_SM4_MAC8_ISO9797_M2_NEW:
		case C_ALG_SM4_MAC16_ISO9797_M2_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case C_ALG_SM4_MAC4_PKCS5_NEW:
		case C_ALG_SM4_MAC8_PKCS5_NEW:
		case C_ALG_SM4_MAC16_PKCS5_NEW:
		case C_ALG_SM4_CBC_PKCS5_NEW:
		case C_ALG_SM4_ECB_PKCS5_NEW:
			symPaddingMode = SYM_PADDING_PKCS5;
			break;
		case C_ALG_SM4_CBC_NOPAD_NEW:
		case C_ALG_SM4_ECB_NOPAD_NEW:
		case C_ALG_SM4_MAC4_NOPAD_NEW:
		case C_ALG_SM4_MAC8_NOPAD_NEW:
		case C_ALG_SM4_MAC16_NOPAD_NEW:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			faultDetected();
			break;
		}		
	}
	return symPaddingMode;
}

static u16 Sym_SM4DePadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
	
	if(!GUOMI_STANDARD)
	{
		switch (algType)
		{
		case ALG_SM4_CBC_ISO9797_M1:
		case ALG_SM4_ECB_ISO9797_M1:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case ALG_SM4_CBC_ISO9797_M2:  // depadding 0x80, and then zeros
		case ALG_SM4_ECB_ISO9797_M2:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case ALG_SM4_CBC_NOPAD:
		case ALG_SM4_ECB_NOPAD:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			faultDetected();
			break;
		}
	}
	else
	{
		switch (algType)
		{
		case ALG_SM4_CBC_ISO9797_M1_NEW:
		case ALG_SM4_ECB_ISO9797_M1_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case ALG_SM4_CBC_ISO9797_M2_NEW:  // depadding 0x80, and then zeros
		case ALG_SM4_ECB_ISO9797_M2_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case ALG_SM4_CBC_NOPAD_NEW:
		case ALG_SM4_ECB_NOPAD_NEW:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		case C_ALG_SM4_MAC4_PKCS5_NEW:
		case C_ALG_SM4_MAC8_PKCS5_NEW:
		case C_ALG_SM4_MAC16_PKCS5_NEW:
		case C_ALG_SM4_CBC_PKCS5_NEW:
		case C_ALG_SM4_ECB_PKCS5_NEW:
			symPaddingMode = SYM_PADDING_PKCS5;
			break;
		default:
			faultDetected();
			break;
		}

	}
	return symPaddingMode;
}

#endif //WITH_SM4

#if WITH_SM7

static u16 Sym_SM7PadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
//	if(!GUOMI_STANDARD)
//	{
//		switch(algType)
//		{
//		case C_ALG_SM7_CBC_ISO9797_M1: // padding zeros
//		case C_ALG_SM7_ECB_ISO9797_M1:
//		case C_ALG_SM7_MAC16_ISO9797_M1:
//		case C_ALG_SM7_MAC8_ISO9797_M1:
//			if (dataLen & SYM_ISO9797_M1_PAD_MASK)
//			{
//				ecpMem_Set(dataBuff, 0, SM7_BLOCKSIZE);
//				dataLen = SM7_BLOCKSIZE;
//			}
//			else
//			{
//				loopCounter = dataLen & SM7_BLOCKSIZE_MASK;
//				if (loopCounter != 0)
//				{
//					loopCounter = SM7_BLOCKSIZE - loopCounter;
//					while (loopCounter != 0)
//					{
//						dataBuff[dataLen] = 0;
//						U8_INC(dataLen);
//						U8_DEC(loopCounter);
//					}
//				}
//			}
//			break;
//		case C_ALG_SM7_CBC_ISO9797_M2:  // padding 0x80, and then zeros
//		case C_ALG_SM7_ECB_ISO9797_M2:
//		case C_ALG_SM7_MAC8_ISO9797_M2:
//		case C_ALG_SM7_MAC16_ISO9797_M2:
//			dataBuff[dataLen] = 0x80;
//			U8_INC(dataLen);
//			loopCounter = dataLen & SM7_BLOCKSIZE_MASK;
//			if (loopCounter != 0)
//			{
//				loopCounter = SM7_BLOCKSIZE - loopCounter;
//				while (loopCounter != 0)
//				{
//					dataBuff[dataLen] = 0;
//					U8_INC(dataLen);
//					U8_DEC(loopCounter);
//				}
//			}
//			break;
//		case C_ALG_SM7_CBC_NOPAD:
//		case C_ALG_SM7_ECB_NOPAD:
//		case C_ALG_SM7_MAC8_NOPAD:
//		case C_ALG_SM7_MAC16_NOPAD:
//			break;
//		default:
//			faultDetected();
//			break;
//		}
//	}
//	else
//	{
		switch(algType)
		{
		case C_ALG_SM7_CBC_ISO9797_M1_NEW: // padding zeros
		case C_ALG_SM7_ECB_ISO9797_M1_NEW:
		case C_ALG_SM7_MAC8_ISO9797_M1_NEW:
		case C_ALG_SM7_MAC4_ISO9797_M1_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case C_ALG_SM7_CBC_ISO9797_M2_NEW:  // padding 0x80, and then zeros
		case C_ALG_SM7_ECB_ISO9797_M2_NEW:
		case C_ALG_SM7_MAC4_ISO9797_M2_NEW:
		case C_ALG_SM7_MAC8_ISO9797_M2_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case C_ALG_SM7_CBC_NOPAD_NEW:
		case C_ALG_SM7_ECB_NOPAD_NEW:
		case C_ALG_SM7_MAC4_NOPAD_NEW:
		case C_ALG_SM7_MAC8_NOPAD_NEW:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			faultDetected();
			break;
		}		
//	}
	return symPaddingMode;
}

static u16 Sym_SM7DePadAlg(PX8 dataBuff, u16 dataLen, u8 algType)
{
	u16 symPaddingMode;
	symPaddingMode = SYM_PADDING_UNUESD;
//	if(!GUOMI_STANDARD)
//	{
//		switch (algType)
//		{
//		case ALG_SM7_CBC_ISO9797_M1:
//		case ALG_SM7_ECB_ISO9797_M1:
//			// do nothing. ?
//	//		for (loopCounter = SM7_BLOCKSIZE - 0x01; loopCounter != 0xFF; loopCounter--)
//	//		{//due to the type of loopCounter, loopCounter is always bigger than zero
//	//			if (dataBuff[dataLen - 1] != 0x00)
//	//			{
//	//				break;
//	//			}
//	//			//depaddedLen++;
//	//			U8_DEC(dataLen);
//	//		}
//			break;
//		case ALG_SM7_CBC_ISO9797_M2:  // depadding 0x80, and then zeros
//		case ALG_SM7_ECB_ISO9797_M2:
//			U8_DEC(dataLen);
//			for (loopCounter = 0; loopCounter < SM7_BLOCKSIZE; loopCounter++)
//			{
//				if (dataBuff[dataLen] != 0x80)
//				{
//					U8_DEC(dataLen);
//				}
//				else
//				{
//					break;
//				}
//			}

//			if (loopCounter == SM7_BLOCKSIZE)
//			{
//				dataLen = WRONG_PADDING;
//			}
//			else
//			{
//				dataBuff[dataLen] = 0x00;
//			}
//			break;
//		case ALG_SM7_CBC_NOPAD:
//		case ALG_SM7_ECB_NOPAD:
//			break;
//		default:
//			faultDetected();
//			break;
//		}
//	}
//	else
//	{
		switch (algType)
		{
		case ALG_SM7_CBC_ISO9797_M1_NEW:
		case ALG_SM7_ECB_ISO9797_M1_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M1;
			break;
		case ALG_SM7_CBC_ISO9797_M2_NEW:  // depadding 0x80, and then zeros
		case ALG_SM7_ECB_ISO9797_M2_NEW:
			symPaddingMode = SYM_PADDING_ISO9797_M2;
			break;
		case ALG_SM7_CBC_NOPAD_NEW:
		case ALG_SM7_ECB_NOPAD_NEW:
			symPaddingMode = SYM_PADDING_NOPAD;
			break;
		default:
			faultDetected();
			break;
		}

//	}
	return symPaddingMode;
}

#endif //WITH_SM7

u8 getSYMalgType(u8 symAlg, u8 algType, u8 currentMode)
{
	u8 		D_A		algTypeMode;
	
	algTypeMode 	= SYM_MODE_NONE;

	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_DES
		case ALG_DES:
			if(CIPHER_MODE == currentMode)
			{
				if( (ALG_DES_CBC_NOPAD == algType) || (ALG_DES_ECB_NOPAD == algType) )
				{
					algTypeMode = SYM_MODE_NOPAD;
				}
				else if ( (ALG_DES_CBC_ISO9797_M1 == algType) || (ALG_DES_ECB_ISO9797_M1 == algType) )
				{
					algTypeMode	= SYM_MODE_ISO9797_M1;
				}
			}
			else
			{
				if((ALG_DES_MAC4_ISO9797_1_M2_ALG3 == algType) || (ALG_DES_MAC8_ISO9797_1_M2_ALG3 == algType)|| (ALG_DES_MAC4_ISO9797_1_M1_ALG3 == algType) || (ALG_DES_MAC8_ISO9797_1_M1_ALG3 == algType))
				{
					algTypeMode	= SYM_MODE_ISO9797_1_ALG3;
				}
				else if((ALG_DES_MAC4_ISO9797_M1 == algType) || (ALG_DES_MAC8_ISO9797_M1 == algType))
				{
					algTypeMode	= SYM_MODE_ISO9797_M1;
				}
				else if( (ALG_DES_MAC4_NOPAD == algType) || (ALG_DES_MAC8_NOPAD == algType) )
				{
					algTypeMode = SYM_MODE_NOPAD;
				}else if(ALG_DES_CMAC8 == algType){
					algTypeMode = SYM_MODE_CMAC;
				}
			}
		break;
#endif //WITH_DES
#if WITH_AES
		case ALG_AES:
			if(CIPHER_MODE == currentMode)
			{
				if( (ALG_AES_BLOCK_128_CBC_NOPAD == algType) || (ALG_AES_BLOCK_128_ECB_NOPAD == algType) )
				{
					algTypeMode = SYM_MODE_NOPAD;
				}
				else if ( (ALG_AES_CBC_ISO9797_M1 == algType) || (ALG_AES_ECB_ISO9797_M1 == algType) )
				{
					algTypeMode = SYM_MODE_ISO9797_M1;
				}
#if WITH_AEAD
				else if (ALG_AES_GCM == algType)
				{
					algTypeMode = SYM_MODE_AEAD_GCM;
				}
				else if (ALG_AES_CCM == algType)
				{
					algTypeMode = SYM_MODE_AEAD_CCM;
				}
#endif
			}
			else
			{
// There is only ALG_AES_MAC_128_NOPAD, so just make sure is block aligned!
//				if( (ALG_AES_MAC_128_NOPAD == algType) )
//				{
//				}
				if(ALG_AES_CMAC16 == algType){
					algTypeMode = SYM_MODE_CMAC;
				}
				else
				{
					algTypeMode = SYM_MODE_NOPAD;
				}
			}
		break;
#endif //WITH_AES
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
			if(CIPHER_MODE == currentMode)
			{
				;
			}
			else
			{
// There is only ALG_KOREAN_SEED_MAC_NOPAD, so just make sure is block aligned!
//				if(ALG_KOREAN_SEED_MAC_NOPAD == algType) 
//				{
					algTypeMode = SYM_MODE_NOPAD;
//				}
			}
		break;
#endif //WITH_KOREANSEED
#if WITH_SM1
		case ALG_SM1:
			if(!GUOMI_STANDARD)
			{
				if(CIPHER_MODE == currentMode)
				{
					if( (ALG_SM1_CBC_NOPAD == algType) || (ALG_SM1_ECB_NOPAD == algType) )
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if ( (ALG_SM1_CBC_ISO9797_M1 == algType) || (ALG_SM1_ECB_ISO9797_M1 == algType) )
					{
						algTypeMode = SYM_MODE_ISO9797_M1;
					}
				}
				else
				{
					if ((algType == ALG_SM1_MAC16_NOPAD) || (algType == ALG_SM1_MAC8_NOPAD))  // NOPAD mode
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					if((ALG_SM1_MAC8_ISO9797_M1 == algType) || (ALG_SM1_MAC16_ISO9797_M1 == algType))
					{
						algTypeMode	= SYM_MODE_ISO9797_M1;
					}
				}
			}
			else
			{
				if(CIPHER_MODE == currentMode)
				{
					if( (ALG_SM1_CBC_NOPAD_NEW == algType) || (ALG_SM1_ECB_NOPAD_NEW == algType) )
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if ( (ALG_SM1_CBC_ISO9797_M1_NEW == algType) || (ALG_SM1_ECB_ISO9797_M1_NEW == algType) )
					{
						algTypeMode = SYM_MODE_ISO9797_M1;
					}
				}
				else
				{
					if ((algType == ALG_SM1_MAC4_NOPAD_NEW) || (algType == ALG_SM1_MAC16_NOPAD_NEW) || (algType == ALG_SM1_MAC8_NOPAD_NEW))  // NOPAD mode
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if((ALG_SM1_MAC4_ISO9797_M1_NEW == algType) || (ALG_SM1_MAC8_ISO9797_M1_NEW == algType) || (ALG_SM1_MAC16_ISO9797_M1_NEW == algType))
					{
						algTypeMode				= SYM_MODE_ISO9797_M1;
					}
				}
				
			}
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			if(!GUOMI_STANDARD)
			{
				if(CIPHER_MODE == currentMode)
				{
					if( (ALG_SM4_CBC_NOPAD == algType) || (ALG_SM4_ECB_NOPAD == algType) )
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if ( (ALG_SM4_CBC_ISO9797_M1 == algType) || (ALG_SM4_ECB_ISO9797_M1 == algType) )
					{
						algTypeMode = SYM_MODE_ISO9797_M1;
					}
				}
				else
				{
					if ((algType == ALG_SM4_MAC16_NOPAD) || (algType == ALG_SM4_MAC8_NOPAD))  // NOPAD mode
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					if((ALG_SM4_MAC8_ISO9797_M1 == algType) || (ALG_SM4_MAC16_ISO9797_M1 == algType))
					{
						algTypeMode	= SYM_MODE_ISO9797_M1;
					}
				}
			}
			else
			{
				if(CIPHER_MODE == currentMode)
				{
					if( (ALG_SM4_CBC_NOPAD_NEW == algType) || (ALG_SM4_ECB_NOPAD_NEW == algType) )
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if ( (ALG_SM4_CBC_ISO9797_M1_NEW == algType) || (ALG_SM4_ECB_ISO9797_M1_NEW == algType) )
					{
						algTypeMode = SYM_MODE_ISO9797_M1;
					}
				}
				else
				{
					if ((algType == ALG_SM4_MAC4_NOPAD_NEW) || (algType == ALG_SM4_MAC16_NOPAD_NEW) || (algType == ALG_SM4_MAC8_NOPAD_NEW))  // NOPAD mode
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if((ALG_SM4_MAC4_ISO9797_M1_NEW == algType) || (ALG_SM4_MAC8_ISO9797_M1_NEW == algType) || (ALG_SM4_MAC16_ISO9797_M1_NEW == algType))
					{
						algTypeMode				= SYM_MODE_ISO9797_M1;
					}
				}
				
			}
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
//			if(!GUOMI_STANDARD)
//			{
//				if(CIPHER_MODE == currentMode)
//				{
//					if( (ALG_SM7_CBC_NOPAD == algType) || (ALG_SM7_ECB_NOPAD == algType) )
//					{
//						algTypeMode = SYM_MODE_NOPAD;
//					}
//					else if ( (ALG_SM7_CBC_ISO9797_M1 == algType) || (ALG_SM7_ECB_ISO9797_M1 == algType) )
//					{
//						algTypeMode = SYM_MODE_ISO9797_M1;
//					}
//				}
//				else
//				{
//					if ((algType == ALG_SM7_MAC16_NOPAD) || (algType == ALG_SM7_MAC8_NOPAD))  // NOPAD mode
//					{
//						algTypeMode = SYM_MODE_NOPAD;
//					}
//					if((ALG_SM7_MAC8_ISO9797_M1 == algType) || (ALG_SM7_MAC16_ISO9797_M1 == algType))
//					{
//						algTypeMode	= SYM_MODE_ISO9797_M1;
//					}
//				}
//			}
//			else
			{
				if(CIPHER_MODE == currentMode)
				{
					if( (ALG_SM7_CBC_NOPAD_NEW == algType) || (ALG_SM7_ECB_NOPAD_NEW == algType) )
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if ( (ALG_SM7_CBC_ISO9797_M1_NEW == algType) || (ALG_SM7_ECB_ISO9797_M1_NEW == algType) )
					{
						algTypeMode = SYM_MODE_ISO9797_M1;
					}
				}
				else
				{
					if ((algType == ALG_SM7_MAC4_NOPAD_NEW) || (algType == ALG_SM7_MAC8_NOPAD_NEW))  // NOPAD mode
					{
						algTypeMode = SYM_MODE_NOPAD;
					}
					else if((ALG_SM7_MAC4_ISO9797_M1_NEW == algType) || (ALG_SM7_MAC8_ISO9797_M1_NEW == algType) )
					{
						algTypeMode				= SYM_MODE_ISO9797_M1;
					}
				}
			}	
//		}
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			if(CIPHER_MODE == currentMode)
			{
				;
			}
			else
			{
				if ((algType == ALG_SSF33_MAC16_NOPAD) || (algType == ALG_SSF33_MAC8_NOPAD))  // NOPAD mode
				{
					algTypeMode = SYM_MODE_NOPAD;
				}
			}
		break;
#endif //WITH_SSF33
		default:
			faultDetected();
		break;
	}
	return algTypeMode;
}

u8 getSYMBlockSize(u8 symAlg)
{
	u8		D_A		blockSize;
	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_DES
		case ALG_DES:
			blockSize 		= DES_BLOCKSIZE;
		break;
#endif //WITH_DES
#if WITH_AES
		case ALG_AES:
			blockSize 		= AES_BLOCKSIZE;
		break;
#endif //WITH_AES
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
			blockSize 		= KOREANSEED_BLOCKSIZE;
		break;
#endif //WITH_KOREANSEED
#if WITH_SM1
		case ALG_SM1:
			blockSize 		= SM1_BLOCKSIZE;
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			blockSize 		= SM4_BLOCKSIZE;
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
			blockSize 		= SM7_BLOCKSIZE;
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			blockSize 		= SSF33_BLOCKSIZE;
		break;
#endif //WITH_SSF33
		default:
			faultDetected();
		break;
	}
	return blockSize;
}

u16 getSYMSinReturnLength(u8 symAlg, u8 algType, u16 inLen)
{
	u8 retLen = inLen;
	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_DES
		case ALG_DES:
			switch (algType)
			{
				case ALG_DES_MAC4_NOPAD:
				case ALG_DES_MAC4_ISO9797_M1:
				case ALG_DES_MAC4_ISO9797_M2:
				case ALG_DES_MAC4_PKCS5:
				case ALG_DES_MAC4_ISO9797_1_M2_ALG3:
				case ALG_DES_MAC4_ISO9797_1_M1_ALG3:
					retLen 	= 4;
				break;
				case ALG_DES_MAC8_NOPAD:
				case ALG_DES_MAC8_ISO9797_M1:
				case ALG_DES_MAC8_ISO9797_M2:
				case ALG_DES_MAC8_PKCS5:
				case ALG_DES_MAC8_ISO9797_1_M2_ALG3:
				case ALG_DES_MAC8_ISO9797_1_M1_ALG3:
				case ALG_DES_CMAC8:
					retLen 	= 8;
				break;
				default:
					faultDetected();
					break;
			}
		break;
#endif //WITH_DES
#if WITH_AES
		case ALG_AES:
			retLen 		= AES_BLOCKSIZE;
		break;
#endif //WITH_AES
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
			retLen 		= KOREANSEED_BLOCKSIZE;
		break;
#endif //WITH_KOREANSEED
#if WITH_SM1
		case ALG_SM1:
			if(!GUOMI_STANDARD)
			{
				switch (algType)
				{
					case ALG_SM1_MAC8_NOPAD:
					case ALG_SM1_MAC8_ISO9797_M1:
					case ALG_SM1_MAC8_ISO9797_M2:
						retLen 	= 8;
					break;
					case ALG_SM1_MAC16_NOPAD:
					case ALG_SM1_MAC16_ISO9797_M1:
					case ALG_SM1_MAC16_ISO9797_M2:
						retLen 	= 16;
					break;
					default:
						faultDetected();
						break;
				}
			}
			else
			{
				switch (algType)
				{
					case ALG_SM1_MAC4_NOPAD_NEW:
					case ALG_SM1_MAC4_ISO9797_M1_NEW:
					case ALG_SM1_MAC4_ISO9797_M2_NEW:
						retLen 	= 4;
						break;
					case ALG_SM1_MAC8_NOPAD_NEW:
					case ALG_SM1_MAC8_ISO9797_M1_NEW:
					case ALG_SM1_MAC8_ISO9797_M2_NEW:
						retLen 	= 8;
						break;
					case ALG_SM1_MAC16_NOPAD_NEW:
					case ALG_SM1_MAC16_ISO9797_M1_NEW:
					case ALG_SM1_MAC16_ISO9797_M2_NEW:
						retLen 	= 16;
						break;
					default:
						faultDetected();
						break;
				}
			}
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			if(!GUOMI_STANDARD)
			{
				switch (algType)
				{
					case ALG_SM4_MAC8_NOPAD:
					case ALG_SM4_MAC8_ISO9797_M1:
					case ALG_SM4_MAC8_ISO9797_M2:
						retLen 	= 8;
					break;
					case ALG_SM4_MAC16_NOPAD:
					case ALG_SM4_MAC16_ISO9797_M1:
					case ALG_SM4_MAC16_ISO9797_M2:
						retLen 	= 16;
					break;
					default:
						faultDetected();
						break;
				}
			}
			else
			{
				switch (algType)
				{
					case ALG_SM4_MAC4_NOPAD_NEW:
					case ALG_SM4_MAC4_ISO9797_M1_NEW:
					case ALG_SM4_MAC4_ISO9797_M2_NEW:
					case ALG_SM4_MAC4_PKCS5_NEW:
						retLen 	= 4;
						break;
					case ALG_SM4_MAC8_NOPAD_NEW:
					case ALG_SM4_MAC8_ISO9797_M1_NEW:
					case ALG_SM4_MAC8_ISO9797_M2_NEW:
					case ALG_SM4_MAC8_PKCS5_NEW:
						retLen 	= 8;
						break;
					case ALG_SM4_MAC16_NOPAD_NEW:
					case ALG_SM4_MAC16_ISO9797_M1_NEW:
					case ALG_SM4_MAC16_ISO9797_M2_NEW:
					case ALG_SM4_MAC16_PKCS5_NEW:
						retLen 	= 16;
						break;
					default:
						faultDetected();
						break;
				}
			}
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
//			if(!GUOMI_STANDARD)
//			{
//				switch (algType)
//				{
//					case ALG_SM7_MAC8_NOPAD:
//					case ALG_SM7_MAC8_ISO9797_M1:
//					case ALG_SM7_MAC8_ISO9797_M2:
//						retLen 	= 8;
//					break;
//					case ALG_SM7_MAC16_NOPAD:
//					case ALG_SM7_MAC16_ISO9797_M1:
//					case ALG_SM7_MAC16_ISO9797_M2:
//						retLen 	= 16;
//					break;
//					default:
//						faultDetected();
//						break;
//				}
//			}
//			else
//			{
				switch (algType)
				{
					case ALG_SM7_MAC4_NOPAD_NEW:
					case ALG_SM7_MAC4_ISO9797_M1_NEW:
					case ALG_SM7_MAC4_ISO9797_M2_NEW:
						retLen 	= 4;
						break;
					case ALG_SM7_MAC8_NOPAD_NEW:
					case ALG_SM7_MAC8_ISO9797_M1_NEW:
					case ALG_SM7_MAC8_ISO9797_M2_NEW:
						retLen 	= 8;
						break;
					default:
						faultDetected();
						break;
				}
//			}
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			switch (algType)
			{
				case ALG_SSF33_MAC8_NOPAD:
				case ALG_SSF33_MAC8_ISO9797_M1:
				case ALG_SSF33_MAC8_ISO9797_M2:
					retLen 	= 8;
				break;
				case ALG_SSF33_MAC16_NOPAD:
				case ALG_SSF33_MAC16_ISO9797_M1:
				case ALG_SSF33_MAC16_ISO9797_M2:
					retLen 	= 16;
				break;
				default:
					faultDetected();
				break;
			}
		break;
#endif //WITH_SSF33
		default:
		break;
	}
	return retLen;
}

static u16 SYMPadDispatcher(PX8 dataBuff, u16 dataLen, u8 algType, u8 symAlg)
{
	u16 		X_A		result;
	u8 		X_A 			loopCounter;
	u8 		X_A 			symBlockSize;
	u8 		X_A 			symBlockMask;

	result = 0x00;
	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_AES
		case ALG_AES:
			result = Sym_AESPadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_AES
#if WITH_DES
		case ALG_DES:
			result = Sym_DESPadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_DES
#if WITH_SM1
		case ALG_SM1:
			result = Sym_SM1PadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			result = Sym_SM4PadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
			result = Sym_SM7PadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			result = Sym_SSF33PadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SSF33
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
//			result = Sym_KOREANSEEDPadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_KOREANSEED
		default:
			faultDetected();
		break;
	}
	symBlockSize 		= getSYMBlockSize(symAlg);
	symBlockMask 		= (symBlockSize - 1);
	switch(result)
	{
	case SYM_PADDING_ISO9797_M1:  // padding zeros
		if (U16_AND_U16(dataLen, SYM_ISO9797_M1_PAD_MASK))
		{
			ecpMem_Fill(dataBuff, 0, symBlockSize);
			dataLen = symBlockSize;
		}
		else
		{
			loopCounter = U8_AND_U8(dataLen, symBlockMask);
			if (0 != loopCounter)
			{
				loopCounter = U8_S_U8(symBlockSize, loopCounter);
				ecpMem_Set(&dataBuff[dataLen], 0, loopCounter);
				dataLen		= U16_A_U8(dataLen, loopCounter);
			}
		}
		break;
#if WITH_AEAD
	case SYM_PADDING_AEADPAD:	// padding zeros
		if (U16_AND_U16(dataLen, SYM_ISO9797_M1_PAD_MASK))
		{
			;
		}
		else
		{
			loopCounter = U8_AND_U8(dataLen, symBlockMask);
			if (0 != loopCounter)
			{
				loopCounter = U8_S_U8(symBlockSize, loopCounter);
				ecpMem_Set(&dataBuff[dataLen], 0, loopCounter);
			}
		}
		break;
#endif
	case SYM_PADDING_ISO9797_M2:  // padding 0x80, and then zeros
		dataBuff[dataLen] = 0x80;
		U16_INC(dataLen);
		loopCounter = U8_AND_U8(dataLen, symBlockMask);
		if (0 != loopCounter)
		{
			loopCounter = U8_S_U8(symBlockSize, loopCounter);
			ecpMem_Set(&dataBuff[dataLen], 0, loopCounter);
			dataLen		= U16_A_U16(dataLen, loopCounter);
		}
		break;
	case SYM_PADDING_PKCS5:
		loopCounter = U8_AND_U8(dataLen, symBlockMask);
		loopCounter = U8_S_U8(symBlockSize, loopCounter);
//		loopCounter   = pkcs5PadValue;
		ecpMem_Set(&dataBuff[dataLen], loopCounter, loopCounter);
		dataLen		= U16_A_U16(dataLen, loopCounter);
		break;
	case SYM_PADDING_NOPAD:
		break;
	default:
		faultDetected();
		break;
	}
	return dataLen;
}

static u16 SYMDepadDispatcher(PX8 dataBuff, u16 dataLen, u8 algType, u8 symAlg)
{
	u16 		X_A			result;
	u8 		X_A 			loopCounter;
	u8 		X_A 			symBlockSize;
	
	PATCH_BOOT_SYM_API_010();
	
	result = 0x00;
	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_AES
		case ALG_AES:
			result = Sym_AESDePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_AES
#if WITH_DES
		case ALG_DES:
			result = Sym_DESDePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_DES
#if WITH_SM1
		case ALG_SM1:
			result = Sym_SM1DePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			result = Sym_SM4DePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
			result = Sym_SM7DePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			result = Sym_SSF33DePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_SSF33
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
//			result = Sym_KOREANSEEDDePadAlg(dataBuff, dataLen, algType);
		break;
#endif //WITH_KOREANSEED
		default:
			faultDetected();
		break;
	}

	symBlockSize 		= getSYMBlockSize(symAlg);

	switch (result)
	{
	case SYM_PADDING_ISO9797_M1:
#if WITH_AEAD
	case SYM_PADDING_AEADPAD:
#endif
		break;
	case SYM_PADDING_ISO9797_M2:  // depadding 0x80, and then zeros
		U16_DEC(dataLen);
		for (loopCounter = 0; loopCounter < symBlockSize; U8_INC(loopCounter))
		{
			if (0x80 != dataBuff[dataLen])
			{
				U16_DEC(dataLen);
			}
			else
			{
				break;
			}
		}

		if (symBlockSize == loopCounter)
		{
			dataLen = WRONG_PADDING;
		}
		else
		{
			dataBuff[dataLen] = 0x00;
		}
		break;
	case SYM_PADDING_PKCS5:
		loopCounter = dataBuff[U16_S_U8(dataLen, 1)];
		if (loopCounter > symBlockSize)
		{
			dataLen = WRONG_PADDING;
		}
		else
		{
			dataLen = U16_S_U8(dataLen, loopCounter);
			for (loopCounter = dataLen; loopCounter < symBlockSize; U8_INC(loopCounter))
			{
				dataBuff[loopCounter] = 0x00;
			}
		}
		break;
	case SYM_PADDING_NOPAD:
		break;
	default:
		faultDetected();
		break;
	}
	return dataLen;
}

void DealDESSensitiveData(Sym_LoadKey_Param_t * myLoadKeyParam, u16 flag)
{

	PX32 					X_A			keyData;

	keyData					= myLoadKeyParam->pKeyBuffer;

	if((myLoadKeyParam->keyByteLen << 3) == LENGTH_3DES_2KEY)
	{

		if(SENSITIVE_DATA_MASK_NOT_CHECK == flag)
		{
			memCopyToRam(PX8_A_U16(MEMtPX8(keyData) , DES_KEY3_OFFSET), PX8_A_U16(MEMtPX8(keyData) , DES_KEY2_OFFSET), LENGTH_DES_BYTE);
		}
		else
		{
			memCopyToRam(PX8_A_U16(MEMtPX8(keyData) , DES_KEY3_OFFSET), MEMtPX8(keyData), LENGTH_DES_BYTE);
		}

	}
	else if ((myLoadKeyParam->keyByteLen << 3) == LENGTH_DES)
	{
		memCopyToRam(PX8_A_U16(MEMtPX8(keyData) , DES_KEY2_OFFSET), MEMtPX8(keyData), LENGTH_DES_BYTE);
		memCopyToRam(PX8_A_U16(MEMtPX8(keyData) , DES_KEY3_OFFSET), MEMtPX8(keyData), LENGTH_DES_BYTE);
	}
	myLoadKeyParam->keyByteLen = (LENGTH_3DES_3KEY >> 3);
}

void DealSensitiveData(Sym_LoadKey_Param_t * myLoadKeyParam, u16 flag, u8 algType)
{

	switch(U8_AND_U8(algType, SBC_TPYE_MASK))
	{
		#if WITH_AES
		case ALG_AES:
		break;
#endif //WITH_AES
#if WITH_DES
		case ALG_DES:
// 			SC_DealDESSensitiveData(myLoadKeyParam,flag);
		break;
#endif //WITH_DES
#if WITH_SM1
		case ALG_SM1:
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
		break;
#endif //WITH_SSF33
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
		break;
#endif //WITH_KOREANSEED
		default:
			faultDetected();
		break;
	}

}

void SymKeyInitParamsInit(Sym_LoadKey_Param_t * myLoadKeyParam, PM8 keyBuff, PX32 symKeyBuffer, u16 keyBitLen, u8 mode, u16 flag, u8 algType)
{
	myLoadKeyParam->pKeyBuffer 		= 	symKeyBuffer;
	myLoadKeyParam->keyByteLen	 	= 	U16_SHR_U8(keyBitLen, 3);
	
	
	myLoadKeyParam->option 			= 	mode;
	
	if((SENSITIVE_DATA_MASK != flag) && (SENSITIVE_DATA_MASK_NOT_CHECK != flag))
	{
		memCopyToRam(MEMtPX8(myLoadKeyParam->pKeyBuffer), keyBuff, myLoadKeyParam->keyByteLen);
	}
	else
	{
		SC_checkReturnCode(SC_SYMgetKey_C(MEMtPM8(myLoadKeyParam->pKeyBuffer), keyBuff, myLoadKeyParam->keyByteLen, flag));	
	}
	
	DealSensitiveData(myLoadKeyParam, flag, algType);
	return;
}

void SymKeyInitDispatcher(Sym_LoadKey_Param_t * myLoadKeyParam, u8 algType)
{
	u16 	X_A		result;
	result = ERR_SYM_LOADKEY_FAIL;
	switch(U8_AND_U8(algType, SBC_VERIFY_MASK))
	{
#if WITH_AES
		case ALG_AES:
			result = Sym_AES_LoadKey(myLoadKeyParam);
		break;
#if WITH_AEAD
		case ALG_AEAD:
			result = Sym_AEAD_LoadKey(myLoadKeyParam);
		break;
#endif
#endif //WITH_AES
#if WITH_DES
		case ALG_DES:
			result = Sym_DES_LoadKey(myLoadKeyParam);
		break;
#endif //WITH_DES
#if WITH_SM1
		case ALG_SM1:
			result = Sym_SM1_LoadKey(myLoadKeyParam);
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			result = Sym_SM4_LoadKey(myLoadKeyParam);
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
			result = Sym_SM7_LoadKey(myLoadKeyParam);
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			result = Sym_SSF33_LoadKey(myLoadKeyParam);
		break;
#endif //WITH_SSF33
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
			result = Sym_KOREANSEED_LoadKey(myLoadKeyParam);
		break;
#endif //WITH_KOREANSEED
		default:
			faultDetected();
		break;
	}

	if (ERR_SYM_LOADKEY_OK != result)
	{
		faultDetected();
	}
	incrementCodeFlowControlFlag();
	return;

}


void SymCryptParamsInit(Sym_Crypt_Param_t * mycryptParam, PM8 ICVBuff, PX32 symKeyBuffer, u16 keyByteLen, PX8 pInput, PX8 pOutput, u8 mode)
{
	mycryptParam->pIV         	= ICVBuff;// ECB will ignore this value
	mycryptParam->pInput      	= pInput;
	mycryptParam->pOutput     	= pOutput;// pOutput = pInput is possible for ECB en-/decryption, CBC en-/decryption and CBC MAC.
	mycryptParam->pKeyBuffer	= symKeyBuffer;
//	myCryptParam.noOfBlocks   	= SYM_MAX_BLOCKS_ONE_STEP;
	mycryptParam->keyByteLen	 = keyByteLen;
	mycryptParam->option      	= mode; //default use ENCRYPT_ECB_MODE
	return;
}


void SymCryptDispatcher(Sym_Crypt_Param_t * myCryptParam, u8 symAlg)
{
	u16 	X_A		result;
	result = ERR_SYM_CRYPT_FAIL;
	switch(U8_AND_U8(symAlg, SBC_VERIFY_MASK)) 
	{
#if WITH_AES
		case ALG_AES:
			result = Sym_AES_Crypt(myCryptParam);
		break;
#if WITH_AEAD
		case ALG_AEAD:
			result = Sym_AEAD_Crypt(myCryptParam);
		break;
#endif
#endif //WITH_AES
#if WITH_DES
		case ALG_DES:
			result = Sym_DES_Crypt(myCryptParam);
		break;
#endif //WITH_DES
#if WITH_SM1
		case ALG_SM1:
			result = Sym_SM1_Crypt(myCryptParam);
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			result = Sym_SM4_Crypt(myCryptParam);
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
			result = Sym_SM7_Crypt(myCryptParam);
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			result = Sym_SSF33_Crypt(myCryptParam);
		break;
#endif //WITH_SSF33
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
			result = Sym_KOREANSEED_Crypt(myCryptParam);
		break;
#endif //WITH_KOREANSEED
		default:
			faultDetected();
		break;
	}

	if (ERR_SYM_CRYPT_OK != result)
	{
		faultDetected();
	}
	return;
}


void SymAPIParamsInit(Sym_API_Param_t * myAPIParam, u8 symAlg, u8 algType, u8 currentMode)
{
	u8		X_A		algSYM;
	PATCH_BOOT_SYM_API_011();
	algSYM											= U8_AND_U8(symAlg, SBC_TPYE_MASK);
	myAPIParam->option								= getSYMalgType(algSYM, algType, currentMode);
	
	switch(algSYM)
	{
#if WITH_AES
		case ALG_AES:
			myAPIParam->byteNumInBlockMASK 			= AES_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= AES_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= AES_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= AES_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= AES_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= AES_ICV_OFFSET_IN_ONE_STEP;
		break;
#endif //WITH_AES
#if WITH_DES
		case ALG_DES:
			myAPIParam->byteNumInBlockMASK 			= DES_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= DES_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= DES_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= DES_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= DES_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= DES_ICV_OFFSET_IN_ONE_STEP;
			if(SIGNATURE_MODE == currentMode)
			{
				if(SYM_MODE_ISO9797_1_ALG3 == myAPIParam->option)
				{
					myAPIParam->firstKeyByteLen		= BYTE_LENGTH_DES;
					myAPIParam->secondKeyByteLen	= BYTE_LENGTH_3DES_2KEY;
				}
			}
		break;
#endif //WITH_DES
#if WITH_SM1
		case ALG_SM1:
			myAPIParam->byteNumInBlockMASK 			= SM1_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= SM1_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= SM1_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= SM1_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= SM1_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= SM1_ICV_OFFSET_IN_ONE_STEP;
		break;
#endif //WITH_SM1
#if WITH_SM4
		case ALG_SM4:
			myAPIParam->byteNumInBlockMASK 			= SM4_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= SM4_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= SM4_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= SM4_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= SM4_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= SM4_ICV_OFFSET_IN_ONE_STEP;
		break;
#endif //WITH_SM4
#if WITH_SM7
		case ALG_SM7:
			myAPIParam->byteNumInBlockMASK 			= SM7_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= SM7_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= SM7_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= SM7_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= SM7_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= SM7_ICV_OFFSET_IN_ONE_STEP;
		break;
#endif //WITH_SM7
#if WITH_SSF33
		case ALG_SSF33:
			myAPIParam->byteNumInBlockMASK 			= SSF33_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= SSF33_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= SSF33_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= SSF33_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= SSF33_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= SSF33_ICV_OFFSET_IN_ONE_STEP;
		break;
#endif //WITH_SSF33
#if WITH_KOREANSEED
		case ALG_KOREANSEED:
			myAPIParam->byteNumInBlockMASK 			= KOREANSEED_BYTENUM_IN_MULTIBLOCK_MASK;
			myAPIParam->SYMShiftMASK 				= KOREANSEED_SHIFT_MASK;
			myAPIParam->blockSizeMASK				= KOREANSEED_BLOCKSIZE_MASK;
			myAPIParam->blockSize 					= KOREANSEED_BLOCKSIZE;
			myAPIParam->maxBlocksOneStep 			= KOREANSEED_MAX_BLOCKS_ONE_STEP;
			myAPIParam->ICVOffsetInOneStep 			= KOREANSEED_ICV_OFFSET_IN_ONE_STEP;
		break;
#endif //WITH_KOREANSEED
		default:
			faultDetected();
		break;
	}
	
	myAPIParam->optionCheck		= U8_INV(myAPIParam->option);

	return;
}

void SymAPIParamsCheck(Sym_API_Param_t * myAPIParam)
{
	if(myAPIParam->optionCheck != U8_INV(myAPIParam->option))
	{
		faultDetected();
	}
}

/*******************************************************************************
 * Function name: SymCipherUpdate
 * Description: SYM cipher update algorithm.
 * Input Parameters:	(precondition)
 *		PM8 inBuff		address of input buffer.
 *		u16 inLen		length of input buffer.
 *		PM8 outBuff		address of out buffer.
 *      PM8 keyBuff		input key buffer.
 *      u16 keyBitLen	bit length of SYM key.
 *		PM8 ICVBuff		initial const vector used in CBC mode.
 *		PM8 SYMBuff		the parameters of the algorithm (algType |  mode   | tmpData).
 *      u8 mode			the value in the above byte.
 *      u8 sbcState		processing state.
 * Output Parameters:	(postcondition)
 *		none.
 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_DECRYPT or MODE_ENCRYPT
 *       0000 0x00 --- CBC or ECB
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
#if !BOOTLOADER_PRO
void SymCipherUpdate(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 mode, u8 sbcState, u8 algType, u8 symAlg) SMALL_MODE
{
	u32					X_A		symKeyBuffer[SYM_EXT_KEY_BYTE_LEN >> 2];
	u1					B_A 	isEncrypt;
	u16 				D_A 	tmpLen;
	u8 					D_A 	remainLen;
	u8 					X_A 	tmpICV[SYM_BLOCKSIZE];
	u8 					X_A 	tmpDataIn[SYM_MAX_BYTES_ONE_STEP];
	u8 					X_A 	tmpDataOut[SYM_MAX_BYTES_ONE_STEP];

	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	Sym_API_Param_t		X_A		myAPIParam;
	
	PATCH_BOOT_SYM_API_012();
	
	ecpMem_Set(tmpDataOut, 0, SYM_MAX_BYTES_ONE_STEP);
	
	isEncrypt = (MODE_ENCRYPT == U8_AND_U8(mode, MODE_EN_OR_DE_MASK));

	// initialize values for SYM operation
	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, SENSITIVE_DATA_MASK, symAlg);
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);

	SymAPIParamsInit(&myAPIParam, symAlg, algType, CIPHER_MODE);

	SymCryptParamsInit(&myCryptParam, ICVBuff, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), tmpDataIn, tmpDataOut, mode);
	myCryptParam.noOfBlocks = myAPIParam.maxBlocksOneStep;
	myCryptParam.inputLength = SYM_MAX_BYTES_ONE_STEP;

	remainLen = 0;
	if (SBC_UPDATE == sbcState)
	{
		remainLen = U8_SHR_U8(mode, 4);
		if (0 != remainLen)
		{
			memCopyToRam(tmpDataIn, SYMBuff, remainLen);
			inLen = U16_A_U8(inLen, remainLen);
		}
	}

	tmpLen = 0;
	while (inLen >= SYM_MAX_BYTES_ONE_STEP)
	{
		tmpLen = U16_S_U16(SYM_MAX_BYTES_ONE_STEP, remainLen);
		memCopyToRam(&tmpDataIn[remainLen], inBuff, tmpLen);
		remainLen = 0;

		if (false == isEncrypt) // store ICV for decryption
		{
			ecpMem_Copy(tmpICV, &tmpDataIn[myAPIParam.ICVOffsetInOneStep], myAPIParam.blockSize);
		}

		SymCryptDispatcher(&myCryptParam, symAlg);

		if ( SYM_CBC_MODE == U8_AND_U8(mode, SYM_ECB_OR_CBC_MASK) ) // if CBC, updata ICV
		{
			if (false == isEncrypt)
			{
				ecpMem_Copy(ICVBuff, tmpICV, myAPIParam.blockSize);
			}
			else
			{
				ecpMem_Copy(ICVBuff, &tmpDataOut[myAPIParam.ICVOffsetInOneStep], myAPIParam.blockSize);
			}
		}

		memCopyNoAtomic(outBuff, tmpDataOut, SYM_MAX_BYTES_ONE_STEP);
		inLen   = U16_S_U16(inLen, SYM_MAX_BYTES_ONE_STEP);
		inBuff = PM8_A_U16(inBuff, tmpLen);
		outBuff = PM8_A_U16(outBuff, SYM_MAX_BYTES_ONE_STEP);
	}

//	tmpLen = 0;

	if (0 != remainLen)
	{
		memCopyToRam(&tmpDataIn[remainLen], inBuff, U16_S_U8(inLen, remainLen));
	}
	else
	{
		if(0 != inLen)
		{
			memCopyToRam(tmpDataIn, inBuff, inLen);
		}
	}

	if (inLen >= myAPIParam.blockSize)
	{
		tmpLen = U16_AND_U16(inLen, myAPIParam.byteNumInBlockMASK); //(lzm modified according [MISA])

		myCryptParam.noOfBlocks = U16_SHR_U8(tmpLen, myAPIParam.SYMShiftMASK);
		myCryptParam.inputLength = tmpLen;

		if (false == isEncrypt)// store ICV for decryption
		{
			ecpMem_Copy(tmpICV, &tmpDataIn[U16_S_U16(tmpLen, myAPIParam.blockSize)], myAPIParam.blockSize);
		}

		SymCryptDispatcher(&myCryptParam, symAlg);

		if ( SYM_CBC_MODE == U8_AND_U8(mode, SYM_ECB_OR_CBC_MASK) ) // if CBC, updata ICV
		{
			if (false == isEncrypt)
			{
				ecpMem_Copy(ICVBuff, tmpICV, myAPIParam.blockSize);
			}
			else
			{
				ecpMem_Copy(ICVBuff, &tmpDataOut[U16_S_U16(tmpLen, myAPIParam.blockSize)], myAPIParam.blockSize);
			}
		}

		memCopyNoAtomic(outBuff, tmpDataOut, tmpLen);
		inLen     = U16_S_U16(inLen, tmpLen);
		inBuff    = PM8_A_U16(inBuff, U16_S_U16(tmpLen, remainLen));
		remainLen = 0;  //clear remainLen, whether it is 0 or not   //yfj
	}

	if(inLen != 0)
	{
		memCopyNoAtomic(PM8_A_U16(SYMBuff, remainLen), inBuff, U16_S_U16(inLen, remainLen));
	}

	// backup the last 8 bytes decrypted data in order for depadding if required.
	if ((isEncrypt == false) && (inLen == 0))
	{
		memCopyNoAtomic(SYMBuff, &tmpDataOut[U16_S_U16(tmpLen, myAPIParam.blockSize)], myAPIParam.blockSize);
	}

	if (U8_SHR_U8(U8_AND_U8(mode, 0xF0), 4) != inLen)
	{
		mode = U8_AND_U8(mode, 0x0F); // clear the value
		mode = U8_OR_U8(mode, U16t8(U16_SHL_U8(inLen, 4)));//(lzm modified according [MISA])
		SYMBuff = PM8_S_U8(SYMBuff, 0x01);
		memWriteU8NoAtomic(MEMtPM8(SYMBuff), mode);
	}
}

/*******************************************************************************
 * Function name: SymCipherDofinal
 * Description: SYM cipher dofinal algorithm.
 * Input Parameters:   (precondition)
 *		PM8 inBuff			 address of input buffer.
 *		u16 inLen				 length of input buffer.
 *		PM8 outBuff	     address of out buffer.
 *      PM8 keyBuff		 input key buffer.
 *      u16 keyBitLen	         bit length of SYM key.
 *		PM8 ICVBuff	     initial const vector used in CBC mode.
 *		PM8 SYMBuff		 the parameters of the algorithm (algType |  mode   | tmpData).
 *      u8 algType               the byte in the above.
 *      u8 mode                  the byte in the above.
 *      u8 sbcState			     processing state.
 * Output Parameters:  (postcondition)
 *		padded byte number if decryption.
 *      WRONG_PADDING if depadding wrong.
 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_DECRYPT or MODE_ENCRYPT
 *       0000 0x00 --- CBC or ECB
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
u16 SymCipherDofinal(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg)
{
	u32					X_A		symKeyBuffer[SYM_EXT_KEY_BYTE_LEN >> 2];
	u1					B_A 	isEncrypt;
	u1 					B_A 	paddingFlag; // the paddingFlag is used for ISO9797m1 only.
	u8 					D_A 	remainLen;   //wzq : locate in I_A will wrong
	u16 				D_A 	tmpLen;
	u8 					X_A 	tmpDataIn[SYM_MAX_BYTES_ONE_STEP];
	u8 					X_A 	tmpDataOut[SYM_MAX_BYTES_ONE_STEP];
	u8 					X_A 	tmpICV[SYM_BLOCKSIZE];

	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	Sym_API_Param_t		X_A		myAPIParam;
	
	PATCH_BOOT_SYM_API_013();
	
	ecpMem_Set(tmpDataOut, 0, SYM_MAX_BYTES_ONE_STEP);
	
	isEncrypt = (MODE_ENCRYPT == U8_AND_U8(mode, MODE_EN_OR_DE_MASK));

	SymAPIParamsInit(&myAPIParam, symAlg, algType, CIPHER_MODE);
	remainLen = 0;
	if (sbcState == SBC_UPDATE)
	{
		remainLen = U8_SHR_U8(mode, 4);
	}

	if ((isEncrypt == false) && (inLen == 0))
	{
		if(remainLen == 0)
		{
			memCopyToRam(tmpDataOut, SYMBuff, myAPIParam.blockSize);
	//		ecpMem_Copy(tmpDataOut, SYMBuff, myAPIParam.blockSize);
			tmpLen = SYMDepadDispatcher(tmpDataOut, myAPIParam.blockSize, algType, symAlg);

			if (tmpLen == WRONG_PADDING)
			{
				return WRONG_PADDING;
			}
			return U16_S_U16(myAPIParam.blockSize, tmpLen);
		}
	}
	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, SENSITIVE_DATA_MASK, symAlg);
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);

	SymCryptParamsInit(&myCryptParam, ICVBuff, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), tmpDataIn, tmpDataOut, mode);
	myCryptParam.noOfBlocks = myAPIParam.maxBlocksOneStep;
	myCryptParam.inputLength = SYM_MAX_BYTES_ONE_STEP;

	if (0 != remainLen)
	{
		memCopyToRam(tmpDataIn, SYMBuff, remainLen);
//			ecpMem_Copy(tmpDataIn, SYMBuff, remainLen);
		inLen = U16_A_U8(inLen, remainLen);
	}

	paddingFlag = false;

	if ((0x00 == inLen) && (SBC_UPDATE != sbcState))
	{
		if (SYM_MODE_ISO9797_M1 == myAPIParam.option)
		{
			paddingFlag = true; // the total length input is zero, just padding with SYM_BLOCKSIZE '0's.
		}
	}

	while (inLen >= SYM_MAX_BYTES_ONE_STEP)
	{
		tmpLen = U16_S_U8(SYM_MAX_BYTES_ONE_STEP, remainLen);
		memCopyToRam(&tmpDataIn[remainLen], inBuff, tmpLen);
		remainLen = 0;

		if (false == isEncrypt) // store ICV for decryption
		{
			ecpMem_Copy(tmpICV, &tmpDataIn[myAPIParam.ICVOffsetInOneStep], myAPIParam.blockSize);
		}

		SymCryptDispatcher(&myCryptParam, symAlg);

		if ( SYM_CBC_MODE == U8_AND_U8(mode, SYM_ECB_OR_CBC_MASK) ) // if CBC, updata ICV
		{
			if (false == isEncrypt)
			{
				ecpMem_Copy(ICVBuff, tmpICV, myAPIParam.blockSize);
			}
			else
			{
				ecpMem_Copy(ICVBuff, &tmpDataOut[myAPIParam.ICVOffsetInOneStep], myAPIParam.blockSize);
			}
		}

		memCopyNoAtomic(outBuff, tmpDataOut, SYM_MAX_BYTES_ONE_STEP);
		inLen   = U16_S_U16(inLen, SYM_MAX_BYTES_ONE_STEP);
		inBuff = PM8_A_U16(inBuff, tmpLen);
		outBuff = PM8_A_U16(outBuff, SYM_MAX_BYTES_ONE_STEP);
	}

	if (false == isEncrypt)
	{
		if (inLen != 0)
		{
			memCopyToRam(&tmpDataIn[remainLen], inBuff, inLen);
			myCryptParam.noOfBlocks = U16_SHR_U8(U16_A_U16(inLen, myAPIParam.blockSizeMASK), myAPIParam.SYMShiftMASK);
			myCryptParam.inputLength = inLen;

			SymCryptDispatcher(&myCryptParam, symAlg);

			tmpLen = SYMDepadDispatcher(tmpDataOut, U16t8(inLen), algType, symAlg);

			if (tmpLen == WRONG_PADDING)
			{
				return WRONG_PADDING;
			}

			remainLen = U16t8(U16_S_U16(inLen, tmpLen));  //(lzm modified according [MISA])
			memCopyNoAtomic(outBuff, tmpDataOut, tmpLen);
		}
		else
		{
			tmpLen = SYMDepadDispatcher(tmpDataOut, SYM_MAX_BYTES_ONE_STEP, algType, symAlg);

			if (tmpLen == WRONG_PADDING)
			{
				return WRONG_PADDING;
			}

			remainLen = U16_S_U16(SYM_MAX_BYTES_ONE_STEP, tmpLen); // length of the padding bytes
			outBuff = PM8_S_U8(outBuff, remainLen);
			memCopyNoAtomic(outBuff, &tmpDataOut[tmpLen], remainLen);
		}
	}
	else
	{
		memCopyToRam(&tmpDataIn[remainLen], inBuff, U16_S_U8(inLen, remainLen));

		if (((inLen == 0) && (SYM_MODE_NOPAD == myAPIParam.option)) == false)
		{
			if (paddingFlag)
			{
				// using the most significant bit in one byte to indicate that the tmpData should be padded with zeros,
				// when the algorithm is ISO9797_M1.
				inLen = U16_OR_U16(inLen, SYM_ISO9797_M1_PAD_MASK);
			}

			tmpLen = SYMPadDispatcher(tmpDataIn, inLen, U8_OR_U8(algType, CIPHER_MODE), symAlg);
			myCryptParam.noOfBlocks = U16_SHR_U8(U16_A_U16(tmpLen, myAPIParam.blockSizeMASK), myAPIParam.SYMShiftMASK);
			myCryptParam.inputLength = tmpLen;

			if (0 != myCryptParam.noOfBlocks) // this interface could not accept zero length!
			{
				SymCryptDispatcher(&myCryptParam, symAlg);
			}

			memCopyNoAtomic(outBuff, tmpDataOut, tmpLen);
		}
		return SBC_ENCRYPT_OK;
	}

	return remainLen;
}

/*******************************************************************************
 * Function name: SymSignatureUpdate
 * Description: SYM signature update algorithm.
 * Input Parameters:   (precondition)
 *		PM8 inBuff			 address of input buffer.
 *		u16 inLen				 length of input buffer.
 *      PM8 keyBuff		 input key buffer.
 *      u16 keyBitLen	         bit length of SYM key.
 *		PM8 ICVBuff	     initial const vector used in CBC mode.
 *		PM8 SYMBuff		 the parameters of the algorithm (algType |  mode   | tmpData).
 *      u8 mode                  the value in the above byte.
 *      u8 algType               algorithm type.
 *      u8 sbcState			     processing state.
 * Output Parameters:  (postcondition)
 *		none.
 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_SIGN or MODE_VERIFY
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
void SymSignatureUpdate(PM8 inBuff, u16 inLen, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg) SMALL_MODE
{
	u32					X_A		symKeyBuffer[SYM_EXT_KEY_BYTE_LEN >> 2];
	u16 				D_A 	tmpLen;
	u8 					D_A 	remainLen;
	u8 					X_A 	tmpDataIn[SYM_MAX_BYTES_ONE_STEP];
	u8 					X_A 	tmpDataOut[SYM_MAX_BYTES_ONE_STEP];
	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	Sym_API_Param_t		X_A		myAPIParam;

	PATCH_BOOT_SYM_API_014();
	
	// initialize values for SYM operation
	SymAPIParamsInit(&myAPIParam, symAlg, algType, SIGNATURE_MODE);

	
	if (SYM_MODE_ISO9797_1_ALG3 == myAPIParam.option)
	{
		SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, CBC_MAC_Encrypt, SENSITIVE_DATA_MASK_NOT_CHECK, symAlg);
		myLoadKeyParam.keyByteLen	= myAPIParam.firstKeyByteLen;
	}
	else
	{
		SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, CBC_MAC_Encrypt, SENSITIVE_DATA_MASK, symAlg);
	}
	
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);

	SymCryptParamsInit(&myCryptParam, ICVBuff, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), tmpDataIn, tmpDataOut, CBC_MAC_Encrypt);
	myCryptParam.noOfBlocks = myAPIParam.maxBlocksOneStep;

	remainLen = 0;
	if (SBC_UPDATE == sbcState)
	{
		remainLen = U8_SHR_U8(mode, 4);
		if (0 != remainLen)
		{
			memCopyToRam(tmpDataIn, SYMBuff, remainLen);
			//ecpMem_Copy(tmpDataIn, SYMBuff, remainLen);
			inLen = U16_A_U8(inLen, remainLen);
		}
	}

	while (inLen >= SYM_MAX_BYTES_ONE_STEP)
	{
		tmpLen = U16_S_U16(SYM_MAX_BYTES_ONE_STEP, remainLen);
		memCopyToRam(&tmpDataIn[remainLen], inBuff, tmpLen);

		SymCryptDispatcher(&myCryptParam, symAlg);

		remainLen = 0;
		inLen  = U16_S_U16(inLen, SYM_MAX_BYTES_ONE_STEP);
		inBuff = PM8_A_U16(inBuff, tmpLen);
		ecpMem_Copy(ICVBuff, tmpDataOut, myAPIParam.blockSize);
	}

	if (0 != remainLen)
	{
		memCopyToRam(&tmpDataIn[remainLen], inBuff, U16_S_U8(inLen, remainLen));
	}
	else
	{
		memCopyToRam(tmpDataIn, inBuff, inLen);
	}

	if (inLen >= myAPIParam.blockSize)
	{
		myCryptParam.noOfBlocks = U16_SHR_U8(inLen, myAPIParam.SYMShiftMASK);

		SymCryptDispatcher(&myCryptParam, symAlg);

		ecpMem_Copy(ICVBuff, tmpDataOut, myAPIParam.blockSize);
	}

	tmpLen = U16_AND_U16(inLen, myAPIParam.blockSizeMASK);  // remain bytes
	inLen  = U16_S_U16(inLen, tmpLen);// offset in tmpData
	memCopyNoAtomic(SYMBuff, &tmpDataIn[inLen], tmpLen);
	if (U8_SHR_U8(U8_AND_U8(mode, 0xF0), 4) != tmpLen)
	{
		mode    = U8_AND_U8(mode, 0x0F); // clear the value
		mode    = U8_OR_U8(mode, U8_SHL_U8(tmpLen, 4));//(lzm modified according [MISA])
		SYMBuff = PM8_S_U8(SYMBuff, 0x01);

		memWriteU8NoAtomic(SYMBuff, mode);	
	}
	incrementCodeFlowControlFlag();
}

/*******************************************************************************
 * Function name: SignatureDofinal
 * Description: SYM signature doFinal algorithm.
 * Input Parameters:   (precondition)
 *		PM8 inBuff			 address of input buffer.
 *		u16 inLen				 length of input buffer.
 *		PM8 outBuff	     address of out buffer.
 *      PM8 keyBuff		 input key buffer.
 *      u16 keyBitLen	         bit length of SYM key.
 *		PM8 ICVBuff	     initial const vector used in CBC mode.
 *		PM8 SYMBuff		 the parameters of the algorithm (algType |  mode   | tmpData).
 *      u8 algType               the byte in the above.
 *      u8 mode                  the byte in the above.
 *      u8 sbcState			     processing state.
 *      u8 macLen                the length of MAC.
 * Output Parameters:  (postcondition)
 *		none.
 * Output Parameters:  (postcondition)
 *		verify reslut:
 *      0x5A, if verify succesfully, 0xA5 if verify failed.
 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_SIGN or MODE_VERIFY
 *       0000 x000 --- indicate whether this interface is used by sign or verify
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
u8 SymSignatureDofinal(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 macLen, u8 symAlg) SMALL_MODE
{
	u32					X_A		symKeyBuffer[SYM_EXT_KEY_BYTE_LEN >> 2];
	u1					B_A 	paddingFlag; // the paddingFlag is used for ISO9797m1 only;
	u8 					D_A 	remainLen;
	u16 				D_A 	tmpLen;
	u8 					X_A 	tmpDataIn[SYM_MAX_BYTES_ONE_STEP];
	u8 					X_A 	tmpDataOut[SYM_MAX_BYTES_ONE_STEP];

	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	Sym_API_Param_t		X_A		myAPIParam;

	PATCH_BOOT_SYM_API_015();
	
	SC_codeFlowRamCnterInit();
	
	// initialize values for SYM operation
	SymAPIParamsInit(&myAPIParam, symAlg, algType, SIGNATURE_MODE);

	if (SYM_MODE_ISO9797_1_ALG3 == myAPIParam.option)
	{	
		SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, CBC_MAC_Encrypt, SENSITIVE_DATA_MASK_NOT_CHECK, symAlg);
		myLoadKeyParam.keyByteLen	= myAPIParam.firstKeyByteLen;
	}
	else
	{
		SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, CBC_MAC_Encrypt, SENSITIVE_DATA_MASK, symAlg);
	}
	

	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);

	SC_codeFlowRamSetCnter(SEC_STEP_COUNT_1);

	SymCryptParamsInit(&myCryptParam, ICVBuff, symKeyBuffer, U8t16(myLoadKeyParam.keyByteLen), tmpDataIn, tmpDataOut, CBC_MAC_Encrypt);//修改处
	myCryptParam.noOfBlocks = myAPIParam.maxBlocksOneStep;

	// inorder to do with NO_PAD_ALG verify
	ecpMem_Copy(tmpDataOut, ICVBuff, myAPIParam.blockSize);

	remainLen = 0;
	if (SBC_UPDATE == sbcState)
	{
		remainLen = U8_SHR_U8(mode, 4);
		if (0 != remainLen)
		{
			memCopyToRam(tmpDataIn, SYMBuff, remainLen);
			inLen = U16_A_U8(inLen, remainLen);
		}
	}

	paddingFlag = false;
	if ((0x00 == inLen) && (SBC_UPDATE != sbcState))
	{
		if (SYM_MODE_ISO9797_M1 == myAPIParam.option)
		{
			paddingFlag = true; // the total length input is zero, just padding with SYM_BLOCKSIZE '0's.
		}
	}

	while (inLen >= SYM_MAX_BYTES_ONE_STEP)
	{
		tmpLen = U16_S_U8(SYM_MAX_BYTES_ONE_STEP, remainLen);
		memCopyToRam(&tmpDataIn[remainLen], inBuff, tmpLen);
		SymCryptDispatcher(&myCryptParam, symAlg);

		remainLen = 0;
		inLen     = U16_S_U16(inLen, SYM_MAX_BYTES_ONE_STEP);
		inBuff = PM8_A_U16(inBuff, tmpLen);
		ecpMem_Copy(ICVBuff, tmpDataOut, myAPIParam.blockSize);
	}

	memCopyToRam(&tmpDataIn[remainLen], inBuff, U16_S_U8(inLen, remainLen));

	if (paddingFlag)
	{
		// using the most significant bit in one byte to indicate that the tmpData should be padded with zeros,
		// when the algorithm is ISO9797_M1.
		inLen  = U16_OR_U16(inLen, SYM_ISO9797_M1_PAD_MASK);
	}
	tmpLen = SYMPadDispatcher(tmpDataIn, inLen, U8_OR_U8(algType, SIGNATURE_MODE), symAlg);

	if(SYM_MODE_ISO9797_1_ALG3 == myAPIParam.option)
	{
		 // keep last block!
		if(0x00 == tmpLen)// modify by jiangxiaohui@20220628 ISO9797_1_ALG3 算法会存在dofina 长度0的情况。需要特殊处理
		{
			ecpMem_Copy(tmpDataIn, ICVBuff, myAPIParam.blockSize);
			ecpMem_Set(ICVBuff, 0, myAPIParam.blockSize);
			myCryptParam.noOfBlocks 	= 1;
			myCryptParam.option     	= U8_OR_U8(MODE_DECRYPT, SYM_ECB_MODE);

			SymCryptDispatcher(&myCryptParam, symAlg);
			ecpMem_Copy(tmpDataIn, tmpDataOut, myAPIParam.blockSize);
		}
		else
		{
			tmpLen = U16_S_U8(tmpLen, myAPIParam.blockSize);
		}
	}

	myCryptParam.noOfBlocks = U16_SHR_U8(tmpLen, myAPIParam.SYMShiftMASK);
	if (0 != myCryptParam.noOfBlocks) // this interface could not accept zero length!
	{
		SymCryptDispatcher(&myCryptParam, symAlg);
	}
	SC_codeFlowRamSetCnter(SEC_STEP_COUNT_2);

	if (SYM_MODE_ISO9797_1_ALG3 == myAPIParam.option)
	{
		// do with last block
//		decrementCodeFlowControlFlag();
//		decrementCodeFlowControlFlag();
		// initialize values for SYM operation
//		SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, LENGTH_3DES_2KEY, CBC_MAC_Encrypt, SENSITIVE_DATA_MASK);
		
		myLoadKeyParam.keyByteLen	= myAPIParam.secondKeyByteLen;
		DealSensitiveData(&myLoadKeyParam, SENSITIVE_DATA_MASK, symAlg);
		
		SymKeyInitDispatcher(&myLoadKeyParam, symAlg);
		decrementCodeFlowControlFlag();
		SC_codeFlowRamSetCnter(SEC_STEP_COUNT_3);

		if(0 != tmpLen)//if tmpLen == 0,don't do SymCryptDispatcher;
		{
			ecpMem_Copy(ICVBuff, tmpDataOut, myAPIParam.blockSize);
		}
		myCryptParam.pInput 		= &tmpDataIn[tmpLen];
		myCryptParam.keyByteLen 	= myAPIParam.secondKeyByteLen;
		myCryptParam.noOfBlocks 	= 1;
		myCryptParam.option     	= U8_OR_U8(MODE_ENCRYPT, SYM_CBC_MAC_MODE);

		SymCryptDispatcher(&myCryptParam, symAlg);
		SC_codeFlowRamCnterCheck(SEC_STEP_COUNT_3);
	}
	
	SymAPIParamsCheck(&myAPIParam);
	
	if (SYM_MODE_ISO9797_1_ALG3 == myAPIParam.option)
	{
		SC_codeFlowRamCnterCheck(SEC_STEP_COUNT_3);
	}

	if (U8_AND_U8(symAlg, SBC_VERIFY))
	{
		return SC_memCompareEqualWithDFA(MEMtPM8(outBuff), MEMtPM8(tmpDataOut), macLen);
	}
	else
	{
		SC_MemCopyNoAtomicAPI(outBuff, tmpDataOut, macLen);
		incrementCodeFlowControlFlag();
	}
	return 0;
}
#endif

#if WITH_AEAD
void AeadCryptParamsInit(Sym_AEADCrypt_Param_t * myAEADCryptParam, PM8 nonceBuf, u16 nonceLen, PM8 aadBuf, u16 adataLen, PM8 aeadGiv, PM8 aeadCiv, PM8 baseEctr, PM8 tag, u16 messageLen, u16 tagSize)
{
	myAEADCryptParam->pNonceBuf         	= nonceBuf;// ECB will ignore this value
	myAEADCryptParam->pAdataBuf      		= aadBuf;
	myAEADCryptParam->pAeadGiv     			= aeadGiv;// pOutput = pInput is possible for ECB en-/decryption, CBC en-/decryption and CBC MAC.
	myAEADCryptParam->pAeadCiv     			= aeadCiv;// pOutput = pInput is possible for ECB en-/decryption, CBC en-/decryption and CBC MAC.
	myAEADCryptParam->pBaseEctr     		= baseEctr;// pOutput = pInput is possible for ECB en-/decryption, CBC en-/decryption and CBC MAC.
	myAEADCryptParam->pTag     				= tag;// pOutput = pInput is possible for ECB en-/decryption, CBC en-/decryption and CBC MAC.
	myAEADCryptParam->nonceLen	 			= nonceLen;
	myAEADCryptParam->adataLen      		= adataLen; //default use ENCRYPT_ECB_MODE
	myAEADCryptParam->messageLen	 		= messageLen;
	myAEADCryptParam->tagLen      			= tagSize; //default use ENCRYPT_ECB_MODE
	return;
}


/*******************************************************************************
 * Function name: SymCipherDofinal
 * Description: SYM cipher dofinal algorithm.
 * Input Parameters:   (precondition)
 *		PM8 nonceBuf			 address of input buffer.
 *		u16 nonceLen				 length of input buffer.
 *		PM8 aadBuf	     address of out buffer.
 *      PM8 adataLen		 input key buffer.
 * Output Parameters:  (postcondition)

 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_DECRYPT or MODE_ENCRYPT
 *       0000 0x00 --- CBC or ECB
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
u8 SymCipherInit(PM8 nonceBuf, u16 nonceLen, PM8 aadBuf, u16 adataLen, u16 messageLen, u16 tagSize, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 symAlg)
{
	u32					X_A		symKeyBuffer[SYM_EXT_KEY_BYTE_LEN >> 2];
	u8 					X_A 	tmpICV[SYM_BLOCKSIZE * 3];

	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_API_Param_t		X_A		myAPIParam;
	Sym_AEADCrypt_Param_t X_A 	myAEADCryptParam;

	SymAPIParamsInit(&myAPIParam, symAlg, algType, CIPHER_MODE);

	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, SENSITIVE_DATA_MASK, symAlg);
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);


	memCopyToRam(tmpICV, ICVBuff, myAPIParam.blockSize * 3);
	AeadCryptParamsInit(&myAEADCryptParam, nonceBuf, nonceLen, aadBuf, adataLen, tmpICV, MEMtPX8(&tmpICV[myAPIParam.blockSize]),  MEMtPX8(&tmpICV[myAPIParam.blockSize * 2]),  NULL, messageLen, tagSize);
	Sym_AEAD_starts(&myLoadKeyParam, &myAEADCryptParam);

	memCopyNoAtomic(ICVBuff, tmpICV, myAPIParam.blockSize * 3);

	return SBC_ENCRYPT_OK;
}


/*******************************************************************************
 * Function name: SymCipherDofinal
 * Description: SYM cipher dofinal algorithm.
 * Input Parameters:   (precondition)
 *		PM8 nonceBuf			 address of input buffer.
 *		u16 nonceLen				 length of input buffer.
 *		PM8 aadBuf	     address of out buffer.
 *      PM8 adataLen		 input key buffer.
 * Output Parameters:  (postcondition)

 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_DECRYPT or MODE_ENCRYPT
 *       0000 0x00 --- CBC or ECB
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
u8 SymCipherTag(PM8 tagBuf, u16 tagSize, u16 adataLen, u16 nonceLen, u16 messageLen, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 symAlg)
{
	u32					X_A		symKeyBuffer[SYM_EXT_KEY_BYTE_LEN >> 2];
	u8 					X_A 	tmpICV[SYM_BLOCKSIZE * 3];
	u8 					X_A 	tmpTag[SYM_BLOCKSIZE];

	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_API_Param_t		X_A		myAPIParam;
	Sym_AEADCrypt_Param_t X_A 	myAEADCryptParam;

	SymAPIParamsInit(&myAPIParam, symAlg, algType, CIPHER_MODE);

	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, SENSITIVE_DATA_MASK, symAlg);
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);


	memCopyToRam(tmpICV, ICVBuff, myAPIParam.blockSize * 3);
	AeadCryptParamsInit(&myAEADCryptParam, NULL, nonceLen, NULL, adataLen, tmpICV, MEMtPX8(&tmpICV[myAPIParam.blockSize]),  MEMtPX8(&tmpICV[myAPIParam.blockSize * 2]),  tmpTag, messageLen, tagSize);
	mbedtls_AEAD_finish(&myLoadKeyParam, &myAEADCryptParam);
	if (U8_AND_U8(symAlg, SBC_VERIFY))
	{
		return SC_memCompareEqualWithDFA(MEMtPM8(tagBuf), MEMtPM8(tmpTag), tagSize);
	}
	else
	{
		SC_MemCopyNoAtomicAPI(tagBuf, tmpTag, tagSize);
		incrementCodeFlowControlFlag();
	}
	return tagSize;
}
#endif /*WITH_AEAD*/
/*******************************************************************************
 * Function name: SignatureDofinal
 * Description: SYM signature doFinal algorithm.
 * Input Parameters:   (precondition)
 *		PM8 inBuff			 address of input buffer.
 *		u16 inLen				 length of input buffer.
 *		PM8 outBuff	     address of out buffer.
 *      PM8 keyBuff		 input key buffer.
 *      u16 keyBitLen	         bit length of SYM key.
 *		PM8 ICVBuff	     initial const vector used in CBC mode.
 *		PM8 SYMBuff		 the parameters of the algorithm (algType |  mode   | tmpData).
 *      u8 algType               the byte in the above.
 *      u8 mode                  the byte in the above.
 *      u8 sbcState			     processing state.
 *      u8 macLen                the length of MAC.
 * Output Parameters:  (postcondition)
 *		none.
 * Output Parameters:  (postcondition)
 *		verify reslut:
 *      0x5A, if verify succesfully, 0xA5 if verify failed.
 * NOTE:
 *
 * The data stored in this buffer is as follows:
 * --------------------------------
 * | algType |  mode   | tmpData  |
 * --------------------------------
 * | 1 byte  | 1 byte  | 7 bytes  |
 * --------------------------------
 * mode: 0000 00xx --- MODE_SIGN or MODE_VERIFY
 *       0000 x000 --- indicate whether this interface is used by sign or verify
 *       0xxx 0000 --- length of data in tmpData
 *
 ********************************************************************************/
#if VM_WITH_SYM
u8 SymCMACDofinal(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 macLen, u8 symAlg) SMALL_MODE
{
	u8		D_A		tempBuf[SYM_BLOCKSIZE];
	u8		D_A		subKeyIcv[SYM_BLOCKSIZE];
	u8		D_A		remainLen;

	remainLen = 0;
	if (SBC_UPDATE == sbcState)
	{
		remainLen = U8_SHR_U8(mode, 4);
	}
	ecpMem_Set(subKeyIcv, 0x00, SYM_BLOCKSIZE);

	symCmacAlgorithmCal(SYMBuff, remainLen, inBuff, inLen, tempBuf, keyBuff, keyBitLen, subKeyIcv, CBC_MAC_Encrypt, symAlg);
	if (U8_AND_U8(symAlg, SBC_VERIFY))
	{
		return SC_memCompareEqualWithDFA(MEMtPM8(outBuff), MEMtPM8(tempBuf), macLen);
	}
	else
	{
		SC_MemCopyNoAtomicAPI(outBuff, tempBuf, macLen);
	}
	return 0;
}
#endif
#endif /* WITH_SYM */

