/*
 * SCP03.c
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */

#include "profile.h"
#include "global_variable.h"
#include "SCP03.h"
#include "SCPInterface.h"
#include "opFunction.h"
#include "SCPGenKeyInterface.h"
#include "SCPDealData.h"
#include "keySet.h"
#include "Crypto_Constant.h"
#include "SEEUICCInterface.h"  // remove warning,dengyiwen,2022/9/22


#if GP__AMENDMENT__D



/*
 * Function name: generatePseudoRandom
 * Description: generate pseudo-random.
 * Input Parameters:   (precondition)
 *		u8 *staticKey.
 *		u8 *cardChallengeBuffer.
 *		memref bCurentAidAdd
 *		u8 bCurentAidLen
 * Output Parameters:  (postcondition)
 *		NULL
 * NOTE:
*/
//need to define derivationData[100]

void SCP03_generatePseudoRandom(PN8 keySetObj, PX8 cardChallengeBuffer, memref bCurentAidAdd, u8 bCurentAidLen)
{
#define DERIVATION_DATA_LENGTH_SCP03		0x30
	u8 		X_A		derivationData[DERIVATION_DATA_LENGTH_SCP03];
	u8 		X_A		ICV[AES_BLOCKSIZE];
	u32 	X_A		sequence_counter;
	u16 	X_A		tempKeyLen;
	u8		X_A		tempI;
//	memref 	X_A		keyAddress;
	u8 		X_A		keyAddress[SESSION_KEY_LENGTH];
	PN8 				seqCounterField;

	ecpMem_Set(ICV, CLEARED_VALUE, AES_BLOCKSIZE);
	ecpMem_Set((PX8) derivationData, CLEARED_VALUE, DERIVATION_DATA_LENGTH_SCP03);

	g_SCP_sesKeyLen = (u8)keyData_getKeyDataById(keySetObj, SCP_STATICKEY_FIRST_KEYID, 0x00, MEMtPX8(keyAddress));

	derivationData[OFFSET_11] = SCP03_DERI_CONST_CARD_RAND;
	derivationData[OFFSET_14] = VALUE_40;
	derivationData[OFFSET_15] = VALUE_01;

	seqCounterField = MEMtPN8(keySet_getSequenceCounter(keySetObj, SCP03_SEQUENCE_CONTER_SIZE));
	sequence_counter = readScp03Counter(seqCounterField);//PN_R_U24(seqCounterField);
    //added by xuxiaojun@20220628 用于SCP02和SCP03共同支持伪counter. 与SCP02的伪counter方案一致
	if( (JcvmConfig_GetSsdAuthCounter()  != 0xFF) && (g_bSDId[selected_channel] != 0)/*|| bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA*/ )
	{
		if(AID_partialEquals_array(getAppletAid(getIndexByID(g_bSDId[selected_channel])), (PF8)AuthCounterNoChangesdPackageAID, 0, sizeof(AuthCounterNoChangesdPackageAID)) == true)
		{
			sequence_counter = JcvmConfig_GetSsdAuthCounter();
		}
	}
	derivationData[OFFSET_16] = (u8)(sequence_counter >> 16);
	derivationData[OFFSET_17] = (u8)(sequence_counter >> 8);
	derivationData[OFFSET_18] = sequence_counter & 0xff;
	//pad aid

	ecpMem_Copy((PX8)(PX8_A_U16(derivationData , OFFSET_19)), MEMtPN8(bCurentAidAdd), bCurentAidLen);

	tempI = (u8)U8_A_U8(bCurentAidLen, OFFSET_19);

	tempKeyLen = (u16)(g_SCP_sesKeyLen << 3);
//	subKey((PN8)(keyAddress), tempKeyLen, g_current_SCP_DEK_KEY);
//	cal_CMAC((PN8)(keyAddress), tempKeyLen, (PX8) derivationData, TEMPBuffer, g_current_SCP_DEK_KEY, tempI, ICV);
	symCmacAlgorithmCal(NULL, 0, derivationData, tempI, TEMPBuffer, keyAddress, tempKeyLen, ICV, CBC_MAC_Encrypt, ALG_AES);

	memRefresh();

	//ecpMem_Copy不能写NVM，要调用memCopyAtomic来写
	//ecpMem_Copy(cardChallengeBuffer,TEMPBuffer,CARD_CHALLENGE_LENGTH);
	memCopyAtomic(cardChallengeBuffer,TEMPBuffer,CARD_CHALLENGE_LENGTH);
#undef DERIVATION_DATA_LENGTH_SCP03
}



/*
*generate s-mac/r-mac/s-enc key
*
*/
void SCP03_generateKey(PN8 keySetObj)
{

#define		ICV				(PX8_A_U16(derivationData , 0x20))
	u8		X_A		derivationData[0x40];

	ecpMem_Set(ICV, CLEARED_VALUE, AES_BLOCKSIZE);

	generateCENCKey(derivationData, keySetObj);

	generateCMACKey(derivationData, keySetObj);

	generateRMACKey(derivationData, keySetObj);

	generateCDECKey(derivationData, keySetObj);

#undef	ICV
}


void SCP03_generateKeyProc(PN8 staticKey, u8 keyLen ,PX8 sessionKeyAdd, u8 keyType , PX8 derivationData)
{
#define 	hostChallengeBuff	(PX8_A_U16(derivationData , 0x10))
#define 	cardChallengeBuff	(PX8_A_U16(derivationData , 0x18))
#define		ICV					(PX8_A_U16(derivationData , 0x20))

	u16 	X_A		keyBitLen;

	keyBitLen = (u16)( keyLen << 3 );
	switch(keyLen)
	{
		case 16:
		case 24:
		case 32:

			ecpMem_Set(derivationData, 0x00, OFFSET_11);

			derivationData[OFFSET_11] = keyType;
			derivationData[OFFSET_12] = 0x00;
			derivationData[OFFSET_13] = (u8)( keyBitLen >>8 );
			derivationData[OFFSET_14] = (u8)keyBitLen;
			derivationData[OFFSET_15] = 0x01;

			ecpMem_Copy((PX8)PX8_A_U16(derivationData , 0x10), g_current_SCP_hostChallenge, HOST_CHALLENGE_LENGTH);
			ecpMem_Copy((PX8)PX8_A_U16(derivationData , 0x18), g_current_SCP_cardChallenge, CARD_CHALLENGE_LENGTH);

//			subKey(staticKey, keyBitLen, sessionKeyAdd);
//			cal_CMAC(staticKey, keyBitLen, derivationData, g_current_SCP_DEK_KEY, sessionKeyAdd, 32, ICV);
//			aesCmacAlgorithmCal(NULL, 0, derivationData, 32, g_current_SCP_DEK_KEY, staticKey, keyBitLen, ICV, CBC_MAC_Encrypt, ALG_AES);
			Cmac(staticKey, keyLen, derivationData, 32, NULL, 0, g_current_SCP_DEK_KEY);

			if( keyLen > 16 )
			{
				derivationData[OFFSET_15] = 0x02;
				ecpMem_Copy(hostChallengeBuff, g_current_SCP_hostChallenge, HOST_CHALLENGE_LENGTH);
				ecpMem_Copy(cardChallengeBuff, g_current_SCP_cardChallenge, CARD_CHALLENGE_LENGTH);
//				cal_CMAC(staticKey, keyBitLen, derivationData, g_current_SCP_DEK_KEY_L, sessionKeyAdd, 32, ICV);
//				aesCmacAlgorithmCal(NULL, 0, derivationData, 32, g_current_SCP_DEK_KEY_L, staticKey, keyBitLen, ICV, CBC_MAC_Encrypt, ALG_AES);
				Cmac(staticKey, keyLen, derivationData, 32, NULL, 0, g_current_SCP_DEK_KEY_L);
			}

			memRefresh();

			ecpMem_Copy(sessionKeyAdd, g_current_SCP_DEK_KEY, keyLen);
		break;

		default:
		break;

	}
#undef 	hostChallengeBuff
#undef 	cardChallengeBuff
#undef	ICV
}



void SCP03_CalMAC(PX8 srcAddress, u8 len, PX8 keyBuff, u8 keyByteLen)
{
	u8 		X_A		notAlignByteLen;
	u16 	X_A		macKeyBitLen;
	//u8		X_A		subKeyTemp[32];  // remove warning,dengyiwen,2022/9/22
	u8 		X_A		tmpBuf[16];
	u8		X_A		temI;
	PX8				temPtr;

	notAlignByteLen = (u8)( len & 0x0F );
	if( 0x00 == notAlignByteLen )
	{
		notAlignByteLen = 0x10 ;
	}

	temI = (u8)U8_S_U8(len, notAlignByteLen);
	temPtr = (PX8) (PX8_A_U8(srcAddress , temI));
	//save apduBuffer data
	ecpMem_Copy(tmpBuf, temPtr, notAlignByteLen );

	// clear TEMPBuffer.
	ecpMem_Set(TEMPBuffer, CLEARED_VALUE, AES_BLOCKSIZE);

	//calculate mac
	macKeyBitLen = (u16) ( keyByteLen << 3 );
//	subKey(keyBuff, macKeyBitLen, subKeyTemp);
//	AES(g_current_SCP_LAST_MAC, keyBuff, macKeyBitLen, TEMPBuffer, 16, TEMPBuffer, CBC_Encrypt);
//	cal_CMAC(keyBuff, macKeyBitLen, srcAddress, TEMPBuffer, subKeyTemp, (u16)(len), TEMPBuffer);
	symCmacAlgorithmCal(MEMtPM8(g_current_SCP_LAST_MAC), 16, srcAddress, len, TEMPBuffer, keyBuff, macKeyBitLen, TEMPBuffer, CBC_MAC_Encrypt, ALG_AES);

	//restore apdubuffer data
	ecpMem_Copy( temPtr, tmpBuf, notAlignByteLen );

}


void SCP03_generateCardCryptogram(PX8 firstChallenge, PX8 secondChallenge, PX8 derivationData, u8 bOp)
{

#define		ICV			(PX8) (PX8_A_U16(derivationData , 32))
#define		tempBuf		(PX8) (PX8_A_U16(derivationData , 48))
	u16 	X_A		keyBitLen;
	ecpMem_Set(ICV, CLEARED_VALUE, AES_BLOCKSIZE);
	ecpMem_Set(derivationData, CLEARED_VALUE, 0x20);
	derivationData[14] = 0x40;
	derivationData[15] = 0x01;

	ecpMem_Copy((PX8) (PX8_A_U16(derivationData , 16)), firstChallenge, HOST_CHALLENGE_LENGTH);
	ecpMem_Copy((PX8) (PX8_A_U16(derivationData , 24)), secondChallenge, CARD_CHALLENGE_LENGTH);

	if( GENERATE_CARD_CRYPRTOGRAM == bOp )
	{
		//generate card crypto
		derivationData[11] = SCP03_DERI_CONST_CARD_CRY;
	}
	else
	{
		//compare host crypto
		derivationData[11] = SCP03_DERI_CONST_HOST_CRY;
	}
	keyBitLen = (u16)(g_SCP_macKeyLen << 3);

//	subKey(g_current_SCP_MAC_KEY, keyBitLen, tempBuf);
//	cal_CMAC(g_current_SCP_MAC_KEY, keyBitLen, derivationData, TEMPBuffer, tempBuf, 32, ICV);
	symCmacAlgorithmCal(NULL, 0, derivationData, 32, TEMPBuffer, MEMtPM8(g_current_SCP_MAC_KEY), keyBitLen, ICV, CBC_MAC_Encrypt, ALG_AES);
#undef	ICV
#undef	tempBuf
}



void SCP03_increaseICVCount()
{
	u8 		X_A		tempI;

	tempI = SCP03_ICV_COUNTER_LASTOFF;
	while(0x00 != tempI)
	{
		U8_INC(g_SCP_ICV_Counter[tempI]);
		if(0x00 == g_SCP_ICV_Counter[tempI])
		{
			U8_DEC(tempI);
		}
		else
		{
			break;
		}
	}

//	add by jiangXiaoHui .if the ICV is matched or beyond the max, throw something or tell something to the users
//	if(0x01 == tempI)
//	{
//		if(ICV_Counter[tempI] > 0xFE)
//		{
//			throw_error(status1);
//		}
//	}
//	else if(0x00 == tempI)
//	{
//		throw_error(status2);
//	}
}


//void SCP03_subICVCount(PX8 counter)
//{
//	u8 		X_A		tempI;
//
//	tempI = SCP03_ICV_COUNTER_LASTOFF;
//	while(0x00 != tempI)
//	{
//		U8_DEC(counter[tempI]);
//		if(0xFF == counter[tempI])
//		{
//			U8_DEC(tempI);
//		}
//		else
//		{
//			break;
//		}
//	}
//}


/*
 *
 *
 * */
void SCP03_decryptCommandData(PX8 apduBuf)
{
	u8 		X_A		ICV[AES_BLOCKSIZE];
	u8 		X_A		tempLength;
	u16 	X_A		encBitLen;
	PX8				tempPtr1;
	PX8				tempPtr2;

	tempLength = apduBuf[OFFSET_LC];

	if (0x00 != (U16_AND_U16(tempLength, AES_BLOCKSIZE_MASK)) || (0x00 == tempLength)) //length not multiple of AES_BLOCKSIZE or length is zero.
	{
		return;
	}

	SCP03_increaseICVCount();//，ICV的初始值是0  ， 附录D  The encryption counter’s start value shall be set to 1 for the first command following a successful EXTERNAL AUTHENTICATE command.

	tempPtr1 = (PX8)ICV;

	// get ICVCount
	ecpMem_Copy(tempPtr1, (PX8)g_SCP_ICV_Counter, SCP03_ICV_COUNTER_LENGTH);

	//get encrypted ICV
	encBitLen = g_SCP_sesKeyLen << 3;

//	AES(tempPtr1, g_current_SCP_SES_KEY, encBitLen, tempPtr1, (u16)AES_BLOCKSIZE, tempPtr1, ECB_Encrypt);
	SymCipherDofinal(tempPtr1, (u16)AES_BLOCKSIZE, tempPtr1, MEMtPM8(g_current_SCP_SES_KEY), encBitLen, NULL, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, ECB_Encrypt, SBC_INIT, ALG_AES);

	//decrypt command data
	tempPtr2 = PX8_A_U16(apduBuf , OFFSET_CDATA);

//	AES(tempPtr2, g_current_SCP_SES_KEY, encBitLen, tempPtr2, tempLength, tempPtr1, CBC_Decrypt);
	SymCipherDofinal(tempPtr2, tempLength, tempPtr2, MEMtPM8(g_current_SCP_SES_KEY), encBitLen, tempPtr1, NULL, ALG_AES_BLOCK_128_CBC_NOPAD, CBC_Decrypt, SBC_INIT, ALG_AES);

	// updata ICVCount
//	SCP03_increaseICVCount();
}

u8 SCP03_dealRMacData(PX8 SrcAddr, u8 length, u8 option, u8 sw1, u8 sw2)
{
	PX8		tempPtr;

	tempPtr = SrcAddr;
	if(BEFORE_MAC_CAL == option)
	{
		// RespLen length need add 2
		//put status word into response buffer
		tempPtr[length] = sw1;
		U8_INC(length);

		tempPtr[length] = sw2;
		U8_INC(length);
	}
	else
	{
		// pop 2 status
//		tempPtr = XUADDR_SUB_CONST(tempPtr , 2);
		tempPtr = PX8_S_CONST(tempPtr , 2);
		tempPtr = PX8_A_U8(tempPtr , length);
		ecpMem_Copy(tempPtr, TEMPBuffer, 8);
		length = U8_A_U8(length, 0x06);
	}

	return length;
}

u16 SCP03_EncryptOfResponse(PX8 SrcAddr ,u16 length)
{
	u8 		X_A		ICV[AES_BLOCKSIZE];
	u16 	X_A		keyBitLen;
	u8 		X_A		blockNum;

	if(0x00 != length)
	{
		ecpMem_Copy(ICV, g_SCP_ICV_Counter, SCP03_ICV_COUNTER_LENGTH);

		//SCP03_subICVCount((PX8)ICV); //scp03的icv counter初始化成0 ，每次指令先+1再使用 ，解密与加密用的都是同一个ICV 不需要再减1

		ICV[0] = CRYPTO_PADDING_80;
		keyBitLen = (u16)(g_SCP_sesKeyLen << 3);
		//get encrypted ICV
//		AES(ICV, g_current_SCP_SES_KEY, keyBitLen, ICV, (u16)AES_BLOCKSIZE, ICV, ECB_Encrypt);
		SymCipherDofinal(ICV, (u16)AES_BLOCKSIZE, ICV, MEMtPM8(g_current_SCP_SES_KEY), keyBitLen, ICV, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, ECB_Encrypt, SBC_INIT, ALG_AES);
		//padding data
		blockNum = (u8)add80Padding(SrcAddr, length, AES_BLOCKSIZE);
		length = (u8) (blockNum << AES_KEY_PADDING_OFF);
//		AES(SrcAddr, g_current_SCP_SES_KEY, keyBitLen, SrcAddr, length, ICV, CBC_Encrypt);
		SymCipherDofinal(SrcAddr, length, SrcAddr, MEMtPM8(g_current_SCP_SES_KEY), keyBitLen, ICV, NULL, ALG_AES_BLOCK_128_CBC_NOPAD, CBC_Encrypt, SBC_INIT, ALG_AES);//modify by xuxiaojun@20220608 参与计算的输入数据长度错误，SCP11测试脚本: SCP11c_auth_level_Test_isd
	 }

	return length;
}

#endif
u32 readScp03Counter(PX8 CounterFieldAddr)
{
	u8     X_A    byte1,byte2,byte3;
	u32    X_A    counterValue;

	byte1 = memReadU8(CounterFieldAddr);

	PX8_INC(CounterFieldAddr);
	byte2 = memReadU8(CounterFieldAddr);

	PX8_INC(CounterFieldAddr);
	byte3 = memReadU8(CounterFieldAddr);

	counterValue = (byte1<<16)|(byte2<<8)|(byte3);

	return counterValue;
}

