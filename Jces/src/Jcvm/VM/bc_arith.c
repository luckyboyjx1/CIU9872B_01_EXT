/*****************************************************************************
 * File:  bc_arith.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 Original author: David Getreu
 *****************************************************************************/
#include "bc_arith.h"
#include "context.h"
#include "memory.h"
#include "stack.h"
#include "anomaly.h"
#include "global_variable.h"

#include "ecpReadWrite.h"
#include "ecpOperation.h"

/*****************************************************************************
 * This method implements the Java Card 2.1 sadd instruction.
 * Format:
 * sadd
 *
 * Stack Image:
 * ..., value1, value2 --> result
 *****************************************************************************/
//TODO: check asm code OK
#if !EMV_OPTIMIZATION_CODE
WEAK void _sadd()
{
	PX8	  D_A  p;
	
	JSP		= PX8_S_CONST(JSP, 2);
	
	p 		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	PXS_W_U16(p, U16_A_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
}
#endif //!EMV_OPTIMIZATION_CODE

/*****************************************************************************
 * This method implements the Java Card 2.1 ssub instruction.
 * Format:
 * ssub
 *
 * Stack Image:
 * ..., value1, value2 --> result = value1 - value1alue2

 *****************************************************************************/
//TODO: check asm code NO
#if !EMV_OPTIMIZATION_CODE
WEAK void _ssub()
{
	PX8	  D_A   p;

	JSP		= PX8_S_CONST(JSP, 2);
	
	p 		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	PXS_W_U16(p, U16_S_U16(PXS_R_U16(p), PXS_R_U16(JSP)));

}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 smul instruction.
 * Format:
 * smul
 *
 * Stack Image:
 * ..., value1, value2 --> result
 *****************************************************************************/
//TODO: check asm code OK
#if !EMV_OPTIMIZATION_CODE
WEAK void _smul()
{
	PX8	    D_A    p;
	
	JSP		= PX8_S_CONST(JSP, 2);
	
	p 		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	PXS_W_U16(p, U16_M_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sdiv instruction.
 * Format:
 * sdiv
 *
 * Stack Image:
 * ..., value1, value2 --> result
 *****************************************************************************/
//TODO: check asm code NO
void _sdiv()
{
	PX8		D_A    p;
	u16		D_A    temp;

	JSP		= PX8_S_CONST(JSP, 2);

	p 		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	if (0 == PXS_R_U16(JSP))
	{
		throw_error(ARITHMETIC_EXCEPTION);
	}
	
	temp =  S16tU16(S16_D_S16(PXS_R_U16(p), PXS_R_U16(JSP)));
	PXS_W_U16(p,temp);
}

/********************************************************************************
 * This method implements the Java Card 2.1 srem instruction.
 * Format:
 * srem
 *
 * Stack Image:
 * ..., value1, value2 --> result
 *****************************************************************************/
//TODO: check asm code NO
void _srem()
{
	PX8		D_A    p;
	u16 	D_A    temp;

	JSP		= PX8_S_CONST(JSP, 2);

	p 		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	if (0 == PXS_R_U16(JSP))
	{
		throw_error(ARITHMETIC_EXCEPTION);
	}
	temp	=	S16tU16(S16_R_S16(PXS_R_U16(p), PXS_R_U16(JSP)));
	PXS_W_U16(p, temp);
}

/********************************************************************************
 * This method implements the Java Card 2.1 sneg instruction.
 * Format:
 * sneg
 *
 * Stack Image:
 * ..., value --> ..., result
 *****************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sneg()
{
	PX8	    D_A    p;
	
	p		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);
	
	PXS_W_U16(p, U16_S_U16(0, PXS_R_U16(p)));
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sshl instruction.
 * Format:
 * sshl
 *
 * Stack Image:
 * ..., value1, value2 --> ..., result
 *****************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sshl()
{
	PX8	  D_A    p;
	u16	  D_A    temp;

	JSP		= PX8_S_CONST(JSP, 2);

	p 		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	temp	=	U32t16(U32_SHL_U16(PXS_R_U16(p), U16_AND_U16(PXS_R_U16(JSP), 0x001F)));
	
	PXS_W_U16(p, temp);
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sshr instruction.
 * Format:
 * sshr
 *
 * Stack Image:
 * ..., value1, value2 --> ..., result
 *****************************************************************************/
void _sshr()
{
	s16			D_A		value2;
	s16			D_A		value1;

	u8			D_A		temp;
	value2		= (s16) popShort();
	value1		= (s16) popShort();

	temp =  U16t8(U16_AND_U16(value2, 0x001f));
	if(value1  < 0)
	{
		//pushShort( U16_INV(U16_SHR_U8(U16_INV(value1), temp)));
		//pushShort((u16)(~((~value1)>> temp)));
		pushShort(S16_INV( S16_SHR_U8(S16_INV(value1), temp)));
	}
	else
	{
		pushShort(U32_SHR_U16(value1, temp));
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 sushr instruction.
 * Format:
 * sushr
 *
 * Stack Image:
 * ..., value1, value2 --> ..., result
 *****************************************************************************/
#pragma SAVE_OT	                            
#pragma NO_OT
void _sushr()
{

	u16			D_A		value2;
	u16			D_A		value1;
	PX8			D_A		p;
	u16			D_A		temp;

	JSP		= PX8_S_CONST(JSP, 2);

	value2	= U16_AND_U16(PXS_R_U16(JSP), 0x001F);
	
	p		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);
	
	value1	= PXS_R_U16(p);
	
	if (0x8000 != U16_AND_U16(value1, 0x8000))
	{
		temp	=	U32t16(U16_SHR_U16(value1, value2));
	}
	else
	{
		if (value2 <= 16)
		{
			/* Equivalent to signed shift */
			temp	=	U32t16(~U16_SHR_U16(~value1, value2));
		}
		else
		{
			/* Equivalent to unsigned shift of 0xffff by (value2 - 16) positions */
			temp	=	U32t16(U16_SHR_U16(0xffff, U16_S_U8(value2, 16)));
		}
	}
	PXS_W_U16(p, temp);
}

#pragma RESTORE_OT
/********************************************************************************
 * This method implements the Java Card 2.1 sand instruction.
 * Format:
 * sand
 *
 * Stack Image:
 * ..., value1, value2 --> result

 *****************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sand()
{
	PX8		D_A		p;

	JSP		= PX8_S_CONST(JSP, 2);

	p		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	PXS_W_U16(p, U16_AND_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sor instruction.
 * Format:
 * sor
 *
 * Stack Image:
 * ..., value1, value2 --> result

 *****************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sor()
{
	PX8		D_A		p;

	JSP		= PX8_S_CONST(JSP, 2);

	p		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	PXS_W_U16(p, U16_OR_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sxor instruction.
 * Format:
 * sxor
 *
 * Stack Image:
 * ..., value1, value2 --> result

 *****************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _sxor()
{
	PX8		D_A		p;

	JSP		= PX8_S_CONST(JSP, 2);

	p		= PX8_S_CONST(JSP, 2);
	
	checkPop(p);

	PXS_W_U16(p, U16_XOR_U16(PXS_R_U16(p), PXS_R_U16(JSP)));
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sinc instruction.
 * Format:
 * sinc
 *
 * Stack Image:
 * ... --> ...
 */
#if !EMV_OPTIMIZATION_CODE
WEAK void _sinc()
{
	PX8			D_A		p;
	u8			D_A		index;
	u8			D_A		constant;

	index = fetchByte();
	constant = fetchByte();

	checkLocal(index);
	p		= PX8_A_U8(FP, U8_M_U8(2, index));
	PXS_W_U16(p, S16_A_S8(PXS_R_U16(p), constant));
}
#endif //!EMV_OPTIMIZATION_CODE

/********************************************************************************
 * This method implements the Java Card 2.1 sinc_w instruction.
 * Format:
 * sinc_w
 *
 * Stack Image:
 * ... --> ...
 *****************************************************************************/
void _sinc_w()
{
	/* Assume byte code has been verified */
	u8			D_A		index;
	u16			D_A		constant;
	PX8			D_A		p;

	index		= fetchByte();
	constant	= fetchShort();

	checkLocal(index);
	p			= PX8_A_U8(FP, U8_M_U8(2, index));

	PXS_W_U16(p, U16_A_U16(PXS_R_U16(p), constant));
}

/********************************************************************************
 * This method implements the Java Card 2.1 s2b instruction.
 * Format:
 * s2b
 *
 * Stack Image:
 * ..., value --> ..., result
 *****************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _s2b()
{
	PX8	    D_A    p;
	
	p			= PX8_S_CONST(JSP, 2);
	
	checkPop(p);
	
	PXS_W_U16(p, U8tS16(U16t8(PXS_R_U16(p))));
}
#endif //!EMV_OPTIMIZATION_CODE

#if VM_INTS_SUPPORTED

/********************************************************************************
 * This method implements the Java Card 2.1 iadd instruction.
 * Format:
 * iadd
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2

 *****************************************************************************/
void _iadd()
{
	pushInt(U32_A_U32(popInt(), popInt()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 isub instruction.
 * Format:
 * isub
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _isub()
{
	u32   D_A     value2 ;
	u32   D_A     value1 ;
	value2 = popInt();
	value1 = popInt();
	pushInt(U32_S_U32(value1, value2));
}

/********************************************************************************
 * This method implements the Java Card 2.1 imul instruction.
 * Format:
 * imul
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _imul()
{
	pushInt(U32_M_U32(popInt(), popInt()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 idiv instruction.
 * Format:
 * idiv
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2

 *****************************************************************************/
void _idiv()
{
	u32			D_A		divisor;
	u32			D_A		dividend;

	divisor		= popInt();
	dividend	= popInt();

	if (divisor == 0)
	{
		throw_error(ARITHMETIC_EXCEPTION);

	}
	else if ((dividend == JINT_MIN) && (divisor == (0xFFFFFFFF)))
	{
		//    /* overflow but no expection per JCVM JSPec */
		pushInt(dividend);
	}
	else
	{
		pushInt(S32_D_S32(dividend, divisor));
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 irem instruction.
 * Format:
 * irem
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _irem()
{
	u32			D_A		divisor;
	u32			D_A		dividend;

	divisor		= popInt();
	dividend	= popInt();

	if (divisor == 0)
	{
		throw_error(ARITHMETIC_EXCEPTION);

	}
	else if ((dividend == JINT_MIN) && ( divisor == (0xFFFFFFFF)))
	{
		pushInt(0);
	}
	else
	{
		pushInt(S32_R_S32(dividend, divisor));
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 ineg instruction.
 * Format:
 * ineg
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ..., result.word1, result.word2
 *****************************************************************************/
void _ineg()
{
	pushInt(-U32tS32(popInt()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 ishl instruction.
 * Format:
 * ishl
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _ishl()
{
	u32			D_A		value2;
	u32			D_A		value1;

	value2		= popInt();
	value1		= popInt();

	pushInt(U32_SHL_U8(value1, U8_AND_U8(value2, 0x1f)));
}

/********************************************************************************
 * This method implements the Java Card 2.1 ishr instruction.
 * Format:
 * ishr
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _ishr()
{
	u32			D_A		value2;
	u32			D_A		value1;

	value2		= popInt();
	value1		= popInt();

	if(value1 & INT_SIGN)
	{
		pushInt(~U32_SHR_U8(~value1, U8_AND_U8(value2, 0x1f)));
	}
	else
	{
		pushInt(U32_SHR_U8(value1, U8_AND_U8(value2, 0x1f)));
	}
}

/********************************************************************************
 * This method implements the Java Card 2.1 iushr instruction.
 * Format:
 * iushr
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _iushr()
{
	u32			D_A		value2;
	u32			D_A		value1;

	value2		= popInt();
	value1		= popInt();

	/* Shift to the right by (value2 & 0x1f) bit positions with zero extension */
	pushInt(U32tS32(U32_SHR_U8(value1, U8_AND_U8(value2, 0x1f))));
}

/********************************************************************************
 * This method implements the Java Card 2.1 iand instruction.
 * Format:
 * iand
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2

 *****************************************************************************/
void _iand()
{
	pushInt(U32_AND_U32(popInt() , popInt()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 ior instruction.
 * Format:
 * ior
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2

 *****************************************************************************/
void _ior()
{
	pushInt(U32_OR_U32(popInt(), popInt()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 ixor instruction.
 * Format:
 * ixor
 *
 * Stack Image:
 * ..., value1.word1, value1.word2, value2.word1, value2.word2 --> result.word1, result.word2
 *****************************************************************************/
void _ixor()
{
	pushInt(U32_XOR_U32(popInt(), popInt()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 iinc instruction.
 * Format:
 * iinc
 *
 * Stack Image:
 * ... --> ...
 *****************************************************************************/
void _iinc()
{
	/* Assumption here that the Index is valid for this frame (Converter checked...) */
	u8			D_A		index;
	u8			D_A		constant;

	index		= fetchByte();
	constant	= fetchByte();

	setLocalInt(index, S32_A_S8(getLocalInt(index), constant));
}

/********************************************************************************
 * This method implements the Java Card 2.1 iinc_w instruction.
 * Format:
 * iinc_w
 *
 * Stack Image:
 * ... --> ...
 *****************************************************************************/
void _iinc_w()
{
	/* Assumption here that the Index is valid for this frame (Converter checked...) */
	u8 			D_A		index;
	u16			D_A		constant;

	index		= fetchByte();
	constant	= fetchShort();

	setLocalInt(index, S32_A_S16(getLocalInt(index), constant));
}

/********************************************************************************
 * This method implements the Java Card 2.1 s2i instruction.
 * Format:
 * s2i
 *
 * Stack Image:
 * ..., value --> ..., result
 *****************************************************************************/
void _s2i()
{
	pushInt(U16tS32(popShort()));
}

/********************************************************************************
 * This method implements the Java Card 2.1 i2b instruction.
 * Format:
 * i2b
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ..., result

 *****************************************************************************/
void _i2b()
{
	pushShort(U8tS16(U32t8(U32_AND_U32(popInt(), C_BYTE_SIZE))));
}

/********************************************************************************
 * This method implements the Java Card 2.1 i2s instruction.
 * Format:
 * i2s
 *
 * Stack Image:
 * ..., value.word1, value.word2 --> ..., result

 *****************************************************************************/
void _i2s()
{
	pushShort(U32t16(popInt()));
}

#endif
