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

#if TRANSACTION_OLDDESIGN

/************************************************原事务保护方案************************************************************************************
 *************************************************************************************************************************************************/ 

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
	gBackUp_curSize			= 0;
	gBackUp_curOffset		= 0;
}


void memAbortTransaction(u1 isReset)
{	
	PN8			X_A		flashPageProtectBuff;
	PN8			I_A		tempAddress;
	PX8			X_A		tempRealWriteBuf;
	u8 			I_A		tempBackUpPageFlag[MAX_BACK_UP_PAGE];
	u16			I_A		tempOffset, tempLength;
	u8			D_A		i, j, isFoundFakeEEPROMPage, bFound, backUpBlockType;
	
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
	
	if(ABORT_ON_RESET == isReset)
	{
			/*COS must call HED_HEFlash_Init before any operation of writing HEFlash. 
	   This apdu is 1st APDU. 
	   For example :You can add 'call HED_HEFlash_Init' in here .
		User need to judge ret status in detail */   
	#if BOOT__MEMORY__CHIP_PAGE_PROTECT	
		CHIP_PAGE_RECOVER_INIT_FUCNTION();//CIU_FlashInit();
	#endif
		RealWriteBakupPage(tempAddress, tempRealWriteBuf);
	}
 	ecpMem_Set(&tempBackUpPageFlag[0], 0, BACK_UP_MAX_PAGE);
	FlashBackUp_RollBackPage(gBackUp_TempBuf, isReset);
	//find out there is back up or not
	for(i = 0;i < BACK_UP_MAX_PAGE; U8_INC(i))
	{
		ecpMem_Copy(gBackUp_TempBuf, tempAddress, EEPAGE_SIZE);
		
		if(calBackUpPageCrc(gBackUp_TempBuf) == getBackUpPageCrc(gBackUp_TempBuf))
		{
			if((ABORT_ON_RESET == isReset) && (isFoundFakeEEPROMPage == 0) && (FlashBackUp_IsSurpport()))
			{
				for(j = 0; j < 2; j++ )
				{
					if( ecpMem_Compare(gBackUp_TempBuf, tempAddress, HAL_EEPAGE_SIZE) != 0)
					{
						FlashBackUp_PageProtect(PNtMEM(tempAddress), flashPageProtectBuff);
						MemNvmOperation_WriteErase(tempAddress, gBackUp_TempBuf, HAL_EEPAGE_SIZE);
						isFoundFakeEEPROMPage = 1;
						break;
					}
				}
			}
			if(isBackUpPageRollBack(gBackUp_TempBuf))
			{
				if(HardWareCheckBackUpFlag(gBackUp_TempBuf, tempAddress,i, isReset, tempRealWriteBuf))
				{
					bFound = 1;
					gBackUp_PageNumStart = i;
					if((ABORT_ON_RESET == isReset) && (isFoundFakeEEPROMPage == 0) && (FlashBackUp_IsSurpport()))
					{
						FlashBackUp_PageProtect(PNtMEM(tempAddress), flashPageProtectBuff);
						MemNvmOperation_WriteErase(tempAddress, gBackUp_TempBuf, HAL_EEPAGE_SIZE);
					}
					tempBackUpPageFlag[i]	= 1;
				}
				else
				{
					tempBackUpPageFlag[i]	= 1;
				}
				break;
			}
			else if(isBackUpPageCommit(gBackUp_TempBuf))
			{
                if((BACK_UP_PAGE_COMMIT != getBackUpPageCommitFlag(tempAddress))&&(FlashBackUp_IsSurpport()))
				{
					tempBackUpPageFlag[i]	= 1;
				}
			}
			else
			{
				tempBackUpPageFlag[i]	= 1;
			}
		}
		else
		{
			if(ABORT_ON_RESET == isReset)
			{

				if(HardWareCheckCompFlag(gBackUp_TempBuf, tempAddress, i, isReset))
				{
					if((ABORT_ON_RESET == isReset) && (isFoundFakeEEPROMPage == 0) && (FlashBackUp_IsSurpport())&&(FlashPageBackUp_IsSurpport()))
					{
						FlashBackUp_PageProtect(PNtMEM(tempAddress), flashPageProtectBuff);
						MemNvmOperation_WriteErase(tempAddress, gBackUp_TempBuf, HAL_EEPAGE_SIZE);
					}
					break;
				}
				else
				{
					tempBackUpPageFlag[i]	= 1;
				}
			}
		}
		tempAddress	= PN8_A_CONST(tempAddress, EEPAGE_SIZE);
	}
 
	if(bFound)
	{		
		tempOffset		= searchBlocks(0, 0, ILLEGAL_BLOCK_OFFSET, CHECK_IS_ROLL_BACK);
		
		if(ILLEGAL_BLOCK_OFFSET != tempOffset) 
		{
			tempOffset = HardWareCheckBackUpBlock(tempOffset);
		}
		while(ILLEGAL_BLOCK_OFFSET != tempOffset)
		{
			tempOffset		= searchBlocks(0, 0, tempOffset, GET_LAST_BLOCK);
			if(FIRST_BLOCK_IS_ERROR == tempOffset)
			{
				break;
			}
			backUpBlockType = U16t8(U16_SHR_U8(getBackUpBlockTypeSize(PX8_A_CONST(gBackUp_TempBuf, BACK_UP_PAGE_BLOCK_START_OFFSET)), BACK_UP_BLOCK_TYPE_SHIFT_BIT));
			backUpBlockType	= U8_AND_U8(backUpBlockType, BACK_UP_BLOCK_TYPE_MASK);
			/***roll back noatomic only on reset***/
			if((WRITEEP_NOATOMIC == backUpBlockType) && (ABORT_NOT_ON_RESET == isReset))
			{
				continue;
			}
			tempAddress	= MEMtPN8(getBackUpBlockAddress(PX8_A_CONST(gBackUp_TempBuf, BACK_UP_PAGE_BLOCK_START_OFFSET)));
			tempLength	= getBackUpBlockTypeSize(PX8_A_CONST(gBackUp_TempBuf, BACK_UP_PAGE_BLOCK_START_OFFSET));
			tempLength	= U16_AND_U16(tempLength, BACK_UP_BLOCK_LENGTH_MASK);
			FlashBackUp_PageProtect(PNtMEM(tempAddress), &flashPageProtectBuff[0]);
			MemNvmOperation_WriteErase(tempAddress, PX8_A_U8(getBackUpBlockContent(PX8_A_CONST(gBackUp_TempBuf, BACK_UP_PAGE_BLOCK_START_OFFSET)), U8_AND_U8(tempAddress , 0x03)), tempLength);
		}

//		systemCache.Flag = 0;
		FlashBackUp_CommitFlashFlag();
		if(ABORT_NOT_ON_RESET == isReset)
		{
			NvmManager_Reset();
			memCommitTransaction();
		}
		else
		{//如果是复位，需要先初始化全局变量，否则可能会死卡
			NvmManager_Init(MEMtPN8(U32_A_U16(PFtMEM(staticinit), U16_S_U16(EEPROM_IMAGE_SIZE, U16_M_U16(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE))))
/*@ -disspecopt @*/			,MEMtPN8(U32_S_U16(MASK_MAP_START, U16_M_U16(MASK_MAP_ENTRY_COUNT, MAP_ENTRY_SIZE))) /*@ +disspecopt @*/
							,MASK_MAP_ENTRY_COUNT, MEMtPN8(getBackupStart()));
		}
		
	}
	else if(ABORT_ON_RESET != isReset)
	{
		NvmManager_Reset();
	}
    
    //clearn up back up flag
	tempAddress				= MEMtPN8(BACK_UP_AREA_START);
	for(i = 0; i < BACK_UP_MAX_PAGE; U8_INC(i))
	{
	 	if((tempBackUpPageFlag[i] == 1) && (PR_R_U32(tempAddress) != 0))
		{
			fillEeprom(tempAddress, 0x00, EEPAGE_SIZE);
		}
		tempAddress	= PN8_A_CONST(tempAddress, EEPAGE_SIZE);
	}
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
