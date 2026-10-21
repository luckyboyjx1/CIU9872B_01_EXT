#ifndef	 _gsmpublic_h_
#define  _gsmpublic_h_
//#include "resume.h"
#include "typedef.h"
#include "uicc_main.h"
#include "VMAccessEeprom.h"

#define NOT_PROFILE_MODE						0x00
#define PROFILE_MODE							0x10
#define PROFILE_MODE_TEMPLATE					0x11
#define PROFILE_MODE_GENERIC					0x12

//----------------	public.c	------------------------------------------------------------------------
#define  OFFSET_NULL                    0xFFFF
#define  PRIVATETAG_NULL                0xFF

#define  PRIVATETAG_PORSENDTIMES        0x01         //POR重发次数

#define  PRIVATETAG_SWPSAK              0x81       //SWP SAK
#define  PRIVATETAG_SWPATQA             0x82       //SWP ATQA 
#define  PRIVATETAG_SWPFWI              0x8A       //SWP FWI

#define  PRIVATETAG_SWPNOTYPEA          0x83       //Do not creat type A
#define  PRIVATETAG_SWPNOTYPEB          0x84       //Do not creat type B
#define  PRIVATETAG_SWPNOTYPEAREADER    0x85       //Do not creat type A reader
#define  PRIVATETAG_SWPNOTYPEBREADER    0x86       //Do not creat type B reader
#define  PRIVATETAG_SWPNOLOOPBACK       0x87       //Do not creat Loop back gate
#define  PRIVATETAG_SWPNOIDMANAGEMENT   0x88       //Do not creat IDMANAGEMENT 
#define  PRIVATETAG_SWPNOCONNECTIVITY   0x89       //Do not creat CONNECTIVITY

#define  PRIVATETAG_SIMEID_CONFIG       0xF6
#define  PRIVATETAG_UDHI_ZERO           0xF7       //用于设置兼容短信的UDHI为0的情况。是由于SIMPLUS测试时，在华为服务器上UDHI的的值&0x40 == 0x40了，但是服务器不改，所以只能COS去兼容

#define Read_Key	0x01
#define Write_Key	0x02

#define SetStatusMode     0x00
#define ClearStatusMode   0x01
#define GetStatusMode     0x02

#define  AID_FullMatch        0x00
#define  AID_PartitalMatch    0x01

extern BYTE code USIMPartial[8];

extern BYTE code CSIMPartial[8];

extern BYTE code ISIMPartial[8];

extern BYTE GetAIDNumberForMode(BYTE *aidBuf, BYTE mode);

#define Inrange(curRecord, minRecord, maxRecord)	 (((curRecord)>=(minRecord))&&((curRecord)<=(maxRecord)))

#define  GetAIDNumberByPartialAID(aidBuf)       GetAIDNumberForMode(aidBuf, AID_PartitalMatch)

extern UWORD SelectFileByShortId(UWORD  fileAddr,BYTE shortName);

extern void BackFilePath(PathInfo * ptrToFilePath);

extern BYTE ResumeFilePath(PathInfo * ptrToFilePath);

extern void GetAIDNameInFile(BYTE recordNum,BYTE *aidBuf);

extern BYTE GetAIDNumber(BYTE *aidBuf, BYTE profileType);

extern void DeleteDIRNum(BYTE recNum);

extern BYTE OffsetAfterFirstOccurrence(BYTE physicalNo,BYTE value,BYTE recLen);

extern BYTE SearchContaining(SBYTE step,BYTE recordNo,BYTE offsetValue,BYTE * searchString,BYTE length,BYTE needAfterFirstOccurrence);

extern BYTE  SearchStarting(BYTE  recordNo,BYTE maxRecnum,SBYTE step);

extern BYTE FindTag(BYTE * dataPointer,BYTE tag,BYTE lgth);

extern BYTE FindTagFullMatch(BYTE * dataPointer,BYTE tag,BYTE lgth);

extern BYTE ReceiveData(void);                                                                  //接收数据到RECDATA缓冲区中，最多个

extern void SendData(void);                                                                       //发送RECDATA缓冲区中的数据?

extern UWORD SearchFile(UWORD  fileAddr,WORD fileName);

extern UWORD SelectFile(UWORD  fileAddr,WORD fileName);

extern UWORD SelectFileByName(UWORD  fileAddr,BYTE * name,BYTE lgth);

extern BYTE  AccessCondition(UWORD fileAddr,BYTE action);

extern WORD GetAcessPurview(UWORD fileAddr,BYTE action);

extern  BYTE WriteKey(BYTE keyId,BYTE * buf);

extern  BYTE ReadKey(BYTE keyId,BYTE * buf);                                                 //find the addr of the key 

extern void ReadPinStatus(void);                                                                           //把DF文件头信息拼凑成GSM标准的样子

extern void ClearLocalPinStatus(UWORD fileAddr,BYTE *pinstatus);

extern  void SimFdnSYNCtoUSIM(BYTE fdnStatus);

extern BYTE IsFdnServiceActiveInUSIM(void);

extern BYTE IsFdnServiceActiveInSIM(void);

extern void UsimFdnSYNCtoSIM(BYTE fdnStatus);

extern void FileLifeCycleHandle(WORD dfName,WORD efName,BYTE action);

extern void FdnProgcess(void);

extern BYTE  GetKeyRefenseStatus(WORD keyRefense);

extern BYTE ConfigureKeyStatus(WORD keyRefense,BYTE mode);
extern BYTE LocateRecordInFile(UWORD fileAddr, BYTE tag);

//----------------------	main.c	-------------------------------------------------------------
extern void PpsProcess(void);

extern void CommandAttemper(FLASH INSLIST *inslist,BYTE mode);


//----------------------	gsmpublic.c	----------------------------------------------------------------------
extern BYTE GetEfInformation(BYTE * dataBuf);

extern BYTE GetDfInformation(BYTE * dataBuf);

extern BYTE UsimGetEfInformation(BYTE *dataBuf) ;

extern BYTE UsimGetDfInformation(BYTE *dataBuf);

extern BYTE SetChannel(UWORD fileAddr,WORD fileName,BYTE option);

extern BYTE GetCurrentADFAid(WORD fileName,BYTE * buf);

//extern BYTE BaseUsimGetDfInformation(BYTE *dataBuf);

//extern BYTE BaseUsimGetEfInformation(BYTE *dataBuf) ;

#define BaseUsimGetDfInformation UsimGetDfInformation
#define BaseUsimGetEfInformation UsimGetEfInformation
/*
	standhard function of gsmFunction.c
*/

extern UWORD SelectByFileName(WORD fileName);

extern BYTE GetRecordNo(UWORD fileAddr,BYTE type,BYTE mode,BYTE record,BYTE currentRec);

extern BYTE GetPhyNum(BYTE action,UWORD fileAddr,BYTE mode,BYTE accRecord,BYTE* currentRec);

extern  void ReadBinary(void);

extern  void UpdateBinary(void);

extern  void SelectCommand(void);

extern  void StatusCommand(void);

extern  void STKStatusCommand(void);

extern  void GetResponseCommand(void); 

extern  void FetchCommand(void);

extern  void EnvelopeCommand(void);

extern void EnvelopeCommandSTK(void);

extern void STKTerminalProfile(void);

extern  void ReadRecord(void);

extern  void UpdateRecord(void);

extern  void UpdateRecordSTK(void);

extern  void TerminalResponse(void);

extern  void IncreaseCommand(void);

extern void RunGsmAlgorithm2G(void);

extern void RunGsmAlgorithmNEW(void);

extern  void SeekCommand(void);

extern void TerminalProfile(void);

extern BYTE ComputeBinary(BYTE action);

extern WORD GetADFFID(BYTE *PartialAidBuf);


//---------------------	gsmChvManagement.c	---------------------------------------
#define	mode_Verify 	0x02
#define mode_Change		0x01
#define mode_Unblock	0x00

extern	BYTE KeyResume(BYTE x);

extern	void VerifyChangeUnblockChv(void);

extern  void ChvHandle(void);


extern void Sleep(void);

extern  void Invalidate(void);

extern  void Rehabilidate(void);

extern  void FileStatusHandler(void);

extern void ReadAuthPara(void);

extern void IF_ReceiveAPDU(void);
/*
    USIM Command define
*/
extern  void UsimManageChannel(void);

extern  void UsimGetChallenge(void);

extern void TerminalCapability(void);

extern void UsimXORAuthentication(void);

extern  void UsimAuthentication(void);

extern UWORD CheckCurrentDfForADF(void);

/*
    peso command of exteralcommand.c 
*/
//-------------	CreateFile.c	----------------------------------------------------
extern UWORD createfile_sub(UWORD currentDfAddr,BYTE* buf,BYTE len,WORD fileName,BYTE filetype);

extern BYTE DeleteFile_sub(UWORD fileAddr, UWORD CurrentDfAddr);
extern void InitTransFileEX(UWORD fileBodyAddr,BYTE * ptrToLen,WORD bodySize,BYTE mode);
extern void InitRecordFileEX(UWORD fileBodyAddr,BYTE * ptrToLen,BYTE recNum, BYTE recLen,BYTE mode);
extern void StartSecureMoveEeprom(UWORD desAddr,UWORD srcAddr, UWORD dataLen);
extern void StartSecureMoveEepromMapTableExtend(void);

extern  UWORD createfile_sub(UWORD currentDfAddr,BYTE* buf,BYTE len,WORD fileName,BYTE filetype);

extern  void CreateFile(void);

extern void  DeleteFile(void);

extern void  ResizeFile(void);

extern  void GetIdentity(void);

extern  void SuspendUicc(void);

extern WORD  publicResizeFile(BYTE * ptrToValue,BYTE pos,BYTE mode,UWORD fcpData);

extern void InitTransFile(UWORD fileBodyAddr,BYTE * ptrToLen,WORD bodySize);

extern void InitRecordFile(UWORD fileBodyAddr,BYTE * ptrToLen,BYTE recNum, BYTE recLen);

//-------------	resume.c define	-------------------------------------------------------------------
extern void PersoReadEeprom(void);  

extern void PersoWriteEeprom(void);

extern void SetBank(void);

extern void VerifyFlashCRC(void);

//------------------	exteralcommand.c	--------------------------------------------------------
#define	mode_MFKey	        0x00
#define	mode_ManagerKey	    0x01

#define	mode_VerifyKey	        0x00
#define	mode_ReloadKey	        0x80

extern  void Initializtion(void);

extern void ReloadKey(void);

extern void PrivateFileCheck();

#if UICC_FUCTION_PRIVATEAUTH
extern WORD ReloadVerifyManagenerKey(BYTE * apduBuffer);
#endif

#if SE_ONLY
extern void ReloadVerifyManagenerKey(void);
#endif
extern WORD SetRedTeaRootAESKEY(BYTE * apduBuffer);
extern WORD UpdateAuthData(BYTE * apduBuffer);

extern BYTE ComputeAll(BYTE action, BYTE nByShortFID,BYTE nfile);

extern void SmartLumingRead(void);

extern WORD FindPrivateFileTag(BYTE tag,BYTE * outBuf);

extern void ClearFileFirstWord(WORD dfName,WORD efNAme);

extern WORD ReadTFFileWord(UWORD fileAddr);

extern void WriteTFFileWord(UWORD fileAddr,WORD len);

extern WORD ReadLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf);

extern BYTE ReadLFFileValue1ByteLen(UWORD fileAddr,BYTE index,BYTE offset,BYTE * buf);

extern BYTE WriteLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf,WORD length);

extern BYTE WriteTFFileValue(UWORD fileAddr,WORD offset,BYTE * buf,WORD length,WORD totalLen);

extern void ClearFileFirstWordAddr(UWORD fileAddr);

extern UWORD GetBerTlvLength(BYTE * buffer,WORD * lenByte);
 
//extern u8 ASN1_GetBERTlvLen(u32 srcAddr,u32* len); 在ASN1Operation.h中已经声明
 
extern UWORD GetMFUsedSize(void);

extern BYTE BackDFNameForMoveFalsh(WORD DFFileName[PROFILEDELETE_BACKUPFILESYSTEM]);

extern void ReloadDFOrEFAddr(WORD fileName[PROFILEDELETE_BACKUPFILESYSTEM]);

extern BYTE GetMaxLocalPinIndex(void);

extern BYTE ReadOrWritePukInfo(BYTE pukRef,BYTE * buf,BYTE mode);

extern BYTE IF_CleanShareableFile(UWORD fileAddr,WORD fileName);

extern UWORD getADFAddr(void);

extern void dataOrder(BYTE size,BYTE *Buf);

extern void RunAuth(void);

extern BYTE GetPrivateArrInfoToExpand(BYTE arrNum,BYTE * buf,BYTE * bufTemp);

extern BYTE GetDefaultFileSystemPrivateArrABInfo(UWORD addr,BYTE arrNum,BYTE * buf);

extern BYTE CompactToExpand(BYTE * ptrToLen,BYTE * resultBuf);

extern BYTE LocalKeyEstablish(void);

extern void InitTransOrRecordFile(UWORD fileAddr,WORD startOffset,BYTE * initBuf,BYTE heRecNo,BYTE mode);
#endif
