/*
 * Crypto_SoftSHA512.h
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTSHA512_H_
#define CRYPTO_SOFTSHA512_H_

#include "HASH_API.h"

#if (WITH_SHA512 || WITH_SHA384)

#if (WITH_SOFT_SHA512 || WITH_SOFT_SHA384)
extern void SoftSHA512_SHA512Transform(PX32 blockBuffer, PX32 icvBuffer);

#define SHA512Transform(blockBuffer, icvBuffer)                     SoftSHA512_SHA512Transform(blockBuffer, icvBuffer)
#else
extern void SHA512Transform(PX32 blockBuffer, PX32 icvBuffer);
#endif /* (WITH_SOFT_SHA512 || WITH_SOFT_SHA384) */

#endif /* (WITH_SHA512 || WITH_SHA384) */

#endif /* CRYPTO_SOFTSHA512_H_ */
