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
#include "EllipseCurve_API.h"


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
u8 checkSrcIsAllZero(PX8 src, u16 count)
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
		memWriteU8NoAtomic(pointC, U8_XOR_U8(memReadU8(pointA),memReadU8(pointB)));
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

/*支持传入初始向量的CRC计算函数, added by xuweihua@20211227*/
u16 Get_crc16WithIV(u16 crcIV,PM8 pu1CRC, u32 u1CRCLength)
{
	u16 crc;
	u8 i;
	crc = crcIV;
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

#if WITH_RSA
/*********************************************************************
 * Function name: BigData_Mul
 *
 * result = d1*d2
 *********************************************************************/
u16 BigData_Mul(PX8 result,PM8 d1_Addr,u16 d1_len, PM8 d2_Addr, u16 d2_len ,u8 type)
{
//	u32		X_A		data1Buf[MAX_RSA_MUD_BYTE_LEN >> 2];
//  u32		X_A		data2Buf[MAX_RSA_MUD_BYTE_LEN >> 2];
//  u32		X_A		resultBuf[MAX_RSA_MUD_BYTE_LEN >> 2];  
//	PX32  X_A   PKEModel;
//	PX8		X_A		resultPtr;
//  u16 	X_A		resultLen;
//	u8		X_A		offSet;	
//	u8    X_A   zeroLen;
//	u8    X_A   realLenth;

//	offSet			= U16t8(d1_len & 0x03);
//	realLenth       = d1_len;
//	if(0x00 != offSet)
//	{
//		offSet 	 	=  U8_S_U8(4,offSet);
//		realLenth   =  U8_A_U8(realLenth,offSet);
//		data1Buf[U16_SHR_U8(realLenth,2)] = 0 ;
//		data2Buf[U16_SHR_U8(realLenth,2)] = 0 ;
//	}
//	
//	ecpMem_Fill(data1Buf, 0x00, realLenth);
//	ecpMem_Copy(PX8_A_U8(data1Buf, U8_A_U8(realLenth,offSet)), d1_Addr, d1_len);
//	ecpMem_Fill(data2Buf, 0x00, realLenth);
//	ecpMem_Copy(PX8_A_U8(data2Buf, U8_A_U8(realLenth,offSet)), d2_Addr, d2_len);
//	
//	resultLen 		= U16_SHL_U8(realLenth, 1);
//	
// //do Big Data Mul
//	 
//	//1. open PKE
//	PKE_Open();
//	
//	// 2.Assign the init Value of PKE model 
//	PKEModel = resultBuf;
//	ecpMem_Set(MEMtPX8(PKEModel), 0xFF, resultLen);
//	
//	//3.SET the PKE model
//	RSA_SetModulus(PKEModel, U16_SHL_U8(resultLen,3), PKEModel);
//	
//	//4.Mod Mul
//	RSA_BIGINT_ModMul(data1Buf, data2Buf, resultBuf);
//	
//	//5. close PKE
//	PKE_Close();	
//	//End Big Data Mul

//	resultPtr		= MEMtPX8(resultBuf);

//	for(zeroLen = 0; zeroLen < resultLen; U16_INC(zeroLen))
//	{
//		if(0x00 != resultPtr[zeroLen])
//		{
//			break;
//		}
//	}
//   return  U8t16(resultLen - zeroLen);
return 0;
}
#endif
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
#if RSA_CRT_E_SAVE
u8 RSA_CalE_Hal(PM8 pBuff, PM8 qBuff, PM8 DpBuff, PM8 DqBuff, PX8 EBuff, u16 primeLen)
{
//	BigNum_t 					MK_RSA;
//	BigNum_t 					outputBuf;
//	RSAPrivKeyCrt_t 			RSA_PrivKeyCrt;
//			
//	u8 							mask[8];
//	u8 							eData[MAX_RSA_MUD_BYTE_LEN];
//	
//	MK_RSA.p 					= mask;
//	MK_RSA.length 				= sizeof(mask);
//	ecpMem_Set(mask , 0x00, MK_RSA.length);
//	
//	/* RSA-P */
//	RSA_PrivKeyCrt.P.p 			= pBuff;
//	RSA_PrivKeyCrt.P.length 	= primeLen;	

//	/* RSA-Q */
//	RSA_PrivKeyCrt.Q.p			= qBuff;
//	RSA_PrivKeyCrt.Q.length 	= primeLen;
//	
//	/* RSA-DP */
//	RSA_PrivKeyCrt.DP.p 		= DpBuff;
//	RSA_PrivKeyCrt.DP.length 	= primeLen;
//	
//	/* RSA-DQ */
//	RSA_PrivKeyCrt.DQ.p 		= DqBuff;
//	RSA_PrivKeyCrt.DQ.length 	= primeLen;

//	outputBuf.p 				= eData;
//	
//	if(ALG_OK == tmcRSACrtGetE(&outputBuf,  &(RSA_PrivKeyCrt.P), &(RSA_PrivKeyCrt.Q), &(RSA_PrivKeyCrt.DP), &(RSA_PrivKeyCrt.DQ), &MK_RSA))
//	{
//		if(outputBuf.length <= MAX_RSA_E_BYTE_LEN)
//		{
//			ecpMem_Set(EBuff, 0x00, MAX_RSA_E_BYTE_LEN - outputBuf.length);
//			ecpMem_Copy(MEMtPM8(&EBuff[MAX_RSA_E_BYTE_LEN - outputBuf.length]), outputBuf.p, outputBuf.length);
//			return RSA_CRT_CALE_OK;
//		}
//	}
	return RSA_CRT_CALE_NOK;
}

#endif
#if WITH_RSA

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
*
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
//	return (PKESTS & THD89_PKESTS_HALTED);
}

u8 ecpRsaSuspend(void)
{
	return true;
}

void ecpRsaRestart(u8 RSAprocess)
{
//	PKESTS 		|= THD89_PKESTS_HALTED;
//	PKECON2 	|= THD89_PKECON2_CONTINUE;
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
//	__disable_irq();
//	ecpMem_Set(MEMtPX8(PHY_RAM_BASE), 0x00, 0xF00);
//	__enable_irq();

	//Initialize CRAM
	MHCRYPT_CramInit();
//	ecpMem_Set(MEMtPX8(RAM1_BUFFER_ADDRESS), 0x00, 0x1000);
	
//	g_SM4NormalFlag = NonnormalSM4;
	return;
}

/*********************************************************************
 * Function name: ecpAAddB()
 * AbyteLen must not be smaller than BbyteLen
 * C = (A + B)
 *********************************************************************/
void ecpAAddB(PX8 APoint, u16 AbyteLen, PX8 BPoint, u16 BbyteLen, PX8 CPoint)
{
//#define ADD_MOD_BIG_A_OFFSET							0
//#define ADD_MOD_BIG_B_OFFSET							(ADD_MOD_BIG_A_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//#define ADD_MOD_BIG_C_OFFSET							(ADD_MOD_BIG_B_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//#define ADD_MOD_BIG_BUFFER_LENGTH						(ADD_MOD_BIG_C_OFFSET + (MAX_RSA_MUD_BYTE_LEN >> 2))
//	u32						bigCalReuseBuffer[ADD_MOD_BIG_BUFFER_LENGTH];

//	TCLONG 					At, Bt, Ct;  // CLONGs for generating the RSA key
//	CLIB_STATUS 			clResult;   // clResult in order to check the API
//	u16 					wByteLen;
//	u8						dataOff;

//	wByteLen 				= (AbyteLen + 3) & 0xFFFC;
//	
//	dataOff					= (u8)(AbyteLen & 0x03);
//	if(0 != dataOff)
//	{
//		dataOff = U8_S_U8(4, dataOff);
//		bigCalReuseBuffer[ADD_MOD_BIG_A_OFFSET] = 0;
//		bigCalReuseBuffer[ADD_MOD_BIG_B_OFFSET] = 0;
//	}
//	
//	ecpMem_Copy(PX8_A_U8(&bigCalReuseBuffer[ADD_MOD_BIG_A_OFFSET], dataOff), APoint, AbyteLen);
//	ecpMem_Copy(PX8_A_U8(&bigCalReuseBuffer[ADD_MOD_BIG_B_OFFSET], dataOff), BPoint, BbyteLen);

//	LOADTCLONG(At, MAX_RSA_MUD_BYTE_LEN, wByteLen << 3, (unsigned int *)&bigCalReuseBuffer[ADD_MOD_BIG_A_OFFSET], OPERAND_A_CLONG_COOKIE, 0x125E);
//	LOADTCLONG(Bt, MAX_RSA_MUD_BYTE_LEN, wByteLen << 3, (unsigned int *)&bigCalReuseBuffer[ADD_MOD_BIG_B_OFFSET], OPERAND_B_CLONG_COOKIE, 0x125E);
//	LOADTCLONG(Ct, MAX_RSA_MUD_BYTE_LEN, wByteLen << 3, (unsigned int *)&bigCalReuseBuffer[ADD_MOD_BIG_C_OFFSET], OUTPUT_CLONG_COOKIE, 0x125E);


//	/// call infineon sle97 crypto lib
//	// open CryptoLibrary
//	clResult = CryptoOpenLibrary();
//	if (CLIB_FAILED(clResult)) // if failed, then end
//	{
//		while(1);
//	}
//	
//	clResult = CryptoAdd(&At, &Bt, &Ct);
//	
//	CryptoCloseLibrary();

//	ecpMem_Copy(CPoint, PX8_A_U16(Ct.super_.Data, dataOff), AbyteLen);

    return;
}


/*********************************************************************
 * Function name: ecpAAddConst()
 * A = (A + ConstValue)
 *********************************************************************/
void ecpAAddConst(PX8 APoint, PX8 BPoint, u16 AbyteLen, u8 constValue)
{
	u8 cPoint[32];
	cPoint[31] = constValue;
	memset(&cPoint[0], 0, 31);
	ecpAAddB(APoint, AbyteLen, cPoint,  sizeof(cPoint) , BPoint);
    return;
}

/*********************************************************************
 * Function name: ecpACompareB()
 *  Description: Big number Y substract Big number Z, the result is
 *               always put in big number Y.
 *********************************************************************/
u8 ecpACompareB(PX8 APoint, PX8 BPoint, u16 AbyteLen)
{
//	big aPoint, bPoint;
//	miracl *mip;
//	u16 digitNum;
//	u8 result;

//	digitNum  = AbyteLen << 1;
//	mip = mirsys(digitNum, 16);

//	aPoint = mirvar(0);
//	bPoint = mirvar(0);

//	mip->IOBASE = 16;
//	bytes_to_big(AbyteLen, APoint, aPoint);
//	bytes_to_big(AbyteLen, BPoint, bPoint);

//	/*
//	 * Compares two big numbers Parameters:
//	 * ←x
//	 * ←y
//	 * Returns:
//	 * +1 ifx>y; 0 if x=y; -1 ifx<y
//	 * */
//	result = mr_compare(aPoint, bPoint);


//    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//    //A subsequent call to mirsys() will re-initialise the MIRACL system
//    //Must be called after mirsys()
//    mirexit();

//	if(0x01 == result)
//	{
//		return A_IS_BIGGER_THAN_B;
//	}
//	else if(0x00 == result)
//	{
//		return A_IS_EQUAL_WITH_B;
//	}
//	else
//	{
//		return A_IS_SMALLER_THAN_B;
//	}
}
#define MOD_ADD				0x853A
#define MOD_SUB				0x4E83
#define MOD_MUL				0x15AC
#define MOD_INV				0xB853
#define MOD_INVMUL		0xAC25

void ecpBigNum(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint,u16 type)
{
	u8		resultBuf[256],aBuf[256],bBuf[256],cBuf[256];  
	MHBN_BIGNUM   A,B,C,D;
	u32 	result;
	result = 0;

	ecpMem_Set(MEMtPX8(resultBuf), 0x00, byteLen);
	ecpMem_Copy(aBuf,	APoint, byteLen);
	ecpMem_Copy(cBuf,	CPoint, byteLen);
	
	LOADBIGNUM(A,&aBuf[0],byteLen);
	LOADBIGNUM(C,&cBuf[0],byteLen);
	LOADBIGNUM(D,&resultBuf[0],byteLen);
	
	while(MH_CMP_BIGGER!=MHBN_BnCmp(&C,&A))
	{
		MHBN_BnSub(&A,&C,&A);
	}
	
	if(MOD_INV != type)
	{
		ecpMem_Copy(bBuf,	BPoint, byteLen);
		LOADBIGNUM(B,&bBuf[0],byteLen);
		while(MH_CMP_BIGGER!=MHBN_BnCmp(&C,&B))
		{
			MHBN_BnSub(&B,&C,&B);
		}
	}
	
	switch(type)
	{
		case MOD_ADD:
			result = MHBN_BnModAdd(&D,&A,&B,&C);
			break;
		case MOD_SUB:
			result = MHBN_BnModSub(&D,&A,&B,&C);
			break;
		case MOD_MUL:
			result = MHBN_BnModMul(&D,&A,&B,&C);
			break;
		case MOD_INV:
			result = MHBN_BnModInv(&D,&A,&C);
			break;
		case MOD_INVMUL:
			result = MHBN_BnModInv(&D,&A,&C);
			result -= MH_RET_CRYPT_SUCCESS;
			result += MHBN_BnModMul(&D,&D,&B,&C);
			break;
		default:
			result = MH_RET_CRYPT_PARAM_ERR;
			break;
	}
	
	if(MH_RET_CRYPT_SUCCESS != result)
	{
		while(1);
	}
	
	memCopyNoAtomic(DPoint, resultBuf, byteLen);
	
  return;
}



/*********************************************************************
 * Function name: ecpAAddBModC()
 * Description: Big number Y add big number Z in case of overflow, and
 *              then the result is reduced by modulus N, the final
 *              result is put in operandY.
 * D = (A + B) mod C
 *********************************************************************/
void ecpAAddBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{
	ecpBigNum(APoint,byteLen,BPoint,CPoint,DPoint,MOD_ADD);
	
  return;
}



/*********************************************************************
 * Function name: ecpASubBModC()
 *
 * D = (A - B) mod C
 *********************************************************************/
void ecpASubBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{
	ecpBigNum(APoint,byteLen,BPoint,CPoint,DPoint,MOD_SUB);
	
  return;
}


///*********************************************************************
// * Function name: ecpAMulBModC()
// *
// * D = (A * B) mod C
// *********************************************************************/
void ecpAMulBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{
	ecpBigNum(APoint,byteLen,BPoint,CPoint,DPoint,MOD_MUL);

  return;
}

/*********************************************************************
 * Function name: ecpAInvMultBModC()
 *
 * D = (A^(-1) * B) mod C
 *********************************************************************/
void ecpAInvMultBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{
	ecpBigNum(APoint,byteLen,BPoint,CPoint,DPoint,MOD_INVMUL);
	
	return;
}


/*********************************************************************
 * Function name: ecpAInvMultBModC()
 *
 * D = (A^(-1)) mod C
 *********************************************************************/
void ecpAInvModC(PX8 APoint, u16 byteLen, PX8 CPoint, PX8 DPoint)
{
	ecpBigNum(APoint,byteLen,NULL,CPoint,DPoint,MOD_INV);
	
	return;
}
