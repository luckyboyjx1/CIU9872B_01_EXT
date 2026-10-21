/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
            V1.1 修改函数CheckSQN,每次鉴权成功只写一条SQN记录	    ---xyz@20180117
			V1.2 修改函数CheckSQN,修改当下行SEQ为0时的错误			---yangjian@20180713


 */
#include	"string.h"
#include 	"uicc_main.h"		   
#include "uicc_error.h"
#include 	"fileSystem.h"
#include 	"fileAction.h"
#include 	"gsmpublic.h"
//#include 	"resume.h"
#include 	"Authentication.h"
#include    "typedef.h"
#include    "interface.h"
#include 	<string.h>			 


BYTE xdata G_USIMAlgorithmType;	  	   //00: dummy XOR算法
                                 //01: 仿真系统测试,R1~R5 = {0x40,0x00,0x20,0x40,0x60}
									   //其它值:实网卡

BYTE code TestR[5]={0x40,0x00,0x20,0x40,0x60};


#ifndef SingleSIM

/*
 * S-box transformation table
 */
 BYTE code s_box[256] = {
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
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};// f

 /* 
	实现功能： 将a的值乘以b
	如果该值的最高位为1（表示该数值不小于128），则还需要将移位后的结果异或00011011(0x1b)
	注意：a和b都是传值调用，返回结果保存在p中（也即a左移1位的结果）
	@param:
		a=R[0],
		b=0x02(乘以2，也即将该值的二进制位左移1位)
 */
BYTE gmult(BYTE a, BYTE b) 
{

	BYTE idata p = 0, i = 0, hbs = 0;

	for (i = 0; i < 8; i++) 
	{
		if (b & 1) 
		{
			p ^= a;
		}

		hbs = a & 0x80;
		a <<= 1;
		if (hbs) a ^= 0x1b; // 0000 0001 0001 1011	
		b >>= 1;
	}

	return (BYTE)p;
}

// 数组异或
void coef_add(BYTE a[], BYTE b[], BYTE d[]) 
{

// 	d[0] = a[0]^b[0];
// 	d[1] = a[1]^b[1];
// 	d[2] = a[2]^b[2];
// 	d[3] = a[3]^b[3];
	
	BYTE idata i;
	
	for(i=0 ;i <4 ;i ++)
	{
		d[i] = a[i] ^ b[i];
	}
}

BYTE code index1[] = {3,0,1,2};
BYTE code index2[] = {2,3,0,1};
BYTE code index3[] = {1,2,3,0};
void coef_mult(BYTE *a, BYTE *b, BYTE *d) 
{
	BYTE idata i;
	
	for(i=0; i <4 ; i++)
	{
		d[i] =  gmult(a[i],b[0])^gmult(a[index1[i]],b[1])^gmult(a[index2[i]],b[2])^gmult(a[index3[i]],b[3]);
	}

// 	d[0] = gmult(a[0],b[0])^gmult(a[3],b[1])^gmult(a[2],b[2])^gmult(a[1],b[3]);
// 	d[1] = gmult(a[1],b[0])^gmult(a[0],b[1])^gmult(a[3],b[2])^gmult(a[2],b[3]);
// 	d[2] = gmult(a[2],b[0])^gmult(a[1],b[1])^gmult(a[0],b[2])^gmult(a[3],b[3]);
// 	d[3] = gmult(a[3],b[0])^gmult(a[2],b[1])^gmult(a[1],b[2])^gmult(a[0],b[3]);
}
/*
 * Generates the round constant Rcon[i] //轮常数表
 */
/* 每个轮常数的最左边的字节是GF(28)域中2的幂次方。它的另一个表示方法是其每个
	值是前一个值乘上0x02。注意 0x80 × 0x02 = 0x1b 是 0x80 左移1个比特位
	后紧接着与 0x1b 进行异或。
*/
//BYTE xdata Rcycle[4]; // = {0x02, 0x00, 0x00, 0x00};

/*
 RC = {00, 01, 02, 04, 08, 10, 20, 40, 80, 1B, 36}
 这里通过Rcon函数计算，所得的值和直接索引RC表一样。
*/
 #if !JAVA_CARD
BYTE * Rcon(BYTE i,BYTE Rcycle[4]) 
{
	
	if (i == 1) 
	{
		Rcycle[0] = 0x01; // x^(1-1) = x^0 = 1
	} 
	else if (i > 1) 
	{
		Rcycle[0] = 0x02;
		i--;
		while (i-1 > 0) 
		{
			Rcycle[0] = gmult(Rcycle[0], 0x02);
			i--;
		}
	}
	
	return Rcycle;
}
#endif

 /* 
 轮密钥加
 @param 
	state:需要处理的状态数据（4*4矩阵）
	w:轮密钥
	r:使用第r轮密钥（4*4矩阵为一轮）
 */
void add_round_key(BYTE *state, BYTE *w, BYTE r) 
{
	
	BYTE idata  c;
    BYTE idata i;
	
	for (c = 0; c < 4; c++) 
	{
// 		state[4*0+c] = state[4*0+c]^w[4*4*r+4*c+0];   //debug, so it works for Nb !=4 
// 		state[4*1+c] = state[4*1+c]^w[4*4*r+4*c+1];
// 		state[4*2+c] = state[4*2+c]^w[4*4*r+4*c+2];
// 		state[4*3+c] = state[4*3+c]^w[4*4*r+4*c+3];	
		
		for(i =0 ; i <4 ;i ++)
		{
			state[4*i+c] = state[4*i+c]^w[4*4*r+4*c+i];	
		}
	}
}

/*
将State字节列的值进行数学"域加"和"域乘"的结果代替每个字节
假设State[0,1]的值是0x09，并且列1上的其它值分别为0x60，
0xe1和0x04，那么State[0,1]的新值计算如下：
State[0,1]=(State[0,1]*0x01)+(State[1,1]*0x02)+(State[2,1]*0x03)+(State[3,1]*0x01)
		   =(0x09*0x01)+(0x60*0x02)+(0xe1*0x03)+(0x04*0x01)
		   =0x57
此处加法和乘法是专门的数学域操作，而不是平常整数的加法和乘法
数学域操作 查阅相关资料
*/
void mix_columns(BYTE *state) 
{

	BYTE xdata a[] = {0x02, 0x01, 0x01, 0x03}; // a(x) = {02} + {01}x + {01}x2 + {03}x3
	BYTE idata i, j, col[4], res[4];

	for (j = 0; j < 4; j++) 
  {
		// 按列取出state数据
		for (i = 0; i < 4; i++) 
		{
			col[i] = state[4*i+j];
		}
		// 对state的每一列，执行“域运算”
		coef_mult(a, col, res);
		// 将“域运算”的结果存放回state
		for (i = 0; i < 4; i++) 
		{
			state[4*i+j] = res[i];
		}
	}
}
 /*
 ShiftRows是一个置换操作，它将State矩阵中的字节向左旋转
 State的第0行被向左旋转0个位置，
 State的第1行被向左旋转1个位置，
 State的第2行被向左旋转2个位置，
 State的第3行被向左旋转3个位置
 */
void shift_rows(BYTE *state) 
{

	BYTE idata i, k, s, tmp;

	for (i = 1; i < 4; i++) 
	{
		s = 0;
		while (s < i) 
			{
			tmp = state[4*i+0];
			
			for (k = 1; k < 4; k++) 
			{
				state[4*i+k-1] = state[4*i+k];
			}

			state[4*i+4-1] = tmp;
			s++;
		}
	}
}
 /* 
 将State矩阵中的每个字节替换成一个由Sbox决定的新字节
 比如，State[0,1]的值是0x40如果你想找到它的代替者，你取State[0,1]的值(0x40)并
 让x等于左边的数字(4)并让y等于右边的数字(0)。然后你用x和y作为索引进
 到Sbox表中寻找代替值,Sbox[x,y]
 */
void sub_bytes(BYTE *state) 
{
	BYTE idata  i, j;

	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < 4; j++) 
		{
			state[4*i+j] = s_box[state[4*i+j]];
		}
	}
}

/* 
	使用替换表 Sbox 对一个给定的一行密钥调度表 w[] 进行逐字节替换
	比如0x27的代替结果是 x＝2 和 y＝7，并且 Sbox[2,7] 返回 0xcc
*/
void sub_word(BYTE *w) 
{

	BYTE idata i;

	for (i = 0; i < 4; i++) 
	{
		w[i] = s_box[w[i]];
	}
}

// 接受一个4个字节的数组并将它们向左旋转一个位置
void rot_word(BYTE *w) 
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
AES加密和解密算法使用了一个由种子密钥字节数组生成的密钥调度表。AES规范
中称之为密钥扩展例程（KeyExpansion）。从本质上讲，从一个原始密钥中生成多
重密钥以代替使用单个密钥大大增加了比特位的扩散.
*/
void key_expansion(BYTE *key, BYTE *w) 
{

	BYTE xdata tmp[4];
	BYTE idata i;
	BYTE xdata len = 4*(10+1); // 4*(14+1) = 60, Nk=8,
	BYTE xdata Rcycle[4];       //轮换常数改为局部变量
	// 将种子密钥的值拷贝到密钥调度字节表 w[], 因为 w[] 表总是 4 列，假如种子密钥
	// 是 192 位（24字节），在这种情况下KeyExapansion 将种子密钥拷贝到 w[] 的前面 6 行
	
	Rcycle[0]=0x02;      //轮常数
	Rcycle[1]=0x00;
	Rcycle[2]=0x00;
	Rcycle[3]=0x00;
	
	for (i = 0; i < 4; i++) 
	{
		w[4*i+0] = key[4*i+0];
		w[4*i+1] = key[4*i+1];
		w[4*i+2] = key[4*i+2];
		w[4*i+3] = key[4*i+3];
	}
	
	// 计算剩余w[]的值
	for (i = 4; i < len; i++) 
	{
		// w[j-4]
		tmp[0] = w[4*(i-1)+0];
		tmp[1] = w[4*(i-1)+1];
		tmp[2] = w[4*(i-1)+2];
		tmp[3] = w[4*(i-1)+3];

		if (i%4 == 0) 
		{
			//将tmp循环左移一个字节
			rot_word(tmp);
			// 分别对每个字节按S盒进行映射
			sub_word(tmp);
			// 与32 bits的常量（RC[i/Nk],0,0,0）进行异或
			coef_add(tmp, Rcon(i/4,Rcycle), tmp);

		} 
// 		else if (Nk > 6 && i%Nk == 4)        //256bit密钥长度时候
// 		{
// 			sub_word(tmp);
// 		}

		w[4*i+0] = w[4*(i-4)+0]^tmp[0];
		w[4*i+1] = w[4*(i-4)+1]^tmp[1];
		w[4*i+2] = w[4*(i-4)+2]^tmp[2];
		w[4*i+3] = w[4*(i-4)+3]^tmp[3];
	}
}

void AESEncryptCoreCipher(BYTE *in, BYTE *out, BYTE *w) 
{

	BYTE xdata state[4*4];
	BYTE idata r, i, j;
	// 将输入数组拷贝到state(4*4矩阵)
	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < 4; j++) 
		{
			state[4*i+j] = in[i+4*j];
		}
	}

	add_round_key(state, w, 0);
	// Nr=14
	for (r = 1; r < 10; r++) 
	{
		// 将State矩阵中的每个字节替换成一个由Sbox决定的新字节
		sub_bytes(state);
		// 将State矩阵中的字节向左旋转
		shift_rows(state);
		// 将State字节列的值进行数学"域加"和"域乘"的结果代替每个字节
		mix_columns(state);
		// 轮密钥加
		add_round_key(state, w, r);
	}

	sub_bytes(state);
	shift_rows(state);
	add_round_key(state, w, 10);	

	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < 4; j++) 
		{
			out[i+4*j] = state[4*i+j];
		}
	}
}

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

		memcpy(output, temp, 16);		
	}                                                                                                                    
}

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
	
	AESEncryptCoreCipher(output, output,&tempBuf[80]);
	
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
		memcpy(&sqnAMFsqnAMF[i*8], &SQN[0], 6);

		memcpy(&sqnAMFsqnAMF[i*8+6], &AMF[0], 2);
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

	//AESEncrypt(output, output);
	AESEncryptCoreCipher(output, output,&tempBuf[80]);

	for (i=0; i<16; i++)
	{
		output[i] ^= OPc[i];  // the first 8 byte of output is just XMAC
	}

	return;	
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Milenage2G
  
  Data   content:    output：	鉴权函数计算的输出结果
					 
  Description:       3G网络中进行2G鉴权中的主控函数的实现,网络端的内容存放在RecData[0]开始的地方

  Coder:                     Tester:
  
  Status:			   

 -------------------------------------------------------------------------------------------------------------------*/

void Milenage2G(BYTE * output)
{
	BYTE xdata tmp[16], sRes[4];
	BYTE idata i;
	

	for(i=0; i<16; i++)
	{
		tmp[i] = RAND[i] ^ OPc[i];
	}	

	//AESEncrypt(tmp, tmp);							//得到输入参数Ek的内容


	AESEncryptCoreCipher(tmp, tmp,&tempBuf[80]);					//AES计算,得到F1到F5的公用输入参数
				

	for(i=0; i<16; i++)								//经过AES计算的结果和OPc进行异或
	{
		tmp[i] = tmp[i] ^ OPc[i];					//tmp内保存了F2到F5的输入参数
	}

	F2toF5Function(tmp, output, D_F2_Mode);			//得到sRes的值

	memmove(&output[0], &output[8], 8);

	memset(&output[8], 0x00, 8);

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

	memcpy(&output[1], sRes, 4);

	output[5] = 0x08;

	memcpy(&output[6], tmp, 8);

	return;
}



#endif


