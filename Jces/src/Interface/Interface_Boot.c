/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   Load.c
*
* Description: 该文件主要用于存放实现 安全域相关的函数
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0
*
************************************************************************************************************/

#include "Interface_Boot.h"
#include "Io_Uart.h" 

/*
*@funtion    :IF_HTTPS_HandShake 
*
*@description: 判断当前的bip数据是https的数据，并且在https握手期间
*
*@param      :
*
*@return     :true or false, 
*
*@history    :create- jiangqb @20200319   
*/

u8 IF_IoUart_isActived()
{
	return IoUart_isActived();
}

