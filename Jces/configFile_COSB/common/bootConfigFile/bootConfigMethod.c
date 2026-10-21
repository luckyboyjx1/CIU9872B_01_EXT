/*
 * bootConfigMethod.c
 *
 *  Created on: 2015Äê1ÔÂ4ÈÕ
 *      Author: jiangxiaohui
 */

#include "ecpDefine.h"
#include "Apdu.h"
#include "patch.h"
#include "MemOperation.h"
#include "bootConfigMethod.h"
#include "memory.h"
#include "chipHead.h"
#include "memBackUpServe.h"
#include "MemBackUp.h"
#include "bootConfigProfile.h"
#include "MemNvmOperation.h"
#include "Crypto_common_API.h"
#include "SysConfigData.h"
#include "FlashBackUp.h"
#include "SysConfig.h"
#include "Boot.h"
#include "NvmMap.h"
#include "NvmHal.h"
#if HAL__PATCH
#if HAL_PATCH_NATIVE
/****************************************************************************************************************************
***************************************************used for patch************************************************************
*****************************************************************************************************************************/
#if RUN_IN_SIMULATOR
    memref 										        dwWatchRegAddress;
    memref 										        dwPatchStartAddress;
    memref 										        dwPatchRetAddress;
    memref 										        PatchMapIndex;
    u16  											    patchLength;
    u8       											patchNo;
    u8       											patchStatus;
#else
    #define 											dwWatchRegAddress	  		class_component_Addresses
    #define 											dwPatchStartAddress	  		method_component_Addresses
    #define 											dwPatchRetAddress	  		staticfield_component_Addresses
    #define 											PatchMapIndex		  		export_component_Addresses
    #define 											patchLength           		t_Component_length
    #define 											patchNo               		export_component_offset
    #define 											patchStatus           		k_count

#endif 		

u16 patch_init(PX8 apdubuffer, PN8 PatchAreaPtr)
{
 //   u8          X_A         tempValue;
	u32 		X_A 		totalPatchSize;
	u32						src;
	u16	 					patchLength;
//	if (apdubuffer[APDU_P3_OFFSET] != 0x03)
//	{
//		return PATCH_WRONG_LENGTH;
//	}
	patchStatus 			= STATUS_INIT;
	//patchNo 				= apdubuffer[APDU_DATA_OFFSET];
	patchLength 			= PX_R_U16(MEMtPM8(&apdubuffer[APDU_DATA_OFFSET]));
	
//	// patchStartAddress 4 byte align
//	patchLength				= U16_A_U8(patchLength, 3);
//	patchLength				= U16_AND_U16(patchLength, 0xFFFC);
//	
//    if(0x00 == sysConfig_GetEePatchSize())
//	{
//        totalPatchSize		= EE_PATCH_DEFAULT_SIZE;
//        fillEeprom(chip_getPatchEntryTable(PatchAreaPtr), 0xFF, EE_PATCH_DEFAULT_SIZE);
//        nvmWriteU8NoAtomic(MEMtPN8(chip_getPatchConfigPtr(PatchAreaPtr)), 0x00);
//	}
//    else
//    {
//		totalPatchSize		= sysConfig_GetEePatchSize();
//    }
//	
//	dwPatchStartAddress 	= chip_getPatchEntryTable(PatchAreaPtr) + totalPatchSize;
//	totalPatchSize 			= U32_A_U16(totalPatchSize, patchLength);
//	nvmWriteU32Atomic(ecpPatch_getPatchSizeBase(patchAreaPtr), totalPatchSize); 
	
	// patchStartAddress 8 byte align
	patchLength				= U16_A_U8(patchLength, 7);
	patchLength				= U16_AND_U16(patchLength, 0xFFF8);
	totalPatchSize			= sysConfig_GetEePatchSize();
	
	if ( totalPatchSize + patchLength > MAX_PATCH_EEP_SIZE)
	{
		return PATCH_WRONG_LENGTH;
	}
	
	src								= 	(u32)(&(Image$$ER_PATCH_START$$Base)) + totalPatchSize;
	if(0x00 != patchLength)
	{		
		//totalPatchSize 			= U32_A_U16(memReadU32(MEMtPM8(EE_PATCH_SIZE)), patchLength);
		totalPatchSize 			= U32_A_U16(totalPatchSize, patchLength);
	}
	else	
	{		
		//totalPatchSize 			= memReadU32(MEMtPM8(EE_PATCH_SIZE));
		//totalPatchSize 			= 0;
		return PATCH_WRONG_LENGTH;
	}
	sysConfig_SetEePatchSize(totalPatchSize);
	
	patchStatus 			= STATUS_READY;
	PX_W_U32(MEMtPX8(&apdubuffer[0]) , src);
	
	apdu_ApduSetOutgoingAndSend(0,4);

	return PATCH_NO_ERROR;
}

u16 patch_load(PX8 apdubuffer, PN8 PatchAreaPtr)
{
	u32		D_A		patchAddr;
	u8 				patchLen;

	if (STATUS_READY != patchStatus)
	{
		return PATCH_CONDITIONS_NOT_SATISFIED;
	}
	if (apdubuffer[APDU_P3_OFFSET] < 0x03)
	{
		return PATCH_WRONG_LENGTH;
	}
	
	patchLen		= apdubuffer[APDU_P3_OFFSET] - 4;
	
	if((patchLen %0x08)!=0x00)
	{
		return PATCH_WRONG_LENGTH;			
	}
	patchAddr = ((apdubuffer[OFFSET_ADDR_PATCH] << 24) | (apdubuffer[OFFSET_ADDR_PATCH+1] << 16) | (apdubuffer[OFFSET_ADDR_PATCH+2] << 8) | (apdubuffer[OFFSET_ADDR_PATCH+3]));
	//nvmCopyNoAtomic(MEMtPN8(U16_A_U32(patchOff, dwPatchStartAddress)), MEMtPM8(&apdubuffer[APDU_DATA_OFFSET + 2]), U8t16(apdubuffer[APDU_P3_OFFSET] - 2));
	memCopyAtomic(MEMtPM8(patchAddr), &apdubuffer[OFFSET_DATA_PATCH], patchLen);
	if (apdubuffer[APDU_P1_OFFSET] == 0x80)
	{
		patchStatus = STATUS_FINISHED;
	}
	return PATCH_NO_ERROR;
}


u16 patch_install(PX8 apdubuffer, PN8 patchAreaPtr)
{
	u32		patchAddr;
	if (STATUS_FINISHED != patchStatus)
	{
		return PATCH_SECURITY_STATUS_NOT_SATISFIED;
	}
//	if (apdubuffer[APDU_P3_OFFSET] != 0x09)
//	{
//		return PATCH_WRONG_LENGTH;
//	}
	patchAddr = ((apdubuffer[OFFSET_ADDR_PATCH] << 24) | (apdubuffer[OFFSET_ADDR_PATCH + 1] << 16) | (apdubuffer[OFFSET_ADDR_PATCH + 2] << 8) | (apdubuffer[OFFSET_ADDR_PATCH + 3]));
	nvmCopyAtomic(MEMtPM8(patchAddr), MEMtPM8((memref)(&apdubuffer[OFFSET_DATA_PATCH])), apdubuffer[APDU_P3_OFFSET]-4);
 

	return PATCH_NO_ERROR;
}

u16 patch_check(PX8 apdubuffer, PN8 PatchAreaPtr)
{
	return PATCH_SECURITY_STATUS_NOT_SATISFIED;
}

u16 patch_modify_default(PX8 apdubuffer, PN8 PatchAreaPtr)
{
//	/*
//		P1= 00 change dynamic patch area to default value
//		p1= 01 change dynamic patch area config byte: bit 7 Write or read, default read
//		p1= 02 change dynamic patch area config byte: bit 3 Service select for EEPROM or ROM, default ROM
//		p1= 03 change dynamic patch area config byte: bit 2 patch entrance select that is EEPROM or ROM, default ROM
//		P1= 04 change dynamic patch area config byte: set config byte to be apdubuffer input data 
//	*/
////	nvmCopyNoAtomic(ecpPatch_getPatchTableBase(PatchAreaPtr), MEMtPM8(&apdubuffer[APDU_DATA_OFFSET ]), 4);
//	u8 X_A temp;
//	PN8 tempPtr;
//	
//	tempPtr = MEMtPN8(ecpPatch_getPatchConfigPtr(PatchAreaPtr));
//	temp = PN_R_U8(tempPtr);
//	
//	if (apdubuffer[APDU_P3_OFFSET] != 0x01)
//	{
//		return PATCH_WRONG_LENGTH;
//	}
//	
//	switch(apdubuffer[APDU_P1_OFFSET])
//	{
//		case 0x00:
//				fillEeprom(ecpPatch_getPatchEntryTable(PatchAreaPtr), 0xFF, EE_PATCH_DEFAULT_SIZE);
////				nvmWriteU8NoAtomic(ecpPatch_getPatchConfigPtr(PatchAreaPtr), 0);
////				nvmWriteU8NoAtomic(tempPtr, 0);
//				temp = 0;
//				break;
//		case 0x01:
//				if(0 != apdubuffer[APDU_DATA_OFFSET])
//				{
//					temp |=  0x80;
//				}
//				else
//				{
//					temp &=  ~0x80;
//				}
//				break;
//		case 0x02:
//				if(0 != apdubuffer[APDU_DATA_OFFSET])
//				{
//					temp |=  0x08;
//				}
//				else
//				{
//					temp &=  ~0x08;
//				}
//				break;
//		case 0x03:
//				if(0 != apdubuffer[APDU_DATA_OFFSET])
//				{
//					temp |=  0x04;
//				}
//				else
//				{
//					temp &=  ~0x04;
//				}
//				break;
//		case 0x04:
//				temp = apdubuffer[APDU_DATA_OFFSET];
//				break;
//		default:
//				break;
//	}
//	nvmWriteU8NoAtomic(tempPtr, temp);
	return PATCH_NO_ERROR;
}

u16 patch_alloc_EEPROM(PX8 apdubuffer, PN8 PatchAreaPtr)	
{
//	//patch no watch addr , patch length , // return addr
//	if (apdubuffer[APDU_P3_OFFSET] != 0x03)
//	{
//		return PATCH_WRONG_LENGTH;
//	}
//	patchStatus 			= STATUS_INIT;
//	patchNo 				= apdubuffer[APDU_DATA_OFFSET];
//	patchLength 			= PX_R_U16(MEMtPM8(&apdubuffer[6]));

//	//allocate eep for the patch
//	PatchMapIndex 			= NVM_alloc(U16t32(patchLength), 0);
//	dwPatchStartAddress 	= MapTable_GetEntityContent(PatchMapIndex);
//	patchStatus 			= STATUS_READY;
//	PX_W_U32(MEMtPX8(&apdubuffer[0]) , dwPatchStartAddress);
//	apdu_ApduSetOutgoingAndSend(0,4);

	return PATCH_NO_ERROR;
}
//u16 patch_alloc_DTR_RTR(PX8 apdubuffer, PN8 PatchAreaPtr)	
//{
//	u16 dataref;
//	u16 datasize;
//	if (apdubuffer[APDU_P3_OFFSET] != 0x02)
//	{
//		return PATCH_WRONG_LENGTH;
//	}
//	datasize = PX_R_U16(MEMtPM8(&apdubuffer[APDU_DATA_OFFSET]));
//	if(0x80 ==(apdubuffer[APDU_P1_OFFSET]& 0x80))//DTR
//	{
//		dataref = ramAllocate_dtrAlloc(cc, U32t16(datasize));
//	}
//	else//RTR
//	{
//		dataref = ramAllocate_rtrAlloc(U32t16(datasize));
//	}
//	PX_W_U16(MEMtPX8(&apdubuffer[0]) , dataref);
//	apdu_ApduSetOutgoingAndSend(0,2);
//	return PATCH_NO_ERROR; 
//}

u16 patch_install_InRom(PX8 apdubuffer, PN8 patchAreaPtr)
{
	return PATCH_NO_ERROR;
}
#endif //HAL_PATCH_NATIVE

void patch_clear(PN8 patchAreaPtr)
{
//	fillEeprom(MEMtPN8(chip_getPatchFlagStartBase(patchAreaPtr)), 0xFF, PATCH_AREA_CONFIG_LENGTH);
//	fillEeprom(MEMtPN8(chip_getPatchSizeBase(patchAreaPtr)), 0x00, PATCH_TOTAL_SIZE_SIZE);	
//	nvmWriteU32NoAtomic(MEMtPN8(chip_getPatchTableBase(patchAreaPtr)),MASK_MAP_START);
	return;
}
#endif //HAL__PATCH



u8 HardWareCheckBackUpFlag(PX8	tempbackUpTempBuf, PN8	tempAddress, u8 i,u1 isReset,PX8 tempRealWriteBuf)
{
	u8		tempResult;

	tempResult		= true;
//	if(ABORT_ON_RESET == isReset)
//	{
//		if(!FlashBackUp_IsSurpport())
//		{
//			if(0x00 == tempRealWriteBuf[8])
//			{
//				ecpMem_Copy(tempRealWriteBuf, tempAddress, BACK_UP_PAGE_HEADER_SIZE);
//				setBackUpPageFlag(&tempRealWriteBuf[4], TF_BACKUP);
//				setBackUpPageCounter(&tempRealWriteBuf[4], i);
//				setBackUpPageCrc(&tempRealWriteBuf[4], calBackUpPageCrc(&tempRealWriteBuf[4]));
//				MemNvmOperation_WriteErase(MEMtPX8(SAVE_TRANS_FLAG_AREA), tempRealWriteBuf, SAVE_TRANS_FLAG_SIZE);
//
//				MemNvmOperation_WriteErase(tempAddress, tempbackUpTempBuf, BACK_UP_PAGE_HEADER_SIZE);
//			}
//			fillEeprom(MEMtPN8(SAVE_TRANS_FLAG_AREA), 0x00, SAVE_TRANS_FLAG_SIZE);
//			tempResult	= true;
//		}
//	}
//	else
//	{
//		tempResult		= true;
//	}
	return tempResult;
}
static u8 checkCompactFlag(PX8	tempbackUpTempBuf, PN8	tempAddress, u8 i)
{
//	if(	(PX_R_U8(PN8_A_CONST(tempbackUpTempBuf, NVM_COMPACT_TAG_OFFSET)) == NVM_COMPACT_BACKUP) && \
//			(Get_crc16(tempAddress, NVM_COMPACT_HEAD_LENGTH) == PX_R_U16(PN8_A_CONST(tempbackUpTempBuf, NVM_COMPACT_CRC_OFFSET))) && \
//			(i <= (BACK_UP_MAX_PAGE - 3))\
//	)
//	{
//		return  true;
//	}
//	else
	{
		return  false;
	}		
}
u8	HardWareCheckCompFlag(PX8	tempbackUpTempBuf, PN8	tempAddress, u8 i,u1 isReset) 					
{

	u8		tempResult;
	
	tempResult = checkCompactFlag(tempbackUpTempBuf, tempAddress, i);	

	return tempResult;

}

void RealWriteBakupPage(PN8	tempAddress,PX8 tempRealWriteBuf)
{
	u8		i;

	if(!FlashBackUp_IsSurpport())
	{
		ecpMem_Copy(tempRealWriteBuf, MEMtPX8(SAVE_TRANS_FLAG_AREA), SAVE_TRANS_FLAG_SIZE);
		tempRealWriteBuf[8] = 0;
		if( (calBackUpPageCrc(tempRealWriteBuf) == getBackUpPageCrc(tempRealWriteBuf)) && (calBackUpPageCrc(&tempRealWriteBuf[4]) == getBackUpPageCrc(&tempRealWriteBuf[4])))
		{
			if((TF_BACKUP == tempRealWriteBuf[0]) && (TF_BACKUP == tempRealWriteBuf[4]) )
			{
				if(tempRealWriteBuf[5] < BACK_UP_MAX_PAGE)
				{
					tempRealWriteBuf[9] = 0;
					for(i = 0;i < 8;U8_INC(i))
					{
						if(0x00 != ecpMem_Compare(tempRealWriteBuf, MEMtPX8(SAVE_TRANS_FLAG_AREA), SAVE_TRANS_FLAG_SIZE))
						{
							tempRealWriteBuf[9] = 1;
							break;
						}
					}
					if(0x00 == tempRealWriteBuf[9])
					{
						MemNvmOperation_WriteErase(PN8_A_CONST(tempAddress, U16_M_U16(EEPAGE_SIZE, tempRealWriteBuf[5])), tempRealWriteBuf, BACK_UP_PAGE_HEADER_SIZE);
						tempRealWriteBuf[8] = 1;
					}
				}
			}			
		}
	}
}


u16 HardWareCheckBackUpBlock(u16 tempOffset)
{
//	PN8		X_A		tempAddr;
//	u16		X_A		tempOtherOff;
//	u8		X_A		tempSelType;

//	tempSelType		= THD88IsHardWareBackUpSel();
//	tempOtherOff	= 0;
//	tempAddr		= PN8_A_U16(MEMtPN8(BACK_UP_AREA_START), tempOffset);
//	if(tempSelType)
//	{
//		tempOtherOff = U16_AND_U16(getBackUpBlockTypeSize(tempAddr), BACK_UP_BLOCK_LENGTH_MASK);
//
//		if(tempOtherOff > EEPAGE_SIZE)
//		{
//			return searchBlocks(0, 0, tempOffset, GET_LAST_BLOCK);
//		}
//
//		tempOtherOff	= U16_A_U16(tempOtherOff, BACK_UP_BLOCK_HEADER_SIZE);
//
//		if( compareBackUpBlock(tempAddr, 0, tempOffset, tempOtherOff) == false)
//		{
//			return searchBlocks(0, 0, tempOffset, GET_LAST_BLOCK);
//		}
//	}
	return tempOffset;
}


void platformfillEeprom(PN8 dest, u8 value, u32 length)
{
#if HAL__PLATFORM__SIMULATOR
	
	u8 backupPageCount;

	backupPageCount = sysConfig_GetBackUpPageNum();

	while(length > 0x8000)
	{
		fillEeprom(MEMtPN8(dest), NVM_FILL_CHARACTER, 0x8000);
		dest = MEMtPN8(PN8_A_CONST(dest, 0x8000));
		length = U32_S_U16(length, 0x8000);
	}
	
	fillEeprom(MEMtPN8(dest), value, U32t16(length));
	//NVM_FillNByte( value, length, dest ); 
	do
	{
		ecpNvm_ErasePage((PN8)(U32_S_U16(sysConfig_GetNvmTop(), U16_M_U16(EEPAGE_SIZE, backupPageCount))));
		U8_DEC(backupPageCount);
	}
	while(backupPageCount > 0);
#else
	while(length > 0x8000)
	{
		EraseNvm(dest,0x8000);
		HARDWEAR_E2P_Fill(dest, NVM_FILL_CHARACTER, 0x8000);
		
		dest = MEMtPN8(PN8_A_CONST(dest, 0x8000));
		length = U32_S_U16(length, 0x8000);
	}
	EraseNvm(dest,length);
	HARDWEAR_E2P_Fill(dest, NVM_FILL_CHARACTER, length);

#endif
}

#if (RUN_IN_SIMULATOR | MAP_ADDRESS_NEED_DEAL)
memref dealMAPAddress(MapEntry_t  mapEntry, u16 index)
{
	u32 entry;

	entry = ((* (PN32)memoryToPoint((memref)(mapEntry))));

#if RUN_IN_SIMULATOR & BIG_ENDIAN
	entry = htoj32(entry);
#endif

	entry = entry & 0x00FFFFFF;
	if(entry == 0)
	{
		return 0;
	}
	else
	{
		return PNtMEM(U32_OR_U32(entry,ROM_BASE));
	}

}
#endif
//void rtnBackup(void)
//{
//	;
//}



