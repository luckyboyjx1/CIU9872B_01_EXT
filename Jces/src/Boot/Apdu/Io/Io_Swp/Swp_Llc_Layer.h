
#ifndef __SWP_LLC_LAYER_h__
#define __SWP_LLC_LAYER_h__

#include "ecpDefine.h"
#include "ecpReadWrite.h"

#define  ICCTEST_VER_1_13         1

typedef u32 ( * RFDISP_CallBackTypeA_t ) ( u8 **, u32* );

typedef struct
{
    u8                          SelectedCltApplication;
    u8                          SelectedTypeAApplication;
    u8                          TypeAMode; ///< format of the command used by MIFARE DESFire
    RFDISP_CallBackTypeA_t      TypeAFct;
    u8                          DESFireLibInitStatus;
    u8                          NonMifareAIDSize;
    PM8                         NonMifareAID;
    u8                          ClassicEnabled;
    u8                          DESFireEnabled;
    u16                          DESFireInstance;
    u16                          ClassicInstance;

} RFDISP_Context_t;

typedef enum
{
    cMifareClassicAPIReturnCodeSuccess = 0x30,
    cMifareClassicAPIReturnCodeCallMeAgain = 0x31,
    cMifareClassicAPIReturnCodeNoMemory = 0xE0,
    cMifareClassicAPIReturnCodeWrongState = 0xE1,
    cMifareClassicAPIReturnCodeInvalidInstance = 0xE2,
    cMifareClassicAPIReturnCodeInvalidParameter = 0xE3,
    cMifareClassicAPIReturnCodeErrorPointer = 0xE4,
    cMifareClassicAPIReturnCodeErrorFlashError1 = 0xF0,
    cMifareClassicAPIReturnCodeErrorFlashError2 = 0xF2,
    cMifareClassicAPIReturnCodeErrorFlashError3 = 0xF3,
    cMifareClassicAPIReturnCodeSecurityException = 0xEE
}
tMifareClassicAPIReturnCode;

/// @brief Return value for success
#define RFDISP_OK                           0x00
/// @brief Return value for error
#define RFDISP_NOK                          0xFF

/// @brief Size of buffer used to store the answer to MIFARE commands
#define RFDISP_BUFFER_SIZE                  260

/// @brief Value when no application is selected for CLT frames
#define RFDISP_NO_APP_SELECTED              0xFF
/// @brief Value when Classic application is selected for CLT frames
#define RFDISP_CLT_MIFARE_SELECTED          0x01
/// @brief Value when a Non-MIFARE application is selected for CLT frames
#define RFDISP_CLT_NONMIFARE_SELECTED       0x02
/// @brief Mask to get check if a MIFARE Classic command is an authentication command
#define RFDISP_MIFARE_CLASSIC_AUTH_CMD_MASK 0xF0
/// @brief Value of a MIFARE Classic authentication command
#define RFDISP_MIFARE_CLASSIC_AUTH_CMD_BYTE 0x60

/// @brief Value when DESFire application is selected for Type A commands
#define RFDISP_TYPEA_MIFARE_SELECTED        0x10
/// @brief Value when a non-MIFARE application is selected for Type A commands
#define RFDISP_TYPEA_NONMIFARE_SELECTED     0x20
/// @brief Value when no mode is selected for DESFire
#define RFDISP_DESFIRE_MODE_UNKNOWN         0x00
/// @brief Value when native mode is selected for DESFire
#define RFDISP_DESFIRE_MODE_NATIVE          0x01
/// @brief Value when ISo mode is selected for DESFire
#define RFDISP_DESFIRE_MODE_ISO             0x02

/// @brief Value of ISO7816-4 SELECT INS byte
#define RFDISP_ISO_SELECT                   0xA4
/// @brief Size of DESFire EV0 AID
#define RFDISP_DESFIRE_AID_EV0_SIZE         7

/********************************following defines are for SWP_INTERFACE**************************************/
/// @brief LLC CLT layer: mask to get admin field
#define SWP_LLC_CLT_ADMIN_FIELD_MASK        0x0F
/// @brief LLC CLT layer: mask to get data field
#define SWP_LLC_CLT_DATA_FIELD_MASK         0x10
/// @brief LLC CLT layer: no admin command in CLT frame
#define SWP_LLC_CLT_NO_ADM_CMD              0x00
/// @brief LLC CLT layer: CL_PROTO_INF(A) command
#define SWP_LLC_CLT_CL_PROTO_INF_A          0x08
/// @brief LLC CLT layer: CL_PROTO_INF(F) command
#define SWP_LLC_CLT_CL_PROTO_INF_F          0x09
/// @brief LLC CLT layer: GOTO_INIT command
#define SWP_LLC_CLT_CL_GOTO_INIT            0x01
/// @brief LLC CLT layer: GOTO_HALT command
#define SWP_LLC_CLT_CL_GOTO_HALT            0x02
/// @brief Internal library state when UA has been sent
#define SWP_ESTABLISHED_LINK_STATE          0x38
/// @brief Internal libray state to sent a RSET to CLF
#define SWP_SEND_RSET_STATE                 0x30
/// @brief Internal libray state to sent a UA to CLF
#define SWP_SEND_UA_STATE                   0x34


/// @brief number of received parameters for ADM_NOTIFY_PIPE_CREATED
#define ADM_NOTIFY_PIPE_CREATED_PARAM_SIZE  (1 + 5)
/// @brief number of received parameters for ADM_NOTIFY_PIPE_DELETED
#define ADM_NOTIFY_PIPE_DELETED_PARAM_SIZE  (1 + 1)
/// @brief number of received parameters for ADM_NOTIFY_PIPE_DELETED
#define ADM_NOTIFY_ALL_PIPE_CLEARED_PARAM_SIZE  (1 + 1)

#define SWP_LLC_GetLpduData()  ((u8 *)g_SWP_LLC_Context.ReadLpdu->Data)
#define SWP_LLC_GetLpduSize()  ((u8)g_SWP_LLC_Context.ReadLpdu->Size)
#define SWP_LLC_SendLpduData()  ((u8 *)g_SWP_LLC_Context.WriteLpdu->Data)
#define SWP_LLC_SendLpduSize()  ((u8)g_SWP_LLC_Context.WriteLpdu->Size)

#define SWP_LLC_SHDLCLinkEstablished() (g_SWP_LLC_Context.Status >= SWP_ESTABLISHED_LINK_STATE)


/********************************following defines are for LLC**************************************/
#define SWP_LLC_LOW_POWER_MODE        0x00
#define SWP_LLC_FULL_POWER_MODE       0x01


#define ACT_CTRL_MASK               0x60
#define SHDLC_CTRL_MASK             0x80
#define CLT_CTRL_MASK               0x40

#define SHDLC_RR                0x00    ///< used only 2 Bits  4 & 3
#define SHDLC_REJ               0x08    ///< used only 2 Bits  4 & 3
#define SHDLC_RNR               0x10    ///< used only 2 Bits  4 & 3
#define SHDLC_SREJ              0x18    ///< used only 2 Bits  4 & 3

#define SHDLC_REJ_FALSE         0
#define SHDLC_REJ_TRUE          1

#define SWP_LLC_295NS_SUPPORTED         2
#define SWP_LLC_590NS_SUPPORTED         1
#define SWP_LLC_10000NS_SUPPORTED       0

#define DEFAULT_WS                      4       // Windows Sliding is set to 2
#define DEFAULT_SREJ                    0       // SREJ is not yet supported

#define SWP_LLC_MAXRETRANSMIT           3       //Number of maximum retransmission allowed for Transmit timeout

#define SWP_LLC_CTRL_MASK               0xe0
#define SWP_LLC_CLT_CODE                0x40
#define SWP_LLC_ACT_CODE                0x60
#define SWP_LLC_SHDLC_CODE              0x80


#define SWP_LLC_INITIALIZED             0x00
#define SWP_LLC_CLOSED                  0x10
#define SWP_LLC_OPENED                  0x11


#define SWP_LLC_ACT_SYNC_SENT           0x20
#define SWP_LLC_ACT_STOPPED             0x28
#define SWP_LLC_ACT_READY_SENT          0x2C
#define SWP_LLC_SHD_RSET_RECEIVED       0x30    // It means that we have to send our RSET Config
#define SWP_LLC_SHD_RSET_SENT           0x34
#define SWP_LLC_SHD_UA                  0x38
#define SWP_LLC_SHD_I_FRAME             0x3C
#define SWP_LLC_READ_S_FRAME            0x3E
#define SWP_LLC_READ_I_FRAME            0x40
#define SWP_LLC_READ_RNR_S_FRAME        0x44
#define SWP_LLC_RNR_S_FRAME_RESEND      0x46
#define SWP_LLC_READ_RR_S_FRAME         0x48
#define SWP_LLC_READ_REJ_S_FRAME        0x4A
#define SWP_LLC_CLT_SESSION_STARTED     0x80
#define SWP_LLC_CLT_MIFARE_SESSION      0x01
#define SWP_LLC_CLT_NONMIFARE_SESSION   0x02
#define SWP_LLC_CLT_NOK                 0xFF
#define SWP_LLC_CLT_OK                  0x00
#define SWP_LLC_OK                      0x00
#define SWP_LLC_INIT_ERROR              0xE0
#define SWP_LLC_ACTIVATE_ERROR          0xE1
#define SWP_LLC_DEACTIVATE_ERROR        0xE2
#define SWP_LLC_SETSTATUS_ERROR         0xE3
#define SWP_LLC_RECEIVE_ERROR           0xE4
#define SWP_LLC_ACT_READY_SENT_ERROR    0xE5
#define SWP_LLC_SHD_RECEIVED_ERROR      0xE6
#define SWP_LLC_REJ_FRAME               0xE7
#define SWP_LLC_SREJ_FRAME              0xE8
#define SWP_LLC_SEND_BUFFER_SIZE_ERROR  0xE9
#define SWP_WRITEBYTE_OVERFLOW_ERROR    0xEA
#define SWP_RECEIVEBYTE_OVERFLOW_ERROR  0xEB
#define SWP_SENDBLOCK_ERROR             0xEC

#define SWP_EVT_INITIALIZE              0x100
/// @brief Size of LLC buffer ( fixed by SWP spec )
#define LLC_BUF_SIZE                        30

/********************************following defines are for LLC_SHDLC**********************************/
#define SHDLC_FIELD                 0x80      ///< this represent the value of 1 MSB LLC control field dedicated to SHDLC LLC layer
#define SHDLC_S_FRAME               0xc0    ///< used only 3 MSB
#define SHDLC_U_FRAME               0xE0    ///< used only 3 MSB
#define SHDLC_I_FRAME               0x80    ///< used only 2 MSB
#define SHDLC_NS_MASK               0x38
#define SHDLC_NR_MASK               0x07
#define TYPE_SFRAME_MASK            0x18
#define SFRAME_RR                   0x0
#define SFRAME_REJ                  0x1
#define SFRAME_RNR                  0x2
#define SFRAME_SREJ                 0x3 ///< this mode is not supported
#define SHDLC_UA                    0x06    // used last 5 bits
#define SHDLC_RSET                  0x19    // used last 5 bits

/********************************following defines are for LLC_ACT**********************************/


#define ACT_READY                   0x00
#define ACT_SYNC                    0x01
#define ACT_POWER_MODE              0x02
#define ACT_FR_SET                  0x10
#define ACT_INF_SET                 0x08
#define ACT_FIELD                   0x60      ///< this represent the value of 3 MSB LLC control field dedicated to ACT LLC layer
#define ACT_INF                     3         ///< this is the bit number of the ACT LLC control Field dedicated to INF used in UICC to CLF message
#define ACT_FR                      4         ///< this is the bit number of the ACT LLC control Field dedicated to FR used in CLF to UICC message
#define ACT_CTRL_READY              0x00     ///< this represent the value of 3 LSB LLC control field mean ACT_READY used UICC to CLF message
#define ACT_CTRL_POWER              0x02    ///< this represent the value of 3 LSB LLC control field mean ACT_POWER_MODE used CLF to UICC message
#define ACT_CTRL_SYNC               0x01    ///< this represent the value of 3 LSB LLC control field mean ACT_SYNC used UICC to CLF message


/********************************following defines are for LLC_CLT**********************************/
#define CLT_FIELD                   0x40
#define RETURNSUCCESS               0x00000000

#define SWP_CLT_APP_NOK             0xFF
#define SWP_CLT_APP_OK              0x00

#define CLT_MIFARE_SELECTED         0x01
#define CLT_NONMIFARE_SELECTED      0x00


#define BITTOBYTES(bitIndex)             (U32_SHL_U8(1,bitIndex))

/// @brief Type definition for Lpdu structure
typedef struct
{
    u8 Size;
    u8 Data[LLC_BUF_SIZE];   ///< Any LPDu is maximum 30 bytes long
} SWP_LLC_Lpdu_t;

/// @brief Type definition for Sync ID
// typedef union
// {
//     u16 u2SId;
//     u8  u1SId[2];
// } SWP_LLC_SyncId_t;

/// @brief Enumeration for SWP timer type
typedef enum
{
    T1_ACKNOWLEDGE_TIMEOUT,
    T2_GUARDING_TIMEOUT,
    T3_CONNECTION_TIMEOUT,
    TS2_INHIBIT_TIMEOUT,
    T_REJ_FRAMES_RESEND,
    NO_TIMEOUT,
} ShdlcTimeT;

/// @brief Type definition for SWP timer
typedef struct
{
    volatile ShdlcTimeT     Status;
    volatile u16     	      Timercdv;
}SWP_Timer_t;

typedef struct
{
    volatile u8         Status;
    volatile u8         Ns;
    volatile u8         Nr;
    volatile u8         DNr;

//    SWP_LLC_SyncId_t    SyncId; // copy of a value stored in Flash
      u8                  WS;
	  u8                  privNr;
	  u8                  PowerMode;

	  volatile u8         ResendOngoing;
	  u8                  ResendFrameNum;
      u8                  NbRetransmit;
    volatile u8         IrqOccured;  // This value set to False at power on and set to true after SWP Irq


    SWP_LLC_Lpdu_t      *ReadLpdu;
    SWP_LLC_Lpdu_t      *WriteLpdu;
	SWP_LLC_Lpdu_t		ResendLpdu[4];
    SWP_Timer_t         Timer;

}SWP_LLC_t;

extern SWP_LLC_t g_SWP_LLC_Context;
//extern SWP_LLC_SyncId_t g_MGR_SyncId ;
//extern u8 swpActivateSuccess;
//extern volatile u8 gReceRR;
//extern SWP_LlcT g_Llc;
//extern SWP_LLC_SyncId_t g_MGR_SyncId;

/// @addtogroup SWP_LLC_API
/// @{

#define SWP_LLC_WaitForSend( ) ((g_SWP_LLC_Context.Ns >= g_SWP_LLC_Context.DNr) ? \
									(g_SWP_LLC_Context.WS > (g_SWP_LLC_Context.Ns - g_SWP_LLC_Context.DNr)) : \
										(g_SWP_LLC_Context.WS > (g_SWP_LLC_Context.Ns + (8 - g_SWP_LLC_Context.DNr))))

/// @brief Returns 1 if SWP is in deactivated state
//#define SWP_LLC_ACT_IsSwpDeactivated( )         ( r_SWP_STAT & BIT_SET(SWP_STAT_DEACT_STATE_BIT) )

/// @brief Returns 1 if SWP is in Suspend state
//#define SWP_LLC_ACT_IsSwpSuspended( )           ( r_SWP_STAT & BIT_SET(SWP_STAT_SUSP_STATE_BIT) )

/// @brief Returns 1 if SWP is in activated state
//#define SWP_LLC_ACT_IsSwpActivated( )           ( ( r_SWP_STAT & ( BIT_SET(SWP_STAT_ACT_EVT_BIT) ) ) )

extern void setLlcContextSyncId(u32 syncId);
extern void setLlcContextStatus(u8 status);
extern void Swp_Llc_Init( void );
extern void Swp_Llc_Close( void );
extern void Swp_Llc_Open ( void );
extern void Swp_Llc_ClearResendFrame(void);
extern void Swp_Llc_ClearFrame( void );
extern int   SWP_LLC_SetStatus(SWP_LLC_t* StateLlc,int Status);
extern int   SWP_LLC_GetStatus(SWP_LLC_t* StateLlc);
extern int   SWP_LLC_Reset(SWP_LLC_t* StateLlc);
extern void Swp_Llc_StoreFrame(u8 *Data, u8 size);

extern u8   Swp_Llc_Send_StypeFrame(u8 type);
extern void Swp_Llc_Shdlc_CheckDNR(u8 Y, u8 Rej);
extern u8   Swp_Llc_Shdlc_CheckReceivedIframeNsNr(void);
extern u8   Swp_Llc_Shdlc_Receive( void );
extern u8   Swp_Llc_Shdlc_Resend( u8 Ns );
extern u8   Swp_Llc_Shdlc_Send( void );

extern void Swp_Llc_Act_ChangeMode(void);
extern u8   Swp_Llc_Act_Receive (void);
extern u8   Swp_Llc_Act_Send(void);

extern u8   Swp_Llc_Clt_Receive(void);



#endif
