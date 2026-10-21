/*
 * Crypto_softSHA224.c
 *
 *  Created on: 2014-2-25
 *      Author: jiangxiaohui
 */


#include "Crypto_softSHA224.h"
#include "Crypto_SoftSHA256.h"

#if WITH_SOFT_SHA224
	#define  SHA224Transform(blockBuffer, icvBuffer) 		SHA256Transform(blockBuffer, icvBuffer)
#else
	extern void SHA224Transform(PX32 blockBuffer, PX32 icvBuffer);
#endif
