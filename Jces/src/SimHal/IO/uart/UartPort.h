/*
 * UartPort.h
 *
 *  Created on: 2014年2月19日
 *      Author: mowenpan
 */

#ifndef UARTPORT_H_
#define UARTPORT_H_

#include "HwIo.h"
#include "TLP224.h"

#define UARTPORT_BUFFER_SIZE                   265

#define UARTPORT_CODE_POWER_UP                0x11 //上电
#define UARTPORT_CODE_RESET                   0x12 //复会
#define UARTPORT_CODE_POWER_DOWN              0x13 //下电
#define UARTPORT_CODE_RECEIVED_APDU           0x21 //接收 APDU
#define UARTPORT_CODE_SEND_APDU               0x22 //发送 APDU

#define UARTPORT_DIRECT_INCOMING              0x10 //接收
#define UARTPORT_DIRECT_OUTGOING              0x20 //发送


#define RECEIVE_COMMAND										0x11
#define RECEIVE_DATA										0x12
#define SEND_PRC_BYTE										0x60
#define SEND_ACK_BYTE										0x66
#define SEND_ATR                    						0x20
#define SEND_DATA											0x21
#define SEND_STATUS											0x22
#define SEND_60												0x23
#define SEND_PPS											0x24

/** 端口数据结构  **/
typedef struct
{
	u8 code; //操作码
	u8 direction; //表示 端口处理于 接收数据还是发送数据
	u16 offset;
	HwIo_Buffer buffer;
} uartPort;
extern uartPort hl_uartPort;

/** 端口操作方法  **/
void uartPort_txByte(u8 txByte);
u8 uartPort_rxByte(void);

void uartPort_setReceiveMode(void);
void uartPort_setTransmitMode(void);

void uartPort_rxBytes(PX8 buf, u16 len, u8 flag);

void uartPort_txBytes(PX8 buf, u16 len, u8 flag);

void uartPort_init();


#endif /* UARTPORT_H_ */
