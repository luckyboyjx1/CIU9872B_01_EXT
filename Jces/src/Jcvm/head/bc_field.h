/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  bc_field.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_field.h
 * Last modified on 03/06/06 at 13:56:53
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCFIELD_H__
#define    __BCFIELD_H__

#include "profile.h"

extern	void _getfield_a(void);
extern	void _getfield_a_w(void);
extern	void _getfield_a_this(void);
extern	void _getfield_b(void);
extern	void _getfield_b_w(void);
extern	void _getfield_b_this(void);
//extern	void _getfield_s(void);
//extern	void _getfield_s_w(void);
extern	void _getfield_s_this(void);
extern	void _putfield_a(void);
extern	void _putfield_a_w(void);
extern	void _putfield_a_this(void);
extern	void _putfield_b(void);
extern	void _putfield_b_w(void);
extern	void _putfield_b_this(void);
extern	void _putfield_s(void);
extern	void _putfield_s_w(void);
extern	void _putfield_s_this(void);
//extern	void _getfield_a_common(u8 index, jref objref)		SMALL_MODE;
#define _getfield_s_common(index,  objref)     _getfield_a_common(index, objref)		
#define _getfield_s          _getfield_a
#define _getfield_s_w        _getfield_a_w
#define _getfield_s_this     _getfield_a_this
extern	PN8  _putgetfield_abs_common(u8 index,jref objref);
#if VM_INTS_SUPPORTED
extern	void _getfield_i(void);
extern	void _getfield_i_w(void);
extern	void _getfield_i_this(void);
extern	void _putfield_i(void);
extern	void _putfield_i_w(void);
extern	void _putfield_i_this(void);
#endif

#endif /* __BCFIELD_H_ */
