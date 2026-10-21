#ifndef __MH_SM9_H
#define __MH_SM9_H

#ifdef __cplusplus
extern "C" {
#endif
#include "mh_bignum.h"

#define MH_SM9_BASE_BYTE		    (32)

#define MH_RET_SM9_INIT			            (0x5D8359AA)
#define	MH_RET_SM9_PARAM_ERROR	            (MH_RET_SM9_INIT + 0x01)
#define MH_RET_SM9_TRNG_ERR                 (MH_RET_SM9_INIT + 0x02)
#define MH_RET_SM9_ERR                      (MH_RET_SM9_INIT + 0x03)
#define MH_RET_SM9_SIGN_TIMEOUT		        (MH_RET_SM9_INIT + 0x04)
#define MH_RET_SM9_VIRIFY_SIGN_FAILURE		(MH_RET_SM9_INIT + 0x05)
#define MH_RET_SM9_DECRYPT_FAILURE		    (MH_RET_SM9_INIT + 0x06)
#define MH_RET_SM9_ENCRYPT_TIMEOUT		    (MH_RET_SM9_INIT + 0x07)
#define MH_RET_SM9_ENCAP_TIMEOUT    		(MH_RET_SM9_INIT + 0x08)
#define MH_RET_SM9_VERIFY_POINT_FAILURE     (MH_RET_SM9_INIT + 0x09)
#define MH_RET_SM9_SUCCESS		            (MH_RET_SM9_INIT + 0x0100)

typedef struct
{
    uint32_t au32Space[5];
} MH_SM9_INNER_SPACE;

uint32_t MHSM9_SetParam(uint8_t *pu8Base, MH_SM9_INNER_SPACE *pInBase);

uint32_t MHSM9_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
