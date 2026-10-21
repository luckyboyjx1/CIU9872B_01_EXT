/*
 * HASH_SM3_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "HASH_API.h"
#include "Crypto_softSM3.h"
#include "chipHead.h"

#if WITH_SM3


#if !WITH_SOFT_SM3
void SM3Transform(PX32 blockBuffer, PX32 icvBuffer)
{
    
}
#endif

void HASH_SM3InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
//	PX8      workSpaceDataLen; 
//	if(myInitParam -> blockNum == 0)
//	{
//		HED_SM3Initial((u32 *)myInitParam->hashResult);
//	}
//	else
//	{
//		ecpMem_Copy(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->hashIVSize);
//		workSpaceDataLen = PX8_A_U8((myInitParam->hashResult), (myAPIParam -> hashIVSize));
//		PX_W_U32(workSpaceDataLen, (myInitParam -> blockNum));
//	}
	return;
}


void HASH_SM3UpdateMain(HASH_Init_Param_t * myInitParam)
{
//	u32 result;
//	result = HED_SM3Block((u32 *)myInitParam->hashResult,(u32 *)myInitParam->hashWorkSpace,SM3_BLOCK_SIZE);
//	if(0 != result)
//	{
//		while(1);
//	}
	return;	
}


void HASH_SM3FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
//	u32 result;
//	result = HED_SM3Block((u32 *)myInitParam->hashResult,(u32 *)myInitParam->hashWorkSpace,myInitParam->remainLength);
//	if(0 != result)
//	{
//		while(1);
//	}

	return;	
}
#endif // WITH_SM3

