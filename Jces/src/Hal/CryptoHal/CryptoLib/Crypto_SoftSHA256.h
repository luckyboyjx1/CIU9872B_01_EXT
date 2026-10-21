/*
 * Crypto_SoftSHA256.h
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTSHA256_H_
#define CRYPTO_SOFTSHA256_H_

#include "HASH_API.h"

#if (WITH_SHA256 || WITH_SHA224)

#if (WITH_SOFT_SHA256 || WITH_SOFT_SHA224)
extern void SoftSHA256_SHA256Transform(PX32 blockBuffer, PX32 icvBuffer);

#define SHA256Transform(blockBuffer, icvBuffer)                     SoftSHA256_SHA256Transform(blockBuffer, icvBuffer)
#else
extern void SHA256Transform(PX32 blockBuffer, PX32 icvBuffer);
#endif /* (WITH_SOFT_SHA256 || WITH_SOFT_SHA224) */

#endif /* (WITH_SHA256 || WITH_SHA224) */

#endif /* CRYPTO_SOFTSHA256_H_ */
