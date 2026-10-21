/*****************************************************************************
 * File:  bc_stack.c
 * Version 1.11

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/

#include <context.h>
#include <ecpDefine.h>
#include <ecpOperation.h>
#include <ecpReadWrite.h>
#include <error.h>
#include <global_variable.h>
#include <profile.h>
#include <stack.h>
#include "anomaly.h"
#include "bc_stack.h"

//#include "memory.h"

/******************************************************************************
 * This method implements the Java Card 2.1 pop instruction.
 * Format:
 * pop
 *
 * Stack Image:
 * ..., word --> ...

 ******************************************************************************/
#if !EMV_OPTIMIZATION_CODE
WEAK void _pop()
{
	JSP		= PX8_S_CONST(JSP, 2);

	checkPop(JSP);
}
#endif //!EMV_OPTIMIZATION_CODE

/******************************************************************************
 * This method implements the Java Card 2.1 pop2 instruction.
 * Format:
 * pop2
 *
 * Stack Image:
 * ..., word2, word1 --> ...

 ******************************************************************************/
void _pop2()
{
	JSP		= PX8_S_CONST(JSP, 4);
	
	checkPop(JSP);
}

/******************************************************************************
 * This method implements the Java Card 2.1 dup instruction.
 * Format:
 * dup
 *
 * Stack Image:
 * ..., word --> ..., word, word

 ******************************************************************************/
void _dup()
{
	PX8	  D_A    p;
	
	p	= PX8_S_CONST(JSP, 2);
	checkPop(p);
	
	PXS_W_U16(JSP, PXS_R_U16(p));

	JSP	= PX8_A_CONST(JSP, 2);
#if JCVM__VM__CHECK_STACK
	if (JSP > Top) 
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif

}

/******************************************************************************
 * This method implements the Java Card 2.1 dup2 instruction.
 * Format:
 * dup2
 *
 * Stack Image:
 * ..., word2, word1 --> ..., word2, word1, word2, word1

 ******************************************************************************/
void _dup2()
{
	PX8  D_A 	p;
	
	p	= PX8_S_CONST(JSP, 4);
	checkPop(p);
	
	PXS_W_U16(JSP, PXS_R_U16(p));
	PXS_W_U16(PX8_A_CONST(JSP, 2), PXS_R_U16(PX8_S_CONST(JSP, 2)));

	JSP		= PX8_A_CONST(JSP, 4);
#if JCVM__VM__CHECK_STACK
	if (JSP > Top)
	{
		fatal_error(STACK_OVER_FLOW_ERROR);
	}
#endif
}

/******************************************************************************
 * This method implements the Java Card 2.1 dup_x instruction.
 * Format:
 * dup_x
 *
 * Stack Image:
 * ..., wordN, ..., wordM, ..., word1 --> ..., wordM, ..., word1, wordN, ..., wordM, ..., word1

 ******************************************************************************/
void _dup_x()
{
	u8			D_A		bytesDepth;
	u8			D_A		M;
	u8			D_A		N;
	u16			D_A		N_depth[8]; /* rule is no more than (M_bytes + 4) bytes */
	u8			D_A		i;				  // i must be s8

	bytesDepth	= fetchByte();
	M = U8_SHR_U8(U8_AND_U8(bytesDepth, HI_NIBBLE), NIBBLE_SHIFT);
	N = U8_AND_U8(bytesDepth, LO_NIBBLE);


#if JCVM__VM__NOT_TRUSTED
#if VM_INTS_SUPPORTED
	/* check 1 <= M <= 4, M <= N <= M+4 or N == 0 */
	if (!(1 <= M && M <= 4 && ((N == 0) || (M <= N && N <= U8_A_U8(M,4)))))
	{
		fatal_error(BAD_DUPX_MN_VALUE_ERROR);
	}
#else  /* ints are not supported */
	/* check 1 <= M <= 2, M <= N <= M+2 or N == 0 */
	if (!(1 <= M && M <= 2) && ((N == 0) || (M <= N && N <= U8_A_U8( M,2))))
	{
		fatal_error(BAD_DUPX_MN_VALUE_ERROR);
	}
#endif /* VM_INTS_SUPPORTED */
#endif /* NOT_TRUSTED */

	if (N == 0)
	{
		/* remove M elements */
		for (i = 0; i < M; U8_INC(i))
		{
			N_depth[i] = popShort();
		}

		/* push the M elements onto the stack */
		for (i = U8_S_U8(M, 1); 0x80 != U8_AND_U8(i, 0x80); U8_DEC(i))
		{
			pushShort(N_depth[i]);
		}


		/* push back the M elements onto the M elements... */
		for (i = U8_S_U8(M, 1); 0x80 != U8_AND_U8(i, 0x80); U8_DEC(i))
		{
			pushShort(N_depth[i]);
		}
	}
	else
	{
		/* remove N elements */
		for (i = 0; i < N; U8_INC(i))
		{
			N_depth[i] = popShort();
		}

		/* push the first M elements onto the stack */
		for (i = U8_S_U8(M, 1); 0x80 != U8_AND_U8(i, 0x80); U8_DEC(i))
		{
			pushShort(N_depth[i]);
		}


		/* push back the N elements onto the M elements... */
		for (i = U8_S_U8(N, 1); 0x80 != U8_AND_U8(i, 0x80); U8_DEC(i))
		{
			pushShort(N_depth[i]);
		}
	}
}

/* ******************************************************************************
 * This method implements the Java Card 2.1 swap_x instruction.
 * Format:
 * swap_x
 *
 * Stack Image:
 * ..., wordM+N, ..., wordM_1, wordM, ..., word1 --> ..., wordM, ..., word1, wordM+N, ..., wordM+1

 * ******************************************************************************/
void _swap_x()
{
	u8		D_A	bytesDepth;
	u8		D_A	M;
	u8		D_A	N;

	u16		D_A	N_depth[4];
	u8		D_A	i;

	bytesDepth	= fetchByte();
	M = U8_SHR_U8(U8_AND_U8(bytesDepth, HI_NIBBLE), NIBBLE_SHIFT);
	N = U8_AND_U8(bytesDepth, LO_NIBBLE);


#if JCVM__VM__NOT_TRUSTED
#if VM_INTS_SUPPORTED
	/* check M and N are both >= 1 and <= 2 */
	if (M>2 || M<1 || N>2 || N<1)
	{
		fatal_error(BAD_SWAPX_MN_VALUE_ERROR);
	}
#else
	/* check M and N are both == 1 */
	if (M!=1 || N!=1)
	{
		fatal_error(BAD_SWAPX_MN_VALUE_ERROR);
	}
#endif /* VM_INTS_SUPPORTED */
#endif /* NOT_TRUSTED */


	/* remove N+M elements */
	for (i = 0; i < U16t8(U8_A_U8(N, M)); U8_INC(i))
	{
		N_depth[i] = popShort();
	}

	/* push the first M elements onto the stack */
	for (i = 0; i < M; U8_INC(i))
	{
		pushShort(N_depth[U8_S_U8(U8_S_U8(M, i), 1)]);
	}


	/* push back the M+1 to M+N elements onto the M elements... */
	for (i = 0; i < N; U8_INC(i))
	{
		pushShort(N_depth[U8_S_U8(U8_S_U8(U16t8(U8_A_U8(M, N)), i), 1)]);
	}
}

