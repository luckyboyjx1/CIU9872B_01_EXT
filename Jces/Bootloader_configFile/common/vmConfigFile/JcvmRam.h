#ifndef __VM_RAM_H_
#define __VM_RAM_H_

#include "chipHead.h"

#if  HAL__PLATFORM__CHIP_ARM
#pragma  arm section rwdata "Jcvm_Section",zidata = "Jcvm_Section"
#endif
#define JCVM_CHANNEL_MANAGER_RAM_START			userclass(xdata = jcvm_ram)
#define JCVM_CHANNEL_MANAGER_RAM_END

#define JCVM_GLOBAL_VARIABLE_RAM_START			userclass(xdata = jcvm_ram)
#define JCVM_GLOBAL_VARIABLE_RAM_END

#define JCVM_VM_RAM_START						userclass(xdata = fx_ram)
#define JCVM_VM_RAM_END

#define JCVM_APDU_NATIVE_RAM_START				userclass(xdata = fx_ram)
#define JCVM_APDU_NATIVE_RAM_END

#define JCVM_LOADER_RAM_START					userclass(xdata = jcvm_ram)
#define JCVM_LOADER_RAM_END

#define JCVM_RAM_MANAGER_RAM_START				userclass(xdata = jcvm_ram)
#define JCVM_RAM_MANAGER_RAM_END

#define JCVM_ERROR_RAM_START					userclass(xdata = jcvm_ram)
#define JCVM_ERROR_RAM_END

#define JCVM_OB_MANAGER_RAM_START				userclass(xdata = jcvm_ram)
#define JCVM_OB_MANAGER_RAM_END

#define JCVM_CONFIG_RAM_START					userclass(xdata = jcvm_ram)
#define JCVM_CONFIG_RAM_END

#define JCVM_APP_API_START						userclass(xdata = jcvm_ram)
#define JCVM_APP_API_END

#endif

