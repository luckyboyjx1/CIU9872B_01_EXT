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

void _astore()
{
	u8 paramIndex = fetchByte();
//	setLocalReference(fetchByte(), popReference());
	JSP	= PX8_S_CONST(JSP, 2);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
#if JCVM__VM__CHECK_LOCAL
		if(FPsize > paramIndex)
		{
#endif
			PXS_W_U16(PX8_A_U16(FP, U8_M_U8(paramIndex, 2)), PXS_R_U16(JSP));
			return;
#if JCVM__VM__CHECK_LOCAL
		}
		else
		{
			fatal_error(STACK_UNDER_FLOW_ERROR);
		}
#endif
	
#if JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
}
void _astore_0()
{
//	setLocalReference(0, popReference());
	
	JSP	= PX8_S_CONST(JSP, 2);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
#if JCVM__VM__CHECK_LOCAL
		if(FPsize > 0)
		{
#endif
			PXS_W_U16(FP, PXS_R_U16(JSP));
			return;
#if JCVM__VM__CHECK_LOCAL
		}
		else
		{
			fatal_error(STACK_UNDER_FLOW_ERROR);
		}
#endif
	
#if JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
	
}
void _astore_1()
{
//	setLocalReference(1, popReference());
	
	JSP	= PX8_S_CONST(JSP, 2);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
#if JCVM__VM__CHECK_LOCAL
		if(FPsize > 1)
		{
#endif
			PXS_W_U16(PX8_A_U16(FP, 2), PXS_R_U16(JSP));
			return;
#if JCVM__VM__CHECK_LOCAL
		}
		else
		{
			fatal_error(STACK_UNDER_FLOW_ERROR);
		}
#endif
	
#if JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
}
void _astore_2()
{
//	setLocalReference(2, popReference());
	
	JSP	= PX8_S_CONST(JSP, 2);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
#if JCVM__VM__CHECK_LOCAL
		if(FPsize > 2)
		{
#endif
			PXS_W_U16(PX8_A_U16(FP, 4), PXS_R_U16(JSP));
			return;
#if JCVM__VM__CHECK_LOCAL
		}
		else
		{
			fatal_error(STACK_UNDER_FLOW_ERROR);
		}
#endif
	
#if JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
}
void _astore_3()
{
//	setLocalReference(3, popReference());

	JSP	= PX8_S_CONST(JSP, 2);

#if JCVM__VM__CHECK_STACK
	if (JSP >= FPend)
	{
#endif
#if JCVM__VM__CHECK_LOCAL
		if(FPsize > 3)
		{
#endif
			PXS_W_U16(PX8_A_U16(FP, 6), PXS_R_U16(JSP));
			return;
#if JCVM__VM__CHECK_LOCAL
		}
		else
		{
			fatal_error(STACK_UNDER_FLOW_ERROR);
		}
#endif
	
#if JCVM__VM__CHECK_STACK
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
	
}

#endif
