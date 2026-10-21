/*
 * Crypto_SoftSHA384.h
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTSHA384_H_
#define CRYPTO_SOFTSHA384_H_

#include "HASH_API.h"
#include "Crypto_SoftSHA512.h"

#if WITH_SHA384

//extern void SHA384Transform(PX32 blockBuffer, PX32 icvBuffer);
#define  SHA384Transform(blockBuffer, icvBuffer) 		SHA512Transform(blockBuffer, icvBuffer)

#endif /* WITH_SHA384 */

#endif /* CRYPTO_SOFTSHA384_H_ */
