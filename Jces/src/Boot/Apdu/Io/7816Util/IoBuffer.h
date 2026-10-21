#ifndef __IO_BUFFER_H_
#define __IO_BUFFER_H_

#include "DataOperation.h"
#include "7816Util.h"

#define	UART_INT_BUF_PCK_OFFSET				0
#define	CL_INT_BUF_PCK_OFFSET				1
#define	SPI_INT_BUF_PCK_OFFSET				1

#define IO_CACHE_BUFFER_SIZE				266
#define RESPONSE_BUFFER_SIZE				266
#define	IO_T0_CMD_BUF_SIZE					266
#define IO_CMD_BUF_SIZE						266

#define	NO_BUF_PCK_TYPE						0
#define	UART_BUF_PCK_TYPE					1
#define CL_BUF_PCK_TYPE						2
#define SPI_BUF_PCK_TYPE					2
#define I2C_BUF_PCK_TYPE					2


typedef struct
{
	PX8		pdata;
	u16		size;
}IoBufType;

typedef struct
{
	IoBufType				ioCacheBuf;
	IoBufType				responseBuf;
	IoBufType				commandBuf;
	ppIso7816Util_BufPck_t	Uart7816UtilBufPck;
}IoBufPck;

extern u8 X_A bResBuffer[RESPONSE_BUFFER_SIZE];
extern u8 X_A IoCacheBuffer[IO_CACHE_BUFFER_SIZE];
extern u8 X_A bResBuffer1[RESPONSE_BUFFER_SIZE];

extern CONST IoBufPck IoBuffer[];

#endif
