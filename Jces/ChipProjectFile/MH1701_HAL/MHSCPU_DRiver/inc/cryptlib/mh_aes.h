/************************ Copyright (c) 2021-2022, All rights reserved.   ******************
 * @file                : mh_aes.h
 * @author              :
 * @version             : V2.1
 * @date                : 2021-12-10
 * @brief               : This file contains all the functions prototypes for the aes algorithm
 *********************************************************************************************************/

#ifndef __MH_AES_H
#define __MH_AES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_crypt.h"

#define MH_AES_OPT_MODE_Mask        (BIT(0))         
#define MH_AES_OPT_MODE_ENCRYPT     (0)
#define MH_AES_OPT_MODE_DECRYPT     (BIT(0))

#define MH_AES_OPT_BLK_Mask         (BIT(1)|BIT(2))
#define MH_AES_OPT_BLK_CBC          (BIT(1))
#define MH_AES_OPT_BLK_ECB          (BIT(2))
#define MH_AES_OPT_BLK_CBC_MAC      (BIT(1)|BIT(2))

#define MH_AES_OPT_KEY_LEN_Mask     (BIT(3)|BIT(4))
#define MH_AES_OPT_KEY_128          (BIT(3))
#define MH_AES_OPT_KEY_192          (BIT(4))
#define MH_AES_OPT_KEY_256          (BIT(3)|BIT(4))

#define MH_RET_AES_SUCCESS                      (0x5471BAAA)
#define MH_RET_AES_INIT                         (MH_RET_AES_SUCCESS - 0x100)
#define MH_RET_AES_PARAM_ERR                    (MH_RET_AES_INIT + 0x01)
#define MH_RET_AES_DFA_LOCK                     (MH_RET_AES_INIT + 0x02)
#define MH_RET_AES_TRNG_ERR                     (MH_RET_AES_INIT + 0x03)
#define MH_RET_AES_ATTACKED                     (MH_RET_AES_INIT + 0x04)
#define MH_RET_AES_PACK_MODE_ERROR              (MH_RET_AES_INIT + 0x05)
#define MH_RET_AES_OUTBUF_TOO_SMALL             (MH_RET_AES_INIT + 0x06)
#define MH_RET_AES_INPUT_SIZE_ERROR             (MH_RET_AES_INIT + 0x07)
#define MH_RET_AES_KEY_SIZE_ERROR               (MH_RET_AES_INIT + 0x08)
#define MH_RET_AES_MODE_ERROR                   (MH_RET_AES_INIT + 0x09)


uint32_t MHAES_EncDec(MH_SYM_CRYPT_CALL *pCall);

uint32_t MHAES_GetVersion(void);

#endif

#ifdef __cplusplus
}
#endif

