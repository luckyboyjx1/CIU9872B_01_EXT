#include "UartHal.h"
#include "chipHead.h"
#if HAL__PLATFORM__SIMULATOR
	#include <uart/UartPort.h>
#endif
#if HAL__IO__ISO7816_T0

#define		NO_PROCESS_616C			0
#define		PROCESS_61				1
#define		PROCESS_6C				2

u8	X_A	process616c;
u8	X_A	process616cLength;


volatile u8 g_Iso7816Reset;//ATR sent or not

void ecpUart_EnRstIrq(void)
{
}
void ecpUart_DisRstIrq(void)
{   
}

void ecpUart_EnRxIrq(void)
{
	
}
void ecpUart_Init(void)
{

}

void ecpUart_T0Init(void)
{

}

void ecpUart_SendDirectTs(void)
{
	gAtrBuf[0] = ATR_T0_DIRECT_TS;
}

void ecpUart_SendinverseTs(void)
{
	gAtrBuf[0] = ATR_T0_INVERSE_TS;
}

void ecpUart_TsDelay(void)
{

}

void ecpUart_AtrDelay(void)
{
	
}

void ecpUart_SendRemaindAtr(PX8 atrBuf, u8 atrLen, u8 type)    SMALL_MODE
{
	ecpUart_TxMulBytes(atrBuf, atrLen, SEND_ATR);
}

u8 ecpUart_CheckFiDi(u8 FiDi)
{
	return 1;
}

void ecpUart_SetFiDi(u8 FiDi)
{

}

/*
void ecpUart_SetRecieveMode(void)
{

}
*/

void ecpUart_RxByte(PX8  buf)
{

}

void ecpUart_RxMulBytes(PR8 buf, u16 len, u8 flag)
{
	uartPort_rxBytes(buf, len, flag);
}

/*
void ecpUart_SetTransmitMode(void)
{

}

void ecpUart_TxByte(PX8 byteData)
{

}
*/

void ecpUart_TxMulBytes(PR8 buf, u16 len, u8 flag)	SMALL_MODE
{
	uartPort_txBytes(buf, len, flag);
}

void ecpUart_EnterSleepMode(void)
{

}

void ecpUart_GuardTime(void)
{

}

void ecpUart_StopTx(void)
{
	
}

u8 ecpUart_TxRxRunning(void)
{
	return 0;
}
void ecpUart_ClearFifo(void)
{
}
boolean ecpUart_IsReset(void)
{
}

boolean ecpUart_WarnReset(void)
{
}

void ecpUart_ClearResetFlag(void)
{
}

boolean ecpUart_IsRisingState(void)
{
	return true;
}

void ecpUart_CloseInterrupt(void)
{

}

void ecpUart_OpenInterrupt(void)
{

}
void ecpUart_SetRxLength(u8 len)
{
	//r_IART_RX_SIZE		= len;
}
boolean ecpUart_DiscoverError(void)
{
//    if ( r_IART_STAT & ( 0x18 ) )
//    {
//		// ??|??：???′?′??：?a IART???：?????：?23???|??━?? Overrun Error o??：?a Frame Error
//        // write error status in NVM
//        //reg_data = (u1)r_IART_STAT;
//        //NVM_WriteZone( &reg_data, 0x01, & ( g_ISO7816_3_Perso->IartErrorStatus ) );
//      
//        // If error is enabled and error triggers an event in perso, then do a software reset
//        if ( ( r_IART_STAT & ( 0x18 ) ) != 0x00 )
//        {
//            // Software reset
//            r_SEC_CTRL_BIT(SEC_CTRL_INF_SW_RST_BIT) = 1;
//            r_SEC_CTRL_BIT(SEC_CTRL_SW_RST_REQ_BIT) = 1;
//        }

//        // Clear communication errors
//        r_IART_STAT = 0;
//				return true;
//    }
//		if(r_IART_STAT & BIT_SET(IART_STAT_PAR_ERR_BIT))
//	  {
//		    r_IART_STAT = r_IART_STAT & (~BIT_SET(IART_STAT_PAR_ERR_BIT));
//			  return true;
//	  }
//		return false;
}

#endif  //HAL__IO__ISO7816_T0

#if HAL__IO__ISO7816_T1

u16 ecpUartCalT1Crc(PX8 buf, u16 length)
{
	return 0;
}

void ecpUartT1_SetCrcCal(void)
{

}

void ecpUart_T1Init(void)
{

}

u16 ecpUART_RxBuffer(PR8 pData, u16 pDataSize, PX16 fullness, u16 cwt, u8 isEnterSleepMode)
{
	return 0;
}

u16 ecpUart_TxBuffer(PR8 pData, u16 pDataSize, u16 fullness, u8 txOptions)
{
	return 0;
}
#endif
