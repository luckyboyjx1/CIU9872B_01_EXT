/*****************************************************************************
 * File:  bc_store.c
 * Version 1.8

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "bc_store.h"
#include "global_variable.h"
#include "stack.h"
#include "memory.h"
#include "context.h"
#include "anomaly.h"

/******************************************************************************
 * This method implements the Java Card 2.1 sstore instruction.
 * Format:      sstore index
 *
 * Stack Image:
 * ..., value --> ...
 *
 ******************************************************************************/
//void _sstore()
//{
//	setLocalShort(fetchByte(), popShort());
//}

/******************************************************************************
 * This method implements the Java Card 2.1 sstore_0 instruction.
 * Format:      sstore_0
 *
 * Stack Image:
 * ..., value --> ...
 *
 ******************************************************************************/
//void _sstore_0()
//{
//	setLocalShort(0, popShort());
//}

/******************************************************************************
 * This method implements the Java Card 2.1 sstore_1 instruction.
 * Format:      sstore_1
 *
 * Stack Image:
 * ..., value --> ...
 *
 ******************************************************************************/
//void _sstore_1()
//{
//	setLocalShort(1, popShort());
//}

/******************************************************************************
 * This method implements the Java Card 2.1 sstore_2 instruction.
 * Format:      sstore_2
 *
 * Stack Image:
 * ..., value --> ...
 *
 ******************************************************************************/
//void _sstore_2()
//{
//	setLocalShort(2, popShort());
//}

/******************************************************************************
 * This method implements the Java Card 2.1 sstore_3 instruction.
 * Format:      sstore_3
 *
 * Stack Image:
 * ..., value --> ...
 *
 ******************************************************************************/
//void _sstore_3()
//{
//	setLocalShort(3, popShort());
//}

/******************************************************************************
 * This method implements the Java Card 2.1 astore instruction.
 * Format:      astore index
 *
 * Stack Image:
 * ..., objref --> ...
 *
 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _astore()
{
	setLocalReference(fetchByte(), popReference());
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 astore_0 instruction.
 * Format:      astore_0
 *
 * Stack Image:
 * ..., objref --> ...
 *
 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _astore_0()
{
	setLocalReference(0, popReference());
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 astore_1 instruction.
 * Format:      astore_1
 *
 * Stack Image:
 * ..., objref --> ...
 *
 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _astore_1()
{
	setLocalReference(1, popReference());
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 astore_2 instruction.
 * Format:      astore_2
 *
 * Stack Image:
 * ..., objref --> ...
 *
 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _astore_2()
{
	setLocalReference(2, popReference());
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 astore_3 instruction.
 * Format:      astore_3
 *
 * Stack Image:
 * ..., objref --> ...
 *
 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _astore_3()
{
	setLocalReference(3, popReference());
}
#endif //!EMV_OPTIMIZATION_CODE

#if VM_INTS_SUPPORTED

/******************************************************************************
 * This method implements the Java Card 2.1 istore instruction.
 * Format:      istore index
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ...
 *
 ******************************************************************************/
void _istore()
{
	setLocalInt(fetchByte(), popInt());
}

/******************************************************************************
 * This method implements the Java Card 2.1 istore_0 instruction.
 * Format:      istore_0
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ...
 *
 ******************************************************************************/
void _istore_0()
{
	setLocalInt(0, popInt());
}

/******************************************************************************
 * This method implements the Java Card 2.1 istore_1 instruction.
 * Format:      istore_1
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ...
 *
 ******************************************************************************/
void _istore_1()
{
	setLocalInt(1, popInt());
}

/******************************************************************************
 * This method implements the Java Card 2.1 istore_2 instruction.
 * Format:      istore_2
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ...
 *
 ******************************************************************************/
void _istore_2()
{
	setLocalInt(2, popInt());
}

/******************************************************************************
 * This method implements the Java Card 2.1 istore_3 instruction.
 * Format:      istore_3
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ...
 *
 ******************************************************************************/
void _istore_3()
{
	setLocalInt(3, popInt());
}

#endif
