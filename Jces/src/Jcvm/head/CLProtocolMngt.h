#ifndef __CLProtocolMngt_H__
#define __CLProtocolMngt_H__

typedef struct{

	PM8 ProtocolData;
	PM8 MandatoryMask;

	PM8 CurrentProtocolData;
	PM8 CurrentMandatoryMask;
	PM8 CurrentCompactData;

	PM8 PrepaProtocolData;
	PM8 PrepaMandatoryMask;
	PM8 PrepaCompactData;

	u8  ResultData[49];	//TODO:大小确认
	u8  ResultMask[49];

} ComputContextStruct;


typedef struct{

	PM8 ProtocolData1;
	PM8 MandatoryMask1;
	PM8 ProtocolData2;
	PM8 MandatoryMask2;
	u8 ConflictDetected;

} DetectionContextStruct;


#define   XOR_OPERATOR   0x0F
#define   MAX_OPERATOR   0xFF

// TYPE A ?????????
//#define UIDRANDOM_TYPEA_OFF                  (EECFG_TYPEA_UIDRANDOM - EECFG_TYPEA)
//#define UIDLEN_TYPEA_OFF                     (EECFG_TYPEA_UIDLEN - EECFG_TYPEA)
//#define UID_TYPEA_OFF                        (EECFG_TYPEA_UID - EECFG_TYPEA)
//#define ATSLEN_TYPEA_OFF                     (EECFG_TYPEA_ATSLEN - EECFG_TYPEA)
//#define ATS_TYPEA_OFF                        (EECFG_TYPEA_ATS - EECFG_TYPEA)
//#define ATS_LEN_TYPEA_OFF                    (EECFG_TYPEA_ATS_LEN - EECFG_TYPEA)
//#define ATS_FMT_TYPEA_OFF                    (EECFG_TYPEA_ATS_FMT - EECFG_TYPEA)
//#define ATS_TA1_TYPEA_OFF                    (EECFG_TYPEA_ATS_TA1 - EECFG_TYPEA)
//#define ATS_TB1_TYPEA_OFF                    (EECFG_TYPEA_ATS_TB1 - EECFG_TYPEA)
//#define ATS_TC1_TYPEA_OFF                    (EECFG_TYPEA_ATS_TC1 - EECFG_TYPEA)
//#define ATS_HIS_TYPEA_OFF                    (EECFG_TYPEA_ATS_HIS - EECFG_TYPEA)
//#define UPDATE_TYPEA_FDT_ADJ_TYPEA_OFF       (EECFG_TYPEA_UPDATE_TYPEA_FDT_ADJ - EECFG_TYPEA)
//#define ATQA_TYPEA_OFF                       (EECFG_TYPEA_ATQA - EECFG_TYPEA)
//#define SAK_TYPEA_OFF                        (EECFG_TYPEA_SAK - EECFG_TYPEA)
//#define SAK_MIFARE1K_OFF                     (MIFARE_SAK_1K - EECFG_TYPEA)
//#define SAK_MIFARE4K_OFF                     (MIFARE_SAK_4K - EECFG_TYPEA)

// TYPE B 简单格式的参数偏移

//#define MBLI_TYPEB_OFF                       (EECFG_TYPEB_MBLI - EECFG_TYPEB)
//#define AFI_TYPEB_OFF                        (EECFG_TYPEB_AFI - EECFG_TYPEB)
//#define PUPIRANDOM_TYPEB_OFF                 (EECFG_TYPEB_PUPIRANDOM - EECFG_TYPEB)
//#define ATQBLEN_TYPEB_OFF                    (EECFG_TYPEB_ATQBLEN - EECFG_TYPEB)
//#define ATQB_TYPEB_OFF                       (EECFG_TYPEB_ATQB - EECFG_TYPEB)
//#define TYPEB_PUPI_OFF                       (EECFG_TYPEB_PUPI - EECFG_TYPEB)
//#define TYPEB_AFI_OFF                        (EECFG_TYPEB_AFI - EECFG_TYPEB)
//#define TYPEB_CRC1_OFF                       (EECFG_TYPEB_CRC1 - EECFG_TYPEB)
//#define TYPEB_CRC2_OFF	                     (EECFG_TYPEB_CRC2 - EECFG_TYPEB)
//#define TYPEB_APPLICATIONNUMBER_OFF			 (EECFG_TYPEB_APPLICATIONNUMBER - EECFG_TYPEB)
//#define TYPEB_BITRATE_OFF					 (EECFG_TYPEB_BITRATE - EECFG_TYPEB)
//#define TYPEB_MAXFRAMESIZE_OFF				 (EECFG_TYPEB_MAXFRAMESIZE - EECFG_TYPEB)
//#define TYPEB_FWI_ADC_FO_OFF				 (EECFG_TYPEB_FWI_ADC_FO - EECFG_TYPEB)
//#define TYPEB_EXTENDED_ATQB_OFF			     (EECFG_TYPEB_EXTENDED_ATQB - EECFG_TYPEB)
//#define TYPEB_HISTORICAL_BYTES_OFF			 (EECFG_TYPEB_HISTORICAL_BYTES - EECFG_TYPEB)
//#define TYPEB_TYPEBPARA_MAX_OFF				 (EECFG_TYPEB_TYPEBPARA_MAX - EECFG_TYPEB)

// TYPE A TLV 格式的参数偏移

#define  UID_TLV             0
#define  SAK_TLV             (UID_TLV + 13) //13
#define  ATQA_TLV            (SAK_TLV + 3)  //16
#define  ATS_HIS_TLV         (ATQA_TLV + 4)  //20
#define  FWI_SFGI_TLV        (ATS_HIS_TLV + 18)  //38
#define  CID_TLV             (FWI_SFGI_TLV + 3)  //41
#define  DATA_RATE_MAX_TLV   (CID_TLV + 3)  //44

// TYPE B TLV 格式的参数偏移
#define  PUPI_TLV              0
#define  AFI_TLV               (PUPI_TLV + 7)
#define  ATQB_TLV              (AFI_TLV + 3)
#define  TB_HIS_TLV            (ATQB_TLV + 6) //TODO:确认HIS最大长度
#define  TB_DATA_RATE_MAX_TLV  (TB_HIS_TLV + 18)

//TYPE A para length
#define  PARA_TYPEA_LENGTH         49
#define  PARAMASK_TYPEA_LENGTH     49

//TYPE B para length
#define  PARA_TYPEB_LENGTH         49
#define  PARAMASK_TYPEB_LENGTH     49


extern ComputContextStruct * ComputContext;
extern DetectionContextStruct * DetectionContext;
extern PN8 bEECFG;

memref GetCurrentCLParaBuf(void);
memref GetPrepareCLParaBuf(void);
void SwitchCLParaBuf(void);

void LvDataComput(short TagOff);
void BasicDataComput(u16 ValueOff, u16 Len);
void NumericDataComput(u16 ValueByteOff, u8 fullbyte);
void BasicMaskComput(u16 TagOff,  u8 ValueOff);
void LMaskComput(u16 TagOff);
void TypeAProtoDataTLVToCompact(PM8 scr, PM8 dest);
void TypeAProtoDataCompactToTLV(PM8 scr, PM8 dest);
void TypeBProtoDataTLVToCompact(PM8 scr, PM8 dest);
void TypeBProtoDataCompactToTLV(PM8 scr, PM8 dest);

void BasicCflctDetcXOR(u16 ValueOff, u8 length);
void LCflctDetc(u16 TagOff, u8 ProtocolType);
void DetectionInit(PM8 Data1, PM8 Mask1, PM8 Data2, PM8 Mask2);

u8 TypeAConflictDetection(PM8 Data1, PM8 Mask1, PM8 Data2, PM8 Mask2, u8 ProtcolType);
u8 TypeBConflictDetection(PM8 Data1, PM8 Mask1, PM8 Data2, PM8 Mask2, u8 ProtcolType);

void ComputStart(u8 ProtoType);
u16 GetProtocolPara(memref ArrayObj, u8 offset, u8 type, u8 flag);
void StoreProtocolTypeBPara(PM8 ParaBuf, u16 ParaLen);
void StoreProtocolTypeAPara(PM8 ParaBuf, u16 ParaLen);

void InitializeProtocolTypeBPara(void);
void InitializeProtocolTypeAPara(void);
void InitComputContext(void);
void ComputEnd(u8 ProtoType);
void TypeAComputeCurrentProtocolParameter(void);
void TypeBComputeCurrentProtocolParameter(void);
extern void conflictDetectionNM(void);
extern void ComputeCurrentProtocolParameterNM(void);
extern void ProtocolParameterBootstrapNM(void);
extern void StoreProtocolParaNM(void);
extern void GetProtocolParaNM(void);
extern void SetCurrentProtocolDataNM(void);
void GetCurrentProtocolParaNM(u8 ProtoType) ;
void GetDefaultProtocolParaNM(memref ProtoParaObject) ;

void clearProtocolData(void);

#if INFINEON_FSR8
void ModifyCurrentSAKValue(u32 protocolParaAddress, u8 sak);
#endif
#endif
