
#include "../gw/CosPriAuthentication.h"

#include	"string.h"
#include 	"ecpReadWrite.h"
#include 	"Authentication.h"
#include "euiccConfig.h"
#include "profile.h"
#if UICC_FUCTION_PRIVATEAUTH
extern void StringSub(BYTE * str1, BYTE * str2, BYTE * result, BYTE len);

extern void EcpMem_Set_Process_JumpArea(BYTE *dest, BYTE value, WORD len);
extern void EcpMem_Copy_Process_JumpArea(BYTE *dest, BYTE *src, WORD len);
extern int ecpMem_Compare_Process_JumpArea(BYTE * buf1,BYTE *buf2, WORD lgth);
extern void ecpMem_Move_Process_JumpArea(BYTE *dest, BYTE *src, WORD len);
extern void SymCipherUpdate_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 mode, u8 sbcState, u8 algType, u8 symAlg);
extern u8 SymCipherDofinal_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg);
extern void SymSignatureUpdate_JumpArea(PM8 inBuff, u16 inLen, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 symAlg);
extern u8 SymSignatureDofinal_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, PM8 keyBuff, u16 keyBitLen, PM8 ICVBuff, PM8 SYMBuff, u8 algType, u8 mode, u8 sbcState, u8 macLen, u8 symAlg);
extern void HASHUpdate_JumpArea(PM8 inBuff, u16 inLen, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType);
extern void HASHDoFinal_JumpArea(PM8 inBuff, u16 inLen, PM8 outBuff, u8 mdState, PM8 mdBuff, PM8 tmpBuff, u8 algType);
extern void Pirvate_GetGWSecData_JumpArea(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length);
extern void Pirvate_SetSWSecData_JumpArea(u16 srcOffset, PM8 desAddr, u16 desOffset, u8 length);

__attribute__((section("cospriauthfunctinonlist")))
CosPriAuthFunctionEntry CONST CosPriAuthFunctionList={
				EcpMem_Set_Process_JumpArea,//memset
				EcpMem_Copy_Process_JumpArea,
				ReadTFBuf_Process,//读透明文件的内容
				ReadLFBuf_Process,//读记录文件的内容
				WriteTFBuf_Process,
				WriteLFBuf_Process,
				KeyExpansion,//鉴权过程中AESkey的设置
				AESEncryptForUICC,
				GetOrSetSqn_Process,//SQN参数操作
				ecpMem_Compare_Process_JumpArea,
				ecpMem_Move_Process_JumpArea,
				StringSub,
				GetGtempBuf_Process,//tempBuf开发使用
				SymCipherUpdate_JumpArea,
				SymCipherDofinal_JumpArea,
				SymSignatureUpdate_JumpArea,
				SymSignatureDofinal_JumpArea,
				HASHUpdate_JumpArea,
				HASHDoFinal_JumpArea,
				Pirvate_GetGWSecData_JumpArea,	
				Pirvate_SetSWSecData_JumpArea
};
#endif
