/*
 * NvmManager.c
 *
 *  Created on: 2014年10月28日
 *      Author: linguoming1
 */

#include "NvmManager.h"
#include "NvmMap.h"
#include "FreeTable.h"
#include "NvmDeallocate.h"
#include "global_variable.h"
#include "NvmAllocate.h"
#include "NvmCompact.h"
#include "BootAreaInitRam.h"
#include "NvmResize.h"
//#include "SEEUICCInterface.h"
#include "stack.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if BOOT__MEMORY__NVM_MANAGER

u1	gNvmManager_isAlloOrDeallo;
/*
 * modify by jiangxiaohui@20210429  addReference中分配的引用没有提交，会被重复使用
 */
u1	NvmManager_isAlloOrDeallo()
{
	return	gNvmManager_isAlloOrDeallo;
}

void NvmManager_Boot(u16 appStartIndex, PN8 topNvm)
{
	topNvm	= MEMtPN8(MapTable_GetBase(topNvm));
	topNvm	= MEMtPN8(FreeTable_GetBase(topNvm));
	topNvm	= MEMtPN8(NvmCompact_GetBase(topNvm));
	
	//delete this code after optimize scp
//topNvm	= PN8_S_U32(topNvm, SCP_DATA_LENGTH);
	if(U32_R_U8(PNtMEM(topNvm), 4) != 0)
	{
		topNvm	= PN8_S_U8(topNvm, U32_R_U8(PNtMEM(topNvm), 4));
	}
	
	MapTable_Boot(appStartIndex);
	FreeTable_Boot(topNvm);
	NvmCompact_Boot();
}

void NvmManager_Init(PN8 SysEntryStart, PN8 AppEntryStart, u16 appStartIndex, PN8 topNvm)
{
	topNvm	= MEMtPN8(MapTable_GetBase(topNvm));
	topNvm	= MEMtPN8(FreeTable_GetBase(topNvm));
	topNvm	= MEMtPN8(NvmCompact_GetBase(topNvm));
	
	gNvmManager_isAlloOrDeallo	= false;
	MapTable_Init(SysEntryStart, AppEntryStart, appStartIndex);
	FreeTable_Init();
}

void NvmManager_Reset(void)
{
	MapTable_ResetCache();
	gNvmManager_isAlloOrDeallo	= false;
	FreeTable_Init();
}

u16 NvmManager_Allocate(u32 size, u8 pckId)
{
	gNvmManager_isAlloOrDeallo	= true;
	return NvmAllocate_Main(size, pckId);
}

u16 NvmManager_AllocateBlock(u32 size)
{
	return NvmManager_Allocate(size, MAP_ENTRY_BLOCK_PCK_ID);
}

void NvmManager_Free(u16 entryIndex, u8 pckId)
{
	gNvmManager_isAlloOrDeallo	= true;
	NvmDeallocate_Free(entryIndex, pckId);
}

void NvmManager_FreeBlock(u16 entryIndex)
{
	NvmManager_Free(entryIndex, MAP_ENTRY_BLOCK_PCK_ID);
}

memref NvmManager_LocateBlock(u16 entryIndex)
{
	return MapTable_GetEntityContent(entryIndex);
}

void NvmManager_Deallocate(u1 isRollBack)
{
	NvmDeallocate_Main(isRollBack);
#if (EUICC_ONLY||SE_AND_EUICC)
	ExecuEnvironmentInit(G_CurExecutingProfileIndex);
#endif
}

u32 NvmManager_GetAvailMemory(void)
{
	return FreeTable_GetRemaindNvm();
}

memref NvmManager_GetTopNvm(void)
{
	return NvmCompact_GetFlagTableStart();
}

memref NvmManager_GetMapTopAddress(void)
{
	return MapTable_GetMapTopAddress();
}

memref NvmManager_GetTopFreeNvm(void)
{
	return FreeTable_GetTopFreeNvm();
}

memref NvmManager_GetAppBitMap(void)
{
	return MapTable_GetAppBitMap();
}

void NvmManager_Commit(void)
{
	MapTable_Commit();
	
	if(true == gNvmManager_isAlloOrDeallo)
	{
		FreeTable_ParaCommit();
		gNvmManager_isAlloOrDeallo	= false;
	}
}

void NvmManager_FreeObject(u1 isRollBack)
{
	u1	B_A		isDeletePkg;

	isDeletePkg		= popShort();
	//tranDeallocate_compactRam(COMPACT_SPACE);
	//ramDeallocate_compactRam(COMPACT_SPACE);
	if(FREE_TABLE_NO_OPERATION != FreeTable_RoomArrange(isRollBack, true, isDeletePkg))
	{
		NvmCompact_ResetTables();
	}
	else
	{
		FreeTable_Init();
		FreeTable_Commit();
	}
}


void NvmManager_ResizeObj(u16 entryIndex, u32 reSize)
{
	NvmResize_Main(entryIndex, MEM_NULL , reSize, false);
}

void NvmManager_ResizeBody(u16 entryIndex, PN8 bodyAddr, u32 reSize, u1 isExtend)
{
	if(MEM_NULL != reSize)
	{
		NvmResize_Main(entryIndex, bodyAddr , reSize, isExtend);
	}
}

#endif
