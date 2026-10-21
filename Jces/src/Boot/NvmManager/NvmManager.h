/*
 * NvmManager.h
 *
 *  Created on: 2014Äê10ÔÂ28ÈÕ
 *      Author: linguoming1
 */

#ifndef __NVM_MANAGER_H_
#define __NVM_MANAGER_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "NvmMap.h"

#define SYSTEMEXCEPT_ILLEGAL_USE					0x9001
#define SYSTEMEXCEPT_NO_RESOURCE            		0x9005

#if BOOT__MEMORY__NVM_MANAGER
	extern u1	  NvmManager_isAlloOrDeallo(void);
	extern void   NvmManager_Boot(u16 appStartIndex, PN8 topNvm);
	extern void   NvmManager_Init(PN8 SysEntryStart, PN8 AppEntryStart, u16 appStartIndex, PN8 topNvm);
	extern void	  NvmManager_Reset(void);
	extern u16    NvmManager_Allocate(u32 size, u8 pckId);
	extern u16 	  NvmManager_AllocateBlock(u32 size);
	extern void   NvmManager_Free(u16 entryIndex, u8 pckId);
	extern u32    NvmManager_GetAvailMemory(void);
	extern memref NvmManager_GetTopNvm(void);
	extern memref NvmManager_GetMapTopAddress(void);
	extern memref NvmManager_GetTopFreeNvm(void);
	extern memref NvmManager_GetAppBitMap(void);
	extern void   NvmManager_FreeBlock(u16 entryIndex);
	extern memref NvmManager_LocateBlock(u16 entryIndex);
	extern void   NvmManager_Deallocate(u1 isRollBack);
	extern void   NvmManager_Commit(void);
	extern	void  NvmManager_FreeObject(u1 isRollBack);

#else
	#define NvmManager_isAlloOrDeallo()												0
	#define NvmManager_Boot(appStartIndex, topNvm)
	#define NvmManager_Init(SysEntryStart, AppEntryStart, appStartIndex, topNvm)
	#define NvmManager_Reset()
	#define NvmManager_Allocate(size, pckId)										0
	#define NvmManager_AllocateBlock(size)											0
	#define NvmManager_Free(entryIndex, pckId)
	#define NvmManager_GetAvailMemory()												0
	#define NvmManager_GetTopNvm()													0
	#define NvmManager_GetMapTopAddress()											0
	#define NvmManager_GetTopFreeNvm()												0
	#define NvmManager_GetAppBitMap()												0
	#define NvmManager_FreeBlock(entryIndex)
	#define NvmManager_LocateBlock(entryIndex)										0
	#define NvmManager_Deallocate(isRollBack)
	#define NvmManager_Commit()
#endif

#endif /* NVMMANAGER_H_ */
