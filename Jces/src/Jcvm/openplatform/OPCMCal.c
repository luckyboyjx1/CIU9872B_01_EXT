/*
 * OPCMCal.c
 *
 *  Created on: 2013-10-15
 *      Author: jiangxiaohui
 */



#include "CgmManager.h"
#include "typejava.h"
#include "object.h"
#include "ob_manager.h"
#include "cardmanager.h"
#include "ecpDefine.h"
#include "SCPInterface.h"
#include "crypto_impl.h"
#include "OPCMCal.h"
#include "scp10.h"
#include "keySet.h"
#include "securityDomainData.h"
#include "global_variable.h"
#include "error.h"
#include "casdStoreKey.h"
#include "tagNode.h"
#include "ob_manager.h"
#include "opFunction.h"
#include "KeyManagerOld.h"
#include "KeyLogicManager.h"
#include "SCPDealData.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


/**
 * To do Load File Data Block DAP verification
 * @param buffer - Array containing the authentication data.
 * @param offset - Offset within the buffer where authentication data begin.
 * @param sdId_hashLen - Id of SD and hash length.
 * @param sigDate - Signature data.
 * @param offset - Offset within the signature buffer.
 * @param SigLen - Length of signature data.
 * @return TRUE if the operation is successful, FALSE otherwise.
 * modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
 */
#if	GP__DAP__FOR_MASK
// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 支持ECC算法的DAP
boolean verifyDAP(PX8 sigDate, u16 SigOff, u16 SigLen, u8 sdIdForDap)
{

	boolean 	X_A		result;
	u8			X_A		bKeyType;
	jref		X_A		sdRef;
	jref		X_A		sdDataRef;
	memref		X_A		theAppletTableEntry;
	PN8					sdDataObj;
	PN8					keySetObj;
	PN8					tempPtr;

	result = NOK;

	theAppletTableEntry = getEntryByAppId(sdIdForDap);
	sdRef = PX_R_U16(get_object_field_ref(theAppletTableEntry, theApplet_INDEX));
	sdDataRef = sdData_getSDdataRef(sdRef);
	sdDataObj = MEMtPN8(locateObjectInMemory(sdDataRef));
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, getDesDapKeyVersion(sdIdForDap)));
#else
	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, DAP_KEY_VERSION));
#endif
	bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);

	tempPtr = PX8_A_U8(sigDate , SigOff);
	if(KEY_TYPE_SYMMETRIC == bKeyType)
	{
		result = nativeDESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(&bRamBuffer[HASHBUFER_OFF]), SHA1_DIGEST_SIZE, PNtMEM(tempPtr), SigLen, ALG_DES_MAC8_ISO9797_1_M2_ALG3, MODE_VERIFY, U8_OR_U8(ALG_DES, SBC_VERIFY) );
	}
#if VM_WITH_RSA
	else if(KEY_TYPE_RSA_PUBLIC == bKeyType)
	{
		result = nativeRSASignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_01, &bRamBuffer[HASHBUFER_OFF], SHA1_DIGEST_SIZE, tempPtr, SigLen, ALG_RSA_SHA_PKCS1, MODE_VERIFY);
	}
#endif
	else if (KEY_AES_TYPE == bKeyType)
	{
		result = nativeAESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(&bRamBuffer[HASHBUFER_OFF]), SHA1_DIGEST_SIZE, PNtMEM(tempPtr), SigLen, MODE_VERIFY, U8_OR_U8(ALG_AES, SBC_VERIFY) );
	}
	else if(ECC_PublicKey_DGI_TYPE == bKeyType)
	{
		result = nativeECCSignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_01, &bRamBuffer[HASHBUFER_OFF], SHA1_DIGEST_SIZE, tempPtr, SigLen, MODE_VERIFY);
	}
	else
	{
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}

	return result;
}
#endif
// modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
#if	GP__RECEIPT__FOR_MASK
// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 支持ECC算法的Receipt
u8 ReceiptGeneration(PX8 inBuffer,u16 inOffset, u16 inLength, u16 confirmationCounter)
{

	u8			X_A		bKeyType;
	u8 			X_A		ReceiptLen;
	u8 			X_A		bConfirmationLen;
	u8 			X_A		btemp1;
	u8 			X_A		btemp2;
	u16 		X_A		bReceipteDataLen;

	jref		X_A		firstTagNodeRef;
	jref		X_A		tagArrayRef;


	PN8					sdDataObj;
	PN8					keySetObj;
	PN8					bIssuerBinArr;
	PN8					bCardIssuerDataArr;
	PN8					issuerNumDataArr;
	PN8					issuerNum;
	PX8					tempPtr;

	tempPtr = cacheBuffer;

	ReceiptLen = 0;

	bReceipteDataLen = 0;

	sdDataObj = MEMtPN8(getCMSDDataObj(RECEIPT_KEY_VERSION));

	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, RECEIPT_KEY_VERSION));

	bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	//获取seid的地址
	bCardIssuerDataArr = MEMtPN8(GetCardSEIDaddr());
	btemp1 = (u8)tagNode_getTagArrayLength(bCardIssuerDataArr);
	btemp1--;
	btemp1--;
	bIssuerBinArr = tagNode_getTagArrayContent(bCardIssuerDataArr);
#else
	firstTagNodeRef = sdData_getFirstTagNode(sdDataObj);

	tagArrayRef = tagNode_getTagArray(firstTagNodeRef, U32t16(OP_ISSUER_BIN), 0x00, 0x00, MEM_NULL,false);
	bCardIssuerDataArr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tagArrayRef))));
	btemp1 = U16t8(tagNode_getTagArrayLength(bCardIssuerDataArr));

	if(0 == btemp1)
	{
		bCardIssuerDataArr = MEMtPN8(getAssociatedSDTag(OP_ISSUER_BIN, btemp1));
		
		if(bCardIssuerDataArr != ADDR_IS_NULL)//getAssociatedSDTag 有可能返回空，所以这里需要判断返回值
		{			
			bIssuerBinArr = tagNode_getTagArrayContent(bCardIssuerDataArr);//content1
			btemp1 = U16t8(tagNode_getTagArrayLength(bCardIssuerDataArr));
		}
	}
	else
	{
		bIssuerBinArr = tagNode_getTagArrayContent(bCardIssuerDataArr);//content1
	}
	tagArrayRef = tagNode_getTagArray(firstTagNodeRef, U32t16(OP_CARD_ISSUER_DATA), 0x00, 0x00, MEM_NULL,false);
	issuerNumDataArr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tagArrayRef))));
	btemp2 = U16t8(tagNode_getTagArrayLength(issuerNumDataArr));
	if(0 == btemp2)
	{
		issuerNumDataArr = MEMtPN8(getAssociatedSDTag(OP_CARD_ISSUER_DATA, btemp2));
		if(issuerNumDataArr != ADDR_IS_NULL) //getAssociatedSDTag 有可能返回空，所以这里需要判断返回值
		{
			issuerNum = tagNode_getTagArrayContent(issuerNumDataArr);//content1
			btemp2 = U16t8(tagNode_getTagArrayLength(issuerNumDataArr));
		}
	}
	else
	{
		issuerNum = tagNode_getTagArrayContent(issuerNumDataArr);//content1
	}
#endif
	//keyData_getKeyDataById(keySetObj, SCP_STATICKEY_FIRST_KEYID, bKeyType, &keyAddress);


	bConfirmationLen = CONFIRMATION_COUNTER_LEN;

	tempPtr = PX8_A_U16(cacheBuffer, RECEIPTSTARSOFF);

	PX_W_U8(tempPtr, CONFIRMATION_COUNTER_LEN);
	PX8_INC(tempPtr);

	PX_W_U16(tempPtr, confirmationCounter);
	tempPtr = PX8_A_U8(tempPtr, 0x02);

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	PX_W_U8(tempPtr, btemp1);
	PX8_INC(tempPtr);
	memCopyToRam(tempPtr, PX8_A_U8(bIssuerBinArr, 0x02), btemp1);
	tempPtr = PX8_A_U8(tempPtr, btemp1);
#else

	PX_W_U8(tempPtr, U8_A_U8(btemp1, btemp2));
	PX8_INC(tempPtr);

	memCopyToRam(tempPtr, MEMtPM8(PN8_INC(bIssuerBinArr)), btemp1);
	tempPtr = PX8_A_U8(tempPtr, btemp1);
	//btemp1 = 0x04;			// confirmation counter len + SD Uqiue data len ;why?? modified by zm


	memCopyToRam(tempPtr, MEMtPM8(PN8_INC(issuerNum)), btemp2);
	tempPtr = PX8_A_U8(tempPtr, btemp2);
	btemp1 = U8_A_U8(btemp1, btemp2);
#endif
	btemp1 = U8_A_U8(btemp1, U32t8(0x02));//2 = the len of uique data's len + the len of counter's len
	bConfirmationLen = U8_A_U8(bConfirmationLen, btemp1); // confirmation counter len + SD Uqiue data len ;modified by zm


	memCopyAtomic(MEMtPM8(tempPtr) , MEMtPM8(inBuffer), inLength);
	//bReceipteDataLen = U8_A_U8(btemp1, inLength);
	bReceipteDataLen = U16_A_U8(inLength, bConfirmationLen);
	tempPtr = PX8_A_U8(APDUBuffer , 0x02);
	if(KEY_DES_TYPE == bKeyType) // if(KEY_TYPE_SYMMETRIC == bKeyType)两个宏定义的值是一样的，修改之后可读性更好
	{
//		DESSignature.init(DESReceiptKey,Signature.MODE_SIGN);
//		ReceiptLen = (u8)DESSignature.sign(cacheBuffer,RECEIPTSTARSOFF,bReceipteDataLen,apduBuf,(u16)0x02);
		ReceiptLen = U16t8(nativeDESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(&cacheBuffer[RECEIPTSTARSOFF]), bReceipteDataLen, PXtMEM(tempPtr), 0x00, ALG_DES_MAC8_ISO9797_1_M2_ALG3, MODE_SIGN, U8_OR_U8(ALG_DES, SBC_SIGN) ));

	}
#if VM_WITH_RSA
	else if (KEY_TYPE_RSA_PUBLIC == bKeyType)
	{

//		RSA_Signature.init(RSAReceiptKey, Signature.MODE_SIGN);
//		ReceiptLen = (u8)RSA_Signature.sign(cacheBuffer,RECEIPTSTARSOFF,bReceipteDataLen,apduBuf,(u16)0x02);
		ReceiptLen = U16t8(nativeRSASignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_01, &cacheBuffer[RECEIPTSTARSOFF], bReceipteDataLen, tempPtr, 0x00, ALG_RSA_SHA_PKCS1, MODE_SIGN));
	}
#endif
	else if (KEY_AES_TYPE == bKeyType)
	{
		// modify chentao@20211012 合并李鑫修改的AES 收条bug
		//ecpMem_Set(PXtMEM(&cacheBuffer[RECEIPTSTARSOFF - 0x10]), 0x00, 0x10);
		//ReceiptLen = nativeAESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(&cacheBuffer[RECEIPTSTARSOFF - 0x10]), bReceipteDataLen + 0x10, PXtMEM(tempPtr), 0x00,  MODE_SIGN, U8_OR_U8(ALG_AES, SBC_SIGN) );
		ReceiptLen = nativeAESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(&cacheBuffer[RECEIPTSTARSOFF]), bReceipteDataLen, PXtMEM(tempPtr), 0x00,  MODE_SIGN, U8_OR_U8(ALG_AES, SBC_SIGN) ); //Cmac函数中默认ICV为16字节的0
	}
	else if((ECC_PrivateKey_DGI_TYPE == bKeyType))//ECC Private Key
	{
		ReceiptLen = U16t8(nativeECCSignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_01, &cacheBuffer[RECEIPTSTARSOFF], bReceipteDataLen, tempPtr, 0x00, MODE_SIGN));
	}
	else
	{
		throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}

	APDUBuffer[1] = ReceiptLen;
	memCopyAtomic(MEMtPM8(&APDUBuffer[U8_A_U8(ReceiptLen, 2)]), MEMtPM8(&cacheBuffer[RECEIPTSTARSOFF]), U8t16((bConfirmationLen)));
	ReceiptLen = U16t8(U16_AND_U16(U8t16(U8_A_U8(U8_A_U8(ReceiptLen, 0x01), bConfirmationLen)), 0x00ff));

	return ReceiptLen;
}

memref getAssociatedSDTag(u16 tag, u8 len)
{
	#define 			ISD_SD_ID			0x00
	memref				entry;
	jref		X_A		sdRef;
	jref		X_A		sdDataRef;
	jref		X_A		firstTagNodeRef;
	jref		X_A		tagArrayRef;
	PN8					sdDataObj;
	PN8  	 			bCardIssuerDataArr;
	u8                  sdId;

	sdId = g_bSDId[selected_channel];
	bCardIssuerDataArr = 0;

	while(0 == len)
	{
		if(sdId == 0xFF)
		{
			sdId = ISD_SD_ID;
		}
		entry				= getEntryByAppId(sdId);
		sdRef 				= PX_R_U16(get_object_field_ref(entry, theApplet_INDEX));
		sdDataRef 			= sdData_getSDdataRef(sdRef);
		sdDataObj 			= MEMtPN8(locateObjectInMemory(sdDataRef));
		firstTagNodeRef		= sdData_getFirstTagNode(sdDataObj);
		tagArrayRef 		= tagNode_getTagArray(firstTagNodeRef, tag, 0x00, 0x00, MEM_NULL,false);
		bCardIssuerDataArr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tagArrayRef))));
		len 				= U16t8(tagNode_getTagArrayLength(bCardIssuerDataArr));

		if((sdId == ISD_SD_ID) && (len == 0))// add bychentao@20230719 MNOSD 不存在42tag导致死卡 --03.设备版esim测试脚本-移动测试版 - EPX.sol]<29.新增测试用例工程集(通用版)BugList_Https><BugList><BugList_01_01>
		{
			//throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);//找到最顶层都没有找到42 45  直接抛异常 否则会死循环
			return ADDR_IS_NULL; // modify by  chentao@20231123找到最顶层都没有找到42 45，也就是 在MNOSD没有预置42 45 的情况下返回空-- [03.个人esim测试脚本-移动测试版 - EPX.sol]<31.新增测试用例工程集(通用版)BugList_Https><BugList><BugList_01_01>和<BugList_01_02>
		}
		sdId = getAssociateSDId(sdId);
	}

	return PNtMEM(bCardIssuerDataArr);
}
#endif

#if	GP__TOKEN__FOR_MASK
/**
*modify by wuronghua@20210301: 修正计算TOKEN 的方法（运营商都按照国际规范实现了）,注释掉if(!GALITT_TEST)相关内容
*modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
*modify merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 增加INS为0xEA, 增加ECC TOKEN
*/
boolean verifyToken(PX8 buffer, u16 tokenoff, u16 tokenLen, boolean combined_LoadInstallandMakeSelectable, u8 lenTag)
{
	boolean 	X_A		result;
	u8			X_A		bKeyType;
	u8			X_A		tempI;
	u8 			X_A		tmpLen ; // Lc without token
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u8						LC;
#endif
	PN8					sdDataObj;
	PN8					keySetObj;
	PX8					tempPtr1;
	PX8					tempPtr2;

#if GP__VERSION__FOR_CHINA_TELECOM_MASK || GP__VERSION__FOR_CHINA_UNICOM_MASK
	#define   TOKEN_ICCID_LEN     U16_A_U8(U16_A_U8(0x100, 10), 3) //如果支持install的级联，则不能这样做
	u32       X_A          fileAddr;
	u8        X_A          tempInBuffer[TOKEN_ICCID_LEN];
#endif

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u16 				tagLength;
	PN8			tagArrayAField;
#endif

	result = NOK;
	tempI  = 0;

	sdDataObj = MEMtPN8(getCMSDDataObj(TOKEN_KEY_VERSION));

//	sdDataObj = getCurrentSdDataObj(NOT_NEED_CHECK_SD_LOCKED);

	keySetObj = MEMtPX8(getKeySetObj(sdDataObj, TOKEN_KEY_VERSION));

	bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);

	if(0x00 == lenTag)
	{
		tempI = 0x01;
	}
	else if(0x81 == lenTag)
	{
		tempI = 0x02;
	}
	else if(0x82 == lenTag)
	{
		tempI = 0x03;
	}

	if((buffer[OFFSET_INS] == U32t8(0xE4))||(buffer[OFFSET_INS] == U32t8(0xEA)))
	{
		tempI = U8_A_U8(tempI, 6);
	}
	else
	{
		tempI = U8_A_U8(tempI, 5);
	}

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	LC = buffer[OFFSET_LC];
#endif

	buffer[OFFSET_LC] = U16t8(U16_S_U8(tokenoff, tempI));

	tmpLen = U8_A_U8(buffer[OFFSET_LC], 3);  //p1 + p2 + lc + data

	//buffer = (PX8)pointer_to_ref(buffer);
	tempPtr1 = PX8_A_U8(buffer , 0x02);
	tempPtr2 = PX8_A_U8(buffer , tokenoff);
//#if (GP__VERSION__FOR_CHINA_TELECOM_MASK || GP__VERSION__FOR_CHINA_UNICOM_MASK)
	//if(!GALITT_TEST)
//	{
//		//copy ICCID
//		fileAddr =SelectTFFile(DF_MF,EF_ICCID);
//		ReadTFBuf(fileAddr,0x00,tempInBuffer, 0x0A);
//		//copy data
//		ecpMem_Move(&tempInBuffer[10],tempPtr1,tmpLen);
//		//change ptr
//		tempPtr1 = tempInBuffer;
//		tmpLen = U8_A_U8(tmpLen, 10);
//	}
//#endif
#if CMS2AC
//查找seid
	tagArrayAField		= MEMtPN8(GetCardSEIDaddr());
	tagLength			= tagNode_getTagArrayLength(tagArrayAField);
	tagLength--;
	//指向数据域
	tempPtr1 = PX8_A_U8(buffer , 0x05);
	tempPtr2 = PX8_A_U8(buffer , 0x05+tagLength);
	//预留seid的空间
	ecpMem_Move(tempPtr2,tempPtr1,LC);
	//搬移seid长度及value
	memCopyToRam((memref)tempPtr1,  (memref)(tagNode_getTagArrayContent(tagArrayAField)+1), tagLength);
	tmpLen = buffer[OFFSET_LC] + tagLength+3;
	buffer[OFFSET_LC] += tagLength;
	tokenoff += tagLength;
	tempPtr2 = PX8_A_U8(buffer , tokenoff);
	tempPtr1 = PX8_A_U8(buffer , 0x02);
#endif

	if (KEY_TYPE_SYMMETRIC == bKeyType)
	{
//		DESSignature.init(TOKENDESKey, Signature.MODE_VERIFY);
//		if(combined_LoadInstallandMakeSelectable)
//		{
//			DESSignature.update(OPPlatform.InstallForLoadCommandBuffer, (u16)0, (u16)OPPlatform.InstallForLoadCommandLength); // TokenBuffer :install for load data
//		}
		if(DES_TOEKN_LEN != tokenLen)
		{
			throw_error(SW_WRONG_DATA);
		}
		result = nativeDESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(tempPtr1), tmpLen, PXtMEM(tempPtr2), tokenLen, ALG_DES_MAC8_ISO9797_1_M2_ALG3, MODE_VERIFY, U8_OR_U8(ALG_DES, SBC_VERIFY) );
//		if (DESSignature.verify(buffer, (u16)0x02, tmpLen, buffer, tokenoff, tokenLen)) {
//			return true;
//		}
	}
#if VM_WITH_RSA
	else if (KEY_TYPE_RSA_PUBLIC == bKeyType)
	{
//		RSA_Signature.init(TOKENRSAPBkey, Signature.MODE_VERIFY);
//		//tmpLen =(u8)(tmpLen - 0x81);
//		if(combined_LoadInstallandMakeSelectable)
//		{
//			RSA_Signature.update(OPPlatform.InstallForLoadCommandBuffer, (u16)0, (u16)OPPlatform.InstallForLoadCommandLength); // TokenBuffer :install for load data
//		}
		if(RSA_TOEKN_LEN != tokenLen)
		{
			throw_error(SW_WRONG_DATA);
		}
		result = nativeRSASignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_01, tempPtr1, tmpLen, tempPtr2, tokenLen, ALG_RSA_SHA_PKCS1, RSA_VERIFY);
//		if (RSA_Signature.verify(buffer, (u16)0x02, tmpLen, buffer, tokenoff, tokenLen)) {
//			return true;
//		}
	}
#endif
	else if (KEY_AES_TYPE == bKeyType)
	{
		if(AES_TOEKN_LEN != tokenLen)
		{
			throw_error(SW_WRONG_DATA);
		}
		result = nativeAESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(tempPtr1), tmpLen, PXtMEM(tempPtr2), tokenLen, MODE_VERIFY, U8_OR_U8(ALG_AES, SBC_VERIFY) );
	}
	else if (ECC_PublicKey_DGI_TYPE == bKeyType)
	{
		result = nativeECCSignatureDoFinalCal(keySetObj, bKeyType, KEY_ID_01, tempPtr1, tmpLen, tempPtr2, tokenLen, MODE_VERIFY);
	}
	else
	{
		 throw_error(SW_CONDITIONS_NOT_SATISFIED);
	}

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	//去掉seid数据
	tempPtr1 = PX8_A_U8(buffer , 0x05);
	tempPtr2 = PX8_A_U8(buffer , 0x05+tagLength);
	ecpMem_Move(tempPtr1,tempPtr2,LC);
	buffer[OFFSET_LC] = LC;
#endif

	return result;
}

//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
boolean verifyTokenAndReciptKey(void)
{
	#define 			ISD_SD_ID			0x00
	boolean 		X_A		result;
	u8				X_A		bKeyType;
	jref			X_A		sdRef;
	jref			X_A		sdDataRef;
	u32 			X_A		sdEntryAddr;
	PN8						sdDataObj;
	PN8						keySetObj;
	PX8						tempPtr1;
	PX8						tempPtr2;

	sdEntryAddr = getEntryByAppId(ISD_SD_ID);

	sdRef = PX_R_U16(get_object_field_ref(sdEntryAddr, theApplet_INDEX));
	sdDataRef = sdData_getSDdataRef(sdRef);
	sdDataObj = MEMtPN8(locateObjectInMemory(sdDataRef));

	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, TOKEN_KEY_VERSION));
	bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);

	tempPtr1 = PX8_A_U8(APDUBuffer, OFFSET_CDATA);
	if (KEY_TYPE_RSA_PUBLIC != bKeyType)
	{
		//not support DES token, so throw error
		throw_error(SW_WRONG_DATA);
	}
#if VM_WITH_RSA
	else
	{
		tempPtr2 = tempPtr1;
		nativeRSAChiperDoFinalCal(keySetObj, bKeyType, KEY_ID_01, PXtMEM(tempPtr1), RSA_TOEKN_LEN, PXtMEM(tempPtr2), RSA_TOEKN_LEN, ALG_RSA_NOPAD, MODE_DECRYPT);
	}
#endif

	keySetObj = MEMtPN8(getKeySetObj(sdDataObj, RECEIPT_KEY_VERSION));
	bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);

	result = NOK;
	if(KEY_TYPE_SYMMETRIC == bKeyType)
	{
		tempPtr2 = PX8_A_U8(tempPtr1, RSA_TOEKN_LEN);
		result = nativeDESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(tempPtr1), RSA_TOEKN_LEN, PXtMEM(tempPtr2), DES_RECEIPT_LEN, ALG_DES_MAC8_NOPAD, MODE_VERIFY, U8_OR_U8(ALG_DES, SBC_VERIFY) );
	}
	else if (KEY_AES_TYPE == bKeyType)
	{
		result = nativeAESSignatureDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(tempPtr1), RSA_TOEKN_LEN, PXtMEM(tempPtr2), AES_TOEKN_LEN,  MODE_VERIFY, U8_OR_U8(ALG_AES, MODE_VERIFY) );
	}
	else
	{
		//not support rsa ReceiptGeneration, so throw error
		throw_error(SW_WRONG_DATA);
	}
	return result;
}
#endif

#if !GP__VERSION__FOR_MINI_MASK
//modify by jiangqingbin@20220307 同步代码， epx2.0英飞凌电信备案时， 遇到的 在 远程SCP80安装SSD后，再给该SSD put psk key 抛6A83 的问题(缺陷编号366)
boolean decipheredLoadBlockData()
{
	#define DES_CBC_MODE_MASK  0x04

	u8		X_A		offset;
	u8 		X_A 	p1;
	u8 		X_A 	p2;
	u8 		X_A 	mode;
	u16 	X_A 	bLoadBlockLen;
	u8 		X_A 	sbcState;
	u8 		X_A 	bKeyType;
	jref	X_A 	keySetRef;
	jref	X_A 	sdDataRef;

	PN8 keySetObj;
	PN8 sdDataObj;

	memref X_A tempDesBuf;
	memref X_A processState;
	memref X_A ICVBuff;

	sdDataRef = sdData_getSDdataRef(popReference());
	sdDataObj = MEMtPN8(locateObjectInMemory(sdDataRef));
	
	processState = PNtMEM(getJavaObjectDataArray());
	tempDesBuf = PNtMEM(getJavaObjectDataArray());
	ICVBuff = PNtMEM(getJavaObjectDataArray());

	offset = OFFSET_CDATA;
	p1 = APDUBuffer[OFFSET_P1];
	p2 = APDUBuffer[OFFSET_P2];

	bLoadBlockLen = U8t16(APDUBuffer[OFFSET_LC]);
	//bOff = 0;

	// first block init the crypto param
	if( 0x00 == p2 )
	{
		PX_W_U8(processState, SBC_INIT);
		nvmWriteU8NoAtomic(MEMtPN8(tempDesBuf), U8_OR_U8(MODE_DECRYPT, DES_CBC_MODE_MASK));
		memRefresh();
#if GPC_2_3_1
		if(g_icvsize != 0)	//"D3:ICV"参数是否存在
		{
			memCopyToRam(MEMtPX8(ICVBuff), MEMtPX8(g_icvbuf), g_icvsize);
		}
		else
#endif
		{
			memFillToRam(MEMtPX8(ICVBuff), DES_BLOCKSIZE, CLEARED_VALUE);
		}
	}

	sbcState = PX_R_U8(processState);
	mode = PX_R_U8(tempDesBuf);
	U32_INC(tempDesBuf);


	//sdDataObj = getCurrentSdDataObj(IS_NO_NEED_CHECK_SD);
	keySetRef = keySet_getKeySet(sdDataObj, DECIPER_LOAD_FILE_VESION);
	if(REF_NULL == keySetRef)
	{
		return false;
	}
	keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
	bKeyType = keySet_getKeyType(keySetObj, CONTENT_KEY_ID,THROW_ERROR);

	if (KEY_TYPE_SYMMETRIC == bKeyType)
	{
		if(p1 != 0x80)
		{
			if (bLoadBlockLen == 0)
			{
				return false;
			}
			//bLoadBlockLen = 	SimpleDESUpdate(keySetObj, bkeyType, (memref)APDUBuffer, bLoadBlockLen, (memref)cacheBuffer, 0x00, mode, sbcState);
			bLoadBlockLen = nativeDESChiperUpdateCal(keySetObj, KEY_ID_01, PXtMEM(&APDUBuffer[offset]), bLoadBlockLen, PXtMEM(cacheBuffer), MEMtPX8(ICVBuff), MEMtPX8(tempDesBuf), mode, sbcState);
			if ( SBC_INIT == sbcState )
			{
				PX_W_U8(processState, SBC_UPDATE);
			}
		}
		else
		{
			//bLoadBlockLen = SimpleDESDoFinalCal(keySetObj, bkeyType, (memref)APDUBuffer, bLoadBlockLen, (memref)cacheBuffer, 0x00, algType, mode, sbcState);
			bLoadBlockLen = nativeDESChiperDoFinalCal(keySetObj, KEY_ID_01, PXtMEM(&APDUBuffer[offset]), bLoadBlockLen, PXtMEM(cacheBuffer), MEMtPX8(ICVBuff), MEMtPX8(tempDesBuf), ALG_DES_CBC_ISO9797_M2, mode, sbcState);
		}
	}
	else
	{
		// need add rsa ?
		return false;
	}

	ecpMem_Copy(&APDUBuffer[offset], cacheBuffer, bLoadBlockLen);
	APDUBuffer[OFFSET_LC] = U16t8(bLoadBlockLen);

	
	return true;
}
#endif

#if	(GP__RECEIPT__FOR_MASK | GP__TOKEN__FOR_MASK)
memref GetCardSEIDaddr()
{
	jref		X_A		firstTagNodeRef;
	jref		X_A		tagArrayRef;
	jref		X_A		sdRef;
	jref		X_A		sdDataRef;
	u32 		X_A		sdEntryAddr;
	memref				tagArrayAField;
	PN8					sdDataObj;

	sdEntryAddr = getEntryByAppId(0);
	sdRef = PX_R_U16(get_object_field_ref(sdEntryAddr, theApplet_INDEX));
	sdDataRef = sdData_getSDdataRef(sdRef);
	sdDataObj = MEMtPN8(locateObjectInMemory(sdDataRef));
	firstTagNodeRef		= sdData_getFirstTagNode(sdDataObj);
	tagArrayRef			= tagNode_getTagArray(firstTagNodeRef, CARD_ID, 0, 0, MEM_NULL, false);
	if(REF_NULL == tagArrayRef)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
	tagArrayAField		= get_array_data(MEMtPF8(locateObjectInMemory(tagArrayRef)));

	return tagArrayAField;
}

memref getCMSDDataObj(u8 keyVersion)
{
	#define 		ISD_SD_ID			0x00
	u8		X_A		sdId;
	u8 		X_A		sdId_temp;
	u8 		X_A		bSecondPrivilageByte;
	jref 	X_A		bPrivRef ;
	jref	X_A		sdRef;
	jref	X_A		sdDataRef;
	u32 	X_A		sdEntryAddr;
	memref 	X_A		bPrivArray ;
	memref			sdDataObj;

	sdId = g_bSDId[selected_channel];
	sdEntryAddr = getEntryByAppId(sdId);

	while(ISD_SD_ID != sdId)
	{
		bPrivRef = PX_R_U16(get_object_field_ref(MEMtPF8(sdEntryAddr), bPriv_INDEX));
		bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
		bSecondPrivilageByte = PX_R_U8(bPrivArray+1);

		//Authorized Management privilage
		if(0x40 == U8_AND_U8(0x40, bSecondPrivilageByte))
		{
			sdRef = PX_R_U16(get_object_field_ref(sdEntryAddr, theApplet_INDEX));
			sdDataRef = sdData_getSDdataRef(sdRef);
			sdDataObj = locateObjectInMemory(sdDataRef);
			if(REF_NULL != keySet_getKeySet(MEMtPF8(sdDataObj), keyVersion))
			{
				break;
			}
		}

		sdId_temp = getAppAssociatedSdId(MEMtPF8(sdEntryAddr));//PX_R_U8(get_object_field_ref(sdEntryAddr, bsdId_INDEX));

		if(sdId_temp == sdId)
		{
			/* 进入这里说明sdId代表的SD为自关联，且不存在82tag(进入wihle循环已判断)，因此可以直接使用ISD的 */
			sdId = ISD_SD_ID;
		}
		else
		{
			sdId = sdId_temp;
		}
		sdEntryAddr = getEntryByAppId(sdId);
	}

	sdRef = PX_R_U16(get_object_field_ref(sdEntryAddr, theApplet_INDEX));
	sdDataRef = sdData_getSDdataRef(sdRef);
	sdDataObj = locateObjectInMemory(sdDataRef);

	return sdDataObj;

}
#endif
