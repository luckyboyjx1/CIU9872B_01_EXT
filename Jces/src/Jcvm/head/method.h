/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  method.h
 * Version 1.10
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.method.h
 * Last modified on 03/06/06 at 13:23:00
 * Last modifed by: Alexandre Frey, Nicolas Regnault
 * Original author: David Getreu
 -*/

#ifndef    __METHOD_H__
#define    __METHOD_H__

#include "DataOperation.h"

/*
 * DEFINITION
 */
#define ACC_NATIVE 0x20

/* "Normal" Method Header Nibble Field definition masks */
#define FLAG_NIBBLE 0xF0        /* First Byte */
#define STACK_NIBBLE 0x0F       /* First Byte */
#define PARAMS_NIBBLE 0xF0      /* Second Byte */
#define LOCALS_NIBBLE 0x0F      /* Second Byte */

/**
 * Get the number of parameter of a given method
 **/
extern  u8 getMethodParamSize(PF8 methodAddr);

/**
 * Get the address of a virtual method with specified token in the given
 * object.
 **/
extern  memref resolveVirtualMethod(PF8 objref, u8 method_token)									SMALL_MODE;

/**
 * Get the address of an interface method with the specified method token
 * (w.r.t. to the given interface).
 **/
extern  memref resolveInterfaceMethod(PF8 objref, PF8 interface_addr, u8 method_intf_token)		SMALL_MODE;

/**
 * Prepare interpreter to execute the given method in context new_cc. If the
 * method is native, set the nativeMethod register. Otherwise, create a new
 * stack frame, and set PC to first bytecode of method
 **/
extern  void callmethod(PF8 methodAddr, u8 methodPkg, u16 new_cc)								SMALL_MODE;

#endif /* __METHOD_H__ */
