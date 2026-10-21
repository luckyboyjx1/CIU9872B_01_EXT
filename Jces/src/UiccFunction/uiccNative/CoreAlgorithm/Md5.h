#ifndef MD5_H
#define MD5_H
/* GLOBAL.H - RSAREF types and constants
 */

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
  The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
/**********************************
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif
***********************************/
#include "typedef.h"
#include "md5.h"

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
/******************************
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
********************************/
/* Md5.h - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* MD5 context. */

typedef __packed struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

extern MD5_CTX  xdata G_MD5_Context;

extern void MD5Init (MD5_CTX *context);

extern void MD5Final (MD5_CTX *context,BYTE * digest/*[16]*/);

extern void MD5Update (MD5_CTX *context,unsigned char *input, unsigned int inputLen, BYTE mode, UWORD fileAddr);

//extern void MD5DoFinalSingleCall(unsigned char* digest /*[16]*/, unsigned char* sourcedata,unsigned int length, BYTE mode,UWORD fileAddr);
#endif
