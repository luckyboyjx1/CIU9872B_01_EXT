/*
 * FreeTableResize.c

 *
 *  Created on: 2016Äê6ÔÂ7ÈÕ
 *      Author: linguoming1
 */

#include "FreeTable.h"
#include "FreeTableResize.h"
#include "MemOperation.h"
#include "NvmCompact.h"

#if BOOT__MEMORY__NVM_RESIZE

u32 FreeTableResize_GetChangeSize(PN8 entityStartAddr, PN8 entityEndAddr, u16 freeTableEntryNum, u1 isExtend)
{
	u16		freeTableIndex;
	PX8		curEntryAddr;
	PN8		curFreeBlockStart;
	u32		curtFreeBlockSize;
	PX8		nextEntryAddr;
	PN8		nextFreeBlockStart;
	u32		nextFreeBlockSize;
	u32		tempSize;
	u32		changeSize;
	u32		preFreeBlockSize;

	changeSize					= 0;
	preFreeBlockSize			= 0;

	for(freeTableIndex = 0; freeTableIndex < freeTableEntryNum; U16_INC(freeTableIndex))
	{
		//reach the last free block, no operation
		if(freeTableIndex == U16_S_U8(freeTableEntryNum, 1))
		{
			break;
		}

		curEntryAddr			= MEMtPX8(FreeTable_GetEntry(freeTableIndex));
		curFreeBlockStart		= MEMtPN8(FreeTable_GetFreeBlockStart(curEntryAddr));
		curtFreeBlockSize		= FreeTable_GetFreeBlockSize(curEntryAddr);

		nextEntryAddr			= MEMtPX8(FreeTable_GetEntry(U32t16(U8_A_U16(1, freeTableIndex))));
		nextFreeBlockStart		= MEMtPN8(FreeTable_GetFreeBlockStart(nextEntryAddr));
		nextFreeBlockSize		= FreeTable_GetFreeBlockSize(nextEntryAddr);

		if(true == isExtend)
		{
			tempSize			= U32_S_U32(curtFreeBlockSize, nextFreeBlockSize);
			if((entityStartAddr < nextFreeBlockStart) &&
				PN8_A_U32(entityEndAddr, curtFreeBlockSize) >= PN8_A_U32(nextFreeBlockStart, tempSize))
			{
				changeSize		= tempSize;
				//clear the extend area data for unit test and broken test.
				memFillNoAtomic(nextFreeBlockStart, 0, curtFreeBlockSize);
				break;
			}
		}
		else
		{
			tempSize			= U32_S_U32(curtFreeBlockSize, preFreeBlockSize);
			if((entityStartAddr < PN8_A_U32(curFreeBlockStart, curtFreeBlockSize)) &&
				PN8_S_U32(entityEndAddr, curtFreeBlockSize)  >= PN8_A_U32(curFreeBlockStart, curtFreeBlockSize))
			{
				changeSize		= tempSize;
				break;
			}
			preFreeBlockSize	= curtFreeBlockSize;
		}
	}

	return changeSize;
}

u32 FreeTableResize_AnalyseTblForExtend(u16 freeTableEntryNum)
{
	u16		freeTableIndex;
	PX8 	curEntryAddr;
	PN8 	freeBlockStart;
	u32		curFreeBlockSize;
	u32		cumulateSize;

	cumulateSize	= 0;

	for(freeTableIndex = freeTableEntryNum - 1; freeTableIndex >= 0; U16_DEC(freeTableIndex))
	{
		curEntryAddr			= MEMtPX8(FreeTable_GetEntry(freeTableIndex));
		curFreeBlockSize		= FreeTable_GetFreeBlockSize(curEntryAddr);
		if(0 == freeTableIndex)
		{
			freeBlockStart		= PN8_A_U32(MEMtPN8(FreeTable_GetFreeBlockStart(curEntryAddr)), curFreeBlockSize);
			freeBlockStart		= PN8_S_U32(freeBlockStart, cumulateSize);
			FreeTable_SetFreeBlockStart(curEntryAddr, PNtMEM(freeBlockStart));
			FreeTable_SetFreeBlockSize(curEntryAddr, 0);
			break;
		}
		cumulateSize			= U32_A_U32(curFreeBlockSize, cumulateSize);
	}

	return cumulateSize;
}


void FreeTableResize_Main(PM8 blockStart, u32 blockSize, u1 isExtend)
{
	FreeTable_BufInit();

	//create freeTable
	if(true == isExtend)
	{
		FreeTable_SetState(FREE_TABLE_GENERATE_EXTEND);
	}
	else
	{
		FreeTable_SetState(FREE_TABLE_GENERATE_SHORTEN);
	}

	FreeTable_Insert(blockStart, blockSize, isExtend);

	RESET_POWER(POSITION_NVM_RESIZE + 1, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	nvmCopyAtomic(MEMtPN8(FreeTable_GetStart()), MEMtPM8(FreeTable_GetPtr()), FREE_TABLE_IN_NVM_SIZE);
	RESET_POWER(POSITION_NVM_RESIZE + 2, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

	FreeTable_RoomArrange(false, true, false);
	RESET_POWER(POSITION_NVM_RESIZE + 7, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	NvmCompact_ResetTables();
}

#endif
