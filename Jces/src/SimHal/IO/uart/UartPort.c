/*
 * UartPort.c
 *
 *  Created on: 2014年2月19日
 *      Author: mowenpan
 */

#include "TLP224.h"
#include "UartPort.h"
#include "ecpDefine.h"
#include "hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


uartPort hl_uartPort;
static boolean isRecv;

void uartPort_init()
{
	//hl_uartPort.code
	hl_uartPort.offset = 0;
	hl_uartPort.buffer.len = 0;
	hl_uartPort.direction = UARTPORT_DIRECT_INCOMING;
}

void uartPort_reset()
{
	hl_uartPort.offset = 0;
	hl_uartPort.buffer.len = 0;
	memset(hl_uartPort.buffer.data, 0, HWIO_BUFFER_MAX_SIZE);
}


static void _toUartBuffer(PX8 buf, u16 len)
{
	memcpy(&hl_uartPort.buffer.data[hl_uartPort.buffer.len], buf, len);
	hl_uartPort.buffer.len += len;
}

/*
 *
#define RECEIVE_COMMAND				1
#define SEND_PRC_BYTE				2
#define SEND_DATA					3
#define SEND_STATUS					4
#define RECEIVE_DATA				5
 */
void uartPort_txBytes(PX8 buf, u16 len, u8 flag)
{
	if (SEND_ATR == flag)
	{
		memmove(&buf[1], &buf[0], len);
		buf[0] = gAtrBuf[0];
		len ++ ;
	}
	if ((SEND_PRC_BYTE == flag) || (SEND_ACK_BYTE == flag))
	{
		return;
	}

	if (hl_uartPort.direction != UARTPORT_DIRECT_OUTGOING)
	{
		uartPort_reset();
		hl_uartPort.direction = UARTPORT_DIRECT_OUTGOING;
	}

	/*
	if (flag == SEND_ATR)
	{
		u8 atrPads[3] = {0x28, 0x02};
		atrPads[2] = len;
		_toUartBuffer(atrPads, 3);
	}
	 */
	_toUartBuffer(buf, len);

	if (flag != SEND_DATA)
	{
		/*
		g_msg.buf[0] = (char) TLP_ACK;
		g_msg.buf[1] = (char) ((hl_uartPort.offset + 1)>>8);
		g_msg.buf[2] = (char) (hl_uartPort.offset + 1)&0xff;
		g_msg.buf[3] = STATUS_SUCCESS;
		memcpy(&g_msg.buf[4], hl_uartPort.buffer, hl_uartPort.offset);
		g_msg.buf[hl_uartPort.offset + 4] = tlp_computeLRC(g_msg.buf, hl_uartPort.offset + 4);
		g_msg.len = hl_uartPort.offset + 5;

		if (!tlp_sendMessage(&g_msg))
		{
			fprintf(stderr, "send data error.");
			fflush(stderr);
			//while (true); //TODO jces: 处理异常
		}
		*/
		if (true == isRecv)
		{
			HwIo_Tx(&hl_uartPort.buffer);
		}
	}
}

void uartPort_rxBytes(PX8 buf, u16 len, u8 flag)
{
	isRecv = true;

	if (RECEIVE_COMMAND == flag && hl_uartPort.direction != UARTPORT_DIRECT_INCOMING)
	{
		uartPort_reset();
		HwIo_Rx(&hl_uartPort.buffer);
		hl_uartPort.direction = UARTPORT_DIRECT_INCOMING;

#if 0
		if (!tlp_receiveMessage(&g_msg))
		{
			fprintf(stderr, "send data error.");
			fflush(stderr);
			return ;
			//while (true); //TODO jces: 处理异常
		}
		hl_uartPort.direction = UARTPORT_DIRECT_INCOMING;
		tlp_printMSG(&g_msg);

		uartPort_reset();
		hl_uartPort.code = g_msg.buf[3] & 0xFF;
		switch (hl_uartPort.code)
		{
		case POWER_RESET:
			hw_gotoResetHandler();
			break;
		case POWER_PPS:
			//TODO: pps
			break;
		case POWER_UP:
			if ( (u16)((u8)(g_msg.buf[1] <<8 ) + (u8)(g_msg.buf[2] & 0xff) ) != (u16) 4)
			{
				if (!tlp_statusResponse(&g_msg, STATUS_INCORRECT_NUMBER_OF_ARGS))
				{
					return ;
				}
			}
			else
			{
				if (!tlp_statusResponse(&g_msg, STATUS_SUCCESS))
				{
					return ;
				}
			}
			break;

		case POWER_DOWN:
			if (!tlp_statusResponse(&g_msg, STATUS_SUCCESS))
			{
				return;
			}

			/*
			 * change state from active to closed
			 */
			tlp_closeConnection();
			g_msg.ioState = TLP_STATE_CLOSED;
			exit(EXIT_SUCCESS);
			break;

		case ISO_INPUT:
			/*
			 * INS cannot be 0x6x or 0x9x
			 */
			if ((g_msg.buf[OFFSET_INS_] & 0xff) == 0x60
					|| (g_msg.buf[OFFSET_INS_] & 0xff) == 0x61)
			{
				tlp_statusResponse(&g_msg, STATUS_ISO_CMD_ERROR);
				return;
			}

			/*
			 * P3 must match length of received data
			 */
			if ((u8) g_msg.buf[OFFSET_P3_] != (u8) (g_msg.len - 10))
			{
				tlp_statusResponse(&g_msg, STATUS_ISO_LC_ERROR); //6cxx
				return ;
			}

			memcpy(hl_uartPort.buffer, &(g_msg.buf[OFFSET_CLA_]), 5);
			if ((u8) g_msg.buf[OFFSET_P3_] > 0)
			{
				memcpy(&hl_uartPort.buffer[5], &(g_msg.buf[OFFSET_CLA_ + 5]), (u8)g_msg.buf[OFFSET_P3_]);
				g_msg.ioFlag = TLP_INPUT;
				g_msg.ioOffset = OFFSET_DATA_IN;
			}
			break;

		case ISO_OUTPUT:
			/*
			 * INS cannot be 0x6x or 0x9x
			 */
			if ((g_msg.buf[OFFSET_INS_] & 0xff) == 0x60
					|| (g_msg.buf[OFFSET_INS_] & 0xff) == 0x61)
			{
				tlp_statusResponse(&g_msg, STATUS_ISO_CMD_ERROR);
				return ;
			}
			/*
			 * We must have a 5 byte command APDU (plus TLP224 Header and LRC)
			 */
			if (g_msg.len != 10)
			{
				tlp_statusResponse(&g_msg, STATUS_INCORRECT_NUMBER_OF_ARGS);
				return ;
			}

			/* This seems a redundant line - is it really necessary? */
			memcpy(hl_uartPort.buffer, &(g_msg.buf[OFFSET_CLA_]), 5);
			hl_uartPort.length = 5;
			hl_uartPort.offset = 0;
			g_msg.ioFlag = TLP_OUTPUT;
			g_msg.ioOffset = OFFSET_CLA_;
			g_msg.len = OFFSET_CLA_;

			break;

		default:
			tlp_statusResponse(&g_msg, STATUS_COMMAND_UNKNOWN);
			return;
			break;
		}
#endif
	}

	memcpy(buf, &hl_uartPort.buffer.data[hl_uartPort.offset], len);
	hl_uartPort.offset += len;
}
