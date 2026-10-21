#ifndef TIMERMODULE_INTERFACE_H
#define TIMERMODULE_INTERFACE_H
#include "ecpDefine.h"

#if TIMERMODULE
#include "TimerModuleProces.h"
extern void IF_TimerModule_ReleaseTimer(BYTE* timerID);
#endif
extern void IF_TimerModule_StartTimer(u8* timerID, u8* timerValue);
#endif



