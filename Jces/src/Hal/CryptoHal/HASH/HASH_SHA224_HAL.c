/*
 * HASH_SHA224_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "HASH_API.h"
#include "Crypto_softSHA224.h"
#include "chipHead.h"

#if WITH_SHA224

#if !WITH_SOFT_SHA224
void SHA224Transform(PX32 blockBuffer, PX32 icvBuffer)
{

}
#endif

void HASH_SHA224InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	MHSHA256_Context ctx;
	PX8      workSpaceDataLen; 
	
	workSpaceDataLen = PX8_A_U8((myInitParam->hashResult), (myAPIParam -> hashIVSize));
	if(myInitParam -> blockNum == 0)
	{
		MHSHA224_Starts(&ctx);
		ecpMem_Copy(myInitParam->hashResult, ctx.au32State, myAPIParam->hashIVSize);
		PX_W_U32(workSpaceDataLen, ctx.au32Total[0]);
	}
	else
	{
		memCopyToRam(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->hashIVSize);
		PX_W_U32(workSpaceDataLen, (myInitParam -> blockNum<< (myAPIParam -> blockNumShift) ));//<< (myAPIParam -> blockNumShift) << 3
	}
	return;
}

void HASH_SHA224UpdateMain(HASH_Init_Param_t * myInitParam)
{
	MHSHA256_Context ctx;
	PX8      workSpaceDataLen; 
	u32 result;
	
	memset(&ctx,0,sizeof(ctx));
	
				//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_SYMC,ENABLE);
	
	workSpaceDataLen = PX8_A_U8((myInitParam->hashResult), SHA224_ICV_SIZE);
	ctx.au32Total[0] = PX_R_U32(workSpaceDataLen);
	
	ecpMem_Copy(MEMtPX8(ctx.au32State), myInitParam->hashResult, SHA224_ICV_SIZE);
	
	result = MHSHA224_Update(&ctx,myInitParam->hashWorkSpace,SHA224_BLOCK_SIZE);
	
		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_SYMC,DISABLE);
	
	if(MH_RET_SHA_SUCCESS != result)
	{
		while(1);
	}
	
	ecpMem_Copy(myInitParam->hashResult, MEMtPX8(ctx.au32State),SHA224_ICV_SIZE);
	PX_W_U32(workSpaceDataLen, ctx.au32Total[0]);
	
	return;
}

void HASH_SHA224FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	MHSHA256_Context ctx;
	u32 result;
	
	memset(&ctx,0,sizeof(ctx));
	
					//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_SYMC,ENABLE);
	
	ctx.au32Total[0] = (u32)(((u32)myInitParam->blockNum) << myAPIParam->blockNumShift);
	ctx.au32Total[1] = 0;				//(u32)(((u32)myInitParam->blockNum) << myAPIParam->blockNumShift);
	ecpMem_Copy(MEMtPX8(ctx.au32State), myInitParam->hashResult, SHA224_ICV_SIZE);
	
	result = MHSHA224_Update(&ctx, myInitParam->hashWorkSpace, myInitParam->remainLength);
	if(MH_RET_SHA_SUCCESS != result)
	{
		while(1);
	}
	result = MHSHA224_Finish(&ctx,myInitParam->hashResult);
	
			//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_SYMC,DISABLE);
	
	if(MH_RET_SHA_SUCCESS != result)
	{
		while(1);
	}
	
	return;
}
#endif // WITH_SHA224

