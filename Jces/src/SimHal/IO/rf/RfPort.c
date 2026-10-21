/*
 * RfPort.c
 *
 *  Created on: 2015Äê10ÔÂ27ÈÕ
 *      Author: mowenpan
 */
#include "HwIo.h"
#include "RfPort.h"
#include "stdio.h"
#include "ecpDefine.h"
#include "ecpOperation.h"
#include "ecpReadWrite.h"
#include "hal_callback.h"

#define MIN(a,b)    ((a)<(b)?(a):(b))
#define REQA                                (0x26 * 2)  /**< ISO14443-3 A REQA command (*2 because reception of less than 8 bits is MSB aligned)*/
#define WUPA                                (0x52 * 2)  /**< ISO14443-3 A WUPA command (*2 because reception of less than 8 bits is MSB aligned)*/

#define PCB_CHAIN_MASK						0x10
#define PCB_I_BLOCK_MASK					0x2
#define PCB_BLOCK_NUMBER_MASK				0x1

static phIso14443L3_State_t rfStatus;
static u8 blockNumber;
static RfPort_Data rfPortData;
static boolean isATS;
static boolean isRecv;

static void rfPordDataReset()
{
	rfPortData.direction = HWIO_DIRECTION_UNKNOWN;
	rfPortData.fullness = 0;
	rfPortData.buffer.len = 0;
}

void rfPort_init() {
	rfStatus = RF_ISO14443L3_IDLE_STATE;
	blockNumber = 0;

	rfPordDataReset();
	isATS = false;
}

u16	rfPort_Anticollision(PX8 pData, u16 dataSize, PX16 fullness, u8 sakComplete, u8 sakIncomplete, PR8 pCascadeLevel)
{
	rfStatus = RF_ISO14443L3_ACTIVE_STATE;
	return 0x9CC9;
}

static u16 rxCmd(PX8 pData, u16 dataSize, PR16 fullness, u16 fsc)
{
	u16 rxSize;
	u8 pcb;
	u16 remainSize;
	u8 pLen;
	u16 iLen;
	int i;

	if (rfPortData.ack != 0)
	{
		pData[0] = rfPortData.ack;
		*fullness = 1;

		rfPortData.ack = 0;
		blockNumber	= U8_XOR_U8(blockNumber, 1);
		return PHDD_CIU_OK;
	}

	pLen = 1;
	if (HWIO_DIRECTION_RECV != rfPortData.direction)
	{
		HwIo_Rx(&(rfPortData.buffer));
		rfPortData.fullness = 0;
		remainSize = rfPortData.buffer.len;
	}
	else
	{
		remainSize = U16_S_U16(rfPortData.buffer.len, rfPortData.fullness);
	}

	rxSize = MIN(dataSize, fsc);

	if (rxSize > remainSize)
	{
		pcb = PCB_I_BLOCK_MASK | (blockNumber & PCB_BLOCK_NUMBER_MASK);
		iLen = remainSize;
	}
	else
	{
		pcb = PCB_I_BLOCK_MASK | PCB_CHAIN_MASK | (blockNumber & PCB_BLOCK_NUMBER_MASK);
		iLen = U16_S_U16(rxSize , pLen);
	}

	pData[0] = pcb;
	ecpMem_Copy(&pData[1], &rfPortData.buffer.data[rfPortData.fullness], iLen);
	rfPortData.fullness = U16_A_U16(rfPortData.fullness, iLen);

	*fullness = U16_A_U16(iLen, pLen);


	rfPortData.direction = HWIO_DIRECTION_RECV;
	blockNumber	= U8_XOR_U8(blockNumber, 1);

	return PHDD_CIU_OK;
}

u16 rfPort_RxBuffer(PX8 pData, u16 dataSize, PR16 fullness, u8 flags, u16 fsc)
{
	u16 result;

	switch (rfStatus)
	{
	case RF_ISO14443L3_IDLE_STATE:
		*fullness = 0;
		*pData = REQA;
		result = RF_ISO14443L3_SHORT_FRAME_LENGTH;
		break;
	//case RF_ISO14443L3_READY_STATE:
	//	break;
	case RF_ISO14443L3_ACTIVE_STATE: //RATS
		*fullness = 4;
		pData[0] = RF_ISO14443L3_RATS;
		pData[1] = 0x80; // FSDI | CID
		isATS = true;
		//CRC
		result = PHDD_CIU_OK;
		rfStatus = RF_ISO14443L3_ISO14443L4_STATE;
		break;
	case RF_ISO14443L3_HALT_STATE:
		break;
	case RF_ISO14443L3_READY_S_STATE:
		break;
	case RF_ISO14443L3_ACTIVE_S_STATE:
		break;
	case RF_ISO14443L3_ISO14443L4_STATE:
		isRecv = true;
		result = rxCmd(pData, dataSize, fullness, fsc);
		break;
	default:
		result = PHDD_CIU_RX_ERROR;
		break;
	}
	return result;
}

void rfPort_TxBuffer(PX8 pData, u16 fullness, u8 flags)
{
	int i;
	u8 pcb;
	u8 pLen;
	u16 iLen;

	if (fullness <= 0)
	{
		return ;
	}

	fprintf(stderr, "rfPort_TxBuffer:  ");
	for (i = 0; i < fullness; i++)
	{
		if (pData[i] > 0xF)
		{
			fprintf(stderr, "%x ", pData[i]);
		}
		else
		{
			fprintf(stderr, "0%x ", pData[i]);
		}

	}

	fprintf(stderr, "\n");
	fflush(stderr);

	if ((fullness == 2) && (pData[0] == *PhIso14443L3AtqaB1b8WholeByteL) &&  (pData[1] == *PhIso14443L3AtqaB1b8WholeByteL)) //ATQA
	{
		rfStatus = RF_ISO14443L3_READY_STATE;
		return ;
	}


	pcb = pData[0];
	if (false == isRecv)
	{

	}
	else if (true == isATS)
	{

		u8 hisOffset = 2;
		u8 hisLen = 0;
		u8 atrLen;
		boolean isTckPresent;
		u8 tckValue;

		isTckPresent = true;

		if (((pData[1] >> 4) & 0x1) == 1)
		{
			U8_INC(hisOffset);
		}

		if (((pData[1] >> 5) & 0x1) == 1)
		{
			U8_INC(hisOffset);
		}

		if (((pData[1] >> 6) & 0x1) == 1)
		{
			U8_INC(hisOffset);
		}

		hisLen = pData[0] - hisOffset;
		atrLen = hisLen + 4;

		u8 atrs[atrLen];

		atrs[0] = 0x3B;
		atrs[1] = 0x80 | hisLen;
		atrs[2] = 0x80;
		atrs[3] = 0x01;

		ecpMem_Copy(&atrs[4], &pData[hisOffset], hisLen);
		memcpy(rfPortData.buffer.data, atrs, atrLen);

		if(isTckPresent)
		{
			tckValue = atrs[1];

			for (i = 2; i < atrLen; U8_INC(i))
			{
				tckValue			= U8_XOR_U8(tckValue, atrs[i]);
			}

			rfPortData.buffer.data[atrLen]	= tckValue;
			U8_INC(atrLen);
		}


		rfPortData.buffer.len = atrLen;
		HwIo_Tx(&(rfPortData.buffer));
		isATS = false;
	}
	else if ((pcb & HW_RF_I_BLOCK_MASK) == HW_RF_I_BLOCK) //i block
	{
		pLen = 1;
		if (HWIO_DIRECTION_SEND != rfPortData.direction)
		{
			rfPordDataReset();
			rfPortData.direction = HWIO_DIRECTION_SEND;
		}

		iLen = U16_S_U8(fullness, pLen);
		ecpMem_Copy(&rfPortData.buffer.data[rfPortData.buffer.len], &pData[1], iLen);
		rfPortData.buffer.len = U16_A_U16(rfPortData.buffer.len, iLen);


		if ((pcb & PCB_CHAIN_MASK) == PCB_CHAIN_MASK)
		{
			//send ACK
			rfPortData.ack = 0xA2 | (blockNumber & PCB_BLOCK_NUMBER_MASK);
		}
		else
		{
			HwIo_Tx(&(rfPortData.buffer));
		}

	}
}
