/************************ Copyright (c) 2021-2022, All rights reserved.   ******************
 * @file                : mh_rand.h
 * @author              :
 * @version             : V2.1
 * @date                : 2021-12-10
 * @brief               : This file contains all the functions prototypes for the random number generation
 *********************************************************************************************************/

#ifndef __MH_RAND_H
#define __MH_RAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_crypt.h"


#define MH_RET_RAND_INIT                         (0xA62F79AA)
#define MH_RET_RAND_PARAM_ERR                    (MH_RET_RAND_INIT + 1)
#define MH_RET_RAND_FAILURE                      (MH_RET_RAND_INIT + 2)
#define MH_RET_RAND_CHI_ALARM                    (MH_RET_RAND_INIT + 3)
#define MH_RET_RAND_TRNG_EMPTY                   (0x562F79AA)
#define MH_RET_RAND_SUCCESS                      (MH_RET_RAND_TRNG_EMPTY + 0x0100)


uint32_t MHRAND_Init(void *pu32Buff, uint32_t u32BuffSize);

uint32_t MHRAND_ReInit(void);

uint32_t MHRAND_Trand(uint32_t *pu32Dst, uint32_t u32Len, uint32_t *pu32CrcInit, uint32_t *pu32CrcEnd);

int32_t MHRAND_TrandNoBlk(uint32_t *pu32Dst, uint32_t u32Len, uint32_t *pu32CrcInit, uint32_t *pu32CrcEnd);

uint32_t MHRAND_Prand(void *pvDst, uint32_t u32Len);

uint32_t MHCRC_CalcBuff(uint16_t u16CrcInit, void *pvData, uint32_t u32Size);

uint32_t MHRAND_CheckBuff(uint32_t u32CheckInit, uint32_t *pu32Data, uint32_t u32Len);

uint32_t MHRAND_UpdatePSeed(void);

#ifdef __cplusplus
}
#endif

#endif


