/***************************************************************************************
 * Copyright 2012 Eastcompeace. Co., Ltd.                                              *
 * All rights reserved.                                                                *
 *                                                                                     *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,            *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,           *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  IN NO       *
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,                 *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;         *
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY             *
 * WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR            *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF              *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                          *
 *                                                                                     *
 ***************************************************************************************/

/** @cond INCLUDE_FILE_DESCRIPTION */
/**
 * @brief
 * Basic SWP HCI API layer implementation.
 *
 * @file
 */
/** @endcond */
/**
 * @addtogroup IFX_SWP_GROUP
 * @{
 */

/*  includes
****************************************************************/
#include "typejava.h"
#include "profile.h"
#include "Io_Swp.h"
#include "stack.h"
#include "object.h"
#include "ob_manager.h"
#include "packageloc.h"
#include "method.h"
#include "utilMethod.h"
#include "SwpHal.h"
//#include "Swp_Hcp_Layer.h"
#include "Swp_Private.h"

/*  pragmas
****************************************************************/

#if BOOT__IO__HCIAPI
#if BOOT__IO__SWP

/*  defines
****************************************************************/



/// @{ for HCI API
/*  private prototypes
****************************************************************/

/*  constants
****************************************************************/

/*  variables
****************************************************************/

#pragma arm section rwdata = "JC_SWP_RWDATA", zidata = "JC_SWP_ZIDATA"

//record Contactless state request tag
u8 Swp_UiccContactlessState;

#pragma arm section rwdata

/*  functions
****************************************************************/




void SWP_HCP_AnyGetParameterHCI (u8 PipeId , u8 Index)
{
//	u8* writeMsgBuf = (u8*)SWP_HCP_GetWriteMessageAddr();

//    SWP_HCP_SetCommandHeader( HCP_CMD_ANY_GET_PARAMETER );
//    writeMsgBuf[0] = Index;
//    SWP_HCP_SetWriteMessageSize(1);

//    // send command
//    SWP_HCP_InitMessageTransmission( );
//    Swp_Hcp_SendMessage();

	u8 txLen;
	
	Swp_Buffer[0] = HCP_MSG_COMMAND_TYPE | HCP_CMD_ANY_GET_PARAMETER;
	Swp_Buffer[1] = Index;
	
	txLen = 2;
	
	HCI_SendMessage(g_Mode, g_RFType, Swp_Buffer, txLen);
}


/// @brief HCIMessage native functions

void isHeaderHCI(void)
{
	//pushShort((jbyte)SWP_HCP_MsgIsHeader());
	pushShort((jbyte)1);
}

void isCompleteHCI(void)
{
	//pushShort((jbyte)SWP_HCP_MsgIsComplete());
	pushShort((jbyte)1);
}

//获取接收到的HCI message指令类型
void getInstructionTypeHCI(void)
{
	jbyte instructionType;
	//instructionType = SWP_HCP_GetMessageType();
	instructionType = (u8)(g_Message[0] & HCP_MSG_TYPE_MASK);
	
	pushShort(instructionType);
}

//获取接收到的HCI message对应的指令
void getInstructionHCI(void)
{
	jbyte instruction;
	//instruction = SWP_HCP_GetMessageInstruction();
	
	instruction =  ( u8 )( g_Message[0] & HCP_MSG_INSTRUCTION_MASK );

	pushShort(instruction);
}

//
void getReceiveBufferHCI(void)
{
	PN8 addr;
	addr = getJavaObjectDataArray();
	
	memCopyNoAtomic((u8*)addr, (u8*)&g_Message[1], ( u16 )( (g_MessageLen == 0) ? 0x00: (g_MessageLen-1) ));
}

void getDataLength(void)
{
	pushShort((jbyte)( (g_MessageLen == 0) ? 0x00: (g_MessageLen-1) ));
}


/// @HCIDevice
///
void HCI_getPowerMode(void)
{
//	if (isISO7816Actived())
//	{
//		pushShort((jbyte)SWP_LLC_FULL_POWER_MODE);
//		return;
//	}
//	pushShort((jbyte)g_SWP_LLC_Context.PowerMode);
	u8 powerMode;
	
	powerMode = HCI_GetPowerMode();
	
	if (powerMode == HCI_POWER_MODE_NO_POWER)
	{
		pushShort((jbyte)0xFF);
	}
	else if (powerMode == HCI_POWER_MODE_LOW)
	{
		pushShort((jbyte)0x00);
	}
	else
	{
		pushShort((jbyte)0x01);
	}
}


//获取RF 类型；
void getRFTypeHCI(void)
{
	u8 GateId;
	
	//GateId = g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId;
	
	switch(g_Mode)
	{
		case HCI_MODE_CARD_EMUALTION:
			if (g_RFType == HCI_RF_TYPE_TYPE_A)
			{
				GateId = HCP_GATE_ID_CARD_TYPEA;
			}
			else if (g_RFType == HCI_RF_TYPE_TYPE_B)
			{
				GateId = HCP_GATE_ID_CARD_TYPEB;
			}
			break;
		
		case HCI_MODE_READER:
			if (g_RFType == HCI_RF_TYPE_TYPE_A)
			{
				GateId = HCP_GATE_ID_READER_TYPEA;
			}
			else if (g_RFType == HCI_RF_TYPE_TYPE_B)
			{
				GateId = HCP_GATE_ID_READER_TYPEB;
			}
			break;

		case HCI_MODE_CONNECTIVITY:
			GateId = HCP_GATE_ID_CONNECTIVITY;
			break;
	}

	if(GateId == HCP_GATE_ID_CARD_TYPEA)
	{
		pushShort(HCP_GATE_ID_CARD_TYPEA);
	}
	else if(GateId == HCP_GATE_ID_CARD_TYPEB)
	{
		pushShort(HCP_GATE_ID_CARD_TYPEB);
	}
	//ReaderMode
	else if(GateId == HCP_GATE_ID_READER_TYPEA)
	{
		pushShort(HCP_GATE_ID_READER_TYPEA);
	}
	else if(GateId == HCP_GATE_ID_READER_TYPEB)
	{
		pushShort(HCP_GATE_ID_READER_TYPEB);
	}
	else
	{
		pushShort((jbyte)-1);
	}
}

#define READER_SERVICE_ID          2
#define CARD_EMULATION_SERVICE_ID  3
#define CONNECTIVITY_SERVICE_ID    4

void isHciServiceAvaliable(void)
{
	u8 serviceID = (u8)popShort();
	u8 result = 0;
	u8 state;
	
	state = HCI_GetServiceState();
	
	//if ((s8)g_SWP_LLC_Context.Status < SWP_ESTABLISHED_LINK_STATE)
	if ((state & HCI_SERVICE_STATE_SWP_SUPPORT) == 0x00)
	{
		result = 2;
		pushShort(result);
		return;
	}
	
	if (JcvmConfig_GetRfCommunicationSwitch() == 1)
	{
		result = 2;
		pushShort(result);
		return;
	}
	
	if (serviceID == READER_SERVICE_ID)
	{
		//if ( (GetPipeIndex(HCP_GATE_ID_READER_TYPEA) == HCP_PIPE_NOT_CREATED) || 
		//	 (GetPipeIndex(HCP_GATE_ID_READER_TYPEB) == HCP_PIPE_NOT_CREATED) )
		if ( (state & HCI_SERVICE_STATE_READER_TYPE_A) == 0 ||
			 (state & HCI_SERVICE_STATE_READER_TYPE_B) == 0 )
		{
			result = 1;
		}
	}
	else if (serviceID == CARD_EMULATION_SERVICE_ID)
	{
		//if ( (GetPipeIndex(HCP_GATE_ID_CARD_TYPEA) == HCP_PIPE_NOT_CREATED) || 
		//	 (GetPipeIndex(HCP_GATE_ID_CARD_TYPEB) == HCP_PIPE_NOT_CREATED) )
		if ( (state & HCI_SERVICE_STATE_CARD_EMULATION_TYPE_A) == 0 ||
			 (state & HCI_SERVICE_STATE_CARD_EMULATION_TYPE_B) == 0 )
		{
			result = 1;
		}
	}
	else if (serviceID == CONNECTIVITY_SERVICE_ID)
	{
		//if (GetPipeIndex(HCP_GATE_ID_CONNECTIVITY) == HCP_PIPE_NOT_CREATED)
		if ((state & HCI_SERVICE_STATE_CONNECTIVITY) == 0)
		{
			result = 1;
		}
	}
	else
	{
		//check terminal profile 26th byte bit2 and 31th byte bit1 is 00
		result = 1;
	}
	
	pushShort(result);
}


/// @CardEmulationMessage : public void prepareAndSendSendDataEvent(byte[] data, short offset, short len)
void sendSendDataEventHCI(void)
{
	jshort dataLen/*, null*/;
	PN8 bArrayAddr;
	
	/*null = */	popShort();   // not use parameter

	dataLen = popShort();
	bArrayAddr = getArrayDataWithCheck(dataLen);;
	
	//SWP_HCP_SetEventHeader(HCP_CE_EVT_SEND_DATA);
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_CARD_GATE_EVT_SEND_DATA );

 	EP_EepromToRam((u8*)&Swp_Buffer[1], (u8*)(bArrayAddr), dataLen);

 	//SWP_HCP_SetWriteMessageSize(dataLen);

 	//SWP_HCP_InitMessageTransmission( );

	HCI_SendMessage(g_Mode, g_RFType, Swp_Buffer, dataLen+1);	
}


/// @CardEmulationMessage : public void prepareAndSendGetParameterCommand(byte paramID)
/// @ReaderMessage        : public void prepareAndSendGetParameterCommand(byte paramid)
void getParaCommandHCI(void)
{
	u8 paramID;
	u8 GateId, PipeId;
	
	paramID = (jbyte)popShort();
	
//	GateId = g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId;
//	PipeId = g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].PipeId;
	
	switch(g_Mode)
	{
		case HCI_MODE_CARD_EMUALTION:
			if (g_RFType == HCI_RF_TYPE_TYPE_A)
			{
				GateId = HCP_GATE_ID_CARD_TYPEA;
			}
			else if (g_RFType == HCI_RF_TYPE_TYPE_B)
			{
				GateId = HCP_GATE_ID_CARD_TYPEB;
			}
			break;
		
		case HCI_MODE_READER:
			if (g_RFType == HCI_RF_TYPE_TYPE_A)
			{
				GateId = HCP_GATE_ID_READER_TYPEA;
			}
			else if (g_RFType == HCI_RF_TYPE_TYPE_B)
			{
				GateId = HCP_GATE_ID_READER_TYPEB;
			}
			break;

		case HCI_MODE_CONNECTIVITY:
			GateId = HCP_GATE_ID_CONNECTIVITY;
			break;
	}
	
	if(GateId == HCP_GATE_ID_CARD_TYPEA)
	{
		if(paramID > 0x09)
		{
			//抛出”HCI_RESOURCES_NOT_AVAILABLE“
			pushShort((jbyte)-1);
			return;
		}
	}
	else if(GateId == HCP_GATE_ID_CARD_TYPEB)
	{
		if(paramID > 0x06)
		{
			//抛出”HCI_RESOURCES_NOT_AVAILABLE“
			pushShort((jbyte)-1);
			return;
		}
	}
	//ReaderMode
	else if(GateId == HCP_GATE_ID_READER_TYPEA)
	{
		//nothing todo
	}
	else if(GateId == HCP_GATE_ID_READER_TYPEB)
	{
		//nothing todo
	}
	else
	{
		pushShort(0x00);
		return;
	}
	
	// start a get parameter listener
	g_SwpHciEvents[EVENT_GET_PARAMETER_RESPONSE] = HCI_API_EVENT_GET_PARAMETER_RESPONSE;

	SWP_HCP_AnyGetParameterHCI ( PipeId , (u8)paramID);

	pushShort(0x00);
}

/// @ConnectivityService : public void prepareAndSendTransactionEvent(byte[] aid, short aidOffset, short aidLen, byte[] parameters, short parametersOffset, short parametersLen)
void SWP_HCI_Interface_TransactionEvent_6paras(void)
{
	u16 parametersLen = (u16)popShort();
	PN8 parametersAddr = getArrayDataWithCheck(parametersLen);
	
	u16 aidLen = (u16)popShort();
	PN8 aidAddr = getArrayDataWithCheck(aidLen);

	if ((HCI_GetServiceState() & HCI_SERVICE_STATE_CONNECTIVITY) == 0x00)
	{
		return;
	}

	//SWP_HCP_SetEventHeader(HCP_CNN_EVT_TRANSACTION);
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_CONNECTIVITY_GATE_EVT_TRANSACTION );
	
	if (aidLen != 0)
	{
		// process 6 parameters
		EP_EepromToRam((u8*)&Swp_Buffer[1], (u8*)(aidAddr), aidLen);
		//SWP_HCP_SetWriteMessageSize(aidLen);
	}
	
 	EP_EepromToRam((u8*)&Swp_Buffer[1+aidLen], (u8*)(parametersAddr), parametersLen);

 	//SWP_HCP_SetWriteMessageSize(parametersLen);

 	//SWP_HCP_InitMessageTransmission( );

	//Swp_Hcp_SendMessageByPipeID( g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CONNECTIVITY)].PipeId );
	
	HCI_SendMessage(HCI_MODE_CONNECTIVITY, g_RFType, Swp_Buffer, 1+aidLen+parametersLen);
	
	SWP_UICC_Main();      // SWP process function
}


/// @ConnectivityService : public void prepareAndSendTransactionEvent(byte[] parameters, short parametersOffset, short parametersLen)
void SWP_HCI_Interface_TransactionEvent_3paras(void)
{
	u32 i;
	u16 parametersLen = (u16)popShort();
	PN8 parametersAddr = getArrayDataWithCheck(parametersLen);
	
	if ((HCI_GetServiceState() & HCI_SERVICE_STATE_CONNECTIVITY) == 0x00)
	{
		return;
	}

	//SWP_HCP_SetEventHeader(HCP_CNN_EVT_TRANSACTION);
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_CONNECTIVITY_GATE_EVT_TRANSACTION );
	
 	EP_EepromToRam((u8*)&Swp_Buffer[1], (u8*)(parametersAddr), parametersLen);

 	//SWP_HCP_SetWriteMessageSize(parametersLen);

 	//SWP_HCP_InitMessageTransmission( );

	//Swp_Hcp_SendMessageByPipeID( g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CONNECTIVITY)].PipeId );
	HCI_SendMessage(HCI_MODE_CONNECTIVITY, g_RFType, Swp_Buffer, 1+parametersLen);
	
	SWP_UICC_Main();      // SWP process function
}

/// @ReaderMessage : public void prepareAndSendWriteXchgDataCommand(byte timeout, byte[] data, short offset, short len)
void SendWriteXchgDataCommand(void)
{
	jshort  len;
	jref	dataRef;
	jbyte   timeout;
	PN8  dataAddr;
	
	timeout = (jbyte)popShort();
	len = popShort();

	dataAddr = getArrayDataWithCheck(len);

	//SWP_HCP_SetCommandHeader(HCP_RM_CMD_XCHG_DATA);
	Swp_Buffer[0] = HCP_MSG_COMMAND_TYPE | HCP_READER_GATE_CMD_XCHG_DATA;
	
	//swpWriteMsgDataAddr = (u8*)SWP_HCP_GetWriteMessageAddr();

	Swp_Buffer[1] = 0x00;//CTR字节
	if(timeout != (jbyte)-1)
	{
		Swp_Buffer[1] |= (timeout | 0x10);
	}

 	EP_EepromToRam((u8*)&Swp_Buffer[2], (u8*)(dataAddr), len);

 	//SWP_HCP_SetWriteMessageSize(len + 1);//加上CTR的一个字节数据；

 	//SWP_HCP_InitMessageTransmission( );

	//Swp_Hcp_SendMessage( );
	
	HCI_SendMessage(HCI_MODE_READER, g_RFType, Swp_Buffer, 2+len);

	g_SwpHciEvents[EVENT_WRITE_EXCHANGE_DATA_RESPONSE] = HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE;
	
	SWP_UICC_Main();      // SWP process function
}

void restartReaderModeProcedure(void)
{
	u8 state;
	
	state = HCI_GetServiceState();
	
	if ( ((state & HCI_SERVICE_STATE_READER_TYPE_A) == 0x00) && ((state & HCI_SERVICE_STATE_READER_TYPE_B) == 0x00) )
	{
		pushShort(0);
		return;
	}
	
	//SWP_HCP_SetEventHeader(HCP_EVT_HCI_END_OF_OPERATION);
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_READER_GATE_EVT_END_OPERATION );
	
	// send event
	//SWP_HCP_InitMessageTransmission( );
	//Swp_Hcp_SendMessage();
	
	if ((state & HCI_SERVICE_STATE_READER_TYPE_A) != 0x00)
	{
		HCI_SendMessage(HCI_MODE_READER, 1, Swp_Buffer, 1);
	}
	else
	{
		HCI_SendMessage(HCI_MODE_READER, 2, Swp_Buffer, 1);
	}
	
	SWP_UICC_Main();      // SWP process function
	
	//send request
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_READER_GATE_EVT_REQUESTED );
	
	if ((state & HCI_SERVICE_STATE_READER_TYPE_A) != 0x00)
	{
		HCI_SendMessage(HCI_MODE_READER, 1, Swp_Buffer, 1);
	}
	else
	{
		HCI_SendMessage(HCI_MODE_READER, 2, Swp_Buffer, 1);
	}
	
	SWP_UICC_Main();      // SWP process function
	
	pushShort(0);
}

void eventReaderRequestProcedure(void)
{
	u8 state;
	
	state = HCI_GetServiceState();
	
	if ( ((state & HCI_SERVICE_STATE_READER_TYPE_A) == 0x00) && ((state & HCI_SERVICE_STATE_READER_TYPE_B) == 0x00) )
	{
		pushShort(0);
		return;
	}
	
	//SWP_HCP_SetEventHeader(HCP_EVT_HCI_END_OF_OPERATION);
	//Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_EVT_HCI_END_OF_OPERATION );
	
	//send request
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_READER_GATE_EVT_REQUESTED );
	
	if ((state & HCI_SERVICE_STATE_READER_TYPE_A) != 0x00)
	{
		HCI_SendMessage(HCI_MODE_READER, 1, Swp_Buffer, 1);
	}
	else
	{
		HCI_SendMessage(HCI_MODE_READER, 2, Swp_Buffer, 1);
	}
	
	SWP_UICC_Main();      // SWP process function

	pushShort(0);
}

void getListenerEvent(void)
{
	u8 i;

	//EVENT_ON_SEND_DATA 最后一个字节为01 表示EVENT_HCI_RECEPTION_FAILED
	//if (SWP_HCP_GetMessageErrorIndicator() == 0x01)
	if(g_Message[((g_MessageLen == 0) ? 0x00: g_MessageLen-1)] == 0x01)
	{
		pushShort((jbyte)0x82);
		return;
	}
	
	for (i=EVENT_ON_SEND_DATA; i<EVENTS_NUM; i++)
	{
		if (g_SwpHciEvents[i] & HCI_API_LISTENER_PROCESSED)
		{
			pushShort((jbyte)(g_SwpHciEvents[i] & ~HCI_API_LISTENER_PROCESSED));
			//g_SwpHciEvents[i] = 0;
			return;
		}
	}
	
	pushShort(0);
}


void connectivity_prepareAndSendCnnEvent()
{
	u32 i;

	//if (GetPipeIndex(HCP_GATE_ID_CONNECTIVITY) == HCP_PIPE_NOT_CREATED)
	if ((HCI_GetServiceState() & HCI_SERVICE_STATE_CONNECTIVITY) == 0x00)
	{
		pushShort(0);
		return;
	}

//	SWP_HCP_SetEventHeader(HCP_CNN_EVT_CONNECTIVITY);

// 	SWP_HCP_InitMessageTransmission( );

//	Swp_Hcp_SendMessageByPipeID( g_HCP_Context.NvmContext->Pipes[GetPipeIndex(HCP_GATE_ID_CONNECTIVITY)].PipeId );
	
	Swp_Buffer[0] = HCP_MSG_EVENT_TYPE | ( HCP_CONNECTIVITY_GATE_EVT_CONNECTIVITY ); 
	
	HCI_SendMessage(HCI_MODE_CONNECTIVITY, g_RFType, Swp_Buffer, 1);
	
	SWP_UICC_Main();      // SWP process function

	pushShort(0);

}
/// @end of group SWP_HCP Event Functions
/// @}

#endif 	 // BOOT__IO__SWP
#endif   // end of HCI_API
