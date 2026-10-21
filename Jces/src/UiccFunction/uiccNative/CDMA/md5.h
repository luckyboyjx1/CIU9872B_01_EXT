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
/* MD5.H - header file for MD5C.C
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


#define CHAP 	0x00
#define MN_HA 	0x01
#define MIP_RRQ 0x02
#define MN_AAA 	0x03
#define HRPD 	0x04

#define EF_CDMA_UIMIM 			0x6F31

#if NATIVE_CARD_UICC
#define EF_CHAP_SS  			0x6E80 // 0x7F25 (TF)
#define EF_CHAP_SS_TABLE	 	0x6E81 // 0x7F25 (TF)(确保该文件记录数小于0xFF，否则会进入死循环)
#define MN_HA_SS_TABLE           	0x6E82 // 0x7F25 (TF)
#define EF_ComputerIp            	0x6E83 // 0x7F25 (TF)
#define MN_AAA_SS_TABLE          	0x6E84 // 0x7F25 (TF)
#else
#define EF_CHAP_SS  			        0x1B80 // 6E80
#define EF_CHAP_SS_TABLE	 	        0x1B81 // 6E81
#define MN_HA_SS_TABLE           	    0x1B82 // 6E82
#define EF_ComputerIp            	    0x1B83 // 6E83
#define MN_AAA_SS_TABLE          	    0x1B84 // 6E84
#endif


extern void RunMD5Algorithm(void);

extern void Auth_MD5(UWORD fileAddr, WORD len, unsigned char * output_authsign);
//extern void Auth_MD5(WORD len, unsigned char * output_authsign);

extern unsigned char  xdata PADDING[64];

extern BYTE xdata G_RamMD5[0x500];
