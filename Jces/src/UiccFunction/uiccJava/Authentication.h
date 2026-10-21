#ifndef    _AUTHENTICATION_H_
#define    _AUTHENTICATION_H_

#include "typedef.h"
#include "crypto_impl.h"

//modify by xuweihua@20210310  AES算法key修改成4字节对齐（270改成272）
#define roundKeys  ((BYTE *)U32_AND_U32(U32_A_U16(&tempBuf[0], 0x14c), 0xFFFFFFFC))  //确保roundKeys是4字节对齐的,借用tempBuf 最后182 字节作为roundKeys 数组，减少RAM 使用空间, by xwh 20190328

#define ROUND_KEYS_SIZE  176 


#define P1_FirstBlockData	0x80
#define P1_NextBlockData	0x00

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

#define KI_MODE    0x00
#define OPC_MODE   0x01
#define OTHER_MODE 0x02

#define	D_SQNCheck_Offset	102

#define	RAND	(&RecData[1])		//保存网络下发的随机数（16字节）
#define	AUTH	(&RecData[18])		//保存网络下发的AUTH字段（16字节）
#define SQN		(&RecData[18])		//保存网络下发的SQN字段（6字节）
#define	AMF		(&RecData[24])		//保存网络下发的AMF字段（2字节）
#define	MAC		(&RecData[26])		//保存网络下发的MAC字段（8字节）
#define	ki		(&RecData[100])		//保存卡内鉴权的Key（16字节）
#define	OPMode	(RecData[116])		//保存卡内鉴权用OP字段的模式（Bit2）和是否SQN需要AK进行异或（Bit1）（1字节）
#define	OPc		(&RecData[117])		//保存卡内鉴权用OP字段内容（16字节）
#define	R		(&RecData[133])		//保存卡内鉴权常数R字段内容（5字节）
#define C		(&RecData[138])		//保存卡内鉴权常数C字段内容（80字节）
#define RESLGTH	(RecData[226])		//保存RES输出长度(1字节)
#define SQNAK  (&RecData[227])		//保存SQN和xRes异或值,EPS用到该值;


#define AUTH_MODE_GBA_BOOT   1
#define AUTH_MODE_GBA_ME     3
#define AUTH_MODE_GBA_CLEAR  0xFF

extern BYTE xdata G_ResponseLen;


//extern void KeyAdd(BYTE round);

//extern void ByteSub(void);

//extern void ShiftRow(void);

//extern void MixColumn(void);

//extern BYTE xdata state[4][4];;

extern BYTE USIMXORAuthenticate(void);

extern BYTE CheckSQN(void);

//extern void KeySchedule(BYTE key[16]);

//extern void AESEncrypt(BYTE input[16], BYTE output[16]);

//extern BYTE xdata G_Auth_Rand[16];

//extern BYTE xdata G_Auth_Ks[32];

extern BYTE GBANAFAuthenticate(void);

extern BYTE xdata G_GBA_AUTH_Status;

extern BYTE xdata G_Auth_Rand[17];			

extern BYTE xdata G_Auth_Ks[33];			

//extern void SHA1Compute(unsigned char * out, unsigned char * in, unsigned long len);

extern void HMAC_SHA256(unsigned char * k, unsigned int lk, unsigned char * d, unsigned int ld, unsigned char * out);




extern void  RUNA3A8_V1(BYTE keyId[16],BYTE  randNumber[16]);

extern void  RUNA3A8_V2(BYTE  key[16],BYTE randNumber[16]);

extern void  RUNA3A8_V3(BYTE  key[16],BYTE randNumber[16]);

extern BYTE generateME_KsNAF(BYTE* nafId,BYTE nafIdLen,BYTE *outBuf);

extern BYTE Milenage(BYTE * output);
extern BYTE Milenage2G(BYTE * output);
extern BYTE GSMXORA3A8(void);
#endif
