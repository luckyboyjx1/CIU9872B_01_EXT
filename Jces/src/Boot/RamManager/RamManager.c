#include "RamManager.h"
#include "DataOperation.h"
#include "error.h"
#include "BootOS.h"
#include "Boot.h"
#include "BootNoAreaInitRam.h"
#include "rtrSpace.h"
#include "tranManager.h"

#if BOOT__MEMORY__RAM_MANAGER

STATIC u16 gRamMgr_usedRamPtr;
STATIC u16 gRamMgr_autoReleaseNum;

void RamManager_initVal(void)
{
	gRamMgr_autoReleaseNum 	= 0;
}
#if JCVM__RAM__DTR__RTR__NEW
u16 RamManager_calReserveDtrUesdAmount()
{
	u16     D_A			usedRamPtr;
	u16 	D_A 		blockHeader;
	u16 	D_A 		blockLen;
	u16		D_A			ReserveDtrUesdAmount;

	ReserveDtrUesdAmount	= 0;
	blockHeader				= U32t16(BOOT_RAM_BASE);
	usedRamPtr				= gRamMgr_usedRamPtr;

	while(blockHeader < usedRamPtr)
	{
		if(INVALID_ALLOC_RAM_ADDRESS != MEMtPX8(RamManager_getAllocBlockHeaderStart(blockHeader)))
		{
			blockLen				= RamManager_getAllocBlockHeaderLen(blockHeader);
			//  此处只考虑一条RTR逻辑通道，对于EUICC产品此处还需要修改。
			if(blockHeader == BOOT_RAM_BASE)
			{
				blockHeader 			= U32t16(U16_A_U16(blockHeader, blockLen));
			}
			else
			{
				blockHeader 			= U32t16(U16_A_U16(blockHeader, blockLen));
				blockLen				-= ALLOC_BLOCK_HEADER_SIZE;
				if(blockLen > MAX_DTR_RAM_RESERVE)
				{
					blockLen	= MAX_DTR_RAM_RESERVE;
				}
				ReserveDtrUesdAmount	+= blockLen;
			}
		}
		else
		{
			break;
		}
	}

	return ReserveDtrUesdAmount;
}
#endif
u8 RamManager_calRamBlocksAmount()
{
	u16     D_A			usedRamPtr;
	u16 	D_A 		blockHeader;
	u16 	D_A 		blockLen;
	u8		D_A			blockN;

	blockN					= 0;
	blockHeader				= U32t16(BOOT_RAM_BASE);
	usedRamPtr				= gRamMgr_usedRamPtr;

	while(blockHeader < usedRamPtr)
	{
		if(INVALID_ALLOC_RAM_ADDRESS != MEMtPX8(RamManager_getAllocBlockHeaderStart(blockHeader)))
		{
			U8_INC(blockN);
		}
		else
		{
			break;
		}
		blockLen			= RamManager_getAllocBlockHeaderLen(blockHeader);
		blockHeader 		= U32t16(U16_A_U16(blockHeader, blockLen));
	}

	return blockN;
}

void RamManager_init(void)
{
	u32 bootRamBase;
	
	gRamMgr_autoReleaseNum 	= 0;
	bootRamBase 			= BOOT_RAM_BASE;
	gRamMgr_usedRamPtr 		= U32t16(bootRamBase);
#if JCVM__RAM__DTR__RTR__NEW	
	ecpMem_Set(MEMtPX8(RAMBase), 0x00, RAMSize);
#else
	ecpMem_Set(MEMtPX8(RAMBase), 0x00, Transient_SIZE);
#endif	
}

memref RamManager_getRamBase(void)
{
	return RAMBase;
}

u16 RamManager_refHasBeenAllocated(PX8 ref)
{
	u16 	D_A 		blockHeader;
	u16 	D_A 		blockLen;
	
	blockHeader				= U32t16(BOOT_RAM_BASE);

	while(blockHeader < gRamMgr_usedRamPtr)			//遍历
	{
		if(ref == MEMtPX8(RamManager_getAllocBlockHeaderStart(blockHeader))) //已经alloc过了
		{
			return blockHeader;
		}
		blockLen			= RamManager_getAllocBlockHeaderLen(blockHeader);
		blockHeader 		= U32t16(U16_A_U16(blockHeader, blockLen));
	}
	
	return INVALID_ALLOC_RAM_ADDRESS;
}

/**
 * initSize = Transient_SIZE-(chn+1)*7
 *
 * ramAvailable
 *	= initSize - allocLen(content length only)
 *	= initSize - (gRamMgr_usedRamPtr - BOOT_RAM_BASE - n * 7)
 *	= Transient_SIZE - (chn+1)*7 - gRamMgr_usedRamPtr + n * 7  + BOOT_RAM_BASE
 *	= Transient_SIZE + BOOT_RAM_BASE  - gRamMgr_usedRamPtr ¨C (chn + 1 - n)*7
 */
u16 RamManager_ramOutAvailable(void)
{
	u8 	blockN;
	u16 ramAvailable;
	u16	temp;

	blockN = RamManager_calRamBlocksAmount();
	temp   = U8_A_U8(MAX_LOGICAL_CHANNELS, MAX_RTR_SPACE_CHANNELS);
	temp   = U16_S_U8(temp, blockN);
	temp   = U32t16(U8_M_U16(ALLOC_BLOCK_HEADER_SIZE, temp));
#if JCVM__RAM__DTR__RTR__NEW
	ramAvailable = U32t16(U16_A_U16(Transient_SIZE, BOOT_RAM_BASE));
	ramAvailable = U16_A_U16(ramAvailable, RamManager_calReserveDtrUesdAmount());
	ramAvailable = U16_S_U16(ramAvailable, gRamMgr_usedRamPtr);
	if(ramAvailable < temp)
	{
		throw_error(ALLOC_RAM_SYSTEM_ERROR);
	}
#else	
	ramAvailable = U32t16(U16_A_U16(Transient_SIZE, BOOT_RAM_BASE));
	ramAvailable = U16_S_U16(ramAvailable, gRamMgr_usedRamPtr);
#endif	
	ramAvailable = U16_S_U16(ramAvailable, temp);

	return ramAvailable;
}

u16 RamManager_ramAvailable(void)
{
	u16 ramAvailable;
	u32 bootRamBase;
	
	bootRamBase 			= BOOT_RAM_BASE;
#if JCVM__RAM__DTR__RTR__NEW	
	ramAvailable 			= U16_S_U16(RAMSize, U16_S_U16(gRamMgr_usedRamPtr, bootRamBase));
#else	
	ramAvailable 			= U16_S_U16(Transient_SIZE, U16_S_U16(gRamMgr_usedRamPtr, bootRamBase));
#endif	
	return ramAvailable;
}

u16 RamManager_alloc(PX8 ref, u16 size, u8 isAutoRelease)
{
	u16 result;

	result	= NO_MEMORY_AVAILABLE;

	//"reAlloc" should not be limit by ramAvailable check-up below;
	if(INVALID_ALLOC_RAM_ADDRESS != RamManager_refHasBeenAllocated(ref))
	{
		result = RamManager_reAlloc(ref, size);
		if (result != NO_MEMORY_AVAILABLE)
		{
			result	= ALLOC_RAM_REF_HAS_BEEN_ALLOCATED;
		}
	}
	else
	{
#if JCVM__RAM__DTR__RTR__NEW		
		if( (RamManager_ramOutAvailable() + MAX_DTR_RAM_RESERVE) >= size)
#else			
		size			= U32t16(U16_A_U8(size, ALLOC_BLOCK_HEADER_SIZE));
		if(RamManager_ramAvailable() >= size)
#endif		
		{
#if JCVM__RAM__DTR__RTR__NEW			
			size			= U32t16(U16_A_U8(size, ALLOC_BLOCK_HEADER_SIZE));
#endif			
			RamManager_setAllocBlockHeaderStart(gRamMgr_usedRamPtr, PXtMEM(ref));
			RamManager_setAllocBlockHeaderLen(gRamMgr_usedRamPtr, size);
			if(true == isAutoRelease)
			{
				RamManager_setAllocBlockHeaderIsAutoRelease(gRamMgr_usedRamPtr, isAutoRelease);
				U16_INC(gRamMgr_autoReleaseNum);
			}

			PX_W_REF(PMtMEM(ref), PXtMEM(RamManager_getAlloRamBase(U16_A_U8(gRamMgr_usedRamPtr, ALLOC_BLOCK_HEADER_SIZE))));
			gRamMgr_usedRamPtr 	= U32t16(U16_A_U16(gRamMgr_usedRamPtr, size));

			result	= BOOT_NO_ERROR;
		}
		else
		{
			result	= NO_MEMORY_AVAILABLE;
		}
	}

	return result;
}

void RamManager_updatePointer(u16 tempPointer)
{
	u16 	D_A			tempBlockLen;
	memref  D_A			p_head;
	
	while(tempPointer < gRamMgr_usedRamPtr)
	{
		p_head	= RamManager_getAllocBlockHeaderStart(tempPointer);
		if(INVALID_ALLOC_RAM_ADDRESS == p_head)
		{
			break;
		}
		PX_W_REF(p_head, PXtMEM(RamManager_getAlloRamBase(U16_A_U8(tempPointer, ALLOC_BLOCK_HEADER_SIZE))));
		tempBlockLen = RamManager_getAllocBlockHeaderLen(tempPointer);
		
		tempPointer = U32t16(U16_A_U16(tempPointer, tempBlockLen));
	}
}

void RamManager_free(PX8 ref)
{
	u16 	D_A 		blockOffset;
	u16 	D_A  		blockLen;
	u8		D_A			isAutoRelease;
	
	blockOffset  	= RamManager_refHasBeenAllocated(ref);
	if(INVALID_ALLOC_RAM_ADDRESS == blockOffset)
	{
		throw_error(ALLOC_RAM_REF_NOT_FOUND);
	}
	
	blockLen		= RamManager_getAllocBlockHeaderLen(blockOffset);
	isAutoRelease 	= RamManager_getAllocBlockHeaderIsAutoRelease(blockOffset);
	if(true == isAutoRelease)
	{
		U16_DEC(gRamMgr_autoReleaseNum);
	}
	
	if(gRamMgr_usedRamPtr != U32t16(U16_A_U16(blockOffset, blockLen)))
	{
		ecpMem_Copy(RamManager_getAlloRamBase(blockOffset), 
					RamManager_getAlloRamBase(U16_A_U16(blockOffset, blockLen)),
					U16_S_U16(gRamMgr_usedRamPtr, U16_A_U16(blockOffset, blockLen)));
	}
	
	gRamMgr_usedRamPtr = U16_S_U16(gRamMgr_usedRamPtr, blockLen);
	ecpMem_Set(MEMtPX8(RamManager_getAlloRamBase(gRamMgr_usedRamPtr)), 0x00, blockLen);
	RamManager_updatePointer(blockOffset);
	
	//将ref置空, 32字节指针,移植注意, 这里置成0xFFFFFFFF更好
	PX_W_REF(ref, INVALID_ALLOC_RAM_ADDRESS);
}

u16 RamManager_reAlloc(PX8 ref, u16 size)
{
	u16 	D_A 		blockOffset;
	u16 	D_A  		blockLen;
	u16 	D_A  		dataLen;
	u16		D_A			AvailableSize;
	u16		D_A			differSize;

	blockOffset  	= RamManager_refHasBeenAllocated(ref);
	if(INVALID_ALLOC_RAM_ADDRESS == blockOffset)
	{
		throw_error(ALLOC_RAM_REF_NOT_FOUND); 
	}
	blockLen		= RamManager_getAllocBlockHeaderLen(blockOffset);
	dataLen			= U16_S_U8(blockLen, ALLOC_BLOCK_HEADER_SIZE);
	if(size > dataLen)
	{
		differSize		= U16_S_U16(size, dataLen);
		AvailableSize	= RamManager_ramAvailable();
		if(differSize > AvailableSize)
		{
			blockOffset	= NO_MEMORY_AVAILABLE;
		}
		else
		{
			ecpMem_Move(MEMtPX8(RamManager_getAlloRamBase(U32t16(U32_A_U16(U16_A_U16(blockOffset, blockLen), differSize)))),
						MEMtPX8(RamManager_getAlloRamBase(U32t16(U16_A_U16(blockOffset, blockLen)))),
						U16_S_U16(gRamMgr_usedRamPtr, U32t16(U16_A_U16(blockOffset, blockLen))));//搬移

			RamManager_setAllocBlockHeaderLen(blockOffset, U16_A_U16(blockLen, differSize));
			gRamMgr_usedRamPtr = U32t16(U16_A_U16(gRamMgr_usedRamPtr, differSize));
			ecpMem_Set(MEMtPX8(RamManager_getAlloRamBase(U16_A_U16(blockOffset, blockLen))), 0x00, differSize);//有必要清不？头中不是记录了长度？ think in struct
		}
	}
	else if(size < dataLen)
	{
		differSize		= U16_S_U16(dataLen, size);
		ecpMem_Copy(RamManager_getAlloRamBase(U16_S_U16(U16_A_U16(blockOffset, blockLen), differSize)),
					RamManager_getAlloRamBase(U16_A_U16(blockOffset, blockLen)),
					U16_S_U16(gRamMgr_usedRamPtr, U16_A_U16(blockOffset, blockLen)));//搬移
		
		RamManager_setAllocBlockHeaderLen(blockOffset, U16_S_U16(blockLen, differSize));
		gRamMgr_usedRamPtr = U16_S_U16(gRamMgr_usedRamPtr, differSize);
		ecpMem_Set(MEMtPX8(RamManager_getAlloRamBase(gRamMgr_usedRamPtr)), 0x00, differSize);//有必要清？think in struct
	}
	
	if(NO_MEMORY_AVAILABLE != blockOffset)
	{
		RamManager_updatePointer(blockOffset);
	}

	return blockOffset;
}

void RamManager_autoReleaseRam()
{
	PX8		D_A		refAddr;
	u16		D_A		tempPointer;
	u16		D_A		tempBlockLen;
	u8		D_A		isAutoRelease;
	
	tempPointer		= U32t16(BOOT_RAM_BASE);
	while(0 != gRamMgr_autoReleaseNum)
	{
		isAutoRelease = RamManager_getAllocBlockHeaderIsAutoRelease(tempPointer);
		if(true == isAutoRelease)
		{
			refAddr = MEMtPX8(RamManager_getAllocBlockHeaderStart(tempPointer));
			RamManager_free(refAddr);
		}
		else
		{
			tempBlockLen = RamManager_getAllocBlockHeaderLen(tempPointer);
			tempPointer = U32t16(U16_A_U16(tempPointer, tempBlockLen));
		}
	}
}

u16 RamManager_getUsedDtrSize(void)
{
	u16 	D_A 		dtrSize;
	dtrSize   = U32t16(U8_M_U16(ALLOC_BLOCK_HEADER_SIZE, RamManager_calRamBlocksAmount()));
	dtrSize = U16_S_U16(gRamMgr_usedRamPtr, dtrSize);
	dtrSize = U16_S_U16(dtrSize, BOOT_RAM_BASE);
	dtrSize = U16_S_U16(dtrSize, tranMananger_getRtp());
#if JCVM__RAM__DTR__RTR__NEW	
	return dtrSize - RamManager_calReserveDtrUesdAmount();
#else
	return dtrSize;
#endif
}

#endif

