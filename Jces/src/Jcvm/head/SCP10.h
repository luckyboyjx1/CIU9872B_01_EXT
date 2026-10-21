/*
 * scp10.h
 *
 *  Created on: 2013-9-16
 *      Author: jiangxiaohui
 */

#ifndef SCP10_H_
#define SCP10_H_

//SCP10 INS 移动到opFunction.h中，避免出现incompatible redefinition of macro "INS_OP_PERFORM_SECURITY_OPERATION"

    /** Bit setting */
#define	BIT_8 												0x80
#define BIT_7												0x40
#define BIT_6 												0x20
#define BIT_5 												0x10
#define BIT_4												0x08
#define BIT_3 												0x04
#define BIT_2 												0x02
#define BIT_1 												0x01

#define	TAG_80												0x80
//#define	TAG_81												0x81
//#define	TAG_82												0x82
#define	TAG_86												0x86
#define	TAG_87												0x87
#define	TAG_8E												0x8E
#define	TAG_96												0x96
#define	TAG_97												0x97
#define	TAG_99												0x99
#define	TAG_42												0x42
#define	TAG_53												0x53
#define	TAG_93												0x93
#define	TAG_95												0x95
#define	TAG_5F20											0x5F20
#define	TAG_5F24											0x5F24
#define	TAG_5F25											0x5F25

#define	SKSDAUTCRT											0xAA
#define PKSDAUTSF											0xAB
#define	PKOCEAUT											0xAC
#define PKTP_EXAUT											0xAD


#define SESSION_KEY_TAG										0xD1
#define CCT_TAG												0xB4
#define CT_TAG												0xB8
#define KEY_USAGE_QUA										0x95
#define INI_SEQ_CNT											0x91
#define SUBJECT_ID											0x5F20


#define IS_NEED_CHECK_TAG_NAME_YES							0x5A
#define IS_NEED_CHECK_TAG_NAME_NO							0xA5
#define	ONE_BYTE_TAG										0x01
#define	TWO_BYTE_TAG										0x02
#define	SIZE_OF_ONEBYTETAG_TAG								0x01
#define	SIZE_OF_ONEBYTETAG_TAGANDLENGTH						0x02
#define	SIZE_OF_TWOBYTETAG_TAG								0x02
#define	SIZE_OF_TWOBYTETAG_TAGANDLENGTH						0x03


#define	SIZE_LENGTH_OF_82_TAG								0x03
#define	SIZE_LENGTH_OF_81_TAG								0x02
#define	SIZE_LENGTH_OF_OTHER_TAG							0x01


#define OFFSET_STATE_MACHINE								0x00
#define OFFSET_KEYS_CRT_LENGTH 								(OFFSET_STATE_MACHINE+1)
//#define OFFSET_HASH										(OFFSET_CHALLENGE+16)
#define OFFSET_KEY_FLAG										(OFFSET_KEYS_CRT_LENGTH+1)
#define OFFSET_REG_APID										(OFFSET_KEY_FLAG+1)
#define OFFSET_CHALLENGE									(OFFSET_REG_APID+1)
#define OFFSET_CMAC_CNT										(OFFSET_REG_APID+1)
#define OFFSET_RMAC_CNT										(OFFSET_CMAC_CNT+1)
#define OFFSET_RDEK_CNT										(OFFSET_RMAC_CNT+1)
#define OFFSET_CDEK_CNT										(OFFSET_RDEK_CNT+1)
#define OFFSET_CENC_CNT										(OFFSET_CDEK_CNT+1)
#define OFFSET_RENC_CNT										(OFFSET_CENC_CNT+1)
#define OFFSET_FOUR_CS										(OFFSET_RENC_CNT+1)
#define OFFSET_FOUR_OES										(OFFSET_FOUR_CS+4)
#define OFFSET_CDATA_ADD_CONST_1                            0x06
#define OFFSET_CDATA_ADD_CONST_2                            0x07
#define OFFSET_CDATA_ADD_CONST_3                            0x08
#define OFFSET_CDATA_ADD_CONST_4                            0x09

//产生密钥状态
#define OFFSET_STATE_GENKEYPAIR								OFFSET_STATE_MACHINE
//TP根公钥存在状态
#define OFFSET_STATE_TPEXPK									(OFFSET_STATE_GENKEYPAIR+1)
//公钥证书存在状态
#define OFFSET_STATE_CERT									(OFFSET_STATE_TPEXPK+1)


#define SW_AUTHENTICATION_FAILED_WARNING 					0x6300

#define AGREEMENT_OPTION									0x01
#define TRANSPORT_OPTION									0x02

#define EXTERNAL_AUT_STATE 									0x06
#define INTERNAL_AUT_STATE									0x05
#define GET_CHALLENGE_STATE 								0x04
#define PERFORM_DECIPHER_STATE								0x03
#define GET_VERIFY_CERT_STATE								0x02
#define MANAGE_SEC_ENV_STATE 								0x01
//#define INIT_STATE											0x00  //remove warning,dengyiwen,2022/9/22

#define LENGTH_RSA_1024										1024
#define PRIVATE_KEY											0x00
#define PUBLIC_KEY											0x01


#define REG_APID_MATCH 										0x01
#define REG_APID_NO_MATCH									0x00

#define writeRamOneByte(src, value1)							* src = value1;

#define writeRamTwoByte(src, value1, value2)					* src = value1;\
																* (src + 1) = value2;

#define writeRamThreeByte(src, value1, value2, value3)			* src = value1;\
																* (src + 1) = value2;\
																* (src + 2) = value3;

#define writeRamFourByte(src, value1, value2, value3, value4)	* src = value1;\
																* (src + 1) = value2;\
																* (src + 2) = value3;\
																* (src + 3) = value4;

#if GP__SCP__SCP10__FOR_MASK

extern u16 scp10_process(void);
extern void scp10_manageSecurityEnvironment(PX8	tempBufForSCP10);
extern void scp10_performVerifyCertificate(PX8	tempBufForSCP10);
//extern u8 scp10_recoverKey(NVM_U8_P keySetObj, u8* inBuff, u8 inOffset,u8* outBuff, u8 outOffset);
extern void scp10_singleKeyParam(PX8 sessionKeyFlag, PX8 CNT1, PX8 CNT2, PX8 tempBuff ,u8 bitValue, u8 value1, u8 value2, u8 temOffset, u8 needSetBit);
extern void scp10_doubleKeyParam(PX8 sessionKeyFlag, PX8 CNT, PX8 tempBuff ,u8 bitValue, u8 value, u8 optionValue, u8 temOffset1, u8 temOffset2);
extern void scp10_dealSecLvlCRTData(PM8 dataBuf, u8 dataBufOff, u8 dataBufLen, u8 optionValue, PX8	tempBufForSCP10);
extern u16 scp10_searchTag(PM8 inBuffer, u8 offTag, u8 LenTag, u16 Tag);
extern void scp10_genICVSCP10(u8 option , PM8 SrcAddr, PM8 DesAddr , u8 length);
extern void scp10_proSesKeysCRT(PM8 buffer, u16 offset, u8 length,  u8 flag, u8 option);
extern void scp10_performDecipher(PX8 TransientDataBufForSCP10, PX8	tempBufForSCP10);
extern u16 scp10_getChallenge(PX8	tempBufForSCP10);
extern void scp10_externalAuthenticate(PX8 TransientDataBufForSCP10, PX8	tempBufForSCP10, PN8 pushArray);
extern u16 scp10_internalAuthenticate(PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10, boolean processSecurityFlag);
extern void scp10_agreementExternalAuthenticate(PX8 inBuff, u8 inOffset, u8 inLength, PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10, PN8 pushArray);
extern void scp10_transportExternalAuthenticate(PM8 inBuff, u16 inOffset, u16 inLength, PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10);
extern u16 scp10_agreementInternalAuthenticate(PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10);
extern void scp10_computeAndSetSessionKey(PM8 inBuff, PM8 tempBuff);
extern u16 scp10_transportInternalAuthenticate(PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10);
extern void scp10_generateKeyPair(void);
extern void CalMACSCP10(PM8 srcAddress, u8 len, PM8 keyBuff, u8 mode);
extern void scp10_ICVSeqCntINC(PM8 seq, u8 format);
extern u8 scp10_dealencData(u8 le);
extern u8 scp10_dealMacDataProc(u8 tag, u8 tagLen, u8 bLc , u8 le);
extern u8 scp10_dealMacData(u8 le, u8 option);
extern u16 decryptCommandDataSCP10(void);
extern u8 scp10_dealRMacData(PM8 SrcAddr, u8 length, u8 option, u8 sw1, u8 sw2);
extern u16 EncryptOfResponseSCP10(u8 securityLevel, PM8 SrcAddr, u8 length, u16 sw);
extern void clearLevelThrowError(PX8 tempBufForSCP10, u16 err);
extern void clearPKOCE(void);
#endif

#endif /* SCP10_H_ */
