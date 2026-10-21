/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   smPublicSoft.c
 *
 * Description: 存储国密软实现的公共函数
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
#include <string.h>
#include <stdio.h>
#include "smPublicSoft.h"
/*
 * 32-bit integer manipulation macros (big endian)
 */
unsigned long GET_ULONG_BE(unsigned char* b)
{
	unsigned long n;
	
	n = ( (unsigned long) b[0] << 24 ) +( (unsigned long)b[1] << 16 ) + ( (unsigned long) (b)[2] <<  8 ) +((unsigned long) (b)[3]);  

	return n;
}
/*
*@description:将unsigned long n转换后存放在数组b中
*
*@function   :PUT_ULONG_BE
*
*@param      :unsigned long n
*@param      :unsigned char *b 
*/
void PUT_ULONG_BE(unsigned long n,unsigned char *b )
{
	 b[0] = (unsigned char) ( (n) >> 24 );     
     b[1] = (unsigned char) ( (n) >> 16 );      
     b[2] = (unsigned char) ( (n) >>  8 );      
     b[3] = (unsigned char) ( (n)       );  
}
