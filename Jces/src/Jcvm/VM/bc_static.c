/*****************************************************************************
 * File:  bc_static.c
 * Version 1.9

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "bc_static.h"
#include "global_variable.h"
#include "context.h"
#include "stack.h"
#include "memory.h"
#include "firewall.h"
#include "ob_manager.h"
#include "reference.h"
#include "packageloc.h"

/******************************************************************************
 * This method implements the Java Card 2.1 getstatic_a instruction.
 * Format:
 * getstatic_a
 *
 * Stack Image:
 * ... --> .., value

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _getstatic_a()
{
	jref		D_A		index;
	jref		D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= nvmReadU16(indexAddr);
	pushReference(value);
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 getstatic_b instruction.
 * Format:
 * getstatic_b
 *
 * Stack Image:
 * ... --> .., value

 ******************************************************************************/
void _getstatic_b()
{
	jref		D_A		index;
	u8			D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= nvmReadU8(indexAddr);
	pushShort(U8tS16(U8tS16(value)));
}

/******************************************************************************
 * This method implements the Java Card 2.1 getstatic_s instruction.
 * Format:
 * getstatic_s
 *
 * Stack Image:
 * ... --> .., value

 ******************************************************************************/
void _getstatic_s()
{
	jref		D_A		index;
	u16			D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= nvmReadU16(indexAddr);//lmc:
	pushShort(value);
}

/******************************************************************************
 * This method implements the Java Card 2.1 putstatic_a instruction.
 * Format:
 * putstatic_a
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _putstatic_a()
{
	jref		D_A		index;
	jref		D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= popReference();


#if JCVM__VM__FIRE_WALL
	checkStaticStore(MEMtPF8(locateObjectInMemory(value))); /* Exit early if the security check fails */
#endif


	/* place the value where it belongs... */
	memWriteU16Atomic(indexAddr, value);
}

/******************************************************************************
 * This method implements the Java Card 2.1 putstatic_b instruction.
 * Format:
 * putstatic_b
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _putstatic_b()
{
	jref		D_A		index;
	u8			D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= U16t8(popShort());


	/* place the value where it belongs... */
	memWriteU8Atomic(indexAddr, value);
}

/******************************************************************************
 * This method implements the Java Card 2.1 putstatic_s instruction.
 * Format:
 * putstatic_s
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _putstatic_s()
{
	jref		D_A		index;
	u16			D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= popShort();


	/* place the value where it belongs... */
	memWriteU16Atomic(indexAddr, value);
}

#if VM_INTS_SUPPORTED
/******************************************************************************
 * This method implements the Java Card 2.1 getstatic_i instruction.
 * Format:
 * getstatic_i
 *
 * Stack Image:
 * ... --> .., value.word1, value.word2

 ******************************************************************************/
void _getstatic_i()
{
	jref		D_A		index;
	u32			D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= nvmReadU32(indexAddr);
	pushInt(value);
}

/******************************************************************************
 * This method implements the Java Card 2.1 putstatic_i instruction.
 * Format:
 * putstatic_i
 *
 * Stack Image:
 * ..., value --> ...
 ******************************************************************************/
void _putstatic_i()
{
	jref		D_A		index;
	u32			D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	indexAddr	= MEMtPF8(resolveReferenceAddress(index, PCpkgID, COMPONENT_STATICFIELD));
	value		= popInt();


	/* place the value where it belongs... */
	nvmWriteU32Atomic(indexAddr, value);
}

#endif

