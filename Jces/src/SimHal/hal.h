/*
 * hal.h
 *
 *  Created on: 2014年2月14日
 *      Author: mowenpan
 */

#ifndef HAL_H_
#define HAL_H_
#include <setjmp.h>
#include "SysConfigTarget.h"
#include "hal_callback.h"

extern u8 gAtrBuf[1];
extern u8 RAMSTACK[];
extern u8 ROMSTACK[];
extern u8 EEPROMSTACK[];

//extern u8 scpData[]; //deleted by jiangxiaohui@20220324 分层
extern jmp_buf hl_osReset;
#define hw_setResetHandler()  setjmp(hl_osReset)
#define hw_gotoResetHandler() longjmp(hl_osReset, 0)

extern void hal_init(char* host,u16 port,u16 eport);


#endif /* HAL_H_ */
