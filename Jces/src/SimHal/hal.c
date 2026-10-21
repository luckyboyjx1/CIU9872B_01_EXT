/*
 * hal.c
 *
 *  Created on: 2014年2月14日
 *      Author: mowenpan
 */

#include "TLP224.h"
#include "hal.h"
#include <stdlib.h>
#include <setjmp.h>
#include "ecpDefine.h"
#include "SysConfigTarget.h"
#include "stdio.h"
#include "hwConfig.h"
#include "hal_callback.h"
#include "SCPInterface.h"

jmp_buf hl_osReset;

u8 gAtrBuf[1];
//deleted by jiangxiaohui@20220324 分层
//u8 scpData[SCP_DATA_LENGTH] = {0x00}; // merger from EPX\v2.1\BRANCH\wuronghua\FM151M @20220301: SCP_DATA_START定义成sysConfig_GetNvmTop，而如果sct文件修改后不一定能保证SCP_DATA_START和sysConfig_GetNvmTop一定对应(缺陷系统编号365)
u8 RAMSTACK[HW_RAM_SIZE] = { 0 };
u8 ROMSTACK[HW_ROM_SIZE] = { 0 };
u8 EEPROMSTACK[HW_EEPROM_SIZE] = { 0 };

void hal_init(char* host,u16 port,u16 eport)
{
	HwIo_Init(host, port, eport);
}

