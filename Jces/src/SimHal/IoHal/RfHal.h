#ifndef __RF_HAL_H__
#define	__RF_HAL_H__

#include "DataOperation.h"
#include "SysConfigProfile.h"

/*===========================================================================*/
/* flags */
/** @brief flag for CRC off and no copy machine usage */
#define PHDD_CIU_NONE      						0x01
/** @brief flag for CRC on */
#define PHDD_CIU_CRC_ON    						0x02
/** @brief flag to force useage of copy machine */
#define PHDD_CIU_USE_CM    						0x04
/** @brief flag to enable parity (currently only used in TxByte function) */
#define PHDD_CIU_PARITY_ON 						0x08
/** @brief flag to enable parity (currently only used in TxByte function) */
#define PHDD_CIU_CHECK_FIRST_COMMOND_ON 		0x10
/** @brief flag to for deselect command, if transmit and receive in one method, 
*** and after send the deselect Response, then must go to deal the wupa command.*/
#define PHDD_CIU_RESPONSE_IS_DESELECT_ON 		0x20
/** @brief flag to for PPS command, if transmit and receive in one method.*/
#define PHDD_CIU_RESPONSE_IS_PPS_ON 			0x40
/** @brief flag to for PPS command, if transmit and receive in one method.*/
#define PHDD_CIU_RESPONSE_IS_WTX_ON 			0x80


#define PH_ISO14443L3_SHORT_FRAME_LENGTH    	7           /**< length of ISO14443-3 short frame (alwyas 7 bit) */


/*	RF AUTO Frequency Contorl or auto run contorl	*/
#define	RF_AUTO_FREQUENCY_CONTROL_BEFORE_REQA	0x01
#define RF_AUTO_FREQUENCY_CONTROL_BEFORE_ATS	0x02
#define RF_AUTO_FREQUENCY_CONTROL_BEFORE_PPS	0x04
#define RF_AUTO_FREQUENCY_CONTROL_AFTER_PPS		0x08


#define PHDD_CIU_OK                         	(0x5A5A) /**< Successful execution of function. */
#define PHDD_CIU_OK_WITH_FIRST_ERROR           	(0x5AA5) /**< Successful execution of function. */
/** @brief Successfully selected PICC */
#define PHDD_CIU_PICC_SELECTED              	(0x9CC9)
/** @brief Error during execution of function. */
#define PHDD_CIU_ERROR                      	(0xA5A5)
/** @brief Error during execution of function. */
#define PHDD_CIU_TX_ERROR                   	(0x3A3A)
/** @brief Error during execution of function. */
#define PHDD_CIU_RX_ERROR                   	(0x3AA3)
#define PHDD_CIU_FULLBYTE_ERROR             	(0xF0F0)
#define PHDD_CIU_RX_CHECKVALUE_ERROR           	(0x0F0F)

/** @brief Error during execution of function. */
#define PHDD_CIU_ANTICOLLISION_ERROR        	(0x3BB3)
/** @brief Error during reception operation; Buffer was overwritten before it had been read. */
#define PHDD_CIU_RX_OVERFLOW                	(0xA55A)
/** @brief Error component not initialized before calling the function. */
#define PHDD_CIU_NOT_INIT                   	(0x5AA5)
/** @brief Error function called with bad parameter(s). */
#define PHDD_CIU_BAD_PARAMETER              	(0x55AA)
/** @brief Error function is blocked due to an ongoing operation. */
#define PHDD_CIU_BUSY                       	(0xB33B)
/** @brief Anticollision CL2 bypass is enabled and an unknown command was received after CL1. */
#define PHDD_CIU_CL2_BYPASS                  	(0x6AA6)

/******************************************************************************************************************************************************
 ******************************************************************************************************************************************************
 * functions defined for CIU
 ******************************************************************************************************************************************************
 ******************************************************************************************************************************************************/

 /*============================================================================*/
/**
 * \ingroup ecpDriverMethod
 *
 * \brief  This function sets the CIU baudrate.
 *
 * \param  [in] u8 inBaudrate
 *                       New baudrate to be set.
 *         \code
 *         bit 7-4: PICC -> PCD baud rate
 *                  0 000 - 106 kBaud (PHDD_CIU_TX_BAUDRATE_106K)
 *                  0 001 - 212 kBaud (PHDD_CIU_TX_BAUDRATE_212K)
 *                  0 010 - 424 kBaud (PHDD_CIU_TX_BAUDRATE_424K)
 *                  0 011 - 848 kBaud (PHDD_CIU_TX_BAUDRATE_848K)
 *
 *         bit 3-0: PCD -> PICC baud rate
 *                  0 000 - 106 kBaud (PHDD_CIU_RX_BAUDRATE_106K)
 *                  0 001 - 212 kBaud (PHDD_CIU_RX_BAUDRATE_212K)
 *                  0 010 - 424 kBaud (PHDD_CIU_RX_BAUDRATE_424K)
 *                  0 011 - 848 kBaud (PHDD_CIU_RX_BAUDRATE_848K)
 *         \endcode
 *         [out] u8 * pOutBaudrate
 *                         Current baudrate is stored here
 *
 * \return PHDD_CIU_BAUDRATE_OK                 baudrate set or read successfully
 *         PHDD_CIU_BAUDRATE_BAD_PARAMETER      invalid baudrate value
 *         PHDD_CIU_BAUDRATE_ACCESS_COND_ERR    not allowed to access secrow values
 *         PHDD_CIU_BAUDRATE_NOT_SUPPORTED      baudrate not supported
 *
 * \note   None.
 */
/*============================================================================*/
extern  u8	ecpCiu_SetBaudrate(u8 inBaudrate, PR8 pOutBaudrate)	SMALL_MODE;

/*============================================================================*/
/**
 * \ingroup ecpDriverMethod
 *
 * \brief  This function controls an anti-collision loop by using the CIU
 *         hardware support. The anti-collision loop is done for the passed
 *         cascade level.
 *
 * \param  [in] u8 * pUidBuffer \n
 *         Pointer to buffer containing the UID
 *
 * \param  [in] u8 sakComplete \n
 *         SAK byte for final cascade level
 *
 * \param  [in] u8 sakInomplete \n
 *         SAK byte for other cascade levels
 *
 * \param  [out] u8 * pCascadeLevel \n
 *         Actual cascade level

 * \param  [out] pphBuffer_t pRxBuffer \n
 *         Pointer to buffer that contains the received command in case cascade
 *         level 2 bypass is enabled and a command is received instead of CL2.
 *
 * \return PHDD_CIU_ANTICOLLISION_ERROR  in case of any error \n
 *         PHDD_CIU_PICC_SELECTED     in case card is selected \n
 *         PHDD_CIU_OK   in case cascade level is ok, but not last cascade level
 *
 * \note   None
 */
/*============================================================================*/
extern  u16	ecpCiu_Anticollision(PX8 pData, u16 dataSize, PX16 fullness, u8 sakComplete, u8 sakIncomplete, PR8 pCascadeLevel);

/*============================================================================*/
/**
 * \ingroup ecpDriverMethod
 *
 * \brief  This function sets the CIU to reception mode and puts bytes into
 *         the buffer referenced by the passed pointer as long as no EndOfFrame
 *         is detected or the the size of the buffer is infringed
 *
 * \param  [in, out] pphBuffer_t pRxBuffer                                    \n
 *         Pointer to the receive buffer struct (members:buffer, size, fullness)
 *
 * \param  [in] u8     flags                                             \n
 *         Bit 0: if '1' the CRC of the received data is checked              \n
 *         Bit 1: if '1' the copy machine is used
 *
 * \return PHDD_CIU_RX_ERROR,                                                 \n
 *         Number of valid bits in the last byte                              \n
 *         PHDD_CIU_OK                                                        \n
 *
 * \note   None
 */
/*============================================================================*/
extern  u16 ecpCiu_RxBuffer(PX8 pData, u16 dataSize, PR16 fulless, u8 flags, u16 fsc)	SMALL_MODE;

/*============================================================================*/
/**
 * \ingroup ecpDriverMethod
 *
 * \brief  The function phddCiu_TxBuffer() transmits a buffer of bytes via the
 *         CIU.
 *
 * \param  [in] pphBuffer_t pTxBuffer \n
 *         Pointer to the transmission buffer struct (members:buffer, size, fullness)
 *
 * \param  [in] u8     flags \n
 *         Bit 0: if '1' a CRC is appended to the data sent \n
 *         Bit 1: if '1' the copy machine is used
 *
 * \return PHDD_CIU_OK
 *
 * \note   None
 */
/*============================================================================*/
extern  void	ecpCiu_TxBuffer(PX8 pData, u16 fullness, u8 flags);

extern void RfAutoFrequeControl(u8 flag);

extern void ecpCiu_Init(void);
#if WITH_RF_DERIVE_UID
extern void ecpCiu_derive4UidFrom7Uid(PX8 des);
#endif // WITH_RF_DERIVE_UID
extern void ecpCiu_RandomUID(PX8 rUID_Buffer);
#if BOOT__IO__ISO14443_TA_RF_LEVEL3
	extern 	u8   ecpCiu_HardWareSupportStateMachine(PX8 pData, PX16 fullness, u8 isDeselect);
	extern 	void ecpCiu_HardwareHandleDeselect(void);
	extern 	void ecpCiu_HardwareHandleErrorCommond(void);
	extern 	void ecpCiu_HardwareInit(u8 isDeselect, u8 sak, u8 insak,u8 ATQA_b9_b16,u8 ATQA_b1_b8, PX8 uid );
	//#define iso14443TaL3_StateMachine() 	   ecpCiu_HardWareSupportStateMachine()
#else
	#define ecpCiu_HardwareInit(isDeselect, sak, insak, ATQA_b9_b16, ATQA_b1_b8, uid ) 		
	#define ecpCiu_HardwareHandleDeselect(void)				 							
#endif
	
#if WITH_RF_USER_MODIFY_PARAM	
	extern void ecpCiu_SetUID(PM8 uidAry);
	extern void ecpCiu_SetAtqa(u16 atqaValue);
	extern void ecpCiu_SetSakComplete(u8 sakComplete);
	extern void ecpCiu_SetSakInComplete(u8 sakIncomplete);
#else
	#define ecpCiu_SetUID(uidAry) 		                                            
	#define ecpCiu_SetAtqa( atqaValue)                                                 
	#define ecpCiu_SetSakComplete( sakComplete)                                       
	#define ecpCiu_SetSakInComplete( sakIncomplete)                                   
#endif	
	


#endif
