/**
  ******************************************************************************
  * @file    C98BHS666ASC_ALG_API_HEX.h
  * @author  Firmware Team
  * @brief   Alg Api Entry Header File
  * @date    2024-03-22
  * @version V1.0
  ******************************************************************************
  * @attention C98BHS666ASC_ALG_API_HEX.h
  *
  * <h2><center>CEC Huada Electronic Design Co.,Ltd</center></h2>
  *
  * The copyright of this manual is reserved by CEC Huada Electronic Design Co., Ltd.
  * Without prior permission, any duplication, printing or publication and distribution
  * of the manual will constitute a violation of copyright towards CEC Huada Electronic
  * Design Co., Ltd. CEC Huada Electronic Design Co., Ltd. reserves the right to bring
  * them to court. \n
  * The right to change the manual without notice to the user is reserved by CEC Huada
  * Electronic Design Co., Ltd. Though we have checked content of this manual, errors
  * are unavoidable. So, we will regularly check the content and make necessary
  * modifications in the next version. We suggest you obtain the latest version of this
  * manual from CEC Huada Electronic Design Co., Ltd. before making final design.
  *
  * @par Modify log
  * <table>
  * <tr><th>Date        <th>Version     <th>Author          <th>Description   </tr>
  * <tr><td>2021/04/08  <td>V1.0        <td>Firmware Team   <td>Create        </tr>
  * <tr><td>2021/04/08  <td>V1.1        <td>Firmware Team   <td>ModifyXXXX    </tr>
  * </table>
  ******************************************************************************
  */
/** @defgroup ALG
  * @brief ALG Header File
  * @{
  */
/** @defgroup C98BHS666ASC_ALG_API_HEX.h
  * @brief C98BHS666ASC_ALG_API_HEX.h
  * @{
  */
#ifndef C98BHS666ASC_ALG_API_HEX_H__
#define C98BHS666ASC_ALG_API_HEX_H__
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
/*
################################################################################
********************* Enumeration and structure Definition *********************
################################################################################
*/
// Symmetric algorithm
typedef struct
{
    unsigned int *iDataPtr;
    unsigned int DataLen;
    unsigned int *iKeyPtr;
    unsigned int *IVPtr;
    unsigned int *oDataPtr;
    unsigned char Type;
    unsigned int  paraChkSum;
} DES_Parameters, TDES_Parameters, AES_Parameters;

// power ctrl
typedef enum
{
    SYSCLK_5p5Mhz   =  0x0,
    SYSCLK_11Mhz    =  0x1,
    SYSCLK_16p5Mhz  =  0x2,
    SYSCLK_22Mhz    =  0x3,
    SYSCLK_27p5Mhz  =  0x4,
    SYSCLK_33Mhz    =  0x5,
    SYSCLK_38p5Mhz  =  0x6,
    SYSCLK_44Mhz    =  0x7,
    SYSCLK_49p5Mhz  =  0x8,
    SYSCLK_55Mhz    =  0x9,
    SYSCLK_60p5Mhz  =  0xA,
    SYSCLK_66Mhz    =  0xB,
    SYSCLK_71p5Mhz  =  0xC,
    SYSCLK_77Mhz    =  0xD,
    SYSCLK_82p5Mhz  =  0xE,
    SYSCLK_88Mhz    =  0xF,
} SYS_CLK_DIVISOR;

typedef enum
{
    SYSCLK_LF_3p39Mhz   =  0x0,
    SYSCLK_LF_6p78Mhz   =  0x1,
    SYSCLK_LF_10p17Mhz  =  0x2,
    SYSCLK_LF_13p56Mhz  =  0x3,
} SYS_CLK_DIVISOR_LF;


// Asymmetric algorithm-ECC
typedef struct
{
    const unsigned char *iDataPtr;
    unsigned char       *oDataPtr;
    const unsigned int  *rndPtr;
    unsigned int        *priKeyPtr;
    unsigned int        *pubKeyPtr;
    const unsigned int  *argPtr;
    unsigned int  paraChkSum;
} ECC_Parameters, SM2_Parameters;
typedef struct
{
    const unsigned int *iEccP;
    const unsigned int *iEccA;
    const unsigned int *iEccB;
    const unsigned int *iEccR;
    const unsigned int *iEccG;
    unsigned int       *oDataPtr;
    unsigned int        bitLen;
    unsigned int        hashType;
} ECC_Init_Parameters;
typedef struct
{
    unsigned int    byteLen_SharedInfo1;
    const void     *SharedInfo1;
    unsigned int    byteLen_SharedInfo2;
    const void     *SharedInfo2;
} ECIES_Parameters;

typedef struct
{
    unsigned char   *pData;
    unsigned short  __pad;
    unsigned short  dataLen;
} KDF_BigNum;
typedef struct
{
    unsigned int         byteLen;               //!< 常量数据的字节长度
    const unsigned char *data;                  //!< 常量数据的首地址
} BigNumR;

typedef enum
{
    DESKEYMsk = 4,
    TDES2KEYMsk = 5,
    AES128KEYMsk = 7,
    AES192KEYMsk = 8,
    AES256KEYMsk = 9,
} SymmetricKeyMask;
/******************************************************************************/
/*
################################################################################
************************ Function Prototype Declaration ************************
################################################################################
*/
unsigned int HED_ALGAPI_Version(void);
// Self Check
unsigned int HED_AlgSelfCheck(unsigned int AlgChkBit);
// Key Mask
unsigned int HED_SymmetricKeyMask(unsigned int *iKeyMaskPtr, SymmetricKeyMask KeyMask);
// Symmetric algorithm
unsigned int HED_DESBlock(DES_Parameters *DES_Param);
unsigned int HED_TDESBlock_2Key(TDES_Parameters *TDES_Param);
unsigned int HED_AESBlock(AES_Parameters *AES_Param);
// power ctrl
void HED_ChangeSYSFreq(SYS_CLK_DIVISOR SysDivLmt,
                       SYS_CLK_DIVISOR SysDivLowLmt,
                       SYS_CLK_DIVISOR_LF SysDivLmt_lf,
                       SYS_CLK_DIVISOR_LF SysDivLowLmt_lf);
void PreSet_SYSCLK_ICS(unsigned int PMStrategyIndex, unsigned int AlgTypeIndex, unsigned int *bakSFRBuf);
void PostSet_SYSCLK_ICS(unsigned int PMStrategyIndex, unsigned int *bakSFRBuf);
void PreSet_SYSCLK(unsigned int *bakSFRBuf);
void PostSet_SYSCLK(unsigned int *bakSFRBuf);
void HED_OpenICS(void);
void HED_CloseICS(void);
void HED_ChangeICSSet(unsigned char tgtICSSet);
void HED_ConfigICSLMT(unsigned int tgtICSLMT);
// Hash algorithm
void HED_SHA1_Init(unsigned int *V);
unsigned int HED_SHA1_Block(unsigned int *V, const unsigned int *DataValue, unsigned int DataLen);
void HED_SHA224_Init(unsigned int *V);
unsigned int HED_SHA224_Block(unsigned int *V, const unsigned int *DataValue, unsigned int DataLen);
void HED_SHA256_Init(unsigned int *V);
unsigned int HED_SHA256_Block(unsigned int *V, const unsigned int *DataValue, unsigned int DataLen);
void HED_SHA384_Init(unsigned int *V);
unsigned int HED_SHA384_Block(unsigned int *V, const unsigned int *DataValue, unsigned int DataLen);
void HED_SHA512_Init(unsigned int *V);
unsigned int HED_SHA512_Block(unsigned int *V, const unsigned int *DataValue, unsigned int DataLen);
// Asymmetric algorithm-ECC
unsigned int HED_ECCCurveCheck(ECC_Init_Parameters *EccInitPara);
unsigned int HED_ECCInitPara(ECC_Init_Parameters *EccInitPara);
unsigned int HED_ECCIsLegalPriKey(ECC_Parameters *ECC_Param);
unsigned int HED_ECCIsLegalPubKey(ECC_Parameters *ECC_Param);
unsigned int HED_ECCGenKey(ECC_Parameters *ECC_Param);
unsigned int HED_ECCGenKey2(ECC_Parameters *ECC_Param);
unsigned int HED_ECCSign(ECC_Parameters *ECC_Param);
unsigned int HED_ECCVerify(ECC_Parameters *ECC_Param);
unsigned int HED_ECCEncrypt(ECC_Parameters *ECC_Param, unsigned int iLen);
unsigned int HED_ECCDecrypt(ECC_Parameters *ECC_Param, unsigned int iLen);
unsigned int HED_ECCEncrypt_SEC(ECC_Parameters *ECC_Param, ECIES_Parameters *IES_Param, unsigned int iLen);
unsigned int HED_ECCDecrypt_SEC(ECC_Parameters *ECC_Param, ECIES_Parameters *IES_Param, unsigned int iLen);
unsigned int HED_ECKA_BSI(ECC_Parameters *ECC_Param);
unsigned int HED_ECCSSign(ECC_Parameters *ECC_Param, unsigned int iLen);
unsigned int HED_ECCSVerify(ECC_Parameters *ECC_Param, unsigned int iLen);
unsigned int HED_ECCBdh(ECC_Parameters* ECC_Param );
void HED_KDF_X963(const KDF_BigNum *zAB, const KDF_BigNum *r, KDF_BigNum *key, unsigned int hashType);
void HED_ECCPointCompress(unsigned char *x_y, const unsigned int *point, unsigned int bitLen);
unsigned int HED_ECCPointDecompress(unsigned int *point, const unsigned char *x_y, const unsigned int *argPtr);
// Asymmetric algorithm-X25519
typedef struct
{
    const unsigned char *priKeyPtr;             //!<[in]       己方私钥
    const unsigned char *pubKeyPtr;             //!<[in]       对方公钥
    unsigned char       *sharedKey;             //!<[out]      共享密钥
    unsigned int         paraChkSum;            //!<[out]      安全函数输出运算参数校验值
} X25519_Parameters;
unsigned int HED_X25519Sec(X25519_Parameters *X25519_Para);

/******************************************************************************/
#define ALG_API_ENTRY             (0x1FF80000)
#define ENTRY_STEP_SIZE           (0x08)
#define ALGAPI_Version_Addr       (ALG_API_ENTRY + ENTRY_STEP_SIZE * 1)
// Self Check
#define AlgSelfCheck_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 2)
// Key Mask
#define SymmetricKeyMask_Addr     (ALG_API_ENTRY + ENTRY_STEP_SIZE * 3)
// Symmetric algorithm
#define DESBlock_Addr             (ALG_API_ENTRY + ENTRY_STEP_SIZE * 4)
#define TDESBlock_2Key_Addr       (ALG_API_ENTRY + ENTRY_STEP_SIZE * 5)
#define AESBlock_Addr             (ALG_API_ENTRY + ENTRY_STEP_SIZE * 6)
// power ctrl
#define ChangeSYSFreq_Addr        (ALG_API_ENTRY + ENTRY_STEP_SIZE * 7)
#define PreSet_SYSCLK_ICS_Addr    (ALG_API_ENTRY + ENTRY_STEP_SIZE * 8)
#define PostSet_SYSCLK_ICS_Addr   (ALG_API_ENTRY + ENTRY_STEP_SIZE * 9)
#define PreSet_SYSCLK_Addr        (ALG_API_ENTRY + ENTRY_STEP_SIZE * 10)
#define PostSet_SYSCLK_Addr       (ALG_API_ENTRY + ENTRY_STEP_SIZE * 11)
#define OpenICS_Addr              (ALG_API_ENTRY + ENTRY_STEP_SIZE * 12)
#define CloseICS_Addr             (ALG_API_ENTRY + ENTRY_STEP_SIZE * 13)
#define ChangeICSSet_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 14)
#define ConfigICSLMT_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 15)
// Hash algorithm
#define SHA1_Init_Addr            (ALG_API_ENTRY + ENTRY_STEP_SIZE * 16)
#define SHA1_Block_Addr           (ALG_API_ENTRY + ENTRY_STEP_SIZE * 17)
#define SHA224_Init_Addr          (ALG_API_ENTRY + ENTRY_STEP_SIZE * 18)
#define SHA224_Block_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 19)
#define SHA256_Init_Addr          (ALG_API_ENTRY + ENTRY_STEP_SIZE * 20)
#define SHA256_Block_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 21)
#define SHA384_Init_Addr          (ALG_API_ENTRY + ENTRY_STEP_SIZE * 22)
#define SHA384_Block_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 23)
#define SHA512_Init_Addr          (ALG_API_ENTRY + ENTRY_STEP_SIZE * 24)
#define SHA512_Block_Addr         (ALG_API_ENTRY + ENTRY_STEP_SIZE * 25)
// Asymmetric algorithm-ECC
#define ECCCurveCheck_Addr        (ALG_API_ENTRY + ENTRY_STEP_SIZE * 26)
#define ECCInitPara_Addr          (ALG_API_ENTRY + ENTRY_STEP_SIZE * 27)
#define ECCIsLegalPriKey_Addr     (ALG_API_ENTRY + ENTRY_STEP_SIZE * 28)
#define ECCIsLegalPubKey_Addr     (ALG_API_ENTRY + ENTRY_STEP_SIZE * 29)
#define ECCGenKey_Addr            (ALG_API_ENTRY + ENTRY_STEP_SIZE * 30)
#define ECCGenKey2_Addr           (ALG_API_ENTRY + ENTRY_STEP_SIZE * 31)
#define ECCSign_Addr              (ALG_API_ENTRY + ENTRY_STEP_SIZE * 32)
#define ECCVerify_Addr            (ALG_API_ENTRY + ENTRY_STEP_SIZE * 33)
#define ECCEncrypt_Addr           (ALG_API_ENTRY + ENTRY_STEP_SIZE * 34)
#define ECCDecrypt_Addr           (ALG_API_ENTRY + ENTRY_STEP_SIZE * 35)
#define ECCEncrypt_SEC_Addr       (ALG_API_ENTRY + ENTRY_STEP_SIZE * 36)
#define ECCDecrypt_SEC_Addr       (ALG_API_ENTRY + ENTRY_STEP_SIZE * 37)
#define ECKA_BSI_Addr             (ALG_API_ENTRY + ENTRY_STEP_SIZE * 38)
#define ECCSSign_Addr             (ALG_API_ENTRY + ENTRY_STEP_SIZE * 39)
#define ECCSVerify_Addr           (ALG_API_ENTRY + ENTRY_STEP_SIZE * 40)
#define ECCBdh_Addr               (ALG_API_ENTRY + ENTRY_STEP_SIZE * 41)
#define KDF_X963_Addr             (ALG_API_ENTRY + ENTRY_STEP_SIZE * 42)
#define ECCPointCompress_Addr     (ALG_API_ENTRY + ENTRY_STEP_SIZE * 43)
#define ECCPointDecompress_Addr   (ALG_API_ENTRY + ENTRY_STEP_SIZE * 44)
// Asymmetric algorithm-X25519
#define X25519_Addr               (ALG_API_ENTRY + ENTRY_STEP_SIZE * 45)
/******************************************************************************/
#define HED_ALGAPI_Version() (*((unsigned int (*)(void))ALGAPI_Version_Addr))()
// Self Check
#define HED_AlgSelfCheck(AlgChkBit) (*((unsigned int (*)(unsigned int))AlgSelfCheck_Addr))(AlgChkBit)
// Key Mask
#define HED_SymmetricKeyMask(iKeyMaskPtr, KeyMask) (*((unsigned int (*)(unsigned int *, SymmetricKeyMask))SymmetricKeyMask_Addr))(iKeyMaskPtr, KeyMask)
// Symmetric algorithm
#define HED_DESBlock(DES_Param) (*((unsigned int (*)(DES_Parameters *))DESBlock_Addr))(DES_Param)
#define HED_TDESBlock_2Key(TDES_Param) (*((unsigned int (*)(TDES_Parameters *))TDESBlock_2Key_Addr))(TDES_Param)
#define HED_AESBlock(AES_Param) (*((unsigned int (*)(AES_Parameters *))AESBlock_Addr))(AES_Param)
// power ctrl
#define HED_ChangeSYSFreq(SysDivLmt, SysDivLowLmt, SysDivLmt_lf, SysDivLowLmt_lf) (*((void (*)(SYS_CLK_DIVISOR, SYS_CLK_DIVISOR, SYS_CLK_DIVISOR_LF, SYS_CLK_DIVISOR_LF))ChangeSYSFreq_Addr))(SysDivLmt, SysDivLowLmt, SysDivLmt_lf, SysDivLowLmt_lf)
#define PreSet_SYSCLK_ICS(PMStrategyIndex, AlgTypeIndex, bakSFRBuf) (*((void (*)(unsigned int, unsigned int, unsigned int *))PreSet_SYSCLK_ICS_Addr))(PMStrategyIndex, AlgTypeIndex, bakSFRBuf)
#define PostSet_SYSCLK_ICS(PMStrategyIndex, bakSFRBuf) (*((void (*)(unsigned int, unsigned int *))PostSet_SYSCLK_ICS_Addr))(PMStrategyIndex, bakSFRBuf)
#define PreSet_SYSCLK(bakSFRBuf) (*((void (*)(unsigned int *))PreSet_SYSCLK_Addr))(bakSFRBuf)
#define PostSet_SYSCLK(bakSFRBuf) (*((void (*)(unsigned int *))PostSet_SYSCLK_Addr))(bakSFRBuf)
#define HED_OpenICS() (*((void (*)(void))OpenICS_Addr))()
#define HED_CloseICS() (*((void (*)(void))CloseICS_Addr))()
#define HED_ChangeICSSet(tgtICSSet) (*((void (*)(unsigned char))ChangeICSSet_Addr))(tgtICSSet)
#define HED_ConfigICSLMT(tgtICSLMT) (*((void (*)(unsigned int))ConfigICSLMT_Addr))(tgtICSLMT)
// Asymmetric algorithm-HASH
#define HED_SHA1_Init(V) (*((void (*)(unsigned int *))SHA1_Init_Addr))(V)
#define HED_SHA1_Block(V, DataValue, DataLen) (*((unsigned int (*)(unsigned int *, const unsigned int *, unsigned int))SHA1_Block_Addr))(V, DataValue, DataLen)
#define HED_SHA224_Init(V) (*((void (*)(unsigned int *))SHA224_Init_Addr))(V)
#define HED_SHA224_Block(V, DataValue, DataLen) (*((unsigned int (*)(unsigned int *, const unsigned int *, unsigned int))SHA224_Block_Addr))(V, DataValue, DataLen)
#define HED_SHA256_Init(V) (*((void (*)(unsigned int *))SHA256_Init_Addr))(V)
#define HED_SHA256_Block(V, DataValue, DataLen) (*((unsigned int (*)(unsigned int *, const unsigned int *, unsigned int))SHA256_Block_Addr))(V, DataValue, DataLen)
#define HED_SHA384_Init(V) (*((void (*)(unsigned int *))SHA384_Init_Addr))(V)
#define HED_SHA384_Block(V, DataValue, DataLen) (*((unsigned int (*)(unsigned int *, const unsigned int *, unsigned int))SHA384_Block_Addr))(V, DataValue, DataLen)
#define HED_SHA512_Init(V) (*((void (*)(unsigned int *))SHA512_Init_Addr))(V)
#define HED_SHA512_Block(V, DataValue, DataLen) (*((unsigned int (*)(unsigned int *, const unsigned int *, unsigned int))SHA512_Block_Addr))(V, DataValue, DataLen)
// Asymmetric algorithm-ECC
#define HED_ECCCurveCheck(EccInitPara) (*((unsigned int (*)(ECC_Init_Parameters *))ECCCurveCheck_Addr))(EccInitPara)
#define HED_ECCInitPara(EccInitPara) (*((unsigned int (*)(ECC_Init_Parameters *))ECCInitPara_Addr))(EccInitPara)
#define HED_ECCIsLegalPriKey(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCIsLegalPriKey_Addr))(ECC_Param)
#define HED_ECCIsLegalPubKey(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCIsLegalPubKey_Addr))(ECC_Param)
#define HED_ECCGenKey(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCGenKey_Addr))(ECC_Param)
#define HED_ECCGenKey2(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCGenKey2_Addr))(ECC_Param)
#define HED_ECCSign(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCSign_Addr))(ECC_Param)
#define HED_ECCVerify(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCVerify_Addr))(ECC_Param)
#define HED_ECCEncrypt(ECC_Param, iLen) (*((unsigned int (*)(ECC_Parameters *, unsigned int))ECCEncrypt_Addr))(ECC_Param, iLen)
#define HED_ECCDecrypt(ECC_Param, iLen) (*((unsigned int (*)(ECC_Parameters *, unsigned int))ECCDecrypt_Addr))(ECC_Param, iLen)
#define HED_ECCEncrypt_SEC(ECC_Param, IES_Param, iLen) (*((unsigned int (*)(ECC_Parameters *, ECIES_Parameters *, unsigned int))ECCEncrypt_SEC_Addr))(ECC_Param, IES_Param, iLen)
#define HED_ECCDecrypt_SEC(ECC_Param, IES_Param, iLen) (*((unsigned int (*)(ECC_Parameters *, ECIES_Parameters *, unsigned int))ECCDecrypt_SEC_Addr))(ECC_Param, IES_Param, iLen)
#define HED_ECKA_BSI(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECKA_BSI_Addr))(ECC_Param)
#define HED_ECCSSign(ECC_Param, iLen) (*((unsigned int (*)(ECC_Parameters *, unsigned int))ECCSSign_Addr))(ECC_Param, iLen)
#define HED_ECCSVerify(ECC_Param, iLen) (*((unsigned int (*)(ECC_Parameters *, unsigned int))ECCSVerify_Addr))(ECC_Param, iLen)
#define HED_ECCBdh(ECC_Param) (*((unsigned int (*)(ECC_Parameters *))ECCBdh_Addr))(ECC_Param)
#define HED_KDF_X963(zAB, r, key, hashType) (*((void (*)(const KDF_BigNum *, const KDF_BigNum *, KDF_BigNum *, unsigned int))KDF_X963_Addr))(zAB, r, key, hashType)
#define HED_ECCPointCompress(x_y, point, bitLen) (*((void (*)(unsigned char *, const unsigned int *,unsigned int))ECCPointCompress_Addr))(x_y, point, bitLen)
#define HED_ECCPointDecompress(point, x_y, argPtr) (*((unsigned int (*)(unsigned int *, const unsigned char *, const unsigned int *))ECCPointDecompress_Addr))(point, x_y, argPtr)
// Asymmetric algorithm-X25519
#define HED_X25519Sec(X25519_Para) (*((unsigned int (*)(X25519_Parameters *))X25519_Addr))(X25519_Para)
/******************************************************************************/
#endif/* C98BHS666ASC_ALG_API_HEX_H__ */
/**
  * @}
  */
/**
  * @}
  */

