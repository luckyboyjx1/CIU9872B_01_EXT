#include "Io_14443TaL3.h"
#include "ecpReadWrite.h"
#include "BootRam.h"
#include "RfHal.h"
#include "MemBackUp.h"
#include "MemOperation.h"
#include "bootConfigMethod.h"

#if HAL__IO__ISO14443_TA
#include "SysConfig.h"

#pragma IO_14443TA_L3_RAM_START

phIso14443L3_Control_t						phIso14443L3_control;
phIso14443L3_Control2_t						phIso14443L3_control2;

IO_14443TA_L3_RAM_END

/*===========================================================================*
 * Defines:
 *===========================================================================*/
#define REQA                                (0x26 * 2)  /**< ISO14443-3 A REQA command (*2 because reception of less than 8 bits is MSB aligned)*/
#define WUPA                                (0x52 * 2)  /**< ISO14443-3 A WUPA command (*2 because reception of less than 8 bits is MSB aligned)*/
#define SEL_CL1                             0x93        /**< ISO14443-3 A SEL command cascade level 1 */
#define SEL_CL2                             0x95        /**< ISO14443-3 A SEL command cascade level 2 */
#define SEL_CL3                             0x97        /**< ISO14443-3 A SEL command cascade level 3 */
#define CASCADE_TAG                         0x88        /**< ISO14443-3 A Indicates cascaded UID */





/*#define PH_ISO14443L3_MEMCPY( d, s, n )     memcpy( d, s, n )			*/

/*===========================================================================*
 * Types:
 *===========================================================================*/

/*===========================================================================*
 * Local data:
 *===========================================================================*/

/*===========================================================================*
 * Local functions:
 *===========================================================================*/
 #if !BOOT__IO__ISO14443_TA_RF_LEVEL3
static u8              		iso14443TaL3_ReceiveReqaWupa( void );
static void                 iso14443TaL3_SendAtqa( void );
static phddCiu_ErrorCode_t  iso14443TaL3_Anticollision( void );

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function receives the REQA or WUPA command in IDLE and HALT state.
 *
 * \param  None
 *
 * \return The received command (e.g. REQA, WUPA) or \n
 *         PH_ISO14443L3_GEN_ERROR in case of an reception error
 *
 * \note   In case PHFL_SETBAUDRATE_DEFAULT_ON_DESELECT is defined at compile
 *         time the default baudrate isn't set in this function. It is called by
 *         the function phIso14443L3_Deselect instead. The integrating
 *         OS is responsible to set the 106kbit/s before entering the 14443L3
 *         protocol.
 */
/*===========================================================================*/
static u8 iso14443TaL3_ReceiveReqaWupa( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef phddCiu_ErrorCode_t ciuResult - Error code for CIU calls. */
    phddCiu_ErrorCode_t            X_A		ciuResult;
    /** \vardef u8 received_command - Stores the received command. */
    u8                             	X_A		received_command;
    /** \vardef phBuffer_t ciuBuffer - CIU buffer structure for reception. */
    pIso7816Util_Buffer_t            X_A		ciuBuffer;
#if	WITH_SET_BAUDRATE_DEFAULT_ON_DESELECT

	u8 								D_A		outBaudrate;

#endif /* PHFL_SETBAUDRATE_DEFAULT_ON_DESELECT */


    /*===========================================================================*/

    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Init local variables. */
		 
	PATCH_BOOT_APDU_IO14443TAL3_001();
		
    ciuBuffer.size			= 1;
    ciuBuffer.fullness		= 0;
    ciuBuffer.pData			= &received_command;

    /** \processingstep
     * If PHFL_SET_BAUDRATE_DEFAULT_ON_DESELECT is enabled, set baudrate to 106kbit/s . */
#if WITH_SET_BAUDRATE_DEFAULT_ON_DESELECT
    ecpCiu_SetBaudrate( (PHDD_CIU_TX_BAUDRATE_106K | PHDD_CIU_RX_BAUDRATE_106K), &outBaudrate );
#endif  /* PHFL_SETBAUDRATE_DEFAULT_ON_DESELECT */

    /** \processingstep
     * Receive REQA or WUPA command. */
    //ciuResult = ecpCiu_RxBuffer( &ciuBuffer, PHDD_CIU_NONE, 1);
		ciuResult = ecpCiu_RxBuffer(ciuBuffer.pData, ciuBuffer.size, &ciuBuffer.fullness, PHDD_CIU_NONE, 1);
		
    /** \processingstep
     * REQA or WUPA command is 7 bits long, if the length of the received command
     * is different from 7 bits, return error code. Else return the received command. */
    if( ciuBuffer.fullness != 0 || ciuResult != PH_ISO14443L3_SHORT_FRAME_LENGTH )
    {
        return PH_ISO14443L3_GEN_ERROR;
    }

    return received_command;
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sends the ATQA. To be called after successfully
 *         received REQA or WUPA command.
 *
 * \param  None
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
static void iso14443TaL3_SendAtqa( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef phBuffer_t ciuBuffer - CIU buffer structure. */
    pIso7816Util_Buffer_t          D_A		ciuBuffer;
    u8             		D_A		tmp;

    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * send ATQA. */
	PATCH_BOOT_APDU_IO14443TAL3_002();
	
    ciuBuffer.size     	= 2;
    ciuBuffer.fullness 	= 2;
    ciuBuffer.pData    	= phIso14443L3_control.pCiuBuffer->pData;
    tmp					= phIso14443L3_control.atqa.b1b8.wholeByteL;
    ciuBuffer.pData[0] 	= tmp;
    tmp					= phIso14443L3_control.atqa.b9b16.wholeByteH;
    ciuBuffer.pData[1] 	= tmp;

    ecpCiu_TxBuffer( ciuBuffer.pData, ciuBuffer.fullness, PHDD_CIU_NONE );

    return;
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function performs the anticollision in ISO 14443 layer 3
 *         READY and READY* states.
 *
 * \param  None
 *
 * \return PHDD_CIU_PICC_SELECTED in case of an successful SELECT command   \n
 *         PHDD_CIU_CL2_BYPASS in case cascade level 2 bypass is active and triggered \n
 *         PHDD_CIU_ERROR in case of an error
 *
 * \note   None
 */
/*===========================================================================*/
static phddCiu_ErrorCode_t iso14443TaL3_Anticollision( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef u8 offset - offset for cascaded UID. */
    u8                     	X_A		offset;
    /** \vardef u8 cascadeLevel - current cascade level in anticollision loop. */
    u8                     	X_A		cascadeLevel;
    /** \vardef phddCiu_ErrorCode_t ciuResult - Error code for CIU calls. */
    phddCiu_ErrorCode_t		X_A		ciuResult;

	pIso7816Util_Buffer_t				D_A		l3a_UidBuffer;

	PATCH_BOOT_APDU_IO14443TAL3_003();
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * reset the cascade level. */
    cascadeLevel = 0;

    /** \processingstep
     * run anticollision loop. */
    phIso14443L3_control.bBypassCl2Active = false;
    do
    {
        /* calculate offset for cascaded uid */
		offset	= U8_M_U8(cascadeLevel, 5);

        /* this function carries out the anticollision loop. If the return value is
         * PHDD_CIU_PICC_SELECTED a successful SELECT command was received.
         * If the return value is PHDD_CIU_OK a successful ANTICOLLISION command
         * was carried out otherwise an error occured
         */
        //PH_ISO14443L3_MEMCPY(phIso14443L3_control.pCiuBuffer->pData, &phIso14443L3_control2.uid[offset], 5);
				l3a_UidBuffer.pData = &phIso14443L3_control2.uid[offset];
        ciuResult = ecpCiu_Anticollision( l3a_UidBuffer.pData, l3a_UidBuffer.size, MEMtPX16(&l3a_UidBuffer.fullness), phIso14443L3_control.sakComplete, phIso14443L3_control.sakIncomplete, MEMtPR8(&cascadeLevel) );

        /* in case bypass of CL2 is enabled, the function must */
        /* exit after CL1 in case some command was received */
        if (phIso14443L3_control.bBypassCl2 == true)
        {
            if ((cascadeLevel == 1) && (ciuResult == PHDD_CIU_ANTICOLLISION_ERROR))
            {
                phIso14443L3_control.bBypassCl2Active = true;
                return PHDD_CIU_CL2_BYPASS;
            }
        }
    } while( ciuResult == PHDD_CIU_OK );

    /** \processingstep
     * Return result. */
    return ciuResult;
}

/*===========================================================================*
 * Exported functions:
 *===========================================================================*/


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function receives the RATS, HALTA or another command.
 *         It can be called in ISO 14443 layer 3 ACTIVE and ACTIVE* state.
 *
 * \param  None
 *
 * \return PH_ISO14443L3_OK in case of a valid reception            \n
 *         PH_ISO14443L3_HALTA in case of a HALTA command           \n
 *         PH_ISO14443L3_SEL in case of a SELECT command            \n
 *         PH_ISO14443L3_RATS in case of a RATS command             \n
 *         PH_ISO14443L3_RX_ERROR in case of an reception error     \n
 *         PH_ISO14443L3_RATS_ERR in case of an invalid CID         \n
 *         PH_ISO14443L3_GEN_ERROR in case of an invalid command
 *
 * \note   None
 */
/*===========================================================================*/
static u8 iso14443TaL3_ReceiveRatsHalt( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef phddCiu_ErrorCode_t ciuResult - Error code for CIU calls. */
    phddCiu_ErrorCode_t		I_A         ciuResult;
		
	PATCH_BOOT_APDU_IO14443TAL3_004();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * receive RATS or HALT command with 2 CRC bytes . */
    //ciuResult = ecpCiu_RxBuffer( phIso14443L3_control.pCiuBuffer, PHDD_CIU_CRC_ON, 4);
		ciuResult = ecpCiu_RxBuffer( phIso14443L3_control.pCiuBuffer->pData, phIso14443L3_control.pCiuBuffer->size, &phIso14443L3_control.pCiuBuffer->fullness, PHDD_CIU_CRC_ON, 4);
    /** \processingstep
     * check for a reception error. */
    if( ciuResult != PHDD_CIU_OK )
    {
        if (ciuResult == PHDD_CIU_RX_ERROR)
        {
            return (PH_ISO14443L3_RX_ERROR);
        }
        else
        {
            return (PH_ISO14443L3_FULLBYTE_ERROR);
        }
    }

    /** \processingstep
     * check at least 1 u8 has been received for a valid command. */
    if( phIso14443L3_control.pCiuBuffer->fullness < 1 )
    {
        return (PH_ISO14443L3_GEN_ERROR);
    }

    /** \processingstep
     * parse received command. */
    switch ( phIso14443L3_control.pCiuBuffer->pData[0] )
    {
        case PH_ISO14443L3_RATS:
            /* check if the CID is valid */
            if(U8_AND_U8(phIso14443L3_control.pCiuBuffer->pData[1], 0x0F) != 15 )
            {
                return (PH_ISO14443L3_RATS);
            }
            else
            {
                return (PH_ISO14443L3_RATS_ERR);
            }

        case PH_ISO14443L3_HALTA:
            /* check whether the second u8 is 0 */
            if(phIso14443L3_control.pCiuBuffer->pData[1] != 0 )
            {
                /* still listen on REQA -> return an error */
                return (PH_ISO14443L3_GEN_ERROR);
            }
            /* The HLTA command consists of two bytes followed by CRC_A */
            if (phIso14443L3_control.pCiuBuffer->fullness != 2)
            {
                /* still listen on REQA -> return an error */
                return (PH_ISO14443L3_GEN_ERROR);
            }
            /* HALT received */
            return (PH_ISO14443L3_HALTA);

        case SEL_CL1:
        case SEL_CL2:
        case SEL_CL3:
            return (PH_ISO14443L3_SEL);

        default:
            return (PH_ISO14443L3_OK);
    }
/*    return (PH_ISO14443L3_GEN_ERROR);	*/
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function implements the ISO 14443 Layer 3 state diagram.
 *
 * \param  None
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
phIso14443L3_State_t iso14443TaL3_StateMachine( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef u8 received_command - Command received from CIU. */
    u8                     X_A		received_command;
		
	PATCH_BOOT_APDU_IO14443TAL3_005();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Receive command. */
    /** \processingstep
     * Handle ISO14443L3 states accordingly. See ISO/IEC FDIS 14443-3:2009
     * for a description of the state machine */
    switch( phIso14443L3_control.state )
    {
    case PH_ISO14443L3_IDLE_STATE:
        /* receive REQA or WUPA received_command */
        received_command = iso14443TaL3_ReceiveReqaWupa();

        /* if received command was REQA or WUPA */
        if( (received_command == REQA) || (received_command == WUPA) )
        {
            /* send ATQA */
            iso14443TaL3_SendAtqa();
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
            phIso14443L3_control.state = PH_ISO14443L3_READY_STATE;
        }
        else
        {
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_INV_CMD;
        }
        break;

    case PH_ISO14443L3_READY_STATE:
        /* anticollision */
        switch (iso14443TaL3_Anticollision())
        {
        case PHDD_CIU_PICC_SELECTED:
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
            phIso14443L3_control.state  = PH_ISO14443L3_ACTIVE_STATE;
            break;
        case PHDD_CIU_CL2_BYPASS:
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_CL2BYPASS;
            phIso14443L3_control.state  = PH_ISO14443L3_IDLE_STATE;
            break;
        default:
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
            phIso14443L3_control.state  = PH_ISO14443L3_IDLE_STATE;
            break;
        }
        break;

    case PH_ISO14443L3_ACTIVE_STATE:
        /* receive command */
        received_command = iso14443TaL3_ReceiveRatsHalt();
        switch (received_command)
        {
        case PH_ISO14443L3_HALTA:
            /* HALTA command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_RATS:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state = PH_ISO14443L3_ISO14443L4_STATE;
            break;
        case PH_ISO14443L3_RATS_ERR:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_RATS_ERR;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_SEL:
             // in case of any SELECT command , go back to IDLE state
             phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
             phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_OK:
            /* Valid command has been received - e.g. for MIFARE */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
//            phIso14443L3_control.state = PH_ISO14443L3_ACTIVE_STATE;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_RX_ERROR:
            /* CIU reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_REC_ERROR;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_FULLBYTE_ERROR:
            /* CIU reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_FULLBYTE_ERROR;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        default:
            /* Reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_INV_CMD;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        }
        break;

    case PH_ISO14443L3_HALT_STATE:
        /* receive WUPA command */
        received_command = iso14443TaL3_ReceiveReqaWupa();

        /* if received command was REQA or WUPA */
        if( received_command == WUPA )
        {
            /* send ATQA */
            iso14443TaL3_SendAtqa();
            phIso14443L3_control.state = PH_ISO14443L3_READY_S_STATE;
        }
        break;

    case PH_ISO14443L3_READY_S_STATE:
        /* anticollision */
        switch (iso14443TaL3_Anticollision())
        {
        case PHDD_CIU_PICC_SELECTED:
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
            phIso14443L3_control.state  = PH_ISO14443L3_ACTIVE_S_STATE;
            break;
        case PHDD_CIU_CL2_BYPASS:
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_CL2BYPASS;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        default:
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        }
        break;

    case PH_ISO14443L3_ACTIVE_S_STATE:
        /* receive command */
        received_command = iso14443TaL3_ReceiveRatsHalt();
        switch (received_command)
        {
        case PH_ISO14443L3_HALTA:
            /* HALTA command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_RATS:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state  = PH_ISO14443L3_ISO14443L4_STATE;
            break;
        case PH_ISO14443L3_RATS_ERR:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_RATS_ERR;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_SEL:
            // in case of any SELECT command , go back to HALT state
             phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
             phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_OK:
            /* Valid command has been received - e.g. for MIFARE */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
//            phIso14443L3_control.state  = PH_ISO14443L3_ACTIVE_S_STATE;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_FULLBYTE_ERROR:
            /* CIU reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_FULLBYTE_ERROR;
            phIso14443L3_control.state = PH_ISO14443L3_HALT_STATE;
            break;
        default:
            /* Reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_INV_CMD;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        }
        break;

    case PH_ISO14443L3_ISO14443L4_STATE:
        break;

    default:
        break;
    }
    return phIso14443L3_control.state;
}
#else

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function implements the ISO 14443 Layer 3 state diagram.
 *
 * \param  None
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
phIso14443L3_State_t iso14443TaL3_StateMachine( void )
{
    /*===========================================================================*/
    /** \variables
     ** \vardef u8 received_command - Command received from CIU. */
    u8                     D_A		received_command;
	
	PATCH_BOOT_APDU_IO14443TAL3_006();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Receive command. */
    /** \processingstep
     * Handle ISO14443L3 states accordingly. See ISO/IEC FDIS 14443-3:2009
     * for a description of the state machine */
	
	
    received_command = phIso14443L3_control.pCiuBuffer->pData[0];
    switch( phIso14443L3_control.state )
    {
    
    case PH_ISO14443L3_ACTIVE_STATE:
        /* receive command */
        switch (received_command)
        {
        case PH_ISO14443L3_HALTA:
            /* HALTA command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_RATS:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state = PH_ISO14443L3_ISO14443L4_STATE;
            break;
        case PH_ISO14443L3_RATS_ERR:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_RATS_ERR;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_SEL:
             // in case of any SELECT command , go back to IDLE state
             phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
             phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_OK:
            /* Valid command has been received - e.g. for MIFARE */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
//            phIso14443L3_control.state = PH_ISO14443L3_ACTIVE_STATE;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_RX_ERROR:
            /* CIU reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_REC_ERROR;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        case PH_ISO14443L3_FULLBYTE_ERROR:
            /* CIU reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_FULLBYTE_ERROR;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        default:
            /* Reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_INV_CMD;
            phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            break;
        }
        break;
    case PH_ISO14443L3_ACTIVE_S_STATE:
        /* receive command */
        switch (received_command)
        {
        case PH_ISO14443L3_HALTA:
            /* HALTA command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_RATS:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
            phIso14443L3_control.state  = PH_ISO14443L3_ISO14443L4_STATE;
            break;
        case PH_ISO14443L3_RATS_ERR:
            /* RATS command has been received */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_RATS_ERR;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_SEL:
            // in case of any SELECT command , go back to HALT state
             phIso14443L3_control.reason = PH_ISO14443L3_REASON_ISO_CMD;
             phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_OK:
            /* Valid command has been received - e.g. for MIFARE */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_NONE;
//            phIso14443L3_control.state  = PH_ISO14443L3_ACTIVE_S_STATE;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        case PH_ISO14443L3_FULLBYTE_ERROR:
            /* CIU reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_FULLBYTE_ERROR;
            phIso14443L3_control.state = PH_ISO14443L3_HALT_STATE;
            break;
        default:
            /* Reception error */
            phIso14443L3_control.reason = PH_ISO14443L3_REASON_INV_CMD;
            phIso14443L3_control.state  = PH_ISO14443L3_HALT_STATE;
            break;
        }
        break;

    default:
        break;
    }
    return phIso14443L3_control.state;
}

#endif

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function initilaizes the ISO 14443 layer 3 protocol component.
 *
 * \param[in]  pphBuffer_t pRxTxBuffer\n
 *                      pointer to the exchange buffer structure
 *
 * \return none
 *
 * \note   None
 */
/*===========================================================================*/
void iso14443TaL3_Config( void )
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Initialize instance structure. */
/*	phIso14443L4_ErrorCode_t result;		 */
	PN8 					X_A		TaUserUIDAddr;
	u8 						D_A		IO_TCL_CONFIG;

	PATCH_BOOT_APDU_IO14443TAL3_007();
	
	IO_TCL_CONFIG							= sysConfig_GetTaConf();

	if(IO_TCL_CONFIG & IOCONFIG_TA_CONF_USERSAK_COMPLETE_MASK)
	{
		phIso14443L3_control.sakComplete	= sysConfig_GetTaUsersakComplete();
	}
	else
	{
		phIso14443L3_control.sakComplete	= chip_GetSakComplete();
	}

	if(IO_TCL_CONFIG & IOCONFIG_TA_CONF_USERSAK_INCOMPLETE_MASK)
	{
		phIso14443L3_control.sakIncomplete	= sysConfig_GetTaUsersakInComplete();
	}
	else
	{
		phIso14443L3_control.sakIncomplete	= U8_OR_U8(chip_GetSakInComplete(), 0x04);
	}
	 /* array holding the atq */
	if(IO_TCL_CONFIG & IOCONFIG_TA_CONF_USERATQ_MASK)
	{
		phIso14443L3_control.atqa.b9b16.wholeByteH	= PN_R_U8(MEMtPN8(sysConfig_GetTaUserATQBase())); 
		phIso14443L3_control.atqa.b1b8.wholeByteL	= PN_R_U8(PN8_A_U8(MEMtPN8(sysConfig_GetTaUserATQBase()) , 1));
	}
	else
	{
		phIso14443L3_control.atqa.b1b8.wholeByteL	= chip_GetAtqLowByte();
		phIso14443L3_control.atqa.b9b16.wholeByteH	= chip_GetAtqHighByte();
	}

	/* array holding the UID */
	if(IO_TCL_CONFIG & IOCONFIG_TA_CONF_USERUID_MASK)
	{
		TaUserUIDAddr								= MEMtPN8(sysConfig_GetTAUIDBase());
		phIso14443L3_control2.uid[0]				= PN_R_U8(TaUserUIDAddr);
		if(phIso14443L3_control2.uid[0] == 0x08)// random uid
		{
			ecpCiu_RandomUID(phIso14443L3_control2.uid);
		}
		else
		{
			PX_W_U32(&phIso14443L3_control2.uid[0], PN_R_U32(MEMtPN8(TaUserUIDAddr)));
			phIso14443L3_control2.uid[4]	= U8_XOR_U8(phIso14443L3_control2.uid[0], U8_XOR_U8(phIso14443L3_control2.uid[1], U8_XOR_U8(phIso14443L3_control2.uid[2], phIso14443L3_control2.uid[3])));
			if(phIso14443L3_control2.uid[0] == 0x88)
			{
				TaUserUIDAddr						= PN8_A_U8(TaUserUIDAddr, 4L);
				PX_W_U32(&phIso14443L3_control2.uid[5], PN_R_U32(TaUserUIDAddr));
				//	BCC is calculated for the UID part two.
				phIso14443L3_control2.uid[9]	= U8_XOR_U8(phIso14443L3_control2.uid[5], U8_XOR_U8(phIso14443L3_control2.uid[6], U8_XOR_U8(phIso14443L3_control2.uid[7], phIso14443L3_control2.uid[8])));
				if(phIso14443L3_control2.uid[5] == 0x88)
				{
					TaUserUIDAddr					= PN8_A_U8(TaUserUIDAddr, 4L);
					PX_W_U32(&phIso14443L3_control2.uid[10], PN_R_U32(TaUserUIDAddr));
					//	BCC is calculated for the UID part three
					phIso14443L3_control2.uid[14]	= U8_XOR_U8(phIso14443L3_control2.uid[10], U8_XOR_U8(phIso14443L3_control2.uid[11], U8_XOR_U8(phIso14443L3_control2.uid[12], phIso14443L3_control2.uid[13])));
				}
			}
		}
	}
	else
	{
		//	get the first part of the UID out of the Security Row
		chip_GetCascade1UID(phIso14443L3_control2.uid, (IO_TCL_CONFIG & IOCONFIG_TA_CONF_UID_LEVEL));
		if(phIso14443L3_control2.uid[0] == 0x88)
		{
			//	get the second part of the UID out of the Security Row
			chip_GetCascade2UID(phIso14443L3_control2.uid);
			if(phIso14443L3_control2.uid[5] == 0x88)
			{
				//	the third part of the UID can not be fetched from the Security Row as the SmartMX is not
				//	designed for a 10 bit UID. So this part is simulated by the software
				chip_GetCascade3UID(phIso14443L3_control2.uid);
			}
		}
	}
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function initilaizes the ISO 14443 layer 3 protocol buffer.
 *
 * \param[in]  pphBuffer_t pRxTxBuffer\n
 *                      pointer to the exchange buffer structure
 *
 * \return none
 *
 * \note   None
 */
/*===========================================================================*/
void iso14443TaL3_Init( void )
{
	ppIso7816Util_Buffer_t		D_A    cl3i_pRxTxBuffer;
	
	PATCH_BOOT_APDU_IO14443TAL3_008();
	
	iso7816Util_GetIoBuffer( (ppIso7816Util_Buffer_t *)&cl3i_pRxTxBuffer );
    cl3i_pRxTxBuffer -> fullness = 0;
	
    phIso14443L3_control.pCiuBuffer						= cl3i_pRxTxBuffer;              /* CIU RxTx buffer for data exchange */
    //phIso14443L3_control.state                          = PH_ISO14443L3_IDLE_STATE; /* ISO 14443 layer 3 state */
    phIso14443L3_control.reason                         = PH_ISO14443L3_REASON_NONE;/* reason for state transition */
    phIso14443L3_control.bBypassCl2                     = false;                    /* bypass of cascade level 2 */
    phIso14443L3_control.bBypassCl2Active               = false;                    /* stores if bypass of CL 2 is active */
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sends a response in ISO 14443 Layer 3 ACTIVE and
 *         ACTIVE* state.
 *
 * \param  None
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
phIso14443L3_State_t iso14443TaL3_Transmit( void )
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Check if state is ACTIVE or ACTIVE*. */
	
	PATCH_BOOT_APDU_IO14443TAL3_009();
	
    if( (phIso14443L3_control.state == PH_ISO14443L3_ACTIVE_STATE) ||
        (phIso14443L3_control.state == PH_ISO14443L3_ACTIVE_S_STATE) )
    {
        /** \processingstep
         * Transmit response. */
    	ecpCiu_TxBuffer( phIso14443L3_control.pCiuBuffer->pData, phIso14443L3_control.pCiuBuffer->fullness, PHDD_CIU_CRC_ON );

    }

    /** \processingstep
     * Return current state. */
    return phIso14443L3_control.state;
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function receives a command in ISO 14443 Layer 3 ACTIVE and
 *         ACTIVE* state.
 *
 * \param  None
 *
 * \return The current ISO 14443 Layer 3 state                 \n
 *         - PH_ISO14443L3_IDLE_STATE,                         \n
 *         - PH_ISO14443L3_READY_STATE,                        \n
 *         - PH_ISO14443L3_ACTIVE_STATE,                       \n
 *         - PH_ISO14443L3_HALT_STATE,                         \n
 *         - PH_ISO14443L3_READY_S_STATE,                      \n
 *         - PH_ISO14443L3_ACTIVE_S_STATE,                     \n
 *         - PH_ISO14443L3_ISO14443L4_STATE
 *
 * \note   No HALTA handling is applied. To detect HALTA call the function
 *         phIso14443L3_StateMachine. There is a usecase in the Mifare IP
 *         where this behaviour is required. Be sure that when using this
 *         function an L3 command is expected.
 */
/*===========================================================================*/
/*
phIso14443L3_State_t phIso14443L3_Receive( void )
{
  //  ===========================================================================
  //   \variables
  //    \vardef phddCiu_ErrorCode_t ciuResult - Error code for CIU calls.
  //  phddCiu_ErrorCode_t             ciuResult;

  //  ===========================================================================
   //  \processing
   //   \processingstep
  //    Check if state is ACTIVE or ACTIVE*.
    if( (phIso14443L3_control.state == PH_ISO14443L3_ACTIVE_STATE) ||
        (phIso14443L3_control.state == PH_ISO14443L3_ACTIVE_S_STATE) )
    {
        // \processingstep
        // Receive a command.
        ciuResult = ecpCiu_RxBuffer( phIso14443L3_control.pCiuBuffer, PHDD_CIU_CRC_ON );

        // \processingstep
        // Handle reception errors.
        if( PHDD_CIU_OK != ciuResult )
        {
            if (ciuResult == PHDD_CIU_FULLBYTE_ERROR)
            {
                phIso14443L3_control.reason = PH_ISO14443L3_REASON_FULLBYTE_ERROR;
            }
            else
            {
                // PH_ISO14443L3_RX_ERROR
                phIso14443L3_control.reason = PH_ISO14443L3_REASON_REC_ERROR;
            }

            if( phIso14443L3_control.state == PH_ISO14443L3_ACTIVE_STATE )
            {
                phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
            }
            if( phIso14443L3_control.state == PH_ISO14443L3_ACTIVE_S_STATE)
            {
                phIso14443L3_control.state = PH_ISO14443L3_HALT_STATE;
            }
        }
    }
    // \processingstep
     // Return current state.
    return phIso14443L3_control.state;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function handles an ISO 14443 Layer 4 DESELECT command.
 *
 * \param  None
 *
 * \return The current ISO 14443 Layer 3 state                 \n
 *         - PH_ISO14443L3_IDLE_STATE,                         \n
 *         - PH_ISO14443L3_READY_STATE,                        \n
 *         - PH_ISO14443L3_ACTIVE_STATE,                       \n
 *         - PH_ISO14443L3_HALT_STATE,                         \n
 *         - PH_ISO14443L3_READY_S_STATE,                      \n
 *         - PH_ISO14443L3_ACTIVE_S_STATE
 *
 * \note   In case PHFL_SET_BAUDRATE_DEFAULT_ON_DESELECT is defined at compile
 *         time the default baudrate is set in this function. The integrating
 *         OS is responsible to set the 106kbit/s before entering the 14443L3
 *         protocol.
 */
/*===========================================================================*/
phIso14443L3_State_t iso14443TaL3_Deselect( void )	SMALL_MODE
{
    /*===========================================================================*/
    /** \variables
     ** \vardef  u8 outBaudrate - This variable is used to store the output
                          of phddCiu_SetBaudrate() which returns the new baudrate. */
#if WITH_SET_BAUDRATE_DEFAULT_ON_DESELECT
    u8          I_A           outBaudrate;
#endif  /* PHFL_SETBAUDRATE_DEFAULT_ON_DESELECT */

	PATCH_BOOT_APDU_IO14443TAL3_010();
	
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * In case the statemachine is in PH_ISO14443L3_ISO14443L4_STATE,set the
       state to PH_ISO14443L3_HALT_STATE and set baudrate to 106kbit/s. */
    if( phIso14443L3_control.state == PH_ISO14443L3_ISO14443L4_STATE )
    {
		ecpCiu_HardwareHandleDeselect();

        phIso14443L3_control.state = PH_ISO14443L3_HALT_STATE;
#if WITH_SET_BAUDRATE_DEFAULT_ON_DESELECT
        /* in idle state the baudrate has to be set to 106kbit/s */
        ecpCiu_SetBaudrate( (PHDD_CIU_TX_BAUDRATE_106K | PHDD_CIU_RX_BAUDRATE_106K), &outBaudrate );
#endif  /* PHFL_SETBAUDRATE_DEFAULT_ON_DESELECT */
    }

    /** \processingstep
     * Return current state. */
    return phIso14443L3_control.state;
}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function forces the ISO 14443 Layer 3 state to
 *         PH_ISO14443L3_IDLE_STATE.
 *
 * \param  None
 *
 * \return The current ISO 14443 Layer 3 state                 \n
 *         - PH_ISO14443L3_IDLE_STATE
 *
 * \note   None
 */
/*===========================================================================*/
//phIso14443L3_State_t phIso14443L3_Reset( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Set current state to PH_ISO14443L3_IDLE_STATE. */
 //   phIso14443L3_control.state = PH_ISO14443L3_IDLE_STATE;
    /** \processingstep
     * Return current state (PH_ISO14443L3_IDLE_STATE). */
//    return phIso14443L3_control.state;
//}


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sets the UID.
 *
 * \param[in]  u8  length\n
 *                       UID length
 * \param[in]  u8* pUID\n
 *                       pointer to new UID
 *
 * \return None
 *
 * \note   UID size is 7 bytes
 */
/*===========================================================================*/
/*
u8 phIso14443L3_SetUID( u8 length, u8 * pUID )
{
    //==========================================================================
    // \processing
    // \processingstep
    // If UID length is 4 byte, store 4 byte from pUID in instance structure and generate BCC byte.
    // \processingstep
     // If UID length is 7 byte, store 7 byte from pUID in instance structure and generate BCC bytes.
     // \processingstep
     // If UID length is neither 4 nor 7 byte, return error code.
    if( length == PH_ISO14443L3_UID_SIZE_SINGLE )
    {
        phIso14443L3_control2.uid[0] = pUID[0];
        phIso14443L3_control2.uid[1] = pUID[1];
        phIso14443L3_control2.uid[2] = pUID[2];
        phIso14443L3_control2.uid[3] = pUID[3];

        // BCC byte UID part 1
        phIso14443L3_control2.uid[4] = phIso14443L3_control2.uid[0] ^ phIso14443L3_control2.uid[1];
        phIso14443L3_control2.uid[4] ^= phIso14443L3_control2.uid[2];
        phIso14443L3_control2.uid[4] ^= phIso14443L3_control2.uid[3];

        return PH_ISO14443L3_OK;
    }
    else if( length == PH_ISO14443L3_UID_SIZE_DOUBLE )
    {
        phIso14443L3_control2.uid[0] = CASCADE_TAG;
        phIso14443L3_control2.uid[1] = pUID[0];
        phIso14443L3_control2.uid[2] = pUID[1];
        phIso14443L3_control2.uid[3] = pUID[2];

        // BCC byte UID part 1
        phIso14443L3_control2.uid[4] = phIso14443L3_control2.uid[0] ^ phIso14443L3_control2.uid[1];
        phIso14443L3_control2.uid[4] ^= phIso14443L3_control2.uid[2];
        phIso14443L3_control2.uid[4] ^= phIso14443L3_control2.uid[3];

        phIso14443L3_control2.uid[5] = pUID[3];
        phIso14443L3_control2.uid[6] = pUID[4];
        phIso14443L3_control2.uid[7] = pUID[5];
        phIso14443L3_control2.uid[8] = pUID[6];

        // BCC byte UID part 2
        phIso14443L3_control2.uid[9] = phIso14443L3_control2.uid[5] ^ phIso14443L3_control2.uid[6];
        phIso14443L3_control2.uid[9] ^= phIso14443L3_control2.uid[7];
        phIso14443L3_control2.uid[9] ^= phIso14443L3_control2.uid[8];

        return PH_ISO14443L3_OK;
    }
    else
    {
        return PH_ISO14443L3_PARAM_ERROR;
    }
    return PH_ISO14443L3_PARAM_ERROR;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function retrieves the current UID.
 *
 * \param[out]  u8* pUID\n
 *                        pointer to memory to copy the current UID
 *
 * \return length of UID in bytes
 *
 * \note   UID size is 7 bytes
 */
/*===========================================================================*/
/*
u8 phIso14443L3_GetUID( u8 * pUID )
{
    //===========================================================================
    // \processing
     // \processingstep
     // Copy UID bytes from instance structure to pUID.
    if( phIso14443L3_control2.uid[0] == CASCADE_TAG )
    {
        pUID[0] = phIso14443L3_control2.uid[1];
        pUID[1] = phIso14443L3_control2.uid[2];
        pUID[2] = phIso14443L3_control2.uid[3];
        pUID[3] = phIso14443L3_control2.uid[5];
        pUID[4] = phIso14443L3_control2.uid[6];
        pUID[5] = phIso14443L3_control2.uid[7];
        pUID[6] = phIso14443L3_control2.uid[8];
        return (PH_ISO14443L3_UID_SIZE_DOUBLE);
    }
    else
    {
        pUID[0] = phIso14443L3_control2.uid[0];
        pUID[1] = phIso14443L3_control2.uid[1];
        pUID[2] = phIso14443L3_control2.uid[2];
        pUID[3] = phIso14443L3_control2.uid[3];
        return (PH_ISO14443L3_UID_SIZE_SINGLE);
    }
    // \processingstep
     // Return UID size.
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sets the sak byte for the final cascade level.
 *
 * \param[in]  u8 sakComplete
 *                      sak byte
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
/*
void phIso14443L3_SetSakComplete( u8 sakComplete )
{
    //===========================================================================/
    // \processing
     // \processingstep
     // Store SAK byte in instance structure. /
    phIso14443L3_control.sakComplete = sakComplete;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function returns the current sak byte for the final cascade level.
 *
 * \param  None
 *
 * \return sak byte
 *
 * \note   None
 */
/*===========================================================================*/
/*
u8 phIso14443L3_GetSakComplete( void )
{
    //===========================================================================
    // \processing
    // \processingstep
    // Return current SAK byte.
    return phIso14443L3_control.sakComplete;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sets the sak byte for the incomplete cascade levels.
 *
 * \param[in]  u8 sakIncomplete\n
 *                      sak byte
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
/*
void phIso14443L3_SetSakIncomplete( u8 sakIncomplete )
{
    //===========================================================================
    // \processing
    // \processingstep
     // Store SAK byte in instance structure.
    phIso14443L3_control.sakIncomplete = sakIncomplete;
}
*/


/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function returns the current sak byte for the incomplete cascade levels.
 *
 * \param  None
 *
 * \return sak byte
 *
 * \note   None
 */
/*===========================================================================*/
/*
u8 phIso14443L3_GetSakIncomplete( void )
{
    //===========================================================================
    // \processing
    // \processingstep
     // Return SAK byte.
    return phIso14443L3_control.sakIncomplete;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sets the ATQA.
 *
 * \param[in]  phIso14443L3_AtqA_t atqa\n
 *                       The ATQ value (2 bytes)
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
 /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Copy 2 bytes ATQA from pATQA (function parameter) to instance structure. */
	 /*
void phIso14443L3_SetAtqA( phIso14443L3_AtqA_t atqA )
{

    phIso14443L3_control.atqa = atqA;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function returns the current ATQA.
 *
 * \param[out]   phIso14443L3_AtqA_t *pATQA
 *                         pointer to memory location to store ATQA (2 bytes)
 *
 * \return sak byte
 *
 * \note   None
 */
/*===========================================================================*/
/*
void phIso14443L3_GetAtqA( phIso14443L3_AtqA_t *pAtqA )
{
    //===========================================================================
    // \processing
     // \processingstep
     // Copy 2 bytes ATQA from instance structure to pATQA (function parameter).
    *pAtqA = phIso14443L3_control.atqa;
}
*/

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function returns the current state of the ISO 14443 Layer 3
 *         state machine.
 *
 * \param  None
 *
 * \return The current ISO 14443 Layer 3 state:                \n
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
//#if 0
//phIso14443L3_State_t phIso14443L3_GetState( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Return current state. */
//    return phIso14443L3_control.state;
//}
//#endif

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function returns the reason for the last state transition.
 *
 * \param  None
 *
 * \return The reason for the last state transition      \n
 *         - PH_ISO14443L3_REASON_REC_ERROR,             \n
 *         - PH_ISO14443L3_REASON_INV_CMD,               \n
 *         - PH_ISO14443L3_REASON_ISO_CMD,               \n
 *         - PH_ISO14443L3_REASON_CL2BYPASS,             \n
 *         - PH_ISO14443L3_REASON_RATS_ERR,              \n
 *         - PH_ISO14443L3_REASON_NONE
 *
 * \note   None
 */
/*===========================================================================*/
//phIso14443L3_Reason_t phIso14443L3_GetReason( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Return reason for last state transition. */
  //  return phIso14443L3_control.reason;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function sets the state of the ISO 14443 Layer 3 state machine.
 *
 * \param[in]  phIso14443L3_State_t newState\n
 *                                   ISO 14443 Layer 3 state to be entered
 *
 * \return The current ISO 14443 Layer 3 state:                \n
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
void iso14443TaL3_SetState( phIso14443L3_State_t newState )
{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store parameter newState in instance structure. */
    phIso14443L3_control.state = newState ;
    /** \processingstep
     * Return current state. */
    return;
}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function allows to enable bypassing of anticollision
 *         cascade level 2 for legacy reasons.
 *
 * \param[in]  boolean bEnabled\n
 *                   enable / disable bypass of CL2
 *
 * \return None
 *
 * \note   None
 */
/*===========================================================================*/
//void phIso14443L3_SetCl2Bypass( boolean bEnabled )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Store parameter bEnabled in instance structure. */
  //  phIso14443L3_control.bBypassCl2 = bEnabled;
//}

/*===========================================================================*/
/**
 * \ingroup M_phIso14443L3
 *
 * \brief  This function returns if bypassing of anticollision
 *         cascade level 2 is active.
 *
 * \param  None
 *
 * \return The status of CL2 bypass
 *
 * \note   None
 */
/*===========================================================================*/
//boolean phIso14443L3_GetCl2BypassActive( void )
//{
    /*===========================================================================*/
    /** \processing
     ** \processingstep
     * Return if CL2 bypass has been activated. */
 //   return phIso14443L3_control.bBypassCl2Active;
//}

#endif	//HAL__IO__ISO14443_TA
