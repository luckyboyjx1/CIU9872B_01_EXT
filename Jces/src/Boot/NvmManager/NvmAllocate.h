/*
 * NvmAllocate.h
 *
 *  Created on: 2014Äê10ÔÂ28ÈÕ
 *      Author: linguoming1
 */

#ifndef __NVM_ALLOCATE_H_
#define __NVM_ALLOCATE_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"

#if BOOT__MEMORY__NVM_MANAGER_NVM_ALLOCATE
	extern u32 NvmAllocate_GetAlignSize(u32 size);
	extern void NvmAllocate_SetEntityHeaderSize(PN8 startAddr, u32 size);
	extern u16  NvmAllocate_Main(u32 size, u8 pckId);
	extern void NvmAllocate_Resize(u16 entryIndex, PN8 bodyAddr, u32 reSize, u1 isExtend);
#else
	#define NvmAllocate_GetAlignSize(size)						0
	#define NvmAllocate_SetEntityHeaderSize(startAddr, size)
	#define NvmAllocate_Main(size, pckId)						0
	#define NvmAllocate_Resize(entryIndex, reSize)
#endif

#endif /* NVMALLOCATE_H_ */
