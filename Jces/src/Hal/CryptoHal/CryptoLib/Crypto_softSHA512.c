/*
 * Crypto_softSHA512.c
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */


#include "Crypto_softSHA512.h"

#if (WITH_SOFT_SHA384 || WITH_SOFT_SHA512)

#ifdef WITH_SOFT_SHA384
#define	SHA384_OR_512_BLOCK_SIZE	SHA384_BLOCK_SIZE
#else	// WITH_SOFT_SHA512
#define	SHA384_OR_512_BLOCK_SIZE	SHA512_BLOCK_SIZE
#endif //WITH_SOFT_SHA384

#define SUPPORT_INT64  0

#define SHR(x,n) ((x) >> (n))
#define ROTR(x,n) (SHR(x,n) | ((x) << (32 - (n))))

#if SUPPORT_INT64
#define U8_2_U64_INVERSE  1
typedef unsigned long long u64;
#define PX64 	u64	X_A *
/*******************************************************************************
 * Define the basic SHA512 or SHA512 macro functions.
 ******************************************************************************/
#define ROTR64(x, s) (((x) >> (s)) | ((x) << (64 - (s))))

#define SHA512SRight(x,s) ((x) >> (s))
#define SHA512RR(x,s) ROTR64(x, s)
#define CH(x,y,z) ((x&(y^z))^z)
#define MAJ(x,y,z) (((x|y)&z)|(x&y))

#define SIG0(x) (SHA512RR(x,28) ^ SHA512RR(x,34) ^ SHA512RR(x,39))
#define SIG1(x) (SHA512RR(x,14) ^ SHA512RR(x,18) ^ SHA512RR(x,41))
#define sig0(x) (SHA512RR(x,1) ^ SHA512RR(x,8) ^ SHA512SRight(x,7))
#define sig1(x) (SHA512RR(x,19) ^ SHA512RR(x,61) ^ SHA512SRight(x,6))

#define ROUND(a,b,c,d,e,f,g,h,w,k)	\
	tempForSHA512 = h + SIG1(e) + CH(e,f,g) + k + w;	\
	h = tempForSHA512 + SIG0(a) + MAJ(a,b,c);	\
	d += tempForSHA512

#define Rw(t) ( \
    tempIcvbuffer[t&15] = sig1(tempIcvbuffer[(t-2)&15]) + tempIcvbuffer[(t-7)&15] + sig0(tempIcvbuffer[(t-15)&15]) + tempIcvbuffer[(t-16)&15] \
	)

/*******************************************************************************
 * Function name: swapu64
 * Description: This function converts big endian u64 to little endian u64, or
 *              converts little endian u64 to big endian u64.
 *      -----------------------------------------------------------------
 *      | byte1 | byte2 | byte3 | byte4 | byte5 | byte6 | byte7 | byte8 |
 *      -----------------------------------------------------------------
 *                                      |
 *                                      |
 *                                      V
 *      -----------------------------------------------------------------
 *      | byte8 | byte7 | byte6 | byte5 | byte4 | byte3 | byte2 | byte1 |
 *      -----------------------------------------------------------------
 * Input Parameters:
 *	   	u64 n	    	    a number of u64
 * Output Parameters:
 * 		the number that has been inversed
 * NOTE:
 ******************************************************************************/
static u64 swapu64(u64 n)
{
	return ( ((n & ((u64) 0xff)      ) << 56) |
			 ((n & ((u64) 0xff) <<  8) << 40) |
			 ((n & ((u64) 0xff) << 16) << 24) |
			 ((n & ((u64) 0xff) << 24) <<  8) |
			 ((n & ((u64) 0xff) << 32) >>  8) |
			 ((n & ((u64) 0xff) << 40) >> 24) |
			 ((n & ((u64) 0xff) << 48) >> 40) |
			 ((n & ((u64) 0xff) << 56) >> 56) );
}

/*******************************************************************************
 * Function name: SHA512Transform
 * Description: SHA512 or SHA512 basic transformation. Transforms state on block.
 * Input Parameters:
 *	   	u32 state[16]	    	    state of SHA512 (SHA512) (through global variable g_MDCtxt).
 *      u8 * blockBuf               block buffer (through global variable g_MDCtxt).
 * Output Parameters:
 * 		None
 * NOTE:
 ******************************************************************************/
void SoftSHA512_SHA512Transform(PX64 blockBuffer, PX64 icvBuffer)
{
	// temp variables
	u64 	X_A		tempForSHA512;
	PX64 			tempIcvbuffer;

	// fetch the state from global variable
	u64 	X_A		ICVPatrOne;
	u64 	X_A		ICVPatrTwo;
	u64 	X_A		ICVPatrThree;
	u64 	X_A		ICVPatrFour;
	u64 	X_A		ICVPatrFive;
	u64 	X_A		ICVPatrSix;
	u64 	X_A		ICVPatrSeven;
	u64 	X_A		ICVPatrEight;

	tempIcvbuffer 	= blockBuffer;
#if U8_2_U64_INVERSE
	u8 t = 16;
	while(t--)
	{
		tempForSHA512 = swapu64(*tempIcvbuffer);
		*(tempIcvbuffer++) = tempForSHA512;
	}
#endif


	ICVPatrOne 		= icvBuffer[0];
	ICVPatrTwo 		= icvBuffer[1];
	ICVPatrThree 	= icvBuffer[2];
	ICVPatrFour 	= icvBuffer[3];
	ICVPatrFive 	= icvBuffer[4];
	ICVPatrSix 		= icvBuffer[5];
	ICVPatrSeven 	= icvBuffer[6];
	ICVPatrEight 	= icvBuffer[7];

	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,tempIcvbuffer[ 0],0x428a2f98d728ae22ULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,tempIcvbuffer[ 1],0x7137449123ef65cdULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,tempIcvbuffer[ 2],0xb5c0fbcfec4d3b2fULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,tempIcvbuffer[ 3],0xe9b5dba58189dbbcULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,tempIcvbuffer[ 4],0x3956c25bf348b538ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,tempIcvbuffer[ 5],0x59f111f1b605d019ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,tempIcvbuffer[ 6],0x923f82a4af194f9bULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,tempIcvbuffer[ 7],0xab1c5ed5da6d8118ULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,tempIcvbuffer[ 8],0xd807aa98a3030242ULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,tempIcvbuffer[ 9],0x12835b0145706fbeULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,tempIcvbuffer[10],0x243185be4ee4b28cULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,tempIcvbuffer[11],0x550c7dc3d5ffb4e2ULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,tempIcvbuffer[12],0x72be5d74f27b896fULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,tempIcvbuffer[13],0x80deb1fe3b1696b1ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,tempIcvbuffer[14],0x9bdc06a725c71235ULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,tempIcvbuffer[15],0xc19bf174cf692694ULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(16),0xe49b69c19ef14ad2ULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(17),0xefbe4786384f25e3ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(18),0x0fc19dc68b8cd5b5ULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(19),0x240ca1cc77ac9c65ULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(20),0x2de92c6f592b0275ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(21),0x4a7484aa6ea6e483ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(22),0x5cb0a9dcbd41fbd4ULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(23),0x76f988da831153b5ULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(24),0x983e5152ee66dfabULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(25),0xa831c66d2db43210ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(26),0xb00327c898fb213fULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(27),0xbf597fc7beef0ee4ULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(28),0xc6e00bf33da88fc2ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(29),0xd5a79147930aa725ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(30),0x06ca6351e003826fULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(31),0x142929670a0e6e70ULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(32),0x27b70a8546d22ffcULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(33),0x2e1b21385c26c926ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(34),0x4d2c6dfc5ac42aedULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(35),0x53380d139d95b3dfULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(36),0x650a73548baf63deULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(37),0x766a0abb3c77b2a8ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(38),0x81c2c92e47edaee6ULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(39),0x92722c851482353bULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(40),0xa2bfe8a14cf10364ULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(41),0xa81a664bbc423001ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(42),0xc24b8b70d0f89791ULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(43),0xc76c51a30654be30ULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(44),0xd192e819d6ef5218ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(45),0xd69906245565a910ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(46),0xf40e35855771202aULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(47),0x106aa07032bbd1b8ULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(48),0x19a4c116b8d2d0c8ULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(49),0x1e376c085141ab53ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(50),0x2748774cdf8eeb99ULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(51),0x34b0bcb5e19b48a8ULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(52),0x391c0cb3c5c95a63ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(53),0x4ed8aa4ae3418acbULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(54),0x5b9cca4f7763e373ULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(55),0x682e6ff3d6b2b8a3ULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(56),0x748f82ee5defb2fcULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(57),0x78a5636f43172f60ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(58),0x84c87814a1f0ab72ULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(59),0x8cc702081a6439ecULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(60),0x90befffa23631e28ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(61),0xa4506cebde82bde9ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(62),0xbef9a3f7b2c67915ULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(63),0xc67178f2e372532bULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(64),0xca273eceea26619cULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(65),0xd186b8c721c0c207ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(66),0xeada7dd6cde0eb1eULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(67),0xf57d4f7fee6ed178ULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(68),0x06f067aa72176fbaULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(69),0x0a637dc5a2c898a6ULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(70),0x113f9804bef90daeULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(71),0x1b710b35131c471bULL);
	ROUND(ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,Rw(72),0x28db77f523047d84ULL);
	ROUND(ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,Rw(73),0x32caab7b40c72493ULL);
	ROUND(ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,Rw(74),0x3c9ebe0a15c9bebcULL);
	ROUND(ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,Rw(75),0x431d67c49c100d4cULL);
	ROUND(ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,ICVPatrFour,Rw(76),0x4cc5d4becb3e42b6ULL);
	ROUND(ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,ICVPatrThree,Rw(77),0x597f299cfc657e2aULL);
	ROUND(ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,ICVPatrTwo,Rw(78),0x5fcb6fab3ad6faecULL);
	ROUND(ICVPatrTwo,ICVPatrThree,ICVPatrFour,ICVPatrFive,ICVPatrSix,ICVPatrSeven,ICVPatrEight,ICVPatrOne,Rw(79),0x6c44198c4a475817ULL);

	icvBuffer[0] += ICVPatrOne;
	icvBuffer[1] += ICVPatrTwo;
	icvBuffer[2] += ICVPatrThree;
	icvBuffer[3] += ICVPatrFour;
	icvBuffer[4] += ICVPatrFive;
	icvBuffer[5] += ICVPatrSix;
	icvBuffer[6] += ICVPatrSeven;
	icvBuffer[7] += ICVPatrEight;
}

#else // use u32 (unsigned long)
static u32 CONST SHA512_CONSTANTS[160] =
{
	0x428A2F98, 0xD728AE22, 0x71374491, 0x23EF65CD,
	0xB5C0FBCF, 0xEC4D3B2F, 0xE9B5DBA5, 0x8189DBBC,
	0x3956C25B, 0xF348B538, 0x59F111F1, 0xB605D019,
	0x923F82A4, 0xAF194F9B, 0xAB1C5ED5, 0xDA6D8118,
	0xD807AA98, 0xA3030242, 0x12835B01, 0x45706FBE,
	0x243185BE, 0x4EE4B28C, 0x550C7DC3, 0xD5FFB4E2,
	0x72BE5D74, 0xF27B896F, 0x80DEB1FE, 0x3B1696B1,
	0x9BDC06A7, 0x25C71235, 0xC19BF174, 0xCF692694,
	0xE49B69C1, 0x9EF14AD2, 0xEFBE4786, 0x384F25E3,
	0x0FC19DC6, 0x8B8CD5B5, 0x240CA1CC, 0x77AC9C65,
	0x2DE92C6F, 0x592B0275, 0x4A7484AA, 0x6EA6E483,
	0x5CB0A9DC, 0xBD41FBD4, 0x76F988DA, 0x831153B5,
	0x983E5152, 0xEE66DFAB, 0xA831C66D, 0x2DB43210,
	0xB00327C8, 0x98FB213F, 0xBF597FC7, 0xBEEF0EE4,
	0xC6E00BF3, 0x3DA88FC2, 0xD5A79147, 0x930AA725,
	0x06CA6351, 0xE003826F, 0x14292967, 0x0A0E6E70,
	0x27B70A85, 0x46D22FFC, 0x2E1B2138, 0x5C26C926,
	0x4D2C6DFC, 0x5AC42AED, 0x53380D13, 0x9D95B3DF,
	0x650A7354, 0x8BAF63DE, 0x766A0ABB, 0x3C77B2A8,
	0x81C2C92E, 0x47EDAEE6, 0x92722C85, 0x1482353B,
	0xA2BFE8A1, 0x4CF10364, 0xA81A664B, 0xBC423001,
	0xC24B8B70, 0xD0F89791, 0xC76C51A3, 0x0654BE30,
	0xD192E819, 0xD6EF5218, 0xD6990624, 0x5565A910,
	0xF40E3585, 0x5771202A, 0x106AA070, 0x32BBD1B8,
	0x19A4C116, 0xB8D2D0C8, 0x1E376C08, 0x5141AB53,
	0x2748774C, 0xDF8EEB99, 0x34B0BCB5, 0xE19B48A8,
	0x391C0CB3, 0xC5C95A63, 0x4ED8AA4A, 0xE3418ACB,
	0x5B9CCA4F, 0x7763E373, 0x682E6FF3, 0xD6B2B8A3,
	0x748F82EE, 0x5DEFB2FC, 0x78A5636F, 0x43172F60,
	0x84C87814, 0xA1F0AB72, 0x8CC70208, 0x1A6439EC,
	0x90BEFFFA, 0x23631E28, 0xA4506CEB, 0xDE82BDE9,
	0xBEF9A3F7, 0xB2C67915, 0xC67178F2, 0xE372532B,
	0xCA273ECE, 0xEA26619C, 0xD186B8C7, 0x21C0C207,
	0xEADA7DD6, 0xCDE0EB1E, 0xF57D4F7F, 0xEE6ED178,
	0x06F067AA, 0x72176FBA, 0x0A637DC5, 0xA2C898A6,
	0x113F9804, 0xBEF90DAE, 0x1B710B35, 0x131C471B,
	0x28DB77F5, 0x23047D84, 0x32CAAB7B, 0x40C72493,
	0x3C9EBE0A, 0x15C9BEBC, 0x431D67C4, 0x9C100D4C,
	0x4CC5D4BE, 0xCB3E42B6, 0x597F299C, 0xFC657E2A,
	0x5FCB6FAB, 0x3AD6FAEC, 0x6C44198C, 0x4A475817
};
/*******************************************************************************
 * Define the basic SHA512 or SHA512 macro functions.
 ******************************************************************************/

void ROTR64(u32 * x, u8 n, u32 * r)
{
	u8 m;
	if((n) < 33)
	{
		m = 32 - (n);
		r[1] 	= (u32)(SHR(x[1], n) | ((x[0]) << (m)));
		r[0] 	= (u32)(SHR(x[0], n) | ((x[1]) << (m)));
	}
	else
	{
		m = 64 - (n);
		n = (n) - 32;
		r[1] = (u32)(((x[0]) >> (n)) | ((x[1]) << (m)));
		r[0] = (u32)(((x[1]) >> (n)) | ((x[0]) << (m)));
	}
}

// just shift right
void SHR64(u32 * x, u8 n, u32 * r) {
	if((n) < 33)
	{
		r[1] = (SHR(x[1], n) | ((x[0]) << (32 - (n))));
		r[0] = SHR(x[0], n);
	}
	else
	{
		r[1] = ((x[0]) >> ((n) - 32));
		r[0] = 0x00000000;
	}
}

void CH64(u32 * x, u32 * y, u32 * z, u32 * r)
{
	r[0] = y[0] ^ z[0];
	r[1] = y[1] ^ z[1];
	r[0] = r[0] & x[0];
	r[1] = r[1] & x[1];
	r[0] = r[0] ^ z[0];
	r[1] = r[1] ^ z[1];
}

void MAJ64(u32 * x, u32 * y, u32 * z, u32 * r)
{
	r[0] = x[0] | y[0];
	r[1] = x[1] | y[1];
	r[0] = r[0] & z[0];
	r[1] = r[1] & z[1];
	r[0] = r[0] | (x[0] & y[0]);
	r[1] = r[1] | (x[1] & y[1]);
}

// the four basic macro functions need temp (tmpLeft, tmpRight) and result (rleft, rright) variables
void SIG0(u32 * x, u32 * r, u32 * tmp)
{
	ROTR64(x, 28, tmp);
	ROTR64(x, 34, r);
	r[0] ^= tmp[0];
	r[1] ^= tmp[1];
	ROTR64(x, 39, tmp);
	r[0] ^= tmp[0];
	r[1] ^= tmp[1];
}

void SIG1(u32 * x, u32 * r, u32 * tmp)
{
	ROTR64(x, 14, tmp);
	ROTR64(x, 18, r);
	r[0] = (u32)(r[0] ^ tmp[0]);
	r[1] = (u32)(r[1] ^ tmp[1]);
	ROTR64(x, 41, tmp);
	r[0] = (u32)(r[0] ^ tmp[0]);
	r[1] = (u32)(r[1] ^ tmp[1]);
}

void sig0(u32 * x, u32 * r, u32 * tmp)
{
	ROTR64(x, 1, tmp);
	ROTR64(x, 8, r);
	r[0] ^= tmp[0];
	r[1] ^= tmp[1];
	SHR64(x, 7, tmp);
	r[0] ^= tmp[0];
	r[1] ^= tmp[1];
}

void sig1(u32 * x, u32 * r, u32 * tmp)
{
	ROTR64(x, 19, tmp);
	ROTR64(x, 61, r);
	r[0] ^= tmp[0];
	r[1] ^= tmp[1];
	SHR64(x, 6, tmp);
	r[0] ^= tmp[0]; r[1] ^= tmp[1];
}

//Note: here compare first operand and the result, so the two values MUST NOT be the same value!!!
void add64(u32 * x, u32 * y, u32 * r)
{
	r[1] = x[1] + y[1];
	r[0] = x[0] + y[0];
	if(r[1] < x[1])
	{
		r[0] += 1;
	}
}

//the below macro functions need temp (tmpLeft, tmpRight) and result (rleft, rright) variables
#define Rleft(w, t)		&w[((t-16)<<1)&31]

u32 * Rright(u32 * w, u8 t)
{
	return &w[(((t-16)<<1)&31) + 1];
}


void ComputeNextWord64(u32 * w, u32 * r, u32 * tmp, u8 t)
{
	PX32		 		tempA;
	PX32		 		tempB;
	tempA = &w[((t-2)<<1) & 31];
	sig1(tempA, r, tmp);

	tempA = &w[((t-16)<<1)&31];
	add64(r, tempA, tempA);

	tempB = &w[((t-7)<<1)&31];
	add64(tempB, tempA, tempA);

	tempB = &w[((t-15)<<1)&31];
	sig0(tempB, r, tmp);
	add64(r, tempA, tempA);
}

// the below macro functions need temp (tmpLeft, tmpRight), result (rleft, rright) and roundTmp (rndTmplft, rndTmprgt) variables
void ROUND(u32 * a, u32 * b, u32 * c, u32 * d, u32 * e, u32 * f, u32 * g, u32 * h, u32 * w, u32 * k, u32 * r, u32 * tmp, u32 * rndTmp)
{
	//	SIG1(eleft, eright, rleft, rright);
	SIG1(e, r, tmp);
//	add64(hleft, hright, rleft, rright, rndTmplft, rndTmprgt);
	add64(h, r, rndTmp);
//	CH64(eleft, eright, fleft, fright, gleft, gright, rleft, rright);
	CH64(e, f, g, r);
//	add64(rleft, rright, rndTmplft, rndTmprgt, rndTmplft, rndTmprgt);
	add64(r, rndTmp, rndTmp);
//	add64(kleft, kright, rndTmplft, rndTmprgt, rndTmplft, rndTmprgt);
	add64(k, rndTmp, rndTmp);
//	add64(wleft, wright, rndTmplft, rndTmprgt, rndTmplft, rndTmprgt);
	add64(w, rndTmp, rndTmp);
//	SIG0(aleft, aright, rleft, rright);
	SIG0(a, r, tmp);
//	add64(rndTmplft, rndTmprgt, rleft, rright, hleft, hright);
	add64(rndTmp, r, h);
//	MAJ64(aleft, aright, bleft, bright, cleft, cright, rleft, rright);
	MAJ64(a, b, c, r);
//	add64(rleft, rright, hleft, hright, hleft, hright);
	add64(r, h, h);
//	add64(rndTmplft, rndTmprgt, dleft, dright, dleft, dright);
	add64(rndTmp, d, d);
}


/*******************************************************************************
 * Function name: SHA512Transform
 * Description: SHA512 or SHA512 basic transformation. Transforms state on block.
 * Input Parameters:
 *	   	u32 state[16]	    	    state of SHA512 (SHA512) (through global variable g_MDCtxt).
 *      u8 * blockBuf               block buffer (through global variable g_MDCtxt).
 * Output Parameters:
 * 		None
 * NOTE:
 ******************************************************************************/
void SoftSHA512_SHA512Transform(PX32 blockBuffer, PX32 icvBuffer)
{
	u32 	X_A			workSpace[6];
	u32 	X_A			backIcv[0x10];
	// fetch the state from global variable
	PX32		 		ICVPatrOne;
	PX32		 		ICVPatrTwo;
	PX32		 		ICVPatrThree;
	PX32		 		ICVPatrFour;
	PX32		 		ICVPatrFive;
	PX32		 		ICVPatrSix;
	PX32		 		ICVPatrSeven;
	PX32		 		ICVPatrEight;
	// temp variables used in macro functions
	PX32		 		r;
	PX32		 		tmp;
	PX32		 		rndTmp;
	PX32 				w;
	PX32 				constArray;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be big endian order
	BigEndian2LittleEndian((PX8)blockBuffer, SHA384_OR_512_BLOCK_SIZE);
#endif
	ecpMem_Copy(MEMtPX8(backIcv), MEMtPX8(icvBuffer), SHA512_ICV_SIZE);
	
	ICVPatrOne  		= &backIcv[0];
	ICVPatrTwo  		= &backIcv[2];
	ICVPatrThree  		= &backIcv[4];
	ICVPatrFour  		= &backIcv[6];
	ICVPatrFive  		= &backIcv[8];
	ICVPatrSix 			= &backIcv[10];
	ICVPatrSeven  		= &backIcv[12];
	ICVPatrEight	  	= &backIcv[14];

	r 					= &workSpace[0];
	tmp	 				= &workSpace[2];
	rndTmp 				= &workSpace[4];

	// cast unsigned char buffer to unsigned long
	w 					= &blockBuffer[0];
	constArray			= (PX32)&SHA512_CONSTANTS[0];

	// 80 rounds operations.
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, &w[0], &constArray[0], r, tmp, rndTmp);  //1
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, &w[2], &constArray[2], r, tmp, rndTmp);  //2
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, &w[4], &constArray[4], r, tmp, rndTmp);  //3
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, &w[6], &constArray[6], r, tmp, rndTmp);  //4
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, &w[8], &constArray[8], r, tmp, rndTmp);  //5
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, &w[10], &constArray[10], r, tmp, rndTmp);//6
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, &w[12], &constArray[12], r, tmp, rndTmp);//7
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, &w[14], &constArray[14], r, tmp, rndTmp);//8
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, &w[16], &constArray[16], r, tmp, rndTmp);//9
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, &w[18], &constArray[18], r, tmp, rndTmp);//10
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, &w[20], &constArray[20], r, tmp, rndTmp);//11
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, &w[22], &constArray[22], r, tmp, rndTmp);//12
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, &w[24], &constArray[24], r, tmp, rndTmp);//13
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, &w[26], &constArray[26], r, tmp, rndTmp);//14
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, &w[28], &constArray[28], r, tmp, rndTmp);//15
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, &w[30], &constArray[30], r, tmp, rndTmp);//16
	ComputeNextWord64(w, r, tmp, 16);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 16), &constArray[32], r, tmp, rndTmp);//17
	ComputeNextWord64(w, r, tmp, 17);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 17), &constArray[34], r, tmp, rndTmp);//18
	ComputeNextWord64(w, r, tmp, 18);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 18), &constArray[36], r, tmp, rndTmp);//19
	ComputeNextWord64(w, r, tmp, 19);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 19), &constArray[38], r, tmp, rndTmp);//20
	ComputeNextWord64(w, r, tmp, 20);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 20), &constArray[40], r, tmp, rndTmp);//21
	ComputeNextWord64(w, r, tmp, 21);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 21), &constArray[42], r, tmp, rndTmp);//22
	ComputeNextWord64(w, r, tmp, 22);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 22), &constArray[44], r, tmp, rndTmp);//23
	ComputeNextWord64(w, r, tmp, 23);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 23), &constArray[46], r, tmp, rndTmp);//24
	ComputeNextWord64(w, r, tmp, 24);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 24), &constArray[48], r, tmp, rndTmp);//25
	ComputeNextWord64(w, r, tmp, 25);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 25), &constArray[50], r, tmp, rndTmp);//26
	ComputeNextWord64(w, r, tmp, 26);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 26), &constArray[52], r, tmp, rndTmp);//27
	ComputeNextWord64(w, r, tmp, 27);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 27), &constArray[54], r, tmp, rndTmp);//28
	ComputeNextWord64(w, r, tmp, 28);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 28), &constArray[56], r, tmp, rndTmp);//29
	ComputeNextWord64(w, r, tmp, 29);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 29), &constArray[58], r, tmp, rndTmp);//30
	ComputeNextWord64(w, r, tmp, 30);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 30), &constArray[60], r, tmp, rndTmp);//31
	ComputeNextWord64(w, r, tmp, 31);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 31), &constArray[62], r, tmp, rndTmp);//32
	ComputeNextWord64(w, r, tmp, 32);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 32), &constArray[64], r, tmp, rndTmp);//33
	ComputeNextWord64(w, r, tmp, 33);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 33), &constArray[66], r, tmp, rndTmp);//34
	ComputeNextWord64(w, r, tmp, 34);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 34), &constArray[68], r, tmp, rndTmp);//35
	ComputeNextWord64(w, r, tmp, 35);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 35), &constArray[70], r, tmp, rndTmp);//36
	ComputeNextWord64(w, r, tmp, 36);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 36), &constArray[72], r, tmp, rndTmp);//37
	ComputeNextWord64(w, r, tmp, 37);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 37), &constArray[74], r, tmp, rndTmp);//38
	ComputeNextWord64(w, r, tmp, 38);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 38), &constArray[76], r, tmp, rndTmp);//39
	ComputeNextWord64(w, r, tmp, 39);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 39), &constArray[78], r, tmp, rndTmp);//40
	ComputeNextWord64(w, r, tmp, 40);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 40), &constArray[80], r, tmp, rndTmp);//41
	ComputeNextWord64(w, r, tmp, 41);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 41), &constArray[82], r, tmp, rndTmp);//42
	ComputeNextWord64(w, r, tmp, 42);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 42), &constArray[84], r, tmp, rndTmp);//43
	ComputeNextWord64(w, r, tmp, 43);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 43), &constArray[86], r, tmp, rndTmp);//44
	ComputeNextWord64(w, r, tmp, 44);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 44), &constArray[88], r, tmp, rndTmp);//45
	ComputeNextWord64(w, r, tmp, 45);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 45), &constArray[90], r, tmp, rndTmp);//46
	ComputeNextWord64(w, r, tmp, 46);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 46), &constArray[92], r, tmp, rndTmp);//47
	ComputeNextWord64(w, r, tmp, 47);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 47), &constArray[94], r, tmp, rndTmp);//48
	ComputeNextWord64(w, r, tmp, 48);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 48), &constArray[96], r, tmp, rndTmp);//49
	ComputeNextWord64(w, r, tmp, 49);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 49), &constArray[98], r, tmp, rndTmp);//50
	ComputeNextWord64(w, r, tmp, 50);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 50), &constArray[100], r, tmp, rndTmp);//51
	ComputeNextWord64(w, r, tmp, 51);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 51), &constArray[102], r, tmp, rndTmp);//52
	ComputeNextWord64(w, r, tmp, 52);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 52), &constArray[104], r, tmp, rndTmp);//53
	ComputeNextWord64(w, r, tmp, 53);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 53), &constArray[106], r, tmp, rndTmp);//54
	ComputeNextWord64(w, r, tmp, 54);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 54), &constArray[108], r, tmp, rndTmp);//55
	ComputeNextWord64(w, r, tmp, 55);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 55), &constArray[110], r, tmp, rndTmp);//56
	ComputeNextWord64(w, r, tmp, 56);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 56), &constArray[112], r, tmp, rndTmp);//57
	ComputeNextWord64(w, r, tmp, 57);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 57), &constArray[114], r, tmp, rndTmp);//58
	ComputeNextWord64(w, r, tmp, 58);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 58), &constArray[116], r, tmp, rndTmp);//59
	ComputeNextWord64(w, r, tmp, 59);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 59), &constArray[118], r, tmp, rndTmp);//60
	ComputeNextWord64(w, r, tmp, 60);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 60), &constArray[120], r, tmp, rndTmp);//61
	ComputeNextWord64(w, r, tmp, 61);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 61), &constArray[122], r, tmp, rndTmp);//62
	ComputeNextWord64(w, r, tmp, 62);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 62), &constArray[124], r, tmp, rndTmp);//63
	ComputeNextWord64(w, r, tmp, 63);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 63), &constArray[126], r, tmp, rndTmp);//64
	ComputeNextWord64(w, r, tmp, 64);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 64), &constArray[128], r, tmp, rndTmp);//65
	ComputeNextWord64(w, r, tmp, 65);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 65), &constArray[130], r, tmp, rndTmp);//66
	ComputeNextWord64(w, r, tmp, 66);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 66), &constArray[132], r, tmp, rndTmp);//67
	ComputeNextWord64(w, r, tmp, 67);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 67), &constArray[134], r, tmp, rndTmp);//68
	ComputeNextWord64(w, r, tmp, 68);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 68), &constArray[136], r, tmp, rndTmp);//69
	ComputeNextWord64(w, r, tmp, 69);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 69), &constArray[138], r, tmp, rndTmp);//70
	ComputeNextWord64(w, r, tmp, 70);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 70), &constArray[140], r, tmp, rndTmp);//71
	ComputeNextWord64(w, r, tmp, 71);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 71), &constArray[142], r, tmp, rndTmp);//72
	ComputeNextWord64(w, r, tmp, 72);
	ROUND(ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, Rleft(w, 72), &constArray[144], r, tmp, rndTmp);//73
	ComputeNextWord64(w, r, tmp, 73);
	ROUND(ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, Rleft(w, 73), &constArray[146], r, tmp, rndTmp);//74
	ComputeNextWord64(w, r, tmp, 74);
	ROUND(ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, Rleft(w, 74), &constArray[148], r, tmp, rndTmp);//75
	ComputeNextWord64(w, r, tmp, 75);
	ROUND(ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, Rleft(w, 75), &constArray[150], r, tmp, rndTmp);//76
	ComputeNextWord64(w, r, tmp, 76);
	ROUND(ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, ICVPatrFour, Rleft(w, 76), &constArray[152], r, tmp, rndTmp);//77
	ComputeNextWord64(w, r, tmp, 77);
	ROUND(ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, ICVPatrThree, Rleft(w, 77), &constArray[154], r, tmp, rndTmp);//78
	ComputeNextWord64(w, r, tmp, 78);
	ROUND(ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, ICVPatrTwo, Rleft(w, 78), &constArray[156], r, tmp, rndTmp);//79
	ComputeNextWord64(w, r, tmp, 79);
	ROUND(ICVPatrTwo, ICVPatrThree, ICVPatrFour, ICVPatrFive, ICVPatrSix, ICVPatrSeven, ICVPatrEight, ICVPatrOne, Rleft(w, 79), &constArray[158], r, tmp, rndTmp);//80

	// update state
	add64(ICVPatrOne, &icvBuffer[0], &icvBuffer[0]);
	add64(ICVPatrTwo, &icvBuffer[2], &icvBuffer[2]);
	add64(ICVPatrThree, &icvBuffer[4], &icvBuffer[4]);
	add64(ICVPatrFour, &icvBuffer[6], &icvBuffer[6]);
	add64(ICVPatrFive, &icvBuffer[8], &icvBuffer[8]);
	add64(ICVPatrSix, &icvBuffer[10], &icvBuffer[10]);
	add64(ICVPatrSeven, &icvBuffer[12], &icvBuffer[12]);
	add64(ICVPatrEight, &icvBuffer[14], &icvBuffer[14]);
}
#endif


#endif  /* (WITH_SOFT_SHA384 || WITH_SOFT_SHA512) */
