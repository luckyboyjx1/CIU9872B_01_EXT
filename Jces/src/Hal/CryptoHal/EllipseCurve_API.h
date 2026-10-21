/*
 * EllipseCurve_API.h
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#ifndef ELLIPSECURVE_API_H_
#define ELLIPSECURVE_API_H_

#include "Crypto_Constant.h"
#include "halCryptoProfile.h"

#if WITH_ECC

#define HASH_DONE              0x5A5A

//typedef struct
//{
//    const unsigned char *iDataPtr;
//    unsigned char       *oDataPtr;
//    const unsigned int  *rndPtr;
//    unsigned int        *priKeyPtr;
//    unsigned int        *pubKeyPtr;
//    const unsigned int  *argPtr;
//} ECC_Parameters;

//typedef struct
//{
//    const unsigned char *iDataPtr;
//    unsigned char       *oDataPtr;
//    const unsigned int  *rndPtr;
//    unsigned int        *priKeyPtr;
//    unsigned int        *pubKeyPtr;
//    const unsigned int  *argPtr;
//} ECKA_Parameters;

typedef struct ecp_NumG {
	u8 * pPtr;                        /**< \brief Pointer. */
  	u16 len;                          /**< \brief Byte length. */
} ecp_NumG_t;

/**
 * \brief Structure for point in affine representation.
 */
typedef struct ecpEccGfp_PtAff {
	u8    *x;      /**< \brief x-coordinate */
	u8    *y;      /**< \brief y-coordinate */
} ecpEccGfp_PtAff_t;

/**
 * \brief Structure for ECC domain parameter.
 */
typedef struct ecpEccGfp_DomainParam {
	ecp_NumG_t		             p;  /**< \brief Prime p */
    u8                    		*a;  /**< \brief Curve parameter a */
    u8                    		*b;  /**< \brief Curve parameter b */
    ecpEccGfp_PtAff_t     		 G;  /**< \brief Base point G */
    ecp_NumG_t            		 n;  /**< \brief Base point order n */
} ecpEccGfp_DomainParam_t;

/**
 * \brief Structure of parameters for function ::ecpEccGfp_KeyGen
 */
typedef struct ecpEccGfp_KeyGen_Param {
    u8 X_A *pWorkspaceCPU;               /**< \brief Pointer to WorkspaceCPU */
    u8 X_A *pWorkspaceFX;                /**< \brief Pointer to WorkspaceFX */
    ecpEccGfp_DomainParam_t domain;      /**< \brief ECC domain parameter */
    ecpEccGfp_PtAff_t precPt;            /**< \brief ECC precompted point */
    u8 X_A *privKey;                     /**< \brief Pointer to memory area to store private key */
    u8 X_A *pubKey;                      /**< \brief Pointer to memory area to store public key */
} ecpEccGfp_KeyGen_Param_t;
//----------------------------------------------------------------
// ECC operation result code
//----------------------------------------------------------------
#define EC_ILLEGAL_VALUE          0xFFA5
#define EC_PROCESS_OK             0x00
#define EC_PROCESS_FAIL           0x01
#define EC_NOT_SUPPORTED          0x02

#define EC_DOMAINPARAMS_CHECK_OK   0x01
#define EC_DOMAINPARAMS_CHECK_FAIL 0x00

#define EC_KEY_CHECK_OK           0x01
#define EC_KEY_CHECK_FAIL	      0x00

#define EC_VERIFY_OK             0x01
#define EC_VERIFY_FAIL           0x00

//----------------------------------------------------------------
// ECC key type, complain with Java layer
//----------------------------------------------------------------
#define TYPE_EC_FP_PRIVATE                          12
#define TYPE_EC_FP_PRIVATE_TRANSIENT_DESELECT       31
#define TYPE_EC_FP_PRIVATE_TRANSIENT_RESET          30
#define TYPE_EC_F2M_PRIVATE                         10
#define TYPE_EC_F2M_PRIVATE_TRANSIENT_DESELECT      29
#define TYPE_EC_F2M_PRIVATE_TRANSIENT_RESET         28

#define TYPE_EC_FP_PUBLIC                           11
#define TYPE_EC_F2M_PUBLIC                           9

/*
 * the structure of area pointed by pb_domainParams is as follows
 * --------------------------------------------------------------
 * | prime P | param A | param B | 0x04, param G(x,y) | param R |
 * --------------------------------------------------------------
*/
typedef struct {
	u8 b_PLength;
	u8 b_RLength;
	PM8 pb_domainParams;

	PM8 pb_PrivateKeyS;
	PM8 pb_PublicKeyQxy;

	u8 b_HValue;
    u8 bParamStatus;
	u8 bKeyType;
} ECKeyRefStore;

extern ECKeyRefStore g_ECKeyStore;


/***********************************************************************************
 * ECC signature mode according the definition of Java
 ***********************************************************************************/
#ifndef ECC_SIGN
#define ECC_SIGN              0x01
#endif

#ifndef ECC_VERIFY
#define ECC_VERIFY            0x02
#endif

//----------------------------------------------------------------
// temp memory for ECC compution
//----------------------------------------------------------------
//#define ECKEYGEN_BASE_A(keyLen)    		 (u8 *)(FXRAM_START_ADDRESS)
//#define ECKEYGEN_BASE_B(keyLen)    		 (u8 *)(ECKEYGEN_BASE_A(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_Gx(keyLen)   	   	 (u8 *)(ECKEYGEN_BASE_B(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_Gy(keyLen)  		 (u8 *)(ECKEYGEN_BASE_Gx(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_R(keyLen)   		 (u8 *)(ECKEYGEN_BASE_Gy(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_P(keyLen)   		 (u8 *)(ECKEYGEN_BASE_R(keyLen) + ((keyLen) + 3))
//#define ECKEYGEN_BASE_D(keyLen)	  		 (u8 *)(ECKEYGEN_BASE_P(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_Qx(keyLen)  		 (u8 *)(ECKEYGEN_BASE_D(keyLen) + ((keyLen) + 3))
//#define ECKEYGEN_BASE_Qy(keyLen)  		 (u8 *)(ECKEYGEN_BASE_Qx(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_TempQx(keyLen) 	 (u8 *)(ECKEYGEN_BASE_Qy(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_TempQy(keyLen)  	 (u8 *)(ECKEYGEN_BASE_TempQx(keyLen) + ((keyLen) + 2))
////#define ECKEYGEN_BASE_TempQxy(keyLen)  	 (u8 *)(ECKEYGEN_BASE_TempQy(keyLen) + ((keyLen) + 2))
//#define ECKEYGEN_BASE_TempQxy(keyLen)  	 (u8 *)bTempArr

//----------------------------------------------------------------
// Define ECC type, complain with java layer
//----------------------------------------------------------------
#define ALG_EC_FP				0x05
#define ALG_EC_F2M				0x04

//----------------------------------------------------------------
// Define ECC Signature type, complain with java layer
//----------------------------------------------------------------
//#define ALG_ECDSA_SHA           0x11
//#define ALG_ECDSA_SHA_224       0x25
//#define ALG_ECDSA_SHA_256       0x21
//#define ALG_ECDSA_SHA_384       0x22
//#define ALG_ECDSA_SHA_512       0x26

//----------------------------------------------------------------
// Define ECC type, complain with java layer
//----------------------------------------------------------------
#define ALG_EC_FP_KEY_112			0x0E
#define ALG_EC_FP_KEY_128			0x10
#define ALG_EC_FP_KEY_160			0x14
#define ALG_EC_FP_KEY_192			0x18
#define ALG_EC_FP_KEY_224			0x1c
#define ALG_EC_FP_KEY_256			0x20
#define ALG_EC_FP_KEY_384			0x30
#if WITH_ECC_KEY_512
#define ALG_EC_FP_KEY_512			0x40 //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#endif
#if WITH_ECC_KEY_521
#define ALG_EC_FP_KEY_521			0x42
#endif

#if WITH_ECC_KEY_521 // modify by xuxiaojun@20220830: 由于nativeECCSignatureDoFinalCal函数定义局部变量时使用MAX_ECC_KEY_BYTELEN，消耗的栈空间比较大，所以用开关WITH_ECC_KEY_512和WITH_ECC_KEY_521对所支持ECC的最大长度进行区分，在产品对这两个没有需求，且RAM空间不够的情况下，可以关闭这两个开关
#define MAX_ECC_KEY_BYTELEN         ((ALG_EC_FP_KEY_521 + 0x03) & 0xFFFC) //modify by jiangxiaohui@20220525
#elif WITH_ECC_KEY_512
#define MAX_ECC_KEY_BYTELEN         ((ALG_EC_FP_KEY_512 + 0x03) & 0xFFFC) //modify by jiangxiaohui@20220525
#else
#define MAX_ECC_KEY_BYTELEN         ((ALG_EC_FP_KEY_384 + 0x03) & 0xFFFC) //modify by jiangxiaohui@20220525 
#endif
//----------------------------------------------------------------
// Define ECDH type
//----------------------------------------------------------------
#define ALG_EC_SVDP_DH					0x01
#define ALG_EC_SVDP_DH_PLAIN			0x03
#define ALG_EC_SVDP_DHC                 0x02
#define ALG_EC_SVDP_DHC_PLAIN           0x04

//----------------------------------------------------------------
// Define ECKey status
//----------------------------------------------------------------
#define DOMAIN_PARAM_UNSETED       0x00
#define FP_DOMAIN_PARAM_MASK       0x3F
#define FP_DOMAIN_PARAM_SETED      0x3E
#define DOMAIN_COFACTOR_POSITION   0x01
//#define F2M3_DOMAIN_PARAM_SETED    0x4F
//#define F2M5_DOMAIN_PARAM_SETED    0x8F
//#define F2M_DOMAIN_PARAM_MASK      0xCF
//#define F2M_DOMAIN_PARAM_SETED     0x0F

//----------------------------------------------------------------
// Define ECC API
//----------------------------------------------------------------
u16 getRealLength(u8 X_A * dataIn, u16 length);
u16 getRealLength2(u8 X_A * dataIn, u16 length);
void GenECKeyPair(void);
void ECCSign(PM8 inBuff, u16 inLen, PM8 mdBuff, PM8 tmpBuff, u8 algType,u16 preHashFlag);
boolean ECVerify(PM8 inBuff, u16 inLen, PM8 sigBuff, u16 sigLen, PM8 mdBuff, PM8 tmpBuff, u8 algType,u16 preHashFlag);
boolean CheckECDomainParams(void);
u8 CheckECPrivateKey(void);
boolean CheckECPublicKey(void);
u8 ECDiffieHellman(u8 bECDHType);

void EncodeSignature(u8 X_A * rSign, u8 X_A * rSignLen, u8 X_A * sSign, u8 X_A * sSignLen);
boolean DecodeSignature(u8 X_A * rSign, u8 X_A * rSignLen, u8 X_A * sSign, u8 X_A * sSignLen, u8 * pbSignAddr, u8 wSignLen);
void InitDefaultECDomainParams(void);
u8 DoECOpertaion(u8 bHashLen);
extern signed char ECC_compare(unsigned char * buffA, unsigned char * buffB, unsigned short size);

#endif  // WITH_ECC

#if WITH_SM2
			
typedef struct
{
	PM8 privteKeyBuffer;
	PM8 publicKeyBuffer;
}SM2KeyRefStore;


extern SM2KeyRefStore		X_A				g_SM2KeyStore;
extern  u8 	CONST SM2_256_DOMAINPARAMS[];

#define SM2_DOMAINPARAMS_POINT_P				&SM2_256_DOMAINPARAMS[0]
#define SM2_DOMAINPARAMS_POINT_A				&SM2_256_DOMAINPARAMS[SM2_DOMAIN_NLEN]
#define SM2_DOMAINPARAMS_POINT_B				&SM2_256_DOMAINPARAMS[SM2_DOMAIN_NLEN * 2]
#define SM2_DOMAINPARAMS_POINT_G				&SM2_256_DOMAINPARAMS[SM2_DOMAIN_NLEN * 3]
#define SM2_DOMAINPARAMS_POINT_GX				SM2_DOMAINPARAMS_POINT_G
#define SM2_DOMAINPARAMS_POINT_GY				&SM2_256_DOMAINPARAMS[SM2_DOMAIN_NLEN * 4]
#define SM2_DOMAINPARAMS_POINT_N				&SM2_256_DOMAINPARAMS[SM2_DOMAIN_NLEN * 5]

#define	SM2_DOMAINPARAMS_REUSEDATA_NN_LEN		0x08
#define SM2_DOMAINPARAMS_REUSEDATA_POINT_NN		&SM2_256_DOMAINPARAMS_REUSEDATA[0]
#define SM2_DOMAINPARAMS_REUSEDATA_POINT_QN		&SM2_256_DOMAINPARAMS_REUSEDATA[SM2_DOMAINPARAMS_REUSEDATA_NN_LEN]

#define GMSM2KeyExchange_SEND_MODE   1
#define GMSM2KeyExchange_RECIVE_MODE 0

#endif // WITH_SM2

#if WITH_ECC || WITH_SOFT_SM2

#define A_IS_BIGGER_THAN_B			0x01
#define A_IS_SMALLER_THAN_B			0x02
#define A_IS_EQUAL_WITH_B			0x03

#define CAL_SUCCESSED				0x00
#define POINT_NOT_IN_ELL			0x01

extern void ecpAAddB(PX8 APoint, u16 AbyteLen, PX8 BPoint,  u16 BbyteLen, PX8 CPoint);
extern void ecpAAddConst(PX8 APoint, PX8 BPoint, u16 AbyteLen, u8 constValue);
extern u8 ecpACompareB(PX8 APoint, PX8 BPoint, u16 AbyteLen);
extern void ecpAAddBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint);
extern void ecpASubBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint);
extern void ecpAMulBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint);
extern void ecpAInvMultBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint);
extern u8 ecpAMulPointXY(PM8 APoint, PM8 PointPXY, PX8 PointQXY);
extern u8 ecpPointAddPoint(PX8 PointMxy, PM8 PointNxy, PX8 PointQXY);
extern u8 ecpUniversalAMulPointXY(PM8 APoint, PM8 PointPXY, PX8 PointQXY, PX8 ECDomainParams);
extern boolean PublicKeyComputeYFromX(PX8 xPointPtr, PX8 yPointPtr, PX8 ECDomainParams);
extern u8 calcSHS(u8 *publicKey, u8 publicKeyLen, u8 keyRef, u8 *privateKey, u8 privateKeyLen, u8 *shsBuf);
extern u8 ECPointAdd(PM8 ECPointA, PM8 ECPointB, PM8 ECPointC);
extern u8 ECPointSub(PM8 ECPointA, PM8 ECPointB, PM8 ECPointC);

extern u8 X25519(u8* p_publicKey, u8* p_privateKey,u8* Basepoint);
extern u32 PointDecompress(u8 *point,u8 *x_y,u8 *argBuf);
#endif //WITH_ECC || WITH_SOFT_SM2

#endif /* ELLIPSECURVE_API_H_ */

