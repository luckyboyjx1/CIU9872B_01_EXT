#ifndef    __PATCH_H__
#define    __PATCH_H__

#include "DataOperation.h"
#include "bootConfigProfile.h"

#if HAL__PATCH
#if HAL_PATCH_NATIVE
	#define STATUS_INIT     						0
	#define STATUS_READY    						1
	#define STATUS_FINISHED    						2
	#define STATUS_ENABLE    						3

	#define PATCH_WRONG_LENGTH					    0x6700
	#define PATCH_CONDITIONS_NOT_SATISFIED	        0x6985
	#define PATCH_SECURITY_STATUS_NOT_SATISFIED     0x6982
	#define PATCH_MEMORY_NOT_ENOUGH				    0x6A84
	#define PATCH_NO_ERROR						    0x9000

	extern u16 patch_download(PX8 apdubuffer)						SMALL_MODE;
	extern u16 patch_init(PX8 apdubuffer, PN8 PatchAreaPtr);
	extern u16 patch_load(PX8 apdubuffer, PN8 PatchAreaPtr);
	extern u16 patch_install(PX8 apdubuffer, PN8 PatchAreaPtr);
	extern u16 patch_install_InRom(PX8 apdubuffer, PN8 patchAreaPtr);
	extern u16 patch_check(PX8 apdubuffer, PN8 PatchAreaPtr);
	extern u16 patch_modify_default(PX8 apdubuffer, PN8 PatchAreaPtr);
	extern	u16 patch_alloc_EEPROM(PX8 apdubuffer, PN8 PatchAreaPtr);
	extern	u16 patch_alloc_DTR_RTR(PX8 apdubuffer, PN8 PatchAreaPtr);
	#define patch_BackupOperation(tempData1, tempData2)			ecpPatch_BackupOperation(tempData1,tempData2)
	#define patch_RecoveryOperation(tempData1, tempData2)		ecpPatch_RecoveryOperation(tempData1,tempData2)
#endif
	extern void patch_clear(PN8 patchAreaPtr);
	#define patch_initBeforAts() 			ecpPatch_initBeforAts()
	#define patch_initAfterAts() 			ecpPatch_initAfterAts()
	#define patch_reInit() 					patch_clear(MEMtPN8(sysConfig_GetPatchAreaBase()))
#else

	#define patch_download(apdubuffer)								0
	#define patch_initBeforAts()
	#define patch_initAfterAts()
	#define patch_init(apdubuffer, PatchAreaPtr)					0
	#define patch_load(apdubuffer, PatchAreaPtr)					0
	#define patch_install(apdubuffer, PatchAreaPtr)					0
	#define patch_install_InRom(apdubuffer, PatchAreaPtr)			0
	#define patch_check(apdubuffer, PatchAreaPtr)					0
	#define	patch_reInit()
	#define patch_modify_default(apdubuffer, PatchAreaPtr)			0
	#define patch_alloc_EEPROM										0
	#define patch_alloc_DTR_RTR										0
	#define	patch_BackupOperation(tempData1,tempData2)				0
	#define	patch_RecoveryOperation(tempData1,tempData2)			0
#endif

#endif  //__PATCH_H__
