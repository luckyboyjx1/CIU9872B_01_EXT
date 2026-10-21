#ifndef __SWP_HAL_h__
#define __SWP_HAL_h__

#include "ecpDefine.h"
#include "chipHead.h"

#define SWP_MAC_RNR                     0
#define SWP_MAC_RR                      1
#define SWP_MAC_STAT_RTR_RTS            0
#define SWP_MAC_STAT_ONGOING_SENDING    1

#define SWP_MAC_LAYER_ACTIVATED         0x11
#define SWP_MAC_LAYER_DEACTIVATED       0x00
#define SWP_MAC_LAYER_SUSPENDED         0x10

#define SWP_MAC_MAX_PAYLOAD             30

#define SWP_MAC_CHECK_RX                0x00
#define SWP_MAC_CHECK_TX                0x01

#define SWP_MAC_OK                      0x00

#define SWP_MAC_INIT_ERROR              0xFF70
#define SWP_MAC_ACTIVATE_ERROR          0xFF71
#define SWP_MAC_DEACTIVATE_ERROR        0xFF72
#define SWP_MAC_SETSTATUS_ERROR         0xFF80
#define SWP_MAC_RESENDFRAME_ERROR       0xFF90
#define SWP_MAC_MAX_PAYLOAD_ERROR       0xff91
#define SWP_MAC_NO_TX_FRAME_FREE        0xffa0
#define SWP_MAC_TOO_MUCH_ON_GOING_ERROR 0xffa1
#define SWP_MAC_RECEIVE_ERROR           0xffb0
#define SWP_MAC_SYNCHRONIZED_ERROR      0xffb1
#define SWP_MAC_RX_STAT_ERROR           0xffb2

#define SWP_EVT_INITIALIZE              0x100


#define SWP_LLC_ACT_IsSwpDeactivated( )         ( r_SWP_STAT & BIT_SET(SWP_STAT_DEACT_STATE_BIT) )

/// @brief Returns 1 if SWP is in Suspend state
#define SWP_LLC_ACT_IsSwpSuspended( )           ( r_SWP_STAT & BIT_SET(SWP_STAT_SUSP_STATE_BIT) )

/// @brief Returns 1 if SWP is in activated state
#define SWP_LLC_ACT_IsSwpActivated( )           ( ( r_SWP_STAT & ( BIT_SET(SWP_STAT_ACT_EVT_BIT) ) ) )

//#define SWP_MAC_NoBufferFree( ) ( g_SWP_MAC_Context.Nb_FrameToTransmit >= g_SWP_LLC_Context.WS )

typedef struct SWP_MAC
{
    u32                 Status;
    u8                  LastSent_Frame;         ///< number of last transmitted frame.
    u8                  Received_Frame;         /// number of last received frame
    volatile u8         Nb_FrameToTransmit;     ///< used to know which are the number of written frame in the peripheral. Must be between 4 -Sent_Frame
    volatile u32        TX_PreRNRState;
	u8                  CurRead;    // should be between [0 3]
    u8                  CurWrite;   // should be between [0 3]
    u8                  CptRead;    // It contains the value read during readblock
    u8                  CptWrite;   //contains the index of the written buffer.
}SWP_MAC_t;

#if HAL__IO__SWP
	extern SWP_MAC_t  g_SWP_MAC_Context;


	extern  void  Swp_FirstIrqHardWareHandle(void);
	extern  u8    Swp_GetIrqEventType(void);
	extern  void  Swp_HardWare_PullDownManagementS2(void);
	extern  u8    Swp_HardWare_PullUpManagementS1(u8 swpS1);


	extern  void hardWareCheckSwpState(void);

	extern u8 SWP_MAC_NoBufferFree(void);
	extern void Swp_EnableInterrupt(void);
	extern void Swp_DisableInterrupt(void);
	extern void Swp_RegisterFindActivate(void);
	extern void Swp_RegisterHandleForS1Appear(void);

	extern void swpControlConfig(void);
	extern void Swp_HardWare_StopSwp(void);
	extern void Swp_HardWare_StartSwp(void);

	extern void SwpHardwareHandleFullPower(void);

	/*************** Mac_Layer  extern function ****************/
	extern  void Swp_Mac_Init( void );
	extern  u32  Swp_Mac_Close( void );
	extern  void Swp_Mac_Open( void );
	extern  u8   Swp_Mac_SendFrame(PX8 sendAddr);
	extern  u32  Swp_Mac_ReceiveFrame(PX8 pSwpFrame, PX8 lengthAddr ,u8 rxBufNum);
	extern  u32  Swp_Mac_GetStatus( void )       ;
	extern  u32  Swp_Mac_SetStatus( u32 Status)  ;
	extern  u32  Swp_Mac_Deactivate( void )      ;
	extern  u32  Swp_Mac_Activate( void )        ;
	extern  u8   IsSwpTransmitting(void)          ;
	extern  u8   IsSwpReceived(void)              ;


	extern  u8  Get_TxState(u8 bufi);
	extern  u8  Get_RxState(u8 bufi);
	extern  void CTRL_SetTX_BuffIndexTo( u1 bufi, u1 TxState);
	extern  u8   ecpSwpMifareHandle(void);
	extern  u8   ecpSwpMifareHandleContinue(void);
#else
	#define  Swp_FirstIrqHardWareHandle()
	#define  Swp_GetIrqEventType()       0
	#define  Swp_HardWare_PullDownManagementS2()
	#define  Swp_HardWare_PullUpManagementS1( swpS1) 0
	#define  hardWareCheckSwpState()
	#define  SWP_MAC_NoBufferFree() 0
	#define  Swp_EnableInterrupt()
	#define  Swp_DisableInterrupt()
	#define  Swp_RegisterFindActivate()
	#define  Swp_RegisterHandleForS1Appear()
	#define  swpControlConfig()
	#define  Swp_HardWare_StopSwp()
	#define  Swp_HardWare_StartSwp()
	#define  SwpHardwareHandleFullPower()
	#define  Swp_Mac_Init(  )
	#define  Swp_Mac_Close(  ) 0
	#define  Swp_Mac_Open(  )
	#define  Swp_Mac_SendFrame( sendAddr) 0
	#define  Swp_Mac_ReceiveFrame( pSwpFrame,  lengthAddr , rxBufNum) 0
	#define  Swp_Mac_GetStatus(  )       0
	#define  Swp_Mac_SetStatus(  Status)  0
	#define  Swp_Mac_Deactivate(  )      0
	#define  Swp_Mac_Activate(  )        0
	#define  IsSwpTransmitting()         0
	#define  IsSwpReceived()             0
	#define  Get_TxState( bufi)             0
	#define  Get_RxState( bufi)             0
	#define  CTRL_SetTX_BuffIndexTo(  bufi,  TxState)
	#define  ecpSwpMifareHandle()        0
	#define  ecpSwpMifareHandleContinue() 0
#endif

#endif
