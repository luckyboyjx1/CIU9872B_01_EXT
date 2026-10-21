/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  bc_load.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_load.h
 * Last modified on 03/06/06 at 13:22:43
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCLOAD_H__
#define    __BCLOAD_H__

#include "profile.h"

extern	void _sload(void);
extern	void _sload_0(void);
extern	void _sload_1(void);
extern	void _sload_2(void);
extern	void _sload_3(void);
//extern	void _aload(void);
//extern	void _aload_0(void);
//extern	void _aload_1(void);
//extern	void _aload_2(void);
//extern	void _aload_3(void);
#define	_aload	_sload
#define	_aload_0	_sload_0
#define	_aload_1	_sload_1
#define	_aload_2	_sload_2
#define	_aload_3	_sload_3

#if VM_INTS_SUPPORTED
extern	void _iload(void);
extern	void _iload_0(void);
extern	void _iload_1(void);
extern	void _iload_2(void);
extern	void _iload_3(void);
#endif

#endif /* __BCLOAD_H__ */
