/*
 * Crypto_Soft_SM4.c
 *
 *  Created on: 2014Äê4ÔÂ10ÈÕ
 *      Author: jiangxiaohui
 */

#include "Crypto_SoftSM4.h"
#include "Sym_API.h"
#include "SysConfigHalRam.h"
#include "SysConfig.h"

#if WITH_SM4
#if WITH_SOFT_SM4

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (u32) (b)[(i)    ] << 24 )        \
        | ( (u32) (b)[(i) + 1] << 16 )        \
        | ( (u32) (b)[(i) + 2] <<  8 )        \
        | ( (u32) (b)[(i) + 3]       );       \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (u8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (u8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (u8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (u8) ( (n)       );       \
}
#endif

/*
 *rotate shift left marco definition
 *
 */
#define SHL(x,n) (((x) & 0xFFFFFFFF) << n)
#define ROTL(x,n) (SHL((x),n) | ((x) >> (32 - n)))

#define SWAP(a,b) { \
	u32 t ; \
    t= a; \
    a = b;\
    b = t; \
    }


/*
 * Expanded SM4 S-boxes
 * Sbox table: 8bits input convert to 8 bits output*/

static const u8 SboxTable[16][16] =
{
	{0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05},
	{0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99},
	{0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62},
	{0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6},
	{0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8},
	{0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35},
	{0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87},
	{0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e},
	{0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1},
	{0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3},
	{0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f},
	{0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51},
	{0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8},
	{0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0},
	{0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84},
	{0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48}
};

/* System parameter */
static const u32 FK[4] = {0xa3b1bac6,0x56aa3350,0x677d9197,0xb27022dc};

/* fixed parameter */
static const u32 CK[32] =
{
	0x00070e15,0x1c232a31,0x383f464d,0x545b6269,
	0x70777e85,0x8c939aa1,0xa8afb6bd,0xc4cbd2d9,
	0xe0e7eef5,0xfc030a11,0x181f262d,0x343b4249,
	0x50575e65,0x6c737a81,0x888f969d,0xa4abb2b9,
	0xc0c7ced5,0xdce3eaf1,0xf8ff060d,0x141b2229,
	0x30373e45,0x4c535a61,0x686f767d,0x848b9299,
	0xa0a7aeb5,0xbcc3cad1,0xd8dfe6ed,0xf4fb0209,
	0x10171e25,0x2c333a41,0x484f565d,0x646b7279
};

/*
 * private function:
 * look up in SboxTable and get the related value.
 * args:    [in] inch: 0x00~0xFF (8 bits unsigned value).
 */
static u8 sm4Sbox(u8 inch)
{
    PM8 D_A pTable = (PM8)SboxTable;
    return (u8)(pTable[inch]);
}

/*
 * private F(Lt) function:
 * "T algorithm" == "L algorithm" + "t algorithm".
 * args:    [in] a: a is a 32 bits unsigned value;
 * return: c: c is calculated with line algorithm "L" and nonline algorithm "t"
 */
static u32 sm4Lt(u32 ka)
{
    u32		bb;
    u32		c;
    u8		a[4];
	u8		b[4];
	
    PUT_ULONG_BE(ka,a,0)
    b[0] = sm4Sbox(a[0]);
    b[1] = sm4Sbox(a[1]);
    b[2] = sm4Sbox(a[2]);
    b[3] = sm4Sbox(a[3]);
	GET_ULONG_BE(bb,b,0)
    c = bb^(ROTL(bb, 2))^(ROTL(bb, 10))^(ROTL(bb, 18))^(ROTL(bb, 24));
    return c;
}

/* private function:
 * Calculating round encryption key.
 * args:    [in] a: a is a 32 bits unsigned value;
 * return: sk[i]: i{0,1,2,3,...31}.
 */
static u32 sm4CalciRK(u32 ka)
{
    u32		I_A	bb ;
    u32		I_A	rk ;
    u8		I_A	a[4];
    u8		I_A	b[4];
    PUT_ULONG_BE(ka,a,0)
    b[0] = sm4Sbox(a[0]);
    b[1] = sm4Sbox(a[1]);
    b[2] = sm4Sbox(a[2]);
    b[3] = sm4Sbox(a[3]);
	GET_ULONG_BE(bb,b,0)
    rk = bb^(ROTL(bb, 13))^(ROTL(bb, 23));
    return rk;
}

/*******************************************************************************
 * Function name: SM4KeyExpand
 * Description: SM4 key expanded using g_SM4_Key and g_SM4_KeyLen.
 * Input Parameters:   (precondition)
 *      none.
 * Output Parameters:  (postcondition)
 *		none(the expanded key is stored in glable buffer g_SM4KeySchedule)
 * NOTE:
 ********************************************************************************/
void SoftSM4_SM4KeyExpand(u16 keyBitLen, PX32 pExternKey, u8 cryptoMode)
{
	PX32	rk;
	u8	 	I_A	i;
	u8 		SM4KeyByteLen;

	rk 		= pExternKey;
	
	SM4KeyByteLen = keyBitLen >> 3;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian((PX8)rk, SM4KeyByteLen);
#endif

	/*
	 * SM4 key schedule (128-bit, encryption)
	 */
    rk[0] = rk[0]^FK[0];
    rk[1] = rk[1]^FK[1];
    rk[2] = rk[2]^FK[2];
    rk[3] = rk[3]^FK[3];

    rk[0] = rk[0] ^ (sm4CalciRK(CK[0]^rk[1]^rk[2]^rk[3]));
    rk[1] = rk[1] ^ (sm4CalciRK(CK[1]^rk[2]^rk[3]^rk[0]));
    rk[2] = rk[2] ^ (sm4CalciRK(CK[2]^rk[3]^rk[0]^rk[1]));
    rk[3] = rk[3] ^ (sm4CalciRK(CK[3]^rk[0]^rk[1]^rk[2]));

    for(i = 0; i<28; i++)
	{
        rk[i+4] = rk[i] ^ (sm4CalciRK(CK[i+4]^rk[i+1]^rk[i+2]^rk[i+3]));
	}

	/*
	 * SM4 key schedule (128-bit, decryption)
	 */

	if( U8_AND_U8(cryptoMode,MODE_DECRYPT) == MODE_DECRYPT)
	{
	    for( i = 0; i < 16; i ++ )
	    {
	        SWAP( rk[ i ], rk[ 31-i] );
	    }
	}

}

void SoftSM4_DoSM4_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 KeyByteLen)
{
	PX32	D_A		rk;
    u32		X_A		MK[4];
	u8	 	D_A		i;

	ecpMem_Copy((PX8)MK, dataIn, KeyByteLen);
	
#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian((PX8)MK, KeyByteLen);
#endif

    i		= 0x00;
	rk 		= pExternKey;
    while(i<32)
    {
    	MK[0] = MK[0] ^ sm4Lt(MK[1] ^ MK[2] ^ MK[3] ^ rk[i]);
    	i++;
    	MK[1] = MK[1] ^ sm4Lt(MK[2] ^ MK[3] ^ MK[0] ^ rk[i]);
    	i++;
    	MK[2] = MK[2] ^ sm4Lt(MK[3] ^ MK[0] ^ MK[1] ^ rk[i]);
    	i++;
    	MK[3] = MK[3] ^ sm4Lt(MK[0] ^ MK[1] ^ MK[2] ^ rk[i]);
	    i ++;
    }
	
	SWAP(MK[0],MK[3]);
	SWAP(MK[1],MK[2]);
	
	
#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian((PX8)MK, KeyByteLen);
#endif

	ecpMem_Copy(dataOut, (PX8)MK, KeyByteLen);
}

#endif //WITH_SOFT_SM4
#endif //WITH_SM4

