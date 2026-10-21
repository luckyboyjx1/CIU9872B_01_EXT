#ifndef __SYS_CONFIG_HAL_RAM_H_
#define __SYS_CONFIG_HAL_RAM_H_

#include "chipHead.h"

#if  HAL__PLATFORM__CHIP_ARM
//#pragma  arm section rwdata "Hal_Section",zidata = "Hal_Section"
#endif
#define SYS_CONFIG_RAM_START
#define SYS_CONFIG_RAM_END

#define SYS_HASH_SHA384_RAM_START
#define SYS_HASH_SHA384_RAM_END

#define SYS_PATCH_RAM_START
#define SYS_PATCH_RAM_END

#define RSA_RAM_START
#define RSA_RAM_END

#define UART_HAL_RAM_START
#define UART_HAL_RAM_END

#define SYM_DES_HAL_RAM_START
#define SYM_DES_HAL_RAM_END

#define SOFT_AES_RAM_START
#define SOFT_AES_RAM_END

#define SOFT_DES_RAM_START
#define SOFT_DES_RAM_END

#define SOFT_KOREANSEED_RAM_START
#define SOFT_KOREANSEED_RAM_END

#endif
