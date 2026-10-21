/*
 * RNG_API.h
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#ifndef __RNG_API_H__
#define __RNG_API_H__

#include "Crypto_Constant.h"
#include "SecOperationHal.h"

#if WITH_RANDOMDATA

/* Error codes */
#define ECP_ERR_RNG_OUT_BUFF_CHECK_OK              0x9A74
#define ECP_ERR_RNG_OUT_BUFF_CHECK_FAILURE         0xCC4E

#if WITH_RANDOMDATA
	#define SYM_DES_RNG_KEY1_OFFSET			  		DES_KEY1_OFFSET
	#define SYM_DES_RNG_KEY2_OFFSET			  		DES_KEY2_OFFSET
	#define SYM_DES_RNG_KEY3_OFFSET			  		DES_KEY3_OFFSET
	#define SYM_RNG_KEY_OFFSET						SYM_DES_RNG_KEY1_OFFSET

	/* Error codes */
	#define ECP_ERR_RNG_GETRANDOM_OK              0x9A7C
	#define ECP_ERR_RNG_GETRANDOM_FAILURE         0xCC8D

	#define TRNG_DATA_SIZE                8
	#define TRNG_DATA_INDEX_MASK          7
	#define TRNG_CACHE_BYTE_SIZE          (TRNG_DATA_SIZE << 2)
	#define TRNG_CACHE_INDEX_MASK         0x1F
	typedef struct
	{
		PX32         Data;
		u16          IndexIn;
		u16          IndexOut;
	} TRNG_Context;
	typedef struct
	{
		PX8 rngBuff;
		u32 length;
		u8 checksum;
	} rngParaStruct;
	
	extern u8 g_rngRegFlag;
	extern TRNG_Context g_TRNG_Cache;
	extern u16 genRandomNumber_FourByteAlign(TRNG_Context * TrngContext, u16 rngLen);
	extern u16 getDataFromRngCache(PX8 outBuff, u16 size);
	extern u16 genRandomNumber(rngParaStruct rngPara);
	extern u16 Random_number_generator(PX8 rngStoreAddr, u16 rngLenth , u8 mode);
	extern u16  Random_number_generator_API(PM8 rngStoreAddr, u16 rngLenth , u8 mode);
	extern void TrngCacheInit(void);
#else
	#define genRandomNumber_FourByteAlign(TrngContext, rngLen)
	#define getDataFromRngCache(outBuff, size)
	#define genRandomNumber(rngPara)
	#define Random_number_generator(rngStoreAddr, rngLenth ,mode)						0
	#define Random_number_generator_API(rngStoreAddr, rngLenth ,mode)					0
	#define TrngCacheInit(void)
#endif  // WITH_RANDOMDATA

#endif  // /* WITH_RANDOMDATA */
#endif  // __CRYPTO_API_H__
