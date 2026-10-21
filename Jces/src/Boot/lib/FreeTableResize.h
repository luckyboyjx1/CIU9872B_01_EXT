/*
 * FreeTableResize.h
 *
 *  Created on: 2016Äê6ÔÂ7ÈÕ
 *      Author: linguoming1
 */

#ifndef SRC_BOOT_LIB_FREETABLERESIZE_H_
#define SRC_BOOT_LIB_FREETABLERESIZE_H_

#include <setjmp.h>
#include "DataOperation.h"
#include "bootConfigProfile.h"

#if BOOT__MEMORY__NVM_RESIZE
	extern u32  FreeTableResize_GetChangeSize(PN8 entityStartAddr, PN8 entityEndAddr, u16 freeTableEntryNum, u1 isExtend);
	extern u32  FreeTableResize_AnalyseTblForExtend(u16 freeTableEntryNum);
	extern void FreeTableResize_Main(PM8 blockStart, u32 blockSize, u1 isExtend);
#else
	#define FreeTableResize_GetChangeSize(entityStartAddr, entityEndAddr, freeTableEntryNum, isExtend)		0
	#define FreeTableResize_AnalyseTblForExtend(freeTableEntryNum)											0
	#define FreeTableResize_Main(blockStart, blockSize, isExtend)				
#endif

#endif /* SRC_BOOT_LIB_FREETABLERESIZE_H_ */
