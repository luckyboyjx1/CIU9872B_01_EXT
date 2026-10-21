/*
 * HASH_API.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */
#include "HASH_API.h"
#include "Crypto_API.h"
#include "MemOperation.h"
#include "Crypto_Constant.h"
#include "halCryptoConfig.h"

#if WITH_HASH

#if WITH_MD5
#if U8_2_U32_INVERSE
static u8 CONST MD5_INIT[MD_ICV_SIZE] = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};
#else
static u8 CONST MD5_INIT[MD_ICV_SIZE] = {0x67,0x45,0x23,0x01,0xEF,0xCD,0xAB,0x89,0x98,0xBA,0xDC,0xFE,0x10,0x32,0x54,0x76};
#endif //U8_2_U32_INVERSE
#endif // WITH_SHA1

#if (WITH_SHA1 || WITH_RIPEMD160)

#if WITH_SHA1
#define SHA1_ORRIPEMD160_ICV_SIZE	SHA1_ICV_SIZE
#else  //WITH_RIPEMD160
#define SHA1_ORRIPEMD160_ICV_SIZE	RIPEMD160_ICV_SIZE
#endif  //WITH_SHA1

#if U8_2_U32_INVERSE
static u8 CONST SHA1_OR_RIPEMD160_INIT[SHA1_ORRIPEMD160_ICV_SIZE] = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10,0xF0,0xE1,0xD2,0xC3};
#else
static u8 CONST SHA1_OR_RIPEMD160_INIT[SHA1_ORRIPEMD160_ICV_SIZE] = {0x67,0x45,0x23,0x01,0xEF,0xCD,0xAB,0x89,0x98,0xBA,0xDC,0xFE,0x10,0x32,0x54,0x76,0xC3,0xD2,0xE1,0xF0};
#endif //U8_2_U32_INVERSE

#if WITH_SHA1
#define SHA1_INIT					SHA1_OR_RIPEMD160_INIT
#endif // WITH_SHA1

#if WITH_RIPEMD160
#define RIPEMD160_INIT				SHA1_OR_RIPEMD160_INIT
#endif // WITH_RIPEMD160
#endif // (WITH_SHA1 || WITH_RIPEMD160)

#if WITH_SHA224
#if U8_2_U32_INVERSE
static u8 CONST SHA224_INIT[SHA224_ICV_SIZE] = {0xD8,0x9E,0x05,0xC1,0x07,0xD5,0x7C,0x36,0x17,0xDD,0x70,0x30,0x39,0x59,0x0E,0xF7,0x31,0x0B,0xC0,0xFF,0x11,0x15,0x58,0x68,0xA7,0x8F,0xF9,0x64,0xA4,0x4F,0xFA,0xBE};
#else
static u8 CONST SHA224_INIT[SHA224_ICV_SIZE] = {0xC1,0x05,0x9E,0xD8,0x36,0x7C,0xD5,0x07,0x30,0x70,0xDD,0x17,0xF7,0x0E,0x59,0x39,0xFF,0xC0,0x0B,0x31,0x68,0x58,0x15,0x11,0x64,0xF9,0x8F,0xA7,0xBE,0xFA,0x4F,0xA4};
#endif //U8_2_U32_INVERSE
#endif // WITH_SHA224

#if WITH_SHA256
#if U8_2_U32_INVERSE
static u8 CONST SHA256_INIT[SHA256_ICV_SIZE] = {0x67,0xE6,0x09,0x6A,0x85,0xAE,0x67,0xBB,0x72,0xF3,0x6E,0x3C,0x3A,0xF5,0x4F,0xA5,0x7F,0x52,0x0E,0x51,0x8C,0x68,0x05,0x9B,0xAB,0xD9,0x83,0x1F,0x19,0xCD,0xE0,0x5B};
#else
static u8 CONST SHA256_INIT[SHA256_ICV_SIZE] = {0x6A,0x09,0xE6,0x67,0xBB,0x67,0xAE,0x85,0x3C,0x6E,0xF3,0x72,0xA5,0x4F,0xF5,0x3A,0x51,0x0E,0x52,0x7F,0x9B,0x05,0x68,0x8C,0x1F,0x83,0xD9,0xAB,0x5B,0xE0,0xCD,0x19};
#endif //U8_2_U32_INVERSE
#endif // WITH_SHA256


#if WITH_SHA384
#if U8_2_U32_INVERSE
static u8 CONST SHA384_INIT[SHA384_ICV_SIZE] = {0x5D,0x9D,0xBB,0xCB,0xD8,0x9E,0x05,0xC1,0x2A,0x29,0x9A,0x62,0x07,0xD5,0x7C,0x36,0x5A,0x01,0x59,0x91,0x17,0xDD,0x70,0x30,0xD8,0xEC,0x2F,0x15,0x39,0x59,0x0E,0xF7,0x67,0x26,0x33,0x67,0x31,0x0B,0xC0,0xFF,0x87,0x4A,0xB4,0x8E,0x11,0x15,0x58,0x68,0x0D,0x2E,0x0C,0xDB,0xA7,0x8F,0xF9,0x64,0x1D,0x48,0xB5,0x47,0xA4,0x4F,0xFA,0xBE};
#else
static u8 CONST SHA384_INIT[SHA384_ICV_SIZE] = {0xCB,0xBB,0x9D,0x5D,0xC1,0x05,0x9E,0xD8,0x62,0x9A,0x29,0x2A,0x36,0x7C,0xD5,0x07,0x91,0x59,0x01,0x5A,0x30,0x70,0xDD,0x17,0x15,0x2F,0xEC,0xD8,0xF7,0x0E,0x59,0x39,0x67,0x33,0x26,0x67,0xFF,0xC0,0x0B,0x31,0x8E,0xB4,0x4A,0x87,0x68,0x58,0x15,0x11,0xDB,0x0C,0x2E,0x0D,0x64,0xF9,0x8F,0xA7,0x47,0xB5,0x48,0x1D,0xBE,0xFA,0x4F,0xA4};
#endif //U8_2_U32_INVERSE
#endif // WITH_SHA384

#if WITH_SHA512
#if U8_2_U32_INVERSE
static u8 CONST SHA512_INIT[SHA512_ICV_SIZE] = {0x67,0xE6,0x09,0x6A,0x08,0xC9,0xBC,0xF3,0x85,0xAE,0x67,0xBB,0x3B,0xA7,0xCA,0x84,0x72,0xF3,0x6E,0x3C,0x2B,0xF8,0x94,0xFE,0x3A,0xF5,0x4F,0xA5,0xF1,0x36,0x1D,0x5F,0x7F,0x52,0x0E,0x51,0xD1,0x82,0xE6,0xAD,0x8C,0x68,0x05,0x9B,0x1F,0x6C,0x3E,0x2B,0xAB,0xD9,0x83,0x1F,0x6B,0xBD,0x41,0xFB,0x19,0xCD,0xE0,0x5B,0x79,0x21,0x7E,0x13};
#else
static u8 CONST SHA512_INIT[SHA512_ICV_SIZE] = {0x6A,0x09,0xE6,0x67,0xF3,0xBC,0xC9,0x08,0xBB,0x67,0xAE,0x85,0x84,0xCA,0xA7,0x3B,0x3C,0x6E,0xF3,0x72,0xFE,0x94,0xF8,0x2B,0xA5,0x4F,0xF5,0x3A,0x5F,0x1D,0x36,0xF1,0x51,0x0E,0x52,0x7F,0xAD,0xE6,0x82,0xD1,0x9B,0x05,0x68,0x8C,0x2B,0x3E,0x6C,0x1F,0x1F,0x83,0xD9,0xAB,0xFB,0x41,0xBD,0x6B,0x5B,0xE0,0xCD,0x19,0x13,0x7E,0x21,0x79};
#endif //U8_2_U32_INVERSE
#endif // WITH_SHA512

#if WITH_SM3
#if U8_2_U32_INVERSE
static u8 CONST SM3_INIT[SM3_ICV_SIZE] = {0x6F,0x16,0x80,0x73,0xB9,0xB2,0x14,0x49,0xD7,0x42,0x24,0x17,0x00,0x06,0x8A,0xDA,0xBC,0x30,0x6F,0xA9,0xAA,0x38,0x31,0x16,0x4D,0xEE,0x8D,0xE3,0x4E,0x0E,0xFB,0xB0};
#else
static u8 CONST SM3_INIT[SM3_ICV_SIZE] = {0x73,0x80,0x16,0x6F,0x49,0x14,0xB2,0xB9,0x17,0x24,0x42,0xD7,0xDA,0x8A,0x06,0x00,0xA9,0x6F,0x30,0xBC,0x16,0x31,0x38,0xAA,0xE3,0x8D,0xEE,0x4D,0xB0,0xFB,0x0E,0x4E};
#endif //U8_2_U32_INVERSE
#endif // WITH_SM3

static void HASHInitDispatcher(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	//memRefresh();//测试 SHA256 报错，参考原来89产业化 TMC_THD89_480_VG 项目 取消原来的注释。
	switch(myInitParam->algType)
	{
#if WITH_MD5
	case ALG_MD:
		HASH_MDInitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_RIPEMD160
	case ALG_RIPEMD160:
		HASH_RIPEMD160InitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA1
	case ALG_SHA_1:
		HASH_SHA1InitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA224
	case ALG_SHA_224:
		HASH_SHA224InitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA256
	case ALG_SHA_256:
		HASH_SHA256InitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA384
	case ALG_SHA_384:
		HASH_SHA384InitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA512
	case ALG_SHA_512:
		HASH_SHA512InitMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SM3
	case ALG_SM3:
		HASH_SM3InitMain(myInitParam, myAPIParam);
		break;
#endif
		default:
		break;
	}
}

static void HASHUpdateDispatcher(HASH_Init_Param_t * myInitParam)
{
	switch(myInitParam->algType)
	{
#if WITH_MD5
	case ALG_MD:
		HASH_MDUpdateMain(myInitParam);
		break;
#endif

#if WITH_RIPEMD160
	case ALG_RIPEMD160:
		HASH_RIPEMD160UpdateMain(myInitParam);
		break;
#endif

#if WITH_SHA1
	case ALG_SHA_1:
		HASH_SHA1UpdateMain(myInitParam);
		break;
#endif

#if WITH_SHA224
	case ALG_SHA_224:
		HASH_SHA224UpdateMain(myInitParam);
		break;
#endif

#if WITH_SHA256
	case ALG_SHA_256:
		HASH_SHA256UpdateMain(myInitParam);
		break;
#endif

#if WITH_SHA384
	case ALG_SHA_384:
		HASH_SHA384UpdateMain(myInitParam);
		break;
#endif

#if WITH_SHA512
	case ALG_SHA_512:
		HASH_SHA512UpdateMain(myInitParam);
		break;
#endif

#if WITH_SM3
	case ALG_SM3:
		HASH_SM3UpdateMain(myInitParam);
		break;
#endif
		default:
		break;
	}
}

static void HASHFinalizeDispatcher(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	switch(myInitParam->algType)
	{
#if WITH_MD5
	case ALG_MD:
		HASH_MDFinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_RIPEMD160
	case ALG_RIPEMD160:
		HASH_RIPEMD160FinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA1
	case ALG_SHA_1:
		HASH_SHA1FinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA224
	case ALG_SHA_224:
		HASH_SHA224FinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA256
	case ALG_SHA_256:
		HASH_SHA256FinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA384
	case ALG_SHA_384:
		HASH_SHA384FinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SHA512
	case ALG_SHA_512:
		HASH_SHA512FinalizeMain(myInitParam, myAPIParam);
		break;
#endif

#if WITH_SM3
	case ALG_SM3:
		HASH_SM3FinalizeMain(myInitParam, myAPIParam);
		break;
#endif
		default:
		break;
	}
}

static void HASHAPIParamsInit(HASH_API_Param_t * myAPIParam, u8 HASHAlg)
{
	PATCH_BOOT_HASH_API_001();
	
	switch(HASHAlg)
	{
#if WITH_MD5
	case ALG_MD:
		myAPIParam->pICV 						= MEMtPM8(MD5_INIT);
		myAPIParam->hashIVSize					= MD_ICV_SIZE;
		myAPIParam->blockSize 					= MD_BLOCK_SIZE;
		myAPIParam->blockNumShift				= MD_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= MD_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= MD_MESSAGE_BIT_BYTELENGTH;
		break;
#endif

#if WITH_RIPEMD160
	case ALG_RIPEMD160:
		myAPIParam->pICV 						= MEMtPM8(RIPEMD160_INIT);
		myAPIParam->hashIVSize					= RIPEMD160_ICV_SIZE;
		myAPIParam->blockSize 					= RIPEMD160_BLOCK_SIZE;
		myAPIParam->blockNumShift				= RIPEMD160_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= RIPEMD160_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= RIPEMD160_MESSAGE_BIT_BYTELENGTH;
		break;
#endif

#if WITH_SHA1
	case ALG_SHA_1:
		myAPIParam->pICV 						= MEMtPM8(SHA1_INIT);
		myAPIParam->hashIVSize					= SHA1_ICV_SIZE;
		myAPIParam->blockSize 					= SHA1_BLOCK_SIZE;
		myAPIParam->blockNumShift				= SHA1_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= SHA1_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= SHA1_MESSAGE_BIT_BYTELENGTH;
		break;
#endif //WITH_SHA1
#if WITH_SHA224
	case ALG_SHA_224:
		myAPIParam->pICV	 					= MEMtPM8(SHA224_INIT);
		myAPIParam->hashIVSize					= SHA224_ICV_SIZE;
		myAPIParam->blockSize 					= SHA224_BLOCK_SIZE;
		myAPIParam->blockNumShift				= SHA224_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= SHA224_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= SHA224_MESSAGE_BIT_BYTELENGTH;
		break;
#endif //WITH_SHA224

#if WITH_SHA256
	case ALG_SHA_256:
		myAPIParam->pICV	 					= MEMtPM8(SHA256_INIT);
		myAPIParam->hashIVSize					= SHA256_ICV_SIZE;
		myAPIParam->blockSize 					= SHA256_BLOCK_SIZE;
		myAPIParam->blockNumShift				= SHA256_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= SHA256_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= SHA256_MESSAGE_BIT_BYTELENGTH;
		break;
#endif //WITH_SHA256

#if WITH_SHA384
	case ALG_SHA_384:
		myAPIParam->pICV	 					= MEMtPM8(SHA384_INIT);
		myAPIParam->hashIVSize					= SHA384_ICV_SIZE;
		myAPIParam->blockSize 					= SHA384_BLOCK_SIZE;
		myAPIParam->blockNumShift				= SHA384_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= SHA384_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= SHA384_MESSAGE_BIT_BYTELENGTH;
		break;
#endif //WITH_SHA384

#if WITH_SHA512
	case ALG_SHA_512:
		myAPIParam->pICV	 					= MEMtPM8(SHA512_INIT);
		myAPIParam->hashIVSize					= SHA512_ICV_SIZE;
		myAPIParam->blockSize 					= SHA512_BLOCK_SIZE;
		myAPIParam->blockNumShift				= SHA512_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= SHA512_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= SHA512_MESSAGE_BIT_BYTELENGTH;
		break;
#endif //WITH_SHA512

#if WITH_SM3
	case ALG_SM3:
		myAPIParam->pICV	 					= MEMtPM8(SM3_INIT);
		myAPIParam->hashIVSize					= SM3_ICV_SIZE;
		myAPIParam->blockSize 					= SM3_BLOCK_SIZE;
		myAPIParam->blockNumShift				= SM3_BLOCKNUM_SHIFT_BIT;
		myAPIParam->digestSize 					= SM3_DIGEST_SIZE;
		myAPIParam->msgBitByteLen				= SM3_MESSAGE_BIT_BYTELENGTH;
		break;
#endif //WITH_SM3
		default:
		break;
	}
	return;
}

static void HASHInitParamsInit(HASH_Init_Param_t * myInitParam, PX8 hashWorkTmp, PM8 inDataBuffer, u16 inDataLength, u8 hashBlockSize, u8 algType)
{
	myInitParam->inDataBuffer		= inDataBuffer;
	myInitParam->hashWorkSpace 		= hashWorkTmp;
	myInitParam->hashResult 		= PX8_A_U8(hashWorkTmp, hashBlockSize); 
	myInitParam->blockNum			= 0x0000;
	myInitParam->inDataLength		= inDataLength;
	myInitParam->remainLength 		= 0x00;
	myInitParam->algType			= algType;
}

static void HASHInitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam, PM8 hashConsumerBuff, PM8 RemainDataBuff, u8 mdState)
{
	u16  	X_A			blockNumber;
	u8  	X_A			remainLength;

	PATCH_BOOT_HASH_API_002();
	
	if (HASH_UPDATE == mdState)
	{
		blockNumber 	= memReadU16(hashConsumerBuff); // block number that has been updated.
		remainLength 	= memReadU8(RemainDataBuff);
	}
	else
	{
		blockNumber 	= 0x00;
		remainLength 	= 0x00;
		if (NULL != hashConsumerBuff)
		{
			if (0 != memReadU16(hashConsumerBuff))
			{
				// clear block number in mdBuffer
				memWriteU16NoAtomic(hashConsumerBuff, 0); 
			}
	    }
		if (NULL != RemainDataBuff)
		{
			if (0 != memReadU8(RemainDataBuff))
			{
			 	 // clear remain data length in mdBuffer
				memWriteU8NoAtomic(RemainDataBuff, 0);
			}
		}
	}

	if (0x00 != blockNumber)
	{
		myAPIParam->pICV 		= PM8_A_CONST(hashConsumerBuff, 2);
		myInitParam->blockNum 	= blockNumber;
	}

	if (0x00 != remainLength)
	{
		// remained data in last updating.
		//myInitParam->inDataBuffer = PM8_A_CONST(RemainDataBuff, 1);
		myInitParam->remainLength = remainLength;
		// copy last to workSpace
		memCopyToRam(myInitParam->hashWorkSpace, PM8_A_CONST(RemainDataBuff, 1), U8t16(myInitParam->remainLength));
	}
	// just deal the custom ICV
	HASHInitDispatcher(myInitParam, myAPIParam);
	return;
}

static void HASHUpdateMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	u8	 		X_A 		tempLen;
	u16 		X_A			inDataLength;
	PM8 					inDataBuffer;

	PATCH_BOOT_HASH_API_003();
	
	inDataLength = U16_A_U16(myInitParam->inDataLength, myInitParam->remainLength);
	inDataBuffer = myInitParam->inDataBuffer;

	tempLen = U16_S_U16(myAPIParam->blockSize, myInitParam->remainLength);
	if(inDataLength >= myAPIParam->blockSize)
	{
		memCopyToRam(&myInitParam->hashWorkSpace[myInitParam->remainLength], inDataBuffer, tempLen);
	    // update block number
		myInitParam->blockNum = U16_A_U16(myInitParam->blockNum, U16_SHR_U8(inDataLength, myAPIParam->blockNumShift));
		// Launch the SHA transform
		HASHUpdateDispatcher(myInitParam);
		inDataLength = U16_S_U16(inDataLength, myAPIParam->blockSize);
		inDataBuffer = PM8_A_U8(inDataBuffer, tempLen);

		while (inDataLength >= myAPIParam->blockSize)
		{
			memCopyToRam(myInitParam->hashWorkSpace, inDataBuffer, myAPIParam->blockSize);

			HASHUpdateDispatcher(myInitParam);
			inDataLength -= myAPIParam->blockSize;
			inDataBuffer = PM8_A_U8(inDataBuffer, myAPIParam->blockSize);
			// Launch the SHA transform
		}
		myInitParam->inDataBuffer = inDataBuffer;
		myInitParam->inDataLength = inDataLength;

		memCopyToRam(myInitParam->hashWorkSpace, inDataBuffer, inDataLength);
	}
	else
	{
		memCopyToRam(&myInitParam->hashWorkSpace[myInitParam->remainLength], inDataBuffer, myInitParam->inDataLength);
	}
	myInitParam->remainLength = inDataLength;
}
static void HASHFinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	HASHFinalizeDispatcher(myInitParam, myAPIParam);
}

/*******************************************************************************
 * Function name: HASHUpdate
 * Description: HASH block update operation. Continues an SHA message-digest operation,
 *              processing another message block, and updating the context.
 * Input Parameters:
 *	MEM_U8_P inBuff	       The source data address.
 *	u16 inLen	       The source data length.
 *      u8 mdState	       state of MD calculate.
 *      MEM_U8_P mdBuff        buffer containing the updated information.
 *      MEM_U8_P tmpBuff       buffer containing the remained data.
 *      u8 algType             algorithm type.
 * Output parameter:
 *		none.
 * NOTE:
 * The data stored in mdBuff is as follows:
 * ---------------------------------------------------------
 * | algType | updated block number |  intermediate hash   |
 * ---------------------------------------------------------
 * | 1 byte  |       2 byte         |      DIGEST_SIZE     |
 * ---------------------------------------------------------
 * The data stored in tmpBuff is as follows:
 * -------------------------------------
 * | tmpDataLen |       tmpData        |
 * -------------------------------------
 * |   1 byte   | 63 bytes / 127 bytes |
 * -------------------------------------
 * |             BLOCK_SIZE            |
 ******************************************************************************/
void HASHUpdate(PM8 inBuff, u16 inLen, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType)
{
	u32 		X_A			shaWorkTmp[U16_SHR_U8(HASH_WORKSPACE_SIZE, 2)];
	HASH_API_Param_t 		myAPIParam;
	HASH_Init_Param_t		myInitParam;
	
	PATCH_BOOT_HASH_API_004();
	
	HASHAPIParamsInit(&myAPIParam, algType);
	HASHInitParamsInit(&myInitParam, MEMtPX8(shaWorkTmp), inBuff, inLen, myAPIParam.blockSize, algType);

	HASHInitMain(&myInitParam, &myAPIParam, mdBuff, tmpBuff, mdState);
	HASHUpdateMain(&myInitParam, &myAPIParam);

	if (0x0000 != myInitParam.blockNum)
	{
		//E2P_write_u16((memref)mdBuff, blockNum);
		memWriteU16NoAtomic(mdBuff, myInitParam.blockNum);
		mdBuff = PM8_A_CONST(mdBuff, 2);
        //E2P_write_array((memref)mdBuff, (memref)shaResult, digestSize);
		memCopyNoAtomic(mdBuff, (PM8)myInitParam.hashResult, myAPIParam.hashIVSize);
	}

	//E2P_write_u8((memref)tmpBuff, inLen);
	memWriteU8NoAtomic(tmpBuff, myInitParam.remainLength);
	if (0x00 != myInitParam.remainLength)  // there are some bytes remained that could not construct a block
	{
		tmpBuff = PM8_A_CONST(tmpBuff, 1); // skip the length byte
		// skip the messge in last update process
        tmpBuff = PM8_A_U8(tmpBuff, U16_S_U16(myInitParam.remainLength, myInitParam.inDataLength));
		//E2P_write_array((memref)tmpBuff, (memref)inBuff, (inLen - wkspIndex));
        memCopyNoAtomic(tmpBuff, myInitParam.inDataBuffer, myInitParam.inDataLength);
	}

}

/*******************************************************************************
 * Function name: HASHDoFinal
 * Description: HASH finalization. Ends an SHA-1 or SHA-2, SHA384, SHA512
 *              message-digest operation, writing the message
 *              digest and zeroing the context.
 * Input Parameters:
 *	MEM_U8_P inBuff	       The source data address.
 *	u16 inLen	       	   The source data length.
 *  MEM_U8_P outBuff       The destination address.
 *  u8 mdState	       	   state of MD calculate.
 *  MEM_U8_P mdBuff        buffer containing the updated information.
 *  MEM_U8_P tmpBuff       buffer containing the remained data.
 *  u8 algType             algorithm type.
 * Output parameter:
 *		none.
 * NOTE:
 * The data stored in mdBuff is as follows:
 * ---------------------------------------------------------
 * | algType | updated block number |  intermediate hash   |
 * ---------------------------------------------------------
 * | 1 byte  |       2 byte         |      DIGEST_SIZE     |
 * ---------------------------------------------------------
 * The data stored in tmpBuff is as follows:
 * -------------------------------------
 * | tmpDataLen |       tmpData        |
 * -------------------------------------
 * |   1 byte   | 63 bytes / 127 bytes |
 * -------------------------------------
 * |             BLOCK_SIZE            |
 ******************************************************************************/
void HASHDoFinal(PM8 inBuff, u16 inLen, PM8 outBuff, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType)
{
	u32 		X_A			shaWorkTmp[U16_SHR_U8(HASH_WORKSPACE_SIZE, 2)];
	HASH_API_Param_t 		myAPIParam;
	HASH_Init_Param_t		myInitParam;
	
	PATCH_BOOT_HASH_API_005();
	
	HASHAPIParamsInit(&myAPIParam, algType);
	HASHInitParamsInit(&myInitParam, MEMtPX8(shaWorkTmp), inBuff, inLen, myAPIParam.blockSize, algType);

	HASHInitMain(&myInitParam, &myAPIParam, mdBuff, tmpBuff, mdState);
	HASHUpdateMain(&myInitParam, &myAPIParam);
	HASHFinalizeMain(&myInitParam, &myAPIParam);

	memCopyNoAtomic(outBuff, myInitParam.hashResult, myAPIParam.digestSize);
}
#endif //WITH_HASH

#if WITH_MD_INIT
#define MD_MAX_ACC_UPDATE_BLOCKS  0x7FFF

/*********************************************************************************
 * Function name: MDInitAlg
 * Description: Initialize MD initial value.
 * Input Parameters:
 *		MEM_U8_P initialDigestBuff	    initial value address.
 *		u16 initialDigestLength         initial value length.
 *		MEM_U8_P digestedMsgLenBuff	    buffer of digested length.
 *		u16 digestedMsgLenLength        length of digestedMsgLenBuf.
 *		MEM_U8_P mdBuff                 buffer containing the updated information.
 * Output Parameters:
 *		return false if error occurs.
 *		return true if no errors.
 * NOTE:
 * The data stored in mdBuff is as follows:
 * ---------------------------------------------------------
 * | algType | updated block number |  intermediate hash   |
 * ---------------------------------------------------------
 * | 1 byte  |       1 byte         |   MD5_DIGEST_SIZE    |
 * ---------------------------------------------------------
 ******************************************************************************/
boolean HASHInitAlg(PM8 initialDigestBuff, u16 initialDigestLength, PM8 digestedMsgLenBuff, u16 digestedMsgLenLength, PM8 mdBuff, u8 algType)
{
	u32 		X_A			shaIVTmp[U16_SHR_U8(HASH_ICV_SIZE, 2)];
	HASH_API_Param_t 		myAPIParam;
	u32 					blockNum;
	u32 					msgLen;
	u8						Be_2_Le_flag;

	HASHAPIParamsInit(&myAPIParam, algType);

	blockNum = 0;

	if ((initialDigestLength != myAPIParam.hashIVSize) || (digestedMsgLenLength == 0))
	{
		return false;
	}
	
//	if(0x01 == digestedMsgLenLength)
//	{
//		msgLen = memReadU8(digestedMsgLenBuff);
//	}
//	else if(0x02 == digestedMsgLenLength)
//	{
//		msgLen = memReadU16(digestedMsgLenBuff);
//	}
//	else if(0x04 == digestedMsgLenLength)
//	{
//		msgLen = memReadU32(digestedMsgLenBuff);
//	}
//	else
//	{
//		return false;
//	}
	while (digestedMsgLenLength--)
	{
		// the message length must be multiple of block size every time !!
		msgLen = memReadU8(digestedMsgLenBuff);
		if (/*(msgLen == 0) || */(msgLen & (myAPIParam.blockSize - 1)))
		{
			return false;
		}

		blockNum += (msgLen >> myAPIParam.blockNumShift);

		if (blockNum > MD_MAX_ACC_UPDATE_BLOCKS)
		{
			return false;
		}

		PM8_INC(digestedMsgLenBuff);
	}
//	blockNum = (msgLen >> myAPIParam.blockNumShift);	//modify by chenpingdi @20230110  更正blockNum计算结果
	if (blockNum == 0)
	{
		return false;
	}
//	if (blockNum > MD_MAX_ACC_UPDATE_BLOCKS)
//	{
//		return false;
//	}
	//E2P_write_u16((memref)mdBuff, blockNum);
	memWriteU16NoAtomic(mdBuff, blockNum);
	mdBuff = PM8_A_CONST(mdBuff, 2);
	//E2P_write_array((memref)mdBuff, (memref)shaResult, digestSize);
	
	memCopyToRam(MEMtPX8(shaIVTmp), (PM8)initialDigestBuff, myAPIParam.hashIVSize);
#if U8_2_U32_INVERSE
	switch(algType)
	{
#if WITH_MD5&&WITH_SOFT_MD5
		case ALG_MD:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_RIPEMD160&&WITH_SOFT_RIPEMD160
		case ALG_RIPEMD160:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_SHA1&&WITH_SOFT_SHA1
		case ALG_SHA_1:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_SHA224&&WITH_SOFT_SHA224
		case ALG_SHA_224:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_SHA256&&WITH_SOFT_SHA256
		case ALG_SHA_256:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_SHA384&&WITH_SOFT_SHA384
		case ALG_SHA_384:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_SHA512&&WITH_SOFT_SHA512
		case ALG_SHA_512:
			Be_2_Le_flag = true;
		break;
#endif
#if WITH_SM3&&WITH_SOFT_SM3
		case ALG_SM3:
			Be_2_Le_flag = true;
		break;
#endif
		default:
			Be_2_Le_flag = false;
		break;
	}
	if(Be_2_Le_flag == true)
	{
		BigEndian2LittleEndian((PX8)shaIVTmp, (u16)(myAPIParam.hashIVSize));
	}
	
#endif //U8_2_U32_INVERSE	
	memCopyNoAtomic(mdBuff, (PM8)shaIVTmp, myAPIParam.hashIVSize);

	return true;
}
#endif // WITH_MD_INIT

