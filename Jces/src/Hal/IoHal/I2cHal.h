#ifndef __I2cHal_H__
#define	__I2cHal_H__

#include "DataOperation.h"
#include "ecpDefine.h"
#include "SysConfigProfile.h"
/*=================================================================*
 * Types:
 *=================================================================*/

/*=================================================================*
 * Defines:
 *=================================================================*/
 
 
#if HAL__IO__I2C 


#define NTF_IO          0 // GPIO0
#define NTF_Out()       (GPIODIR &= ~(0x00000001<<NTF_IO))      //  output mode
#define NTF_In()        (GPIODIR |= (0x00000001<<NTF_IO))      //  imput mode
#define NTF_Set()       (GPIODAT_B(NTF_IO) = 1)             // Write 1
#define NTF_Clr()       (GPIODAT_B(NTF_IO) = 0)             // Write 0


#define IS_I2C_RX        0x01

#define MODE_R          1
#define MODE_W          0
#define I2C_NAK         1
#define I2C_ACK         0


/* Exported macro ------------------------------------------------------- */
#define I2C_ENABLE              0x40
#define I2C_DISABLE             0x3C
#define I2C_START_SET           0x20
#define I2C_START_CLEAR         0x5C
#define I2C_STOP_SET            0x10
#define I2C_STOP_CLEAR          0x6C
#define I2C_SI_CLEAR            0x74
#define I2C_AA_SET              0x04
#define I2C_AA_CLEAR            0x78
#define I2C_SUCCESS             0x01
#define I2C_ERROR               0xFF

/* -- MASTER STATES -- */
#define STAT_BUS_ERROR        0x00           /* Bus error during MST or selected slave modes */
#define STAT_I2C_IDLE         0xF8           /* No activity and no interrupt either... */
#define STAT_START            0x08           /* start condition sent */
#define STAT_RESTART          0x10           /* repeated start */
#define STAT_SLAW_ACK         0x18           /* SLA+W sent, ack received */
#define STAT_SLAW_NACK        0x20           /* SLA+W sent, nack received */
#define STAT_TX_DATA_ACK      0x28           /* Data sent, ACK'ed */
#define STAT_TX_DATA_NACK     0x30           /* Data sent, NACK'ed */
#define STAT_LOST_ARB         0x38           /* Master lost arbitration */
#define STAT_SLAR_ACK         0x40           /* SLA+R sent, ACK'ed */
#define STAT_SLAR_NACK        0x48           /* SLA+R sent, NACK'ed */
#define STAT_RX_DATA_ACK      0x50           /* Data received, ACK sent */
#define STAT_RX_DATA_NACK     0x58           /* Data received, NACK sent */
#define STAT_RESET_ACTIVATED  0xD0           /* Master reset is activated */
#define STAT_STOP_TRANSMIT    0xE0           /* Stop has been transmitted */

/* -- SLAVE STATES -- */
#define STAT_SLAVE_SLAW       0x60           /* SLA+W received */
#define STAT_SLAVE_SLAR_ACK   0xA8           /* SLA+R received, ACK returned */
#define STAT_SLV_LA           0x68           /* Slave lost arbitration */
#define STAT_GCA              0x70           /* GCA received */
#define STAT_GCA_LA           0x78           /* GCA lost arbitration */
#define STAT_RDATA            0x80           /* Data received */
#define STAT_SLA_NACK         0x88           /* Slave addressed, NACK returned */
#define STAT_GCA_ACK          0x90           /* Previously addresses with GCA, data ACKed */
#define STAT_GCA_NACK         0x98           /* GCA addressed, NACK returned */
#define STAT_RSTOP            0xA0           /* Stop received */
#define STAT_SLARW_LA         0xB0           /* Arbitration lost */
#define STAT_RACK             0xB8           /* Byte sent, ACK received */
#define STAT_SLAVE_RNACK      0xC0           /* Byte sent, NACK received */
#define STAT_FINAL            0xC8           /* Final byte sent, ACK received */
#define STAT_SLV_RST          0xD8

#define RECEIVE_OK 0
#define RECEIVE_ERROR 1
#define TAG_ERROR 2
#define LENGTH_ERROR 3
#define COMMAND_ERROR 4
#define CHECK_ERROR 5

#define SEND_OK 0

#define I2CHal_TYPE_NONE	 0
#define I2CHal_TYPE_TX		 1
#define I2CHal_TYPE_RX		 2

#define I2C_BUFFER_SIZE  270

typedef struct
{
    u8 volatile event;
    u16 wFrameLen;
    u8 g_commBuf[I2C_BUFFER_SIZE];
} ST_I2C;

extern ST_I2C g_stI2C;


//#define I2CSoftInit(pbI2CBuf) {g_stI2C.g_commBuf = pbI2CBuf;}
//#define I2C_GetFrameBuffer() g_stI2C.g_commBuf

void I2C_IRQHandlerProcess(void);

extern u8  RxStartFlag;
extern u8  I2CHal_CachBuffer[I2C_BUFFER_SIZE];
extern  void I2CHal_SlaveInit(u8 addr);
extern void I2CHal_enableInterrupt(void);
extern void I2CHal_rxReady(u16 length);
extern void I2CHal_StopTxData(void);
extern void I2CHal_StopRxData(void);
extern u16 I2CHal_txData(u8* txData, u16 txLength);
extern u8 I2CHal_isBusy(void);

#else

//#define  ecpSpi_SlaveInit(clkMode)
//#define  ecpSpi_DmaRx(len)
//#define  ecpSpi_DmaTx(buf,len)
//#define  ecpSpi_stopDMATransmit()
//#define  SPI_ResetChip(void)
//#define  SPI_isSoftReset(void)
#define I2CHal_isBusy()			0


#endif

#endif
