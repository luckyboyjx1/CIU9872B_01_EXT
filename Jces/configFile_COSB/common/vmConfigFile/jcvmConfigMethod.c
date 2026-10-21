/*
 * jcvmConfigMethod.c
 *
 *  Created on: 2014-12-18
 *      Author: mafuxuan
 */

#include "memory.h"
void JcvmConfig_GetConstructAuthChipBase(PX8 buf)
{


}
void dealWithPseudoRandom()
{
//	PN8 tempDataAddr;
//
//	tempDataAddr = MEMtPN8(U32_A_U8(locateObjectInMemory(PN_R_U16(MEMtPN8(cardData_INDEX))) ,0x3A));
//	if(GP_SECOND_CONFIG1_SSD_PSEUDO_RANDOM)
//	{
//		if(PN_R_U8(tempDataAddr) != 0x55)
//		{
//			nvmWriteU8NoAtomic(tempDataAddr, 0x55);
//		}
//	}
//	else
//	{
//		if(PN_R_U8(tempDataAddr) != 0x15)
//		{
//			nvmWriteU8NoAtomic(tempDataAddr, 0x15);
//		}
//	}

}

void getData_FF32()
{	
//	Crypto1GetVersion(MEMtPX8(&APDUBuffer[OFFSET_CDATA]));
//	CryptoSM1Version(MEMtPX8(&APDUBuffer[OFFSET_CDATA + 2]));
//	CrytoSULibVersion(MEMtPX8(&APDUBuffer[OFFSET_CDATA + 4]));
//	CrytoSM4LibVersion(MEMtPX8(&APDUBuffer[OFFSET_CDATA + 6]));
//	GetSystemClockLibVersion(MEMtPX8(&APDUBuffer[OFFSET_CDATA + 8]));
//	tmcPKEGetVer(MEMtPX8(&APDUBuffer[OFFSET_CDATA + 0x0A]));

	//Get OS BootVersion
	ecpMem_Copy(MEMtPX8(&APDUBuffer[OFFSET_CDATA + 0x0C]), &bootVersion[0], BOOT_VERSION_SIZE);
	
    apdu_ApduSetOutgoingAndSend(5, 0x0C + BOOT_VERSION_SIZE);
}

memref JcvmConfig_GetSaveTransFlagAreaBase(void)
{
	return (E2P_IMAGE_START - 0x10);
}

//u16 getPrivateData(PN8 apduBuffer,u8 type)
//{
//	return 0x9000;
//}
