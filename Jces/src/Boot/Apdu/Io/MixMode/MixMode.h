/*
 * MixMode.h
 *
 *  Created on: 2015Äê1ÔÂ13ÈÕ
 *      Author: linguoming
 */

#ifndef _MIX_MODE_H_
#define _MIX_MODE_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "7816Util.h"

typedef struct
{
	 u8 T0PpsInprogress	;
	 u8 T0ReceiveCmd    ;
	 u8 T0Receive       ;
	 u8 T1Receive		;
	 u8 SpiReceive		;
	 u8 I2cReceive		;
	 u8 TaReceive		;
	 u8 SwpReceive		;
} MixModeStatus; 

#define NO_INTERFACE_ACTIVED                  0X00
#define ISO7816_INTERFACE                     0X01
#define SPI_INTERFACE                         0X02
#define IIC_INTERFACE                         0X04
#define SWP_INTERFACE                         0X08
#define USB_INTERFACED                        0X10

#if BOOT__IO__MIX_MODE
	#define	SWP_MOST_SEQUENCE					5

	extern void 	MixMode_UartActive(void);
	extern void 	MixMode_UartDeactive(void);
	extern void 	MixMode_TaActive(void);
	extern void 	MixMode_SwpActive(void);
	extern void 	MixMode_UartReceive(void);
	extern void 	MixMode_TaReceiveCommand(void);
	extern void 	MixMode_SwpReceiveCommand(void);
	extern void 	MixMode_Init(void);
	extern boolean	MixMode_IsSupport(void);
	extern void 	MixMode_EnterSleepMode(void);
	extern void 	MixMode_SetCommand(PX8 commandBuf, u16 size);
	extern void		MixMode_GetCommand(ppIso7816Util_Command_t * ppCommand);
	extern void 	MixMode_CopyCommad(void);
	extern u8 		MixMode_GetCurProtocol(void);
	extern void 	MixMode_UartInit(void);
	extern void 	MixMode_UartReadyRx(u16 len);
	extern void     MixMode_UartReadyRxData(void);
	extern void 	MixMode_UartRxMulBytes(PR8 buf, u16 len, u8 flag);
	extern  u8      MixMode_UartEnterSleepMode(void);
	extern  u8      MixMode_isUartReceiveHeader(void);
	extern  u16      MixMode_GetUartRxLength(void);
	extern  void    MixMode_WarnReset(u8 protocolType);
	extern  u8      MixMode_InterfaceIsConcurrency(u8 protocolType);
	extern  void      MixMode_SetInterfaceActivedFlag(u8 interfaceType);
	extern  u8		MixMode_GetInterfaceActivedFlag(u8 interfaceType);
#else
	#define MixMode_UartActive()
	#define	MixMode_UartDeactive()
	#define MixMode_TaActive()
	#define MixMode_SwpActive()
	#define MixMode_UartReceive()
	#define MixMode_UartReceiveCommand()
	#define MixMode_TaReceiveCommand()
	#define MixMode_SwpReceiveCommand()
	#define MixMode_Init()
	#define MixMode_IsSupport()											false
	#define MixMode_SetCommand(commandBuf, size)
	#define MixMode_GetCommand(ppCommand)
	#define MixMode_CopyCommad()
	#define MixMode_EnterSleepMode()
	#define MixMode_GetCurProtocol()									PROTOCOL_NONE
	#define MixMode_UartInit()
	#define	MixMode_UartReadyRx(len)
	#define MixMode_UartReadyRxData()
	#define MixMode_UartRxMulBytes(buf, len, flag)						ecpUart_RxMulBytes(buf, len, flag)
	#define MixMode_UartEnterSleepMode()         0
	#define MixMode_isUartReceiveHeader()        1
	#define MixMode_GetUartRxLength()            0
	#define MixMode_WarnReset(protocolType)
    #define MixMode_InterfaceIsConcurrency(protocolType)
    #define MixMode_SetInterfaceActivedFlag(interfaceType)
	#define MixMode_GetInterfaceActivedFlag(interfaceType)
#endif

#endif /* IOMIX_H_ */
