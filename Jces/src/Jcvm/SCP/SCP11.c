/*
 * SCP11.c
 *
 *  Created on: 2021-7-9
 *      Author: xuxiaojun
 */

#include "SCPInterface.h"
#include "CardManager.h"
#include "object.h"
#include "Loader.h"
#include "opFunction.h"
#include "global_variable.h"
#include "error.h"
#include "profile.h"
#include "SCP11.h"
#include "nativemethod_impl.h"
#include "SCPGenKeyInterface.h"
#include "SCPDealData.h"
#include "SCPParamCheck.h"
#include "OPCMCal.h"
#include "keySet.h"
#include "SecurityDomainData.h"
#include "casdStoreKey.h"
#include "keyData.h"
#include "dataManager.h"
#include "ecpDefine.h"
#include "EUICCInterface.h"
#include "tagNode.h"
#include "KeyLogicManager.h"
#include "MemOperation.h"
#include "EUICCCustomerCertificate.h"
#include "ASN1Operation.h"
#include "EllipseCurve_API.h"
#include "SEEUICCInterface.h" // warning implicit declaration of function 'Cmac' [-Wimplicit-function-declaration]

#if GP__SCP__SCP11__FOR_MASK

//u8    X_A    tempBuf[512]; deleted by xuxiaojun@20220927: RAM优化
u8 G_SCP11SessionPara[SCP11SessionPara_LENGTH * MAX_LOGICAL_CHANNELS];
PX8 G_TmpCert; // modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR

#if GP__SCP__SCP11__CERT_IN_GLOBAL_RAM
u8 G_TmpCertInRam[TmpCert_LENGTH * MAX_LOGICAL_CHANNELS]; // modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
#endif

/*
*@Function: SCP11PerformSecurityOperation_InitGlobalVar
*
*@Descrip : 初始化Perform SecurityOperation指令执行过程中相关的全局变量
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA
*
*@param	  : void
*
*@Return  : void
*
*@History : V1.0 --- created by xuxiaojun @20210722
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*         : V1.2 --- modify by xuxiaojun@20221101: 该函数适用于当前通道上的操作，不能把其他通道的数据也给清掉
*/
void SCP11PerformSecurityOperation_InitGlobalVar()
{
	ecpMem_Set(G_TmpCert, 0x00, TmpCert_LENGTH);
	
	ecpMem_Set(g_current_SCP11SessionPara, 0x00, SCP11SessionPara_LENGTH);

	ecpMem_Set(g_current_PerformSecurityOperation, 0x00, PerformSecurityOperation_LENGTH);
}
/*
*@Function: AnalyzeA6_83
*
*@Descrip : analyze the validity of tag '83'
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA
*
*@param	  : lvBuffer   --- the length and data of tag '83', LV
*@param	  : keyVersion --- the Key Version Number referenced in tag '83', V
*@param	  : keyId      --- the Key Identifier referenced in tag '83', V
*
*@Return  : return true if 83 is valid, else return false;
*
*@History : V1.0 --- created by qiuping @20210713
*/
u8 SCP11AnalyzeA6_83(PX8 lvBuffer, PX8 keyVersion, PX8 keyId)
{
	u8		len;
	u8		kid;
	u8 		kvn;

	//判断83长度的合法性
	len = lvBuffer[0];
	kid = lvBuffer[1];
	kvn = lvBuffer[2];

	if(len != 2)
	{
		return false;
	}

	if(IsSCP11Key(kvn)) // 调用该函数时，还无法确定key id是公钥的，还是私钥的，所以这里只判断key version
	{
		*keyId = kid;
		*keyVersion = kvn;
		return true;
	}

	return false;
}
/*
*@Function: AnalyzeA6_42
*
*@Descrip : analyze the validity of tag '42'
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA
*
*@param	  : lvBuffer   --- the length and data of tag '42', LV
*@param	  : Identifier --- the CA-KLOC Identifier referenced in tag '42',LV
*
*@Return  : return the bytes of Identifier
*
*@History : V1.0 --- created by qiuping @20210713
*/
u8 SCP11AnalyzeA6_42(PX8 lvBuffer, PX8 Identifier)
{

	u8 		len;

	len = lvBuffer[0];
	//判断42长度的合法性
	if((len >= 0x01) && (len <= 0x10))
	{
		ecpMem_Copy(Identifier, lvBuffer, len);

		return (len + 1);
	}

	return 0x00;
}

/*
*@Function: Analyze92
*
*@Descrip : analyze the validity of tag '92'
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA (Whitelist) Command
*
*@param	  : lvBuffer  --- the length and data of tag '92', LV
*@param	  : counter   --- the Whitelist counter referenced in tag '92',V
*
*@Return  : return TRUE if counter valid; else return FALSE
*
*@History : V1.0 --- created by qiuping @20210713
*/
u8 SCP11Analyze92(PX8 lvBuffer, PX16 counter)
{
	//counter 合法性检查
	u8 		len;
	u16		coun;

	len = lvBuffer[0];
	coun = PX_R_U16(&lvBuffer[1]);
	//2-byte positive integer up to 32767
	if((len != 0x02) || (*counter >= 0x7fff))
	{
		return FALSE;
	}
	if(((coun >= 0x0000) && (coun <= 0x7fff))
		&& (coun > *counter))
	{
		*counter = coun;

		return TRUE;
	}
	return FALSE;
}

/*
*@Function: Analyze70
*
*@Descrip : analyze the validity of tag '70'
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA (Whitelist) Command
*
*@param	  : lvBuffer  --- input, the length and data of tag '70', LV
*
*@Return  : return TRUE if Whitelist is valid; else return FALSE
*
*@History : V1.0 --- created by qiuping @20210713
*         : V1.1 --- modify by xuxiaojun@20220528 未考虑remove white list的情况， 测试用例： SCP11c_1  C_sm_STORE_DATA_Whitelist_SUCCESS_removal_256_a1_60_20
*         : V1.2 --- modify by xuxiaojun@20220923 RAM优化，根据代码逻辑，在白名单合法的情况下，lvBuffer的内容对应的就是白名单的LV结构,所以，没有必要再拷贝一遍
*/
u8 SCP11Analyze70(PX8 lvBuffer)
{
	//whitelist 合法性检查，包括其子TAG的检查
	u32 	len;
	u16		off;
	u8		tag;
	u8		tagLen;
	//获取白名单第一个子tag的偏移以及白名单总长度
	off = ASN1_GetBERTlvLen(PXtMEM(lvBuffer), (u32*)&len);

	if(len == 0x00)
	{
		return TRUE; //GPC_2.3_F_SCP11_v1.3_PublicRelease.pdf:  To remove a whitelist, the  whitelist TLV of the command shall have a length of zero
	}

	len = U32_A_U16(len , off);

	for( ; off < len ; )
	{
		tag = lvBuffer[off];
		tagLen = lvBuffer[off + 1];
		//子tag结构要满足：93 | L(1-16) | V
		if((tag == 0x93) && ((tagLen >= 0x01) && (tagLen <= 0x10)))
		{
			off = off + tagLen + 2;
		}
		else
		{
			return FALSE; // 白名单不合法
		}
	}
	return TRUE;
}


/*
*@Function: SCP11ApplyNewPKCAObj
*
*@Descrip : 申请一个新的 SCP11_PK_CA_KLOC_ECDSA 结构体对象，并把其索引写入链表中的上一个结构体对象的对应位置中
*
*@See     : 
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210726
*/
jref SCP11ApplyNewPKCAObj(PN8 sdDataObj)
{
	jref I_A pkCaRef;
	jref I_A newPkcaRef;
	PN8 I_A pkCaObj;

	newPkcaRef = newSCP11_PK_CA_KLOC_ECDSA();
		
	pkCaRef = sdData_getFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj);

	if (pkCaRef ==  NULL_REF)
	{
		sdData_setFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj, newPkcaRef);
	}
	else
	{
		do
		{
			pkCaObj = MEMtPN8(locateObjectInMemory(pkCaRef));

			pkCaRef = SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(pkCaObj);
			
		}while ( pkCaRef !=  NULL_REF);
		
		SCP11_PK_CA_KLOC_ECDSA_setNextPKCARef(pkCaObj, newPkcaRef);
	}

	return newPkcaRef;
	
}

/*
*@Function: SCP11ApplyNewSKSDObj
*
*@Descrip : 申请一个新的 SCP11_SK_SD_ECKA 结构体对象，并把其索引写入链表中的上一个结构体对象的对应位置中
*
*@See     : 
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210726
*/
jref SCP11ApplyNewSKSDObj(PN8 sdDataObj)
{
	jref I_A skSdRef;
	jref I_A newSkSdRef;
	PN8 I_A skSdObj;

	newSkSdRef = newSCP11_SK_SD_ECKA();
		
	skSdRef = sdData_getFirstSCP11_SK_SD_ECKARef(sdDataObj);

	if (skSdRef ==  NULL_REF)
	{
		sdData_setFirstSCP11_SK_SD_ECKARef(sdDataObj, newSkSdRef);
	}
	else
	{
		while ( skSdRef !=  NULL_REF)
		{
			skSdObj = MEMtPN8(locateObjectInMemory(skSdRef));

			skSdRef = SCP11_SK_SD_ECKA_getNextSKRef(skSdObj);	
		}
		
		SCP11_SK_SD_ECKA_setNextSKRef(skSdObj, newSkSdRef);
	}

	return newSkSdRef;
}

/*
*@Function: SCP11InitKey
*
*@Descrip : scp11key在keySet中有按照标准的ecc方式的存储，在SCP11_PK_CA_KLOC_ECDSA/SCP11_SK_SD_ECKA结构体中仅保存SCP11key的KVN,KID
*
*@See     : GPC2.3-11.8关于标准putkey ecc的格式的APDU数据, 本函数中参考<EPX3.0_GP2.3附录F方案设计.docx>5.2章节关于PK.CA.ECKA的结构体设计,进行存储
*
*@param	  : sdDataObj  --- input, urityDomainData of the performing SD
*@param	  : oldKeyVersion  --- input, curr exist keyVersion
*@param	  : keyVersion ---input,  new keyVersion
*@param	  : keyType      ---input,  new keyType
*@param	  : status      --- input, process mode,such as TAG_NEW_KEY or TAG_UPDATE_KEY_VERSION
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210726
*/
void SCP11InitKey(PN8 sdDataObj, u8 oldKeyVersion, u8 keyVersion, u8 keyId, u8 keyType, u8 status)
{
	jref I_A scp11ObjRef;
	PN8 I_A scp11Obj;
	
	switch(status)
	{
		case TAG_NEW_KEY:		
			
			if (keyType == ECC_PublicKey_DGI_TYPE)
			{
				scp11ObjRef = SCP11GetPKCARefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

				if(scp11ObjRef != NULL_REF)	//indicate key has exist
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}
				
				scp11ObjRef = SCP11ApplyNewPKCAObj(sdDataObj);

				scp11Obj = MEMtPF8( locateObjectInMemory(scp11ObjRef));

				SCP11_PK_CA_KLOC_ECDSA_setKeyVersion(scp11Obj,  keyVersion);

				SCP11_PK_CA_KLOC_ECDSA_setKeyId(scp11Obj, keyId);
			}
			else
			{
				scp11Obj = MEMtPN8(GetSkObjByKeyVerAndKeyId(sdDataObj, keyVersion, keyId));

				if(scp11Obj != ILLEGAL_ADDRESS)	//indicate key has exist
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}
				
				scp11ObjRef = SCP11ApplyNewSKSDObj(sdDataObj);

				scp11Obj = MEMtPF8( locateObjectInMemory(scp11ObjRef));

				SCP11_SK_SD_ECKA_setKeyVersion(scp11Obj,  keyVersion);

				SCP11_SK_SD_ECKA_setKeyId(scp11Obj, keyId);
			}

			break;
		
		case TAG_UPDATE_KEY:
			
			if (keyType == ECC_PublicKey_DGI_TYPE)
			{
				scp11ObjRef = SCP11GetPKCARefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

				if(scp11ObjRef == NULL_REF)	
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}

				scp11Obj = MEMtPF8( locateObjectInMemory(scp11ObjRef));

				SCP11_PK_CA_KLOC_ECDSA_setCAKLOCIdentifierRef(scp11Obj, NULL_REF);					//clear Whitelist
			}
			else
			{
				scp11Obj = MEMtPN8(GetSkObjByKeyVerAndKeyId(sdDataObj, keyVersion, keyId));

				if(scp11Obj == ILLEGAL_ADDRESS)	//modify by xuxiaojun@20220626 修改之前的笔误  indicate key not exist
				{
					keyManager_abortLogicManagerThorwError(SW_WRONG_DATA);
				}

				SCP11_SK_SD_ECKA_setECKACertificateRef(scp11Obj, NULL_REF);						//clear  ecka
			}
			break;
			
		case TAG_UPDATE_KEY_VERSION: 	//replace keyKvn
		
			throw_error(SW_FUNC_NOT_SUPPRORT);

			break;
			
		default:
			break;
	}	
	
}


/*
*@Function: SetDeleteScp11SkKey
*
*@Descrip : scp11key在keySet中有按照标准的ecc方式的删除，在SCP11_PK_CA_KLOC_ECDSA结构体中也删除对应KVN,KID的整个结构体对象
*
*@See     : GPC2.3-11.8关于标准delete key的格式的APDU数据, 本函数中参考<EPX3.0_GP2.3附录F方案设计.docx>5.2章节关于PK.CA.ECKA的结构体设计,进行删除
*
*@param	  : sdDataObj  ---input,  the SecurityDomainData of the performing SD 
*@param	  : keyVersion --- input, keyVersion referenced by the key being delete,
*@param	  : keyId          --- input,  the kid referenced by the key being delete,
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210726
*@History : V1.1 --- modify by xuxiaojun@20220624 修改V1.0里面的笔误
*/
void SetDeleteScp11PkKey(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	jref I_A scp11ObjRef;
	PN8 I_A scp11Obj;
	PN8 I_A preScp11Obj;
	
	preScp11Obj = ILLEGAL_ADDRESS;

	scp11ObjRef = sdData_getFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj);

	while(scp11ObjRef != NULL_REF)
	{
		scp11Obj = MEMtPN8(locateObjectInMemory(scp11ObjRef));

		if (keyVersion == SCP11_PK_CA_KLOC_ECDSA_getKeyVersion(scp11Obj))
		{
			if (keyId == KEY_VERSION_INIT_VALUE)	
			{
				if (preScp11Obj == ILLEGAL_ADDRESS)
				{
					sdData_setFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj, SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(scp11Obj));
				}
				else
				{
					SCP11_PK_CA_KLOC_ECDSA_setNextPKCARef(preScp11Obj, SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(scp11Obj));
				}

				scp11Obj = preScp11Obj;	//当前Obj已不在链表中
			}
			else
			{
				if  (keyId == SCP11_PK_CA_KLOC_ECDSA_getKeyId(scp11Obj))
				{
					if (preScp11Obj == ILLEGAL_ADDRESS)
					{
						sdData_setFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj, SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(scp11Obj));
					}
					else
					{
						SCP11_PK_CA_KLOC_ECDSA_setNextPKCARef(preScp11Obj, SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(scp11Obj));
					}
					
					return;
				}
			}
		}

		if (scp11Obj == ILLEGAL_ADDRESS)
		{
			scp11ObjRef = sdData_getFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj);
		}
		else
		{
			preScp11Obj = scp11Obj;

			scp11ObjRef = SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(scp11Obj);	
		}
	}
}


/*
*@Function: SetDeleteScp11SkKey
*
*@Descrip : scp11key在keySet中有按照标准的ecc方式的删除，在SCP11_SK_SD_ECKA结构体中也删除对应KVN,KID的整个结构体对象
*
*@See     : GPC2.3-11.8关于标准delete key的格式的APDU数据, 本函数中参考<EPX3.0_GP2.3附录F方案设计.docx>5.2章节关于SCP11_SK_SD_ECKA的结构体设计,进行删除
*
*@param	  : sdDataObj  ---input,  the SecurityDomainData of the performing SD 
*@param	  : keyVersion --- input, keyVersion referenced by the key being delete,
*@param	  : keyId          --- input,  the kid referenced by the key being delete,
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210727
*/
void SetDeleteScp11SkKey(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	jref I_A scp11ObjRef;
	PN8 I_A scp11Obj;
	PN8 I_A preScp11Obj;
	
	preScp11Obj = ILLEGAL_ADDRESS;

	scp11ObjRef = sdData_getFirstSCP11_SK_SD_ECKARef(sdDataObj);

	while(scp11ObjRef != NULL_REF)
	{
		scp11Obj = MEMtPN8(locateObjectInMemory(scp11ObjRef));

		if (keyVersion == SCP11_SK_SD_ECKA_getKeyVersion(scp11Obj))
		{
			if (keyId == KEY_VERSION_INIT_VALUE)		//all the indicate keyVersion key  need to be delete
			{
				if (preScp11Obj == ILLEGAL_ADDRESS)
				{
					sdData_setFirstSCP11_SK_SD_ECKARef(sdDataObj, SCP11_SK_SD_ECKA_getNextSKRef(scp11Obj));
				}
				else
				{
					SCP11_SK_SD_ECKA_setNextSKRef(preScp11Obj, SCP11_SK_SD_ECKA_getNextSKRef(scp11Obj));
				}

				scp11Obj = preScp11Obj;	//当前Obj已不在链表中
			}
			else
			{
				if(keyId == SCP11_SK_SD_ECKA_getKeyId(scp11Obj))
				{
					if(preScp11Obj == ILLEGAL_ADDRESS)
					{
						sdData_setFirstSCP11_SK_SD_ECKARef(sdDataObj, SCP11_SK_SD_ECKA_getNextSKRef(scp11Obj));
					}
					else
					{
						SCP11_SK_SD_ECKA_setNextSKRef(preScp11Obj, SCP11_SK_SD_ECKA_getNextSKRef(scp11Obj));
					}
					return;
				}
			}
		}

		if (scp11Obj == ILLEGAL_ADDRESS)
		{
			scp11ObjRef = sdData_getFirstSCP11_SK_SD_ECKARef(sdDataObj);
		}
		else
		{
			preScp11Obj = scp11Obj;

			scp11ObjRef = SCP11_SK_SD_ECKA_getNextSKRef(scp11Obj);
		}
	}
}

/*
*@Function: SCP11DeleteKey
*
*@Descrip : scp11key在keySet中有按照标准的ecc方式的删除，在SCP11_PK_CA_KLOC_ECDSA/SCP11_SK_SD_ECKA结构体中也删除对应KVN,KID的整个结构体对象
*
*@See     : GPC2.3-11.8关于标准DELETE KEY的格式的APDU数据, 本函数中参考<EPX3.0_GP2.3附录F方案设计.docx>5.2章节关于SCP11_PK_CA_KLOC_ECDSA/SCP11_SK_SD_ECKA的结构体设计,进行删除
*
*@param	  : sdDataObj  ---input,  the SecurityDomainData of the performing SD 
*@param	  : keyVersion --- input, keyVersion
*@param	  : keyId          --- input,  
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210726
*/
void SCP11DeleteKey(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	if (keyId == KEY_VERSION_INIT_VALUE)		//all the indicate keyVersion key  need to be delete
	{
		SetDeleteScp11PkKey(sdDataObj, keyVersion, keyId);

		SetDeleteScp11SkKey(sdDataObj, keyVersion, keyId);
	}
	else if((keyId == 0x10)  || ( (keyId >= 0x20) && (keyId <= 0x2F)))	//PK
	{
		SetDeleteScp11PkKey(sdDataObj, keyVersion, keyId);	
	}
	else if ((keyId == 0x11) || (keyId == 0x13) || (keyId == 0x15))	//SK
	{
		SetDeleteScp11SkKey(sdDataObj, keyVersion, keyId);
	}
	else
	{
		return;
	}

	return;

}


/*
*@Function: SCP11StoreWhitelist
*
*@Descrip :
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  6.9 STORE DATA (whitelist) Command
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*@param	  : whitelist  --- the whitelist(L + TLV + TLV + TLV)
*@param	  : keyVersion --- the key version referenced by PK.CA-KLOC.ECDSA
*@param	  : keyId      --- the key identifier referenced by PK.CA-KLOC.ECDSA
*
*@Return  : u16    --- 当前 white list的操作类型，以及数据是否合法
*
*@History : V1.0 --- created by qiuping @20210713
*         : V1.1 --- modify by xuxiaojun@20220528 未考虑到remove white list的操作。 测试用例：SCP11c_1  C_sm_STORE_DATA_Whitelist_SUCCESS_removal_256_a1_60_20
*                    在这个函数内不需要考虑失误，因为被调用的地方还需要考虑counter，这两部分属于同一个事务操作
*         ： V1.2 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 SCP11StoreWhitelist(PN8 sdDataObj, PX8 whitelist, u8 keyVersion, u8 keyId)
{
	jref 	pkcaRef;
	jref    oceCertRef;
	PN8		pkcaObj;
	u16		preWhitelistLen;
	u32		whitelistLen;
	u16		off;
	PX8		preWhitelist;
	jref	tmpRef;
	PX8		tmpPtr;

#define REMOVE_WHITE_LIST  0x00
#define CREATE_WHITE_LIST  0x01
#define REPLACE_WHITE_LIST 0x02

	//Update of the whitelist shall only be accepted if the Security Level is AUTHENTICATED
	if(U8_AND_U8(channelInfo.security_level[selected_channel], SCF_AUTHENTICATED) != SCF_AUTHENTICATED )
	{
		return SW_SECURITY_STATUS_NOT_SATISFIED;
	}

	pkcaRef = SCP11GetPKCARefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

	if(pkcaRef == NULL_REF)
	{
		return SW_REFERENCED_DATA_NOT_FOUND;
	}

	pkcaObj = MEMtPN8(locateObjectInMemory(pkcaRef));

	//When a whitelist is newly stored or replaced, a stored PK.OCE.ECKA shall be deleted by the SD
	oceCertRef = sdData_getSCP11_certRef(sdDataObj);
	
	ecpMem_Set(g_current_SCP11SessionPara_ocePkRef, 0x00, 0x01);
	
	ecpMem_Set(g_current_SCP11SessionPara_ocePKValueLV, 0x00, 2*MAX_ECC_KEY_BYTELEN + 2);

	//获取白名单第一个子tag偏移及总长度
	off = ASN1_GetBERTlvLen(PXtMEM(whitelist), (u32*)&whitelistLen);

	if(SCP11_PK_CA_KLOC_ECDSA_getWhitelistRef(pkcaObj) == NULL_REF)//StoreWhitelist
	{
		if(whitelistLen == 0x00)
		{
			return REMOVE_WHITE_LIST;
		}
		else
		{
			whitelistLen = U32_A_U16(whitelistLen , off);

			if(whitelistLen < 0x04)// if created, the whitelist shall always contain at least one certificate
			{
				return SW_WRONG_DATA;
			}

			if(oceCertRef != NULL_REF)// When a whitelist is newly stored or replaced, a stored PK.OCE.ECKA shall be deleted by the SD
			{
				bRamBuffer[GC_Requested] = 1;

				sdData_setSCP11_certRef(sdDataObj, NULL_REF);
			}

			tmpRef = _newarrayObject(AType_Byte, whitelistLen, BIT_NEED_REFRESH);
			tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

			nvmCopyAtomic(MEMtPM8(tmpPtr), whitelist, U8t16(whitelistLen));
			SCP11_PK_CA_KLOC_ECDSA_setWhitelistRef(pkcaObj, tmpRef);

			return CREATE_WHITE_LIST;
		}
	}
	else//replaceWhitelist
	{
		if(oceCertRef != NULL_REF) //When a whitelist is newly stored or replaced, a stored PK.OCE.ECKA shall be deleted by the SD
		{
			bRamBuffer[GC_Requested] = 1;

			sdData_setSCP11_certRef(sdDataObj, NULL_REF);
		}

		if(whitelistLen == 0x00)
		{
			SCP11_PK_CA_KLOC_ECDSA_setWhitelistRef(pkcaObj, REF_NULL);

			return REMOVE_WHITE_LIST;
		}
		else
		{
			preWhitelist = MEMtPN8(SCP11_PK_CA_KLOC_ECDSA_getWhitelistDataObj(pkcaObj));
			preWhitelistLen = U16t8(get_array_length(preWhitelist));

			if(preWhitelistLen < whitelistLen)
			{
				bRamBuffer[GC_Requested] = 1;

				tmpRef = _newarrayObject(AType_Byte, whitelistLen, BIT_NEED_REFRESH);
				tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

				nvmCopyAtomic(MEMtPM8(tmpPtr), whitelist, U8t16(whitelistLen));
				SCP11_PK_CA_KLOC_ECDSA_setWhitelistRef(pkcaObj, tmpRef);

			}
			else
			{
				ecpMem_Copy(preWhitelist, whitelist, whitelistLen);
			}

			return REPLACE_WHITE_LIST;
		}
	}
}

/*
*@Function: SCP11StoreIdentifier
*
*@Descrip :
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  6.9 STORE DATA (CA-KLOC Identifier) Command
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*@param	  : Identifier --- the CA-KLOC Identifier, LV
*@param	  : keyVersion --- the key version referenced by PK.CA-KLOC.ECDSA
*@param	  : keyId      --- the key identifier referenced by PK.CA-KLOC.ECDSA
*
*@Return  : void
*
*@History : V1.0 --- created by qiuping @20210713
*/
void SCP11StoreIdentifier(PN8 sdDataObj, PX8 Identifier,u8 keyVersion, u8 keyId)
{
	jref 	pkcaRef;
	jref 	tmpRef;
	jref 	caIdRef;
	PN8		pkcaObj;
	PX8		tmpPtr;
	u8		identifierLen;
	u8		caIdLen;
	u8		transactionStarted;

	// 通过keyVersion和keyId定位到唯一的SCP11_PK_CA_KLOC_ECDSA
	pkcaRef = SCP11GetPKCARefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

	identifierLen = Identifier[0];
	identifierLen = U8_A_U8(identifierLen , 1);

	if(pkcaRef == REF_NULL)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
	else // Store or Replace Identifier
	{
			//需要开事务
		if (!isInProgress())
		{
			begin_transaction(SINGLE_APDU_PROCESS);
			transactionStarted = 1;
		}
		
		pkcaObj = MEMtPN8(locateObjectInMemory(pkcaRef));

		caIdRef = SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierRef(pkcaObj);

		if(caIdRef == NULL_REF)	//StoreIdentifier
		{
			tmpRef = _newarrayObject(AType_Byte, identifierLen, BIT_NEED_REFRESH);
			tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

			nvmCopyAtomic(MEMtPM8(tmpPtr), Identifier, U8t16(identifierLen));
			SCP11_PK_CA_KLOC_ECDSA_setCAKLOCIdentifierRef(pkcaObj, tmpRef);
		}
		else	//ReplaceIdentifier
		{
			caIdLen = U16t8(get_array_length(locateObjectInMemory(caIdRef)));

			if(caIdLen < identifierLen)
			{
				bRamBuffer[GC_Requested] = 1;
				tmpRef = _newarrayObject(AType_Byte, identifierLen, BIT_NEED_REFRESH);
				tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

				nvmCopyAtomic(MEMtPM8(tmpPtr), Identifier, U8t16(identifierLen));
				SCP11_PK_CA_KLOC_ECDSA_setCAKLOCIdentifierRef(pkcaObj, tmpRef);
			}
			else
			{
				ecpMem_Copy(caIdRef, Identifier, identifierLen);
			}
		}
		
		if (transactionStarted)
		{
			commit_transaction();
		//需要提交事务
		}

	}	
}

/*
*@Function: SCP11PersistentlyStore5F20AndOcePk
*
*@Descrip :
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*@param	  : scpOptions --- current sd scp11 option
*
*@Return  : return SW_NO_ERROR if store success, else return others;
*
*@History : V1.0 --- created by qiuping @20210824
*/
u16 SCP11PersistentlyStore5F20AndOcePk(PN8 sdDataObj,u8	scpOptions)
{
	jref 	certRef;
	jref 	tmpRef;
	PN8		certObj;
	PX8		tmpPtr;
	PX8		preSubId;
	PX8		preOcePk;
	u8		preSubIdLen;
	u8		preOcePkLen;
	u8		transactionStarted;
	u8      tmpOceSubjectIdLen;
	
	tmpOceSubjectIdLen = PX_R_U8(g_current_SCP11SessionPara_oceSubjectIdLv);

	if ((scpOptions & PERSISTENTLY_STORES_PK_OCE) != PERSISTENTLY_STORES_PK_OCE)
	{
		return SW_NO_ERROR;
	}

	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}

	certRef = sdData_getSCP11_certRef(sdDataObj);
	if( certRef == NULL_REF )
	{
		certRef = newSCP11_CERT_OCE_ECKA();

		sdData_setSCP11_certRef(sdDataObj, certRef);

	}

	certObj = MEMtPN8(locateObjectInMemory(certRef));

	//stored Subject Identifier persistently
	if(SCP11_CERT_OCE_ECKA_getTag_5F20Ref(certObj) == NULL_REF)//Store5F20
	{

		tmpRef = _newarrayObject(AType_Byte, tmpOceSubjectIdLen + 1, BIT_NEED_REFRESH);
		tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

		nvmCopyAtomic(MEMtPM8(tmpPtr), g_current_SCP11SessionPara_oceSubjectIdLv, U8t16(tmpOceSubjectIdLen + 1));
		SCP11_CERT_OCE_ECKA_setTag_5F20Ref(certObj, tmpRef);
	}
	else//replace5F20
	{
		preSubId = MEMtPN8(SCP11_CERT_OCE_ECKA_getTag_5F20DataObj(certObj));
		preSubIdLen = U16t8(get_array_length(preSubId));

		if(preSubIdLen < (tmpOceSubjectIdLen + 1))
		{
			bRamBuffer[GC_Requested] = 1;

			tmpRef = _newarrayObject(AType_Byte, tmpOceSubjectIdLen + 1, BIT_NEED_REFRESH);
			tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

			nvmCopyAtomic(MEMtPM8(tmpPtr), g_current_SCP11SessionPara_oceSubjectIdLv, U8t16(tmpOceSubjectIdLen + 1));
			SCP11_CERT_OCE_ECKA_setTag_5F20Ref(certObj, tmpRef);

		}
		else
		{
			ecpMem_Copy(preSubId, g_current_SCP11SessionPara_oceSubjectIdLv, tmpOceSubjectIdLen + 1);
		}
	}

	//stored PK.OCE.ECKA persistently
	if(SCP11_CERT_OCE_ECKA_getPkValueOfOCERef(certObj) == NULL_REF)//Store PK.OCE.ECKA
	{

		tmpRef = _newarrayObject(AType_Byte, tmpOceSubjectIdLen + 1, BIT_NEED_REFRESH);
		tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

		nvmCopyAtomic(MEMtPM8(tmpPtr), MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), U8t16(tmpOceSubjectIdLen + 1));
		SCP11_CERT_OCE_ECKA_setPkValueOfOCERef(certObj, tmpRef);
	}
	else//replace PK.OCE.ECKA
	{
		preOcePk = MEMtPN8(SCP11_CERT_OCE_ECKA_getPkValueOfOCERef(certObj));
		preOcePkLen = U16t8(get_array_length(preOcePk));

		if(preOcePkLen < (tmpOceSubjectIdLen + 1))
		{
			bRamBuffer[GC_Requested] = 1;

			tmpRef = _newarrayObject(AType_Byte, tmpOceSubjectIdLen + 1, BIT_NEED_REFRESH);
			tmpPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

			nvmCopyAtomic(MEMtPM8(tmpPtr), MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), U8t16(tmpOceSubjectIdLen + 1));
			SCP11_CERT_OCE_ECKA_setPkValueOfOCERef(certObj, tmpRef);

		}
		else
		{
			ecpMem_Copy(preSubId, MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), tmpOceSubjectIdLen + 1);
		}
	}

	if (transactionStarted)
	{
		commit_transaction();
	//需要提交事务
	}

	return SW_NO_ERROR;
}


/*
*@Function: SCP11GetPersistently5F20AndOcePk
*
*@Descrip : get 5F20 And OcePk from EEP, use  g_SCP11SessionPara.ocePKValueLV and  g_SCP11SessionPara.oceSubjectIdLv store the get data
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*
*@Return  : return SW_NO_ERROR if get success, else return others;
*
*@History : V1.0 --- created by jiangqingbin @20210827
*/
u16 SCP11GetPersistently5F20AndOcePk(PN8 sdDataObj)
{
	jref 	certRef;
	PN8	certObj;
	
	jref dataOcePkRef;
	PN8 dataOcePkAddr;
	
	jref data5F20Ref;
	PN8 data5F20Addr;	

	certRef = sdData_getSCP11_certRef(sdDataObj);

	if (certRef != NULL_REF)
	{
		certObj = MEMtPN8(locateObjectInMemory(certRef));

		//ocepk
		dataOcePkRef = SCP11_CERT_OCE_ECKA_getPkValueOfOCERef(certObj);

		if (dataOcePkRef == NULL_REF)
		{
			return SW_CONDITIONS_NOT_SATISFIED;	
		}

		dataOcePkAddr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(dataOcePkRef))));

		PX_W_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), memReadU8(dataOcePkAddr));

		ecpMem_Copy(PX8_A_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), 1),  dataOcePkAddr+1, PX_R_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV)));
		
		//5f20
		data5F20Ref = SCP11_CERT_OCE_ECKA_getTag_5F20Ref(certObj);
		
		if (data5F20Ref == NULL_REF)
		{
			return SW_CONDITIONS_NOT_SATISFIED;	
		}
		
		data5F20Addr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(data5F20Ref))));

		PX_W_U8(g_current_SCP11SessionPara_oceSubjectIdLv, memReadU8(data5F20Addr));

		ecpMem_Copy(PX8_A_U8(g_current_SCP11SessionPara_oceSubjectIdLv, 1),  data5F20Addr+1, PX_R_U8(g_current_SCP11SessionPara_oceSubjectIdLv));

		return SW_NO_ERROR;
		
	}

	return SW_CONDITIONS_NOT_SATISFIED;
}


/*
public class SCP11_PK_CA_KLOC_ECDSA {
	byte keyVersion;
    byte keyId;

    short  internalCounter;

    // When a PK.CA-KLOC.ECDSA is deleted, a whitelist linked to it shall also be deleted by the SD
    byte[] Whitelist; //L + TLV + TLV + TLV

    byte[] pkValueOfCA; // L + V

    // When PK.CA-KLOC.ECDSA  is  deleted  or  replaced,  the CA-KLOC Identifier  store shall  be  automatically deleted by the SD
    byte[] CA_KLOC_Identifier; // 17 bytes; LV

    SCP11_CERT_OCE_ECKA cert; // When a whitelist is newly stored or replaced, a stored PK.OCE.ECKA shall be deleted by the SD

    SCP11_PK_CA_KLOC_ECDSA nextPK_CA_KLOC_ECDSA;
}
public class SCP11_PK_CA_KLOC_ECDSA {
	byte[] PK.OCE.ECKA;	//7f49
	byte[] Subject Identifier;	//5f20
}
*/
/*
*@Function: SCP11GetKIDandKVNByCAID
*
*@Descrip : retrieve from  the  SD the KID and the KVN corresponding  to  a  given  CA-KLOC Identifier
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.3  GET DATA (CA-KLOC KID-KVN) Command
*
*@param	  : sdDataObj --- the SecurityDomainData of the performing SD
*@param	  : buffer    --- out buffer; the data of ECKA Certificate, LV
*@param	  : bufSize   --- the maximum of buffer
*
*@Return  : return the bytes of buffer
*
*@History : V1.0 --- created by qiuping @20210712
*/
u8 SCP11GetKIDandKVNByCAID(PN8 sdDataObj, PX8 buffer, u16 bufSize)
{
	jref 	pkcaRef;
	PN8		pkcaObj;
	PX8		tempBuff;
	PX8		caIdBuff;
	u8		a6Len;
	u8		caIdLen;

	//判断指令数据域有效性
	if((APDUBuffer[APDU_DATA_OFFSET] != 0xA6) || (APDUBuffer[APDU_DATA_OFFSET + 2] != 0x42))
	{
		throw_error(SW_WRONG_DATA);
	}
	a6Len = APDUBuffer[APDU_DATA_OFFSET + 1];
	if((a6Len < 0x03) || (a6Len > 0x12))
	{
		throw_error(SW_WRONG_DATA);
	}
	caIdLen	= APDUBuffer[APDU_DATA_OFFSET + 3] + 1;
	caIdBuff = &APDUBuffer[APDU_DATA_OFFSET + 3];

	pkcaRef = sdData_getFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj);

	while(pkcaRef != NULL_REF)
	{
		pkcaObj =  MEMtPN8(locateObjectInMemory(pkcaRef));
		tempBuff = MEMtPN8(SCP11_PK_CA_KLOC_ECDSA_getCAKLOCIdentifierDataObj(pkcaObj));
		tempBuff = MEMtPN8(get_array_data(tempBuff));

		if(memCompare(tempBuff,caIdBuff,caIdLen) == ECP_CMP_RESULT_EQUAL)
		{
			buffer[0] = 0x83;
			buffer[1] = 0x02;
			buffer[2] = SCP11_PK_CA_KLOC_ECDSA_getKeyId(pkcaObj);
			buffer[3] = SCP11_PK_CA_KLOC_ECDSA_getKeyVersion(pkcaObj);
			return 4;
		}

		pkcaRef = SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(pkcaObj);
	}
	return 0;
}


/*
*@Function: SCP11GetCAKLOCIdentifier
*
*@Descrip : retrieve from  the  SD the KID and the KVN corresponding  to  a  given  CA-KLOC Identifier
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.3  GET DATA (CA-KLOC KID-KVN) Command
*
*@param	  : sdDataObj 	--- the SecurityDomainData of the performing SD
*@param	  : keyVersion  --- the keyVersion of PK_CA_KLOC_ECDSA
*@param	  : keyId   	--- the keyId of PK_CA_KLOC_ECDSA
*
*@Return  : return the SCP11_PK_CA_KLOC_ECDSA reference corresponding  to  a  given  CA-KLOC Identifier
*
*@History : V1.0 --- created by qiuping @20210712
*/
jref SCP11GetPKCARefByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	jref	pkRef;
	PN8		pkObj;

	pkRef = sdData_getFirstSCP11_PK_CA_KLOC_ECDSARef(sdDataObj);

	while(pkRef != NULL_REF)
	{
		pkObj = MEMtPN8(locateObjectInMemory(pkRef));

		if((keyVersion == SCP11_PK_CA_KLOC_ECDSA_getKeyVersion(pkObj))
				&& (keyId == SCP11_PK_CA_KLOC_ECDSA_getKeyId(pkObj)))
		{
			return pkRef;
		}

		pkRef = SCP11_PK_CA_KLOC_ECDSA_getNextPKCARef(pkObj);

	}

	return NULL_REF;
}


/*
public class SCP11_SK_SD_ECKA {
	byte keyVersion;
	byte keyId;
	byte[] skValue;		L(1 BYTE)V
	byte[] dekValue;	L(1 BYTE)V

	//Each  SK.SD.ECKA must have one associated CERT.SD.ECKA
    //When the SK.SD.ECKA is deleted or replaced, the SCP11 certificate store shall be automatically deleted by the SD
     byte[] ECKACertificate;		L(2 BYTE)V

     SCP11_SK_SD_ECKA nextSK_SD_ECKA;
}
*/

/*
*@Function: AnalyzeA6_42
*
*@Descrip : analyze the validity of tag 'BF21'
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA
*
*@param	  : lvBuffer   --- the length and data of tag 'BF21', LV
*@param	  : certificate--- the CA-KLOC Identifier referenced in tag '42'
*
*@Return  : return the bytes of certificate
*
*@History : V1.0 --- created by jiangqingbin @20210713， 目前方案是写入sd的ecka证书时不验证合法性，证书合法性由数据中心保证(或其他手段保证)，本接口先保留，可能后续实际应用中会有需求时再完善
*/

u8 SCP11AnalyzeBF21(PX8 lvBuffer, PX8 certificate)
{
	// 验证证书的有效性
	// starting from  the one signed  by CA-KLCC and  ending with CERT.SD.ECKA
	return 0;
}

/*
*@Function: SCP11StoreIdentifier
*
*@Descrip : store sd cert(chaining)
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  6.9 STORE DATA (CA-KLOC Identifier) Command
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*@param	  : certificate--- the ECKA Certificate begin from BF21
*@param	  : keyVersion --- the key version referenced by SK.SD.ECKA
*@param	  : keyId      --- the key identifier referenced SK.SD.ECKA
*@param	  : currStoreLen ---curr APDU remain length
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210713
*/
void SCP11StoreECKACert(PN8 sdDataObj, PX8 certificate, u8 keyVersion, u8 keyId, u16 currStoreLen)
{
	jref eckaRef;
	PN8 skObj;
	u32 certChainingLen;
	memref CertAddr;
	u16 offSet;
	u16 storeAllLength;
	u32 exitLength;
	u8 scpOptions;
	u8 tagValue[2];
	u8 tagAndLByte;
	u32 firstCertLength;
	
	offSet = 2;	//point to L of  BF21

	eckaRef = REF_NULL;

	offSet += (u16)ASN1_GetBERTlvLen(PXtMEM(certificate+offSet),  (u32*)&certChainingLen);

	storeAllLength = offSet + certChainingLen;	// LV: 2(all len) +offSet(BF21 +certChainingLen code in byte)  + certChainingLen(value len)

	tagAndLByte = ASN1_GetBERTlvTag(PXtMEM(certificate+offSet),  tagValue);

	if ((tagAndLByte != 2) ||(tagValue[0] != 0x7f) ||(tagValue[1] != 0x21) )
	{
		throw_error(SW_WRONG_DATA);
	}

	tagAndLByte += ASN1_GetBERTlvLen(PXtMEM(certificate+offSet+tagAndLByte),  (u32*)&firstCertLength);

	firstCertLength += tagAndLByte;

	if (certChainingLen > firstCertLength)	//不止一个证书，即证书链
	{
		scpOptions = sdData_getScpOptionByScpType(sdDataObj, SCP_11);

		if ( (scpOptions == 0xFF) || ((scpOptions & CERT_CHAIN_SUPPORTED) != CERT_CHAIN_SUPPORTED) )	 //sd不支持scp11协议或者不支持证书链形式证书
		{
			throw_error(SW_WRONG_DATA);
		}
	}

	//获取kid，kvn对应的skSDObj的索引
	skObj = MEMtPN8(GetSkObjByKeyVerAndKeyId(sdDataObj, keyVersion, keyId));

	if(skObj == ILLEGAL_ADDRESS)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	eckaRef = SCP11_SK_SD_ECKA_getECKACertificateRef(skObj);	

	begin_transaction(MULTIL_APDU_PROCESS);

	if(eckaRef == REF_NULL)
	{
		//创建对象
		eckaRef	= _newarrayObject(AType_Byte, storeAllLength, BIT_NEED_REFRESH);

		CertAddr = get_array_data(MEMtPF8(locateObjectInMemory(eckaRef)));
	}
	else 
	{
		CertAddr = get_array_data(MEMtPF8(locateObjectInMemory(eckaRef)));

		exitLength = get_array_length(MEMtPF8(locateObjectInMemory(eckaRef)));//modify by xuxiaojun@20220611

		if (storeAllLength <= exitLength)	//原有证书空间可以存储新的证书
		{
			//write data in exit eckaRef Addr
		}
		else								//原有证书空间过小， 不可以存储新的证书
		{
			eckaRef	= _newarrayObject(AType_Byte, storeAllLength, BIT_NEED_REFRESH);

			CertAddr = get_array_data(MEMtPF8(locateObjectInMemory(eckaRef)));

			bRamBuffer[GC_Requested] = 1; 		//需要GC流程标记
		}
	}

	//写入索引
	SCP11_SK_SD_ECKA_setECKACertificateRef(skObj, eckaRef);

	//写入数据
	if (storeAllLength <= currStoreLen)
	{
		nvmCopyAtomic(MEMtPM8(CertAddr), certificate, storeAllLength);

		commit_transaction();
	}
	else		//NEED MORE APDU
	{
		nvmCopyAtomic(MEMtPM8(CertAddr), certificate, currStoreLen);

		g_tagArrayContent[selected_channel] = CertAddr;

		g_curStoreDataTagOffset[selected_channel] = currStoreLen;	//已写入长度

		g_curStoreDataTagLength[selected_channel] = storeAllLength-currStoreLen;	//剩余还有多少数据没有写入

		g_storeDataState[selected_channel] = NEXT_STORE_DATA;

		//级联指令在主函数dataManager_storeData处理最后一条指令，状态是STORE_DATA_FINISH 时会进行 commit_transaction();
	}

}
/*
*@Function: GPGetECKACert
*
*@Descrip : get the data of ECKA Certificate by GET DATA Command
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.2  GET DATA (ECKA Certificate) Command
*
*@param	  : sdDataObj --- the SecurityDomainData of the performing SD
*@param	  : buf       --- out buffer; the data of ECKA Certificate
*@param	  : bufSize   --- the maximum of buf
*@param	  : *certRef  --- the cert ref addr
*@param	  : certDataOffSet   ---the cert Data has been read
*
*@Return  : return the bytes of buf
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
u16 SCP11GetECKACert(PN8 sdDataObj, PX8 buf, u16 bufSize, jref *certRef, u16 certDataOffSet)
{
	u8 keyVersion;
	u8 keyId;
	jref eckaCertRef;
	memref bufferFeild;
	u32	arrayLen;
	u8	tempLen;

	if ((*certRef == NULL_REF) && (certDataOffSet == 0))	//the first time call this func
	{
		// 判断数据域的合法性
		if  ((APDUBuffer[OFFSET_CDATA] != 0xA6)
			|| (APDUBuffer[OFFSET_CDATA+1] != 0x04)
			|| (APDUBuffer[OFFSET_CDATA+2] != 0x83)
			|| (APDUBuffer[OFFSET_CDATA+3] != 0x02) )
		{
			throw_error(SW_WRONG_DATA);
		}

		// 解析数据域，得到key version和key id
		keyId = APDUBuffer[OFFSET_CDATA+4];

		keyVersion = APDUBuffer[OFFSET_CDATA+5];

		if  ((keyId != 0x11) && (keyId != 0x13)&&(keyId != 0x15))//scp11 sdsk kid
		{
			throw_error(SW_WRONG_DATA);
		}

		if (!IsSCP11Key(keyVersion))	//scp11 sdsk kvn
		{
			throw_error(SW_WRONG_DATA);
		}

		//获取kid，kvn对应的ecka的索引
		eckaCertRef = GetECKACertRefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

		if (eckaCertRef == NULL_REF)
		{
			throw_error(SW_REFERENCED_DATA_NOT_FOUND); //SCP11测试脚本:  C_sm_STORE_DATA_ECKA_Cert_ERROR_SK_SD_ECKA_does_not_exist_KID11h_256_a1_20_03
		}

		bufferFeild = get_array_data(MEMtPF8(locateObjectInMemory(eckaCertRef))); // 存储格式为BF21 LV

		tempLen = ASN1_GetBERTlvLen(PXtMEM(bufferFeild + 2),  (u32*)&arrayLen);

		arrayLen += (tempLen + 2);

		// 判断证书长度是否大于bufSize
		if (arrayLen <= bufSize )
		{
			ecpMem_Copy(buf, bufferFeild, arrayLen);

			return arrayLen;
		}
		else
		{
			ecpMem_Copy(buf, bufferFeild, bufSize);

			*certRef = eckaCertRef;

			return bufSize;
		}
	}
	//TODO:本函数的调用函数目前不支持级联上发一个tag的数据，此问题后续需要时再增加支持级联发送...
	else if ((*certRef != NULL_REF) && (certDataOffSet != 0))	//not the first time call this func
	{
		bufferFeild = get_array_data(MEMtPF8(locateObjectInMemory(*certRef)));

		tempLen = ASN1_GetBERTlvLen(PXtMEM(bufferFeild + 2), (u32*)&arrayLen);

		arrayLen += (tempLen + 2);

		if ((arrayLen - certDataOffSet) <= bufSize)
		{
			ecpMem_Copy(buf, bufferFeild + certDataOffSet, arrayLen - certDataOffSet);

			*certRef = NULL_REF;

			return arrayLen - certDataOffSet;
		}
		else
		{
			ecpMem_Copy(buf, bufferFeild + certDataOffSet, bufSize);

			return bufSize;
		}
	}
	else
	{
		return 0;
	}

}


/*
*@Function: GetDekRefByKeyVerAndKeyId
*
*@Descrip : get the reference of dek which referenced the keyVersion and keyId
*
*@See     : <EPX3.0_GP2.3附录F方案设计.docx>5.1章节关于CERT.SD.ECKA的结构体设计
*
*@param	  : sdDataObj  --- input, the SecurityDomainData of the performing SD
*@param	  : keyVersion --- input, the Key Version Number of DEK
*@param	  : keyId      --- input, the Key Identifier of DEK
*
*@Return  : return the ref linked to the Dek which is referenced by the keyVersion and keyId
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
u16 GetDekByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	return keyData_getKeyData(g_bSDId[selected_channel], keyVersion, keyId, PNtMEM(NULL));
}



/*
*@Function: GetCertRefOfECKAByKeyVerAndKeyId
*
*@Descrip : get the reference of SK.SD.ECKA which referenced the keyVersion and keyId
*
*@See     : <EPX3.0_GP2.3附录F方案设计.docx>5.1章节关于CERT.SD.ECKA的结构体设计
*
*@param	  : sdDataObj --- input, the SecurityDomainData of the performing SD
*@param	  : keyVersion --- input, the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input, the Key Identifier of SK.SD.ECKA
*
*@Return  : return the ref linked to the private key SK.SD.ECKA which is referenced by the keyVersion and keyId
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
//jref GetSkValueRefByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId)
//{
//	memref	skObj;
//
//	skObj =  GetSkObjByKeyVerAndKeyId(sdDataObj,  keyVersion,  keyId);
//
//	return  SCP11_SK_SD_ECKA_getSkValueOfCARef(MEMtPN8(skObj));
//}

/*
*@Function: GetCertRefOfECKAByKeyVerAndKeyId
*
*@Descrip : get the value of SK.SD.ECKA
*
*@See     : keyset结构体获取数据,使用原有接口
*
*@param	  : sdDataObj  --- input,  the SecurityDomainData of the performing SD
*@param	  : keyVersion --- input,  the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input, the Key Identifier of SK.SD.ECKA
*@param	  : skValue    --- output, the skValue, V值;
*
*@Return  : return the length of  SK.SD.ECKA which is referenced by the keyVersion and keyId
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
u16  GetSkValueByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId, PX8 skValue)
{
	return keyData_getKeyData(g_bSDId[selected_channel], keyVersion, keyId, PNtMEM(&skValue[0]));
}


/*
*@Function: GetSkAccessByKeyVerAndKeyId
*
*@Descrip : get the Access of SK.SD.ECKA
*
*@See     : keyset结构体获取数据接口
*
*@param	  : sdDataObj  --- input,  the SecurityDomainData of the performing SD
*@param	  : keyVersion --- input,  the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input, the Key Identifier of SK.SD.ECKA
*
*@Return  : return the value of  SK.SD.ECKA Access
*
*@History : V1.0 --- created by jiangqingbin @20210803
*/
u8  GetSkAccessByKeyVerAndKeyId(PN8 sdDataObj, u8 skKvn, u8 skKid)
{
	jref keySetRef;
	u8 keyAccess;
	PN8 keySetObj;
		
	keySetRef =	keySet_getKeySet(sdDataObj, skKvn);

	if (REF_NULL == keySetRef)
	{
		return 0xFF;
	}
	
	keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
	
	keyData_getKeyAccess(keySetObj , skKid,  &keyAccess);

	return keyAccess;
}

/*
*@Function: GetCertRefOfECKAByKeyVerAndKeyId
*
*@Descrip : get the reference of ECKA Certificate
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.2  GET DATA (ECKA Certificate) Command
*
*@param	  : sdDataObj --- input,  the SecurityDomainData of the performing SD
*@param	  : keyVersion --- input, the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input, the Key Identifier of SK.SD.ECKA
*
*@Return  : return the ref linked to the ECKA(chaining) which is referenced by the keyVersion and keyId
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
jref GetECKACertRefByKeyVerAndKeyId(PN8	sdDataObj, u8 keyVersion, u8 keyId)
{
	memref	skObj;
	
	skObj =  GetSkObjByKeyVerAndKeyId(sdDataObj,  keyVersion,  keyId);

	if (skObj == ILLEGAL_ADDRESS)
	{
		return NULL_REF;
	}
	
	return SCP11_SK_SD_ECKA_getECKACertificateRef(MEMtPN8(skObj));
}



/*
*@Function: GetECKACertAddrByKeyVerAndKeyId
*
*@Descrip : get the addr of ECKA Certificate
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.2  GET DATA (ECKA Certificate) Command
*
*@param	  : sdDataObj	---input,current sd obj
*@param	  : keyVersion --- input,the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input,the Key Identifier of SK.SD.ECKA
*
*@Return  : return the ECKA addr which is referenced by the keyVersion and keyId
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/     
memref GetECKACertAddrByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	jref eckaRef;
	
	eckaRef =  GetECKACertRefByKeyVerAndKeyId(sdDataObj,  keyVersion,  keyId);

	if (eckaRef == NULL_REF)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	return  get_array_data(MEMtPF8(locateObjectInMemory(eckaRef)));
}


/*
*@Function: GetECKACertSubValueByKeyVerAndKeyId
*
*@Descrip : get the son tag value of ECKA Certificate  (获取sd的ecka证书中的某个字段的LV)
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4 (ECKA Certificate chaining) 
*
*@param	  : sdDataObj --- input, the SecurityDomainData of the performing SD
*@param	  : keyVersion --- input, the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input, the Key Identifier of SK.SD.ECKA
*@param	  : wantTag  ---input, the wanted tag 
*@param	  : bufLv  --- output, the wanted tag LV
*@param	  : bufSize  --- input, the length of bufLv
*
*@Return  : return TRUE if the indicate tag has find or false;
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
u8 GetECKACertSubValueByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId, u8 wantTag, PX8 bufLv, u8 bufSize)
{
	memref X_A eckaField;
	ASN1Info X_A caTlv;
	u16 I_A offSet;
	u32 X_A certChainLen;
	u8	I_A pKLenOff;
	u32	I_A pKLen;

	eckaField = GetECKACertAddrByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);		//cert  addr begin from BF21

	offSet = 2;
		
	offSet += ASN1_GetBERTlvLen(eckaField+2,  (u32*)&certChainLen);

	certChainLen += offSet;

	ASN1_InitTLVInfo(&caTlv, eckaField, certChainLen);		//begin  from BF21, init to  AsnTlv format

	if(FALSE == ASN1_FindTLV(&caTlv, GPCERT_Tag, FALSE, 1))
	{
		return FALSE;
	}

	ASN1_SetNextDepthToFindTLV(&caTlv); 

	while (TRUE == ASN1_FindTLV(&caTlv, GPCERT_Tag, TRUE, 1));	   //按顺序查找，直到最后一个cert就是sdecka

	if (RESPONSE_NoError != GPCertificate_GetField(MEMtPM8(caTlv.bufferAddr+caTlv.curTagOff), caTlv.curTLVLen, wantTag, bufLv, bufSize))
	{
		return FALSE;
	}

	if (wantTag == GPCERT_PublicKey_Tag)
	{
		if (bufLv[1] == 0xB0)
		{
			pKLenOff = ASN1_GetBERTlvLen(PXtMEM(&bufLv[2]), (u32*)&pKLen);

			bufLv[0] = pKLen;

			ecpMem_Move(bufLv+1,  &bufLv[2+pKLenOff], pKLen);
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;

}



/*
*@Function: GetSkObjRefByKeyVerAndKeyId
*
*@Descrip : get the SkObjRef by kid and kvn
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> <EPX3.0_GP2.3附录F方案设计.docx>5.1章节结构体设计
*
*@param	  : sdDataObj	---input,current sd obj
*@param	  : keyVersion --- input,the Key Version Number of SK.SD.ECKA
*@param	  : keyId      --- input,the Key Identifier of SK.SD.ECKA
*
*@Return  : return the SkObjRef which is referenced by the keyVersion and keyId
*
*@History : V1.0 --- created by jiangqingbin @20210712
*/
memref GetSkObjByKeyVerAndKeyId(PN8 sdDataObj, u8 keyVersion, u8 keyId)
{
	u8 		tmpKeyVersion;
	u8 		tmpKeyId;
	jref  		skObjRef;
	memref	skObj;

	skObjRef =  sdData_getFirstSCP11_SK_SD_ECKARef(sdDataObj);
	
	while(1)
	{
		if (skObjRef == NULL_REF)
		{
			//throw_error(SW_REFERENCED_DATA_NOT_FOUND);
			return ILLEGAL_ADDRESS;
		}
	
		skObj = locateObjectInMemory(skObjRef);
		
		tmpKeyVersion = SCP11_SK_SD_ECKA_getKeyVersion(MEMtPM8(skObj));

		tmpKeyId = SCP11_SK_SD_ECKA_getKeyId(MEMtPM8(skObj));

		if ((tmpKeyVersion == keyVersion) && (tmpKeyId == keyId))
		{
			return skObj;
		}

		skObjRef = SCP11_SK_SD_ECKA_getNextSKRef(MEMtPM8(skObj));
	}

}


/*
*@Function: SCP11SetSessionKey
*
*@Descrip : set scp11 session key
*
*@See     :  set current channel security session key
*
*@param	  : keyBuf	---input,all key buf
*@param	  : allKeyLen	---input,all key length
*@param	  : perKeylen ---input, each key length
*@param	  : lastMacBuf --- input, the mac chaining (receipt value)
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210721
*         : V1.1 --- modify by xuxiaojun@20220525 生成会话密钥之后，需要给对应的长度变量赋值。 SCP11测试脚本： C_sm_PSO_SUCCESS_KID20h_certChain_384_a1_f1_06
*/
void SCP11SetSessionKey(PX8 keyBuf, u8 allKeyLen, u8 perKeylen, PX8 lastMacBuf)
{
	//S-ENC
	g_SCP_sesKeyLen = perKeylen;
	memCopyNoAtomic(MEMtPM8(g_current_SCP_SES_KEY), keyBuf, perKeylen);

//	initBootSCPsessionKey(PNtMEM(g_current_SCP_SES_KEY)); //todo 这个是干嘛的？

	//S-MAC
	g_SCP_macKeyLen = perKeylen;
	memCopyNoAtomic(MEMtPM8(g_current_SCP_MAC_KEY), keyBuf+perKeylen, perKeylen);

	//S-RMAC
	g_SCP_rmacKeyLen = perKeylen;
	memCopyAtomic(MEMtPN8(g_current_SCP_RMAC_KEY), keyBuf+2*perKeylen, perKeylen);

	//S-DEK
	if (allKeyLen == (4*perKeylen))
	{
		g_SCP_dekKeyLen = perKeylen;
		memCopyNoAtomic(MEMtPM8(g_current_SCP_DEK_KEY), keyBuf+3*perKeylen, perKeylen);
	}
	else
	{
		g_SCP_dekKeyLen = 0x00; // todo 这里是否应该赋值为0？
	}

	//参考其他安全会话.....
	memCopyNoAtomic(MEMtPM8((memref)g_current_SCP_LAST_MAC), lastMacBuf, LAST_MAC_LENGTH); //modify by xuxiaojun@20221007: g_current_SCP_LAST_MAC的长度不一定是g_SCP_macKeyLen，MAC的长度只可能是8字节或16字节，测试用例： SCP11c_auth_level_Test_isd_scp03keylen_18
}



/*
*@Function: getSdTagValueBySdobj
*
*@Descrip : get the tag value ,such as 42,45,5f20
**
*@param	  : sdDataObj	---input,current sd obj
*@param	  : wantTag	---input, the wanting tag value
*@param	  : tagDataLv ---output,  contain the tag LV
*@param	  : tagDataSize --- input, the tagDataLv size
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210819
*           V1.1 --- modify by xuxioajun@20220527 获取data数据时，对于存储结构没有弄清楚。  SCP11c_1: C_sm_STORE_DATA_Whitelist_SUCCESS_KID20h_256_a1_60_10
*/
void getSdTagValueBySdobj(PN8 sdDataObj, u16 wantTag, PX8 tagDataLv, u8 tagDataSize)
{
	jref I_A firstTagNodeRef;
	jref I_A tagRef;
	memref X_A tagObj;
	memref X_A tagAddr;
	PN8	tagArrayAField;

	firstTagNodeRef	= sdData_getFirstTagNode(sdDataObj);

	tagRef =  tagNode_getTagArray(firstTagNodeRef, wantTag, 0x00, 0x00, MEM_NULL, false);

	if (tagRef != NULL_REF)
	{
		tagArrayAField = MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(tagRef))));

		tagDataLv[0] = tagNode_getTagArrayLength(tagArrayAField);

		if(U8_A_U8(tagDataLv[0], 1) > tagDataSize)
		{
			throw_error(SW_FILE_FULL);
		}
		else
		{
			memCopyToRam(MEMtPX8(&tagDataLv[1]), tagNode_getTagArrayContent(tagArrayAField), tagDataLv[0]);
		}
	}

	return;
	
}
/*
*@Function: SCP11SetAuthenticatedLevel
*
*@Descrip : As defined in [GPCS] section 10.4.2, 
*ANY_AUTHENTICATED is the Security Level achieved if any OCE not being the owner of the SD authenticates using asymmetric cryptography. 
*AUTHENTICATED is achieved if the owner of the SD or Application authenticates. 
*The SD identifies the owner by the Subject Identifier (TLV with tag '5F20') in the OCE certificate matching the Application Provider Identifier of the SD or Application, 
*which was provided as a parameter (TLV with tag '5F20' within the CRT TLV with tag 'B6') in the INSTALL [for install] command.  
*
*@See     :   <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> session 4.7 and Table B-1;
*
*@param	  : sdDataOb ---input,current sd obj
*@param	  : keyUsage ---input,scp11 session keyUsage
*
*@Return  : set channel Level success or not
*
*@History : V1.0 --- created by jiangqingbin @20210721
*         ： V1.1 --- modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
*/
u16 SCP11SetAuthenticatedLevel(PN8 sdDataObj, u8 keyUsage, u8 scp11ScpPara)
{
	u8 I_A securityLevel;
	u8 X_A sdProviderIdentifierLv[0x11];

	securityLevel = 0;
	ecpMem_Set(sdProviderIdentifierLv, 0x00, sizeof(sdProviderIdentifierLv));

	getSdTagValueBySdobj(sdDataObj, 0x5F20, sdProviderIdentifierLv, sizeof(sdProviderIdentifierLv));
		
	if (ecpMem_Compare(g_current_SCP11SessionPara_oceSubjectIdLv, sdProviderIdentifierLv, 1+sdProviderIdentifierLv[0]) == 0)
	{	
		switch(keyUsage)			//AUTHENTICATED == 0x80
		{
			case 0x34:		//AUTHENTICATED | C_MAC | R_MAC
				securityLevel = SCF_C_MAC_R_MAC;	//0x91
				break;
				
			case 0x3C:		//AUTHENTICATED | C_MAC | C_DECRYPTION | R_MAC | R_ENCRYPTION 
				securityLevel = SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION;	//0xB3
				break;
				
			case 0x1C:		//AUTHENTICATED | C_MAC | C_DECRYPTION 	
				securityLevel = SCF_C_MAC_DECRYPTION;		//0x83
				break;
				
			case 0x74:		//AUTHENTICATED | C_MAC | C_DECRYPTION | R_MAC 
				securityLevel = SCF_C_MAC_R_MAC_DECRYPTION;	//0x93
				break;
				
			default:
				break;
		}
	}
	else							//ANY_AUTHENTICATED == 0x40
	{
		//SCP11C中ANY_AUTHENTICATED级别的通道，其指令的访问规则必须依赖BF20规则
// todo, 这里的理解可能存在问题，暂时注释掉
//		if ( (g_SCP11SessionPara.oceAuthorizationsLv[0] == 0) && (U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_C))
//		{
//			return SW_WRONG_DATA;
//		}
	
		switch(keyUsage)
		{
			case 0x34:		//ANY_AUTHENTICATED | C_MAC | R_MAC
				securityLevel = SCF_C_MAC_R_MAC_ANY;		//0x51
				break;
				
			case 0x3C:		//ANY_AUTHENTICATED | C_MAC | C_DECRYPTION | R_MAC | R_ENCRYPTION 
				securityLevel = SCF_C_MAC_R_MAC_DECRYPTION_ENCRYPTION_ANY;				//0x73
				break;
				
			case 0x1C:		//ANY_AUTHENTICATED | C_MAC | C_DECRYPTION 
				securityLevel = SCF_C_MAC_DECRYPTION_ANY;		//0x43
				break;
				
			case 0x74:		//ANY_AUTHENTICATED | C_MAC | C_DECRYPTION | R_MAC 
				securityLevel = SCF_C_MAC_R_MAC_DECRYPTION_ANY;		//0x53
				break;
				
			default:
				break;
		}
	}
	
	if(U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_C)
	{
		PX_W_U8(g_current_SCP11SessionPara_sessionType, SCP11_C);
	}
	else if (U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_A)
	{
		PX_W_U8(g_current_SCP11SessionPara_sessionType, SCP11_A);

		if((keyUsage == 0x1C) || (keyUsage == 0x7c)) // '1C' (secure messaging with C - MAC and C - ENC, only for SCP11c)
		{                                            // '74' (secure messaging wi th C - MAC, C - ENC, and R - MAC, only for SCP11c)
			return SW_WRONG_DATA;
		}
	}
	else
	{
		return SW_WRONG_DATA;
	}

	if (U8_AND_U8(securityLevel, SCF_R_MAC_FLAG))
	{
		securityLevel = U8_OR_U8(securityLevel, SCF_ZERO_APDU_RMAC_FLAG); 	
		g_SCP_rmacTemp[0] = 0x00;  //setup rmac
	}

	g_SCP_Type[selected_channel] = SCP_11; // 该函数中对于通道的处理，参考verifyExternalAuthenticate函数
	bRamBuffer[SD_REQUESTED_SECURITY_LEVEL] = securityLevel; //modify by xuxiaojun@20220528: 此处不赋值的话，JAVA层的 checkTokenCondition使用到该变量时，存在问题。 AmendH测试脚本-SCP11c:  DELETE_error_SD_related_to_elf_in_current_session_00_06_02
	generateSecureChannelNumber();
	managerChannelLevel(securityLevel);
   	managercompulsorySessionSecurityLevel(securityLevel) ;

	return SW_NO_ERROR;
}

/*
*@Function: SCP11PerformSecurityOperation_P1Valid
*
*@Descrip : 判断PERFORM SECURITY OPERATION指令中的P1是否合法, 该函数只适用于证书的第一条数据
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : p1 ---
*@param	  : certLen --- 证书长度，包含7F21L
*
*@Return  : return true if P1 is valid else return false;
*
*@History : V1.0 --- created by qiuping @20210714
*/
u8 SCP11PerformSecurityOperation_P1Valid(u8 p1, u16 certLen)
{
	u8 isLastCommand;

	isLastCommand = U8_AND_U8(p1 , MORE_COMMAND); // Table 6-8:  Values of Reference Control Parameter P1

	if(isLastCommand == MORE_COMMAND) // If  the certificate doesn’t exceed 255 bytes,  the  command  chaining  information (b8) shall  be  set  to  0  (Last command). Otherwise, bit 8 shall be set to 1 (More command(s)) until the last block of the certificate.
	{
		if(certLen > 0xFF)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else // isLastCommand == LAST_COMMAND
	{
		if(certLen > 0xFF)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}
/*
*@Function: SCP11PerformSecurityOperation_P2Valid
*
*@Descrip : 判断PERFORM SECURITY OPERATION指令中的P2是否合法
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  :sdDataObj
*@param	  : p2       ---
*@param	  : keyUsage --- lv格式  '00 80':  Key agreement (CERT.OCE.ECKA) ; '82':  Digital signature verification (CERT.KA-KLOC.ECDSA)
*@param	  :mode		检查P2还是检查keyUsage
*
*@Return  : return SW_NO_ERROR if P2 is valid else return others;
*
*@History : V1.0 --- created by qiuping @20210714
*/
u16 SCP11PerformSecurityOperation_P2Valid(PN8 sdDataObj, u8 p2, PX8 keyUsage, u8 mode)
{
	u8 isLastCertificate;
	u8 scpOptions;

	// 判断isLastCertificate的合法性:  Bit 8 shall be set to 0 (Last certificate) if the submitted certificate is CERT.OCE.ECKA, i.e. the last certificate.	Otherwise, for intermediate certificates, bit 8 shall be set to 1 (More certificate(s) expected).
	// 证书链模式时，要求starting from the first 	CERT.KA-KLOC.ECDSA(or only) and ending with CERT.OCE.ECKA

	isLastCertificate = U8_AND_U8(p2 , MORE_CERTIFICATE);

	if (mode == NOT_CHECK_KEYUSAGE)
	{
		scpOptions = sdData_getScpOptionByScpType(sdDataObj, SCP_11);
		
		if(scpOptions == 0xFF)
		{
			return SW_REFERENCED_DATA_NOT_FOUND;//SD 不支持SCP11 不能进行安全会话
		}

		if(((scpOptions & CERT_CHAIN_SUPPORTED) != CERT_CHAIN_SUPPORTED) && (isLastCertificate == MORE_CERTIFICATE))//If bit 8 is set to 1 and SCP options (see Table 4-1) indicate that certificate chains are not supported, then a response of '6A86' shall be returned
		{
			return SW_INCORRECT_P1P2;
		}
		
		return SW_NO_ERROR;
	}

	if(keyUsage[0] == 0x01)
	{
		if(keyUsage[1] == GPCERTFICATE_KEY_USAGE_VERIFICATION)
		{
			if(isLastCertificate == LAST_CERTIFICATE) // 表示是最后一个证书
			{
				//return SW_INCORRECT_P1P2;
				return SW_WRONG_DATA;//证书链中的最后的证书的keyUsage必须是0x0080，否则抛错6600/6a80/6982,还不知道是否会影响其他用例(当前用例：用例名称:  C_sm_PSO_ERROR_wrongKeyUsage_certChain_lastCert_82_256_a1_30_43)
			}
			else // MORE_CERTIFICATE
			{
				return SW_NO_ERROR;
			}
		}
		else
		{
			return SW_WRONG_DATA;
		}
	}
	else // if(keyUsage[0] == 0x02)
	{
		if(PX_R_U16(&keyUsage[1]) == GPCERTFICATE_KEY_USAGE_KEYAGREEMENT)
		{
			if(isLastCertificate == MORE_CERTIFICATE) // 不是最后一个证书
			{
				//return SW_INCORRECT_P1P2;
				return SW_WRONG_DATA;//证书链中的非最后的证书的keyUsage必须是0x82，否则抛错6600/6a80/6982,还不知道是否会影响其他用例(当前用例：C_sm_PSO_ERROR_wrongKeyUsage_certChain_firstCert_0080_256_a1_30_42)
			}
			else
			{
				return SW_NO_ERROR;
			}
		}
		else
		{
			return SW_WRONG_DATA;
		}
	}

	return SW_NO_ERROR;
}
/*
*@Function: SCP11PerformSecurityOperation_BakTmpCert
*
*@Descrip : 执行PERFORM SECURITY OPERATION指令时，备份证书的数据
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : void
*
*@Return  : 状态字
*
*@History : V1.0 --- created by xuxiaojun @20210722
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 SCP11PerformSecurityOperation_BakTmpCert()
{
	if(U8_A_U16(APDUBuffer[OFFSET_LC], PX_R_U16(g_current_TmpCert_offset)) > MAX_CERTIFICATE_LEN)
	{
		SCP11PerformSecurityOperation_InitGlobalVar();

		return SW_FILE_FULL; // 缓存能力不够
	}

	ecpMem_Copy(PX8_A_U16(g_current_TmpCert_cert, PX_R_U16(g_current_TmpCert_offset)), &APDUBuffer[OFFSET_CDATA], APDUBuffer[OFFSET_LC]);

	PX_W_U16(g_current_TmpCert_offset, U16_A_U8(PX_R_U16(g_current_TmpCert_offset), APDUBuffer[OFFSET_LC]));

	PX_W_U8(g_current_PerformSecurityOperation_curCertCount, U8_A_U8(PX_R_U8(g_current_PerformSecurityOperation_curCertCount), 1));

	return SW_NO_ERROR;
}
/*
*@Function: SCP11PerformSecurityOperation
*
*@Descrip : 执行PERFORM SECURITY OPERATION指令
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : APDUDataFieldIsReceived --- 表示数据是否已经收全
*
*@Return  : 状态字
*
*@History : V1.0 --- created by xuxiaojun @20210720
*@History : V1.1 --- modify by xuxiaojun@20220525 收到证书的第一条数据时需要将G_TmpCert.offset清0
*         : V1.2 --- modify by jiangqingbin@20220525 增加GPCertificate_ValidOfPK函数，用于判断证书的PK格式是否合法
*         : V1.3 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 SCP11PerformSecurityOperation(u8 APDUDataFieldIsReceived)
{
	u16		X_A		result;
	u8      I_A     keyVersion;
	u8      I_A     keyId;
	jref    I_A     pkCARef;
	u32   X_A     certLen;
	u8      I_A     lenOfLength;
	u8      I_A     isLastCommand;
	u8      I_A     isLastCert;
	u8      I_A     keyUsage[3];
	PN8		pkCAObj;
	PX8     tmpCert;
	u16     tmpCertLen;
	PN8		sdDataObj;
	u8		scpOptions;

	//会不会通过RAM方式下来？
//	if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL])&&(G_RAMSessionSdId == 0xFF) )//RAM APDU不需要进行安全会话, 却包含了Secure Channel命令，RAM指令报错
//	//	if( (ROCESS_LEVEL_OTA == bRamBuffer[PROCESS_LEVEL]) && (SCP81_DISPATCHER_TO_ITSELF == bRamBuffer[SCP81_DISPATCHER]))
//	{
//		return SW_INS_NOT_SUPPORTED;
//	}

	//此处的收数据操作已移到check_Op_Command1的checkIsAuthenticated中， 因为2A指令有可能是在安全通道中下来的
//	if((APDU_DATAFIELD_IS_NOT_RECEIVED == APDUDataFieldIsReceived) && (bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA ))
//	{
//		// receive the DATA
//		apdu_ApduSetIncomingAndReceive();
//	}

	keyVersion = U8_AND_U8(APDUBuffer[OFFSET_P1] , MAX_KEY_VERSION);

	if(false == IsSCP11Key(keyVersion))
	{
		return SW_WRONG_DATA; // Incorrect values in command data
	}

	isLastCommand = U8_AND_U8(APDUBuffer[OFFSET_P1] , 0x80);

	keyId = U8_AND_U8(APDUBuffer[OFFSET_P2] , MAX_KEY_ID);

	if((keyId != 0x10) && ((keyId < 0x20) || (keyId > 0x2f))) // KID '10' for PK.CA-KLOC.ECDSA
	{
		return SW_WRONG_DATA; // Incorrect values in command data
	}

	isLastCert = U8_AND_U8(APDUBuffer[OFFSET_P2] , 0x80);

	sdDataObj = MEMtPN8(getCurrentSdObj(IS_NEED_CHECK_SD_LOCKED));

	pkCARef = SCP11GetPKCARefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

	if( pkCARef == NULL_REF )
	{
		return SW_REFERENCED_DATA_NOT_FOUND; // Referenced PK.CA-KLOC.ECDSA not found
	}

	pkCAObj = MEMtPN8(locateObjectInMemory(pkCARef));

	if ((PX_R_U8(g_current_PerformSecurityOperation_state) == PERFORMSECURITYOPERATION_STATE_START) ||(PX_R_U8(g_current_PerformSecurityOperation_state) == PERFORMSECURITYOPERATION_STATE_END))// 收到第一条PERFORM SECURITY  OPERATION指令 或重新收到2A指令
	{
		PX_W_U8(g_current_PerformSecurityOperation_keyVersion, keyVersion);

		PX_W_U8(g_current_PerformSecurityOperation_keyId, keyId);
	
		PX_W_U8(g_current_PerformSecurityOperation_state, PERFORMSECURITYOPERATION_STATE_CONTINUE);
	}
	else	 // 后续所有指令的key version和key id必须和第一条的一致
	{
		if((keyVersion != PX_R_U8(g_current_PerformSecurityOperation_keyVersion)) || (keyId != PX_R_U8(g_current_PerformSecurityOperation_keyId)))
		{
			return SW_WRONG_DATA; // Incorrect values in command data
		}
	}

	result = SCP11PerformSecurityOperation_P2Valid(sdDataObj, APDUBuffer[OFFSET_P2], NULL, NOT_CHECK_KEYUSAGE);

	if(SW_NO_ERROR != result)
	{
		SCP11PerformSecurityOperation_InitGlobalVar();

		return result; // Incorrect values in command data
	}
	if(PX_R_U8(g_current_PerformSecurityOperation_curCertCount) == 0x00) // 当前收到的是证书的第一条
	{
		PX_W_U16(g_current_TmpCert_offset, 0x00);  //SCP11测试脚本:  C_sm_PSO_ERROR_wrongCertificatePK_missingKeyParamRef_certChain_last_512_a1_32_6d

		if((APDUBuffer[OFFSET_CDATA] != 0x7F) || (APDUBuffer[OFFSET_CDATA + 1] != 0x21))
		{
			return SW_WRONG_DATA; // Incorrect values in command data
		}

		lenOfLength = ASN1_GetBERTlvLen(PXtMEM(&APDUBuffer[OFFSET_CDATA+2]), (u32*)&certLen);

		if(false == SCP11PerformSecurityOperation_P1Valid(APDUBuffer[OFFSET_P1], certLen + 2 + lenOfLength))
		{
			return SW_WRONG_DATA; // Incorrect values in command data
		}

		if(isLastCommand == LAST_COMMAND)//唯一一个APDU就一个证书
		{
			SCP11PerformSecurityOperation_BakTmpCert(); // modify by xuxiaojun@20220923: RAM优化，由于G_TmpCert在双向认证之后，就不会被复写，而g_SCP11SessionPara.oceSubjectIdLv又是从证书中取的，所以直接指向证书中的地址即可

			tmpCert = g_current_TmpCert_cert;

			tmpCertLen = PX_R_U16(g_current_TmpCert_offset);

			PX_W_U8(g_current_PerformSecurityOperation_certNum, U8_A_U8(PX_R_U8(g_current_PerformSecurityOperation_certNum), 1));
		}
		else
		{		
			return SCP11PerformSecurityOperation_BakTmpCert();
		}
	}
	else
	{
		result = SCP11PerformSecurityOperation_BakTmpCert();

		if ((isLastCommand == LAST_COMMAND) && (result == SW_NO_ERROR))
		{
			tmpCert = g_current_TmpCert_cert;

			tmpCertLen = PX_R_U16(g_current_TmpCert_offset);

			PX_W_U8(g_current_PerformSecurityOperation_certNum, U8_A_U8(PX_R_U8(g_current_PerformSecurityOperation_certNum), 1));
		}
		else
		{
			if (result != SW_NO_ERROR)
			{
				SCP11PerformSecurityOperation_InitGlobalVar();
			}
			else
			{
				if (APDUBuffer[OFFSET_LC] != 0xFF)
				{
					SCP11PerformSecurityOperation_InitGlobalVar();

					return SW_WRONG_DATA;
				}
			}
					
			return result;
		}
	}

	PX_W_U8(g_current_PerformSecurityOperation_curCertCount, 0x00);

	//解析证书数据，得到证书的keyUsage
	if(RESPONSE_NoError != GPCertificate_GetField(tmpCert, tmpCertLen, GPCERT_KeyUsage_Tag, keyUsage, sizeof(keyUsage)))
	{
		SCP11PerformSecurityOperation_InitGlobalVar();

		return SW_WRONG_DATA; // Incorrect values in command data
	}

	result = SCP11PerformSecurityOperation_P2Valid(sdDataObj, APDUBuffer[OFFSET_P2], keyUsage, CHECK_KEYUSAGE);

	if(SW_NO_ERROR != result)
	{
		SCP11PerformSecurityOperation_InitGlobalVar();

		return result; // Incorrect values in command data
	}

	//检查当前证书的公钥格式是否符合要求
	result = GPCertificate_ValidOfPK(tmpCert, tmpCertLen);

	if(SW_NO_ERROR != result)
	{
		SCP11PerformSecurityOperation_InitGlobalVar();

		return result;
	}

	//判断证书的有效性
	if(PX_R_U8(g_current_PerformSecurityOperation_certNum) == 0x01)
	{
		lenOfLength = GetSkValueByKeyVerAndKeyId(sdDataObj, keyVersion, keyId, MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV));

		PX_W_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), lenOfLength - 1);

		PX_W_U8(g_current_SCP11SessionPara_ocePkRef, keySet_getEccKeyParaRefValue(MEMtPN8(locateObjectInMemory(keySet_getKeySet(sdDataObj, keyVersion)))));
	}
	
	result = GPCertificate_Valid(pkCAObj, tmpCert, tmpCertLen, MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV));

	if(SW_NO_ERROR != result)
	{
		SCP11PerformSecurityOperation_InitGlobalVar();

		return result;
	}

	if(isLastCert == 0x00) // 收到最后一个证书
	{
		scpOptions = sdData_getScpOptionByScpType(sdDataObj, SCP_11);

		// 从证书中获取5F20并存储,
		if(GPCertificate_GetField(tmpCert, tmpCertLen, GPCERT_SubjectId_Tag, g_current_SCP11SessionPara_oceSubjectIdLv, 0x11) != RESPONSE_NoError)	//获取证书 5F20 字段，LV
		{
			SCP11PerformSecurityOperation_InitGlobalVar();
			return SW_WRONG_DATA;
		}

		// 从证书中获取BF20并存储
		result = GPCertificate_GetField(tmpCert, tmpCertLen, GPCERT_Authorizations_Tag, g_current_SCP11SessionPara_oceAuthorizationsLv, Authorizations_under_SCP11c_MAX_LEN);
		//If a Security Domain does not support the 'BF20' authorization mechanism (as indicated by its SCP11 “i”
		//option) and tag 'BF20' is present, then the command shall fail with an error of '6A81'
		if(((scpOptions & BF20_SUPPORTED) != BF20_SUPPORTED) && (result == RESPONSE_NoError))
		{
			SCP11PerformSecurityOperation_InitGlobalVar();

			return SW_FUNC_NOT_SUPPORTED;
		}

		PX_W_U8(g_current_PerformSecurityOperation_state, PERFORMSECURITYOPERATION_STATE_END);
	}

	return SW_NO_ERROR;
}
/*
*@Function: SCP11MutualAuthenticate_A6Valid
*
*@Descrip : 判断MUTUAL AUTHENTICATE指令中的A6是否合法，如果合法的话就提取出相应的 ScpPara, key usage, key type, key length, HostID
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  : a6Buf    --- input, A6LV格式
*@param	  : ScpPara  --- output,
*@param	  : keyUsage --- output,
*@param	  : keyType  --- output,
*@param	  : keyLen   --- output,
*@param   : hostIDLv --- output,
*
*@Return  : 返回A6LV的总字节数;
*
*@History : V1.0 --- created by jiangqingbin @20210716
*/
u8 SCP11MutualAuthenticate_A6Valid(PX8 a6Buf, PX8 ScpPara, PX8 keyUsage, PX8 keyType, PX8 keyLen, PX8 hostIDLv)
{
	u8 I_A offSet;
	u32 X_A a6Len;
	u8 I_A a6LenOfLength;
	
	a6LenOfLength = ASN1_GetBERTlvLen(PXtMEM(&a6Buf[1]), (u32*)&a6Len);

	offSet = U8_A_U8(a6LenOfLength, 1); // point to '90' in A6: SCP identifier and parameters

	if((a6Buf[offSet] != 0x90) || (a6Buf[offSet+1] != 0x02)||(a6Buf[offSet+2] != SCP_11))
	{
		return FALSE;
	}

	*ScpPara = a6Buf[offSet+3];

	offSet = U8_A_U8(offSet, 4); // point to '95' in A6: Key Usage Qualifier

	if ((a6Buf[offSet] != 0x95) || (a6Buf[offSet+1] != 0x01))
	{
		return FALSE;
	}
	
	//The usage of values '1C' and '74' is only allowed for SCP11c		
	if(U8_AND_U8(*ScpPara, SCP11_PARAMETER_VARIANT_MASK) == 	SCP11_PARAMETER_VARIANT_C)	 //11c
	{
		if ((a6Buf[offSet+2] != 0x34) && (a6Buf[offSet+2] != 0x3C) && (a6Buf[offSet+2] != 0x1C) && (a6Buf[offSet+2] != 0x74))
		{
			return FALSE;
		}
	}
	else		//11a
	{
		if((a6Buf[offSet+2] != 0x3C) && (a6Buf[offSet+2] != 0x34))
		{
			return FALSE;
		}
	}

	*keyUsage = a6Buf[offSet+2];

	offSet = U8_A_U8(offSet, 3); // point to '80' in A6: Key Type

	if ((a6Buf[offSet] != 0x80) || (a6Buf[offSet+1] != 0x01)||(a6Buf[offSet+2] != 0x88))
	{
		return FALSE;
	}

	*keyType =  0x88;

	offSet = U8_A_U8(offSet, 3); // point to '81' in A6: Key Length

	if ((a6Buf[offSet] != 0x81) || (a6Buf[offSet+1] != 0x01))
	{
		return FALSE;
	}

	//If mandated by the security policy, key length shall be checked according to the recommendations defined in section 5.2.
	if ((a6Buf[offSet+2] != BYTE_LENGTH_AES_128)  &&  (a6Buf[offSet+2] != BYTE_LENGTH_AES_192)  &&  (a6Buf[offSet+2] != BYTE_LENGTH_AES_256) )
	{
		return FALSE;
	}

	*keyLen = a6Buf[offSet+2];

	offSet = U8_A_U8(offSet, 3); // point to '84' in A6: HostID

	//If bit 3 of the SCP parameters is set (“Include Host and Card ID in key derivation process”) and tag '84' (Host ID) is not present within tag 'A6', then an error shall be returned.
	if (U8_AND_U8(*ScpPara, SCP11_PARAMETER_HOSTID_MASK) == SCP11_PARAMETER_HOSTID_INCLUDE)
	{
		if (a6Buf[offSet] != 0x84)
		{
			return FALSE;
		}

		U8_INC(offSet);	//offSet point to 84 tag L

		offSet += ASN1_GetBERTlvLen(PXtMEM(&a6Buf[offSet]), (u32*)hostIDLv);	//offSet point  to 84 value

		if(hostIDLv[0] > HOST_ID_MAX_LEN)
		{
			return FALSE; // 超出卡片能力
		}
		else
		{
			ecpMem_Copy(hostIDLv+1, &a6Buf[offSet], hostIDLv[0]);
		}

		offSet += hostIDLv[0];
	}
	else // Similarly, if bit 3 is not set and tag '84' (Host ID) is present within tag 'A6', then an error shall be returned.
	{
		if (a6Buf[offSet] == 0x84)
		{
			return FALSE;
		}
	}

	if(offSet != (a6Len + a6LenOfLength + 1)) //有多余的TAG
	{
		return FALSE;
	}

	return a6Len + a6LenOfLength + 1; // 返回A6 TLV的总字节数
}
/*
*@Function: SCP11MutualAuthenticate_5F49Valid
*
*@Descrip : 判断MUTUAL AUTHENTICATE指令中的5F49是否合法
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  : dataBuf, input, 5F49的TLV格式
*@param	  : ePKBufOffSet, output, 临时公钥5F49的value相对dataBuf的偏移
*
*@Return  : return the 5F49 tlv all length else return false;
*
*@History : V1.0 --- created by jiangqingbin @20210716
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u8 SCP11MutualAuthenticate_5F49Valid(PX8 dataBuf, PX8 ePKBufOffSet)
{
	u8 I_A offSet;
	u32 X_A epklength;
	
	//GP附录F没有详细给定临时公钥的结构，参考以前M2M-EUICC的GP证书格式的临时公钥做法, ePK.SR.ECKA 的格式为7F494104XXXXXXXXXXXXX  <WQ_Offline_Function_Test / TC_ES8_EstablishISDPKeyset_SMS_Invalid_EPubKey_SecondStoreData_Para_Error_01>
	offSet = 2;
	
	offSet += ASN1_GetBERTlvLen(PXtMEM(&dataBuf[offSet]),  (u32*)&epklength);

	if ((epklength != (2*ALG_EC_FP_KEY_256 + 1)) && (epklength != (2*ALG_EC_FP_KEY_384 + 1))
#if WITH_ECC_KEY_512
			&& (epklength != (2*ALG_EC_FP_KEY_512 + 1))
#endif
#if WITH_ECC_KEY_521
			&& (epklength != (2*ALG_EC_FP_KEY_521 + 1))
#endif
			)
	{
		return FALSE;	//临时公钥长度不对
	}

	//When  contained  in  a  command  or  a  response,  static  or  ephemeral  public  keys shall  be formatted  using uncompressed  encoding  as  specified  in [TR 03111]  section 3.1.1,  with  most  significant  byte  coming  first (hence  the  value  shall  start  with  the  coding  identifier  byte '04')
	if (dataBuf[offSet] != 0x04) // 5F49数据域的第一个字节
	{
		return FALSE;
	}

	ePKBufOffSet[0] = (offSet+1);

	if (checkPointOnEccCurve(dataBuf+offSet+1, epklength-1, PX_R_U8(g_current_SCP11SessionPara_ocePkRef)) == FALSE)
	{
		return FALSE;
	}
	
	return U8_A_U8(offSet, epklength); // 返回5F49LV的总字节数
}



/*
*@Function: patchMutualAuthenticateResponseData
*
*@Descrip : MUTUAL AUTHENTICATE指令响应数据拼装
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  :sdDataObj 	 --- input, the SecurityDomainData of the performing SD
*@param	  :PkLV		 ---input, ePK.SD.ECKA LV  or PK.SD.ECKA LV
*@param	  :receipt		 ---input, receipt value
*
*@Return  : response data length; 响应数据长度
*
*@History : V1.0 --- created by jiangqingbin @20210820
*/
u8 patchMutualAuthenticateResponseData(PX8 resBuf,  PX8 pkLV,  PX8 receipt)
{
	u8 I_A offSet;
	
	offSet = 0;

	resBuf[offSet++] = 0x5F;
	resBuf[offSet++] = 0x49;

	if (pkLV[0] < 0x80)
	{
		ecpMem_Copy(resBuf+offSet, pkLV, 1+pkLV[0]);
	}
	else
	{
		resBuf[offSet++] = 0x81;

		ecpMem_Copy(resBuf+offSet, pkLV, 1+pkLV[0]);
	}

	offSet += 1+pkLV[0];
	
	resBuf[offSet++]  = 0x86;

	resBuf[offSet++]  = 0x10;

	ecpMem_Copy(resBuf+offSet, receipt, 0x10);

	offSet += 0x10;

	return offSet;
}


/*
*@Function: SCP11GenerateShS
*
*@Descrip : MUTUAL AUTHENTICATE指令中计算shs
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  :sdDataObj	 --- input, the SecurityDomainData of the performing SD
*@param	  :skKvn		---input, ePK.SD.ECKA LV  or PK.SD.ECKA LV
*@param	  :skKid		 ---input, receipt value
*@param	  :scp11ScpPara---input, 
*@param	  :shsBufLv	---output, 
*@param	  :oceEPkBuf	---input,  epk value of oce 
*
*@Return  :return SW_NO_ERROR or other error code
*
*@History : V1.0 --- created by jiangqingbin @20210820
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 SCP11GenerateShS(PN8 sdDataObj, u8 skKvn, u8 skKid, u8 scp11ScpPara, PX8 shsBufLv, PX8 oceEPkBuf)
{
	u8 scpOptions;
	u8 skKeyAccess;
	u8 X_A scp11SkValueLV[MAX_ECC_KEY_BYTELEN + 1];
	//获取的数据是LV而不是V，L大于80小于FF时是81 L的格式，所以长度需要加1, 统一使用宏定义更方便修改
	u8 X_A scp11PkValueLV[SCP11_PK_MAX_LV_LEN]; // 这里获取的是证书中7F49的内容，格式为7F49L + B0L04V + F001XX
	u8 ocePkRef;
	PX8 ocePKValueLV;

 #if GP__SCP__SCP11__A
	u8 X_A scp11SdEskValue[0x42];	//eSK.SD.ECKA临时私钥
	u8 X_A scp11SdEpkValue[0x85];	//ePK.SD.ECKA 临时公钥, 	// 产生的临时公钥第一个字节是04
#endif	

	ecpMem_Set(scp11SkValueLV, 0x00, sizeof(scp11SkValueLV));

#if GP__SCP__SCP11__A
	ecpMem_Set(scp11SdEskValue, 0x00, sizeof(scp11SdEskValue));
	ecpMem_Set(scp11SdEpkValue, 0x00, sizeof(scp11SdEpkValue));
#endif

	ocePkRef = PX_R_U8(g_current_SCP11SessionPara_ocePkRef);

	ocePKValueLV = MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV);

	scpOptions = sdData_getScpOptionByScpType(sdDataObj, SCP_11);

	 //get sd.sk value
	scp11SkValueLV[0] = GetSkValueByKeyVerAndKeyId(sdDataObj, skKvn, skKid, scp11SkValueLV+1);

	if (scp11SkValueLV[0]  == 0)
	{
		return SW_REFERENCED_DATA_NOT_FOUND;
	}

	if(U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_C) // SCP11c
	{
		if(skKid != KEY_ID_FOR_SK_SD_ECKA_FOR_SCP11c)	//scp11c kid
		{
			return SW_WRONG_DATA;
		}

		//SD not supported SCP11 or sd options not supported SCP11c
		if ((scpOptions == 0xFF) || ((scpOptions & SCP11C_SUPPORTED) != SCP11C_SUPPORTED))
		{
			return SW_WRONG_DATA;
		}

		// The SD shall remove all associated objects that have the "pending  completion" mark.
		if (isInProgress())
		{
			abortTransaction();
		}

		//The Key Access Conditions as defined in [GPCS] shall be supported by the SD for SK.SD.ECKA and Key-DEK
		skKeyAccess = GetSkAccessByKeyVerAndKeyId(sdDataObj, skKvn, skKid);

		if ((skKeyAccess != 0x00) && (skKeyAccess != 0x01))
		{
			return SW_WRONG_DATA;
		}
			
		// generate the shared secret ShSes
		if (calcSHS((u8*)oceEPkBuf, 2*scp11SkValueLV[0],  ocePkRef, (u8*)&scp11SkValueLV[1], scp11SkValueLV[0],  (u8*)&shsBufLv[1]) == FALSE)
		{
			return SW_WRONG_DATA;
		}

		// generate the shared secret ShSss	
		if (calcSHS(PX8_A_U8(ocePKValueLV, 1), PX_R_U8(ocePKValueLV), ocePkRef, (u8*)&scp11SkValueLV[1], scp11SkValueLV[0],  (u8*)&shsBufLv[1+scp11SkValueLV[0]]) == FALSE)
		{
			return SW_WRONG_DATA;
		}

		if (FALSE == GetECKACertSubValueByKeyVerAndKeyId(sdDataObj, skKvn, skKid, GPCERT_PublicKey_Tag, scp11PkValueLV, sizeof(scp11PkValueLV)))
		{
			return SW_REFERENCED_DATA_NOT_FOUND;
		}

		ecpMem_Copy(ocePKValueLV, scp11PkValueLV, 1+scp11PkValueLV[0]);		 //计算完成shs之后，ocePKValueLV的全局数据已无作用，此时用来复用保存sdpk，后续拼装响应数据时使用到
		
		shsBufLv[0] = 2*scp11SkValueLV[0];

		return SW_NO_ERROR;
	}
 #if GP__SCP__SCP11__A
	else if(U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_A) 	// SCP11a
	{
		
		if(skKid != KEY_ID_FOR_SK_SD_ECKA_FOR_SCP11a)	//scp11a kid
		{
			return SW_WRONG_DATA;
		}

		//SD not supported SCP11 or sd options not supported SCP11a
		if ((scpOptions == 0xFF) || ((scpOptions & SCP11A_SUPPORTED) != SCP11A_SUPPORTED))
		{
			return SW_WRONG_DATA;
		}

		//If the SD also supports SCP11c, it shall check that the CERT.OCE.ECKA provided by the previousPERFORM SECURITY OPERATION command did not contain tag 'BF20'. 
		if  ((scpOptions & SCP11C_SUPPORTED) == SCP11C_SUPPORTED)
		{
			if (PX_R_U8(g_current_SCP11SessionPara_oceAuthorizationsLv) != 0)
			{
				return SW_WRONG_DATA;
			}
		}

		//The SD shall generate an ephemeral key pair eSK.SD.ECKA and ePK.SD.ECKA
		GenECCKeyPair(scp11SdEpkValue, scp11SdEskValue, ocePkRef);

		//The Key Access Conditions as defined in [GPCS] shall be supported by the SD for SK.SD.ECKA and Key-DEK
		skKeyAccess = GetSkAccessByKeyVerAndKeyId(sdDataObj, skKvn, skKid);

		if ((skKeyAccess != 0x00) && (skKeyAccess != 0x01))
		{
			return SW_WRONG_DATA;
		}
			

		//The SD shall concatenate ShSee and ShSss to form the shared secret ShS which constitutes the input for the Key Derivation process. 
		//The SD shall use ePK.OCE.ECKA and eSK.SD.ECKA to generate the shared secret ShSee according to section 3.1.1. 
		if (calcSHS((u8*)oceEPkBuf, 2*scp11SkValueLV[0],  ocePkRef, (u8*)&scp11SdEskValue[1], scp11SdEskValue[0],  (u8*)&shsBufLv[1]) == FALSE)
		{
			return SW_WRONG_DATA;
		}

		//The SD shall use PK.OCE.ECKA and SK.SD.ECKA to generate the shared secret ShSss according tosection 3.1.1. 
		if (calcSHS(PX8_A_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), 1), PX_R_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV)),  ocePkRef, (u8*)&scp11SkValueLV[1], scp11SkValueLV[0],  (u8*)&shsBufLv[1+scp11SkValueLV[0]]) == FALSE)
		//if (SCP11GenerateShSssOrShSes(g_SCP11SessionPara.ocePKValueLV, scp11SkValueLV, &tempBuf[0]) == 0)
		{
			return SW_WRONG_DATA;
		}

		ecpMem_Copy(PX8_A_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), 1), scp11SdEpkValue+1, 2*scp11SkValueLV[0]);		 //计算完成shs之后，ocePKValueLV的全局数据已无作用，此时用来复用保存sdepk，后续拼装响应数据时使用到

		shsBufLv[0] = 2*scp11SkValueLV[0];

		return SW_NO_ERROR;
	}
#endif
	else
	{
		return SW_WRONG_DATA;
	}
}


/*
*@Function: pendingDataforReceiptCal
*
*@Descrip : MUTUAL AUTHENTICATE指令中拼装计算收条的数据，即把pkLv拼装到dataBuf之后
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  :dataBuf			 --- input/output, the SecurityDomainData of the performing SD
*@param	  :pkLv 			---input, sd epkLv or pkLv
*
*@Return  :return the length of data to be cal Receipt
*
*@History : V1.0 --- created by jiangqingbin @20210820
*/
u16 pendingDataforReceiptCal(PX8 dataBuf,  PX8 pkLv)	//g_SCP11SessionPara.ocePKValueLV
{
	u8 off;
	
	off = 0x00;

	dataBuf[off++] = 0x5F;

	dataBuf[off++] = 0x49;

	if (pkLv[0] < 0x80)
	{
		ecpMem_Copy(dataBuf+off, pkLv, 1+pkLv[0]);
	}
	else
	{
		dataBuf[off++] = 0x81;

		ecpMem_Copy(dataBuf+off, pkLv, 1+pkLv[0]);
	}

	off += 1+pkLv[0];

	return off;
}


/*
*@Function: SCP11PendingShareInfo
*
*@Descrip : MUTUAL AUTHENTICATE指令中拼装shareInfo
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  :sdDataObj 		--- input, the SecurityDomainData of the performing SD
*@param	  :skKvn			---input,
*@param	  :skKid 			---input, 
*@param	  :sharedInfo	 	---output,
*@param	  :scp11ScpPara 	---input,
*@param	  :scp11KeyUsage 	---input,
*@param	  :scp11KeyType 	---input,
*@param	  :scp11KeyLength 	---input,
*@param	  :scp11HostIdLV	---input,
*
*@Return  :return the length of data to be cal Receipt
*
*@History : V1.0 --- created by jiangqingbin @20210820
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u8 SCP11PendingShareInfo(PN8 sdDataObj,  u8 skKvn, u8 skKid, PX8 sharedInfo, u8 scp11ScpPara, u8 scp11KeyUsage, u8 scp11KeyType, u8 scp11KeyLength, PX8 scp11HostIdLV)
{
	u8 I_A sharedInfoLen;
	u8 X_A scp11CardGroupIDLV[GROUP_ID_MAX_LEN + 1];

#if GP__SCP__SCP11__A
	u8 X_A scp11SINValueLV[SIN_MAX_LEN + 1];	//sd tag 42 value,The  security  domain  Provider  Identification  Number
	u8 X_A scp11SDINValueLV[SDIN_MAX_LEN + 1];	//sd tag 45 value,The  security  domain  Identification  Number
#endif

#if GP__SCP__SCP11__A
	ecpMem_Set(scp11SINValueLV, 0x00, sizeof(scp11SINValueLV));
	ecpMem_Set(scp11SDINValueLV, 0x00, sizeof(scp11SDINValueLV));
#endif

	sharedInfoLen = 0;
	
	sharedInfo[sharedInfoLen++] = scp11KeyUsage;
	sharedInfo[sharedInfoLen++] = scp11KeyType;
	sharedInfo[sharedInfoLen++] = scp11KeyLength;

	if(U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_HOSTID_MASK) == SCP11_PARAMETER_HOSTID_INCLUDE) //bit 3 of the SCP parameters indicate "Include Host and Card ID" in shareInfo
	{		
		ecpMem_Copy(sharedInfo+sharedInfoLen, scp11HostIdLV, scp11HostIdLV[0]+1);

		sharedInfoLen += (1 + scp11HostIdLV[0]);
		
 #if GP__SCP__SCP11__A		
		if(U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_A) 		//11a
		{
			getSdTagValueBySdobj(sdDataObj, OP_ISSUER_BIN, scp11SINValueLV, sizeof(scp11SINValueLV));

			getSdTagValueBySdobj(sdDataObj, OP_CARD_ISSUER_DATA, scp11SDINValueLV, sizeof(scp11SDINValueLV));

			//SIN OR SDIN not found
			if ((scp11SINValueLV[0] == 0) || (scp11SDINValueLV[0] == 0))
			{
				return FALSE;
			}

			ecpMem_Copy(sharedInfo+sharedInfoLen, scp11SINValueLV, scp11SINValueLV[0]+1);

			sharedInfoLen += (1 + scp11SINValueLV[0]);

			ecpMem_Copy(sharedInfo+sharedInfoLen, scp11SDINValueLV, scp11SDINValueLV[0]+1);

			sharedInfoLen += (1 + scp11SDINValueLV[0]);
		}
		else	//11c
#endif
		{
			//get sd ecka subject identifier
			GetECKACertSubValueByKeyVerAndKeyId(sdDataObj, skKvn, skKid, GPCERT_SubjectId_Tag, scp11CardGroupIDLV, sizeof(scp11CardGroupIDLV)); // Card Group ID-LV is the length and the value field of tag '5F20' (subject identifier) in CERT.SD.ECKA.

			if (scp11CardGroupIDLV[0] == 0)
			{
				return FALSE;
			}

			ecpMem_Copy(sharedInfo+sharedInfoLen, scp11CardGroupIDLV, scp11CardGroupIDLV[0]+1);

			sharedInfoLen += (1 + scp11CardGroupIDLV[0]);
		}	
	}

	return sharedInfoLen; //the max value is 0x33 + 3
}


/*
*@Function: check5F20AndOcePk
*
*@Descrip : MUTUAL AUTHENTICATE Command need to check whether exit pkoce and 5f20 
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  :sdDataObj 		--- input, the SecurityDomainData of the performing SD
*@param	  :scpOptions			---input,the performing SD scp11 option,
*@param	  :scp11ScpPara 			---input, the SCP parameters in MUTUAL AUTHENTICATE Command
*
*@Return  :return the length of data to be cal Receipt
*
*@History : V1.0 --- created by jiangqingbin @20210820
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 check5F20AndOcePk(PN8 sdDataObj, u8 scpOptions,u8 scp11ScpPara)
{
	u16 result;

	result = SW_NO_ERROR;
	
#if GP__SCP__SCP11__A
	if( (U8_AND_U8(scp11ScpPara, SCP11_PARAMETER_VARIANT_MASK) == SCP11_PARAMETER_VARIANT_A))//only 11a need store pkoce and 5f20 persistently
	{
		if(PX_R_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV)) == 0)
		{
			//get Persistently store 5F20 And OcePk
			result = SCP11GetPersistently5F20AndOcePk(sdDataObj);

			if(result != SW_NO_ERROR)
			{
				return result;
			}
		}
		else
		{
			//store PK.OCE.ECKA(OcePk) and SubjectId(5F20) Persistently
			result = SCP11PersistentlyStore5F20AndOcePk(sdDataObj,scpOptions);
		
			if(result != SW_NO_ERROR)
			{
				return result;
			}
		}
	}
	else
#endif
	{
		if ((PX_R_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV)) == 0) ||(PX_R_U8(g_current_SCP11SessionPara_oceSubjectIdLv) == 0) )
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
	}

	return SW_NO_ERROR;
}


/*
*@Function: SCP11MutualAuthenticate
*
*@Descrip : MUTUAL AUTHENTICATE指令中数据合法之后，需要初始化安全会话
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.5  MUTUAL AUTHENTICATE Command
*
*@param	  :sdDataObj --- input, the SecurityDomainData of the performing SD
*
*@Return  : response data length; 响应数据长度
*
*@History : V1.0 --- created by jiangqingbin @20210716
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*         : V1.2 --- modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
*/
u16 SCP11MutualAuthenticate(u8 APDUDataFieldIsReceived)
{
	PX8 sharedInfo;
	PX8 receipt;
	PX8 shsBuf;
	PX8 keyBuf;
	PX8 inputDataForReceipt;
	PN8 sdDataObj;
	u8 I_A skKvn;
	u8 I_A skKid;
	u8 I_A offSet;
	u16 I_A result;
	u8 I_A scp11ScpPara;
	u8 I_A scp11KeyUsage;
	u8 I_A scp11KeyLength;
	u8 I_A scp11KeyType;
	u8 X_A scp11HostIdLV[HOST_ID_MAX_LEN + 1];
	u8 I_A keyDataLen;				//要分散密钥数据的总长度
	u8 I_A p3;
	u16 I_A receiptDataLen;
	u8 I_A shsLen;         //the max value is 2*MAX_ECC_KEY_BYTELEN
	u8 I_A sharedInfoLen ; //the max value is 0x36
	u8 I_A scpOptions;
	u8 I_A oceEpkValueOffSet;
	u8 X_A tmpBuf[2*MAX_ECC_KEY_BYTELEN + 4 + 0x36 + 5*BYTE_LENGTH_AES_256]; // todo: shsBuf + 4 + sharedInfo + 5*scp11KeyLength

	sdDataObj = MEMtPN8(getCurrentSdObj(IS_NEED_CHECK_SD_LOCKED));

	ecpMem_Set(scp11HostIdLV, 0x00, sizeof(scp11HostIdLV));

	if((apdu_IsSetIncoming() == 0)&&(APDU_DATAFIELD_IS_NOT_RECEIVED == APDUDataFieldIsReceived) && (bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA ))// modify by xuxiaojun@20220627:如果 g_SCP11SessionPara.sessionType == SCP11_C时，数据在前面已经收过了
	{
		// receive the DATA
		apdu_ApduSetIncomingAndReceive();
	}

	skKvn = APDUBuffer[APDU_P1_OFFSET]; // P1 references the Key Version Number of the SK.SD.ECKA. It is coded on bits 1 to 7. Bit 8 is RFU and set to zero.
	skKid = APDUBuffer[APDU_P2_OFFSET]; // P2 references the Key Identifier of the SK.SD.ECKA. It is coded on bits 1 to 7.	Bit 8 is RFU and set to zero
	p3 = APDUBuffer[APDU_P3_OFFSET];

	// The MUTUAL AUTHENTICATE command terminates any ongoing secure channel session (whichever secure channel protocol is currently used) on the same logical channel
	closeSecuritychannel(SC_TERMINATED);// modify by liweimin

	//one SCP11c session is already ongoing on another logical channe
	result = checkIsSDAcitveOnOtherLogicChannel();

	if(result != SW_NO_ERROR)
	{
		return result;
	}

	//If no PK.OCE.ECKA was provided earlier to the SD using a PERFORM SECURITY OPERATION command or  if  a  whitelist  was  updated  since PK.OCE.ECKA  was  provided  (see  section 6.8),  the  MUTUAL AUTHENTICATE command shall fail with error condition “Conditions of use not satisfied”
	if(PX_R_U8(MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV)) == 0)
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	//check indicate sd.sk kvn
	if(IsSCP11Key(skKvn) == FALSE)
	{
		return SW_WRONG_DATA;
	}

	if(APDUBuffer[APDU_DATA_OFFSET] != 0xA6) // point to 'A6'
	{
		return SW_WRONG_DATA;
	}

	result = SCP11MutualAuthenticate_A6Valid(&APDUBuffer[APDU_DATA_OFFSET],  &scp11ScpPara, &scp11KeyUsage, &scp11KeyType, &scp11KeyLength, scp11HostIdLV);

	if (result == FALSE)
	{
		return SW_WRONG_DATA;
	}

	offSet = U8_A_U8(APDU_DATA_OFFSET, result); // point to '5F49'

	if((APDUBuffer[offSet] != 0x5F) || (APDUBuffer[offSet+1] != 0x49))
	{
		return SW_WRONG_DATA;
	}
	
	result = SCP11MutualAuthenticate_5F49Valid(&APDUBuffer[offSet], &oceEpkValueOffSet);

	if (result == FALSE)
	{
		return SW_WRONG_DATA;
	}

	oceEpkValueOffSet += offSet;

	offSet = U8_A_U8(offSet, result);
	
	if(offSet != U8_A_U8(APDUBuffer[APDU_P3_OFFSET], APDU_DATA_OFFSET)) // 有多余的TAG
	{
		return SW_WRONG_DATA;
	}
	
	scpOptions = sdData_getScpOptionByScpType(sdDataObj, SCP_11);

	result = check5F20AndOcePk(sdDataObj, scpOptions, scp11ScpPara);

	if(result != SW_NO_ERROR)
	{
		return result;
	}

	//cacl shs
	shsBuf = tmpBuf;
	
	result = SCP11GenerateShS(sdDataObj, skKvn, skKid, scp11ScpPara, shsBuf, &APDUBuffer[oceEpkValueOffSet]);
	
	if (result != SW_NO_ERROR)
	{
		return result;
	}

	shsLen = shsBuf[0];
	
	ecpMem_Move(shsBuf, shsBuf+1, shsBuf[0]);

	//pending shareInfo	
	sharedInfo = shsBuf+shsLen+4;

	sharedInfoLen = SCP11PendingShareInfo(sdDataObj,  skKvn, skKid, sharedInfo, scp11ScpPara, scp11KeyUsage, scp11KeyType, scp11KeyLength, scp11HostIdLV);
	
	if (sharedInfoLen == FALSE)
	{
		return SW_REFERENCED_DATA_NOT_FOUND;;
	}

	//S-DEK (if no Key-DEK is present)
	if(0x00 == GetDekByKeyVerAndKeyId(sdDataObj, skKvn, skKid+1))
	{
		keyDataLen = 5*scp11KeyLength; // todo scp11KeyLength的值最大为32字节
	}
	else
	{
		keyDataLen = 4*scp11KeyLength;
	}
   	//modify by xuxiaojun@20220609 参考 verifyExternalAuthenticate中对于SCP03的处理, 测试用例： AmendH测试脚本-SCP11c：  Annex_B_scenario_01_AM_with_autoRestore_bb_01_02 将TypesSpecification.js文件中的curSCP11_Level值分别设置为74
	memFillNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC),0x00,LAST_MAC_LENGTH);
	ecpMem_Set(g_SCP_ICV_Counter,0,SCP03_ICV_COUNTER_LENGTH);

	//KeyDerivationFunction 使用X9.63 Key Derivation Function分散密钥， 参考规范附录E
	keyBuf = sharedInfo+sharedInfoLen;
	KeyDerivationFunction((u8*)&shsBuf[0], shsLen, (u8*)&sharedInfo[0], sharedInfoLen, keyDataLen, (u8*)&tmpBuf[0], (u8*)&keyBuf[0]); //todo 这里不能用APDUBuffer，因为数据后续还要使用

	receiptDataLen = p3;

	receiptDataLen += pendingDataforReceiptCal(APDUBuffer+APDU_DATA_OFFSET+p3, MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV));
	
	receipt = tmpBuf;

	inputDataForReceipt = APDUBuffer+APDU_DATA_OFFSET;

	//cacl receipt
	Cmac(keyBuf, scp11KeyLength, inputDataForReceipt, receiptDataLen, NULL, 0, receipt);

	SCP11SetSessionKey(keyBuf+scp11KeyLength, keyDataLen-scp11KeyLength, scp11KeyLength, receipt);
	
	result = SCP11SetAuthenticatedLevel(sdDataObj, scp11KeyUsage,scp11ScpPara);
		
	if (result != SW_NO_ERROR)
	{
		return result;
	}

	APDUBuffer[OFFSET_LC]  = patchMutualAuthenticateResponseData(APDUBuffer + APDU_DATA_OFFSET,  MEMtPX8(g_current_SCP11SessionPara_ocePKValueLV), receipt);
	
	return SW_NO_ERROR;
}



/*
*@Function: GPCertificate_ValidOfPK
*
*@Descrip : 检查证书的pk是否符合规范，仅支持 7F21 开头的证书结构
*
*@See     :	《rfc5480.pdf》 ,	<GPC_2.3_F_SCP11_v1.3_PublicRelease.pdf>
*
*@param	  : certBuf     	--- 输入参数，证书数据, 7F21LV
*@param	  : totleLen     	--- 输入参数，证书数据总的TLV长度
*
*@Return  : return SW_NO_ERROR if PK is Valid, else return others;
*
*@History : V1.0 --- created by jiangqingbin @20220525
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 GPCertificate_ValidOfPK(PM8 certBuf, u16 totleLen)
{
	ASN1Info X_A caTlv;
	//获取的数据是LV而不是V，L大于80小于FF时是81 L的格式，所以长度需要加1, 统一使用宏定义更方便修改
	u8 X_A scp11PkValueLV[SCP11_PK_MAX_LV_LEN]; // 这里获取的是证书中7F49的内容，格式为7F49L + B0L04V + F001XX

	if(GPCertificate_GetField(certBuf, totleLen, GPCERT_PublicKey_Tag, scp11PkValueLV, sizeof(scp11PkValueLV)) != RESPONSE_NoError)	//获取证书 PublicKey 字段，LV
	{
		return SW_WRONG_DATA;
	}

	ASN1_InitTLVInfo(&caTlv, PXtMEM(&scp11PkValueLV[0x01]), scp11PkValueLV[0]);

	if(FALSE == ASN1_FindTLV(&caTlv, GPCERT_PublicKey_VALUE_Tag, FALSE, 1))	//Public Key-Q ‘B0' 字段是否存在
	{
		return SW_WRONG_DATA;
	}

	//《rfc5480.pdf》 2.2节:	ecc公钥的编码格式，分为压缩和非压缩两种
	//<GPC_2.3_F_SCP11_v1.3_PublicRelease.pdf> 5.1 节:	SCP11所有公钥都必须采用非压缩格式编码
	if (scp11PkValueLV[0x01 + caTlv.curValueOff] != GPCERT_PublicKey_UnCompressed)
	{
		return SW_WRONG_DATA;
	}

	return SW_NO_ERROR;
}


/*
*@Function: GPCertificate_Valid
*
*@Descrip : 判断PERFORM SECURITY OPERATION指令中某个证书有效性
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : pkcaObj     	--- 输入参数，通过kid,kvn获取到的SCP11_PK_CA_KLOC_ECDSA 对象
*@param	  : certBuf     	--- 输入参数，证书数据, 7F21LV
*@param	  : totleLen    	--- 输入参数，certBuf中有效数据的长
*@param	  : pKExtracted 	--- 输入/输出参数，先用其校验证书签名，然后再从证书中提取出public key value; LV结构，其中V不包括最前面的04
*
*@Return  : return SW_NO_ERROR if cert is Valid, else return others;
*
*@History : V1.0 --- created by qiuping @20210720
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u16 GPCertificate_Valid(PN8 pkcaObj, PM8 certBuf, u16 totleLen, PX8 pKExtracted)
{
	ASN1Info X_A caTlv;
	u32 pkBufLen;
	u16 result;
	u8	pkRef;
	//获取的数据是LV而不是V，L大于80小于FF时是81 L的格式，所以长度需要加1, 统一使用宏定义更方便修改
	u8 X_A scp11PkValueLV[SCP11_PK_MAX_LV_LEN]; // 这里获取的是证书中7F49的内容，格式为7F49L + B0L04V + F001XX

	if(PX_R_U8(g_current_PerformSecurityOperation_certNum) == 0x01)	//is first cert
	{
		//验证证书中 CSN 字段的有效性, 若对应白名单存在，第一个证书的 CSN 应存在于对应白名单匹配
		result = GPCertificate_ValidOfCSN(pkcaObj, certBuf, totleLen);

		if(result != SW_NO_ERROR)
		{
			return result;
		}
	}

	//验证证书中 Signature 字段的有效性,
	result = GPCertificate_ValidOfSignature(certBuf, totleLen, pKExtracted, PX_R_U8(g_current_SCP11SessionPara_ocePkRef));

	if(result != SW_NO_ERROR)
	{
		return result;
	}

	if(GPCertificate_GetField(certBuf, totleLen, GPCERT_PublicKey_Tag, scp11PkValueLV, sizeof(scp11PkValueLV)) != RESPONSE_NoError)	//获取证书 PublicKey 字段，LV
	{
		return SW_WRONG_DATA;
	}

	pkBufLen = scp11PkValueLV[0];

	ASN1_InitTLVInfo(&caTlv, PXtMEM(&scp11PkValueLV[0x01]), pkBufLen);

	if(FALSE == ASN1_FindTLV(&caTlv, GPCERT_PublicKey_VALUE_Tag, FALSE, 1))	//Public Key-Q ‘B0' 字段是否存在
	{
		return SW_WRONG_DATA;
	}

	pKExtracted[0] = caTlv.curValueLen - 1;

	memCopyToRam(&pKExtracted[1], MEMtPM8(caTlv.bufferAddr + caTlv.curValueOff + 1), pKExtracted[0]);	//获取PublicKey的Public Key-Q ‘B0' 字段，LV

	if(FALSE == ASN1_FindTLV(&caTlv, GPCERT_PublicKey_REFERENCE_Tag, FALSE, 1))	//Key Parameter Reference 'F0' 字段是否存在
	{
		return SW_WRONG_DATA;
	}
	memCopyToRam(MEMtPX8(&pkRef), MEMtPM8(caTlv.bufferAddr + caTlv.curValueOff), 1 );

	if((caTlv.curValueLen > 0x01) || (pkRef != PX_R_U8(g_current_SCP11SessionPara_ocePkRef)))	//PkRef暂只支持一个字节
	{
		return SW_WRONG_DATA;
	}

	if(checkPointOnEccCurve(&pKExtracted[1], pKExtracted[0], PX_R_U8(g_current_SCP11SessionPara_ocePkRef)) == false)		//检查PublicKey是否在曲线上
	{
		return SW_WRONG_DATA;
	}

	return SW_NO_ERROR;
}


/*
*@Function: GPCertificate_ValidOfSignature
*
*@Descrip : 验证证书中 Signature 字段的有效性
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : certBuf     --- 证书数据 , TLV
*@param	  : totleLen    --- certBuf中有效数据的长
*@param	  : pKValue		--- 验签Public key Value , LV结果，V中不包含04
*@param	  : pKRef		--- 验签Public key Reference , V
*
*@Return  : return SW_NO_ERROR if Signature is Valid, else return others;
*
*@History : V1.0 --- created by qiuping @20210720
*/
u16 GPCertificate_ValidOfSignature(PM8 certBuf, u16 totleLen, PX8 pKValue, u8 pKRef)
{
	u8 	signatureHalfLen;
	u16	off, signatureBufOff;
	u32	len;
	ASN1Info X_A caTlv;
	u8 algType;

	//获取被签名数据起始偏移
	off = ASN1_GetBERTlvLen(PXtMEM(&certBuf[2]), (u32*)&len);

	off = off + 2;

	ASN1_InitTLVInfo(&caTlv,  PMtMEM(&certBuf[off]), totleLen - off);

	if(FALSE == ASN1_FindTLV(&caTlv, GPCERT_Signature_Tag, FALSE, 1))
	{
		return SW_WRONG_DATA;
	}

	switch(caTlv.curValueLen)
	{
		case 2*ALG_EC_FP_KEY_256:
			algType = ALG_ECDSA_SHA_256;
			break;
		case 2*ALG_EC_FP_KEY_384:
			algType = ALG_ECDSA_SHA_384;
			break;
#if WITH_ECC_KEY_512
		case 2*ALG_EC_FP_KEY_512:
			algType = ALG_ECDSA_SHA_512;
			break;
#endif
#if WITH_ECC_KEY_521
		case 2*ALG_EC_FP_KEY_521:
			algType = ALG_ECDSA_SHA_512;
			break;
#endif
		default:
			return SW_WRONG_DATA;

	}

	len = len - caTlv.curTLVLen;	//被签名数据的长度

	signatureBufOff = caTlv.curValueOff + off;	//签名值的偏移

	signatureHalfLen = caTlv.curValueLen / 2;

	if(VerifySignForECDSA(&pKValue[1], pKValue[0], pKRef, &certBuf[off], len, &certBuf[signatureBufOff], &certBuf[signatureHalfLen + signatureBufOff], caTlv.curValueLen, algType) == true)
	{
		return SW_NO_ERROR;
	}

	return SW_VERIFICATION_CERT_FAILED;
}


/*
*@Function: GPCertificate_ValidOfCSN
*
*@Descrip : 验证证书中 CSN 字段的有效性
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf> 6.4  PERFORM SECURITY OPERATION Command
*
*@param	  : certBuf     --- 证书数据
*@param	  : totleLen    --- certBuf中有效数据的长
*
*@Return  : return SW_NO_ERROR if CSN is Valid, else return others;
*
*@History : V1.0 --- created by qiuping @20210720
*/
u16 GPCertificate_ValidOfCSN(PN8 pkcaObj, PM8 certBuf, u16 totleLen)
{

	u8 X_A 	cSNBuf[0x11];
	jref 		whitelistRef;
	PX8 		whitelistBuf;
	PX8 		tagBuf;
	u8			tagLen;
	u32			len;	//这个len变量定义为u16时，当代码中使用强转操作时，会冲掉相邻的局部变量的值，这个是编译器问题还是固定出现的问题？？？
	u16			off;

	ecpMem_Set(cSNBuf, 0x00, sizeof(cSNBuf));
	if(GPCertificate_GetField(certBuf, totleLen, GPCERT_CertSerNum_Tag, cSNBuf, sizeof(cSNBuf)) != RESPONSE_NoError)	//获取证书Certificate Serial Number字段，LV
	{
		return SW_WRONG_DATA;
	}
	if((cSNBuf[0] < 0x01) || (cSNBuf[0] > 0x10))
	{
		return SW_WRONG_DATA;
	}

	//判断CSN是否在白名单内
	whitelistRef = SCP11_PK_CA_KLOC_ECDSA_getWhitelistRef(pkcaObj);
	//If no whitelist is defined for the referenced PK.CA-KLOC.ECDSA (see section 6.8), CSNs found in the certificate chain shall not be checked.
	if(whitelistRef == NULL_REF)
	{
		return SW_NO_ERROR;
	}
	whitelistBuf = MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(whitelistRef))));
	if(whitelistBuf[0] == 0x00)
	{
		return SW_NO_ERROR;
	}

	off = ASN1_GetBERTlvLen(PXtMEM(whitelistBuf), (u32*)&len);
	len = len + off;

	for( ; off < len ; )
	{
		tagLen = whitelistBuf[off + 1];
		tagBuf = &whitelistBuf[off + 1];
		if(memCompare(tagBuf, cSNBuf, tagLen+1) == ECP_CMP_RESULT_EQUAL) // if(NvmCmp(tagBuf, cSNBuf, tagLen+1) == EQUAL)
		{
			return SW_NO_ERROR;
		}
		off = off + tagLen + 2;
	}

	return SW_CERT_NOT_IN_WHITELIST;
}


/*
*@function    :GPCertificate_GetField
*
*@description :获取GP证书的相关字段
*
*@param       :u8 *certificateAddr  :input, cert addr  7F21 TLV
*@param       :WORD  certificateLen   :input, cert length 包括7F21LV的总长度
*@param       :u8  wantTag          :input,指示待获取的字段    (比如GPCERT_CAId_Tag)
*@param       :u8 *dataBufLv        :output, LV格式的输出结果
*@param       :u16   bufSize          :input,dataBuf的可使用空间
*
*@return      :RESPONSE_NoError表示对应的字段存在且合法/others表示其他错误
*
*@history     :modify by jiangqingbin@20210715
*             :modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
u8 GPCertificate_GetField(PM8 certificateAddr,u16 certificateLen, u8 wantTag, PM8 dataBufLv, u16 bufSize)
{
	ASN1Info X_A caTlv;
	u8 I_A tagOccur;
	
	ASN1_InitTLVInfo(&caTlv,  PMtMEM(certificateAddr), certificateLen);		

	if(FALSE == ASN1_FindTLV(&caTlv, GPCERT_Tag, FALSE, 1))
	{
		return ErrorCode_invalidCertificate;
	}

	ASN1_SetNextDepthToFindTLV(&caTlv);  

	tagOccur = 1;
		
	if (GPCERT_Authorizations_Tag == wantTag)	//证书内双tag的5F20 和  BF20都是查找0x20
	{
		wantTag = 0x20;
		tagOccur = 2;
	}
	
	if(FALSE == ASN1_FindTLV(&caTlv, wantTag, FALSE, tagOccur))
	{
		return ErrorCode_invalidCertificate;
	}

	if (bufSize >= (caTlv.curValueLen +1))
	{
		if(GPCERT_Authorizations_Tag == wantTag)
		{
			PX_W_U32(g_current_SCP11SessionPara_oceAuthorizationsLv, caTlv.bufferAddr + caTlv.curValueOff - 1);
		}
		else
		{
			dataBufLv[0] = caTlv.curValueLen;

			memCopyToRam(dataBufLv+1, MEMtPM8(caTlv.bufferAddr + caTlv.curValueOff), dataBufLv[0] );
		}
	}
	else
	{
		return ErrorCode_invalidCertificate;
	}
	
	return RESPONSE_NoError;
}

/*
*@function    :sdData_getSCP11SessionPara
*
*@description :获取SCP11相关的全局变量PerformSecurityOperation、SCP11SessionPara、TmpCert，如果不存在的话则创建
*
*@See     	  :<GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>
*
*@param       :PN8 sdDataObj --- SecurityDomainData
*
*@return      :void
*
*@history     :created by xuxiaojun@20220926
*             :modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中。修改之后，代码可以合并到CIU9872B_01中
*/
void sdData_getSCP11SessionPara(PN8 sdDataObj, u8 mode)
{
#if (!GP__SCP__SCP11__CERT_IN_GLOBAL_RAM)
	jref		D_A		certRef;
	PN8			D_A		object;

	certRef = sdData_getSCP11_TmpCert(sdDataObj);

	if (REF_NULL == certRef)
	{
		if(mode == MODE_CREATE_SCP11_SESSIONPARA)
		{
			certRef = make_Transient_Array(SIZE_BYTE, Header_ArrayType_Byte, Byte_PA_CLASS_OFFSET, CLEAR_ON_DESELECT, TmpCert_LENGTH, MAKE_TRANSIENT_ARRAY_COMMON_MODE);
			sdData_setSCP11_TmpCert(sdDataObj, certRef);
		}
		else
		{
			return;
		}
	}
	else
#endif
	{
		if(mode == MODE_GET_SCP11_SESSIONPARA)
		{
#if GP__SCP__SCP11__CERT_IN_GLOBAL_RAM
			G_TmpCert = g_current_SCP11_TmpCert;
#else			
			object		= MEMtPN8(locateObjectInMemory(certRef));

			G_TmpCert = MEMtPX8(get_array_data(object));
#endif
		}
		else
		{
			return;
		}
	}
}

/*
*@function    :BF20_GetField
*
*@description :获取BF20的相关字段
*
*@See     	  : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  B.2 Limitations on Commands Received through SCP11c
*
*@param       :u8  wantTag        :input,指示待获取的字段
*@param       :PM8 dataBufLv      :output, LV格式的输出结果
*@param       :u16 bufSize        :input,dataBuf的可使用空间
*@param       :u8  tagOccur  		:input,Tag查找次数
*
*@return      :true表示对应的字段存在且合法/false表示其他错误
*
*@history     :modify by qiuping@20210727
*             :modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
#if GP__SCP__SCP11__C_AUTHORIZATIONS
u8 BF20_GetField(u8 wantTag, PM8 dataBufLv, u16 bufSize, u8 tagOccur)
{
	ASN1Info X_A caTlv;
	u16 off;
	u32	len;

	off = ASN1_GetBERTlvLen(PXtMEM(g_current_SCP11SessionPara_oceAuthorizationsLv), (u32*)&len);

	ASN1_InitTLVInfo(&caTlv,  PXtMEM(PX8_A_U8(g_current_SCP11SessionPara_oceAuthorizationsLv, off)), len);

	switch(wantTag)
	{
		case 0x8A:
		case 0xC5:
		{
			if(FALSE == ASN1_FindTLV(&caTlv, 0xE3, FALSE, 1))
			{
				return false;
			}
			break;
		}
		case 0x82:
		case 0x83:
		case 0x8B:
		case 0x8C:
		case 0x8E:
		case 0x8F:
		{
			if(FALSE == ASN1_FindTLV(&caTlv, 0xEF, FALSE, 1))
			{
				return false;
			}
			break;
		}
		default:
			return false;
	}


	ASN1_SetNextDepthToFindTLV(&caTlv);

	if(FALSE == ASN1_FindTLV(&caTlv, wantTag, FALSE, tagOccur))
	{
		return false;
	}

	if (bufSize >= (caTlv.curValueLen +1))
	{
	 	dataBufLv[0] = caTlv.curValueLen;

		memCopyToRam(dataBufLv+1, MEMtPM8(caTlv.bufferAddr + caTlv.curValueOff), dataBufLv[0] );
	}
	else
	{
		return false;
	}

	return true;
}
#endif

/*
*@Function: SCP11AnalyzeStoreData
*
*@Descrip : analyze the APDU-STORE DATA  (ECKA Certificate/Whitelist/CA-KLOC Identifier)
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  STORE DATA(ECKA Certificate/Whitelist/CA-KLOC Identifier)的解析函数入口
*
*@param	  : sdDataObj  --- the SecurityDomainData of the performing SD
*
*@Return  : void
*
*@History : V1.0 --- created by jiangqingbin @20210713，
*         : V1.1 --- modify by xuxiaojun@20220528 未考虑到remove white list的操作。 测试用例：SCP11c_1  C_sm_STORE_DATA_Whitelist_SUCCESS_removal_256_a1_60_20
          : V1.2 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*/
void SCP11AnalyzeStoreData(PN8 sdDataObj)
{
	jref	pkcaRef;
	PN8		pkcaObj;
	u16 I_A offSet;	//APDU的偏移，不是RecData的偏移，有可能大于0xff
	u8 I_A p3;
	u8 I_A tagInByte ;	//tag 占用的字节数
	u8 I_A keyVersion;
	u8 I_A keyId;
	u8 I_A tagValue[2];//tag 的value
	u8 X_A tagOffset[4];	//tagOffset[0] indicate offSet of A6, tagOffset[1] indicate offSet of BF21,   tagOffset[2] indicate offSet of 92, tagOffset[3] indicate offSet of 70
	u8 X_A IdentifierBufLv[0x17];
	u16 I_A counterValue;
	u32 X_A tagValueLen;	//tag 的value的长度值
	u16 I_A tmpOffset;
	u8	I_A transactionStarted;
	u16  I_A result;

	transactionStarted = 0;

#define TAG_A6     0x00
#define TAG_BF21   0x01
#define TAG_92     0x02
#define TAG_70     0x03

	ecpMem_Set(IdentifierBufLv, 0x00, sizeof(IdentifierBufLv)); // IdentifierBufLv[0] = 0;
	counterValue = 0;
	tagValueLen = 0;
	keyVersion = 0;
	keyId = 0;
	
	p3 = APDUBuffer[APDU_P3_OFFSET];

	offSet = APDU_DATA_OFFSET;		//5

	ecpMem_Set(tagOffset, 0x00, sizeof(tagOffset));
	
	while (offSet < U8_A_U8(APDU_DATA_OFFSET, p3))
	{
		 tagInByte = ASN1_GetBERTlvTag(PXtMEM(&APDUBuffer[offSet]), tagValue);

		if(tagInByte == 1)
		{
			if (tagValue[0] == 0xA6)
			{
				if(tagOffset[TAG_A6] != 0x00) // A6重复出现
				{
					throw_error(SW_WRONG_DATA);
				}

				tagOffset[TAG_A6] = offSet;
			}
			else if (tagValue[0] == 0x92)
			{
				if(tagOffset[TAG_92] != 0x00) // 0x92重复出现
				{
					throw_error(SW_WRONG_DATA);
				}

				tagOffset[TAG_92] =  offSet;
			}
			 else if (tagValue[0] == 0x70)
			{
				if(tagOffset[TAG_70] != 0x00) // 0x70重复出现
				{
					throw_error(SW_WRONG_DATA);
				}

				tagOffset[TAG_70] =  offSet;
			}
			else
			{
				throw_error(SW_WRONG_DATA);	//unknow tag
			}			
		}
		else//double tag
		{
			if ((tagValue[0] == 0xBF) && (tagValue[1] == 0x21))
			{
				if(tagOffset[TAG_BF21] != 0x00) // 0xBF21重复出现
				{
					throw_error(SW_WRONG_DATA);
				}

				tagOffset[TAG_BF21] =  offSet;
			}
			else
			{

				throw_error(SW_WRONG_DATA);	//unknow tag
			}
		}

		offSet += tagInByte;

		offSet += ASN1_GetBERTlvLen(PXtMEM(&APDUBuffer[offSet]), (u32*)&tagValueLen);

		offSet += tagValueLen;
	}

	if(tagOffset[TAG_A6] == 0x00) // A6不存在 , 这个是冗余代码，因为进入该函数的前提条件就是store data后的第一个tag是0xA6
	{
		throw_error(SW_WRONG_DATA);

		return;
	}

	//analy A6
	offSet = tagOffset[TAG_A6];

	offSet = U8_A_U8(offSet, 1); // point to A6 L

	offSet += ASN1_GetBERTlvLen(PXtMEM(&APDUBuffer[offSet]), (u32*)&tagValueLen);	//offSet point to A6 V

	tmpOffset = ASN1_FindTagTLV(PXtMEM(&APDUBuffer[offSet]), tagValueLen, 0x83); // A6的子TAG 83是否存在

	if(tmpOffset == TAG_NOT_FOUND) // A6下面的83是必须存在的
	{
		throw_error(SW_WRONG_DATA);

		return;
	}

	if(FALSE == SCP11AnalyzeA6_83( &APDUBuffer[offSet + tmpOffset + 1], &keyVersion, &keyId)) //解析83
	{
		throw_error(SW_WRONG_DATA); // 0x83的数据不合法
	}

	tmpOffset = ASN1_FindTagTLV(PXtMEM(&APDUBuffer[offSet]), tagValueLen, 0x42); // A6的子TAG 42是否存在

	if(tmpOffset != TAG_NOT_FOUND) // A6下面存在42
	{
		if((tagOffset[TAG_BF21] != 0x00) || (tagOffset[TAG_92] != 0x00) || (tagOffset[TAG_70] != 0x00))
		{
			throw_error(SW_WRONG_DATA); // 存在与A6并列的TAG
		}

		if(FALSE == SCP11AnalyzeA6_42(&APDUBuffer[offSet + tmpOffset + 1], IdentifierBufLv))
		{
			throw_error(SW_WRONG_DATA); // 0x42的数据不合法
		}

		SCP11StoreIdentifier(sdDataObj, IdentifierBufLv,  keyVersion,  keyId); // 这里是没有判断A6下面有多余tag的情况，如果有多余的，按照现在的代码逻辑，是会忽略的
	}
	else // A6下面不存在42
	{
		if(tagOffset[TAG_BF21] != 0x00) // 存在与A6并列的BF21
		{
			if((tagOffset[TAG_92] != 0x00) || (tagOffset[TAG_70] != 0x00)) // 存在与A6并列的其他TAG
			{
				throw_error(SW_WRONG_DATA);
			}

//			SCP11AnalyzeBF21();//证书的合法性由外部保证
			SCP11StoreECKACert(sdDataObj, APDUBuffer + tagOffset[TAG_BF21] , keyVersion, keyId, p3-(tagOffset[TAG_BF21]-APDU_DATA_OFFSET));
		}
		else if(tagOffset[TAG_70] != 0x00) // 存在与A6并列的70
		{
			//Tag '92' shall be present in the command if it is received in an SCP11c session and shall not be present otherwise.
			if(((tagOffset[TAG_92] == 0x00) && (PX_R_U8(g_current_SCP11SessionPara_sessionType) == SCP11_C))
				|| ((tagOffset[TAG_92] != 0x00) && (PX_R_U8(g_current_SCP11SessionPara_sessionType) != SCP11_C)))
			{
				throw_error(SW_WRONG_DATA);
			}

			pkcaRef = SCP11GetPKCARefByKeyVerAndKeyId(sdDataObj, keyVersion, keyId);

			if(pkcaRef == NULL_REF)
			{
				throw_error(SW_REFERENCED_DATA_NOT_FOUND);
			}

			if(FALSE == SCP11Analyze70(APDUBuffer + tagOffset[TAG_70] + 1)) // 检测白名单的合法性
			{
				throw_error(SW_WRONG_DATA);
			}

			// 需要开事务
			if (!isInProgress())
			{
				begin_transaction(SINGLE_APDU_PROCESS);
				transactionStarted = 1;
			}

			result = SCP11StoreWhitelist(sdDataObj, APDUBuffer + tagOffset[TAG_70] + 1, keyVersion,   keyId); // 没有考虑多条指令的情况

			if((result == SW_SECURITY_STATUS_NOT_SATISFIED) || (result == SW_REFERENCED_DATA_NOT_FOUND) || (result == SW_WRONG_DATA))
			{
				throw_error(result); //此时抛一场时，前面还没有这与，所有这里不需要abort事务
			}

			if(result == REMOVE_WHITE_LIST)//不需要再写counter
			{

			}
			else
			{
				pkcaObj = MEMtPN8(locateObjectInMemory(pkcaRef));

				if(tagOffset[TAG_92] != 0x00)
				{
					counterValue = SCP11_PK_CA_KLOC_ECDSA_getInternalCounter(pkcaObj);

					if(FALSE == SCP11Analyze92(APDUBuffer+tagOffset[2]+1,  &counterValue))
					{
						throw_error(SW_WRONG_DATA);
					}
					SCP11_PK_CA_KLOC_ECDSA_setInternalCounter(pkcaObj, counterValue);
				}
			}

			if (transactionStarted)
			{
				commit_transaction();
			//需要提交事务
			}
		}
		else
		{
			throw_error(SW_WRONG_DATA);
		}
	}
}


/*
*@Function: checkSCP11cSTOREDATATagInBF20
*
*@Descrip : 检查 STORE DATAT 指令tag是否符合BF20规定
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  B.2 Limitations on Commands Received through SCP11c
*
*@param	  : tag   --- STORE DATAT tag
*@param	  : isDgi --- true if tag is dgi, else false
*
*@Return  : return true if the tag is valid, else return false;
*
*@History : V1.0 --- created by qiuping @20210728
*/
#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
u8 checkSCP11cSTOREDATATagInBF20(u16 tag, u8 isDgi)
{
	u8 tagBuf[0x20];
	u8 off;
	
	if((PX_R_U8(g_current_SCP11SessionPara_sessionType) != SCP11_C) ||
		(U8_AND_U8(channelInfo.security_level[selected_channel], SCF_AUTHENTICATED_ANY) != SCF_AUTHENTICATED_ANY ))
	{
		return true;
	}

	ecpMem_Set(tagBuf, 0x00, sizeof(tagBuf));

	if(isDgi == true)
	{
		if(BF20_GetField(BF20_DGI_LIST_STOREDATA_TAG, tagBuf, sizeof(tagBuf), 1) == false)
		{
			return false;
		}
		for(off = 1; off < sizeof(tagBuf); )
		{
			if(PX_R_U16(tagBuf[off]) != tag)
			{
				off = U8_A_U8(off, 0x02);
			}
			else
			{
				return true;
			}
		}
	}
	else
	{
		if(BF20_GetField(BF20_TLV_LIST_STOREDATA_TAG, tagBuf, sizeof(tagBuf), 1) == false)
		{
			return false;
		}
		for(off = 1; off < sizeof(tagBuf); )
		{
			if(DOUBLE_TAG == U16_AND_U16(DOUBLE_TAG , tagBuf[off]) || (0x2F == tagBuf[off]))
			{
				if(PX_R_U16(tagBuf[off]) != tag)
				{
					off = U8_A_U8(off, 0x02);
				}
				else
				{
					return true;
				}
			}
			else
			{
				if(tagBuf[off] != tag)
				{
					off = U8_A_U8(off, 0x01);
				}
				else
				{
					return true;
				}
			}
		}
	}


	return false;
}
#endif
/*
*@Function: SCP11_decryptDataValid
*
*@Descrip : 检查调用API是否合法
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  5.3.3  Key Access Conditions
*
*@param	  : void
*
*@Return  : return true if valid, else return false;
*
*@History : V1.0 --- created by xuxiaojun @20210830
*         : V1.1 --- modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
*         : V1.2 --- modify by xxj@20230512: 修正缺陷487时，做对应的调整
*/
u8 SCP11_decryptDataValid(void)
{
	u8 keyAccess;
	u8 tmpKeyVersion;
	PN8 keySetObj;
	jref keySetRef;

#if GP__SCP__SCP11__FOR_MASK
	if(g_current_SCP_Type == SCP_11) // 5.3.3  Key Access Conditions  这里做的不完整
	{
		keySetRef = keySet_getKeySet((PN8)getCurrentSdDataObj(IS_NO_NEED_CHECK_SD, &tmpKeyVersion), PX_R_U8(g_current_PerformSecurityOperation_keyVersion));

		if(REF_NULL == keySetRef)
		{
			throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
		}

		keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));

		keyData_getKeyAccess(keySetObj, 0x16, &keyAccess); // KID '16' for the optional static Key-DEK used with SCP11c only

		if(keyAccess == 0x00) // Setting up a Secure Channel with an SK.SD.ECKA having Key Access Conditions set to '00' shall	always be accepted.
		{

		}
		else if(keyAccess == 0x01) // Setting up a Secure Channel with an SK.SD.ECKA having Key Access Conditions set to '01' shall only be accepted if the SD holding the keys is the selected or the targeted application.
		{

		}
		else if(keyAccess == 0x02) // An attempt by the SD to use the Key-DEK having Key Access Conditions set to '02' shall fail
		{
			return false;
		}
	}
#endif
	return true;

}
#endif

