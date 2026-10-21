/*
 * TLP224.c
 *
 *  Created on: 2014Äê2ÔÂ19ÈÕ
 *      Author: mowenpan
 */

#include    <errno.h>
#include "TLP224.h"
#include    <time.h>
#include    <string.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>
#include 	"ecpDefine.h"

// for clock(), This include will need to be ctime.h for Visual C++

#if DEBUG_VERBOSE
#define servertrace(x) if (debug_native_classes) fprintf(stdout, "\nserver: %s \n", x);
#endif /* DEBUG_VERBOSE */

#define serverString2   "setsockopt return %i\n"


/*
 * globals
 */
TLP_MSG g_msg;

unsigned short port_number = JC_DEFAULT_PORT_NUMBER;

#if DEBUG_VERBOSE
int socket_debug = 0;
#endif

#ifdef WIN32
SOCKET fd_socket[1];
unsigned long nonblock = 0;
#else
int fd_socket[1];
#endif /* WIN32 */

int fd_primary = 0;
int fd_secondary = 1;

int first_byte = false;

boolean tlp_createSockets(u16 dport)
{

	if (tlp_createSocket(fd_primary, dport) == false)
	{
		return false;
	}
	return true;
}

/**
 * initialize the socket
 */
boolean tlp_createSocket(int sockid, u_short portnum)
{
	struct sockaddr_in serv_addr;
	int val;

#ifdef WIN32
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0)
	{
#if DEBUG_VERBOSE
		fprintf(stderr, serverInit, portnum);
#endif
		return false;
	}
#endif
#if DEBUG_VERBOSE
	servertrace("createSocket()");
#endif

	/*
	 * open socket
	 */
#ifdef WIN32
	if ((fd_socket[sockid] = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
#else
	if ((fd_socket[sockid] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif
	{
#if DEBUG_VERBOSE
		servertrace(serverString1);
		fprintf(stderr, serverOpen, portnum);
#endif
		return false;
	}
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portnum);
	val = 1;

#ifdef WIN32
	if (ioctlsocket(fd_socket[sockid], FIONBIO, &nonblock))
	{
		fprintf(stderr, serverString2, errno);
	}
	if (bind(fd_socket[sockid], (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) == SOCKET_ERROR)
#else
	if (setsockopt(fd_socket[sockid], SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val)) < 0)
	fprintf(stderr, serverString2, errno);
	if (bind(fd_socket[sockid], (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
#endif
	{
#if DEBUG_VERBOSE
		servertrace(serverString3);
		fprintf(stderr, serverBind, portnum);
#endif
		return false;
	}

	/*
	 * listen...
	 */
	listen(fd_socket[sockid], 5);

	/*
	 * initialize the global TLP message structure
	 */
	tlp_initMSG(fd_socket[sockid], sockid);

	return true; /* success */
}

/**
 * close the socket
 */
void tlp_closeConnection()
{
	clock_t clkDelay;
	clkDelay = clock();
	while (clkDelay < (CLOCKS_PER_SEC / 58))
	{
		clkDelay = clock();
	}
#if DEBUG_VERBOSE
	servertrace("closeConnection()");
#endif

#ifdef WIN32
	shutdown(fd_socket[fd_primary], SD_BOTH);
	closesocket(fd_socket[fd_primary]);
#else
	shutdown(fd_socket[fd_primary], 2);
	close(fd_socket[fd_primary]);
#endif
}

void tlp_prinBytes(PX8 buf, u32 len)
{
	int i;
	fprintf(stderr, " Header: (%d):", len);
	for (i = 0; i < len; i++)
	{
		fprintf(stderr, " %2x", buf[i] & 0xff);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}


/**
 * initialize the global TLP message structure
 */
void tlp_initMSG(int fd, int sockid)
{
#if DEBUG_VERBOSE
	servertrace("initMSG()");
#endif

	g_msg.fd = fd;
	g_msg.ioFlag = TLP_UNKNOWN;
	g_msg.ioState = TLP_STATE_CLOSED;
	g_msg.ioOffset = 0;

}

/**
 * reset the global TLP message structure
 */
void tlp_resetMSG(TLP_MSG *msg)
{
#if DEBUG_VERBOSE
	servertrace("resetMSG()");
#endif
	msg->ioFlag = TLP_UNKNOWN;
	msg->ioOffset = 0;
}

/**
 * calculate LRC
 */
int tlp_computeLRC(char *buf, int length)
{
	int lrc = 0;
	int i;
	for (i = 0; i < length; i++)
	{
		lrc ^= buf[i];
	}
	return (lrc & 0xff);
}

/**
 * write one byte to the socket
 */
boolean tlp_socket_writeByte(int fd, char c)
{
	int n;

#ifdef WIN32
	if ((n = send(fd, &c, 1, 0)) != 1)
#else
	if ((n = write(fd, &c, 1)) != 1)
#endif
	{
#if DEBUG_DEV
#ifdef WIN32
		n = WSAGetLastError();
#endif
#if DEBUG_VERBOSE
		servertrace(serverString5);
		//fprintf(stderr, "\nserver: serverWriteByte\n");
		fprintf(stderr, serverWriteByte, port_number);
#endif
#endif
		return false;
	}
	return true;
}

/**
 * read one byte from the socket
 */
int tlp_socket_readByte(int fd)
{
	int n;
	char c = -1;

#ifdef WIN32
	if ((n = recv(fd, &c, 1, 0)) != 1)
#else
	if ((n = read(fd, &c, 1)) != 1)
#endif
	{
#if DEBUG_DEV
#ifdef WIN32
		n = WSAGetLastError();
#endif
#if DEBUG_VERBOSE
		servertrace(serverString6);
		fprintf(stderr, serverReadByte, port_number);
#endif
#endif
		return (-1);
	}

	// if not T=0 protocol, quit CREF
	if (first_byte == false)
	{
		if ((c & 0x00ff) != (T0_FIRST_BYTE & 0x00FF))
		{
#if CJCRE
			fprintf(stdout, comm_mismatch);
#endif
			exit(0);
		}
		first_byte = true;
	}

	return ((int) (c & 0xff));
}

/**
 * put n bytes to the message buffer
 */
void tlp_putNBytes(TLP_MSG *msg, unsigned char *data, int offset, int length)
{
	memcpy(&(msg->buf[msg->ioOffset]), &(data[offset]), length);
	msg->ioOffset += length;
	msg->len = msg->ioOffset;
}

/**
 * get n bytes from the buffer
 */
int tlp_getNBytes(TLP_MSG *msg, unsigned char *data, int offset, int length)
{
	int bytes_available = msg->len - msg->ioOffset - 1;
	int count;

#if DEBUG_DEV
#if DEBUG_VERBOSE
	/*    if (msg->len > 200) {
	 fprintf(stdout, serverString7, length, msg->len, msg->ioOffset, bytes_available);
	 } */
#endif
#endif

	if (length > bytes_available)
	{
		count = bytes_available;
	}
	else
	{
		count = length;
	}
	memcpy(&(data[offset]), &(msg->buf[msg->ioOffset]), count);
	msg->ioOffset += count;
	return count;
}

/**
 * print the TLP message in ASCII (for debugging)
 */
void tlp_printMSG(TLP_MSG *msg)
{
	int i;
	fprintf(stderr, "(%d):", msg->len);
	for (i = 0; i < msg->len; i++)
	{
		fprintf(stderr, "%02x", msg->buf[i] & 0xff);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}

/**
 * print the TLP message in ASCII (for debugging)
 */
void tlp_print(int len,char* buf)
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


/**
 * Receive a TLP224 formatted message from the input stream.
 * This method reads bytes from the input stream until an EOT (0x03) character is
 * received. The resulting message is decoded and stored in the TL224Message msg.
 * In the event of a transmission error, this method will send attempt error recovery
 * by sending a TLP224 NACK message to the sender. Up to 5 retries will be performed.
 * @param msg The object to store the received message in.
 * @exception TLP224Exception If an unrecoverable transmission error occurs.
 * @exception IOException If an error occurs while reading from the input stream.
 */
boolean tlp_receiveMessage(TLP_MSG *msg)
{
    int tries = 0;
    while (1)
    {
        int messageTooLong = 0;
        int xmitError = 0;
        int got = 0;

        //
        // Only retry link level errrors 5 times before giving up.
        //
        if (tries++ > 5)
		{
			/* Retry error -- Giving up! */
			return(false);
		}

        //
        // loop reading characters until EOT is received.
        //
        while (1)
		{
            //
            // first byte is the high nibble
            //
            int hiNibble = tlp_socket_readByte(msg->fd);
            int lowNibble;
            if (hiNibble == (int) TLP_EOT)
			{
                break;
			}
	    /*            else if (hiNibble == SOCKET_ERROR)*/
	      else if (hiNibble == -1)
			{
				/* Receive errors -- SOCKET ERROR */
				return(false);
			}
            hiNibble -= 0x30;
            if (hiNibble > 9)
			{
                hiNibble -= 7;
			}
            if (hiNibble < 0 || hiNibble > 0xf)
			{
                xmitError = 1;
			}

            //
            // second is the low nibble
            //
            lowNibble = tlp_socket_readByte(msg->fd);
            if (lowNibble == (int) TLP_EOT)
			{
                xmitError = 1;
                break;
            }
	    /* else if (lowNibble == SOCKET_ERROR)*/
	    else if (lowNibble == -1)
			{
				/* Receive errors -- SOCKET ERROR */
				return(false);
			}
            lowNibble -= 0x30;
            if (lowNibble > 9)
			{
                lowNibble -= 7;
			}
            if (lowNibble < 0 || lowNibble > 0xf)
			{
                xmitError = 1;
			}

            if (messageTooLong || xmitError)
			{
                continue;
			}

            msg->buf[got++] = (char) ((hiNibble << 4) | lowNibble);
        }
//        int end = 1;
//        while(end>0){
//        	end = tlp_socket_readByte(msg->fd);
//        }

        if (xmitError)
		{
            if (!tlp_transmissionError())
				return(false);
            continue;
        }
		else if (messageTooLong)
		{
            if (!tlp_statusResponse(msg, STATUS_MESSAGE_TOO_LONG))
				return(false);
            continue;
        }

        msg->len = got;

        //
        // The message must contain a valid LRC
        //
        if (msg->buf[got-1] != (char) tlp_computeLRC(msg->buf, got-1))
		{
            if (!tlp_transmissionError())
				return(false);
	    continue;
        }

        //
        // The second byte of the message is the command length. The total message length
        // includes the ACK/NACK, the length and the LRC
        //
        if ((u16)((msg->buf[1]<<8 ) + (msg->buf[2]&0xff))!= (u16)(got - 4))
		{
            if (!tlp_transmissionError())
				return(false);
            continue;
        }

        //
        // The first byte of the message must be either an ACK or a NACK
        //
        if ((msg->buf[0] != (char) TLP_ACK) && (msg->buf[0] != (char) TLP_NACK))
		{
            if (!tlp_statusResponse(msg, STATUS_PROTOCOL_ERROR))
				return(false);
            continue;
        }
        break;
    }
#if DEBUG_VERBOSE
    if (socket_debug)
    {
        DEBUG_MSG(io_cadString1);
        printMSG(msg);
    }
#endif
    return(true);	/* No errors! */
}


/**
 * Format a TLP224Message and send to the output stream.
 * This method formats a TLP224Message into it's ASCII representation and
 * sends the message on the output stream. After the message is completed, an
 * ASCII EOT character is then sent to the output stream.
 */
boolean tlp_sendMessage(TLP_MSG *msg)
{
    int i;

    for (i = 0; i < msg->len; i++)
    {
        int nibble = msg->buf[i] >> 4 & 0xf;
        if (nibble < 10)
		{
            nibble += 0x30;     // add ASCII '0'
		}
        else
		{
            nibble += 0x37;     // add ASCII 'A'
		}
        if (!tlp_socket_writeByte(msg->fd, (char)(nibble & 0xff)))
			return(false);

        nibble = msg->buf[i] & 0xf;
        if (nibble < 10)
		{
            nibble += 0x30;     // add ASCII '0'
		}
        else
		{
            nibble += 0x37;     // add ASCII 'A'
		}
        if (!tlp_socket_writeByte(msg->fd, (char)(nibble & 0xff)))
			return(false);
    }
    if (!tlp_socket_writeByte(msg->fd, TLP_EOT))
		return(false);
#if DEBUG_VERBOSE
    if (socket_debug)
    {
        DEBUG_MSG(io_cadString2);
        printMSG(msg);
    }
#endif
    tlp_resetMSG(msg);
    return(true);
}

/**
 * Send a one byte TLP224 status response
 */
boolean tlp_statusResponse(TLP_MSG *msg, char code)
{
    msg->buf[0] = (char) TLP_ACK;
    msg->buf[1] = (char) 0;
    msg->buf[2] = (char) 1;
    msg->buf[3] = STATUS_SUCCESS;
    msg->buf[4] = (char) tlp_computeLRC(msg->buf, 4);
    msg->len = 5;
    if (!tlp_sendMessage(msg))
    {
		return(false);
    }
    else
    {
		return(true);
    }
}

/**
 * Send a TLP224 Transmission Error response
 */
boolean tlp_transmissionError()
{
    g_msg.buf[0] = (char) TLP_NACK;
    g_msg.buf[1] = (char) 0;
    g_msg.buf[2] = (char) 0;
    g_msg.buf[3] = (char) tlp_computeLRC(g_msg.buf, 3);
    g_msg.len = 4;
    if (!tlp_sendMessage(&g_msg))
		return(false);
    else
		return(true);
}

