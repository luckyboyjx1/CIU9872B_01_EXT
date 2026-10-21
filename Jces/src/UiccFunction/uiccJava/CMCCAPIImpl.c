/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   Interpreter.c
 *
 * Description: 该文件主要用于实现CMCCAPI等相关API 函数
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
#include "stdlib.h"
#include "CMCCAPIImpl.h"
#include "typedef.h"
#include "Interpreter.h"
#include "Object.h"
#include "Exception.h"
#include "Exception.h"
#include "gsmpublic.h"
#include "arithmatic.h"
#include "fileAction.h"
#include "Authentication.h"
#include "ToolkitAPIPublic.h"
#include "filesystem.h"
#include "anomaly.h"
#include "platform_api.h"
#include "Interface_Boot.h"


#if 0
#define EMPTYMODE 0x00
#define ALLOCATEDMODE 0x01
#define GETDATAMODE 0x00
#define SETDATAMODE 0x01
#define ON 0x01
#define OFF 0x00

/*
*function:	NM_ASCToUCS2
*
*@description:  convert ASCII to UCS2  The method is used in the situation: the difference between the UCS2 format and ASCII format of a character is one byte '00' more.
*
*@函数原型:		public static short ASCToUCS2(byte[] inArray,
                                  short inOffset,
                                  short inLength,
                                  byte[] outArray,
                                  short outOffset) throws NullPointerException,
                                                          ArrayIndexOutOfBoundsException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result UCS2 stream
*
*@exception :
			Throws: 
						java.lang.NullPointerException - if either inArray or outArray is null 
						java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative

*
*@author   :create by lgy
*
*@hisoty   :V1.0: create by lgy @20160929
*/
void NM_CMCCASCToUCS2(void)	
{
	WORD idata inArrayRef,outArrayRef;
	SWORD idata inLen,inOffset,outOffset;
	UWORD xdata inAddr,outAddr;
	BYTE idata i;
	
	outOffset = PopOperandStackShort();
	outArrayRef = PopOperandStackRef();
	inLen = PopOperandStackShort(); 
	inOffset = PopOperandStackShort();
	inArrayRef = PopOperandStackRef();
	

	inAddr = GetValidArrayDataAddr(inArrayRef,inOffset,inLen);
	
	outAddr = GetValidArrayDataAddr(outArrayRef,outOffset,inLen*2);
	
	if(inLen<=256)	//判断是否能装入tempBuf里
	{
		VMReadBuf(&tempBuf[0x100],inAddr,inLen);
	}
	else
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	for(i=0;i<inLen;i++)
	{
		tempBuf[i*2] = 0x00;
		tempBuf[i*2+1] =tempBuf[0x100 + i];
	}
	
	VMWriteBuf(outAddr,tempBuf,inLen*2);
	
	PushOperandStackShort(inLen*2);	
		
}
/*
*function:
*
*@description:  * convert UCS2 to ASCII
				* The method is used in the situation: the difference between the UCS2 format and ASCII format of a character is one byte '00' more.

*
*@函数原型:		public static short UCS2ToASC(byte[] inArray,
                                  short inOffset,
                                  short inLength,
                                  byte[] outArray,
                                  short outOffset) throws NullPointerException,
                                                          ArrayIndexOutOfBoundsException,
                                                          CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result ASCII stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException(INPUT_DATA_INVALID) - if inLength is odd 
					CMCClibException

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/

void NM_CMCCUCS2ToASC(void)	 
{
	WORD idata inArrayRef,outArrayRef,i,curOffset,curLen;
	SWORD idata inLen,inOffset,outOffset;
	UWORD xdata inAddr,outAddr;
	
	outOffset = PopOperandStackShort();
	outArrayRef = PopOperandStackRef();
	inLen = PopOperandStackShort(); 
	inOffset = PopOperandStackShort();
	inArrayRef = PopOperandStackRef();
	
	inAddr = GetValidArrayDataAddr(inArrayRef, inOffset,inLen);
	
	outAddr = GetValidArrayDataAddr(outArrayRef, outOffset,inLen/2);	

	if((inLen%2) != 0)
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID); //CMCClibException(INPUT_DATA_INVALID) - if inLength is odd 
	}
	
	curLen = 0;
	curOffset = 0;
	
	while(inLen > 0)
	{
		curLen = inLen;
		
		if(curLen > 0x100)
		{
			curLen = 0x100;
		}
		
		VMReadBuf(&tempBuf[0x100],inAddr + curOffset,curLen);
		
		for(i=0;i<(inLen/2);i++)
		{
			if((tempBuf[0x100 + i*2] != 0x00)||(tempBuf[0x101 + i*2] > 0x39)||(tempBuf[0x101 + i*2] < 0x30))//ucs2码格式： 00xx,  xx必须在30到39之间
			{
				CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
			}
			
			tempBuf[i] = tempBuf[0x101 + i*2];
		}
		
		VMWriteBuf(outAddr + curOffset/2,tempBuf,curLen/2);
		
		curOffset += curLen;
		
		inLen -= curLen;
		
	}
	
	PushOperandStackShort(curOffset/2);	
	
}

/*
*function:
*
*@description:  * convert ASCII to reverse BCD(ADN format)
				* The method is used in the situation: every byte in the source stream is between "0x30" and "0x39".
				* example: 0x3132 -> 0x21
				* example: 0x313233 -> 0x21F3
*
*@函数原型:		public static short ASCToRBCD(byte[] inArray, 
                                  short inOffset, 
                                  short inLength, 
                                  byte[] outArray, 
                                  short outOffset) throws NullPointerException,
                                                          ArrayIndexOutOfBoundsException,
                                                          CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result RBCD stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException(INPUT_DATA_INVALID) - Some byte in inArray is not between "0x30" and "0x39". 
					CMCClibException

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/

void NM_CMCCASCToRBCD(void)
{
	WORD idata inArrayRef,outArrayRef,i,remain;
	SWORD idata inLen,inOffset,outOffset;
	UWORD xdata inAddr,outAddr;
	
	outOffset = PopOperandStackShort();
	outArrayRef = PopOperandStackRef();
	inLen = PopOperandStackShort(); 
	inOffset = PopOperandStackShort();
	inArrayRef = PopOperandStackRef();
	
	
	inAddr = GetValidArrayDataAddr(inArrayRef,inOffset,inLen);	

	remain = inLen%2; //该变量主要用来保存：输入数据是奇数个时剩余的字节个数 （最后一个字节）
	inLen = (inLen/2)*2;//取2的整数倍处理 
	
	if(inLen > 512)
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}	
	
	outAddr = GetValidArrayDataAddr(outArrayRef,outOffset,(inLen/2) + remain);	
	
	VMReadBuf(&tempBuf[0],inAddr,inLen);

	for(i=0;i<(inLen/2);i++)
	{
		if((tempBuf[i*2] < 0x30 || tempBuf[i*2] > 0x39)	
		||(tempBuf[1 + i*2] < 0x30 || tempBuf[1 + i*2] > 0x39))
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);	//Some byte in inArray is not between "0x30" and "0x39"
		}
		
		tempBuf[i] = ((tempBuf[1 + i*2]<<4)&0xF0) + (tempBuf[i*2]&0x0F); // 高低位颠倒 			
		
	}
				
	if(remain != 0)  //处理最后一个单字节
	{
		tempBuf[0x100] = VMReadByte(inAddr + inLen);
		
		tempBuf[i] = 0xF0 + (tempBuf[0x100]&0x0F);
		
		VMWriteBuf(outAddr,tempBuf,inLen/2 + 1);
		
		PushOperandStackShort((SWORD)(inLen/2 + 1)) ;	
	}
	else
	{
		VMWriteBuf(outAddr,tempBuf,inLen/2);
		
		PushOperandStackShort((SWORD)(inLen/2)) ;
	}

}

/*
*function:
*
*@description:   *  convert reverse BCD to ASCII
				 * 
				 *  example: 0x21 -> 0x3132
				 *  example: 0x21F3 -> 0x313233
				 *  BCD only support digit 0-9 and F, not support A,B,C,D,E
*
 * NOTE: 
				 * inArray and outArray can be the same reference or different reference. 
				 *  If inArray and outArray refer to the same array object, then the conversion of this method should perform as following steps:
				 *  (1) The data from inOffset to inOffset+inLength-1 were converted to a temporary array.
				 *  (2) The data of this temporary array were copied into outArray from outOffset to outOffset+inLength*2-1.

*@函数原型:		public static short RBCDToASC(byte[] inArray,
                                  short inOffset,
                                  short inLength,
                                  byte[] outArray,
                                  short outOffset) throws NullPointerException,
                                                          ArrayIndexOutOfBoundsException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result ASC stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException.INPUT_DATA_INVALID - if the low 4bits of the last byte is "A~F" 
					CMCClibException

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*          :V1.1: modify by xuxiaojun@20191008 白盒测试，增加inLength为0的处理，规范中未明确如何处理，暂时返回0处理
*/

void NM_CMCCRBCDToASC(void)
{
	WORD idata inArray,outArray;
	SWORD idata inOffset,inLength,outOffset;
	UWORD xdata inAddr,outAddr;
	BYTE idata i;
	
	outOffset = PopOperandStackShort();
	outArray = PopOperandStackRef();
	inLength = PopOperandStackShort();
	inOffset = PopOperandStackShort();
	inArray = PopOperandStackRef();
	
	if(inLength == 0)
	{
		PushOperandStackShort(inLength);	
		
		return;
	}
	
	if(inLength > 256)
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	inAddr = GetValidArrayDataAddr(inArray,inOffset, inLength);
	
	outAddr = GetValidArrayDataAddr(outArray,outOffset, inLength*2);

	VMReadBuf(&tempBuf[0x100],inAddr,inLength);
		
	for(i=0;i<inLength;i=i++)
	{		
		tempBuf[i*2+1] = ((tempBuf[0x100 + i]>>4)&0x0F)+0x30;	//取高四位 +0x30  高低位颠倒
		tempBuf[i*2] = (tempBuf[0x100 + i]&0x0F)+0x30;
		
		if((i != inLength-1)&&((tempBuf[i*2] > 0x39) ||(tempBuf[i*2+1] > 0x39 )))	// 不是最后一个字节 转换后必须在在30到39范围
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);	//CMCClibException.INPUT_DATA_INVALID - if the low 4bits of the last byte is "A~F" 
		}			
		else	//最后字节的低位判断
		{
			if(tempBuf[i*2] > 0x39)	//仅最后一个字节的高4比特允许为0x0f 转码后== 0x3F
			{
				CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);	//CMCClibException.INPUT_DATA_INVALID - if the low 4bits of the last byte is "A~F" 
			}
			
		}		
	}
	
	if(tempBuf[inLength*2-1] == 0x3F)	//如果最后一位高位是3F,应该是转换时 F 补位，所以不把它加上去
	{
		inLength = inLength*2 -1;
		
		VMWriteBuf(outAddr,tempBuf,inLength);
	}
	else
	{
		inLength = inLength*2;
		
		VMWriteBuf(outAddr,tempBuf,inLength);
	}
	
	PushOperandStackShort((SWORD)inLength) ;	

}

/*
*function:
*
*@description:  	 * convert ASCII to BCD
					 * The method is used in the situation: every byte in the source stream is between "0x30" and "0x39".
					 *
					 * example: 0x3132 -> 0x12
					 * example: 0x313233 -> throw exception
					 * 
					 * NOTE: 
					 * inArray and outArray can be the same reference or different reference. 

*@函数原型:			 public static short ASCToBCD(byte[] inArray,
                                 short inOffset,
                                 short inLength,
                                 byte[] outArray,
                                 short outOffset) throws NullPointerException,
                                                         ArrayIndexOutOfBoundsException,
                                                         CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result BCD stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException(INPUT_DATA_INVALID) - if inLength is odd 
					CMCClibException


*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/

void NM_CMCCASCToBCD(void)
{
	WORD idata inArray,outArray,i;
	SWORD idata inOffset,inLength,outOffset;
	UWORD xdata inAddr,outAddr;

	outOffset = PopOperandStackShort();
	outArray = PopOperandStackRef();
	inLength = PopOperandStackShort();
	inOffset = PopOperandStackShort();
	inArray = PopOperandStackRef();
	
	if(((inLength%2)!=0)||(inLength > 512))
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);	//if inLength is odd 
	}

	
	inAddr = GetValidArrayDataAddr(inArray, inOffset, inLength);
	
	outAddr = GetValidArrayDataAddr(outArray,outOffset, inLength/2);
	
	VMReadBuf(&tempBuf[0],inAddr,inLength);
	
	for(i = 0;i < inLength;i= i+2)
	{
		if((tempBuf[i] > 0x39)||(tempBuf[i] < 0x30)
		 ||(tempBuf[1 + i] > 0x39)||(tempBuf[1 + i] < 0x30))
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
		}
		
		tempBuf[i/2] = ((tempBuf[i]<<4)&0xF0) + (tempBuf[1 + i]&0x0F);
		
	}
	
	VMWriteBuf(outAddr,tempBuf,inLength/2);	
		
	PushOperandStackShort((SWORD)inLength/2) ;	
	
}

/*
*function:
*
*@description:  	 * convert BCD to ASCII, 
					 * 
					 * example: 0x00 0x12 -> 0x30303132 (the highest digit 0 should be convert to 0x30, can't be remove )
					 * example: 0x12 -> 0x3132
					 * example: 0x123F - > throw exception 
					 * 
					 *  NOTE: 
					 *  inArray and outArray can be the same reference or different reference. 
					 *  If inArray and outArray refer to the same array object, then the conversion of this method should perform as following steps:
					 *  (1) The data from inOffset to inOffset+inLength-1 were converted to a temporary array.
					 *  (2) The data of this temporary array were copied into outArray from outOffset to outOffset+inLength*2-1.


*@函数原型:			 public static short BCDToASC(byte[] inArray,
                                 short inOffset,
                                 short inLength,
                                 byte[] outArray,
                                 short outOffset) throws NullPointerException,
                                                         ArrayIndexOutOfBoundsException,
                                                         CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result ASC stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException.INPUT_DATA_INVALID - if the low 4bits of the last byte is "A~F" 
					CMCClibException

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/

void NM_CMCCBCDToASC(void)
{
	WORD idata inArray,outArray;
	SWORD idata inOffset,inLength,outOffset;
	UWORD xdata inAddr,outAddr;
	BYTE idata i;

	outOffset = PopOperandStackShort();
	outArray = PopOperandStackRef();
	inLength = PopOperandStackShort();
	inOffset = PopOperandStackShort();
	inArray = PopOperandStackRef();
	
	if(inLength > 256)//输入输出数组可能是同一个，这里暂时先限制长度 所有数据用tempbuf一次性转   避免边转换边写时区域覆盖 
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}

	inAddr = GetValidArrayDataAddr(inArray, inOffset, inLength);	
	
	outAddr = GetValidArrayDataAddr(outArray,outOffset, inLength*2);
			
	VMReadBuf(&tempBuf[0x100],inAddr,inLength);
	
	for(i=0;i<inLength;i++)
	{
		tempBuf[i*2] = ((tempBuf[0x100 + i]>>4)&0x0F)+0x30;
		tempBuf[i*2+1] = (tempBuf[0x100 + i]&0x0F)+0x30;
		
		if((tempBuf[i*2+1] > 0x39)||(tempBuf[i*2] > 0x39))  //BCD和RBCD的区别在于不需要补码？？？？？  这里将所有半字节都判断了 限制在0到9
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);	//CMCClibException.INPUT_DATA_INVALID - if the low 4bits of the last byte is "A~F" 
		}
	}
	
	VMWriteBuf(outAddr,tempBuf,inLength*2);
	
	PushOperandStackShort((SWORD)inLength*2) ;	
}

/*
*function:
*
*@description:  	 * convert ASCII to HEX
					 * The method is used in the situation: every byte in the source stream is between "0x30" and "0x39".
					 * 
					 * example: 0x3132(ascii) -> 12(decimal) -> 0x0C(hex)
					 * example: 0x3635353335(ascii) -> 65535(decimal) -> 0xFFFF(hex)
					 * 
					 * NOTE: 
					 * inArray and outArray can be the same reference or different reference. 


*@函数原型:			 public static short ASCToHEX(byte[] inArray,
                                 short inOffset,
                                 short inLength,
                                 byte[] outArray,
                                 short outOffset) throws NullPointerException,
                                                         ArrayIndexOutOfBoundsException,
                                                         CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result HEX stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException(INPUT_DATA_INVALID) - If decimal value of ascii is more than '2147483647'(0x7FFFFFFF), or less than zero. The ascii code in the inArray is out of range from 0x30 to 0x39. 
					CMCClibException
*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCASCToHEX(void)
{
	WORD idata inArray,outArray;
	SWORD idata inOffset,inLength,outOffset;
	UWORD xdata inAddr,outAddr;
	UWORD xdata value;
	BYTE idata i;
	BYTE xdata buf[11];

	outOffset = PopOperandStackShort();
	outArray = PopOperandStackRef();
	inLength = PopOperandStackShort();
	inOffset = PopOperandStackShort();
	inArray = PopOperandStackRef();
	
	inAddr = GetValidArrayDataAddr(inArray, inOffset, inLength);
	
	if((inLength > 10)||(inLength == 0))
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}

	buf[0] = inLength;
	
	VMReadBuf(&buf[1],inAddr,buf[0]);
	
	for(i=0;i<buf[0];i++)
	{
		if(((buf[i+1]&0xF0) != 0x30)||((buf[i+1]&0x0F) > 0x09))
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);		//检查合法性(ascii的值在0x30~0x39)
		}
	}
	
	value =0;
	
	for(i=1;i<=buf[0];i++)  //转成16进制
	{
		value = value*10 + (buf[i]&0x0F);
	}
	
	if(value > 0x7FFFFFFF)  //value是UWORD类型  若当成有符号数用 小于0时 最高位是比特1  CMCClibException(INPUT_DATA_INVALID) - If decimal value of ascii is more than '2147483647'(0x7FFFFFFF), or less than zero.
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	buf[0] = 4; //转换后的数据长度
	buf[1] = (value>>24)&0xFF;
	buf[2] = (value>>16)&0xFF;
	buf[3] = (value>>8)&0xFF;
	buf[4] = value&0xFF;
	
	for(i=1;i<4;i++) //去掉无效的0（最多保留1个0）
	{
		if(buf[i] == 0)
		{
			buf[0]--;
		}
	}
	
	outAddr = GetValidArrayDataAddr(outArray, outOffset, buf[0]);
	
	VMWriteBuf(outAddr,&buf[4 - buf[0] + 1],buf[0]);
	
	PushOperandStackShort((SWORD)buf[0]) ;	
}

/*
*function:
*
*@description:  	 * convert UWORD HEX to ASCII
					 
* @param :		UWORD value  需要转换的16进制数 最大到0x7FFFFFFF
* @param :		BYTE* buf    转成十进制后存入该数组 LV结构 不对数组越界判断 外部控制
*
* @return ：    无
*
*@author   :create by lgy
*
*@hisoty   :V1.0: create by  @20160929
*/
void ConvertHexToASCII(UWORD value,BYTE* buf)
{
	BYTE idata i;
	UWORD xdata mask;
	
	buf[0] = 10;       //保存最大长度
	mask = 1000000000; //0x7FFFFFFF转为十进制为2147483647  ，用value/mask可以获取转为十进制后的最高位
		
	for(i = 1;i<=10;i++)
	{
		buf[i] = value/mask + 0x30;
		
		value = value%mask;
		
		mask = mask/10;
	}
	
	for(; buf[0] > 1;)//去掉无效的30 最多留1位 （输入的数据转换后为10位数 前面可能为补0数据 转成十进制asc后是30，需要去掉）
	{	
		if(buf[1] != 0x30)
		{
			break;
		}
		else
		{
			ecpMem_Move(&buf[1], &buf[2], buf[0]-1);

			buf[0] --;
		}
	}
}

/*
*function:
*
*@description:  	 * convert HEX to ASCII
					 * 
					 * example: 0x0C(hex) -> 12(decimal) -> 0x3132(ascii)
					 * example: 0xFFFF(hex) -> 65535(decimal) -> 0x3635353335(ascii)
					 * example:  0x00 0x00 0x10(hex) - 16 (decimal)- 3136(ascii)  (Remove the useless 0x00 in the highest byte.)
					 * 
					 *  NOTE: 
					 *  inArray and outArray can be the same reference or different reference. 
					 *  If inArray and outArray refer to the same array object, then the conversion of this method should perform as following steps:
					 *  (1) The data from inOffset to inOffset+inLength-1 were converted to a temporary array.
					 *  (2) The data of this temporary array were copied into outArray from outOffset to outOffset+inLength*2-1.


*@函数原型:			 public static short HEXToASC(byte[] inArray,
                                 short inOffset,
                                 short inLength,
                                 byte[] outArray,
                                 short outOffset) throws NullPointerException,
                                                         ArrayIndexOutOfBoundsException,
                                                         CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		inLength       input data length
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the length of the result ASC stream
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative 
					CMCClibException(INPUT_DATA_INVALID) - If inArray value is more than 0x7FFFFFFF or less than zero. 
					CMCClibException
*
*@author   :create by lgy
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCHEXToASC(void)
{
	WORD idata inArray,outArray;
	SWORD idata inOffset,inLength,outOffset;
	UWORD xdata inAddr,outAddr,value;
	
	outOffset = PopOperandStackShort();
	outArray = PopOperandStackRef();
	inLength = PopOperandStackShort();
	inOffset = PopOperandStackShort();
	inArray = PopOperandStackRef();

	inAddr = GetValidArrayDataAddr(inArray, inOffset, inLength);
	
	if((inLength > 4)||(inLength == 0))
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	ecpMem_Set(tempBuf,0x00,4);
	
	VMReadBuf((BYTE*)&tempBuf[4-inLength],inAddr,inLength);
	
	value = ((UWORD)tempBuf[0]<<24)&0xFF000000;
	value += ((UWORD)tempBuf[1]<<16)&0xFFFF0000;
	value += ((UWORD)tempBuf[2]<<8)&0xFFFFFF00;
	value += tempBuf[3];
	
	if(value >0x7FFFFFFF) // If inArray value is more than 0x7FFFFFFF or less than zero.  注：有符号数高位是符号位,此处当成无符号数 数值上大于0x7FFFFFFF时是负数
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	ConvertHexToASCII(value,tempBuf);
	
	outAddr = GetValidArrayDataAddr(outArray,outOffset, tempBuf[0]);
	
	VMWriteBuf(outAddr,&tempBuf[1],tempBuf[0]);
	
	PushOperandStackShort((SWORD)tempBuf[0]) ;	
}

/*
*function:
*
*@description:  	 * Format balance value as the correct format for display text.
					 * 
					 * Example1: inDataArray: 0x00,0x34,0x56,0x78    --> outStringArray: 34300.08  -UCS2 code
					 * Example2: inDataArray: 0x00,0x00,0x00,0x20    --> outStringArray: 0.32  -UCS2 code
					 * 
					 *  NOTE: 
					 *  inDataArray and outStringArray can be the same reference or different reference. 
					 *  If inDataArray and outStringArray refer to the same array object, then the conversion of this method should perform as following steps:
					 *  (1) The data from inOffset to inOffset+inLength-1 were converted to a temporary array.
					 *  (2) The data of this temporary array were copied into outArray from outOffset to outOffset+inLength*2-1.


*@函数原型:			public static short balanceFormat(byte[] inDataArray,
                                      short inDataOffset,
                                      byte[] outStringArray,
                                      short outStringOffset) throws NullPointerException,
                                                                    ArrayIndexOutOfBoundsException,
                                                                    CMCClibException
*
* @param :		inArray        input data array
* @param :		inOffset	   input data offset
* @param :		outArray	   output data array
* @param :		outOffset	   output data offset
*
* @return the offset of the ending of display content. i.e. outStringOffset + output String Length
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either inDataArray or outStringArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds 
					CMCClibException(INPUT_DATA_INVALID) - If inDataArray value is more than 0x7FFFFFFF or less than zero. 
					CMCClibException

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCBalanceFormat(void)
{
	WORD idata inArray,outArray;
	SWORD idata inOffset,inLength,outOffset;
	UWORD xdata inAddr,outAddr,value;
	BYTE idata i;

	outOffset = PopOperandStackShort();
	outArray = PopOperandStackRef();
	inOffset = PopOperandStackShort();
	inArray = PopOperandStackRef();
	
	inLength = 4;
	
	inAddr = GetValidArrayDataAddr(inArray,inOffset, inLength);
	
	VMReadBuf((BYTE*)&tempBuf[0],inAddr,inLength);
	
	value = ((UWORD)tempBuf[0]<<24)&0xFF000000;
	value += ((UWORD)tempBuf[1]<<16)&0xFFFF0000;
	value += ((UWORD)tempBuf[2]<<8)&0xFFFFFF00;
	value += tempBuf[3];
	
	if(value > 0x7FFFFFFF) // If inArray value is more than 0x7FFFFFFF or less than zero.  注：有符号数高位是符号位,此处当成无符号数处理 数值上大于0x7FFFFFFF时是负数
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
}

	ConvertHexToASCII(value,tempBuf); //转成10进制 ascii   30~39
	
	if(tempBuf[0] ==1) //小数点前至少保留一位，小数点后保留2位   0.01/0.02 ....
	{
		tempBuf[4] = tempBuf[1];
		tempBuf[3] = 0x30;
		tempBuf[2] = 0x2E; //小数点 转为UCS2后为 002E
		tempBuf[1] = 0x30;
		tempBuf[0] = 4; 
		
	}
	else if(tempBuf[0] ==2) // 0.xx
	{
		tempBuf[4] = tempBuf[2];
		tempBuf[3] = tempBuf[1];
		tempBuf[2] = 0x2E; //小数点 转为UCS2后为 002E
		tempBuf[1] = 0x30;
		tempBuf[0] = 4; 
	}
	else
	{
		tempBuf[tempBuf[0] + 1] = tempBuf[tempBuf[0]];
		tempBuf[tempBuf[0]] = tempBuf[tempBuf[0] - 1];
		
		tempBuf[tempBuf[0] - 1] = 0x2E; //倒数第二个数之前补小数点
		tempBuf[0]++;
	}
	
	for(i = 0;i<tempBuf[0];i++)//将ascii转为ucs2 在前面补00
	{
		tempBuf[20 + i*2] = 0x00;
		tempBuf[20 + i*2 + 1] = tempBuf[i+1];
	}
	
	tempBuf[0] =tempBuf[0]*2;
		
	outAddr = GetValidArrayDataAddr(outArray, outOffset, tempBuf[0]);
	
	VMWriteBuf(outAddr,&tempBuf[20],tempBuf[0]);
	
	PushOperandStackShort((SWORD)(outOffset + tempBuf[0])) ;		
	
}

/*
*function:
*
*@description:  	 *Convert byte to short type


*@函数原型:			public static short byteToShort(byte byteToconvert)
*
* @param :		byteToconvert     

*
* @return short result
*
*@exception :
			Throws: 
					CMCClibException - with the following reason codes: 
					INPUT_DATA_INVALID if an input parameter is not valid (e.g. occurence = 0)
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCByteToShort(void)
{
	BYTE idata value;
	
	value = (SBYTE)PopOperandStackShort();
	
	PushOperandStackShort((SWORD)(WORD)value) ;
}

/*
*function:
*
*@description:  	 * Add arrayB (addend加数 ) to arrayA (augend被加数 )
					 * Note: Max Length of Sum is 4 bytes.if beyond 4-bytes, return 1.  
					 * 
					 * NOTE:
					 *  iaugend, iaddend and osum can be the same reference or different reference.
					 *

*@函数原型:			public static byte arrayAdd(byte[] iaugend,
                                short augoffset,
                                byte[] iaddend,
                                short addoffset,
                                byte[] osum,
                                short offset)  throws NullPointerException,
                                                      ArrayIndexOutOfBoundsException,
                                                      CMCClibException
*
* @param iaugend        the valid length of iaugend is always fixed as 4-bytes to support 4-bytes length operation. 
 *                       In buffer, right-align is adopted.  e.g. 0x11 00 12 34
 *                       The data in the buffer is hexidecimal string.
 * @param augoffset      Offset of the iaugend buffer data.
 * @param iaddend        The valid length of iaddend is always fixed as 4-bytes to support 4-bytes length operation. 
 *                       In buffer, right-align is adopted. e.g. 0x81 00 12 34
 *                       The data in the buffer is hexidecimal string.
 * @param addoffset      Offset of the iaddend buffer data.
 * @param osum           The buffer which store the result of operation. The valid length of osum is always fixed 
 *                       as 4-bytes to support 4-byte length operation.In buffer, right-align is adopted.  e.g.   0x00 00 12 34
 * @param offset         Offset of the osum output data. 

*
* @return the caculation result
			             0 - caculate successfully
			             1 - caculate incorrectly, eg. the lenght of caculated result is bigger than 4 bytes
*
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either iaddend or iaugend or osum is null 
					java.lang.ArrayIndexOutOfBoundsException - if adding would cause access of data outside array bounds 
					CMCClibException

*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCArrayAdd(void)
{
	WORD idata temp1,temp2;
	BYTE idata i,augend[4],addend[4],osum[4];
	WORD idata arrayAddRef,iaddendRef,osumRef;
	SWORD idata augoffset,addoffset,osumOffset;
	UWORD xdata arrayAddAddr,iaddendAddr,osumAddr;
	
	osumOffset = PopOperandStackShort();
	osumRef = PopOperandStackRef();
	
	addoffset = PopOperandStackShort();
	iaddendRef = PopOperandStackRef();
	
	augoffset = PopOperandStackShort();
	arrayAddRef = PopOperandStackRef();

	osumAddr = GetValidArrayDataAddr(osumRef, osumOffset, 4);  //被加数 加数 结果 均为4字节
	
	iaddendAddr = GetValidArrayDataAddr(iaddendRef,addoffset, 4);// 加数
	
	VMReadBuf((BYTE*)&addend, iaddendAddr, 4);//加数
	
	arrayAddAddr = GetValidArrayDataAddr(arrayAddRef,augoffset, 4);  //被加数
	
	VMReadBuf((BYTE*)&augend, arrayAddAddr, 4);//被加数
	
	temp2 =0;//进位
	
	for(i=0;i<4;i++)
	{
		temp1 = augend[3-i];//被加数
		temp1 += temp2; //加上上一次的进位信号
		temp1 += addend[3-i];
		
		osum[3-i] = temp1&0xFF;
		temp2 = (temp1>>8)&0x00FF; //进位						
	}
	
	if(temp2 != 0)//运算溢出
	{
		PushOperandStackShort(1);//the lenght of caculated result is bigger than 4 bytes
		
		return ;
	}
	
	VMWriteBuf(osumAddr, osum, 4);//将结果写入指定地址
	
	PushOperandStackShort(0);
}

/*
*function:
*
*@description:  	 * Do 4 byte array subtration, it should be guaranteed that the minuend（被减数） is not less than the
					 * subtrahend（减数）, if the minuend（被减数）is less than subtrahend（减数）, reture result 1.
					 * Note: Max Length of Subtrahend and Minuend is 4 bytes.
					 * 
					 * NOTE:
					 * iaMinuend, iaSubtrahend and oaDifference can be the same reference or different reference.
					 *

*@函数原型:			public static byte arraySub(byte[] iaMinuend,
	                                 short sMinuendOffset,
                                         byte[] iaSubtrahend,
                                         short sSubtrahendOffset,
                                         byte[] oaDifference,
                                         short sDiffOffset) throws NullPointerException,
                                                               ArrayIndexOutOfBoundsException,
                                                               CMCClibException
*
* @param iaMinuend              Buffer to store the value of minuend（被减数）.
 *                               The valid length of iaMinuend is always fixed as 4-bytes to support 4-byte length operation.
 *                               In buffer, right-align is adopted. e.g.   0x12 34 12 34
 * @param sMinuendOffset         the offset of the iaMinuend valid data
 * @param iaSubtrahend           Buffer to store the value of subtrahend（减数）.
 *                               The valid length of iaSubtrahend is always fixed as 4-bytes to support 4-byte length operation.
 *                               In buffer, right-align is adopted. e.g.   0x65 00 12 34
 * @param sSubtrahendOffset      the offset of the iaSubtrahend valid data
 * @param oaDifference           the buffer which store the operation result. the valid length of oaDifference is always fixed 
 *                               as 4-bytes to support 4-byte length operation.In buffer, right-align is adopted
 *                               e.g. 0x00 00 12 34
 * @param sDiffOffset            the offset of the operation result.    

*
* @return the caculation result
 *           0 - caculate successfully
 *           1 - caculate incorrectly, eg. the data in minuend（被减数）is smaller than data in subtrahend（减数）
 *
*
*@exception :
			Throws: 
					java.lang.NullPointerException - if either iaSubtrahend or iaMinuend or oaDifference is null 
					java.lang.ArrayIndexOutOfBoundsException - if decreasing would cause access of data outside array bounds 
					CMCClibException
*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCArraySub(void)
{
	UWORD xdata iaMinuend,iaSubtrahend,oaDifference;
	UWORD xdata iaMinuendAddr,iaSubtrahendAddr,oaDifferenceAddr;
	WORD idata  iaMinuendRef,iaSubtrahendRef,oaDifferenceRef;
	SWORD idata iaMinuendOffset,iaSubtrahendOffset,oaDifferenceOffset;
	BYTE idata temp[4];

	oaDifferenceOffset = PopOperandStackShort();
	oaDifferenceRef = PopOperandStackRef();
	iaSubtrahendOffset = PopOperandStackShort();
	iaSubtrahendRef = PopOperandStackRef();
	iaMinuendOffset = PopOperandStackShort();
	iaMinuendRef = PopOperandStackRef();
	
	oaDifferenceAddr = GetValidArrayDataAddr(oaDifferenceRef, oaDifferenceOffset, 4);  //被减数 减数 结果 均为4字节
	
	iaSubtrahendAddr = GetValidArrayDataAddr(iaSubtrahendRef, iaSubtrahendOffset, 4);  //被减数 减数 结果 均为4字节
	
	VMReadBuf(temp, iaSubtrahendAddr, 4);//减数
	
	iaSubtrahend = ((UWORD)temp[0]<<24) + ((UWORD)temp[1]<<16) + ((UWORD)temp[2]<<8) + temp[3];
	
	iaMinuendAddr = GetValidArrayDataAddr(iaMinuendRef, iaMinuendOffset, 4);  //被减数 减数 结果 均为4字节
	
	VMReadBuf(temp, iaMinuendAddr, 4);//被减数
	
	iaMinuend = ((UWORD)temp[0]<<24) + ((UWORD)temp[1]<<16) + ((UWORD)temp[2]<<8) + temp[3];
	
	if(iaMinuend < iaSubtrahend)
	{
		PushOperandStackShort(1); //1 - caculate incorrectly, eg. the data in minuend（被减数）is smaller than data in subtrahend（减数） 
		
		return ;
	}

	oaDifference = iaMinuend - iaSubtrahend;
	
	VMWriteInt(oaDifferenceAddr, oaDifference);
	
	PushOperandStackShort(0);//0 - caculate successfully 
	
}

/*
*function:
*
*@description:  	 * Looks for the indicated occurence of a TLV element from the beginning of
					 * the TLV list (inArray buffer). If the method is successful ,return the offset of 
					 * the corresponding TLV , else reture result -1.
					 * This search method should compare and find the full matched tag.
					 *
					 * TLV format is BER-TLV.

*@函数原型:			public static short findTLV(byte tag, byte occurrence, byte[] inArray,
                            short inOffset,short inLength) throws CMCClibException, 
                         			                  NullPointerException,
			                                          ArrayIndexOutOfBoundsException
*
* @param tag            the tag of the TLV element to search
* @param occurrence     the occurrence number of the TLV element (1 for the first, 2 for the second...)
* @param inArray        input data array	
* @param inOffset       input data offset
* @param inLength       input data length

*
* @return  offset of the corresponding TLV,if the required occurrence of the TLV element does not exist,return result -1
*
*@exception :
			Throws: 
					CMCClibException - with the following reason codes: 
					INPUT_DATA_INVALID if an input parameter is not valid (e.g. occurence = 0)
					java.lang.NullPointerException - if either inArray or outArray is null 
					java.lang.ArrayIndexOutOfBoundsException - if converting would cause access of data outside array bounds or if inLength is negative
*
*@author   :create by 
*
*@hisoty   :V1.0: create by  @20160929
*/
void NM_CMCCFindTLV(void)
{
	WORD idata inArrayRef,offset,tagLen;
	SWORD idata inOffset,inLen;
	SBYTE idata tag,occurrence;
	BYTE idata num,i,temp[4];
	UWORD xdata inAddr;
	
	
	inLen = PopOperandStackShort();
	inOffset = PopOperandStackShort();
	inArrayRef = PopOperandStackRef();
	occurrence = PopOperandStackShort();
	tag = PopOperandStackShort();
	
	inAddr =GetValidArrayDataAddr(inArrayRef,inOffset, inLen);
	
	if(occurrence == 0)
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
		
	i = 0;
	tagLen = 0;
	offset = 0;
	num =  1;
	
	for(offset = 0; offset < inLen;)
	{
		VMReadBuf(temp, inAddr+offset, 4);//TL  最小2字节 最大4字节 
		
		if(temp[1] < 0x80)
		{
			i = 0x02;           //tag len (1 byte)
			
			tagLen = temp[1];
		}
		else if(temp[1] == 0x81)
		{
			i = 0x03;           //tag(1 byte) + 0x81(1 byte) + len (1 byte)
			
			tagLen = temp[2];
		}
		else if(temp[1] == 0x81)
		{
			i = 0x04;          //tag(1 byte) + 0x82(1 byte) + len (2 bytes)
			
			tagLen = (((WORD)temp[2] << 8)&0xFF00) + temp[3];
		}
		else // tlv 不合法
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
		}
		
		if(tag == temp[0])
		{			
			if(occurrence == num)
			{
				PushOperandStackShort(offset);//需要返回value处的偏移还是tag处？
				
				return ;
			}
			
			num++;
		}
		
		offset += (tagLen + i);
	}
	
	PushOperandStackShort(-1);
		
}
/*
*@function:   CMCCExistedSFI
*
*@description:判断当前是否存在同样的SFI文件
*
*@param       : BYTE SFI
*@param       : WORD fileObjectRef
*@param       : BYTE mode :mode:0x00：查找当前SFI是否存在/ 0x01:查找一个空的引用位置
*
*@return      :SFI：存在则返回当前数组对象的引用 or 当前有效引用位置+1 /else:00
×
×@author      : xuyz
*
*@history     :V1.0:create by xuyz
*/
WORD CMCCExistedSFI(BYTE currentSFI, WORD  fileHeadRef, BYTE mode)
{
    UWORD xdata fileHeadAddr;
    WORD idata fileHeadAlllen,i;  
    BYTE idata tmpSFI,fileExist;
    
    fileHeadAddr = GetObjectHeaderAddr(fileHeadRef);
    
    fileHeadAlllen = VMReadShortCode(fileHeadAddr + ObjLen_Offset);  //读取当前引用对象所存储的引用数目
    
    fileHeadAddr =  GetObjDataAddr(fileHeadAddr);
    
    fileExist = FALSE;  //当前文件不存在
    
    for(i=0; i< fileHeadAlllen;  i += CMCC_FileHead_Len)
    {
        fileExist = VMReadByte(fileHeadAddr + i + CMCC_FileExist_Offset);
        
        if(mode == 0x00)
        {
            if(fileExist == TRUE)   //该偏移处的引用不为空，则判断该引用处存储的SFI与待比较的SFI 是否一致
            {
                tmpSFI = VMReadByte(fileHeadAddr + i + CMCC_FileSFI_Offset);
                
                if((tmpSFI&0x1F) == (currentSFI&0x1F))
                {
                   return  ((i/CMCC_FileHead_Len)+1);  //返回该SFI在文件头的第i-1个位置处
                }
            }
        }
        else   //查找一个空引用，即判断当前文件数目是否已经达到最大
        {
            if(fileExist == FALSE)
            {
                return ((i/CMCC_FileHead_Len) + 1);  //返回该SFI在文件头的第i-1个位置处
            }
        }
        
    }
    
    return FALSE;
	  
}
/*
*@function:NM_CMCCCreateEFFile
*
*@descirption : Create EF based on the file information provided in filoInfo[] buffer. The data in the file is not defined in this method.
*
* @param      :BYTE FileInfo:存储当前管理的所有文件信息 
* @param      :BYTE *fileInfo:byte 0=FID   byte 1=file type  byte 2~4=access right  byte 5~6=file length
* @param      :Data offset
* @param      :Data length
*
* @return     0 succeed  / 1 failed 
*
* @exception CMCClibException.INPUT_DATA_INVALID - if File type is record or Cyclic record, if number of records equal to 00 or the length of records equal to 00.
*                                                - if File type is transparent file or variable record file, if file size equal to 0x00 0x00.
 *                                                - if File ID has been existed.	
 *            CMCClibException.SW_NO_NVM_RESOURCE - if NVM memory is not enough.
 *                                             
 * @exception CMCClibException.BEYOND_LIMITATION  - the total file number is beyond the maximum number which is defined in construct method.
 * @exception  NullPointerException     
 *             if <code>buffer</code> is <code>null</code>
 * @exception  ArrayIndexOutOfBoundsException <ul> 
 *             if <code>offset</code> would cause access outside array bounds, or if <code>bLength</code> is negative.
 *
 *@author   :
*  
*/
void NM_CMCCCreateEFFile(void)
{
    WORD idata fileHeadRef,fileInfoRef,nullIndex,size,fileBodyRef,arrayref;
    SWORD idata offset,length;
    UWORD xdata fileInfoAddr,arrayAddr;
    BYTE idata transActioinFlag;
    
    fileBodyRef = PopOperandStackShort();    //存储文件体的引用信息
    
    fileHeadRef = PopOperandStackShort();  //存储当前所有文件头信息
    
    length = PopOperandStackShort();        //输入数据的长度 
    
    offset = PopOperandStackShort();        //输入数据的偏移
    
    fileInfoRef =  PopOperandStackShort();  //输入数据
      
    if(length != 0x07)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
    }
	   
	fileInfoAddr = GetValidArrayDataAddr(fileInfoRef,offset,length); //字节1：SFI, 字节2  fileType，字节3-5：ACCESS right 字节6-7：fileSize or RecordNumber/RecordLength
	
    ecpMem_Set(tempBuf, 0x00, 8);
    
    VMReadBuf(tempBuf, fileInfoAddr, 7); //读取7个字节的fileInfo数据
	
	tempBuf[0] &= 0x1F;                 //文件SFI只保留最后5bit有效数据
	
	tempBuf[1] &= 0x03;                 //文件类型，保留最后2bit
    
    if((tempBuf[1]&0x01) == 0x01)       //01 fixed record file   03 cyclic record file
    {
        if((tempBuf[5] == 0x00)||(tempBuf[6] == 0x00))  //对透明文件或可变记录文件，如果fileSize为0 ，则抛异常INPUT_DATA_INVALID；对于记录文件，如果 记录数目和记录长度为0 ，则也抛异常INPUT_DATA_INVALID
        {
            CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
        }
        
        size = tempBuf[5]  * tempBuf[6];
        
        tempBuf[7] = 0x01;              //初始化记录指针
    }
    else                                // 00 transparent file    02 variable record file 
    {
        if((tempBuf[5] == 0x00)&&(tempBuf[6] == 0x00))  //对透明文件或可变记录文件，如果fileSize为0 ，则抛异常INPUT_DATA_INVALID；对于记录文件，如果 记录数目和记录长度为0 ，则也抛异常INPUT_DATA_INVALID
        {
            CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
        }
        
        size = (((WORD)tempBuf[5] <<8)&0xFF00) | tempBuf[6];
    }
    
    if(CMCCExistedSFI(tempBuf[0],fileHeadRef,0x00) != 0x00)  //该SFI已经存在
    {
       CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID); 
    }
    
    nullIndex = CMCCExistedSFI(0x00,fileHeadRef,0x01);   //he total file number is beyond the maximum number which is defined in construct method.
    
    if(nullIndex == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_BEYOND_LIMITATION);  
    }
    
    transActioinFlag = FALSE;
    
    transActioinFlag = IF_BeginTransaction();
	
    arrayAddr = VMObjectHeapAlloc(size + PermanentObjHeader_Len, &arrayref);	

	if(ADDR_IS_NULL == arrayAddr)	    // 分配E方空间失败
	{  
		IF_AbortTransaction(transActioinFlag);
        
		ThrowError(SYSTEMEXCEPT_NO_RESOURCE);
	}
    
	ecpMem_Set(&tempBuf[0x20], 0x00, PermanentObjHeader_Len);  //初始化对象头数组

    tempBuf[0x20] = OBJECT_TYPE_BYTE;              // u1 objType
	tempBuf[0x21] = (G_CurContext >> 8) & 0xFF;	  // u2 context
	tempBuf[0x22] = G_CurContext & 0xFF;
	tempBuf[0x23] = (size >> 8) & 0xFF;		      // u2 objLen
	tempBuf[0x24] = size & 0xFF;
	
	WritePageCacheEX(arrayAddr, &tempBuf[0x20], PermanentObjHeader_Len); //对于事务中分配的空间，不采用原子写，因为分配空间时，会用事务写ADDR_INDEX_TABLE和FlashUsedAddr，如果事务回滚或者断电时，这两个数据会回写，分配的HEAP中的空间里的数据没有关系，所以事务中分配的就不用事务写
	
	SetEepromNonAtomic(arrayAddr + PermanentObjHeader_Len, 0x00, size); // 初始化数组内容    JCVM7.5.71   All of the elements of the new array ar e initialized to the default initial value for its type

    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));
	
	if(fileInfoAddr == FALSE)
	{
		IF_AbortTransaction(transActioinFlag);
		
		PushOperandStackShort(0x00);    

		return;
	}
    
    VMWriteShort(fileInfoAddr + (nullIndex-1)*2, arrayref);
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileHeadRef));
	
	if(fileInfoAddr == FALSE)
	{
		IF_AbortTransaction(transActioinFlag);
		
		PushOperandStackShort(0x00);    

		return;
	}
    
    ecpMem_Move(&tempBuf[1],&tempBuf[0], 0x08);
    
    tempBuf[0] = 0x01;  //当前文件已创建
    
    VMWriteBuf(fileInfoAddr + (nullIndex-1)*CMCC_FileHead_Len, &tempBuf[0], CMCC_FileHead_Len);
    
    IF_CommitTransaction(transActioinFlag);

    CacheFlush(); //由于对对象头的访问都是采用的是NOCache读的方式，因此此处更新完对象头之后要提交cache

    PushOperandStackShort(0x00);    
}
/*
*@function:NM_CMCCgetType
*
*@description:Get the type of the file indicated by the input file.
*
*@param  :  BYTE FileInfo:存储当前管理的所有文件信息 的引用
*@param  :  Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
*
*@return File type includes 00 transparent file，01 fixed record file，02 variable record file and 03 cyclic recor file. 
*
*@exception : CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.
*
*@author     :
*/
void NM_CMCCgetType(void)
{
    WORD idata fileInfoRef,sfiIndex,currentSFI;
    UWORD xdata fileInfoAddr;
    BYTE idata fileType;   
    
    fileInfoRef = PopOperandStackShort();  //存储当前所有文件对象的头及体的引用信息  
    
    currentSFI = PopOperandStackShort();  //弹出SFI
    
    sfiIndex = CMCCExistedSFI(currentSFI,fileInfoRef,0x00); 
    
    if(sfiIndex == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileInfoRef));
    
    VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileType_Offset, 0x01);
    
    PushOperandStackShort(fileType);
    
}
/*
*@fucntion : NM_CMCCgetAccessRight
*
*@description:Get the access right of the file indicated by the input file.The output of access condition is 3 bytes.
*
*@param      :BYTE FileInfo:存储当前管理的所有文件信
*@param      bSFI : Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
* @param     data: Buffer to store the returned data of access right
* @param    sOffset:  Data offset
*
*@return     void
*
*@exception : * CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.
 * @exception  NullPointerException     *             if <code>buffer</code> is <code>null</code>
 *@exception  ArrayIndexOutOfBoundsException  if <code>offset</code> would cause access outside array bounds, or if <code>bLength</code> is negative.
*
*@author   
*
*@history  :V1.0:create 

*/
void NM_CMCCgetAccessRight(void)
{
    WORD idata fileInfoRef,outRef,sfiIndex,currentSFI;
    UWORD xdata fileInfoAddr,outAddr;
    SWORD idata outOffset; 
    
    fileInfoRef = PopOperandStackShort();  //存储当前所有文件对象的头及体的引用信息  
    
    outOffset = PopOperandStackShort();  
    
    outRef = PopOperandStackShort(); 
    
    currentSFI = PopOperandStackShort();  //弹出SFI
    	
	outAddr = GetValidArrayDataAddr(outRef, outOffset,3);
    
    sfiIndex = CMCCExistedSFI(currentSFI,fileInfoRef,0x00); //该SFI是否存在
    
    if(sfiIndex == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileInfoRef));
    
    VMReadBuf(&tempBuf[0],fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileAccess1_Offset, 0x03);
    
    VMWriteBuf(outAddr,&tempBuf[0], 0x03);

}
/*
*@function  : NM_CMCCgetFileSize
*
*@description :Get the length of the file indicated by the input file. For binary or variable record file, return last 2 bytes in fileInfo;  For fixed or cyclic record file, return record number multiply record length.
*
*@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
 @param      :    bSFI   Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
 * 
* @exception CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.
* 
* @return    the total length of file.
*
*@author     :create by
*
*@history    :V1.0:
*/
void NM_CMCCgetFileSize(void)
{
    WORD idata fileInfoRef,sfiIndex,currentSFI;
    UWORD xdata fileInfoAddr;
    WORD idata fileSize;   
    
    fileInfoRef = PopOperandStackShort();  //存储当前所有文件对象的头及体的引用信息  
    
    currentSFI = PopOperandStackShort();  //弹出SFI
    
    sfiIndex = CMCCExistedSFI(currentSFI,fileInfoRef,0x00); 
    
    if(sfiIndex == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileInfoRef));
    
    fileSize = VMReadShort(fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileRecordNumber_Offset);
    
    PushOperandStackShort(fileSize);
    
}
/**
*@function :NM_CMCCgetRecordNumber
*
*@description: Get the record number of the file indicated by the input file.
*                Note: this method support record type file including fixed record and cycled record, except variable record file.
 *
**@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
* @param      :   bSFI   Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
* 
* @exception CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.  
*                                                - If the variable record file or Transparent file indicated by bSFI
*                                                 
* 
* @return the record number of record type file.
*
*@author   :
*
*@history  :
*/
void NM_CMCCgetRecordNumber(void)
{
    WORD idata fileInfoRef,sfiIndex;
    UWORD xdata fileInfoAddr;
    BYTE idata fileType,currentSFI;   
    
    fileInfoRef = PopOperandStackShort();  //存储当前所有文件对象的头及体的引用信息  
    
    currentSFI = PopOperandStackShort();  //弹出SFI
    
    sfiIndex = CMCCExistedSFI(currentSFI,fileInfoRef,0x00); 
    
    if(sfiIndex == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileInfoRef));
    
    VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileType_Offset, 0x01);
      
    fileType = fileType&0x03;
    
    if((fileType&0x01) == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileRecordNumber_Offset, 0x01);  //读取记录号
    
    PushOperandStackShort(fileType);   
}
/**
*@function   : NM_CMCCgetRecordLength
*
*@description:  Get the record length of the file indicated by the input file.
*               Note: this method support record type file including fixed record, cycled record and variable record file.
*                       If the variable record file indicated by bSFI, then return the whole length of TLV record;
* 
***@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
* @param       : bSFI   Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
* @param       :bRecordNumber   Record number in ABSOLUTE search mode. For the fixed or cyclic record, bRecordNumber is invalid.
* 
* @exception CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.
*                                                - If Transparent file indicated by bSFI
* @exception  NullPointerException     
*             if <code>buffer</code> is <code>null</code>
* 
* @return    the record length of record type file.   For variable record file, the returned RecordLength includes Tag + Length + Value
*
*@author     :
*
*@history    :
*/
void NM_CMCCgetRecordLength(void)
{
    WORD idata fileInfoRef,sfiIndex,fileBodyRef;
    UWORD xdata fileInfoAddr,fileBodyAddr;
    BYTE idata fileType,currentSFI,recordNumber,i;   
    
    fileBodyRef = PopOperandStackShort();
    
    fileInfoRef = PopOperandStackShort();  //存储当前所有文件对象的头及体的引用信息  
    
    recordNumber = PopOperandStackShort(); 
    
    currentSFI = PopOperandStackShort();  //弹出SFI
    
    sfiIndex = CMCCExistedSFI(currentSFI,fileInfoRef,0x00); 
    
    if(sfiIndex == 0x00)
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileInfoRef));
    
    VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileType_Offset, 0x01);
      
    fileType = fileType&0x03;
    
    if(fileType == CMCC_File_Transparent)  //0x00:透明文件
    {
        CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
    }
    
    if((fileType&0x01) == 0x01)  //线性文件或循环文件
    {
        VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileRecordLength_Offset, 0x01);  //读取记录长度
    
        PushOperandStackShort(fileType);   
        
        return;
    }
    else  //可变记录文件
    {        
        VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len + CMCC_FileRecordPointer_Offset, 0x01);  //读取可变记录截至目前已经append的记录数目
        
        if(recordNumber > fileType)  //记录号已越界
        {
             CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
        }
        
        fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef)); 
        
        fileBodyRef = VMReadShort(fileInfoAddr+(sfiIndex-1)*2);  //获取文件体的引用
            
        fileBodyAddr =  GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));
        
        fileInfoRef = 0 ; //复用该变量，当作数据体的当前偏移使用
        
        for(i=1; i <=fileType; i++ )
        {
            VMReadBuf(&tempBuf[0],fileBodyAddr + fileInfoRef , 0x02);  //读取tag L
            
            if(i == recordNumber)
            {
                PushOperandStackShort(tempBuf[1]+2);   
        
                return; 
            }
            
            fileInfoRef += tempBuf[1] +2;      
        }       
    }
}
/*
*@description:  对透明文件进行读写操作
*
*@function    :CMCC_OperateBinaryPublic
*
*@param       :BYTE action
*
*@return      :void
*
*@author      :xuyz              Modify: caibo
*
*@histoory    :V1.0:create by xuyz
*/
void CMCC_OperateBinaryPublic(BYTE action)
{
	WORD idata fileInfoRef,sdataRef ,sfiIndex,fileBodyRef,fileSize;
    UWORD xdata fileInfoAddr,sdataAddr,readAddr,updateAddr;
    SWORD idata sLength,sOffset,initOffset;
    BYTE idata fileType,bSFI;

    fileBodyRef = PopOperandStackShort();
    
    fileInfoRef  =  PopOperandStackShort();  //弹出当前所有的文件头
    
    sLength   = PopOperandStackShort();     //待更新的数据长度
    
    sOffset = PopOperandStackShort();       //待更新的数据起始偏移
    
    sdataRef = PopOperandStackShort();      //待更新的数据源 
    
    initOffset = PopOperandStackShort();    //待更新文件的起始偏移
    
    bSFI = PopOperandStackShort();          //待更新文件的 SFI
	
	if(initOffset < 0)
	{
		ThrowError(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);	
	}
	
	sdataAddr = GetValidArrayDataAddr(sdataRef, sOffset,sLength );
        
    sfiIndex = CMCCExistedSFI(bSFI,fileInfoRef,0x00); 
    
    if(sfiIndex == 0x00)   //待更新的文件不存在
    {
        PushOperandStackShort(0x6A82); 
		return;
    }
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileInfoRef));
    
    VMReadBuf(&fileType,fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileType_Offset, 0x01);
    
    if(fileType != CMCC_File_Transparent)  //0x00:透明文件
    {
         PushOperandStackShort(0x6981); 
		 return;
    }
 
    fileSize = VMReadShort(fileInfoAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordNumber_Offset);
    
    fileInfoAddr = GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));
    
    fileBodyRef = VMReadShort(fileInfoAddr+(sfiIndex-1)*2);  //获取文件体的引用
            
    fileInfoAddr =  GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));   //获取文件体的物理地址
    
    if((initOffset + sLength) > fileSize)
    {
        PushOperandStackShort(0x6B00); 
		return;;    
    }
    
    fileInfoAddr += initOffset;  //指向待更新的偏移处
	
	if(action == CMCC_UpdateAction)
	{
		readAddr = sdataAddr;   //该地址可能是ram也可能是FLASH
		
		updateAddr = fileInfoAddr;	//地址一定是flash
	}
	else   //读操作
	{
		readAddr = fileInfoAddr;  //一定是FLASH地址
		
		updateAddr = sdataAddr;   //地址可能是RAM 或FLASH
	}
	
	readAddr  =  LogicToPhysics(readAddr);    //readAddr地址可能是RAM 也可能是E2地址
	
	if(IsEepromAddr(updateAddr))
	{
		VMWriteBuf(updateAddr,(BYTE *)readAddr ,sLength) ;
	}
	else   //updateAddr地址在RAM中
	{
		VMReadBuf((BYTE*)updateAddr,readAddr,sLength);
	}
    
    PushOperandStackShort(0x9000);
        
}
/*
*@Ffunction : NM_CMCCUpdateBinary
*
*@description :Update binary of transparent file without MAC check  For update Binary with MAC check, see verifyMAC in Security class.
*
*@ @param    :   bSFI  Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
*@param      :   sInitOffset  Offset in the transparent file to point
 @param      :   data   Buffer to store the data to be updated
*@param      :   sOffset Data offset
*@param     :    sLength   Data length
*@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
*
* @return    status word   ‘90’ ‘00’ 执行成功
                          ‘69’ ‘81’ 命令与文件结构不相容
                          ‘6A’ ‘82’ 未找到文件
                          ‘6B’ ‘00’ 参数错误(偏移地址超出了EF)
* @exception CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.   Or length is bigger than maximum length specified when create file
* @exception  NullPointerException      if <code>buffer</code> is <code>null</code>
* @exception  ArrayIndexOutOfBoundsException   if <code>offset</code> would cause access outside array bounds, or if <code>bLength</code> is negative.
*
*@author     :
*
*@history    :V1.0:create function
  
*/
void NM_CMCCUpdateBinary(void)
{
    CMCC_OperateBinaryPublic(CMCC_UpdateAction);    
}

/*
*@function    :NM_CMCCreadBinary
*
*@description : Read binary of transparent file without MAC check. * For Read binary with MAC check, see verifyMAC in Security class. 
*
*@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
* @param     :  bSFI  Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
* @param     : sInitOffset  Offset in the transparent file to point
* @param     : data   Data to be read
* @param     :sOffset:  Data offset
* @param     :sLength  Data length
*
* @return status word
*             SW1 SW2 含 义
*             ‘90’ ‘00’ 执行成功
*             ‘69’ ‘81’ 命令与文件结构不相容
*             ‘6A’ ‘82’ 未找到文件
*             ‘6B’ ‘00’ 参数错误(偏移地址超出了EF)
* @exception  NullPointerException     
*             if <code>buffer</code> is <code>null</code>
* @exception  ArrayIndexOutOfBoundsException <ul> 
*             if <code>offset</code> would cause access outside array bounds, or if <code>bLength</code> is negative.
*
*@author     :
*
*@history    ：
*/
void NM_CMCCreadBinary(void)
{
	 CMCC_OperateBinaryPublic(CMCC_ReadAction);
}
/*
*@function: CMCC_GetRecNo
*
*@description:获取CMCC 文件中的实际记录号，进行读写更新
*
*@param      :UWORD fileHeadAddr :记录头的地址
*@param      :WORD sfiIndex      :该文件的索引位置
*@param      :BYTE currentNum    :当前记录号
*@param      :BYTE mode          :读取模式
*@param       BYTE recordNumber  :记录号
*
*@return     :记录文件中的实际记录号
*
*@author      
*/
BYTE CMCC_GetRecNo(UWORD fileHeadAddr,WORD sfiIndex, BYTE currentNum,BYTE mode,BYTE recordNumber,BYTE action)
{
	BYTE idata recordNo;
	BYTE idata totalRecNum,RecIndicator,fileType;
	
	fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileType_Offset);    //获取文件类型
	
	if((fileType == CMCC_File_CyclicRecord)&&(action == CMCC_UpdateAction)&&(mode != CMCC_RecordMode_Previous))
	{
		PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	   // UpdateRecord: Four modes are defined, of which only PREVIOUS is allowed for cyclic files, and only absolute mode is allowed for variable record files	
		return FALSE;
	}
	
	totalRecNum = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordNumber_Offset);  	

	if(mode == CMCC_RecordMode_CurAndAbsolute)  //在绝对模式下
	{
		if(recordNumber > totalRecNum)
		{
			PushOperandStackShort(SW_RECORD_NOT_FOUND);	
			return FALSE;
		}
	}
	
	RecIndicator = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset);
	
	recordNo = FALSE;
	
	if(fileType == CMCC_File_FixedRecord)  //线性文件
	{
		if(mode == CMCC_RecordMode_Next)
		{
			if( currentNum == 0x00 )
			{
			   	recordNo =  0x01;
			}
			else if(Inrange(currentNum+1,1,totalRecNum) == TRUE)
			{ 
				recordNo = (currentNum+1);
			}
			else
			{
				PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
				return FALSE;
			}	
		}
		else if(mode == CMCC_RecordMode_Previous)
		{
			if(currentNum == 0x00 )
			{
			   	recordNo = totalRecNum;
			}
			else if(Inrange(currentNum-1,1,totalRecNum) == TRUE )
			{ 
				recordNo = currentNum - 1;
			}
			else
			{
				PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
				return FALSE;
			}
		}
		else
		{
			if(recordNumber == 0x00)
			{
				if(Inrange(currentNum,1,totalRecNum) == TRUE)
				{
					recordNo = currentNum;
				}
				else
				{					
					PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
                    
					return FALSE;
				}
			}
			else
			{
			    recordNo= recordNumber;
			}
		}
	}
	else  //循环文件
	{
		if(mode == CMCC_RecordMode_Next)
		{
			if(currentNum == FALSE)
			{
				recordNo = RecIndicator;
			}
			else
			{
				recordNo = Inrange(currentNum+1,1,totalRecNum)?(currentNum+1):(currentNum+1-totalRecNum);
			}

		}
		else if(mode == CMCC_RecordMode_Previous)
		{
			if(action == CMCC_UpdateAction)
			{
				recordNo = Inrange(RecIndicator-1,1,totalRecNum)?(RecIndicator-1):(RecIndicator-1+totalRecNum);
			}		
			else
			{
				if(currentNum == FALSE)
				{
					recordNo = (RecIndicator+(totalRecNum-1) > totalRecNum?(RecIndicator+(totalRecNum-1)-totalRecNum):RecIndicator+(totalRecNum-1));
				}
				else
				{
					recordNo = Inrange(currentNum-1,1,totalRecNum)?(currentNum-1):(currentNum-1+totalRecNum);
				}
			}
		}
		else
		{
			if(recordNumber == 0x00)
			{
				if(currentNum == FALSE)
				{
					PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
					return FALSE;
				}
				else
				{
					recordNo = currentNum;	
				}
			}
			else
			{
				recordNo = ((RecIndicator+recordNumber-1) > totalRecNum?((RecIndicator+recordNumber-1)-totalRecNum):(RecIndicator+recordNumber-1));                 //get the logical recordNo. 	
			}	
		}
	}
	
	return recordNo;

}
/*
*@funtion    : CMCC_OperateRecordPublic
*
*@description: 读取或更新记录文件
*
*@param       ：mode:action
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by @20161009
*/
void CMCC_OperateRecordPublic(BYTE action)
{
	WORD idata fileBodyRef, fileHeadRef,operateRef,operateMode,sfiIndex,currentRecRef;
	SWORD idata operateLength,operateOffset;
	UWORD xdata fileBodyAddr,fileHeadAddr,operateAddr,currentRecAddr;
	BYTE idata fileType,currentRec,operateRecNo,actualLength,i,recordNumber,curSFI;
	BYTE idata tmp[2];
	
	currentRecRef = PopOperandStackShort();  //保存记录文件的当前记录信息
	fileBodyRef = PopOperandStackShort();//保存各个管理文件的文件体信息
    fileHeadRef  =  PopOperandStackShort();  //保存所有文件的文件头信息
	operateLength = PopOperandStackShort();
	operateOffset = PopOperandStackShort();
	operateRef =  PopOperandStackShort();
	operateMode =  PopOperandStackShort();
	recordNumber =  PopOperandStackShort();
	curSFI =  PopOperandStackShort();
		
	operateAddr = GetValidArrayDataAddr(operateRef,operateOffset,operateLength);
	
	sfiIndex = CMCCExistedSFI(curSFI,fileHeadRef,0x00); 
    
    if(sfiIndex == 0x00)   //待更新的文件不存在
    {
		PushOperandStackShort(SW_FILE_NOT_FOUND);	
		return;
    }
    
	fileBodyAddr = GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef)); 
    
    fileBodyRef = VMReadShort(fileBodyAddr+(sfiIndex-1)*2);  //获取当前SFI文件所对应的文件体的引用
            
    fileBodyAddr =  GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));   //获取文件体的物理地址
	
	currentRecAddr = GetObjDataAddr(GetObjectHeaderAddr(currentRecRef));  //获取当前记录号信息
	
	currentRec = VMReadByte(currentRecAddr+(sfiIndex-1));  //读取该记录文件的当前记录信息
	
	fileHeadAddr = GetObjDataAddr(GetObjectHeaderAddr(fileHeadRef));  //获取存储文件头的数据体位置
	
	fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileType_Offset); 
	
	if(fileType == CMCC_File_Transparent)
	{
		PushOperandStackShort(SW_FileStruture_Error);	
		return ;
	}
	
	if((operateMode != CMCC_RecordMode_Next)&&(operateMode != CMCC_RecordMode_Previous)&&(operateMode != CMCC_RecordMode_CurAndAbsolute))
	{
		PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);
		return;
	}
	
	if((fileType&0x01) == 0x01)  //线性文件或循环文件
	{
		operateRecNo = CMCC_GetRecNo(fileHeadAddr,sfiIndex,currentRec,operateMode,recordNumber,action);
	
		if(operateRecNo == FALSE)
		{
			return;
		}
		actualLength = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordLength_Offset);
		
		if(operateLength != actualLength)
		{
			PushOperandStackShort(SW_FileStruture_Error);
			return;
		}
		
		if(action == CMCC_ReadAction)
		{		
			VMReadBuf(&tempBuf[0], fileBodyAddr+(operateRecNo-1)*actualLength, actualLength);  //读取记录内容
			
			VMWriteBuf(operateAddr, &tempBuf[0],actualLength);
		}
		else
		{
			VMReadBuf(&tempBuf[0], operateAddr,actualLength);  //读取记录内容
		
			VMWriteBuf(fileBodyAddr+(operateRecNo-1)*actualLength,  &tempBuf[0],actualLength);
		}
		
		if(operateMode != CMCC_RecordMode_CurAndAbsolute)
		{
			VMSetBuf(currentRecAddr+(sfiIndex-1),operateRecNo, 1);  //更新当前记录号,currentRecAddr在RAM 中
			
			if(action == CMCC_UpdateAction)
			{
				VMSetBuf(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset,operateRecNo, 1);  //更新记录指针
			}
		}
		
	
	}
	else  //可变记录文件
	{
		if(operateMode != CMCC_RecordMode_CurAndAbsolute)  //绝对模式/当前模式
		{  
			PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
			return ;
		}
		
		fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset);
		
		if(recordNumber == 0x00)
		{ 
			PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
			return;
		}
		if(action == CMCC_ReadAction)
		{
			if(recordNumber > fileType)   //读可变记录文件，只能读取当前已存在的记录
			{
				PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
				return;
			}
		}
		else  //更新可变记录文件时，可以更新当前已存在的记录号，也可以在可变记录文件后面进行新的记录号的追加,即append功能
		{
			if(recordNumber > (fileType+1))   //读可变记录文件，只能读取当前已存在的记录
			{
				PushOperandStackShort(SW_FUNC_NOT_SUPPORTED);	
				return;
			}
		}
		
		fileHeadRef = 0 ; //复用该变量，当作数据体的当前偏移使用
		
		VMReadBuf(&tempBuf[0],operateAddr, operateLength);  
		
		if(recordNumber <= fileType)  //在当前已存在的记录中操作
		{   
			if(action == CMCC_UpdateAction)  //更新可变长文件时，不允许新更新的记录tag与其他记录tag重复
			{	
				for(i=1; i<= fileType; i ++)
				{
					VMReadBuf(&tmp[0],fileBodyAddr + fileHeadRef , 0x02);  //读取tag L
					
					if((tmp[0] == tempBuf[0])&&(i != recordNumber))  //该记录中的tag已存在，但是记录号却不对
					{
						PushOperandStackShort(0x6985);
						return;
					}
					
					fileHeadRef += tmp[1]+2 ;
				}
			}
			
			fileHeadRef = 0 ; //复用该变量，当作数据体的当前偏移使用
			
			for(i=1; i <=fileType; i++ )
			{
				VMReadBuf(&tmp[0],fileBodyAddr + fileHeadRef , 0x02);  //读取tag L
				
				if(i == recordNumber)
				{
					if(operateLength != (tmp[1] +2))
					{
						if(action == CMCC_ReadAction)
						{ 
							PushOperandStackShort(SW_FileStruture_Error);
							return;
						}
						else
						{
							PushOperandStackShort(SW_CORRECT_LENGTH_00|(tmp[1]+2));
							return;
						}
					}
				  
					if(action == CMCC_ReadAction)
					{					
						VMReadBuf(&tempBuf[0],fileBodyAddr + fileHeadRef , operateLength);  
					
						VMWriteBuf(operateAddr, &tempBuf[0],operateLength);
					}
					else
					{
						VMReadBuf(&tempBuf[0],operateAddr, operateLength);  
					
						VMWriteBuf(fileBodyAddr + fileHeadRef , &tempBuf[0],operateLength);			
					}
					
					break;
				}
				
				fileHeadRef += tmp[1] +2;      
			} 
		}
		else  //如果 record大于当前已存在的记录总数，则表示当前在新增一条记录，类似append的功能
		{
			VMReadBuf(&tempBuf[0],operateAddr, operateLength);  
			
			if((tempBuf[1] +2)> 0xFF)  //因为API函数要求getRecordLength返回的长度为单字节，此处更新的时候就做控制
			{
				PushOperandStackShort(0x6985);
				return;
			}
			
			fileHeadRef = 0 ; //复用该变量，当作数据体的当前偏移使用
	
			for(i=1; i<= fileType; i ++)
			{
				VMReadBuf(&tmp[0],fileBodyAddr + fileHeadRef , 0x02);  //读取tag L
				
				if(tmp[0] == tempBuf[0])  //该记录中的tag已存在，但是记录号却不对
				{
					PushOperandStackShort(0x6985);
					return;
				}
				
				fileHeadRef += tmp[1]+2 ;
			}
			
			operateRef  = VMReadShort(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordNumber_Offset);  //复用operateRef用于缓存可变记录文件的总大小
	
			if((fileHeadRef+operateLength) >= operateRef)  //判断当前数据的长度是否可以更新到可变记录文件中去
			{
				PushOperandStackShort(0x6A84);
				return;
			}
				
			VMWriteBuf(fileBodyAddr+fileHeadRef,&tempBuf[0],tempBuf[1]+2);
				
			fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset);
			
			fileType ++;
			
			VMSetBuf(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset, fileType, 1);  //更新记录总数
					
		}
	}
	
	PushOperandStackShort(0x9000);
}
/*
*@function    : NM_CMCCreadRecord
*
*@descruotuib : Read record of record file without MAC check  For record record with MAC check, see verifyMAC in Security class.
*               Note: For the variable record, Tag, Length, Value of the TLV data should return by this API. 
*
*@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
*@param      :   bSFI  Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
*@param      :  bRecordNumber Record number	 
*@param bMode	 
*              '02' = next record;                                                                                          
*              '03' = previous record;                                                                                      
*              '04' = absolute mode/current mode, the record number is given in param bRecordNumber with bRecordNumber='00' denoting the current record. 
*@param   dataBuffer  to store the read data
@param    sOffset Data offset
*@param   sLength Record length to be read.
	 *            For variable record file, the Record Length includes Tag + Length + Value
*
* @return status word
        *            SW1 SW2 含 义
        *            ‘90’ ‘00’ 执行成功
        *            ‘69’ ‘81’ 命令与文件结构不相容
        *            ‘6A’ ‘82’ 未找到文件
        *            ‘6A’ ‘81’ 不支持此功能
        *            ‘6A’ ‘83’ 未找到记录
* @exception  NullPointerException     
*             if <code>buffer</code> is <code>null</code>
* @exception  ArrayIndexOutOfBoundsException <ul> 
*             if <code>offset</code> would cause access outside array bounds, or if <code>bLength</code> is negative.
*
*@author
*
*@history     :V1.0
     
*/
void NM_CMCCreadRecord(void)
{
	CMCC_OperateRecordPublic(CMCC_ReadAction);
}
/*
*@function    :NM_CMCCupdateRecord
*
*@description :Update record of record file without MAC check.  * For update record with MAC check, see verifyMAC in Security class.
*
*@param    :bSFI  *            Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
*@param     bRecordNumber         Record number	 
* @param      bMode
*           '02' = next record;                                                                                          
*           '03' = previous record;                                                                                      
*        '04' = absolute mode/current mode, the record number is given in aram bRecordNumber with bRecordNumber='00' denoting the current record.
* @param data Data in the record, For the variable record, the data includes the Tag ,Length, Value field
* @param sOffset: Data offset
* @param sLength: Data length	, For the variable record, the Length includes the Tag ,Length, Value field
*
* @return status word      
        *            SW1 SW2 含 义
	    *            ‘90’ ‘00’ 执行成功
        *            ‘69’ ‘81’ 命令与文件结构不相容
        *            ‘6A’ ‘81’ 不支持此功能
        *            ‘6A’ ‘82’ 未找到文件
        *            ‘6A’ ‘83’ 未找到记录
        *            ‘6C’ ‘XX’ 记录的长度不是当前变长记录的初始长度（XX为该记录初始设置的长度）?
*
*@author   :xuyz
*
*hisotry   :
  
*/
void NM_CMCCupdateRecord(void)
{
	CMCC_OperateRecordPublic(CMCC_UpdateAction);  //CMCC API/filesystem/6.4.9.6 测试在absolute模式下当可变记录为空时，更新可变记录文件
}
/*
*@function: NM_CMCCfindRecordByTag
*
*@description: This method can only be used for seeking the indicated TAG element from the beginning of* the variable record file. If the method is successful, return the record number,else return -1.
               * This search method is Comprehension Required flag independent.
*
 *@param      :  :Object FileInfo:存储当前管理的所有文件信息引用 
* @param bSFI    Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
* @param bTag   the tag of the TLV element to search
 *
 * @return record number of corresponding TAG: <ul>
 *      <li><code>-1</code> if the required TAG does not exist
* @exception CMCClibException.INPUT_DATA_INVALID - if File SFI is not existed.
*
*@author
*
*@history

*/
void NM_CMCCfindRecordByTag(void)
{
	WORD idata fileBodyRef,fileHeadRef,bSFI;
	BYTE idata sfiIndex;   //存储文件的位置索引
	UWORD xdata fileBodyAddr, fileHeadAddr;
	BYTE idata fileType,i,tmp[0x02],bTag;
	
	fileBodyRef = PopOperandStackShort();
	fileHeadRef = PopOperandStackShort();
	bTag =  PopOperandStackShort();
	bSFI =  PopOperandStackShort();
	
	sfiIndex = CMCCExistedSFI(bSFI,fileHeadRef,0x00); 
    
    if(sfiIndex == 0x00)   //待更新的文件不存在
    {	
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID); 	
    }
	
	fileHeadAddr = GetObjDataAddr(GetObjectHeaderAddr(fileHeadRef));  //获取存储数据头的数组地址
	
	fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileType_Offset);  //获取文件类型
	
	if(fileType != CMCC_File_VariableRecord)  //该API 函数仅支持可变记录文件操作
	{	
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID); 	
	}
	
	fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset); //读取可变记录文件中当前已存在的记录数
	
	fileBodyAddr = GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef)); 
    
    fileBodyRef = VMReadShort(fileBodyAddr+(sfiIndex-1)*2);  //获取当前SFI文件所对应的文件体的引用
            
    fileBodyAddr =  GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));   //获取文件体的物理地址
	
	fileHeadRef = 0 ; //复用该变量，当作数据体的当前偏移使用
	
	for(i=1; i<= fileType; i ++)
	{
		VMReadBuf(&tmp[0],fileBodyAddr + fileHeadRef , 0x02);  //读取tag L
		
		if(tmp[0] == bTag)  //找到tag
		{
			PushOperandStackShort(i);  //可变记录文件中指定的tag已找到
			return;
		}
		
		fileHeadRef += tmp[1]+2 ;
	}

	PushOperandStackShort(-1);  //可变记录文件中指定的tag未找到		
}
/*
*@function : NM_CMCCappendRecord
*
*@description :  This method is used to append contents to variable record file. 
                * This method can only be used to append record to the variable record file but not the cyclic record file. 
* 
* Note: For the variable record, if the TAG of appended record is same to the TAG of certain existed record, 
*       this method will return ISO7816.SW_CONDITIONS_NOT_SATISFIED.
* 
* @param :bSFI :  Value of sfi is 0x00--0x1F. low 5 bit is valid, namely sfi = (sfi&0x1F)
* @param :record  Record data. Tag + Length + Value, the range of Tag value is from 0x00 to 0xFE, can't be 0xFF.
* @param sOffset  Record data offset
* @param sLength  Length of the record data to be appended. This is the actual record length.
* @return status word
*            The following value is possible.
*            SW1 SW2 含 义
*            ‘90’ ‘00’ 执行成功
*            ‘69’ ‘81’ 命令与文件结构不相容
*            ‘69’ ‘85’ Tag 重复
*            ‘6A’ ‘82’ 未找到文件
*            ‘6A’ ‘84’ 文件中存储空间不够
* @exception  NullPointerException     
*             if <code>buffer</code> is <code>null</code>
* @exception ArrayIndexOutOfBoundsException <ul> 
*             if <code>offset</code> would cause access outside array bounds, or if <code>bLength</code> is negative.
* @exception CMCClibException.INPUT_DATA_INVALID  if Tag value is 0xFF.
*
*@author     :
*
*@history:
*/
void NM_CMCCappendRecord(void)
{
	WORD idata fileBodyRef,fileHeadRef,recordRef,bSFI;
	SWORD idata sLength,sOffset;
	UWORD xdata fileBodyAddr,recordAddr,fileHeadAddr;
	BYTE idata tmp[2],allRecord,sfiIndex,fileType,i;
	
	fileBodyRef = PopOperandStackShort();  //存储所管理的所有文件的文件体引用
	fileHeadRef = PopOperandStackShort();  //存储所管理的所有文件的文件头信息
	sLength =  PopOperandStackShort();     //待操作的数据长度
	sOffset =  PopOperandStackShort();     //待操作的记录偏移
	recordRef = PopOperandStackShort();    //待操作的记录引用
	bSFI = PopOperandStackShort();         //当前要append的文件的SFI 
	
	recordAddr = GetValidArrayDataAddr(recordRef, sOffset, sLength);
	
	VMReadBuf(&tempBuf[0],recordAddr,0x02);  //读取t ,L
	
	if(tempBuf[0] == 0xFF)  //TAG非法
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
	}
	
	if((tempBuf[1] +2)> 0xFF)  //因为API函数要求getRecordLength返回的长度为单字节，此处更新的时候就做控制
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);   
	}
	
	VMReadBuf(&tempBuf[2],recordAddr+2,tempBuf[1]);  //读取Value
	
	sfiIndex = CMCCExistedSFI(bSFI,fileHeadRef,0x00); 
    
    if(sfiIndex == 0x00)   //待更新的文件不存在
    {		
		PushOperandStackShort(0x6A82);
		return ;	
    }
	
	fileHeadAddr = GetObjDataAddr(GetObjectHeaderAddr(fileHeadRef));  //获取存储数据头的数组地址
	
	fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileType_Offset);  //获取文件类型
	
	if(fileType != CMCC_File_VariableRecord)  //该API 函数仅支持可变记录文件操作
	{	
		PushOperandStackShort(0x6981);
		
		return ;	
	}
	
	fileType = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset); //读取可变记录文件中当前已存在的记录数
	
	fileBodyAddr = GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef)); 
    
    fileBodyRef = VMReadShort(fileBodyAddr+(sfiIndex-1)*2);  //获取当前SFI文件所对应的文件体的引用
            
    fileBodyAddr =  GetObjDataAddr(GetObjectHeaderAddr(fileBodyRef));   //获取文件体的物理地址
	
	fileHeadRef = 0 ; //复用该变量，当作数据体的当前偏移使用
	
	for(i=1; i<= fileType; i ++)
	{
		VMReadBuf(&tmp[0],fileBodyAddr + fileHeadRef , 0x02);  //读取tag L
		
		if(tmp[0] == tempBuf[0])  //找到tag
		{
			PushOperandStackShort(0x6985);
			return;
		}
		
		fileHeadRef += tmp[1]+2 ;
	}	
	
	recordRef  = VMReadShort(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordNumber_Offset);  //复用recordRef用于缓存可变记录文件的总大小
	
	if((fileHeadRef+tempBuf[1]+2) >= recordRef)  //判断当前数据的长度是否可以更新到可变记录文件中去
	{
		PushOperandStackShort(0x6A84);
		return;
	}
		
	VMWriteBuf(fileBodyAddr+fileHeadRef,&tempBuf[0],tempBuf[1]+2);
		
	allRecord = VMReadByte(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset);
	
	allRecord ++;
	
	VMSetBuf(fileHeadAddr+(sfiIndex-1)*CMCC_FileHead_Len+ CMCC_FileRecordPointer_Offset, allRecord,1);  //更新记录总数，对于线性变长记录文件而言，只要记录总是没有被更新成功，那么之前更新的记录内容无任何意义，所以此处不采用事务机制进行数据的保护
	

	PushOperandStackShort(0x9000);

}
/*
*@function :CMCCLibThrowException:
*
*@description:该函数用于在C层抛出CMCC LIB 的异常
*
*@function    :short reason
*
*@auhtor      :xuyz
*
*@history     :
*/
void CMCCLibThrowException(short reason)
{
    ThrowError(CMCC_LIB_EXCEPTION_MASK|reason);
}
/*
* description： NM_CMCCCalculateMTAC篘M_CMCCVerifyMAC的公共函数 具体参数参考这两个函数
* para ：BYTE* buf  计算出来的mac值存入该数组  必须保证长度不越界 最大8字节
* para ：BYTE* MacLen   mac的长度  每种算法取的mac长度可能不一样
* return : UWORD addr   需要用来校验的mac地址 E2/mac最终需要存放的地址 E2
*/

UWORD CMCCCalculatePublic(BYTE* buf,BYTE* MacLen)
{
	WORD idata keyValueRef,keyBitLen,inDataBufRef,macBufRef,ivRef,bAlgoID;
	SWORD idata sInDataOff,sInDataLen,sMacOff,sExpectMacLgth,sIVoff;
	UWORD xdata keyValueAddr,inDataBufAddr,macBufAddr,ivAddr;
	BYTE  xdata ivData[0x08],tmpBuffer[0x08];
	BYTE idata isPad;
	
	sIVoff = PopOperandStackShort();     //初始向量的起始偏移
	ivRef =   PopOperandStackShort();    //初始向量的引用位置
	sExpectMacLgth = PopOperandStackShort();   //期望的mac或tac输出长度
	sMacOff = PopOperandStackShort();      //存放mac或tac的起始偏移
	macBufRef =  PopOperandStackShort();   //存放mac的引用位置
	sInDataLen = PopOperandStackShort();   //输入数据长度
	sInDataOff = PopOperandStackShort();   //输入数据的起始偏移
	inDataBufRef = PopOperandStackShort(); //存放输入数据的byte型数组处
	bAlgoID = PopOperandStackShort();        //算法类型
	keyBitLen =  PopOperandStackShort();     //key的bit长度
	keyValueRef = PopOperandStackShort();    //存放keyValue的byte型数组
	
	if((bAlgoID > 4)||(bAlgoID == 0))
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);    // CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
	}
	
	if((bAlgoID == 1) ||(bAlgoID == 4))  //3DES
	{
		if(keyBitLen != LENGTH_3DES_2KEY)
		{
			ThrowError(CryptoException_ILLEGAL_USE);
		}  
	}
	
	if((keyBitLen != LENGTH_DES)&&(keyBitLen != LENGTH_3DES_2KEY)&&(keyBitLen != LENGTH_3DES_3KEY))
	{
		ThrowError(CryptoException_ILLEGAL_USE);
	}
	
	inDataBufAddr = GetValidArrayDataAddr(inDataBufRef,sInDataOff,sInDataLen);   //获取输入数据的起始位置
	
	if(bAlgoID > 2)  //DES_CBC_NOPAD   3DES_CBC_NOPAD
	{
		if(((sInDataLen&0x07) != 0x00)||(sInDataLen == 0x00))   //不填充的模式下，输入数据长度必须为8的倍数
		{
			CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);    
		}	
		
		isPad = TRUE;  //不需要填充
	}
	else  //padding mode : ISO9797_1_M2
	{	
		isPad = FALSE;  //需要填充
	}
	
	keyValueAddr  = GetValidArrayDataAddr(keyValueRef,0,0);   //获取keyValue的起始位置
	
	macBufAddr = GetValidArrayDataAddr(macBufRef,sMacOff,sExpectMacLgth);   //获取存储mac的起始地址
	
	if(sExpectMacLgth > 8)  //如果期望的mac长度超过了实际参与运算的数据长度，则报错
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	ecpMem_Set(ivData, 0x00, sizeof(ivData));   //初始化初始向量
	
	if(ivRef != REF_NULL)
	{
		ivAddr = GetValidArrayDataAddr(ivRef,sIVoff,8);   //获取初始向量的起始地址
		
		VMReadBuf(ivData,ivAddr, 0x08);		
	}
	
	VMReadBuf(tempBuf,keyValueAddr,(keyBitLen/8)); //读取密钥值到tempBuf[0]的起始处
	
	inDataBufRef = 0x00;  //复用该变量，用作源数据的起始偏移
	
	while(1)
	{
		if(sInDataLen < 8)  //对于DES_CBC_NOPAD   3DES_CBC_NOPAD这两种情况下，会直接退出
		{
			if(isPad == TRUE)
			{
				break;  //最后一组数据执行完成 ，跳出循环
			}
			
			VMReadBuf(tmpBuffer, (inDataBufAddr + inDataBufRef), sInDataLen);
			
			SignaturePadding(tmpBuffer, sInDataLen,ALG_DES_MAC8_ISO9797_M2);

			isPad = TRUE;
			
			sInDataLen = 8;		
		}
		else
		{
			VMReadBuf(tmpBuffer, (inDataBufAddr + inDataBufRef), 8);
		}
		
		sInDataLen -= 0x08;
		
		inDataBufRef += 0x08; 
		
		if(bAlgoID == 1)  //ALG_DES_MAC8_ISO9797_1_M2_ALG3
		{
			if(isPad == TRUE)  //最后一组 执行 3DES  CBC_Init_Encry
			{
				TripleDES(tmpBuffer,tempBuf,ivData,CBC_Init_Encrypt,0x08,0x10);
			}
			else  //前 n组执行 DES CBC_Init_Encry
			{
				SingleDES(tmpBuffer,tempBuf,ivData,CBC_Init_Encrypt,0x08);
			}	
		}
		else
		{
			switch(keyBitLen)
			{
				case LENGTH_DES:
					SingleDES(tmpBuffer,tempBuf,ivData,CBC_Init_Encrypt,0x08);
					break;
				case LENGTH_3DES_2KEY:
					TripleDES(tmpBuffer,tempBuf,ivData,CBC_Init_Encrypt,0x08,0x10);
					break;
				case LENGTH_3DES_3KEY:
					TripleDES(tmpBuffer,tempBuf,ivData,CBC_Init_Encrypt,0x08,0x18);
					break;	
				default:
					break;
			}
		}
	}	
	
	ecpMem_Copy(buf,ivData,sExpectMacLgth);
	
	MacLen[0] = sExpectMacLgth;
	
	return macBufAddr;
}
/**
* Calculate MAC or TAC(can be used to calculate MAC and TAC，only the key is different) 
* with key indicated in the desKey and key algo in bAlgoID.
* 
* @param desKey
*            key for calculate MAC or TAC
* @param  keyLength
* @param bAlgoID
*            Algorithm ID for caculation.
*            - 1:  ALG: DES_CBC_ALG3,  Padding Mode:ISO9797_1_M2;
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_ISO9797_1_M2_ALG3;
*            - 2:  ALG: DES_CBC,, Padding Mode:ISO9797_1_M2 ; 
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_ISO9797_M2;
*            - 3:  ALG: DES_CBC_NOPAD.
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_NOPAD;
*            - 4:  ALG: 3DES_CBC_NOPAD
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_NOPAD;
*
*            Note:ISO9797_1_M2 means that the 0x80 will be padded at the end of input data first, then if data length is 8 times then 
*            no future padding, else pad data with 00 till the right length.
*            Padding: This padding is done by this method.
*
* @param inDataBuf   data buffer
* @param sInDataOff  data offset
* @param sInDataLen  data length
* @param macBuf      Mac buffer 
* @param sMacOff     Mac offset
* @param sExpectMacLgth  Length of the expected MAC or TAC output
*            If length of expected is less than 8 bytes, this method return the left part of the MAC with length equals sExpectMacLgth.
* @param IV  IV buffer
*            The default length of IV buffer is always 8 bytes  ;If IV is null, the default IV(8 bytes zero) will be used.
* @param sIVOff IV offset 
* 
*  @exception ArrayIndexOutOfBoundsException - if sInDataOff is negative or the sInDataOff array is too short 
*  @exception CryptoException.UNINITIALIZED_KEY if desKey instance is uninitialized.
*  @exception NullPointerException -if the inDataBuf parameter is null
*  @exception CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
*/
void NM_CMCCCalculateMTAC(void)
{
	UWORD xdata macBufAddr;
	BYTE idata MacLen;
	
	macBufAddr = CMCCCalculatePublic(tempBuf,&MacLen);
	
	VMWriteBuf(macBufAddr,tempBuf,MacLen);
	
}
/**
* Calculate session key with derivation parameter input, with key indicated in the desKey and key algo in bAlgoID.
* Currently the algo in this method is often 3DES.
* 
* @param desKey
*            original key for session key calculation
*@param  keyBitlength
* @param bAlgoID
*            Algorithm ID for caculation.
*            - 1: Algo: 3DES_ECB   Length of sessionKeyBuf is 16 bytes. Input Key is 3DES key. Lenght of Derivation ParamBuf is 8 bytes.
*                 The Derivation Data will be composed by two 8 bytes. The first 8 bytes is the input Derivation ParamBuf.
*                 The last 8 bytes will be reversed value of each bytes of input Derivation ParamBuf.
*            - 2: Algo: 3DES_ECB . Length of sessionKeyBuf is 8 bytes. Input Key is 3DES key. Lenght of Derivation ParamBuf is 8 bytes. 
*                 Derivation Data is equal to the value in Derivation ParamBuf
* @param derivationParamBuf
*            input derivation Parameter Data buffer
* @param sDerOff
*            distribute Parameter data offset
*            Lenght of Derivation ParamBuf is 8 bytes.(without padding)
* @param sessionKeyBuf
* 	          session key buffer
* @param sessionKeyOff
*            session key offset
*            Lenght of sessionKey depends on bAlgoID.
* 
* 
* @exception CryptoException with the following reason codes:<ul>
*           
*           <li><code>CryptoException.UNINITIALIZED_KEY</code> if key not
*               initialized.
*
*           </ul>
*  @exception ArrayIndexOutOfBoundsException - if sDerOff is negative or the derivationParamBuf array is too short 
*  @exception NullPointerException -if the derivationParamBuf parameter is null
*  @exception CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
*/
void NM_CMCCCalculateSessionKey(void)
{
	WORD idata sessionBufRef,derivationParamBufRef,keyBitLength,desKeyValue;
	SWORD idata sessionKeyOff,sDerOff;
	BYTE idata bAlgoID,i;
	UWORD xdata sessionKeyAddr,derivationAddr,keyValueAddr;
	
	sessionKeyOff = PopOperandStackShort();     //会话密钥存放位置的起始偏移
    
	sessionBufRef = PopOperandStackShort();     //会话密钥的数组引用
    
	sDerOff = PopOperandStackShort();           //derivation数据的起始偏移
    
	derivationParamBufRef = PopOperandStackShort();   //derivation数据的数组引用
    
	bAlgoID = PopOperandStackShort();        //算法标识
    
	keyBitLength = PopOperandStackShort();   //密钥bit长度
    
	desKeyValue =  PopOperandStackShort();   //存储密钥值的数组引用
	
	if((bAlgoID != 0x01)&&(bAlgoID != 0x02))
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	if(keyBitLength != LENGTH_3DES_2KEY)  //3DES算法，密钥必须是16字节
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	keyValueAddr = GetValidArrayDataAddr(desKeyValue,0, 0x10);
	
	VMReadBuf(&tempBuf[0x100], keyValueAddr , 0x10);  //读取16字节的密钥到缓存中
	
	derivationAddr = GetValidArrayDataAddr(derivationParamBufRef,sDerOff, 0x08);
	
	VMReadBuf(&tempBuf[0],derivationAddr, 0x08 );  //读取derivationData
	
	if(bAlgoID == 0x01)
	{
		for(i=0; i < 0x08 ; i++)
		{
			tempBuf[0x08+i] = tempBuf[i] ^ 0xFF;  //对于算法1来说 ，输入数据的后半部分为前半部分的取反值
		}
	}
	
	sessionKeyAddr = GetValidArrayDataAddr(sessionBufRef,sessionKeyOff, 0x10/bAlgoID);
	
	TripleDES(&tempBuf[0],&tempBuf[0x100],&tempBuf[0x20],ECB_Encrypt,0x10/bAlgoID,0x10);   //输出结果缓存在tempBuf[0x20]起始的位置处
	
	VMWriteBuf(sessionKeyAddr,&tempBuf[0x20],0x10/bAlgoID);	
}
/**
* Decrypt data with key indicated in the desKey and key algo in bAlgoID.
* Currently the algo in this method is often 3DES.
* The length of output clear data is times of 8 bytes including padding data. 
*
* 
* @param desKey
*            object for desKey
* @param keyBitlength
* @param bAlgoID
*            Algorithm ID for caculation.
*            - 1 AlgID: 3DES_ECB,  the length of desKey is 16-bytes. 
*            - 2 AlgID: DES_ECB,   the length of desKey is 8-bytes.  
* @param inDataBuf
*            buffer contains data to be decrypted
* @param sInDataOff
*            offset to the data
* @param sInDataLgth
* 	      length of input data
* @param outputBuf
*            clear data buffer
*            The default length of decrypted data is same as length of input data
* @param sOutputOff
* 	      out put data offset
* 
*
* @exception CryptoException with the following reason codes:<ul>
*            
*            <li><code>CryptoException.UNINITIALIZED_KEY</code> if key not
*                initialized.
*            <li><code>CryptoException.ILLEGAL_USE</code> If the input data is not 
*            (8-byte) block aligned it 
*
*            </ul>	 
* @exception ArrayIndexOutOfBoundsException - if sInDataOff is negative or the inDataBuf array is too short 
* @exception NullPointerException -if the inDataBuf parameter is null
* @exception CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
*/
void NM_CMCCDecryptData(void)
{
	WORD idata outputBufRef,inDataBufRef,keyBitLength,keyValueRef,tmpLength;
	SWORD idata sOutputOff,sInDataLgth,sInDataOff;
	BYTE idata bAlgoID,transActioinFlag;
	UWORD xdata inDataAddr,outDataAddr,keyValueAddr;
	
	sOutputOff = PopOperandStackShort();      //输出数据的起始偏移
    
	outputBufRef = PopOperandStackShort();    //输出数据的数组引用 
    
	sInDataLgth = PopOperandStackShort();     //输入数据长度
    
	sInDataOff = PopOperandStackShort();      //输入数据起始偏移
    
	inDataBufRef = PopOperandStackShort();    //输入数据引用
    
	bAlgoID = PopOperandStackShort();         //算法标识
    
	keyBitLength =  PopOperandStackShort();   //算法密钥bit长度
    
	keyValueRef = PopOperandStackShort();     //密钥值的数组引用 
	
	if((bAlgoID != 0x01)&&(bAlgoID != 0x02))
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	if(keyBitLength != (LENGTH_3DES_2KEY/bAlgoID))  //AlgID: 3DES_ECB,  the length of desKey is 16-bytes. 2 AlgID: DES_ECB,   the length of desKey is 8-bytes.  
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	inDataAddr = GetValidArrayDataAddr(inDataBufRef,sInDataOff, sInDataLgth);
	
	if((sInDataLgth&0x07) != 0x00)   //输入数据不是8的倍数
	{
		ThrowError(CryptoException_ILLEGAL_USE);	
	}
	
	outDataAddr = GetValidArrayDataAddr(outputBufRef,sOutputOff, sInDataLgth);  //输出数据中包括padding data 
	
	keyValueAddr =  GetValidArrayDataAddr(keyValueRef,0, 0x10/bAlgoID);   //获取密钥地址
	
	VMReadBuf(&tempBuf[0],keyValueAddr, 0x10/bAlgoID);  //8字节的密钥存储在tempBuf[0]-tempBuf[7]的位置，16字节的密钥存储在tempBuf[0]- tempbuf[15]的位置
	
	transActioinFlag = IF_BeginTransaction();
	
	for(tmpLength = 0; tmpLength < sInDataLgth; tmpLength += 0x08)
	{
		VMReadBuf(&tempBuf[0x20],inDataAddr+tmpLength, 0x08);
		
		if(bAlgoID == 0x01)
		{
			TripleDES(&tempBuf[0x20],&tempBuf[0x00],&tempBuf[0x40],ECB_Decrypt,0x08,0x10);  
		}
		else
		{
			SingleDES(&tempBuf[0x20],&tempBuf[0x00],&tempBuf[0x40],ECB_Decrypt,0x08);  
		}
		
		VMWriteBuf(outDataAddr+tmpLength,&tempBuf[0x40], 0x08);	
	}
 
	IF_CommitTransaction(transActioinFlag);
}
/**
* Encrypt data with key indicated in the desKey and key algo in bAlgoID, the input data is clear data without padding part. 
* Currently the algo in this method is often 3DES.
*
* Padding:  This padding is done by this method. Depend on bAlgoID, see description in bAlgoID.
*          
*
* @param desKey
*            object for desKey
* @param keyBitLength
* @param bAlgoID
*            Algorithm ID for caculation.
*            - 1 : 3DES_ECB,the length of desKey is 16-bytes. PaddingMode:CMCC Padding mode,i.e. If data length is 8 times then no padding, 
*                  else pad data with 80, then with 00 till the right length.
*            - 2 : DES_ECB,the length of desKey is 8-bytes. PaddingMode:ISO9797_1_M2
*            - 3 : DES_ECB_NOPAD,the length of desKey is 8-bytes. PaddingMode: No Padding.
*            - 4 : 3DES_ECB_NOPAD,the length of desKey is 16-bytes. PaddingMode:No Padding.
*
*            Note:ISO9797_1_M2 means that the 0x80 will be padded at the end of input data first, then if data length is 8 times then 
*            no future padding, else pad data with 00 till the right length.
*            Padding: This padding is done by this method.
* @param dataBuffer
*            buffer for clear data without padding.
* @param sDataOff
*            offset of clear data
* @param sDataLen
*            clear data length
* @param outputBuf
* 	      out put data buffer
* @param sOutputOff
* 	      out put data offset
* @return length of ciphered data
*
* @exception CryptoException with the following reason codes:<ul>

*            <li><code>CryptoException.UNINITIALIZED_KEY</code> if key not
*                initialized.
*            <li><code>CryptoException.ILLEGAL_USE</code> if this
*                <code>Cipher</code> algorithm does not pad the message and the
*                message is not block aligned.
*           </ul>
* @exception ArrayIndexOutOfBoundsException - if sDataOff is negative or the dataBuffer array is too short 
* @exception NullPointerException -if the dataBuffer parameter is null
* @exception CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
*/
void NM_CMCCEncryptData(void)
{
	SWORD idata sOutputOff,sDataLen,sDataOff;
	WORD idata outputBufRef,dataBufferRef,bAlgoID,keyBitLength,keyValueRef,actualOutLength;
	UWORD xdata keyValueAddr, outDataAddr,inDataAddr;
	BYTE idata isPad,transActioinFlag;
	BYTE xdata tmp[0x08];
	
	sOutputOff = PopOperandStackShort();  //输出数据缓冲区的起始偏移
    
	outputBufRef = PopOperandStackShort(); //输出数据引用
    
	sDataLen = PopOperandStackShort();     //输入数据长度
    
	sDataOff =  PopOperandStackShort();    //输入数据缓冲区的起始偏移
    
	dataBufferRef = PopOperandStackShort(); //输入数据的数组引用
    
	bAlgoID = PopOperandStackShort();  //算法标识
    
	keyBitLength = PopOperandStackShort();  //密钥长度
    
	keyValueRef = PopOperandStackShort();  //密钥数组引用
	
	if((bAlgoID > 4)||(bAlgoID == 0x00))  //CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
	{
		CMCCLibThrowException(CMCCLIB_INPUT_DATA_INVALID);
	}
	
	if((bAlgoID == 1)||(bAlgoID == 4))  //3DES_ECB 3DES_ECB_NOPAD
	{
		if(keyBitLength != LENGTH_3DES_2KEY)
		{
			ThrowError(CryptoException_ILLEGAL_USE);
		}
	}
	else
	{
		if(keyBitLength != LENGTH_DES) //DES_ECB  DES_ECB_NOPAD
		{
			ThrowError(CryptoException_ILLEGAL_USE);
		}		
	}
	
	inDataAddr = GetValidArrayDataAddr(dataBufferRef, sDataOff, sDataLen);
	
	if(bAlgoID > 2)  //3.4未非填充算法
	{
		if(((sDataLen&0x07) != 0x00)||(sDataLen == 0x00))  //输入数据长度不是8的倍数
		{
			ThrowError(CryptoException_ILLEGAL_USE);
		}
		actualOutLength = sDataLen;
		
		isPad = TRUE;
	}
	else  //填充算法
	{
		if(bAlgoID == 1) // 1 : 3DES_ECB,the length of desKey is 16-bytes. PaddingMode:CMCC Padding mode,i.e. If data length is 8 times then no padding,  else pad data with 80, then with 00 till the right length.
		{
			dataBufferRef = sDataLen;  //复用变量dataBufferRef ，当作中间变量借用一下
		}
		else //Note:ISO9797_1_M2 means that the 0x80 will be padded at the end of input data first, then if data length is 8 times then   no future padding, else pad data with 00 till the right length.
		{
			dataBufferRef = sDataLen+1;
		}
		
		if((dataBufferRef&0x07) == 0x00)  
		{
			actualOutLength = dataBufferRef;
		}
		else
		{
			actualOutLength = dataBufferRef + 8-(dataBufferRef&0x07);
		}
		
		isPad = FALSE;
		
	}
	
	outDataAddr = GetValidArrayDataAddr(outputBufRef, sOutputOff, actualOutLength);
	
	keyValueAddr = GetValidArrayDataAddr(keyValueRef, 0 ,keyBitLength/8);
	
	VMReadBuf(&tempBuf[0], keyValueAddr, keyBitLength/8);  //读取密钥值在tempBuf[0] -tempBuf[0x10]中
	
	transActioinFlag = FALSE;
	
	if((TransactionGetDepth()  == FALSE)&&(IsEepromAddr(outDataAddr)))	 // TODO:该处若采用事物写，则需要判断当前是否已开启了事物，如果没有开启，则开启一个新的事物
	{
		beginTransaction(0);
		
		transActioinFlag = TRUE;
	}
    
	while(1)
	{
		if(sDataLen < 8)
		{
			if(isPad == TRUE)  //对于非填充算法，都执行完后则直接退出
			{
				break;
			}
			
			VMReadBuf(tmp, inDataAddr , sDataLen);
			
			SignaturePadding(tmp, sDataLen,ALG_DES_MAC8_ISO9797_M2);
			
			isPad = TRUE;
			
			sDataLen = 8;				
		}
		else
		{
			VMReadBuf(tmp, inDataAddr, 0x08);
		}
		
		sDataLen -= 0x08;
		
		if(bAlgoID == 0x01)  // // 3DES_ECB_CMCCPAD
		{
			TripleDES(tmp,&tempBuf[0x00],&tempBuf[0x20],ECB_Decrypt,0x08,0x10); 
			
			if(sDataLen == 0)  //如果已经是8的倍数，则不再填充
			{
				isPad = TRUE;
			}
		}
		else if(bAlgoID == 0x02)  //DES_ECB,the length of desKey is 8-bytes. PaddingMode:ISO9797_1_M2
		{
			SingleDES(tmp,&tempBuf[0x00],&tempBuf[0x20],ECB_Encrypt,0x08);  
		}
		else if(bAlgoID == 0x03)  //DES_ECB_NOPAD,
		{
			SingleDES(tmp,&tempBuf[0x00],&tempBuf[0x20],ECB_Encrypt,0x08);  
		}
		else  // 3DES_ECB_NOPAD,
		{
			TripleDES(tmp,&tempBuf[0x00],&tempBuf[0x20],ECB_Decrypt,0x08,0x10);  
		}
		
		VMWriteBuf(outDataAddr,&tempBuf[0x20], 0x08);	
		
		inDataAddr += 8;
		
		outDataAddr += 8;			
	}
	
	IF_CommitTransaction(transActioinFlag);
	
	PushOperandStackShort(actualOutLength);
}
/**
* Generate random data.
* 
* @param sExpectRandomLgth
*            The expected length of random data. Max is 8 bytes.
* @param destBuf
*            the destination buffer to save random data
* @param sDataOff
*            the offset for the random data in the destination buffer
* 
* 
* @exception CryptoException with the following reason codes:<ul>
*            <li><code>CryptoException.ILLEGAL_VALUE </code> if the sExpectRandomLgth parameter is zero.
*
*            </ul>
* @exception ArrayIndexOutOfBoundsException - if sDataOff is negative or the dataBuffer array is too short 
* @exception NullPointerException -if the destBuf parameter is null
* 
*/
void NM_CMCCGetRandom(void)
{
	SWORD idata sDataOff;
	WORD idata destBufRef,sExpectRandomLgth;
	UWORD xdata destBufAddr;
	
	sDataOff = PopOperandStackShort();
	destBufRef =  PopOperandStackShort();
	sExpectRandomLgth = PopOperandStackShort();
	
	if((sExpectRandomLgth > 8)||(sExpectRandomLgth <= 0x00))
	{
		ThrowError(CryptoException_ILLEGAL_USE);
	}
	
	destBufAddr = GetValidArrayDataAddr(destBufRef, sDataOff, sExpectRandomLgth);
	
	Random_Number_Generator_Get(&tempBuf[0],sExpectRandomLgth);
	
	VMWriteBuf(destBufAddr,tempBuf,sExpectRandomLgth );
}
/**
* Verify the MAC with key indicated in the desKey and key algo in bAlgoID.
* Currently the algo in this method is often 3DES.
*
*	 
* 
* @param desKey
*            object for desKey
* @param  keyBitLength
* @param bAlgoID
*            Algorithm ID for caculation.
*            - 1:  ALG: DES_CBC_ALG3, MAC:4bytes, Padding Mode:ISO9797_1_M2;
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_ISO9797_1_M2_ALG3;
*            - 2:  ALG: DES_CBC, MAC:4bytes,, Padding Mode:ISO9797_1_M2 ; 
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_ISO9797_M2;
*            - 3:  ALG: DES_CBC_NOPAD.
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_NOPAD;
*            - 4:  ALG: 3DES_CBC_NOPAD
*                  i.e. the definition of Algorithm is the same as the javacard.security.Signature.ALG_DES_MAC8_NOPAD;
*
*            Note: ISO9797_1_M2 means that the 0x80 will be padded at the end of input data first, then if data length is 8 times then 
*            no future padding, else pad data with 00 till the right length.
*            Padding: This padding is done by this method.
* @param dataBuf
*            data buffer for MAC calculation without MAC part.
* @param sDataOff
*            offset to the data
* @param sDataLgth
*            length of Data
* @param macBuf
* 	      MAC buffer 
* @param sMacOff
*            offset to MAC(to be verified)
* @param sMacLgth
*            length of MAC(to be verified) 
* @param IV
* 	      IV buffer
*            The default length of IV buffer is always 8 bytes
*            If IV is null, the default IV(8 bytes zero) will be used.
* @param sIVOff
* 	      IV offset
* @return true if MAC is right, otherwise false
* 
*  @exception ArrayIndexOutOfBoundsException - if sDataOff is negative or the dataBuf array is too short 
*  @exception NullPointerException -if the inDataBuf parameter is null.
*  @exception CryptoException with the following reason codes:<ul>
*            <li>CryptoException.UNINITIALIZED_KEY if desKey instance is uninitialized.
*  
*  	      <li>CryptoException.ILLEGAL_USE  if if one of the following conditions is met:
*            <ul>
*            <li>  if this Signature algorithm does not pad the message 
*                and the message is not block aligned. 
*            
*            </ul>
*            </ul>
*  @exception CMCClibException.INPUT_DATA_INVALID - if bAlgoID is out of scope.
*/
void NM_CMCCVerify(void)
{
	UWORD xdata macBufAddr;
	BYTE idata sExpectMacLgth;
	
	macBufAddr = CMCCCalculatePublic(tempBuf,&sExpectMacLgth);
	
	VMReadBuf(&tempBuf[10], macBufAddr, sExpectMacLgth);
	
	if(ecpMem_Compare(&tempBuf[10],tempBuf,sExpectMacLgth) == EQUAL)
	{
		PushOperandStackShort(TRUE);
	}
	else
	{
		PushOperandStackShort(FALSE);
	}	
}
/**
* Get MSISDN stored in the current SE or SIM card.
*
* <p>Notes:<ul>
* <li><em>The default value of MSISDN is filled with 0xFF.</em>
* </ul>
*
* @param outArray     output data array to store the MSISDN.
*                     The format of MSISDN is TON and NPI(1 byte) + Dialling Number/SSC String(10 bytes), which is same as in EF MSISDN or EF ADN in 3GPP Spec.
* @param outOffset    output data offset.  
*
* @exception java.lang.NullPointerException if either outArray is null.
* @exception java.lang.ArrayIndexOutOfBoundsException if data in outArray is outside array bounds.     
*
* @return the length of the MSISDN data.
*
* Note: The default value of MSISDN is filled with 0xFF.
*
*/
void NM_CMCCGetMSISDN(void)
{
	WORD idata outArrayRef;
	SWORD idata outOffset;
	UWORD xdata outArrayAddr,fileAddr;
	BYTE  idata recordLength;
	
	outOffset = PopOperandStackShort();
    
	outArrayRef =  PopOperandStackShort();
	
	outArrayAddr = GetValidArrayDataAddr(outArrayRef, outOffset, 0x0B);
	
	fileAddr = SelectLFFile(0x7F10, 0x6F40);
	
	if(fileAddr != FALSE)
	{
		recordLength = GetLFRecNumLgth(fileAddr,FileLgth);
		
		ReadLFBuf(fileAddr,0x01,recordLength-13,&tempBuf[0], 0x0B);
		
		VMWriteBuf(outArrayAddr,&tempBuf[0], 0x0B);	
		
		PushOperandStackShort(0x0B);
	}
	else
	{
		PushOperandStackShort(FALSE);
	}
}


UWORD SearchAPIAllocateAddr(BYTE mode, BYTE * aidBuf)
{
	BYTE idata i,aidLen;
	BYTE idata temp[0x10];
	
	for(i=0;i<0x14;i++)
	{
		VMReadBuf(&aidLen,APIALLOCATEADDR + i*0x14+1,0x01);
		
		if(aidLen == (BYTE)0xFF)
		{
			if(mode == EMPTYMODE)
			{
				return APIALLOCATEADDR + i*0x14+1;
			}
			else
			{
				continue;
			}
		}
		else
		{
			VMReadBuf(&temp[0],APIALLOCATEADDR + i*0x14+2,aidLen);
			
			if((aidLen == aidBuf[0]) && (ecpMem_Compare(&temp[0],&aidBuf[1],aidLen) == EQUAL))
			{
				return APIALLOCATEADDR + i*0x14+1;
			}
		}
						
	}
	
	return ADDR_IS_NULL;
	
}

void NM_CMCCAllocate(void) 
{
	WORD idata aidLen;	
	BYTE xdata aidBuf[0x11];
	BYTE xdata num,appId,transActioinFlag;
	WORD idata AddrRef;
	UWORD xdata addr,allocAddr;
	SBYTE idata result;

	//previousContext = GetPreviousStackContext(); //获取前一个上下文
	
    //appId = ChannelManagement[G_CurSelectedChannel].context&0xFF;  //获取APPID
	appId = G_CurContext&0xff;
	
	aidLen = GetAIDDataByAppID(appId,&aidBuf[1]);
	
	aidBuf[0] = aidLen;
	
	if(aidLen > 0x10)
	{
		result = -1;
		PushOperandStackShort(result);
		return;
	}
	result = 0x00;
	//VMReadBuf(&num,APIAllocate_Ref,0x01);
	
	//entryAddr = GetAddrByIndexAddr(APIAllocate_Ref);
	
	VMReadBuf(&num,APIALLOCATEADDR,0x01);
	
	addr = SearchAPIAllocateAddr(ALLOCATEDMODE, aidBuf);
	
	transActioinFlag = IF_BeginTransaction();
	
	if(addr == ADDR_IS_NULL)
	{
		if((num == 0xFF) || (num<20))
		{
			allocAddr =  VMCOSHeapAlloc(0x10, (WORD*)&AddrRef, CardMemory_Mode);
			
			if(allocAddr == ADDR_IS_NULL)
			{					
				IF_AbortTransaction(transActioinFlag);
				
				result = -1;
			}						
			
			addr = SearchAPIAllocateAddr(EMPTYMODE, aidBuf);
			
			VMWriteBuf(addr, &aidBuf[0], aidLen+1);
			
			VMWriteBuf(addr+aidLen+1, (BYTE*)&AddrRef, 0x02);
			
			if(num == 0xFF)
			{
				num =0x00;
			}
			
			num++;
			
			//VMReadBuf(&num,APIALLOCATEADDR,0x01);
			
			VMWriteBuf(APIALLOCATEADDR, (BYTE*)&num, 0x01);
			
			result = 0x00;						
		}
		else
		{
			result = -1;
		}
	}
	else
	{
		result = 0x00;
	}

	IF_CommitTransaction(transActioinFlag);
		
	PushOperandStackShort(result);	
}

UWORD CMCCGetValidArrayDataAddr(WORD arrayRef, SWORD soffset,SWORD  sLength)
{
	UWORD xdata operateAddr;
	
	if(REF_NULL == arrayRef)
	{
		return  0xFFFF;
	}
	
	operateAddr = GetObjectHeaderAddr(arrayRef);
	
	if((soffset < 0)||(sLength < 0)||(((soffset+sLength) > VMReadShortCode(operateAddr + ObjLen_Offset))))  //从对象头中读取该数组对象的长度,如果index大于该数组对象的长度，抛异常	EEPROM_BASE
	{
		return  0xFFFF;			//Signexc01204 当长度 为0 时，do nothing ,按照ArrayIndexOutOfBoundsException描述，index不能为负数，不能大于等于长度，此处需确认。涉及的函数修改 _aastore   _aload_sub
	}

    //CheckArrayAddrAndBound(operateAddr, soffset, sLength);
    
    operateAddr = GetObjDataAddr(operateAddr) + soffset;  

	return operateAddr;
}





void CMCCSetOrGetData(BYTE mode)
{
	WORD idata aidLen;	
	BYTE xdata aidBuf[0x11],dataBuf[0x10];
	BYTE xdata appId,len,length;
	WORD idata AddrRef;
	UWORD idata addr,dataAddr,AllocateAddr;
	WORD idata dataRef,dataOff,dataLen;
	SBYTE idata result;
	
	dataLen =  PopOperandStackShort();     //数据的长度
	dataOff =  PopOperandStackShort();     // buf的偏移
	dataRef = PopOperandStackShort();    //buf的引用
	
	
	dataAddr = CMCCGetValidArrayDataAddr(dataRef, dataOff, dataLen);//获取src数组的实际地址
	
	if(dataAddr == 0xFFFF)
	{
		result = -1;
		
		PushOperandStackShort(result);
		
		return;
	}
			
	//previousContext = GetPreviousStackContext(); //获取前一个上下文
	
	appId = G_CurContext&0xff;
	
	//appId = ChannelManagement[G_CurSelectedChannel].context&0xFF;  //获取APPID
	
	aidLen = GetAIDDataByAppID(appId,&aidBuf[1]);
	
	aidBuf[0] = aidLen;
	
	
	addr = SearchAPIAllocateAddr(ALLOCATEDMODE, aidBuf);
	
	if(addr == ADDR_IS_NULL)
	{
		result = -1;
	}
	else
	{
		if(dataLen > 0x10)//数据长度不能超过16字节，因为分配时只分配了16字节
		{
			result = -1;
		}
		else
		{
			length =(BYTE) (dataLen&0xFF);
			
			VMReadBuf((BYTE *)&AddrRef,addr+1+aidLen,0x02);
			
			AllocateAddr = GetAddrByTableIndex(AddrRef);		
			
			if(mode == GETDATAMODE)
			{
				VMReadBuf(&len, addr+1+aidLen+0x02, 0x01);
								
				
				if((len == 0xFF) || (length > len))
				{
					result = -1;
				}
				else
				{
					VMReadBuf(&dataBuf[0], AllocateAddr, dataLen);//数据长度不能超过16字节，因为分配时只分配了16字?

					VMWriteBuf(dataAddr, &dataBuf[0], dataLen);
					
					result = dataLen;
				}							

			}
			else
			{
				VMReadBuf(&dataBuf[0], dataAddr, dataLen);
				
				VMWriteBuf(AllocateAddr, &dataBuf[0], dataLen);//数据长度不能超过16字节，因为分配时只分配了16字节
				
				VMWriteBuf(addr+1+aidLen+0x02, (BYTE *)&length, 0x01);//数据长度不能超过16字节，因为分配时只分配了16字节
				
				result = dataLen;
			}				
		}
		
		
				
	}
	
	PushOperandStackShort(result);
	
}

void NM_CMCCSetData(void) 
{
	CMCCSetOrGetData(SETDATAMODE);
		
}

void NM_CMCCGetData(void) 
{
	CMCCSetOrGetData(GETDATAMODE);		
}

void NM_CMCCRelease(void) 
{
	WORD idata aidLen;	
	BYTE xdata aidBuf[0x11];
	BYTE xdata num,appId,transActioinFlag;
	WORD idata AddrRef;
	UWORD idata addr;
	
	//previousContext = GetPreviousStackContext(); //获取前一个上下文
	
	//appId = ChannelManagement[G_CurSelectedChannel].context&0xFF;  //获取APPID
	appId = G_CurContext&0xff;
	
	aidLen = GetAIDDataByAppID(appId,&aidBuf[1]);
	
	aidBuf[0] = aidLen;
	
	addr = SearchAPIAllocateAddr(ALLOCATEDMODE, aidBuf);
	
	VMReadBuf(&num,APIALLOCATEADDR,0x01);
	
	transActioinFlag = FALSE;
	
	if(addr == ADDR_IS_NULL)
	{
		return;
	}
	else
	{
		VMReadBuf((BYTE*)&AddrRef,addr+1+aidLen,0x02);
				
		transActioinFlag = IF_BeginTransaction();
		
		VMCOSHeapFree(AddrRef, 0x10,CardMemory_Mode);
		
		//AddrRef = REF_NULL;
		
		VMSetBuf(addr, 0xFF, 0x14);	
		
		num--;

		VMSetBuf(APIALLOCATEADDR,num,0x01);	
		
		IF_CommitTransaction(transActioinFlag);
	}
	
}
/**
  检查应用是否是已经设置好的应用
*
*                     
*/
BYTE IsDesignateAPP(void)
{
	WORD idata aidLen;	
	BYTE xdata aidBuf[0x10];
	BYTE xdata appId;
	
	appId = G_CurContext&0xff;
	
	aidLen = GetAIDDataByAppID(appId,&aidBuf[0]);
	
	VMReadBuf(&tempBuf[0],NFCRECORD_APP,0x01);
	
	VMReadBuf(&tempBuf[1],NFCRECORD_APP+0x01,tempBuf[0]);
	
	if((aidLen == tempBuf[0]) && (ecpMem_Compare(&tempBuf[1],&aidBuf[0],aidLen)==0))
	{
		return TRUE;
	}		
	else
	{
		return FALSE;
	}
	
}

/**
  设置NFC刷卡记录功能的开启/关闭状态
*
* @param OnOff    值为0x00：关闭，值为0x01：开启
*                     
*/

void NM_CMCCSetNFCRecordState(void)
{
	SBYTE idata OnOff;
		
	OnOff = PopOperandStackShort();
			
	if(IsDesignateAPP() == FALSE)
	{
		ThrowError(SYSTEMEXCEPT_ILLEGAL_USE);
	}
	
	if((OnOff != ON)&&(OnOff != OFF))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	VMWriteBuf(NFCRECORD_ONOFF, (BYTE *)&OnOff, 0x01);			
}
/**
  获取NFC刷卡记录功能的开启/关闭状态
*                   
*/
void NM_CMCCGetNFCRecordState(void)
{
	BYTE idata OnOff;
	
	if(IsDesignateAPP() == FALSE)
	{
		ThrowError(SYSTEMEXCEPT_ILLEGAL_USE);
	}
	
	VMReadBuf((BYTE *)&OnOff, NFCRECORD_ONOFF, 0x01);
	
	if(OnOff != 0x01)
	{
		OnOff = 0x00;
	}

	PushOperandStackShort(OnOff);
}

/**
  获取NFC刷卡记录信息
*
* @param dataAddr     输出数据的数组.
*                     
* @param dataOff    待输出数据所在数组的偏移  
*
*/
void NM_CMCCGetNFCRecord(void)
{
	WORD idata dataRef,dataOff;
	UWORD xdata dataAddr;
	BYTE idata OnOff;
		
	dataOff =  PopOperandStackShort();     // buf的偏移
	dataRef = PopOperandStackShort();    //buf的引用
		
	//dataAddr = CMCCGetValidArrayDataAddr(dataRef, dataOff, dataLen);//获取src数组的实际地址
	if(IsDesignateAPP() == FALSE)
	{
		ThrowError(SYSTEMEXCEPT_ILLEGAL_USE);
	}
	
	ecpMem_Set(tempBuf,0x00,sizeof(tempBuf));
	
	VMReadBuf((BYTE *)&OnOff, NFCRECORD_ONOFF, 0x01);
	
	if(OnOff == OFF)
	{
		PushOperandStackShort(0x00);
		return;
	}
	
	
	VMReadBuf(&tempBuf[0], NFCRECORD_AID, 0x01);
	
	if((tempBuf[0] == 0x00) || (tempBuf[0] == 0xFF))
	{
		tempBuf[0] = 0x00;
	}
	else
	{
		VMReadBuf(&tempBuf[1], NFCRECORD_AID+1, tempBuf[0]);
		
		dataAddr = CMCCGetValidArrayDataAddr(dataRef, dataOff, tempBuf[0]);//获取src数组的实际地址
	
		VMWriteBuf(dataAddr, &tempBuf[1], tempBuf[0]);
		
		VMSetBuf(NFCRECORD_AID,0x00,0x11);//获取数据之后需要清空数据
	}	
	
	PushOperandStackShort(tempBuf[0]);
}

#endif

#if  UICC__FUNCTION_CMCCAPI


/**
  终端在网状态查询
*
*return ：0x00：State_Function_Not_Supported
*		  0x01: State_Offline                 
*     	  0x02:	State_Online
*
*/
void NM_CMCCGetOnlineState(void) 
{
	BYTE xdata result;

	if(IF_IoUart_isActived() == 0x00)//关机刷卡 // modify by xuxiaojun@20220318 分层
	{
		RAM_Function_Supported = 0x01;
		
		result = State_Offline;
	}
	else
	{
		if(RAM_Function_Supported == 0x00)
		{
			result = State_Function_Not_Supported;
		}
		else
		{
			if(RAM_Online_Flag == 0x00)
			{
				result = State_Offline;
			}
			else
			{
				result = State_Online;
			}
		}
	}		
		
	PushOperandStackShort(result);	
}

void CMCCGetM1RecordPublic(BYTE isM1)
{
	BYTE xdata temp[0x11];
	WORD idata offset,arraryRef;
	UWORD xdata arrayAddr;
	BYTE idata haveRecord;

	temp[0]  = 0;
	offset  = PopOperandStackShort();

	arraryRef  = PopOperandStackRef();
#if BOOT__IO__SWP
#if UICC__FUNCTION__RECORD_NFC_NEW
	haveRecord = isM1?G_M1ShuaKa:G_UIDShuaKa;
#else
	haveRecord = isM1?SWP_M1Counter4CM():G_UIDShuaKa;
#endif	
	if(haveRecord != 0)
	{
		G_UIDShuaKa = FALSE;
#if UICC__FUNCTION__RECORD_NFC_NEW
		G_M1ShuaKa = FALSE;
		G_CPUShuaKa = FALSE;
#endif		
		if(arraryRef == REF_NULL)//浙江需求只返回1字节A5/00
		{
			PushOperandStackShort(0xA5);
			return;
		}
		temp[0] = 16;
		ecpMem_Set(&temp[1],0xFF,12);
		
		if(!isM1)
		{
			temp[12] = 0x00;         //若离场时未选择CPU应用或M1应用，则记录11字节FF+1字节00+4字节SIM卡当前UID信息。
		}
		
		//ecpMem_Copy(&temp[13],&aReg_CardRFTypeA_UID[1],4);
    ecpMem_Copy(&temp[13],&g_TypeAProtocolParam.UID[1],4);
		arrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(arraryRef)))  + offset;//获取数组对象地址
		VMWriteBuf(arrayAddr, &temp[1], temp[0]);
	}
#endif
	PushOperandStackShort(temp[0]);
}
/**
  查询M1刷卡记录
*
* added by liguiyu@20220113
*/
void NM_CMCCGetM1Record(void)
{
	CMCCGetM1RecordPublic(TRUE);
}

void NM_CMCCGetUIDRecord(void)
{
	CMCCGetM1RecordPublic(FALSE);
}

/**
 clear M1刷卡标记
*
* added by xwh@20220129
*/
void NM_CMCCClearM1Tag(void)
{
#if BOOT__IO__SWP

#if UICC__FUNCTION__RECORD_NFC_NEW	
	G_M1ShuaKa = FALSE;
#endif	
	G_UIDShuaKa = FALSE;
#endif	
}


/**
 clear M1刷卡记录
*
* added by xwh@20220129
*/
void NM_CMCCClearM1Record(void)
{
#if BOOT__IO__SWP
	//SWP_M1Counter4CM();20230202not found fuction impl
#if UICC__FUNCTION__RECORD_NFC_NEW	
	G_M1ShuaKa = FALSE;
#endif	
	G_UIDShuaKa = FALSE;
#endif	
}

/*
 * 设置NFC刷卡的状态
 * added by yjr@20220424
*/
void NM_CMCCSetNFCRecordState(void)
{
	BYTE xdata onOff,tmp;

	onOff = popShort();

	tmp = JcvmConfig_GetNFCRecord();

	if(onOff == 0x01)
	{
		SetBit(tmp,Bit_NFCRecordState);
	}
	else
	{
		ClearBit(tmp,Bit_NFCRecordState);
	}

	JcvmConfig_SetNFCRecord(tmp);//bit1用来表示NFC的刷卡状态，值为1表示开启，值为0表示关闭

}
/*
 * 获取NFC刷卡的状态
 * added by yjr@20220424
 */
void NM_CMCCGetNFCRecordState(void)
{
	BYTE xdata onOff;

	onOff = JcvmConfig_GetNFCRecord();//bit1用来表示NFC的刷卡状态，值为1表示开启，值为0表示关闭

	PushOperandStackShort((onOff&0x01));
}
/*
 * 获取NFC刷卡的状态
 * added by yjr@20220424
 */
void NM_CMCCGetCPUShuakaRecord(void)
{
	#if UICC__FUNCTION__RECORD_NFC_NEW	
	PushOperandStackShort(G_CPUShuaKa);
    #else
	PushOperandStackShort(0);
	#endif
}

/*
 * 获取NFC刷卡的状态
 * added by yjr@20220424
 */
void NM_CMCCSetCPUShuakaRecord(void)
{
	#if UICC__FUNCTION__RECORD_NFC_NEW	
	G_CPUShuaKa = TRUE;
	#endif
}
/*
 * 获取NFC刷卡的应用是否指定AID
 * added by yjr@20220424
 */
void NM_IsDesignateAPP(void)
{
	BYTE xdata temp;

	temp = JcvmConfig_GetNFCRecord();//bit2用来表示NFC刷卡应用是否需要指定AID，值为0表示指定，值为1表示不需要

	PushOperandStackShort((temp&0x02));

}


#define CIPERMODE 0x00
#define SIGNMODE 0x01

#define IMPI_LEN  49
#define IMSI_Len  15
#define MNC_Off   23
#define MCC_Off   30
/*234101943787656 @ims.mnc010.mcc234.3gppnetwork.org*/
BYTE imsiToIMPI(BYTE* outBuf)
{
	UWORD xdata fileAddr;
	BYTE idata i,mncLen;
	BYTE xdata impiBuf[49] = {0x32,0x33,0x34,0x31,0x30,0x31,0x39,0x34,0x33,0x37,0x38,0x37,0x36,0x35,0x36,0x40,0x69,0x6D,0x73,0x2E,0x6D,0x6E,0x63,0x30,0x31,0x30,0x2E,0x6D,0x63,0x63,0x32,0x33,0x34,0x2E,0x33,0x67,0x70,0x70,0x6E,0x65,0x74,0x77,0x6F,0x72,0x6B,0x2E,0x6F,0x72,0x67};
	BYTE xdata imsiBuf[8];
	BYTE xdata imsiAsciiBuf[16];

	fileAddr = SelectTFFile(DF_PHASE,EF_IMSI);//

	if(fileAddr == FALSE)
	{
		return 0;
	}
    ReadTFBuf(fileAddr,1,imsiBuf,8);

	//IMSI:08 2943019134876765
	//->bcd->ASCII:39 323334 313031393433373837363536
	//mcc:323334   mnc:	3130


    for(i=0;i<8;i++)//
	{
		imsiAsciiBuf[i*2] = (imsiBuf[i]&0x0F)+0x30;//
		imsiAsciiBuf[i*2+1] = ((imsiBuf[i]>>4)&0x0F)+0x30;//
	}


	fileAddr = SelectTFFile(G_DF_ID_USIM,0x6FAD);//
	if(fileAddr == FALSE)
	{
		return 0;
	}
	ReadTFBuf(fileAddr,0x03,&mncLen,1);//length of  MNC in the IMSI

	ecpMem_Copy(impiBuf,&imsiAsciiBuf[1],15);//imsi
	if(mncLen == 2)
	{
		impiBuf[MNC_Off] = 0x30;
		ecpMem_Copy(impiBuf+MNC_Off+1,&imsiAsciiBuf[4],2);//mnc

	}
	else
	{
		ecpMem_Copy(impiBuf+MNC_Off,&imsiAsciiBuf[4],3);//mnc
	}

	ecpMem_Copy(impiBuf+MCC_Off,&imsiAsciiBuf[1],3);//mcc

	ecpMem_Copy(outBuf,impiBuf,IMPI_LEN);

	return IMPI_LEN;
}

BYTE generateME_KsNAF(BYTE* nafId,BYTE nafIdLen,BYTE *outBuf)
{
	UWORD xdata fileAddrbp,fileAuthMode;
	WORD  idata pos;
	BYTE idata impiLen;
	BYTE xdata gbame[] = {0x67,0x62,0x61,0x2D,0x6D,0x65};	//gba-me

	fileAddrbp = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);

	fileAuthMode = SelectLFFile(G_DF_ID_USIM,EF_AUTH_MODE);

	if((fileAddrbp == FALSE) || (fileAuthMode == FALSE))
	{
		return FALSE;
	}

	tempBuf[0] = 0x01;												//FC

	ecpMem_Copy(&tempBuf[1], (BYTE *)gbame, sizeof(gbame));				//P0

	tempBuf[1+sizeof(gbame)] = 0x00;								//L0
	tempBuf[2+sizeof(gbame)] = sizeof(gbame);

	pos = 1+2+sizeof(gbame);										//point to P1

	ReadTFBuf(fileAddrbp, 0x01, &tempBuf[pos], 16);

	tempBuf[pos+16] = 0x00;						//L1

	tempBuf[1+pos+16] = 16;

	pos += (2+16);	////point to P2

	impiLen = imsiToIMPI(&tempBuf[pos]);  //p2

	if(impiLen == 0)
	{
		return FALSE;
	}

	pos += impiLen;

	tempBuf[pos++] = 0;

	tempBuf[pos++] = impiLen; //L2

	ecpMem_Copy(&tempBuf[pos],nafId,nafIdLen);//P3

	pos += nafIdLen;

	tempBuf[pos++] = 0;

	tempBuf[pos++] = nafIdLen; //L3

	ReadTFBuf(fileAuthMode, 0x01, &tempBuf[pos], 0x20);

	HMACSha256DoFinalSingleCall(&tempBuf[pos],0x20,&tempBuf[0],pos,&outBuf[0]);

	return TRUE;

}

/**
  获取NAF_ID相对应记录的Ks_int_NAF
*
* @param outArray     output data array to store the Ks_int_NAF.
*                     
* @param outOffset    output data offset.  
* @param nafIDLen     NAF_ID的长度
* @param nafIDOff     NAF_ID的偏移
* @param nafIDRef     NAF_ID数组的引用 
*
*/
void NM_GetKs_int_NAF(void)
{

	WORD idata nafIDRef,nafIDOff,nafIDLen,destBufRef,desBufOff,mode;
	UWORD xdata nafIDAddr,destBufAddr,fileAddr,fileAddrKs,fileGBAModeAddr;
	BYTE idata i,result,gbaAuthMode,recNum;
	
	result = FALSE;
	
	desBufOff = PopOperandStackShort();  //output data array 的偏移
	destBufRef = PopOperandStackShort();  //output data array to store the Ks_int_NAF.
	nafIDLen =  PopOperandStackShort();     //NAF_ID的长度
	nafIDOff =  PopOperandStackShort();     // NAF_ID的偏移
	nafIDRef = PopOperandStackShort();    //NAF_ID数组的引用 
	mode= PopOperandStackShort();//mode:加解密/签名验签
	
	nafIDAddr = GetValidArrayDataAddr(nafIDRef, nafIDOff, nafIDLen);//获取NAF_ID数组的实际地址
	destBufAddr = GetValidArrayDataAddr(destBufRef, desBufOff, 0x10);//获取output data array的实际地址
			
	
	fileAddr = SelectTFFile(G_DF_ID_USIM, EF_GBANL_USIM);		//选择NAFGBA列表文件
	
	fileAddrKs = SelectLFFile(G_DF_ID_USIM,EF_GBA_KS); //EF_GBA_KS文件名不再区分USIM  ISIM ，统一使用EF_GBA_KS,by xwh 20220718
	
	if((fileAddr == FALSE) || (fileAddrKs == FALSE))
	{
		PushOperandStackShort(FALSE);
		return;
	}
	
	if(nafIDLen > 0x30)//移动v2x规范，nafIDLen最大不能超过0x30
	{
		PushOperandStackShort(FALSE);
		return;
	}
	
	VMReadBuf(&tempBuf[400], nafIDAddr, nafIDLen);//读取NAF_ID数据到tempbuf[400]位置
	
	fileGBAModeAddr =  SelectLFFile(G_DF_ID_USIM,EF_AUTH_MODE);
	
	if(fileGBAModeAddr == FALSE)
	{
		PushOperandStackShort(FALSE);		
		return;
	}
	
	ReadTFBuf(fileGBAModeAddr, 0, &gbaAuthMode, 1);//gbaAuthMode放在eeprom中，GBA_ME_init_01_07需求，鉴权复位之后，模式还要维持上一次的值	
	
	if(G_GBA_AUTH_Status == AUTH_MODE_GBA_BOOT)//GBA-U模式
	{
		recNum = GetLFRecNumLgth(fileAddr,FileNum);
		for(i = 0x01; i<= recNum; i++)//遍历EF_GBANL_USIM文件，查找和NAF_ID相匹配的记录
		{
			ReadLFBuf(fileAddr, i, 0x00 , &tempBuf[0], 2);
			
			if (tempBuf[0] == 0x80)
			{ 			
				ReadLFBuf(fileAddr, i, 2 , &tempBuf[2], tempBuf[1]);
				
				if((tempBuf[1] == nafIDLen) && (ecpMem_Compare(&tempBuf[2],&tempBuf[400],nafIDLen) == 0))
				{
					break;
				}
			}
		}
		
		if(i <= recNum)//找到相匹配的记录
		{		
			ReadLFBuf(fileAddrKs, i, 0 , &tempBuf[0], GetLFRecNumLgth(fileAddrKs,FileLgth));//在Ks_int_NAF文件中读取该条记录的Ks_int_NAF数据，并写到输出数组中
			
			if(mode ==  CIPERMODE)	//sign算法的key不需要XOR
			{
				for(i=0;i<16;i++)//算法:AES-128-CBC-NOPAD  秘钥:Key[0-15] = (Ks_int_NAF[0-15]) XOR (Ks_int_NAF[16-31])
				{
					tempBuf[i] = tempBuf[i] ^  tempBuf[16+i];
				}
				
				VMWriteBuf(destBufAddr, &tempBuf[0], 0x10);
			}
			else
			{
				VMWriteBuf(destBufAddr, &tempBuf[0], 0x20);
			}
			
			PushOperandStackShort(TRUE);
		}
		else//未找到相匹配的记录
		{
			PushOperandStackShort(FALSE);
		}
	}
	else
	{
		if(gbaAuthMode == AUTH_MODE_GBA_ME)//GBA-ME模式
		{
			result = generateME_KsNAF(&tempBuf[400],nafIDLen,&tempBuf[400]);
			if(result == FALSE)
			{
				PushOperandStackShort(FALSE);
			}
			else////算法:AES-128-CBC-NOPAD  秘钥:Key[0-15] = (Ks_int_NAF[0-15]) XOR (Ks_int_NAF[16-31])
			{
				if(mode ==  CIPERMODE)	//sign算法的key不需要XOR
				{
					for(i=0;i<16;i++)
					{
						tempBuf[400+i] = tempBuf[400+i] ^  tempBuf[400+16+i];
					}
					
					VMWriteBuf(destBufAddr, &tempBuf[400], 0x10);
				}
				else
				{
					VMWriteBuf(destBufAddr, &tempBuf[400], 0x20);
				}
											
				PushOperandStackShort(TRUE);
			}
		}
		else
		{
			PushOperandStackShort(FALSE);//init之前需要进行鉴权操作
		}
	}

}

/*调用共享接口应用AID在授权应用列表(EF_AFL)中*/
void NM_Check_AFL(void)
{

	WORD idata previousContext,aidLen;
	BYTE idata appId,i,tag,recNum,recLen;
	UWORD xdata AFLAddr,nafIDAddr,fileAddr;
	BYTE xdata aidBuf[0x10];
	WORD idata nafIDRef,nafIDOff,nafIDLen;
	
	nafIDLen =  PopOperandStackShort();     //NAF_ID的长度
	nafIDOff =  PopOperandStackShort();     // NAF_ID的偏移
	nafIDRef = PopOperandStackShort();    //NAF_ID数组的引用 
	
	if(nafIDRef != REF_NULL)
	{
		nafIDAddr = GetValidArrayDataAddr(nafIDRef, nafIDOff, nafIDLen);//获取NAF_ID数组的实际地址
	}
	else
	{
		nafIDAddr = FALSE;
	}
	
	
	previousContext = getPreviousStackContext(); //获取前一个上下文
	
    appId = previousContext&0xFF;  //获取当前context所指示的appId	
	
	aidLen = GetAIDDataByAppID(appId,aidBuf);
	
    if(aidLen == 0)
	{
		PushOperandStackShort(FALSE);
		return;
	}	
	
	tag=0xF0;
	
	fileAddr= SelectTFFile(DF_MF, EF_CARDCFG);
	
	if(fileAddr!=FALSE)
	{
		ReadTFBuf(fileAddr,0,&tag,1);
	}

	G_DF_ID_USIM = ((0x7F<<8)&0xFF00)+(0xFF&tag);//GBA_ME_init_01_07，复位之后，发送0070指令，没有进入UICCInition，因此G_USIMADFFID没有赋值
	
	AFLAddr = SelectLFFile(G_DF_ID_USIM,EF_AFL);//选择授权列表文件
	
	if(AFLAddr == FALSE)
	{
		PushOperandStackShort(FALSE);
		return;
	}
	
	recNum = GetLFRecNumLgth(AFLAddr,FileNum);
	recLen = GetLFRecNumLgth(AFLAddr,FileLgth);
	
	for(i=1 ; i<=recNum;i++)
	{
		ReadLFBuf(AFLAddr, i, 0 , &tempBuf[0], recLen);
		if((tempBuf[0] == aidLen) && (ecpMem_Compare(aidBuf,tempBuf+1,aidLen)==0))//调用共享接口应用AID在授权应用列表(EF_AFL)中
		{
			if((tempBuf[1+aidLen] == 0xFF) || (nafIDLen == 0))
			{
				PushOperandStackShort(TRUE);
			    return;
			}
			else//若授权应用列表中AID对应的记录存在NAF_ID，仅能使用对应NAF_ID的ks_int_NAF
			{
				VMReadBuf(&tempBuf[100], nafIDAddr, nafIDLen);//读取NAF_ID数据	
				if((nafIDLen == tempBuf[1+aidLen]) && (ecpMem_Compare(&tempBuf[100],&tempBuf[2+aidLen],nafIDLen)==0))
				{
					PushOperandStackShort(TRUE);
			        return;
				}
			}
			
		}		
	}
	
	PushOperandStackShort(FALSE);
}


void NM_CheckGlobalArray(void)
{
	jref idata arrary;
	PN8			D_A		arraryAddr;

	arrary  = PopOperandStackRef();  	//

	arraryAddr = MEMtPN8(locateObjectInMemory(arrary));

	if(arraryAddr == NULL)
	{
		ThrowError(SECURITY_EXCEPTION);
	}

	if(!IS_GLOBAL_ARRAY(arraryAddr))
	{
		ThrowError(SECURITY_EXCEPTION);
	}
}

#endif

	
	
		

