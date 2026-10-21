/*
 * HASH_MD_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */
#include "HASH_API.h"
#include "Crypto_softMD.h"

#if WITH_MD5

#if !WITH_SOFT_MD5
void MDTransform(PX32 blockBuffer, PX32 icvBuffer)
{
    
}
#endif

void HASH_MDInitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	ecpMem_Copy(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->hashIVSize);
	return;
}


void HASH_MDUpdateMain(HASH_Init_Param_t * myInitParam)
{
	MDTransform(MEMtPM32(myInitParam->hashWorkSpace),MEMtPM32(myInitParam->hashResult));
	return;
}


void HASH_MDFinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	u32			X_A			bitNum;
	u8	 		X_A 		tempLen;
	u8	 		X_A 		remainLength;


	remainLength = myInitParam->remainLength;
	myInitParam->hashWorkSpace[remainLength] = 0x80;  // padding 0x80
	U8_INC(remainLength);

	// append bit ( not byte ) length of unpadded message as 64-bit big-endian integer to message
	// To MD the max update message length is 16383 bytes, the max dofinal message length is 32767 bytes,
	tempLen = U8_S_U8(myAPIParam->blockSize, remainLength);

	ecpMem_Fill(&myInitParam->hashWorkSpace[remainLength], 0, tempLen);

	if (tempLen < myAPIParam->msgBitByteLen)
	{
		// Launch the SHA transform
		MDTransform(MEMtPM32(myInitParam->hashWorkSpace),MEMtPM32(myInitParam->hashResult));

		ecpMem_Fill(myInitParam->hashWorkSpace, 0, myAPIParam->blockSize);
	}

	bitNum = U32_SHL_U8(U32_A_U8(U32_SHL_U8(myInitParam->blockNum, myAPIParam->blockNumShift), myInitParam->remainLength), 3); 

	// append bit ( not byte ) length of unpadded message as 64-bit little-endian integer to message.
		myInitParam->hashWorkSpace[56] = (u8)(bitNum & 0xFF);
		myInitParam->hashWorkSpace[57] = (u8)(bitNum >> 8);
		myInitParam->hashWorkSpace[58] = (u8)(bitNum >> 16);
//		myInitParam->hashWorkSpace[59] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[60] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[61] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[62] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[63] = (u8)(0x00);
	// Launch the SHA transform
	MDTransform(MEMtPM32(myInitParam->hashWorkSpace),MEMtPM32(myInitParam->hashResult));

#if U8_2_U32_INVERSE
	// nothing to do!
#else
	BigEndian2LittleEndian(MEMtPX8(myInitParam->hashResult), (u16)(myAPIParam->digestSize));
#endif

	return;
}
#endif // WITH_MD5

