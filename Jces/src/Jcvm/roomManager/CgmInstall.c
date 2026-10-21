/*
 * cgmInstallTime.c
 *
 *  Created on: 2015年10月21日
 *      Author: liuhanren
 */
#include "CgmManager.h"

#if GP__CGM__FOR_MASK
static void CgmInstall_CommitChangeSize(u16 backupCC, u16 RamChange, u32 NvmChange)
{
	u16		bContext;

	bContext	= backupCC;

	if(g_bCheckFlag != CHECK_CLOSE)
	{
		//在RUN TIME下，对关联SD的CGM管理，在CC改变和一条指令结束时才进行
		if(U8_AND_U8(g_bRoomChanged, RAM_SIZE_CHANGED) && (RamChange != 0))
		{
			CgmManager_RamChange(bContext, RamChange, 0);
		}
		if(U8_AND_U8(g_bRoomChanged, NVM_SIZE_CHANGED) && (NvmChange != 0))
		{
			CgmManager_NvmChange(bContext, NvmChange, 0);
		}
	}
}

void CgmInstall_ChangeCC(u16 context)
{
	if(context != g_CCInLoadOrInst)
	{
		// 安装过程中所有的对象都统计到应用
		if(JCRE_CONTEXT != context)
		{
			CgmInstall_setConsumeInfo();

			CgmField_getEntryInfo(context);
			g_CCInLoadOrInst = context;
		}
	}
}

void CgmInstall_setConsumeInfo()
{
	CgmInstall_CommitChangeSize(g_CCInLoadOrInst, g_UsedRAM, g_UsedNVM);

	if(g_bCheckFlag != CHECK_CLOSE)
	{
		if(U8_AND_U8(g_bRoomChanged, RAM_SIZE_CHANGED) && (g_UsedRAM != 0))
		{
			g_UsedRAM = 0;
		}

		if(U8_AND_U8(g_bRoomChanged, NVM_SIZE_CHANGED) && (g_UsedNVM != 0))
		{
			g_UsedNVM = 0;
		}

		g_bRoomChanged	 = NO_CHANGED;
		g_CCInLoadOrInst = NULL_CONTEXT;
	}
}

void CgmInstall_ConsumeCumulate(u32 pSize, u8 cgm_mask)
{
	CgmInstall_ChangeCC(cc);

	if(cgm_mask & HAS82_MASK)
	{
		g_UsedRAM = U32_A_U32(g_UsedRAM,pSize);
		g_bRoomChanged |= RAM_SIZE_CHANGED;
	}
	else
	{
		if(g_systemConsumeConfig == 0x00)
		{
			g_UsedNVM = U32_A_U32(g_UsedNVM,pSize);
		}
		else
		{
			//pSize -= MAP_ENTRY_SIZE;
			g_UsedNVM = U32_A_U32(g_UsedNVM,pSize);
		}
		g_bRoomChanged |= NVM_SIZE_CHANGED;
	}
}

void CgmInstall_End(u8 appId, u8 setAppSpacePara,u8 isMNOSDflag)
{
	u8  			associatedsdId;
	u16				context;
	u32 			cgmRam;
	u32 			cgmNvm;

	u8 				hasC7;
	u8 				hasC8;
	u8 				hasD7;
	u8 				hasD8;
	u8 				has82;
	u8 				has83;
	u32 			bSDmax_CGMRam;
	u32 			bSDmax_CGMNvm;
	PN8 			cgmArrayAddr;

	cgmRam  		= 0;
	cgmNvm  		= 0;
	bSDmax_CGMRam   = 0;
	bSDmax_CGMNvm   = 0;
	hasC7 			= U8_AND_U8(setAppSpacePara , HASC7_MASK);
	hasC8 			= U8_AND_U8(setAppSpacePara , HASC8_MASK);
	hasD7 			= U8_AND_U8(setAppSpacePara , HASD7_MASK);
	hasD8 			= U8_AND_U8(setAppSpacePara , HASD8_MASK);
	has82 			= U8_AND_U8(setAppSpacePara , HAS82_MASK);
	has83 			= U8_AND_U8(setAppSpacePara , HAS83_MASK);

	context			= U8t16(memReadU8(MEMtPM8(get_object_field_ref(g_appEntryAddr, theContext_INDEX))));
	context			= U16_A_U8(U16_SHL_U8(context, 8), appId);
	cgmArrayAddr 	= MEMtPN8(CgmField_getArrayObj(MEMtPF8(g_appEntryAddr)));

	//若安装的APP存在8283，则从关联SD的CGM中划出
	if(has82)
	{
		bSDmax_CGMRam = CgmField_Cache82();
	}
	if(has83)
	{
		bSDmax_CGMNvm = CgmField_Cache83();
	}

	associatedsdId = bRamBuffer[INSTALL_APPLET_SDID];
	if(has82 || has83||isMNOSDflag)
	{
		CgmManager_manageSdMenSize(associatedsdId, bSDmax_CGMRam, bSDmax_CGMNvm, SD_DECREASE_SIZE, false);
		//初始化安装应用本身的CGM值
		if(isMNOSDflag)
		{
			CgmField_set8283Value(cgmArrayAddr, setAppSpacePara, bSDmax_CGMRam, bSDmax_CGMNvm, INIT_CGM_WHEN_INSTALL_MNOSD);
		}
		else
		{
			CgmField_set8283Value(cgmArrayAddr, setAppSpacePara, bSDmax_CGMRam, bSDmax_CGMNvm, INIT_CGM_WHEN_INSTALL_SD);
		}

	}

	#if CHINA_UMICOM_SPECIAL_MANAGE
	if(has83)
	{
		g_iUsedNVM -= CHINA_UNICOM_ISD_CONSUME_FOR_SSD;
	}
	#endif

	CgmManager_calaConsumeSpace(context, &cgmNvm, &cgmRam, false, CGM_GET_ALL);
	//处理CGM空间变化，自身或者关联的SD(g_keepNInstall\g_keepRInstall之前复用于空间统计,register 之前归算于当前app)
	if(hasD7)
	{
		if(CgmField_CacheD7() > cgmRam)
		{
			cgmRam = CgmField_CacheD7();
		}
	}
	if(hasD8)
	{
		if(CgmField_CacheD8() > cgmNvm)
		{
			cgmNvm = CgmField_CacheD8();
		}
	}
	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	appId = associatedsdId;//bRamBuffer[INSTALL_APPLET_SDID];
	#endif

	#if CHINA_UMICOM_SPECIAL_MANAGE
	if(has83) //如果82也有现在再加上
	{
		CgmManager_manageSdMenSize(APP_FIRST, 0, CHINA_UNICOM_ISD_CONSUME_FOR_SSD, SD_DECREASE_SIZE, false);
	}
	#endif

	// init C7 C8 D7 D8 and consume
	if(hasC7)
	{
		if(CgmField_CacheC7() < cgmRam)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
		}
		if(cgmArrayAddr==MEM_NULL)
		{
			return;
		}
		CgmField_setC7(cgmArrayAddr,setAppSpacePara,CgmField_CacheC7());
	}

	if(hasC8)
	{
		if(CgmField_CacheC8() < cgmNvm)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_RESOURCE);
		}
		if(cgmArrayAddr==MEM_NULL)
		{
			return;
		}
		CgmField_setC8(cgmArrayAddr,setAppSpacePara,CgmField_CacheC8());
	}

	if(hasD7)
	{
		CgmField_setD7(cgmArrayAddr,setAppSpacePara,CgmField_CacheD7());
	}

	if(hasD8)
	{
		/*移动特殊标准下，D8的值由C8赋的，而C8是U32的，故这里写如进去的D8有可能不对（C8 > 0xFFFF时）
		  但我们在运行时读取 D8内容时也采取读取C8的值，故这里写得是否正确可以忽略。 */
		CgmField_setD8(cgmArrayAddr,setAppSpacePara,CgmField_CacheD8());
	}

	//减掉系统消耗空间
	if(!isMNOSDflag)
	{
		CgmManager_manageSdMenSize(appId, cgmRam, cgmNvm, SD_DECREASE_SIZE, false);
	}

	g_bRoomChanged	= NO_CHANGED;
	CgmInit_changeFlag(CHECK_RUN);
	CgmInit_consumeVar();

	return;
}
#endif
#if(GP__CGM__FOR_APPPARACHECK)
void CgmInstall_LimitCheck(u8 appId, u8 setAppSpacePara,u8 isMNOSDflag)
{
	u16				context;
	u32 			cgmRam;
	u32 			cgmNvm;
	u8 				hasC7;
	u8 				hasC8;
	u8 				has82;
	u8 				has83;
	u32 			bSDmax_CGMRam;
	u32 			bSDmax_CGMNvm;
	PN8 			cgmArrayAddr;
	//PN8 			sdEntryAddr;

	cgmRam  		= 0;
	cgmNvm  		= 0;
	bSDmax_CGMRam   = 0;
	bSDmax_CGMNvm   = 0;



	hasC7 			= U8_AND_U8(setAppSpacePara , HASC7_MASK);
	hasC8 			= U8_AND_U8(setAppSpacePara , HASC8_MASK);
	has82 			= U8_AND_U8(setAppSpacePara , HAS82_MASK);
	has83 			= U8_AND_U8(setAppSpacePara , HAS83_MASK);

	context			= U8t16(memReadU8(MEMtPM8(get_object_field_ref(g_appEntryAddr, theContext_INDEX))));
	context			= U16_A_U8(U16_SHL_U8(context, 8), appId);
	cgmArrayAddr 	= MEMtPN8(CgmField_getArrayObj(MEMtPF8(g_appEntryAddr)));
//	getAppTable();
//	sdEntryAddr = MEMtPN8(getSdEntryObj(appId));
//	cgmArrayAddr 	= MEMtPN8(CgmField_getArrayObj(sdEntryAddr));
	//若安装的APP存在8283，则从关联SD的CGM中划出
	if(has82)
	{
		bSDmax_CGMRam = CgmField_Cache82();
	}
	if(has83)
	{
		bSDmax_CGMNvm = CgmField_Cache83();
	}
	if(isMNOSDflag)
	{
		CgmField_set8283Value(cgmArrayAddr, setAppSpacePara, bSDmax_CGMRam, bSDmax_CGMNvm, INIT_CGM_WHEN_INSTALL_MNOSD);
	}
	if(has82 || has83)
	{
		CgmField_set8283Value(cgmArrayAddr, setAppSpacePara, bSDmax_CGMRam, bSDmax_CGMNvm, INIT_CGM_WHEN_INSTALL_SD);
	}


	if(hasC7 || hasC8)
	{
		CgmManager_calaConsumeSpace(context, &cgmNvm, &cgmRam, false, CGM_GET_ALL);
	//处理CGM空间变化，自身或者关联的SD(g_keepNInstall\g_keepRInstall之前复用于空间统计,register 之前归算于当前app)
	}
	//  RAM consume
	if(hasC7)
	{
		if(CgmField_CacheC7() < cgmRam)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
		}
	}
	//  NVM consume
	if(hasC8)
	{
		if(CgmField_CacheC8() < cgmNvm)
		{
			CgmManager_error(SYSTEMEXCEPT_NO_RESOURCE);
		}

	}



	return;
}



#endif
