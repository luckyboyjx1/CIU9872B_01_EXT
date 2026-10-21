#ifndef __BOOTLoader_H_
#define __BOOTLoader_H_

#include "typedef.h"
#include "FunImp.h"

#define 	InitJCRE3()
#define 	reset()
#if !NATIVE_CARD
#define 	run()								uicc_main()

//#define 	InitGlobalVariable()				initBootLoaderVar()
#define 	InitGlobalVariable()                do{initBootLoaderVar(); SystemInit_Variable();}while(0)
#else
#define 	run()	
#define 	InitGlobalVariable() 				initBootLoaderVar();
#endif
	
#define 	initCardData(apduBuffer)			BOOT_CLA_NOT_SUPPORTED

#define 	SetCOSVersionData(ApduBuffer)    0x6985
#define 	getSessionkey(ApduBuffer)        0x6985



#endif
