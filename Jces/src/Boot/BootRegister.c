#include "BootRegister.h"
#include "ob_manager.h"
#include "keySet.h"


/*@ -type @*/
GET_OBJ_SIZE_CALLBACK * CONST getObjSize		= getObjTotalSize;

#if JCVM__MEMORY__TRAN_DEALLO
	IS_TRAN_OBJ * CONST isTranObj					= tranMananger_IsTransient;
	GET_TRAN_INF_CALLBACK * CONST getTranInfo		= tranDeallocate_GetTranObjInfo;
	GET_TRAN_START_CALLBACK * CONST getTranStart	= tranDeallocate_getTrObjNodeStart;
	GET_TRAN_SIZE_CALLBACK * CONST getTranSize		= tranDeallocate_getTrObjNodeSize;
	TRAN_MOV_CALLBACK * CONST tranMoveEntity		= tranDeallocate_moveEntity;
	TRAN_OBJ_MODIFY_CALLBACK* CONST trObjModify		= tranDeallocate_modifyTrObjPtr;

#else
	GET_TRAN_INF_CALLBACK * CONST getTranInfo		= 0;
	GET_TRAN_START_CALLBACK * CONST getTranStart	= 0;
	GET_TRAN_SIZE_CALLBACK * CONST getTranSize		= 0;
	TRAN_MOV_CALLBACK * CONST tranMoveEntity		= 0;
	TRAN_OBJ_MODIFY_CALLBACK* CONST trObjModify		= 0;
#endif


#if GP__CGM__FOR_MASK
//	CGM_GET_FLAG_CALLBACK * CONST getCheckFlag  = CgmInit_GetFlag;
//	CGM_SWITCH_CALLBACK *  CONST  cgmSwitch		= CgmInit_EnableOrNot;
//	CGM_RUN_CHANGECC_CALLBACK * CONST changeCc  = CgmRun_ChangeCcByEntryAdd;
//	CGM_SPACE_ALLOC_CALLBACK * CONST cgmSpaceAlloc = CgmManager_Alloc;
//	CGM_SPACE_FREE_CALLBACK * CONST cgmSpaceFree   = CgmManager_Free;
#endif
#if		BOOT__MEMORY__NVM_RESIZE
	JCSTACK_GET_UPDATE_PTR_NUM_CALLBACK * CONST JcStack_GetUpdatePtrNumCallback	= JCstack_GetUpdatePtrNum;
	JCSTACK_UPDATE_PTR_CALLBACK	* CONST JcStack_UpdatePtrCallBack	= JCstack_UpdatePtr;
#else
	JCSTACK_GET_UPDATE_PTR_NUM_CALLBACK * CONST JcStack_GetUpdatePtrNumCallback	= 0;
	JCSTACK_UPDATE_PTR_CALLBACK	* CONST JcStack_UpdatePtrCallBack	= 0;
#endif

#if  BOOT__CONFIGDATA__POWER_OFF_FOR_TEST
	GET_COUNTER * CONST getKeyCounter					= keySet_GetCounter;
#else
	GET_COUNTER * CONST getKeyCounter					= 0;
#endif
/*@ +type @*/
