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
#include "optspace.h"
#include "error.h"
#include "bc_control.h"
#include "utilMethod.h"

#if EMV_OPTIMIZATION_CODE


/*****************************************************************************
 * This method implements the Java Card 2.1 sadd instruction.
 * Format:
 * sadd
 *
 * Stack Image:
 * ..., value1, value2 --> result
 *****************************************************************************/
//TODO: check asm code OK
void _sadd()
{
	PX8	  D_A  p;
	
	JSP		= PX8_S_CONST(JSP, 2);
	
	p 		= PX8_S_CONST(JSP, 2);
	
	//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_A_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
		
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/*****************************************************************************
 * This method implements the Java Card 2.1 ssub instruction.
 * Format:
 * ssub
 *
 * Stack Image:
 * ..., value1, value2 --> result = value1 - value1alue2

 *****************************************************************************/
//TODO: check asm code NO
void _ssub()
{
	PX8	  D_A   p;

	JSP		= PX8_S_CONST(JSP, 2);
	
	p 		= PX8_S_CONST(JSP, 2);
	
	//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_S_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 smul instruction.
 * Format:
 * smul
 *
 * Stack Image:
 * ..., value1, value2 --> result
 *****************************************************************************/
//TODO: check asm code OK
void _smul()
{
	PX8	    D_A    p;
	
	JSP		= PX8_S_CONST(JSP, 2);
	
	p 		= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_M_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sneg instruction.
 * Format:
 * sneg
 *
 * Stack Image:
 * ..., value --> ..., result
 *****************************************************************************/
void _sneg()
{
	PX8	    D_A    p;
	
	p		= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_S_U16(0, PXS_R_U16(p)));
	//	INTERPRETER_QUICK1();
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sshl instruction.
 * Format:
 * sshl
 *
 * Stack Image:
 * ..., value1, value2 --> ..., result
 *****************************************************************************/
void _sshl()
{
	PX8	  D_A    p;
	u16	  D_A    temp;

	JSP		= PX8_S_CONST(JSP, 2);

	p 		= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		temp	=	U32t16(U32_SHL_U16(PXS_R_U16(p), U16_AND_U16(PXS_R_U16(JSP), 0x001F)));
		PXS_W_U16(p, temp);
		
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sand instruction.
 * Format:
 * sand
 *
 * Stack Image:
 * ..., value1, value2 --> result

 *****************************************************************************/
void _sand()
{
	PX8		D_A		p;

	JSP		= PX8_S_CONST(JSP, 2);

	p		= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_AND_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sor instruction.
 * Format:
 * sor
 *
 * Stack Image:
 * ..., value1, value2 --> result

 *****************************************************************************/
void _sor()
{
	PX8		D_A		p;

	JSP		= PX8_S_CONST(JSP, 2);

	p		= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_OR_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
//		INTERPRETER_QUICK1();
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sxor instruction.
 * Format:
 * sxor
 *
 * Stack Image:
 * ..., value1, value2 --> result

 *****************************************************************************/
void _sxor()
{
	PX8		D_A		p;

	JSP		= PX8_S_CONST(JSP, 2);

	p		= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U16_XOR_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
		
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sinc instruction.
 * Format:
 * sinc
 *
 * Stack Image:
 * ... --> ...
 */
void _sinc()
{
	PX8			D_A		p;
	u8			D_A		index;
	u8			D_A		constant;

	index = fetchByte();
	constant = fetchByte();

	//checkLocal(index);
	if(FPsize > index)
	{
		p		= PX8_A_U8(FP, U8_M_U8(2, index));
		PXS_W_U16(p, S16_A_S8(PXS_R_U16(p), constant));
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 s2b instruction.
 * Format:
 * s2b
 *
 * Stack Image:
 * ..., value --> ..., result
 *****************************************************************************/
void _s2b()
{
	PX8	    D_A    p;
	
	p			= PX8_S_CONST(JSP, 2);
	
//	checkPop(p);
	if (p >= FPend)
	{
		PXS_W_U16(p, U8tS16(U16t8(PXS_R_U16(p))));
		
		
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
	
}

#endif
