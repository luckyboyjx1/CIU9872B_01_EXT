#ifndef   __FLASHBACKUP_H__
#define   __FLASHBACKUP_H__

#include "bootConfigProfile.h"
#include "MemBackUp.h"
#include "MemNvmOperation.h"
#include "MemBackUp.h"

// modify by dengyiwen@20220106

#if !TRANSACTION_OLDDESIGN


#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	extern u8 			X_A										gFlash_Page1Num;
	extern u8			X_A										gFlash_FlagNum;
	extern u8 			X_A										gFlash_CurPage;
	extern u16			X_A										gFlash_BackUpFlagOffset;
	extern u16			X_A										gFlash_FlagBlockOffset;
#endif

#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
void FlashBackUp_PageProtect_Wordimpl(memref backUpDes, PX8 tempBuffer);
void FlashBackUp_CommitFlashFlag_Wordimpl(void);
#else
void FlashBackUp_PageProtect_Pageimpl(memref backUpDes, PX8 tempBuffer);
void FlashBackUp_CommitFlashFlag_Pageimpl(void);
#endif


	#define	MOST_CHECK_TIME										2

	#define BACK_UP_PAGE_NOT_COMMIT								0xFFFFFFFF
	#define	BACK_UP_PAGE_COMMIT									0xAAAAAAAA

	#define	FLASH_FLAG_NOT_FIN								0xFFFFFFFF
	#define FLASH_FLAG_FIN									0xAAAAAAAA
	#define	FLASH_FLAG_ILLEGAL_PAGE							0xFFFF

	#define BACK_UP_PAGE_COMMIT_BYTE				0xAA
	#define BACK_UP_PAGE_PAD_BYTE					0xDD	


	#define	FLASH_FLAG_INFO_ILLEGAL_OFFSET						0xFFFF
	#define	FLASH_FLAG_INFO_ILLEGAL_CRC							0xFFFF

	#define	FLASH_FLAG_INFO_DES_NULL							0x00000000

//	#define	FLASH_PROTECT_PAGE_NUM								0x03
//	#define BLOCKSIZE											0x04

	#define	FLASH_FLAG_INFO_DES_SIZE						4
	#define	FLASH_FLAG_INFO_PAGE_SIZE						4
	#define FLASH_FLAG_INFO_PAD_SIZE						0
	#define	FLASH_FLAG_INFO_CRC_SIZE						4
	#define	FLASH_FLAG_INFO_FIN_SIZE						4

	#define	FLASH_FLAG_INFO_DES_OFFSET							0
	#define	FLASH_FLAG_INFO_PAGE_OFFSET							(FLASH_FLAG_INFO_DES_OFFSET + FLASH_FLAG_INFO_DES_SIZE)
	#define	FLASH_FLAG_INFO_PAD_OFFSET							(FLASH_FLAG_INFO_PAGE_OFFSET + FLASH_FLAG_INFO_PAGE_SIZE)
	#define FLASH_FLAG_INFO_CRC_OFFSET							(FLASH_FLAG_INFO_PAD_OFFSET + FLASH_FLAG_INFO_PAD_SIZE)
	#define FLASH_FLAG_INFO_FIN_OFFSET							(FLASH_FLAG_INFO_CRC_OFFSET + FLASH_FLAG_INFO_CRC_SIZE)
	

	#define FLASH_FLAG_INFO_SIZE								(FLASH_FLAG_INFO_DES_SIZE + FLASH_FLAG_INFO_FIN_SIZE + \
																	FLASH_FLAG_INFO_PAGE_SIZE + FLASH_FLAG_INFO_CRC_SIZE + FLASH_FLAG_INFO_PAD_SIZE)
																	
	#define FLASH_FLAG_INFO_ILLEGAL								0xFF
	
	#define FLASH_FLAG_MAX_OFFSET								U16_S_U8(EEPAGE_SIZE, (EEPAGE_SIZE % FLASH_FLAG_INFO_SIZE))
	#define getFlashFlagInfoDes(tempAddress)																PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_DES_OFFSET))
	#define getFlashFlagInfoPage(tempAddress)																PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET))
	#define getFlashFlagInfoPad(tempAddress1,tempAddress2)									ecpMem_Copy(tempAddress1,PN8_A_U8(tempAddress2, FLASH_FLAG_INFO_PAD_OFFSET),padsize)																																			//PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET), page)		
	#define getFlashFlagInfoCrc(tempAddress)																PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_CRC_OFFSET))
	#define getFlashFlagInfoFinish(tempAddress)															getFlashFlagInfoFinish_impl(tempAddress)//PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET))

	#define FlashBackUp_ResumeFlashFlag()		(gFlash_FlagBlockOffset	= gFlash_BackUpFlagOffset)
	#define	FlashBackUp_IsSurpport()							true
		
	extern void 	FlashBackUp_RollBackPage(PD8 tempBuffer, u1 isReset);

#if CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	
	/************************************************handle flash flag information**************************************************************
	*************************************************************************************************************************************************/

		#define setFlashFlagInfoDes(tempAddress, des)						PX_W_U32((memref)PX8_A_U16(tempAddress, FLASH_FLAG_INFO_DES_OFFSET), des)
		#define setFlashFlagInfoPage(tempAddress, page)					PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET), page)
		#define setFlashFlagInfoPad(tempAddress, padsize)				ecpMem_Set(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_PAD_OFFSET),BACK_UP_PAGE_PAD_BYTE,padsize)																																			//PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET), page)
		#define setFlashFlagInfoCrc(tempAddress, crc)						PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_CRC_OFFSET), crc)
		#define setFlashFlagInfoFinish(tempAddress ,flag)				ecpMem_Set(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET),BACK_UP_PAGE_COMMIT_BYTE,FLASH_FLAG_INFO_FIN_SIZE)//PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET), flag)

		#define setFlashFlagInfoNotFinish(destAdd, srcAdd)		ecpMem_Copy(PX8_A_U16(destAdd, FLASH_FLAG_INFO_FIN_OFFSET), PN8_A_U16(srcAdd, FLASH_FLAG_INFO_FIN_OFFSET), FLASH_FLAG_INFO_FIN_SIZE)
	/************************************************************************************************************************************************
	*************************************************************************************************************************************************/

	extern void 	FlashBackUp_Init(void);
	extern void 	FlashBackUp_SetPageInfo(PX8 tempBuffer, PN8 tempAddress);
	extern void 	setFlashBackUpPageInfo(PX8 tempBuffer);
	extern void 	FlashBackUp_PageProtect(memref backUpDes, PX8 tempBuffer);
	extern void 	FlashBackUp_CommitFlashFlag(void);
	extern void 	FlashBackUp_commitPageInfo(PX8 tempBuffer);
	extern u8 		FlashBackUp_isRollBackPage(PN8 backUpPage);

#else
/***********************************************************************页擦页写*******************************************************************/
		#define NOFLAGPAGE															0xFF
		#define MAXCOUNT																50
		#define MAXNUM																	20
		#define PAGEPROTECTMARK													0x5A5AA5A5
		#define MAXBACKUPNUM													U8_D_U8(U8_S_U8(FLASH_PROTECT_PAGE_NUM ,1), 2)  //保证轮询不覆盖的最大页保护数据页数  （页保护区大小 - 1）/2
		#define MAXBACKUPINDEX													U8_S_U8(FLASH_PROTECT_PAGE_NUM ,1)
		#define PAGEPROTECTCOMMIT												0xAAAAAAAA
		#define PAGEPROTECTILLEGALADDR									0xFF
		
		#define FLASH_PAGEFLAG_PAGEHEAD_FLAG_SIZE				0x04																						//页保护flag页页头标志大小
		#define FLASH_PAGEFLAG_PAGEHEAD_COUNT_SIZE			0x02																						//页保护flag页页头count大小
		#define	FLASH_PAGEFLAG_PAGEHEAD_NUM_SIZE				0x02
		#define FLASH_PAGEFLAG_PAGEHEAD_CRC_SIZE				0x04																						//页保护flag页页头crc大小
		#define FLASH_PAGEFLAG_PAGEHEAD_SIZE						(FLASH_PAGEFLAG_PAGEHEAD_FLAG_SIZE + FLASH_PAGEFLAG_PAGEHEAD_COUNT_SIZE + \
																											FLASH_PAGEFLAG_PAGEHEAD_NUM_SIZE + FLASH_PAGEFLAG_PAGEHEAD_CRC_SIZE)
	
		#define FLASH_PAGEFLAG_BLOCK_DES_SIZE						0x04																						//页保护页备份flag块des大小
		#define FLASH_PAGEFLAG_BLOCK_INDEX_SIZE					0x04																						//页保护页备份flag块index大小
		#define FLASH_PAGEFLAG_BLOCK_CRC_SIZE						0x04																						//页保护页备份flag块CRC大小
		#define FLASH_PAGEFLAG_BLOCK_COMMIT_SIZE				0x04																						//页保护页备份flag块commit大小
		#define FLASH_PAGEFLAG_BLOCK_SIZE								(FLASH_PAGEFLAG_BLOCK_DES_SIZE + FLASH_PAGEFLAG_BLOCK_INDEX_SIZE + \
																											FLASH_PAGEFLAG_BLOCK_CRC_SIZE + FLASH_PAGEFLAG_BLOCK_COMMIT_SIZE)   
	
		#define FLASH_PAGEFLAG_PAGEHEAD_FLAG_OFFSET			0x00																																								//页保护区页头标志偏移
		#define FLASH_PAGEFLAG_PAGEHEAD_COUNT_OFFSET    (FLASH_PAGEFLAG_PAGEHEAD_FLAG_OFFSET + FLASH_PAGEFLAG_PAGEHEAD_FLAG_SIZE)				//页保护区页头count偏移
		#define	FLASH_PAGEFLAG_PAGEHEAD_NUM_OFFSET			(FLASH_PAGEFLAG_PAGEHEAD_COUNT_OFFSET + FLASH_PAGEFLAG_PAGEHEAD_COUNT_SIZE)
		#define FLASH_PAGEFLAG_PAGEHEAD_CRC_OFFSET			(FLASH_PAGEFLAG_PAGEHEAD_NUM_OFFSET + FLASH_PAGEFLAG_PAGEHEAD_NUM_SIZE)					//页保护区页头crc偏移
		#define FLASH_PAGEFLAG_PAGEHEAD_OFFSET					(FLASH_PAGEFLAG_PAGEHEAD_CRC_OFFSET + FLASH_PAGEFLAG_PAGEHEAD_CRC_SIZE)					//页保护区页头偏移
	
		#define FLASH_PAGEFLAG_BLOCK_DES_OFFSET					0x00																																								//页保护区备份块des偏移
		#define FLASH_PAGEFLAG_BLOCK_INDEX_OFFSET				(FLASH_PAGEFLAG_BLOCK_DES_OFFSET + FLASH_PAGEFLAG_BLOCK_DES_SIZE)								//页保护区备份块index偏移
		#define FLASH_PAGEFLAG_BLOCK_CRC_OFFSET					(FLASH_PAGEFLAG_BLOCK_INDEX_OFFSET + FLASH_PAGEFLAG_BLOCK_INDEX_SIZE)						//页保护区备份块crc偏移
		#define FLASH_PAGEFLAG_BLOCK_COMMIT_OFFSET			(FLASH_PAGEFLAG_BLOCK_CRC_OFFSET + FLASH_PAGEFLAG_BLOCK_CRC_SIZE)								//页保护区备份块commit偏移
		#define FLASH_PAGEFLAG_BLOCK_OFFSET							(FLASH_PAGEFLAG_BLOCK_COMMIT_OFFSET + FLASH_PAGEFLAG_BLOCK_COMMIT_SIZE)					//页保护区备份块偏移
		
		
		//页备份页头操作
		#define getPageProtectHeadFlag(tempAddr)											PN_R_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_FLAG_OFFSET))
		#define	getPageProtectHeadCount(tempAddr)											PN_R_U16(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_COUNT_OFFSET))
		#define getPageProtectHeadNum(tempAddr)												PN_R_U16(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_NUM_OFFSET))
		#define getPageProtectHeadCrc(tempAddr)												PN_R_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_CRC_OFFSET))
		
		#define setPageProtectHeadFlag(tempAddr)											PXB_W_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_FLAG_OFFSET),PAGEPROTECTMARK)
		#define setPageProtectHeadCount(tempAddr,count)								PXB_W_U16(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_COUNT_OFFSET),count)
		#define setPageProtectHeadNum(tempAddr,num)										PXB_W_U16(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_NUM_OFFSET),num)
		#define setPageProtectHeadCrc(tempAddr)												PXB_W_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_PAGEHEAD_CRC_OFFSET),Get_crc16(tempAddr,U8_S_U8(FLASH_PAGEFLAG_PAGEHEAD_SIZE,FLASH_PAGEFLAG_PAGEHEAD_CRC_SIZE)))
		
		//页备份块操作
		#define	getPageProtectBlockDes(tempAddr)											PN_R_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_DES_OFFSET))
		#define	getPageProtectBlockIndex(tempAddr)										PN_R_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_INDEX_OFFSET))
		#define	getPageProtectBlockCrc(tempAddr)											PN_R_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_CRC_OFFSET))
		#define	getPageProtectBlockCommit(tempAddr)										PN_R_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_COMMIT_OFFSET))
		
		#define	setPageProtectBlockDes(tempAddr,backUpDes) 						PXB_W_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_DES_OFFSET),backUpDes)
		#define	setPageProtectBlockIndex(tempAddr,index) 							PXB_W_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_INDEX_OFFSET),index)
		#define	setPageProtectBlockCommit(tempAddr,commit) 						PXB_W_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_COMMIT_OFFSET),commit)
		#define	setPageProtectBlockCrc(tempAddr,crc) 									PXB_W_U32(PM8_A_CONST(tempAddr,FLASH_PAGEFLAG_BLOCK_CRC_OFFSET),crc)
		
		
		extern PN8 FindBackUpPage();
		extern void FlashBackUp_WriteFlag(memref backUpDes,PX8 tempBuffer);
	//	extern void WriteFlagPageHead(PX8 tempBuffer);
		extern u8 IsAlreadyBackUp(memref backUpDes);
		extern PN8 FindValidFlagPage(PX8 TempBuffer);
		extern void FlashBackUp_PageProtect_Pageimpl(memref backUpDes, PX8 tempBuffer);
		extern void FlashBackUp_CommitFlashFlag_Pageimpl(void);
		extern u8 CheckPageProtectBlockCrc(PX8 PageFlagAddr);
		extern void 	FlashBackUp_Init(void);
		extern void 	FlashBackUp_CommitFlashFlag(void);
		extern void 	FlashBackUp_PageProtect(memref backUpDes, PX8 tempBuffer);
		
		#define PAGE_SEQUENCE_LEN	10
		
#endif		

#else  //TRANSACTION_OLDDESIGN

#if BOOT__MEMORY__FLASH_BACK_UP

	extern u8 			X_A										gFlash_Page1Num;
	extern u8			X_A										gFlash_FlagNum;
	extern u8 			X_A										gFlash_CurPage;
	extern u16			X_A										gFlash_BackUpFlagOffset;

	#define	MOST_CHECK_TIME										2

	#define BACK_UP_PAGE_NOT_COMMIT								0xFFFFFFFF
	#define	BACK_UP_PAGE_COMMIT									0xAAAAAAAA

	#if	BOOT__MEMORY__FLASH_BACK_UP_4_BYTE_WRITE
		#define	FLASH_FLAG_NOT_FIN								0xFFFFFFFF
		#define FLASH_FLAG_FIN									0xAAAAAAAA
		#define	FLASH_FLAG_ILLEGAL_PAGE							0xFFFF
	#else
		#define	FLASH_FLAG_NOT_FIN								0xFF
		#define FLASH_FLAG_FIN									0xAA
		#define	FLASH_FLAG_ILLEGAL_PAGE							0xFF
	#endif

	#define	FLASH_FLAG_INFO_ILLEGAL_OFFSET						0xFFFF
	#define	FLASH_FLAG_INFO_ILLEGAL_CRC							0xFFFF

	#define	FLASH_FLAG_INFO_DES_NULL							0x00000000

//	#define	FLASH_PROTECT_PAGE_NUM								0x03
//	#define BLOCKSIZE											0x04

	#define	FLASH_FLAG_INFO_DES_SIZE							4
	#if	BOOT__MEMORY__FLASH_BACK_UP_4_BYTE_WRITE
		#define	FLASH_FLAG_INFO_FIN_SIZE						4
		#define	FLASH_FLAG_INFO_PAGE_SIZE						2
	#else
		#define	FLASH_FLAG_INFO_FIN_SIZE						1
		#define	FLASH_FLAG_INFO_PAGE_SIZE						1
	#endif
	#define	FLASH_FLAG_INFO_CRC_SIZE							2

	#define	FLASH_FLAG_INFO_FIN_OFFSET							0
	#define	FLASH_FLAG_INFO_DES_OFFSET							(FLASH_FLAG_INFO_FIN_OFFSET + FLASH_FLAG_INFO_FIN_SIZE)
	#define	FLASH_FLAG_INFO_PAGE_OFFSET							(FLASH_FLAG_INFO_DES_OFFSET + FLASH_FLAG_INFO_DES_SIZE)
	#define FLASH_FLAG_INFO_CRC_OFFSET							(FLASH_FLAG_INFO_PAGE_OFFSET + FLASH_FLAG_INFO_PAGE_SIZE)

	#define FLASH_FLAG_INFO_SIZE								(FLASH_FLAG_INFO_DES_SIZE + FLASH_FLAG_INFO_FIN_SIZE + \
																	FLASH_FLAG_INFO_PAGE_SIZE + FLASH_FLAG_INFO_CRC_SIZE)
																	
	#define FLASH_FLAG_INFO_ILLEGAL								0xFF
	
	#define FLASH_FLAG_MAX_OFFSET								U16_S_U8(EEPAGE_SIZE, (EEPAGE_SIZE % FLASH_FLAG_INFO_SIZE))

	/************************************************handle flash back up page information***********************************************************
	*************************************************************************************************************************************************/
	#define getBackUpPageCommitFlag(tempAddress)				PX_R_U32(PN8_A_U8(tempAddress, BACK_UP_PAGE_COMMIT_FLAG_OFFSET))
	#define	getBackUpPageCommitCrc(tempAddress)					PX_R_U32(PN8_A_U8(tempAddress, BACK_UP_PAGE_COMMIT_CRC_OFFSET))

	#define setBackUpPageCommitFlag(tempAddress, flag)			PX_W_U32(PX8_A_U16(tempAddress, BACK_UP_PAGE_COMMIT_FLAG_OFFSET), flag)
	#define	setBackUpPageCommitCrc(tempAddress, crc)			PX_W_U32(PX8_A_U16(tempAddress, BACK_UP_PAGE_COMMIT_CRC_OFFSET), crc)

	#define calBackUpPageCommitCrc(tempAddress)					((u32)~PM_R_U32(PN8_A_U8(tempAddress, BACK_UP_PAGE_COMMIT_FLAG_OFFSET)))

	/************************************************************************************************************************************************
	*************************************************************************************************************************************************/

	/************************************************handle flash flag information**************************************************************
	*************************************************************************************************************************************************/
	#if	BOOT__MEMORY__FLASH_BACK_UP_4_BYTE_WRITE
		#define getFlashFlagInfoDes(tempAddress)				PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_DES_OFFSET))
		#define getFlashFlagInfoFinish(tempAddress)				PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET))
		#define getFlashFlagInfoPage(tempAddress)				PX_R_U16(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET))
		#define getFlashFlagInfoCrc(tempAddress)				PX_R_U16(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_CRC_OFFSET))

		#define setFlashFlagInfoDes(tempAddress, des)			PX_W_U32((memref)PX8_A_U16(tempAddress, FLASH_FLAG_INFO_DES_OFFSET), des)
		#define setFlashFlagInfoFinish(tempAddress, flag)		PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET), flag)
		#define setFlashFlagInfoPage(tempAddress, page)			PX_W_U16(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET), page)
		#define setFlashFlagInfoCrc(tempAddress, crc)			PX_W_U16(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_CRC_OFFSET), crc)
	#else
		#define getFlashFlagInfoDes(tempAddress)				PX_R_U32(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_DES_OFFSET))
		#define getFlashFlagInfoFinish(tempAddress)				PX_R_U8(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET))
		#define getFlashFlagInfoPage(tempAddress)				PX_R_U8(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET))
		#define getFlashFlagInfoCrc(tempAddress)				PX_R_U16(PN8_A_U8(tempAddress, FLASH_FLAG_INFO_CRC_OFFSET))

		#define setFlashFlagInfoDes(tempAddress, des)			PX_W_U32(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_DES_OFFSET), des)
		#define setFlashFlagInfoFinish(tempAddress, flag)		PX_W_U8(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_FIN_OFFSET), flag)
		#define setFlashFlagInfoPage(tempAddress, page)			PX_W_U8(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_PAGE_OFFSET), page)
		#define setFlashFlagInfoCrc(tempAddress, crc)			PX_W_U16(PX8_A_U16(tempAddress, FLASH_FLAG_INFO_CRC_OFFSET), crc)
	#endif
		#define setFlashFlagInfoNotFinish(destAdd, srcAdd)		ecpMem_Copy(PX8_A_U16(destAdd, FLASH_FLAG_INFO_FIN_OFFSET), PN8_A_U16(srcAdd, FLASH_FLAG_INFO_FIN_OFFSET), FLASH_FLAG_INFO_FIN_SIZE)
	/************************************************************************************************************************************************
	*************************************************************************************************************************************************/

	#define	FlashBackUp_IsSurpport()							true
	//extern void 	FlashBackUp_Init(void);
//	extern void 	FlashBackUp_ResumeFlashFlag(void);
	//#define FlashBackUp_ResumeFlashFlag()		(gFlash_FlagBlockOffset	= gFlash_BackUpFlagOffset)
	extern void 	FlashBackUp_SetPageInfo(PX8 tempBuffer, PN8 tempAddress);
	extern void 	setFlashBackUpPageInfo(PX8 tempBuffer);
	//extern void 	FlashBackUp_PageProtect(memref backUpDes, PX8 tempBuffer);
	//extern void 	FlashBackUp_CommitFlashFlag(void);
	extern void 	FlashBackUp_commitPageInfo(PX8 tempBuffer);
	extern u8 		FlashBackUp_isRollBackPage(PN8 backUpPage);
	//extern void 	FlashBackUp_RollBackPage(PD8 tempBuffer, u1 isReset);
	
//	extern u8		MemNvmOperation_Erase(PN8 dest, u16 length);
	#if BOOT__MEMORY__CHIP_PAGE_PROTECT
	#define FlashBackUp_clearFlagArea()
	#define FlashBackUp_Init()
	#define FlashBackUp_CommitFlashFlag()
	#define FlashBackUp_ResumeFlashFlag()
	#define FlashBackUp_PageProtect(backUpDes, tempBuffer)
	#define FlashBackUp_RollBackPage(tempBuffer, isReset)
	#define	FlashPageBackUp_IsSurpport()						false
	#define CHIP_PAGE_RECOVER_INIT_FUCNTION()           ecpNvm_FlashBackUp_RollBackPage()
	#else
	extern  void FlashBackUp_clearFlagArea(void);
	extern void 	FlashBackUp_Init(void);	
	extern void 	FlashBackUp_CommitFlashFlag(void);
	#define FlashBackUp_ResumeFlashFlag()		(gFlash_FlagBlockOffset	= gFlash_BackUpFlagOffset)
	extern void 	FlashBackUp_PageProtect(memref backUpDes, PX8 tempBuffer);
	extern void 	FlashBackUp_RollBackPage(PD8 tempBuffer, u1 isReset);
	#define	FlashPageBackUp_IsSurpport()						true
	#define CHIP_PAGE_RECOVER_INIT_FUCNTION()           
	
	#endif
#else
	#define FlashBackUp_IsSurpport()							false
	#define FlashBackUp_Init()
	#define FlashBackUp_ResumeFlashFlag()
	#define FlashBackUp_SetPageInfo(tempBuffer, tempAddress)
	#define FlashBackUp_PageProtect(backUpDes, tempBuffer)
	#define FlashBackUp_CommitFlashFlag()
	#define FlashBackUp_commitPageInfo(tempBuffer)
	#define FlashBackUp_isRollBackPage(backUpPage)				false
//	#define FlashBackUp_RollBackPage(tempBuffer, isReset)
	#define FlashBackUp_RollBackPage(tempBuffer, isReset) ecpNvm_FlashBackUp_RollBackPage()
//	#define MemNvmOperation_Erase(dest, length)
	#define FlashBackUp_clearFlagArea()
#endif

#endif //TRANSACTION_OLDDESIGN

#endif
