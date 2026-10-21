/*
 * scp01.h
 *
 *  Created on: 2013-9-24
 *      Author: jiangxiaohui
 */

#ifndef SCP01_H_
#define SCP01_H_

#include "profile.h"
#include "global_variable.h"
#include "SCP03.h"
#include "SCPInterface.h"

#if GP__SCP__SCP01

extern void generateKeyScp01(PN8 keySetObj);
extern void CalMACSCP01(PR8 srcAddress, u8 len, PR8 keyBuff, PR8 ICVBuff);

#endif






#endif /* SCP01_H_ */

