/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  bc_array.h
 * Version 1.8
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.bc_array.h
 * Last modified on 03/06/06 at 13:56:50
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __BCARRAY_H__
#define    __BCARRAY_H__

#include "profile.h"
#include "typejava.h"

#define BIT_NEED_NOTHING                 0x00 // 不需要执行任何操作
#define BIT_NEED_REFRESH                 0x01 // 需要提交cache
#define BIT_SET_JCRE_ENTRY_Temporary     0x02 // 需要设置为临时入口点对象
#define BIT_SET_JCRE_ENTRY_Permanent     0x04 // 需要设置成永久入口点对象
#define BIT_SET_JCRE_GLOBAL              0x08 // 需要设置成全局对象

extern	jref _newarrayObject(u8 atype, u16 count, u8 needRefresh)		SMALL_MODE;
extern	void _newarray(void);
extern	void _anewarray(void);
extern	void _arraylength(void);
extern	void _aaload(void);
extern	void _aastore(void);
extern	void _baload(void);
extern	void _bastore(void);
extern	void _saload(void);
extern	void _sastore(void);

#if VM_INTS_SUPPORTED
extern	void _iaload(void);
extern	void _iastore(void);
#endif

#endif /* __BCARRAY_H__ */
