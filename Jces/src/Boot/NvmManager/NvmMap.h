/*
 * MapTable.h
 *
 *  Created on: 2014年10月28日
 *      Author: linguoming1
 */

#ifndef __NVM_MAP_H_
#define __NVM_MAP_H_
#include <setjmp.h>
#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "JcvmConfig.h"
#include "mask.h"
#ifndef MAP_ENTRY_SIZE
#define MAP_ENTRY_SIZE	                            				0x04
#endif

#define MAP_TABLE_ENTRY_STATUS_AVAIL                   				0x00
#define MAP_TABLE_ENTRY_STATUS_CREATION                				0x01
#define MAP_TABLE_ENTRY_STATUS_FREE                    				0x03

#if SE_ONLY
	#define MapTable_IsCurrentProfile(entryIndex, profileId) 		true
#else
	extern u1     MapTable_IsCurrentProfile(u16 entryIndex, u8 profileId);
#endif

#define MAP_TABLE_ENTRY_STATUS_MASK                     			0x03

#define MAP_ENTRY_OBJECT_PCK_ID										0x80
#define MAP_ENTRY_BLOCK_PCK_ID										0xC0

#define	MAP_ENTRY_ENTITY_TYPE										0xFF
#define MAP_ENTRY_ENTITY_ALIGN_SIZE									0x04
#define	MAP_ENTRY_ENTITY_TYPE_SIZE									1
#define MAP_ENTRY_ENTITY_RFU_SIZE									1
#define MAP_ENTRY_ENTITY_LEN_SIZE									2

#define MAP_ENTRY_ENTITY_HEAER_SIZE									(MAP_ENTRY_ENTITY_TYPE_SIZE + MAP_ENTRY_ENTITY_RFU_SIZE + MAP_ENTRY_ENTITY_LEN_SIZE)

#define	MAP_ENTRY_ENTITY_TYPE_OFFSET								0
#define	MAP_ENTRY_ENTITY_RFU_OFFSET									(MAP_ENTRY_ENTITY_TYPE_OFFSET + MAP_ENTRY_ENTITY_TYPE_SIZE)
#define MAP_ENTRY_ENTITY_LEN_OFFSET									(MAP_ENTRY_ENTITY_RFU_OFFSET + MAP_ENTRY_ENTITY_RFU_SIZE)

#define MAP_TABLE_ILLEGAL_INDEX										0

#if BOOT__MEMORY__NVM_MANAGER_MAP_TABLE
	typedef struct
	{
		#if HAL__MEMORY__POINTER_24_BIT
			PN24		entry_addr;
			u8			pckId;
		#else
			PN32		entry_addr;
		#endif
	} MapEntry, *MapEntry_t;

	#define MAP_TABLE_ILLEGAL_OBJECT_EXCEPTION            			0x8010

	#define	MAP_BITMAP_SEG_SIZE										0x80
	#define	MAP_BITMAP_SIZE											U8_M_U8(JcvmConfig_GetMapBitTabSeg(), MAP_BITMAP_SEG_SIZE)

#if SE_ONLY
	#define MAP_BITMAP_CACHE_PAGE_SIZE								0x10
	#define	MAP_BITMAP_CACHE_PAGE_MASK								0xFFFFFFF0
	#define	MAP_BITMAP_CACHE_INDEX_MASK								0x000F
	#define MAP_BITMAP_ENTRY										0x04   //表示的是BITMAP的一个字节表示的对象的个数
#else
	#define MAP_BITMAP_CACHE_PAGE_SIZE								0x10*4
	#define	MAP_BITMAP_CACHE_PAGE_MASK								0xFFFFFFC0
	#define	MAP_BITMAP_CACHE_INDEX_MASK								0x003F
	#define MAP_BITMAP_ENTRY										0x01
#endif
	#define	MAP_BITMAP_CACHE_BIT_MASK								0x03
	#define MAP_BITMAP_CACHE_BIT_SHIFT								0x02
	#define MAP_BITMAP_CACHE_NUM_PER_BYTE							0x04

#if BIG_ENDIAN
//Big endian
	#define MAP_ENTRY_ENTITY_ADDRSS_OFFSET                       	1
	#define MAP_ENTRY_PKG_OFFSET                        			0
#else
//little endian
	#define MAP_ENTRY_ENTITY_ADDRSS_OFFSET                       	0
	#define MAP_ENTRY_PKG_OFFSET                        			3
#endif


	#define MAP_ADDRESS_INFO_SHIFT									8
	
	#if HAL__MEMORY__POINTER_24_BIT
		#define MapTable_GetEntryInfoAddr(entry)					PNtMEM(entry->entry_addr)
		#define MapTable_GetEntryInfoPckId(entry)					(entry->pckId)
	#else
		#define MapTable_GetEntryInfoAddr(entry)					dealMAPAddress(entry,0)//PNtMEM(U32_OR_U32(PN_R_U24(MEMtPN8(entry)),ROM_BASE))
		#define MapTable_GetEntryInfoPckId(entry)					PN_R_U8(PN8_A_CONST(MEMtPF8(entry), MAP_ENTRY_PKG_OFFSET))
	#endif

#if (!MAP_ADDRESS_NEED_DEAL&&!RUN_IN_SIMULATOR)
#define dealMAPAddress(mapEntry, index)	PN_R_U24(MEMtPN8(mapEntry))

#else
	extern memref dealMAPAddress(MapEntry_t  mapEntry, u16 index);
#endif

	extern u16    MapTable_GetCacheCopySize(void);
	extern void   MapTable_SetEntryStatus(u16 entryIndex, u8 status);
	extern u8     MapTable_GetEntryStatus(u16 entryIndex);
	extern memref MapTable_GetBase(PN8 topNvm);
	extern void   MapTable_Boot(u16 appStartIndex);
	extern void   MapTable_Init(PN8 SysEntryStart, PN8 AppEntryStart, u16 appStartIndex);
	extern void   MapTable_ResetCache(void);
	extern void   MapTable_Commit(void);
	extern void   MapTable_Reset(void);
	extern u16    MapTable_GetAvailableIndex(void);
	extern memref MapTable_GetMapTopAddress(void);
	extern u16    MapTable_GetTopIndex(void);
	extern memref MapTable_GetAppMap(void);
	extern memref MapTable_GetAppBitMap(void);
	extern u16    MapTable_GetAppStartIndex(void);
	extern u1 	  MapTable_IsCreationObject(u16 objectId);
	extern u16 	  MapTable_AddEntry(PN8 address, u8 pckId);
	extern memref MapTable_GetObjAddr(u16 objectId);
	extern memref MapTable_GetEntityContent(u16 entryIndex);
	extern u32 	  MapTable_GetEntitySize(PN8 blockAddr);
	extern memref MapTable_GetEntryAddr(u16 entryIndex);
	extern u8 	  MapTable_GetEntryPckId(u16 entryIndex);
	extern void   MapTable_SetEntryAddr(u16 entryIndex, PN8 blockStart);
	extern void   MapTable_SetEntryInfo(u16 entryIndex, PN8 blockStart, u8 pckId);
#else
	#define	MapTable_GetCacheCopySize()											0
	#define	MapTable_SetEntryStatus(entryIndex, status)
	#define	MapTable_GetEntryStatus(entryIndex)									0
	#define MapTable_GetBase(topNvm)											0
	#define MapTable_Boot(appStartIndex)
	#define	MapTable_Init(SysEntryStart, AppEntryStart, appStartIndex)
	#define MapTable_ResetCache()
	#define MapTable_Commit()
	#define MapTable_Reset()
	#define	MapTable_GetAvailableIndex()										0
	#define	MapTable_GetMapTopAddress()											0
	#define	MapTable_GetTopIndex()												0
	#define	MapTable_GetAppStartIndex()											0
	#define MapTable_IsCreationObject(index)									0
	#define MapTable_AddEntry(address, pckId)									0
	#define MapTable_GetObjAddr(objectId)										0
	#define MapTable_GetEntityContent(entryIndex)								0
	#define MapTable_GetEntitySize(blockAddr)									0
	#define MapTable_GetEntryAddr(entryIndex)									0
	#define MapTable_GetEntryPckId(entryIndex)									0
	#define MapTable_SetEntryAddr(entryIndex, blockStart)
	#define MapTable_SetEntryInfo(entryIndex, blockStart, pckId)
#endif

#endif /* MAPTABLE_H_ */
