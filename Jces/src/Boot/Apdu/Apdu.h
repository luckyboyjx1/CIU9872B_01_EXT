#ifndef __APDU_H__
#define	__APDU_H__

#include "DataOperation.h"
#include "SysConfig.h"
#include "BootErrorHandle.h"
#include "bootConfigProfile.h"
#include "SysPlatformConfigProfile.h"

#define APDU_PROTOCOL_T0						0x00
#define APDU_PROTOCOL_T1						0x01
#define APDU_PROTOCOL_TA                  		0x02
#define APDU_PROTOCOL_TB                  		0x03

#define PROTOCOL_MEDIA_MASK                       U32t8(0xF0)
#define PROTOCOL_MEDIA_DEFAULT                    U32t8(0x00)
#define PROTOCOL_MEDIA_CONTACTLESS_TYPE_A         U32t8(0x80)
#define PROTOCOL_MEDIA_CONTACTLESS_TYPE_B         U32t8(0x90)
#define PROTOCOL_MEDIA_USB                        U32t8(0xA0)
#define PROTOCOL_MEDIA_CONTACTLESS_TYPE_F		  U32t8(0xB0)

#define APDU_CLA_OFFSET    						0
#define APDU_INS_OFFSET							1
#define APDU_P1_OFFSET							2
#define APDU_P2_OFFSET     						3
#define APDU_P3_OFFSET     						4
#define APDU_DATA_OFFSET   						5
#define APDU_HEADER_LENGTH						5
#define APDU_EXT_CDATA_OFFSET					7

#define	APDU_NO_STATUS							U32t16(IO_NO_STATUS)

/* APDU_EXCEPTION */
#define APDU_EXCEPTION_ILLEGAL_USE				0xB001
#define APDU_EXCEPTION_BUFFER_BOUNDS			0xB002
#define APDU_EXCEPTION_BAD_LENGTH				0xB003
#define APDU_EXCEPTION_T1_IFD_ABORT				0xB0AB
#define APDU_EXCEPTION_NO_T0_REISSUE			0xB0AC

#define ISO7816_SW_WRONG_LENGTH					0x6700

// APDU exceptions
#define ILLEGAL_USE 							1
#define BUFFER_BOUNDS 							2
#define BAD_LENGTH 								3
#define IO_ERROR 								4
#define NO_T0_GETRESPONSE 						0xAA
#define T1_IFD_ABORT							0xAB


#define APDU_CMD_MASK     						0xB0
#define APDU_CLA_B3B4_MASK						0x0C
#define APDU_CLA_B6_MASK						0x40
#define APDU_SM_MASK      						0x0C
//add by  chenyunhui
#define APDU_CHAINING_MASK						0x10
#define APDU_ISOCLASS_MASK						0x80
#define APDU_ISType16CLA_MASK 					0x40
#define APDU_SM_MASK_TYPE4						0x0C
#define APDU_SM_MASK_TYPE16						0x20

#define STATE_INITIAL                   		0
#define STATE_PARTIAL_INCOMING          		1
#define STATE_FULL_INCOMING             		2
#define STATE_OUTGOING                  		3
#define STATE_OUTGOING_LENGTH_KNOWN     		4
#define STATE_PARTIAL_OUTGOING          		5
#define STATE_FULL_OUTGOING             		6
#define STATE_ERROR_NO_T0_GETRESPONSE  			-1
#define STATE_ERROR_T1_IFD_ABORT       			-2
#define STATE_ERROR_IO                 			-3
#define STATE_ERROR_NO_T0_REISSUE      			-4

#define APDU_NULL_CALL_BACK						0

#if BOOT__IO__APDU_EXTENDED_LENGTH
#define MAX_EXT_LE 32767
#endif

extern TYPE_JUP_BUF	_deselect_env;

typedef struct
{
	u8 SYS_SET_INCOMING :1; // bit 0 : APDU set incoming and receive flag
	u8 SET_INCOMING 	:1; // bit 0 : APDU set incoming and receive flag
	u8 SET_OUTGOING 	:1; // bit 1 : APDU set outgoing flag
	u8 SET_OUTGOING_LEN :1; // bit 2 : APDU set outgoing length flag
//	u8 LR_IS_256 :1; // bit 3 : If LR = 256 flag
//	u8 T1_CHAIN_ABORT :1; // bit 4 : The flag for recording terminal send a s-block to abort chaining block.
//	u8 T1_RESYNCH :1; // bit 5 :
	u8 SET_OUTGOING_AND_SEND :1;// bit 6 :
	u8 SET_OUTGOING_NO_CHAINING :1; // bit 7:
//	u8 NO_GET_REPONSE :1;  //该标志位用于send616cxx() get reponse是否成功
//	u8 SEND_IN_PROGRESS :1;	// 该标志位 用于是否在发送过程中，若为0，表示卡没有在发数据要发数据得先发INS
#if BOOT__IO__APDU_EXTENDED_LENGTH
//	u8 envelopeFlag :1;
	u8 extendedAPDUFlag :1;
//	u8 extendedSupportFlag :1;
//	u8 determinedLE :1;
#endif
} APDUStatus; /* Declare a status byte*/

/*===========================================================================*/
/** @brief  Structure to hold a complete command APDU. */
/*===========================================================================*/
typedef struct pApdu_Command
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
} pApdu_Command_t,
    *ppApdu_Command_t; /**< pointer to structure pphIso7816L4_Command_t */


/*===========================================================================*/
/** @brief  Structure to hold a complete command APDU. */
/*===========================================================================*/
typedef struct pApdu_Response
{
	PX8							pData;    /**< pointer to response buffer */
	u16							lr;
	u16							length;   /**< length of response */
	u16							SW1SW2;   /**< status word */
} pApdu_Response_t,
    *ppApdu_Response_t; /**< pointer to structure phIso7816L4_Response_t */

typedef boolean ( SETOUTGOING_CALLBACK )( u16 offset, u16 len );

#if BOOT__IO__APDU
	extern	u8 	    apdu_GetLogicalChannel(void);
	extern	void    apdu_SetCommand(PX8 commandBuf, u16 size);
	extern	void    apdu_GetCommand( ppApdu_Command_t * ppCommand );
	extern	void    apdu_GetResponse( ppApdu_Response_t *ppResponse );
	extern	u16 	apdu_GetResLen(void);
	extern	void 	apdu_SetResLen(u16 len);
	extern	void    apdu_Init(u8 isDeselect)    SMALL_MODE;
	extern	u8	    apdu_GetProtocol(void);
	extern  u8      apdu_getCLAChannel(void);
	extern	void    apdu_FetchComand(PX8 apduBuffer);
	extern	void    apdu_ReceiveFistComand(void);
	extern	void    apdu_SendStatusAndReceiveCommand(u16 status);
#if (JAVA_CARD_UICC||BOOTLOADER_PRO)//modify by jiangxiaohui@20220324  分层
	extern  void    apdu_SendStatusAndReceiveCommandSingle(u16 status);
#endif
	extern	u16	    apdu_ApduSetIncomingAndReceive(void);
	extern	void 	apdu_UndoSetIncomingAndReceive(void);
	extern	void    apdu_ApduSendBytes(u16 bOff, u16 len);
	extern	void    apdu_ApduSetOutgoingAndSend(u16 bOff, u16 len) SMALL_MODE;
	extern	void	apdu_InitSetOutGoingCallBack(SETOUTGOING_CALLBACK * callBackFun);
	extern  boolean apdu_IsSetIncoming(void);
	extern  void    apdu_SuspendAllInterrupts(void);
	extern  void    apdu_ResumeAllInterrupts(void);
	extern  void apdu_SendStatus(u16 status);
	#define apdu_SetDeselectHandler()       (setjmp(_deselect_env))      /*引用的是Io_14443TaL4的全局，需测试*/
	#if		HAL__IO__ISO14443_TA
			extern  u8	 	apdu_GetSakComplete(void);
			extern  void 	apdu_SetSakComplete(u8 sakComplete);
			extern  u8	 	apdu_GetSakIncomplete(void);
			extern  void 	apdu_SetSakIncomplete(u8 sakIncomplete);
			extern  u8	 	apdu_GetUid(PX8 uidAry);
			extern  void 	apdu_SetUid(PM8 uidAry);
			extern  u16     apdu_GetAtqa(void);
			extern  void    apdu_SetAtqa(PM8 atqaAry);
	#else
			#define apdu_GetSakComplete()														0
			#define apdu_SetSakComplete(sakComplete)
			#define apdu_GetSakIncomplete()														0
			#define apdu_SetSakIncomplete(sakIncomplete)										0
			#define apdu_GetUid(uidAry)															0
			#define apdu_SetUid(uidAry)
			#define apdu_GetAtqa()																0
			#define apdu_SetAtqa(atqaAry)
	#endif

#else
	#define	apdu_GetLogicalChannel()													0
	#define	apdu_SetCommand(commandBuf, size)
	#define	apdu_GetCommand(ppCommand )
	#define	apdu_GetResponse(ppResponse )
	#define apdu_GetResLen()															0
	#define apdu_SetResLen(len)
	#define	apdu_Init(isDeselect)
	#define	apdu_GetProtocol(void)														0
    #define apdu_getCLAChannel()                                                        0
	#define	apdu_FetchComand(apduBuffer)
	#define apdu_InitSetOutGoingCallBack(callBackFun)									0
	#define	apdu_IsSetIncoming()
	#define	apdu_ReceiveFistComand()
	#define	apdu_SendStatusAndReceiveCommand(status)
	#define	apdu_ApduSetIncomingAndReceive()											0
	#define	apdu_UndoSetIncomingAndReceive()
	#define	apdu_ApduSendBytes(bOff, len)
	#define	apdu_ApduSetOutgoingAndSend(bOff, len)
	#define apdu_SuspendAllInterrupts()
	#define apdu_WtxTimerWakeUp()
	
	#define apdu_SetDeselectHandler()
	#define apdu_GetSakComplete()														0
	#define apdu_SetSakComplete(sakComplete)
	#define apdu_GetSakIncomplete()														0
	#define apdu_SetSakIncomplete(sakIncomplete)										0
	#define apdu_GetUid(uidAry)															0
	#define apdu_SetUid(uidAry)
	#define apdu_GetAtqa()																0
	#define apdu_SetAtqa(atqaAry)
#endif

#if (BOOT__IO__APDU_API && BOOT__IO__APDU)
	extern	u8 	 apdu_GetCurrentState(void);
	extern	u16  apdu_getInBlockSize(void);
	extern	u16  apdu_GetOutBlockSize(void);
	extern	void apdu_WaitExtension(void);
	extern	u8 	 apdu_IsValidCLA(void);
	extern	u16  apdu_SetOutgoing(void);
	extern	void apdu_SendBytesLong(PM8 ArrayAddr, u16 bOff, u16 len)    SMALL_MODE;
	extern	u16  apdu_SetOutgoingNoChaining(void);
	extern	void apdu_SetOutgoingLength(u16 len)     SMALL_MODE;
	extern	u16  apdu_ReceiveBytes(u16 bOff);
	extern	u16  apdu_GetIncomingLength(void);
	extern	u8 	 apdu_GetNAD(void);
	extern	u16  apdu_GetOffsetCdata(void);
	extern	u8	 apdu_IsCommandChainingCLA(void);
	extern	u8	 apdu_IsISOInterindustryCLA(void);
	extern	u8	 apdu_IsSecureMessagingCLA(void);
#else
	#define	apdu_GetCurrentState()											0
	#define	apdu_getInBlockSize()											0
	#define	apdu_GetOutBlockSize()											0
	#define	apdu_WaitExtension()
	#define	apdu_IsValidCLA()												0
	#define	apdu_SetOutgoing()												0
	#define	apdu_SendBytesLong(ArrayAddr, bOff, len)
	#define	apdu_SetOutgoingNoChaining()									0
	#define	apdu_SetOutgoingLength(len)										0
	#define	apdu_ReceiveBytes(bOff)											0
	#define	apdu_GetIncomingLength()										0
	#define	apdu_GetNAD()													0
	#define	apdu_GetOffsetCdata()											0
	#define	apdu_IsCommandChainingCLA()										0
	#define	apdu_IsISOInterindustryCLA()									0
	#define	apdu_IsSecureMessagingCLA()										0
#endif


#endif
