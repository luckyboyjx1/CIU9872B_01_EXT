#ifndef __IO_INTERFACE_H__
#define	__IO_INTERFACE_H__

#include "DataOperation.h"


extern u8 IOInterface_HandleCommand(void);
extern void IOInterface_SendStatus(void);
extern void IOInterface_init(void);

#endif
