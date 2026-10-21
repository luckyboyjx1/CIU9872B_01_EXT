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
	big_to_bytes(length, (big)src, des, (BOOL)true);
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
    u8 					nump[MAX_RSA_MUD_BYTE_LEN];
	big 				primeP;
	big 				primeQ;
	big 				dP;
	big 				dQ;
	big 				Qinv;
	big 				p1;
	big 				q1;
	big 				phi;
	big 				t;
	big 				modulusN;
	big 				pbE;
	big 				privD;
	miracl 				* mip;
	u32			 		digitNum;
    u16					keyBitLength;
	u16					primeBitLength;
	u16			 		primeByteLength;
	time_t 				seed;

	keyBitLength 		= g_RSAKeyStore.wModulusNLength << 3;
	primeBitLength 		= keyBitLength >> 1;
	primeByteLength 	= g_RSAKeyStore.wModulusNLength >> 1;

	digitNum 			= keyBitLength >> 2;
	mip 				= mirsys(digitNum, 16);
	p1 					= mirvar(0);
	q1 					= mirvar(0);
	phi 				= mirvar(0);
	t 					= mirvar(0);

	primeP 				= mirvar(0);
	primeQ 				= mirvar(0);


	modulusN 			= mirvar(0);
	pbE 				= mirvar(0);
	privD 				= mirvar(0);

	dP 					= mirvar(0);
    dQ 					= mirvar(0);
    Qinv 				= mirvar(0);

	// init random number generator
	time(&seed);
    irand((unsigned long)seed);   /* change parameter for different values */

    ecpMem_Copy(nump, g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
	bytes_to_big(g_RSAKeyStore.wExpLength, nump , pbE);

	do
	{
		bigbits(primeBitLength, primeP);
		big_to_bytes(primeByteLength, primeP, nump, true);
		if(nump[0] < 0xc0)
		{
			nump[0] |= 0xc0;
			bytes_to_big(g_RSAKeyStore.wModulusNLength >> 1, nump , primeP);
		}

		if(subdivisible(primeP, 2)) incr(primeP, 1, primeP);
		while(!isprime(primeP)) incr(primeP, 2, primeP);

		bigbits(primeBitLength, primeQ);
		big_to_bytes(primeByteLength, primeQ, nump, true);
		if(nump[0] < 0xb0)
		{
			nump[0] |= 0xb0;
			bytes_to_big(g_RSAKeyStore.wModulusNLength >> 1, nump , primeQ);
		}
		if(subdivisible(primeQ, 2)) incr(primeQ, 1, primeQ);
		while (!isprime(primeQ)) incr(primeQ, 2, primeQ);

		decr(primeP, 1, p1);
		decr(primeQ, 1, q1);
		multiply(p1, q1, phi);  /* phi =(p-1)*(q-1) */
	} while(xgcd(pbE, phi, privD, privD, t)!=1);

	multiply(primeP, primeQ, modulusN);

	myRsaKeyRef->pbModulusN = (PX8)modulusN;

	if(ALG_RSA == g_RSAKeyStore.bRSAKeyType)
	{
		myRsaKeyRef->pbParmP = (PX8)privD;
	}
	else
	{ // RSA CRT key
		xgcd(primeQ, primeP, Qinv, Qinv, Qinv);   /* 1/p mod q */

		copy(privD, dP);
		copy(privD, dQ);
		divide(dP, p1, p1);   /* dp=d mod p-1 */
		divide(dQ, q1, q1);   /* dq=d mod q-1 */

		myRsaKeyRef->pbParmP = (PX8)primeP;
		myRsaKeyRef->pbParmQ = (PX8)primeQ;
		myRsaKeyRef->pbParmDP = (PX8)dP;
		myRsaKeyRef->pbParmDQ = (PX8)dQ;
		myRsaKeyRef->pbParmInvQ = (PX8)Qinv;
	}

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    //mirexit();

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
	//standard RSA key
    u8 					nump[MAX_RSA_MUD_BYTE_LEN];
	big 				mod;
	big 				exponent;
	big 				inputMessage;
	big 				outputMessage;
	u16 				keyByteLength;
	u16 				digitNum;
	miracl 				* mip;
	keyByteLength 		= g_RSAKeyStore.wModulusNLength;
	digitNum  			= keyByteLength << 1;
	mip 				= mirsys(digitNum, 16);

	mod 				= mirvar(0);
	exponent 			= mirvar(0);
	inputMessage 		= mirvar(0);
	outputMessage 		= mirvar(0);

	mip->IOBASE 		= 16;
	//N

    ecpMem_Copy(nump, g_RSAKeyStore.pbModulusN, keyByteLength);
	bytes_to_big(keyByteLength, nump, mod);
	//E

    ecpMem_Copy(nump, g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
	bytes_to_big(g_RSAKeyStore.wExpLength, nump, exponent);

	bytes_to_big(g_CryptoData.wBufferLen, g_CryptoData.pbBuffer, inputMessage);
	powmod(inputMessage, exponent, mod, outputMessage);
	big_to_bytes(g_CryptoData.wBufferLen, outputMessage, g_CryptoData.pbBuffer, TRUE);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

    incrementCodeFlowControlFlag();
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
    u8 					temp[MAX_RSA_P_BYTE_LEN];
//	u8 					nump[MAX_RSA_P_BYTE_LEN];
//    u8					numq[MAX_RSA_P_BYTE_LEN];
	//RSA CRT key
	big 				primeP;
	big 				primeQ;
	big 				dP;
	big 				dQ;
	big 				Qinv;
	big 				inputMessage;
	big 				outputMessage;
	big 				pm[2];
	miracl 				* mip;
	u16 				keyByteLength;
	u16 				primeLength;
	u16 				digitNum;
	keyByteLength 		= g_RSAKeyStore.wModulusNLength;
	if(0x01 == (keyByteLength & 0x01) )
	{
		keyByteLength += 1;
	}
	primeLength 		= keyByteLength >> 1;
	digitNum  			= keyByteLength << 1;
	mip 				= mirsys(digitNum, 16);

	primeP 				= mirvar(0);
	primeQ 				= mirvar(0);
	dP 					= mirvar(0);
	dQ 					= mirvar(0);
	Qinv 				= mirvar(0);
	inputMessage 		= mirvar(0);
	outputMessage 		= mirvar(0);
	pm[0]				= mirvar(0);
    pm[1]				= mirvar(0);

    // store parameter
    mip->IOBASE 		= 16;
    ecpMem_Copy(temp, g_RSAKeyStore.pbParmP, primeLength);
	bytes_to_big(primeLength, temp, primeP);
    ecpMem_Copy(temp, g_RSAKeyStore.pbParmQ, primeLength);
	bytes_to_big(primeLength, temp, primeQ);
    ecpMem_Copy(temp, g_RSAKeyStore.pbParmDP1, primeLength);
	bytes_to_big(primeLength, temp, dP);
    ecpMem_Copy(temp, g_RSAKeyStore.pbParmDQ1, primeLength);
	bytes_to_big(primeLength, temp, dQ);
//    ecpMem_Copy(temp, g_RSAKeyStore.pbParmInvQ, primeLength);
//	bytes_to_big(primeLength, temp, Qinv);
	bytes_to_big(g_CryptoData.wBufferLen, g_CryptoData.pbBuffer, inputMessage);

	powmod(inputMessage, dP, primeP, pm[0]);    /* get result mod p */
    powmod(inputMessage, dQ, primeQ, pm[1]);    /* get result mod q */

//    big_to_bytes(primeLength, pm[0], nump, TRUE);
//    big_to_bytes(primeLength, pm[1], numq, TRUE);

//    if(nump[0]> numq[0])
	if(0x01 == mr_compare(pm[0], pm[1]))
    {
		ecpMem_Copy(temp, g_RSAKeyStore.pbParmInvQ, primeLength);
		bytes_to_big(primeLength, temp, Qinv);
//    	bytes_to_big(primeLength, g_RSAKeyStore.pbParmInvQ, Qinv);

		subtract(pm[0], pm[1], pm[0]);  /* poor man's CRT */
		mad(Qinv, pm[0], Qinv, primeP, primeP, outputMessage);
		multiply(outputMessage, primeQ, outputMessage);
		add(outputMessage, pm[1], outputMessage);
    }
    else
    {
    	xgcd(primeP, primeQ, Qinv, Qinv, Qinv);   /* 1/p mod q */

		subtract(pm[1], pm[0], pm[1]);  /* poor man's CRT */
		mad(Qinv, pm[1], Qinv, primeQ, primeQ, outputMessage);
		multiply(outputMessage, primeP, outputMessage);
		add(outputMessage, pm[0], outputMessage);
    }
	big_to_bytes(g_CryptoData.wBufferLen, outputMessage, g_CryptoData.pbBuffer, TRUE);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

    incrementCodeFlowControlFlag();
    return ECP_ERR_OK;
}
#endif
