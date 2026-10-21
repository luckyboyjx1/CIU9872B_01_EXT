#ifndef    _CMCCAPIIMPL_h
#define	   _CMCCAPIIMPL_h
#include "typedef.h"

/*-------------------------------CMCC FILE tYPE----------------------------------------*/
#define CMCC_File_Transparent                  0x00
#define CMCC_File_FixedRecord                  0x01
#define CMCC_File_VariableRecord               0x02
#define CMCC_File_CyclicRecord                 0x03


/*----------------------------CMCC ACtion define----------------------------------------*/
#define CMCC_ReadAction                        0x00
#define CMCC_UpdateAction                      0x01
#define CMCC_AppendTagAction                   0x02
/*-------------------------------------Record Mode define------------------------------*/
#define CMCC_RecordMode_Next                   0x02
#define CMCC_RecordMode_Previous               0x03
#define CMCC_RecordMode_CurAndAbsolute         0x04
/*------------------------------------FILE System define -----------------------------*/
#define CMCC_FileHead_Len                      0x09

#define CMCC_FileExist_Offset                  0x00
#define CMCC_FileSFI_Offset                    0x01
#define CMCC_FileType_Offset                   0x02
#define CMCC_FileAccess1_Offset                0x03
#define CMCC_FileAccess2_Offset                0x04
#define CMCC_FileAccess3_Offset                0x05
#define CMCC_FileRecordNumber_Offset           0x06   //对于透明文件或可变记录文件来说，这两个字节指示的是文件大小的第一字节
#define CMCC_FileRecordLength_Offset           0x07   //对于透明文件或可变记录文件来说，这两个字节指示的是文件大小的第二字节
#define CMCC_FileRecordPointer_Offset          0x08   //对于透明文件，该字节无意义；对于线性文件来说，指示的是记录指针;对可变记录文件，该字节存储的是当前已append的记录号



/*----------------EXCEPTION DEFINE----------------------------------*/
#define CMCCLIB_SW_NO_NVM_RESOURCE              0x0007   //如果在C层要抛该 异常的话，需要在处理异常的地方InterpreterRun将该值修改成0x6484,暂时未处理 
#define CMCCLIB_SW_NO_TRANSIENT_RESOURCE        0x0008   //如果在C层要抛该异常的话，需要在处理异常的地方InterpreterRun将该值修改成0x6485，暂时未处理
#define CMCCLIB_INPUT_DATA_INVALID              0x0009
#define CMCCLIB_RESULT_OVER_FLOW                0x000A
#define CMCCLIB_BEYOND_LIMITATION               0x000B
/*------------------FUNCTION EXTERN-------------------------------*/
extern void NM_CMCCASCToUCS2(void);
extern void NM_CMCCUCS2ToASC(void);
extern void NM_CMCCASCToRBCD(void);
extern void NM_CMCCRBCDToASC(void);
extern void NM_CMCCASCToBCD(void);
extern void NM_CMCCBCDToASC(void);
extern void NM_CMCCASCToHEX(void);
extern void ConvertHexToASCII(UWORD value,BYTE* buf);
extern void NM_CMCCHEXToASC(void);
extern void NM_CMCCBalanceFormat(void);
extern void NM_CMCCByteToShort(void);
extern void NM_CMCCArrayAdd(void);
extern void NM_CMCCArraySub(void);
extern void NM_CMCCFindTLV(void);

extern void NM_CMCCCreateEFFile(void);
extern void NM_CMCCgetType(void);
extern void NM_CMCCgetAccessRight(void);
extern void NM_CMCCgetFileSize(void);
extern void NM_CMCCgetRecordNumber(void);
extern void NM_CMCCgetRecordLength(void);
extern void NM_CMCCUpdateBinary(void);
extern void NM_CMCCreadRecord(void);
extern void NM_CMCCreadBinary(void);
extern void NM_CMCCupdateRecord(void);
extern void NM_CMCCfindRecordByTag(void);
extern void NM_CMCCappendRecord(void);

extern void CMCCLibThrowException(short reason);
extern BYTE CMCC_GetRecNo(UWORD fileHeadAddr,WORD sfiIndex, BYTE currentNum,BYTE mode,BYTE recordNumber,BYTE action);

extern UWORD CMCCCalculatePublic(BYTE* buf,BYTE* MacLen);
extern void NM_CMCCCalculateMTAC(void);
extern void NM_CMCCCalculateSessionKey(void);
extern void NM_CMCCDecryptData(void);
extern void NM_CMCCEncryptData(void);
extern void NM_CMCCGetRandom(void);
extern void NM_CMCCVerify(void);

extern void NM_CMCCGetMSISDN(void);
extern void NM_GetKs_int_NAF(void);

extern void NM_Check_AFL(void);

extern void NM_CMCCGetOnlineState(void);
extern void NM_CMCCAllocate(void);
extern void CMCCSetOrGetData(BYTE mode);
extern void NM_CMCCSetData(void);
extern void NM_CMCCGetData(void);
extern void NM_CMCCRelease(void); 
extern void NM_CMCCSetNFCRecordState(void);
extern void NM_CMCCGetNFCRecordState(void);
extern void NM_CMCCGetNFCRecord(void);

#define 	State_Function_Not_Supported   0x00
#define    	State_Offline                              0x01
#define    State_Online                              0x02


#define Bit_NFCRecordState                 0x00   //该bit位表示NFC刷卡的状态
#define Bit_IsDesignateAPP                 0x01   //该bit位表示是否指定应用可以获取NFC刷卡的信息


extern 		BYTE 	xdata 	RAM_Function_Supported;
extern 		BYTE 	xdata 	RAM_Online_Flag;
//extern 		BYTE 	xdata	RAM_ATR_Flag; // modify by xuxiaojun@20220318 分层, 该变量属于Boot层
extern		BYTE 	xdata 	RAM_Location_Status_Flag;

#endif

