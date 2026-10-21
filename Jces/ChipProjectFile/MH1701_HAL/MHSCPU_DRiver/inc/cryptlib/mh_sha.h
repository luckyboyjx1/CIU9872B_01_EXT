/************************ Copyright (c) 2021-2022, All rights reserved.   ******************
 * @file                : mh_sha.h
 * @author              : 
 * @version             : V2.1
 * @date                : 2021-12-10
 * @brief               : This file contains all the functions prototypes for the sha algorithm
 *********************************************************************************************************/

#ifndef __MH_SHA_H
#define __MH_SHA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "mh_crypt.h"


#define MH_RET_SHA_INIT                         (0x576C39AA)
#define MH_RET_SHA_PARAM_ERR                    ((MH_RET_SHA_INIT + 0x01))
#define MH_RET_SHA_BUSY                         ((MH_RET_SHA_INIT + 0x02))
#define MH_RET_SHA_MODE_ERR                     ((MH_RET_SHA_INIT + 0x03))
#define MH_RET_SHA_SUCCESS                      ((MH_RET_SHA_INIT + 0x0100))


typedef struct
{
    uint32_t au32Total[2];
    uint8_t au8Buffer[64];
    uint32_t au32State[5];
}MHSHA1_Context;

typedef struct
{
    uint32_t au32Total[2];
    uint8_t  au8Buffer[64];
    uint32_t au32State[8];
}MHSHA256_Context;

typedef struct
{
    uint32_t au32Total[4];
    uint8_t  au8Buffer[128];
    uint32_t au32State[16];
}MHSHA512_Context;

typedef enum
{
    MHSHA1 = 0,
    MHSHA224 = 1,
    MHSHA256 = 2,
    MHSHA384 = 3,
    MHSHA512 = 4
}MHSHA_MOD_TYPE;

uint32_t MHSHA1_Starts(MHSHA1_Context *pCtx);
uint32_t MHSHA1_Update(MHSHA1_Context *pCtx, const uint8_t *pu8Input, uint32_t u32InputLen);
uint32_t MHSHA1_Finish(MHSHA1_Context *pCtx, uint8_t au8Digest[20]);

uint32_t MHSHA256_Starts(MHSHA256_Context *pCtx);
uint32_t MHSHA256_Update(MHSHA256_Context *pCtx, const uint8_t *pu8Input, uint32_t u32InputLen);
uint32_t MHSHA256_Finish(MHSHA256_Context *pCtx, uint8_t au8Digest[32]);

uint32_t MHSHA512_Starts(MHSHA512_Context *pCtx);
uint32_t MHSHA512_Update(MHSHA512_Context *pCtx, const uint8_t *pu8Input, uint32_t u32InputLen);
uint32_t MHSHA512_Finish(MHSHA512_Context *pCtx, uint8_t au8Digest[64]);

uint32_t MHSHA224_Starts(MHSHA256_Context *pCtx);
uint32_t MHSHA224_Update(MHSHA256_Context *pCtx,const uint8_t *pu8Input,uint32_t u32Ilen);
uint32_t MHSHA224_Finish(MHSHA256_Context *pCtx, uint8_t au8Digest[28]);

uint32_t MHSHA384_Starts(MHSHA512_Context *pCtx);
uint32_t MHSHA384_Update(MHSHA512_Context *pCtx,const uint8_t *pu8Input,uint32_t u32Ilen);
uint32_t MHSHA384_Finish(MHSHA512_Context *pCtx, uint8_t au8Digest[64]);

uint32_t MHSHA_Sha(MHSHA_MOD_TYPE ShaType, const uint8_t *pu8Input, uint32_t u32Ibytes, uint8_t *pu8Output, uint32_t u32Outlen);

#ifdef __cplusplus
}
#endif  

#endif
