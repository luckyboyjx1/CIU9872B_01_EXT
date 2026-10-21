/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  bc_const.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_const.h
 * Last modified on 03/06/06 at 13:22:42
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCCONST_H__
#define    __BCCONST_H__

#include "profile.h"

extern	void _aconst_null(void);
extern	void _sconst_m1(void);
extern	void _sconst_0(void);
extern	void _sconst_1(void);
extern	void _sconst_2(void);
extern	void _sconst_3(void);
extern	void _sconst_4(void);
extern  void _sconst_5(void);
extern	void _bspush(void);
extern	void _sspush(void);

#if VM_INTS_SUPPORTED
extern	void _iconst_m1(void);
extern	void _iconst_0(void);
extern	void _iconst_1(void);
extern	void _iconst_2(void);
extern	void _iconst_3(void);
extern	void _iconst_4(void);
extern	void _iconst_5(void);
extern	void _bipush(void);
extern	void _sipush(void);
extern	void _iipush(void);
#endif

#endif /* __BCCONST_H__ */

