#ifndef ALGORITHM_LIST
#define ALGORITHM_LIST

#define GET_ALGORITHM       0x01
#define GET_MESSAGEDIGEST   0x02
#define GET_CIPHER          0x03
#define GET_PADDING         0x04

/********************MessageDigest********************/
#define ALG_NULL               0
#define ALG_SHA                1
#define ALG_MD5                2
//#define ALG_RIPEMD160          3
//#define ALG_SHA_256            4
//#define ALG_SHA_384            5
//#define ALG_SHA_512            6
//#define ALG_SHA_224            7
#define ALG_SHA3_224           8
#define ALG_SHA3_256           9
#define ALG_SHA3_384           10
#define ALG_SHA3_512           11

/*******************Signature*************************/
#define SIG_CIPHER_DES_MAC4    1
#define SIG_CIPHER_DES_MAC8    2
#define SIG_CIPHER_RSA         3
#define SIG_CIPHER_DSA         4
#define SIG_CIPHER_ECDSA       5
#define SIG_CIPHER_ECDSA_PLAIN 9
#define SIG_CIPHER_AES_MAC128  6
#define SIG_CIPHER_AES_CMAC128 10
#define SIG_CIPHER_HMAC        7
#define SIG_CIPHER_KOREAN_SEED_MAC   8

/************************Cipher**********************/
#define PAD_NULL               0
#define PAD_NOPAD              1
#define PAD_ISO9797_M1         2
#define PAD_ISO9797_M2         3
#define PAD_ISO9797_1_M1_ALG3  4
#define PAD_ISO9797_1_M2_ALG3  5
#define PAD_PKCS5              6
#define PAD_PKCS1              7
#define PAD_PKCS1_PSS          8
#define PAD_PKCS1_OAEP         9
#define PAD_PKCS1_OAEP_SHA224  13
#define PAD_PKCS1_OAEP_SHA256  14
#define PAD_PKCS1_OAEP_SHA384  15
#define PAD_PKCS1_OAEP_SHA512  16
#define PAD_PKCS1_OAEP_SHA3_224 17
#define PAD_PKCS1_OAEP_SHA3_256 18
#define PAD_PKCS1_OAEP_SHA3_384 19
#define PAD_PKCS1_OAEP_SHA3_512 20
#define PAD_ISO9796             10
#define PAD_ISO9796_MR          11
#define PAD_RFC2409             12

#define CIPHER_AES_CBC          1
#define CIPHER_AES_ECB          2
#define CIPHER_DES_CBC          3
#define CIPHER_DES_ECB          4
#define CIPHER_KOREAN_SEED_CBC  5
#define CIPHER_KOREAN_SEED_ECB  6
#define CIPHER_RSA              7
#define CIPHER_AES_CCM          0xF2
#define CIPHER_AES_GCM          0xF1


#endif /*ALGORITHM_LIST*/
