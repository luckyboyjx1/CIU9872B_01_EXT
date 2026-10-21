/*
 * hw_memory.h
 *
 *  Created on: 2015Äê1ÔÂ30ÈÕ
 *      Author: mowenpan
 */

#ifndef HW_MEMORY_H_
#define HW_MEMORY_H_
#include <windows.h>

#include "cardAPI.h"
extern void hw_memoryInit(char *initHex, char *outHex);
extern void hw_nvmWrite(PN8 desc, PM8 src, u16 length);
extern void hw_memoryInit(char *initHex, char *outHex);
//extern void hw_ramReset();

#endif /* HW_MEMORY_H_ */
