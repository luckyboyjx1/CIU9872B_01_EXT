/*****************************************************************************
 * File:  method.c
 * Version 1.7
 *
 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/03 at 13:23:48
 00/12/04 at 13:23:48 .Created
 * Original authors: Alexandre Frey, Nicolas Regnault

 ****************************************************************************/
#include "method.h"
#include "error.h"
#include "context.h"
#include "interpreter.h"
#include "object.h"
#include "memory.h"
#include "anomaly.h"
#include "packageloc.h"
#include "reference.h"
#include "loader.h"
#include "global_variable.h"
#include "SEEUICCInterface.h"

#include "mask.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

u8 getMethodParamSize(PF8 methodAddr)
{
	u8		D_A		tmp[4];
    u8  tempParamSize;

	PD_W_U32(tmp, PF_R_U32(methodAddr));
	if (U8_AND_U8(tmp[0], ACC_EXTENDED) == ACC_EXTENDED)
	{
		// extended header
		tempParamSize = tmp[2];
	}
	else
	{
		// short header
		tempParamSize = U8_SHR_U8(U8_AND_U8(tmp[1], PARAMS_NIBBLE), NIBBLE_SHIFT);
	}
	return tempParamSize;
}

#if !EMV_OPTIMIZATION_CODE
WEAK memref resolveVirtualMethod(PF8 objref, u8 method_token)		SMALL_MODE
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
				lastVMPkg	= resolveReferencePackage(methodRef, classPkg);
				return PFtMEM(methodAddr);
			}
		}
	}
}
#endif //!EMV_OPTIMIZATION_CODE

memref resolveInterfaceMethod(PF8 objref, PF8 interface_addr, u8 method_intf_token)	SMALL_MODE
{
	jref		D_A		classRef;
	PF8			D_A		classAddr;
	u8			D_A		classPkg;
	PF8			D_A		intf_table_addr;
	u8			D_A		method_token;
	u8			D_A		numInterfaces;
	u8			D_A		i;
	u8			D_A		matchFound;
	jref		D_A		if_ref;
	PF8			D_A		if_ref_addr;
	
	PATCH_JCVM_VM_METHOD_002();
	
	matchFound	= 0;
	classAddr	= MEMtPF8(get_object_class(objref));
	classPkg	= get_object_class_pkg(objref);

	while (!matchFound)
	{
		numInterfaces = U8_AND_U8(PF_R_U8(PF8_A_CONST(classAddr, CR_Header)), CRH_InterfaceCount);


		/* Calculate the Class's initial Interface index table Address */
		intf_table_addr	= MEMtPF8(U8_M_U8(sizeof(jref), PF_R_U8(PF8_A_CONST(classAddr, (CR_PublicMethods + MTH_NumMethods)))));
		intf_table_addr	= PF8_A_U16(intf_table_addr, U8_M_U8(sizeof(jref), PF_R_U8(PF8_A_CONST(classAddr, (CR_PackageMethods + MTH_NumMethods)))));
		intf_table_addr = PF8_A_U32(PF8_A_CONST(classAddr, CR_MethodsTableStart), intf_table_addr); 


		/* match up one of the Class's Interface entries with the Interface Class Address */
		for (i = 0; i < numInterfaces; U8_INC(i))
		{
			if_ref		= PF_R_U16(intf_table_addr);
			if_ref_addr = MEMtPF8(resolveReferenceAddress(if_ref, classPkg, COMPONENT_CLASS));
			if (if_ref_addr == interface_addr)
			{
				matchFound = 1;
				break;
			}
			else
			{
				intf_table_addr = PF8_A_U16(intf_table_addr, U8_M_U8(PF_R_U8(PF8_A_CONST(intf_table_addr, IIT_NumInterfaces)), IIT_IndexSize));
				intf_table_addr	= PF8_A_CONST(intf_table_addr, IIT_FirstIndex);
			}
		}
		if (matchFound)
			break;
		/* The interface may have been implemented by a super class*/
		classRef = get_super_class(classAddr);
#if JCVM__VM__NOT_TRUSTED
		if (classRef == TERMINAL_CLASS_REF)
		{
			fatal_error(RESOLVE_VIRTUAL_METHOD_ERROR);
		}
#endif
		classAddr		= MEMtPF8(resolveReferenceAddress(classRef, classPkg, COMPONENT_CLASS));
		classPkg		= lastVMPkg;
	}

	/* check to see that the methodToken < number of entries of the current interface table address */
	if (method_intf_token >= PF_R_U8(PF8_A_CONST(intf_table_addr, IIT_NumInterfaces)))
	{
		fatal_error(METHOD_TOKEN_ERROR); /* methodToken exceeds the interface table's number of entries! */
	}

	/* Get method's virtual token */
	method_token = PF_R_U8(PF8_A_U16(intf_table_addr, U8_A_U16(IIT_FirstIndex, U8_M_U8(method_intf_token, IIT_IndexSize))));


	/* resolve virtual method */
	return resolveVirtualMethod(objref, method_token);
}

/*
* modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
*                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
*/
void callmethod(PF8 methodAddr, u8 methodPkg, u16 new_cc)	SMALL_MODE
{
/*
#if JCVM__VM__CHECK_STACK_POP
	u8 m_stacksize;
#endif

	u8 m_paramsize;
	u8 m_localsize;
	u8 btemp;
*/
	PF8				D_A		pkg_info_addr;
	u8				D_A		temp[4];
#if U8_2_U32_INVERSE
	#define  methodType		temp[3]
	#define  m_stacksize    temp[2]
	#define  btemp          temp[2]
	#define  m_paramsize    temp[1]
	#define  m_localsize    temp[0]
#else
	#define  methodType		temp[0]
	#define  m_stacksize    temp[1]
	#define  btemp          temp[1]
	#define  m_paramsize    temp[2]
	#define  m_localsize    temp[3]	
#endif	//U8_2_U32_INVERSE
	
	PATCH_JCVM_VM_METHOD_003();
	
//	if(PatchTableNumber)
//	{
//		u16 i;
//		for(i = 0; i < PatchTableNumber; U16_INC(i))
//		{
//			if(methodAddr == PatchTableEntry[i].oldAddressInMask)
//			{
//				methodAddr = MEMtPF8(MapTable_GetEntityContent(PatchTableEntry[i].patchAddressMapIndex));
//				break;
//			}
//		}
//	}

	
	//btemp = CODE_read_u8(methodAddr++);
	PXS_W_U32(&temp[0], PF_R_U32(methodAddr));

	/*
	 Determine the type of main()V method so as to
	 determine the amount of offset to add to the PC
	 and to the Stack Pointer
	 */
/*
	if ((btemp & ACC_EXTENDED) == ACC_EXTENDED)
	{ // extended header
#if JCVM__VM__CHECK_STACK_POP
		//stacksize
		m_stacksize = CODE_read_u8(methodAddr++);
#else
		methodAddr++;
#endif


		// paramsize
		m_paramsize = CODE_read_u8(methodAddr++);


		// localsize
		m_localsize = CODE_read_u8(methodAddr++);
	}
	else
	{ // short header
#if JCVM__VM__CHECK_STACK_POP
		// stackSize
		m_stacksize = btemp & STACK_NIBBLE;
#endif


		// Get next byte for next two fields
		btemp = CODE_read_u8(methodAddr++);


		// paramsize
		m_paramsize = (btemp & PARAMS_NIBBLE) >> NIBBLE_SHIFT;


		// localsize
		m_localsize = btemp & LOCALS_NIBBLE;
	}
*/


	if (U8_AND_U8(methodType, ACC_EXTENDED) != ACC_EXTENDED)
	{
		// paramsize
		m_paramsize	= U8_SHR_U8(U8_AND_U8(btemp, PARAMS_NIBBLE), NIBBLE_SHIFT);


		// localsize
		m_localsize = U8_AND_U8(btemp, LOCALS_NIBBLE);
		
#if JCVM__VM__CHECK_STACK_POP
		// stackSize
		m_stacksize = U8_AND_U8(methodType, STACK_NIBBLE);
#endif
	}

	/* make the new frame */
#if JCVM__VM__CHECK_STACK_POP
	pushFrame(m_paramsize, m_localsize, m_stacksize);
#else
	pushFrame(m_paramsize, m_localsize);
#endif

	cc = new_cc;


	/* next byte code will be the first after the header in this method */
	JPC = methodAddr;

	if (U8_AND_U8(methodType, ACC_EXTENDED) == ACC_EXTENDED)
	{
		JPC	= PF8_A_CONST(JPC, 4);
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, 2);
	}
	

	if (methodPkg != PCpkgID)
	{
		pkg_info_addr		= MEMtPF8(MapTable_GetEntityContent(PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(methodPkg, SIZE_MEMREF)))));

		if(methodPkg < NPACKAGES) // mask里的包
		{
			cachedClassCompLoc	= MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(pkg_info_addr, (COMPONENT_CLASS * (SIZE_MEMREF + SIZE_U16) + SIZE_U16))));
			cachedMethodCompLoc = MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(pkg_info_addr, (COMPONENT_METHOD * (SIZE_MEMREF + SIZE_U16) + SIZE_U16))));
			cachedFieldCompLoc	= MapTable_GetEntityContent(PF_R_U16(PN8_A_U16(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16) + SIZE_U16))));
	//		cachedFieldCompLen  = PF_R_U16(PF8_A_CONST(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16)+SIZE_U32)));

		}
		else
		{
			cachedClassCompLoc	= PFtMEM(pkg_info_addr + PF_R_U32(PN8_A_U16(pkg_info_addr, (COMPONENT_CLASS * (SIZE_MEMREF + SIZE_U16)))));
			cachedMethodCompLoc = PFtMEM(pkg_info_addr + PF_R_U32(PN8_A_U16(pkg_info_addr, (COMPONENT_METHOD * (SIZE_MEMREF + SIZE_U16)))));
			cachedFieldCompLoc	= PFtMEM(pkg_info_addr + PF_R_U32(PN8_A_U16(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16)))));
	//		cachedFieldCompLen  = PF_R_U16(PF8_A_CONST(pkg_info_addr, (COMPONENT_STATICFIELD * (SIZE_MEMREF + SIZE_U16)+SIZE_U32)));
		}
		PCpkgID				= methodPkg;
	}

}

