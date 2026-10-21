#ifndef   __MEMBACKUP_H__
#define   __MEMBACKUP_H__

#include "DataOperation.h"
#include "SysConfig.h"
#include "SysConfigTarget.h"
#include "bootConfigProfile.h"
#include "bootConfigMethod.h"
#include "MemBackUpServe.h"

//  modify by dengyiwen@20220106

#if !TRANSACTION_OLDDESIGN

/************************************************constant used in MemBackUp module***************************************************************
 *************************************************************************************************************************************************/
/***value of back up page flag***/
#define	TF_BACK_UP_NONE									0xFF
#define TF_BACKUP_First									0xEE
#define TF_BACKUP										0xA5
#define	TF_COMMIT										0x5A
#define	TF_CLEAR										0x00
#define TF_FLAG_SIZE									1

/***value of gBackUp_writeType and gBackUp_cacheType***/
#define WRITEEP_NOATOMIC          						0xD0
#define WRITEEP_ATOMIC            						0xB0
#define WRITEEP_TRANSACTION       						0xE0
#define	WRITEEP_LOST									0x70

/***value used in back up page information***/
#define BACK_UP_PAGE_FLAG_SIZE						1
#define BACK_UP_PAGE_COUNTER_SIZE					1
#define BACK_UP_PAGE_CRC_SIZE						2
	
#define BACK_UP_PAGE_START_OFFSET					0
#define BACK_UP_PAGE_COUNTER_OFFSET					(BACK_UP_PAGE_START_OFFSET + BACK_UP_PAGE_FLAG_SIZE)
#define BACK_UP_PAGE_CRC_OFFSET						(BACK_UP_PAGE_COUNTER_OFFSET + BACK_UP_PAGE_COUNTER_SIZE)



#define BACK_UP_PAGE_COMMIT_FLAG_OFFSET				(BACK_UP_PAGE_CRC_OFFSET + BACK_UP_PAGE_CRC_SIZE)
#define BACK_UP_PAGE_COMMIT_CRC_OFFSET				(BACK_UP_PAGE_COMMIT_FLAG_OFFSET + BACK_UP_PAGE_COMMIT_FLAG_SIZE)
#define BACK_UP_PAGE_COMMIT_FLAG_SIZE				4
#define BACK_UP_PAGE_COMMIT_CRC_SIZE				4
#if BOOT__MEMORY__FLASH_BACK_UP

	#define BACK_UP_PAGE_BLOCK_START_OFFSET				(BACK_UP_PAGE_COMMIT_CRC_OFFSET + BACK_UP_PAGE_COMMIT_CRC_SIZE)

	#define BACK_UP_PAGE_HEADER_SIZE					(BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE + BACK_UP_PAGE_COMMIT_FLAG_SIZE + BACK_UP_PAGE_COMMIT_CRC_SIZE)	
#else
	#define BACK_UP_PAGE_BLOCK_START_OFFSET				(BACK_UP_PAGE_CRC_OFFSET + BACK_UP_PAGE_CRC_SIZE)
	#define BACK_UP_PAGE_HEADER_SIZE					(BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE)
#endif
/***value used in back up block information***/
#define	BACK_UP_BLOCK_CRC_OFFSET						0
#define BACK_UP_BLOCK_TYPE_LENGTH_OFFSET				(BACK_UP_BLOCK_CRC_OFFSET + BACK_UP_BLOCK_CRC_SIZE)
#define BACK_UP_BLOCK_ADDR_OFFSET						(BACK_UP_BLOCK_TYPE_LENGTH_OFFSET + BACK_UP_BLOCK_TYPE_LENGTH_SIZE)
#define BACK_UP_BLOCK_CONTENT_OFFSET					(BACK_UP_BLOCK_ADDR_OFFSET + BACK_UP_BLOCK_ADDR_SIZE)

#define BACK_UP_BLOCK_CRC_SIZE							2
#define BACK_UP_BLOCK_TYPE_LENGTH_SIZE					2
#define BACK_UP_BLOCK_ADDR_SIZE							4
#define BACK_UP_BLOCK_HEADER_SIZE						(BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE + BACK_UP_BLOCK_ADDR_SIZE)

#define BACK_UP_BLOCK_TYPE_SHIFT_BIT					8
#define BACK_UP_BLOCK_TYPE_MASK							0xF0
#define BACK_UP_BLOCK_LENGTH_MASK						0x0FFF

/***function of getNextBlockOffset***/
#define GET_NEW_BLOCK_OFFSET							0x01
#define SEARCH_NEXT_BLOCK								0x02

/***function of searchBlocks***/
#define CHECK_IS_BACK_UP								0x01
#define CHECK_IS_ROLL_BACK								0x02
#define GET_LAST_BLOCK									0x03
#define READ_RECENT_BACK_UP_DATA						0x04
#define READ_PRE_BACK_UP_DATA							0x05

/***return of searchBlocks***/
#define ALREADY_BACK_UP									0xA0A0
#define NOT_BACK_UP_YET									0x0000
#define ILLEGAL_BLOCK_OFFSET							0xFFFF
#define FIRST_BLOCK_IS_ERROR							0xFFFE

/***parameter of abort_transaction***/
#define ABORT_ON_RESET									1
#define ABORT_NOT_ON_RESET								0

/***parameter of begin_transaction***/
#define SINGLE_APDU_PROCESS								1
#define MULTIL_APDU_PROCESS								2

/***other constant***/
#define BACK_UP_PAGE_FRAG								(BACK_UP_BLOCK_HEADER_SIZE + EECELL_SIZE)
#define BACK_UP_MAX_SIZE								U16_M_U16(BACK_UP_MAX_PAGE , EEPAGE_SIZE)
#define	MAX_BACK_UP_COUNT								250
#define BLOCK_PART_1									0x01
#define BLOCK_PART_2									0x02
#define BACK_UP_BUFFER_SIZE								(BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_HEADER_SIZE + EEPAGE_SIZE + BACK_UP_PAGE_COUNTER_SIZE)
#define TRANSACTION_BUFFER_OVERFLOW						((u16)0x0101)

#if BOOT__MEMORY__BACK_UP
	/*
	 * 开启事物
	 * */
	extern  void memBeginTransaction(u8 isMultiApduProcess);
	/*
	 * 终止事物
	 * */
	extern void memAbortTransaction(u1 isReset);
	/*
	 * 事物提交
	 * */
	extern  void memCommitTransaction(void);

	/*
	 * JCRE单独封装的函数，应用不能调用
	 * */
	extern void memSetInProgress(void);

	extern  void memInitTransaction(void);
	extern  void memCopyBackUpData(PX8 des, PN8 src, u16 length);

	/*
	 * 获取Membackup 的起始地址
	 */

#define isInProgress()  (gBackUp_inSingleApdu || gBackUp_inMutilApdu)

#define setWriteEepType(typeValue) 			gBackUp_writeType = typeValue
#define setCacheDataType(typeValue) 		gBackUp_cacheType = typeValue
#define getWriteEepType() 					gBackUp_writeType
#define getCacheDataType() 					gBackUp_cacheType
#define isWriteAndCacheTheSame()  			(gBackUp_writeType == gBackUp_cacheType)
#define isInSingleApduProgress()  			gBackUp_inSingleApdu
#define isInMultiApduProgress() 			gBackUp_inMutilApdu
#define closeProgress()  					gBackUp_inSingleApdu = false;\
											gBackUp_inMutilApdu = false;
#define openBackUpPageNumStart() 			gBackUp_PageNumStart = TF_BACKUP_First
#define get_max_commit_capacity() 			(BACK_UP_MAX_SIZE)
//#define get_unused_commit_capacity()  		U16_S_U16(BACK_UP_MAX_SIZE, gBackUp_curSize)
#define getBackupStart() 					(BACK_UP_AREA_START)
#define getBackUpMaxPage() 					(BACK_UP_MAX_PAGE)

//	extern  memref getBackupStart(void);
	extern  void initMemBackUpGloableVar(void);
	extern  void memSetNoBackAreaPointer(PF8 maxEntryAddr, PF8 usedHeapAddr);
	extern  void FlashBackUp_clearFlagArea(void);
//	extern  u1 isInProgress(void);
//	extern  void setWriteEepType(u8 typeValue);
//	extern  void setCacheDataType(u8 typeValue)	;
//	extern  u8 getWriteEepType(void);
//	extern  u8 getCacheDataType(void);
//	extern  u1 isWriteAndCacheTheSame(void);
//	extern  u1 isInSingleApduProgress(void);
//	extern  u1 isInMultiApduProgress(void);
//	extern  void closeProgress(void);
//	extern  void openBackUpPageNumStart(void);
//	extern  u16 get_max_commit_capacity(void);
//	extern  u16 get_unused_commit_capacity(void);
//	extern  u8 getBackUpMaxPage(void);
	#define memCopyBackUpData 						copyBackUpData
#else
	#define  memBeginTransaction(isMultiApduProcess)
	#define  memAbortTransaction(isReset)
	#define  memCommitTransaction()
	#define  memSetInProgress()
	#define  memInitTransaction()
	#define  memCopyBackUpData(des, src, length)
	#define  getBackupStart()														0
	#define  initMemBackUpGloableVar()
	#define  memSetNoBackAreaPointer(maxEntryAddr, usedHeapAddr)
	#define  isInProgress()															0
	#define  setWriteEepType(typeValue)
	#define  setCacheDataType(typeValue)	
	#define  getWriteEepType()														0
	#define  getCacheDataType()														0
	#define  isWriteAndCacheTheSame()												0
	#define  isInSingleApduProgress()												0
	#define  isInMultiApduProgress()												0
	#define  closeProgress()
	#define  openBackUpPageNumStart()
	#define  get_max_commit_capacity()												0
	#define  get_unused_commit_capacity()											0
	#define  getBackUpMaxPage()														0
	#define  memCopyBackUpData(des, src, length)
#endif

#else	//TRANSACTION_OLDDESIGN

/************************************************constant used in MemBackUp module***************************************************************
 *************************************************************************************************************************************************/
/***value of back up page flag***/
#define	TF_BACK_UP_NONE									0xFF
#define TF_BACKUP_First									0xEE
#define TF_BACKUP										0xA5
#define	TF_COMMIT										0x5A
#define	TF_CLEAR										0x00
#define TF_FLAG_SIZE									1

/***value of gBackUp_writeType and gBackUp_cacheType***/
#define WRITEEP_NOATOMIC          						0xD0
#define WRITEEP_ATOMIC            						0xB0
#define WRITEEP_TRANSACTION       						0xE0
#define	WRITEEP_LOST									0x70

/***value used in back up page information***/
#define BACK_UP_PAGE_FLAG_SIZE						1
#define BACK_UP_PAGE_COUNTER_SIZE					1
#define BACK_UP_PAGE_CRC_SIZE						2
	
#define BACK_UP_PAGE_START_OFFSET					0
#define BACK_UP_PAGE_COUNTER_OFFSET					(BACK_UP_PAGE_START_OFFSET + BACK_UP_PAGE_FLAG_SIZE)
#define BACK_UP_PAGE_CRC_OFFSET						(BACK_UP_PAGE_COUNTER_OFFSET + BACK_UP_PAGE_COUNTER_SIZE)



#define BACK_UP_PAGE_COMMIT_FLAG_OFFSET				(BACK_UP_PAGE_CRC_OFFSET + BACK_UP_PAGE_CRC_SIZE)
#define BACK_UP_PAGE_COMMIT_CRC_OFFSET				(BACK_UP_PAGE_COMMIT_FLAG_OFFSET + BACK_UP_PAGE_COMMIT_FLAG_SIZE)
#define BACK_UP_PAGE_COMMIT_FLAG_SIZE				4
#define BACK_UP_PAGE_COMMIT_CRC_SIZE				4
#if BOOT__MEMORY__FLASH_BACK_UP

	#define BACK_UP_PAGE_BLOCK_START_OFFSET				(BACK_UP_PAGE_COMMIT_CRC_OFFSET + BACK_UP_PAGE_COMMIT_CRC_SIZE)

	#define BACK_UP_PAGE_HEADER_SIZE					(BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE + BACK_UP_PAGE_COMMIT_FLAG_SIZE + BACK_UP_PAGE_COMMIT_CRC_SIZE)	
#else
	//#define BACK_UP_PAGE_BLOCK_START_OFFSET				(BACK_UP_PAGE_CRC_OFFSET + BACK_UP_PAGE_CRC_SIZE)
	//#define BACK_UP_PAGE_HEADER_SIZE					(BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE)
	#define BACK_UP_PAGE_BLOCK_START_OFFSET				(BACK_UP_PAGE_COMMIT_CRC_OFFSET + BACK_UP_PAGE_COMMIT_CRC_SIZE)

	#define BACK_UP_PAGE_HEADER_SIZE					(BACK_UP_PAGE_FLAG_SIZE + BACK_UP_PAGE_COUNTER_SIZE + BACK_UP_PAGE_CRC_SIZE + BACK_UP_PAGE_COMMIT_FLAG_SIZE + BACK_UP_PAGE_COMMIT_CRC_SIZE)	
#endif
/***value used in back up block information***/
#define	BACK_UP_BLOCK_CRC_OFFSET						0
#define BACK_UP_BLOCK_TYPE_LENGTH_OFFSET				(BACK_UP_BLOCK_CRC_OFFSET + BACK_UP_BLOCK_CRC_SIZE)
#define BACK_UP_BLOCK_ADDR_OFFSET						(BACK_UP_BLOCK_TYPE_LENGTH_OFFSET + BACK_UP_BLOCK_TYPE_LENGTH_SIZE)
#define BACK_UP_BLOCK_CONTENT_OFFSET					(BACK_UP_BLOCK_ADDR_OFFSET + BACK_UP_BLOCK_ADDR_SIZE)

#define BACK_UP_BLOCK_CRC_SIZE							2
#define BACK_UP_BLOCK_TYPE_LENGTH_SIZE					2
#define BACK_UP_BLOCK_ADDR_SIZE							4
#define BACK_UP_BLOCK_HEADER_SIZE						(BACK_UP_BLOCK_CRC_SIZE + BACK_UP_BLOCK_TYPE_LENGTH_SIZE + BACK_UP_BLOCK_ADDR_SIZE)

#define BACK_UP_BLOCK_TYPE_SHIFT_BIT					8
#define BACK_UP_BLOCK_TYPE_MASK							0xF0
#define BACK_UP_BLOCK_LENGTH_MASK						0x0FFF

/***function of getNextBlockOffset***/
#define GET_NEW_BLOCK_OFFSET							0x01
#define SEARCH_NEXT_BLOCK								0x02

/***function of searchBlocks***/
#define CHECK_IS_BACK_UP								0x01
#define CHECK_IS_ROLL_BACK								0x02
#define GET_LAST_BLOCK									0x03
#define READ_RECENT_BACK_UP_DATA						0x04
#define READ_PRE_BACK_UP_DATA							0x05

/***return of searchBlocks***/
#define ALREADY_BACK_UP									0xA0A0
#define NOT_BACK_UP_YET									0x0000
#define ILLEGAL_BLOCK_OFFSET							0xFFFF
#define FIRST_BLOCK_IS_ERROR							0xFFFE

/***parameter of abort_transaction***/
#define ABORT_ON_RESET									1
#define ABORT_NOT_ON_RESET								0

/***parameter of begin_transaction***/
#define SINGLE_APDU_PROCESS								1
#define MULTIL_APDU_PROCESS								2

/***other constant***/
#define BACK_UP_PAGE_FRAG								(BACK_UP_BLOCK_HEADER_SIZE + EECELL_SIZE)
#define BACK_UP_MAX_SIZE								U16_M_U16(BACK_UP_MAX_PAGE , EEPAGE_SIZE)
#define	MAX_BACK_UP_COUNT								250
#define BLOCK_PART_1									0x01
#define BLOCK_PART_2									0x02
#define BACK_UP_BUFFER_SIZE								(BACK_UP_PAGE_HEADER_SIZE + BACK_UP_BLOCK_HEADER_SIZE + EEPAGE_SIZE + BACK_UP_PAGE_COUNTER_SIZE)
#define TRANSACTION_BUFFER_OVERFLOW						((u16)0x0101)

#if BOOT__MEMORY__BACK_UP
	/*
	 * 开启事物
	 * */
	extern  void memBeginTransaction(u8 isMultiApduProcess);
	/*
	 * 终止事物
	 * */
	extern void memAbortTransaction(u1 isReset);
	/*
	 * 事物提交
	 * */
	extern  void memCommitTransaction(void);

	/*
	 * JCRE单独封装的函数，应用不能调用
	 * */
	extern void memSetInProgress(void);

	extern  void memInitTransaction(void);
	extern  void memCopyBackUpData(PX8 des, PN8 src, u16 length);

	/*
	 * 获取Membackup 的起始地址
	 */

#define isInProgress()  (gBackUp_inSingleApdu || gBackUp_inMutilApdu)

#define setWriteEepType(typeValue) 			gBackUp_writeType = typeValue
#define setCacheDataType(typeValue) 		gBackUp_cacheType = typeValue
#define getWriteEepType() 					gBackUp_writeType
#define getCacheDataType() 					gBackUp_cacheType
#define isWriteAndCacheTheSame()  			(gBackUp_writeType == gBackUp_cacheType)
#define isInSingleApduProgress()  			gBackUp_inSingleApdu
#define isInMultiApduProgress() 			gBackUp_inMutilApdu
#define closeProgress()  					gBackUp_inSingleApdu = false;\
											gBackUp_inMutilApdu = false;
#define openBackUpPageNumStart() 			gBackUp_PageNumStart = TF_BACKUP_First
//#define get_max_commit_capacity() 			(BACK_UP_MAX_SIZE)
#define get_max_commit_capacity() 			(BACK_UP_MAX_SIZE + EEPAGE_SIZE)
//#define get_unused_commit_capacity()  		U16_S_U16(BACK_UP_MAX_SIZE, gBackUp_curSize)
#define getBackupStart() 					(BACK_UP_AREA_START)
#define getBackUpMaxPage() 					(BACK_UP_MAX_PAGE)

//	extern  memref getBackupStart(void);
	extern  void initMemBackUpGloableVar(void);
	extern  void memSetNoBackAreaPointer(PF8 maxEntryAddr, PF8 usedHeapAddr);
//	extern  void FlashBackUp_clearFlagArea(void);
//	extern  u1 isInProgress(void);
//	extern  void setWriteEepType(u8 typeValue);
//	extern  void setCacheDataType(u8 typeValue)	;
//	extern  u8 getWriteEepType(void);
//	extern  u8 getCacheDataType(void);
//	extern  u1 isWriteAndCacheTheSame(void);
//	extern  u1 isInSingleApduProgress(void);
//	extern  u1 isInMultiApduProgress(void);
//	extern  void closeProgress(void);
//	extern  void openBackUpPageNumStart(void);
//	extern  u16 get_max_commit_capacity(void);
//	extern  u16 get_unused_commit_capacity(void);
//	extern  u8 getBackUpMaxPage(void);
	#define memCopyBackUpData 						copyBackUpData
#else
	#define  memBeginTransaction(isMultiApduProcess)
	#define  memAbortTransaction(isReset)
	#define  memCommitTransaction()
	#define  memSetInProgress()
	#define  memInitTransaction()
	#define  memCopyBackUpData(des, src, length)
	#define  getBackupStart()														0
	#define  initMemBackUpGloableVar()
	#define  memSetNoBackAreaPointer(maxEntryAddr, usedHeapAddr)
	#define  isInProgress()															0
	#define  setWriteEepType(typeValue)
	#define  setCacheDataType(typeValue)	
	#define  getWriteEepType()														0
	#define  getCacheDataType()														0
	#define  isWriteAndCacheTheSame()												0
	#define  isInSingleApduProgress()												0
	#define  isInMultiApduProgress()												0
	#define  closeProgress()
	#define  openBackUpPageNumStart()
	#define  get_max_commit_capacity()												0
	#define  get_unused_commit_capacity()											0
	#define  getBackUpMaxPage()														0
	#define  memCopyBackUpData(des, src, length)
#endif

#endif //TRANSACTION_OLDDESIGN

#endif  //__MEMBACKUP_H__
