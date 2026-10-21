#include "RNG_API.h"
#include "Crypto_Constant.h"
#include "chipHead.h"

#if WITH_RANDOMDATA

/// @brief global variable contains RAM data of the TRNG driver
u32 g_TRNG_CacheBuffer[TRNG_DATA_SIZE];
TRNG_Context g_TRNG_Cache;
//TRNG_Context TRNGLocalCache;
//u8 g_rngRegFlag;
//u8 TRNGLocalCacheBuffer[NUM_OF_WORDS_IN_256BIT_FRAME];

void TrngCacheInit(void)
{
	ecpMem_Set(MEMtPX8(&g_TRNG_Cache), 0x00, sizeof(TRNG_Context));
	g_TRNG_Cache.Data = g_TRNG_CacheBuffer;
//	ecpMem_Set(MEMtPX8(&TRNGLocalCache), 0x00, sizeof(TRNG_Context));
//	TRNGLocalCache.Data = TRNGLocalCacheBuffer;

}
u16 genRandomNumber_OneByte(PX8 rngBuff, u32 rngLen)
{
	return 0;
}


//TRNG_DATA_SIZE
u16 genRandomNumber_FourByteAlign(TRNG_Context * TrngContext, u16 rngLen)
{
	u16  retStatus; 
	retStatus = ECP_ERR_RNG_GETRANDOM_FAILURE;

	retStatus = Random_number_generator(TrngContext-> Data, rngLen << 2, 1);
	TrngContext->IndexOut = 0;
	TrngContext->IndexIn = rngLen << 2;
	
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
 * Function name: Random_number_generator
 * Description: This function generates random numbers by hardware.
 * input parameter:
 *      XDATA_U8_P rngStoreAddr         random number address.
 *      u16 rngLenth                    byte length of random number.
 * output parameter:
 *		none (The generated key is stored in the defined ram region)
 * NOTE:
 * modify by https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/dengyiwen @20210812
 **********************************************************************************/
u16 Random_number_generator(PX8 rngStoreAddr, u16 rngLenth , u8 mode)
{
	u16 i;
	static u32 j;
	miracl 				* mip;
	time_t 				seed;
	if(ALG_PSEUDO_RANDOM == mode)
	{
		SC_FUNCTION_EXC_SET_STATUS_START();

		j++;
		j = j % 14;
		for(i = 0 ; i < rngLenth ; i++)
		{
			//*(rngStoreAddr + i) = j++;
			PXB_W_U8((rngStoreAddr + i), (u8)j++);
			//*(buf + i) = 0;
		}
		SC_FUNCTION_EXC_SET_STATUS_PROCESS();
	}
	else
	{
		mip 				= mirsys(0x40, 16);
		SC_FUNCTION_EXC_SET_STATUS_START();
		// init random number generator
		time(&seed);
		seed += j;
	    irand((unsigned long)seed);   /* change parameter for different values */
		for(i = 0 ; i < rngLenth ; i++)
		{
			PXB_W_U8((rngStoreAddr + i), (u8)brand());
		}
		j = brand();
		SC_FUNCTION_EXC_SET_STATUS_PROCESS();
	}
	SC_FUNCTION_EXC_SET_STATUS_END();
	return ECP_ERR_RNG_GETRANDOM_OK;
}
/**
* modify by https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/dengyiwen @20210812
*/
u16  Random_number_generator_API(PM8 rngStoreAddr, u16 rngLenth , u8 mode)
{
	u16 i;
	static u32 j;
	miracl 				* mip;
	time_t 				seed;
	if(ALG_PSEUDO_RANDOM == mode)
	{
		SC_FUNCTION_EXC_SET_STATUS_START();

		j++;
		j = j % 14;
		for(i = 0 ; i < rngLenth ; i++)
		{
			//*(rngStoreAddr + i) = j++;
			PXB_W_U8((rngStoreAddr + i), (u8)j++);
			//*(buf + i) = 0;
		}
		SC_FUNCTION_EXC_SET_STATUS_PROCESS();
	}
	else
	{
		mip 				= mirsys(0x40, 16);
		SC_FUNCTION_EXC_SET_STATUS_START();
		// init random number generator
		time(&seed);
		seed += j;
	    irand((unsigned long)seed);   /* change parameter for different values */
		for(i = 0 ; i < rngLenth ; i++)
		{
			PXB_W_U8((rngStoreAddr + i), (u8)brand());
		}
		j = brand();
		SC_FUNCTION_EXC_SET_STATUS_PROCESS();
	}
	SC_FUNCTION_EXC_SET_STATUS_END();
	return ECP_ERR_RNG_GETRANDOM_OK;
}

#endif  //WITH_RANDOMDATA
