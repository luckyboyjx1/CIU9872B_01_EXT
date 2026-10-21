#ifndef _WHITECARDBEWRITED_H_
#define _WHITECARDBEWRITED_H_

extern u8 X_A WhiteCardBuf[0xFF];
extern u8 CONST WhiteCardWriteCardErr[];

extern u8    X_A    G_personalDataTagM;
extern u8    X_A    G_personalDataTagO;
extern u8    X_A    G_SDK_command_step;
extern u8 X_A G_applySMS;
extern u8 X_A G_applyActivate;
extern u8    X_A    G_AID_tag;
extern u8    X_A    G_Lgth_P3;
//===============================================================================
//	Bit define personal data tag
//===============================================================================
#define    B_ICCID				0                                                
#define    B_IMSI				1                                                
#define    B_SMSP				2
#define    B_PIN1				3
#define    B_PIN2				4
#define    B_PUK1				5
#define    B_PUK2				6

#define    B_K					0
#define    B_OPC				1
//===============================================================================
//===============================================================================
//	Bit define write card command step
//===============================================================================
#define    B_GETCARDINFO				0                                                
#define    B_INITIALIZEFORWRITE			1   
#define    B_WRITECARD			        2                                              


//===============================================================================

//===============================================================================
//	Bit define write card support
//===============================================================================
#define    B_XCXK				0                                                
#define    B_LBYK			    1   
#define    B_BKXK			    2                                              


//===============================================================================

#define		EF_seriesID	        0x2F02

#define		EF_WhiteCardConstructor	    0x8F01
#define		EF_WhiteCardConfig	        0x8F02
#define		EF_WhiteCardRandom	        0x8F03
#define		EF_WhiteCardNumber	        0x8F04

#define		EF_WhiteCardK1  0x8F05
#define		EF_WhiteCardK2	0x8F06
#define		EF_WhiteCardKIC	0x8F07
#define		EF_WhiteCardKID	0x8F08

#define		EF_WhiteCardBuf	0x8F09
//#define		EF_WhiteCardAID	0x8F0A
#define		EF_WhiteCardTPOA	0x8F0B
#define		EF_WhiteCardPresetTPDA	0x8F0C

#define		D_WhiteCardRFMTag				0xE8

#define	    SendSMS_SUCCESS			    0x01
#define	    SendSMS_NeedResent			0x02
#define	    SendSMS_Fail			    0x03

#define	    WriteCard_SUCCESS			        0x00
#define	    WriteCard_DataMACNotMatch			0x02
#define	    WriteCard_DataWrong			        0x03

//extern u8 X_A SelectAID_tag;
//extern u8 X_A Lgth_P3;
//extern u8 X_A Step_B5_0;
//extern u8 X_A Step_B5_1;
extern u8 X_A AppWriteResult;
extern u8 X_A G_StatusSendSMS;
extern u8 X_A G_StatusSendSMSCounter;
extern u8 X_A G_auth;
extern u8 X_A G_SMSCounter;
extern u8 X_A G_StatusFirstTime;
extern u8 X_A G_FirstAPDU;

extern void ResponseFunction(u8 SW_1,u8 SW_2);
extern void SaveRandomNumber(u8 * RandomNu);

extern void SDK_GetInformation(void);
extern void SDK_WhiteCardWriteCardHandler(void);
extern void SDK_GetWriteCardCommandResult(void);
extern u8 SDK_selectAID(void);

extern u8 SMS_WhiteCardWriteCardHandler(void);
extern void SendSMS_WriteCardApply(u8 Retry);
extern void SendSMS_NewNumberActivate(void);
extern u8 SendSMS_WriteCardResult(u8 WriteCardResult);
//extern void TAC_MAC_Calculate(u8 tpye,u8 keyIndex,u8 *inputData,u16 len,u8 * resultBuf);
extern void MAC_Calculate(u8 keyIndex,u8 *inputData,u16 len,u8 * resultBuf);
extern void TAC_Calculate(u8 * resultBuf);

extern u8 WhiteCardWriteCardHandler(void);
extern u8 WhiteCardWriteCardCheakCommand(u8 * inputData,u8 lgth);
extern u8 WhitrCardWriteCardProcess(void);
extern void SetWriteCardFlag(void);
extern u8 GetWriteCardFlag(void);

extern u8 Get_ResponseResultSMS_TR(void);
extern u8 WhiteCardcephSendShortMessageSingle(u8 commQualify,u8 CONST * alphaIdentifier, u8 * address, u8 * smsTPDU,u8 type);
extern void PowerOffProtection(u8 process);
extern void CheckProtection(void);

extern u8 GetWriteCardSupport(u8 BitIndex);

extern void ClearLociSqn(void);

extern u8 UpdateICCID(void);
extern u8 UpdateIMSI(void);
extern u8 UpdateKI(void);
extern u8 UpdateOPC(void);
extern u8 UpdateSMSP(void);
extern u8 UpdateKey(void);


#endif

