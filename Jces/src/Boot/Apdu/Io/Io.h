#ifndef __IO_H__
#define	__IO_H__

#include "DataOperation.h"
#include "7816Util.h"
#include "Io_Swp.h"
#include "bootConfigProfile.h"
#include "profile.h"	//remove warning,dengyiwen,2022/9/27

#define	IO_NO_STATUS							U32t16(0xFC5A)
#define ECPIO_STATUS_INIT       0x00     //
#define ECPIO_STATUS_COLDRESET      0x01     //
#define ECPIO_STATUS_WARMRESET      0x02     //

//  extern  u8	 X_A   io_isWarmReset;
  extern  volatile u8	 X_A   g_Iso7816Reset;//modify by jiangxiaohui@20220324 
	
#if BOOT__IO__PROTOCOL
	extern	u8 	 io_GetLogicalChannel(void);
	extern	void io_GetIoBuffer( ppIso7816Util_Buffer_t *ppIoBuffer );
	extern	void io_GetCommand( ppIso7816Util_Command_t * ppCommand );
	extern	void io_GetResponse( ppIso7816Util_Response_t *ppResponse );
	extern  void io_SetCommand(PX8 commandBuf, u16 size);
	extern	void io_Init(u8 isDeselect)    SMALL_MODE;
	extern  void io_ReceiveFirstCommand(void);
	extern 	void io_FetchCommand(PX8 buf);
	extern	u16  io_Receive(void);
	extern	void io_SendStatusAndReceiveCommand(u16 status);
#if (JAVA_CARD_UICC||BOOTLOADER_PRO)//modify by jiangxiaohui@20220324 ·Ö²ã
	extern  void io_SendStatusAndReceiveCommandSingle(u16 status);
#endif
	extern	u8   io_GetCurrentProtocol(void);
	extern  void io_SetCurrentProtocol(u8 type);
	extern	void io_SetResPrameter( u16 resLength, u16 Lr, u16 SW1SW2);
	extern	void io_SuspendAllInterrupts(void);
	extern	void io_ResumeAllInterrupts(void);
	#if		HAL__IO__ISO14443_TA
			extern  u8	 io_GetSakComplete(void);
			extern  void io_SetSakComplete(u8 sakComplete);
			extern  u8	 io_GetSakIncomplete(void);
			extern  void io_SetSakIncomplete(u8 sakIncomplete);
			extern  u8	 io_GetUid(PX8 uidAry);
			extern  void io_SetUid(PM8 uidAry);
			extern  u16  io_GetAtqa(void);
			extern  void io_SetAtqa(PM8 atqaArray);
	#else
			#define io_GetSakComplete()																0
			#define io_SetSakComplete(sakComplete)
			#define io_GetSakIncomplete()															0
			#define io_SetSakIncomplete(sakIncomplete)
			#define io_GetUid(uidAry)																0
			#define io_SetUid(uidAry)
			#define io_GetAtqa()																	0
			#define io_SetAtqa(atqaArray)
	#endif
	extern  void io_WtxRequest(void);
	
#else
	#define	io_GetLogicalChannel()															0
	#define	io_GetIoBuffer(ppIoBuffer)
	#define	io_GetCommand(ppCommand)
	#define	io_GetResponse(ppResponse)
	#define	io_Init(isDeselect)
	#define io_ReceiveFirstCommand()
	#define io_FetchCommand(buf)
	#define	io_Receive()																	0
	#define	io_SendStatusAndReceiveCommand(status)
	#define	io_GetCurrentProtocol()															0
	#define	io_SetResPrameter(resLength, Lr, SW1SW2)
	#define	io_SuspendIoInterrupts()
	#define	io_ResumeIoInterrupts()
	#define io_GetSakComplete()																0
	#define io_SetSakComplete(sakComplete)
	#define io_GetSakIncomplete()															0
	#define io_SetSakIncomplete(sakIncomplete)
	#define io_GetUid(uidAry)																0
	#define io_SetUid(uidAry)
	#define io_GetAtqa()																	0
	#define io_SetAtqa(atqaArray)
	#define io_WtxRequest()
#endif

#endif
