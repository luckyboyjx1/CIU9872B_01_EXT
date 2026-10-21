/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  bc_object.h
 * Version 1.5
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_object.h
 * Last modified on 03/06/06 at 13:56:54
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCOBJECT_H__
#define    __BCOBJECT_H__

#include "typejava.h"

extern	void _invokevirtual(void);
extern	void _invokespecial(void);
extern	void _invokestatic(void);
extern	void _invokeinterface(void);
extern	void _new(void);
extern	void _checkcast(void);
extern	void _instanceof(void);

extern	jref newObject(u16 classOffset, u8 classPkg, u8 needRefresh)	SMALL_MODE;

#endif /* __BCOBJECT_H__ */
