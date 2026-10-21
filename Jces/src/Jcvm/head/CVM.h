#ifndef  __CVM_H__
#define  __CVM_H__
#include "typejava.h"
#include "JcvmConfig.h"

#define GLOBAL_PIN_DATA_SIZE	13
#define	GLOBAL_PIN_SIZE		8

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
#define CVM_TRY_MAX			CvmDataAddress  //CVM_PIN
#else
#define CVM_TRY_MAX			  VM_CVM_PIN//CVM_PIN
#endif
#define CVM_TRY_MAX_OFFSET			0
#define	CVM_TRY_REMAIND_OFFSET		1
#define	CVM_PIN_FORMAT_OFFSET		2
#define	CVM_PIN_LENGTH_OFFSET		3
#define	CVM_PIN_VALUE_OFFSET		4

#define	CVM_TRY_REMAIND		(CVM_TRY_MAX + 1)
#define	CVM_PIN_FORMAT		(CVM_TRY_REMAIND + 1)
#define	CVM_PIN_LENGTH		(CVM_PIN_FORMAT + 1)
#define	CVM_PIN_VALUE		(CVM_PIN_LENGTH + 1)
#if GP__CVM__FOR_MASK
	extern u8 checkPinValue(void);
	extern u8 updatePinValue(void);
	extern u8 accessPinInfo(void);
	extern void getIsdGlobalPinData(void);
#else
	#define checkPinValue()      		0
	#define updatePinValue()     		0
	#define accessPinInfo()      		0
	#define getIsdGlobalPinData()
#endif
//void decrementTriesRemain(void);

#endif

