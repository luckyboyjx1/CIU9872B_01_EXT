#ifndef __RSA_API__
#define __RSA_API__

#include "Crypto_Constant.h"
#include "Crypto_common_API.h"
#include "halCryptoConfig.h"

/***********************************************************************************
 * RSA public exponent length for key paire generation
 ***********************************************************************************/
#define RSA_GEN_PUB_EXP_LEN    0x03

/***********************************************************************************
 * data stucture used mainly used in RSA operation.
 ***********************************************************************************/
typedef struct
{
	u16 wBufferLen;
	PX8 pbBuffer;
} CryptoData;

//-------------------------------------------------------------------------------------------------
// Define RSA Key Reference Store Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
	PM8 pbExp;	
	PM8 pbModulusN;	
	PM8 pbParmP;
	PM8 pbParmQ;
	PM8 pbParmDP1;
	PM8 pbParmDQ1;
	PM8 pbParmInvQ;
	u16 wExpLength;
	u16 wModulusNLength; // bytes length
	u16 wPrimeLength;
	u8 bRSAKeyType;
} RSAKeyRefStore;

//-------------------------------------------------------------------------------------------------
// Define RSA Key GEN Store Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
	PX8 pbModulusN;			/* used to store the point public or private key Modulus */
	PX8 pbParmP;			/* used to store the point private key D or CRT private key P */
	PX8 pbParmQ;			/* used to store the point CRT private key Q */
	PX8 pbParmDP;			/* used to store the point CRT private key DP */
	PX8 pbParmDQ;			/* used to store the point CRT private key DQ */
	PX8 pbParmInvQ;			/* used to store the point CRT private key INVQ */
} RSAKeyGenParam;

#if	(RSA_CRT_MODULAR_SAVE || RSA_CRT_E_SAVE)
typedef struct
{
#if RSA_CRT_MODULAR_SAVE
	PM8 pbCRTModulusN;
#endif
#if RSA_CRT_E_SAVE
	PM8 pbCRTExp;
#endif	
}RSAKeyGenCRTNEStore;

extern RSAKeyGenCRTNEStore		X_A		g_RSAKeyGenCRTNEStore;
#endif	
typedef struct RSA_SIGN_API_Param {
	PM8		pDigestInfo;            /**< \brief Pointer to initial value */
    u16		algType;				/***/
	u16		pDigestInfoLen;			/**< \length of Current Crypto' Block*/
    u16		digestSize;				/**< \byte number in nulti block mask*/
    u8		HashAlg;			/**< \byte number in nulti block mask*/
} RSA_SIGN_API_Param_t;

#define ISO9796_SIGN					0x5A5A
#define PKCS1_PSS_SIGN					0x5AA5
#define PKCS1_SIGN						0x5A3C
#define PKCS1_OAEP_SIGN                 0x4A4C    //added by jiangxiaohui@20220525 

#define ISO9796_VERIFICATION			0xA55A
#define PKCS1_PSS_VERIFICATION			0xA5A5
#define PKCS1_VERIFICATION				0xA53C

extern RSAKeyRefStore		  X_A		 g_RSAKeyStore;
extern CryptoData			  X_A		 g_CryptoData;
//-------------------------------------------------------------------------------------------------
// Hardware RSA API
//-------------------------------------------------------------------------------------------------
#if WITH_RSA
#if RSA_USE_CRYPTO_RAM
	extern void CryptoRAMCopyToUserRAM(PX8 des, PX8 src, u16 length);
#endif //RSA_USE_CRYPTO_RAM
#if WITH_RSA_KEY_GEN
	#define GET_MIN_RSA_MUD_BIT_LEN				512
	extern void GenRSAKeyPair(RSAKeyGenParam * myRsaKeyRef);
#else
	#define GenRSAKeyPair( myRsaKeyRef)
#endif //WITH_RSA_KEY_GEN
	extern u16 RSASTDCore(void);
	extern u16 RSACRTCore(void);
#else      //WITH_RSA
	#define GenRSAKeyPair( myRsaKeyRef)
	#define RSASTDCore()
	#define RSACRTCore()

#endif	//  WITH_RSA

#endif	// __RSA_API__
