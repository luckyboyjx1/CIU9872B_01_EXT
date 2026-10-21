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

#include "DataOperation.h"
#include "MemBackUpServe.h"
#include "memOperationServe.h"
#include "bootConfigProfile.h"
#include "SysConfigTarget.h"
#include "memNvmOperation.h"
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
#include "SCPInterface.h"

#if !TRANSACTION_OLDDESIGN

/************************************************公共全局变量**************************************************************************************
 *************************************************************************************************************************************************/ 
#pragma MEM_BACK_UP_SERVE_RAM_START
u8      X_A     gBackUp_writeType;
u8      X_A     gBackUp_cacheType;
u8      X_A     BACK_UP_MAX_PAGE;
u8      X_A     FLASH_PROTECT_PAGE_NUM;
PN8     X_A     gBackUp_NotBackUpBottom;
PN8     X_A     gBackUp_NotBackUpTop;
u1 			B_A     gBackUp_inSingleApdu;
u1			B_A    	gBackUp_inMutilApdu;
u8      X_A     gBackUp_PageNumStart;
PN8     X_A     BACK_UP_AREA_START;



/************************************************芯片不同配置变量*********************************************************************************
 *************************************************************************************************************************************************/ 
#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
memref 	 g_Transaction_curpage;
u16 		 g_Transaction_curblockoffset;
u8		 	 g_Transaction_lock_bit;
u8    	 g_Transaction_capacity;
memref 	 g_Transaction_flagaddr;	
#else
u8 			 Transaction_PageFlagIndex;																																			//事务flag区页索引
u8 			 Transaction_BackUpNum;																																						//事务备份数据区大小
u8 			 Transaction_PageFlagNum;																																					//事务备份flag区大小
u16 		 Transaction_PageFlagCount;																																		//事务备份flag页flag地址
memref 	 g_Transaction_curpage;
memref 	 g_Transaction_flagaddr;	
u16 		 g_Transaction_curblockoffset;
u8		 	 g_Transaction_lock_bit;
u8    	 g_Transaction_capacity;

#endif



/************************************************公共接口代码**************************************************************************************
 *************************************************************************************************************************************************/ 
 
 
 
 /***************************************************************************
 * 函数名 ：initMemBackUpGloableVar
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：初始化断电保护全局变量
 * 备注		：
 *
 ****************************************************************************/
 void initMemBackUpGloableVar(void) {
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_001();
	
	BACK_UP_MAX_PAGE		= sysConfig_GetBackUpPageNum();
	FLASH_PROTECT_PAGE_NUM	= sysConfig_GetFlashProtectPageNum();
	#if LIFE_OR_PERFORMANCE
		if(FLASH_PROTECT_PAGE_NUM % 2 != 0)
		{
			FLASH_PROTECT_PAGE_NUM = FLASH_PROTECT_PAGE_NUM + 1;
		}
	#endif
	BACK_UP_AREA_START		= MEMtPN8(U32_S_U16(sysConfig_GetNvmTop(), BACK_UP_MAX_SIZE));//lmc
	 
	 
	#if !CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	#if !LIFE_OR_PERFORMANCE 
	 if(FLASH_PROTECT_PAGE_NUM < 3)
	{
		FLASH_PROTECT_PAGE_NUM = 4;//基于方案页和flag混合使用，该全局变量有最小值要求。
	}
 
	if(U8_R_U8(FLASH_PROTECT_PAGE_NUM,2))
	{
		FLASH_PROTECT_PAGE_NUM += 1;
	}
	#endif
	 
	#endif

	gBackUp_inSingleApdu	= false;
	gBackUp_inMutilApdu		= false;
	gBackUp_cacheType			= 0;
	gBackUp_writeType			= 0;
	gBackUp_PageNumStart	= TF_BACK_UP_NONE;
	gBackUp_NotBackUpBottom        = 0;
	gBackUp_NotBackUpTop        = 0;
	
	FlashBackUp_Init();
	initTransactionMemBackUpGloableVar();
	E2P_stream_flush_clear();
	
	if(BACK_UP_MAX_PAGE > MAX_BACK_UP_PAGE)
	{
			while(1);
	}
}
 


 /***************************************************************************
 * 函数名 ：initTransactionMemBackUpGloableVar
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：初始化事务全局变量
 * 备注		：
 *
 ****************************************************************************/
void initTransactionMemBackUpGloableVar(void)
{
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	g_Transaction_flagaddr	= U32_A_U32(BACK_UP_AREA_START,U16_M_U16(U8_S_U8(BACK_UP_MAX_PAGE,1),EEPAGE_SIZE));	//事务flag页为事务备份区末尾的一页也就是高地址的那一页
	#endif

	#if! CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	//g_Transaction_curpage					= 0;
	#else
	g_Transaction_curpage					= 0;
	#endif
	g_Transaction_curblockoffset 	= 0;
	g_Transaction_lock_bit				= 1;//锁Set_Block流程，只有先get才能set
	g_Transaction_capacity				= 0;
	
	#if !CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	if(sysConfig_GetTransationFlagNum() <= 1)
	{
		while(1);	//小于等于一的时候不符合轮询的逻辑
	}
	Transaction_PageFlagNum = sysConfig_GetTransationFlagNum();
	Transaction_BackUpNum = BACK_UP_MAX_PAGE - Transaction_PageFlagNum;
	
	if(Transaction_PageFlagNum > TRANSACTION_MAXCOUNT)
	{
		while(1);		//调大TRANSACTION_MAXCOUNT，不然回滚无法排序
	}
	
	#endif
}



 /***************************************************************************
 * 函数名 ：clearBackUpPageFlag
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：提交断电Flag标志
 * 备注		：
 *
 ****************************************************************************/
 void clearBackUpPageFlag(void) {
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_002();
	
			FlashBackUp_CommitFlashFlag();
			Transaction_CommitFlag(gBackUp_TempBuf);
}
 


 /***************************************************************************
 * 函数名 ：Transaction_CommitFlag
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：提交事务断电Flag标志
 * 备注		：
 *
 ****************************************************************************/
void Transaction_CommitFlag(PN8 tempBuffer)
{
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
		Transaction_CommitFlag_Wordimpl(tempBuffer);
	#else
		Transaction_CommitFlag_Pageimpl(tempBuffer);
	#endif
	g_Transaction_capacity				= 0;
	g_Transaction_curblockoffset 	= 0;
	g_Transaction_lock_bit 				= 1;
}



 /***************************************************************************
 * 函数名 ：Transaction_FindNoatomicData
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：查询非原子数据并将其写入tempbuffer用于更新备份区
 * 备注		：
 *
 ****************************************************************************/
void Transaction_FindNoatomicData(memref dest_addr,memref bakup_addr,PN8 gBackUp_TempBuf)
{
	u16 tempoffset;
	u16 start_offset;
	u16 templen;
	u32 tempaddr;
	
	tempoffset = U32_AND_U32(dest_addr,EEPAGE_MASK);
	tempaddr   = U32_AND_U32(dest_addr,NOR_EEPAGE_MASK);
	
	ecpMem_Copy(gBackUp_TempBuf,MEMtPX8(bakup_addr),EEPAGE_SIZE);
	
//	if(tempoffset == EEPAGE_MASK)
//	{
//		return;
//	}
	
	for(;tempoffset<EEPAGE_SIZE;tempoffset++)
	{
		if(PN_R_U8(PM8_A_CONST(tempaddr,tempoffset)) != PN_R_U8(PM8_A_CONST(stream_buffer,tempoffset)))
		{
			start_offset = tempoffset;
			if(tempoffset == EEPAGE_MASK)
			{
				ecpMem_Copy(PM8_A_CONST(gBackUp_TempBuf,start_offset),PM8_A_CONST(stream_buffer,start_offset),1);
				break;
			}
			tempoffset++;
			for(;tempoffset<EEPAGE_SIZE;tempoffset++)
			{
				if( (PN_R_U8(PM8_A_CONST(tempaddr,tempoffset)) == PN_R_U8(PM8_A_CONST(stream_buffer,tempoffset)) )|| (tempoffset == EEPAGE_MASK))
				{
					if(tempoffset == EEPAGE_MASK)
					{
						templen	 = tempoffset - start_offset + 1;
					}
					else
					{
						templen	 = tempoffset - start_offset;					
					}
					ecpMem_Copy(PM8_A_CONST(gBackUp_TempBuf,start_offset),PM8_A_CONST(stream_buffer,start_offset),templen);
					break;
				}
			}
			 
		}
	}
	
}



 /***************************************************************************
 * 函数名 ：Set_Block
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：写块和头的公共API
 * 备注		：会更新能力、当前页以及逻辑锁等值，用于下次get的异常判断
 *
 ****************************************************************************/
memref Set_Block(memref e2p_addr,PN8 TempBuff)
{

	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
		Set_Block_Wordimpl(e2p_addr);
	#else
		Set_Block_Pageimpl(e2p_addr,TempBuff);
	#endif
	
	g_Transaction_curblockoffset += TRANSACTION_BAKUP_BLOCK_SIZE;
	
	g_Transaction_capacity++;
	g_Transaction_curpage++;
	g_Transaction_lock_bit = 1;
	
	return 0;
}



 /***************************************************************************
 * 函数名 ：Get_NextPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：获取下一页地址
 * 备注		：会对能力、页底、锁等进行判断
 *
 ****************************************************************************/
memref Get_NextPage(void)
{
	memref tempaddr;
	
	if(!g_Transaction_lock_bit)
	{
		while(1);//流程错误
	}

	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	if(g_Transaction_capacity > U8_S_U8(BACK_UP_MAX_PAGE,2))//大于能力值
	{
		bootErrorHandle_ThrowError(TRANSACTION_BUFFER_OVERFLOW);//能力错误
	}
	if(g_Transaction_curpage > U8_S_U8(BACK_UP_MAX_PAGE,2))//大于底端
	{
		g_Transaction_curpage = 0;
	}
	#else
	if(g_Transaction_capacity > U8_S_U8(Transaction_BackUpNum,1))//大于能力值
	{
		bootErrorHandle_ThrowError(TRANSACTION_BUFFER_OVERFLOW);//能力错误
	}
	if(g_Transaction_curpage > U8_S_U8(Transaction_BackUpNum,1))//大于底端
	{
		g_Transaction_curpage = 0;
	}
	#endif
	tempaddr = PN8_A_U32(PNtMEM(BACK_UP_AREA_START), U8_M_U16(g_Transaction_curpage, EEPAGE_SIZE));
	g_Transaction_lock_bit = 0;
	
	return tempaddr;
}



 /***************************************************************************
 * 函数名 ：isAlreadybak
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：事务中检查是否已经备份
 * 备注		：
 *
 ****************************************************************************/
u32 isAlreadybak(memref e2p_addr)
{
	u32	   tempindex;
	memref tempaddr;
	
	if(g_Transaction_curblockoffset == 0)
	{	
		return ILLEGAL_FLAG;
	}
	
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE 
	tempaddr = Findflag_word(e2p_addr);
	#else
	tempaddr = Findflag_page(e2p_addr);
	#endif
	
	if(tempaddr != 0)
	{
		tempindex = getTransactionBlockIndex(tempaddr);
	}
	else
	{
		tempindex = ILLEGAL_FLAG;
	}
	return tempindex;
}



 /***************************************************************************
 * 函数名 ：Transaction_bakup_Noatomic
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：事务非原子备份
 * 备注		：
 *
 ****************************************************************************/
void Transaction_bakup_Noatomic(memref e2p_addr, u16 addrindex)
{
	
	u32 tempaddr;
	
	tempaddr = addrindex * EEPAGE_SIZE + PNtMEM(BACK_UP_AREA_START); //找到需要修改的地址
	
	FlashBackUp_PageProtect(tempaddr,gBackUp_TempBuf);//页保护该需要修改的数据
	
	Transaction_FindNoatomicData(e2p_addr,tempaddr,gBackUp_TempBuf);//找到非原子写的数据
	
	MemNvmOperation_WriteErase(MEMtPN8(tempaddr),gBackUp_TempBuf,EEPAGE_SIZE);//改写备份区
	
	FlashBackUp_CommitFlashFlag();
	
	//是否需要提交页保护数据？
	
}



 /***************************************************************************
 * 函数名 ：Transaction_bakup_Noatomic
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：事务原子备份
 * 备注		：
 *
 ****************************************************************************/
void Transaction_bakup_Normal(memref e2p_addr,PN8 TempBuff)
{
	memref tempaddr;
		
	tempaddr = Get_NextPage(); //考虑事务能力,获取下一个块
	
	MemNvmOperation_WriteErase(MEMtPN8(tempaddr),MEMtPN8(U32_AND_U32(e2p_addr,NOR_EEPAGE_MASK)),EEPAGE_SIZE);//写备份数据
	
	Set_Block(e2p_addr,TempBuff);//设置下一个块 //还只是字写得部分
	
}



 /***************************************************************************
 * 函数名 ：save_E2P
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：断电保护备份逻辑的主入口
 * 备注		：
  * modifed by xxj@20210328:  对于load过程中组建的写可以不进行保护
 ****************************************************************************/
void save_E2P(memref e2p_addr, u16 length) 
{ 
	u32 tempindex;
    memref startAddr;
    memref endAddr;
	
	PATCH_BOOT_MEMORY_MEMBACKUPSERVE_009();
	
	/* do not back up any data when creating cardmanager.if tearing in the cardmanager creating, clear all eeprom*/
	if (BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)  
	{
		return;
	}

	startAddr = (G_PkgComponentAddrStartAddr == 0x00)?0x00:((G_PkgComponentAddrStartAddr + EEPAGE_MASK) & NOR_EEPAGE_MASK);
	endAddr = (G_PkgComponentAddrEndAddr == 0x00)?0x00:(G_PkgComponentAddrEndAddr & NOR_EEPAGE_MASK); // G_PkgComponentAddrEndAddr会小于0x1FF吗？

	if((e2p_addr >= startAddr) && (e2p_addr < endAddr))
	{
		return;
	}
	
	startAddr = ((SCP_DATA_START + EEPAGE_MASK) & NOR_EEPAGE_MASK);
	endAddr =  ((SCP_DATA_START + SCP_DATA_LENGTH) & NOR_EEPAGE_MASK);

	if((e2p_addr >= startAddr) && (e2p_addr < endAddr)) // 这个条件永远不会满足
	{
		return;
	}

	if(WRITEEP_LOST == getCacheDataType()) 
	{
		if( checkIsBackUp(e2p_addr, length) )
		{
			return;
		}
	}
	
	if(isInProgress() || (WRITEEP_TRANSACTION == getCacheDataType()) ) 
	{
		
		if(checkIsBackUp(e2p_addr, length) || (WRITEEP_LOST == getCacheDataType())) 
		{
			FlashBackUp_PageProtect(e2p_addr, gBackUp_TempBuf);
			return ;
		}
		
		tempindex = isAlreadybak(e2p_addr);//记录块的index好还是地址的index好？
		
		if(WRITEEP_NOATOMIC == getCacheDataType())
		{
			if(tempindex == ILLEGAL_FLAG)
			{
				FlashBackUp_PageProtect(e2p_addr, gBackUp_TempBuf);
			}
			else
			{
				Transaction_bakup_Noatomic(e2p_addr,tempindex);
			}

		}
		else
		{
			if(tempindex == ILLEGAL_FLAG)
			{
				Transaction_bakup_Normal(e2p_addr, gBackUp_TempBuf);
			}
		}
		
		return;
	}
	
	FlashBackUp_PageProtect(e2p_addr, gBackUp_TempBuf);
}



 /***************************************************************************
 * 函数名 ：commitSaveE2p
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：提交事务
 * 备注		：
 *
 ****************************************************************************/
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



 /***************************************************************************
 * 函数名 ：memSetNoBackAreaPointer
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：设置不进行备份的区域
 * 备注		：
 *
 ****************************************************************************/
void memSetNoBackAreaPointer(PF8 maxEntryAddr, PF8 usedHeapAddr) {
	
	#if !WITH_BREAK_TEST_CODE
	gBackUp_NotBackUpBottom 	= MEMtPN8(MBUS_getNotBackUpBottom(maxEntryAddr));
	gBackUp_NotBackUpTop 		= MEMtPN8(MBUS_getNotBackUpTop(usedHeapAddr));
	#endif
}



 /***************************************************************************
 * 函数名 ：Transaction_RollBack
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：回滚函数
 * 备注		：
 *
 ****************************************************************************/
void Transaction_RollBack(PN8 tempbuufer,PN8 reusebuffer,u8 isReset)
{
	PN8 		tempaddr;
	memref 		destaddr;
	memref  	tempindex;
	u16  		limit;
    #if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	u8          i;
	u32         commitflag;
    #endif
	
	#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	if(MEMtPN8(g_Transaction_flagaddr) == NULL)
	{
		return;
	}
	ecpMem_Copy(tempbuufer,g_Transaction_flagaddr,EEPAGE_SIZE);
	#else
	g_Transaction_flagaddr = Transaction_FindValidFlagPage(tempbuufer);
	if(MEMtPN8(g_Transaction_flagaddr) == NULL)
	{
		return;
	}
	g_Transaction_curpage = getTransactionBlockIndex(tempbuufer + TRANSACTION_BAKUP_HEAD_SIZE + TRANSACTION_BAKUP_BLOCK_SIZE * (getTransactionHEAD_NUM(tempbuufer) - 1)) + 1;
	if(g_Transaction_curpage > U8_S_U8(Transaction_BackUpNum,1))
	{
		g_Transaction_curpage = 0;
	}
	#endif

#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	if(isReset == ABORT_ON_RESET)
	{
		commitflag = PM_R_U32(tempbuufer);
		for(i = 0; i < 10; i++)
		{
			if(commitflag != PN_R_U32(g_Transaction_flagaddr))
			{
				return;
			}
		}
	}
#else
	if(isReset == ABORT_ON_RESET)
	{
		FlashBackUp_PageProtect(g_Transaction_flagaddr,reusebuffer);
		MemNvmOperation_WriteErase(MEMtPN8(g_Transaction_flagaddr),tempbuufer,EEPAGE_SIZE);	//实写
		FlashBackUp_CommitFlashFlag();
	}
#endif
	
#if	WITH_BREAK_TEST_CODE
	if(isReset == ABORT_ON_RESET)
	{
		FlashBackUp_PageProtect(g_Transaction_flagaddr,reusebuffer);
		FlashBackUp_CommitFlashFlag();
	}
#endif
	if(getTransactionHEAD_MARK(tempbuufer)== TRANSACTION_BAKUP_HEAD_MARK_VALUE)
	{
		#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
		if(getTransactionHEAD_SERIALNUMBER(tempbuufer) == TRANSACTION_BAKUP_SERIALNUMBER_VALUE)		
		#else
		if(getTransactionHEAD_NUM(tempbuufer) <= Transaction_BackUpNum)
		#endif
		{
			ecpMem_Copy(reusebuffer,tempbuufer,TRANSACTION_BAKUP_HEAD_SIZE);
			setTransactionHEAD_CRC(reusebuffer);
			
			if(getTransactionHEAD_CRC(tempbuufer) == getTransactionHEAD_CRC(reusebuffer))
			{
				getTransactionHEAD_COMMIT(reusebuffer,tempbuufer);
				ecpMem_Set(PN8_A_U16(reusebuffer,TRANSACTION_BAKUP_HEAD_COMMIT_SIZE),TRANSACTION_BAKUP_HEAD_COMMIT_VALUE,TRANSACTION_BAKUP_HEAD_COMMIT_SIZE);
				
				if(ecpMem_Compare(reusebuffer,PN8_A_U16(reusebuffer,TRANSACTION_BAKUP_HEAD_COMMIT_SIZE),TRANSACTION_BAKUP_HEAD_COMMIT_SIZE) != 0)
				{					
//					if(isReset == ABORT_ON_RESET)
//					{
//						FlashBackUp_PageProtect(g_Transaction_flagaddr,reusebuffer);
//						MemNvmOperation_WriteErase(MEMtPN8(g_Transaction_flagaddr),tempbuufer,EEPAGE_SIZE);	//实写
//					}
					
					tempaddr = PN8_A_U16(MEMtPN8(tempbuufer),TRANSACTION_BAKUP_HEAD_SIZE);
					limit		 = U16_S_U16(EEPAGE_SIZE,TRANSACTION_BAKUP_HEAD_SIZE);
					
					for(g_Transaction_curblockoffset = 0;g_Transaction_curblockoffset < limit;)
					{
						
						ecpMem_Copy(reusebuffer,PN8_A_U16(tempaddr,g_Transaction_curblockoffset),U16_S_U16(TRANSACTION_BAKUP_BLOCK_SIZE,TRANSACTION_BAKUP_CRC_SIZE));
						setTransactionBlockCRC(reusebuffer);
						
						if(getTransactionBlockCRC(PN8_A_U16(tempaddr,g_Transaction_curblockoffset)) != getTransactionBlockCRC(reusebuffer))
						{
							break;
						}
						else
						{
							destaddr 	= getTransactionBlockAddr(PN8_A_U16(tempaddr,g_Transaction_curblockoffset));
							tempindex = getTransactionBlockIndex(PN8_A_U16(tempaddr,g_Transaction_curblockoffset));
							tempindex	= U32_A_U32(PNtMEM(BACK_UP_AREA_START),U16_M_U16(tempindex,EEPAGE_SIZE));
							MemNvmOperation_WriteErase(MEMtPN8(U32_AND_U32(destaddr,NOR_EEPAGE_MASK)),MEMtPN8(tempindex),EEPAGE_SIZE);
						}
						g_Transaction_curblockoffset = U16_A_U16(g_Transaction_curblockoffset,TRANSACTION_BAKUP_BLOCK_SIZE);
					}
					if(isReset == ABORT_NOT_ON_RESET)
					{
						NvmManager_Reset();
						memCommitTransaction();
					}
					else
					{
//						FlashBackUp_CommitFlashFlag();
						#if !CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
						g_Transaction_curblockoffset = 0xFFFF;       //页擦写事务回滚后做提交时需要满足提交条件
						#endif
						Transaction_CommitFlag(tempbuufer);	
					}
					
				}
				
			}
		}
	}
	
	#if !CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	g_Transaction_curblockoffset = 0;
	#endif
	
	NvmManager_Reset();
	
}



 /************************************************字写代码实现*************************************************************************************
 *************************************************************************************************************************************************/



#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
 /***************************************************************************
 * 函数名 ：Transaction_CommitFlag_Wordimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：字写提交flag实现
 * 备注		：
 *
 ****************************************************************************/
 void Transaction_CommitFlag_Wordimpl(PN8 tempBuffer)
{
	SC_deactiveEDC();
	setTransactionHEAD_COMMIT(tempBuffer);
	if(g_Transaction_curblockoffset != 0)
	{
		MemNvmOperation_WriteOnly(PX8_A_U16(MEMtPN8(g_Transaction_flagaddr),TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET),PX8_A_U16(tempBuffer, TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET),TRANSACTION_BAKUP_HEAD_COMMIT_SIZE);
	}
	SC_activeEDC();
}



 /***************************************************************************
 * 函数名 ：Set_Block_Wordimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：字写设置块实现
 * 备注		：
 *
 ****************************************************************************/
memref Set_Block_Wordimpl(memref e2p_addr)
{
	u8 	tempbuffer[TRANSACTION_BAKUP_HEAD_SIZE + TRANSACTION_BAKUP_BLOCK_SIZE];
	u16 tempoffset;
	
	
	tempoffset = 0x00;
	ecpMem_Set(tempbuffer, 0xFF, sizeof(tempbuffer));
	
	if(g_Transaction_lock_bit)
	{
		while(1);//错误流程
	}
	if( U16_A_U16(g_Transaction_curblockoffset,TRANSACTION_BAKUP_BLOCK_SIZE) > EEPAGE_SIZE)
	{
		bootErrorHandle_ThrowError(TRANSACTION_BUFFER_OVERFLOW);
	}
	if(g_Transaction_curblockoffset == 0x00)
	{
		MemNvmOperation_Erase(MEMtPN8(g_Transaction_flagaddr),EEPAGE_SIZE);
		MemNvmOperation_WriteOnly(PN8_A_U16(MEMtPN8(g_Transaction_flagaddr),U16_S_U8(EEPAGE_SIZE, 4)),tempbuffer,0x04);	//forM25
		setTransactionHEAD_MARK(tempbuffer,TRANSACTION_BAKUP_HEAD_MARK_VALUE);
		setTransactionHEAD_SERIALNUMBER(tempbuffer,TRANSACTION_BAKUP_SERIALNUMBER_VALUE);
		setTransactionHEAD_PAD(tempbuffer);
		setTransactionHEAD_NOTFIN(tempbuffer,g_Transaction_flagaddr);
		setTransactionHEAD_CRC(tempbuffer);
		g_Transaction_curblockoffset 		= TRANSACTION_BAKUP_HEAD_SIZE;
		tempoffset							= TRANSACTION_BAKUP_HEAD_SIZE;
	}
	setTransactionBlockAddr(PM8_A_CONST(tempbuffer,tempoffset),e2p_addr);
	setTransactionBlockIndex(PM8_A_CONST(tempbuffer,tempoffset),g_Transaction_curpage);
	setTransactionBlockPAD(PM8_A_CONST(tempbuffer,tempoffset));
	setTransactionBlockCRC(PM8_A_CONST(tempbuffer,tempoffset));
	
	if(g_Transaction_curblockoffset == TRANSACTION_BAKUP_HEAD_SIZE)
	{
		MemNvmOperation_WriteOnly(PN8_A_U16(MEMtPN8(g_Transaction_flagaddr),TRANSACTION_BAKUP_HEAD_MARK_OFFSET),PN8_A_U16(tempbuffer,TRANSACTION_BAKUP_HEAD_MARK_OFFSET),U16_S_U16(U16_A_U16(TRANSACTION_BAKUP_HEAD_SIZE,TRANSACTION_BAKUP_BLOCK_SIZE),TRANSACTION_BAKUP_HEAD_COMMIT_SIZE));
	}
	else
	{
		MemNvmOperation_WriteOnly(PN8_A_U16(MEMtPN8(g_Transaction_flagaddr),g_Transaction_curblockoffset),tempbuffer,TRANSACTION_BAKUP_BLOCK_SIZE);
	}	
	
	return 0;
}



 /***************************************************************************
 * 函数名 ：Findflag_word
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：字写寻找flag实现
 * 备注		：
 *
 ****************************************************************************/
memref Findflag_word(memref e2p_addr)
{
	PN8 	limit;
	PN8 	blockaddr;
	memref  tempaddr;
//	limit			 = U16_D_U8(U16_S_U16(EEPAGE_SIZE,TRANSACTION_BAKUP_HEAD_SIZE),TRANSACTION_BAKUP_BLOCK_SIZE); 
//	blockaddr		 = PN8_A_U8(MEMtPN8(g_Transaction_flagaddr),TRANSACTION_BAKUP_HEAD_SIZE);

	limit		= PN8_A_U32(MEMtPN8(g_Transaction_flagaddr),EEPAGE_SIZE);
	blockaddr	= PN8_A_U32(PN8_A_U32(MEMtPN8(g_Transaction_flagaddr),TRANSACTION_BAKUP_HEAD_SIZE),TRANSACTION_BAKUP_ADDR_OFFSET);
	tempaddr	= U32_AND_U32(e2p_addr,NOR_EEPAGE_MASK);
	
	while(blockaddr < limit)    //块跨页时退出循环
	{
		
		if(U32_AND_U32(memReadU32(blockaddr),NOR_EEPAGE_MASK) == tempaddr)
		{
			return PNtMEM(blockaddr);
		}
		if(U32_AND_U32(memReadU32(blockaddr),NOR_EEPAGE_MASK) == ERASE_VALUE)
		{
			return 0;
		}
		
		blockaddr = PN8_A_U8(blockaddr,TRANSACTION_BAKUP_BLOCK_SIZE);
	}
	
	return 0;
}



#endif
 /************************************************页写代码实现************************************************************************************
 *************************************************************************************************************************************************/



#if !CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
 /***************************************************************************
 * 函数名 	：Transaction_CommitFlag_Pageimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写提交flag实现
 * 备注		：
 *
 ****************************************************************************/
void Transaction_CommitFlag_Pageimpl(PN8 tempBuffer)
{
	PN8 PreFlagPageAddr;
	SC_deactiveEDC();
	if(g_Transaction_curblockoffset == 0x00)
	{
		return ;
	}
	PreFlagPageAddr = PN8_A_U32(BACK_UP_AREA_START,U8_M_U32(U8_A_U8(Transaction_BackUpNum, Transaction_PageFlagIndex),EEPAGE_SIZE));
	ecpMem_Copy(tempBuffer,PreFlagPageAddr,EEPAGE_SIZE);
	setTransactionHEAD_COMMIT(tempBuffer);
	if(getTransactionHEAD_SERIALNUMBER(tempBuffer) == TRANSACTION_MAXCOUNT)
	{
		setTransactionHEAD_SERIALNUMBER(tempBuffer,0);
	}
	else
	{
		setTransactionHEAD_SERIALNUMBER(tempBuffer,getTransactionHEAD_SERIALNUMBER(tempBuffer) + 1);
	}
	setTransactionHEAD_CRC(tempBuffer);
	
	Transaction_PageFlagIndex++;
	if(Transaction_PageFlagIndex > U8_S_U8(Transaction_PageFlagNum, 1))
	{
		Transaction_PageFlagIndex = 0;
	}
	MemNvmOperation_WriteErase(PN8_A_U32(BACK_UP_AREA_START,U8_M_U32(U8_A_U8(Transaction_BackUpNum, Transaction_PageFlagIndex),EEPAGE_SIZE)),tempBuffer,EEPAGE_SIZE);
	SC_activeEDC();
}



 /***************************************************************************
 * 函数名 	：Set_Block_Pageimpl
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写设置块实现
 * 备注		：
 *
 ****************************************************************************/
memref Set_Block_Pageimpl(memref e2p_addr,PN8 TempBuff)
{
	PN8 FlagPageAddr;
	PN8 PreFlagPageAddr;
	
	if(Transaction_PageFlagIndex == 0xFF)
	{
		FlagPageAddr = PN8_A_U32(BACK_UP_AREA_START,U8_M_U32(U8_A_U8(Transaction_BackUpNum, 0),EEPAGE_SIZE));
	}
	else
	{
		FlagPageAddr = PN8_A_U32(BACK_UP_AREA_START,U8_M_U32(U8_A_U8(Transaction_BackUpNum, Transaction_PageFlagIndex),EEPAGE_SIZE));
	}
	PreFlagPageAddr = FlagPageAddr;							//上一页
	FlagPageAddr = PN8_A_U16(FlagPageAddr, EEPAGE_SIZE);
	
	
	if(FlagPageAddr == PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(BACK_UP_MAX_PAGE, EEPAGE_SIZE)))  //当事务FLAG页到达边界后 循环 也可以改成该判断（Transaction_PageFlagNum == Transaction_PageFlagIndex + 1）
	{
		FlagPageAddr = PN8_A_U32(BACK_UP_AREA_START,U8_M_U32(Transaction_BackUpNum,EEPAGE_SIZE));
	}
	
	if(g_Transaction_curblockoffset == 0x00)
	{
		ecpMem_Set(TempBuff,0xFF,EEPAGE_SIZE);
		setTransactionHEAD_MARK(TempBuff,TRANSACTION_BAKUP_HEAD_MARK_VALUE);
		if(Transaction_PageFlagIndex == 0xFF)
		{
			setTransactionHEAD_SERIALNUMBER(TempBuff,TRANSACTION_BAKUP_SERIALNUMBER_VALUE);
		}
		else
		{
			if(getTransactionHEAD_SERIALNUMBER(PreFlagPageAddr) == TRANSACTION_MAXCOUNT)
			{
				setTransactionHEAD_SERIALNUMBER(TempBuff,TRANSACTION_BAKUP_SERIALNUMBER_VALUE);
			}
			else
			{
				setTransactionHEAD_SERIALNUMBER(TempBuff,getTransactionHEAD_SERIALNUMBER(PreFlagPageAddr) + 1);
			}
		}
		setTransactionHEAD_NUM(TempBuff,1);
		setTransactionHEAD_CRC(TempBuff);
		g_Transaction_curblockoffset 		= TRANSACTION_BAKUP_HEAD_SIZE;
	}
	else
	{
		ecpMem_Copy(TempBuff,PreFlagPageAddr,EEPAGE_SIZE);
		if(getTransactionHEAD_SERIALNUMBER(TempBuff) == TRANSACTION_MAXCOUNT)
		{
			setTransactionHEAD_SERIALNUMBER(TempBuff,0);
		}
		else
		{
			setTransactionHEAD_SERIALNUMBER(TempBuff,U16_A_U8(getTransactionHEAD_SERIALNUMBER(TempBuff), 1));
		}
		setTransactionHEAD_NUM(TempBuff,U16_A_U8(getTransactionHEAD_NUM(TempBuff), 1));
		setTransactionHEAD_CRC(TempBuff);
	}
	
	setTransactionBlockAddr(PN8_A_U16(TempBuff, g_Transaction_curblockoffset),e2p_addr);
	setTransactionBlockIndex(PN8_A_U16(TempBuff, g_Transaction_curblockoffset),g_Transaction_curpage);
	setTransactionBlockCRC(PN8_A_U16(TempBuff, g_Transaction_curblockoffset));
	
	if(Transaction_PageFlagIndex == 0xFF)
	{
		Transaction_PageFlagIndex = 0;
		MemNvmOperation_WriteErase(PreFlagPageAddr,TempBuff,EEPAGE_SIZE);
	}
	else
	{
		Transaction_PageFlagIndex++;
		if(Transaction_PageFlagIndex > U8_S_U8(Transaction_PageFlagNum, 1))
		{
			Transaction_PageFlagIndex = 0;
		}
		
		MemNvmOperation_WriteErase(FlagPageAddr,TempBuff,EEPAGE_SIZE);

	}
	
	return 0;
}



 /***************************************************************************
 * 函数名 	：Findflag_page
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写查找flag实现
 * 备注		：
 *
 ****************************************************************************/
memref Findflag_page(memref e2p_addr)
{
	u16 PageFlagBlockOffset;
	PN8 PageFlagAddr;
	u32 TempAddr;
	
	TempAddr = U32_AND_U32(e2p_addr,NOR_EEPAGE_MASK);
	PageFlagAddr = PN8_A_U32(BACK_UP_AREA_START, U8_M_U32(U8_A_U8(Transaction_BackUpNum,Transaction_PageFlagIndex), EEPAGE_SIZE));
	PageFlagBlockOffset = TRANSACTION_BAKUP_HEAD_SIZE;
	
	for(;PageFlagBlockOffset < g_Transaction_curblockoffset;)
	{
		if(U32_AND_U32(getTransactionBlockAddr(PN8_A_U16(PageFlagAddr, PageFlagBlockOffset)),NOR_EEPAGE_MASK) == TempAddr)
		{
			return (u32)PN8_A_U16(PageFlagAddr, PageFlagBlockOffset);
		}
		PageFlagBlockOffset = U8_A_U8(PageFlagBlockOffset, TRANSACTION_BAKUP_BLOCK_SIZE);
	}
	
	return 0;
}



 /***************************************************************************
 * 函数名 	：Transaction_FindValidFlagPage
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦查找有效flag页实现
 * 备注		：
 *
 ****************************************************************************/
memref Transaction_FindValidFlagPage(PN8 tempbuffer)
{
	PN8 PageFlagAddr;
	u32 ValidFlagPageAddr;
	u32	PreValidAddr;
	u8  i,j;
	u8  validnums;
	u8 	ValidFlagindex;
	u8 	PrevalidFlagindex;
	Flagarry_ST temparry[TRANSACTION_SEQUENCE_LEN];
	
	
	if( Transaction_PageFlagNum > TRANSACTION_SEQUENCE_LEN)
	{
		while(1);//temparry越界，开大temparry即可
	}
	
	validnums	= 0;
	Transaction_PageFlagIndex = 0xFF;
	Transaction_PageFlagCount = 0;
	g_Transaction_curpage			= 0;
	PageFlagAddr = PN8_A_U32(BACK_UP_AREA_START, U8_M_U16(Transaction_BackUpNum, EEPAGE_SIZE));
	
	PreValidAddr = TRANSACTION_BACKUP_ILLEGALADDR;
	ValidFlagPageAddr = TRANSACTION_BACKUP_ILLEGALADDR;
	
	for(i = 0;i < Transaction_PageFlagNum;i++)
	{
		ecpMem_Copy(tempbuffer,PageFlagAddr,TRANSACTION_BAKUP_HEAD_SIZE);
		if(getTransactionHEAD_MARK(tempbuffer) == TRANSACTION_BAKUP_HEAD_MARK_VALUE)																											//is pageflag
		{
			if(getTransactionHEAD_CRC(tempbuffer) == Get_crc16(tempbuffer,U8_S_U8(TRANSACTION_BAKUP_HEAD_SIZE,TRANSACTION_BAKUP_HEAD_CRC_SIZE)))			//check head crc
			{
				temparry[i].count = getTransactionHEAD_SERIALNUMBER(tempbuffer);
				validnums ++;
			}
			else																																																																				//head crc wrong
			{
				MemNvmOperation_Erase((PN8)PageFlagAddr,EEPAGE_SIZE);
				temparry[i].count = ILLEGAL_FLAG;
			}
			temparry[i].pageindex	= i;
		}
		else
		{
			temparry[i].count = ILLEGAL_FLAG;
			temparry[i].pageindex	= i;
		}
		PageFlagAddr = PN8_A_U32(PageFlagAddr,EEPAGE_SIZE);
	}
	
	if(validnums == 0)
	{
		return 0;
	}
	
	FindIndex_RollBack(&ValidFlagindex, &PrevalidFlagindex, &validnums, Transaction_PageFlagNum ,temparry);//查找有效的index 传实参
	
	ValidFlagPageAddr		  = U32_A_U32(BACK_UP_AREA_START, U8_M_U16(U8_A_U32(Transaction_BackUpNum,ValidFlagindex), EEPAGE_SIZE));
	PreValidAddr     		  = U32_A_U32(BACK_UP_AREA_START, U8_M_U16(U8_A_U32(Transaction_BackUpNum,PrevalidFlagindex), EEPAGE_SIZE));
	
	
	ecpMem_Copy(tempbuffer,(u8*)ValidFlagPageAddr,EEPAGE_SIZE);
	if(getTransactionHEAD_CRC(tempbuffer) == Get_crc16(tempbuffer,U8_S_U8(TRANSACTION_BAKUP_HEAD_SIZE,TRANSACTION_BAKUP_HEAD_CRC_SIZE)) && TransactionChekBlockCrc(tempbuffer))
	{
		Transaction_PageFlagIndex = ValidFlagindex;	
		return ValidFlagPageAddr;
	}
	
	MemNvmOperation_Erase((PN8)ValidFlagPageAddr,EEPAGE_SIZE);
	ecpMem_Copy(tempbuffer,(PN8)PreValidAddr,EEPAGE_SIZE);
	
	if(PreValidAddr == ValidFlagPageAddr)
	{
		Transaction_PageFlagIndex = 0xFF;
		return 0;
	}
	else
	{
		Transaction_PageFlagIndex = PrevalidFlagindex;
		return PreValidAddr;
	}

}



 /***************************************************************************
 * 函数名 	：TransactionChekBlockCrc
 * 时间		：2021/2/2
 * 版本		：v1.0.0
 * 功能		：页擦写检查所有块合法性实现
 * 备注		：
 *
 ****************************************************************************/
u8 TransactionChekBlockCrc(PN8 tempbuffer)
{
	u8 i;
	u16 BlockOffset;
	u32 BlockNum;
	
	BlockNum = getTransactionHEAD_NUM(tempbuffer);
	
	BlockOffset = U16_A_U16(TRANSACTION_BAKUP_HEAD_SIZE, U8_M_U8(U8_S_U8(BlockNum, 1),TRANSACTION_BAKUP_BLOCK_SIZE));											//判断最后一块是否合法
	if(getTransactionBlockCRC(PN8_A_U16(tempbuffer,BlockOffset)) != Get_crc16(PN8_A_U16(tempbuffer,BlockOffset),8))
	{
		return 0;
	}

	BlockOffset = TRANSACTION_BAKUP_HEAD_SIZE;																													//遍历除了最后一块的每一块判断是否合法
	for(i = 0;i < BlockNum - 1;i++)
	{
		if(getTransactionBlockCRC(PN8_A_U16(tempbuffer,BlockOffset)) != Get_crc16(PN8_A_U16(tempbuffer,BlockOffset),8))
		{
			return 0;
		}
		BlockOffset = U8_A_U8(BlockOffset, TRANSACTION_BAKUP_BLOCK_SIZE);
	}
	
	return 1;
}



 /***************************************************************************
 * 函数名 	：FindIndex_RollBack
 * 时间		：2021/3/11
 * 版本		：v1.0.0
 * 功能		：回滚时从备份区找到最新的FLAG页index
 * 备注		：事务和页保护共用
 *
 ****************************************************************************/
void FindIndex_RollBack(PN8 ValidFlagindex,PN8 PrevalidFlagindex,PN8 validnums, u8 PageFlagNum ,Flagarry_ST temparry[])
{
	
	int i,j;
	
	for( i = 0 ; i < PageFlagNum ; i++ )			//排序
	{
		for( j = 0 ; j < PageFlagNum - 1 ; j++ )
		{
			if(temparry[j].count > temparry[j+1].count)
			{
				temparry[j].count 	= temparry[j].count + temparry[j+1].count;
				temparry[j+1].count = temparry[j].count - temparry[j+1].count;
				temparry[j].count 	= temparry[j].count - temparry[j+1].count;
				
				temparry[j].pageindex 	= temparry[j].pageindex + temparry[j+1].pageindex;
				temparry[j+1].pageindex = temparry[j].pageindex - temparry[j+1].pageindex;
				temparry[j].pageindex 	= temparry[j].pageindex - temparry[j+1].pageindex;
				
			}
		}
	}
	
	PXB_W_U8(ValidFlagindex,temparry[0].pageindex);
	PXB_W_U8(PrevalidFlagindex,temparry[0].pageindex);
		
	for( i = 0 ; i < memReadU8(validnums) - 1; i++ )
	{
		if( (temparry[i].count + 1) != temparry[i+1].count )
		{
			if(temparry[i].count == 0)
			{
				PXB_W_U8(PrevalidFlagindex,temparry[memReadU8(validnums) - 1].pageindex);
			}
			break;
		}
		else
		{
			
			PXB_W_U8(ValidFlagindex,temparry[i+1].pageindex);
			PXB_W_U8(PrevalidFlagindex,temparry[i].pageindex);
			
		}
		
	}
	
}

#endif //!CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE

#endif //!TRANSACTION_OLDDESIGN






