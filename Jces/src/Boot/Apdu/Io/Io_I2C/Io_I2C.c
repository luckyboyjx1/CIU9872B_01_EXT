
#include "DataOperation.h"
#include "SysConfig.h"
#include "7816Util.h"
#include "Io_I2C.h"
#include "I2cHal.h"
#include "SysConfig.h"
#include "chipHead.h"
#include "Io.h"
#include "string.h"
#include "FunImp.h"
#include "ecpReadWrite.h"
#include "IoBuffer.h"
#include "Io_Uart.h"
#include "nativemethod_impl.h"
#include "MixMode.h"


#if HAL__IO__I2C

#include "Io_Protocol.h"


#define I2C_SLEEP_COUNT      		20
#define PH_I2C_RXSUB_READY      0x10
#define PH_I2C_STATE_RX_FINISH  0x11
#define PH_I2C_RXSUB_I_BLOCK    0x12
  
#define I2C_PCB_HEADER       (1)
#define I2C_APDU_OFFSET        1




//#define I2C_HEADER_RESEND_LRC_OFFSET    5


//#pragma DATA("bootRamArea")
volatile u8 I2CResetFlag;
//#pragma DATA()


volatile u8  g_I2cIsReceiveApdu;
volatile u8  I2c_HandleFinishFlag;
volatile u16  I2c_PrivTxLength;



u8  g_I2c_SBlockBuffer[I2C_S_BLOCK_BUFFER_SIZE];
u8  g_I2c_IBlockBuffer[I2C_I_BLOCK_BUFFER_SIZE];



void I2C_TxData(u8* txBuf, u16 length);
static void I2C_SendATR(void);

extern volatile u8 g_softReset;

void I2C_Init(void)
{
	
	I2CHal_SlaveInit(sysConfig_GetI2CAddr());

	ecpIoProtocol_Init(PROTOCOL_I2C_ID, I2C_TxData, g_I2c_IBlockBuffer, g_I2c_SBlockBuffer);
    //I2c_PrivTxLength = 0;
	I2C_RxDataReady();
	if (I2CResetFlag == I2C_STATUS_WARMRESET)
	{
		//is trigger system, but cold reset is clear, then is 7816 reset
		I2CResetFlag = I2C_STATUS_NONERESET;
		I2C_SendATR();
	}
    //else
    //{
    //    I2C_RxDataReady();
    //}
	
	g_I2cIsReceiveApdu = 0;
	
}

void I2C_RxDataReady(void)
{
	I2CHal_rxReady(protocol_status[PROTOCOL_I2C_ID].pPrivSendLen);
}

//u8 I2c_RxDataProcess(u16 dataLen)
//{
//	u16 i;
//
//	if ( (dataLen + g_TxRxOffset) > I2C_BUFFER_SIZE )
//	{
//		return 1;
//	}
//
//	g_TxRxOffset += dataLen;
//
//	return 0;
//}

void I2C_RxDataFinish(u8* rxData, u16 rxLength)
{
	u8 result;

	result = ecpIoProtocol_DecodeBlock(rxData, g_I2c_IBlockBuffer, rxLength, PROTOCOL_I2C_ID);
	//decode receive data
	if (result == TYPE_RECEIVE_DATA_TYPE_APDU)
	{

		g_I2cIsReceiveApdu = 1;
		I2CHal_StopRxData();
		I2CHal_StopTxData();
	}
	/*
	else if((result == TYPE_ERROR_RETRANSMIT_MAX)||(result ==TYPE_EMPTY_FRAME))
	{
		if(result == TYPE_ERROR_RETRANSMIT_MAX)
		{
			I2c_PrivTxLength = 0;
		}
		I2cHal_rxReady(I2c_PrivTxLength);
	}
	*/
	else if((result == TYPE_ERROR_RETRANSMIT_MAX)||(result ==TYPE_EMPTY_FRAME))
	{
		//I2c_PrivTxLength = 0;
		//I2CHal_rxReady(I2c_PrivTxLength);
		I2C_RxDataReady();
	}

	//else if(result == TYPE_EMPTY_FRAME)
//	{
//#if !I2C_GW_CUSTOM
//			I2c_PrivTxLength = 0;
//#endif
//			I2CHal_rxReady(I2c_PrivTxLength);
//	}
	else if(result == TYPE_WARMRESET)
	{
		I2CResetFlag = I2C_STATUS_WARMRESET;

	    //only i2c interface actived ,reset chip.
		if(MixMode_InterfaceIsConcurrency(IIC_INTERFACE)==0)
		{
			rtnBackup(I2C_REREST_FLAG);
		}
	}
}

void I2C_TxData(u8* txBuf, u16 length)
{
	MixMode_SetInterfaceActivedFlag(IIC_INTERFACE);
	I2CHal_txData(txBuf, length);
	//I2c_PrivTxLength = length;

}


/*
void I2c_TxDataFinsh(void)
{
	I2cHal_rxReady(I2c_PrivTxLength);
}
*/

static void I2C_SendATR(void)
{
//	u16 atrLen;
	ecpIoProtocol_TxStateMachine(BLOCK_NO_ERROR_SEND_ATR,0,0,PROTOCOL_I2C_ID);
}


void I2C_RxStateMachine(void)
{
}

void I2C_setandSendData(u8 *buffer, u16 length)
{
}


u8 I2C_isReceCommand(void)
{
    ////not only spi interface actived,when spi request reset
	if(I2CResetFlag == I2C_STATUS_WARMRESET)
	{
		MixMode_WarnReset(PROTOCOL_I2C);
		I2C_Init();
	}
	if (g_I2cIsReceiveApdu == 1)
	{
		g_I2cIsReceiveApdu = 0;
		I2c_HandleFinishFlag =1;
		return 1;
	}
//	else
//
//		I2c_HandleFinishFlag = 0;
//	}
	return 0;
}


u8 I2C_EnterSleep(void)
{
	if (g_I2cIsReceiveApdu == 1)
	{
		return 1;
	}
	
	if (I2CHal_isBusy())
	{
		return 1;
	}

	if(I2CResetFlag == I2C_STATUS_WARMRESET)
	{
		return 1;
	}
	return 0;
}
void I2cClearCount(void)
{

}
void I2C_GetCommand(void)	
{
	ppIso7816Util_Command_t		D_A		command;
	iso7816Util_GetCommand( &command );

	// 7816 protocol:cla ins p1 p2 p3  data(byte) le1 le2
	//i2c buff    :  cla ins p1 p2 lc  data(byte) le1
	command->Le =  ecpIoProtocol_GetApduLe(PROTOCOL_I2C_ID);

	command->length = g_I2c_IBlockBuffer[I2C_COMMAND_HEADER_LEN]+5;

    ecpMem_Copy(command->pData, g_I2c_IBlockBuffer, command->length);

	//ecpMem_Copy(&command->pData[I2C_COMMAND_HEADER_LEN], &g_I2c_IBlockBuffer[I2C_COMMAND_HEADER_LEN], g_I2c_IBlockBuffer[I2C_COMMAND_HEADER_LEN]+1);

	command->Le = (command->Le==0)?MAX_LE:command->Le;
	if( command->length == 5)
	{
		command->pData[I2C_COMMAND_HEADER_LEN] = command->Le;
	}
	ecpMem_Set(&command->pData[command->length], 0, U16_S_U16(IO_CMD_BUF_SIZE,command->length));


}
u8 I2C_HandleCommand(void)
{
	if(I2c_HandleFinishFlag)
	{
		I2c_HandleFinishFlag = 0;
		I2C_GetCommand( );
		return 1;
	}
	else
	{
		return 0;
	}
	
}


void I2C_SendStatus(void)
{

	ppIso7816Util_Response_t 	D_A		response;

	iso7816Util_GetResponse(&response);

	if (response->SW1SW2 == IO_NO_STATUS)
	{
		return;
	}

	ecpMem_Copy(&g_I2c_IBlockBuffer[RESPONSE_OFFSET_DATA], &response->pData[0], response->length - 2);

	ecpIoProtocol_TxStateMachine(BLOCK_NO_ERROR, response->SW1SW2, response->length - 2, PROTOCOL_I2C_ID);
	//I2C_Transmit_JVM();
	return ;
	
}


void I2C_IrqProcess(void)
{

}

//u8 I2c_IsActived(void)
//{
//	return I2cActivedFlag;
//}

#endif

