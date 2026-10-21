/*****************************************************************************
 * File:  ob_manager.c
 * Version 1.9
 *
 wang hui
 eastcompeace Technology Corporation

 History:
 Last modified on 07/07/03 at 13:23:48
 00/12/04 at 13:23:48 .Created
 * Original authors: David Getreu
 ****************************************************************************/
#include "CgmManager.h"
#include "ob_manager.h"
#include "object.h"
#include "gc.h"
#include "mask.h"
#include "context.h"
#include "temp_ob_manager.h"
#include "reference.h"
#include "packageloc.h"
#include "MemBackUp.h"
#include "global_variable.h"
#include "RamManager.h"
#include "JcvmRam.h"
#include "NvmMap.h"

#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_OB_MANAGER_RAM_START
#endif

/*********************************************************************************
 Static Method Declarations
 **********************************************************************************/
u32	getObjTotalSize(PN8 objAddress)
{
	u8		obType;
	u32 	objSize;
	
	obType		= PN_R_U8(objAddress);
	
	if (U8_AND_U8(obType, HEADER_JCRE_TransObj_IsTrans) == HEADER_JCRE_TransObj_IsTrans)
	{
		//obType		= U8_AND_U8(obType, OBJ_HDR_OBJECT_TYPE_MASK);
		
		if(GC_TRANSIENT_REFERENCE_ARRAY == TYPE(obType))
		{
			objSize	= U8t32(SIZE_OBJECT_ARRAY_HEADER);
		}
		else
		{
			objSize	= U8t32(SIZE_PRIMITIVE_ARRAY_HEADER);
		}
		
		objSize	= U32_A_U8(objSize, sizeof(jref));
	}
	else
	{
		objSize	= getObjectSize(objAddress);
	}
	
	return objSize;
}

memref locateObjectInMemory(jref objectId) 
{

#if JCVM__VM__RMI

	if (objectId >= START_TEMPORARY_OBJECT_ID)
	{
		// temporary object
		return PNtMEM(PN8_A_U16(MEMtPN8(Temporary_Array_BASE), U8_M_U8(TEMP_OBJECT_HEADER_LENGTH, U8_S_U8(objectId, START_TEMPORARY_OBJECT_ID))));
	}

#endif

	return MapTable_GetObjAddr(objectId);	
}/*end locateObjectInMemory*/

/**************************************************************************************
 * FUNCTION:      getObjectSize
 * OVERVIEW:      Calculates and returns the size of the object
 *
 * INTERFACE:
 *   parameters:  Object Address
 *   returns:     Object size
 *  modify by wanghui 2007.7.3,
 *  remove the method compute_array_size(u8 header_size, u16 count, u8 element_size)
 *************************************************************************************/
u32 getObjectSize(PF8 objectAddr)
{
	u8			D_A		type;
	u8			D_A		objectHeaderSize;
	u8			D_A		size;
	u16			D_A		count;
	jref		D_A		classRef2;
	PN8 		D_A		classAddress;
	u8 			D_A		clPkg;
	
	PATCH_JCVM_VM_OB_MANAGER_004();
	
	objectHeaderSize	= 0;
	size				= 0;
	count				= 0;
	type				= TYPE(PF_R_U8(objectAddr));
	/*If the object is a simple instance */
	if (type == GC_SIMPLE_INSTANCE)
	{
		classRef2		= PF_R_U16(PF8_A_U8(objectAddr, OBJ_HDR_ClassRecordREF));
		clPkg			= PF_R_U8(PF8_A_U8(objectAddr, OBJ_HDR_ClassRecordPKG));
		classAddress	= MEMtPF8(resolveReferenceAddress(classRef2, clPkg, COMPONENT_CLASS));
		//TODO:check asm code NO
		return U16t32(U8_A_U16(SIZE_OBJECT_HEADER, (U8_M_U8(SIZE_SHORT, get_class_size(classAddress)))));
	}/*end if*/
	else
	{
		/*
		 If the object is an array. Switch on array types. In case of transients we don't add the
		 size of the header since that is in EEPROM, that is why in calcuating the size of these
		 the first parameter to the method "compute_array_size" is 0.
		 */
		/*count is the 5th and 6th byte in the object header*/

		count			= PF_R_U16(PF8_A_U8(objectAddr, OA_NumFields_WORD));
		switch (type)
		{
		case GC_BOOLEAN_ARRAY:
			size = SIZE_BOOLEAN;
			objectHeaderSize = SIZE_PRIMITIVE_ARRAY_HEADER;
			break;
		case GC_BYTE_ARRAY:
			size = SIZE_BYTE;
			objectHeaderSize = SIZE_PRIMITIVE_ARRAY_HEADER;
			break;
		case GC_SHORT_ARRAY:
			size = SIZE_SHORT;
			objectHeaderSize = SIZE_PRIMITIVE_ARRAY_HEADER;
			break;
		case GC_REFERENCE_ARRAY:
			size = SIZE_REF;
			objectHeaderSize = SIZE_OBJECT_ARRAY_HEADER;
			break;
		case GC_INT_ARRAY:
			size = SIZE_INT;
			objectHeaderSize = SIZE_PRIMITIVE_ARRAY_HEADER;
			break;
		case GC_TRANSIENT_BOOLEAN_ARRAY:
			size = SIZE_BOOLEAN;
			break;
		case GC_TRANSIENT_BYTE_ARRAY:
			size = SIZE_BYTE;
			break;
		case GC_TRANSIENT_SHORT_ARRAY:
			size = SIZE_SHORT;
			break;
		case GC_TRANSIENT_INT_ARRAY:
			size = SIZE_INT;
			break;
		case GC_TRANSIENT_REFERENCE_ARRAY:
			size = SIZE_REF;
			break;
		default:
			break;
		}/*end siwtch*/

		return U32_A_U8(U16t32(U16_M_U16(U8t16(size), count)), objectHeaderSize);
	}/*end else*/

}/*end getObjectSize*/
