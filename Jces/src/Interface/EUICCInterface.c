/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC Interface 接口中的函数，变量等，供产业化人员进行修改
*
*@history     :V1.0 :create @ 20181106  
*/

#include "EUICCInterface.h"
#include "MemBackUp.h"
#include "mask.h"
#include "CgmManager.h"
#include "nativemethod_impl.h"
#include "SEEUICCInterface.h"
#include "ecpOperation.h"
#include "NvmDeallocate.h"
#include "euiccConfig.h"
#include "gc.h"
#include "RSA_API.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 


#if (EUICC_ONLY||SE_AND_EUICC)

#if 0  // 勿删，用于参考euiccInfo_section1对应EUICCINFO2_SECTION1; euiccInfo_section2对应EUICCINFO2_SECTION2; euiccInfo_euiccCategory对应EUICCINFO2_CATEGORY; euiccInfo_section3对应EUICCINFO2_SECTION3
u8 CONST euiccInfo_section1[15] = {
0x81,0x03,     0x02,0x01,0x00,          //profileVersion：   T: 81   L:03    V:2.1.0
0x82,0x03,     0x02,0x02,0x00,  	    //SVNVersion:        T: 82   L:03    V: 2.2.0
0x83,0x03,     0x04,0x02,0x00           //euiccFirmwareVer:  T: 83   L:03    V:4.2.0
}; 

u8 CONST euiccInfo_section2[19] = {
0x85,0x03,   0x01,0x7E,0x3A,                 //uiccCapability:         T: 85   L:04    V:  
0x86,0x03,   0x09,0x02,0x00,	                  //ts102241Version:        T: 86   L:03    根据others\test\02 外部测试\eUICC test RSP 20190107\eSIM_Test_CMCC.sol中的配置进行修改
0x87,0x03,   0x02,0x03,0x00,                      //globalplatformVersion : T: 87   L:03    V:2.3.0
0x88,0x02,   0x04,0xD0                            //rspCapability           T: 88   L:02    V:04 90         	
};

u8 CONST euiccInfo_euiccCategory[3] = {0x8B,0x01,0x00};//euiccCategory  T: 8B  L:01 V:00， 根据others\test\02 外部测试\eUICC test RSP 20190107\eSIM_Test_CMCC.sol中的配置进行修改

u8 CONST euiccInfo_section31[0x1D] = {  // 无certificationDataObject字段
0x04,0x03,   0x00,0x01,0x01,	//ppVersion: T: 04   L:03   V:0.1.1
0x0C,0x16,   0x53,0x41,0x53,0x41,0x63,0x63,0x72,0x65,0x64,0x69,0x74,0x61,0x74,0x69,0x6F,0x6E,0x4E,0x75,0x6D,0x62,0x65,0x72	 //sasAcreditationNumber : 0x0C  L: 0x16  V: 后续字节
};

u8 CONST euiccInfo_section32[0x37] = { // 带certificationDataObject字段
0x04,0x03,   0x00,0x01,0x01,         //ppVersion              T: 04   L:03    V:0.1.1
0x0C,0x16,   0x53,0x41,0x53,0x41,0x63,0x63,0x72,0x65,0x64,0x69,0x74,0x61,0x74,0x69,0x6F,0x6E,0x4E,0x75,0x6D,0x62,0x65,0x72,	 //sasAcreditationNumber : 0x04  L: 0x16  V: 后续字节
0xAC,0x18,   0x80,0x0D,0x50,0x6C,0x61,0x74,0x66,0x6F,0x72,0x6D,0x4C,0x61,0x62,0x65,0x6C,0x81,0x07,0x44,0x6C,0x6F,0x61,0x55,0x72,0x6C	 //certificationDataObject : T: AC: L :18 :V :后续字节
};


#endif
	
// u8 CONST defaultCIPkid[0x14] = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
// 	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11
// };	//为兼容SGP02_V3.2以下版本 建立scp03密钥过程中下发的SM-SR Certificate 的“73”tag字段 子tag不带“C9"的情况 ，定义一组默认的CI PKID ，当C9 不存在时找该PKID对应的CI证书校验下发的SM-SR Certificate ，该数组的值必须与个人化时预置的默认证书的PKID一致 
	
/*
*@funtion: GPGetCardRemainMemory
*
*@description:获取卡片的可用空间，包括flash(GPFlash_Mode)或ram(GPRam_Mode)
*
*@param
*
*@return   :memref remainMemory
*
*@History  :V1.0-- 创建该函数
*          :V1.1-- modify by xuxiaojun@20190611 在该函数内部不进行碎片整理。因为该函数可能会在执行过程中被调用，其中有一些局部变量，如果碎片整理之后，变量的实际地址需要重新获取。 <删除RAT/重置RAT/下载RSP版的profile/AuthenticateServer_SendResponseSuccess>
*/
memref  GPGetCardRemainMemory(u8 mode)
{
	memref  X_A  iFreeSize_;

	if(mode == GPFlash_Mode)
	{
#if GP__CGM__FOR_MASK
		if (g_bCheckFlag == CHECK_CLOSE)
		{
			iFreeSize_ = NvmManager_GetAvailMemory();
		}
		else
		{
			iFreeSize_ = CgmField_getSpaceInfo(APP_FIRST, SD_CGM_REMAIN_NVM_SIZE);
		}
#else
		iFreeSize_ = NvmManager_GetAvailMemory();
#endif
	}
	else
	{
#if GP__CGM__FOR_MASK
		if (g_bCheckFlag == CHECK_CLOSE)
		{
			iFreeSize_ = tranAllocate_rtrAvailable();
		}
		else
		{
			iFreeSize_ = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE);
		}
#else
		iFreeSize_ = tranAllocate_rtrAvailable();
#endif
	}

	return iFreeSize_;
}


/*
 *设置PE中安装的appid 用于后续对该APP进行个人化等
 */
void setProfileInstallAppId(void)
{
	u8   I_A   appId;

	appId =  popShort();

	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)
	{
		G_profileGPState.peInstallAppId = appId;
	}
	else
	{
		G_profileGPState.peInstallAppId = APP_NULL;
	}
}

void CopySETableToProfile(void)
{
	u8   I_A   profileIndex,bfreeIndexNum,pkgNum;
	memref  X_A   ProfileEntryAddr,srcAddr,dstAddr;
	u16   I_A   copyLen;
//	u8   X_A   tmp[3];

	bfreeIndexNum = popShort();
	pkgNum = popShort();
	profileIndex =  popShort();

	ProfileEntryAddr = GetProfileEntryAddr(profileIndex);

//	dstAddr = GetProfile_theAppTableValueAddr(ProfileEntryAddr);
//	srcAddr = locateObjectInMemory(memReadU16(MEMtPN8(AM_theAppTable_JC)));
//	srcAddr = get_array_data(MEMtPN8(srcAddr));
//	copyLen = bfreeIndexNum*2;//object数组
//	memCopyAtomic(dstAddr, srcAddr, copyLen);

	dstAddr = GetProfile_pkgTableValueAddr(ProfileEntryAddr);
	srcAddr = locateObjectInMemory(memReadU16(MEMtPN8(PM_f_pkgTable_JC)));
	srcAddr = get_array_data(MEMtPN8(srcAddr));
	copyLen = pkgNum*2;//object数组
	memCopyAtomic(MEMtPM8(dstAddr), MEMtPM8(srcAddr), copyLen);

	dstAddr = GetProfile_pkgLocationTableValueAddr(ProfileEntryAddr);
	srcAddr = PACKAGE_LOCATION_TABLE_ADDRESS;
	copyLen = pkgNum*4;//int 数组
	memCopyAtomic(MEMtPM8(dstAddr), MEMtPM8(srcAddr), copyLen);
/*
	tmp[0] = 0x95;
	tmp[1] = 0x01;
	tmp[2] = PROFILE_CLASS_OPERATIONAL;
	meteDataAddr = GetProfileMetaDataAddr(profileIndex);
	VMWriteBuf(GetMetaData_profileClassAddr(meteDataAddr),tmp,sizeof(tmp));//profile class 设置默认值
*/
}
#if M2M_EUICC
/*
 * 下载profile失败 删除PE的应用数据部分  只剩ISDP
 */
void DeleteProfilePE(u8 deleteProfileIndex)
{
	u8      X_A   backProfilelIndex;
	u8   I_A   bfreeIndexNum,pkgNum,appPkgNum,transActioinFlag;
	memref  X_A   ProfileEntryAddr,srcAddr,dstAddr;
	u16   I_A   ref,tempRef,deleteProfileRef;

	//gcFlag = FALSE;

	deleteProfileRef = GetProfileEntryRef(deleteProfileIndex);

	if(deleteProfileRef == REF_NULL)
	{
		return ;
	}

	ProfileEntryAddr = locateObjectInMemory(deleteProfileRef);//GetProfileEntryAddr(deleteProfileIndex);

	backProfilelIndex = G_CurExecutingProfileIndex;

	if(G_CurExecutingProfileIndex != deleteProfileIndex)
	{
		G_CurExecutingProfileIndex = deleteProfileIndex;

		if(ExecuEnvironmentInit(G_CurExecutingProfileIndex) == FALSE)//profileEntry 不在 ，ISDP还未创建
		{
			return ;
		}
	}

	transActioinFlag = IF_BeginTransaction();

	Euicc_ConfigDataReset(JcvmConfig_GetEUICCRTP_ProfileInitialBack());

	//Profile_pkgTable

	dstAddr = GetProfile_pkgTableValueAddr(ProfileEntryAddr);

	pkgNum = memReadU8(MEMtPM8(EEPkgNum_common_JC));

	ref = memReadU16(MEMtPM8(U32_A_U16(dstAddr,U8_M_U8(pkgNum,SIZE_REF))));

	if(ref != REF_NULL) // 说明profile下载过程中下载了cap包 ，
	{
//		gcFlag = TRUE;
		memFillAtomic(MEMtPM8(U32_A_U16(dstAddr,U8_M_U8(pkgNum,SIZE_REF))),0,U8_M_U8(ON_CARD_PKG_MAX-pkgNum,SIZE_REF));

		//Profile_packageContextTable  初始值FF
		dstAddr = GetProfile_packageContextTableValueAddr(ProfileEntryAddr);
		appPkgNum = memReadU8(MEMtPM8(appletPkgCount_common_JC));
		memFillAtomic(MEMtPM8(U32_A_U8(dstAddr,appPkgNum)),0xFF,(ON_CARD_APPLET_PKG_MAX-appPkgNum));
		nvmWriteU8Atomic(MEMtPN8(GetProfile_appletPkgCountAddr(ProfileEntryAddr)),appPkgNum);

		//Profile_pkgStatus 初始值全0  没有逻辑删除不需要改动

		//Profile_pkgLocationTable
//		dstAddr = GetProfile_pkgLocationTableValueAddr(ProfileEntryAddr);
//		memFillAtomic(MEMtPM8(U32_A_U16(dstAddr,U8_M_U8(pkgNum,SIZE_INT))),0,U8_M_U8(ON_CARD_PKG_MAX-pkgNum,SIZE_INT));
	}

	// Profile_theAppTable
	dstAddr = GetProfile_theAppTableValueAddr(ProfileEntryAddr);

	//如果创建了MNOSD 需要清掉还原成模块的引用
	ref = memReadU16(MEMtPM8(dstAddr));

	srcAddr = locateObjectInMemory(memReadU16(MEMtPM8(Profile_theAppTable_common_JC)));

	srcAddr = get_array_data(MEMtPM8(srcAddr));

	tempRef = memReadU16(MEMtPM8(srcAddr));//备份区引用

	if(ref != tempRef)
	{
//		gcFlag = TRUE;//创建了MNOSD

		memWriteU16Atomic(MEMtPM8(dstAddr),tempRef);
	}

	bfreeIndexNum = memReadU16(MEMtPM8(bFreeEntryIndex_common_JC));

	ref = memReadU16(MEMtPM8(U32_A_U16(dstAddr,U8_M_U8(bfreeIndexNum,SIZE_REF))));

	if(ref != REF_NULL)//新增了可执行模块或者应用
	{
		memFillAtomic(MEMtPM8(U32_A_U16(dstAddr,U8_M_U8(bfreeIndexNum,SIZE_REF))),0,U8_M_U8(APP_END-bfreeIndexNum,SIZE_REF));
		nvmWriteU16Atomic(MEMtPN8(GetProfile_bFreeEntryIndexAddr(ProfileEntryAddr)),bfreeIndexNum);
	}

	initialProfileAppFlag(ProfileEntryAddr , iAppIdFlag_common_JC);//Profile_iAppIdFlag
	manageAppFlag(ISDP_APPID,SET_ID_FLAG);//设置isdp

	initialProfileAppFlag(ProfileEntryAddr , iMutiFlag_common_JC);//Profile_iMutiFlag
	initialProfileAppFlag(ProfileEntryAddr , iExtraditonFlag_common_JC);//Profile_iExtraditonFlag

	IF_CommitTransaction(transActioinFlag);

	profilePESweep(deleteProfileIndex ,deleteProfileRef);

	//Profile_pkgLocationTable 在前面释放会导gc时找不到包组件
	dstAddr = GetProfile_pkgLocationTableValueAddr(ProfileEntryAddr);
	memFillAtomic(MEMtPM8(U32_A_U16(dstAddr,U8_M_U8(pkgNum,SIZE_INT))),0,U8_M_U8(ON_CARD_PKG_MAX-pkgNum,SIZE_INT));

	if(backProfilelIndex != G_CurExecutingProfileIndex)
	{
		G_CurExecutingProfileIndex = backProfilelIndex;
		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	}
}

void initialProfileAppFlag(memref profileEntryAddr , memref srcArrayRefAddr)
{
	u32    X_A     srcAddr,dstAddr;
	u16    X_A     len;

	if(srcArrayRefAddr == iAppIdFlag_common_JC)
	{
		dstAddr = GetProfile_iAppIdFlagValueAddr(profileEntryAddr);
	}
	else if(srcArrayRefAddr == iMutiFlag_common_JC)
	{
		dstAddr = GetProfile_iMutiFlagValueAddr(profileEntryAddr);
	}
	else //iExtraditonFlag_common_JC
	{
		dstAddr = GetProfile_iExtraditonFlagValueAddr(profileEntryAddr);
	}

	srcAddr = locateObjectInMemory(memReadU16(MEMtPM8(srcArrayRefAddr)));

	len = get_array_length(MEMtPM8(srcAddr));

	srcAddr = get_array_data(MEMtPM8(srcAddr));

	memCopyAtomic(MEMtPM8(dstAddr),MEMtPM8(srcAddr), U16_M_U16(len,SIZE_INT));
}
#endif

//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP 该函数被移到其他位置
//u8 KeyDerivationFunction(u8 * shsBuf, u8 *shareInfoBuf, u8 shareInfoLen, u8 keyByteLen ,u8* keyCaclBuf,u8 *keyBuf)

//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP 该函数被移到其他位置
//u8 SHA256DoFinalSingleCall_Ex(memref inAddr,u16 inLen ,u8 *hashResult)

//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP 该函数被移到其他位置,且增加参数u8 algType
//u8 VerifySignForECDSA(u8 * publicKey,u8  publicKeyLen,u8 keyRef, u8  *inBuff, u16  inLen, u8  *sigatureR,u8  *sigatureS,u8  signatureLen )
#if RSP_EUICC
/*
*@description: 计算M 的签名值(r.s)
*
*@see        : :<EllipticCurveCryptographyVersion.pdf> 4.2.1 Signature Algorithm
*
*@function   : CalSignValueForECDSA
*
*@param      :u8 *privateKey
*@param      :u8  privateKeyLen
*@param      :u8  keyRef:曲线参数
*@param      :u8 *inBuff
*@param      :u8 inLen
*@param      :u8 *signOutBuf(即 r s)
*
*@return     :u8 signOutLen or FALSE
*/
u8 CalSignValueForECDSA(u8 *privateKey, u8 privateKeyLen, u8 keyRef, u8 *inBuff, u16 inLen, u8 *signOutBuf)
{
	u8 I_A rLength,sLength;

	if(FALSE == setECDomainParams(keyRef))
	{
		return FALSE;
	}

	setECPubOrPrivKeyRef(TYPE_EC_FP_PRIVATE, privateKey, NULL);

	ECCSign(MEMtPN8(inBuff), inLen, NULL, NULL, ALG_ECDSA_SHA_256,REF_NULL);

	if(EC_VERIFY_FAIL == DecodeSignature(signOutBuf, &rLength, signOutBuf+g_ECKeyStore.b_RLength, &sLength, g_CryptoData.pbBuffer, (u8)g_CryptoData.wBufferLen))
	{
		return FALSE;
	}

	return 2 * g_ECKeyStore.b_RLength;
}
#endif

#if EUICC__FUNCTION__GUOMI_eSIM
/*
*@description: 基于SM2算法计算M 的签名值(r.s)
*
*@function   : CalSignValueForSM2
*
*@param      :u8 *privateKey          --- 输入参数
*@param      :u8 *inBuff              --- 输入参数
*@param      :u8 inLen                --- 输入参数
*@param      :u8 *signOutBuf(即 r s)   --- 输出参数
*
*@return     :SM2_PUBLICKEYLEN
*/
u8 CalSignValueForSM2(u8 *privateKey, u8 *inBuff, u16 inLen, u8 *signOutBuf)
{
	PM8		X_A		tempZa;
	PM8		X_A		tempID;
	u8		D_A		IDlength;
	u8 		X_A		publickey[SM2_PUBLICKEYLEN+1];

	SM2_storePriKey(privateKey); //赋值g_SM2KeyStore.privteKeyBuffer

	tempID = MEMtPM8(defaltID);

	IDlength = 16;

	tempZa = MEMtPM8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));

	SM2_ECP_GenKeyPair(privateKey, publickey); //根据私钥privateKey，计算得到公钥PB=[dB]G = [privateKey]G

	SM2_ECP_GetZa(tempID, IDlength, publickey, tempZa); //求Za

	SM2_Signature(inBuff, inLen, signOutBuf, tempZa, NULL, NULL, SM2_SIGN); //计算签名

	return SM2_PUBLICKEYLEN;
}

/*
*description: 在 InitialiseSecureChannel 时进行SM2密钥交换
*
*@@See      : "国密算法SM2-1.pdf" 第3部分
*              SGP22_V2.2 Annex G  Key Derivation Process (Normative)
*
*@function    :InitialiseSecureChannel_KeyExchangeForSM2
*
*@param       :memref smdpOtpkAddr --- 输入参数 5F49的TAG起始地址，point to the tag of smdpOtpk [APPLICATION 73] OCTET STRING  5F49, 5F49 41 04 V
*@param       :u8 *hostId
*@param       :u8 hostIdLen
*@param       :u8 exchangeKeyLen 输出的密钥总长度
*@param       :u8 *outBuf : tempBuf来的, V结构, 存放密钥交换的值
*
*@return      :u8 errorCode
*/
u8 InitialiseSecureChannel_KeyExchangeForSM2(memref smdpOtpkAddr, u8 *hostId, u8 hostIdLen, u8 exchangeKeyLen, u8 *outBuf)
{
	u8 I_A tmp, certLen[2];
	memref I_A tmpLen;
	memref X_A caAddr;
	u16 I_A ref, certificateLen;

	tmp = ASN1_GetBERTlvLen(smdpOtpkAddr+2, (memref*)&tmpLen);

	if((tmp != 0x01) || (tmpLen != 65) || (VMReadByte(smdpOtpkAddr + 3) != 0x04)) //仅支持非压缩的公钥，当公钥的第一字节为04时，表示当前是非压缩模式，公钥字节本身是64字节长
	{
		return ErrorReason_incorrectInputValues;
	}

	VMReadBuf(tempBuf, smdpOtpkAddr + 2 + tmp + 1, 64); //读取64字节的  otPK.DP.SM2KEYEX, 1是偏移掉0x04

	VMReadBuf(&tempBuf[64], EUICC_OpKeyBuf + EUICC_OpKeySK_Off , 32); //读取 32 字节的  otSK.EUICC.SM2KEYEX

	VMReadBuf(&tempBuf[96], EUICC_OpKeyBuf + EUICC_OpKeyPK_Off , 64); //读取 64字节的  otPK.EUICC.SM2KEYEX

	VMReadBuf(&tempBuf[160], EUICC_AuthenTmpBuf + TempPKDPpb_Off +1, 64); //读取 PK.DPbp.SM2KEYEX 到缓存 tempBuf[160]开始的位置

	caAddr = Certificate_SearchEUICCCerficateByCIPKID(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off + 1, VMReadByte(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off), &ref); //通过CI PKID 查找属于该ci链上的EUICC 证书是否存在，并返回证书链的地址

	if(caAddr == ADDR_IS_NULL)
	{
		return ErrorReason_incorrectInputValues; //CI PKID 对应ci链上的EUICC 证书不存在，抛错
	}

	ecpMem_Copy(&certLen[0], CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(caAddr), 0x02);

	certificateLen = MEM_R_U16((memref)certLen); //BuildWord(&certLen[0]); modify by xuxiaojun@20220521 代码分层，使用平台层函数，而BuildWord是uicc层定义的

	TBSCertificate_GetField((u8 *)(CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(caAddr)+2), certificateLen, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey, &tempBuf[224]); //EUICC证书的PK,tempBuf[224]为长度

	VMReadBuf(&tempBuf[224+SM2_PUBLICKEYLEN+1], CAEUMAndEUICCListGet_skEUICCArraysValueAddr(caAddr)+1, SM2_PRIVEYLEN); //EUICC证书的SK

	SM2_KeyExchange(&tempBuf[224+1], &tempBuf[160], &tempBuf[96], tempBuf, &tempBuf[224+SM2_PUBLICKEYLEN+1], &tempBuf[64], MEMtPM8(EUICCEIDInfo_Addr+1), hostId, EUICCEIDLVLEN-1, hostIdLen, outBuf, exchangeKeyLen, GMSM2KeyExchange_RECIVE_MODE); //SM2密钥交换

	return RESPONSE_NoError;
}


/*
*@description:验证消息签名(r,s )on M
*
*@function   :VerifySignForSM2
*
*@param      :u8 * publicKey    --- 输入参数
*@param      :u8  *inBuff       --- 输入参数
*@param      :u8  inLen         --- 输入参数
*@param      :u8  *signatureRS  --- 输入参数
*
*@return     :TRUE/FALSE
*/
u8 VerifySignForSM2(u8 * publicKey, u8  *inBuff, u16  inLen, u8  *signatureRS)
{
	PM8		X_A		tempZa;
	PM8		X_A		tempID;
	u8		D_A		IDlength;
	u8 		X_A		publickeyTemp[SM2_PUBLICKEYLEN+1];

	publickeyTemp[0] = 0x04;

	ecpMem_Copy(&publickeyTemp[1], publicKey, SM2_PUBLICKEYLEN);

	SM2_storePubKey(publickeyTemp); //赋值g_SM2KeyStore.publicKeyBuffer

	tempID = MEMtPM8(defaltID);

	IDlength = 16;

	tempZa = MEMtPM8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));

	SM2_ECP_GetZa(tempID, IDlength, publickeyTemp+1, tempZa); //求Za ,publicKey skip the Tag 0x04.

	if(SM2_VERIFY_SUCCESS == SM2_Signature(inBuff, inLen, signatureRS, tempZa, NULL, NULL, SM2_VERIFY)) //验签
	{
		return TRUE;
	}

	return FALSE;
}
#endif
#endif

// 以下几个函数在SCP11中也会被使用到，所以调整一下位置
#if (EUICC_ONLY||SE_AND_EUICC || GP__SCP__SCP11__FOR_MASK)
/*
*@description: KDF, 由shs分散出来收条密钥和scp03密钥
*
*@see        :参考GP附录E 4.8.1 与ECC规范, "X9.63 Key Derivation Function"
*
*@function   :KeyDerivation
*
*@param      : u8 * shsBuf      --- 输入数据， shs的值, 64字节长，目前仅支持ECC 256
*@param      : u8 shsBufLen     --- 输入数据，shsBuf中有效数据的字节数
×@param      : u8 * ShareInfoBuf--- 输入数据， 共享数据
×@param		 : u8 ShareInfoLen  --- 输入数据， shareInfoBuf中有效数据的长度 ，最大支持110字节
*@param      : u8 ku8teLen    --- 输入数据， 需要计算后输出的keyBuf中的有效数据长度
*@param      : u8* keyCaclBuf   --- 用于缓存计算过程中间值  shsBuf.x + counter（4字节）+ ShareInfoBuf ，由调用层开缓存 不在本函数内直接使用tempBuf
*@param      : u8 *keyBuf       --- 输出数据
*@param      : u8 eccParaRef    --- 输入数据， ecc曲线参数
*
*@out        : u8 × keyBuf：输出数据
*/
u8 KeyDerivationFunction(u8 * shsBuf, u8 shsBufLen, u8 *shareInfoBuf, u8 shareInfoLen, u8 keyByteLen ,u8* keyCaclBuf,u8 *keyBuf)
{
	memref X_A Counter;
	u8 I_A i, j,off;

	Counter = 1;

	ecpMem_Copy(keyCaclBuf, shsBuf, shsBufLen);	//取shs的x坐标，即前32byte

	if ((keyByteLen % SHA256_DIGEST_SIZE) != 0)  //j = [k/L]向上取整
	{
		j = (keyByteLen / SHA256_DIGEST_SIZE) + 1;
	}
	else
	{
		j = (keyByteLen / SHA256_DIGEST_SIZE);
	}

	off = 0;

	for (i = 1; i <= j; i++)
	{
		MEM_W_U32((memref)(&keyCaclBuf[shsBufLen]),Counter); //SetUWord(&keyCaclBuf[shsBufLen], Counter);

		if (shareInfoBuf != 0)
		{
			ecpMem_Copy(&keyCaclBuf[shsBufLen + 4], shareInfoBuf, shareInfoLen); //todo 这里可以优化，由于这里的数据一直不变，所以可以只在循环前面做一 次即可
		}

		SHA256DoFinalSingleCall_Ex((memref)keyCaclBuf, shsBufLen + 4 + shareInfoLen, &keyBuf[off]);

		off = SHA256_DIGEST_SIZE * i;

		Counter++;
	}
	return off;
}
/*
*@description:验证消息签名(r,s )on M
*
*@see        :<EllipticCurveCryptographyVersion.pdf> 4.2.2 verifiaction Algorithm
*
*@function   :VerifySignForECDSA
*
*@param      :u8  *publicKey     --- 输入参数, 不带04
*@param      :u8  publicKeyLen   --- 输入参数
*@param      :u8  keyRef         --- 输入参数    Card Specification v2.2 Version 1.0.1 – Amendment E :4.5 Preloaded ECC Curve Parameters -> Table 4-3: Key Parameter Reference Values
*@param      :u8  *inBuff        --- 输入参数
*@param      :u8  inLen          --- 输入参数
*@param      :u8  *sigatureR     --- 输入参数
*@param      :u8  *sigatureS     --- 输入参数
*@param      :u8  signatureLen   --- 输入参数  为sigatureR +  sigatureS 的长度
*
*@return     :TRUE/FALSE
*@history    :modify by xuxiaojun@20210816 局部变量固定为0x46可能会越界。 signatureLen的值是所支持的ECC算法的签名值，可能会大于0x40. 测试用例： SCP11测试脚本/SCP11c/C_sm_PSO_ERROR_certificateChainsNotSupported_bySD_384_a1_31_4a
*/
u8 VerifySignForECDSA(u8 * publicKey,u8  publicKeyLen,u8 keyRef, u8  *inBuff, u16  inLen, u8  *sigatureR,u8  *sigatureS,u8  signatureLen ,u8 algType)
{
	u8 I_A sigature[2*MAX_ECC_KEY_BYTELEN + 6]; // 签名的长度与ECC算法有关，可能会大于0x40
	u8 I_A pkValue[2*MAX_ECC_KEY_BYTELEN+1];
	u8 I_A signatureRSLen, num;

	if(FALSE == setECDomainParams(keyRef))
	{
		return FALSE;
	}

	pkValue[0] = 0x04; //添加04
	ecpMem_Copy(&pkValue[1], publicKey, publicKeyLen);

	setECPubOrPrivKeyRef(TYPE_EC_FP_PUBLIC, NULL, pkValue);

	signatureRSLen = signatureLen/2;
	num = 0x00;

	//拼装sigatureR和sigatureS
	sigature[num++] = 0x30;
	sigature[num++] = signatureLen+4;
	sigature[num++] = 0x02;
	sigature[num++] = signatureRSLen;
	ecpMem_Copy(&sigature[num], sigatureR, signatureRSLen); //sigatureR
	num += signatureRSLen;
	sigature[num++] = 0x02;
	sigature[num++] = signatureRSLen;
	ecpMem_Copy(&sigature[num], sigatureS, signatureRSLen); //sigatureS


	if(EC_VERIFY_OK == ECVerify(MEMtPM8(inBuff), inLen, MEMtPM8(sigature), signatureLen+6, NULL, NULL, algType,REF_NULL))
	{
		return TRUE;
	}

	return FALSE;
}
/*
*function:	SHA256DoFinalSingleCall_Ex
*
*description: 单次调用sha256时的操作
*
*para：   memref inAddr  :输入数据
*para：   u8  inLen	    ：输入长度

*@param   u8 *hashResult   32字节的中间状态
*
*@return  :TRUE，结果存在hashResult 中
*/
u8 SHA256DoFinalSingleCall_Ex(memref inAddr,u16 inLen ,u8 *hashResult)
{
	HASHDoFinal(MEMtPM8(inAddr), inLen, MEMtPM8(hashResult), HASH_INIT, NULL, NULL, ALG_SHA_256); //SHA256

	return TRUE;
}
#endif
