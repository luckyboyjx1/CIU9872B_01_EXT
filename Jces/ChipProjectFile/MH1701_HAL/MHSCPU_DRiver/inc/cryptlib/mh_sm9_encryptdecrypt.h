#ifndef __MH_SM9_ENCRYPTDECRYPT_H
#define __MH_SM9_ENCRYPTDECRYPT_H


#ifdef __cplusplus
extern "C" {
#endif
#include "mh_bignum.h"
#include "mh_sm9.h"

#define     MH_ENC_SPACE_LEN  (480)

typedef enum{
    SM9_ENC_KDF_XOR = 0,
    SM9_ENC_KDF_SM4 = 1
}SM9_ENC_KDF_TYPE;

typedef struct{
    uint8_t au8Space[MH_ENC_SPACE_LEN];
}MH_SM9_ENC_SPACE;

typedef struct{
    uint32_t u32KdfType;
    uint8_t *pu8De;
    MHBN_BIGNUM *pInput;
    MHBN_BIGNUM *pOutput;
    MHBN_BIGNUM *pIDb;
    MH_SM9_ENC_SPACE *pSpace;
    MH_SM9_INNER_SPACE *pInBase;
    uint32_t u32Crc;
}MH_SM9_ENC_DEC_CALL;



uint32_t MHSM9_GenEncPubKey(uint8_t *pu8PubKey, uint8_t *pu8PriKey, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_SetEnc(MH_SM9_ENC_SPACE *pSpace, uint8_t *pu8PubKey, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_GenEncSecKey(uint8_t *pu8SecKey, uint8_t *pu8PriKey, MHBN_BIGNUM *pID, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_Encrypt(MH_SM9_ENC_DEC_CALL *pEncCall);

uint32_t MHSM9_Decrypt(MH_SM9_ENC_DEC_CALL *pDecCall);




#ifdef __cplusplus
}
#endif

#endif
