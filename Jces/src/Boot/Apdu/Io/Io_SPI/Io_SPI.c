
#include "DataOperation.h"
#include "SysConfig.h"
#include "7816Util.h"
#include "Io_SPI.h"
#include "SpiHal.h"
#include "SysConfig.h"
#include "chipHead.h"
#include "Io.h"
#include "string.h"
#include "FunImp.h"
#include "ecpReadWrite.h"
#include "IoBuffer.h"
#include "MixMode.h"



#if HAL__IO__SPI

#include "Io_Protocol.h"

//extern u8    spitransmit ;
//extern u8    SpiTxStartFlag;
//extern u8  SPI_TxBuffer[];
//static u16 SPI_Transmit_JVM(void);
//static void SPI_SendError(u8 status);

//extern u8    SpiSleepCount ;

//#define SPI_SLEEP_COUNT      		20
//#define PH_SPI_RXSUB_READY      0x10
//#define PH_SPI_STATE_RX_FINISH  0x11
//#define PH_SPI_RXSUB_I_BLOCK    0x12
  
//#define SPI_PCB_HEADER       (1)
//#define SPI_APDU_OFFSET        1




//#define SPI_HEADER_RESEND_LRC_OFFSET    5


//#pragma DATA("bootRamArea")
volatile u8 SpiResetFlag;
//u8 SpiActivedFlag;
//#pragma DATA()


volatile u8  g_SpiIsReceiveApdu;
volatile u8  Spi_HandleFinishFlag;
//volatile u16  Spi_PrivTxLength;



u8  g_Spi_SBlockBuffer[S_BLOCK_BUFFER_SIZE];
//__attribute__( ( section( "spiDataArea" ) ) )
u8  g_Spi_IBlockBuffer[I_BLOCK_BUFFER_SIZE];







void Spi_TxData(u8* txBuf, u16 length);
static void SPI_SendATR(void);


void Spi_Init(void)
{
	
#if HAL_IO_SPI_MASTER
	ecpSpi_MasterInit();
#else

	SpiHal_SlaveInit();

	ecpIoProtocol_Init(PROTOCOL_SPI_ID, Spi_TxData, g_Spi_IBlockBuffer, g_Spi_SBlockBuffer);

//	Spi_PrivTxLength = 0;
	Spi_RxDataReady();
//	if (SpiResetFlag == SPI_STATUS_WARMRESET)
//	{
//		//is trigger system, but cold reset is clear, then is 7816 reset
//		SpiResetFlag = SPI_STATUS_NONERESET;
//		SPI_SendATR();
//	}
	
	g_SpiIsReceiveApdu = 0;
#endif
	
}
#if !HAL_IO_SPI_MASTER
void Spi_RxDataReady(void)
{
	SpiHal_rxReady(protocol_status[PROTOCOL_SPI_ID].pPrivSendLen);
	//SpiHal_rxReady();
}

//u8 Spi_RxDataProcess(u16 dataLen)
//{
//	u16 i;
//
//	if ( (dataLen + g_TxRxOffset) > SPI_BUFFER_SIZE )
//	{
//		return 1;
//	}
//
//	g_TxRxOffset += dataLen;
//
//	return 0;
//}

void Spi_RxDataFinish(u8* rxData, u16 rxLength)
{
	u8 result;

	result = ecpIoProtocol_DecodeBlock(rxData, g_Spi_IBlockBuffer, rxLength, PROTOCOL_SPI_ID);

	//decode receive data
	if (result == TYPE_RECEIVE_DATA_TYPE_APDU)
	{

		g_SpiIsReceiveApdu = 1;
		SpiHal_StopRxData();
		SpiHal_StopTxData();
	}
	else if((result == TYPE_ERROR_RETRANSMIT_MAX)||(result ==TYPE_EMPTY_FRAME))
	{
		//SpiHal_rxReady();
		Spi_RxDataReady();
	}

	else if(result == TYPE_WARMRESET)
	{
		SpiResetFlag = SPI_STATUS_WARMRESET;
	//		if(IoUart_isActived()!=1)
	    //only spi interface actived ,reset chip.
		//if(MixMode_InterfaceIsConcurrency(SPI_INTERFACE)==0)
		{
			//ClearrfFirstSelectCmdFlag();
			//ecpSysHal_SystemReset(SPI_REREST_FLAG);
			//rtnBackup(SPI_REREST_FLAG);
		}
	}
}

void Spi_TxData(u8* txBuf, u16 length)
{
//	MixMode_SetInterfaceActivedFlag(SPI_INTERFACE);
	SpiHal_txData(txBuf, length);
}
#endif

/*
void Spi_TxDataFinsh(void)
{
	SpiHal_rxReady(Spi_PrivTxLength);
}
*/

static void SPI_SendATR(void)
{
//	u16 atrLen;
	ecpIoProtocol_TxStateMachine(BLOCK_NO_ERROR_SEND_ATR,0,0,PROTOCOL_SPI_ID);
}


void SPI_RxStateMachine(void)
{
//	SPI_Ctrl.rxState = PH_SPI_STATE_RX_FINISH;
//
//	memcpy(&SPI_Buffer[0], &SPI_CachBuffer[0], SPI_Ctrl.fullness);
}

void Spi_setandSendData(u8 *buffer, u16 length)
{
//	ecpSpi_Slave_Tx_IT(buffer, length);
}


u8 Spi_isReceCommand(void)
{
    ////not only spi interface actived,when spi request reset
//	if(SpiResetFlag == SPI_STATUS_WARMRESET)
//	{
//		//MixMode_WarnReset(PROTOCOL_SPI);
//		Spi_Init();
//	}
	if (g_SpiIsReceiveApdu == 1)
	{
		g_SpiIsReceiveApdu = 0;
		Spi_HandleFinishFlag =1;
		return 1;
	}
		
	return 0;
}


u8 Spi_EnterSleep(void)
{
	if(g_SpiIsReceiveApdu == 1)
	{
		return 1;
	}
//	if( ecpSpi_isProcess() == 1)
//	{
//		return 1;
//	}
	if(SpiResetFlag == SPI_STATUS_WARMRESET)
	{
		return 1;
	}
	return 0;
}
void SpiClearCount(void)
{
//	SpiSleepCount = 0;
}
void Spi_GetCommand(void)	
{
	ppIso7816Util_Command_t		D_A		command;
	iso7816Util_GetCommand( &command );
#if PROTOCOL_EXTEND_LENGTH
	//EXT 7816 protocol:cla ins p1 p2 p3  data(byte) le1 le2
	//spi buff    :     cla ins p1 p2 lc1 lc2 data(byte)

	// copy commamd head 4 byte
	ecpMem_Copy(command->pData, g_Spi_IBlockBuffer, SPI_COMMAND_HEADER_LEN);
	//data lc length
	command->Le =  ecpIoProtocol_GetApduLe(PROTOCOL_SPI_ID);
	command->Lc = (g_Spi_IBlockBuffer[SPI_COMMAND_EXT_LC1]<<8)+ (g_Spi_IBlockBuffer[SPI_COMMAND_EXT_LC2]);

	if(protocol_status[PROTOCOL_SPI_ID].extendflag ==1)
	//if(sysConfig_GetSpiFrameConvert() == 0x01)
	{
		bRamBuffer[APDU_EXTENDEDLENGTH_FLAG] = 1;
		command->length =  APDU_EXT_HEADER_LENGTH + command->Lc;
		//if(command->Lc == 0)
		//{
		//	command->pData[APDU_P3_OFFSET] = 0;
		//	command->pData[APDU_P3_OFFSET +1] = (command->Le>>8)&0xff;
		//	command->pData[APDU_P3_OFFSET +2] = (command->Le)&0xff;
		//}
		//else
		//{
		    //copy len+data
			command->pData[APDU_P3_OFFSET] = 0;
		    ecpMem_Copy(&command->pData[APDU_P3_OFFSET+1], &g_Spi_IBlockBuffer[SPI_COMMAND_EXT_LC1], command->Lc+2);
		//}
		command->Le = (command->Le==0)?MAX_EXTEND_LE:command->Le;
		ecpMem_Set(&command->pData[command->length], 0, U16_S_U16(IO_CMD_BUF_SIZE,command->length));
	}
	else
	{
		bRamBuffer[APDU_EXTENDEDLENGTH_FLAG] = 0;
		command->length =  APDU_HEADER_LENGTH + command->Lc;
		
	//	if(command->Lc == 0)
	//	{
	//		command->pData[APDU_P3_OFFSET] = command->Le ;
	//	} 
	//	else
	//	{
		    //copy len+data
		    ecpMem_Copy(&command->pData[APDU_P3_OFFSET], &g_Spi_IBlockBuffer[SPI_COMMAND_EXT_LC2], command->Lc+1);
	//	}
		command->Le = (command->Le==0)?MAX_LE:command->Le;
		ecpMem_Set(&command->pData[command->length], 0, U16_S_U16(IO_CMD_BUF_SIZE,command->length));
		if((command->pData[APDU_INS_OFFSET] == 0xC0) && (command->length == 6))
		{
			command->length = 5;
			command->pData[APDU_P3_OFFSET] = command->pData[APDU_DATA_OFFSET];
			command->pData[APDU_DATA_OFFSET] = 0;
			command->Le = command->pData[APDU_DATA_OFFSET];
			command->Le = command->Le==0?MAX_LE:command->Le;
		}
	}
#else

	// 7816 protocol:cla ins p1 p2 p3  data(byte) le1 le2
	//spi buff    :  cla ins p1 p2 lc  data(byte) le1
	command->Le =  ecpIoProtocol_GetApduLe(PROTOCOL_SPI_ID);

	command->length = g_Spi_IBlockBuffer[SPI_COMMAND_HEADER_LEN]+5;

    ecpMem_Copy(command->pData, g_Spi_IBlockBuffer, command->length);

	//ecpMem_Copy(&command->pData[SPI_COMMAND_HEADER_LEN], &g_Spi_IBlockBuffer[SPI_COMMAND_HEADER_LEN], g_Spi_IBlockBuffer[SPI_COMMAND_HEADER_LEN]+1);

	command->Le = (command->Le==0)?MAX_LE:command->Le;
	if( command->length == 5)
	{
		command->pData[SPI_COMMAND_HEADER_LEN] = command->Le;
	}
	ecpMem_Set(&command->pData[command->length], 0, U16_S_U16(IO_CMD_BUF_SIZE,command->length));

#endif

}
u8 Spi_HandleCommand(void)
{
	if(Spi_HandleFinishFlag)
	{
		Spi_HandleFinishFlag = 0;
		Spi_GetCommand( );
		return 1;
	}
	else
	{
		return 0;
	}
	
}


void Spi_SendStatus(void)
{

	ppIso7816Util_Response_t 	D_A		response;

	iso7816Util_GetResponse(&response);

	if (response->SW1SW2 == IO_NO_STATUS)
	{
		return;
	}

	ecpMem_Copy(&g_Spi_IBlockBuffer[RESPONSE_OFFSET_DATA], &response->pData[0], response->length - 2);

	ecpIoProtocol_TxStateMachine(BLOCK_NO_ERROR, response->SW1SW2, response->length - 2, PROTOCOL_SPI_ID);
	//SPI_Transmit_JVM();
	return ;
	
}


void Spi_IrqProcess(void)
{

}
/*
u8 Spi_IsActived(void)
{
	//return SpiActivedFlag;
	if(g_interfaceActivedFlag & SPI_INTERFACE)
	{
		return 1;
	}
	return  0;
}
*/
#if HAL_IO_SPI_MASTER

u8 SPI_Master_Send(u8* cmdBuf, u16 cmdLen, u8 isPullUpSsn)
{
	u8 isreturn;
	SpiMasterHal_txData(cmdBuf, cmdLen, isPullUpSsn);
	/*wait transmit finish*/
	do
	{
		isreturn = getTransmitFinishFlag();
		
	}while(false == isreturn);
}

u8 SPI_Master_Receive(u8 * buf, u16 len, u8 isPullUpSsn)
{
	SpiMasterHal_rxData(buf, len, isPullUpSsn);
}



u8  ecpSpiMasterSoftReset(u8 *cipBuf, u32 *datalen, u32 bufSize)
{

//	u8 offset;
//	u8 ioBuffer[20];
//	u8 i;
//	u8  lrc;
//	u32	receiveLength;
//	u8  pcbByte;


//	offset = 0;
//	

//	pcbByte = 0x55;  // IIC 0x33 SPI 0X55
//	
//	ioBuffer[offset++] = pcbByte;
//	ioBuffer[offset++] = 0xCF;
//	ioBuffer[offset++] = 0x00;
//	ioBuffer[offset++] = 0x00;
//	ioBuffer[offset++] = 0x00;
//	lrc = 0;
//	for (i=1; i<offset; i++)
//	{
//		lrc = lrc ^ ioBuffer[i];
//	}
//	ioBuffer[offset++] = ~lrc;

//	//send
//	ecpSpi_MasterSendNBytes(ioBuffer,offset);
//	//clear pcb byte
//	ioBuffer[0] = 0x00;
//	
//	
//	ecpMem_Set(ioBuffer,0x00,5);
//	receiveLength = 1;
//	ecpSpi_MasterReceiveNBytes(&ioBuffer[0],receiveLength);

//	if(ioBuffer[0] != 0x55)
//	{
//		return 0;	
//	}
//	receiveLength = 5;
//	ecpSpi_MasterReceiveNBytes(&ioBuffer[0],receiveLength);
//	
//	if((ioBuffer[0] == 0xEF)&&(ioBuffer[1] == 0x00)&&(ioBuffer[2] == 0x00)&&(ioBuffer[3] == 0x00))
//	{
//		return 1;

//	}
//    return 0;

}

#endif
#endif

