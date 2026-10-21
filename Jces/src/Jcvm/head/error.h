/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  error.h
 * Version 1.9
 * Path: /home/sa112047/ws/refimpl.saqib/cjdk/VM/cjcre/src/platforms/common/SCCS/s.error.h
 * Last modified on 01/03/26 at 15:18:16
 * Last modifed by: Eric Nellen
 * Original author: Alexandre Frey, Nicolas Regnault
 -*/

#ifndef   __ERROR_H__
#define   __ERROR_H__

#include "profile.h"
#include "typejava.h"

/*
 * Private implementation variable, needed by the macros below.
 */
#include <setjmp.h>
extern TYPE_JUP_BUF * _error_env;


typedef u16 error_t;
#define NO_ERROR_ ((error_t)0)
/* note: the trailing "_" is necessary to avoid conflict with NO_ERROR in winerror.h */

/**
 * This function is called when a I/O error is encountered or when we
 * want to force a reset.
 **/
extern  void end_session(void)
#ifdef GCC
__attribute__((noreturn))
#endif
;

/**
 * Lock card session. This function is called when a fatal error is
 * encountered or when we want to force a reset. This function is also called
 * when an error is thrown while there is no error handler.
 **/
extern  void halt_session(u16 errorcode)
#ifdef GCC
__attribute__((noreturn))
#endif
;
/**
 * Either returns NO_ERROR_ when handler is set or
 * an error code different from NO_ERROR_ when an error is to be handled.
 **/
//#define set_error_handler() ((error_t)setjmp(_error_env))
#define set_error_handler(ERR_ENV) ((error_t)setjmp(ERR_ENV))
/**
 * This function must be called when the function containing the corresponding
 * set_error_handler exits.
 **/
#define clear_error_handler() (_error_env = MEM_NULL)
/**
 * This function never returns.
 * It either locks the session if there is no current error handler or
 * goes to the current error handler (whose set_error_handler then returns code).
 * Note:  throw_error must not be called with NO_ERROR_.
 **/
//#define throw_error(errorcode) longjmp(_error_env, errorcode)
extern void throw_error(u16 errorcode);
extern void setWtxError(u8 WtxErrorReason) SMALL_MODE;
extern void setReceiveError(u8 ReceiveErrorReason);
extern void setIntError(u8 IntErrorReason);
//extern  u16 stackUnderFlow(void);

/**
 * This function always lock the session
 **/
#define fatal_error(errorcode) halt_session(errorcode)

#endif /* __PLATFORM_ERROR_H__ */
