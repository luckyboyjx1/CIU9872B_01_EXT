#ifndef __IO_I2C_H__
#define	__IO_I2C_H__

#include "DataOperation.h"
#include "SysConfig.h"
#include "I2cHal.h"
#include "chipHead.h"
#include "bootConfigProfile.h"


#if HAL__IO__I2C

#include "Io_Protocol.h"

extern volatile u8 I2CResetFlag;
extern volatile u8  g_I2cIsReceiveApdu;

#define RECEIVE_OK           				   0
#define RECEIVE_NOT_FINISH           		   1
#define RECEIVE_EEROR           			   2

#define I2C_COMMAND_HEADER_LEN  4
#define I2C_COMMAND_EXT_LC1    4
#define I2C_COMMAND_EXT_LC2    5

#define I2C_S_BLOCK_BUFFER_SIZE      (9)

#if  I2C_GW_CUSTOM
#define I2C_I_BLOCK_BUFFER_SIZE      (2060+512)
#endif

#if  !I2C_GW_CUSTOM
#define I2C_I_BLOCK_BUFFER_SIZE      (270)
#endif



#define I2C_STATUS_INIT       		0xFF     //
#define I2C_STATUS_NONERESET  		0x00     //
#define I2C_STATUS_COLDRESET     	0x01     //
#define I2C_STATUS_WARMRESET      	0x02     //
#define I2C_REQUEST_WARMRESET      	0x03     //


#define I2C_INTERFACE_DEACTIVED        0X00
#define I2C_INTERFACE_ACTIVED          0X01

typedef enum
{
	I2C_INIT,
	I2C_STATE_RX,
	I2C_STATE_TX,
	I2C_STATE_ERROR,
}I2C_STATE;




 /*I2C protocol structure
 *=================================================================*/
typedef struct
{
	//u8                          I2c_ConBuffer[264];
	u16                         fullness;
	u8     	                    state;        // spi state	
	u8     	                    rxState;        // spi state	
	
	u8                          receiverror;
	 		
} I2c_Control_t, *Si2c_Control_t;

extern volatile u16  I2c_PrivTxLength;

extern void I2C_Init(void);
extern u8 I2C_ReceCOMMANDProcess(u8 * pdata ,u16  len);
extern void I2C_RxDataReady(void);
extern void I2C_RxStateMachine(void);
extern void I2C_setandSendData(u8 *buffer, u16 length);
extern u8 I2C_isReceCommand(void);
extern void I2C_GetCommand(void);
extern u8 I2C_isDataTransmitStatus(void);

extern void I2C_RxDataFinish(u8* rxData, u16 rxLength);


extern void I2C_SendStatus(void);
extern void I2C_TranmitFinshProcess(void);
extern u8 I2C_HandleCommand(void);
extern u8 I2C_EnterSleep(void);
extern u8 I2C_IsActived(void);
#else
#define I2C_Init()
#define I2C_ReceCOMMANDProcess(pdata,len)
#define I2C_SendATR()
#define I2C_RxStateMachine()
#define I2C_setandSendData(buffer, length)
#define I2C_isReceCommand( )                 0
#define I2C_GetCommand()
#define I2C_SendStatus()
#define I2C_isDataTransmitStatus()            0
#define I2C_HandleCommand()                   0
#define masterI2cSendData(apdu, len)
#define masterI2cSendData(apdu, len)
#define masterI2cReceiveData(buf, len)
#define ecpI2cMasterSoftReset(cipBuf, datalen,  bufSize)
#define I2C_EnterSleep()							0

#endif


#endif

