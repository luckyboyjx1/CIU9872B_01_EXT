#ifndef   __RAM_MANAGER_H__
#define   __RAM_MANAGER_H__

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "target.h"
#include "global_variable.h"

#define	RamManager_getAlloRamBase(ramAddr)											PX8_A_U16(PHY_RAM_BASE, ramAddr)

#if BOOT__MEMORY__RAM_MANAGER
	#define	RamManager_setAllocBlockHeaderStart(header, start)						PX_W_U32(RamManager_getAlloRamBase(U32t16(U16_A_U8(header, ALLOC_BLOCK_HEADER_START_OFFSET))), start)
	#define	RamManager_setAllocBlockHeaderLen(header, len)							PX_W_U16(RamManager_getAlloRamBase(U32t16(U16_A_U8(header, ALLOC_BLOCK_HEADER_LEN_OFFSET))), len)
	#define	RamManager_setAllocBlockHeaderIsAutoRelease(header, isAutoRelease)		PX_W_U8(RamManager_getAlloRamBase(U32t16(U16_A_U8(header, ALLOC_BLOCK_HEADER_AUTO_RELEASE_OFFSET))), isAutoRelease)

	#define	RamManager_getAllocBlockHeaderStart(header)								PX_R_U32(RamManager_getAlloRamBase(U32t16(U16_A_U8(header, ALLOC_BLOCK_HEADER_START_OFFSET))))
	#define	RamManager_getAllocBlockHeaderLen(header)								PX_R_U16(RamManager_getAlloRamBase(U32t16(U16_A_U8(header, ALLOC_BLOCK_HEADER_LEN_OFFSET))))
	#define	RamManager_getAllocBlockHeaderIsAutoRelease(header)						PX_R_U8(RamManager_getAlloRamBase(U32t16(U16_A_U8(header, ALLOC_BLOCK_HEADER_AUTO_RELEASE_OFFSET))))

	#define ALLOC_BLOCK_HEADER_START_SIZE											4
	#define ALLOC_BLOCK_HEADER_LEN_SIZE												2
	#define ALLOC_BLOCK_HEADER_AUTO_RELEASE_SIZE									2

	#define ALLOC_BLOCK_HEADER_START_OFFSET											0
	#define ALLOC_BLOCK_HEADER_LEN_OFFSET											(ALLOC_BLOCK_HEADER_START_OFFSET + ALLOC_BLOCK_HEADER_START_SIZE)
	#define ALLOC_BLOCK_HEADER_AUTO_RELEASE_OFFSET									(ALLOC_BLOCK_HEADER_LEN_OFFSET + ALLOC_BLOCK_HEADER_LEN_SIZE)
	#define ALLOC_BLOCK_HEADER_SIZE													(ALLOC_BLOCK_HEADER_START_SIZE + ALLOC_BLOCK_HEADER_LEN_SIZE + ALLOC_BLOCK_HEADER_AUTO_RELEASE_SIZE)

	#define	INVALID_ALLOC_RAM_ADDRESS												0x00
	#define	ALLOC_RAM_OFFSET_MASK													(u32)0x0000FFFF

	#define Transient_SIZE 															(RAMSize - (MAX_TEMPORARY_OBJECTS * TEMP_OBJECT_HEADER_LENGTH))
	#define ALLOC_RAM_NOT_AVAILABLE													U32t16(0x9002)
	#define ALLOC_RAM_REF_NOT_FOUND													U32t16(0x9003)
	#define ALLOC_RAM_REF_HAS_BEEN_ALLOCATED										U32t16(0x9004)
#if JCVM__RAM__DTR__RTR__NEW	
	#define ALLOC_RAM_SYSTEM_ERROR													U32t16(0x9005)
	#define ALLOC_RAM_FLAG_DTR														0xA5
	#define ALLOC_RAM_FLAG_RTR														0x5A
#endif

	#define BOOT_RAM_BASE															U32_S_U32(RAMBase, PHY_RAM_BASE)
	extern u16 RamManager_getUsedDtrSize(void);
	extern void   RamManager_initVal(void);
	extern void   RamManager_init(void);
	extern memref RamManager_getRamBase(void);
	extern u16 	  RamManager_refHasBeenAllocated(PX8 ref);
	extern u16 	  RamManager_ramAvailable(void);
	extern u16	  RamManager_alloc(PX8 ref, u16 size, u8 isAutoRelease);
	extern u16	  RamManager_reAlloc(PX8 ref, u16 size);
	extern void   RamManager_free(PX8 ref);
	extern void   RamManager_autoReleaseRam(void);
	extern void   RamManager_updatePointer(u16 tempPointer);
	extern u16    RamManager_ramOutAvailable(void);
#else		
	#define RAM_MANAGER_NOT_SUPPORTED												(u16)0x9001
	#define RamManager_initVal()
	#define RamManager_init()
	#define RamManager_getRamBase()													0
	#define RamManager_refHasBeenAllocated(ref)
	#define RamManager_ramAvailable()												0
	#define RamManager_alloc(ref, size, isAutoRelease)								//bootErrorHandle_ThrowError(RAM_MANAGER_NOT_SUPPORTED)
	#define RamManager_free(ref)													//bootErrorHandle_ThrowError(RAM_MANAGER_NOT_SUPPORTED)
	#define RamManager_reAlloc(ref, size)											//bootErrorHandle_ThrowError(RAM_MANAGER_NOT_SUPPORTED)
	#define RamManager_autoReleaseRam()												//bootErrorHandle_ThrowError(RAM_MANAGER_NOT_SUPPORTED)
	#define RamManager_ramOutAvailable()											0
#endif

#endif
