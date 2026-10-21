/*****************************************************************************
 * File:  context.c
 * Version 1.4

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,4	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "context.h"
#include "memory.h"
#include "global_variable.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h"

/*****************************************************************************
 * opcode fetching
 *****************************************************************************/
WEAK u8 fetchByte()
{
	u8		D_A		value;

	value	= PF_R_U8(JPC); /* PC is either in E2P or in ROM... */
	PF8_INC(JPC);
	return (value);
}

WEAK u16 fetchShort()
{
	u16		D_A		value;

	value	= PF_R_U16(JPC);
	JPC		= PF8_A_CONST(JPC, SIZE_SHORT_L);
	return (value);
}

#if VM_INTS_SUPPORTED

u32 fetchInt()
{
	u32		D_A		value;

	value	= PF_R_U32(JPC);
	JPC		= PF8_A_CONST(JPC, SIZE_INT_L);
	return (value);
}

#endif

