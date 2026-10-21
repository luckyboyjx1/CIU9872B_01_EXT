/*****************************************************************************
 * File:  bc_object.c
 * Version 1.12

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu, Alexandre Frey, Nicolas Regnault
 ****************************************************************************/
#include "bc_object.h"

#include "profile.h"
#include "stack.h"
#include "object.h"
#include "context.h"
/*#include "typejava.h"		   */
#include "mask.h"
#include "firewall.h"
#include "method.h"
#include "except.h"
#include "anomaly.h"
#include "memory.h"
#include "ob_manager.h"
#include "packageloc.h"
#include "reference.h"
#include "global_variable.h"
#include "native_interface.h"
#include "JcvmRam.h"
#include "NvmMap.h"
#include "crypto_impl.h"

/******************************************************************************
 * This method implements the Java Card 2.1 invokevirtual instruction.
 * Format:
 * invokevirtual
 *
 * Stack Image:
 * ..., objref [,arg1 [,arg2] [,...]]] --> ...

 ******************************************************************************/
void _invokevirtual()
{
	jref		D_A		objref;
	PF8			D_A		objaddr;
	PF8			D_A		methodAddr;
	u16			D_A		new_cc;

	u8			D_A		tmp[2]; // 0:nargs, 1:methodField
	u16			D_A		value;

	value	= fetchShort();
	PX_W_U16(&tmp[0], value);

	objref	= fetchStackReference(tmp[0]);
	objaddr	= MEMtPF8(locateObjectInMemory(objref));


	/* Is this really a valid Object Reference? */
	check_null(objaddr);


#if JCVM__VM__FIRE_WALL
	if (IS_REF_SIMPLEOBJECT(objaddr))
	{
		checkEntryPoint(objaddr); /* Security Exception */
	}
	else
	{ /* objref is an array reference */
		checkArrayRead(objaddr); /* Security Exception */
	}
#endif
	if (IS_ENTRY_OBJ(objaddr))
	{
		new_cc				= JCRE_CONTEXT;
	}
	else
	{
		new_cc				= get_object_owner(objaddr);
	}
	methodAddr	= MEMtPF8(resolveVirtualMethod(objaddr, tmp[1]));

	callmethod(methodAddr, lastVMPkg, new_cc);
}

/*******************************************************************************
 * This method implements the Java Card 2.1 invokespecial instruction.
 * Format:
 * invokespecial
 *
 * Stack Image:
 * ..., objref [,arg1 [,arg2] [,...]]] --> ...

 *******************************************************************************/
void _invokespecial()
{
	PF8			D_A		objaddr;
	jref		D_A		methodRef;
	PF8			D_A		methodAddr;
	u8			D_A		paramSize;

	methodRef	= fetchReference();
	methodAddr	= MEMtPF8(resolveReferenceAddress(methodRef, PCpkgID, COMPONENT_METHOD));

	paramSize	= getMethodParamSize(methodAddr);
#if JCVM__VM__NULLOBJ_CHECK
	objaddr		= MEMtPF8(locateObjectInMemory(fetchStackReference(paramSize)));
	/* Is this really a valid Object Reference? */
	check_null(objaddr);
#endif
	callmethod(methodAddr, lastVMPkg, cc);
}

/********************************************************************************
 * This method implements the Java Card 2.1 invokestatic instruction.
 * Format:
 * invokestatic
 *
 * Stack Image:
 * ..., [,arg1 [,arg2] [,...]]] --> ...
 this byte code is very high frequency, so is important for speed 3
 ********************************************************************************/
void _invokestatic()
{
	jref		D_A		methodRef;
	PF8			D_A		methodAddr;
	u8			D_A		methodPkg;

	methodRef	= fetchReference();

	if (U16_AND_U16(methodRef, 0x8000) != 0)
	{
		/* Reference is external - resolve using global reference table */
		methodAddr	= MEMtPF8(getReferenceAddress(methodRef));
		methodPkg	= lastVMPkg;
	}
	else
	{
		methodAddr	= PF8_A_U16(MEMtPF8(cachedMethodCompLoc), methodRef);
		methodPkg	= PCpkgID;
	}

	if (U8_AND_U8(PF_R_U8(methodAddr), ACC_NATIVE) == ACC_NATIVE)
	{
		/*  The native method pointer points to the address
		 *  of the native method that is to be executed
		 */
		if(methodPkg > (NPACKAGES - 2))
		{
			fatal_error(UNKNOWN_NATIVE);
		}
		//PF8_INC(methodAddr);
		callnative((u16)(PF_R_U16(methodAddr)&0x01FF));
	}
	else
	{
		callmethod(methodAddr, methodPkg, cc);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 invokeinterface instruction.
 * Format:
 * invokeinterface
 *
 * Stack Image:
 * ..., objref [,arg1 [,arg2] [,...]]] --> ...
 ********************************************************************************/
void _invokeinterface()
{
	PF8			D_A		objaddr;
	jref		D_A		interfaceClass;
	PF8			D_A		interfaceClassAddr;
	PF8			D_A		methodAddr;
	u8			D_A		methodPkg;
	u8			D_A		nargs;
	u16			D_A		new_cc;
	u8			D_A		methodToken;
	u8			D_A		interfacePackage;

	nargs				= fetchByte();
	interfaceClass		= fetchReference();
	interfacePackage	= resolveReferencePackage(interfaceClass, PCpkgID);
	interfaceClassAddr	= MEMtPF8(resolveReferenceAddress(interfaceClass, interfacePackage, COMPONENT_CLASS));
	methodToken			= fetchByte();


	/* Get Object Reference off of the stack */
	objaddr				= MEMtPF8(locateObjectInMemory(fetchStackReference(nargs)));
	/* Is this really a valid Object Reference? */
	check_null(objaddr);

	methodAddr			= MEMtPF8(resolveInterfaceMethod(objaddr, interfaceClassAddr, methodToken));

	methodPkg			= lastVMPkg;


#if JCVM__VM__FIRE_WALL
	if (IS_SHAREABLE(interfaceClassAddr))
	{
		checkShareable(objaddr); /* Security Exception */
		checkChannelAccess(objaddr);
	}
	else
	{
//		checkRead(objaddr);/* Security Exception */
		 checkEntryPoint(objaddr);    //实现了JCRE ENTRY的interface object是一个临时JCRE ENTRY object，测试uicc.toolkit中EnvelopeHandlerSystem.getTheHandler()

	}
#endif
	if (IS_ENTRY_OBJ(objaddr))
	{
		new_cc				= JCRE_CONTEXT;
	}
	else
	{
		new_cc				= get_object_owner(objaddr);
	}

	callmethod(methodAddr, methodPkg, new_cc);
}

/********************************************************************************
 * This method implements the Java Card 2.1 new instruction.
 * Format:
 * new
 *
 * Stack Image:
 * ... --> ..., objref
 * modified by xuxiaojun@20210328 调整memRefresh位置，减少写E2的次数
 * modified by xuxiaojun@20210414 增加参数，用于表示该函数中新创建对象的属性
 ********************************************************************************/
jref newObject(u16 classOffset, u8 classPkg, u8 needOperator)	SMALL_MODE
{
	u8			D_A		transactionStarted;
	PF8			D_A		classAddr;
	u32			D_A		size;
	jref		D_A		objref;
	PN8			D_A		objaddr;
	u16			D_A		objectDataSize;
	u8			X_A		TempObjBuffer[6];

	transactionStarted	= 0;
	//objectDataSize		= 0;

	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}/*end if*/
	classAddr		= PF8_A_U16(MEMtPF8(getPackageComponentLocation(classPkg, COMPONENT_CLASS)), classOffset);


	/* Determine Object size */
	objectDataSize	= get_class_size(classAddr);
	size			= U8_A_U32(SIZE_OBJECT_HEADER, U8_M_U16(SIZE_SHORT, objectDataSize));

	/* Try to obtain EEPROM space for the new object */
	objref			= NvmManager_Allocate(size, MAP_ENTRY_OBJECT_PCK_ID);
	objaddr			= MEMtPN8(MapTable_GetEntryAddr(objref));


	//check if memory allocation was successful and if not, abort the started transaction
	//and throw SYSTEMEXCEPT_NO_RESOURCE
	checkNullAddrAndManageTransaction(objaddr, transactionStarted);

	if(needOperator&BIT_SET_JCRE_ENTRY_Permanent) // if(ValueBit(needOperator, BIT_SET_JCRE_ENTRY_Permanent))
	{
		TempObjBuffer[0] = U8_OR_U8(Header_Simple_Object, U8_OR_U8(HEADER_JCRE_EntryPoint_Yes, HEADER_JCRE_STATE_Permanent));
	}
	else if(needOperator&BIT_SET_JCRE_ENTRY_Temporary) // if(ValueBit(needOperator, BIT_SET_JCRE_ENTRY_Temporary))
	{
		TempObjBuffer[0] = U8_OR_U8(Header_Simple_Object, HEADER_JCRE_EntryPoint_Yes);
	}
	else
	{
		TempObjBuffer[0] = Header_Simple_Object;
	}

	PXB_W_U16(&TempObjBuffer[1], cc);
	PXB_W_U16(&TempObjBuffer[3], classOffset);
	TempObjBuffer[5] = classPkg;
	nvmCopyNoAtomic(objaddr, TempObjBuffer, 6);

//	objref = (jref) addObject(objaddr,OBJECT_TABLE);
	/*check if addition of object was successful and we didn't get REF_NULL*/
	checkNullAndManageTransaction(objref, transactionStarted);

	nvmFillNoAtomic(PN8_A_CONST(objaddr, 6), 0, U8_M_U16(SIZE_SHORT, objectDataSize));

	if(needOperator&BIT_NEED_REFRESH)
	{
		memRefresh();
	}

	if (transactionStarted)
	{
		commit_transaction();
	}/*end if*/
	return objref;
}
/*
 * modified by xuxiaojun@20210328 调整memRefresh位置，减少写E2的次数
 */
void _new()
{
	u8			D_A		transactionStarted;
	jref		D_A		classRef;
	PF8			D_A		classAddr;
	jref		D_A		classOffset;
	u32			D_A		size;
	jref		D_A		objref;
	PN8			D_A		objaddr;
	u16			D_A		objectDataSize;
	u8			D_A		TempObjBuffer[6];

	transactionStarted = 0;
//	size = 0;
	//objectDataSize = 0;

	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}

	/* Fetch Class Reference from the instruction stream */
	classRef		= fetchReference();
	classAddr		= MEMtPF8(resolveReferenceAddress(classRef, PCpkgID, COMPONENT_CLASS));
	classOffset		= U32_S_U32(PFtMEM(classAddr), getPackageComponentLocation(lastVMPkg, COMPONENT_CLASS));


	/* Determine Object size */
	objectDataSize	= get_class_size(classAddr);
	size			= U8_A_U32(SIZE_OBJECT_HEADER, U8_M_U16(SIZE_SHORT, objectDataSize));


	/* Try to obtain EEPROM space for the new object */
	objref			= NvmManager_Allocate(size, MAP_ENTRY_OBJECT_PCK_ID);
	objaddr			= MEMtPN8(MapTable_GetEntryAddr(objref));


	//check if memory allocation was successful and if not, abort the started transaction
	//and throw SYSTEMEXCEPT_NO_RESOURCE
	checkNullAddrAndManageTransaction(objaddr, transactionStarted);

	TempObjBuffer[0] = Header_Simple_Object;
	PXB_W_U16(&TempObjBuffer[1], cc);
	PXB_W_U16(&TempObjBuffer[3], classOffset);
	TempObjBuffer[5] = lastVMPkg;
	nvmCopyNoAtomic(objaddr, TempObjBuffer, 6);

//	objref = (jref) addObject(objaddr,OBJECT_TABLE);
	/*check if addition of object was successful and we didn't get REF_NULL*/
	checkNullAndManageTransaction(objref, transactionStarted);

	/* init contents to zeros */
	nvmFillNoAtomic(PN8_A_CONST(objaddr, 6), 0, U8_M_U16(SIZE_SHORT, objectDataSize));
	memRefresh();

	/* Push the new object's reference onto the stack */
	pushReference(objref);

	if (transactionStarted)
	{
		commit_transaction();
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 checkcast instruction.
 * Format:
 * checkcast
 *
 * Stack Image:
 * ..., objref --> ..., objref

 ********************************************************************************/
void _checkcast()
{
	u8			D_A		atype;
	jref		D_A		classT;
	jref		D_A		objref;
	PF8			D_A		objAddr;
	PF8			D_A		classAddr;

	atype		= fetchByte();
	classT		= fetchReference();
	classAddr	= MEMtPF8(resolveReferenceAddress(classT, PCpkgID, COMPONENT_CLASS));

	objref		= popReference();
	objAddr		= MEMtPF8(locateObjectInMemory(objref));

	if (check_addr_null(objAddr))
	{
		pushReference(U32t16(PFtMEM(objAddr)));
	}
	else
	{


#if JCVM__VM__FIRE_WALL
		checkInstanceAndCast(objAddr, classAddr); /* Security Exception */
#endif

		if (isinstanceof(objAddr, atype, classAddr))
		{
			pushReference(objref);
		}
		else
		{
			throw_error(CLASS_CAST_EXCEPTION);
		}
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 instanceof instruction.
 * Format:
 * instanceof
 *
 * Stack Image:
 * ..., objref --> ..., result

 ********************************************************************************/
void _instanceof()
{
	u8			D_A		atype;
	jref		D_A		classT;
	PF8			D_A		objaddr;
	PF8			D_A		classAddr;

	atype		= fetchByte();


	/*In this case class is the class structure not the class instance. So the value
	 does not need to come from the object manager but from memory.*/
	classT		= fetchReference();
	classAddr	= MEMtPF8(resolveReferenceAddress(classT, PCpkgID, COMPONENT_CLASS));

	objaddr		= getJavaObjectDataRef();

	if (check_addr_null(objaddr))
	{
		pushShort(false);
	}
	else
	{


#if JCVM__VM__FIRE_WALL
		checkInstanceAndCast(objaddr, classAddr); /* Security Exception */
#endif

		if (isinstanceof(objaddr, atype, classAddr))
		{
			pushShort(true);
		}
		else
		{
			pushShort(false);
		}
	}
}

