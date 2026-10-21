#ifndef __SYS_HAL_H_
#define __SYS_HAL_H_

#include "DataOperation.h"
#include "SysConfigProfile.h"
#include "ecpDefine.h"
#if HAL__SYSTEM_CONFIG
	extern	void reset_Init(void);
	extern  u8   reset_ReInit(u8 resetStauts);
	extern  void ecpSysHal_Init(void);
	extern  u8	 ecpSysHal_GetCpuPower(void);
	extern	u8	 ecpSysHal_IsWarmReset(void);
	extern	u8	 ecpSysHal_IsDirectConversion(void);
	extern	void ecpSysHal_SetCpuClk(u32 clk);
	extern	u16  ecpSysHal_GetCpuClk(void);
	extern	void ecpSysHal_SetIntBeforeRfWtx(void);
	extern	void ecpSysHal_SetIntAfterRfWtx(void);
	extern	u8	 ecpSysHal_ChipConfigFun(PX8 buf, u8 infoType)    SMALL_MODE;
	extern  u8   ecpSysHal_SeInfoFun(PX8 buf, u16 desOff,u16 datalen ,u8 infoType);
	extern	u8 	 ecpSysHal_SuspendAllInterrupts(void);
	extern	u8	 ecpSysHal_ResumeAllInterrupts(void);
	extern	u8	 ecpSysHal_AssignProtocolPriority(void);
	extern  void ecpSysHal_ManageNvmRepair(void);
	extern  void ecpSysHal_ManageSetClockConfig(u8 type);
	extern  u8   ecpSysHal_ClearCard(void);
	extern  void ecpSysHal_SystemReset(u8 resetFlag);
#else
	#define ecpSysHal_InitVar()								
	#define ecpSysHal_GetCpuPower()							0
	#define	ecpSysHal_IsWarmReset()							0
	#define	ecpSysHal_IsDirectConversion()					0
	#define	ecpSysHal_SetCpuClk(clk)
	#define	ecpSysHal_GetCpuClk()							0
	#define	ecpSysHal_SetIntBeforeRfWtx()
	#define	ecpSysHal_SetIntAfterRfWtx()
	#define	ecpSysHal_ChipConfigFun(buf, infoType)			0
    #define	ecpSysHal_SeInfoFun(buf, infoType)			0
	#define ecpSysHal_getSEINFO(buf,desOff,datalen,infoType)   0
	#define ecpSysHal_SurspendAllInterrupts()
	#define ecpSysHal_ResumeAllInterrupts()
	#define	ecpSysHal_AssignProtocolPriority()
	#define ecpSysHal_ClearCard()							0
    #define  ecpSysHal_SystemReset()                       0
#endif

#endif
