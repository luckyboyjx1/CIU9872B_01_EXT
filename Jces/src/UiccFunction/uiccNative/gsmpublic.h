#ifndef	 _gsmpublic_h_
#define  _gsmpublic_h_
#include   "typedef.h"
#include   "uicc_main.h"

#define CDMAFDN  1
#define GSMFDN   0


//----------------	public.c	------------------------------------------------------------------------
#define Read_Key	0x01
#define Write_Key	0x02

#define SetStatusMode     0x00
#define ClearStatusMode   0x01
#define GetStatusMode     0x02

/***************COS 可配置信息 Tag****************************************************************************/
#define MODE_CONFIG_WRITE        0x00
#define MODE_CONFIG_READ         0x01
#define MODE_CONFIG_CLEARTAG     0x02
#define MODE_CONFIG_CLEARALL     0x03
#define MODE_CONFIG_READALL      0x04

#define  PRIVATETAG_NULL   		  0xFF
#define  OFFSET_NULL      		  0xFF
#define  PRIVATETAG_INVALID       0x00

#define Update6F5BRAM	        0x00
#define Update6F5BFlash	        0x01

#define Tag_Config_Algorithm           0x01         //V1/V2/V3算法配置
#define Tag_Config_Update6F5B          0x02         //文件6F5B更新方法标记
#define Tag_Config_javaapi             0x03         //api中CRC16 的版本，31.048的标记
#define Tag_Config_TA1				   0x04         //
#define Tag_USIMAlgorithm_Type		   0x51
#define Tag_Config_Por_SendTimes       0x81         //POR发送次数
#define Tag_Config_NotUpdateFile       0xFE

#define Tag_Config_IfSetUpMenu         0xA1         //SetupMenu的标记
#define Tag_Config_WFRS_SendTimes      0xA2         //WFRS发送次数
#define Tag_Config_IfDisplayUnkownTar  0xA3         //是否显示未知TAR错误的标记
#define Tag_Config_IfDisplayWibTag     0xA4         //错误信息’%D’不显示WIB command tag
#define Tag_Config_BackStep            0xA5         //逐级回退标志

#define Inrange(curRecord, minRecord, maxRecord)	 (((curRecord)>=(minRecord))&&((curRecord)<=(maxRecord)))

#define  AID_FullMatch        0x00
#define  AID_PartitalMatch    0x01


//#define  GetAIDNumber(aidBuf)                   GetAIDNumberForMode(aidBuf, AID_FullMatch)
#define  GetAIDNumberByPartialAID(aidBuf)       GetAIDNumberForMode(aidBuf, AID_PartitalMatch)

extern BYTE GetAIDNumberForMode(BYTE *aidBuf, BYTE mode);

extern UWORD SelectFileByShortId(UWORD  fileAddr,BYTE shortName);

extern void BackFilePath(PathInfo * ptrToFilePath);

extern BYTE ResumeFilePath(PathInfo * ptrToFilePath);

extern BYTE ReadOrWriteKey(BYTE keyId, BYTE * buf, BYTE mode);

extern void GetAIDNameInFile(BYTE recordNum,BYTE *aidBuf);

extern BYTE GetAIDNumber(BYTE *aidBuf);

extern void DeleteDIRNum(BYTE recNum);

extern BYTE OffsetAfterFirstOccurrence(UWORD fileContentAddr,BYTE physicalNo,BYTE value,BYTE recLen);

extern BYTE SearchContaining(signed char step,BYTE recordNo,BYTE offsetValue,BYTE * searchString,BYTE length,BYTE needAfterFirstOccurrence);

extern BYTE  SearchStarting(BYTE  recordNo,BYTE maxRecnum,signed step);

extern void IF_InitSystem(void);

extern BYTE FindTag(BYTE * dataPointer,BYTE tag,BYTE lgth);

extern void SendAtrProcess(void);

extern BYTE ReceiveData(void);                                                                  //接收数据到RECDATA缓冲区中，最多个

extern void SendData(void);                                                                       //发送RECDATA缓冲区中的数据?

extern UWORD GetSpaceFileAddr(UWORD fileAddr);                                                                     

extern UWORD SearchFile(UWORD  fileAddr,WORD fileName);

extern UWORD SelectFile(UWORD  fileAddr,WORD fileName);

extern UWORD SelectFileByName(UWORD  fileAddr,BYTE * name,BYTE lgth);

extern BYTE  AccessCondition(UWORD fileAddr,BYTE action);

extern BYTE GetAcessPurview(UWORD fileAddr,BYTE action);

extern  BYTE WriteKey(BYTE keyId,BYTE * buf);

extern  BYTE ReadKey(BYTE keyId,BYTE * buf);                                                 //find the addr of the key 

extern void ReadPinStatus(void);                                                                           //把DF文件头信息拼凑成GSM标准的样子

//extern void FdnProgcess(void);
extern void FdnProgcess(BYTE mode);

extern BYTE  GetKeyRefenseStatus(BYTE keyRefense);

extern BYTE ConfigureKeyStatus(BYTE keyRefense,BYTE mode);

extern  BYTE GetPINRecordNum(BYTE keyRefense);
//----------------------	main.c	-------------------------------------------------------------
extern void PpsProcess(void);

extern void CommandAttemper(FLASH INSLIST *inslist);


//----------------------	gsmpublic.c	----------------------------------------------------------------------

extern BYTE GetEfInformation(void);

extern BYTE UsimGetEfInformation(void);

extern BYTE GetDfInformation(void);

extern BYTE UsimGetDfInformation(void);

extern BYTE SetChannel(UWORD fileAddr,WORD fileName,BYTE option);

extern BYTE Funtion134Write(void);

extern BYTE SmartLumingWrite(void);

extern void SmartLumingRead(void);

extern BYTE GetCurrentADFAid(WORD fileName,BYTE * buf);

extern UWORD GetFileHeaderEndAddr(void);
//----------------------	a3a8.c	-----------------------------------------------------------------
extern void RunA3A8_V1(BYTE keyId[16],BYTE  randNumber[16]);

extern void RUNA3A8_V2(BYTE keyId[16],BYTE randNumber[16]);

extern void RUNA3A8_V3(BYTE keyId[16],BYTE  randNumber[16]);
//extern void RunA3A8_V2(BYTE ki_key[16],BYTE  randNumber[16]);

//extern void RunA3A8_V3(BYTE ki_key[16],BYTE  randNumber[16]);

extern UWORD VMReadUword(UWORD srcAddr);

extern void VMWriteUword(UWORD destAddr, UWORD value);

extern BYTE VMReadByte(UWORD srcAddr);

/*
	standhard function of gsmFunction.c
*/

extern UWORD SelectByFileName(WORD fileName);

extern BYTE GetRecordNo(UWORD fileAddr,BYTE type,BYTE mode,BYTE record,BYTE currentRec);

extern BYTE ComputeRecord(BYTE action);
extern BYTE GetPhyNum(BYTE action,UWORD fileAddr,BYTE mode,BYTE accRecord,BYTE* currentRec);
extern BYTE ComputeBinary(BYTE action);

extern  void ReadBinary(void);

extern  void UpdateBinary(void);

extern  void SelectCommand(void);

//extern void RfmSelectCommand(void);

extern BYTE StatusPublic(void);

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

extern BYTE TerminalProfileCommon(void);
extern  void IncreaseCommand(void);

extern void RunAuth(void);

extern  void RunAlgorithm(void);

extern void RunGsmAlgorithm(void);

extern void RunGsmAlgorithmA3A8(void);

extern void RunGsmAlgorithmNEW(void);

extern void RunGsmAlgorithmV0V5(void); 

extern void RunGSMAlgorithmV3(void);

extern  void SeekCommand(void);

extern void TerminalProfile(void);

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

extern void UsimXORAuthentication(void);

extern  void UsimAuthentication(void);

extern BYTE fdnInvalidate(WORD dfId,WORD efID);

extern void SimFdnSYNCtoUSIM(BYTE fdnStatus);

extern void UsimFdnSYNCtoSIM(BYTE fdnStatus);

extern BYTE IsFdnServiceActiveInUSIM(void);

extern BYTE IsFdnServiceActiveInSIM(void);

extern void FileLifeCycleHandle(WORD dfName,WORD efName,BYTE action);

extern WORD FindCOSConfigTag(BYTE tag,BYTE * outByte);

//extern BYTE FindPrivateTag(BYTE tag,BYTE mode,BYTE * outBuf);

extern void GetCardInfoData(void);

extern void SetCardLifeCycle(void);

extern void SetCARDConfig(void);

extern void SetGetCOSCofigData(void);

extern void GetCardSpaceInfo(void);

//extern void SetCardSecureData(void);

extern UWORD VMHeapAlloc(UWORD size,WORD* index);

/*
    peso command of exteralcommand.c 
*/
//-------------	CreateFile.c	----------------------------------------------------
extern  void CreateFile(void);

extern void  DeleteFile(void);

extern void  ResizeFile(void);

//-------------	resume.c define	-------------------------------------------------------------------
extern void PersoReadEeprom(void);  

extern void PersoWriteEeprom(void);

extern void SetBank(void);

extern void CalCRC(void);

//------------------	exteralcommand.c	--------------------------------------------------------
#define	mode_ReloadMFKey	0x10
#define	mode_VerifyMFKey	0x08

extern void  EepormErase(void);

extern  void Initializtion(void);

extern void EraseFileSystem(void);

extern void UpdateFile(void);

extern void ReloadKey(void);

extern void ReloadVerifyManagenerKey(void);

extern void ChangeBaud(void);

extern WORD GetADFFID(BYTE *PartialAidBuf);

extern void ReloadVerifySuperKey(void);

extern void UnicomSetIMSI(void);
extern BYTE ComputeAll(BYTE action, BYTE nByShortFID,BYTE nfile);

extern void GetFileAcc(UWORD fileAddr,BYTE * buf);

extern BYTE Update6F7BFile_LD(WORD fileName);

extern UWORD GetBerTlvLength(BYTE * buffer,WORD * lenByte);

extern BYTE ReadLFFileValue1ByteLen(UWORD fileAddr,BYTE index,BYTE offset,BYTE * buf);

extern BYTE SetBerTlvLength(BYTE * buf,WORD totalLen);

extern BYTE ToolkitGetCPLLength(BYTE CPL1);
#endif
