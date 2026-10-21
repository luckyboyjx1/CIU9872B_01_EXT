/*
 * halCryptoConfig.h
 *
 *  Created on: 2014年9月29日
 *      Author: jiangxiaohui
 */

#ifndef HALCRYPTOCONFIG_H_
#define HALCRYPTOCONFIG_H_

#include "Crypto_Constant.h"
#include "halCryptoProfile.h"
#include "SysPlatformConfigProfile.h"


#if WITH_AES_PRIVATED_AUTH
		#define WITH_AES_PRIVATED_AUTH               1
		
#else
		#define WITH_AES_PRIVATED_AUTH               0
#endif

#if WITH_HASH

#if	(HASH_BLOCK_SIZE || HASH_ICV_SIZE)
	#if	((HASH_BLOCK_SIZE + HASH_ICV_SIZE) > LIB_HASH_WORKSPACE_SIZE)
		#define HASH_WORKSPACE_SIZE				(HASH_BLOCK_SIZE + HASH_ICV_SIZE)
	#else
		#define HASH_WORKSPACE_SIZE				LIB_HASH_WORKSPACE_SIZE
	#endif
#else
	#define HASH_WORKSPACE_SIZE					LIB_HASH_WORKSPACE_SIZE
#endif // (HASH_BLOCK_SIZE || HASH_ICV_SIZE)

#endif //WITH_HASH


#if WITH_RSA

	#define RSA_CRT_MODULAR_SAVE					0
	#define RSA_CRT_E_SAVE							1
#if HAL__PLATFORM__SIMULATOR
	/*Wirte RASKeyGen data to eeprom in hal*/
	#define RSA_KEYGEN_WIRTE_IN_HAL         		0  //SIMULATOR 模式 只能是0
#else
/*Wirte RASKeyGen data to eeprom in hal*/
	#define RSA_KEYGEN_WIRTE_IN_HAL         		1 //0 or  1
#endif

	#define RSA_USE_CRYPTO_RAM						1
	#define MAX_RSA_MUD_BIT_LEN						2048
	#define MAX_RSA_MUD_BYTE_LEN					(MAX_RSA_MUD_BIT_LEN >> 3)
	#define MAX_RSA_P_BYTE_LEN						(MAX_RSA_MUD_BYTE_LEN >> 1)
	#define MAX_RSA_E_BYTE_LEN						0x04

#if WITH_SAFETY_CRYPTO
	// HEAD 4 BYTE + FIRST 3 BYTE + LAST 3 BYTE
	// EEPROMBUFFER	+ TOP ADDRESS ALIGN + BOT ADDRESS ALIGN
	#if ((BIG_REUSE_BUFFER_SIZE - 0x0A) > (MAX_RSA_MUD_BYTE_LEN * 2))
		#define WITH_SAFETY_REUSED_BUFFER			1
	#else
		#define WITH_SAFETY_REUSED_BUFFER			0
	#endif
#endif

#endif //WITH_RSA

#endif /* HALCRYPTOCONFIG_H_ */
