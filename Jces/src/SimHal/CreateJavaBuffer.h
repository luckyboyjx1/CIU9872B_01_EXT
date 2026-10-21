/*
 * CreateJavaBuffer.h
 *
 *  Created on: 2018Äê12ÔÂ10ÈÕ
 *      Author: Administrator
 */

#ifndef SIMHAL_CREATEJAVABUFFER_H_
#define SIMHAL_CREATEJAVABUFFER_H_

#include "ecpDefine.h"
#include "SysPlatformConfigProfile.h"
//#include "halCryptoProfile.h"

#if !HAL__PLATFORM__SIMULATOR
#include "global_variable.h"
#endif


#define	DT_RT_RAM_ALIGN_DWORD		0x1C

#if HAL__PLATFORM__SIMULATOR
	extern PX8							DtTrRamAlign;
	extern PX8							g_PBOC_instRAMBuff;
	extern PX16							bShortBuffer;
	extern PX8							bRamBuffer;
	extern PX8							APDUBuffer;
	extern PX8							javaExceStaticFeild;
#else
	extern u8		X_A					DtTrRamAlign[/*DT_RT_RAM_ALIGN_DWORD*/];
	extern u8		X_A					g_PBOC_instRAMBuff[/*RTR_TOP - (u16)(RAMBase) - SHORT_BUFFER_SIZE * 2 - RAM_BUFFER_SIZE - APDU_BUFFER_SIZE - DT_RT_RAM_ALIGN_DWORD*/];
	extern u16		X_A					bShortBuffer[/*SHORT_BUFFER_SIZE*/];
	extern u8		X_A					bRamBuffer[/*RAM_BUFFER_SIZE*/];
	extern u8		X_A					APDUBuffer[/*APDU_BUFFER_SIZE*/];
	extern u8		X_A					javaExceStaticFeild[/*RTR_BASE - RTR_TOP*/];
#endif


#endif /* SIMHAL_CREATEJAVABUFFER_H_ */
