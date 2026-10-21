
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MHCARD_CONF_H
#define __MHCARD_CONF_H

/* Includes ------------------------------------------------------------------*/

  
#include <string.h>
#include <stdint.h>
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/

#define USE_FPGA        (0)
#if   defined ( __CC_ARM )
    #define DBG_PRINT(format, args...)  printf(format, ##args)

#elif defined ( __GNUC__ )
    int printf_GCC(char *fmt, ...);
    #define DBG_PRINT(fmt, args...)      printf_GCC(fmt, ##args)

#elif defined ( __ICCARM__ )

    #define DBG_PRINT(format, args...)  printf(format, ##args)
#endif

#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed. 
  *   If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */


/* Uncomment the line below to enable peripheral header file inclusion */
#include "mhcard_crc.h"
#include "mhcard_flash.h"
#include "mhcard_gpio.h"
#include "mhcard_timer.h"
#include "mhcard_wdt.h"
#include "mhcard_iso7816.h"
#include "mhcard_sysctrl.h"
#include "mhcard_cache.h"
#include "mhcard_spi.h"
#include "mhcard_ssc.h"
#include "misc.h"

#endif 
