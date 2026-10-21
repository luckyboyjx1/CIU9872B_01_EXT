/*****************************************************************************
 * File:  bc_return.c
 * Version 1.8

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "bc_return.h"
#include "global_variable.h"
#include "stack.h"
#include "ob_manager.h"

/******************************************************************************
 * This method implements the Java Card 2.1 sreturn instruction.
 * Format:
 * sreturn
 *
 * Stack Image:
 * ..., value --> [empty]

 ******************************************************************************/
//void _sreturn()
//{
//	u16		D_A		value;

//	value	= popShort();

//	popFrame();
//	pushShort(value);
//}

/******************************************************************************
 * This method implements the Java Card 2.1 areturn instruction.
 * Format:
 * areturn
 *
 * Stack Image:
 * ..., objref --> [empty]

 ******************************************************************************/
void _areturn()
{
	jref	D_A		value;

	value = popReference();
	popFrame();
	pushReference(value);
}

/******************************************************************************
 * This method implements the Java Card 2.1 return instruction.
 * Format:
 * return
 *
 * Stack Image:
 * ... --> [empty]

 ******************************************************************************/
void _return()
{
 	popFrame();
}

#if VM_INTS_SUPPORTED

/******************************************************************************
 * This method implements the Java Card 2.1 ireturn instruction.
 * Format:
 * ireturn
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> [empty]

 ******************************************************************************/
void _ireturn()
{
	u32		D_A		value;

	value = popInt();
	popFrame();
	pushInt(value);
}

#endif
