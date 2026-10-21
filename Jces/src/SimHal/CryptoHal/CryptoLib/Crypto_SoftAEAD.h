/**
 * \file Crypto_SoftAEAD.h
 *
 * \brief Galois/Counter mode for 128-bit block ciphers
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#ifndef MBEDTLS_GCM_H
#define MBEDTLS_GCM_H

#include "Crypto_SoftAEAD.h"
#include "Crypto_SoftAES.h"
#include "Sym_API.h"
#include "ecpDefine.h"
#include <stdint.h>
#include <stddef.h>

#if WITH_AEAD

#define MBEDTLS_GCM_ENCRYPT     1
#define MBEDTLS_GCM_DECRYPT     0

#define MBEDTLS_ERR_GCM_AUTH_FAILED                       -0x0012  /**< Authenticated decryption failed. */
#define MBEDTLS_ERR_GCM_BAD_INPUT                         -0x0014  /**< Bad input parameters to function. */


/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
{                                                       \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )             \
        | ( (uint32_t) (b)[(i) + 1] << 16 )             \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 3]       );            \
}
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

void SoftAes_AEADKeyExpand(Sym_LoadKey_Param_t * myLoadkeyParam);
void gcm_mult(PX32 pKeyBuffer, u8 x[16], u8 output[16] );
#define AEADKeyExpand(myKeyParam)			SoftAes_AEADKeyExpand(myKeyParam)


#ifdef __cplusplus
}
#endif

#endif /* gcm.h */
#endif /*WITH_AEAD*/
