#include "Io_7816T1.h"
#include "SysConfigTarget.h"
#include "BootRam.h"
#include "7816Channel.h"
#include "Io_Uart.h"
#if HAL__IO__ISO7816_T1

#pragma IO_7816_T1_RAM_START

static phIso7816T1_Control_t	X_A 				gIso7816T1_Ctrl;

IO_7816_T1_RAM_END

/*===========================================================================*
 * Defines:
 *===========================================================================*/
/*#define PH_ISO7816T1_PPSS			0xFF		 */

/* ignore LRC */
//#define PHFLPROTOCOL_T1_IGNORE_LRC

/* HEADER OFFSETS */
#define HEADER_NAD_OFFSET			0
#define HEADER_PCB_OFFSET			1
#define HEADER_LEN_OFFSET			2
#define HEADER_PAY_OFFSET			3

/* I BLOCK */
/*#define I_BLOCK_MASK				(0x80)		*/
#define I_BLOCK_SIGNATURE			(0x00)
#define I_BLOCK_CHAINING			(0x20)
#define I_BLOCK_SEQ					(0x40)

/* R BLOCK */
#define R_BLOCK_MASK				(0xC0)
#define R_BLOCK_SIGNATURE			(0x80)
#define R_BLOCK_ERROR_MASK			(0x0F)
#define R_BLOCK_SEQ					(0x10)

/* SUPPORTED S BLOCKS */
#define T1_S_IFS_REQUEST			(0xC1)
#define T1_S_IFS_ACK				(0xE1)
#define T1_S_BWT_REQUEST			(0xC3)
#define T1_S_BWT_ACK				(0xE3)

/* MINiMUM SIZE OF BLOCK PAYLOAD */
#define MIN_IFS             1
/* MAXIMUM SIZE OF BLOCK PAYLOAD */
#define MAX_IFS           254

/* at least 4 bytes are required */
#define PH_ISO7816T1_MIN_BLOCK_SIZE  0x04
/*#define PH_ISO7816T1_MIN_BLOCK_SIZE_CRC 0x05 */

#define	T1_BLOCK_NO_ERROR			0x00
#define	T1_BLOCK_OTHER_ERROR		0x01
#define	T1_BLOCK_CRC_OR_LCR_ERROR	0x02


/*===========================================================================*
 * Local functions:
 *===========================================================================*/
static void Iso7816T1_RxStateMachine( void );
static void Iso7816T1_TxStateMachine( u8 errorStatus );
static void Iso7816T1_SendBlock( void );
static void Iso7816T1_DecodeBlock( void );
static u8 Iso7816T1_CalculateLrc( PX8 pBuffer, u16 length );


static void iso7816T1_InitVar( void )
{
	PATCH_BOOT_APDU_IO7816T1_001();
	
	gIso7816T1_Ctrl.CWT				= U16_A_U16(U16_SHL_U8(1, U8_AND_U8(sysConfig_GetUartT1TB(), 0x0F)),  11);
	gIso7816T1_Ctrl.T1CheckSumType	= sysConfig_GetUartT1TC();
	if(1 == gIso7816T1_Ctrl.T1CheckSumType)
	{
		ecpUartT1_SetCrcCal();
	}
}

static u16 Iso7816T1_CalculateCrc(PX8 pBuffer, u16 length)
{
	return ecpUartCalT1Crc(pBuffer, length);
}

/*===========================================================================*/
/**
 * @brief  The function Iso7816T1_CalculateLrc() calculates the LRC
 *         checksum for the T=1 protocol.
 *
 * @param  uint8_t * pBuffer
 *         buffer for that the checksum shall be calculated
 * @param  uint16_t length
 *         number of bytes for checksum calculation
 *
 * @return None
 *
 * @note   None
 */
/*===========================================================================*/
static u8 Iso7816T1_CalculateLrc(PX8 pBuffer, u16 length )
{
    u16 I_A i;
    u8  I_A lrc;

	lrc = 0;

	for (i = 0; i < length; U16_INC(i) )
    {
		lrc	= U8_XOR_U8(lrc, pBuffer[i]);
    }

    return lrc;
}

/*===========================================================================*/
/**
 * @brief  The function Iso7816T1_SendBlock() sends a block
 *         for the T=1 protocol.
 *
 * @param  None
 *
 * @return None
 *
 * @note   None
 */
/*===========================================================================*/
static void Iso7816T1_SendBlock( void )
{
	PATCH_BOOT_APDU_IO7816T1_002();
	
	if( (gIso7816T1_Ctrl.T1CheckSumType == 0) || (gIso7816T1_Ctrl.pUartBuffer->pData[0] == 0xFF)   )
	{
	    // calculate and append checksum to buffer
	    gIso7816T1_Ctrl.pUartBuffer->pData[gIso7816T1_Ctrl.pUartBuffer->fullness] =
		Iso7816T1_CalculateLrc( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->fullness );
	    U16_INC(gIso7816T1_Ctrl.pUartBuffer->fullness);
	}
	else
	{
		PX_W_U16( &gIso7816T1_Ctrl.pUartBuffer->pData[ gIso7816T1_Ctrl.pUartBuffer->fullness ] ,          \
	    Iso7816T1_CalculateCrc( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->fullness ) );
	    gIso7816T1_Ctrl.pUartBuffer->fullness += 2;
	}

#if ( defined (PHFL_UART_USE_CM0) || defined (PHFL_UART_USE_CM1) )
    if( PHDD_UART_OK == ecpUart_TxBuffer(gIso7816T1_Ctrl.pUartBuffer) )
    {
        gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
    }

    /* wait for reception done */
    while ( phddUart_GetStatus() == PHDD_UART_BUSY )
	{
        ;
	}
#else
    if( PHDD_UART_OK == ecpUart_TxBuffer(gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->size, gIso7816T1_Ctrl.pUartBuffer->fullness, PHDD_UART_TX_OPTION_LCT) )
    {
        gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
    }
#endif

    return;
}

static void Iso7816T1_Pps(void)
{
	u8	pps0;
	u8	pps1;

	pps0	= gIso7816T1_Ctrl.pUartBuffer->pData[1];
	pps1	= gIso7816T1_Ctrl.pUartBuffer->pData[2];

	if(true == IoUart_Pps(gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->fullness))
	{
		iso7816T1_AnswerPps(T1_PPS_SUCESS, gIso7816T1_Ctrl.pUartBuffer->fullness);
		IoUart_SetPps0(pps0);
		if(U8_AND_U8(pps0, 0x10))
		{
			IoUart_SetPps1(pps1);
		}
	}
	else
	{
		iso7816T1_AnswerPps(T1_PPS_FAIL, 3);
	}
}

/*===========================================================================*/
/**
 * @brief  The function Iso7816T1_DecodeBlock() decodes a received
 *         block for the T=1 protocol.
 *
 * @param  None
 *
 * @return None
 *
 * @note   None
 */
/*===========================================================================*/
#define						CHANNEL_MASK		0x03
static void Iso7816T1_DecodeBlock( void )
{
	u8   						D_A					lrc;
	u8							D_A					logicalChannel;
	u16  						D_A					blockSize ;
	u8							X_A	 				error;
	u16                         X_A                 crc;
    PX8 						D_A					pBlock;
    ppIso7816Util_Control_t							control;
    ppIso7816Util_Command_t							comand;
	
	PATCH_BOOT_APDU_IO7816T1_003();
	
    iso7816Util_GetCommand( &comand );
    iso7816Util_GetControl( &control );
	error     = T1_BLOCK_NO_ERROR;
	pBlock    = MEMtPX8(gIso7816T1_Ctrl.pUartBuffer->pData);

	if( (gIso7816T1_Ctrl.T1CheckSumType == 0) || (gIso7816T1_Ctrl.pUartBuffer->pData[0] == 0xFF)   )
	{
		blockSize	= U16_S_U8(gIso7816T1_Ctrl.pUartBuffer->fullness, 1);
	   	lrc			= gIso7816T1_Ctrl.pUartBuffer->pData[U16_S_U8(gIso7816T1_Ctrl.pUartBuffer->fullness, 1)];
		    /* check checksum */
	   	if (Iso7816T1_CalculateLrc( pBlock, blockSize ) != lrc )
	    {
	        	// checksum error
	       	 error = T1_BLOCK_CRC_OR_LCR_ERROR;
	    }
	}
	else
	{
		blockSize	= U16_S_U8(gIso7816T1_Ctrl.pUartBuffer->fullness, 2);
	   	crc         = PX_R_U16(&gIso7816T1_Ctrl.pUartBuffer->pData[gIso7816T1_Ctrl.pUartBuffer->fullness-2]);
	   	if (Iso7816T1_CalculateCrc( pBlock, blockSize ) != crc)
	    {
	         // checksum error
	       	 error = T1_BLOCK_CRC_OR_LCR_ERROR;
	    }
	}

	if(error == T1_BLOCK_NO_ERROR)
	{
		if((IO_UART_PPS_BYTE == pBlock[0]) && (true == IoUart_IsPpsAllowed()))				//	PPS	request
		{
			Iso7816T1_Pps();
			return;
		}
    }

		  /* check size */
	if (gIso7816T1_Ctrl.pUartBuffer->fullness < PH_ISO7816T1_MIN_BLOCK_SIZE)
 	{
	        // no valid block error
	     error = T1_BLOCK_OTHER_ERROR;
    }

    /* check if LEN field is correct */
    if ( U16_A_U8(pBlock[HEADER_LEN_OFFSET], HEADER_PAY_OFFSET) != blockSize )
    {
        // Len field not correct error
        error = T1_BLOCK_OTHER_ERROR;
    }


	if ( (R_BLOCK_SIGNATURE == U8_AND_U8(pBlock[HEADER_PCB_OFFSET], R_BLOCK_MASK)) &&
		 (U8_AND_U8(pBlock[HEADER_PCB_OFFSET], R_BLOCK_ERROR_MASK) == 0) )
	{
			/*check R-BLOCK INF */
		if(gIso7816T1_Ctrl.pUartBuffer->fullness == PH_ISO7816T1_MIN_BLOCK_SIZE)
		{
			if(T1_BLOCK_NO_ERROR == error)
			{
				if(gIso7816T1_Ctrl.rxState != PH_ISO7816T1_RXSUB_EXPECT_R_ACK )
				{
					if(U8_AND_U8(U8_SHR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], 4), 0x01)  != U8_AND_U8(U8_SHR_U8(gIso7816T1_Ctrl.seqNumTerm, 6), 0x01))
					{
						 gIso7816T1_Ctrl.seqNumCard				= U8_XOR_U8(gIso7816T1_Ctrl.seqNumCard, I_BLOCK_SEQ);
						 control->numBytesToTransmitFromSW1SW2	= 2;
								  // receive error response
						 control->isRetransmitBlock				= 1;
						 Iso7816T1_TxStateMachine( T1_BLOCK_NO_ERROR );
						 return;
					}
					else
					{
						error = T1_BLOCK_OTHER_ERROR;
					}
				}
				else
				{
					if(U8_AND_U8(U8_SHR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], 4), 0x01) != U8_AND_U8(U8_SHR_U8(gIso7816T1_Ctrl.seqNumCard, 6), 0x01))
					{
						 gIso7816T1_Ctrl.seqNumCard				= U8_XOR_U8(gIso7816T1_Ctrl.seqNumCard, I_BLOCK_SEQ);
						 control->numBytesToTransmitFromSW1SW2	= 2;
								  // receive error response
						 control->isRetransmitBlock				= 1;
							Iso7816T1_TxStateMachine( T1_BLOCK_NO_ERROR );
						 return;
					}

				}
			}
		}
		else
		{
			error = T1_BLOCK_OTHER_ERROR;
		}
   	}


	if(U8_AND_U8(pBlock[HEADER_PAY_OFFSET], 0x40) == 0) //没扩展情况
	{
		logicalChannel			= U8_AND_U8(pBlock[HEADER_PAY_OFFSET], CHANNEL_MASK);
		gIso7816T1_Ctrl.t1_nad[logicalChannel]	= pBlock[HEADER_NAD_OFFSET];
	}

    /* check NAD field is correct */
    if ( 0 != pBlock[HEADER_NAD_OFFSET] )
 	{
    	error = T1_BLOCK_OTHER_ERROR;
    }

	    /* received error response, retransmit last packet */
    if ( (R_BLOCK_SIGNATURE == U8_AND_U8(pBlock[HEADER_PCB_OFFSET], R_BLOCK_MASK)) &&
		 U8_AND_U8(pBlock[HEADER_PCB_OFFSET], R_BLOCK_ERROR_MASK) )
    {
        // go back to last sequence number
		gIso7816T1_Ctrl.seqNumCard				= U8_XOR_U8(gIso7816T1_Ctrl.seqNumCard, I_BLOCK_SEQ);
		control->numBytesToTransmitFromSW1SW2	= 2;
        // receive error response
		control->isRetransmitBlock = 1;
        Iso7816T1_TxStateMachine( T1_BLOCK_NO_ERROR );

		return;
    }
		//0528
    if( (0!= gIso7816T1_Ctrl.seqNumTerm )&&(0xE1 == pBlock[HEADER_PCB_OFFSET]) && (0xFE!=pBlock[HEADER_PCB_OFFSET]))
	{
		//gIso7816T1_Ctrl.seqNumTerm = 0x40;//0528
		error = T1_BLOCK_OTHER_ERROR;
    }
		//0528
    if (T1_BLOCK_NO_ERROR == error)
    {
        Iso7816T1_RxStateMachine();
    }
    else
    {
		if(gIso7816T1_Ctrl.R_block_count >= 2)
		{
			gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
			gIso7816T1_Ctrl.state=PH_ISO7816T1_STATE_RX;
		 	return;
		}

		Iso7816T1_TxStateMachine( error );

		U8_INC(gIso7816T1_Ctrl.R_block_count);
    }

	return;
}


/*===========================================================================*/
/**
 * @brief  The function Iso7816T1_TxStateMachine() implements the
 *         transmit state machine of the T=1 protocol.
 *
 * @param  None
 *
 * @return None
 *
 * @note   None
 */
/*===========================================================================*/
static void Iso7816T1_TxStateMachine( u8 errorStatus )
{
    u16		D_A		length;
    ppIso7816Util_Control_t		control;

	PATCH_BOOT_APDU_IO7816T1_004();
	
    iso7816Util_GetControl( &control );
    if (T1_BLOCK_NO_ERROR == errorStatus)
    {
        switch(gIso7816T1_Ctrl.txState)
        {
        case PH_ISO7816T1_TXSUB_I:
            /* max buffer space minus header and footer */
            length = gIso7816T1_Ctrl.ifs;

            /* prepare block */
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET]	= 0;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]	= I_BLOCK_SIGNATURE;
			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] 	= U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], gIso7816T1_Ctrl.seqNumCard);

            /* use chaining if necessary */
            if ( iso7816Util_GetResponseLength( PH_ISO7816L4_FLAG_BYPASSPARSER ) > length )
            {
                /* new rx state (expected block) */
                // Chaining packet
                gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_EXPECT_R_ACK;
				gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] = U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], I_BLOCK_CHAINING);
                length = gIso7816T1_Ctrl.ifs;
            }
            else
            {
				if(control->isRetransmitBlock && control->dataIndex)
				{
					gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] = U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], I_BLOCK_CHAINING);
				}
                /* new rx state (expected block) */
                gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_READY;
                length					= iso7816Util_GetResponseLength( PH_ISO7816L4_FLAG_BYPASSPARSER );
            }

			gIso7816T1_Ctrl.seqNumCard = U8_XOR_U8(gIso7816T1_Ctrl.seqNumCard, I_BLOCK_SEQ);
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET] = length;
            iso7816Util_SetResponseData( PX8_A_CONST(gIso7816T1_Ctrl.pUartBuffer->pData, HEADER_PAY_OFFSET), length, PH_ISO7816L4_FLAG_BYPASSPARSER );
			gIso7816T1_Ctrl.pUartBuffer->fullness = U8_A_U16(HEADER_PAY_OFFSET, length);
            break;

        case PH_ISO7816T1_TXSUB_S_IFS_ACK:
            /* process IFS request */
            if (U8_A_U16((PH_ISO7816L4_MAX_EPILOGUE_LENGTH + PH_ISO7816L4_MAX_PROLOGUE_LENGTH), gIso7816T1_Ctrl.ifs) > gIso7816T1_Ctrl.pUartBuffer->size)
            {
                gIso7816T1_Ctrl.ifs	= U16_S_U8(gIso7816T1_Ctrl.pUartBuffer->size, (PH_ISO7816L4_MAX_EPILOGUE_LENGTH + PH_ISO7816L4_MAX_PROLOGUE_LENGTH));
            }
            /* check against maximum value */
            if( MAX_IFS < gIso7816T1_Ctrl.ifs )
            {
                gIso7816T1_Ctrl.ifs = MAX_IFS;
            }
            /* new rx state (expected I block) */
            gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_I_BLOCK;

            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET]	= 0x00;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]	= T1_S_IFS_ACK;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET]	= 0x01;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PAY_OFFSET]	= gIso7816T1_Ctrl.ifs;
            gIso7816T1_Ctrl.pUartBuffer->fullness					= HEADER_PAY_OFFSET + 1;
            break;

        case PH_ISO7816T1_TXSUB_S_BWT_ACK:
            /* new rx state (expected block) */
            gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_I_BLOCK;

            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET]	= 0x00;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]	= T1_S_BWT_ACK;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET]	= 0x01;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PAY_OFFSET]	= 1;
            gIso7816T1_Ctrl.pUartBuffer->fullness					= HEADER_PAY_OFFSET + 1;
            break;

        case PH_ISO7816T1_TXSUB_S_IFS_REQUEST:
            /* new rx state (expected block) */
            gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_S_IFS_ACK;

            /* prepare ifs request */
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET]	= 0x00;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]	= T1_S_IFS_REQUEST;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET]	= 0x01;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PAY_OFFSET]	= gIso7816T1_Ctrl.ifs;
            gIso7816T1_Ctrl.pUartBuffer->fullness					= HEADER_PAY_OFFSET + 1;
            break;

        case PH_ISO7816T1_TXSUB_ACK:
            /* new rx state (expected block) */
            gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_I_BLOCK;

            /* prepare ifs request */
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET] 	= 0x00;
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] 	= R_BLOCK_SIGNATURE;
            if ( 0 != gIso7816T1_Ctrl.seqNumTerm )
            {
				gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] = U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], R_BLOCK_SEQ);
            }
            gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET] 	= 0x00;
            gIso7816T1_Ctrl.pUartBuffer->fullness = HEADER_PAY_OFFSET;
            break;

        default:
            // Critical internal error
            return;
/*            break;	*/
        }
    }
    else
    {
        /* errors are no specific states */
        gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET]		= 0x00;
			//length/number follow ATR request 00 81 00 81 not 00 82 00 82
// 		if(/*(0xE1!=gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET])&&*/(0xC1!=gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET])&& (U8_AND_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], I_BLOCK_SEQ) != gIso7816T1_Ctrl.seqNumTerm ))
// 		{
// 			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]		= R_BLOCK_SIGNATURE;
// 			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]		= U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], (R_BLOCK_ERROR_MASK & 0x02));	
//         }
// 		else if(/*(0xE1!=gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET])&&*/(0xC1!=gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET])&&(U16_S_U8(gIso7816T1_Ctrl.pUartBuffer->fullness, 1) != U8_A_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET],HEADER_PAY_OFFSET)))
// 		{
// 			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]		= R_BLOCK_SIGNATURE;
// 			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]		= U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], (R_BLOCK_ERROR_MASK & 0x02));

// 		}
		if(T1_BLOCK_CRC_OR_LCR_ERROR == errorStatus)
		{
			/* TODO define a specific error 81 */
			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]		= U8_OR_U8(R_BLOCK_SIGNATURE, 0x01);
		}
		else 
		{
			/* TODO define a specific error 82 */
			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]		= U8_OR_U8(R_BLOCK_SIGNATURE, 0x02);
        }		
		if( 0 != gIso7816T1_Ctrl.seqNumTerm )
        {
			gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]	= U8_OR_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], R_BLOCK_SEQ);
        }
        gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET]		= 0x00;
        gIso7816T1_Ctrl.pUartBuffer->fullness						= HEADER_PAY_OFFSET;
    }

    /* send prepared block */
    Iso7816T1_SendBlock();

    /* set to RX-MODE */
    gIso7816T1_Ctrl.state=PH_ISO7816T1_STATE_RX;

    return;
}


/*===========================================================================*/
/**
 * @brief  The function Iso7816T1_RxStateMachine() implements the
 *         receive state machine of the T=1 protocol.
 *
 * @param  None
 *
 * @return None
 *
 * @note   None
 */
/*===========================================================================*/
static void Iso7816T1_RxStateMachine( void )
{
	PATCH_BOOT_APDU_IO7816T1_005();
	
	switch(gIso7816T1_Ctrl.rxState)
	{
	case PH_ISO7816T1_RXSUB_S_IFS_ACK:
	  gIso7816T1_Ctrl.txState	= PH_ISO7816T1_TXSUB_READY;
	  break;

	case PH_ISO7816T1_RXSUB_S_BWT_ACK:
	  gIso7816T1_Ctrl.txState	= PH_ISO7816T1_TXSUB_READY;
	  break;

	case PH_ISO7816T1_RXSUB_I_BLOCK:
	  if (gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] == T1_S_IFS_REQUEST)
	  {
		gIso7816T1_Ctrl.ifs		=  gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PAY_OFFSET];
		gIso7816T1_Ctrl.txState = PH_ISO7816T1_TXSUB_S_IFS_ACK;
	  }
	  else if (gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET] == T1_S_BWT_REQUEST)
	  {
		gIso7816T1_Ctrl.txState = PH_ISO7816T1_TXSUB_S_BWT_ACK;
		/* need to process ack */
	  }
	  else if(((U8_AND_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], 0xF0) == 0xC0) ||
	  		   (U8_AND_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], 0xF0)== 0xE0))&&
	  			U8_AND_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], 0x0C) != 0)
	  {

			if(gIso7816T1_Ctrl.R_block_count < 2)
			{
				Iso7816T1_TxStateMachine( T1_BLOCK_OTHER_ERROR );
				U8_INC(gIso7816T1_Ctrl.R_block_count);
				return;
			}
			else
			{
				return;
			}
	  }
	  else
	  {
		/* check sequence counter */
		  if ( U8_AND_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], I_BLOCK_SEQ) != gIso7816T1_Ctrl.seqNumTerm )
		{
		  	// Received Sequence Number is not correct error
		  	if(gIso7816T1_Ctrl.R_block_count < 2)
			{
				Iso7816T1_TxStateMachine( T1_BLOCK_OTHER_ERROR );
				U8_INC(gIso7816T1_Ctrl.R_block_count);
				return;
			}
			else
			{
				return;
			}
		}

		iso7816Util_ParseBytes( &(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PAY_OFFSET]), gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET], PH_ISO7816L4_FLAG_BYPASSPARSER );

		if ( 0 != U8_AND_U8(gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET], I_BLOCK_CHAINING) )
		{
		  // chaining
		  gIso7816T1_Ctrl.txState = PH_ISO7816T1_TXSUB_ACK;
		}
		else
		{
		  // No chaining
		  gIso7816T1_Ctrl.txState = PH_ISO7816T1_TXSUB_READY;
		}

		/* next expected I-Block */
		gIso7816T1_Ctrl.seqNumTerm = U8_XOR_U8(gIso7816T1_Ctrl.seqNumTerm, I_BLOCK_SEQ);
	  }
	  break;

	case PH_ISO7816T1_RXSUB_EXPECT_R_ACK:
	  gIso7816T1_Ctrl.txState = PH_ISO7816T1_TXSUB_I;
	  break;

	default:
	  break;
	}

	if (PH_ISO7816T1_TXSUB_READY != gIso7816T1_Ctrl.txState)
	{
		Iso7816T1_TxStateMachine(T1_BLOCK_NO_ERROR);
	}

	return;
}

void iso7816T1_Init()
{
	ppIso7816Util_Buffer_t			tempIoBuffer;

	PATCH_BOOT_APDU_IO7816T1_006();
	
	ecpUart_T1Init();
	iso7816T1_InitVar();
	iso7816Util_GetIoBuffer((ppIso7816Util_Buffer_t *) &tempIoBuffer );
	tempIoBuffer->fullness	= 0;
	gIso7816T1_Ctrl.pUartBuffer	= tempIoBuffer;

    /* check buffer size against IFS */
    /* minimum IFS size is one, so the buffer size
       minimum is one data byte plus prologue and epilogue size */
    if( gIso7816T1_Ctrl.pUartBuffer->size < ( MIN_IFS + PH_ISO7816L4_MAX_PROLOGUE_LENGTH + PH_ISO7816L4_MAX_EPILOGUE_LENGTH ) )
    {
        while(1);
    }
    /* calculate IFS based on buffer size */
    gIso7816T1_Ctrl.ifs = U16_S_U8(U16_S_U8(gIso7816T1_Ctrl.pUartBuffer->size, PH_ISO7816L4_MAX_EPILOGUE_LENGTH), PH_ISO7816L4_MAX_PROLOGUE_LENGTH);
    /* check against maximum IFS value */
    if( MAX_IFS < gIso7816T1_Ctrl.ifs )
    {
        gIso7816T1_Ctrl.ifs = MAX_IFS;
    }
    gIso7816T1_Ctrl.seqNumTerm  = 0;
    gIso7816T1_Ctrl.seqNumCard  = 0;

    /* activate Receive mode */
    gIso7816T1_Ctrl.state	= PH_ISO7816T1_STATE_RX;

    /* expect I_S BLock */
    gIso7816T1_Ctrl.rxState	= PH_ISO7816T1_RXSUB_READY;

    /* sent finished block */
    gIso7816T1_Ctrl.txState	= PH_ISO7816T1_TXSUB_READY;
		
}

//void iso7816T1_FetchCommand(PX8 buf)
//{

//}

u16 iso7816T1_Receive(void)
{
   	u16		D_A		sr_result;

	PATCH_BOOT_APDU_IO7816T1_007();	
	
  	gIso7816T1_Ctrl.R_block_count = 0;

    if( (gIso7816T1_Ctrl.rxState != PH_ISO7816T1_RXSUB_READY) || (gIso7816T1_Ctrl.state != PH_ISO7816T1_STATE_RX) )
    {
    	while(1);
    }

    iso7816Util_ResetParser();

    while( gIso7816T1_Ctrl.state == PH_ISO7816T1_STATE_RX )
    {
		
		gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_I_BLOCK;
		gIso7816T1_Ctrl.txState = PH_ISO7816T1_TXSUB_I;
        /* receive data */
        if( gIso7816T1_Ctrl.pUartBuffer->fullness == 0 )
        {
			sr_result = ecpUART_RxBuffer( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->size, &gIso7816T1_Ctrl.pUartBuffer->fullness, gIso7816T1_Ctrl.CWT, sysConfig_IsUartEnterSleepMode());

            if(PHDD_UART_OK != sr_result)
            {
				if(sr_result == PHDD_UART_ERROR)
				{
					gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_TX;
					if(gIso7816T1_Ctrl.R_block_count >= 2)
					{
						gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_RX ;
						gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
						continue;
					}
				    Iso7816T1_TxStateMachine( T1_BLOCK_CRC_OR_LCR_ERROR );
					U8_INC(gIso7816T1_Ctrl.R_block_count);
					continue;
				}
				else
				{
					gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_RX ;
					gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
				    continue;
				}
            }
        }

        /* set to TX-MODE */
        gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_TX;

		Iso7816T1_DecodeBlock();
    }

    return gIso7816T1_Ctrl.pUartBuffer->fullness ;
}

u16 iso7816T1_Transmit(void)
{
    u16		D_A		sr_result;
	
	PATCH_BOOT_APDU_IO7816T1_008();
	
    gIso7816T1_Ctrl.R_block_count = 0;
    if (gIso7816T1_Ctrl.txState != PH_ISO7816T1_TXSUB_READY || gIso7816T1_Ctrl.state != PH_ISO7816T1_STATE_TX )
    {
    	return PH_ISO7816T1_ERROR;
    }

    gIso7816T1_Ctrl.rxState	= PH_ISO7816T1_RXSUB_I_BLOCK;
    gIso7816T1_Ctrl.txState	= PH_ISO7816T1_TXSUB_I;
	iso7816Util_ResetParser();

    /* invoke state machine */
    Iso7816T1_TxStateMachine( T1_BLOCK_NO_ERROR );

    while( gIso7816T1_Ctrl.rxState != PH_ISO7816T1_RXSUB_READY )
    {
        /* receive data */
        if( gIso7816T1_Ctrl.pUartBuffer->fullness == 0 )
        {
			sr_result = ecpUART_RxBuffer( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->size, &gIso7816T1_Ctrl.pUartBuffer->fullness, gIso7816T1_Ctrl.CWT, sysConfig_IsUartEnterSleepMode());

			if(PHDD_UART_OK != sr_result)
			{
				if(sr_result == PHDD_UART_ERROR)
				{
					gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_TX;
					if(gIso7816T1_Ctrl.R_block_count >= 2)
					{
						gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_EXPECT_R_ACK ;
						gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
						continue;
					}
					Iso7816T1_TxStateMachine(T1_BLOCK_CRC_OR_LCR_ERROR);
					U8_INC(gIso7816T1_Ctrl.R_block_count);
					continue;
				}
				else
				{
					gIso7816T1_Ctrl.rxState = PH_ISO7816T1_RXSUB_EXPECT_R_ACK ;
					gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
					continue;
				}
            }
        }

        /* set to TX-MODE */
        gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_TX;
        Iso7816T1_DecodeBlock();
    }

    gIso7816T1_Ctrl.state = PH_ISO7816T1_STATE_RX;

    return 0;
}

void iso7816T1_SendStatus(void)
{
	iso7816T1_Transmit();
	return;
}

void iso7816T1_RecieveCommand(void)
{
	iso7816T1_Receive();
	return;
}

void iso7816T1_AnswerPps(u8 ppsResult, u8 bCount)
{
	if(T1_PPS_SUCESS == ppsResult)
	{
		gIso7816T1_Ctrl.pUartBuffer->fullness	= bCount;
	}
	else
	{
		/* prepare ifs request */
		gIso7816T1_Ctrl.pUartBuffer->pData[0] = 0xFF;
		gIso7816T1_Ctrl.pUartBuffer->pData[1] = 0x00;
		gIso7816T1_Ctrl.pUartBuffer->pData[2] = 0xFF;
		gIso7816T1_Ctrl.pUartBuffer->fullness = 3;
	}
	ecpUart_TxBuffer(gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->size, gIso7816T1_Ctrl.pUartBuffer->fullness, PHDD_UART_TX_OPTION_LCT);
}

u8 iso7816T1_GetNad(void)
{
	return gIso7816T1_Ctrl.t1_nad[iso7816Channel_GetCurLogicalChn()];
}
/*============================================================================*/
/**
 * @brief  This function requests a Waiting Time eXtension.
 *
 * @param  none
 *
 * @return none
 *
 * @note   none
 */
/*============================================================================*/
void iso7816T1_RequestWtx( void )
{
	PATCH_BOOT_APDU_IO7816T1_009();
	
    /* generate wtx request */
    gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_NAD_OFFSET]	= 0x00;
    gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PCB_OFFSET]	= T1_S_BWT_REQUEST;
    gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_LEN_OFFSET]	= 0x01;
    gIso7816T1_Ctrl.pUartBuffer->pData[HEADER_PAY_OFFSET]	= gIso7816T1_Ctrl.wtxm ;
    gIso7816T1_Ctrl.pUartBuffer->fullness					= HEADER_PAY_OFFSET + 1;

    /* generate LRC */
	if(gIso7816T1_Ctrl.T1CheckSumType == 0)
	{
	    gIso7816T1_Ctrl.pUartBuffer->pData[ gIso7816T1_Ctrl.pUartBuffer->fullness ] =
	    Iso7816T1_CalculateLrc( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->fullness );
	    U16_INC(gIso7816T1_Ctrl.pUartBuffer->fullness);
	}
	else
	{
		PX_W_U16( &gIso7816T1_Ctrl.pUartBuffer->pData[ gIso7816T1_Ctrl.pUartBuffer->fullness ] ,          \
	    Iso7816T1_CalculateCrc( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->fullness ) );
	    gIso7816T1_Ctrl.pUartBuffer->fullness += 2;
	}

    /* send request */
    if(PHDD_UART_OK == ecpUart_TxBuffer( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->size, gIso7816T1_Ctrl.pUartBuffer->fullness, PHDD_UART_TX_OPTION_LCT ))
    {
        gIso7816T1_Ctrl.pUartBuffer->fullness = 0;
    }

    /* receive response */
    if( gIso7816T1_Ctrl.pUartBuffer->fullness == 0 )
    {
        if(PHDD_UART_OK != ecpUART_RxBuffer( gIso7816T1_Ctrl.pUartBuffer->pData, gIso7816T1_Ctrl.pUartBuffer->size, &gIso7816T1_Ctrl.pUartBuffer->fullness, gIso7816T1_Ctrl.CWT, sysConfig_IsUartEnterSleepMode()))
        {
			while(1);	
        }
    }

	return;
}


/*============================================================================*/
/**
 * @brief  This function sets the Waiting Time eXtension Multiplier that is
 *         used for block waiting time out.
 *
 * @param  uint8_t wtxm                                                        \n
 *         The new Waiting Time eXtension Multiplier.
 *
 * @return PH_ISO7816T1_OK,                                                   \n
 *         PHDD_PROTOCOL_ERROR
 *
 * @note   none
 */
/*============================================================================*/
u8 iso7816T1_SetWtxm( u8 wtxm )
{
    /*=======================================================================*/
    /** @par Variables */
    /** @par
     *  none */

    /*=======================================================================*/
    /** @par Processing */
    /** @par
     * The function saves the multiplier and answers with PH_ISO7816T1_OK */

    /* The multiplier has following range 0 < WTXM < ?? */
		
	PATCH_BOOT_APDU_IO7816T1_010();
	
    if ( 0 < wtxm /*<256 && (wtxm * bwtValue) < MAX_bwt*/)
    {
        /* valid range */
        gIso7816T1_Ctrl.wtxm = wtxm;
		return PH_ISO7816T1_OK;
    }
    else
    {
        /* invalid range */
        return PH_ISO7816T1_ERROR;
    }

	
}

u32 iso7816T1_GetT1WtxTimerValue(void)
{
    u32     bwi;
    u32     cwi;
	u8		bwiValue;
	u8      cwiValue;

	PATCH_BOOT_APDU_IO7816T1_011();
	
	bwiValue	= sysConfig_GetUartT1TB();
	cwiValue    = U8_AND_U8(bwiValue , 0x0F);
    
	bwiValue	= U8_AND_U8(U8_SHR_U8(bwiValue, 4), 0x0F);
	
#if HAL__IO__T1_TIME_MODE
	bwi			= U32_SHL_U8(1, bwiValue);
	bwi			= U32_M_U32(sysConfig_GetUartWtx(), U16_M_U16(IoUart_GetF(), bwi));
#else
	bwi 		= U32_M_U32(sysConfig_GetUartWtx(), U8_M_U16(IoUart_GetD(), 372));
	bwi 		= U32_D_U32(bwi, IoUart_GetF()) ;
	bwi 		= U8_A_U32(11, U32_M_U32(bwi, U32_SHL_U8(1, bwiValue)));
#endif
    
	cwi         = U32_SHL_U8(1,cwiValue);
	cwi         = U8_A_U32(11,cwi);

	return ecpDivideT1WtxTimerFrequency(bwi);
}

#endif	//HAL__IO__ISO7816_T1
