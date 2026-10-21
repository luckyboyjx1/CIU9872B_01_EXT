/*
 * cgmLogicManager.c
 *
 *  Created on: 2015年10月21日
 *      Author: liuhanren
 */

#include "CgmDel.h"
#include "CgmInstall.h"
#include "CgmLoad.h"
#include "CgmManager.h"
#include "CgmRun.h"
#include "cardManager.h"
#include "gc.h"
#include "tranAllocate.h"
#include "BootLifeCycle.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "Interface_LTEFromPlatform.h"

#if GP__CGM__FOR_MASK


u32  CgmManager_getRamConsume(u8 cgmFlag, u16 appId, PN8 cgmArrayAddr)
{
	u32			sD7Value;
	u32 			maxRam;

	maxRam	= 0;

	CgmManager_calaConsumeSpace(appId, MEM_NULL, &maxRam, false, CGM_GET_RAM);
	if(U8_AND_U8(cgmFlag, HASD7_MASK))
	{
		sD7Value = CgmField_getD7(cgmArrayAddr, cgmFlag);
		if(sD7Value > maxRam)
		{
			maxRam = sD7Value;
		}
	}
	return maxRam;
}

u32  CgmManager_getNvmConsume(u8 cgmFlag, u16 appId, PN8 cgmArrayAddr)
{
	u32 			maxNvm;
	u32				sD8Value;

	maxNvm			= 0;
	CgmManager_calaConsumeSpace(appId, &maxNvm, MEM_NULL, false, CGM_GET_NAM);
	if(U8_AND_U8(cgmFlag, HASD8_MASK))
	{
		sD8Value = CgmField_getD8(cgmArrayAddr,cgmFlag);
		if(sD8Value > maxNvm)
		{
			maxNvm = sD8Value;
		}
	}
	return maxNvm;
}



void CgmManager_SetSdMenInfo(u8 sdId,u32 cgmRam,u32 cgmNvm, u8 manageType)
{
	u8 				type;
	u8 				isExtraditionOneself;

	isExtraditionOneself = 0;

	if(sdId == ILLEGAL_ID)
	{
		CgmManager_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	type 				 = U8_AND_U8(manageType, SD_EXTRADITION_ONESELF_MASK);
	isExtraditionOneself = U8_AND_U8(manageType, SD_EXTRADITION_ONESELF);

	CgmManager_manageSdMenSize(sdId, cgmRam, cgmNvm, type, isExtraditionOneself);
	return;
}

void CgmManager_Alloc(u32 pSize, u8 cgm_mask)
{
	if(g_bCheckFlag != NO_CHECK && g_bCheckFlag != CHECK_CLOSE)
	{

		if(g_bCheckFlag == CHECK_LOAD)
		{
			CgmLoad_ConsumeCumulate(pSize, cgm_mask);
		}
		else if(g_bCheckFlag == CHECK_INSTALL)
		{
			CgmInstall_ConsumeCumulate(pSize, cgm_mask);
		}
		//计算安装应用或者辅助安全域时的系统消耗
		else if(g_bCheckFlag == CHECK_INSTALL_SYSTEM_CONSUME)
		{
			//TODO:计算到ISD底下
		}
		else	//RUN TIME
		{
			CgmRun_ConsumeCumulate(pSize, cgm_mask);
		}
	}
	return;
}

void CgmManager_Free(u32 chunkSize, u8 cgm_mask)
{
	//LOAD过程中申请的空间没有计入CGM，所以释放时也不需要处理
	if(g_bCheckFlag == CHECK_GC)
	{
		if(U8_AND_U8(cgm_mask, HAS82_MASK))
		{
			if(g_UsedRAM < chunkSize)
			{
				g_UsedRAM = 0;
			}
			else
			{
				g_UsedRAM = U32_S_U32(g_UsedRAM, chunkSize);
			}
			g_bRoomChanged = U8_OR_U8(g_bRoomChanged, RAM_SIZE_CHANGED);
		}
		else
		{
			//根据配置的开关计算系统消耗，00为系统消耗算由SD消耗，01为ISD消耗
//			if(g_systemConsumeConfig != 0x00)
//			{
//				//由于释放NVM时，chunkSize本身就多加了4字节MAP表项长度，此处扣出来加回给ISD
//				chunkSize = U16_S_U8(chunkSize, MAP_ENTRY_SIZE);
//			}

			if(g_UsedNVM < chunkSize)	//防卡死掉处理
			{
				g_UsedNVM = 0;
			}
			else
			{
				g_UsedNVM = U32_S_U16(g_UsedNVM, chunkSize);
			}
			g_bRoomChanged = U8_OR_U8(g_bRoomChanged, NVM_SIZE_CHANGED);
		}
	}
	return;
}

/*
*	现阶段8283一般是同时存在或不存在，但为了以后可扩展性，这里任然把8283
*	分开处理。
* 返回值： true -- 正常结束
*          false -- 抛异常了
* modify by xuxiaojun@20220427 根据分层代码 第一次代码评审(20220415).xlsx 将GetFileSystemTotalSize()宏定义为0
*/
void CgmManager_manageSdMenSize(u8 sdId, u32 cgmRam, u32 cgmNvm, u8 type, boolean isExtraditionOneself)
{
	u8 						cgmFlag;
	u8 						CgmRamOffset;
	u8						CgmNvmOffset;
	u8 						sdId_temp;
	u8  					cgm_mask;
	u32 					remainCGMram;
	u32						remainCGMnvm;
	boolean 				isCgmNvmFind;
	boolean 				isCgmRamFind;
	PN8 					sdEntryAddr;
	PN8 					cgmRamArrayAddr;
	PN8 					cgmNvmArrayAddr;
	u8 						sdId_itself;
	u32                     fileSize = 0;
	if(g_bCheckFlag == CHECK_CLOSE)
	{
		return;
	}

	cgm_mask 				= 0;
	sdId_itself				= 0;
	isCgmNvmFind 			= false;
	isCgmRamFind 			= false;

	if(isExtraditionOneself)  //自引渡
	{
		sdId 	= 	APP_FIRST;   //消耗ISD CGM
	}
	getAppTable();
	sdEntryAddr = MEMtPN8(getSdEntryObj(sdId));
	if(MEM_NULL == sdEntryAddr)
	{
		return;	//在应用entry被删除前出现了应用申请空间的操作，导致应用entry被删除后找不到entry，由于entry已被删除，就不需要更改CGM内容。例如CRS应用的删除。
	}

	cgmFlag 	= CgmField_getCGMFlag(sdEntryAddr);
	cgm_mask 	= U8_A_U8(HAS82_MASK, HAS83_MASK);
	while(cgm_mask)
	{
		while(!U8_AND_U8(cgmFlag, cgm_mask))
		{
			sdId_temp    = getAppAssociatedSdId(sdEntryAddr);
			if(sdId_temp == sdId)
			{
				/* 进入这里说明sdId代表的SD为自关联，且不存在82tag(进入wihle循环已判断)，因此可以直接使用ISD的 */
				sdId   	 = 0;
				sdId_itself = 1;
			}
			else
			{
				sdId   	 = sdId_temp;
			}

			sdEntryAddr  = MEMtPN8(getSdEntryObj(sdId));
			cgmFlag   	 = CgmField_getCGMFlag(sdEntryAddr);
			if((sdId_itself == 1))
			{
				if(cgmFlag==0)
				return;
			}
		}


		if((HAS82_MASK == U8_AND_U8(cgmFlag, HAS82_MASK))&&(!isCgmRamFind))
		{
			cgmRamArrayAddr = MEMtPN8(CgmField_getArrayObj(sdEntryAddr));
			if(cgmRamArrayAddr==MEM_NULL)
			{
				return;
			}
			CgmRamOffset 	= CgmField_getCGMarrayOffset(cgmFlag,HAS82_MASK);
			cgm_mask 		= U8_S_U8(cgm_mask, HAS82_MASK);
			isCgmRamFind 	= true;
		}
		if((HAS83_MASK == U8_AND_U8(cgmFlag, HAS83_MASK))&&(!isCgmNvmFind))
		{
			cgmNvmArrayAddr = MEMtPN8(CgmField_getArrayObj(sdEntryAddr));
			if(cgmNvmArrayAddr==MEM_NULL)
			{
				return;
			}
			CgmNvmOffset 	= CgmField_getCGMarrayOffset(cgmFlag,HAS83_MASK);
			cgm_mask 		= U8_S_U8(cgm_mask, HAS83_MASK);
			isCgmNvmFind 	= true;
//revised by weipingjiao@202209201、去掉CGM管理中获取文件大小的宏开关管控，因为在EUICC中CGM功能会关掉，保证当不下载profile时，EUICC平台下CGM功能完整。
//#if SE_ONLY  //SE_ONLY时  ISD的CGM可作为卡片CGM ，真正可用的CGM需要减去文件系统空间 ，为了减少修改文件系统时频繁写E2修改CGM，在这里判断真正的可用空间 ，带EUICC功能时 每个Profile下都有一个ISD ，需要重新考虑CGM方案
			if((sdId == APP_FIRST)&& (JcvmConfig_GetSEState() > CARD_OP_READY))
			{
				fileSize = GetFileSystemTotalSize(); 
			}
//#endif
		}
	}

	remainCGMram = CgmField_getNvmSizeFromCGMarray(cgmRamArrayAddr, CgmRamOffset);
	remainCGMnvm = CgmField_getNvmSizeFromCGMarray(cgmNvmArrayAddr, CgmNvmOffset);


	if (SD_INCREASE_SIZE == type)
	{
		remainCGMram 	 = U32_A_U32(remainCGMram, cgmRam);
		remainCGMnvm 	 = U32_A_U32(remainCGMnvm, cgmNvm);
	}
	else
	{
		if (remainCGMram < cgmRam)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
		}
		if(U32_S_U32(remainCGMnvm,fileSize) < cgmNvm)// if(remainCGMnvm < cgmNvm)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_RESOURCE);
		}
		remainCGMram 	= U32_S_U32(remainCGMram, cgmRam);
		remainCGMnvm 	= U32_S_U32(remainCGMnvm, cgmNvm);
	}

	nvmWriteU32Atomic(MEMtPN8(U32_A_U8(PNtMEM(cgmRamArrayAddr), CgmRamOffset)), remainCGMram);
	nvmWriteU32Atomic(MEMtPN8(U32_A_U8(PNtMEM(cgmNvmArrayAddr), CgmNvmOffset)), remainCGMnvm);
	return;
}

static void CgmManager_SetChangeSize(u32 reserve,u32 oldConsume,u32 newConsume,u8 cgm_mask, u16 backupCC)
{

	u8 					appId;
	u8 					sdId;
	u32 				tempRoom;
	u32					changedRoom;
	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	memref appEntryObj;
	#endif

	appId 				= U16t8(U16_AND_U16(backupCC, APPLET_MASK));

	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	appEntryObj = getSdEntryObj(appId);
	sdId = getAppAssociatedSdId(appEntryObj);
	#else
	sdId 				= appId;
	#endif

	if(newConsume > oldConsume)
	{
		if(newConsume > reserve)
		{
			tempRoom 	 = oldConsume;
			if(reserve > oldConsume)
			{
				tempRoom = reserve;
			}
			changedRoom  = U32_S_U32(newConsume, tempRoom);
			if(U8_AND_U8(cgm_mask, HAS82_MASK) == HAS82_MASK)
			{
				CgmManager_manageSdMenSize(sdId, (changedRoom), 0, SD_DECREASE_SIZE, false);
			}
			else
			{
				CgmManager_manageSdMenSize(sdId, 0, changedRoom, SD_DECREASE_SIZE, false);
			}
		}
	}
	else
	{
		if(oldConsume > reserve)
		{
			tempRoom 	 = newConsume;
			if(reserve > newConsume)
			{
				tempRoom = reserve;
			}
			changedRoom  = U32_S_U32(oldConsume, tempRoom);
			if(U8_AND_U8(cgm_mask, HAS82_MASK) == HAS82_MASK)
			{
				CgmManager_manageSdMenSize(sdId, (changedRoom), 0, SD_INCREASE_SIZE, false);
			}
			else
			{
				CgmManager_manageSdMenSize(sdId, 0, changedRoom, SD_INCREASE_SIZE, false);
			}
		}
	}
}

void CgmManager_RamChange(u16 context, u32 newRamValue, u32 newSystemRamValue)
{
	CgmManager_SetChangeSize(g_sD7Value, g_OldUsedRAM, newRamValue, HAS82_MASK, context);
}

void CgmManager_NvmChange(u16 context, u32 newNvmValue, u32 newSystemNvmValue)
{
	CgmManager_SetChangeSize(g_sD8Value, g_OldUsedNVM, newNvmValue, HAS83_MASK, context);
}

#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
memref getPkgInfoAddr(u8 pkgId)
{
	pkgInfoc6c7c8* INFOPtr;
	u8 i;

	if(pkgId < NPACKAGES)
	{
		/*mask in pkg*/
		return REF_NULL;
	}

	INFOPtr = (pkgInfoc6c7c8 *)PKG_CGM_INFO_ARRAY_BASE;
	for(i = 0; i < maxLoadPkgNum; i++)
	{
		if(getRecord_LoadPkgId(INFOPtr) != pkgId)
		{
			INFOPtr++;
		}
		else
		{
			break;
		}
	}
	if(i == maxLoadPkgNum)
	{
		CgmManager_error(REFERENCED_DATA_NOT_FOUND);
	}
	return (memref)INFOPtr;
}

void getLoadFileInfo(u8 pkgId,u8 *buf)
{
	pkgInfoc6c7c8* INFOPtr;
	u16 c6;
	u16 c7;
	u16 c8;
	u8 offset = 0;

	INFOPtr = (pkgInfoc6c7c8 *)getPkgInfoAddr(pkgId);

	if(INFOPtr != REF_NULL)
	{
		c6 = getRecord_LoadC6(INFOPtr);
		c7 = getRecord_LoadC7(INFOPtr);
		c8 = getRecord_LoadC8(INFOPtr);
	}
	else
	{
		c6 = 0;
		c7 = 0;
		c8 = 0;
	}

	buf[offset++] = c6 >> 8;
	buf[offset++] = (u8)c6;
	buf[offset++] = c7 >> 8;
	buf[offset++] = (u8)c7;
	buf[offset++] = c8 >> 8;
	buf[offset++] = (u8)c8;

	return;
}

void SetRecordOfMemNull(pkgInfoc6c7c8 * INFOPtr)
{
	u8 i;
	//pkgInfoc6c7c8 * INFOPtr;

	//INFOPtr = (pkgInfoc6c7c8 *)getPkgInfoAddr(pkgId);
	setRecord_LoadPkgId(INFOPtr, 0XFF);

	return;
}

void writeRecord(pkgInfoc6c7c8 *pPkgInfo)
{
	pkgInfoc6c7c8 * INFOPtr;
	u8 i;
	INFOPtr = (pkgInfoc6c7c8 *)PKG_CGM_INFO_ARRAY_BASE;
	for(i = 0; i < maxLoadPkgNum; i++)
	{
		if(getRecord_LoadPkgId(INFOPtr) != 0xFF)
		{
			INFOPtr++;
		}
		else
		{
			break;
		}
	}
	if(i == maxLoadPkgNum)
	{
		CgmManager_error(REFERENCED_DATA_NOT_FOUND);
	}

	if((pPkgInfo->loadC6 + pPkgInfo->loadC8) < g_iUsedNVM)//表示不带C6C7C8
	{
		pPkgInfo->loadC6 = g_iUsedNVM;
		pPkgInfo->loadC8 = 0;
		pPkgInfo->loadC7 = 0;
	}
	setRecord_LoadPkgId(INFOPtr, pPkgInfo->pkgId);
	setRecord_LoadC6(INFOPtr, pPkgInfo->loadC6);
	setRecord_LoadC7(INFOPtr, pPkgInfo->loadC7);
	setRecord_LoadC8(INFOPtr, pPkgInfo->loadC8);
	return;
}

void initiateMemInfoArray()
{
	nvmFillAtomic(PKG_CGM_INFO_ARRAY_BASE,totalSizeOfPkgCgmArray,0XFF);
	return;
}

u16 handlRoomOfGetAppRam(memref appEntryAddr)
{
	u8  		cgmFlag;
	u16 		ramSize;
	memref cgmArrayObj;

	cgmFlag = CgmField_getCGMFlag(appEntryAddr);
	if((HASD7_MASK & cgmFlag) && (!OS_C7C8_NOCHECK))
	{
		cgmArrayObj = (memref)CgmField_getArrayObj(appEntryAddr);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		ramSize 		= CgmField_getD7(cgmArrayObj, cgmFlag);
#else
		ramSize 		= GProomManager_getC7(cgmArrayObj, cgmFlag);
#endif
	}
	else
	{
		ramSize = GProomManager_getAppUsedRam(appEntryAddr);
	}
	return ramSize;
}

u32 handlRoomOfGetAppNvm(memref appEntryAddr)
{
	u8  cgmFlag;
	u32 nvmSize;
	u32 cgmArrayObj;

	cgmFlag = CgmField_getCGMFlag(appEntryAddr);
	if((HASD8_MASK & cgmFlag) && (!OS_C7C8_NOCHECK))
	{
		cgmArrayObj = (memref)CgmField_getArrayObj(appEntryAddr);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		nvmSize		  = CgmField_getD8(cgmArrayObj, cgmFlag);
#else
		nvmSize		  = GProomManager_getC8(cgmArrayObj, cgmFlag);
#endif
	}
	else
	{
		nvmSize = GProomManager_getAppUsedNvm(appEntryAddr);
	}
	return nvmSize;
}

#endif //CHINA_MOBILE_DESIGNATED_REQUIREMENT
/**
*modify by weipingjiao@20210425 删除5G_GC的代码，EPX产业化\INFENION_SLM97\Develop
*/
void CGM_NVMObjResize(u16 entry_index,u16 context)
{
	u32			D_A		 objSize;
	u16					  appIndex;
	jref				  objectId;
	u8 					  bsdId;
	memref 		X_A 	 APP_Entry_Object_NVM_Cash;
	if(g_bCheckFlag == CHECK_CLOSE)
	{
		return;
	}
	
	objSize=0;
	CgmManager_getSizeByObjId(entry_index, &objSize, MEM_NULL, CGM_GET_NAM);
	appIndex			  = getIndexByID(U16t8(U16_AND_U16(context, APPLET_MASK)));
	if(0xFFFF == appIndex)
	{
		return;
	}
//	if(appIndex > 0)
//	{
	objectId			  = nvmReadU16(MEMtPN8(U32_A_U16(APP_Table_Cash, U16_M_U16(appIndex, SIZE_REF))));

	APP_Entry_Object_NVM_Cash = locateObjectInMemory(objectId);
	bsdId = getAppAssociatedSdId(MEMtPF8(APP_Entry_Object_NVM_Cash));

	CgmManager_manageSdMenSize(bsdId, 0, objSize, SD_INCREASE_SIZE, false);

//	}


}
#endif
#if GP__CGM__FOR_APPPARACHECK
void  CgmManager_getSizeByObjId(u16 objectId, PX32 nvmSize, PX32 ramSize, u8 getType)
{
	u8			D_A		obType;
	u16			D_A		count;
	u32			D_A		size;
	u32 		D_A		eeSize;
	PN8			D_A		objectAddr;

	size		= 0;
	objectAddr	= MEMtPN8(MapTable_GetEntryAddr(objectId));
	obType		= PN_R_U8(objectAddr);

	if (U8_AND_U8(obType, HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
	{
			count	= get_array_length(PNtMEM(objectAddr));

			if(NULL_POINTER != nvmSize)
			{
				eeSize		= U8t32(U8_A_U8((TYPE(obType) == GC_TRANSIENT_REFERENCE_ARRAY ?
															SIZE_OBJECT_ARRAY_HEADER : SIZE_PRIMITIVE_ARRAY_HEADER) /* size of array header */
																						, sizeof(jref)));
				eeSize		= CgmField_AlignSize(eeSize);
				nvmSize[0]	= eeSize;
			}

			if(U8_AND_U8(HEADER_JCRE_TransMode_Deselect, obType) != HEADER_JCRE_TransMode_Deselect)
			{
				switch (U8_AND_U8(obType, OBJ_HDR_OBJECT_TYPE_MASK))
				{
					case Header_ArrayType_Byte:
					case Header_ArrayType_Boolean:
						size = 1;
						break;
					case Header_ArrayType_Short:
					case Header_ArrayType_Reference:
						size = 2;
						break;
					case Header_ArrayType_Int:
						size	= SIZE_INT;
						break;
					default:;
				}

				if(NULL_POINTER != ramSize)
				{
					ramSize[0] = U16_M_U32(count, size);
				}
			}
	}
	else
	{
		if(getType != CGM_GET_RAM)
		{
			if((objectAddr != NULL_POINTER) && nvmSize != NULL_POINTER)
			{
				size = CgmField_AlignSize(getObjectSize(objectAddr));
				nvmSize[0] = size;
			}
		}
	}

	return ;
}

void  CgmManager_calaConsumeSpace(u16 Id, PX32 nvmSize, PX32 ramSize, u8 isPkg, u8 type)
{
	u16					objectId;
	u16					tempId;
	u16					max_entry_count;
	u32					nvm;
	u32					ram;
	u32					entryAdd;

	nvm					= 0;
	ram					= 0;
	entryAdd			= MEM_NULL;
	max_entry_count		= MapTable_GetTopIndex();

	for (objectId = JcvmConfig_GetJcreEntryEnd(); objectId < max_entry_count; U16_INC(objectId))
	{
		#if (EUICC_ONLY||SE_AND_EUICC)
		if(MapTable_IsCurrentProfile(objectId, G_CurExecutingProfileIndex))
		#endif
		{
			tempId		= U8t16(MapTable_GetEntryPckId(objectId));
			if(isPkg>0)
			{
				;
			}
			else
			{
				if(MAP_ENTRY_OBJECT_PCK_ID == tempId) // an object//modify by weipingjiao移动测试脚本47.uicc_configuration_additional_test-不同大小的应用重复下载安装删除的测试，在重复测试该用例时，会随机测出问题，其中的一个脚本AppLoadTestLoopLog_1_1在统计空间时会多出0x10字节
				//if(U16_AND_U16(tempId , MAP_ENTRY_OBJECT_PCK_ID)) // an object
				{
					entryAdd	= MapTable_GetEntryAddr(objectId);

					if(MEM_NULL != entryAdd)
					{
						tempId		= get_object_owner(entryAdd);
					}
					else
					{
						tempId 		= 0xFFFF;
					}
				}
				else// just reference
				{
					tempId 		= 0xFFFF;
				}
			}

			if((Id != tempId) || (MAP_TABLE_ENTRY_STATUS_CREATION != MapTable_GetEntryStatus(objectId)))
			{
				continue;
			}
			else
			{
				CgmManager_getSizeByObjId(objectId, &nvm, &ram, type);
				if(nvmSize != MEM_NULL)
				{
					nvmSize[0]		= U32_A_U32(nvmSize[0], nvm);
					nvm				= 0;
				}

				if(ramSize != MEM_NULL)
				{
					ramSize[0]		= U32_A_U32(ramSize[0], ram);
					ram				= 0;
				}
			}
		}
	}

	return	;
}

void CgmManager_error(u16 errorcode)
{
	g_bRoomChanged = NO_CHANGED;
	throw_error(errorcode);
}

void CgmManager_Main(void)
{
	u8  flag;
	u16 destId;
	u16 srcId;
	u16 entryIndex;
	u8 isMNOSDflag;

	flag       = U16t8(popShort());
	destId     = popShort();
	srcId      = popShort();
	entryIndex = popReference();
	isMNOSDflag =U16t8(popShort());

	/* do not back up any data when creating cardmanager.if tearing in the cardmanager creating, clear all eeprom*/
	if (BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)
//	if(JcvmConfig_GetCardState() < CARD_SECURED)
	{
		return;
	}
	#if GP__CGM__FOR_MASK
	if(g_bCheckFlag != CHECK_CLOSE)
	{
		switch(U8_AND_U8(flag, U8_INV(ROOM_MANAGE_OF_EXTRADITION_PKG)))
		{
			case ROOM_MANAGE_OF_LOAD:
				CgmLoad_End(entryIndex);
				break;
			case ROOM_MANAGE_OF_INSTALL:
				CgmInstall_End(U16t8(srcId), destId, isMNOSDflag);
				break;
			case ROOM_MANAGE_OF_EXTRADITION:
				CgmRun_Extradition(entryIndex, srcId, destId, flag);
				break;
			case ROOM_MANAGE_OF_REGISTRY_UPDATE:
				CgmRun_RegistryUpdate(entryIndex, srcId, destId);
				break;
			case ROOM_MANAGE_OF_REMOVE_APPLET:
				cgmDel_App(U16t8(destId));
				break;
			case ROOM_MANAGE_OF_REMOVE_PACKAGE:
				cgmDel_Pkg(U16t8(srcId), destId);
				break;
			case ROOM_MANAGE_OF_CHECK_LOAD:
				CgmLoad_LimitCheck(U16t8(destId));
				break;
			default:
				break;
	  }
  }
	else
		#endif
	{

		switch(U8_AND_U8(flag, U8_INV(ROOM_MANAGE_OF_EXTRADITION_PKG)))
		{
			case ROOM_MANAGE_OF_INSTALL:
				CgmInstall_LimitCheck(U16t8(srcId), destId,isMNOSDflag);
				break;
			case ROOM_MANAGE_OF_CHECK_LOAD:
				CgmLoad_LimitCheck(U16t8(destId));
				break;
			default:
				break;


		}
	}
	return;
}
/**
*added by jiangxiaohui@20210615: 优化性能，EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*/
void getCGMarraySize(void) 
{
	pushShort(getCGMarraySizeImpl(popShort()));
}
/**
*added by jiangxiaohui@20210615: 优化性能，EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop

*modify by jiangqingbin-20211216, 所有CGM参数修改为4字节存储
*/
u8 getCGMarraySizeImpl(u8 flag) 
{
	u8 lenth = 0;
	
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		lenth = 22;
#else
	if ((flag & HASD7_MASK) == HASD7_MASK) 	//reserveRAM(D7) 
	{
		lenth += CGM_Mask_PerPara_Len;
	}
	if ((flag & HASD8_MASK) == HASD8_MASK)	//reserveNVM(D8)
	{
		lenth += CGM_Mask_PerPara_Len;
	}
	if ((flag & HAS82_MASK) == HAS82_MASK) 	//maxCGMram(82) remainCGMram(82)
	{
		lenth += 2*CGM_Mask_PerPara_Len; //4+4	//双倍长度是因为82有两个值，一个是最大值，一个是剩余值
	}
	if ((flag & HAS83_MASK) == HAS83_MASK) 	//maxCGMnvm(83) remainCGMnvm(83)
	{
		lenth += 2*CGM_Mask_PerPara_Len; //4+4	//双倍长度是因为83有两个值，一个是最大值，一个是剩余值
	}
	if ((flag & HASC8_MASK) == HASC8_MASK) //quotaNVM(C8)
	{
		lenth += CGM_Mask_PerPara_Len;
	}
	if ((flag & HASC7_MASK) == HASC7_MASK) //quotaRAM(C7)
	{
		lenth += CGM_Mask_PerPara_Len;
	}
#endif
	return lenth;
}

#endif
