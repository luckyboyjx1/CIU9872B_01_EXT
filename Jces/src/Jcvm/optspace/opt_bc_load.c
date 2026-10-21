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


/******************************************************************************
 * This method implements the Java Card 2.1 sload instruction.
 * Format:
 * sload
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
void _sload()
{
	//	pushShort(getLocalShort(fetchByte()));
	
	u8		D_A		offset;

	offset	= PF_R_U8(JPC); /* PC is either in E2P or in ROM... */
	PF8_INC(JPC);
	if(FPsize > offset)
	{
		PXS_W_U16(JSP, PXS_R_U16(PX8_A_U16(FP, U8_M_U8(2, offset))));
		JSP	= PX8_A_CONST(JSP, 2);   //wzq: 	JSP++;
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 sload_0 instruction.
 * Format:
 * sload_0
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
void _sload_0()
{
	//	pushShort(getLocalShort(0));
	if(FPsize > 0)
	{
		PXS_W_U16(JSP, PXS_R_U16(FP));
		JSP	= PX8_A_CONST(JSP, 2);   //wzq: 	JSP++;
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 sload_1 instruction.
 * Format:
 * sload_1
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
void _sload_1()
{
	//	pushShort(getLocalShort(1));
	if(FPsize > 1)
	{
		PXS_W_U16(JSP, PXS_R_U16(PX8_A_U16(FP, 2)));
		JSP	= PX8_A_CONST(JSP, 2);   //wzq: 	JSP++;
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 sload_2 instruction.
 * Format:
 * sload_2
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
void _sload_2()
{
	//	pushShort(getLocalShort(2));
	if(FPsize > 2)
	{
		PXS_W_U16(JSP, PXS_R_U16(PX8_A_U16(FP, 4)));
		JSP	= PX8_A_CONST(JSP, 2);   //wzq: 	JSP++;
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 sload_3 instruction.
 * Format:
 * sload_3
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
void _sload_3()
{
	//	pushShort(getLocalShort(3));
	if(FPsize > 3)
	{
		PXS_W_U16(JSP, PXS_R_U16(PX8_A_U16(FP, 6)));
		JSP	= PX8_A_CONST(JSP, 2);   //wzq: 	JSP++;
		return;
	}
	else
	{
		fatal_error(STACK_UNDER_FLOW_ERROR);
	}
}

#endif
