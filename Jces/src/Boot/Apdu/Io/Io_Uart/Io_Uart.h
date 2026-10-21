/*
 * Io_Uart.h
 *
 *  Created on: 2015Äê1ÔÂ12ÈÕ
 *      Author: linguoming
 */
#ifndef IO_UART_H_
#define IO_UART_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "UartHal.h"
#include "7816Util.h"

#define IO_UART_PPS_BYTE							0xFF
#define IO_UART_PPS_MIN_LEN							3


#if HAL__IO__ISO7816_UART
	typedef struct
	{
		u8 ppsAllowed 		:1;
		u8 isT0Protocol		:1;
	} UartSatus; /* Declare a status byte*/

	extern void   IoUart_InitVar(void);
	extern void 	IoUart_Init(void);
	extern void 	IoUart_DisablePps(void);
	extern u8		Io_Uart_AnalysPps0(u8 pps0);
	extern boolean	IoUart_IsPpsAllowed(void);
	extern boolean 	IoUart_Pps(PX8 ppsBuffer, u8 len);
	extern void 	IoUart_SetPps0(u8 pps0);
	extern void 	IoUart_SetPps1(u8 pps1);
	extern boolean  IoUart_isPpsInProgress(void);
	extern boolean  IoUart_isT0Protocol(void);
	extern u8		IoUart_GetD(void);
	extern u16  	IoUart_GetF(void);
	extern u8 IoUart_isActived(void);
	extern u8 IoUart_Busy(void);
	extern void IoUart_ClearFifo(void);
#else
	#define IoUart_InitVar()
	#define IoUart_Init()
	#define IoUart_DisablePps()
	#define Io_Uart_AnalysPps0()
	#define IoUart_IsPpsAllowed()
	#define IoUart_Pps(ppsBuffer, len)
	#define IoUart_SetPps0(pps0)
	#define IoUart_SetPps1(pps1)
	#define IoUart_isPpsInProgress()
	#define IoUart_isT0Protocol()
	#define IoUart_GetD()
	#define IoUart_GetF()
	#define IoUart_isActived()
#endif

#endif
