/*
 * SCP02.c
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */


#include "profile.h"
#include "opFunction.h"
#include "global_variable.h"
#include "scp02.h"
#include "SCPInterface.h"
#include "SCPGenKeyInterface.h"
#include "SCPDealData.h"
#include "keySet.h"
#include "Crypto_Constant.h"
#include "Crypto_API.h"

#if GP__SCP__SCP02

void generatePseudoRandomSCP02(PX8 cardChallengeBuffer, memref bCurentAidAdd, u8 bCurentAidLen)
{
#define NEED_CARD_CHALLENGE_LENGTH		0x06
	// for AID max length is 16, and padding 80 00 00 00 00 00 00 00 , so need 24 byte
	u8 		X_A 		tmpBuf[24];
	u8    	X_A		icv[8];

	memCopyToRam(MEMtPX8(icv), MEMtPM8(g_current_SCP_LAST_MAC), DES_BLOCKSIZE);
	ecpMem_Copy(tmpBuf, MEMtPM8(bCurentAidAdd), bCurentAidLen);

	//ecpMem_Copy(tepLastMac, (u8 *) (&lastMAC[selected_channel << 4]), (u8) 0x08);

	CalMACSCP02(MEMtPX8(tmpBuf), bCurentAidLen, MEMtPM8(g_current_SCP_MAC_KEY), icv);

	//ecpMem_Set(g_current_SCP_LAST_MAC, CLEARED_VALUE, MAC_LENGTH);
	memFillNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC), CLEARED_VALUE, MAC_LENGTH);
	//ecpMem_Copy((u8 *) (&lastMAC[selected_channel << 4]), tepLastMac, (u8) 0x08);

	//ecpMem_Copy(cardChallengeBuffer, TEMPBuffer, NEED_CARD_CHALLENGE_LENGTH);
	memCopyNoAtomic(MEMtPM8(cardChallengeBuffer), TEMPBuffer, NEED_CARD_CHALLENGE_LENGTH);
}

#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)
// Used Triple DES two key CBC mode for SCP02
// Generate Encryption session key/C-MAC session key/R-MAC session key/data encryption session key
// modify by xxj@20230512: 修正缺陷487时，做对应的调整
void BeginRmacGenKeyScp02()
{
#define DERIVATION_DATA_LENGTH 			0x10
	u8 		X_A 		derivationData[DERIVATION_DATA_LENGTH];
	PN8 		keySetObj;
	PN8 		sdDataObj;
	u8          tmpKeyVersion;
	// fill the  SCP02 derivationData

	sdDataObj = MEMtPN8(getCurrentSdDataObj(IS_NO_NEED_CHECK_SD, &tmpKeyVersion));
	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, currKeySetVer[selected_channel]));

	ecpMem_Set(derivationData, CLEARED_VALUE, DERIVATION_DATA_LENGTH);

	// scp02void cons
	derivationData[0] = U32t8(0x01);

	U16_INC(g_current_SCP_seqCounter) ;
	PX_W_U16((PX8)PX8_A_U16(derivationData , 0x02), g_current_SCP_seqCounter);
	generateRMACKey(MEMtPX8(derivationData), keySetObj);
}
#endif


// Used Triple DES two key CBC mode for SCP02
// Generate Encryption session key/C-MAC session key/R-MAC session key/data encryption session key
void generateKeyScp02(PN8 keySetObj)
{
#define DERIVATION_DATA_LENGTH 			0x10
	u8 		X_A 		derivationData[DERIVATION_DATA_LENGTH];
	u16 	X_A 		sequence_counter;
	PX8					tempPtr;
	PN8 				seqCounterField;
	u8		X_A		    sdId;
	jref	D_A		aid;
	// fill the  SCP02 derivationData
	ecpMem_Set(derivationData, CLEARED_VALUE, DERIVATION_DATA_LENGTH);

	seqCounterField  = MEMtPN8(keySet_getSequenceCounter(keySetObj, SCP02_SEQUENCE_CONTER_SIZE));
	sequence_counter = PN_R_U16(seqCounterField);

	if( (JcvmConfig_GetSsdAuthCounter()  != 0xFF) && (SC0102_PSEU_RAND == ( g_current_SCP_I_Option & SC0102_PSEU_RAND )) && (g_bSDId[selected_channel] != 0)/*|| bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA*/ )
	{
		sdId = g_bSDId[selected_channel];
		aid	= getAppletAid(getIndexByID(sdId));
		if(AID_partialEquals_array(aid, (PF8)AuthCounterNoChangesdPackageAID, 0, sizeof(AuthCounterNoChangesdPackageAID)) == true)
		{
			sequence_counter = JcvmConfig_GetSsdAuthCounter();
		}
	}
	g_current_SCP_seqCounter = sequence_counter;

	//PX_W_U16(PX8_A_U16(derivationData , 0x02), sequence_counter);
	derivationData[2] = U16t8(U16_AND_U16(U16_SHR_U8(sequence_counter, 8), 0xFF));
	derivationData[3] = U16t8(U16_AND_U16(sequence_counter, 0xFF));

	tempPtr = MEMtPX8(g_current_SCP_cardChallenge);
	
	memCopyNoAtomic(tempPtr,&derivationData[2],0x02);
	// scp02void cons
	derivationData[0] = U32t8( 0x01);

	generateCENCKey(MEMtPX8(derivationData), keySetObj);
	generateCMACKey(MEMtPX8(derivationData), keySetObj);
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)
	generateRMACKey(MEMtPX8(derivationData), keySetObj);
#endif
	generateCDECKey(MEMtPX8(derivationData), keySetObj);
	memRefresh(); // mac session key will missing; if page is not full; modified by tulei & zm;
}



/*
 * check MAC  // first single des, final triple des
 *
 * */
void CalMACSCP02(PX8 srcAddress, u8 len, PX8 keyBuff, PM8 ICVBuff)
{
	//u8 blockNum,len,length;
#define ONE_BLOCK 			0x01

	u8 		X_A		blockNum;
	u16		X_A		length;
	u16		X_A		tempI;
	PN8 			tempPtr;
	u8		X_A		temArry[DES_BLOCKSIZE];
	
	ecpMem_Set(temArry, CLEARED_VALUE, DES_BLOCKSIZE);
	
	blockNum = add80Padding(srcAddress, len, DES_BLOCKSIZE);
	length   = U16_SHL_U8(U8t16(blockNum), 3);

	tempI   = U16_S_U8(length , DES_BLOCKSIZE);
	tempPtr = srcAddress;
	

	
	memCopyToRam(MEMtPX8(temArry), ICVBuff, DES_BLOCKSIZE);
	if (ONE_BLOCK != blockNum)
	{
		single_DESMAC(tempPtr, keyBuff, TEMPBuffer, tempI, temArry);
	}

	tempPtr = PX8_A_U8(srcAddress , tempI);
	Triple_DES(tempPtr, keyBuff, MEMtPX8(TEMPBuffer), U8t16(DES_BLOCKSIZE), temArry, U32t8(CBC_Encrypt), SENSITIVE_DATA_MASKED);
	//E2P_fill_array((memref)ICVBuff, 8, 0x00);
	memCopyNoAtomic(ICVBuff, MEMtPX8(temArry), DES_BLOCKSIZE);
	memRefresh();
}


#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
u8 dealSCP0102DataRMac(PX8 SrcAddr, u8 length, u8 option, u8 sw1, u8 sw2)
{
#define ONE_BYTE						0x01
#define MACLENGTH_SUB_STATUS_LENGTH		0x06
	PX8 	X_A		ptr;
	u8 		X_A		tem[1];

	if(BEFORE_MAC_CAL == option)
	{
		//put status word into response buffer
		ptr = PX8_A_U8(SrcAddr , length);
		PX_W_U8(ptr, sw1);

		ptr = PX8_A_U16(ptr , 0x01);
		PX_W_U8(ptr, sw2);

		tem[0] = length;
		// RespLen length need add 1
		RmacUpdate(MEMtPX8(tem), ONE_BYTE);

		// RespLen length need add 2
		//plus status word length
		//length = U8_A_U8(length , 2);

		RmacUpdate(SrcAddr, U8_A_U8(length , 2));

		ecpMem_Copy(TEMPBuffer, &g_SCP_rmacTemp[1], g_SCP_rmacTemp[0]);
	}
	else
	{
//		memCopyToRam(&bResBuffer[wRespLen], g_current_SCP_LAST_MAC_L, MAC_LENGTH);

//		wRespLen[responseDataChannelNum] = (u16)U16_ADD_U8(wRespLen[responseDataChannelNum] , 2);
		//CalRmac = CalRMAC_Ture ;
		g_SCP_rmacTemp[0] = 0x00;
//		length = U8_A_U8(length, MACLENGTH_SUB_STATUS_LENGTH);
//		length = (u8)U8_S_U8(length , 2);
	}

	return length;
}


#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)

//void generateCardCryptogramSCP0102(PX8 firstChallenge, PX8 secondChallenge, PX8 derivationData)
//{
//
//#define PADDING_REMAIN_LEN		0x07
//#define CHALLENGE_LEN			0x08
//#define PADDING_80				0x80
//#define icv derivationData
//
//	PX8 tempPtr;
//	tempPtr = MEMtPX8(TEMPBuffer);
//
//	ecpMem_Copy(tempPtr, firstChallenge, CHALLENGE_LEN);
//	tempPtr = PX8_A_U16(tempPtr , CHALLENGE_LEN);
//
//	ecpMem_Copy(tempPtr, secondChallenge, CHALLENGE_LEN);
//	tempPtr = PX8_A_U16(tempPtr , CHALLENGE_LEN);
//
//	// do padding 0x80
//	PX_W_U8(tempPtr, PADDING_80);
//	tempPtr = PX8_A_U16(tempPtr , 0x01);
//
//	//do padding 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	ecpMem_Set(tempPtr, CLEARED_VALUE, PADDING_REMAIN_LEN);
//
//	//clear ICV for DES CAL
//	ecpMem_Set(MEMtPX8(icv), CLEARED_VALUE, DES_BLOCKSIZE);
//
//	tempPtr = MEMtPX8(TEMPBuffer);
//	Triple_DES(tempPtr, g_current_SCP_SES_KEY, tempPtr, (u16) 24, icv, CBC_Encrypt, SENSITIVE_DATA_MASKED);
//}

void decryptCommandDataSCP0102(PX8 apduBuf, u8 dataLen)
{
	//REVIEW
	PX8				 tempPtr;
	u8		X_A   	 ICV[DES_BLOCKSIZE];

	tempPtr = PX8_A_U16(apduBuf , OFFSET_CDATA);
	ecpMem_Set(MEMtPX8(ICV), CLEARED_VALUE, DES_BLOCKSIZE);

	Triple_DES(tempPtr, MEMtPN8(g_current_SCP_SES_KEY), tempPtr, U8t16(dataLen), MEMtPX8(ICV), CBC_Decrypt, SENSITIVE_DATA_MASKED);
}

#endif  // GP__SCP__SCP02
