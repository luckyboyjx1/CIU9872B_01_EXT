#ifndef    _EAPAUTHENTICATION_H_
#define    _EAPAUTHENTICATION_H_
#include  "typedef.h"

//extern BYTE xdata state[4][4];
extern WORD xdata   G_EapBlockOffset;

/*typedef struct TagEapContext{

	//BYTE Authstutas;
	BYTE EapDirindex;
	BYTE Messageindex;
	BYTE Eaptype;
	BYTE Eapsubtype;
	BYTE VersionListlen;
	WORD VersionList[16];//暂定8个version
	WORD SelectedVersion;
	BYTE Nonce[16];
	BYTE MK[20];
	BYTE K_encr[16];
	BYTE K_auth[16];
	BYTE MSK[64];
	BYTE EMSK[64];


	BYTE EapHavePermID;
	BYTE EapHavePseuID;
	BYTE EapHaveReauID;
	BYTE EapAuthMode;
	BYTE EapResendFlag;
	BYTE EapHaveATResultInd;
}EapContext;*/

typedef struct TagEapContext{

	//BYTE EapDirindex;
	BYTE Messageindex;
	BYTE Eaptype;
	BYTE Eapsubtype;
	BYTE VersionListlen;
	WORD VersionList[16];//暂定8个version
	WORD SelectedVersion;
	//BYTE Nonce[16];
	//BYTE MK[20];
	//BYTE K_encr[16];
	//BYTE K_auth[16];
	//BYTE MSK[64];
	//BYTE EMSK[64];


	//BYTE EapHavePermID;
	BYTE EapHavePseuID;
	BYTE EapHaveReauID;
	BYTE EapAuthMode;
	BYTE EapResendFlag;
	BYTE EapHaveATResultInd;
}EapContext;

#define  NonceOffset		0x00
#define  MKOffset			0x10
#define  K_encrOffset		0x24
#define  K_authOffset		0x34

#define  Nonce    (&ResData[NonceOffset])
#define  MK		  (&ResData[MKOffset])
#define  K_encr   (&ResData[K_encrOffset])
#define  K_auth	  (&ResData[K_authOffset])


#define AT_MAC_OFFSET		0x04

#define AT_IV_OFFSET		0x14

#define AT_AUTN_OFFSET		0x24

#define AT_RAND_OFFSET		0x34


#define     EAP_Identity				0x01
#define     EAP_SIM						0x12
#define		EAP_AKA						0x17

#define 	EAP_REQUEST	    			0x01
#define 	EAP_RESPONSE				0x02
#define		EAP_SUCESS	    			0x03
#define 	EAP_FAILURE					0x04

#define     AKA_Challenge			    0x01
#define     AKA_AuthenticationReject	0x02
#define     AKA_SynchronizationFailure	0x04
#define     AKA_Identity				0x05

#define     SIM_Start				    0x0A
#define     SIM_Challenge				0x0B

#define     EAP_Notification			0x0C
#define     EAP_Reauthentication		0x0D
#define     EAP_ClintError				0x0E

#define     ERROR_UnknowPacket			0x01
#define     ERROR_UnsupportVersion		0x02
#define     ERROR_NeedmoreRand		    0x03
#define     ERROR_NeedfreshRand		    0x04

#define  	AT_RAND                     0x01
#define  	AT_AUTN                     0x02
#define  	AT_RES                      0x03
#define  	AT_AUTS                     0x04
#define  	AT_PADDING                  0x06
#define  	AT_NONCE_MT                 0x07
#define  	AT_PERMANENT_ID_REQ         0x0A
#define  	AT_MAC                      0x0B
#define  	AT_NOTIFICATION             0x0C
#define  	AT_ANY_ID_REQ               0x0D                                     
#define  	AT_IDENTITY                 0x0E                                     
#define  	AT_VERSION_LIST             0x0F                                     
#define  	AT_SELECTED_VERSION         0x10                                     
#define  	AT_FULLAUTH_ID_REQ          0x11                                     
#define  	AT_COUNTER                  0x13                                     
#define  	AT_COUNTER_TOO_SMALL        0x14                                     
#define  	AT_NONCE_S                  0x15                                     
#define  	AT_CLIENT_ERROR_CODE        0x16                                     
#define  	AT_IV                       0x81                                     
#define  	AT_ENCR_DATA                0x82                                     
#define  	AT_NEXT_PSEUDONYM           0x84                                     
#define  	AT_NEXT_REAUTH_ID           0x85                                     
#define  	AT_CHECKCODE                0x86                                     
#define  	AT_RESULT_IND               0x87 

#define		EF_EAPKEYS					0x4F01
#define		EF_EAPSTATUS				0x4F02
#define     EF_PUID						0x4F03
#define     EF_PSID						0x4F04
#define		EF_CurID					0x4F20
#define		EF_ReID						0x4F21
#define		EF_Realm					0x4F22

#define		DF_EAP  					0x77F0
#define		EF_EapBlockData				0x4FFF


#define		EAP_AuthEdle				0x00
#define		EAP_Authenticating			0x01
#define		EAP_AuthSucess				0x02
#define		EAP_AuthFailure				0x03

#define     EAP_FullAuth				0x00
#define     EAP_FastAuth				0x01

// #define     P1_FirstBlockData			    0x80
// #define     P1_NextBlockData			    0x00
//#define     P1_RetransmitPreviousData       0x40
//#define     P1_FirstBlockResponse			0xA0
//#define     P1_NextBlockResponse			0x20
//#define     P1_RetransmitPreviousResponse   0x60

#define     BER_TAG			                0x53
#define     AUTHENTICATE_ODD                0x89

extern 		EapContext   xdata   G_currentEAPContext;

//extern		BYTE 		 idata	 G_ResponseLen;

extern BYTE EAPAuthenticate(void);

extern void IF_EapSystemInit(void);

//extern BYTE SaveBlockData(void);

extern BYTE IsEapApplication(void);

extern void  AESArithFunction(BYTE* initVector,BYTE * input,BYTE * keyContext,BYTE * output, BYTE bMod, BYTE lgth,BYTE addrMode);

extern int fips186_2_prf(const u8 *seed, unsigned char seed_len, u8 *x, unsigned char xlen);


//extern BYTE IF_SelectByEAPDFName(void);
extern BYTE IF_SelectDFFile(BYTE dirNo);

/////////////////////////////////////////////////////start

extern BYTE EapAuthCheck(void);

extern BYTE EAPAuthentication(void);

#endif
