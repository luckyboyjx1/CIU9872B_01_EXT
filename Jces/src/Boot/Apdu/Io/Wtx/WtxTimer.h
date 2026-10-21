#ifndef __WTX_TIMER_H__
#define	__WTX_TIMER_H__

#include "DataOperation.h"
#include "Io_7816T0.h"
#include "Io_7816T1.h"
#include "Io_14443TaL4.h"
#include "WtxTimer.h"
#include "WtxTimerHal.h"
#include "bootConfigMethod.h"

#define T0_WTX_TIMER							0
#define T1_WTX_TIMER							1
#define TA_WTX_TIMER							2
#define NO_WTX_TIMER							0xFF
extern 		u8		g_timerType;
#if HAL__IO__TIMER
	#define wtxTimer_SetCurWtxTimerType(timerType) 		(g_timerType = timerType)
	#define wtxTimer_GetCurWtxTimerType()				g_timerType
	extern	void io_OpenTimer(void);
	extern	void wtxTimer_WtxTimerInit(void);
	extern	void wtxTimer_WtxTimerStart(void);
	extern	void wtxTimer_WtxTimerSuspend(void);
	extern	void wtxTimer_WtxTimerStop(void);
	extern	void wtxTimer_WtxTimerResume(void);
	extern	void wtxTimer_InteruptInit(memref timeInteruptAddress);
#endif

#endif
