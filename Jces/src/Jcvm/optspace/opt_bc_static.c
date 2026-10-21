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

void _getstatic_a()
{
	jref		D_A		index;
	jref		D_A		value;
	PF8			D_A		indexAddr;

	index		= fetchReference();
	if (U16_AND_U16(index, 0x8000) == 0)
	{
		// removed by WH 2007.7.4 no used , this can be done in the founction getPackageComponentLocation
		//but do like this must be faster
		/* Performance: Do not lookup on packageloc table if in current package */
		
		lastVMPkg = PCpkgID;
		
		if (cachedFieldCompLen >= index)
		{
			indexAddr	= MEMtPF8(U16_A_U32(index, cachedFieldCompLoc));
			value		= nvmReadU16(indexAddr);
			pushReference(value);
			return;
		}
		faultDetected();
	}
	/* Reference is external - resolve using global reference table */
	indexAddr 			= MEMtPF8(getReferenceAddress(index));
	value				= nvmReadU16(indexAddr);
	pushReference(value);
}

#endif
