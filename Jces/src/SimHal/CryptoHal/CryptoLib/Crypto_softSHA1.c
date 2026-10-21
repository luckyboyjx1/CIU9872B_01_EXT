/*
 * Crypto_softSHA1.c
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */


#include "Crypto_softSHA1.h"


#if WITH_SOFT_SHA1

void SoftSHA1_SHA1Transform(PX32 blockBuffer, PX32 icvBuffer)
{
	u32 	X_A 					ICVPatrOne;
	u32 	X_A 					ICVPatrTwo;
	u32 	X_A 					ICVPatrThree;
	u32 	X_A 					ICVPatrFour;
	u32 	X_A 					ICVPatrFive;

    u32 	X_A						tempBlockbuffer;
    u32 	X_A						tempForSHA1;            // Tempory value

    u8 		I_A						loop;               // loop counter

#if U8_2_U32_INVERSE
    BigEndian2LittleEndian((PX8)blockBuffer, SHA1_BLOCK_SIZE);
#else
// nothing to do!
#endif

    // init the tempory variable with the last message digest
    ICVPatrOne 						= icvBuffer[0];
    ICVPatrTwo 						= icvBuffer[1];
    ICVPatrThree 					= icvBuffer[2];
    ICVPatrFour 					= icvBuffer[3];
    ICVPatrFive 					= icvBuffer[4];

    // Main loop
    for( loop = 0; loop < 80 ; loop++ )
    {
        // Compute Temp = Ft(B,C,D) + Kt where Kt is constant
        if ( loop < 20)
        {
            tempForSHA1 			= (ICVPatrFour ^ (ICVPatrTwo & ( ICVPatrThree ^ ICVPatrFour))) + 0x5A827999 ;
        }
        else if (loop < 40)
        {
			tempForSHA1 			= ( ICVPatrTwo ^ ICVPatrThree ^ ICVPatrFour ) + 0x6ED9EBA1;
        }
        else if (loop < 60)
        {
			tempForSHA1 			= (( ICVPatrTwo & ICVPatrThree ) | ( ICVPatrFour & ( ICVPatrTwo | ICVPatrThree ))) + 0x8F1BBCDC;
        }
		else
		{
			tempForSHA1 			= ( ICVPatrTwo ^ ICVPatrThree ^ ICVPatrFour ) + 0xCA62C1D6;
		}

        // Calculate FT(B,C,D) + Kt + W(s) where s = t mod 16
        if ( loop >= 16 )
        {
            tempBlockbuffer    		= blockBuffer[(loop+13)&0xF] ^ blockBuffer[(loop+8)&0xF] ^ blockBuffer[loop&0xF] ^ blockBuffer[(loop+2)&0xF] ;
            blockBuffer[loop & 0xF] =  (tempBlockbuffer>>31)|(tempBlockbuffer<<1);
        }

        tempForSHA1 				= tempForSHA1 + blockBuffer[loop&0xF] + ICVPatrFive;

        ICVPatrFive 				= ICVPatrOne;

        ICVPatrOne 					= (ICVPatrOne>>27)|(ICVPatrOne<<5);
        ICVPatrOne 					= tempForSHA1 + ICVPatrOne;

        tempForSHA1 				= (ICVPatrTwo>>2)|(ICVPatrTwo<<30);

        ICVPatrTwo 					= ICVPatrFive;
        ICVPatrFive 				= ICVPatrFour;
        ICVPatrFour 				= ICVPatrThree;
        ICVPatrThree 				= tempForSHA1;

    }

    icvBuffer[0] 					+= ICVPatrOne;
    icvBuffer[1] 					+= ICVPatrTwo;
    icvBuffer[2] 					+= ICVPatrThree;
    icvBuffer[3] 					+= ICVPatrFour;
    icvBuffer[4] 					+= ICVPatrFive;

}
#endif /* WITH_SOFT_SHA1 */
