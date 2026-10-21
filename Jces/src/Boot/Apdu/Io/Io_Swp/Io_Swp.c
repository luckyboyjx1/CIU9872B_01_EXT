
#include <stdio.h>
#include <string.h>
#include "ecpDefine.h"
//#include "SWP_HCP.h"
#include "7816Util.h"
#include "BootNoAreaInitRam.h"
#include "SwpTimer.h"
#include "SysHal.h"
#include "chipHead.h"
#include "swpDataConfig.h"
#include "Swp_Private.h"
#include "io.h"

#if BOOT__IO__SWP

u8 Swp_Buffer[267];


volatile u8  xdata G_HCP_Card_Status;
u8  xdata  G_Swp_PowerMode; //for SWP(UL) test , 103 115 6.1.1.2, two swp active operation, first is full power mode , second is no power mode frame
u8 xdata   G_Swp_EndFlag;
u8  xdata  G_Swp_NeedUpdataPara;

/// @brief: SWP Events Manager
u8 g_SwpHciEvents[EVENTS_NUM];

u8 g_HCI_API_TrigEvent;
u16 g_SwpTxLength;

u8 Swp_EnterSleep(void)
{
	return SwpHal_EnterSleep();
}

#if HAL__IO__SWP__LIB

//------------------------------  reader mode ------------------------------ 
void HCI_StartReaderOperation(void)
{
     u8 temp[5];
//     temp[0] = 0x40 | EVT_READER_REQUESTED;                  // HCI_HEADER: EVENT + EVT_READER_REQUESTED       THE STEP1

     HCI_SendMessage(HCI_MODE_READER, HCI_RF_TYPE_TYPE_A, temp, 1);

     return;
}

u32 HCI_ReaderRecvResponseOperation(u8* RespMessage, u8* RecvMessage, u16 u16MessageLen)
{
    u32 responseLen = 0;

//    switch (RecvMessage[0] & 0x3f)    // The receive data is APDU
//    {
//        case ANY_OK:                  // RESPOSNE ANY_OK FOR WR_XCHG_DATA      THE STEP3  :RESPOSNE 都是用于响应Response的，不会用于响应 EVT，EVT_TARGET_DISCOVERED
//        {
//            //跟LastCmd有关
//            if ((RecvMessage[1] == 0x90) && (RecvMessage[2] == 0x00))    // SEND EVT_END_OPERATION
//            {
//                RespMessage[0]  = 0x40 | EVT_END_OPERATION;
//                responseLen = 0x01;
//            }
//        }
//            break;

//        case ANY_E_NOK:
//        case ANY_E_TIMEOUT:
//            break;

//        default:
//            break;
//    }
    return responseLen;
}

u32 HCI_ReaderRecvEventOperation(u8* RespMessage, u8* RecvMessage, u16 u16MessageLen)
{
    u32 responseLen = 0;

//    switch (RecvMessage[0] & 0x3f)
//    {
//        case EVT_TARGET_DISCOVERED:
//        {
//            // Now reader can send apdu, by command WR_XCHG_DATA  - THE STEP2
//            RespMessage[0]  = 0x00 | WR_XCHG_DATA;         // CMD_H
//            RespMessage[1]  = 0x00;         // CRT
//            RespMessage[2]  = 0x00;         // APDU_0
//            RespMessage[3]  = 0xA4;         // APDU_1
//            RespMessage[4]  = 0x00;         // APDU_2
//            RespMessage[5]  = 0x00;         // APDU_3
//            RespMessage[6]  = 0x00;         // APDU_4

//            responseLen = 0x07;
//        }
//            break;

////        case EVT_READER_STATUS:  //规范新加的，可以先不处理。
////            break;

//        default:
//            break;
//    }
    return responseLen;
}

#endif

#if !HAL__IO__SWP__LIB

#define NO_SEND_SW                                ((u16)0xFC5A)  //与java层对应

static volatile u8   g_SwpEnabled;
                u8   g_SWP_S1;

u8 getSwpSigner(void)
{
	return g_SWP_S1;
}
#endif

u8 isSwpInterfaceEnabled(void)
{
	  //return g_SwpEnabled;
}

void setSwpInterfaceEnabled(u8 ablity)
{
	  //g_SwpEnabled = ablity;
}

void Swp_Init(void)
{
#if HAL__IO__SWP__LIB
	
	SwpHal_Init();
	
	memset((u8*)&g_SwpHciEvents[0], 0x00, EVENTS_NUM);
	g_HCI_API_TrigEvent = 0;
	
#else
    if (!g_SWP_S1 )
    {
        Swp_RegisterHandleForS1Appear();
		  	Swp_Hcp_Init();
        Swp_Llc_Init();
    }
	
	setSwpInterfaceEnabled( !Swp_HardWare_PullUpManagement( g_SWP_S1 ) );

	if ( isSwpInterfaceEnabled() ) // Means there is a clf
    {
			
			Swp_EnableInterrupt();
			Swp_RegisterFindActivate();
    }
	if ( isSwpInterfaceEnabled() ) // Means there is a clf
    {
	    Swp_Llc_Act_ChangeMode();
	}
#endif
}

//-------------------------------------------------------------------------
//  Function Name : SWP_HCP_ProcessEvent
/// @brief  This function manages HCP event. It distinguishes 3 cases:\n
/// - EVT_POST_DATA : event that can be send either to the loopback gate or to a
/// host specific gate managed by the OS
/// - EVT_HOT_PLUG : not implemented yet
/// - other events : they are forwarded to host specific gates if the associated
/// callback is valid and if the gate ID is >= 0x10 ( HCI 4.3 )
/// In all other cases events are ignored.
///
/// @note \b global \b Function.
///
/// \retval 2 cases: 0 SWP_HCP_OK
//-------------------------------------------------------------------------
u32 SWP_HCP_ProcessEvent(void)
{
    switch ( g_Message[0] & HCP_MSG_INSTRUCTION_MASK )
    {
		//event for only card aplication or APDU card mode sent data
		case HCP_CARD_GATE_EVT_SEND_DATA:
			if (g_Mode == HCI_MODE_CARD_EMUALTION)
			{
//				if ( (g_HCP_Card_Status != SWP_HCP_CARD_DEACTIVATED) && (g_HCP_Card_Status != SWP_HCP_CARD_FIELD_OFF) )
				{
//					g_HCP_Event_Pipe = g_HCP_CurrentPipeId;
					
					g_SwpHciEvents[EVENT_ON_SEND_DATA] = HCI_API_EVENT_ON_SEND_DATA | HCI_API_LISTENER_TRIGGER;
					
					//当收到SEND DATA事件后，如果Filed Off事件还没处理，则不处理，否则会造成Filed Off处理过慢导致后续命令处理失败
					if ((u8)g_SwpHciEvents[EVENT_FIELD_OFF] == (HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER))
					{
						g_SwpHciEvents[EVENT_FIELD_OFF] = 0x00;
					}
				}
			}
			else if (g_Mode == HCI_MODE_CONNECTIVITY)
			{
				g_SwpHciEvents[EVENT_STAND_BY] = HCI_API_EVENT_STAND_BY | HCI_API_LISTENER_TRIGGER;
			}
			else if (g_Mode == HCI_MODE_READER)
			{
				g_SwpHciEvents[EVENT_TARGET_DISCOVERED] = HCI_API_EVENT_TARGET_DISCOVERED | HCI_API_LISTENER_TRIGGER;
			}
			break;
		
		case HCP_CARD_GATE_EVT_FIELD_ON:
			//g_HCP_Card_Status = SWP_HCP_CARD_FIELD_ON;
			break;
		case HCP_CARD_GATE_EVT_CARD_ACTIVATED:
			//g_HCP_Card_Status = SWP_HCP_CARD_ACTIVATED;
			break;
		
		case HCP_CARD_GATE_EVT_CARD_DEACTIVATED:
		case HCP_CARD_GATE_EVT_FIELD_OFF:
			if (g_SwpHciEvents[EVENT_FIELD_OFF] == 0)
			{
				g_SwpHciEvents[EVENT_FIELD_OFF] = HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER;
			}
			//g_HCP_Card_Status = SWP_HCP_CARD_FIELD_OFF;
			break;

        default:
            // other events ( host specific )
            // if callback not valid ignore the message
            break;
    }
    return SWP_HCP_OK;
}

u8 Swp_DetectEvent( void )
{
	if (g_SwpHciEvents[EVENT_FIELD_OFF] == (HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER))
	{
		return SWP_DETECT_FIELD_OFF;
	}
	
	if (g_SwpHciEvents[EVENT_ON_SEND_DATA] == (HCI_API_EVENT_ON_SEND_DATA | HCI_API_LISTENER_TRIGGER))
	{
		return RECEIVE_APDU_CMD;
	}

#if BOOT__IO__HCIAPI
	//需要对HCI API事件触发应用
	if (g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] == (HCI_API_EVENT_GET_PARAMETER_RESPONSE | HCI_API_LISTENER_TRIGGER))
	{
		return SWP_EVENT_GET_PARAMETER_RESPONSE;
	}
	else if (g_SwpHciEvents[EVENT_STAND_BY] == (HCI_API_EVENT_STAND_BY | HCI_API_LISTENER_TRIGGER))
	{
		return SWP_EVENT_STAND_BY;
	}
	else if (g_SwpHciEvents[EVENT_TARGET_DISCOVERED] == (HCI_API_EVENT_TARGET_DISCOVERED | HCI_API_LISTENER_TRIGGER))
	{
		return SWP_EVENT_TARGET_DISCOVERED;
	}
	else if (g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] == (HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE | HCI_API_LISTENER_TRIGGER))
	{
		return SWP_EVENT_EXCHANGE_DATA_RESPONSE;
	}
#endif
	
	return NO_EVENTS;
}


void Swp_SetEventProcess( u8 eventType )
{
	u8 type;
	switch ( eventType )
	{
		case RECEIVE_APDU_CMD:
			type = EVENT_ON_SEND_DATA;
			break;
		case SWP_EVENT_GET_PARAMETER_RESPONSE:
			type = EVENT_GET_PARAMETER_RESPONSE;
			break;
		case SWP_EVENT_STAND_BY:
			type = EVENT_STAND_BY;
			break;
		case SWP_DETECT_FIELD_OFF:
			type = EVENT_FIELD_OFF;
			break;
		case SWP_EVENT_TARGET_DISCOVERED:
			type = EVENT_TARGET_DISCOVERED;
			break;
		case SWP_EVENT_EXCHANGE_DATA_RESPONSE:
			type = EVENT_WRITE_EXCHANGE_DATA_RESPONSE;
			break;
		default:
			return;
	}
	
	if ( (g_SwpHciEvents[type] & HCI_API_LISTENER_TRIGGER) == HCI_API_LISTENER_TRIGGER )
	{
		g_SwpHciEvents[type] = g_SwpHciEvents[type] & ~HCI_API_LISTENER_TRIGGER;
		g_SwpHciEvents[type] |= HCI_API_LISTENER_PROCESSED;
	}
	
	if ( (eventType != NO_EVENTS) && (eventType != RECEIVE_APDU_CMD) )
	{
		g_HCI_API_TrigEvent = 1;
	}
}


u8 Swp_TrigEvent( void )
{
//	if (isSwpInterfaceEnabled() == 0)
//	{
//		return NO_EVENTS;
//	}

#if BOOT__IO__HCIAPI
	if (g_HCI_API_TrigEvent == 1)
	{
		g_HCI_API_TrigEvent = 0;
		return 1;
	}
#endif

	return NO_EVENTS;
}

// V1.1 modify by xwh@20211210，若更新CLF不成功，打上标记G_Swp_NeedUpdataPara，在SWP_Process中自动更新（解决更新UID后需要重启手机的问题）
u8 SWP_HCI_Update_TypeA_ParametersByIndex(u8 index)
{
	u8 tmpData[0x10];
	u8 len;
	u8 regIndex;
	
	//index tranmit to reg index
	switch(index) {
		case HCP_RFTYPEA_REG_UPDATE_FLAG_UID:
// 			len = *((unsigned char *)(SWP_CONFIG_UID));
// 			memcpy(&tmpData[0], ((unsigned char *)(SWP_CONFIG_UID+1)), len);		
			len = aReg_CardRFTypeA_UID[0];
			memcpy(&tmpData[0], ((unsigned char *)&aReg_CardRFTypeA_UID[1]), len);
			regIndex = 0x02;
			break;
		case HCP_RFTYPEA_REG_UPDATE_FLAG_SAK:
			len = 0x01;
			//tmpData[0] = *((unsigned char *)(SWP_CONFIG_SAK));
			tmpData[0] = aReg_CardRFTypeA_SAK[1];
			regIndex = 0x03;
			break;
		case HCP_RFTYPEA_REG_UPDATE_FLAG_ATQA:
			len = 0x02;
// 			tmpData[0] = *((unsigned char *)(SWP_CONFIG_ATQA));
// 			tmpData[1] = *((unsigned char *)(SWP_CONFIG_ATQA + 1));
			tmpData[0] = aReg_CardRFTypeA_ATQA[1];
			tmpData[1] = aReg_CardRFTypeA_ATQA[2];
			regIndex = 0x04;
			break;
		case HCP_RFTYPEA_REG_UPDATE_FLAG_ATS:
// 			len = *((unsigned char *)APPLICATION_DATA);
// 			memcpy(&tmpData[0], ((unsigned char *)(APPLICATION_DATA+1)), len);
			len = aReg_CardRFTypeA_History[0];
			memcpy(&tmpData[0], ((unsigned char *)(&aReg_CardRFTypeA_History[1])), len);
			regIndex = 0x05;
			break;
		case HCP_RFTYPEA_REG_UPDATE_FLAG_FWI:
			len = 0x01;
			//tmpData[0] = *((unsigned char *)SWP_FWI_SFGI);
			tmpData[0] = aReg_CardRFTypeA_TB1[1];
			regIndex = 0x06;
			break;
		case HCP_RFTYPEA_REG_UPDATE_FLAG_CID:
			len = 0x01;
			//tmpData[0] = *((unsigned char *)SWP_TYPEA_MODE_CID_SUPPORT);
			tmpData[0] = aReg_CardRFTypeA_CID_SUPPORT[1];
			regIndex = 0x07;
			break;
		case HCP_RFTYPEA_REG_UPDATE_FLAG_DRM:
			len = 0x03;
// 			tmpData[0] = *((unsigned char *)DATARATE_MAX_TA);
// 			tmpData[1] = *((unsigned char *)(DATARATE_MAX_TA+1));
// 			tmpData[2] = *((unsigned char *)(DATARATE_MAX_TA+2));
			tmpData[0] = aReg_CardRFTypeA_MAXDataRate[1];
			tmpData[1] = aReg_CardRFTypeA_MAXDataRate[2];
			tmpData[2] = aReg_CardRFTypeA_MAXDataRate[3];
			regIndex = 0x09;
			break;
		default:
			len = 0;
			break;
	}
	
	if (len != 0)
	{
		return 0;
	}
	
	SWP_UICC_Main();      // SWP process function
	
//	if(UICC_SHDLC_ConectIsOrNot() == 1)
//	{
//		CLKSYSSEL = 0x00; //THD89 clkInit//内部时钟 1分频
//	}
    if(gSwpCardResetRequest)
	{// equal 1 when received RF field off event
    		//reset();  //do something to clear RF chanel security state
		
		if (g_Mode == HCI_MODE_CARD_EMUALTION)
		{
			ClearrfFirstSelectCmdFlag();
			if (PX_R_U16(&channelInfo.selected_context[U8_M_U8((NFC_BASIC_CHANNEL_NUM),2)]) != NULL_CONTEXT)
			{
				if (g_SwpHciEvents[EVENT_FIELD_OFF] == 0)
				{
					g_SwpHciEvents[EVENT_FIELD_OFF] = HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER;
				}
			}
		}

        gSwpCardResetRequest = 0;
    }

    if (g_SWPFlag == 1)
    {
        g_SWPFlag = 0;
		
		CLKSYSSEL = 0x00;

		// first check the type of instruction
		if ( ( g_Message[0] & HCP_MSG_TYPE_MASK ) == HCP_MSG_COMMAND_TYPE )
		{
			// command message
		}
		else if(( g_Message[0] & HCP_MSG_TYPE_MASK ) == HCP_MSG_EVENT_TYPE )
		{
			// event message
			SWP_HCP_ProcessEvent( );
		}
		else
		{
			// Response message
			// for HCI API
			if (g_Mode == HCI_MODE_READER)
			{
				if (g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] == HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE)
				{
					g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] = HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE | HCI_API_LISTENER_TRIGGER;
				}
			}
			else
			{
				if (g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] == HCI_API_EVENT_GET_PARAMETER_RESPONSE)
				{
					g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] |= HCI_API_LISTENER_TRIGGER;
				}
			}
		}
    }
	
	if (Swp_DetectEvent() != NO_EVENTS)
	{
		return 1;
	}

	return 0;
}

void swp_ReceiveCommand(void)
{

//    u16                             Size ;
//	Size = SWP_HCP_GetReadMessageDataSize();
//	

//	if (Size == 0)
//	{
//		return;
//	}

//	iso7816Util_ParseBytes((u8*)(SWP_HCP_GetReadMessageDataAddr()),Size,PH_ISO7816L4_FLAG_BYPASSPARSER);
	return;
}

u16 swp_ReceiveData(void)
{
// 	u16						D_A		dataLength;
// 	PX8						D_A		commandBuffer;
// 	ppIso7816Util_Command_t	D_A		comand;

// 	iso7816Util_GetCommand( &comand );
// 	commandBuffer 		= comand->pData;
// 	dataLength			= commandBuffer[ISO7816UTIL_P3_OFFSET];
	//if swp shdlc has been reset
//	if ( !SWP_HCP_SessionIsFinish() )
//	{
//		return 0;
//	}
//	iso7816Util_ParseBytes((u8*)(SWP_HCP_GetReadMessageDataAddr()+5),*((u8*)(SWP_HCP_GetReadMessageDataAddr()+4)),PH_ISO7816L4_FLAG_BYPASSPARSER);
	//ecpMem_Copy(commandBuffer, (u8*)(SWP_HCP_GetReadMessageDataAddr()+5), dataLength);
	return 0;
}

u8  swp_HandleReceive(void)
{
//	u8     NbReceptions;
//	u8     needHandled;
//	u32    ret_val;
//	needHandled = 0;
//	NbReceptions = 0;
//	do
//	{
//		ret_val = Swp_Mac_ReceiveFrame(MEMtPX8(&g_SWP_LLC_Context.ReadLpdu), &g_SWP_LLC_Context.ReadLpdu->Size, 0);
//		if(ret_val != SWP_MAC_OK)
//		{
//			return 0x01;
//		}
//		if ( ( g_SWP_LLC_Context.ReadLpdu->Data[0] & SWP_LLC_CTRL_MASK ) == SWP_LLC_CLT_CODE )
//		{
//			  Swp_Llc_Clt_Receive();
//		}
//		else if ( ( g_SWP_LLC_Context.ReadLpdu->Data[0] & SWP_LLC_CTRL_MASK ) == SWP_LLC_ACT_CODE )
//		{
//			  Swp_Llc_Act_Receive();
//		}
//		else if ( ( g_SWP_LLC_Context.ReadLpdu->Data[0] & SWP_LLC_SHDLC_CODE ) == SWP_LLC_SHDLC_CODE )
//		{
//			  needHandled = Swp_Llc_Shdlc_Receive();
//		}

//		NbReceptions++;
//	}while ( ( Get_RxState( g_SWP_MAC_Context.CurRead ) == 0x02 ) &&   // continue processing if other receptions have already happened
//			 ( NbReceptions < g_SWP_LLC_Context.WS ) );   
//	if(needHandled != 0x0)
//	{
//	    return 0x2;
//	}
  return 0x0;
	
}


u16 swp_SendData(void)
{
//	u16 dataLength;
//	
//	dataLength = iso7816Util_GetResponseLength(PH_ISO7816L4_FLAG_BYPASSPARSER);
//	//if swp shdlc has been reset
//	if ( !SWP_HCP_SessionIsFinish() )
//	{
//		return dataLength;
//	}

//	SWP_HCP_SetEventHeader(HCP_CE_EVT_SEND_DATA);
//	iso7816Util_SetResponseData(&g_HCP_Context.WriteMessage.Buf[1],dataLength,PH_ISO7816L4_FLAG_BYPASSPARSER);
//	//ecpMem_Copy(&g_HCP_Context.WriteMessage.Buf[1], sendBuffer, dataLength);
//	g_HCP_Context.WriteMessage.Size += dataLength;
//	Swp_Hcp_SendMessage();
//	return dataLength;
}


void Swp_SendStatus(void)
{
	ppIso7816Util_Response_t 	D_A		response;
	u8 i;

	iso7816Util_GetResponse(&response);
	
	//	//不需要发送状态字
	if (response->SW1SW2 == IO_NO_STATUS)
	{
		for (i=EVENT_ON_SEND_DATA; i<EVENTS_NUM; i++)
		{
			if ( (g_SwpHciEvents[i] & HCI_API_LISTENER_PROCESSED) == HCI_API_LISTENER_PROCESSED )
			{
				g_SwpHciEvents[i] = 0;
			}
		}
		
		if (g_SwpTxLength != 0x00)
		{
			//如果已经收到FieldOff事件，则不需要发送状态字
			if (g_SwpHciEvents[EVENT_FIELD_OFF] == (HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER))
			{
				g_SwpTxLength = 0x00;
			}
			else
			{
				HCI_SendMessage(g_Mode, g_RFType, Swp_Buffer, g_SwpTxLength);
			}
		}
		return;
	}
	
	g_SwpHciEvents[EVENT_ON_SEND_DATA] = 0x00;

	//如果已经收到FieldOff事件，则不需要发送状态字
	if (g_SwpHciEvents[EVENT_FIELD_OFF] == (HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER))
	{
		g_SwpTxLength = 0x00;
		return;
	}
	
	if (response->length == 0)
	{
		return;
	}
	
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | HCP_CARD_GATE_EVT_SEND_DATA;
	g_SwpTxLength = 1;
	
	memcpy(&Swp_Buffer[1], response->pData, response->length);
	g_SwpTxLength += response->length;
	
	HCI_SendMessage(g_Mode, g_RFType, Swp_Buffer, g_SwpTxLength);
	
	g_SwpTxLength = 0;

}

//boolean  swp_GetEventNeedHandle(void)
//{
//	boolean result;
//	result = false;

//	if (!isSwpInterfaceEnabled())
//	{
//		return result;
//	}
//	
//	if ( SWP_HCP_isNeedProcessEvent() )
//	{
//		result = true;
//	}
//	if ( SWP_LLC_SHDLCLinkEstablished() && (!SWP_HCP_SessionIsFinish()) )
//	{
//		result = true;
//	}
//	else if (g_SwpHciEvents[EVENT_FIELD_OFF] == (HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER))
//	{
//		result = true;
//	}
//	else if(g_SwpHciEvents[EVENT_ON_SEND_DATA] == (HCI_API_EVENT_ON_SEND_DATA | HCI_API_LISTENER_TRIGGER))
//	{
//		result = true;
//	}
//	else if(g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] == (HCI_API_EVENT_GET_PARAMETER_RESPONSE | HCI_API_LISTENER_TRIGGER))
//	{
//		result = true;
//	}
//	else if(g_SwpHciEvents[EVENT_STAND_BY] == (HCI_API_EVENT_STAND_BY | HCI_API_LISTENER_TRIGGER))
//	{
//		result = true;
//	}
//	else if (g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] == (HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE | HCI_API_LISTENER_TRIGGER))
//	{
//		result = true;
//	}
//	else if(g_SwpHciEvents[EVENT_TARGET_DISCOVERED] == (HCI_API_EVENT_TARGET_DISCOVERED | HCI_API_LISTENER_TRIGGER))
//	{
//		result = true;
//	}
//	return result;
//}


u8 Swp_HandleCommand(void)
{
#if HAL__IO__SWP__LIB
	u8 eventListener;
	
	eventListener = Swp_DetectEvent();
	if (eventListener != NO_EVENTS)
	{
		Swp_SetEventProcess(eventListener);
		
		if (eventListener == SWP_DETECT_FIELD_OFF)
		{
	//		IF_CardDetectFieldOff();

	//		io_warmResetFlag = 1;
			//Swp_SetEventProcess(SWP_DETECT_FIELD_OFF);
			//return 1;
		}
		if (eventListener == RECEIVE_APDU_CMD)
		{
			ppIso7816Util_Command_t		D_A		command;
			
			if ((PN_R_U8(JcvmConfig_GetVmConfigAddr(RF_COMMUNICATION_SWITCH))) == 1)
			{
				return 0;
			}

			iso7816Util_GetCommand( &command );
			
			memcpy(command->pData, &g_Message[1], g_MessageLen - 2);
			command->length = g_MessageLen - 2;
			command->Le = 0;
			
			ecpMem_Set(&command->pData[command->length], 0, U16_S_U16(266,command->length));
		}
		
		return 1;
	}
	
	return 0;

#else
	PX8						D_A		commandBuffer;
	ppIso7816Util_Command_t	D_A		comand;
	iso7816Util_GetCommand( &comand );
	commandBuffer 		= comand->pData;
	if (!isSwpInterfaceEnabled())
	{
		return 0;
	}

	if (g_HCP_NeedProcess)
	{
		g_HCP_NeedProcess--;
	}
	if ( SWP_LLC_SHDLCLinkEstablished() && (!SWP_HCP_SessionIsFinish()) )
	{

		ecpSysHal_ManageNvmRepair();
		Swp_SessionInit();
		return 0;
	}
		//如果是非接功能失效，则不做任何处理
	if (PN_R_U8(JcvmConfig_GetVmConfigAddr(RF_COMMUNICATION_SWITCH))== 1)
	{
#if GP__VERSION__FOR_CHINA_UNICOM_MASK
		if ((GALITT_TEST) == 0x00)//该情况下AmendmentC不需要返回信息 zm&&wuronghua
		{
			//联通特殊需求，
			g_HCP_Context.WriteMessage.Size = 0;
				
			SWP_HCP_SetEventHeader(HCP_CE_EVT_SEND_DATA);
			g_HCP_Context.WriteMessage.Buf[g_HCP_Context.WriteMessage.Size++] = 0x6D;
			g_HCP_Context.WriteMessage.Buf[g_HCP_Context.WriteMessage.Size++] = 0x00;
			
			Swp_Hcp_SendMessage( );
		}
#endif
		return 0;
	}

	if (g_SwpHciEvents[EVENT_FIELD_OFF] == (HCI_API_EVENT_FIELD_OFF | HCI_API_LISTENER_TRIGGER))
	{
#if ICCTEST_VER_1_13
		if (g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] == (HCI_API_EVENT_GET_PARAMETER_RESPONSE | HCI_API_LISTENER_TRIGGER))
		{
			swp_SetEvent(SWP_EVENT_GET_PARAMETER_RESPONSE);
			return 1;
		}
#endif
		swp_SetEvent(SWP_DETECT_FIELD_OFF);
		
		if (g_HCP_NeedProcess <= 0x1)
		{
			g_SwpHciEvents[EVENT_ON_SEND_DATA] = 0x00;
		}
		return 1;
	}
	if (g_SwpHciEvents[EVENT_ON_SEND_DATA] == (HCI_API_EVENT_ON_SEND_DATA | HCI_API_LISTENER_TRIGGER))
	{
		swp_SetEvent(RECEIVE_APDU_CMD);
		ecpMem_Copy(commandBuffer, MEMtPX8(SWP_HCP_GetReadMessageDataAddr()), SWP_HCP_GetEvtSendDataSize());
		comand -> length = SWP_HCP_GetEvtSendDataSize();
	  	return 1;
	}
	
	if (g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] == (HCI_API_EVENT_GET_PARAMETER_RESPONSE | HCI_API_LISTENER_TRIGGER))
	{
		swp_SetEvent(SWP_EVENT_GET_PARAMETER_RESPONSE);
		return 1;
	}
	else if (g_SwpHciEvents[EVENT_STAND_BY] == (HCI_API_EVENT_STAND_BY | HCI_API_LISTENER_TRIGGER))
	{
		swp_SetEvent(SWP_EVENT_STAND_BY);
		return 1;
	}
	else if (g_SwpHciEvents[EVENT_TARGET_DISCOVERED] == (HCI_API_EVENT_TARGET_DISCOVERED | HCI_API_LISTENER_TRIGGER))
	{
		swp_SetEvent(SWP_EVENT_TARGET_DISCOVERED);
		return 1;
	}
	else if (g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] == (HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE | HCI_API_LISTENER_TRIGGER))
	{
		swp_SetEvent(SWP_EVENT_WRITE_EXCHANGE_DATA_RESPONSE);
		return 1;
	}

	if (g_HCP_Context.Status == EVT_LOOKBACK)// loopbackgate is supposed to send back exactly the same data
	{
		if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_OPENED)
		{
			SWP_HCP_SetEventHeader( HCP_EVT_POST_DATA );
			ecpMem_Copy(SWP_HCP_GetWriteMessageAddr() , SWP_HCP_GetReadMessageDataAddr() , SWP_HCP_GetReadMessageDataSize() );
			g_HCP_Context.WriteMessage.Size = g_HCP_Context.ReadMessage.Size;
		#if ICCTEST_VER_1_13
			if( (g_HCP_Context.CurrentPipeId == 0x02) && (g_SWP_LLC_Context.PowerMode == SWP_LLC_LOW_POWER_MODE) )
			{
				g_HCP_Context.WriteMessage.Size = 0;	
			}
		#endif
		}
		else if (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].State == HCP_PIPE_CLOSED)
		{
			g_HCP_Context.WriteMessage.Size = 0;
		}
		SWP_HCP_ResetHCPStatus();
		Swp_Hcp_SendMessage();//send back to CLF
	}
	if (g_HCP_Context.Status == PROCESS_COMMAND)
	{
		SWP_HCP_ResetHCPStatus();
		SWP_HCP_ProcessCommand();
	}
	return 0;

#endif 
}

//判断当前使用的是TYPE A 还是TYPE B;
u8 get_swp_protocol_type(void)
{
	if (g_RFType == HCI_RF_TYPE_TYPE_B)
	{
		return (PROTOCOL_T1 | PROTOCOL_MEDIA_CONTACTLESS_TYPE_B);
	}
	else
	{
		return (PROTOCOL_T1 | PROTOCOL_MEDIA_CONTACTLESS_TYPE_A);
	}
}


void swp_ResetSwpConfigData(void)
{
	SwpHal_ClearSessionID();
}
 
void swp_PullDownManagementS2(void)
{
    Swp_HardWare_PullDownManagementS2();
}
 
 void setContactlessUid(u8 *uidData, u8 uidLen)
{
//	u8 i, temp[11];
//	
//	temp[0] = uidLen;
//	for (i=0; i<uidLen; i++)
//	{
//		temp[i + 1] = uidData[i];
//	}
//	
//	nvmCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_UID)),MEMtPN8(temp),uidLen+1); 
}

#endif   // end of BOOT__IO__SWP


