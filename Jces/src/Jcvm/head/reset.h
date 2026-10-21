/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  reset.h
 * Version 1.7
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.reset.h
 * Last modified on 03/06/06 at 13:23:03
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __RESET_H__
#define    __RESET_H__

#include "typejava.h"

/*
 * METHOD
 */
extern  void InitGlobalVariable(void);
extern  void  InitJCRE1(void);
extern  void  InitJCRE2(void);
extern  void InitJCRE3(void);
extern  void reset(void);

#endif /* __RESET_H__ */

