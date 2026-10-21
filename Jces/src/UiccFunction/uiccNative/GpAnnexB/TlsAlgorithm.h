#ifndef  _TLSALGORITHM_H_
#define  _TLSALGORITHM_H_
#include "typedef.h"

/****************macro define***********************************/
#define ALG_NULL         0x00
#define ALG_DES_CBC      0x01
#define ALG_DES_ECB	     0x02
#define ALG_AES_CBC		 0x03
#define ALG_AES_ECB		 0x04
#define ALG_AES_GCM		 0x05

/*******************function define*******************************/
extern void TLSP_HASH(BYTE alg, BYTE * secret, BYTE secretLen, BYTE  * label, BYTE * seed, BYTE * out, BYTE outLen);
extern void TLSPRF(BYTE * secret,BYTE secretLen, BYTE * label, BYTE * seed, BYTE * out, BYTE outLen);
extern void TLSKeyBlockCalculation(void);
extern void TLSGetSessionKey(BYTE keyFileDataOff,BYTE *outBuf);
extern void TLSWriteSessionKey(BYTE keyFileDataOff,BYTE *inBuf);
extern void TLSCalMasterSecret(void);
extern BYTE TLSGetPreMasterSecret(BYTE *preMasterSecretBuf);
extern BYTE TLSGetFinishedVerifyData(BYTE * verifyData,  BYTE * label);
extern void TLSCalMsgMAC(BYTE msgMode, BYTE blockMode, BYTE *macKeyBuf, BYTE *dataBuf, WORD dataLen, BYTE ommitLen, BYTE *macBuf, BYTE * HashWorkSpace);
extern BYTE TLSGetPendingLen(WORD length);
extern void TLSEncryDecryMsg(BYTE *keyBuf, BYTE *IVBuf,  BYTE *dataBuf,WORD dataLgth, BYTE operMode, BYTE blockMode);
extern BYTE HttpsHandShakeHashUpdata(BYTE *hashShakeBuf, BYTE hashShakeBufLen, BYTE blockNum, BYTE *outBuf);
extern WORD DESCBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out);
extern BYTE TLSGetPSK(BYTE* psk);
#endif

