/*
 * NvmFreeTable.c
 *
 *  Created on: 2014年11月27日
 *      Author: linguoming
 */

#include "FreeTable.h"
#include "MemGetReuseRam.h"
#include "MemOperation.h"
#include "NvmManager.h"
#include "NvmMap.h"
#include "BootErrorHandle.h"
#include "BootRegister.h"
#include "MemBackUp.h"
#include "FreeTableResize.h"
#include "NvmCompact.h"
#include "NvmResize.h"
#include "BootAreaInitRam.h"
#include "BootLifeCycle.h"

#if BOOT__MEMORY__FREE_TABLE

STATIC PX8 			X_A 		gFreeTable_ptr;
STATIC u16			X_A			gFreeTable_maxFreeEntryNum;
STATIC PN8			X_A			gFreeTable_lastFreeBlockTop;
STATIC PN8			X_A			FREE_TABLE_IN_NVM_START;
//u16 gFreeTable_topIndex;

/********************************************************************************************************
 *free table structure
 *_________________________________________________________________________________________________________________________________
 *							|						|															|
 *							|						|															|
 *free table generation flag|free table entry num 	|free table entry start										|last free block
 *							|						|															|(used when table
 *							|						|															| full)
 *__________________________|_______________________|___________________________________________________________|__________________
 *
 *
*********************************************************************************************************/
#define	INIT_FREE_TABLE_ENTRY_NUM					0
#define	INIT_LAST_FREE_BLOCK_START					0xFFFFFFFF
#define	INIT_LAST_FREE_BLOCK_SIZE					0
#define	INIT_FREE_TABLE_VALUE						0xFFFFFFFF

#define	FREE_TABLE_GENERATION_FLAG_LEN				2
#define	FREE_TABLE_ENTRY_NUM_LEN					2
#define FREE_TABLE_MAP_TOP_INDEX_LEN				2
#define	FREE_TABLE_OBJECTID_FOUND_LEN				2
#define FREE_TABLE_HEADER_LEN						(FREE_TABLE_GENERATION_FLAG_LEN + FREE_TABLE_ENTRY_NUM_LEN + FREE_TABLE_MAP_TOP_INDEX_LEN + FREE_TABLE_OBJECTID_FOUND_LEN)

#define	FREE_TABLE_GENERATION_FLAG_OFFSET			0
#define	FREE_TABLE_ENTRY_NUM_OFFSET					(FREE_TABLE_GENERATION_FLAG_OFFSET + FREE_TABLE_GENERATION_FLAG_LEN)
#define	FREE_TABLE_MAP_TOP_INDEX_OFFSET				(FREE_TABLE_ENTRY_NUM_OFFSET	 + FREE_TABLE_ENTRY_NUM_LEN)
#define	FREE_TABLE_OBJECTID_FOUNDED_OFFSET			(FREE_TABLE_MAP_TOP_INDEX_OFFSET + FREE_TABLE_MAP_TOP_INDEX_LEN)
#define	FREE_TABLE_ENTRY_START						(FREE_TABLE_OBJECTID_FOUNDED_OFFSET + FREE_TABLE_OBJECTID_FOUND_LEN)

#define	FREE_TABLE_BLOCK_START_OFFSET				0
#define	FREE_TABLE_BLOCK_SIZE_OFFSET				(FREE_TABLE_BLOCK_START_OFFSET + FREE_TABLE_BLOCK_START_LEN)
void FreeTable_SetState(u16 state)
{
	PX_W_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_GENERATION_FLAG_OFFSET), state);
}

u16 FreeTable_GetState(void)
{
	return PX_R_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_GENERATION_FLAG_OFFSET));
}

u16	FreeTable_GetStateNvm(void)
{
	return PN_R_U16(PN8_A_U16(FREE_TABLE_IN_NVM_START, FREE_TABLE_GENERATION_FLAG_OFFSET));
}

void FreeTable_SetStateNvm(u16 state)
{
	nvmWriteU16Atomic(PN8_A_U16(FREE_TABLE_IN_NVM_START, FREE_TABLE_GENERATION_FLAG_OFFSET), state);
}

void FreeTable_SetEntryNum(u16 size)
{
	PX_W_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_ENTRY_NUM_OFFSET), size);
}

u16 FreeTable_GetEntryNum(void)
{
	return PX_R_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_ENTRY_NUM_OFFSET));
}

u16 FreeTable_GetEntryNumNvm(void)
{
	return PN_R_U16(PN8_A_CONST(FREE_TABLE_IN_NVM_START, FREE_TABLE_ENTRY_NUM_OFFSET));
}

void FreeTable_SetMapTopIndex(u16 index)
{
	PX_W_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_MAP_TOP_INDEX_OFFSET), index);
}

void FreeTable_SetMapTopIndexNvm(u16 index)
{
	nvmWriteU16Atomic(PN8_A_CONST(FREE_TABLE_IN_NVM_START, FREE_TABLE_MAP_TOP_INDEX_OFFSET), index);
}

u16 FreeTable_GetMapTopIndexNvm(void)
{
	return PN_R_U16(PN8_A_CONST(FREE_TABLE_IN_NVM_START, FREE_TABLE_MAP_TOP_INDEX_OFFSET));
}

u16 FreeTable_GetObjectIdFound(void)
{
	return PX_R_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_OBJECTID_FOUNDED_OFFSET));
}

void FreeTable_SetObjectIdFound(u16 index)
{
	PX_W_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_OBJECTID_FOUNDED_OFFSET), index);
}

u16 FreeTable_GetObjectIdFoundNVM()
{
	return PX_R_U16(PX8_A_CONST(FREE_TABLE_IN_NVM_START, FREE_TABLE_OBJECTID_FOUNDED_OFFSET));
}

void FreeTable_SetObjectIdFoundNVM(u16 index)
{
	PX_W_U16(PX8_A_CONST(FREE_TABLE_IN_NVM_START, FREE_TABLE_OBJECTID_FOUNDED_OFFSET), index);
}

memref FreeTable_GetEntry(u16 i)
{
	return PXtMEM(PX8_A_U16(PX8_A_CONST(gFreeTable_ptr, FREE_TABLE_ENTRY_START), U16_M_U16(i, FREE_TABLE_ENTRY_LEN)));
}

memref FreeTable_GetEntryNvm(u16 i)
{
	return PXtMEM(PN8_A_U16(PN8_A_CONST(FREE_TABLE_IN_NVM_START, FREE_TABLE_ENTRY_START), U16_M_U16(i, FREE_TABLE_ENTRY_LEN)));
}

void FreeTable_SetFreeBlockStart(PX8 a, u32 start)
{
	PX_W_U32(PX8_A_CONST(a, FREE_TABLE_BLOCK_START_OFFSET), start);
}

memref FreeTable_GetFreeBlockStart(PM8 a)
{
	return PM_R_U32(PM8_A_CONST(a, FREE_TABLE_BLOCK_START_OFFSET));
}

void FreeTable_SetFreeBlockSize(PX8 a, u32 size)
{
	PX_W_U32(PX8_A_CONST(a, FREE_TABLE_BLOCK_SIZE_OFFSET), size);
}

u32 FreeTable_GetFreeBlockSize(PM8 a)
{
	return PM_R_U32(PM8_A_CONST(a, FREE_TABLE_BLOCK_SIZE_OFFSET));
}

memref FreeTable_GetBase(PN8 topNvm)
{
	FREE_TABLE_IN_NVM_START		= PN8_S_U16(topNvm, FREE_TABLE_IN_NVM_SIZE);
	
	return PNtMEM(FREE_TABLE_IN_NVM_START);
}

memref FreeTable_GetTblAddr(void)
{
	return PNtMEM(FREE_TABLE_IN_NVM_START);
}

u16 FreeTable_GetMaxEntryNum(void)
{
	return gFreeTable_maxFreeEntryNum;
}

memref FreeTable_GetPtr(void)
{
	return PXtMEM(gFreeTable_ptr);
}

memref FreeTable_GetStart(void)
{
	return PXtMEM(FREE_TABLE_IN_NVM_START);
}

u16 FreeTable_GetCurEntryNum(void)
{
	u16		maxFreeEntryNum;

	maxFreeEntryNum		= FreeTable_GetEntryNum();

	if (FREE_TABLE_GENERATE_CVER_FLOW == FreeTable_GetGenFinState(FreeTable_GetState()))
	{
		U16_DEC(maxFreeEntryNum);
	}

	return maxFreeEntryNum;
}

u1 FreeTable_CheckIsCompact(u16 status)
{
	u1 result;

	if((FREE_TABLE_GENERATE_SHORTEN == status) || (FREE_TABLE_GENERATE_EXTEND == status))
	{
		result	= false;
	}
	else
	{
		result	= true;
	}

	return result;
}

void FreeTable_Boot(PN8 topNvm)
{
	u16		clearSize;
	
	clearSize					= U16_S_U16(U16_S_U16(FREE_TABLE_IN_NVM_SIZE, FREE_TABLE_ENTRY_START), FREE_TABLE_ENTRY_LEN);
	gFreeTable_ptr				= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(FREE_TABLE_IN_NVM_SIZE));
	
	FreeTable_SetEntryNum(1);
	ecpMem_Fill(MEMtPX8(FreeTable_GetEntry(1)), INIT_FREE_TABLE_VALUE, clearSize);
	FreeTable_SetState(FREE_TABLE_GENERATE_NO_OPERATE);
	gFreeTable_lastFreeBlockTop	= topNvm;
	
	FreeTable_Commit();
	FreeTable_Init();
}

void FreeTable_Init(void)
{
	PN8	entryAddress;

	if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_INIT)
	{
		return;
	}

	entryAddress				= MEMtPN8(FreeTable_GetEntryNvm(U16_S_U8(FreeTable_GetEntryNumNvm(), 1)));
	gFreeTable_lastFreeBlockTop	= MEMtPN8(PN8_A_U32(FreeTable_GetFreeBlockStart(entryAddress), FreeTable_GetFreeBlockSize(entryAddress)));
	gFreeTable_maxFreeEntryNum	= U16_D_U16(U16_S_U16(FREE_TABLE_IN_NVM_SIZE, FREE_TABLE_ENTRY_START), FREE_TABLE_ENTRY_LEN);
	gFreeTable_ptr				= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(FREE_TABLE_IN_NVM_SIZE));
}

void FreeTable_BufInit(void)
{
	ecpMem_Copy(gFreeTable_ptr, FREE_TABLE_IN_NVM_START, FREE_TABLE_IN_NVM_SIZE);
}

void FreeTable_InitTranCompact(u16 lastTran)
{
	PX8		entryAddress;

	entryAddress		= MEMtPX8(FreeTable_GetEntry(0));
	FreeTable_SetFreeBlockStart(entryAddress, lastTran);
	FreeTable_SetFreeBlockSize(entryAddress, 0);
	FreeTable_SetObjectIdFound(MapTable_GetAppStartIndex());
	FreeTable_SetEntryNum(1);
	FreeTable_SetState(FREE_TABLE_GENERATE_NO_OPERATE);
}

void FreeTable_Commit(void)
{
	memref 	mapTopAddress;
	PX8		entryAddress;
	
	mapTopAddress		= MapTable_GetMapTopAddress();

	FreeTable_SetMapTopIndex(MapTable_GetTopIndex());
	FreeTable_SetObjectIdFound(MapTable_GetAppStartIndex());
	
	//set remaind nvm
	entryAddress		= MEMtPX8(FreeTable_GetEntry(U16_S_U8(FreeTable_GetEntryNum(),1)));
	FreeTable_SetFreeBlockStart(entryAddress, mapTopAddress);
	FreeTable_SetFreeBlockSize(entryAddress, U32_S_U32(PNtMEM(gFreeTable_lastFreeBlockTop), mapTopAddress));
	nvmCopyAtomic(MEMtPN8(FREE_TABLE_IN_NVM_START), gFreeTable_ptr, FREE_TABLE_IN_NVM_SIZE);
}

void FreeTable_ParaCommit(void)
{
	memref 	mapTopAddress;
	u8		entryArray[FREE_TABLE_ENTRY_LEN];
	
	//set remaind nvm
	mapTopAddress		= MapTable_GetMapTopAddress();
	FreeTable_SetFreeBlockStart(MEMtPX8(entryArray), mapTopAddress);
	FreeTable_SetFreeBlockSize(MEMtPX8(entryArray), U32_S_U32(PNtMEM(gFreeTable_lastFreeBlockTop), mapTopAddress));
	nvmCopyAtomic(MEMtPN8(FreeTable_GetEntryNvm(U16_S_U8(FreeTable_GetEntryNumNvm(), 1))), entryArray, FREE_TABLE_ENTRY_LEN);
	
	FreeTable_SetMapTopIndexNvm(MapTable_GetTopIndex());
}

void FreeTable_Reset(void)
{
	u16		freeTableNum;
	PX8		entryAddress;
	memref	freeBlockTop;

	freeTableNum		= FreeTable_GetEntryNum();
	entryAddress		= MEMtPX8(FreeTable_GetEntry(U16_S_U8(freeTableNum, 1)));
	if((FREE_TABLE_GENERATE_EXTEND == FreeTable_GetOpType(FreeTable_GetTableState(true))))
	{
		entryAddress		= MEMtPX8(FreeTable_GetEntry(0));//add
//		freeBlockTop		= U32_S_U32(FreeTable_GetFreeBlockStart(0), FreeTable_GetFreeBlockSize(entryAddress));
		freeBlockTop		= FreeTable_GetFreeBlockStart(entryAddress);
	}
	else
	{
		freeBlockTop		= U32_A_U32(FreeTable_GetFreeBlockStart(entryAddress), FreeTable_GetFreeBlockSize(entryAddress));
	}

	FreeTable_Boot(MEMtPN8(freeBlockTop));
	FreeTable_Commit();
}

boolean FreeTable_IsFull(void)
{
	u1 isFull;

	if(FreeTable_GetEntryNum() == gFreeTable_maxFreeEntryNum)
	{
		isFull	= true;
	}
	else
	{
		isFull	= false;
	}

	return isFull;
}

memref FreeTable_GetTopFreeNvm(void)
{
	return PNtMEM(gFreeTable_lastFreeBlockTop);
}

u32 FreeTable_GetRemaindNvm(void)
{
	return PNtMEM(PN8_S_U32(gFreeTable_lastFreeBlockTop, MapTable_GetMapTopAddress()));
}

memref FreeTable_Alloc(u32 size)
{
	if(size <= FreeTable_GetRemaindNvm())
	{
		gFreeTable_lastFreeBlockTop		= PN8_S_U32(gFreeTable_lastFreeBlockTop, size);
	}
	else
	{
		bootErrorHandle_ThrowError(SYSTEMEXCEPT_NO_RESOURCE);
	}

	return PNtMEM(gFreeTable_lastFreeBlockTop);
}

void FreeTable_DeAlloc(u32 size)
{
	gFreeTable_lastFreeBlockTop		= PN8_A_U32(gFreeTable_lastFreeBlockTop, size);
}

u1 FreeTable_IsFreeBlock(u16 entryIndex, PX8 trObjNode, u1 isNvmFreeTable)
{
	u1 result;
	
	result	= false;
	
	if(MAP_TABLE_ENTRY_STATUS_FREE == MapTable_GetEntryStatus(entryIndex))
	{
		result	= true;
	}
	
	if(true == result)
	{
		if(true != isNvmFreeTable)
		{
		/*@ -type @*/	result	= getTranInfo(entryIndex, MEMtPR8(trObjNode)); /*@ +type @*/
		}
	}
	
	return result;
}

memref FreeTable_GetFreeStart(u16 entryIndex, PX8 trObjNode, u1 isNvmFreeTable)
{
	u32	addrStart;
	
	if(true == isNvmFreeTable)
	{
		addrStart	= MapTable_GetEntryAddr(entryIndex);
	}
	else
	{
		addrStart	= tranDeallocate_getTrObjNodeStart(trObjNode);
	}
	
	return addrStart;
}

u32 FreeTable_GetFreeSize(PN8 blockStart, PX8 trObjNode, u1 isNvmFreeTable)
{
	u32 size;
	
	if(true == isNvmFreeTable)
	{
		size	= MapTable_GetEntitySize(blockStart);
	}
	else
	{
		size	= U16t32(getTranSize(trObjNode));
	}
	
	return size;
}

u16 FreeTable_GetTableState(u1 isNvmFreeTable)
{
	u16	flag;
	
	if(true == isNvmFreeTable)
	{
		flag	= FreeTable_GetStateNvm();
	}
	else
	{
		flag	= FreeTable_GetState();
	}
	
	return flag;
}

void FreeTable_MoveEntity(pNvmCompactInfo nvmComInfo, pFreeInfo freeBlockInfo, u1 isNvmFreeTable)
{
	if(true == isNvmFreeTable)
	{
		NvmCompact_MoveEntity(nvmComInfo, freeBlockInfo);
	}
	else
	{
		tranMoveEntity(freeBlockInfo);
	}
}

void FreeTable_ModifyObj(u1 isNvmFreeTable)
{
	if(true == isNvmFreeTable)
	{
		NvmCompact_UpdateMapTable();
//缺陷：删除包断电问题：在removePackageNM/NvmManager_Deallocate断电时，就会有问题，会把配置区第一页写坏，导致无ATR
//修改数研所测试数字货币删包断电坏卡的问题，by jiangxiaohui 20230609
//		NvmCompact_UpdateMapBitMap();
		if(false == FreeTable_CheckIsCompact(FreeTable_GetOpType(FreeTable_GetState())))
		{
			NvmResize_UpdateEtityHeader();
			NvmResize_UpdateJcvmRelativeInfo();
		}
	}
	else
	{
		trObjModify();
	}
}

void FreeTable_Merge(void)
{
	u1		isMerge;
	u16		i;
	u16		copySize;
	u16		maxFreeEntryNum;
	PX8		curFreeEntry;
	PX8		preFreeEntry;
	PM8		curFreeBlockStart;
	PM8		preFreeBlockStart;
	u32		curFreeBlockSize;
	u32		preFreeBlockSize;

	maxFreeEntryNum				= FreeTable_GetEntryNum();
	i							= U16_S_U8(maxFreeEntryNum, 1);
	isMerge						= false;

	for(; i > 0 ; U16_DEC(i))
	{
		curFreeEntry			= MEMtPX8(FreeTable_GetEntry(i));
		curFreeBlockStart		= MEMtPM8(FreeTable_GetFreeBlockStart(curFreeEntry));
		curFreeBlockSize		= FreeTable_GetFreeBlockSize(curFreeEntry);

		preFreeEntry			= MEMtPX8(FreeTable_GetEntry(U16_S_U8(i, 1)));
		preFreeBlockStart		= MEMtPM8(FreeTable_GetFreeBlockStart(preFreeEntry));
		preFreeBlockSize		= FreeTable_GetFreeBlockSize(preFreeEntry);

		if(U32_A_U32(PMtMEM(curFreeBlockStart), curFreeBlockSize) == PMtMEM(preFreeBlockStart))
		{
			isMerge				= true;
		}
		
		if(U32_A_U32(PMtMEM(preFreeBlockStart), preFreeBlockSize) == PMtMEM(curFreeBlockStart))
		{
			isMerge				= true;
			curFreeBlockStart	= preFreeBlockStart;
		}
		
		if(true == isMerge)
		{
			curFreeBlockSize	= U32_A_U32(curFreeBlockSize, preFreeBlockSize);
			FreeTable_SetFreeBlockStart(curFreeEntry, PMtMEM(curFreeBlockStart));
			FreeTable_SetFreeBlockSize(curFreeEntry, curFreeBlockSize);
			copySize			= U16_S_U8(maxFreeEntryNum, i);
			copySize			= U32t16(U16_M_U16(copySize, FREE_TABLE_ENTRY_LEN));
			ecpMem_Copy(preFreeEntry, curFreeEntry, copySize);
			U16_DEC(maxFreeEntryNum);
			isMerge				= false;
		}
	}
	
	FreeTable_SetEntryNum(maxFreeEntryNum);
}

u1 FreeTable_Insert(PM8 blockStart, u32 blockSize, u1 isIncrease)
{
	u1		isfound;
	u32		moveSize;
	u16		entryIndex;
	u16		freeTableNum;
	PM8		curBlockStart;
	u32		curBlockSize;
	PX8		entryAddr;

	isfound			= true;
	freeTableNum	= FreeTable_GetEntryNum();

	curBlockStart   = 0;
	curBlockSize    = 0;
	entryAddr       = 0;	
	
	for(entryIndex = 0; entryIndex <= freeTableNum; U16_INC(entryIndex))
	{
		entryAddr		= MEMtPX8(FreeTable_GetEntry(entryIndex));
		if(freeTableNum == gFreeTable_maxFreeEntryNum)
		{
			isfound			= false;
			break;
		}
		curBlockStart	= MEMtPM8(FreeTable_GetFreeBlockStart(entryAddr));
		curBlockSize	= FreeTable_GetFreeBlockSize(entryAddr);

		if(blockStart < curBlockStart)
		{
			//merge the new free block and a old free block
			if(curBlockStart == PM8_A_U32(blockStart, blockSize))
			{
				curBlockSize	= U32_A_U32(curBlockSize, blockSize);
				curBlockStart	= blockStart;
				break;
			}
			else if(curBlockStart < PM8_A_U32(blockStart, blockSize))
			{
				while(1);
			}
		}
		else
		{
			//merge both the new free block and the old free block
			if(blockStart == PM8_A_U32(curBlockStart, curBlockSize))
			{
				curBlockSize	= U32_A_U32(curBlockSize, blockSize);
				break;
			}
			else if(blockStart < PM8_A_U32(curBlockStart, curBlockSize))
			{
				while(1);
			}
		}

		//insert a new block
		if(((false == isIncrease) && (blockStart > curBlockStart))
		  || ((true == isIncrease) && (blockStart < curBlockStart)))
		{
			moveSize		= U8_M_U16(FREE_TABLE_ENTRY_LEN, U16_S_U16(freeTableNum, entryIndex));
			ecpMem_Move(MEMtPX8(FreeTable_GetEntry(U8_A_U8(entryIndex, 1))), MEMtPX8(FreeTable_GetEntry(entryIndex)), moveSize);
			curBlockStart			= blockStart;
			curBlockSize			= blockSize;
			U16_INC(freeTableNum);
			FreeTable_SetEntryNum(freeTableNum);
			break;
		}
	}
	
	if(true == isfound)
	{
		FreeTable_SetFreeBlockStart(entryAddr, PNtMEM(curBlockStart));
		FreeTable_SetFreeBlockSize(entryAddr, curBlockSize);
	}

	return isfound;
}

u32 FreeTable_Compare(PM8 entityStart, u16 maxFreeEntryNum, u1 isIncrease)
{
	u16		i;
	PX8		curFreeEntry;
	PX8		nextFreeEntry;
	PM8		curFreeBlockStart;
	PM8		nextFreeBlockStart;
	u32		curFreeBlockSize;
	u32		nextFreeBlockSize;

	curFreeBlockSize	= 0;
	nextFreeBlockSize	= 0;
	U16_DEC(maxFreeEntryNum);


	for(i = 0; i < maxFreeEntryNum; U16_INC(i))
	{
		curFreeEntry			= MEMtPX8(FreeTable_GetEntry(i));
		curFreeBlockStart		= MEMtPN8(FreeTable_GetFreeBlockStart(curFreeEntry));

		nextFreeEntry			= MEMtPX8(FreeTable_GetEntry(U32t16(U16_A_U8(i, 1))));
		nextFreeBlockStart		= MEMtPN8(FreeTable_GetFreeBlockStart(nextFreeEntry));
		//when extend room is very big need adding nextFreeBlockSize
		if(false == FreeTable_CheckIsCompact(FreeTable_GetOpType(FreeTable_GetState())))
		{
			nextFreeBlockSize		= FreeTable_GetFreeBlockSize(curFreeEntry);
		}

		if(((false == isIncrease) && (entityStart < curFreeBlockStart) && (entityStart >= nextFreeBlockStart))
			||((true == isIncrease) && (entityStart > curFreeBlockStart) && (entityStart < PM8_A_U32(nextFreeBlockStart, nextFreeBlockSize))))
		{
			curFreeBlockSize	= FreeTable_GetFreeBlockSize(curFreeEntry);
			break;
		}
	}

	return curFreeBlockSize;
}
//缺陷：删除包断电问题：在removePackageNM/NvmManager_Deallocate断电时，就会有问题，会把配置区第一页写坏，导致无ATR
//修改数研所测试数字货币删包断电坏卡的问题，by jiangxiaohui 20230609
boolean FreeTable_Generate(u1 isRollBack, u1 isNvmFreeTable)
{
	u1		isTableOverFlow;
	u1	 	isCompact;
	u8		trObjNode[FREE_TABLE_ENTRY_LEN];
	u16		entryIndex;
	u16 	maxTableEntryIndex;
	u16		minTableEntryIndex;
	u16		freeTableStateFlag;
	PX8		firstEntryAddr;
	PX8		lastBlockEntryAddr;
	PM8		blockStart;
	u1 		isfound;
	u1 		isTransaction;

	//gFreeTable_topIndex = 0;
	freeTableStateFlag		= FreeTable_GetTableState(isNvmFreeTable);
	isCompact = false;

	if (true == isRollBack)
	{
		if (FREE_TABLE_GENERATE_NO_OPERATE == freeTableStateFlag)
		{
			isCompact			= false;
		}
		else if (true == isNvmFreeTable)
		{
			// when rolling back, copy from nvm freeTable with interrupted information to the freetable in ram.
			FreeTable_BufInit();
			isCompact			= true;
		}
	}
	else
	{
		if((true == isNvmFreeTable) && (FREE_TABLE_GENERATE_CVER_FLOW != FreeTable_GetGenFinState(freeTableStateFlag)))
		{
			FreeTable_BufInit();
		}
		
		if(true == FreeTable_CheckIsCompact(FreeTable_GetOpType(freeTableStateFlag)))
		{
			minTableEntryIndex				= FreeTable_GetObjectIdFound();
			maxTableEntryIndex				= MapTable_GetTopIndex();

			if(FREE_TABLE_GENERATE_CVER_FLOW == FreeTable_GetGenFinState(freeTableStateFlag))
			{
				//begin next table generation cycle because free table is not enough on the previous generation cycle.
				firstEntryAddr				= MEMtPX8(FreeTable_GetEntry(0));
				lastBlockEntryAddr			= MEMtPX8(FreeTable_GetEntry(U16_S_U16(gFreeTable_maxFreeEntryNum, 2)));
				FreeTable_SetEntryNum(2);
				ecpMem_Copy(firstEntryAddr, lastBlockEntryAddr, U16_M_U16(2, FREE_TABLE_ENTRY_LEN));
			}

			do
			{
				isTableOverFlow				= false;

				//insert a new entry and merge the free table.
				for(entryIndex = minTableEntryIndex; entryIndex <= maxTableEntryIndex; U16_INC(entryIndex))
				{
					if(FreeTable_IsFreeBlock(entryIndex, trObjNode, isNvmFreeTable))
					{
						blockStart			= MEMtPM8(FreeTable_GetFreeStart(entryIndex, trObjNode, isNvmFreeTable));
						if (true == isNvmFreeTable)
						{
							isfound = FreeTable_Insert(blockStart, FreeTable_GetFreeSize(blockStart, trObjNode, isNvmFreeTable), false);
						}
						else
						{
							isfound = FreeTable_Insert(blockStart, FreeTable_GetFreeSize(blockStart, trObjNode, isNvmFreeTable), true);
						}


						if (isfound == true)
						{
							FreeTable_SetObjectIdFound(entryIndex);
						}
						else
						{
							break;
						}
					}
				}

				minTableEntryIndex = U32t16(U16_A_U8(entryIndex, 1));

				if(gFreeTable_maxFreeEntryNum == FreeTable_GetEntryNum())
				{
					isTableOverFlow			= true;
				}
				FreeTable_Merge();
			}while((true == isTableOverFlow) && (gFreeTable_maxFreeEntryNum != FreeTable_GetEntryNum()));

			if(gFreeTable_maxFreeEntryNum == FreeTable_GetEntryNum())
			{
				FreeTable_SetState(FREE_TABLE_GENERATE_CVER_FLOW);
			}
			else
			{
				FreeTable_SetState(FREE_TABLE_GENERATE_FINISH);
			}

			if(true == isNvmFreeTable)
			{
				RESET_POWER(POSITION_FREETABLE_GENERATE, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

				if (false == isInProgress())
				{
					memBeginTransaction(SINGLE_APDU_PROCESS);
					isTransaction = true;
				}
				else
				{
					isTransaction = false;
				}
	
				nvmCopyAtomic(MEMtPN8(FREE_TABLE_IN_NVM_START), gFreeTable_ptr, FREE_TABLE_IN_NVM_SIZE);
				NvmCompact_UpdateMapBitMap();
				if(isTransaction == true)
				{
					memCommitTransaction();
				}
				RESET_POWER(POSITION_FREETABLE_GENERATE + 1, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
			}
		}

		isCompact				= true;
	}

	return isCompact;
}
//缺陷：删除包断电问题：在removePackageNM/NvmManager_Deallocate断电时，就会有问题，会把配置区第一页写坏，导致无ATR
//修改数研所测试数字货币删包断电坏卡的问题，by jiangxiaohui 20230609
u32 FreeTable_RoomArrange(u1 isRollBack, u1 isNvmFreeTable,u1 isDeletePkg)
{
	u16				freeTableGenerateResult;
	u16				freeTableIndex;
	u16				freeTableEntryNum;
	PX8				curEntryAddr;
	PX8				nextEntryAddr;
	freeInfo		freeBlockInfo;
	nvmCompactInfo	nvmComInfo;
	u1              compactflag;
	/*@ -typedefparam @*/
	/*@ -type @*/
	compactflag = false;
	if(true == isNvmFreeTable)
	{
		NvmCompact_GetBrokenMoveFlag(&nvmComInfo);
	}

	RESET_POWER(POSITION_COMPACT, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

	//begin compact free block
	do
	{
		freeBlockInfo.cumulateSize			= FREE_TABLE_NO_OPERATION;
		freeBlockInfo.isLastFreeBlock		= false;

		//check a compact is in process or not, if not no operation.
		if(false == FreeTable_Generate(isRollBack, isNvmFreeTable))
		{
			break;
		}

		RESET_POWER(POSITION_COMPACT + 1, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

		freeTableEntryNum						= FreeTable_GetEntryNum();
		freeTableGenerateResult					= FreeTable_GetTableState(isNvmFreeTable);
		RESET_POWER(POSITION_NVM_RESIZE + 6, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
		if(FREE_TABLE_GENERATE_CVER_FLOW == FreeTable_GetGenFinState(freeTableGenerateResult))
		{
			U16_DEC(freeTableEntryNum);
			compactflag = true;
		}
		if((isDeletePkg == false)||(compactflag == true))
		{
			curEntryAddr							= MEMtPX8(FreeTable_GetEntry(0));
			if(FREE_TABLE_GENERATE_EXTEND == FreeTable_GetOpType(freeTableGenerateResult))
			{
				freeBlockInfo.cumulateSize			= FreeTableResize_AnalyseTblForExtend(freeTableEntryNum);
				NvmCompact_SetOverLapBound(MEMtPN8(PN8_A_U32(FreeTable_GetFreeBlockStart(curEntryAddr), freeBlockInfo.cumulateSize)));
				freeBlockInfo.isExtend				= true;
			}
			else
			{
				freeBlockInfo.cumulateSize			= 0;
				NvmCompact_SetOverLapBound(MEMtPN8(FreeTable_GetFreeBlockStart(curEntryAddr)));
				freeBlockInfo.isExtend				= false;
			}

			RESET_POWER(POSITION_COMPACT + 2, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
			RESET_POWER(POSITION_NVM_RESIZE + 5, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

			for(freeTableIndex = 0; freeTableIndex < freeTableEntryNum; U16_INC(freeTableIndex))
			{
				RESET_POWER(POSITION_COMPACT + 3, freeTableIndex, freeTableEntryNum);

				curEntryAddr						= MEMtPX8(FreeTable_GetEntry(freeTableIndex));
				freeBlockInfo.curFreeBlockStart		= MEMtPM8(FreeTable_GetFreeBlockStart(curEntryAddr));
				freeBlockInfo.curFreeBlockSize		= FreeTable_GetFreeBlockSize(curEntryAddr);
				if(false == freeBlockInfo.isExtend)
				{
					freeBlockInfo.cumulateSize		= U32_A_U32(freeBlockInfo.curFreeBlockSize, freeBlockInfo.cumulateSize);
				}
				else
				{
					freeBlockInfo.cumulateSize		= U32_S_U32(freeBlockInfo.cumulateSize, freeBlockInfo.curFreeBlockSize);
				}

				//cumulate the free block size for update map
				FreeTable_SetFreeBlockSize(curEntryAddr, freeBlockInfo.cumulateSize);

				//reach the last free block, no operation
				if(freeTableIndex == U16_S_U8(freeTableEntryNum, 1))
				{
					break;
				}
				if(freeTableIndex == U16_S_U8(freeTableEntryNum, 2))
				{
					freeBlockInfo.isLastFreeBlock		= true;
				}

				nextEntryAddr							= MEMtPX8(FreeTable_GetEntry(U8_A_U16(1, freeTableIndex)));
				freeBlockInfo.nextFreeBlockStart		= MEMtPM8(FreeTable_GetFreeBlockStart(nextEntryAddr));
				freeBlockInfo.nextFreeBlockSize			= FreeTable_GetFreeBlockSize(nextEntryAddr);

				//move next entity
				FreeTable_MoveEntity(&nvmComInfo, &freeBlockInfo, isNvmFreeTable);
				RESET_POWER(POSITION_COMPACT + 6, freeTableIndex, freeTableEntryNum);
				RESET_POWER(POSITION_NVM_RESIZE + 4, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
			}
		}


		if(true == isInProgress())
		{
			memCommitTransaction();
		}
		if((isDeletePkg == false)||(compactflag == true))
		{
			FreeTable_ModifyObj(isNvmFreeTable);
		}
//		else
//		{
//			NvmCompact_UpdateMapBitMap();
//		}
		isRollBack				= false;

		RESET_POWER(POSITION_COMPACT + 9, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
		RESET_POWER(POSITION_NVM_RESIZE + 3, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

	}while(FREE_TABLE_GENERATE_CVER_FLOW == FreeTable_GetGenFinState(freeTableGenerateResult));
	/*@ +type @*/
	/*@ +typedefparam @*/
	return freeBlockInfo.cumulateSize;
}

#endif
