/*
 * IoMix.c
 *
 *  Created on: 2015年1月13日
 *      Author: linguoming
 */
#include "MixMode.h"
#include "Io_Uart.h"
#include "Io_7816T0.h"
#include "Io_7816T1.h"
#include "Io_14443TaL4.h"
#include "IoBuffer.h"
#include "BootNoAreaInitRam.h"
#include "Io_Swp.h"
#include "Io_SPI.h"
#include "Io_I2C.h"
#include "io.h"

#if JAVA_CARD_UICC
	#include "packageloc.h"
	#include "SCPInterface.h"
	#include "method.h"
	#include "interpreter.h"
	#include "stack.h"
	#include "nativemethod_impl.h"
#endif

#if BOOT__IO__MIX_MODE

MixModeStatus				X_A	g_mixModeStatus;
static u8							X_A	g_swpOpeSequence;
static pIso7816Util_Command_t		X_A g_curCommand;

static PX8							X_A g_pUartDataField;
static u8							X_A	g_uartCmd[ISO7816UTIL_HEADER_LENGTH];
static u16							X_A g_uartCmdOffset;
volatile static u16					X_A g_uartRxLength;
static u8							X_A g_ppsLength;
static u16							X_A	g_uartRxSegLen;
static u16							X_A	g_uartExpectRxLen;

static u8							X_A	g_interfaceActivedFlag; //interface actived flag;  :01 7816; 02 spi ;04 iic ; 08 swp ;10 usb



/****************************************interrupt function**********************************
*********************************************************************************************/
void MixMode_UartActive(void)
{
	u8	timerType;
	u8	tempBufPckType;

	timerType		= wtxTimer_GetCurWtxTimerType();
	tempBufPckType	= iso7816Util_GetBufPckType();
	iso7816Util_SetBufPckType(UART_BUF_PCK_TYPE);
	
	MixMode_UartInit();
	
	wtxTimer_SetCurWtxTimerType(timerType);
	iso7816Util_SetBufPckType(tempBufPckType);
}

void MixMode_UartDeactive(void)
{
	u8	timerType;
	
	timerType			= wtxTimer_GetCurWtxTimerType();
	
	ecpUart_StopTx();
		
	if(true == IoUart_isT0Protocol())
	{
		wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
	}
	else
	{
		wtxTimer_SetCurWtxTimerType(T1_WTX_TIMER);
	}
	
	wtxTimer_WtxTimerStop();
	
	wtxTimer_SetCurWtxTimerType(timerType);
}

u8 MixMode_UartEnterSleepMode(void)
{
	return ( (g_mixModeStatus.T0Receive || g_mixModeStatus.T0PpsInprogress) ||
	         (wtxTimer_Tunning()) );
}

void MixMode_TaActive(void)
{

}

void MixMode_SwpActive(void)
{
	u8	tempBufPckType;

	tempBufPckType		= iso7816Util_GetBufPckType();

	iso7816Util_InitClBufPck();

	g_mixModeStatus.SwpReceive		= false;
	iso7816Util_SetBufPckType(tempBufPckType);
}

u8 MixMode_isUartReceiveHeader(void)
{
	if(g_mixModeStatus.T0Receive == true)
	{
		return 1;
	}
	return 0;
}

void MixMode_UartClearFifo(void)
{
    IoUart_ClearFifo();
    ecpMem_Set(g_uartCmd,0,5);
	g_uartRxLength =0;

}

void MixMode_UartReceive(void)
{
	boolean	isHandlePps;
	u8		timerType;
	u16		remaindLen;
	u8      rxLen;
	isHandlePps		= false;
	if(true == ecpUart_DiscoverError())
	{
		 return;
	}
	if(true == g_mixModeStatus.T0ReceiveCmd)
	{
//		if(g_Iso7816ReceiveCommandflag==0)
		{
			//rxLen = ecpUart_getRxLen();  //modify by weipingjiao@20240318 修改97波特率在CLOUD2700R的通信问题
			ecpUart_RxMulBytes(&g_uartCmd[g_uartRxLength], g_uartRxSegLen, RECEIVE_COMMAND);
			g_uartRxLength +=  g_uartRxSegLen;
					
			if((IO_UART_PPS_BYTE == g_uartCmd[0]) && (true == IoUart_IsPpsAllowed()))
			{
				g_mixModeStatus.T0PpsInprogress		= true;
				ecpUart_RxMulBytes(&g_uartCmd[g_uartRxLength], 1, RECEIVE_PPS);
				g_uartRxLength		= U16_A_U16(g_uartRxLength, 1);
			}
	  }
//		else
//		{
//			 ecpUart_RxMulBytes(&g_uartCmd[0], 0x05, RECEIVE_COMMAND);
//			 g_uartRxLength +=  0x05;
//		}
		//process pps
		if((true == g_mixModeStatus.T0PpsInprogress))
		{
			if(2 == g_uartRxLength)
			{
				g_ppsLength		= U8_A_U8(g_ppsLength, Io_Uart_AnalysPps0(g_uartCmd[ISO7816UTIL_INS_OFFSET]));
				ecpUart_RxMulBytes(&g_uartCmd[g_uartRxLength],(g_ppsLength - g_uartRxLength), RECEIVE_PPS);
				g_uartRxLength = g_ppsLength;
			}
			
			if(g_ppsLength == g_uartRxLength)
			{
				if(true == IoUart_isT0Protocol())
				{
					iso7816T0_RecieveCommand();
				}
				else
				{
					iso7816T1_RecieveCommand();
				}
				if(true == IoUart_isT0Protocol())
				{
					wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
					MixMode_UartReadyRx(ISO7816UTIL_HEADER_LENGTH);
				}
				else
				{
					wtxTimer_SetCurWtxTimerType(T1_WTX_TIMER);
				}
				
				wtxTimer_WtxTimerInit();
				
				g_uartCmdOffset						= 0;
				g_uartRxLength						= 0;
				g_mixModeStatus.T0PpsInprogress		= false;
			}
			isHandlePps		= true;
		}
		else
		{
			//command receive
			if(ISO7816UTIL_HEADER_LENGTH == g_uartRxLength)
			{
				timerType	= wtxTimer_GetCurWtxTimerType();
				
				if(true == IoUart_isT0Protocol())
				{
					g_mixModeStatus.T0Receive	= true;
					wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
				}
				else
				{
					g_mixModeStatus.T1Receive	= true;
					wtxTimer_SetCurWtxTimerType(T1_WTX_TIMER);
				}
				
				wtxTimer_WtxTimerStart();
				wtxTimer_SetCurWtxTimerType(timerType);
				
				g_uartCmdOffset			= 0;
				g_uartRxLength			= 0;
			}
		}
	}
	else
	{
		ecpUart_RxMulBytes(&g_pUartDataField[g_uartRxLength], g_uartRxSegLen, RECEIVE_DATA);
//		memcpy(&g_pUartDataField[g_uartRxLength],COMBUF,g_uartRxSegLen);
		g_uartRxLength		= U16_A_U16(g_uartRxLength, g_uartRxSegLen);
	}
	
	if(false == isHandlePps)
	{
		remaindLen			= U16_S_U16(g_uartExpectRxLen, g_uartRxLength);
		if((remaindLen < MAX_RX_LENGTH_PER_INT) && (0 != remaindLen))
		{
			MixMode_UartReadyRx(remaindLen); //modify by weipingjiao@20240318 修改97波特率在CLOUD2700R的通信问题
			//ecpUart_SetRxLength(remaindLen);
		}
	}
}

void MixMode_TaReceiveCommand(void)
{

}

void MixMode_SwpReceiveCommand(void)
{
	u8	tempBufPckType;

	tempBufPckType		= iso7816Util_GetBufPckType();

	iso7816Util_SetBufPckType(CL_BUF_PCK_TYPE);

	//if(0x2 == swp_HandleReceive())
	//{
	    //g_mixModeStatus.SwpReceive		= true;
	//}
	iso7816Util_SetBufPckType(tempBufPckType);
}
void MixMode_SpiReceiveCommand(void)
{
	u8	tempBufPckType;

	tempBufPckType		= iso7816Util_GetBufPckType();

	iso7816Util_SetBufPckType(SPI_BUF_PCK_TYPE);
  if(Spi_isReceCommand())
	{
		 g_mixModeStatus.SpiReceive = true;
	}

	iso7816Util_SetBufPckType(tempBufPckType);
}


void MixMode_WarnReset(u8 protocolType)
{
	
#if JAVA_CARD_UICC
	
	PF8	D_A	methodAddr;
	u8 basicChannel;
	u8 channelNum;
	u8 channelOpenFlag;
	u8 i;
#if BOOT__IO__ISO7816
	if(protocolType == PROTOCOL_T0)
	{
		basicChannel = ISO_BASIC_CHANNEL_NUM;
		channelNum = ISO_MAX_LOGICAL_CHANNELS;
	}
#endif
#if BOOT__IO__SPI
	if(protocolType == PROTOCOL_SPI)
	{
		basicChannel = SPI_BASIC_CHANNEL_NUM;
		channelNum = SPI_MAX_LOGICAL_CHANNELS;
	}
#endif
#if BOOT__IO__I2C
	if(protocolType == PROTOCOL_I2C)
	{
		basicChannel = I2C_BASIC_CHANNEL_NUM;
		channelNum = I2C_MAX_LOGICAL_CHANNELS;
	}
#endif
	channelOpenFlag = 0;
    for(i=(basicChannel+1);i<(basicChannel+channelNum);i++)
    {
    	if(channelInfo.channel_status[i] == CHANNEL_OPEN)
    	{
    		channelOpenFlag = 1;
    		break;
    	}
    }
	if (( (PX_R_U16(&channelInfo.selected_context[U8_M_U8((basicChannel),2)]) != NULL_CONTEXT) && (channelInfo.channel_status[basicChannel] == CHANNEL_OPEN))|| (channelOpenFlag == 1))
	{
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), 0x00);

		for (i=basicChannel; i<(basicChannel + channelNum); i++)
		{
			selected_channel = i;
			closeSecuritychannel(SC_TERMINATED);
		}

		io_SetCurrentProtocol(protocolType);

		methodAddr = PF8_A_U16(MEMtPF8(getPackageComponentLocation(IMPL_PKG_ID, COMPONENT_METHOD)), WarmResetCallBack_METHOD_ADDR);

		callmethod(methodAddr, IMPL_PKG_ID, JCRE_CONTEXT);

		run();

		popFrame();

		//need to do flush
		memRefresh();

		io_SetCurrentProtocol(PROTOCOL_NONE);
	}
	ClearrfFirstSelectCmdFlag();
	
#endif
}


u8 MixMode_InterfaceIsConcurrency(u8 interfaceType)
{
	u8 num;
	u8 i;
	u8 interfaceFlag;
	num = 0;
	interfaceFlag = g_interfaceActivedFlag;
	interfaceFlag &= ~interfaceType;
	if(interfaceFlag != 0)
	{
		return 1;
	}
	return 0;
}


//??????????weipingjiao@20221229,???????void??
void MixMode_SetInterfaceActivedFlag(u8 interfaceType)
{
    if(interfaceType == NO_INTERFACE_ACTIVED)
    {
    	g_interfaceActivedFlag = NO_INTERFACE_ACTIVED;
    }
    else
    {
    	g_interfaceActivedFlag |= interfaceType;
    }
}
/* return  interfaceType is actived or not*/
u8 MixMode_GetInterfaceActivedFlag(u8 interfaceType)
{
    if(g_interfaceActivedFlag & interfaceType)
    {
    	return 1;
    }
    return 0;
}
/********************************************************************************************
*********************************************************************************************/

/*******************************protocol control function************************************
*********************************************************************************************/
static void MixMode_IsReceiveCommand(void)
{
//	boolean	result;
	#if BOOT__IO__SWP
	if(swp_isReceCommand())
	{
 		 g_mixModeStatus.SwpReceive		= true;
	}
	else
	#endif
	if(Spi_isReceCommand())
	{
		g_mixModeStatus.SpiReceive   = true;
	
	}
	else if(I2C_isReceCommand())
	{
		g_mixModeStatus.I2cReceive   = true;
	
	}
	
	
// 	if(!(g_mixModeStatus.T0Receive || g_mixModeStatus.T1Receive || g_mixModeStatus.SwpReceive))
// 	{
// 		result		= false;
// 	}
// 	else
// 	{
// 		result		= true;
// 	}
	
	return ;
}

u16 MixMode_GetUartRxLength(void)
{
	return g_uartRxLength;
}

void MixMode_Init(void)
{
	ecpSysHal_AssignProtocolPriority();
	ecpMem_Set(MEMtPX8(&g_mixModeStatus), 0x00, sizeof(MixModeStatus));
	g_swpOpeSequence				= 0;
	g_curCommand.pData				= IoBuffer[UART_INT_BUF_PCK_OFFSET].responseBuf.pdata;
	g_curCommand.size				= IoBuffer[UART_INT_BUF_PCK_OFFSET].responseBuf.size;
	//g_pUartDataField must be init here, warm reset may change the buffer from APDUbuffer to response.
	g_pUartDataField					= &IoBuffer[UART_INT_BUF_PCK_OFFSET].responseBuf.pdata[ISO7816UTIL_DATA_OFFSET];
}

boolean MixMode_IsSupport(void)
{
	return true;
}

void MixMode_EnterSleepMode(void)
{
	if(!(g_mixModeStatus.T0Receive || g_mixModeStatus.T1Receive || IoUart_Busy()||g_mixModeStatus.SwpReceive|| Swp_EnterSleep() ||g_mixModeStatus.T0PpsInprogress ||g_mixModeStatus.SpiReceive ||g_mixModeStatus.I2cReceive || Spi_EnterSleep() || I2C_EnterSleep()))
	{
		  ecpUart_EnterSleepMode();
	}
}

void MixMode_SetCommand(PX8 commandBuf, u16 size)
{
	g_curCommand.pData		= commandBuf;
	g_curCommand.size		= size;
	if(true == IoUart_isT0Protocol())
	{
		g_pUartDataField	= &commandBuf[ISO7816UTIL_DATA_OFFSET];
		iso7816Util_SetCommand(commandBuf, size, UART_BUF_PCK_TYPE);
	}
}

void MixMode_GetCommand(ppIso7816Util_Command_t * ppCommand)
{
	*ppCommand			= &g_curCommand;
}

void MixMode_CopyCommad(void)
{
	PX8						tempApduBuf;
	ppIso7816Util_Command_t	command;
	iso7816Util_GetCommand(&command);
	

	tempApduBuf			= g_curCommand.pData;

	ecpMem_Copy(&g_curCommand, command, sizeof(pIso7816Util_Command_t));
	ecpMem_Copy(tempApduBuf, command->pData, command->length);

	g_curCommand.pData	= tempApduBuf;
}

u8 MixMode_GetCurProtocol(void)
{
	u8	curProtocol;
	
	
	MixMode_IsReceiveCommand();
	
	//swp owns the highest priority
	if(true == g_mixModeStatus.SwpReceive)
	{
		curProtocol					= PROTOCOL_SWP;
		g_mixModeStatus.SwpReceive	= false;
		g_swpOpeSequence			= 0;
		iso7816Util_SetBufPckType(CL_BUF_PCK_TYPE);
	}
	else if(true == g_mixModeStatus.T0Receive)
	{
		curProtocol					= PROTOCOL_T0;
		g_mixModeStatus.T0Receive	= false;
		g_swpOpeSequence			= 0;
		iso7816Util_SetBufPckType(UART_BUF_PCK_TYPE);
		wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
		MixMode_UartReadyRx(ISO7816UTIL_HEADER_LENGTH);
		iso7816T0_RecieveCommand();
	}
	else if(true == g_mixModeStatus.SpiReceive)
	{
		curProtocol					= PROTOCOL_SPI;
		g_mixModeStatus.SpiReceive	= false;
		g_swpOpeSequence			= 0;
		iso7816Util_SetBufPckType(SPI_BUF_PCK_TYPE);
		//wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
		//MixMode_UartReadyRx(ISO7816UTIL_HEADER_LENGTH);
		//Spi_GetCommand();
	}
	else if(true == g_mixModeStatus.I2cReceive)
	{
		curProtocol					= PROTOCOL_I2C;
		g_mixModeStatus.I2cReceive	= false;
		g_swpOpeSequence			= 0;
		iso7816Util_SetBufPckType(I2C_BUF_PCK_TYPE);
		//wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
		//MixMode_UartReadyRx(ISO7816UTIL_HEADER_LENGTH);
		//Spi_GetCommand();
	}
	
	else if(true == g_mixModeStatus.T1Receive)
	{
		curProtocol					= PROTOCOL_T1;
		g_mixModeStatus.T1Receive	= false;
		g_swpOpeSequence			= 0;
		iso7816Util_SetBufPckType(UART_BUF_PCK_TYPE);
		wtxTimer_SetCurWtxTimerType(T1_WTX_TIMER);
	}
	else
	{
		if(SWP_MOST_SEQUENCE == g_swpOpeSequence)
		{
			g_swpOpeSequence = 0;
		}
		curProtocol		= PROTOCOL_NONE;
	}

	return curProtocol;
}
/********************************************************************************************
*********************************************************************************************/

/****************************Uart control function in interrupt******************************
*********************************************************************************************/
void MixMode_UartInit(void)
{
	IoUart_Init();
	
	if(0 == sysConfig_UartDefaultProtocl())
	{
		g_mixModeStatus.T0Receive	= false;
		iso7816T0_Init();
		wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
	}
	else
	{
		g_mixModeStatus.T1Receive	= false;
		iso7816T1_Init();
		wtxTimer_SetCurWtxTimerType(T1_WTX_TIMER);
	}

	wtxTimer_WtxTimerInit();

	g_uartCmdOffset						= 0;
	g_uartRxLength						= 0;
	g_uartRxSegLen						= 1;
	g_uartExpectRxLen					= 5;
	g_ppsLength							= IO_UART_PPS_MIN_LEN;
	g_mixModeStatus.T0PpsInprogress		= false;
	g_mixModeStatus.T0ReceiveCmd		= true;
	ecpUart_SetRxLength(g_uartRxSegLen);
}

void MixMode_UartReadyRx(u16 len)
{
	g_uartExpectRxLen		= len;
	if(len > MAX_RX_LENGTH_PER_INT)
	{
		g_uartRxSegLen	= MAX_RX_LENGTH_PER_INT;
	}
	else
	{
		g_uartRxSegLen	= len;
	}
	ecpUart_SetRxLength(g_uartRxSegLen);
}

void MixMode_UartReadyRxData(void)
{
	g_mixModeStatus.T0ReceiveCmd	= false;
}

void MixMode_UartRxMulBytes(PR8 buf, u16 len, u8 flag)
{
	if(RECEIVE_COMMAND == flag)
	{
		ecpMem_Copy(buf, &g_uartCmd[g_uartCmdOffset], len);
//		U8_INC(g_uartCmdOffset);
		g_uartCmdOffset += len;
	}
	else
	{
//		g_mixModeStatus.T0ReceiveCmd	= false;
		
		while(len != MixMode_GetUartRxLength())
		{
         ;
		}
		g_uartRxLength					= 0;
		g_mixModeStatus.T0ReceiveCmd	= true;
		g_uartCmdOffset					= 0;
		MixMode_UartReadyRx(ISO7816UTIL_HEADER_LENGTH);
	}
}
#endif
/********************************************************************************************
*********************************************************************************************/
