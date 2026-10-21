/*
 * SCP10.c
 *
 *  Created on: 2013-9-16
 *      Author: jiangxiaohui
 */

#include "SCPInterface.h"
#include "CardManager.h"
#include "object.h"
#include "Loader.h"
#include "opFunction.h"
#include "global_variable.h"
#include "error.h"
#include "profile.h"
#include "scp10.h"
#include "nativemethod_impl.h"
#include "SCPGenKeyInterface.h"
#include "SCPDealData.h"
#include "SCPParamCheck.h"
#include "OPCMCal.h"
#include "keySet.h"
#include "SecurityDomainData.h"
#include "casdStoreKey.h"
#include "keyData.h"


#if GP__SCP__SCP10__FOR_MASK


#define 		CARD_CHALLEGE_ID_LENGTH						0x10
#define 		RAND_DATA_LENGTH							0x20
#define 		CARD_CHALLEGE_ID_ADD_RAND_DATA_LENGTH		0x30


#define SCP10_DEAL_SECLVCRT_CDATA_DATA_SIZE			0x02
const u8 SCP10_DEAL_SECLVCRT_CDATA_DATA_VALUE[SCP10_DEAL_SECLVCRT_CDATA_DATA_SIZE]={0xD3, 0x01};

#define SCP10_GENKEYPAIR_CDATA_DATA_SIZE			0x08
const u8 SCP10_GENKEYPAIR_CDATA_DATA_VALUE[SCP10_GENKEYPAIR_CDATA_DATA_SIZE] = {0xC0,0x02,0x74,0x03,0xC2,0x02,0x74,0x02};

#define SCP10_MANA_SECURITY_CDATA_DATA_SIZE			0x03
const u8 SCP10_MANA_SECURITY_CDATA_DATA_VALUE[SCP10_MANA_SECURITY_CDATA_DATA_SIZE] = {0x80,0x02,0x10};

#define		RSA_PRIVATE_KEY_P_LEN		0x40
#define		RSA_PRIVATE_KEY_Q_LEN		0x40
#define		RSA_PRIVATE_KEY_DP_LEN		0x40
#define		RSA_PRIVATE_KEY_DQ_LEN		0x40
#define		RSA_PRIVATE_KEY_QINV_LEN	0x40
#define		RSA_PUBLIC_KEY_N_LEN		0x80
#define		RSA_PUBLIC_KEY_E_LEN		0x03
#define		RSA_PUBLIC_KEY_D_LEN		0x80
const u8 RSA_PUBKEY_E[RSA_PUBLIC_KEY_E_LEN]={0x01, 0x00, 0x01};
/*
extern const u8 SCP10_DEAL_SECLVCRT_CDATA_DATA_VALUE[SCP10_DEAL_SECLVCRT_CDATA_DATA_SIZE];

extern const u8 SCP10_GENKEYPAIR_CDATA_DATA_VALUE[SCP10_GENKEYPAIR_CDATA_DATA_SIZE];

extern const u8 SCP10_MANA_SECURITY_CDATA_DATA_VALUE[SCP10_MANA_SECURITY_CDATA_DATA_SIZE];
*/
/**
 * @param apdu
 */
u16 scp10_process()
{
	u8 		XDATA_AREA		bINS_;
	u16		XDATA_AREA		resLen;
	PN8				sdDataObj;

	jref	XDATA_AREA		sdRef;
	jref	XDATA_AREA		sdDataRef;
	memref	XDATA_AREA		sdEntryAddr;

	boolean XDATA_AREA		processSecurityFlag;
	PN8				pushArray;
	PX8				TransientDataBufForSCP10;
	PX8				tempBufForSCP10;

//	sdDataObj = getCurrentSdDataObj(IS_NEED_CHECK_SD_LOCKED);
	sdEntryAddr = getEntryByAppId(g_bSDId[selected_channel]);

	sdRef = PX_R_U16(get_object_field_ref(sdEntryAddr, theApplet_INDEX));
	sdDataRef = sdData_getSDdataRef(sdRef);
	sdDataObj = (PN8)locateObjectInMemory(sdDataRef);
	//g_current_SCP_Type = sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP10_WITHOUT_IOPTION_MODE, 0x00);
	g_current_SCP_Type = sdData_getScpType(sdDataObj);
	g_current_SCP_I_Option = sdData_getScpOption(sdDataObj);

	if(SCP_10 != g_current_SCP_Type)
	{
		g_current_SCP_Type = sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP10_WITHOUT_IOPTION_MODE, 0x00);
		if(SCP_10 != g_current_SCP_Type)
			throw_error(SW_INS_NOT_SUPPORTED);
	}

	bINS_ = APDUBuffer[OFFSET_INS];
	check_p1p2Lc_param(bINS_);

	processSecurityFlag = (boolean) popShort();
	TransientDataBufForSCP10 = (PX8) (RAM_ref_to_pointer(get_array_data((u8 *)locateObjectInMemory(popReference()))));
	tempBufForSCP10 = (PX8) (RAM_ref_to_pointer(get_array_data((u8 *)locateObjectInMemory(popReference()))));
	pushArray = (PX8) (RAM_ref_to_pointer(get_array_data((u8 *)locateObjectInMemory(popReference()))));
	resLen = 0x00;

	switch (bINS_)
	{
	case INS_OP_MANAGE_SECURITY_ENVIRONMENT:
		scp10_manageSecurityEnvironment(tempBufForSCP10);
		break;
	case INS_OP_PERFORM_SECURITY_OPERATION:
		if(0x00 == APDUBuffer[OFFSET_P1])
		{			
			scp10_performVerifyCertificate(tempBufForSCP10);
		}
		else
		{
			scp10_performDecipher(TransientDataBufForSCP10, tempBufForSCP10);
		}
		break;
	case INS_OP_GET_CHALLENGE:
		resLen = scp10_getChallenge(tempBufForSCP10);
		break;
	case INS_OP_EXTERNAL_AUTHENTICATION:
		scp10_externalAuthenticate(TransientDataBufForSCP10, tempBufForSCP10, pushArray);
		break;
	case INS_OP_INTERNAL_AUTHENTICATION:
		resLen = scp10_internalAuthenticate(TransientDataBufForSCP10, tempBufForSCP10, processSecurityFlag);
		break;
	//case INS_OP_GET_DATA:
		//resLen = (u16)(getData(APDUBuffer)&0x00FF);
		//break;
	//case INS_OP_GENERATE_KEYPAIR:
		//scp10_generateKeyPair();
		//break;
	default:
		throw_error(SW_INS_NOT_SUPPORTED);
		break;
	}
	return resLen;
}


void clearLevelThrowError(PX8 tempBufForSCP10, u16 err)
{
	managerChannelLevel(SCF_NO_AUTHENTICATED);
	tempBufForSCP10[OFFSET_STATE_MACHINE] = INIT_STATE;
	throw_error(err);
}

/**
 *管理SCP10的安全级别 @param apdu
 */
void scp10_manageSecurityEnvironment(PX8	tempBufForSCP10)
{
	PX8			ptr;
	u8		XDATA_AREA	iOption;
	PN8				sdDataObj;

	managerChannelLevel(SCF_NO_AUTHENTICATED);
	tempBufForSCP10[OFFSET_STATE_MACHINE] = MANAGE_SEC_ENV_STATE;
	tempBufForSCP10[OFFSET_REG_APID] = REG_APID_NO_MATCH;

	ptr = PX8_A_U16(APDUBuffer ,OFFSET_CDATA);

	if(0x00 != MEM_arrayCompare((memref)SCP10_MANA_SECURITY_CDATA_DATA_VALUE, (memref)ptr, SCP10_MANA_SECURITY_CDATA_DATA_SIZE))
	{
		throw_error(SW_WRONG_DATA);
	}

	// offset to I_option
	ptr = PX8_A_U16(ptr, SCP10_MANA_SECURITY_CDATA_DATA_SIZE);
	iOption = PX_R_U8(ptr);
	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);
	if(g_current_SCP_I_Option != PX_R_U8(ptr) )
	{
		g_current_SCP_Type = sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP10_WITH_IOPTION_MODE, PX_R_U8(ptr));
		if(0xFF == g_current_SCP_Type)
		{
					throw_error(SW_WRONG_DATA);
		}
		g_current_SCP_I_Option = sdData_getScpOption(sdDataObj);
	}

	if( (0x81 == APDUBuffer[OFFSET_P1]) && (0x01 == iOption) )
	{
		throw_error(SW_WRONG_DATA);
	}

}



/**
 * 恢复公钥证书保存公钥证书@param apdu
 */

void scp10_performVerifyCertificate(PX8	tempBufForSCP10)
{
	u8 		XDATA_AREA		recoverResult;
	PN8				sdDataObj;
	PN8				keySetObj;

	if(tempBufForSCP10[OFFSET_STATE_MACHINE] != MANAGE_SEC_ENV_STATE)
	{
		clearLevelThrowError(tempBufForSCP10, SW_CONDITIONS_NOT_SATISFIED);
	}
	if( (0x7F != APDUBuffer[OFFSET_CDATA]) && (0x21 != APDUBuffer[OFFSET_CDATA_ADD_CONST_1]) )
	{
		throw_error(SW_WRONG_DATA);
	}
	managerChannelLevel(SCF_NO_AUTHENTICATED);
	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);
	//sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	recoverResult = scp10_recoverKey(keySetObj, APDUBuffer, OFFSET_CDATA, APDUBuffer, OFFSET_CDATA);
	if(MR_VERIFY_SUCCESS != recoverResult )
	{
		throw_error(SW_WRONG_DATA);
	}
	tempBufForSCP10[OFFSET_STATE_MACHINE] = GET_VERIFY_CERT_STATE;
}
/*
u8 getValueOffByTag(MEM_U8_P inBuffer, u8 currentLenOff)
{
	u16 	XDATA_AREA		currentLenSize;
	u8 		XDATA_AREA		sizeOfTagLength;
	u8 		XDATA_AREA		currentValueOff;
	PX8				tempPtr;

	tempPtr = PX8_A_U16(inBuffer, currentLenOff);
	currentLenSize = (u8)PX_R_U8(tempPtr);

	switch(currentLenSize)
	{
		case TAG_82:
			sizeOfTagLength = SIZE_LENGTH_OF_82_TAG;
			break;
		case TAG_81:
			sizeOfTagLength = SIZE_LENGTH_OF_81_TAG;
			break;
		default:
			sizeOfTagLength = SIZE_LENGTH_OF_OTHER_TAG;
			break;
	}
	currentValueOff = U8_A_U8(currentLenOff ,sizeOfTagLength);
	return currentValueOff;
}


u16 getLenByTag(MEM_U8_P inBuff, u8 inOffset)
{
	u8 			XDATA_AREA		bTag;
	u16 		XDATA_AREA		bLength;
	MEM_U8_P 					tempPtr;
	tempPtr = PX8_A_U8(inBuff, inOffset);
	bTag = PX_R_U8(tempPtr);
	switch(bTag)
	{
		case TAG_82:
			PX8_INC(tempPtr);
			bLength = RAM_READ_U16(tempPtr);
			break;
		case TAG_81:
			PX8_INC(tempPtr);
			bLength = (u16)PX_R_U8(tempPtr);
			break;
		default:
			bLength = (u16)PX_R_U8(tempPtr);
			break;
	}

	return bLength;
}
*/
/*
 *
 * get the NEXT TagOff by current TagOff.
 *
 *	nextTagOff = currentTagOff + sizeof(currentT)+sizeof(CurrentL) +sizeof(CurrentV)
 *	if not find the tag, return currentTagOff
 *
 * IN:
 * InBuffer	:		data struct is T1L1V1T2L2V2T3L3V3..........
 * currentTagOff:	offSet of TX in TXLXVX
 * tagType:			ONE_BYTE_TAG  or  TWO_BYTE_TAG
 * TagName:			Value of TX
 *
 * OUT:
 * nextTagOff:		offSet of T(X+1) in T(X+1)L(X+1)V(X+1)
 *
 */
/*
u16 getNextTagOff(u8 * inBuffer, u8 currentTagOff, u8 tagType, u16 tagName, u8 IsNeedCheckTagName)
{
	u8 		XDATA_AREA		sizeOfCurrentT;
	u16 	XDATA_AREA		sizeOfCurrentV;
	u16 	XDATA_AREA		nextTagOff;
	u16		XDATA_AREA		currentTagName;
	PX8				tempPtr;

	tempPtr = PX8_A_U16(inBuffer, currentTagOff);

	switch(tagType)
	{
		case ONE_BYTE_TAG:
			currentTagName = PX_R_U8(tempPtr);
			sizeOfCurrentT = SIZE_OF_ONEBYTETAG_TAG;
		break;

		case TWO_BYTE_TAG:
			currentTagName = (u16)RAM_READ_U16(tempPtr);
			sizeOfCurrentT = SIZE_OF_TWOBYTETAG_TAG;
		break;

		default:
		break;
	}

	if(tagName != currentTagName)
	{
		if(IS_NEED_CHECK_TAG_NAME_YES == IsNeedCheckTagName)
		{
			throw_error(SW_WRONG_DATA);
		}
		else
		{
			return currentTagOff;
		}
	}

	tempPtr = PX8_A_U16(tempPtr, sizeOfCurrentT);
	nextTagOff = (u16)U16_ADD_U8(currentTagOff, sizeOfCurrentT);

	sizeOfCurrentV = PX_R_U8(tempPtr);
	switch(sizeOfCurrentV)
	{
		case TAG_82:
			PX8_INC(tempPtr);
			nextTagOff = (u16)U16_A_U8(nextTagOff, SIZE_OF_TWOBYTETAG_TAG);
			sizeOfCurrentV = RAM_READ_U16(tempPtr);
			break;
		case TAG_81:
			PX8_INC(tempPtr);
			nextTagOff = (u16)U16_A_U8(nextTagOff, SIZE_OF_ONEBYTETAG_TAG);
			sizeOfCurrentV = (u16)PX_R_U8(tempPtr);
			break;
		default:
			break;
	}

	// add LEN
	nextTagOff = U16_A_U8(nextTagOff, 0x01);
	nextTagOff = (u16)U16_A_U16(nextTagOff, sizeOfCurrentV);

	return nextTagOff;
}
*/

/**
 * @param inBuff
 * @param inOffset
 * @param outBuff
 * @param outOffset
 * @return
 */
//恢复类似CASD使用的公钥证书，对于X.509证书的处理，没有实现
/*
// 256 - 20 - 1
#define SIZE_OF_RSA_TEMP_BUFFLEN  				235
// 1+ 2 + 20
#define	SIZE_OF_MD_DIGEST_BUFFER_LEN			0x17
// 1+ 63
#define	SIZE_OF_MD_TEMP_BUFFER_LEN				0x40


u8 scp10_recoverKey(PN8 keySetObj, PX8 inBuff, u8 inOffset,PX8 outBuff, u8 outOffset)
{

	boolean XDATA_AREA		result;
	u8 		XDATA_AREA		mdBuff[SIZE_OF_MD_DIGEST_BUFFER_LEN];
	u8 		XDATA_AREA		tmpBuff[SIZE_OF_MD_TEMP_BUFFER_LEN];
	u8 		XDATA_AREA		tempRecoverBuff[SIZE_OF_RSA_TEMP_BUFFLEN];
//	u8 		XDATA_AREA		simulator_mdBuff[SIZE_OF_MD_DIGEST_BUFFER_LEN];
//	u8 		XDATA_AREA		simulator_tmpBuff[SIZE_OF_MD_TEMP_BUFFER_LEN];
//	u8 		XDATA_AREA		simulator_tempRecoverBuff[SIZE_OF_RSA_TEMP_BUFFLEN];

//	PX8		mdBuff = pointer_to_ref(g_ReuseBigBuf);
//	PX8		tmpBuff = (PX8)pointer_to_ref(g_ReuseBigBuf + SIZE_OF_MD_DIGEST_BUFFER_LEN);
//	PX8		tempRecoverBuff = (PX8)pointer_to_ref(g_ReuseBigBuf + SIZE_OF_MD_DIGEST_BUFFER_LEN + SIZE_OF_MD_TEMP_BUFFER_LEN);

	//PKTP_EXAUTRsaSignMR.init(PKTP_EXAUT, Signature.MODE_VERIFY);

	u8	 	XDATA_AREA		tempTagLenOff;
	u8	 	XDATA_AREA		savedFirstTagOff;
	u8	 	XDATA_AREA		savedLastTagOff;

	u8 		XDATA_AREA		signLength;
	u8 		XDATA_AREA		recoverLength;
	u8 		XDATA_AREA		remLength;
	u8 		XDATA_AREA		combineOff;
	u8 		XDATA_AREA		tmpLen;

	PX8				pTempSrc;
	PX8				pTempDes;

	result = false;
	// offset for the length after Tag 7F21.
	tempTagLenOff = U8_A_U8(inOffset, SIZE_OF_TWOBYTETAG_TAG);

	// offset for the Tag 93.
	tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

	// save the first tag off for next used
	savedFirstTagOff = tempTagLenOff;

	// offset for the Tag 42.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_93, IS_NEED_CHECK_TAG_NAME_YES);

	// offset for the Tag 5F20.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_42, IS_NEED_CHECK_TAG_NAME_YES);

	// offset for the Tag 95.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, TWO_BYTE_TAG, TAG_5F20, IS_NEED_CHECK_TAG_NAME_YES);

	// offset for the Tag 5F25.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_95, IS_NEED_CHECK_TAG_NAME_NO);

	// offset for the Tag 5F24.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, TWO_BYTE_TAG, TAG_5F25, IS_NEED_CHECK_TAG_NAME_NO);

	// offset for the Tag 53.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, TWO_BYTE_TAG, TAG_5F24, IS_NEED_CHECK_TAG_NAME_YES);

	// offset for the Tag 5F37.
	tempTagLenOff = getNextTagOff(inBuff, tempTagLenOff, ONE_BYTE_TAG, TAG_53, IS_NEED_CHECK_TAG_NAME_NO);

	// save the first tag off for next used
	savedLastTagOff = tempTagLenOff;

	// offset for the Length 5F37.
	tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_TWOBYTETAG_TAG);

	// get size of Value tag 5F37
	signLength = getLenByTag((MEM_U8_P) inBuff, tempTagLenOff);
	// offset for the Value 5F37.
	tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

//	sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
	//sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
//	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	//bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);
	getRSAKeyRef(KEY_TYPE_RSA_PUBLIC, keySetObj, KEY_ID_01);
//	getRSAKeyRef(PKTP_EXAUT);

	// recover the pub exp and part modulus.
	g_CryptoData.pbBuffer = bTempArr;  // for 2048 bits RSA computation, this buffer should be at least 256 u8s.
	pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);

	recoverLength = msgRecoverAlg((MEM_U8_P)pointer_to_ref(pTempSrc), signLength);
	SHAUpdate(pTempSrc, recoverLength, MD_INIT, (MEM_U8_P)mdBuff, (MEM_U8_P)tmpBuff, ALG_SHA1);
	//recoverLength = PKTP_EXAUTRsaSignMR.beginVerify(inBuff, inOffset, signLength);

	//record the off for next use
	combineOff = tempTagLenOff;

	memCopyAtomic((memref)tempRecoverBuff, (memref)pTempSrc, (u16)(recoverLength+20));
	//arrayCopyNonAtomic(inBuff, inOffset, recoverBuff, (u16)0, (u16)(recoverLength+20));

	// offset for the Tag 5F38 (pub modulus remainder).
	tempTagLenOff = U8_A_U8(tempTagLenOff, signLength);

	 // offset to length of 5F38 (pub modulus remainder).
	tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_TWOBYTETAG_TAG);

	// get size of Value tag 5F38 (pub modulus remainder)
	remLength = getLenByTag((MEM_U8_P) inBuff, tempTagLenOff);

	// offset for the Value 5F38.
	tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

	pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);
	SHAUpdate(pTempSrc, remLength, MD_UPDATE, (MEM_U8_P)mdBuff, (MEM_U8_P)tmpBuff, ALG_SHA1);

	//PKTP_EXAUTRsaSignMR.update(inBuff, remOffset, remLength);
	pTempDes = PX8_A_U8(inBuff, combineOff);
	pTempDes = PX8_A_U8(pTempDes, recoverLength);
	memCopyAtomic( (memref)pointer_to_ref(pTempDes), (memref)pTempSrc, remLength);
	//arrayCopyNonAtomic(inBuff, remOffset, inBuff, combineOff, remLength);

	// verify the data in certificate.
	tmpLen = (u8)U8_S_U8(savedLastTagOff, savedFirstTagOff);
	pTempSrc = PX8_A_U8(inBuff, savedFirstTagOff);
	result = signMRVerifyAlg(pTempSrc, tmpLen, (MEM_U8_P)tempRecoverBuff, recoverLength,  (MEM_U8_P)mdBuff, (MEM_U8_P)tmpBuff, ALG_RSA_SHA_ISO9796_MR);

	if (MR_VERIFY_SUCCESS == result)
	{
//		sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);
//		keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);

		if (REF_NULL == (NUM_TO_NPTR)fm03_locateKeyType(keySetObj, KEY_ID_04))
		{
			tmpLen = 0;
			tmpLen = keyData_computeKeyDataLen(KEY_FORMAT_03, tmpLen, RSA_PUBLIC_KEY_N_LEN, true);
			tmpLen = keyData_computeKeyDataLen(KEY_FORMAT_03, tmpLen, RSA_PUBLIC_KEY_E_LEN, false);
			keyData_addKeyDataSpace(keySetObj, tmpLen);
		}
		pTempDes = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_04, RSA_E_DGI_TYPE , RSA_PUBLIC_KEY_E_LEN);

		// offset for the length after Tag 7F49.
		tempTagLenOff = U8_A_U8(combineOff, SIZE_OF_TWOBYTETAG_TAG);

		// offset for the Tag 82 (pub exp).
		tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

		// offset for the Length 82(pub exp).
		tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_ONEBYTETAG_TAG);

		// get size of Value tag 82(pub exp)
		tmpLen = getLenByTag((MEM_U8_P) inBuff, tempTagLenOff);

		// offset for the Value 82(pub exp).
		tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

		pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);

		if(0x01 == tmpLen)
		{
			pTempDes += 2;
		}
		nvmCopyAtomic( (memref)pTempDes, (memref)pTempSrc, tmpLen);
		
		outBuff[outOffset] = tmpLen;
		outOffset ++;
		memCopyAtomic( (memref)&outBuff[outOffset], (memref)pTempSrc, tmpLen);
		outOffset += tmpLen;

		// offset for the Tag 81 (pub Mode).
		tempTagLenOff = U8_A_U8(tempTagLenOff, tmpLen);

		// offset for the Length 81 (pub Mode).
		tempTagLenOff = U8_A_U8(tempTagLenOff, SIZE_OF_ONEBYTETAG_TAG);

		// get size of Value tag 81 (pub Mode).
		tmpLen = getLenByTag((MEM_U8_P) inBuff, tempTagLenOff);

		// offset for the Value 81 (pub Mode).
		tempTagLenOff = getValueOffByTag(inBuff, tempTagLenOff);

		pTempSrc = PX8_A_U8(inBuff, tempTagLenOff);

		pTempDes = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_04, RSA_N_DGI_TYPE , RSA_PUBLIC_KEY_N_LEN);

		nvmCopyAtomic((memref)pTempDes, (memref)pTempSrc, tmpLen);
		outBuff[outOffset] = tmpLen;
		outOffset ++;
		memCopyAtomic( (memref)&outBuff[outOffset], (memref)pTempSrc, tmpLen);

	}
	return result;
}
*/


void scp10_singleKeyParam(PX8 sessionKeyFlag, PX8 CNT1, PX8 CNT2, PX8 tempBuff ,u8 bitValue, u8 value1, u8 value2, u8 temOffset, u8 needSetBit)
{
	u8 		XDATA_AREA		tempCont1;
	u8 		XDATA_AREA		tempCont2;
	tempCont1 = PX_R_U8(CNT1);

	sessionKeyFlag[0] |= bitValue;
	if(0x00 != (tempCont1 & bitValue))
	{
		throw_error(SW_WRONG_DATA);
	}

	tempCont1 |= bitValue;
	PX_W_U8(CNT1, tempCont1);

	tempCont2 = PX_R_U8(CNT2);
	if(0x00 == (tempCont2 & needSetBit))
	{
		tempBuff[temOffset] = value1;
		tempCont2 |= bitValue;
		PX_W_U8(CNT2, tempCont2);
	}
	else
	{
		tempBuff[temOffset] = value2;
	}
}

void scp10_doubleKeyParam(PX8 sessionKeyFlag, PX8 CNT, PX8 tempBuff ,u8 bitValue, u8 value, u8 optionValue, u8 temOffset1, u8 temOffset2)
{
	u8 		XDATA_AREA		tempCont;
	tempCont = PX_R_U8(CNT);

	sessionKeyFlag[0] |= bitValue;
	if(0x00 != (tempCont & bitValue))
	{
		throw_error(SW_WRONG_DATA);
	}

	tempCont |= bitValue;
	PX_W_U8(CNT, tempCont);

	if(optionValue == 0x01)
	{
		tempBuff[temOffset1]=value;
		tempBuff[temOffset2]=value;
	}
}

void scp10_dealSecLvlCRTData(PX8 dataBuf, u8 dataBufOff, u8 dataBufLen, u8 optionValue, PX8	tempBufForSCP10)
{
	u16 	XDATA_AREA		tempCRTLen;
	u16 	XDATA_AREA		tempI;
	u16 	XDATA_AREA		tempLen;
	u16 	XDATA_AREA		sessionKeyLen;
	u16 	XDATA_AREA		Tag91Off;
	u8 		XDATA_AREA		secLvl;
	u16		XDATA_AREA		tempTagOff;
	u8 		XDATA_AREA		tempTag;
	u8 		XDATA_AREA		keyUsageQua;
	u8		XDATA_AREA		sessionKeyFlag;

	u8 		XDATA_AREA		tempKeyCNT;
	u8 		XDATA_AREA		tempICNT;
	u8 		XDATA_AREA      CRTLen_in;
	PX8				tempPtr;

	tempKeyCNT = 0x00;
	tempICNT = 0x00;

	tempTagOff = dataBufOff;

	tempPtr = PX8_A_U8(dataBuf, dataBufOff);

	if(0x00 != MEM_arrayCompare((memref)SCP10_DEAL_SECLVCRT_CDATA_DATA_VALUE, (memref)tempPtr, SCP10_DEAL_SECLVCRT_CDATA_DATA_SIZE))
	{
		throw_error(SW_WRONG_DATA);
	}

	tempPtr = PX8_A_U16(tempPtr, SCP10_DEAL_SECLVCRT_CDATA_DATA_SIZE);

	//获取安全级别
	secLvl = PX_R_U8(tempPtr);
	if(0x00 != (secLvl & 0xC0))
	{
		throw_error(SW_WRONG_DATA);
	}


	tempLen = dataBufLen;
	if(0x02 == optionValue)//transport模式
	{
		tempLen = U16_S_U8(tempLen,0x03);
	}
	if(0x01 == optionValue)//agreement模式
	{
		tempLen = U16_S_U8(tempLen,0x23);
	}

	for(tempI=0; tempI<tempLen ; )
	{
		sessionKeyFlag = 0x00;
		tempTagOff = U8_A_U8(tempI, 0x03);
		tempPtr = PX8_A_U8(dataBuf, tempTagOff);
		tempTag = PX_R_U8(tempPtr);

		PX8_INC(tempPtr);
		tempCRTLen = PX_R_U8(tempPtr);

		PX8_INC(tempPtr);
		tempTagOff = U16_A_U8(tempTagOff, 0x02);
		tempTagOff = scp10_searchTag(dataBuf, (u16)tempTagOff, tempCRTLen, KEY_USAGE_QUA);

		if( 0xffff == tempTagOff )
		{
			throw_error(SW_WRONG_DATA);
		}

		U16_INC(tempTagOff);
		tempPtr = PX8_A_U8(dataBuf, tempTagOff);
		if( 0x01 != PX_R_U8(tempPtr) )
		{
			throw_error(SW_WRONG_DATA);
		}

		PX8_INC(tempPtr);
		keyUsageQua = PX_R_U8(tempPtr);//获取密钥使用属性
		if( CCT_TAG == tempTag )
		{
			switch(keyUsageQua)
			{
				case (u8)0x10://C-MAC
					scp10_singleKeyParam(&sessionKeyFlag, &tempKeyCNT, &tempICNT, tempBufForSCP10 ,BIT_1, 0x01, 0x03, OFFSET_CMAC_CNT, (u8)( BIT_1 | BIT_2 ));
					break;
				case (u8)0x20://R-MAC
					scp10_singleKeyParam(&sessionKeyFlag, &tempKeyCNT, &tempICNT, tempBufForSCP10 ,BIT_2, 0x01, 0x03, OFFSET_RMAC_CNT, (u8)( BIT_1 | BIT_2 ));
					break;
				case (u8)0x30://CR-MAC
					scp10_doubleKeyParam(&sessionKeyFlag, &tempKeyCNT, tempBufForSCP10 ,(u8)( BIT_1 | BIT_2 | BIT_7) , 0x01, optionValue, OFFSET_CMAC_CNT, OFFSET_RMAC_CNT);
					break;
				default:
					throw_error(SW_WRONG_DATA);
					break;
			}
		}
		else if(CT_TAG == tempTag)
		{
			switch(keyUsageQua)
			{
				case (u8)0x10://C-ENC
					scp10_singleKeyParam(&sessionKeyFlag, &tempKeyCNT, &tempICNT, tempBufForSCP10 ,BIT_5, 0x02, 0x04, OFFSET_CENC_CNT, (u8)( BIT_5 | BIT_6 ));
					break;
				case (u8)0x20://R-ENC
					scp10_singleKeyParam(&sessionKeyFlag, &tempKeyCNT, &tempICNT, tempBufForSCP10 ,BIT_6, 0x02, 0x04, OFFSET_RENC_CNT, (u8)( BIT_5 | BIT_6 ));
					break;
				case (u8)0x30://CR-ENC
					scp10_doubleKeyParam(&sessionKeyFlag, &tempKeyCNT, tempBufForSCP10 ,(u8)( BIT_5 | BIT_6 ) , 0x02, optionValue, OFFSET_CENC_CNT, OFFSET_RENC_CNT);
					break;
				case (u8)0x40://R-DEK
					scp10_singleKeyParam(&sessionKeyFlag, &tempKeyCNT, &tempICNT, tempBufForSCP10 ,BIT_3, 0x05, 0x06, OFFSET_RDEK_CNT, (u8)( BIT_3 | BIT_4 ));
					break;
				case (u8)0x80://C-DEK
					scp10_singleKeyParam(&sessionKeyFlag, &tempKeyCNT, &tempICNT, tempBufForSCP10 ,BIT_4, 0x05, 0x06, OFFSET_CDEK_CNT, (u8)( BIT_3 | BIT_4 ));
					break;
				case (u8)0xC0://CR-DEK
					scp10_doubleKeyParam(&sessionKeyFlag, &tempKeyCNT, tempBufForSCP10 ,(u8)( BIT_3 | BIT_4 ) , 0x05, optionValue, OFFSET_CDEK_CNT, OFFSET_RDEK_CNT);
					break;
				default:
					throw_error(SW_WRONG_DATA);
					break;
			}
		}
		else
		{
			throw_error(SW_WRONG_DATA);
		}

		//if(option == 0x01)
		tempBufForSCP10[OFFSET_KEY_FLAG] |= sessionKeyFlag;

		//判断0xD1
		tempTagOff = scp10_searchTag(dataBuf, (u16)(U8_A_U8(tempI, 0x05)), tempCRTLen, SESSION_KEY_TAG);
		if( (0x02 == optionValue) && (0xffff == tempTagOff) )//transport
		{
			throw_error(SW_WRONG_DATA);
		}
		if( (0x01 == optionValue) && (0xffff != tempTagOff) )//agreement
		{
			throw_error(SW_WRONG_DATA);
		}

		//判断0x91
		Tag91Off = scp10_searchTag(dataBuf, (u16)(U8_A_U8(tempI, 0x05)), tempCRTLen, INI_SEQ_CNT);

		switch(optionValue)
		{
			case 0x01:
				if((0xffff != Tag91Off))
				{
					throw_error(SW_WRONG_DATA);
				}
			break;
			case 0x02:
				if((0x00 != (sessionKeyFlag & 0x3C))&& (0xffff != Tag91Off))
				{
					throw_error(SW_WRONG_DATA);
				}
				if((0x00!=(sessionKeyFlag & 0x03))&& (0xffff == Tag91Off))
				{
					throw_error(SW_WRONG_DATA);
				}
				sessionKeyLen = dataBuf[(u16)(U8_A_U8(tempTagOff,0x01))];
				scp10_proSesKeysCRT(dataBuf, (u16)(U8_A_U8(tempTagOff, 0x02)), sessionKeyLen, sessionKeyFlag, optionValue);
			break;
		}

		CRTLen_in = U8_A_U8(tempCRTLen,0x02);
		tempI = U8_A_U8(tempI,CRTLen_in);
	}

}


/**
 * @param apduBuf
 * @param offTag Tag的偏移
 * @param LenTag Tag的总长度
 * @param tag value which is searched
 * return the searched tag offset in buf
 */
u16 scp10_searchTag(MEM_U8_P inBuffer, u8 offTag, u8 LenTag, u16 Tag)
{
	u8 		XDATA_AREA		tempI;
	u8 		XDATA_AREA		tempTLVLen;
	u16 	XDATA_AREA		tempTag;
	PX8				tempPtr;
	PX8				inPtr;

	inPtr = PX8_A_U8(inBuffer ,offTag);
	for(tempI = 0; tempI< LenTag;  )//循环处理tag
	{
		tempPtr = PX8_A_U8(inPtr ,tempI);
		tempTag = PX_R_U8(tempPtr);
		if( 0x1F == (tempTag & 0x1F) )//两个字节的Tag
		{
			tempTag = PX_R_U16(tempPtr);
			tempPtr = PX8_A_U16(tempPtr, 0x02);
		}
		else//一个字节的Tag
		{
			tempPtr = PX8_A_U16(tempPtr, 0x01);
		}

		if( Tag == tempTag )//找到对应的tag
		{
			return (u16)(U8_A_U8(offTag, tempI));
		}

		tempTLVLen = PX_R_U8(tempPtr);
		if((u8)0x81 == tempTLVLen)//长度为2个字节 81 XX
		{
			tempPtr = PX8_A_U16(tempPtr, 0x01);
			tempTLVLen = PX_R_U8(tempPtr);
			tempTLVLen = U8_A_U8(tempTLVLen, 0x03);
		}
		else//长度为1个字节
		{
			tempTLVLen = U8_A_U8(tempTLVLen, 0x02);
		}
		tempI =  U8_A_U8(tempI, tempTLVLen);
	}
	return (u16)( 0xffff) ;
}

void scp10_genICVSCP10(u8 option , PX8 SrcAddr, PX8 DesAddr , u8 length)
{
	u8 		XDATA_AREA		length_in;
	if(0x02 == option)//transport模式
	{
		length_in = U8_A_U8(length,0x02);
		SrcAddr = PX8_A_U8(SrcAddr , length_in);//SrcAddr = SrcAddr+length;

		ecpMem_Copy(DesAddr, SrcAddr, 8);
	}

	if(0x01 == option && 0x08 == length )//agreement模式
	{
		ecpMem_Copy(DesAddr, SrcAddr, length);//ICV
	}
}



//设置会话密钥及ICV
void scp10_proSesKeysCRT(PX8 buffer, u16 offset, u8 length,  u8 flag, u8 option)
{
	PX8		tempPtr;


	sesKeyFlag |= flag;//设置会话密钥标志
	tempPtr = (PX8)PX8_A_U8(buffer,offset);

	if(BIT_1 == (flag & BIT_1) )//C-MAC
	{
		if(0x10==length)
		{
			generateCMACKey(NULL, tempPtr);
		}

		scp10_genICVSCP10(option , tempPtr, g_current_SCP_cardChallenge, length);
	}

	if(BIT_2 == (flag & BIT_2) )//R-MAC
	{
		if(0x10==length)
		{
			generateRMACKey(NULL, tempPtr);
		}

		scp10_genICVSCP10(option , tempPtr, g_current_SCP_hostChallenge, length);
	}

	if(BIT_3 == (flag & BIT_3) )//R-DEK
	{
		generateRDECKey(NULL, tempPtr);
	}

	if(BIT_4 == (flag & BIT_4) )//C-DEK
	{
		generateCDECKey(NULL, tempPtr);
	}

	if(BIT_5 == (flag & BIT_5) )//C-ENC
	{
		generateCENCKey(NULL, tempPtr);
	}

	if(BIT_6 == (flag & BIT_6) )//R-ENC
	{
		generateRENCKey(NULL, tempPtr);
	}

}

/**
 * @param apdu
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 */
void scp10_performDecipher(PX8 TransientDataBufForSCP10, PX8	tempBufForSCP10)
{
	u16 	XDATA_AREA		retLength;
	u16 	XDATA_AREA		Len;
	u8 		XDATA_AREA		optionValue;

	u8		XDATA_AREA		bKeyType;
	PN8				sdDataObj;
	PN8				keySetObj;
	PN8				tempPtr;

	optionValue =  g_current_SCP_I_Option;

	if(0x02 != optionValue)
	{
		throw_error(SW_INS_NOT_SUPPORTED);
	}
	if(tempBufForSCP10[OFFSET_STATE_MACHINE] != GET_VERIFY_CERT_STATE)
	{
		clearLevelThrowError(tempBufForSCP10, SW_CONDITIONS_NOT_SATISFIED);
	}
	retLength = 0;

	//PKAPCTRsaCipher.init(SKSDAUTCRT, Cipher.MODE_DECRYPT);
	//retLength = PKAPCTRsaCipher.doFinal(apduBuf, ISO7816.OFFSET_CDATA, APDUBuffer[OFFSET_LC], TransientDataBuf, (u16)0);

	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);
	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_02,THROW_ERROR);

	tempPtr = PX8_A_U8(APDUBuffer , OFFSET_CDATA);
	retLength = nativeRSAChiperDoFinalCal(keySetObj, bKeyType, KEY_ID_02, (memref)tempPtr, APDUBuffer[OFFSET_LC], (memref)pointer_to_ref(TransientDataBufForSCP10), retLength, ALG_RSA_SHA_PKCS1, MODE_DECRYPT);
	//retLength = SimpleRSADoFinalCal((u8 *) &APDUBuffer[OFFSET_CDATA], APDUBuffer[OFFSET_LC], TransientDataBufForSCP10, retLength, SKSDAUTCRT, MODE_DECRYPT);
	tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH] = (u8)retLength;

	if( 0x00 == retLength )
	{
		throw_error(SW_WRONG_DATA);
	}

	//判断解密数据是否正确，并查找和设置session keys和icv
	Len  = (u16)(tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH] & 0x00FF);

	scp10_dealSecLvlCRTData(TransientDataBufForSCP10, (u16)0, Len, optionValue, tempBufForSCP10);
	tempBufForSCP10[OFFSET_STATE_MACHINE] = PERFORM_DECIPHER_STATE;
}

/**
 * @param apdu
 */
u16 scp10_getChallenge(PX8	tempBufForSCP10)
{
	u16 	XDATA_AREA		randLen;

	randLen = (u16)(g_current_SCP_I_Option << 3);
	Random_number_generator(&APDUBuffer[OFFSET_CDATA], randLen, 0x00 );

	if((g_current_SCP_I_Option == AGREEMENT_OPTION) && 
	   (tempBufForSCP10[OFFSET_STATE_MACHINE] != GET_VERIFY_CERT_STATE) )
	{
		clearLevelThrowError(tempBufForSCP10, SW_CONDITIONS_NOT_SATISFIED);
	}
	if((g_current_SCP_I_Option == TRANSPORT_OPTION) && 
	   (tempBufForSCP10[OFFSET_STATE_MACHINE] != PERFORM_DECIPHER_STATE) )
	{
		clearLevelThrowError(tempBufForSCP10, SW_CONDITIONS_NOT_SATISFIED);
	}
	
	tempBufForSCP10[OFFSET_STATE_MACHINE] = GET_CHALLENGE_STATE;
	//根据不同模式，保存8/16字节卡片随机数到TransientData最后面
	//memCopyNoAtomic( (memref) &tempBufForSCP10[OFFSET_CHALLENGE], (memref)&APDUBuffer[OFFSET_CDATA], randLen);
	ecpMem_Copy( (PX8) &tempBufForSCP10[OFFSET_CHALLENGE], (PX8)&APDUBuffer[OFFSET_CDATA], randLen);
	//Util.arrayCopyNonAtomic(apduBuf, (u16)5, tempBuff, OPConstants.OFFSET_CHALLENGE, randLen);

//	if(!sesKeyFlag)
//	{
		apdu_ApduSetOutgoingAndSend((u16) 5, randLen);
//	}
	return randLen;
}




/**
 * @param apdu
 */
void scp10_externalAuthenticate(PX8 TransientDataBufForSCP10, PX8	tempBufForSCP10, PN8 pushArray)
{
	u8 		XDATA_AREA		optionValue;
	u8 		XDATA_AREA		tempLc;

	tempLc = APDUBuffer[OFFSET_LC];

	optionValue =  g_current_SCP_I_Option;

	if(GET_CHALLENGE_STATE != tempBufForSCP10[OFFSET_STATE_MACHINE])
	{
		clearLevelThrowError(tempBufForSCP10, SW_CONDITIONS_NOT_SATISFIED);
	}

	//agreement模式
	if(0x01 == optionValue)
	{
		scp10_agreementExternalAuthenticate(APDUBuffer, OFFSET_CDATA, tempLc, TransientDataBufForSCP10, tempBufForSCP10, pushArray);
	}

	//transport模式
	if(0x02 == optionValue)
	{
		scp10_transportExternalAuthenticate(APDUBuffer, OFFSET_CDATA, tempLc, TransientDataBufForSCP10, tempBufForSCP10);
	}

	tempBufForSCP10[OFFSET_STATE_MACHINE] = EXTERNAL_AUT_STATE;
}


/**
 * @param apdu
 */
u16 scp10_internalAuthenticate(PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10, boolean processSecurityFlag)
{

	u8 		XDATA_AREA		optionValue;
	u16 	XDATA_AREA		resLen;
	u16 	XDATA_AREA		tempLc;
	u8 		XDATA_AREA		secLvl;

	resLen = 0x00;

	if(EXTERNAL_AUT_STATE != tempBufForSCP10[OFFSET_STATE_MACHINE])
	{
		clearLevelThrowError(tempBufForSCP10, SW_CONDITIONS_NOT_SATISFIED);
	}

	optionValue =  g_current_SCP_I_Option;
	tempLc = (u16)(APDUBuffer[OFFSET_LC] & 0x00FF);
	if( 0x01 == optionValue)//agreement模式
	{
		if( 0x10 != tempLc )
		{
			throw_error(SW_WRONG_LENGTH);
		}
		resLen = scp10_agreementInternalAuthenticate(TransientDataBufForSCP10, tempBufForSCP10);
	}
	if(0x02 == optionValue )//transport模式
	{
		if(0x18 != tempLc)
		{
			clearLevelThrowError(tempBufForSCP10, SW_WRONG_LENGTH);
		}
		resLen = scp10_transportInternalAuthenticate(TransientDataBufForSCP10, tempBufForSCP10);
	}

	secLvl = TransientDataBufForSCP10[2];
	//建立安全通道标志
	secLvl |= 0x04;

	if(REG_APID_MATCH == tempBufForSCP10[OFFSET_REG_APID] )
	{
		secLvl |= 0x40;
	}
	else
	{
		secLvl |= 0x80;
	}

	if(RESULT_NOK == ManageSecurityLevel(secLvl))
	{
		throw_error(SW_WRONG_DATA);
	}

	if(!processSecurityFlag)
	{
		apdu_ApduSetOutgoingAndSend(OFFSET_CDATA, resLen);
	}

	managerRequestedChannelLevel(channelInfo.security_level[selected_channel]);
	tempBufForSCP10[OFFSET_STATE_MACHINE] = INTERNAL_AUT_STATE;
	return resLen;
}


/**
 * @param inBuff
 * @param inOffset
 * @param inLength
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 */
void scp10_agreementExternalAuthenticate(PX8 inBuff, u8 inOffset, u8 inLength, PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10, PN8 pushArray)
{
#define 		CHALLEGE_LENGTH		0x08
#define 		CARD_ID_LENGTH		0x08
	u8 		XDATA_AREA		sOff;
	u8 		XDATA_AREA		tempI;
	u8 		XDATA_AREA		tempJ;
	u8 		XDATA_AREA		tagLen;
	u8 		XDATA_AREA		hashOff;
	u8		XDATA_AREA		bKeyType;
	u8 		XDATA_AREA		bkeySize;
	u8 		XDATA_AREA		resultLength;
	u8 		XDATA_AREA		endPaddingOff;
	u8 		XDATA_AREA		randPaddingOff;
	u32						keyAddress;

	PN8				sdDataObj;
	PN8				keySetObj;

	PX8				tempPtrIn;
	PX8				tempSrcPtr;
	PX8				tempDesPtr;


	//pushArray =

	//解密加密的签名数据
	//RSAObject.init(SKSDAUTCRT, Cipher.MODE_DECRYPT);
	//RSAObject.doFinal(inBuff, inOffset, inLength, inBuff, inOffset);

	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);
	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_02,THROW_ERROR);

	tempPtrIn = PX8_A_U8(inBuff , inOffset);
	resultLength = nativeRSAChiperDoFinalCal(keySetObj, bKeyType, KEY_ID_02, (memref)pointer_to_ref(tempPtrIn), inLength, (memref)pointer_to_ref(tempPtrIn), 0x00, ALG_RSA_NOPAD, MODE_DECRYPT);
	//length = SimpleRSADoFinalCal((u8 *) &inBuff[inOffset], inLength, &inBuff[inOffset], length, SKSDAUTCRT, MODE_DECRYPT);

	//加密签名数据
	//RSAObject.init(PKOCEAUT, Cipher.MODE_ENCRYPT);
	//length = RSAObject.doFinal(inBuff, inOffset, inLength, inBuff, inOffset);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_04,THROW_ERROR);
	resultLength = nativeRSAChiperDoFinalCal(keySetObj, bKeyType, KEY_ID_04, (memref)pointer_to_ref(tempPtrIn), resultLength, (memref)pointer_to_ref(tempPtrIn), 0x00, ALG_RSA_NOPAD, MODE_ENCRYPT);
//	length = SimpleRSADoFinalCal((u8 *) &inBuff[inOffset], inLength, &inBuff[inOffset], length, PKOCEAUT, MODE_ENCRYPT);


	//获取N.PK.OCE.AUT公钥值
	//need modify   20131111
	tempDesPtr = PX8_A_U8(tempPtrIn , inLength);
	bkeySize = keyData_getKeyDataById(keySetObj, KEY_ID_04, RSA_N_DGI_TYPE, (RAM_U32_P)&keyAddress);
	memCopyToRam((memref)tempDesPtr, (memref)keyAddress, bkeySize);
//	PKOCEAUT.getModulus(inBuff, (short)(inOffset+inLength));
	//处理待验证的数据，是否需要进行modulus - V
	//need modify   20131111
	signAndVerifyProcessISO9797_2(tempPtrIn, tempDesPtr, inLength, VERIFY_PROCESS);

	//判断head padding 0x6A
	if((u8)0x6A != PX_R_U8(tempPtrIn))
	{
		throw_error(SW_AUTHENTICATION_FAILED_WARNING);
	}

	endPaddingOff = U8_S_U8(resultLength, 0x01);
	//判断end padding 0xBC
	tempSrcPtr = PX8_A_U8(tempPtrIn , endPaddingOff);
	if((u8)0xBC != PX_R_U8(tempSrcPtr))
	{
		throw_error(SW_AUTHENTICATION_FAILED_WARNING);
	}

	hashOff = (u8)U8_S_U8(endPaddingOff, SHA1_DIGEST_SIZE);

	//保存hash value
	tempSrcPtr = PX8_A_U8(tempPtrIn , hashOff);
	tempDesPtr = PX8_A_U8(tempSrcPtr, 0x24);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, SHA1_DIGEST_SIZE);
	//Util.arrayCopyNonAtomic(inBuff, hashOff, inBuff, (u16)(hashOff+0x24), (u16)0x14);
	//Util.arrayCopyNonAtomic(inBuff, hashOff, tempBuff, OPConstants.OFFSET_HASH, (u16)0x14);

	//搬移卡片随机数
	tempDesPtr = tempSrcPtr;
	tempSrcPtr = PX8_A_U16(tempBufForSCP10 , OFFSET_CHALLENGE);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, CHALLEGE_LENGTH);
	//Util.arrayCopyNonAtomic(tempBuff, OFFSET_CHALLENGE, inBuff, hashOff, (u16)8);

	tagLen = getLenByTag(pushArray, 3);
	sOff = getValueOffByTag(pushArray, 3);

	sOff = scp10_searchTag(pushArray, sOff, tagLen, SUBJECT_ID);

	//搬移card id
	sOff = (u8)(U8_A_U8(sOff, 0x03));
	tempDesPtr = PX8_A_U16(tempDesPtr , CHALLEGE_LENGTH);
	tempSrcPtr = PX8_A_U8(pushArray , sOff);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, CARD_ID_LENGTH);
	//arrayCopyNonAtomic(pushArray, sOff, inBuff, (u16)(hashOff+8), (u16)8);

	//计算hash数据的长度
	sOff = (u8)(U8_S_U8(resultLength, 0x06));//sOff = (u16)(length-22+16);
	//计算hash值
	tempDesPtr = PX8_A_U16(tempDesPtr , CARD_ID_LENGTH);
	tempSrcPtr = PX8_A_U8(tempPtrIn , 0x01);
	HASHDoFinal((PN8)tempSrcPtr, sOff, (PN8)pointer_to_ref(tempDesPtr), MD_INIT, (MEM_U8_P)NULL, (MEM_U8_P)NULL, ALG_SHA1);
	//doFinal(inBuff, (u16)(inOffset+1), sOff, inBuff, (u16)(hashOff+16));
	//比较hash值
	//if(Util.arrayCompare(inBuff, (u16)(hashOff+16), tempBuff, OPConstants.OFFSET_HASH, (u16)0x14) != 0)
	tempSrcPtr = PX8_A_U16(tempDesPtr ,SHA1_DIGEST_SIZE);
	if(MEM_arrayCompare( (memref) tempSrcPtr, (memref)tempDesPtr, SHA1_DIGEST_SIZE) != 0)
	{
	//if(arrayCompare(inBuff, (u16)(hashOff+16), inBuff, (u16)(hashOff+0x24), (u16)0x14) != 0)
		throw_error(SW_AUTHENTICATION_FAILED_WARNING);
	}

	//搬移恢复的数据到TransientData[]中,并保存恢复数据长度到tempBuff[OPConstants.OFFSET_KEYS_CRT_LENGTH]中，
	//由于填充随机数没有格式判断,无法去除随机填充的数据，填充数据修改为0xBB
	bkeySize = g_RSAKeyStore.wModulusNLength;

	tempSrcPtr = PX8_A_U8(tempPtrIn , 0x01);
	randPaddingOff = (u16)(U8_A_U8(inOffset,0x01));
	if(0xBB != PX_R_U8(tempSrcPtr))
	{
		throw_error(SW_WRONG_DATA);
	}

	tempJ = (u8)(U16_S_U8(bkeySize,0x36));

	for(tempI = 0; tempI < tempJ; )
	{
		if((u8)0xBB != PX_R_U8(tempSrcPtr))
		{
			break;
		}
		PX8_INC(tempSrcPtr);
		U8_INC(tempI);
	}

//	bkeySize_inn = (u16)(U16_S_U8(bkeySize,0x13));
//	if(tempI > bkeySize_inn)
//	{
//		throw_error(SW_WRONG_DATA);
//	}
	//i个BB
	tempI = (u8)(U8_A_U8(tempI, 0x16));
	bkeySize = (u8)(U16_SUB_U8(bkeySize, tempI));

	tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH] = (u8)bkeySize;

	ecpMem_Copy( TransientDataBufForSCP10, tempSrcPtr, bkeySize);
	//arrayCopyNonAtomic(inBuff, --randPaddingOff, TransientDataBuf, (u16)0, bkeySize);
	//判断恢复数据是否正确，并计算counter
	tempJ  = tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH];
	scp10_dealSecLvlCRTData(TransientDataBufForSCP10, (u16)0, tempJ, g_current_SCP_I_Option, tempBufForSCP10);
}

/**
 * @param inBuff
 * @param inOffset
 * @param inLength
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 */
void scp10_transportExternalAuthenticate(u8 * inBuff, u16 inOffset, u16 inLength, PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10)
{
	u8	 	XDATA_AREA		tempLen ;
	u16 	XDATA_AREA		secLvl;
	u8 		XDATA_AREA		result;

	u8		XDATA_AREA		bKeyType;
	PN8				sdDataObj;
	PN8				keySetObj;

	PX8				tempSrcPtr;
	PX8				tempDesPtr;

	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);

	tempLen = tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH];
	//搬移卡片随机数到keys CRT后面

	tempSrcPtr = PX8_A_U16(tempBufForSCP10 , OFFSET_CHALLENGE);
	tempDesPtr = PX8_A_U8(TransientDataBufForSCP10 , tempLen);
//	memCopyNoAtomic( (memref) tempDesPtr, (memref)tempSrcPtr, CARD_CHALLEGE_ID_LENGTH );
	ecpMem_Copy( tempDesPtr, tempSrcPtr, (u16)CARD_CHALLEGE_ID_LENGTH);
	tempLen += CARD_CHALLEGE_ID_LENGTH;
	//arrayCopyNonAtomic(tempBuff, OFFSET_CHALLENGE, TransientDataBuf, Len, (u16)16);

	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_04,THROW_ERROR);

	tempSrcPtr = TransientDataBufForSCP10;
	tempDesPtr = PX8_A_U8(inBuff , inOffset);
	result = nativeRSASignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_04, (memref)tempSrcPtr, tempLen, (memref)tempDesPtr, inLength, ALG_RSA_SHA_PKCS1, MODE_VERIFY);
	if(false == result)
	{
		throw_error(SW_AUTHENTICATION_FAILED_WARNING);
	}
//	length = SimpleRSADoFinalCal((u8 *) &inBuff[inOffset], inLength, &inBuff[inOffset], length, PKOCEAUT, MODE_VERIFY);
//	if (RSA_PROCESS_OK != RSAVerify((MEM_U8_P)TransientDataBufForSCP10, (u16)(Len+16), NULL, 0, NULL, NULL, PKCS1_SHA1_VERIFICATION, length))
//	{
//		throw_error(SW_AUTHENTICATION_FAILED_WARNING);
//	}
	//PKAPAUTRsaSig.init(PKOCEAUT, Signature.MODE_VERIFY);
	//boolean bSuccess =  PKAPAUTRsaSig.verify(TransientDataBuf, (u16)0, (u16)(Len+16), inBuff, inOffset, inLength);

	//设置安全级别
	secLvl = TransientDataBufForSCP10[2];
	//设置建立安全通道标志
	secLvl |= 0x04;
	if(REG_APID_MATCH == tempBufForSCP10[OFFSET_REG_APID])
	{
		secLvl|=0x80;
	}
	else
	{
		secLvl|=0x40;
	}

	if(RESULT_NOK == ManageSecurityLevel(secLvl))
	{
		throw_error(SW_WRONG_DATA);
	}
	managerRequestedChannelLevel(channelInfo.security_level[selected_channel]);
}



/**
 * @param apdu
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 */
u16 scp10_agreementInternalAuthenticate(PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10)
{

	// PKLen-1-1-32-20
#define		PADDING_COUNT				0x36
	// OFFSET_CDATA + 32+ 1
#define		RAND_ADD_ID_OFF				0x26


	u16		XDATA_AREA		pKeyLen;
	u8 		XDATA_AREA		randPaddingLen;
	u8 		XDATA_AREA		tempI;
	u8 		XDATA_AREA		crtKeyLen;
	u8 		XDATA_AREA		tempOff;

	u8		XDATA_AREA		bKeyType;
	PN8				sdDataObj;
	PN8				keySetObj;
	PX8				tempSrcPtr;
	PX8				tempDesPtr;
	PN8				tempPtr;
	memref	XDATA_AREA		keyAddress;


	crtKeyLen = tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH];

	//获取签名数据长度
	//PKLen = (u16)(SKSDAUTCRT.getSize()/8);
	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);
	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	pKeyLen = (keyData_getKeyDataById(keySetObj, KEY_ID_02, RSA_CRT_P_DGI_TYPE, (RAM_U32_P)&keyAddress) << 1);

	//计算随机填充个数
	randPaddingLen = (u8)U16_S_U8(pKeyLen, PADDING_COUNT);

	//搬移终端随机数和id到末尾
	tempOff = (u8)(U8_A_U8(randPaddingLen , RAND_ADD_ID_OFF));
	tempDesPtr = PX8_A_U8(APDUBuffer , tempOff);
	tempSrcPtr = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, CARD_CHALLEGE_ID_LENGTH);

	//arrayCopyNonAtomic(apduBuf, ISO7816.OFFSET_CDATA, apduBuf, sOff, (u16)16);
	//填充randPaddingLen个随机数
	PX8_INC(tempSrcPtr);
	//alea.generateData(apduBuf, randPaddingOff, randPaddingLen);
	//或者填充randPaddingLen个BB,规范需要修改
	memFillToRam((memref)pointer_to_ref(tempSrcPtr), randPaddingLen, (u8)0xBB);
	//arrayFillNonAtomic(apduBuf, randPaddingOff, randPaddingLen, (u8)0xBB);

	//生成card secret
	tempDesPtr = PX8_A_U8(TransientDataBufForSCP10 , crtKeyLen);
	Random_number_generator(tempDesPtr, RAND_DATA_LENGTH, 0x00 );
	//generateData(TransientDataBuf, (u16)(tempBuff[OFFSET_KEYS_CRT_LENGTH] & 0x00FF), (u16)32);

	//搬移card secret到apduBuff
	tempPtr = PX8_A_U8(tempSrcPtr , randPaddingLen);
	ecpMem_Copy( tempPtr, tempDesPtr, (u16)32);
	//Util.arrayCopyNonAtomic(TransientDataBuf, (u16)(tempBuff[OPConstants.OFFSET_KEYS_CRT_LENGTH]&0x00FF), apduBuf, cardSecretOff, (u16)32);

	//计算hash值
	tempDesPtr = PX8_A_U8(APDUBuffer , tempOff);
	tempI = U8_A_U8(randPaddingLen , CARD_CHALLEGE_ID_ADD_RAND_DATA_LENGTH);
	HASHDoFinal(tempSrcPtr, tempI, (MEM_U8_P)pointer_to_ref(tempDesPtr), HASH_INIT, (MEM_U8_P)NULL, (MEM_U8_P)NULL, ALG_SHA1);
	//hash.doFinal(apduBuf, randPaddingOff, (u16)(randPaddingLen+32+16), apduBuf, sOff);

	//填充head padding 0x6A
	APDUBuffer[OFFSET_CDATA] = (u8)0x6A;

	//填充end padding 0xBC
	tempOff =(u8)(U8_A_U8(tempOff, SHA1_DIGEST_SIZE)); //sOff+=0x14;
	APDUBuffer[tempOff] = (u8)0xBC;
	//计算签名数据
	tempPtr = PX8_A_U8(APDUBuffer , OFFSET_CDATA);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_02,THROW_ERROR);
	nativeRSAChiperDoFinalCal(keySetObj, bKeyType, KEY_ID_02, (memref)pointer_to_ref(tempPtr), pKeyLen, (memref)pointer_to_ref(tempPtr), pKeyLen, ALG_RSA_NOPAD, MODE_DECRYPT);
	//RSAObject.init(SKSDAUTCRT, Cipher.MODE_DECRYPT);
	//RSAObject.doFinal(apduBuf, ISO7816.OFFSET_CDATA, PKLen, apduBuf, ISO7816.OFFSET_CDATA);

	//获取N.PK.SD.AUT公钥值
	pKeyLen = keyData_getKeyDataById(keySetObj, KEY_ID_03, RSA_N_DGI_TYPE, (RAM_U32_P)&keyAddress);
	tempDesPtr = PX8_A_U8(tempPtr , pKeyLen);
	memCopyToRam((memref)tempDesPtr, (memref)keyAddress, pKeyLen);
	//PKSDAUTSF.getModulus(apduBuf, (short)(ISO7816.OFFSET_CDATA+PKLen));

	//处理签名数据，取SIG.SD.AUT and (N.PK.SD.AUT – SIG.SD.AUT)最小值
	signAndVerifyProcessISO9797_2(tempPtr, tempDesPtr, pKeyLen, SIGN_PROCESS);
	//signAndVerifyProcessISO9797_2(APDUBuffer, OFFSET_CDATA, APDUBuffer, OFFSET_CDATA + pKeyLen, pKeyLen, SIGN_PROCESS);
	//SecurityNativeMethods.signAndVerifyProcessISO9797_2(apduBuf, (short)ISO7816.OFFSET_CDATA, apduBuf, (short)(ISO7816.OFFSET_CDATA+PKLen), PKLen, OPConstants.SIGN_PROCESS);

	//获取加密数据长度
	pKeyLen = keyData_getKeyDataById(keySetObj, KEY_ID_04, RSA_N_DGI_TYPE, (RAM_U32_P)&keyAddress);
	//加密签名数据
	tempPtr = PX8_A_U8(APDUBuffer , OFFSET_CDATA);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_04,THROW_ERROR);
	pKeyLen = nativeRSAChiperDoFinalCal(keySetObj, bKeyType, KEY_ID_04, (memref)pointer_to_ref(tempPtr), pKeyLen, (memref)pointer_to_ref(tempPtr), pKeyLen, ALG_RSA_NOPAD, MODE_ENCRYPT);
	//PKLen = SimpleRSADoFinalCal((u8 *) &APDUBuffer[OFFSET_CDATA], PKLen, &APDUBuffer[OFFSET_CDATA], PKLen, PKOCEAUT, MODE_ENCRYPT);
	//RSAObject.init(PKOCEAUT, Cipher.MODE_ENCRYPT);
	//PKLen = RSAObject.doFinal(apduBuf, ISO7816.OFFSET_CDATA, PKLen, apduBuf, ISO7816.OFFSET_CDATA);

	//保存后4字节卡片秘密数据
	tempI = U8_A_U8(crtKeyLen , 0x1C);
	tempDesPtr = PX8_A_U16(tempBufForSCP10 , OFFSET_FOUR_CS);
	tempSrcPtr = PX8_A_U8(TransientDataBufForSCP10 , tempI);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, (u16)4);
	//Util.arrayCopyNonAtomic(TransientDataBuf, (u16)((tempBuff[OPConstants.OFFSET_KEYS_CRT_LENGTH]&0x00FF)+28), tempBuff, OPConstants.OFFSET_FOUR_CS, (u16)4);

	//保存后4字节卡外实体秘密数据
	tempI = U8_S_U8(crtKeyLen , 0x04);
	tempDesPtr = PX8_A_U16(tempBufForSCP10 , OFFSET_FOUR_OES);
	tempSrcPtr = PX8_A_U8(TransientDataBufForSCP10 , tempI);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, (u16)4);
	//Util.arrayCopyNonAtomic(TransientDataBuf, (u16)((tempBuff[OPConstants.OFFSET_KEYS_CRT_LENGTH]&0x00FF)-4), tempBuff, OPConstants.OFFSET_FOUR_OES, (u16)4);
	//计算、设置session key和ICV
	scp10_computeAndSetSessionKey(TransientDataBufForSCP10, tempBufForSCP10);
	//设置安全级别
	return pKeyLen;
}


/**
 * @param inBuff
 * @param tempBuff
 */
void scp10_dealcomputeAndSetSessionKey(u8 * inBuff, u8 * tempBuff, u16 counterOff, u16 offCardSecretOff, u16 sessionKeyOff)
{
	u8 		XDATA_AREA		optionValue;
	u8 		XDATA_AREA		tempBIT;
	u8 		XDATA_AREA		sessionKeyFlag;

	PX8				tempCountPtr;
	PX8				cardSecretPtr;
	PX8				sessionKeyPtr;


	optionValue = g_current_SCP_I_Option;
	sessionKeyFlag = tempBuff[OFFSET_KEY_FLAG];
	tempCountPtr = PX8_A_U8(tempBuff, OFFSET_CMAC_CNT);
	cardSecretPtr = PX8_A_U8(inBuff, offCardSecretOff);
	sessionKeyPtr = PX8_A_U8(inBuff, sessionKeyOff);

	tempBIT = BIT_1;

	while(tempBIT <= BIT_6)
	{
		if( tempBIT == (tempBIT & sessionKeyFlag) )
		{
			inBuff[counterOff] = PX_R_U8(tempCountPtr);
			HASHDoFinal(cardSecretPtr, (u16)(36), (MEM_U8_P)pointer_to_ref(sessionKeyPtr), MD_INIT, (MEM_U8_P)NULL, (MEM_U8_P)NULL, ALG_SHA1);
			//hash.doFinal(inBuff, offCardSecretOff, (u16)(36), inBuff, sessionKeyOff);
			scp10_proSesKeysCRT(inBuff, sessionKeyOff, (u16)16, tempBIT, optionValue);
			if(tempBIT <= BIT_2)
			{
				scp10_proSesKeysCRT(tempBuff, OFFSET_FOUR_CS, (u16)8, tempBIT, optionValue);//设置ICV
			}
		}

		tempBIT = tempBIT << 1;
		PX8_INC(tempCountPtr);
	}

}



/**
 * @param inBuff
 * @param tempBuff
 */
void scp10_computeAndSetSessionKey(u8 * inBuff, u8 * tempBuff)
{
	u16 	XDATA_AREA		cardSecretOff;
	u16 	XDATA_AREA		offCardSecretOff;
	u16 	XDATA_AREA		sessionKeyOff;
	u16 	XDATA_AREA		counterOff;
	u8 		XDATA_AREA		optionValue;
	u8 		XDATA_AREA		tempI;

	optionValue = g_current_SCP_I_Option;

	cardSecretOff = (u16)( tempBuff[OFFSET_KEYS_CRT_LENGTH] & 0x00FF);
	offCardSecretOff = (u16)(U16_S_U8(cardSecretOff ,0x20));
	for(tempI=0; tempI<32; tempI++)
	{
		inBuff[offCardSecretOff] = (u8)(inBuff[offCardSecretOff]^inBuff[U16_INC(cardSecretOff)]);
		U16_INC(offCardSecretOff);
	}

	memFillToRam((memref)pointer_to_ref(&inBuff[offCardSecretOff]), 0x04, (u8)0x0);


	//Util.arrayFillNonAtomic(inBuff, offCardSecretOff, (u16)4, (u8)0);
	sessionKeyOff = (u16)(U8_A_U8(offCardSecretOff,0x04));
	counterOff = (u16)(U8_A_U8(offCardSecretOff,0x03));
	offCardSecretOff = (u16)(U16_S_U8(offCardSecretOff,0x20));

	scp10_dealcomputeAndSetSessionKey(inBuff, tempBuff, counterOff, offCardSecretOff, sessionKeyOff);

}

/**
 * @param apdu
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 */
u16 scp10_transportInternalAuthenticate(PX8 TransientDataBufForSCP10, PX8 tempBufForSCP10)
{
	u8 		XDATA_AREA		apduLc;
	u8 		XDATA_AREA		Len;
	u8 		XDATA_AREA		sOff;
	u8 		XDATA_AREA		keyOff;
	u8 		XDATA_AREA		tempI;
	u8 		XDATA_AREA		tempJ;
	u8 		XDATA_AREA		CRTLen;
	u8 		XDATA_AREA		sessionKeyLen;


	u8		XDATA_AREA		bKeyType;
	PN8				sdDataObj;
	PN8				keySetObj;
	PN8				tempPtr;
	PX8				tempSrcPtr;
	PX8				tempDesPtr;


	Len = 0;
	keyOff = 0;
	apduLc = APDUBuffer[OFFSET_LC];

	tempSrcPtr = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
	tempDesPtr = PX8_A_U8(tempBufForSCP10 , OFFSET_CHALLENGE);
//	memCopyNoAtomic( (memref)tempDesPtr, (memref)tempSrcPtr, CARD_CHALLEGE_ID_LENGTH);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, CARD_CHALLEGE_ID_LENGTH);
	//Util.arrayCopyNonAtomic(apduBuf, (u16)5, tempBuff, OPConstants.OFFSET_CHALLENGE, (u16)16);

	//copy session keys to apduBuff
	Len = tempBufForSCP10[OFFSET_KEYS_CRT_LENGTH];

	Len = (u8)(U8_S_U8(Len,0x03));//Len -= 3;

	tempI = 0;
	while(tempI < Len)
	{
		tempJ = U8_A_U8(tempI, 0x04);
		CRTLen = TransientDataBufForSCP10[tempJ];

		U8_INC(tempJ);
		sOff = scp10_searchTag(TransientDataBufForSCP10, tempJ, CRTLen, SESSION_KEY_TAG);

		U8_INC(sOff);
		sessionKeyLen = TransientDataBufForSCP10[sOff];

		U8_INC(sOff);
		tempSrcPtr = PX8_A_U16(TransientDataBufForSCP10 , sOff);
		tempDesPtr = PX8_A_U8(APDUBuffer, keyOff);
		ecpMem_Copy( tempDesPtr, tempSrcPtr, sessionKeyLen);
//		memCopyNoAtomic( (memref) tempDesPtr, (memref)tempSrcPtr, sessionKeyLen);
		//Util.arrayCopyNonAtomic(TransientDataBuf, (u16)(sOff+2), apduBuf, keyOff, sessionKeyLen);

		keyOff = (u8)U8_A_U8(keyOff, sessionKeyLen);//keyOff+=sessionKeyLen;
		CRTLen = (u8)(U8_A_U8(CRTLen, 0x02));
		tempI = (u8)(U8_A_U8(tempI, CRTLen));//i+=(CRTLen+2);
	}

	//copy OCE challenge to apduBuff
	tempSrcPtr = PX8_A_U16(tempBufForSCP10 , OFFSET_CHALLENGE);
	tempDesPtr = PX8_A_U8(APDUBuffer, keyOff);
	ecpMem_Copy( tempDesPtr, tempSrcPtr, CARD_CHALLEGE_ID_LENGTH);
//	memCopyNoAtomic( (memref) tempDesPtr, (memref)tempSrcPtr, (u16)CARD_CHALLEGE_ID_LENGTH);
	//Util.arrayCopyNonAtomic(tempBuff, OPConstants.OFFSET_CHALLENGE, apduBuf, keyOff, (u16)16);

	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);
	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);
	bKeyType = keySet_getKeyType(keySetObj, KEY_ID_02,THROW_ERROR);
	tempPtr = PX8_A_U8(APDUBuffer , OFFSET_CDATA);

	Len = (u8)(U8_A_U8(keyOff, CARD_CHALLEGE_ID_LENGTH));
	Len = nativeRSASignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_02, (memref)APDUBuffer, Len, (memref)tempPtr, Len, ALG_RSA_SHA_PKCS1, MODE_SIGN);
	//Len = SimpleRSADoFinalCal((u8 *) APDUBuffer, Len, &APDUBuffer[OFFSET_CDATA], Len, SKSDAUTCRT, MODE_SIGN);
	//PKAPAUTRsaSig.init(SKSDAUTCRT, Signature.MODE_SIGN);
	//Len = PKAPAUTRsaSig.sign(apduBuf, (u16)0, Len, apduBuf, (u16)5);

	return Len;
}

/**
 * @param apdu
 * @return
 */
void scp10_generateKeyPair()
{
	u8 		XDATA_AREA		bP2_;
	u16 	XDATA_AREA		sCDATA;
	u16 	XDATA_AREA		modLen;
	u16 	XDATA_AREA		expLen;
	u16 	XDATA_AREA		tempI;
	u16 	XDATA_AREA		keyDataLen;

	PN8					sdDataObj;
	PN8					keySetObj;
	PN8 					keyLocation;
	PN8					keyData;
	PN8					tempPtrSrc;
	PN8					tempPtrDes;
	
	bP2_ = APDUBuffer[OFFSET_P2];
	//命令数据域判断
	sCDATA = OFFSET_CDATA;

	if(0x00 != MEM_arrayCompare((memref)SCP10_GENKEYPAIR_CDATA_DATA_VALUE, (memref)&APDUBuffer[sCDATA], SCP10_GENKEYPAIR_CDATA_DATA_SIZE))
	{
		throw_error(SW_WRONG_DATA);
	}

	sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);	
	keySetObj = getKeySetObj(sdDataObj, CASD_KEY_VERSION);

	keyLocation = (NUM_TO_NPTR)fm03_locateKeyType(keySetObj, KEY_ID_02);
	if (REF_NULL == keyLocation)
	{
		keyDataLen = 0;
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PUBLIC_KEY_N_LEN, true);
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PUBLIC_KEY_E_LEN, false);
		keyData_addKeyDataSpace(keySetObj, keyDataLen);

		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_03, RSA_N_DGI_TYPE , RSA_PUBLIC_KEY_N_LEN);
//		nvmCopyAtomic(keyData, key5, RSA_PUBKEY_N_LEN);

		g_RSAKeyStore.wModulusNLength = RSA_PUBLIC_KEY_N_LEN;
		g_RSAKeyStore.pbModulusN = (MEM_U8_P)MEMtPX8(keyData);

		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_03, RSA_E_DGI_TYPE , RSA_PUBLIC_KEY_E_LEN);
		nvmCopyAtomic((memref)keyData, (memref)RSA_PUBKEY_E, RSA_PUBLIC_KEY_E_LEN);

		g_RSAKeyStore.wExpLength = RSA_PUBLIC_KEY_E_LEN;
		g_RSAKeyStore.pbExp = (MEM_U8_P)MEMtPX8(keyData);


		keyDataLen = 0;
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PRIVATE_KEY_P_LEN, true);
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PRIVATE_KEY_Q_LEN, false);
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PRIVATE_KEY_DP_LEN, false);
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PRIVATE_KEY_DQ_LEN, false);
		keyDataLen = keyData_computeKeyDataLen(KEY_FORMAT_03, keyDataLen, RSA_PRIVATE_KEY_QINV_LEN, false);
		keyData_addKeyDataSpace(keySetObj, keyDataLen);

		g_RSAKeyStore.wPrimeLength = RSA_PRIVATE_KEY_P_LEN;
		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, RSA_CRT_P_DGI_TYPE , RSA_PRIVATE_KEY_P_LEN);
		g_RSAKeyStore.pbParmP = (MEM_U8_P)MEMtPX8(keyData);

		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, RSA_CRT_Q_DGI_TYPE , RSA_PRIVATE_KEY_Q_LEN);
		g_RSAKeyStore.pbParmQ = (MEM_U8_P)MEMtPX8(keyData);

		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, RSA_CRT_DP_DGI_TYPE , RSA_PRIVATE_KEY_DP_LEN);
		g_RSAKeyStore.pbParmDP1 = (MEM_U8_P)MEMtPX8(keyData);

		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, RSA_CRT_DQ_DGI_TYPE , RSA_PRIVATE_KEY_DQ_LEN);
		g_RSAKeyStore.pbParmDQ1 = (MEM_U8_P)MEMtPX8(keyData);

		keyData = keyData_prepareKeyData(keySetObj, TAG_NEW_KEY, KEY_ID_02, RSA_CRT_IQP_DGI_TYPE , RSA_PRIVATE_KEY_QINV_LEN);
		g_RSAKeyStore.pbParmInvQ = (MEM_U8_P)MEMtPX8(keyData);
		
		g_RSAKeyStore.bRSAKeyType = ALG_RSA_CRT;
	}

//	GenRSAKeyAddress(SKSDAUTCRT, LENGTH_RSA_1024, TYPE_RSA_CRT_PRIVATE);
//	GenRSAKeyAddress(PKSDAUTSF, LENGTH_RSA_1024, TYPE_RSA_PUBLIC);
	memRefresh();
	RSAKeyGenCore();

	if(0x00==bP2_  )//返回公钥
	{
		modLen = g_RSAKeyStore.wModulusNLength;
		tempPtrSrc = (PN8)pointer_to_ref(g_RSAKeyStore.pbModulusN);
		tempPtrDes = PX8_A_U16(APDUBuffer , OFFSET_CDATA_ADD_CONST_3);
		memCopyToRam((memref) tempPtrDes, (memref)tempPtrSrc, modLen);

		expLen = g_RSAKeyStore.wExpLength;//指数长度3个字节
		tempPtrSrc = (PN8)pointer_to_ref(g_RSAKeyStore.pbExp);
		PX8_INC(tempPtrDes);
		tempPtrDes  = PX8_A_U8(tempPtrDes , modLen);
		memCopyToRam((memref) tempPtrDes, (memref)tempPtrSrc, expLen);

		APDUBuffer[OFFSET_CDATA] = (u8)0xC3;
		APDUBuffer[OFFSET_CDATA_ADD_CONST_1] = (u8)0x81;

		tempI = (u8)(U8_A_U8(modLen, expLen));
		U16_INC(tempI);
		APDUBuffer[OFFSET_CDATA_ADD_CONST_2] = tempI;

		tempI = (u16)(U16_A_U8(tempI, 0x03));//tem += 3;
		apdu_ApduSetOutgoingAndSend(OFFSET_CDATA, tempI);
	}

}


/*
 * check MAC  // first single des, final triple des
 *
 * */
void CalMACSCP10(u8 * srcAddress, u8 len, u8 * keyBuff, u8 mode)
{
	u8 		XDATA_AREA		blockNum;
	u8 		XDATA_AREA		bLength;
	u8 		XDATA_AREA		tempI;
	u8 		XDATA_AREA		ICV[DES_BLOCKSIZE];
	PX8 				tempPtr;

	if(CMAC_CAL == mode )
	{
		ecpMem_Copy((PN8)(TEMPBuffer+8), (PX8)srcAddress, (u8)0x04);
		TEMPBuffer[8] &= 0xF0;
		TEMPBuffer[12] = 0x80;
		tempPtr = PX8_A_U8(srcAddress , OFFSET_CDATA);
		len -= OFFSET_CDATA;
	}
	else
	{
		tempPtr = srcAddress;
	}
	blockNum = (u8)add80Padding(tempPtr, len, DES_BLOCKSIZE);
	bLength = (u16) (blockNum << DES_KEY_PADDING_OFF);

	ecpMem_Set((PX8) ICV, CLEARED_VALUE, DES_BLOCKSIZE);
	bLength = U8_S_U8(bLength , DES_BLOCKSIZE);

	if(CMAC_CAL == mode )
	{
		//加密计数器
		//HARDWARE_DES((u8 *)&cardChallenge[(selected_channel << 4)], (u8 *)&macKey[(selected_channel << 4)], ICV, 0x01);
		single_DESMAC(g_current_SCP_cardChallenge, keyBuff, (PN8)TEMPBuffer, (u16)DES_BLOCKSIZE, ICV);
		single_DESMAC((PN8)(TEMPBuffer+8), keyBuff, (PN8)TEMPBuffer, (u16)DES_BLOCKSIZE, ICV);
	}
	else
	{
		//加密顺序计数器
		single_DESMAC(g_current_SCP_hostChallenge, keyBuff, (PN8)TEMPBuffer, (u16)DES_BLOCKSIZE, ICV);
	}

	if ( 0x01 != blockNum )
	{
		single_DESMAC(tempPtr, keyBuff, (PN8)(TEMPBuffer), bLength, ICV);
	}
	tempPtr = PX8_A_U8(tempPtr , bLength);
	Triple_DES(tempPtr, keyBuff, (PN8)(TEMPBuffer), (u16)DES_BLOCKSIZE, ICV, (u8)CBC_Encrypt, SENSITIVE_DATA_MASKED);
}


/*
 * 	用于CR-mac的计数器加1
 *	endian, 0:small; 1:big
 */


void scp10_ICVSeqCntINC(PX8 seq, u8 format)
{
	#define		ltob32(x)		x
	u32 	XDATA_AREA		cntLow;
	u32		XDATA_AREA		cntHigh;
	PX8				tempPtr;

	tempPtr = seq;
	cntHigh = ltob32(XRAM_READ_U32(tempPtr));
//	VAR_WRITE_U32((NPTR_TO_NUM)cntHigh, XRAM_READ_U32(tempPtr));

	tempPtr = PX8_A_U16(tempPtr , 0x04);
	cntLow = ltob32(XRAM_READ_U32(tempPtr));
//	VAR_WRITE_U32((NPTR_TO_NUM)cntHigh, XRAM_READ_U32(tempPtr));

	U32_INC(cntLow);
	XRAM_WRITE_U32((memref)tempPtr, cntLow);
	if(0x00 == cntLow)
	{
		cntHigh ++;
		tempPtr = seq;
		XRAM_WRITE_U32((memref)tempPtr, cntHigh);
	}
}


#define 	TAG_86_87		0x01
#define 	TAG_96_97		0x02
u8 checkTagIsAvailForCurrentSecurity(tag, compare_tag)
{
	u8 		XDATA_AREA		bOK;
	u8		XDATA_AREA		tempTag;
	u8 		XDATA_AREA		bSecureLevel;

	bOK = 0x01;
	bSecureLevel = channelInfo.security_level[selected_channel];

	if((bSecureLevel & 0x03) == 0x02)//命令数据密文
	{
	 if(tag==TAG_96)//修改处
			{
		                tempTag = TAG_96;
						APDUBuffer[OFFSET_LC]=0;
						APDUBuffer[OFFSET_CDATA]=APDUBuffer[OFFSET_CDATA+2];
						bOK = 1;
			}
	 else{
		switch(compare_tag)
		{
			case TAG_86_87:
				tempTag = TAG_86;
				break;

			case TAG_96_97:
				tempTag = TAG_96;
				break;
		  }
		}
	}
	else if((bSecureLevel & 0x03) == 0x03)//命令数据密文+mac
	{
		switch(compare_tag)
		{
			case TAG_86_87:
				tempTag = TAG_87;
				break;

			case TAG_96_97:
				tempTag = TAG_97;
				break;
		}
	}

	if(tag != tempTag)
	{
		bOK = 0;
	}

	return bOK;
}


u8 scp10_dealencData(u8 le)
{
	u8 		XDATA_AREA		bOK;
	u8 		XDATA_AREA		ciphLen;
	u8 		XDATA_AREA		leVal;
	u8 		XDATA_AREA		bLc;
	u8 		XDATA_AREA		tag;
	u8 		XDATA_AREA		tagLen;
	u8 		XDATA_AREA		bSecureLevel;
	PX8				tempPtr;

	bOK = 1;
	bLc = APDUBuffer[OFFSET_LC];
	tag = APDUBuffer[OFFSET_CDATA];
	tagLen = APDUBuffer[OFFSET_CDATA_ADD_CONST_1];
	bSecureLevel = channelInfo.security_level[selected_channel];

	bOK = checkTagIsAvailForCurrentSecurity(tag, TAG_86_87);
    if(0 == APDUBuffer[OFFSET_LC]){                     //修改处
    	return bOK;
    }
    else{
	if(tagLen == 0x81)//两个字节长度
	{
		tag = APDUBuffer[OFFSET_CDATA_ADD_CONST_3];
		tagLen = APDUBuffer[OFFSET_CDATA_ADD_CONST_2];
		ciphLen = tagLen+3;
	}
	else//一个字节
	{
		tag = APDUBuffer[OFFSET_CDATA_ADD_CONST_2];
		ciphLen = tagLen + 2;
	}

	if(0x01 != (u8)(tagLen & 0x07))
	{
		bOK = 0;
	}

	if(0x01 != tag)
	{
		bOK = 0;
	}

	tagLen =(u8)( bLc - ciphLen );

	tempPtr = PX8_A_U16(APDUBuffer , OFFSET_CDATA);
	tempPtr = PX8_A_U8(tempPtr , ciphLen);

	if(0x03 == tagLen)
	{
		tag = PX_R_U8(tempPtr);

		PX8_INC(tempPtr);
		tagLen = PX_R_U8(tempPtr);

		PX8_INC(tempPtr);
		leVal = PX_R_U8(tempPtr);

		bOK = checkTagIsAvailForCurrentSecurity(tag, TAG_96_97);

		if(0x01 != tagLen)
		{
			bOK = 0;
		}
		/*if(le != leVal) //要检验TAG 97偏移+2处的Le和命令末尾的Le作比较，但是接触卡命令中，我们无法接收到Le
		{
			bOK = 0;
		}*/
	}
	else if(0x00 == tagLen)
	{
		if(0x00 != le)
		{
			bOK = 0;
		}
	}
	else
	{
		bOK = 0;
	}
    }
	return bOK;
}




u8 scp10_dealMacDataProc(u8 tag, u8 tagLen, u8 bLc , u8 le)
{
	u8 		XDATA_AREA		bOK;
	u8 		XDATA_AREA		offSet;
	PX8				tempPtr1;
	PX8				tempPtr2;

	bOK = NOK;
	switch(tag)
	{
		case 0x80:
			if(10 == bLc)
			{
				APDUBuffer[OFFSET_LC] = 0x00;
				bOK = 1;
			}
			break;
		case 0x97:
			if(13 == bLc)
			{
				APDUBuffer[OFFSET_LC]= 0x00;
				if((APDUBuffer[OFFSET_CDATA_ADD_CONST_2] == le) || ( 0x00== le))//TAG97后面的Le长度B4，要和命令后面的Le进行对比，但在接触情况下收不到命令后面的Le,所以后面的le就一直为0
				{
					bOK = 1;
					APDUBuffer[OFFSET_CDATA] = le;
				}
			}
			if(0x01 != tagLen)
			{
				bOK = NOK;
			}
			break;
		case 0x81:

			if(0x81 == tagLen)//2字节长度
			{
				offSet = OFFSET_CDATA_ADD_CONST_2;
			}
			else//1字节长度
			{
				offSet = OFFSET_CDATA_ADD_CONST_1;
			}
			tempPtr1 = PX8_A_U8(APDUBuffer, offSet);
			tagLen = PX_R_U8(tempPtr1);

			PX8_INC(tempPtr1);
			tempPtr2 = tempPtr1;

			tempPtr1 = PX8_A_U8(tempPtr1, tagLen);
			tag = PX_R_U8(tempPtr1);

			if(0x97 == tag)
			{
				PX8_INC(tempPtr1);
				if(0x01 == PX_R_U8(tempPtr1))
				{
					PX8_INC(tempPtr1);
					if((APDUBuffer[OFFSET_CDATA+tagLen+5]!=le)&&(le!=0))//在这里又将TAG97后面的Le和命令后面的Le做对比，但是收不到后面那个Le啊
						bOK = 0;
					else
					{
						//tempPtr1 = PX8_A_U16(APDUBuffer, OFFSET_CDATA);
						//tempPtr1 = PX8_A_U8(tempPtr1, tagLen);
						//PX_W_U8(tempPtr1, le);
						bOK = 1;
					}
				}
			}
			else if(tag==0x80)
			{
				bOK = 1;
			}

			APDUBuffer[OFFSET_LC] = tagLen;
			tempPtr1 = PX8_A_U8(APDUBuffer, OFFSET_CDATA);

			ecpMem_Move(tempPtr1, tempPtr2, (u8)tagLen);

			break;
		default:
			break;
	}

	return bOK;
}

u8 scp10_dealMacData(u8 le, u8 option)
{
	u8 		XDATA_AREA		bSecureLevel;
	u8 		XDATA_AREA		bLc;
	u8 		XDATA_AREA		tag;
	u8 		XDATA_AREA		tagLen;
	u8 		XDATA_AREA		bOK;
	u8 		XDATA_AREA		bOff;


	bSecureLevel = channelInfo.security_level[selected_channel];
	bLc = APDUBuffer[OFFSET_LC];
	tag = APDUBuffer[OFFSET_CDATA];
	tagLen = APDUBuffer[OFFSET_CDATA+1];

	if(option == BEFORE_MAC_CAL)
	{
		bOK = OK;
		/*if(BIT_1 != (BIT_1 & sesKeyFlag) )//无C-MAC会话密钥
		{
			bOK = NOK;
		}*/
		//offset to tag
		bOff = bLc + OFFSET_CDATA - 10;
		if(APDUBuffer[bOff] != 0x8E)
		{
			bOK = NOK;
		}

		//offset to len
		bOff ++ ;
		if(APDUBuffer[bOff] != 0x08)
		{
			bOK = NOK;
		}
	}
	else
	{
		bOK = NOK;
		if((bSecureLevel & 0x03) == 0x03)//命令数据密文+MAC
		{
			if(tag == 0x87)
			{
				APDUBuffer[OFFSET_LC] -=10;
				if(le!=0)
				{
					APDUBuffer[OFFSET_CDATA+APDUBuffer[OFFSET_LC]] = le;
				}
				bOK = OK;
			}
			else if(tag == 0x81)
			{
				bOK = OK;
			}
			else
			{
				bOK = scp10_dealMacDataProc(tag, tagLen, bLc , le);
			}
		}

		if((bSecureLevel & 0x03) == 0x01)//命令数据明文+MAC
		{
			bOK = scp10_dealMacDataProc(tag, tagLen, bLc , le);
		}

	}

	return bOK;
}


/*
 *
 *
 * */
u16 decryptCommandDataSCP10()
{
	u8 		XDATA_AREA		bOff;
	u8 		XDATA_AREA		tagLen;
	u8 ICV[8];

	bOff = 0x00;
	tagLen = APDUBuffer[OFFSET_CDATA_ADD_CONST_1];

	if(0x81 == tagLen)//两个字节长度
	{
		bOff = OFFSET_CDATA_ADD_CONST_4;
		tagLen = APDUBuffer[OFFSET_CDATA_ADD_CONST_2];
	}
	else//一个字节
	{
		bOff = OFFSET_CDATA_ADD_CONST_3;
	}


	tagLen = U8_S_U8(tagLen,0x01);//tagLen -=1;

	ecpMem_Set(ICV , 0x00, (u8)sizeof(ICV));

	Triple_DES((u8 *)(APDUBuffer+bOff), g_current_SCP_SES_KEY, (u8 *)(APDUBuffer+OFFSET_CDATA), (u16)tagLen, ICV, CBC_Decrypt, SENSITIVE_DATA_MASKED);

	return tagLen;
}


u8 scp10_dealRMacData(u8 * SrcAddr, u8 length, u8 option, u8 sw1, u8 sw2)
{
	PX8			ptr;
	u8		XDATA_AREA	securityLevel;

	if(BEFORE_MAC_CAL == option)
	{
		securityLevel = channelInfo.security_level[selected_channel];
		if( SCF_R_MAC_FLAG == (securityLevel & SCF_R_MAC_ENCRYPTION_FLAG )  )//r-mac  only
		{
			if(0x00 != length)
			{
				if(length >= 0x80)//2个字节长度
				{
					ptr = PX8_A_U16(SrcAddr, 0x03);
					ecpMem_Move(ptr, SrcAddr, length);

					ptr = PX8_A_U16(SrcAddr, 0x01);
					writeRamTwoByte(ptr, 0x81, length)

					length = U8_A_U8(length, 3);
				}
				else
				{
					ptr = PX8_A_U16(SrcAddr, 0x02);
					ecpMem_Move(SrcAddr+2, SrcAddr, length);

					ptr = PX8_A_U16(SrcAddr, 0x01);
					writeRamOneByte(ptr, length);

					length = U8_A_U8(length, 2);
				}
			}

			writeRamOneByte(SrcAddr, 0x81);
			ptr = PX8_A_U8(SrcAddr, length);

			writeRamFourByte(ptr, 0x99, 0x02, sw1, sw2);
			length = U8_A_U8(length, 4);
		}
	}
	else
	{
		ptr = PX8_A_U8(SrcAddr, length);
		writeRamTwoByte(ptr, 0x8E, 0x08)
		length = U8_A_U8(length, 2);

		ptr = PX8_A_U8(SrcAddr, length);
		ecpMem_Copy(ptr, (PX8)(TEMPBuffer), 8);
		length = U8_A_U8(length, 8);

		scp10_ICVSeqCntINC(g_current_SCP_hostChallenge, 0);

		//CR-mac同顺序计数器
		if( BIT_7 == (BIT_7 & sesKeyFlag) )
		{
			scp10_ICVSeqCntINC(g_current_SCP_cardChallenge, 0);
		}

	}

	return length;
}

u16 EncryptOfResponseSCP10(u8 securityLevel, u8 * SrcAddr, u8 length, u16 sw)
{
	u8 		XDATA_AREA		ICV[8];
	PX8				ptr;
	u8 		XDATA_AREA		sw1;
	u8 		XDATA_AREA		sw2;

	sw1 = (u8)( sw >> 8 );
	sw2 = (u8)sw;

	//2字节状态字
	if(0x00 == length)
	{
		if( SCF_R_MAC_ENCRYPTION == (securityLevel & SCF_R_MAC_ENCRYPTION) )//r-enc+r-mac
		{
			writeRamFourByte(SrcAddr, 0x99, 0x02, sw1, sw2);
			length = 0x04;
		}
	}
	else
	{
		ecpMem_Set(ICV, 0x00, sizeof(ICV));

		length = (u8)(add80Padding(SrcAddr, length, DES_BLOCKSIZE) << DES_KEY_PADDING_OFF);

		Triple_DES(SrcAddr, g_current_SCP_RSES_KEY, SrcAddr, (u16)length, ICV, CBC_Encrypt, SENSITIVE_DATA_MASKED);//R-enc

		ptr = XUADDR_ADD_1(SrcAddr);//ptr = SrcAddr+1;
		if( 0x80 <= U8_A_U8(length,0x01) )
		{
			ecpMem_Move((XUADDR_ADD_4(SrcAddr)), SrcAddr, length);//ecpMem_Move(SrcAddr+4, SrcAddr, length);
			writeRamThreeByte(ptr, 0x81, (u8)(U8_A_U8(length,0x01)), 0x01);
			length = U8_A_U8(length,0x04);//length +=4;
		}
		else
		{
			ecpMem_Move((XUADDR_ADD_3(SrcAddr)), SrcAddr, length);//ecpMem_Move(SrcAddr+3, SrcAddr, length);
			writeRamTwoByte(ptr, (u8)(U8_A_U8(length,0x01)), 0x01);
			length = U8_A_U8(length,0x03);//length +=3;
		}

		if( SCF_R_MAC_ENCRYPTION_FLAG == (securityLevel & SCF_R_MAC_ENCRYPTION_FLAG )  )//r-enc+r-mac
		{
			writeRamOneByte(SrcAddr, 0x87);

			ptr = PX8_A_U8(SrcAddr,length);//SrcAddr + length;
			writeRamFourByte(ptr, 0x99, 0x02, sw1, sw2);

			length = U8_A_U8(length,0x04);//length += 4;
		}
		else//r-enc
		{
			writeRamOneByte(SrcAddr, 0x86);
		}

	}
	return length;
}

void clearPKOCE()
{
	PN8	sdDataObj;
	u16			sdRef;
	sdRef				= popReference();
	sdDataObj			= (NUM_TO_NPTR)locateObjectInMemory(sdData_getSDdataRef(sdRef));	
	
	keySet_deleteScKey(sdDataObj, CASD_KEY_VERSION, KEY_ID_04);
}

#endif

