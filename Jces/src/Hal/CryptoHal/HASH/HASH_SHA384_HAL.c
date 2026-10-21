/*
 * HASH_SHA384_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "HASH_API.h"
#include "Crypto_softSHA384.h"
#include "chipHead.h"

#if WITH_SHA384

void HASH_SHA384InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
//		PX8      workSpaceDataLen; 
//	if(myInitParam -> blockNum == 0)
//	{
//		HED_SHA384_Init((unsigned int *)myInitParam->hashResult);
//	}
//	else
//	{
//		ecpMem_Copy(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->hashIVSize);
//		workSpaceDataLen = PX8_A_U8((myInitParam->hashResult), (myAPIParam -> hashIVSize));
//		PX_W_U32(workSpaceDataLen, 0);
//		workSpaceDataLen = PX8_A_U8(workSpaceDataLen, 4);
//		PX_W_U32(workSpaceDataLen, (myInitParam -> blockNum));
//	}
	
	return;
}


void HASH_SHA384UpdateMain(HASH_Init_Param_t * myInitParam)
{
//	u32 result;
//	
//	result = HED_SHA384_Block((unsigned int *)myInitParam->hashResult,(unsigned int *)myInitParam->hashWorkSpace,SHA384_BLOCK_SIZE);
//	if(0 != result)
//	{
//		while(1);
//	}

	return;
}


void HASH_SHA384FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
//	u32 result;
//	
//	result = HED_SHA384_Block((unsigned int *)myInitParam->hashResult,(unsigned int *)myInitParam->hashWorkSpace,myInitParam->remainLength);
//	if(0 != result)
//	{
//		while(1);
//	}
	
	return;
	
	
}
#endif // WITH_SHA384

