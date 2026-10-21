/************************ Copyright (c) 2021-2022 Megahuntmicro, All rights reserved.   ******************
 * @file                : mh_ecc.h
 * @author              : Megahuntmicro
 * @version             : V2.1
 * @date                : 2021-12-10
 * @brief               : This file contains all the functions prototypes for the ecc algorithm
 *********************************************************************************************************/

#ifndef __MH_ECC_H
#define __MH_ECC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "mh_bignum.h"
#include "mh_crypt.h"
#include "mh_eccpoint.h"

typedef struct
{
    uint32_t au32InBase[2];
}MH_ECC_INNER_BASE;

typedef struct
{
    //Call parameters
    uint8_t *pu8Input;
    uint8_t *pu8Output;
    uint8_t *pu8SharedMsg1;
    uint8_t *pu8SharedMsg2;
    uint16_t u16InLen;
    uint16_t u16M1Len;
    uint16_t u16M2Len;
    uint16_t u16MackeyLen;
    MH_ECC_INNER_BASE *pInBase;
    MHECC_PARAM *pParam;
    uint32_t u32Crc;
    uint16_t u16OutLen;
} MHECC_ENC_DEC_CALL;

typedef struct
{
    uint8_t *pu8SignR;
    uint8_t *pu8SignS;
}MHECC_SIGN;


typedef struct
{
    //Call parameters
    uint8_t *pu8Msg;
    uint8_t *pu8E;
    uint32_t u32MLen;
    uint16_t u16ELen;
    MHECC_SIGN *pu8Sign;
    MH_ECC_INNER_BASE *pInBase;
    MHECC_PARAM *pParam;
    uint32_t u32Crc;
} MHECC_SIG_VER_CALL;

typedef struct
{
    //Call parameters
    uint8_t *pu8KeyOut;
    MH_ECC_INNER_BASE *pInBaseU;
    MH_ECC_INNER_BASE *pInBaseV;
    MHECC_PARAM *pParam;
    uint32_t u32Crc;
} MHECC_ECDH_CALL;


#define MH_RET_ECC_SUCCESS                   (0x53C2DAAA)
#define MH_RET_ECC_INIT                      (MH_RET_ECC_SUCCESS - 0x100)  
#define MH_RET_ECC_PARAM_ERR                 (MH_RET_ECC_INIT + 0x01)
#define MH_RET_ECC_DFA_LOCK                  (MH_RET_ECC_INIT + 0x02)
#define MH_RET_ECC_TRNG_ERR                  (MH_RET_ECC_INIT + 0x03)
#define MH_RET_ECC_ERR                       (MH_RET_ECC_INIT + 0x04)
#define MH_RET_ECC_VERIFY_SIGN_FAILURE       (MH_RET_ECC_INIT + 0x05)
#define MH_RET_ECC_FAILURE                   (MH_RET_ECC_INIT + 0x06)


uint32_t MHECC_ParamInit(MHECC_PARAM *pEccPara);

uint32_t MHECC_SetKey(MHECC_KEY *pKeySrc, MH_ECC_INNER_BASE *pInBase, uint8_t *pu8Base, MHECC_PARAM *pEccPara);

uint32_t MHECC_HashE(MHECC_SIG_VER_CALL *pCall);

uint32_t MHECC_EcdsaSign(MHECC_SIG_VER_CALL *pCall);

uint32_t MHECC_EcdsaVerify(MHECC_SIG_VER_CALL *pCall);

uint32_t MHECC_ECDH(MHECC_ECDH_CALL *pCall);

uint32_t MHECC_EciesEnc(MHECC_ENC_DEC_CALL *pCall);

uint32_t MHECC_EciesDec(MHECC_ENC_DEC_CALL *pCall);

uint32_t MHECC_GenKey(MHECC_KEY *pKey, MHECC_PARAM *pEccPara);

uint32_t MHECC_CompleteKey(MHECC_KEY *pKey,  MHECC_PARAM *pEccPara);

uint32_t MHECC_PointCompress(uint8_t *pu8OutPoint, MHECC_POINT *pInPoint, MHECC_PARAM *pParam);

uint32_t MHECC_PointUncompress(MHECC_POINT *pOutPoint, uint8_t *pu8InPoint, MHECC_PARAM *pParam);

uint32_t MHECC_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif

