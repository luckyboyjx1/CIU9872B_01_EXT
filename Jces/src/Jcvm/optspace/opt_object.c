#include "object.h"
#include "stack.h"
#include "memory.h"
#include "context.h"
#include "mask.h"
#include "anomaly.h"
#include "context.h"
#include "chn_manager.h"
#include "packageloc.h"
#include "reference.h"
#include "global_variable.h"
#include "RamManager.h"
#include "optspace.h"
#include "error.h"
#include "bc_control.h"
#include "utilMethod.h"

#if EMV_OPTIMIZATION_CODE

memref get_object_class_impl(PF8 objAddr)
{
	//TODO:reduce u32 cal NO
	jref		I_A		classRecRef;
	u8			I_A		classRecPkg;
	u16		D_A		map_index;
	PF8		I_A		map_addr;

							   	
	classRecRef	= PF_R_U16(PF8_A_CONST(objAddr, OBJ_HDR_ClassRecordREF));
	classRecPkg	= PF_R_U8(PF8_A_CONST(objAddr, OBJ_HDR_ClassRecordPKG));

	
	if (U16_AND_U16(classRecRef, 0x8000) == 0)
	{
		// removed by WH 2007.7.4 no used , this can be done in the founction getPackageComponentLocation
		//but do like this must be faster
		/* Performance: Do not lookup on packageloc table if in current package */
		
		lastVMPkg = classRecPkg;
		if (classRecPkg == PCpkgID)
		{
			return U16_A_U32(classRecRef, cachedClassCompLoc);
		}
		/* Reference is internal - resolve using package table */
		//TODO:check asm code RECHECK U8 FAR * OK
//		return (loadAddress_NOCache((u8 far *)loadAddress_NOCache((u8 far *)(PACKAGE_LOCATION_TABLE_ADDRESS	+ pkgID * SIZE_MEMREF)) + (memref)(comp_type * (SIZE_MEMREF + SIZE_U16))) + referenceID);
		map_index	= PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(classRecPkg, SIZE_MEMREF)));
		map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
		map_addr	= PF8_A_U16(map_addr, U8_M_U8(COMPONENT_CLASS, (SIZE_MEMREF + SIZE_U16)));
//		map_index	= U32t16(PF_R_U32(map_addr));
		if (PF_R_U16(PF8_A_U8(map_addr , SIZE_U32)) >= classRecRef)
		{
//			map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
//			map_addr 	= PF8_A_U16(map_addr, classRecRef);
//			return PFtMEM(map_addr);
			
			map_index	= PF_R_U16(map_addr + SIZE_U16);
			map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));

			map_addr 	= PF8_A_U16(map_addr, classRecRef);
			return PFtMEM(map_addr);
			
		}
		faultDetected();
	}
	/* Reference is external - resolve using global reference table */
	return (getReferenceAddress(classRecRef));
}

#endif
