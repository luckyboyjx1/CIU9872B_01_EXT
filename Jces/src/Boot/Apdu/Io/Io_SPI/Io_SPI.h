#ifndef __IO_SPI_H__
#define	__IO_SPI_H__

#include "DataOperation.h"
#include "SysConfig.h"
#include "SpiHal.h"
#include "chipHead.h"
#include "bootConfigProfile.h"


#if HAL__IO__SPI

#include "Io_Protocol.h"

extern volatile u8 SpiResetFlag;
extern volatile u8  g_SpiIsReceiveApdu;

#define RECEIVE_OK           				   0
#define RECEIVE_NOT_FINISH           		   1
#define RECEIVE_EEROR           			   2

#define SPI_COMMAND_HEADER_LEN  4
#define SPI_COMMAND_EXT_LC1    4
#define SPI_COMMAND_EXT_LC2    5





#define SPI_STATUS_INIT       		0xFF     //
#define SPI_STATUS_NONERESET  		0x00     //
#define SPI_STATUS_WARMRESET      	0x01     //



//#define SPI_INTERFACE_DEACTIVED        0X00
//#define SPI_INTERFACE_ACTIVED          0X01

typedef enum
{
	SPI_INIT,
	SPI_STATE_RX,
	SPI_STATE_TX,
	SPI_STATE_ERROR,
}SPI_STATE;




 /*SPI protocol structure
 *=================================================================*/
typedef struct
{
	//u8                          Spi_ConBuffer[264];
	u16                         fullness;
	u8     	                    state;        // spi state	
	u8     	                    rxState;        // spi state	
	
	u8                          receiverror;
	 		
} Spi_Control_t, *Sspi_Control_t;

extern void Spi_Init(void);
extern u8 SPI_ReceCOMMANDProcess(u8 * pdata ,u16  len);
extern void Spi_RxDataReady(void);
extern void SPI_RxStateMachine(void);
extern void Spi_setandSendData(u8 *buffer, u16 length);
extern u8 Spi_isReceCommand(void);
extern void Spi_GetCommand(void);
extern u8 Spi_isDataTransmitStatus(void);

extern void Spi_RxDataFinish(u8* rxData, u16 rxLength);
#if  HAL_IO_SPI_MASTER
extern u8 SPI_Master_Send(u8* cmdBuf, u16 cmdLen, u8 isPullUpSsn);
extern u8 SPI_Master_Receive(u8 * buf, u16 len, u8 isPullUpSsn);
extern u8  ecpSpiMasterSoftReset(u8 *cipBuf, u32 *datalen, u32 bufSize);
#endif

extern void Spi_SendStatus(void);
extern void SPI_TranmitFinshProcess(void);
extern u8 Spi_HandleCommand(void);
extern u8 Spi_EnterSleep(void);
#else
#define Spi_Init()
#define SPI_ReceCOMMANDProcess(pdata,len)
#define SPI_SendATR()
#define SPI_RxStateMachine()
#define Spi_setandSendData(buffer, length)
#define Spi_isReceCommand( )                 0
#define Spi_GetCommand()
#define Spi_SendStatus()
#define Spi_isDataTransmitStatus()            0
#define Spi_HandleCommand()                   0
#define masterSpiSendData(apdu, len)
#define masterSpiSendData(apdu, len)
#define masterSpiReceiveData(buf, len)
#define ecpSpiMasterSoftReset(cipBuf, datalen,  bufSize)
#define Spi_EnterSleep()							0

#endif


#endif

