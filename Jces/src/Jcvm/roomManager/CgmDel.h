/*
 * cgmDelTime.h
 *
 *  Created on: 2015Äê11ÔÂ5ÈÕ
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMDEL_H_
#define SRC_JCVM_ROOMMANAGER_CGMDEL_H_
#include "CgmManager.h"

#if GP__CGM__FOR_MASK
	extern void cgmDel_App(u8 appId);
	extern void cgmDel_Pkg(u8 pkgSdId,u8 pkgId);
	extern void cgmDel_Key(PN8 keySetObjectPa);
#else

	#define cgmDel_App(appId)
	#define	cgmDel_Pkg(pkgSdId,pkgId)
	#define	cgmDel_Key(keySetObjectPa)
#endif /* SRC_JCVM_ROOMMANAGER_CGMDEL_H_ */
#endif
