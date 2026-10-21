/******************************************************************************/
/// @cond INCLUDE_FILE_DESCRIPTION
/*
 * @file
 * @brief
 * System Initialization Source File for Infineon SLCx7 C40 Device Series
 *
 */
/// @endcond
/* @addtogroup IFX_TASK_MAN_GROUP
 *  @{
 *
 * Based on system_ARMCM3.c
 * CMSIS Device System Header File for ARMCM3 Device,
 * V1.0.1, 15. November 2019
 *
 * Modifications for Infineon security controllers
 * marked by "Added by Infineon" below
 *
 * THE SOURCE CODE AND ITS RELATED DOCUMENTATION IS PROVIDED "AS IS". INFINEON
 * TECHNOLOGIES MAKES NO OTHER WARRANTY OF ANY KIND,WHETHER EXPRESS,IMPLIED OR,
 * STATUTORY AND DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * SATISFACTORY QUALITY, NON INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * THE SOURCE CODE AND DOCUMENTATION MAY INCLUDE ERRORS. INFINEON TECHNOLOGIES
 * RESERVES THE RIGHT TO INCORPORATE MODIFICATIONS TO THE SOURCE CODE IN LATER
 * REVISIONS OF IT, AND TO MAKE IMPROVEMENTS OR CHANGES IN THE DOCUMENTATION OR
 * THE PRODUCTS OR TECHNOLOGIES DESCRIBED THEREIN AT ANY TIME.
 *
 * INFINEON TECHNOLOGIES SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGE OR LIABILITY ARISING FROM YOUR USE OF THE SOURCE CODE OR
 * ANY DOCUMENTATION, INCLUDING BUT NOT LIMITED TO, LOST REVENUES, DATA OR
 * PROFITS, DAMAGES OF ANY SPECIAL, INCIDENTAL OR CONSEQUENTIAL NATURE, PUNITIVE
 * DAMAGES, LOSS OF PROPERTY OR LOSS OF PROFITS ARISING OUT OF OR IN CONNECTION
 * WITH THIS AGREEMENT, OR BEING UNUSABLE, EVEN IF ADVISED OF THE POSSIBILITY OR
 * PROBABILITY OF SUCH DAMAGES AND WHETHER A CLAIM FOR SUCH DAMAGE IS BASED UPON
 * WARRANTY, CONTRACT, TORT, NEGLIGENCE OR OTHERWISE.
 *
 * (C) COPYRIGHT 2017 INFINEON TECHNOLOGIES AG
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY INFORMATION OF INFINEON
 * TECHNOLOGIES AG. All rights reserved.
 */
/******************************************************************************/
/* Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************/

/* Added by Infineon */
/*  includes */
#include <stdint.h>
#include <IFX_SLE90.h>

#include ".\IFX_Hal\IFX_Memory\IFX_Memory.h"
#include ".\IFX_Hal\Include\IFX_Framework.h"
#include ".\IFX_Hal\IFX_Uart\IFX_GpioSetUartMode.h"
#include ".\IFX_Hal\IFX_Imm\IFX_Imm.h"
#include ".\IFX_Hal\IFX_Clk_Pwr\IFX_Clk_PWr.h"
#include ".\IFX_Hal\IFX_Interrupts\IFX_InterruptHandler.h"
#include "HslNvm_OS_Utils.h"
#include "IFX_Nvm.h"

#if !defined (SLCx7V11a) && !defined (SLCx7V14) && !defined (SLCx7V15) && !defined (SLCx7V16)
    #error "This file is intended for use with SLCx7V11a, SLCx7V14 and SLCx7V15 and SLCx7V16!"
#endif

/*  defines
****************************************************************/
/* Make filename available to DbC */
//DEFINE_THIS_FILE

/** @brief disables the watchdog timer after successful transition from BOS to user mode
*/
static __INLINE void disableWatchdog(void){
    SLCxx_WDT0->WDT_CTRL = 0UL;
}

/*-------- DEFINES -----------------------------------------------------------*/

extern uint32_t __RamVectors[];

extern void Image$$ER_AN_RW_DATA$$Base;
extern void Image$$ER_DtTrRamAlign_RAM$$Base;
#define RAM_GLOABSS_START                    ((unsigned int)&Image$$ER_AN_RW_DATA$$Base)
#define RAM_GLOABSS_SIZE                     (((unsigned int)&Image$$ER_DtTrRamAlign_RAM$$Base) - RAM_GLOABSS_START)
/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup and initialize the system.
 */
SECT90(ifx_startup_code) void SystemInit(void)
{
	unsigned int *	addr;
	unsigned int	off;
	
    // disable watchdog(s)
    disableWatchdog();

	//clear ram
	addr = (unsigned int *)RAM_GLOABSS_START;
	for(off=0; off< (RAM_GLOABSS_SIZE / 4); off++)
	{
		*(addr + off) = 0x00;
	}	

#if (IFX_CFG_INSTRUMENTATION == 1)
    // start instrumentation procedure to measure the system clocks for start up will needed
    IFX_Instrument_Timer_Start(INS_TIMER_SIMPLIFIED_SETUP);
#endif

    // initialize RAM needed for startup
    // can be postponed for CL only as we do not use any interrupts
    if (CB_POWER_SELECTED())
    {
        /* initialize RAM needed for startup, also copies the RAM vector table */
        IFX_Memory_CopyRamCodeRegion(ER_AN_STARTUP_RAM_CODE);
        // switch to vector table in RAM, RAM vector table must be copied beforehand
        SCB->VTOR = (uint32_t)&__RamVectors;

        // initialize remaining RAM code
        // can be postponed for CL only as we do not use any interrupts
        IFX_Memory_CopyRamCodeRegion(ER_AN_RAM_CODE);
#if (IFX_CFG_UART == 1)
        // no need to initialize GPIO to UART in CL only mode
        IFX_GpioSetUartMode();
#endif
    }
		
//		HslNvm_OS_Utils_Init();
		
    /* Init IFX_AppNote heap space (needed for e.g. IFX_Alloc())
       @note IFX_Alloc also requires @ref IFX_TmInit_Startup() for the access to the current task ID
    */
//    IFX_Memory_Init();
		
		/* initialize IMM */
    IFX_Imm_InitStartup();
		
		/* initialize the interrupt handler required to handle IMM interrupts correctly before
       starting the devices. e.g. IFX_InterruptHandler_DispatchNvmReadCallback needs
       a correctly set programMarker */
    IFX_InterruptHandler_Init();
		
#if ( IFX_CFG_CLK == 1)
    /* Initialize CLK and Power module */
    IFX_Clk_Pwr_Init();
#endif

#if ( IFX_EARLY_INIT_SEQUENCE == 0 )
    /* initialize IMM */
    IFX_Imm_InitFull();
#endif /* IFX_EARLY_INIT_SEQUENCE == 0 */
		
    IFX_HslStartup_HslNvmDrvInit();
		
    HslNvm_InitSystemIpuInstance();
		
//    IfxAppNote_HslNvmIpu_SystemInstanceTearingCheck();
//		
//    IfxAppNote_HslNvmIpu_SystemInstanceTearingRepair();
}
/** @} */ // end of IFX_TASK_MAN_GROUP
/* Added by Infineon end */
