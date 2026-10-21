/*
 *  NIST SP800-38D compliant GCM implementation
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

/*
 * http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf
 *
 * See also:
 * [MGV] http://csrc.nist.gov/groups/ST/toolkit/BCM/documents/proposedmodes/gcm/gcm-revised-spec.pdf
 *
 * We use the algorithm described as Shoup's method with 4-bit tables in
 * [MGV] 4.1, pp. 12-13, to enhance speed without using too much memory.
 */


#include "Crypto_SoftAEAD.h"
#include "halCryptoProfile.h"
#if WITH_AEAD
/*
 * Shoup's method for multiplication use this table with
 *      last4[x] = x times P^128
 * where x and last4[x] are seen as elements of GF(2^128) as in [MGV]
 */
static const u64 last4[16] =
{
    0x0000, 0x1c20, 0x3840, 0x2460,
    0x7080, 0x6ca0, 0x48c0, 0x54e0,
    0xe100, 0xfd20, 0xd940, 0xc560,
    0x9180, 0x8da0, 0xa9c0, 0xb5e0
};

/*
 * Precompute small multiples of H, that is set
 *      HH[i] || HL[i] = H times i,
 * where i is seen as a field element as in [MGV], ie high-order bits
 * correspond to low powers of P. The result is stored in the same way, that
 * is the high-order bit of HH corresponds to P^0 and the low-order bit of HL
 * corresponds to P^127.
 */
void SoftAes_AEADKeyExpand(Sym_LoadKey_Param_t * myLoadkeyParam)
{
    s32          i;
    s32          j;
    u32          T;
    u64          hi;
	u64          lo;
    u64          vl;
	u64          vh;
    u64 *        HL;
    u64 *        HH;
    u64 *        HiL;
    u64 *        HiH;
    u8           h[16];

    HL   = (u64 *)PX8_A_U16(MEMtPX8(myLoadkeyParam->pKeyBuffer), SOFT_AES_EXT_KEY_BYTE_LEN);
    HH   = (u64 *)PX8_A_U16(MEMtPX8(myLoadkeyParam->pKeyBuffer), SOFT_AES_EXT_KEY_BYTE_LEN + 128);

    memset( h, 0, 16 );
    DoAES_Crypt(h, h, MODE_ENCRYPT, myLoadkeyParam->pKeyBuffer, myLoadkeyParam->keyByteLen);
    /* pack h as two 64-bits ints, big-endian */
    GET_UINT32_BE( hi, h,  0  );
    GET_UINT32_BE( lo, h,  4  );
    vh = (u64) hi << 32 | lo;

    GET_UINT32_BE( hi, h,  8  );
    GET_UINT32_BE( lo, h,  12 );
    vl = (u64) hi << 32 | lo;

    /* 8 = 1000 corresponds to 1 in GF(2^128) */
    HL[8] = vl;
    HH[8] = vh;

    /* 0 corresponds to 0 in GF(2^128) */
    HH[0] = 0;
    HL[0] = 0;

    for( i = 4; i > 0; i >>= 1 )
    {
        T = ( vl & 1 ) * 0xe1000000U;
        vl  = ( vh << 63 ) | ( vl >> 1 );
        vh  = ( vh >> 1 ) ^ ( (u64) T << 32);

        HL[i] = vl;
        HH[i] = vh;
    }

    for( i = 2; i <= 8; i *= 2 )
    {
        HiL = HL + i;
        HiH = HH + i;
        vh = *HiH;
        vl = *HiL;
        for( j = 1; j < i; j++ )
        {
            HiH[j] = vh ^ HH[j];
            HiL[j] = vl ^ HL[j];
        }
    }
}

/*
 * Sets output to x times H using the precomputed tables.
 * x and output are seen as elements of GF(2^128) as in [MGV].
 */
void gcm_mult(PX32 pKeyBuffer, u8 x[16], u8 output[16] )
{
    u8             lo;
    u8             hi;
    u8             rem;
    s32             i;
    u64             zh;
    u64             zl;
    u64 *           HL;
    u64 *           HH;

    i    = 0;
    HL   = (u64 *)PX8_A_U16(MEMtPX8(pKeyBuffer), SOFT_AES_EXT_KEY_BYTE_LEN);
    HH   = (u64 *)PX8_A_U16(MEMtPX8(pKeyBuffer), SOFT_AES_EXT_KEY_BYTE_LEN + 128);
    lo   = x[15] & 0xf;
    zh   = HH[lo];
    zl   = HL[lo];

    for( i = 15; i >= 0; i-- )
    {
        lo = x[i] & 0xf;
        hi = x[i] >> 4;

        if( i != 15 )
        {
            rem = (u8) zl & 0xf;
            zl = ( zh << 60 ) | ( zl >> 4 );
            zh = ( zh >> 4 );
            zh ^= (u64) last4[rem] << 48;
            zh ^= HH[lo];
            zl ^= HL[lo];

        }

        rem = (u8) zl & 0xf;
        zl = ( zh << 60 ) | ( zl >> 4 );
        zh = ( zh >> 4 );
        zh ^= (u64) last4[rem] << 48;
        zh ^= HH[hi];
        zl ^= HL[hi];
    }

    PUT_UINT32_BE( zh >> 32, output, 0 );
    PUT_UINT32_BE( zh, output, 4 );
    PUT_UINT32_BE( zl >> 32, output, 8 );
    PUT_UINT32_BE( zl, output, 12 );
}

#endif /* MBEDTLS_GCM_C */
