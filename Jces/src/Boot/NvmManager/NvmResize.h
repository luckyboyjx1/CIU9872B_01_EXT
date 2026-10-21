/*
 * NvmResize.h
 *
 *  Created on: 2016Äê6ÔÂ7ÈÕ
 *      Author: linguoming1
 */

#include <setjmp.h>
#include "DataOperation.h"
#include "bootConfigProfile.h"

#ifndef SRC_BOOT_NVMMANAGER_NVMRESIZE_H_
#define SRC_BOOT_NVMMANAGER_NVMRESIZE_H_

#if BOOT__MEMORY__NVM_RESIZE
	void NvmResize_Main(u16 entryIndex, PN8 bodyAddr, u32 reSize, u1 isExtend);
	void NvmResize_UpdateEtityHeader(void);
	void NvmResize_UpdateJcvmRelativeInfo(void);
#else
	#define NvmResize_Main(entryIndex, bodyAddr, reSize, isExtend)
	#define NvmResize_UpdateEtityHeader()
	#define NvmResize_UpdateJcvmRelativeInfo()
#endif

#endif /* SRC_BOOT_NVMMANAGER_NVMRESIZE_H_ */
