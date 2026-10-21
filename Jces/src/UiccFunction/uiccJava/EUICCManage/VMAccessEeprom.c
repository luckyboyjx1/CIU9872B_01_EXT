/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   VMAccessEeprom.c
 *
 * Description: 该文件 VM读写操作，所涉及的地址为物理地址，可能为RAM，也可能为FLASH
 *
 * auther：     xxxx xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/

#include "typedef.h"
#include "hard_api.h"
#include "VMAccessEeprom.h"
#include "FunctionSwitch.h"
#include "MemOperationServe.h"
/*
*@Function: 读取一个 BYTE
*
*@Descrip:  VM统一使用该接口，这里可以读取RAM或FLASH中的数据，因此，传入地址必须是物理地址
*
*@See:
*
*@Param:    srcAddr-读取的数据在FLASH或RAM中的物理地址
*
*@Return:   BYTE-读取一个字节的值
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
BYTE VMReadByte(UWORD srcAddr)
{
	BYTE idata value;

	EP_EepromToRam(&value,(BYTE *)srcAddr,1);

	return value;
}

/*
*@Function: 按 SWORD（signed short）类型数据的 写入
*
*@Descrip:  涉及大小端问题，用于写入指定物理地址的SWORD类型数据
*
*@See:
*
*@Param:    destAddr-目标 物理地址
*           value-要写入的SWORD（signed short）数据的值
*
*@Return:   void
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
void VMWriteShort(UWORD destAddr, SWORD value)
{
	BYTE idata tmp[2];

	tmp[0] = ((value>>8)&0xFF);

	tmp[1] = (value&0xFF);

	VMWriteBuf(destAddr, tmp, 2);
}
/*
*@Function: 按 int 读取
*
*@Descrip:  涉及大小端问题，用于读取指定物理地址的SDWORD类型数据
*
*@See:
*
*@Param:    srcAddr-待读取数据的目标 物理地址
*
*@Return:   SDWORD-读取的int数据
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
SDWORD VMReadInt(UWORD srcAddr)
{
	BYTE xdata value[4];
	UWORD xdata destValue;
	
	EP_EepromToRam(&value[0],(BYTE *)srcAddr,4);

//	return((SDWORD)((value[0]<<24)|(value[1]<<16)|(value[2]<<8)|value[3]));
	destValue = value[0];
	
	destValue = (destValue<<8) + value[1];
	
	destValue = (destValue<<8) + value[2];
	
	destValue = (destValue<<8) + value[3];

	return(destValue);
}

/*
*@Function: 按 int 写入
*
*@Descrip:  涉及大小端问题，用于写入指定物理地址的SDWORD类型数据
*
*@See:
*
*@Param:    destAddr-目标 物理地址
*           value   -要写入的int数据的值
*
*@Return:   void
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
void VMWriteInt(UWORD destAddr, SDWORD value)
{
	BYTE xdata tmp[4];

	tmp[0] = ((value>>24)&0xFF);

	tmp[1] = ((value>>16)&0xFF);

	tmp[2] = ((value>>8)&0xFF);

	tmp[3] = (value&0xFF);

	VMWriteBuf(destAddr, tmp, 4);
}


/*
*@Function: 读3字节地址
*
*@Descrip:  涉及大小端问题，用于读取指定物理地址的3字节地址（读取二级表中3字节地址）
*
*@See:
*
*@Param:    srcAddr-待读取数据的目标 物理地址
*
*@Return:   UWORD-读取的引用值
*
*@Author:   weiys
*
*@History:  V1.0--创建该函数
*/
UWORD VMReadThreeBytesAddr(UWORD srcAddr)
{
	BYTE xdata value[3];
	UWORD xdata destValue;

	destValue = 0x00;
    
	EP_EepromToRam(&value[0],(BYTE *)srcAddr,3);
        
//	destValue = ((value[0]<<16) | (value[1]<<8) | value[2]);
	destValue = value[0];
	
	destValue = (destValue<<8) + value[1];
	
	destValue = (destValue<<8) + value[2];

	return(destValue);
}

/*
*@Function: 读地址
*
*@Descrip:  涉及大小端问题，用于读取指定物理地址的UWORD类型数据(通常是值引用类型数据)
*
*@See:
*
*@Param:    srcAddr-待读取数据的目标 物理地址
*
*@Return:   WORD-读取的引用值
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
UWORD VMReadUword(UWORD srcAddr)
{
	BYTE xdata value[4];
	UWORD xdata destValue;

	EP_EepromToRam(&value[0],(BYTE *)srcAddr,4);

//	destValue = ((value[0]<<24)| (value[1]<<16)| (value[2]<<8)| value[3]);
	destValue = value[0];
	destValue = (destValue<<8) + value[1];
	destValue = (destValue<<8) + value[2];
	destValue = (destValue<<8) + value[3];
	
	return(destValue);
}

/*
*@Function: 写地址
*
*@Descrip:  涉及大小端问题，用于写入指定物理地址的UWORD类型数据(通常是值引用类型数据)
*
*@See:
*
*@Param:    destAddr-目标 物理地址
*           value-要写入的UWORD数据的值
*
*@Return:   void
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
void VMWriteUword(UWORD destAddr, UWORD value)
{
	BYTE xdata tmp[4];

	tmp[0] = ((value>>24)&0xFF);

	tmp[1] = ((value>>16)&0xFF);

	tmp[2] = ((value>>8)&0xFF);

	tmp[3] = (value&0xFF);

	VMWriteBuf(destAddr,tmp,4); 
}


/*
*@Function: 写地址
*
*@Descrip:  涉及大小端问题，用于写入指定物理地址的UWORD类型数据(通常是值引用类型数据)
*
*@See:
*
*@Param:    destAddr-目标 物理地址
*           value-要写入的UWORD数据的值
*
*@Return:   void
*
*@Author:   zzj 2014-4-23
*
*@History:  V0.0--
*/
void WriteUwordNoAtomic(UWORD destAddr, UWORD value)
{
	BYTE xdata tmp[4];

	tmp[0] = ((value>>24)&0xFF);

	tmp[1] = ((value>>16)&0xFF);

	tmp[2] = ((value>>8)&0xFF);

	tmp[3] = (value&0xFF);

	WriteEepromEX(destAddr,tmp,4); 
}
