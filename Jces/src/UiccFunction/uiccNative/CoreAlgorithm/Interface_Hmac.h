#ifndef _INTERFACE_HMAC_H_
#define _INTERFACE_HMAC_H_
#include "typedef.h"
#include "Sha1.h"
#include "md5.h"

/**************macro define****************************************/

#define ALG_HMAC_NULL               0
#define ALG_HMAC_MD5	            1
#define ALG_HMAC_SHA1               2
#define ALG_HMAC_SHA256	            3

#define HMAC_DIGEST_LENGTH_MD5      16
#define HMAC_DIGEST_LENGTH_SHA1     20
#define HMAC_DIGEST_LENGTH_SHA256   32
#define HMAC_DIGEST_LENGTH_MAX      32  //目前最大的hmac——hash输出为sha256 32 字节

/*************function define**************************************/
extern BYTE HMACSha256Init(BYTE* kBuf, BYTE kLen, BYTE xorChar, SHA_INFO * context);
extern void HMACSha256Update(BYTE * src, WORD srcLgth, SHA_INFO * context);
extern BYTE HMACSha256DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, SHA_INFO * context);
extern void HMACSha256DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest);

extern BYTE HMACMD5Init(BYTE* kBuf, BYTE kLen, BYTE xorChar, MD5_CTX * context);
extern void HMACMD5Update(BYTE * src, WORD srcLgth, MD5_CTX * context);
extern BYTE HMACMD5DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, MD5_CTX * context);
extern void HMACMD5DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest);

extern BYTE HMACSha1Init(BYTE* kBuf, BYTE kLen, BYTE xorChar, SHA1_CTX * context);
extern void HMACSha1Update(BYTE * src, WORD srcLgth, SHA1_CTX * context);
extern BYTE HMACSha1DoFinal(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest, SHA1_CTX * context);
extern void HMACSha1DoFinalSingleCall(BYTE * keyBuf,BYTE keyLen, BYTE * src, WORD srcLgth, BYTE * dest);

extern BYTE HMACSha256DoFinalInEEprom(BYTE * keyBuf,BYTE keyLen, UWORD fileAddr, WORD srcLgth, BYTE * dest, SHA_INFO * context);
extern void HMACSha256DoFinalSingleCallInEEprom(BYTE * keyBuf,BYTE keyLen, UWORD fileAddr, WORD srcLgth, BYTE * dest);

#endif
