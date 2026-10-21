#include "DataOperation.h"
#include "MemBackUpServe.h"
#include "MemOperationServe.h"
#include "bootConfigProfile.h"
#include "SysConfigTarget.h"
#include "MemNvmOperation.h"
#include "MemBackUp.h"
#include "NvmHal.h"
#include "boot.h"
#include "BootLifeCycle.h"
#include "BootErrorHandle.h"
#include "MemOperation.h"
#include "Crypto_API.h"
#include "BootRam.h"
#include "FlashBackUp.h"
#include "memory.h"

#if TRANSACTION_OLDDESIGN

/************************************************原事务保护方案************************************************************************************
 *************************************************************************************************************************************************/ 


/*************************************** develop warnning ***************************************/
/*	修改本文件内代码或互用全局变量时需要注意如下：													*/	
/*	1.备份区与搬移互用同一块区域																	*/	
/*		1.1.事物回滚不要需要考虑是否直接影响到搬移回滚												*/	
/*		1.2.other																				*/	
/*	2.other																						*/	
/*		2.1.other																				*/	
/*	3.other																						*/	
/*****************add by jiangxiaohui 20150803***************************************************/

/*******************************************global variable used in MemBackUp module*************************************************************
 *************************************************************************************************************************************************/
#pragma MEM_BACK_UP_SERVE_RAM_START
u8      X_A     gBackUp_writeType;
u8      X_A     gBackUp_cacheType;
u8      X_A     BACK_UP_MAX_PAGE;
u8      X_A     FLASH_PROTECT_PAGE_NUM;
PN8     X_A     gBackUp_NotBackUpBottom;
PN8     X_A     gBackUp_NotBackUpTop;


#if BOOT__MEMORY__BACK_UP
u1 		B_A     gBackUp_inSingleApdu;
u1		B_A    	gBackUp_inMutilApdu;
u8      X_A     gBackUp_PageNumStart;
u16     X_A     gBackUp_curSize;
u16     X_A     gBackUp_curOffset;
PN8     X_A     BACK_UP_AREA_START;

MEM_BACK_UP_SERVE_RAM_END

/************************************************************************************************************************************************
 *************************************************************************************************************************************************/

/************************************************start of methods in the MemBackUp module********************************************************
 *************************************************************************************************************************************************/
void initMemBackUpGloableVar(void) {
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_001();
	
	BACK_UP_MAX_PAGE		= sysConfig_GetBackUpPageNum();
	FLASH_PROTECT_PAGE_NUM	= sysConfig_GetFlashProtectPageNum();
	BACK_UP_AREA_START		= MEMtPN8(U32_S_U16(sysConfig_GetNvmTop(), BACK_UP_MAX_SIZE));//lmc

	gBackUp_inSingleApdu	= false;
	gBackUp_inMutilApdu		= false;
	gBackUp_cacheType			= 0;
	gBackUp_writeType			= 0;
	gBackUp_curSize			= 0;
	gBackUp_curOffset		= 0;
	gBackUp_PageNumStart	= TF_BACK_UP_NONE;
	gBackUp_NotBackUpBottom        = 0;
	gBackUp_NotBackUpTop        = 0;
	
	FlashBackUp_Init();
    E2P_stream_flush_clear();
	
    if(BACK_UP_MAX_PAGE > MAX_BACK_UP_PAGE)
    {
        while(1);
    }
}

void clearBackUpPageFlag(void) {
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_002();
	
	FlashBackUp_CommitFlashFlag();
	if (gBackUp_PageNumStart < BACK_UP_MAX_PAGE)
	{
		if(FlashBackUp_IsSurpport())
		{
			FlashBackUp_commitPageInfo(gBackUp_TempBuf);
		}
		else
		{
			ecpMem_Copy(gBackUp_TempBuf, PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), U8_M_U16(gBackUp_PageNumStart , EEPAGE_SIZE)),
						BACK_UP_PAGE_HEADER_SIZE);
			ecpMem_Set(gBackUp_TempBuf, TF_COMMIT, TF_FLAG_SIZE);
			setBackUpPageCrc(gBackUp_TempBuf, calBackUpPageCrc(gBackUp_TempBuf));
			MemNvmOperation_WriteErase(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), U8_M_U16(gBackUp_PageNumStart , EEPAGE_SIZE)),
								  gBackUp_TempBuf, BACK_UP_PAGE_HEADER_SIZE);
		}
		gBackUp_PageNumStart = TF_BACK_UP_NONE;

		gBackUp_curSize			= 0;
		gBackUp_curOffset		= 0;

	}
}

static u8 getFirstBackUpPage(void) {
	PN8 tempBackUpPage;
	u8	tempCounter;
	u8	maxCounter;
	u8	i;

	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_003();
	
	SC_deactiveEDC();	
	tempBackUpPage = MEMtPN8(BACK_UP_AREA_START);
	maxCounter = 0;
	tempCounter = 0;

	gBackUp_curSize = 0;
	gBackUp_PageNumStart = 0;
	gBackUp_curOffset = 0;

	for(i = 0;i < BACK_UP_MAX_PAGE; U8_INC(i))
	{
		if(isBackUpPageCommit(tempBackUpPage))
		{
			if(calBackUpPageCrc(tempBackUpPage) == getBackUpPageCrc(tempBackUpPage))
			{
				tempCounter = getBackUpPageCounter(tempBackUpPage);
				if((tempCounter <= MAX_BACK_UP_COUNT) && (tempCounter > maxCounter))
				{
					maxCounter = tempCounter;
					gBackUp_PageNumStart = i;
				}
			}
		}
		tempBackUpPage = PN8_A_CONST(tempBackUpPage, EEPAGE_SIZE);
	}

	if(MAX_BACK_UP_COUNT == maxCounter)
	{
		maxCounter = 0;
		tempBackUpPage = MEMtPN8(BACK_UP_AREA_START);

		for(i = 0;i < BACK_UP_MAX_PAGE; U8_INC(i))
		{
			if(isBackUpPageCommit(tempBackUpPage))
			{
				if(calBackUpPageCrc(tempBackUpPage) == getBackUpPageCrc(tempBackUpPage))
				{
					tempCounter = getBackUpPageCounter(tempBackUpPage);
					if((tempCounter <= BACK_UP_MAX_PAGE) && (tempCounter > maxCounter))
					{
						maxCounter = tempCounter;
						gBackUp_PageNumStart = i;
					}
				}
			}
			tempBackUpPage = PN8_A_CONST(tempBackUpPage, EEPAGE_SIZE);
		}
	}

	//find the max page, get the next page as the back up page
	U8_INC(gBackUp_PageNumStart);
	if(BACK_UP_MAX_PAGE == gBackUp_PageNumStart)
	{
		gBackUp_PageNumStart = 0;
	}

	U8_INC(maxCounter);

	SC_activeEDC();
	return maxCounter;
}

static u16 eraseBackUpPage(u16 offset, u16 bufOffset, u16 length, u8 isPart1) {
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_004();
	
	if (BLOCK_PART_1 == isPart1) {
		if (length >= EEPAGE_SIZE) {
			length		=  EEPAGE_SIZE;
		}
		else
		{
			bufOffset = U16_A_U16(bufOffset, length);
		}
	} else {
		ecpMem_Copy(PX8_A_CONST(gBackUp_TempBuf, BACK_UP_PAGE_BLOCK_START_OFFSET + BACK_UP_BLOCK_ADDR_OFFSET),
				PX8_A_U16(gBackUp_TempBuf, bufOffset), length);
		bufOffset = U8_A_U16(BACK_UP_PAGE_BLOCK_START_OFFSET + BACK_UP_BLOCK_ADDR_OFFSET, length);
	}

	//erase other content of the gBackUp_TempBuf
	//length = U16_S_U16(BACK_UP_BUFFER_SIZE, bufOffset);
	//ecpMem_Set(PX8_A_U16(gBackUp_TempBuf, bufOffset), 0x00, U16_S_U16(BACK_UP_BUFFER_SIZE, bufOffset));
	if(FlashBackUp_IsSurpport())
	{
		MemNvmOperation_Erase(PN8_A_U32(BACK_UP_AREA_START, U32_AND_U32(offset , NOR_EEPAGE_MASK)), EEPAGE_SIZE);
	}
	else
	{
		if(EEPAGE_SIZE != length)
		{
			//erase other content of the gBackUp_TempBuf
			ecpMem_Set(PX8_A_U16(gBackUp_TempBuf, bufOffset), 0x00, U16_S_U16(BACK_UP_BUFFER_SIZE, bufOffset));
		}
	}
	
	
	return length;
	
	
}

static void saveBackUpData(u8 bufOffset, u16 length,u1 isfirstblockflag)
{
	u16 X_A tempLength;
	u16 X_A tempOffset;

	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_005();
	
	//check the block is splited in two page or not, if so, save the part1 of block
	tempLength = U16_S_U16(EEPAGE_SIZE,
			U16_AND_U16(gBackUp_curOffset, EEPAGE_MASK));
	if (length > tempLength) {
		length = U16_S_U16(length, tempLength);
	} else {
		tempLength = length;
		length = 0;
	}

	SC_deactiveEDC();
	//if back up start at a new page, earse this page first.
	if (0 == U16_AND_U16(gBackUp_curOffset, EEPAGE_MASK)) {
		tempLength = eraseBackUpPage(gBackUp_curOffset,bufOffset, tempLength, BLOCK_PART_1);
		if( BACK_UP_PAGE_START_OFFSET == bufOffset )
		{
			FlashBackUp_SetPageInfo(gBackUp_TempBuf, PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset));
			if( FlashBackUp_IsSurpport() )
			{
				ecpMem_Copy(PX8_A_U16(gBackUp_TempBuf, BACK_UP_PAGE_COMMIT_FLAG_OFFSET), PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), U16_A_U16(gBackUp_curOffset, BACK_UP_PAGE_COMMIT_FLAG_OFFSET)), BACK_UP_PAGE_COMMIT_FLAG_SIZE);
			}
		}
	}
	//save back up block
	if(isfirstblockflag)
	{
		if(FlashBackUp_IsSurpport())
		{
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset + BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE),
					PX8_A_U16(gBackUp_TempBuf, bufOffset + BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE), (tempLength - (BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE)));
		}
		else
		{
		//写入的长度应是期望长度，不是整页长度。dengyiwen
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset),
					PX8_A_U16(gBackUp_TempBuf, bufOffset), tempLength);
		}
	}
	else
	{
		if(FlashBackUp_IsSurpport())
		{
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset+ BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE),
							PX8_A_U16(gBackUp_TempBuf, bufOffset+ BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE), tempLength - (BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE));
		}
		else
		{
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset),
				PX8_A_U16(gBackUp_TempBuf, bufOffset), tempLength);
		}
	}


	//then if part 2 present, save the part 2
	if (length > 0) {
		//if curOffset point to the last page, then page 0 is the next page
		if (U16_S_U16(BACK_UP_MAX_SIZE, gBackUp_curOffset) <= EEPAGE_SIZE) {
			tempOffset = 0;
		} else {
			tempOffset = U16_A_U16(gBackUp_curOffset, tempLength);
		}

		//erase next page first
		length = eraseBackUpPage(tempOffset, U16_A_U16(bufOffset, tempLength), length, BLOCK_PART_2);

		//back up part 2 data
		//修改eeprom事务方案跨页写偏移错误。dengyiwen
		programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), tempOffset),
				PX8_A_CONST(gBackUp_TempBuf, BACK_UP_PAGE_BLOCK_START_OFFSET + BACK_UP_BLOCK_ADDR_OFFSET),
				length);
	}

	if(FlashBackUp_IsSurpport())
	{
		if(isfirstblockflag)
		{
			//跳过写提交标志，不将擦过的值写入提交标志，避免有些芯片不能再写。
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset),
			PX8_A_U16(gBackUp_TempBuf, BACK_UP_PAGE_START_OFFSET), BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE);
		
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset + BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE + BACK_UP_PAGE_COMMIT_FLAG_SIZE),
					PX8_A_U16(gBackUp_TempBuf, BACK_UP_PAGE_START_OFFSET + BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE + BACK_UP_PAGE_COMMIT_FLAG_SIZE), BACK_UP_PAGE_COMMIT_CRC_SIZE\
					+ BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE);
		}
		else
		{
			programBackUpBlock(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), gBackUp_curOffset),
							PX8_A_U16(gBackUp_TempBuf, BACK_UP_PAGE_HEADER_SIZE), BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE);
		}	
	}
	SC_activeEDC();	
}

static void alignBackUpData(PR32 desVar, PX16 lengthVar)
{
	memref X_A tempAddr;
	u16 X_A tempLength;

	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_006();
	
	//make sure the address is EECELL_SIZE aligned and length is multiple of EECELL_SIZE
//	tempAddr    = PR_R_U32(desVar);
//	tempLength  = PX_R_U16(lengthVar);
//	tempLength  = U8_A_U16(U32t8(U32_AND_U32(tempAddr, EECELL_MASK)), tempLength);
//	tempLength  = U16_AND_U16(U8_A_U16(EECELL_MASK, tempLength), NOR_EECELL_MASK);
//	PR_W_U32(desVar, U32_AND_U32(tempAddr, NOR_EECELL_MASK));
//	PX_W_U16(lengthVar, tempLength);	
	tempAddr    = desVar[0];
	tempLength  = lengthVar[0];
	tempLength  = U8_A_U16(U32t8(U32_AND_U32(tempAddr, EECELL_MASK)), tempLength);
	tempLength  = U16_AND_U16(U8_A_U16(EECELL_MASK, tempLength), NOR_EECELL_MASK);
	desVar[0]	= U32_AND_U32(tempAddr, NOR_EECELL_MASK);
	lengthVar[0]= tempLength;
}

static u16 getNextBlockOffset(u16 curOffset, u16 dataLength, u8 type) {
	PN8 X_A tempBackUpBlock;
	u16 X_A tempOffset;
	PN8 X_A tempAddr;
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_007();
	
	tempBackUpBlock = PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), curOffset);
	if((WRITEEP_NOATOMIC == U8_AND_U8(U16t8(U16_SHR_U8(getBackUpBlockTypeSize(tempBackUpBlock),BACK_UP_BLOCK_TYPE_SHIFT_BIT)), BACK_UP_BLOCK_TYPE_MASK))
			&& (GET_NEW_BLOCK_OFFSET == type))
	{
		return curOffset;
	}

	//get next block offset
	tempOffset = U16_AND_U16(getBackUpBlockTypeSize(tempBackUpBlock), BACK_UP_BLOCK_LENGTH_MASK);
	tempAddr = MEMtPN8(getBackUpBlockAddress(tempBackUpBlock));
	alignBackUpData(MEMtPR32(&tempAddr), MEMtPX16(&tempOffset));
	tempOffset = U8_A_U16(BACK_UP_BLOCK_HEADER_SIZE, tempOffset);
	tempOffset = U16_AND_U16(U8_A_U16(EECELL_MASK, tempOffset), NOR_EECELL_MASK);
	curOffset = U16_A_U16(tempOffset, curOffset);
	
	if(SEARCH_NEXT_BLOCK == type)
	{
		//check the next block is placed in next page or not
		if(U32_AND_U32(curOffset, NOR_EEPAGE_MASK) != U32_AND_U32(U16_A_U16(curOffset, (BACK_UP_PAGE_FRAG - 1)), NOR_EEPAGE_MASK))
		{
			curOffset = U32t16(U32_AND_U32(U16_A_U16(curOffset, EEPAGE_MASK), NOR_EEPAGE_MASK));
		}
	}

	if(curOffset >= BACK_UP_MAX_SIZE)
	{
		curOffset = U16_S_U16(curOffset,BACK_UP_MAX_SIZE);
	}

	if(GET_NEW_BLOCK_OFFSET == type)
	{
		dataLength = U8_A_U16(BACK_UP_BLOCK_HEADER_SIZE, dataLength);
		dataLength = U16_AND_U16(U8_A_U16(EECELL_MASK, dataLength), NOR_EECELL_MASK);

		//check the reamin size of back up page is enough or not
		tempOffset = U16_AND_U16(curOffset, EEPAGE_MASK);
		if(U16_A_U16(dataLength , tempOffset) > EEPAGE_SIZE)
		{
			if((dataLength <= BACK_UP_PAGE_FRAG) || (U16_S_U16(EEPAGE_SIZE, tempOffset) <= BACK_UP_BLOCK_HEADER_SIZE))
			{
				dataLength = U16_A_U16(dataLength, U16_S_U16(EEPAGE_SIZE, tempOffset));

				//if curOffset point to the last page, then page 0 is the next page
				if(U16_S_U16(BACK_UP_MAX_SIZE, curOffset) < EEPAGE_SIZE)
				{
					curOffset = 0;
				}
				else
				{
					curOffset = U16_AND_U16(U16_A_U16(EEPAGE_MASK, curOffset), NOR_EEPAGE_MASK);
				}
			}
		}

		gBackUp_curSize = U16_A_U16(gBackUp_curSize, dataLength);
		if(gBackUp_curSize > BACK_UP_MAX_SIZE)
		{
			if(!sysConfig_IsTranExceptionOpen())
			{
				bootErrorHandle_ThrowError(TRANSACTION_BUFFER_OVERFLOW);
			}
		}
	}

	return curOffset;

}

static u16 calBackUpBlockCrc(u16 length, u8 counter) {
	setBackUpBlockCounter(PX8_A_U16(gBackUp_TempBuf, length), counter);

	length =
			U16_A_U16(
					(BACK_UP_BLOCK_HEADER_SIZE - BACK_UP_BLOCK_CRC_SIZE + BACK_UP_PAGE_COUNTER_SIZE),
					length);
	return Get_crc16(
			PX8_A_CONST(gBackUp_TempBuf,
					(BACK_UP_PAGE_BLOCK_START_OFFSET + BACK_UP_BLOCK_CRC_SIZE)),
			length);
}

void copyBackUpBlockInfo(PN8 blockInfoAddress, u16 eepBufferOffset, u16 blockInfoOffset, u16 InfoLength, PX8 tempBackUpTempBuf)
{
	u16 tempLength;
	
	//part2 of block is saved in the page 0
	if(U16_A_U16(blockInfoOffset, InfoLength) > BACK_UP_MAX_SIZE)
	{
		if(blockInfoOffset > BACK_UP_MAX_SIZE)
		{
			tempLength =	U16_S_U16(blockInfoOffset, BACK_UP_MAX_SIZE);
			ecpMem_Copy(PX8_A_U16(tempBackUpTempBuf,eepBufferOffset),
						PN8_A_U16(MEMtPM8(BACK_UP_AREA_START), tempLength),
						InfoLength
						);
		}
		else
		{
			tempLength		= U16_S_U16(BACK_UP_MAX_SIZE, blockInfoOffset);
			ecpMem_Copy(PX8_A_U16(tempBackUpTempBuf, eepBufferOffset),
						blockInfoAddress,
						tempLength
						);
			ecpMem_Copy(PX8_A_U16(PX8_A_U16(tempBackUpTempBuf,eepBufferOffset), tempLength),
						MEMtPM8(BACK_UP_AREA_START),
						U16_S_U16(InfoLength, tempLength)
						);
		}
	}
	else
	{
		ecpMem_Copy(PX8_A_U16(tempBackUpTempBuf, eepBufferOffset),
					blockInfoAddress,
					InfoLength
					);
	}
}

u16 searchBlocks(PN8 backUpDes, u16 backUpLength, u16 lastFitOffset, u8 type)
{
	PN8 X_A tempBackUpDesTop;
	PN8 X_A tempAddr;
	PN8 X_A BackUpTempAddr;
	u16 X_A curOffset, tempLength, preFitOffset;
	u8 X_A backUpType, backUpCounter;
	u1  B_A isSearchNextPage;
	u16	X_A	tempPreBlockOffset;			//当前无效备份块的前一备份块偏移
	u16	X_A	tempPrePreBlockOffset;		//当前无效备份块的前一备份块的前一备份块偏移
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_008();
	
	tempBackUpDesTop = PN8_A_U16(backUpDes, backUpLength);
	isSearchNextPage = false;
	preFitOffset = ILLEGAL_BLOCK_OFFSET;
	tempPreBlockOffset		= ILLEGAL_BLOCK_OFFSET;
	tempPrePreBlockOffset = ILLEGAL_BLOCK_OFFSET;
	
	//point to first block
	tempAddr = PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), U8_M_U16(gBackUp_PageNumStart, EEPAGE_SIZE));
	backUpCounter = getBackUpPageCounter(tempAddr);
	tempAddr = PN8_A_CONST(tempAddr, BACK_UP_PAGE_HEADER_SIZE);
	curOffset = U8_A_U16(BACK_UP_PAGE_BLOCK_START_OFFSET, U8_M_U16(gBackUp_PageNumStart, EEPAGE_SIZE));

	do
	{
		backUpType = U8_AND_U8(U16t8(U16_SHR_U8(getBackUpBlockTypeSize(tempAddr), BACK_UP_BLOCK_TYPE_SHIFT_BIT)), BACK_UP_BLOCK_TYPE_MASK);
		//search blocks in the page
		while((WRITEEP_TRANSACTION == backUpType) || (WRITEEP_NOATOMIC == backUpType) || (WRITEEP_ATOMIC == backUpType))
		{
			if( (CHECK_IS_ROLL_BACK == type) || ((GET_LAST_BLOCK == type) && (lastFitOffset == curOffset)))
			{
				//buffer the block in the gBackUp_TempBuf
				backUpLength = U16_AND_U16(getBackUpBlockTypeSize(tempAddr), BACK_UP_BLOCK_LENGTH_MASK);
				BackUpTempAddr = MEMtPN8(getBackUpBlockAddress(tempAddr));
				alignBackUpData(MEMtPR32(&BackUpTempAddr), MEMtPX16(&backUpLength));
				//illegal length
				if(backUpLength > EEPAGE_SIZE)
				{
					return lastFitOffset;
				}

				tempLength			= U16_A_U16(backUpLength, BACK_UP_BLOCK_HEADER_SIZE);



				copyBackUpBlockInfo(tempAddr, BACK_UP_PAGE_BLOCK_START_OFFSET, curOffset, tempLength, gBackUp_TempBuf);
				
				if((GET_LAST_BLOCK == type) && (PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), tempPreBlockOffset) == tempAddr))//如果条件成立，说明有一个备份块无效,就返回上一个备份块
				{
					return	preFitOffset;
				}

				if(GET_LAST_BLOCK == type)
				{
					if(calBackUpBlockCrc(backUpLength, backUpCounter) != getBackUpBlockCrc(PN8_A_U16(BACK_UP_AREA_START, curOffset)))
					{
						if(lastFitOffset != U8_A_U16(BACK_UP_PAGE_BLOCK_START_OFFSET, U8_M_U16(gBackUp_PageNumStart, EEPAGE_SIZE)))
						{
							lastFitOffset	= tempPreBlockOffset;
							curOffset		= tempPreBlockOffset;
							tempAddr		= PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), tempPreBlockOffset);
							preFitOffset	= tempPrePreBlockOffset;
							continue;
						}
						else
						{
							return FIRST_BLOCK_IS_ERROR;
						}
					}
					else
					{
				 		return preFitOffset;
					}
				}

				//check the back up block is legal or not
				if(calBackUpBlockCrc(backUpLength, backUpCounter) != getBackUpBlockCrc(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), curOffset)))
				{
					return lastFitOffset;
				}

				lastFitOffset = curOffset;
			}

			if((CHECK_IS_BACK_UP == type) || (READ_RECENT_BACK_UP_DATA == type) || (READ_PRE_BACK_UP_DATA == type))
			{
				backUpLength = U16_AND_U16(getBackUpBlockTypeSize(tempAddr), BACK_UP_BLOCK_LENGTH_MASK);
				tempAddr = MEMtPN8(getBackUpBlockAddress(tempAddr));

				if(CHECK_IS_BACK_UP == type)
				{
					if((backUpDes >= tempAddr) && (tempBackUpDesTop <= PN8_A_U16(tempAddr, backUpLength)))
					{
						return ALREADY_BACK_UP;	//the data is already back up
					}
				}

				if((READ_RECENT_BACK_UP_DATA == type) || ((READ_PRE_BACK_UP_DATA == type) && (curOffset != lastFitOffset)))
				{
					if(!((tempBackUpDesTop <= tempAddr) || (backUpDes >= PN8_A_U16(tempAddr, backUpLength))))
					{
						preFitOffset = curOffset;//parts of data is already back up
					}
				}

				if(curOffset == lastFitOffset)
				{
					if(CHECK_IS_BACK_UP == type)
					{
						return NOT_BACK_UP_YET;	//the data is mot back up yet
					}
					else
					{
						return preFitOffset;
					}
				}
			}
			else if(GET_LAST_BLOCK == type)
			{
				tempPreBlockOffset 	= curOffset;
				tempPrePreBlockOffset = preFitOffset;
				preFitOffset = curOffset;
			}
			else
			{
				;
			}
			curOffset = getNextBlockOffset(curOffset, 0, SEARCH_NEXT_BLOCK);
			tempAddr = PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), curOffset);
			backUpType = U8_AND_U8(U16t8(U16_SHR_U8(getBackUpBlockTypeSize(tempAddr), BACK_UP_BLOCK_TYPE_SHIFT_BIT)), BACK_UP_BLOCK_TYPE_MASK);
			isSearchNextPage = false;
		} //end while

		if(true == isSearchNextPage)
		{
			//there is no legal block in the next page
			break;
		}

		//is there a legal block in the next page?
		tempLength = U16_S_U16(BACK_UP_MAX_SIZE, curOffset);
		if(tempLength < EEPAGE_SIZE)
		{
			curOffset = 0;
		}
		else
		{
			curOffset = U16_AND_U16(U16_A_U16(curOffset, EEPAGE_MASK), NOR_EEPAGE_MASK);
		}
		tempAddr = PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), curOffset);
		isSearchNextPage = true;

	}while(true == isSearchNextPage);

	if(CHECK_IS_BACK_UP == type)
	{
		return NOT_BACK_UP_YET;
	}
	else if((READ_RECENT_BACK_UP_DATA == type) || (READ_PRE_BACK_UP_DATA == type))
	{
		return ILLEGAL_BLOCK_OFFSET;
	}
	else
	{
		return lastFitOffset;
	}

}

void save_E2P(memref e2p_addr, u16 length) {

	u8 D_A BackUpPageCounter;
	u1 B_A isFirstBlock;
	memref BackupE2p_addr;
	u16 BackupLength;

	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_009();
	
	/* do not back up any data when creating cardmanager.if tearing in the cardmanager creating, clear all eeprom*/
	

	if (BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE) 
	{
		return;
	}



	/*
	 * Checks if the data to be backed up is within a block of memory that
	 * is allocated within the same transaction. If so, do not back up the data
	 */
	BackupE2p_addr=e2p_addr;
	BackupLength=length;

	alignBackUpData(MEMtPR32(&e2p_addr), MEMtPX16(&length));
	
	if(WRITEEP_LOST == getCacheDataType())
	{
		if( checkIsBackUp(e2p_addr, length) )
		{
			return;
		}
	}
	
	FlashBackUp_PageProtect(e2p_addr, gBackUp_TempBuf);
	
	if(isInProgress() && checkIsBackUp(e2p_addr, length))
	{
		return;
	}
	
	if (((WRITEEP_NOATOMIC == gBackUp_cacheType) && (!BOOT__MEMORY__NOATOMIC_BACK_UP)) || ((WRITEEP_ATOMIC == gBackUp_cacheType) && (!BOOT__MEMORY__ATOMIC_BACK_UP)))
    {
		return;
	}

	if(WRITEEP_LOST == getCacheDataType())
	{
		return;
	}
	
	if ((TF_BACKUP_First != gBackUp_PageNumStart) && (TF_BACK_UP_NONE != gBackUp_PageNumStart))
    {
		SC_deactiveEDC();
		if (ALREADY_BACK_UP == searchBlocks(MEMtPN8(BackupE2p_addr), BackupLength, gBackUp_curOffset, CHECK_IS_BACK_UP))
        {
			SC_activeEDC();
			return;
		}

		gBackUp_curOffset = getNextBlockOffset(gBackUp_curOffset, length, GET_NEW_BLOCK_OFFSET);
		BackUpPageCounter = getBackUpPageCounter(PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), U8_M_U16(gBackUp_PageNumStart, EEPAGE_SIZE)));
		SC_activeEDC();
		isFirstBlock = false;
	} 
    else 
    {
		/* construct back up page information */
		BackUpPageCounter = getFirstBackUpPage();
		setBackUpPageFlag(gBackUp_TempBuf, TF_BACKUP);
		setBackUpPageCounter(gBackUp_TempBuf, BackUpPageCounter);
		setBackUpPageCrc(gBackUp_TempBuf, calBackUpPageCrc(gBackUp_TempBuf));
		gBackUp_curOffset = U8_M_U16(gBackUp_PageNumStart, EEPAGE_SIZE);
		isFirstBlock = true;
	}

	/* construct back up block information */
	setBackUpBlockTypeSize(gBackUp_TempBuf, U16_OR_U16(U16_SHL_U8(gBackUp_cacheType, BACK_UP_BLOCK_TYPE_SHIFT_BIT), BackupLength));
	setBackUpBlockAddress(gBackUp_TempBuf, BackupE2p_addr);
	setBackUpBlockContent(gBackUp_TempBuf, MEMtPM8(e2p_addr), length);
	setBackUpBlockCrc(gBackUp_TempBuf, calBackUpBlockCrc(length, BackUpPageCounter));

	if (isFirstBlock) 
    {
		length = U8_A_U16(U8_A_U8(BACK_UP_PAGE_HEADER_SIZE, BACK_UP_BLOCK_HEADER_SIZE), length);
		saveBackUpData(BACK_UP_PAGE_START_OFFSET, length,isFirstBlock);
		gBackUp_curOffset = U8_A_U16(BACK_UP_PAGE_BLOCK_START_OFFSET, gBackUp_curOffset);
		gBackUp_curSize = U16_AND_U16(U8_A_U16(EECELL_MASK, length), NOR_EECELL_MASK);
	} 
    else 
    {
		length = U8_A_U16(BACK_UP_BLOCK_HEADER_SIZE, length);
		saveBackUpData(BACK_UP_PAGE_BLOCK_START_OFFSET, length,isFirstBlock);
	}

}

void commitSaveE2p(void)
{
	if((getCacheDataType() == WRITEEP_NOATOMIC) && (getWriteEepType() == WRITEEP_TRANSACTION) && (isInProgress()))
	{
		FlashBackUp_CommitFlashFlag();
	}
	
	if ((WRITEEP_ATOMIC == getCacheDataType()) || ((WRITEEP_NOATOMIC == getCacheDataType()) && (!isInProgress())))
	{
		clearBackUpPageFlag();
		closeProgress();
	}
}

void memSetNoBackAreaPointer(PF8 maxEntryAddr, PF8 usedHeapAddr) {
	gBackUp_NotBackUpBottom 	= MEMtPN8(MBUS_getNotBackUpBottom(maxEntryAddr));
	gBackUp_NotBackUpTop 		= MEMtPN8(MBUS_getNotBackUpTop(usedHeapAddr));
}

//u1 isInProgress(void) {
//	return (gBackUp_inSingleApdu || gBackUp_inMutilApdu);
//}

//void setWriteEepType(u8 typeValue) {
//	gBackUp_writeType = typeValue;
//}
//void setCacheDataType(u8 typeValue) {
//	gBackUp_cacheType = typeValue;
//}

//u8 getWriteEepType(void) {
//	return gBackUp_writeType;
//}

//u8 getCacheDataType(void) {
//	return gBackUp_cacheType;
//}

//u1 isWriteAndCacheTheSame(void) {
//	return (gBackUp_writeType == gBackUp_cacheType);
//}

//u1 isInSingleApduProgress(void) {
//	return gBackUp_inSingleApdu;
//}

//u1 isInMultiApduProgress(void) {
//	return gBackUp_inMutilApdu;
//}

//void closeProgress(void) {
//	gBackUp_inSingleApdu = false;
//	gBackUp_inMutilApdu = false;
//}

//void openBackUpPageNumStart(void) {
//	gBackUp_PageNumStart = TF_BACKUP_First;
//}

//u16 get_max_commit_capacity(void) {
//	return BACK_UP_MAX_SIZE;
//}
//u16 get_unused_commit_capacity(void) {
//	return U16_S_U16(BACK_UP_MAX_SIZE, gBackUp_curSize);
//}
//memref getBackupStart(void) {
//	return PNtMEM(BACK_UP_AREA_START);
//}

//u8 getBackUpMaxPage(void) {
//	return BACK_UP_MAX_PAGE;
//}

#endif

#endif  //TRANSACTION_OLDDESIGN
