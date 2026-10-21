/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  bc_control.h
 * Version 1.6
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_control.h
 * Last modified on 03/06/06 at 13:56:51
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCCONTROL_H__
#define    __BCCONTROL_H__

#include "profile.h"

extern	void _nop(void);
extern	void _athrow(void);
extern	void _ifeq(void);
extern	void _ifne(void);
extern	void _iflt(void);
extern	void _ifge(void);
extern	void _ifgt(void);
extern	void _ifle(void);
extern	void _ifnull(void);
extern	void _ifnonnull(void);
extern	void _if_acmpeq(void);
extern	void _if_acmpne(void);
extern	void _if_scmpeq(void);
extern	void _if_scmpne(void);
extern	void _if_scmplt(void);
extern	void _if_scmpge(void);
extern	void _if_scmple(void);
extern	void _if_scmpgt(void);
extern	void _ifeq_w(void);
extern	void _ifne_w(void);
extern	void _iflt_w(void);
extern	void _ifge_w(void);
extern	void _ifgt_w(void);
extern	void _ifle_w(void);
extern	void _ifnull_w(void);
extern	void _ifnonnull_w(void);
extern	void _if_acmpeq_w(void);
extern	void _if_acmpne_w(void);
extern	void _if_scmpeq_w(void);
extern	void _if_scmpne_w(void);
extern	void _if_scmplt_w(void);
extern	void _if_scmpge_w(void);
extern	void _if_scmpgt_w(void);
extern	void _if_scmple_w(void);
extern	void _goto(void);
extern	void _goto_w(void);
extern	void _jsr(void);
extern	void _ret(void);
extern	void _stableswitch(void);
extern	void _slookupswitch(void);

#if VM_INTS_SUPPORTED
extern  void _icmp(void);
extern  void _itableswitch(void);
extern  void _ilookupswitch(void);
#endif

#endif /* __BCCONTROL_H__ */
