/************************ Copyright (c) 2023-2024 Megahuntmicro, All rights reserved.   ******************
 * @file                : mh_25519.h
 * @author              : Megahuntmicro
 * @version             : V1.0
 * @date                : 2024-08-20
 * @brief               : This file contains all the definition for the ecc algorithm
 *********************************************************************************************************/

#ifndef __MH_25519_H
#define __MH_25519_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_bignum.h"


#define MH_RET_25519_SUCCESS                           (0x53D2CAAA)
#define MH_RET_25519_INIT                              (MH_RET_25519_SUCCESS - 0x100)  
#define MH_RET_25519_PARAM_ERR                         (MH_RET_25519_INIT + 0x01)
#define MH_RET_25519_TRNG_ERR                          (MH_RET_25519_INIT + 0x03)
#define MH_RET_25519_ERR                               (MH_RET_25519_INIT + 0x04)
#define MH_RET_25519_VERIFY_FAILURE                    (MH_RET_25519_INIT + 0x05)

typedef struct
{
    uint8_t *pu8D;
    uint8_t *pu8Pub;
    uint32_t u32Crc;
} MH25519_KEY;

typedef struct
{
    uint32_t au32InBase[3];
}MH25519_INNER_BASE;

typedef struct
{
    //Call parameters
    MH25519_INNER_BASE *pInBaseSelf;    //Pri
    MH25519_INNER_BASE *pInBaseOther;   //pub
    uint8_t *pu8OutK;
    uint32_t u32Crc;
} MHX25519_DH_CALL;

//for X25519 key exchange
uint32_t MHX25519_GenKey(MH25519_KEY *pKey);
uint32_t MHX25519_CompleteKey(MH25519_KEY *pKey);
uint32_t MHX25519_SetKey(uint8_t *pu8SelfD, uint8_t *pu8OtherPub, MH25519_INNER_BASE *pInBaseSelf, MH25519_INNER_BASE *pInBaseOther);
uint32_t MHX25519_DH(MHX25519_DH_CALL *pCall);


#ifdef __cplusplus
}
#endif

#endif
