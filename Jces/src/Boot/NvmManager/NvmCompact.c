#include "NvmCompact.h"
#include "MemBackUp.h"
#include "MemOperation.h"
#include "BootErrorHandle.h"
#include "SysConfigTarget.h"
#include "NvmManager.h"
#include "NvmMap.h"
#include "global_variable.h"
#include "NvmAllocate.h"
#include "BootRegister.h"
#include "memBackUpServe.h"

#if BOOT__MEMORY__NVM_MANAGER_NVM_COMPACT

//STATIC	u1			X_A					gNvmCompact_isExtend;
STATIC	PN8			X_A					gNvmCompact_overLapBound;
STATIC	PN8			X_A					MOVE_FLAG_TABLE_IN_NVM_START;

#define	MAP_FLAG_TABLE_IN_NVM_START		(MOVE_FLAG_TABLE_IN_NVM_START + MOVE_FLAG_TABLE_IN_NVM_SIZE)

/********************************************************************************************************
 *
 *_______________nextEntityStart____________________________________________preEntityStart________________
 *						|								|						|
 *						|								|						|
 *	next free block		|	next entity block			|	free block			|	previous entity block
 *						|								|						|
 *______________________|_______________________________|_______________________|_________________________
 *												curFreeBlockStart
 *
*********************************************************************************************************/
#define	MOVE_FLAG_SRC_NULL								0xFFFFFFFF
#define	MAP_FLAG_REF_NULL								0xFFFF

#define	MOVE_FLAG_SRC_SIZE								4
#define	MOVE_FLAG_DES_SIZE								4
#define	MOVE_FLAG_ENTRY_SIZE							(MOVE_FLAG_SRC_SIZE + MOVE_FLAG_DES_SIZE)
#define MAP_FLAG_ENTRY_SIZE								2

#define	SET_MOVE_FLAG_ENTRY								0
#define	GET_MOVE_FLAG_ENTRY								1

#define	NvmCompact_GetMoveFlagEntry(i)					PN8_A_U16(MOVE_FLAG_TABLE_IN_NVM_START, U16_M_U16(i, MOVE_FLAG_ENTRY_SIZE))
#define	NvmCompact_GetMoveFlagSrc(a)					PN_R_U32(a)
#define	NvmCompact_SetMoveFlagSrc(a, v)					nvmWriteU32Atomic(MEMtPN8(a), PNtMEM(v))
#define	NvmCompact_GetMoveFlagDes(a)					PN_R_U32(PN8_A_U8(a, MOVE_FLAG_SRC_SIZE))
#define	NvmCompact_SetMoveFlagDes(a, v)					nvmWriteU32Atomic(MEMtPN8(PN8_A_U8(a, MOVE_FLAG_SRC_SIZE)), PNtMEM(v))

#define	NvmCompact_GetMapFlagEntry(i)					PN8_A_U16(MAP_FLAG_TABLE_IN_NVM_START, U16_M_U16(i, MAP_FLAG_ENTRY_SIZE))
#define	NvmCompact_GetMapFlagIndex(a)					PN_R_U16(a)
#define	NvmCompact_SetMapFlagIndex(a, i)				nvmWriteU16Atomic(a, i)

memref NvmCompact_GetBase(PN8 topNvm)
{
	MOVE_FLAG_TABLE_IN_NVM_START		= PN8_S_U16(topNvm, DEALLO_FLAG_TABLE_IN_NVM_SIZE);
	
	return PNtMEM(MOVE_FLAG_TABLE_IN_NVM_START);
}

memref NvmCompact_GetFlagTableStart(void)
{
	return PNtMEM(MOVE_FLAG_TABLE_IN_NVM_START);
}

void NvmCompact_Boot(void)
{
	nvmFillNoAtomic(MOVE_FLAG_TABLE_IN_NVM_START, 0xFF, DEALLO_FLAG_TABLE_IN_NVM_SIZE);
}

void NvmCompact_InitMoveFlagTable(void)
{
	gNvmCompact_overLapBound	= 0;
}

void NvmCompact_SetLastMoveFlag(PN8 src, PN8 des)
{
	u16 	i;
	u16 	tableMaxEntry;
	PN8		tempAddr;

	tableMaxEntry		= U16_D_U16(MOVE_FLAG_TABLE_IN_NVM_SIZE, MOVE_FLAG_ENTRY_SIZE);

	for(i = 0; i < tableMaxEntry; U16_INC(i))
	{
		tempAddr		= NvmCompact_GetMoveFlagEntry(i);
		if(MOVE_FLAG_SRC_NULL == NvmCompact_GetMoveFlagSrc(tempAddr))
		{
			NvmCompact_SetMoveFlagSrc(tempAddr, src);
			NvmCompact_SetMoveFlagDes(tempAddr, des);
			break;
		}
	}

	if(i == tableMaxEntry)
	{
		nvmFillAtomic(MEMtPN8(MOVE_FLAG_TABLE_IN_NVM_START), U32t8(MOVE_FLAG_SRC_NULL), MOVE_FLAG_TABLE_IN_NVM_SIZE);
		NvmCompact_SetMoveFlagSrc(MOVE_FLAG_TABLE_IN_NVM_START, src);
		NvmCompact_SetMoveFlagDes(MOVE_FLAG_TABLE_IN_NVM_START, des);
	}
}

u16 NvmCompact_GetLastMoveFlag(void)
{
	u16 	i;
	u16 	tableMaxEntry;
	PN8		tempAddr;

	tableMaxEntry		= U16_D_U16(MOVE_FLAG_TABLE_IN_NVM_SIZE, MOVE_FLAG_ENTRY_SIZE);

	for(i = 0; i < tableMaxEntry; U16_INC(i))
	{
		tempAddr		= NvmCompact_GetMoveFlagEntry(i);
		if(MOVE_FLAG_SRC_NULL == NvmCompact_GetMoveFlagSrc(tempAddr))
		{
			break;
		}
	}

	if(0 != i)
	{
		U16_DEC(i);
	}

	return i;
}

void NvmCompact_ClearMoveFlagTable(void)
{
	nvmFillAtomic(MEMtPN8(MOVE_FLAG_TABLE_IN_NVM_START), U32t8(MOVE_FLAG_SRC_NULL), MOVE_FLAG_TABLE_IN_NVM_SIZE);
}

void NvmCompact_GetBrokenMoveFlag(pNvmCompactInfo info)
{
	PN8		brokenEntryAddr;
	
	//check copy is broken or not in last entity move
	brokenEntryAddr			= MEMtPN8(NvmCompact_GetMoveFlagEntry(NvmCompact_GetLastMoveFlag()));
	info->brokenCopySrc		= MEMtPN8(NvmCompact_GetMoveFlagSrc(brokenEntryAddr));
	info->brokenCopyDes		= MEMtPN8(NvmCompact_GetMoveFlagDes(brokenEntryAddr));
	if(MEMtPN8(MOVE_FLAG_SRC_NULL) != info->brokenCopySrc)
	{
		info->isCopyBroken	= true;
	}
	else
	{
		info->isCopyBroken	= false;
	}
}

void NvmCompact_SetLastUpdateMapIndex(u16 mapIndex)
{
	u16 	i;
	u16 	tableMaxEntry;
	PN8		tempAddr;

	tableMaxEntry		= U16_D_U16(MAP_FLAG_TABLE_IN_NVM_SIZE, MAP_FLAG_ENTRY_SIZE);

	for(i = 0; i < tableMaxEntry; U16_INC(i))
	{
		tempAddr		= NvmCompact_GetMapFlagEntry(i);
		if(MAP_FLAG_REF_NULL == NvmCompact_GetMapFlagIndex(tempAddr))
		{
			NvmCompact_SetMapFlagIndex(MEMtPN8(tempAddr), mapIndex);
			break;
		}
	}

	if(i == tableMaxEntry)
	{
		nvmFillAtomic(MEMtPN8(MAP_FLAG_TABLE_IN_NVM_START), U32t8(MAP_FLAG_REF_NULL), MOVE_FLAG_TABLE_IN_NVM_SIZE);
		NvmCompact_SetMapFlagIndex(MEMtPN8(MAP_FLAG_TABLE_IN_NVM_START), mapIndex);
	}
}

void NvmCompact_ClearMapFlagTable(void)
{
	nvmFillAtomic(MEMtPN8(MAP_FLAG_TABLE_IN_NVM_START), U32t8(MAP_FLAG_REF_NULL), MOVE_FLAG_TABLE_IN_NVM_SIZE);
}

u16 NvmCompact_GetLastUpdateMapIndex(void)
{
	u16 	i;
	u16		index;
	u16 	tableMaxEntry;
	PN8		tempAddr;

	tableMaxEntry		= U16_D_U16(MAP_FLAG_TABLE_IN_NVM_SIZE, MAP_FLAG_ENTRY_SIZE);

	for(i = 0; i < tableMaxEntry; U16_INC(i))
	{
		tempAddr		= NvmCompact_GetMapFlagEntry(i);
		if(MAP_FLAG_REF_NULL == NvmCompact_GetMapFlagIndex(tempAddr))
		{
			break;
		}
	}

	if(0 == i)
	{
		index			= MapTable_GetAppStartIndex();
	}
	else
	{
		U16_DEC(i);
		tempAddr		= NvmCompact_GetMapFlagEntry(i);
		index			= NvmCompact_GetMapFlagIndex(tempAddr); //TODO mwp: fix bug
		U16_INC(index);
	}

	return index;
}

void NvmCompact_SetOverLapBound(PN8 bound)
{
	gNvmCompact_overLapBound		= bound;
}

void NvmCompact_ResetTables(void)
{
	memBeginTransaction(SINGLE_APDU_PROCESS);
	NvmCompact_ClearMoveFlagTable();
	RESET_POWER(POSITION_RESET_TABLE, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	NvmCompact_ClearMapFlagTable();
	RESET_POWER(POSITION_RESET_TABLE + 1, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	MapTable_Reset();
	FreeTable_Reset();
	memCommitTransaction();
}

void NvmCompact_CheckBackUpCapacity(void)
{
	if(get_max_commit_capacity() < U32t16(U16_M_U16(3, EEPAGE_SIZE)))
	{
		while(1){};
	}
}

void NvmCompact_IsCommitLastMove(PN8 copySrc, PN8 copyDes, u32 nextEntitySize)
{
	if(true == isInProgress())
	{
		if(nextEntitySize > EEPAGE_SIZE)
		{
			NvmCompact_SetLastMoveFlag(copySrc, copyDes);
			memCommitTransaction();
			gNvmCompact_overLapBound			= copySrc;
		}
	}
}

u1 NvmCompact_IsOpenTransaction(PN8 copyDes, u1 isExtend)
{
	u1 isOpenTransaction;
	
	isOpenTransaction	= false;
	
	if(false == isInProgress())
	{
		//free block is less than one page, open transaction.
		if(((false == isExtend) && PN8_S_U32(copyDes, gNvmCompact_overLapBound) < MEMtPN8(EEPAGE_SIZE))
		  || ((true == isExtend) && PN8_S_U32(gNvmCompact_overLapBound, copyDes) < MEMtPN8(EEPAGE_SIZE)))
		{
			isOpenTransaction		= true;
		}
		else
		{
			isOpenTransaction		= false;
		}
	}
	
	return isOpenTransaction;
}

void NvmCompact_UpdateMapTable(void)
{
	u1		checkIsCompact;
	u1		isExtend;
	u8		entryStatus;
	u16		i;
	u16     entry_modified;
	u16		maxFreeEntryNum;
	u16		tranPageCapacity;
	u32		freeBlockSize;
	u16		maxMapEntry;
	PN8		entityStart;

	isExtend			= false;
	i					= NvmCompact_GetLastUpdateMapIndex();
	entry_modified		= i;
	maxFreeEntryNum		= FreeTable_GetCurEntryNum();
	maxMapEntry			= MapTable_GetTopIndex();
	tranPageCapacity	= get_unused_commit_capacity();
	checkIsCompact		= FreeTable_CheckIsCompact(FreeTable_GetOpType(FreeTable_GetState()));

	if(FREE_TABLE_GENERATE_EXTEND == FreeTable_GetOpType(FreeTable_GetState()))
	{
		isExtend		= true;
	}

	if(MIN_FREE_TABLE_NUM != maxFreeEntryNum)
	{
		for(; i <= maxMapEntry; U16_INC(i))
		{
			if(false == isInProgress())
			{
				memBeginTransaction(SINGLE_APDU_PROCESS);
			}

			RESET_POWER(POSITION_MODIFY, NvmCompact_GetLastUpdateMapIndex(), maxMapEntry);

			entryStatus		= MapTable_GetEntryStatus(i);

			if(MAP_TABLE_ENTRY_STATUS_AVAIL != entryStatus)
			{
				if((MAP_TABLE_ENTRY_STATUS_FREE == entryStatus) || (MAP_TABLE_ENTRY_STATUS_CREATION == entryStatus)	|| (false == checkIsCompact))
				{
					entityStart			= MEMtPN8(MapTable_GetEntryAddr(i));

					freeBlockSize		= FreeTable_Compare(entityStart, maxFreeEntryNum, isExtend);

					if(0 != freeBlockSize)
					{
						if(false == isExtend)
						{
							entityStart		= PN8_A_U32(entityStart, freeBlockSize);
						}
						else
						{
							entityStart		= PN8_S_U32(entityStart, freeBlockSize);
						}
						MapTable_SetEntryAddr(i, entityStart);
						entry_modified  = i;

						RESET_POWER(POSITION_MODIFY + 3, NvmCompact_GetLastUpdateMapIndex(), maxMapEntry);

						tranPageCapacity		= get_unused_commit_capacity();
					}
				}
			}
			if(tranPageCapacity <= U32t16(U16_M_U16(2, EEPAGE_SIZE)) || (i == maxMapEntry))
			{
				NvmCompact_SetLastUpdateMapIndex(entry_modified);

				RESET_POWER(POSITION_MODIFY + 6, NvmCompact_GetLastUpdateMapIndex(), maxMapEntry);

				memCommitTransaction();
				tranPageCapacity		= get_unused_commit_capacity();
			}

		}

	}
	RESET_POWER(POSITION_MODIFY + 9, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
}

void NvmCompact_UpdateMapBitMap(void)
{
	u16 i;
	u16	maxIndex;
	u16  freeTableState;
	u1 isTransaction;
	
	freeTableState	= FreeTable_GetState();

	if(true == FreeTable_CheckIsCompact(FreeTable_GetOpType(freeTableState)))
	{
		i			= MapTable_GetAppStartIndex();
		maxIndex	= FreeTable_GetObjectIdFound();

		if (false == isInProgress())
		{
			memBeginTransaction(SINGLE_APDU_PROCESS);
			isTransaction = true;
		}
		else
		{
			isTransaction = false;
		}
	
		RESET_POWER(POSITION_MODIFY + 10, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

		for(; i <= maxIndex; U16_INC(i))
		{
			if(MAP_TABLE_ENTRY_STATUS_FREE == MapTable_GetEntryStatus(i))
			{
				MapTable_SetEntryStatus(i, MAP_TABLE_ENTRY_STATUS_AVAIL);
			}

			RESET_POWER(POSITION_MODIFY + 11, MapTable_GetAppStartIndex(), maxIndex);
		}

		RESET_POWER(POSITION_MODIFY + 14, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
		if(isTransaction == true)
		{
			memCommitTransaction();
		}
	}
}

void NvmCompact_MoveInTran(PN8 copyDes, PN8 copySrc, u16 copySize)
{
	u16		tranPageCapacity;

	if(false == isInProgress())
	{
		memBeginTransaction(SINGLE_APDU_PROCESS);
	}

	nvmCopyAtomic(MEMtPN8(copyDes), MEMtPN8(copySrc), copySize);

	RESET_POWER(POSITION_MOVE_ENTITRY + 1, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

	tranPageCapacity		= get_unused_commit_capacity();
	if(tranPageCapacity <= ((2 * EEPAGE_SIZE) + (EEPAGE_SIZE / 2)))
	{
		// todo make test instance
		RESET_POWER(POSITION_MOVE_ENTITRY + 2, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

		NvmCompact_SetLastMoveFlag(copySrc, copyDes);
		memCommitTransaction();
		gNvmCompact_overLapBound	= copySrc;

		RESET_POWER(POSITION_MOVE_ENTITRY + 3, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	}
}

void NvmCompact_MoveInNoTran(PN8 preEntityStart, PN8 preCopyDes, PN8 preCopySrc, PN8 copyDes, PN8 copySrc, u16 copySize, u1 isExtend)
{
	u1 isWriteNoAtomic;

	isWriteNoAtomic		= false;

	//if the src and des is in the same page, then move in NoAtomic.It is for flash.
	if((U32_AND_U32(PNtMEM(preEntityStart), NOR_EEPAGE_MASK) == U32_AND_U32(PNtMEM(copyDes), NOR_EEPAGE_MASK))
		|| (U32_AND_U32(PNtMEM(gNvmCompact_overLapBound), NOR_EEPAGE_MASK) == U32_AND_U32(PNtMEM(copyDes), NOR_EEPAGE_MASK))
		)
	{
		isWriteNoAtomic		= true;
	}

	if(((false == isExtend) && (copyDes <= gNvmCompact_overLapBound))
	  || ((true == isExtend) && (copyDes >= gNvmCompact_overLapBound)))
	{
		NvmCompact_SetLastMoveFlag(preCopySrc, preCopyDes);
		gNvmCompact_overLapBound		= preCopySrc;

		RESET_POWER(POSITION_MOVE_ENTITRY + 4, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	}

	if(true == isWriteNoAtomic)
	{
		nvmCopyNoAtomic(MEMtPN8(copyDes), MEMtPN8(copySrc), copySize);
		isWriteNoAtomic		= false;

		RESET_POWER(POSITION_MOVE_ENTITRY + 5, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	}
	else
	{
		nvmCopyLost(MEMtPN8(copyDes), MEMtPN8(copySrc), copySize);

		RESET_POWER(POSITION_MOVE_ENTITRY + 6, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
	}

	return;
}

void NvmCompact_MoveEntity(pNvmCompactInfo nvmComInfo, pFreeInfo freeBlockInfo)
{
	u1 		isOpenTransaction;
	u1		isMoveEntity;
	u16 	preEntityPageFrag;
	u16 	copySize;
	PN8 	copySrc;
	PN8 	copyDes;
	PN8		preCopySrc;
	PN8		preCopyDes;
	PN8		preEntityStart;
	PN8		nextEntityStart;
	u32		moveEntitySize;
	
	nextEntityStart				= PN8_A_U32(freeBlockInfo->nextFreeBlockStart, freeBlockInfo->nextFreeBlockSize);
	if(false == freeBlockInfo->isExtend)
	{
		preEntityStart				= PN8_A_U32(freeBlockInfo->curFreeBlockStart, freeBlockInfo->cumulateSize);
		moveEntitySize				= U32_S_U32(PMtMEM(freeBlockInfo->curFreeBlockStart), PNtMEM(nextEntityStart));
	}
	else
	{
		preEntityStart				= PN8_S_U32(freeBlockInfo->curFreeBlockStart, freeBlockInfo->curFreeBlockSize);
		moveEntitySize				= U32_S_U32(PNtMEM(nextEntityStart), PNtMEM(PN8_A_U32(preEntityStart, freeBlockInfo->cumulateSize)));
	}
	isMoveEntity					= true;
	
	// start from "break point" if move action is broken on last time.
	if((true == nvmComInfo->isCopyBroken)
			&& (((false == freeBlockInfo->isExtend) && (freeBlockInfo->nextFreeBlockStart >= nvmComInfo->brokenCopySrc))
			||  ((true == freeBlockInfo->isExtend)  && (freeBlockInfo->nextFreeBlockStart <= nvmComInfo->brokenCopySrc))))
	{
			isMoveEntity		= false;
	}
	else if (true == nvmComInfo->isCopyBroken)
	{
		gNvmCompact_overLapBound	= nvmComInfo->brokenCopySrc;
		preEntityStart				= nvmComInfo->brokenCopyDes;
		if(false == freeBlockInfo->isExtend)
		{
			moveEntitySize				= U32_S_U32(PNtMEM(nvmComInfo->brokenCopySrc), PNtMEM(nextEntityStart));
		}
		else
		{
			moveEntitySize				= U32_S_U32(PNtMEM(nextEntityStart), PNtMEM(nvmComInfo->brokenCopySrc));
		}
		nvmComInfo->isCopyBroken	= false;
	}
	
	if(true == isMoveEntity)
	{
		if(false == freeBlockInfo->isExtend)
		{
			copyDes			= preEntityStart;
			copySrc			= PN8_A_U32(nextEntityStart, moveEntitySize);
		}
		else
		{
			copyDes			= preEntityStart;
			copySrc			= PN8_A_U32(preEntityStart,freeBlockInfo->cumulateSize);
		}
			
		NvmCompact_IsCommitLastMove(copySrc, copyDes, moveEntitySize);
		isOpenTransaction	= NvmCompact_IsOpenTransaction(copyDes, freeBlockInfo->isExtend);
		RESET_POWER(POSITION_MOVE_ENTITRY, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

		do
		{
			if(false == freeBlockInfo->isExtend)
			{
				preEntityPageFrag			= U32t16(U32_AND_U32(PNtMEM(copyDes), EEPAGE_MASK));
			}
			else
			{
				preEntityPageFrag			= U32t16(U32_S_U32(EEPAGE_MASK, U32_AND_U32(PNtMEM(copyDes), EEPAGE_MASK)));
			}

			//if previous entity is page align, copy to the next page
			if(0 == preEntityPageFrag)
			{
				preEntityPageFrag	= EEPAGE_SIZE;
			}

			//insure that copySize is less or equal to page size.
			if(moveEntitySize < preEntityPageFrag)
			{
				copySize				= U32t16(moveEntitySize);
			}
			else
			{
				copySize				= preEntityPageFrag;
			}

			preCopySrc					= copySrc;
			preCopyDes					= copyDes;
			if(false == freeBlockInfo->isExtend)
			{
				copySrc						= PN8_S_U16(copySrc, copySize);
				copyDes						= PN8_S_U16(copyDes, copySize);
				moveEntitySize				= U32_S_U16(moveEntitySize, copySize);
			}

			//move process in transaction because overlay happened.
			if(true == isOpenTransaction)
			{
				NvmCompact_MoveInTran(copyDes, copySrc, copySize);
			}
			else
			{
				if(true == freeBlockInfo->isExtend)
				{
					preEntityStart			= PN8_A_U32(preEntityStart, freeBlockInfo->cumulateSize);
				}
				NvmCompact_MoveInNoTran(preEntityStart, preCopyDes, preCopySrc, copyDes, copySrc, copySize, freeBlockInfo->isExtend);
			}

			if(true == freeBlockInfo->isExtend)
			{
				copySrc						= PN8_A_U16(copySrc, copySize);
				copyDes						= PN8_A_U16(copyDes, copySize);
				moveEntitySize				= U32_S_U16(moveEntitySize, copySize);
			}

		}while(0 != moveEntitySize);

		if(true == freeBlockInfo->isLastFreeBlock)
		{
			RESET_POWER(POSITION_NVM_RESIZE + 8, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);
			NvmCompact_SetLastMoveFlag(copySrc, copyDes);
		}
	}
}
#endif
