/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   Load.c
*
* Description: 该文件主要用于存放实现 安全域相关的函数
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0
*
************************************************************************************************************/
#include "Interface_TimerModule.h"
#include "TimerModuleProces.h"
#include "profile.h"
/*
*@function   :IF_TimerModule_StartTimer
*
*@description:start timer
*
*@See        : 
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20180725   
*            :V1.1  modidy by xuxiaojun@20180803   增加函数参数timerValue， 方便从上层传入
*            :V1.2  modify by xuxiaojun@20210412   同步TPX 5G代码， *timerID为FALSE时已经为非法值，不需要将其再赋值为0xFF
*/
void IF_TimerModule_StartTimer(BYTE* timerID, BYTE* timerValue)
{
#if TIMERMODULE
	
	*timerID = TimerModuleAllocTimerId();  //申请一个有效的timerId
	
	if(*timerID == FALSE)
	{
		return;
	}

	TimerModuleStartTimer(*timerID,timerValue);		
	
#endif	
}

/*
*@function   :IF_TimerModule_ReleaseTimer
*
*@description:start timer
*
*@See        :
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20220107
*/
void IF_TimerModule_ReleaseTimer(BYTE* timerID)
{
#if TIMERMODULE
	TimerModuleReaseTimerId(timerID);
#endif
}
