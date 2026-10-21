#ifndef  __OPINSTALLER_H__
#define  __OPINSTALLER_H__
#include "typejava.h"

extern	u16 opInstallForLoad(void);
extern	u16 opInstallForInstall(void);
extern	u8  checkMandatedDAP(void);
extern  u16 opInstallForOther(u8  installFlag)    SMALL_MODE;
extern  u16 opLoadCheck(void);

#endif
