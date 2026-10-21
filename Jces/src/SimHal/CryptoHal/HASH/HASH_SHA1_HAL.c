/*
 * HASH_SHA1_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "HASH_API.h"
#include "Crypto_softSHA1.h"
#include "chipHead.h"

#if WITH_SHA1


#if !WITH_SOFT_SHA1
void SHA1Transform(PX32 blockBuffer, PX32 icvBuffer)
{
    
}
#endif

void HASH_SHA1InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	ecpMem_Copy(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->hashIVSize);
	return;
}


void HASH_SHA1UpdateMain(HASH_Init_Param_t * myInitParam)
{
	SHA1Transform((u32 *)myInitParam->hashWorkSpace,(u32 *)myInitParam->hashResult);
	return;
}


void HASH_SHA1FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	u8	 		X_A 		tempLen;
	u8	 		X_A 		remainLength;
	u32			X_A			bitNum;

	remainLength = myInitParam->remainLength;
	myInitParam->hashWorkSpace[remainLength] = 0x80;  // padding 0x80
	remainLength++;

	// append bit ( not byte ) length of unpadded message as 64-bit big-endian integer to message
	// To SHA1 the max update message length is 16383 bytes, the max dofinal message length is 32767 bytes,
	tempLen = myAPIParam->blockSize - remainLength;

	ecpMem_Fill(&myInitParam->hashWorkSpace[remainLength], 0, tempLen);

	if (tempLen < myAPIParam->msgBitByteLen)
	{
		// Launch the SHA transform
		SHA1Transform((u32 *)myInitParam->hashWorkSpace,(u32 *)myInitParam->hashResult);

		ecpMem_Fill(myInitParam->hashWorkSpace, 0, myAPIParam->blockSize);
	}

	bitNum = (u32)(((((u32)myInitParam->blockNum) << myAPIParam->blockNumShift) + myInitParam->remainLength) << 3);

	// append bit ( not byte ) length of unpadded message as 64-bit big-endian integer to message.
	myInitParam->hashWorkSpace[myAPIParam->blockSize - 3] = (u8)(bitNum >> 16);
	myInitParam->hashWorkSpace[myAPIParam->blockSize - 2] = (u8)(bitNum >> 8);
	myInitParam->hashWorkSpace[myAPIParam->blockSize - 1] = (u8)(bitNum & 0xFF);

	// Launch the SHA transform
	SHA1Transform((u32 *)myInitParam->hashWorkSpace,(u32 *)myInitParam->hashResult);

#if U8_2_U32_INVERSE
	BigEndian2LittleEndian((PX8)myInitParam->hashResult, (myAPIParam->digestSize));
#else
// nothing to do!
#endif

	return;
}
#endif // WITH_SHA1

