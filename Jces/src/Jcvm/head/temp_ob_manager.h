/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  temp_ob_manager.h
 * Version 1.5
 * Path: /net/mumble/javacard_users/home/vo113324/ws/javacard_refimpl_victor/cjdk/VM/cjcre/src/platforms/common/SCCS/s.temp_ob_manager.h
 * Last modified on 01/10/12 at 17:19:41
 * Last modifed by: Michael Saltz
 * Original author: Michael Saltz
 -*/
#ifndef   __TEMP_OB_MANAGER__
#define   __TEMP_OB_MANAGER__

#include "profile.h"
#include "typejava.h"

/**
 * The starting object id for temporary arrays for RMI
 */
#define START_TEMPORARY_OBJECT_ID 	0xFFD0

#if JCVM__VM__RMI
extern  jref commonCreateTempArray(u8 atype, u8 count, jref arrayref);
extern  void resetTempObjManager(void);
#endif

#endif /*__TEMP_OB_MANAGER__*/
