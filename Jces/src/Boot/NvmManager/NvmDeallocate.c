/*
 * NvmDeallocate.c
 *
 *  Created on: 2014Äê10ÔÂ28ÈÕ
 *      Author: linguoming1
 */

#include "NvmMap.h"
#include "NvmDeallocate.h"
#include "NvmManager.h"
#include "FreeTable.h"
#include "NvmCompact.h"
#include "MemBackUp.h"
#include "BootRegister.h"
#include "BootAreaInitRam.h"
#include "CgmManager.h"
#if BOOT__MEMORY__NVM_MANAGER_NVM_DEALLOCATE

void NvmDeallocate_Free(u16 entryIndex, u8 pckId)
{
	u1		isChangeAvail;
	u8		transactionStarted;
	u32		objectSize;
	memref 	entryStart;

	isChangeAvail		= false;
	transactionStarted	= 0;
	objectSize			= 0;
	entryStart			= MapTable_GetEntryAddr(entryIndex);
/*@ -type @*/
#if GP__CGM__FOR_MASK
	if((g_bCheckFlag == CHECK_GC))
	{
		CgmRun_ChangeCC(get_object_owner(entryStart));
	}
#endif
/*@ +type @*/
	if((MAP_ENTRY_OBJECT_PCK_ID == pckId) || (MAP_ENTRY_BLOCK_PCK_ID == pckId))
	{
		if(entryStart == FreeTable_GetTopFreeNvm())
		{
			//check that it is a permanent object or it is a block
			if((MAP_ENTRY_BLOCK_PCK_ID == pckId)
				|| ((MAP_ENTRY_OBJECT_PCK_ID == pckId) && (/*@ -type @*/false == isTranObj(MEMtPN8(entryStart))/*@ +type @*/)))
			{
				if (!isInProgress())
				{
					memBeginTransaction(SINGLE_APDU_PROCESS);
					transactionStarted = 1;
				}

				MapTable_SetEntryStatus(entryIndex, MAP_TABLE_ENTRY_STATUS_AVAIL);
				if(0x01 == FreeTable_GetEntryNum())
				{
					MapTable_Reset();
				}
				objectSize	= MapTable_GetEntitySize(MEMtPN8(entryStart));
				FreeTable_DeAlloc(objectSize);

				if(transactionStarted)
				{
					memCommitTransaction();
				}

				isChangeAvail		= true;
			}
		}

		if(false == isChangeAvail)
		{
			MapTable_SetEntryStatus(entryIndex, MAP_TABLE_ENTRY_STATUS_FREE);
		}
		if(!objectSize)
		{
			objectSize	= MapTable_GetEntitySize(MEMtPN8(entryStart));
		}
		/*@ -type @*/CgmManager_Free(objectSize, HAS83_MASK);/*@ +type @*/
	}
	else
	{
		MapTable_SetEntryStatus(entryIndex, MAP_TABLE_ENTRY_STATUS_AVAIL);
	}
}

void NvmDeallocate_Main(u1 isRollBack)
{
	if(FREE_TABLE_NO_OPERATION != FreeTable_RoomArrange(isRollBack, true, false))
	{
		NvmCompact_ResetTables();
	}
}

#endif
