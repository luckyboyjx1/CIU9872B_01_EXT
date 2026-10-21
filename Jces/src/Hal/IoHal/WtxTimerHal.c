#include "WtxTimerHal.h"
#include "chipHead.h"
#include "Io.h"
#include "mhcard_iso7816.h"

#if HAL__IO__TIMER

void ecpSetT0WtxTimer(u8 T0BwtH, u8 T0BwtL)
{	
	SCI->CWTDATA = U16t32(U16_SHL_U8(T0BwtH, 8) | T0BwtL);
}

void ecpClearT0WtxTimer(void)
{
	SCI->CWTDATA = 0xFFFF;
}

void ecpEnableT0WtxTimer(void)
{

}

void ecpDisableT0WtxTimer(void)
{

}

void ecpStartT0WtxTimer(void)
{
	SCI->CTRL |= SCI_CTRL_CWT_INT_EN;	
	SCI->BGTCTRL |= SCI_BGTCTRL_CWT_EN;
}

void ecpStopT0WtxTimer(void)
{
	SCI->CTRL &= ~SCI_CTRL_CWT_INT_EN;
	SCI->BGTCTRL &= ~SCI_BGTCTRL_CWT_EN;
}

void ecpSuspendT0WtxTimer(void)
{

}

void ecpResumeT0WtxTimer(void)
{

}

void ecpInitT0TimeInterupt(memref timeInterupT0ddress)
{
	return;
}

void ecpClrT0WtxTimerOverFlow(void)
{

}

u32 ecpDivideT0WtxTimerFrequency(u32 bwi)
{
	return bwi;
}

#if HAL__IO__ISO7816_T1
void ecpSetT1WtxTimer(u8 T1BwtH, u8 T1BwtL)
{

}

void ecpClearT1WtxTimer(void)
{

}

void ecpEnableT1WtxTimer(void)
{

}

void ecpDisableT1WtxTimer(void)
{

}

void ecpStartT1WtxTimer(void)
{

}

void ecpStopT1WtxTimer(void)
{

}

void ecpSuspendT1WtxTimer(void)
{

}

void ecpResumeT1WtxTimer(void)
{

}

void ecpInitT1TimeInterupt(memref timeInterupT1ddress)
{
	return;
}

void ecpClrT1WtxTimerOverFlow(void)
{

}

u32 ecpDivideT1WtxTimerFrequency(u32 bwi)
{
	return bwi;
}
#endif
#if HAL__IO__ISO14443_TA
void ecpSetTaWtxTimer(u8 fwi)
{
   return;
}

void ecpClearTaWtxTimer(void)
{

}

void ecpEnableTaWtxTimer(void)
{

}

void ecpDisableTaWtxTimer(void)
{

}

void ecpStartTaWtxTimer(void)
{

}

void ecpStopTaWtxTimer(void)
{

}

void ecpSuspendTaWtxTimer(void)
{

}

void ecpResumeTaWtxTimer(void)
{

}

void ecpInitTATimeInterupt(memref timeInteruptAddress)
{
	return;
}

void ecpClrTaWtxTimerOverFlow(void)
{

}
#endif
//void TIM0_IRQHandler(void) SECT90(ifx_ram_code);
void TIM0_IRQHandler(void) /*interrupt 5*/ //interrupt 5 = TIMER1_INT
{
	io_WtxRequest();
}

#endif	//HAL__IO__TIMER
