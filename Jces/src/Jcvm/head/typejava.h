/*****************************************************************************
 *types.h
 *Define all types used in program.
 *wang hui
 *eastcompeace Technology Corporation
 *Flags:
 *.
 *Note:
 *.
 *History:
 *2007,6,23	.Created
 *****************************************************************************/

#ifndef __TYPEJAVA_H__
#define __TYPEJAVA_H__

#include "profile.h"
#include "DataOperation.h"


/* C / VM types */


//typedef signed short 			s16;
//typedef signed long 			s32;
//typedef unsigned char 			boolean;

/* java types */
typedef s8 						jbyte;
typedef s16 					jshort;
typedef u8 						jboolean;

//typedef u16 					jref;


#define SIZE_MEMREF   			SIZE_U32

typedef s32 					jint;



#define NOK 					0
#define OK  					1
#define OTHER                   2
/* sizes */
#define SIZE_BYTE				sizeof(jbyte)
#define SIZE_BOOLEAN			sizeof(jboolean)
#define SIZE_SHORT				sizeof(jshort)
#define SIZE_INT				sizeof(jint)
#define SIZE_REF				sizeof(jref)
#define SIZE_FIELD				sizeof(jshort)

#define ARRAY_TYPE_BOOLEAN		1
#define ARRAY_TYPE_BYTE			2
#define ARRAY_TYPE_SHORT		3
#define ARRAY_TYPE_INT			4
#define ARRAY_TYPE_OBJECT		5

#define SIZE_BYTE_L				1L
#define SIZE_SHORT_L			2L
#define SIZE_REF_L				2L
#define SIZE_INT_L				4L
#define SIZE_MEMREF_L			4L

/* size masks */
#define BYTE_SIZE				0x7F
#define SHORT_SIZE				0x7FFF
#define INT_SIZE				0x7FFFFFFF
#define BYTE_SIGN				0x80
#define SHORT_SIGN				0x8000
#define INT_SIGN				0x80000000

#define JBYTE_MAX				S8_MAX
#define JSHORT_MAX				S16_MAX
#define JBYTE_MIN				S8_MIN
#define JSHORT_MIN				S16_MIN

#define JINT_MAX				S32_MAX
#define JINT_MIN				S32_MIN

/* Data Type masks for Java equivalents */
#define C_BYTE_SIZE             0x000000FF
#define C_SHORT_SIZE            0x0000FFFF
#define C_INT_SIZE              0x00FFFFFF
#define C_BYTE_SHIFT            0x00000008
#define C_SHORT_SHIFT           0x00000010
#define C_INT_SHIFT             0x00000018

#define SHIFT_LIMIT             0x0000001F

#define MODULO_BYTE_RANGE       0x100

#define HI_BYTE_OF_SHORT        0xFF00
#define LOW_BYTE_OF_SHORT        0x00FF
#define HI_NIBBLE               0xF0
#define LO_NIBBLE               0x0F
#define NIBBLE_SHIFT            0x04

/* JCVM 2.1 Array Type Definitions */
#define AType_Object            0
#define AType_Boolean           10
#define AType_Byte              11
#define AType_Short             12
#define AType_Int               13
#define AType_REF               14

/* parameter type definitions for remote method parameters */
#define Void_Param				0x1
#define Boolean_Param			0x2
#define Byte_Param				0x3
#define Short_Param				0x4
#define Int_Param				0x5
#define Ref_Param				0x6
#define Boolean_Array_Param		0xA
#define Byte_Array_Param		0xB
#define Short_Array_Param		0xC
#define Int_Array_Param			0xD

	#define U8_MIN 					((u8)0)
	#define U8_MAX 					((u8)0xff)
	#define S8_MIN 					((s8)0x80)
	#define S8_MAX 					((s8)0x7f)

	#define U16_MIN 				((u16)0)
	#define U16_MAX 				((u16)0xffff)
	#define S16_MIN 				((s16)0x8000)
	#define S16_MAX 				((s16)0x7fff)

	#define U32_MIN 				((u32)0)
	#define U32_MAX 				((u32)0xffffffff)
	#define S32_MIN 				((u32)0x80000000)
	#define S32_MAX 				((u32)0x7fffffff)


	/* Note: jtoh32 and htoj32 perform the identical operation.
	 One may compile to fewer instructions than the other. */
	#define div32(n, d) 			((s32)(((s32)(n)) / ((s32)(d))))
	#define rem32(n, d) 			((s32)(((s32)(n)) % ((s32)(d))))
	/*
	#define increace_3BYTE(value)  	(*((u8 *)&value + 2))++;\
									if(*((u8 *)&value + 2) == 0x00)\
									{							  \
										(*((u8 *)&value + 1))++;	   \
										if(*((u8 *)&value + 1) == 0x00)\
										{\
											(*((u8 *)&value))++;\
										}\
									}
	*/
	#define increace_3BYTE(value)	PF8_INC(value)

#endif
