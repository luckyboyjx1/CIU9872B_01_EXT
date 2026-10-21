#ifndef    _COMPACTMANAGE_H
#define	   _COMPACTMANAGE_H
#include "typedef.h"


#define		TAR3_SHARE_FILE		0x02	//共享文件
#define		TAR3_CSIM_ADF		0x20	//CSIM ADF
#define		TAR3_USIM_ADF		0x21	//USIM ADF
#define		TAR3_USIM_ADF_Add		0x00	//USIM ADF
#define		TAR3_ISIM_ADF		0x22	//ISIM ADF
#define		TAR3_SIM			0x11

#define D_TmpBufStart 232
#define D_TmpBufStartWithAES 208
////////////Variable Tag//////////////////////
#define		LDFileTPOATag			0xA1	//TPOA缓存标志
#define		LDFileDeliverTag		0xA2	//Deliver级联使用

typedef __packed  struct extendedRFMScriptContext
{
    BYTE     firstScriptValue;     //记录第一条scriptValue
    BYTE     previousValue;        //记录上一条的chainingValue
    BYTE     pinStatus;
    BYTE     recNum;                               
    WORD     dfName;
    WORD     efName;
    UWORD    dfAddr;
    UWORD    efAddr;
    //WORD     secondeParrentName;    
}extendedRFMContext;	

//*************************************function********************************
extern extendedRFMContext xdata ChainingValue;
extern BYTE xdata G_FormatAndType;
//*******************************extended RFM extern ******************************************* 
extern BYTE xdata G_CurrentChainingValueTag;   //保存当前操作的ChainingValue
extern WORD xdata G_ErrorActionOffsetLgth[2];   //指示当前最后一个ErrorActionTLV在文件中的偏移
extern BYTE xdata G_EarlyResponseTag ;         //标志当前是否存在early response TAG
extern BYTE xdata G_CAPDUResponse;
extern BYTE xdata  G_CurrentComNUM;  
extern BYTE xdata G_ChainingTagRec;
//******************文件操作信息*********************************
#define     ReadFailed              0x00//0xB1
#define     CreateFailed            0x00//0xB2 
#define     ResizeFailed            0x00//0xB3
#define     UpdateFailed            0x00//0xB4
#define     DeleteFailed            0x00//0xB5
#define     DeactiveFaile           0x00//0xB6
#define     ActiveFailed            0x00//0xB7

#define TAG_CommandScriptingTemplateDefiniteLength     0xAA      //确定长度
#define TAG_ResponseScriptingTemplateDefiniteLength    0xAB
#define TAG_CommandScriptingTemplateInDefiniteLength   0xAE      //长度不确定
#define TAG_ResponseScriptingTemplateInDefiniteLength  0xAF
#define TAG_C_APDU                             0x22
#define TAG_ImmediateAction                    0x81
#define TAG_ErrorAction                        0x82
#define TAG_ScriptChaining                     0x83

#define TAG_R_APDU                             0x23
#define TAG_NumberCommandTLV                   0x80
#define TAG_NumberCAPDU                        0x80
#define TAG_ImmediateActionResponse            0x81
#define TAG_ScriptChainingResponse             0x83
#define TAG_BadFormat                          0x90
#define FLAG_TagNotFound                       0x01
#define FLAG_WrongLgth                         0x02
#define FLAG_LgthNotFound                      0x03
#define Value_SuspensionError                  0x01
#define TAG_ClearResponseFile                  0xAA

#define  Action_ReadBit                        0x00
#define  Action_UpdateBit                      0x01
#define  Action_ResizeBit                      0x02
#define  Action_DeleteBit                      0x03
#define  Action_DeactiveBit                    0x04
#define  Action_ActiveBit                      0x05

#define  RemoteDataMidBlock                  0x00   //不是最后一块 对http用于
#define  RemoteDataEndBlock                    0x01   //最后一块     sms/CATTP 都是调用这个    Http end时候也是用这个

/**********************function define*************************************************************************/
extern void RemoteReadBinary(void);
extern BYTE setCurDfNameAndAddrbyTAR(void);
extern void InitRemoteManagevariable(void);
extern BYTE RAMApduProcess(UWORD fileAddr,WORD commandOffset, WORD dataLen);
extern BYTE RFMApduProcess(UWORD fileAddr,WORD commandOffset, WORD dataLen);
extern void GetSCP80KeyValueBeforeUpdate(BYTE *buff,BYTE mode);
extern void ProcessRFMinit(WORD *fileName);
extern BYTE ProcessRAMinit(void);
extern void ProcessRemoteMoveFalshForHTTP(UWORD SrcfileAddr,WORD Offset, WORD cplLen);
extern BYTE ProcessRemoteCheckCAPDU(WORD cplLen);
extern BYTE CdmaTransGsm(void);
extern void InitVariableRemoteModule(void);
/********************duan song song *****************************************************/
extern BYTE GetTlvTandLBytes(BYTE * dataBuf);

extern WORD GetTlvLValue(BYTE * dataBuf);

extern BYTE AddLtoTlv(BYTE * dataBuf, WORD dataLen);

extern BYTE RemoteAPDUProcessFunction(UWORD fileAddr,WORD commandOffset, WORD cplLen, BYTE mode,BYTE *isTriHttp);
//*******************************extended RFM extern *******************************************
extern BYTE ExtendedRFMProcessBadTLV(BYTE errorCode);
extern void cdmaRemoteReadBinary(void);
extern BYTE ExtendedRFMUpdateResponseTLV(BYTE tag);

//#define TAR2_UICCFile_System        0x02   //B00002: UICC shared File System
//#define TAR2_USIMFile_System        0x03   //B00020  USIM fileSystem  移动USIM TAR
//#define TAR2_CSIMFile_System        0x04   //B00021  CSIM fileSystem  移动USIM TAR
//#define TAR2_PRL_System             0x05   //BF01FF  PRL TAR    //目前ppdownload里面的PRL TAR只用来PUSH 来通道 当做RFM TAR来处理
//#define TAR2_EPRL_System            0x06   //BF02FF  EPRL TAR
//#define TAR2_OPLMN_System           0x07   //BF03FF  OPLMN TAR
//#define TAR2_PKCS_System            0x08   //B001FF  PKCS TAR  路径0x7F50开始


	
#endif
