#ifndef   __FREE_TABLE_H__
#define   __FREE_TABLE_H__

#include "DataOperation.h"
#include "bootConfigProfile.h"
#if HAL__JCES_CHIP__FOR_TEST
	#include "hw.h"
	#include "JcesTestRegister.h"
#else
	#define ResetPower_SetPosition(position)
	#define ResetPower_GetPosition()
	#define ResetPower_Disable()
	#define ResetPower_Enable(n)
	#define RESET_POWER(pos, min, max)
#endif

#define MIN_FREE_TABLE_NUM										1
#define	FREE_TABLE_IN_NVM_SIZE									0x100

#define	FREE_TABLE_BLOCK_START_LEN								4
#define	FREE_TABLE_BLOCK_SIZE_LEN								4
#define	FREE_TABLE_ENTRY_LEN									(FREE_TABLE_BLOCK_START_LEN + FREE_TABLE_BLOCK_SIZE_LEN)
#define	FREE_TABLE_GENERATE_CVER_FLOW							0x5A00
#define FREE_TABLE_GENERATE_FINISH								0xA500
#define	FREE_TABLE_GENERATE_SHORTEN								0x00B3
#define FREE_TABLE_GENERATE_EXTEND								0x003B
#define FREE_TABLE_GENERATE_COMPACT								0x00C3
#define	FREE_TABLE_GENERATE_NO_OPERATE							0xA5A5

#define FREE_TABLE_GENERTAE_FIN_SHIFT							8
#define FREE_TABLE_GENERTAE_FIN_MASK							0xFF00
#define FREE_TABLE_OPT_MASK										0x00FF

#define FREE_TABLE_NO_OPERATION									0xFFFFFFFF

#define FreeTable_GetGenFinState(t)								U16_AND_U16(t, FREE_TABLE_GENERTAE_FIN_MASK)
#define FreeTable_SetGenFinState(ot, nt)						U16_OR_U16(U16_AND_U16(ot, FREE_TABLE_OPT_MASK), nt)

#define FreeTable_GetOpType(t)									U16_AND_U16(t, FREE_TABLE_OPT_MASK)
#define FreeTable_SetOpType(ot, nt)								U16_OR_U16(U16_AND_U16(ot, FREE_TABLE_GENERTAE_FIN_MASK), nt)

typedef struct free_table_info
{
	PM8 curFreeBlockStart;
	u32	curFreeBlockSize;
	PM8 nextFreeBlockStart;
	u32 nextFreeBlockSize;
	u32	cumulateSize;
	u8  isExtend;
	u8	isLastFreeBlock;
} freeInfo,  *pFreeInfo;

#if BOOT__MEMORY__FREE_TABLE
	extern void    FreeTable_SetState(u16 state);
	extern u16     FreeTable_GetState(void);
	extern u16	   FreeTable_GetStateNvm(void);
	extern void    FreeTable_SetStateNvm(u16 state);
	extern void    FreeTable_SetEntryNum(u16 size);
	extern u16     FreeTable_GetEntryNum(void);
	extern u16     FreeTable_GetEntryNumNvm(void);
	extern void    FreeTable_SetMapTopIndex(u16 index);
	extern u16 	   FreeTable_GetMapTopIndexNvm(void);
	extern void    FreeTable_SetMapTopIndexNvm(u16 index);
	extern u16	   FreeTable_GetObjectIdFound(void);
	extern void	   FreeTable_SetObjectIdFound(u16 index);
	extern u16	   FreeTable_GetObjectIdFoundNVM(void);
	extern void    FreeTable_SetObjectIdFoundNVM(u16 index);
	extern memref  FreeTable_GetEntry(u16 i);
	extern memref  FreeTable_GetEntryNvm(u16 i);
	extern void    FreeTable_SetFreeBlockStart(PX8 a, u32 start);
	extern memref  FreeTable_GetFreeBlockStart(PM8 a);
	extern void    FreeTable_SetFreeBlockSize(PX8 a, u32 size);
	extern u32     FreeTable_GetFreeBlockSize(PM8 a);
	extern memref  FreeTable_GetBase(PN8 topNvm);
	extern memref  FreeTable_GetTblAddr(void);
	extern u16 	   FreeTable_GetMaxEntryNum(void);
	extern memref  FreeTable_GetPtr(void);
	extern memref  FreeTable_GetStart(void);
	extern u16 	   FreeTable_GetCurEntryNum(void);
	extern u1 	   FreeTable_CheckIsCompact(u16 status);
	extern void    FreeTable_Boot(PN8 topNvm);
	extern void    FreeTable_Init(void);
	extern void    FreeTable_InitTranCompact(u16 lastTran);
	extern void    FreeTable_Commit(void);
	extern void    FreeTable_ParaCommit(void);
	extern void    FreeTable_Reset(void);
	extern boolean FreeTable_IsFull(void);
	extern memref  FreeTable_GetTopFreeNvm(void);
	extern u32     FreeTable_GetRemaindNvm(void);
	extern memref  FreeTable_Alloc(u32 size);
	extern void    FreeTable_DeAlloc(u32 size);
	extern u1      FreeTable_IsFreeBlock(u16 entryIndex, PX8 trObjNode, u1 isNvmFreeTable);
	extern memref  FreeTable_GetFreeStart(u16 entryIndex, PX8 trObjNode, u1 isNvmFreeTable);
	extern u32     FreeTable_GetFreeSize(PN8 blockStart, PX8 trObjNode, u1 isNvmFreeTable);
	extern u16     FreeTable_GetTableState(u1 isNvmFreeTable);
	extern void    FreeTable_ModifyObj(u1 isNvmFreeTable);
	extern void    FreeTable_Merge(void);
	extern u1	   FreeTable_Insert(PM8 blockStart, u32 blockSize, u1 isIncrease);
	extern u32     FreeTable_Compare(PM8 entityStart, u16 maxFreeEntryNum, u1 isIncrease);
	extern boolean FreeTable_Generate(u1 isRollBack, u1 isNvmFreeTable);
	extern u32     FreeTable_RoomArrange(u1 isRollBack, u1 isNvmFreeTable, u1 isDeletePkg);
	extern void    FreeTable_BufInit(void);
#else
	#define FreeTable_SetState(state)
	#define FreeTable_GetState()																0
	#define FreeTable_GetStateNvm()																0
	#define FreeTable_SetEntryNum(size)
	#define FreeTable_GetEntryNum()																0
	#define FreeTable_GetEntryNumNvm()															0
	#define FreeTable_SetMapTopIndex(index)
	#define FreeTable_SetMapTopIndexNvm(index)
	#define FreeTable_GetMapTopIndexNvm()														0
	#define FreeTable_GetEntry(i)																0
	#define FreeTable_GetEntryNvm(i)															0
	#define FreeTable_SetFreeBlockStart(a, start)
	#define FreeTable_GetFreeBlockStart(a)														0
	#define FreeTable_SetFreeBlockSize(a, size)
	#define FreeTable_GetFreeBlockSize(a)														0
	#define FreeTable_GetBase(topNvm)															0
	#define FreeTable_GetPtr()																	0
	#define FreeTable_GetStart()																0
	#define FreeTable_GetCurEntryNum()															0
	#define FreeTable_CheckIsCompact(status)													0
	#define FreeTable_Boot(topNvm)
	#define FreeTable_Init()
	#define FreeTable_InitTranCompact(lastTran)
	#define FreeTable_Commit()
	#define FreeTable_ParaCommit()
	#define FreeTable_Reset()
	#define FreeTable_IsFull()																	0
	#define FreeTable_GetTopFreeNvm()															0
	#define FreeTable_GetRemaindNvm()															0
	#define FreeTable_Alloc(size)																0
	#define FreeTable_DeAlloc(size)
	#define FreeTable_IsFreeBlock(entryIndex, trObjNode, isNvmFreeTable)						0
	#define FreeTable_GetFreeStart(entryIndex, trObjNode, isNvmFreeTable)						0
	#define FreeTable_GetFreeSize(blockStart, trObjNode, isNvmFreeTable)						0
	#define FreeTable_GetTableState(isNvmFreeTable)												0
	#define FreeTable_MoveEntity(nvmComInfo, freeBlockInfo, isNvmFreeTable)
	#define FreeTable_ModifyObj(isNvmFreeTable)
	#define FreeTable_Merge()
	#define FreeTable_Insert(blockStart, blockSize, isNvmFreeTable)
	#define FreeTable_Compare(entityStart, maxFreeEntryNum, isIncrease)							0
	#define FreeTable_Generate(isRollBack, isNvmFreeTable)										0
	#define FreeTable_Compact(isRollBack, isNvmFreeTable)										0
#endif

#endif
