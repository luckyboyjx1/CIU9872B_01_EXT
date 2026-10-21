#include "FlashBackUp.h"
#include "BootLifeCycle.h"
#include "ecpOperation.h"
#include "MemBackUpServe.h"
#include "BootErrorHandle.h"
#include "Crypto_common_API.h"
#include "MemBackUp.h"
#include "NvmHal.h"
#include "SecureOperation.h"
#include "MemOperationServe.h"

#if TRANSACTION_OLDDESIGN

#if BOOT__MEMORY__FLASH_BACK_UP

u8 			X_A									gFlash_Page1Num;
u8			X_A									gFlash_FlagNum;
u8 			X_A									gFlash_CurPage;
u16			X_A									gFlash_FlagBlockOffset;
u16			X_A									gFlash_BackUpFlagOffset;

#if !BOOT__MEMORY__CHIP_PAGE_PROTECT
void FlashBackUp_clearFlagArea(void)
{
	u8 tempbuffer[4] = {0x01,0x02,0x03,0x04};
	
	MemNvmOperation_Erase(PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)), EEPAGE_SIZE);
	MemNvmOperation_WriteOnly(PN8_A_U16(PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)),0x01FC),tempbuffer,0x04);
	gFlash_BackUpFlagOffset		= 0;
	gFlash_FlagBlockOffset		= 0;
	return;
}
#endif

static u16 FlashBackUp_calcFlagCrc(PN8 flagAddress)
{
	//Get_crc16
	return Get_crc16(PX8_A_U16(flagAddress, FLASH_FLAG_INFO_FIN_OFFSET), U8_S_U8(FLASH_FLAG_INFO_SIZE, FLASH_FLAG_INFO_CRC_SIZE));
}

void FlashBackUp_GetNextBackUpCurPage(void)
{
	if(gFlash_CurPage < U8_S_U8(FLASH_PROTECT_PAGE_NUM, 1))
	{
		U8_INC(gFlash_CurPage);
	}
	else
	{
		gFlash_CurPage = 0;
	}
	return;
}

#if !BOOT__MEMORY__CHIP_PAGE_PROTECT
void FlashBackUp_Init(void)
{
//	memref	nvmTopAddr;
	
//	nvmTopAddr		= sysConfig_GetNvmTop();
	
	/*if(U32_R_U16(nvmTopAddr , U8_M_U16(FLASH_PROTECT_PAGE_NUM , EEPAGE_SIZE)) ==0)
	{
		BACK_UP_MAX_PAGE			= U8_S_U8(BACK_UP_MAX_PAGE , U8_A_U8(FLASH_PROTECT_PAGE_NUM , 1));
		gFlash_FlagNum				= BACK_UP_MAX_PAGE;
		gFlash_Page1Num		= U8_A_U8(BACK_UP_MAX_PAGE ,1);
	}
	else
	{
		BACK_UP_MAX_PAGE			= U8_S_U8( BACK_UP_MAX_PAGE , ((nvmTopAddr & (FLASH_PROTECT_PAGE_NUM * EEPAGE_SIZE -1))/EEPAGE_SIZE + ERASEBLOCKSIZE));
		gFlash_FlagNum				= U8_A_U8(BACK_UP_MAX_PAGE ,FLASH_PROTECT_PAGE_NUM);
		gFlash_Page1Num		= BACK_UP_MAX_PAGE;
	}*/
	BACK_UP_MAX_PAGE			= U8_S_U8(BACK_UP_MAX_PAGE , U8_A_U8(FLASH_PROTECT_PAGE_NUM , 1));
	gFlash_FlagNum				= BACK_UP_MAX_PAGE;
	gFlash_Page1Num		= U8_A_U8(BACK_UP_MAX_PAGE ,1);
	gFlash_CurPage			= 0;
	gFlash_FlagBlockOffset			= 0;
	//gFlash_BackUpFlagOffset is init after reset in abortTransaction
	//gFlash_BackUpFlagOffset			= FLASH_FLAG_INFO_ILLEGAL_OFFSET;
}
#endif

//void FlashBackUp_ResumeFlashFlag(void)
//{
//	gFlash_FlagBlockOffset			= gFlash_BackUpFlagOffset;
//}

void FlashBackUp_SetPageInfo(PX8 tempBuffer, PN8 tempAddress)
{
	SC_deactiveEDC();
	setBackUpPageCommitCrc(tempBuffer, calBackUpPageCommitCrc(tempAddress));
	SC_activeEDC();
	return;
}

u8 FlashBackUp_GetAlreadyBackUpPage(memref backUpDes)
{
	u8	pageNum;
	u16 tempOffset;
	PN8	flashFlagOffsetAddress;
	
	flashFlagOffsetAddress		= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	flashFlagOffsetAddress		= PN8_A_U16(flashFlagOffsetAddress, gFlash_FlagBlockOffset);
	tempOffset					= gFlash_FlagBlockOffset;
	pageNum						= FLASH_FLAG_INFO_ILLEGAL;

	for( ; tempOffset < gFlash_BackUpFlagOffset ; )
	{
		if(backUpDes == getFlashFlagInfoDes(flashFlagOffsetAddress))
		{
			pageNum				= getFlashFlagInfoPage(flashFlagOffsetAddress);
			break;
		}
		tempOffset				= U16_A_U16(tempOffset, FLASH_FLAG_INFO_SIZE);
		flashFlagOffsetAddress	= PN8_A_U16(flashFlagOffsetAddress, FLASH_FLAG_INFO_SIZE);
	}
	
	return pageNum;
}

void FlashBackUp_copyDesToBackPage(memref backUpDes, PX8 tempBuffer)
{
	u8			tempPageNum;
	PN8			flashBackUpPageAddress;
	
	//block erase
	if(0 == gFlash_CurPage)
	{
		FlashBackUp_CommitFlashFlag();
		flashBackUpPageAddress		= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_Page1Num, EEPAGE_SIZE));
		MemNvmOperation_Erase(flashBackUpPageAddress, U8_M_U16(FLASH_PROTECT_PAGE_NUM , EEPAGE_SIZE));
	}
	else
	{
		tempPageNum = U8_A_U8(gFlash_Page1Num, gFlash_CurPage);
		flashBackUpPageAddress		= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(tempPageNum, EEPAGE_SIZE));
	}

	ecpMem_Copy(tempBuffer, MEMtPN8(backUpDes), EEPAGE_SIZE);	

	//copy dest page to page protect area
	MemNvmOperation_WriteOnly(flashBackUpPageAddress, tempBuffer, EEPAGE_SIZE);
	return;
}

#if !BOOT__MEMORY__CHIP_PAGE_PROTECT
void FlashBackUp_PageProtect(memref backUpDes, PX8 tempBuffer)
{	
	PN8     	flashFlagPageAddress;
	u8			flashBackUpPage;
	
	backUpDes	= U32_AND_U32(backUpDes, NOR_EEPAGE_MASK);
	SC_deactiveEDC();
	flashBackUpPage		= FlashBackUp_GetAlreadyBackUpPage(backUpDes);
	if(FLASH_FLAG_INFO_ILLEGAL == flashBackUpPage)
	{
		//write des to backup flash page
		FlashBackUp_copyDesToBackPage(backUpDes, tempBuffer);
		flashBackUpPage	= gFlash_CurPage;
		FlashBackUp_GetNextBackUpCurPage();
		
		if(U16_D_U16(U16_S_U16(FLASH_FLAG_MAX_OFFSET, gFlash_FlagBlockOffset), FLASH_FLAG_INFO_SIZE) < FLASH_PROTECT_PAGE_NUM)
		{
			FlashBackUp_clearFlagArea();
		}
		
		flashFlagPageAddress		= MEMtPN8(PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)));
		flashFlagPageAddress		= PN8_A_U16(flashFlagPageAddress, gFlash_BackUpFlagOffset);
		gFlash_BackUpFlagOffset		= U16_A_U16(gFlash_BackUpFlagOffset, FLASH_FLAG_INFO_SIZE);
		
		setFlashFlagInfoDes(tempBuffer, backUpDes);
		setFlashFlagInfoNotFinish(tempBuffer, flashFlagPageAddress);
		setFlashFlagInfoPage(tempBuffer, flashBackUpPage);
		setFlashFlagInfoCrc(tempBuffer, FlashBackUp_calcFlagCrc(tempBuffer));

		//write flag to flash
		MemNvmOperation_WriteOnly(PN8_A_U16(flashFlagPageAddress, FLASH_FLAG_INFO_DES_OFFSET), &tempBuffer[FLASH_FLAG_INFO_DES_OFFSET], (FLASH_FLAG_INFO_DES_SIZE + FLASH_FLAG_INFO_PAGE_SIZE + FLASH_FLAG_INFO_CRC_SIZE));
	}
	SC_activeEDC();	
	return;
}

void FlashBackUp_CommitFlashFlag(void)
{
	u8	tempBuffer[FLASH_FLAG_INFO_SIZE];
	u16 tempOffset;
	PN8	flashFlagOffsetAddress;
	SC_deactiveEDC();
	flashFlagOffsetAddress		= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	flashFlagOffsetAddress		= PN8_A_U16(flashFlagOffsetAddress, gFlash_FlagBlockOffset);
	flashFlagOffsetAddress		= PN8_A_U8(flashFlagOffsetAddress, FLASH_FLAG_INFO_FIN_OFFSET);
	tempOffset					= gFlash_FlagBlockOffset;

	for( ; tempOffset < gFlash_BackUpFlagOffset ; )
	{
		setFlashFlagInfoFinish(tempBuffer, FLASH_FLAG_FIN);
		MemNvmOperation_WriteOnly(flashFlagOffsetAddress, PX8_A_U16(tempBuffer, FLASH_FLAG_INFO_FIN_OFFSET), FLASH_FLAG_INFO_FIN_SIZE);
		tempOffset				= U16_A_U16(tempOffset, FLASH_FLAG_INFO_SIZE);
		flashFlagOffsetAddress	= PN8_A_U16(flashFlagOffsetAddress, FLASH_FLAG_INFO_SIZE);
	}
	
	FlashBackUp_ResumeFlashFlag();
	SC_activeEDC();	
	return;
}
#endif

void FlashBackUp_commitPageInfo(PX8 tempBuffer)
{
	SC_deactiveEDC();
	//BACK_UP_PAGE_COMMIT
	setBackUpPageCommitFlag(tempBuffer, BACK_UP_PAGE_COMMIT);
	MemNvmOperation_WriteOnly(MEMtPN8(U32_A_U8(U16_A_U32(U8_M_U16(gBackUp_PageNumStart , EEPAGE_SIZE) ,BACK_UP_AREA_START),BACK_UP_PAGE_COMMIT_FLAG_OFFSET)),\
				PX8_A_U16(tempBuffer, BACK_UP_PAGE_COMMIT_FLAG_OFFSET),\
				BACK_UP_PAGE_COMMIT_FLAG_SIZE);
	SC_activeEDC();
	return;
}

u8 FlashBackUp_isRollBackPage(PN8 backUpPage)
{
	u8 result;
	
	if(getBackUpPageCommitCrc(backUpPage) == calBackUpPageCommitCrc(backUpPage))
	{
		result	= true;
	}
	else
	{
		result	= false;
	}
	
	return result;
}

u8 isWeakWrite(PN8 desAddr)
{	
	u8			flashPageProtectBuff[HAL_EEPAGE_SIZE];
	u8 			i;
	
	ecpMem_Copy(&flashPageProtectBuff[0], desAddr, HAL_EEPAGE_SIZE);
	for(i = 0; i < 2; i++ )
	{
		ecpMem_Copy(&flashPageProtectBuff[0], desAddr, HAL_EEPAGE_SIZE);	
		if(ecpMem_Compare(desAddr, &flashPageProtectBuff[0], HAL_EEPAGE_SIZE) != 0)
		{
			FlashBackUp_clearFlagArea();
			return true;
		}
	}
	return false;
}

#if !BOOT__MEMORY__CHIP_PAGE_PROTECT
void FlashBackUp_RollBackPage(PX8 tempBuffer, u1 isReset)
{	
	PN8			flashBackUpAddress;
	PN8			flashFlagAddress;
	memref		desAddrss;
	u16			flashCurPage;
	boolean		isBlockStartFind;
	boolean		isFlagTableDirty;
	u8			i;
	
	isBlockStartFind	= false;
	isFlagTableDirty	= false;
	flashBackUpAddress	= PN8_A_U16(BACK_UP_AREA_START, (u16)U8_M_U16(gFlash_Page1Num, EEPAGE_SIZE));
	flashFlagAddress	= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	
	ecpMem_Copy(stream_buffer, flashFlagAddress, HAL_EEPAGE_SIZE);
	for(i = 0; i<2 ; i++ )
	{
		if( ecpMem_Compare(stream_buffer, flashFlagAddress, HAL_EEPAGE_SIZE) != 0)
		{
			FlashBackUp_clearFlagArea();
			return;
		}
	}

	flashFlagAddress  = stream_buffer;
	
	for(gFlash_BackUpFlagOffset = 0; gFlash_BackUpFlagOffset < FLASH_FLAG_MAX_OFFSET; )
	{
		desAddrss	= getFlashFlagInfoDes(flashFlagAddress);

		if(desAddrss != FLASH_FLAG_INFO_DES_NULL)
		{
			if((getFlashFlagInfoCrc(flashFlagAddress) == FlashBackUp_calcFlagCrc(flashFlagAddress))
			   && (getFlashFlagInfoFinish(flashFlagAddress) != FLASH_FLAG_FIN))
			{
				flashCurPage			= getFlashFlagInfoPage(flashFlagAddress);
				if(flashCurPage > U8_S_U8(FLASH_PROTECT_PAGE_NUM, 1))
				{
					isFlagTableDirty	= true;
				}
				else
				{
					//write backup page to des
					desAddrss						= getFlashFlagInfoDes(flashFlagAddress);					
					flashCurPage					= U8_A_U8(gFlash_Page1Num , flashCurPage);
					flashBackUpAddress				= PN8_A_U16(BACK_UP_AREA_START, (u16)U8_M_U16(flashCurPage, EEPAGE_SIZE));
					ecpMem_Copy(tempBuffer, flashBackUpAddress, EEPAGE_SIZE);
					MemNvmOperation_WriteErase(MEMtPN8(U32_AND_U32(PNtMEM(desAddrss), NOR_EEPAGE_MASK)),tempBuffer,EEPAGE_SIZE);
					
					if(false == isBlockStartFind)
					{
						FlashBackUp_ResumeFlashFlag();
						isBlockStartFind			= true;
					}
				}
			}
			else if(getFlashFlagInfoFinish(flashFlagAddress) == FLASH_FLAG_FIN)
			{
//				setFlashFlagInfoFinish(flashFlagAddress, FLASH_FLAG_NOT_FIN);
//				if(getFlashFlagInfoCrc(flashFlagAddress) != FlashBackUp_calcFlagCrc(flashFlagAddress))
//				{
//					isFlagTableDirty		= true;
//				}
			}
			else
			{
				isFlagTableDirty		= true;
			}
		}
		else
		{
			if((FLASH_FLAG_NOT_FIN == getFlashFlagInfoFinish(flashFlagAddress))
			   && (FLASH_FLAG_ILLEGAL_PAGE == getFlashFlagInfoPage(flashFlagAddress))
			   && (FLASH_FLAG_INFO_ILLEGAL_CRC == getFlashFlagInfoCrc(flashFlagAddress))
			  )
			{
				//FlashBackUp_ResumeFlashFlag();
				break;	//do nothing
			}
			else
			{
				isFlagTableDirty		= true;
			}
		}
		
		gFlash_BackUpFlagOffset 		= U16_A_U8(gFlash_BackUpFlagOffset, FLASH_FLAG_INFO_SIZE);
		flashFlagAddress				= PN8_A_U16(flashFlagAddress, FLASH_FLAG_INFO_SIZE);
	}
	if(ABORT_ON_RESET == isReset)
	{	
		FlashBackUp_clearFlagArea();
	}
	else
	{
		if(true == isFlagTableDirty)
		{
			FlashBackUp_clearFlagArea();
		}
		else
		{
			FlashBackUp_CommitFlashFlag();
		}
	}
	
	return ;
}
#endif

#endif

#endif 

