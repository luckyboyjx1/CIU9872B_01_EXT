#ifndef __SWP_TIMER_HAL_h__
#define __SWP_TIMER_HAL_h__

#include "ecpDefine.h"


/*
	Timer 1 for ISO 7816 send WTX
	Timer 2 for SWP send timer out frame ( Clock = 1MHz )
	Timer 3 RFU
*/


#define  T0_TIMER_INDEX   0
#define  SWP_TIMER_INDEX  1



#if HAL__IO__SWP
	extern void ecpStartSwpTimer(u16 loadValue);
	extern void ecpStopSwpTimer( void );
	extern void ecpSwpTimeDelay(u16 timeCounter);
#else
	#define ecpStartSwpTimer(loadValue)
	#define ecpStopSwpTimer( )
	#define ecpSwpTimeDelay(timeCounter)
#endif

#endif
