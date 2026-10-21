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
#include "firewall.h"

#if EMV_OPTIMIZATION_CODE


PM8 getArrayDataWithCheck_bounds(void)
{
	PM8		D_A		outBuff;
	u16		D_A		outOffset;
	
	JSP	= PX8_S_CONST(JSP, 2);
	if (JSP >= FPend)
	{
		outOffset = PXS_R_U16(JSP);
		JSP	= PX8_S_CONST(JSP, 2);
		if (JSP >= FPend)
		{
			outBuff = MEMtPM8( locateObjectInMemory( PXS_R_U16(JSP) ) );

			 if (PFtMEM(outBuff)!= REF_NULL) 
			 {
				checkArrayRead(outBuff);
				 
				check_bounds((PF8)outBuff, outOffset, Header_ArrayType_Byte);
				
				outBuff = PM8_A_U16(MEMtPM8(get_array_data((PF8)outBuff)), outOffset);
				
				return outBuff;
			 }
			 else
			 {
				throw_error(NULL_POINTER_EXCEPTION);
			 }
		}
		else
		{
			fatal_error(STACK_OVER_FLOW_ERROR);
		}
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}



void _baload()
{
	PXS_W_U16(JSP, U8tS16(memReadU8(getArrayDataWithCheck_bounds()))); 
	JSP	= PX8_A_CONST(JSP, 2); 
	if (JSP <= Top)		
	{
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/*
 * This method implements the Java Card 2.1 bastore instruction.
 * Format:
 * bastore
 *
 * Stack Image:
 * ..., arrayref, index, value --> ...
 */
void _bastore()
{
	u16		D_A		value;

	JSP	= PX8_S_CONST(JSP, 2);
	if (JSP >= FPend)
	{
		value = U16t8(PXS_R_U16(JSP));
		memWriteU8Atomic(getArrayDataWithCheck_bounds(), value);
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/*
 * This method implements the Java Card 2.1 saload instruction.
 * Format:
 * saload
 *
 * Stack Image:
 * ..., arrayref, index --> ..., value
 */
void _saload()
{
	u16			D_A			index;
	PN8			D_A			arrayaddr;

	index		= popShort();
	arrayaddr	= MEMtPN8(locateObjectInMemory(popReference()));	// pop arrayref

	/* null pointer check and FireWall check */
	if (PFtMEM(arrayaddr)!= REF_NULL) 
	{

#if JCVM__VM__FIRE_WALL
		checkArrayRead(arrayaddr);
#endif

		check_bounds(arrayaddr, index, Header_ArrayType_Short);

		pushShort(memReadU16(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_SHORT, index))));
		
		return;
	}
	throw_error(NULL_POINTER_EXCEPTION);
}

/*
 * This method implements the Java Card 2.1 sastore instruction.
 * Format:
 * sastore
 *
 * Stack Image:
 * ..., arrayref, index, value --> ...
 */
void _sastore()
{
	u16		D_A		value;
	u16		D_A		index;
	PN8		D_A		arrayaddr;

	value		= popShort();
	index		= popShort();
	arrayaddr	= MEMtPN8(locateObjectInMemory(popReference()));

	/* null pointer check and FireWall check */
	if (PFtMEM(arrayaddr)!= REF_NULL) 
	{
#if JCVM__VM__FIRE_WALL
		checkArrayRead(arrayaddr);
#endif

		check_bounds(arrayaddr, index, Header_ArrayType_Short);

		/* place the value where it belongs... */
		memWriteU16Atomic(PN8_A_U32(MEMtPN8(get_array_data(arrayaddr)), U8_M_U16(SIZE_SHORT, index)), value);
		return;
	}
	throw_error(NULL_POINTER_EXCEPTION);
}

#endif
