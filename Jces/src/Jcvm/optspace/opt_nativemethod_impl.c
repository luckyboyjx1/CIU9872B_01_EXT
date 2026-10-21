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

#if SEC_COUNTER_IN_OS

void arrayCompare()
{
	PM8		D_A		s;
	PM8		D_A		t;
	u16		D_A		length;
	u16		D_A		lengthBak;
	u16		D_A		destOff;
	PM8		D_A		destaddr;
	u16		D_A		srcOff;
	PM8		D_A		srcaddr;
	u8 volatile D_A	cmp;
	u8 volatile	D_A	cmpFlag;
	
	PATCH_JCVM_VM_NATIVEMETHOD_IMPL_008();	
	
	length		= popShort();
	lengthBak   = length;
	destOff		= popShort();
	destaddr	= getJavaObjectDataRef();
	srcOff		= popShort();
	srcaddr		= getJavaObjectDataRef();
	
	SC_InOutSaveBack(destaddr, srcaddr, destOff, srcOff);

	/* Check for src or dest being 'null' */
	/* check for firewall */
	s	= GetArrayDataAndCheckBounds(srcaddr, srcOff, length);
	t	= GetArrayDataAndCheckBounds(destaddr, destOff, length);
		
	cmpFlag = 0;
	cmp = SC_memCompare(t, s, length);
	
	if (length != lengthBak)
	{
		faultDetected();
	}
	
	SC_InOutCheckBack(t, s);
	
	if (ECP_CMP_RESULT_EQUAL == cmp)
	{
		cmpFlag = 0x5A;
		pushShort(0);
	}
	
	if (ECP_CMP_RESULT_SMALLER == cmp)
	{
		cmpFlag = 0xA5;
		pushShort(0xFFFF);
	}
	
	if (ECP_CMP_RESULT_GREATER == cmp)
	{
		cmpFlag = 0x6A;
		pushShort(1);
	}
	
	//double check
	if (ECP_CMP_RESULT_EQUAL == cmp)
	{
		if (cmpFlag != 0x5A)
		{
			faultDetected();
		}
	}
	
	if (ECP_CMP_RESULT_SMALLER == cmp)
	{
		if (cmpFlag != 0xA5)
		{
			faultDetected();
		}
	}
	
	if (ECP_CMP_RESULT_GREATER == cmp)
	{
		if (cmpFlag != 0x6A)
		{
			faultDetected();
		}
	}
}

#endif
