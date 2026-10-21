/*
 * FunEntryConfig.c
 *
 *  Created on: 20190314
 *      Author: ymf
 */
#include "FunEntryConfig.h"
#include "bootPlaformConfigProfile.h"
extern int __main(void);
#if BOOTLOADER
__attribute__( ( section( "FunBootEntry_data" ) ) )

FunctionEntry_t	CONST	FuncBootEntry	={
	app_main,
	CT_IRQHandlerProcess,
	TIMER0_IRQHandlerProcess,
	ALGORITHM_IRQHandlerProcess,
	SECURITY_IRQHandlerProcess,
	TIMER1_IRQHandlerProcess,
	TIMER2_IRQHandlerProcess,
	SPI_IRQHandlerProcess,
	GPIO_IRQHandlerProcess,
	HardFault_IRQHandlerProcess,
	//I2C_IRQHandlerProcess,

	app_main,
	CT_IRQHandlerProcess,
	TIMER0_IRQHandlerProcess,
	ALGORITHM_IRQHandlerProcess,
	SECURITY_IRQHandlerProcess,
	TIMER1_IRQHandlerProcess,
	TIMER2_IRQHandlerProcess,
	SPI_IRQHandlerProcess,
	GPIO_IRQHandlerProcess,
	HardFault_IRQHandlerProcess,
	//I2C_IRQHandlerProcess,
 /*DownLoad_ENKEY           0x10(0x010)*/
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 /*PORT_MODE             0x01(0x020)*/
	0x00, 
 /*EN_MODE               0x01(0x021)*/
	0x01, 	
};

//__weak int app_main(void)
//{
//	return 0;
//}

//__weak void Timer0_IRQHandlerFunc(void)
//{
//	
//}
//__weak void GPIO_IRQHandler(void)
//{
//	
//}

//__weak void SPI_IRQHandlerFunc(void)
//{
//	
//}

#endif
