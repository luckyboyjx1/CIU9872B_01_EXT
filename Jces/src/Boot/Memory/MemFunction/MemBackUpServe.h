#ifndef   __MEMBACKUP_SERVE_H__
#define   __MEMBACKUP_SERVE_H__

#include "DataOperation.h"
#include "MemBackUp.h"
#include "bootConfigMethod.h"
#include "SysConfigTarget.h"
#include "bootConfigProfile.h"

/***********************************modify by dengyiwen@20220106*********************************/
#if !TRANSACTION_OLDDESIGN

extern u8 				X_A 							gBackUp_writeType ;
extern u8 				X_A 							gBackUp_cacheType ;
extern u8				X_A								BACK_UP_MAX_PAGE;
extern u8				X_A								FLASH_PROTECT_PAGE_NUM;
extern PN8     			X_A     						gBackUp_NotBackUpBottom;
extern PN8     			X_A     						gBackUp_NotBackUpTop;
extern u8    	 		X_A								g_Transaction_capacity;

	extern u8				X_A											gBackUp_PageNumStart;
	extern u1 				B_A     							gBackUp_inSingleApdu;
	extern u1				B_A    									gBackUp_inMutilApdu;
	extern PN8			    X_A									BACK_UP_AREA_START;
	
	
#if (BACK_UP_BUFFER_SIZE > EEPROM_BUFFER_SIZE)
	#define				gBackUp_TempBuf					"ERROR"
#else
	#define				gBackUp_TempBuf					r_memEepromBuffer
#endif

	#define checkIsBackUp(e2p_addr, length)			((e2p_addr >= PNtMEM(gBackUp_NotBackUpBottom)) && (U16_A_U32(length , e2p_addr) <= PNtMEM(gBackUp_NotBackUpTop)))
	#define MBUS_getNotBackUpBottom(addr)				U32_AND_U32(U32_A_U16(PNtMEM(addr) , EEPAGE_SIZE) , NOR_EEPAGE_MASK)
	#define MBUS_getNotBackUpTop(addr)					U32_AND_U32(PNtMEM(addr) , NOR_EEPAGE_MASK)
	#define get_unused_commit_capacity(void) 		U16_S_U16(BACK_UP_MAX_SIZE,U16_M_U16(U16_A_U16(g_Transaction_capacity,1),EEPAGE_SIZE)) //TODO:lyt

	
	
	extern  void clearBackUpPageFlag(void);
	extern  void abortTransactionByJCRE(void);
	extern  void save_E2P(memref e2p_addr, u16 length);
	extern	void commitSaveE2p(void);
	extern  void initTransactionMemBackUpGloableVar(void);
	extern  void Transaction_CommitFlag(PN8 tempBuffer);
	extern  void Transaction_RollBack(PN8 tempbuufer,PN8 reusebuffer,u8 isReset);


#if	CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE
	void Transaction_CommitFlag_Wordimpl(PN8 tempBuffer);
	memref Set_Block_Wordimpl(memref e2p_addr);
	memref Findflag_word(memref e2p_addr);
	extern memref g_Transaction_flagaddr;
	extern u16 	  g_Transaction_curblockoffset;
#else	
	typedef struct {
	
	 u16 count;
	 u8  pageindex;
	}Flagarry_ST;
	void 	 Transaction_CommitFlag_Pageimpl(PN8 tempBuffer);
	memref Set_Block_Pageimpl(memref e2p_addr,PN8 TempBuff);
	memref Findflag_page(memref e2p_addr);
	extern memref Transaction_FindValidFlagPage(PN8 tempbuffer);
	extern u8 		TransactionChekBlockCrc(PN8 tempbuffer);
	extern u16 	  g_Transaction_curblockoffset;
	extern void FindIndex_RollBack(PN8 ValidFlagindex,PN8 PrevalidFlagindex,PN8 validnums, u8 PageFlagNum ,Flagarry_ST temparry[]);
#endif

	#define TRANSACTION_SEQUENCE_LEN							0x10


	#define TRANSACTION_BAKUP_HEAD_MARK_VALUE						0xA5A55A5A	
	#define TRANSACTION_BAKUP_HEAD_COMMIT_VALUE					0xAA	
	#define TRANSACTION_BAKUP_PAD_VALUE									0xDD	
	#define TRANSACTION_BAKUP_SERIALNUMBER_VALUE				0x00	




	#define TRANSACTION_BAKUP_HEAD_MARK_SIZE						0x04
	#define TRANSACTION_BAKUP_HEAD_SERIALNUMBER_SIZE		0x02
	#define TRANSACTION_BAKUP_HEAD_NUM_SIZE							0x02
	#define TRANSACTION_BAKUP_HEAD_PAD_SIZE							0x00
	#define TRANSACTION_BAKUP_HEAD_CRC_SIZE							0x04
	#define TRANSACTION_BAKUP_HEAD_COMMIT_SIZE					0x04

	#define TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET					0x00
	#define TRANSACTION_BAKUP_HEAD_MARK_OFFSET						(TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET + TRANSACTION_BAKUP_HEAD_COMMIT_SIZE)
	#define TRANSACTION_BAKUP_HEAD_SERIALNUMBER_OFFSET		(TRANSACTION_BAKUP_HEAD_MARK_OFFSET + TRANSACTION_BAKUP_HEAD_MARK_SIZE)
	#define TRANSACTION_BAKUP_HEAD_NUM_OFFSET							(TRANSACTION_BAKUP_HEAD_SERIALNUMBER_OFFSET + TRANSACTION_BAKUP_HEAD_SERIALNUMBER_SIZE)
	#define TRANSACTION_BAKUP_HEAD_PAD_OFFSET							(TRANSACTION_BAKUP_HEAD_NUM_OFFSET + TRANSACTION_BAKUP_HEAD_NUM_SIZE)	
	#define TRANSACTION_BAKUP_HEAD_CRC_OFFSET							(TRANSACTION_BAKUP_HEAD_PAD_OFFSET + TRANSACTION_BAKUP_HEAD_PAD_SIZE)	


	#define TRANSACTION_BAKUP_HEAD_SIZE										(TRANSACTION_BAKUP_HEAD_MARK_SIZE + TRANSACTION_BAKUP_HEAD_SERIALNUMBER_SIZE + TRANSACTION_BAKUP_HEAD_NUM_SIZE + TRANSACTION_BAKUP_HEAD_PAD_SIZE + TRANSACTION_BAKUP_HEAD_CRC_SIZE + TRANSACTION_BAKUP_HEAD_COMMIT_SIZE)

	#define getTransactionHEAD_MARK(tempAddress)											PN_R_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_MARK_OFFSET))
	#define getTransactionHEAD_SERIALNUMBER(tempAddress)							PN_R_U16(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_SERIALNUMBER_OFFSET))
	#define getTransactionHEAD_NUM(tempAddress)												PN_R_U16(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_NUM_OFFSET))
	#define getTransactionHEAD_PAD(tempAddress1,tempAddress2)					ecpMem_Copy(tempAddress1,PM8_A_CONST(tempAddress2,TRANSACTION_BAKUP_HEAD_PAD_OFFSET),TRANSACTION_BAKUP_HEAD_PAD_SIZE)
	#define getTransactionHEAD_CRC(tempAddress)												PN_R_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_CRC_OFFSET))
	#define getTransactionHEAD_COMMIT(tempAddress1,tempAddress2)			ecpMem_Copy(tempAddress1,PM8_A_CONST(tempAddress2,TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET),TRANSACTION_BAKUP_HEAD_COMMIT_SIZE)


	#define setTransactionHEAD_MARK(tempAddress,mark)										PXB_W_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_MARK_OFFSET),mark)
	#define setTransactionHEAD_SERIALNUMBER(tempAddress,ser)						PXB_W_U16(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_SERIALNUMBER_OFFSET),ser)
	#define setTransactionHEAD_NUM(tempAddress,ser)											PXB_W_U16(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_NUM_OFFSET),ser)
	#define setTransactionHEAD_PAD(tempAddress)													ecpMem_Set(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_PAD_OFFSET),TRANSACTION_BAKUP_PAD_VALUE,TRANSACTION_BAKUP_HEAD_PAD_SIZE);
	#define setTransactionHEAD_CRC(tempAddress)													PXB_W_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_CRC_OFFSET),(u32)(Get_crc16(tempAddress, U8_S_U8(TRANSACTION_BAKUP_HEAD_SIZE, TRANSACTION_BAKUP_HEAD_CRC_SIZE))))
	#define setTransactionHEAD_COMMIT(tempAddress)											ecpMem_Set(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET),TRANSACTION_BAKUP_HEAD_COMMIT_VALUE,TRANSACTION_BAKUP_HEAD_COMMIT_SIZE);
	#define setTransactionHEAD_NOTFIN(destAdd,srcAdd)										ecpMem_Copy(PX8_A_U16(destAdd, TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET), PN8_A_U16(srcAdd, TRANSACTION_BAKUP_HEAD_COMMIT_OFFSET), TRANSACTION_BAKUP_HEAD_COMMIT_SIZE); 				


	#define TRANSACTION_BAKUP_ADDR_SIZE			0x04
	#define TRANSACTION_BAKUP_INDEX_SIZE		0x04
	#define TRANSACTION_BAKUP_PAD_SIZE			0x00
	#define TRANSACTION_BAKUP_CRC_SIZE			0x04

	#define TRANSACTION_BAKUP_ADDR_OFFSET		0x00
	#define TRANSACTION_BAKUP_INDEX_OFFSET		(TRANSACTION_BAKUP_ADDR_OFFSET + TRANSACTION_BAKUP_ADDR_SIZE)
	#define TRANSACTION_BAKUP_PAD_OFFSET		(TRANSACTION_BAKUP_INDEX_OFFSET + TRANSACTION_BAKUP_INDEX_SIZE)
	#define TRANSACTION_BAKUP_CRC_OFFSET		(TRANSACTION_BAKUP_PAD_OFFSET + TRANSACTION_BAKUP_PAD_SIZE)			

	#define TRANSACTION_BAKUP_BLOCK_SIZE 		(TRANSACTION_BAKUP_ADDR_SIZE + TRANSACTION_BAKUP_INDEX_SIZE + TRANSACTION_BAKUP_PAD_SIZE + TRANSACTION_BAKUP_CRC_SIZE)


	#define getTransactionBlockAddr(tempAddress)								PN_R_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_ADDR_OFFSET))
	#define getTransactionBlockIndex(tempAddress)								PN_R_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_INDEX_OFFSET))
	#define getTransactionBlockPAD(tempAddress1,tempAddress2)		ecpMem_Copy(tempAddress1,PM8_A_CONST(tempAddress2,TRANSACTION_BAKUP_PAD_OFFSET),TRANSACTION_BAKUP_PAD_SIZE)
	#define getTransactionBlockCRC(tempAddress)									PN_R_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_CRC_OFFSET))

	#define setTransactionBlockAddr(tempAddress,addr)						PXB_W_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_ADDR_OFFSET),addr)
	#define setTransactionBlockIndex(tempAddress,index)					PXB_W_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_INDEX_OFFSET),index)
	#define setTransactionBlockPAD(tempAddress)									ecpMem_Set(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_PAD_OFFSET),TRANSACTION_BAKUP_PAD_VALUE,TRANSACTION_BAKUP_PAD_SIZE);
	#define setTransactionBlockCRC(tempAddress)									PXB_W_U32(PM8_A_CONST(tempAddress,TRANSACTION_BAKUP_CRC_OFFSET),Get_crc16(tempAddress, U8_S_U8(TRANSACTION_BAKUP_BLOCK_SIZE, TRANSACTION_BAKUP_CRC_SIZE)))
		
	
	
	#define ILLEGAL_FLAG	0xFFFF

	#define ERASE_VALUE	0xFFFFFFFF
	#define TRANSACTION_BAKUP_PAGE_HEAD_COMMIT_VALUE 0xAAAAAAAA
	#define TRANSACTION_MAXCOUNT											0x06
	#define TRANSACTION_BACKUP_ILLEGALADDR						0xFF


#else //TRANSACTION_OLDDESIGN
/************************************************原事务保护方案************************************************************************************
 *************************************************************************************************************************************************/ 


extern u8 				X_A 							gBackUp_writeType ;
extern u8 				X_A 							gBackUp_cacheType ;
extern u8				X_A								BACK_UP_MAX_PAGE;
extern u8				X_A								FLASH_PROTECT_PAGE_NUM;
extern PN8     			X_A     						gBackUp_NotBackUpBottom;
extern PN8     			X_A     						gBackUp_NotBackUpTop;

#if BOOT__MEMORY__BACK_UP

/************************************************************************************************************************************************
*************************************************************************************************************************************************/

/******************************************global variable shared with other module**************************************************************
*************************************************************************************************************************************************/
extern u8				X_A								gBackUp_PageNumStart;
extern u16    			X_A     						gBackUp_curSize;
extern u16    			X_A     						gBackUp_curOffset;
extern u1 				B_A     						gBackUp_inSingleApdu;
extern u1				B_A    							gBackUp_inMutilApdu;
extern PN8			    X_A								BACK_UP_AREA_START;
#if (BACK_UP_BUFFER_SIZE > EEPROM_BUFFER_SIZE)
	#define				gBackUp_TempBuf					"ERROR"
#else
	#define				gBackUp_TempBuf					r_memEepromBuffer
#endif
/************************************************************************************************************************************************
*************************************************************************************************************************************************/
#endif


/************************************************handle back up page information*****************************************************************
*************************************************************************************************************************************************/
#define getBackUpPageFlag(tempAddress)					PN_R_U8(PN8_A_CONST(tempAddress,BACK_UP_PAGE_START_OFFSET))//getBackUpPageFlagAndCounter_IFX(tempAddress,BACK_UP_PAGE_START_OFFSET)//PN_R_U8(tempAddress)
#define	getBackUpPageCounter(tempAddress)				PN_R_U8(PN8_A_CONST(tempAddress, BACK_UP_PAGE_COUNTER_OFFSET))
#define	getBackUpPageCrc(tempAddress)					PN_R_U16(PN8_A_CONST(tempAddress, BACK_UP_PAGE_CRC_OFFSET))

#define setBackUpPageFlag(tempAddress, tempFlag)		PXB_W_U8(tempAddress, tempFlag)
#define setBackUpPageCounter(tempAddress, tempCounter)	PXB_W_U8(PX8_A_CONST(tempAddress, BACK_UP_PAGE_COUNTER_OFFSET), tempCounter)
#define setBackUpPageCrc(tempAddress, crc)				PXB_W_U16(PX8_A_CONST(tempAddress, BACK_UP_PAGE_CRC_OFFSET), crc)
#define calBackUpPageCrc(tempAddress)					(u16)(~PM_R_U16(tempAddress))

/************************************************************************************************************************************************
*************************************************************************************************************************************************/

/************************************************handle back up block information****************************************************************
*************************************************************************************************************************************************/
#define getBackUpBlockCrc(tempAddress)					PN_R_U16(PM8_A_CONST(tempAddress,BACK_UP_BLOCK_CRC_OFFSET))//getBackUpBlockCrcAndTypeSize_IFX(tempAddress,BACK_UP_BLOCK_CRC_OFFSET)//PN_R_U16(tempAddress)
#define getBackUpBlockTypeSize(tempAddress)				PM_R_U16(PM8_A_CONST(tempAddress,BACK_UP_BLOCK_TYPE_LENGTH_SIZE))//getBackUpBlockCrcAndTypeSize_IFX(tempAddress,BACK_UP_BLOCK_TYPE_LENGTH_SIZE)//PM_R_U16(PM8_A_CONST(tempAddress, BACK_UP_BLOCK_TYPE_LENGTH_SIZE))
#define getBackUpBlockAddress(tempAddress)				PM_R_U32(PM8_A_CONST(tempAddress,BACK_UP_BLOCK_ADDR_OFFSET))//getBackUpBlockAddress_IFX(tempAddress,BACK_UP_BLOCK_ADDR_OFFSET)//PM_R_U32(PM8_A_CONST(tempAddress, BACK_UP_BLOCK_ADDR_OFFSET))
#define getBackUpBlockContent(tempAddress)				PM8_A_CONST(tempAddress, BACK_UP_BLOCK_CONTENT_OFFSET)

#define setBackUpBlockCrc(tempAddress, crc)				PXB_W_U16(PX8_A_CONST(tempAddress, (BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_CRC_OFFSET)), crc)
#define setBackUpBlockTypeSize(tempAddress, ts)			PXB_W_U16(PX8_A_CONST(tempAddress, (BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_OFFSET)), ts)
#define setBackUpBlockAddress(tempAddress, des)			PXB_W_U32(PX8_A_CONST(tempAddress, (BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_ADDR_OFFSET)), PMtMEM(des))
#define setBackUpBlockContent(tempAddress, des, len)	ecpMem_Copy(PX8_A_CONST(tempAddress, (BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_HEADER_SIZE)), des, len)

#define setBackUpBlockCounter(tempAddress, counter)		PXB_W_U8(PX8_A_CONST(tempAddress, (BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_HEADER_SIZE)), counter)
/************************************************************************************************************************************************
*************************************************************************************************************************************************/

#define checkIsBackUp(e2p_addr, length)					((e2p_addr >= PNtMEM(gBackUp_NotBackUpBottom)) && (U16_A_U32(length , e2p_addr) <= PNtMEM(gBackUp_NotBackUpTop)))
#define programBackUpBlock(des,src,len)					MemNvmOperation_WriteOnly(des, src, len)
	#if BOOT__MEMORY__FLASH_BACK_UP
		#define isBackUpPageCommit(tempBackUpPage)			((TF_BACKUP == getBackUpPageFlag(tempBackUpPage)) && (!FlashBackUp_isRollBackPage(tempBackUpPage)))
		#define isBackUpPageRollBack(tempBackUpPage)		((TF_BACKUP == getBackUpPageFlag(tempBackUpPage)) && FlashBackUp_isRollBackPage(tempBackUpPage))
		#define MBUS_getNotBackUpBottom(addr)				U32_AND_U32(U32_A_U16(PNtMEM(addr) , EEPAGE_SIZE) , NOR_EEPAGE_MASK)
		#define MBUS_getNotBackUpTop(addr)					U32_AND_U32(PNtMEM(addr) , NOR_EEPAGE_MASK)
	#else
		#define isBackUpPageCommit(tempBackUpPage)			(TF_COMMIT == getBackUpPageFlag(tempBackUpPage))
		#define isBackUpPageRollBack(tempBackUpPage)		(TF_BACKUP == getBackUpPageFlag(tempBackUpPage))
		#define MBUS_getNotBackUpBottom(addr)				(addr)
		#define MBUS_getNotBackUpTop(addr)					U32_AND_U32(PNtMEM(addr) , NOR_EECELL_MASK)
	#endif
#if BOOT__MEMORY__BACK_UP
	extern  void clearBackUpPageFlag(void);
	extern  void abortTransactionByJCRE(void);
	extern  void copyBackUpData(PX8 des, PN8 src, u16 length);
	extern  u16 searchBlocks(PN8 backUpDes, u16 backUpLength, u16 lastFitOffset, u8 type);
	extern  void copyBackUpBlockInfo(PN8 blockInfoAddress, u16 eepBufferOffset, u16 blockInfoOffset, u16 InfoLength, PX8 tempBackUpTempBuf);
	extern  void save_E2P(memref e2p_addr, u16 length);
	extern	void commitSaveE2p(void);
#else
	#define clearBackUpPageFlag()
	#define abortTransactionByJCRE()
	#define copyBackUpData(des, src, length)
	#define searchBlocks(backUpDes, backUpLength, lastFitOffset, type)
	#define save_E2P(e2p_addr, length)
	#define commitSaveE2p()
#endif

#define get_unused_commit_capacity() 			U16_S_U16(BACK_UP_MAX_SIZE, gBackUp_curSize)

#endif

#endif /* __MEMBACKUP_SERVE_H__ */
