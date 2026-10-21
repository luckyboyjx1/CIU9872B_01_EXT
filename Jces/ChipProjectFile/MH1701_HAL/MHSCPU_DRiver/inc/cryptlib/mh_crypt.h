/************************ Copyright (c) 2021-2022, All rights reserved.   ******************
 * @file                : mh_crypt.h
 * @author              :
 * @version             : V2.1
 * @date                : 2021-12-10
 * @brief               : This file contains all the functions prototypes for the crypto config
 *********************************************************************************************************/

#ifndef __MH_CRYPT_H
#define __MH_CRYPT_H

#ifdef __cplusplus
 extern "C" {
#endif
     
#include <stdint.h>
#include <string.h>

#define BIT(n)                  (1UL << (n))
#define BITS(H_start,L_end)     ((0xFFFFFFFFUL>>(31 - (H_start))) & (0xFFFFFFFFUL << (L_end)))

#define MH_RET_CRYPTCFG_INIT             (0x5E4829AAUL)
#define MH_RET_CRYPTCFG_PARAM_ERR        (MH_RET_CRYPTCFG_INIT + 0x01)
#define MH_RET_CRYPTCFG_FAILURE          (MH_RET_CRYPTCFG_INIT + 0x02)
#define MH_RET_CRYPTCFG_SUCCESS          (MH_RET_CRYPTCFG_INIT + 0x100)
     
     
void MHCRYPT_SYMCITClear(void);

void MHCRYPT_ASYMCITClear(void);

void MHCRYPT_SetBusKey(uint32_t pu32Key[2]);

void MHCRYPT_CramInit(void);

uint32_t MHCRYPT_GetVersion(void);

typedef struct
{
    uint8_t *pu8In;
    uint8_t *pu8Out;
    uint8_t *pu8IV;
    uint8_t *pu8IVout;
    uint8_t *pu8Key;
    uint32_t u32InLen;
    uint32_t u32OutLen;
    uint16_t u16Opt;
    uint32_t u32Crc;
} MH_SYM_CRYPT_CALL;

#ifdef __cplusplus
}
#endif

#endif // __MH_CRYPT_H
