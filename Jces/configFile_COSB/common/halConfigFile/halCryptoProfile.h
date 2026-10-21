/*
 * halCryptoProfile.h
 *
 *  Created on: 2014年9月29日
 *      Author: jiangxiaohui
 */

#ifndef HALCRYPTOPROFILE_H_
#define HALCRYPTOPROFILE_H_

#include "SysPlatformConfigProfile.h"

#define RUN_IN_SIMULATOR 						HAL__PLATFORM__SIMULATOR

/*=========================================================================== */
/*	security features														  */
/*=========================================================================== */

#define WITH_GUOMI                  			1

#if WITH_GUOMI
	#define WITH_SM1			        		0
	#define WITH_SM2			        		1
	#define WITH_SM3			        		1
	#define WITH_SM4			        		1
	#define WITH_SM7			        		1
	#define WITH_SSF33			        		0
#else
	#define WITH_SM1			        		0
	#define WITH_SM2			        		0
	#define WITH_SM3			        		0
	#define WITH_SM4			        		0
	#define WITH_SM7			        		0
	#define WITH_SSF33			        		0
#endif //WITH_GUOMI

#if WITH_SM2
	#if RUN_IN_SIMULATOR
		#define WITH_SOFT_SM2               	1
	#else
		#define WITH_SOFT_SM2               	1
	#endif
#endif //WITH_RSA

#define WITH_MD5     							1
#define WITH_RIPEMD160              			0
#define WITH_SHA1    							1
#define WITH_SHA224                 			1
#define WITH_SHA256                 			1
#define WITH_SHA384                 			1
#define WITH_SHA512                 			1

#define WITH_HASH 	(WITH_MD5 || WITH_RIPEMD160 || WITH_SHA1 || WITH_SHA224 || WITH_SHA256 || WITH_SHA384 || WITH_SHA512 || WITH_SM3)

#if WITH_HASH

	#define WITH_MD_INIT						1
	#define WITH_HMAC                   		1

	#if WITH_MD5
		#define WITH_SOFT_MD5					1
	#else
		#define WITH_SOFT_MD5					1
	#endif //WITH_MD5

	#if WITH_RIPEMD160
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_RIPEMD160			1
		#else
			#define WITH_SOFT_RIPEMD160			1
		#endif
	#endif //WITH_RIPEMD160

	#if WITH_SHA1
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SHA1				1
		#else
			#define WITH_SOFT_SHA1				1
		#endif
	#endif //WITH_SHA1

	#if WITH_SHA224
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SHA224			1
		#else
			#define WITH_SOFT_SHA224			1
		#endif
	#endif //WITH_SHA224

	#if WITH_SHA256
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SHA256			1
		#else
			#define WITH_SOFT_SHA256			1
		#endif
	#endif //WITH_SHA256

	#if WITH_SHA384
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SHA384			1
		#else
			#define WITH_SOFT_SHA384			1
		#endif
	#endif //WITH_SHA384

	#if WITH_SHA512
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SHA512			1
		#else
			#define WITH_SOFT_SHA512			1
		#endif
	#endif //WITH_SHA512

	#if WITH_SM3
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SM3				1
		#else
			#define WITH_SOFT_SM3				1
		#endif
	#endif //WITH_SM3

#endif //WITH_HASH

#define WITH_DES     							1
#define WITH_AES     							1
#define WITH_KOREANSEED							0

#define WITH_SYM 	(WITH_DES || WITH_AES || WITH_KOREANSEED || WITH_SM1 || WITH_SM4 || WITH_SSF33)

#if WITH_SYM

	#if WITH_DES
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_DES				1
		#else
			#define WITH_SOFT_DES				0
		#endif
	#endif //WITH_DES

	#if WITH_AES
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_AES				1
		#else
			#define WITH_SOFT_AES				0
		#endif

		#if WITH_SOFT_AES
			#define SOFT_AES_DECRYPTO			1
		#else
			#define SOFT_AES_DECRYPTO			0
		#endif
	#endif //WITH_AES

	#if WITH_KOREANSEED
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_KOREANSEED		1
		#else
			#define WITH_SOFT_KOREANSEED		0
		#endif
	#endif //WITH_KOREANSEED

	#if WITH_SM4
		#if RUN_IN_SIMULATOR
			#define WITH_SOFT_SM4				1
		#else
			#define WITH_SOFT_SM4				1
		#endif
	#endif //WITH_SM4

#endif //WITH_SYM

#define WITH_AEAD                               1

#define WITH_RSA     							1

#if WITH_RSA
	#define WITH_RSA_KEY_GEN               		1
	#define WITH_SIGNMR                 		0
	#define WITH_RSA_KEY_DEAL              		0
	#define WITH_PKCS1_OAEP_PADDING             1 //added by jiangxiaohui@20220525 
    #define WITH_PKCS1_PSS_PADDING              1 //added by jiangxiaohui@20220525 
#else
	#define WITH_RSA_KEY_GEN               		0
	#define WITH_SIGNMR                 		0
	#define WITH_RSA_KEY_DEAL              		0
#endif //WITH_RSA

#define WITH_CRC     							1

#if WITH_CRC
	#define WITH_CRC_32							1
	#define WITH_CRC_16							1
#endif //WITH_CRC
#if WITH_CRC
	#if RUN_IN_SIMULATOR
		#define WITH_SOFT_CRC					1
	    #define WITH_CHECKSUM						0
	#else
		#define WITH_SOFT_CRC					1
	#endif
	#define WITH_CHECKSUM						0
#endif //WITH_CRC

#define WITH_RANDOMDATA 						1

#define WITH_ECC                    			1

#if WITH_ECC
	#define WITH_ECC_KEY_512                    1 // added by xuxiaojun@20220830: 由于nativeECCSignatureDoFinalCal函数定义局部变量时使用MAX_ECC_KEY_BYTELEN，消耗的栈空间比较大，所以用开关WITH_ECC_KEY_512和WITH_ECC_KEY_521对所支持ECC的最大长度进行区分，在产品对这两个没有需求，且RAM空间不够的情况下，可以关闭这两个开关
	#define WITH_ECC_KEY_521                    1 // added by xuxiaojun@20220830: 由于nativeECCSignatureDoFinalCal函数定义局部变量时使用MAX_ECC_KEY_BYTELEN，消耗的栈空间比较大，所以用开关WITH_ECC_KEY_512和WITH_ECC_KEY_521对所支持ECC的最大长度进行区分，在产品对这两个没有需求，且RAM空间不够的情况下，可以关闭这两个开关
#endif

#endif /* HALCRYPTOPROFILE_H_ */
