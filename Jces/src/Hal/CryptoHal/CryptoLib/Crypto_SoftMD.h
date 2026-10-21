/*
 * Crypto_SoftMD.h
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTMD_H_
#define CRYPTO_SOFTMD_H_

#include "HASH_API.h"

#if WITH_MD5

#if WITH_SOFT_MD5
extern void SoftMD_MDTransform(PX32 blockBuffer, PX32 icvBuffer);

#define MDTransform(blockBuffer, icvBuffer)                         SoftMD_MDTransform(blockBuffer, icvBuffer)
#else
extern void MDTransform(PX32 blockBuffer, PX32 icvBuffer);
#endif /* WITH_SOFT_MD5 */

#endif /* WITH_MD5 */

#endif /* CRYPTO_SOFTMD_H_ */
