/*
 * cmgLoadTime.h
 *
 *  Created on: 2015Äê11ÔÂ5ÈÕ
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMLOAD_H_
#define SRC_JCVM_ROOMMANAGER_CGMLOAD_H_
#include "CgmManager.h"

#if GP__CGM__FOR_MASK
extern void CgmLoad_End(u16 entryIndex);
extern void CgmLoad_LimitCheck(u8 appID);
extern void CgmLoad_error(void);
extern void CgmLoad_ConsumeCumulate(u32 pSize, u8 cgm_mask);
#elif GP__CGM__FOR_APPPARACHECK
extern void CgmLoad_LimitCheck(u8 appID);
extern void CgmLoad_error(void);
#define  CgmLoad_End(entryIndex)
#define  CgmLoad_ConsumeCumulate(pSize,cgm_mask)
#else
#define  CgmLoad_End(entryIndex)
#define  CgmLoad_LimitCheck(appID)
#define  CgmLoad_error()
#define  CgmLoad_ConsumeCumulate(pSize,cgm_mask)
#endif
#endif /* SRC_JCVM_ROOMMANAGER_CGMLOAD_H_ */
