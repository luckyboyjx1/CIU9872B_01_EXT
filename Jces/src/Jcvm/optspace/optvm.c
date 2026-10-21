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

PM8 getArrayDataWithCheck(u16 outLen)
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
				check_bounds_interval((PF8)outBuff, outOffset, outLen);
				
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


PM8 getJavaObjectDataRef(void)
{
	JSP	= PX8_S_CONST(JSP, 2);
	if (JSP >= FPend)
	{
		return MEMtPM8(locateObjectInMemory(PXS_R_U16(JSP)));
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

#endif
