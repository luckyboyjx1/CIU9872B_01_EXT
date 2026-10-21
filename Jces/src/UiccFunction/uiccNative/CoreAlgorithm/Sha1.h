
#ifndef SHA1_H
#define SHA1_H
#include "typedef.h"


#define Sn(a,x) (((a)<<(x))|((a)>>(32-x)))	//rotates a left x bits  循环左移位操作

#define DIGEST_LENGTH 20
#define DIGEST_LENGTH_256 32

#define HMAC_DIGEST_LENGTH_SHA1     20
#define HMAC_DIGEST_LENGTH_SHA256   32

typedef __packed struct 
{
    unsigned long  state[5];    /*初始值H0,H1,H2,H3,H4*/
    unsigned char buffer[64];
    unsigned long  count[2];
}SHA1_CTX;



typedef struct {
	unsigned char digest[DIGEST_LENGTH_256]; /* Message digest */
	DWORD count[2]; /* count of bits */
	DWORD vdata[16]; /* data buffer */
}SHA_INFO;


extern SHA1_CTX  xdata G_Sha1Context;
extern SHA_INFO  xdata G_SHA256_Context;

/*****************************function define************************************/
extern void convertBig2Little(unsigned char tgData[64],unsigned char len);
extern void SHA1Init( SHA1_CTX * context );
extern void SHA1Update(SHA1_CTX * context, unsigned char * dat, unsigned int len,BYTE mode,UWORD addr) ;
extern void SHA1DoFinalSingleCall(unsigned char * digest /*[20]*/, unsigned char* sourcedata,unsigned int length, BYTE mode,UWORD fileAddr);
//extern void SHA1Final(SHA1_CTX* shaContext , BYTE * result);
extern void SHA1Transform(  unsigned long * state /*[5]*/,unsigned char * buffer /*[64]*/ );

extern void SHA256Init(SHA_INFO *sha_info);
extern void SHA256Update(SHA_INFO *sha_info, unsigned char *buffer, unsigned int count, BYTE mode, UWORD fileAddr);
extern void SHA256Final(SHA_INFO *sha_info, BYTE * result);
#if !JAVA_CARD_UICC
//extern void SHA256DoFinalSingleCall(unsigned char * digest /*[32]*/, unsigned char* sourcedata,unsigned int length, BYTE mode, UWORD fileAddr);
#endif


#endif
