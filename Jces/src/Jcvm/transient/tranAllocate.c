#include "tranAllocate.h"
#include "tranManager.h"
#include "RamManager.h"
#include "JcvmRam.h"
#include "CgmManager.h"
#include "rtrSpace.h"



#if JCVM__MEMORY__TRAN_ALLO
u16 tranAllocate_rtrAlloc(u16 size)						SMALL_MODE
{
	u16		rtp;
	u16		block;
	u16		result;
	u16 	availableCount;
	u8   D_A	  spaceId;

	availableCount = U32t16(tranAllocate_rtrAvailable());

	if (size > availableCount)
	{
		rtp = NO_MEMORY_AVAILABLE;
	}
	else
	{

		rtp		= tranMananger_getRtp();
		block	= U32t16(U16_A_U16(rtp, size));

		spaceId  	= RtrSpace_searchExistRtrID();
		result = RamManager_reAlloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(spaceId)),block);


		if (result == NO_MEMORY_AVAILABLE)
		{
			rtp = NO_MEMORY_AVAILABLE;
		}
		else
		{
			tranMananger_setRtp(block);
		}
		if(U16t8(U16_AND_U16(cc, APPLET_MASK) < 0x80))
		{
			CgmManager_Alloc(U16t32(size), HAS82_MASK);
		}
	}

	return rtp;
}

u16 tranAllocate_dtrAlloc(u16 contextId, u16 size)		SMALL_MODE
{
	u16			D_A		dtrPointer;
	u16			D_A		maxdtp;
	u16			D_A		block;
	u8			D_A		ctxId;
	u16			D_A		result;
	u16			D_A		availableCount;

	ctxId			= GET_PKGID(contextId);
	dtrPointer		= tranMananger_getDtpByCtxId(ctxId);
	block 			= U32t16(U16_A_U16(dtrPointer, size));
#if JCVM__RAM__DTR__RTR__NEW
	availableCount = U32t16(tranAllocate_dtrAvailable(ctxId));
#else	
	availableCount = U32t16(tranAllocate_dtrAvailable());
#endif	
	if (size > availableCount)
	{
		dtrPointer = NO_MEMORY_AVAILABLE;
	}
	else
	{
		result = RamManager_reAlloc(MEMtPX8(dtrSpace_getDtrSpaceEntry(dtrSpace_getDtrSpaceId(selected_channel))), block);
		if (result == NO_MEMORY_AVAILABLE)
		{
			dtrPointer = NO_MEMORY_AVAILABLE;
		}
		else
		{
			tranMananger_setDtpByCtxId(ctxId, block);
			maxdtp			= tranMananger_getMaxDtp();
			if (block > maxdtp)
			{
				tranMananger_setMaxDtp(block);
			}
		}
	}

	return dtrPointer;
}

memref tranAllocate_rtrAvailable(void)
{
	return RamManager_ramOutAvailable();
}
#if JCVM__RAM__DTR__RTR__NEW
memref tranAllocate_dtrAvailable(u8 pkgId)
{
	memref ramAvailableSize;
	memref ramcgmAvailableSize;
	u16	   reserveDtrUesd;
	ramAvailableSize = RamManager_ramOutAvailable();
#if GP__CGM__FOR_MASK
	if (g_bCheckFlag != CHECK_CLOSE)
	{
		ramcgmAvailableSize = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE)- RamManager_getUsedDtrSize();
		if(ramcgmAvailableSize < ramAvailableSize)
		{
			ramAvailableSize = ramcgmAvailableSize;
		}
	}
#endif

	ramAvailableSize += MAX_DTR_RAM_RESERVE;

	reserveDtrUesd = 0;
	if(INVALID_SPACE_ID != dtrSpace_searchExistDtrCtx(U16_SHL_U8(U8t16(pkgId), 8)))
	{
		reserveDtrUesd = tranMananger_getDtpByCtxId(pkgId);
		if(reserveDtrUesd > MAX_DTR_RAM_RESERVE)
		{
			reserveDtrUesd = MAX_DTR_RAM_RESERVE;
		}
	}

	return (ramAvailableSize - reserveDtrUesd);
}
#else
memref tranAllocate_dtrAvailable(void)
{
	memref ramAvailableSize;
	memref ramcgmAvailableSize;
	ramAvailableSize = RamManager_ramOutAvailable();
#if GP__CGM__FOR_MASK
	if (g_bCheckFlag != CHECK_CLOSE)
	{
		ramcgmAvailableSize = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE) - RamManager_getUsedDtrSize();
		if(ramcgmAvailableSize < ramAvailableSize)
		{
			ramAvailableSize = ramcgmAvailableSize;
		}
	}
#endif
	return ramAvailableSize;
}
#endif
#endif
