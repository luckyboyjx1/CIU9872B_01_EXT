/*
 * IoBuffer.c
 *
 *  Created on: 2015Äê1ÔÂ15ÈÕ
 *      Author: linguoming
 */

#include "IoBuffer.h"
#include "7816Util.h"
#include "SysConfigProfile.h"
#include "bootConfigProfile.h"
#include "BootNoAreaInitRam.h"

#if (/*HAL__IO__ISO7816_T0 && HAL__IO__ISO7816_T1 && */HAL__IO__ISO14443_TA && !BOOT__IO__MIX_MODE)

	u8						X_A	IoCacheBuffer[IO_CACHE_BUFFER_SIZE];
	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck;

	CONST IoBufPck IoBuffer[] =
	{
	/*******************Uart buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&(bResBuffer[5]), (RESPONSE_BUFFER_SIZE - 5),
			 (PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck},
	/*******************CL buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck},
	};
#endif

#if 0 /*(HAL__IO__ISO7816_T0 && !HAL__IO__ISO7816_T1 && HAL__IO__ISO14443_TA && !BOOT__IO__MIX_MODE)*/

	u8						X_A	IoCacheBuffer[IO_CACHE_BUFFER_SIZE];
	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	u8						X_A	bResBuffer2[RESPONSE_BUFFER_SIZE];
	u8						X_A	commandBuffer2[IO_CMD_BUF_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck1;
	pIso7816Util_BufPck_t	X_A	UtilBufPck2;

	CONST IoBufPck IoBuffer[] =
	{
	/*******************Uart buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			(PX8)&(bResBuffer[5]), (RESPONSE_BUFFER_SIZE - 5),
			 (PX8)&bResBuffer, IO_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck1},
	/*******************CL buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer2, IO_CACHE_BUFFER_SIZE,
			 (PX8)&commandBuffer2, IO_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck2},
	};
#endif

#if  (HAL__IO__ISO7816_T0 && !HAL__IO__ISO7816_T1 && !HAL__IO__ISO14443_TA && BOOT__IO__MIX_MODE)

	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck1;

	CONST IoBufPck IoBuffer[] =
	{
	/*******************Uart buffer****************/
			 {(PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (PX8)&(bResBuffer[5]), (RESPONSE_BUFFER_SIZE - 5),
			 (PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck1},
			 
	/*******************CL buffer****************/
	};
#endif

#if 0 /*(HAL__IO__ISO7816_T0 && !HAL__IO__ISO7816_T1 && HAL__IO__ISO14443_TA && BOOT__IO__MIX_MODE)*/

	u8						X_A	IoCacheBuffer[IO_CACHE_BUFFER_SIZE];
	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	u8						X_A	bResBuffer2[RESPONSE_BUFFER_SIZE];
	u8						X_A	commandBuffer2[IO_CMD_BUF_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck1;
	pIso7816Util_BufPck_t	X_A	UtilBufPck2;

	CONST IoBufPck IoBuffer[] =
	{
	/*******************Uart buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer, IO_T0_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck1},
	/*******************CL buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer2, IO_CACHE_BUFFER_SIZE,
			 (PX8)&commandBuffer2, IO_T0_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck2},
	};
#endif

#if (HAL__IO__ISO7816_T0 && HAL__IO__ISO7816_T1 && HAL__IO__ISO14443_TA && BOOT__IO__MIX_MODE)

	u8						X_A	IoCacheBuffer[IO_CACHE_BUFFER_SIZE];
	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	u8						X_A	bResBuffer2[RESPONSE_BUFFER_SIZE];
	u8						X_A	commandBuffer2[IO_CMD_BUF_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck1;
	pIso7816Util_BufPck_t	X_A	UtilBufPck2;

	CONST IoBufPck IoBuffer[] =
	{
	/*******************Uart buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer, IO_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck1},
	/*******************CL buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer2, IO_CACHE_BUFFER_SIZE,
			 (PX8)&commandBuffer2, IO_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck2},
	};
#endif

#if 0//(HAL__IO__ISO7816_T0 && !HAL__IO__ISO7816_T1 && !HAL__IO__ISO14443_TA && BOOT__IO__MIX_MODE)

	u8						X_A	IoCacheBuffer[IO_CACHE_BUFFER_SIZE];
	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	u8						X_A	bResBuffer2[RESPONSE_BUFFER_SIZE];
	u8						X_A	commandBuffer2[IO_CMD_BUF_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck1;
	pIso7816Util_BufPck_t	X_A	UtilBufPck2;

	CONST IoBufPck IoBuffer[] = 
	{
	/*******************Uart buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&(bResBuffer[5]), (RESPONSE_BUFFER_SIZE - 5),
			 (PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck1},
	/*******************CL buffer****************/
			{(PX8)&IoCacheBuffer, IO_CACHE_BUFFER_SIZE,
			 (PX8)&bResBuffer2, RESPONSE_BUFFER_SIZE,
			 (PX8)&commandBuffer2, IO_CMD_BUF_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck2},
	};
#endif


#if (HAL__IO__ISO7816_T0 && !HAL__IO__ISO7816_T1 && !HAL__IO__ISO14443_TA && !BOOT__IO__MIX_MODE)

	u8						X_A	bResBuffer[RESPONSE_BUFFER_SIZE];
	pIso7816Util_BufPck_t	X_A	UtilBufPck1;

	CONST IoBufPck IoBuffer[] =
	{
	/*******************Uart buffer****************/
			 {(PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (PX8)&(bResBuffer[5]), (RESPONSE_BUFFER_SIZE - 5),
			 (PX8)&bResBuffer, RESPONSE_BUFFER_SIZE,
			 (ppIso7816Util_BufPck_t)&UtilBufPck1},
			 
	/*******************CL buffer****************/
	};

#endif

	
