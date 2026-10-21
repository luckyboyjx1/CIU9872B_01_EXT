/*
 * Crypto_softSHA256.c
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */


#include "Crypto_softSHA256.h"


#if (WITH_SOFT_SHA256 || WITH_SOFT_SHA224)


#if WITH_SOFT_SHA224
#define	SHA224_OR_256_BLOCK_SIZE	SHA224_BLOCK_SIZE
#else	// WITH_SOFT_SHA256
#define	SHA224_OR_256_BLOCK_SIZE	SHA256_BLOCK_SIZE
#endif //WITH_SOFT_SHA224


//-------------------------------------------------------------------------
//  Function Name : SHA256_transform
/// @brief  This function computes the SHA1 transformation from the message shedule.
/// @note \b local \b Function
///
/// @param[in] Ws   the message shedule on 16 words(512 bits)
/// @param[in] hash the result area on 8 words (256 bits):the last message digest or the area initialized
/// @param[out] hash the result area on 8 words (256 bits)
///
//-------------------------------------------------------------------------

#define mod_16(x) ((x) & 0xF)

#define rotr(x,n) ((x>>n)|(x<<(32-n)))

#define Ch(x,y,z) (z^(x&(y^z)))
#define Maj(x,y,z) ((x&y)|(z&(x|y)))


// for SHA224 and SHA256

#define Si0(x) (rotr(x,2)^rotr(x,13)^rotr(x,22))
#define Si1(x) (rotr(x,6)^rotr(x,11)^rotr(x,25))
#define s0(x) (rotr(x,7)^rotr(x,18)^(x>>3))
#define s1(x) (rotr(x,17)^rotr(x,19)^(x>>10))


static u32 CONST K_256[64]= {
0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

void SoftSHA256_SHA256Transform(PX32 blockBuffer, PX32 icvBuffer)
{
	u32 	X_A 		ICVPatrOne;
	u32 	X_A 		ICVPatrTwo;
	u32 	X_A 		ICVPatrThree;
	u32 	X_A 		ICVPatrFour;
	u32 	X_A 		ICVPatrFive;
	u32 	X_A 		ICVPatrSix;
	u32 	X_A 		ICVPatrSeven;
	u32 	X_A 		ICVPatrEight;
    u32 	X_A			tempIcvbuffer;
    u32 	X_A			tempForSHA256;           // Tempory value
    u8 		I_A			loop;               	// loop counter

#if U8_2_U32_INVERSE
    BigEndian2LittleEndian((PX8)blockBuffer, SHA224_OR_256_BLOCK_SIZE);
#else
// nothing to do!
#endif

    // init the tempory variable with the last message digest
    ICVPatrOne 			= icvBuffer[0];
    ICVPatrTwo 			= icvBuffer[1];
    ICVPatrThree 		= icvBuffer[2];
    ICVPatrFour 		= icvBuffer[3];
    ICVPatrFive 		= icvBuffer[4];
    ICVPatrSix 			= icvBuffer[5];
    ICVPatrSeven 		= icvBuffer[6];
    ICVPatrEight 		= icvBuffer[7];

    // Main loop
    for( loop=0 ; loop < 64 ; loop++ )
    {
        if ( loop >= 16 )
        {
            blockBuffer[mod_16(loop)] += s1(blockBuffer[mod_16(loop+14)])+s0(blockBuffer[mod_16(loop+1)])+blockBuffer[mod_16(loop+9)];
        }

        tempIcvbuffer 	= ICVPatrEight + Si1(ICVPatrFive) + Ch(ICVPatrFive,ICVPatrSix,ICVPatrSeven) + K_256[loop] + blockBuffer[mod_16(loop)];

        tempForSHA256 	= Si0(ICVPatrOne) + Maj(ICVPatrOne,ICVPatrTwo,ICVPatrThree);
        ICVPatrEight 	= ICVPatrSeven;
        ICVPatrSeven 	= ICVPatrSix;
        ICVPatrSix 		= ICVPatrFive;
        ICVPatrFive 	= ICVPatrFour + tempIcvbuffer;
        ICVPatrFour 	= ICVPatrThree;
        ICVPatrThree 	= ICVPatrTwo;
        ICVPatrTwo 		= ICVPatrOne;
        ICVPatrOne 		= tempIcvbuffer + tempForSHA256;
    }

    icvBuffer[0] 		+= ICVPatrOne;
    icvBuffer[1] 		+= ICVPatrTwo;
    icvBuffer[2] 		+= ICVPatrThree;
    icvBuffer[3] 		+= ICVPatrFour;
    icvBuffer[4] 		+= ICVPatrFive;
    icvBuffer[5] 		+= ICVPatrSix;
    icvBuffer[6] 		+= ICVPatrSeven;
    icvBuffer[7] 		+= ICVPatrEight;
}

#endif /* (WITH_SOFT_SHA256 || WITH_SOFT_SHA224) */

