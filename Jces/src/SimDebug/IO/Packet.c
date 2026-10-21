/*
 * Packet.c
 *
 *  Created on: 2015年3月6日
 *      Author: mowenpan
 */
#include "Packet.h"

#include <winsock.h>
#include <winsock2.h>
#include <stdio.h>

#include "JavaDebug.h"
#include "pthread.h"

Packet_RecvData g_packetRecvData;
Packet_SendData g_packetSendData;
u32 g_packetCommandId;

SOCKET g_javaDebugSocket;

static int readBytes(int fd, char *buf, int len)
{
	int n;
#ifdef WIN32
	if ((n = recv(fd, buf, len, 0)) == SOCKET_ERROR)
#else
	if ((n = read(fd, buf, len)) == SOCKET_ERROR)
#endif
	{
#if DEBUG_DEV
#ifdef WIN32
		n = WSAGetLastError();
#endif
#if JAVA_DEBUG_VERBOSE
		jdwp_servertrace(serverString6);
		fprintf(stderr, serverReadByte, JDWP_port_number);
#endif
#endif
		return (-1);
	}

	return (n);
}

static boolean writeBytes(int fd, char * chs, int len)
{
	int n;

#ifdef WIN32
	if ((n = send(fd, chs, len, 0)) == SOCKET_ERROR)
#else
	if ((n = write(fd, &chs, len)) == SOCKET_ERROR)
#endif
	{
#if DEBUG_DEV
#ifdef WIN32
		n = WSAGetLastError();
#endif
#if JAVA_DEBUG_VERBOSE
		jdwp_servertrace(serverString5);
		fprintf(stderr, serverWriteByte, JDWP_port_number);
#endif
#endif
		return false;
	}
	return true;
}

static void printCharArray(char *pch, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (i != 0)
		{
			fprintf(stderr, " ");
		}

		if ((pch[i] & 0xFF) > 0xF)
		{
			fprintf(stderr, "%2x", pch[i] & 0xff);
		}
		else
		{
			fprintf(stderr, "0%x", pch[i] & 0xff);
		}

	}
	fprintf(stderr, "\n");
}

static void printPacket(Packet *packet)
{
	char buf[11];

	if (packet->flgs == PACKET_FLGS_REPLY)
	{
		fprintf(stderr, "\nReply Packet:\n");
	}
	else
	{
		fprintf(stderr, "\n\n************************************\n");
		fprintf(stderr, "Debug Command Packet:\n");
	}

	write_u32(&buf[PACKET_LENGTH_OFFSET], packet->dataLen + PACKET_HEADER_SIZE);
	write_u32(&buf[PACKET_ID_OFFSET], packet->commandId);
	buf[PACKET_FLAG_OFFSET]= packet->flgs;
	write_u16(&buf[PACKET_FUNCID_OFFSET], packet->funcId);

	printCharArray(buf, PACKET_HEADER_SIZE);

	if (packet->dataLen > 0)
	{
		printCharArray(packet->data, packet->dataLen);
	}
	fflush(stderr);
}

u32 Packet_getCommandId()
{
	return g_packetCommandId++;
}

boolean Packet_write(Packet *packet)
{
#if JAVA_DEBUG_VERBOSE
	printPacket(packet);
#endif

	u32 dataLen = PACKET_HEADER_SIZE + packet->dataLen;
	char buf[dataLen];

	write_u32(&buf[PACKET_LENGTH_OFFSET], dataLen);
	write_u32(&buf[PACKET_ID_OFFSET], packet->commandId);
	buf[PACKET_FLAG_OFFSET]= packet->flgs;
	write_u16(&buf[PACKET_FUNCID_OFFSET], packet->funcId);

	if (packet->dataLen > 0)
	{
		memcpy(&buf[PACKET_DATA_OFFSET], packet->data, packet->dataLen);
	}
	return writeBytes(g_javaDebugSocket, buf, dataLen);
}

boolean Packet_read(Packet *packet)
{
	u32 len;
	u32 dataLen;
	u32 status;
	//read command header
	char header_buf[PACKET_HEADER_SIZE];
	status = readBytes(g_javaDebugSocket, header_buf, PACKET_HEADER_SIZE);

	if (status == SOCKET_ERROR)
	{
		return false;
	}

	len = read_u32(&header_buf[PACKET_LENGTH_OFFSET]);
	packet->commandId = charArrayToInt(header_buf, PACKET_ID_OFFSET);
	packet->flgs = header_buf[PACKET_FLAG_OFFSET];
	packet->funcId = read_u16(&header_buf[PACKET_FUNCID_OFFSET]);


	//read command data
	dataLen = len - PACKET_HEADER_SIZE;
	if (dataLen <= 0)
	{
		packet->dataLen = 0;
		packet->data = NULL;
	}
	else
	{
		packet->data = MEMtU8(calloc(dataLen, sizeof(char)));
		status = readBytes(g_javaDebugSocket, packet->data, dataLen);

		if (status == SOCKET_ERROR)
		{
			return false;
		}

		packet->dataLen = dataLen;
	}

#if JAVA_DEBUG_VERBOSE
	printPacket(packet);
#endif
	return true;
}

void *Packet_recvRun(void *arg)
{
	Packet packet;
	Packet *dPacket;
	u8 replyCount;

	while (true)
	{
		//read packet
		Packet_read(&packet);

		if (packet.flgs == PACKET_FLGS_REPLY) //回复包
		{
			pthread_mutex_lock(&g_packetRecvData.mutex);

			replyCount = g_packetRecvData.replyCount;
			dPacket = &(g_packetRecvData.replyPackets[replyCount]);
			memcpy(dPacket, &packet, PACKET_SZIE);
			replyCount++;
			g_packetRecvData.replyCount = replyCount;


			pthread_cond_signal(&g_packetRecvData.cond);
			pthread_mutex_unlock(&g_packetRecvData.mutex);
		}
		else //命令包
		{
			Packet replyPacket;
			replyPacket.commandId = packet.commandId;
			replyPacket.flgs = PACKET_FLGS_REPLY;
			replyPacket.dataLen = 0;
			replyPacket.data = NULL;

			JavaDebug_callVmFunc(&packet, &replyPacket);
			Packet_addPacketToSend(&replyPacket);
		}

		//free commandPacket
		if (packet.data != NULL)
		{
			free(packet.data);
			packet.data = NULL;
		}
	}
}


boolean Packet_getCommand(Packet *packet)
{

	return false;
}

boolean Packet_getReply(Packet *packet)
{
	boolean flg;
	flg = false;
	pthread_mutex_lock(&g_packetRecvData.mutex);
	while (true)
	{
		if (g_packetRecvData.replyCount < 1)
		{
			pthread_cond_wait(&g_packetRecvData.cond, &g_packetRecvData.mutex);

		}

		if (g_packetRecvData.replyCount)
		{
			memcpy(packet, &g_packetRecvData.replyPackets[0], PACKET_SZIE);
			if (g_packetRecvData.replyCount > 1)
			{
				memcpy(&g_packetRecvData.replyPackets[0], &g_packetRecvData.replyPackets[1], (g_packetRecvData.replyCount * PACKET_SZIE));
			}
			else
			{
				memset(&g_packetRecvData.replyPackets[0], 0, PACKET_SZIE);
			}

			g_packetRecvData.replyCount--;
			flg = true;
			break;
		}
	}
	pthread_mutex_unlock(&g_packetRecvData.mutex);
	return flg;
}

void *Packet_sendRun(void *arg)
{
	while(true)
	{
		pthread_mutex_lock(&g_packetSendData.mutex); //lock

		if (g_packetSendData.count == 0)
		{
			pthread_cond_wait(&g_packetSendData.cond, &g_packetSendData.mutex); //wait
		}

		if (g_packetSendData.count > 0)
		{
			//send data
			Packet_write(&g_packetSendData.sendPackets[0]);

			//free data
			if (g_packetSendData.sendPackets[0].data != NULL)
			{
// TODO:xxxxxxxxxxxxxx
				free(g_packetSendData.sendPackets[0].data);
				g_packetSendData.sendPackets[0].data = NULL;
			}

			if (g_packetSendData.count > 1)
			{
				PX8 src = (PX8)&(g_packetSendData.sendPackets[1]);
				PX8 desc = (PX8)&(g_packetSendData.sendPackets[0]);
				u32 size = PACKET_SZIE * (g_packetSendData.count - 1);

				memcpy(desc,  src, size);
			}
			else
			{
				memset(&g_packetSendData.sendPackets[0], 0, PACKET_SZIE);
			}

			g_packetSendData.count--;
		}

		pthread_mutex_unlock(&g_packetSendData.mutex); //unlock
	}
}

static void PacketCopy(Packet *dest, Packet *src)
{
	dest->commandId = src->commandId;
	dest->data = src->data;
	dest->dataLen = src->dataLen;
	dest->flgs = src->flgs;
	dest->funcId = src->funcId;
}

/**
 *
 */
void Packet_addPacketToSend(Packet *packet)
{
	Packet *dPacket;

	pthread_mutex_lock(&g_packetSendData.mutex); //lock

	dPacket = &g_packetSendData.sendPackets[g_packetSendData.count];
	memcpy(dPacket, packet, PACKET_SZIE);

	g_packetSendData.count++;

	pthread_cond_signal(&g_packetSendData.cond);

	/*
	if (packet->data != NULL)
	{
		free(packet->data);
		packet->data = NULL;
	}
	pthread_cond_signal(&g_packetSendData.cond);
	*/
	pthread_mutex_unlock(&g_packetSendData.mutex);//unlock
}
