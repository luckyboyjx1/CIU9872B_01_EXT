#ifndef __WTX_TIMER_HAL_H__
#define	__WTX_TIMER_HAL_H__

#include "DataOperation.h"
#include "SysConfigProfile.h"

#if HAL__IO__TIMER
	#if HAL__IO__ISO7816_T0
		void ecpSetT0WtxTimer(u8 T0BwtH, u8 T0BwtL);
		void ecpClearT0WtxTimer(void);
		void ecpEnableT0WtxTimer(void);
		void ecpDisableT0WtxTimer(void);
		void ecpStartT0WtxTimer(void);
		void ecpStopT0WtxTimer(void);
		void ecpSuspendT0WtxTimer(void);
		void ecpResumeT0WtxTimer(void);
		void ecpInitT0TimeInterupt(memref timeInterupT0ddress);
		void ecpClrT0WtxTimerOverFlow(void);
		u32 ecpDivideT0WtxTimerFrequency(u32 bwi);
		extern  u8   wtxTimer_Tunning(void);
	#else
		#define ecpSetT0WtxTimer(T0BwtH,T0BwtL)
		#define ecpClearT0WtxTimer()
		#define ecpEnableT0WtxTimer()
		#define ecpDisableT0WtxTimer()
		#define ecpStartT0WtxTimer()
		#define ecpStopT0WtxTimer()
		#define ecpSuspendT0WtxTimer()
		#define ecpResumeT0WtxTimer()
		#define ecpInitT0TimeInterupt(timeInterupT0ddress)
		#define ecpClrT0WtxTimerOverFlow()
		#define ecpDivideT0WtxTimerFrequency(bwi)
		#define wtxTimer_Tunning()
	#endif
	#if HAL__IO__ISO7816_T1
		void ecpSetT1WtxTimer(u8 T1BwtH, u8 T1BwtL);
		void ecpClearT1WtxTimer(void);
		void ecpEnableT1WtxTimer(void);
		void ecpDisableT1WtxTimer(void);
		void ecpStartT1WtxTimer(void);
		void ecpStopT1WtxTimer(void);
		void ecpSuspendT1WtxTimer(void);
		void ecpResumeT1WtxTimer(void);
		void ecpInitT1TimeInterupt(memref timeInterupT1ddress);
		void ecpClrT1WtxTimerOverFlow(void);
		u32 ecpDivideT1WtxTimerFrequency(u32 bwi);
	#else
		#define ecpSetT1WtxTimer(T1BwtH,T1BwtL)
		#define ecpClearT1WtxTimer()
		#define ecpEnableT1WtxTimer()
		#define ecpDisableT1WtxTimer()
		#define ecpStartT1WtxTimer()
		#define ecpStopT1WtxTimer()
		#define ecpSuspendT1WtxTimer()
		#define ecpResumeT1WtxTimer()
		#define ecpInitT1TimeInterupt(timeInterupT1ddress)
		#define ecpClrT1WtxTimerOverFlow()
		#define ecpDivideT1WtxTimerFrequency(bwi)	bwi
	#endif
	#if HAL__IO__ISO14443_TA
		void ecpSetTaWtxTimer(u8 fwi);
		void ecpClearTaWtxTimer(void);
		void ecpEnableTaWtxTimer(void);
		void ecpDisableTaWtxTimer(void);
		void ecpStartTaWtxTimer(void);
		void ecpStopTaWtxTimer(void);
		void ecpSuspendTaWtxTimer(void);
		void ecpResumeTaWtxTimer(void);
		void ecpInitTATimeInterupt(memref timeInteruptAddress);
		void ecpClrTaWtxTimerOverFlow(void);
	#else
		#define ecpSetTaWtxTimer(fwi)
		#define ecpClearTaWtxTimer()
		#define ecpEnableTaWtxTimer()
		#define ecpDisableTaWtxTimer()
		#define ecpStartTaWtxTimer()
		#define ecpStopTaWtxTimer()
		#define ecpSuspendTaWtxTimer()
		#define ecpResumeTaWtxTimer()
		#define ecpInitTATimeInterupt(timeInteruptAddress)
		#define ecpClrTaWtxTimerOverFlow()
	#endif

#else
	#define ecpSetT0WtxTimer(T0BwtH,T0BwtL)
	#define ecpClearT0WtxTimer()
	#define ecpEnableT0WtxTimer()
	#define ecpDisableT0WtxTimer()
	#define ecpStartT0WtxTimer()
	#define ecpStopT0WtxTimer()
	#define ecpSuspendT0WtxTimer()
	#define ecpResumeT0WtxTimer()
	#define ecpInitT0TimeInterupt(timeInterupT0ddress)
	#define ecpClrT0WtxTimerOverFlow()
	#define ecpDivideT0WtxTimerFrequency(bwi)	bwi

	#define ecpSetT1WtxTimer(T1BwtH,T1BwtL)
	#define ecpClearT1WtxTimer()
	#define ecpEnableT1WtxTimer()
	#define ecpDisableT1WtxTimer()
	#define ecpStartT1WtxTimer()
	#define ecpStopT1WtxTimer()
	#define ecpSuspendT1WtxTimer()
	#define ecpResumeT1WtxTimer()
	#define ecpInitT1TimeInterupt(timeInterupT1ddress)
	#define ecpClrT1WtxTimerOverFlow()
	#define ecpDivideT1WtxTimerFrequency(bwi)	bwi

	#define ecpSetTaWtxTimer(fwi)
	#define ecpClearTaWtxTimer()
	#define ecpEnableTaWtxTimer()
	#define ecpDisableTaWtxTimer()
	#define ecpStartTaWtxTimer()
	#define ecpStopTaWtxTimer()
	#define ecpSuspendTaWtxTimer()
	#define ecpResumeTaWtxTimer()
	#define ecpInitTATimeInterupt(timeInteruptAddress)
	#define ecpClrTaWtxTimerOverFlow()
#endif	//HAL__IO__TIMER

#if BOOT__IO__ISO14443_TA_RF_LEVEL3
	extern  void ecpRfIrqTimer1(void);
#endif
	
#endif
