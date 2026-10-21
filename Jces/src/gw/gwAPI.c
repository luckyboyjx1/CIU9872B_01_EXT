/*
 * gwAPI.c
 *
 *  Created on: 2020年10月19日
 *      Author: jiangxiaohui
 */
#include "gwAPI.h"
#include "JcvmConfig.h"
#include "ecpDefine.h"
#include "profile.h"
#include "JcvmDataConfig.h"
#include "typedef.h"
#include "uicc_main.h"

#if UICC_FUCTION_PRIVATEAUTH
//u8 		g_SM4NormalFlag; //是否是标准的SM4，0表示非标准SM4，1表示标准SM4
u8		g_SymGuomiPro;

//u8 JcvmConfig_GetProfileEnablePriAuth(u8 profileindex)
//{
//	return PN_R_U8(PN8_A_U16(JcvmConfig_GetEnablePriAuthBase(), profileindex));
//}
//
//void JcvmConfig_SetProfileEnablePriAuth(u8 profileindex, u8 flag)
//{
//	nvmWriteU8NoAtomic(PN8_A_U16(MEMtPN8(JcvmConfig_GetEnablePriAuthBase()), profileindex), flag);
//}

//u8 JcvmConfig_GetEnablePriAuth(void)
//{
//	u8 profileindex = Euicc_getCurExecProfile();
//
//	if(profileindex == ILLEGAL_PROFILEINDEX)
//	{
//		return PN_R_U8(MEMtPN8(JcvmConfig_GetEnablePriAuthBase()));
//	}
//	else
//	{
//		return	JcvmConfig_GetProfileEnablePriAuth(profileindex);
//	}
//}

#if UICC__GUOWANG__AUTH_SPECIAL_CONFIG
u8 Pirvate_CheckGWSecData(u16 srcOffset, u8 length)
{
	if(srcOffset > GW_SEC_DATA_MAX_SIZE)
	{
		return false;
	}
	else if(srcOffset + length > GW_SEC_DATA_MAX_SIZE )
	{
		return false;
	}
	return true;
}

void SetOrGetGWSecDataNative(void)
{
	PM8				desBuf;
	u16 	X_A 	length;
	u16 	X_A 	desOffset;
	u16 	X_A 	srcOffset;
	u8		D_A		mode;


	length = popShort();
	desOffset = popShort();
	desBuf = getJavaObjectDataArray();
	srcOffset = popShort();
	mode	= U16t8(popShort());

	if(mode == SETGWSECDATA_MODE)
	{
		Pirvate_SetSWSecData(srcOffset,desBuf,desOffset,length);
	}
	else
	{
		Pirvate_GetGWSecData(srcOffset,desBuf,desOffset,length);
	}

}

void Pirvate_GetGWSecData(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length)
{
	if(Pirvate_CheckGWSecData(srcOffset, length))
	{
		memCopyNoAtomic(PM8_A_U16(desAddr, desOffset), PM8_A_U16(MEMtPM8(GW_SEC_DATA_ADDRESS), srcOffset), length);
	}
}

void Pirvate_SetSWSecData(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length)
{
	if(Pirvate_CheckGWSecData(srcOffset, length))
	{
		memCopyNoAtomic(PM8_A_U16(MEMtPM8(GW_SEC_DATA_ADDRESS), srcOffset), PM8_A_U16(desAddr, desOffset), length);
	}
}

#endif

BYTE *GetGtempBuf_Process(void)
{
	return &tempBuf[0];
}

void EcpMem_Set_Process_JumpArea(BYTE *dest, BYTE value, WORD len)
{
	ecpMem_Set(dest, value, len);
}
void EcpMem_Copy_Process_JumpArea(BYTE *dest, BYTE *src, WORD len)
{
	ecpMem_Copy(dest, src, len);
}
int ecpMem_Compare_Process_JumpArea(BYTE * buf1,BYTE *buf2,WORD lgth)
{
	return ecpMem_Compare(buf1,buf2,lgth);
}
void ecpMem_Move_Process_JumpArea(BYTE *dest, BYTE *src, WORD len)
{
	ecpMem_Move(dest,src,len);
}

void SymCipherUpdate_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 mode, u8 sbcState, u8 algType, u8 symAlg)
{
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
	SymCipherUpdate( inBuff,  inLen,  outBuff,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  mode,  sbcState,  algType, symAlg);
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
}

u8 SymCipherDofinal_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg)
{
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
	return SymCipherDofinal(inBuff,  inLen,  outBuff,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  algType,  mode,  sbcState,  symAlg);
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
}

void SymSignatureUpdate_JumpArea(PM8 inBuff, u16 inLen, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg)
{
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
	SymSignatureUpdate( inBuff,  inLen,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  algType,  mode,  sbcState,  symAlg);
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
}

u8 SymSignatureDofinal_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 macLen, u8 symAlg)
{
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_GW);
	return SymSignatureDofinal( inBuff,  inLen,  outBuff,  keyBuff,  keyBitLen,  ICVBuff,  SYMBuff,  algType,  mode,  sbcState,  macLen,  symAlg);
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
}

void HASHUpdate_JumpArea(PM8 inBuff, u16 inLen, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType)
{
	HASHUpdate( inBuff,  inLen,  mdState,  mdBuff,  tmpBuff,  algType);
}

void HASHDoFinal_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType)
{
	HASHDoFinal( inBuff,  inLen,  outBuff,  mdState,  mdBuff,  tmpBuff,  algType);
}

void Pirvate_GetGWSecData_JumpArea(u16 srcOffset, BYTE *  desAddr, u16 desOffset, u8 length)
{
	Pirvate_GetGWSecData(srcOffset, desAddr, desOffset, length);
}

void Pirvate_SetSWSecData_JumpArea(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length)
{
	Pirvate_SetSWSecData(srcOffset, desAddr, desOffset, length);
}

#endif
