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
//  modify by dengyiwen@20220106

#if  !TRANSACTION_OLDDESIGN

/************************************************根据芯片配置全局变量*****************************************************************************
 *************************************************************************************************************************************************/
#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
u8 			X_A									gFlash_Page1Num;
u8			X_A									gFlash_FlagNum;
u8 			X_A									gFlash_CurPage;
u16			X_A									gFlash_FlagBlockOffset;
u16			X_A									gFlash_BackUpFlagOffset;
#else
u8			X_A									gFlash_FlagNum;										//事务备份区大小
u16 												g_BackUpPage_BlockOffset;					//记录flag页内块的偏移
u8 													g_BackUpPageNum;										//备份数据的页数
u16 												g_FlagCommitOffset;								//flag页内已提交的块偏移
u16			X_A									gFlash_FlagBlockOffset;						//块偏移
u16			X_A									gFlash_BackUpFlagOffset;					//回滚时使用
u8 			X_A									gFlash_Page1Num;									//回滚时使用
u8 			X_A									gFlash_CurPage;
u8 													g_BackUpPage_Index;               	//页保护区轮询索引，指向最新写过的一页
u8 													g_BackUpPage_FlagIndex;						//页保护区flag页索引

#if LIFE_OR_PERFORMANCE
PN8 gFlashBackUpAreaStart;
PN8 gFlashBackUpFlagStart;
#endif

typedef struct PageFlagCount
{
	u8  Index;
	u16 Count;
}PageFlagCount;
#endif



/************************************************公共接口代码**************************************************************************************
 *************************************************************************************************************************************************/ 
 
 
 
 /***************************************************************************
 * 函数名 ：FlashBackUp_clearFlagArea
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：擦除页保护flag
 * 备注		：
 *
 ****************************************************************************/
 void FlashBackUp_clearFlagArea(void)
{
	u8 tempbuffer[4];	//forM25
	
	#if LIFE_OR_PERFORMANCE
	PN8 pageflagaddr;
	#endif
	
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	  ecpMem_Set(tempbuffer, 0xFF, sizeof(tempbuffer));
		MemNvmOperation_Erase(PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)), EEPAGE_SIZE);
		MemNvmOperation_WriteOnly(PN8_A_U16(PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)),U16_S_U8(EEPAGE_SIZE, 4)),tempbuffer,0x04);	//forM25
	
	#else
	#if LIFE_OR_PERFORMANCE
		pageflagaddr = PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(g_BackUpPage_Index, EEPAGE_SIZE));
		MemNvmOperation_Erase(pageflagaddr,EEPAGE_SIZE);
	#endif
		g_FlagCommitOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
		g_BackUpPage_BlockOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;

	#endif
	gFlash_BackUpFlagOffset		= 0;
	gFlash_FlagBlockOffset		= 0;
	return;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_calcFlagCrc
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：计算CRC接口
 * 备注		：
 *
 ****************************************************************************/
static u32 FlashBackUp_calcFlagCrc(PN8 flagAddress)
{
	//Get_crc16
	u8 tempbuffer[FLASH_FLAG_INFO_SIZE - FLASH_FLAG_INFO_CRC_SIZE];
	u8 templen;
	
	templen = FLASH_FLAG_INFO_SIZE - FLASH_FLAG_INFO_CRC_SIZE - FLASH_FLAG_INFO_FIN_SIZE;
	
	ecpMem_Copy(tempbuffer,flagAddress,templen);
	ecpMem_Copy(PN8_A_U8(tempbuffer,templen),PN8_A_U16(flagAddress,FLASH_FLAG_INFO_FIN_OFFSET),FLASH_FLAG_INFO_FIN_SIZE);
	
	return (u32)(Get_crc16(tempbuffer, U8_S_U8(FLASH_FLAG_INFO_SIZE, FLASH_FLAG_INFO_CRC_SIZE)));
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_GetNextBackUpCurPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：获取下一个将使用的页
 * 备注		：
 *
 ****************************************************************************/
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



 /***************************************************************************
 * 函数名 ：getFlashFlagInfoFinish_impl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：实现判断Flag是否提交
 * 备注		：
 *
 ****************************************************************************/
u32 getFlashFlagInfoFinish_impl(PN8 flashFlagAddress)
{
	u8 tempbuffer[FLASH_FLAG_INFO_FIN_SIZE];
	u8 count;
	
	ecpMem_Copy(tempbuffer,PN8_A_U16(flashFlagAddress, FLASH_FLAG_INFO_FIN_OFFSET),FLASH_FLAG_INFO_FIN_SIZE);
	
	for(count=0;count<FLASH_FLAG_INFO_FIN_SIZE;count++)
	{
		if(tempbuffer[count] != BACK_UP_PAGE_COMMIT_BYTE)
		{
			return FLASH_FLAG_NOT_FIN;
		}
	}
	return FLASH_FLAG_FIN;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_Init
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页保护全局变量初始化
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_Init(void)
{
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
		BACK_UP_MAX_PAGE			= U8_S_U8(BACK_UP_MAX_PAGE , U8_A_U8(FLASH_PROTECT_PAGE_NUM , 1)); //事务备份区 ： 配置区配置断电保护区总大小 - （ 配置区页保护大小 + 页flag大小 ）
	#else
		BACK_UP_MAX_PAGE			= U8_S_U8(BACK_UP_MAX_PAGE , FLASH_PROTECT_PAGE_NUM); //事务备份区 ： 配置区配置断电保护区总大小 -  配置区页保护大小
	#endif
	gFlash_FlagNum				= BACK_UP_MAX_PAGE;																		
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	
		gFlash_Page1Num		= U8_A_U8(BACK_UP_MAX_PAGE ,1);	//字写方案FLAG页要单独使用一页
	
	#else
	
		gFlash_Page1Num		= BACK_UP_MAX_PAGE;//页擦写方案FLAG页和数据混用
	#if !LIFE_OR_PERFORMANCE	
		if(U8_R_U8(FLASH_PROTECT_PAGE_NUM,2))
		{
			while(1);//方案要求页数必须为偶数;
		}
	#else
		gFlashBackUpAreaStart = PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
    gFlashBackUpFlagStart = PN8_A_U32(gFlashBackUpAreaStart, U8_M_U16(U8_D_U8(FLASH_PROTECT_PAGE_NUM, 2), EEPAGE_SIZE));
		g_BackUpPage_Index = 0;
	#endif
	/*********************************/
	g_BackUpPageNum = 0;
	g_FlagCommitOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
	g_BackUpPage_BlockOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
	#endif
//	gFlash_CurPage			= 0;
//	gFlash_FlagBlockOffset			= 0;  //only init once
	
}



  /***************************************************************************
 * 函数名 ：FlashBackUp_PageProtect
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页保护的主要入口
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_PageProtect(memref backUpDes, PX8 tempBuffer)
{	
	PN8     	flashFlagPageAddress;
	u8			flashBackUpPage;
	
	SC_deactiveEDC();
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	
		FlashBackUp_PageProtect_Wordimpl(backUpDes, tempBuffer);//FlashBackUp_PageProtect_Pageimpl(backUpDes, tempBuffer);//FlashBackUp_PageProtect_Wordimpl(backUpDes, tempBuffer);
	
	#else
	
		FlashBackUp_PageProtect_Pageimpl(backUpDes, tempBuffer);
	
	#endif
	SC_activeEDC();	
	return;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_CommitFlashFlag
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页保护flag提交的主入口
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_CommitFlashFlag(void)
{

	SC_deactiveEDC();
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	
		FlashBackUp_CommitFlashFlag_Wordimpl();
	
	#else
	
		FlashBackUp_CommitFlashFlag_Pageimpl();
	#endif
	SC_activeEDC();	
	return;
}



 /***************************************************************************
 * 函数名 ：isWeakWrite
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：判断是否是弱写
 * 备注		：
 *
 ****************************************************************************/
u8 isWeakWrite(PN8 desAddr,PN8 tempbuffer)
{	
//	u8			flashPageProtectBuff[HAL_EEPAGE_SIZE];
	u8 			i;
	
	ecpMem_Copy(tempbuffer, desAddr, HAL_EEPAGE_SIZE);
	for(i = 0; i < 2; i++ )
	{
		if(ecpMem_Compare(desAddr, tempbuffer, HAL_EEPAGE_SIZE) != 0)
		{
			FlashBackUp_clearFlagArea();
			return true;
		}
	}
	return false;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_RollBackPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：回滚的代码
 * 备注		：
 *
 ****************************************************************************/
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
	
	if(isReset == ABORT_ON_RESET)
	{
		gFlash_CurPage			= 0;
		gFlash_FlagBlockOffset			= 0;
	}
	
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE//原本的页保护方案
	flashBackUpAddress	= PN8_A_U16(BACK_UP_AREA_START, (u16)U8_M_U16(gFlash_Page1Num, EEPAGE_SIZE));
	flashFlagAddress	= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	if(isReset == ABORT_ON_RESET)
	{
		if(isWeakWrite( flashFlagAddress, stream_buffer))//弱写返回
		{
				return;
		}
 	}
	flashFlagAddress  = stream_buffer;
	#else
	flashFlagAddress = FindValidFlagPage(stream_buffer);//找到有效的FLAG页接口内已经判断是否弱写，所以不再调用isWeakWrite接口
	if(flashFlagAddress == NULL)//找不到有效页返回
	{
		return;
	}
	flashFlagAddress  = stream_buffer + FLASH_PAGEFLAG_PAGEHEAD_SIZE;
	#endif
	//除了页擦写有页头，页保护块结构一致，所以回滚也一致，不改动。
	for(gFlash_BackUpFlagOffset = 0; gFlash_BackUpFlagOffset < FLASH_FLAG_MAX_OFFSET; )
	{
		desAddrss	= getFlashFlagInfoDes(flashFlagAddress);

		if(desAddrss != FLASH_FLAG_INFO_DES_NULL)
		{
			if((getFlashFlagInfoCrc(flashFlagAddress) == FlashBackUp_calcFlagCrc(flashFlagAddress))
			   && (getFlashFlagInfoFinish(flashFlagAddress) != FLASH_FLAG_FIN))
			{
				flashCurPage			= getFlashFlagInfoPage(flashFlagAddress);
//				if(flashCurPage > U8_S_U8(FLASH_PROTECT_PAGE_NUM, 1))
//				{
//					isFlagTableDirty	= true;
//				}
//				else
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



 /***************************************************************************
 * 函数名 ：GetPageProtectAddr
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：获取页保护地址
 * 备注		：
 *
 ****************************************************************************/
PN8 GetPageProtectAddr(PN8 tempbuffer)
{
	PN8	flashflagaddr;
	
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	flashflagaddr	= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	#else
	flashflagaddr = FindValidFlagPage(tempbuffer);
	#endif
		return flashflagaddr;
}



 /************************************************字写代码实现**************************************************************************************
 *************************************************************************************************************************************************/ 
 
#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
/***************************************************************************
 * 函数名 ：FlashBackUp_copyDesToBackPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：将目标区拷贝到备份区
 * 备注		：字写
 *
 ****************************************************************************/
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




 /***************************************************************************
 * 函数名 ：FlashBackUp_GetAlreadyBackUpPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：获取已经备份的数据的页索引
 * 备注		：字写
 *
 ****************************************************************************/
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



 /***************************************************************************
 * 函数名 ：FlashBackUp_PageProtect_Wordimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页保护字写实现
 * 备注		：
 *
 ****************************************************************************/
 void FlashBackUp_PageProtect_Wordimpl(memref backUpDes, PX8 tempBuffer)
{

	PN8     	flashFlagPageAddress;
	u8			flashBackUpPage;
	backUpDes	= U32_AND_U32(backUpDes, NOR_EEPAGE_MASK);
	flashBackUpPage		= FlashBackUp_GetAlreadyBackUpPage(backUpDes);
	if(FLASH_FLAG_INFO_ILLEGAL == flashBackUpPage)
	{
		//write des to backup flash page
		FlashBackUp_copyDesToBackPage(backUpDes, tempBuffer);
		flashBackUpPage	= gFlash_CurPage;
		FlashBackUp_GetNextBackUpCurPage();
		
		if((U16_D_U16(U16_S_U16(FLASH_FLAG_MAX_OFFSET, gFlash_FlagBlockOffset), FLASH_FLAG_INFO_SIZE) < FLASH_PROTECT_PAGE_NUM) || (U16_A_U16(gFlash_BackUpFlagOffset, FLASH_FLAG_INFO_SIZE) >= EEPAGE_SIZE))
		{
			FlashBackUp_clearFlagArea();
		}
		
		flashFlagPageAddress			= MEMtPN8(PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)));
		flashFlagPageAddress			= PN8_A_U16(flashFlagPageAddress, gFlash_BackUpFlagOffset);
		gFlash_BackUpFlagOffset		= U16_A_U16(gFlash_BackUpFlagOffset, FLASH_FLAG_INFO_SIZE);
		
		setFlashFlagInfoDes(tempBuffer, backUpDes);
		setFlashFlagInfoPage(tempBuffer, flashBackUpPage);
		setFlashFlagInfoPad(tempBuffer,FLASH_FLAG_INFO_PAD_SIZE);		
		setFlashFlagInfoNotFinish(tempBuffer,flashFlagPageAddress);
		setFlashFlagInfoCrc(tempBuffer, FlashBackUp_calcFlagCrc(tempBuffer));

		//write flag to flash
		MemNvmOperation_WriteOnly(PN8_A_U16(flashFlagPageAddress, FLASH_FLAG_INFO_DES_OFFSET), &tempBuffer[FLASH_FLAG_INFO_DES_OFFSET], (FLASH_FLAG_INFO_DES_SIZE + FLASH_FLAG_INFO_PAGE_SIZE + FLASH_FLAG_INFO_PAD_SIZE + FLASH_FLAG_INFO_CRC_SIZE));
	}

}



 /***************************************************************************
 * 函数名 ：FlashBackUp_CommitFlashFlag_Wordimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：提交字写实现
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_CommitFlashFlag_Wordimpl(void)
{
	u8	tempBuffer[FLASH_FLAG_INFO_SIZE];
	u16 tempOffset;
	PN8	flashFlagOffsetAddress;
	SC_deactiveEDC();
	flashFlagOffsetAddress		= PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	flashFlagOffsetAddress		= PN8_A_U16(flashFlagOffsetAddress, gFlash_FlagBlockOffset);
	flashFlagOffsetAddress		= PN8_A_U16(flashFlagOffsetAddress, FLASH_FLAG_INFO_FIN_OFFSET);
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



 /************************************************页擦写代码实现**************************************************************************************
 *************************************************************************************************************************************************/ 



#if !CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
 /***************************************************************************
 * 函数名 ：FlashBackUp_CopyDesToBackPage_Pageimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写拷贝目标区到备份区
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_CopyDesToBackPage_Pageimpl(memref backUpDes, PX8 tempBuffer)
{
	PN8			flashBackUpPageAddress;
	u16  tempindex;
	flashBackUpPageAddress = FindBackUpPage();
	ecpMem_Copy(tempBuffer, MEMtPN8(backUpDes), EEPAGE_SIZE);	
	MemNvmOperation_WriteErase(flashBackUpPageAddress, tempBuffer, EEPAGE_SIZE);
	g_BackUpPageNum++;
	return;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_PageProtect_Pageimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写实现页保护
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_PageProtect_Pageimpl(memref backUpDes, PX8 tempBuffer)
{	
	memref    tempaddr;
	
	tempaddr	= backUpDes;
	tempaddr	= U32_AND_U32(tempaddr, NOR_EEPAGE_MASK);							//做页对齐
	SC_deactiveEDC();
	#if !LIFE_OR_PERFORMANCE
	if(!IsAlreadyBackUp(tempaddr))																	//不是已经备份过的页
	{
		if(g_BackUpPageNum == MAXBACKUPNUM)																			//未提交的备份数据达到MAXBACKUPNUM的时候提交一次
		{
			FlashBackUp_CommitFlashFlag_Pageimpl();
		}
		
		FlashBackUp_CopyDesToBackPage_Pageimpl(tempaddr, tempBuffer);	//备份数据
		
		g_BackUpPage_Index++;														//备份完成将索引指向当前页
		if(g_BackUpPage_Index > MAXBACKUPINDEX)
		{
			g_BackUpPage_Index = 0;
		}
		
		FlashBackUp_WriteFlag(tempaddr,tempBuffer);			//写一个未提交的flag
	}
	#else
		if(g_BackUpPageNum == 1)																			//未提交的备份数据达到MAXBACKUPNUM的时候提交一次
		{
			FlashBackUp_CommitFlashFlag_Pageimpl();
		}
		FlashBackUp_CopyDesToBackPage_Pageimpl(tempaddr, tempBuffer);	//备份数据
		FlashBackUp_WriteFlag(tempaddr,tempBuffer);			//写一个未提交的flag
	#endif


	SC_activeEDC();	
	return;
}



 /***************************************************************************
 * 函数名 ：FindBackUpPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：找到备份页
 * 备注		：
 *
 ****************************************************************************/
PN8 FindBackUpPage()
{
	PN8 BackUpAddr;
	#if !LIFE_OR_PERFORMANCE
	u32 Index;
	
	Index = U8_A_U8(g_BackUpPage_Index, 1);
	
	if(Index > MAXBACKUPINDEX)
	{
		Index = 0;
	}
	
	BackUpAddr = PN8_A_U32(PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)), U8_M_U16(Index, EEPAGE_SIZE));
	#else
	BackUpAddr = PN8_A_U32(gFlashBackUpAreaStart, U8_M_U16(g_BackUpPage_Index, EEPAGE_SIZE));
	#endif
	return BackUpAddr;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_WriteFlag
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：写flag
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_WriteFlag(memref backUpDes,PX8 tempBuffer)
{
	u8	 TemBlockCrcBuf[12];
	u16  Count;
	PN8	 pageflagaddr;
	
	
	#if !LIFE_OR_PERFORMANCE
	pageflagaddr = PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));													//页保护起始地址
	if(g_BackUpPage_BlockOffset == FLASH_PAGEFLAG_PAGEHEAD_OFFSET)																									//没有block，重新拼页头，复位或者主动abort都会重新初始化g_BackUpPage_BlockOffset
	{
		ecpMem_Set(tempBuffer,0xFF,EEPAGE_SIZE);
		setPageProtectHeadFlag(tempBuffer);
		setPageProtectHeadNum(tempBuffer,1);
		if(g_BackUpPage_FlagIndex == PAGEPROTECTILLEGALADDR)																																						//没有flag页，count设置为0
		{
			setPageProtectHeadCount(tempBuffer,0);
		}
		else																																																					//有flag页，count从此flag页获取之后加一
		{
			Count = getPageProtectHeadCount(PN8_A_U32(pageflagaddr, U8_M_U16(g_BackUpPage_FlagIndex, EEPAGE_SIZE)));
			if(Count == MAXCOUNT)
			{
				Count = 0;
			}
			else
			{
				Count++;
			}
			setPageProtectHeadCount(tempBuffer,Count);
		}
			setPageProtectHeadCrc(tempBuffer);
	}
	else
	{
		ecpMem_Copy(tempBuffer,PN8_A_U32(pageflagaddr, U8_M_U16(g_BackUpPage_FlagIndex, EEPAGE_SIZE)),EEPAGE_SIZE);	//有block，拷贝已有的flag页修改其信息
		if(getPageProtectHeadCount(tempBuffer) == MAXCOUNT)																													//页头的count达到最大值，清0。
		{
			setPageProtectHeadCount(tempBuffer, 0);
		}
		else
		{
			setPageProtectHeadCount(tempBuffer,PM8_A_CONST(getPageProtectHeadCount(tempBuffer), 1));									//否则count加一
		}
		if(getPageProtectHeadNum(tempBuffer) == MAXNUM)																															//block块达到最大值，将页头num置一，并清除之前记录的block块
		{
			setPageProtectHeadNum(tempBuffer, 1);
			ecpMem_Set(&tempBuffer[FLASH_PAGEFLAG_PAGEHEAD_OFFSET],0xFF,U16_S_U8(EEPAGE_SIZE,FLASH_PAGEFLAG_PAGEHEAD_SIZE));
			g_BackUpPage_BlockOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
			g_FlagCommitOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
		}
		else
		{
			setPageProtectHeadNum(tempBuffer,PM8_A_CONST(getPageProtectHeadNum(tempBuffer), 1));
		}
		setPageProtectHeadCrc(tempBuffer);
	}
	/**********************************************写块**************************************************************/
	setPageProtectBlockDes(PM8_A_U16(tempBuffer, g_BackUpPage_BlockOffset),backUpDes);
	setPageProtectBlockIndex(PM8_A_U16(tempBuffer, g_BackUpPage_BlockOffset),g_BackUpPage_Index);
	
	PXB_W_U32(TemBlockCrcBuf,backUpDes);
	PXB_W_U32(&TemBlockCrcBuf[4],g_BackUpPage_Index);
	PXB_W_U32(&TemBlockCrcBuf[8],getPageProtectBlockCommit(PM8_A_U16(tempBuffer, g_BackUpPage_BlockOffset)));
	
	setPageProtectBlockCrc(PM8_A_U16(tempBuffer, g_BackUpPage_BlockOffset),Get_crc16(TemBlockCrcBuf,12));
	/**********************************将buff中的内容写入页flag*****************************************************/
	g_BackUpPage_Index++;																																													//在写flag页之前将索引指向下一页
	
	if(g_BackUpPage_Index > MAXBACKUPINDEX)																																				//索引到达边界，重新轮询
	{
		g_BackUpPage_Index = 0;
	}
	
	MemNvmOperation_WriteErase(PN8_A_U32(pageflagaddr, U8_M_U16(g_BackUpPage_Index,EEPAGE_SIZE)),tempBuffer,EEPAGE_SIZE);	//将buffer中的数据写入flash，flag页

	g_BackUpPage_BlockOffset = U16_A_U16(g_BackUpPage_BlockOffset, FLASH_PAGEFLAG_BLOCK_OFFSET);									//块偏移加一个块的大小
	g_BackUpPage_FlagIndex = g_BackUpPage_Index;																																	//更新flag页索引
	#else
		ecpMem_Set(tempBuffer,0xFF,EEPAGE_SIZE);
		setPageProtectHeadFlag(tempBuffer);
		setPageProtectHeadCount(tempBuffer,0);
		setPageProtectHeadNum(tempBuffer,1);
		setPageProtectHeadCrc(tempBuffer);
		
		setPageProtectBlockDes(PM8_A_U16(tempBuffer, FLASH_PAGEFLAG_PAGEHEAD_OFFSET),backUpDes);
		setPageProtectBlockIndex(PM8_A_U16(tempBuffer, FLASH_PAGEFLAG_PAGEHEAD_OFFSET),g_BackUpPage_Index);

		PXB_W_U32(TemBlockCrcBuf,backUpDes);
		PXB_W_U32(&TemBlockCrcBuf[4],g_BackUpPage_Index);
		PXB_W_U32(&TemBlockCrcBuf[8],getPageProtectBlockCommit(PM8_A_U16(tempBuffer, FLASH_PAGEFLAG_PAGEHEAD_OFFSET)));		
		
		setPageProtectBlockCrc(PM8_A_U16(tempBuffer, FLASH_PAGEFLAG_PAGEHEAD_OFFSET),Get_crc16(TemBlockCrcBuf,12));
		
		g_FlagCommitOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
		g_BackUpPage_BlockOffset = U16_A_U16(FLASH_PAGEFLAG_PAGEHEAD_OFFSET, FLASH_PAGEFLAG_BLOCK_SIZE);
		
		MemNvmOperation_WriteOnly(PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(g_BackUpPage_Index, EEPAGE_SIZE)),tempBuffer,EEPAGE_SIZE);
	#endif
}



 /***************************************************************************
 * 函数名 ：IsAlreadyBackUp
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：检查目标区是否是否已经备份
 * 备注		：
 *
 ****************************************************************************/
u8 IsAlreadyBackUp(memref backUpDes)
{
	
	u16 Offset;																																				//遍历块
	u8 * FlagPage;																																		//要遍历的flag页
	
	Offset = g_FlagCommitOffset;
	FlagPage = PN8_A_U32(PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE)), U8_M_U16(g_BackUpPage_FlagIndex, EEPAGE_SIZE));
	
	for(;Offset < g_BackUpPage_BlockOffset;)
	{
		if(getPageProtectBlockDes(PN8_A_U16(FlagPage, Offset)) == backUpDes)
		{
			return 1;
		}
		
		Offset = U16_A_U16(Offset, FLASH_PAGEFLAG_BLOCK_OFFSET);
	}
	
	return 0;
}



 /***************************************************************************
 * 函数名 ：FlashBackUp_CommitFlashFlag_Pageimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写实现提交flag
 * 备注		：
 *
 ****************************************************************************/
void FlashBackUp_CommitFlashFlag_Pageimpl(void)
{
	u8 	TemBlockCrcBuf[12];
	
	PX8 tempBuffer;
	#if !LIFE_OR_PERFORMANCE
	PN8	pageflagaddr;
	tempBuffer = gBackUp_TempBuf;
	#endif
	if(g_FlagCommitOffset < g_BackUpPage_BlockOffset)																														//提交偏移小于块偏移时才做提交
	{
		#if !LIFE_OR_PERFORMANCE
		pageflagaddr = PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
		ecpMem_Copy(tempBuffer,PN8_A_U32(pageflagaddr, U8_M_U16(g_BackUpPage_FlagIndex, EEPAGE_SIZE)),EEPAGE_SIZE);		//拷贝最新flag页做提交
		if(getPageProtectHeadCount(tempBuffer) == MAXCOUNT)																												//提交时count需要加一，num不需要改变
		{
			setPageProtectHeadCount(tempBuffer,0);
		}
		else
		{
			setPageProtectHeadCount(tempBuffer,PM8_A_CONST(getPageProtectHeadCount(tempBuffer),1));
		}
		setPageProtectHeadCrc(tempBuffer);
		
		for(;g_FlagCommitOffset < g_BackUpPage_BlockOffset;)																											//轮询block做提交
		{
			setPageProtectBlockCommit(PM8_A_U16(tempBuffer, g_FlagCommitOffset),PAGEPROTECTCOMMIT);
			
			PXB_W_U32(TemBlockCrcBuf,getPageProtectBlockDes(PM8_A_U16(tempBuffer, g_FlagCommitOffset)));
			PXB_W_U32(&TemBlockCrcBuf[4],getPageProtectBlockIndex(PM8_A_U16(tempBuffer, g_FlagCommitOffset)));
			PXB_W_U32(&TemBlockCrcBuf[8],getPageProtectBlockCommit(PM8_A_U16(tempBuffer, g_FlagCommitOffset)));
			
			setPageProtectBlockCrc(PM8_A_U16(tempBuffer, g_FlagCommitOffset),Get_crc16(TemBlockCrcBuf,12));
			
			g_FlagCommitOffset = U16_A_U16(g_FlagCommitOffset, FLASH_PAGEFLAG_BLOCK_OFFSET);
		}
		
		g_BackUpPage_Index++;																													//写flash之前将更新索引									
		if(g_BackUpPage_Index > MAXBACKUPINDEX)
		{
			g_BackUpPage_Index = 0;
		}
		
		MemNvmOperation_WriteErase(PN8_A_U32(pageflagaddr, U8_M_U16(g_BackUpPage_Index, EEPAGE_SIZE)),tempBuffer,EEPAGE_SIZE);

		g_BackUpPage_FlagIndex = g_BackUpPage_Index;																	//更新flag页索引
		#else
			MemNvmOperation_Erase(PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(g_BackUpPage_Index, EEPAGE_SIZE)),EEPAGE_SIZE);
			g_FlagCommitOffset = U16_A_U8(g_FlagCommitOffset, FLASH_PAGEFLAG_BLOCK_SIZE);
			g_BackUpPage_Index++;
			if(g_BackUpPage_Index > U8_S_U8(U8_D_U8(FLASH_PROTECT_PAGE_NUM, 2), 1))
			{
				g_BackUpPage_Index = 0;
			}
		#endif
		g_BackUpPageNum = 0;
	}
}



 /***************************************************************************
 * 函数名 ：FindValidFlagPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：找到合法的falg页
 * 备注		：
 *
 ****************************************************************************/
PN8 FindValidFlagPage(PX8 TempBuffer)
{
	PN8	 pageflagaddr;
	#if !LIFE_OR_PERFORMANCE
	u8   i;
	u8   j;
	u8   FlagNum;																																									//找到的flag页数
	u8   ValidIndex;																																							//有效的flag页索引
	u8   PrivalidIndex;																																						//上一页有效页
	u32  PriValidFlahPageAddr;
	u32  ValidFlagPageAddr;
	Flagarry_ST PageFlag[PAGE_SEQUENCE_LEN];																																		//存储找到的flag页的count和索引                             																			


	if(U8_S_U8(FLASH_PROTECT_PAGE_NUM , MAXBACKUPNUM) > PAGE_SEQUENCE_LEN)
	{
		while(1);
	}

	g_BackUpPage_Index = MAXBACKUPINDEX;
	g_BackUpPage_FlagIndex = NOFLAGPAGE;
	FlagNum = 0;
	pageflagaddr = PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));
	ValidFlagPageAddr = PAGEPROTECTILLEGALADDR;
	PriValidFlahPageAddr = PAGEPROTECTILLEGALADDR;

	//PrivalidIndex = 0xFF;
	
	for(i = 0;i < FLASH_PROTECT_PAGE_NUM ; i++)
	{
		ecpMem_Copy(TempBuffer,pageflagaddr,FLASH_PAGEFLAG_PAGEHEAD_SIZE);																		//拷贝页头到buffer中 
		if(getPageProtectHeadFlag(TempBuffer) == PAGEPROTECTMARK)																									//是flag页标志
		{
			if(getPageProtectHeadCrc(TempBuffer) == Get_crc16(TempBuffer,8))												//页头crc正确，将此页的count和索引记录到数组中
			{
				PageFlag[FlagNum].pageindex = i;
				PageFlag[FlagNum].count = getPageProtectHeadCount(TempBuffer);
				FlagNum++;
			}
			else
			{
				//发生了弱写
				MemNvmOperation_Erase(pageflagaddr,EEPAGE_SIZE);
			}
		}
		pageflagaddr = PN8_A_U32(pageflagaddr, EEPAGE_SIZE);
	}
	
	if(FlagNum == 0)																																							//没有找到flag页
	{
		g_BackUpPage_Index = MAXBACKUPINDEX;
		g_BackUpPage_FlagIndex = 0xFF;
		return NULL;
	}
	
	FindIndex_RollBack( &ValidIndex, &PrivalidIndex, &FlagNum, FlagNum, PageFlag);
	
	pageflagaddr = PN8_A_U16(BACK_UP_AREA_START, U8_M_U16(gFlash_FlagNum, EEPAGE_SIZE));

PriValidFlahPageAddr = (u32)PN8_A_U32(pageflagaddr, PrivalidIndex*EEPAGE_SIZE);
ValidFlagPageAddr = (u32)PN8_A_U32(pageflagaddr, ValidIndex*EEPAGE_SIZE);

ecpMem_Copy(TempBuffer,(PN8)ValidFlagPageAddr,EEPAGE_SIZE);
if(getPageProtectHeadCrc(TempBuffer) == Get_crc16(TempBuffer,8) && CheckPageProtectBlockCrc(TempBuffer))		//最新flag页有效
{
	g_BackUpPage_Index = ValidIndex;
	g_BackUpPage_FlagIndex = ValidIndex;
	return (PN8)ValidFlagPageAddr;
}
/****************************最新flag页无效，使用前一页************************************/
g_BackUpPage_Index = PrivalidIndex;

MemNvmOperation_Erase((PN8)ValidFlagPageAddr,EEPAGE_SIZE);                //最新flag页无效，擦除
if(!(PriValidFlahPageAddr == ValidFlagPageAddr))
{
	g_BackUpPage_FlagIndex = PrivalidIndex;
	ecpMem_Copy(TempBuffer,(PN8)PriValidFlahPageAddr,EEPAGE_SIZE);
	return (PN8)PriValidFlahPageAddr;
}
else
{
	g_BackUpPage_FlagIndex = 0xFF;
	return NULL;
}
#else
u8 i;
u8 j;

for(i = 0;i < U8_D_U8(FLASH_PROTECT_PAGE_NUM, 2);i++)
{
	ecpMem_Copy(TempBuffer,PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(i, EEPAGE_SIZE)),EEPAGE_SIZE);
	if(getPageProtectHeadFlag(TempBuffer) == PAGEPROTECTMARK)
	{
		if(CheckPageProtectBlockCrc(TempBuffer) && getPageProtectHeadCrc(TempBuffer) == Get_crc16(TempBuffer,8))
		{
			for(j = 0;j < 6;j++)
			{
				if(ecpMem_Compare(PN8_A_U32((memref)gFlashBackUpFlagStart, U8_M_U16(i, EEPAGE_SIZE)),TempBuffer,U8_A_U8(FLASH_PAGEFLAG_PAGEHEAD_SIZE, FLASH_PAGEFLAG_BLOCK_SIZE)) != 0)
				{
					MemNvmOperation_Erase(PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(i, EEPAGE_SIZE)),EEPAGE_SIZE);
					return NULL;
				}
			}
			g_BackUpPage_Index = i;
			return PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(i, EEPAGE_SIZE));
		}
		else
		{
			MemNvmOperation_Erase(PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(i, EEPAGE_SIZE)),EEPAGE_SIZE);
			return NULL;
		}
	}
	else
	{
		MemNvmOperation_Erase(PN8_A_U32(gFlashBackUpFlagStart, U8_M_U16(i, EEPAGE_SIZE)),EEPAGE_SIZE);
	}
}
	return NULL;
	

#endif
}



 /***************************************************************************
 * 函数名 ：CheckPageProtectBlockCrc
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：检查flag页合法性
 * 备注		：
 *
 ****************************************************************************/
u8 CheckPageProtectBlockCrc(PX8 PageFlagAddr)
{
	u8 i;
	u8 BlockNum;
	u16 BlockOffset;
	u8 TempBuff[16];
	
	BlockNum = getPageProtectHeadNum(PageFlagAddr);
																						//判断最后一块是否非法
	BlockOffset = U16_A_U32(FLASH_PAGEFLAG_PAGEHEAD_OFFSET, U16_M_U16(U8_S_U8(BlockNum, 1),FLASH_PAGEFLAG_BLOCK_SIZE));
	PXB_W_U32(TempBuff,getPageProtectBlockDes(PM8_A_U16(PageFlagAddr, BlockOffset)));
	PXB_W_U32(&TempBuff[4],getPageProtectBlockIndex(PM8_A_U16(PageFlagAddr, BlockOffset)));
	PXB_W_U32(&TempBuff[8],getPageProtectBlockCommit(PM8_A_U16(PageFlagAddr, BlockOffset)));
	
	if(getPageProtectBlockCrc(PM8_A_U16(PageFlagAddr, BlockOffset)) != Get_crc16(TempBuff,12))
	{
		return 0;
	}
	BlockOffset = FLASH_PAGEFLAG_PAGEHEAD_OFFSET;
	
	for(i = 0;i < BlockNum - 1;i++)
	{
		PXB_W_U32(TempBuff,getPageProtectBlockDes(PM8_A_U16(PageFlagAddr, BlockOffset)));
		PXB_W_U32(&TempBuff[4],getPageProtectBlockIndex(PM8_A_U16(PageFlagAddr, BlockOffset)));
		PXB_W_U32(&TempBuff[8],getPageProtectBlockCommit(PM8_A_U16(PageFlagAddr, BlockOffset)));
		
		if(getPageProtectBlockCrc(PM8_A_U16(PageFlagAddr, BlockOffset)) == Get_crc16(TempBuff,12))
		{
			BlockOffset = U16_A_U8(BlockOffset, FLASH_PAGEFLAG_BLOCK_SIZE);
			continue;
		}
		else
		{
			return 0;
		}
	}
	
	
	return 1;
}

#endif



 /************************************************页擦写代码结束**************************************************************************************
 *************************************************************************************************************************************************/ 




#endif 



