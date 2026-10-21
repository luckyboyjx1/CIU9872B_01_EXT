/*
 * <callback/debugMask.h>
 *
 *  Created on: 2014Äê12ÔÂ15ÈÕ
 *      Author: mowenpan
 */

#ifndef DEBUGMASK_H_
#define DEBUGMASK_H_

#include "DataOperation.h"
#include "profile.h"
#include "mask.h"
#include "profile.h"
#include "hal_callback.h"

#if VM_WITH_DEBUG
extern ROM_ARRAY(u8, debugMask);
	#define __DEBUG_MASK_HEADER__
	#include "debugMask.c"
	#undef  __DEBUG_MASK_HEADER__
#endif

#endif /* DEBUGMASK_H_ */
