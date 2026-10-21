/*****************************************************************************
 * @(#)mematom.c	1.20 03/08/13
 * Version 1.7
 *
 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/30
 .Created
 Original authors: wanghui
  modify by dengyiwen@20220106
 ****************************************************************************/

/*************************************** develop warnning ***************************************/
/*	修改本文件内代码或互用全局变量时需要注意如下：												*/	
/*	1.备份区与搬移互用同一块区域																*/	
/*		1.1.事物回滚不要需要考虑是否直接影响到搬移回滚											*/	
/*		1.2.other																				*/	
/*	2.other																						*/	
/*		2.1.other																				*/	
/*	3.other																						*/	
/*****************add by jiangxiaohui 20150803***************************************************/

#include "DataOperation.h"
#include "MemOperationServe.h"
#include "MemBackUpServe.h"
#include "bootConfigProfile.h"
#include "SysConfigTarget.h"
#include "MemNvmOperation.h"
#include "MemOperation.h"
#include "Crypto_common_API.h"
#include "FlashBackUp.h"
#include "SecOperationHal.h"
#include "CgmManager.h"
#include "Io.h"
#include "Boot.h"

#if !TRANSACTION_OLDDESIGN

#if BOOT__MEMORY__BACK_UP

void memBeginTransaction(u8 isMultiApduProcess)
{
	PATCH_BOOT_MEMORY_MEMBACKUP_001();
	memSetNoBackAreaPointer(MEMtPF8(NvmManager_GetMapTopAddress()), MEMtPF8(NvmManager_GetTopFreeNvm()));
	
	E2P_stream_flush();
	gBackUp_writeType			= WRITEEP_TRANSACTION;
	gBackUp_PageNumStart	= TF_BACKUP_First;
	if(MULTIL_APDU_PROCESS == isMultiApduProcess)
	{
		gBackUp_inMutilApdu		= true;
	}
	else
	{
		gBackUp_inSingleApdu	= true;
	}
}

void memCommitTransaction(void)
{
	NvmManager_Commit();
	E2P_stream_flush();
	clearBackUpPageFlag();
	gBackUp_PageNumStart	= TF_BACK_UP_NONE;
	gBackUp_writeType			= WRITEEP_NOATOMIC;
	gBackUp_cacheType			= WRITEEP_NOATOMIC;
	gBackUp_inSingleApdu	= false;
	gBackUp_inMutilApdu		= false;
}


void memAbortTransaction(u1 isReset)
{	
	PN8			X_A		flashPageProtectBuff;
	PN8			I_A		tempAddress;
	PX8			X_A		tempRealWriteBuf;
	u8 			I_A		tempBackUpPageFlag[MAX_BACK_UP_PAGE];
	u16			I_A		tempOffset, tempLength;
	u8			D_A		i, j, isFoundFakeEEPROMPage, bFound, backUpBlockType;
	
	ppIso7816Util_Command_t 	ppCommand1;	//特殊处理放废卡 后续需要删除
	
#if WITH_BREAK_TEST_CODE
	ppIso7816Util_Command_t 	ppCommand;
	
	io_GetCommand(&ppCommand);
	
	switch(ppCommand->pData[1])
	{
		case INS_BREAK_TEST_PAGE_NOTBAK:
		case INS_BREAK_TEST_PAGE_COMMIT:
		case INS_BREAK_TEST_PAGE_SELECT:
		case INS_BREAK_TEST_PAGE_READ:
		case INS_BREAK_TEST_PAGE_WRITE:
//		case INS_BREAK_TEST_PAGE_TRANSACTION_ACTION:
		case INS_BREAK_TEST_PAGE_WRITE_ATOMIC:
		case INS_BREAK_TEST_PAGE_ERASE:
		case INS_BREAK_TEST_PAGE_PROG:
			return;
		
		default:
			break;
	}
#endif
	
	io_GetCommand(&ppCommand1);
	
	if(ppCommand1->pData[1] == INS_CLEAR_OS && ppCommand1->pData[0] == 0xe0)
	{
		return;
	}
	
	
	PATCH_BOOT_MEMORY_MEMBACKUP_002();
	
	if (gBackUp_cacheType == WRITEEP_NOATOMIC)	  //write all cache to EEP when noatomic
	{
		E2P_stream_flush();
		FlashBackUp_CommitFlashFlag();
	}
	else if(ABORT_NOT_ON_RESET == isReset)
	{
		FlashBackUp_CommitFlashFlag();
	}
#if GP__CGM__FOR_MASK
	g_bRoomChanged = NO_CHANGED;//
#endif
	E2P_stream_clear();
	gBackUp_inSingleApdu	= false;
	gBackUp_inMutilApdu		= false;
	bFound					= 0;
	tempAddress				= MEMtPN8(BACK_UP_AREA_START);
	SC_deactiveEDC();
	isFoundFakeEEPROMPage	= 0;	
	tempRealWriteBuf		= MEMtPX8(sysConfig_GetTempArrBuffer(12));
	//Reuse
	flashPageProtectBuff 	= stream_buffer;
	
 	ecpMem_Set(&tempBackUpPageFlag[0], 0, BACK_UP_MAX_PAGE);
	FlashBackUp_RollBackPage(gBackUp_TempBuf, isReset);
	Transaction_RollBack(stream_buffer,gBackUp_TempBuf,isReset);
	
	SC_activeEDC();
}

void memSetInProgress(void)
{
	gBackUp_inSingleApdu	= true;
	gBackUp_inMutilApdu		= true;
}

void memInitTransaction(void)
{
	gBackUp_PageNumStart	= TF_BACK_UP_NONE;
	//BackupBlockNum			= 0;memory模块的，暂时拿到InitGlobalVariable()去
	initMemBackUpGloableVar();
}

#endif


#endif 


