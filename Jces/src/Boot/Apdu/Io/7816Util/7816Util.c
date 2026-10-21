#include "7816Util.h"
#include "DataOperation.h"
#include "BootNoAreaInitRam.h"
#include "SysConfig.h"
#include "IoBuffer.h"

#if BOOT__IO__ISO7816

static u8							X_A	curBufPckType;
static ppIso7816Util_BufPck_t		X_A uartBufPck;
static ppIso7816Util_BufPck_t		X_A	clBufPck;

static void iso7816Util_GetLcNoParser( ppIso7816Util_Command_t	command )
{
	command->Lc	= U8t16(command->pData[ISO7816UTIL_P3_OFFSET]);
}

static void iso7816Util_GetLeNoParser( ppIso7816Util_Command_t	command)
{
	u16 					temp_length;
	u16						temp_le;
	u16						temp_lc;

	temp_length		= 	command->length;
	temp_lc			= 	U8t16(command->pData[ISO7816UTIL_P3_OFFSET]);
	temp_length 	=	U16_S_U8(temp_length, ISO7816UTIL_HEADER_LENGTH);
	//case 2:
	if(temp_length == 0)
	{
		if(temp_lc == 0)
		{
			temp_le	= U32t16(MAX_LE);
		}
		else
		{
			temp_le	= temp_lc;
		}
	}
	//case 3
	else
	{
		if(temp_lc == temp_length)
		{
			temp_le = temp_lc;
		}
	//case 4
		else if(temp_lc > temp_length)
		{
			temp_le = U32t16(MAX_LE);
		}
		else
		{
			if(command->pData[U8_A_U16(ISO7816UTIL_HEADER_LENGTH, temp_lc)] == 0x00)
			{
				temp_le = U32t16(MAX_LE);
			}
			else
			{
				temp_le = 	U8t16(command->pData[U8_A_U16(ISO7816UTIL_HEADER_LENGTH, temp_lc)]);
			}
		}
	}

	command->Le	= temp_le;
}

void iso7816Util_Init(void)
{
	//g_current_D		= 1;
	//g_current_F		= 372;

#if BOOT__IO__APDU_EXTENDED_LENGTH
    /*=======================================================================*/
    /** \processing
     ** \processingstep
     *  initialize command structure */
	iso7816Util_command.CLA            = 0xFF;
	iso7816Util_command.INS            = 0xFF;
	iso7816Util_command.P1             = 0;
	iso7816Util_command.P2             = 0;
	iso7816Util_command.haveLc         = false;
	iso7816Util_command.Lc             = 0;
	iso7816Util_command.haveLe         = false;
#endif
    //iso7816Util_command.Le             = 0;
    //iso7816Util_command.Lc             = 0;
    //iso7816Util_command.length         = 0;
    //iso7816Util_command.pData          = l4i_ApduBuffer;
    //iso7816Util_command.size           = l4i_ApduBuffer_Size;

    /** \processingstep
     * initialize response structure */
    //iso7816Util_response.SW1SW2        = PH_ISO7816L4_SW_NONE;
    //iso7816Util_response.pData         = PX8_A_CONST(l4i_ResponseBuffer, PH_ISO7816L4_MAX_PROLOGUE_LENGTH);
    //iso7816Util_response.length        = 0;

    /** \processingstep
     * initialize control structure */
    //iso7816Util_control.dataIndex                     = 0;
#if BOOT__IO__APDU_EXTENDED_LENGTH
    iso7816Util_control.numBytesReceived              = 0;
    iso7816Util_control.numBytesExpected              = 0;
    iso7816Util_control.errorDetected                 = false;
    iso7816Util_control.lastCLA                       = 0xFF;
    iso7816Util_control.lastNumSW1SW2BytesTransmitted = 0;
    iso7816Util_control.lastRespLen                   = 0;
    iso7816Util_control.numBytesToTransmitFromSW1SW2  = 0;
    iso7816Util_control.isRetransmitBlock			  = 0;
    iso7816Util_control.retransmitLength			  = 0;
    iso7816Util_control.retransmitDataIndex			  = 0;
#endif
}

void iso7816Util_InitUartBufPck(void)
{
	uartBufPck					= IoBuffer[UART_INT_BUF_PCK_OFFSET].Uart7816UtilBufPck;
	ecpMem_Set(uartBufPck, 0, sizeof(pIso7816Util_BufPck_t));
	uartBufPck->IoBuffer.pData	= IoBuffer[UART_INT_BUF_PCK_OFFSET].ioCacheBuf.pdata;
	uartBufPck->IoBuffer.size	= IoBuffer[UART_INT_BUF_PCK_OFFSET].ioCacheBuf.size;
	uartBufPck->command.pData	= IoBuffer[UART_INT_BUF_PCK_OFFSET].commandBuf.pdata;
	uartBufPck->command.size	= IoBuffer[UART_INT_BUF_PCK_OFFSET].commandBuf.size;
	uartBufPck->reponse.pData	= IoBuffer[UART_INT_BUF_PCK_OFFSET].responseBuf.pdata;
	ecpMem_Set(uartBufPck->command.pData, 0x00, ISO7816UTIL_HEADER_LENGTH);
	curBufPckType				= UART_BUF_PCK_TYPE;
}


void iso7816Util_InitClBufPck(void)
{
#if HAL__IO__ISO14443_TA
	clBufPck					= IoBuffer[CL_INT_BUF_PCK_OFFSET].Uart7816UtilBufPck;
	ecpMem_Set(clBufPck, 0, sizeof(pIso7816Util_BufPck_t));
	clBufPck->IoBuffer.pData	= IoBuffer[CL_INT_BUF_PCK_OFFSET].ioCacheBuf.pdata;
	clBufPck->IoBuffer.size		= IoBuffer[CL_INT_BUF_PCK_OFFSET].ioCacheBuf.size;
	clBufPck->command.pData		= IoBuffer[CL_INT_BUF_PCK_OFFSET].commandBuf.pdata;
	clBufPck->command.size		= IoBuffer[CL_INT_BUF_PCK_OFFSET].commandBuf.size;
	clBufPck->reponse.pData		= IoBuffer[CL_INT_BUF_PCK_OFFSET].responseBuf.pdata;
	ecpMem_Set(clBufPck->command.pData, 0x00, ISO7816UTIL_HEADER_LENGTH);
	curBufPckType				= CL_BUF_PCK_TYPE;
#endif
}


void iso7816Util_InitBufPck(void)
{
	iso7816Util_InitClBufPck();
	iso7816Util_InitUartBufPck();
}

void iso7816Util_SetBufPckType(u8 BufPckType)
{
	curBufPckType		= BufPckType;
}

u8 iso7816Util_GetBufPckType(void)
{
	return curBufPckType;
}

void iso7816Util_SetBufPck(ppIso7816Util_BufPck_t bufPck, u8 BufPckType)
{
	if(UART_BUF_PCK_TYPE == BufPckType)
	{
		uartBufPck		= bufPck;
	}
	else
	{
		clBufPck		= bufPck;
	}
}

void iso7816Util_GetBufPck(ppIso7816Util_BufPck_t * bufPck, u8 BufPckType)
{
	if(UART_BUF_PCK_TYPE == BufPckType)
	{
		*bufPck 		= uartBufPck;
	}
	else
	{
		*bufPck 		= clBufPck;
	}
}

void iso7816Util_GetIoBuffer( ppIso7816Util_Buffer_t *ppIoBuffer )
{
    /*=======================================================================*/
    /** \processing
     ** \processingstep
     *  assign pointer to response structure to ppResponse */
    if(UART_BUF_PCK_TYPE == curBufPckType)
	{
		*ppIoBuffer		= &uartBufPck->IoBuffer;
	}
	else
	{
		*ppIoBuffer		= &clBufPck->IoBuffer;
	}
}

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_GetCommand() can be called to retrieve
 *         the address of the command structure
 *
 * \param  [out] pphIso7816L4_Command_t * ppCommand                                 \n
 *         pointer to the location to store the address of the command
 *         buffer
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
void iso7816Util_GetCommand( ppIso7816Util_Command_t * ppCommand )
{
    /*=======================================================================*/
    /** \processing
     ** \processingstep
     *  assign pointer to command structure to ppCommand */
    if(UART_BUF_PCK_TYPE == curBufPckType)
	{
		*ppCommand		= &uartBufPck->command;
	}
	else
	{
		*ppCommand		= &clBufPck->command;
	}
}

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_GetResponse() can be called to retrieve
 *         the address of the response structure
 *
 * \param  [out] pphIso7816L4_Command_t * ppCommand                                 \n
 *         pointer to the location to store the address of the command
 *         buffer
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
void iso7816Util_GetResponse( ppIso7816Util_Response_t *ppResponse )
{
    /*=======================================================================*/
    /** \processing
     ** \processingstep
     *  assign pointer to response structure to ppResponse */
    if(UART_BUF_PCK_TYPE == curBufPckType)
	{
		*ppResponse		= &uartBufPck->reponse;
	}
	else
	{
		*ppResponse		= &clBufPck->reponse;
	}
}

void iso7816Util_GetControl( ppIso7816Util_Control_t *ppControl )
{
    /*=======================================================================*/
    /** \processing
     ** \processingstep
     *  assign pointer to response structure to ppResponse */
    if(UART_BUF_PCK_TYPE == curBufPckType)
	{
		*ppControl		= &uartBufPck->control;
	}
	else
	{
		*ppControl		= &clBufPck->control;
	}
}

void iso7816Util_SetCommand(PX8 commandBuf, u16 size, u8 bufPckType)
{
	ppIso7816Util_Command_t			command;
	
	switch(bufPckType)
	{
		case UART_BUF_PCK_TYPE:
			uartBufPck->command.pData	= commandBuf;
			uartBufPck->command.size	= size;
			break;
		case CL_BUF_PCK_TYPE:
			clBufPck->command.pData		= commandBuf;
			clBufPck->command.size		= size;
			break;
		default:
			iso7816Util_GetCommand(&command);
			command->pData				= commandBuf;
			command->size				= size;
			break;
	}
}

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_SetSW1SW2() can be called to set the SW1SW2
 *
 * \param  [in] pphIso7816L4_Command_t * ppCommand                                 \n
 *         pointer to the location to store the address of the command
 *         buffer
 * \param  [in] bool bBypassParser                                                 \n
 *         bypass the ISO7816 parser, copy SW1SW2 to response buffer
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
void iso7816Util_SetResPrameter( u16 resLength, u16 Lr, u16 SW1SW2, u8 flags )
{
	ppIso7816Util_Response_t response;

	iso7816Util_GetResponse(&response);
    if (U8_AND_U8(flags, PH_ISO7816L4_FLAG_BYPASSPARSER))
	{
    	PX_W_U16(&(response->pData[resLength]), SW1SW2);
    	response->lr		= Lr;
    	response->length 	= U32t16(U16_A_U8(resLength, ISO7816UTIL_STATUS_BYTE_LENGTH));
    	response->SW1SW2 	= SW1SW2;
	}
}

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_ParseBytes() parses the given data and
 *         fills the command structure. The function is reentrant.
 *
 * \param  [in] uint8_t * pData                                                   \n
 *         pointer to location in memory where the parser has to be started
 * \param  [in] uint16_t length                                                   \n
 *         number of bytes that have to be parsed
 * \param  [in] bool bBypassParser                                                 \n
 *         bypass the ISO7816 parser, just copy data to command buffer
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
u16 iso7816Util_ParseBytes(PX8 pData, u16 length, u8 flags )
{
    /*=======================================================================*/
    /** \variables
     ** \vardef uint16_t count = length; to be used as loop counter */
    u16						D_A		count;
	u16						D_A		dataIndex;
	ppIso7816Util_Control_t			control;
	ppIso7816Util_Command_t			command;
	ppIso7816Util_Response_t		response;

	iso7816Util_GetControl(&control);
	iso7816Util_GetCommand(&command);
	iso7816Util_GetResponse(&response);

	count = length;

	if(control->isRetransmitBlock)
	{
		dataIndex = control->retransmitDataIndex;
	}
	else
	{
		dataIndex = control->dataIndex;
	}

    /*=======================================================================*/
    //if (bBypassParser == TRUE)
    if (U8_AND_U8(flags, PH_ISO7816L4_FLAG_BYPASSPARSER))
    {
    	if(U8_AND_U8(flags, PH_ISO7816L4_FLAG_T0_REV_CMD))
    	{
    		command->Le		= U32t16(MAX_LE);
			dataIndex		= U32t16(U16_A_U16(dataIndex, length));
    	}
    	else
    	{
			if (U16_A_U16(dataIndex, length) <= command->size)
			{
				while( 0 != count )
				{
					if( 0 == control->numBytesReceived )
					{
						command->length   = 0;
					}
					U16_DEC(count);
					command->pData[dataIndex]	= PX_R_U8(pData);
					U16_INC(dataIndex);
					PX8_INC(pData);
					U16_INC(control->numBytesReceived);
				}
				command->length	= control->numBytesReceived;
				//iso7816Util_response.SW1SW2	= PH_ISO7816L4_SW_NORMAL_PROCESSING;
			}
			else
			{
				//control.numBytesReceived+=length;
				command->length	= control->numBytesReceived;
				//control.errorDetected = true;
				response->SW1SW2	= PH_ISO7816L4_SW_WRONG_LENGTH;
			}

			if(control->numBytesReceived >= 5)
			{
				iso7816Util_GetLeNoParser(command);
				iso7816Util_GetLcNoParser(command);
			}
    	}
    }
#if BOOT__IO__APDU_EXTENDED_LENGTH
    else
    {
    	pIso7816Util_Control_t			iso7816Util_control;
		pIso7816Util_Command_t			iso7816Util_command;
		pIso7816Util_Response_t			iso7816Util_response;

		ecpMem_Copy(&iso7816Util_control, &control, sizeof(iso7816Util_control));
		ecpMem_Copy(&iso7816Util_command, &command, sizeof(iso7816Util_command));
		ecpMem_Copy(&iso7816Util_response, &response, sizeof(iso7816Util_response));

        /*=======================================================================*/
        /** \processing
         ** \processingstep
         *  loop over all bytes */
        while( 0 != count )
        {
            count--;
            switch( iso7816Util_control.numBytesReceived )
            {
            /** \processingstep
             * 1st byte */
            case 0:
                /** \processingstep
                 * set control structure */
            	iso7816Util_control.errorDetected = true;
            	iso7816Util_control.numBytesToTransmitFromSW1SW2  = sizeof( u16 );
            	iso7816Util_control.lastNumSW1SW2BytesTransmitted = 0;

                /** \processingstep
                 * reset the Lc settings */
            	iso7816Util_command.haveLc   = false;
            	iso7816Util_command.Lc       = 0;

                /** \processingstep
                 * reset the Le settings */
            	iso7816Util_command.haveLe   = false;
            	iso7816Util_command.Le       = 0;

            	iso7816Util_command.extended = false;
            	iso7816Util_command.length   = 0;

                /** \processingstep
                 *  1st byte is CLA */
            	iso7816Util_command.CLA = *pData;
            	iso7816Util_command.pData[iso7816Util_control.dataIndex] = *pData;
            	iso7816Util_control.dataIndex++;
                break;

            /** \processingstep
             * 2nd byte */
            case 1:
                /** \processingstep
                 *  2nd byte is INS */
            	iso7816Util_command.INS = *pData;
            	iso7816Util_command.pData[iso7816Util_control.dataIndex] = *pData;
            	iso7816Util_control.dataIndex++;
                break;

            /** \processingstep
             * 3rd byte */
            case 2:
                /** \processingstep
                 *  3rd byte is P1 */
            	iso7816Util_command.P1 = *pData;
            	iso7816Util_command.pData[iso7816Util_control.dataIndex] = *pData;
            	iso7816Util_control.dataIndex++;
                break;

            /** \processingstep
             * 4th byte */
            case 3:
                /** \processingstep
                 *  4th byte is P2 */
            	iso7816Util_command.P2 = *pData;
            	iso7816Util_command.pData[iso7816Util_control.dataIndex] = *pData;
            	iso7816Util_control.dataIndex++;
                if( count == 0 )
                {
                	iso7816Util_control.errorDetected = false;
                }
                break;

            /** \processingstep
             * 5th byte */
            case 4:
                /** \processingstep
                 *  check for standard Le, extended Le, standard Lc or extended Lc */
                /** \processingstep
                 *  case standard Le */
                if( length == PH_ISO7816L4_HEADER_MIN_LENGTH + PH_ISO7816L4_STANDARD_LE_LENGTH )
                {
                	iso7816Util_control.numBytesExpected 			= 0;
                	iso7816Util_command.haveLe           			= true;
                	iso7816Util_command.Le               			= *pData++;
                	iso7816Util_command.pData[iso7816Util_control.dataIndex] 	= *pData;
                	iso7816Util_control.dataIndex++;
                	iso7816Util_control.errorDetected    			= false;
                    break;
                }
                /** \processingstep
                 *  case extended Le, standard Lc or extended Lc */
                else
                {
                    /* extended APDU */
                    if( 0 == *pData )
                    {
                        count--;
                        pData++;
                        iso7816Util_control.numBytesReceived++;
                        iso7816Util_command.extended = true;
                        count--;
                        iso7816Util_command.Lc = ((u16)(*pData++) << 8);
                        iso7816Util_command.pData[iso7816Util_control.dataIndex] 	= *pData;
                        iso7816Util_control.numBytesReceived++;
                        iso7816Util_command.Lc |= *pData;
                        iso7816Util_command.pData[iso7816Util_control.dataIndex] 	= *pData;
                        iso7816Util_control.dataIndex += 2;

                        /* extended Le */
                        if( length == PH_ISO7816L4_HEADER_MIN_LENGTH + PH_ISO7816L4_EXTENDED_LE_LENGTH )
                        {
                        	iso7816Util_control.numBytesExpected = 0;
                        	iso7816Util_command.Le = iso7816Util_command.Lc;
                        	iso7816Util_command.Lc = 0;
                            count = 0;
                            iso7816Util_command.haveLe = true;
                            iso7816Util_control.errorDetected = false;
                            break;
                        }
                    }

                    /* standard APDU */
                    else
                    {
                    	iso7816Util_command.extended = false;
                    	iso7816Util_command.Lc = *pData;
                    	iso7816Util_command.pData[iso7816Util_control.dataIndex] 	= *pData;
                    	iso7816Util_control.dataIndex++;
                    }

                    /* standard or extended Lc */
                    iso7816Util_command.haveLc = true;
                    iso7816Util_control.numBytesExpected = iso7816Util_command.Lc + iso7816Util_control.numBytesReceived +1;
                }
                break;

            /** \processingstep
             * all other bytes */
            default:
                /** \processingstep
                 * handle bytes as payload data */
                if( (iso7816Util_command.haveLc == true) && (iso7816Util_command.Lc <= iso7816Util_command.size) )
                {
                    if( iso7816Util_control.dataIndex < iso7816Util_control.numBytesExpected )
                    {
                        /* APDU data */
                    	iso7816Util_command.pData[iso7816Util_control.dataIndex++] = *pData;

                        /* in case of no Le the count shall be 0 */
                        if( iso7816Util_control.dataIndex == iso7816Util_control.numBytesExpected )
                        {
                            switch( count )
                            {
                            case 0:
                            	iso7816Util_control.errorDetected = false;
                                break;
                            case 1:
                                if( iso7816Util_command.extended == true )
                                {
                                    count = 0;
                                    iso7816Util_control.errorDetected = true;
                                }
                                break;
                            case 2:
                                if( iso7816Util_command.extended == false )
                                {
                                    count = 0;
                                    iso7816Util_control.errorDetected = true;
                                }
                                break;
                            default:
                                count = 0;
                                iso7816Util_control.errorDetected = true;
                            break;
                            }
                        }
                    }
                    else
                    {
                        /* standard Le */
                        /* in case of extended Lc, there must be also an extended Le */
                        if( count == 0 )
                        {
                            /* Le */
                        	iso7816Util_command.Le |= *pData;
                        	iso7816Util_command.haveLe = true;
                        	iso7816Util_control.errorDetected = false;
                        }

                        /* extended Le */
                        else if( (count == 1) && (iso7816Util_command.extended == true) )
                        {
                            /* extended Le */
                        	iso7816Util_command.Le = ((u16)(*pData) << 8);
                        	iso7816Util_control.errorDetected = false;
                        }

                        /* here the rest of bytes shall be only 1 or 0 */
                        /* in case we have more than 1 or 0 here we have a problem */
                        else
                        {
                            /* error */
                            count = 0;
                            iso7816Util_control.errorDetected = true;
                        }
                    }
                }
                else
                {
                    /* error */
                    count = 0;
                    iso7816Util_control.errorDetected = true;
                }
                break;
            }
            pData++;
            iso7816Util_control.numBytesReceived++;
        }

        /** \processingstep
         *  In case an error has been detected set response to PH_ISO7816L4_SW_WRONG_LENGTH.
         *  Otherwise set the response to PH_ISO7816L4_SW_NORMAL_PROCESSING.
         */
        iso7816Util_response.length = 0L;
        iso7816Util_command.length += length;
        if( iso7816Util_control.errorDetected == false )
        {
        	iso7816Util_control.lastCLA = iso7816Util_command.CLA;
        	iso7816Util_response.SW1SW2 = PH_ISO7816L4_SW_NORMAL_PROCESSING;
        }
        else
        {
        	iso7816Util_response.SW1SW2 = PH_ISO7816L4_SW_WRONG_LENGTH;
        }

        if(!iso7816Util_control.isRetransmitBlock)
		{
			iso7816Util_control.dataIndex = dataIndex;
		}

        ecpMem_Copy(&iso7816Util_control, &control, sizeof(iso7816Util_control));
		ecpMem_Copy(&iso7816Util_command, &command, sizeof(iso7816Util_command));
		ecpMem_Copy(&iso7816Util_response, &response, sizeof(iso7816Util_response));

		return iso7816Util_response.SW1SW2;
    }
#endif

	if(!control->isRetransmitBlock)
	{
		control->dataIndex = dataIndex;
	}

    return response->SW1SW2;
}

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_ResetParser() resets the (input) parser
 *         such that a new C-APDU can be received/parsed. Called internally
 *         but might be useful to call from external functions.
 *
 * \param  None
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
void iso7816Util_ResetParser( void )
{
	ppIso7816Util_Control_t			control;

	iso7816Util_GetControl(&control);

    /*=======================================================================*/
    /** \processing
     ** \processingstep
     *  reset control structure */
	control->dataIndex        				= 0;
	control->numBytesReceived				= 0;
	control->numBytesToTransmitFromSW1SW2  	= 2;
	control->isRetransmitBlock 				= 0;
#if BOOT__IO__APDU_EXTENDED_LENGTH
	control.errorDetected    				= false;
	control.numBytesExpected 				= 0;
#endif
}

#if		(BOOT__IO__SWP || HAL__IO__ISO14443_TA || HAL__IO__ISO7816_T1)
/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_GetResponseLength() returns the total
 *         response length.
 *
 * \param  [in] bool bBypassParser \n
 *         bypass the ISO7816 parser
 *
 * \return length of response
 *
 * \note   None
 */
/*===========================================================================*/
u16 iso7816Util_GetResponseLength( u8 flags )
{
	u16								resLength;
	ppIso7816Util_Control_t			control;
	ppIso7816Util_Response_t		response;

	iso7816Util_GetControl(&control);
	iso7816Util_GetResponse(&response);

	resLength = 0x0000;

    if (U8_AND_U8(flags, PH_ISO7816L4_FLAG_BYPASSPARSER))
    {
		if(control->isRetransmitBlock)
		{
			resLength =  control->retransmitLength;
		}
		else
		{
			resLength =  response->length;
		}
    }
    else
    {
#if BOOT__IO__APDU_EXTENDED_LENGTH
        /*=======================================================================*/
        /** \variables
         ** \vardef uint32_t length = 0; to calculate the response length */
        u16 length = 0;
        /*=======================================================================*/

        /*=======================================================================*/
        /** \processing
         ** \processingstep
         * The total ISO response length is the number of bytes in RAM,
         * plus 2 bytes for the SW1SW2.
         * But, there is another aspect to this dictated by the Le byte.
         * We are allowed to send up to Le bytes from the response data field
         * before the status word.
         * If there is more data available, this is then reflected in the status
         * word.
         */
/*
        if( command.haveLe )
        {
            length = response.length;
        }*/
		length = response->length;

        /** \processingstep
         *  return length */
        resLength = length + (u16)control->numBytesToTransmitFromSW1SW2;
#endif
    }
	return resLength;
}

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_ParserHaveResponse() returns true if the
 *         ISO7816-4 parser has a predefined response for the received
 *         APDU.
 *
 * \param  None
 *
 * \return true in case of a predefined response, else false
 *
 * \note   None
 */
/*===========================================================================*/
/*
boolean phIso7816L4_ParserHaveResponse( void )
{
    if( control.errorDetected )
    {
        //=======================================================================
        // \processing
         // \processingstep
         // If the parser has detected an error in the C-APDU, this means we can
        // assemble a response without the APDU handler functions.

        // to be sure we clear the potential left-overs in old answers
        response.length = 0L;
        command.Le      = 0;
        command.haveLe  = false;

        // the status word to be sent is a general error (6F00)
        response.SW1SW2 = PH_ISO7816L4_SW_GENERAL_ERROR;
        return true;
   }

    // \processingstep
     //  Else check whether this can be a GET RESPONSE command (remember, it has to
     //  carry the same CLS byte as the last command).
     //  INS = 0xC0
    if( PH_ISO7816L4_INS_GET_RESPONSE == command.INS )
    {
        // P1 and P2 must be zero
        if( PH_ISO7816L4_P1_GET_RESPONSE == command.P1 )
        {
            if( PH_ISO7816L4_P2_GET_RESPONSE == command.P2 )
            {
                // final check, we require an Le byte and don't allow Lc
                if( !command.haveLc && command.haveLe )
                {
                    if( command.CLA != control.lastCLA )
                    {
                        // hmm, difficult, without checking the class byte the response
                         // is UNKNOWN INSTRUCTION when GET RESPONSE is issued as first command.
                         // I think the best is to send a CONDITIONS_OF_USE_NOT_SATISFIED.
                        response.length = 0L;
                        response.SW1SW2 = PH_ISO7816L4_SW_CONDITIONS_OF_USE_NOT_SATISFIED;
                    }

                    // perfect. we have a response to send.
                    // the parser didn't touch the response pointer or length,
                     // so we can just indicate that we already have a response
                     // and let the transport layers do their stuff.
                    return true;
                }
            }
        }
    }

    // ok, here we have to clear all response data from previous commands
    response.SW1SW2  = PH_ISO7816L4_SW_NONE;
    response.length  = 0L;
    control.lastCLA  = command.CLA;

    return false;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso7816L4
 *
 * \brief  The function phIso7816L4_SetResponse() fetches the response.
 *
 * \param  [out] uint8_t * pResponseData                                           \n
 *         destination pointer for the response data
 * \param  [in] uint16_t numBytes                                                 \n
 *         number of response bytes to be set
 * \param  [in] bool bBypassParser                                                 \n
 *         bypass the ISO7816 parser, just copy data from response buffer
 *
 * \return None
 *
 * \note   Make sure the length parameter is correctly set to the size of
 *         the comm buffer or the number of remaining bytes in order to make
 *         the protocol correct.
 */
/*===========================================================================*/
void iso7816Util_SetResponseData(PX8 pResponseData, u16 numBytes, u8 flags )
{
	ppIso7816Util_Control_t			control;
	ppIso7816Util_Response_t		response;

	iso7816Util_GetControl(&control);
	iso7816Util_GetResponse(&response);

    /*=======================================================================*/
    /** \processing
     ** \processingstep
     * In case bBypassParser is TRUE, just copy response to pResponseData */
    if (U8_AND_U8(flags, PH_ISO7816L4_FLAG_BYPASSPARSER))
    {
        /*=======================================================================*/
        /** \variables
         ** \vardef uint16_t numBytesToCopy number of response bytes to copy */
    	u16   D_A   numBytesToCopy;

		if(control->isRetransmitBlock)
		{
			ecpMem_Copy( pResponseData, &response->pData[control->retransmitDataIndex], control->retransmitLength );
		 	return;
		}

		control->retransmitDataIndex = control->dataIndex;

        /* copy length bytes to the response buffer */
        if (numBytes <= response->length)
        {
            numBytesToCopy 		= numBytes;
            ecpMem_Copy( pResponseData, &response->pData[control->dataIndex], numBytesToCopy );
            response->length		= U16_S_U16(response->length, numBytes);
            control->dataIndex	= U32t16(U16_A_U16(control->dataIndex, numBytesToCopy));
        }
        else
        {
            numBytesToCopy		= response->length;
            ecpMem_Copy( pResponseData, &response->pData[control->dataIndex], numBytesToCopy );
            control->dataIndex = 0;
        }

        control->retransmitLength = numBytesToCopy;
    }
#if BOOT__IO__APDU_EXTENDED_LENGTH
    else
    {

    	pIso7816Util_Control_t			iso7816Util_control;
		pIso7816Util_Command_t			iso7816Util_command;
		pIso7816Util_Response_t			iso7816Util_response;

        /*=======================================================================*/
        /** \variables
         ** \vardef length */
        u16    D_A   length = 0;

        ecpMem_Copy(&iso7816Util_control, &control, sizeof(iso7816Util_control));
		ecpMem_Copy(&iso7816Util_command, &command, sizeof(iso7816Util_command));
		ecpMem_Copy(&iso7816Util_response, &response, sizeof(iso7816Util_response));
        /*=======================================================================*/

        /*=======================================================================*/
        /** \processing
         ** \processingstep
         * Else start processing the APDU: */

        /** \processingstep
         *  Since the last C-APDU was fully parsed now the parser state can be reset */
        //phIso7816L4_ResetParser();

        /** \processingstep
         * integrity check with Le */
        if( iso7816Util_command.haveLe == true )
        {
            if( 0 == iso7816Util_command.Le )
            {
                if( iso7816Util_command.extended == true )
                {
                    length = 65535;
                }
                else
                {
                    length = 256;
                }
            }
            else
            {

         //       length = command.Le;
				length = iso7816Util_response.length;
            }
        }
        else
        {
            //length = 0;
			length = iso7816Util_response.length;
        }

        /** \processingstep
         * boundary check */
        if( length > iso7816Util_response.length )
        {
            length = iso7816Util_response.length;
        }

        /** \processingstep
         * data filling */
        if( 0 != length )
        {
            /* check if response incl. SW1SW2 fits into the destination buffer */
            if ( numBytes >= (length + PH_ISO7816L4_SW_LENGTH) )
            {
                /* response incl. SW1SW2 fits into destination */
                numBytes -= length;

                /* copy length bytes to the response buffer */
                pResponseData  = ecpMem_Copy( pResponseData, iso7816Util_response.pData, (int)length );
                pResponseData += length;

                /* last response has been processed completely, re-set response pointer */
            //    response.pData = command.pData + PH_ISO7816L4_MAX_PROLOGUE_LENGTH ;
            }
            else
            {
                /* response incl. SW1SW2 doesn't fit into destination */
                if( length <= numBytes)
                {
                    /* reduce response.length */
                	iso7816Util_response.length -= length;
                    /* reduce numBytes for later check if parts of SW1SW2 fits into destination */
                    numBytes        -= length;
                    /* copy numBytes bytes to the response buffer */
                    pResponseData = ecpMem_Copy( pResponseData, iso7816Util_response.pData, (int)length );
                    /* increase response pointers, so that next call copies the next part of the response */
                    iso7816Util_response.pData += length;
                    pResponseData  += length;
                }
                else
                {
                    /* response longer than numBytes */
                	iso7816Util_response.length -= numBytes;
                    /* copy numBytes bytes to the response buffer */
                    pResponseData = ecpMem_Copy( pResponseData, iso7816Util_response.pData, (int)numBytes );
                    /* increase response pointers, so that next call copies the next part of the response */
                    iso7816Util_response.pData += numBytes;
                    pResponseData  += numBytes;
                    /* set numBytes to 0 to prevent adding of status word */
                    /* status word will be sent with last chunk. */
                    numBytes = 0;
                }
            }
        }

        if( 0 != numBytes ) {
            /* if there is space left, copy the status word */
            if( 1 == numBytes ) {
                /* we need to check which byte of the SW is to be transmitted */
                if( 0 == iso7816Util_control.lastNumSW1SW2BytesTransmitted )
                {
                    /* this is the first byte, send SW1 */
                    *pResponseData = (u8) (0x00FF & (iso7816Util_response.SW1SW2 >> 8));
                    /* remember this */
                    iso7816Util_control.lastNumSW1SW2BytesTransmitted = 1;
                    iso7816Util_control.numBytesToTransmitFromSW1SW2 = 1;
                }
                else if( 1 == iso7816Util_control.lastNumSW1SW2BytesTransmitted )
                {
                    /* we already sent SW1, only SW2 is required */
                    *pResponseData = (u8) (0x00FF & iso7816Util_response.SW1SW2);
                    /* remember this */
                    iso7816Util_control.lastNumSW1SW2BytesTransmitted = 2;
                    iso7816Util_control.numBytesToTransmitFromSW1SW2  = 0;
                }
                else
                {
                    /* as long as there are only two bytes... */
                }
            }
            else
            {
                /* ok, copy the SW1SW2 */
                *pResponseData   = (u8) (0x00FF & (iso7816Util_response.SW1SW2 >> 8));
                *(pResponseData+1) = (u8) (0x00FF & iso7816Util_response.SW1SW2);
                /* this of course adds to the number of sent bytes... */
                iso7816Util_control.lastNumSW1SW2BytesTransmitted = 2;
                iso7816Util_control.numBytesToTransmitFromSW1SW2  = 0;
            }
            /* NOTE: if the callee didn't check the available response length the
             * length parameter might be > 2. We don't care about this and don't touch the rest
             * of the buffer.
             */
        }

        ecpMem_Copy(&iso7816Util_control, &control, sizeof(iso7816Util_control));
		ecpMem_Copy(&iso7816Util_command, &command, sizeof(iso7816Util_command));
		ecpMem_Copy(&iso7816Util_response, &response, sizeof(iso7816Util_response));
    }
#endif
	return;
}
#endif

#endif //BOOT__IO__ISO7816
