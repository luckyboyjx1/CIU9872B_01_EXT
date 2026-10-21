/*****************************************************************************
 * File:  bc_load.c
 * Version 1.8

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "bc_load.h"
#include "global_variable.h"
#include "stack.h"
#include "memory.h"
#include "context.h"

/******************************************************************************
 * This method implements the Java Card 2.1 sload instruction.
 * Format:
 * sload
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sload()
{
	pushShort(getLocalShort(fetchByte()));
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 sload_0 instruction.
 * Format:
 * sload_0
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sload_0()
{
	pushShort(getLocalShort(0));
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 sload_1 instruction.
 * Format:
 * sload_1
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sload_1()
{
	pushShort(getLocalShort(1));
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 sload_2 instruction.
 * Format:
 * sload_2
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sload_2()
{
	pushShort(getLocalShort(2));
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 sload_3 instruction.
 * Format:
 * sload_3
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sload_3()
{
	pushShort(getLocalShort(3));
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 aload instruction.
 * Format:
 * aload
 *
 * Stack Image:
 * ... --> objref

 ******************************************************************************/
//void _aload()
//{
//	pushReference(getLocalReference(fetchByte()));
//}

///******************************************************************************
// * This method implements the Java Card 2.1 aload_0 instruction.
// * Format:
// * aload_0
// *
// * Stack Image:
// * ... --> objref

// ******************************************************************************/
//void _aload_0()
//{
//	pushReference(getLocalReference(0));
//}

///******************************************************************************
// * This method implements the Java Card 2.1 aload_1 instruction.
// * Format:
// * aload_1
// *
// * Stack Image:
// * ... --> objref

// ******************************************************************************/
//void _aload_1()
//{
//	pushReference(getLocalReference(1));
//}

///******************************************************************************
// * This method implements the Java Card 2.1 aload_2 instruction.
// * Format:
// * aload_2
// *
// * Stack Image:
// * ... --> objref

// ******************************************************************************/
//void _aload_2()
//{
//	pushReference(getLocalReference(2));
//}

///******************************************************************************
// * This method implements the Java Card 2.1 aload_3 instruction.
// * Format:
// * aload_3
// *
// * Stack Image:
// * ... --> objref

// ******************************************************************************/
//void _aload_3()
//{
//	pushReference(getLocalReference(3));
//}

#if VM_INTS_SUPPORTED

/******************************************************************************
 * This method implements the Java Card 2.1 iload instruction.
 * Format:
 * iload
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _iload()
{
	pushInt(getLocalInt(fetchByte()));
}

/******************************************************************************
 * This method implements the Java Card 2.1 iload_0 instruction.
 * Format:
 * iload_0
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _iload_0()
{
	pushInt(getLocalInt(0));
}

/******************************************************************************
 * This method implements the Java Card 2.1 iload_1 instruction.
 * Format:
 * iload_1
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _iload_1()
{
	pushInt(getLocalInt(1));
}

/******************************************************************************
 * This method implements the Java Card 2.1 iload_2 instruction.
 * Format:
 * iload_2
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _iload_2()
{
	pushInt(getLocalInt(2));
}

/******************************************************************************
 * This method implements the Java Card 2.1 iload_3 instruction.
 * Format:
 * iload_3
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _iload_3()
{
	pushInt(getLocalInt(3));
}

#endif
