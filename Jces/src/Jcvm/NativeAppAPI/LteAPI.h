/*
 * LteAPI.h
 *
 *  Created on: 2016Äê6ÔÂ7ÈÕ
 *      Author: linguoming1
 */

#ifndef SRC_JCVM_NATIVEAPPAPI_LTEAPI_H_
#define SRC_JCVM_NATIVEAPPAPI_LTEAPI_H_

#include "DataOperation.h"
#include "profile.h"
#include "stack.h"
#include "LteApi.h"

#if LTE
#include "GeneralNativeMethods.h"
#include "FileViewNative.h"
#include "InitVar.h"
#include "privatedAlgorithmImpl.h"
#define  initInterServiceVar()
#else
	extern  u8 direction;
	extern  u8 paraPointer;
	extern  u16 para[10];
	#define serviceReturnNM()
	extern void initInterServiceVar(void);
	extern void resetPointer(void);
	extern void sendParaObj(void);
	extern void sendParaShort(void);
	extern void rcvParaObj(void);
	extern void rcvParaShort(void);
#endif

#endif /* SRC_JCVM_NATIVEAPPAPI_LTEAPI_H_ */
