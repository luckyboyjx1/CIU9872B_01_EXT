#include "WtxTimer.h"
#include "Io_14443TaL4.h"
#include "SysConfig.h"
#include "WtxTimerHal.h"
#include "BootRam.h"

#if HAL__IO__TIMER

#pragma WTX_TIMER_RAM_START

		u8		g_timerType;
		u8		g_wtxDelay;
		u8		g_wtxDelayLimit;
		u8		g_uartBwtH;
		u8		g_uartBwtL;

WTX_TIMER_RAM_END

static void wtxTimer_GetUartBwt(u32 value)    SMALL_MODE
{
	u16	    I_A 	bwiValue;

	PATCH_BOOT_APDU_WTXTIMER_001();	
	
	if(value > 0xFFFF)
	{
		g_wtxDelayLimit		= U32_A_U8(U32_D_U16(value, 0xFFFF), 1);
	}
	else
	{
		g_wtxDelayLimit  		= 1;
	}

	bwiValue	= U32t16(U32_AND_U32(value, 0x0000FFFF));

	if(bwiValue == 0)
	{
		g_wtxDelayLimit		= U8_S_U8(g_wtxDelayLimit, 1);
	}
	
#if !WITH_TIMER_DECREASETIMECOUNT	
	bwiValue				= U16_S_U16(0xFFFF, bwiValue);
#endif
	
	g_uartBwtH				= U16t8(U16_SHR_U8(bwiValue, 8));
	g_uartBwtL				= U16t8(bwiValue);
}
#if		HAL__IO__ISO14443_TA
static u8 wtxTimer_GetRFFwi(void)
{
	g_wtxDelayLimit			= sysConfig_GetTaWtxDelayLimit();
	return iso14443TaL4_GetFwi();
}
#endif


void wtxTimer_WtxTimerInit(void)
{
	switch(g_timerType)
	{
		case T0_WTX_TIMER:
			wtxTimer_GetUartBwt(iso7816T0_GetT0WtxTimerValue());
			ecpSetT0WtxTimer(g_uartBwtH, g_uartBwtL);
			break;
#if HAL__IO__ISO7816_T1
		case T1_WTX_TIMER:
			wtxTimer_GetUartBwt(iso7816T1_GetT1WtxTimerValue());
			ecpSetT1WtxTimer(g_uartBwtH, g_uartBwtL);
			break;
#endif
		case TA_WTX_TIMER:
			ecpSetTaWtxTimer(wtxTimer_GetRFFwi());
			break;
		default:
			break;
	}
	
	g_wtxDelay = 0;
	return;
}

void wtxTimer_WtxTimerStart(void)
{
	switch(g_timerType)
	{
		case T0_WTX_TIMER:
			ecpSetT0WtxTimer(g_uartBwtH,g_uartBwtL);
			ecpStartT0WtxTimer();
			break;
#if HAL__IO__ISO7816_T1
		case T1_WTX_TIMER:
			ecpSetT1WtxTimer(g_uartBwtH,g_uartBwtL);
			ecpStartT1WtxTimer();
			break;
#endif
		case TA_WTX_TIMER:
			ecpStartTaWtxTimer();
			break;
		default:
			break;
	}
	g_wtxDelay = 0;
	return;
}

void wtxTimer_WtxTimerSuspend(void)
{
	switch(g_timerType)
	{
		case T0_WTX_TIMER:
			ecpSuspendT0WtxTimer();
			break;
#if HAL__IO__ISO7816_T1
		case T1_WTX_TIMER:
			ecpSuspendT1WtxTimer();
			break;
#endif
		case TA_WTX_TIMER:
			ecpSuspendTaWtxTimer();
			break;
	    default:
			break;
	}
	return;
}

void wtxTimer_WtxTimerStop(void)
{
	switch(g_timerType)
	{
		case T0_WTX_TIMER:
			ecpStopT0WtxTimer();
			break;
#if HAL__IO__ISO7816_T1
		case T1_WTX_TIMER:
			ecpStopT1WtxTimer();
			break;
#endif
		case TA_WTX_TIMER:
			ecpStopTaWtxTimer();
			break;
		default:
			break;
	}
	return;
}

void wtxTimer_WtxTimerResume(void)
{
	switch(g_timerType)
	{
		case T0_WTX_TIMER:
			ecpResumeT0WtxTimer();
			break;
#if HAL__IO__ISO7816_T1
		case T1_WTX_TIMER:
			ecpResumeT1WtxTimer();
			break;
#endif
		case TA_WTX_TIMER:
			ecpResumeTaWtxTimer();
			break;
		default:
			break;
	}
	return;
}

void wtxTimer_InteruptInit(memref timeInteruptAddress)
{			 
	switch(g_timerType)
	{
		case T0_WTX_TIMER:
#if WITH_TIMER_FOR_T0
			ecpInitT0TimeInterupt(timeInteruptAddress);
			break;
#endif
		case T1_WTX_TIMER:
#if WITH_TIMER_FOR_T1
			ecpInitT1TimeInterupt(timeInteruptAddress);
#endif
			break;
		case TA_WTX_TIMER:
			ecpInitTATimeInterupt(timeInteruptAddress);
			break;
		default:
			break;
	}
	return;
}

#endif
