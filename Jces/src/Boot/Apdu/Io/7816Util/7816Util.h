#ifndef __7816_UTIL_H_
#define __7816_UTIL_H_

#include "DataOperation.h"
#include "SysConfig.h"
#include "bootConfigProfile.h"

/*============================================================================*
 * Project include files:
 *============================================================================*/
#define PROTOCOL_T0										0x00
#define PROTOCOL_T1										0x01
#define PROTOCOL_TA                  	0x02
#define PROTOCOL_TB                  	0x03
#define PROTOCOL_SWP                  	0x05
#define PROTOCOL_I2C      				0x06
#define PROTOCOL_SPI      				0x07
//#define PROTOCOL_INTERFACE						PROTOCOL_SWP
#define PROTOCOL_NONE                 0xFF

//java 代码:getProtocol所需要的返回值定义,该返回值以半字节定义
//#define PROTOCOL_MEDIA_MASK                       (u8)0xF0
//#define PROTOCOL_MEDIA_DEFAULT                    (u8)0x00
//#define PROTOCOL_MEDIA_CONTACTLESS_TYPE_A         (u8)0x80
//#define PROTOCOL_MEDIA_CONTACTLESS_TYPE_B         (u8)0x90
//#define PROTOCOL_MEDIA_USB                        (u8)0xA0
//#define PROTOCOL_MEDIA_CONTACTLESS_TYPE_F		  (u8)0xB0

//由于protocol_type 复用数组下标，必须设置成01
//#define PROTOCOL_MEDIA_SWP                        ((u8)0x05)

#define ISO7816UTIL_CLA_OFFSET    						0
#define ISO7816UTIL_INS_OFFSET							1
#define ISO7816UTIL_P1_OFFSET							2
#define ISO7816UTIL_P2_OFFSET     						3
#define ISO7816UTIL_P3_OFFSET     						4
#define ISO7816UTIL_DATA_OFFSET   						5
#define ISO7816UTIL_HEADER_LENGTH						5
#define ISO7816UTIL_EXT_CDATA_OFFSET					7
#define	ISO7816UTIL_STATUS_BYTE_LENGTH					2

#define APDU_EXCEPTION_IO_ERROR							0xB004
#define APDU_EXCEPTION_NO_T0_GETRESPONSE				0xB0AA

#define MAX_LE											256
/*============================================================================*
 * Defines:
 *============================================================================*/
/** \addtogroup M_phIso7816L4
*  \{*/
/*===========================================================================*/
/** @brief PH_ISO7816L4_HEADER_MIN_LENGTH is 4 bytes: CLA + INS + P1 + P2 */
#define PH_ISO7816L4_HEADER_MIN_LENGTH   				4
/** @brief PH_ISO7816L4_STANDARD_LC_LENGTH is 1 byte 0x01 up to 0xFF */
#define PH_ISO7816L4_STANDARD_LC_LENGTH  				1
/** @brief PH_ISO7816L4_EXTENDED_LC_LENGTH is defined as 3 bytes.
 * An extended Le field consists of either three bytes (one byte set to '00'
 * followed by two bytes with any value) if the Lc field is absent, or two bytes
 * (with any value) if an extended Lc field is present. */
#define PH_ISO7816L4_EXTENDED_LC_LENGTH  				3
/** @brief PH_ISO7816L4_STANDARD_LE_LENGTH is 1 byte.
 * A short Le field consists of one byte with any value. */
#define PH_ISO7816L4_STANDARD_LE_LENGTH  				1
/** @brief PH_ISO7816L4_EXTENDED_LE_LENGTH is 3 bytes.
 * An extended Le field consists of either three bytes (one byte set to '00'
 * followed by two bytes with any value) if the Lc field is absent, or two
 * bytes (with any value) if an extended Lc field is present. */
#define PH_ISO7816L4_EXTENDED_LE_LENGTH  				3
/** @brief PH_ISO7816L4_MAX_PROLOGUE_LENGTH is 3 bytes */
#define PH_ISO7816L4_MAX_PROLOGUE_LENGTH 				3
/** @brief PH_ISO7816L4_MAX_EPILOGUE_LENGTH is 1 bytes for LRC */
#define PH_ISO7816L4_MAX_EPILOGUE_LENGTH 				1
/** @brief PH_ISO7816L4_SW_LENGTH is 2 bytes (SW1 and SW2) */
#define PH_ISO7816L4_SW_LENGTH 							2
/*===========================================================================*/

/*===========================================================================*/
/** @brief internal ISO APDU GET RESPONSE */
#define PH_ISO7816L4_INS_GET_RESPONSE  					0xC0
/** @brief internal value of parameter byte P1 */
#define PH_ISO7816L4_P1_GET_RESPONSE   					0x00
/** @brief internal value of parameter byte P2 */
#define PH_ISO7816L4_P2_GET_RESPONSE   					0x00
/*===========================================================================*/

/*===========================================================================*/
/** ISO error codes (or status words) */
/** @brief Nothing special happened */
#define PH_ISO7816L4_SW_NONE                             0x0000
/** @brief No further qualification (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_NORMAL_PROCESSING                0x9000
/** @brief Data available, SW2 tells the number of response bytes still available (see ISO/IEC CD 7816-4) */
/*#define PH_ISO7816L4_SW_NORMAL_PROCESSING_HAVE_DATA      0x6100	  */
/** @brief State of non-volatile memory has changed (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_VERIFICATION_FAILED              0x6300
/** @brief State of non-volatile memory has changed (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_MEMORY_NO_INFORMATION_GIVEN      0x6500
/** @brief State of non-volatile memory has changed (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_MEMORY_FAILURE                   0x6581
/** @brief Wrong length (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_WRONG_LENGTH                     0x6700
/** @brief Command not allowed: Security status not satisfied (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_SECURITY_STATUS_NOT_SATISFIED    0x6982
/** @brief Command not allowed: Authentication method blocked (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_AUTHENTICATION_METHOD_BLOCKED    0x6983
/** @brief Command not allowed: Conditions of use not satisfied (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_CONDITIONS_OF_USE_NOT_SATISFIED  0x6985
/** @brief Wrong parameters P1-P2: Incorrect parameters in the command data field (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_INVALID_COMMAND_DATA_FIELD       0x6A80
/** @brief Wrong parameters P1-P2: Function not supported (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_FUNCTION_NOT_SUPPORTED           0x6A81
/** @brief Wrong parameters P1-P2: File not found (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_FILE_NOT_FOUND                   0x6A82
/** @brief Wrong parameters P1-P2: Record not found (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_RECORD_NOT_FOUND                 0x6A83
/** @brief Wrong parameters P1-P2: Incorrect parameters P1-P2 (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_INVALID_P1P2                     0x6A86
/** @brief Wrong parameters P1-P2: Referenced data not found (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_INVALID_REFERENCE_DATA           0x6A88
/** @brief Wrong parameters P1-P2 (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_WRONG_PARAMETERS_P1P2            0x6B00
/** @brief Wrong length Le; SW2 tells the exact length (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_INVALID_LE                       0x6C00
/** @brief Instruction code not supported or invalid (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_INSTRUCTION_NOT_SUPPORTED        0x6D00
/** @brief Class not supported (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_CLASS_NOT_SUPPORTED              0x6E00
/** @brief No precise diagnosis (see ISO/IEC CD 7816-4) */
#define PH_ISO7816L4_SW_GENERAL_ERROR                    0x6F00
/*===========================================================================*/
/** @brief Bypass the command processing, just copy the payload to command buffer. */
#define PH_ISO7816L4_FLAG_BYPASSPARSER         			 0x01
#define	PH_ISO7816L4_FLAG_T0_REV_CMD					 0x02

/*============================================================================*
 * Types:
 *============================================================================*/

/*===========================================================================*/
/** @brief  Structure to hold APDU parser status and control data. */
/*===========================================================================*/
typedef struct pIso7816Util_Control
{
    u16							dataIndex;                     /**< current data index */
	u8							isRetransmitBlock;
	u16							retransmitLength;
    u16							numBytesReceived;              /**< number of received bytes */
	u16							retransmitDataIndex;
    u8							numBytesToTransmitFromSW1SW2;  /**< number of bytes to transmit from status word */
#if BOOT__IO__APDU_EXTENDED_LENGTH
    u16							numBytesExpected;              /**< number of expected bytes */
    boolean						errorDetected;                 /**< true in case an error was detected */
    u8							lastCLA;                       /**< class byte of last APDU */
    u16							lastRespLen;                   /**< length of last response */
    u8							lastNumSW1SW2BytesTransmitted; /**< number of already transmitted bytes from status word */
#endif
} pIso7816Util_Control_t,
    *ppIso7816Util_Control_t; /**< pointer to structure phIso7816L4_Control_t */


/*===========================================================================*/
/** @brief  Structure to hold a complete command APDU. */
/*===========================================================================*/
typedef struct pIso7816Util_Command
{
#if BOOT__IO__APDU_EXTENDED_LENGTH
    u8                     CLA;        /**< class byte */
    u8                     INS;        /**< instruction byte */
    u8                     P1;         /**< P1 byte */
    u8                     P2;         /**< P2 byte */
    boolean                extended;   /**< true in case of an extended APDU */
    boolean                haveLc;     /**< true in case of payload bytes */
    boolean                haveLe;     /**< true in case of response */
#endif
    u8							chn;
    u16                    		Le;         /**< response length */
    u16                    		Lc;         /**< payload length */
    u16                    		length;     /**< total APDU length */
    PX8							pData;      /**< pointer to payload buffer */
    u16                    		size;       /**< size of payload buffer */
} pIso7816Util_Command_t,
    *ppIso7816Util_Command_t; /**< pointer to structure pphIso7816L4_Command_t */

/*===========================================================================*/
/** @brief  Structure to hold a complete command APDU. */
/*===========================================================================*/
typedef struct phIso7816L4_Response
{
    PX8							pData;    /**< pointer to response buffer */
    u16							lr;
    u16							length;   /**< length of response */
    u16							SW1SW2;   /**< status word */
} pIso7816Util_Response_t,
    *ppIso7816Util_Response_t; /**< pointer to structure phIso7816L4_Response_t */

#define	IS_SET_INCOMING			0x01

/*===========================================================================*/
/** @brief  Structure to hold a complete command APDU. */
/*===========================================================================*/
typedef struct phBuffer
{
    u16							size;		   /**< size of pData */
    u16							fullness;	   /**< the number of received bytes in the pData */
    PX8							pData;		   /**< pointer to data buffer> */
} pIso7816Util_Buffer_t, * ppIso7816Util_Buffer_t;

typedef struct
{
	pIso7816Util_Buffer_t	IoBuffer;
	pIso7816Util_Command_t	command;
	pIso7816Util_Response_t	reponse;
	pIso7816Util_Control_t	control;
}pIso7816Util_BufPck_t, * ppIso7816Util_BufPck_t;

#if BOOT__IO__ISO7816
	extern  void iso7816Util_InitUartBufPck(void);
	extern	void iso7816Util_InitClBufPck(void);
	extern	void iso7816Util_InitBufPck(void);
	extern	void iso7816Util_SetBufPckType(u8 BufPckType);
	extern	u8 	 iso7816Util_GetBufPckType(void);
	extern	void iso7816Util_SetBufPck(ppIso7816Util_BufPck_t bufPck, u8 BufPckType);
	extern  void iso7816Util_GetBufPck(ppIso7816Util_BufPck_t * bufPck, u8 BufPckType);
	extern	void iso7816Util_GetIoBuffer( ppIso7816Util_Buffer_t *ppIoBuffer );
	extern	void iso7816Util_GetCommand( ppIso7816Util_Command_t * ppCommand );
	extern	void iso7816Util_GetResponse( ppIso7816Util_Response_t *ppResponse );
	extern	void iso7816Util_GetControl( ppIso7816Util_Control_t *ppControl );
	extern	void iso7816Util_SetCommand(PX8 commandBuf, u16 size, u8 bufPckType);
	extern	void iso7816Util_Init(void);

	/*===========================================================================*/
	/**
	 * @brief  The function phIso7816L4_ResetParser() resets the (input) parser
	 *         such that a new C-APDU can be received/parsed. Called internally
	 *         but might be useful to call from external functions.
	 *
	 * @param  None
	 *
	 * @return None
	 *
	 * @note   None
	 */
	/*===========================================================================*/
	extern  void iso7816Util_ResetParser( void );


	/*===========================================================================*/
	/**
	 * @brief  The function phIso7816L4_ParseBytes() parses the given data and
	 *         fills the command structure. The function is reentrant.
	 *
	 * @param  uint8_t * pData                                                   \n
	 *         pointer to location in memory where the parser has to be started
	 * @param  uint16_t length                                                   \n
	 *         number of bytes that have to be parsed
	 * @param  bool bBypassParser                                                 \n
	 *         bypass the ISO7816 parser, just copy data to command buffer
	 *
	 * @return None
	 *
	 * @note   None
	 */
	/*===========================================================================*/
	extern  u16 iso7816Util_ParseBytes(PX8 pData, u16 length, u8 flags );


	/*===========================================================================*/
	/**
	 * @brief  The function phIso7816L4_GetCommand() can be called to retrieve
	 *         the address of the command structure
	 *
	 * @param  pphIso7816L4_Command_t * ppCommand                                 \n
	 *         pointer to the location to store the address of the command
	 *         buffer
	 *
	 * @return None
	 *
	 * @note   None
	 */
	/*===========================================================================*/
	//extern  void iso7816Util_GetCommand( ppIso7816Util_Command_t *ppCommand );

	/*===========================================================================*/
	/**
	 * @brief  The function phIso7816L4_GetResponse() can be called to retrieve
	 *         the address of the response structure
	 *
	 * @param  pphIso7816L4_Command_t * ppCommand                                 \n
	 *         pointer to the location to store the address of the command
	 *         buffer
	 *
	 * @return None
	 *
	 * @note   None
	 */
	/*===========================================================================*/
	//extern  void iso7816Util_GetResponse( ppIso7816Util_Response_t *ppResponse );


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
	extern  void iso7816Util_SetResPrameter( u16 resLength, u16 Lr, u16 SW1SW2, u8 flags );


	/*===========================================================================*/
	/**
	 * @brief  The function phIso7816L4_GetResponseLength() returns the total
	 *         response length.
	 *
	 * @param  bool bBypassParser                                                 \n
	 *         bypass the ISO7816 parser
	 *
	 * @return response length
	 *
	 * @note   None
	 */
	/*===========================================================================*/
	extern  u16 iso7816Util_GetResponseLength( boolean bBypassParser );

	/*===========================================================================*/
	/**
	 * @brief  The function phIso7816L4_SetResponse() fetches the response.
	 *
	 * @param  uint8_t * pResponseData                                           \n
	 *         destination pointer for the response data
	 * @param  uint16_t numBytes                                                 \n
	 *         number of response bytes to be set
	 * @param  bool bBypassParser                                                 \n
	 *         bypass the ISO7816 parser, just copy data from response buffer
	 *
	 * @return None
	 *
	 * @note   Make sure the length parameter is correct. Set to the size of
	 *         the comm buffer or the number of remaining bytes in order to make
	 *         the protocol correct.
	 */
	/*===========================================================================*/
	extern  void iso7816Util_SetResponseData(PX8 pResponseData, u16 numBytes, u8 flags );
#else
	#define iso7816Util_InitUartBufPck()
	#define iso7816Util_InitClBufPck()
	#define iso7816Util_InitBufPck()
	#define iso7816Util_SetCurBufPck(BufPckType)
	#define	iso7816Util_GetBufPckType()													0
	#define iso7816Util_SetBufPck(bufPck, BufPckType)
	#define iso7816Util_GetBufPck(bufPck, BufPckType)
	#define	iso7816Util_GetIoBuffer(ppIoBuffer)
	#define	iso7816Util_GetCommand(ppCommand)
	#define	iso7816Util_GetResponse(ppResponse)
	#define	iso7816Util_GetControl(ppControl)
	#define iso7816Util_SetCommand(commandBuf, size, bufPckType)
	#define	iso7816Util_Init()
	#define iso7816Util_ResetParser()
	#define iso7816Util_ParseBytes(pData, length, flags)								0
	#define iso7816Util_SetResPrameter(resLength, Lr, sw, flags)
	#define iso7816Util_GetResponseLength(bBypassParser)								0
	#define iso7816Util_SetResponseData(pResponseData, numBytes, flags)
#endif	//BOOT__IO__ISO7816

#endif
