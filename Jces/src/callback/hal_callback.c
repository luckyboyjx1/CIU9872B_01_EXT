/*
 * hal_callback.c
 *
 *  Created on: 2019Äê12ÔÂ16ÈÕ
 *      Author: jiangxiaohui
 */


#include "ecpDefine.h"
#include "ecpReadWrite.h"
#include "SysPlatformConfigProfile.h"

#if !HAL__PLATFORM__SIMULATOR
#include "global_variable.h"
#endif

#if HAL__PLATFORM__SIMULATOR

u8 * PhIso14443L3AtqaB1b8WholeByteL;

__attribute__((weak))  u8  memReadU8(PM8 addr)
{
	return PM_R_U8(addr);
}


__attribute__((weak)) void halInitJavaBuffer(void)
{
//	DtTrRamAlign						= &RAMSTACK[(u16)RAMBase];
//	g_PBOC_instRAMBuff					= &RAMSTACK[DT_RT_RAM_ALIGN_DWORD + (u16)RAMBase];
//	bShortBuffer 						= &RAMSTACK[RTR_TOP - SHORT_BUFFER_SIZE * 2 - RAM_BUFFER_SIZE - APDU_BUFFER_SIZE];
//	bRamBuffer 							= &RAMSTACK[RTR_TOP - RAM_BUFFER_SIZE - APDU_BUFFER_SIZE];
//	APDUBuffer 							= &RAMSTACK[RTR_TOP - APDU_BUFFER_SIZE];
//	javaExceStaticFeild 				= &RAMSTACK[RTR_TOP];
}

__attribute__((weak))  void initHalCallback()
{
	return;
}


#endif
