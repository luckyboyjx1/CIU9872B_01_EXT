/*
 * HMAC_API.h
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#ifndef HMAC_API_H_
#define HMAC_API_H_

#include "bootConfigProfile.h"
#include "ecpDefine.h"
#include "halCryptoProfile.h"

#if WITH_HMAC
#define IPAD    0x36
#define OPAD    0x5C

/**
 * \brief Structure of parameters for function ::HMAC_API_Param
 */

typedef struct HMAC_API_Param
{
    u8		h_mac_digest_size;
    u8		h_mac_block_size;
    u8		h_mac_hash_type;
} HMAC_API_Param_t;

#endif   // WITH_HMAC

#endif /* HMAC_API_H_ */
