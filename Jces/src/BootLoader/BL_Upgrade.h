#ifndef __BL_UPGRADE_H_
#define __BL_UPGRADE_H_

#include "profile.h"
#include "bootPlaformConfigProfile.h"

#if COS_UPDATE_ONLINE_BY_NativeOS||BOOTLOADER
#include "FunImp.h"
#endif

#if BOOTLOADER_PRO
#include "BootLoader.h"
#endif

#if COS_UPDATE_ONLINE_MUTUAL
#include "UpCos.h"
#endif


#endif
