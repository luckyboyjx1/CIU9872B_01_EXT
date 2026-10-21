
/*
 * 	 SHA256 Arithmetic
 *
 *   in = 616263
 *
 *   out = ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
 *   
 */
#include <string.h> 
#include "Sha1.h"
#include "typedef.h"
#include "Interface_LTEFromBoot.h"
#include "stkhead.h"

#ifdef SHA256
//#include "VmAccessEeprom.h"

// __align(4) SHA_INFO  xdata G_SHA256_Context;

// static unsigned long A, B, C, D, E, F, G, H;

unsigned char code SHA256_IV[32] = {
	0x6a, 0x09, 0xe6, 0x67,
	0xbb, 0x67, 0xae, 0x85,
	0x3c, 0x6e, 0xf3, 0x72,
	0xa5, 0x4f, 0xf5, 0x3a,
	0x51, 0x0e, 0x52, 0x7f,
	0x9b, 0x05, 0x68, 0x8c,
	0x1f, 0x83, 0xd9, 0xab,
	0x5b, 0xe0, 0xcd, 0x19
};

DWORD code K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

DWORD ROTR( DWORD x, DWORD n)			{ return ( (x >> n) | (x << (32-n)) ); }
DWORD SHR( DWORD x, DWORD n)			{ return ( x >> n ); }
DWORD sigma0( DWORD x)					{ return ( ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3) ); }
DWORD sigma1( DWORD x)					{ return ( ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10) ); }
DWORD SIGMA10( DWORD x)					{ return ( ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22) ); }
DWORD SIGMA11( DWORD x)					{ return ( ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25) ); }
DWORD Ch( DWORD x, DWORD y, DWORD z)	{ return ( (x & y) ^ ( (~x) & z ) ); }
DWORD Maj( DWORD x, DWORD y, DWORD z)	{ return ( (x & y) ^ ( x & z ) ^ (y & z) ); }


static void sha256Hash(SHA_INFO *sha_info)
{
	DWORD T1, T2, A0, B0, C0, D0, E0, F0, G0, H0, W[64];
	int i;
//	DWORD xdata A, B, C, D, E, F, G, H;
	
	/* set the temporary digest values from the current digest,
	using shifts to ensure machine-independence */
// 	A0 = (unsigned long)sha_info->digest[0] << 24;
// 	A0 += (unsigned long)sha_info->digest[1] << 16;
// 	A0 += (unsigned long)sha_info->digest[2] << 8;
// 	A0 += (unsigned long)sha_info->digest[3];
	A0 = ByteBufToUWORD(&(sha_info->digest[0]));
	
// 	B0 = (unsigned long)sha_info->digest[4] << 24;
// 	B0 += (unsigned long)sha_info->digest[5] << 16;
// 	B0 += (unsigned long)sha_info->digest[6] << 8;
// 	B0 += (unsigned long)sha_info->digest[7];
	B0 = ByteBufToUWORD(&(sha_info->digest[4]));
	
// 	C0 = (unsigned long)sha_info->digest[8] << 24;
// 	C0 += (unsigned long)sha_info->digest[9] << 16;
// 	C0 += (unsigned long)sha_info->digest[10] << 8;
// 	C0 += (unsigned long)sha_info->digest[11];
	C0 = ByteBufToUWORD(&(sha_info->digest[8]));
	
// 	D0 = (unsigned long)sha_info->digest[12] << 24;
// 	D0 += (unsigned long)sha_info->digest[13] << 16;
// 	D0 += (unsigned long)sha_info->digest[14] << 8;
// 	D0 += (unsigned long)sha_info->digest[15];
	D0 = ByteBufToUWORD(&(sha_info->digest[12]));
	
// 	E0 = (unsigned long)sha_info->digest[16] << 24;
// 	E0 += (unsigned long)sha_info->digest[17] << 16;
// 	E0 += (unsigned long)sha_info->digest[18] << 8;
// 	E0 += (unsigned long)sha_info->digest[19];
	E0 = ByteBufToUWORD(&(sha_info->digest[16]));
	
// 	F0 = (unsigned long)sha_info->digest[20] << 24;
// 	F0 += (unsigned long)sha_info->digest[21] << 16;
// 	F0 += (unsigned long)sha_info->digest[22] << 8;
// 	F0 += (unsigned long)sha_info->digest[23];
	F0 = ByteBufToUWORD(&(sha_info->digest[20]));
	
// 	G0 = (unsigned long)sha_info->digest[24] << 24;
// 	G0 += (unsigned long)sha_info->digest[25] << 16;
// 	G0 += (unsigned long)sha_info->digest[26] << 8;
// 	G0 += (unsigned long)sha_info->digest[27];
	G0 = ByteBufToUWORD(&(sha_info->digest[24]));
	
// 	H0 = (unsigned long)sha_info->digest[28] << 24;
// 	H0 += (unsigned long)sha_info->digest[29] << 16;
// 	H0 += (unsigned long)sha_info->digest[30] << 8;
// 	H0 += (unsigned long)sha_info->digest[31];
	H0 = ByteBufToUWORD(&(sha_info->digest[28]));
	
	/* save A-H */
// 	A0 = A;
// 	B0 = B;
// 	C0 = C;
// 	D0 = D;
// 	E0 = E;
// 	F0 = F;
// 	G0 = G;
// 	H0 = H;
	
	/* move the data into the first 16 words of W */
	for (i = 0; i < 16; i++)
	{
		W[i] = sha_info->vdata[i];
	}
	for (i = 16; i<64; i++)
	{
		W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];
	}
	
	/* perform the 64 rounds, using the "alternate method" in which
	the later values of W are computed in place */
	for (i = 0; i < 64; i++)
	{
		T1 = H0 + SIGMA11(E0) + Ch(E0,F0,G0) + K[i] + W[i];
		T2 = SIGMA10(A0) + Maj(A0,B0,C0);
		H0=G0;
		G0=F0;
		F0=E0;
		E0=D0 + T1;
		D0=C0;
		C0=B0;
		B0=A0;
		A0=T1 + T2;
	}
	
	/* add in the original values of A-H */
// 	A += A0;
// 	B += B0;
// 	C += C0;
// 	D += D0;
// 	E += E0;
// 	F += F0;
// 	G += G0;
// 	H += H0;
// 	A0 += (unsigned long)sha_info->digest[0] << 24;
// 	A0 += (unsigned long)sha_info->digest[1] << 16;
// 	A0 += (unsigned long)sha_info->digest[2] << 8;
// 	A0 += (unsigned long)sha_info->digest[3];
// 	B0 += (unsigned long)sha_info->digest[4] << 24;
// 	B0 += (unsigned long)sha_info->digest[5] << 16;
// 	B0 += (unsigned long)sha_info->digest[6] << 8;
// 	B0 += (unsigned long)sha_info->digest[7];
// 	C0 += (unsigned long)sha_info->digest[8] << 24;
// 	C0 += (unsigned long)sha_info->digest[9] << 16;
// 	C0 += (unsigned long)sha_info->digest[10] << 8;
// 	C0 += (unsigned long)sha_info->digest[11];
// 	D0 += (unsigned long)sha_info->digest[12] << 24;
// 	D0 += (unsigned long)sha_info->digest[13] << 16;
// 	D0 += (unsigned long)sha_info->digest[14] << 8;
// 	D0 += (unsigned long)sha_info->digest[15];
// 	E0 += (unsigned long)sha_info->digest[16] << 24;
// 	E0 += (unsigned long)sha_info->digest[17] << 16;
// 	E0 += (unsigned long)sha_info->digest[18] << 8;
// 	E0 += (unsigned long)sha_info->digest[19];
// 	F0 += (unsigned long)sha_info->digest[20] << 24;
// 	F0 += (unsigned long)sha_info->digest[21] << 16;
// 	F0 += (unsigned long)sha_info->digest[22] << 8;
// 	F0 += (unsigned long)sha_info->digest[23];
// 	G0 += (unsigned long)sha_info->digest[24] << 24;
// 	G0 += (unsigned long)sha_info->digest[25] << 16;
// 	G0 += (unsigned long)sha_info->digest[26] << 8;
// 	G0 += (unsigned long)sha_info->digest[27];
// 	H0 += (unsigned long)sha_info->digest[28] << 24;
// 	H0 += (unsigned long)sha_info->digest[29] << 16;
// 	H0 += (unsigned long)sha_info->digest[30] << 8;
// 	H0 += (unsigned long)sha_info->digest[31];
	
	A0 += ByteBufToUWORD(&(sha_info->digest[0]));
	
	B0 += ByteBufToUWORD(&(sha_info->digest[4]));
	
	C0 += ByteBufToUWORD(&(sha_info->digest[8]));
	
	D0 += ByteBufToUWORD(&(sha_info->digest[12]));
	
	E0 += ByteBufToUWORD(&(sha_info->digest[16]));
	
	F0 += ByteBufToUWORD(&(sha_info->digest[20]));
	
	G0 += ByteBufToUWORD(&(sha_info->digest[24]));
	
	H0 += ByteBufToUWORD(&(sha_info->digest[28]));

	
	/* save resulting digest, again using shifts to ensure
	machine independence */
// 	sha_info->digest[0] = (unsigned char)(A0 >> 24);
// 	sha_info->digest[1] = (unsigned char)((A0 >> 16) & 0xff);
// 	sha_info->digest[2] = (unsigned char)((A0 >> 8) & 0xff);
// 	sha_info->digest[3] = (unsigned char)(A0 & 0xff);
// 	sha_info->digest[4] = (unsigned char)(B0 >> 24);
// 	sha_info->digest[5] = (unsigned char)((B0 >> 16) & 0xff);
// 	sha_info->digest[6] = (unsigned char)((B0 >> 8) & 0xff);
// 	sha_info->digest[7] = (unsigned char)(B0 & 0xff);
// 	sha_info->digest[8] = (unsigned char)(C0 >> 24);
// 	sha_info->digest[9] = (unsigned char)((C0 >> 16) & 0xff);
// 	sha_info->digest[10] = (unsigned char)((C0 >> 8) & 0xff);
// 	sha_info->digest[11] = (unsigned char)(C0 & 0xff);
// 	sha_info->digest[12] = (unsigned char)(D0 >> 24);
// 	sha_info->digest[13] = (unsigned char)((D0 >> 16) & 0xff);
// 	sha_info->digest[14] = (unsigned char)((D0 >> 8) & 0xff);
// 	sha_info->digest[15] = (unsigned char)(D0 & 0xff);
// 	sha_info->digest[16] = (unsigned char)(E0 >> 24);
// 	sha_info->digest[17] = (unsigned char)((E0 >> 16) & 0xff);
// 	sha_info->digest[18] = (unsigned char)((E0 >> 8) & 0xff);
// 	sha_info->digest[19] = (unsigned char)(E0 & 0xff);
// 	sha_info->digest[20] = (unsigned char)(F0 >> 24);
// 	sha_info->digest[21] = (unsigned char)((F0 >> 16) & 0xff);
// 	sha_info->digest[22] = (unsigned char)((F0 >> 8) & 0xff);
// 	sha_info->digest[23] = (unsigned char)(F0 & 0xff);
// 	sha_info->digest[24] = (unsigned char)(G0 >> 24);
// 	sha_info->digest[25] = (unsigned char)((G0 >> 16) & 0xff);
// 	sha_info->digest[26] = (unsigned char)((G0 >> 8) & 0xff);
// 	sha_info->digest[27] = (unsigned char)(G0 & 0xff);
// 	sha_info->digest[28] = (unsigned char)(H0 >> 24);
// 	sha_info->digest[29] = (unsigned char)((H0 >> 16) & 0xff);
// 	sha_info->digest[30] = (unsigned char)((H0 >> 8) & 0xff);
// 	sha_info->digest[31] = (unsigned char)(H0 & 0xff);
	
	UWORDToByteBuf(A0,&(sha_info->digest[0]));
	UWORDToByteBuf(B0,&(sha_info->digest[4]));
	UWORDToByteBuf(C0,&(sha_info->digest[8]));
	UWORDToByteBuf(D0,&(sha_info->digest[12]));
	UWORDToByteBuf(E0,&(sha_info->digest[16]));
	UWORDToByteBuf(F0,&(sha_info->digest[20]));
	UWORDToByteBuf(G0,&(sha_info->digest[24]));
	UWORDToByteBuf(H0,&(sha_info->digest[28]));
	
	/* clear the data so that further updates can be added in */
	for (i = 0; i < 16; i++)
	{
		sha_info->vdata[i] = 0;
	}
}

/*
*@description:初始化SHA256的结构体SHA_INFO   initialize sha_info
*
*@function   :SHA256Init
*
*@See        :
*
*@param      :SHA_INFO *sha_info        结构体数据
*
*@return     :void
*
*@history    :V1.0 : created 
*             V1.1 : modify by xuxiaojun@20180413  增加函数注释
*/
void SHA256Init(SHA_INFO *sha_info)
{
	int i;
	
	/* set digest to its initial value. Done one char at a time
	to ensure machine independence. */
	for (i = 0; i < DIGEST_LENGTH_256; i++)
	{
		sha_info->digest[i] = SHA256_IV[i];
	}
	
	/* clear data so that updates can be added in */
	for (i = 0; i < 16; i++)
	{
		sha_info->vdata[i] = 0;
	}
	
	/* set bit count to zero */
	sha_info->count[0] = sha_info->count[1] = 0;
}

/*
*@description:SHA256源数据update   update the digest using additional message data
*
*@function   :SHA256Update
*
*@See        :
*
*@param      :SHA_INFO *sha_info        结构体数据
*@param      :unsigned char *buffer     需要被计算的源数据所在数组
*@param      :unsigned long count       需要被计算的数据的长度
*@param      :BYTE mode                 0表示源数据来自于ram数组/1表示源数据来自于E2地址
*@param      :UWORD fileAddr            输入的源数据的起始E2地址, 逻辑地址
*
*@return     :void
*
*@history    :V1.0 : created 
*             V1.1 : modify by xuxiaojun@20180413  在函数内部首先将count = 8*count，避免在外部调用该函数时每次都用8*len，统一到函数内部实现
*/
void SHA256Update(SHA_INFO *sha_info, unsigned char *buffer, unsigned int count, BYTE mode, UWORD fileAddr)
{
	unsigned long data_count, t, mask_size, offset;
	unsigned char *bptr, c, last, mask;
	
	if(count == 0)
	{
		return;
	}
	
	bptr = buffer;  //给个初始值，去去除警告，该值只有mode==0时才会使用
	
	count = count*8;
	offset = 0;
	
	/* enter the message data into the data buffer. When the buffer is full (512 bits entered, update the digest and clear the data buffer for the next update. */
	data_count = sha_info->count[1]%512;
	
	if(mode == 0)
	{
		bptr = buffer + (offset/8);
		
		last = *bptr++; // first fill the current octet of the buffer, so that the bit offset into the buffer is a multiple of 8
	}
	else //if(mode == 1)
	{

		ReadEeprom(&last,fileAddr,1);
		fileAddr = fileAddr+1;

	}
	
	if (data_count%8)
	{
		c = last; // get a full byte from the buffer
		
		if(mode == 0)
		{
			last = *bptr++;
		}
		else //if(mode == 1)
		{

            ReadEeprom(&last,fileAddr,1);
			fileAddr = fileAddr+1;
		}
		
		if (offset%8)
		{
			c <<= (offset%8);
			
			c += last >> (8 - (offset%8));
		}
		
		/* set mask to fill the remaining bits of the octet */
		mask_size = 8 - (data_count%8);
		
		mask = 0xff << (data_count%8);
		
		/* adjust for short count */
		if (count < mask_size)
		{
			mask <<= (mask_size-count);
			mask_size = count;
		}
		
		/* store the bits */
		c = (c & mask) >> (data_count%8);
		
		sha_info->vdata[data_count/32] += (unsigned long)c << 8*(3 - ((data_count%32)/8));
		
		/* update count */
		t = sha_info->count[1];
		
		sha_info->count[1] += mask_size;
		
		if (sha_info->count[1] < t)
		{
			sha_info->count[0]++;
		}
		
		/* if the data buffer is full, update the digest */
		data_count += mask_size;
		
		if (data_count == 512)
		{
			sha256Hash(sha_info);
			data_count = 0;
		}
		
		/* start over with updated offset and count */
		offset += mask_size;
		
		count -= mask_size;
		
		if(mode == 0)
		{
			bptr = buffer + (offset/8);

			last = *bptr++;
		}
		else //if(mode == 1)
		{
            ReadEeprom(&last,fileAddr,1);
			fileAddr = fileAddr+1;
		}
	}
	while (count != 0)
	{
		/* get the next full octet from the buffer */
		c = last;
		
		if(mode == 0)
		{
			last = *bptr++;
		}
		else //if(mode == 1)
		{
			ReadEeprom(&last,fileAddr,1);
			fileAddr = fileAddr+1;
		}
		
		if (offset%8)
		{
			c <<= (offset%8);
			c += last >> (8 - (offset%8));
		}
		
		/* set mask to a full octet */
		mask_size = 8;
		
		mask = 0xff;
		
		/* adjust for short count */
		if (count < mask_size)
		{ 
			mask <<= (mask_size-count);
			mask_size = count;
		}
		
		/* store the bits */
		c &= mask;
		
		sha_info->vdata[data_count/32] += (unsigned long)c << 8*(3 - ((data_count%32)/8));
		
		/* update count */
		t = sha_info->count[1];
		
		sha_info->count[1] += mask_size;
		
		if (sha_info->count[1] < t)
		{
			sha_info->count[0]++;
		}
		
		/* if the data buffer is full, update the digest */
		data_count += mask_size;
		
		if (data_count == 512)
		{
			sha256Hash(sha_info);
			data_count = 0;
		}
		
		count -= mask_size;
	}
}

/*
*@description:结束SHA256计算，输出32字节的digest   add pad bit of '1', zero fill and bit length, then update the digest
*
*@function   :SHA256Final
*
*@See        :
*
*@param      :SHA_INFO *sha_info        结构体数据
*@param      :BYTE * result             32字节的hash值
*
*@return     :void
*
*@history    :V1.0 : created 
*             V1.1 : modify by xuxiaojun@20180413  增加函数注释，修改函数定义和声明
*/
void SHA256Final(SHA_INFO *sha_info, BYTE * result)
{
	sha_info->vdata[(sha_info->count[1]%512)/32] += (1L << (31 - (sha_info->count[1]%32)));		/* add the pad bit of '1' */
	
	if ((sha_info->count[1]%512) == 511)														/* if the data buffer is full, update the digest */
	{
		sha256Hash(sha_info);
	}
	
	/* if there isn't room for 64 bits of bit length, leave the
	buffer zero filled to the end, update the digest and clear
	the buffer. */
	if ((sha_info->count[1]%512) > (512-65))
	{
		sha256Hash(sha_info);
	}
	
	sha_info->vdata[14] = sha_info->count[0];		/* put in the bit length */
	sha_info->vdata[15] = sha_info->count[1];
	
	sha256Hash(sha_info);							/* update the digest */
	
	memcpy(result, &(sha_info->digest[0]), 32);
}

/*
 *@description :this function is used to implement Secure Hash Algorithm1，该函数的使用是独立的，只需要输入源数据和数据长度即可计算出HASH值.
 *在hardware_cryptography.c文件中有实现同样功能的硬算法  SHA256DoFinalSingleCall_Ex(UWORD inAddr,WORD inLen ,BYTE *hashResult), 传入参数后续需要统一
 *
 *@param       :digest                    输出的HASH值，长度固定为32字节
 *@param       :sourcedata                待处理的源数据
 *@param       :length                    待处理的源数据长度 
 *@param       :BYTE mode                 0表示源数据来自于ram数组/1表示源数据来自于E2地址
 *@param       :UWORD fileAddr            输入的源数据的起始E2地址, 逻辑地址
 *      
 */
#if !JAVA_CARD_UICC
//void SHA256DoFinalSingleCall(unsigned char * digest /*[32]*/, unsigned char* sourcedata,unsigned int length, BYTE mode, UWORD fileAddr) 
//{
//	SHA_INFO xdata context;        
//			   
//	SHA256Init(&context);      /*初始化结构体*/

//	SHA256Update(&context,sourcedata,length,mode,fileAddr);

//	SHA256Final(&context, digest);
//}
#endif

#endif
