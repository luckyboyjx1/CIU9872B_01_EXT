#ifndef   __MEM_GET_REUSE_RAM_H__
#define   __MEM_GET_REUSE_RAM_H__

#include "DataOperation.h"
#include "SysConfig.h"

#define	MemGetReuseRam_GetTempArrBuffer(bufSize)				sysConfig_GetTempArrBuffer(bufSize)
#define	MemGetReuseRam_GetTempArrFromTopBase(bufSize)			PNtMEM(&bTempArr[TEMP_ARR_BUF_SIZE - bufSize])

#define	MemGetReuseRam_GetEEpBuffer(bufSize)					sysConfig_GetEepromBuffer(bufSize)
	

//  let the addRess 4 byte align
//return U32_AND_U32(tempAddress, 0xFFFFFFFC);
#define	MemGetReuseRam_GetFromTopBase(bufSize)					PNtMEM(&bigReuseBuf[EEPROM_BUFFER_SIZE + TEMP_ARR_ALIGN_SIZE + TEMP_ARR_BUF_SIZE - bufSize])


//	not include eepRomBuffer head 4 byte, and let the addRess 4 byte align
//return U32_AND_U32(U32_A_U8(tempAddress, 0x07), 0xFFFFFFFC);
#define	MemGetReuseRam_GetFromBotBase(bufSize)					U32_AND_U32(U32_A_U8(&bigReuseBuf[0], 0x07), 0xFFFFFFFC)


#if !RAM_COMPACT_BUFFER_IN_LOCAL
#define MemGetReuseRam_GetRamCompactBuffer(bufSize)				sysConfig_GetRamCompactBuffer(bufSize)
#endif //!RAM_COMPACT_BUFFER_IN_LOCAL

#endif	/*	__MEM_GET_REUSE_RAM_H__		*/

