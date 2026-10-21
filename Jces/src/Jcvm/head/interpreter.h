/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  interpreter.h
 * Version 1.15
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.interpreter.h
 * Last modified on 03/06/06 at 13:57:04
 * Last modifed by: Eric Nellen
 * Original author: David Getreu, Alexandre Frey, Nicolas Regnault
 -*/

#ifndef    __INTERPRETER_H__
#define    __INTERPRETER_H__

#include "typejava.h"

/**
 * Initialize the interpreter
 **/
extern  void initInterpreter(void);

/**
 * Execute bytecode until PC is equal to INITIAL_PC
 **/
extern  void run(void);

#endif /* __INTERPRETER_H__ */
