#include "DataOperation.h"
#include "SysConfig.h"
#include "7816Util.h"
#include "Io_SPI.h"
#include "Io_Protocol.h"
#include "SpiHal.h"
#include "SysConfig.h"
#include "chipHead.h"
#include "Io.h"
#include "string.h"
#include "nativemethod_impl.h"

#if BOOT__IO__SPI_I2C_PROTOCOL_A



PROTOCOL_STATUS  protocol_status[PROTOCOL_CHANNEL_NUM];

/* at least 4 bytes are required */
#define PH_MIN_BLOCK_SIZE   	0x08
#define PH_R_TRN_BLOCK_SIZE 	0x06
#define PH_S_TRN_BLOCK_SIZE 	0x06

#if PROTOCOL_EXTEND_LENGTH
#define LC_LENGTH_ERROR         0X6700
#define LRC_CHECK_ERROR         0X6A90
#define RRAMER_HEADER_ERROR     0X6A90
#define CLA_NOT_SUPPORT         0X6E00
#else
#define LC_LENGTH_ERROR         0X8200
#define LRC_CHECK_ERROR         0X8100
#define RRAMER_HEADER_ERROR     0X8200
#define CLA_NOT_SUPPORT         0X8200
#endif

#define LRC_ERROR     			0x81
#define OTHER_ERROR    			0x82
#define SWR_REQUEST    			0xCF
#define SWR_RESPONSE    		0xEF
/**
  *@brief	clear the protocol error  count
  *@param	the protocol ID
  *@param	
  *@retval	None.
  */
void ecpClearProtocolStatus(u8 protocol_id)
{
	protocol_status[protocol_id].R_block_NUMcnt = 0;
}
/**
  *@brief	init protocol 
  *@param	the protocol ID
  *@param	
  *@retval	None.
  */
void ecpIoProtocol_Init(u8 protocol_id, pSendToIO pFunc, u8* IblockBuf, u8* SblockBuf)
{

	protocol_status[protocol_id].R_block_NUMcnt = 0;
	protocol_status[protocol_id].ApduLe =0;
#if PROTOCOL_EXTEND_LENGTH
	protocol_status[protocol_id].extendflag =0;
#endif
	protocol_status[protocol_id].pIBlockBuf = IblockBuf;
	protocol_status[protocol_id].iBlockDataLen = 0;
	protocol_status[protocol_id].pSBlockBuf = SblockBuf;
	protocol_status[protocol_id].pSendProtocol = pFunc;
	protocol_status[protocol_id].pPrivSendLen = 0;

}
/**
  *@brief	return the value of LE bu protocol ID
  *@param	the protocol ID
  *@param	
  *@retval	the value
  */
u8 ecpIoProtocol_GetApduLe(u8 protocol_id)
{
	return protocol_status[protocol_id].ApduLe;
}
/**
  *@brief	culate the  lrc
  *@param	the buff to be culated
  *@param	the length of the buf
  *@retval	the lrc value.
  */
static u8 ecpIoProtocol_getLrc(PX8 pBuffer, u16 length )
{
    u16 I_A i;
    u8  I_A lrc;

	lrc = 0;

	for (i = 0; i < length; U16_INC(i) )
    {
		lrc	= U8_XOR_U8(lrc, pBuffer[i]);
    }
	lrc=~lrc;

    return lrc;
}
/**
  *@brief	Decode the receive data
  *@param	the buff which is to decoded
  *@param	the length of the data
  *@retval	the result .
  */

u8 ecpIoProtocol_DecodeBlock( u8* revBuf, u8* cmdBuf, u16 revLen ,u8 protocol_id)
{
	u8   						D_A					lrc;
	u16  						D_A					blockSize ;
	u8							X_A	 				error;
    PX8 						D_A					pBlock;
	u16							D_A					lc;
#if PROTOCOL_EXTEND_LENGTH
	u16							D_A					num;
#endif

	error     = BLOCK_NO_ERROR;
	pBlock    = revBuf;
	blockSize = revLen;

#if PROTOCOL_EXTEND_LENGTH
	num = 0;
    // 判断如果是全0xff或者0x00,认为是取数据帧。不返回错误
	while(num < revLen)
	{
		if((pBlock[num] != 0x00)&&(pBlock[num] != 0xff))
		{
			break;
		}
		num++;
	}
	if(num == revLen)
	{
		return TYPE_EMPTY_FRAME;
	}
#else
	if(0 == revLen)
	{
		return TYPE_EMPTY_FRAME;
	}
#endif
	if( revLen > I_BLOCK_BUFFER_SIZE)  //大于2.5K  返回长度错误
	{
		error =  BLOCK_LENGTH_ERROR;
	}

	if((pBlock[RESPONSE_OFFSET_SYNC] != IBLOCK_PCB_COMMAND_BLOCK)&&(pBlock[RESPONSE_OFFSET_SYNC] != SBLOCK_PCB_COMMAND_BLOCK)&&(pBlock[RESPONSE_OFFSET_SYNC] != RBLOCK_PCB_COMMAND_BLOCK))
	{
		error = BLOCK_FRAME_HEADER_ERROR;
	}
	else if(blockSize == PH_S_TRN_BLOCK_SIZE )//blockSize=6
	{
        if ( (pBlock[RESPONSE_OFFSET_SW1] == SWR_REQUEST) && (pBlock[RESPONSE_OFFSET_SYNC] == SBLOCK_PCB_COMMAND_BLOCK) )
		{
            //null
		}
#if !PROTOCOL_EXTEND_LENGTH
        else if(((pBlock[RESPONSE_OFFSET_SW1]==LRC_ERROR)||(pBlock[RESPONSE_OFFSET_SW1]==OTHER_ERROR)) && (pBlock[RESPONSE_OFFSET_SYNC] == RBLOCK_PCB_COMMAND_BLOCK))
        {

        }
#endif
        else
        {
        	error =  BLOCK_CLA_NOT_SUPPORT;
        }
	}
	else if(blockSize >= PH_MIN_BLOCK_SIZE) //blockSize >= 10
	{
		if (pBlock[RESPONSE_OFFSET_SYNC] != IBLOCK_PCB_COMMAND_BLOCK)
		{
			error = BLOCK_FRAME_HEADER_ERROR;
		}
		else
		{
#if PROTOCOL_EXTEND_LENGTH
			lc = (pBlock[HEADER_LC1_OFFSET]<<8) + pBlock[HEADER_LC2_OFFSET];
			if(pBlock[HEADER_LC1_OFFSET] ==0)
			{
				protocol_status[protocol_id].extendflag =0;
			}
			else
			{
				protocol_status[protocol_id].extendflag =1;
			}
#else
			lc =pBlock[HEADER_LC_OFFSET];
#endif
	        
	        blockSize = PH_MIN_BLOCK_SIZE + lc;
	        /*
			if (lc != 0)
			{
				blockSize = PH_MIN_BLOCK_SIZE + lc;
			}
			else
			{
				blockSize = PH_MIN_BLOCK_SIZE;
			}
            */
			if (blockSize >  revLen)
			{
				error =  BLOCK_LENGTH_ERROR;
			}

		}
	}
	else
	{
		error =  BLOCK_LENGTH_ERROR;
	}
	if(error == BLOCK_NO_ERROR)
	{
		lrc = pBlock[blockSize - 1];
		
		if (ecpIoProtocol_getLrc(&pBlock[HEADER_CLA_OFFSET], blockSize - 2) != lrc)
		{
			error =  BLOCK_CRC_OR_LRC_ERROR;
		}
	}

	if(error == BLOCK_NO_ERROR)
	{
		//if (pBlock[RESPONSE_OFFSET_SYNC] == SBLOCK_PCB_COMMAND_BLOCK)   //0x33
		{
			if((pBlock[RESPONSE_OFFSET_SW1]==SWR_REQUEST)&&(pBlock[RESPONSE_OFFSET_SW2]==0x00)&&(blockSize == PH_S_TRN_BLOCK_SIZE))
			{
//				ClearrfFirstSelectCmdFlag();
				//sysConfig_SetCpuClkCb();//switch SYSCLK to high level
//				REG_SET_BIT(SYSRSTCFG,SYSRSTCFG_7816RSTEN);  //7816 rst as sys rst,config first
				//rtnBackup(SPI_REREST_FLAG);

				ecpIoProtocol_TxStateMachine(BLOCK_NO_ERROR_SEND_ATR, 0, 0, protocol_id);
				protocol_status[protocol_id].R_block_NUMcnt = 0;
//				ecpSysHal_SystemReset();
				return TYPE_WARMRESET;
			}
		}
		
		if(protocol_status[protocol_id].R_block_NUMcnt >(1+COMMUNICATION_REPEAT_NUM))
		{
			return TYPE_ERROR_RETRANSMIT_MAX;
		}

		protocol_status[protocol_id].R_block_NUMcnt = 0;
#if !PROTOCOL_EXTEND_LENGTH
		if (pBlock[RESPONSE_OFFSET_SYNC] == RBLOCK_PCB_COMMAND_BLOCK)   //0x99
		{
			if( ((pBlock[RESPONSE_OFFSET_SW1]==LRC_ERROR)||(pBlock[RESPONSE_OFFSET_SW1]==OTHER_ERROR)) && (pBlock[RESPONSE_OFFSET_SW2]==0x00) &&(blockSize == PH_S_TRN_BLOCK_SIZE) )
			{
				protocol_status[protocol_id].pSendProtocol(protocol_status[protocol_id].pPrivSendBuf, protocol_status[protocol_id].pPrivSendLen);
				return TYPE_REQUEST_RETRANSMISSION;
			}
		}
#endif
#if PROTOCOL_EXTEND_LENGTH
		protocol_status[protocol_id].ApduLe = 0;
		lc = (pBlock[HEADER_LC1_OFFSET]<<8)+pBlock[HEADER_LC2_OFFSET];
		ecpMem_Copy(&cmdBuf[0], &pBlock[HEADER_CLA_OFFSET],blockSize-2);
#else
		protocol_status[protocol_id].ApduLe = pBlock[HEADER_LE_OFFSET];
		ecpMem_Copy(&cmdBuf[0], &pBlock[HEADER_CLA_OFFSET],IO_COMMAND_HEADER_LEN);
		ecpMem_Copy(&cmdBuf[4], &pBlock[HEADER_LC_OFFSET], pBlock[HEADER_LC_OFFSET]+1);
#endif
		/*
		ecpMem_Copy(&apduBuf[0], &pBlock[HEADER_CLA_OFFSET], 4);
        //保存LC和LE数据
		if(blockSize == PH_MIN_BLOCK_SIZE)
		{
			apduBuf[4] = pBlock[HEADER_LC1_OFFSET];
			apduBuf[5] = pBlock[HEADER_LC2_OFFSET];
			apduBuf[6] = 0;
			apduBuf[7] = 0;
		}
		else if(blockSize > PH_MIN_BLOCK_SIZE)
		{
			apduBuf[4] = 0;
			apduBuf[5] = 0;
			apduBuf[6] = pBlock[HEADER_LC1_OFFSET];
			apduBuf[7] = pBlock[HEADER_LC2_OFFSET];
		}

		lc = (apduBuf[6] <<8) + apduBuf[7] ;
		ecpMem_Copy(&apduBuf[8], &pBlock[HEADER_PAY_OFFSET], lc);
        */
		return TYPE_RECEIVE_DATA_TYPE_APDU;
	}
	else
	{
	    //出现帧错误，清除上一帧数据。
		//Spi_PrivTxLength = 0;
		if(protocol_status[protocol_id].R_block_NUMcnt >COMMUNICATION_REPEAT_NUM)
		{
			protocol_status[protocol_id].pPrivSendLen = 0;
			return TYPE_ERROR_RETRANSMIT_MAX;
		}
		ecpIoProtocol_TxStateMachine(error, 0, 0, protocol_id);


		U8_INC(protocol_status[protocol_id].R_block_NUMcnt);
		return TYPE_ERROR_HAPPEN;
	}
}
/**
  *@brief	protocol send data 
  *@param	the type of send
  *@param	the buff of data
  *@param	the length of sent data
  *@retval	None.
  */
void ecpIoProtocol_TxStateMachine(u8 errorStatus, u16 sw, u16 txDataLen, u8 protocol_id)
{
	u16 offset;
	u16 length;
	u8* ptrBuf;
	
	PROTOCOL_STATUS *pProtocolCtrl;

	pProtocolCtrl = &protocol_status[protocol_id];

	offset = 0;

	if (BLOCK_NO_ERROR == errorStatus)
	{
		ptrBuf = pProtocolCtrl->pIBlockBuf;
		ptrBuf[offset++] = IBLOCK_PCB_COMMAND_BLOCK;  // 0x55
		ptrBuf[offset++] = (u8)(sw >> 8);    //SW1
		ptrBuf[offset++] = (u8)(sw & 0xFF);    //SW2
		ptrBuf[offset++] = (u8)(txDataLen >> 8);   //len1
		ptrBuf[offset++] = (u8)(txDataLen & 0xFF); //len2

		ptrBuf[RESPONSE_OFFSET_DATA + txDataLen] = ecpIoProtocol_getLrc(&ptrBuf[RESPONSE_OFFSET_SW1], txDataLen + RESPONSE_OFFSET_LEN2);

		pProtocolCtrl->iBlockDataLen = txDataLen + RESPONSE_I_BLOCK_MINISIZE;

		length = pProtocolCtrl->iBlockDataLen;
		/*  REQUEST RETRANSMISSION 20220519  */
		pProtocolCtrl->pPrivSendBuf = ptrBuf;
		pProtocolCtrl->pPrivSendLen = length;

	}
	else if (BLOCK_CLA_NOT_SUPPORT == errorStatus)
	{
		ptrBuf = pProtocolCtrl->pSBlockBuf;
		ptrBuf[offset++] = RBLOCK_PCB_COMMAND_BLOCK;
		ptrBuf[offset++] = (CLA_NOT_SUPPORT>>8)&0xff;//SW1
		ptrBuf[offset++] = (CLA_NOT_SUPPORT&0xff);//SW2
		ptrBuf[offset++] = 0x00;//LEN1
		ptrBuf[offset++] = 0x00;//LEN2
		ptrBuf[offset++] = ecpIoProtocol_getLrc(&ptrBuf[RESPONSE_OFFSET_SW1], 4);//LEN2

		length = PH_R_TRN_BLOCK_SIZE;
	}
	else if (BLOCK_FRAME_HEADER_ERROR == errorStatus)
	{
		ptrBuf = pProtocolCtrl->pSBlockBuf;
		ptrBuf[offset++] = RBLOCK_PCB_COMMAND_BLOCK;
		ptrBuf[offset++] = (RRAMER_HEADER_ERROR>>8)&0xff;//SW1
		ptrBuf[offset++] = (RRAMER_HEADER_ERROR&0xff);//SW2
		ptrBuf[offset++] = 0x00;//LEN1
		ptrBuf[offset++] = 0x00;//LEN2
		ptrBuf[offset++] = ecpIoProtocol_getLrc(&ptrBuf[RESPONSE_OFFSET_SW1], 4);//LEN2

		length = PH_R_TRN_BLOCK_SIZE ;
	}
	else if (BLOCK_CRC_OR_LRC_ERROR == errorStatus)
	{
		ptrBuf = pProtocolCtrl->pSBlockBuf;
		ptrBuf[offset++] = RBLOCK_PCB_COMMAND_BLOCK;
		ptrBuf[offset++] = (LRC_CHECK_ERROR>>8)&0xff;//SW1
		ptrBuf[offset++] = (LRC_CHECK_ERROR&0xff);//SW2
		ptrBuf[offset++] = 0x00;//LEN1
		ptrBuf[offset++] = 0x00;//LEN2
		ptrBuf[offset++] = ecpIoProtocol_getLrc(&ptrBuf[RESPONSE_OFFSET_SW1], 4);//LEN2

		length = PH_R_TRN_BLOCK_SIZE;
	}
	else if(BLOCK_LENGTH_ERROR == errorStatus)
	{
		ptrBuf = pProtocolCtrl->pSBlockBuf;
		ptrBuf[offset++] = RBLOCK_PCB_COMMAND_BLOCK;
		ptrBuf[offset++] = (LC_LENGTH_ERROR>>8)&0Xff;//SW1
		ptrBuf[offset++] = LC_LENGTH_ERROR & 0xff;//SW2
		ptrBuf[offset++] = 0x00;//LEN1
		ptrBuf[offset++] = 0x00;//LEN2
		ptrBuf[offset++] = ecpIoProtocol_getLrc(&ptrBuf[RESPONSE_OFFSET_SW1], 4);//LEN2

		length = PH_R_TRN_BLOCK_SIZE;
	}
	else if(BLOCK_NO_ERROR_SEND_ATR == errorStatus)
	{
		ptrBuf = pProtocolCtrl->pSBlockBuf;
		ptrBuf[offset++] = SBLOCK_PCB_COMMAND_BLOCK;
		ptrBuf[offset++] = SWR_RESPONSE;//SW1
		ptrBuf[offset++] = 0x00;//SW2
		ptrBuf[offset++] = 0x00;//LEN1
		ptrBuf[offset++] = 0x00;//LEN2
		ptrBuf[offset++] = ecpIoProtocol_getLrc(&ptrBuf[RESPONSE_OFFSET_SW1], 4);//LEN2

		length = PH_R_TRN_BLOCK_SIZE;
	}
	else
	{
        return;
	}
#if PROTOCOL_EXTEND_LENGTH
	pProtocolCtrl->pPrivSendBuf = ptrBuf;
	pProtocolCtrl->pPrivSendLen = length;
#endif
	pProtocolCtrl->pSendProtocol(ptrBuf, length);
}

#endif

