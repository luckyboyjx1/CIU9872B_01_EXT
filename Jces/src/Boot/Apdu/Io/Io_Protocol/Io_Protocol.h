#ifndef __IO_PROTOCOL_H__
#define	__IO_PROTOCOL_H__

#include "DataOperation.h"
#include "SysConfig.h"
#include "SpiHal.h"
#include "chipHead.h"

typedef  void (*pSendToIO)(u8 *pData,u16 len);
typedef  void (*pRevFinshCallBack)(void);
typedef struct
{
	u8      			extendflag;
	u8 					R_block_NUMcnt;
    u16 					ApduLe;
	u8*				    pIBlockBuf;
	u16					iBlockDataLen;
	u8*				    pSBlockBuf;
	u8*					pPrivSendBuf;
	u16					pPrivSendLen;
	pSendToIO			pSendProtocol;
}PROTOCOL_STATUS;

#define RESPONSE_OFFSET_SYNC      	0
#define RESPONSE_OFFSET_SW1      	1
#define RESPONSE_OFFSET_SW2     	2
#define RESPONSE_OFFSET_LEN1     	3
#define RESPONSE_OFFSET_LEN2     	4
#define RESPONSE_OFFSET_DATA     	5

#define RESPONSE_I_BLOCK_MINISIZE   6

#define COMMUNICATION_REPEAT_NUM  	2

#define PROTOCOL_SPI_ID          0
#define PROTOCOL_I2C_ID          1
#define PROTOCOL_CHANNEL_NUM     2

#define	BLOCK_NO_ERROR				0x00
#define	BLOCK_CRC_OR_LRC_ERROR		0x01
#define BLOCK_OTHER_ERROR			0x02
#define BLOCK_NO_ERROR_SEND_ATR 	0x03
#define BLOCK_FRAME_HEADER_ERROR 	0x04
#define BLOCK_CLA_NOT_SUPPORT    	0x05
#define BLOCK_LENGTH_ERROR      	0x06
#define RESET_SEND_ATR              0x07

#define IBLOCK_PCB_COMMAND_BLOCK           0x55
#define RBLOCK_PCB_COMMAND_BLOCK           0x99
#define SBLOCK_PCB_COMMAND_BLOCK           0x33

#define HEADER_PCB_OFFSET           0
#define HEADER_CLA_OFFSET           1
#define HEADER_INS_OFFSET           2
#define HEADER_P1_OFFSET            3
#define HEADER_P2_OFFSET            4
#define HEADER_LC1_OFFSET           5
#define HEADER_LC2_OFFSET			6
#define HEADER_PAY_OFFSET			7

#define HEADER_LE_OFFSET            5
#define HEADER_LC_OFFSET            6

#define IO_COMMAND_HEADER_LEN       4


#define IBLOCK_HEADER_LENTH         7 
#define LRC_LENTH                   1

#define TYPE_RECEIVE_DATA_TYPE_APDU		0
#define TYPE_ERROR_RETRANSMIT_MAX		1
#define TYPE_REQUEST_RETRANSMISSION	    2
#define TYPE_WARMRESET					3
#define TYPE_FRAME_HEADER_ERROR	        4
#define TYPE_EMPTY_FRAME                5
#define TYPE_CLA_NOT_SUPPORT	        6
#define TYPE_ERROR_HAPPEN               7

#define S_BLOCK_BUFFER_SIZE      (9)

#if  PROTOCOL_EXTEND_LENGTH
#define I_BLOCK_BUFFER_SIZE      (2060+512)
#endif

#if  !PROTOCOL_EXTEND_LENGTH
#define I_BLOCK_BUFFER_SIZE      (270)
#endif

extern PROTOCOL_STATUS  protocol_status[];


extern void ecpIoProtocol_Init(u8 protocol_id, pSendToIO pFunc, u8* IblockBuf, u8* SblockBuf);
extern u8   ecpIoProtocol_GetApduLe(u8 protocol_id);
extern u8 	ecpIoProtocol_DecodeBlock( u8* prev, u8* apduBuf, u16 revLen ,u8 protocol_id);
extern void ecpIoProtocol_TxStateMachine(u8 errorStatus, u16 sw, u16 txDataLen, u8 protocol_id);

#endif

