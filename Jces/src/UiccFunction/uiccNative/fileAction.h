#ifndef    _fileAction_h_
#define    _fileAction_h_
#include	"uicc_main.h"
#include	"typedef.h"
//#include	"resume.h"

#define DF_Depth         0x04    


#define  FileNum          F_RecNum_Offset
#define  FileLgth   	  F_RecLgth_Offset

#define  SelectTFFile 		SelectLFFile    
#define  GetLFRecNumLgth 	GetFileHeadByte

//*********EF_ToolkitApp文件的偏移定义***********
#define EventList_Length     7
#define AppletStatus_OFFSET                 0                                
#define AppId_OFFSET                       (AppletStatus_OFFSET + 1)   
#define TrigerMode_OFFSET                  (AppId_OFFSET + 1)  //使用APPID获取对应数据，比如AID data时，要注意如果在installforinstall中调用了SIMAPI此时应用并未写到注册表中，注册表中的信息是获取不到的
#define EventList_OFFSET          		   (TrigerMode_OFFSET +1)
#define MaxTimerCount_OFFSET               (EventList_OFFSET + EventList_Length)   

extern UWORD  GetNVMAvailMemory(void);
extern UWORD getFileContentAddr(UWORD  fileAddr);

extern BYTE ClearAddrIndex(UWORD  DFAddr);

extern WORD FindFreeIndexFromAddrIndexTable(void);

extern BYTE getCycleFPoint(UWORD fileAddr);

extern UWORD GetFileHeadUsedSize(UWORD fileAddr);

extern BYTE CompareExpandValueInFile(BYTE * ptrToResult);

extern BYTE ActionOfBit(BYTE bitPos);

extern BYTE SecurityConditionByte(BYTE scByte);

extern BYTE CompactToExpand(BYTE * ptrToLen,BYTE * resultBuf);

extern BYTE FindArrNum(BYTE *ptrToValue,BYTE pos);

extern BYTE InitPattern(BYTE * ptrToLen,BYTE * pos);

extern void InitTransFile(UWORD fileBodyAddr,BYTE * ptrToLen,WORD bodySize,BYTE updatetag);

extern void InitRecordFile(UWORD fileBodyAddr,BYTE * ptrToLen,BYTE recNum, BYTE recLen);

extern UWORD CreateDFFile(UWORD fileUseSize,WORD fileName);

extern UWORD CreateDFFileSECURE(UWORD fileUseSize,WORD fileName);

extern UWORD CreateEFFile(UWORD fileUseSize,WORD fileName);

extern UWORD CreateEFFileSECURE(UWORD fileUseSize,WORD fileName);

extern UWORD SecureDataStore(WORD efName,WORD* index);

extern UWORD VMHeapAlloc(UWORD size,WORD* index);

extern BYTE GetFileHeadByte(UWORD fileAddr,BYTE operationType);

extern UWORD GetFileHeadUword(UWORD fileAddr,BYTE operationType);

extern WORD GetFileHeadWord(UWORD fileAddr,BYTE operationType);

extern UWORD FindTFAddr(UWORD fileAddr, WORD offset, BYTE lgth);
	
extern UWORD FindLFAddr(UWORD fileAddr, BYTE index, BYTE offset, BYTE len);

extern BYTE SetLFConst(UWORD fileAddr, BYTE index, BYTE offset, BYTE  dataContext,BYTE lgth);

extern BYTE WriteLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len);

extern BYTE ReadLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len);

extern BYTE SetTFConst(UWORD fileAddr, WORD offset, BYTE ch,BYTE lgth);

extern BYTE WriteTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len);

extern BYTE ReadTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, BYTE len);

extern BYTE MoveTFContent(UWORD fileAddr, WORD srcOffset, WORD desOffset, WORD len);

extern WORD GetTFSize(UWORD fileAddr);

extern UWORD SelectLFFile(WORD dfName,WORD efName);

extern UWORD GetParentOrMapFileAddr (UWORD fileAddr,BYTE offset);

extern BYTE modifyGrandParentSize2(SDWORD headSize,SDWORD contentsize,BYTE mode);

extern BYTE FindExtandArr(BYTE *ptrToValue,BYTE pos);

extern WORD ReadLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf);

extern BYTE WriteLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf,WORD length);

extern WORD ReadTFFileWord(UWORD fileAddr);

extern void WriteTFFileWord(UWORD fileAddr,WORD len);

extern BYTE AddOrDeleteToolkitAppRecord(BYTE recordNo);

#define  FileRecIndicator 0x02
#define  FileTypeMode     0x03
#define  FileStatusMode	  0x04

#define  FileNum          F_RecNum_Offset
#define  FileLgth   	  F_RecLgth_Offset

#define  SelectTFFile 		SelectLFFile 

#define  GetLFHead    GetLFRecNumLgth   

#define  GetLFRecNumLgth 	GetFileHeadByte
#define  WriteLFBufForJavaAccess 		WriteLFBuf
#define  WriteTFBufForJavaAccess 		WriteTFBuf
#define  ReadLFBufForJavaAccess         ReadLFBuf
#define  ReadTFBufForJavaAccess         ReadTFBuf
#define  SetLFConstForJavaAccess        SetLFConst
#define  SetTFConstForJavaAccess        SetTFConst
	

#if NATIVE_CARD_UICC
#define  SelectPrivateLFFile       SelectLFFile    
#else
extern UWORD SelectPrivateLFFile(WORD dfName, WORD efName);
#endif

#define  SelectPrivateTFFile 		SelectPrivateLFFile 
#define ReadToolkitAppData          ReadLFBuf
#endif
