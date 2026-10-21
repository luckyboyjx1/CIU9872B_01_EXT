/************************************************************************************************
Copyright (C), 1988-2018, Eastcompeace. Co., Ltd.

File name   : AES.c

Author:     : zhhj  

Version:    : V1.0

Description :  该文件存放AES的底层功能实现
************************************************************************************************/ 
#include "AES.h"
#include "uicc_main.h"
//#include "Transaction.h"

#include "Interface_LTEFromBoot.h"
#include "typedef.h"
#include <string.h>
//#include "VMAccessEeprom.h"
//#include "Interface_PlatformFromLTE.h"
//#include "uicc_main.h"

#ifdef AES

BYTE xdata AES_Nk;       //使用AES时候 必须先给AES_Nr/AES_Nk 赋值
BYTE xdata AES_Nr;
/*
keyLen = 16;  AES_Nk = 4; AES_Nr = 10;     默认密钥长度为16，鉴权时候用到的密钥长度也是16字节  目前只有OTA等用到的密钥长度可能为24  32
keyLen = 24;  AES_Nk = 6; AES_Nr = 12;      密钥扩展前 key_expansion  对着两个全局变量赋值
keyLen = 32;  AES_Nk = 8; AES_Nr = 14;
*/
/*
*@escription:根据密钥长度对AES的全局变量赋值
*
*keyLen = 16;  AES_Nk = 4; AES_Nr = 10;     默认密钥长度为16，鉴权时候用到的密钥长度也是16字节  目前只有OTA等用到的密钥长度可能为24  32
*keyLen = 24;  AES_Nk = 6; AES_Nr = 12;      密钥扩展前 key_expansion  对着两个全局变量赋值
*keyLen = 32;  AES_Nk = 8; AES_Nr = 14;
*
*@return    :TRUE/FALSE
*
*@author    :
*
*@history   :
*
*/

#if  !JAVA_CARD
/*
deleted by yangjian@20221205: 该函数未使用
BYTE CheckAesKeyLength(BYTE keyLength)
{
	switch (keyLength)
	{
		case 16:
			AES_Nk = 4; 
			AES_Nr = 10;			
			break;
		case 24:	  
			AES_Nk = 6; 
			AES_Nr = 12;        
			break;
		case 32:
			AES_Nk = 8; 
		    AES_Nr = 14;
			break;
		default:
			
			return FALSE;
	}

   	return TRUE;                                                     		//REQUEST COMPAREING THE DATA   
}
*/
#endif
/*
 * S-box transformation table
 FIPS 197 5.1.1
 */
 BYTE code sbox[256] = {
	// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, // 0
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, // 1
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, // 2
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, // 3
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, // 4
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, // 5
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, // 6
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, // 7
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, // 8
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, // 9
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, // a
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, // b
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, // c
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, // d
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, // e
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16  // f
};

/*
*@description: CoefAdd，数组异或
*有限域中两个元素的加法定义为其多项式表示的相应系数的“加法”。这里加法是异或
*运算(记为⊕ )，就是模2加，1 ⊕ 1 ＝0 ，1 ⊕ 0 ＝1 ，0 ⊕ 0 ＝0 。因此，多项式减法与多项式加
*法的规则相同。
*
*@function   : CoefAdd
*
*@See        : FIPS 197 4.1
*
*@param      : BYTE a[]
*              BYTE b[]
*              BYTE d[]
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/
void CoefAdd(BYTE a[], BYTE b[], BYTE d[]) 
{
	BYTE idata i;
	
	for(i = 0 ;i < 4 ;i ++)
	{
		d[i] = a[i] ^ b[i];
	}
}

	/*
*@description: GMult，多项式乘法。将a的值乘以b，多项式乘。
*1.在GF(2^8)上执行多项乘法模运算需要用到多项式乘法和除法运算，时间开销大，难以用软件硬件实现，需要改进
* 算法以便计算机实现。
*2.有详细的推导过程（见《AES算法中的多项式模运算及其性能分析》2多项式模运算算法的改进），x乘以多项式g(x)
* 的运算可以通过"多项式八位的二进制系数左移一位后按位异或 (0001 1011)完成"，即 
*  x乘以g(x)= 
*   b7=0时 b6 b5 b4 b3 b2 b1 0
*   b7=1时 b6 b5 b4 b3 b2 b1 0 异或 00011011
*3.有详细的推导过程（见《AES算法中的多项式模运算及其性能分析》2多项式模运算算法的改进）在有限域 GF(2^8)上，
*多项式乘法运算可以直接使用两个八位的二进制整数相乘，通过多个中间结果作模2加(就是异或)来实现。
*！！！通过下面简洁的代码实现了复杂的多项式乘法运算。
*注意：a和b都是传值调用，返回结果保存在p中
*
*@function   : GMult
*
*@See        :FIPS 197 4.2.1
*
*@param      : BYTE a
*              BYTE b
*
*@return     : BYTE
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/
BYTE GMult(BYTE a, BYTE b) 
{
	BYTE idata p, i, hbs;
	
	p = 0;
	
	i = 0;
	
	hbs = 0;

	for (i = 0; i < 8; i++) 
	{
		if (b & 1)   //b0==1 b的各位顺序b7 b6 b5 b4 b3 b2 b1 b0
		{
			p ^= a;  //结果异或
		}

		hbs = a & 0x80; //hbs b7
	
		a <<= 1; //a向左移动1位 
	
		if (hbs) //b7 == 1
		{
			a ^= 0x1b; // 0000 0001 0001 1011	
		}
		
		b >>= 1;  //b向右移动1位
	}

	return (BYTE)p;
}

/*
*@description: SubBytes，字节替换。把State矩阵中的每个字节替换成由Sbox决定的新字节
*
*@function   : SubBytes
*
*@See        : FIPS 197 5.1.1
*
*@param      : BYTE *state   State矩阵
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/

void SubBytes(BYTE *state) 
{
	BYTE idata  i, j;

	for (i = 0; i < 4; i++) 
	{
		
		for (j = 0; j < AES_Nb; j++) 
		{
			state[ AES_Nb * i + j ] = sbox[ state[ AES_Nb * i + j ] ]; //把State矩阵中的每个字节替换成由Sbox决定的新字节
		}
	}
}

/*
*@description: ShiftRows，行移位。ShiftRows是一个置换操作，它将State矩阵中的字节向左旋转
* State的第0行不旋转移位，
* State的第1行被向左旋转1个字节，
* State的第2行被向左旋转2个字节，
* State的第3行被向左旋转3个字节
*
*@function   : ShiftRows
*
*@See        : FIPS 197 5.1.2
*
*@param      : BYTE *state   State矩阵
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/

void ShiftRows(BYTE *state) 
{
	BYTE idata i, k, s, tmp;

	for (i = 1; i < 4; i++) 
	{
		s = 0;
		
		while (s < i) 
		{
			tmp = state[ AES_Nb * i];
			
			for (k = 1; k < AES_Nb; k++) 
			{
				state[ AES_Nb * i + k - 1 ] = state[ AES_Nb * i + k ];
			}

			state[ AES_Nb * i + AES_Nb - 1 ] = tmp;
			
			s++;
		}
	}
}

/*
*@description: CoefMult 矩阵乘法。把矩阵T的行和b的列的元素执行gmult，结果进行异或运算，
*运算结果存放在d数组里面
*a = {0x02, 0x01, 0x01, 0x03}
*b = {
*		b[0], 
*		b[1], 
*		b[2], 
*		b[3],
*	}
*矩阵 T = {
*	a[0],a[3],a[2],a[1],
*	a[1],a[0],a[3],a[2],
*	a[2],a[1],a[0],a[3],
*	a[3],a[2],a[1],a[0],
*} 
*
*@function   : CoefMult
*
*@See        : FIPS 197 5.1.3
*
*@param      : BYTE *a
*              BYTE *b
*              BYTE *d
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/

void CoefMult(BYTE *a, BYTE *b, BYTE *d) 
{
	BYTE idata i;
	BYTE code index1[] = {3,0,1,2};
	BYTE code index2[] = {2,3,0,1};
	BYTE code index3[] = {1,2,3,0};
	
	for(i=0; i <4 ; i++)
	{
		d[i] =  GMult( a[i] , b[0] ) ^ GMult( a[ index1[i] ] , b[1] ) ^ GMult( a[ index2[i] ] , b[2]) ^ GMult( a[ index3[i] ] , b[3] ); //矩阵乘法。把矩阵T的行和b的列的元素执行gmult，结果进行异或运算，运算结果存放在d数组里面
	}
}


/*
*@description: MixColumns，列混合变换。
*列混合(MixColumns( )) 变换在状态上按照每一列进行运算，并将每一列看作 4.3节中描
*述的4 次多项式，即将状态的列看作 GF(2^8)上的多项式且被一个固定的多项式a(x)模 x^4+1
*乘，a(x)为： 
*   a(x)={03}x^3+{01}x^2+{01}x+{02}  
*
*@function   : MixColumns
*
*@See        : FIPS 197 5.1.3
*
*@param      : BYTE *state   State矩阵
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/

void MixColumns_(BYTE *state) 
{
	BYTE xdata a[] = {0x02, 0x01, 0x01, 0x03}; // a(x) = {02} + {01}x + {01}x^2 + {03}x^3
	BYTE idata i, j; 
	BYTE xdata col[4], res[4];

	for (j = 0; j < AES_Nb; j++) 
  {
		
		for (i = 0; i < 4; i++)  // 按列取出state数据
		{
			col[i] = state[ AES_Nb * i + j ];
		}
		
		CoefMult(a, col, res); // 对state的每一列，执行“域运算”
		
		for (i = 0; i < 4; i++) 
		{
			state[ AES_Nb * i + j ] = res[i]; // “域运算”的结果存放回state
		}
	}
}

/*
*@description: AddRoundKey，轮密钥加
* 在轮密钥加变换中，用比特异或把一个轮密钥作用在状态上。
* 每一个轮密钥由通过密钥编排得到的Nb个字组成,将这Nb个字异或到状态的列上，
*[s'0,c s'1,c s'2,c  s'3,c]=[s0,c s1,c s2,c  s3,c] 异或 [w round*Nb+c]  (0<=c<Nb)
*
*@function   : AddRoundKey
*
*@See        : FIPS 197 5.1.4
*
*@param      : BYTE *state   需要处理的状态数据（4*4矩阵）
*							 BYTE *w:轮密钥
*							 BYTE r:使用第r轮密钥（4*4矩阵为一轮）
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/
void AddRoundKey(BYTE *state, BYTE *w, BYTE r) 
{
	
	BYTE idata  c;
  BYTE idata i;
	
	for (c = 0; c < AES_Nb; c++) 
	{
		for(i = 0 ; i < 4 ;i ++)
		{
			state[ AES_Nb * i + c ] = state[ AES_Nb * i + c ] ^ w[ 4 * AES_Nb * r + 4 * c + i];	//用比特异或把一个轮密钥作用在状态上。
		}
	}
}


/*
*@description: SubWord，接受一个 4-byte 输入字，对每个字节应用 S 盒得到输出字。
*
*@function   : SubWord
*
*@See        : FIPS 197 5.2
*
*@param      : BYTE *w   4-byte 输入字
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :

*/

void SubWord(BYTE *w) 
{
	BYTE idata i;

	for (i = 0; i < 4; i++) 
	{
		w[i] = sbox[ w[i] ]; //对每个字节应用 S 盒得到输出字。
	}
}

/*
*@description: RotWord，接受一个4个字节的数组把它们向左旋转一个字节
*接受字[a0,a1,a2,a3] 作为输入，执行循环移位后返回字[a1,a2,a3,a0]
*
*@function   : RotWord
*
*@See        : FIPS 197 5.2
*
*@param      : BYTE *w   4字节数组
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :

*/
void RotWord(BYTE *w) 
{
	BYTE idata tmp;
	BYTE idata i;

	tmp = w[0];

	for (i = 0; i < 3; i++) 
	{
		w[i] = w[i+1];
	}

	w[3] = tmp;
}

/*
*@description: RCon，生成轮常数表
*
*轮常数字数组 Rcon[i]，包含了由[x^(i-1),{00},{00},{00}]给定的值。
*i 从1 开始，不是 0，x^(i-1)是有限域GF(2^8)上x(x记为{02})的指数幂。
*
*@function   : RCon
*
*@See        :FIPS 197 5.2
*
*@param      : BYTE i   
*							 BYTE Rcycle[4]:轮常数
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :
*/
BYTE * RCon(BYTE i,BYTE Rcycle[4]) 
{
	if (i == 1) 
	{
		Rcycle[0] = 0x01; // x^(1-1) = x^0 = 1
	} 
	else if (i > 1) 
	{
		Rcycle[0] = 0x02;
		
		i--;
		
		while ((i-1) > 0) 
		{
			Rcycle[0] = GMult( Rcycle[0], 0x02);
			
			i--;
		}
	}
	
	return Rcycle;
}


/*
*@description: KeyExpansion，秘密密钥字节数组生成密钥编排。
*AES算法接受密码的秘密密钥K，运行密钥扩展程序来生成密钥编排的结果。
*密钥扩展总共生成Nb(Nr+1)个字：该算法需要一个Nb个字组成的初始集合，
*Nr轮中的每一轮需要Nb个字的密钥数据。密钥编排结果由一个4-byte 字的
*线性数组组成，记为[wi]其中0<=i<Nb(Nr+1)。
*@function   : KeyExpansion
*
*@See        : FIPS 197 5.2
*
*@param      : BYTE *key   秘密密钥字节数组。
*              BYTE *w     密钥编排。
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :V1.1 调用 这个函数前  调用这个函数CheckAesKeyLength给 AES_Nr/AES_Nk赋值 或者手动赋值

*/
#if !JAVA_CARD
/*
deleted by yangjian@20221205: 该函数未使用
void KeyExpansion(BYTE *key, BYTE *w) 
{
	BYTE xdata tmp[4];
	BYTE idata i;
	BYTE xdata len; 
	BYTE xdata Rcycle[4];       //轮常数
	
	len = AES_Nb * ( AES_Nr + 1 ); 
	
	Rcycle[0]=0x02;      //轮常数
	
	Rcycle[1]=0x00;
	
	Rcycle[2]=0x00;
	
	Rcycle[3]=0x00;
	
	for (i = 0; i < AES_Nk; i++) // 把秘密密钥的值拷贝到密钥编排 w[], w[] 表是 4 列
	{
		w[4*i] = key[4*i];
		
		w[4*i+1] = key[4*i+1];
		
		w[4*i+2] = key[4*i+2];
		
		w[4*i+3] = key[4*i+3];
	}
	
	for (i = AES_Nk; i < len; i++)  // 计算剩余w[]的值
	{
		tmp[0] = w[4*(i-1)];
		
		tmp[1] = w[4*(i-1)+1];
		
		tmp[2] = w[4*(i-1)+2];
		
		tmp[3] = w[4*(i-1)+3];

		if (i%AES_Nk == 0) 
		{
			RotWord(tmp);//tmp循环左移一个字节

			SubWord(tmp); // 对每个字节按S盒进行替换
			
			CoefAdd(tmp, RCon(i/AES_Nk,Rcycle), tmp);// 与32 bits的常量（RC[i/Nk],0,0,0）进行异或
		} 
		else if ((AES_Nk > 6) && ((i%AES_Nk) == 4))        //256bit密钥长度时候,使用。
		{
			SubWord(tmp);
		}

		w[4*i] = w[4*(i-AES_Nk)]^tmp[0];
		
		w[4*i+1] = w[4*(i-AES_Nk)+1]^tmp[1];
		
		w[4*i+2] = w[4*(i-AES_Nk)+2]^tmp[2];
		
		w[4*i+3] = w[4*(i-AES_Nk)+3]^tmp[3];
	}
}
*/
#endif


/*
*@description: AES Encrypt的基本算法，对16字节的in明文，使用密钥调度表w，进行aes加密，得到16字节的out密文
*
*@function   : AESEncryptCore
*
*@See        : 
*
*@param      : BYTE *in   明文输入
*              BYTE *out  密文输出
*              BYTE *w     
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :

*/
void AESEncryptCore(BYTE *in, BYTE *out, BYTE *w) 
{
	BYTE xdata state[4*AES_Nb];
	BYTE idata r, i, j;
	
	for (i = 0; i < 4; i++)  // 把输入数组拷贝到state(4*4矩阵)
	{
		for (j = 0; j < AES_Nb; j++) 
		{
			state[AES_Nb*i+j] = in[i+4*j];
		}
	}

	AddRoundKey(state, w, 0);
	
	for (r = 1; r < AES_Nr; r++) 
	{
		SubBytes(state);// 把State矩阵中的每个字节替换成一个由Sbox决定的新字节
		
		ShiftRows(state);//把State矩阵中的字节向左旋转
		
		MixColumns_(state);//把State字节列的值进行数学"域加"和"域乘"的结果代替每个字节
		
		AddRoundKey(state, w, r); //轮密钥加
	}

	SubBytes(state);
	
	ShiftRows(state);
	
	AddRoundKey(state, w, AES_Nr);	

	for (i = 0; i < 4; i++) 
	{
		
		for (j = 0; j < AES_Nb; j++) 
		{
			out[i+4*j] = state[AES_Nb*i+j];
		}
	}
}

/* Inverse S-box transformation table
 FIPS 197 5.3.2
 */
 BYTE code invsbox[256] = {
	// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, // 0
	0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, // 1
	0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, // 2
	0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, // 3
	0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, // 4
	0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, // 5
	0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, // 6
	0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, // 7
	0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, // 8
	0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, // 9
	0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, // a
	0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, // b
	0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, // c
	0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, // d
	0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, // e
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};// f

/*
*@description: InvMixColumns，逆列混合(InvMixColumns（）)变换.
*是列混合(MixColumns（）)变换的逆变换
*逆列混合(InvMixColumns （）)变换在状态上对每一列进行运算，将每一列看作4.3 节中描述
*的 4次多项式。将状态的列看作GF(2^8)上的多项式且被一个固定的多项式 ) a逆(x)模 x^4+1
*乘， a逆(x)为：
*   a逆(x)={0b}x^3+{0d}x^2+{09}x+{0e}   
*
*@function   : InvMixColumns
*
*@See        : FIPS 197 5.3.3
*
*@param      : BYTE *state   State矩阵
*
*@return     : void
*
*@author     : zhhj 20180410
*
*@complete   : 
*
*@history    :
*/
void InvMixColumns(BYTE *state) 
{
	BYTE xdata a[] = {0x0e, 0x09, 0x0d, 0x0b}; // a(x) = {0e} + {09}x + {0d}x2 + {0b}x3
	BYTE idata i, j, col[4], res[4];

	for (j = 0; j < AES_Nb; j++) 
  {
		for (i = 0; i < 4; i++) 
		{
			col[i] = state[AES_Nb*i+j];
		}

		CoefMult(a, col, res);

		for (i = 0; i < 4; i++) 
		{
			state[AES_Nb*i+j] = res[i];
		}
	}
}

/*
*@description: InvShiftRows，逆行移位。逆行移位(InvShiftRows （）)变换是行移位(ShiftRows（）)变换的逆变换。
* State的第0行不移位，
* State的第1行被向左旋转3个字节，
* State的第2行被向左旋转2个字节，
* State的第3行被向左旋转1个字节
*
*@function   : ShiftRows
*
*@See        : FIPS 197 5.1.2
*
*@param      : BYTE *state   State矩阵
*
*@return     : void
*
*@author     : zhhj 20180410
*
*@complete   : 
*
*@history    :
*/
void InvShiftRows(BYTE *state) 
{
	BYTE idata  i, k, s, tmp;

	for (i = 1; i < 4; i++) 
	{
		s = 0;
		
		while (s < i) 
		{
			tmp = state[AES_Nb*i+AES_Nb-1];
			
			for (k = AES_Nb-1; k > 0; k--) 
			{
				state[AES_Nb*i+k] = state[AES_Nb*i+k-1];
			}

			state[AES_Nb*i] = tmp;
			
			s++;
		}
	}
}

/*
*@description: InvSubBytes，逆字节替代。逆字节替代(InvSubBytes （）)变换是字节替代(SubBytes（）)变换的逆变换。
*
*@function   : InvSubBytes
*
*@See        : FIPS 197 5.3.2
*
*@param      : BYTE *state   State矩阵
*
*@return     : void
*
*@author     : zhhj 20180410
*
*@complete   : 
*
*@history    :
*/
void InvSubBytes(BYTE *state) 
{
	BYTE idata i, j;

	for (i = 0; i < 4; i++) 
	{
		
		for (j = 0; j < AES_Nb; j++) 
		{
			state[ AES_Nb * i + j ] = invsbox[ state [ AES_Nb * i + j ] ];
		}
		
	}
	
}

/*
*@description: AES Decrypt的基本算法，对16字节的in密文，使用密钥编排w，进行aes解密，得到16字节的out明文
*
*@function   : AESDecryptCore
*
*@See        : 
*
*@param      : BYTE *in   明文输入
*              BYTE *out  密文输出
*              BYTE *w     
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :

*/
void AESDecryptCore(BYTE *in, BYTE *out, BYTE *w) 
{
	BYTE xdata state[ 4 * AES_Nb ];
	BYTE idata r, i, j;

	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < AES_Nb; j++) 
		{
			state[ AES_Nb * i + j ] = in[ i + 4 * j ];
		}
	}

	AddRoundKey(state, w, AES_Nr);

	for (r = AES_Nr - 1; r >= 1; r--) 
	{
		InvShiftRows(state);
		
		InvSubBytes(state);
		
		AddRoundKey(state, w, r);
		
		InvMixColumns(state);
	}

	InvShiftRows(state);
	
	InvSubBytes(state);
	
	AddRoundKey(state, w, 0);

	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < AES_Nb; j++) 
		{
			out[ i + 4 * j] = state[ AES_Nb * i + j ];
		}
	}
}
/*
*@description: ECB模式的AES 
*
*@function   : AESECB
*
*@See        : 
*
*@param      : BYTE *in              输入（明文|密文） 
*              WORD inlen            输入的长度							 
*              BYTE *key             密钥
*              BYTE keylen           密钥的长度	
*              BYTE chipherMode      加密：AES_ENCRYPT 解密：AES_DECRYPT
*              BYTE *out             输出（密文|明文）
*@return     : BYTE                  0 失败 其他：成功，输出的长度
*
*@author     : zhhj 20180410
*
*@complete   : 
*
*@history    :

*/
/*
BYTE AESECB(BYTE *in,WORD inlen, BYTE *key,BYTE keylen,BYTE chipherMode,BYTE *out) 
{
	BYTE xdata tmpInData[16],tmpOutData[16];
	BYTE idata i,n;
	BYTE xdata *w; //密钥编排 128: 4*4*11=176 256:4*4*15=240
	w=tempBuf; //使用tempBuf做密钥编排
	
	if(CheckAesKeyLength(keylen) == FALSE)
	{
		return FALSE;
	}	
	
	if( keylen != (4 * AES_Nk) ) //密钥长度
	{
		return 0;
	}
	
	if ( (inlen % ( 4 * AES_Nb )) != 0 ) //明文输入要是16的倍数,本函数不处理填充,最后一块填充80000...
	{
		return 0;
	}
	
	KeyExpansion(key,w); //产生密钥编排
	
	n = inlen / ( 4 * AES_Nb );
	
	for ( i = 0 ; i < n ; i++)
	{		
		VMReadBuf(tmpInData, (UWORD)(in + i *16), 16);		
		
		if (chipherMode == AES_ENCRYPT)
		{	
			AESEncryptCore(&tmpInData[0], &tmpOutData[0], w);
		}
		else
		{
			AESDecryptCore(&tmpInData[0], &tmpOutData[0], w);
		}
		
		VMWriteBuf((UWORD)(out + i *16), tmpOutData,16 );
	}
	
	return inlen;
}
*/

/*
*@description: CBC模式的AES 
*
*@function   : AESCBC
*
*@param      : BYTE *in              输入（明文|密文） : 支持RAM //    E2地址native平台暂时不支持
*              WORD inlen            输入的长度							 
*              BYTE *key             密钥
*              BYTE keylen           密钥的长度	
*              BYTE *iv              初始向量
*              BYTE ivlen            初始向量的长度
*              BYTE chipherMode      加密：AES_ENCRYPT 解密：AES_DECRYPT
*              BYTE *out             输出（密文|明文）：支持RAM或E2地址
*@return     : BYTE                  0 失败 其他：成功，输出的长度

*
*@nOTE       :此函数会耗用tempBuf资源
*
*@history    : created by zhenghuijie@20180410
*              modify by xuxiaojun@20180512   增加in和out一致时的BUG修正

*/
WORD AESCBC(BYTE *in, WORD inlen, BYTE *key, BYTE keylen, BYTE *iv, BYTE ivlen, BYTE chipherMode, BYTE *out) 
{
	BYTE xdata tmpInData[16],tmpOutData[16];
	BYTE xdata tempIV[16];
	BYTE idata i,n;
	BYTE xdata *w; //密钥编排 128: 4*4*11=176 256:4*4*15=240

	w = tempBuf; //使用tempBuf做密钥编排

	if(CheckAesKeyLength(keylen) == FALSE)
	{
		return FALSE;
	}	
	
	if(keylen != (4 * AES_Nk)) //密钥长度
	{
		return 0;
	}
	
	if(ivlen != (4 * AES_Nb)) //初始向量长度
	{
		return 0;
	}
	
	if ((inlen % ( 4 * AES_Nb ))!=0) //输入要是16的倍数,本函数不处理填充
	{
		return 0;
	}
	
	KeyExpansion(key,w); //产生密钥编排
	
	n = inlen/( 4 * AES_Nb );  //分了多少组
	
	for (i=0;i<n;i++) 
	{	
		//VMReadBuf(tmpInData, (UWORD)(in + i *16), 16);
		//目前  VMReadBuf 和 VMWriteBuf 不支持读取ram地址，后续
		memcpy(tmpInData, (in + i *16), 16);
		
		if (chipherMode==AES_ENCRYPT)
		{			
			Xor(&tmpInData[0 ] , iv , 4 * AES_Nb);//CBC模式 ，先异或，再加密
		
			AESEncryptCore(iv , &tmpOutData[ 0 ] , w);

			memcpy(iv , &tmpOutData[ 0 ] , 4 * AES_Nb);
		}
		else
		{
			memcpy(tempIV, &tmpInData[0], 4 * AES_Nb);   //先备份上一次的源数据
			
			AESDecryptCore(&tmpInData[ 0 ], &tmpOutData[ 0 ], w);

			Xor(iv,&tmpOutData[ 0 ],4 * AES_Nb);//CBC模式 ，先解密，再异或

			memcpy(iv,tempIV,4 * AES_Nb);
		}
		
		//VMWriteBuf((UWORD)(out + i *16), tmpOutData,16 );
		memcpy((out + i *16), tmpOutData, 16 );
	}
	
	return inlen;
}

#endif

/*
*@description: Xor 源数据和目标数据异或，结果放入目标数据
*@function   : Xor
*
*@See        :
*
*@param      :BYTE *src    -- 源数据
*             BYTE *dest  --  目标数据
*             WORD lgth --    数据长度
*
*@return     : void
*
*@author     : zhhj 20180403
*
*@complete   : 
*
*@history    :V1.0 :create by zhhj@20180403

*/
void Xor(BYTE * src, BYTE * dest, WORD lgth)
{
	BYTE i;
	
	for (i=0;i<lgth;i++)
	{
		dest[i] = src[i] ^ dest[i];
	}
}
