/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
			V1.1 增加ISIM鉴权功能，需定义宏(#define ISIM) --xuqs @20140408

 */
#include	"string.h"
#include 	"uicc_main.h"
//#include 	"main.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
//#include 	"resume.h"
#include 	"Authentication.h"
#include    "stkhead.h"
#include 	<string.h>			 
#include <uicc_error.h>
//#include "sha1.h"
#include "tuak.h"
#include "profileApi.h"

//BYTE xdata roundKeys[11][4][4];		//用于AES的Key


//BYTE xdata state[4][4];				//用于AES算法的全局数组
//
//BYTE code S_uicc[256] =
//{
//	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
//	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
//	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
//	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
//	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
//	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
//	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
//	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
//	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
//	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
//	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
//	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
//	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
//	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
//	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
//	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
//};
//
//BYTE code Xtime[256] =
//{
//	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
//	0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
//	0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
//	0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E,
//	0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E,
//	0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBE,
//	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
//	0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE,
//	0x1B, 0x19, 0x1F, 0x1D, 0x13, 0x11, 0x17, 0x15, 0x0B, 0x09, 0x0F, 0x0D, 0x03, 0x01, 0x07, 0x05,
//	0x3B, 0x39, 0x3F, 0x3D, 0x33, 0x31, 0x37, 0x35, 0x2B, 0x29, 0x2F, 0x2D, 0x23, 0x21, 0x27, 0x25,
//	0x5B, 0x59, 0x5F, 0x5D, 0x53, 0x51, 0x57, 0x55, 0x4B, 0x49, 0x4F, 0x4D, 0x43, 0x41, 0x47, 0x45,
//	0x7B, 0x79, 0x7F, 0x7D, 0x73, 0x71, 0x77, 0x75, 0x6B, 0x69, 0x6F, 0x6D, 0x63, 0x61, 0x67, 0x65,
//	0x9B, 0x99, 0x9F, 0x9D, 0x93, 0x91, 0x97, 0x95, 0x8B, 0x89, 0x8F, 0x8D, 0x83, 0x81, 0x87, 0x85,
//	0xBB, 0xB9, 0xBF, 0xBD, 0xB3, 0xB1, 0xB7, 0xB5, 0xAB, 0xA9, 0xAF, 0xAD, 0xA3, 0xA1, 0xA7, 0xA5,
//	0xDB, 0xD9, 0xDF, 0xDD, 0xD3, 0xD1, 0xD7, 0xD5, 0xCB, 0xC9, 0xCF, 0xCD, 0xC3, 0xC1, 0xC7, 0xC5,
//	0xFB, 0xF9, 0xFF, 0xFD, 0xF3, 0xF1, 0xF7, 0xF5, 0xEB, 0xE9, 0xEF, 0xED, 0xE3, 0xE1, 0xE7, 0xE5
//};

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Rotata
  
  Data   content:    input[16]
  					 output[16]
					 r                                                   
                                                                                                                                                
  Description:        

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
void Rotata(BYTE input[16],BYTE output[16],BYTE r)                                     
{                                                                                                    
	BYTE idata i, shift;
	BYTE xdata temp[16];

	shift = r >> 3;
	
	for(i=0; i<16; i++)
	{
		output[(i + 16 - shift) &0x0F] = input[i];	
	}                                                                     
                                                                                                     
    shift = r & 0x07;
	
	if(shift != 0x00)
	{
		for(i=0;i<16;i++)
		{
			temp[i] = ((output[i] << shift) | (output[(i+1)&0x0F] >> (8-shift)));
		}

		ecpMem_Copy(output, temp, 16);		
	}                                                                                                                    
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 KeySchedule
  
  Data   content:    Key[16]:	输入卡内鉴权的KI                                                    
                                                                                                                                                
  Description:       对AES Key全局变量进行初始化  

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
//void KeySchedule(BYTE key[16])
//{
//	BYTE idata roundConst;
//	BYTE idata i, j,k;
//
//	for (i=0; i<16; i++)
//	{
//		//roundKeys[0][i & 0x03][i >> 2] = key[i];
//		roundKeys[(i & 0x03)*4 + (i >> 2)] = key[i];
//	}
//
//	roundConst = 1;
//
//	for (i=1; i<11; i++)
//	{
//		//roundKeys[i][0][0] = S[roundKeys[i - 1][1][3]] ^ roundKeys[i - 1][0][0] ^ roundConst;
//		roundKeys[i*16] = S_uicc[roundKeys[(i - 1)*16+(1*4)+3]] ^ roundKeys[(i - 1)*16] ^ roundConst;
//
//		for(k=1;k<4;k++)
//		{
//			//roundKeys[i][k][0] = S[roundKeys[i - 1][(k+1)&0x03][3]] ^ roundKeys[i - 1][k][0];
//			roundKeys[(i*16)+(k*4)] = S_uicc[roundKeys[((i - 1)*16)+(((k+1)&0x03)*4)+3]] ^ roundKeys[((i - 1)*16)+(k*4)];
//		}
//
//		for (j = 0; j < 4; j++)
//		{
//			for(k=1;k<4;k++)
//			{
//				//roundKeys[i][j][k] = roundKeys[i - 1][j][k] ^ roundKeys[i][j][k-1];
//				roundKeys[(i*16)+(j*4)+k] = roundKeys[((i - 1)*16)+(j*4)+k] ^ roundKeys[(i*16)+(j*4)+(k-1)];
//			}
//		}
//
//		roundConst = Xtime[roundConst];
//	}
//
//	return;
//}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 KeyAdd
  
  Data   content:    round：	AES算法中循环移动的字节数                                                      
                                                                                                                                                
  Description:       AES算法子函数

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void KeyAdd(BYTE round)
//{
//	BYTE idata i, j;
//
//	for (i=0; i<4; i++)
//	{
//		for (j=0; j<4; j++)
//		{
//			//state[i][j] ^= roundKeys[round][i][j];
//			state[i][j] ^= roundKeys[(round*16)+(i*4)+j];
//		}
//	}
//
//	return;
//}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ByteSub
  
  Data   content:    void                                                      
                                                                                                                                                
  Description:       AES算法子函数

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void ByteSub(void)
//{
//	BYTE idata i, j;
//
//	for (i = 0; i < 4; i++)
//	{
//		for (j = 0; j < 4; j++)
//		{
//			state[i][j] = S_uicc[state[i][j]];
//		}
//	}
//
//	return;
//}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ShiftRow
  
  Data   content:    void                                                      
                                                                                                                                                
  Description:       AES算法子函数

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void ShiftRow(void)
//{
//	BYTE idata temp;
//
//	temp = state[1][0];
//
//	state[1][0] = state[1][1];
//
//	state[1][1] = state[1][2];
//
//	state[1][2] = state[1][3];
//
//	state[1][3] = temp;
//
//	temp = state[2][0];
//
//	state[2][0] = state[2][2];
//
//	state[2][2] = temp;
//
//	temp = state[2][1];
//
//	state[2][1] = state[2][3];
//
//	state[2][3] = temp;
//
//	temp = state[3][0];
//
//	state[3][0] = state[3][3];
//
//	state[3][3] = state[3][2];
//
//	state[3][2] = state[3][1];
//
//	state[3][1] = temp;
//
//	return;
//}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 MixColumn
  
  Data   content:    void                                                      
                                                                                                                                                
  Description:       AES算法

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void MixColumn(void)
//{
//	BYTE idata temp, tmp, tmp0;
//	BYTE idata i,k;
//
//	for (i = 0; i < 4; i++)
//	{
//		temp = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i];
//
//		tmp0 =  state[0][i];
//
//		for(k=0;k<3;k++)
//		{
//			tmp = Xtime[state[k][i] ^ state[(k+1)&0x03][i]];
//
//			state[k][i] ^= temp ^ tmp;
//		}
//
//		tmp = Xtime[state[3][i] ^ tmp0];
//
//		state[3][i] ^= temp ^ tmp;
//	}
//
//	return;
//}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 AESEncrypt
  
  Data   content:    input[16]:	输入的数据

  					 output[16]:输出的AES计算结果                                                      
                                                                                                                                                
  Description:       AES算法的主函数，参数input和output可以共用一个数组

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

//void AESEncrypt(BYTE input[16], BYTE output[16])
//{
//	BYTE idata i, r;
//
//	for (i=0; i<16; i++)
//	{
//		state[i & 0x3][i >> 2] = input[i];
//	}
//
//	KeyAdd(0);
//
//	for (r = 1; r <= 9; r++)
//	{
//		ByteSub();
//
//		ShiftRow();
//
//		MixColumn();
//
//		KeyAdd(r);
//	}
//
//	ByteSub();
//
//	ShiftRow();
//
//	KeyAdd(r);
//
//	for (i=0; i<16; i++)
//	{
//		output[i] = state[i & 0x03][i >> 2];
//	}
//
//	return;
//}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 StringSub
  
  Data   content:   
                                                                                                                                                
  Description:       str1减去str2的结果存入result中，要求str1和str2的长度一致

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

void StringSub(BYTE * str1, BYTE * str2, BYTE * result, BYTE len)
{
	BYTE idata i,carry;
	WORD idata tmp1, tmp2;

	carry = 0;

	for(i=len-1; ; i--)
	{
		tmp1 = str1[i];

		tmp2 = str2[i] + carry;
		
		if(tmp1 >= tmp2)
		{
			carry = 0x00;
		}
		else
		{
			tmp1 = str1[i] + 0x100;

			carry = 0x01;
		}

		result[i] = tmp1 - tmp2;

		if(i==0x00)
		{
			break;
		}
	}
}

/**
*wuronghua移植CIU98M25 平台时，把const误删掉,20210223. 根据EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop还原
*/
BYTE CheckSQN(void)
{
	UWORD 		xdata fileAddr,adfAddr;
	BYTE  		idata index;
	signed char 	idata i;
	BYTE  		xdata seq[6], seqms[6];
	BYTE		xdata delta[6], tmp[6], L[6];
	BYTE		idata ind, deltaC,algorithmID;
	BYTE        xdata sqnMax[6];  //获取当前文件中的最大SQN 值
	
	BYTE        xdata indexSeq[0x06];
	
	BYTE        xdata seqmax[0x06] = {0x07,0xFF,0xFF,0xFF,0xFF,0xFF};
	
	BYTE        idata SQNWraparoundAllowed;

    BYTE        xdata zero[6];
      
    ecpMem_Set(&zero[0],0x00,sizeof(zero));
	
	SQNWraparoundAllowed = FALSE;
	
	adfAddr =  CheckCurrentDfForADF();
	
	algorithmID = getAlgorithmID(adfAddr);
	
	fileAddr = SelectFile(adfAddr, EF_PRIVATE_AUTHCFG);
	
	if(algorithmID == 1)//milenage
	{
		ReadTFBuf(fileAddr, D_SQNCheck_Offset, &tempBuf[0], 8);
		if((OPMode & 0x04) == 0x04)
		{
			SQNWraparoundAllowed = TRUE;
		}
	}
	else//TUAK
	{
		ReadTFBuf(fileAddr, OFFSET_TUAK_AUTHCFG_IND, &tempBuf[0], 8);
		
		if((TUAK_MODE & 0x04) == 0x04)
		{
			SQNWraparoundAllowed = TRUE;
		}
	}

	

	ind = tempBuf[0];

	ecpMem_Copy(&L[0],&tempBuf[1],sizeof(L));

	deltaC= tempBuf[7];
	
	index = ~(0xFF<<ind);			//根据配置的IND的bit位计算IND的值

	index = (SQN[5] & index) + 1;

	for(i=6; i>=1; i--)					//得到网络的SEQ的值
	{
		seq[i-1] = SQN[i-1] >> ind;

		if(i > 0x01)
		{
			seq[i-1] |= SQN[i-2]<<(8-ind);
		}
	}
	

	fileAddr = SelectFile(adfAddr, EF_SQNCheck);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
	
	ecpMem_Set(sqnMax, 0x00, sizeof(sqnMax));
	
	for(i=1;  i<= 32; i++)  //记录当前文件中最大的那条记录的SQN
	{
		ReadLFBuf(fileAddr, i, 0x00, &tmp[0], 6);	

		if(memcmp_p(&tmp[0], &sqnMax[0], 0x06) > EQUAL) 
		{
			ecpMem_Copy(sqnMax, tmp, 0x06);
		}
	}

	for(i=6; i>=1; i--)					//get seqms
	{
		seqms[i-1] = sqnMax[i-1] >> ind;

		if(i > 0x01)
		{
			seqms[i-1] |= sqnMax[i-2]<<(8-ind);
		}
	}
	
	//In case SQN wrap around is allowed it means that SQN verification will be disabled if the respective SEQ value has reached the maximum value 07FFFFFFFFFF
	if(SQNWraparoundAllowed)
	{
		if((ecpMem_Compare(seqms,seqmax,6) == 0) || (ecpMem_Compare(sqnMax,seqmax,6) == 0))
		{
			return TRUE;
		}
	}	

	if(memcmp_p(&seq[0], &seqms[0], 0x06) > EQUAL)
	{
		if(deltaC != 0xFF)
		{
			StringSub(&seq[0], &seqms[0], &tmp[0], 0x06);

			ecpMem_Set(&delta[0], 0x00, sizeof(delta));

			i = deltaC >> 3;

			delta[5-i] = 0x01 << (deltaC & 0x07);				//根据卡内配置的数值n，计算delta=2^n

			if(memcmp_p(&tmp[0], &delta[0], 0x06) > EQUAL)  	// compare with delta
			{                  
				ecpMem_Copy(&SQN[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

				return D_F1Start_Mode;
					
			}
		}		
	}
	else		  										//添加对"L"的判断				 
	{	
        if (memcmp_p(&L[0], &zero[0], 0x06) != EQUAL)	//当L等于0时不对L进行判断  
		{	
			StringSub(&seqms[0], &seq[0], &tmp[0], 0x06);
	
			if(memcmp_p(&tmp[0], &L[0], 0x06) >= EQUAL)  	  	//SQN - SQNms < L
			{
				ecpMem_Copy(&SQN[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

				return D_F1Start_Mode;
				
			}
		}
	}

	ReadLFBuf(fileAddr, index, 0x00, &delta[0], 0x06);

	for(i=6; i>=1; i--)					//当前索引的seq值
	{
		indexSeq[i-1] = delta[i-1] >> ind;

		if(i > 0x01)
		{
			indexSeq[i-1] |= delta[i-2]<<(8-ind);
		}
	}

	
	if(memcmp_p(&seq[0], &indexSeq[0], 0x06) > EQUAL)  //由于文件初始化为全0 ，如果直接比较SQN ，则会导致当下行SEQ为0，但是SQN不为0时，该比较成立，其实不应该成立
	{
		WriteLFBuf(fileAddr, index, 0x00, &SQN[0], 0x06);        //更新当前的Index的SQN内容

		return TRUE;
	}

	ecpMem_Copy(&SQN[0], sqnMax, 0x06);//替换当前数据的SQN为卡内的最大的SQNms

	return D_F1Start_Mode;	
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 F2toF5Function
  
  Data   content:    input[16]:	标准规定的f2~f5函数输入的数据
  
  					 output：	函数f2~f5的输出结果
					 
					 mode:		当前调用的模式，用于区分函数f2~f5时候不同的输出内容                                                      
                                                                                                                                                
  Description:       3G鉴权中的f2~f5函数的实现，根据存在RecData中的鉴权常数R和C输出不同的结果

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

void F2toF5Function(BYTE input[16], BYTE output[16], BYTE mode)
{
	BYTE idata i;

	Rotata(input, output, R[mode]);

	for (i=0; i<16; i++)
	{
		output[i] ^= C[16*mode + i];
	}
	
//	AESEncrypt(output, output);
	AESEncryptForUICC(output, output,roundKeys);
	
	for(i=0; i<16; i++)
	{
		output[i] ^= OPc[i];
	}
	
	return;	
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 F1Function
  
  Data   content:    input[16]:	标准规定的f1函数输入的数据
  
  					 output：	函数f1的输出结果(xMac)
					 
  Description:       3G鉴权中的f1函数的实现,

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

void F1Function(BYTE input[16], BYTE output[16])
{
	BYTE xdata sqnAMFsqnAMF[16];
	BYTE idata i;
	
	for (i=0; i<2; i++)                 //得到SQN||AMF||SQN||AMF
	{
		ecpMem_Copy(&sqnAMFsqnAMF[i*8], &SQN[0], 6);

		ecpMem_Copy(&sqnAMFsqnAMF[i*8+6], &AMF[0], 2);
	}

	for (i=0; i<16; i++)
	{
		sqnAMFsqnAMF[i] = sqnAMFsqnAMF[i] ^ OPc[i];
	}

	Rotata(&sqnAMFsqnAMF[0], &output[0], R[0]);

	for(i=0; i<16; i++)
	{
		output[i] ^= input[i];
	}

	for(i=0; i<16; i++)
	{
		output[i] ^= C[i];
	}

//	AESEncrypt(output, output);
	AESEncryptForUICC(output, output,roundKeys);

	for (i=0; i<16; i++)
	{
		output[i] ^= OPc[i];  // the first 8 byte of output is just XMAC
	}

	return;	
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 MilenageInit
  
  Data   content:    output：	读KI的函数
					 
  Description:       目前ISIM 与USIM的KI用的是同一套 

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/
BYTE  MilenageInit(void)
{
	UWORD idata fileAddr,adfAddr;
	BYTE  idata result;
	BYTE xdata tmp[16];
	
	if(G_CurrentAdfName[G_ChannelNum] == FALSE)//2g
	{
		adfAddr = SelectLFFile(DF_MF,G_DF_ID_USIM);
	}
	else
	{
		adfAddr = CheckCurrentDfForADF();
	}

	fileAddr = SelectFile(adfAddr, EF_KI);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(fileAddr, 0x01, 0x00, &RecData[100], 16);		//读出鉴权的KI

	fileAddr = SelectFile(adfAddr, EF_PRIVATE_AUTHCFG);

	if(fileAddr== FALSE)
	{
		return FALSE;
	}

	ReadTFBuf(fileAddr,0x00, &RecData[116], 111);			//读出鉴权用的常数字段OPMode，OP，R，C等

//	KeySchedule(&RecData[100]);								//利用Ki进行AES Key的初始化
	G_AESKeyByteLen = 16;
	KeyExpansion(&RecData[100],roundKeys);					//利用Ki进行AES Key的初始化

	if((RecData[116]&D_OPc) == D_OPc)						//如果需要计算OPc字段
	{
//		AESEncrypt(&RecData[117], &tmp[0]);
		AESEncryptForUICC(&RecData[117], &tmp[0],roundKeys);
		
		for (result=0; result<16; result++)
		{
			RecData[117+result] ^= tmp[result];
		}
	}
	return TRUE;
}

#if UICC__OPERATOR__CHINATELECOM
void SetGBAAuthMode(BYTE mode)
{
	UWORD xdata fileGBAModeAddr;
	BYTE idata gbaAuthMode;

	fileGBAModeAddr =  SelectTFFile(G_DF_ID_USIM,EF_AUTH_MODE);

	if(fileGBAModeAddr != FALSE)
	{
		ReadTFBuf(fileGBAModeAddr,0,&gbaAuthMode,0x01);

		if(gbaAuthMode != mode)
		{
			SetTFConst(fileGBAModeAddr,0,mode,0x01);//鉴权复位之后，模式还要维持上一次的值，电信备案测试，握奇TC_NAF_PIN1Disable

		}
	}


}
#endif

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Milenage
  
  Data   content:    output：	鉴权函数计算的输出结果
					 
  Description:       3G鉴权中的主控函数的实现,网络端的内容存放在RecData[0]开始的地方

  Coder:                     Tester:
  
  Status:			   

  优化空间 by xuweihua@20210625
  优化鉴权性能（88/C0指令不写文件，其他指令再写）,移动备案性能测试 by yjr20211213
 -------------------------------------------------------------------------------------------------------------------*/

BYTE Milenage(BYTE * output)
{
	BYTE  idata i;
	BYTE  xdata tmp[16], xRes[16], eKresult[16];

	//UWORD xdata fileAddr,fileAddr1;  //支持GBA
	BYTE  xdata IsimSha[20];  //IsimSha 大小为  DIGEST_LENGTH
/*	
	fileAddr = FALSE;

#if UICC__ISIM	
	if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
	{		
		fileAddr = SelectTFFile(G_DF_ID_ISIM, EF_GBABP);
	}
	else
#endif		
	{
		fileAddr = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);	
	}
*/	
	if(MilenageInit() == FALSE)
	{
		return FALSE;
	}
	for(i=0; i<16; i++)
	{
		tmp[i] = RAND[i] ^ OPc[i];					//计算出随机数和OPc的异或结果
	}

//	AESEncrypt(&tmp[0], &tmp[0]);					//AES计算,得到F1到F5的公用输入参数
	AESEncryptForUICC(&tmp[0], &tmp[0],roundKeys);					//AES计算,得到F1到F5的公用输入参数

	ecpMem_Copy(&eKresult[0], &tmp[0], 16);				//eKresult保存了F1的输入参数

	for(i=0; i<16; i++)								//经过AES计算的结果和OPc进行异或
	{
		tmp[i]  ^= OPc[i];					//tmp内保存了F2到F5的输入参数
	}

	F2toF5Function(tmp, xRes, D_F2_Mode);			//得到xRES和AK的内容前8个字节是xRes的值

	if((OPMode&D_F5_Mode) == D_F5_Mode)				//需要还原SQN的内容
	{
		for(i=0; i<6; i++)
		{
			SQN[i] = xRes[i] ^ SQN[i];			
		}
	}
#if UICC__AUTH__EPS
	for(i=0; i<6; i++)
	{
		SQNAK[i] = SQN[i] ^ xRes[i];
	}	
#endif

	F1Function(eKresult, output);						//计算xMac
	
	if (P2 == 0x84)
	{
		F2toF5Function(tmp, xRes, D_F4_Mode);			    //得到xRES和IK的内容
	
		SHA1Final(IsimSha, &xRes[0], 0x10, 0, 0);		    //sha-1哈希算法

		for (i=0; i < 8; i++)							    //计算GBA MAC* = MAC xor Trunc(SHA-1(IK))
		{
			output[i] = IsimSha[i] ^ output[i];			
		}	
	}	
	
	if(ecpMem_Compare(&output[0], &MAC[0], 8) != EQUAL)
	{
		return FALSE;
	}

	i = CheckSQN();

	if(i == FALSE)
	{
		return FALSE;
	}

	if(i == D_F1Start_Mode)
	{
		AMF[0] = 0x00;

		AMF[1] = 0x00;                                   //计算 MAC-S时，AMF设置为全0，参考TS133102                     

		F1Function(eKresult, output);					//计算xMac
		
		F2toF5Function(tmp, xRes, D_F5Start_Mode);		//得到F5*计算AK*的内容

		if((OPMode&D_F5_Mode) == D_F5_Mode)				//需要还原SQN的内容
		{
			for(i=0; i<6; i++)
			{
				SQN[i] = xRes[i] ^ SQN[i];			
			}
		}

		output[0] = 0xDC;

		output[1] = 0x0E;

		ecpMem_Copy(&output[2], SQN, 6);

		return 16;
	}
	else
	{
		output[0] = 0xDB;
	
		output[1] = RESLGTH;			  				//RES
		
		F2toF5Function(tmp, xRes, D_F2_Mode);			 //得到xRES和AK的内容前8个字节是xRes的值
	
		ecpMem_Copy(&output[2], &xRes[8], RESLGTH);
	
		F2toF5Function(tmp, xRes, D_F3_Mode);			//得到xRES和CK的内容
	
		output[2+RESLGTH] = 16;
		
		ecpMem_Copy(&output[3+RESLGTH], xRes, 16);
	
		F2toF5Function(tmp, xRes, D_F4_Mode);			//得到xRES和IK的内容
	
		output[19+RESLGTH] = 16;
		
		ecpMem_Copy(&output[20+RESLGTH], xRes, 16);
		
		//fileAddr1 = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_AUTH_MODE);

		//if ((fileAddr == FALSE) || (fileAddr1 == FALSE))
		//{
		//	ConditionSatisfied();									//Send 69 85
		//	return FALSE;
		//}

		//if (fileAddr != FALSE)
		//{
		//	WriteTFBuf(fileAddr, 0x00, &RecData[0], RecData[0]+1);			//更新6FD5中的RAND值
		//}
        ecpMem_Copy(G_Auth_Rand,&RecData[0],RecData[0]+1);		

		if (P2 == 0x84)
		{
#if UICC__OPERATOR__CHINATELECOM
			SetGBAAuthMode(AUTH_MODE_GBA_BOOT);//by yjr20211213鉴权算法优化性能时，INS为88和C0时不写文件，电信测试时，该值是从文件中读的，此时两条88指令之间没有插入其他指令，导致出错 test：42.USIM_GBA_N_P006
#else			
			G_GBA_AUTH_Status = 0x01;									//GBA Bootstrapping Mode鉴权成功
#endif

			G_Auth_Ks[0] = AUTH_MODE_GBA_BOOT;
		}
		else
		{
			G_Auth_Ks[0] = AUTH_MODE_GBA_ME;
		}
		
		ecpMem_Copy(&G_Auth_Ks[1],&tempBuf[11],0x10);
		ecpMem_Copy(&G_Auth_Ks[0x11],&tempBuf[28],0x10);

		//fileAddr1 = SelectLFFile(G_CurrentAdfName[G_ChannelNum],EF_AUTH_MODE);

		//if (fileAddr1 != FALSE)
		//{
		//	WriteTFBuf(fileAddr1,0x00, &authKs[0],0x21);//更新ks的值
		//}

		return 36+RESLGTH;
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Milenage2G
  
  Data   content:    output：	鉴权函数计算的输出结果
					 
  Description:       3G网络中进行2G鉴权中的主控函数的实现,网络端的内容存放在RecData[0]开始的地方

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

BYTE Milenage2G(BYTE * output)
{
	BYTE xdata tmp[16], sRes[4];
	BYTE idata i;	
	
		
	if(MilenageInit() == FALSE)
	{
		return FALSE;
	}
	
	for(i=0; i<16; i++)
	{
		tmp[i] = RAND[i] ^ OPc[i];
	}	

//	AESEncrypt(tmp, tmp);							//得到输入参数Ek的内容
	AESEncryptForUICC(tmp, tmp,roundKeys);							//得到输入参数Ek的内容

	for(i=0; i<16; i++)								//经过AES计算的结果和OPc进行异或
	{
		tmp[i] = tmp[i] ^ OPc[i];					//tmp内保存了F2到F5的输入参数
	}

	F2toF5Function(tmp, output, D_F2_Mode);			//得到sRes的值

	ecpMem_Move(&output[0], &output[8], 8);

	ecpMem_Set(&output[8], 0x00, 8);

	for (i=0; i<4; i++)
	{
		sRes[i] = output[i] ^ output[i + 4] ^ output[i + 8] ^ output[i + 12];
	}

	F2toF5Function(tmp, &output[0], D_F3_Mode);		//得到CK的值

	F2toF5Function(tmp, &output[16], D_F4_Mode);	//得到IK的值

	for (i=0; i<8; i++)
	{
		tmp[i] = output[i] ^ output[i+8] ^ output[16+i] ^ output[16+i+8];
	}

	output[0] = 0x04;

	ecpMem_Copy(&output[1], sRes, 4);

	output[5] = 0x08;

	ecpMem_Copy(&output[6], tmp, 8);

	return 14;
}

