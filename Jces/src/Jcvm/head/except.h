/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  except.h
 * Version 1.13
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.except.h
 * Last modified on 03/06/06 at 13:22:51
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __EXCEPTION_H__
#define    __EXCEPTION_H__

#include "profile.h"
#include "typejava.h"


/*
 * DEFINITION
 */

typedef struct t_exceptionEntry
{
	jref start;
	jshort length;
	jref handler;
	jref classT;
} exceptionEntry;
#define ExceptionEntrySize              ((3 * sizeof(jref)) + sizeof(jshort))

#define ET_start                        0x00
#define ET_length                       (ET_start + sizeof(jref))
#define ET_handler                      (ET_length + sizeof(jshort))
#define ET_class                        (ET_handler + sizeof(jref))

/* Exception Table List Structure */
typedef struct t_ExceptionList
{
	u8 ExceptionTableLength;
	exceptionEntry ExceptionTable[1]; /* dummy size of '1' */
} ExceptionList;

#define ExceptionListSize               (sizeof(ExceptionList))

#define EL_ExceptionTableLength         0x00
#define EL_ExceptionTable               (EL_ExceptionTableLength + sizeof(u8))

/*in mask file,wo save the next exception table list index in the first 4 bytes,
the current table list offset in the second 4 bytes,package ID ine the last byte.*/
#define ExceptionTableNodeSize          ((SIZE_MEMREF * 2) + 1)	   
#define EL_pNextExceptionTable          0x00
#define EL_pExceptionTable              (EL_pNextExceptionTable + SIZE_MEMREF)
#define EL_pkgId                        (EL_pExceptionTable + SIZE_MEMREF)
#define ExceptionTable_ACTIVE_LENGTH_MASK               0x7FFF
#define ExceptionTable_STOPBIT_MASK                     0x8000

#define NULL_ExceptionList              				0x0000

/*
 * METHOD
 */

extern  void handleException(jref objref, PF8 objectAddr)			SMALL_MODE;

/**
 * Add an exception table to the global linked list of exception tables
 **/
extern  void add_exception_table(void);
extern  void addexceptiontable(u8 pkgId)							SMALL_MODE;

/**
 * Remove the last exception table from the global linked lists of exception
 * handler tables.
 **/
extern  void remove_exception_table(u8 pkgId)						SMALL_MODE;

#endif /* __EXCEPTION_H__ */
