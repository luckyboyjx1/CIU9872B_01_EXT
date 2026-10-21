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

memref resolveVirtualMethod(PF8 objref, u8 method_token)		SMALL_MODE
{
	jref		D_A		classRef;
	PF8			D_A		classAddr;
	u8			D_A		classPkg;
	PF8					methodAddr;
	jref				methodRef;
	u8					pkg_mtb;
	u8					pk_mtc;
	u16					offset;
	u8 					pub_mtb;
	u8 					pub_mtc;
	PATCH_JCVM_VM_METHOD_001();
	classAddr			= MEMtPF8(get_object_class(objref));
	classPkg			= get_object_class_pkg(objref);

	/* Is this a package-private method? */
	if (U8_AND_U8(method_token, PACKAGE_VMT) == PACKAGE_VMT)
	{
		/* search through the package method tables of this class or its
		 super-classes */
		method_token		= U8_AND_U8(method_token, ~PACKAGE_VMT);
		while (true)
		{
			pkg_mtb			= PF_R_U8(PF8_A_CONST(classAddr, CR_PackageMethods));
			pk_mtc			= PF_R_U8(PF8_A_CONST(classAddr, (CR_PackageMethods + MTH_NumMethods)));
			if ((method_token < pkg_mtb) || pk_mtc == 0 || (U8_A_U8(pkg_mtb, pk_mtc) <= method_token))
			{
				/* couldn't find method_token in this class, go to super-class */
				classRef = get_super_class(classAddr);
#if JCVM__VM__NOT_TRUSTED
				if (classRef == TERMINAL_CLASS_REF)
				{
					fatal_error(RESOLVE_VIRTUAL_METHOD_ERROR);
				}
#endif
				classAddr	= MEMtPF8(resolveReferenceAddress(classRef, classPkg, COMPONENT_CLASS));
				classPkg	= lastVMPkg;
			}
			else
			{
				/* success! get and return method address */
				offset		= U8t16(PF_R_U8(PF8_A_CONST(classAddr, (CR_PublicMethods + MTH_NumMethods))));
				offset		= U8_M_U16(SIZE_REF, U8_A_U16(U8_S_U8(method_token, pkg_mtb), offset));
				methodRef	= PF_R_U16(PF8_A_U16(classAddr, U8_A_U16(CR_MethodsTableStart, offset)));

				methodAddr	= MEMtPF8(resolveReferenceAddress(methodRef, classPkg, COMPONENT_METHOD));
				return PFtMEM(methodAddr);
			}
		}
	}
	else
	{
		while (true)
		{
			/* this is a token of public method */
			pub_mtb			= PF_R_U8(PF8_A_CONST(classAddr, (CR_PublicMethods + MTH_MethodTableBase)));
			pub_mtc			= PF_R_U8(PF8_A_CONST(classAddr, (CR_PublicMethods + MTH_NumMethods)));
			if ((method_token < pub_mtb) || (U8_A_U8(pub_mtb, pub_mtc) <= method_token))
			{
				/* method token not found in this class, go to super class */
				classRef = get_super_class(classAddr);
#if JCVM__VM__NOT_TRUSTED
				if (classRef == TERMINAL_CLASS_REF)
				{
					fatal_error(RESOLVE_VIRTUAL_METHOD_ERROR);
				}
#endif
				classAddr	= MEMtPF8(resolveReferenceAddress(classRef, classPkg, COMPONENT_CLASS));
				classPkg	= lastVMPkg;
			}
			else
			{
				/* success. Get and return method address */
				offset		= U8_M_U8(U8_S_U8(method_token, pub_mtb), SIZE_REF);
				methodRef	= PF_R_U16(PF8_A_U16(classAddr, U8_A_U16(CR_MethodsTableStart, offset)));
				methodAddr	= MEMtPF8(resolveReferenceAddress(methodRef, classPkg, COMPONENT_METHOD));
				
				if (U16_AND_U16(methodRef, 0x8000) == 0)
				{		
					u16		D_A		map_index;
					PF8		I_A		map_addr;
					// removed by WH 2007.7.4 no used , this can be done in the founction getPackageComponentLocation
					//but do like this must be faster
					/* Performance: Do not lookup on packageloc table if in current package */
					
					lastVMPkg = classPkg;
					if (classPkg == PCpkgID)
					{
						return U16_A_U32(methodRef, cachedMethodCompLoc);
					}
					/* Reference is internal - resolve using package table */
					//TODO:check asm code RECHECK U8 FAR * OK
			//		return (loadAddress_NOCache((u8 far *)loadAddress_NOCache((u8 far *)(PACKAGE_LOCATION_TABLE_ADDRESS	+ pkgID * SIZE_MEMREF)) + (memref)(comp_type * (SIZE_MEMREF + SIZE_U16))) + referenceID);
					map_index	= PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(classPkg, SIZE_MEMREF)));
					map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
					map_addr	= PF8_A_U16(map_addr, U8_M_U8(COMPONENT_METHOD, (SIZE_MEMREF + SIZE_U16)));
					//map_index	= U32t16(PF_R_U32(map_addr));
					if (PF_R_U16(PF8_A_U8(map_addr , SIZE_U32)) >= methodRef)
					{
						map_index	= PF_R_U16(map_addr + SIZE_U16);
						map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));

						map_addr 	= PF8_A_U16(map_addr, methodRef);
						return PFtMEM(map_addr);
//						map_addr	= MEMtPF8(MapTable_GetEntityContent(map_index));
//						map_addr 	= PF8_A_U16(map_addr, methodRef);
//						return PFtMEM(map_addr);
					}
					faultDetected();
				}
				lastVMPkg	= getReferencePackageID(methodRef);
				/* Reference is external - resolve using global reference table */
				return (getReferenceAddress(methodRef));
			}
		}
	}
}

#endif
