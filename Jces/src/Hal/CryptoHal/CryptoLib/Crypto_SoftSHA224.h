/*
 * Crypto_SoftSHA224.h
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTSHA224_H_
#define CRYPTO_SOFTSHA224_H_

#include "HASH_API.h"
#include "Crypto_SoftSHA256.h"

#if WITH_SHA224

#if WITH_SOFT_SHA224
	#define  SHA224Transform(blockBuffer, icvBuffer) 		SHA256Transform(blockBuffer, icvBuffer)
#else
	extern void SHA224Transform(PX32 blockBuffer, PX32 icvBuffer);
#endif

#endif /* WITH_SHA224 */

#endif /* CRYPTO_SOFTSHA224_H_ */
