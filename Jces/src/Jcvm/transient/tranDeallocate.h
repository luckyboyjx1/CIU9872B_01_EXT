#ifndef __TRAN_DEALLOCATE_H__
#define __TRAN_DEALLOCATE_H__

#include "target.h"
#include "profile.h"
#include "FreeTable.h"
#include "DataOperation.h"

#define COMPACT_SPACE								0
#define RE_ALLOCATE_RAM_SPACE						1

typedef struct
{
	u8	ctxId;
	u8	handleType;
} ramDeallocateInfo;

#if JCVM__MEMORY__TRAN_DEALLO
	extern void tranDeallocate_modifyTrObjPtr(void);    SMALL_MODE
	extern void tranDeallocate_moveEntity(pFreeInfo freeBlockInfo);
	extern void	tranDeallocate_compactRam(u8 compactType);
	extern u16	tranDeallocate_getTrObjNodeStart(PX8 TrObjNode);
	extern u16	tranDeallocate_getTrObjNodeSize(PX8 TrObjNode);
	extern u1	tranDeallocate_GetTranObjInfo(u16 index, PR8 trObjNode);
#else
	#define tranDeallocate_modifyTrObjPtr()
	#define tranDeallocate_moveEntity(freeBlockInfo)
	#define tranDeallocate_compactRam(compactType)
	#define tranDeallocate_getTrObjNodeStart(TrObjNode)					0
	#define tranDeallocate_getTrObjNodeSize(TrObjNode)					0
	#define tranDeallocate_GetTranObjInfo(index, trObjNode)				0
#endif

#endif
