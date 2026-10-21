/************************************************************************************************************ 
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   VMAccessEeprom.h
 *
 * Description: 该文件 VM读写EEPROM操作，主要解决地址操作中，大小端的问题
 *
 * auther：     xxxx    xxxx.xx.xx     
 *
 * history：    V1.0
 *  
 ************************************************************************************************************/
 
#ifndef VMACCESSEEPROM_H
#define VMACCESSEEPROM_H

#include "typedef.h"

extern BYTE VMReadByte(UWORD srcAddr);

extern void VMReadBuf(BYTE *destRam, UWORD srcAddr, UWORD length);

extern BYTE VMWriteBuf(UWORD destAddr, BYTE *srcData, WORD length);

//extern void VMSetBuf(UWORD destAddr, BYTE hex, WORD length);

extern SWORD VMReadShort(UWORD srcAddr);

extern void VMWriteShort(UWORD destAddr, SWORD value);

extern SDWORD VMReadInt(UWORD srcAddr);

extern void VMWriteInt(UWORD destAddr, SDWORD value);

extern WORD VMReadRef(UWORD srcAddr);

extern UWORD VMReadUword(UWORD srcAddr);

extern void  VMWriteUword(UWORD destAddr, UWORD value);

extern void WriteUwordNoAtomic(UWORD destAddr, UWORD value);

extern UWORD VMReadThreeBytesAddr(UWORD srcAddr);

extern void VMWriteThreeBytesAddr(UWORD destAddr, UWORD value);

extern void WriteEepromThreeBytes(UWORD destAddr, UWORD value);

extern SBYTE NvmCmp(BYTE * dst,BYTE * src,WORD len);

extern SBYTE NvmWORDCmp(BYTE * dst,BYTE * src,WORD len);

extern SBYTE NvmDWORDCmp(BYTE * dst,BYTE * src,WORD len);

extern UWORD VMReadThreeBytesAddrCode(UWORD srcAddr);

#define VMWriteRef(destAddr, value)  VMWriteShort(destAddr, value);

#define VMReadJCVMByte(srcAddr)  (*((signed char *)(srcAddr)))

#define VMReadByteCode(srcAddr)  (*((unsigned char *)(srcAddr+EEPROM_BASE)))

#define VMReadBufCode              ReadEeprom

extern WORD VMReadRefCode(UWORD srcAddr);

extern SWORD VMReadShortCode(UWORD srcAddr);

extern SDWORD VMReadJCVMInt(UWORD srcAddr);

extern SWORD VMReadJCVMShort(UWORD srcAddr);

extern WORD VMReadJCVMRef(UWORD srcAddr);

#endif 

