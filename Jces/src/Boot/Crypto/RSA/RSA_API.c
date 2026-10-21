/*
 * RSA_API.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */
#include "RSA_API.h"
#include "Crypto_API.h"
#include "MemOperation.h"
#include "Crypto_Constant.h"
#include "SecureCrypto.h"
#include "MemGetReuseRam.h"
#include "SysConfigTarget.h"
#include "RNG_API.h"

#if WITH_RSA

// Hash algorithm identifier.
#if WITH_MD5
static u8 CONST DIGEST_INFO_MD5[18] 		= {0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x04,0x10};
#endif
#if WITH_RIPEMD160
static u8 CONST DIGEST_INFO_RIPEMD160[15] 	= {0x30,0x21,0x30,0x09,0x06,0x05,0x2B,0x24,0x03,0x02,0x01,0x05,0x00,0x04,0x14};
#endif
#if WITH_SHA1
static u8 CONST DIGEST_INFO_SHA1[15]		= {0x30,0x21,0x30,0x09,0x06,0x05,0x2B,0x0E,0x03,0x02,0x1A,0x05,0x00,0x04,0x14};
static u8 CONST SHA_1[20] = {0xda,0x39,0xa3,0xee,0x5e,0x6b,0x4b,0x0d,0x32,0x55,0xbf,0xef,0x95,0x60,0x18,0x90,0xaf,0xd8,0x07,0x09};//added by jiangxiaohui@20220225 
#endif
#if WITH_SHA224
static u8 CONST DIGEST_INFO_SHA224[19] 		= {0x30,0x2d,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x04,0x05,0x00,0x04,0x1c};
static u8 CONST SHA_224[28] = {0xd1,0x4a,0x02,0x8c,0x2a,0x3a,0x2b,0xc9,0x47,0x61,0x02,0xbb,0x28,0x82,0x34,0xc4,0x15,0xa2,0xb0,0x1f,0x82,0x8e,0xa6,0x2a,0xc5,0xb3,0xe4,0x2f};//added by jiangxiaohui@20220225 
#endif
#if WITH_SHA256
static u8 CONST DIGEST_INFO_SHA256[19] 		= {0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20};
static u8 CONST SHA_256[32] = {0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24,0x27,0xae,0x41,0xe4,0x64,0x9b,0x93,0x4c,0xa4,0x95,0x99,0x1b,0x78,0x52,0xb8,0x55};//added by jiangxiaohui@20220225 
#endif
#if WITH_SHA384
static u8 CONST DIGEST_INFO_SHA384[19] 		= {0x30,0x41,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,0x05,0x00,0x04,0x30};
static u8 CONST SHA_384[48] = {0x38,0xb0,0x60,0xa7,0x51,0xac,0x96,0x38,0x4c,0xd9,0x32,0x7e,0xb1,0xb1,0xe3,0x6a,0x21,0xfd,0xb7,0x11,0x14,0xbe,0x07,0x43,0x4c,0x0c,\
									0xc7,0xbf,0x63,0xf6,0xe1,0xda,0x27,0x4e,0xde,0xbf,0xe7,0x6f,0x65,0xfb,0xd5,0x1a,0xd2,0xf1,0x48,0x98,0xb9,0x5b};//added by jiangxiaohui@20220225 
#endif
#if WITH_SHA512
static u8 CONST DIGEST_INFO_SHA512[19] 		= {0x30,0x51,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,0x04,0x40};
static u8 CONST SHA_512[64] = {0xcf,0x83,0xe1,0x35,0x7e,0xef,0xb8,0xbd,0xf1,0x54,0x28,0x50,0xd6,0x6d,0x80,0x07,0xd6,0x20,0xe4,0x05,0x0b,0x57,0x15,0xdc,0x83,0xf4,0xa9,0x21,0xd3,0x6c,0xe9,0xce,\
									0x47,0xd0,0xd1,0x3c,0x5d,0x85,0xf2,0xb0,0xff,0x83,0x18,0xd2,0x87,0x7e,0xec,0x2f,0x63,0xb9,0x31,0xbd,0x47,0x41,0x7a,0x81,0xa5,0x38,0x32,0x7a,0xf9,0x27,0xda,0x3e};//added by jiangxiaohui@20220225 
#endif

#if WITH_RSA_KEY_GEN
static u8 CONST RSAPublicExp[3] 			= {0x01, 0x00, 0x01};
#endif

void RSA_storeMEKey(PM8 pbExp, PM8 pbModulusN, u16 wExpLength, u16 wModulusNLength, u8 bRSAKeyType)
{
	g_RSAKeyStore.pbExp 			= pbExp;
	g_RSAKeyStore.pbModulusN 		= pbModulusN;
	g_RSAKeyStore.wExpLength 		= wExpLength;
	g_RSAKeyStore.wModulusNLength 	= wModulusNLength;
	g_RSAKeyStore.bRSAKeyType 		= bRSAKeyType;
	// Clear cache
	memRefresh();
}



void RSA_storeCRTKey(PM8 pbParmP, PM8 pbParmQ, PM8 pbParmDP1, PM8 pbParmDQ1, PM8 pbParmInvQ, u16 wModulusNLength, u16 wPrimeLength, u8 bRSAKeyType)
{
	g_RSAKeyStore.pbParmP 			= pbParmP;
	g_RSAKeyStore.pbParmQ 			= pbParmQ;
	g_RSAKeyStore.pbParmDP1 		= pbParmDP1;
	g_RSAKeyStore.pbParmDQ1 		= pbParmDQ1;
	g_RSAKeyStore.pbParmInvQ 		= pbParmInvQ;
	g_RSAKeyStore.wModulusNLength 	= wModulusNLength;
	g_RSAKeyStore.wPrimeLength		= wPrimeLength;
	g_RSAKeyStore.bRSAKeyType 		= bRSAKeyType;
	
	RSA_storeKey_CRTNE(PM8_A_U16(pbParmInvQ, wPrimeLength + 2), (wPrimeLength << 1), RSA_CRT_ME_KEY_CAL);

	memRefresh();		// Clear cache
}

void RSA_storeKEYGENCRTKey(PM8 pbParmP, PM8 pbParmQ, PM8 pbParmDP1, PM8 pbParmDQ1, PM8 pbParmInvQ, u16 primeLen)  SMALL_MODE  
{
	g_RSAKeyStore.pbParmP 			= pbParmP;
	g_RSAKeyStore.pbParmQ 			= pbParmQ;
	g_RSAKeyStore.pbParmDP1 		= pbParmDP1;
	g_RSAKeyStore.pbParmDQ1 		= pbParmDQ1;
	g_RSAKeyStore.pbParmInvQ 		= pbParmInvQ;
	
	RSA_storeKey_CRTNE(PM8_A_U16(pbParmInvQ, primeLen + 2), (primeLen << 1), RSA_CRT_ME_KEY_GEN);
	// Clear cache
	memRefresh();
}

// use g_RSAKeyStore.pbParmP to store the private key reference
// the modulus address of private key
void RSA_storeKEYGENPriKey(PM8 pbModulusN, PM8 pbParmD, u16 primeLen)
{
	g_RSAKeyStore.pbParmP 			= pbModulusN;
	g_RSAKeyStore.pbParmQ 			= pbParmD;
	// Clear cache
	memRefresh();
}

/**********************************************************************************
 * Function name: PKCS1PaddingEngine
 * Description: This function pads or depads the buffer according PKCS1.
 * input parameter:
 *      u8 keyType              the type of RSA key, public, plain private or crt private.
 *      u16 keyLen              byte length of RSA private key.
 *      u8 pkcs1Mode		    the mode pad or depad.
 * output parameter:
 *      The RSA Processing Result Code.
 * NOTE:
 **********************************************************************************/
u8 PKCS1PaddingEngine(u8 keyType, u16 keyLen, u8 pkcs1Mode)		SMALL_MODE
{
	PX8		X_A			rsaBuff;
	u16 	D_A 		tmpLen;
	u16 	D_A 		rsaBuffLen;
	u8 		D_A			status;
	u8 		D_A 		tmpByte;

	PATCH_BOOT_RSA_API_001();
	status 		= RSA_PROCESS_OK;
	rsaBuff 	= g_CryptoData.pbBuffer;
	rsaBuffLen 	= g_CryptoData.wBufferLen;
	// PKCS1 padding format: EB = 00 | PKCS1_RSA_PRIV_KEY_TAG_PAD or PKCS1_RSA_PUB_KEY_TAG_PAD | PS | 00 | Data |
	switch (pkcs1Mode)
	{
	case PKCS1_PAD:
		tmpLen 	= U16_S_U8(U16_S_U16(keyLen, rsaBuffLen), 3);// length of bytes needing to be padded.
		PXB_W_U8(rsaBuff, 0x00);
		PX8_INC (rsaBuff);
		switch (keyType)
		{
		case TYPE_RSA_PUBLIC:
			PXB_W_U8(rsaBuff, PKCS1_RSA_PUB_KEY_TAG_PAD);  // Add the public key type byte 0x02.
			PX8_INC (rsaBuff);

			SC_checkRandamReturnCode(Random_number_generator(MEMtPX8(rsaBuff), tmpLen , ALG_SECURE_RANDOM));
			
			// Checking if 0x00 exists in the generated random byte, if exist, replace it with PKCS1_ZERO_PAD_BYTE_REPACE.
			while (0x00 != tmpLen)
			{
				if (0x00 == PXB_R_U8(rsaBuff))
				{
					PXB_W_U8(rsaBuff, PKCS1_ZERO_PAD_BYTE_REPACE);
				}
				PX8_INC (rsaBuff);
				U16_DEC(tmpLen);
			}
			PXB_W_U8(rsaBuff, 0x00);  // Add the border byte 0x00.
			break;
		default:  // TYPE_RSA_PRIVATE or TYPE_RSA_CRT_PRIVATE
			PXB_W_U8(rsaBuff, PKCS1_RSA_PRIV_KEY_TAG_PAD); // Add the private key type byte = 0x01.
			PX8_INC (rsaBuff);
			ecpMem_Fill(rsaBuff, PKCS1_PAD_BYTE_FOR_PRIV_KEY, tmpLen);
			rsaBuff[tmpLen] = 0x00; // Add the border byte 0x00.
			break;
		}
		g_CryptoData.wBufferLen = keyLen;
		break;
	case PKCS1_DEPAD:
		if (0x00 != PXB_R_U8(rsaBuff))
		{
			return RSA_WRONG_PADDING;
		}

		// Check if second byte equals to 0x01(Private Key) or 0x02(Public Key).
		PX8_INC (rsaBuff);
		tmpByte = PXB_R_U8(rsaBuff);

		PX8_INC (rsaBuff);
		tmpLen = U16_S_U8(keyLen, 2);
		switch (keyType)
		{
		case TYPE_RSA_PUBLIC:
			if (PKCS1_RSA_PRIV_KEY_TAG_PAD != tmpByte)
			{
				return RSA_WRONG_PADDING;
			}
			// Find the border byte and check if the padding bytes all equal to PKCS1_PAD_BYTE_FOR_PRIV_KEY.
			while (0x00 != tmpLen)
			{
				tmpByte = PXB_R_U8(rsaBuff);
				if (PKCS1_PAD_BYTE_FOR_PRIV_KEY != tmpByte)
				{
					if (0x00 != tmpByte)
					{
						return RSA_WRONG_PADDING;
					}
					break;
				}

				PX8_INC (rsaBuff);
				U16_DEC(tmpLen);
			}
			break;
		default:  // plain private or crt private key
			if (PKCS1_RSA_PUB_KEY_TAG_PAD != tmpByte)
			{
				return RSA_WRONG_PADDING;
			}
			// Find the border byte
			while (0x00 != tmpLen)
			{
				if (0x00 == PXB_R_U8(rsaBuff))
				{
					break;
				}

				PX8_INC (rsaBuff);
				U16_DEC(tmpLen);
			}
			break;
		}

		if (0x00 != tmpLen)  // Border byte found
		{
			PX8_INC (rsaBuff);
			g_CryptoData.pbBuffer = rsaBuff;  // Record the address for hash value.
			U16_DEC(tmpLen);
			g_CryptoData.wBufferLen = tmpLen;  // Record the length for hash value.
		}
		else
		{
			status = RSA_WRONG_PADDING;
		}
		break;
	default:
		faultDetected();
		break;
	}
	return status;
}

/**********************************************************************************
 * Function name: ISO9796PaddingEngine
 * Description: This function pads or depads the buffer according ISO9796.
 * input parameter:
 *      u16 keyLen                  byte length of RSA private key.
 *      u8 iso9796Mode				the mode pad or depad.
 * output parameter:
 *      The RSA Processing Result Code.
 * NOTE:
 **********************************************************************************/
u8 ISO9796PaddingEngine(u16 keyLen, u8 iso9796Mode)
{
	PX8		D_A		rsaBuff;
	u16 	D_A 	tmpLen;
	u16 	D_A 	rsaBuffLen;
	PATCH_BOOT_RSA_API_002();
	rsaBuff = g_CryptoData.pbBuffer;
	rsaBuffLen = g_CryptoData.wBufferLen;

	// ISO9796 padding format: EB = 6A or 4A or 4B | BB...BB | BA | Data + 'BC' |
	switch (iso9796Mode)
	{
	case ISO9796_PAD:

		// Add the first byte "0x6A", "0x4A", or "0x4B"
		if (rsaBuffLen == keyLen)
		{
			PXB_W_U8(rsaBuff, 0x6A);
		}
		else if (rsaBuffLen == (U16_S_U8(keyLen,1)))
		{
			PXB_W_U8(rsaBuff, 0x4A);
		}
		else
		{
			PXB_W_U8(rsaBuff, 0x4B);
			PX8_INC (rsaBuff);

			// Add padding byte "0xBB"
			tmpLen = U16_S_U8((U16_S_U16(keyLen, rsaBuffLen)), 1);
			while (0x00 != tmpLen)
			{
				PXB_W_U8(rsaBuff, 0xBB);
				PX8_INC (rsaBuff);
				U16_DEC(tmpLen);
			}
			PXB_W_U8(rsaBuff, 0xBA);
		}
		g_CryptoData.wBufferLen = keyLen;
		break;
	case ISO9796_DEPAD:
		if (0xBC != PXB_R_U8(PX8_S_U8((PX8_A_U16(rsaBuff,keyLen)),1)))
		{
			return RSA_WRONG_PADDING;
		}

		if (0x6A == (PXB_R_U8(rsaBuff)) || (0x4A == PXB_R_U8(rsaBuff)))
		{
			g_CryptoData.wBufferLen = U16_S_U8(g_CryptoData.wBufferLen, 2);  // the result length.
			PX8_INC (rsaBuff);
			g_CryptoData.pbBuffer = rsaBuff;  // pointer to the reslut.
		}
		else if ((0x4B == PXB_R_U8(rsaBuff)))
		{
			PX8_INC (rsaBuff);

			// Find the border byte 0xBA
			for (tmpLen = 0; tmpLen < keyLen; U16_INC(tmpLen))
			{
				if ((0xBB != PXB_R_U8(rsaBuff)))
				{
					if (0xBA != (PXB_R_U8(rsaBuff)))
					{
						return RSA_WRONG_PADDING;
					}
					PX8_INC (rsaBuff);
					break;
				}
				PX8_INC (rsaBuff);
			}

			// the value of loopCounter indicates the number of "BB", then delete "4B" "BA" "BC" from loopCounter.
			g_CryptoData.wBufferLen = U16_S_U8(U16_S_U16(keyLen, tmpLen), 3);  // the result length.
			g_CryptoData.pbBuffer = rsaBuff;  // pointer to the reslut.
		}
		else
		{
			return RSA_WRONG_PADDING;
		}
		break;
	default:
		faultDetected();
		break;
	}
	return RSA_PROCESS_OK;
}

/**
 * Generate and apply the MGF1 operation (from PKCS#1 v2.1) to a buffer.
 *
 * \param dst       buffer to mask
 * \param dst_len   ength of destination buffer
 * \param src       source of the mask generation
 * \param src_len   length of the source buffer
 * \param md_ctx    message digest context to use
 * modify by jiangxiaohui@20220525
 */
static void mgf_mask(PX8 dst, u16 dst_len, PX8 src, u16 src_len, u8 hashType)
{
	u8    X_A		maskIn[MAX_RSA_MUD_BYTE_LEN + 4];     // hash || counter
	u8    X_A		maskHash[HASH_ICV_SIZE];     // hash || counter
	PX8   X_A       pCounter;
	PX8   X_A       p;
	u32   X_A       i;
	u32   X_A       mask_len;
	u32   X_A       hash_len;
	u32   X_A       maskHashCount;

	p = dst;

	hash_len = src_len;

	pCounter = maskIn + src_len;
	maskHashCount = 0;
	memCopyToRam(maskIn, src, src_len);

	while(dst_len > 0)
	{
        PX_W_U32(pCounter, maskHashCount);
		HASHDoFinal((PM8)maskIn, src_len + 4, maskHash, HASH_INIT, NULL, NULL, hashType); /*将生成的hash数据单独使用空间存放，避免数据过长导致参与hash计算的数据被覆盖*/
		if(dst_len < hash_len)
		{
			 mask_len = dst_len;
		}
		else
		{
			mask_len = hash_len;
		}
        for(i = 0; i < mask_len; i++)
        {
            *p++ ^= maskHash[i];
        }
        dst_len -= mask_len;
        maskHashCount =  PX_R_U32(pCounter);
        maskHashCount++;
	}
}



/**
 * mplementation of the PKCS#1 v2.1 RSASSA-PSS-SIGN function
 *
 * \param keyType   key Type
 * \param keyLen    key lenght
 * \param pkcs1Mode the mode pad or depad.
 * \param src_len   length of the source buffer
 * \param md_ctx    message digest context to use
 * modify by jiangxiaohui@20220525: 将salt[HASH_ICV_SIZE]修改为salt[SHA512_BLOCK_SIZE]
 */
u8 PKCS1_PSS_PaddingEngine(u16 keyLen, u8 pkcs1Mode, u8 hashType, u8 hashlen)		SMALL_MODE
{
	u8			X_A         salt[HASH_ICV_SIZE];
	PX8			X_A			rsaBuff;
	u8 			X_A			status;
	u16     	 X_A         rsaBuffLen;
	u16      	X_A         pslen;
	u16      	X_A         dblen;
	u16      	X_A         i;


	status 		= RSA_PROCESS_OK;
	rsaBuff 	= g_CryptoData.pbBuffer;
	rsaBuffLen 	= g_CryptoData.wBufferLen;

	if(keyLen < (2*hashlen + 2) )
	{
//		faultDetected();
	}

	pslen       = keyLen - 2*hashlen - 2 ;
	dblen       = keyLen - hashlen - 1 ;

	if(pkcs1Mode == PKCS1_PSS_PAD)
	{
		//ecpMem_Set(salt,0,hashlen);  //salt set zero for test
		Random_number_generator(salt,hashlen,true);


		//step 1: get (M') = 0000000000000000 || mHash || salt
		ecpMem_Set(rsaBuff,0,8);                               //pad;
														   //mHash have calc in RSAPreSign()
		ecpMem_Copy(&rsaBuff[hashlen + 8], salt, hashlen);      //set salt
		HASHDoFinal((PM8)rsaBuff, (2*hashlen + 8), &rsaBuff[keyLen-hashlen-1], HASH_INIT, NULL, NULL, hashType);

		//step 2: get DB = PS || 0x01 || salt1
		ecpMem_Set(rsaBuff,0,pslen);                                //set PS all 0
		rsaBuff[pslen] = 0x01;                                  //pad 0x01
		ecpMem_Copy(&rsaBuff[pslen + 1], salt, hashlen);             //set salt

		//step 3: mask DB
		mgf_mask(rsaBuff, dblen, &rsaBuff[dblen], hashlen, hashType );
		 /*at cn_pkcs_1 v2.1 9.1 EMSA-PSS Set the leftmost 8emLen - emBits bits of the leftmost octet in maskedDB to zero.*/
		rsaBuff[0] = rsaBuff[0] & 0x7F ;

		//step 4: pad 0xBC
		rsaBuff[keyLen-1] = 0xBC;

		// EM = maskDB || M' || 0xBC
		g_CryptoData.wBufferLen = keyLen;
	}
	else if(pkcs1Mode == PKCS1_PSS_DEPAD)
	{
		if(rsaBuffLen != keyLen)
		{
			return RSA_WRONG_PADDING;
		}
		if( ((rsaBuff[0] & 0x80) == 0x80) || (rsaBuff[keyLen-1]  != 0xBC) )
		{
			return RSA_WRONG_PADDING;
		}
		//maskDB -> DB
		mgf_mask(rsaBuff, dblen, &rsaBuff[dblen], hashlen, hashType );
		 /*at cn_pkcs_1 v2.1 9.1 EMSA-PSS Set the leftmost 8emLen - emBits bits of the leftmost octet in maskedDB to zero.*/
		rsaBuff[0] = rsaBuff[0] & 0x7F ;

		//check 0x01 pad
		if(rsaBuff[pslen] != 0x01)
		{
			return RSA_WRONG_PADDING;
		}
		//check DB PS
		for(i=0; i< pslen; i++)
		{
			if (0 != *(PX8)(&rsaBuff[i]))
			{
				return RSA_WRONG_PADDING;
			}
		}
		//return 0000000000000000 || M' || salt
		for(i=0; i<hashlen; i++)
		{
			rsaBuff[hashlen + 8 + i] = rsaBuff[pslen + 1 + i];
			rsaBuff[8 + i] =  rsaBuff[dblen + i];        //M'
		}
		g_CryptoData.pbBuffer = rsaBuff;
		g_CryptoData.wBufferLen = 2*hashlen + 8;
	}
	else
	{
		faultDetected();
	}

	return status;
}

#if WITH_PKCS1_OAEP_PADDING
/*
 * name:      PKCS1_OAEP_PaddingEngine
 * function:  将数据使用OAEP填充方式填充到key的长度
 * param：
 * u16 keyLen:    key的长度
 * u8 pkcs1Mode：   填充或者解填充方式
 * u8 hashType：     hash算法的类型
 * u8 hashlen :   hash算法的输出长度
 * added by jiangxiaohui@20220525
 */
u8 PKCS1_OAEP_PaddingEngine(u16 keyLen, u8 pkcs1Mode, u8 hashType, u8 hashlen)
{

	PX8			X_A			rsaBuff;
	u8 lHash[64]; 
	PX8 seed;
	u8 status;
	u16 i;
	u16 mlen;
	u16 pslen;
	u16 dblen;
	
	status 		= RSA_PROCESS_OK;
	rsaBuff 	= &g_CryptoData.pbBuffer[hashlen + 1];
	seed = &g_CryptoData.pbBuffer[1];
	mlen = g_CryptoData.wBufferLen;
	pslen = keyLen - mlen - 2*hashlen - 2;
	dblen = keyLen - hashlen - 1;
		
	if(pkcs1Mode == PKCS1_OAEP_PAD)
	{	
			
		if(mlen > keyLen - 2*hashlen - 2)
		{
			faultDetected();
		}
		
		switch(hashType)
		{
			case ALG_SHA_1:
				ecpMem_Copy(lHash,SHA_1,hashlen);
				break;
			case ALG_SHA_256:
				ecpMem_Copy(lHash,SHA_256,hashlen);
				break;
			case ALG_SHA_384:
				ecpMem_Copy(lHash,SHA_384,hashlen);
				break;
			case ALG_SHA_512:
				ecpMem_Copy(lHash,SHA_512,hashlen);
				break;
			case ALG_SHA_224:
				ecpMem_Copy(lHash,SHA_224,hashlen);
				break;
		}
		//db = lHash||PS||0x01||M
		ecpMem_Copy(rsaBuff,lHash,hashlen);
		ecpMem_Set(&rsaBuff[hashlen],0,pslen);
		rsaBuff[hashlen + pslen] = 0x01;
	//generate a random string seed of length hlen;
		Random_number_generator(seed,hashlen,true);
	//maskdb
		mgf_mask(rsaBuff,dblen,seed,hashlen,hashType);
	//maskseed
		mgf_mask(seed,hashlen,rsaBuff,dblen,hashType);
	//EM = 0x00||maskedseed||maskedDB	
		rsaBuff = g_CryptoData.pbBuffer;
		rsaBuff[0] = 0x00;
		
		g_CryptoData.wBufferLen = keyLen;
	}
	else if(pkcs1Mode == PKCS1_OAEP_DEPAD)
	{
		//seed
		mgf_mask(seed,hashlen,rsaBuff,dblen,hashType);
		//DB
		mgf_mask(rsaBuff,dblen,seed,hashlen,hashType);
		rsaBuff = &rsaBuff[hashlen];
		for(i = 0;i < keyLen - 2*hashlen - 2;i++)
		{
			if(rsaBuff[i] == 0x00)
			{
				continue;
			}
			else if(rsaBuff[i] == 0x01)
			{
				g_CryptoData.pbBuffer = &rsaBuff[i + 1];
				g_CryptoData.wBufferLen = keyLen - 2*hashlen - 2 - i;
				return status;
			}
			else
			{
				faultDetected();
			}
		}
	}
	else
	{
		faultDetected();
	}
	return status;
}
#endif

/**********************************************************************************
 * Function name: DoRSAOpertaion
 * Description: This function does kernel operation using RSA key.
 * input parameter:
 *		u8 algType				RSA alg type.
 *      u16 keyLen              RSA modulus length.
 * output parameter:
 *		The RSA Processing Result Code.
 * NOTE:
 * the structure of algType for cipher is as follows
 *  ----------------------------------------------------------------------
 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
 *  |           6bit  5bit            |        4bit 3bit 2bit 1bit       |
 *  ----------------------------------------------------------------------
 * the structure of algType for signature is as follows
 *  ----------------------------------------------------------------------
 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
 *  |           8bit  7bit            |   6bit 5bit 4bit 3bit 2bit 1bit  |
 *  ----------------------------------------------------------------------
 * modify by jiangxiaohui@20220525: 增加OAEP算法，并且用开关隔离
 **********************************************************************************/
u8 DoRSAOpertaion(u8 algType, u16 keyLen)	SMALL_MODE
{
	u8				D_A		status;
	u8				D_A		rsaKeyType;

#if WITH_SECURE_DATA_STORAGE
	#if	WITH_SAFETY_REUSED_BUFFER
	PX8				X_A		enCryKeyBuf;
	enCryKeyBuf				= MEMtPX8(MemGetReuseRam_GetFromBotBase(MAX_RSA_MUD_BYTE_LEN));
	#else
	u8 				X_A		enCryKeyBuf[MAX_RSA_MUD_BYTE_LEN];
	#endif //WITH_SAFETY_REUSED_BUFFER
#endif	//WITH_SECURE_DATA_STORAGE

	PATCH_BOOT_RSA_API_003();
	
	status 					= RSA_PROCESS_OK;
	rsaKeyType 				= g_RSAKeyStore.bRSAKeyType;
	switch (algType)
	{
	case PKCS1_CIPHER_ENCRYPTION:
	case PKCS1_SHA1_SIGN:
	case PKCS1_MD5_SIGN:
	case PKCS1_RIPEMD160_SIGN:
	case PKCS1_SHA224_SIGN:
	case PKCS1_SHA256_SIGN:
	case PKCS1_SHA384_SIGN:
	case PKCS1_SHA512_SIGN:
		PKCS1PaddingEngine(rsaKeyType, keyLen, PKCS1_PAD);
		break;
	case ISO9796_SHA1_SIGN:
	case ISO9796_RIPEMD160_SIGN:
		ISO9796PaddingEngine(keyLen, ISO9796_PAD);
		break;
#if WITH_PKCS1_PSS_PADDING
	case PKCS1_PSS_SHA1_SIGN:
		PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_PAD, ALG_SHA_1,SHA1_DIGEST_SIZE );
		break;
	case PKCS1_PSS_SHA224_SIGN:
		PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_PAD, ALG_SHA_224,SHA224_DIGEST_SIZE );
		break;
	case PKCS1_PSS_SHA256_SIGN:
		PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_PAD, ALG_SHA_256,SHA256_DIGEST_SIZE );
		break;
	case PKCS1_PSS_SHA384_SIGN:
		PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_PAD, ALG_SHA_384,SHA384_DIGEST_SIZE );
		break;
	case PKCS1_PSS_SHA512_SIGN:
		PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_PAD, ALG_SHA_512,SHA512_DIGEST_SIZE );
		break;
#endif
#if WITH_PKCS1_OAEP_PADDING
	case PKCS1_OAEP_SHA1_SIGN:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_PAD,ALG_SHA_1,SHA1_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA224_SIGN:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_PAD,ALG_SHA_224,SHA224_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA256_SIGN:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_PAD,ALG_SHA_256,SHA256_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA384_SIGN:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_PAD,ALG_SHA_384,SHA384_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA512_SIGN:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_PAD,ALG_SHA_512,SHA512_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_CIPHER_ENCRYPTION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_PAD,ALG_SHA_1,SHA1_DIGEST_SIZE);
		break;
#endif
	default:
		break;
	}
	
    memRefresh();

	SC_PKIGetAllKey(enCryKeyBuf, rsaKeyType);

	switch (rsaKeyType)
	{
	case TYPE_RSA_PUBLIC:
	case TYPE_RSA_PRIVATE:
		SC_checkReturnCode(RSASTDCore());
		break;
	case TYPE_RSA_CRT_PRIVATE:
		SC_checkReturnCode(RSACRTCore());
		break;
	default:
		faultDetected();
		break;
	}

	switch (algType)
	{
	case PKCS1_CIPHER_DECRYPTION:
	case PKCS1_SHA1_VERIFICATION:
	case PKCS1_MD5_VERIFICATION:
	case PKCS1_RIPEMD160_VERIFICATION:
	case PKCS1_SHA224_VERIFICATION:
	case PKCS1_SHA256_VERIFICATION:
	case PKCS1_SHA384_VERIFICATION:
	case PKCS1_SHA512_VERIFICATION:
		status = PKCS1PaddingEngine(rsaKeyType, keyLen, PKCS1_DEPAD);
		break;
	case ISO9796_SHA1_VERIFICATION:
	case ISO9796_RIPEMD160_VERIFICATION:
		status = ISO9796PaddingEngine(keyLen, ISO9796_DEPAD);
		break;
	case PKCS1_PSS_SHA1_VERIFICATION:
		status = PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_DEPAD, ALG_SHA_1, SHA1_DIGEST_SIZE);
		break;
	case PKCS1_PSS_SHA224_VERIFICATION:
		status = PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_DEPAD, ALG_SHA_224, SHA224_DIGEST_SIZE);
		break;
	case PKCS1_PSS_SHA256_VERIFICATION:
		status = PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_DEPAD, ALG_SHA_256, SHA256_DIGEST_SIZE);
		break;
	case PKCS1_PSS_SHA384_VERIFICATION:
		status = PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_DEPAD, ALG_SHA_384, SHA384_DIGEST_SIZE);
		break;
	case PKCS1_PSS_SHA512_VERIFICATION:
		status = PKCS1_PSS_PaddingEngine(keyLen,PKCS1_PSS_DEPAD, ALG_SHA_512, SHA512_DIGEST_SIZE);
		break;
#if WITH_PKCS1_OAEP_PADDING
	case PKCS1_OAEP_SHA1_VERIFICATION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_DEPAD,ALG_SHA_1,SHA1_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA224_VERIFICATION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_DEPAD,ALG_SHA_224,SHA224_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA256_VERIFICATION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_DEPAD,ALG_SHA_256,SHA256_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA384_VERIFICATION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_DEPAD,ALG_SHA_384,SHA384_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_SHA512_VERIFICATION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_DEPAD,ALG_SHA_512,SHA512_DIGEST_SIZE);
		break;
	case PKCS1_OAEP_CIPHER_DECRYPTION:
		PKCS1_OAEP_PaddingEngine(keyLen,PKCS1_OAEP_DEPAD,ALG_SHA_1,SHA1_DIGEST_SIZE);
		break;
	default:
		break;
#endif
	}
	return status;
}

/**********************************************************************************
 * Function name: GenRSAKeyPair
 * Description: This function generates the required RSA private key and public key
 *              using parameters input.
 * input parameter:
 *		u16 wRSAKeyByteLen			byte length of RSA private key
 *		u8 bKeyType				    RSA key type, STD or CRT
 * output parameter:
 *		none.
 * NOTE:
 **********************************************************************************/

#if WITH_RSA_KEY_GEN

#if !RSA_KEYGEN_WIRTE_IN_HAL
static void RSAKeyGenStoreKey(PX8 keytempbuff, PM8 keyData, u16 keyOffs, PM8 inBuffer, u16 keyLen, u8 keyType, u16 realKeyLen, u8 mode, PM8 realKeyData, u8 storeKeyType)
{
	if(RSA_C_NATIVE_GEN_KEY == storeKeyType)
	{
		memCopyNoAtomic(realKeyData, inBuffer, realKeyLen);
	}
	else
	{
//		SC_PKIsetKey(keytempbuff, keyData, keyOffs, inBuffer, keyLen, TYPE_RSA_PRIVATE, realKeyLen, mode);
		SC_PKIsetKey(keytempbuff, keyData, keyOffs, inBuffer, keyLen, keyType, realKeyLen, mode);
	}
}
#endif

void RSAKeyGenCore(u8 storeKeyType)
{
#if	(RSA_USE_CRYPTO_RAM | SEC_MEM_OPERATION)
	u8 				X_A		tempBuffer[MAX_RSA_MUD_BYTE_LEN];
#else
	//#define			tempBuffer			MEMtPX8(0) //此定义  访问tempBuffer有问题    memCopyNoAtomic(MEMtPM8(&tempBuffer[MAX_RSA_E_BYTE_LEN - g_RSAKeyStore.wExpLength]), g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
	u8 				X_A		tempBuffer[MAX_RSA_MUD_BYTE_LEN];
#endif

	RSAKeyGenParam	I_A 	myRsaKeyRef;

#if !RSA_KEYGEN_WIRTE_IN_HAL
	u16 			I_A		wRSAKeyByteLen;
	u8 				I_A		bPrimeLen;
#endif
	
	PATCH_BOOT_RSA_API_004();
	


	if(0x00 == g_RSAKeyStore.wExpLength)
	{
		// Use the default key value.
		memCopyNoAtomic(g_RSAKeyStore.pbExp, MEMtPM8(RSAPublicExp), 3);
		g_RSAKeyStore.wExpLength = 3;
	}

    memRefresh();
	
	GenRSAKeyPair(&myRsaKeyRef);
	
#if !RSA_KEYGEN_WIRTE_IN_HAL  //wzq: 此处不同于主干，由于ram空间限制，P60平台选择直接在hal层将生成的密钥数据拷贝到EEPROM。
	wRSAKeyByteLen 			= g_RSAKeyStore.wModulusNLength;
	bPrimeLen 				= U16t8(U16_SHR_U8(wRSAKeyByteLen, 1));
#if	RSA_USE_CRYPTO_RAM
	CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbModulusN, wRSAKeyByteLen);
	// store the modulus to the destination area
//	memCopyNoAtomic(g_RSAKeyStore.pbModulusN, tempBuffer, wRSAKeyByteLen);
	RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbModulusN, 0x00, tempBuffer, wRSAKeyByteLen, TYPE_RSA_PUBLIC, wRSAKeyByteLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbModulusN, storeKeyType);
#else
//    memCopyNoAtomic(g_RSAKeyStore.pbModulusN, myRsaKeyRef.pbModulusN, wRSAKeyByteLen);
	RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbModulusN, 0x00, myRsaKeyRef.pbModulusN, wRSAKeyByteLen, TYPE_RSA_PUBLIC, wRSAKeyByteLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbModulusN, storeKeyType);
#endif

	switch(g_RSAKeyStore.bRSAKeyType)
	{
		case ALG_RSA:
#if	RSA_USE_CRYPTO_RAM
			// set private key n.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmP, tempBuffer, wRSAKeyByteLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, 0x00, tempBuffer, wRSAKeyByteLen, TYPE_RSA_PRIVATE ,wRSAKeyByteLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmP, storeKeyType);
			// set private key d.
			CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbParmP, wRSAKeyByteLen);
//			memCopyNoAtomic(g_RSAKeyStore.pbParmQ, tempBuffer, wRSAKeyByteLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, wRSAKeyByteLen, tempBuffer, wRSAKeyByteLen, TYPE_RSA_PRIVATE, wRSAKeyByteLen, SC_PKI_IN_ENCRY_TEXT, g_RSAKeyStore.pbParmQ, storeKeyType);
#else
			// set private key n.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmP, g_RSAKeyStore.pbModulusN, wRSAKeyByteLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, 0x00, g_RSAKeyStore.pbModulusN, wRSAKeyByteLen, TYPE_RSA_PRIVATE, wRSAKeyByteLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmP, storeKeyType);
			// set private key d.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmQ, myRsaKeyRef.pbParmP, wRSAKeyByteLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, wRSAKeyByteLen, myRsaKeyRef.pbParmP, wRSAKeyByteLen, TYPE_RSA_PRIVATE, wRSAKeyByteLen, SC_PKI_IN_ENCRY_TEXT, g_RSAKeyStore.pbParmQ, storeKeyType);
#endif
			break;
		case ALG_RSA_CRT:
#if	RSA_USE_CRYPTO_RAM
			// set private key P.
			CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbParmP, bPrimeLen);
//			memCopyNoAtomic( g_RSAKeyStore.pbParmP, tempBuffer, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, 0x00, tempBuffer, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_ENCRY_TEXT, g_RSAKeyStore.pbParmP, storeKeyType);
			// set private key Q.
			CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbParmQ, bPrimeLen);
//			memCopyNoAtomic( g_RSAKeyStore.pbParmQ, tempBuffer, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, bPrimeLen, tempBuffer, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_ENCRY_TEXT, g_RSAKeyStore.pbParmQ, storeKeyType);
			// set private key dP.
			CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbParmDP, bPrimeLen);
//			memCopyNoAtomic( g_RSAKeyStore.pbParmDP1, tempBuffer, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, U8_M_U8(bPrimeLen, 2), tempBuffer, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmDP1, storeKeyType);
			// set private key dQ.
			CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbParmDQ, bPrimeLen);
//			memCopyNoAtomic( g_RSAKeyStore.pbParmDQ1, tempBuffer, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, U8_M_U8(bPrimeLen, 3), tempBuffer, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmDQ1, storeKeyType);
			// set private key InvQ.
			CryptoRAMCopyToUserRAM(tempBuffer, myRsaKeyRef.pbParmInvQ, bPrimeLen);
//			memCopyNoAtomic( g_RSAKeyStore.pbParmInvQ, tempBuffer, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, U8_M_U8(bPrimeLen, 4), tempBuffer, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmInvQ, storeKeyType);
#else
			// set private key P.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmP, myRsaKeyRef.pbParmP, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, 0x00, myRsaKeyRef.pbParmP, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_ENCRY_TEXT, g_RSAKeyStore.pbParmP, storeKeyType);
			// set private key Q.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmQ, myRsaKeyRef.pbParmQ, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, bPrimeLen, myRsaKeyRef.pbParmQ, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_ENCRY_TEXT, g_RSAKeyStore.pbParmQ, storeKeyType);
			// set private key dP.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmDP1, myRsaKeyRef.pbParmDP, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, U8_M_U8(bPrimeLen, 2), myRsaKeyRef.pbParmDP, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmDP1, storeKeyType);
			// set private key dQ.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmDQ1, myRsaKeyRef.pbParmDQ, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, U8_M_U8(bPrimeLen, 3), myRsaKeyRef.pbParmDQ, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmDQ1, storeKeyType);
			// set private key InvQ.
//			memCopyNoAtomic(g_RSAKeyStore.pbParmInvQ, myRsaKeyRef.pbParmInvQ, bPrimeLen);
			RSAKeyGenStoreKey(tempBuffer, g_RSAKeyStore.pbParmP, U8_M_U8(bPrimeLen, 4), myRsaKeyRef.pbParmInvQ, bPrimeLen, TYPE_RSA_CRT_PRIVATE, bPrimeLen, SC_PKI_IN_PLAIN_TEXT, g_RSAKeyStore.pbParmInvQ, storeKeyType);
#endif
#if	RSA_CRT_E_SAVE
			ecpMem_Set(tempBuffer, 0x00, MAX_RSA_E_BYTE_LEN);
			if(g_RSAKeyStore.wExpLength <= MAX_RSA_E_BYTE_LEN)
			{
				memCopyNoAtomic(MEMtPM8(&tempBuffer[MAX_RSA_E_BYTE_LEN - g_RSAKeyStore.wExpLength]), g_RSAKeyStore.pbExp, g_RSAKeyStore.wExpLength);
			}
			memCopyNoAtomic(g_RSAKeyGenCRTNEStore.pbCRTExp, tempBuffer, MAX_RSA_E_BYTE_LEN);
#endif	//RSA_CRT_E_SAVE
			break;
			default:
				faultDetected();
				break;
	}
#endif  //!RSA_KEYGEN_WIRTE_IN_HAL  
}


#endif //WITH_RSA_KEY_GEN

// modify by jiangxiaohui@20220525: 增加PKCS1_PSS_SHA384_SIGN和PKCS1_PSS_SHA512_SIGN、PKCS1_PSS_SHA384_VERIFICATION、PKCS1_PSS_SHA512_VERIFICATION
static void HASHAPIParamsInit(RSA_SIGN_API_Param_t * myAPIParam, u8 algType)
{
	switch (algType)
	{
#if WITH_SHA1
	case ISO9796_SHA1_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_1;
		myAPIParam->algType			= ISO9796_SIGN;
		break;
	case PKCS1_SHA1_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_1;
		myAPIParam->algType			= PKCS1_SIGN;
		break;
	case PKCS1_PSS_SHA1_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_1;
		myAPIParam->algType			= PKCS1_PSS_SIGN;
		break;
#endif  // WITH_SHA1

#if WITH_MD5
	case PKCS1_MD5_SIGN:
		myAPIParam->HashAlg			= ALG_MD;
		myAPIParam->algType			= PKCS1_SIGN;
		break;
#endif

#if WITH_RIPEMD160
	case ISO9796_RIPEMD160_SIGN:
		myAPIParam->HashAlg			= ALG_RIPEMD160;
		myAPIParam->algType			= ISO9796_SIGN;
		break;
	case PKCS1_RIPEMD160_SIGN:
		myAPIParam->HashAlg			= ALG_RIPEMD160;
		myAPIParam->algType			= PKCS1_SIGN;
#endif

#if WITH_SHA224
	case PKCS1_SHA224_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_224;
		myAPIParam->algType			= PKCS1_SIGN;
		break;
	case PKCS1_PSS_SHA224_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_224;
		myAPIParam->algType			= PKCS1_PSS_SIGN;
		break;
#endif

#if WITH_SHA256
	case PKCS1_SHA256_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_256;
		myAPIParam->algType			= PKCS1_SIGN;
		break;
	case PKCS1_PSS_SHA256_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_256;
		myAPIParam->algType			= PKCS1_PSS_SIGN;
		break;
#endif

#if WITH_SHA384
	case PKCS1_SHA384_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_384;
		myAPIParam->algType			= PKCS1_SIGN;
		break;
	case PKCS1_PSS_SHA384_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_384;
		myAPIParam->algType			= PKCS1_PSS_SIGN;
		break;
#endif

#if WITH_SHA512
	case PKCS1_SHA512_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_512;
		myAPIParam->algType			= PKCS1_SIGN;
		break;
	case PKCS1_PSS_SHA512_SIGN:
		myAPIParam->HashAlg			= ALG_SHA_512;
		myAPIParam->algType			= PKCS1_PSS_SIGN;
		break;
#endif
#if WITH_SHA1
	case ISO9796_SHA1_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_1;
		myAPIParam->algType			= ISO9796_VERIFICATION;
		break;
	case PKCS1_SHA1_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_1;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
	case PKCS1_PSS_SHA1_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_1;
		myAPIParam->algType			= PKCS1_PSS_VERIFICATION;
		break;
#endif //WITH_SHA1

#if WITH_MD5
	case PKCS1_MD5_VERIFICATION:
		myAPIParam->HashAlg			= ALG_MD;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
#endif

#if WITH_RIPEMD160
	case ISO9796_RIPEMD160_VERIFICATION:
		myAPIParam->HashAlg			= ALG_RIPEMD160;
		myAPIParam->algType			= ISO9796_VERIFICATION;
		break;
	case PKCS1_RIPEMD160_VERIFICATION:
		myAPIParam->HashAlg			= ALG_RIPEMD160;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
#endif

#if WITH_SHA224
	case PKCS1_SHA224_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_224;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
	case PKCS1_PSS_SHA224_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_224;
		myAPIParam->algType			= PKCS1_PSS_VERIFICATION;
		break;
#endif

#if WITH_SHA256
	case PKCS1_SHA256_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_256;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
	case PKCS1_PSS_SHA256_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_256;
		myAPIParam->algType			= PKCS1_PSS_VERIFICATION;
		break;
#endif

#if WITH_SHA384
	case PKCS1_SHA384_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_384;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
	case PKCS1_PSS_SHA384_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_384;
		myAPIParam->algType			= PKCS1_PSS_VERIFICATION;
		break;
#endif

#if WITH_SHA512
	case PKCS1_SHA512_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_512;
		myAPIParam->algType			= PKCS1_VERIFICATION;
		break;
	case PKCS1_PSS_SHA512_VERIFICATION:
		myAPIParam->HashAlg			= ALG_SHA_512;
		myAPIParam->algType			= PKCS1_PSS_VERIFICATION;
		break;
#endif
	default:
		faultDetected();
		break;
	}

	switch(myAPIParam->HashAlg)
	{
#if WITH_MD5
	case ALG_MD:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_MD5);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_MD5;
		myAPIParam->digestSize 					= MD_DIGEST_SIZE;
		break;
#endif

#if WITH_RIPEMD160
	case ALG_RIPEMD160:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_RIPEMD160);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_RIPEMD160;
		myAPIParam->digestSize 					= RIPEMD160_DIGEST_SIZE;
		break;
#endif

#if WITH_SHA1
	case ALG_SHA_1:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_SHA1);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_SHA1;
		myAPIParam->digestSize 					= SHA1_DIGEST_SIZE;
		break;
#endif //WITH_SHA1
#if WITH_SHA224
	case ALG_SHA_224:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_SHA224);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_SHA224;
		myAPIParam->digestSize 					= SHA224_DIGEST_SIZE;
		break;
#endif //WITH_SHA224

#if WITH_SHA256
	case ALG_SHA_256:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_SHA256);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_SHA256;
		myAPIParam->digestSize 					= SHA256_DIGEST_SIZE;
		break;
#endif //WITH_SHA256

#if WITH_SHA384
	case ALG_SHA_384:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_SHA384);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_SHA384;
		myAPIParam->digestSize 					= SHA384_DIGEST_SIZE;
		break;
#endif //WITH_SHA384

#if WITH_SHA512
	case ALG_SHA_512:
		myAPIParam->pDigestInfo 				= MEMtPM8(DIGEST_INFO_SHA512);
		myAPIParam->pDigestInfoLen				= LENGTH_DIGESTINFO_SHA512;
		myAPIParam->digestSize 					= SHA512_DIGEST_SIZE;
		break;
#endif //WITH_SHA512
		default:
		break;
	}
	return;
}





/**********************************************************************************
 * Function name: RSAPreSign
 * Description: This function prepare a message before RSA signature.
 * input parameter:
 * 		MEM_U8_P inBuff             The source data address.
 * 		u16 inLen                   The length of source data.
 * 		MEM_U8_P tmpSrcBuff         A buffer for storing plain text(just for ISO9796 padding).
 * 		u16 tmpBuffLen              The length of pbSrcTempBuffer, max length is the private key length - 21.
 * 		MEM_U8_P mdBuff             md information.
 *      MEM_U8_P tmpBuff			md temp data.
 *      u8 algType					algorithm and mode.
 * 		u16 keyLen					key length.
 * output parameter:
 *		none.
 * NOTE:
 * the structure of algType is as follows
 *  ----------------------------------------------------------------------
 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
 *  |           8bit  7bit            |   6bit 5bit 4bit 3bit 2bit 1bit  |
 *  ----------------------------------------------------------------------
 * modify by jiangxiaohui@20220525: 增加PKCS1_OAEP_SIGN
 **********************************************************************************/
void RSAPreSign(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 tmpBuffLen, PM8 mdBuff, PM8 tmpBuff, u8 algType, u16 keyLen,u16 sigLen)		SMALL_MODE
{
	RSA_SIGN_API_Param_t myRsaApiParam;
	PX8 	X_A 	rsaBuff;
	u16	    D_A	    inputLenSum;
	u16	    D_A     tmpLen;

	PATCH_BOOT_RSA_API_005();

	tmpLen = 0;

	inputLenSum = U16_A_U16(inLen, tmpBuffLen);

	if (((U8_AND_U8(algType, RSA_SIGN_ALG_TYPE_MASK)) == ALG_RSA_SHA_ISO9796)
		|| ((U8_AND_U8(algType, RSA_SIGN_ALG_TYPE_MASK)) == ALG_RSA_RIPEMD160_ISO9796))
	{
		// the minimum byte number that must be reserved in ISO9796 padding is 22 bytes,
		// also, the number is just the number to be padded with the source data.
		tmpLen = U16_S_U8(keyLen, 22);

		if (inputLenSum > tmpLen)
		{
			g_CryptoData.wBufferLen = keyLen;
		}
		else if (inputLenSum == tmpLen)
		{
			g_CryptoData.wBufferLen = U16_S_U8(keyLen, 1);
		}
		else
		{
			g_CryptoData.wBufferLen = U8_A_U16(22, inputLenSum);
			tmpLen = inputLenSum;
		}
	}

	rsaBuff = &g_CryptoData.pbBuffer[U16_S_U8(keyLen, 1)];
	HASHAPIParamsInit(&myRsaApiParam, algType);
	switch (myRsaApiParam.algType)
	{
	case ISO9796_SIGN:
		PXB_W_U8(rsaBuff, 0xBC);  // Add ISO9796 trail byte: 0xBC
		rsaBuff = PX8_S_U8(rsaBuff, myRsaApiParam.digestSize);
		if(NULL == mdBuff)
		{
			if(sigLen == HASH_DONE)
			{
				memCopyToRam(rsaBuff,inBuff,inLen);
			}
			else
			{
				HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(rsaBuff), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);
			}
			rsaBuff = PX8_S_U16(rsaBuff,tmpLen);
			memCopyToRam(rsaBuff, inBuff, tmpLen);
		}
		else
		{
			HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(rsaBuff), HASH_UPDATE, mdBuff, tmpBuff, myRsaApiParam.HashAlg);
			rsaBuff = PX8_S_U16(rsaBuff,tmpLen);
			if (tmpLen > tmpBuffLen)
			{
				memCopyToRam(rsaBuff, tmpSrcBuff, tmpBuffLen);
				rsaBuff = PX8_A_U16(rsaBuff, tmpBuffLen);
				memCopyToRam(rsaBuff, inBuff, U16_S_U16(tmpLen, tmpBuffLen));
			}
			else
			{
				memCopyToRam(rsaBuff, tmpSrcBuff, tmpLen);
			}
		}
		break;
	case PKCS1_SIGN:
		rsaBuff = PX8_S_U8((PX8_A_CONST(rsaBuff,1)),myRsaApiParam.digestSize);
		if(NULL == mdBuff)
		{
			if(sigLen == HASH_DONE)
			{
				memCopyToRam(rsaBuff,inBuff,inLen);
			}
			else
			{
				HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(rsaBuff), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);
			}
		}
		else
		{
			HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(rsaBuff), HASH_UPDATE, mdBuff, tmpBuff, myRsaApiParam.HashAlg);
		}
		rsaBuff = PX8_S_U8(rsaBuff,myRsaApiParam.pDigestInfoLen);
		memCopyToRam(rsaBuff, MEMtPM8(myRsaApiParam.pDigestInfo), myRsaApiParam.pDigestInfoLen);
		g_CryptoData.wBufferLen = U8_A_U8(myRsaApiParam.pDigestInfoLen, myRsaApiParam.digestSize);
		break;
	case PKCS1_PSS_SIGN:
		//M' = 0000000000000000 || mHash || salt
		rsaBuff = &g_CryptoData.pbBuffer[8] ;
		if(NULL == mdBuff)
		{
			if(sigLen == HASH_DONE)
			{
				memCopyToRam(rsaBuff,inBuff,inLen);
			}
			else
			{
				HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(rsaBuff), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);//mHash
			}
		}
		else
		{
			HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(rsaBuff), HASH_UPDATE, mdBuff, tmpBuff, myRsaApiParam.HashAlg);      //mHash
		}
		g_CryptoData.wBufferLen = myRsaApiParam.digestSize;
		break;
	case PKCS1_OAEP_SIGN:
		rsaBuff = PX8_S_U16(rsaBuff, inLen);
		ecpMem_Copy(rsaBuff, inBuff, inLen);
	  g_CryptoData.wBufferLen = inLen;
		break;
	default:
		faultDetected();
		break;
	}
	incrementCodeFlowControlFlag();
}

/**********************************************************************************
 * Function name: RSAVerify
 * Description: This function verifies a signature after RSA compuation and depad.
 * input parameter:
 * 		MEM_U8_P inBuff             The source data address.
 * 		u16 inLen                   The length of source data.
 * 		MEM_U8_P tmpSrcBuff         A buffer for storing plain text(just for ISO9796 padding).
 * 		u16 tmpBuffLen              The length of pbSrcTempBuffer, max length is the private key length - 21.
 * 		MEM_U8_P mdBuff             md information.
 *      MEM_U8_P tmpBuff			md temp data.
 *      u8 algType					algorithm and mode.
 * 		u16 keyLen					key length.
 * 		u16 preHashFlag             已经做过hash运算
 * output parameter:
 *		verify result.
 * NOTE:
 * the structure of algType is as follows
 *  ----------------------------------------------------------------------
 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
 *  |           8bit  7bit            |   6bit 5bit 4bit 3bit 2bit 1bit  |
 *  ----------------------------------------------------------------------
 * modify by jiangxiaohui@20220525: 
 **********************************************************************************/
u8 RSAVerify(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 tmpBuffLen, PM8 mdBuff, PM8 tmpBuff, u8 algType, u16 keyLen,u16 preHashFlag)    SMALL_MODE
{
	RSA_SIGN_API_Param_t myRsaApiParam;
	PX8	 			rsaBuff;
	u16		D_A		tmpLen;
	u16		D_A		inputLenSum;
	u8 		X_A 	status;

#if WITH_SHA512
	u8 X_A myHash[HASH_ICV_SIZE + HASH_ICV_SIZE];
#endif

	PATCH_BOOT_RSA_API_006();
	
	tmpLen = 0;
	inputLenSum = U16_A_U16(inLen, tmpBuffLen);

	if (((U8_AND_U8(algType, RSA_SIGN_ALG_TYPE_MASK)) == ALG_RSA_SHA_ISO9796)
		|| ((U8_AND_U8(algType, RSA_SIGN_ALG_TYPE_MASK)) == ALG_RSA_RIPEMD160_ISO9796))
	{
		// the minimum byte number that must be reserved in ISO9796 padding is 22 bytes,
		// also, the number is just the number to be compared with.
		tmpLen = U16_S_U8(keyLen, 22);

		if (inputLenSum < tmpLen)
		{
			tmpLen = inputLenSum;
		}
	}

	rsaBuff = g_CryptoData.pbBuffer;
	HASHAPIParamsInit(&myRsaApiParam, algType);
	status = RSA_PROCESS_OK;
	switch (myRsaApiParam.algType)
	{

	case ISO9796_VERIFICATION:
		if (NULL == mdBuff)
		{
			if(preHashFlag == HASH_DONE)
			{
				memCopyToRam(myHash,inBuff,inLen);
			}
			else
			{
				HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(myHash), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);
			}
			status = U8_OR_U8(status, SC_memCompare(rsaBuff, inBuff, tmpLen));
			rsaBuff = PX8_A_U16(rsaBuff, tmpLen);
		}
		else
		{
			HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(myHash), HASH_UPDATE, mdBuff, tmpBuff, myRsaApiParam.HashAlg);
			if (tmpBuffLen >= tmpLen)
			{
				status = U8_OR_U8(status, SC_memCompareEqualWithDFA(rsaBuff, tmpSrcBuff, tmpLen));
				rsaBuff = PX8_A_U16(rsaBuff, tmpLen);
			}
			else
			{
				status = U8_OR_U8(status, SC_memCompareEqualWithDFA(rsaBuff, tmpSrcBuff, tmpBuffLen));
				rsaBuff = PX8_A_U16(rsaBuff, tmpBuffLen);
				tmpLen = U16_S_U16(tmpLen, tmpBuffLen);
				status = U8_OR_U8(status, SC_memCompareEqualWithDFA(rsaBuff, inBuff, tmpLen));
				rsaBuff = PX8_A_U16(rsaBuff, tmpLen);
			}
		}
		status = U8_OR_U8(status, SC_CmpRamVsRam(rsaBuff, myHash, myRsaApiParam.digestSize));
		break;
		
	case PKCS1_VERIFICATION:
		if (g_CryptoData.wBufferLen != (U8_A_U8(myRsaApiParam.pDigestInfoLen , myRsaApiParam.digestSize)))
		{
			status = U8_OR_U8(status, RSA_DATA_DIFFERENT);
		}

		status = U8_OR_U8(status, SC_CmpRamVsRom(MEMtPM8(rsaBuff), MEMtPM8(myRsaApiParam.pDigestInfo), myRsaApiParam.pDigestInfoLen));
// 		rsaBuff += LENGTH_DIGESTINFO_SHA1;
		rsaBuff = PX8_A_U8(rsaBuff, myRsaApiParam.pDigestInfoLen);

		if (NULL == mdBuff)
		{
			if(preHashFlag == HASH_DONE)
			{
				memCopyToRam(myHash,inBuff,inLen);
			}
			else
			{
				HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(myHash), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);
			}
		}
		else
		{
			HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(myHash), HASH_UPDATE, mdBuff, tmpBuff, myRsaApiParam.HashAlg);
		}

		status  = U8_OR_U8(status, SC_CmpRamVsRam(rsaBuff, myHash, myRsaApiParam.digestSize));
		break;

	case PKCS1_PSS_VERIFICATION:
		// rsaBuff = 0000000000000000 || M' || salt
		if (NULL == mdBuff)
		{
			if(preHashFlag == HASH_DONE)
			{
				memCopyToRam(myHash,inBuff,inLen);
			}
			else
			{
				HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(myHash), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);
			}
		}
		else
		{
			HASHDoFinal(MEMtPM8(inBuff), inLen, MEMtPM8(myHash), HASH_UPDATE, mdBuff, tmpBuff, myRsaApiParam.HashAlg);
		}

		//myHash = M' by inbuffer calc || M' from sign
		ecpMem_Copy(&myHash[myRsaApiParam.digestSize], &rsaBuff[8], myRsaApiParam.digestSize);
		ecpMem_Copy(&rsaBuff[8], myHash, myRsaApiParam.digestSize);

		//get self M' by inBuff
		HASHDoFinal(rsaBuff, (myRsaApiParam.digestSize * 2 + 8), MEMtPM8(myHash), HASH_INIT, NULL, NULL, myRsaApiParam.HashAlg);

		status  = U8_OR_U8(status, SC_CmpRamVsRam(myHash, &myHash[myRsaApiParam.digestSize], myRsaApiParam.digestSize));
//		status |= ecpMem_Compare((void *)myHash, (void *)&myHash[myRsaApiParam.digestSize], myRsaApiParam.digestSize);
		break;
	default:
		faultDetected();
		break;
	}
	incrementCodeFlowControlFlag();
	return status;
}

//---------------------------------------------
// wzq (2015-3-31)
//---------------------------------------------
u16 RSA_CipherUpdate(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 accOffset, u16 tmpMaxLen, u8 algType, u8 mode)
{
	accOffset += inLen;
	if ( (ALG_RSA_NOPAD == algType) || (MODE_DECRYPT == mode) )
	{
		if (accOffset > tmpMaxLen)	
		{
			return RSA_CIPHER_ILLEGAL_USE;
		}
	}
	// EB = 00 || 01 (02) || PS || 00 || D, the length of PS must be at least 8 bytes.
	else if (accOffset > (tmpMaxLen - 11))
	{
		return RSA_CIPHER_ILLEGAL_USE;
	}
	
	memCopyNoAtomic(tmpSrcBuff, inBuff, inLen);
	return U8t16(RSA_PROCESS_OK);
}

//---------------------------------------------
// lzm (2009-10-18)
// modify by jiangxiaohui@20220525: 增加tempDataBuffer的初始化
//---------------------------------------------
u16 RSA_CipherDofinal(PM8 inBuff, u16 inLen, PM8 tmpSrcBuff, u16 accOffset, u8 algType, u8 mode, u8 rsaState)
{
	u16		D_A		retLen;
	u16     D_A     inOffset;
	PX8		X_A		tempDataBuffer;
	
	retLen 			= RSA_GetModuluLength();

	if (RSA_INIT == rsaState)
	{
		inOffset 	= inLen;  // calculate the total input length.
	}
	else
	{
		inOffset 	= inLen + accOffset;  // inOffset save the total input length
	}

	if ( (ALG_RSA_NOPAD == algType) || (MODE_DECRYPT == mode) )
	{
		if (inOffset != retLen)
		{
			return RSA_CIPHER_ILLEGAL_USE;
		}
	}
	// EB = 00 || 01 (02) || PS || 00 || D, the length of PS must be at least 8 bytes.
	else if (inOffset > (retLen - 11))
	{
		return RSA_CIPHER_ILLEGAL_USE;
	}
	
	tempDataBuffer	= MEMtPX8(MemGetReuseRam_GetTempArrFromTopBase(MAX_RSA_MUD_BYTE_LEN));
	RSA_Set_InData(tempDataBuffer);
	RSA_Set_InDataLength(inOffset);

	ecpMem_Set(tempDataBuffer, 0x00, retLen - inOffset);
	
	if (RSA_INIT == rsaState)
	{
		memCopyToRam((tempDataBuffer + retLen - inOffset), inBuff, inOffset);
	}
	else
	{
		inOffset = retLen - inOffset;  // the offset input data would put in internal buffer
		memCopyToRam((tempDataBuffer + inOffset), tmpSrcBuff, accOffset);
		memCopyToRam((tempDataBuffer + inOffset + accOffset), inBuff, inLen);
	}

	/** the structure of algType is as follows
	 *  ----------------------------------------------------------------------
	 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
	 *  |           6bit  5bit            |        4bit 3bit 2bit 1bit       |
	 *  ----------------------------------------------------------------------
	 */
	algType = (u8)(algType | (mode << 4));
	return U8t16(DoRSAOpertaion(algType, retLen));
}


/**********************************************************************************
 * name: RSA_Signature interface for app(jcvm or pboc etc)
 * Description: This function Signature inBuff date using RSA
 *
 *      inBuff                       input buffer to Signature
 *      inLen                 		 byte length of input buffer
 *      sigBuff         			 save signature date
 *      tmpSrcBuff
 *      mdBuff
 *      tmpBuff
 *      accLen                       algorithm type
 *      sigLen
 *      algType
 *      mode
 * output parameter:
 *      result of verify, MR_VERIFY_SUCCESS for success, other for fail.
 * NOTE:
 **********************************************************************************/
u16 RSA_Signature(PM8 inBuff, u16 inLen, PM8 sigBuff, PM8 tmpSrcBuff, PM8 mdBuff, PM8 tmpBuff, u16 accLen, u16 sigLen, u8 algType, u8 mode,u16 preHashFlag)
{

	u16 	D_A		retLen;
	u16 	D_A		result;
	PX8		D_A		tempDataBuffer;

	retLen 			= RSA_GetModuluLength();
	

	/** the structure of algType is as follows
	 *  ----------------------------------------------------------------------
	 *  | operation mode (sign or verify) | algorithm type (MD5 or SHA1 etc) |
	 *  |           8bit  7bit            |   6bit 5bit 4bit 3bit 2bit 1bit  |
	 *  ----------------------------------------------------------------------
	 */
	tempDataBuffer		= MEMtPX8(MemGetReuseRam_GetTempArrFromTopBase(MAX_RSA_MUD_BYTE_LEN));
	RSA_Set_InData(tempDataBuffer);
	
	algType = (u8)(algType | (mode << 6));

	if (RSA_SIGN == mode)
	{
		if (isInE2P(inBuff))
		{
			memRefresh();
		}

		incrementCodeFlowControlFlag();
		RSAPreSign(inBuff, inLen, tmpSrcBuff, accLen, mdBuff, tmpBuff, algType, retLen,sigLen);
		DoRSAOpertaion(algType, retLen);
		memCopyNoAtomic(sigBuff, (PM8)RSA_Get_OutData(), retLen);
		result	= retLen;
	}
	else  // for verify
	{
		incrementCodeFlowControlFlag();
		memCopyToRam(tempDataBuffer, sigBuff, retLen);
		RSA_Set_InDataLength(retLen);
		if (RSA_PROCESS_OK == DoRSAOpertaion(algType, retLen))
		{
			memRefresh();
			if (RSA_PROCESS_OK == RSAVerify(inBuff, inLen, tmpSrcBuff, accLen, mdBuff, tmpBuff, algType, retLen,preHashFlag))
			{
				result = true;
			}
			else
			{
				result = false;
			}
		}
		else
		{
			result = false;
		}
		
	}
	return result;
}

#if WITH_SIGNMR
/**********************************************************************************
 * Function name: msgRecoverAlg
 * Description: This function recover message from signature.
 * input parameter:
 *     MEM_U8_P sigAndRecDataBuff     signature buffer and the recovered data is also
 *                                    put in this buffer.
 *      u16 sigLength                 byte length of RSA signature
 * output parameter:
 *      byte length of recovered message or MSG_RECOVER_FAIL(0xFF5A)
 * NOTE:
 **********************************************************************************/
u16 msgRecoverAlg(PM8 sigAndRecDataBuff, u16 sigLength)
{
	u16 	X_A 	recMsgLen;
	u8 		X_A 	status;
	PATCH_BOOT_RSA_API_007();
	if (sigLength != g_RSAKeyStore.wModulusNLength)
	{
		return MSG_RECOVER_FAIL;
	}

	memCopyToRam(g_CryptoData.pbBuffer, sigAndRecDataBuff, sigLength);

	g_CryptoData.wBufferLen = sigLength;

	RSASTDCore();  // must be public key to do this function.

	status = ISO9796PaddingEngine(sigLength, ISO9796_DEPAD);

	if (status != RSA_PROCESS_OK)
	{
		return MSG_RECOVER_FAIL;
	}

	recMsgLen = U16_S_U8(g_CryptoData.wBufferLen, SHA1_DIGEST_SIZE);  // SHA1_DIGEST_SIZE is equal to RIPEMD160_DIGEST_SIZE

	if (0x8000 == U16_AND_U16(recMsgLen, 0x8000))
	{
		return MSG_RECOVER_FAIL;
	}

	memCopyNoAtomic(MEMtPM8(sigAndRecDataBuff), MEMtPM8(g_CryptoData.pbBuffer), g_CryptoData.wBufferLen);
	return recMsgLen;
}

/**********************************************************************************
 * Function name: signMRDofinalAlg
 * Description: This function generate MR signature from the input buffer.
 * input parameter:
 *      MEM_U8_P inBuff              input buffer to verify
 *      u16 inLength                 byte length of input buffer
 *      MEM_U8_P sigBuff             signature result buffer
 *      u8 algType                   algorithm type
 * output parameter:
 *      none
 * NOTE:
 **********************************************************************************/
void signMRDofinalAlg(PM8 inBuff, u16 inLength, PM8 sigBuff, u8 algType)
{
	u16 	X_A 	padMsgLen;
	u16 	X_A 	keyLen;
	PX8	    		rsaBuff;
	
	PATCH_BOOT_RSA_API_008();
	
	keyLen  = g_RSAKeyStore.wModulusNLength;
	rsaBuff = PX8_S_U8((PX8_A_U16(g_CryptoData.pbBuffer,keyLen)),1);

	// the minimum byte number that must be reserved in ISO9796 padding is 22 bytes,
	// also, the number is just the number to be padded with the source data.
	padMsgLen = U16_S_U8(keyLen, 22);

	if (inLength > padMsgLen)
	{
		g_CryptoData.wBufferLen = keyLen;
	}
	else if (inLength == padMsgLen)
	{
		g_CryptoData.wBufferLen = U16_S_U8(keyLen, 1);
	}
	else
	{
		g_CryptoData.wBufferLen = U8_A_U16(22, inLength);
		padMsgLen = inLength;
	}

	PXB_W_U8(rsaBuff, 0xBC);  // Add ISO9796 trail byte: 0xBC

	rsaBuff = PX8_S_U8(rsaBuff, SHA1_DIGEST_SIZE);  // SHA1_DIGEST_SIZE == RIPEMD160_DIGEST_SIZE

	switch (algType)
	{
	case ALG_RSA_SHA_ISO9796_MR:
		HASHDoFinal(MEMtPM8(inBuff), inLength, MEMtPM8(rsaBuff), HASH_INIT, NULL, NULL, ALG_SHA_1);
		break;
#if WITH_RIPEMD160
	case ALG_RSA_RIPEMD160_ISO9796_MR:
		HASHDoFinal(MEMtPM8(inBuff), inLength, MEMtPM8(rsaBuff), HASH_INIT, NULL, NULL, ALG_RIPEMD160);
		break;
#endif
	default:
		faultDetected();
		break;
	}

	rsaBuff = PX8_S_U16(rsaBuff, padMsgLen);
	memCopyToRam(rsaBuff, inBuff, padMsgLen);


	ISO9796PaddingEngine(keyLen, ISO9796_PAD);

	switch (g_RSAKeyStore.bRSAKeyType)
	{
	case TYPE_RSA_PUBLIC:
	case TYPE_RSA_PRIVATE:
		RSASTDCore();
		break;
	case TYPE_RSA_CRT_PRIVATE:
		RSACRTCore();
		break;
	default:
		faultDetected();
		break;
	}

	memCopyNoAtomic(MEMtPM8(sigBuff), MEMtPM8(g_CryptoData.pbBuffer), keyLen);
}

/**********************************************************************************
 * Function name: signMRUpdateFinalAlg
 * Description: This function sign message with recover data and hash value.
 *              This function is only be called after update method is invoked.
 * input parameter:
 *      MEM_U8_P inBuff              input buffer to verify
 *      u16 inLength                 byte length of input buffer
 *      MEM_U8_P sigBuff             signature result buffer
 *      u8 algType                   algorithm type
 *      MEM_U8_P tmpSrcBuff          temp source buffer that is updated.
 *      u16 tmpBuffLen               temp source buffer length
 *      MEM_U8_P mdBuff              information used in md
 *      MEM_U8_P tmpBuff             may be some remained bytes when doing md update
 * output parameter:
 *      none.
 * NOTE:
 **********************************************************************************/
void signMRUpdateFinalAlg(PM8 inBuff, u16 inLength, PM8 sigBuff, u8 algType, PM8 tmpSrcBuff, u16 tmpBuffLen, PM8 mdBuff, PM8 tmpBuff)
{
	u16 	X_A 	padMsgLen;
	u16 	X_A 	inputSumLen;
	u16 	X_A 	keyLen;
	PX8	            rsaBuff;
	PATCH_BOOT_RSA_API_009();
	keyLen = g_RSAKeyStore.wModulusNLength;
	rsaBuff = PX8_S_U8((PX8_A_U16(g_CryptoData.pbBuffer,keyLen)),1);
	// the minimum byte number that must be reserved in ISO9796 padding is 22 bytes,
	// also, the number is just the number to be padded with the source data.
	padMsgLen = U16_S_U8(keyLen, 22);

	inputSumLen = U16_A_U16(inLength, tmpBuffLen);

	if (inputSumLen > padMsgLen)
	{
		g_CryptoData.wBufferLen = keyLen;
	}
	else if (inputSumLen == padMsgLen)
	{
		g_CryptoData.wBufferLen = U16_S_U8(keyLen, 1);
	}
	else
	{
		g_CryptoData.wBufferLen = U8_A_U16(22, inputSumLen);
		padMsgLen = inputSumLen;
	}

	PXB_W_U8(rsaBuff, 0xBC);  // Add ISO9796 trail byte: 0xBC

	rsaBuff = PX8_S_U8(rsaBuff, SHA1_DIGEST_SIZE);  // SHA1_DIGEST_SIZE == RIPEMD160_DIGEST_SIZE

	switch (algType)
	{
	case ALG_RSA_SHA_ISO9796_MR:
		HASHDoFinal(MEMtPM8(inBuff), inLength, MEMtPM8(rsaBuff), HASH_UPDATE, mdBuff, tmpBuff, ALG_SHA_1);
		break;
#if WITH_RIPEMD160
	case ALG_RSA_RIPEMD160_ISO9796_MR:
		HASHDoFinal(MEMtPM8(inBuff), inLength, MEMtPM8(rsaBuff), HASH_UPDATE, mdBuff, tmpBuff, ALG_RIPEMD160);
		break;
#endif
	default:
		faultDetected();
		break;
	}

	rsaBuff = PX8_S_U16(rsaBuff, padMsgLen);
	if (padMsgLen > tmpBuffLen)
	{
		memCopyToRam(rsaBuff, tmpSrcBuff, tmpBuffLen);
		rsaBuff = PX8_A_U16(rsaBuff, tmpBuffLen);
		memCopyToRam(rsaBuff, inBuff, (U16_S_U16(padMsgLen, tmpBuffLen)));
	}
	else
	{
		memCopyToRam(rsaBuff, tmpSrcBuff, padMsgLen);
	}


	ISO9796PaddingEngine(keyLen, ISO9796_PAD);

	switch (g_RSAKeyStore.bRSAKeyType)
	{
	case TYPE_RSA_PUBLIC:
	case TYPE_RSA_PRIVATE:
		RSASTDCore();
		break;
	case TYPE_RSA_CRT_PRIVATE:
		RSACRTCore();
		break;
	default:
		faultDetected();
		break;
	}

	memCopyNoAtomic(MEMtPM8(sigBuff), MEMtPM8(g_CryptoData.pbBuffer), keyLen);
}

/**********************************************************************************
 * Function name: signMRVerifyAlg
 * Description: This function verify message with recover data and hash value.
 * input parameter:
 *      MEM_U8_P inBuff              input buffer to verify
 *      u16 inLength                 byte length of input buffer
 *      MEM_U8_P recoverBuff         recovered data and hash value
 *      u16 recoverDataLen           recovered data length
 *      MEM_U8_P mdBuff              information used in md
 *      MEM_U8_P tmpBuff             may be some remained bytes when doing md update
 *      u8 algType                   algorithm type
 * output parameter:
 *      result of verify, MR_VERIFY_SUCCESS for success, other for fail.
 * NOTE:
 **********************************************************************************/
boolean signMRVerifyAlg(PM8 inBuff, u16 inLength, PM8 recoverBuff, u16 recoverDataLen, PM8 mdBuff, PM8 tmpBuff, u8 algType)
{
	PX8 		hashBuff;
	PATCH_BOOT_RSA_API_010();
	hashBuff = g_CryptoData.pbBuffer;
	if (ALG_RSA_SHA_ISO9796_MR == algType)
	{
		HASHDoFinal(inBuff, inLength, MEMtPM8(hashBuff), HASH_UPDATE, mdBuff, tmpBuff, ALG_SHA_1);
	}
	else
	{
#if WITH_RIPEMD160
		HASHDoFinal(inBuff, inLength, MEMtPM8(hashBuff), HASH_UPDATE, mdBuff, tmpBuff, ALG_RIPEMD160);
#endif
	}

	// pointer to the hash value, this is appended in the end of recovered data
	recoverBuff = PM8_A_U16(recoverBuff, recoverDataLen);

	if (ECP_CMP_RESULT_EQUAL != SC_memCompareEqualWithDFA(MEMtPM8(hashBuff), recoverBuff, SHA1_DIGEST_SIZE))  // SHA1_DIGEST_SIZE == RIPEMD160_DIGEST_SIZE
	{
		return MR_VERIFY_FAIL;
	}
	else
	{
		return MR_VERIFY_SUCCESS;
	}
}

#endif	// WITH_SIGNMR


#endif	// WITH_RSA
