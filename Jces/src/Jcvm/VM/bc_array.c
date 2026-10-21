/*****************************************************************************
 * File:  bc_array.c
 * Version 1.10

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 *****************************************************************************/
#include "bc_array.h"

#include "profile.h"
#include "context.h"
#include "firewall.h"
#include "memory.h"
#include "stack.h"
#include "object.h"
#include "anomaly.h"
#include "packageloc.h"
#include "reference.h"
#include "ob_manager.h"
#include "global_variable.h"
#include "crypto_impl.h"
#include "mask.h"
#include "NvmMap.h"
/*@ -typedefparam@*/
/*@ -type@*/
/***************************************************************************************
 * This method implements the Java Card 2.1 newarray instruction.
 * Format:
 * newarray atype
 *
 * Stack Image:
 * ..., count --> ..., arrayref
 * modified by xuxiaojun@20210328 调整memRefresh位置，减少写E2的次数
 * modified by xuxiaojun@20210414 增加参数，用于表示该函数中新创建对象的属性
 ****************************************************************************************/
jref _newarrayObject(u8 atype, u16 count, u8 needOperator)		SMALL_MODE
{
	u32			D_A		size;
	jref		D_A		arrayref;
	PN8			D_A		arrayaddr;
	jref		D_A		internalTypeClass;
//	u8			D_A		internalTypeClassPkg;
	u8			D_A		internalType;
	u8			D_A		typeSize;
	u8			D_A		transactionStarted;
	u8			X_A		TempObjBuffer[8];

	internalTypeClass		= 0;
	internalType			= 0;
	typeSize				= 0;
	transactionStarted		= 0;

	/* check to see if the count is negative; if so, throw exception */
	if (0x8000 == U16_AND_U16(count, 0x8000))
	{
		throw_error(NEGATIVE_ARRAY_SIZE_EXCEPTION);
	}

	/* Determine elements size, internal type, and internal class */
//	internalTypeClassPkg 	= VM_ARRAY_IMPLEMENTATION_PKG;
	switch (atype)
	{
	case AType_Byte:
		typeSize			= SIZE_BYTE;
		internalType 		= Header_ArrayType_Byte;
		internalTypeClass	= Byte_PA_CLASS_OFFSET;
		break;
	case AType_Boolean:
		typeSize			= SIZE_BOOLEAN;
		internalType		= Header_ArrayType_Boolean;
		internalTypeClass	= Boolean_PA_CLASS_OFFSET;
		break;
	case AType_Short:
		typeSize			= SIZE_SHORT;
		internalType		= Header_ArrayType_Short;
		internalTypeClass	= Short_PA_CLASS_OFFSET;
		break;
#if VM_INTS_SUPPORTED
	case AType_Int:
		typeSize			= SIZE_INT;
		internalType		= Header_ArrayType_Int;
		internalTypeClass	= Int_PA_CLASS_OFFSET;
		break;
#endif
#if JCVM__VM__NOT_TRUSTED
		default:
		fatal_error(BAD_ARRAY_TYPE_ERROR);
#endif
	}
	// the total size
	size	= U32_A_U32(U8t32(SIZE_PRIMITIVE_ARRAY_HEADER), U16_M_U16(count, typeSize));
	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}/*end if*/

	//arrayaddr = E2P_alloc(size);
	arrayref	= NvmManager_Allocate(size, MAP_ENTRY_OBJECT_PCK_ID);
	arrayaddr	= MEMtPN8(MapTable_GetEntryAddr(arrayref));


	/*check if memory allocation was successful*/
	checkNullAddrAndManageTransaction(arrayaddr, transactionStarted);


	/* Initialize the Array's Header */
	if(needOperator&BIT_SET_JCRE_ENTRY_Permanent) // if(ValueBit(needOperator, BIT_SET_JCRE_ENTRY_Permanent))
	{
		TempObjBuffer[0] = U8_OR_U8(internalType, U8_OR_U8(HEADER_JCRE_EntryPoint_Yes, HEADER_JCRE_STATE_Permanent));
	}
	else if(needOperator&BIT_SET_JCRE_ENTRY_Temporary) // if(ValueBit(needOperator, BIT_SET_JCRE_ENTRY_Temporary))
	{
		TempObjBuffer[0] = U8_OR_U8(internalType, HEADER_JCRE_EntryPoint_Yes);
	}
	else
	{
		TempObjBuffer[0] = internalType;
	}

	PXB_W_U16(&TempObjBuffer[1], cc);
	PXB_W_U16(&TempObjBuffer[3], internalTypeClass);
	TempObjBuffer[5] = VM_ARRAY_IMPLEMENTATION_PKG;
	PXB_W_U16(&TempObjBuffer[6], count);
	nvmCopyNoAtomic(arrayaddr, TempObjBuffer, 8);
	

//	arrayref = addObject(arrayaddr,OBJECT_TABLE);
	/*check if object addition was successful*/
	checkNullAndManageTransaction(arrayref, transactionStarted);

	/* init contents to zeros *///this used noatomic modified 2009.4.14
	nvmFillNoAtomic(PN8_A_CONST(arrayaddr, 8), 0, U32_S_U8(size, SIZE_PRIMITIVE_ARRAY_HEADER));

	if(needOperator&BIT_NEED_REFRESH)
	{
		memRefresh();
	}

	if (transactionStarted)
	{
		commit_transaction();

	}/*end if*/
	return arrayref;
	
}
/*
 * modified by xuxiaojun@20210414 根据被调用函数的参数增加而进行修改
 */
void _newarray()
{
	/* fetch array type from instruction stream */
	u8	 D_A	atype;
	/* pop array element count from the stack */
	u16	 D_A	count;

	atype	= fetchByte();
	count	= popShort();

	pushReference(_newarrayObject(atype,count, BIT_NEED_REFRESH));
}

/****************************************************************************************
 * This method implements the Java Card 2.1 anewarray instruction.
 * Format:
 * anewarray eltclass
 *
 * (We assume the linker has replaced the original index to constant pool by
 * the direct address of element class record)
 *
 * Stack Image:
 * ..., count --> ..., arrayref
 * modified by xuxiaojun@20210328 调整memRefresh位置，减少写E2的次数
 ****************************************************************************************/
void _anewarray()
{
	u16			D_A		count;
	PN8			D_A		arrayaddr;
	jref		D_A		arrayref;
	jref		D_A		eltoffset;
	PF8			D_A		eltclass;
	jref		D_A		eltRef;
	u32			D_A		size;
	u8			D_A		transactionStarted;
	u8			D_A		TempObjBuffer[11];

	transactionStarted	= 0;
	/* fetch array 'Component Class' Class Record from instruction stream */
	eltRef				= fetchReference();
	eltclass			= MEMtPF8(resolveReferenceAddress(eltRef, PCpkgID, COMPONENT_CLASS));
	// modified by wh 2010.2.22 , used cache must be faster.
	eltoffset			= U32t16(U32_S_U32(PFtMEM(eltclass), getPackageComponentLocation(lastVMPkg, COMPONENT_CLASS)));
	/* pop array element count from the stack */
	count				= popShort();


	/* check for negative count */
	if (0x8000 == U16_AND_U16(count, 0x8000))
	{
		throw_error(NEGATIVE_ARRAY_SIZE_EXCEPTION);
	}

	/* compute total amound of storage needed */
	size	= U32_A_U32(U8t32(SIZE_OBJECT_ARRAY_HEADER), U16_M_U16(count, SIZE_REF));

	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}/*end if*/
	/* Request EEPROM storage space */

	arrayref	= NvmManager_Allocate(size, MAP_ENTRY_OBJECT_PCK_ID);
	arrayaddr	= MEMtPN8(MapTable_GetEntryAddr(arrayref));

	/*check if memory allocation was successful*/
	checkNullAddrAndManageTransaction(arrayaddr, transactionStarted);

	/* Initialize the Array's Object Header fields */
	/* Global = 0; JCRE_EntryPointObj = 0; TransientMode = 0; TransientObj = 0; */
	/* Don't mind atomicity: this block is not yet accessible */ 
	TempObjBuffer[0] = Header_ArrayType_Reference;
	PXB_W_U16(&TempObjBuffer[1], cc);
	PXB_W_U16(&TempObjBuffer[3], Object_PA_CLASS_OFFSET);
	TempObjBuffer[5] = VM_ARRAY_IMPLEMENTATION_PKG;
	PXB_W_U16(&TempObjBuffer[6], count);
	PXB_W_U16(&TempObjBuffer[8], eltoffset);
	TempObjBuffer[10] = lastVMPkg;
	nvmCopyNoAtomic(arrayaddr, TempObjBuffer, 11);
	/* init contents to zeros */

//	arrayref = addObject(arrayaddr,OBJECT_TABLE);
	/*check if object addition was successful*/
	checkNullAndManageTransaction(arrayref, transactionStarted);

	/* init contents to zeros */
	nvmFillNoAtomic(PN8_A_CONST(arrayaddr, 11), 0, U8_M_U16(SIZE_REF, count));
	memRefresh();

	pushReference(arrayref);

	if (transactionStarted)
	{
		commit_transaction();
	}/*end if*/
}

/*
 * This method implements the Java Card 2.1 arraylength instruction.
 * Format:
 * arraylength
 *
 * Stack Image:
 * ..., arrayref --> ..., length
 */
void _arraylength()
{
	PN8		D_A		arrayaddr;
	u8 		D_A		arrayType;

	arrayaddr	= 	getJavaObjectDataRef();	// pop arrayref
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif

#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif
	arrayType 	=	get_object_header_type(arrayaddr);
	arrayType	= 	U8_AND_U8(arrayType, OBJ_HDR_OBJECT_TYPE_MASK);
	switch(arrayType)
	{
		case Header_ArrayType_Boolean:
		case Header_ArrayType_Byte:
		case Header_ArrayType_Short:
		case Header_ArrayType_Int:
		case Header_ArrayType_Reference :
			pushShort(get_array_length(arrayaddr));
		break;

		default:
			faultDetected();
		break;
	}
}

/*
 * This method implements the Java Card 2.1 aaload instruction.
 * Format:
 * aaload
 *
 * Stack Image:
 * ..., arrayref, index --> ..., value
 */
void _aaload()
{
	u16		D_A		index;
	PF8		D_A		arrayaddr;

	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();	// pop arrayref

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Reference);

	pushReference(memReadU16(PM8_A_U32(MEMtPM8(get_array_data(arrayaddr)), U8_M_U16(SIZE_REF, index))));
}

/*
 * This method implements the Java Card 2.1 aastore instruction.
 * Format:
 * aastore
 *
 * Stack Image:
 * ..., arrayref, index, value --> ...
 */
void _aastore()
{
	jref		D_A		arrayClassRef;
	u8			D_A		arrayClassPkg;


	/*Saqib:
	 In this case variable valueId has been added, which is to be actually stored in the array
	 itself rather than the actual address. value variable is used to get the actual
	 address used to check if the value is of the type that is being stored in the array
	 */
	jref		D_A		valueId;
	u16			D_A		index;

	PN8			D_A		arrayaddr;
	PN8			D_A		valueaddr;

	valueId		= popReference();
	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();	// pop arrayref
	valueaddr	= MEMtPN8(locateObjectInMemory(valueId));

#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif

	arrayClassRef = get_array_component_class(arrayaddr);
	arrayClassPkg = get_array_component_pkg(arrayaddr);

	if (isinstanceof(valueaddr, AType_Object, MEMtPF8(resolveReferenceAddress(arrayClassRef, arrayClassPkg, COMPONENT_CLASS))))
	{
		/* Now do the FireWall checks */

#if JCVM__VM__FIRE_WALL
		checkArrayStore(arrayaddr, valueId);
#endif

		check_bounds(arrayaddr, index, Header_ArrayType_Reference);


		/* place the value where it belongs... */
		/*Put the value ID in there because that's what needs to go in the array, not the
		 original address in the memory*/
		memWriteU16Atomic(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_REF, index)), valueId);
	}
	else
	{
		/* throw a ClassCastException */
		throw_error(ARRAY_STORE_EXCEPTION);
	}
}

/*
 * This method implements the Java Card 2.1 baload instruction.
 * Format:
 * baload
 *
 * Stack Image:
 * ..., arrayref, index --> ..., value
 */
#if !EMV_OPTIMIZATION_CODE
WEAK void _baload()
{
	u16		D_A		index;
	PF8		D_A		arrayaddr;

	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();	// pop arrayref

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Byte);

	pushShort(U8tS16(memReadU8(PF8_A_U16(MEMtPF8(get_array_data(arrayaddr)), index))));
}
#endif //!EMV_OPTIMIZATION_CODE

/*
 * This method implements the Java Card 2.1 bastore instruction.
 * Format:
 * bastore
 *
 * Stack Image:
 * ..., arrayref, index, value --> ...
 * 性能优化 by xuxiaojun@20211130
 */
#if !EMV_OPTIMIZATION_CODE
WEAK void _bastore()
{
	u16		D_A		value;
	u16		D_A		index;
	PN8		D_A		arrayaddr;

	value		= popShort();
	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();	// pop arrayref

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Byte);


	/* place the value where it belongs... */
	arrayaddr = PN8_A_U16(MEMtPN8(get_array_data(arrayaddr)), index);

	if(memReadU8(arrayaddr) != value)
	{
		memWriteU8Atomic(arrayaddr, U16t8(value));
	}

}
#endif //!EMV_OPTIMIZATION_CODE

/*
 * This method implements the Java Card 2.1 saload instruction.
 * Format:
 * saload
 *
 * Stack Image:
 * ..., arrayref, index --> ..., value
 */
#if !EMV_OPTIMIZATION_CODE
WEAK void _saload()
{
	u16			D_A			index;
	PN8			D_A			arrayaddr;

	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();	// pop arrayref

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Short);

	pushShort(memReadU16(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_SHORT, index))));
}
#endif //!EMV_OPTIMIZATION_CODE

/*
 * This method implements the Java Card 2.1 sastore instruction.
 * Format:
 * sastore
 *
 * Stack Image:
 * ..., arrayref, index, value --> ...
 */
#if !EMV_OPTIMIZATION_CODE
WEAK void _sastore()
{
	u16		D_A		value;
	u16		D_A		index;
	PN8		D_A		arrayaddr;

	value		= popShort();
	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Short);


	/* place the value where it belongs... */
	memWriteU16Atomic(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_SHORT, index)), value);
}
#endif //!EMV_OPTIMIZATION_CODE

#if VM_INTS_SUPPORTED

/*
 * This method implements the Java Card 2.1 iaload instruction.
 * Format:
 * iaload
 *
 * Stack Image:
 * ..., arrayref, index --> ..., value.word1, value.word2
 */
void _iaload()
{
	u16		D_A		index;
	PN8		D_A		arrayaddr;

	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();	// pop arrayref

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Int);

	pushInt(memReadU32(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_INT, index))));
}

/*
 * This method implements the Java Card 2.1 iastore instruction.
 * Format:
 * iastore
 *
 * Stack Image:
 * ..., arrayref, index, value.word1, value.word2 --> ...

 */
void _iastore()
{
	u32		D_A		value;
	u16		D_A		index;
	PN8		D_A		arrayaddr;

	value		= popInt();
	index		= popShort();
	arrayaddr	= getJavaObjectDataRef();

	/* null pointer check and FireWall check */
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif


#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif

	check_bounds(arrayaddr, index, Header_ArrayType_Int);


	/* place the value where it belongs... */
	memWriteU32Atomic(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_INT, index)), value);
}

#endif

