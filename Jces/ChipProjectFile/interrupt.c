#include "FunEntryConfig.h"
//#include "profile.h"
#include "bootPlaformConfigProfile.h"
#include "chipHead.h"
#include "MixMode.h"
#include "UartHal.h"
#include "SpiHal.h"

#pragma CODE("interrupt_Section")


// irq handler
void SCIRST_IRQHandler(void)
{
#if BOOTLOADER
	RealEntry.Irq1HandlerFunc();
#else
	//Uart_RstIrqHandlerProcess();
#endif
}

void SCIReceive_IRQHandler(void)
{
#if BOOTLOADER
	RealEntry.Irq2HandlerFunc();
#else
	//Uart_RxIrqHandlerProcess();
#endif
}

void Timer0_IRQHandler(void)
{
#if BOOTLOADER
	RealEntry.Irq3HandlerFunc();
#else

#endif
}

void SPI_IRQHandler(void)
{
#if BOOTLOADER
	RealEntry.Irq4HandlerFunc();
#else
	//use for spi rxtx interface
#if HAL__IO__SPI
	SPI_IRQHandlerProcess();
#endif

#endif
}

void I2C_IRQHandler(void)
{
#if BOOTLOADER
	RealEntry.Irq5HandlerFunc();
#else
#if HAL__IO__I2C
	I2C_IRQHandlerProcess();
#endif

#endif
}

void halt_handler (void)
{
	while(1);
}

void SMU_IRQHandler (void)
{
	while(1);
}

void RNG_IRQHandler (void)
{
	while(1);
}

void WDT_IRQHandler (void)
{
	while(1);
}

void PUF_IRQHandler (void)
{
	while(1);
}

void FLASH1_IRQHandler (void)
{
	while(1);
}

void FLASH2_IRQHandler (void)
{
	while(1);
}


