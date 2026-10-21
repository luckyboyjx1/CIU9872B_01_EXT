#ifndef    _USATENVELOPEHANDLER_H
#define	   _USATENVELOPEHANDLER_H
#include  "typedef.h"

/*******************************************************macro*****************************************************************************/
#define EVENT_FORMATTED_SMS_PP_ENV                         2
#define EVENT_FORMATTED_SMS_PP_UPD                         3
#define EVENT_UNFORMATTED_SMS_PP_ENV                       4
#define EVENT_UNFORMATTED_SMS_PP_UPD                       5
#define EVENT_UNFORMATTED_SMS_CB                           6
#define EVENT_MO_SHORT_MESSAGE_CONTROL_BY_NAA              10
#define EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM              10
#define EVENT_FORMATTED_SMS_CB                             24
#define EVENT_FORMATTED_USSD                               121
#define EVENT_UNFORMATTED_USSD                             122
#define EVENT_EVENT_DOWNLOAD_NETWORK_REJECTION             31
#define EVENT_EVENT_DOWNLOAD_CSG_CELL_SELECTION_EVENT      33
#define EVENT_EVENT_DOWNLOAD_IMS_REGISTRATION              119
#define EVENT_EVENT_DOWNLOAD_INCOMING_IMS_DATA             120
#define BTAG_CELL_BROADCATS_DOWNLOAD                       0xD2
#define BTAG_CELL_BROADCAST_DOWNLOAD                       0xD2
#define BTAG_MO_SHORT_MESSAGE_CONTROL                      0xD5
#define BTAG_SMS_PP_DOWNLOAD                               0xD1
#define BTAG_USSD_DOWNLOAD                                 0xD9
#define BTAG_GEOGRAPHICAL_LOCATION_REPORTING               0xDD
#define PRO_CMD_SEND_SS                                    0x11
#define PRO_CMD_SEND_USSD                                  0x12
#define RES_CMD_PERF_USSD_TRANSAC_TERM                     0x14
#define RES_ERROR_SS_RETURN_ERROR                          0x34
#define RES_ERROR_SMS_RP_ERROR                             0x35
#define RES_ERROR_USSD_RETURN_ERROR                        0x37 
#define RES_CMD_CALL_MO_CONTROL_PROBLEM                    0x39 
#define TAG_BC_INDICATOR                                   0x2A
#define TAG_BCCH_CHANNEL_LIST                              0x1D
#define TAG_CAUSE                                          0x1A
#define TAG_CELL_BROADCAST_PAGE                            0x0C
#define TAG_SMS_TPDU                                       0x0B
#define TAG_SS_STRING                                      0x09
#define TAG_TIMING_ADVANCE                                 0x2E
#define TAG_TRANSACTION_IDENTFIER                          0x1C
#define TAG_USSD_STRING                                    0x0A
#define TAG_PDP_CONTEXT_ACTIVATION_PARAMETERS              0x52
#define TAG_UTRAN_MEASUREMENT_QUALIFIER                    0x69
#define TAG_CSG_ID_LIST                                    0x7E
#define TAG_CSG_SELECTION_STATUS                           55
#define TAG_CSG_SELECTION_STATUS_N                         55
#define TAG_CSG_ID                                        0x56
#define TAG_HNB_NAME                                      0x57 
#define TAG_I_WLAN_IDENTIFIER                             0x4A
#define TAG_I_WLAN_ACCESS_STATUS                          0x4B
#define TAG_PLMN_WACT_LIST                                0x72
#define TAG_ROUTING_AREA_INFORMATION                      0x73
#define TAG_UPDATE_ATTACH                                 0x74
#define TAG_REJECTION_CAUSE_CODE                          0x75
#define TAG_GEOGRAPHICAL_LOCATION_PARAMETERS              0x76
#define TAG_GAD_SHAPES                                    0x77
#define TAG_NMEA_SENTENCE                                 0x78
#define TAG_PLMN_LIST                                     0x79
#define TAG_EPS_PDN_CONNECTION_ACTIVATION_PARAMETERS      0x7C
#define TAG_TRACKING_AREA_IDENTIFICATION                  0x7D
#define TAG_IMS_REQUEST_URI                               0x31 
#define TAG_IARI                                          0x76 
#define TAG_IMPU_LIST                                     0x77 
#define TAG_IMS_STATUS_CODE                               0x78 
/******************************************************function********************************************************************/
extern void USATEnvelop_getShortMessageOffset(void);

extern void USATEnvelop_getShortMessageLength(void);

#endif
