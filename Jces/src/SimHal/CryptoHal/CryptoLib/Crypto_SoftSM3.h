/*
 * Crypto_SoftSM3.h
 *
 *  Created on: 2014Äê4ÔÂ10ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFT_SM3_H_
#define CRYPTO_SOFT_SM3_H_


#include "HASH_API.h"

#if WITH_SM3

#if WITH_SOFT_SM3
extern void SoftSM3_SM3Transform(PX32 blockBuffer, PX32 icvBuffer);

#define SM3Transform(blockBuffer, icvBuffer)                   SoftSM3_SM3Transform(blockBuffer, icvBuffer)
#else
extern void SM3Transform(PX32 blockBuffer, PX32 icvBuffer);
#endif /* WITH_SOFT_SM3 */

#endif /* WITH_SM3 */

#endif /* CRYPTO_SOFT_SM3_H_ */
