/*
 * cgmRunTime.h
 *
 *  Created on: 2015Äê11ÔÂ5ÈÕ
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMRUN_H_
#define SRC_JCVM_ROOMMANAGER_CGMRUN_H_
#include "CgmManager.h"

#if GP__CGM__FOR_MASK
extern void CgmRun_CommitChangeSize(u16 backupCC, u32 RamChange, u32 NvmChange);
extern void CgmRun_ConsumeCumulate(u32 pSize, u8 cgm_mask);
extern void CgmRun_Extradition(u16 entryIndex,u8 srcId,u8 destSdId,u8 flag);
extern void CgmRun_RegistryUpdate(u16 appEntry,u8 srcSdId,u8 destSdId);
extern void CgmRun_ChangeCC(u16 context);
extern void CgmRun_setConsumeInfo(void);
#else
#define  CgmRun_CommitChangeSize(backupCC, RamChange, NvmChange)
#define  CgmRun_ConsumeCumulate(pSize, cgm_mask)
#define  CgmRun_Extradition(entryIndex,srcId,destSdId,flag)
#define  CgmRun_RegistryUpdate(appEntry,srcSdId,destSdId)
#define  CgmRun_ChangeCC(context)
#define  CgmRun_setConsumeInfo()
#endif
#endif /* SRC_JCVM_ROOMMANAGER_CGMRUN_H_ */
