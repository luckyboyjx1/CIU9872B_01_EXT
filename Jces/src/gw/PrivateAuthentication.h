#ifndef    _PRIVATEAUTHENTICATION_H_
#define    _PRIVATEAUTHENTICATION_H_
#include "typedef.h"
#include "profile.h"

// #define KI_MODE    0x00
// #define OPC_MODE   0x01
// #define OTHER_MODE 0x02

#define	D_AUTHMODE_2G		0x02
#define	D_AUTHMODE_3G		0x03
#define D_AUTHMODE_GBA_BOOT	0x04
#define	D_OP_Mode			0x00
#define	D_OPc				0x02


#define	D_F2_Mode			0x01
#define	D_F3_Mode			0x02
#define	D_F4_Mode			0x03
#define	D_F5_Mode			0x01
#define	D_F1Start_Mode		0x02
#define D_F5Start_Mode		0x04

#define SQN_MAX_Mode 		0x01
#define INDEX_SQN_Mode 	0x02
#define SET_SQN_Mode		0x03


typedef struct
{
	void (*PriRotata_Process_entry) (BYTE *,BYTE *,BYTE );
	void (*PriF2toF5Function_Process_entry) (BYTE * , BYTE *, BYTE *, BYTE );
	void (*PriF1Function_Process_entry) (BYTE * , BYTE *, BYTE *);
	BYTE (*PriCheckSQN_Process_entry) (BYTE *, BYTE *);
	BYTE (*PriMilenage_Process_entry) (BYTE * ,BYTE *);
	void (*PriMilenage2G_Process_entry) (BYTE * ,BYTE *);
	BYTE (*PriUSIMAuthenticate_Process_entry) (BYTE, BYTE *);
	DWORD a[32-7];
}PriAuthFunctionEntry;

extern PriAuthFunctionEntry const PriAuthFunctionList;

extern void PriRotata_Process(BYTE input[16],BYTE output[16],BYTE r);
extern void PriF2toF5Function_Process(BYTE * inData, BYTE input[16], BYTE output[16], BYTE mode);
extern void PriF1Function_Process(BYTE * inData, BYTE input[16], BYTE output[16]);
extern BYTE PriCheckSQN_Process(BYTE * input,BYTE * inData);
extern BYTE PriMilenage_Process(BYTE * input, BYTE * output);
extern void PriMilenage2G_Process(BYTE * input, BYTE * outpu);
extern BYTE PriUSIMAuthenticate_Process(BYTE mode, BYTE *output);

#endif
