/*
 * Crypto_API.c
 *
 *  Created on: 2019年12月13日
 *      Author: jiangxiaohui
 */

#include "Crypto_Constant.h"
#include "Crypto_common_API.h"
#include "SysConfigTarget.h"
#include "SysConfigHalRam.h"
#include "SecOperationHal.h"
#include "RSA_API.h"
#include "RNG_API.h"
#include "chipHead.h"
#include "SecureCrypto.h"
#include "memory.h"
#include "MemGetReuseRam.h"
#include "MemOperation.h"
#include "MemBackUp.h"
#include "Crypto_API.h"
#include "Crypto_SoftAES.h"
#include "EllipseCurve_API.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#if !RUN_IN_SIMULATOR
#endif

#if WITH_RSA

#if RSA_CRT_E_SAVE
void RSA_CalE(PM8 keyBuff, u16 primeLen)
{
	PM8					pBuff;
	PM8					qBuff;
	PM8					DpBuff;
	PM8					DqBuff;
	PM8					EBuff;
	PX8					tempEBuff;
	u16		X_A			wModulusNLength;

	wModulusNLength		= primeLen << 1;

	pBuff				= MEMtPX8(MemGetReuseRam_GetTempArrFromTopBase((MAX_RSA_P_BYTE_LEN + MAX_RSA_P_BYTE_LEN + MAX_RSA_E_BYTE_LEN)));
	qBuff				= PX8_A_U16(pBuff, MAX_RSA_P_BYTE_LEN);
	tempEBuff			= pBuff;

	DpBuff      		= keyBuff + (primeLen * 2);
	DqBuff      		= keyBuff + (primeLen * 3);

	EBuff				= PM8_A_U16(keyBuff, (primeLen * 5) + 2);
	EBuff				= PM8_A_U16(EBuff, (E_INDEX  * wModulusNLength));

	SC_checkReturnCode(SC_PKIgetKey(pBuff, pBuff, keyBuff, 0x00, primeLen, TYPE_RSA_CRT_PRIVATE, primeLen, (SC_PKI_IN_ENCRY_TEXT | SC_PKI_IN_DEAL_TEXT)));
	SC_checkReturnCode(SC_PKIgetKey(qBuff, qBuff, keyBuff, primeLen, primeLen, TYPE_RSA_CRT_PRIVATE, primeLen, (SC_PKI_IN_ENCRY_TEXT | SC_PKI_IN_DEAL_TEXT)));
	memRefresh();

	if(RSA_CRT_CALE_OK != RSA_CalE_Hal(pBuff, qBuff, DpBuff, DqBuff, tempEBuff, primeLen))
	{
		ecpMem_Set(tempEBuff, 0x00, MAX_RSA_E_BYTE_LEN);
	}
	memCopyNoAtomic(EBuff, tempEBuff, MAX_RSA_E_BYTE_LEN);
	memRefresh();
}
#endif
u16 calcModuleN(PM8 keyBuff, u16 wPrimeLength, u8 mode)
{
	PX8		X_A		tempParmPuffer;
	PX8		X_A		tempParmQuffer;
	u16				wModulusNLength;
    u16     D_A     OneByteModleValue;
	
	tempParmPuffer		= MEMtPX8(MemGetReuseRam_GetTempArrFromTopBase(MAX_RSA_MUD_BYTE_LEN));
	tempParmQuffer		= PX8_A_U16(tempParmPuffer, MAX_RSA_P_BYTE_LEN);
	
	SC_checkReturnCode(SC_PKIgetKey(tempParmPuffer, tempParmPuffer, keyBuff, 0x00, wPrimeLength, TYPE_RSA_CRT_PRIVATE, wPrimeLength, (SC_PKI_IN_ENCRY_TEXT | SC_PKI_IN_DEAL_TEXT)));
	SC_checkReturnCode(SC_PKIgetKey(tempParmQuffer, tempParmQuffer, keyBuff, wPrimeLength, wPrimeLength, TYPE_RSA_CRT_PRIVATE, wPrimeLength, (SC_PKI_IN_ENCRY_TEXT | SC_PKI_IN_DEAL_TEXT)));

	if(TRUE == mode)
	{
		wModulusNLength = BigData_Mul((keyBuff + (5 * wPrimeLength + 2)), tempParmPuffer, wPrimeLength, tempParmQuffer, wPrimeLength, TRUE);
	}
	else
	{
		OneByteModleValue = U8_M_U8(tempParmPuffer[0],tempParmQuffer[0]);
		if((OneByteModleValue & 0xFFFF) > 0x00FF)  //???????
		{
		   wModulusNLength = U16_SHL_U8(wPrimeLength , 1);
		}
		else if(((OneByteModleValue & 0xFFFF) < 0x00E1) && (0x00 != tempParmPuffer[0]) && (0x00 != tempParmQuffer[0]))//??????,??P,Q???????0x00
		{
		   wModulusNLength = U16_S_U8((U16_SHL_U8(wPrimeLength , 1)),1);
		}
		else
		{
		   wModulusNLength = BigData_Mul(NULL, tempParmPuffer, wPrimeLength, tempParmQuffer, wPrimeLength ,FALSE);
		}		
	}
	return wModulusNLength;
	
}

void SetKeyPinv(PM8 keyBuff, u16 realKeyLen)
{
	u8 keytempbuff[MAX_RSA_MUD_BYTE_LEN];
	
	u8 transactionStarted;
	E2P_stream_flush();
	if(RSA_CRT_PINV == RSA_CRTKeyPinv_Cal(keyBuff, MEMtPM8(&keyBuff[realKeyLen]), MEMtPM8(&keyBuff[realKeyLen * 2]), MEMtPM8(&keyBuff[realKeyLen * 3]), MEMtPM8(&keyBuff[realKeyLen * 4]), realKeyLen))
	{
		
		transactionStarted = 0;
		if (!isInProgress())
		{
			begin_transaction(SINGLE_APDU_PROCESS);
			transactionStarted = 1;
		}
	
		// save p
		memCopyToRam(keytempbuff, keyBuff, realKeyLen);
		// copy q to p
		memCopyAtomic(keyBuff, PM8_A_U16(keyBuff, realKeyLen), realKeyLen);
		// copy p to q
		memCopyAtomic(PM8_A_U16(keyBuff, realKeyLen), keytempbuff, realKeyLen);
		
		// save dp
		memCopyToRam(keytempbuff, PM8_A_U16(keyBuff, realKeyLen * 2), realKeyLen);
		// copy dq to dp
		memCopyAtomic(PM8_A_U16(keyBuff, realKeyLen * 2), PM8_A_U16(keyBuff, realKeyLen * 3), realKeyLen);
		// copy dp to dq
		memCopyAtomic(PM8_A_U16(keyBuff, realKeyLen * 3), keytempbuff, realKeyLen);
		
		if (transactionStarted)
		{
			commit_transaction();
		}/*end if*/
	}
	
}
#if	(RSA_CRT_MODULAR_SAVE || RSA_CRT_E_SAVE)

#if	RSA_CRT_MODULAR_SAVE
void RSA_storeKey_CRTN(PM8 pbModulusN, u16 wModulusNLength, u8 mode)
{
	if(RSA_CRT_ME_KEY_GEN == mode)
	{
		g_RSAKeyGenCRTNEStore.pbCRTModulusN	= pbModulusN;
	}
	else
	{
		g_RSAKeyStore.pbModulusN 			= pbModulusN;
	}
}
#endif	//RSA_CRT_MODULAR_SAVE

#if	RSA_CRT_E_SAVE
void RSA_storeKey_CRTE(PM8 pbExp, u16 pbExpLen, u8 mode)
{
	if(RSA_CRT_ME_KEY_GEN == mode)
	{
		g_RSAKeyGenCRTNEStore.pbCRTExp 		= pbExp;
	}
	else
	{
		g_RSAKeyStore.pbExp 				= pbExp;
		g_RSAKeyStore.wExpLength				= pbExpLen;
	}
}
#endif	//RSA_CRT_E_SAVE

void RSA_storeKey_CRTNE(PM8 pbModulusN, u16 wModulusNLength, u8 mode)
{
	RSA_storeKey_CRTN(pbModulusN, wModulusNLength, mode);

	RSA_storeKey_CRTE(PM8_A_U16(pbModulusN, (E_INDEX  * wModulusNLength )), 4, mode);
}
#endif /*RSA_CRT_MODULAR_SAVE*/

#endif

void ShlGetSubkey(PX8 outPutData,u16 symBlockSize)
{
	u8  X_A     count;
	u8  X_A 	Rb128[0x10] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x87};
	u8  X_A 	Rb64[0x8] = {0,0,0,0,0,0,0,0x1B};
	PX8 X_A     RbBuf;
	switch(symBlockSize)
	{
	    case 8:
		    RbBuf = MEMtPX8(Rb64);
		    break;
	    case 16:
		    RbBuf = MEMtPX8(Rb128);
		    break;
	    default:
		    break;
	}
	if(U8_AND_U8(outPutData[0] , 0x80))
	{
		for(count = 0; count < symBlockSize; U8_INC(count))
		{
			outPutData[count]=U16_SHL_U8(outPutData[count],1);

			if(count != (symBlockSize - 1))
			{
				outPutData[count] = U8_OR_U8(outPutData[count],U8_SHR_U8(outPutData[count+1],7));
			}

			outPutData[count] = U8_XOR_U8(outPutData[count],RbBuf[count]);
		}
	}
	else
	{
		for(count = 0; count < symBlockSize; U8_INC(count))
		{
			outPutData[count]=U16_SHL_U8(outPutData[count],1);

			if(count != (symBlockSize - 1))
			{
				outPutData[count] = U8_OR_U8(outPutData[count],U8_SHR_U8(outPutData[count+1],7));
			}
		}

	}

}


#if (WITH_AES || WITH_AES_PRIVATED)

#define    AES_CMAC_BLOCK_SIZE        U32t8(0x80)
#define    AES_CMAC_BLOCK_MASK        U32t8(0x7F)
#define    AES_CMAC_BLOCK_SHIFT       7
#define    AES_CMAC_RESULT_SIZE       U32t8(0x8)
//#define    AES_CMAC_LAST_BLOCK_TAG    U32t8(0xA5)
#endif

#if WITH_SM4
#define    SM4_CMAC_BLOCK_SIZE        U32t8(0x80)
#endif

#define    SYM_CMAC_BLOCK_SIZE        U32t8(0x80)
#define    SYM_CMAC_BLOCK_MASK        U32t8(0x7F)
#define    SYM_CMAC_BLOCK_SHIFT       7
#define    SYM_CMAC_LAST_BLOCK_TAG    U32t8(0xA5)
#define    SYM_CMAC_NULL_BLOCK_TAG    U32t8(0x33)

/*
 * function name : subKeyGenerate
 * function: Using the keyBuff you given,to calculate subkey:K1 and K2.

 * */
void subKeyGenerate(PM8  keyBuff,u16  keyBitLen,PX8  icv  ,PX8  subFirstKeyBuf,PX8  subSecondKeyBuf,u8  mode,u8  symAlg)
{
	u32					X_A		symKeyBuffer[U16_SHR_U8(SYM_EXT_KEY_BYTE_LEN, 2)]; // modify by xuxiaojun@20220812: when WITH_SOFT_AES opened, SYM_EXT_KEY_BYTE_LEN exceed 0xFF
//	u1					B_A 	isEncrypt;

	u8 					X_A 	inPutData[AES_BLOCKSIZE];
	u8                  X_A 	outPutData[AES_BLOCKSIZE];
	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
	u8					X_A		symBlockSize;

	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_DES
		case ALG_DES:
			symBlockSize = DES_BLOCKSIZE;
		break;
#endif //WITH_AES
#if WITH_AES
		case ALG_AES:
			symBlockSize = AES_BLOCKSIZE;
		break;
#endif //WITH_AES
#if WITH_SM4
		case ALG_SM4:
			symBlockSize = SM4_BLOCKSIZE;
		break;
#endif
		default:
			return;
	}

	ecpMem_Set(inPutData,0,symBlockSize);

//	if(MODE_ENCRYPT == U8_AND_U8(mode, MODE_EN_OR_DE_MASK))
//	{
//		isEncrypt	= true;
//	}
//	else
//	{
//		isEncrypt	= false;
//	}
	// initialize values for SYM operation
//	SC_SYMgetKey(MEMtPM8(symKeyBuffer), keyBuff, U16_SHR_U8(keyBitLen, 3));
//	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, isEncrypt, SENSITIVE_DATA_NOT_MASK, ALG_AES);
	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, SENSITIVE_DATA_NOT_MASK, symAlg);
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);
	SymCryptParamsInit(&myCryptParam, icv, symKeyBuffer, U16t8(U16_SHR_U8(keyBitLen, 3)), inPutData, outPutData, mode);
	myCryptParam.noOfBlocks   	= 1;
	SymCryptDispatcher(&myCryptParam, symAlg);

	ShlGetSubkey(MEMtPX8(outPutData),symBlockSize);//use AES result and KeyBitlen To get K1
	ecpMem_Copy(subFirstKeyBuf, MEMtPX8(outPutData), symBlockSize);
	ShlGetSubkey(MEMtPX8(outPutData),symBlockSize);//use K1 and KeyBitlen To get K2
	ecpMem_Copy(subSecondKeyBuf,MEMtPX8(outPutData), symBlockSize);
	return;

}


/*
 * function name: AesCmacAlgorithmCal
 * function: Using inBuff,inLen and keybuf,to calculate CMAC,it contains subkey generate and aes encrypto xor.
 * attention: parameter inBuff and outBuff shall be in Ram Address,or will be wrong.
 * */

void symCmacAlgorithmCal(PM8 inBuff1, u16 inLen1,PM8 inBuff2, u16 inLen2,PM8 outBuff, PM8 keyBuff, u16 keyBitLen,PM8 IcvBuf, u8 mode, u8 symAlg)
{
	u8    subFKeyBuf[SYM_BLOCKSIZE];
	u8    subSKeyBuf[SYM_BLOCKSIZE];
	u8    subKeyIcv[SYM_BLOCKSIZE];
	u8    inData[SYM_CMAC_BLOCK_SIZE];
	u8    outPutData[SYM_BLOCKSIZE];
	PM8   inPtr;
	u16   div128Count;
//	u8    remain128;
	u16   i;
	u16   inLength;
	u16   offset;
	u8    cmacBlockSize, macLen;

	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_DES
		case ALG_DES:
			cmacBlockSize = SYM_CMAC_BLOCK_SIZE;
			macLen = DES_MAC_LENGTH;
		break;
#endif //WITH_AES
#if WITH_AES
		case ALG_AES:
			cmacBlockSize = SYM_CMAC_BLOCK_SIZE;
			macLen = AES_MAC_LENGTH;
		break;
#endif //WITH_AES
#if WITH_SM4
		case ALG_SM4:
			cmacBlockSize = SYM_CMAC_BLOCK_SIZE;
			macLen = SM4_MAC_LENGTH;
		break;
#endif
		default:
			return;
	}

	ecpMem_Set(MEMtPX8(subKeyIcv),0,SYM_BLOCKSIZE);
	subKeyGenerate(keyBuff,keyBitLen,MEMtPX8(subKeyIcv),MEMtPX8(subFKeyBuf),MEMtPX8(subSKeyBuf), mode, symAlg);
	
	inLength = inLen1 + inLen2;

	div128Count = U16_A_U16(inLength , SYM_CMAC_BLOCK_MASK) >> SYM_CMAC_BLOCK_SHIFT;

//	div128Count = U16_D_U8(inLength,AES_CMAC_BLOCK_SIZE);
//	remain128   = U16_R_U8(inLength,AES_CMAC_BLOCK_SIZE);
//	if(remain128 != 0)
//	{
//		div128Count = U16_A_U16(div128Count , 1);
//	}
	inPtr = inBuff1;
	offset = 0;
	if(inLength == 0){
		CmacAlgorithmCal(MEMtPX8(inData), inLength, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg, SYM_CMAC_NULL_BLOCK_TAG);//calculate every 128 block bytes
	}
	else
	{
		if(inLength <= cmacBlockSize)//input data length <= 128,we will calculate one Step.
		{
	//		memCopyToRam(MEMtPX8(inData), inBuff1, inLen1);
	//		memCopyToRam(PX8_A_U16(MEMtPX8(inData),inLen1), inBuff2, inLen2);
	//		CmacAlgorithmCal(MEMtPX8(inData),inLength, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,AES_CMAC_LAST_BLOCK_TAG);
		}
		else//input data length > 128,we will calculate per every 128 bytes,every result(16 bytes) used as next calculate's ICV .
		{
			for(i = 0 ;i < U16_S_U8(div128Count,1) ;U16_INC(i))
			{
				if(inLen1 >= cmacBlockSize)
				{
					memCopyToRam(MEMtPX8(inData), inPtr, cmacBlockSize);
					inPtr 	+= cmacBlockSize;
					inLen1 	-= cmacBlockSize;
				}
				else if(inLen1 > 0)
				{
					memCopyToRam(MEMtPX8(inData), inPtr, inLen1);
					offset = U16_S_U16(cmacBlockSize,inLen1);
					memCopyToRam(PX8_A_U16(MEMtPX8(inData),inLen1), inBuff2, offset);
					inLen1	= 0;
				}
				else
				{
					memCopyToRam(MEMtPX8(inData), PM8_A_U16(inBuff2 , offset) , cmacBlockSize);
					offset = U16_A_U16(offset,cmacBlockSize);
				}
				CmacAlgorithmCal(MEMtPX8(inData),cmacBlockSize, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,U8_INV(SYM_CMAC_LAST_BLOCK_TAG));//calculate every 128 block bytes

				inLength -= cmacBlockSize;
			}
		}
		memCopyToRam(MEMtPX8(inData), inPtr, inLen1);
		memCopyToRam(PM8_A_U16(inData , inLen1), PM8_A_U16(inBuff2 , offset) , U16_S_U16(inLen2 , offset));
		CmacAlgorithmCal(MEMtPX8(inData), inLength, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,SYM_CMAC_LAST_BLOCK_TAG);//calaulate  last block (<= 128 bytes)
	}
	
//	memCopyNoAtomic(outBuff, outPutData, AES_CMAC_RESULT_SIZE);//规范中只需要前8个字节作为MAC，16字节的话会冲掉Lte的Buf的其他数据
	memCopyNoAtomic(outBuff, outPutData, macLen); //SCP03tDecryptAndCalMac中需要用16字节的结果作为下一次计算CMac的initMacValue
	return;
}


//#if WITH_SM4
//
//#define    SM4_CMAC_BLOCK_SIZE        U32t8(0x80)
//#define    SM4_CMAC_LAST_BLOCK_TAG    U32t8(0xB5)
//
//void SM4CmacAlgorithmCal(PM8 inBuff1, u16 inLen1,PM8 inBuff2, u16 inLen2,PM8 outBuff, PM8 keyBuff, u16 keyBitLen,PM8 IcvBuf, u8 mode, u8 symAlg)
//{
//	u8    subFKeyBuf[SM4_BLOCKSIZE];
//	u8    subSKeyBuf[SM4_BLOCKSIZE];
//	u8    subKeyIcv[SM4_BLOCKSIZE];
//	u8    inData[SM4_CMAC_BLOCK_SIZE];
//	u16   div128Count;
//	u8    remain128;
//	u16   i;
//	u8    outPutData[SM4_BLOCKSIZE];
//	u16   inLength;
//	u16   offset;
//
//	ecpMem_Set(MEMtPX8(subKeyIcv),0,SM4_BLOCKSIZE);
//	subKeyGenerate(keyBuff,keyBitLen,MEMtPX8(subKeyIcv),MEMtPX8(subFKeyBuf),MEMtPX8(subSKeyBuf), ECB_Encrypt, symAlg);
//
//	inLength = inLen1 + inLen2;
//	div128Count = U16_D_U8(inLength,SM4_CMAC_BLOCK_SIZE);
//	remain128   = U16_R_U8(inLength,SM4_CMAC_BLOCK_SIZE);
//	if(remain128 != 0)
//	{
//		div128Count = U16_A_U16(div128Count , 1);
//	}
//	if(inLength <= SM4_CMAC_BLOCK_SIZE)//input data length <= 128,we will calculate one Step.
//	{
//		memCopyToRam(MEMtPX8(inData), inBuff1, inLen1);
//		memCopyToRam(PX8_A_U16(MEMtPX8(inData),inLen1), inBuff2, inLen2);
//		CmacAlgorithmCal(MEMtPX8(inData),inLength, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,SM4_CMAC_LAST_BLOCK_TAG);
//	}
//	else//input data length > 128,we will calculate per every 128 bytes,every result(16 bytes) used as next calculate's ICV .
//	{
//		memCopyToRam(MEMtPX8(inData), inBuff1, inLen1);
//		offset = U16_S_U16(SM4_CMAC_BLOCK_SIZE,inLen1);
//		memCopyToRam(PX8_A_U16(MEMtPX8(inData),inLen1), inBuff2, offset);
//		CmacAlgorithmCal(MEMtPX8(inData),SM4_CMAC_BLOCK_SIZE, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,U8_INV(SM4_CMAC_LAST_BLOCK_TAG));//calculate the first 128 block bytes
//
//		for(i = 1 ;i < U16_S_U8(div128Count,1) ;U16_INC(i))
//		{
//			memCopyToRam(MEMtPX8(inData), PM8_A_U16(inBuff2 , offset) , SM4_CMAC_BLOCK_SIZE);
//			offset = U16_A_U16(offset,SM4_CMAC_BLOCK_SIZE);
//			CmacAlgorithmCal(MEMtPX8(inData),SM4_CMAC_BLOCK_SIZE, outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,U8_INV(SM4_CMAC_LAST_BLOCK_TAG));//calculate every 128 block bytes
//		}
//
//		memCopyToRam(MEMtPX8(inData), PM8_A_U16(inBuff2 , offset) , U16_S_U16(inLen2 , offset));
//		CmacAlgorithmCal(MEMtPX8(inData),U16_S_U16(inLen2 , offset), outPutData, keyBuff, keyBitLen,IcvBuf,subFKeyBuf,subSKeyBuf,mode,symAlg,SM4_CMAC_LAST_BLOCK_TAG);//calaulate  last block (<= 128 bytes)
//
//	}
//
//	memCopyNoAtomic(outBuff, outPutData, SM4_MAC_LENGTH); //SCP03tDecryptAndCalMac中需要用16字节的结果作为下一次计算CMac的initMacValue
//	return;
//}
//#endif

/*
 * function name:CmacAlgorithmCal
 * function: Using the subkey you given,to calculate CMAC return value.
 * attention: parameter inBuff and outBuff shall be in Ram Address,or will be wrong.
 * */
void CmacAlgorithmCal(PM8  inBuff,u16  inLen,PM8  outBuff, PX8  keyBuff, u16  keyBitLen,PX8  IcvBuf,PX8  subFirstKeyBuf,PX8  subSecondKeyBuf,u8  mode,u8  symAlg,u8  cmacLastBlock)
{
	u8    numberOfDivision;
	u8    numberOfRemainder;
	u8    i;
	u32		X_A		symKeyBuffer[U16_SHR_U8(SYM_EXT_KEY_BYTE_LEN, 2)]; // modify by xuxiaojun@20220812: when WITH_SOFT_AES opened, SYM_EXT_KEY_BYTE_LEN exceed 0xFF
//	u1		B_A 	isEncrypt;
//	u16         X_A 	totalLen;
	u8    symBlockSize;

	Sym_LoadKey_Param_t X_A 	myLoadKeyParam;
	Sym_Crypt_Param_t 	X_A 	myCryptParam;

	numberOfDivision = 0x8;
	//following if else step  giving ths correctly  last block
	switch(U8_AND_U8(symAlg, SBC_TPYE_MASK))
	{
#if WITH_DES
		case ALG_DES:
			symBlockSize = DES_BLOCKSIZE;
		break;
#endif //WITH_AES
#if WITH_AES
		case ALG_AES:
			symBlockSize = AES_BLOCKSIZE;
		break;
#endif //WITH_AES
#if WITH_SM4
		case ALG_SM4:
			symBlockSize = SM4_BLOCKSIZE;
		break;
#endif
		default:
			return;
	}

	if(cmacLastBlock == SYM_CMAC_LAST_BLOCK_TAG)
	{
		numberOfDivision  = U16t8(U16_D_U8(inLen,symBlockSize));
		numberOfRemainder = U16t8(U16_R_U8(inLen,symBlockSize));

		if(numberOfRemainder == 0x0)
		{
			for(i = 0;i < symBlockSize ;U8_INC(i))
			{
				inBuff[U16_S_U8(U32t16(U16_A_U8(inLen,i)), symBlockSize)] = U8_XOR_U8(inBuff[U16_S_U8(U32t16(U16_A_U8(inLen,i)), symBlockSize)],subFirstKeyBuf[i]);
			}
		}
		else
		{
			inBuff[inLen] = 0x80;
			ecpMem_Set(MEMtPM8(&inBuff[U16_A_U8(inLen,0x1)]),0x00,U8_S_U8(U8_S_U8(symBlockSize, 0x1),numberOfRemainder));

			for(i = 0;i < symBlockSize ;U8_INC(i))
			{
				inBuff[U16_A_U8(U32t16(U8_M_U16(symBlockSize,numberOfDivision)),i)] = U8_XOR_U8(inBuff[U16_A_U8(U32t16(U8_M_U16(symBlockSize,numberOfDivision)),i)],subSecondKeyBuf[i]);
			}
			numberOfDivision = U16t8(U8_A_U8(numberOfDivision,1));
		}
	}
	else if(cmacLastBlock == SYM_CMAC_NULL_BLOCK_TAG)
	{
		inBuff[0] = 0x80;
		ecpMem_Set(MEMtPM8(&inBuff[1]),0x00, U8_S_U8(symBlockSize, 0x1));

		for(i = 0;i < symBlockSize ;U8_INC(i))
		{
			inBuff[i] = U8_XOR_U8(inBuff[i],subSecondKeyBuf[i]);
		}
		numberOfDivision = 1;
	}

//	if(MODE_ENCRYPT == U8_AND_U8(mode, MODE_EN_OR_DE_MASK))
//	{
//		isEncrypt	= true;
//	}
//	else
//	{
//		isEncrypt 	= false;
//	}

	// initialize values for SYM operation
//	SC_SYMgetKey(MEMtPM8(symKeyBuffer), keyBuff, U16_SHR_U8(keyBitLen, 3));

//	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, isEncrypt, SENSITIVE_DATA_NOT_MASK, ALG_AES);
	SymKeyInitParamsInit(&myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, SENSITIVE_DATA_NOT_MASK, U8_AND_U8(symAlg, SBC_TPYE_MASK));
	SymKeyInitDispatcher(&myLoadKeyParam, symAlg);
	SymCryptParamsInit(&myCryptParam, IcvBuf, symKeyBuffer, U16t8(U16_SHR_U8(keyBitLen, 3)), inBuff, outBuff, mode);
	myCryptParam.noOfBlocks = U8t16(numberOfDivision);
	SymCryptDispatcher(&myCryptParam, symAlg);
}

#if  WITH_HASH_DOFINAL_SHA
void HASHDoFinal_SIM(PM8 inBuff, u16 inLen, PM8 outBuff,  PM8 initIvBuff)
{
	u32 		X_A			shaWorkTmp[U8_SHR_U8(HASH_WORKSPACE_SIZE, 2)];
	HASH_API_Param_t 		myAPIParam;
	HASH_Init_Param_t		myInitParam;
	u8	 		X_A 		tempLen;
	u8	 		X_A 		remainLength;
	HASHAPIParamsInit(&myAPIParam, ALG_SHA_1);
	if(initIvBuff != MEM_NULL)
	{
	   myAPIParam.pICV  = initIvBuff;
	}
	HASHInitParamsInit(&myInitParam, MEMtPX8(shaWorkTmp), inBuff, inLen, myAPIParam.blockSize, ALG_SHA_1);
	HASHInitMain(&myInitParam, &myAPIParam, MEM_NULL, MEM_NULL, HASH_INIT);
	HASHUpdateMain(&myInitParam, &myAPIParam);
	remainLength = myInitParam.remainLength;
	tempLen = U8_S_U8(myAPIParam.blockSize, remainLength);
	ecpMem_Fill(&myInitParam.hashWorkSpace[remainLength], 0, tempLen);
	SHA1Transform(MEMtPX32(myInitParam.hashWorkSpace),MEMtPX32(myInitParam.hashResult));
#if U8_2_U32_INVERSE
	BigEndian2LittleEndian(myInitParam.hashResult, myAPIParam.digestSize);
#else
#endif
	memCopyNoAtomic(outBuff, myInitParam.hashResult, myAPIParam.digestSize);
}
#endif


/*******************************************************************************
 * Function name:	TripleDESForUICC
 * Description: UICC中3DES算法的接口函数
 * Input parameter:
 *		pbSrcStr:	源数据(若有初始值，则可用参数pbTgtStr传递，用参数bMod来区分)
 *		keyContext:	主密钥
 *		pbTgtStr:	结果数据(若有初始值，则可用此参数传递，用参数bMod来区分)
 *		bMod: 	ECB_Encrypt     : 	ECB	加密
 *				ECB_Decrypt     :	ECB	解密
 *				CBC_Encrypt     :   无初始值CBC加密
 *				CBC_Decrypt  	:	无初始值CBC解密
 *				CBC_Init_Encrypt:	有初始值CBC加密
 *				CBC_Init_Decrypt:	有初始值CBC解密
 *		lgth:	源数据长度
 *		keyLength:	主密钥长度
 *
 * Output parameter:     void
 ********************************************************************************/
void TripleDESForUICC(u8 * pbSrcStr,u8 keyContext[16],u8 * pbTgtStr, u8 bMod, u16 lgth, u8 keyLength)
{
	u8 X_A icv[8];

	if((bMod & CBC_Init_Decrypt) == CBC_Init_Decrypt) //有初始值解密
	{
		bMod = CBC_Decrypt;

		ecpMem_Copy(icv, pbTgtStr, 8);
	}
	else if((bMod & CBC_Init_Encrypt) == CBC_Init_Encrypt) //有初始值加密
	{
		bMod = CBC_Encrypt;

		ecpMem_Copy(icv, pbTgtStr, 8);
	}
	else //无初始值
	{
		ecpMem_Set(icv, 0x00, 8);
	}

	if(0x08 == keyLength)
	{
		bMod = U8_OR_U8(bMod, 0x40);
	}
	else if(0x18 == keyLength)
	{
		bMod = U8_OR_U8(bMod, 0x80);
	}

	Triple_DES(MEMtPX8(pbSrcStr), MEMtPM8(keyContext), MEMtPX8(pbTgtStr), lgth, MEMtPM8(icv), bMod, SENSITIVE_DATA_MASKED);
}

/*******************************************************************************
 * Function name:	SingleDESForUICC
 * Description: UICC中SingleDES算法的接口函数
 * Input parameter:
 *		pbSrcStr:	源数据(若有初始值，则可用参数pbTgtStr传递，用参数bMod来区分)
 *		keyContext:	主密钥
 *		pbTgtStr:	结果数据(若有初始值，则可用此参数传递，用参数bMod来区分)
 *		bMod: 	ECB_Encrypt     : 	ECB	加密
 *				ECB_Decrypt     :	ECB	解密
 *				CBC_Encrypt     :   无初始值CBC加密
 *				CBC_Decrypt  	:	无初始值CBC解密
 *				CBC_Init_Encrypt:	有初始值CBC加密
 *				CBC_Init_Decrypt:	有初始值CBC解密
 *		lgth:	源数据长度
 *
 * Output parameter:     void
 ********************************************************************************/
void SingleDESForUICC(u8 * pbSrcStr, u8 keyContext[8], u8 * pbTgtStr, u8 bMod, u16 lgth)
{
	TripleDESForUICC(pbSrcStr, keyContext, pbTgtStr, bMod, lgth, 0x08);
}

///*
//*@description: UICC的AES加解密接口
//*
//*@function   : AESCryptForUICC
//*
//*@param      : u8 *dataIn   输入
//*              u8 *dataOut  输出
//*              u8 *keyExpansion  密钥
//*              u8 KeyByteLen  密钥长度
//*              u8 mode  模式：加解密
//*
//*@return     : void
//*/
//void AESCryptForUICC(u8 *dataIn, u8 *dataOut, u8 *keyExpansion, u8 KeyByteLen, u8 mode)
//{
//	DoAES_Crypt(MEMtPX8(dataIn), MEMtPX8(dataOut), (MODE_EN_OR_DE_MASK & mode), MEMtPX32(keyExpansion), KeyByteLen);
//}

u8 I_A G_AESKeyByteLen; //AESkey的长度

/*
*@description: AES Encrypt的基本算法，对16字节的dataIn明文，使用编排后的密钥keyExpansion，进行aes加密，得到16字节的dataOut密文
*
*@function   : AESEncryptCore
*
*@See        :
*
*@param      : u8 *dataIn   明文输入
*              u8 *dataOut  密文输出
*              u8 *keyExpansion
*
*@return     : void
*/
void AESEncryptForUICC(u8 *dataIn, u8 *dataOut, u8 *keyExpansion)
{
	Sym_Crypt_Param_t 	X_A 	myCryptParam;

//	memCopyNoAtomic(MEMtPM8(dataOut), MEMtPM8(dataIn), AES_BLOCKSIZE);
	
	SymCryptParamsInit(&myCryptParam, NULL, MEMtPX32(keyExpansion), U8t16(G_AESKeyByteLen), dataIn, dataOut, ECB_Encrypt);
	myCryptParam.noOfBlocks = 1;
	if (ERR_SYM_CRYPT_OK != Sym_AES_Crypt(&myCryptParam))
	{
		faultDetected();
	}

	return;

//	DoAES_Crypt(MEMtPX8(dataOut), MEMtPX8(dataOut), MODE_ENCRYPT, MEMtPX32(keyExpansion), G_AESKeyByteLen);
}

/*
*@description: AES Decrypt的基本算法，对16字节的dataIn密文，使用编排后的密钥keyExpansion，进行aes解密，得到16字节的dataOut明文
*
*@function   : AESDecryptCore
*
*@See        :
*
*@param      : u8 *dataIn   明文输入
*              u8 *dataOut  密文输出
*              u8 *keyExpansion
*
*@return     : void
*/
void AESDecryptForUICC(u8 *dataIn, u8 *dataOut, u8 *keyExpansion)
{
	Sym_Crypt_Param_t 	X_A 	myCryptParam;
//	memCopyNoAtomic(MEMtPM8(dataOut), MEMtPM8(dataIn), AES_BLOCKSIZE);

	SymCryptParamsInit(&myCryptParam, NULL, MEMtPX32(keyExpansion), U8t16(G_AESKeyByteLen), dataIn, dataOut, ECB_Decrypt);
	myCryptParam.noOfBlocks = 1;
	if (ERR_SYM_CRYPT_OK != Sym_AES_Crypt(&myCryptParam))
	{
		faultDetected();
	}

	return;
	
//	DoAES_Crypt(MEMtPX8(dataOut), MEMtPX8(dataOut), MODE_DECRYPT, MEMtPX32(keyExpansion), G_AESKeyByteLen);
}

/*
*@description: UICC的密钥编排的接口
*
*@function   : KeyExpansionForUICC
*
*@param      : u8 *key  原始密钥
*@param        u8 KeyByteLen  密钥长度
*@param        u8 *keyExpansion  编排后的密钥
*              u8 mode  模式：加解密
*
*@return     : void
*/
void KeyExpansion(u8 *key, u8 *keyExpansion)
{
	u32 X_A symKeyBuffer[U16_SHR_U8(SYM_EXT_KEY_BYTE_LEN, 2)];
	u16 X_A keyBitLen;
	Sym_LoadKey_Param_t X_A myLoadKeyParam;

	keyBitLen = U16_SHL_U16(U8t16(G_AESKeyByteLen),3);

	SymKeyInitParamsInit(&myLoadKeyParam, MEMtPM8(key), symKeyBuffer, keyBitLen, MODE_ENCRYPT, SENSITIVE_DATA_NOT_MASK, ALG_AES);

	SymKeyInitDispatcher(&myLoadKeyParam, ALG_AES); //密钥编排

	ecpMem_Copy(keyExpansion, myLoadKeyParam.pKeyBuffer, 4*(G_AESKeyByteLen +28)); //将编排后的密钥拷贝到keyExpansion
}

u8 CheckAesKeyLength(u8 keyLength)
{
	if((keyLength == 16) || (keyLength == 24) || (keyLength == 32))
	{
		G_AESKeyByteLen = keyLength;

		return TRUE;
	}

	return FALSE;
}
#if VM_WITH_ECC

#if WITH_ECC_KEY_256_P
u8 const  ECC_P256_DOMAINPARAMS[] =
{
	/* P */0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/* a */0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
    /* b */0x5A,0xC6,0x35,0xD8,0xAA,0x3A,0x93,0xE7,0xB3,0xEB,0xBD,0x55,0x76,0x98,0x86,0xBC,0x65,0x1D,0x06,0xB0,0xCC,0x53,0xB0,0xF6,0x3B,0xCE,0x3C,0x3E,0x27,0xD2,0x60,0x4B,
	0x04,
    /* Gx*/0x6B,0x17,0xD1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2,0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96,
    /* Gy*/0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16,0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xF5,
	/* n */0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51,
	/* h */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	/* d */0xCF,0x99,0x03,0x58,0xD3,0x88,0x23,0x09,0x78,0xA1,0xFB,0x1E,0x69,0xCC,0x80,0x14,0xDE,0x65,0x27,0x9A,0x27,0x21,0xE6,0x16,0xB6,0xA7,0xCF,0x28,0x4B,0xE0,0x63,0x70
};
#endif

#if WITH_ECC_KEY_384_P
u8 const  ECC_P384_DOMAINPARAMS[] =
{
	/* P */0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
	/* a */0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFC,
	/* b */0xB3,0x31,0x2F,0xA7,0xE2,0x3E,0xE7,0xE4,0x98,0x8E,0x05,0x6B,0xE3,0xF8,0x2D,0x19,0x18,0x1D,0x9C,0x6E,0xFE,0x81,0x41,0x12,0x03,0x14,0x08,0x8F,0x50,0x13,0x87,0x5A,0xC6,0x56,0x39,0x8D,0x8A,0x2E,0xD1,0x9D,0x2A,0x85,0xC8,0xED,0xD3,0xEC,0x2A,0xEF,
	0x04,
	/* Gx*/0xAA,0x87,0xCA,0x22,0xBE,0x8B,0x05,0x37,0x8E,0xB1,0xC7,0x1E,0xF3,0x20,0xAD,0x74,0x6E,0x1D,0x3B,0x62,0x8B,0xA7,0x9B,0x98,0x59,0xF7,0x41,0xE0,0x82,0x54,0x2A,0x38,0x55,0x02,0xF2,0x5D,0xBF,0x55,0x29,0x6C,0x3A,0x54,0x5E,0x38,0x72,0x76,0x0A,0xB7,
	/* Gy*/0x36,0x17,0xDE,0x4A,0x96,0x26,0x2C,0x6F,0x5D,0x9E,0x98,0xBF,0x92,0x92,0xDC,0x29,0xF8,0xF4,0x1D,0xBD,0x28,0x9A,0x14,0x7C,0xE9,0xDA,0x31,0x13,0xB5,0xF0,0xB8,0xC0,0x0A,0x60,0xB1,0xCE,0x1D,0x7E,0x81,0x9D,0x7A,0x43,0x1D,0x7C,0x90,0xEA,0x0E,0x5F,
	/* n */0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC7,0x63,0x4D,0x81,0xF4,0x37,0x2D,0xDF,0x58,0x1A,0x0D,0xB2,0x48,0xB0,0xA7,0x7A,0xEC,0xEC,0x19,0x6A,0xCC,0xC5,0x29,0x73,
	/* h */0x01
};
#endif

#if WITH_ECC_KEY_256_BPR1
u8 const  ECC_brainpoolP256r1_DOMAINPARAMS[] =
{
  /* p */0xA9,0xFB,0x57,0xDB,0xA1,0xEE,0xA9,0xBC,0x3E,0x66,0x0A,0x90,0x9D,0x83,0x8D,0x72,0x6E,0x3B,0xF6,0x23,0xD5,0x26,0x20,0x28,0x20,0x13,0x48,0x1D,0x1F,0x6E,0x53,0x77,
  /* a */0x7D,0x5A,0x09,0x75,0xFC,0x2C,0x30,0x57,0xEE,0xF6,0x75,0x30,0x41,0x7A,0xFF,0xE7,0xFB,0x80,0x55,0xC1,0x26,0xDC,0x5C,0x6C,0xE9,0x4A,0x4B,0x44,0xF3,0x30,0xB5,0xD9,
  /* b */0x26,0xDC,0x5C,0x6C,0xE9,0x4A,0x4B,0x44,0xF3,0x30,0xB5,0xD9,0xBB,0xD7,0x7C,0xBF,0x95,0x84,0x16,0x29,0x5C,0xF7,0xE1,0xCE,0x6B,0xCC,0xDC,0x18,0xFF,0x8C,0x07,0xB6,
  0x04,
  /* Gx*/0x8B,0xD2,0xAE,0xB9,0xCB,0x7E,0x57,0xCB,0x2C,0x4B,0x48,0x2F,0xFC,0x81,0xB7,0xAF,0xB9,0xDE,0x27,0xE1,0xE3,0xBD,0x23,0xC2,0x3A,0x44,0x53,0xBD,0x9A,0xCE,0x32,0x62,
  /* Gy*/0x54,0x7E,0xF8,0x35,0xC3,0xDA,0xC4,0xFD,0x97,0xF8,0x46,0x1A,0x14,0x61,0x1D,0xC9,0xC2,0x77,0x45,0x13,0x2D,0xED,0x8E,0x54,0x5C,0x1D,0x54,0xC7,0x2F,0x04,0x69,0x97,
  /* n */0xA9,0xFB,0x57,0xDB,0xA1,0xEE,0xA9,0xBC,0x3E,0x66,0x0A,0x90,0x9D,0x83,0x8D,0x71,0x8C,0x39,0x7A,0xA3,0xB5,0x61,0xA6,0xF7,0x90,0x1E,0x0E,0x82,0x97,0x48,0x56,0xA7,
  /*h */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
};
#endif
#if WITH_ECC_KEY_256_BPT1
u8 const ECC_brainpoolP256t1_DOMAINPARAMS[] =
{
	/* p */0xA9,0xFB,0x57,0xDB,0xA1,0xEE,0xA9,0xBC,0x3E,0x66,0x0A,0x90,0x9D,0x83,0x8D,0x72,0x6E,0x3B,0xF6,0x23,0xD5,0x26,0x20,0x28,0x20,0x13,0x48,0x1D,0x1F,0x6E,0x53,0x77,
	/* a */0xA9,0xFB,0x57,0xDB,0xA1,0xEE,0xA9,0xBC,0x3E,0x66,0x0A,0x90,0x9D,0x83,0x8D,0x72,0x6E,0x3B,0xF6,0x23,0xD5,0x26,0x20,0x28,0x20,0x13,0x48,0x1D,0x1F,0x6E,0x53,0x74,
	/* b */0x66,0x2C,0x61,0xC4,0x30,0xD8,0x4E,0xA4,0xFE,0x66,0xA7,0x73,0x3D,0x0B,0x76,0xB7,0xBF,0x93,0xEB,0xC4,0xAF,0x2F,0x49,0x25,0x6A,0xE5,0x81,0x01,0xFE,0xE9,0x2B,0x04,
	0x04,
	/* Gx*/0xA3,0xE8,0xEB,0x3C,0xC1,0xCF,0xE7,0xB7,0x73,0x22,0x13,0xB2,0x3A,0x65,0x61,0x49,0xAF,0xA1,0x42,0xC4,0x7A,0xAF,0xBC,0x2B,0x79,0xA1,0x91,0x56,0x2E,0x13,0x05,0xF4,
	/* Gy*/0x2D,0x99,0x6C,0x82,0x34,0x39,0xC5,0x6D,0x7F,0x7B,0x22,0xE1,0x46,0x44,0x41,0x7E,0x69,0xBC,0xB6,0xDE,0x39,0xD0,0x27,0x00,0x1D,0xAB,0xE8,0xF3,0x5B,0x25,0xC9,0xBE,
	/* n */0xA9,0xFB,0x57,0xDB,0xA1,0xEE,0xA9,0xBC,0x3E,0x66,0x0A,0x90,0x9D,0x83,0x8D,0x71,0x8C,0x39,0x7A,0xA3,0xB5,0x61,0xA6,0xF7,0x90,0x1E,0x0E,0x82,0x97,0x48,0x56,0xA7,
	/* h */0x01
};
#endif
#if WITH_ECC_KEY_512
u8 const ECC_brainpoolP512r1_DOMAINPARAMS[] =
{
	/* p */0xAA,0xDD,0x9D,0xB8,0xDB,0xE9,0xC4,0x8B,0x3F,0xD4,0xE6,0xAE,0x33,0xC9,0xFC,0x07,0xCB,0x30,0x8D,0xB3,0xB3,0xC9,0xD2,0x0E,0xD6,0x63,0x9C,0xCA,0x70,0x33,0x08,0x71,0x7D,0x4D,0x9B,0x00,0x9B,0xC6,0x68,0x42,0xAE,0xCD,0xA1,0x2A,0xE6,0xA3,0x80,0xE6,0x28,0x81,0xFF,0x2F,0x2D,0x82,0xC6,0x85,0x28,0xAA,0x60,0x56,0x58,0x3A,0x48,0xF3,
	/* a */0x78,0x30,0xA3,0x31,0x8B,0x60,0x3B,0x89,0xE2,0x32,0x71,0x45,0xAC,0x23,0x4C,0xC5,0x94,0xCB,0xDD,0x8D,0x3D,0xF9,0x16,0x10,0xA8,0x34,0x41,0xCA,0xEA,0x98,0x63,0xBC,0x2D,0xED,0x5D,0x5A,0xA8,0x25,0x3A,0xA1,0x0A,0x2E,0xF1,0xC9,0x8B,0x9A,0xC8,0xB5,0x7F,0x11,0x17,0xA7,0x2B,0xF2,0xC7,0xB9,0xE7,0xC1,0xAC,0x4D,0x77,0xFC,0x94,0xCA,
	/* b */0x3D,0xF9,0x16,0x10,0xA8,0x34,0x41,0xCA,0xEA,0x98,0x63,0xBC,0x2D,0xED,0x5D,0x5A,0xA8,0x25,0x3A,0xA1,0x0A,0x2E,0xF1,0xC9,0x8B,0x9A,0xC8,0xB5,0x7F,0x11,0x17,0xA7,0x2B,0xF2,0xC7,0xB9,0xE7,0xC1,0xAC,0x4D,0x77,0xFC,0x94,0xCA,0xDC,0x08,0x3E,0x67,0x98,0x40,0x50,0xB7,0x5E,0xBA,0xE5,0xDD,0x28,0x09,0xBD,0x63,0x80,0x16,0xF7,0x23,
	0x04,
	/* Gx*/0x81,0xAE,0xE4,0xBD,0xD8,0x2E,0xD9,0x64,0x5A,0x21,0x32,0x2E,0x9C,0x4C,0x6A,0x93,0x85,0xED,0x9F,0x70,0xB5,0xD9,0x16,0xC1,0xB4,0x3B,0x62,0xEE,0xF4,0xD0,0x09,0x8E,0xFF,0x3B,0x1F,0x78,0xE2,0xD0,0xD4,0x8D,0x50,0xD1,0x68,0x7B,0x93,0xB9,0x7D,0x5F,0x7C,0x6D,0x50,0x47,0x40,0x6A,0x5E,0x68,0x8B,0x35,0x22,0x09,0xBC,0xB9,0xF8,0x22,
	/* Gy*/0x7D,0xDE,0x38,0x5D,0x56,0x63,0x32,0xEC,0xC0,0xEA,0xBF,0xA9,0xCF,0x78,0x22,0xFD,0xF2,0x09,0xF7,0x00,0x24,0xA5,0x7B,0x1A,0xA0,0x00,0xC5,0x5B,0x88,0x1F,0x81,0x11,0xB2,0xDC,0xDE,0x49,0x4A,0x5F,0x48,0x5E,0x5B,0xCA,0x4B,0xD8,0x8A,0x27,0x63,0xAE,0xD1,0xCA,0x2B,0x2F,0xA8,0xF0,0x54,0x06,0x78,0xCD,0x1E,0x0F,0x3A,0xD8,0x08,0x92,
	/* n */0xAA,0xDD,0x9D,0xB8,0xDB,0xE9,0xC4,0x8B,0x3F,0xD4,0xE6,0xAE,0x33,0xC9,0xFC,0x07,0xCB,0x30,0x8D,0xB3,0xB3,0xC9,0xD2,0x0E,0xD6,0x63,0x9C,0xCA,0x70,0x33,0x08,0x70,0x55,0x3E,0x5C,0x41,0x4C,0xA9,0x26,0x19,0x41,0x86,0x61,0x19,0x7F,0xAC,0x10,0x47,0x1D,0xB1,0xD3,0x81,0x08,0x5D,0xDA,0xDD,0xB5,0x87,0x96,0x82,0x9C,0xA9,0x00,0x69,
	/* h */0x01
};
#endif
#if WITH_ECC_KEY_521
u8 const ECC_P_521_DOMAINPARAMS[] =
{
	/* p */0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	/* a */0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
	/* b */0x00,0x51,0x95,0x3E,0xB9,0x61,0x8E,0x1C,0x9A,0x1F,0x92,0x9A,0x21,0xA0,0xB6,0x85,0x40,0xEE,0xA2,0xDA,0x72,0x5B,0x99,0xB3,0x15,0xF3,0xB8,0xB4,0x89,0x91,0x8E,0xF1,0x09,0xE1,0x56,0x19,0x39,0x51,0xEC,0x7E,0x93,0x7B,0x16,0x52,0xC0,0xBD,0x3B,0xB1,0xBF,0x07,0x35,0x73,0xDF,0x88,0x3D,0x2C,0x34,0xF1,0xEF,0x45,0x1F,0xD4,0x6B,0x50,0x3F,0x00,
	0x04,
	/* Gx*/0x00,0xC6,0x85,0x8E,0x06,0xB7,0x04,0x04,0xE9,0xCD,0x9E,0x3E,0xCB,0x66,0x23,0x95,0xB4,0x42,0x9C,0x64,0x81,0x39,0x05,0x3F,0xB5,0x21,0xF8,0x28,0xAF,0x60,0x6B,0x4D,0x3D,0xBA,0xA1,0x4B,0x5E,0x77,0xEF,0xE7,0x59,0x28,0xFE,0x1D,0xC1,0x27,0xA2,0xFF,0xA8,0xDE,0x33,0x48,0xB3,0xC1,0x85,0x6A,0x42,0x9B,0xF9,0x7E,0x7E,0x31,0xC2,0xE5,0xBD,0x66,
	/* Gy*/0x01,0x18,0x39,0x29,0x6A,0x78,0x9A,0x3B,0xC0,0x04,0x5C,0x8A,0x5F,0xB4,0x2C,0x7D,0x1B,0xD9,0x98,0xF5,0x44,0x49,0x57,0x9B,0x44,0x68,0x17,0xAF,0xBD,0x17,0x27,0x3E,0x66,0x2C,0x97,0xEE,0x72,0x99,0x5E,0xF4,0x26,0x40,0xC5,0x50,0xB9,0x01,0x3F,0xAD,0x07,0x61,0x35,0x3C,0x70,0x86,0xA2,0x72,0xC2,0x40,0x88,0xBE,0x94,0x76,0x9F,0xD1,0x66,0x50,
	/* n */0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFA,0x51,0x86,0x87,0x83,0xBF,0x2F,0x96,0x6B,0x7F,0xCC,0x01,0x48,0xF7,0x09,0xA5,0xD0,0x3B,0xB5,0xC9,0xB8,0x89,0x9C,0x47,0xAE,0xBB,0x6F,0xB7,0x1E,0x91,0x38,0x64,0x09,
	/* h */0x01
};
#endif
typedef struct ECDomainParamsStruct 
{
	u16 keyParaRef;
	u8 ECCKeylength;
	u16 hValue;
	PM8 domainParams;
}ECDomainParamsStruct;

static const ECDomainParamsStruct ECDomainParamsList[]= 
{
	#if WITH_ECC_KEY_256_P
	{ECCCure_P_256_KeyParaRef,				ALG_EC_FP_KEY_256,	0x01,	(PM8)(ECC_P256_DOMAINPARAMS)},
	#endif
	#if WITH_ECC_KEY_384_P
	{ECCCure_P_384_KeyParaRef,				ALG_EC_FP_KEY_384,	0x01,	(PM8)(ECC_P384_DOMAINPARAMS)},
	#endif
	#if WITH_ECC_KEY_256_BPR1
	{ECCCure_brainpoolP256r1_KeyParaRef,	ALG_EC_FP_KEY_256,	0x01,	(PM8)(ECC_brainpoolP256r1_DOMAINPARAMS)},
	#endif
	#if WITH_ECC_KEY_256_BPT1
	{ECCCure_brainpoolP256t1_KeyParaRef,	ALG_EC_FP_KEY_256,	0x01,	(PM8)(ECC_brainpoolP256t1_DOMAINPARAMS)},
	#endif
#if WITH_ECC_KEY_512
	{ECCCure_brainpoolP512r1_KeyParaRef,	ALG_EC_FP_KEY_512,	0x01,	(PM8)(ECC_brainpoolP512r1_DOMAINPARAMS)},
#endif
#if WITH_ECC_KEY_521 
	{ECCCure_P_521_KeyParaRef,				ALG_EC_FP_KEY_521,	0x01,	(PM8)(ECC_P_521_DOMAINPARAMS)},
#endif
};
/*
*@description: 设置ECC参数到全局变量
*
*@function   : setECDomainParams
*
*@param      : u16 keyRef
*
*@return     : TRUE\FALSE
*@modify     : merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 修改参数类型为u16, 函数写法优化
*/
u8 setECDomainParams(u16 keyRef)
{
	u8 I_A count, sum;

	sum = (sizeof(ECDomainParamsList)/sizeof(ECDomainParamsStruct));

	for(count = 0; count < sum; count++)
	{
		if(keyRef == ECDomainParamsList[count].keyParaRef)
		{
			g_ECKeyStore.b_PLength = ECDomainParamsList[count].ECCKeylength;

			g_ECKeyStore.b_RLength = g_ECKeyStore.b_PLength;

			g_ECKeyStore.b_HValue = ECDomainParamsList[count].hValue;

			g_ECKeyStore.pb_domainParams = ECDomainParamsList[count].domainParams;

			break;
		}
	}

	if(count == sum) //没有找到该keyRef对应的ECDomainParams
	{
		return FALSE;
	}

	g_ECKeyStore.bParamStatus = FP_DOMAIN_PARAM_SETED;

	g_CryptoData.pbBuffer = bTempArr;

	//memRefresh();

	return TRUE;
}


/*
*@description: 设置ECC公钥或私钥到全局变量
*
*@function   : setECPubOrPrivKeyRef
*
*@param      : u8 keyType  TYPE_EC_FP_PRIVATE \ TYPE_EC_FP_PUBLIC
*@param        u8 *privateKey	私钥
*@param        u8 *publicKey	公钥
*
*@return     : void
*/
void setECPubOrPrivKeyRef(u8 keyType, u8 *privateKey, u8 *publicKey)
{
    switch(keyType)
    {
	case TYPE_EC_FP_PRIVATE:
    	g_ECKeyStore.pb_PrivateKeyS = MEMtPN8(privateKey);
    	break;

    case TYPE_EC_FP_PUBLIC:
    	g_ECKeyStore.pb_PublicKeyQxy = MEMtPN8(publicKey);
    	break;
    }
}

#if (M2M_EUICC || GP__SCP__SCP11__FOR_MASK) // merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: SCP11也用到ECC曲线
/*
*@function    :checkPointOnEccCurve
*
*@description :用于EUICC M2M版建立密钥过程中 检查下发的公钥是否在曲线上（曲线参数从EUICC证书中获取）
*
*@see         :
*
*@param       :u8 *publicKey  公钥  64字节
*@param       :u8 keyRef     曲线参数类型
*
*@return      :  TRUE/FALSE
*/
u8 checkPointOnEccCurve(u8 *publicKey, u8 publicKeyLen, u8 keyRef)
{
	u8 pkValue[2*MAX_ECC_KEY_BYTELEN+1];

	if(FALSE == setECDomainParams(keyRef))
	{
		return FALSE;
	}

	pkValue[0] = 0x04; //添加04

	ecpMem_Copy(&pkValue[1], publicKey, publicKeyLen);

	setECPubOrPrivKeyRef(TYPE_EC_FP_PUBLIC, NULL, pkValue);

	if(EC_KEY_CHECK_FAIL == CheckECPublicKey())
	{
		return FALSE;
	}

	return TRUE;
}
#endif //M2M_EUICC

#endif //VM_WITH_ECC

/*
 *@description :this function is used to implement Secure Hash Algorithm1，该函数的使用是独立的，只需要输入源数据和数据长度即可计算出HASH值
 *
 *@param       :unsigned char* digest     输出的HASH值，长度固定为16字节
 *@param       :unsigned char* sourcedata 输入的源数据的起始ram地址
 *@param       :unsigned int length       输入数据的长度
 *@param      : u8 mode                 0表示源数据来自于ram数组/1表示源数据来自于E2地址
 *@param      : memref fileAddr            输入的源数据的起始E2地址
 *
 */
void MD5DoFinalSingleCall(PM8 digest /*[16]*/, PM8 sourcedata,u16 length, u8 mode,PM8 fileAddr)
{
	HASHDoFinal(MEMtPM8(sourcedata), length, MEMtPM8(digest), HASH_INIT, NULL, NULL, ALG_MD); //MD5
}

/*
 *@description :this function is used to implement Secure Hash Algorithm1，该函数的使用是独立的，只需要输入源数据和数据长度即可计算出HASH值.
 *在hardware_cryptography.c文件中有实现同样功能的硬算法  SHA256DoFinalSingleCall_Ex(memref inAddr,u16 inLen ,u8 *hashResult), 传入参数后续需要统一
 *
 *@param       :digest                    输出的HASH值，长度固定为32字节
 *@param       :sourcedata                待处理的源数据
 *@param       :length                    待处理的源数据长度
 *@param       :u8 mode                 0表示源数据来自于ram数组/1表示源数据来自于E2地址
 *@param       :memref fileAddr            输入的源数据的起始E2地址, 逻辑地址
 *
 */
void SHA256DoFinalSingleCall(PM8 digest, PM8 sourcedata,u16 length, u8 mode, PM8 fileAddr) 
{
	HASHDoFinal(MEMtPM8(sourcedata), length, MEMtPM8(digest), HASH_INIT, NULL, NULL, ALG_SHA_256); //SHA256
}

/*
 *@description :this function is used to implement Secure Hash Algorithm1
 *
 *@param       :digest :the result  buf,输出长度固定为20字节
 *@param       :sourcedata:待处理的数据缓冲区
 *@param       :length    :待处理的数据长度
 *
 * Add padding and return the message digest
 */
void SHA1Final( PM8 digest, PM8 sourcedata,u16 length, PM8 fileAddr, u8 mode)
{
	HASHDoFinal(MEMtPM8(sourcedata), length, MEMtPM8(digest), HASH_INIT, NULL, NULL, ALG_SHA_1); //SHA1
}
