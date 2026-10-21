#ifndef __7816_CHANNEL_H_
#define __7816_CHANNEL_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"

#if BOOT__IO__ISO7816
	extern  void iso7816Channel_Init(void);
	extern  void iso7816Channel_SetLogicalChannel(void);
	extern  u8 	 iso7816Channel_GetCurLogicalChn(void);
	extern 	u8 	 iso7816Channel_CalLogicalChannel(u8 cla);
#else
	#define	iso7816Channel_Init()
	#define	iso7816Channel_SetLogicalChannel()
	#define iso7816Channel_GetCurLogicalChn()					0
#endif

#endif	/*	__7816_CHANNEL_H_	*/

