/*
 * Sym_API.h
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "Crypto_Constant.h"
#include "Crypto_common_API.h"

#ifndef SYM_API_H_
#define SYM_API_H_

//#if WITH_SYM

/***********************************************************************************
 * SYM max processing bytes in one call to lib API
 ***********************************************************************************/
#define SYM_MAX_BYTES_ONE_STEP        		0x100


/***********************************************************************************
 * ISO9797_M1 mask flag
 ***********************************************************************************/
#define SYM_ISO9797_M1_PAD_MASK     		0x8000


/** \brief Values for checksum byte (enable / disable) */
#define SYM_CHECKSUM_VERIFY					0x7A
#define SYM_CHECKSUM_NO_VERIFY				0xA7

/** \brief Values for dfaProtection byte (enable / disable) */
#define SYM_ENABLE_DFA_PROTECTION			0x7A
#define SYM_DISABLE_DFA_PROTECTION			0xA7

/***********************************************************************************
 * For DES from lib of NXP, below
 ***********************************************************************************/
#define ERR_SYM_LOADKEY_OK                 	0x33A5  /**< \brief Load key successful */
#define ERR_SYM_LOADKEY_FAIL               	0x335A  /**< \brief Load key fail */
#define ERR_SYM_CRYPT_OK                   	0x55A5  /**< \brief Crypt data successful */
#define ERR_SYM_CRYPT_FAIL                 	0x555A  /**< \brief Load key fail */
#define ERR_SYM_CMACGENERATESUBKEYS_OK     	0x66A5  /**< \brief Generate sub keys successful */
#define ERR_SYM_CMACPROCESSLASTBLOCK_OK    	0x99A5  /**< \brief Process last block successful */
#define ERR_SYM_FAULT_ATTACK               	0xAAA5  /**< \brief Fault attack detected */
#define ERR_SYM_WRONG_CHECKSUM             	0xCCA5  /**< \brief Wrong checksum */


 /**
 * \brief Structure of parameters for function ::Sym_LoadKey_Param
 */
typedef struct Sym_LoadKey_Param {
	PX32	pKeyBuffer;            	/**< \brief Pointer to key (DES, 3-DES 2 key, 3-DES 3 key) */
	u8		keyByteLen;         	/**< \brief Pointer to key (DES, 3-DES 2 key, 3-DES 3 key) */
    u8		option;               	/**< \brief Option byte
									 *  bit 5-7:
	                                        - 0000: not check integrity
		                                    - 1010: check integrity
	                                 *  bit 2-4:
	                                        - rfu
	                                 *  bit 1:
									 *  bit 0:
									 *      - 2: ENCRYPTO
									 *      - 1: DECRYPTO
									 *
									 */
	u16		checksum;             	/**< \brief Checksum calculation enabled/disabled */
} Sym_LoadKey_Param_t;



/**
 * \brief Structure of parameters for function ::Sym_Crypt_Param
 */

typedef struct Sym_Crypt_Param {
  //Sym_LoadKey_Param_t * keyParam;				/**< \brief Pointer to output block */
    PX32	pKeyBuffer;				/**< \brief Pointer to output block */
	PM8		pIV;               		/**< \brief Pointer to initial value */
	PX8		pInput;            		/**< \brief Pointer to input block */
    PX8		pOutput;           		/**< \brief Pointer to output block */
    u16		noOfBlocks;        		/**< \brief Number of input blocks */
    u16		inputLength;        	/**< \brief length of input blocks */
	u8		keyByteLen;         	/**< \brief Pointer to key (DES, 3-DES 2 key, 3-DES 3 key) */
    u8		dfaProtect;       		/**< \brief DFA protection byte (enable/disable) */
	u8		checksum;       		/**< \brief DFA protection byte (enable/disable) */
    u8		option;            		/**< \brief Option byte
									 *  bit 7-4:
									 *      - rfu
									 *  bit 3:
									 *      - 1: MAC MODE
									 *  bit 2:
									 *      - 0: ECB MDOE
									 *      - 1: CBC MDOE
									 *  bit 1:
									 *      - 1: DECRYPTO
									 *  bit 0:
									 *      - 1: ENCRYPTO
									 */
} Sym_Crypt_Param_t;

/**
 * \brief Structure of parameters for function ::Sym_AEADCrypt_Param
 */
#if WITH_AEAD
typedef struct Sym_AEADCrypt_Param {
	Sym_Crypt_Param_t * symCryptParam;
	PX8		pNonceBuf;            		/**< \brief Pointer to nonceBuf block */
    PX8		pAdataBuf;           		/**< \brief Pointer to adataBuf block */
    PX8		pAeadGiv;           		/**< \brief Pointer to AEAD G initial value */
    PX8		pAeadCiv;           		/**< \brief Pointer to AEAD C initial value */
    PX8		pBaseEctr;           		/**< \brief Pointer to AEAD base ectr value */
    PX8		pTag;           			/**< \brief Pointer to AEAD tag value */
    u16		nonceLen;        			/**< \brief Length of input nonce */
    u16		adataLen;        			/**< \brief Length of input adata */
    u16		messageLen;        			/**< \brief Length of input all input */
    u16		tagLen;        				/**< \brief Length of tag input */
} Sym_AEADCrypt_Param_t;
#endif
/**
 * \brief Structure of parameters for function ::Sym_API_Param
 */
#define SYM_MODE_NONE     					0x00
#define SYM_MODE_NOPAD     					0x01
#define SYM_MODE_ISO9797_M1					0x02
#define SYM_MODE_ISO9797_1_ALG3				0x04
#define SYM_MODE_AEAD_GCM					0x05
#define SYM_MODE_AEAD_CCM					0x06
#define SYM_MODE_CMAC						0x07
#if WITH_AEAD
#define SYM_MODE_AEAD_GCM					0x05
#define SYM_MODE_AEAD_CCM					0x06
#endif
typedef struct Sym_API_Param {
    u16		byteNumInBlockMASK;		/**< \byte number in nulti block mask*/
    u8		SYMShiftMASK; 			/**< \SYM shift mask*/
    u8		blockSizeMASK; 			/**< \length of Current Crypto' Block mask*/
    u8		blockSize;          	/**< \length of Current Crypto' Block*/
    u8		maxBlocksOneStep;   	/**< \max Blocks One Step*/
    u8		ICVOffsetInOneStep; 	/**< \ICV Offset In One Step*/
    u8		firstKeyByteLen;        /**< \for ISO9797_1_M2 mode first used key length*/
    u8		secondKeyByteLen;       /**< \for ISO9797_1_M2 mode second used key length*/
    u8		option;				 	/**< \specific padding Type
									 *  bit 7-2:
									 *      - rfu
									 *  bit 2:
									 *      - 1: ISO9797_1_M2
									 *  bit 1:
									 *      - 1: ISO9797_M1
									 *  bit 0:
									 *      - 1: NOPAD
									 */
	u8		optionCheck;
} Sym_API_Param_t;



/***********************************************************************************

 ***********************************************************************************/
#define DES_SHIFT_MASK							(u8)3
#define DES_MAX_BLOCKS_ONE_STEP      			U16_SHR_U8(SYM_MAX_BYTES_ONE_STEP, DES_SHIFT_MASK)
#define DES_ICV_OFFSET_IN_ONE_STEP   			U16_S_U8(SYM_MAX_BYTES_ONE_STEP, DES_BLOCKSIZE)

#define BYTE_LENGTH_DES			        		(u8)8
#define BYTE_LENGTH_3DES_2KEY					(u8)16
#define BYTE_LENGTH_3DES_3KEY					(u8)24

/***********************************************************************************
 * Cipher Type, these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_DES_CBC_NOPAD						U8_OR_U8(ALG_DES_CBC_NOPAD     			, CIPHER_MODE)
#define C_ALG_DES_CBC_ISO9797_M1				U8_OR_U8(ALG_DES_CBC_ISO9797_M1			, CIPHER_MODE)
#define C_ALG_DES_CBC_ISO9797_M2				U8_OR_U8(ALG_DES_CBC_ISO9797_M2			, CIPHER_MODE)
#define C_ALG_DES_CBC_PKCS5						U8_OR_U8(ALG_DES_CBC_PKCS5     			, CIPHER_MODE)
#define C_ALG_DES_ECB_NOPAD						U8_OR_U8(ALG_DES_ECB_NOPAD     			, CIPHER_MODE)
#define C_ALG_DES_ECB_ISO9797_M1				U8_OR_U8(ALG_DES_ECB_ISO9797_M1			, CIPHER_MODE)
#define C_ALG_DES_ECB_ISO9797_M2				U8_OR_U8(ALG_DES_ECB_ISO9797_M2			, CIPHER_MODE)
#define C_ALG_DES_ECB_PKCS5						U8_OR_U8(ALG_DES_ECB_PKCS5     			, CIPHER_MODE)

/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_DES_MAC4_NOPAD					U8_OR_U8(ALG_DES_MAC4_NOPAD 			, SIGNATURE_MODE)
#define C_ALG_DES_MAC8_NOPAD					U8_OR_U8(ALG_DES_MAC8_NOPAD 			, SIGNATURE_MODE)
#define C_ALG_DES_MAC4_ISO9797_M1				U8_OR_U8(ALG_DES_MAC4_ISO9797_M1 		, SIGNATURE_MODE)
#define C_ALG_DES_MAC8_ISO9797_M1				U8_OR_U8(ALG_DES_MAC8_ISO9797_M1 		, SIGNATURE_MODE)
#define C_ALG_DES_MAC4_ISO9797_M2				U8_OR_U8(ALG_DES_MAC4_ISO9797_M2 		, SIGNATURE_MODE)
#define C_ALG_DES_MAC8_ISO9797_M2				U8_OR_U8(ALG_DES_MAC8_ISO9797_M2 		, SIGNATURE_MODE)
#define C_ALG_DES_MAC4_PKCS5 					U8_OR_U8(ALG_DES_MAC4_PKCS5 			, SIGNATURE_MODE)
#define C_ALG_DES_MAC8_PKCS5					U8_OR_U8(ALG_DES_MAC8_PKCS5 			, SIGNATURE_MODE)
#define C_ALG_DES_MAC4_ISO9797_1_M2_ALG3        U8_OR_U8(ALG_DES_MAC4_ISO9797_1_M2_ALG3	, SIGNATURE_MODE)
#define C_ALG_DES_MAC8_ISO9797_1_M2_ALG3        U8_OR_U8(ALG_DES_MAC8_ISO9797_1_M2_ALG3 , SIGNATURE_MODE)
#define C_ALG_DES_MAC4_ISO9797_1_M1_ALG3        U8_OR_U8(ALG_DES_MAC4_ISO9797_1_M1_ALG3	, SIGNATURE_MODE)
#define C_ALG_DES_MAC8_ISO9797_1_M1_ALG3        U8_OR_U8(ALG_DES_MAC8_ISO9797_1_M1_ALG3 , SIGNATURE_MODE)

#define DES_PKCS5_PADDING               		1

/***********************************************************************************
 * 											DES API
 ***********************************************************************************/
 #if WITH_DES
extern u16 Sym_DES_LoadKey(Sym_LoadKey_Param_t * myKeyParam);
extern u16 Sym_DES_Crypt(Sym_Crypt_Param_t * myCryptParam);

#define Sym_Wipekey()  
#else
	#define  Sym_DES_LoadKey( myKeyParam)						0
	#define  Sym_DES_Crypt(myCryptParam)						0
#endif // WITH_DES




/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define AES_SHIFT_MASK							(u8)4
#define AES_MAX_BLOCKS_ONE_STEP      			U16_SHR_U8(SYM_MAX_BYTES_ONE_STEP, AES_SHIFT_MASK)
#define AES_ICV_OFFSET_IN_ONE_STEP   			U16_S_U8(SYM_MAX_BYTES_ONE_STEP, AES_BLOCKSIZE)

#define BYTE_LENGTH_AES_128			        	(u8)16
#define BYTE_LENGTH_AES_192						(u8)24
#define BYTE_LENGTH_AES_256						(u8)32

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_AES_BLOCK_128_CBC_NOPAD 			U8_OR_U8(ALG_AES_BLOCK_128_CBC_NOPAD 	, CIPHER_MODE)
#define C_ALG_AES_BLOCK_128_ECB_NOPAD			U8_OR_U8(ALG_AES_BLOCK_128_ECB_NOPAD 	, CIPHER_MODE)

#define C_ALG_AES_CBC_ISO9797_M1 				U8_OR_U8(ALG_AES_CBC_ISO9797_M1 		, CIPHER_MODE)
#define C_ALG_AES_CBC_ISO9797_M2				U8_OR_U8(ALG_AES_CBC_ISO9797_M2 		, CIPHER_MODE)

#define C_ALG_AES_CBC_PKCS5						U8_OR_U8(ALG_AES_CBC_PKCS5 				, CIPHER_MODE)
#define C_ALG_AES_ECB_PKCS5						U8_OR_U8(ALG_AES_ECB_PKCS5 				, CIPHER_MODE)

#define C_ALG_AES_ECB_ISO9797_M1				U8_OR_U8(ALG_AES_ECB_ISO9797_M1 		, CIPHER_MODE)
#define C_ALG_AES_ECB_ISO9797_M2 				U8_OR_U8(ALG_AES_ECB_ISO9797_M2 		, CIPHER_MODE)

#define AES_PKCS5_PADDING           			1

/***********************************************************************************
 * 									  AES   API
 ***********************************************************************************/
#if WITH_AES
	//extern static void DoAES_Crypt(PX8 dataIn , PX8 dataOut , u8 en_deCrypt , u8 KeyByteLen);
	extern u16 Sym_AES_LoadKey(Sym_LoadKey_Param_t * myKeyParam);
	extern u16 Sym_AES_Crypt(Sym_Crypt_Param_t * myCryptParam);
#else
	//#define DoAES_Crypt( dataIn , dataOut ,  en_deCrypt ,  KeyByteLen)
	#define  Sym_AES_LoadKey( myKeyParam)			0
	#define  Sym_AES_Crypt( myCryptParam)			0
#endif // WITH_AES




/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define KOREANSEED_SHIFT_MASK							(u8)4
#define KOREANSEED_MAX_BLOCKS_ONE_STEP      			(SYM_MAX_BYTES_ONE_STEP >> KOREANSEED_SHIFT_MASK)
#define KOREANSEED_ICV_OFFSET_IN_ONE_STEP   			(SYM_MAX_BYTES_ONE_STEP - KOREANSEED_BLOCKSIZE)

#define BYTE_LENGTH_KOREANSEED_128			        	(u8)16
#define BYTE_LENGTH_KOREANSEED_192						(u8)24
#define BYTE_LENGTH_KOREANSEED_256						(u8)32

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
//#define C_ALG_KOREANSEED_BLOCK_128_CBC_NOPAD 			(ALG_KOREANSEED_BLOCK_128_CBC_NOPAD 	| CIPHER_MODE)
//#define C_ALG_KOREANSEED_BLOCK_128_ECB_NOPAD			(ALG_KOREANSEED_BLOCK_128_ECB_NOPAD 	| CIPHER_MODE)
//
//#define C_ALG_KOREANSEED_CBC_ISO9797_M1 				(ALG_KOREANSEED_CBC_ISO9797_M1 		| CIPHER_MODE)
//#define C_ALG_KOREANSEED_CBC_ISO9797_M2				(ALG_KOREANSEED_CBC_ISO9797_M2 		| CIPHER_MODE)
//
//#define C_ALG_KOREANSEED_CBC_PKCS5						(ALG_KOREANSEED_CBC_PKCS5 				| CIPHER_MODE)
//#define C_ALG_KOREANSEED_ECB_PKCS5						(ALG_KOREANSEED_ECB_PKCS5 				| CIPHER_MODE)
//
//#define C_ALG_KOREANSEED_ECB_ISO9797_M1				(ALG_KOREANSEED_ECB_ISO9797_M1 		| CIPHER_MODE)
//#define C_ALG_KOREANSEED_ECB_ISO9797_M2 				(ALG_KOREANSEED_ECB_ISO9797_M2 		| CIPHER_MODE)
//
//#define KOREANSEED_PKCS5_PADDING           			1

/***********************************************************************************
 * 							KOREANSEED  API
 ***********************************************************************************/
#if WITH_KOREANSEED
	//extern static void DoKOREANSEED_Crypt(PX8 dataIn , PX8 dataOut , u8 en_deCrypt , u8 KeyByteLen);
	extern u16 Sym_KOREANSEED_LoadKey(Sym_LoadKey_Param_t * myKeyParam);
	extern u16 Sym_KOREANSEED_Crypt(Sym_Crypt_Param_t * myCryptParam);
#else
	//#define   DoKOREANSEED_Crypt( dataIn ,  dataOut ,  en_deCrypt ,  KeyByteLen)
	#define  Sym_KOREANSEED_LoadKey(  myKeyParam)					0
	#define  Sym_KOREANSEED_Crypt(  myCryptParam)					0
#endif // WITH_KOREANSEED



/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define SM1_SHIFT_MASK							(u8)4
#define SM1_MAX_BLOCKS_ONE_STEP      			(SYM_MAX_BYTES_ONE_STEP >> SM1_SHIFT_MASK)
#define SM1_ICV_OFFSET_IN_ONE_STEP   			(SYM_MAX_BYTES_ONE_STEP - SM1_BLOCKSIZE)

#define BYTE_LENGTH_SM1_128			        	(u8)16
#define BYTE_LENGTH_SM1_192						(u8)24
#define BYTE_LENGTH_SM1_256						(u8)32

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_SM1_CBC_NOPAD						U8_OR_U8(ALG_SM1_CBC_NOPAD 			, CIPHER_MODE)
#define C_ALG_SM1_CBC_ISO9797_M1				U8_OR_U8(ALG_SM1_CBC_ISO9797_M1 	, CIPHER_MODE)
#define C_ALG_SM1_CBC_ISO9797_M2				U8_OR_U8(ALG_SM1_CBC_ISO9797_M2 	, CIPHER_MODE)
#define C_ALG_SM1_ECB_NOPAD			 			U8_OR_U8(ALG_SM1_ECB_NOPAD 			, CIPHER_MODE)
#define C_ALG_SM1_ECB_ISO9797_M1				U8_OR_U8(ALG_SM1_ECB_ISO9797_M1 	, CIPHER_MODE)
#define C_ALG_SM1_ECB_ISO9797_M2				U8_OR_U8(ALG_SM1_ECB_ISO9797_M2 	, CIPHER_MODE)

/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_SM1_MAC16_NOPAD					U8_OR_U8(ALG_SM1_MAC16_NOPAD 		, SIGNATURE_MODE)
#define C_ALG_SM1_MAC8_NOPAD					U8_OR_U8(ALG_SM1_MAC8_NOPAD 		, SIGNATURE_MODE)
#define C_ALG_SM1_MAC16_ISO9797_M1				U8_OR_U8(ALG_SM1_MAC16_ISO9797_M1 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC8_ISO9797_M1				U8_OR_U8(ALG_SM1_MAC8_ISO9797_M1 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC16_ISO9797_M2				U8_OR_U8(ALG_SM1_MAC16_ISO9797_M2 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC8_ISO9797_M2				U8_OR_U8(ALG_SM1_MAC8_ISO9797_M2 	, SIGNATURE_MODE)

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_SM1_CBC_NOPAD_NEW					U8_OR_U8(ALG_SM1_CBC_NOPAD_NEW 			, CIPHER_MODE)
#define C_ALG_SM1_CBC_ISO9797_M1_NEW			U8_OR_U8(ALG_SM1_CBC_ISO9797_M1_NEW 	, CIPHER_MODE)
#define C_ALG_SM1_CBC_ISO9797_M2_NEW			U8_OR_U8(ALG_SM1_CBC_ISO9797_M2_NEW 	, CIPHER_MODE)
#define C_ALG_SM1_ECB_NOPAD_NEW			 		U8_OR_U8(ALG_SM1_ECB_NOPAD_NEW 			, CIPHER_MODE)
#define C_ALG_SM1_ECB_ISO9797_M1_NEW			U8_OR_U8(ALG_SM1_ECB_ISO9797_M1_NEW 	, CIPHER_MODE)
#define C_ALG_SM1_ECB_ISO9797_M2_NEW			U8_OR_U8(ALG_SM1_ECB_ISO9797_M2_NEW 	, CIPHER_MODE)

/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_SM1_MAC16_NOPAD_NEW				U8_OR_U8(ALG_SM1_MAC16_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM1_MAC8_NOPAD_NEW				U8_OR_U8(ALG_SM1_MAC8_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM1_MAC4_NOPAD_NEW				U8_OR_U8(ALG_SM1_MAC4_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM1_MAC16_ISO9797_M1_NEW			U8_OR_U8(ALG_SM1_MAC16_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC8_ISO9797_M1_NEW			U8_OR_U8(ALG_SM1_MAC8_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC4_ISO9797_M1_NEW			U8_OR_U8(ALG_SM1_MAC4_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC16_ISO9797_M2_NEW			U8_OR_U8(ALG_SM1_MAC16_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC8_ISO9797_M2_NEW			U8_OR_U8(ALG_SM1_MAC8_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM1_MAC4_ISO9797_M2_NEW			U8_OR_U8(ALG_SM1_MAC4_ISO9797_M2_NEW 	, SIGNATURE_MODE)

/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define SM4_SHIFT_MASK							(u8)4
#define SM4_MAX_BLOCKS_ONE_STEP      			(SYM_MAX_BYTES_ONE_STEP >> SM4_SHIFT_MASK)
#define SM4_ICV_OFFSET_IN_ONE_STEP   			(SYM_MAX_BYTES_ONE_STEP - SM4_BLOCKSIZE)

#define BYTE_LENGTH_SM4_128			        	(u8)16
#define BYTE_LENGTH_SM4_192						(u8)24
#define BYTE_LENGTH_SM4_256						(u8)32

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_SM4_CBC_NOPAD						U8_OR_U8(ALG_SM4_CBC_NOPAD 			, CIPHER_MODE)
#define C_ALG_SM4_CBC_ISO9797_M1				U8_OR_U8(ALG_SM4_CBC_ISO9797_M1 	, CIPHER_MODE)
#define C_ALG_SM4_CBC_ISO9797_M2				U8_OR_U8(ALG_SM4_CBC_ISO9797_M2 	, CIPHER_MODE)
#define C_ALG_SM4_ECB_NOPAD			 			U8_OR_U8(ALG_SM4_ECB_NOPAD 			, CIPHER_MODE)
#define C_ALG_SM4_ECB_ISO9797_M1				U8_OR_U8(ALG_SM4_ECB_ISO9797_M1 	, CIPHER_MODE)
#define C_ALG_SM4_ECB_ISO9797_M2				U8_OR_U8(ALG_SM4_ECB_ISO9797_M2 	, CIPHER_MODE)

/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_SM4_MAC16_NOPAD					U8_OR_U8(ALG_SM4_MAC16_NOPAD 		, SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_NOPAD					U8_OR_U8(ALG_SM4_MAC8_NOPAD 		, SIGNATURE_MODE)
#define C_ALG_SM4_MAC16_ISO9797_M1				U8_OR_U8(ALG_SM4_MAC16_ISO9797_M1 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_ISO9797_M1				U8_OR_U8(ALG_SM4_MAC8_ISO9797_M1 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC16_ISO9797_M2				U8_OR_U8(ALG_SM4_MAC16_ISO9797_M2 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_ISO9797_M2				U8_OR_U8(ALG_SM4_MAC8_ISO9797_M2 	, SIGNATURE_MODE)

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_SM4_CBC_NOPAD_NEW					U8_OR_U8(ALG_SM4_CBC_NOPAD_NEW 			, CIPHER_MODE)
#define C_ALG_SM4_CBC_ISO9797_M1_NEW			U8_OR_U8(ALG_SM4_CBC_ISO9797_M1_NEW 	, CIPHER_MODE)
#define C_ALG_SM4_CBC_ISO9797_M2_NEW			U8_OR_U8(ALG_SM4_CBC_ISO9797_M2_NEW 	, CIPHER_MODE)
#define C_ALG_SM4_ECB_NOPAD_NEW			 		U8_OR_U8(ALG_SM4_ECB_NOPAD_NEW 			, CIPHER_MODE)
#define C_ALG_SM4_ECB_ISO9797_M1_NEW			U8_OR_U8(ALG_SM4_ECB_ISO9797_M1_NEW 	, CIPHER_MODE)
#define C_ALG_SM4_ECB_ISO9797_M2_NEW			U8_OR_U8(ALG_SM4_ECB_ISO9797_M2_NEW 	, CIPHER_MODE)
#define C_ALG_SM4_CBC_PKCS5_NEW     			U8_OR_U8(ALG_SM4_CBC_PKCS5_NEW      	, CIPHER_MODE)
#define C_ALG_SM4_ECB_PKCS5_NEW     			U8_OR_U8(ALG_SM4_ECB_PKCS5_NEW      	, CIPHER_MODE)


/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_SM4_MAC16_NOPAD_NEW				U8_OR_U8(ALG_SM4_MAC16_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_NOPAD_NEW				U8_OR_U8(ALG_SM4_MAC8_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM4_MAC4_NOPAD_NEW				U8_OR_U8(ALG_SM4_MAC4_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM4_MAC16_ISO9797_M1_NEW			U8_OR_U8(ALG_SM4_MAC16_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_ISO9797_M1_NEW			U8_OR_U8(ALG_SM4_MAC8_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC4_ISO9797_M1_NEW			U8_OR_U8(ALG_SM4_MAC4_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC16_ISO9797_M2_NEW			U8_OR_U8(ALG_SM4_MAC16_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_ISO9797_M2_NEW			U8_OR_U8(ALG_SM4_MAC8_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC4_ISO9797_M2_NEW			U8_OR_U8(ALG_SM4_MAC4_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM4_MAC4_PKCS5_NEW			    U8_OR_U8(ALG_SM4_MAC4_PKCS5_NEW 	    , SIGNATURE_MODE)
#define C_ALG_SM4_MAC8_PKCS5_NEW			    U8_OR_U8(ALG_SM4_MAC8_PKCS5_NEW 	    , SIGNATURE_MODE)
#define C_ALG_SM4_MAC16_PKCS5_NEW			    U8_OR_U8(ALG_SM4_MAC16_PKCS5_NEW 	    , SIGNATURE_MODE)



/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define SM7_SHIFT_MASK							(u8)3
#define SM7_MAX_BLOCKS_ONE_STEP      			(SYM_MAX_BYTES_ONE_STEP >> SM7_SHIFT_MASK)
#define SM7_ICV_OFFSET_IN_ONE_STEP   			(SYM_MAX_BYTES_ONE_STEP - SM7_BLOCKSIZE)

#define BYTE_LENGTH_SM7				        	(u8)8

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_SM7_CBC_NOPAD						U8_OR_U8(ALG_SM7_CBC_NOPAD 			, CIPHER_MODE)
#define C_ALG_SM7_CBC_ISO9797_M1				U8_OR_U8(ALG_SM7_CBC_ISO9797_M1 	, CIPHER_MODE)
#define C_ALG_SM7_CBC_ISO9797_M2				U8_OR_U8(ALG_SM7_CBC_ISO9797_M2 	, CIPHER_MODE)
#define C_ALG_SM7_ECB_NOPAD			 			U8_OR_U8(ALG_SM7_ECB_NOPAD 			, CIPHER_MODE)
#define C_ALG_SM7_ECB_ISO9797_M1				U8_OR_U8(ALG_SM7_ECB_ISO9797_M1 	, CIPHER_MODE)
#define C_ALG_SM7_ECB_ISO9797_M2				U8_OR_U8(ALG_SM7_ECB_ISO9797_M2 	, CIPHER_MODE)

/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_SM7_MAC16_NOPAD					U8_OR_U8(ALG_SM7_MAC16_NOPAD 		, SIGNATURE_MODE)
#define C_ALG_SM7_MAC8_NOPAD					U8_OR_U8(ALG_SM7_MAC8_NOPAD 		, SIGNATURE_MODE)
#define C_ALG_SM7_MAC16_ISO9797_M1				U8_OR_U8(ALG_SM7_MAC16_ISO9797_M1 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC8_ISO9797_M1				U8_OR_U8(ALG_SM7_MAC8_ISO9797_M1 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC16_ISO9797_M2				U8_OR_U8(ALG_SM7_MAC16_ISO9797_M2 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC8_ISO9797_M2				U8_OR_U8(ALG_SM7_MAC8_ISO9797_M2 	, SIGNATURE_MODE)

/***********************************************************************************
 * Cipher Type,  these definations are masked with information of cipher.
 ***********************************************************************************/
#define C_ALG_SM7_CBC_NOPAD_NEW					U8_OR_U8(ALG_SM7_CBC_NOPAD_NEW 			, CIPHER_MODE)
#define C_ALG_SM7_CBC_ISO9797_M1_NEW			U8_OR_U8(ALG_SM7_CBC_ISO9797_M1_NEW 	, CIPHER_MODE)
#define C_ALG_SM7_CBC_ISO9797_M2_NEW			U8_OR_U8(ALG_SM7_CBC_ISO9797_M2_NEW 	, CIPHER_MODE)
#define C_ALG_SM7_ECB_NOPAD_NEW			 		U8_OR_U8(ALG_SM7_ECB_NOPAD_NEW 			, CIPHER_MODE)
#define C_ALG_SM7_ECB_ISO9797_M1_NEW			U8_OR_U8(ALG_SM7_ECB_ISO9797_M1_NEW 	, CIPHER_MODE)
#define C_ALG_SM7_ECB_ISO9797_M2_NEW			U8_OR_U8(ALG_SM7_ECB_ISO9797_M2_NEW 	, CIPHER_MODE)

/***********************************************************************************
 * Signature Type, these definations are masked with information of signature.
 ***********************************************************************************/
#define C_ALG_SM7_MAC16_NOPAD_NEW				U8_OR_U8(ALG_SM7_MAC16_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM7_MAC8_NOPAD_NEW				U8_OR_U8(ALG_SM7_MAC8_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM7_MAC4_NOPAD_NEW				U8_OR_U8(ALG_SM7_MAC4_NOPAD_NEW 		, SIGNATURE_MODE)
#define C_ALG_SM7_MAC16_ISO9797_M1_NEW			U8_OR_U8(ALG_SM7_MAC16_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC8_ISO9797_M1_NEW			U8_OR_U8(ALG_SM7_MAC8_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC4_ISO9797_M1_NEW			U8_OR_U8(ALG_SM7_MAC4_ISO9797_M1_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC16_ISO9797_M2_NEW			U8_OR_U8(ALG_SM7_MAC16_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC8_ISO9797_M2_NEW			U8_OR_U8(ALG_SM7_MAC8_ISO9797_M2_NEW 	, SIGNATURE_MODE)
#define C_ALG_SM7_MAC4_ISO9797_M2_NEW			U8_OR_U8(ALG_SM7_MAC4_ISO9797_M2_NEW 	, SIGNATURE_MODE)

/***********************************************************************************
 * 								SM4 API
 ***********************************************************************************/
#if WITH_SM4
	//extern static void DoSM4_Crypt(PX8 dataIn , PX8 dataOut , u8 en_deCrypt , u8 KeyByteLen);
	extern u16 Sym_SM4_LoadKey(Sym_LoadKey_Param_t * myKeyParam);
	extern u16 Sym_SM4_Crypt(Sym_Crypt_Param_t * myCryptParam);
#else
	//#define  DoSM4_Crypt( dataIn ,  dataOut ,  en_deCrypt ,  KeyByteLen)     
	#define  Sym_SM4_LoadKey( myKeyParam)			0
	#define  Sym_SM4_Crypt( myCryptParam)			0
#endif // WITH_SM4


/***********************************************************************************
 * 								SM1 API
 ***********************************************************************************/
#if WITH_SM1
	extern u16 Sym_SM1_LoadKey(Sym_LoadKey_Param_t * myKeyParam);
	extern u16 Sym_SM1_Crypt(Sym_Crypt_Param_t * myCryptParam);
#else
	#define  Sym_SM1_LoadKey( myKeyParam)			0
	#define  Sym_SM1_Crypt( myCryptParam)			0
#endif // WITH_SM1


/***********************************************************************************
 * 								SM7 API
 ***********************************************************************************/
#if WITH_SM7
	extern u16 Sym_SM7_LoadKey(Sym_LoadKey_Param_t * myKeyParam);
	extern u16 Sym_SM7_Crypt(Sym_Crypt_Param_t * myCryptParam);
#else
	#define  Sym_SM7_LoadKey( myKeyParam)			0
	#define  Sym_SM7_Crypt( myCryptParam)			0
#endif // WITH_SM7

/***********************************************************************************
 * 								Sym api
 ***********************************************************************************/
#if	WITH_SYM
	extern u8 getSYMalgType(u8 symAlg, u8 algType, u8 currentMode);
	extern u8 getSYMBlockSize(u8 symAlg);
	extern u16 getSYMSinReturnLength(u8 symAlg, u8 algType, u16 inLen);
	extern void SymKeyInitDispatcher(Sym_LoadKey_Param_t * myLoadKeyParam, u8 algType);
	extern void SymCryptDispatcher(Sym_Crypt_Param_t * myCryptParam, u8 symAlg);
	extern void SymKeyInitParamsInit(Sym_LoadKey_Param_t * myLoadKeyParam, PM8 keyBuff, PX32 symKeyBuffer, u16 keyBitLen, u8 mode, u16 flag, u8 algType);
	extern void SymCryptParamsInit(Sym_Crypt_Param_t * mycryptParam, PM8 ICVBuff, PX32 symKeyBuffer, u16 keyByteLen, PX8 pInput, PX8 pOutput, u8 mode);
	extern void SymAPIParamsInit(Sym_API_Param_t * myAPIParam, u8 symAlg, u8 algType, u8 currentMode);
#else
	#define  getSYMalgType(symAlg, algType, currentMode)		SYM_MODE_NONE
	#define  SymKeyInitParamsInit( myLoadKeyParam, keyBuff, symKeyBuffer, keyBitLen, mode, flag, algType)
	#define  SymCryptParamsInit( mycryptParam, ICVBuff, symKeyBuffer, keyByteLen, pInput, pOutput, mode)
	#define  SymAPIParamsInit( myAPIParam,  symAlg,  algType,  currentMode)
#endif /* WITH_SYM */

#if WITH_AEAD
	extern int Sym_AEAD_starts(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam  );
	extern int mbedtls_AEAD_finish(Sym_LoadKey_Param_t * myLoadkeyParam, Sym_AEADCrypt_Param_t * myAEADCryptParam );
#endif /* WITH_AEAD*/

#endif /* SYM_API_H_ */

