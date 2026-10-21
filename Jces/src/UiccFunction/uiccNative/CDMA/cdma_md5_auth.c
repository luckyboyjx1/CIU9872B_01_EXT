//#include "stdafx.h"
/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
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

#include "md5.h"
#include "string.h"
#include "typedef.h"
#include "uicc_main.h"
#include "fileAction.h"
/* Constants for MD5Transform routine.
 */



#ifdef CDMA

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

BYTE xdata PADDING[64]; // modified by WG, 20080619
UINT4 xdata a[4] ;

/*
unsigned char  xdata PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
*/

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

//UINT4 P(UINT4 x,UINT4 y,UINT4 z,BYTE mode)
UINT4 P_ex(UINT4 x,UINT4 y,UINT4 z,BYTE mode)
{
	if(mode == 0x01)
	{
		return  (((x) & (y)) | ((~x) & (z)));
	}
	else if(mode == 0x02)
	{
		return  (((x) & (z)) | ((y) & (~z)));
	}
	else if(mode == 0x03)
	{
		 return ((x) ^ (y) ^ (z));
	}
	else
	{
		return ((y) ^ ((x) | (~z)));
	}

}
void  PP(UINT4 x ,UINT4 s,UINT4 ac,BYTE mode,BYTE i)
{	 
 	//(a[i%4]) += P ((a[(i+1)%4]), (a[(i+2)%4]), (a[(i+3)%4]),(mode)) + (x) + (UINT4)(ac); 
	(a[i%4]) += P_ex ((a[(i+1)%4]), (a[(i+2)%4]), (a[(i+3)%4]),(mode)) + (x) + (UINT4)(ac); 
 	(a[i%4]) = ROTATE_LEFT ((a[i%4]), (s)); 
 	(a[i%4]) += (a[(i+1)%4]); 
    
}

#define MD5_memcpy memcpy
#define MD5_memset memset
/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */


/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (UINT4 *output,unsigned char *input,unsigned char len)
{
  unsigned char idata i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (unsigned char *output,UINT4 *input,unsigned char len)
{
  unsigned char idata i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}
/* MD5 basic transformation. Transforms state based on block.
 */
 UWORD code T1[] = {
 0xd76aa478,
 0xe8c7b756,
 0x242070db,
 0xc1bdceee,
 0xf57c0faf,
 0x4787c62a,
 0xa8304613,
 0xfd469501,
 0x698098d8,
 0x8b44f7af,
 0xffff5bb1,
 0x895cd7be,
 0x6b901122,
 0xfd987193,
 0xa679438e,
 0x49b40821,
 };

UWORD code T2[] = {
0xf61e2562,
0xc040b340,
0x265e5a51,
0xe9b6c7aa,
0xd62f105d,
0x2441453,
0xd8a1e681,
0xe7d3fbc8,
0x21e1cde6,
0xc33707d6,
0xf4d50d87,
0x455a14ed,
0xa9e3e905,
0xfcefa3f8,
0x676f02d9,
0x8d2a4c8a,
 };

UWORD code T3[] ={
0xfffa3942,
0x8771f681,
0x6d9d6122,
0xfde5380c,
0xa4beea44,
0x4bdecfa9,
0xf6bb4b60,
0xbebfbc70,
0x289b7ec6,
0xeaa127fa,
0xd4ef3085,
0x4881d05,
0xd9d4d039,
0xe6db99e5,
0x1fa27cf8,
0xc4ac5665,

};

UWORD code T4[]={
 0xf4292244,
 0x432aff97,
 0xab9423a7,
 0xfc93a039,
 0x655b59c3,
 0x8f0ccc92,
 0xffeff47d,
 0x85845dd1,
 0x6fa87e4f,
 0xfe2ce6e0,
 0xa3014314,
 0x4e0811a1,
 0xf7537e82,
 0xbd3af235,
 0x2ad7d2bb,
 0xeb86d391,
 };


BYTE xdata S2[4] ;
BYTE xdata S3[4] ;
BYTE xdata S4[4];
static void MD5Transform (UINT4 state[4],BYTE * block)
{
    BYTE  idata i,j,k,n;
    UINT4 xdata x[16];

    S2[0] = 5;   S2[1] = 9; 	S2[2] = 14;  S2[3] = 20;

    S3[0] = 4;  S3[1] = 11;     S3[2] = 16;  S3[3] = 23;

    S4[0] = 6; 	S4[1] = 10;     S4[2] = 15;  S4[3] = 21;

    for(i = 0;i <4 ;i++)
    {	
        a[i] = state[i];
    }


   Decode (x, block, 64);


   /* Round 1 */

  for(i=0;i<16;i ++ )
  {
	 k = i%4;
	 PP(x[i],   7+(k)*5,  T1[i],1,4-k);
  }

   /* Round 2 */
  for(i=0;i<16;i++)
  {
	k = i%4;
	j = i/4;
	n = (4*j +1+k*5)&0x0F;

	PP(x[n],   S2[k],  T2[i],2,4-k);
  }

   /* Round 3 */
  for(i=0;i<16;i++)
  {	
	 k = i%4;
	 j = i/4;
	 n = (21 +k*3 - 4*j )&0x0F;

	 PP (x[ n], S3[k], T3[i],3,4-k);
  }

   /* Round 4 */
  for(i=0;i<16;i++)
  {
	 k = i%4;
	 j = i/4;
	 n = (16 +k*7 - 4*j )&0x0F;

	 PP (x[n], S4[k], T4[i],4,4-k);
  }

  for(i = 0;i <4 ;i++)
  {	
 	 state[i] += a[i];
  }

  /* Zeroize sensitive information.
   */
  MD5_memset ((POINTER)x, 0, sizeof (x));
}


/* Note: Replace "for loop" with standard memcpy if possible.

static void MD5_memcpy (POINTER output,POINTER input,unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    output[i] = input[i];
}
*/
/* Note: Replace "for loop" with standard memset if possible.
static void MD5_memset (POINTER output,int value,unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
 ((char *)output)[i] = (char)value;
}
 */
void MD5Init (MD5_CTX *context)         /* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.

  tested by WG 20070627
 */

void MD5Update (MD5_CTX *context,unsigned char *input, WORD inputLen,BYTE mode,UWORD fileAddr)
//void MD5Update (MD5_CTX *context,unsigned char *input, WORD inputLen,BYTE mode)
{
	BYTE idata index, partLen;
	WORD idata 		i; 				// 'i' is changed from 'unsigned char' to 'WORD' by WG, 20070627

	//Compute number of bytes mod 64 
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	//Update number of bits
	if (((context->count[0]) += ((UINT4)inputLen << 3))< ((UINT4)inputLen << 3))
		(context->count[1]) ++;
	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	//Transform as many times as possible.

	if (mode == 0)
	{
		if (inputLen >= partLen) 
		{
			MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);

			MD5Transform ((UINT4 *)context->state, (unsigned char xdata *)context->buffer);

			for (i = partLen; i + 63 < inputLen; i += 64)
				MD5Transform ((UINT4 *)context->state, (unsigned char*)&input[i]);

			index = 0;
		}
		else
		{
			i = 0;
		}
		MD5_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i],inputLen-i);
	}
	else
	{
		if (inputLen >= partLen) 
		{

#ifndef MD5RAM
			ReadTFBuf(fileAddr, 2, &RecData[0], partLen);
#else
			memcpy(&RecData[0],&G_RamMD5[2],partLen);
#endif

			MD5_memcpy((POINTER)&context->buffer[index], (POINTER)RecData, partLen);

			MD5Transform ((UINT4 *)context->state, (unsigned char *)context->buffer);

			for (i = partLen; i + 63 < inputLen; i += 64)
			{

#ifndef MD5RAM
				ReadTFBuf(fileAddr, (2+i), &RecData[0], partLen);
#else
				memcpy(&RecData[0],&G_RamMD5[2+i],partLen);
#endif

				MD5Transform ((UINT4 *)context->state, (unsigned char*)&RecData[0]);
			}
			index = 0;
		}
		else
		{
			i = 0;
		}
		
#ifndef MD5RAM
		ReadTFBuf(fileAddr,(2+i), &RecData[0], inputLen-i);
#else
		memcpy(&RecData[0],&G_RamMD5[2+i],inputLen-i);
#endif

		MD5_memcpy((POINTER)&context->buffer[index], (POINTER)&RecData[0],inputLen-i);
	} 
}
/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (unsigned char digest[16],MD5_CTX *context)
{
  unsigned char xdata bits[8];
  unsigned char idata index, padLen;

  /* Save number of bits */
  Encode (bits, (UINT4 *)context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);

  MD5Update (context, PADDING, padLen,0,0);

  /* Append length (before padding) */

  MD5Update (context, bits, 8,0,0);
  /* Store state in digest */
  Encode (digest, (UINT4 *)context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}
/*************************************MD5***************************************
验证输出
input：输入的数据
len：输入的长度
output_authsign：输出16字节的结果
********************************************************************************/
void Auth_MD5(UWORD fileAddr,WORD len,unsigned char * output_authsign)
//void Auth_MD5(WORD len,unsigned char * output_authsign)
{
  MD5_CTX xdata context;

  MD5Init (&context);

  MD5Update (&context, 0, len,0x01,fileAddr);

  MD5Final (output_authsign, &context);
}
#endif

