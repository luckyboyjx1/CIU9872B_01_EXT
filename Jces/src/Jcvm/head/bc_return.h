/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  bc_return.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_return.h
 * Last modified on 03/06/06 at 13:22:44
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCRETURN_H__
#define    __BCRETURN_H__

#include "profile.h"

//extern	void _sreturn(void);
#define _sreturn _areturn
extern	void _areturn(void);
extern	void _return(void);

#if VM_INTS_SUPPORTED
extern  void _ireturn(void);
#endif

#endif /* __BCRETURN_H__ */
