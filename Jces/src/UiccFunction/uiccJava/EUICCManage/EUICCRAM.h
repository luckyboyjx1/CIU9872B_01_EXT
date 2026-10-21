#ifndef _EUICCRAM_H
#define _EUICCRAM_H

#include "typedef.h"

extern BYTE xdata G_StoreDataReservedProfileIndex;  //登记保留的ISD-P Index
//extern WORD xdata G_RamResponseWordLen;             //保存执行RAM时响应数据超过255时的响应长度

/***********************function define********************************/
extern BYTE EUICCPreInstallForPersonnalization(void);
extern BYTE EUICC_PreRAMCommand(void);
extern void NM_IsRAMProcessed(void);



#endif

