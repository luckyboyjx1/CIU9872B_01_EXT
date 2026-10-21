/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*+
 * File:  firewall.h
 * Version 1.11
 * Path: /usr/re/jcdevkit/ws/src/share/c/common/SCCS/s.firewall.h
 * Last modified on 03/06/06 at 13:22:53
 * Last modifed by: Eric Nellen
 * Original author: David Getreu, Alexandre Frey, Nicolas Regnault
 -*/

#ifndef    __FIREWALL_H__
#define    __FIREWALL_H__

#include "profile.h"
#include "typejava.h"


/*
 * These methods test for various security conditions.
 * If these condition are not satisfied, they throw a SECURITY_EXCEPTION.
 *
 * See the module implementation for the details.
 */
#if JCVM__VM__FIRE_WALL

extern	void checkRead(PF8 objAddr);
extern	void checkEntryPoint(PF8 objAddr);
extern	void checkFieldStore(PF8 objAddr, jref value)					SMALL_MODE;
extern	void checkArrayStore(PF8 arrayAddr, jref value)					SMALL_MODE;
extern	void checkStaticStore(PF8 objAddr)								SMALL_MODE;
extern	void checkShareable(PF8 objAddr)								SMALL_MODE;
extern	void checkChannelAccess(PF8 objAddr);
extern	void checkArrayRead(PF8 arrayAddr)								SMALL_MODE;
extern	void checkInstanceAndCast(PF8 objAddr, PF8 classAddr)			SMALL_MODE;
extern	void checkDTRCreation(void);
extern	void checkPrevContextAccess(PF8 objectAddr)						SMALL_MODE;
extern	void checkAPDUObjectAccess(void);
#endif

#endif /* __FIREWALL_H__ */
