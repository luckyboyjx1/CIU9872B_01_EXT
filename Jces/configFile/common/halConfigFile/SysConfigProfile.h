/*****************************************************************************
 profile.h

 config the OS.

 wang hui
 eastcompeace Technology Corporation
 Flags:
 .
 Note:
 .

 History:
 2007,6,23	.Created
 *****************************************************************************/

#ifndef   __SYS_CONFIG_PROFILE_H__
#define   __SYS_CONFIG_PROFILE_H__


#include "SysPlatformConfigProfile.h"
#include "bootConfigProfile.h"

/*=========================================================================== */
/*							APDU , IO features								  */
/*=========================================================================== */


//#include "bootPlaformConfigProfile.h"
//#include "profile.h"


/*=========================================================================== */
/*							MEM OPERATION features							  */
/*=========================================================================== */
#define HAL__MEMORY__OPERATION						1

/********************************断电保护配置项功能表****************************************

CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE | TRANSACTION_OLDDESIGN | LIFE_OR_PERFORMANCE |  功能
									|                       |                     |
				Any					|          1     	    |          Any        |	1.使用旧版本的断电保护方案	      （事务保护字写、按长度保护、页保护flag字写、芯片需支持字写）
				1					|          0            |          0          | 2.使用新版本的断电保护字写方案    （事务保护已页为单位且不重复保护、页保护保护非事务中的原子非原子、芯片需支持字写）
				0					|          0            |          0          | 3.使用新版本的断电保护页写寿命方案 (事务保护方案和页保护方案中Flag页皆采用页擦写、页保护中数据保护页和Flag页混合轮换、芯片支持页写或字写都行、性能比方案2慢）
				0					|          0            |          1          | 4.使用新版本的断电保护页写性能方案 (较方案3，性能较快、页保护页寿命较短、也会轮询、达到和方案3相同的寿命需要开较大的空间)
*******************************************************************************************/

#define CHIP_SUPPORT_WORDWRITE_OR_PAGEWRITE	 0
#define TRANSACTION_OLDDESIGN				 1
#define LIFE_OR_PERFORMANCE         		 0


#define WITH_BREAK_TEST_CODE				 0

/*=========================================================================== */
/*							PATCH features									  */
/*=========================================================================== */

/*=========================================================================== */
/*							USER CONFIG features							  */
/*=========================================================================== */
#define HAL__SYSTEM_CONFIG							1
#define HAL__CONFIG_AREA__MEMORY					1
#define HAL__CONFIG_AREA__CLK_OPERATION				0

#define HAL__CONFIG_AREA__OLD						1
#define HAL__CONFIG_AREA__NEW						0

#define HAL__MEMORY__RAM_MAP_IN_C51      			0
#define HAL__UNIT__FOR_DEBUG						0
#define CLEAR_OS									1
/*=========================================================================== */
/*							SECURE SAFE CRYPTO								  */
/*=========================================================================== */
#define HAL__SECURITY__SAFETY_CRYPTO				0


//#if BOOT__IO__SWP

//#define HAL__IO__SWP							    1
//#else
//#define HAL__IO__SWP							    0
//#endif

//#if HAL__IO__SWP
//#define HAL__IO__SWP__LIB                           1
//#else
//#define HAL__IO__SWP__LIB                           0
//#endif

//#if (BOOT__IO__MIFARE)
//	#if HAL__PLATFORM__SIMULATOR
//		#define	HAL__IO__MIFARE						0
//	#else
//		#define	HAL__IO__MIFARE						1
//	#endif
//#else
//		#define	HAL__IO__MIFARE						0
//#endif



#if HAL__IO__ISO14443_TA
#if HAL__PLATFORM__SIMULATOR
	#define BOOT__IO__ISO14443_TA_RF_LEVEL3				0
#else
	#define BOOT__IO__ISO14443_TA_RF_LEVEL3				1
#endif
	#define WITH_SET_BAUDRATE_DEFAULT_ON_DESELECT	0
	#define WITH_RF_SETBAUTRATE_FIRST				1
	#define WITH_RF_SETSTATE_FIRST					1
	
	#define WITH_RF_USER_MODIFY_PARAM               0
	#define WITH_RF_DERIVE_UID			            0
#else
#define BOOT__IO__ISO14443_TA_RF_LEVEL3			0
#endif

#if	BOOT__BOOTCOMMAND__LOCKED
	#define HAL__BOOTCOMMAND__LOCKED			1
#else
	#define HAL__BOOTCOMMAND__LOCKED			0
#endif
/*=========================================================================== */
/*							SECURE SAFE CRYPTO								  */
/*=========================================================================== */
#define SEC_COUNTER_IN_OS                               0
#if SEC_COUNTER_IN_OS
	#define SYM_KEY_MASK			                    1
	#define PARAMETER_INTEGRITY_CHECK                   1
	#define L_COUNTER_CODE_FLOW_CTRL                    1
	#define G_COUNTER_CODE_FLOW_CTRL                    1
		//if the chip has hardware counter, then set the following HARDWARE_COUNTER macro to one,
		//otherwise set the RAM_COUNTER to one.
		#define HARDWARE_COUNTER                        1
		#define RAM_COUNTER                             1
	#define RNG_OUT_BUFF_CHECK                          1
	#define CODE_SIGNATURE                              1
	#define MAX_ATTACK_COUNTER                          32
	#define ATTACK_COUNTER_AUDIT                        0
	#define WITH_SECURE_DATA_STORAGE                    1
	#define DOUBLE_CHECK                                1
	#define RANDOM_DELAY_ENABLE                         1
	#define SEC_MEM_OPERATION                           1
	#define INVERSE_OPERATION_SUPPORT					1
	#define WITH_RSA_MODULUS_RANDOMIZATION			    0   //used in RSASD mode, and could close it.
	#define DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY        1
	#define SC_PLATFORM_ADM_KEY_LEN						PLATFORM_ADM_KEY_LEN + 0x01
	#define SC_EDC_CHECK_SWITCH_ON						1
#else
	#define SYM_KEY_MASK			                    0
	#define PARAMETER_INTEGRITY_CHECK                   0
	#define L_COUNTER_CODE_FLOW_CTRL                    0
	#define G_COUNTER_CODE_FLOW_CTRL                    0
		//if the chip has hardware counter, then set the following HARDWARE_COUNTER macro to one,
		//otherwise set the RAM_COUNTER to one.
		#define HARDWARE_COUNTER                        0
		#define RAM_COUNTER                             0
	#define RNG_OUT_BUFF_CHECK                          0
	#define CODE_SIGNATURE                              0
	#define MAX_ATTACK_COUNTER                          32
	#define ATTACK_COUNTER_AUDIT                        0
	#define WITH_SECURE_DATA_STORAGE                    0
	#define DOUBLE_CHECK                                0
	#define RANDOM_DELAY_ENABLE                         0
	#define SEC_MEM_OPERATION                           0
	#define INVERSE_OPERATION_SUPPORT					0
	#define WITH_RSA_MODULUS_RANDOMIZATION			    0  //used in RSASD mode, and could close it.
	#define DOUBLE_CHECK_BYTECODE_OF_IF_CATEGORY        0
	#define SC_PLATFORM_ADM_KEY_LEN						PLATFORM_ADM_KEY_LEN
	#define SC_EDC_CHECK_SWITCH_ON						0
#endif

#define EMV_OPTIMIZATION_CODE							0
#define  ARM_                                           0
#endif
