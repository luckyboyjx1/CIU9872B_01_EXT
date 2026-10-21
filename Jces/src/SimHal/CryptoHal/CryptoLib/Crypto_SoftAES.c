/************************************************************
  Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
  FileName: Crypto_SoftAES.c.
  Advanced Encryption Standard (AES) of the U.S Federal
  Information Processing Standard (FIPS)
  Author: Lizumeng.
  Version : 1.0.
  Date: 2010/04/20.
  Description: This source file implements functions used to
               encrypt or decrypt with AES cryptography.
  History:
      <author>    <time>    <version>    <desc>
      Lizumeng   10/04/20     1.0     build this module
*************************************************************/

#include "Crypto_SoftAES.h"
#include "Sym_API.h"
#include "SysConfigHalRam.h"

#if WITH_SOFT_AES

#if AES_LOOKUPTABLE_CACULATE
/*******************************************************************************
 * Function name: AESKeyExpand
 * Description: AES key expanded using g_AES_Key and g_AES_KeyLen.
 * Input Parameters:   (precondition)
 *      none.
 * Output Parameters:  (postcondition)
 *		none(the expanded key is stored in glable buffer g_AESKeySchedule)
 * NOTE:
 ********************************************************************************/
void SoftAes_AESKeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	u32 * rk, t, i, j;
	u8 AESKeyByteLen = keyBitLen >> 3;
	u8 aESRoundNumber;

	rk = pExternKey;
	aESRoundNumber = (AESKeyByteLen >> 2) + 6;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian((PX8)rk, AESKeyByteLen);
#endif

	i = 0;

	if(keyBitLen == LENGTH_AES_128)
	{
		while(1)
		{
			t = rk[3];
			t = (_ae4[(t >> 16) & 0xFF] & 0xFF000000) ^
				(_ae4[(t >>  8) & 0xFF] & 0x00FF0000) ^
				(_ae4[(t      ) & 0xFF] & 0x0000FF00) ^
				(_ae4[(t >> 24)       ] & 0x000000FF) ^
				 _arc[i];
			rk[4] = (t ^= rk[0]);
			rk[5] = (t ^= rk[1]);
			rk[6] = (t ^= rk[2]);
			rk[7] = (t ^= rk[3]);
			if(++i == 10)
				break;
			rk += 4;
		}
	}
	else if(keyBitLen == LENGTH_AES_192)
	{
		while(1)
		{
			t = rk[5];
			t = (_ae4[(t >> 16) & 0xFF] & 0xFF000000) ^
				(_ae4[(t >>  8) & 0xFF] & 0x00FF0000) ^
				(_ae4[(t      ) & 0xFF] & 0x0000FF00) ^
				(_ae4[(t >> 24)       ] & 0x000000ff) ^
				 _arc[i];
			rk[6] = (t ^= rk[0]);
			rk[7] = (t ^= rk[1]);
			rk[8] = (t ^= rk[2]);
			rk[9] = (t ^= rk[3]);
			if(++i == 8)
				break;
			rk[10] = (t ^= rk[4]);
			rk[11] = (t ^= rk[5]);
			rk += 6;
		}
	}
	else if(keyBitLen == LENGTH_AES_256)
	{
		while(1)
		{
			t = rk[7];
			t = (_ae4[(t >> 16) & 0xFF] & 0xFF000000) ^
				(_ae4[(t >>  8) & 0xFF] & 0x00FF0000) ^
				(_ae4[(t      ) & 0xFF] & 0x0000FF00) ^
				(_ae4[(t >> 24)       ] & 0x000000FF) ^
				 _arc[i];
			rk[8] = (t ^= rk[0]);
			rk[9] = (t ^= rk[1]);
			rk[10] = (t ^= rk[2]);
			rk[11] = (t ^= rk[3]);
			if(++i == 7)
				break;
			t = (_ae4[(t >> 24)       ] & 0xFF000000) ^
				(_ae4[(t >> 16) & 0xFF] & 0x00FF0000) ^
				(_ae4[(t >>  8) & 0xFF] & 0x0000FF00) ^
				(_ae4[(t      ) & 0xFF] & 0x000000FF);
			rk[12] = (t ^= rk[4]);
			rk[13] = (t ^= rk[5]);
			rk[14] = (t ^= rk[6]);
			rk[15] = (t ^= rk[7]);
			rk += 8;
		}
	}

	if(cryptoMode == MODE_DECRYPT)
	{
		rk = pExternKey;

		for(i = 0, j = (aESRoundNumber << 2); i < j; i += 4, j -= 4)
		{
			t = rk[i  ]; rk[i  ] = rk[j  ]; rk[j  ] = t;
			t = rk[i+1]; rk[i+1] = rk[j+1]; rk[j+1] = t;
			t = rk[i+2]; rk[i+2] = rk[j+2]; rk[j+2] = t;
			t = rk[i+3]; rk[i+3] = rk[j+3]; rk[j+3] = t;
		}
		for(i = 1; i < aESRoundNumber; i++)
		{
			rk += 4;
			rk[0] =
				_ad0[_ae4[(rk[0] >> 24)       ] & 0xFF] ^
				_ad1[_ae4[(rk[0] >> 16) & 0xFF] & 0xFF] ^
				_ad2[_ae4[(rk[0] >>  8) & 0xFF] & 0xFF] ^
				_ad3[_ae4[(rk[0]      ) & 0xFF] & 0xFF];
			rk[1] =
				_ad0[_ae4[(rk[1] >> 24)       ] & 0xFF] ^
				_ad1[_ae4[(rk[1] >> 16) & 0xFF] & 0xFF] ^
				_ad2[_ae4[(rk[1] >>  8) & 0xFF] & 0xFF] ^
				_ad3[_ae4[(rk[1]      ) & 0xFF] & 0xFF];
			rk[2] =
				_ad0[_ae4[(rk[2] >> 24)       ] & 0xFF] ^
				_ad1[_ae4[(rk[2] >> 16) & 0xFF] & 0xFF] ^
				_ad2[_ae4[(rk[2] >>  8) & 0xFF] & 0xFF] ^
				_ad3[_ae4[(rk[2]      ) & 0xFF] & 0xFF];
			rk[3] =
				_ad0[_ae4[(rk[3] >> 24)       ] & 0xFF] ^
				_ad1[_ae4[(rk[3] >> 16) & 0xFF] & 0xFF] ^
				_ad2[_ae4[(rk[3] >>  8) & 0xFF] & 0xFF] ^
				_ad3[_ae4[(rk[3]      ) & 0xFF] & 0xFF];
		}
	}
}

static void etfs(u32 * t0, u32 * t1, u32 * t2, u32 * t3, u32 s0, u32 s1, u32 s2, u32 s3, u32 * rk, u8 i)
{
	* (u32 *)t0 = _ae0[(s0 >> 24)] ^ _ae1[(s1 >> 16) & 0xFF] ^ _ae2[(s2 >>  8) & 0xFF] ^ _ae3[(s3) & 0xFF] ^ rk[i+0];
	* (u32 *)t1 = _ae0[(s1 >> 24)] ^ _ae1[(s2 >> 16) & 0xFF] ^ _ae2[(s3 >>  8) & 0xFF] ^ _ae3[(s0) & 0xFF] ^ rk[i+1];
	* (u32 *)t2 = _ae0[(s2 >> 24)] ^ _ae1[(s3 >> 16) & 0xFF] ^ _ae2[(s0 >>  8) & 0xFF] ^ _ae3[(s1) & 0xFF] ^ rk[i+2];
	* (u32 *)t3 = _ae0[(s3 >> 24)] ^ _ae1[(s0 >> 16) & 0xFF] ^ _ae2[(s1 >>  8) & 0xFF] ^ _ae3[(s2) & 0xFF] ^ rk[i+3];
}

static void dtfs(u32 * t0, u32 * t1, u32 * t2, u32 * t3, u32 s0, u32 s1, u32 s2, u32 s3, u32 * rk, u8 i)
{
	* (u32 *)t0 = _ad0[(s0 >> 24)] ^ _ad1[(s3 >> 16) & 0xFF] ^ _ad2[(s2 >>  8) & 0xFF] ^ _ad3[(s1) & 0xFF] ^ rk[i+0];
	* (u32 *)t1 = _ad0[(s1 >> 24)] ^ _ad1[(s0 >> 16) & 0xFF] ^ _ad2[(s3 >>  8) & 0xFF] ^ _ad3[(s2) & 0xFF] ^ rk[i+1];
	* (u32 *)t2 = _ad0[(s2 >> 24)] ^ _ad1[(s1 >> 16) & 0xFF] ^ _ad2[(s0 >>  8) & 0xFF] ^ _ad3[(s3) & 0xFF] ^ rk[i+2];
	* (u32 *)t3 = _ad0[(s3 >> 24)] ^ _ad1[(s2 >> 16) & 0xFF] ^ _ad2[(s1 >>  8) & 0xFF] ^ _ad3[(s0) & 0xFF] ^ rk[i+3];
}

static void esft(u32 t0, u32 t1, u32 t2, u32 t3, u32 * s0, u32 * s1, u32 * s2, u32 * s3, u32 * rk, u8 i)
{
	* (u32 *)s0 = _ae0[(t0 >> 24)] ^ _ae1[(t1 >> 16) & 0xFF] ^ _ae2[(t2 >>  8) & 0xFF] ^ _ae3[(t3) & 0xFF] ^ rk[i+0];
	* (u32 *)s1 = _ae0[(t1 >> 24)] ^ _ae1[(t2 >> 16) & 0xFF] ^ _ae2[(t3 >>  8) & 0xFF] ^ _ae3[(t0) & 0xFF] ^ rk[i+1];
	* (u32 *)s2 = _ae0[(t2 >> 24)] ^ _ae1[(t3 >> 16) & 0xFF] ^ _ae2[(t0 >>  8) & 0xFF] ^ _ae3[(t1) & 0xFF] ^ rk[i+2];
	* (u32 *)s3 = _ae0[(t3 >> 24)] ^ _ae1[(t0 >> 16) & 0xFF] ^ _ae2[(t1 >>  8) & 0xFF] ^ _ae3[(t2) & 0xFF] ^ rk[i+3];
}

static void dsft(u32 t0, u32 t1, u32 t2, u32 t3, u32 * s0, u32 * s1, u32 * s2, u32 * s3, u32 * rk, u8 i)
{
	* (u32 *)s0 = _ad0[(t0 >> 24)] ^ _ad1[(t3 >> 16) & 0xFF] ^ _ad2[(t2 >>  8) & 0xFF] ^ _ad3[(t1) & 0xFF] ^ rk[i+0];
	* (u32 *)s1 = _ad0[(t1 >> 24)] ^ _ad1[(t0 >> 16) & 0xFF] ^ _ad2[(t3 >>  8) & 0xFF] ^ _ad3[(t2) & 0xFF] ^ rk[i+1];
	* (u32 *)s2 = _ad0[(t2 >> 24)] ^ _ad1[(t1 >> 16) & 0xFF] ^ _ad2[(t0 >>  8) & 0xFF] ^ _ad3[(t3) & 0xFF] ^ rk[i+2];
	* (u32 *)s3 = _ad0[(t3 >> 24)] ^ _ad1[(t2 >> 16) & 0xFF] ^ _ad2[(t1 >>  8) & 0xFF] ^ _ad3[(t0) & 0xFF] ^ rk[i+3];
}

static void elr(u32 t0, u32 t1, u32 t2, u32 t3, u32 * s0, u32 * s1, u32 * s2, u32 * s3, u32 * rk)
{
	* (u32 *)s0 = (_ae4[(t0 >> 24)] & 0xFF000000) ^ (_ae4[(t1 >> 16) & 0xFF] & 0x00FF0000) ^ (_ae4[(t2 >>  8) & 0xFF] & 0x0000FF00) ^ (_ae4[(t3) & 0xFF] & 0x000000FF) ^ rk[0];
	* (u32 *)s1 = (_ae4[(t1 >> 24)] & 0xFF000000) ^ (_ae4[(t2 >> 16) & 0xFF] & 0x00FF0000) ^ (_ae4[(t3 >>  8) & 0xFF] & 0x0000FF00) ^ (_ae4[(t0) & 0xFF] & 0x000000FF) ^ rk[1];
	* (u32 *)s2 = (_ae4[(t2 >> 24)] & 0xFF000000) ^ (_ae4[(t3 >> 16) & 0xFF] & 0x00FF0000) ^ (_ae4[(t0 >>  8) & 0xFF] & 0x0000FF00) ^ (_ae4[(t1) & 0xFF] & 0x000000FF) ^ rk[2];
	* (u32 *)s3 = (_ae4[(t3 >> 24)] & 0xFF000000) ^ (_ae4[(t0 >> 16) & 0xFF] & 0x00FF0000) ^ (_ae4[(t1 >>  8) & 0xFF] & 0x0000FF00) ^ (_ae4[(t2) & 0xFF] & 0x000000FF) ^ rk[3];
}

static void dlr(u32 t0, u32 t1, u32 t2, u32 t3, u32 * s0, u32 * s1, u32 * s2, u32 * s3, u32 * rk)
{
	* (u32 *)s0 = (_ad4[(t0 >> 24)] & 0xFF000000) ^ (_ad4[(t3 >> 16) & 0xFF] & 0x00FF0000) ^ (_ad4[(t2 >>  8) & 0xFF] & 0x0000FF00) ^ (_ad4[(t1) & 0xFF] & 0x000000FF) ^ rk[0];
	* (u32 *)s1 = (_ad4[(t1 >> 24)] & 0xFF000000) ^ (_ad4[(t0 >> 16) & 0xFF] & 0x00FF0000) ^ (_ad4[(t3 >>  8) & 0xFF] & 0x0000FF00) ^ (_ad4[(t2) & 0xFF] & 0x000000FF) ^ rk[1];
	* (u32 *)s2 = (_ad4[(t2 >> 24)] & 0xFF000000) ^ (_ad4[(t1 >> 16) & 0xFF] & 0x00FF0000) ^ (_ad4[(t0 >>  8) & 0xFF] & 0x0000FF00) ^ (_ad4[(t3) & 0xFF] & 0x000000FF) ^ rk[2]; \
	* (u32 *)s3 = (_ad4[(t3 >> 24)] & 0xFF000000) ^ (_ad4[(t2 >> 16) & 0xFF] & 0x00FF0000) ^ (_ad4[(t1 >>  8) & 0xFF] & 0x0000FF00) ^ (_ad4[(t0) & 0xFF] & 0x000000FF) ^ rk[3];
}

/*******************************************************************************
 * Function name: AESEncryptCore
 * Description: AES encryption.
 * Input Parameters:   (precondition)
 *      none(both input buffer and output buffer are g_AES_TempInBuffer)
 * Output Parameters:  (postcondition)
 *		none
 * NOTE:
 ********************************************************************************/
static void AESEncryptCore(u8 * pDataIn , PX32 pExternKey,u16 keyBitLen)
{
	u32 t0, t1, t2, t3;
	u8 aESRoundNumber = (keyBitLen >> 5) + 6;
	u32 * rk = pExternKey;
	u32 * src;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
#endif

	src = (u32 *)pDataIn;

	src[0] = src[0] ^ rk[0];
	src[1] = src[1] ^ rk[1];
	src[2] = src[2] ^ rk[2];
	src[3] = src[3] ^ rk[3];

#define s0 src[0]
#define s1 src[1]
#define s2 src[2]
#define s3 src[3]

//	etfs(4);		/* round 1 */
	etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 4);
//	esft(8);		/* round 2 */
	esft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 8);
//	etfs(12);		/* round 3 */
	etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 12);
//	esft(16);		/* round 4 */
	esft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 16);
//	etfs(20);		/* round 5 */
	etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 20);
//	esft(24);		/* round 6 */
	esft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 24);
//	etfs(28);		/* round 7 */
	etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 28);
//	esft(32);		/* round 8 */
	esft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 32);
//	etfs(36);		/* round 9 */
	etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 36);

	if(aESRoundNumber > 10)
	{
//		esft(40);	/* round 10 */
		esft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 40);
//		etfs(44);	/* round 11 */
		etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 44);
		if(aESRoundNumber > 12)
		{
//			esft(48);	/* round 12 */
			esft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 48);
//			etfs(52);	/* round 13 */
			etfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 52);
		}
	}

	rk += (aESRoundNumber << 2);

//	elr(); /* last round */
	elr(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk);
#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
#endif
}
#if	SOFT_AES_DECRYPTO
/*******************************************************************************
 * Function name: AESDecryptCore
 * Description: AES decryption.
 * Input Parameters:   (precondition)
 *      none(both input buffer and output buffer are g_AES_TempInBuffer)
 * Output Parameters:  (postcondition)
 *		none
 * NOTE:
 ********************************************************************************/
static void AESDecryptCore(u8 * pDataIn , PX32 pExternKey, u16 keyBitLen)
{
	u32 t0, t1, t2, t3;
	u8 aESRoundNumber = (keyBitLen >> 5) + 6;
	u32 * rk = pExternKey;
	u32 * src;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
#endif

	src = (u32 *)pDataIn;

	src[0] = src[0] ^ rk[0];
	src[1] = src[1] ^ rk[1];
	src[2] = src[2] ^ rk[2];
	src[3] = src[3] ^ rk[3];

#define s0 src[0]
#define s1 src[1]
#define s2 src[2]
#define s3 src[3]

//	dtfs(4);		/* round 1 */
	dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 4);
//	dsft(8);		/* round 2 */
	dsft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 8);
//	dtfs(12);		/* round 3 */
	dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 12);
//	dsft(16);		/* round 4 */
	dsft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 16);
//	dtfs(20);		/* round 5 */
	dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 20);
//	dsft(24);		/* round 6 */
	dsft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 24);
//	dtfs(28);		/* round 7 */
	dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 28);
//	dsft(32);		/* round 8 */
	dsft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 32);
//	dtfs(36);		/* round 9 */
	dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 36);

	if(aESRoundNumber > 10)
	{
//		dsft(40);	/* round 10 */
		dsft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 40);
//		dtfs(44);	/* round 11 */
		dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 44);
		if(aESRoundNumber > 12)
		{
//			dsft(48);	/* round 12 */
			dsft(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk, 48);
//			dtfs(52);	/* round 13 */
			dtfs(&t0, &t1, &t2, &t3, s0, s1, s2, s3, rk, 52);
		}
	}

	rk += (aESRoundNumber << 2);

//	dlr(); /* last round */
	dlr(t0, t1, t2, t3, &s0, &s1, &s2, &s3, rk);

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
#endif
}
#endif
#endif //AES_LOOKUPTABLE_CACULATE

#if AES_BIT_CACULATE

/*Matrix Sbox for the Sbox operation in encryption procedure*/
const unsigned char SBOX[256]={  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
			    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
			    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
			    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
			    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
			    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
			    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
			    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
			    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
			    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
			    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
			    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
			    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
			    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
			    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
			    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

const unsigned char Rcon[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};




// -------------------------------------------------------------------------------
/*Multiply for 2 a byte */
//if ((State2[index] & 0x80)!=0) {State2[index]= (State2[index] << 1) ^ 0x1b;}
//else State2[index]= (State2[index] << 1) ^0;
// -------------------------------------------------------------------------------

void xtime(PX8 State2, u8 index)
{
	if ((State2[index] & 0x80)!=0)
	{
		State2[index]= (State2[index] << 1) ^ 0x1b;
	}
	else
	{
		State2[index]= (State2[index] << 1) ^0;
	}
}

//-------------------------------------------------------------------------------
// void MixColumns(void)
// Compute State= Mixcolumn(State2)
//-------------------------------------------------------------------------------
void MixColumns(PX8 State, PX8 State2)
{
	unsigned char t0;

	t0=State2[2]^State2[3];		// t0 = s2 xor s3
	State[0]=t0 ^ State2[1];	// s0' = s1 xor s2 xor s3 = s1 xor t0
	State[1]=t0 ^ State2[0];	// s1' = s0 xor s2 xor s3 = s0 xor t0

	t0=State2[0]^State2[1];		// t0 = s0 xor s1
	State[2]=t0 ^ State2[3];	// s2' = s0 xor s1 xor s3 = t0 xor s3
	State[3]=t0 ^ State2[2];	// s3' = s0 xor s1 xor s2 = t0 xor s2

	t0=State2[6]^State2[7];		// t0 = s6 xor s7
	State[4]=t0 ^ State2[5];	// s4' = s5 xor s6 xor s7 = s5 xor t0
	State[5]=t0 ^ State2[4];	// s5' = s4 xor s6 xor s7 = s4 xor t0

	t0=State2[4]^State2[5];		// t0 = s4 xor s5
	State[6]=t0 ^ State2[7];	// s6' = s4 xor s5 xor s7 = t0 xor s7
	State[7]=t0 ^ State2[6];	// s7' = s4 xor s5 xor s6 = t0 xor s6

	t0=State2[10]^State2[11];	// t0 = s10 xor s11
	State[8]=t0 ^ State2[9];	// s8' = s9 xor s10 xor s11 = t0 xor s9
	State[9]=t0 ^ State2[8];	// s9' = s8 xor s10 xor s11 = t0 xor s8

	t0=State2[8]^State2[9];		// t0 = s8 xor s9
	State[10]=t0 ^ State2[11];	// s10' = s8 xor s9 xor s11 = t0 xor s11
	State[11]=t0 ^ State2[10];	// s11' = s8 xor s9 xor s10 = t0 xor s10

	t0=State2[14]^State2[15];	// t0 = s14 xor s15
	State[12]=t0 ^ State2[13];	// s12' = s13 xor s14 xor s15 = t0 xor s13
	State[13]=t0 ^ State2[12];	// s13' = s12 xor s14 xor s15 = t0 xor s12

	t0=State2[12]^State2[13];	// t0 = s12 xor s13
	State[14]=t0 ^ State2[15];	// s14' = s12 xor s13 xor s15 = t0 xor s15
	State[15]=t0 ^ State2[14];	// s15' = s12 xor s13 xor s14 = t0 xor s14

// compute {02}.s0, {02}.s1,... {02}.s15 in State2
//	for (i=15; i>0;i--) State2[i]=xtime(State2[i]);
	xtime(State2, 0);
	xtime(State2, 1);
	xtime(State2, 2);
	xtime(State2, 3);
	xtime(State2, 4);
	xtime(State2, 5);
	xtime(State2, 6);
	xtime(State2, 7);
	xtime(State2, 8);
	xtime(State2, 9);
	xtime(State2, 10);
	xtime(State2, 11);
	xtime(State2, 12);
	xtime(State2, 13);
	xtime(State2, 14);
	xtime(State2, 15);

	State[0]^=State2[0]^State2[1];  //s0' = s0' ^ ({02}.s1) xor ({02}.s0)
	State[1]^=State2[1]^State2[2];  //s1' = s1' ^ ({02}.s1) xor ({02}.s2)
	State[2]^=State2[2]^State2[3];  //s2' = s2' ^ ({02}.s2) xor ({02}.s3)
	State[3]^=State2[3]^State2[0];  //s3' = s3' ^ ({02}.s3) xor ({02}.s0)

//	 compute final s4, s5, s6, s7
	State[4]^=State2[4]^State2[5];  //s4' = s4' ^ ({02}.s4) xor ({02}.s5)
	State[5]^=State2[5]^State2[6];  //s5' = s5' ^ ({02}.s5) xor ({02}.s6)
	State[6]^=State2[6]^State2[7];  //s6' = s6' ^ ({02}.s6) xor ({02}.s7)
	State[7]^=State2[7]^State2[4];  //s7' = s7' ^ ({02}.s7) xor ({02}.s4)

//	 compute final s8, s9, s10, s11
	State[8]^=State2[8]^State2[9];  //s8' = s8' ^ ({02}.s8) xor ({02}.s9)
	State[9]^=State2[9]^State2[10];  //s9' = s9' ^ ({02}.s9) xor ({02}.s10)
	State[10]^=State2[10]^State2[11];  //s10' = s10' ^ ({02}.s10) xor ({02}.s11)
	State[11]^=State2[11]^State2[8];  //s11' = s11' ^ ({02}.s11) xor ({02}.s8)

//	 compute final s12, s13, s14, s15
	State[12]^=State2[12]^State2[13];  //s8' = s8' ^ ({02}.s8) xor ({02}.s9)
	State[13]^=State2[13]^State2[14];  //s9' = s9' ^ ({02}.s9) xor ({02}.s10)
	State[14]^=State2[14]^State2[15];  //s10' = s10' ^ ({02}.s10) xor ({02}.s11)
	State[15]^=State2[15]^State2[12];  //s11' = s11' ^ ({02}.s11) xor ({02}.s8)
}


//-------------------------------------------------------------------------------
// void Shiftrows_Subbyte(void)
// Compute State2= Shiftrow (Subbyte(State))
//-------------------------------------------------------------------------------
void Shiftrows_Subbyte(PX8 State, PX8 State2)
{
	// row 0
	State2[0]=SBOX[State[0]];
	State2[4]=SBOX[State[4]];
	State2[8]=SBOX[State[8]];
	State2[12]=SBOX[State[12]];

	// row 2
	State2[1]=SBOX[State[5]];
	State2[5]=SBOX[State[9]];
	State2[9]=SBOX[State[13]];
	State2[13]=SBOX[State[1]];

	// row 1
	State2[10]=SBOX[State[2]];
	State2[14]=SBOX[State[6]];
	State2[2]=SBOX[State[10]];
	State2[6]=SBOX[State[14]];

	// row 3
	State2[3]=SBOX[State[15]];
	State2[7]=SBOX[State[3]];
	State2[11]=SBOX[State[7]];
	State2[15]=SBOX[State[11]];
}






/*******************************************************************************
 * Function name: AESKeyExpand
 * Description: AES key expanded using g_AES_Key and g_AES_KeyLen.
 * Input Parameters:   (precondition)
 *      none.
 * Output Parameters:  (postcondition)
 *		none(the expanded key is stored in glable buffer g_AESKeySchedule)
 * NOTE:
 ********************************************************************************/
void SoftAes_AESKeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	PX8 rk;
	u8 t, i, j;
	u8	State[0x10];
	u8	State2[0x10];
	u8 aESRoundNumber;
	u8 AESKeyByteLen;
	AESKeyByteLen = keyBitLen >> 3;
	rk = MEMtPX8(pExternKey);

//#if U8_2_U32_INVERSE
//	// processing block buffer of u32 should be big endian order
//	BigEndian2LittleEndian((PX8)rk, AESKeyByteLen);
//#endif
	t = 0x00;
	i = AESKeyByteLen;
	aESRoundNumber = 4*(AESKeyByteLen +28);
	while (i < aESRoundNumber)
	{
		j=i-4;
		State[0]=State2[0]=rk[j++];
		State[1]=rk[j++];
		State[2]=rk[j++];
		State[3]=rk[j];

		if ((i % AESKeyByteLen)==0)
		{
			State[0]=SBOX[State[1]] ^ Rcon[t];
			State[1]=SBOX[State[2]];
			State[2]=SBOX[State[3]];
			State[3]=SBOX[State2[0]];
			t++;
		}
		else if ((AESKeyByteLen == 32) && (i % 32 == 16))
		{
			State[0]=SBOX[State[0]];
			State[1]=SBOX[State[1]];
			State[2]=SBOX[State[2]];
			State[3]=SBOX[State[3]];
		}
		j=i-AESKeyByteLen;

		rk[i++]=State[0]^rk[j++];
		rk[i++]=State[1]^rk[j++];
		rk[i++]=State[2]^rk[j++];
		rk[i++]=State[3]^rk[j];
	}

//#if U8_2_U32_INVERSE
//	// processing block buffer of u32 should be big endian order
//	BigEndian2LittleEndian((PX8)rk, AESKeyByteLen);
//#endif

}

/*******************************************************************************
 * Function name: AESEncryptCore
 * Description: AES encryption.
 * Input Parameters:   (precondition)
 *      none(both input buffer and output buffer are g_AES_TempInBuffer)
 * Output Parameters:  (postcondition)
 *		none
 * NOTE:
 ********************************************************************************/
static void AESEncryptCore(u8 * pDataIn , PX32 pExternKey,u16 keyBitLen)
{
	PX8 rk;
	PX8 Plain;
	u8	State[0x10];
	u8	State2[0x10];
	u8 aESRoundNumber, i, round;

//#if U8_2_U32_INVERSE
//	// processing block buffer of u32 should be big endian order
//	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
//#endif
	rk = MEMtPX8(pExternKey);
	Plain = pDataIn;
	aESRoundNumber = U8_M_U8(4, ((keyBitLen >> 3) +24));

	//AddRoundKey
	for (i=15; i>0;i--)
	{
		State[i]=Plain[i]^rk[i];
	}
	State[0]=Plain[0]^rk[0];

	round=16;
	do
	{
		Shiftrows_Subbyte(State, State2);
		MixColumns(State, State2);
		//AddRoundKey
		for (i=0; i<16;i++) State[i]^= rk[round++];
	} while (round<aESRoundNumber);

	Shiftrows_Subbyte(State, State2);
	//AddRoundKey
	for (i=0; i<16;i++)
	{
		Plain[i]=State2[i]^ rk[round++];
	}

//#if U8_2_U32_INVERSE
//	// processing block buffer of u32 should be big endian order
//	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
//#endif
}

#if	SOFT_AES_DECRYPTO
/*Matrix INVSBOX for the procedure of decryption*/
const unsigned char INVSBOX[256] ={0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
			    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
			    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
			    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
			    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
			    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
			    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
			    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
			    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
			    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
			    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
			    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
			    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
			    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
			    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
			    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};
//-------------------------------------------------------------------------------
// compute State=InvMixcolumn_end(State) with State=MixColumns(State2)
// note:	s0'' = State[0]
//		s0' = State2[0]
//		s0 = Initial State2[0]

//Input :

// s0'= {02}.s0,
// s1'= {02}.s1,...  s15'={02}.s15

// Formula gives for the 1st column of State:
// s0'' = ({02}.s0) xor ({03}.s1) xor s2 xor s3
// s1'' = s0 xor ({02}.s1) xor ({03}.s2) xor s3
// s2'' = s0 xor s1 xor ({02}.s2) xor ({03}.s3)
// s3'' = ({03}.s0) xor s1 xor s2 xor ({02}.s3)

//Output
// Formula gives for the 1st column of State:
// s0'' = ({0E}.s0) xor ({0B}.s1) xor ({0D}.s2) xor ({09}.s3)
// s1'' = ({09}.s0) xor ({0E}.s1) xor ({0B}.s2) xor ({0D}.s3)
// s2'' = ({0D}.s0) xor ({09}.s1) xor ({0E}.s2) xor ({0B}.s3)
// s3'' = ({0B}.s0) xor ({0D}.s1 xor ({09}.s2) xor ({0E}.s3)

// -------------------------------------------------------------------------------
void InvMixcolumn_end (PX8 State, PX8 State2)
{
	// column 0
	State2[0]^=State2[2];
	xtime(State2, 0);	// s0'= {2}.(s0' xor s2') = ({04}.s0) xor ({04}.s2)
	State2[1]^=State2[3];
	xtime(State2, 1);	// s1' = {2}.(s1' xor s3')= ({04}.s1) xor ({04}.s3)

	State[1]^=State2[1];			// s1'' = s1' xor s1'' =  s0 xor ({06}.s1) xor ({03}.s2) xor ({05}.s3)
	State[0]^=State2[0];			// s0'' = s0' xor s0'' = ({06}.s0) xor ({03}.s1) xor ({05}.s2) xor s3
	State[2]^=State2[0];			// s2'' = s0' xor s2'' = {05}.s0) xor s1 xor ({06}.s2) xor ({03}.s3)
	State[3]^=State2[1];			// s3'' = s1' xor s3'' = ({03}.s0) xor ({05}.s1) xor s2 xor {06}.s3)

	State2[0]^=State2[1];
	xtime(State2, 0);	// s0'= {2}.(s0' xor s1') = ({08}.s0) xor ({08}.s1) xor ({08}.s2)xor ({08}.s3)

	State[0]^=State2[0];			// s0'' = s0' xor s0'' = ({0E}.s0) xor ({0B}.s1) xor ({0D}.s2) xor ({09}.s3)
	State[1]^=State2[0];			// s1'' = s0' xor s1'' = ({09}.s0) xor ({0E}.s1) xor ({0B}.s2)xor ({0D}.s3)
	State[2]^=State2[0];			// s2'' = s0' xor s2'' = {0D}.s0) xor ({09}.s1) xor ({0E}.s2) xor ({0B}.s3)
	State[3]^=State2[0];			// s3'' = s0' xor s3''= ({0B}.s0) xor ({0D}.s1 xor ({09}.s2) xor ({0E}.s3)

	// compute column 1
	State2[4]^=State2[6];
	xtime(State2, 4);	// s4'= {2}.(s4' xor s6')
	State2[5]^=State2[7];
	xtime(State2, 5);	// s5' = {2}.(s5' xor s7')
	State[5]^=State2[5];			// s5'' = s5' xor s5''
	State[4]^=State2[4];			// s4'' = s4' xor s4''
	State[6]^=State2[4];			// s6'' = s4' xor s6''
	State[7]^=State2[5];			// s7'' = s5' xor s7''

	State2[4]^=State2[5];
	xtime(State2, 4);	// s4'= {2}.(s4' xor s5')
	State[4]^=State2[4];			// s4'' = s4' xor s4''
	State[5]^=State2[4];			// s5'' = s4' xor s5''
	State[6]^=State2[4];			// s2'' = s4' xor s6''
	State[7]^=State2[4];			// s3'' = s4' xor s7''

	// compute column 2
	State2[8]^=State2[10];
	xtime(State2, 8);	// s8'= {2}.(s8' xor s10')
	State2[9]^=State2[11];
	xtime(State2, 9);	// s9' = {2}.(s9' xor s11')
	State[9]^=State2[9];			// s9'' = s9' xor s9''
	State[8]^=State2[8];			// s8'' = s8' xor s8''
	State[10]^=State2[8];			// s10'' = s8' xor s10''
	State[11]^=State2[9];			// s11'' = s9' xor s11''

	//State2[8]=xtime(State2[8]^State2[9]);	// s8' = {2}.(s8' xor s9')
	State2[8]^=State2[9];
	xtime(State2, 8);	// s8' = {2}.(s8' xor s9')
	State[8]^=State2[8];			// s8'' = s8' xor s8''
	State[9]^=State2[8];			// s9'' = s8' xor s9''
	State[10]^=State2[8];			// s10'' = s8' xor s10''
	State[11]^=State2[8];			// s11'' = s8' xor s11''

	// compute column 3
	State2[12]^=State2[14];
	xtime(State2, 12);// s12' = {2}.(s12' xor s14')
	State2[13]^=State2[15];
	xtime(State2, 13);// s13' = {2}.(s13' xor s15')
	State[13]^=State2[13];			// s13'' = s13' xor s13''
	State[12]^=State2[12];			// s12'' = s12' xor s12''
	State[14]^=State2[12];			// s14'' = s12' xor s12''
	State[15]^=State2[13];			// s15'' = s13' xor s15''

	State2[12]^=State2[13];
	xtime(State2, 12);// s12' = {2}.(s12' xor s13')
	State[12]^=State2[12];			// s12'' = s12' xor s12''
	State[13]^=State2[12];			// s13'' = s12' xor s13''
	State[14]^=State2[12];			// s14'' = s12' xor s14''
	State[15]^=State2[12];			// s15'' = s12' xor s15''
}

//-------------------------------------------------------------------------------
// void Invshiftrows_Invsubbyte(void)
// Compute State2= Invshiftrows (Invsubbyte(State))
//-------------------------------------------------------------------------------
void Invshiftrows_Invsubbyte(PX8 State, PX8 State2)
{
	// row 0
	State2[0]=INVSBOX[State[0]];
	State2[4]=INVSBOX[State[4]];
	State2[8]=INVSBOX[State[8]];
	State2[12]=INVSBOX[State[12]];

	// row 2
	State2[10]=INVSBOX[State[2]];
	State2[14]=INVSBOX[State[6]];
	State2[2]=INVSBOX[State[10]];
	State2[6]=INVSBOX[State[14]];

	// row 1
	State2[1]=INVSBOX[State[13]];
	State2[5]=INVSBOX[State[1]];
	State2[9]=INVSBOX[State[5]];
	State2[13]=INVSBOX[State[9]];

	// row 3
	State2[3]=INVSBOX[State[7]];
	State2[7]=INVSBOX[State[11]];
	State2[11]=INVSBOX[State[15]];
	State2[15]=INVSBOX[State[3]];
}

/*******************************************************************************
 * Function name: AESDecryptCore
 * Description: AES decryption.
 * Input Parameters:   (precondition)
 *      none(both input buffer and output buffer are g_AES_TempInBuffer)
 * Output Parameters:  (postcondition)
 *		none
 * NOTE:
 ********************************************************************************/
static void AESDecryptCore(u8 * pDataIn , PX32 pExternKey, u16 keyBitLen)
{
	u8 i, round;
	PX8 rk;
	PX8 src;
	u8	State[0x10];
	u8	State2[0x10];

//#if U8_2_U32_INVERSE
//	// processing block buffer of u32 should be big endian order
//	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
//#endif

	rk = MEMtPX8(pExternKey);
	src = pDataIn;

	round = U8_M_U8(4, ((keyBitLen >> 3) + 28))-1;

	//AddRoundKey
	for (i=15; i>0;i--)
	{
		State[i]=src[i]^rk[round--];
	}
	State[0]=src[0]^rk[round--];

	do	{

	 // compute State2 = InvShiftrow(InvSubbyte(State))
		Invshiftrows_Invsubbyte(State, State2);
		//AddRoundKey
		for (i=15; i>0;i--) State2[i]^=rk[round--];
		State2[0]^=rk[round--];

	 //InvMixcolumn
		// Compute State = InvMixcolumn(State2)
		MixColumns(State, State2);
		InvMixcolumn_end(State, State2);

	} while (round>15);

	// compute State2 = InvShiftrow(InvSubbyte(State))
	Invshiftrows_Invsubbyte(State, State2);

	//AddRoundKey
	for (i=15; i>0;i--)
	{
		src[i]=State2[i]^rk[i];
	}
	src[0]=State2[0]^rk[0];

//#if U8_2_U32_INVERSE
//	// processing block buffer of u32 should be big endian order
//	BigEndian2LittleEndian(pDataIn, AES_BLOCKSIZE);
//#endif
}
#endif
#endif //AES_BIT_CACULATE
void SoftAes_DoAES_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
		u16 tData[AES_BLOCKSIZE];
		ecpMem_Copy(tData , dataIn , AES_BLOCKSIZE);
	    if(MODE_ENCRYPT == en_deCrypt)
    	{
    		AESEncryptCore(tData, pExternKey, (u16)(KeyByteLen << 3));
    	}
#if	SOFT_AES_DECRYPTO
		else
		{
			AESDecryptCore(tData, pExternKey, (u16)(KeyByteLen << 3));
		}
#endif
    	ecpMem_Copy(dataOut, tData , AES_BLOCKSIZE);
}

#endif  // WITH_SOFT_AES
