/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  bc_static.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_static.h
 * Last modified on 03/06/06 at 13:56:56
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCSTATIC_H__
#define    __BCSTATIC_H__

#include "profile.h"

extern	void _getstatic_a(void);
extern	void _getstatic_b(void);
extern	void _getstatic_s(void);
extern	void _putstatic_a(void);
extern	void _putstatic_b(void);
extern	void _putstatic_s(void);

#if VM_INTS_SUPPORTED
extern  void _getstatic_i(void);
extern  void _putstatic_i(void);
#endif

#endif /* __BCSTATIC_H__ */
