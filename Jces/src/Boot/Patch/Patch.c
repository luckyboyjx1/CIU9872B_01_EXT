#include "bootConfigProfile.h"
#include "PatchHal.h"
#include "Apdu.h"
#include "SysConfig.h"
#include "MemOperation.h"
#include "Patch.h"

#if !RUN_IN_SIMULATOR
#include "global_variable.h"
#endif

#if HAL_PATCH_NATIVE
u8	patch_inrom;
/****************************************************************************************************************************/
u16 patch_download(PX8 apdubuffer)		SMALL_MODE
{
    PN8     D_A  patchAreaPtr;
	u16		D_A	 result;
	u16				tempData1;
	u16				tempData2; 

	patch_BackupOperation((PX8)&tempData1,(PX8)&tempData2);
	apdu_ApduSetIncomingAndReceive();

	patchAreaPtr			= MEMtPN8(sysConfig_GetPatchAreaBase());
	result					= PATCH_SECURITY_STATUS_NOT_SATISFIED;

	switch (apdubuffer[APDU_P2_OFFSET])
	{
        case 0x00: //start loading
            result			= patch_init(apdubuffer, patchAreaPtr);
            break;
        case 0x01: //downloading
            result			= patch_load(apdubuffer, patchAreaPtr);
            break;
        case 0x02: //install and register
            result			= patch_install(apdubuffer, patchAreaPtr);
            break;
        case 0x82: //install and register
            result			= patch_install_InRom(apdubuffer, patchAreaPtr);
            break;
        case 0x03: //check
            result			= patch_check(apdubuffer, patchAreaPtr);
            break;
//		case 0x04://modify patch table entry address
//			result			= patch_modify_default(apdubuffer,patchAreaPtr);
//			break;
//		case 0x05://modify patch table entry address
//			result			= patch_alloc_EEPROM(apdubuffer,patchAreaPtr);
//			break;
//		case 0x06://modify patch table entry address
//			result			= patch_alloc_DTR_RTR(apdubuffer,patchAreaPtr);
//			break;
        default:
        	result			= PATCH_SECURITY_STATUS_NOT_SATISFIED;
	}
	patch_RecoveryOperation(tempData1,tempData2);
	return result;
}



#endif
