#include "SwpHal.h"
#include "SysConfigHalRam.h"
#include "chipHead.h"
#include "ecpReadWrite.h"
//#include "Swp_Llc_Layer.h"
#if HAL__IO__SWP
SWP_MAC_t  g_SWP_MAC_Context;
#define SWP_IRQ_EVENT_DEACTIVATE         0x00        
#define SWP_IRQ_EVENT_ACTIVATE           0x01
#define SWP_IRQ_EVENT_SEND               0x02
#define SWP_IRQ_EVENT_RECEIVE            0x03
#define SWP_IRQ_EVENT_RFU                0x04

u8 SWP_MAC_NoBufferFree(void)
{

	return 0;
}

void Swp_FirstIrqHardWareHandle(void)
{

}

u8  Swp_GetIrqEventType(void)
{
	return 0;
}

void Swp_EnableInterrupt(void)
{

}

void Swp_RegisterFindActivate(void)
{

}

void Swp_RegisterHandleForS1Appear(void)
{

}

void swpControlConfig(void)
{


}

u8 isActivateActSent(void)
{

	return 0;
}

void Swp_StopSwpRun(void)
{

}

void Swp_StartSwpRun(void)
{

}

void Swp_CheckSuspendAndHandle(void)
{

}

memref Swp_GetSendRegisterAddr(void)
{
	return 0;
}

memref Swp_GetReceiveRegisterAddr(void)
{
	return 0;
}

void Swp_HardWare_PullDownManagementS2(void)
{

}

u8 Swp_HardWare_PullUpManagementS1(u8 swpS1)
{
    return  0;
}



void hardWareCheckSwpState(void)
{
}

void HardWare_PullDown_S2(void)
{
}

void Swp_HardWare_ForSuspend(void)
{
}


void SwpHardwareHandleFullPower(void)
{
}

/**********************************following functions are for SWP_MAC_LAYER**********************************************/

void CTRL_SetRX_BuffIndexToRTR( u1 bufi)
{

}

void CTRL_SetTX_BuffIndexTo( u1 bufi, u1 TxState)
{

}

u8 Count_AvailabeTxState(void)
{
    return 0;
}

u32 Check_TxRxStatus ( u8 TxOrRx)
{
    return 0;
}


u8 Get_TxState(u8 bufi)
{
	return 0;
}

u8 Get_RxState(u8 bufi)
{
	return 0;
}

void Swp_Mac_Init( void )
{
    return ;
}

void Swp_Mac_Open( void )
{

}

u32 Swp_Mac_Close( void )
{
    return 0;
}

u32 Swp_Mac_ReceiveFrame(PX8 pSwpFrame, PX8 lengthAddr ,u8 rxBufNum)
{
 	return 0;
}

u8 Swp_Mac_SendFrame(PX8 sendAddr)
{
    return 0;
}


u32 Swp_Mac_GetStatus( void )
{
	return 0;
}

u32 Swp_Mac_SetStatus( u32 Status)
{
	return 0;
}


u32 Swp_Mac_Deactivate( void )
{
	return 0;
}


u32 Swp_Mac_Activate( void )
{
	return 0;
}

u8 IsSwpTransmitting(void)
{
	return 0;
}

u8 IsSwpReceived(void)
{
	return 0;
}
















u8 ecpSwpMifareHandle(void)
{
    return 1;
}
u8 ecpSwpMifareHandleContinue(void)
{
    return 1;
}
#endif
