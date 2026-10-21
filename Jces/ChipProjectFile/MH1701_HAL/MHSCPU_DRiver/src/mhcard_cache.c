/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_cache.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the CACHE firmware functions
 *****************************************************************************/

 /* Includes ----------------------------------------------------------------*/
#include "mhcard_cache.h"


/*clean cache flash data/instructions*/
uint32_t CACHE_Clean(CACHE_TypeDef *Cache, uint32_t u32Addr, uint32_t u32Len)
{
	uint32_t i ,u32Tmp;
    
    u32Tmp = (u32Addr & ~(CACHE_UNIT_SIZE - 1));
    if (u32Tmp < CACHE_ADDRESS_START
        || u32Tmp >= CACHE_ADDRESS_END
        || u32Len > CACHE_SIZE)
    {
        return ERROR;
    }
    u32Tmp = CACHE_REF_REFRESH_INDEX(u32Tmp);
    for (i = 0; i < u32Len; i += CACHE_UNIT_SIZE)
    {
        while (CACHE->CACHE_CS & CACHE_CS_BUSY);
        CACHE->CACHE_REF = u32Tmp + i;
        CACHE->CACHE_REF |= CACHE_REF_REFRESH_ENABLE;
        while ((CACHE->CACHE_REF & CACHE_REF_REFRESH_ENABLE));	
    }
    return SUCCESS;
}

void CACHE_CleanAll(CACHE_TypeDef *Cache)
{
	//flush cache all 
	while (CACHE->CACHE_CS & CACHE_CS_BUSY);
	CACHE->CACHE_REF |= CACHE_REF_REFRESH_ALL;
	CACHE->CACHE_REF |= CACHE_REF_REFRESH_ENABLE;
	while ((CACHE->CACHE_REF & CACHE_REF_REFRESH_ENABLE));	
}


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
