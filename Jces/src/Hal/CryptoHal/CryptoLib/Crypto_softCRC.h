/*
 * Crypto_softCRC.h
 *
 *  Created on: 2014-3-3
 *      Author: jiangxiaohui
 */

#ifndef CRYPTO_SOFTCRC_H_
#define CRYPTO_SOFTCRC_H_


#include "CRC_API.h"

#if WITH_SOFT_CRC
/***********************************************************************************
 * CRC control switch
 ***********************************************************************************/
#define CRC16_BIT_CACULATE          		0
#define CRC16_LOOKUPTABLE_CACULATE  		0
#define CRC32_BIT_CACULATE          		0
#define CRC32_LOOKUPTABLE_CACULATE  		0

#if CRC16_BIT_CACULATE
extern u16 Calculate_CRC16_CCITT_BitCalculateMethod(u8 * inBuff, u16 inLen, u16 checkSum);
#endif	// CRC16_BIT_CACULATE

#if CRC16_LOOKUPTABLE_CACULATE
extern u16 Calculate_CRC16_CCITT_LookTableMethod(u8 * inBuff, u16 inLen, u16 checkSum);
#endif	// CRC16_LOOKUPTABLE_CACULATE

#if CRC32_BIT_CACULATE
extern u32 Calculate_CRC32_IEEE802_3_BitCalculateMethod(u8 * inBuff, u16 inLen, u32 checkSum, u8 mode);
#endif	//CRC32_BIT_CACULATE

#if CRC32_LOOKUPTABLE_CACULATE
extern u32 Calculate_CRC32_LookTableMethod(u8 * inBuff, u16 inLen, u32 checkSum, u8 mode);
#endif	//CRC32_LOOKUPTABLE_CACULATE

#endif //WITH_SOFT_CRC

#endif /* CRYPTO_SOFTCRC_H_ */
