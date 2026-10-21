/*
 * Crypto_SoftSHA1.h
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTSHA1_H_
#define CRYPTO_SOFTSHA1_H_

#include "HASH_API.h"

#if WITH_SHA1

#if WITH_SOFT_SHA1
extern void SoftSHA1_SHA1Transform(PX32 blockBuffer, PX32 icvBuffer);

#define SHA1Transform(blockBuffer, icvBuffer)                   SoftSHA1_SHA1Transform(blockBuffer, icvBuffer)
#else
extern void SHA1Transform(PX32 blockBuffer, PX32 icvBuffer);
#endif /* WITH_SOFT_SHA1 */

#endif /* WITH_SHA1 */

#endif /* CRYPTO_SOFTSHA1_H_ */
