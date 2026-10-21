#ifndef __IO_14443_TA_L3_H__
#define	__IO_14443_TA_L3_H__

#include "DataOperation.h"
#include "RfHal.h"
//#include "Io_14443TaL4.h"
#include "UartHal.h"
#include "SysConfig.h"
#include "7816Util.h"

/*============================================================================*
 * Defines:
 *============================================================================*/
/** \addtogroup M_phIso14443L3
*  \{*/
#define PH_ISO14443L3_RATS					0xE0    /**< ISO7816-4 RATS command */
#define PH_ISO14443L3_OK					0xA5	/**< OK */
#define PH_ISO14443L3_GEN_ERROR  			0x5A	/**< generic error */
#define PH_ISO14443L3_RATS_ERR				0xF7    /**< Invalid CID in RATS command */
#define PH_ISO14443L3_RX_ERROR              0xC3    /**< CIU reception error */
#define PH_ISO14443L3_FULLBYTE_ERROR        0x4D    /**< last received byte was incomplete */
#define PH_ISO14443L3_PARAM_ERROR           0xD4    /**< invalid function parameter */
#define PH_ISO14443L3_HALTA                 0x50    /**< ISO14443-3 A HALTA command */
#define PH_ISO14443L3_SEL                   0x91    /**< ISO14443-3 A SEL command */
#define PH_ISO14443L3_ATQA_B1_B8            0       /**< ISO14443-3 ATQA bits 1..8 index for pATQA array */
#define PH_ISO14443L3_ATQA_B9_B16           1       /**< ISO14443-3 ATQA bits 9..16 index for pATQA array */
#define PH_ISO14443L3_UID_SIZE_SINGLE       4       /**< single uid size (4 bytes) */
#define PH_ISO14443L3_UID_SIZE_DOUBLE       7       /**< double uid size (7 bytes) */
#define PH_ISO14443L3_UID_SIZE_TRIPLE       10      /**< triple uid size (10 bytes) */
#define PH_ISO14443L3_SAK_INCOMPLETE_MASK   (0x04)  /**< Cascade bit set: UID not complete */
/**\}*/

/*===========================================================================*
 * Types:
 *===========================================================================*/
typedef u16 phddCiu_ErrorCode_t;

/** \addtogroup M_phIso14443L3
*  \{*/
/** @brief States of ISO 14443 layer 3 type A protocol */
typedef enum
{
    PH_ISO14443L3_IDLE_STATE  = 0xf0, /**< IDLE state */
    PH_ISO14443L3_READY_STATE,        /**< READY state */
    PH_ISO14443L3_ACTIVE_STATE,       /**< ACTIVE state */
    PH_ISO14443L3_HALT_STATE,         /**< READY state */
    PH_ISO14443L3_READY_S_STATE,      /**< READY* state */
    PH_ISO14443L3_ACTIVE_S_STATE,     /**< ACTIVE* state */
    PH_ISO14443L3_ISO14443L4_STATE    /**< entered ISO 14443 layer 4 protocol */
} phIso14443L3_State_t;

/** @brief reason of state transition */
typedef enum
{
    PH_ISO14443L3_REASON_REC_ERROR,            /**< CIU reception error */
    PH_ISO14443L3_REASON_FULLBYTE_ERROR,       /**< last received byte was incomplete */
    PH_ISO14443L3_REASON_INV_CMD,              /**< invalid command */
    PH_ISO14443L3_REASON_ISO_CMD,              /**< ISO14443 command */
    PH_ISO14443L3_REASON_CL2BYPASS,            /**< Bypass of cascade level 2 */
    PH_ISO14443L3_REASON_RATS_ERR,			   /**< Invalid CID in RATS command */
    PH_ISO14443L3_REASON_NONE                  /**< no transition */
} phIso14443L3_Reason_t;

/**
 * \ingroup M_phIso14443L3
 * \brief structure describing coding of ATQA */
#define PH_ISO14443L3_ATQA_UIDSZ_SINGLE  0        /**< single size UID (4 byte) */
#define PH_ISO14443L3_ATQA_UIDSZ_DOUBLE  1        /**< double size UID (7 byte) */
#define PH_ISO14443L3_ATQA_UIDSZ_TRIPLE  2        /**< triple size UID (10 byte) */
typedef struct
{
    union
    {
        struct
        {
            u8     bf_ac   : 5;        /**< Bit frame anticollision */
            u8     rfu     : 1;        /**< RFU */
            u8 uid_sz      : 2;        /**< UID size bit frame */
        } bitsL;
        u8 wholeByteL;
    } b1b8; /**< bits 1 to 8 according to description in ISO/IEC FDIS 14443-3:2009, 6.5.2.1 Coding of ATQA */
    union
    {
        struct
        {
            u8 prop_code   : 4;        /**< Proprietary coding */
            u8 rfu2        : 4;        /**< RFU */
        } bitsH;
        u8 wholeByteH;
    } b9b16; /**< bits 9 to 16 according to description in ISO/IEC FDIS 14443-3:2009, 6.5.2.1 Coding of ATQA */
} phIso14443L3_AtqA_t,      /**< structure describing coding of ATQA */
    *pphIso14443L3_AtqA_t;  /**< pointer to structure describing coding of ATQA */

typedef struct
{
    ppIso7816Util_Buffer_t                      pCiuBuffer;     /**< CIU RxTx buffer for data exchange */
    phIso14443L3_State_t					              state;          /**< ISO 14443 layer 3 state */
    u8					                       	reason;         /**< reason for last state transition */
    phIso14443L3_AtqA_t                         atqa;           /**< array holding the atq */
    u8                                     		sakComplete;    /**< sak u8 for final cascade level*/
    u8                                     		sakIncomplete;  /**< sak u8 for other cascade levels */
    u8                                        	bBypassCl2;     /**< allows bypassing of CL2 for legacy reasons */
    u8                                        	bBypassCl2Active;/**< stores if CL2 bypass is active */
} phIso14443L3_Control_t, /**< ISO 14443 layer 3 type A protocol control structure */
    *pphIso14443L3_Control_t; /**< pointer to ISO 14443 layer 3 type A protocol control structure */

typedef struct
{
#ifdef PHFL_TRIPLE_UID
    u8                uid[15];        /**< array holding the uid */
#else
    u8                uid[15];        /**< array holding the uid */
#endif
} phIso14443L3_Control2_t, /**< ISO 14443 layer 3 type A protocol control structure */
    *pphIso14443L3_Control2_t; /**< pointer to ISO 14443 layer 3 type A protocol control structure */

extern phIso14443L3_Control_t				phIso14443L3_control;

/**\}*/
/*===========================================================================*
 * Exported functions:
 *===========================================================================*/

extern  phIso14443L3_State_t 	iso14443TaL3_Transmit( void );
extern  phIso14443L3_State_t 	iso14443TaL3_Deselect( void )		SMALL_MODE;
extern  void				 	iso14443TaL3_Config( void );
extern  void				 	iso14443TaL3_Init( void );
extern  void				 	iso14443TaL3_SetState(phIso14443L3_State_t State );
extern  void 				 	iso14443TaL3_SetSakComplete(u8 sakComplete);
extern  u8 					 	iso14443TaL3_GetSakIncomplete(void);
extern  void 				 	iso14443TaL3_SetSakIncomplete(u8 sakIncomplete);
extern  u8 					 	iso14443TaL3_GetUid(PX8 uidAry);
extern  void 				 	iso14443TaL3_SetUid(PM8 uidAry);
extern  u16                     iso14443TaL3_GetAtqa(void);
extern  void                    iso14443TaL3_SetAtqa(PM8 atqaArray);
#define phIso14443L3_GetState()	phIso14443L3_control.state

extern phIso14443L3_Control2_t	phIso14443L3_control2;
extern  phIso14443L3_State_t iso14443TaL3_StateMachine( void );

#define iso14443TaL3_GetSakComplete(void)       phIso14443L3_control.sakComplete;
#endif
