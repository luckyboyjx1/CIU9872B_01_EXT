/*
 * Crypto_softMD.c
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */


#include "Crypto_softMD.h"

#if WITH_SOFT_MD5
static u32 CONST MD5_CONSTANTS[64] =
{
	0xD76AA478,0xE8C7B756,0x242070DB,0xC1BDCEEE,
	0xF57C0FAF,0x4787C62A,0xA8304613,0xFD469501,
	0x698098D8,0x8B44F7AF,0xFFFF5BB1,0x895CD7BE,
	0x6B901122,0xFD987193,0xA679438E,0x49B40821,
	0xF61E2562,0xC040B340,0x265E5A51,0xE9B6C7AA,
	0xD62F105D,0x02441453,0xD8A1E681,0xE7D3FBC8,
	0x21E1CDE6,0xC33707D6,0xF4D50D87,0x455A14ED,
	0xA9E3E905,0xFCEFA3F8,0x676F02D9,0x8D2A4C8A,
	0xFFFA3942,0x8771F681,0x6D9D6122,0xFDE5380C,
	0xA4BEEA44,0x4BDECFA9,0xF6BB4B60,0xBEBFBC70,
	0x289B7EC6,0xEAA127FA,0xD4EF3085,0x04881D05,
	0xD9D4D039,0xE6DB99E5,0x1FA27CF8,0xC4AC5665,
	0xF4292244,0x432AFF97,0xAB9423A7,0xFC93A039,
	0x655B59C3,0x8F0CCC92,0xFFEFF47D,0x85845DD1,
	0x6FA87E4F,0xFE2CE6E0,0xA3014314,0x4E0811A1,
	0xF7537E82,0xBD3AF235,0x2AD7D2BB,0xEB86D391
};
static u8 CONST SHIFT_ROUNDS[64] =
{
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

/* cyclically rotates Sdata over nBit bits to the left */
/* Sdata must be of an unsigned 32 bits type and 0 <= nBit < 32. */
#define ROTATE_LEFT(Sdata, nBit) (((Sdata) << (nBit)) | ((Sdata) >> (32-(nBit))))

/**********************************************************************************
 * Function name: MD5Transform
 * Description: This function does MD5 kernel operation.
 * Input parameter:
 *	   	XDATA_U32_P state	    	state of MD5.
 *      XDATA_U8_P blockBuff        block buffer.
 * Output parameter:
 *		none.
 * NOTE:
 **********************************************************************************/
void SoftMD_MDTransform(PX32 blockBuffer, PX32 icvBuffer)
{
	//hash a single 512-bit chunk. This is the core of the algorithm
	u32 	X_A				ICVPatrOne;
	u32 	X_A				ICVPatrTwo;
	u32 	X_A				ICVPatrThree;
	u32 	X_A				ICVPatrFour;
	u32 	X_A				tempIcvbuffer;
	u32 	X_A 			tempForMD;
	PX32		 			tempBlockbuffer;
	u8 		I_A 			loop;
	u8 		I_A 			tempForLoop;
	PATCH_HAL_CRYPTO_SOFTMD_001() ;	
	//convert bufferIn into sixteen 32-bit words:
	tempBlockbuffer 		= blockBuffer;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be little endian order for RIPEMD160
#else
	// break buffer into sixteen 32-bit little-endian dwords tmp[i], 0 ¡Ü i ¡Ü 15
	BigEndian2LittleEndian((PX8)tempBlockbuffer, (u16)(MD_BLOCK_SIZE));
#endif

	//copy state[] to working vars:
	ICVPatrOne 				= icvBuffer[0];
	ICVPatrTwo 				= icvBuffer[1];
	ICVPatrThree 			= icvBuffer[2];
	ICVPatrFour 			= icvBuffer[3];

	//main loop:
	for (loop = 0; loop < 64; U8_INC(loop))
	{
		if (loop < 16)
		{
			//f = (b & c) | ((~b) & d);
			tempIcvbuffer 	= ((~ICVPatrTwo) & ICVPatrFour);
			tempIcvbuffer 	= tempIcvbuffer | (ICVPatrTwo & ICVPatrThree);
			tempForLoop 	= loop;
		}
		else if (loop < 32)
		{
			//f = (d & b) | ((~d) & c);
			tempIcvbuffer 	= ((~ICVPatrFour) & ICVPatrThree);
			tempIcvbuffer 	= tempIcvbuffer | (ICVPatrFour & ICVPatrTwo);
			//g = (5 * i + 1) & 0x0F;
			tempForLoop 	= U8_A_U8(U8_M_U8(loop, 5), 1) & 0x0F;
		}
		else if (loop < 48)
		{
			tempIcvbuffer 	= ICVPatrTwo ^ ICVPatrThree ^ ICVPatrFour;
			//g = (3 * i + 5) & 0x0F;
			tempForLoop 	= U8_A_U8(U8_M_U8(loop, 3), 5) & 0x0F;
		}
		else
		{
			//f = c ^ (b | (~d));
			tempIcvbuffer 	= (ICVPatrTwo | (~ICVPatrFour));
			tempIcvbuffer 	^= ICVPatrThree;
			//g = (7 * i) & 0x0F;
			tempForLoop 	= U8_M_U8(loop, 7) & 0x0F;
		}

		tempForMD 			= ICVPatrFour;
		ICVPatrFour 		= ICVPatrThree;
		ICVPatrThree 		= ICVPatrTwo;
		//b += ROTATE_LEFT((a + f + MD5_CONSTANTS[i] + buffer[g]), SHIFT_ROUNDS[i]);
		//ICVPatrTwo 			= U32_A_U32(ICVPatrTwo, ROTATE_LEFT((U32_A_U32(U32_A_U32(U32_A_U32(ICVPatrOne, tempIcvbuffer), MD5_CONSTANTS[loop]), tempBlockbuffer[tempForLoop])), SHIFT_ROUNDS[loop]));
		ICVPatrOne			= U32_A_U32(U32_A_U32(U32_A_U32(ICVPatrOne, tempIcvbuffer), MD5_CONSTANTS[loop]), tempBlockbuffer[tempForLoop]);
		ICVPatrTwo			= U32_A_U32(ICVPatrTwo, ROTATE_LEFT(ICVPatrOne, SHIFT_ROUNDS[loop]));
		ICVPatrOne 			= tempForMD;
	}

	//add this chunk's hash to result so far:
	icvBuffer[0] 			= U32_A_U32(icvBuffer[0], ICVPatrOne);
	icvBuffer[1] 			= U32_A_U32(icvBuffer[1], ICVPatrTwo);
	icvBuffer[2] 			= U32_A_U32(icvBuffer[2], ICVPatrThree);
	icvBuffer[3] 			= U32_A_U32(icvBuffer[3], ICVPatrFour);
}
#endif /* WITH_SOFT_MD5 */
