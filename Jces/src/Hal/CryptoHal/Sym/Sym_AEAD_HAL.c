/*
 * Sym_AEAD_HAL.c
 *
 *  Created on: 2021-8-9
 *      Author: jiangxiaohui
 */
#include "Sym_API.h"
#include "Crypto_softAES.h"
#include "chipHead.h"
#include "RNG_API.h"

#if WITH_AEAD
#include <Crypto_SoftAEAD.h>
#include "Crypto_Constant.h"
#include "Crypto_API.h"
#endif
#if WITH_AEAD
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


/*
 * Macros for common operations.
 * Results in smaller compiled code than static inline functions.
 */

/*
 * Update the CBC-MAC state in y using a block in b
 * (Always using b as the source helps the compiler optimise a bit better.)
 */
 //ret = mbedtls_cipher_update( &ctx->cipher_ctx, y, 16, y, &olen ) ) != 0
#define UPDATE_CBC_MAC                                                      \
    for( i = 0; i < 16; i++ )                                               \
    {                                                                       \
        y[i] ^= temp[i];                                                       \
    }                                                                       \
                                                                            \
    DoAES_Crypt(y, y, MODE_ENCRYPT, keyData, keyLength);
		
				

/*
 * Encrypt or decrypt a partial block with CTR
 * Warning: using b for temporary storage! src and dst must not be b!
 * This avoids allocating one more 16 bytes buffer while allowing src == dst.
 */
// mbedtls_cipher_update( &ctx->cipher_ctx, ctr, 16, b, &olen )

#define CTR_CRYPT( dst, src, len  )                                            \
	DoAES_Crypt(ctr, temp, MODE_ENCRYPT, keyData, keyLength);                     \
                                                                               \
    for( i = 0; i < len; i++ )                                                 \
    {                                                                          \
        dst[i] = src[i] ^ temp[i];                                                \
    }

int Sym_AEADCCM_starts(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam  )
{
    u8              i;
    u8              q;
	u8              enOrDeCrypt;
    u16             len_left;
	u16             add_len;
	u16             tag_len;
	u16             iv_len;
	u16             keyLength;
	u16             length;
	u16             use_len;
	PX32             keyData;
	PM8             iv;
	PM8             add;
    u8              temp[16];
    PM8             y;
    PM8             ctr;
    PM8             src;

	keyData      =  myLoadkeyParam->pKeyBuffer;
	keyLength    =  myLoadkeyParam->keyByteLen;
	length       =  myAEADCryptParam->messageLen;
	iv           =  myAEADCryptParam->pNonceBuf;
	iv_len       =  myAEADCryptParam->nonceLen;
	add          =  myAEADCryptParam->pAdataBuf;
	add_len      =  myAEADCryptParam->adataLen;
	tag_len      =  myAEADCryptParam->tagLen;
	enOrDeCrypt  =  U8_AND_U8(myLoadkeyParam->option, MODE_EN_OR_DE_MASK);
    y            =  myAEADCryptParam->pAeadCiv;
    ctr          =  myAEADCryptParam->pAeadGiv;

    q = U8_S_U8(U8_S_U8(16 , 1), U16t8(iv_len));

    /*
     * First block B_0:
     * 0        .. 0        flags
     * 1        .. iv_len   nonce (aka iv)
     * iv_len+1 .. 15       length
     *
     * With flags as (bits):
     * 7        0
     * 6        add present?
     * 5 .. 3   (t - 2) / 2
     * 2 .. 0   q - 1
     */
    temp[0] = 0;
    temp[0] |= ( add_len > 0 ) << 6;
    temp[0] |= U16_D_U8(U16_S_U8(tag_len, 2), 2) << 3;
    temp[0] |= U8_S_U8(q, 1);

    ecpMem_Copy( temp + 1, iv, iv_len );

    for( i = 0, len_left = length; i < q; i++, len_left >>= 8 )
    {
        temp[15-i] = (u8)( len_left & 0xFF );
    }
    /* Start CBC-MAC with first block */
    ecpMem_Set( y, 0, 16 );
    UPDATE_CBC_MAC;
    /* If there is additional data, update CBC-MAC with add_len, add, 0 (padding to a block boundary) */
    if( add_len > 0 )
    {
        len_left = add_len;
        src = add;

        ecpMem_Set( temp, 0, 16 );
        temp[0] = (u8)(( add_len >> 8 ) & 0xFF);
        temp[1] = (u8)(( add_len      ) & 0xFF);

        use_len = len_left < 16 - 2 ? len_left : 16 - 2;
        ecpMem_Copy( temp + 2, src, use_len );
        len_left -= use_len;
        src += use_len;

        UPDATE_CBC_MAC;

        while( len_left > 0 )
        {
            use_len = len_left > 16 ? 16 : len_left;

            ecpMem_Set( temp, 0, 16 );
            ecpMem_Copy( temp, src, use_len );
            UPDATE_CBC_MAC;

            len_left -= use_len;
            src += use_len;
        }
    }

    /*
     * Prepare counter block for encryption:
     * 0        .. 0        flags
     * 1        .. iv_len   nonce (aka iv)
     * iv_len+1 .. 15       counter (initially 1)
     *
     * With flags as (bits):
     * 7 .. 3   0
     * 2 .. 0   q - 1
     */
    ctr[0] = q - 1;
    ecpMem_Copy( ctr + 1, iv, iv_len );
    ecpMem_Set( ctr + 1 + iv_len, 0, q );
    ctr[15] = 1;

    return 0;
}

/**
 * brief This function implements the AES en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClAES_Crypt_Param
 *
 * returns Status of the operation
 *
 * retval ::PH_ERR_SMX2CLAES_CRYPT_OK  Operation finished successfully
 * retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 */

u16 Sym_AEADCCM_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
	u8                  i;
	u8 			X_A 	tmpICV[SYM_BLOCKSIZE * 3];
	u8                  enOrDeCrypt;
	u16                 length;
	u16                 len_left;
	u16                 keyLength;
    u16                 use_len;
	PX32                keyData;
	PM8                 output;
    u8                  temp[16];
    PM8                 dst;
    PM8                 y;
    PM8                 ctr;
	CONST u8*           input;
    CONST u8*           src;

	keyData         =  myCryptParam->pKeyBuffer;
	keyLength       =  myCryptParam->keyByteLen;
	input           =  myCryptParam->pInput;
	output          =  myCryptParam->pOutput;
	enOrDeCrypt 	=  U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK);
    length          =  myCryptParam->inputLength;
    y               =  &tmpICV[AES_BLOCKSIZE];
    ctr             =  tmpICV;

    ecpMem_Copy(tmpICV, myCryptParam->pIV, AES_BLOCKSIZE * 2);

    len_left = length;
    src = input;
    dst = output;

    while( len_left > 0 )
    {
        use_len = len_left > 16 ? 16 : len_left;

        if( enOrDeCrypt == MODE_ENCRYPT )
        {
        	ecpMem_Set( temp, 0, 16 );
        	ecpMem_Copy( temp, src, use_len );
            UPDATE_CBC_MAC;
        }

        CTR_CRYPT( dst, src, use_len );

        if( enOrDeCrypt == MODE_DECRYPT )
        {
        	ecpMem_Set( temp, 0, 16 );
        	ecpMem_Copy( temp, dst, use_len );
            UPDATE_CBC_MAC;
        }

        dst += use_len;
        src += use_len;
        len_left -= use_len;
        /* No need to check for overflow thanks to the length check above.*/
        for( i = 0; i < 16; i++ )
        {
           if( ++ctr[15-i] != 0 )
           {
                break;
           }
        }
    }

    ecpMem_Copy(myCryptParam->pIV, tmpICV, AES_BLOCKSIZE * 2);

    return ERR_SYM_CRYPT_OK;
}


int mbedtls_ccm_finish(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam )
{
    u8                   i;
    u8                   q;
    u16                  tag_len;
	u16                  keyLength;
	u16                  iv_len;
	PM8                  tag;
    PM8                  y;
    PM8                  ctr;
    u8                   temp[16];
    PX32                 keyData;

	keyData            =  myLoadkeyParam->pKeyBuffer;
	keyLength          =  myLoadkeyParam->keyByteLen;
	iv_len             =  myAEADCryptParam->nonceLen;
	tag                =  myAEADCryptParam->pTag;
	tag_len            =  myAEADCryptParam->tagLen;
    y                  =  myAEADCryptParam->pAeadCiv;
    ctr                =  myAEADCryptParam->pAeadGiv;

    q = U8_S_U8(U8_S_U8(16, 1), (u8)iv_len);

    /* Authentication: reset counter and crypt/mask internal tag*/
    for( i = 0; i < q; i++ )
    {
        ctr[15-i] = 0;
    }

    CTR_CRYPT( y, y, 16 );
    ecpMem_Copy( tag, y, tag_len );

    return( 0 );
}
#if MBEDTLS_GCM_C
#if WITH_AES

/*32-bit integer manipulation macros (big endian)*/
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


int Sym_AEADGCM_starts(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam  )
{
    u16               i;
    u16               adataLen;
    u16               nonceLen;
    u16               use_len;
    PX8               y;
    PX8               buf;
    u8                work_buf[16];
    CONST u8*         p;

    nonceLen         =  myAEADCryptParam->nonceLen;
    adataLen         =  myAEADCryptParam->adataLen;
    y                =  myAEADCryptParam->pAeadCiv;
    buf              =  myAEADCryptParam->pAeadGiv;

    ecpMem_Set( y, 0x00, AES_BLOCKSIZE);

    if( nonceLen == 12 )
    {
    	ecpMem_Copy( y, myAEADCryptParam->pNonceBuf, nonceLen );
        y[15] = 1;
    }
    else
    {
        ecpMem_Set( work_buf, 0x00, 16 );
        PUT_UINT32_BE( nonceLen * 8, work_buf, 12 );

        p = myAEADCryptParam->pNonceBuf;
        while( nonceLen > 0 )
        {
            use_len = ( nonceLen < 16 ) ? nonceLen : 16;

            for( i = 0; i < use_len; i++ )
                y[i] ^= p[i];

            gcm_mult( myLoadkeyParam->pKeyBuffer, y, y );

            nonceLen -= use_len;
            p += use_len;
        }

        for( i = 0; i < 16; i++ )
            y[i] ^= work_buf[i];

        gcm_mult( myLoadkeyParam->pKeyBuffer, y, y );
    }

    DoAES_Crypt(y, myAEADCryptParam->pBaseEctr, MODE_ENCRYPT, myLoadkeyParam->pKeyBuffer, myLoadkeyParam->keyByteLen);

    ecpMem_Set( buf, 0x00, AES_BLOCKSIZE);
    p = myAEADCryptParam->pAdataBuf;
    while( adataLen > 0 )
    {
        use_len = ( adataLen < 16 ) ? adataLen : 16;

        for( i = 0; i < use_len; i++ )
        {
           buf[i] ^= PM_R_U8(PM8_A_U8(p, i));
        }

        gcm_mult( myLoadkeyParam->pKeyBuffer, buf, buf );

        adataLen -= use_len;
        p += use_len;
    }

    return( 0 );
}

int mbedtls_gcm_finish(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam )
{
    u8               work_buf[16];
    u16               i;
    PX8               buf;
    u64          orig_len;
    u64          orig_add_len;

    orig_len          =  myAEADCryptParam->messageLen * 8;
    orig_add_len      =  myAEADCryptParam->adataLen * 8;
    buf               =  myAEADCryptParam->pAeadGiv;

    ecpMem_Copy( myAEADCryptParam->pTag, myAEADCryptParam->pBaseEctr, myAEADCryptParam->tagLen );

    if( orig_len || orig_add_len )
    {
        ecpMem_Set( work_buf, 0x00, 16 );

        PUT_UINT32_BE( ( orig_add_len >> 32 ), work_buf, 0  );
        PUT_UINT32_BE( ( orig_add_len       ), work_buf, 4  );
        PUT_UINT32_BE( ( orig_len     >> 32 ), work_buf, 8  );
        PUT_UINT32_BE( ( orig_len           ), work_buf, 12 );

        for( i = 0; i < 16; i++ )
        {
            buf[i] ^= work_buf[i];
        }

        gcm_mult( myLoadkeyParam->pKeyBuffer, buf, buf );

        for( i = 0; i < myAEADCryptParam->tagLen; i++ )
        {
        	myAEADCryptParam->pTag[i] ^= buf[i];
        }
    }

    return 0;
}

#endif /* MBEDTLS_GCM_C */

#if !WITH_SOFT_AES
void AESKeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	return;
}

void DoAES_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
	PATCH_HAL_CRYPTO_SYM_AES_HAL_002();
	return;
}
#endif
/**
 * brief This function implements the AES key loading.
 * The parameters shall be passed using the structure ::phSmx2ClAES_LoadKey_Param
 *
 * returns Status of the operation
 *
 * retval ::PH_ERR_SMX2CLAES_LOADKEY_OK  Operation finished successfully
 * retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 * retval ::PH_ERR_SMX2CLAES_WRONG_CHECKSUM   Wrong CRC checksum

 */
u16 Sym_AEADGCM_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
	u8 		X_A			keyByteLen;
	u8 		X_A			cryptoMode;
	PATCH_HAL_CRYPTO_SYM_AES_HAL_001();
	keyByteLen	 	= myKeyParam->keyByteLen;
    cryptoMode 		= myKeyParam->option;

//    if(SYM_CHECKSUM_VERIFY == myKeyParam->checksum)
//    {
//    	;
//    }
//    DoAES_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, KeyByteLen);
    AESKeyExpand((u16)(keyByteLen << 3), myKeyParam->pKeyBuffer, cryptoMode);
    AEADKeyExpand(myKeyParam);

    return ERR_SYM_LOADKEY_OK;
}

/**
 * brief This function implements the AES en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClAES_Crypt_Param
 *
 * returns Status of the operation
 *
 * retval ::PH_ERR_SMX2CLAES_CRYPT_OK  Operation finished successfully
 * retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 */
u16 Sym_AEADGCM_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
	u8 			X_A 	tmpICV[SYM_BLOCKSIZE * 3];
    u16                 i;
    u16                 length;
    u16                 use_len;
    PM8                 out_p;
    PX8                 y;
    PX8                 buf;
    u8                  ectr[16];
    const u8*           p;


    p           = myCryptParam->pInput;
    out_p       = myCryptParam->pOutput;
    length      = myCryptParam->inputLength;

    ecpMem_Copy(tmpICV, myCryptParam->pIV, AES_BLOCKSIZE * 2);

    buf = tmpICV;
    y = &tmpICV[AES_BLOCKSIZE];

    while( length > 0 )
    {
        use_len = ( length < 16 ) ? length : 16;

        for( i = 16; i > 12; i-- )
        {
            if( ++y[i - 1] != 0 )
            {
                break;
            }
        }
        DoAES_Crypt(y, ectr, MODE_ENCRYPT, myCryptParam->pKeyBuffer, myCryptParam->keyByteLen);

        for( i = 0; i < use_len; i++ )
        {
            if( U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK) == MODE_DECRYPT )
            {
            	buf[i] ^= p[i];
            }
            out_p[i] = ectr[i] ^ p[i];
            if( U8_AND_U8(myCryptParam->option, MODE_EN_OR_DE_MASK) == MODE_ENCRYPT )
            {
            	buf[i] ^= out_p[i];
            }
        }

        gcm_mult(myCryptParam->pKeyBuffer, buf, buf );

        length -= use_len;
        p += use_len;
        out_p += use_len;
    }
    ecpMem_Copy(myCryptParam->pIV, tmpICV, AES_BLOCKSIZE * 2);

    return ERR_SYM_CRYPT_OK;
}

int Sym_AEAD_starts(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam  )
{
	if(myLoadkeyParam->option & 0x08)
	{
		return Sym_AEADGCM_starts(myLoadkeyParam, myAEADCryptParam);
	}
	else
	{
		return Sym_AEADCCM_starts(myLoadkeyParam, myAEADCryptParam);
	}
}

int mbedtls_AEAD_finish(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam )
{
	if(myLoadkeyParam->option & 0x08)
	{
		return mbedtls_gcm_finish(myLoadkeyParam, myAEADCryptParam);
	}
	else
	{
		return mbedtls_ccm_finish(myLoadkeyParam, myAEADCryptParam);
	}
}

#if !WITH_SOFT_AES
void AESKeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	return;
}

void DoAES_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
	PATCH_HAL_CRYPTO_SYM_AES_HAL_002();
	return;
}
#endif
/**
 * brief This function implements the AES key loading.
 * The parameters shall be passed using the structure ::phSmx2ClAES_LoadKey_Param
 *
 * returns Status of the operation
 *
 * retval ::PH_ERR_SMX2CLAES_LOADKEY_OK  Operation finished successfully
 * retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 * retval ::PH_ERR_SMX2CLAES_WRONG_CHECKSUM   Wrong CRC checksum
 */
u16 Sym_AEAD_LoadKey(Sym_LoadKey_Param_t * myKeyParam)
{
	if(myKeyParam->option & 0x08)
	{
		return Sym_AEADGCM_LoadKey(myKeyParam);
	}
	else
	{
		return Sym_AES_LoadKey(myKeyParam);
	}
}

/**
 * brief This function implements the AES en-/decryption.
 * The parameters shall be passed using the structure ::phSmx2ClAES_Crypt_Param
 *
 * returns Status of the operation
 *
 * retval ::PH_ERR_SMX2CLAES_CRYPT_OK  Operation finished successfully
 * retval ::PH_ERR_SMX2CLAES_POSSIBLE_ATTACK  Possible attack detected
 */
u16 Sym_AEAD_Crypt(Sym_Crypt_Param_t * myCryptParam)
{
	if(myCryptParam->option & 0x08)
	{
		return Sym_AEADGCM_Crypt(myCryptParam);
	}
	else
	{
		return Sym_AEADCCM_Crypt(myCryptParam);
	}

    return ERR_SYM_CRYPT_OK;
}


#endif //WITH_AES
#endif //WITH_AEAD


