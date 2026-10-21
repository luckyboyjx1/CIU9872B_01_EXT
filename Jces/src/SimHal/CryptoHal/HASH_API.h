/*
 * HASH_API.h
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#ifndef HASH_API_H_
#define HASH_API_H_

#include "Crypto_Constant.h"
#include "Crypto_common_API.h"

#if WITH_HASH

/**
 * \brief Structure of parameters for function ::HASH_API_Param
 */
typedef struct HASH_API_Param {
	PM8		pICV;            		/**< \brief Pointer to initial value */
	u8		hashIVSize;				/**< \length of Current Crypto' Block*/
    u8		blockSize;          	/**< \length of Current Crypto' Block*/
    u8		blockNumShift; 			/**< \length of Current Crypto' Block mask*/
    u8		digestSize;				/**< \byte number in nulti block mask*/
    u8		msgBitByteLen;			/**< \byte number in nulti block mask*/
    u8		algType;				/**< \specific padding Type
									 *  bit 0:
									 *      - 1: only the last block is being Cal.
									 */
} HASH_API_Param_t;

/**
 * \brief Structure of parameters for function ::HASH_Init_Param
 */

typedef struct HASH_Init_Param {
	PM8		inDataBuffer;          	/**< \brief Pointer to initial value */
	PX8		hashWorkSpace;          /**< \brief Pointer to initial value */
	PX8		hashResult;          	/**< \length of Current Crypto' Block*/
    u16		blockNum; 				/**< \length of Current Crypto' Block mask*/
	u16		inDataLength;          	/**< \brief Pointer to initial value */
	u8		remainLength;          	/**< \brief Pointer to initial value */
    u8		algType;				/**< \specific padding Type*/
} HASH_Init_Param_t;

#if WITH_MD_INIT
extern boolean HASHInitAlg(PM8 initialDigestBuff, u16 initialDigestLength, PM8 digestedMsgLenBuff, u16 digestedMsgLenLength, PM8 mdBuff, u8 algType);
#endif
#if WITH_MD5
extern void HASH_MDInitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_MDUpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_MDFinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_MD */

#if WITH_RIPEMD160
extern void HASH_RIPEMD160InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_RIPEMD160UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_RIPEMD160FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_RIPEMD160 */

#if WITH_SHA1
extern void SHA1Transform(PX32 blockBuffer, PX32 icvBuffer);
extern void HASH_SHA1InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_SHA1UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_SHA1FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_SHA1 */

#if WITH_SHA224
extern void HASH_SHA224InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_SHA224UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_SHA224FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_SHA224 */

#if WITH_SHA256
extern void HASH_SHA256InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_SHA256UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_SHA256FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_SHA256 */

#if WITH_SHA384
extern void HASH_SHA384InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_SHA384UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_SHA384FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_SHA384 */

#if WITH_SHA512
extern void HASH_SHA512InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_SHA512UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_SHA512FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_SHA512 */

#if WITH_SM3
extern void HASH_SM3InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
extern void HASH_SM3UpdateMain(HASH_Init_Param_t * myInitParam);
extern void HASH_SM3FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam);
#endif /* WITH_SM3 */
#endif /* WITH_HASH */

#endif /* HASH_API_H_ */

