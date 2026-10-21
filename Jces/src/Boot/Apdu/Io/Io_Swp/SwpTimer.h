#ifndef __SWP_TIMER_h__
#define __SWP_TIMER_h__
/**************************************************************************/
/*!  \b Project Driver Library for ST32 Smartcard products.
*
*    \b Component : SWP
*    \file    SWP_TIMER.h
*    \brief   Prototypes that are relative to SWP_TIMER.c source file
*
***************************************************************************
*
*    \b Organization : MMS/DSA/SCP/ScDevTools Rousset
*
*    \b Compiler: KEIL/ARM
***************************************************************************
*    \b COPYRIGHT : (C) ST Microelectronics 2003 - 2008 ALL RIGHTS RESERVED\n
*    This program contains proprietary information and is not to be used, copied, nor \n
*    disclosed without the written consent of ST Microelectronics.
**************************************************************************
*    \internal
*
*    \b Maturity : Status => In Progress
*
*    \b History :
*    \code
*    Date ----- Author ------ Comments ----------------------------------------------
*
*
*    \endcode
*
***************************************************************************/

#include "SwpTimerHal.h"
#if BOOT__IO__SWP

	/// @addtogroup SWP_TIMER
	/// @{
	extern u8 isSwpTimerRunning(void);

	/// @brief Starts a SWP timer
	extern void swpTimer_SwpTimerStart(u8 swpTimerType);

	/// @brief Stops a SWP timer
	extern void swpTimer_SwpTimerStop( void );
	extern void swpTimer_SwpTimerHandle(void);

/// @} SWP_TIMER
#else
	#define  isSwpTimerRunning() 0
	#define  swpTimer_SwpTimerStart( swpTimerType)
	#define  swpTimer_SwpTimerStop()
	#define  swpTimer_SwpTimerHandle()
#endif
#endif
