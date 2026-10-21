/*
 * HASH_RIPEMD160_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */
#include "HASH_API.h"

#if WITH_RIPEMD160

/***********************************************************************************
 * macro for dword rotate left define
 ***********************************************************************************/
#define ROTATE_LEFT(Sdata, nBit) (((Sdata) << (nBit)) | ((Sdata) >> (32-(nBit))))

/*******************************************************************************
 * Define the basic RIPEMD160 macro functions.
 ******************************************************************************/
/* the five basic functions RIPEMD160_F(), RIPEMD160_G() and RIPEMD160_H() */
#define RIPEMD160_F(x, y, z)        ((x) ^ (y) ^ (z))
#define RIPEMD160_G(x, y, z)        (((x) & (y)) | (~(x) & (z)))
#define RIPEMD160_H(x, y, z)        (((x) | ~(y)) ^ (z))
#define RIPEMD160_I(x, y, z)        (((x) & (z)) | ((y) & ~(z)))
#define RIPEMD160_J(x, y, z)        ((x) ^ ((y) | ~(z)))

/* the ten basic operations RIPEMD160_FF() through RIPEMD160_III() */
#define RIPEMD160_FF(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_F((b), (c), (d)) + (x);\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_GG(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_G((b), (c), (d)) + (x) + 0x5A827999;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_HH(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_H((b), (c), (d)) + (x) + 0x6ED9EBA1;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_II(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_I((b), (c), (d)) + (x) + 0x8F1BBCDC;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_JJ(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_J((b), (c), (d)) + (x) + 0xA953FD4E;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_FFF(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_F((b), (c), (d)) + (x);\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_GGG(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_G((b), (c), (d)) + (x) + 0x7A6D76E9;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_HHH(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_H((b), (c), (d)) + (x) + 0x6D703EF3;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_III(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_I((b), (c), (d)) + (x) + 0x5C4DD124;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }
#define RIPEMD160_JJJ(a, b, c, d, e, x, s)        {\
      (a) += RIPEMD160_J((b), (c), (d)) + (x) + 0x50A28BE6;\
      (a) = ROTATE_LEFT((a), (s)) + (e);\
      (c) = ROTATE_LEFT((c), 10);\
   }

/**********************************************************************************
 * Function name: RIPEMD160Transform
 * Description: This function does RIPEMD160 kernel operation.
 * Input parameter:
 *	   	PX32 state	    	state of RIPEMD160.
 *      XDATA_U8_P blockBuff        block buffer.
 * Output parameter:
 *		none.
 * NOTE:
 **********************************************************************************/
static void RIPEMD160Transform(PX32 blockBuff, PX32 state)
{
    PX32 X_A tmp;

	// fetch the state from global variable.
	u32 X_A aa = state[0];
	u32 X_A bb = state[1];
	u32 X_A cc = state[2];
	u32 X_A dd = state[3];
	u32 X_A ee = state[4];

	u32 X_A aaa = state[0];
	u32 X_A bbb = state[1];
	u32 X_A ccc = state[2];
	u32 X_A ddd = state[3];
	u32 X_A eee = state[4];

	// cast unsigned char buffer to unsigned long
	tmp = (PX32)blockBuff;

#if U8_2_U32_INVERSE
	// processing block buffer of u32 should be little endian order for RIPEMD160
#else
	// break g_MDCtxt.bMDBlock[MD5_BLOCK_SIZE] into sixteen 32-bit little-endian dwords tmp[i], 0 = i = 15
	BigEndian2LittleEndian(MEMtPX8(tmp), RIPEMD160_BLOCK_SIZE );
#endif

	/* round 1 */
	RIPEMD160_FF(aa, bb, cc, dd, ee, tmp[ 0], 11);
	RIPEMD160_FF(ee, aa, bb, cc, dd, tmp[ 1], 14);
	RIPEMD160_FF(dd, ee, aa, bb, cc, tmp[ 2], 15);
	RIPEMD160_FF(cc, dd, ee, aa, bb, tmp[ 3], 12);
	RIPEMD160_FF(bb, cc, dd, ee, aa, tmp[ 4],  5);
	RIPEMD160_FF(aa, bb, cc, dd, ee, tmp[ 5],  8);
	RIPEMD160_FF(ee, aa, bb, cc, dd, tmp[ 6],  7);
	RIPEMD160_FF(dd, ee, aa, bb, cc, tmp[ 7],  9);
	RIPEMD160_FF(cc, dd, ee, aa, bb, tmp[ 8], 11);
	RIPEMD160_FF(bb, cc, dd, ee, aa, tmp[ 9], 13);
	RIPEMD160_FF(aa, bb, cc, dd, ee, tmp[10], 14);
	RIPEMD160_FF(ee, aa, bb, cc, dd, tmp[11], 15);
	RIPEMD160_FF(dd, ee, aa, bb, cc, tmp[12],  6);
	RIPEMD160_FF(cc, dd, ee, aa, bb, tmp[13],  7);
	RIPEMD160_FF(bb, cc, dd, ee, aa, tmp[14],  9);
	RIPEMD160_FF(aa, bb, cc, dd, ee, tmp[15],  8);

	/* round 2 */
	RIPEMD160_GG(ee, aa, bb, cc, dd, tmp[ 7],  7);
	RIPEMD160_GG(dd, ee, aa, bb, cc, tmp[ 4],  6);
	RIPEMD160_GG(cc, dd, ee, aa, bb, tmp[13],  8);
	RIPEMD160_GG(bb, cc, dd, ee, aa, tmp[ 1], 13);
	RIPEMD160_GG(aa, bb, cc, dd, ee, tmp[10], 11);
	RIPEMD160_GG(ee, aa, bb, cc, dd, tmp[ 6],  9);
	RIPEMD160_GG(dd, ee, aa, bb, cc, tmp[15],  7);
	RIPEMD160_GG(cc, dd, ee, aa, bb, tmp[ 3], 15);
	RIPEMD160_GG(bb, cc, dd, ee, aa, tmp[12],  7);
	RIPEMD160_GG(aa, bb, cc, dd, ee, tmp[ 0], 12);
	RIPEMD160_GG(ee, aa, bb, cc, dd, tmp[ 9], 15);
	RIPEMD160_GG(dd, ee, aa, bb, cc, tmp[ 5],  9);
	RIPEMD160_GG(cc, dd, ee, aa, bb, tmp[ 2], 11);
	RIPEMD160_GG(bb, cc, dd, ee, aa, tmp[14],  7);
	RIPEMD160_GG(aa, bb, cc, dd, ee, tmp[11], 13);
	RIPEMD160_GG(ee, aa, bb, cc, dd, tmp[ 8], 12);

	/* round 3 */
	RIPEMD160_HH(dd, ee, aa, bb, cc, tmp[ 3], 11);
	RIPEMD160_HH(cc, dd, ee, aa, bb, tmp[10], 13);
	RIPEMD160_HH(bb, cc, dd, ee, aa, tmp[14],  6);
	RIPEMD160_HH(aa, bb, cc, dd, ee, tmp[ 4],  7);
	RIPEMD160_HH(ee, aa, bb, cc, dd, tmp[ 9], 14);
	RIPEMD160_HH(dd, ee, aa, bb, cc, tmp[15],  9);
	RIPEMD160_HH(cc, dd, ee, aa, bb, tmp[ 8], 13);
	RIPEMD160_HH(bb, cc, dd, ee, aa, tmp[ 1], 15);
	RIPEMD160_HH(aa, bb, cc, dd, ee, tmp[ 2], 14);
	RIPEMD160_HH(ee, aa, bb, cc, dd, tmp[ 7],  8);
	RIPEMD160_HH(dd, ee, aa, bb, cc, tmp[ 0], 13);
	RIPEMD160_HH(cc, dd, ee, aa, bb, tmp[ 6],  6);
	RIPEMD160_HH(bb, cc, dd, ee, aa, tmp[13],  5);
	RIPEMD160_HH(aa, bb, cc, dd, ee, tmp[11], 12);
	RIPEMD160_HH(ee, aa, bb, cc, dd, tmp[ 5],  7);
	RIPEMD160_HH(dd, ee, aa, bb, cc, tmp[12],  5);

	/* round 4 */
	RIPEMD160_II(cc, dd, ee, aa, bb, tmp[ 1], 11);
	RIPEMD160_II(bb, cc, dd, ee, aa, tmp[ 9], 12);
	RIPEMD160_II(aa, bb, cc, dd, ee, tmp[11], 14);
	RIPEMD160_II(ee, aa, bb, cc, dd, tmp[10], 15);
	RIPEMD160_II(dd, ee, aa, bb, cc, tmp[ 0], 14);
	RIPEMD160_II(cc, dd, ee, aa, bb, tmp[ 8], 15);
	RIPEMD160_II(bb, cc, dd, ee, aa, tmp[12],  9);
	RIPEMD160_II(aa, bb, cc, dd, ee, tmp[ 4],  8);
	RIPEMD160_II(ee, aa, bb, cc, dd, tmp[13],  9);
	RIPEMD160_II(dd, ee, aa, bb, cc, tmp[ 3], 14);
	RIPEMD160_II(cc, dd, ee, aa, bb, tmp[ 7],  5);
	RIPEMD160_II(bb, cc, dd, ee, aa, tmp[15],  6);
	RIPEMD160_II(aa, bb, cc, dd, ee, tmp[14],  8);
	RIPEMD160_II(ee, aa, bb, cc, dd, tmp[ 5],  6);
	RIPEMD160_II(dd, ee, aa, bb, cc, tmp[ 6],  5);
	RIPEMD160_II(cc, dd, ee, aa, bb, tmp[ 2], 12);

	/* round 5 */
	RIPEMD160_JJ(bb, cc, dd, ee, aa, tmp[ 4],  9);
	RIPEMD160_JJ(aa, bb, cc, dd, ee, tmp[ 0], 15);
	RIPEMD160_JJ(ee, aa, bb, cc, dd, tmp[ 5],  5);
	RIPEMD160_JJ(dd, ee, aa, bb, cc, tmp[ 9], 11);
	RIPEMD160_JJ(cc, dd, ee, aa, bb, tmp[ 7],  6);
	RIPEMD160_JJ(bb, cc, dd, ee, aa, tmp[12],  8);
	RIPEMD160_JJ(aa, bb, cc, dd, ee, tmp[ 2], 13);
	RIPEMD160_JJ(ee, aa, bb, cc, dd, tmp[10], 12);
	RIPEMD160_JJ(dd, ee, aa, bb, cc, tmp[14],  5);
	RIPEMD160_JJ(cc, dd, ee, aa, bb, tmp[ 1], 12);
	RIPEMD160_JJ(bb, cc, dd, ee, aa, tmp[ 3], 13);
	RIPEMD160_JJ(aa, bb, cc, dd, ee, tmp[ 8], 14);
	RIPEMD160_JJ(ee, aa, bb, cc, dd, tmp[11], 11);
	RIPEMD160_JJ(dd, ee, aa, bb, cc, tmp[ 6],  8);
	RIPEMD160_JJ(cc, dd, ee, aa, bb, tmp[15],  5);
	RIPEMD160_JJ(bb, cc, dd, ee, aa, tmp[13],  6);

	/* parallel round 1 */
	RIPEMD160_JJJ(aaa, bbb, ccc, ddd, eee, tmp[ 5],  8);
	RIPEMD160_JJJ(eee, aaa, bbb, ccc, ddd, tmp[14],  9);
	RIPEMD160_JJJ(ddd, eee, aaa, bbb, ccc, tmp[ 7],  9);
	RIPEMD160_JJJ(ccc, ddd, eee, aaa, bbb, tmp[ 0], 11);
	RIPEMD160_JJJ(bbb, ccc, ddd, eee, aaa, tmp[ 9], 13);
	RIPEMD160_JJJ(aaa, bbb, ccc, ddd, eee, tmp[ 2], 15);
	RIPEMD160_JJJ(eee, aaa, bbb, ccc, ddd, tmp[11], 15);
	RIPEMD160_JJJ(ddd, eee, aaa, bbb, ccc, tmp[ 4],  5);
	RIPEMD160_JJJ(ccc, ddd, eee, aaa, bbb, tmp[13],  7);
	RIPEMD160_JJJ(bbb, ccc, ddd, eee, aaa, tmp[ 6],  7);
	RIPEMD160_JJJ(aaa, bbb, ccc, ddd, eee, tmp[15],  8);
	RIPEMD160_JJJ(eee, aaa, bbb, ccc, ddd, tmp[ 8], 11);
	RIPEMD160_JJJ(ddd, eee, aaa, bbb, ccc, tmp[ 1], 14);
	RIPEMD160_JJJ(ccc, ddd, eee, aaa, bbb, tmp[10], 14);
	RIPEMD160_JJJ(bbb, ccc, ddd, eee, aaa, tmp[ 3], 12);
	RIPEMD160_JJJ(aaa, bbb, ccc, ddd, eee, tmp[12],  6);

	/* parallel round 2 */
	RIPEMD160_III(eee, aaa, bbb, ccc, ddd, tmp[ 6],  9);
	RIPEMD160_III(ddd, eee, aaa, bbb, ccc, tmp[11], 13);
	RIPEMD160_III(ccc, ddd, eee, aaa, bbb, tmp[ 3], 15);
	RIPEMD160_III(bbb, ccc, ddd, eee, aaa, tmp[ 7],  7);
	RIPEMD160_III(aaa, bbb, ccc, ddd, eee, tmp[ 0], 12);
	RIPEMD160_III(eee, aaa, bbb, ccc, ddd, tmp[13],  8);
	RIPEMD160_III(ddd, eee, aaa, bbb, ccc, tmp[ 5],  9);
	RIPEMD160_III(ccc, ddd, eee, aaa, bbb, tmp[10], 11);
	RIPEMD160_III(bbb, ccc, ddd, eee, aaa, tmp[14],  7);
	RIPEMD160_III(aaa, bbb, ccc, ddd, eee, tmp[15],  7);
	RIPEMD160_III(eee, aaa, bbb, ccc, ddd, tmp[ 8], 12);
	RIPEMD160_III(ddd, eee, aaa, bbb, ccc, tmp[12],  7);
	RIPEMD160_III(ccc, ddd, eee, aaa, bbb, tmp[ 4],  6);
	RIPEMD160_III(bbb, ccc, ddd, eee, aaa, tmp[ 9], 15);
	RIPEMD160_III(aaa, bbb, ccc, ddd, eee, tmp[ 1], 13);
	RIPEMD160_III(eee, aaa, bbb, ccc, ddd, tmp[ 2], 11);

	/* parallel round 3 */
	RIPEMD160_HHH(ddd, eee, aaa, bbb, ccc, tmp[15],  9);
	RIPEMD160_HHH(ccc, ddd, eee, aaa, bbb, tmp[ 5],  7);
	RIPEMD160_HHH(bbb, ccc, ddd, eee, aaa, tmp[ 1], 15);
	RIPEMD160_HHH(aaa, bbb, ccc, ddd, eee, tmp[ 3], 11);
	RIPEMD160_HHH(eee, aaa, bbb, ccc, ddd, tmp[ 7],  8);
	RIPEMD160_HHH(ddd, eee, aaa, bbb, ccc, tmp[14],  6);
	RIPEMD160_HHH(ccc, ddd, eee, aaa, bbb, tmp[ 6],  6);
	RIPEMD160_HHH(bbb, ccc, ddd, eee, aaa, tmp[ 9], 14);
	RIPEMD160_HHH(aaa, bbb, ccc, ddd, eee, tmp[11], 12);
	RIPEMD160_HHH(eee, aaa, bbb, ccc, ddd, tmp[ 8], 13);
	RIPEMD160_HHH(ddd, eee, aaa, bbb, ccc, tmp[12],  5);
	RIPEMD160_HHH(ccc, ddd, eee, aaa, bbb, tmp[ 2], 14);
	RIPEMD160_HHH(bbb, ccc, ddd, eee, aaa, tmp[10], 13);
	RIPEMD160_HHH(aaa, bbb, ccc, ddd, eee, tmp[ 0], 13);
	RIPEMD160_HHH(eee, aaa, bbb, ccc, ddd, tmp[ 4],  7);
	RIPEMD160_HHH(ddd, eee, aaa, bbb, ccc, tmp[13],  5);

	/* parallel round 4 */
	RIPEMD160_GGG(ccc, ddd, eee, aaa, bbb, tmp[ 8], 15);
	RIPEMD160_GGG(bbb, ccc, ddd, eee, aaa, tmp[ 6],  5);
	RIPEMD160_GGG(aaa, bbb, ccc, ddd, eee, tmp[ 4],  8);
	RIPEMD160_GGG(eee, aaa, bbb, ccc, ddd, tmp[ 1], 11);
	RIPEMD160_GGG(ddd, eee, aaa, bbb, ccc, tmp[ 3], 14);
	RIPEMD160_GGG(ccc, ddd, eee, aaa, bbb, tmp[11], 14);
	RIPEMD160_GGG(bbb, ccc, ddd, eee, aaa, tmp[15],  6);
	RIPEMD160_GGG(aaa, bbb, ccc, ddd, eee, tmp[ 0], 14);
	RIPEMD160_GGG(eee, aaa, bbb, ccc, ddd, tmp[ 5],  6);
	RIPEMD160_GGG(ddd, eee, aaa, bbb, ccc, tmp[12],  9);
	RIPEMD160_GGG(ccc, ddd, eee, aaa, bbb, tmp[ 2], 12);
	RIPEMD160_GGG(bbb, ccc, ddd, eee, aaa, tmp[13],  9);
	RIPEMD160_GGG(aaa, bbb, ccc, ddd, eee, tmp[ 9], 12);
	RIPEMD160_GGG(eee, aaa, bbb, ccc, ddd, tmp[ 7],  5);
	RIPEMD160_GGG(ddd, eee, aaa, bbb, ccc, tmp[10], 15);
	RIPEMD160_GGG(ccc, ddd, eee, aaa, bbb, tmp[14],  8);

	/* parallel round 5 */
	RIPEMD160_FFF(bbb, ccc, ddd, eee, aaa, tmp[12],  8);
	RIPEMD160_FFF(aaa, bbb, ccc, ddd, eee, tmp[15],  5);
	RIPEMD160_FFF(eee, aaa, bbb, ccc, ddd, tmp[10], 12);
	RIPEMD160_FFF(ddd, eee, aaa, bbb, ccc, tmp[ 4],  9);
	RIPEMD160_FFF(ccc, ddd, eee, aaa, bbb, tmp[ 1], 12);
	RIPEMD160_FFF(bbb, ccc, ddd, eee, aaa, tmp[ 5],  5);
	RIPEMD160_FFF(aaa, bbb, ccc, ddd, eee, tmp[ 8], 14);
	RIPEMD160_FFF(eee, aaa, bbb, ccc, ddd, tmp[ 7],  6);
	RIPEMD160_FFF(ddd, eee, aaa, bbb, ccc, tmp[ 6],  8);
	RIPEMD160_FFF(ccc, ddd, eee, aaa, bbb, tmp[ 2], 13);
	RIPEMD160_FFF(bbb, ccc, ddd, eee, aaa, tmp[13],  6);
	RIPEMD160_FFF(aaa, bbb, ccc, ddd, eee, tmp[14],  5);
	RIPEMD160_FFF(eee, aaa, bbb, ccc, ddd, tmp[ 0], 15);
	RIPEMD160_FFF(ddd, eee, aaa, bbb, ccc, tmp[ 3], 13);
	RIPEMD160_FFF(ccc, ddd, eee, aaa, bbb, tmp[ 9], 11);
	RIPEMD160_FFF(bbb, ccc, ddd, eee, aaa, tmp[11], 11);

	ddd += cc + state[1];
	state[1] = state[2] + dd + eee;
	state[2] = state[3] + ee + aaa;
	state[3] = state[4] + aa + bbb;
	state[4] = state[0] + bb + ccc;
	state[0] = ddd;
}

void HASH_RIPEMD160InitMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	ecpMem_Copy(myInitParam->hashResult, myAPIParam->pICV, myAPIParam->digestSize);
//	ecpMem_Copy(myInitParam->hashWorkSpace, myInitParam->inDataBuffer, myInitParam->remainLength);
	return;
}


void HASH_RIPEMD160UpdateMain(HASH_Init_Param_t * myInitParam)
{
	RIPEMD160Transform((u32 *)myInitParam->hashWorkSpace,(u32 *)myInitParam->hashResult);
	return;
}


void HASH_RIPEMD160FinalizeMain(HASH_Init_Param_t * myInitParam, HASH_API_Param_t * myAPIParam)
{
	u8	 		X_A 		tempLen;
	u8	 		X_A 		remainLength;
	u32			X_A			bitNum;

	remainLength = myInitParam->remainLength;
	myInitParam->hashWorkSpace[remainLength] = 0x80;  // padding 0x80
	remainLength++;

	// append bit ( not byte ) length of unpadded message as 64-bit big-endian integer to message
	// To RIPEMD160 the max update message length is 16383 bytes, the max dofinal message length is 32767 bytes,
	tempLen = myAPIParam->blockSize - remainLength;

	ecpMem_Fill(&myInitParam->hashWorkSpace[remainLength], 0, tempLen);

	if (tempLen < myAPIParam->msgBitByteLen)
	{
		// Launch the SHA transform
		RIPEMD160Transform((u32 *)myInitParam->hashWorkSpace,(u32 *)myInitParam->hashResult);

		ecpMem_Fill(myInitParam->hashWorkSpace, 0, myAPIParam->blockSize);
	}

	bitNum = (u32)(((((u32)myInitParam->blockNum) << myAPIParam->blockNumShift) + myInitParam->remainLength) << 3);

	// append bit ( not byte ) length of unpadded message as 64-bit little-endian integer to message.
		myInitParam->hashWorkSpace[56] = (u8)(bitNum & 0xFF);
		myInitParam->hashWorkSpace[57] = (u8)(bitNum >> 8);
		myInitParam->hashWorkSpace[58] = (u8)(bitNum >> 16);
//		myInitParam->hashWorkSpace[59] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[60] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[61] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[62] = (u8)(0x00);
	//	myInitParam->hashWorkSpace[63] = (u8)(0x00);

	// Launch the SHA transform
	RIPEMD160Transform((u32 *)myInitParam->hashWorkSpace,(u32 *)myInitParam->hashResult);

#if U8_2_U32_INVERSE
	// nothing to do!
#else
	BigEndian2LittleEndian((PX8)myInitParam->hashResult, (u16)(myAPIParam->digestSize));
#endif

	return;
}
#endif // WITH_RIPEMD160

