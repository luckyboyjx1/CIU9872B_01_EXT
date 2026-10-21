/**
 * @File Name	des.c
 *
 * @brief 		DES加密和解密函数
 *
 * @author 		WG
 *
 * @see 		FIPS 46-3, Data Encryption Standard (DES)
 * 			http://people.csail.mit.edu/rivest/Destest.txt

 * @version 	1.9/CVS_1.4	WG,20080516:修改语法，简化语句，测试通过,
 * 				CALMSHINEPLUS v1.56d: 1730 字节;
 *
 * @version 	1.8/CVS_1.3	WG,20080516:修改'code static'为 'static code';删除函数声明中的'code';
 * 				删除'ptr'定义时的'code'关键字;添加'(unsigned char *)'
 *
 * @version 	1.7		速度：v1.7: 64s
 				同比：v1.6: 105s v1.5: 144s olddes.c：26s(执行20条批处理)
 				编译大小：SS96F1: 1214+584=1798
 *
 *
 * @version 	1.6		速度：v1.6:1.570338ms
 				同比：v1.5:2.254479ms olddes.c：1.504942ms
 * 				编译大小：SS96F1: 1238+584=1822
 *
 * @version 	1.5
 * 				编译大小：SS96F1: 1014+696=1710
 *
 * @history 	v1.5 2007-11-19 14:20
 * 				删除存储16支子密钥的全局变量，改用局部变量
 * 				4.943252 ms
 *
 * @history 	v1.4 2007-11-19
 * 				修改了 Des_Key(): 将加密和解密的子密钥产生过程拆分开
 * 				4.901332 ms

 * @history 	v1.3 2007-11-18
 * 				修改了 S_func(): 将 S_Box 从 512 字节压缩到 256 字节
 * 				修改了 Des_Run(): 将加密和解密流程合并
 * 				5.092 ms
 *
 * @history 	v1.2 2007-11-18
 * 				修改了 valBitArray(): 将 '除' 运算替换为 '移位'；将 '取余' 替换为 '按位与'
 * 				在 TURBO C 2.01 上运行 0xFFFF 次 rivest_des_test 通过,
 * 				耗时 326.81318 秒，平均每次约  4.986 ms

 * @history 	v1.1 2007-11-18
 * 				在 TURBO C 2.01 上运行 0xFFFF 次 rivest_des_test 通过,
 * 				耗时 741.18131 秒，平均每次约 11.309 ms
 *
 * @history 	v1.1 2007-11-18  测试通过
 */
#include "string.h"
#include "typedef.h"
#include "Des.h"

//#include "VmAccessEeprom.h"
#define ENCRYPT_0 0
#define DECRYPT_1 1

#ifndef HARDWARE_DES 
/* initial permutation IP */
static code unsigned char IP_Table[64] = {
	58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6,
	64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17,  9, 1,
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5,
	63, 55, 47, 39, 31, 23, 15, 7
};
/* final permutation IP^-1  */
static code unsigned char IPR_Table[64] = {
	40, 8, 48, 16, 56, 24, 64, 32,
	39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30,
	37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28,
	35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26,
	33, 1, 41,  9, 49, 17, 57, 25
};
/* expansion operation matrix */
static code unsigned char E_Table[48] = {
	32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};
/* 32-bit permutation function P used on the output of the S-boxes  */
static code unsigned char P_Table[32] = {
	16,  7, 20, 21, 29, 12, 28, 17,
	 1, 15, 23, 26,  5, 18, 31, 10,
	 2,  8, 24, 14, 32, 27,  3,  9,
	19, 13, 30,  6, 22, 11,  4, 25
};

/* permuted choice table (key)  */
static code unsigned char PC1_Table[56] = {
	57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};

/* permuted choice key (table)  */
static code unsigned char PC2_Table[48] = {
	14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

/* The (in)famous S-boxes  */
static code unsigned char S_Box[8][4][8] = {
	0xE4, 0xD1, 0x2F, 0xB8, 0x3A, 0x6C, 0x59, 0x07,
	0x0F, 0x74, 0xE2, 0xD1, 0xA6, 0xCB, 0x95, 0x38,
	0x41, 0xE8, 0xD6, 0x2B, 0xFC, 0x97, 0x3A, 0x50,
	0xFC, 0x82, 0x49, 0x17, 0x5B, 0x3E, 0xA0, 0x6D,

	0xF1, 0x8E, 0x6B, 0x34, 0x97, 0x2D, 0xC0, 0x5A,
	0x3D, 0x47, 0xF2, 0x8E, 0xC0, 0x1A, 0x69, 0xB5,
	0x0E, 0x7B, 0xA4, 0xD1, 0x58, 0xC6, 0x93, 0x2F,
	0xD8, 0xA1, 0x3F, 0x42, 0xB6, 0x7C, 0x05, 0xE9,

	0xA0, 0x9E, 0x63, 0xF5, 0x1D, 0xC7, 0xB4, 0x28,
	0xD7, 0x09, 0x34, 0x6A, 0x28, 0x5E, 0xCB, 0xF1,
	0xD6, 0x49, 0x8F, 0x30, 0xB1, 0x2C, 0x5A, 0xE7,
	0x1A, 0xD0, 0x69, 0x87, 0x4F, 0xE3, 0xB5, 0x2C,

	0x7D, 0xE3, 0x06, 0x9A, 0x12, 0x85, 0xBC, 0x4F,
	0xD8, 0xB5, 0x6F, 0x03, 0x47, 0x2C, 0x1A, 0xE9,
	0xA6, 0x90, 0xCB, 0x7D, 0xF1, 0x3E, 0x52, 0x84,
	0x3F, 0x06, 0xA1, 0xD8, 0x94, 0x5B, 0xC7, 0x2E,

	0x2C, 0x41, 0x7A, 0xB6, 0x85, 0x3F, 0xD0, 0xE9,
	0xEB, 0x2C, 0x47, 0xD1, 0x50, 0xFA, 0x39, 0x86,
	0x42, 0x1B, 0xAD, 0x78, 0xF9, 0xC5, 0x63, 0x0E,
	0xB8, 0xC7, 0x1E, 0x2D, 0x6F, 0x09, 0xA4, 0x53,

	0xC1, 0xAF, 0x92, 0x68, 0x0D, 0x34, 0xE7, 0x5B,
	0xAF, 0x42, 0x7C, 0x95, 0x61, 0xDE, 0x0B, 0x38,
	0x9E, 0xF5, 0x28, 0xC3, 0x70, 0x4A, 0x1D, 0xB6,
	0x43, 0x2C, 0x95, 0xFA, 0xBE, 0x17, 0x60, 0x8D,

	0x4B, 0x2E, 0xF0, 0x8D, 0x3C, 0x97, 0x5A, 0x61,
	0xD0, 0xB7, 0x49, 0x1A, 0xE3, 0x5C, 0x2F, 0x86,
	0x14, 0xBD, 0xC3, 0x7E, 0xAF, 0x68, 0x05, 0x92,
	0x6B, 0xD8, 0x14, 0xA7, 0x95, 0x0F, 0xE2, 0x3C,

	0xD2, 0x84, 0x6F, 0xB1, 0xA9, 0x3E, 0x50, 0xC7,
	0x1F, 0xD8, 0xA3, 0x74, 0xC5, 0x6B, 0x0E, 0x92,
	0x7B, 0x41, 0x9C, 0xE2, 0x06, 0xAD, 0xF3, 0x58,
	0x21, 0xE7, 0x4A, 0x8D, 0xFC, 0x90, 0x35, 0x6B,
};

/* number left rotations of pc1  */
static code unsigned char LOOP_Table[16] = {
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

/**
 * @brief 	从数组中获取指定的比特位
 *
 * @param 	a：unsigned char a[N], N为数组大小
 *
 * @param 	index：序号，范围[1, 8*N], N为数组大小
 *
 * @Returns     TRUE/FALSE
 *
 * @author 	WG

 * @history 	optimizted  by WG, @071118 20:06
 * @history 	tested  by WG, @071101 15:51
 * @history 	created by WG, @071030 11:03
 * @history 	modified by TL, @080107 10:35
 */

unsigned char Bit[8];

unsigned char valBitArray(unsigned char a[], unsigned char index)
{
	index --;

	return (a[index>>3] >>(7-(index&0x07)))&0x01;
	/*
	 * return ValueBit(a[index/8], 7-(index%8));
	 *
	 * 将 '除' 运算替换为 '移位'；将 '取余' 替换为 '按位与'
	 * 测试时间由 750 秒减少至 347 秒
	 *
	 */
}

/**
 * @brief 		按照置换表，将输入数组的比特位置换到输出数组（输入和输出可以是同一个数组）
 *
 * @param 		Out : unsigned char Out[len/8]
 * @param 		In  : unsigned char Iut[len/8]
 * @param 		Table:unsigned char Table[len]
 * @param 		len:  unsigned char,范围[8, 64], 应为8的整数倍
 *
 * @author 		WG
 * @history 	tested  by WG, @071101 20:16
 * @history 	created by WG, @071030 14:10
 * @history 	modified by TL, @080107 11:12
 */
void Transform(unsigned char Out[], unsigned char In[], unsigned char Table[], unsigned char len)
{
	unsigned char i, j, index;
	unsigned char *ptr;

	ptr = Table;

	for (i=0;i<(len>>0x03);i++)
	{
		Bit[i] = 0x00;

		for (j=0;j<8;j++)
		{
			Bit[i] = Bit[i]<<1;

			index = *ptr;

			ptr ++;

			index --;

			Bit[i] |= (In[index>>3] >>(7-(index&0x07)))&0x01;
		}
	}

	memcpy(Out, Bit, (len>>3));
}

/**
 * @brief 		将数组前28比特循环左移1位；将数组后28比特循环左移1位
 *
 * @param 		In : unsigned char In[7]，即56比特
 *
 * @author 		TL
 *
 *@history 		created by TL, @080102 15:20
 *@history 		tested  by TL, @080102 16:11
 */
void LoopLeft(unsigned char In[])
{
	unsigned char i, *ptr;

	ptr = In;

	for (i=0x00;i<0x07;i++)
	{
		Bit[i] = ((*ptr) >>7)|0x00;

		(*ptr) <<= 0x01;

		ptr ++;
	}

	Bit[7] = (In[3] >>4)&0x01;

	if (Bit[7] == 0x01)
	{
		In[3] &= ((Bit[0]<<0x04)|0xEF);
	}
	else
	{
		In[3] |= ((Bit[0]<<0x04)&0x10);
	}

	ptr = In;

	for (i=0x01;i<0x08;i++)
	{
		(*ptr) |= Bit[i];

		ptr ++;
	}
}

/**
 * @brief 		将数组前28比特循环左移1或2位；将数组后28比特循环左移1或2位
 *
 * @param 		In : unsigned char In[7]，即56比特
 *
 * @author 		WG
 *
 * @modify		TL
 * @history 		tested  by WG, @071101 21:06
 * @history 		created by WG, @071030 16:48
 * @history 		modified by TL, @080107 09:48
 */
void RotateL(unsigned char In[], unsigned char loop)
{
	while (loop)
	{
		LoopLeft(In);
		loop --;
	}
}

/**
 * @brief 		将数组前28比特循环右移1位；将数组后28比特循环右移1位
 *
 * @param 		In : unsigned char In[7]，即56比特
 *  *
 * @author 		TL
 *
 * @history 		created by TL, @080102 16:30
 * @history 		tested  by TL, @080102 16:51
 */
void LoopRight(unsigned char In[])
{
	unsigned char i, *ptr;

	ptr = In;

	for (i=0x00;i<0x07;i++)
	{
		Bit[i+1] = (*ptr) &0x01;
		(*ptr) >>= 0x01;
		ptr ++;
	}

	Bit[0] = (In[3]>>3)&0x01;

	if (Bit[0] == 0x01)
	{
		In[3] &= ((Bit[7]<<0x03)|0xF7);
	}
	else
	{
		In[3] |= ((Bit[7]<<0x03)&0x08);
	}

	ptr = In;

	for (i=0x00;i<0x07;i++)
	{
		(*ptr) |= (Bit[i] <<7);
		ptr ++;
	}
}

/**
 * @brief 		将数组前28比特循环右移1或2位；将数组后28比特循环右移1或2位
 *
 * @param 		In : unsigned char In[7]，即56比特
 * @param 		loop:右移位数(1/2)
 *
 * @author 		WG

 * @history 		created by WG, @071119 10:26
 * @history 		modified by TL, @080107 09:50
 */
void RotateR(unsigned char In[], unsigned char loop)
{
	while (loop)
	{
		LoopRight(In);
		loop --;
	}
}
/**
 * @brief 		将两数组进行异或
 *
 * @param 		DesA: unsigned char DesA[len/8]
 * @param 		SrcB: unsigned char SrcA[len/8]
 * @param 		len: unsigned char len
 *
 * @author 		WG
 *
 * @history 		tested  by WG, @071117 18:35
 * @history 		created by WG, @071101 11:39
 * @history 		midified by TL, @080107 10:37
 */
void Xor(unsigned char DesA[], unsigned char SrcB[], unsigned char len)
{
	unsigned char *ptrA, *ptrB;

	ptrA = DesA;
    
	ptrB = SrcB;

	while(len)
	{
		(*ptrA) ^= (*ptrB);
		ptrB ++;
		ptrA ++;
		len  --;
	}
}

/**
 * @brief 		S函数，从48比特中得到32比特
 *
 * @param 		Out: unsigned char Out[4]
 * @param 		In : unsigned char In[6]
 *
 * @author 		WG
 *
 * @history 		optimizted  by WG, @071118 21:29
 * 			将 S_Box 从 512 字节压缩到 256 字节
 *
 * @history 		optimizted  by WG, @071118 21:00
 * @history 		tested  by WG, @071118 10:43
 * @history 		created by WG, @071101 13:22
 * @history 		modified by TL, @080106 14:47
 */
#define bit1 1
#define bit2 2
#define bit3 3
#define bit4 4
#define bit5 5
#define bit6 6

void S_func(unsigned char Out[4], unsigned char In[6])
{
	unsigned char  i, j;
	unsigned char  row, col;

	/* 将输入的 48 比特分为 8 组，每组 6 比特 */
	/* 每组分别使用对应的 S-Box 进行变换，得出 4 比特 */
	/* 最后得出 4 字节 */

	for(i=0; i<8; i++)
	{
		j = (i<<2)+(i<<1);

		row = (valBitArray(In, (bit1 +j)) <<1) + valBitArray(In, (bit6 +j));

		col = (valBitArray(In, (bit2 +j)) <<3) + (valBitArray(In,(bit3 +j)) <<2) + (valBitArray(In, (bit4 +j)) <<1) + (valBitArray(In, (bit5 +j)));


		j = i>>1;

		Out[j] = Out[j] << 4;

		if ((col &0x01) == 1)
		{
			Out[j] += (S_Box[i][row][col>>1] &0x0F);
		}
		else
		{
			Out[j] += ((S_Box[i][row][col>>1] &0xF0) >>4);
		}
	}
}

/**
 * @brief 		F函数变换: 将32比特的输入与48比特的子密钥进行变换
 *
 * @param 		In: unsigned char In[4]
 * @param 		Ki: unsigned char Ki[6]
 *
 * @author 		WG
 *
 * @history 		created by WG, @071101 13:09
 */
void F_func(unsigned char In[4], unsigned char Ki[6])
{
	unsigned char  MR[6];

	Transform(MR, In, (unsigned char *)E_Table, sizeof(E_Table)); 	/* 将32比特的输入扩展为48比特 */

	Xor(MR, Ki, sizeof(MR)); 			/* 将48比特与 Ki 异或 */

	S_func(In, MR); 				/* S 函数变换 */

	Transform(In, In, (unsigned char *)P_Table, sizeof(P_Table)); 	/* 将结果进行 P 变换 */
}

/**
 * @brief 		DES 加密/解密函数
 *
 * @param 		Out: 无
 * @param 		In : 明文(输入), unsigned char ;plain[8], 密文，cipher[8]
 * @param 		Mode: ENCRYPT_0/DECRYPT_1
 *
 * @author 		WG
 *
 * @history 		optimizted by WG, @071118 22:10
 * @history 		created by WG, @071101 11:19
 */
void DES(unsigned char plain[8], unsigned char key[8], unsigned char cipher[8], unsigned char mode)
{
	unsigned char  M[8], Temp[4];
	unsigned char  K[8], SubKey[6];
	unsigned char  *Li;
	unsigned char  *Ri;
	unsigned char  i, *Tmp, *ptr;

	Li = &M[0];
	Ri = &M[4];

	memcpy(M, plain, sizeof(M));

	/* 将明文进行 Initial Permutation 变换 */
	Transform(M, M, (unsigned char *)IP_Table, sizeof(IP_Table));

	Tmp = Temp;

	if (mode == DECRYPT_1)
	{
		memcpy(Tmp, Ri, sizeof(Temp));
		memcpy(Ri, Li,  sizeof(Temp));
		memcpy(Li, Tmp, sizeof(Temp));

		Li = &M[4];
		Ri = &M[0];
	}

	memcpy(K, key, sizeof(K));

	Transform(K, K, (unsigned char *)PC1_Table, sizeof(PC1_Table));

	for(i=0; i<16; i++)
	{
		/*
		 * 备份 original_R
		 * 将 original_R 进行 F函数 变换
		 * 将 R 与 L 异或
		 * 将 original_R 复制到 L
		 */

		memcpy(Tmp, Ri, sizeof(Temp));

		if (mode == ENCRYPT_0)
		{
			RotateL(K, LOOP_Table[i]);
		}

		Transform(SubKey, K, (unsigned char *)PC2_Table, sizeof(PC2_Table));

		if (mode == DECRYPT_1)
		{
			RotateR(K, LOOP_Table[0x0F - i]);
		}

		F_func(Ri, SubKey);

		Xor(Ri, Li, sizeof(M)>>1);

		ptr = Tmp;
		Tmp = Li;
		Li = ptr;
	}

	if (mode == ENCRYPT_0)
	{
		memcpy(Tmp, Ri, sizeof(Temp));
		memcpy(Ri, Li,  sizeof(Temp));
		memcpy(Li, Tmp, sizeof(Temp));
	}

	/* 将密文进行 Inverse Initial Permutation 变换 */
	Transform(M, M, (unsigned char *)IPR_Table, sizeof(IPR_Table));

	memcpy(cipher, M, sizeof(M));
}



void SoftDESCalc(BYTE * pbSrcStr,BYTE keyContext[16],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength)
{
	BYTE xdata dataPointer[8];
	BYTE xdata resultData[8];
	BYTE xdata TgtTmpData[8];
	WORD idata num;
	BYTE idata i,mode;

	if(((lgth & 0x07) != 0x00) || (lgth == 0x00))		// 长度不是 8 的整数倍
	{
		return;
	}
	if((keyLength&0xE7)!=0)	                            //key长度 8,16,24
	{
		return;
	}

	if(bMod == CBC_Init_Encrypt)
	{
		memcpy(&dataPointer[0], pbTgtStr, sizeof(dataPointer));

		bMod = bMod & 0x8F;
	}
	else
	{
		memset(dataPointer, 0x00, sizeof(dataPointer));
	}

	if(bMod == CBC_Init_Decrypt)
	{
		memcpy(&resultData[0], pbTgtStr, sizeof(resultData));

		bMod = bMod & 0x8F;
	}
	else
	{
		 memset(resultData, 0x00, sizeof(resultData));
	}

	mode = ( ((bMod&0x0F) == 0x0E) ? 0x01 : 0x00);	        // 0x01	-> DEC
															// 0x00 -> ENC
	for(num = 0x00; num < lgth; num += 0x08)
	{
		memcpy(&TgtTmpData[0], (pbSrcStr+num), 0x08);

		for(i = 0x00; i < 0x08; i++)
		{
			if(bMod == CBC_Encrypt)
			{
				dataPointer[i] ^= TgtTmpData[i];
			}
			else
			{
				dataPointer[i] = TgtTmpData[i];
			}
		}

		if(keyLength == 0x08)	                                                    //singleDes
		{
			DES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);
		}
		else if((keyLength == 0x10)||(keyLength == 0x18))                           //16或24	为3des
		{
			if(mode == 0x01)                                                        //解密
			{
				if(keyLength == 24)
				{
					DES(&dataPointer[0], &keyContext[0x10], &dataPointer[0], mode);  //3 key解密步骤为：k3解密，k2加密，k1解密
				}
				else
				{
					DES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);  //2 key解密步骤为：k1解密，k2加密，k1解密
		        }
						
				DES(&dataPointer[0], &keyContext[0x08], &dataPointer[0], ((mode == 0x01) ? 0x00: 0x01));
					
				DES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);						
			}
			else                                                                        //加密
			{	
				DES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);
		        		
				DES(&dataPointer[0], &keyContext[0x08], &dataPointer[0], ((mode == 0x01) ? 0x00: 0x01));
		
				if(keyLength == 24)
				{
					DES(&dataPointer[0], &keyContext[0x10], &dataPointer[0], mode);  //3 key加密步骤为：k1加密，k2解密，k3加密
				}
				else
				{
					DES(&dataPointer[0], &keyContext[0x00], &dataPointer[0], mode);  //2 key加密步骤为：k1加密，k2解密，k1加密
				}
			}
		}
		else
		{
			return;
		}

		for(i = 0x00; i < 0x08; i++)
		{				
			if(bMod == CBC_Decrypt)
			{
				TgtTmpData[i] = dataPointer[i] ^ resultData[i];
			}
			else
			{
				TgtTmpData[i]	= dataPointer[i];
			}		
		}

		memcpy(&resultData[0], (pbSrcStr+num), 0x08);

		memcpy((pbTgtStr+num),(BYTE*)TgtTmpData,0x08);
	}
}
/*-------------------------------------------------------------------------------------------------------//

  Function name:	SingleDES

 Input parameter:
 			        pbSrcStr:源数据(若有初始值，则可用参数pbTgtStr传递，用参数bMod来区分)

				keyContext  :主密钥

				pbTgtStr:结果数据(若有初始值，则可用此参数传递，用参数bMod来区分)

				bMod: 	ECB_Encrypt     : 	ECB	加密
						ECB_Decrypt    :	ECB	解密
						CBC_Encrypt     :    无初始值CCB加密
						CBC_Decrypt  	:	无初始值CCB解密
						CBC_Init_Encrypt	:有初始值时，初始值已以参数方式传递的CBC加密
						CBC_Init_Decrypt	:有初始值时，初始值已以参数方式传递	的CBC解密

				lgth:源数据长度

  Output parameter:     void

  Description:          单DES算法。
  				注意:若是CBC_Encrypt模式而且有初始值，则可在函数调用之前用初始值与源数据的第一个BLOCK
  				作一次异或后再传递给pbSrcStr;若是CBC_Decrypt模式，则可在做完3DES算法后用初始值与pbTgtStr的第
  				一个BLOCK作一次异或，得到的数据才是结果数据

  Example:		data: 			7897875668685365
				des-key  : 		7890847689465788
				des-ECB加密: 	6C39BBF88D955E3D
				des-ECB解密:	3279102C0E5C7BE1

				data : 			789787566868536528276634596754467865475640574334
				des-key  : 		7890847689465788
				des-CBC加密: 	6C39BBF88D955E3D8280767E287A159822FEEE23CF4E0AC7
				des-CBC解密:	3279102C0E5C7BE16C977287D5649322281F75FD79869ACD


  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void SingleDES(BYTE * pbSrcStr, BYTE keyContext[8],BYTE * pbTgtStr, BYTE bMod, WORD lgth)
{
	SoftDESCalc(pbSrcStr, keyContext, pbTgtStr, bMod, lgth, 8);
}

/*-------------------------------------------------------------------------------------------------------//

  Function name:	TripleDES

  Input parameter:
 			        pbSrcStr:源数据(若有初始值，则可用参数pbTgtStr传递，用参数bMod来区分)

				keyContext  :主密钥

				pbTgtStr:结果数据(若有初始值，则可用此参数传递，用参数bMod来区分)

				bMod: 	ECB_Encrypt     : 	ECB	加密
						ECB_Decrypt    :	ECB	解密
						CBC_Encrypt     :    无初始值CCB加密
						CBC_Decrypt  	:	无初始值CCB解密
						CBC_Init_Encrypt	:有初始值时，初始值已以参数方式传递的CBC加密
						CBC_Init_Decrypt	:有初始值时，初始值已以参数方式传递	的CBC解密

				lgth:源数据长度

  Output parameter:     void

  Description:          3DES算法。
  				注意:若是CBC_Encrypt模式而且有初始值，则可在函数调用之前用初始值与源数据的第一个BLOCK
  				作一次异或后再传递给pbSrcStr;若是CBC_Decrypt模式，则可在做完3DES算法后用初始值与pbTgtStr的第
  				一个BLOCK作一次异或，得到的数据才是结果数据

  Example:		data: 			789787566868536528276634596754467865475640574334
				3des-key  : 	26777978045645893542467496856409
				3DES-ECB加密: 	F8AFFBC2AE4EB17734B97F68D2CB9A2C1163959669DC44BA
				3DES-ECB解密: 	81DDE3AAE0ADDD59BA276AFBACB0AFD79F2CAEA219653975
				3DES-CBC加密：	F8AFFBC2AE4EB177CB5B0E3EAF841CDF532F5745172E0787
				3DES-CBC解密：	81DDE3AAE0ADDD59C2B0EDADC4D8FCB2B70BC89640026D33

  Status:		OK

 -------------------------------------------------------------------------------------------------------------------*/
void TripleDES(BYTE * pbSrcStr,BYTE keyContext[24],BYTE * pbTgtStr, BYTE bMod, WORD lgth, BYTE keyLength)
{ 
	SoftDESCalc(pbSrcStr, keyContext, pbTgtStr, bMod, lgth,keyLength);

}
#endif

