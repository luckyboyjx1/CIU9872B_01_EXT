/*
 * SCP01.c
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */


#include "profile.h"
#include "SCPInterface.h"
#include "global_variable.h"
#include "scp01.h"
#include "SCPGenKeyInterface.h"
#include "SCPDealData.h"
#include "opFunction.h"


#if GP__SCP__SCP01



// Used Triple DES two key ECB mode for SCP01
// Generate Encryption session key/C-MAC session key/R-MAC session key/data encryption session key
void generateKeyScp01(PN8 keySetObj)
{
#define 	DERIVATION_DATA_LENGTH_SCP01 				0x10
#define 	DATA_ONE_STEP_ADDOFFSET_CDATA				0x09

	u8		XDATA_AREA 		derivationData[DERIVATION_DATA_LENGTH_SCP01];
	PX8 				tempPtr;
	tempPtr = (PX8)derivationData;
	// fill the  SCP01 derivationData
	ecpMem_Copy((PX8) tempPtr, (PX8) (PX8_A_U16(g_current_SCP_cardChallenge , 0x04)), 0x04); //cardChallenge + 4
	ecpMem_Copy((PX8) (PX8_A_U16(tempPtr , 0x04)), (PX8) (PX8_A_U16(APDUBuffer , OFFSET_CDATA)), 0x04); // hostChallenge + 0
	ecpMem_Copy((PX8) (PX8_A_U16(tempPtr , 0x08)), g_current_SCP_cardChallenge, 0x04); //cardChallenge + 0
	ecpMem_Copy((PX8) (PX8_A_U16(tempPtr , 0x0C)), (PX8) (PX8_A_U16(APDUBuffer , DATA_ONE_STEP_ADDOFFSET_CDATA)), 0x04); //hostChallenge + 4


	generateCENCKey(derivationData, keySetObj);
	generateCMACKey(derivationData, keySetObj);
	generateCDECKey(derivationData, keySetObj);
}


/*
 * check MAC  // first single des, final triple des
 *
 * */
void CalMACSCP01(RAM_U8_P srcAddress, u8 len, RAM_U8_P keyBuff, RAM_U8_P ICVBuff)
{
	//u8 blockNum,len,length;
	u8 		XDATA_AREA		blockNum;
	u8 		XDATA_AREA		tempI;
	PX8              tempPtr;
	u8		XDATA_AREA		tempArry[DES_BLOCKSIZE];

	blockNum = add80Padding(srcAddress, len, DES_BLOCKSIZE);
	tempPtr = srcAddress;
	memCopyToRam((memref)tempArry, (memref)ICVBuff, DES_BLOCKSIZE);
	for (tempI = 0; tempI < blockNum; U8_INC(tempI))
	{
		Triple_DES(tempPtr, (MEM_U8_P) keyBuff, (PX8) (TEMPBuffer), (u16) DES_BLOCKSIZE, tempArry, (u8) CBC_Encrypt, SENSITIVE_DATA_MASKED);
		tempPtr = PX8_A_U16(tempPtr , DES_BLOCKSIZE);
	}
	memCopyNoAtomic((memref)ICVBuff, (memref)tempArry, DES_BLOCKSIZE);
	memRefresh();
}

#endif
