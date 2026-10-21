#ifndef    _error_h_
#define    _error_h_
#include   "typedef.h"
//#define	SecurityError	SecuritySatisfied
extern void SecurityError(void);

extern  void NoInforStateOfMemoryUnchanged(void);

extern  void ChannelNotSupport(void);

extern  void SecureNotSupport(void);

extern  void RepeatPreviousCmdHeader(BYTE length);

extern  void WarningNoInformationGiven(void);

extern  void RecordNotFound(void);

extern  void NoEfSelected(void);

extern  void P3InconsistentWithP1P2(void);

extern  void FunctionNotSupport(void);

extern  void ReferencedDataInvalid(void);

extern  void ReferencedDataNotFound(void);

extern  void PinBlocked(void);

extern  void PinRetryCounter(BYTE retryLeft);

extern  void FileNotFound(void);

extern  void SelectedFileInvalid(void);

extern  void MemoryError(void);

extern 	void P3Error(void);

extern  void P3ReturnRightLength2G(BYTE RightLength);

extern  void P1P2Error(void);

extern  void InsError(void);

extern  void ClaError(void);

extern  void P1P2Incorrect(void);

extern  void FileStructureError(void);

extern  void SecuritySatisfied(void);

extern  void GSMSecurityContextNotSupported(void);

extern  void IncorrentParameter(void);

extern  void NoEnoughSpace(void);

extern  void  TechnicalProblem(void);

extern  void  SimAppTkitBusy(void);

extern 	void  ConditionSatisfied(void);

extern  void  AddressInvilid(void);

extern  void  FileExist(void);

extern  void  DfNameExist(void);

extern void   ChvContradiction(void);

extern void   InvalidationContradiction(void);

extern void   MaxReached(void);

extern void  AuthenticateError(void);

//extern BYTE SecurityFileValid( BYTE nFileStatus);

extern void  CommandFinish(BYTE  length);

extern void  SendResponse(void);

extern BYTE P1P2Check(void);

extern BYTE P1P2P3Check(BYTE value,BYTE nCondition);


/******************CDMA error************************/
extern void FileNoFind(void);

extern void FileInconsistent(void);

extern void CDMAError(void);

extern void KeyCheckSumNotValid(void);

#endif
