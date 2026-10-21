/*
 * CgmInit.c
 *
 *  Created on: 2015年11月5日
 *      Author: liuhanren
 */

#include "CgmManager.h"
#include "BootOS.h"
#include "CgmField.h"
#include "Boot.h"
#include "tranManager.h"

#if (GP__CGM__FOR_APPPARACHECK)
void CgmInit_ValueForISD(void)
{
	u32 			ramSize;
	u32 			nvmSize;
	u8 				cgmFlag;
	PN8 			sdEntryAddr;
	PN8 			cgmArrayAddr;

	cgmFlag 		= 0;
	ramSize 		= tranAllocate_rtrAvailable();
	nvmSize 		= NvmManager_GetAvailMemory();

	getAppTable();
	sdEntryAddr     = MEMtPN8(getSdEntryObj(APP_FIRST));
	cgmArrayAddr    = MEMtPN8(CgmField_getArrayObj(sdEntryAddr));

	cgmFlag 		= U8_OR_U8(HAS82_MASK, HAS83_MASK);  //ISD只带8283
	//ISD只带8283
	if(CgmField_getCGMFlag(sdEntryAddr) != cgmFlag)
	{
		CgmManager_error(SYSTEMEXCEPT_ILLEGAL_VALUE);
	}
	else
	{
		nvmWriteU32NoAtomic(PN8_A_U8(cgmArrayAddr, CgmField_getMaxCGMOffset(cgmFlag,HAS82_MASK)), RTR_SEGMENT_SIZE);
		nvmWriteU32NoAtomic(PN8_A_U8(cgmArrayAddr, CgmField_getMaxCGMOffset(cgmFlag,HAS83_MASK)), U32_S_U32(sysConfig_GetNvmTopBase(), E2PBase));
		//init remainCGM
		nvmWriteU32NoAtomic(PN8_A_U8(cgmArrayAddr, CgmField_getCGMarrayOffset(cgmFlag,HAS82_MASK)), ramSize);
	#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		nvmWriteU32NoAtomic(PN8_A_U8(cgmArrayAddr, CgmField_getCGMarrayOffset(cgmFlag,HAS83_MASK)), U32_S_U32(nvmSize - CMCC_CGM_REPAIR_CONST)); //1K用于抵消CGM统计误差
	#else
		nvmWriteU32NoAtomic(PN8_A_U8(cgmArrayAddr, CgmField_getCGMarrayOffset(cgmFlag,HAS83_MASK)), nvmSize);

	#endif		
	}
}



void CgmInit_consumeVar(void)
{
	g_UsedRAM        = 0;
	g_UsedNVM        = 0;
}

void CgmInit_GlobalVar(void)
{
	g_sC7Value = 0;
	g_sC8Value = 0;
	g_sD7Value = 0;
	g_sD8Value = 0;
	g_bRoomChanged = 0;
	g_appEntryAddr = 0;
	g_CCInLoadOrInst = 0;

	//CgmInit_SystemConsumeConfig();
	if(OS_C7C8_NOCHECK || OS_CGM_CLOSE)
	{
		g_bCheckFlag = CHECK_CLOSE;
	}
	else
	{
		g_bCheckFlag = NO_CHECK;
	}
}

void CgmInit_changeFlag(u8 flag)
{
	if(g_bCheckFlag != CHECK_CLOSE)
	{
		g_bCheckFlag = flag;
	}
}
void CgmInit_UpdateTag(void)
{
	CgmField_updateCgmNvmFlag 	=	0;
	CgmField_updateCgmRamFlag	=   0;
}
void CgmInit_keepVarValue(PX8 pa1, u8 value1)
{
	pa1[0]	= value1;
}
#endif
#if GP__CGM__FOR_MASK
void CgmInit_SystemConsumeConfig(void)
{
	g_systemConsumeConfig = JcvmConfig_GetCGMsystemConsumeConfig();//modify by xuxiaojun@20211009 统一使用写法
}
u8	CgmInit_EnableOrNot(PM8 commandBuffer, u8 type)
{
	u8				reValue;

	reValue		    = true;

	if(INS_GET_DATA == type)
	{
		commandBuffer[APDU_DATA_OFFSET] = U8_SHR_U8(OS_CGM_CLOSE, 1);
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, 1);
	}
	else
	{
		if(((1 != commandBuffer[APDU_DATA_OFFSET]) && (0 != commandBuffer[APDU_DATA_OFFSET])) || (1 != commandBuffer[APDU_P3_OFFSET]))
		{
			reValue    = false;
		}
		else
		{
			//when close cgm, there no need to init CGM various
			if(!commandBuffer[APDU_DATA_OFFSET])
			{
				CgmInit_ValueForISD();
			}
			commandBuffer[APDU_DATA_OFFSET] = U8_OR_U8(U8_AND_U8(JcvmConfig_GetVmConfig(), 0xfd), U8_SHL_U8(commandBuffer[APDU_DATA_OFFSET], 1)); //modify by xuxiaojun@20211009 统一使用写法
			JcvmConfig_SetVmConfig(commandBuffer[APDU_DATA_OFFSET]); //modify by xuxiaojun@20211009 统一使用写法

		}
	}

	return    reValue;
}

void CgmInit_keepVarValue(PX8 pa1, u8 value1)
{
	pa1[0]	= value1;
}

void CgmInit_SetFlag(void)
{
	u8 			flag;

	flag		= U16t8(popShort());
	pushShort(U8t16(g_bCheckFlag));
	CgmInit_changeFlag(flag);
}



void CgmInit_UpdateTag(void)
{
	CgmField_updateCgmNvmFlag 	=	0;
	CgmField_updateCgmRamFlag	=   0;
}

#endif
