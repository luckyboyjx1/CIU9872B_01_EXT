/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
*
*FileName    :T0ApduImpl.c
*
*Author      :
*
*Version     :V1.0
*
*Date        :2013-12-4

*Description :该文件主要用于管理T0数据收发
*
*History     :

author    time      version    description
 xuyz    2013/12/04   V1.0       创建该文件
************************************************************************************************************************************************************************************************/

#include "typedef.h"
#include "T0ApduImpl.h"
#include "Interpreter.h"
#include "interface.h"
#include "uicc_main.h"
#include "Apdu.h"
#include "UartHal.h"
#include "Io.h"
#include "error.h"
#include "ToolkitRFM.h"

#if (UICC__FUNCTION_CMCCAPI||UICC__FUNCTION__UICCAPI)	//remove warning,dengyiwen,2022/9/27
#include  "uicc_interface.h"	
#endif

void SetOutgoingAndSend(SWORD bOff,SWORD len)
{
	apdu_ApduSetOutgoingAndSend(bOff, len);
}
//void Delay4etu(void)
//{

//}
//void timer0(BYTE mode)
//{

//}

//void SendByte(BYTE sendByte)
//{

//}

void GlobalSendResponse(void)
{
	WORD  idata status;
	BYTE  idata curChannel;

	status = (SW1<<8)|SW2;

#if (UICC__FUNCTION_CMCCAPI||UICC__FUNCTION__UICCAPI)
	ClearTheVolatileByteArray(); ////The content of this array is not guaranteed across APDUs.
#endif
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT	
	if((G_Deliver_T0_Submit & 0x02)==0x02)		
	{
		apdu_SendStatusAndReceiveCommandSingle(status);
	}
	else
#endif
	{
		apdu_SendStatusAndReceiveCommand(status);
	}

	curChannel = GPGetCurrenteChannel();

	G_NextAPDUFlag = curChannel + 1;
}

void GlobalReceiveData(void)
{
	apdu_ApduSetIncomingAndReceive();
}

void GlobalSendData(void)
{
	WORD  idata dataLength;

	dataLength = ((P3==0x00)?0x100:P3);

	apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, dataLength);
}

void ThrowError(WORD exceptionType)
{
	throw_error(exceptionType);
}
BYTE GetCLAChannel(void)
{
	BYTE idata channelNum;

	channelNum = ILLEGAL_CHANNEL;

	if(CLA == 0xFF)
	{
		return  ILLEGAL_CHANNEL;
	}

    switch(CLA & 0xF0)
    {
        case 0x00:
        case 0x10:
        case 0x80:  // JCRE: In addition, cards compliant with the Java  Card 3 Platform specification must also support proprietary class values of the CLA byte equal to 0x8X, 0x9X, 0xAx and 0xBX for channel numbers in the range 0-3
        case 0x90:
        case 0xA0:
            channelNum = CLA & 0x03;//0x0X and 0x1X in the APDU command encode channel
            break;
        case 0xB0:
        case 0x20:
        case 0x30:
            return 0x00;
        default:
            break ;
    }
	return channelNum;
}

BYTE GPGetCurrenteChannel(void)
{
	BYTE idata curChannel;

	curChannel = GetCLAChannel();

	if(curChannel == ILLEGAL_CHANNEL)
	{
		return curChannel;
	}

#if JCVM__IO__SWP
	if(io_GetCurrentProtocol() == PROTOCOL_SWP)
	{
		curChannel += VM_MAX_LOGICAL_CHANNELS_CT;
	}
#endif
	#if JCVM__IO__SPI
	if(io_GetCurrentProtocol() == PROTOCOL_SPI)
	{
		curChannel += VM_MAX_LOGICAL_CHANNELS_CT;
	}
#endif

	if(curChannel >= VM_MAX_LOGICAL_CHANNELS)
	{
		return ILLEGAL_CHANNEL;
	}

	return curChannel;
}

void SendResponseSingle(void)
{
	WORD  idata status;
//	BYTE  idata curChannel;
	
	status = (SW1<<8)|SW2;

	apdu_SendStatusAndReceiveCommandSingle(status);

	//curChannel = GPGetCurrenteChannel();

	//G_NextAPDUFlag = curChannel + 1;

}

void SendDataSingle(void)
{
	BYTE  idata flag;

	flag = INS;
	
	//ecpUart_TxMulBytes(&g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_INS_OFFSET], 1, SEND_PRC_BYTE);
	ecpUart_TxMulBytes(&flag, 1, SEND_PRC_BYTE);
	ecpUart_TxMulBytes(RecData,  (P3==0)?0x100:P3, SEND_DATA);		
}
