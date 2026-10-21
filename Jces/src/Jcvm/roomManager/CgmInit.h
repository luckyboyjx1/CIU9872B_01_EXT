/*
 * CgmInit.h
 *
 *  Created on: 2015Äê11ÔÂ5ÈÕ
 *      Author: liuhanren
 */

#ifndef SRC_JCVM_ROOMMANAGER_CGMINIT_H_
#define SRC_JCVM_ROOMMANAGER_CGMINIT_H_
#include "CgmManager.h"
#include "profile.h"

#if GP__CGM__FOR_MASK
extern u8	  g_systemConsumeConfig;

extern	void CgmInit_changeFlag(u8 flag);
extern  void CgmInit_SetFlag(void);
extern  u8 CgmInit_EnableOrNot(PM8 commandBuffer, u8 type);
extern	void CgmInit_GlobalVar(void);
extern 	void CgmInit_consumeVar(void);
extern  void CgmInit_SystemConsumeConfig(void);
extern 	void CgmInit_ValueForISD(void);
extern	void CgmInit_UpdateTag(void);
extern  void CgmInit_keepVarValue(PX8 pa1, u8 value1);
#elif GP__CGM__FOR_APPPARACHECK
extern	void CgmInit_GlobalVar(void);
extern 	void CgmInit_consumeVar(void);
extern	void CgmInit_UpdateTag(void);
extern  void CgmInit_keepVarValue(PX8 pa1, u8 value1);
extern 	void CgmInit_ValueForISD(void);
extern	void CgmInit_changeFlag(u8 flag);
#else
#define   CgmInit_changeFlag(flag)
#define   CgmInit_SetFlag()
#define   CgmInit_EnableOrNot(commandBuffer, type)	 0
#define   CgmInit_GlobalVar()
#define	  CgmInit_consumeVar()
#define   CgmInit_SystemConsumeConfig()
#define   CgmInit_ValueForISD()
#define	  CgmInit_UpdateTag()
#define	  CgmInit_keepVarValue(pa1, value1)
#endif
#endif /* SRC_JCVM_ROOMMANAGER_CGMINIT_H_ */
