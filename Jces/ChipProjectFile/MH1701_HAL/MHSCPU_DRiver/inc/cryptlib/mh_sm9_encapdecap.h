#ifndef __MH_SM9_ENCAPDECAP_H
#define __MH_SM9_ENCAPDECAP_H


#ifdef __cplusplus
extern "C" {
#endif
#include "mh_bignum.h"
#include "mh_sm9.h"

#define     MH_ENCAP_SPACE_LEN  (480)

typedef struct{
    uint8_t au8Space[MH_ENCAP_SPACE_LEN];
}MH_SM9_ENCAP_SPACE;

typedef struct{
    uint8_t *pu8Cipher;
    uint8_t *pu8De;
    MHBN_BIGNUM *pOutKey;
    MHBN_BIGNUM *pIDb; 
    MH_SM9_ENCAP_SPACE *pSpace;
    MH_SM9_INNER_SPACE *pInBase;
    uint32_t u32Crc;
}MH_SM9_ENCAP_DECAP_CALL;


uint32_t MHSM9_GenEncapPubKey(uint8_t *pu8PubKey, uint8_t *pu8PriKey, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_SetEncap(MH_SM9_ENCAP_SPACE *pSpace, uint8_t *pu8PubKey, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_GenEncapSecKey(uint8_t *pu8SecKey, uint8_t *pu8PriKey, MHBN_BIGNUM *pID, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_Encap(MH_SM9_ENCAP_DECAP_CALL *pEncapCall);

uint32_t MHSM9_Decap(MH_SM9_ENCAP_DECAP_CALL *pDecapCall);


#ifdef __cplusplus
}
#endif

#endif
