/*****************************************************************************
 * File:  object.c
 * Version 1.9
 *
 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/03 at 13:23:48
 00/12/04 at 13:23:48 .Created
 * Original authors: David Getreu, Alexandre Frey, Nicolas Regnault
 ****************************************************************************/
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
#include "dtrspace.h"
#include "rtrSpace.h"
#include "tranManager.h"
#include "crypto_impl.h"

#define INSTANCEOF_ATYPE_CLASS_REF			0x0000

void set_JCRE_entry(PN8 objref, boolean isTemporary)
{
	u8 objType;

	objType = U8_AND_U8(get_object_header_type(objref), U8_INV(HEADER_JCRE_STATE_Permanent));

	if (isTemporary)
	{
		objType = U8_OR_U8(objType, HEADER_JCRE_EntryPoint_Yes);
	}
	else
	{
		objType = U8_OR_U8(objType, U8_OR_U8(HEADER_JCRE_EntryPoint_Yes, HEADER_JCRE_STATE_Permanent));
	}
	nvmWriteU8Atomic(objref, objType);

	memRefresh();
}

void set_JCRE_global(PN8 arrayref)
{
	u8 objType;

	objType = get_object_header_type(arrayref);
	nvmWriteU8Atomic(arrayref, U8_OR_U8(objType, HEADER_Global_Shared));

	memRefresh();
}

/****************************************************************************************************
 * Checks if S is a sub class of T.
 * S & T are references to Class Records.
 * S & T cannot be an Array. It can only be object or interface
 *****************************************************************************************************/
boolean subType(PF8 S, u8 Spkg, PF8 T)
{                                                                                                                                                                                                                   
	PF8			I_A		intf_addr;                                                                                                                                                                                                 
	jref		I_A		Sclass;                                                                                                                                                                                                      
	jref		I_A		ISclass;                                                                                                                                                                                                     
	PF8			I_A		IS;                                                                                                                                                                                                        
	u8			I_A		numInterfaces;                                                                                                                                                                                                 
	u8			I_A		i;                                                                                                                                                                                                             
	u1			B_A		result;
                                                                                                                                                                                                                    
	PATCH_JCVM_VM_OBJECT_001();	  
	
	result = false;
	/* Check to see if S is a Class */                                                                                                                                                                                
	if (!IS_INTERFACE(S))                                                                                                                                                                                             
	{                                                                                                                                                                                                                 
		/* S IS a class type, so check to see if T is a Class type */                                                                                                                                                   
		if (!IS_INTERFACE(T))                                                                                                                                                                                           
		{                                                                                                                                                                                                               
			/* T is a class type...so see if S is the same class as T or a subclass of T */                                                                                                                               
			do                                                                                                                                                                                                            
			{                                                                                                                                                                                                             
				if (S == T)                                                                                                                                                                           
				{                                                                                                                                                                                                           
					return (true);                                                                                                                                                                                            
				}                                                                                                                                                                                                           
				else                                                                                                                                                                                                        
				{                                                                                                                                                                                                           
					if (PF_R_U16(PF8_A_CONST(S, CR_SuperClass)) == TERMINAL_CLASS_REF)                                                                                                                                     
					{                                                                                                                                                                                                         
						return false;                                                                                                                                                                                           
					}                                                                                                                                                                                                         
					else                                                                                                                                                                                                      
					{                                                                                                                                                                                                         
						Sclass	= PF_R_U16(PF8_A_CONST(S, CR_SuperClass));                                                                                                                                                    
						S		= MEMtPF8(resolveReferenceAddress(Sclass, Spkg, COMPONENT_CLASS));                                                                                                                                        
						Spkg	= lastVMPkg;                                                                                                                                                                                       
					}                                                                                                                                                                                                         
				}                                                                                                                                                                                                           
			} while (true);                                                                                                                                                                                               
		}                                                                                                                                                                                                               
		else
		{
			do
			{
				if (PF_R_U16(PF8_A_CONST(S, CR_SuperClass)) == TERMINAL_CLASS_REF)
				{
					return false;
				}
				else
				{
					/* T IS an Interface type, so check to see if S implements T */

					/* Calculate the Class's initial Interface index table Address */
					//TODO:check asm code OK
					intf_addr		= MEMtPF8(U8_M_U8(sizeof(jref), PF_R_U8(PF8_A_CONST(S, (CR_PublicMethods + MTH_NumMethods)))));
					intf_addr		= PF8_A_U16(intf_addr, U8_M_U8(sizeof(jref), PF_R_U8(PF8_A_CONST(S, (CR_PackageMethods + MTH_NumMethods)))));                                                                                                                                                                                     
					intf_addr 		= PF8_A_U32(PF8_A_CONST(S, CR_MethodsTableStart), intf_addr);
		                                                                                                                                                                                                                    
					numInterfaces	= U8_AND_U8(PF_R_U8(PF8_A_CONST(S, CR_Header)), CRH_InterfaceCount);                                                                                                                                   
                                                                                                                                                                                                                    

					/* match up one of the Class's Interface entries with the Interface Class Address */
					for (i = 0; i < numInterfaces; U8_INC(i))
					{                                                                                                                                                                                                             
						ISclass 	= PF_R_U16(intf_addr);                                                                                                                                                                         
						IS 			= MEMtPF8(resolveReferenceAddress(ISclass, Spkg, COMPONENT_CLASS));                                                                                                                                          
						if (IS == T)                                                                                                                                                                          
						{                                                                                                                                                                                                           
							/* Got a match! */                                                                                                                                                                                        
							return (true);                                                                                                                                                                                            
						}                                                                                                                                                                                                           
						else                                                                                                                                                                                                        
						{
							intf_addr	= PF8_A_U16(intf_addr, U8_M_U8(PF_R_U8(PF8_A_CONST(intf_addr, IIT_NumInterfaces)), IIT_IndexSize));
							intf_addr	= PF8_A_CONST(intf_addr, IIT_FirstIndex);                                                                          
						}                                                                                                                                                                                                           
					}                                                                                                                                                                                                             
					/* No match! - Go to superclass */
					Sclass			= PF_R_U16(PF8_A_CONST(S, CR_SuperClass));
					S				= MEMtPF8(resolveReferenceAddress(Sclass, Spkg, COMPONENT_CLASS));
					Spkg			= lastVMPkg;
				}
			} while (true);
		}
	}
	else
	{
		/* S IS an interface type, so check to see if T is a Class Type */
		if (IS_INTERFACE(T))
		{
			/* T IS an Interface Type, so determine if it is same as S or a SuperInterface */
			if (S == T)                                                                                                                                                                             
			{
				/* Same Interface! */
				return (true);
			}
			else
			{
				/* Check to see if T is a SuperInterface of S */
				numInterfaces = U8_AND_U8(PF_R_U8(PF8_A_CONST(S, CR_Header)), CRH_InterfaceCount);

				/* Calculate the Class's initial Interface index table Address */
				intf_addr		= PF8_A_CONST(S, CR_SuperClass);

				for (i = 0; i < numInterfaces; U8_INC(i))
				{
					ISclass		= PF_R_U16(intf_addr);                                                                                                                                                                       
					IS			= MEMtPF8(resolveReferenceAddress(ISclass, Spkg, COMPONENT_CLASS));

					if (IS == T)                                                                                                                                                                        
					{
						return (true);
					}
					else
					{
						intf_addr = PF8_A_CONST(intf_addr, sizeof(jref));
					}
				}
				/* If we get here, then we have 'walked' the superlinks upward to where we've reached
				 the last SuperInterface and failed to find a match */
				return false;
			}
		}
		else
		{
			/* T is a Class Type, so it had better be 'CLASS' OBJECT! */                                                                                                                                                  
		//comError                                                                                                                                                                                                      
		//	return (boolean) ((u8 far *) T == resolveReferenceAddress((jref) OBJECT_CLASS_OFFSET, (u8) JAVA_LANG_PKG_ID, (u8) COMPONENT_CLASS));
			if(PFtMEM(T) == resolveReferenceAddress(OBJECT_CLASS_OFFSET, JAVA_LANG_PKG_ID, COMPONENT_CLASS))
			{
				result	 = true;
			}
			else
			{
				result	 = false;
			}

			return result;
		}
	}                                                                                                                                                                                                
}

boolean isinstanceof(PF8 ObjectAddr, u8 AType, PF8 ClassAddr)	SMALL_MODE
{
	PF8				I_A		S;
	jref 			I_A		Sclass;
	u8 				D_A		Spkg;
	u8				D_A		objectType;
	u1				B_A		result;
	
	PATCH_JCVM_VM_OBJECT_002();	 
	
	result = false;
	if (check_addr_null(ObjectAddr))
		return true;

	objectType = U8_AND_U8(get_object_header_type(ObjectAddr), OBJ_HDR_OBJECT_TYPE_MASK);

	switch (objectType)
	{
	case Header_Simple_Object:
		objectType = AType_Object;
		break;
	case Header_NoDefine:
		fatal_error(BAD_ARRAY_TYPE_ERROR);
		break;
	case Header_ArrayType_Byte:
		objectType = AType_Byte;
		break;
	case Header_ArrayType_Boolean:
		objectType = AType_Boolean;
		break;
	case Header_ArrayType_Short:
		objectType = AType_Short;
		break;
#if VM_INTS_SUPPORTED
	case Header_ArrayType_Int:
		objectType = AType_Int;
		break;
#endif
	case Header_ArrayType_Reference:
		objectType = AType_REF;
		break;
#if JCVM__VM__NOT_TRUSTED
		default:
		fatal_error(BAD_ARRAY_TYPE_ERROR);
#endif
	}

	switch (AType)
	{
	case AType_Byte:
	case AType_Boolean:
	case AType_Short:
#if VM_INTS_SUPPORTED
	case AType_Int:
#endif

		return (objectType == AType);

	case AType_REF:

		//TODO:all load -> CODE_read_ YES
		if (objectType == AType_REF)
		{
			/* Assumption here is that the ObjectRef is of type 'ObjectArray' */
			Sclass	= PF_R_U16(PF8_A_CONST(ObjectAddr, OA_ComponentClassClassRec));
			Spkg	= PF_R_U8(PF8_A_CONST(ObjectAddr, OA_ComponentClassClassRecPkg));
			S		= MEMtPF8(resolveReferenceAddress(Sclass, Spkg, COMPONENT_CLASS));
			//comError
	//		return subType(S, Spkg, ClassAddr);
			result	= subType(S, Spkg, ClassAddr);
			return result;
		}

		return false;

	case AType_Object:
		/* (... instanceof Object) -> always true */
		if (PFtMEM(ClassAddr) == resolveReferenceAddress(OBJECT_CLASS_OFFSET, JAVA_LANG_PKG_ID, COMPONENT_CLASS))
			return true;

		if (objectType == AType_Object)
		{
			Sclass	= PF_R_U16(PF8_A_CONST(ObjectAddr, OBJ_HDR_ClassRecordREF));
			Spkg	= PF_R_U8(PF8_A_CONST(ObjectAddr, OBJ_HDR_ClassRecordPKG));
			S 		= MEMtPF8(resolveReferenceAddress(Sclass, Spkg, COMPONENT_CLASS));
			return subType(S, Spkg, ClassAddr);
		}

		return false;
#if JCVM__VM__NOT_TRUSTED
  default:
    fatal_error(BAD_ARRAY_TYPE_ERROR);
#endif
	}

	return false;
}

/*****************************************************************************************************
 * Get the address where the elements of array_ref are stored. These elements
 * may be in E2PROM for persistent arrays or in RAM for transient arrays.
 *
 *****************************************************************************************************/
memref get_array_data(PF8 array_ref)
{
	PF8		base_address;
	u8		array_header;

	array_header	= PF_R_U8(array_ref);
	
	if (U8_AND_U8(array_header, Header_ArrayType_Reference) == Header_ArrayType_Reference)
	{
		base_address = PF8_A_CONST(array_ref, OA_DataArray_START);
	}
	else
	{
		base_address = PF8_A_CONST(array_ref, PA_DataArray_START);
	}

	if (U8_AND_U8(array_header, HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
	{
		// Logical channels: get transient RAM segment baseaddress.
		// Determine whether this is a ClearOnReset or ClearOnDeselect
		// to choose currectly the address to retrieve transient data.
		if (U8_AND_U8(array_header, HEADER_JCRE_TransMode_Deselect) == HEADER_JCRE_TransMode_Deselect)
		{
			base_address	= MEMtPF8(U32_A_U32(PFtMEM(PF_R_U16(base_address)), dtrSpace_getDtrSpaceBase(dtrSpace_getDtrSpaceId(selected_channel))));
		}
		else
		{
#if (EUICC_ONLY||SE_AND_EUICC) //modify by jiangxiaohui@20220324 ·Ö²ã
			if(U8_AND_U8(array_header, OBJ_HDR_PROFILE_RTR_MASK) == HEADER_PROFILE_RTR)//reuse this bit to show whether a proile rtr or a SE RTR
			{
				base_address	= MEMtPF8(U32_A_U16(PFtMEM(PF_R_U16(base_address)), rtrSpace_getRtrSpaceBase(RtrSpace_searchProfileRtrID())));
			}
			else
#endif
			{
				base_address	= MEMtPF8(U32_A_U16(PFtMEM(PF_R_U16(base_address)), rtrSpace_getRtrSpaceBase(SE_SPACE_ID)));
			}

		}
		base_address = MEMtPF8(RamManager_getAlloRamBase(U32t16(PFtMEM(base_address))));
	}
	
	return PFtMEM(base_address);
}

void check_bounds(PF8 array_ref, u16 index, u8 arrayType)
{
	u8 objecType;
	
	objecType = U8_AND_U8(get_object_header_type(array_ref), OBJ_HDR_OBJECT_TYPE_MASK);
	if((arrayType == objecType) || (objecType == Header_ArrayType_Boolean)) 
	{
		/* verify index is >= 0 and < array.length */
		if ((0x8000 != (index & 0x8000)) && (index < get_array_length(array_ref)))
		{
			return;
		}
		else
		{
			throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
		}
				
	}
	else
	{
		throw_error(CLASS_CAST_EXCEPTION);
	}

}

void checkNullAndManageTransaction(jref objRef, u8 transactionStarted)
{
	if (objRef == REF_NULL)
	{
		/* out of memory! */
		if (transactionStarted)
		{
			abort_transaction(ABORT_NOT_ON_RESET);
		}/*end if*/
		throw_error(SYSTEMEXCEPT_NO_RESOURCE);
	}
}

void checkNullAddrAndManageTransaction(PF8 objAddr, u8 transactionStarted)		
{
	if (check_addr_null(objAddr))
	{
		/* out of memory! */
		if (transactionStarted)
		{
			abort_transaction(ABORT_NOT_ON_RESET);
		}/*end if*/
		throw_error(SYSTEMEXCEPT_NO_RESOURCE);
	}
}

#if !EMV_OPTIMIZATION_CODE
WEAK memref get_object_class_impl(PF8 objAddr)
{
	//TODO:reduce u32 cal NO
	jref		I_A		classRecRef;
	u8			I_A		classRecPkg;
							   	
	classRecRef	= PF_R_U16(PF8_A_CONST(objAddr, OBJ_HDR_ClassRecordREF));
	classRecPkg	= PF_R_U8(PF8_A_CONST(objAddr, OBJ_HDR_ClassRecordPKG));

	return resolveReferenceAddress(classRecRef, classRecPkg, COMPONENT_CLASS);
}
#endif //!EMV_OPTIMIZATION_CODE

void check_null(PF8 ref) 
{
	 if (PFtMEM(ref)== REF_NULL) 
	 {
	 	throw_error(NULL_POINTER_EXCEPTION);
	 }
}


void checkIsGlobal(void)
{
	PN8 objAddr;

	objAddr = getJavaObjectDataRef();

	if (U8_AND_U8(get_object_header_type(objAddr), HEADER_Global_Shared) == HEADER_Global_Shared)
	{
		pushShort(true);
		return;
	}

	pushShort(false);
}
