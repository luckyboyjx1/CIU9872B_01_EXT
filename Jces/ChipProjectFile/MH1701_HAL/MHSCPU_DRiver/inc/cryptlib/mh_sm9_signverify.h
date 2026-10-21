#ifndef __MH_SM9_SIGNVERIFY_H
#define __MH_SM9_SIGNVERIFY_H


#ifdef __cplusplus
extern "C" {
#endif
#include "mh_bignum.h"
#include "mh_sm9.h"

#define     MH_SIGN_SPACE_LEN   (576)


typedef struct{
    uint8_t au8Space[MH_SIGN_SPACE_LEN];
}MH_SM9_SIGN_SPACE;

typedef struct{
	uint8_t au8H[MH_SM9_BASE_BYTE];
	uint8_t au8S[MH_SM9_BASE_BYTE*2];
}MH_SM9_SIGN;

typedef struct{
	uint8_t *pu8KeyD;
    MHBN_BIGNUM *pMsg;
    MHBN_BIGNUM *pIDa;
    MH_SM9_SIGN_SPACE *pSpace;
    MH_SM9_SIGN *pSign;
    MH_SM9_INNER_SPACE *pInBase;
    uint32_t u32Crc;
}MH_SM9_SIGN_CALL;

//for KGC
uint32_t MHSM9_GenSignPubKey(uint8_t *pu8PubKey, uint8_t *pu8PriKey, MH_SM9_INNER_SPACE *pInBase);
//for KGC
uint32_t MHSM9_GenSignSecKey(uint8_t *pu8SecKey, uint8_t *pu8PriKey, MHBN_BIGNUM *pID, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_SetSign(MH_SM9_SIGN_SPACE *pSpace, uint8_t *pu8PubKey, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_Sign(MH_SM9_SIGN_CALL *pSignCall);

uint32_t MHSM9_Verify(MH_SM9_SIGN_CALL *pVeriCall);

#ifdef __cplusplus
}
#endif

#endif
