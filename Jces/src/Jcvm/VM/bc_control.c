/*****************************************************************************
 *  File:  bc_control.c
 *  Version 1.10

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 ****************************************************************************/
#include "bc_control.h"
#include "context.h"
#include "global_variable.h"
#include "memory.h"
#include "stack.h"
#include "object.h"
#include "except.h"
#include "firewall.h"
#include "mask.h"
#include "packageloc.h"
#include "ob_manager.h"
#include "opcode.h"

/******************************************************************************
 * This method implements the Java Card 2.1 nop instruction.
 * Format:
 * nop
 *
 * Stack Image:
 * ... --> ...

 ******************************************************************************/
void _nop()
{
	if(PF_R_U8(JPC -1) != _NOP)
	{
		faultDetected();
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 athrow instruction.
 * Format:
 * athrow
 *
 * Stack Image:
 * ..., objref --> ..., objref
 ******************************************************************************/
void _athrow()
{
	jref		D_A		objref;
	PN8			D_A		objcetAddr;


	/* Get reference to the exception */
	objref		= popReference();

	objcetAddr	= MEMtPN8(locateObjectInMemory(objref));


	/* Do the check to see if the Object Reference is NULL or is an instance of the Throwable Class */
	if (check_addr_null(objcetAddr))
	{
		/* throw NullPointerException instead */
		objref		= PN_R_U16(MEMtPN8(OBJECT_NullPointerException_OFFSET));
		objcetAddr	= MEMtPN8(locateObjectInMemory(objref));
	}
	else
	{


#if JCVM__VM__FIRE_WALL
		checkEntryPoint(objcetAddr); /* Security Exception */
#endif

/*下面java层未生成，先注释，但不能去掉
#if JCVM__VM__NOT_TRUSTED
		if (!isinstanceof(objcetAddr, AType_Object, java_lang_Throwable_CLASS_OFFSET))
		{
			fatal_error(THROW_NON_THROWABLE);
		}
#endif
		*/
	}


	/* Object is an instance of Throwable or one it's subclasses */
	handleException(objref, objcetAddr);
}

/*******************************************************************************
 *      branching bytecodes
 *
 *      These BCs branch the PC to a new location.  The branch locations
 *      are all PC-relative.  The base PC is obtained from (PC - 2).
 *      The -1 is there because the PC has already been advanced to point
 *      to the next bytecode.
 *
 *      wide branching bytecodes
 *
 *      These BCs branch the PC to a new location.  The branch locations
 *      are all PC-relative.  The base PC is obtained from (PC - 2).
 *      The -1 is there because the PC has already been advanced to point
 *      to the next bytecode.
 ******************************************************************************/

/******************************************************************************
 * This method implements the Java Card 2.1 ifeq instruction.
 * Format:
 * ifeq
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifeq()
{
	if (popShort() == 0)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifne instruction.
 * Format:
 * ifne
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifne()
{
	if (popShort() != 0)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 iflt instruction.
 * Format:
 * iflt
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _iflt()
{
	if (0x8000 == U16_AND_U16(popShort(), 0x8000))
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifge instruction.
 * Format:
 * ifge
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifge()
{
	if (0x8000 != U16_AND_U16(popShort(), 0x8000))
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifgt instruction.
 * Format:
 * ifgt
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifgt()
{
	if (U16tS16(popShort()) > 0)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifle instruction.
 * Format:
 * ifle
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifle()
{
	if (U16tS16(popShort()) <= 0)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifnull instruction.
 * Format:
 * ifnull
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifnull()
{
	if (popReference() == REF_NULL)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifnonnull instruction.
 * Format:
 * ifnonnull
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifnonnull()
{
	if (popReference() != REF_NULL)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpeq instruction.
 * Format:
 * if_acmpeq
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_acmpeq()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();

	if (r1 == r2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpne instruction.
 * Format:
 * if_acmpne
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_acmpne()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();

	if (r1 != r2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpeq instruction.
 * Format:
 * if_scmpeq
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpeq()
{
	u16   D_A    s2;
	u16   D_A    s1;
 
	s2 = popShort();
	s1 = popShort();

	if (s1 == s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpne instruction.
 * Format:
 * if_scmpne
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpne()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = popShort();
	s1 = popShort();

	if (s1 != s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmplt instruction.
 * Format:
 * if_scmplt
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmplt()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 < s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpge instruction.
 * Format:
 * if_scmpge
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpge()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 >= s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmple instruction.
 * Format:
 * if_scmple
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmple()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 <= s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpgt instruction.
 * Format:
 * if_scmpgt
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpgt()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 > s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifeq_w instruction.
 * Format:
 * ifeq_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifeq_w()
{
	if (popShort() == 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifne_w instruction.
 * Format:
 * ifne_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifne_w()
{
	if (popShort() != 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 iflt_w instruction.
 * Format:
 * iflt_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _iflt_w()
{
	if (0x8000 == U16_AND_U16(popShort(), 0x8000))
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifge_w instruction.
 * Format:
 * ifge_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifge_w()
{
	if (0x8000 != U16_AND_U16(popShort(), 0x8000))
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifgt_w instruction.
 * Format:
 * ifgt_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifgt_w()
{
	if (U16tS16(popShort()) > 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifle_w instruction.
 * Format:
 * ifle_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifle_w()
{
	if (U16tS16(popShort()) <= 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifnull_w instruction.
 * Format:
 * ifnull_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifnull_w()
{
	if (popReference() == REF_NULL)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 ifnonnull_w instruction.
 * Format:
 * ifnonnull_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _ifnonnull_w()
{
	if (popReference() != REF_NULL)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpeq_w instruction.
 * Format:
 * if_acmpeq_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_acmpeq_w()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();

	if (r1 == r2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpne_w instruction.
 * Format:
 * if_acmpne_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_acmpne_w()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();

	if (r1 != r2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpeq_w instruction.
 * Format:
 * if_scmpeq_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpeq_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = popShort();
	s1 = popShort();

	if (s1 == s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpne_w instruction.
 * Format:
 * if_scmpne_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpne_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = popShort();
	s1 = popShort();

	if (s1 != s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmplt_w instruction.
 * Format:
 * if_scmplt_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmplt_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 < s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpge_w instruction.
 * Format:
 * if_scmpge_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpge_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 >= s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS
/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpgt_w instruction.
 * Format:
 * if_scmpgt_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmpgt_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 > s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmple_w instruction.
 * Format:
 * if_scmple_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
#if !SEC_COUNTER_IN_OS
WEAK void _if_scmple_w()
{
	u16   D_A    s2;
	u16   D_A    s1;
	
	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);

	if (s1 <= s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
}
#endif //!SEC_COUNTER_IN_OS

/******************************************************************************
 * This method implements the Java Card 2.1 goto instruction.
 * Format:
 * goto
 *
 * Stack Image:
 * ... --> ...

 ******************************************************************************/
void _goto()
{
	//  memref data basePC = (memref) (PC - 1L);
	//  s8   data offset = (s8)fetchByte();
	//  PC = basePC + offset;
	s16   D_A     offset;

	offset	= S16_S_S8(U8tS16(PF_R_U8(JPC)), 1);
	JPC		= PF8_A_S16(JPC, offset);
 }

/******************************************************************************
 * This method implements the Java Card 2.1 goto_w instruction.
 * Format:
 * goto_w
 *
 * Stack Image:
 * ... --> ...

 ******************************************************************************/
void _goto_w()
{
	s16   D_A    offset;

	offset	= S16_S_S8(U16tS16(PF_R_U16(JPC)), 1);
	JPC		= PF8_A_S16(JPC, offset);
}

/******************************************************************************
 * This method implements the Java Card 2.1 jsr instruction.
 * Format:
 * jsr
 *
 * Stack Image:
 * ... --> ..., address

 ******************************************************************************/
void _jsr()
{
	s16   D_A    offset;

	pushReference(U8_A_U16(2, U32t16(U32_S_U32(PFtMEM(JPC), cachedMethodCompLoc))));

	offset	= S16_S_S8(U16tS16(PF_R_U16(JPC)), 1);
	JPC		= PF8_A_S16(JPC, offset);
}

/******************************************************************************
 * This method implements the Java Card 2.1 ret instruction.
 * Format:
 * ret
 *
 * Stack Image:
 * ... --> ...
 ******************************************************************************/
void _ret()
{
	JPC		= PF8_A_U16(MEMtPF8(cachedMethodCompLoc), getLocalReference(PF_R_U8(JPC)));
}

/******************************************************************************
 * This method implements the Java Card 2.1 stableswitch instruction.
 * Format:
 * stableswitch
 *
 * Stack Image:
 * ..., index --> ...

 ******************************************************************************/
void _stableswitch()
{
	s16 		D_A		defaultOffset;
	s16 		D_A		low;
	s16 		D_A		high;
	s16			D_A		index;
	PF8			D_A		basePC;
	
	basePC			= JPC;
	defaultOffset	= U16tS16(fetchShort());
	low				= U16tS16(fetchShort());
	high			= U16tS16(fetchShort());
	index			= U16tS16(popShort());
	
	if ((index < low) || (index > high))
	{
		JPC		= PF8_A_S16(basePC, S16_S_S8(defaultOffset, 1));
	}
	else
	{
		JPC		= PF8_A_S16(basePC, S32_A_S8(S8_M_S16(2, S16_S_S16(index, low)), 6));
		JPC		= PF8_A_S16(basePC, S16_S_S16(U16tS16(PF_R_U16(JPC)), 1));
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 slookupswitch instruction.
 * Format:
 * slookupswitch
 *
 * Stack Image:
 * ..., key --> ...

 ******************************************************************************/
void _slookupswitch()
{
	s16		D_A		defaultOffset;
	u16		D_A		npairs;
	u16		D_A		match;
	s16		D_A		offset;
	u16		D_A		key;
	PF8		D_A		basePC;
	
	basePC = JPC;
	defaultOffset 	= U16tS16(fetchShort());
	npairs 			= fetchShort();
	key = popShort();

	while (npairs)
	{
		match 		= fetchShort();
		offset 		= U16tS16(fetchShort());
		if (key == match)
		{
		  	JPC = PF8_A_S16(basePC, S16_S_S8(offset, 1));
		  	return;
		}
		U16_DEC(npairs);
	}
	
	JPC	= PF8_A_S16(basePC, S16_S_S8(defaultOffset, 1));
	return ;
}

#if VM_INTS_SUPPORTED

/******************************************************************************
 * This method implements the Java Card 2.1 icmp instruction.
 * Format:
 * icmp
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> ..., result

 ******************************************************************************/
void _icmp()
{
	u16		D_A		result;
	u32 	D_A		i2;
	u32 	D_A		i1;

	i2 = U32_XOR_U32(popInt(), 0x80000000);
	i1 = U32_XOR_U32(popInt(), 0x80000000);

	if (i1 > i2)
	{
		result = 1;
	}
	else if (i1 < i2)
	{
		result = 0xFFFF;
	}
	else
	{
		result = 0;
	}
	pushShort(result);
}

/******************************************************************************
 * This method implements the Java Card 2.1 itableswitch instruction.
 * Format:
 * itableswitch
 *
 * Stack Image:
 * ..., index --> ...

 ******************************************************************************/
void _itableswitch()
{
	s16		D_A		defaultOffset;
	s32		D_A		low, high, index;


	/*
	 Note that the order of the next five lines is important
	 to the correct operation of this routine
	 */
	PF8		D_A		basePC;

	basePC			= PF8_S_CONST(JPC, 1);
	defaultOffset	= U16tS16(fetchShort());
	low				= U32tS32(fetchInt());
	high			= U32tS32(fetchInt());
	index			= U32tS32(popInt());

	if((index < low) || (index > high))
	{
		JPC	= PF8_A_S16(basePC, defaultOffset);
	}
	else
	{
		JPC	= PF8_A_S16(JPC, S8_M_S32(SIZE_SHORT, S32_S_S32(index, low))); /* offset into jump table */
		defaultOffset =U16tS16(fetchShort());
		JPC = PF8_A_S16(basePC, defaultOffset);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 ilookupswitch instruction.
 * Format:
 * ilookupswitch
 *
 * Stack Image:
 * ..., key.word1, key.word1 --> ...

 ******************************************************************************/
void _ilookupswitch()
{
	s16		D_A		defaultOffset, offset;
	u16		D_A		npairs;
	u32		D_A		key, match;
	u32		D_A		i;
	u1		B_A		noMatch;


	/* subtract one because PC points to NEXT bytecode */
	PF8		D_A		basePC;

	noMatch			= true;
	basePC			= PF8_S_CONST(JPC, 1);
	defaultOffset	=  U16tS16(fetchShort());
	npairs			= fetchShort();
	key				= popInt();

	for (i = 0; i < npairs; U32_INC(i))
	{
		match		= fetchInt();
		offset		= U16tS16(fetchShort());
		if (key == match)
		{
			JPC		= PF8_A_S16(basePC, offset); /* branch offset */
			noMatch = false;
			break;
		}
	}
	if (noMatch)
	{
		JPC	= PF8_A_S16(basePC, defaultOffset);
	}
}

#endif

