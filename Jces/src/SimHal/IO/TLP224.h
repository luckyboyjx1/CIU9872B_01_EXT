/*
 * TLP224.h
 *
 *  Created on: 2014Äê2ÔÂ19ÈÕ
 *      Author: mowenpan
 */

#ifndef TLP224_H_
#define TLP224_H_
#include "ecpDefine.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#if DEBUG_VERBOSE
extern int socket_debug;
#endif

extern unsigned short port_number;

#define JC_DEFAULT_PORT_NUMBER          9025
#define JC_TCP_WELLKNOWN_PORT           1024

#define T0_FIRST_BYTE                   0x36
#define T1_FIRST_BYTE                   0xF0

#define	RESP_INS_SUCCESSFUL			0x9000

#define MAXLINE						512
#define ATR_LEN						6

#define TLP_UNKNOWN				-1
#define TLP_INPUT				0
#define TLP_OUTPUT				1

#define TLP_STATE_UNKNOWN		-1
#define TLP_STATE_CLOSED		0
#define TLP_STATE_OPEN			1
#define TLP_STATE_ACTIVE		2

#define TLP_EOT					0x03
#define TLP_ACK					0x60
#define TLP_NACK				0xe0

#define POWER_DOWN				0x4d
#define POWER_UP				0x6e

#define ISO_INPUT				0xda
#define ISO_OUTPUT				0xdb
#define POWER_RESET  	        0x80
#define POWER_PPS    	        0x81
#define RESET_CODE              POWER_RESET
#define OFFSET_CLA_				4
#define OFFSET_INS_				5
#define OFFSET_P1_				6
#define OFFSET_P2_				7
#define OFFSET_P3_				8
#define OFFSET_DATA_IN			9

#define STATUS_SUCCESS					0x00
#define STATUS_INCORRECT_NUMBER_OF_ARGS	0x03
#define STATUS_COMMAND_UNKNOWN			0x04
#define STATUS_PROTOCOL_ERROR			0x09
#define STATUS_ISO_CMD_ERROR			0x11
#define STATUS_MESSAGE_TOO_LONG			0x12
#define STATUS_CARD_TURNED_OFF			0x15
#define STATUS_UNKNOWN_PROTOCOL			0x17
#define STATUS_ISO_LC_ERROR				0x1A
#define STATUS_CARD_PROTOCOL_ERROR		0xA1
#define STATUS_CARD_MALFUNCTION			0xA2
#define STATUS_CARD_ABORTED_CHAINING	0xA4
#define STATUS_READER_ABORTED_CHAINING	0xA5
#define STATUS_INVALID_PROCEDURE_BYTE	0xE4
#define STATUS_INTERRUPTED_EXCHANGE		0xE5
#define STATUS_CARD_ERROR				0xE7
#define STATUS_CARD_REMOVED				0xF7
#define STATUS_CARD_MISSING				0xFB

#define MAX_MESSAGE_LEN					258

typedef struct data {
    int ioFlag;
    int len;
    int fd;
    int ioState;
    int ioOffset;
    char buf[MAXLINE];
} TLP_MSG;

extern SOCKET fd_socket[1];
extern TLP_MSG g_msg;
extern int fd_primary;
/**
 * initialize all sockets
 */
boolean tlp_createSockets(u16 dport);

/**
 * initialize the socket
 */
boolean tlp_createSocket(int sockid, u_short portnum);

/**
 * establish a socket connection
 */
boolean tlp_getConnection(TLP_MSG *msg);

/**
 * close the socket
 */
void tlp_closeConnection();

/**
 * initialize the global TlP messge structure
 */
void tlp_initMSG(int fd, int sockid);

/**
 * reset the global TlP messge structure
 */
extern void tlp_resetMSG(TLP_MSG *msg);

/**
 * calculate LRC
 */
int tlp_computeLRC(char *buf, int length);

/**
 * write one byte to the socket
 */
boolean tlp_socket_writeByte(int fd, char c);

/**
 * read one byte from the socket
 */
int tlp_socket_readByte(int fd);

/**
 * put n bytes to the message buffer
 */
void tlp_putNBytes(TLP_MSG *msg, unsigned char *data, int offset, int length);

/**
 * get n bytes from the buffer
 */
int tlp_getNBytes(TLP_MSG *msg, unsigned char *data, int offset, int length);

/**
 * centralized exit function in the event of an error
 */
void tlp_quit(int status);

/**
 * print the TLP message in ascii (for debugging)
 */
void tlp_printMSG(TLP_MSG *msg);
void tlp_print(int len,char* buf);

boolean tlp_receiveMessage(TLP_MSG *msg);
boolean tlp_sendMessage(TLP_MSG *msg);

boolean tlp_statusResponse(TLP_MSG *msg, char code);
boolean tlp_transmissionError();

#endif /* TLP224_H_ */
