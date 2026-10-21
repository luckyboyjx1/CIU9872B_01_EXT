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

u8 volatile g_doubleCheckFlag = 0;

void _ifeq()
{
	JSP	= PX8_S_CONST(JSP, 2);

	if (JSP >= FPend)
	{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
		u16 value = PXS_R_U16(JSP);
		if (value == 0)
		{
			g_doubleCheckFlag = 0x5A;
		}
		else
		{
			g_doubleCheckFlag = 0xA5;
		}
		if (value == 0)
		{
			if (0x5A == g_doubleCheckFlag)
			{
				_goto();
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			if (0xA5 == g_doubleCheckFlag)
			{
				increace_3BYTE(JPC);
			}
			else
			{
				faultDetected();
			}
		}
#else
		if (PXS_R_U16(JSP) == 0)
		{
			_goto();
		}
		else
		{
			increace_3BYTE(JPC);
		}
#endif

		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifne instruction.
 * Format:
 * ifne
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifne()
{
	JSP	= PX8_S_CONST(JSP, 2);

	if (JSP >= FPend)
	{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
		u16 value = PXS_R_U16(JSP);
		if (value != 0)
		{
			g_doubleCheckFlag = 0x5A;
		}
		else
		{
			g_doubleCheckFlag = 0xA5;
		}
		if (value != 0)
		{
			if (0x5A == g_doubleCheckFlag)
			{
				_goto();
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			if (0xA5 == g_doubleCheckFlag)
			{
				increace_3BYTE(JPC);
			}
			else
			{
				faultDetected();
			}
		}
#else
		if (PXS_R_U16(JSP) != 0)
		{
			_goto();
		}
		else
		{
			increace_3BYTE(JPC);
		}
#endif
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 iflt instruction.
 * Format:
 * iflt
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _iflt()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = U16_AND_U16(popShort(), 0x8000);
	if (0x8000 == value)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (0x8000 == value)
	{
		if (0x5A == g_doubleCheckFlag)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (0xA5 == g_doubleCheckFlag)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (0x8000 == U16_AND_U16(popShort(), 0x8000))
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif	
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifge instruction.
 * Format:
 * ifge
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifge()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = U16_AND_U16(popShort(), 0x8000);
	if (0x8000 != value)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (0x8000 != value)
	{
		if (0x5A == g_doubleCheckFlag)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (0xA5 == g_doubleCheckFlag)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (0x8000 != U16_AND_U16(popShort(), 0x8000))
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif	
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifgt instruction.
 * Format:
 * ifgt
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifgt()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	s16 value = U16tS16(popShort());
	if (value > 0)
	{
		g_doubleCheckFlag = 0x7A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (value > 0)
	{
		if (g_doubleCheckFlag == 0x7A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (U16tS16(popShort()) > 0)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifle instruction.
 * Format:
 * ifle
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifle()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	s16 value = U16tS16(popShort());
	if (value <= 0)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA7;
	}
	if (value <= 0)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (U16tS16(popShort()) <= 0)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifnull instruction.
 * Format:
 * ifnull
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifnull()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = popReference();
	if (value == REF_NULL)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (value == REF_NULL)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (popReference() == REF_NULL)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifnonnull instruction.
 * Format:
 * ifnonnull
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifnonnull()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = popReference();
	if (value != REF_NULL)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (value != REF_NULL)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (popReference() != REF_NULL)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpeq instruction.
 * Format:
 * if_acmpeq
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_acmpeq()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (r1 == r2)
	{
		g_doubleCheckFlag = 0xA7;
	}
	else
	{
		g_doubleCheckFlag = 0x7A;
	}
	if (r1 == r2)
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0x7A)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (r1 == r2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpne instruction.
 * Format:
 * if_acmpne
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_acmpne()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (r1 != r2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0x7A;
	}
	if (r1 != r2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0x7A)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (r1 != r2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpeq instruction.
 * Format:
 * if_scmpeq
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpeq()
{
	u16   D_A    s2;
	u16   D_A    s1;

	JSP	= PX8_S_CONST(JSP, 4);
	s2	= PXS_R_U16(JSP + 2);
	s1	= PXS_R_U16(JSP);
	
	if (JSP >= FPend)
	{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
		if (s1 == s2)
		{
			g_doubleCheckFlag = 0x5A;
		}
		else
		{
			g_doubleCheckFlag = 0xA5;
		}
		if (s1 == s2)
		{
			if (g_doubleCheckFlag == 0x5A)
			{
				_goto();
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			if (g_doubleCheckFlag == 0xA5)
			{
				increace_3BYTE(JPC);
			}
			else
			{
				faultDetected();
			}
		}
#else
		if (s1 == s2)
		{
			_goto();
		}
		else
		{
			increace_3BYTE(JPC);
		}
#endif
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpne instruction.
 * Format:
 * if_scmpne
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpne()
{
	u16   D_A    s2;
	u16   D_A    s1;

	JSP	= PX8_S_CONST(JSP, 4);
	s2	= PXS_R_U16(JSP + 2);
	s1	= PXS_R_U16(JSP);

	if (JSP >= FPend)
	{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
		if (s1 != s2)
		{
			g_doubleCheckFlag = 0x5A;
		}
		else
		{
			g_doubleCheckFlag = 0xA5;
		}
		if (s1 != s2)
		{
			if (g_doubleCheckFlag == 0x5A)
			{
				_goto();
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			if (g_doubleCheckFlag == 0xA5)
			{
				increace_3BYTE(JPC);
			}
			else
			{
				faultDetected();
			}
		}
#else
		if (s1 != s2)
		{
			_goto();
		}
		else
		{
			increace_3BYTE(JPC);
		}
#endif
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmplt instruction.
 * Format:
 * if_scmplt
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmplt()
{
	s16   D_A    s2;
	s16   D_A    s1;

	JSP	= PX8_S_CONST(JSP, 4);
	s2	= PXS_R_U16(JSP + 2);
	s1	= PXS_R_U16(JSP);

	if (JSP >= FPend)
	{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
		if (s1 < s2)
		{
			g_doubleCheckFlag = 0x5A;
		}
		else
		{
			g_doubleCheckFlag = 0xA5;
		}
		if (s1 < s2)
		{
			if (g_doubleCheckFlag == 0x5A)
			{
				_goto();
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			if (g_doubleCheckFlag == 0xA5)
			{
				increace_3BYTE(JPC);
			}
			else
			{
				faultDetected();
			}
		}
#else
		if (s1 < s2)
		{
			_goto();
		}
		else
		{
			increace_3BYTE(JPC);
		}
#endif
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpge instruction.
 * Format:
 * if_scmpge
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpge()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 >= s2)
	{
		g_doubleCheckFlag = 0x7A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 >= s2)
	{
		if (g_doubleCheckFlag == 0x7A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 >= s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmple instruction.
 * Format:
 * if_scmple
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmple()
{
	s16   D_A    s2;
	s16   D_A    s1;

	JSP	= PX8_S_CONST(JSP, 4);
	s2	= PXS_R_U16(JSP + 2);
	s1	= PXS_R_U16(JSP);

	if (JSP >= FPend)
	{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
		if (s1 <= s2)
		{
			g_doubleCheckFlag = 0x5A;
		}
		else
		{
			g_doubleCheckFlag = 0xA5;
		}
		if (s1 <= s2)
		{
			if (g_doubleCheckFlag == 0x5A)
			{
				_goto();
			}
			else
			{
				faultDetected();
			}
		}
		else
		{
			if (g_doubleCheckFlag == 0xA5)
			{
				increace_3BYTE(JPC);
			}
			else
			{
				faultDetected();
			}
		}
#else
		if (s1 <= s2)
		{
			_goto();
		}
		else
		{
			increace_3BYTE(JPC);
		}
#endif
		return;
	}
	else
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpgt instruction.
 * Format:
 * if_scmpgt
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpgt()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 > s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 > s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			increace_3BYTE(JPC);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 > s2)
	{
		_goto();
	}
	else
	{
		increace_3BYTE(JPC);
	}
#endif	
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifeq_w instruction.
 * Format:
 * ifeq_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifeq_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = popShort();
	if (value == 0)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA7;
	}
	if (value == 0)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (popShort() == 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifne_w instruction.
 * Format:
 * ifne_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifne_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = popShort();
	if (value != 0)
	{
		g_doubleCheckFlag = 0xA7;
	}
	else
	{
		g_doubleCheckFlag = 0x5A;
	}
	if (value != 0)
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (popShort() != 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 iflt_w instruction.
 * Format:
 * iflt_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _iflt_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = U16_AND_U16(popShort(), 0x8000);
	if (0x8000 == value)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA7;
	}
	if (0x8000 == value)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (0x8000 == U16_AND_U16(popShort(), 0x8000))
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifge_w instruction.
 * Format:
 * ifge_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifge_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = U16_AND_U16(popShort(), 0x8000);
	if (0x8000 != value)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA7;
	}
	if (0x8000 != value)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (0x8000 != U16_AND_U16(popShort(), 0x8000))
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifgt_w instruction.
 * Format:
 * ifgt_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifgt_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	s16 value = U16tS16(popShort());
	if (value > 0)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (value > 0)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag	== 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (U16tS16(popShort()) > 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifle_w instruction.
 * Format:
 * ifle_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifle_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	s16 value = U16tS16(popShort());
	if (value <= 0)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (value <= 0)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag	== 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (U16tS16(popShort()) <= 0)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifnull_w instruction.
 * Format:
 * ifnull_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifnull_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = popReference();
	if (value == REF_NULL)
	{
		g_doubleCheckFlag = 0xA5;
	}
	else
	{
		g_doubleCheckFlag = 0x5A;
	}
	if (value == REF_NULL)
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (popReference() == REF_NULL)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 ifnonnull_w instruction.
 * Format:
 * ifnonnull_w
 *
 * Stack Image:
 * ..., value --> ...

 ******************************************************************************/
void _ifnonnull_w()
{
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	u16 value = popReference();
	if (value != REF_NULL)
	{
		g_doubleCheckFlag = 0xA5;
	}
	else
	{
		g_doubleCheckFlag = 0x5A;
	}
	if (value != REF_NULL)
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (popReference() != REF_NULL)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpeq_w instruction.
 * Format:
 * if_acmpeq_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_acmpeq_w()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (r1 == r2)
	{
		g_doubleCheckFlag = 0x7A;
	}
	else
	{
		g_doubleCheckFlag = 0xA7;
	}
	if (r1 == r2)
	{
		if (g_doubleCheckFlag == 0x7A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (r1 == r2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_acmpne_w instruction.
 * Format:
 * if_acmpne_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_acmpne_w()
{
	jref   D_A    r2;
	jref   D_A    r1;

	r2 = popReference();
	r1 = popReference();
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (r1 != r2)
	{
		g_doubleCheckFlag = 0x7A;
	}
	else
	{
		g_doubleCheckFlag = 0xA7;
	}
	if (r1 != r2)
	{
		if (g_doubleCheckFlag == 0x7A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA7)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (r1 != r2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpeq_w instruction.
 * Format:
 * if_scmpeq_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpeq_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = popShort();
	s1 = popShort();
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 == s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 == s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 == s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpne_w instruction.
 * Format:
 * if_scmpne_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpne_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = popShort();
	s1 = popShort();
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 != s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 != s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 != s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmplt_w instruction.
 * Format:
 * if_scmplt_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmplt_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 < s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 < s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 < s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpge_w instruction.
 * Format:
 * if_scmpge_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpge_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 >= s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 >= s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 >= s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmpgt_w instruction.
 * Format:
 * if_scmpgt_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmpgt_w()
{
	u16   D_A    s2;
	u16   D_A    s1;

	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 > s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 > s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else
	if (s1 > s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 if_scmple_w instruction.
 * Format:
 * if_scmple_w
 *
 * Stack Image:
 * ..., value1, value2 --> ...

 ******************************************************************************/
void _if_scmple_w()
{
	u16   D_A    s2;
	u16   D_A    s1;
	
	s2 = U16_XOR_U16(popShort(), 0x8000);
	s1 = U16_XOR_U16(popShort(), 0x8000);
#if DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY
	if (s1 <= s2)
	{
		g_doubleCheckFlag = 0x5A;
	}
	else
	{
		g_doubleCheckFlag = 0xA5;
	}
	if (s1 <= s2)
	{
		if (g_doubleCheckFlag == 0x5A)
		{
			_goto_w();
		}
		else
		{
			faultDetected();
		}
	}
	else
	{
		if (g_doubleCheckFlag == 0xA5)
		{
			JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
		}
		else
		{
			faultDetected();
		}
	}
#else	
	if (s1 <= s2)
	{
		_goto_w();
	}
	else
	{
		JPC	= PF8_A_CONST(JPC, SIZE_SHORT_L);
	}
#endif
}

#endif
