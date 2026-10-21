#ifndef _HARD_API_H_
#define _HARD_API_H_
#include "string.h"
#include "typedef.h"
#include "SysConfigTarget.h"


#define    LOADER_START_ADDR	  0x37C00
#define    EEPROM_SIZE      	  (FlashLimit-G_CODE_SIZE)
#define    BOOTLOARD_SIZE        0x1400                    	//flash最后0x1400空间为loader
#define    Bootloader_StartAddr  (EEPROM_SIZE-BOOTLOARD_SIZE)

#define     CRC16_TypeA   					0x6363
#define     CRC16_TypeB   					0xFFFF

#define		EepromError MemoryError

//同步TPX 5G代码（CDMA，强更新文件） by xuweihua@20210224
#define	    HE_FLASH_BASE		0x0011C000								//HE_FLASH start addr
#define	    HE_FLASH_SIZE		0x2000									//大小为8k

#define  GetPageOffset(x)         ((x)&HAL_EEPAGE_MASK)

#define D_EEPMoveTag		0xA5
#define D_EEPMoveTag_FAST	0xA6
#define D_EEPNoMoveTag		0x5A

#define D_EEMoveSize		0x80
#define D_MoveForward                   0x01
#define D_MoveBackward                  0x02 

//用于区别VMCopyDataToAddr函数的数据移动时是否需要进行保护
#define MODE_PROTECT_WRITE            1
#define MODE_NO_PROTECT_WRITE         0

#if 1
#define SecureWriteData_MaxLength           36  
#define SecureInitData_MaxLength              48  


#define SecureMoveEepromTag_Offset          0                                                                                                                       // 1 BYTE
#define SecureMoveEepromSrcAddr_Offset   (SecureMoveEepromTag_Offset +sizeof(BYTE))                                                     // 4 BYTE
#define SecureMoveEepromDesAddr_Offset   (SecureMoveEepromSrcAddr_Offset +sizeof(UWORD))                                          // 4 BYTE
#define SecureMoveEepromMoveLen_Offset   (SecureMoveEepromDesAddr_Offset +sizeof(UWORD))                                         // 4 BYTE

#define SecureMoveEepromLen_Offset   (SecureMoveEepromMoveLen_Offset + sizeof(UWORD))                                                // 1 BYTE
#define SecureMoveEepromDataBuf_Offset   (SecureMoveEepromLen_Offset +sizeof(BYTE))                                                     // D_EEMoveSize BYTE

#define SecureMoveEepromWriteDesAddr_Offset   (SecureMoveEepromDataBuf_Offset +D_EEMoveSize)                              // 4 BYTE
#define SecureMoveEepromWriteDataLen_Offset   (SecureMoveEepromWriteDesAddr_Offset + sizeof(UWORD))                             // 1 BYTE
#define SecureMoveEepromWriteDataBuf_Offset   (SecureMoveEepromWriteDataLen_Offset +sizeof(BYTE))                          // SecureWriteData_MaxLength BYTE

#define SecureMoveEepromInitDesAddr_Offset   (SecureMoveEepromWriteDataBuf_Offset + SecureWriteData_MaxLength)                              // 4 BYTE
#define SecureMoveEepromInitHeRecordNo_Offset   (SecureMoveEepromInitDesAddr_Offset + sizeof(UWORD))                               // 1 BYTE,记录HE 区的页号，只用于强更新文件
#define SecureMoveEepromInitStartOffset_Offset   (SecureMoveEepromInitHeRecordNo_Offset + sizeof(BYTE))                               // 2 BYTE
#define SecureMoveEepromInitDataLen_Offset   (SecureMoveEepromInitStartOffset_Offset + sizeof(WORD))         // 1 BYTE
#define SecureMoveEepromInitDataBuf_Offset   (SecureMoveEepromInitDataLen_Offset +sizeof(BYTE))                                    // SecureInitData_MaxLength BYTE

//#define SecureMoveEepromSetDesAddr_Offset   (SecureMoveEepromInitDataBuf_Offset + SecureInitData_MaxLength)                              // 4 BYTE
//#define SecureMoveEepromSetDataLen_Offset   (SecureMoveEepromSetDesAddr_Offset + sizeof(UWORD))                             // 2 BYTE
//#define SecureMoveEepromSetDataValue_Offset   (SecureMoveEepromSetDataLen_Offset +sizeof(WORD))                          // 1 BYTE


#define SecureMoveStuct_LengthPart              (1+4+4+4+1+D_EEMoveSize)
//#define SecureMoveStuct_LengthAll              (1+4+4+4+1+D_EEMoveSize+4+1+SecureWriteData_MaxLength+4+2+1+SecureInitData_MaxLength+4+2+1)  
#define SecureMoveStuct_LengthAll              (1+4+4+4+1+D_EEMoveSize+4+1+SecureWriteData_MaxLength+4+1+2+1+SecureInitData_MaxLength)  

#else
#define MoveEepAntiTeaingTag_Offset   (D_EEMoveSize+1)
#define MoveEepAntiTeaingSrc_Offset   (MoveEepAntiTeaingTag_Offset +sizeof(BYTE))
#define MoveEepAntiTeaingDes_Offset   (MoveEepAntiTeaingSrc_Offset +sizeof(UWORD))
#define MoveEepAntiTeaingLen_Offset   (MoveEepAntiTeaingDes_Offset +sizeof(UWORD))
#endif



//typedef struct
//{
//    const unsigned char *iDataPtr;
//    unsigned char       *oDataPtr;
//    const unsigned int  *rndPtr;
//    unsigned int        *priKeyPtr;
//    unsigned int        *pubKeyPtr;
//    const unsigned int  *argPtr;
//} ECC_Parameters;

//typedef struct
//{
//    const unsigned char *iDataPtr;
//    unsigned char       *oDataPtr;
//    const unsigned int  *rndPtr;
//    unsigned int        *priKeyPtr;
//    unsigned int        *pubKeyPtr;
//    const unsigned int  *argPtr;
//} ECKA_Parameters;

//extern BYTE SetEeprom(UWORD eepAddr, BYTE HEX,WORD length);
//extern BYTE WriteEeprom(UWORD LogEepAddr,BYTE * RAM_addr,WORD length);
extern void ReadEeprom(BYTE * RAM_addr,UWORD sourceAddr,UWORD length);
extern BYTE WriteEepromEX(UWORD eepAddr,BYTE * SourceAddr,WORD length);
extern BYTE SecureMoveEeprom(UWORD desAddr,UWORD srcAddr, UWORD dataLen);
extern BYTE SetEepromEX(UWORD eepAddr,BYTE Hex,WORD length);
extern BYTE ReadEECacheByte(UWORD logicStartAddr);
extern BYTE SetEepromAtomic(UWORD logDstEEPAddr,BYTE hex,WORD size);
extern BYTE WriteEepromAtomic(UWORD logDstEEPAddr,BYTE * RAMaddr,WORD size);

#define SetEeprom    SetEepromEX
#define WriteEeprom  WriteEepromEX

extern BYTE WriteEepromNonAtomic(UWORD logDstEEPAddr,BYTE * ramAddr,WORD size);
extern void VMReadBuf(BYTE * destRam, UWORD srcAddr, UWORD length);
#define ReadEEPromCache ReadEeprom

extern void VMReadBuf(BYTE *destRam, UWORD srcAddr, UWORD length);
#define CacheFlush  memRefresh
extern BYTE WriteEepromWithoutProtect(UWORD destAddr, BYTE *srcData, WORD length);
extern BYTE VMWriteBuf(UWORD destAddr, BYTE *srcData, WORD length);

extern void VMSetBuf(UWORD destAddr, BYTE hex, WORD length);
extern BYTE VMCopyDataToAddr(UWORD destAddr,WORD destOffset,WORD maxSize,UWORD srcAddr,WORD srcOffset,UWORD copyLen, BYTE mode,BYTE isDstFile,BYTE isSrcFile);
extern UWORD GetHeapBase(void);
extern void abortTransaction(void);
#define IsEepromAddr isInE2P

#define TransactionGetDepth   isInProgress
#define TransactionAbort      abortTransaction
//#define TransactionCommit     commitTransaction
//#define TransactionBegin      beginTransaction(0)

extern BYTE IF_BeginTransaction(void);
extern void IF_CommitTransaction(BYTE need_commit);
extern void IF_AbortTransaction(BYTE tag);

extern SBYTE NvmCmp(BYTE * dst,BYTE * src,WORD len);

extern BYTE VMWriteEepromOrRam(BYTE* destAddr, BYTE* srcAddr, WORD length);

extern SWORD VMReadShort(UWORD srcAddr);

extern BYTE WritePageCacheEX(UWORD logEEPAddr,BYTE * RAMaddr,WORD Length);

extern void ECCKeyAgreet_ProfileA(BYTE * HNPublicKey_smallendian, BYTE * USIMSKey_smallendian, BYTE * shareKey_smallendian);

extern void ECCKeyPair_ProfileA(BYTE * pkBuf_smallendian, BYTE * skBuf_smallendian);
extern void ECCKeyAgreet_ProfileB(BYTE * HNPublicKey, BYTE * USIMSKey, BYTE * shareKey);

extern BYTE ANS_X963_KDF_Hal(BYTE* Z,u8 ZLen, BYTE keydatalen,BYTE* sharedInfo, BYTE shardInfoLen, BYTE* K);
extern void AESEncrypt_Hal(BYTE* Key, BYTE* aIn,BYTE* aOut);
extern void HMACSha256DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest);
extern BYTE SetPageCacheEX(UWORD logEEPAddr,BYTE hex,WORD Length);
extern void SetEepromNonAtomic(UWORD logDstEEPAddr,BYTE Value,UWORD size);
extern void MoveEepromResume(void);
extern BYTE IF_BeginTransaction(void);
extern void StartSecureMoveEeprom(UWORD desAddr,UWORD srcAddr, UWORD dataLen);
extern void SetMovDateBufSrcDstDataLen(BYTE * movDateBuf,UWORD srcAddr,UWORD desAddr,UWORD dataLen);
extern void setChannelNum(BYTE cla);
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
extern BYTE WriteHEFlashPage(UWORD updateAddr,BYTE * RamBuffer);
extern WORD Hardware_CalCRC(BYTE * ptr, UWORD count);
#endif
extern BYTE IsUiccApp(BYTE  channelNum);
