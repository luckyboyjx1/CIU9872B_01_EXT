/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)native_interface.h	1.16 03/06/06
 */

#ifndef    __NATIVEINTERFACE_H__
#define    __NATIVEINTERFACE_H__

#include "typejava.h"

/* error returns for native methods */

/* New 32-bit constants */
#define OBJECT_NOT_FOUND 0xffffff01
#define OBJECT_IS_NOT_REMOTE 0XffffFF02
#define METHOD_NOT_FOUND 0xffffff03
#define OTHER_EXCEPTION 0xffffffff


/**
 * Execute a give native method.
 * @param func_index an integer that identifies the native method. Native
 * method identifiers are generated in the mask by the mask generator.
 **/
extern  void callnative(u16 func_index);

#endif /* __NATIVEINTERFACE_H__ */
