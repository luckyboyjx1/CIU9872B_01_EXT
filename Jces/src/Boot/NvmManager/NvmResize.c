/*
 * NvmRezie.c
 *
 *  Created on: 2016Äê6ÔÂ7ÈÕ
 *      Author: linguoming1
 */

#include "FreeTable.h"
#include "NvmMap.h"
#include "NvmManager.h"
#include "NvmResize.h"
#include "BootRegister.h"
#include "FreeTableResize.h"
#include "NvmAllocate.h"

#if BOOT__MEMORY__NVM_RESIZE

void NvmResize_Main(u16 entryIndex, PN8 bodyAddr, u32 reSize, u1 isExtend)
{
	u1	isResize;
	PN8 entityStart;
	u32 entitySize;
	PN8 blockStart;
	u32 blockSize;


	entityStart	= MEMtPN8(MapTable_GetEntryAddr(entryIndex));
	entitySize	= MapTable_GetEntitySize(MEMtPN8(entityStart));
	isResize	= true;

	RESET_POWER(POSITION_NVM_RESIZE, INVALID_MIN_CIRCLE, INVALID_MAX_CIRCLE);

	if(MAP_ENTRY_ENTITY_TYPE != PN_R_U8(entityStart))
	{
		bootErrorHandle_ThrowError(SYSTEMEXCEPT_ILLEGAL_USE);
	}

	reSize	= NvmAllocate_GetAlignSize(reSize);

	//for resize a object
	if(MEM_NULL == bodyAddr)
	{
		if(reSize != entitySize)
		{
			//increase entity size
			if(reSize > entitySize)
			{
				blockSize	= U32_S_U32(reSize, entitySize);
				isExtend	= true;
			}
			//decrease entity size
			else
			{
				blockSize	= U32_S_U32(entitySize, reSize);
				isExtend	= false;
			}
		}
		else
		{
			isResize	= false;
		}
	}
	else //for resize a small part of the body in a object
	{
		if((false == isExtend) && (U32_S_U32(PNtMEM(bodyAddr), MapTable_GetEntryAddr(entryIndex)) < U32_A_U32(reSize, MAP_ENTRY_ENTITY_HEAER_SIZE)))
		{
			bootErrorHandle_ThrowError(SYSTEMEXCEPT_ILLEGAL_USE);
		}
		entitySize		= U32_S_U32(PNtMEM(bodyAddr),PNtMEM(entityStart));
		blockSize		= reSize;
	}

	if(true == isResize)
	{
		if(true == isExtend)
		{
			//check the remain size is enough
			FreeTable_Alloc(blockSize);
		}

		blockStart	= PN8_A_U32(entityStart, U32_S_U32(entitySize, blockSize));

		FreeTableResize_Main(blockStart, blockSize, isExtend);
	}
}

void NvmResize_UpdateEtityHeader(void)
{
	u1 		isExtend;
	u16		i;
	u16		maxFreeEntryNum;
	u16		maxMapEntry;
	PN8		entityStart;
	u32		entitySize;
	u32		changeSize;

	isExtend			= false;
	i					= MapTable_GetAppStartIndex();
	maxFreeEntryNum		= FreeTable_GetCurEntryNum();
	maxMapEntry			= MapTable_GetTopIndex();

	if(FREE_TABLE_GENERATE_EXTEND == FreeTable_GetOpType(FreeTable_GetState()))
	{
		isExtend		= true;
	}

	if(MIN_FREE_TABLE_NUM != maxFreeEntryNum)
	{
		for(; i <= maxMapEntry; U16_INC(i))
		{
			if(MAP_TABLE_ENTRY_STATUS_CREATION == MapTable_GetEntryStatus(i))
			{
				entityStart			= MEMtPN8(MapTable_GetEntryAddr(i));
				if(MAP_ENTRY_ENTITY_TYPE == PN_R_U8(entityStart))
				{
					entitySize	= MapTable_GetEntitySize(MEMtPN8(entityStart));
					changeSize	= FreeTableResize_GetChangeSize(entityStart,PN8_A_U32(entityStart, entitySize), maxFreeEntryNum, isExtend);
					if(true == isExtend)
					{
						NvmAllocate_SetEntityHeaderSize(entityStart, U32_A_U32(entitySize, changeSize));
					}
					else
					{
						NvmAllocate_SetEntityHeaderSize(entityStart, U32_S_U32(entitySize, changeSize));
					}
				}
			}
		}
	}
}

void NvmResize_UpdateJcvmRelativeInfo(void)
{
	u1		isExtend;
	u8		i;
	u8		maxPtr;
	PM8		ptrValue;
	u16		maxFreeEntryNum;
	u32		freeBlockSize;

	isExtend				= false;
	maxFreeEntryNum			= FreeTable_GetCurEntryNum();
	maxPtr					= JcStack_GetUpdatePtrNumCallback();

	if(FREE_TABLE_GENERATE_EXTEND == FreeTable_GetOpType(FreeTable_GetState()))
	{
		isExtend			= true;
	}

	for(i = 0; i < maxPtr; U8_INC(i))
	{
		ptrValue			= MEMtPM8(JcStack_UpdatePtrCallBack(i, 0, false, false));
		freeBlockSize		= FreeTable_Compare(ptrValue, maxFreeEntryNum, isExtend);
		if(0 != freeBlockSize)
		{
			JcStack_UpdatePtrCallBack(i, freeBlockSize, isExtend, true);
		}
	}
}

#endif
