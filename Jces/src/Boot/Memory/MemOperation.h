/*
 *
 *  Created on: 2014-2-17
 *      Author: yangfajun
 */
#ifndef __MEM_OPERATION_H__
#define	__MEM_OPERATION_H__

#include "DataOperation.h"
#include "MemOperationServe.h"
#include "MemNvmOperation.h"

#define EQUAL               0
#define GREATER             1
#define LESS                0xFF
#define WITH_MEMCOMPARE     1
#if BOOT__MEMORY__OPERATION
	/* memory 刷新
	 * 该函数用于强制提交EEPROM写缓存内的数据，
	 * 使memory物理地址上的数据是最新数据
	 * */
	extern  void memStreamClr(void);

#define memCopyToRam(dest, src, length)  EP_EepromToRam(dest, src, length)
#define memFillToRam(dest, value, length)  ecpMem_Set(dest, value, length)
#define memRefresh()  E2P_stream_flush()
//	extern  void memRefresh(void);

	/*
	 * ram array copy, eeprom to ram or ram to ram
	 * */
//	extern  void memCopyToRam(PX8 dest, PM8 src, u16 length);

	/*
	 * ram array fill,
	 * */
//	extern  void memFillToRam(PX8 dest, u8 value, u16 length);

	/*
	 *  nvm/code area read
	 * */
//	extern  u8  nvmReadU8(PN8 addr);
//	extern  u16 nvmReadU16(PN8 addr);
//	extern  u32 nvmReadU32(PN8 addr);
	#define	nvmReadU8(addr)	    memReadU8(addr)
	#define	nvmReadU16(addr)	memReadU16(addr)
	#define	nvmReadU32(addr)	memReadU32(addr)

	extern  void nvmWriteU8Atomic(PN8 dest, u8 value);
	extern  void nvmWriteU16Atomic(PN8 dest, u16 value);
	extern  void nvmWriteU32Atomic(PN8 dest, u32 value);
	extern  u16  nvmCopyAtomic(PN8 dest, PM8 src, u16 length);
	extern  void nvmFillAtomic(PN8 dest, u8 value, u16 length);

#define    nvmWriteU8NoAtomic(dest,value)          E2P_write_u8_getType(dest, value, WRITEEP_NOATOMIC)
#define    nvmWriteU16NoAtomic(dest,value)         E2P_write_u16_getType(dest, value, WRITEEP_NOATOMIC)         
#define    nvmWriteU32NoAtomic(dest,value)         E2P_write_u32_getType(dest, value, WRITEEP_NOATOMIC)         
#define    nvmCopyNoAtomic( dest,src,length)             E2P_write_array_getType(dest, src, length, WRITEEP_NOATOMIC) 
#define    nvmFillNoAtomic(dest,value,length)             E2P_fill_array_getType(dest, length, value, WRITEEP_NOATOMIC)

//	extern	void nvmWriteU8NoAtomic(PN8 dest, u8 value);
//	extern	void nvmWriteU16NoAtomic(PN8 dest, u16 value);
//	extern	void nvmWriteU32NoAtomic(PN8 dest, u32 value);
//	extern	void nvmCopyNoAtomic(PN8 dest, PM8 src, u16 length);
//	extern	void nvmFillNoAtomic(PN8 dest, u8 value, u16 length);

	#define nvmWriteU8Lost( dest,  value)                 	E2P_write_u8_getType(dest, value, WRITEEP_LOST)         
	#define nvmWriteU16Lost( dest,  value)                 	E2P_write_u16_getType(dest, value, WRITEEP_LOST)        
	#define nvmWriteU32Lost( dest,  value)              	E2P_write_u32_getType(dest, value, WRITEEP_LOST)        
	#define nvmCopyLost( dest,  src,  length)           	E2P_write_array_getType(dest, src, length, WRITEEP_LOST)
	#define nvmFillLost( dest,  value,  length)         	E2P_fill_array_getType(dest, length, value, WRITEEP_LOST)
//	extern	void nvmWriteU8Lost(PN8 dest, u8 value);
//	extern	void nvmWriteU16Lost(PN8 dest, u16 value);
//	extern	void nvmWriteU32Lost(PN8 dest, u32 value);
//	extern	void nvmCopyLost(PN8 dest, PM8 src, u16 length);
//	extern	void nvmFillLost(PN8 dest, u8 value, u16 length);

	/*
	 *  nvm/ram/code area read
	 * */
	extern	u8  memReadU8(PM8 addr);
	extern	u16 memReadU16(PM8 addr);
	extern	u32 memReadU32(PM8 addr);
	extern	u8  memCompare(PM8 addr1, PM8 addr2, u16 length);
	extern  u8  memCompareEqual(PM8 addr1, PM8 addr2, u16 length);
	extern  u8  memCompareSigned(PM8 addr1, PM8 addr2, u16 length);
	extern	u8	memCompareWithStatus(PM8 addr1, PM8 addr2, u16 length, u8 statusOK, u8 statusFail);
    extern  u8 memCompareU16(PM8 addr1, PM8 addr2, u16 length);
	extern  u8 memCompareU32(PM8 addr1, PM8 addr2, u16 length);
	extern	void memWriteU8Atomic(PM8 dest, u8 value);
	extern	void memWriteU16Atomic(PM8 dest, u16 value);
	extern	void memWriteU32Atomic(PM8 dest, u32 value);
	extern	u16  memCopyAtomic(PM8 dest, PM8 src, u16 length);
	extern	void memFillAtomic(PM8 dest, u8 value, u16 length);

	extern	void memWriteU8NoAtomic(PM8 dest, u8 value);
	extern	void memWriteU16NoAtomic(PM8 dest, u16 value);
	extern	void memWriteU32NoAtomic(PM8 dest, u32 value);
	extern	u16  memCopyNoAtomic(PM8 dest, PM8 src, u16 length);
	extern	void memFillNoAtomic(PM8 dest, u8 value, u16 length);

	extern	void memWriteU8Lost(PM8 dest, u8 value);
	extern	void memWriteU16Lost(PM8 dest, u16 value);
	extern	void memWriteU32Lost(PM8 dest, u32 value);
	extern	void memCopyLost(PM8 dest, PM8 src, u16 length);
	extern	void memFillLost(PM8 dest, u8 value, u16 length);

	extern	void writeEeprom(PN8 dest, PM8 src, u16 length);
	#define fillEeprom(dest, value, length) MemNvmOperation_Fill(dest, value, length)
//	extern	void fillEeprom(PN8 dest, u8 value, u16 length);

		#define	 fillEepromAccordingToPlatform(dest, value, length)  platformfillEeprom(dest, value, length)

//		#define  fillEepromAccordingToPlatform(dest, value, length) MemNvmOperation_Fill(dest, value, length)
	extern	void nvmCopyNoCache(PN8 dest, PM8 src, u16 length);
	extern	void nvmFillNoCache(PN8 dest, u8 value, u16 length);
	extern	void nvmClearNoCache(PN8 dest, u16 length);
#else
	#define	 memStreamClr()
	#define  memRefresh()
	#define  memCopyToRam(dest, src, length)
	#define  memFillToRam(dest, value, length)
	#define  nvmReadU8(addr)												0
	#define  nvmReadU16(addr)												0
	#define  nvmReadU32(addr)												0
	#define  nvmWriteU8Atomic(dest, value)
	#define  nvmWriteU16Atomic(dest, value)
	#define  nvmWriteU32Atomic(dest, value)
	#define  nvmCopyAtomic(dest, src, length)
	#define  nvmFillAtomic(dest, value, length)
	#define	 nvmWriteU8NoAtomic(dest, value)
	#define	 nvmWriteU16NoAtomic(dest, value)
	#define	 nvmWriteU32NoAtomic(dest, value)
	#define	 nvmCopyNoAtomic(dest, src, length)
	#define	 nvmFillNoAtomic(dest, value, length)
	#define	 nvmWriteU8Lost(dest, value)
	#define	 nvmWriteU16Lost(dest, value)
	#define	 nvmWriteU32Lost(dest, value)
	#define	 nvmCopyLost(dest, src, length)
	#define	 nvmFillLost(dest, value, length)
	#define	 memReadU8(addr)												0
	#define	 memReadU16(addr)												0
	#define	 memReadU32(addr)												0
	#define	 memCompare(addr1, addr2, length)								0
	#define	 memWriteU8Atomic(dest, value)
	#define	 memWriteU16Atomic(dest, value)
	#define	 memWriteU32Atomic(dest, value)
	#define	 memCopyAtomic(dest, src, length)
	#define	 memFillAtomic(dest, value, length)
	#define	 memWriteU8NoAtomic(dest, value)
	#define	 memWriteU16NoAtomic(dest, value)
	#define	 memWriteU32NoAtomic(dest, value)
	#define	 memCopyNoAtomic(dest, src, length)
	#define	 memFillNoAtomic(dest, value, length)
	#define	 memWriteU8Lost(dest, value)
	#define	 memWriteU16Lost(dest, value)
	#define	 memWriteU32Lost(dest, value)
	#define	 memCopyLost(dest, src, length)
	#define	 memFillLost(dest, value, length)
	#define	 writeEeprom(dest, src, length)
	#define	 fillEeprom(dest, value, length)
	#define	 fillEepromAccordingToPlatform(dest, value, length)
	#define	 nvmCopyNoCache(dest, src, length)
	#define	 nvmFillNoCache(dest, value, length)
	#define	 nvmClearNoCache(dest, length);
#endif

#endif //__MEM_OPERATION_H__


