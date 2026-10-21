/*
 * SCPGenKeyInterface.c
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */


#include "SCPInterface.h"
#include "CardManager.h"
#include "Loader.h"
#include "opFunction.h"
#include "global_variable.h"
#include "error.h"
#include "profile.h"
#include "scp01.h"
#include "scp02.h"
#include "SCP03.h"
#include "scp10.h"
#include "Crypto_Constant.h"
#include "SCPGenKeyInterface.h"
#include "Crypto_API.h"
#include "Boot.h"

#if GP__SCP

void generateSCPKey(PX8 keySetObj)
{

	switch(g_current_SCP_Type)
	{
#if SCP_SWITCH
#if GP__SCP__SCP01
		case SCP_01:
			generateKeyScp01(keySetObj);
			break;
#endif
#endif // GP__SCP__SCP01
		case SCP_02:
			generateKeyScp02(keySetObj);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			SCP03_generateKey(keySetObj);
			break;
#endif //GP__AMENDMENT__D
		default:
			break;
	}
	//ecpMem_Set(g_current_SCP_LAST_MAC, CLEARED_VALUE, LAST_MAC_LENGTH);
	//memCopyNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC), CLEARED_VALUE, LAST_MAC_LENGTH);
	memFillNoAtomic(MEMtPM8(g_current_SCP_LAST_MAC), CLEARED_VALUE, LAST_MAC_LENGTH); // todo 这里将g_current_SCP_LAST_MAC的长度固定为LAST_MAC_LENGTH是存在问题的，该长度应该是与g_SCP_macKeyLen 或

}

void generateCMACKey(PX8 derivationData, PN8 keySetObj)
{
	#define KEY_LENGTH_SUPPORT 0x10
	PX8				pt;
	u8 		X_A		icv[8];
	//memref 	X_A		keyAddress;

	u8 		X_A		sesKeyBuf[SESSION_KEY_LENGTH];
//	u8 		X_A		keyAddress[LAST_MAC_LENGTH];
	u8 		X_A		keyAddress[SESSION_KEY_LENGTH];

	if(SCP_10 != g_current_SCP_Type)
	{
		g_SCP_macKeyLen = U16t8(keyData_getKeyDataById(keySetObj, SCP_STATICKEY_SECOND_KEYID, 0x00,  keyAddress));
	}
	else
	{
		g_SCP_macKeyLen = 0x10;
	}

	ecpMem_Set(icv, CLEARED_VALUE, sizeof(icv));
	//pt = g_current_SCP_MAC_KEY;
	pt = sesKeyBuf;

	switch(g_current_SCP_Type)
	{
#if SCP_SWITCH
		case SCP_01:
			Triple_DES(derivationData, (PN8) keyAddress, pt, (u16) g_SCP_macKeyLen, (PX8) icv, ECB_Encrypt, SENSITIVE_DATA_MASKED);
			break;
#endif // GP__SCP__SCP01
		case SCP_02:
			if(KEY_LENGTH_SUPPORT != g_SCP_macKeyLen)
			{// do not support, if key length is not 16bytes
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
			derivationData[1] = U32t8( CONSTANTS_C_MAC);
			Triple_DES(derivationData, MEMtPM8(keyAddress) , pt, U8t16(g_SCP_macKeyLen), MEMtPM8( icv), CBC_Encrypt, SENSITIVE_DATA_MASKED);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			SCP03_generateKeyProc((PN8) keyAddress, g_SCP_macKeyLen, pt, SCP03_DERI_CONST_S_MAC , derivationData);
			break;
#endif //GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			ecpMem_Copy(pt, keySetObj, g_SCP_macKeyLen);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}
	
	memCopyNoAtomic(MEMtPM8(g_current_SCP_MAC_KEY), pt, g_SCP_macKeyLen);//modify by jiangxiaohui@20220324 分层
}

#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
void generateRMACKey(PX8 derivationData, PN8 keySetObj)
{
	PX8				pt;
	u8 		X_A		icv[8];
//	u8 		X_A		keyAddress[LAST_MAC_LENGTH];
	u8 		X_A		keyAddress[SESSION_KEY_LENGTH];
	u8 		X_A		sesKeyBuf[SESSION_KEY_LENGTH];

	if(SCP_10 != g_current_SCP_Type)
	{
		g_SCP_rmacKeyLen = U16t8(keyData_getKeyDataById(keySetObj, SCP_STATICKEY_SECOND_KEYID, 0x00, keyAddress));
	}
	else
	{
		g_SCP_rmacKeyLen = 0x10;
	}

	ecpMem_Set(icv, CLEARED_VALUE, sizeof(icv));
	//pt = g_current_SCP_RMAC_KEY;
	pt = sesKeyBuf;

	switch(g_current_SCP_Type)
	{
#if SCP_SWITCH
		case SCP_01:
			break;
#endif // GP__SCP__SCP01
		case SCP_02:
			derivationData[1] = U32t8( CONSTANTS_R_MAC);
			Triple_DES(derivationData, MEMtPN8( keyAddress) ,pt, U32t16(16), MEMtPX8(icv), CBC_Encrypt, SENSITIVE_DATA_MASKED);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			SCP03_generateKeyProc((PN8) keyAddress, g_SCP_rmacKeyLen, pt, SCP03_DERI_CONST_S_RMAC , derivationData);
			break;
#endif //GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			ecpMem_Copy(pt, keySetObj, g_SCP_rmacKeyLen);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}
	
	memCopyAtomic(MEMtPN8(g_current_SCP_RMAC_KEY), MEMtPM8(pt), g_SCP_rmacKeyLen);
}
#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC ||GP__SCP__SCP03_RMAC)

void generateCENCKey(PX8 derivationData, PN8 keySetObj)
{
	#define KEY_LENGTH_SUPPORT 0x10
	
	PX8				pt;
	u8 		X_A		icv[8];
	//memref 	X_A		keyAddress;
//	u8 		X_A		keyAddress[LAST_MAC_LENGTH];
	u8 		X_A		keyAddress[SESSION_KEY_LENGTH];
	u8 		X_A		sesKeyBuf[SESSION_KEY_LENGTH];
	
	if(SCP_10 != g_current_SCP_Type)
	{
		g_SCP_sesKeyLen = U16t8(keyData_getKeyDataById(keySetObj, SCP_STATICKEY_FIRST_KEYID, 0x00, keyAddress));
	}
	else
	{
		g_SCP_sesKeyLen = 0x10;
	}

	ecpMem_Set(icv, CLEARED_VALUE, sizeof(icv));
	//pt = g_current_SCP_SES_KEY;
	pt = sesKeyBuf;


	switch(g_current_SCP_Type)
	{
#if SCP_SWITCH
		case SCP_01:
			memCopyToRam((memref) pt, (memref) keyAddress, (u16) g_SCP_sesKeyLen);
			break;
#endif // GP__SCP__SCP01
		case SCP_02:
			if(KEY_LENGTH_SUPPORT != g_SCP_sesKeyLen)
			{// do not support, if key length is not 16bytes
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
		
			derivationData[1] = U32t8( CONSTANTS_S_ENC);
			Triple_DES(derivationData, MEMtPM8( keyAddress) , pt, U8t16( g_SCP_sesKeyLen), MEMtPM8( icv), CBC_Encrypt, SENSITIVE_DATA_MASKED);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
			SCP03_generateKeyProc((PN8) keyAddress, g_SCP_sesKeyLen , pt, SCP03_DERI_CONST_S_ENC , derivationData);
			break;
#endif //GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			ecpMem_Copy(pt, keySetObj, g_SCP_sesKeyLen);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}
	
	memCopyNoAtomic(MEMtPM8(g_current_SCP_SES_KEY), pt, g_SCP_sesKeyLen);//modify by jiangxiaohui@20220324 分层
	initBootSCPsessionKey(PNtMEM(g_current_SCP_SES_KEY));//接口需调整
}
#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC||GP__SCP__SCP03_RMAC)
void generateRENCKey(PX8 derivationData, PN8 keySetObj)
{
//	PX8				pt;
//	u32 keyAddress;

//	if(SCP_10 != g_current_SCP_Type)
//	{
//		g_SCP_rencKeyLen = (u8)keyData_getKeyDataById(keySetObj, SCP_STATICKEY_FIRST_KEYID, 0x00, (RAM_U32_P) &keyAddress);
//	}
//	else
//	{
		g_SCP_rencKeyLen = 0x10;
//	}
//	pt = MEMtPX8(g_current_SCP_RSES_KEY);

	switch(g_current_SCP_Type)
	{
#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			//ecpMem_Copy(pt, keySetObj, g_SCP_rencKeyLen);
			nvmCopyAtomic(MEMtPN8(pt), MEMtPM8(keySetObj), g_SCP_rencKeyLen);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}
}
#endif //(GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC ||GP__SCP__SCP03_RMAC)
void generateCDECKey(PX8 derivationData, PN8 keySetObj)
{
	#define KEY_LENGTH_SUPPORT 0x10
	PX8				pt;
	u8 		X_A		icv[8];
	//memref 	X_A		keyAddress;
//	u8 		X_A		keyAddress[LAST_MAC_LENGTH];
	u8 		X_A		keyAddress[SESSION_KEY_LENGTH];
	u8 		X_A		sesKeyBuf[SESSION_KEY_LENGTH];

	if(SCP_10 != g_current_SCP_Type)
	{
		g_SCP_dekKeyLen = U16t8(keyData_getKeyDataById(keySetObj, SCP_STATICKEY_THIRD_KEYID, 0x00, keyAddress));
	}
	else
	{
		g_SCP_dekKeyLen = 0x10;
	}

	ecpMem_Set(icv, CLEARED_VALUE, sizeof(icv));
	//pt = g_current_SCP_DEK_KEY;
	pt = sesKeyBuf;

	switch(g_current_SCP_Type)
	{
#if SCP_SWITCH
		case SCP_01:
			Triple_DES(derivationData, (PN8) keyAddress, pt, (u16) g_SCP_dekKeyLen, (PX8) icv, ECB_Encrypt, SENSITIVE_DATA_MASKED);
			break;
#endif // GP__SCP__SCP01
		case SCP_02:
			if (KEY_LENGTH_SUPPORT != g_SCP_dekKeyLen)
			{// do not support, if key length is not 16bytes
				throw_error(SW_CONDITIONS_NOT_SATISFIED);
			}
			derivationData[1] = U32t8( CONSTANTS_DEK);
			Triple_DES(derivationData, MEMtPM8( keyAddress) , pt, U8t16( g_SCP_dekKeyLen), MEMtPM8( icv), CBC_Encrypt, SENSITIVE_DATA_MASKED);
			break;
#if GP__AMENDMENT__D
		case SCP_03:
//			memCopyToRam((memref)pt, (memref)pointer_to_ref(keyAddress), g_SCP_dekKeyLen);
			memCopyToRam(MEMtPX8((memref)pt), MEMtPX8((memref)keyAddress), g_SCP_dekKeyLen);
			break;
#endif //GP__AMENDMENT__D

#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			ecpMem_Copy(pt, keySetObj, g_SCP_dekKeyLen);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}
	
	memCopyNoAtomic(MEMtPM8(g_current_SCP_DEK_KEY), pt, g_SCP_dekKeyLen);//modify by jiangxiaohui@20220324 分层
}

void generateRDECKey(PX8 derivationData, PN8 keySetObj)
{
#if GP__SCP__SCP10__FOR_MASK
	PN8				pt;
	
	g_SCP_rdekKeyLen = 0x10;

	pt = MEMtPN8(g_current_SCP_RDEK_KEY);
#endif

	switch(g_current_SCP_Type)
	{
#if GP__SCP__SCP10__FOR_MASK
		case SCP_10:
			//ecpMem_Copy(pt, keySetObj, g_SCP_rdekKeyLen);
			nvmCopyNoAtomic(pt, keySetObj, g_SCP_rdekKeyLen);
			break;
#endif //GP__SCP__SCP10__FOR_MASK
		default:
			break;
	}

}


#if BASE_KEY
void generateStaticKey(PM8 baseKey, PM8 staticKey, PM8 derivationData)
{
	u8		X_A		bTemp[16];
	u8		X_A		icv[8];
	ecpMem_Copy(bTemp, derivationData, sizeof(bTemp));

	bTemp[7] = U32t8(0x01);
	bTemp[15] = U32t8( 0x01);

	Triple_DES(MEMtPX8(bTemp), baseKey, MEMtPX8(&staticKey[0]), U32t16(16), MEMtPM8(icv), ECB_Encrypt, SENSITIVE_DATA_MASKED);

	bTemp[7] = U32t8( 0x02);
	bTemp[15] = U32t8( 0x02);

	Triple_DES(MEMtPX8(bTemp), baseKey, MEMtPX8( &staticKey[16]), U32t16(16), MEMtPM8( icv), ECB_Encrypt, SENSITIVE_DATA_MASKED);

	bTemp[7] = U32t8(0x03);
	bTemp[15] = U32t8( 0x03);
//	pt = (u8 *) (&staticKey[32]);
	Triple_DES(MEMtPX8 (bTemp), baseKey, MEMtPX8( &staticKey[32]), U32t16(16), MEMtPM8( icv), ECB_Encrypt, SENSITIVE_DATA_MASKED);

}

void generateStaticKeyScp03(u8 * baseKey, u8 * staticKey, u8  * derivationData)
{
	u8		X_A		bTemp[16];
	u8		X_A		icv[16];
	u8 enclen, maclen, deklen;
	ecpMem_Copy(bTemp, derivationData, 16);
//	enclen = RAM_READ_BIG_U8(baseKey, SCP03_ENC_KEY_LENGTH);
//	deklen = RAM_READ_BIG_U8(baseKey, SCP03_DEK_KEY_LENGTH);
//	maclen = RAM_READ_BIG_U8(baseKey, SCP03_MAC_KEY_LENGTH);
	enclen = PM_R_U8(baseKey + SCP03_ENC_KEY_LENGTH);
	deklen = PM_R_U8(baseKey + SCP03_DEK_KEY_LENGTH);
	maclen = PM_R_U8(baseKey + SCP03_MAC_KEY_LENGTH);

	bTemp[7] = (u8) 0x01;
	bTemp[15] = (u8) 0x01;

//	AES((u8 *) (bTemp), (u8 *) (baseKey+SCP03_KEYDATA), (u16)(enclen*8), (u8 *) &staticKey[0], (u16) 16, (u8 *) icv, ECB_Encrypt);
	SymCipherDofinal(bTemp, 16, &staticKey[0], baseKey+SCP03_KEYDATA, (u16)(enclen*8), icv, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, ECB_Encrypt, SBC_INIT, ALG_AES);

	bTemp[7] = (u8) 0x02;
	bTemp[15] = (u8) 0x02;

//	AES((u8 *) (bTemp), (u8 *) (baseKey+SCP03_KEYDATA), (u16)(maclen*8), (u8 *) &staticKey[16], (u16) 16, (u8 *) icv, ECB_Encrypt);
	SymCipherDofinal(bTemp, 16, &staticKey[16], baseKey+SCP03_KEYDATA, (u16)(maclen*8), icv, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, ECB_Encrypt, SBC_INIT, ALG_AES);

	bTemp[7] = (u8) 0x03;
	bTemp[15] = (u8) 0x03;

//	AES((u8 *) (bTemp), (u8 *) (baseKey+SCP03_KEYDATA), (u16)(deklen*8), (u8 *) &staticKey[32], (u16) 16, (u8 *) icv, ECB_Encrypt);
	SymCipherDofinal(bTemp, 16, &staticKey[32], baseKey+SCP03_KEYDATA, (u16)(deklen*8), icv, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, ECB_Encrypt, SBC_INIT, ALG_AES);

//	TEMPBuffer[0] = RAM_READ_BIG_U8(baseKey, SCP03_SEQUENCE_COUNTER);
//	TEMPBuffer[1] = RAM_READ_BIG_U8(baseKey, SCP03_SEQUENCE_COUNTER+1);
//	TEMPBuffer[2] = RAM_READ_BIG_U8(baseKey, SCP03_SEQUENCE_COUNTER+2);
	TEMPBuffer[0] = PM_R_U8(baseKey + SCP03_SEQUENCE_COUNTER);
	TEMPBuffer[1] = PM_R_U8(baseKey + SCP03_SEQUENCE_COUNTER+1);
	TEMPBuffer[2] = PM_R_U8(baseKey + SCP03_SEQUENCE_COUNTER+2);
}
#endif

#endif
