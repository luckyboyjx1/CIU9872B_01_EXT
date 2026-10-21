/************************************************************
 Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
 FileName: Crypto_ECC.c.
 Elliptic Curve Cryptography [IEEE P1363]
 The equation of the curves for keys of type FP
 is y^2 = x^3 + A * x + B.
 The equation of the curves for keys of type F2M
 is y^2 + x * y = x^3 + A * x^2 + B.
 A specific implementation need not
 support the compressed form, but must support the uncompressed form of the point.
 Author: Lizumeng.
 Version : 1.0.
 Date: 2010/04/20.
 Description: This source file contains functions used to encrypt or
 decrypt with ECC cryptography.
 History:
 <author>    <time>    <version>    <desc>
 Lizumeng   10/04/20     1.0     build this moudle
 *************************************************************/

#include "EllipseCurve_API.h"
#include "Crypto_API.h"
#include "Crypto_Constant.h"
#include "BootRam.h"
#include "SysConfig.h"
#include "MemOperation.h"
#include "SysConfigTarget.h"
#include "MemGetReuseRam.h"
#include "bootConfigMethod.h"
#include "SecureCrypto.h"
#include "RSA_API.h"
#include "error.h"
#include "anomaly.h"

#if WITH_ECC

#pragma ELLIPSE_CURVE_RAM_START

ECKeyRefStore g_ECKeyStore;

ELLIPSE_CURVE_RAM_END

/* Recommended elliptic curve domain parameters based on Fp */
/* all cofactors of recommended elliptic curves below are COFACTOR = 0x01 */

/**
 * The bit length for key is limited between 128 and 576 bits on NXP P60 platform.
 * So here the min key bit length we support is 128 bits (the security level is as RSA 512 bits).
 * The max bit length 384 is not supporte by now on this platform.
 * @author lizumeng
 static u8 code E112_FP[14] = {0xDB,0x7C,0x2A,0xBF,0x62,0xE3,0x5E,0x66,0x80,0x76,0xBE,0xAD,0x20,0x8B};
 static u8 code E112_A[14] = {0xDB,0x7C,0x2A,0xBF,0x62,0xE3,0x5E,0x66,0x80,0x76,0xBE,0xAD,0x20,0x88};
 static u8 code E112_B[14] = {0x65,0x9E,0xF8,0xBA,0x04,0x39,0x16,0xEE,0xDE,0x89,0x11,0x70,0x2B,0x22};
 static u8 code E112_Gxy[29] = {0x04,0x09,0x48,0x72,0x39,0x99,0x5A,0x5E,0xE7,0x6B,0x55,0xF9,0xC2,0xF0,0x98,0xA8,0x9C,0xE5,0xAF,0x87,0x24,0xC0,0xA2,0x3E,0x0E,0x0F,0xF7,0x75,0x00};
 static u8 code E112_N[14] = {0xDB,0x7C,0x2A,0xBF,0x62,0xE3,0x5E,0x76,0x28,0xDF,0xAC,0x65,0x61,0xC5};
 //the seed S: 00F50B02 8E4D696E 67687561 51752904 72783FB1
 */

#if WITH_ECC_KEY_128
#define EC128_FP_P_LEN   16
#define EC128_FP_R_LEN   16
u8 CONST E128_DOMAINPARAMS[] = {0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
	0xE8,0x75,0x79,0xC1,0x10,0x79,0xF4,0x3D,0xD8,0x24,0x99,0x3C,0x2C,0xEE,0x5E,0xD3,
	0x16,0x1F,0xF7,0x52,0x8B,0x89,0x9B,0x2D,0x0C,0x28,0x60,0x7C,0xA5,0x2C,0x5B,0x86,0xCF,0x5A,0xC8,0x39,0x5B,0xAF,0xEB,0x13,0xC0,0x2D,0xA2,0x92,0xDD,0xED,0x7A,0x83,
	0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x75,0xA3,0x0D,0x1B,0x90,0x38,0xA1,0x15};
#endif

/*
 static u8 code E128_FP[16] = {0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
 static u8 code E128_A[16] = {0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC};
 static u8 code E128_B[16] = {0xE8,0x75,0x79,0xC1,0x10,0x79,0xF4,0x3D,0xD8,0x24,0x99,0x3C,0x2C,0xEE,0x5E,0xD3};
 static u8 code E128_Gxy[33] = {0x04,0x16,0x1F,0xF7,0x52,0x8B,0x89,0x9B,0x2D,0x0C,0x28,0x60,0x7C,0xA5,0x2C,0x5B,0x86,0xCF,0x5A,0xC8,0x39,0x5B,0xAF,0xEB,0x13,0xC0,0x2D,0xA2,0x92,0xDD,0xED,0x7A,0x83};
 static u8 code E128_N[16] = {0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x75,0xA3,0x0D,0x1B,0x90,0x38,0xA1,0x15};
 */
//the seed S: 000E0D4D 696E6768 75615175 0CC03A44 73D03679
#if WITH_ECC_KEY_160
#define EC160_FP_P_LEN   20
#define EC160_FP_R_LEN   21
u8 CONST E160_DOMAINPARAMS[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xAC,0x73,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,
	0x3B,0x4C,0x38,0x2C,0xE3,0x7A,0xA1,0x92,0xA4,0x01,0x9E,0x76,0x30,0x36,0xF4,0xF5,0xDD,0x4D,0x7E,0xBB,0x93,0x8C,0xF9,0x35,0x31,0x8F,0xDC,0xED,0x6B,0xC2,0x82,0x86,0x53,0x17,0x33,0xC3,0xF0,0x3C,0x4F,0xEE,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xB8,0xFA,0x16,0xDF,0xAB,0x9A,0xCA,0x16,0xB6,0xB3};
#endif
/*
 static u8 code E160_FP[20] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xAC,0x73};
 static u8 code E160_A[20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 static u8 code E160_B[20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07};
 static u8 code E160_Gxy[41] = {0x04,0x3B,0x4C,0x38,0x2C,0xE3,0x7A,0xA1,0x92,0xA4,0x01,0x9E,0x76,0x30,0x36,0xF4,0xF5,0xDD,0x4D,0x7E,0xBB,0x93,0x8C,0xF9,0x35,0x31,0x8F,0xDC,0xED,0x6B,0xC2,0x82,0x86,0x53,0x17,0x33,0xC3,0xF0,0x3C,0x4F,0xEE};
 static u8 code E160_N[21] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xB8,0xFA,0x16,0xDF,0xAB,0x9A,0xCA,0x16,0xB6,0xB3};
 */
#if WITH_ECC_KEY_192
#define EC192_FP_P_LEN   24
#define EC192_FP_R_LEN   24
u8 CONST E192_DOMAINPARAMS[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xEE,0x37,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,
	0xDB,0x4F,0xF1,0x0E,0xC0,0x57,0xE9,0xAE,0x26,0xB0,0x7D,0x02,0x80,0xB7,0xF4,0x34,0x1D,0xA5,0xD1,0xB1,0xEA,0xE0,0x6C,0x7D,0x9B,0x2F,0x2F,0x6D,0x9C,0x56,0x28,0xA7,0x84,0x41,0x63,0xD0,0x15,0xBE,0x86,0x34,0x40,0x82,0xAA,0x88,0xD9,0x5E,0x2F,0x9D,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x26,0xF2,0xFC,0x17,0x0F,0x69,0x46,0x6A,0x74,0xDE,0xFD,0x8D};
#endif
/*
 static u8 code E192_FP[24] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xEE,0x37};
 static u8 code E192_A[24] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 static u8 code E192_B[24] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03};
 static u8 code E192_Gxy[49] = {0x04,0xDB,0x4F,0xF1,0x0E,0xC0,0x57,0xE9,0xAE,0x26,0xB0,0x7D,0x02,0x80,0xB7,0xF4,0x34,0x1D,0xA5,0xD1,0xB1,0xEA,0xE0,0x6C,0x7D,0x9B,0x2F,0x2F,0x6D,0x9C,0x56,0x28,0xA7,0x84,0x41,0x63,0xD0,0x15,0xBE,0x86,0x34,0x40,0x82,0xAA,0x88,0xD9,0x5E,0x2F,0x9D};
 static u8 code E192_N[24] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x26,0xF2,0xFC,0x17,0x0F,0x69,0x46,0x6A,0x74,0xDE,0xFD,0x8D};
 */
#if WITH_ECC_KEY_224
#define EC224_FP_P_LEN   28
#define EC224_FP_R_LEN   28
u8 CONST E224_DOMAINPARAMS[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
	0xB4,0x05,0x0A,0x85,0x0C,0x04,0xB3,0xAB,0xF5,0x41,0x32,0x56,0x50,0x44,0xB0,0xB7,0xD7,0xBF,0xD8,0xBA,0x27,0x0B,0x39,0x43,0x23,0x55,0xFF,0xB4,
	0xB7,0x0E,0x0C,0xBD,0x6B,0xB4,0xBF,0x7F,0x32,0x13,0x90,0xB9,0x4A,0x03,0xC1,0xD3,0x56,0xC2,0x11,0x22,0x34,0x32,0x80,0xD6,0x11,0x5C,0x1D,0x21,0xBD,0x37,0x63,0x88,0xB5,0xF7,0x23,0xFB,0x4C,0x22,0xDF,0xE6,0xCD,0x43,0x75,0xA0,0x5A,0x07,0x47,0x64,0x44,0xD5,0x81,0x99,0x85,0x00,0x7E,0x34,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x16,0xA2,0xE0,0xB8,0xF0,0x3E,0x13,0xDD,0x29,0x45,0x5C,0x5C,0x2A,0x3D};
#endif
/*
 static u8 code E224_FP[28] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
 static u8 code E224_A[28] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE};
 static u8 code E224_B[28] = {0xB4,0x05,0x0A,0x85,0x0C,0x04,0xB3,0xAB,0xF5,0x41,0x32,0x56,0x50,0x44,0xB0,0xB7,0xD7,0xBF,0xD8,0xBA,0x27,0x0B,0x39,0x43,0x23,0x55,0xFF,0xB4};
 static u8 code E224_Gxy[57] = {0x04,0xB7,0x0E,0x0C,0xBD,0x6B,0xB4,0xBF,0x7F,0x32,0x13,0x90,0xB9,0x4A,0x03,0xC1,0xD3,0x56,0xC2,0x11,0x22,0x34,0x32,0x80,0xD6,0x11,0x5C,0x1D,0x21,0xBD,0x37,0x63,0x88,0xB5,0xF7,0x23,0xFB,0x4C,0x22,0xDF,0xE6,0xCD,0x43,0x75,0xA0,0x5A,0x07,0x47,0x64,0x44,0xD5,0x81,0x99,0x85,0x00,0x7E,0x34};
 static u8 code E224_N[29] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x16,0xA2,0xE0,0xB8,0xF0,0x3E,0x13,0xDD,0x29,0x45,0x5C,0x5C,0x2A,0x3D};
 */
#if WITH_ECC_KEY_256
#define EC256_FP_P_LEN   32
#define EC256_FP_R_LEN   32
u8 CONST E256_DOMAINPARAMS[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFC,0x2F,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,
	0x79,0xBE,0x66,0x7E,0xF9,0xDC,0xBB,0xAC,0x55,0xA0,0x62,0x95,0xCE,0x87,0x0B,0x07,0x02,0x9B,0xFC,0xDB,0x2D,0xCE,0x28,0xD9,0x59,0xF2,0x81,0x5B,0x16,0xF8,0x17,0x98,0x48,0x3A,0xDA,0x77,0x26,0xA3,0xC4,0x65,0x5D,0xA4,0xFB,0xFC,0x0E,0x11,0x08,0xA8,0xFD,0x17,0xB4,0x48,0xA6,0x85,0x54,0x19,0x9C,0x47,0xD0,0x8F,0xFB,0x10,0xD4,0xB8,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x41};
#endif
	
#if WITH_ECC_KEY_384
#define EC384_FP_P_LEN   48
#define EC384_FP_R_LEN   48
u8 CONST E384_DOMAINPARAMS[] = {
	/*p*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
	/*a*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFC,
	/*b*/0xB3,0x31,0x2F,0xA7,0xE2,0x3E,0xE7,0xE4,0x98,0x8E,0x05,0x6B,0xE3,0xF8,0x2D,0x19,0x18,0x1D,0x9C,0x6E,0xFE,0x81,0x41,0x12,0x03,0x14,0x08,0x8F,0x50,0x13,0x87,0x5A,0xC6,0x56,0x39,0x8D,0x8A,0x2E,0xD1,0x9D,0x2A,0x85,0xC8,0xED,0xD3,0xEC,0x2A,0xEF,
	/*G*/0xAA,0x87,0xCA,0x22,0xBE,0x8B,0x05,0x37,0x8E,0xB1,0xC7,0x1E,0xF3,0x20,0xAD,0x74,0x6E,0x1D,0x3B,0x62,0x8B,0xA7,0x9B,0x98,0x59,0xF7,0x41,0xE0,0x82,0x54,0x2A,0x38,0x55,0x02,0xF2,0x5D,0xBF,0x55,0x29,0x6C,0x3A,0x54,0x5E,0x38,0x72,0x76,0x0A,0xB7,0x36,0x17,0xDE,0x4A,0x96,0x26,0x2C,0x6F,0x5D,0x9E,0x98,0xBF,0x92,0x92,0xDC,0x29,0xF8,0xF4,0x1D,0xBD,0x28,0x9A,0x14,0x7C,0xE9,0xDA,0x31,0x13,0xB5,0xF0,0xB8,0xC0,0x0A,0x60,0xB1,0xCE,0x1D,0x7E,0x81,0x9D,0x7A,0x43,0x1D,0x7C,0x90,0xEA,0x0E,0x5F,
	/*n*/0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC7,0x63,0x4D,0x81,0xF4,0x37,0x2D,0xDF,0x58,0x1A,0x0D,0xB2,0x48,0xB0,0xA7,0x7A,0xEC,0xEC,0x19,0x6A,0xCC,0xC5,0x29,0x73};
#endif
/****************************************************************************
 * Function name: getRealLength
 * Description: get the real length of big number, return with the first digit
 *              is not zero.
 * Input Parameters:
 *      u8 X_A * dataIn          address of data
 * 		u16 length               length of data
 * Output Parameters:		The processing result code
 *	    return the real length of data
 * NOTE:
 ****************************************************************************/
u16 getRealLength(u8 X_A * dataIn, u16 length)
{
	u16 X_A tmpLength, outLength = 0;
	u8 X_A * pData = dataIn;

	while((*pData == 0) && (outLength < length ))
	{
		pData++;
		outLength++;
	}

	if (outLength > 0)
	{
		length = (u16)(length - outLength);
		tmpLength = length;
		while (tmpLength--)
		{
			*dataIn++ = *pData++;
		}
	}
	return length;
}

//如果第一个字节>0x7F,前面加0x00,RealLength=length+1
u16 getRealLength2(u8 X_A * dataIn, u16 length)
{
	u8 X_A * pData = dataIn;

	if (*pData>0x7F)
	{
		ecpMem_Move(pData+1,pData,length);
		*pData=0x00;
		length=length+1;
	}
	return length;
}
signed char ECC_compare(unsigned char * buffA, unsigned char * buffB, unsigned short size)
{
	u8 x, y;

	while (size--)
	{
		x = *buffA;
		y = *buffB;

		if (x < y)
		{
			return 0xFF;
		}
		else if (x > y)
		{
			return 0x01;
		}
		buffA++;
		buffB++;
	}
	return 0x00;
}

/****************************************************************************
 * Function name: EncodeSignature
 * Description: This function is used to format the signature result (r,s)
 *    ------------------------------------------------------------------------
 *    | 0x30 | rLen + sLen + 4 | 0x02 | rLen | rValue | 0x02 | sLen | sValue |
 *    ------------------------------------------------------------------------
 * Input Parameters:
 *      u8 X_A * rSign			   signature data r
 *      u8 X_A * rSignLen           length of signature r
 *      u8 X_A * sSign              signature data s
 * 		u8 X_A* sSignLen 		   length of signature s
 * Output Parameters:
 *	    none.
 * NOTE:
 ****************************************************************************/
void EncodeSignature(u8 X_A * rSign, u8 X_A * rSignLen, u8 X_A * sSign, u8 X_A * sSignLen)
{
	u8 X_A * cryptBuff = g_CryptoData.pbBuffer;

	*(cryptBuff++) = 48;

	*(cryptBuff++) = (u8)((*rSignLen) + (*sSignLen) + 4);

	*(cryptBuff++) = 2;

	*(cryptBuff++) = (u8)(*rSignLen);

	ecpMem_Copy((PM8)cryptBuff, (PM8)rSign, (*rSignLen));

	cryptBuff += (*rSignLen);

	*(cryptBuff++) = 2;

	*(cryptBuff++) = (u8)(*sSignLen);

	ecpMem_Copy((PM8)cryptBuff, (PM8)sSign, (*sSignLen));
}

/****************************************************************************
 * Function name: DecodeSignature
 * Description: This function is used to obtain the real signature (r,s) from
 *              format signature
 * Input Parameters:
 *      u8 X_A * rSign			   signature data r
 *      u8 X_A * rSignLen           length of signature r
 *      u8 X_A * sSign              signature data s
 * 		u8 X_A * sSignLen 		   length of signature s
 *      u8 * pbSignAddr                    signature data
 *      u16 wSignLen                       length of signature
 * Output Parameters:		The processing result code
 *	    EC_VERIFY_OK			If decoding success
 *		EC_VERIFY_FAIL			If fail to decode
 * NOTE:
 * Econding signature result (r, s):
 *  ------------------------------------------------------------------------
 *  | 0x30 | rLen + sLen + 4 | 0x02 | rLen | rValue | 0x02 | sLen | sValue |
 *  ------------------------------------------------------------------------
 ****************************************************************************/
boolean DecodeSignature(u8 X_A * rSign, u8 X_A * rSignLen, u8 X_A * sSign, u8 X_A * sSignLen, u8 * pbSignAddr, u8 wSignLen)
{
	u8 X_A sequenceLength;
	u8 X_A i;
	u8 X_A tmpLen, orderLen = g_ECKeyStore.b_RLength;
	
//	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_001();
	
	if(nvmReadU8((PN8)pbSignAddr++) != 48)
	{
		return EC_VERIFY_FAIL;
	}

	sequenceLength = nvmReadU8((PN8)pbSignAddr++);
	if(wSignLen != (sequenceLength + 2))
	{

		return EC_VERIFY_FAIL;
	}

	if(nvmReadU8((PN8)pbSignAddr++) != 2)
	{
		return EC_VERIFY_FAIL;
	}

	*rSignLen = nvmReadU8((PN8)pbSignAddr++);
	sequenceLength -= *rSignLen;
	if (*rSignLen > orderLen)
	{
		tmpLen = *rSignLen - orderLen;
		for(i = 0; i < tmpLen; i ++)
		{
			if(0 != memReadU8(pbSignAddr))
			{
				return EC_VERIFY_FAIL;
			}
			pbSignAddr ++;
		}
//		pbSignAddr = pbSignAddr + tmpLen;
		*rSignLen = orderLen;
		memCopyToRam(rSign, pbSignAddr, orderLen);
		pbSignAddr += (*rSignLen);
	}
	else if(*rSignLen < orderLen)
	{
		tmpLen = orderLen - *rSignLen;
		ecpMem_Set(rSign, 0x00, tmpLen);
		rSign += tmpLen;
		memCopyToRam(rSign, pbSignAddr, (*rSignLen));
		pbSignAddr += (*rSignLen);
		*rSignLen = orderLen;
	}
	else
	{
		memCopyToRam(rSign, pbSignAddr, (*rSignLen));
		pbSignAddr += (*rSignLen);
	}

	if(nvmReadU8((PN8)pbSignAddr++) != 2)
	{
		return EC_VERIFY_FAIL;
	}

	*sSignLen = nvmReadU8((PN8)pbSignAddr++);
	sequenceLength -= *sSignLen;
	if(4 != sequenceLength)
	{
		return EC_VERIFY_FAIL;
	}

	if(*sSignLen > orderLen)
	{
		tmpLen = *sSignLen - orderLen;
		for(i = 0; i < tmpLen; i ++)
		{
			if(0 != memReadU8(pbSignAddr))
			{
				return EC_VERIFY_FAIL;
			}
			pbSignAddr ++;
		}
//		pbSignAddr = pbSignAddr + tmpLen;
		*sSignLen = orderLen;
		memCopyToRam(sSign, pbSignAddr, (*sSignLen));
	}
	else if(*sSignLen < orderLen)
	{
		tmpLen = orderLen - *sSignLen;
		ecpMem_Set(sSign, 0x00, tmpLen);
		sSign += tmpLen;
		memCopyToRam(sSign, pbSignAddr, (*sSignLen));
		*sSignLen = orderLen;
	}
	else
	{
		memCopyToRam(sSign, pbSignAddr, (*sSignLen));
	}
	return EC_VERIFY_OK;
}

/****************************************************************************
 * Function name: InitDefaultECDomainParams
 * Description: Through this function the curve parameters are initialized
 * Input Parameters:
 * 		none
 * Output Parameters:
 *	    none.
 * NOTE: now just support ALG_EC_FP
 ****************************************************************************/
void InitDefaultECDomainParams(void)
{
	PM8 tmpPtr = NULL;
	u16 X_A tmpLen;
	u8 X_A pLen = g_ECKeyStore.b_PLength;
//	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_002();
	
	if(g_ECKeyStore.bParamStatus == DOMAIN_PARAM_UNSETED)
	{
		switch(pLen)
		{
			#if WITH_ECC_KEY_128
			case ALG_EC_FP_KEY_128:
				tmpPtr = (PM8)E128_DOMAINPARAMS;
				g_ECKeyStore.b_RLength = EC128_FP_R_LEN;
				break;
			#endif
			#if WITH_ECC_KEY_160
			case ALG_EC_FP_KEY_160:
				tmpPtr = (PM8)E160_DOMAINPARAMS;
				g_ECKeyStore.b_RLength = EC160_FP_R_LEN;
				break;
			#endif
			#if WITH_ECC_KEY_192
			case ALG_EC_FP_KEY_192:
				tmpPtr = (PM8)E192_DOMAINPARAMS;
				g_ECKeyStore.b_RLength = EC192_FP_R_LEN;
				break;
			#endif
			#if WITH_ECC_KEY_224
			case ALG_EC_FP_KEY_224:
				tmpPtr = (PM8)E224_DOMAINPARAMS;
				g_ECKeyStore.b_RLength = EC224_FP_R_LEN;
				break;
			#endif
			#if WITH_ECC_KEY_256
			case ALG_EC_FP_KEY_256:
				tmpPtr = (PM8)E256_DOMAINPARAMS;
				g_ECKeyStore.b_RLength = EC256_FP_R_LEN;
				break;
			#endif
			#if WITH_ECC_KEY_384
			case ALG_EC_FP_KEY_384:
				tmpPtr = (PM8)E384_DOMAINPARAMS;
				g_ECKeyStore.b_RLength = EC384_FP_R_LEN;
				break;
			#endif
			default:
				throw_error(CRYPTOEXCEPT_ILLEGAL_VALUE);
				break;
		}

		// set domain parameters P, A, B
		tmpLen = pLen * 3;
		memCopyNoAtomic((PM8)g_ECKeyStore.pb_domainParams, (PM8)tmpPtr, tmpLen);

		//set type byte 0x04, and set G and R value
		memWriteU8NoAtomic((PM8)(g_ECKeyStore.pb_domainParams + tmpLen), 0x04);
		memCopyNoAtomic((PM8)(g_ECKeyStore.pb_domainParams + tmpLen + 1), (PM8)(tmpPtr + tmpLen), (pLen << 1) + g_ECKeyStore.b_RLength);
		//memRefresh();
	}
}

/**********************************************************************************
 * Function name: RSAPreSign
 * Description: This function prepare a message before RSA signature.
 * input parameter:
 * 		PM8 inBuff             The source data address.
 * 		u16 inLen                   The length of source data.
 * 		PM8 mdBuff             md information.
 *      PM8 tmpBuff			md temp data.
 *      u8 algType					algorithm.
 * output parameter:
 *		none.
 * NOTE:
 **********************************************************************************/
void ECCSign(PM8 inBuff, u16 inLen, PM8 mdBuff, PM8 tmpBuff, u8 algType,u16 preHashFlag)
{
	u8 X_A mdState = HASH_INIT, hashType = 0, hashLen = 0;

//	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_004();
	
	if (mdBuff != NULL)
	{
		mdState = HASH_UPDATE;
	}

	switch(algType)
	{
	case ALG_ECDSA_SHA:
		hashType = ALG_SHA_1;
		hashLen = SHA1_DIGEST_SIZE;
		break;
	case ALG_ECDSA_SHA_224:
		hashType = ALG_SHA_224;
		hashLen = SHA224_DIGEST_SIZE;
		break;
	case ALG_ECDSA_SHA_256:
		hashType = ALG_SHA_256;
		hashLen = SHA256_DIGEST_SIZE;
		break;
	case ALG_ECDSA_SHA_384:
		hashType = ALG_SHA_384;
		hashLen = SHA384_DIGEST_SIZE;
		break;
	case ALG_ECDSA_SHA_512:
		hashType = ALG_SHA_512;
		hashLen = SHA512_DIGEST_SIZE;
		break;
	}

	if(preHashFlag == HASH_DONE)
	{
		memCopyToRam((PM8)g_CryptoData.pbBuffer,inBuff,inLen);
	}
	else
	{
		HASHDoFinal((PM8)inBuff, inLen, (PM8)g_CryptoData.pbBuffer, mdState, mdBuff, tmpBuff, hashType);
	}
	DoECOpertaion(hashLen);
}

//u8 const  X25519_P[32] = {0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xED};
////u8 const  X25519_A[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x6D,0x06};
////u8 const  X25519_Gx[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09};
////u8 const  X25519_Gy[32] = {0x20,0xAE,0x19,0xA1,0xB8,0xA0,0x86,0xB4,0xE0,0x1E,0xDD,0x2C,0x77,0x48,0xD1,0x4C,0x92,0x3D,0x4D,0x7E,0x6D,0x7C,0x61,0xB2,0x29,0xE9,0xC5,0xA2,0x7E,0xCE,0xD3,0xD9};
////u8 const  X25519_N[32] = {0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0xDE,0xF9,0xDE,0xA2,0xF7,0x9C,0xD6,0x58,0x12,0x63,0x1A,0x5C,0xF5,0xD3,0xED};

//	
//#ifndef  NUM_ECC_DIGITS
//	#define NUM_ECC_DIGITS 8
//#endif

///* Returns nonzero if bit p_bit of p_vli is set. */
//static uint32_t vli_testBit(uint8_t *p_vli, uint32_t p_bit)
//{
//    return (p_vli[p_bit/8] & (1 << (p_bit % 8)));
//}

////LittleEndian2BigEndian(reversible)
//static void vli_littleToBig(uint8_t *des, const uint32_t *src, u8 bytelen) 
//{
//    int8_t i;
//    for (i = 0; i < bytelen; ++i) {
//        unsigned b = bytelen - 1 - i;
//        des[i] = src[b / 4] >> (8 * (b % 4));
//    }
//}

//static void cswap(uint32_t swap,uint32_t* a, uint32_t* b)
//{
//	int i;
//	uint32_t dummy,mask;

//	mask = 0 - swap;
//	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
//		dummy = mask & ( a[i] ^ b[i] );
//		a[i] = a[i] ^ dummy;
//		b[i] = b[i] ^ dummy;
//    }
//}


///*
//	x25519
//*/
//uint8_t X25519(uint8_t* p_publicKey, uint8_t* p_privateKey,uint8_t* Basepoint)
//{
//    u32 a24[NUM_ECC_DIGITS] = {0x00000000,  0x00000000,  0x00000000,  0x00000000,  0x00000000,  0x00000000,  0x00000000, 0x41DB0100};
//	u8 l_private[NUM_ECC_DIGITS<<2];
//	uint32_t x_1[NUM_ECC_DIGITS],x_2[NUM_ECC_DIGITS],x_3[NUM_ECC_DIGITS],z_2[NUM_ECC_DIGITS],z_3[NUM_ECC_DIGITS];
//	uint32_t A[NUM_ECC_DIGITS],B[NUM_ECC_DIGITS],C[NUM_ECC_DIGITS],D[NUM_ECC_DIGITS],x25519_P[NUM_ECC_DIGITS];
//	
//	MHBN_BIGNUM Bx_1,Bx_2,Bx_3,Bz_2,Bz_3,BA,BB,BC,BD,BX25519_P,Ba24;
//	uint32_t swap,b,byteLen;
//    int i;
//	
//	byteLen = NUM_ECC_DIGITS<<2;
//	
//	swap = 0;
//	
//	ecpMem_Copy(l_private,p_privateKey,byteLen);
//	
//    l_private[0] &= 248;
//    l_private[31] &= 127;
//    l_private[31] |= 64;
//	
//	vli_littleToBig((uint8_t*)x_1,(uint32_t *)Basepoint,32);
//	ecpMem_Copy(x_3,x_1,byteLen);
//	ecpMem_Copy(x25519_P,X25519_P,byteLen);
//	
//	memset(z_2,0,byteLen);
//	memset(x_2,0,byteLen);
//	memset(z_3,0,byteLen);
//	x_2[NUM_ECC_DIGITS-1] =  0x01000000;
//	z_3[NUM_ECC_DIGITS-1] =  0x01000000;
//	
//	LOADBIGNUM(Bx_1,&x_1[0],byteLen);
//	LOADBIGNUM(Bx_2,&x_2[0],byteLen);
//	LOADBIGNUM(Bx_3,&x_3[0],byteLen);
//	LOADBIGNUM(Bz_2,&z_2[0],byteLen);
//	LOADBIGNUM(Bz_3,&z_3[0],byteLen);
//	LOADBIGNUM(BA,&A[0],byteLen);
//	LOADBIGNUM(BB,&B[0],byteLen);
//	LOADBIGNUM(BC,&C[0],byteLen);
//	LOADBIGNUM(BD,&D[0],byteLen);
//	LOADBIGNUM(BX25519_P,&x25519_P[0],byteLen);
//	LOADBIGNUM(Ba24,&a24[0],byteLen);
//	
//	for(i = 255; i >=0; --i)
//	{
//		b = (vli_testBit(l_private, i)!=0);
//		swap ^= b;
//	
//		cswap(swap, x_2, x_3);
//		cswap(swap, z_2, z_3);
//	
//		swap = b;
//		
//		MHBN_BnModAdd(&BA,&Bx_2,&Bz_2,&BX25519_P);
//		MHBN_BnModSub(&BB,&Bx_2,&Bz_2,&BX25519_P);
//		MHBN_BnModAdd(&BC,&Bx_3,&Bz_3,&BX25519_P);
//		MHBN_BnModSub(&BD,&Bx_3,&Bz_3,&BX25519_P);
//		
//		MHBN_BnModMul(&Bx_2,&BD,&BA,&BX25519_P);		//x_2 = DA
//		MHBN_BnModMul(&Bz_2,&BC,&BB,&BX25519_P);		//z_2 = CB
//	
//		MHBN_BnModAdd(&Bx_3,&Bx_2,&Bz_2,&BX25519_P);		//x_3 = DA + CB
//		MHBN_BnModMul(&Bx_3,&Bx_3,&Bx_3,&BX25519_P);		//x_3 = (DA + CB)^2
//		MHBN_BnModSub(&Bz_3,&Bx_2,&Bz_2,&BX25519_P);		//z_3 = DA - CB
//		MHBN_BnModMul(&Bz_3,&Bz_3,&Bz_3,&BX25519_P);		//z_3 = (DA - CB)^2
//		MHBN_BnModMul(&Bz_3,&Bx_1,&Bz_3,&BX25519_P);		//z_3 = x_1*(DA - CB)^2
//		
//		MHBN_BnModMul(&BC,&BA,&BA,&BX25519_P);			//C=AA
//		MHBN_BnModMul(&BD,&BB,&BB,&BX25519_P);			//D=BB
//		MHBN_BnModMul(&Bx_2,&BC,&BD,&BX25519_P);		//x_2 = AA*BB		
//		
//		MHBN_BnModSub(&BA,&BC,&BD,&BX25519_P);			//A = E = AA-BB
//		MHBN_BnModMul(&Bz_2,&Ba24,&BA,&BX25519_P);		//z_2 = a24*E
//		MHBN_BnModAdd(&Bz_2,&Bz_2,&BC,&BX25519_P);		//z_2 = AA + a24*E
//		MHBN_BnModMul(&Bz_2,&Bz_2,&BA,&BX25519_P);		//z_2 = E*(AA + a24*E)	

////		ecpAAddBModC(MEMtPX8(x_2),byteLen,MEMtPX8(z_2),MEMtPX8(X25519_P),MEMtPX8(A));
////		ecpASubBModC(MEMtPX8(x_2),byteLen,MEMtPX8(z_2),MEMtPX8(X25519_P),MEMtPX8(B));
////		ecpAAddBModC(MEMtPX8(x_3),byteLen,MEMtPX8(z_3),MEMtPX8(X25519_P),MEMtPX8(C));
////		ecpASubBModC(MEMtPX8(x_3),byteLen,MEMtPX8(z_3),MEMtPX8(X25519_P),MEMtPX8(D));
////		ecpAMulBModC(MEMtPX8(D),byteLen,MEMtPX8(A),MEMtPX8(X25519_P),MEMtPX8(x_2));		//x_2 = DA
////		ecpAMulBModC(MEMtPX8(C),byteLen,MEMtPX8(B),MEMtPX8(X25519_P),MEMtPX8(z_2));		//z_2 = CB
////	
////		ecpAAddBModC(MEMtPX8(x_2),byteLen,MEMtPX8(z_2),MEMtPX8(X25519_P),MEMtPX8(x_3));		//x_3 = DA + CB
////		ecpAMulBModC(MEMtPX8(x_3),byteLen,MEMtPX8(x_3),MEMtPX8(X25519_P),MEMtPX8(x_3));		//x_3 = (DA + CB)^2
////		ecpASubBModC(MEMtPX8(x_2),byteLen,MEMtPX8(z_2),MEMtPX8(X25519_P),MEMtPX8(z_3));		//z_3 = DA - CB
////		ecpAMulBModC(MEMtPX8(z_3),byteLen,MEMtPX8(z_3),MEMtPX8(X25519_P),MEMtPX8(z_3));		//z_3 = (DA - CB)^2
////		ecpAMulBModC(MEMtPX8(z_3),byteLen,MEMtPX8(x_1),MEMtPX8(X25519_P),MEMtPX8(z_3));		//z_3 = x_1*(DA - CB)^2
////		
////		ecpAMulBModC(MEMtPX8(A),byteLen,MEMtPX8(A),MEMtPX8(X25519_P),MEMtPX8(C));			//C=AA
////		ecpAMulBModC(MEMtPX8(B),byteLen,MEMtPX8(B),MEMtPX8(X25519_P),MEMtPX8(D));			//D=BB
////		ecpAMulBModC(MEMtPX8(C),byteLen,MEMtPX8(D),MEMtPX8(X25519_P),MEMtPX8(x_2));			//x_2 = AA*BB		
////		
////		ecpASubBModC(MEMtPX8(C),byteLen,MEMtPX8(D),MEMtPX8(X25519_P),MEMtPX8(A));			//A = E = AA-BB
////		ecpAMulBModC(MEMtPX8(a24),byteLen,MEMtPX8(A),MEMtPX8(X25519_P),MEMtPX8(z_2));		//z_2 = a24*E
////		ecpAAddBModC(MEMtPX8(z_2),byteLen,MEMtPX8(C),MEMtPX8(X25519_P),MEMtPX8(z_2));		//z_2 = AA + a24*E
////		ecpAMulBModC(MEMtPX8(z_2),byteLen,MEMtPX8(A),MEMtPX8(X25519_P),MEMtPX8(z_2));		//z_2 = E*(AA + a24*E)	

//	}
//	
//	cswap(swap, x_2, x_3);
//	cswap(swap, z_2, z_3);
//	
//	MHBN_BnModInv(&BA,&Bz_2,&BX25519_P);			//A = 1/Z
//	MHBN_BnModMul(&BB,&Bx_2,&BA,&BX25519_P);		//res = x_2 * z_2^(p-2) =  x_2 / z_2
//	
////	ecpAInvModC(MEMtPX8(z_2),byteLen,MEMtPX8(X25519_P),MEMtPX8(A));			//A = 1/Z
////	ecpAMulBModC(MEMtPX8(x_2),byteLen,MEMtPX8(A),MEMtPX8(X25519_P),MEMtPX8(B));		//res = x_2 * z_2^(p-2) =  x_2 / z_2
//	
//	vli_littleToBig(p_publicKey,(uint32_t *)B,32);
//}

//u32 PointDecompress(u8 *point,u8 *x_y,u8 *argBuf)
//{
//	u32 	_num[8];
//	u32 	curve_b[8],curve_p[8];
//	u8	 	r_x[32],pub[32];
//	MHBN_BIGNUM 	BX,BB,BP,Bpub,BN;
//	u32 	byteLen;
//	
//	byteLen = g_ECKeyStore.b_PLength;
//	
//	ecpMem_Copy(r_x,x_y+1,byteLen);
//	ecpMem_Copy(curve_b,&argBuf[MAX_ECC_KEY_BYTELEN*2],byteLen);
//	ecpMem_Copy(curve_p,&argBuf[0],byteLen);
//	
//	memset(_num,0,byteLen);
//	_num[7] = 0x03000000;			//num = 3 
//	
//	LOADBIGNUM(BX,&r_x[0],byteLen);
//	LOADBIGNUM(Bpub,&pub[0],byteLen);
//	LOADBIGNUM(BP,&curve_p[0],byteLen);
//	LOADBIGNUM(BB,&curve_b[0],byteLen);
//	LOADBIGNUM(BN,&_num[0],byteLen);
//	
//	MHBN_BnModMul(&Bpub,&BX,&BX,&BP);		//r = x^2
//	MHBN_BnModSub(&Bpub,&Bpub,&BN,&BP);		//r = x^2 - 3 
//	MHBN_BnModMul(&Bpub,&Bpub,&BX,&BP);		//r = x^3 - 3x
//	MHBN_BnModAdd(&Bpub,&Bpub,&BB,&BP);		//y^2 = x^3 - 3x + b
//	
//	
//	/* When curve->p == 3 (mod 4), we can compute
//   sqrt(a) = a^((curve->p + 1) / 4) (mod curve->p). */
//   
//   ecpMem_Copy(r_x,curve_p,byteLen);
//   _num[7]  = 0x01000000;			//num = 1
//   MHBN_BnAdd(&BX,&BX,&BN);			// r_x = p+1
//			
//   _num[7]  = 0x04000000;			//num = 4
//   
//   MHBN_BnDiv(&BX,&BB,&BX,&BN);			//r_x =  p+1/4
//   MHBN_BnModExp(&Bpub,&Bpub,&BX,&BP);	// y = sqrt(y)
//   
//   if ((pub[31] & 0x01) != (x_y[0] & 0x01)) {
//		MHBN_BnModSub(&Bpub,&BP,&Bpub,&BP);	
//   }
//   ecpMem_Copy(point,(x_y+1),byteLen);
//   ecpMem_Copy(point+byteLen,pub,byteLen);
//   
//   return MH_RET_ECC_SUCCESS;
//}

#endif // WITH_ECC

#if WITH_SM2

#pragma SM2_RAM_START

SM2KeyRefStore		X_A				g_SM2KeyStore;

SM2_RAM_END

/* Recommended elliptic curve domain parameters based on Fp for SM2. */
/* all cofactors of recommended elliptic curves below are COFACTOR = 0x01 */
#define SM2_256_FP_P_LEN   32
#define SM2_256_FP_R_LEN   32

// extern u8 CONST SM2_256_DOMAINPARAMS[] =
// {
// 	/* p */0x85,0x42,0xD6,0x9E,0x4C,0x04,0x4F,0x18,0xE8,0xB9,0x24,0x35,0xBF,0x6F,0xF7,0xDE,0x45,0x72,0x83,0x91,0x5C,0x45,0x51,0x7D,0x72,0x2E,0xDB,0x8B,0x08,0xF1,0xDF,0xC3,
// 	/* a */0x78,0x79,0x68,0xB4,0xFA,0x32,0xC3,0xFD,0x24,0x17,0x84,0x2E,0x73,0xBB,0xFE,0xFF,0x2F,0x3C,0x84,0x8B,0x68,0x31,0xD7,0xE0,0xEC,0x65,0x22,0x8B,0x39,0x37,0xE4,0x98,
// 	/* b */0x63,0xE4,0xC6,0xD3,0xB2,0x3B,0x0C,0x84,0x9C,0xF8,0x42,0x41,0x48,0x4B,0xFE,0x48,0xF6,0x1D,0x59,0xA5,0xB1,0x6B,0xA0,0x6E,0x6E,0x12,0xD1,0xDA,0x27,0xC5,0x24,0x9A,
// 	/* Gx*/0x42,0x1D,0xEB,0xD6,0x1B,0x62,0xEA,0xB6,0x74,0x64,0x34,0xEB,0xC3,0xCC,0x31,0x5E,0x32,0x22,0x0B,0x3B,0xAD,0xD5,0x0B,0xDC,0x4C,0x4E,0x6C,0x14,0x7F,0xED,0xD4,0x3D,
// 	/* Gy*/0x06,0x80,0x51,0x2B,0xCB,0xB4,0x2C,0x07,0xD4,0x73,0x49,0xD2,0x15,0x3B,0x70,0xC4,0xE5,0xD7,0xFD,0xFC,0xBF,0xA3,0x6E,0xA1,0xA8,0x58,0x41,0xB9,0xE4,0x6E,0x09,0xA2,
// 	/* n */0x85,0x42,0xD6,0x9E,0x4C,0x04,0x4F,0x18,0xE8,0xB9,0x24,0x35,0xBF,0x6F,0xF7,0xDD,0x29,0x77,0x20,0x63,0x04,0x85,0x62,0x8D,0x5A,0xE7,0x4E,0xE7,0xC3,0x2E,0x79,0xB7,
// 	/* Nn = (-N) ^(-1) mod (2 ^ 64) */
// 	  0xF9,0x4B,0xF5,0x62,0x3E,0x06,0xE3,0x0D,
//     /* Qn = Q ^2 mod N using inversion algorithm */
// 	0x53,0xD0,0x27,0x11,0x94,0x2B,0x21,0xCE,0x84,0x0B,0xDC,0x3A,0x2A,0xF5,0x45,0xD5,0xC3,0xDC,0xD0,0xE3,0xDB,0x0B,0xC0,0xBC,0x7F,0xF5,0x48,0xF6,0x3A,0xD3,0x3C,0x62
// };

/*
 *ZA = ENTLa||ID||a||b||G(x,y)||P(x,y);
 * */

void SM2_ECP_GetZa(PM8 IDa, u8 ID_len, PM8 publicKeyBuffer, PM8 ZaOutBuff)
{
	PX8  			za_Buff;
	u8 		X_A 	za_ptrBak[SM2_ZABUFFLEN];
	u16 	X_A		IdBitLen;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_018();
	//ENTLa
	za_Buff = za_ptrBak;
	IdBitLen = ID_len << 3;
	PXB_W_U16(za_Buff, IdBitLen);
	za_Buff = PX8_A_U8(za_Buff, 0x02);

	//ID
	ecpMem_Copy(za_Buff, IDa, U8t16(ID_len));
	za_Buff = PX8_A_U8(za_Buff, ID_len);

	//a || b || G(x,y)
	ecpMem_Copy(za_Buff, SM2_DOMAINPARAMS_POINT_A, SM2_DOMAIN_A_B_GXY_LEN);
	za_Buff = PX8_A_U8(za_Buff, SM2_DOMAIN_A_B_GXY_LEN);

	//public key P(x,y)
	ecpMem_Copy(za_Buff, publicKeyBuffer, SM2_PUBLICKEYLEN);

	// the total length
	IdBitLen = U16_A_U8(SM2_ZABUFFLEN_MINUS_IDLEN , ID_len);
	// hash ZA buffer
	HASHDoFinal(za_ptrBak, IdBitLen, ZaOutBuff, HASH_INIT, NULL, NULL, ALG_SM3);
}

void SM2_ECP_GetE(PM8 inBuff, u16 inLen, PM8 ZA, PX8 EOutBuff)
{
	u8 		X_A 		tmpBuff[SM3_BLOCK_SIZE];
	u8 		X_A 		mdBuff[SM3_ICV_SIZE + 2];
	
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_019();
	ecpMem_Set(mdBuff, 0x00, sizeof(mdBuff));
	ecpMem_Set(tmpBuff, 0x00, sizeof(tmpBuff));

	HASHUpdate(ZA, SM2_ZALEN, HASH_INIT, mdBuff, tmpBuff, ALG_SM3);
	HASHDoFinal(inBuff, inLen, (PM8)EOutBuff, HASH_UPDATE, mdBuff, tmpBuff, ALG_SM3);
}

/*
 * Function name: KDF()
 * Description: Generate the session key .
 * Input Parameters:  (precondition)
 *      PX8  Z             input pointer for storing plain text.
 *      PX8  klen          input length ot storing plain text.

 * Output Parameters:
 *    PX8 outbuff 		  output pointer for the result.

 * Note: The following notes are just for using Fame2 of NXP SmartMX2.
 *     1、input Z area must be structed as follow:
 *				 Z	    ||	zeros
 *            64 bytes  || 4 bytes
 */

void SM2_ECP_KDF(PX8 Z, u16 klen, PX8 outbuff) SMALL_MODE
{
#define		SM3_DIGEST_SHIFT_BIT		0x05
	//the u8 type value 'BlockNum' limit the max message length not longer than 8160 bytes.
	u8 		X_A 			tempbuff[SM3_DIGEST_SIZE];
	PX8 					tempZ;
	u8 		D_A 			BlockNum;
	u8 		D_A 			remainlen;
	u8 		D_A 			temp;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_014();
	BlockNum 	= U16t8(klen >> SM3_DIGEST_SHIFT_BIT); // klen/v
	remainlen 	= klen & SM2_DOMAIN_NLEN_MASK;

	while (BlockNum--)
	{
		tempZ 		= PX8_A_U8(Z, 67);
		temp 		= PXB_R_U8(tempZ);
		U8_INC(temp);
		PXB_W_U8(tempZ, temp);
		HASHDoFinal(Z, SM2_PUBLICKEYLEN+4, outbuff, HASH_INIT, NULL, NULL, ALG_SM3);
		outbuff = PX8_A_U8(outbuff, SM2_DOMAIN_NLEN);
	}
	if (remainlen)
	{
		tempZ 		= PX8_A_U8(Z, 67);
		temp 		= PXB_R_U8(tempZ);
		U8_INC(temp);
		PXB_W_U8(tempZ, temp);
//		ecpMem_Set(tempbuff, 0, SM3_DIGEST_SIZE);
		HASHDoFinal(Z, SM2_PUBLICKEYLEN+4, tempbuff, HASH_INIT, NULL, NULL, ALG_SM3);
		ecpMem_Copy(outbuff, tempbuff, remainlen);
	}
}

/*
 * Function name: KDF()
 * Description: Generate the session key .
 * Input Parameters:  (precondition)
 *      PX8  Z             input pointer for storing plain text.
 *      PX8  klen          input length ot storing plain text.

 * Output Parameters:
 *    PX8 outbuff 		  output pointer for the result.

 * Note: The following notes are just for using Fame2 of NXP SmartMX2.
 *     1、input Z area must be structed as follow:
 *				 Z	    ||	zeros
 *            64 bytes  || 4 bytes
 */

void SM2_ECP_KDF2(PX8 Z, u16 zlen, u16 klen, PX8 outbuff) SMALL_MODE
{
#define		SM3_DIGEST_SHIFT_BIT		0x05
	//the u8 type value 'BlockNum' limit the max message length not longer than 8160 bytes.
	u8 		X_A 			tempbuff[SM3_DIGEST_SIZE];
	PX8 					tempZ;
	u8 		D_A 			BlockNum;
	u8 		D_A 			remainlen;
	u8 		D_A 			temp;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_014();
	BlockNum 	= U16t8(klen >> SM3_DIGEST_SHIFT_BIT); // klen/v
	remainlen 	= klen & SM2_DOMAIN_NLEN_MASK;

	while (BlockNum--)
	{
		tempZ 		= PX8_A_U8(Z, zlen-1);
		temp 		= PXB_R_U8(tempZ);
		U8_INC(temp);
		PXB_W_U8(tempZ, temp);
		HASHDoFinal(Z, zlen, outbuff, HASH_INIT, NULL, NULL, ALG_SM3);
		outbuff = PX8_A_U8(outbuff, SM2_DOMAIN_NLEN);
	}
	if (remainlen)
	{
		tempZ 		= PX8_A_U8(Z, zlen-1);
		temp 		= PXB_R_U8(tempZ);
		U8_INC(temp);
		PXB_W_U8(tempZ, temp);
//		ecpMem_Set(tempbuff, 0, SM3_DIGEST_SIZE);
		HASHDoFinal(Z, zlen, tempbuff, HASH_INIT, NULL, NULL, ALG_SM3);
		memCopyNoAtomic(outbuff, tempbuff, remainlen);
	}
}

#if WITH_SOFT_SM2

void SM2_ECP_GenKeyPair(PM8 privateKey, PX8 tempPublicKey)
{
	ecpAMulPointXY(privateKey, MEMtPM8(SM2_DOMAINPARAMS_POINT_G), tempPublicKey);
}

/*********************************************************************
 * Function name: SM2_ECP_GetKey()
 * Description: Generate the key pair or SM2 computation.
 * Input Parameters:  (precondition)
 *      PX8 * prikey             input pointer for storing private key, with length of 32 bytes.
 *      PX8 * pubkey             input pointer for storing public key, with length of 65 bytes: 0x04 || x || y.
 *      u8 isgenkeypair          flag to indicate whether to generate key pair. true：generate priKey and pubkey; false: cal pubkey by priKey
 * Output Parameters:  (postcondition)
 *      The keys are put in prikey and pubkey, and the operation status is returned.
 * Note: The following notes are just for using Fame2 of NXP SmartMX2.
 *       1. The output key is in big-endian format.
 *********************************************************************/
void SM2_ECP_GetKey(PX8 prikey, PX8 publickey, u8 isgenkeypair)
{
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_020();
	//生成32位随机数作为私钥,并且在区域（1，n）中
//	if(isgenkeypair == true)
//	{
		SC_checkRandamReturnCode(Random_number_generator(prikey, SM2_PRIVEYLEN, ALG_SECURE_RANDOM));
		prikey[0] &= 0xED;
		prikey[SM2_DOMAIN_NLEN_MASK] |= 0x01;
//	}

	ecpAMulPointXY(prikey, (PM8)(SM2_DOMAINPARAMS_POINT_G), publickey); //根据私钥prikey，计算得到公钥PB=[dB]G = [prikey]G

	if (isgenkeypair == true)
	{
		memCopyNoAtomic(g_SM2KeyStore.privteKeyBuffer, prikey, SM2_DOMAIN_NLEN);
		memWriteU8NoAtomic(g_SM2KeyStore.publicKeyBuffer, 0x04);
		memCopyNoAtomic(g_SM2KeyStore.publicKeyBuffer+1, publickey, SM2_PUBLICKEYLEN);
	}
}

void SM2_ECP_Sign(PX8 E_Buff, PX8 privteKey, PM8 rs)
{
	u8 		X_A			kGxyBuf[SM2_DOMAIN_POINTXY_LEN];
	u8 		X_A			kBuf[SM2_DOMAIN_NLEN];
	u8 		X_A			nBuf [SM2_DOMAIN_NLEN];
	u8 		X_A			rBuf[SM2_DOMAIN_NLEN];
	u8 		X_A			privteKeyBuffer[SM2_DOMAIN_NLEN];
	PX8					tempBuf;
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_009();
	ecpMem_Copy(nBuf, SM2_DOMAINPARAMS_POINT_N, SM2_DOMAIN_NLEN);
	ecpMem_Copy(privteKeyBuffer, privteKey, SM2_DOMAIN_NLEN);
	tempBuf = MEMtPX8(&kGxyBuf[SM2_DOMAIN_NLEN]);

	do {

		do{
			/* gen k and (x1,y1)=[k]G */
			SM2_ECP_GetKey(kBuf, kGxyBuf, false);
			/* r = (e + x1) mod n  */
			ecpAAddBModC(kGxyBuf, (u16)SM2_ELEN, E_Buff, nBuf, rBuf);
			//r can't equel 0
		}while(checkSrcIsAllZero(rBuf, SM2_ELEN));

		/* tem = (r + k) mod n	*/
		ecpAAddBModC(rBuf, (u16)SM2_ELEN, kBuf, nBuf, tempBuf);
		//(r + k) can't equel n
	}while (checkSrcIsAllZero(tempBuf, SM2_ELEN));

	//store the r into rs memory;
	memCopyNoAtomic(rs, (PM8)rBuf, SM2_DOMAIN_NLEN);

	// tem  = (r * dA) mod n
	ecpAMulBModC(rBuf, SM2_ELEN, privteKeyBuffer, nBuf , tempBuf);

	// h1 = (k - r*dA)modN = (k - tem) mod n
	ecpASubBModC(kBuf, SM2_PRIVEYLEN, tempBuf, nBuf, tempBuf); //此处tempBuf本就是复用，不如直接删了？或是只是提醒最后的那个参数是保存输出数据？

	// dA = 1 + dA;
	ecpAAddConst(privteKeyBuffer, privteKeyBuffer, SM2_PRIVEYLEN, 1);

	//s = ((1 + dA)^(-1) * (k - r*dA)) mod n = ((1 + dA)^(-1) * h1) mod n
	ecpAInvMultBModC(privteKeyBuffer, SM2_PRIVEYLEN, tempBuf, nBuf, kGxyBuf);

	//store the s into rs memory;
	memCopyNoAtomic(PM8_A_U16(rs , SM2_DOMAIN_NLEN), (PM8)kGxyBuf, SM2_DOMAIN_NLEN);
}

u8 SM2_ECP_Verify(PM8 E, PM8 publicKey, PM8 rs) SMALL_MODE
{
	u8 		X_A			rBuf[SM2_DOMAIN_NLEN];
	u8 		X_A			sBuf[SM2_DOMAIN_NLEN];
	u8 		X_A			nBuf[SM2_DOMAIN_NLEN];
	u8 		X_A			tempBuf[SM2_DOMAIN_POINTXY_LEN];
	u8 		X_A			extraRam[SM2_DOMAIN_POINTXY_LEN];
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_010();
	ecpMem_Copy(rBuf, rs, SM2_DOMAIN_NLEN);
	//check r,if r =0,return SM2_VERIFY_FAILED
	if (checkSrcIsAllZero(rBuf, SM2_DOMAIN_NLEN))
	{
		return SM2_VERIFY_FAILED;
	}

	//check r,if r >n - 1 ----> (r + 1 > n),return SM2_VERIFY_FAILED
	ecpAAddConst(rBuf, tempBuf, SM2_DOMAIN_NLEN, 0x01);
	ecpMem_Copy(nBuf, SM2_DOMAINPARAMS_POINT_N, SM2_DOMAIN_NLEN);
	//if (A_IS_BIGGER_THAN_B == ecpACompareB(tempBuf, nBuf, U32t16(SM2_DOMAIN_NLEN)))
	if (0 < ecpMem_Compare(tempBuf, nBuf, U32t16(SM2_DOMAIN_NLEN)))
	{
		return SM2_VERIFY_FAILED;
	}

	ecpMem_Copy(sBuf, rs + SM2_DOMAIN_NLEN, SM2_DOMAIN_NLEN);
	//check s,if s =0,return SM2_VERIFY_FAILED
	if (checkSrcIsAllZero(sBuf, SM2_DOMAIN_NLEN))
	{
		return SM2_VERIFY_FAILED;
	}

	//check s,if s >n - 1 ----> (s + 1 > n),return SM2_VERIFY_FAILED
	ecpAAddConst(sBuf, tempBuf, SM2_DOMAIN_NLEN, 0x01);
	//if(A_IS_BIGGER_THAN_B == ecpACompareB(sBuf, nBuf, U32t16(SM2_DOMAIN_NLEN)))
	if(0 < ecpMem_Compare(sBuf, nBuf, U32t16(SM2_DOMAIN_NLEN)))
	{
		return SM2_VERIFY_FAILED;
	}

	// t = (r + s)modn
	ecpAAddBModC(rBuf, SM2_DOMAIN_NLEN, sBuf, nBuf , extraRam);

	// [t]*PA
	if(CAL_SUCCESSED != ecpAMulPointXY(extraRam, publicKey, tempBuf))
	{
		return SM2_VERIFY_FAILED;
	}

	// s*G
	ecpAMulPointXY(sBuf, (PM8)(SM2_DOMAINPARAMS_POINT_G), extraRam);

	// (x,y) = s*G + t*PA
	ecpPointAddPoint(tempBuf, extraRam, tempBuf);

	// R = (e + x)modn .
	//the final result is put in the result.
	ecpAAddBModC(tempBuf, SM2_ELEN, E, nBuf, extraRam);

	//check R = r.
	ecpMem_Copy(rBuf, rs, SM2_DOMAIN_NLEN);
	//if (A_IS_EQUAL_WITH_B != ecpACompareB(rBuf, extraRam, SM2_DOMAIN_NLEN))
	if (0 != ecpMem_Compare(rBuf, extraRam, SM2_DOMAIN_NLEN))
	{
		return SM2_VERIFY_FAILED;
	}

	//CFG_CLKSEL = backCLKSEL;
	return SM2_VERIFY_SUCCESS;
}

//The max value of inLen is 183( 280 - SM2_CIPHER_PADDING_LEN)
//Note: The content of inBuff is modified after the calculation!
/************************************************************
 ******Fuction name:SM2_Encrypt
 ****** input parameter:
 ******      inBuff:   				ram address;
 ******			publickeyBuff:    e2p address
 ******			outBuff:    			e2p address
 ************************************************************/
void SM2_ECP_Encrypt(PX8 inBuff, u16 inLen, PM8 publickeyBuff, PM8 outBuff)
{
	
	u8 		X_A			extraRam[1025];	// computing KDF extraRam needs 183 bytes area
	u8 		X_A			KGbuff[68];
//	u8 		X_A			tembuffer[64];	
	u8 		X_A			pubkeyBuff[SM2_PUBLICKEYLEN];
	u8 		X_A			Krbuff[SM2_DOMAIN_NLEN+4];
	u8		X_A			tempLen;
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_012();
	//from E2P to ram
	memCopyToRam(pubkeyBuff, publickeyBuff, SM2_PUBLICKEYLEN);

	do {
		//step1: calculate C1;  点C1=[k]G=(x1,y1)
		SM2_ECP_GetKey(Krbuff, KGbuff, false);
		memCopyNoAtomic(outBuff, (PM8)KGbuff, SM2_PUBLICKEYLEN);
//		ecpMem_Copy(tembuffer, (PM8)KGbuff, SM2_PUBLICKEYLEN);

		//step2: calculate KDF;
		//点[k]PB=(x2,y2)
		ecpAMulPointXY(Krbuff, pubkeyBuff, KGbuff);
		ecpMem_Copy(pubkeyBuff, KGbuff, SM2_PUBLICKEYLEN);
		//when calculate KDF, the format of input is 64 bytes + 4 bytes, and the 4 bytes be initially set zeros.
		ecpMem_Set((KGbuff + SM2_PUBLICKEYLEN), 0, 4);
		//t=KDF(x2 ∥ y2, klen)，
		SM2_ECP_KDF(KGbuff, inLen, extraRam);
	}while( checkSrcIsAllZero(extraRam, inLen)); //if the result equal zero, return step1.

	//step3: C2 = M XOR t;
	ecpAXorB(extraRam, inBuff, extraRam, inLen);

	//step 4: C3 = SM3(x2||inBuff||y2)，.
	HASHUpdate( pubkeyBuff, SM2_PRIVEYLEN, HASH_INIT, Krbuff, KGbuff, ALG_SM3);
	HASHUpdate( inBuff, inLen, HASH_UPDATE, Krbuff, KGbuff, ALG_SM3);

	tempLen = SM2_PUBLICKEYLEN;
	if (!OS_CONF4_SM2_ENC_C132_MODE)
	{
		//after inversed,	C2 be put in outBuff
		//finally C1||C2||C3 be put in outBuff
		memCopyNoAtomic(PM8_A_U8(outBuff, tempLen), (PM8)extraRam, inLen);
		tempLen = U8_A_U8(tempLen , inLen);
		HASHDoFinal(PM8_A_U8(pubkeyBuff, SM2_PRIVEYLEN), SM2_PRIVEYLEN , PM8_A_U8(outBuff, tempLen), HASH_UPDATE, Krbuff, KGbuff, ALG_SM3);
	}
	else
	{
		//C2 be put in outBuff(C1||C3||C2)
		//finally C1||C3||C2 be put in outBuff.
		HASHDoFinal(PM8_A_U8(pubkeyBuff, SM2_PRIVEYLEN), SM2_PRIVEYLEN , PM8_A_U8(outBuff, tempLen), HASH_UPDATE, Krbuff, KGbuff, ALG_SM3);
		//tempLen = U8_A_U8(SM2_PUBLICKEYLEN , SM3_DIGEST_SIZE);
		memCopyNoAtomic(PM8_A_U8(outBuff, (  SM2_PUBLICKEYLEN + SM3_DIGEST_SIZE)), (PM8)extraRam, inLen);
	}
//	memCopyNoAtomic(outBuff, tembuffer, SM2_PUBLICKEYLEN);
	//put in 0x04 as a flag for cryptograph
//	memWriteU8NoAtomic(outBuff, 0x04);
	return;
}

//The max value of inLen is 183( 280 - SM2_CIPHER_PADDING_LEN)
/*
 * the structure of algType is as follows
 *  ------------------------------------------------------------------------------------------
 *  | SM2_DOMAIN_NLEN 						| 184		 |SM2_DOMAIN_POINTXY_LEN + 4 		 |
 *  | privare Key, SM2_DOMAIN_POINTX, HASH  | Indata ,C2 |C1buff, KDF RAM,  SM2_DOMAIN_POINTY|
 *  ------------------------------------------------------------------------------------------
 *
*/
u8 SM2_ECP_Decrypt(PM8 inBuff, u16 inLen, PM8 privatekeybuff, PM8 outBuff)
{
	u8		X_A		extraRam[SM2_DOMAIN_NLEN + 1025 + SM2_DOMAIN_POINTXY_LEN + 4];
	PX8				C1buff;
	PX8				C2buff;
	
	
//	u8		X_A      extraRam[248];
	// u8							SM2workspace[PH_SMX2CL_SM2DECRYPT_SIZEOF_WS_FX_256 + 7];
//	PX8 C2buff = g_ReuseBigBuf + 1000;
//	PX8 C1buff = C2buff + 183 + 1;// computing KDF extraRam needs 183 bytes area, 8 bytes aligned.
//	PX8 extraRam = C1buff + 64;
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_013();
	
	C1buff			= &extraRam[SM2_DOMAIN_NLEN + 1025];
	C2buff			= &extraRam[SM2_DOMAIN_NLEN];
	
	inLen = inLen - SM2_CIPHER_PADDING_LEN;// the length of the message.

	//step 1: [dA]C1 = (x2,y2),	inBuff is structed as: 04 || C1 || C2 || C3.
	memCopyToRam(extraRam, privatekeybuff, SM2_PRIVEYLEN);
	memCopyToRam(C1buff, inBuff, SM2_PUBLICKEYLEN);

	if(CAL_SUCCESSED != ecpAMulPointXY(extraRam, C1buff, C2buff))
	{
		return SM2_VERIFY_FAILED;
	}
	ecpMem_Copy(C1buff, C2buff, SM2_PUBLICKEYLEN);

	do {
		// step2: t = KDF(x2||y2, kelen), kelen equals the length of C2.
		ecpMem_Set((C1buff + SM2_PUBLICKEYLEN), 0, 4);
		SM2_ECP_KDF(C1buff, inLen, C2buff);

	}while(checkSrcIsAllZero(C2buff, inLen)); // if t equal zeros, reture step2.

	//step3: M' = C2 xor t
	if (OS_CONF4_SM2_ENC_C132_MODE)
	{
		// the sequence "04 || C1 || C3 || C2"
		//ecpMem_Copy(&C2buff[SM2_PRIVEYLEN], inBuff + 1 + SM2_PUBLICKEYLEN + SM3_DIGEST_SIZE, inLen);
		ecpAXorB(C2buff, inBuff + SM2_PUBLICKEYLEN + SM3_DIGEST_SIZE, C2buff, inLen);
	}
	else
	{
		// the sequence "04 || C1 || C2 || C3".
		//ecpMem_Copy(&C2buff[SM2_PRIVEYLEN], inBuff + SM2_PUBLICKEYLEN + 1, inLen);
		ecpAXorB(C2buff, inBuff + SM2_PUBLICKEYLEN, C2buff, inLen);
	}

	//step4: u = SM3(x2||M'||y2)
	ecpMem_Copy(extraRam, C1buff , SM2_PRIVEYLEN);
//	ecpMem_Copy(extraRam + SM2_PRIVEYLEN, C2buff, inLen);
	ecpMem_Copy(extraRam + inLen + SM2_PRIVEYLEN, C1buff + SM2_PRIVEYLEN, SM2_PRIVEYLEN);
	HASHDoFinal(extraRam, SM2_PUBLICKEYLEN + inLen, C1buff, HASH_INIT, NULL, NULL, ALG_SM3);
	//compare u and C3,  if u != C3,returen fail;
	if (OS_CONF4_SM2_ENC_C132_MODE)
	{
		// the sequence "04 || C1 || C3 || C2"
		memCopyToRam(extraRam, inBuff + SM2_PUBLICKEYLEN, SM3_DIGEST_SIZE);
	}
	else
	{
		// the sequence "04 || C1 || C2 || C3".
		memCopyToRam(extraRam, (inBuff + SM2_PUBLICKEYLEN + inLen), SM3_DIGEST_SIZE);
	}

	//if(A_IS_EQUAL_WITH_B != ecpACompareB(C1buff, extraRam ,SM3_DIGEST_SIZE ))
	if(0 != ecpMem_Compare(C1buff, extraRam ,SM3_DIGEST_SIZE ))
	{
		return SM2_VERIFY_FAILED;
	}

	//CFG_CLKSEL = backCLKSEL;
	memCopyNoAtomic(outBuff, (PM8)C2buff, inLen);
	return SM2_VERIFY_SUCCESS;
}
#endif // WITH_SOFT_SM2

u16 SM2_Signature(PM8 inBuff, u16 inLen, PM8 sigBuff, PM8 Za, PM8 mdBuff, PM8 tmpBuff,  u8 mode)
{
	PX8				Ep;
	PX8				RSp;
	//PM8				tempSigBuff;
	PX8				tempKeyBuffer;
	PX8             tempBuffer;
	u16				result;
	
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_015();
	if (isInE2P(inBuff))
	{
	   memRefresh();
	}

	tempBuffer      = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));
	//E值
	Ep 				= &tempBuffer[0];
	//保存sign结果
	RSp 			= &tempBuffer[32];
	tempKeyBuffer 	= &tempBuffer[96];
	//tempSigBuff 	= &tempBuffer[160]; // the length of this buffer si 64 bytes.

	
	if(mdBuff == NULL)
	{
		SM2_ECP_GetE(inBuff, inLen, Za, Ep);
	}
	else
	{
		HASHDoFinal(inBuff, inLen, Ep, HASH_UPDATE, mdBuff, tmpBuff, ALG_SM3);
	}
	
	
	if (SM2_SIGN == mode)
	{	
		memCopyToRam(tempKeyBuffer, SM2_getPriKey(), SM2_PRIVEYLEN);
	    SM2_ECP_Sign(Ep, tempKeyBuffer, sigBuff);
		result = SM2_PUBLICKEYLEN;
	}
	else  // for verify
	{
	  	memCopyToRam(tempKeyBuffer, PM8_A_U8(SM2_getPubKey() , 1), SM2_PUBLICKEYLEN);
		memCopyToRam(RSp, sigBuff, SM2_PUBLICKEYLEN);
		result = SM2_ECP_Verify(Ep, tempKeyBuffer, RSp);
	}
	return result ;
}


u16 SM2_SigVerifyWithNoSM3(PM8 inBuff, u16 inLen, PM8 sigBuff, u16 sigLen, PM8 tmpSrcBuff, u16 accOffset, u8 mode, u8 sm2State)
{
	PX8				Ep;
	PX8				RSp;
	PX8				tempKeyBuffer;
	PX8             tempBuffer ;
	u16				inOffset;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_016();
	tempBuffer      = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));
	Ep 				= &tempBuffer[0];
	RSp 			= &tempBuffer[32];
	tempKeyBuffer 	= &tempBuffer[96];

	if (SM2_INIT == sm2State)
	{
		inOffset 	= inLen;  // calculate the total input length.
		memCopyToRam(Ep, inBuff, inOffset);
	}
	else
	{
//		inOffset 	= inLen + accOffset;  // calculate the total input length.
		memCopyToRam(Ep , tmpSrcBuff, accOffset);
		memCopyToRam(PX8_A_U16(Ep, accOffset), inBuff, inLen);
	}

	if (SM2_SIGN == mode)
	{
		memCopyToRam(tempKeyBuffer, SM2_getPriKey(), SM2_PRIVEYLEN);
	    SM2_ECP_Sign(Ep, tempKeyBuffer, sigBuff);
		return SM2_PUBLICKEYLEN;
	}
	else  // for verify
	{
	  	memCopyToRam(tempKeyBuffer, PM8_A_U8(SM2_getPubKey() , 1), SM2_PUBLICKEYLEN);
		memCopyToRam(RSp, sigBuff, SM2_PUBLICKEYLEN);
		return SM2_ECP_Verify(Ep, tempKeyBuffer, RSp);
	}
}


/*****************************************************************************
 *  name: SM2_CipherDofinal
 *  param:
            inBuff  	input date
            inLen   	input buffer length
            outBuff 	the pointer of cipher date
            tmpSrcBuff
            accOffset
            mode        SM2ENCRYPT(0x02) or SM2DECRYPT(0x01)
            SM2State    SM2_INIT(0x00)
 *****************************************************************************/
u16 SM2_CipherDofinal(PM8 inBuff, u16 inLen,  PM8 outBuff, PM8 tmpSrcBuff, u16 accOffset, u8 mode, u8 SM2State)
{
	PX8				pbBuffer;
//	u8				tmp_buf[0x400+97];	
	u16		D_A		wBufferLen;
	u16     D_A     inOffset;
	
	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_017();
	if (SM2_INIT == SM2State)
	{
		inOffset 	= inLen;  // calculate the total input length.
	}
	else
	{
		inOffset 	= inLen + accOffset;  // calculate the total input length.
	}

	wBufferLen = inOffset;  // total length input.

//		pbBuffer 	= MEMtPX8(MemGetReuseRam_GetTempArrBuffer(0x400+97)); // for 183 bytes SM2 encrypt computation, this buffer should be at least 280 bytes.


	if (SM2_INIT == SM2State)
	{
//		memCopyToRam(pbBuffer, inBuff, inOffset);
		pbBuffer = inBuff;
	}
	else
	{
//		memCopyToRam(pbBuffer, tmpSrcBuff, accOffset);
//		memCopyToRam(PX8_A_U16(pbBuffer, accOffset), inBuff, inLen);
		pbBuffer = tmpSrcBuff;
		memCopyNoAtomic(PX8_A_U16(pbBuffer, accOffset), inBuff, inLen);
	}

	if (SM2ENCRYPT == mode)
	{
		//check_bounds_interval((PF8)outBuff2, outOffset, (inOffset + SM2_CIPHER_PADDING_LEN));
		if(!GUOMI_STANDARD)
		{
			SM2_ECP_Encrypt(pbBuffer, wBufferLen, PM8_A_U8(SM2_getPubKey() , 1), PM8_A_U8(outBuff, 1));
			memWriteU8NoAtomic(outBuff, 0x04);
			wBufferLen += (SM2_CIPHER_PADDING_LEN + 1);
		}
		else
		{
			SM2_ECP_Encrypt(pbBuffer, wBufferLen, PM8_A_U8(SM2_getPubKey() , 1), outBuff);
			wBufferLen += SM2_CIPHER_PADDING_LEN;
		}
	}
	else  // decrypt
	{
		//check_bounds_interval((PF8)outBuff2, outOffset, (inOffset - SM2_CIPHER_PADDING_LEN));
		if(!GUOMI_STANDARD)
		{
			if (pbBuffer[0] == 0x04)
			{
				U16_DEC(wBufferLen);
				PX8_INC(pbBuffer);
			}
			else
			{	
				return S16tU16(-1);
			}
		}

		if (wBufferLen >= SM2_CIPHER_PADDING_LEN)
		{
			if (SM2_ECP_Decrypt(pbBuffer, wBufferLen, SM2_getPriKey(), outBuff) == SM2_VERIFY_SUCCESS)
			{
				wBufferLen -= SM2_CIPHER_PADDING_LEN;
			}
			else
			{
				wBufferLen = S16tU16(-1);
			}
		}
		else
		{
			wBufferLen = S16tU16(-1);
		}
	}
	return wBufferLen;
}


#endif  // WITH_SM2
