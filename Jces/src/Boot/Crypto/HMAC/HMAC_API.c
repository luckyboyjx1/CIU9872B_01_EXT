/************************************************************
  Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
  FileName: Crypto_HMAC.c.
  Author: Lizumeng.
  Version : 1.0.
  Date: 2010/04/16.
  Description: This source file contains functions used to compute
               HMAC (with hash function MD5, SHA1,SHA256).
  History:
      <author>    <time>    <version>    <desc>
      Lizumeng   10/04/16     1.0     build this moudle
*************************************************************/
#include "HMAC_API.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#include "MemOperation.h"

#if WITH_HMAC

static void HMACAPIParamsInit(HMAC_API_Param_t * myHmacAPIParam, u8 hmacType)
{
	switch(hmacType)
	{
#if WITH_MD5
		case ALG_HMAC_MD5:
			myHmacAPIParam->h_mac_digest_size 	= MD_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= MD_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_MD;
		break;
#endif

#if WITH_RIPEMD160
		case ALG_HMAC_RIPEMD160:
			myHmacAPIParam->h_mac_digest_size 	= RIPEMD160_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= RIPEMD160_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_RIPEMD160;
		break;
#endif

#if WITH_SHA1
		case ALG_HMAC_SHA1:
			myHmacAPIParam->h_mac_digest_size 	= SHA1_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= SHA1_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_SHA_1;
		break;
#endif

// no sha224 hamc
//#if WITH_SHA224
//		case ALG_HMAC_SHA_224:
//			myInitParam->h_mac_digest_size 		= SHA224_DIGEST_SIZE;
//			myInitParam->h_mac_block_size 		= SHA224_BLOCK_SIZE;
//			myInitParam->h_mac_hash_type 		= ALG_SHA_224;
//		break;
//#endif

#if WITH_SHA256
		case ALG_HMAC_SHA_256:
			myHmacAPIParam->h_mac_digest_size 	= SHA256_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= SHA256_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_SHA_256;
		break;
#endif

#if WITH_SHA384
		case ALG_HMAC_SHA_384:
			myHmacAPIParam->h_mac_digest_size 	= SHA384_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= SHA384_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_SHA_384;
		break;
#endif

#if WITH_SHA512
		case ALG_HMAC_SHA_512:
			myHmacAPIParam->h_mac_digest_size 	= SHA512_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= SHA512_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_SHA_512;
		break;
#endif

#if WITH_SM3
		case ALG_HMAC_SM3:
			myHmacAPIParam->h_mac_digest_size 	= SM3_DIGEST_SIZE;
			myHmacAPIParam->h_mac_block_size 	= SM3_BLOCK_SIZE;
			myHmacAPIParam->h_mac_hash_type 	= ALG_SM3;
		break;
#endif
		default :
			break ;
		}
		return;
}

/*******************************************************************************
 * Function name: HMACInit
 * Description: Initialization of HMAC computation.
 * Input Parameters:
 *   u8 *    inputKey 			HMAC initial secret key
 *   u16     inputKeyLen 		length of the HMAC initial secret key
 *   u8 *    hmacType     		the type which HMAC is used
 * Output Parameters:
 * 		None.
 * NOTE:
 ******************************************************************************/
//void HMACInit(u8 * inputKey, u16 inputKeyLen, u8 hmacType)
void HMACInit(PM8 inputKey, u16 inputKeyLen, PM8 tmpBuff, PM8 mdBuff, PM8 hMacGenKeyBuffer, u8 hmacType)
{
	PM8					h_mac_key;
	u8		X_A			h_mac_gen_key[HASH_BLOCK_SIZE];
	HMAC_API_Param_t 	myHmacAPIParam;
	u8 		X_A 		h_mac_gen_key_len;
	u8 		D_A 		loop_counter;
	PATCH_BOOT_HMAC_API_001();
	HMACAPIParamsInit(&myHmacAPIParam, hmacType);

	// first process the key: padding or digesting
	if(inputKeyLen > myHmacAPIParam.h_mac_block_size)
	{
		// keyLen is longer than block size, digest hmacKey whose length is digest size from key
		HASHDoFinal(inputKey, inputKeyLen, MEMtPM8(h_mac_gen_key), HASH_INIT, MEMtPM8(NULL), MEMtPM8(NULL), myHmacAPIParam.h_mac_hash_type);
		//MessageDigest(g_HmacKey, inputKey, inputKeyLen, g_HashType);
		h_mac_gen_key_len = myHmacAPIParam.h_mac_digest_size;
	}
	else
	{
		// keyLen is smaller than block size or equal to block size
		ecpMem_Copy(h_mac_gen_key, inputKey, inputKeyLen);
		h_mac_gen_key_len = U16t8(inputKeyLen);
	}

	memWriteU8NoAtomic(hMacGenKeyBuffer, h_mac_gen_key_len);
	if (0x00 != h_mac_gen_key_len)
	{
		PM8_INC(hMacGenKeyBuffer); // skip the length byte
		memCopyNoAtomic(hMacGenKeyBuffer, MEMtPM8(h_mac_gen_key), h_mac_gen_key_len);
	}

	h_mac_key = MEMtPM8(h_mac_gen_key);
	// Pad hmacKey and XOR with IPAD for inner digest
	for(loop_counter = 0; loop_counter < h_mac_gen_key_len; U8_INC(loop_counter))
	{
		h_mac_key[loop_counter] = U8_XOR_U8(h_mac_gen_key[loop_counter], IPAD);
	}
	for(loop_counter = h_mac_gen_key_len; loop_counter < myHmacAPIParam.h_mac_block_size; U8_INC(loop_counter))
	{
		h_mac_key[loop_counter] = IPAD;
	}

	/**** Inner Digest (inner HMAC Key digest) ****/
	HASHUpdate(MEMtPM8(h_mac_key), myHmacAPIParam.h_mac_block_size, HASH_INIT, MEMtPM8(mdBuff), MEMtPM8(tmpBuff), myHmacAPIParam.h_mac_hash_type);

	return;
}

/*******************************************************************************
 * Function name: HMACUpdate
 * Description: Update of HMAC computation.
 * Input Parameters:
 *   u8 *    inputData 			    the input buffer
 *   u16     inputDataLen 			length of the input buffer
 * Output Parameters:
 * 		None.
 * NOTE:
 ******************************************************************************/
void HMACUpdate(PM8 inputData, u16 inputDataLen, PM8 tmpBuff, PM8 mdBuff, u8 hmacType)
{
	HMAC_API_Param_t 	myHmacAPIParam;
	HMACAPIParamsInit(&myHmacAPIParam, hmacType);

	/**** Inner Digest (digest of input data) ****/
	HASHUpdate(inputData, inputDataLen, HASH_UPDATE, MEMtPM8(mdBuff), MEMtPM8(tmpBuff), myHmacAPIParam.h_mac_hash_type);
	return;
}

/*******************************************************************************
 * Function name: HMACDofinal
 * Description: Final operation of HMAC computation.
 * Input Parameters:
 *   u8 *    inputData 			    input data buffer
 *   u16     inputDataLen 			length of the input data buffer
 *   u8 *    outputData     		output data buffer
 *   u8      outputDataLen          required length of output MAC
 * Output Parameters:
 * 		length of the result data
 * NOTE:
 ******************************************************************************/
//u8 HMACDofinal(u8 * inputData, u16 inputDataLen, u8 * outputData/*, u8 outputDataLen*/)
u8 HMACDofinal(PM8 inputData, u16 inputDataLen, PM8 outputData, PM8 mdBuff, PM8 tmpBuff, PM8 hMacGenKeyBuffer, u8 hmacType)
{
	PX8					h_mac_gen_key;
	PX8					tmpPtr;
	u8		X_A			workSpace[HASH_BLOCK_SIZE + HASH_ICV_SIZE];
	HMAC_API_Param_t 	myHmacAPIParam;
	u8 		X_A 		h_mac_gen_key_len;
	u8 		D_A 		loop_counter;
	
	PATCH_BOOT_HMAC_API_002();
	
	HMACAPIParamsInit(&myHmacAPIParam, hmacType);

	h_mac_gen_key = workSpace;
	tmpPtr = &workSpace[myHmacAPIParam.h_mac_block_size];

	// whether the update method is invoked, just invoke messageDigest method
	HASHDoFinal(MEMtPM8(inputData), inputDataLen, MEMtPM8(tmpPtr), HASH_UPDATE, MEMtPM8(mdBuff), MEMtPM8(tmpBuff), myHmacAPIParam.h_mac_hash_type);

	// Pad hmacKey and XOR with OPAD for outer digest
	h_mac_gen_key_len = memReadU8(hMacGenKeyBuffer);

	if (0 != h_mac_gen_key_len)  // there are some data remained in last updating.
	{
		PM8_INC(hMacGenKeyBuffer);
		memCopyToRam(h_mac_gen_key, hMacGenKeyBuffer, h_mac_gen_key_len);
	}

	for(loop_counter = 0; loop_counter < h_mac_gen_key_len; U8_INC(loop_counter))
	{
		h_mac_gen_key[loop_counter] = U8_XOR_U8(h_mac_gen_key[loop_counter], OPAD);
	}
	for(loop_counter = h_mac_gen_key_len ; loop_counter < myHmacAPIParam.h_mac_block_size ; U8_INC(loop_counter))
	{
		h_mac_gen_key[loop_counter] = OPAD;
	}

	/**** Outer Digest ****/
	loop_counter = U8_A_U8(myHmacAPIParam.h_mac_digest_size, myHmacAPIParam.h_mac_block_size);
	tmpPtr = workSpace;

	HASHDoFinal(MEMtPM8(h_mac_gen_key), loop_counter, MEMtPM8(tmpPtr), HASH_INIT, NULL, NULL, myHmacAPIParam.h_mac_hash_type);

	memCopyNoAtomic(outputData, MEMtPM8(tmpPtr), myHmacAPIParam.h_mac_digest_size);
	return myHmacAPIParam.h_mac_digest_size;
}

/*******************************************************************************
 * Function name: GeneralHMAC
 * Description: General edition of HMAC computation.
 * Input Parameters:
 *   u8 *    inputKey 				HMAC initial secret key
 *   u16     inputKeyLen 			length of the HMAC initial secret key
 *   u8 *    inputData     		    input data buffer
 *   u16     inputDataLen    		length of input data
 *   u8 *    outputData  			output buffer contains HMAC
 *	 u8 	 outputDataLen			length of output data
 *	 u8 	 hmacType	 			type of HMAC function used
 * Output Parameters:
 * 		length of the result data
 * NOTE:
 * The data stored in mdBuff is as follows:
 * ---------------------------------------------------------
 * | updated block number |  intermediate hash   |
 * ---------------------------------------------------------
 * |       2 byte         |      DIGEST_SIZE     |
 * ---------------------------------------------------------
 * The data stored in tmpBuff is as follows:
 * --------------------------------
 * | tmpDataLen | 		tmpData  	   |
 * ---------------------------------
 * |   1 byte   | 63 or  127 bytes |
 * ---------------------------------
 * |      				BLOCK_SIZE       |
 * The data stored in bTempArr is as follows:
 * ----------------------------------------------------------
 * | h_mac_gen_key | 	  tmpResult  		|
 * -----------------------------------------------------------
 * | XX_BLOCK_SIZE | XX_DIGEST_SIZE 	|
 * -----------------------------------------------------------
 * |      				 |						h_mac_key       						|
 * -----------------------------------------------------------
 * |      				 |						XX_BLOCK_SIZE       				|
 ******************************************************************************/

u8 GeneralHMAC(PM8 inputKey, u16 inputKeyLen, PM8 inputData, u16 inputDataLen, PM8 outputData, u8 outputDataLen, u8 hmacType)
{
	u8		X_A			mdBuff[HASH_ICV_SIZE + 4];
	PX8					h_mac_gen_key;
	u8					tmpBuff[HASH_BLOCK_SIZE];
	u8		X_A			h_mac_key[HASH_BLOCK_SIZE];
	u8		X_A			workSpace[HASH_BLOCK_SIZE + HASH_ICV_SIZE];
	HMAC_API_Param_t 	myHmacAPIParam;
	u8 		X_A 		h_mac_gen_key_len;
	u8 		D_A 		loop_counter;
	
	PATCH_BOOT_HMAC_API_003();
	
	HMACAPIParamsInit(&myHmacAPIParam, hmacType);

	h_mac_gen_key_len = 0;

	h_mac_gen_key = workSpace;
	// first process the key: padding or digesting
	if(inputKeyLen > myHmacAPIParam.h_mac_block_size)
	{
		// keyLen is longer than block size, digest hmacKey whose length is digest size from key
		HASHDoFinal(MEMtPM8(inputKey), inputKeyLen, MEMtPM8(h_mac_gen_key), HASH_INIT, NULL, NULL, myHmacAPIParam.h_mac_hash_type);
		//MessageDigest(g_HmacKey, inputKey, inputKeyLen, g_HashType);
		h_mac_gen_key_len = myHmacAPIParam.h_mac_digest_size;
	}
	else
	{
		// keyLen is smaller than block size or equal to block size
		ecpMem_Copy(h_mac_gen_key, inputKey, inputKeyLen);
		h_mac_gen_key_len = U16t8(inputKeyLen);
	}

	// Pad hmacKey and XOR with IPAD for inner digest
	for(loop_counter = 0; loop_counter < h_mac_gen_key_len; U8_INC(loop_counter))
	{
		h_mac_key[loop_counter] = U8_XOR_U8(h_mac_gen_key[loop_counter], IPAD);
	}
	for(loop_counter = h_mac_gen_key_len; loop_counter < myHmacAPIParam.h_mac_block_size; U8_INC(loop_counter))
	{
		h_mac_key[loop_counter] = IPAD;
	}

	/**** Inner Digest ****/
//	mdBuff 		= &workSpace[myHmacAPIParam.h_mac_block_size];

	HASHUpdate(MEMtPM8(h_mac_key), myHmacAPIParam.h_mac_block_size, HASH_INIT, MEMtPM8(mdBuff), MEMtPM8(tmpBuff), myHmacAPIParam.h_mac_hash_type);
	HASHDoFinal(MEMtPM8(inputData), inputDataLen, MEMtPM8(&workSpace[myHmacAPIParam.h_mac_block_size]), HASH_UPDATE, MEMtPM8(mdBuff), MEMtPM8(tmpBuff), myHmacAPIParam.h_mac_hash_type);

	// Pad hmacKey and XOR with OPAD for outer digest
	for(loop_counter = 0; loop_counter < h_mac_gen_key_len; U8_INC(loop_counter))
	{
		workSpace[loop_counter] = U8_XOR_U8(h_mac_gen_key[loop_counter], OPAD);
	}
	for(loop_counter = h_mac_gen_key_len ; loop_counter < myHmacAPIParam.h_mac_block_size ; U8_INC(loop_counter))
	{
		workSpace[loop_counter] = OPAD;
	}

	/**** Outer Digest ****/
	loop_counter = U8_A_U8(myHmacAPIParam.h_mac_digest_size, myHmacAPIParam.h_mac_block_size);
	HASHDoFinal(MEMtPM8(workSpace), loop_counter, MEMtPM8(workSpace), HASH_INIT, NULL, NULL, myHmacAPIParam.h_mac_hash_type);

	// truncate current result to required length
	if(outputDataLen < myHmacAPIParam.h_mac_digest_size)
	{
		myHmacAPIParam.h_mac_digest_size = outputDataLen;
	}
	memCopyNoAtomic(outputData, MEMtPM8(workSpace), myHmacAPIParam.h_mac_digest_size);
	return myHmacAPIParam.h_mac_digest_size;
}

#endif    // WITH_HMAC
