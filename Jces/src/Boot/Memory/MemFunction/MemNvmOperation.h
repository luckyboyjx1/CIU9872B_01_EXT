/*
 *
 *  Created on: 2014-2-19
 *      Author: yangfajun
 */
#ifndef __MEM_HAL_H__
#define	__MEM_HAL_H__


#include "DataOperation.h"
#include "MemBackUpServe.h"
#include "SysConfigProfile.h"

#if BOOT__MEMORY__OPERATION
	#define	WRITE_ONLY					0
	#define WRITE_ERASE					1
	#define FILL_ERASE					2
	
	/*
	 * NVM 擦
	 * dest   ：擦操作开始地址，可以不页对齐
	 * length ：擦除长度，长度无需字、半字或页对齐
	 * 返回        : 0(擦操作成功)，非0(非正常返回，执行不完全成功)
	 * 说明         ：在FLASH平台，可能会出现只擦不写的情况，因此该接口在FLASH平台
	 *          应该实现。
	 * */
	extern u8 MemNvmOperation_Erase(PN8 dest, u16 length);

	extern u8 MemNvmOperation_WriteOnly(PN8 dest, PM8 src, u16 length);

	/*
	 * NVM 写
	 * dest   :目的地址
	 * src    :源地址
	 * length :拷贝长度，任意长度，无需字、半字或页对齐
	 * 返回        : 0(擦操作成功)，非0(非正常返回，执行不完全成功)
	 * 说明         ：在EEPROM平台，该函数就是底层写EEPROM函数；
	 *          在FLASH平台，该函数应该集成——处理对齐、擦除和写于一体。
	 * */
	extern u8 MemNvmOperation_WriteErase(PN8 dest, PM8 src, u16 length);

	/*
	 * NVM 填充
	 * dest   :目的地址
	 * value  :填充数值
	 * length :填充长度，任意长度，无需字、半字或页对齐
	 * 返回        : 0(擦操作成功)，非0(非正常返回，执行不完全成功)
	 * 说明         ：在EEPROM平台，该函数就是底层填充EEPROM函数；
	 *          在FLASH平台，该函数应该集成——处理对齐、擦除和写于一体。
	 * */
	extern u8 MemNvmOperation_Fill(PN8 dest, u8 value, u16 length);

#else
	#define MemNvmOperation_Erase(dest, length)							0
	#define MemNvmOperation_Write(dest, src, length)					0
	#define MemNvmOperation_Fill(dest, value, length)					0
#endif

#endif  //__MEM_HAL_H__


