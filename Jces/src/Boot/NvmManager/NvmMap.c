/*
 * MapTable.c
 *
 *  Created on: 2014年10月28日
 *      Author: linguoming1
 */

#include "NvmMap.h"
#include "MemOperation.h"
#include "MemBackUp.h"
#include "BootErrorHandle.h"
#include "FreeTable.h"
#include "BootRegister.h"
#include "BootAreaInitRam.h"
#include "profile.h"
#include "mask.h"
#include "Interface_PlatformFromLTE.h"
#include "NvmManager.h"
#include "BootLifeCycle.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if BOOT__MEMORY__NVM_MANAGER_MAP_TABLE

static	u16			X_A			gNvmMap_TopIndex;
static  u16			X_A			gNvmMap_AppStartIndex; //TODO: mwp: 有必须用全局变量吗？ 芯片空间本来空间已经很小了，还创建这些没必要的全局变量。 这个变量是从mask.c 的宏取值。
static	PN8			X_A			gNvmMap_AppEntryStart;
static	PN8			X_A			gNvmMap_SysEntryStart;
static	u16			X_A			gNvmMap_LastFindIndex;
static	PN8			X_A			MAP_BITMAP_START;

static	u8			X_A			gNvmMap_bitMapCache[MAP_BITMAP_CACHE_PAGE_SIZE];
static	PN8			X_A			gNvmMap_bitMapCachePage;
static	u1						gNvmMap_bitMapIsCached;


#if !SE_ONLY
u1 MapTable_IsCurrentProfile(u16 entryIndex, u8 profileId)
{
	u1		isInCache;
	u8		byteValue;
	u16		byteIndex;
	PN8		bitAddress;

	isInCache						= false;
	byteIndex						= U16_S_U16(entryIndex, gNvmMap_AppStartIndex);
	bitAddress						= PN8_A_U16(MAP_BITMAP_START, byteIndex);
	if(true == gNvmMap_bitMapIsCached)
	{
		if(U32_AND_U32(PNtMEM(bitAddress), MAP_BITMAP_CACHE_PAGE_MASK) == PNtMEM(gNvmMap_bitMapCachePage))
		{
			//hit the cache
			byteIndex					= U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_INDEX_MASK);
			byteValue					= gNvmMap_bitMapCache[byteIndex];
			isInCache					= true;
		}
	}

	if(false == isInCache)
	{
		byteValue					= PN_R_U8(bitAddress);
	}

	if((profileId & 0x3F)  == (byteValue & 0x3F))
	{
		if(byteValue & (MAP_TABLE_ENTRY_STATUS_MASK << 6))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

//	bitValue  = bitValue & 0x3F;
//	return bitValue;

}

#endif
/********************************************************************************************************
 * map bit_map table structure
 *_________________________________________________________________________________________________________________________________
 *							|
 *							|
 *		map top index flag  | map bit map area start
 *							|
 *							|
 *__________________________|______________________________________________________________________________________________________
 *
 *
*********************************************************************************************************/

/**********************************************************MAP BIT_MAP********************************************************/
/*****************************************************************************************************************************/
void MapTable_SetEntryStatus(u16 entryIndex, u8 status)
{
#if SE_ONLY
	u8		oldBitValue;	
	u8		bitOffset;
#endif
	u16		byteIndex;
	PN8		newBitMapPage;
	u8		newBitValue;

	byteIndex						= U16_S_U16(entryIndex, gNvmMap_AppStartIndex);
	if(byteIndex >= U8_M_U16(MAP_BITMAP_ENTRY, MAP_BITMAP_SIZE))
	{
		bootErrorHandle_ThrowError(SYSTEMEXCEPT_NO_RESOURCE);
	}
#if SE_ONLY
	bitOffset						= U16t8(U8_M_U8(U16t8(U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_BIT_MASK)), MAP_BITMAP_CACHE_BIT_SHIFT));
	byteIndex						= U16_D_U8(byteIndex, MAP_BITMAP_CACHE_NUM_PER_BYTE);
	newBitMapPage					= MEMtPN8(U32_AND_U32(PNtMEM(PN8_A_U16(MAP_BITMAP_START, byteIndex)), MAP_BITMAP_CACHE_PAGE_MASK));
	byteIndex						= U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_INDEX_MASK);
	
	if(true == gNvmMap_bitMapIsCached)
	{
		//not hit the cache
		if(newBitMapPage != gNvmMap_bitMapCachePage)
		{
			//wirte back the old value
			nvmCopyAtomic(gNvmMap_bitMapCachePage, gNvmMap_bitMapCache, MAP_BITMAP_CACHE_PAGE_SIZE);
			memRefresh();

			//cache new value
			gNvmMap_bitMapCachePage		= newBitMapPage;
			ecpMem_Copy(gNvmMap_bitMapCache, gNvmMap_bitMapCachePage, MAP_BITMAP_CACHE_PAGE_SIZE);
		}
	}
	else
	{
		//cache new value
		gNvmMap_bitMapCachePage			= newBitMapPage;
		gNvmMap_bitMapIsCached			= true;
		ecpMem_Copy(gNvmMap_bitMapCache, newBitMapPage, MAP_BITMAP_CACHE_PAGE_SIZE);
	}

	oldBitValue						= gNvmMap_bitMapCache[byteIndex];
	oldBitValue						= U8_AND_U8(oldBitValue, U8_INV(U8_SHL_U8(MAP_BITMAP_CACHE_BIT_MASK, bitOffset)));

	status							= U8_SHL_U8(status, bitOffset);
	newBitValue						= U8_OR_U8(status, oldBitValue);
	gNvmMap_bitMapCache[byteIndex]	= newBitValue;
#else
	newBitMapPage					= MEMtPN8(U32_AND_U32(PNtMEM(PN8_A_U16(MAP_BITMAP_START, byteIndex)), MAP_BITMAP_CACHE_PAGE_MASK));
	byteIndex						= U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_INDEX_MASK);
	if(true == gNvmMap_bitMapIsCached)
	{
		//not hit the cache
		if(newBitMapPage != gNvmMap_bitMapCachePage)
		{
			//wirte back the old value
			nvmCopyAtomic(gNvmMap_bitMapCachePage, gNvmMap_bitMapCache, MAP_BITMAP_CACHE_PAGE_SIZE);
			memRefresh();

			//cache new value
			gNvmMap_bitMapCachePage		= newBitMapPage;
			ecpMem_Copy(gNvmMap_bitMapCache, gNvmMap_bitMapCachePage, MAP_BITMAP_CACHE_PAGE_SIZE);
		}
	}
	else
	{
		//cache new value
		gNvmMap_bitMapCachePage			= newBitMapPage;
		gNvmMap_bitMapIsCached			= true;
		ecpMem_Copy(gNvmMap_bitMapCache, newBitMapPage, MAP_BITMAP_CACHE_PAGE_SIZE);
	}

	if(status == MAP_TABLE_ENTRY_STATUS_CREATION)
		newBitValue 					= ((status & 0x3)<<6)|(G_CurExecutingProfileIndex & 0x3F) ;
	else
		newBitValue 					= (status & 0x3)<<6;

	gNvmMap_bitMapCache[byteIndex]	= newBitValue;
#endif
}

u8 MapTable_GetEntryStatus(u16 entryIndex)
{
	u1		isInCache;
#if SE_ONLY
	u8		bitOffset;
#endif
	u8		bitValue;
	u16		byteIndex;
	PN8		bitAddress;

	isInCache						= false;
#if SE_ONLY
	byteIndex						= U16_S_U16(entryIndex, gNvmMap_AppStartIndex);
	bitOffset						= U16t8(U8_M_U8(U16t8(U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_BIT_MASK)), MAP_BITMAP_CACHE_BIT_SHIFT));
	byteIndex						= U16_D_U8(byteIndex, MAP_BITMAP_CACHE_NUM_PER_BYTE);
	bitAddress						= PN8_A_U16(MAP_BITMAP_START, byteIndex);

	if(true == gNvmMap_bitMapIsCached)
	{
		if(U32_AND_U32(PNtMEM(bitAddress), MAP_BITMAP_CACHE_PAGE_MASK) == PNtMEM(gNvmMap_bitMapCachePage))
		{
			//hit the cache
			byteIndex					= U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_INDEX_MASK);
			bitValue					= gNvmMap_bitMapCache[byteIndex];
			isInCache					= true;
		}
	}
	
	if(false == isInCache)
	{
		bitValue					= PN_R_U8(bitAddress);
	}

	bitValue						= U8_SHR_U8(bitValue, bitOffset);
	bitValue						= U8_AND_U8(bitValue, MAP_BITMAP_CACHE_BIT_MASK);
	return bitValue;
#else
	byteIndex						= U16_S_U16(entryIndex, gNvmMap_AppStartIndex);
	bitAddress						= PN8_A_U16(MAP_BITMAP_START, byteIndex);
	if(true == gNvmMap_bitMapIsCached)
	{
		if(U32_AND_U32(PNtMEM(bitAddress), MAP_BITMAP_CACHE_PAGE_MASK) == PNtMEM(gNvmMap_bitMapCachePage))
		{
			//hit the cache
			byteIndex					= U16_AND_U16(byteIndex, MAP_BITMAP_CACHE_INDEX_MASK);
			bitValue					= gNvmMap_bitMapCache[byteIndex];
			isInCache					= true;
		}
	}

	if(false == isInCache)
	{
		bitValue					= PN_R_U8(bitAddress);
	}

	bitValue  = (bitValue>>6) & 0x3;
	return bitValue;
#endif
}
/**********************************************************MAP BIT_MAP********************************************************/
/*****************************************************************************************************************************/

/************************************************************** MAP **********************************************************/
/*****************************************************************************************************************************/
memref MapTable_GetBase(PN8 topNvm)
{
	MAP_BITMAP_START				= PN8_S_U16(topNvm, MAP_BITMAP_SIZE);
	
	return PNtMEM(MAP_BITMAP_START);
}

void MapTable_Boot(u16 appStartIndex)
{
	nvmFillNoAtomic(MEMtPN8(MAP_BITMAP_START), 0x00, MAP_BITMAP_SIZE);
	gNvmMap_TopIndex				= appStartIndex;
}

void MapTable_Init(PN8 SysEntryStart, PN8 AppEntryStart, u16 appStartIndex)
{
	if (BootLifeCycle_GetLifeCycle() >= LIFE_CYCLE_INIT)//modify by jiangxiaohui@20220616
	{
		gNvmMap_TopIndex				= FreeTable_GetMapTopIndexNvm();
	}
	else
	{
		gNvmMap_TopIndex				= appStartIndex;
	}
	gNvmMap_AppStartIndex			= appStartIndex;
	gNvmMap_LastFindIndex			= gNvmMap_AppStartIndex;
	gNvmMap_SysEntryStart			= SysEntryStart;
	gNvmMap_AppEntryStart			= AppEntryStart;
	gNvmMap_bitMapIsCached			= false;
	gNvmMap_bitMapCachePage			= 0;
}

void MapTable_ResetCache(void)
{
	MapTable_Init(gNvmMap_SysEntryStart, gNvmMap_AppEntryStart, gNvmMap_AppStartIndex);
}
/*
 * modify by jiangxiaohui@20210429  addReference中分配的引用没有提交，会被重复使用
 */
void MapTable_Commit(void)
{
	if(true == gNvmMap_bitMapIsCached)
	{
		gNvmMap_bitMapIsCached			= false;
		nvmCopyAtomic(gNvmMap_bitMapCachePage, gNvmMap_bitMapCache, MAP_BITMAP_CACHE_PAGE_SIZE);
		if(false == NvmManager_isAlloOrDeallo())
		{
			FreeTable_SetMapTopIndexNvm(MapTable_GetTopIndex());
		}
	}
}

void MapTable_Reset(void)
{
	u16		i;
	
	u16  freeTableState;

	freeTableState	= FreeTable_GetState();

	if(true == FreeTable_CheckIsCompact(FreeTable_GetOpType(freeTableState)))
	{
		i	= U16_S_U8(gNvmMap_TopIndex, 1);

		for(; i > gNvmMap_AppStartIndex; U16_DEC(i))
		{
			if(MAP_TABLE_ENTRY_STATUS_AVAIL != MapTable_GetEntryStatus(i))
			{
				break;
			}
		}

		gNvmMap_TopIndex		= U32t16(U16_A_U8(i, 1));
		gNvmMap_LastFindIndex	= gNvmMap_AppStartIndex;
	}
}
// modify by jiangxiaohui@20220525: 代码分层
memref MapTable_GetEndAddr(void)
{
	memref alignaddr;
#if JAVA_CARD_UICC
	alignaddr =  U32_AND_U32(U32_A_U16((memref)MASK_MAP_START,U8_M_U16(U8_A_U8(sysConfig_GetMaptable_num(),0x01),EEPAGE_SIZE)),NOR_EEPAGE_MASK);
#else
	alignaddr = FreeTable_GetTopFreeNvm();
#endif
	return alignaddr;
}



//TODO mwp: 写测试用例时，查找MAP_index 要考虑缓存的引起的问题。
u16 MapTable_GetAvailableIndex(void)
{
	u1		isFind;
	u16		i;
	
	isFind			= false;

	if(true == isInProgress()) //TODO mwp: 在这里isInProgress() 一直是为"true", i 永远  == g_mapLastFindIndex，因此，中间的Map_index 项用不上。
	{
		i			= gNvmMap_LastFindIndex;
	}
	else
	{
		i			= gNvmMap_AppStartIndex;
	}

	for(; i < gNvmMap_TopIndex; U16_INC(i))
	{
		if(MAP_TABLE_ENTRY_STATUS_AVAIL == MapTable_GetEntryStatus(i))
		{
			isFind		= true;
			break;
		}
	}

	if(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, U32t16(U16_A_U16(i, 1)))) <= MEMtPN8(MapTable_GetEndAddr())
		&&(i -gNvmMap_AppStartIndex)<U8_M_U16(MAP_BITMAP_ENTRY, MAP_BITMAP_SIZE))
	{
		if(false == isFind)
		{
			U16_INC(gNvmMap_TopIndex); //TODO mwp: g_mapTopIndex 要做边界测试。
		}

		gNvmMap_LastFindIndex	= i; //TODO mwp: g_mapLastFindIndex 这个全局变量取值有问题，要写更多用例。
	}
	else
	{
	   i = IF_GetAvailableMapIndex(i,&gNvmMap_AppStartIndex,&gNvmMap_TopIndex,&gNvmMap_LastFindIndex,isFind);
	}

	return i;
}
/*
 *modify by xuxiaojun@20220328 分层
 */
memref MapTable_GetMapTopAddress(void)
{
#if JAVA_CARD_UICC
	return IF_GetFileSystemEndAddr();
#else
	return PNtMEM(PN8_A_U16(gNvmMap_AppEntryStart, U16_M_U16(gNvmMap_TopIndex, MAP_ENTRY_SIZE)));
#endif	
}

u16 MapTable_GetTopIndex(void)
{
	return gNvmMap_TopIndex;
}

memref MapTable_GetAppMap(void)
{
	return PNtMEM(gNvmMap_AppEntryStart);
}

memref MapTable_GetAppBitMap(void)
{
	return PNtMEM(MAP_BITMAP_START);
}

u16 MapTable_GetAppStartIndex(void)
{
	return gNvmMap_AppStartIndex;
}

u1 MapTable_IsCreationObject(u16 objectId)
{
	u1 	result;
	u8	pckId;
	
	result	= false;
	pckId	= MapTable_GetEntryPckId(objectId);
	
	if(MAP_ENTRY_OBJECT_PCK_ID == pckId)
	{
		if(objectId > gNvmMap_AppStartIndex)
		{
			if(MAP_TABLE_ENTRY_STATUS_CREATION == MapTable_GetEntryStatus(objectId))
			{
				result 	= true;
			}
		}
		else
		{
			result 	= true;
		}
	}
	
	return result;
}

//TODO mwp: pckId 有问题， pckId = 0x80: Object, pckId = 0xC0: block, 其它为refence 所在的pkgId； 当refence 所在的pkgId = 0x80 或 0xC0 就会出问题。
u16 MapTable_AddEntry(PN8 address, u8 pckId)
{
	u16		availIndex;
	
	availIndex			= MapTable_GetAvailableIndex();
	
	if(MAP_TABLE_ILLEGAL_INDEX != availIndex)
	{
		MapTable_SetEntryInfo(availIndex, MEMtPN8(address), pckId);
		MapTable_SetEntryStatus(availIndex, MAP_TABLE_ENTRY_STATUS_CREATION);
	}

	if((MAP_ENTRY_OBJECT_PCK_ID == pckId) || (MAP_ENTRY_BLOCK_PCK_ID == pckId))
	{
		memRefresh();
	}
	
	return availIndex;
}
/*@ -discast @*/
memref MapTable_GetObjAddr(u16 objectId)
{
	MapEntry_t	mapEntry;
	u8			objectPckId;

	if(objectId >= gNvmMap_AppStartIndex)
	{
        if(objectId >= gNvmMap_TopIndex)
    	{
        	bootErrorHandle_ThrowError(MAP_TABLE_ILLEGAL_OBJECT_EXCEPTION);
    	}
		mapEntry 		= (MapEntry_t)(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, objectId)));
	}
 	else
	{
		mapEntry		= (MapEntry_t)PN8_A_U32(gNvmMap_SysEntryStart, U8_M_U16(MAP_ENTRY_SIZE, objectId));
	}

	if(REF_NULL != objectId)
	{
		objectPckId		= MapTable_GetEntryInfoPckId(mapEntry);

		if(MAP_ENTRY_OBJECT_PCK_ID != objectPckId)
		{
			bootErrorHandle_ThrowError(MAP_TABLE_ILLEGAL_OBJECT_EXCEPTION);
		}
	}

	return MapTable_GetEntryInfoAddr(mapEntry);
}

memref MapTable_GetEntityContent(u16 entryIndex)
{
	MapEntry_t	mapEntry;
	memref		entityAddr;

	if(entryIndex >= gNvmMap_AppStartIndex)
	{
		mapEntry 	= (MapEntry_t)(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex)));
		entityAddr	= MapTable_GetEntryInfoAddr(mapEntry);
		entityAddr	= U32_A_U8(entityAddr, MAP_ENTRY_ENTITY_HEAER_SIZE);
	}
 	else
	{
		mapEntry 	= (MapEntry_t)PN8_A_U32(gNvmMap_SysEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex));
		entityAddr	= MapTable_GetEntryInfoAddr(mapEntry);
	}

	return entityAddr;
}

u32 MapTable_GetEntitySize(PN8 blockAddr)
{
	u32 size;

	if(MAP_ENTRY_ENTITY_TYPE == PN_R_U8(blockAddr))
	{
		size	= PN_R_U16(PN8_A_CONST(blockAddr, MAP_ENTRY_ENTITY_LEN_OFFSET));
		size	= U8_M_U32(MAP_ENTRY_ENTITY_ALIGN_SIZE, size);
	}
	else
	{
		/*@ -type @*/size	= getObjSize(blockAddr);/*@ +type @*/
		
		//4 bytes align
		if(0 != U32_R_U8(size, MAP_ENTRY_ENTITY_ALIGN_SIZE))
		{
			size = U32_A_U8(size, U8_S_U8(MAP_ENTRY_ENTITY_ALIGN_SIZE, U32_R_U8(size, MAP_ENTRY_ENTITY_ALIGN_SIZE)));
		}
	}

	return size;
}

memref MapTable_GetEntryAddr(u16 entryIndex)
{
	MapEntry_t	mapEntry;

	if(entryIndex >= gNvmMap_AppStartIndex)
	{
		mapEntry 	= (MapEntry_t)(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex)));
	}
 	else
	{
		mapEntry 	= (MapEntry_t)PN8_A_U32(gNvmMap_SysEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex));
	}

	return MapTable_GetEntryInfoAddr(mapEntry);
}

u8 MapTable_GetEntryPckId(u16 entryIndex)
{
	MapEntry_t	mapEntry;

	if(entryIndex >= gNvmMap_AppStartIndex)
	{
		mapEntry 	= (MapEntry_t)(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex)));
	}
 	else
	{
		mapEntry 	= (MapEntry_t)PN8_A_U32(gNvmMap_SysEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex));
	}

	return MapTable_GetEntryInfoPckId(mapEntry);
}

void MapTable_SetEntryAddr(u16 entryIndex, PN8 blockStart)
{
	u8 buffer[MAP_ENTRY_SIZE];
	u8 pckId	= MapTable_GetEntryInfoPckId((MapEntry_t)(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex))));

#if RUN_IN_SIMULATOR & BIG_ENDIAN
	PXS_W_U32(MEMtPX32(buffer), htoj32(PNtMEM(blockStart)));
#else
	PXS_W_U32(MEMtPX32(buffer), (PNtMEM(blockStart)));
#endif

#if MAP_ENTRY_PKGID_FIRST
	buffer[0] = pckId;
#else
	buffer[3] = pckId;
#endif
	nvmCopyAtomic(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex)), MEMtPM8(buffer), MAP_ENTRY_SIZE);
}

void MapTable_SetEntryInfo(u16 entryIndex, PN8 blockStart, u8 pckId)
{
	u8 buffer[MAP_ENTRY_SIZE];

#if RUN_IN_SIMULATOR & BIG_ENDIAN
	PXS_W_U32(MEMtPX32(buffer), htoj32(PNtMEM(blockStart)));
#else
	PXS_W_U32(MEMtPX32(buffer), (PNtMEM(blockStart)));
#endif

#if MAP_ENTRY_PKGID_FIRST
	buffer[0] = pckId;
#else
	buffer[3] = pckId;
#endif	
	nvmCopyLost(PN8_A_U32(gNvmMap_AppEntryStart, U8_M_U16(MAP_ENTRY_SIZE, entryIndex)), MEMtPM8(buffer), MAP_ENTRY_SIZE);
}
/************************************************************** MAP **********************************************************/
/*****************************************************************************************************************************/
/*@ +discast @*/
#endif
