/*****************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: APDU_xxx.c
 Author:
 Version:
 Date:
 Description:
 Function List:
 History:

******************************************************/
#include  "declare.h"

#define     ECC_SUCCESS                           0x80A5
#define     ECC_CURVE_INVALID                     0x91B7
#define     ECC_ERROR                             0x92C9
#define     ECC_RANDOM_FAILURE                    0x3

#define const_bitLen    256
#define const_wlen      (uint32_t)((const_bitLen + 31) / 32)

uint32_t HedEccPara_nistP256R1[const_wlen * 14 + 8] = {0x00};

//ecc nist p256
const uint32_t const_p[] =
{
    0xFFFFFFFF, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
const uint32_t const_n[] =
{
    0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xBCE6FAAD, 0xA7179E84, 0xF3B9CAC2, 0xFC632551,
};
const uint32_t const_a[] =
{
    0xFFFFFFFF, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFC,
};
const uint32_t const_b[] =
{
    0x5AC635D8, 0xAA3A93E7, 0xB3EBBD55, 0x769886BC, 0x651D06B0, 0xCC53B0F6, 0x3BCE3C3E, 0x27D2604B,
};
const uint32_t const_G[] =
{
    0x6B17D1F2, 0xE12C4247, 0xF8BCE6E5, 0x63A440F2, 0x77037D81, 0x2DEB33A0, 0xF4A13945, 0xD898C296,
    0x4FE342E2, 0xFE1A7F9B, 0x8EE7EB4A, 0x7C0F9E16, 0x2BCE3357, 0x6B315ECE, 0xCBB64068, 0x37BF51F5,
};

/*************************************************
  Function:      cmd_ECC_Init_ALG
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
void ECC_Init_ALG(void)
{
    uint32_t t_retValue;
    ECC_Init_Parameters tmpPara;
    //uint32_t DataBuf[const_wlen*14+8];

    tmpPara.iEccP = const_p ;
    tmpPara.iEccA = const_a ;
    tmpPara.iEccB = const_b ;
    tmpPara.iEccR = const_n ;
    tmpPara.iEccG = const_G ;
    tmpPara.bitLen = const_bitLen;
    tmpPara.hashType = 0x03;
    tmpPara.oDataPtr = HedEccPara_nistP256R1;
    t_retValue = HED_ECCInitPara(&tmpPara);

    if(t_retValue != ECC_SUCCESS)
    {
        SW1SW2 = 0x6F01;
    }

    /*the output's data format is the same as lib "HED32_ECC256_nistP256.lib",
    which called "HedEccPara_nistP256R1[]" in "HED32_ECC256_nistP256.h" */
}


/* bit = 256 */
/* priKey */
const uint32_t ecc_genkeyPrikey[] =
{
    0x33CA766E, 0x2067E0ED, 0x33ABF4CF, 0x1C820523, 0x89E06712, 0x81DE52DD, 0x8112470F, 0xA38552BE,
};
/* pubKey */
const uint32_t ecc_genkeyPubKey[] =
{
    0xDEC8800A, 0xB46DDF27, 0x41C28F43, 0xCC1D9043, 0xFB4C1C48, 0xBF46AD09, 0xAC09B492, 0x73B76A56,
    0x1FFBD087, 0x65A3C574, 0x3447FBF5, 0x989CF170, 0xACC75267, 0x9F8AC3B3, 0xDDF165BA, 0xD0AA931D,
};
const uint32_t *const ecc_genkey_Para = HedEccPara_nistP256R1;
/*************************************************
  Function:      cmd_ECC_GenKey_ALG
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
void ECC_GenKey_ALG(void)
{
    uint32_t wlen;
    uint32_t priKey[const_wlen];
    uint32_t pubKey[const_wlen * 2];
    uint32_t t_retValue;

    ECC_Parameters tmpPara;
    tmpPara.iDataPtr = NULL;
    tmpPara.oDataPtr = NULL;
    tmpPara.rndPtr = ecc_genkeyPrikey;
    tmpPara.priKeyPtr = priKey;
    tmpPara.pubKeyPtr = pubKey;
    tmpPara.argPtr = ecc_genkey_Para;

    t_retValue = HED_ECCGenKey2(&tmpPara);

    if(ECC_SUCCESS != t_retValue)
    {
        SW1SW2 = 0x6F02;
        return;
    }

    wlen = const_wlen;

    if(memcmp(ecc_genkeyPubKey, pubKey, wlen * 4) != 0)
    {
        SW1SW2 = 0x6F03;/* genkey failure */
        return;
    }

    t_retValue = HED_ECCGenKey(&tmpPara);

    if(ECC_SUCCESS != t_retValue)
    {
        SW1SW2 = 0x6F04;
        return;
    }
}

const uint32_t ecc_signKey[] =
{
    0x2CA1411A, 0x41B17B24, 0xCC8C3B08, 0x9CFD033F, 0x1920202A, 0x6C0DE8AB, 0xB97DF149, 0x8D50D2C8,
    0x596375E6, 0xCE57E0F2, 0x0294FC46, 0xBDFCFD19, 0xA39F8161, 0xB58695B3, 0xEC5B3D16, 0x427C274D,
    0x42754DFD, 0x25C56F93, 0x9A79F2B2, 0x04876B3A, 0x3AB1CEB2, 0xE4FF571A, 0xBF4FBF36, 0x326C8B27,
};
const char *const ecc_signMess = "Example of ECDSA with ansip256r1 and SHA-256";
const uint32_t ecc_signE[] =
{
    0x1BD4ED43, 0x0B0F384B, 0x4E8D458E, 0xFF1A8A55, 0x3286D7AC, 0x21CB2F68, 0x06172EF5, 0xF94A06AD,
};
const uint32_t ecc_signK[] =
{
    0xA0640D49, 0x57F27D09, 0x1AB1AEBC, 0x69949D96, 0xE5AC2BB2, 0x83ED5284, 0xA5674758, 0xB12F08DF,
};
const uint32_t *const  ecc_sign_Para = HedEccPara_nistP256R1;
#define HashBlockByteSize   64
/*************************************************
  Function:      cmd_ECC_SignVerify_ALG
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
void ECC_SignVerify_ALG(void)
{
    uint32_t i, j;
    uint32_t wlen;
    ECC_Parameters tmpPara;
    uint32_t t_retValue;
    uint32_t  paraChkSum;


    uint32_t iDataBuf[sizeof(ecc_signE) + 4];
    uint32_t oDataBuf[const_wlen * 2];
    uint8_t *pDest, *pSrc;

    tmpPara.iDataPtr = (const uint8_t *)ecc_signE;
    tmpPara.oDataPtr = (uint8_t *)oDataBuf;
    tmpPara.rndPtr = ecc_signK;
    tmpPara.priKeyPtr = (uint32_t *)ecc_signKey;

    tmpPara.argPtr = ecc_sign_Para ;
    wlen = const_wlen;

    tmpPara.pubKeyPtr = (uint32_t *)(ecc_signKey + wlen);

    memset(iDataBuf, '\0', sizeof(ecc_signE));
    i = strlen(ecc_signMess);
    j = i / HashBlockByteSize;
    pSrc = (void *)ecc_signMess;
    pDest = (void *)oDataBuf;

    HED_SHA256_Init(iDataBuf);

    while(j-- > 0)
    {
        memcpy(pDest, pSrc, HashBlockByteSize);
        HED_SHA256_Block(iDataBuf, oDataBuf, HashBlockByteSize);
        pSrc += HashBlockByteSize;
    }

    j = (HashBlockByteSize - 1)&i;

    memcpy(pDest, pSrc, j);
    HED_SHA256_Block(iDataBuf, oDataBuf, j);

    if(memcmp((void *)iDataBuf, (void *)ecc_signE, sizeof(ecc_signE)) != 0)
    {
        while(1);  /* sha error */
    }


    paraChkSum = (uint32_t)iDataBuf
                 + (uint32_t)oDataBuf
                 + (uint32_t)ecc_signKey;
    tmpPara.iDataPtr = (const uint8_t *)iDataBuf;
    t_retValue = HED_ECCSign(&tmpPara);

    if(ECC_SUCCESS != t_retValue || tmpPara.paraChkSum != paraChkSum)
    {
        SW1SW2 = 0x6F05;
        return;
    }

    if(ECC_SUCCESS != HED_ECCVerify(&tmpPara))
    {
        SW1SW2 = 0x6F07;
        return;/* verify failure */
    }
}

const uint32_t ecc_encKey[] =
{
    0x8ADA4BAB, 0xD0858016, 0xAF68A176, 0xFBCAF551, 0xD80C8044, 0x6829EC96, 0xC71C9949, 0xFEDA67DE,

    0xA18D6E27, 0xCBA8822D, 0x4D6DBDBF, 0xA4EE53E3, 0x6E03619A, 0xFF932997, 0xD1DA587E, 0x27E27093,
    0xF8607EA8, 0xD50ADE6E, 0xE0BCC866, 0x443BA51F, 0x0D30EE6F, 0x6D070D5E, 0x734F6FD1, 0x7A7CCC6D,
};

const uint32_t ecc_encK[] =
{
    0x3B220892, 0x3B94E00E, 0xBED58FB7, 0x2E4BC60A, 0xF4F66C27, 0x83F15E06, 0x90358778, 0x52C9E4C4
};

const uint8_t ecc_encMess[] =
{
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00
};
const uint8_t ecc_encRes[] =
{
    //R
    0x33, 0xB6, 0xFA, 0x78, 0x59, 0xAC, 0xC3, 0x43, 0x86, 0xC3, 0x47, 0xF5, 0x5B, 0x28, 0x7A, 0x41,
    0xFB, 0x49, 0x59, 0x97, 0xC1, 0x0C, 0xD1, 0xB4, 0x74, 0xDD, 0xB0, 0xDF, 0xE5, 0x3E, 0xE1, 0x38,

    0x3C, 0x4A, 0xAF, 0x54, 0x64, 0xCB, 0x82, 0x0D, 0x40, 0x3E, 0x93, 0xFB, 0x08, 0x58, 0x12, 0x0A,
    0xDE, 0x48, 0xFC, 0x80, 0x13, 0xBF, 0xE1, 0x9C, 0xEB, 0x9A, 0x11, 0x45, 0x6E, 0x6E, 0x86, 0x88,
    //T
    0x2E, 0xEB, 0xFD, 0x1F, 0xF6, 0xE8, 0x98, 0xF3, 0x91, 0x61, 0xB4, 0x68, 0x22, 0x93, 0x71, 0xE8,
    //X
    0x17, 0xAA, 0x3D, 0x72, 0x43, 0xD5, 0x94, 0x10, 0x00, 0x2C, 0x6C, 0xC7, 0xFD, 0xBA, 0x07, 0x83

};
const uint32_t *const  ecc_enc_Para = HedEccPara_nistP256R1;

#define MAX_ODATA_LEN                 sizeof(ecc_encRes)
/*************************************************
  Function:      ECC_EncDec_ALG
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
void ECC_EncDec_ALG(void)
{
    uint32_t t_retValue;
    uint32_t wlen;
    ECC_Parameters tmpPara;
    uint8_t OdataBuf[MAX_ODATA_LEN];
    uint32_t  paraChkSum;

    tmpPara.argPtr = ecc_enc_Para;
    //bitLen = const_bitLen;        /* get bitLen */
    wlen = const_wlen;

    tmpPara.iDataPtr = ecc_encMess;
    tmpPara.oDataPtr = OdataBuf;
    tmpPara.rndPtr = ecc_encK;
    tmpPara.priKeyPtr = (uint32_t *)ecc_encKey;
    tmpPara.pubKeyPtr = (uint32_t *)(ecc_encKey + wlen);

    if(ECC_SUCCESS != HED_ECCIsLegalPubKey(&tmpPara))  /* check public key */
    {
        SW1SW2 = 0x6F08;
        return;
    }

    t_retValue = HED_ECCEncrypt(&tmpPara, sizeof(ecc_encMess));

    if(ECC_SUCCESS != t_retValue)
    {
        SW1SW2 = 0x6F09;
        return;
    }

    //dec
    tmpPara.iDataPtr = ecc_encRes;
    tmpPara.oDataPtr = OdataBuf;
    tmpPara.rndPtr = NULL;
    tmpPara.priKeyPtr = (uint32_t *)ecc_encKey;
    tmpPara.argPtr = ecc_enc_Para;
    tmpPara.pubKeyPtr = (uint32_t *)(ecc_encKey + wlen);

    if(ECC_SUCCESS != HED_ECCIsLegalPriKey(&tmpPara))
    {
        SW1SW2 = 0x6F0B;
        return;                           /* failure */
    }

    paraChkSum = (uint32_t)ecc_encRes
                 + (uint32_t)OdataBuf
                 + (uint32_t)ecc_encKey;
    t_retValue = HED_ECCDecrypt(&tmpPara, sizeof(ecc_encRes));

    if(ECC_SUCCESS != t_retValue || tmpPara.paraChkSum != paraChkSum)
    {
        SW1SW2 = 0x6F0C;
        return;
    }

    if(memcmp(ecc_encMess, OdataBuf, sizeof(ecc_encMess)) != 0)
    {
        SW1SW2 = 0x6F0D;
        return;                           /* failure */
    }

    return;
}

/**
 * @defgroup x25519_data X25519Êý¾Ý
 * @{ */

#ifdef  USING_BIG_ENDIAN

// Alice's public key
const unsigned char fp[32] =
{
    0x6a, 0x4e, 0x9b, 0xaa, 0x8e, 0xa9, 0xa4, 0xeb, 0xf4, 0x1a, 0x38, 0x26, 0x0d, 0x3a, 0xbf, 0x0d,
    0x5a, 0xf7, 0x3e, 0xb4, 0xdc, 0x7d, 0x8b, 0x74, 0x54, 0xa7, 0x30, 0x89, 0x09, 0xf0, 0x20, 0x85,
};

// Alice's private key
const unsigned char f[32] =
{

    0x2a, 0x2c, 0xb9, 0x1d, 0xa5, 0xfb, 0x77, 0xb1, 0x2a, 0x99, 0xc0, 0xeb, 0x87, 0x2f, 0x4c, 0xdf,
    0x45, 0x66, 0xb2, 0x51, 0x72, 0xc1, 0x16, 0x3c, 0x7d, 0xa5, 0x18, 0x73, 0x0a, 0x6d, 0x07, 0x77,
};
// Bob's public key
const unsigned char gp[32] =
{
    0x4f, 0x2b, 0x88, 0x6f, 0x14, 0x7e, 0xfc, 0xad, 0x4d, 0x67, 0x78, 0x5b, 0xc8, 0x43, 0x83, 0x3f,
    0x37, 0x35, 0xe4, 0xec, 0xc2, 0x61, 0x5b, 0xd3, 0xb4, 0xc1, 0x7d, 0x7b, 0x7d, 0xdb, 0x9e, 0xde,
};

// Bob's private key
const unsigned char g[32] =
{
    0xeb, 0xe0, 0x88, 0xff, 0x27, 0x8b, 0x2f, 0x1c, 0xfd, 0xb6, 0x18, 0x26, 0x29, 0xb1, 0x3b, 0x6f,
    0xe6, 0x0e, 0x80, 0x83, 0x8b, 0x7f, 0xe1, 0x79, 0x4b, 0x8a, 0x4a, 0x62, 0x7e, 0x08, 0xab, 0x5d,
};

// Expected shared secret
const unsigned char k[32] =
{
    0x42, 0x17, 0x16, 0x1e, 0x3c, 0x9b, 0xf0, 0x76, 0x33, 0x9e, 0xd1, 0x47, 0xc9, 0x21, 0x7e, 0xe0,
    0x25, 0x0f, 0x35, 0x80, 0xf4, 0x3b, 0x8e, 0x72, 0xe1, 0x2d, 0xce, 0xa4, 0x5b, 0x9d, 0x5d, 0x4a,
};

#else      /* -----  not USING_BIG_ENDIAN  ----- */
// Alice's public key
const unsigned char fp[32] = {0x85, 0x20, 0xf0, 0x09, 0x89, 0x30, 0xa7, 0x54,
                              0x74, 0x8b, 0x7d, 0xdc, 0xb4, 0x3e, 0xf7, 0x5a,
                              0x0d, 0xbf, 0x3a, 0x0d, 0x26, 0x38, 0x1a, 0xf4,
                              0xeb, 0xa4, 0xa9, 0x8e, 0xaa, 0x9b, 0x4e, 0x6a
                             };

// Alice's private key
const unsigned char f[32] = {0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
                             0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
                             0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
                             0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a
                            };
// Bob's public key
const unsigned char gp[32] = {0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4,
                              0xd3, 0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37,
                              0x3f, 0x83, 0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d,
                              0xad, 0xfc, 0x7e, 0x14, 0x6f, 0x88, 0x2b, 0x4f
                             };

// Bob's private key
const unsigned char g[32] = {0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
                             0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
                             0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
                             0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb
                            };

// Expected shared secret
const unsigned char k[32] = {0x4a, 0x5d, 0x9d, 0x5b, 0xa4, 0xce, 0x2d, 0xe1,
                             0x72, 0x8e, 0x3b, 0xf4, 0x80, 0x35, 0x0f, 0x25,
                             0xe0, 0x7e, 0x21, 0xc9, 0x47, 0xd1, 0x9e, 0x33,
                             0x76, 0xf0, 0x9b, 0x3c, 0x1e, 0x16, 0x17, 0x42
                            };

#endif     /* -----  not USING_BIG_ENDIAN  ----- */

/*************************************************
  Function:      ECC_EncDec_ALG
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
void X25519_ALG(void)
{
#define X25519_SUCCESS 0x80A5
    uint32_t st;
    uint8_t tmp[32] = {0};
	X25519_Parameters x25519_para;
    x25519_para.sharedKey = &tmp[0];

	x25519_para.pubKeyPtr = NULL;
    x25519_para.priKeyPtr = f;
    st = HED_X25519Sec(&x25519_para);

    if(st != X25519_SUCCESS)
    {
        SW1SW2 = 0x6F0E;
        return;
    }                               /* -----  end if  ----- */

    if(memcmp(x25519_para.sharedKey, fp, 32))
    {
        SW1SW2 = 0x6F0F;
        return;
    }                               /* -----  end if  ----- */

	x25519_para.pubKeyPtr = NULL;
    x25519_para.priKeyPtr = g;
    st = HED_X25519Sec(&x25519_para);

    if(st != X25519_SUCCESS)
    {
        SW1SW2 = 0x6F10;
        return;
    }                               /* -----  end if  ----- */

    if(memcmp(x25519_para.sharedKey, gp, 32))
    {
        SW1SW2 = 0x6F11;
        return;
    }                               /* -----  end if  ----- */

    x25519_para.priKeyPtr = f;
    x25519_para.pubKeyPtr = gp;
    st = HED_X25519Sec(&x25519_para);

    if(st != X25519_SUCCESS)
    {
        SW1SW2 = 0x6F14;
        return;
    }                               /* -----  end if  ----- */

    if(memcmp(x25519_para.sharedKey, k, 32))
    {
        SW1SW2 = 0x6F15;
        return;
    }                               /* -----  end if  ----- */

    x25519_para.priKeyPtr = g;
    x25519_para.pubKeyPtr = fp;
    st = HED_X25519Sec(&x25519_para);

    if(st != X25519_SUCCESS)
    {
        SW1SW2 = 0x6F16;
        return;
    }                               /* -----  end if  ----- */

    if(memcmp(x25519_para.sharedKey, k, 32))
    {
        SW1SW2 = 0x6F17;
        return;
    }                               /* -----  end if  ----- */

    return;
}

/*************************************************
  Function:      cmd_ECC_ALG
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
void cmd_ECC_ALG(void)
{
    SW1SW2 = 0x9000;
    //user should init first,and could save the EccPara to the flash.
    ECC_Init_ALG();

    if(SW1SW2 != 0x9000)
    {
        return;
    }

    ECC_GenKey_ALG();

    if(SW1SW2 != 0x9000)
    {
        return;
    }

    ECC_SignVerify_ALG();

    if(SW1SW2 != 0x9000)
    {
        return;
    }

    ECC_EncDec_ALG();

    if(SW1SW2 != 0x9000)
    {
        return;
    }

    X25519_ALG();

    if(SW1SW2 != 0x9000)
    {
        return;
    }

    return;
}
