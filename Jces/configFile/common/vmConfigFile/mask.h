/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

/*+
 * File:  mask.h
 * Version 1.11
 * Path: /home/sa112047/ws/refimpl.saqib/cjdk/VM/cjcre/src/platforms/common/SCCS/s.mask.h
 * Last modified on 01/03/26 at 15:17:59
 * Last modifed by: Saqib Ahmad
 * Original author: William R. King
 -*/

#ifndef __MASK_H__
#define __MASK_H__

#include "profile.h"
#include "DataOperation.h"
#include "SysConfigTarget.h"
#include "JcvmDataConfig.h"
#include "SysConfigData.h"
#include "uicc_main.h"

#define APPLET_FIELD_INDEX_PADDING	1
#define JCVM_JAVA_CODE_MASK_START			userclass (HCONST = MASK1)
#define JCVM_JAVA_CODE_MASK_END
/*  defines used with "old" mask.c */
#ifdef obsolete
#define HIGH(x) (u8) ((x) >> 8)
#define LOW(x)  (u8) ((x) & 0x00FF)

#define E2HIGH(x)	(u8) (HIGH((x) + E2PBase))
#define E2LOW(x)	(u8) (LOW((x) + E2PBase))
#define ROMHIGH(x)	(u8) (HIGH((x) + ROMBase))
#define ROMLOW(x)	(u8) (LOW((x) + ROMBase))

#define ROM(x)		(jref)((x) + ROMBase)
#define E2(x)		(jref)((x) + E2PBase)
#endif

/*
 * DATA
 */
extern u8 CONST FAR rommask[];
extern u8 CONST FAR mapTableAlign[];


#define ROM_ARRAY(type, name)	  type  CONST name[]

extern ROM_ARRAY(u8, staticinit);

#define __MASK_HEADER__

#define MASK_ROM_BASE             (ROMBase)
#define MASK_ROM_STATICINIT_BASE  ((memref)&staticinit)

#define MASK_E2P_STATICINIT_BASE  (E2P_IMAGE_START)


#define MASK_STATICINIT_BASE_HIGH_3BYTE  ((u8)((MASK_E2P_STATICINIT_BASE) >> 16 & 0xFF))
#define MASK_STATICINIT_BASE_HIGH_2BYTE  ((u8)((MASK_E2P_STATICINIT_BASE) >> 8 & 0xFF))



#include "mask.c"
#undef  __MASK_HEADER__

#endif   /* __MASK_H__ */
