/*
 * NvmAllocate.c
 *
 *  Created on: 2014年10月28日
 *      Author: linguoming1
 */

#include "NvmAllocate.h"
#include "FreeTable.h"
#include "MemBackUp.h"
#include "NvmMap.h"
#include "NvmManager.h"
#include "MemOperation.h"
#include "BootAreaInitRam.h"
#include "CgmManager.h"
#if BOOT__MEMORY__NVM_MANAGER_NVM_ALLOCATE

u32 NvmAllocate_GetAlignSize(u32 size)
{
	//4 bytes align
	if(0 != U32_R_U8(size, MAP_ENTRY_ENTITY_ALIGN_SIZE))
	{
		size = U8_A_U32(U8_S_U8(MAP_ENTRY_ENTITY_ALIGN_SIZE, U32_R_U8(size, MAP_ENTRY_ENTITY_ALIGN_SIZE)), size);
	}
	
	return size;
}

void NvmAllocate_SetEntityHeaderSize(PN8 startAddr, u32 size)
{
	nvmWriteU16NoAtomic(PN8_A_CONST(startAddr, MAP_ENTRY_ENTITY_LEN_OFFSET), U32_D_U8(size, MAP_ENTRY_ENTITY_ALIGN_SIZE));

}
/*
 * modifed by xxj@20210328: 对于新分配的空间在特定情况下不需要初始化
 */
u16 NvmAllocate_Main(u32 size, u8 pckId)
{
	u16		avaiIndex;
	u1		isInTrasaction;
	PN8		startAddr;
	//u32		sizeBackup;

	isInTrasaction		= false;
	
	if(MAP_ENTRY_BLOCK_PCK_ID == pckId)
	{
		size	= U32_A_U8(size, MAP_ENTRY_ENTITY_HEAER_SIZE);
	}
	
	//sizeBackup = size;	//remove warning,dengyiwen,2022/9/26
	size 			 = NvmAllocate_GetAlignSize(size);

	if(false == isInProgress())
	{
		isInTrasaction	= true;
		memBeginTransaction(SINGLE_APDU_PROCESS);
	}
	
	startAddr			= MEMtPN8(FreeTable_Alloc(size));
#if GP__CGM__FOR_MASK
	if(U16t8(U16_AND_U16(cc, APPLET_MASK) < 0x80))
	{
		//CgmManager_Alloc((size + MAP_ENTRY_SIZE),HAS83_MASK);
		CgmManager_Alloc(size,HAS83_MASK);
	}
#endif
	
	avaiIndex			= MapTable_AddEntry(startAddr, pckId);
//#if GP__CGM__FOR_MASK
//	if(U16t8(U16_AND_U16(cc, APPLET_MASK) < 0x80))
//	{
//		//CgmManager_Alloc((size + MAP_ENTRY_SIZE),HAS83_MASK);
//		CgmManager_Alloc(size,HAS83_MASK);
//	}
//#endif

	if(MAP_ENTRY_BLOCK_PCK_ID == pckId)
	{
		nvmWriteU8NoAtomic(startAddr, MAP_ENTRY_ENTITY_TYPE); //TODO mwp: 这里使用noAtomic 是否有性能问题，因为writeEEP_Type != cacheData_Type 引起把buf 的数据写NVM作备份。
		NvmAllocate_SetEntityHeaderSize(startAddr, size);
	}
	
	//for power broken test only
	//initialize the extra memory(GP delete leads to memory move, so that the data in the extra memory is unexpected, and it will lead to a check error in the Power Break Down Test)

//	if ((sizeBackup != size)&&(MAP_ENTRY_BLOCK_PCK_ID != pckId))
//	{
//		nvmFillNoAtomic(PN8_A_U32(startAddr, sizeBackup), 0, U32_S_U32(size, sizeBackup));
//	}
	
	if(true == isInTrasaction)
	{
		memCommitTransaction();
	}

	return avaiIndex;
}

#endif
