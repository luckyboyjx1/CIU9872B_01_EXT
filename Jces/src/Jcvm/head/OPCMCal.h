/*
 * OPCMCal.h
 *
 *  Created on: 2013-10-15
 *      Author: jiangxiaohui
 */

#ifndef OPCMCAL_H_
#define OPCMCAL_H_

#include "profile.h"

#define 		ALG_RSA_SHA_PKCS1 				 10
#define 		ALG_RSA_MD5_PKCS1 				 11
#define 		ALG_RSA_RIPEMD160_PKCS1 		 13
#define 		ALG_DSA_SHA 					 14
#define 		ALG_RSA_SHA_RFC2409 			 15
#define 		ALG_RSA_MD5_RFC2409 			 16
#define 		ALG_ECDSA_SHA 					 17
#define 		ALG_AES_MAC_128_NOPAD 			 18
#define 		ALG_RSA_SHA_PKCS1_PSS 			 21
#define 		ALG_RSA_MD5_PKCS1_PSS 			 22
#define 		ALG_RSA_RIPEMD160_PKCS1_PSS 	 23
#define 		ALG_KOREAN_SEED_MAC_NOPAD 		 32
#define 		ALG_ECDSA_SHA_256 				 33
#define 		ALG_ECDSA_SHA_384 				 34
#define 		ALG_AES_MAC_192_NOPAD 			 35
#define 		ALG_AES_MAC_256_NOPAD 			 36
#define 		ALG_ECDSA_SHA_224 				 37
#define 		ALG_ECDSA_SHA_512 				 38
#define 		ALG_RSA_SHA_224_PKCS1 			 39
#define 		ALG_RSA_SHA_256_PKCS1 			 40
#define 		ALG_RSA_SHA_384_PKCS1 			 41
#define 		ALG_RSA_SHA_512_PKCS1 			 42
#define 		ALG_RSA_SHA_224_PKCS1_PSS 		 43
#define 		ALG_RSA_SHA_256_PKCS1_PSS 		 44
#define 		ALG_RSA_SHA_384_PKCS1_PSS 		 45
#define 		ALG_RSA_SHA_512_PKCS1_PSS 		 46


#define 		CMKEY_STATUS_DES				0x00
#define 		CMKEY_STATUS_RSA				0x01

#define 		SKSDAUTCRT_KEY_VERSION			0x72
#define			PKOCEAUT_KEY_VERSION			0x73
#define			PKTPEXAUT_KEY_VERSION			0x74
#define			PKSDAUTSF_KEY_VERSION			0x75


#define 		RECEIPTSTARSOFF					0x80

#define 		CONFIRMATION_COUNTER_LEN 		0x02


#define  		SD_AGENT_TOKEN_CAL				0x01
#define   		SD_AGENT_DAP_CAL   				0x02

#define         DES_TOEKN_LEN                   0x08
#define         AES_TOEKN_LEN                   0x10
#define         RSA_TOEKN_LEN                   0x80

#define         DES_RECEIPT_LEN                 0x08
#define         RSA_RECEIPT_LEN                 0x80

#define 		OP_ISSUER_BIN					0x42
#define 		OP_CARD_ISSUER_DATA				0x45


extern boolean	verifyDAP(PX8 sigDate, u16 SigOff, u16 SigLen, u8 sdIdForDap);
extern u8 		ReceiptGeneration(PX8 inBuffer,u16 inOffset, u16 inLength, u16 confirmationCounter);
extern boolean	verifyToken(PX8 buffer, u16 tokenoff, u16 tokenLen, boolean combined_LoadInstallandMakeSelectable, u8 lenTag);
#if !GP__VERSION__FOR_MINI_MASK
	extern boolean	decipheredLoadBlockData(void);
#else
	#define decipheredLoadBlockData()			0
#endif
extern memref 	getCMSDDataObj(u8 keyVersion);
extern memref 	getAssociatedSDTag(u16 tag, u8 len);

#if GP__TOKEN__FOR_MASK
	boolean verifyTokenAndReciptKey(void);
#else
	#define verifyTokenAndReciptKey()			true
#endif

#endif /* OPCMCAL_H_ */
