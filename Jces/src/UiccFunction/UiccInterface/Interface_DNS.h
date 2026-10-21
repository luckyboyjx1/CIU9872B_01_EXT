#ifndef DNS_INTERFACE_H
#define DNS_INTERFACE_H
#include "typedef.h"

extern BYTE IF_DNS_GetOpenChannelPara(BYTE *para);
extern BYTE IF_RAS_GetOpenChannelPara(BYTE *para);
extern BYTE IF_DNS_TriggerCheck(UWORD paraAddr, WORD dnsParaOffset, WORD allLen);
extern void IF_DNSDataAvaiable(BYTE channelDataLen, WORD offset);
extern BYTE IF_DNS_TriggerParaAnalyse(UWORD paraAddr, WORD dnsParaOffset, WORD allLen);
extern u8 IF_DNSWithTLSEndState(void);
#endif



