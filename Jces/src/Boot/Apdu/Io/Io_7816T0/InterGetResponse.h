/*
 * InterGetResponse.h
 *
 *  Created on: 2015Äê2ÔÂ2ÈÕ
 *      Author: linguoming
 */

#ifndef __INTER_GET_RESPONSE_H_
#define __INTER_GET_RESPONSE_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"

#if BOOT__CHANNEL__GET_RESPONSE_DATA

	#define	MAX_INTER_CHN						(ISO_MAX_LOGICAL_CHANNELS + 1)
	#define DEFAULT_INDEX						0
	#define NULL_TBL_ENTRY						0

	extern	void InterGetRes_Init(void);
	extern	void InterGetRes_Realease(void);
	extern  void InterGetRes_Main(u8 cla);
#else
	#define	InterGetRes_Init()
	#define InterGetRes_Realease()
	#define	InterGetRes_Main(cla)
#endif

#endif /* INTERGETRESPONSE_H_ */
