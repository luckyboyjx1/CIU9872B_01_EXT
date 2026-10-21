/*
 * CreateJavaBuffer.c
 *
 *  Created on: 2018Äê12ÔÂ10ÈÕ
 *      Author: Administrator
 */
#include "ecpDefine.h"
//#include "halCryptoProfile.h"

#if !HAL__PLATFORM__SIMULATOR
#include "global_variable.h"
#endif

#if HAL__PLATFORM__SIMULATOR
	PX8									DtTrRamAlign;
	PX8									g_PBOC_instRAMBuff;
	PX16								bShortBuffer;
	PX8									bRamBuffer;
	PX8									APDUBuffer;
	PX8									javaExceStaticFeild;
#else
	u8		X_A							DtTrRamAlign[DT_RT_RAM_ALIGN_DWORD];
	u8		X_A							g_PBOC_instRAMBuff[RTR_TOP - (u16)(RAMBase) - SHORT_BUFFER_SIZE * 2 - RAM_BUFFER_SIZE - APDU_BUFFER_SIZE - DT_RT_RAM_ALIGN_DWORD];
	u16		X_A							bShortBuffer[SHORT_BUFFER_SIZE];
	u8		X_A							bRamBuffer[RAM_BUFFER_SIZE];
	u8		X_A							APDUBuffer[APDU_BUFFER_SIZE];
	u8		X_A							javaExceStaticFeild[RTR_BASE - RTR_TOP];
#endif

