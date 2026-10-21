#ifndef AES_H
#define AES_H

#include   "typedef.h"

/*
 * Number of columns (32-bit words) comprising the State. For this 
 * standard, Nb = 4.
 */
#define AES_Nb 4 //4字 = 16字节为一块

#define MODE_ENCRYPT 0
#define MODE_DECRYPT 1

/*
 * Number of 32-bit words comprising the Cipher Key. For this 
 * standard, Nk = 4, 6, or 8.
 */

/*
 * Number of rounds, which is a function of  Nk  and  Nb (which is 
 * fixed). For this standard, Nr = 10, 12, or 14.
 */
// Nr 为给定密钥大小的轮数减 1。加密算法使用的轮数要么是10，12，要
// 么是14，这依赖于种子密钥长度是128位、192位还是256位

extern      BYTE    xdata  AES_Nk;
extern      BYTE    xdata  AES_Nr;

#define AES_ENCRYPT 0x00
#define AES_DECRYPT 0x01

extern void KeyExpansion(BYTE *key, BYTE *w);

extern void CoefMult(BYTE *a, BYTE *b, BYTE *d);
extern void AddRoundKey(BYTE *state, BYTE *w, BYTE r);
extern void KeyExpansion(BYTE *key, BYTE *w);
extern void AESEncryptCore(BYTE *in, BYTE *out, BYTE *w);
extern void AESDecryptCore(BYTE *in, BYTE *out, BYTE *w) ;
extern BYTE CheckAesKeyLength(BYTE keyLength);

extern void Xor(BYTE * src, BYTE * dest, WORD lgth);
extern BYTE AESECB(BYTE *in,WORD inlen, BYTE *key,BYTE keylen,BYTE chipherMode,BYTE *out);
extern WORD AESCBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out) ;

#endif
