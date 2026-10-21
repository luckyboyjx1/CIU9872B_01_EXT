#ifndef _GPSCP81KEYMANAGEMENT_H
#define _GPSCP81KEYMANAGEMENT_H
#include "typedef.h"

/***************************function define****************************************************************/
extern void GPStoreSCP81Key(BYTE* tmpSessionKey, BYTE decryptionAlg, BYTE curSdId);
//extern BYTE GPPutSCP81Key(BYTE* tmpSessionKey, BYTE decryptionAlg, BYTE sdId);
//extern BYTE GPSSDExistCompleteSCP81Key(BYTE sdId);
#endif

