/************************************************************
 Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
 FileName: Crypto_HAL.c.
 Author: Lizumeng.
 Version : 1.0.
 Date: 2010/02/20.
 Description: This source file contains functions used to encrypt or
 decrypt with hardware implement.
 History:
 <author>    <time>    <version>    <desc>
 Lizumeng   10/02/20     1.0     build this moudle
 *************************************************************/
//#include "hardware.h"
#include "RSA_API.h"
#include "SysConfigHalRam.h"
#include "chipHead.h"
#include "SecOperationHal.h"

CryptoData			  X_A		 g_CryptoData;

#if WITH_RSA

#pragma RSA_RAM_START

#if	(RSA_CRT_MODULAR_SAVE || RSA_CRT_E_SAVE)
RSAKeyGenCRTNEStore		X_A		g_RSAKeyGenCRTNEStore;
#endif	
RSAKeyRefStore		  X_A		 g_RSAKeyStore;
CryptoData			  X_A		 g_CryptoData;

RSA_RAM_END

/**********************************************************************************
 * Function name: getPrime
 * Description: This function generates a probable prime for RSA of length primeLength bytes.
 * A seed can be passed as input parameter. If no seed is passed the seed is generated at random.
 * Least Significant Byte First Oriented and vice versa.
 * input parameter:
 *      MEM_U8_P startingSeed    if equals 0 : the seed is generated at random
 *                               if different from 0 : address of the seed in RAM AVR or EEPROM
 *      u8 primeLength           length of prime in bytes multiple of 4
 * output parameter:
 *		OK / NOK.
 * NOTE: The prime is generated in the AdvX RAM : KEYGEN_BASE_N(primeLength).
 * Add of a loop with check of the LSB byte.
 * The LSB byte of the prime number generated must be different from 0x01.
 * If it is not the case a new prime generation must be done.
 * This is targetted to be corrected in version 00.03.01.04 of the Toolbox 3.
 * If startingSeed is different from zero and the probable prime generated has its
 *  LSB byte to 0x01 the function returns NOK.
 **********************************************************************************/
u8 getPrime(PM8 startingSeed, u8 primeLength)
{
    return true;
}

/**********************************************************************************
 * Function name: getPrimeCheckExp
 * Description: This function generates a prime for RSA of length primeLength bytes
 * with the property that it is coprime with the exponent.
 * The seed for the prime is generated at random.
 * input parameter:
 *      u8 primeLength    length of prime in bytes multiple of 4
 *      MEM_U8_P ptrExp   pointer to the encryption exponent in RAM AVR or EEPROM
 *      u8 expLength      length of the exponent in bytes multiple of 4
 * output parameter:
 *		OK / NOK.
 * NOTE: The prime is generated in the AdvX RAM : KEYGEN_BASE_PRECOMP(primeLength).
 **********************************************************************************/
u8 getPrimeCheckExp(u8 primeLength, PM8 ptrExp, u8 expLength)
{
    return true;
}

/**********************************************************************************
 * Function name: calculate_privD
 * Description: This function generates the private exponent for RSA key pair
 * with the public exponent.
 * input parameter:
 *      u8 primeLength    length of prime in bytes multiple of 4
 *      MEM_U8_P ptrExp   pointer to the encryption exponent in RAM AVR or EEPROM
 *      u8 expLength      length of the exponent in bytes multiple of 4
 * output parameter:
 *		OK / NOK.
 * NOTE: The priv exp is generated in the AdvX RAM : KEYGEN_BASE_CONSTANT(primeLength).
 **********************************************************************************/
u8 calculate_privD(u8 primeLength, PM8 pubExp, u8 ExpLength)
{
    return true;
}

/**********************************************************************************
 * Function name: calculate_CRTParam
 * Description: This function generates the CRT private parameter DP, DQ and PQ for RSA key pair
 * with the public exponent.
 * input parameter:
 *      u8 XDATA_AREA * ptrP    pointer to prime P
 *      u8 XDATA_AREA * ptrQ    pointer to prime Q
 *      u8 primeLength          length of prime in bytes multiple of 4
 *      MEM_U8_P ptrExp         pointer to the private exponent in RAM AVR
 *      u16 expLength           length of the exponent in bytes multiple of 4
 * output parameter:
 *		OK / NOK.
 * NOTE: The priv exp is generated in the AdvX RAM : KEYGEN_BASE_CONSTANT(primeLength).
 **********************************************************************************/
u8 calculate_CRTParam(PX8 ptrP, PX8 ptrQ, u8 primeLength, PM8 ptrExp, u16 expLength)
{
    return true;
}

void CryptoRAMCopyToUserRAM(PX8 des, PX8 src, u16 length)
{
//	_HalRamCopy(des, src, length);
	return;
}
/**********************************************************************************
 * Function name: RSAKeyGenCore
 * Description: This function generates RSA standard key parameters or RSA CRT
 * key parameters, and store them in the defined region.
 * input parameter:
 *      The needded information - keyByteLength, keyType, pubExp, pubExpLength
 * are passed through global variable g_RSAKeyStore.
 * output parameter:
 *		none. (The generated key is stored in the defined region)
 * NOTE:
 **********************************************************************************/
#if WITH_RSA_KEY_GEN
void GenRSAKeyPair(RSAKeyGenParam * myRsaKeyRef)
{
//	/*-------------------------------------------------------------------------------*
//	* Back up the current clock configurations 
//	*-------------------------------------------------------------------------------*/
//	#define RSA_Gen_KEY_E_OFFSET					0
//	#define RSA_Gen_KEY_N_OFFSET					(RSA_Gen_KEY_E_OFFSET + (MAX_RSA_E_BYTE_LEN >> 2))
//	#define RSA_Gen_KEY_P_OFFSET					(RSA_Gen_KEY_N_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	#define RSA_Gen_KEY_Q_OFFSET					(RSA_Gen_KEY_P_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_Gen_KEY_DP_OFFSET					(RSA_Gen_KEY_Q_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_Gen_KEY_DQ_OFFSET					(RSA_Gen_KEY_DP_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_Gen_KEY_QInv_OFFSET					(RSA_Gen_KEY_DQ_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_Gen_KEY_KEY_BUFFER_LENGTH			(RSA_Gen_KEY_QInv_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	
//	u32					RSA_Gen_WORKSPACE[RSA_Gen_KEY_KEY_BUFFER_LENGTH];
//	u32 				RSA_Gen_KEY_D[MAX_RSA_MUD_BYTE_LEN >> 2];
//	u32					ret;
//	u16 				PrimeLength;
//	RSA_Info keys;
//	
//	RSA_Gen_WORKSPACE[RSA_Gen_KEY_E_OFFSET] = 0;
//	memCopyNoAtomic(PX8_A_U8(MEMtPX8(&RSA_Gen_WORKSPACE[RSA_Gen_KEY_E_OFFSET]), (MAX_RSA_E_BYTE_LEN - g_RSAKeyStore.wExpLength)), g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
//	
//	keys.pE		=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_E_OFFSET];
//	keys.pN		=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_N_OFFSET];
//	keys.pP		=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_P_OFFSET];
//	keys.pQ		=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_Q_OFFSET];
//	keys.pDp	=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_DP_OFFSET];
//	keys.pDq	=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_DQ_OFFSET];
//	keys.pQInv	=	(unsigned int *)&RSA_Gen_WORKSPACE[RSA_Gen_KEY_QInv_OFFSET];
//	keys.pPreKey = 	NULL;
//	keys.len	=	g_RSAKeyStore.wModulusNLength << 3;
//	PrimeLength =	g_RSAKeyStore.wModulusNLength >> 1;
//	
//	if(ALG_RSA == g_RSAKeyStore.bRSAKeyType)
//	{
//		keys.pD	=	(unsigned int *)RSA_Gen_KEY_D;
//		keys.operMode = 0; //ALG_RSA_STD
//	}
//	else
//	{ 
//		keys.operMode = 4;	//ALG_RSA_CRT
//	}

//	ret = HED_RSA2048_GenKey(&keys);
//	if(ret != 0)
//	{
//		while(1);
//	}
//	
//#if RSA_KEYGEN_WIRTE_IN_HAL	
//	if (g_RSAKeyStore.bRSAKeyType == ALG_RSA)
//	{
//		memCopyNoAtomic(g_RSAKeyStore.pbModulusN,	MEMtPX8(keys.pN),	g_RSAKeyStore.wModulusNLength);
//		memCopyNoAtomic(g_RSAKeyStore.pbParmP,		MEMtPX8(keys.pN),	g_RSAKeyStore.wModulusNLength);
//		memCopyNoAtomic(g_RSAKeyStore.pbParmQ, 		MEMtPX8(keys.pD),	g_RSAKeyStore.wModulusNLength);
//	}
//	else
//	{
//		memCopyNoAtomic(g_RSAKeyStore.pbModulusN,	MEMtPX8(keys.pN),	g_RSAKeyStore.wModulusNLength);

//		memCopyNoAtomic(g_RSAKeyStore.pbParmP,		MEMtPX8(keys.pP),	PrimeLength);
//		memCopyNoAtomic(g_RSAKeyStore.pbParmQ, 		MEMtPX8(keys.pQ),	PrimeLength);
//		memCopyNoAtomic(g_RSAKeyStore.pbParmDP1,	MEMtPX8(keys.pDp),	PrimeLength);
//		memCopyNoAtomic(g_RSAKeyStore.pbParmDQ1,	MEMtPX8(keys.pDq),	PrimeLength);
//		memCopyNoAtomic(g_RSAKeyStore.pbParmInvQ,	MEMtPX8(keys.pQInv), PrimeLength);
//		memCopyNoAtomic(g_RSAKeyGenCRTNEStore.pbCRTExp,	MEMtPX8(keys.pE),	MAX_RSA_E_BYTE_LEN);
//	}
//	
//#else
//	myRsaKeyRef -> pbModulusN 	  =	MEMtPX8( keys.pN);	
//	
//	if(ALG_RSA == g_RSAKeyStore.bRSAKeyType)
//	{	
//		myRsaKeyRef -> pbParmP	  =	MEMtPX8( keys.pD);
//	}
//	else 
//	{
//		myRsaKeyRef -> pbParmP	  =	MEMtPX8(keys.pP);	
//		myRsaKeyRef -> pbParmQ    =	MEMtPX8(keys.pQ );		
//		myRsaKeyRef -> pbParmDP   =	MEMtPX8(keys.pDp);		
//		myRsaKeyRef -> pbParmDQ   =	MEMtPX8(keys.pDq);		
//		myRsaKeyRef -> pbParmInvQ =	MEMtPX8(keys.pQInv);
//	}	
//#endif //RSA_KEYGEN_WIRTE_IN_HAL
//	
	return;
}
#endif
/**********************************************************************************
 * Function name: RSASTDCore
 * Description: This function encrypts the input message, using the following eqaution
 *                   C = M ^ e mod N.
 * input parameter:
 *		u8 XDATA_AREA * cipher		cipher address is equal to message address.
 *		u8 XDATA_AREA * message		global variable g_CryptoData.pbBuffer.
 *		u16 messageLen			    global variable g_CryptoData.wBufferLen.
 *		   RSAKey   			    global variable g_RSAKeyStore.
 * output parameter:
 *		none.
 * NOTE: test through
 **********************************************************************************/
u16 RSASTDCore(void)
{
//	//!特殊情况的处理逻辑
//	//使用平常E的格式作为pv_key的D  。pv_key.setExponent(publicExponent5
//	//publicExponent是0x0003,0x000003,0x00000003,0x0000000003
//	//TYPE_RSA_PUBLIC ，去掉前面0的E一定是小于等于4字节的。
//	
//	#define RSA_STD_KEY_E_OFFSET 	0
//	#define RSA_STD_KEY_N_OFFSET					  (RSA_STD_KEY_E_OFFSET + (MAX_RSA_E_BYTE_LEN >> 2))
//	#define RSA_STD_KEY_IN_OFFSET 					(RSA_STD_KEY_N_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	#define RSA_STD_KEY_OUT_OFFSET					(RSA_STD_KEY_IN_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	#define RSA_STD_KEY_KEY_BUFFER_LENGTH 	(RSA_STD_KEY_OUT_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	
//	u32 				RSA_STD_WORKSPACE[RSA_STD_KEY_KEY_BUFFER_LENGTH];
//	u32 				RSA_STD_KEY_D[MAX_RSA_MUD_BYTE_LEN >> 2];
//	/*-------------------------------------------------------------------------------*
//	* Assign the Key Space 
//	*-------------------------------------------------------------------------------*/		
//	
//	RSA_Info 		keys;
//	u32 				ret;
//	u16					realElength;
//	u16					i;
//	u8 					len_align;
//	PM8					tempPbExp;
//	
//	len_align = U16_R_U8((g_RSAKeyStore.wModulusNLength), 4);
//	
//	if(len_align != 0)
//	{
//		len_align = 4 - len_align;
//		RSA_STD_WORKSPACE[RSA_STD_KEY_N_OFFSET] = 0;
//	}
//	
//	ecpMem_Copy(PX8_A_U16(MEMtPX8(&RSA_STD_WORKSPACE[RSA_STD_KEY_N_OFFSET]), len_align), g_RSAKeyStore.pbModulusN, g_RSAKeyStore.wModulusNLength);
//	 
//	len_align = U16_R_U8((g_CryptoData.wBufferLen), 4);
//	if(len_align != 0)
//	{
//		len_align = 4 - len_align;
//		RSA_STD_WORKSPACE[RSA_STD_KEY_IN_OFFSET] = 0;
//	}
//	ecpMem_Copy(PX8_A_U16(MEMtPX8(&RSA_STD_WORKSPACE[RSA_STD_KEY_IN_OFFSET]), len_align), g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
//	
//	//case 00000000...3
//	realElength	= g_RSAKeyStore.wExpLength;
//	tempPbExp		= g_RSAKeyStore.pbExp;
//	if(realElength > 0x04)
//	{
//		for(i = 0; i < g_RSAKeyStore.wExpLength; U16_INC(i))
//		{
//			if(nvmReadU8(tempPbExp) != 0)
//			{
//				break;
//			}
//			tempPbExp++;
//			realElength--;
//		}
//	}
//	
//	keys.pE			=	(unsigned int *)&RSA_STD_WORKSPACE[RSA_STD_KEY_E_OFFSET];
//	keys.pN			=	(unsigned int *)&RSA_STD_WORKSPACE[RSA_STD_KEY_N_OFFSET];
//	keys.pDataIn	=	(unsigned int *)&RSA_STD_WORKSPACE[RSA_STD_KEY_IN_OFFSET];
//	keys.pDataOut	=	(unsigned int *)&RSA_STD_WORKSPACE[RSA_STD_KEY_OUT_OFFSET];
//	keys.pD = (unsigned int *)RSA_STD_KEY_D;
//	keys.len	= (g_CryptoData.wBufferLen + len_align) << 3;
//	
//	while(1)
//	{
//		if(realElength <= 0x04)
//		{
//			RSA_STD_WORKSPACE[RSA_STD_KEY_E_OFFSET] = 0;
//			ecpMem_Copy(PX8_A_U8(MEMtPX8(&RSA_STD_WORKSPACE[RSA_STD_KEY_E_OFFSET]), (MAX_RSA_E_BYTE_LEN - realElength)), tempPbExp, realElength);
//			keys.operMode = 0;
//			ret = HED_RSA2048_PubOpt(&keys);
//		}
//		else
//		{
//			u8 D_len_align = U16_R_U8((g_RSAKeyStore.wExpLength), 4);
//			if(D_len_align != 0)
//			{
//				D_len_align = 4 - D_len_align;
//				RSA_STD_KEY_D[0] = 0;
//			}
//			ecpMem_Copy(PX8_A_U16(MEMtPX8(RSA_STD_KEY_D), D_len_align), g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
//			
//			keys.operMode = 1 << 1;
//			ret = HED_RSA2048_PriOpt(&keys);
//		}
//		
//		if(ret == 0) // OK
//		{
//			ecpMem_Copy(g_CryptoData.pbBuffer,PX8_A_U8(MEMtPX8(keys.pDataOut), len_align), g_CryptoData.wBufferLen);
//			return ECP_ERR_OK;
//		}
//		else if(ret == 1)// M > N
//		{
//			keys.pDataIn[0]	= 0;
//			ecpMem_Copy(PX8_A_U16(keys.pDataIn, len_align), g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
//			ecpASubBModC(MEMtPX8(keys.pDataIn), (g_RSAKeyStore.wModulusNLength + len_align), MEMtPX8(keys.pN), MEMtPX8(keys.pN), MEMtPX8(keys.pDataIn));
//		}
//		else //check error
//		{
//			while(1);
//		}
//	}
	
	return ECP_ERR_OK;
}

/**********************************************************************************
 * Function name: RSACRTCore
 * Description: This function decrypts the input cipher, using CRT key.
 * input parameter:
 *		u8 XDATA_AREA * cipher		cipher address is equal to message address.
 *		u8 XDATA_AREA * message		global variable g_CryptoData.pbBuffer.
 *		u16 messageLen			global variable g_CryptoData.wBufferLen.
 *		   RSAKey   			global variable g_RSAKeyStore.
 * output parameter:
 *		none.
 * NOTE: test through
 **********************************************************************************/
u16 RSACRTCore(void)
{
//	/*-------------------------------------------------------------------------------*
//	* Back up the current clock configurations 
//	*-------------------------------------------------------------------------------*/	
//	#define RSA_CRT_KEY_IN_OFFSET					0
//	#define RSA_CRT_KEY_OUT_OFFSET					(RSA_CRT_KEY_IN_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_E_OFFSET					(RSA_CRT_KEY_OUT_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_P_OFFSET					(RSA_CRT_KEY_E_OFFSET + (MAX_RSA_E_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_Q_OFFSET					(RSA_CRT_KEY_P_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_DP_OFFSET					(RSA_CRT_KEY_Q_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_DQ_OFFSET					(RSA_CRT_KEY_DP_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_QInv_OFFSET					(RSA_CRT_KEY_DQ_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	#define RSA_CRT_KEY_KEY_BUFFER_LENGTH			(RSA_CRT_KEY_QInv_OFFSET + (MAX_RSA_P_BYTE_LEN >> 2))
//	
//	u32		RSA_CRT_WORKSPACE[RSA_CRT_KEY_KEY_BUFFER_LENGTH];
//	u32		ret;
//	u8		len_align;
//	RSA_Info keys;
//	/*-------------------------------------------------------------------------------*
//	* Assign the Key Space 
//	*-------------------------------------------------------------------------------*/		
//	len_align = U16_R_U8(g_RSAKeyStore.wPrimeLength, 4);
//	if(len_align != 0)
//	{
//		len_align = 4 - len_align;
//		RSA_CRT_WORKSPACE[RSA_CRT_KEY_P_OFFSET] 	= 0;
//		RSA_CRT_WORKSPACE[RSA_CRT_KEY_DP_OFFSET]	= 0;
//		RSA_CRT_WORKSPACE[RSA_CRT_KEY_Q_OFFSET]		= 0;
//		RSA_CRT_WORKSPACE[RSA_CRT_KEY_DQ_OFFSET]	= 0;
//		RSA_CRT_WORKSPACE[RSA_CRT_KEY_QInv_OFFSET] 	= 0;
//	}
//	
//	ecpMem_Copy(PX8_A_U8(MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_P_OFFSET]),    len_align),	g_RSAKeyStore.pbParmP,		g_RSAKeyStore.wPrimeLength);
//	ecpMem_Copy(PX8_A_U8(MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DP_OFFSET]),   len_align),	g_RSAKeyStore.pbParmDP1,	g_RSAKeyStore.wPrimeLength);
//	ecpMem_Copy(PX8_A_U8(MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_Q_OFFSET]),    len_align),	g_RSAKeyStore.pbParmQ,		g_RSAKeyStore.wPrimeLength);
//	ecpMem_Copy(PX8_A_U8(MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DQ_OFFSET]),   len_align),	g_RSAKeyStore.pbParmDQ1,	g_RSAKeyStore.wPrimeLength);
//	ecpMem_Copy(PX8_A_U8(MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_QInv_OFFSET]), len_align),	g_RSAKeyStore.pbParmInvQ,	g_RSAKeyStore.wPrimeLength);
//	if((memReadU32(g_RSAKeyStore.pbExp) == 0)||(g_RSAKeyStore.pbExp == NULL))
//	{
//		ret = HED_RSA2048_CalE((unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_E_OFFSET], (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_P_OFFSET], (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DP_OFFSET], (g_RSAKeyStore.wPrimeLength+len_align) >> 2);
//		if(ret != 0)
//		{
//			while(1);
//		}
//	}
//	else
//	{
//		ecpMem_Copy((MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_E_OFFSET]) + (MAX_RSA_E_BYTE_LEN - g_RSAKeyStore.wExpLength)), g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
//	}
//	
//	keys.pE		=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_E_OFFSET];
//	keys.pN		=	NULL;
//	keys.pP		=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_P_OFFSET];
//	keys.pQ		=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_Q_OFFSET];
//	keys.pDp	=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DP_OFFSET];
//	keys.pDq	=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DQ_OFFSET];
//	keys.pQInv	=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_QInv_OFFSET];
//	keys.pDataIn=	(unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_IN_OFFSET];
//	keys.pDataOut = (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_OUT_OFFSET];
//	keys.operMode = 0;
//	
//	len_align = U16_R_U8(g_CryptoData.wBufferLen, 4);
//	if(len_align != 0)
//	{
//		len_align = 4 - len_align;
//		RSA_CRT_WORKSPACE[RSA_CRT_KEY_IN_OFFSET] = 0;
//	}
//	ecpMem_Copy(MEMtPX8(&RSA_CRT_WORKSPACE[RSA_CRT_KEY_IN_OFFSET]) + len_align, g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
//	keys.len	= (g_CryptoData.wBufferLen + len_align) << 3;
//	keys.pPreKey = NULL;
//	
//	while(1)
//	{
//		ret = HED_RSA2048_PriOpt(&keys);
//		
//		if (ret == 0) // OK
//		{
//			ecpMem_Copy(g_CryptoData.pbBuffer, MEMtPX8(keys.pDataOut) + len_align, g_CryptoData.wBufferLen);
//			return ECP_ERR_OK;
//		}
//		else if (ret == 1)// M > N
//		{
//			keys.pDataIn[0]	= 0;
//			ecpMem_Copy(PX8_A_U16(keys.pDataIn, len_align), g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
//			ecpASubBModC(MEMtPX8(keys.pDataIn), (g_RSAKeyStore.wModulusNLength + len_align), MEMtPX8(keys.pN), MEMtPX8(keys.pN), MEMtPX8(keys.pDataIn));
//		}
//		else if (ret == 2) //len illeage or check error
//		{
//			//while(1);
//			keys.pP 			= (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_Q_OFFSET];
//			keys.pQ 			= (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_P_OFFSET];
//			keys.pDp 			= (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DQ_OFFSET];
//			keys.pDq 			= (unsigned int *)&RSA_CRT_WORKSPACE[RSA_CRT_KEY_DP_OFFSET];
//			keys.pDataIn[0]		= 0;
//			ecpMem_Copy(PX8_A_U8(keys.pDataIn, len_align), g_CryptoData.pbBuffer, g_CryptoData.wBufferLen);
//		}
//		else
//		{
//			while(1);
//		}
//	
//	}
}
#endif
