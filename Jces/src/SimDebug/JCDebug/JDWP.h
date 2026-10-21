/*
 * JDWP.h
 *
 *  Created on: 2012-8-28
 *      Author: mowenpan
 */
#ifndef _JDWP_VM_H
#define _JDWP_VM_H

#include "pthread.h"

/* JCVM Internal Runtime Object Header ObjectType Definitions */
/* 842 1 8 4 2 1 */
#define JCDEBUG_Header_Simple_Object            			0x20 /* 001 - - - - - */
#define	JCDEBUG_Header_NoDefine								        0x40 /* 010 - - - - - */
#define JCDEBUG_Header_ArrayType_Boolean        			0x60 /* 011 - - - - - */
#define JCDEBUG_Header_ArrayType_Byte           			0x80 /* 100 - - - - - */
#define JCDEBUG_Header_ArrayType_Short          			0xA0 /* 101 - - - - - */
#define JCDEBUG_Header_ArrayType_Int            			0xC0 /* 110 - - - - - */
#define JCDEBUG_Header_ArrayType_Reference      			0xE0 /* 111 - - - - - */

/* OBJ_HDR_TYPE_BYTE Masks */
#define JCDEBUG_OBJ_HDR_OBJECT_TYPE_MASK   					0xE0
#define JCDEBUG_CRH_IsInterface    							0x80

#define SIZE_OBJECT_ID          4
#define SIZE_FIELD_ID           4
#define SIZE_INTEGER            4
#define SIZE_CLASS_ID           4
//#define SIZE_LONG               4
#endif
