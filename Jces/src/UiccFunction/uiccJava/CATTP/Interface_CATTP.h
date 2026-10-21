#ifndef _INTERFACE_CATTP_H_
#define _INTERFACE_CATTP_H_
#include "typedef.h"

/*************function define**************************************/
extern void IF_CATTPDataAvaiable(WORD PDULen);
extern BYTE IF_CATTP_ChannelStatus(void);
extern void IF_CATTP_ariableInit(void);
extern BYTE IF_GetOpenChannelParaForCATTP(BYTE *para);
extern BYTE IF_CATTP_ProcessCompactRFMRAMAPDU(BYTE cattype);
extern void IF_CATTP_RetryCattpsessionByTimerExpiration(BYTE timerExpirationId);
#endif
