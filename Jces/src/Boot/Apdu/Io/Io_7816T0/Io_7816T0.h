#ifndef __IO_7816T0_H__
#define	__IO_7816T0_H__

#include "DataOperation.h"
#include "7816Util.h"
#include "SysConfigProfile.h"

#define ACK_NONE 					0
#define ACK_INS						1

#define	RESP_61						0xFF
#define	RESP_6C						0x00
#define	RESP_NO616C					0xAA
#define RESP_HAVE_616C				0x11

#define T0_COMMAND_HEADER_LENGTH	5

#define SW_BYTES_REMAINING_00       0x6100
#define SW_CORRECT_LENGTH_00        0x6C00

#define	ERROR_PPS_RESPONSE			0x00FF00FF


typedef struct
{
	u8						isPpsCommandReceived;
	u8						isDataReceived;
	u16						sw1sw2;
	u8 						b6C61;
	u8						ResponseOffset;
	u8						iso7816T0_PreHeader[ISO7816UTIL_HEADER_LENGTH];
	u8						iso7816T0_CurHeader[ISO7816UTIL_HEADER_LENGTH];
	ppIso7816Util_Command_t command;
} iso7816T0_Control_t, *piso7816T0_Control_t;

extern u16   			X_A		g_sw1sw2;

#if HAL__IO__ISO7816_T0
	extern  void iso7816T0_GetT0Control(piso7816T0_Control_t * toControl);
	extern  void iso7816T0_SetT0Control(piso7816T0_Control_t toControl);
	extern  void iso7816T0_Init(void);
	extern  void iso7816T0_SendStatus(void);
	extern  u8 iso7816T0_HandleCommand(void);
	extern	void iso7816T0_RecieveCommand(void);
	extern  void iso7816T0_FetchCommand(PX8 buf);
	extern	u16	 iso7816T0_Receive(void);
	extern	u16  iso7816T0_Transmit(void);
	extern  void iso7816T0_SendStatusAndReceiveCommand(void);
	extern	u32  iso7816T0_GetT0WtxTimerValue(void);
	extern 	u8 iso7816T0_IsGetResponse(void);
	extern 	piso7816T0_Control_t get_g_pT0Control(void);
	extern void iso7816T0_Clrb6C61(void);
#else
	#define	iso7816T0_GetT0Control()
	#define iso7816T0_SetT0Control()
	#define	iso7816T0_Init()
	#define	iso7816T0_SendStatus()
	#define	iso7816T0_HandleCommand()																0
	#define	iso7816T0_RecieveCommand()
	#define iso7816T0_FetchCommand(buf)
	#define	iso7816T0_Receive()																		0
	#define	iso7816T0_Transmit()																	0
	#define iso7816T0_SendStatusAndReceiveCommand()
	#define	iso7816T0_GetT0WtxTimerValue()															0

#endif

#endif
