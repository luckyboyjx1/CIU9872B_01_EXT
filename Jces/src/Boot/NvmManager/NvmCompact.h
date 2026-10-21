#ifndef   __NVM_COMPACT_H__
#define   __NVM_COMPACT_H__

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "FreeTable.h"

#define		MOVE_FLAG_TABLE_IN_NVM_SIZE					0x40
#define		MAP_FLAG_TABLE_IN_NVM_SIZE					0x40
#define		DEALLO_FLAG_TABLE_IN_NVM_SIZE				(MOVE_FLAG_TABLE_IN_NVM_SIZE + MAP_FLAG_TABLE_IN_NVM_SIZE)


#define 	NVM_COMPACT_NO_RESOURCE           	 		0x9005

typedef struct nvm_compact_info
{
	PN8	brokenCopySrc;
	PN8	brokenCopyDes;
	u8	isCopyBroken;
} nvmCompactInfo,  *pNvmCompactInfo;

#if BOOT__MEMORY__NVM_MANAGER_NVM_COMPACT
	extern memref NvmCompact_GetBase(PN8 topNvm);
	extern memref NvmCompact_GetFlagTableStart(void);
	extern void   NvmCompact_Boot(void);
	extern void   NvmCompact_InitMoveFlagTable(void);
	extern void   NvmCompact_SetLastMoveFlag(PN8 src, PN8 des);
	extern u16    NvmCompact_GetLastMoveFlag(void);
	extern void   NvmCompact_ClearMoveFlagTable(void);
	extern void   NvmCompact_GetBrokenMoveFlag(pNvmCompactInfo info);
	extern void   NvmCompact_SetLastUpdateMapIndex(u16 mapIndex);
	extern void   NvmCompact_ClearMapFlagTable(void);
	extern u16    NvmCompact_GetLastUpdateMapIndex(void);
	extern void   NvmCompact_SetOverLapBound(PN8 bound);
	extern void   NvmCompact_ResetTables(void);
	extern void   NvmCompact_CheckBackUpCapacity(void);
	extern void   NvmCompact_IsCommitLastMove(PN8 copySrc, PN8 copyDes, u32 nextEntitySize);
	extern u1     NvmCompact_IsOpenTransaction(PN8 copyDes, u1 isExtend);
	extern void   NvmCompact_UpdateMapTable(void);
	extern void   NvmCompact_UpdateMapBitMap(void);
	extern void   NvmCompact_MoveEntity(pNvmCompactInfo nvmComInfo, pFreeInfo freeBlockInfo);
#else
	#define NvmCompact_GetBase(topNvm)											0
	#define NvmCompact_GetFlagTableStart()										0
	#define NvmCompact_Boot()
	#define NvmCompact_InitMoveFlagTable()
	#define NvmCompact_SetLastMoveFlag(src, des)
	#define NvmCompact_GetLastMoveFlag()										0
	#define NvmCompact_ClearMoveFlagTable()
	#define NvmCompact_GetBrokenMoveFlag(info)
	#define NvmCompact_SetLastUpdateMapIndex(mapIndex)
	#define NvmCompact_ClearMapFlagTable()
	#define NvmCompact_GetLastUpdateMapIndex()									0
	#define NvmCompact_SetOverLapBound(bound)
	#define NvmCompact_ResetTables()
	#define NvmCompact_CheckBackUpCapacity()
	#define NvmCompact_IsCommitLastMove(copySrc, copyDes, nextEntitySize)
	#define NvmCompact_IsOpenTransaction(copyDes, isExtend)						0
	#define NvmCompact_UpdateMapTable()
	#define NvmCompact_UpdateMapBitMap()
	#define NvmCompact_MoveEntity(nvmComInfo, freeBlockInfo)
#endif

#endif
