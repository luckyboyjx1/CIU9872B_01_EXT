/*
 * cgmInstallTime.h
 *
 *  Created on: 2015Äê11ÔÂ5ÈÕ
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMINSTALL_H_
#define SRC_JCVM_ROOMMANAGER_CGMINSTALL_H_
#include "CgmManager.h"

#if GP__CGM__FOR_MASK
extern void CgmInstall_setConsumeInfo(void);
extern void CgmInstall_End(u8 appId, u8 setAppSpacePara,u8 isMNOSDflag);
extern void CgmInstall_LimitCheck(u8 appId, u8 setAppSpacePara,u8 isMNOSDflag);
extern void CgmInstall_ConsumeCumulate(u32 pSize, u8 cgm_mask);
extern void CgmInstall_ChangeCC(u16 context);
#elif GP__CGM__FOR_APPPARACHECK
extern void CgmInstall_LimitCheck(u8 appId, u8 setAppSpacePara,u8 isMNOSDflag);
#else
#define  CgmInstall_setConsumeInfo()
#define  CgmInstall_End(appId, setAppSpacePara)
#define  CgmInstall_ConsumeCumulate(pSize, cgm_mask)
#define  CgmInstall_ChangeCC(context)
#endif
#endif /* SRC_JCVM_ROOMMANAGER_CGMINSTALL_H_ */
