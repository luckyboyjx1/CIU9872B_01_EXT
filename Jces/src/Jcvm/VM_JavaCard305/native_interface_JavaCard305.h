/*
 * @(#)native_interface_JavaCard305.h	2022/10/20
 */

#ifndef    __native_interface_JavaCard305_H__
#define    __native_interface_JavaCard305_H__

#include "typejava.h"

/**
 * Execute a give native method.
 * @param func_index an integer that identifies the native method. Native
 * method identifiers are generated in the mask by the mask generator.
 **/
extern u8 JavaCard305API_CallNative(u16 fun_index)    SMALL_MODE;

#endif /* __native_interface_JavaCard305_H__ */
