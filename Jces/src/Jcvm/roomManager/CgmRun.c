/*
 * cgmRumTime.c
 *
 *  Created on: 2015年10月21日
 *      Author: liuhanren
 */
#include "CgmManager.h"
#include "CgmRun.h"
#include "cardManager.h"

#if (GP__CGM__FOR_MASK&&GP__CGM__FOR_APPPARACHECK)

void CgmRun_ChangeCC(u16 context)
{
	if(context != g_backupCC)
	{
		CgmRun_setConsumeInfo();

		CgmField_getEntryInfo(context);
		g_backupCC = context;
	}
}

void CgmRun_CommitChangeSize(u16 backupCC, u32 RamChange, u32 NvmChange)
{
	u16		bContext;

	//linguoming:config C7C8
	bContext	= backupCC;

	if(g_bCheckFlag != CHECK_CLOSE)
	{
		//在RUN TIME下，对关联SD的CGM管理，在CC改变和一条指令结束时才进行
		if(g_bRoomChanged & RAM_SIZE_CHANGED)
		{
			CgmManager_RamChange(bContext, RamChange, 0);
		}
		if(g_bRoomChanged & NVM_SIZE_CHANGED)
		{
			CgmManager_NvmChange(bContext, NvmChange, 0);
		}
	}
}

void CgmRun_setConsumeInfo(void)
{
	CgmRun_CommitChangeSize(g_backupCC, g_UsedRAM, g_UsedNVM);

	if(g_bCheckFlag != CHECK_CLOSE)
	{
		if((g_bRoomChanged & RAM_SIZE_CHANGED) && (g_UsedRAM != 0))
		{
			g_UsedRAM = 0;
		}

		if((g_bRoomChanged & NVM_SIZE_CHANGED) && (g_UsedNVM != 0))
		{
			g_UsedNVM = 0;
		}
		g_OldUsedRAM		   =	0;
		g_OldUsedNVM	       = 	0;
		g_bRoomChanged	= NO_CHANGED;
		g_backupCC 		= NULL_CONTEXT;
	}
}

void CgmRun_ConsumeCumulate(u32 pSize, u8 cgm_mask)
{
	u32 		consume;
	u32			quota;

	CgmRun_ChangeCC(cc);

	if(U8_AND_U8(cgm_mask, HAS82_MASK))
	{
		consume = g_UsedRAM;
		quota = g_sC7Value;
	}
	else
	{
		consume = g_UsedNVM;
		quota = g_sC8Value;

	}
	if((U32_A_U32(consume, pSize) > quota)
#if	 !UICC__OPERATOR__CHINAMOBILE   //移动 C7  D7 值为0时 按照0处理 ，其余运营商按照FFFF（不检查）处理  GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
		 && (quota != 0)
#endif
	     )
	{
		if(cgm_mask & HAS82_MASK)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
		}
		else
		{
			CgmManager_error(SYSTEMEXCEPT_NO_RESOURCE);
		}
	}

	if(U8_AND_U8(cgm_mask, HAS82_MASK))
	{
		g_UsedRAM = U32_A_U32(g_UsedRAM,pSize);
		g_bRoomChanged |= RAM_SIZE_CHANGED;
	}
	else
	{
		g_UsedNVM = U32_A_U32(g_UsedNVM,pSize);
		g_bRoomChanged |= NVM_SIZE_CHANGED;
	}

}

void CgmRun_Extradition(u16 entryIndex,u8 srcId,u8 destSdId,u8 flag)
{
	u8  					cgmFlag;
	u8						appId;
	u8  					extraditionFlag;
	u8  					srcSdId;
	u8						pkgId;
	u8 						isExtraditionPkg;
	u16						context;
	u32 					maxRam;
	u32 					maxNvm;
	u16 					appEntry;
	PN8 					EntryAddr;
	PN8 					cgmArrayAddr;

	pkgId 					= 0;
	maxRam 					= 0;
	maxNvm					= 0;
	srcSdId 				= 0;
	extraditionFlag 		= 0;
	isExtraditionPkg    	= U8_AND_U8(flag, ROOM_MANAGE_OF_EXTRADITION_PKG);
	appEntry 				= entryIndex;

	if(isExtraditionPkg)
	{
		pkgId 			 	= srcId;
		maxNvm      	    = PX_R_U32(CgmManager_getPkgUsedNvmObj(MEMtPN8(locateObjectInMemory(getPkgEntry(pkgId)))));
		srcSdId 			= CgmManager_getAppIdFromEntry(appEntry);
	}
	else
	{
		srcSdId 			= srcId;        //原关联SD id
		EntryAddr 			= MEMtPN8(locateObjectInMemory(appEntry));
		appId 				= CgmManager_getAppIdFromEntryObj(EntryAddr);
		cgmFlag 			= CgmField_getCGMFlag(EntryAddr);
		cgmArrayAddr		= MEMtPN8(CgmField_getArrayObj(EntryAddr));

		context				= U16_SHL_U8(U8t16(CgmField_getPkgID(EntryAddr)), 8);
		context				= U16_OR_U16(context, U8t16(appId));
		#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
		if(((cgmFlag & HAS82_MASK) == 1) || ((cgmFlag & HAS83_MASK) == 1))
		{
			CgmManager_error(SW_SECURITY_STATUS_NOT_SATISFIED);
        }
		#endif

		if(appId == destSdId)
		{
			extraditionFlag = SD_EXTRADITION_ONESELF;
		}

		if(U8_AND_U8(cgmFlag, HAS82_MASK))
		{
			maxRam  = CgmField_getMax82(cgmArrayAddr, cgmFlag);
		}
		else
		{
			maxRam  = CgmManager_getRamConsume(cgmFlag, context, cgmArrayAddr);
		}

		if(U8_AND_U8(cgmFlag, HAS83_MASK))
		{
			maxNvm = CgmField_getMax83(cgmArrayAddr, cgmFlag);
		}
		else
		{
			maxNvm = CgmManager_getNvmConsume(cgmFlag, context, cgmArrayAddr);
		}
	}

	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	appEntryAddr = MEMtPN8(getSdEntryObj(destSdId));
	cgmFlag = CgmField_getCGMFlag(appEntryAddr);
	if(((cgmFlag & HAS82_MASK) == 1) || ((cgmFlag & HAS83_MASK) == 1))
	{
		CgmManager_error(SW_SECURITY_STATUS_NOT_SATISFIED);
	}
	#endif
	CgmManager_SetSdMenInfo(srcSdId, maxRam, maxNvm, SD_INCREASE_SIZE); //关联原关联SD
	CgmManager_SetSdMenInfo(destSdId, maxRam, maxNvm, U8_OR_U8(extraditionFlag, SD_DECREASE_SIZE)); //管理目的SD
	return;
}

void CgmRun_RegistryUpdate(u16 appEntry,u8 srcSdId,u8 destSdId)
{
#if !CHINA_MOBILE_DESIGNATED_REQUIREMENT

	u8  				    extraditionFlag;
	u8  				    appId;
	u8  					cgmFlag;
	u32 					maxCGMram;
	u32						remainCGMram;
	u32						newCGMram;
	u32 					sdMaxConsumeReserveRam;
	u32 					maxCGMnvm;
	u32						remainCGMnvm;
	u32						newCGMnvm;
	u32 					sdMaxConsumeReserveNvm;
	boolean  				cgmUpdateRamFlag;
	boolean					cgmUpdateNvmFlag;
	PN8 		  			appEntryAddr;
	PN8 					cgmArrayAddr;

	maxCGMram 				= 0;
	remainCGMram 			= 0;
	newCGMram 				= 0;
    maxCGMnvm 				= 0;
	remainCGMnvm 			= 0;
	newCGMnvm 				= 0;
	sdMaxConsumeReserveRam  = 0;
	sdMaxConsumeReserveNvm  = 0;
	appId 					= 0;

	cgmUpdateRamFlag 		= ((CgmField_updateCgmRamFlag == 1) ? 1 : 0);
	cgmUpdateNvmFlag 		= ((CgmField_updateCgmNvmFlag == 1) ? 1 : 0);

	CgmField_updateCgmRamFlag = 0;  //清除flag
	CgmField_updateCgmNvmFlag = 0;

	appEntryAddr 			= MEMtPN8(locateObjectInMemory(appEntry));
	cgmFlag 				= CgmField_getCGMFlag(appEntryAddr);
	cgmArrayAddr 			= MEMtPN8(CgmField_getArrayObj(appEntryAddr));
	appId 					= CgmManager_getAppIdFromEntryObj(appEntryAddr);

	if(((cgmUpdateRamFlag)&&(U8_AND_U8(cgmFlag, HAS82_MASK) != HAS82_MASK))\
		    ||((cgmUpdateNvmFlag)&&(U8_AND_U8(cgmFlag, HAS83_MASK) != HAS83_MASK)))  //注意，新方案中不带CGM的的SD不能进行CGM更新
	{
		CgmManager_error(SW_CONDITIONS_NOT_SATISFIED);
	}

	extraditionFlag = ((appId == destSdId) ? SD_EXTRADITION_ONESELF : 0); //注意，这里如此处理是否有问题

	if(U8_AND_U8(cgmFlag, HAS82_MASK))
	{
		maxCGMram = CgmField_getMax82(cgmArrayAddr, cgmFlag);
		remainCGMram = CgmField_getRemain82(cgmArrayAddr, cgmFlag);
	}
	else //若没有8283，则求出max(consume,reserve)
	{
		sdMaxConsumeReserveRam = CgmManager_getRamConsume(cgmFlag, appEntry, cgmArrayAddr);
	}

	if(U8_AND_U8(cgmFlag, HAS83_MASK))
	{
		maxCGMnvm = CgmField_getMax83(cgmArrayAddr, cgmFlag);
		remainCGMnvm = CgmField_getRemain83(cgmArrayAddr, cgmFlag);
	}
	else
	{
		sdMaxConsumeReserveNvm = CgmManager_getNvmConsume(cgmFlag, appEntry, cgmArrayAddr);
	}

	if(cgmUpdateRamFlag)
	{
		newCGMram = CgmField_getUpdateCgmRam();
	}
	else
	{
		newCGMram = sdMaxConsumeReserveRam;
		maxCGMram = sdMaxConsumeReserveRam;
	}

	if(cgmUpdateNvmFlag)
	{
		newCGMnvm = CgmField_getUpdateCgmNvm();
	}
	else
	{
		newCGMnvm = sdMaxConsumeReserveNvm;
		maxCGMnvm = sdMaxConsumeReserveNvm;
	}

	//更新的不小于已使用的，且不大于关联SD或迁移目地SD剩余的
	if((newCGMram < U32_S_U32(maxCGMram, remainCGMram))
		  ||(newCGMnvm < U32_S_U32(maxCGMnvm, remainCGMnvm)))
	{
		CgmManager_error(SW_FILE_FULL);
	}

	CgmManager_SetSdMenInfo(srcSdId, maxCGMram, maxCGMnvm, SD_INCREASE_SIZE); //关联原关联SD
	CgmManager_SetSdMenInfo(destSdId, newCGMram, newCGMnvm, U8_OR_U8(extraditionFlag, SD_DECREASE_SIZE)); //管理目的SD

	if(U8_AND_U8(cgmFlag, HAS82_MASK))
	{
		remainCGMram = U32_S_U32(newCGMram, U32_S_U32(maxCGMram, remainCGMram));
	}
	if(U8_AND_U8(cgmFlag, HAS83_MASK))
	{
		remainCGMnvm = U32_S_U32(newCGMnvm, U32_S_U32(maxCGMnvm, remainCGMnvm));
	}
	CgmField_set8283Value(cgmArrayAddr, cgmFlag, remainCGMram, remainCGMnvm, UPDATA_SD_REMAIN_CGM_SIZE);//更新该SD本身的remainCGM
	CgmField_set8283Value(cgmArrayAddr, cgmFlag, newCGMram, newCGMnvm, UPDATA_SD_CGM_SIZE);//更新该SD本身的maxCGM
#else

	CgmManager_error(SW_SECURITY_STATUS_NOT_SATISFIED);

#endif
  return;
}
#endif
