#ifndef    _COSPRIAUTHENTICATION_H_
#define    _COSPRIAUTHENTICATION_H_

#include "typedef.h"
#include "profile.h"

typedef struct
{	
	void (*EcpMem_Set_Process_entry) (BYTE *, BYTE , WORD );
	void (*EcpMem_Copy_Process_entry) (BYTE *, BYTE *, WORD );
	void (*ReadTFBuf_Process_entry) (BYTE * , WORD , BYTE * , WORD );
	void (*ReadLFBuf_Process_entry) (BYTE * , BYTE , WORD , BYTE * , WORD );
	void (*WriteTFBuf_Process_entry) (BYTE * , WORD , BYTE * , WORD );
	void (*WriteLFBuf_Process_entry) (BYTE * , BYTE , WORD , BYTE * , WORD );
	void (*KeySchedule_Process_entry) (BYTE *);
	void (*AESEncrypt_Process_entry) (BYTE *, BYTE *);
	void (*GetOrSetSqn_Process_entry) (BYTE , BYTE , BYTE *);
	int  (*ecpMem_Compare_Process_entry) (BYTE * ,BYTE *,WORD );
	void (*ecpMem_Move_Process_entry) (BYTE *, BYTE *, WORD );
	void (*StringSub_Process_entry) (BYTE * , BYTE * , BYTE * , BYTE );
	BYTE *(*GetGtempBuf_Process_entry) (void);
	void (*SymCipherUpdate_Process_entry)(BYTE * , u16 , BYTE * , BYTE * , u16 , BYTE * , BYTE * , BYTE , u8 , u8 , u8 );
	u8 (*SymCipherDofinal_JumpArea)(BYTE * , u16 , BYTE * , BYTE * , u16 , BYTE * , BYTE * , u8 , u8 , u8 , u8 );
	void (*SymSignatureUpdate_JumpArea)(BYTE * , u16 , BYTE * , u16 , BYTE * , BYTE * , u8 , u8 , u8 , u8 );
	u8 (*SymSignatureDofinal_JumpArea)(BYTE * , u16 , BYTE * , BYTE * , u16 , BYTE * , BYTE * , u8 , u8 , u8 , u8 , u8 );
	void (*HASHUpdate_JumpArea)(BYTE * , u16 , u8 , BYTE * , BYTE * , u8 );
	void (*HASHDoFinal_JumpArea)(BYTE * , u16 , BYTE * , u8 , BYTE * , BYTE * , u8 );
	void (*Pirvate_GetGWSecData_JumpArea) (u16 , BYTE * , u16 , u8 );
	void (*Pirvate_SetSWSecData_JumpArea) (u16 , BYTE * , u16 , u8 );
	DWORD a[32-17];
}CosPriAuthFunctionEntry;

extern CosPriAuthFunctionEntry const CosPriAuthFunctionList;

extern void ReadTFBuf_Process(BYTE * filePath, WORD offset, BYTE * buf, WORD len);

extern void ReadLFBuf_Process(BYTE * filePath, BYTE index, WORD offset, BYTE * buf, WORD len);

extern void WriteTFBuf_Process(BYTE * filePath, WORD offset, BYTE * buf, WORD len);

extern void WriteLFBuf_Process(BYTE * filePath, BYTE index, WORD offset, BYTE * buf, WORD len);

extern void GetOrSetSqn_Process(BYTE mode, BYTE index, BYTE *output);

extern BYTE *GetGtempBuf_Process(void);

#endif
