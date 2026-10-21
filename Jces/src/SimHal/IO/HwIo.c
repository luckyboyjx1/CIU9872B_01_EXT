/*
 * HwIo.c
 *
 *  Created on: 2015Äê10ÔÂ28ÈÕ
 *      Author: mowenpan
 */

#include "HwIo.h"
#include <winsock2.h>
#include <stdio.h>
#include "hal.h"



#define CMD_TYPE_ATR_RESET	0
#define CMD_TYPE_ATR		1
#define CMD_TYPE_TRANSMIT	2


#define PORT 29500
#define EVENT_PORT 29501
#define IP_ADDRESS "127.0.0.1"

static SOCKET apduSocket;
static SOCKET eventSocket;


u32 sendId = 0;
u8 ioflag = 0;


/**
 * In 60 seconds if the sendId equals lastSendId,it minds that nothing send out during this time
 */
void *sendMonitor(void *arg)
{
	sendId = 0;
	u32 lastSendId;
	while(true){
		lastSendId = sendId;
		Sleep(60000);
		if((lastSendId == sendId)  && (ioflag == 1)){
			exit(-1);
		}
	}
}

static boolean createSocket(SOCKET *pSocket, const char* ipAddress, u_short port)
{
	WSADATA Ws;
	struct sockaddr_in ServerAddr;
	int Ret = 0;

	//Init Windows Socket
	if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
	{
		fprintf(stderr, "Init Windows Socket Failed: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
	    return false;
	}
	//Create Socket
	*pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( *pSocket == INVALID_SOCKET )
	{
		fprintf(stderr, "Create Socket Failed: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
	    return false;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(ipAddress);
	ServerAddr.sin_port = htons(port);
	memset(ServerAddr.sin_zero, 0x00, 8);

	Ret = connect(*pSocket,(struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if ( Ret == SOCKET_ERROR )
	{
		fprintf(stderr, "Connect Error: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
	    return false;
	}
	else
	{
		return true;
	}
}

static u32 readInt()
{
	char buf[4];

	if (recv(apduSocket, buf, 4, 0) == SOCKET_ERROR)
	{
		fprintf(stderr, "Recv Error: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
		exit(-1);
	}

	return *(PX32)buf;
}

static void writeInt(u32 value)
{
	char buf[4];
	*(PX32)buf = value;

	if (send(apduSocket, buf, 4, 0) == SOCKET_ERROR)
	{
		fprintf(stderr, "Send Error: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
		exit(-1);
	}
}

static void rxBuffer(PX8 pData, u16 length)
{
	if (recv(apduSocket, (char *)pData, length, 0) == SOCKET_ERROR)
	{
		fprintf(stderr, "Recv Error: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
		exit(-1);
	}

	printData(length,pData);
}


void HwIo_Init(const char* ipAddress, u16 port, u16 eport)
{
	if (false == createSocket(&apduSocket, ipAddress, port))
	{
		exit(-1);
	}

	if (false == createSocket(&eventSocket, ipAddress, eport))
	{
		exit(-1);
	}
}


void HwIo_Rx(PHwIo_Buffer buffer)
{
	u32 type;
	u32 dataLen;
	//waiting for data and not stop.
	ioflag = 0;
	type = readInt();

	switch (type)
	{
	case CMD_TYPE_ATR_RESET:
		fprintf(stderr, "ATR RESET.\n");
		fflush(stderr);
		hw_gotoResetHandler();
		break;
	case CMD_TYPE_ATR:
		fprintf(stderr, "ATR.\n");
		fflush(stderr);
		hw_gotoResetHandler();
		break;
	case CMD_TYPE_TRANSMIT:
		dataLen = readInt();
		buffer->len = dataLen;
		rxBuffer(buffer->data, dataLen);
		break;
	default:
		fprintf(stderr, "Recv Error.\n");
		fflush(stderr);
		exit(-1);
		break;
	}

	ioflag = 1;
}

void HwIo_Tx(PHwIo_Buffer buffer)
{
	sendId++;
	writeInt(buffer->len);
	printData(buffer->len,buffer->data);
	if (send(apduSocket, (char *)buffer->data, buffer->len, 0) == SOCKET_ERROR)
	{
		fprintf(stderr, "Send Error: 0x%x\n", (unsigned int)GetLastError());
		fflush(stderr);
		exit(-1);
	}
}


/**
 * print the TLP message in ASCII (for debugging)
 */
void printData(u16 len,u8* buf)
{
	int i;
	fprintf(stderr, "(%d):", len);
	for (i = 0; i < len; i++)
	{
		fprintf(stderr, "%02x", buf[i] & 0xff);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}
