#ifndef __IO_SWP_H__
#define	__IO_SWP_H__

#include "ecpDefine.h"
#include "ecpReadWrite.h"
#include "SysConfigProfile.h"

#if BOOT__IO__SWP

#define RF_COMMUNICATION_SWITCH_OFF				0
#define RF_COMMUNICATION_SWITCH_ON				1

#if HAL__IO__SWP__LIB


extern u8 Swp_Buffer[];
extern u8 g_SwpHciEvents[];
extern u8 Swp_UiccContactlessState;

extern u8 Swp_EnterSleep(void);
extern void Swp_Init(void);
extern u8 Swp_HandleCommand(void);
extern void Swp_SendStatus(void);
extern u8 Swp_isReceCommand(void);
extern u8      	Swp_TrigEvent( void );
extern u8 get_swp_protocol_type(void);

#else

	extern u8       g_SWP_S1;

	extern void     swp_Init(void);
	extern u8       swp_HandleData(void);

	extern u8       isSwpInterfaceEnabled(void);
	extern void     setSwpInterfaceEnabled(u8 ablity);

	extern void   	swp_SetEvent(u8 eventType);
	extern void   	swp_FetchCommand( u8 *pCommand );
	extern void   	 swp_ReceiveCommand(void);
	extern u16    	swp_ReceiveData(void);
	extern u8       swp_HandleReceive(void);
	extern u16   	  swp_SendData(void);
	extern void  	  swp_SendStatus(void);
	extern boolean  swp_GetEventNeedHandle(void);
	extern boolean	swp_HandleCommand(void);

	extern u8    	  get_swp_protocol_type(void);
	extern void    	swp_ResetSwpConfigData(void);
	extern u8      	Swp_TrigEvent( void );
	extern void     swp_PullDownManagementS2(void);
	extern  u8   io_GetSwpProtocolType(void);
	extern  u8   io_HciNoParaProcess(u8 apiType);
	extern  u8   io_HciOneParaProcess(u8 type,u16 paraId);
	extern  void io_HciMulParaProcess(u8 type,u8 timeout,u16 len,u16 offset,PM8 dataAddr);
	extern  void io_GetReceiveBufferHCI(PM8 addr);
	extern  void io_Swp_Hci_TransactionEvent_6paras(PM8 paraAddr,u16 paraOff,u16 paraLen,PM8 aidAddr,u16 aidOff,u16 aidLen);
	extern  void io_Swp_Hci_TransactionEvent_3paras(PM8 paraAddr,u16 paraOffset,u16 paraLen);
	extern  u8 getSwpSigner(void);

#endif    // end of HAL__IO__SWP__LIB

#else
	#define     getSwpSigner()                                                                                  0
	#define     swp_Init()
	#define     swp_HandleData()   																				0
	#define     isSwpInterfaceEnabled() 																		0
	#define     setSwpInterfaceEnabled()
	#define  	swp_SetEvent()
	#define  	swp_FetchCommand()
	#define  	swp_ReceiveCommand()
	#define  	swp_ReceiveData() 																				0
	#define  	swp_SendData() 																					0
	#define  	swp_SendStatus()
	#define     swp_HandleCommand()  																			0
	#define  	get_swp_protocol_type() 																		0
	#define  	swp_ResetSwpConfigData()                                                                        0
	#define  	Swp_TrigEvent()   																				0
	#define     swp_PullDownManagementS2()    																	0
	#define 	io_GetSwpProtocolType()                 														0
	#define 	io_HciNoParaProcess(apiType)                 													0
	#define 	io_HciOneParaProcess(type,paraId)                 												0
	#define 	io_HciMulParaProcess(type,timeout,len,offset,dataAddr)                 							0
	#define 	io_GetReceiveBufferHCI(addr)                 													0
	#define 	io_Swp_Hci_TransactionEvent_6paras(paraAddr,paraOff,paraLen,aidAddr,aidOff,aidLen)              0
	#define 	io_Swp_Hci_TransactionEvent_3paras(paraAddr,paraOffset,paraLen)                 				0
	#define     swp_GetEventNeedHandle()                                                                        0
	#define     swp_HandleReceive()                                                                             0
	#define 	Swp_EnterSleep()        																		0
	
#endif     // end of BOOT__IO__SWP

#endif    // end of __IO_SWP_H__

