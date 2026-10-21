/*
 * common_API.c
 *
 *  Created on: 2014-2-19
 *      Author: jiangxiaohui
 */

#include "Crypto_Constant.h"
#include "Crypto_common_API.h"
#include "SysConfigTarget.h"
#include "SysConfigHalRam.h"
#include "SecOperationHal.h"
#include "RSA_API.h"
#include "RNG_API.h"
#include "chipHead.h"

//以下两个全局变量于20220521在EPX/v3.0.1/BRANCH_MODULE/INFENION_SLX97/DEVELOP下被小辉给删除
//u8 X_A g_SM4NormalFlag; //是否是标准的SM4，0表示非标准SM4，1表示标准SM4
//u8		g_SymGuomiPro;

/**********************************************************************************
 * byte edition of BigEndian2LittleEndian
 **********************************************************************************/
void BigEndian2LittleEndian(PX8 InOutBuf, u16 inputLen)
{
	u8 tmp;
	inputLen >>= 2;

	while(inputLen--)
	{
		tmp = *InOutBuf;
		*InOutBuf = *(InOutBuf + 3);
		*(InOutBuf + 3) = tmp;

		tmp = *(InOutBuf + 1);
		*(InOutBuf + 1) = *(InOutBuf + 2);
		*(InOutBuf + 2) = tmp;

		InOutBuf += 4;
	}
}

void dataReversal(PM8 InOutBuf, u16 inputLen)
{
	u8 temp;
	u16 i;
	u16 j;
	if(inputLen > 1)
	{
		for(i = inputLen-1,j = 0;(i >= inputLen/2)&&(j <= inputLen/2-1);i--,j++)
		{
			temp = *(InOutBuf+j);
		   *(InOutBuf+j) = *(InOutBuf+i);
		   *(InOutBuf+i) = temp;
		}
    }
}
u8 checkSrcIsAllZero(PX8 src, u8 count)
{
    u8 result;

    result = true;
    while ( count )
    {
    	count --;
        if ( src[count] != 0 )
        {
        	result = false;
            break;
        }
    }
    return result;
}

void ecpAXorB(PX8 pointA, PM8 pointB, PX8 pointC, u16 count)
{
	u16 	D_A		i;
	for(i=0; i< count; i++)
	{
		PX_W_U8(pointC, U8_XOR_U8(PX_R_U8(pointA),memReadU8(pointB)));
		PX8_INC(pointA);
		PM8_INC(pointB);
		PX8_INC(pointC);
	}
	return ;
}

void ecpAXorConst(PX8 pointA, u8 secValue, u16 length)
{
	u16 	D_A		i;
	u8 X_A  randomByte;
	PX8 point;
	point = pointA;
	
	do {
		SC_FUNCTION_EXC_INIT_STATUS();
		Random_number_generator(&randomByte, 1, ALG_SECURE_RANDOM);
		SC_FUNCTION_EXC_CHECK_STATUS();
	} while(randomByte == 0);
	
	secValue = U8_XOR_U8(secValue, randomByte);
	
	for(i=0; i< length; i++)
	{
		PX_W_U8(point, U8_XOR_U8(U8_XOR_U8(PX_R_U8(point), secValue), randomByte));
		PX8_INC(point);
	}
	return ;
}
u8 reflectChar(u8 inNum)
{
	u8		D_A		i;
	u8		D_A		reflection;

	reflection 		= 0x00;

	// Reflect the data about the center bit.
	for (i = 0; i < 8; ++i)
	{
		// If the LSB bit is set, set the reflection of it
		if (inNum & 0x01)
		{
			reflection |= U8_SHL_U8(1 , U8_S_U8(7 , i));  //(lzm modified according [MISA])
		}
		inNum = (inNum >> 1);
	}
	return reflection;
}

/*********************************************************************
 * Function name: reflectShort()
 * Description: Reorder the bits of a binary sequence, by reflecting
 *              them about the middle position.//就是将输入数据倒序排列，比如10101010101010变成0101010101010101;
 * Input Parameters:  (precondition)
 *      u16 inNum           input number
 * Output Parameters:  (postcondition)
 *      The reflection of the original data
 * Notes:
 *********************************************************************/
u16 reflectShort(u16 inNum)
{
	u8		D_A		i;
	u16		D_A		reflection;

	reflection 		= 0x00;

	// Reflect the data about the center bit.
	for (i = 0; i < 16; ++i)
	{
		// If the LSB bit is set, set the reflection of it
		if (inNum & 0x01)
		{
			reflection |= ((u16)1 << (15 - i));
		}
		inNum = (inNum >> 1);
	}
	return reflection;
}
u32 reflectLong(u32 inNum)
{
	u8		D_A		i;
	u32		D_A		reflection;

	reflection 		= 0x00;

	// Reflect the data about the center bit.
	for (i = 0; i < 32; ++i)
	{
		// If the LSB bit is set, set the reflection of it
		if (inNum & 0x01)
		{
			reflection |= ((u32)1 << (31 - i));
		}
		inNum = (inNum >> 1);
	}
	return reflection;
}


void ecpMem_ReflectChar(PX8 outBuffer, PM8 inBuffer, u16 inLength)
{
	u16		D_A		i;
	for(i = 0; i < inLength; i++)
	{
	    outBuffer[i] = reflectChar(inBuffer[i]);
	}
}


u16 Get_crc16(PM8 pu1CRC, u32 u1CRCLength)
{
	u16 crc;
	u8 i;
	crc = 0;
	while (u1CRCLength != 0)
	{
		U32_DEC(u1CRCLength);
		//			crc = crc ^ (u16) *pu1CRC++ << 8;
		crc = U16_XOR_U16(crc, PM_R_U8(pu1CRC) << 8);
		PM8_INC(pu1CRC);
		i = 8;
		do
		{
			if (crc & 0x8000)
			{
				crc = U16_XOR_U16(crc << 1, 0x1021);
			}
			else
			{
				crc = crc << 1;
			}
		} while (--i);
	}
	return (crc);
}


/*********************************************************************
 * Function name: BigData_Padding
 *							   
 *  padding data and return the total dataLeng
 *	example:
 *		-->u8	InBuffer[7] = {0x01,0x02,0x03,0x04,0x05};
 *		-->		Inlength    = 5;		  	
 *
 *	  	-->     option : four Byte align padding from Top
 *		<--		OutBuffer[8]= {0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05};
 *		<--		OutLength   = 8;
 *					
 *	  	-->     option : two Byte align padding from Bot
 *		<--		OutBuffer[6]= {0x01,0x02,0x03,0x04,0x05,0x00};
 *		<--		OutLength   = 6;
 *
 *		-->     other	  (rfu)
 *
 *  padding
 *********************************************************************/

u16 BigData_Padding(PM8 InBuffer, u16 Inlength, PX8 OutBuffer, u8 option)
{
	PX8		X_A			tempPtr;
	u16		X_A			resultLength;  	 
	u8		X_A			optionMask;
	u8		X_A			paddingLength;

	// padding align byte	  (for test 0x08)
	optionMask			= U8_AND_U8(option , BIG_DATA_PADDING_ALIGN_MASK);	
	if(0x00 == optionMask)
	{
		return Inlength;
	}

	// remain byte			  (for test Inlength = 0x13, then paddingLength = 0x03)
	paddingLength		= U8_S_U8(optionMask , 0x01);
	paddingLength		= U8_AND_U8(Inlength , paddingLength);

	// padding length		  (for test the result is 0x05)
	if(0x00 != paddingLength)
	{
		paddingLength		= U8_S_U8(optionMask , paddingLength);
	}

	// the result length	  (for test the result is 0x18)
	resultLength		= U16_A_U16(Inlength, paddingLength);	 
	tempPtr				= OutBuffer;

	if(BIG_DATA_PADDING_FROM_TOP == U8_AND_U8(option, BIG_DATA_PADDING_FROM_MASK))
	{					 
		ecpMem_Set(tempPtr, 0x00 , paddingLength);
		tempPtr			= PX8_A_U16(OutBuffer, paddingLength);
		ecpMem_Copy(tempPtr, InBuffer, Inlength);	
	}	
	else
	{								 
		ecpMem_Copy(tempPtr, InBuffer, Inlength);	
		tempPtr			= PX8_A_U16(OutBuffer, Inlength);  
		ecpMem_Set(tempPtr, 0x00 , paddingLength);
	}
	return resultLength;
}


/*********************************************************************
 * Function name: BigData_Mul
 *
 * result = d1*d2
 *********************************************************************/
u16 BigData_Mul(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type)
{
//	u8 					nump[MAX_RSA_MUD_BYTE_LEN];
//	u8 					i;
//	miracl 				* mip;
//    u16 				dataLen;
//    u16 				digitNum;
//    big 				data1Buf;
//    big 				data2Buf;
//    big 				resultBuf;
//
//    digitNum  			= (d1_len+d2_len)<<1;
//    mip 				= mirsys(digitNum, 16);
//
//    data1Buf 				= mirvar(0);
//    data2Buf 				= mirvar(0);
//    resultBuf				= mirvar(0);
//
//    mip->IOBASE 		= 16;
//    ecpMem_Copy(nump, d1_Addr, d1_len);
//    bytes_to_big(d1_len, nump , data1Buf);
//
//    ecpMem_Copy(nump, d2_Addr, d2_len);
//    bytes_to_big(d2_len, nump , data2Buf);
//
//    multiply(data1Buf, data2Buf, resultBuf);
//
//	dataLen =  d1_len + d2_len;
//	//bigbits(dataLen, resultBuf);
//    big_to_bytes(dataLen, resultBuf, nump, TRUE);
//
//	i = 0;
//	while(nump[i] == 0)
//	{
//	     i++;
//	     dataLen--;
//	}
//	if(type == true)
//	{
//		ecpMem_Copy(result, &nump[i], dataLen);
//	}
//	mirexit();
//    return dataLen;
	return 0;
}

/*********************************************************************
 * Function name: BigData_Add
 *
 * result = d1 + d2
 *********************************************************************/
u16 BigData_Add(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type)
{
	return 0;
}

/*********************************************************************
 * Function name: BigData_Add
 *
 * result = d1 - d2
 *********************************************************************/
u16 BigData_Sub(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type)
{
		return 0;
}
#if WITH_RSA
#if RSA_CRT_E_SAVE
u8 RSA_CalE_Hal(PM8 pBuff, PM8 qBuff, PM8 DpBuff, PM8 DqBuff, PX8 EBuff, u16 primeLen)
{
	return RSA_CRT_CALE_NOK;
}

#endif //RSA_CRT_E_SAVE

#define  RSA_CRT_PINV	0x01
#define  RSA_CRT_QINV	0x02

/**************************************************************************
*Function:
*	RSA_CRTKeyPinv_Cal
*
*Params:
*	tempPinvbuff: 存储inv数据
*	tempOutbuff: 存储P和Q(Q|P)
*	realKeyLen:CRTKEY真实长度
*
*Description:
*
*	判断是否是Qinv:
*	 	(Q * Xinv) mod P = 1
*	 
*	判断是否是Pinv:                               
*	 	(P * Xinv) mod Q = 1
*revised by 蒋小辉@20220920因为boot层对QIN,PIN相关的代码做了处理，simulator这种情况也需要对应增加。
***************************************************************************/

u8 RSA_CRTKeyPinv_Cal(PM8 pBuf, PM8 qBuf, PM8 dpBuf, PM8 dqBuf, PM8 qinvBuf, u16 keyLen)
{	
	u8 tempBuffer[MAX_RSA_P_BYTE_LEN];
	u16 i;
	
	ecpAMulBModC(pBuf, keyLen, qinvBuf, qBuf, tempBuffer);
	i = 0;
	while(tempBuffer[i] == 0)
	{
		 i++;
		 keyLen--;
	}

	if((keyLen ==1) && tempBuffer[i] == 0x01)
	{
		return RSA_CRT_PINV;
	}
	else
	{
		return RSA_CRT_QINV;
	}
}


#endif /*WITH_RSA*/

u8 ecpIsRsaInProgress(void)
{
	return 0;
}

u8 ecpRsaSuspend(void)
{
	return 0;
}

void ecpRsaRestart(u8 RSAprocess)
{
	return;
}

void random_delay()
{
	u8 X_A randomByte;
	do {
		SC_FUNCTION_EXC_INIT_STATUS();
		Random_number_generator(&randomByte, 1, ALG_SECURE_RANDOM);
		SC_FUNCTION_EXC_CHECK_STATUS();
	} while(randomByte == 0);
	
	while(randomByte--);
}

u8 ecpAXorGetChecksum(PX8 pointA, u8 length)
{
	u8 	X_A		i;
	u8  X_A   result;
	PX8 point;
	point = pointA;
	result = 0;
	for (i = 0; i < length; i++)
	{
		result = U8_XOR_U8(PX_R_U8(point), result);
		PX8_INC(point);
	}
	return result;
}

void clearCryptoSpace(void)
{
	TrngCacheInit();
	
	//g_SM4NormalFlag = NonnormalSM4;
	return;
}



/*********************************************************************
 * Function name: ecpAMulBModC()
 *
 * D = (A * B) mod C
 *********************************************************************/
void ecpAMulBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{
	u8 					nump[256];
	big aPoint, bPoint, cPoint, dPoint;

	miracl *mip;
	u16 digitNum;

	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	aPoint = mirvar(0);
	bPoint = mirvar(0);
	cPoint = mirvar(0);
	dPoint = mirvar(0);

	mip->IOBASE = 16;
	ecpMem_Copy(nump, APoint, byteLen);
	bytes_to_big(byteLen, nump, aPoint);
	ecpMem_Copy(nump, BPoint, byteLen);
	bytes_to_big(byteLen, nump, bPoint);
	ecpMem_Copy(nump, CPoint, byteLen);
	bytes_to_big(byteLen, nump, cPoint);

	mad(aPoint, bPoint, aPoint, cPoint, cPoint, dPoint);

    big_to_bytes(byteLen, dPoint, nump, true);
	ecpMem_Copy(DPoint, nump, byteLen);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

    return;
}
