#ifndef   __BOOT_ERROR_HANDLE_H__
#define   __BOOT_ERROR_HANDLE_H__

#include "DataOperation.h"
#include <setjmp.h>

extern  void bootErrorHandle_Init(void);
extern  void bootErrorHandle_SetLongJmp(TYPE_JUP_BUF *  errorHandle);
extern  void bootErrorHandle_ThrowError(u16 errorcode);

#endif
