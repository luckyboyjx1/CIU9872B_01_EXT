/*****************************************************************************
 * File:  bc_const.c
 * Version 1.12

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 ****************************************************************************/
#include "bc_const.h"
#include "context.h"
#include "stack.h"
#include "object.h"
#include "memory.h"
#include "anomaly.h"
#include "global_variable.h"

/******************************************************************************
 * This method implements the Java Card 2.1 aconst_null instruction.
 * Format:
 * aconst_null
 *
 * Stack Image:
 * ... --> ..., null

 ******************************************************************************/
void _aconst_null()
{
	pushReference(REF_NULL);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_m1 instruction.
 * Format:
 * sconst_m1
 *
 * Stack Image:
 * ... --> ..., -1

 ******************************************************************************/
void _sconst_m1()
{
	pushShort(0xFFFF);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_0 instruction.
 * Format:
 * sconst_0
 *
 * Stack Image:
 * ... --> ..., 0

 ******************************************************************************/
void _sconst_0()
{
	pushShort(0);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_1 instruction.
 * Format:
 * sconst_1
 *
 * Stack Image:
 * ... --> ..., 1

 ******************************************************************************/
void _sconst_1()
{
	pushShort(1);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_2 instruction.
 * Format:
 * sconst_2
 *
 * Stack Image:
 * ... --> ..., 2

 ******************************************************************************/
void _sconst_2()
{
	pushShort(2);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_3 instruction.
 * Format:
 * sconst_3
 *
 * Stack Image:
 * ... --> ..., 3

 ******************************************************************************/
void _sconst_3()
{
	pushShort(3);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_4 instruction.
 * Format:
 * sconst_4
 *
 * Stack Image:
 * ... --> ..., 4

 ******************************************************************************/
void _sconst_4()
{
	pushShort(4);
}

/******************************************************************************
 * This method implements the Java Card 2.1 sconst_5 instruction.
 * Format:
 * sconst_5
 *
 * Stack Image:
 * ... --> ..., 5

 ******************************************************************************/
void _sconst_5()
{
	pushShort(5);
}

/******************************************************************************
 * This method implements the Java Card 2.1 bspush instruction.
 * Format:
 * bspush
 *
 * Stack Image:
 * ... --> value

 ******************************************************************************/
void _bspush()
{
	pushShort(U8tS16(fetchByte()));
}

/******************************************************************************
 * This method implements the Java Card 2.1 sspush instruction.
 * Format:
 * sspush
 *
 * Stack Image:
 * ... --> ..., value

 ******************************************************************************/
void _sspush()
{
	pushShort(fetchShort());
}

#if VM_INTS_SUPPORTED

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_m1 instruction.
 * Format:
 * iconst_m1
 *
 * Stack Image:
 * ... --> ..., 0xffff, 0xffff

 ******************************************************************************/
void _iconst_m1()
{
	pushInt(0xFFFFFFFF);
}

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_0 instruction.
 * Format:
 * iconst_0
 *
 * Stack Image:
 * ... --> ..., 0, 0

 ******************************************************************************/
void _iconst_0()
{
	pushInt(0);
}

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_1 instruction.
 * Format:
 * iconst_1
 *
 * Stack Image:
 * ... --> ..., 0, 1

 ******************************************************************************/
void _iconst_1()
{
	pushInt(1);
}

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_2 instruction.
 * Format:
 * iconst_2
 *
 * Stack Image:
 * ... --> ..., 0, 2

 ******************************************************************************/
void _iconst_2()
{
	pushInt(2);
}

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_3 instruction.
 * Format:
 * iconst_3
 *
 * Stack Image:
 * ... --> ..., 0, 3

 ******************************************************************************/
void _iconst_3()
{
	pushInt(3);
}

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_4 instruction.
 * Format:
 * iconst_4
 *
 * Stack Image:
 * ... --> ..., 0, 4

 ******************************************************************************/
void _iconst_4()
{
	pushInt(4);
}

/******************************************************************************
 * This method implements the Java Card 2.1 iconst_5 instruction.
 * Format:
 * iconst_5
 *
 * Stack Image:
 * ... --> ..., 0, 5

 ******************************************************************************/
void _iconst_5()
{
	pushInt(5);
}

/******************************************************************************
 * This method implements the Java Card 2.1 bipush instruction.
 * Format:
 * bipush
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _bipush()
{
	pushInt(U8tS32(fetchByte()));
}

/******************************************************************************
 * This method implements the Java Card 2.1 sipush instruction.
 * Format:
 * sipush
 *
 * Stack Image:
 * ... --> value.word1, value.word2

 ******************************************************************************/
void _sipush()
{
	pushInt(U16tS32(fetchShort()));
}

/******************************************************************************
 * This method implements the Java Card 2.1 iipush instruction.
 * Format:
 * iipush
 *
 * Stack Image:
 * ... --> ..., value.word1, value.word2

 ******************************************************************************/
void _iipush()
{
	pushInt(fetchInt());
}

#endif

