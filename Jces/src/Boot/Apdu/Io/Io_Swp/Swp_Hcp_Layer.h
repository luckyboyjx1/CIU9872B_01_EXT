#ifndef __SWP_HCP_LAYER_h__
#define __SWP_HCP_LAYER_h__

#include "ecpDefine.h"
#include "ecpReadWrite.h"


///
/// @brief : All define
///

/// @brief Pipe state
#define HCP_PIPE_NOT_CREATED                0xFF
/// @brief Pipe state
#define HCP_PIPE_CLOSED                     0xF8
/// @brief Pipe state
#define HCP_PIPE_OPENED                     0x00
/// @brief Mask to get HCP packet chaining bit
#define HCP_PACKET_CB_MASK                  0x80
/// @brief Mask to get HCP packet pipe ID
#define HCP_PACKET_PIPEID_MASK              0x7F
/// @brief Mask to get message instruction type
#define HCP_MSG_TYPE_MASK                   0xc0
/// @brief Mask to get message instruction
#define HCP_MSG_INSTRUCTION_MASK            0x3F
/// @brief HCP message command type
#define HCP_MSG_COMMAND_TYPE                0x00
/// @brief HCP message event type
#define HCP_MSG_EVENT_TYPE                  0x40
/// @brief HCP message response type
#define HCP_MSG_RESPONSE_TYPE               0x80


/// @brief Number of gates in GATES_LIST registry entry array
#define HCP_GATES_LIST_SIZE                 6
/// @brief Size of pipes array stored in NVM ( maximum number of pipes )
#define HCP_MAX_PIPES                       10
/// @brief Buffer to store HCP messages ( size is not specified by ETSI
/// but NXP test suite sends maximum 261 bytes messages - APDU of 260 bytes + HCP header byte + EVT_SEND_DATA RF error indicator byte )
#define HCP_BUF_SIZE                        262


/// @brief HCP packet status
#define HCP_PKG_NO_HEADER                   0x00
#define HCP_PKG_IS_HEADER                   0x01
#define HCP_PKG_COMPLETE                    0x80


///
/// @brief : HCP host id define
///
/// @brief HCP UICC host ID
#define HCP_UICC_HOST                       0x02
/// @brief HCP Terminal host ID
#define HCP_TERMINAL_HOST                   0x01
/// @brief HCP host controller ID
#define HCP_CONTROL_HOST                    0x00


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


///
/// @brief : HCP Static Pipe Id Macro define
///

/// @brief Pipe ID for LINK management gate
#define HCP_PIPE_ID_LINK_MGT           0x00
/// @brief Pipe ID for administration gate
#define HCP_PIPE_ID_ADMIN              0x01



///
/// @brief : HCP Gate Registry Macro Define
///


/// @brief Link management gate: REC ERROR registry entry
#define HCP_LINK_MGT_GATE_REG_REC_ERROR     0x01

/// @brief Administration gate: SESSION IDENTITY registry entry
#define HCP_ADMIN_GATE_REG_SESSION_IDENTITY 0x01
/// @brief Administration gate: WHITELIST registry entry
#define HCP_ADMIN_GATE_REG_WHITELIST        0x03

/// @brief Identity management gate entry registry index VERSION SOFTWARE
#define HCP_IDT_MGT_GATE_REG_INDEX_VERSW    0x01
/// @brief Identity management gate entry registry index VERSION HARDWARE
#define HCP_IDT_MGT_GATE_REG_INDEX_VERHW    0x03
/// @brief Identity management gate entry registry index VERSION HCI
#define HCP_IDT_MGT_GATE_REG_INDEX_VERHCI   0x02
/// @brief Identity management gate entry registry index MODEL ID
#define HCP_IDT_MGT_GATE_REG_INDEX_MODELID  0x05
/// @brief Identity management gate entry registry index VENDOR NAMES
#define HCP_IDT_MGT_GATE_REG_INDEX_VNDNAME  0x04
/// @brief Identity management gate entry registry index GATES LIST
#define HCP_IDT_MGT_GATE_REG_INDEX_GTLIST   0x06

/// @brief RF type A gate entry registry index MODE
#define HCP_RFTYPEA_GATE_REG_INDEX_MODE     0x01
/// @brief RF type A gate entry registry index UID
#define HCP_RFTYPEA_GATE_REG_INDEX_UID      0x02
/// @brief RF type A gate entry registry index SAK
#define HCP_RFTYPEA_GATE_REG_INDEX_SAK      0x03
/// @brief RF type A gate entry registry index ATQA
#define HCP_RFTYPEA_GATE_REG_INDEX_ATQA     0x04
/// @brief RF type A gate entry registry index Application Data
#define HCP_RFTYPEA_GATE_REG_INDEX_APP_DATA 0x05
/// @brief RF type A gate entry registry index FWI SFGI
#define HCP_RFTYPEA_GATE_REG_INDEX_FWI_SFGI 0x06
/// @brief RF type A gate entry registry index CID_SUPPORT
#define HCP_RFTYPEA_GATE_REG_INDEX_CID      0x07
/// @brief RF type A gate entry registry index CLT_SUPPORT
#define HCP_RFTYPEA_GATE_REG_INDEX_CLT      0x08
/// @brief RF type A gate entry registry index DATARATE_MAX
#define HCP_RFTYPEA_GATE_REG_INDEX_DTR_MAX  0x09


/// @brief RF type B gate entry registry index MODE
#define HCP_RFTYPEB_GATE_REG_INDEX_MODE     0x01
/// @brief RF type B gate entry registry index PUPI
#define HCP_RFTYPEB_GATE_REG_INDEX_PUPI     0x02
/// @brief RF type B gate entry registry index AFI
#define HCP_RFTYPEB_GATE_REG_INDEX_AFI		0x03
/// @brief RF type B gate entry registry index ATQB
#define HCP_RFTYPEB_GATE_REG_INDEX_ATQB		0x04
/// @brief RF type B gate entry registry index HIGHER_LAYER_RESPONSE
#define HCP_RFTYPEB_GATE_REG_INDEX_HIGHER_LAYER_RESPONSE     0x05
/// @brief RF type B gate entry registry index DATARATE_MAX
#define HCP_RFTYPEB_GATE_REG_INDEX_DATARATE_MAX 0x06



///
/// @brief : HCI Generic Gates Events Type and Commands Type define
///

/// @brief HCP Event from Host to Host controller to explain end of operation
#define HCP_EVT_HCI_END_OF_OPERATION        0x01
/// @brief HCP Event to send and receive data witha parameter that contains a block of data
#define HCP_EVT_POST_DATA                   0x02
/// @brief HCP Event sent by Host controller to notify connection or disconnection
#define HCP_EVT_HOT_PLUG                    0x03

/// @brief HCP Card Application Gate specific event : EVT_SEND_DATA
#define HCP_CE_EVT_SEND_DATA         		0x10
/// @brief HCP Card Application Gate specific event : EVT_FIELD_ON
#define HCP_CE_EVT_FIELD_ON          		0x11
/// @brief HCP Card Application Gate specific event : EVT_CARD_DEACTIVATED
#define HCP_CE_EVT_CARD_DEACTIVATED  		0x12
/// @brief HCP Card Application Gate specific event : EVT_CARD_ACTIVATED
#define HCP_CE_EVT_CARD_ACTIVATED    		0x13
/// @brief HCP Card Application Gate specific event : EVT_FIELD_OFF
#define HCP_CE_EVT_FIELD_OFF         		0x14

/// @brief HCP Reader RF Gate and Reader RF Application Gate specific command : WR_XCHG_DATA
#define HCP_RM_CMD_XCHG_DATA           		0x10
/// @brief HCP Reader RF Gate specific event : EVT_READER_REQUESTED
#define HCP_RM_EVT_REQUESTED        	    0x10
/// @brief HCP Reader RF Gate specific event : EVT_END_OPERATION
#define HCP_RM_EVT_END_OPERATION      		0x11
/// @brief HCP Reader Application Gate specific event : EVT_TARGET_DISCOVERED
#define HCP_RM_EVT_TARGET_DISCOVERED        0x10
/// @brief HCP Reader Application Gate specific event : EVT_READER_STATUS
#define HCP_RM_EVT_READER_STATUS	        0x11

/// @brief HCP Connectivity Gate specific command : PRO_HOST_REQUEST
#define HCP_CNN_CMD_HOST_REQUEST   			0x10
/// @brief HCP Connectivity Gate specific event : EVT_CONNECTIVITY
#define HCP_CNN_EVT_CONNECTIVITY   			0x10
/// @brief HCP Connectivity Gate specific event : EVT_TRANSACTION
#define HCP_CNN_EVT_TRANSACTION    			0x12
/// @brief HCP Connectivity Gate specific event : EVT_OPERATION_ENDED
#define HCP_CNN_EVT_OPERATION_END  			0x13
/// @brief HCP Connectivity Application Gate specific event : EVT_STANDBY
#define HCP_CNN_EVT_STANDBY        			0x10



///
/// @brief: HCI All Gates Commands and Administrator Gates Commands define
///

/// @brief HCP command for all gates
#define HCP_CMD_ANY_SET_PARAMETER           	0x01
/// @brief HCP command for all gates
#define HCP_CMD_ANY_GET_PARAMETER           	0x02
/// @brief HCP command for all gates
#define HCP_CMD_ANY_OPEN_PIPE               	0x03
/// @brief HCP command for all gates
#define HCP_CMD_ANY_CLOSE_PIPE              	0x04
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_CREATE_PIPE             	0x10
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_DELETE_PIPE             	0x11
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_NOTIFY_PIPE_CREATED     	0x12
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_NOTIFY_PIPE_DELETED     	0x13
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_CLEAR_ALL_PIPE          	0x14
/// @brief HCP command for administration gate only
#define HCP_CMD_ADM_NOTIFY_ALL_PIPE_CLEARED 	0x15



///
/// @brief : HCI Command define
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
#define HCP_REP_ANY_E_REG_ACCESS_DENIED     0x0A
/// @brief HCP response : Permission denied to create a pipe due to a WHITELIST violation
#define HCP_REP_ANY_E_PIPE_ACCESS_DENIED    0x0B




///
/// @brief : HCI Command response data offset define
///

// //create pipe
#define CMD_CREATE_PIPE_SRC_GID     (0x00)
#define CMD_CREATE_PIPE_DST_HID     (0x01)
#define CMD_CREATE_PIPE_DST_GID     (0x02)
#define CMD_CREATE_PIPE_PACKET_SIZE (0x03)


#define CMD_NOTIFY_CREATE_PIPE_SRC_HID      (0x00)
#define CMD_NOTIFY_CREATE_PIPE_SRC_GID      (0x01)
#define CMD_NOTIFY_CREATE_PIPE_DST_HID      (0x02)
#define CMD_NOTIFY_CREATE_PIPE_DST_GID      (0x03)
#define CMD_NOTIFY_CREATE_PIPE_PID          (0x04)





/// @brief: SWP Events define
typedef enum {
	/* Card Emulator   */
	EVENT_ON_SEND_DATA,
	
	/* Card Emulator & Card Reader  */
	EVENT_GET_PARAMETER_RESPONSE,
	
	EVENT_FIELD_OFF,


	/* Connectivity   */
	EVENT_STAND_BY,

	/* Card Reader   */
	EVENT_WRITE_EXCHANGE_DATA_RESPONSE,
	EVENT_TARGET_DISCOVERED,

	

	/* Events MAX */
	EVENTS_NUM,
} SWP_EVENTS;




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


/// @brief : HCP Pipe Structs define {


typedef struct{
    u8 version_sw[3];     //
		u8 version_hard[3];
		u8 vendor_name[21];   // 0:len; 1~20:value
    u8 model_id;
	  u8 hci_version;
    u8 gates_list;
}REG_IDEN_MANAGE_GATE;

typedef struct{
    u8 Pid;
		u8 srcHid;
		u8 srcGid;
		u8 dstHid;
		u8 dstGid;
		u8 opened;
}PIPE_INFO;

/// @brief Type definition for Sync ID
typedef struct
{
   // u16 u2SId;
    u8  u1SId[2];
} HCP_SyncId_t;



/// @brief Typedef for link management gate registry
typedef struct
{
    u8  RecError[2];
} HCP_LinkMgtGate_Registry_t;

/// @brief Typedef for identity management gate registry
typedef struct
{
    u8  VersionSw[3];
    u8  HciVersion;
    u8  VersionHw[3];
    u8  VendorName[8];
    u8  ModelId;
    u8  GatesList[HCP_GATES_LIST_SIZE];
} HCP_IdentityMgtGate_Registry_t,*pHCP_IdentityMgtGate_Registry_t;

/// @brief Typedef for Type A gate registry
typedef struct
{
	u8 mode;
	u8 uid[11];
	u8 sak;
	u8 atqa[2];
	u8 appData[16];
	u8 fwi_sfgi;
	u8 cid;
	u8 dataRate[3];
}HCP_TypeA_Registry_t,*pHCP_TypeA_Registry_t;

/// @brief Typedef for Type B gate registry
typedef struct  //25B
{
	u8 mode;
	u8 pupi_reg[5];
	u8 afi;
	u8 atqb[4];
	u8 higher_layer_response[16];
	u8 datarate[3];
}HCP_TypeB_Registry_t,*pHCP_TypeB_Registry_t;


/// @brief Data structure for array of pipes ( to be located in NVM )
typedef struct
{
    // data in the same format as returned by host controller
    // in response to a ADM_CREATE_PIPE
    u8                      SourceHId;
    u8                      SourceGId;
    u8                      DestinationHId;
    u8                      DestinationGId;
    u8                      PipeId;
    u8                      State;
} HCP_Pipe_t,*pHCP_Pipe_t;


/// @brief Data structure that need to be persistent ( stored in NVM )
typedef struct
{
//    u8                              SessionId[8];       ///< to record whether pipes have already been created or not ( != 0 meaning not connected )
//    HCP_Pipe_t                      Pipes[HCP_MAX_PIPES];
//    HCP_IdentityMgtGate_Registry_t  IdentityMgtGate_Registry;
//    HCP_LinkMgtGate_Registry_t      LinkMgtGate_Registry;
	HCP_TypeA_Registry_t            TypeA_Registry;
	HCP_TypeB_Registry_t			TypeB_Registry;
//	HCP_SyncId_t    	            SyncId;
} SWP_HCP_NVM_Context_t,*pSWP_HCP_NVM_Context_t;

/// @brief Type definition of HCP message structure
typedef  struct
{
    u8 Buf[HCP_BUF_SIZE];
    volatile u32 Size;
} SWP_HCP_Buf_t;


/// @brief session init Status
typedef struct
{
		u8 mainStatus;       //会话主流程状态
		u8 subStatus;        //会话子流程状态
		u8 responseStatus;   //命令响应状态（命令发送后等待响应 或者 响应接收后等待处理）
}SWP_SESSION_STATUS;


/// @brief SWP_HcpT structure definition. The structure contain all layer state machine variables, callBack pointer to function address.
typedef  struct
{
    volatile u8         Status;
	SWP_SESSION_STATUS  sessionStatus;
    SWP_HCP_Buf_t       ReadMessage;
    SWP_HCP_Buf_t       WriteMessage;
    u32                 NbBytesSent;
    u8                  ChainingOnGoing;
    volatile u8			CurrentPipeId;      ///< pipe ID on which the current message is sent
    volatile u8         CurrentIndexInHCP_Pipes;
    SWP_HCP_NVM_Context_t   *NvmContext;
}SWP_HCP_Context_t;

/// } end of HCP Pipe Struct

extern  SWP_HCP_Context_t  g_HCP_Context;

//hci packet instrution
//  |packet header|message header|
//  |______1B_____|______1B______|
//  |__1b__|__7b__|__2b___|__6b__|
//  |  CB  |  Pid |  Type |  Ins |

#define SWP_PROTOCOL_7_02  702
#define SWP_PROTOCOL_7_06  706
#define SWP_VERSION  SWP_PROTOCOL_7_06


/// @addtogroup SWP HCP Status define
/// @{

/// @brief Macro for size of HCP_Pipe_t structure
#define HCP_SIZEOF_PIPE     6
/// @brief HCP layer state : layer initialized
#define SWP_HCP_INITIALIZED                  0x00
/// @brief HCP layer state : message receive complete
#define SWP_HCP_MESSAGE_COMPLETE             0x01
/// @brief HCP layer state : ready to transmit
#define SWP_HCP_READY_TO_TRANSMIT            0x02
/// @brief HCP layer state : message transmission on going
#define SWP_HCP_MESSAGE_TRANSMISSION_ONGOING 0x03
/// @brief HCP layer state : receive data from card mode
#define SWP_HCP_CARD_SENT_DATA               0x04

#define SWP_HCP_CARD_FILED_OFF               0x05
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

///
/// @{   HCI API Macros define
///



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






/// @brief Returns pipe id of incoming HCP packet
//#define SWP_HCP_GetPacketHeading( )             ( ( u1 )( g_SWP_LLC_Context.ReadLpdu->Data[1] & HCP_PACKET_CB_MASK ) )
/// @brief Returns pipe id of incoming HCP packet
#define SWP_HCP_GetPacketPipeId( )              (( u8 )( g_SWP_LLC_Context.ReadLpdu->Data[1] & HCP_PACKET_PIPEID_MASK ))
/// @brief Returns pointer to incoming HCP message data
#define SWP_HCP_GetReadMessageDataAddr( )             ( ( u8 *)( &g_HCP_Context.ReadMessage.Buf[1] ) )
/// @brief Returns size of incoming HCP message data
#define SWP_HCP_GetReadMessageDataSize( )             ( ( u16 )( (g_HCP_Context.ReadMessage.Size == 0) ? 0x00: (g_HCP_Context.ReadMessage.Size-1) ) )
/// @brief Returns size of incoming HCP event data
#define SWP_HCP_GetEvtSendDataSize( )                 ( ( u16 )( (g_HCP_Context.ReadMessage.Size >= 2) ? (g_HCP_Context.ReadMessage.Size-2) : 0x00) )
/// @brief Returns pointer to incoming HCP message
//#define SWP_HCP_GetMessageAddr( )             ( ( u1 *)( g_HCP_Context.ReadMessage.Buf ) )
/// @brief Returns size of incoming HCP message
//#define SWP_HCP_GetMessageSize( )             ( ( u4 )( g_HCP_Context.ReadMessage.Size ) )
/// @brief Returns type of incoming HCP message
#define SWP_HCP_GetMessageType( )             ( ( u8 )( g_HCP_Context.ReadMessage.Buf[0] & HCP_MSG_TYPE_MASK ) )
/// @brief Returns Instruction of incoming HCP message
#define SWP_HCP_GetMessageInstruction( )      ( ( u8 )( g_HCP_Context.ReadMessage.Buf[0] & HCP_MSG_INSTRUCTION_MASK ) )
/// @brief Returns Error Indicator of incoming HCP message
#define SWP_HCP_GetMessageErrorIndicator( )      ( ( u8 )( g_HCP_Context.ReadMessage.Buf[SWP_HCP_GetReadMessageDataSize()]) )

/// @brief Returns pointer to outgoing HCP message
#define SWP_HCP_GetWriteMessageAddr( )             ( ( u32 )( &g_HCP_Context.WriteMessage.Buf[1] ) )
/// @brief Set size of outgoing HCP message
#define SWP_HCP_SetWriteMessageSize(Length)             { g_HCP_Context.WriteMessage.Size += Length; }
/// @brief Clear size of outgoing HCP message
#define SWP_HCP_ClearWriteMessageSize()               { g_HCP_Context.WriteMessage.Size = 0; }

/// @brief Sets the message header byte of a response message
#define SWP_HCP_SetResponseHeader(_error_)    { g_HCP_Context.WriteMessage.Buf[0] = HCP_MSG_RESPONSE_TYPE | ( _error_ ); g_HCP_Context.WriteMessage.Size = 1; }
/// @brief Sets the message header byte of an event
#define SWP_HCP_SetEventHeader(_ins_)         { g_HCP_Context.WriteMessage.Buf[0] = HCP_MSG_EVENT_TYPE | ( _ins_ ); g_HCP_Context.WriteMessage.Size = 1; }
/// @brief Sets the message header byte of an event
#define SWP_HCP_SetCommandHeader(_ins_)       { g_HCP_Context.WriteMessage.Buf[0] = HCP_MSG_COMMAND_TYPE | ( _ins_ ); g_HCP_Context.WriteMessage.Size = 1; }
/// @brief Writes a response in HCP data structure
#define SWP_HCP_SetResponse( _ins_ , _addr_ , _size_ )  {  SWP_HCP_SetResponseHeader( _ins_ );\
                                                           if ( _size_ != 0 ) ecpMem_Copy ( g_HCP_Context.WriteMessage.Buf + 1 , _addr_ , _size_ );\
                                                           g_HCP_Context.WriteMessage.Size += _size_; }
/// @brief Writes a command in HCP data structure
#define SWP_HCP_SetCommand( _ins_ , _addr_ , _size_ )   {  SWP_HCP_SetCommandHeader( _ins_ );\
                                                           if ( _size_ != 0 ) ecpMem_Copy ( g_HCP_Context.WriteMessage.Buf + 1 , _addr_ , _size_ );\
                                                           g_HCP_Context.WriteMessage.Size += _size_; }
/// @brief Writes an event in HCP data structure
#define SWP_HCP_SetEvent( _ins_ , _addr_ , _size_ )     {  SWP_HCP_SetEventHeader( _ins_ );\
                                                           if ( _size_ != 0 ) ecpMem_Copy ( g_HCP_Context.WriteMessage.Buf + 1 , _addr_ , _size_ );\
                                                           g_HCP_Context.WriteMessage.Size += _size_; }

/// @brief Check this session is card mode
#define SWP_HCP_IsCardApliMode( )                       ( (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId == 0x23) ||\
                                                          (g_HCP_Context.NvmContext->Pipes[g_HCP_Context.CurrentIndexInHCP_Pipes].DestinationGId == 0x21))


/// @brief Return pipe id of current receive HCP packet
#define SWP_HCP_GetCurrentPipeId( )      				( ( u8 )g_HCP_Context.CurrentPipeId )



/// @brief Return HCP message packet receive finish
#define SWP_HCP_MsgIsComplete( )              ((g_HCP_PackageStatus & HCP_PKG_COMPLETE) == HCP_PKG_COMPLETE)

/// @brief Return current HCP message packet is header
#define SWP_HCP_MsgIsHeader( )                ((g_HCP_PackageStatus & HCP_PKG_IS_HEADER) == HCP_PKG_IS_HEADER)


/*  extern Functions and Macro
****************************************************************/


/// @brief Initializes the data structure before a transmission of HCP message
#define SWP_HCP_InitMessageTransmission( )    { /*g_HCP_Context.Status = SESSION_INIT_FINISH;*/ g_HCP_Context.NbBytesSent = 0; }
/// @brief Tests whether a message has been completely sent
#define SWP_HCP_IsMessageSent( )              ( g_HCP_Context.WriteMessage.Size == 0 )
/// @brief Extracts the response intruction
#define SWP_HCP_ExtractResponse( )            (g_HCP_Context.ReadMessage.Buf[0] & HCP_MSG_INSTRUCTION_MASK)
/// @brief Function to process a HCP packet. Uses LLC global variables as input.

// EXTERN FUNCTIONS OR VARIABLE
extern volatile u8 gFstFrame;

extern u8 gRecBuf[];
extern volatile u16 gRecBufPos;
extern volatile u16 gRecLen;

extern u8 gRecType;
extern u8 gRecInstru;
//extern volatile u8 gSwpRece;
extern u8 gRecPid;
extern u8 gRfType;
extern u8 gCardRfType;
extern u8 gPowerMode;
extern u8 gLoopBackPid;
extern u8 gTypeBPid;
extern u8 gTypeAPid;
extern u8 gReaderTypeAPid;
extern u8 gRfMode;

extern u8 gRecHeader;
extern u8 gCardStatus;
extern u8 gEvtType;

extern u8 gReaderTypeAPid;
extern u8 gCardRfPid;
extern u8 gReaderRfPid;
extern u8 gConnPid;
extern u8 gTypeAPid;
extern volatile u8 g_HCP_Card_Status;
extern u8 g_HCI_API_TrigEvent;
extern volatile u8 g_HCP_NeedProcess;
extern u8 g_SwpHciEvents[EVENTS_NUM];


extern u8 gSessionID_Uicc[];
extern PIPE_INFO gPipeStru[];
extern u8 ghciPacket[];
extern u8 g_HCP_PackageStatus;

extern u8 SelectedCltApplication;
extern SWP_HCP_NVM_Context_t	g_HCP_RAM_Context;


extern  u8 SWP_HCP_ProcessPacket (void);
extern  u32 SWP_HCP_ProcessCommand( void );
extern  u8 SWP_HCP_SessionIsFinish( void );
extern  void Swp_Hcp_Init( void);
extern  void Swp_Hcp_SendMessageByPipeID ( u8 PipeId );
extern  void Swp_Hcp_SendMessage (void);
extern  void Swp_SessionInit(void);
extern  void SWP_HCP_ResetHCPStatus(void);
extern  u8 GetPipeIndex(u8 dstGateId);
extern  u8 SWP_HCP_isNeedProcessEvent(void);
/// @brief Send a HCP packet on a given pipe
//u32 SWP_HCP_SendPacket ( u8 PipeId );
/// @brief Send a HCP message on current pipe ID

/// @brief Process a HCP command
//zhou
void SWP_HCP_REJ_ResendFrame( void );




u8 find_pipe_id(u8 newPid, u8 *pos);
u8 setParaReg(u8 dstGid, u8 regIndex, u8 *indata, u8 len);
u8 getParaReg(u8 dstGid, u8 regIndex, u8 *outdata, u8 *len);
void cmdOpenPipe(u8 Pid);
void cmdClosePipe(u8 pid);
void cmdCreatePipe(u8 srcGid, u8 dstHid, u8 dstGid);
void cmdDeletePipe(u8 pid);
void cmdClrAllPipe(void);

void cmdSetPara(u8 pid, u8 index, u8 *data, u8 len);
void cmdGetPara(u8 pid, u8 index);

u8 reqSetPara(void);
u8 reqGetPara(u8 *buf, u8 *len);
u8 reqOpenPipe(u8 pid, u8 *num);
u8 reqClosePipe(u8 pid);
u8 reqCreatePipe(u8 *hciBuf);
u8 reqDeletePipe(u8 pid);
u8 reqClrAllPipe(void);

void session_init_status(void);
void session_init(void);

void get_clf_info_status(void);
void get_clf_info(void);

void typea_rf_status(void);
void typea_rf(void);

void typeb_rf_status(void);
void typeb_rf(void);

void reader_typea_rf_status(void);
void reader_typea_rf(void);

void hci_evt_send_data(u8 *data, u16 Len, u8 pipe);

int SWP_HCI_StateMachine(u8 *hciBuf, u8 Len);


//hci interface
void hciVoidNative( u8 func_index );
void hciBoolNative( u8 func_index );
void hciByteNative( u8 func_index );
void getReceiveBuffer( void );
u16 getReceiveOffset( void );
u16 getReceiveLength( void );
void prepareGetParaCommand( u8 paramID );
void prepareSendDataEvent( void );



#endif
