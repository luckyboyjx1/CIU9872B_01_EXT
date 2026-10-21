#ifndef    _AUTHENTICATION_H_
#define    _AUTHENTICATION_H_
#include   "typedef.h"

#define	D_AUTHMODE_2G	0x02
#define	D_AUTHMODE_3G	0x03
#define	D_OP_Mode		0x00
#define	D_OPc			0x02
#define	D_F2_Mode		0x01
#define	D_F3_Mode		0x02
#define	D_F4_Mode		0x03
#define	D_F5_Mode		0x01
#define	D_F1Start_Mode	0x02
#define D_F5Start_Mode	0x04

#define	D_SQNCheck_Offset	102
#define D_R_Offset      17

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

#define AlgorithmType_DummyXOR  				0x00
#define AlgorithmType_MilenageR1R5TEST  		0x01
#define AlgorithmType_MilenageNORMAL  			0x02



/*
 * Number of columns (32-bit words) comprising the State. For this 
 * standard, Nb = 4.
 */
#define AES_Nb 4 //4字 = 16字节为一块

/*
 * Number of 32-bit words comprising the Cipher Key. For this 
 * standard, Nk = 4, 6, or 8.
 */
//BYTE idata Nk;
#define AES_Nk 4 

/*
 * Number of rounds, which is a function of  Nk  and  Nb (which is 
 * fixed). For this standard, Nr = 10, 12, or 14.
 */
// Nr 为给定密钥大小的轮数减 1。加密算法使用的轮数要么是10，12，要
// 么是14，这依赖于种子密钥长度是128位、192位还是256位
//BYTE idata Nr;
#define AES_Nr 10

#define AES_ENCRYPT 0x00
#define AES_DECRYPT 0x01

/*
#ifdef IOTService
#define AES_ENCRYPT 0x01
#define AES_DECRYPT 0x00
#endif
*/

#define CMAC_BLOCKSIZE_MAX     0x10
#define RB_128BITS          0x87 //BlockSize=16字节时，Rb=R128=0(120)10000111=0x(0000......87)


extern BYTE code TestR[5];

extern void AESDeChipher(BYTE * pbSrcStr,BYTE *keyContext,BYTE * pbTgtStr, BYTE aesMod, BYTE lgth,BYTE keyLen);

extern BYTE ToolkitForAESCalculateMac(UWORD eeAddr,BYTE ccLength,WORD length,BYTE *keyContext,BYTE * resultBuffer);

extern void Toolkit_AESPORMac(BYTE *keyContext,BYTE *macBuf,BYTE macLength,UWORD srcAddr,WORD length);

extern void AESChipherForPor(BYTE *keyContext,UWORD srcDstAddr,WORD srcDstOff,BYTE mode,WORD lgth);

extern void AESEncryptCoreCipher(BYTE *in, BYTE *out, BYTE *w);

extern void key_expansion(BYTE *key, BYTE *w);

extern void inv_cipher(BYTE *in, BYTE *out, BYTE *w) ;

extern void coef_mult(BYTE *a, BYTE *b, BYTE *d);

extern void add_round_key(BYTE *state, BYTE *w, BYTE r);

extern void Rotata(BYTE input[16],BYTE output[16],BYTE r);
extern void KeySchedule(BYTE key[16]);
extern void KeyAdd(BYTE round);

extern void ByteSub(void);

extern void sub_bytes(BYTE *state);

extern void shift_rows(BYTE *state);

extern void mix_columns(BYTE *state);

extern void ShiftRow(void);

extern void MixColumn(void);

extern void StringSub(BYTE * str1, BYTE * str2, BYTE * result, BYTE len);
extern BYTE CheckSQN(void);
extern void F2toF5Function(BYTE input[16], BYTE output[16], BYTE mode);
extern void F1Function(BYTE input[16], BYTE output[16]);
extern BYTE Milenage(BYTE * output);
extern void Milenage2G(BYTE * output);
extern BYTE USIMAuthenticate(BYTE mode);
extern BYTE MilenageInit(void);

extern BYTE USIMXORAuthenticate(void);

extern void RunGsmAlgorithmV0V5(void);


#endif
