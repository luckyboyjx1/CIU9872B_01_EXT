#ifndef __WTX_INTERUPT_H__
#define	__WTX_INTERUPT_H__

#include "bootConfigProfile.h"

extern u8 g_wtxDelay;
extern u8 g_wtxDelayLimit;
extern u8 g_uartBwtH;
extern u8 g_uartBwtL;

#if HAL__IO__TIMER
	extern void WtxInterupt_UartRequestWtx(u8 currentProtocol)  SMALL_MODE;
#else
	#define WtxInterupt_UartRequestWtx(currentProtocol)
#endif

#if HAL__IO__ISO14443_TA
	extern void WtxInterupt_RfRequestWtx(void);
#else
	#define WtxInterupt_RfRequestWtx()
#endif    


#endif //__WTX_INTERUPT_H__
