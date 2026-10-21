#ifndef __IO_7816T1_H__
#define	__IO_7816T1_H__

#include "DataOperation.h"
#include "UartHal.h"
#include "7816Util.h"
#include "WtxTimer.h"
#include "bootConfigProfile.h"

#define	T1_PPS_SUCESS										1
#define T1_PPS_FAIL											0

#define PH_ISO7816T1_OK     								0xA5
#define PH_ISO7816T1_ERROR  								0xC3

/*=================================================================*
 * Types:
 *=================================================================*/
/** Main States of T1 protocol */
typedef enum
{
	PH_ISO7816T1_STATE_RX = 0xf0, // receive state
	PH_ISO7816T1_STATE_TX,        // transmit state
	PH_ISO7816T1_STATE_ERROR      // error state

} phIso7816T1_State_t;


/** TX Substates of T1 protocol */
typedef enum
{
	PH_ISO7816T1_TXSUB_READY = 0x70,  // ready to send
	PH_ISO7816T1_TXSUB_S_BWT_ACK,     // send S BLOCK BWT RESPONSE
	PH_ISO7816T1_TXSUB_I,             // send I BLOCK  RESPONSE
	PH_ISO7816T1_TXSUB_S_IFS_ACK,     // send S BLOCK IFS RESPONSE
	PH_ISO7816T1_TXSUB_S_IFS_REQUEST, // send S BLOCK IFS REQUEST
	PH_ISO7816T1_TXSUB_ACK            // send ACK

} phIso7816T1_TxSubState_t;


/** RX Substates of T1 protocol
 * These states represent blocks, that can be expected by the protocol.
 */
typedef enum
{
	PH_ISO7816T1_RXSUB_READY,         // ready to receive
	PH_ISO7816T1_RXSUB_S_IFS_ACK,     // expect S BLOCK IFS RESPONSE
	PH_ISO7816T1_RXSUB_S_BWT_ACK,     // expect S BLOCK BWT RESPONSE
	PH_ISO7816T1_RXSUB_I_BLOCK,       // expect I BLOCK
	PH_ISO7816T1_RXSUB_EXPECT_R_ACK   // expect R BLOCK RESPONSE

} phIso7816T1_RxSubState_t;

/*=================================================================*
 *T1 protocol gIso7816T1_Ctrl structure
 *=================================================================*/
typedef struct
{
	ppIso7816Util_Buffer_t					pUartBuffer;  // UART RxTx buffer for data exchange
	u8							seqNumTerm;   // sequence number of the terminal/reader
	u8                  		seqNumCard;   // sequence number of the card
	u16                 		ifs;          // information field size
	u8                  		wtxm;         // waiting time extension multiplier
	phIso7816T1_RxSubState_t 	rxState;      // rx sub state
	phIso7816T1_TxSubState_t 	txState;      // tx sub state
	phIso7816T1_State_t      	state;        // T=1 state
	u8							T1CheckSumType;
	u8 							R_block_count;
	u16							CWT;
	u8							t1_nad[ISO_MAX_LOGICAL_CHANNELS];

} phIso7816T1_Control_t, *pphIso7816T1_Control_t;

/*=================================================================*
 * Defines:
 *=================================================================*/
#if HAL__IO__ISO7816_T1
	extern 	void iso7816T1_Init(void);
	extern	u16  iso7816T1_Receive(void);
	extern	u16  iso7816T1_Transmit(void);
	extern	void iso7816T1_SendStatus(void);
	extern	void iso7816T1_RecieveCommand(void);
	extern	void iso7816T1_AnswerPps(u8 ppsResult, u8 bCount);
	extern	u8 	 iso7816T1_GetNad(void);
	extern	u8   iso7816T1_SetWtxm( u8 wtxm );
	extern	void iso7816T1_RequestWtx(void);
	extern	u32  iso7816T1_GetT1WtxTimerValue(void);
#else
	#define iso7816T1_Init()								while(1)
	#define iso7816T1_Receive()								0
	#define iso7816T1_Transmit()							0
	#define iso7816T1_SendStatus();
	#define iso7816T1_RecieveCommand()
	#define iso7816T1_AnswerPps(ppsResult, bCount)			while(1)
	#define	iso7816T1_GetNad()								0
	#define iso7816T1_SetWtxm(wtxm)							0
	#define iso7816T1_RequestWtx()							while(1)
	#define iso7816T1_GetT1WtxTimerValue()					0
#endif

#endif
