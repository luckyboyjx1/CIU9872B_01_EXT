/*
 * Crypto_Constant.h
 *
 *  Created on: 2014-2-28
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_CONSTANT_H_
#define CRYPTO_CONSTANT_H_

#include "SysConfigProfile.h"
#include "DataOperation.h"
#include "halCryptoProfile.h"

/***********************************************************************************
 *                      DES  Constant Definitions
 ***********************************************************************************/
#define ALG_DES									0x01

#define DES_MAC_LENGTH					 		8
#define DES_BLOCKSIZE		         			8
#define DES_BLOCKSIZE_MASK               		7
#define DES_BYTENUM_IN_MULTIBLOCK_MASK   		0xFFF8

#define LENGTH_DES			        			64
#define LENGTH_3DES_2KEY						128
#define LENGTH_3DES_3KEY						192

#define DES_KEY1_OFFSET							0
#define DES_KEY2_OFFSET							(LENGTH_DES >> 3)
#define DES_KEY3_OFFSET							(LENGTH_3DES_2KEY >> 3)

#define LENGTH_DES_BYTE							(LENGTH_DES >> 3)

/***********************************************************************************
 * Cipher Type, these definations are comply with JAVA layer definations
 ***********************************************************************************/
#define ALG_DES_CBC_NOPAD						1
#define ALG_DES_CBC_ISO9797_M1					2
#define ALG_DES_CBC_ISO9797_M2		 			3
#define ALG_DES_CBC_PKCS5						4
#define ALG_DES_ECB_NOPAD						5
#define ALG_DES_ECB_ISO9797_M1		 			6
#define ALG_DES_ECB_ISO9797_M2		 			7
#define ALG_DES_ECB_PKCS5						8

/***********************************************************************************
 * Signature Type, these definations are comply with JAVA layer definations
 ***********************************************************************************/
#define ALG_DES_MAC4_NOPAD						1
#define ALG_DES_MAC8_NOPAD						2
#define ALG_DES_MAC4_ISO9797_M1					3
#define ALG_DES_MAC8_ISO9797_M1					4
#define ALG_DES_MAC4_ISO9797_M2					5
#define ALG_DES_MAC8_ISO9797_M2					6
#define ALG_DES_MAC4_PKCS5 						7
#define ALG_DES_MAC8_PKCS5						8
#define ALG_DES_MAC4_ISO9797_1_M1_ALG3  47
#define ALG_DES_MAC8_ISO9797_1_M1_ALG3  48
#define ALG_DES_MAC4_ISO9797_1_M2_ALG3          19
#define ALG_DES_MAC8_ISO9797_1_M2_ALG3          20
#define ALG_DES_MAC4_ISO9797_1_M1_ALG3          47
#define ALG_DES_MAC8_ISO9797_1_M1_ALG3          48

/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define ALG_AES									0x02
#define ALG_AEAD								(ALG_AES | SBC_AEAD)
 
#define AES_MAC_LENGTH					 		16
#define AES_BLOCKSIZE				     		16
#define AES_BLOCKSIZE_MASK               		15
#define AES_BYTENUM_IN_MULTIBLOCK_MASK   		0xFFF0

#define LENGTH_AES_128                  		128
#define LENGTH_AES_192                  		192
#define LENGTH_AES_256                  		256

/***********************************************************************************
 * Cipher Type, these definations are comply with JAVA layer definations
 ***********************************************************************************/
#define ALG_AES_BLOCK_128_CBC_NOPAD 			13
#define ALG_AES_BLOCK_128_ECB_NOPAD				14

//#define ALG_AES_BLOCK_192_CBC_NOPAD 			18
//#define ALG_AES_BLOCK_192_ECB_NOPAD			19

//#define ALG_AES_BLOCK_256_CBC_NOPAD 			20
//#define ALG_AES_BLOCK_256_ECB_NOPAD 			21

#define ALG_AES_CBC_ISO9797_M1 					22
#define ALG_AES_CBC_ISO9797_M2					23

#define ALG_AES_CBC_PKCS5						24
#define ALG_AES_ECB_PKCS5						27

#define ALG_AES_ECB_ISO9797_M1					25
#define ALG_AES_ECB_ISO9797_M2 					26

/***********************************************************************************
 * Signature Type, these definations are comply with JAVA layer definations
 ***********************************************************************************/
#define ALG_AES_MAC_128_NOPAD					18
//#define ALG_AES_MAC_192_NOPAD 				35
//#define ALG_AES_MAC_256_NOPAD 				36
//#define ALG_AES_BLOCK_192_CBC_NOPAD 			18
//#define ALG_AES_BLOCK_192_ECB_NOPAD			19

//#define ALG_AES_BLOCK_256_CBC_NOPAD 			20
//#define ALG_AES_BLOCK_256_ECB_NOPAD 			21
#define ALG_AES_CMAC16							0x66

#if WITH_JC305
#define ALG_AES_CCM 							0xF4
#define ALG_AES_GCM 							0xF3
#else
#define ALG_AES_CCM 							0xB1
#define ALG_AES_GCM 							0xB0
#endif

/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/
#define ALG_KOREANSEED							0x03

#define KOREANSEED_MAC_LENGTH					16
#define KOREANSEED_BLOCKSIZE				    16
#define KOREANSEED_BLOCKSIZE_MASK               15
#define KOREANSEED_BYTENUM_IN_MULTIBLOCK_MASK   0xFFF0

#define LENGTH_KOREANSEED_128          			128
#define LENGTH_KOREANSEED_192          			192
#define LENGTH_KOREANSEED_256          			256

/***********************************************************************************
 * Cipher Type, these definitions are comply with JAVA layer definitions
 ***********************************************************************************/
#define ALG_KOREAN_SEED_ECB_NOPAD				16
#define ALG_KOREAN_SEED_CBC_NOPAD     			17

/***********************************************************************************
 * Signature Type, these definitions are comply with JAVA layer definitions
 ***********************************************************************************/
#define ALG_KOREAN_SEED_MAC_NOPAD	    		32



/***********************************************************************************
 * 						SM7  Constant Definitions
 ***********************************************************************************/
#define ALG_SM7									0x06

#define SM7_MAC_LENGTH					 		(u8)8
#define SM7_BLOCKSIZE				     		(u8)8
#define SM7_BLOCKSIZE_MASK               		(u8)7
#define SM7_BYTENUM_IN_MULTIBLOCK_MASK   		(u16)0xFFF8

#define LENGTH_SM7		                  		64


/***********************************************************************************
 * Cipher Type, these definations are comply with JAVA layer definations  new standard
 ***********************************************************************************/
 
#define ALG_SM7_CBC_NOPAD_NEW      	  			0x11
#define ALG_SM7_CBC_ISO9797_M1_NEW     			0x12
#define ALG_SM7_CBC_ISO9797_M2_NEW     			0x13
#define ALG_SM7_ECB_NOPAD_NEW  		  			0x15
#define ALG_SM7_ECB_ISO9797_M1_NEW     			0x16
#define ALG_SM7_ECB_ISO9797_M2_NEW     			0x17

/***********************************************************************************
 * Signature Type, these definations are comply with JAVA layer definations new standard
 ***********************************************************************************/
#define ALG_SM7_MAC4_NOPAD_NEW    	  			0x11
#define ALG_SM7_MAC4_ISO9797_M1_NEW   			0x13
#define ALG_SM7_MAC4_ISO9797_M2_NEW    			0x15
#define ALG_SM7_MAC8_NOPAD_NEW    	  			0x12
#define ALG_SM7_MAC8_ISO9797_M1_NEW   			0x14
#define ALG_SM7_MAC8_ISO9797_M2_NEW    			0x16



/***********************************************************************************
 * 						SM4  Constant Definitions
 ***********************************************************************************/
#define ALG_SM4									0x05

#define SM4_MAC_LENGTH					 		(u8)16
#define SM4_BLOCKSIZE				     		(u8)16
#define SM4_BLOCKSIZE_MASK               		(u8)15
#define SM4_BYTENUM_IN_MULTIBLOCK_MASK   		(u16)0xFFF0

#define LENGTH_SM4		                  		128

/***********************************************************************************
 * Cipher Type, these definations are comply with JAVA layer definations  old standard
 ***********************************************************************************/

#define ALG_SM4_CBC_NOPAD       	  			7
#define ALG_SM4_CBC_ISO9797_M1        			8
#define ALG_SM4_CBC_ISO9797_M2        			9
#define ALG_SM4_ECB_NOPAD      		  			10
#define ALG_SM4_ECB_ISO9797_M1        			11
#define ALG_SM4_ECB_ISO9797_M2        			12

/***********************************************************************************
 * Signature Type, these definations are comply with JAVA layer definations new standard
 ***********************************************************************************/
#define ALG_SM4_MAC8_NOPAD      	  			7
#define ALG_SM4_MAC8_ISO9797_M1       			8
#define ALG_SM4_MAC8_ISO9797_M2       			9
#define ALG_SM4_MAC16_NOPAD           			10
#define ALG_SM4_MAC16_ISO9797_M1      			11
#define ALG_SM4_MAC16_ISO9797_M2      			12

/***********************************************************************************
 * Cipher Type, these definations are comply with JAVA layer definations  new standard
 ***********************************************************************************/
 
#define ALG_SM4_CBC_NOPAD_NEW      	  			0x09
#define ALG_SM4_CBC_ISO9797_M1_NEW     			0x0A
#define ALG_SM4_CBC_ISO9797_M2_NEW     			0x0B
#define ALG_SM4_CBC_PKCS5_NEW     			    0x0C
#define ALG_SM4_ECB_NOPAD_NEW  		  			0x0D
#define ALG_SM4_ECB_ISO9797_M1_NEW     			0x0E
#define ALG_SM4_ECB_ISO9797_M2_NEW     			0x0F
#define ALG_SM4_ECB_PKCS5_NEW     			    0x10

/***********************************************************************************
 * Signature Type, these definations are comply with JAVA layer definations new standard
 ***********************************************************************************/
#define ALG_SM4_MAC4_NOPAD_NEW    	  			0x09
#define ALG_SM4_MAC4_ISO9797_M1_NEW   			0x0B
#define ALG_SM4_MAC4_ISO9797_M2_NEW    			0x0D
#define ALG_SM4_MAC8_NOPAD_NEW    	  			0x0A
#define ALG_SM4_MAC8_ISO9797_M1_NEW   			0x0C
#define ALG_SM4_MAC8_ISO9797_M2_NEW    			0x0E
#define ALG_SM4_MAC4_PKCS5_NEW                  0x0F
#define ALG_SM4_MAC8_PKCS5_NEW                  0x10
#define ALG_SM4_MAC16_NOPAD_NEW        			0x2A
#define ALG_SM4_MAC16_ISO9797_M1_NEW   			0x2B
#define ALG_SM4_MAC16_ISO9797_M2_NEW  			0x2C
#define ALG_SM4_MAC16_PKCS5_NEW  			    0x2D


/***********************************************************************************
 * 						ALG_SM1
 ***********************************************************************************/
#define ALG_SM1									0x04
#define SM1_MAC_LENGTH					 		(u8)16
#define SM1_BLOCKSIZE				     		(u8)16
#define SM1_BLOCKSIZE_MASK               		(u8)15
#define SM1_BYTENUM_IN_MULTIBLOCK_MASK   		(u16)0xFFF0

//#define LENGTH_SM1		                  		128
//#define LENGTH_SM1_2KEY                         256

/***********************************************************************************
 * Cipher Type, these definitions are comply with JAVA layer definitions  old standard
 ***********************************************************************************/
#define ALG_SM1_CBC_NOPAD       	  			1
#define ALG_SM1_CBC_ISO9797_M1        			2
#define ALG_SM1_CBC_ISO9797_M2        			3
#define ALG_SM1_ECB_NOPAD      		  			4
#define ALG_SM1_ECB_ISO9797_M1        			5
#define ALG_SM1_ECB_ISO9797_M2        			6

/***********************************************************************************
 * Signature Type, these definitions are comply with JAVA layer definitions old standard
 ***********************************************************************************/
#define ALG_SM1_MAC8_NOPAD      	  			1
#define ALG_SM1_MAC8_ISO9797_M1       			2
#define ALG_SM1_MAC8_ISO9797_M2       			3
#define ALG_SM1_MAC16_NOPAD           			4
#define ALG_SM1_MAC16_ISO9797_M1      			5
#define ALG_SM1_MAC16_ISO9797_M2      			6


/***********************************************************************************
 * Cipher Type, these definitions are comply with JAVA layer definitions  new standard
 ***********************************************************************************/
#define ALG_SM1_CBC_NOPAD_NEW					0x01
#define ALG_SM1_CBC_ISO9797_M1_NEW				0x02
#define ALG_SM1_CBC_ISO9797_M2_NEW				0x03
#define ALG_SM1_CBC_PKCS5_NEW					0x04
#define ALG_SM1_ECB_NOPAD_NEW					0x05
#define ALG_SM1_ECB_ISO9797_M1_NEW				0x06
#define ALG_SM1_ECB_ISO9797_M2_NEW				0x07
#define ALG_SM1_ECB_PKCS5_NEW					0x08

/***********************************************************************************
 * Signature Type, these definitions are comply with JAVA layer definitions new standard
 ***********************************************************************************/
#define ALG_SM1_MAC16_ISO9797_M1_NEW			0x23
#define ALG_SM1_MAC16_ISO9797_M2_NEW			0x24
#define ALG_SM1_MAC16_NOPAD_NEW					0x22
#define ALG_SM1_MAC16_PKCS5_NEW					0x29
#define ALG_SM1_MAC4_ISO9797_M1_NEW				0x03
#define ALG_SM1_MAC4_ISO9797_M2_NEW				0x05
#define ALG_SM1_MAC4_NOPAD_NEW					0x01
#define ALG_SM1_MAC4_PKCS5_NEW					0x07
#define ALG_SM1_MAC8_ISO9797_M1_NEW				0x04
#define ALG_SM1_MAC8_ISO9797_M2_NEW				0x06
#define ALG_SM1_MAC8_NOPAD_NEW					0x02
#define ALG_SM1_MAC8_PKCS5_NEW					0x08

/***********************************************************************************
 * 						ALG_SSF33
 ***********************************************************************************/
#define ALG_SSF33								0x06

/***********************************************************************************
 * SBC processing state
 ***********************************************************************************/
#define SBC_INIT              					0x00
#define SBC_UPDATE            					0x5A

#define CIPHER_MODE             				0x00
#define SIGNATURE_MODE							0x80
#define SBC_VERIFY  							0x80
#define SBC_AEAD	  							0x40
#define SBC_VERIFY_MASK  						0x7F
#define SBC_TPYE_MASK	  						0x3F
#define SBC_SIGN    							0x00

/***********************************************************************************
 * SYM signature veify result
 ***********************************************************************************/
#define SBC_VERIFY_OK    						0x5A
#define SBC_VERIFY_FAIL  						0xA5
#define SBC_ENCRYPT_OK  						0xA7

/***********************************************************************************
 * For padding
 ***********************************************************************************/
#define WRONG_PADDING                   		0xFFFF

/***********************************************************************************
 * Mode definations, these definations are comply with JAVA layer definations
 ***********************************************************************************/

#define MODE_DECRYPT							0x01
#define MODE_ENCRYPT							0x02
#define MODE_EN_OR_DE_MASK						0x03
#define SYM_ECB_MODE							0x00	/**< \brief Use ECB mode */
#define SYM_CBC_MODE							0x04	/**< \brief Use CBC mode */
#define SYM_CBC_MAC_MODE						0x08	/**< \brief Use CBC-based MAC mode */

#define SYM_ECB_OR_CBC_MASK						0x04
#define SYM_CBC_MAC_MASK   						0x08
#define SYM_ECB_CBC_MAC_MASK					0x0C

#define	ECB_Mode      							SYM_ECB_MODE
#define	ECB_Encrypt								(SYM_ECB_MODE 		| MODE_ENCRYPT)
#define	ECB_Decrypt								(SYM_ECB_MODE 		| MODE_DECRYPT)

#define	CBC_Mode								SYM_CBC_MODE
#define CBC_Encrypt								(SYM_CBC_MODE 		| MODE_ENCRYPT)
#define CBC_Decrypt								(SYM_CBC_MODE 		| MODE_DECRYPT)

#define CBC_MAC_Encrypt							(SYM_CBC_MAC_MODE 	| MODE_ENCRYPT)

#define MODE_INTEGRITY_CHECK					0xA0
#define MODE_KEY_MASK							0x50

/***********************************************************************************
 * Constant Definitions
 ***********************************************************************************/

#if WITH_AES
	#if WITH_SOFT_AES
		#define SOFT_AES_EXT_KEY_BYTE_LEN		0xF0
		#define AES_EXT_KEY_BYTE_LEN		(SOFT_AES_EXT_KEY_BYTE_LEN + LIB_SYM_WORKSPACE_SIZE)
	#else
		#define SOFT_AES_EXT_KEY_BYTE_LEN	BYTE_LENGTH_AES_256
		#define AES_EXT_KEY_BYTE_LEN		(SOFT_AES_EXT_KEY_BYTE_LEN + LIB_SYM_WORKSPACE_SIZE)
	#endif	//WITH_SOFT_AES

#endif //WITH_AES

#if WITH_SM4
	#if WITH_SOFT_SM4
		#define SM4_EXT_KEY_BYTE_LEN		0x80
	#else
		#define SM4_EXT_KEY_BYTE_LEN		(BYTE_LENGTH_SM4_256 + LIB_SYM_WORKSPACE_SIZE)
	#endif	//WITH_SOFT_SM4

#endif //WITH_SM4

#if WITH_SM1
	#if WITH_SOFT_SM1
		#define SM1_EXT_KEY_BYTE_LEN		0x80
	#else
		#define SM1_EXT_KEY_BYTE_LEN		(BYTE_LENGTH_SM1_256 + LIB_SYM_WORKSPACE_SIZE)
	#endif	//WITH_SOFT_SM1

#endif //WITH_SM1

#if WITH_KOREANSEED
	#if WITH_SOFT_KOREANSEED
		#define KOREANSEED_EXT_KEY_BYTE_LEN	BYTE_LENGTH_KOREANSEED_256
	#else
		#define KOREANSEED_EXT_KEY_BYTE_LEN	(BYTE_LENGTH_KOREANSEED_256 + LIB_SYM_WORKSPACE_SIZE)
	#endif	//WITH_SOFT_KOREANSEED
#endif //WITH_SM4

#if WITH_DES
	#if WITH_SOFT_DES
		#define DES_EXT_KEY_BYTE_LEN		BYTE_LENGTH_3DES_3KEY
	#else
		#define DES_EXT_KEY_BYTE_LEN		(BYTE_LENGTH_3DES_3KEY + LIB_SYM_WORKSPACE_SIZE)
	#endif	//WITH_SOFT_KOREANSEED
#endif

#if WITH_AES
	#define SYM_BLOCKSIZE					AES_BLOCKSIZE
#elif WITH_SM4
	#define SYM_BLOCKSIZE					SM4_BLOCKSIZE
#elif WITH_KOREANSEED
	#define SYM_BLOCKSIZE					KOREANSEED_BLOCKSIZE
#else
	#define SYM_BLOCKSIZE					DES_BLOCKSIZE
#endif

#if (WITH_AES & WITH_SM4)
	#if	(AES_EXT_KEY_BYTE_LEN > SM4_EXT_KEY_BYTE_LEN)
		#define SYM_EXT_KEY_BYTE_LEN		AES_EXT_KEY_BYTE_LEN
	#else
		#define SYM_EXT_KEY_BYTE_LEN		SM4_EXT_KEY_BYTE_LEN
	#endif
#elif WITH_AES
	#define SYM_EXT_KEY_BYTE_LEN			AES_EXT_KEY_BYTE_LEN
#elif WITH_SM4
	#define SYM_EXT_KEY_BYTE_LEN			SM4_EXT_KEY_BYTE_LEN
#elif WITH_KOREANSEED
	#define SYM_EXT_KEY_BYTE_LEN			KOREANSEED_EXT_KEY_BYTE_LEN
#else
	#define SYM_EXT_KEY_BYTE_LEN			DES_EXT_KEY_BYTE_LEN
#endif

/***********************************************************************************
 *                                HASH definations
 ***********************************************************************************/
#define HASH_INIT              					0x00
#define HASH_UPDATE								0x5A

/***********************************************************************************
 *                                SHA_1	 definations
 ***********************************************************************************/
#define ALG_SHA_1								0x01
#define SHA1_ICV_SIZE							0x14
#define SHA1_DIGEST_SIZE						0x14
#define SHA1_BLOCK_SIZE							0x40
#define SHA1_BLOCKNUM_SHIFT_BIT					0x06
#define SHA1_MESSAGE_BIT_BYTELENGTH				0x08

/***********************************************************************************
 *                                MD definations
 ***********************************************************************************/
#define ALG_MD									0x02
#define MD_ICV_SIZE								0x10
#define MD_DIGEST_SIZE							0x10
#define MD_BLOCK_SIZE							0x40
#define MD_BLOCKNUM_SHIFT_BIT					0x06
#define MD_MESSAGE_BIT_BYTELENGTH				0x08

/***********************************************************************************
 *                                RIPEMD160 definations
 ***********************************************************************************/
#define ALG_RIPEMD160							0x03
#define RIPEMD160_ICV_SIZE						0x14
#define RIPEMD160_DIGEST_SIZE					0x14
#define RIPEMD160_BLOCK_SIZE					0x40
#define RIPEMD160_BLOCKNUM_SHIFT_BIT			0x06
#define RIPEMD160_MESSAGE_BIT_BYTELENGTH		0x08

/***********************************************************************************
 *                                HASH256 definations
 ***********************************************************************************/
#define ALG_SHA_256								0x04
#define SHA256_ICV_SIZE							0x20
#define SHA256_DIGEST_SIZE						0x20
#define SHA256_BLOCK_SIZE						0x40
#define SHA256_BLOCKNUM_SHIFT_BIT				0x06
#define SHA256_MESSAGE_BIT_BYTELENGTH			0x08

/***********************************************************************************
 *                                SHA_384 definations
 ***********************************************************************************/
#define ALG_SHA_384								0x05
#define SHA384_ICV_SIZE							0x40
#define SHA384_DIGEST_SIZE						0x30
#define SHA384_BLOCK_SIZE						0x80
#define SHA384_BLOCKNUM_SHIFT_BIT				0x07
#define SHA384_MESSAGE_BIT_BYTELENGTH			0x10

/***********************************************************************************
 *                                SHA512 definations
 ***********************************************************************************/
#define ALG_SHA_512								0x06
#define SHA512_ICV_SIZE							0x40
#define SHA512_DIGEST_SIZE						0x40
#define SHA512_BLOCK_SIZE						0x80
#define SHA512_BLOCKNUM_SHIFT_BIT				0x07
#define SHA512_MESSAGE_BIT_BYTELENGTH			0x10

/***********************************************************************************
 *                                SHA_224	definations
 ***********************************************************************************/
#define ALG_SHA_224								0x07
#define SHA224_ICV_SIZE							0x20
#define SHA224_DIGEST_SIZE						0x1C
#define SHA224_BLOCK_SIZE						0x40
#define SHA224_BLOCKNUM_SHIFT_BIT				0x06
#define SHA224_MESSAGE_BIT_BYTELENGTH			0x08

/***********************************************************************************
 *                                WITH_SM3 definations
 ***********************************************************************************/
#define ALG_SM3									0x08
#define SM3_ICV_SIZE							0x20
#define SM3_DIGEST_SIZE							0x20
#define SM3_BLOCK_SIZE							0x40
#define SM3_BLOCKNUM_SHIFT_BIT					0x06
#define SM3_MESSAGE_BIT_BYTELENGTH				0x08

#if WITH_SHA512

#define HASH_ICV_SIZE							SHA512_ICV_SIZE
#define HASH_BLOCK_SIZE							SHA512_BLOCK_SIZE

#elif WITH_SHA384

#define HASH_ICV_SIZE							SHA384_ICV_SIZE
#define HASH_BLOCK_SIZE							SHA384_BLOCK_SIZE

#elif WITH_SHA256

#define HASH_ICV_SIZE							SHA256_ICV_SIZE
#define HASH_BLOCK_SIZE							SHA256_BLOCK_SIZE

#elif WITH_SHA224

#define HASH_ICV_SIZE							SHA224_ICV_SIZE
#define HASH_BLOCK_SIZE							SHA224_BLOCK_SIZE

#elif WITH_SM3

#define HASH_ICV_SIZE							SM3_ICV_SIZE
#define HASH_BLOCK_SIZE							SM3_BLOCK_SIZE

#elif WITH_SHA1

#define HASH_ICV_SIZE							SHA1_ICV_SIZE
#define HASH_BLOCK_SIZE							SHA1_BLOCK_SIZE

#elif WITH_RIPEMD160

#define HASH_ICV_SIZE							RIPEMD160_ICV_SIZE
#define HASH_BLOCK_SIZE							RIPEMD160_BLOCK_SIZE

#elif WITH_MD5

#define HASH_ICV_SIZE							MD_ICV_SIZE
#define HASH_BLOCK_SIZE							MD_BLOCK_SIZE

#endif


#define  RSA_CRT_PINV							0x01
#define  RSA_CRT_QINV							0x02

/***********************************************************************************
 * RSA processing state
 ***********************************************************************************/
#define RSA_INIT              					0x00
#define RSA_UPDATE            					0x5A
/***********************************************************************************
 * RSA signature mode according the definition of Java
 ***********************************************************************************/
#define RSA_SIGN              					0x01
#define RSA_VERIFY            					0x02

/***********************************************************************************
* Define RSA key type according the definition of Java(KeyBuilder.java).
***********************************************************************************/
#define ALG_RSA									0x01
#define ALG_RSA_CRT								0x02

/***********************************************************************************
* Define all RSA Processing Result Code
***********************************************************************************/

#define RSA_PROCESS_OK							0x5A
#define RSA_PROCESS_NOT_OK						0x00
#define RSA_DATA_DIFFERENT						0x01
#define RSA_VERIFICATION_FAIL					0x02
#define	RSA_INTERNAL_ERROR						0x03
#define	RSA_NO_SUCH_ALGORITHM					0x04
#define	RSA_WRONG_KEY_TYPE						0x05
#define RSA_WRONG_PADDING						0x06
#define RSA_CIPHER_ILLEGAL_USE      			0xFFA5

#define RSA_JAVA_API_GEN_KEY					0x5A
#define RSA_C_NATIVE_GEN_KEY					0xA5

/***********************************************************************************
* 										SIGNMR
***********************************************************************************/
#define MSG_RECOVER_FAIL            			0xFF5A
#define MSG_REVOVER_OK              			0x00
#define MR_VERIFY_FAIL              			0x00
#define MR_VERIFY_SUCCESS           			0x01

/***********************************************************************************
 * Mode definations, these definations are comply with JAVA layer definations
 ***********************************************************************************/
#define MODE_SIGN								1
#define MODE_VERIFY			        			2

/***********************************************************************************
 * Cipher algrithm definations according to JAVA layer definations
 ***********************************************************************************/
#define ALG_RSA_NOPAD  							0x0C
#define ALG_RSA_PKCS1  							0x0A
#define ALG_RSA_PKCS1_OAEP          			0x0F//added by jiangxiaohui@20220525 

#define ALG_DES_MAC4_NOPAD 						1
#define ALG_DES_MAC8_NOPAD  					2
#define ALG_DES_MAC4_ISO9797_M1  				3
#define ALG_DES_MAC8_ISO9797_M1  				4
#define ALG_DES_MAC4_ISO9797_M2  				5
#define ALG_DES_MAC8_ISO9797_M2  				6
#define ALG_DES_MAC4_PKCS5  					7
#define ALG_DES_MAC8_PKCS5  					8
#define ALG_RSA_SHA_ISO9796  					9
#define ALG_RSA_SHA_PKCS1  						10
#define ALG_RSA_MD5_PKCS1  						11
#define ALG_RSA_RIPEMD160_ISO9796  				12
#define ALG_RSA_RIPEMD160_PKCS1  				13
#define ALG_DSA_SHA  							14
#define ALG_RSA_SHA_RFC2409  					15
#define ALG_RSA_MD5_RFC2409  					16
#define ALG_ECDSA_SHA  							17
#define ALG_AES_MAC_128_NOPAD  					18
#define ALG_DES_MAC4_ISO9797_1_M2_ALG3  		19
#define ALG_DES_MAC8_ISO9797_1_M2_ALG3  		20
#define ALG_RSA_SHA_PKCS1_PSS  					21
#define ALG_RSA_MD5_PKCS1_PSS  					22
#define ALG_RSA_RIPEMD160_PKCS1_PSS  			23
#define ALG_KOREAN_SEED_MAC_NOPAD  				32
#define ALG_ECDSA_SHA_256  						33
#define ALG_ECDSA_SHA_384  						34
#define ALG_AES_MAC_192_NOPAD  					35
#define ALG_AES_MAC_256_NOPAD  					36
#define ALG_ECDSA_SHA_224  						37
#define ALG_ECDSA_SHA_512  						38
#define ALG_RSA_SHA_224_PKCS1  					39
#define ALG_RSA_SHA_256_PKCS1  					40
#define ALG_RSA_SHA_384_PKCS1  					41
#define ALG_RSA_SHA_512_PKCS1  					42
#define ALG_RSA_SHA_224_PKCS1_PSS  				43
#define ALG_RSA_SHA_256_PKCS1_PSS  				44
#define ALG_RSA_SHA_384_PKCS1_PSS  				45
#define ALG_RSA_SHA_512_PKCS1_PSS  				46
#define ALG_DES_CMAC8  							0x7A

/***********************************************************************************
 * Signature algorithm definations according to JAVA layer definations
 ***********************************************************************************/
#define ALG_RSA_SHA_ISO9796_MR          		0x1E
#define ALG_RSA_RIPEMD160_ISO9796_MR    		0x1F

//Cihper
#define	NOPAD_CIPHER_ENCRYPTION					0x2C
#define NOPAD_CIPHER_DECRYPTION					0x1C
#define	PKCS1_CIPHER_ENCRYPTION					0x2A
#define	PKCS1_CIPHER_DECRYPTION					0x1A
#define PKCS1_OAEP_CIPHER_DECRYPTION            0x1F //added by jiangxiaohui@20220525 
#define PKCS1_OAEP_CIPHER_ENCRYPTION            0x2F //added by jiangxiaohui@20220525 
//Signature
#define	ISO9796_SHA1_SIGN		        		0x49
#define	PKCS1_SHA1_SIGN			    	    	0x4A
#define	PKCS1_MD5_SIGN			     	    	0x4B
#define ISO9796_RIPEMD160_SIGN              	0x4C
#define PKCS1_RIPEMD160_SIGN                	0x4D
#define PKCS1_PSS_SHA1_SIGN               		0x55
#define PKCS1_PSS_SHA224_SIGN                   0x6B
#define PKCS1_PSS_SHA256_SIGN                   0x6C
#define PKCS1_PSS_SHA384_SIGN                   0x6D //added by jiangxiaohui@20220525 
#define PKCS1_PSS_SHA512_SIGN                   0x6E //added by jiangxiaohui@20220525 
#define PKCS1_SHA224_SIGN                   	0x67
#define PKCS1_SHA256_SIGN                  		0x68
#define PKCS1_SHA384_SIGN                   	0x69
#define PKCS1_SHA512_SIGN                   	0x6A
#define	ISO9796_SHA1_VERIFICATION	        	0x89
#define	PKCS1_SHA1_VERIFICATION		        	0x8A
#define	PKCS1_MD5_VERIFICATION		        	0x8B
#define ISO9796_RIPEMD160_VERIFICATION      	0x8C
#define PKCS1_RIPEMD160_VERIFICATION        	0x8D
#define PKCS1_PSS_SHA1_VERIFICATION             0x95
#define PKCS1_PSS_SHA224_VERIFICATION           0xAB
#define PKCS1_PSS_SHA256_VERIFICATION           0xAC
#define PKCS1_PSS_SHA384_VERIFICATION           0xAD //added by jiangxiaohui@20220525 
#define PKCS1_PSS_SHA512_VERIFICATION           0xAE //added by jiangxiaohui@20220525 
#define PKCS1_SHA224_VERIFICATION           	0xA7
#define PKCS1_SHA256_VERIFICATION           	0xA8
#define PKCS1_SHA384_VERIFICATION           	0xA9
#define PKCS1_SHA512_VERIFICATION           	0xAA
#if WITH_PKCS1_OAEP_PADDING                      //added by jiangxiaohui@20220525 
	#define PKCS1_OAEP_SHA1_SIGN									0xB1
	#define PKCS1_OAEP_SHA224_SIGN								0xB2
	#define PKCS1_OAEP_SHA256_SIGN								0xB3
	#define PKCS1_OAEP_SHA384_SIGN								0xB4
	#define PKCS1_OAEP_SHA512_SIGN								0xB5
	#define PKCS1_OAEP_SHA1_VERIFICATION					0xA1	
	#define PKCS1_OAEP_SHA224_VERIFICATION				0xA2		
	#define PKCS1_OAEP_SHA256_VERIFICATION				0xA3
	#define PKCS1_OAEP_SHA384_VERIFICATION				0xA4
	#define PKCS1_OAEP_SHA512_VERIFICATION				0xA6
#endif

/***********************************************************************************
 * RSA signature algrithm type mask
 ***********************************************************************************/
#define RSA_SIGN_ALG_TYPE_MASK      			0x3F


/***********************************************************************************
 * Define Hash Digest Information Length defined in PKCS1
 ***********************************************************************************/
#define LENGTH_DIGESTINFO_MD5					0x12
#define LENGTH_DIGESTINFO_RIPEMD160 			0x0F
#define LENGTH_DIGESTINFO_SHA1      			0x0F
#define LENGTH_DIGESTINFO_SHA224    			0x13
#define LENGTH_DIGESTINFO_SHA256    			0x13
#define LENGTH_DIGESTINFO_SHA384    			0x13
#define LENGTH_DIGESTINFO_SHA512    			0x13

#define PKCS1_RSA_PRIV_KEY_TAG_PAD  			0x01
#define PKCS1_RSA_PUB_KEY_TAG_PAD   			0x02

#define PKCS1_ZERO_PAD_BYTE_REPACE  			0x55
#define PKCS1_PAD_BYTE_FOR_PRIV_KEY 			0xFF


/***********************************************************************************
 * Define all pad types
 ***********************************************************************************/
#define PKCS1_PAD								0x01
#define PKCS1_DEPAD								0x02
#define ISO9796_PAD                 			0x03
#define ISO9796_DEPAD               			0x04
#define PKCS1_PSS_PAD                           0x05
#define PKCS1_PSS_DEPAD                         0x06
#define PKCS1_OAEP_PAD													0x07 //added by jiangxiaohui@20220525 
#define PKCS1_OAEP_DEPAD												0x08 //added by jiangxiaohui@20220525 
/***********************************************************************************
 * Define RSA key type according the definition of Java(KeyBuilder.java).
 ***********************************************************************************/
#define TYPE_RSA_PUBLIC		    				0x04
#define TYPE_RSA_PRIVATE	    				0x05
#define TYPE_RSA_CRT_PRIVATE	    			0x06


/***********************************************************************************
 * 					HMAC type
 ***********************************************************************************/
#define NO_SUCH_ALG          					0x00
#define ALG_HMAC_SHA1        					0x18
#define ALG_HMAC_SHA_256     					0x19
#define ALG_HMAC_SHA_384     					0x1A
#define ALG_HMAC_SHA_512     					0x1B
#define ALG_HMAC_MD5         					0x1C
#define ALG_HMAC_RIPEMD160   					0x1D
#define ALG_HMAC_SM3   							0x31


/***********************************************************************************
 * 						SM2 param length define
 ***********************************************************************************/
#define SM2_DOMAIN_NLEN  						0x20
#define SM2_DOMAIN_NLEN_MASK  					0x1F
#define SM2_DOMAIN_PLEN  						SM2_DOMAIN_NLEN
#define SM2_DOMAIN_ALEN  						SM2_DOMAIN_NLEN
#define SM2_DOMAIN_BLEN  						SM2_DOMAIN_NLEN
#define SM2_DOMAIN_POINTXY_LEN  				(SM2_DOMAIN_NLEN * 2)
#define SM2_DOMAIN_GLEN  						SM2_DOMAIN_POINTXY_LEN
#define SM2_DOMAIN_A_B_GXY_LEN					(SM2_DOMAIN_ALEN + SM2_DOMAIN_BLEN + SM2_DOMAIN_GLEN)

// key length
#define SM2_PRIVEYLEN  							SM2_DOMAIN_NLEN
#define SM2_PUBLICKEYLEN  						SM2_DOMAIN_POINTXY_LEN

// other param length
#define SM2_ZALEN  								SM2_DOMAIN_NLEN
#define SM2_ELEN  								SM2_DOMAIN_NLEN
#define SM2_ZABUFFLEN_MINUS_IDLEN  				(0x02 + SM2_DOMAIN_A_B_GXY_LEN + SM2_PUBLICKEYLEN)

//ZA = ENTLa||ID||a||b||G(x,y)||P(x,y), the max ID length is 32 bytes
#define	SM2_ZA_BUFFER_MAX_ID_LEN				SM2_DOMAIN_NLEN
#define SM2_ZABUFFLEN  							(SM2_ZABUFFLEN_MINUS_IDLEN + SM2_DOMAIN_NLEN)

#define SM2_CIPHER_PADDING_LEN  				(SM2_PUBLICKEYLEN + SM3_DIGEST_SIZE)


/***********************************************************************************
 * 						SM2 param length define
 ***********************************************************************************/
#define SM2_INIT              					0x00
#define SM2_SIGN              					0x01
#define SM2_VERIFY            					0x02
#define SM2DECRYPT			 					0x01
#define SM2ENCRYPT			 					0x02

// type 
//old
#define TYPE_SM2_PRIVATE 						0x04
#define TYPE_SM2_PUBLIC 						0x05
//new standard
#define TYPE_SM2_PRIVATE_NEW					0x8E
#define TYPE_SM2_PUBLIC_NEW						0x8D

#define SM2_VERIFY_SUCCESS			 			0x01
#define SM2_VERIFY_FAILED			 			0x00

/***********************************************************************************
 * CRC processing state
 ***********************************************************************************/
#define CRC_CONSTRUCTED             			0x00
#define CRC_INIT                    			0x5A
#define CRC_UPDATE                  			0xA5

/***********************************************************************************
 * CRC return value
 ***********************************************************************************/
#define CRC_PROCESS_OK              			0x5A81
#define CRC_PROCESS_FAIL            			0xCC56

/***********************************************************************************
 * CRC Type, these definitions are comply with	JAVA layer definitions
 ***********************************************************************************/
#define ALG_ISO3309_CRC16						1
#define ALG_ISO3309_CRC32						2

/***********************************************************************************
 *         random type
 ***********************************************************************************/
#define ALG_PSEUDO_RANDOM						0x01
#define	ALG_SECURE_RANDOM						0x02

/***********************************************************************************
 *        crc mode (crc16 or crc32)
 ***********************************************************************************/
#define CRC_MODE_JC2							0x01
#define	CRC_MODE_JC3							0x02

/***********************************************************************************
 *  The following switch can not be changed
 ***********************************************************************************/
#if ( !defined WITH_MD5 || WITH_MD5 == 0 )
	#define WITH_SOFT_MD5					0
#endif //WITH_MD5									 

#if ( !defined WITH_RIPEMD160 || WITH_RIPEMD160 == 0 )
	#define WITH_SOFT_RIPEMD160				0
#endif //WITH_RIPEMD160

#if ( !defined WITH_SHA1 || WITH_SHA1 == 0 )
	#define WITH_SOFT_SHA1					0
#endif //WITH_SHA1

#if ( !defined WITH_SHA224 || WITH_SHA224 == 0 )
	#define WITH_SOFT_SHA224				0
#endif //WITH_SHA224

#if ( !defined WITH_SHA256 || WITH_SHA256 == 0 )
	#define WITH_SOFT_SHA256				0
#endif //WITH_SHA256

#if ( !defined WITH_SHA384 || WITH_SHA384 == 0 )
	#define WITH_SOFT_SHA384				0
#endif //WITH_SHA384

#if ( !defined WITH_SHA512 || WITH_SHA512 == 0 )
	#define WITH_SOFT_SHA512				0
#endif //WITH_SHA512

#if ( !defined WITH_SM3 || WITH_SM3 == 0 )
	#define WITH_SOFT_SM3					0
#endif //WITH_SM3

#if ( !defined WITH_DES || WITH_DES == 0 ) 
	#define WITH_SOFT_DES					0
#endif //WITH_DES

#if ( !defined WITH_AES || WITH_AES == 0 ) 
	#define WITH_SOFT_AES					0
#endif //WITH_AES

#if ( !defined WITH_KOREANSEED || WITH_KOREANSEED == 0 ) 
	#define WITH_SOFT_KOREANSEED			0
#endif //WITH_KOREANSEED

#if ( !defined WITH_SM4 || WITH_SM4 == 0 ) 
	#define WITH_SOFT_SM4					0
#endif //WITH_SM4 

#if ( !defined WITH_CRC || WITH_CRC == 0 ) 
	#define WITH_SOFT_CRC						0
#endif
#endif /* CRYPTO_CONSTANT_H_ */
