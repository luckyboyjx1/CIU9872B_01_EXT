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

#include "Md5.h"
#include "string.h"
#include "typedef.h"
#include "Main.h"

/* Constants for MD5Transform routine.
 */

// __align(4) MD5_CTX  xdata G_MD5_Context;

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

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

UINT4 sub_PP(UINT4 x,UINT4 y,UINT4 z,BYTE mode)
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
void  PP(UINT4 x ,UINT4 s,UINT4 ac,BYTE mode,BYTE i,UINT4 state[4])
{	 
 	
	(state[i%4]) += sub_PP ((state[(i+1)%4]), (state[(i+2)%4]), (state[(i+3)%4]),(mode)) + (x) + (UINT4)(ac); 
 	(state[i%4]) = ROTATE_LEFT ((state[i%4]), (s)); 
 	(state[i%4]) += (state[(i+1)%4]);

    
}
#define MD5_memcpy memcpy
#define MD5_memset memset


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



	
//static void MD5Transform (UINT4 state[4],unsigned char block[64])
static void MD5Transform (UINT4 state[4],unsigned char block[64])
{
	
	BYTE  idata i,j,k,n;
	UINT4 xdata x[16];	
	BYTE xdata S2[4] ;
	BYTE xdata S3[4] ;
	BYTE xdata S4[4];
	UINT4 xdata buffer_a[4] ;

	S2[0] = 5;   S2[1] = 9; 	S2[2] = 14;  S2[3] = 20;

	S3[0] = 4;  S3[1] = 11;     S3[2] = 16;  S3[3] = 23;

	S4[0] = 6; 	S4[1] = 10;     S4[2] = 15;  S4[3] = 21;
	
	  
	for(i = 0;i <4 ;i++)
	{	
	 	buffer_a[i] =state[i];
	}

  
 	//Decode (x, block, 64);
	for (i = 0, j = 0; j < 64; i++, j += 4)
	{
		x[i] = ((UINT4)block[j]) | (((UINT4)block[j+1]) << 8) |
		(((UINT4)block[j+2]) << 16) | (((UINT4)block[j+3]) << 24);
	}

   /* Round 1 */

  for(i=0;i<16;i ++ )
  {
	 k = i%4;
	 PP(x[i],   7+(k)*5,  T1[i],1,4-k,buffer_a);
  }

   /* Round 2 */
  for(i=0;i<16;i++)
  {
	k = i%4;
	j = i/4;
	n = (4*j +1+k*5)&0x0F;

	PP(x[n],   S2[k],  T2[i],2,4-k,buffer_a);
  }

   /* Round 3 */
  for(i=0;i<16;i++)
  {	
	 k = i%4;
	 j = i/4;
	 n = (21 +k*3 - 4*j )&0x0F;

	 PP (x[ n], S3[k], T3[i],3,4-k,buffer_a);
  }

   /* Round 4 */
  for(i=0;i<16;i++)
  {
	 k = i%4;
	 j = i/4;
	 n = (16 +k*7 - 4*j )&0x0F;

	 PP (x[n], S4[k], T4[i],4,4-k,buffer_a);
  }

 for(i = 0;i <4 ;i++)
 {	
 	 state[i] += buffer_a[i];
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
  
  memset((BYTE*)context,0,sizeof(MD5_CTX));	   //ST卡片在写E2时 遇到未初始化的变量会死卡 ，为避免后续写入E2时出现死卡问题 将context初始化为0再init成需要的值

  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;

}

/*
*@description: 计算MD5算法的中间数据. MD5 block update operation. Continues an MD5 message-digest operation, processing another message block, and updating the  context.
*
*fucntion    : MD5Update
*
*@param      : MD5_CTX *context        MD5算法的数据结构
*@param      : unsigned char *input    输入的源数据的起始ram地址
*@param      : unsigned int inputLen   输入数据的长度
*@param      : BYTE mode               0表示源数据来自于ram数组；此时fileAddr无意义
*                                      1表示源数据来自于E2地址; 此时input无意义
*@param      : UWORD fileAddr          输入的源数据的起始E2地址
*
*return      : void
*
*@history    : created by WG@20070627
*
*/
void MD5Update (MD5_CTX *context,unsigned char *input, unsigned int inputLen, BYTE mode, UWORD fileAddr)
{
	
	unsigned char 	idata index, partLen;
	WORD idata 		i; 				// 'i' is changed from 'unsigned char' to 'WORD' by WG, 20070627
	BYTE xdata     tmpData[64];
	
	if(inputLen == 0)
	{
		return;
	}

	//Compute number of bytes mod 64 
	index = (unsigned int)((context -> count[0] >> 3) & 0x3F);

	//Update number of bits
	if (((context ->count[0]) += ((UINT4)inputLen << 3))< ((UINT4)inputLen << 3))
		(context ->count[1]) ++;
	context ->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	//Transform as many times as possible.

	if (mode == 0)
	{
		if (inputLen >= partLen) 
		{
			MD5_memcpy((POINTER)&context ->buffer[index], (POINTER)input, partLen);


			MD5Transform ((UINT4 *)context->state, (unsigned char *)context->buffer);

			for (i = partLen; i + 63 < inputLen; i += 64)
			{
				MD5Transform ((UINT4 *)context->state, (unsigned char*)&input[i]);
			}
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
			ReadEeprom((POINTER)&context ->buffer[index], fileAddr, partLen);
	
			MD5Transform ((UINT4 *)context->state, (unsigned char *)context->buffer);
	
			for (i = partLen; i + 63 < inputLen; i += 64)
			{
				ReadEeprom(tmpData, fileAddr+i, partLen);

				MD5Transform ((UINT4 *)context->state, (unsigned char*)tmpData);
			}
			index = 0;
		}
		else
		{
			i = 0;
		}

		ReadEeprom((POINTER)&context ->buffer[index], fileAddr+i, inputLen-i);
	}
	 
}
/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (MD5_CTX *context,BYTE * digest/*[16]*/)
{
	
	unsigned char idata index, padLen;
	unsigned char xdata bits[8];
	unsigned char xdata tmpData[64];

	/* Save number of bits */
	for (index = 0, padLen = 0; padLen < 8; index++, padLen+= 4) 
	{
		bits[padLen] = (unsigned char)(context->count[index] & 0xff);
		bits[padLen+1] = (unsigned char)((context->count[index] >> 8) & 0xff);
		bits[padLen+2] = (unsigned char)((context->count[index] >> 16) & 0xff);
		bits[padLen+3] = (unsigned char)((context->count[index] >> 24) & 0xff);
	}

	/* Pad out to 56 mod 64.*/
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);

	memset(tmpData, 0x00, sizeof(tmpData));

	tmpData[0] = 0x80;

	MD5Update (context,tmpData, padLen,0,0);

	/* Append length (before padding) */
	MD5Update (context,bits, 8,0,0);
	
	/* Store state in digest */
	for (index = 0, padLen = 0; padLen < 16; index++, padLen+= 4) 
	{
		digest[padLen] = (unsigned char)(context->state[index] & 0xff);
		digest[padLen+1] = (unsigned char)((context->state[index] >> 8) & 0xff);
		digest[padLen+2] = (unsigned char)((context->state[index] >> 16) & 0xff);
		digest[padLen+3] = (unsigned char)((context->state[index] >> 24) & 0xff);
	}

	/* Zeroize sensitive information.*/
	MD5_memset((POINTER)context->state, 0, 16);
	MD5_memset((POINTER)context->count, 0,8);

}

/*
 *@description :this function is used to implement Secure Hash Algorithm1，该函数的使用是独立的，只需要输入源数据和数据长度即可计算出HASH值
 *
 *@param       :unsigned char* digest     输出的HASH值，长度固定为16字节
 *@param       :unsigned char* sourcedata 输入的源数据的起始ram地址
 *@param       :unsigned int length       输入数据的长度
 *@param      : BYTE mode                 0表示源数据来自于ram数组/1表示源数据来自于E2地址
 *@param      : UWORD fileAddr            输入的源数据的起始E2地址
 *      
 */
void MD5DoFinalSingleCall(unsigned char* digest /*[16]*/, unsigned char* sourcedata,unsigned int length, BYTE mode,UWORD fileAddr) 
{
	
	MD5_CTX xdata context;        
			   
	MD5Init(&context);      /*初始化结构体*/
	
	MD5Update(&context,sourcedata, length,mode,fileAddr);

	MD5Final(&context, digest);

}

