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
 * This method implements the Java Card 2.1 pop instruction.
 * Format:
 * pop
 *
 * Stack Image:
 * ..., word --> ...

 ******************************************************************************/
void _pop()
{
	JSP		= PX8_S_CONST(JSP, 2);

	//checkPop(JSP);
	
	if (JSP >= FPend)
	{
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
	
}


#endif
