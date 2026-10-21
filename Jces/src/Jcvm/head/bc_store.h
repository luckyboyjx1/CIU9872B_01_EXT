/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  bc_store.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_store.h
 * Last modified on 03/06/06 at 13:22:47
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCSTORE_H__
#define    __BCSTORE_H__

#include "profile.h"

//extern	void _sstore(void);
//extern	void _sstore_0(void);
//extern	void _sstore_1(void);
//extern	void _sstore_2(void);
//extern	void _sstore_3(void);
extern	void _astore(void);
extern	void _astore_0(void);
extern	void _astore_1(void);
extern	void _astore_2(void);
extern	void _astore_3(void);

#define _sstore		_astore
#define _sstore_0	_astore_0
#define _sstore_1	_astore_1
#define _sstore_2	_astore_2
#define _sstore_3	_astore_3

#if VM_INTS_SUPPORTED
extern	void _istore(void);
extern	void _istore_0(void);
extern	void _istore_1(void);
extern	void _istore_2(void);
extern	void _istore_3(void);
#endif

#endif /* __BCSTORE_H__ */
