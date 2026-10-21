#include "Io.h"
#include "Io_14443TaL4.h"
#include "Io_7816T0.h"
#include "Io_Swp.h"
#include "Io_SPI.h"
#include "Io_I2C.h"
#include "SysConfig.h"
#include "WtxTimer.h"
#include "WtxInterupt.h"
#include "BootNoAreaInitRam.h"
#include "Io_Uart.h"
#include "MixMode.h"
#include "IoBuffer.h"
#include "7816Channel.h"
#include "Swp_Hci_Api.h"
#include "SysHal.h"
#include "Io_interface.h"
#include "euiccConfig.h"
#include "chn_manager.h"

#if BOOT__IO__PROTOCOL

static  u8	X_A 	io_protocolType;
 //       u8	X_A   io_isWarmReset;     // = 0: is cold reset , = 1: is warm reset, = 2: deselect current applet
 //       u8	X_A   io_isNeedSendAtr;
static void io_ChoseTimer(void)
{
	switch(io_protocolType)
	{
		case PROTOCOL_T0:
			wtxTimer_SetCurWtxTimerType(T0_WTX_TIMER);
			break;
		case PROTOCOL_T1:
			wtxTimer_SetCurWtxTimerType(T1_WTX_TIMER);
			break;
		case PROTOCOL_TA:
			wtxTimer_SetCurWtxTimerType(TA_WTX_TIMER);
			break;
		default :
			wtxTimer_SetCurWtxTimerType(NO_WTX_TIMER);
			break;
	}
}

void io_OpenTimer(void)
{
	io_ChoseTimer();
	wtxTimer_WtxTimerInit();
}

static void io_EnterSleepMode(void)
{
	if(sysConfig_IsUartEnterSleepMode())
	{
		if(true == MixMode_IsSupport())
		{
			MixMode_EnterSleepMode();
		}
		else
		{
			if(PROTOCOL_TA != io_protocolType)
			{
				ecpUart_EnterSleepMode();
			}
		}
	}
}

void io_Init(u8 isDeselect)    SMALL_MODE
{
	if(true == MixMode_IsSupport())
	{
		IoUart_InitVar();
		ecpUart_Init();
		iso7816Util_InitBufPck();

		
    #if BOOT__IO__I2C
		if (chipConfig_IsI2CIoActive())
		{
			I2C_Init( );
		}
	#endif
		
	#if BOOT__IO__SPI
		if (chipConfig_IsSpiIoActive())
		{
			Spi_Init( );
		}
	#endif

	#if BOOT__IO__SWP
		if (chipConfig_IsSwpIoActive())
		{
			Swp_Init( );
		}
	#endif
		
	#if BOOT__IO__MIFARE
		ecpMifareHal_Init();
	#endif
		MixMode_Init();
		io_protocolType		= PROTOCOL_NONE;
// 		if(ecpUart_IsReset())
// 		{
// 			MixMode_UartInit();
// 			io_protocolType	= PROTOCOL_T0;
// 		}
		io_OpenTimer();
		//for avoid enter reset interrupt too early, make sure interrupt trigger after all necessary init. 
		//uart interrupt must open here.
	
	}
	else
	{
		switch(sysConfig_GetCpuPower())
		{
			case CONTACT_BASE_POWER:
				IoUart_Init();
				iso7816Util_InitUartBufPck();
				if(0 == sysConfig_UartDefaultProtocl())
				{
					io_protocolType	= PROTOCOL_T0;
					iso7816T0_Init();

				}
				else
				{
					io_protocolType = PROTOCOL_T1;
					iso7816T1_Init();

				}
				break;
			case CONTACT_LESS_POWER:
				io_protocolType	= PROTOCOL_TA;
				iso7816Util_InitClBufPck();
				iso14443Ta_Init(isDeselect);
				break;
			default :
				break;
		}
		io_OpenTimer();
	}
}

static void io_SendStatus(u16 status)
{
	if(IO_NO_STATUS != status)
	{
		switch(io_protocolType)
		{
			case PROTOCOL_T0:
					iso7816T0_SendStatus();
					break;
			case PROTOCOL_T1:
					iso7816T1_SendStatus();
					break;
			case PROTOCOL_TA:
					iso14443TaL4_SendStatus();
					break;
		#if BOOT__IO__SPI
			case PROTOCOL_SPI:
					Spi_SendStatus();
					break;
		#endif
		
		#if BOOT__IO__I2C
			case PROTOCOL_I2C:
					I2C_SendStatus();
					break;
		#endif
			default:
					break;
		}
	}
#if BOOT__IO__SWP
	if(PROTOCOL_SWP == io_protocolType)
	{
		Swp_SendStatus();
	}
#endif
	return;
}

void io_FetchCommand(PX8 buf)
{
	ppIso7816Util_Command_t			D_A    comand;
    iso7816Util_GetCommand( &comand );

	switch(io_protocolType)
	{
		case PROTOCOL_T0:
			iso7816T0_FetchCommand(buf);
			break;
		case PROTOCOL_T1:
		case PROTOCOL_TA:
			ecpMem_Copy(buf, comand->pData, comand->length);
		  break;


		default:
			break;
	}
}

static void io_ReceiveCommand(void)
{
	io_EnterSleepMode();
	
	if(true == MixMode_IsSupport())
	{
 		io_protocolType		= MixMode_GetCurProtocol();
	}
	else
	{
		switch(io_protocolType)
		{
			case PROTOCOL_T0:
				iso7816T0_RecieveCommand();
				break;
			case PROTOCOL_T1:
				iso7816T1_RecieveCommand();
				break;
			case PROTOCOL_TA:
				iso14443TaL4_RecieveCommand();
				break;
			case PROTOCOL_SWP:
				//swp_ReceiveCommand();
			default:
				break;
		}
	}

    return;
}

static boolean io_HandleCommand(void)
{
	boolean	isReturn;

	isReturn	= false;
	
	switch(io_protocolType)
	{
		case PROTOCOL_T0:
		case PROTOCOL_T1:
			if(true == IoUart_isT0Protocol())
			{
				isReturn	= iso7816T0_HandleCommand();
			}
			else
			{
				//do nothing;
			}
			break;
		case PROTOCOL_TA:
			isReturn		= true;
			break;
	#if BOOT__IO__SPI
		case PROTOCOL_SPI:
			isReturn	= Spi_HandleCommand();
			break;
	#endif
		
	#if BOOT__IO__I2C
		case PROTOCOL_I2C:
			isReturn	= I2C_HandleCommand();
			break;
	#endif

	#if BOOT__IO__SWP
		case PROTOCOL_SWP:
			isReturn	= Swp_HandleCommand();
			break;
	#endif
		default:
			isReturn		= false;
			break;
	}

	return isReturn;
}

u16 io_Receive(void)
{
	u16 dataLength;
	
	dataLength = 0x00;
	
	wtxTimer_WtxTimerStop();
	
	switch(io_protocolType)
	{
		case PROTOCOL_T0:
			dataLength = iso7816T0_Receive();
			break;
		default:
			break;
	}
	
	wtxTimer_WtxTimerStart();

	return dataLength;
}

void io_SendStatusAndReceiveCommand(u16 status)
{
	u8							D_A		isReturn;
//	ppIso7816Util_Command_t  	D_A		sr_PCommand;
	ppIso7816Util_Response_t	D_A		ppResponse;

	if (status == 0)
	{
//		iso7816Util_GetCommand( &sr_PCommand );
//		io_FetchCommand(sr_PCommand->pData);
	}
	else
	{
		wtxTimer_WtxTimerStop();

		do
		{

			iso7816Util_GetResponse( &ppResponse );

			io_SendStatus(ppResponse->SW1SW2);
			if(true == MixMode_IsSupport()) 
			{
				if(ecpUart_IsReset())
				{
					MixMode_UartActive();
					ecpUart_ClearResetFlag();
					MixMode_WarnReset(PROTOCOL_T0);
				}
		  }
			io_ReceiveCommand();
			isReturn = io_HandleCommand();
		}while(false == isReturn);

		if(false == MixMode_IsSupport())
		{
			wtxTimer_WtxTimerStart();
		}
	}
	
	iso7816Channel_SetLogicalChannel();
	MixMode_CopyCommad();
}
#if (JAVA_CARD_UICC||BOOTLOADER_PRO)
void io_SendStatusAndReceiveCommandSingle(u16 status)
{
	u8							D_A		isReturn;
//	ppIso7816Util_Command_t  	D_A		sr_PCommand;

	if (status == 0)
	{
//		iso7816Util_GetCommand( &sr_PCommand );
//		io_FetchCommand(sr_PCommand->pData);
	}
	else
	{
		wtxTimer_WtxTimerStop();
		isReturn = false;
		do
		{
			io_SendStatus(status);
			if(true == MixMode_IsSupport()) 
			{
						if(ecpUart_IsReset())
						{
							MixMode_UartActive();
//							io_isNeedSendAtr = 0;
						}
		  }
			io_ReceiveCommand();
			
		  //if(io_GetCurrentProtocol() != PROTOCOL_NONE)
		  if(io_GetCurrentProtocol() == PROTOCOL_T0)
		  {
			isReturn = TRUE;

			iso7816T0_Clrb6C61();//清除g_pT0Control->b6C61为RESP_NO616C，针对重入测试/SupportSIMAPI/SIMReEntry_05_14_01修改

			bakLogicalChninfo();//备份逻辑通道信息，针对重入测试/SupportSIMAPI/SIMReEntry_05_14_01修改

		  }
		}while(false == isReturn);

		if(false == MixMode_IsSupport())
		{
			wtxTimer_WtxTimerStart();
		}
	}
	if(PROTOCOL_T0 == io_GetCurrentProtocol())
	{
		IFBakChannelInfo();
	}

	iso7816Channel_SetLogicalChannel();
	MixMode_CopyCommad();
}
#endif
u8 io_GetLogicalChannel(void)
{
	return iso7816Channel_GetCurLogicalChn();
}

void io_GetIoBuffer( ppIso7816Util_Buffer_t *ppIoBuffer )
{
	iso7816Util_GetIoBuffer(ppIoBuffer);
}

void io_GetCommand( ppIso7816Util_Command_t * ppCommand )
{
	if(true == MixMode_IsSupport())
	{
		MixMode_GetCommand(ppCommand);
	}
	else
	{
		iso7816Util_GetCommand(ppCommand);
	}
}

void io_GetResponse( ppIso7816Util_Response_t *ppResponse )
{
	iso7816Util_GetResponse(ppResponse);
}

void io_SetCommand(PX8 commandBuf, u16 size)
{
	if(true == MixMode_IsSupport())
	{
		MixMode_SetCommand(commandBuf, size);
	}
	else
	{
		iso7816Util_SetCommand(commandBuf, size, NO_BUF_PCK_TYPE);
	}
}

u8 io_GetCurrentProtocol(void)
{
	return io_protocolType;
}

void io_SetCurrentProtocol(u8 type)
{
	  io_protocolType = type;
	  return;
}
void io_SetResPrameter( u16 resLength, u16 Lr, u16 SW1SW2)
{
	iso7816Util_SetResPrameter( resLength, Lr, SW1SW2, PH_ISO7816L4_FLAG_BYPASSPARSER );
}

IRQ void io_WtxRequest(void)
{
	wtxTimer_WtxTimerSuspend();

	switch(io_protocolType)
	{
		case PROTOCOL_T0:
		case PROTOCOL_T1:
			WtxInterupt_UartRequestWtx(io_protocolType);
			break;
		case PROTOCOL_TA:
			WtxInterupt_RfRequestWtx();
			break;
		default:
			break;
	}

	wtxTimer_WtxTimerResume();
}

void io_SuspendAllInterrupts(void)
{
	ecpSysHal_SuspendAllInterrupts();
}

void io_ResumeAllInterrupts(void)
{
	ecpSysHal_ResumeAllInterrupts();
}

#if BOOT__IO__SWP
u8 io_GetSwpProtocolType(void)
{
	return  get_swp_protocol_type();
}


#endif
#endif	//BOOT__IO__PROTOCOL
