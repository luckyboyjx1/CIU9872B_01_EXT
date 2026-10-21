/*
 * HwIo.h
 *
 *  Created on: 2015Äê10ÔÂ28ÈÕ
 *      Author: mowenpan
 */

#ifndef SRC_HW_IO_HWIO_H_
#define SRC_HW_IO_HWIO_H_

#include "ecpDefine.h"


#define HWIO_BUFFER_MAX_SIZE		266
#define HWIO_DIRECTION_UNKNOWN		0
#define HWIO_DIRECTION_RECV			1
#define HWIO_DIRECTION_SEND			2

typedef struct
{
	u16 len;
	u8 data[HWIO_BUFFER_MAX_SIZE];
} HwIo_Buffer, *PHwIo_Buffer ;

extern void HwIo_Rx(PHwIo_Buffer buffer);
extern void HwIo_Tx(PHwIo_Buffer buffer);
void HwIo_Init(const char* ipAddress, u16 port, u16 eport);

extern u32 sendId;
extern void *sendMonitor(void *arg);

void printData(u16 len,u8* buf);

#endif /* SRC_HW_IO_HWIO_H_ */
