/*
 * Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  error.c
 * Version 1.13
 * Path: /usr/re/jcdevkit/ws/src/share/c/unix_win32/SCCS/s.error.c
 * Last modified on 03/06/06 at 13:23:14
 * Last modifed by: Eric Nellen
 * Original author: Alexandre Frey, Nicolas Regnault
 -*/
#include	"error.h"
#include	"stack.h"
#include    "anomaly.h"
#include        <setjmp.h>

#include "Apdu.h"
#include "context.h"
#include "JcvmRam.h"
#include "Io_14443TaL4.h"
//#include "VMDebug.h"

#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_ERROR_RAM_START
#endif

/* Define the location for long jump to jump to.  See <setjmp.h> for details. */
//jmp_buf X_A _error_env;
TYPE_JUP_BUF * _error_env;




/* Disable automatic sending of wait-extensions, so that the terminal may end the session */
void halt_session(u16 errorcode)
{	/*@ -discast @*/
	/*@ -type @*/
	*JSP = (s16)errorcode;
	apdu_SuspendAllInterrupts();
	/*@ +type @*/
	/*@ +discast @*/
#if !HAL__PLATFORM__SIMULATOR
	memReadU8(MEMtPM8(rommask[0]));
	memReadU8(MEMtPM8(mapTableAlign[0]));
#endif
	goto_deselect_handler(VM_REREST_FLAG);
//	while(1);
}

void throw_error(u16 errorcode)
{
	if(g_isResolveStaticField)
	{
		//when resolve static field in the load process,the cc will temporarily changed to the loading package's cc.
		//so we should change it back when throw a exception, Or the JCRE's object can not be access when do the
		//PackageMgr.restore() in the java layer.
		cc = JCRE_CONTEXT;
		g_isResolveStaticField = 0;
	}
	
//	longjmp(_error_env, errorcode);
	longjmp(*_error_env, errorcode);

}

