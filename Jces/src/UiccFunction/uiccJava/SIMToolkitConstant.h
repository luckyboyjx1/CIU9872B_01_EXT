#ifndef _SIMTOOLKITCONSTANT_H_
#define _SIMTOOLKITCONSTANT_H_

/****************************************** toolkit exception Constants***************************************************/
    /** This reason code (= 1) is used to indicate that data are to large than
     *  the storage space available in the handler.
     */
    #define  HANDLER_OVERFLOW_SIM       (short)1

    /** This reason code (= 2) is used to indicate that the Handler is not
     *  available (e.g. busy).
     */
    #define HANDLER_NOT_AVAILABLE_SIM    (short)2

    /** This reason code (= 3) is used to indicate that the element is
     *  unavailable in the handler buffer.
     */
    #define UNAVAILABLE_ELEMENT_SIM      (short)3

    /** This reason code (= 4) is used to indicate that the requested menu entry
     *  is not define for the corresponding applet.
     */
    #define MENU_ENTRY_NOT_FOUND_SIM     (short)4

    /** This reason code (= 5) is used to indicate an error in the applet registry
     */
    #define REGISTRY_ERROR_SIM           (short)5

    /** This reason code (= 6) is used to indicate that the event code is not
     *  supported by the toolkit framework
     */
    #define EVENT_NOT_SUPPORTED_SIM      (short)6      

    /** This reason code (= 7) is used to indicate that the maximum number of 
     *  registered applet for this event is already reached (e.g Call Control)
     */
   #define EVENT_ALREADY_REGISTERED_SIM  (short)7

    /** This reason code (= 8) is used to indictae that either the offset, the
     *  length or both are out of current TLV boundaries
     */
    #define OUT_OF_TLV_BOUNDARIES_SIM     (short)8

    /** This reason code (= 9) is used to indicate that the Terminal Profile
     *  data are not available
     */
    #define ME_PROFILE_NOT_AVAILABLE_SIM  (short)9

    /** This reason code (=10) is used to indicate that the provided menu 
     *  entry string is bigger than the space alloacted space
     */
    #define ALLOWED_LENGTH_EXCEEDED_SIM 	(short)10

    /** This reason code (=11) is used to indicate that all the available timers 
     *  or the maximum number of timers have been allocated to the applet
     */
    #define NO_TIMER_AVAILABLE_SIM        (short)11

    /** This reason code (=12) is used to indicate that the indicated timer
     *  identifier is not allocated to this applet.
     */
    #define INVALID_TIMER_ID_SIM 	        (short)12

    /** This reason code (=13) is used to indicate that the registration to an 
     *  indicated event can not be changed by the called method.
     */
    #define EVENT_NOT_ALLOWED_SIM         (short)13

    /** This reason code (=14) is used to indicate that an input parameter of 
     * the method is not valid 
     */
    #define BAD_INPUT_PARAMETER_SIM       (short)14
    
    /** This reason code (=15) is used to indicate that the proactive command 
     *  being sent is not allowed by the SIM Toolkit Framework.
     */
    #define COMMAND_NOT_ALLOWED_SIM       (short)15
    
    /** This reason code (=16) is used to indicate that there is no TAR
     * defined for the applet.
     */
    #define TAR_NOT_DEFINED_SIM           (short)16
    
/******************************SIMView Exception************************************************************************/

    /** This reason code (= 1) is used to indicate that no EF is selected to
     *  complete the called method.
     */
    #define NO_EF_SELECTED_SIM        (short)1

    /** This reason code (= 2) is used to indicate that the current File
     *  inconsistent with the called method.
     */
    #define FILE_INCONSISTENT_SIM     (short)2

    /** This reason code (= 3) is used to indicate that the access condition is
     *  not fulfilled by the calling applet for the called method.
     */
    #define AC_NOT_FULFILLED_SIM      (short)3

    /** This reason code (= 4) is used to indicate to the calling applet that
     *  the file was not found in the current directory.
     */
    #define FILE_NOT_FOUND_SIM        (short)4

    /** This reason code (= 5) is used to indicate to the calling applet that an
     *  Internal error happened during the called method.
     */
    #define INTERNAL_ERROR_SIM        (short)5

    /** This reason code (= 6) is used to indicate that the called method is in
     *  contradiction with the invalidation status of the current file.
     */
    #define INVALIDATION_STATUS_CONTRADICTION_SIM  (short)6

    /** This reason code (7) is used to indicate that either the length, the file
     *  offset or both passed to the called method are out of the current
     *  transparent file boundaries.
     */
    #define OUT_OF_FILE_BOUNDARIES_SIM (short)7

    /** This reason code (8) is used to indicate that either the length, the record
     *  offset or both passed to the called method are out of the current
     *  record boundaries.
     */
    #define OUT_OF_RECORD_BOUNDARIES_SIM           (short)8

    /** This reason code (9) is used to indicate that the indicated record number is
     *  not available is the current file.
     */
    #define RECORD_NUMBER_NOT_AVAILABLE_SIM        (short)9

    /** This reason code (10) is used to indicate that the seek or record access mode
     *  requested is not supported by the called method.
     */
    #define INVALID_MODE_SIM          (short)10

    /** This reason code (11) is used to indicate that the requested pattern was not
     *  by the called method.
     */
    #define PATTERN_NOT_FOUND_SIM     (short)11

    /** This reason code (12) is used to indicate that method can not be performed as
     *  maximum value of the record is reached.
     */
    #define MAX_VALUE_REACHED_SIM     (short)12

    /** This reason code (13) is used to indicate that a memory problem occured.
     */
    #define MEMORY_PROBLEM_SIM        (short)13

 // -------------------------- Poll Interval Constants ---------------------
    /** Poll Interval : request to deregister from proactive polling = 0    */
    #define POLL_NO_DURATION_SIM                           0
    /** Poll Interval : request the system duration                  = -1   */
    #define POLL_SYSTEM_DURATION_SIM                        -1

   // ---------------------- Constants for findTLV method --------------------
    /** findTLV method result : if TLV is not found in the handler   = 0x00 */
    #define TLV_NOT_FOUND                           0
    /** findTLV method result : if TLV is found with CR set          = 0x01 */
    #define  TLV_FOUND_CR_SET                       1
    /** findTLV method result : if TLV is found with CR not set      = 0x02 */
    #define TLV_FOUND_CR_NOT_SET                    2
       
/*
*         字节１　　　　　　　			 字节2				     　　字节３						       　　字节４					      　　字节５					   　　　 字节６                                       字节7
*　b7 b6 b5 b4 b3 b2 b1 b0       b15 b14 b13 b12 b11 b10 b9 b8    b23 b22 b21 b20 b19 b18 b17 b16    b31 b30 b29 b28 b27 b26 b25 b24      b39 b38 b37 b36 b35 b34 b33 b32 	  b47 b46 b45 b44 b43 b42 b41 b40   b55 b54 b53 b52 b51 b50 b49 b48
*
*/
#define  EVENT_PROFILE_DOWNLOAD_bit                     1 /** Event : Profile Download  = 1 */
#define  EVENT_FORMATTED_SMS_PP_ENV_bit                 2 /** Event : Envelope SMS-PP Data Download (03.48 formatted)  = 2*/
#define  EVENT_FORMATTED_SMS_PP_UPD_bit                 3 /** Event : Update Record EF sms APDU     (03.48 formatted)  = 3*/                  
#define  EVENT_UNFORMATTED_SMS_PP_ENV_bit               4 /** Event : Envelope SMS-PP Data Download unformatted sms  = 4*/
#define  EVENT_UNFORMATTED_SMS_PP_UPD_bit               5 /** Event : Update Record EFsms APDU      unformatted sms   = 5*/
#define  EVENT_UNFORMATTED_SMS_CB_bit                   6 /** Event : Cell Broadcast Data Download  = 6*/            
#define  EVENT_MENU_SELECTION_bit                       7 /** Event : Menu Selection  = 7*/                 
#define  EVENT_MENU_SELECTION_HELP_REQUEST_bit          8 /** Event : Menu Selection Help Request = 8*/  
#define  EVENT_CALL_CONTROL_BY_SIM_bit                  9 /** Event : Call Control by SIM = 9*/
#define  EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit      10/** Event : MO Short Message Control by SIM = 10*/        
#define  EVENT_TIMER_EXPIRATION_bit                     11/** Event : Timer Expiration = 11*/ 
#define  EVENT_EVENT_DOWNLOAD_MT_CALL_bit               12/** Event : Event Download - MT call type = 12*/
#define  EVENT_EVENT_DOWNLOAD_CALL_CONNECTED_bit        13/** Event : Event Download - Call connected type = 13 */
#define  EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED_bit     14/** Event : Event Download - Call disconnected type = 14 */
#define  EVENT_EVENT_DOWNLOAD_LOCATION_STATUS_bit       15/** Event : Event Download - Location status type = 15*/                         
#define  EVENT_EVENT_DOWNLOAD_USER_ACTIVITY_bit         16/** Event : Event Download - User activity type = 16*/
#define  EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE_bit 17/** Event : Event Download - Idle screen available type = 17*/
#define  EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS_bit    18/** Event : Event Download - Card Reader Status = 18*/       
#define  EVENT_STATUS_COMMAND_bit                       19/** Event : Status APDU command = 19 */                             
#define  EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION_bit    20/** Event : Event Download - Language Selection = 20*/                             
#define  EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION_bit   21/** Event : Event Download - Browser Termination = 21*/         
#define  EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit     	22/** Event : Event Download - Data Available = 22*/                             
#define  EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit        23/** Event : Event Download - Channel Status = 23 */                            
#define  EVENT_FORMATTED_SMS_CB_bit                     24/** Event : Cell Broadcast Data Download Formatted = 24 */

#define  EVENT_CHV1_VERIFIED_bit                        47/** Event :proprietary Event  EVENT_CHV1_VERIFIED   = 0X8D */	//20140813

#define  EVENT_FIRST_COMMAND_AFTER_SELECT_bit           44/** Event : First Command after GSM application selection = 127*///该事件同UICC EVENT_FIRST_COMMAND_AFTER_ATR_bit事件

#define  EVENT_UNRECOGNIZED_ENVELOPE_bit                0 /** Event : Unrecognized Envelope = -1 */



  // ---------------------- Constants for TLV management --------------------
  /**
   * Value of a TLV Length field's first  in TLV with a Value field is
   * greater than 127 bytes  0x81
   */
  #define TLV_LENGTH_2BYTES  (jbyte)0x81 


 #define  EVENT_PROFILE_DOWNLOAD                       1  
                                                                 
 #define   EVENT_FORMATTED_SMS_PP_ENV                  2 
                                                                 
 #define   EVENT_FORMATTED_SMS_PP_UPD                  3                                         
                                                                                                         
 #define   EVENT_UNFORMATTED_SMS_PP_ENV                4                                         
                                                                                                         
 #define   EVENT_UNFORMATTED_SMS_PP_UPD                5                                         
                                                                                                         
 #define   EVENT_UNFORMATTED_SMS_CB                    6                                         
                                                                                                         
 #define   EVENT_MENU_SELECTION                        7                                         
                                                                                                         
 #define   EVENT_MENU_SELECTION_HELP_REQUEST           8                                         
                                                                                                        
 #define   EVENT_CALL_CONTROL_BY_SIM                   9                                         
                                                                                                         
 #define   EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM       10                                        
                                                                                                         
 #define   EVENT_TIMER_EXPIRATION                      11                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_MT_CALL                12                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_CALL_CONNECTED         13                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED      14                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_LOCATION_STATUS        15                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_USER_ACTIVITY          16                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE  17                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS     18                                        
                                                                                                         
 #define   EVENT_STATUS_COMMAND                        19                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION     20                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION    21                                        
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE 	 22                                       
                                                                                                         
 #define   EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS         23                                        
                                                                                                         
 #define   EVENT_FORMATTED_SMS_CB                      24                                        
                                                                                                         
 #define   EVENT_FIRST_COMMAND_AFTER_SELECT            127                                       
                                                                                                         
 #define   EVENT_UNRECOGNIZED_ENVELOPE                 -1      
 
 #define  EVENT_CHV1_VERIFIED                          -115/** Event :proprietary Event   = 0X8D */    //liguiyu 20140813                       
                                        

 #define  EVENT_ERROR                                    56   //当事件超过该值意味着不支持该事件，该值与卡片所支持的evenl列表有关，比如当前支持7字节的event，则支持的eventBit为 0- 55

 // ------------------------ Simple-TLV Constants -------------------------
    /** Simple-TLV : Command Details tag                             = 0x01 */
    #define TAG_COMMAND_DETAILS                      0x01 
    /** Simple-TLV : Device Identities tag                           = 0x02 */
    #define TAG_DEVICE_IDENTITIES                    0x02 
    /** Simple-TLV : Result tag                                      = 0x03 */
    #define TAG_RESULT                               0x03 
    /** Simple-TLV : Duration tag                                    = 0x04 */
    #define TAG_DURATION                             0x04 
    /** Simple-TLV : Alpha Identifier tag                            = 0x05 */
    #define TAG_ALPHA_IDENTIFIER                     0x05 
    /** Simple-TLV : Address tag                                     = 0x06 */
    #define TAG_ADDRESS                              0x06 
    /** Simple-TLV : Capability Configuration Parameters tag         = 0x07 */
    #define TAG_CAPABILITY_CONFIGURATION_PARAMETERS  0x07 
    /** Simple-TLV : Called Party Subaddress tag                     = 0x08 */
    #define TAG_CALLED_PARTY_SUBADDRESS              0x08 
    /** Simple-TLV : SS String tag                                   = 0x09 */
    #define TAG_SS_STRING                            0x09 
    /** Simple-TLV : USSD String tag                                 = 0x0A */
    #define TAG_USSD_STRING                          0x0A 
    /** Simple-TLV : SMS TPDU tag                                    = 0x0B */
    #define TAG_SMS_TPDU                             0x0B 
    /** Simple-TLV : Cell Broadcast Page tag                         = 0x0C */
    #define TAG_CELL_BROADCAST_PAGE                  0x0C 
    /** Simple-TLV : Text String tag                                 = 0x0D */
    #define TAG_TEXT_STRING                          0x0D 
    /** Simple-TLV : Tone tag                                        = 0x0E */
    #define TAG_TONE                                 0x0E 
    /** Simple-TLV : Item tag                                        = 0x0F */
    #define TAG_ITEM                                 0x0F 
    /** Simple-TLV : Item Identifier tag                             = 0x10 */
    #define TAG_ITEM_IDENTIFIER                      0x10 
    /** Simple-TLV : Response Length tag                             = 0x11 */
    #define TAG_RESPONSE_LENGTH                      0x11 
    /** Simple-TLV : File List tag                                   = 0x12 */
    #define TAG_FILE_LIST                            0x12 
    /** Simple-TLV : Location Information tag                        = 0x13 */
    #define TAG_LOCATION_INFORMATION                 0x13 
    /** Simple-TLV : IMEI tag                                        = 0x14 */
    #define TAG_IMEI                                 0x14 
    /** Simple-TLV : Help Request tag                                = 0x15 */
    #define TAG_HELP_REQUEST                         0x15 
    /** Simple-TLV : Network Measurement Results tag                 = 0x16 */
    #define TAG_NETWORK_MEASUREMENT_RESULTS          0x16 
    /** Simple-TLV : Default Text tag                                = 0x17 */
    #define TAG_DEFAULT_TEXT                         0x17 
    /** Simple-TLV : Items Next Action Indicator tag                 = 0x18 */
    #define TAG_ITEMS_NEXT_ACTION_INDICATOR          0x18 
    /** Simple-TLV : Event List tag                                  = 0x19 */
    #define TAG_EVENT_LIST                           0x19 
    /** Simple-TLV : Cause tag                                       = 0x1A */
    #define TAG_CAUSE                                0x1A 
    /** Simple-TLV : Location Status tag                             = 0x1B */
    #define TAG_LOCATION_STATUS                      0x1B 
    /** Simple-TLV : Transaction Identifier tag                      = 0x1C */
    #define TAG_TRANSACTION_IDENTIFIER               0x1C 
    /** Simple-TLV : BCCH Channel List tag                           = 0x1D */
    #define TAG_BCCH_CHANNEL_LIST                    0x1D 
    /** Simple-TLV : Icon Identifier tag                             = 0x1E */
    #define TAG_ICON_IDENTIFIER                      0x1E 
    /** Simple-TLV : Item Icon Identifier list tag                   = 0x1F */
    #define TAG_ITEM_ICON_IDENTIFIER_LIST            0x1F 
    /** Simple-TLV : Card Reader status tag                          = 0x20 */
    #define TAG_CARD_READER_STATUS                   0x20 
    /** Simple-TLV : Card ATR tag                                    = 0x21 */
    #define TAG_CARD_ATR                             0x21 
    /** Simple-TLV : C-APDU tag                                      = 0x22 */
    #define TAG_C_APDU                               0x22 
    /** Simple-TLV : R-APDU tag                                      = 0x23 */
    #define TAG_R_APDU                               0x23 
    /** Simple-TLV : Timer Identifier tag                            = 0x24 */
    #define TAG_TIMER_IDENTIFIER                     0x24 
    /** Simple-TLV : Timer Value tag                                 = 0x25 */
    #define TAG_TIMER_VALUE                          0x25 
    /** Simple-TLV : Date-Time and Time Zone tag                     = 0x26 */
    #define TAG_DATE_TIME_AND_TIME_ZONE              0x26 
    /** Simple-TLV : Call Control requested action tag               = 0x27 */
    #define TAG_CALL_CONTROL_REQUESTED_ACTION        0x27 
    /** Simple-TLV : AT Command tag                                  = 0x28 */
    #define TAG_AT_COMMAND                           0x28 
    /** Simple-TLV : AT Response tag                                 = 0x29 */
    #define TAG_AT_RESPONSE                          0x29 
    /** Simple-TLV : BC Repeat Indicator tag                         = 0x2A */
    #define TAG_BC_REPEAT_INDICATOR                  0x2A 
    /** Simple-TLV : Immediate response tag                          = 0x2B */
    #define TAG_IMMEDIATE_RESPONSE                   0x2B 
    /** Simple-TLV : DTMF string tag                                 = 0x2C */
    #define TAG_DTMF_STRING                          0x2C 
    /** Simple-TLV : Language tag                                    = 0x2D */
    #define TAG_LANGUAGE                             0x2D 
    /** Simple-TLV : Timing Advance tag                              = 0x2E */
    #define TAG_TIMING_ADVANCE                       0x2E 
    /** Simple-TLV : Browser Identity tag                            = 0x30 */
    #define TAG_BROWSER_IDENTITY                     0x30 
    /** Simple-TLV : URL tag                                         = 0x31 */
    #define TAG_URL                                  0x31 
    /** Simple-TLV : Bearer tag                                      = 0x32 */
    #define TAG_BEARER                               0x32 
    /** Simple-TLV : Provisioning Reference File tag                 = 0x33 */
    #define TAG_PROVISIONING_REFERENCE_FILE          0x33 
    /** Simple-TLV : Browser Termination Cause tag                   = 0x34 */
    #define TAG_BROWSER_TERMINATION_CAUSE            0x34 
    /** Simple-TLV : Bearer Descrition tag                           = 0x35 */
    #define TAG_BEARER_DESCRIPTION	              0x35 
    /** Simple-TLV : Channel Data tag                                = 0x36 */
    #define TAG_CHANNEL_DATA		              0x36 
    /** Simple-TLV : Channel Data length tag                         = 0x37 */
    #define TAG_CHANNEL_DATA_LENGTH	              0x37 
    /** Simple-TLV : Channel Status tag                              = 0x38 */
    #define TAG_CHANNEL_STATUS		              0x38 
    /** Simple-TLV : Buffer Size tag                                 = 0x39 */
    #define TAG_BUFFER_SIZE		              0x39 
    /** Simple-TLV : Card reader identifier tag                      = 0x3A */
    #define TAG_CARD_READER_IDENTIFIER               0x3A 
    /** Simple-TLV : SIM/ME interface transport level tag            = 0x3C */
    #define TAG_SIM_ME_INTERFACE_TRANSPORT_LEVEL     0x3C 
    /** Simple-TLV : Other address (data destination address)        = 0x3E */
    #define TAG_OTHER_ADDRESS                        0x3E 
    /** Simple-TLV : Network Access Name                             = 0x47 */
    #define TAG_NETWORK_ACCESS_NAME                  0x47 


    /** Simple-TLV : mask to set the CR flag of any Simple-TLV tag   = 0x80 */
    #define TAG_SET_CR                               0x80 
    /** Simple-TLV : mask to reset the CR flag of any Simple-TLV tag = 0x7F */
    #define TAG_SET_NO_CR                            0x7F                                         
#endif
