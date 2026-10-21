/*
 * Crypto_softKOREANSEED.h
 *
 *  Created on: 2014Äê3ÔÂ17ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTKOREANSEED_H_
#define CRYPTO_SOFTKOREANSEED_H_


#include "Sym_API.h"

#if WITH_SOFT_KOREANSEED

/* right rotate shift 8 bit, A, B are u32 type variables, temp is u8 variable */
#define ROTATER8(A, B, temp) {\
	temp = (B & 0xFF);                                  \
	B = (((B >> 8) & 0x00FFFFFF) | ((A & 0xFF) << 24)); \
	A = (((A >> 8) & 0x00FFFFFF) | ((u32)temp << 24));       \
}

/* left rotate shift 8 bit, A, B, temp are u32 type variables */
#define ROTATEL8(A, B, temp) {\
	temp = ((A >> 24) & 0xFF);                 \
	A = ((A << 8) | ((B >> 24) & 0x000000FF)); \
	B = ((B << 8) | (temp));                   \
}

/* ROTATEL(INOUT, n) cyclically rotates INOUT over n bits to the left */
/* INOUT must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROTATEL(INOUT, n)  (INOUT = ((INOUT << n) | (INOUT >> (32-n))))

/* ROTATER(INOUT, n) cyclically rotates INOUT over n bits to the right */
/* INOUT must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROTATER(INOUT, n)  (INOUT = ((INOUT >> n) | (INOUT << (32-n))))

/* to get better performance, we define new function G: GN */
#define GN(X_in) (SS_BOX_0[(X_in) & 0xFF] ^ SS_BOX_1[((X_in)>>8) & 0xFF] ^ SS_BOX_2[((X_in)>>16) & 0xFF] ^ SS_BOX_3[((X_in)>>24) & 0xFF])

/* Function F with two 32-bit input blocks C_in, D_in and two 32-bit subkeys Ki0, Ki1, output two 32-bit blocks C_out, D_out
   Function F definition with Macro, some platform with this definitine results stack overflow when compling. */
/*
#define	KOREANSEED_F(C_in, D_in, subKey0, subKey1, C_out, D_out)		{\
	D_out = GN(((u32)(GN(((u32)(GN(((C_in ^ subKey0) ^ (D_in ^ subKey1))) + (C_in ^ subKey0)))) + GN(((C_in ^ subKey0) ^ (D_in ^ subKey1)))))); \
	C_out = D_out + GN(((u32)(GN(((C_in ^ subKey0) ^ (D_in ^ subKey1))) + (C_in ^ subKey0)))); \
} */

/* function converting u8 to u32 */
#define u8_to_u32(L, C, index) (L = (C[index] << 24) | (C[index+1] << 16) | (C[index+2] << 8) | (C[index+3]))
#define u32_to_u8(C, L, index) {   \
	C[index] = (L >> 24) & 0xFF;   \
	C[index+1] = (L >> 16) & 0xFF; \
	C[index+2] = (L >> 8) & 0xFF;  \
	C[index+3] = (L & 0xFF);       \
}

/*****************************************************************************
* API
*****************************************************************************/
extern void SoftKoreanseed_loadKoreanSeedKey(u8 keyByteLen, PX32 pExternKey);
extern void SoftKoreanseed_DoKoreanSeed_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen);

#define loadKoreanSeedKey(keyByteLen, pExternKey)                     				SoftKoreanseed_loadKoreanSeedKey(keyByteLen, pExternKey)
#define DoKoreanSeed_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, KeyByteLen)     SoftKoreanseed_DoKoreanSeed_Crypt(dataIn, dataOut, en_deCrypt, pExternKey, KeyByteLen)
#else
extern void loadKoreanSeedKey(u8 keyByteLen, PX32 pExternKey);
extern void DoKoreanSeed_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen);
#endif  // WITH_SOFT_KOREANSEED

#endif /* CRYPTO_SOFTKOREANSEED_H_ */
