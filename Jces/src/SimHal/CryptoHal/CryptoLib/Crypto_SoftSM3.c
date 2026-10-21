/*
 * Crypto_softSM3.c
 *
 *  Created on: 2014Äê4ÔÂ11ÈÕ
 *      Author: jiangxiaohui
 */


#include "Crypto_softSM3.h"

#include "Crypto_Constant.h"

#if WITH_SM3

#if WITH_SOFT_SM3


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

#define FF0(x,y,z) ( (x) ^ (y) ^ (z))
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define GG0(x,y,z) ( (x) ^ (y) ^ (z))
#define GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )

#define  SHL(x,n) (((x) & 0xFFFFFFFF) << n)
#define ROTL(x,n) (SHL((x),n) | ((x) >> (32 - n)))

#define P0(x) ((x) ^  ROTL((x),9) ^ ROTL((x),17))
#define P1(x) ((x) ^  ROTL((x),15) ^ ROTL((x),23))

void SoftSM3_SM3Transform(PX32 blockBuffer, PX32 icvBuffer)
{
    u32		X_A		W[68];
    u32 	X_A		W1;
	u32		X_A		SS1;
	u32		X_A		SS2;
	u32		X_A		TT1;
	u32		X_A		TT2;
    u32		X_A		A;
    u32		X_A		B;
    u32		X_A		C;
    u32		X_A		D;
    u32		X_A		E;
    u32		X_A		F;
    u32		X_A		G;
    u32		X_A		H;
	u32		X_A		Temp1;
	u32		X_A		Temp2;
	u32		X_A		Temp3;
	u32		X_A		Temp4;
	u32		X_A		Temp5;
	u8		X_A		j;

#if U8_2_U32_INVERSE
    BigEndian2LittleEndian((PX8)blockBuffer, SM3_BLOCK_SIZE);
#else
// nothing to do!
#endif

	ecpMem_Copy(W, blockBuffer, SM3_BLOCK_SIZE);

	for(j = 16; j < 68; j++ )
	{
		Temp1 = W[j-16] ^ W[j-9];
		Temp2 = ROTL(W[j-3],15);
		Temp3 = Temp1 ^ Temp2;
		Temp4 = P1(Temp3);
		Temp5 =  ROTL(W[j - 13],7 ) ^ W[j-6];
		W[j] = Temp4 ^ Temp5;
	}

    A = icvBuffer[0];
    B = icvBuffer[1];
    C = icvBuffer[2];
    D = icvBuffer[3];
    E = icvBuffer[4];
    F = icvBuffer[5];
    G = icvBuffer[6];
    H = icvBuffer[7];

	for(j =0; j < 16; j++)
	{
		SS1 = ROTL((ROTL(A,12) + E + ROTL(0x79CC4519,j)), 7);
		SS2 = SS1 ^ ROTL(A,12);
		W1 	= W[j] ^ W[j+4];
		TT1 = FF0(A,B,C) + D + SS2 + W1;
		TT2 = GG0(E,F,G) + H + SS1 + W[j];
		D = C;
		C = ROTL(B,9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F,19);
		F = E;
		E = P0(TT2);
	}

	for(j =16; j < 64; j++)
	{
		SS1 = ROTL((ROTL(A,12) + E + ROTL(0x7A879D8A,j)), 7);
		SS2 = SS1 ^ ROTL(A,12);
		W1 	= W[j] ^ W[j+4];
		TT1 = FF1(A,B,C) + D + SS2 + W1;
		TT2 = GG1(E,F,G) + H + SS1 + W[j];
		D = C;
		C = ROTL(B,9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F,19);
		F = E;
		E = P0(TT2);
	}

	icvBuffer[0] ^= A;
	icvBuffer[1] ^= B;
	icvBuffer[2] ^= C;
	icvBuffer[3] ^= D;
	icvBuffer[4] ^= E;
	icvBuffer[5] ^= F;
	icvBuffer[6] ^= G;
	icvBuffer[7] ^= H;
}
#endif //WITH_SOFT_SM3

#endif // WITH_SM3

