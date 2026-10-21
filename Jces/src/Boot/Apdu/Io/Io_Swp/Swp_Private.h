/***************************************************************************************
 * Copyright 2011 Infineon Technologies AG ( www.infineon.com ).                       *
 * All rights reserved.                                                                *
 *                                                                                     *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,            *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,           *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  IN NO       *
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,                 *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;         *
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY             *
 * WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR            *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF              *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                          *
 *                                                                                     *
 ***************************************************************************************/

/** @cond INCLUDE_FILE_DESCRIPTION */
/**
 * @brief
 * Private defintions of the SWP module.
 *
 * @file
 */
/** @endcond */

#ifndef SWP_PRIVATE_H
#define SWP_PRIVATE_H
/**
 * @addtogroup IFX_SWP_GROUP
 * @{
 */
//#include "SWP_Section.h"

/*  includes
****************************************************************/
#include "ecpDefine.h"
#include "JcvmConfig.h"

/*  pragmas
****************************************************************/

/*  defines
****************************************************************/

/// @brief swp config
///
// peripheral declarations for backward compatibility


/*****************************************************************************/

/// @brief save hcp pipe information
#define SWP_PARA						1

#define DEFAULT_WS                      4      // Windows Sliding is set to 2
#define DEFAULT_SREJ                    0       // SREJ is not yet supported

#define SWP_LLC_MAXRETRANSMIT           3       //Number of maximum retransmission allowed for Transmit timeout

/// @brief Size of LLC buffer ( fixed by SWP spec )
#define LLC_BUF_SIZE                        30

/// @brief Number of gates in GATES_LIST registry entry array
#define HCP_GATES_LIST_SIZE                 6

/// @brief Size of pipes array stored in NVM ( maximum number of pipes )
#define HCP_MAX_PIPES                       10

/// @brief Buffer to store HCP messages ( size is not specified by ETSI
/// but NXP test suite sends maximum 261 bytes messages - APDU of 260 bytes + HCP header byte )
#define HCP_BUF_SIZE                        261


/// @brief HCP hostes ID
///
/// @brief HCP host controller ID
#define HCP_CONTROL_HOST                    0x00
/// @brief HCP Terminal host ID
#define HCP_TERMINAL_HOST                   0x01
/// @brief HCP UICC host ID
#define HCP_UICC_HOST                       0x02

///
/// @brief : HCP Gate Id Macro define
///
/// @brief Gates type identifier for administration
#define HCP_GATE_ID_ADMIN                   0x00
/// @brief Gates type identifier for LINK mgt gate
#define HCP_GATE_ID_LINK_MGT                0x02
/// @brief Gates type identifier for loop back gate
#define HCP_GATE_ID_LOOPBACK                0x04
/// @brief Gates type identifier for identity mgt gate
#define HCP_GATE_ID_IDENTITY_MGT            0x05
/// @brief Gate ID for type A card application gate
#define HCP_GATE_ID_CARD_TYPEA              0x23
// @brief Gate ID for type B card application gate
#define HCP_GATE_ID_CARD_TYPEB		        0x21
/// @brief Gate ID for type A reader RF gate
#define HCP_GATE_ID_READER_TYPEA            0x13
/// @brief Gate ID for type B reader RF gate
#define HCP_GATE_ID_READER_TYPEB            0x11
/// @brief Gate ID for Connectivity gate
#define HCP_GATE_ID_CONNECTIVITY            0x41


/// @brief HCP Pipes ID
///
/// @brief RFU gate ID to be used in Identity Mgt Gate registry for unused GATES_LIST elements
#define HCP_RFU_GATE                        0x06
/// @brief Pipe ID for LINK management gate
#define HCP_LINK_MGT_GATE_PIPE_ID           0x00
/// @brief Pipe ID for administration gate
#define HCP_ADMIN_GATE_PIPE_ID              0x01

/// @brief HCP commands
///
/// @brief HCP command for all gates
#define HCP_CMD_ANY_SET_PARAMETER           0x01
/// @brief HCP command for all gates
#define HCP_CMD_ANY_GET_PARAMETER           0x02
/// @brief HCP command for all gates
#define HCP_CMD_ANY_OPEN_PIPE               0x03
/// @brief HCP command for all gates
#define HCP_CMD_ANY_CLOSE_PIPE              0x04
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_CREATE_PIPE             0x10
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_DELETE_PIPE             0x11
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_NOTIFY_PIPE_CREATED     0x12
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_NOTIFY_PIPE_DELETED     0x13
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_CLEAR_ALL_PIPE          0x14
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_NOTIFY_ALL_PIPE_CLEARED 0x15

/// @brief HCP events
///
/// @brief HCP Event from Host to Host controller to explain end of operation
#define HCP_EVT_HCI_END_OF_OPERATION        0x01
/// @brief HCP Event to send and receive data witha parameter that contains a block of data
#define HCP_EVT_POST_DATA                   0x02
/// @brief HCP Event sent by Host controller to notify connection or disconnection
#define HCP_EVT_HOT_PLUG                    0x03

/// @brief HCP Card Application Gate specific event
#define HCP_CARD_GATE_EVT_SEND_DATA         0x10
/// @brief HCP Card Application Gate specific event
#define HCP_CARD_GATE_EVT_FIELD_ON          0x11
/// @brief HCP Card Application Gate specific event
#define HCP_CARD_GATE_EVT_CARD_DEACTIVATED  0x12
/// @brief HCP Card Application Gate specific event
#define HCP_CARD_GATE_EVT_CARD_ACTIVATED    0x13
/// @brief HCP Card Application Gate specific event
#define HCP_CARD_GATE_EVT_FIELD_OFF         0x14

/// @brief HCP Reader RF Gate and Reader RF Application Gate specific command : WR_XCHG_DATA
#define HCP_READER_GATE_CMD_XCHG_DATA           0x10
/// @brief HCP Reader RF Gate specific event : READER_REQUEST
#define HCP_READER_GATE_EVT_REQUESTED           0x10
/// @brief HCP Reader RF Gate specific event : END_OPERATION
#define HCP_READER_GATE_EVT_END_OPERATION       0x11
/// @brief HCP Reader Application Gate specific event : TARGET_DISCOVERED
#define HCP_READER_GATE_EVT_DISCOVERED          0x10

/// @brief HCP Connectivity Gate specific command : PRO_HOST_REQUEST
#define HCP_CONNECTIVITY_GATE_CMD_HOST_REQUEST   0x10
/// @brief HCP Connectivity Gate specific event : EVT_CONNECTIVITY 
#define HCP_CONNECTIVITY_GATE_EVT_CONNECTIVITY   0x10
/// @brief HCP Connectivity Gate specific event : EVT_TRANSACTION
#define HCP_CONNECTIVITY_GATE_EVT_TRANSACTION    0x12
/// @brief HCP Connectivity Gate specific event : EVT_OPERATION_ENDED
#define HCP_CONNECTIVITY_GATE_EVT_OPERATION_END  0x13
/// @brief HCP Connectivity Application Gate specific event : EVT_STANDBY
#define HCP_CONNECTIVITY_GATE_EVT_STANDBY        0x10


/// @brief HCP message type
///
/// @brief HCP message command type
#define HCP_MSG_COMMAND_TYPE                0x00
/// @brief HCP message event type
#define HCP_MSG_EVENT_TYPE                  0x40
/// @brief HCP message response type
#define HCP_MSG_RESPONSE_TYPE               0x80

/// @brief HCP response messages
///
/// @brief HCP response : Command completed successfully (with optional parameters)
#define HCP_REP_ANY_OK                      0x00
/// @brief HCP response : the destination host is not connected
#define HCP_REP_ANY_E_NOT_CONNECTED         0x01
/// @brief HCP response : the format of the command parameters is wrong
#define HCP_REP_ANY_E_CMD_PAR_UNKNOWN       0x02
/// @brief HCP response : command was rejected and/or not completed
#define HCP_REP_ANY_E_NOK                   0x03
/// @brief HCP response : no more dynamic pipes can be created
#define HCP_REP_ADM_E_NO_PIPES_FULL         0x04
/// @brief HCP response : no more dynamic pipes can be created ETSI 102 622 V9.0.0
#define HCP_REP_ADM_E_NO_PIPE_AVAILABLE     0x04
/// @brief HCP response : the registry parameter identifier is either unknown to the registry or an optional registry parameter is not implemented
#define HCP_REP_ANY_E_REG_PAR_UNKNOWN       0x05
/// @brief HCP response : the pipe is not open
#define HCP_REP_ANY_E_PIPE_NOT_OPENED       0x06
/// @brief HCP response : the command is not supported by the gate
#define HCP_REP_ANY_E_CMD_NOT_SUPPORTED     0x07
/// @brief HCP response : command is inhibited due to failure of lower layer identity check
#define HCP_REP_ANY_E_INHIBITED             0x08
/// @brief HCP response : an application level time-out occurred
#define HCP_REP_ANY_E_TIMEOUT               0x09
/// @brief HCP response : permission denied to write/read a value to/from a registry
#define HCP_REP_ANY_E_REG_ACCESS_DENIED     0x0a
/// @brief HCP response : Permission denied to create a pipe due to a WHITELIST violation
#define HCP_REP_ANY_E_PIPE_ACCESS_DENIED    0x0b

/// @brief HCP return codes
///
/// @brief HCP layer return code : success
#define SWP_HCP_OK                          0x00
/// @brief HCP layer return code : invalid parameter in command or event
#define SWP_HCP_INVALID_PARAMETER           0x01
/// @brief HCP layer return code
#define SWP_HCP_PROCESS_COMMMAND_FINISHED   0x70
/// @brief HCP layer return code
#define SWP_HCP_PROCESS_COMMMAND_ERROR      0xF0
/// @brief HCP layer return code
#define SWP_HCP_SWP_BUFFER_FULL             0xF1
/// @brief HCP layer return code
#define SWP_HCP_BAD_COMMUNICATION           0xFE
/// @brief HCP layer return code
#define SWP_HCP_ERROR_FLASH_PROGRAMMING     0xFF
/// @brief State used to manage transmission during first connection
#define HCP_NOT_TRANSMITTING                0x00
/// @brief State used to manage transmission during first connection
#define HCP_TRANSMISSION_ERROR              0xFF

/// @brief HCP layer states
///
/// @brief HCP layer state : layer initialized
#define SWP_HCP_INITIALIZED                  0x00
/// @brief HCP layer state : ready to transmit
#define SWP_HCP_READY_TO_TRANSMIT            0x01
/// @brief HCP layer state : message transmission on going
#define SWP_HCP_MESSAGE_TRANSMISSION_ONGOING 0x02
/// @brief HCP layer state : process command
#define SWP_HCP_PROCESS_COMMAND				 0x03
/// @brief HCP layer state : loopback event
#define SWP_HCP_CARD_LOOPBACK				 0x04
/// @brief HCP layer state : receive data from card mode
#define SWP_HCP_CARD_SENT_DATA               0x05

/// @brief HCP layer state : field off event
#define SWP_HCP_CARD_FIELD_OFF               0x01
/// @brief HCP layer state : deactivate event
#define SWP_HCP_CARD_DEACTIVATED             0x02
/// @brief HCP layer state : field on
#define SWP_HCP_CARD_FIELD_ON                0x03
/// @brief HCP layer state : activate event
#define SWP_HCP_CARD_ACTIVATED               0x04

/// @brief HCP packet mask
///
/// @brief Mask to get HCP packet chaining bit
#define HCP_PACKET_CB_MASK                  0x80
/// @brief Mask to get HCP packet pipe ID
#define HCP_PACKET_PIPEID_MASK              0x7F
/// @brief Mask to get message instruction type
#define HCP_MSG_TYPE_MASK                   0xc0
/// @brief Mask to get message instruction
#define HCP_MSG_INSTRUCTION_MASK            0x3F

/// @brief HCP packet status
#define HCP_PKG_NO_HEADER                   0x00
#define HCP_PKG_IS_HEADER                   0x01
#define HCP_PKG_COMPLETE                    0x80

/// @brief Pipe state
#define HCP_PIPE_NOT_CREATED                0xFF
/// @brief Pipe state
#define HCP_PIPE_CLOSED                     0xF8
/// @brief Pipe state
#define HCP_PIPE_OPENED                     0x88

/// @brief Macro for size of HCP_Pipe_t structure
#define HCP_SIZEOF_PIPE     6

/// @brief contantless interface need to do reset operation
#define SWP_HCP_DETECT_FIELDOFF    1

/// @addtogroup SWP_LLC
/// @{

#define ACT_CTRL_MASK               0x60
#define SHDLC_CTRL_MASK             0x80
#define CLT_CTRL_MASK               0x40


#define ACT_READY                   0x00
#define ACT_SYNC                    0x01
#define ACT_POWER_MODE              0x02
#define ACT_FR_SET                  0x10
#define ACT_INF_SET                 0x08

#define ACT_FIELD                   0x60      ///< this represent the value of 3 MSB LLC control field dedicated to ACT LLC layer
#define SHDLC_FIELD                 0x80      ///< this represent the value of 1 MSB LLC control field dedicated to SHDLC LLC layer
#define CLT_FIELD                   0x40      ///< this represent the value of 3 MSB LLC control field dedicated to CLT LLC layer

#define ACT_INF                     3         ///< this is the bit number of the ACT LLC control Field dedicated to INF used in UICC to CLF message
#define ACT_FR                      4         ///< this is the bit number of the ACT LLC control Field dedicated to FR used in CLF to UICC message    
#define ACT_CTRL_READY              0x00     ///< this represent the value of 3 LSB LLC control field mean ACT_READY used UICC to CLF message
#define ACT_CTRL_POWER              0x02    ///< this represent the value of 3 LSB LLC control field mean ACT_POWER_MODE used CLF to UICC message
#define ACT_CTRL_SYNC               0x01    ///< this represent the value of 3 LSB LLC control field mean ACT_SYNC used UICC to CLF message

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
#define IFRAME_RECE                 0x4

#define SHDLC_RR                    0x00    ///< used only 2 Bits  4 & 3
#define SHDLC_REJ                   0x08    ///< used only 2 Bits  4 & 3
#define SHDLC_RNR                   0x10    ///< used only 2 Bits  4 & 3
#define SHDLC_SREJ                  0x18    ///< used only 2 Bits  4 & 3

#define SHDLC_UA                    0x06    // used last 5 bits
#define SHDLC_RSET                  0x19    // used last 5 bits

/// @brief swp llc states
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
#define SWP_LLC_CLT_SESSION_STARTED     0x80
#define SWP_LLC_CLT_MIFARE_SESSION      0x01
#define SWP_LLC_CLT_NONMIFARE_SESSION   0x02
#define SWP_LLC_CLT_NOK                 0xFF
#define SWP_LLC_CLT_OK                  0x00

/// @brief Internal library state when UA has been sent
#define SWP_ESTABLISHED_LINK_STATE      0x38

/// @brief LLC return values
///
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


#define SWP_LOW_POWER_MODE              0x00
#define SWP_FULL_POWER_MODE             0x01

/// @}


/// @{ add group SWP MAC

#define SWP_MAC_OK                      0x00

#define SWP_MAC_INIT_ERROR              0xFF70
#define SWP_MAC_ACTIVATE_ERROR          0xFF71
#define SWP_MAC_DEACTIVATE_ERROR        0xFF72
#define SWP_MAC_SETSTATUS_ERROR         0xFF80
#define SWP_MAC_RESENDFRAME_ERROR       0xFF90
#define SWP_MAC_MAX_PAYLOAD_ERROR       0xff91
#define SWP_MAC_NO_TX_FRAME_FREE        0xffa0
#define SWP_MAC_TOO_MUCH_ON_GOING_ERROR 0xffa1
#define SWP_MAC_RECEIVE_ERROR           0xffb0
#define SWP_MAC_SYNCHRONIZED_ERROR      0xffb1
#define SWP_MAC_RX_STAT_ERROR           0xffb2


/// @}


/// @{ for HCI API

/// @brief for CardEmulationListener
#define HCI_API_EVENT_GET_PARAMETER_RESPONSE   0x01
#define HCI_API_EVENT_ON_SEND_DATA             0x50
#define HCI_API_EVENT_FIELD_OFF                0x54
#define HCI_API_EVENT_CARD_DEACTIVATED         0x52

/// @brief for ConnectivityListener
#define HCI_API_EVENT_STAND_BY                 0x50

/// @brief for ReaderListener
#define HCI_API_EVENT_GET_PARAMETER_RESPONSE   0x01
#define HCI_API_EVENT_WRITE_EXCHANGE_DATA_RESPONSE  0x02
#define HCI_API_EVENT_TARGET_DISCOVERED        0x50

/// @brief for listener event trigger
#define HCI_API_LISTENER_TRIGGER               0x80
#define HCI_API_LISTENER_PROCESSED             0x20

/// @}



/*  typedefs
****************************************************************/

/**
 ** @brief The active SWP CLT RF technology
 **/
typedef enum IFX_SWP_CLT_RF_TECHNOLOGY {
    SWP_CLT_RFTECH_NONE,                    /**< The CLT link layer protocol is not active. */
    SWP_CLT_RFTECH_A,                       /**< The CLT link layer protocol transports Type-A frames. */
    SWP_CLT_RFTECH_F,                       /**< The CLT link layer protocol transports Type-F frames. */
    SWP_CLT_RF_TECHNOLOGY_MAX
} IFX_SWP_CLT_RF_TECHNOLOGY;

/**
 ** @brief The active SWP CLT application
 **/
typedef enum IFX_SWP_CLT_RF_APPLICATION {
    SWP_CLT_RFAPP_NONE,                     /**< No known CLT application is currently active. */
    SWP_CLT_RFAPP_HALT,                     /**< The HALT CLT application is active. */
    SWP_CLT_RFAPP_MIFARE,                   /**< The Mifare CLT application is active. */
    SWP_CLT_RF_APPLICATION_MAX
} IFX_SWP_CLT_RF_APPLICATION;


/**
 ** @brief SWP driver state structure
 **/
//typedef struct IFX_SWP_STATE {
////     struct {
////         u8               State;          /**< Contains the current state of the SWP interface. @sa IFX_SWP_INTERFACE_STATE */
////         u8               StateChange;    /**< Set if an SWP interface state change has occured. */
////     } Interface;
//    struct {
//        u8               Active;         /**< Contains the currently active SWP link layer protocol. @sa IFX_SWP_LINK_LAYER */
//    } LinkLayer;
//     struct {
////         IFX_SWP_SHORT_FRAME Sync1;          /**< The ACT_SYNC frame to be sent after the first SWP interface activation. */
////         IFX_SWP_SHORT_FRAME Sync2;          /**< The ACT_SYNC frame to be sent after subsequent SWP interface activations. */
////         IFX_SWP_SHORT_FRAME LastFrame;      /**< The last ACT link layer frame sent by the card. */
//         u8               PowerMode;      /**< The power mode announced by the last received ACT_POWER_MODE frame. */
//     } Act;
//    struct {
//        u8               Technology;     /**< The RF technology transported by the CLT link layer. @sa IFX_SWP_CLT_RF_TECHNOLOGY */
//        u8               Application;    /**< The active CLT application. @sa IFX_SWP_CLT_RF_APPLICATION */
//    } Clt;
////     struct {
////         u8               MinWindowSize;  /**< The minimum supported SHDLC window size */
////         u8               MaxWindowSize;  /**< The maximum supported SHDLC window size */
////         u8               MaxEndpointCaps;/**< The supported SHDLC endpoint capabilities */
////         u8               RsetState;      /**< The current that of the SHDLC RSET handling */
////         u8               WindowSize;     /**< The negotiated SHDLC window size */
////         u8               EndpointCaps;   /**< The negotiated SHDLC endpoint capabilities */
////     } Shdlc;
//    struct {
//        MIF_OS_CONTEXT     *pHandle;        /**< The MifareOS handle to be used by the Mifare CLT application. */
//    } Mifare;
//} IFX_SWP_STATE;


/// @brief Type definition for Lpdu structure
typedef struct
{
    volatile u8 Size;
    u8 Data[30];   ///< Any LPDu is maximum 30 bytes long
} SWP_LLC_Lpdu_t;

/// @brief Type definition for Sync ID
typedef union
{
    u16 u2SId;
    u8 u1SId[2];
} SWP_LLC_SyncId_t;

/// @brief Enumeration for SWP timers
typedef enum
{
    T1_ACKNOWLEDGE_TIMEOUT,
    T2_GUARDING_TIMEOUT,
    T3_CONNECTION_TIMEOUT,
    TS2_INHIBIT_TIMEOUT,
    WAIT_FOR_10MS,
    NO_TIMEOUT,
} ShdlcTimeT;

/// @brief Type definition for SWP timer
typedef struct
{
    ShdlcTimeT   Status;
    u16          Timercdv;
}SWP_Timer_t;

/// @brief SWP_LLC_t structure definition. The structure contains all layer state machine variables, callBack pointer to function address.
typedef struct
{
    volatile u8         Status;
    volatile u8         Ns;
    volatile u8         Nr;
    volatile u8         DNr;

    u8                  SWP_Bit_Duration;
    SWP_LLC_SyncId_t    SyncId; // copy of a value stored in Flash
    u8                  WS;
	u8                  privNr;
	u8                  PowerMode;
	
    SWP_LLC_Lpdu_t		ReadLpdu;
    SWP_LLC_Lpdu_t      WriteLpdu;
	SWP_LLC_Lpdu_t      ResendLpdu[DEFAULT_WS];
    SWP_Timer_t         Timer;
    u8                  NbRetransmit;
    u8                  MaxRetransmit;
    u8                  ResentOngoing;
    //SWP_LLC_CLT_CallBack_t CltCallback;
}SWP_LLC_t;

/// @brief Typedef for link management gate registry
typedef struct
{
    u16  RecError;
} HCP_LinkMgtGate_Registry_t;

/// @brief Typedef for identity management gate registry
typedef struct  //22B
{
    u8  VersionSw[3];
    u8  HciVersion;
    u8  VersionHw[3];
    u8  VendorName[8];
    u8  ModelId;
    u8  GatesList[HCP_GATES_LIST_SIZE];
} HCP_IdentityMgtGate_Registry_t;

/// @brief Typedef for Type A gate registry
typedef struct  //32B
{
	u8 mode;
	u8 uid[11];
	u8 sak;
	u8 atqa[2];
	u8 appData[16];
	u8 fwi_sfgi;
	u8 cid;
	u8 clt;
	u8 dataRate[3];
}HCP_TypeA_Registry_t;

/// @brief Typedef for Type B gate registry
typedef struct  //25B
{
	u8 mode;
	u8 pupi_reg[5];
	u8 afi;
	u8 atqb[4];
	u8 higher_layer_response[16];
	u8 datarate[3];
}HCP_TypeB_Registry_t;

/// @brief Data structure for array of pipes ( to be located in NVM )
typedef struct  //6B
{
    // data in the same format as returned by host controller
    // in response to a ADM_CREATE_PIPE
    u8                      SourceHId;
    u8                      SourceGId;
    u8                      DestinationHId;
    u8                      DestinationGId;
    u8                      PipeId;
    u8                      State;
    //SWP_HCP_GateCallBack_t  Fct;
} HCP_Pipe_t;

/// @brief Data structure that need to be persistent ( stored in NVM )
typedef struct
{
//    u8                           SessionId[8];       ///< to record whether pipes have already been created or not ( != 0 meaning not connected )
//    HCP_Pipe_t                      Pipes[HCP_MAX_PIPES];
//    HCP_IdentityMgtGate_Registry_t  IdentityMgtGate_Registry;
//    HCP_LinkMgtGate_Registry_t      LinkMgtGate_Registry;
	HCP_TypeA_Registry_t            TypeA_Registry;
	HCP_TypeB_Registry_t			TypeB_Registry;
} SWP_HCP_NVM_Context_t;

/// @brief Type definition of HCP message structure
typedef  struct
{
    u8 Buf[261];
    u16 Size;
} SWP_HCP_Buf_t;

typedef struct
{
	u8 mainStatus;       //会话主流程状态
	u8 subStatus;        //会话子流程状态
	u8 responseStatus;   //命令响应状态（命令发送后等待响应 或者 响应接收后等待处理）
}SWP_SESSION_STATUS;

/// @brief SWP_HcpT structure definition. The structure contain all layer state machine variables, callBack pointer to function address.
typedef  struct
{
    volatile u8        	Status;
    SWP_SESSION_STATUS  sessionStatus;
//    SWP_HCP_Buf_t          ReadMessage;
//    SWP_HCP_Buf_t          WriteMessage;
//    u32                 NbBytesSent;
//    u8                  ChainingOnGoing;
//    u8                  CurrentPipeId;      ///< pipe ID on which the current message is sent
    volatile u8         CurrentIndexInHCP_Pipes;
//	volatile u8         SessionInitStatus;
//    volatile u8         TransmissionState;      ///< record state or transmission ( initialisation sequence )
    SWP_HCP_NVM_Context_t  *NvmContext;
}SWP_HCP_Context_t;



///
/// @brief: SWP Events define
///
typedef enum {    
	/* Card Emulator   */
	EVENT_ON_SEND_DATA,
	EVENT_FIELD_OFF,

	/* Connectivity   */
	EVENT_STAND_BY,

	/* Card Reader   */
	EVENT_WRITE_EXCHANGE_DATA_RESPONSE,
	EVENT_TARGET_DISCOVERED,

	/* Card Emulator & Card Reader  */
	EVENT_GET_PARAMETER_RESPONSE,

	/* Events MAX */
	EVENTS_NUM,
} SWP_EVENTS;

typedef enum {
	NO_EVENTS,
	RECEIVE_APDU_CMD,
	SWP_DETECT_FIELD_OFF,
	SWP_EVENT_GET_PARAMETER_RESPONSE,
	SWP_EVENT_STAND_BY,
	SWP_EVENT_TARGET_DISCOVERED,
	SWP_EVENT_EXCHANGE_DATA_RESPONSE,
}SWP_HCP_EVENTS;	


/// @brief : HCP Status define
typedef enum {
	SESSION_INIT_PROCESS,
	SESSION_INIT_FINISH,
	EVT_LOOKBACK,
	EVT_SEND_DATA,
	PROCESS_COMMAND,
	EVT_CARD_FIELD_OFF,
}HCP_STATUS;

typedef enum {
	INIT_START,
	INIT_GET_SESSIONID,
	INIT_CLEAR_ALL_PIPES,
	INIT_OPEN_ADM_PIPE,
	INIT_OPEN_LINK_PIPE,
	INIT_SET_WHITELIST,
	INIT_CREATE_PIPES,
	INIT_TYPEA_PROCESS,
	INIT_TYPEB_PROCESS,
	INIT_TYPEA_READER_PROCESS,
	INIT_TYPEB_READER_PROCESS,
	INIT_CONNECTIVITY_PROCESS,
	INIT_GET_CLF_INFO_PROCESS,
	INIT_SET_SESSIONID,
	INIT_END_OPERATION,
	INIT_FINISH,
}HCP_SESSION_INIT_MAIN_STATUS;

typedef enum {
	NO_OPERATION,
	INIT_WAIT_FOR_RESPONSE,
	INIT_RESPONSE_PROCESS,
	INIT_GOTO_NEXT_STATUS,
}HCP_SESSION_INIT_RESPONSET_STATUS;


typedef struct {
	u8 Status;
	u8 cmdType;
}INIT_SUB_LIST;


#define STOP_TIMER2()        
#define STOP_TIMER3()        

/*  constants
****************************************************************/

/*  variables
****************************************************************/

#if BOOT__IO__SWP
#if !HAL__IO__SWP__LIB
extern u8 gFailedActivations;
extern IFX_SWP_STATE IFX_Swp_State;

/// @brief LLC variables
extern SWP_LLC_t g_SWP_LLC_Context;

/// @brief HCP variables
extern u8  g_HCP_ReadMessage_Buf[];
extern u16 g_HCP_ReadMessage_Size;

extern u8  g_HCP_WriteMessage_Buf[];
extern volatile u16 g_HCP_WriteMessage_Size;

extern volatile u8 g_HCP_CurrentPipeId;

extern volatile u8 g_HCP_Status;

extern u8 g_HCI_API_TrigEvent;

extern u8 g_HCP_NbBytesSent;

extern SWP_HCP_Context_t g_HCP_Context;

extern volatile u8 g_HCP_TypeAPid;

extern volatile u8 g_HCP_TypeBPid;

extern volatile u8 g_HCP_ConnectivityPid;
//ReaderMode
extern volatile u8 g_HCP_ReaderTypeAPid;

extern volatile u8 g_HCP_ReaderTypeBPid;

extern volatile u8 g_HCP_Event_Pipe;

extern u8 g_HCP_PackageChaining;

//zhou
//extern SWP_HCP_NVM_Context_t	g_HCP_RAM_Context;

extern volatile u8 WAITFOR_RR;

extern u8 g_HCP_PackageStatus;
extern volatile u8 g_ListenerEvent;
extern volatile u8 g_ListenerEvent_next;
extern volatile u8 g_HCP_Card_Status;

extern u8 g_SwpHciEvents[EVENTS_NUM];
/*  extern functions
****************************************************************/


/// @brief Returns pipe id of incoming HCP packet
//#define SWP_HCP_GetPacketHeading( )             ( ( u8 )( g_SWP_LLC_Context.ReadLpdu->Lpdu[1] & HCP_PACKET_CB_MASK ) )
/// @brief Returns pipe id of incoming HCP packet
//#define SWP_HCP_GetPacketPipeId( hcpCtrlByte )    ( ( u8 )( hcpCtrlByte & HCP_PACKET_PIPEID_MASK ) )
/// @brief Returns pointer to incoming HCP message data
#define SWP_HCP_GetMessageDataAddr( )             ( ( u8 *)( &g_HCP_ReadMessage_Buf[1] ) )
/// @brief Returns pointer to incoming HCP message data
#define SWP_HCP_GetMessageDataSize( )             ( ( u16 )( (g_HCP_ReadMessage_Size == 0) ? 0x00:g_HCP_ReadMessage_Size-1 ) )
/// @brief Returns pointer to incoming HCP message
#define SWP_HCP_GetMessageAddr( )             ( ( u8 *)( g_HCP_ReadMessage_Buf ) )
/// @brief Returns size of incoming HCP message
#define SWP_HCP_GetMessageSize( )             ( ( u16 )( g_HCP_ReadMessage_Size ) )
/// @brief Returns type of incoming HCP message
#define SWP_HCP_GetMessageType( )             ( ( u8 )( g_HCP_ReadMessage_Buf[0] & HCP_MSG_TYPE_MASK ) )
/// @brief Sets the message header byte of a response message
#define SWP_HCP_SetResponseHeader(_error_)    { g_HCP_WriteMessage_Buf[0] = HCP_MSG_RESPONSE_TYPE | ( _error_ ); g_HCP_WriteMessage_Size = 1; }
/// @brief Sets the message header byte of an event
#define SWP_HCP_SetEventHeader(_ins_)         { g_HCP_WriteMessage_Buf[0] = HCP_MSG_EVENT_TYPE | ( _ins_ ); g_HCP_WriteMessage_Size = 1; }
/// @brief Sets the message header byte of an event
#define SWP_HCP_SetCommandHeader(_ins_)       { g_HCP_WriteMessage_Buf[0] = HCP_MSG_COMMAND_TYPE | ( _ins_ ); g_HCP_WriteMessage_Size = 1; }
/// @brief Writes a response in HCP data structure
#define SWP_HCP_SetResponse( _ins_ , _addr_ , _size_ )  {  SWP_HCP_SetResponseHeader( _ins_ );\
                                                           if ( _size_ != 0 ) memcpy ( g_HCP_WriteMessage_Buf + 1 , _addr_ , _size_ );\
                                                           g_HCP_WriteMessage_Size += _size_; }
/// @brief Writes a command in HCP data structure
#define SWP_HCP_SetCommand( _ins_ , _addr_ , _size_ )   {  SWP_HCP_SetCommandHeader( _ins_ );\
                                                           if ( _size_ != 0 ) memcpy ( g_HCP_WriteMessage_Buf + 1 , _addr_ , _size_ );\
                                                           g_HCP_WriteMessage_Size += _size_; }
/// @brief Writes an event in HCP data structure
#define SWP_HCP_SetEvent( _ins_ , _addr_ , _size_ )     {  SWP_HCP_SetEventHeader( _ins_ );\
                                                           if ( _size_ != 0 ) memcpy ( g_HCP_WriteMessage_Buf + 1 , _addr_ , _size_ );\
                                                           g_HCP_WriteMessage_Size += _size_; }

/// @brief Tests whether a message has been completely sent
#define SWP_HCP_IsMessageSent( )              ( g_HCP_WriteMessage_Size == 0 )

/// @brief Initializes the data structure before a transmission of HCP message
#define SWP_HCP_InitMessageTransmission( )    { /*g_HCP_Status = SWP_HCP_INITIALIZED;*/ g_HCP_NbBytesSent = 0; }

/// @brief reset hcp state
#define SWP_HCP_ResetHCPStatus( )			(g_HCP_Context.Status = SESSION_INIT_FINISH)

/// @brief SWP HCP receive a event
#define SWP_HCP_RecEvent( )                 (g_HCP_Status == SWP_HCP_CARD_SENT_DATA)

/// @brief HCP message receive finish
#define SWP_HCP_MsgComplete( )              ((g_HCP_PackageStatus & HCP_PKG_COMPLETE) == HCP_PKG_COMPLETE)

#define SWP_LLC_WaitForSend( )              ( (g_SWP_LLC_Context.Ns >= g_SWP_LLC_Context.DNr) ? \
								              (g_SWP_LLC_Context.WS > (g_SWP_LLC_Context.Ns - g_SWP_LLC_Context.DNr)) : \
										      (g_SWP_LLC_Context.WS > (g_SWP_LLC_Context.Ns + (8 - g_SWP_LLC_Context.DNr))) )


void IFX_Swp_sfr2mem(void *pMem, const volatile uint32_t * pSfr, UINT32 length);
void IFX_Swp_mem2sfr(volatile uint32_t * pSfr, const void *pMem, UINT32 length);

/// @brief mac layer
void Swp_Mac_SendFrame( void );

/// @brief act layer
///
BOOL Swp_Llc_Act_Receive(u8 *swpFrame, u8 Length);

/// @brief shdlc layer
///
void Swp_Llc_Shdlc_Init( void );
STATUS Swp_Llc_Shdlc_Send( void );
BOOL Swp_Llc_Shdlc_Receive(u8 *swpFrame, u8 Length);
u8 SWP_LLC_ShdlcLinkEstablished(void);

/// @brief clt layer
///
void Swp_Llc_Clt_Init( void );
void Swp_CltApp_Mifare_Init( void );
BOOL Swp_Llc_Clt_Receive(u8 *swpFrameData, u8 rxLength, u8 rxBufNum);

void Swp_Clt_Send_GotoInit(u8 txBufNum);
void Swp_Clt_Send_GotoHalt(u8 txBufNum);

BOOL Swp_CltApp_Halt_Checker(u8 rxBufNum, u8 rxLength, const IFX_SWP_FRAME *swpFrame);
BOOL Swp_CltApp_Halt_Handler(u8 rxBufNum, u8 rxLength, u8 txBufNum);

BOOL Swp_CltApp_Mifare_Checker(u8 rxBufNum, u8 rxLength, const IFX_SWP_FRAME *swpFrame);
BOOL Swp_CltApp_Mifare_Handler(u8 rxBufNum, u8 rxLength, u8 txBufNum);

/// @brief hcp layer
///
void Swp_Hcp_Initialize( void );
u8 Swp_Hcp_SessionInitFinish( void );
void Swp_Hcp_SendMessage ( void );
u32 Swp_Hcp_ProcessPacket( u8 *pShdlcData, u8 DataLen);
u8 Swp_Hcp_InitStart(void);
u8 SWP_HCP_AnyOpenPipe ( u8 PipeId );
void SWP_HCP_AdmClearAllPipe ( u16 IdentityRefData );
void SWP_HCP_AdmCreatePipe ( u8 SrcGateId , u8 DestHostId, u8 DestGateId );
u8 GetPipeIndex(u8 dstGateId );
u32 ProcessAnySetParam(void);
u32 ProcessAnyClosePipe( void );
u32 ProcessAdmNotify(u8 MessageSize );
u32 ProcessAdmNotifyPipeCreated( void );
u32 ProcessAdmNotifyAllPipeCleared( void );
u32 SWP_HCP_SendPacket ( u8 PipeId );
void SWP_HCP_AnyGetParameter ( u8 PipeId , u8 Index);
u32 ProcessAnyGetParam(void);
u32 ProcessAnyOpenPipe( void );
u32 ProcessAdmNotifyPipeDeleted( void );
void SWP_HCP_AnySetParameter ( u8 PipeId , u8 Index , u8 *Registry, u8 Size );

u8 GetCurrentPipeIndex(u8 PipeId );

extern void SWP_HCP_EvtEndOfOperation(void);

//for HCI API
#if	VM_HCI_API
void SWP_HCP_AnyGetParameterHCI (u8 PipeId ,u8 Index);
extern void getParaCommandHCI(void);
extern void sendSendDataEventHCI(void);
extern void getCardRFTypeHCI(void);
extern void checkHeaderHCI(void);
extern void checkCompleteHCI(void);
extern void getInstructionTypeHCI(void);
extern void getInstructionHCI(void);
extern void getReceiveBufferHCI(void);
extern void getDataLength(void);
extern void SendWriteXchgDataCommand(void);
extern void restartReaderModeProcedure(void);

extern void getListenerEvent(void);
extern void connectivity_prepareAndSendCnnEvent(void);

void isHeaderHCI(void);
void isCompleteHCI(void);
void getRFTypeHCI(void);
void HCI_getPowerMode(void);
extern void isHciServiceAvaliable(void);

extern u8 SWP_LLC_ShdlcLinkFinish(void);

#endif //HCI_API

/// @brief swp timer
///
extern void Swp_TimerStart( ShdlcTimeT SwpTimer );
extern void Swp_TimerStop( void );

extern void SWP_IRQHandlerProcess(void);

extern u16 Swp_Receive(u8 *recBuf);
extern void Swp_sendStatusAndReceiveCommand(u16 status);

extern void Swp_TS2TimerStart( void );
extern void Swp_Llc_Act_Init( void );
extern void Swp_Get_SyncId(void);
extern void Swp_Update_SyncId(u8* syncID);


extern void SWP_HCP_SessionReEstablish( void );

extern void SWP_HCP_SessionInit(void);
extern u8 Swp_DetectEvent( void );
extern u8 Swp_SetEventProcess( u8 eventType );
extern u32 SWP_HCP_ProcessCommand(void);
extern void Swp_SendStatus(void);
extern u8 Swp_TrigEvent( void );
extern u16 isSwpTimerRunning(void);
extern void Swp_Timer_Handler( void );
#endif
#endif
/** @} */ /* end of group IFX_SWP_GROUP */
#endif /* SWP_PRIVATE_H */
