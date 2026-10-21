#include "RNG_API.h"
#include "Crypto_Constant.h"
#include "chipHead.h"

#include "ChipFunction.h"

#if WITH_RANDOMDATA

/// @brief global variable contains RAM data of the TRNG driver
static u32 g_TRNG_CacheBuffer[TRNG_DATA_SIZE];
uint32_t au8RngBuf[0x10];
TRNG_Context g_TRNG_Cache;
//TRNG_Context TRNGLocalCache;
u8 g_rngRegFlag;
//u8 TRNGLocalCacheBuffer[NUM_OF_WORDS_IN_256BIT_FRAME];

void TrngCacheInit(void)
{
	ecpMem_Set(MEMtPX8(&g_TRNG_Cache), 0x00, sizeof(TRNG_Context));
	g_TRNG_Cache.Data = g_TRNG_CacheBuffer;
	ecpMem_Set(MEMtPX8(&TRNGLocalCache), 0x00, sizeof(TRNG_Context));
	TRNGLocalCache.Data = MEMtPX32(TRNGLocalCacheBuffer);
	
		//This interface must be called when testing algorithms and random number
	MHRAND_Init((uint32_t*)&au8RngBuf, sizeof(au8RngBuf));             //hardware generated random number
	Trng_GetRandomWord(&TRNGLocalCache);
}



//TRNG_DATA_SIZE
u16 genRandomNumber_FourByteAlign(TRNG_Context * TrngContext, u16 rngLen)
{
	u16  retStatus; 
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;

	retStatus = Random_number_generator(MEMtPX8(TrngContext-> Data), (u16)(rngLen << 2), 1);
	TrngContext->IndexOut = 0;
	TrngContext->IndexIn = (u16)(rngLen << 2);
	
	if (ECP_ERR_RNG_GETRANDOM_OK != retStatus)
	{
		faultDetected();
	}
	return retStatus;
}
u16 getDataFromRngCache(PX8 outBuff, u16 size)
{
	u16 i, j;
	PX8 rngPtr;
	u16  retStatus; 
	
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;
	if(g_TRNG_Cache.IndexIn != TRNG_CACHE_BYTE_SIZE)
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

	j = g_TRNG_Cache.IndexOut;
	rngPtr = (PX8)g_TRNG_Cache.Data;
	for(i = 0; i < size; i++)
	{
		outBuff[i] = rngPtr[j++];
		j = j & TRNG_CACHE_INDEX_MASK;
	}
	if(i != size)
	{
		faultDetected();
	}
	g_TRNG_Cache.IndexOut = j;
	
	retStatus = ECP_ERR_RNG_GETRANDOM_OK;
	return retStatus;
}

/**********************************************************************************
 * Function name: genRandomNumber
 * Description: This function generates random numbers by hardware.
 * NOTE:
 **********************************************************************************/
u16 genRandomNumber(rngParaStruct rngPara)
{
	PX8  targetBuff;
	u16  retStatus;
	u32  i;
	u32  remainLength;
	TRNG_Context TrngContext;
	i = 0;
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;
	
	SC_parameterIntegrityCheck(rngPara);
	
	targetBuff = rngPara.rngBuff;
	remainLength = rngPara.length;
	
	while(remainLength >= (TRNGLocalCache.IndexIn - TRNGLocalCache.IndexOut))
	{
		ecpMem_Copy(MEMtPX8(targetBuff), PX8_A_U16(TRNGLocalCache.Data, TRNGLocalCache.IndexOut), (TRNGLocalCache.IndexIn - TRNGLocalCache.IndexOut));
		remainLength -= (TRNGLocalCache.IndexIn - TRNGLocalCache.IndexOut);
		targetBuff += (TRNGLocalCache.IndexIn - TRNGLocalCache.IndexOut);
		i	+= (TRNGLocalCache.IndexIn - TRNGLocalCache.IndexOut);
		TRNGLocalCache.IndexOut = TRNGLocalCache.IndexIn;
		// get a set of 16 byte random number resources
		retStatus = Trng_GetRandomWord(&TRNGLocalCache);	
	}

	if(remainLength > 0)
	{
		ecpMem_Copy(MEMtPX8(targetBuff), PX8_A_U16(TRNGLocalCache.Data, TRNGLocalCache.IndexOut), remainLength);
		TRNGLocalCache.IndexOut += remainLength;
		i += remainLength;
		if((~remainLength == ~i) && (i == rngPara.length) )
		{
			retStatus	= ECP_ERR_RNG_GETRANDOM_OK;
		}
		targetBuff += remainLength;
	}
	
	if(targetBuff != rngPara.rngBuff + rngPara.length)
	{
		faultDetected();
	}
	
	if(~i != ~rngPara.length)
	{
		faultDetected();
	}

	return retStatus;
}


/**********************************************************************************
 * Function name: Random_number_generator
 * Description: This function generates random numbers by hardware.
 * input parameter:
 *      XDATA_U8_P rngStoreAddr         random number address.
 *      u16 rngLenth                    byte length of random number.
 *		u8 mode							type of Ramdom.
 * output parameter:
 *		none (The generated key is stored in the defined ram region)
 * NOTE:
 **********************************************************************************/
u16 Random_number_generator(PX8 rngStoreAddr, u16 rngLenth , u8 mode)
{
	unsigned int  ret;
	unsigned int  G_paraCheckSum,u32CrcInit;
	rngParaStruct	X_A	ecpRngPara;
	u16	retStatus;
	u8 retry_count;
	
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;
	retry_count = 0;
	while(1)
	{
		if(mode == ALG_PSEUDO_RANDOM)
		{
			ret = MHRAND_Prand(rngStoreAddr, (unsigned int)rngLenth);
			if(MH_RET_RAND_SUCCESS == ret)
			{
				return ECP_ERR_RNG_GETRANDOM_OK;
			}
		}	
		else if(mode == ALG_SECURE_RANDOM)
		{
			ecpRngPara.rngBuff = rngStoreAddr;
			ecpRngPara.length = rngLenth;
			retStatus = genRandomNumber(ecpRngPara);
			if(retStatus == ECP_ERR_RNG_GETRANDOM_OK)
			{
				return ECP_ERR_RNG_GETRANDOM_OK ;
			}
		}
		else 
		{
			while(1);
		//		return ECP_ERR_RNG_GETRANDOM_FAILURE;
		}
		
		if(retry_count<3)//totile retry 3 times;
		{
			retry_count++;
		}
		else
		{
			while(1);
		}
		
	};
}

u16 Random_number_generator_API(PM8 rngStoreAddr, u16 rngLenth , u8 mode)
{
	
	Random_number_generator(rngStoreAddr, rngLenth, mode);
	
	return 0;
}
//u16  Random_number_generator_API(PM8 rngStoreAddr, u16 rngLenth , u8 mode)
//{

//	return ECP_ERR_RNG_GETRANDOM_OK;
//}

#endif  //WITH_RANDOMDATA
