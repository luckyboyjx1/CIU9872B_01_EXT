/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)native_interface.h	1.16 03/06/06
 */

#ifndef    __JAVAUICCAPI_H__
#define    __JAVAUICCAPI_H__

#include "typejava.h"

/**
 * Execute a give native method.
 * @param func_index an integer that identifies the native method. Native
 * method identifiers are generated in the mask by the mask generator.
 **/
extern u8 JavaUiccAPI_CallNative(u16 fun_index)    SMALL_MODE;

#endif /* __NATIVEINTERFACE_H__ */
