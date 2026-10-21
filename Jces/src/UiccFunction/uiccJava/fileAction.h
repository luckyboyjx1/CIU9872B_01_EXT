#ifndef    _fileAction_h_
#define    _fileAction_h_
#include	"uicc_main.h"
#include	"typedef.h"

#define  FileChangeTimes   100
#define  MaxUseLen  0x80

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
extern BYTE IF_IsHeFlashFile(UWORD addr);
extern BYTE IF_UpdateCycleRecPointer(UWORD addr);
extern UWORD GetHeFlashFileDataAddr(UWORD addr);
extern BYTE GetHeFlashUsableRecordNo(BYTE fileNo);
extern BYTE GetHeFlashRecordNoByFileId(BYTE fileNo,WORD fileId);
extern BYTE UpdateFileInHeFlash(UWORD fileAddr,BYTE para1,BYTE para2,BYTE hex,BYTE * buf,BYTE len);
#endif

extern BYTE GetFileHeadByte(UWORD fileAddr,BYTE operationType);

extern UWORD GetFileHeadUsedSize(UWORD fileAddr);

extern WORD GetFileHeadWord(UWORD fileAddr,BYTE operationType);

extern UWORD GetFileTotalSize(UWORD fileAddr);

extern WORD ReadWORD(UWORD addr);

extern UWORD FindLFAddr(UWORD fileAddr, BYTE index, BYTE offset, BYTE len);

extern UWORD FindTFAddr(UWORD fileAddr, WORD offset, WORD lgth);
	
extern BYTE SetLFConst(UWORD fileAddr, BYTE index, BYTE offset, BYTE  dataContext,BYTE lgth);

extern BYTE WriteLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len);

extern BYTE ReadLFBuf(UWORD fileAddr, BYTE index, BYTE offset, BYTE * buf, BYTE len);

extern BYTE SetTFConst(UWORD fileAddr, WORD offset, BYTE ch,WORD lgth);

extern BYTE WriteTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, WORD len);

extern BYTE ReadTFBuf(UWORD fileAddr, WORD offset, BYTE * buf, WORD len);

extern WORD GetTFSize(UWORD fileAddr);

extern UWORD SelectLFFile(WORD dfName, WORD efName);

extern UWORD GetParentOrMapFileAddr (UWORD fileAddr,BYTE offset);

extern BYTE WriteTFFileNonAtomic(UWORD fileAddr, WORD offset, UWORD srcAddr, WORD len);

extern UWORD SelectPrivateLFFile(WORD dfName, WORD efName);

extern UWORD SelectLFFileWithMfStart(UWORD mfStart,WORD dfName, WORD efName);

extern UWORD SelectProcessingProfileLFFile(WORD dfName, WORD efName);

extern UWORD GetFileHeadUsedSize(UWORD fileAddr);

extern void ClearFileConst(UWORD fileAddr,BYTE hex);

extern UWORD GetFirstLevelDf(UWORD currentDfAddr);

extern UWORD GetLastDFAddr(void);

extern BYTE getCycleFPoint(UWORD fileAddr);
/**
* added by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                             用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
extern BYTE AddOrDeleteToolkitAppRecord(BYTE recordNo);

extern UWORD GetLastDFAddr(void);

extern UWORD getCycleFPointAddr(UWORD fileAddr);

extern UWORD SelectPrivateFile(UWORD  fileAddr,WORD fileName);

#define  FileRecIndicator 0x02
#define  FileTypeMode     0x03
#define  FileStatusMode	  0x04

#define  FileNum          F_RecNum_Offset
#define  FileLgth   	  F_RecLgth_Offset

#define  SelectTFFile 		SelectLFFile 

#define  GetLFHead    GetLFRecNumLgth   

#define  GetLFRecNumLgth 	GetFileHeadByte

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
#define  WriteLFBufForJavaAccess(dstAddr,recNum,off,src,len) 		memCopyAtomic(MEMtPN8(dstAddr+off),src,len)
#define  WriteTFBufForJavaAccess(dstAddr,  off, src,  len) 			memCopyAtomic(MEMtPN8(dstAddr+off),src,len)
#define  ReadLFBufForJavaAccess(srcAddr,recNum,off,dst,len)         memCopyAtomic(dst,MEMtPN8(srcAddr+off),len)
#define  ReadTFBufForJavaAccess(srcAddr,  off, dst,  len)           memCopyAtomic(dst,MEMtPN8(srcAddr+off),len)
#define  SetLFConstForJavaAccess(dstAddr, recNum,off,value,len)     memFillAtomic(MEMtPN8(dstAddr+off),value,len)
#define  SetTFConstForJavaAccess(dstAddr, off, value,len)           memFillAtomic(MEMtPN8(dstAddr+off),value,len)

#else
#define  WriteLFBufForJavaAccess 		WriteLFBuf
#define  WriteTFBufForJavaAccess 		WriteTFBuf
#define  ReadLFBufForJavaAccess         ReadLFBuf
#define  ReadTFBufForJavaAccess         ReadTFBuf
#define  SetLFConstForJavaAccess        SetLFConst
#define  SetTFConstForJavaAccess        SetTFConst
#endif

#define  SelectPrivateTFFile 		SelectPrivateLFFile 
//#define ReadToolkitAppData          ReadLFBuf
#endif
