#ifndef __BOOT_REGISTER_H__
#define __BOOT_REGISTER_H__

#include "DataOperation.h"
#include "profile.h"
#include "tranDeallocate.h"
#include "ob_manager.h"
#include "FreeTable.h"
#include "tranDeallocate.h"
#include "tranManager.h"
#include "stack.h"

typedef u1   	( IS_TRAN_OBJ )(PN8 trObjNode);
typedef u32  	( GET_OBJ_SIZE_CALLBACK )(PN8 objAddr);
typedef u1   	( GET_TRAN_INF_CALLBACK )(u16 index, PR8 trObjNode);
typedef u16  	( GET_TRAN_START_CALLBACK )(PX8 trObjNode);
typedef u16  	( GET_TRAN_SIZE_CALLBACK )(PX8 trObjNode);
typedef void 	( TRAN_MOV_CALLBACK )(pFreeInfo freeBlockInfo);
typedef void 	( TRAN_OBJ_MODIFY_CALLBACK )(void);
typedef u8   	( JCSTACK_GET_UPDATE_PTR_NUM_CALLBACK )(void);
typedef memref 	( JCSTACK_UPDATE_PTR_CALLBACK)(u8 i, u32 changeLen, u1 opType, u1 isUpdatePtr);
typedef u16  	( GET_COUNTER )(PX32 counterAddr, u8 version);

extern IS_TRAN_OBJ * CONST isTranObj;
extern GET_OBJ_SIZE_CALLBACK * CONST getObjSize;
extern GET_TRAN_INF_CALLBACK * CONST getTranInfo;
extern GET_TRAN_START_CALLBACK * CONST getTranStart;
extern GET_TRAN_SIZE_CALLBACK * CONST getTranSize;
extern TRAN_MOV_CALLBACK * CONST tranMoveEntity;
extern TRAN_OBJ_MODIFY_CALLBACK* CONST trObjModify;
extern GET_COUNTER * CONST getKeyCounter;

extern JCSTACK_GET_UPDATE_PTR_NUM_CALLBACK * CONST JcStack_GetUpdatePtrNumCallback;
extern JCSTACK_UPDATE_PTR_CALLBACK	* CONST JcStack_UpdatePtrCallBack;


#endif
