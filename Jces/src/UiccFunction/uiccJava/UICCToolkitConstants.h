#ifndef    _UICCTOOLKICONSTANTS_H
#define	   _UICCTOOLKICONSTANTS_H
#include  "typedef.h"

//*******************************Events Constants for C COS***********************************
 /*
*         字节１　　　　　　　			 字节2				     　　字节３						       　　字节４					      　　字节５					   　　　 字节６                                   字节7
*　b7 b6 b5 b4 b3 b2 b1 b0       b15 b14 b13 b12 b11 b10 b9 b8    b23 b22 b21 b20 b19 b18 b17 b16    b31 b30 b29 b28 b27 b26 b25 b24      b39 b38 b37 b36 b35 b34 b33 b32 	  b47 b46 b45 b44 b43 b42 b41 b40   b55 b54 b53 b52 b51 b50 b49 b48
*
*/
#define  EVENT_PROFILE_DOWNLOAD_bit                     1 

#define  EVENT_FORMATTED_SMS_PP_ENV_bit                 2    //reserved by 3GPP
#define  EVENT_FORMATTED_SMS_PP_UPD_bit                 3    //reserved by 3GPP              
#define  EVENT_UNFORMATTED_SMS_PP_ENV_bit               4    //reserved by 3GPP
#define  EVENT_UNFORMATTED_SMS_PP_UPD_bit               5    //reserved by 3GPP
#define  EVENT_UNFORMATTED_SMS_CB_bit                   6    //reserved by 3GPP   
      
#define  EVENT_MENU_SELECTION_bit                       7                  
#define  EVENT_MENU_SELECTION_HELP_REQUEST_bit          8   
#define  EVENT_CALL_CONTROL_BY_SIM_bit                  9 

#define  EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit      10     //reserved by 3GPP   
 
#define  EVENT_TIMER_EXPIRATION_bit                     11
#define  EVENT_EVENT_DOWNLOAD_MT_CALL_bit               12
#define  EVENT_EVENT_DOWNLOAD_CALL_CONNECTED_bit        13
#define  EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED_bit     14
#define  EVENT_EVENT_DOWNLOAD_LOCATION_STATUS_bit       15                         
#define  EVENT_EVENT_DOWNLOAD_USER_ACTIVITY_bit         16
#define  EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE_bit 17
#define  EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS_bit    18       
#define  EVENT_STATUS_COMMAND_bit                       19                             
#define  EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION_bit    20                             
#define  EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION_bit   21       
#define  EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit     	22                            
#define  EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit        23       
                     
#define  EVENT_FORMATTED_SMS_CB_bit                     24   //reserved by 3GPP
#define  EVENT_FIRST_COMMAND_AFTER_SELECT_bit          44/** Event : First Command after GSM application selection = 127*///该事件同UICC EVENT_FIRST_COMMAND_AFTER_ATR_bit事件

#define  EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_bit          25
#define  EVENT_EVENT_DOWNLOAD_DISPLAY_PARAMETER_CHANGED_bit         26
#define  EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit                  27
#define  EVENT_EVENT_DOWNLOAD_NETWORK_SEARCH_MODE_CHANGE_bit        28
#define  EVENT_EVENT_DOWNLOAD_BROWSING_STATUS_bit                   29


#define    EVENT_EVENT_DOWNLOAD_IWLAN_ACCESS_STATUS_bit                30	   //30  reserved by 3GPP
#define    EVENT_EVENT_DOWNLOAD_NETWORK_REJECTION_bit                  31	   //31  reserved by 3GPP

#define   EVENT_EVENT_DOWNLOAD_HCI_CONNECTIVITY_bit                 32

#define    EVENT_EVENT_DOWNLOAD_CSG_CELL_SELECTION_bit                 33	   //33  reserved by 3GPP

#define   EVENT_EVENT_DOWNLOAD_FRAMES_INFORMATION_CHANGED_bit       34
#define   EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit        35


#define   EVENT_EVENT_DOWNLOAD_IMS_REGISTRATION_bit               36	  //  reserved by 3GPP
#define   EVENT_EVENT_DOWNLOAD_INCOMING_IMS_DATA_bit              37	  //  reserved by 3GPP
#define   EVENT_FORMATTED_USSD_bit                                38	  //  reserved by 3GPP
#define   EVENT_UNFORMATTED_USSD_bit                              39	  //  reserved by 3GPP

#define   EVENT_PROACTIVE_HANDLER_AVAILABLE_bit                     40
#define   EVENT_EXTERNAL_FILE_UPDATE_bit                            41
#define   EVENT_REMOTE_FILE_UPDATE_bit                              42
#define   EVENT_APPLICATION_DESELECT_bit                            43

#define  EVENT_FIRST_COMMAND_AFTER_ATR_bit                       44/** Event : First Command after ATR = 127*/

#define  EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_MULTIPLE_bit 45
#define  EVENT_MEMORY_FAILURE_bit                                   46
  
#define  EVENT_UNRECOGNIZED_ENVELOPE_bit                0 /** Event : Unrecognized Envelope = -1 */

//******************************** Events Constants for Java API ts102241********************************
#define EVENT_PROFILE_DOWNLOAD                                  1
//2
//3                                                     
//4
//5
//6
#define EVENT_MENU_SELECTION                                    7
#define EVENT_MENU_SELECTION_HELP_REQUEST                       8
#define EVENT_CALL_CONTROL_BY_NAA                               9
//10
#define EVENT_TIMER_EXPIRATION                                  11
#define EVENT_EVENT_DOWNLOAD_MT_CALL                            12
#define EVENT_EVENT_DOWNLOAD_CALL_CONNECTED                     13
#define EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED                  14
#define EVENT_EVENT_DOWNLOAD_LOCATION_STATUS                    15
#define EVENT_EVENT_DOWNLOAD_USER_ACTIVITY                      16
#define EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE              17
#define EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS                 18
#define EVENT_STATUS_COMMAND                                    19
#define EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION                 20
#define EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION                21
#define EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE                     22
#define EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS                     23
//24
#define EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE           25
#define EVENT_EVENT_DOWNLOAD_DISPLAY_PARAMETER_CHANGED          26
#define EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION                   27
#define EVENT_EVENT_DOWNLOAD_NETWORK_SEARCH_MODE_CHANGE         28
#define EVENT_EVENT_DOWNLOAD_BROWSING_STATUS                    29   
// event number 30 and 31 are reserved for 3GPP
#define EVENT_EVENT_DOWNLOAD_HCI_CONNECTIVITY                   32
// event number 33 is reserved for 3GPP
#define EVENT_EVENT_DOWNLOAD_FRAMES_INFORMATION_CHANGED         34
#define EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST          35
#define EVENT_PROACTIVE_HANDLER_AVAILABLE                       123
#define EVENT_EXTERNAL_FILE_UPDATE                              124
#define EVENT_REMOTE_FILE_UPDATE                                125
#define EVENT_APPLICATION_DESELECT                              126
#define EVENT_FIRST_COMMAND_AFTER_ATR                           127
#define EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_MULTIPLE  128
#define EVENT_MEMORY_FAILURE                                    129
#define EVENT_UNRECOGNIZED_ENVELOPE                             -1
//********************************Event Contants defined in TS131130********************************
#define EVENT_FORMATTED_SMS_PP_ENV                              2
#define EVENT_FORMATTED_SMS_PP_UPD                              3   
#define EVENT_UNFORMATTED_SMS_PP_ENV                            4
#define EVENT_UNFORMATTED_SMS_PP_UPD                            5
#define EVENT_UNFORMATTED_SMS_CB                                6    
#define EVENT_MO_SHORT_MESSAGE_CONTROL_BY_NAA                   10   
#define EVENT_FORMATTED_SMS_CB                                  24
#define EVENT_EVENT_DOWNLOAD_IWLAN_ACCESS_STATUS                30
#define EVENT_EVENT_DOWNLOAD_NETWORK_REJECTION                  31
#define EVENT_EVENT_DOWNLOAD_CSG_CELL_SELECTION                 33
#define EVENT_EVENT_DOWNLOAD_IMS_REGISTRATION                   119
#define EVENT_EVENT_DOWNLOAD_INCOMING_IMS_DATA                  120
#define EVENT_FORMATTED_USSD                                    121
#define EVENT_UNFORMATTED_USSD                                  122
//********************************BER-TLV Constants**********************************
#define BTAG_PROACTIVE_UICC_COMMAND                             0xD0
#define BTAG_MENU_SELECTION                                     0xD3
#define BTAG_CALL_CONTROL                                       0xD4
#define BTAG_EVENT_DOWNLOAD                                     0xD6
#define BTAG_TIMER_EXPIRATION                                   0xD7
//*************************** Constants for findTLV method***************************
#define TLV_NOT_FOUND                                           0
#define TLV_FOUND_CR_SET                                        1
#define TLV_FOUND_CR_NOT_SET                                    2

#define TAG_SET_CR                                              0x80
#define TAG_SET_NO_CR                                           0x7F

//************************ Comprehension-TLV Constants******************************
	
#define TAG_COMMAND_DETAILS                       0x01		   
#define TAG_DEVICE_IDENTITIES                     0x02	
#define TAG_RESULT                                0x03	
#define TAG_DURATION                              0x04	
#define TAG_ALPHA_IDENTIFIER                      0x05	
#define TAG_ADDRESS                               0x06	
#define TAG_CAPABILITY_CONFIGURATION_PARAMETERS   0x07	
#define TAG_CALLED_PARTY_SUBADDRESS               0x08	
#define TAG_TEXT_STRING                           0x0D	
#define TAG_TONE                                  0x0E	
#define TAG_ITEM                                  0x0F	
#define TAG_ITEM_IDENTIFIER                       0x10	
#define TAG_RESPONSE_LENGTH                       0x11	
#define TAG_FILE_LIST                             0x12	
#define TAG_LOCATION_INFORMATION                  0x13	
#define TAG_IMEI                                  0x14	
#define TAG_HELP_REQUEST                          0x15	
#define TAG_NETWORK_MEASUREMENT_RESULTS           0x16	
#define TAG_DEFAULT_TEXT                          0x17	
#define TAG_ITEMS_NEXT_ACTION_INDICATOR           0x18	
#define TAG_EVENT_LIST                            0x19	
#define TAG_LOCATION_STATUS                       0x1B	
#define TAG_ICON_IDENTIFIER                       0x1E	
#define TAG_ITEM_ICON_IDENTIFIER_LIST             0x1F	
#define TAG_CARD_READER_STATUS                    0x20
#define TAG_CARD_ATR                              0x21	
#define TAG_C_APDU                                0x22	
#define TAG_R_APDU                                0x23	
#define TAG_TIMER_IDENTIFIER                      0x24	
#define TAG_DATE_TIME_AND_TIME_ZONE               0x26	
#define TAG_CALL_CONTROL_REQUESTED_ACTION         0x27	
#define TAG_AT_COMMAND                            0x28	
#define TAG_AT_RESPONSE                           0x29	
#define TAG_IMMEDIATE_RESPONSE                    0x2B	
#define TAG_DTMF_STRING                           0x2C	
#define TAG_LANGUAGE                              0x2D	
#define TAG_AID                                   0x2F	
#define TAG_BROWSER_IDENTITY                      0x30	
#define TAG_URL                                   0x31	
#define TAG_BEARER                                0x32	
#define TAG_PROVISION_REFERENCE_FILE              0x33	
#define TAG_BROWSER_TERMINATION                   0x34	
#define TAG_BEARER_DESCRIPTION                    0x35	
#define TAG_CHANNEL_DATA                          0x36	
#define TAG_CHANNEL_DATA_LENGTH                   0x37	
#define TAG_CHANNEL_DATA_STATUS                   0x38	
#define TAG_BUFFER_SIZE                           0x39	
#define TAG_CARD_READER_ID                        0x3A	
#define TAG_UICC_TERMINAL_TRANSPORT_LEVEL         0x3C	
#define TAG_OTHER_DATA_DESTINATION_ADDRESS        0x3E	
#define TAG_ACCESS_TECHNOLOGY                     0x3F	
#define TAG_DISPLAY_PARAMETERS                    0x40	
#define TAG_SERVICE_RECORD                        0x41	
#define TAG_DEVICE_FILTER                         0x42	
#define TAG_SERVICE_SEARCH                        0x43	
#define TAG_ATTRIBUTE_INFORMATION                 0x44	
#define TAG_SERVICE_AVAILABILITY                  0x45	
#define TAG_ESN                                   0x46	
#define TAG_NETWORK_ACCESS_NAME                   0x47	
#define TAG_REMOTE_ENTITY_ADDRESS                 0x49	
#define TAG_TEXT_ATTRIBUTE_TAG                    0x50	
#define TAG_ITEM_TEXT_ATTRIBUTE_LIST              0x51       
#define TAG_CONTACTLESS_STATE_REQUEST             0x53	
#define TAG_IMEISV                                0x62	
#define TAG_BATTERY_STATE                         0x63	
#define TAG_BROWSING_STATUS                       0x64       
#define TAG_NETWORK_SEARCH_MODE                   0x65


//*********************ToolkitException Constants ********************************
#define HANDLER_OVERFLOW                          1
#define HANDLER_NOT_AVAILABLE                     2
#define UNAVAILABLE_ELEMENT                       3
#define MENU_ENTRY_NOT_FOUND                      4
#define REGISTRY_ERROR                            5
#define EVENT_NOT_SUPPORTED                       6
#define EVENT_ALREADY_REGISTERED                  7
#define OUT_OF_TLV_BOUNDARIES                     8
#define TERMINAL_PROFILE_NOT_AVAILABLE            9
#define ALLOWED_LENGTH_EXCEEDED                   10
#define NO_TIMER_AVAILABLE                        11
#define INVALID_TIMER_ID                          12
#define EVENT_NOT_ALLOWED                         13
#define BAD_INPUT_PARAMETER                       14
#define COMMAND_NOT_ALLOWED                       15
#define NO_SERVICE_ID_AVAILABLE                   16
#define INVALID_SERVICE_ID                        17
#define TAR_NOT_DEFINED                           18
/*****************************************UICC exception********************************/
#define NO_EF_SELECTED                   1
#define FILE_NOT_FOUND                   4
#define RECORD_NOT_FOUND                 9
#define INTERNAL_ERROR                   5
#define INVALID_MODE                     10 
#define OUT_OF_FILE_BOUNDARIES           7
#define MAX_VALUE_REACHED                12
#define MEMORY_PROBLEM                   13
#define SECURITY_STATUS_NOT_SATISFIED    3
#define COMMAND_INCOMPATIBLE             2
#define OUT_OF_RECORD_BOUNDARIES         8
#define REF_DATA_INVALIDATED             6
#define UICC_COMMAND_NOT_ALLOWED         11
#define UICC_CONDITIONS_OF_USE_NOT_SATISFIED  14
#define REFERENCED_DATA_NOT_FOUND        15
#define OUT_OF_DATA_BOUNDARIES           16
#define UICC_NOT_ENOUGH_MEMORY_SPACE     17
#define INVALID_TAG_VALUE                18
/*****************************************system exception********************************/
#define 	ILLEGAL_VALUE                1
#define 	NO_TRANSIENT_SPACE           2
#define 	ILLEGAL_TRANSIENT            3
#define 	ILLEGAL_AID                  4
#define 	NO_RESOURCE                  5
#define 	UICC_ILLEGAL_USE                  6//ILLEGAL_USE在Apdu.h有重複定義
/*****************************************APDU exception********************************/
#define 	BAD_LENGTH                   3
/*****************************************Admin Exception************************************/
#define INCORRECT_PARAMETERS              1
#define FILE_ALREADY_EXISTS               2
#define CONDITIONS_OF_USE_NOT_SATISFIED   3
#define NOT_ENOUGH_MEMORY_SPACE           4
#define DF_NAME_ALREADY_EXISTS            5


#define SIMPLE_SEARCH_START_FORWARD  0x04
#define SIMPLE_SEARCH_START_BACKWARD 0x05
#define ENHANCED_SEARCH              0x06
#define SEARCH_INDICATION_START_FORWARD_FROM_NEXT_GR       (short)0x0400
#define SEARCH_INDICATION_START_BACKWARD_FROM_PREVIOUS_GR  (short)0x0500
#define	SEARCH_INDICATION_START_FORWARD_FROM_NEXT          (short)0x0600
#define SEARCH_INDICATION_START_BACKWARD_FROM_PREVIOUS     (short)0x0700

#endif
