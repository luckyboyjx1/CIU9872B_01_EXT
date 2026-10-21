#ifndef    _SCP03Realize_H_
#define    _SCP03Realize_H_
#include   "typedef.h"
#include   "profile.h"

#define  C_DECRYPTION     0x01
#define  C_ENCRYTION      0x02



#if  EUICC_FUNC_SCP03t
/*****************function define****************************************/
extern WORD SCP03tEncryptionAndRMAC(BYTE *inData, BYTE inDataLen, BYTE operationMode, BYTE *outBuf,BYTE *macKey,BYTE macKeyLen, BYTE *initMacValue, BYTE *encKey,BYTE encKeyLen,BYTE *icvCounter);
extern WORD SCP03tDecryptAndCalMac(UWORD srcDataAddr,UWORD outDataAddr, WORD srcDataLen, BYTE operationMode,BYTE *macKey,BYTE macKeyLen, BYTE *initMacValue, BYTE *encKey,BYTE encKeyLen,BYTE *icvCounter);
extern void SCP03tIncreaseICVCounter(BYTE *icvCounter);

extern WORD AESECB(BYTE *in,WORD inlen, BYTE *key,BYTE keylen,BYTE chipherMode,BYTE *out);

extern WORD AESCBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out);

extern WORD SM4ECB(BYTE *in,WORD inlen, BYTE *key,BYTE keylen,BYTE chipherMode,BYTE *out);

extern WORD SM4CBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out);

extern BYTE Cmac_SM4(BYTE *key, BYTE keylen, BYTE *M, WORD MLen, BYTE * headMsg, BYTE headMsgLen, BYTE *T);

#endif

#endif
