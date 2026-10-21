/*
===============================================================================
 Copyright ?2007-2009 EASTCOMPEACE, All Rights Reserved.

 This software is the confidential and proprietary information of
 EASTCOMPEACE ("Confidential Information").
 You shall not disclose such Confidential Information and shall use it only
 in accordance with the terms of the license agreement you entered into with
 EASTCOMPEACE.

 FILE NAME: main.c
 VERSION  : 1.0
 HISTORY  :
 1. build this file on Dec 12 2011
===============================================================================
*/

#include "global_variable.h"
#include "lifecycle.h"
#include "reset.h"
#include "mask.h"
#include "JcvmConfig.h"
#include "interpreter.h"
#include "patch.h"
#include "Boot.h"
#include "ecpReadWrite.h"
#include "ecpOperation.h"
#include "chipHead.h"
#include "Io_SPI.h"
#include "Io_I2C.h"

#include "BL_Upgrade.h"


void os_init(void)
{
    u8     D_A    isDeselect;
	u8     D_A    resetStauts;
	
    patch_initBeforAts();
    reset_Init();
	resetStauts = apdu_SetDeselectHandler();
	isDeselect = reset_ReInit(resetStauts);

	//clearCryptoSpace();//初始化算法RAM区 LIB，是不是需要放在更前面？
	bootInit(isDeselect);
    
	// 初始化全局变量.
	InitGlobalVariable();


	BootOsMain();

#if !BOOTLOADER_PRO
	TelecomVarInit();// modify by chentao @20230308 HandleFirstCommand 函数中除MoveEepromResume以外的操作挪到私有指令后处理

	NvmManager_Deallocate(true);

	//Init JCRE3: include init ram	,channel info , and interrupt
	InitJCRE3();
	reset();
	run();
#endif
	return;
}

