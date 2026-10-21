/*
 * SCPInterface.h
 *
 *  Created on: 2013-9-13
 *      Author: jiangxiaohui
 */

#ifndef SCPINTERFACE_H_
#define SCPINTERFACE_H_

#include "profile.h"
#include "global_variable.h"

extern u8 CONST  AuthCounterNoChangesdPackageAID[0x0E];//modify by xuxiaojun@20220628 用于SCP02和SCP03共同支持伪counter

#define SC0102_BASE_1KEY 					0x00
#define SC0102_3KEY 						0x01
#define SC0102_ICV_OVER_AID 				0x08
#define SC0102_ICV_FOR_CMAC					0x10
#define SC0102_RMAC_SUPPORT 				0x20
#define SC0102_PSEU_RAND 					0x40
/*
for SCP01/SCP02 parameter i
---------------------------------------------
  b8      |b7|b6|b5|b4|b3|b2|b1|Description
Reserved  |  |	|  |  |  |  |1 |3 Secure Channel Keys
Reserved  |  |  |  |  |	 |	|0 |1 Secure Channel base key
Reserved  |  |  |  |  |	 |1 |  |C-MAC on unmodified APDU
Reserved  |  |  |  |  |	 |0 |  |C-MAC on modified APDU
Reserved  |  |  |  |  |1 |  |  |Initiation mode explicit
Reserved  |  |  |  |  |0 |  |  |Initiation mode implicit
Reserved  |  |  |  |1 |  | 	|  |ICV set to MAC over AID
Reserved  |  |  |  |0 |  | 	|  |ICV set to zero
Reserved  |  |  |1 |  |  |	|  |ICV encryption for C -MAC session
Reserved  |  |  |0 |  |  |	|  |No ICV encryption
Reserved  |  |1 |  |  |	 |	|  |R-MAC support
Reserved  |  |0 |  |  |	 |	|  |No R-MAC support
Reserved  |1 |  |  |  |	 |	|  |Well-known pseudo -random algorithm (card challenge)
Reserved  |0 |  |  |  |	 |	|  |Unspecified card challenge generation method
---------------------------------------------
*/

#define SCP01_IPARA_05   0x05
/*
"i" = '05': Initiation mode explicit, C- MAC on modified APDU, ICV set to zero, no ICV encryption, 3
Secure Channel Keys ;
*/

/*
"i" = '15': Initiation mode explicit, C- MAC on modified APDU, ICV set to  zero, ICV encryption for
C - MAC session, 3 Secure Channel Keys, unspecified card challenge generation method, no R-MAC
*/
#define SCP0102_IPARA_15   0x15
/*
"i" = '1A': Initiation mode implicit, C - MAC on unmodified APDU, ICV set to MAC over AID, ICV
encryption for C- MAC session, 1  Secure Channel  base key. no R - MAC;
*/
#define SCP02_IPARA_1A   0x1A
/*
"i" = '55': Initiation mode explicit, C- MAC on modified APDU, ICV set to zero, ICV encryption for
C - MAC session, 3 Secure Channel Keys, well- known pseudo- random algorithm (card challenge),
no R- MAC
*/
#define SCP02_IPARA_55   0x55


#define SC03_RAND_CARD_CHALL 								0x00
#define SC03_PSE_RAND_CARD_CHALL 							0x10
#define SC03_NORMAC_NORENC 									0x00
#define SC03_SUPPROT_RMAC_NORENC 									0x20
#define SC03_SUPPROT_RMAC_RENC 										0x60
#define SC03_USEDBIT 										0x70

/*
for scp03 parameter i
---------------------------------------------
b8|b7|b6|b5|b4|b3|b2|b1|Description
  |  |	|  |X |X |X |X |RFU (set to 0)
  |  |  |0 |  |	 |	|  |Random card challenge
  |  |  |1 |  |	 |  |  |Pseudo-random card challenge
  |0 |0 |  |  |	 |  |  |No R-MAC/R-ENCRYPTION support
  |0 |1 |  |  |  |  |  |R-MAC support / no R-ENCRYPTION support
  |1 |1 |  |  |  |  |  |R-MAC and R-ENCRYPTION support
X |  |  |  |  |  | 	|  |Reserved
---------------------------------------------
*/

/*
"i" = '1A': Initiation mode implicit, C - MAC on unmodified APDU, ICV set to MAC over AID, ICV
encryption for C- MAC session, 1  Secure Channel  base key. no R - MAC;
*/
#define SCP03_IPARA_USEDBIT   	 0x70
#define SCP03_IPARA_NOTSUPPROT   0x40



/*
 * add by jxh 20130902
 *
 * the method is only used in openSecureChannel method,
 * check if the SCPType we supported
 * check p1, p2 is available for current SCPType's parameter 'i'
 *
 * in:
 * 	scpType: current SCP Type
 * 	i_option : current SCP Type's parameter 'i'
 * out:
 *	SW_NO_ERROR
 *	SW_AUTHCOUNTER_EXPLORED
 *	SW_INCORRECT_P1P2
 *
 * */

#define CHECK_IS_SUPPORT_SCP_AND_PARAM_I  	0x01
#define CHECK_IS_SUPPORT_RMAC  	0x02



#define SCP_STATICKEY_FIRST_KEYID		0x01
#define SCP_STATICKEY_SECOND_KEYID		0x02
#define SCP_STATICKEY_THIRD_KEYID		0x03
#define SCP_STATICKEY_FOURTH_KEYID		0x04
#define SCP_STATICKEY_FIFTH_KEYID		0x05

#define SCF_CMAC_MASK 0x01
#define SCF_DECRY_MASK 0x02
#define SCF_CMAC_DECRY_MASK 0x03
#define SCF_RMAC_MASK 0x10
#define SCF_RMAC_ENCRY_MASK 0x30
#define SCF_CMAC_RMAC_DECRY_ENCRY_MASK 0x33

#define FIRST_COME_GENERATE_CARD_CHALLENGE 0x01
#define SECOND_COME_GENERATE_CARD_CHALLENGE 0x02

#define SD_IS_LOCKED 								0x01
#define TRYCOUNT_BIG_THAN_MAXTRYCOUNT 				0x02
#define KEYVERSION_IS_NOT_EXIST 					0x04
#define SEQUENCE_COUNTER_REACH_MAX 					0x08

#define CARD_CRYPTOGRAM_SCP0102_OFFSET_INAPDU       25
#define CARD_CRYPTOGRAM_SCP03_OFFSET_INAPDU       	26

#define BEFORE_MAC_CAL								0x55
#define AFTER_MAC_CAL								0xAA

#define CMAC_CAL 									0x55
#define RMAC_CAL 									0xAA

#define RESULT_OK 									0x55
#define RESULT_NOK 									0xAA
#define CLEARED_VALUE								0x00

#define	MACLENGTH_SUB_APDUHEADSIZE					0x03

#define GP_80										0x80
#define GP_84										0x84
#define GP_SECURITY_MASK							0x04
#define GP_NO_SECURITY_MASK							0xFB
#define GP_NO_LOGIC_MASK							0xFC
#define GP_NO_LOGIC_NO_SECURITY_MASK				0xF8

#define IS_NO_NEED_CHECK_SD							0x00
#define IS_NEED_CHECK_SD_LOCKED						0x01
#define	IS_NEED_CHECK_SD_PERSONALIZED				0X02
#define	IS_NEED_CHECK_SD_KEY_INTERGRATED			0X04

#define	SCP03_SEQUENCE_CONTER_SIZE					0x03
#define	SCP02_SEQUENCE_CONTER_SIZE					0x02
#define SCP80_SEQUENCE_CONTER_SIZE					0x05

#define SC_ABORTED									0x01
#define SC_TERMINATED								0x02
typedef struct scpDataArea
{
	u8 scpSESKey[SCP_KEY_LENGTH]; // #define SCP_KEY_LENGTH					((SESSION_KEY_LENGTH * MAX_LOGICAL_CHANNELS))
	u8 scpDEKKey[SCP_KEY_LENGTH];
	u8 scpMACKey[SCP_KEY_LENGTH];
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC||GP__SCP__SCP11__FOR_MASK)//modify by jiangxiaohui@20220324 分层
	u8 scpRMACKey[SCP_KEY_LENGTH];
#endif
	u8 scpCardChallenge[SCP_CARDCHALLENGE_LENGTH];
	u8 scpHostChallenge[SCP_HOSTCHALLENGE_LENGTH];
//	u8 scpLastMAC[SCP_LASTMAC_LENGTH]; 被修改为RAM存储
//	u8 scpCurrentMAC[SCP_CURRENTMAC_LENGTH]; 被修改为RAM存储
//	u8 scpBackMAC[SCP_BACKMAC_LENGTH]; 被修改为RAM存储
#if GP__SCP__SCP10__FOR_MASK
	u8 scpRSESKey[SCP_KEY_LENGTH];
	u8 scpRDEKKey[SCP_KEY_LENGTH];
#endif
} scpDataArea_t,
    *pscpDataArea_t; /**< pointer to structure scpDataArea_t */

extern u8 g_SCP02SDID;
extern u8 scpLastMAC[SCP_LASTMAC_LENGTH];
extern u8 scpCurrentMAC[SCP_CURRENTMAC_LENGTH];
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC||GP__SCP__SCP11__FOR_MASK)//modify by jiangxiaohui@20220324 分层
extern u8 scpBackMAC[SCP_BACKMAC_LENGTH];
#endif

#define SCP_DATA_LENGTH					sizeof(scpDataArea_t) /*((SCP_KEY_LENGTH*6) + SCP_CARDCHALLENGE_LENGTH +SCP_HOSTCHALLENGE_LENGTH+SCP_LASTMAC_LENGTH+SCP_CURRENTMAC_LENGTH+SCP_BACKMAC_LENGTH)*/
#define g_SCP_SES_KEY						MEMtPN8(SCP_DATA_START)
#define g_SCP_DEK_KEY						PN8_A_CONST(g_SCP_SES_KEY, SCP_KEY_LENGTH)
#define g_SCP_MAC_KEY						PN8_A_CONST(g_SCP_DEK_KEY, SCP_KEY_LENGTH)
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC||GP__SCP__SCP11__FOR_MASK)//modify by jiangxiaohui@20220324 分层
#define g_SCP_RMAC_KEY						PN8_A_CONST(g_SCP_MAC_KEY, SCP_KEY_LENGTH)
#else
#define g_SCP_RMAC_KEY						g_SCP_MAC_KEY
#endif
#define g_SCP_CARDCHALLENGE					PN8_A_CONST(g_SCP_RMAC_KEY, SCP_KEY_LENGTH)
#define g_SCP_HOSTCHALLENGE					PN8_A_CONST(g_SCP_CARDCHALLENGE, SCP_CARDCHALLENGE_LENGTH)
#if GP__SCP__SCP10__FOR_MASK
#define g_SCP_RSES_KEY						PN8_A_CONST(g_SCP_HOSTCHALLENGE, SCP_HOSTCHALLENGE_LENGTH)
#define g_SCP_RDEK_KEY						PN8_A_CONST(g_SCP_RSES_KEY, SCP_KEY_LENGTH)
#endif

#define g_SCP_LASTMAC			       		scpLastMAC
#define g_SCP_CURRENTMAC					scpCurrentMAC
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC||GP__SCP__SCP11__FOR_MASK)//modify by jiangxiaohui@20220324 分层
#define g_SCP_BACKMAC						scpBackMAC
#endif

#define g_current_SCP_SES_KEY  				PN8_A_U8(g_SCP_SES_KEY, U8_SHL_U8(selected_channel, SESSION_KEY_OFFSET))
#define g_current_SCP_RSES_KEY  			PN8_A_U8(g_SCP_RSES_KEY, U8_SHL_U8(selected_channel, SESSION_KEY_OFFSET))
#define g_current_SCP_DEK_KEY  				PN8_A_U8(g_SCP_DEK_KEY, U8_SHL_U8(selected_channel, SESSION_KEY_OFFSET))
#define g_current_SCP_DEK_KEY_L				PN8_A_CONST(g_current_SCP_DEK_KEY , SESSION_KEY_LENGTH_HALF)

#define g_current_SCP_RDEK_KEY  			PN8_A_U8(g_SCP_RDEK_KEY, U8_SHL_U8(selected_channel, SESSION_KEY_OFFSET))
#define g_current_SCP_MAC_KEY  				PN8_A_U8(g_SCP_MAC_KEY, U8_SHL_U8(selected_channel, SESSION_KEY_OFFSET))
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC||GP__SCP__SCP11__FOR_MASK)//modify by jiangxiaohui@20220324 分层
#define g_current_SCP_RMAC_KEY  			PN8_A_U8(g_SCP_RMAC_KEY, U8_SHL_U8(selected_channel, SESSION_KEY_OFFSET))
#endif

#define g_current_SCP_cardChallenge  		PN8_A_U8(MEMtPN8(g_SCP_CARDCHALLENGE), U8_SHL_U8(selected_channel, CARD_CHALLENGE_LENGTH_OFF))
#define g_current_SCP_hostChallenge  		PN8_A_U8(MEMtPN8(g_SCP_HOSTCHALLENGE), U8_SHL_U8(selected_channel, HOST_CHALLENGE_LENGTH_OFF))

#define g_current_SCP_LAST_MAC				PN8_A_U8(g_SCP_LASTMAC, U8_SHL_U8(selected_channel, LAST_MAC_LENGTH_OFFSET))
#define g_current_SCP_LAST_MAC_H  			g_current_SCP_LAST_MAC
#define g_current_SCP_LAST_MAC_L  			PN8_A_CONST(MEMtPN8(g_current_SCP_LAST_MAC) , LAST_MAC_LENGTH_HALF)

#define g_current_SCP_CURRENT_MAC			PN8_A_U8(MEMtPN8(g_SCP_CURRENTMAC), U8_SHL_U8(selected_channel, CURRENT_MAC_LENGTH_OFFSET))
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC||GP__SCP__SCP11__FOR_MASK)//modify by jiangxiaohui@20220324 分层
#define g_current_SCP_BACK_MAC				PN8_A_U8(MEMtPN8(g_SCP_BACKMAC), U8_SHL_U8(selected_channel, CURRENT_MAC_LENGTH_OFFSET))
#endif




#define managerChannelLevel(level) 				 	(channelInfo.security_level[selected_channel] = level)
#define managerRequestedChannelLevel(level)  		(channelInfo.requested_security_level[selected_channel] = level)
#define managercompulsorySessionSecurityLevel(level) (channelInfo.compulsory_Session_Security_Level[selected_channel] = level)

extern  void initScpInterface(void);

#if GP__SCP
	extern void	scpClr(void);
	extern u16	openSecureChannel(u8 APDUDataFieldIsReceived);
	extern u8	groupApduResponseOfOpenChannel(PN8 keySetObj);
	extern u16	verifyExternalAuthenticate(u8 APDUDataFieldIsReceived);
	extern void	incrementSequenceCounter(void);
	extern u16	operateBeginRmacSession(void);
	extern u16	operateEndRmacSession(void);
	extern void	closeSecuritychannel(u8 isAborted);
//	extern void	SWP_closeSecuritychannel(void);
	extern u16	checkIsAuthenticated(u8 ignoreAuth);
	extern u8	ManageSecurityLevel(u8 SecurityLevel);
	extern memref	getCurrentSdDataObj(u8 isNeedCheckSDIsLocked, u8* tmpKeyVersion);//modify by xxj@20230512: 修正缺陷487
	extern memref	getKeySetObj(PN8 sdDataObj, u8 keyVersion);
	extern memref	InitAndCheckSCPParam(void);
	extern memref	getCurrentSdObj(u8 isNeedCheckSDIsLocked);//只用于获取当前SD的obj
	extern u16 nativeAESSignatureDoFinalCal(PN8 keySetObj, u8 keyId, memref inBuff, u16 inLen, memref sigBuff, u16 sigLen, u8 mode, u8 flag);
#else
	#define scpClr()
	#define openSecureChannel(APDUDataFieldIsReceived)											0
	#define groupApduResponseOfOpenChannel(keySetObj)											0
	#define verifyExternalAuthenticate(APDUDataFieldIsReceived)									0
	#define incrementSequenceCounter()
	#define operateBeginRmacSession()															0
	#define operateEndRmacSession()																0
	#define closeSecuritychannel()
//	#define SWP_closeSecuritychannel()
	#define ManageSecurityLevel(SecurityLevel)													0
	#define getCurrentSdDataObj(isNeedCheckSDIsLocked)											0
	#define getKeySetObj(sdDataObj, keyVersion)													0
	#define InitAndCheckSCPParam()																0
	#define getCurrentSdObj(isNeedCheckSDIsLocked)												0
	#define closeSecuritychannel(isAborted)
	#if GP__VERSION__FOR_MINI_MASK
		extern u16	checkIsAuthenticated(u8 ignoreAuth);
	#else
		#define checkIsAuthenticated(ignoreAuth)												SW_NO_ERROR
	#endif
#endif

#if (JAVA_CARD_UICC && !HAL__PLATFORM__SIMULATOR) //modify by jiangxiaohui@20220324 分层
extern scpDataArea_t CONST SCP_DATA_AREA;
#else
extern scpDataArea_t scpData;
#endif

#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC || GP__SCP__SCP11__FOR_MASK)
	extern u16    GenRMACOfResponse(PX8 SrcAddr ,u16 length, u16 sw, boolean isSCPAPI);
	extern u16    EncryptAndGenRMAC(PM8 srcAddress, u16 length, u16 status, boolean isSCPAPI);
	extern void   EncryRMACResponseData(u16 status);

#else
	#define GenRMACOfResponse(SrcAddr ,length, sw, isSCPAPI)
	#define EncryptAndGenRMAC(srcAddress, length, status, isSCPAPI)
	#define EncryRMACResponseData(status)
	//#define Remote_EncryptAndGenRMAC()
#endif
	extern u16 Remote_EncryptAndGenRMAC(void);
#endif /* SCPINTERFACE_H_ */
