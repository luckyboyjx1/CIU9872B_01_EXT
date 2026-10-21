/*
 * Keccak.h
 *
 *  Created on: 2016Äê4ÔÂ26ÈÕ
 *      Author: yangming
 */

#ifndef _KECCAK_H_
#define _KECCAK_H_
#include "typedef.h"
#include "profile.h"
#define KECCAK_VERSION_BITS 32

#if KECCAK_VERSION_BITS == 64
typedef unsigned long long int  u64;
extern void Keccak_f_64(u64 *s);
#define ROTATE64(value, n) \
((((u64)(value))<<(n)) | (((u64)(value))>>(64-(n))))

#elif KECCAK_VERSION_BITS == 32
extern void Keccak_f_32(UWORD *s);

#elif KECCAK_VERSION_BITS == 8
extern void Keccak_f_8(BYTE s[200]);
#endif


#if KECCAK_VERSION_BITS==64
//static u64 INOUT[25]; /* state to Keccak_f for 64-bit version */
#define INOUT ((u64 *)INOUTBUF) // bug 510
extern void Keccak_f_64(u64 *s);
#define KECCAK_F Keccak_f_64
#define TUAK_ADD_PADDING() INOUT[12] = 0x1FULL, INOUT[16] = (0x01ULL<<63)

#elif KECCAK_VERSION_BITS==32
//static u32 INOUT[50]; /* state to Keccak_f for 32-bit version */
#define INOUT ((UWORD *)INOUTBUF) // bug 510
extern void Keccak_f_32(UWORD * s);
#define KECCAK_F Keccak_f_32
#define TUAK_ADD_PADDING() INOUT[24] = 0x1FUL, INOUT[33] = 0x80000000

#elif KECCAK_VERSION_BITS==8
//static u8 INOUT[200]; /* state to Keccak_f for 8-bit version */
#define INOUT INOUTBUF  // bug 510
extern void Keccak_f_8(BYTE s[200]);
#define KECCAK_F Keccak_f_8
#define TUAK_ADD_PADDING() INOUT[96] = 0x1F, INOUT[135] = 0x80

#else
#error The requested version of Keccak_f is not implemented!
#endif

#endif /* SRC_LTE_AUTHENTICATION_GNETAUTH_KECCAK_H_ */
