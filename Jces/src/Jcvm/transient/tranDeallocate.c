#include "CgmManager.h"
#include "tranAllocate.h"
#include "tranDeallocate.h"
#include "lifecycle.h"
#include "object.h"
#include "tranManager.h"
#include "RamManager.h"
#include "ob_manager.h"
#include "contentManager.h"
#include "nativemethod_impl.h"
#include "target.h"
#include "freeTable.h"
#include "FreeTable.h"
#include "NvmMap.h"
#include "JcvmRam.h"
#include "rtrSpace.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if JCVM__MEMORY__TRAN_DEALLO

ramDeallocateInfo				DealloInfo;

#define		RECORD_DTR_OBJ																0
#define		RECORD_RTR_OBJ																1
#define		RE_ALLOCATE_DTR_OBJ															2
#define		RE_ALLOCATE_RTR_OBJ															3

#define		TR_OBJ_NODE_REF_OFFSET														0
#define		TR_OBJ_NODE_START_OFFSET													2
#define		TR_OBJ_NODE_SIZE_OFFSET														4
#define		TR_OBJ_NODE_CXT_OFFSET														6
#define		TR_OBJ_NODE_NO_CXT_SIZE														4
#define		TR_OBJ_NODE_SIZE															8

#define		TR_OBJ_TABLE_NUM_OFFSET														0
#define		TR_OBJ_TABLE_NUM_SIZE														4
#define		TR_OBJ_TABLE_NODE_START_OFFSET												TR_OBJ_TABLE_NUM_SIZE
#define		TR_OBJ_MAX_NODE_NUM															((TR_OBJ_TABLE_SIZE - TR_OBJ_TABLE_NUM_SIZE) / TR_OBJ_NODE_NO_CXT_SIZE)

#define		ILLEGAL_RAM_ADDRESS															0xFFFF
#define		ILLEGAL_LENGTH																0xFFFF

#define		ILLEGAL_START																0xFFFF
#define		ILLEGAL_END																	0xFFFF
#define		RAM_SPACE_START_SHIFT														16
#define		RAM_SAPCE_END_MASK															0xFFFF

#define		DELETED_TR_OBJ_MASK															0x8000
#define		DELETED_TR_OBJ_REF_MASK														(~ DELETED_TR_OBJ_MASK)
/********************************************************handle transient boject node************************************************************
*************************************************************************************************************************************************/
#define		tranDeallocate_setTrObjNodeRef(TrObjNode, ref)								PR_W_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_REF_OFFSET), ref)
#define		tranDeallocate_setTrObjNodeStart(TrObjNode, start)							PR_W_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_START_OFFSET), start)
#define		tranDeallocate_setTrObjNodeSize(TrObjNode,  size)							PR_W_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_SIZE_OFFSET), size)
#define		tranDeallocate_setTrObjNodeCxt(TrObjNode,  cxt)								PR_W_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_CXT_OFFSET), cxt)

#define		tranDeallocate_getTrObjNodeRef(TrObjNode)									PR_R_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_REF_OFFSET))
#define		tranDeallocate_getTrObjNodeCxt(TrObjNode)									PR_R_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_CXT_OFFSET))
/************************************************************************************************************************************************
*************************************************************************************************************************************************/

/********************************************************handle ramDeInfo ***********************************************************************
*************************************************************************************************************************************************/
#define		tranDeallocate_SetDealloCtxId(v)											(DealloInfo.ctxId = v)
#define		tranDeallocate_SetDealloAppHandleType(tv, rv)								(DealloInfo.handleType = U8_OR_U8(tv, U8_SHL_U8(rv, 4)))

#define		tranDeallocate_GetDealloCtxId()												(DealloInfo.ctxId)
#define		tranDeallocate_GetDealloTranType()											U8_AND_U8(DealloInfo.handleType, 0x0F)
#define		tranDeallocate_GetDealloType()												U8_SHR_U8(DealloInfo.handleType, 4)
/************************************************************************************************************************************************
*************************************************************************************************************************************************/

/********************************************************handle transient table******************************************************************
*************************************************************************************************************************************************/
#define		tranDeallocate_getTrObjNodeOffset(index)									U8_A_U16(TR_OBJ_TABLE_NODE_START_OFFSET, U8_M_U16(TR_OBJ_NODE_NO_CXT_SIZE, index))
#define		tranDeallocate_getTrObjNoCxtNode(TrObjTable, index)							PX8_A_U16(TrObjTable, tranDeallocate_getTrObjNodeOffset(index))

#define		tranDeallocate_setTrObjTableNum(TrObjTable, num)							PX_W_U16(PX8_A_U16(TrObjTable,TR_OBJ_TABLE_NUM_OFFSET), num)
#define		tranDeallocate_getTrObjTableNum(TrObjTable)									PX_R_U16(PX8_A_CONST(TrObjTable,TR_OBJ_TABLE_NUM_OFFSET))
/************************************************************************************************************************************************
*************************************************************************************************************************************************/
u16 tranDeallocate_getTrObjNodeStart(PX8 TrObjNode)									
{
	return PR_R_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_START_OFFSET));
}

u16 tranDeallocate_getTrObjNodeSize(PX8 TrObjNode)
{
	return PR_R_U16(PR8_A_CONST(TrObjNode,TR_OBJ_NODE_SIZE_OFFSET));
}

u8 tranDeallocate_checkSkipCurTrObj(u8 compactState, u8 ctxId, u8 ObjectInternalType, u16 curObjCc)
{
	u1 result;
	
	result		= false;
	
	if((RECORD_DTR_OBJ == compactState) || (RE_ALLOCATE_DTR_OBJ == compactState))
	{
		if (HEADER_JCRE_TransMode_Deselect == U8_AND_U8(ObjectInternalType, HEADER_JCRE_TransMode_Deselect))
		{
			if (ctxId != (GET_PKGID(curObjCc)))
			{
			 	result	= true;
			}
		}
	}

	return result;
}

u16 tranDeallocate_getRamSpaceStart(u16 ctxId, u8 transientType)
{
	u32			D_A		startAddr;
	u8      	D_A		spaceId;
	
	if (OBJ_HDR_TransientMode_DTR == transientType)
	{
		spaceId     	=   dtrSpace_searchExistDtrCtx(ctxId);
		
		if(INVALID_SPACE_ID != spaceId)
		{
			startAddr	= 	PXtMEM(dtrSpace_getDtrSpaceBase(spaceId));
		}
		else
		{
		 	startAddr	= ILLEGAL_ADDRESS;
		}
	}
	else
	{
		startAddr		= tranMananger_getRtrBase();
	}
	
	return U32t16(startAddr);
}

static memref tranDeallocate_getTransientObjectInfo(PR8 infoBuffer, u16 mapEntryIndex, u8 transientType, u1 isOnlyGetArrayPtrAddr)
{
	u8				D_A		objectType;
	u8				D_A		objectElementSize;
	u8				D_A		objectInternalType;
	u16				D_A		objectElementCount;
	u16				D_A		objectContext;
	u16				D_A		objectArrayStart;
	PN8				D_A		objectArrayPtrAddr;
	PN8				D_A		trObjAddr;
			
	objectElementSize			= 0;		
	trObjAddr					= MEMtPN8(MapTable_GetEntryAddr(mapEntryIndex));
	objectInternalType			= get_object_header_type(trObjAddr);
	objectArrayPtrAddr			= ILLEGAL_ADDRESS;
	
	// get object cc, internal type, type
	if ((U8_AND_U8(objectInternalType, HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
		&& (U8_AND_U8(objectInternalType, OBJ_HDR_TransientMode_MASK) == transientType)
	   )
	{
		objectType				= U8_AND_U8(objectInternalType, OBJ_HDR_OBJECT_TYPE_MASK);

		if (Header_ArrayType_Reference == objectType)
		{
			objectArrayPtrAddr	= PN8_A_CONST(trObjAddr, OA_DataArray_START);
		}
		else
		{
			objectArrayPtrAddr	= PN8_A_CONST(trObjAddr, PA_DataArray_START);
		}

		if(true != isOnlyGetArrayPtrAddr)
		{
			objectArrayStart 		= nvmReadU16(objectArrayPtrAddr);
			objectElementCount		= PN_R_U16(PN8_A_CONST(trObjAddr , PA_NumFields_WORD));
			objectContext			= PN_R_U16(PN8_A_CONST(trObjAddr , OBJ_HDR_Context));
	
			switch (objectType)
			{
				case Header_ArrayType_Byte:
				case Header_ArrayType_Boolean:
					objectElementSize 	= 1;
					break;
				case Header_ArrayType_Short:
				case Header_ArrayType_Reference:
					objectElementSize 	= 2;
					break;
				case Header_ArrayType_Int:
					objectElementSize	= SIZE_INT;
					break;
				default:
					break;
			}
	
			tranDeallocate_setTrObjNodeRef(infoBuffer, mapEntryIndex);
			tranDeallocate_setTrObjNodeStart(infoBuffer, objectArrayStart);
			tranDeallocate_setTrObjNodeSize(infoBuffer, U16_M_U16(objectElementSize, objectElementCount));
			tranDeallocate_setTrObjNodeCxt(infoBuffer, objectContext);
		}
	}
	
	return PNtMEM(objectArrayPtrAddr);
}

static void tranDeallocate_finishCompactTrObj(u8 dtrFlag ,u8 rtrMask)
{
	u8				D_A		ctxId;
	u8				D_A		transactionStarted;
	u16				D_A		max;

	max					= 0;
	transactionStarted	= 0;

	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}

	/* Due to compaction, max_dTP may be incorrect, re-compute it */
	for (ctxId = 0; ctxId < NUM_GROUPS; U8_INC(ctxId))
	{
		if(tranMananger_getDtpByCtxId(ctxId) > max)
		{
			max = tranMananger_getDtpByCtxId(ctxId) ;
		}
	}
	
	tranMananger_setMaxDtp(max);
	
	if (max != JcvmConfig_GetMaxDtp())
	{
		JcvmConfig_SetMaxDtp(max);
	}

	if (dtrFlag != 0)
	{
		nvmFillAtomic(MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()), 0x00, dTP_NUM);
	}
	if (rtrMask != 0)
	{
		JcvmConfig_SetRtpObjectDeleted(0);
	}

	if (transactionStarted)
	{
		commit_transaction();
	}
}


//TODO mwp: 逻辑可能存在问题，重点测试。
static u16 tranDeallocate_ReAlLocate(u8 profileindex)
{
	u8				D_A		array_header;
	u8				D_A		trObjNode[TR_OBJ_NODE_SIZE];
	u16				D_A		i;
	u16				D_A		tempRamPtr;
	u16				D_A		objectSize;
	u16				D_A		max_entry_count;
	PN8				D_A		trObjAddr;

	tempRamPtr			= 0;
	max_entry_count 	= MapTable_GetTopIndex();
	
	for (i = MASK_MAP_ENTRY_COUNT; i < max_entry_count; U16_INC(i))
	{
		if((MapTable_IsCreationObject(i))&&(MapTable_IsCurrentProfile(i, profileindex))) //modify by weipj@ 断电测试修改
		{
			trObjAddr			= MEMtPN8(MapTable_GetEntryAddr(i));
			if(true == tranDeallocate_GetTranObjInfo(i, trObjNode))
			{
				objectSize			= tranDeallocate_getTrObjNodeSize(MEMtPX8(trObjNode));
				array_header		= PF_R_U8(trObjAddr);

			if (U8_AND_U8(array_header, Header_ArrayType_Reference) == Header_ArrayType_Reference)
			{
				trObjAddr = PF8_A_CONST(trObjAddr, OA_DataArray_START);
			}
			else
			{
				trObjAddr = PF8_A_CONST(trObjAddr, PA_DataArray_START);
			}
			nvmWriteU16NoAtomic(trObjAddr, tempRamPtr);
			tempRamPtr			= U32t16(U16_A_U16(tempRamPtr, objectSize));
			}
		}
	}
	
	return tempRamPtr;
}

u1 tranDeallocate_GetTranObjInfo(u16 index, PR8 trObjNode)
{
	u1 		getResult;
	u8 		ctxId;
	u8 		compactState;
	u8 		ramType;
	u8		ObjectInternalType;
	u16		curObjCc;
	PN8		trObjAddr;
	
	getResult			= true;
	ctxId				= tranDeallocate_GetDealloCtxId();
	ramType				= tranDeallocate_GetDealloTranType();
	compactState		= tranDeallocate_GetDealloType();
	
	trObjAddr			= MEMtPN8(MapTable_GetEntryAddr(index));
	ObjectInternalType	= get_object_header_type(trObjAddr);
	trObjAddr			= MEMtPN8(tranDeallocate_getTransientObjectInfo(trObjNode, index, ramType, false));
	if(MEMtPN8(ILLEGAL_ADDRESS) == trObjAddr)
	{
	 	getResult		= false;
	}
	
	if(true == getResult)
	{
		curObjCc			= tranDeallocate_getTrObjNodeCxt(trObjNode);
		if(true == tranDeallocate_checkSkipCurTrObj(compactState, ctxId, ObjectInternalType, curObjCc))
		{
			getResult		= false;
		}
	}
	
	return getResult;
}

void tranDeallocate_modifyTrObjPtr(void)    SMALL_MODE
{
	u8		trObjNode[TR_OBJ_NODE_SIZE];
	u8		ObjectInternalType;
	u16		i;
	u16		maxFreeEntryNum;
	u16		freeBlockSize;
	u16		maxMapEntry;
	u16		tempRamPtr;
	PN8		trObjAddr;

	i					= MapTable_GetAppStartIndex();
	maxFreeEntryNum		= FreeTable_GetCurEntryNum();
	maxMapEntry			= MapTable_GetTopIndex();

	if(MIN_FREE_TABLE_NUM != maxFreeEntryNum)
	{
		for(; i < maxMapEntry; U16_INC(i))
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if(i > MASK_MAP_ENTRY_COUNT)
			{
				if(!MapTable_IsCurrentProfile(i, G_CurExecutingProfileIndex))
				{
					continue;
				}
			}
#endif
			if(MapTable_IsCreationObject(i))
			{
				if(true == tranDeallocate_GetTranObjInfo(i, trObjNode))
				{
					trObjAddr			= MEMtPN8(MapTable_GetEntryAddr(i));
					ObjectInternalType	= U8_AND_U8(get_object_header_type(trObjAddr), OBJ_HDR_OBJECT_TYPE_MASK);
					if (Header_ArrayType_Reference == ObjectInternalType)
					{
						trObjAddr		= PN8_A_CONST(trObjAddr, OA_DataArray_START);
					}
					else
					{
						trObjAddr		= PN8_A_CONST(trObjAddr, PA_DataArray_START);
					}
					tempRamPtr			= tranDeallocate_getTrObjNodeStart(MEMtPX8(trObjNode));
					freeBlockSize		= U32t16(FreeTable_Compare(MEMtPM8(tempRamPtr), maxFreeEntryNum, true));
					if(0 != freeBlockSize)
					{
						tempRamPtr		= U16_S_U16(tempRamPtr, freeBlockSize);
						nvmWriteU16NoAtomic(trObjAddr, tempRamPtr);
					}
				}
			}
		}
	}
}

void tranDeallocate_moveEntity(pFreeInfo freeBlockInfo)
{
	u16				D_A		moveLength;
	u16				D_A		ramSpaceStart;
	PX8						copyDes;
	PX8						copySrc;

	ramSpaceStart	= tranDeallocate_getRamSpaceStart(U16_SHL_U8(U8t16(tranDeallocate_GetDealloCtxId()), 8), tranDeallocate_GetDealloTranType());
	
	if(ILLEGAL_ADDRESS != ramSpaceStart)
	{
		copyDes			= MEMtPX8(RamManager_getAlloRamBase(U16_A_U16(ramSpaceStart, U32t16(U32_S_U32(PMtMEM(freeBlockInfo->curFreeBlockStart), U16_S_U16(freeBlockInfo->cumulateSize, freeBlockInfo->curFreeBlockSize))))));
		copySrc			= MEMtPX8(RamManager_getAlloRamBase(U16_A_U16(ramSpaceStart, U32t16(U32_A_U32(PMtMEM(freeBlockInfo->curFreeBlockStart), freeBlockInfo->curFreeBlockSize)))));
		
		moveLength		= U32t16(U32_S_U32(PMtMEM(freeBlockInfo->nextFreeBlockStart), PMtMEM(freeBlockInfo->curFreeBlockStart)));
		moveLength		= U16_S_U16(moveLength, U32t16(freeBlockInfo->curFreeBlockSize));
		
		ecpMem_Copy(copyDes, copySrc, moveLength);
	}
}

void tranDeallocate_compactRam(u8 compactType)
{
	u8				D_A			ctxId;	
	u8				D_A			rtrMask;
	u8				D_A			dtrMask;
	u8				D_A			dtrFlag ;
	u16 			D_A			result;
	u8				I_A			temp[dTP_NUM];
	u8				D_A			j;
	u8				D_A			i;
	u8				D_A			spaceId;
	u16				D_A			tempRamPtr;
	u16				D_A			temPtr;
#if GP__CGM__FOR_MASK
	u16				D_A			backFlag;
#endif
    u8              D_A			rtrReFlag;
	dtrFlag			= 0;
	ctxId			= 0;
	
	ecpMem_Copy(temp, MEMtPN8(JcvmConfig_GetDtpObjectDeletedBase()), dTP_NUM);
	
	rtrMask 	= JcvmConfig_GetRtpObjectDeleted();

	#define			APP_MASK			0x01

#if GP__CGM__FOR_MASK
	//RAM碎片整理时关闭CGM
	backFlag = g_bCheckFlag;
	CgmInit_changeFlag(NO_CHECK);
#endif
	rtrReFlag=0;
	for(j = 0; j< dTP_NUM; U8_INC(j))
	{
		dtrMask = temp[j];
		if (0 != dtrMask)
		{
			i	= U16t8(U8_M_U8(8, U8_A_U8(j, 1)));
			for (ctxId = U16t8(U8_M_U8(8, j)); ctxId < i; U8_INC(ctxId))
			{
				if(U8_AND_U8(U8_SHR_U8(dtrMask, U8_S_U8(ctxId, U8_M_U8(8, j))), APP_MASK))
				{
					tranDeallocate_SetDealloCtxId(ctxId);
					
					if(COMPACT_SPACE == compactType)
					{
						tranDeallocate_SetDealloAppHandleType(OBJ_HDR_TransientMode_DTR, RECORD_DTR_OBJ);
						temPtr		= tranMananger_getDtpByCtxId(ctxId);
						FreeTable_InitTranCompact(temPtr);
						temPtr		= U16_S_U16(temPtr, FreeTable_RoomArrange(false, false, false));
						spaceId  	= dtrSpace_searchExistDtrCtx(U16_SHL_U8(U8t16(ctxId), 8));
						if(INVALID_SPACE_ID !=  spaceId)
						{
							result = RamManager_reAlloc(MEMtPX8(dtrSpace_getDtrSpaceEntry(spaceId)), temPtr);
							if (result == NO_MEMORY_AVAILABLE)
							{
								throw_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
							}
						}
						tranMananger_setDtpByCtxId(ctxId, temPtr);
					}
					else
					{
						tranDeallocate_SetDealloAppHandleType(OBJ_HDR_TransientMode_DTR, RE_ALLOCATE_DTR_OBJ);
						tempRamPtr		= tranDeallocate_ReAlLocate(ILLEGAL_PROFILEINDEX);
						tranMananger_setDtpByCtxId(ctxId, tempRamPtr);
						#if (EUICC_ONLY||SE_AND_EUICC)
						if(ILLEGAL_PROFILEINDEX != G_CurEnabledProfileIndex)
						{
							tempRamPtr		= tranDeallocate_ReAlLocate(G_CurEnabledProfileIndex);
							tranMananger_setDtpByCtxId(ctxId, tempRamPtr);
						}
						#endif
					}
				}
			}
			dtrFlag			= 1;
		}
	}
	
   	if(0 != rtrMask)
	{
		tranDeallocate_SetDealloCtxId(ctxId);

	 	if(COMPACT_SPACE == compactType)
		{
			tranDeallocate_SetDealloAppHandleType(OBJ_HDR_TransientMode_RTR, RECORD_RTR_OBJ);
			temPtr		= tranMananger_getRtp();
			FreeTable_InitTranCompact(temPtr);
			temPtr		= U16_S_U16(temPtr, FreeTable_RoomArrange(false, false, false));

			spaceId  	= RtrSpace_searchExistRtrID();
			result = RamManager_reAlloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(spaceId)),temPtr);

			//result = RamManager_reAlloc(MEMtPX8(&g_rtrStart), temPtr);
			if (result == NO_MEMORY_AVAILABLE)
			{
				throw_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
			}

			tranMananger_setRtp(temPtr);
		}
		else
		{
			rtrReFlag=1;
			tranDeallocate_SetDealloAppHandleType(OBJ_HDR_TransientMode_RTR, RE_ALLOCATE_RTR_OBJ);
			tempRamPtr		= tranDeallocate_ReAlLocate(ILLEGAL_PROFILEINDEX);//SE区的RTR整理
			tranMananger_setRtp(tempRamPtr);
			tranMananger_initSERtp();
#if (EUICC_ONLY||SE_AND_EUICC)
			if(ILLEGAL_PROFILEINDEX != G_CurEnabledProfileIndex)
			{
				tempRamPtr		= tranDeallocate_ReAlLocate(G_CurEnabledProfileIndex);
				tranMananger_setRtp(tempRamPtr);
				tranMananger_initProfileRtp();
			}
#endif
		}
	}
	
	tranDeallocate_finishCompactTrObj(dtrFlag, rtrMask);
	//from  init_ram to this modify @20230529
	if(RE_ALLOCATE_RAM_SPACE == compactType)
	{
		if(rtrReFlag==0)
		{
			tranMananger_initSERtp();
			tranMananger_initProfileRtp();
		}
		//tranMananger_initRtp();
	}
	
	if(compactType == RE_ALLOCATE_RAM_SPACE)//只有在上电的情况下才调tranMananger_getGlobalBuf，否则会导致碎片整理时APDU头被还原，测试用例：个人esim测试脚本-移动测试版 - EPX/RAM_RFM测试/RAM_TEST/移动RAM测试(3G)/AppletManagementBehaviour_00  步骤24：load过程未结束 ，下发80C2XXXXXXXXX80E6XXXX 通过RAM的方式安装应用 ，将LOAD打断，将未下载完成的PKG释放的时候 进行了DTR,RTR整理  ，调用该函数会将APDUBuffer头重新赋值成80C2XXXX ，执行installforinatall时报错。
	{
		tranMananger_getGlobalBuf();
	}

#if GP__CGM__FOR_MASK
	CgmInit_changeFlag(U16t8(backFlag));
#endif
}

#endif
