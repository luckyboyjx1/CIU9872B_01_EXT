#include "Io_14443TaL4.h"
#include "BootRam.h"
#include "MifareHal.h"
#include "Mifare.h"
#include "boot.h"

TYPE_JUP_BUF 											_deselect_env;
void goto_deselect_handler(u8 flag)
{
	longjmp(_deselect_env, flag);
}


#if HAL__IO__ISO14443_TA

#pragma IO_14443TA_L4_RAM_START

static phIso14443L4_Control_t								iso14443L4_Control;

IO_14443TA_L4_RAM_END

/*===========================================================================*
 * Defines:
 *===========================================================================*/
#define MIN(a,b)    ((a)<(b)?(a):(b))       /**< macro to calculate minimum */

/*#define PH_ISO14443L4_MEMCPY( d, s, n )     memcpy( d, s, n )			*/

#define SYMMETRIC_BAUD_RATES_ONLY                   0x80    /**< refer to ISO/IEC 14443-4 5.2.4 TA(1) b8 */
#define PPS_REQUEST_MASK                            0xF0    /**< refer to ISO/IEC 14443-4 5.3.1 PPSS start byte */
#define PPS_REQUEST                                 0xD0    /**< refer to ISO/IEC 14443-4 5.3.1 PPSS start byte */
#define PPS_CID_MASK                                0x0F    /**< refer to ISO/IEC 14443-4 5.3.1 PPSS start byte */
#define PPS1_PRESENT                                0x11    /**< refer to ISO/IEC 14443-4 5.3.2 PPSS Parameter 0 */
#define PPS1_RFU_MASK                               0xF0    /**< refer to ISO/IEC 14443-4 5.3.1 PPSS Parameter 1 */
#define BLOCK_NUMBER_MASK                           0x01    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define CID_BIT                                     0x08    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define CID_MASK                                    0x0F    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define CID_MAX                                     0x0E    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define ALL_S_BLOCKS_MASK                           0xC7    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define ALL_S_BLOCKS                                0xC2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define WTX                                         0xF2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define INVALID_ATS                                 ((u8 *) 0xFACE)  /**< init value for ATS in instance structure */
#define R_BLOCK_SIZE                                4       /**< maximum size of an R-block (PCB, CID, CRC, CRC) */

/* see 7.5.3.2, PICC rules */
/* NOTE 1 The PICC may check if the received block number is not in compliance with PCD rules to decide neither to
          toggle its internal block number nor to send a response block. */
#define PH_ISO14443L4_BLOCK_NUMBER_CHECK_ENABLED    1       /**< set to 1 to check block number of received I-blocks. According to ISO/IEC 14443-4 this is optional */

/*===========================================================================*
 * Types:
 *===========================================================================*/

#define FLAG_CMD_RECEIVED       0x01
#define FLAG_FIRST_CMD          0x02
#define FLAG_SND_DESEL_RESP     0x04
/*#define FLAG_DESELECT_PENDING   0x08		*/

/*===========================================================================*
 * Local data:
 *===========================================================================*/

/** \brief 14443-4 FSDI table constants */
static u16 const iso14443TaL4_fsdiTable[16] =
{
    MIN(  16, PH_ISO14443L4_BUFFER_SIZE ),
    MIN(  24, PH_ISO14443L4_BUFFER_SIZE ),
    MIN(  32, PH_ISO14443L4_BUFFER_SIZE ),
    MIN(  40, PH_ISO14443L4_BUFFER_SIZE ),
    MIN(  48, PH_ISO14443L4_BUFFER_SIZE ),
    MIN(  64, PH_ISO14443L4_BUFFER_SIZE ),
    MIN(  96, PH_ISO14443L4_BUFFER_SIZE ),
    MIN( 128, PH_ISO14443L4_BUFFER_SIZE ),
    MIN( 256, PH_ISO14443L4_BUFFER_SIZE ),
    PH_ISO14443L4_BUFFER_SIZE,
    PH_ISO14443L4_BUFFER_SIZE,
    PH_ISO14443L4_BUFFER_SIZE,
    PH_ISO14443L4_BUFFER_SIZE,
    PH_ISO14443L4_BUFFER_SIZE,
    PH_ISO14443L4_BUFFER_SIZE,
    PH_ISO14443L4_BUFFER_SIZE
};

/*===========================================================================*
 * Local functions:
 *===========================================================================*/
/*===========================================================================*/
/**
 * \ingroup phIso14443L4_InitBuffer
 *
 * \brief  This function initilaizes the
 *         ISO 14443 layer 4 T=CL protocol component.
 *
 * \param[in]  pphBuffer_t pRxTxBuffer
 *                          pointer to the exchange buffer structure
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static void iso14443TaL4_InitBuffer(void)
{
	ppIso7816Util_Buffer_t		cl4i_pRxTxBuffer;
	
	PATCH_BOOT_APDU_IO14443TAL4_001();
	
	iso7816Util_GetIoBuffer((ppIso7816Util_Buffer_t *)&cl4i_pRxTxBuffer);
    /* for the CIU the buffer is organized in the following way:
        0123456789
       |-------------------------------...------------------------------------|
        |    |
        |    |
        |    |
        |    TransmitBuffer
        ReceiveBuffer
     */

    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Init instance structure. */
    //iso14443L4_Control.commandReceived = true; /* forces reset */
    iso14443L4_Control.pCiuBuffer				= cl4i_pRxTxBuffer;
    iso14443L4_Control.pPrologue				= &cl4i_pRxTxBuffer->pData[R_BLOCK_SIZE];
    iso14443L4_Control.pInformation				= &cl4i_pRxTxBuffer->pData[(PH_ISO14443L4_PROLOGUE_SIZE + R_BLOCK_SIZE)];   /* add length of prologue */
    iso14443L4_Control.pDataRx 					= cl4i_pRxTxBuffer->pData;
    iso14443L4_Control.pDataTx 					= &cl4i_pRxTxBuffer->pData[(PH_ISO14443L4_PROLOGUE_SIZE + R_BLOCK_SIZE)];
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function initilaizes the
 *         ISO 14443 layer 4 T=CL protocol component.
 *
 * \param[in]  pphBuffer_t pRxTxBuffer
 *                          pointer to the exchange buffer structure
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static void iso14443TaL4_Init( void )
{
    /* for the CIU the buffer is organized in the following way:
        0123456789
       |-------------------------------...------------------------------------|
        |    |
        |    |
        |    |
        |    TransmitBuffer
        ReceiveBuffer
     */

    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Init instance structure. */
    //iso14443L4_Control.commandReceived = true; /* forces reset */
    //iso14443L4_Control.state					= PH_ISO14443L3_IDLE_STATE;
    iso14443L4_Control.pAts						= INVALID_ATS;
    iso14443L4_Control.sak						= 0x20;
    iso14443L4_Control.baudrate					= 0x00; 									/* set baudrate default to 106kbit/s */
    iso14443L4_Control.ppsEnable				= true; 									/* enable PPS request */
    iso14443L4_Control.wtxEnable				= true; 									/* enable WTX request */
    iso14443L4_Control.blockNumber				= 1;    									/* reset block number */
    //iso14443L4_Control.cid						= 0;    									/* reset CID */
    iso14443L4_Control.cidReceived				= 0;    									/* no CID received so far */
    iso14443L4_Control.nad						= 0;    									/* reset NAD */
    iso14443L4_Control.fullness					= 0;    									/* buffer fullness */
    iso14443L4_Control.length					= 0;    									/* buffer size */
    iso14443L4_Control.fsci						= 0;    									/* FSCI */
    //iso14443L4_Control.fsdi						= 0;    									/* FSDI */
    iso14443L4_Control.pcb						= 0;    									/* protocol iso14443L4_Control byte */
    iso14443L4_Control.bytesPending    			= 0;                                    /* is data pending in CIU buffer ? */
    iso14443L4_Control.flags           			= FLAG_FIRST_CMD | FLAG_SND_DESEL_RESP | FLAG_CMD_RECEIVED;
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function performs a check of the CID on the received block.
 *         It returns the length of the received prologue field.
 *
 * \param[in,out]  pphBuffer_t pBuffer\n
 *         pointer to the buffer that contains the received prologue field
 *         to be checked
 *
 * \return length of the received prologue field \n
 *         2 in case of block contains CID and CID matched the card CID \n
 *         1 in case of block contains no CID and card CID is 0 \n
 *         0 in case of unknown CID \n
 *         -1 in case of other errors \n
 *
 * \note   None
 */
/*===========================================================================*/
static u8 iso14443TaL4_CheckPrologue( ppIso7816Util_Buffer_t pBuffer )    SMALL_MODE
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Check if block contains CID. */
	
	PATCH_BOOT_APDU_IO14443TAL4_002();
	
    if( U8_AND_U8(pBuffer->pData[0], CID_BIT) )
    {
        /** \processingstep
         * If the block contains a CID, check if at least two bytes have been
         * received and the CID is valid. Then adjust data pointer and block
         * length accordingly and return the correct prologue length. */
        iso14443L4_Control.cidReceived = 1;
        if( (pBuffer->fullness == 1) || ( U8_AND_U8(pBuffer->pData[1], CID_MASK) > CID_MAX) )
        {
            /* indicate an invalid block */
			if(BOOT_CONFIG_TA_PROLOGUE_UNSIGNED_VALUE_ON)//Be default configuration: switch off
			{
				return 0x00;
			}
			else
			{
				return U16t8(-1);
			}
        }
        /* check if the received CID matches the current card CID */
        if( iso14443L4_Control.cid != U8_AND_U8(pBuffer->pData[1], CID_MASK) )
        {
            return 0;
        }
        /* move pointer to information field */
		pBuffer->pData		= PX8_A_CONST(pBuffer->pData, 2);
		pBuffer->fullness	= U16_S_U8(pBuffer->fullness, 2);
        /* return length of prologue flag */
        return 2;
    }
    else
    {
        /** \processingstep
         * If the block does not contain a CID, check if the card CID is 0.
         * Then adjust data pointer and block length accordingly and return
         * the correct prologue length. */

        /* no CID has been received */
        iso14443L4_Control.cidReceived = 0;
        /* check if the card CID is 0 */
        if( iso14443L4_Control.cid != 0 )
        {
            return 0;
        }
        /* move pointer to information field */
		pBuffer->pData		= PX8_A_CONST(pBuffer->pData, 1);
		pBuffer->fullness	= U16_S_U8(pBuffer->fullness, 1);
        /* return length of prologue flag */
        return 1;
    }
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sends a TCL block.
 *
 * \param[in]  u8 pcb\n
 *                      The protocol iso14443L4_Control byte.
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static void iso14443TaL4_SendBlock( u8 pcb )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  u8 *pPrologueBackup - Stores backup of prologue. */
    PX8  	X_A pPrologueBackup;
    /** \vardef  u8 *pDataBackup - Stores backup of data pointer. */
    PX8  	X_A pDataBackup;
	u8		X_A flag;

    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Init local variables. */
	
	PATCH_BOOT_APDU_IO14443TAL4_003();
	
    iso14443L4_Control.length = iso14443L4_Control.pCiuBuffer->fullness;
    iso14443L4_Control.pcb    = pcb;

    /** \processingstep
     * If the PCB does not indicate an S-BLOCK, insert block number. */
    if( U8_AND_U8(pcb, ALL_S_BLOCKS_MASK) != ALL_S_BLOCKS )
    {
		pcb = U8_OR_U8(pcb, iso14443L4_Control.blockNumber);
    }
	
	flag					 = (PHDD_CIU_CRC_ON | PHDD_CIU_USE_CM);
	if(PH_ISO14443L4_DESELECT	== pcb)
	{
		flag				|= PHDD_CIU_RESPONSE_IS_DESELECT_ON;
	}
	if(WTX	== pcb)
	{
		flag				|= PHDD_CIU_RESPONSE_IS_WTX_ON;
	}

    /** \processingstep
     * Backup the prologue location for restoring it later. */
    pPrologueBackup = MEMtPX8(iso14443L4_Control.pPrologue);

    /** \processingstep
     * Backup the data location for restoring it later. */
    pDataBackup		= MEMtPX8(iso14443L4_Control.pCiuBuffer->pData);

    /** \processingstep
     * Get the position of prologue before data. */
    iso14443L4_Control.pPrologue	= PX8_S_CONST(iso14443L4_Control.pCiuBuffer->pData, 2);

    if( iso14443L4_Control.cid != 0 || iso14443L4_Control.cidReceived )
    {
        /** \processingstep
         * In case a CID has been received or the CIS is not 0, send also the CID. */
		pcb										= U8_OR_U8(pcb, CID_BIT);
        iso14443L4_Control.pPrologue[0]			= pcb;
        iso14443L4_Control.pPrologue[1]			= iso14443L4_Control.cid;
        iso14443L4_Control.pCiuBuffer->pData	= MEMtPX8(&iso14443L4_Control.pPrologue[0]);
		iso14443L4_Control.pCiuBuffer->fullness	= U8_A_U16(2, iso14443L4_Control.pCiuBuffer->fullness);
    }
    else
    {
        /** \processingstep
         * Else just send the pcb. */
        iso14443L4_Control.pPrologue[1]			= pcb;
        iso14443L4_Control.pCiuBuffer->pData	= MEMtPX8(&iso14443L4_Control.pPrologue[1]);
		iso14443L4_Control.pCiuBuffer->fullness = U8_A_U16(1, iso14443L4_Control.pCiuBuffer->fullness);
    }

    /** \processingstep
     * Restore Prologue buffer location. */
    iso14443L4_Control.pPrologue 				= pPrologueBackup;


    /** \processingstep
     * Transmit buffer. */
    ecpCiu_TxBuffer( iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->fullness, flag );

    /** \processingstep
     * Restore data buffer location. */
    iso14443L4_Control.pCiuBuffer->pData		= MEMtPX8(pDataBackup);
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function evaluates the received PPS request
 *         and configures the new baudrate settings.
 *
 * \param  None
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static void iso14443TaL4_PpsRequest( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  pphIso14443L4_PPS_t pPPS - Pointer to location of PPS in received buffer. */
    pphIso14443L4_PPS_t         pPPS;
    /** \vardef  u8 tmp - Temporarily stores TA(1) byte, also used for baudrate calculation. */
    u8                     		tmp;
	u8							flag;

	PATCH_BOOT_APDU_IO14443TAL4_004();
	
	flag = PHDD_CIU_CRC_ON | PHDD_CIU_USE_CM ;
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * check block length. */
    if( (iso14443L4_Control.pCiuBuffer->fullness != 3) &&
        (iso14443L4_Control.pCiuBuffer->fullness != 2) )
    {
        return;
    }

    /** \processingstep
     * check card CID. */
    pPPS = (pphIso14443L4_PPS_t)iso14443L4_Control.pCiuBuffer->pData;
    if( (pPPS->PPSS & PPS_CID_MASK) != iso14443L4_Control.cid )
    {
        return;
    }

    /** \processingstep
     * check PPS0. */
    if( pPPS->PPS0 != PPS1_PRESENT )
    {
        iso14443L4_Control.pCiuBuffer->fullness = 1;
        ecpCiu_TxBuffer( iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->fullness, flag );
        return;
    }

    /** \processingstep
     * check PPS1. */
    if( U8_AND_U8(pPPS->PPS1, PPS1_RFU_MASK) != 0 )
    {
        return;
    }

    /** \processingstep
     * check requested new baudrate. */
    tmp = PM_R_U8(&iso14443L4_Control.pAts[1]);
    if( U8_AND_U8(tmp, SYMMETRIC_BAUD_RATES_ONLY) )
    {
        /* check symmetry */
        if( U8_SHR_U8(pPPS->PPS1, 2) != U8_AND_U8(pPPS->PPS1, 3) )
        {
            return;
        }
    }

    tmp	= U8_OR_U8(U8_SHL_U8(tmp, 1), 0x11); /* indicate that 106kbit/s are always supported */
    /*
     * Now bit 2^0 indicates 106 kbit/s receiving supported,
     *     bit 2^1 indicates 212 kbit/s receiving supported,
     *     bit 2^2 indicates 424 kbit/s receiving supported,
     *     bit 2^3 indicates 848 kbit/s receiving supported,
     *     bit 2^4 indicates 106 kbit/s transmitting supported,
     *     bit 2^5 indicates 212 kbit/s transmitting supported,
     *     bit 2^6 indicates 424 kbit/s transmitting supported,
     *     bit 2^7 indicates 848 kbit/s transmitting supported,
     */

    /* check DRI */
    if( U8_AND_U8( U8_SHL_U8(1, U8_AND_U8(pPPS->PPS1, 0x03)), tmp ) == 0 )
    {
        return;
    }

    /* check DSI */
    if( U8_AND_U8( U8_SHL_U8(1, U8_SHR_U8(pPPS->PPS1, 2)), U8_SHR_U8(tmp, 4) ) == 0 )
    {
        return;
    }

    /** \processingstep
     * calculate parameter for SetBaudrate function */
	tmp = U8_AND_U8(U8_SHL_U8( pPPS->PPS1, 2 ),  0x30);
	tmp	= U8_OR_U8(tmp, U8_AND_U8(pPPS->PPS1, 0x03));

    /** \processingstep
     * store baudrate in iso14443L4_Control structure */
    iso14443L4_Control.baudrate	= U8_AND_U8(pPPS->PPS1, 0x0F);

    /** \processingstep
     * acknowledge new baudrate parameters. */
    iso14443L4_Control.pCiuBuffer->fullness = 1;
	flag |= PHDD_CIU_RESPONSE_IS_PPS_ON ;
#if WITH_RF_SETBAUTRATE_FIRST	
	/** \processingstep
     * set new baudrate. */
    ecpCiu_SetBaudrate( tmp, &iso14443L4_Control.baudrate );
	
    ecpCiu_TxBuffer( iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->fullness, flag );
		
#else		
    ecpCiu_TxBuffer( iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->fullness, flag );	
		/** \processingstep
     * set new baudrate. */
    ecpCiu_SetBaudrate( tmp, &iso14443L4_Control.baudrate );

#endif

}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sends a S(DESELECT) response and enters the HALT state.
 *
 * \param  None
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static void iso14443TaL4_Deselect( void )
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Send S(DESELECT) response. */
	
	PATCH_BOOT_APDU_IO14443TAL4_005();
	
    iso14443L4_Control.pCiuBuffer->fullness = 0;

    if (U8_AND_U8(iso14443L4_Control.flags, FLAG_SND_DESEL_RESP))
    {
        iso14443TaL4_SendBlock( PH_ISO14443L4_DESELECT );
    }

    /** \processingstep
     * Re-set buffer pointer. */
    iso14443L4_Control.pCiuBuffer->pData	= MEMtPX8(iso14443L4_Control.pDataRx);

    /** \processingstep
     * Enter HALT state. */
    iso14443L4_Control.L3State				= iso14443TaL3_Deselect();
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function handles a received I block.
 *
 * \param[in]   u8 pcb\n
 *                  protocol iso14443L4_Control byte
 *
 * \param[in]   u8 flags\n
 *                   or'able flags that influence the behaviour of the
 *                   function. \n
 *              PH_ISO14443L4_FLAG_BYPASSPARSER\n
 *                   The 7816 command parser is bypassed.\n
 *              PH_ISO14443L4_FLAG_EXTCHAINING\n
 *                   Enables extended (manual) chaining.\n
 *              PH_ISO14443L4_FLAG_NONE\n
 *                   normal operation.
 *
 * \return PH_ISO14443L4_ACTIVE     in case the received block shall be ignored
 *                                  or chaining is ongoing.\n
 *         PH_ISO14443L4_ACTIVE_FULLBUF in case chaining is ongoing and the
 *                                  reception buffer is full.\n
 *         PH_ISO14443L4_OK         in case a valid command has been received.
 *
 * \note   None
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_I_Block( u8 pcb, u8 flags )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  u16 retVal - Used to store return values of functions called in here. */
    u16                     D_A		retVal;

	PATCH_BOOT_APDU_IO14443TAL4_006();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Check block number. */
   if( U8_AND_U8(pcb, BLOCK_NUMBER_MASK) == iso14443L4_Control.blockNumber )
    {
		if(!sysConfig_TaIblockNeedNoCheck())
		{
			return PH_ISO14443L4_ACTIVE;
		}
    }

    /** \processingstep
     * Check for NAD byte. */
    if( U8_AND_U8(pcb, PH_ISO14443L4_NAD_BIT) != 0 )
    {
        return PH_ISO14443L4_ACTIVE;
    }

    /* re-set data pending flag */
    iso14443L4_Control.bytesPending = 0;

    /** \processingstep
     * Parse and copy received data to the application buffer. */
    retVal = iso7816Util_ParseBytes( iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->fullness, flags );

    /* Check if command buffer size */
    /* In case the next chunk of data fits into the command buffer, we can send */
    /* an R(ACK) and continue receiving data. Else we have to give iso14443L4_Control to the */
    /* application because the data can not be chained internally in the command buffer. */
    if ( U8_AND_U8(flags, PH_ISO14443L4_FLAG_EXTCHAINING) )
    {
        if ( retVal == PH_ISO7816L4_SW_WRONG_LENGTH )
        {
            /* no space left in command buffer */
            /* give iso14443L4_Control back to application to handle data */
			iso14443L4_Control.flags				= U8_AND_U8(iso14443L4_Control.flags, ~FLAG_CMD_RECEIVED);
            iso14443L4_Control.bytesPending        	= iso14443L4_Control.pCiuBuffer->fullness;    /* data pending in CIU buffer */
            if( U8_AND_U8(iso14443L4_Control.pDataRx[0], CID_BIT) )
            {
				iso14443L4_Control.bytesPending	= U8_A_U16(2, iso14443L4_Control.bytesPending);
            }
            else
            {
				iso14443L4_Control.bytesPending	= U8_A_U16(1, iso14443L4_Control.bytesPending);
            }
            iso14443L4_Control.pCiuBuffer->fullness = 0;
            return PH_ISO14443L4_ACTIVE_FULLBUF;
        }
    }

    /** \processingstep
     * toggle block number (7.5.3.2, Rule D). */
	iso14443L4_Control.blockNumber = U8_XOR_U8(iso14443L4_Control.blockNumber, 1);

    /** \processingstep
     * check for chaining. */
    if( U8_AND_U8(pcb, PH_ISO14443L4_CHAINING_BIT) )
    {
        /** \processingstep
         * In case of chaining, acknowledge with R(ACK). */
        iso14443L4_Control.pCiuBuffer->fullness = 0;
        iso14443TaL4_SendBlock( PH_ISO14443L4_R_BLOCK );
        return PH_ISO14443L4_ACTIVE;
    }

	iso14443L4_Control.flags = U8_OR_U8(iso14443L4_Control.flags, FLAG_CMD_RECEIVED);

    return PH_ISO14443L4_OK;
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function handles a received R block.
 *
 * \param[in]  u8 pcb\n
 *              protocol iso14443L4_Control byte
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_R_Block( u8 pcb )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  u16 numBytes - Frame size in bytes. */
    u16                     D_A		numBytes;
	ppIso7816Util_Response_t ppResponse;
	ppIso7816Util_Control_t ppControl;

	PATCH_BOOT_APDU_IO14443TAL4_007();
	
	iso7816Util_GetResponse( &ppResponse );
	iso7816Util_GetControl( &ppControl );
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Check if received block number equals PICC block number */
    if( U8_AND_U8(pcb, BLOCK_NUMBER_MASK) == iso14443L4_Control.blockNumber )
    {
        /** \processingstep
         * When an R(ACK) or an R(NAK) block is received, if its block
         * number is equal to the PICC抯 current block number, the last
         * block shall be re-transmitted.. */
        iso14443L4_Control.pCiuBuffer->pData    = MEMtPX8(iso14443L4_Control.pDataTx);
        iso14443L4_Control.pCiuBuffer->fullness = iso14443L4_Control.length;

        /** \processingstep
         * Calculate maximum size of payload */
        if( iso14443L4_Control.cid != 0 || iso14443L4_Control.cidReceived )
        {
            /* reserve space for PCB, CID and CRC16 */
			numBytes = U16_S_U8(iso14443TaL4_fsdiTable[iso14443L4_Control.fsdi], 4);
        }
        else
        {
            /* reserve space for PCB and CRC16 */
			numBytes = U16_S_U8(iso14443TaL4_fsdiTable[iso14443L4_Control.fsdi], 3);
        }
        if( iso14443L4_Control.pCiuBuffer->fullness > numBytes )
        {
            iso14443L4_Control.pCiuBuffer->fullness = numBytes;
        }
				
		ecpMem_Copy( iso14443L4_Control.pCiuBuffer->pData, &ppResponse->pData[ppControl->retransmitDataIndex], iso14443L4_Control.pCiuBuffer->fullness );
        
		iso14443TaL4_SendBlock( iso14443L4_Control.pcb );

        return PH_ISO14443L4_OK;
    }
    else
    {
        if ( 0 == U8_AND_U8(pcb, PH_ISO14443L4_NAK_BIT ))
        {
            /** \processingstep
             * When an R(ACK) block with a block number not equal to the
             * current PICC抯 block number is received, the PICC shall toggle
             * its block number before sending a block (Rule E 7.5.3.2). */
			iso14443L4_Control.blockNumber 	= U8_XOR_U8(iso14443L4_Control.blockNumber, 1);
        }
        else
        {
            /** \processingstep
             * When an R(NAK) block is received, if its block number is not
             * equal to the PICC抯 current block number, an R(ACK) block shall be sent
             * and the block number is not toggled(Rule 12 7.5.4.3). */
            /* Note: Sending of R(ACK) happens below... */
        }

        if (iso14443L4_Control.pCiuBuffer->pData > iso14443L4_Control.pDataTx)
        {
            iso14443L4_Control.pDataTx = iso14443L4_Control.pCiuBuffer->pData;
        }
    }
    if( pcb & PH_ISO14443L4_NAK_BIT )
    {
        /* send R(ACK) */
        iso14443L4_Control.pCiuBuffer->fullness = 0;
        iso14443TaL4_SendBlock( PH_ISO14443L4_R_BLOCK );
        if (iso14443L4_Control.pCiuBuffer->pData > iso14443L4_Control.pDataTx)
        {
            iso14443L4_Control.pDataTx = iso14443L4_Control.pCiuBuffer->pData;
        }
        return PH_ISO14443L4_OK;
    }
	iso14443L4_Control.flags = U8_OR_U8(iso14443L4_Control.flags, FLAG_CMD_RECEIVED);

    return PH_ISO14443L4_ACTIVE;
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function decodes the received block.
 *
 * \param[in]  u8 flags\n
 *                   or'able flags that influence the behaviour of the
 *                   function. \n
 *             PH_ISO14443L4_FLAG_BYPASSPARSER\n
 *                   The 7816 command parser is bypassed.\n
 *             PH_ISO14443L4_FLAG_EXTCHAINING\n
 *                   Enables extended (manual) chaining.\n
 *             PH_ISO14443L4_FLAG_NONE\n
 *                   normal operation.
 *
 * \return PH_ISO14443L4_ACTIVE     in case the received block shall be ignored
 *                                  or chaining is ongoing.\n
 *         PH_ISO14443L4_ACTIVE_FULLBUF in case chaining is ongoing and the
 *                                  reception buffer is full.\n
 *         PH_ISO14443L4_OK         in case a valid command has been received.
 *         PH_ISO14443L4_ERROR      in case a DESELECT command has been received.
 *
 * \note   None
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_DecodeBlock( u8 flags )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  u8 pcb - Stores the protocol iso14443L4_Control byte. */
    u8                      pcb;
	u8						iBlockMask;
	u8						rBlockMask;
	u8						sBlockMask;
	u8						deselectCode;
	u8						rBlock;

    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Get PCB from received data. */
	
	PATCH_BOOT_APDU_IO14443TAL4_008();
	
    pcb = iso14443L4_Control.pCiuBuffer->pData[0];

    /** \processingstep
     * Check for PPS requestand handle PPS if required. */
    if( U8_AND_U8(pcb, PPS_REQUEST_MASK) == PPS_REQUEST )
    {
        if (U8_AND_U8(iso14443L4_Control.flags, FLAG_FIRST_CMD))
        {
            /* handle PPS command */
            iso14443TaL4_PpsRequest();
        }
        else
        {
            /* Ignore PPSs */
        }
        return PH_ISO14443L4_ACTIVE_PPS;
    }

    /** \processingstep
     * Check prologue of received block. */
    if( 1 > iso14443TaL4_CheckPrologue( iso14443L4_Control.pCiuBuffer ) )
    {
        return PH_ISO14443L4_ACTIVE;
    }

    /** \processingstep
     * Check for I-block and process it if applicable. */
	if(sysConfig_IsTaConfEmvTest())
	{
		iBlockMask		= PH_EMV_I_BLOCK_MASK;
		rBlockMask		= PH_EMV_R_BLOCK_MASK;
		sBlockMask		= PH_EMV_S_BLOCK_MASK;
		rBlock			= PH_EMV_R_BLOCK;
		deselectCode	= PH_EMV_DESELECT;
	}
	else
	{
		iBlockMask		= PH_ISO14443L4_I_BLOCK_MASK;
		rBlockMask		= PH_ISO14443L4_R_BLOCK_MASK;
		sBlockMask		= PH_ISO14443L4_S_BLOCK_MASK;
		rBlock			= PH_ISO14443L4_R_BLOCK;
		deselectCode	= PH_ISO14443L4_DESELECT;
	}
    if( U8_AND_U8(pcb, iBlockMask) == PH_ISO14443L4_I_BLOCK )
    {
        return iso14443TaL4_I_Block( pcb, flags );
    }

    /** \processingstep
     * Check for R-block and process it if applicable. */
    if( (U8_AND_U8(pcb, rBlockMask) == rBlock) && (iso14443L4_Control.pCiuBuffer->fullness == 0) )
    {
        return iso14443TaL4_R_Block( pcb );
    }

    /****/
    /* Coding of S-block PCB:                             */
    /* +---------------------------------------+          */
    /* | b8 | b7 | b6 | b5 | b4 | b3 | b2 | b1 |          */
    /* +---------------------------------------+          */
    /* |  1 |  1 |  x |  y |  z |  0 |  a |  0 |          */
    /* +---------------------------------------+          */
    /*                                                    */
    /*  xy = 00, a=1 DESELECT */
    /*  xy = 00, a=0 PARAMETERS */
    /*  xy = 11, a=1 WTX */
    /*  xy = 11, a=0 RFU */
    /*  z  = 1   ->  CID following                        */

    /** \processingstep
     * Check for DESELECT S-block and process it if applicable. */
    if( (U8_AND_U8(pcb, sBlockMask) == deselectCode) && (iso14443L4_Control.pCiuBuffer->fullness == 0) )
    {
		if(sysConfig_IsTaConfEmvTest()&& (!U8_AND_U8(pcb, 0x02)) )	
		{
            //iso14443TaL4_Deselect();		//此处不符合规范，不能要。
			goto_deselect_handler(CL_REREST_FLAG);
		}
        /* check length of DESELECT */
        /* Note: DESELECT does not contain an INF field. */
        /* The NAD field shall only be used for I-blocks. */
        /* So the DESELECT command is just PCB + [CID] (=prologue). */
        /* The prologue length was already subtracted from the buffer fullness */
        /* in phIso14443L4_CheckPrologue, so that DESELECT is valid in case */
        /* iso14443L4_Control.pCiuBuffer->fullness equals 0. */
        if (iso14443L4_Control.pCiuBuffer->fullness == 0)
        {
            iso14443TaL4_Deselect();
			goto_deselect_handler(CL_REREST_FLAG);
            //return PH_ISO14443L4_ERROR;
        }
        /* else fall through to return PH_ISO14443L4_ACTIVE */
        /* because the DESELECT command is invalid and must be ignored. */
    }
    return PH_ISO14443L4_ACTIVE;
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sets the ATS.
 *
 * \param  u8 pATS
 *         pointer to ATS that shall be set
 *
 * \return None
 *
 * \note   The ATS has to be allocated externally. Only the pointer to the
 *         ATS will be set.
 */
/*===========================================================================*/
static void iso14443TaL4_SetATS(PM8 pATS )
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store pointer to ATS in instance structure. */
    iso14443L4_Control.pAts = pATS;
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function returns the address of the current ATS.
 *
 * \param  None
 *
 * \return Address of the current ATS
 *
 * \note   The ATS has to be allocated externally. Only the pointer to the
 *         ATS will be set.
 */
/*===========================================================================*/
//u8 * phIso14443L4_GetATS( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve pointer to ATS from instance structure. */
//    return iso14443L4_Control.pAts;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function retrieves the current fsdi.
 *
 * \param  None
 *
 * \return FSDI parameter
 *
 * \note   None
 */
/*===========================================================================*/
static u16 iso14443TaL4_GetFSC( void )
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve FSDI from instance structure. */
    return iso14443TaL4_fsdiTable[iso14443L4_Control.fsci];
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sends the ATS and shall be used in case RATS
 *         has already been received outside phIso14443L4_Activate.
 *
 * \param  None
 *
 * \return PH_ISO14443L4_ACTIVE     in case the T=CL protocol has been
 *                                      activated \n
 *         PH_ISO14443L4_ERROR      in case the ATS is not set
 *
 * \note   This function is used by phIso14443L4_Activate internally
 * \note   FSDI and CID must be set if this function is used outside of \n
 *         phIso14443L4_Activate
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_SendAts( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  phBuffer_t ciuBuffer - CIU buffer structure (to be filled with ATS). */
    pIso7816Util_Buffer_t      X_A		ciuBuffer;
	u8						   X_A		temATSlength;
	u8						   X_A		ATS_HistoryBytelength;
	u8						   X_A		ATSBuffer[32];
	u16						   X_A		tempTxLen;
	u16						   X_A		tempFsd;
	u8                  	   X_A      tcBytePos;
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Compose ATS. */
	
	PATCH_BOOT_APDU_IO14443TAL4_009();
	
	if(iso14443L4_Control.isInit == PH_ISO14443L4_FLAG_NO_INIT)
	{
		iso14443TaL4_Init();
		iso14443TaL4_SetInitState(PH_ISO14443L4_FLAG_INIT);
	}

	temATSlength					= sysConfig_GetAtsIfLen();
	ecpMem_Copy(&ATSBuffer[1], MEMtPN8(sysConfig_GetAtsIfBase()), temATSlength);
	ATS_HistoryBytelength			= sysConfig_GetAtsHistLen();
	ecpMem_Copy(&ATSBuffer[U8_A_U8(temATSlength , 1)], MEMtPN8(sysConfig_GetAtsHistBase()), ATS_HistoryBytelength);
	ATSBuffer[0]					= U8_A_U8(U8_A_U8(ATS_HistoryBytelength , temATSlength) , 1);

	ciuBuffer.size     				= ATSBuffer[0];
    ciuBuffer.fullness				= ATSBuffer[0];
	ciuBuffer.pData					= ATSBuffer;
		
	iso14443TaL4_SetATS(MEMtPN8(sysConfig_GetAtsIfBase()));


    /* new activation, next command will be the first L4 command */
	iso14443L4_Control.flags		= U8_OR_U8(iso14443L4_Control.flags, (FLAG_FIRST_CMD | FLAG_CMD_RECEIVED));

    /** \processingstep
     * Re-set the block number to 1 during the L4 activation. */
    iso14443L4_Control.blockNumber  = 1;

    /** \processingstep
     * Transmit ATS. */
	 tempTxLen = U16_A_U8(ciuBuffer.fullness,2);
	 tempFsd =  iso14443TaL4_fsdiTable[iso14443L4_Control.fsdi];
	 if(tempTxLen > tempFsd)
	 {
		ATSBuffer[0] = U16_S_U8(tempFsd,2);
		ciuBuffer.fullness = ATSBuffer[0];
	 }
    ecpCiu_TxBuffer( ciuBuffer.pData, ciuBuffer.fullness, PHDD_CIU_CRC_ON );

    /** \processingstep
     * If ATS indicates CID not supported, re-set received CID.
     * According to ISO14443-4 the card has to accept commands without CID in this case. */
    /* QUOTE: When the PCD has sent a RATS containing a CID=n not equal to 0 and
     * a) received an ATS indicating CID is supported, the PCD
     *    - shall send blocks containing CID=n to this PICC and
     *    - shall not use the CID=n for further RATS while this PICC is in ACTIVE state.
     * b) received an ATS indicating CID is not supported, the PCD
     *    - shall send blocks containing no CID to this PICC and
     *    - shall not activate any other PICC while this PICC is in ACTIVE state. */
    if ((ATSBuffer[0] >= 0x03) && ((ATSBuffer[1] & 0x40) != 0x00))
    {
/*        u8                     tcBytePos;*/

        tcBytePos = 2;
        if (U8_AND_U8(ATSBuffer[1], 0x20) != 0x00)
        {
            U8_INC(tcBytePos);
        }
        if (U8_AND_U8(ATSBuffer[1], 0x10) != 0x00)
        {
            U8_INC(tcBytePos);
        }
        if (U8_AND_U8(ATSBuffer[tcBytePos], 0x02) == 0x00)
        {
            iso14443L4_Control.cidReceived = 0;
            iso14443L4_Control.cid = 0;
        }
    }

	iso14443L4_Control.fsci	= U8_AND_U8(ATSBuffer[1], 0x0F);

    return PH_ISO14443L4_ACTIVE;
}

//void iso14443TaL4_FetchCommand(PX8 buf)
//{

//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function performs the ISO 14443 layer 4 T=CL
 *         protocol activation.
 *                          level \n
 * \param[in]  u8 flags\n
 *                   or'able flags that influence the behaviour of the
 *                   function. \n
 *             PH_ISO14443L4_FLAG_ACTIVATE_L4\n
 *                   Always activate to Iso14443-4.\n
 *             PH_ISO14443L4_FLAG_NONE\n
 *                   Activate Iso14443-3 or 14443-4 depending on first
 *                   command after ACTIVE(*) state.
 *
 * \return PH_ISO14443L4_ACTIVE     in case the T=CL protocol has been activated\n
 *         PH_ISO14443L3_OK         in case the ISO14443 protocol has been
 *                                  activated and the 1st command has been received\n
 *         PH_ISO14443L4_ERROR      in case of an error\n
 *
 * \note   None
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_Activate( u8 flags )    SMALL_MODE
{
#if BOOT__IO__MIFARE
	u8		X_A		mifareActiveStatus;
#endif
	
	PATCH_BOOT_APDU_IO14443TAL4_010();
	
   	while(1)
    {
        /*===========================================================================*/
        /** \processing
         ** \processingstep
         * activate card up to ISO14443L3. */
#if BOOT__IO__MIFARE
		if(sysConfig_IsMifareOs())
		{
			mifareActiveStatus	= mifare_Active();
			if(mifareActiveStatus == ECP_MIFARE_L3_RATS_RECEIVED)
			{
				iso14443TaL3_SetState(PH_ISO14443L3_ISO14443L4_STATE);
				iso14443L4_Control.L3State = PH_ISO14443L3_ISO14443L4_STATE;
			}
			else
			{
				return PH_ISO14443L4_ERROR;
			}
		}
		else
#endif
		{
	        iso14443L4_Control.L3State = phIso14443L3_GetState();
	        while( (iso14443L4_Control.L3State != PH_ISO14443L3_ACTIVE_S_STATE) && (iso14443L4_Control.L3State != PH_ISO14443L3_ACTIVE_STATE) )
	        {
	            iso14443L4_Control.L3State = iso14443TaL3_StateMachine();
	        }

			iso14443L4_Control.L3State = iso14443TaL3_StateMachine();
		}

        /** \processingstep
         * Receive next command and either continue ISO14443 T=CL activation
         * or stay in ISO14443L3. */
		if(iso14443L4_Control.isInit == PH_ISO14443L4_FLAG_NO_INIT)
		{
			iso14443TaL4_InitBuffer();
		}

        if( iso14443L4_Control.L3State == PH_ISO14443L3_ISO14443L4_STATE )
        {
#if BOOT__IO__MIFARE
			if(sysConfig_IsMifareOs())
			{
				/* upper nibble is FSDI coding the maximum frame length the PCD is able to receive */
				iso14443L4_Control.fsdi	= U8_SHR_U8(ecpMifare_GetRats1(), 4);
				/* lower nibble is the CID sent by the PCD */
				iso14443L4_Control.cid  = U8_AND_U8(ecpMifare_GetRats1(), 0x0F);
			}
			else
#endif
			{
	            /* upper nibble is FSDI coding the maximum frame length the PCD is able to receive */
	            iso14443L4_Control.fsdi = U8_SHR_U8(iso14443L4_Control.pCiuBuffer->pData[1], 4);
	            /* lower nibble is the CID sent by the PCD */
	            iso14443L4_Control.cid  = U8_AND_U8(iso14443L4_Control.pCiuBuffer->pData[1], 0x0F);
			}

            /* CID has to be in the range from 0 to 14 ( 15 is r.f.u. ) */
            if( iso14443L4_Control.cid > 14 )
            {
              return (PH_ISO14443L4_ERROR);
            }

            /* send ATS and return */
            return iso14443TaL4_SendAts( );
        }
        if( (iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_S_STATE) || (iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_STATE) )
        {
            /* command received in ACTIVE state during requested L4 activation */
            if( U8_AND_U8(flags, PH_ISO14443L4_FLAG_ACTIVATE_L4) )
            {
                if(iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_S_STATE)
                {
                    /* return to halt state */
                    iso14443TaL3_SetState( PH_ISO14443L3_HALT_STATE );
                }
                else
                {
                    /* return to idle state */
                    iso14443TaL3_SetState( PH_ISO14443L3_IDLE_STATE );
                }
            }
            else
            {
                /* return success */
                return ((phIso14443L4_ErrorCode_t)PH_ISO14443L3_OK);
            }
        }
        else
        {
            return PH_ISO14443L4_ERROR;
        }
    }

}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function performs the reception
 *         of an ISO 14443 layer 4 T=CL protocol block.
 *
 * \param[in]  u8 flags\n
 *                   or'able flags that influence the behaviour of the
 *                   function. \n
 *             PH_ISO14443L4_FLAG_RECFROMBUF\n
 *                   No data is received from CIU, command is expected
 *                   to be in the RxTxBuffer already.\n
 *             PH_ISO14443L4_FLAG_BYPASSPARSER\n
 *                   The 7816 command parser is bypassed.\n
 *             PH_ISO14443L4_FLAG_EXTCHAINING\n
 *                   Enables extended (manual) chaining (if command buffer
 *                   is full, return value PH_ISO14443L4_ACTIVE_FULLBUF
 *                   will be returned, so that the application can process
 *                   the data before receiving the next chained frame).\n
 *             PH_ISO14443L4_FLAG_NONE\n
 *                   normal operation.
 *
 * \return PH_ISO14443L4_ACTIVE     in case the card is still active but
 *                                  no command has been received\n
 *         PH_ISO14443L4_ACTIVE_FULLBUF in case chaining is ongoing and the
 *                                  reception buffer is full\n
 *         PH_ISO14443L4_OK         in case the card is still active and
 *                                  a command has been received\n
 *         PH_ISO14443L4_ERROR      in case of an error
 *
 * \note   If PH_ISO14443L4_FLAG_RECFROMBUF is used, the data is not received
 *         via the contactless interface, but is expected to found in pRxTxBuffer
 *         (set via phIso14443L4_Init). This data needs to contain the prologue,
 *         but no CRC. \n
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_ReceiveExt( u8 flags )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  phddCiu_ErrorCode_t ciuResult - CIU driver call results. */
    phddCiu_ErrorCode_t                  D_A		ciuResult;
    /** \vardef  phIso14443L4_ErrorCode_t retVal - Iso14443 internal return codes are stored in here. */
    phIso14443L4_ErrorCode_t             D_A		retVal;

	PATCH_BOOT_APDU_IO14443TAL4_011();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Get state of statemachine. */
    iso14443L4_Control.L3State = phIso14443L3_GetState();

    if( iso14443L4_Control.L3State != PH_ISO14443L3_ISO14443L4_STATE )
    {
         /** \processingstep
          * Activate ISO14443L4 if not yet done. */
        return iso14443TaL4_Activate(PH_ISO14443L4_FLAG_ACTIVATE_L4);
    }
    else
    {
         /** \processingstep
          * Set CIU reception buffer pointer. */
        iso14443L4_Control.pCiuBuffer->pData    = MEMtPX8(iso14443L4_Control.pDataRx);

         /** \processingstep
          * If a complete command was received previously or data is pending in
            the CIU buffer, re-set command parser. */
        if (U8_AND_U8(iso14443L4_Control.flags, FLAG_CMD_RECEIVED))
        {
            /* reset */
            iso7816Util_ResetParser();
			iso14443L4_Control.flags	= U8_AND_U8(iso14443L4_Control.flags, ~FLAG_CMD_RECEIVED);
        }
        if ( iso14443L4_Control.bytesPending > 0 )
        {
            /* reset */
            iso7816Util_ResetParser();
            iso14443L4_Control.pCiuBuffer->fullness = iso14443L4_Control.bytesPending;
        }

         /** \processingstep
          * Receive a block. */
        if ( (U8_AND_U8(flags, PH_ISO14443L4_FLAG_RECFROMBUF) == 0) && (iso14443L4_Control.bytesPending == 0) )
        {
            do
            {
                //ciuResult	= ecpCiu_RxBuffer( iso14443L4_Control.pCiuBuffer, PHDD_CIU_CRC_ON | PHDD_CIU_USE_CM, iso14443TaL4_GetFSC());
				ciuResult	= ecpCiu_RxBuffer(iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->size, &iso14443L4_Control.pCiuBuffer->fullness, PHDD_CIU_CRC_ON | PHDD_CIU_USE_CM | PHDD_CIU_CHECK_FIRST_COMMOND_ON, iso14443TaL4_GetFSC());
				if ( ciuResult == PHDD_CIU_RX_ERROR )
                {
                    /* according to ISO10373 SCN G.19 also a reception error */
                    /* seems to count as a first command and following PPS */
                    /* requests must be ignored. */
					iso14443L4_Control.flags	= U8_AND_U8(iso14443L4_Control.flags, ~FLAG_FIRST_CMD);
                }
				else if( ciuResult == PHDD_CIU_OK_WITH_FIRST_ERROR )
				{
                    /* according to ISO10373 SCN G.19 also a reception error */
                    /* seems to count as a first command and following PPS */
                    /* requests must be ignored. */
					iso14443L4_Control.flags	= U8_AND_U8(iso14443L4_Control.flags, ~FLAG_FIRST_CMD);
					ciuResult					= PHDD_CIU_OK;
				}
            } while (ciuResult != PHDD_CIU_OK);
        }
        else
        {
            ciuResult = PHDD_CIU_OK;
        }


         /** \processingstep
          * Check for reception errors. */
        if( (ciuResult == PHDD_CIU_OK) && (iso14443L4_Control.pCiuBuffer->fullness > 0) )
        {
            retVal = iso14443TaL4_DecodeBlock(flags);
			iso14443L4_Control.flags = U8_AND_U8(iso14443L4_Control.flags, ~FLAG_FIRST_CMD);
            if (retVal ==  PH_ISO14443L4_ACTIVE_PPS)
            {
                return PH_ISO14443L4_ACTIVE;
            }
            else if (retVal ==  PH_ISO14443L4_ERROR)
            {
                return PH_ISO14443L4_ERROR;
            }
			else
			{
				;
			}
        }
        else
        {
            retVal = PH_ISO14443L4_OK;
        }

        /* check if card is still active */
        if( iso14443L4_Control.L3State != PH_ISO14443L3_ISO14443L4_STATE )
        {
            return PH_ISO14443L4_ERROR;
        }

         /** \processingstep
          * check if a complete command block has been received. */
        if (U8_AND_U8(iso14443L4_Control.flags, FLAG_CMD_RECEIVED))
        {
            if (U8_AND_U8(flags, PH_ISO14443L4_FLAG_BYPASSPARSER))
            {
            }
            else
            {
                iso14443L4_Control.pCiuBuffer->fullness = 0;
                iso14443L4_Control.pCiuBuffer->pData	= MEMtPX8(iso14443L4_Control.pInformation);
            }
            return PH_ISO14443L4_OK;
        }
        else
        {
            if (retVal == PH_ISO14443L4_ACTIVE_FULLBUF)
            {
                return PH_ISO14443L4_ACTIVE_FULLBUF;
            }
            else
            {
                return PH_ISO14443L4_ACTIVE;
            }
        }
    }

/*    return (PH_ISO14443L4_ERROR);				 */   /*Unreachable code */
}
/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function performs the reception
 *         of an ISO 14443 layer 4 T=CL protocol block.
 *
 * \param  None
 *
 * \return PH_ISO14443L4_ACTIVE     In case the ISO/IEC 14443 L4 protocol
 *                                  has been activated. \n
 *                                  In case the card is active but
 *                                  no command has been received \n
 *         PH_ISO14443L4_OK         In case the card is active and
 *                                  a command has been received \n
 *         PH_ISO14443L4_ERROR      In case of an error
 *
 * \note   This function calls phIso14443L4_ReceiveExt with the 7816 parser
 *         enabled to provide backward compatibility.
 */
/*===========================================================================*/
u16 iso14443TaL4_Receive(void)
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Call phIso14443L4_ReceiveExt with parameters (PH_ISO14443L4_FLAG_NONE). */
    //return phIso14443L4_ReceiveExt(PH_ISO14443L4_FLAG_NONE);
    return iso14443TaL4_ReceiveExt(PH_ISO7816L4_FLAG_BYPASSPARSER);
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function performs the transmission
 *         of an ISO 14443 layer 4 T=CL protocol block.
 *
 * \param[in]  u8 flags\n
 *                   Or'able flags that influence the behaviour of the
 *                   function. \n
 *             PH_ISO14443L4_FLAG_BYPASSPARSER\n
 *                   The 7816 command parser is bypassed.\n
 *             PH_ISO14443L4_FLAG_EXTCHAINING\n
 *                   Enables the chaining bit in the PCB manually.\n
 *             PH_ISO14443L4_FLAG_NONE\n
 *                   Normal operation.
 *
 * \return PH_ISO14443L4_ACTIVE     in case the transmission is still
 *                                  active (chaining) \n
 *         PH_ISO14443L4_OK         in case the card is still active and
 *                                  the block has been transmitted \n
 *         PH_ISO14443L4_ERROR      in case of an error
 *
 * \note   None
 */
/*===========================================================================*/
static phIso14443L4_ErrorCode_t iso14443TaL4_TransmitExt( u8 flags )    SMALL_MODE
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  u8 pcb - Holds protocol iso14443L4_Control byte. */
    u8                          D_A		pcb;
    /** \vardef  u16 numBytes - Payload size. */
    u16                         D_A		numBytes;
    /** \vardef  phBuffer_t ciuBuffer - CIU buffer structure. */
    pIso7816Util_Buffer_t                  D_A		ciuBuffer;
    /** \vardef  phddCiu_ErrorCode_t ciuResult - CIU driver call result value. */
    phddCiu_ErrorCode_t         D_A		ciuResult;
    /** \vardef  u8 buffer[R_BLOCK_SIZE] - buffer to receive answer in case of chaining. */
    u8                          X_A		buffer[R_BLOCK_SIZE];
    /** \vardef  u8 prologue - contains prologue of received block. */
    u8                          D_A		prologue;
    /** \vardef  phIso14443L4_ErrorCode_t retVal - return value. */
    phIso14443L4_ErrorCode_t    D_A		retVal;
	
	u8							D_A		rBlockMask;
	u8							D_A		rBlock;
	u8							D_A		sBlockMask;
	u8							D_A		deselectCode;
	
    /*===========================================================================*/
	
	PATCH_BOOT_APDU_IO14443TAL4_012();

	pcb       	= PH_ISO14443L4_I_BLOCK;
	retVal		= PH_ISO14443L4_ACTIVE;

    /* payload size is size specified by FSDI - prologue */
    if( iso14443L4_Control.cid != 0 || iso14443L4_Control.cidReceived )
    {
        /* reserve space for PCB, CID and CRC16 */
		numBytes	= U16_S_U8(iso14443TaL4_fsdiTable[iso14443L4_Control.fsdi], 4);
    }
    else
    {
        /* reserve space for PCB and CRC16 */
		numBytes	= U16_S_U8(iso14443TaL4_fsdiTable[iso14443L4_Control.fsdi], 3);
    }

    /** \processingstep
     * Check if Iso14443L4 is activated. */
    if( iso14443L4_Control.L3State == PH_ISO14443L3_ISO14443L4_STATE )
    {
        /* Restore the pData for Tx */
        iso14443L4_Control.pCiuBuffer->pData = MEMtPX8(iso14443L4_Control.pDataTx);

        if( iso14443L4_Control.fullness == 0 ) /* new response */
        {
            iso7816Util_ResetParser( );
            iso14443L4_Control.fullness	= iso7816Util_GetResponseLength(flags);
        }

        /* use chaining if necessary */
        if (( iso14443L4_Control.fullness > numBytes ) || U8_AND_U8(flags, PH_ISO14443L4_FLAG_EXTCHAINING))
        {
            /* enable chaining */
			pcb	= U8_OR_U8(pcb, PH_ISO14443L4_CHAINING_BIT);
            if (U8_AND_U8(flags, PH_ISO14443L4_FLAG_EXTCHAINING))
            {
                /* chaining has been enabled manually */
                /* the user is taking care of the packets sizes */
                if ( iso14443L4_Control.fullness < numBytes )
                {
                    numBytes = iso14443L4_Control.fullness;
                }
            }
        }
        else
        {
            numBytes = iso14443L4_Control.fullness;
        }
        iso14443L4_Control.pCiuBuffer->fullness = numBytes;

        /** \processingstep
         * Compose response in transmission buffer. */
        iso7816Util_SetResponseData( iso14443L4_Control.pCiuBuffer->pData, numBytes, flags );

        /** \processingstep
         * Send block. */
        iso14443TaL4_SendBlock( pcb );

        /** \processingstep
         * Check if chaining is required. */
		iso14443L4_Control.fullness		= U16_S_U16(iso14443L4_Control.fullness, numBytes);
        if( iso14443L4_Control.fullness == 0 )
        {
            return PH_ISO14443L4_OK;
        }

		if(sysConfig_IsTaConfEmvTest())
		{
			rBlockMask		= PH_EMV_R_BLOCK_MASK;
			sBlockMask		= PH_EMV_S_BLOCK_MASK;
			rBlock			= PH_EMV_R_BLOCK;
			deselectCode	= PH_EMV_DESELECT;
		}
		else
		{
			rBlockMask		= PH_ISO14443L4_R_BLOCK_MASK;
			sBlockMask		= PH_ISO14443L4_S_BLOCK_MASK;
			rBlock			= PH_ISO14443L4_R_BLOCK;
			deselectCode	= PH_ISO14443L4_DESELECT;
		}
		
        /* chaining is in progress */

        do
        {
			/** \processing
			 ** \processingstep
			 * Init local variables. */
			ciuBuffer.pData    	= buffer;
			ciuBuffer.size     	= R_BLOCK_SIZE;
			ciuBuffer.fullness 	= 0;

            /** \processingstep
             * In case chaining is used, receive block, make sure it is an R(ACK) */
            do
            {
                ciuResult = ecpCiu_RxBuffer( ciuBuffer.pData, ciuBuffer.size, &ciuBuffer.fullness, PHDD_CIU_CRC_ON, iso14443TaL4_GetFSC());
            } while (ciuResult != PHDD_CIU_OK);

            /* check for reception errors */
            if( ciuBuffer.fullness > 0 )
            {
                /* check prologue of received block */
                prologue = ciuBuffer.pData[0];
                if( 1 > iso14443TaL4_CheckPrologue( &ciuBuffer ) )
                {
                    return PH_ISO14443L4_ERROR;
                }

                /* check for R block */
                if( ((prologue & rBlockMask) == rBlock) &&  (0x00 == ciuBuffer.fullness))
                {
                    retVal =  iso14443TaL4_R_Block( prologue );
                    if (retVal == PH_ISO14443L4_ACTIVE)
                    {

						if ( iso14443L4_Control.fullness > numBytes )
				        {
				          	;
				        }
				        else
				        {
							/* disable chaining */
							pcb	= U8_AND_U8(pcb, ~PH_ISO14443L4_CHAINING_BIT);
				            numBytes = iso14443L4_Control.fullness;
				        }
				        iso14443L4_Control.pCiuBuffer->fullness = numBytes;

						/** \processingstep
				         * Compose response in transmission buffer. */
				        iso7816Util_SetResponseData( iso14443L4_Control.pCiuBuffer->pData, numBytes, flags );

				        /** \processingstep
				         * Send block. */
				        iso14443TaL4_SendBlock( pcb );

				        /** \processingstep
				         * Check if chaining is required. */
						iso14443L4_Control.fullness	= U16_S_U16(iso14443L4_Control.fullness, numBytes);
				        if( iso14443L4_Control.fullness == 0 )
				        {
				            return PH_ISO14443L4_OK;
				        }
						else
						{
							retVal = PH_ISO14443L4_OK;
						}

                    }
                }

                /* check for DESELECT S block */
				else if( (0x00 == ciuBuffer.fullness) && ((prologue & sBlockMask) == deselectCode) )
                {
                    iso14443TaL4_Deselect();
					goto_deselect_handler(CL_REREST_FLAG);
                }
				else
				{
					retVal = PH_ISO14443L4_OK;
				}
            }
        }
        while (retVal == PH_ISO14443L4_OK);

        /* toggle block number */
		iso14443L4_Control.blockNumber	= U8_XOR_U8(iso14443L4_Control.blockNumber, 1);
    }

    /** \processingstep
     * If Iso14443L4 is not activated, but only L3, try to transmit data using phIso14443L3_Transmit(). */
    if( (iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_S_STATE) || (iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_STATE) )
    {
        iso14443L4_Control.L3State = iso14443TaL3_Transmit();

        if( (iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_S_STATE) || (iso14443L4_Control.L3State == PH_ISO14443L3_ACTIVE_STATE) )
        {
            /* return success */
            return ((phIso14443L4_ErrorCode_t)PH_ISO14443L3_OK);
        }
        else
        {
             return (PH_ISO14443L4_ERROR);
        }
    }

    return (PH_ISO14443L4_ERROR);
}

u8 iso14443TaL4_GetFwi(void)
{
	return U8_AND_U8(U8_SHR_U8(PM_R_U8(&iso14443L4_Control.pAts[2]), 4), 0x0F);
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function performs the transmission
 *         of an ISO 14443 layer 4 T=CL protocol block.
 *
 * \param  None
 *
 * \return PH_ISO14443L4_ACTIVE     in case the transmission is still
 *                                  active (chaining)                     \n
 *         PH_ISO14443L4_OK         in case the card is still active and
 *                                  the block has been transmitted        \n
 *         PH_ISO14443L4_ERROR      in case of an error
 *
 * \note   This function calls phIso14443L4_TransmitExt with the 7816 parser
 *         enabled to provide backward compatibility.
 */
/*===========================================================================*/
u16 iso14443TaL4_Transmit(void)
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Call function phIso14443L4_TransmitExt with parameters (FALSE, FALSE). */
    //return phIso14443L4_TransmitExt(PH_ISO14443L4_FLAG_NONE);
		return iso14443TaL4_TransmitExt(PH_ISO7816L4_FLAG_BYPASSPARSER);
}

void iso14443TaL4_SendStatus(void)
{
	u8		storeCLKSEL;
	u16	    sr_Result;

	storeCLKSEL = U16t8(sysConfig_GetCpuCLK());
	sysConfig_SetCpuClkRfCom();

	do
	{
		sr_Result = iso14443TaL4_Transmit();
	}while( (sr_Result == PH_ISO14443L4_ACTIVE) );

	sysConfig_SetCpuCLK(U8t32(storeCLKSEL));
}

void iso14443TaL4_RecieveCommand(void)
{
	u8		storeCLKSEL;
	u16	    sr_Result;

	storeCLKSEL = U16t8(sysConfig_GetCpuCLK());
	sysConfig_SetCpuClkRfCom();

	do
	{
		sr_Result = iso14443TaL4_Receive();
	}while( (PH_ISO14443L4_OK != sr_Result) && (PH_ISO14443L3_OK != sr_Result) );

	sysConfig_SetCpuCLK(U8t32(storeCLKSEL));
}

/*============================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function requests a waiting time extension.
 *
 * \param[in]  u8 wtxm\n
 *                      Waiting Time eXtension Multiplier.
 *
 * \return PH_ISO14443L4_OK     received valid WTX response from PCD \n
 *         PH_ISO14443L4_ERROR  either received a DESELECT or a reception error
 *                              occured.
 *
 * \note   None
 */
 /*============================================================================*/
#define 				PH_EMV_WTX				0xF0
#define 				PH_ISO14443L4_WTX		0xF2
phIso14443L4_ErrorCode_t iso14443TaL4_RequestWtx(void)
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  phddCiu_ErrorCode_t ciuResult - Return value for CIU driver calls. */
    phddCiu_ErrorCode_t     X_A     ciuResult;
    /** \vardef  u8 pcb - Holds the protocol iso14443L4_Control byte. */
    u8                     	X_A 	pcb;

	s8                     	X_A 	checkPrologue;
	u8						X_A		iBlockMask;
	u8						X_A		rBlockMask;
	u8						X_A		sBlockMask;
	u8						X_A		deselectCode;
	u8						X_A		rBlock;
	u8 						X_A		wtxCode;
	
	PATCH_BOOT_APDU_IO14443TAL4_013();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Check that ISO14443L4 is activated. */
    if( iso14443L4_Control.L3State == PH_ISO14443L3_ISO14443L4_STATE )
    {
        /** \processingstep
         * Limit WTXM to a range of 1..59. */
        /* send wtx request */
		
		if(sysConfig_IsTaConfEmvTest())
		{
			iBlockMask		= PH_EMV_I_BLOCK_MASK;
			rBlockMask		= PH_EMV_R_BLOCK_MASK;
			sBlockMask		= PH_EMV_S_BLOCK_MASK;
			rBlock			= PH_EMV_R_BLOCK;
			deselectCode	= PH_EMV_DESELECT;
			wtxCode			= PH_EMV_WTX;
		}
		else
		{
			iBlockMask		= PH_ISO14443L4_I_BLOCK_MASK;
			rBlockMask		= PH_ISO14443L4_R_BLOCK_MASK;
			sBlockMask		= PH_ISO14443L4_S_BLOCK_MASK;
			rBlock			= PH_ISO14443L4_R_BLOCK;
			deselectCode	= PH_ISO14443L4_DESELECT;
			wtxCode			= PH_ISO14443L4_WTX;
		}

        do
        {
            /** \processingstep
             * Send S(WTX) request. */
            iso14443L4_Control.pCiuBuffer->pData	= MEMtPX8(iso14443L4_Control.pInformation);
            iso14443L4_Control.pCiuBuffer->pData[0] = 1;
            iso14443L4_Control.pCiuBuffer->fullness = 1;

            iso14443TaL4_SendBlock( WTX );

            /** \processingstep
             * Receive S(WTX) response. */
			ecpCiu_Receive_command:
            ciuResult = ecpCiu_RxBuffer( iso14443L4_Control.pCiuBuffer->pData, iso14443L4_Control.pCiuBuffer->size, &iso14443L4_Control.pCiuBuffer->fullness, PHDD_CIU_CRC_ON | PHDD_CIU_USE_CM, iso14443TaL4_GetFSC());

            /** \processingstep
             * Check for reception errors and handle them if required. */
			if(ciuResult != PHDD_CIU_OK)
			{
				goto ecpCiu_Receive_command;
			}
			else if( (ciuResult == PHDD_CIU_OK) && (iso14443L4_Control.pCiuBuffer->fullness > 0) )
			{
				pcb = iso14443L4_Control.pCiuBuffer->pData[0];

				/* check prologue of received block */ 
				/* NOTE: phIso14443L4_CheckPrologue may change */
				/* iso14443L4_Control.pCiuBuffer->fullness and iso14443L4_Control.pCiuBuffer->pData*/
				checkPrologue	= iso14443TaL4_CheckPrologue( iso14443L4_Control.pCiuBuffer );
				if ( 0 == checkPrologue )
				{
					/* this is an invalid block or a response to another PICC */
					goto ecpCiu_Receive_command;
				}	 
				else if(checkPrologue < 0)	   
				{
					goto ecpCiu_Receive_command;		
				}		
				/* check for S(WTX) block */
				if(wtxCode == U8_AND_U8(pcb, sBlockMask))
				{
					if ((iso14443L4_Control.pCiuBuffer->fullness == 1))
					{
						/* The PCD shall acknowledge by sending an S(WTX) response containing */
						/* also a 1 byte long INF field that consists of two parts and contains */ 
						/* the same WTXM as received in the request */
						if (((iso14443L4_Control.pCiuBuffer->pData[0] & 0x3F) != 1 )|| (U8_AND_U8(iso14443L4_Control.pCiuBuffer->pData[0], 0xc0) != 0x00))
						{
							goto ecpCiu_Receive_command;
						}
						else
						{
							return PH_ISO14443L4_OK;
						}
					}
					else
					{
						goto ecpCiu_Receive_command;	
					}
				}
				if ((PH_ISO14443L4_I_BLOCK == U8_AND_U8(pcb, iBlockMask)))
				{               
				   goto ecpCiu_Receive_command;
				}
                /* check for R block */
                if ((iso14443L4_Control.pCiuBuffer->fullness == 0) && (rBlock == U8_AND_U8(pcb, rBlockMask)))
                {
                    continue;
                }
                /* check for DESELECT S block */
                if ((iso14443L4_Control.pCiuBuffer->fullness == 0) && (deselectCode == U8_AND_U8(pcb, sBlockMask)))
                {
                    iso14443TaL4_Deselect();
					goto_deselect_handler(CL_REREST_FLAG);
//                    return PH_ISO14443L4_ERROR;
                }
				
				goto ecpCiu_Receive_command;
             }
        } while( 1 );

    }
    return PH_ISO14443L4_ERROR;
}

void iso14443Ta_Init( u8 isDeselect )      SMALL_MODE
{
	u16  D_A    acitveResult;
	
	ecpCiu_Init();
#if BOOT__IO__MIFARE
	if(sysConfig_IsMifareOs())
	{
		mifare_config(isDeselect);
		ecpMifare_Enable();
	}
	else
#endif
	{
	 	iso14443TaL3_Config();
	}

	iso14443TaL3_Init( );
    
    if(0 == isDeselect)
    {
        iso14443TaL3_SetState( PH_ISO14443L3_IDLE_STATE );
    }
    
#if	BOOT__IO__ISO14443_TA_RF_LEVEL3
	//RfAutoFrequeControl(RF_AUTO_FREQUENCY_CONTROL_BEFORE_REQA);
	
	ecpCiu_HardwareInit(isDeselect, phIso14443L3_control.sakComplete,phIso14443L3_control.sakIncomplete,phIso14443L3_control.atqa.b9b16.wholeByteH,phIso14443L3_control.atqa.b1b8.wholeByteL,phIso14443L3_control2.uid);
	while(PH_ISO14443L3_RATS != ecpCiu_HardWareSupportStateMachine(phIso14443L3_control.pCiuBuffer->pData, &phIso14443L3_control.pCiuBuffer->fullness, isDeselect))
	{
		if(PH_ISO14443L3_HALTA == phIso14443L3_control.pCiuBuffer->pData[0])
		{
			ecpCiu_HardwareHandleDeselect();
		}
#if BOOT__IO__MIFARE
		else if(sysConfig_IsMifareOs() && IsMifareCmd(phIso14443L3_control.pCiuBuffer->pData[0]))
		{
			break;
		}
#endif
		else
		{
			ecpCiu_HardwareHandleErrorCommond();
		}
	}
	
	RfAutoFrequeControl(RF_AUTO_FREQUENCY_CONTROL_BEFORE_ATS);

	iso14443TaL3_SetState( PH_ISO14443L3_ACTIVE_STATE );
#endif

	iso14443TaL4_SetInitState(PH_ISO14443L4_FLAG_NO_INIT);

	do
	{
		acitveResult	= iso14443TaL4_Activate( PH_ISO14443L4_FLAG_NONE );
	} while (PH_ISO14443L4_ACTIVE != acitveResult);
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sets the FSDI.
 *
 * \param  u8 fsdi
 *         FSDI parameter
 *
 * \return None
 *
 * \note   This function has to be called in case ISO14443L4 is already
 *         activated.
 */
/*===========================================================================*/
//void phIso14443L4_SetFSDI( u8 fsdi )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store FSDI parameter in instance structure. */
//    iso14443L4_Control.fsdi = fsdi;
//}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function retrieves the current fsdi.
 *
 * \param  None
 *
 * \return FSDI parameter
 *
 * \note   None
 */
/*===========================================================================*/
//u8 phIso14443L4_GetFSDI( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve FSDI from instance structure. */
//    return iso14443L4_Control.fsdi;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sets the CID.
 *
 * \param  u8 cid
 *         CID
 *
 * \return None
 *
 * \note   This function has to be called in case ISO14443L4 is already
 *         activated.
 */
/*===========================================================================*/
//void phIso14443L4_SetCID( u8 cid )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store parameter in instance structure. */
//    iso14443L4_Control.cid = cid;
//}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function retrieves the current CID.
 *
 * \param  None
 *
 * \return CID
 *
 * \note   None
 */
/*===========================================================================*/
//u8 phIso14443L4_GetCID( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve cid from instance structure. */
//    return iso14443L4_Control.cid;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function retrieves the information if a CID has been received.
 *
 * \param  None
 *
 * \return CID
 *
 * \note   None
 */
/*===========================================================================*/
//u8 phIso14443L4_GetCidReceived( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve CID received flag from instance structure. */
 //   return iso14443L4_Control.cidReceived;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sets the information if a CID has been received.
 *
 * \param  None
 *
 * \return CID
 *
 * \note   None
 */
/*===========================================================================*/
//void phIso14443L4_SetCidReceived( u8 cidReceived)
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store CID received flag in instance structure. */
//    iso14443L4_Control.cidReceived = cidReceived;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sets the state of the ISO 14443 protocol state machine.
 *
 * \param  phIso14443L3_State_t State                         \n
 *         ISO 14443 Layer 3 state to be entered
 *
 * \return The current ISO 14443 Layer 3 state                        \n
 *         - PH_ISO14443L3_IDLE_STATE,                         \n
 *         - PH_ISO14443L3_READY_STATE,                        \n
 *         - PH_ISO14443L3_ACTIVE_STATE,                       \n
 *         - PH_ISO14443L3_HALT_STATE,                         \n
 *         - PH_ISO14443L3_READY_S_STATE,                      \n
 *         - PH_ISO14443L3_ACTIVE_S_STATE,                     \n
 *         - PH_ISO14443L3_ISO14443L4_STATE
 *
 * \note   None
 */
/*===========================================================================*/
//phIso14443L3_State_t phIso14443L4_SetState( phIso14443L3_State_t newState )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store new state in instance structure and return the new state. */
//    iso14443L4_Control.state = phIso14443L3_SetState( newState );
//    return iso14443L4_Control.state;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function returns the actual used baudrates.
 *
 * \param  None
 *
 * \return
 *    This function returns the actual used baudrates, coded in form of the
 *    PPSE value in the PPS command.
 *
 * \note   None
 */
/*===========================================================================*/
//u8 phIso14443L4_GetBaudrateSpeed( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve baudrate setting from instance structure. */
 //   return iso14443L4_Control.baudrate;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function returns the actual block number.
 *
 * \param  None
 *
 * \return
 *    This function returns the block number of the PCB before sending an I-FRAME.
 *
 * \note   None
 */
/*===========================================================================*/
//u8 phIso14443L4_GetBlockNr( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Retrieve block number from instance structure. */
  //  return iso14443L4_Control.blockNumber;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function sets the block number.
 *
 * \param[in]  u8 blockNumber\n
 *                      The block number.
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
//void phIso14443L4_SetBlockNr( u8 blockNumber )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store parameter in instance structure. */
  //  iso14443L4_Control.blockNumber = blockNumber;
 //   return;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function configures if a DESELECT response shall be sent
 *         automatically on DESELECT.
 *
 * \param[in]  bool bFlag
 *                   enables / disables sendig of deselect response
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
//void phIso14443L4_SetSndDeselectResp( boolean bFlag )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store parameter in instance structure. */
    //iso14443L4_Control.bSndDeselectResp = bFlag;
/*    if (bFlag == true)
    {
        iso14443L4_Control.flags |= FLAG_SND_DESEL_RESP;
    }
    else
    {
        iso14443L4_Control.flags &= ~FLAG_SND_DESEL_RESP;
    }

    return;
}			 */

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L4
 *
 * \brief  This function re-sets the extended chaining support.
 *
 * \return None
 *
 * \note   This is used to discard additional incoming frames after starting
 *          the reception with extended chaining enabled to avoid a protocol
 *          error. All chained frames have to be received before a response
 *          can be sent, even if the client does not want to process this
 *          additional data.
 */
/*===========================================================================*/
/*void phIso14443L4_ResetExtChaining( void )
{
    iso14443L4_Control.bytesPending = 0;
    iso14443L4_Control.pCiuBuffer->fullness = 0;
    iso14443L4_Control.blockNumber ^= 1;
    phIso14443L4_SendBlock( PH_ISO14443L4_R_BLOCK );
}
			   */
#endif	//HAL__IO__ISO14443_TA
