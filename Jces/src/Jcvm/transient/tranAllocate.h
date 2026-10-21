#ifndef __TRAN_ALLOCATE_H__
#define __TRAN_ALLOCATE_H__

#include "target.h"
#include "profile.h"
#include "DataOperation.h"
#include "tranAllocate.h"

#if JCVM__MEMORY__TRAN_ALLO
	extern	u16 	tranAllocate_rtrAlloc(u16 size)									SMALL_MODE;
	extern	u16 	tranAllocate_dtrAlloc(u16 contextId, u16 size)					SMALL_MODE;
	extern	memref	tranAllocate_rtrAvailable(void);
#if JCVM__RAM__DTR__RTR__NEW	
	extern	memref	tranAllocate_dtrAvailable(u8 pkgId);
#else	
	extern	memref	tranAllocate_dtrAvailable(void);
#endif	
#else
	#define tranAllocate_rtrAlloc(size)												0
	#define tranAllocate_dtrAlloc(contextId, size)									0
	#define tranAllocate_rtrAvailable()												0
	#define tranAllocate_dtrAvailable()												0
#endif

#endif
