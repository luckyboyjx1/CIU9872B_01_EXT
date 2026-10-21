#ifndef _T0APDUIMPL_H
#define _T0APDUIMPL_H

#include "typedef.h"
/*************************funtion define***************************************/
extern void GlobalSendData(void);

extern void GlobalReceiveData(void);

extern void GlobalSendResponse(void);

extern void GlobalSndResponseAndRcvCommand(void);

extern void Send61XX(void);

extern void Send6CXX(void);



extern void Delay4etu(void);

extern void timer0(BYTE mode);

extern void SendByte(BYTE sendByte);

extern void GlobalReceiveData(void);

extern void GlobalSendData(void);

extern BYTE GPGetCurrenteChannel(void);

extern void ThrowError(WORD exceptionType);

extern BYTE SendProactiveCommand(void);

extern void SendResponseSingle(void);

extern void SetOutgoingAndSend(SWORD bOff,SWORD len);

extern void GlobalReceiveCommandSingle(void);

extern void SendDataSingle(void);

#endif
