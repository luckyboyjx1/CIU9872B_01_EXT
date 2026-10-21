/*
 * HASH_SHA512_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "HASH_API.h"
#include "Crypto_softSHA512.h"
#include "chipHead.h"

#if WITH_SHA512

#if !WITH_SOFT_SHA512
void SHA512Transform(PX32 blockBuffer, PX32 icvBuffer)
{
    
}
#endif

void HASH_SHA512InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
//	PX8      workSpaceDataLen; 
//	if(myInitParam -> blockNum == 0)
//	{
//		HED_SHA512_Init((u32 *)myInitParam->hashResult);
//	}
//	else
//	{
//		ecpMem_Copy(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->hashIVSize);
//		workSpaceDataLen = PX8_A_U8((myInitParam->hashResult), (myAPIParam -> hashIVSize));
//		PX_W_U32(workSpaceDataLen, 0);
//		workSpaceDataLen = PX8_A_U8(workSpaceDataLen, 4);
//		PX_W_U32(workSpaceDataLen, (myInitParam -> blockNum));
//	}
//	return;
}


void HASH_SHA512UpdateMain(HASH_Init_Param_t * myInitParam)
{
//	u32 result;
//	
//	result = HED_SHA512_Block((u32 *)myInitParam->hashResult,(u32 *)myInitParam->hashWorkSpace,SHA512_BLOCK_SIZE);
//	if(0 != result)
//	{
//		while(1);
//	}

	return;
}


void HASH_SHA512FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
//	u32 result;
//	
//	result = HED_SHA512_Block((u32 *)myInitParam->hashResult,(u32 *)myInitParam->hashWorkSpace,myInitParam->remainLength);
//	if(0 != result)
//	{
//		while(1);
//	}
	
	return;
}
#endif // WITH_SHA512

