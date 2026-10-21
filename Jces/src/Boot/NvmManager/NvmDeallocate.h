/*
 * NvmDeallocate.h
 *
 *  Created on: 2014Äê10ÔÂ28ÈÕ
 *      Author: linguoming1
 */

#ifndef __NVM_DEALLOCATE_H_
#define __NVM_DEALLOCATE_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"

#if BOOT__MEMORY__NVM_MANAGER_NVM_DEALLOCATE
	extern void   NvmDeallocate_Free(u16 entryIndex, u8 pckId);
	extern void   NvmDeallocate_Main(u1 isRollBack);
#else
	#define NvmDeallocate_Free(entryIndex, pckId)
	#define NvmDeallocate_Main(isRollBack)
#endif

#endif /* NVMDEALLOCATE_H_ */
