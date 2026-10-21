/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  bc_arith.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_arith.h
 * Last modified on 03/06/06 at 13:22:41
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCARITH_H__
#define    __BCARITH_H__

#include "typejava.h"
#include "profile.h"

extern	void _sadd(void);
extern	void _ssub(void);
extern	void _smul(void);
extern	void _sdiv(void);
extern	void _srem(void);
extern	void _sneg(void);
extern	void _sshl(void);
extern	void _sshr(void);
extern	void _sushr(void);
extern	void _sand(void);
extern	void _sor(void);
extern	void _sxor(void);
extern	void _sinc(void);
extern	void _sinc_w(void);
extern	void _s2b(void);

#if VM_INTS_SUPPORTED
extern	void _iadd(void);
extern	void _isub(void);
extern	void _imul(void);
extern	void _idiv(void);
extern	void _irem(void);
extern	void _ineg(void);
extern	void _ishl(void);
extern	void _ishr(void);
extern	void _iushr(void);
extern	void _iand(void);
extern	void _ior(void);
extern	void _ixor(void);
extern  void _iinc(void);
extern	void _iinc_w(void);
extern	void _s2i(void);
extern	void _i2b(void);
extern	void _i2s(void);
#endif

#endif /* __BCARITH_H__ */
