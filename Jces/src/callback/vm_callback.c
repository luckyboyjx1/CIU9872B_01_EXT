/*
 * boot_callback.c
 *
 *  Created on: 2016年4月10日
 *      Author: huangzhigao
 */

#include <vm_callback.h>
#include "mask.h"
#include "target.h"
#include "JcvmDataConfig.h"
#include "packageloc.h"
//#include "MemOperationServe.h"
#include "ob_manager.h"
#include "chipDataConfig.h"
#include "hal_callback.h"
#include "ecpDefine.h"
#include "chipHead.h"
#include "CreateJavaBuffer.h"
#include "NvmAllocate.h"
#include "object.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


void halInitJavaBuffer(void)
{
#define RTR_TOP RAM_LIMIT
	DtTrRamAlign						= &RAMSTACK[0];
	APDUBuffer							= &RAMSTACK[DT_RT_RAM_ALIGN_DWORD];
	cacheBuffer							= &RAMSTACK[DT_RT_RAM_ALIGN_DWORD+APDU_BUFFER_SIZE];
	bRamBuffer							= &RAMSTACK[DT_RT_RAM_ALIGN_DWORD+APDU_BUFFER_SIZE+CACHE_BUFFER_SIZE];
	bShortBuffer						= &RAMSTACK[DT_RT_RAM_ALIGN_DWORD+APDU_BUFFER_SIZE+CACHE_BUFFER_SIZE+RAM_BUFFER_SIZE];

//	g_PBOC_instRAMBuff					= &RAMSTACK[DT_RT_RAM_ALIGN_DWORD + (u16)RAMBase];
//	javaExceStaticFeild 				= &RAMSTACK[RTR_TOP];
}


void romInit()
{
	//ROM mask
	memcpy(&ROMSTACK[ROM_MASK_OFFSET], rommask, ROMSIZE);
	memcpy(&ROMSTACK[ROM_MASK_OFFSET + ROMSIZE], mapTableAlign, 4 -  (ROMSIZE & 0x03));
	memcpy(&ROMSTACK[ROM_MASK_OFFSET + ROMSIZE + 4 -  (ROMSIZE & 0x03)], staticinit, EEPROM_IMAGE_SIZE);
}

void eepConfigInit()
{
	//eep Config data
	memcpy(&EEPROMSTACK[0], sysConfig, CHIP_RFU);
	memcpy(&EEPROMSTACK[CHIP_RFU], chipDataConfig, VM_OS_CONF - CHIP_RFU);
	memcpy(&EEPROMSTACK[VM_OS_CONF], vm_config, CARDINIT_CONFIG_BASE -VM_OS_CONF);
}

void ramReset()
{
	memset(RAMSTACK, 0,  CHIP_RAM_LIMIT-CHIP_RAM_BASE); // 解决编译问题, 将JCVM_RAM_LIMIT替换为CHIP_RAM_LIMIT，JCVM_RAM_BASE为CHIP_RAM_BASE
}


void initVmCallback(){
	initBootCallback();
	VM_NVM_alloc = NvmManager_Allocate;
	VM_MapTable_GetEntityContent = MapTable_GetEntityContent;
}

u32 vm_PACKAGE_LOCATION_TABLE_ADDRESS(){ return G_CurExecuEnvironment.pkgLocationTable;}
memref vm_GET_ARRAY_DATA(PF8 array_ref){return get_array_data(array_ref);}
u16 vm_GET_ARRAY_LENGTH(PF8 array_ref){ return get_array_length(array_ref);}
memref vm_GET_OBJECT_FIELD_REF(PM8 objref, u16 token){ return get_object_field_ref(objref, token);}

u8  vm_MEMREADU8(PM8 addr){return memReadU8(addr);}
u16 vm_MEMREADU16(PM8 addr){return memReadU16(addr);}
u32 vm_MEMREADU32(PM8 addr){return memReadU32(addr);}
u8  vm_NVMREADU8(PN8 addr){return nvmReadU8(addr);}
u16 vm_NVMREADU16(PN8 addr){return nvmReadU16(addr);}
u32 vm_NVMREADU32(PN8 addr){return nvmReadU32(addr);}


void  vm_MEMCOPYATOMIC(PM8 dest, PM8 src, u16 length){memCopyAtomic(dest, src, length);}
memref 	vm_GETPACKAGECOMPONENTLOCATION(u8 pkgID, u8 compID){return getPackageComponentLocation(pkgID, compID);}

u16 	vm_JCVMCONFIG_GETMAXENTRYCOUNTINMAP(void){
//	u16		I_A		max_entry_count;

//#if VM_EEP_ALLOC_CACHE
//	if (U8_AND_U8(systemCache.Flag, MAPENTRY_FLAG))
//	{
//		max_entry_count = systemCache.map_entry_count;
//	}
//	else
//	{
//		max_entry_count = JcvmConfig_GetMaxEntryCountInMap();
//	}
//
//#else
//	max_entry_count = JcvmConfig_GetMaxEntryCountInMap();
//#endif
	return MapTable_GetTopIndex();
//	return max_entry_count;

}

memref vm_LOCATEOBJECTINMEMORY(jref objectId){return locateObjectInMemory(objectId);}
memref vm_GETENTITYCONTENT(jref index){	return MapTable_GetEntityContent(index);
}

#if VM_WITH_DEBUG
#include "debugMask.h"
u32 vm_DEBUG_COMPONENT_PKG_COUNT(){ return DEBUG_COMPONENT_PKG_COUNT;}
PM8 vm_GETDEBUGMASK(){ return debugMask;}

#endif
