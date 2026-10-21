/*
 * RfPort.h
 *
 *  Created on: 2015Äê10ÔÂ27ÈÕ
 *      Author: mowenpan
 */

#ifndef SRC_HW_IO_RF_RFPORT_H_
#define SRC_HW_IO_RF_RFPORT_H_

#include "HwIo.h"
#include "ecpDefine.h"

#define HW_RF_I_BLOCK_MASK      		0xE2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define HW_RF_I_BLOCK           		0x02    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define HW_RF_S_BLOCK_MASK      		0xF7    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define HW_RF_R_BLOCK_MASK      		0xE6    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define HW_RF_R_BLOCK           		0xA2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */

typedef struct
{
	HwIo_Buffer buffer;
	u16 fullness;
	u8 direction;
	u8 ack;
} RfPort_Data, *PRfPortData;

extern void rfPort_init();
extern u16	rfPort_Anticollision(PX8 pData, u16 dataSize, PX16 fullness, u8 sakComplete, u8 sakIncomplete, PR8 pCascadeLevel);
extern u16 rfPort_RxBuffer(PX8 pData, u16 dataSize, PR16 fullness, u8 flags, u16 fsc);
extern void rfPort_TxBuffer(PX8 pData, u16 fullness, u8 flags);





/***********************************************************************************
 * The definition following is copy from BOOT and should change when the definition in BOOT changed.
 **********************************************************************************/

typedef enum
{
    RF_ISO14443L3_IDLE_STATE  = 0xf0, /**< IDLE state */
    RF_ISO14443L3_READY_STATE,        /**< READY state */
    RF_ISO14443L3_ACTIVE_STATE,       /**< ACTIVE state */
    RF_ISO14443L3_HALT_STATE,         /**< READY state */
    RF_ISO14443L3_READY_S_STATE,      /**< READY* state */
    RF_ISO14443L3_ACTIVE_S_STATE,     /**< ACTIVE* state */
    RF_ISO14443L3_ISO14443L4_STATE    /**< entered ISO 14443 layer 4 protocol */
} phIso14443L3_State_t;
#define PHDD_CIU_OK                         	(0x5A5A) /**< Successful execution of function. */
#define RF_ISO14443L3_SHORT_FRAME_LENGTH    	7           /**< length of ISO14443-3 short frame (alwyas 7 bit) */
#define RF_ISO14443L3_RATS					0xE0    /**< ISO7816-4 RATS command */
#define PHDD_CIU_RX_ERROR                   	(0x3AA3)

#endif /* SRC_HW_IO_RF_RFPORT_H_ */
