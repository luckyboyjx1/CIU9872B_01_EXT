/*****************************************************************************
 * File:  temp_ob_manager.c
 * Version 1.4

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,16	.modify
 * Original author: David Getreu
 *****************************************************************************/
#include "temp_ob_manager.h"
#include "memory.h"
#include "object.h"
#include "mask.h"
#include "context.h"
#include "anomaly.h"
#include "global_variable.h"

#if JCVM__VM__RMI

/* In a real implementation, the following two values should be in RAM */

/* The start Id that is incremented for all temporary objects */
//jref X_A returnedTempArrayId = 0;

/* The following should be a ram byte, but needs to be global for these functions */
//u8 X_A noOfTemporaryObjects = 0;

jref commonCreateTempArray(u8 atype, u8 count, u16 arrayref)
{
	jref		X_A	internalTypeClass = 0;
	u8			X_A	internalTypePkg;
	u8			X_A	internalType = 0;

	memref		X_A	dest;

	if (returnedTempArrayId == 0)
		returnedTempArrayId = START_TEMPORARY_OBJECT_ID;

	if (noOfTemporaryObjects >= MAX_TEMPORARY_OBJECTS)
	{
		throw_error(SYSTEMEXCEPT_RMI_RESOURCE);
	}

	internalTypePkg = VM_ARRAY_IMPLEMENTATION_PKG;

	switch (atype)
	{
	case AType_Byte:
		internalType = Header_ArrayType_Byte;
		internalTypeClass = Byte_PA_CLASS_OFFSET;
		break;
	case AType_Boolean:
		internalType = Header_ArrayType_Boolean;
		internalTypeClass = Boolean_PA_CLASS_OFFSET;
		break;
	case AType_Short:
		internalType = Header_ArrayType_Short;
		internalTypeClass = Short_PA_CLASS_OFFSET;
		break;
#if VM_INTS_SUPPORTED
	case AType_Int:
		internalType = Header_ArrayType_Int;
		internalTypeClass = Int_PA_CLASS_OFFSET;
		break;
#endif
		default:
#if JCVM__VM__NOT_TRUSTED
		fatal_error(BAD_ARRAY_TYPE_ERROR);
#endif
       break;
	}
	/* Please note, a temporary object must behave as a transient object in that
	 * the data does not follow the header, the data reference follows the header.
	 * To allow this to happen the internal type will indicate transient object.
	 */
	internalType = (jbyte) (internalType | HEADER_JCRE_TransObj_IsTrans/* | HEADER_JCRE_TransMode_Reset*/
	| HEADER_Global_Shared);

	dest = (memref) (Temporary_Array_BASE + (noOfTemporaryObjects * TEMP_OBJECT_HEADER_LENGTH));
	/* Initialize the Array's Header */

	memWriteU8Atomic(MEMtPM8(dest++), (jbyte)internalType);
	memWriteU16Atomic(MEMtPM8(dest++), (s16)cc);
	dest++;
	memWriteU16Atomic(MEMtPM8(dest++), (s16)internalTypeClass);
	dest++;
	memWriteU8Atomic(MEMtPM8(dest++), (jbyte)internalTypePkg);
	memWriteU16Atomic(MEMtPM8(dest++), count);
	dest++;
	nvmWriteU16Atomic(MEMtPM8(dest), (s16)arrayref);
	noOfTemporaryObjects++;
	return (returnedTempArrayId++);
}

void resetTempObjManager()
{
	returnedTempArrayId = START_TEMPORARY_OBJECT_ID;
	noOfTemporaryObjects = 0;
}

#endif
