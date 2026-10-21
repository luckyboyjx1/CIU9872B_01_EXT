/*****************************************************************************
 * File: cardManger.c	
 * Version 1.0 03/09/24

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/

#ifndef   __CARD__MANAGER_H__
#define   __CARD__MANAGER_H__
#include "profile.h"
#include "typejava.h"
#include "target.h"

/* ISO Exception SW */
#define SW_NO_ERROR		              		(u16) 0x9000
#define SW_BYTES_REMAINING                  (u16) 0x6100
#define SW_MORE_GETDATA						(u16) 0x6310
#define SW_WRONG_LENGTH	              		(u16) 0x6700
#define SW_SECURE_MESSAGEING_NOT_SUPPORTED	(u16) 0x6882
#define SW_SECURITY_STATUS_NOT_SATISFIED    (u16) 0x6982
#define SW_FILE_INVALID                     (u16) 0x6983
#define SW_DATA_INVALID	              		(u16) 0x6984
#define SW_CONDITIONS_NOT_SATISFIED	      	(u16) 0x6985
#define SW_COMMAND_NOT_ALLOWED	      		(u16) 0x6986
#define SW_APPLET_SELECT_FAILED	      		(u16) 0x6999
#define SW_WRONG_DATA	                    (u16) 0x6A80
#define SW_FUNC_NOT_SUPPORTED               (u16) 0x6A81
#define SW_FILE_NOT_FOUND                   (u16) 0x6A82
#define SW_RECORD_NOT_FOUND                 (u16) 0x6A83
#define SW_INCORRECT_P1P2 	              	(u16) 0x6A86
#define SW_REFERENCED_DATA_NOT_FOUND 		(u16) 0x6A88
#define SW_WRONG_P1P2 	              		(u16) 0x6B00
#define SW_CORRECT_LENGTH                   (u16) 0x6C00
#define SW_INS_NOT_SUPPORTED                (u16) 0x6D00
#define SW_CLA_NOT_SUPPORTED                (u16) 0x6E00
#define SW_UNKNOWN                          (u16) 0x6F00
#define SW_BIPMAP_OVER_FLOW					(u16) 0x6F02
#define SW_FUNC_NOT_SUPPRORT				(u16) 0x6A81
#define SW_FILE_FULL                        (u16) 0x6A84
#define SW_LOGICAL_CHANNEL_NOT_SUPPORTED    (u16) 0x6881
#define SW_SECURE_MESSAGING_NOT_SUPPORTED   (u16) 0x6882
#define SW_WARNING_STATE_UNCHANGED          (u16) 0x6200
#define SW_INVALID_KEY_CHECK_VALUE       	(u16) 0x9485
#define SW_AUTHCOUNTER_EXPLORED             (u16) 0x6A8E
#define SW_VERIFICATION_CERT_FAILED         (u16) 0x6600 //modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
#define SW_CERT_NOT_IN_WHITELIST            (u16) 0x6640
//#define SW_PRIVATE_ERROR                    (u16) 0x90FF //modify by guobing@20220809 在throwPrivateError函数中使用 (此函数原为UICC中使用，现测试需要在不开UICC时使用)

/*********************EUICC Exception*********************************************************/
#define EUICC_PROFILE_PREVENT_ACTION    (u16)0x69E1

/*********************Specified Exception*********************************************************/
#define Card_Ability_Exception          (u16)0x6BAB
#define ASN1_Exception                  (u16)0x6BBE
#define PROFILE_ILLEGAL_EXCEPTION       (u16)0x6BBF


/* ISO CLA */
#define CLA_ISO7816                         (u8)0x00
#define INS_SELECT   		              	(u8)0xA4
#define INS_MANAGECHANNEL 		      		(u8)0x70
#define P1_SELECT_DFBYNAME 		      		(u8)0x04

#define P2_SELECT_OPTIONS 		      		(u8)0xFD//(u8)0xE1   //0xFD ICC标准，0xE1 TCK标准
#define P2_SELECT_OPTIONS_ONLY              (u8)0x00
#define P1_OPEN_CHANNEL 		      		(u8)0x00
#define P1_CLOSE_CHANNEL 		      		(u8)0x80
#define P2_AUTOSELECT_CHANNEL 	            (u8)0x00
#define ERR_NO_CHANNEL_AVAILABLE 	      	(u8)0xFF

#define  NO_SPECIFIC_DIAGNOSIS_WARNING      (jshort)0x6200
#define  APPLICATION_INVALIDATED            (jshort)0x6283
#define  AUTHENTICATION_FAILED              (jshort)0x6300
#define  NO_SPECIFIC_DIAGNOSIS_ERROR        (jshort)0x6400
#define  MEMORY_FAILURE                     (jshort)0x6581
#define  AUTHENTICATION_FAILED_WARNING      (jshort)0x6300

#define JCRE_CONTEXTID (u16)0x0000
#define NULL_CONTEXTID (u16)0xFFFF

/* card status and applet status  */
#define CARD_OP_READY                      	(u8)0x01
#define CARD_INITIALIZED                   	(u8)0x07
#define CARD_SECURED                       	(u8)0x0F
#define CARD_LOCKED                        	(u8)0x7F
#define CARD_TERMINATED                    	(u8)0xFF
#define APPLET_LOGICALLY_DELETED          	(u8)0x00
#define APPLET_LOADED                     	(u8)0x01
#define APPLET_INSTALLED                  	(u8)0x03
#define APPLET_SELECTABLE                 	(u8)0x07
#define APPLET_PERSONALIZED               	(u8)0x0F
#define APPLET_BLOCKED                    	(u8)0x7F
//#define APPLET_LOCKED                     	(jbyte)0xFF
#define APPLET_LOCKED                       (u8)0x83
#define SSD_LOCKED					(u8)0x8F
#define SSD_SELECTABLE				(u8)0x07
#define SSD_PERSONALIZED			(u8)0x0F
#define SELECT_APPLET_FLAG    0xEE

#define PROFILE_DISABLE				(u8)0x1F
#define PROFILE_ENABLE				(u8)0x3F

//此函数需要在除UICC的平台中使用，在SPI新方案中使用，在此处重新实现 modify by guobing@20220809
//#define throwPrivateError()     throw_error(SW_PRIVATE_ERROR)

extern  void Dispatcher1(void);
//void selectApplet(void);
extern  void sendSelectResponceAPDU(jref aid, u8 appId);
//void channelManagerCommand(void);
//void selectAppletByFileID(void);

extern  void initCPLCData(void);
extern  void initISDDiversifiedDataArr(void);
extern  u8 initSSDDiversifiedDataArr(void);
extern void initDiversifiedDataArr(void);
extern void SetbSDId(void);
extern u8 NM_getAppIndexByPrivilege(void);
extern  u8 getCardState(u8 isdId);
extern  void setCardState(u8 isdId, u8 state)    SMALL_MODE;
extern  jref getIsdAppletRef(u8 isdId);
extern	void receiptGeneration(void);
extern void channelManagerCommand(void);
extern u8 isHaveVolatilePriority(memref theEntryObjectAddr);
extern void getPrivilegeByAppId(u8 AppID, PX8 privilege);
extern u8 getAppIndexByPrivilege(u8 privilege, u8 number);
extern u16 getAppIDByIndex(u8 index)	SMALL_MODE;
#endif
