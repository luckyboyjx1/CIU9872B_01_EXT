#ifndef	 _Supi_h_
#define  _Supi_h_
//#include "resume.h"
#include "Uicc_Main.h"


#define DF_5GS	0x5FC0
#define EF_NSI		0x4F09
#define EF_ROUTING_INDICATOR	0x4F0A

//#if SE_ONLY  // modify by chentao@20211103 4F01不是必须存在的文件
#define EF_SUCI_CALC_INFO		0x4F07	
//#if EUICC_ONLY || SE_AND_EUICC//modify by zhxx@20221208,解决SE编译不过问题
#define DF_SAIP	    0x5FD0
#define EF_SUCI_CALC_INFO_USIM	0x4F01
//#endif



#define  NULL_SCHEME 0x00
#define  PROFILE_A   0x01
#define  PROFILE_B   0x02
#define  DOMAIN      0x03

#define  IMSI_MODE   0x01
#define  NAI_MODE    0x02

extern BYTE xdata G_SUPILEN;

#define ECCKEYPAIR_PKLEN_PROFILEA 0x40 //模式为profile_a时keyPair的公钥长度暂且定义为该值，具体参考算法

#define ECCKEYPAIR_PKLEN_PROFILEB 0x42 //模式为profile_b时keyPair的公钥长度暂且定义为该值，具体参考算法

#define ECCKEYPAIR_SKLEN 0x20 //keyPair的私钥长度暂且定义为该值，具体参考算法


extern void GetIdentity(void);
//extern BYTE ECCKeyPair_ProfileB(BYTE *pkBuf, BYTE *skBuf);
extern void ECCKeyAgreet_ProfileB(BYTE * HNPublicKey, BYTE * USIMSKey, BYTE * shareKey);
extern void ECCPointDecompress(PX8 HNPublicKey);
#endif
