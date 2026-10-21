#ifndef __IO_14443_TA_L4_H__
#define	__IO_14443_TA_L4_H__

#include "DataOperation.h"
#include "Io_14443TaL3.h"
#include "SysConfig.h"
#include "bootConfigProfile.h"
#include "MifareHal.h"
#include "7816Util.h"
#include "WtxTimer.h"
#include "setjmp.h"

/*============================================================================*
 * Project include files:
 *============================================================================*/


/*============================================================================*
 * Defines:
 *============================================================================*/
/** addtogroup M_phIso14443L4
*  {*/

/********************************************************************************* */
/*brief return codes */
#define PH_ISO14443L4_OK                		0xA5A5  /**< return code for success */
#define PH_ISO14443L4_ERROR             		0x5A5A  /**< return code for an error */
#define PH_ISO14443L4_ACTIVE            		0x3CC3  /**< return code */
#define PH_ISO14443L4_ACTIVE_PPS        		0xC33C  /**< return code */
#define PH_ISO14443L4_ACTIVE_FULLBUF    		0xFCCF  /**< reception buffer is full, but chaining is ongoing */

/*********************************************************************************	 */
/*brief flags  */
#define PH_ISO14443L4_FLAG_NONE         		0x00    /**< normal operation. */
#define PH_ISO14443L4_FLAG_BYPASSPARSER 		0x01    /**< bypass Iso7816 command / response parser */
#define PH_ISO14443L4_FLAG_RECFROMBUF   		0x02    /**< receive commands from buffer instead of contactless interface */
#define PH_ISO14443L4_FLAG_EXTCHAINING  		0x04    /**< enable extended (manual) chaining, this directly sets the chaining bit in the PCB */
#define PH_ISO14443L4_FLAG_ACTIVATE_L4  		0x08    /**< ISO/IEC 14443-4 Activation requested, do not allow activation of 14443-3 only */

/********************************************************************************* */
/*brief useful constants*/
#define PH_ISO14443L4_I_BLOCK_MASK      		0xE2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_I_BLOCK           		0x02    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_S_BLOCK_MASK      		0xF7    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_R_BLOCK_MASK      		0xE6    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_R_BLOCK           		0xA2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_NAD_BIT           		0x04    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_NAK_BIT           		0x10    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_DESELECT          		0xC2    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_CHAINING_BIT      		0x10    /**< refer to ISO/IEC 14443-4 7.1.1.1 */
#define PH_ISO14443L4_PROLOGUE_SIZE     		2       /**< prologue size */

#define PH_EMV_I_BLOCK_MASK						0xC2    /**for EMV test */
#define PH_EMV_R_BLOCK_MASK      				0xE4    /**for EMV test */
#define PH_EMV_S_BLOCK_MASK      				0xF4    /**for EMV test */
#define PH_EMV_R_BLOCK							0xA0    /**for EMV test */
#define PH_EMV_DESELECT							0xC0	/**for EMV test */

/** @brief FSCI according to ISO 14443-4, 5.1 / Table 1 */
#define PH_ISO14443L4_FSCI 8

/** @brief Buffer size used for contact less communication */
#if PH_ISO14443L4_FSCI == 8
    #define PH_ISO14443L4_BUFFER_SIZE         	256
#elif PH_ISO14443L4_FSCI == 7
    #define PH_ISO14443L4_BUFFER_SIZE         	128
#elif PH_ISO14443L4_FSCI == 6
    #define PH_ISO14443L4_BUFFER_SIZE          	96
#elif PH_ISO14443L4_FSCI == 5
    #define PH_ISO14443L4_BUFFER_SIZE          	64
#elif PH_ISO14443L4_FSCI == 4
    #define PH_ISO14443L4_BUFFER_SIZE          	48
#elif PH_ISO14443L4_FSCI == 3
    #define PH_ISO14443L4_BUFFER_SIZE          	40
#elif PH_ISO14443L4_FSCI == 2
    #define PH_ISO14443L4_BUFFER_SIZE          	32
#elif PH_ISO14443L4_FSCI == 1
    #define PH_ISO14443L4_BUFFER_SIZE          	24
#elif PH_ISO14443L4_FSCI == 0
    #define PH_ISO14443L4_BUFFER_SIZE          	16
#endif

/** @brief flag for iso14443L4_Control init or not */
#define	PH_ISO14443L4_FLAG_INIT					0x01
#define	PH_ISO14443L4_FLAG_NO_INIT				0x00

#define PHDD_CIU_MEMTYPE                        XDATA
#define PHDD_CIU_MEM_MODEL_FUNC

#define PH_ISO14443L4_MEM_MODEL_FUNC

/*===========================================================================*
 * Types:
 *===========================================================================*/
/** \brief ISO 14443 layer 4 T=CL protocol PPS structure */
typedef struct
{
    u8  PPSS;   /**< Start Byte */
    u8  PPS0;   /**< Parameter 0 */
    u8  PPS1;   /**< Parameter 1 */
} phIso14443L4_PPS_t, /**< ISO 14443 layer 4 T=CL protocol PPS structure */
    *pphIso14443L4_PPS_t; /**< pointer to ISO 14443 layer 4 T=CL protocol PPS structure */

/** \brief ISO 14443 layer 4 T=CL protocol control structure */


typedef struct
{
    ppIso7816Util_Buffer_t                   	pCiuBuffer;       /**< CIU RxTx buffer for data exchange */
    phIso14443L3_State_t          				L3State;            /**< ISO 14443 Layer 3 state */
    u8                       					*pInformation;    /**< pointer to information field */
    u8                       					*pPrologue;       /**< pointer to prolugue field */
    u8                       					*pDataRx;	        /**< pointer to Rx Data field */
    u8                       					*pDataTx;	        /**< pointer to Tx Data field */
	PM8 															pAts;            /**< pointer to external ATS */
    u8                       					sak;              /**< SAK */
    u8                       					fsci;             /**< FSCI */
    u8                       					fsdi;             /**< FSDI */
    u8                       					baudrate;         /**< Baudrate */
    u8                       					cid;              /**< CID */
    u8                       					cidReceived;      /**< to indicate that a CID has been received */
    u8                       					nad;              /**< NAD */
    u8                       					ppsEnable;        /**< to enable and disable PPS */
    u8                       					wtxEnable;        /**< to enable and disable WTX */
    u8                       					blockNumber;      /**< card block number */
    u8                       					pcb;              /**< remember the last (sent) pcb */
    u16                      					length;           /**< remember length of last block for re-transmission */
    u16                      					fullness;         /**< number of bytes still to be transmitted */
    u16                      					bytesPending;     /**< to indicate that data is pending in CIU buffer (application chaining) */
    u8                       					flags;
	u8						 					isInit;
} phIso14443L4_Control_t, /**< ISO 14443 layer 4 T=CL protocol iso14443L4_Control structure */
    *pphIso14443L4_Control_t; /**< pointer to ISO 14443 layer 4 T=CL protocol iso14443L4_Control structure */

typedef u16 phIso14443L4_ErrorCode_t;

/*===========================================================================*
 * Exported functions:
 *===========================================================================*/
extern 	void goto_deselect_handler(u8 flag);
#if HAL__IO__ISO14443_TA
	extern 	phIso14443L4_ErrorCode_t    				 iso14443TaL4_RequestWtx(void);
	#define	iso14443TaL4_SetInitState(initState) iso14443L4_Control.isInit = initState;
	extern	void iso14443Ta_Init( u8 isDeselect )     SMALL_MODE;
	extern	u16  iso14443TaL4_Receive(void);
	extern	u16  iso14443TaL4_Transmit(void);
	extern	void iso14443TaL4_SendStatus(void);
	extern	void iso14443TaL4_RecieveCommand(void);
	extern	u8   iso14443Ta_DataConfigCheck(u8 tagH, u8 tagL, u8 cursecurityState, u8 lifeCycle, u8 isStoreData);
	extern	u8 	 iso14443Ta_DataConfigGenNewValue(u8 tagH, u8 tagL, u8 oldValue, u8 newValue, u8 isStoreData);
	#define iso14443Ta_Config()							iso14443TaL3_Config()
	#define iso14443Ta_GetSakComplete()					iso14443TaL3_GetSakComplete()
//	#define iso14443Ta_SetSakComplete(sakComplete)		iso14443TaL3_SetSakComplete(sakComplete)
	#define iso14443Ta_GetSakIncomplete()				iso14443TaL3_GetSakIncomplete()
	#define io_SetSakIncomplete iso14443Ta_SetSakIncomplete

	#define iso14443Ta_GetUid(uidAry)					iso14443TaL3_GetUid(uidAry)
//	#define iso14443Ta_SetUid(uidAry)					iso14443TaL3_SetUid(uidAry)
	#define iso14443Ta_GetAtqa()						iso14443TaL3_GetAtqa()
//	#define iso14443Ta_SetAtqa(atqaArray)				iso14443TaL3_SetAtqa(atqaArray)
	
	extern	u8 	 iso14443TaL4_GetFwi(void);

#else
	#define iso14443TaL4_RequestWtx( wtxm )		 											0
	#define	iso14443TaL4_SetInitState(initState) 											while(1)
	#define set_deselect_handler()
	#define iso14443Ta_Init(isDeselect )				 											while(1)
//	#define iso14443TaL4_FetchCommand( buf)		 		while(1)
	#define iso14443TaL4_Receive()					 										0
	#define iso14443TaL4_Transmit()				 											0
	#define iso14443TaL4_SendStatus()
	#define iso14443TaL4_RecieveCommand()
	#define iso14443Ta_DataConfigCheck(tagH, tagL, cursecurityState, lifeCycle, isStoreData)				0
	#define iso14443Ta_DataConfigGenNewValue(tagH, tagL, oldValue, newValue, isStoreData)	0
	#define iso14443Ta_Config()
	#define iso14443Ta_GetSakComplete()														0
	#define iso14443Ta_SetSakComplete(sakComplete)											while(1)
	#define iso14443Ta_GetSakIncomplete()													0
	#define iso14443Ta_SetSakIncomplete(sakIncomplete)										while(1)
	#define iso14443Ta_GetUid(uidAry)														0
	#define iso14443Ta_SetUid(uidAry)														while(1)
	#define iso14443Ta_GetAtqa()															0
	#define iso14443Ta_SetAtqa(atqaArray)
	#define iso14443TaL4_GetFwi()															0
#endif

#endif
