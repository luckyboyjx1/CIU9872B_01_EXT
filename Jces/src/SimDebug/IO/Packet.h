/*
 * Packet.h
 *
 *  Created on: 2015Äê3ÔÂ6ÈÕ
 *      Author: mowenpan
 */

#ifndef HW_IO_PACKET_H_
#define HW_IO_PACKET_H_

#include <cardAPI.h>
#include <winsock.h>
#include "pthread.h"

extern u32 g_packetCommandId;

typedef struct {
	u32 commandId;
	u8 flgs;
	u16 funcId;
	u32 dataLen;
	PX8 data;
} Packet;

typedef struct
{
	//Packet commandPackets[10];
	//u8 commandCount;
	Packet replyPackets[10];
	u8 replyCount;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
} Packet_RecvData;

typedef struct
{
	Packet sendPackets[10];
	u8 count;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
} Packet_SendData;

extern Packet_RecvData g_packetRecvData;
extern Packet_SendData g_packetSendData;
extern SOCKET g_javaDebugSocket;

#define JAVA_DEBUG_VERBOSE                                      1

#define PACKET_SZIE		sizeof(Packet)
#define PACKET_FLGS_REPLY		0x80
#define PACKET_FLGS_COMMAND		0x00

#define PACKET_ERRORCODE_MASK	0x80000000

#define PACKET_HEADER_SIZE		11

#define PACKET_LENGTH_OFFSET       	0
#define PACKET_ID_OFFSET           	4
#define PACKET_FLAG_OFFSET         	8
//#define PACKET_COMMAND_SET_OFFSET  	9
#define PACKET_FUNCID_OFFSET      	9
#define PACKET_ERRORCODE_OFFSET   	9

#define PACKET_DATA_OFFSET   		11

#define charArrayToInt(charArray, offset)      (((charArray[offset + 0] & 0xFF) << 24) | ((charArray[offset + 1] & 0xFF) << 16) | ((charArray[offset + 2] & 0xFF) << 8) | (charArray[offset + 3] & 0xFF))

extern void Packet_addPacketToSend(Packet *packet);


extern void *Packet_recvRun(void *arg);
extern void *Packet_sendRun(void *arg);
extern u32 Packet_getCommandId();
#endif /* HW_IO_PACKET_H_ */
