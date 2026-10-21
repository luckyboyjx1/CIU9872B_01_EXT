;/*****************************************************************************
; * @file:    startup_SLCx7_C40.s
; * @purpose: Device Startup File for Infineon SLx1/SLx3 C40 Device Series
; * @version: V0.0.1
; *
; * Based on startup_ARMCM3.s
; * CMSIS Core Device Startup File for ARMCM3 Device
; * V5.3.1, 09. July 2018
; *
; * Modifications for Infineon security controllers
; * marked by "Added by Infineon" below
; *
; * THE SOURCE CODE AND ITS RELATED DOCUMENTATION IS PROVIDED "AS IS". INFINEON
; * TECHNOLOGIES MAKES NO OTHER WARRANTY OF ANY KIND,WHETHER EXPRESS,IMPLIED OR,
; * STATUTORY AND DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
; * SATISFACTORY QUALITY, NON INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
; *
; * THE SOURCE CODE AND DOCUMENTATION MAY INCLUDE ERRORS. INFINEON TECHNOLOGIES
; * RESERVES THE RIGHT TO INCORPORATE MODIFICATIONS TO THE SOURCE CODE IN LATER
; * REVISIONS OF IT, AND TO MAKE IMPROVEMENTS OR CHANGES IN THE DOCUMENTATION OR
; * THE PRODUCTS OR TECHNOLOGIES DESCRIBED THEREIN AT ANY TIME.
; *
; * INFINEON TECHNOLOGIES SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
; * CONSEQUENTIAL DAMAGE OR LIABILITY ARISING FROM YOUR USE OF THE SOURCE CODE OR
; * ANY DOCUMENTATION, INCLUDING BUT NOT LIMITED TO, LOST REVENUES, DATA OR
; * PROFITS, DAMAGES OF ANY SPECIAL, INCIDENTAL OR CONSEQUENTIAL NATURE, PUNITIVE
; * DAMAGES, LOSS OF PROPERTY OR LOSS OF PROFITS ARISING OUT OF OR IN CONNECTION
; * WITH THIS AGREEMENT, OR BEING UNUSABLE, EVEN IF ADVISED OF THE POSSIBILITY OR
; * PROBABILITY OF SUCH DAMAGES AND WHETHER A CLAIM FOR SUCH DAMAGE IS BASED UPON
; * WARRANTY, CONTRACT, TORT, NEGLIGENCE OR OTHERWISE.
; *
; * (C) COPYRIGHT 2020 INFINEON TECHNOLOGIES AG
; * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY INFORMATION OF INFINEON
; * TECHNOLOGIES AG. All rights reserved.
; */
;/*****************************************************************************/
;/*
; * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */
;/*****************************************************************************/

; /* Added by Infineon */
; /* Include register definitions */
                INCLUDE     reg_slc37.inc
                INCLUDE     IFX_ISS.inc
                INCLUDE     startup_SLCx7_C40.inc
; /* Added by Infineon end */

Stack_Size      EQU     0x00001000

                AREA    ifx_user_stacks, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size ;
; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
                PRESERVE8                         ; 8 Byte allignment of the Stack
                THUMB                             ; interpret subsequent instructions as Thumb instructions                   
                    

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors

; /* Added by Infineon */
                IMPORT  ||Image$$ER_AN_PRIV_STACK$$Limit||

__Vectors       DCD     ||Image$$ER_AN_PRIV_STACK$$Limit||        ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     Startup_Trap_Handler      ; NMI Handler
                DCD     Startup_Trap_Handler      ; Hard Fault Handler
                DCD     Startup_Trap_Handler      ; MPU Fault Handler
                DCD     Startup_Trap_Handler      ; Bus Fault Handler
                DCD     Startup_Trap_Handler      ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     Startup_Trap_Handler      ; PendSV Handler
                DCD     Startup_Trap_Handler      ; SysTick Handler

                ; External Interrupts
                DCD     Startup_Trap_Handler      ; 0:  UART Interrupt
                DCD     RNG_IRQHandler            ; 1:  RNG Interrupt
                DCD     Startup_Trap_Handler      ; 2:  Timer 0 Interrupt
                DCD     Startup_Trap_Handler      ; 3:  Timer 1 Interrupt
                DCD     Startup_Trap_Handler      ; 4:  Timer 2 Interrupt
                DCD     Startup_Trap_Handler      ; 5:  Reserved
                DCD     Startup_Trap_Handler      ; 6:  IMM Interrupt
                DCD     Startup_Trap_Handler      ; 7:  SYSTEM Interrupt
                DCD     Startup_Trap_Handler      ; 8:  I2C Interrupt
                DCD     Startup_Trap_Handler      ; 9:  SCP Interrupt
                DCD     Startup_Trap_Handler      ; 10: Crypto@2304 Interrupt
                DCD     Startup_Trap_Handler      ; 11: Reserved
                DCD     Startup_Trap_Handler      ; 12: Reserved
                DCD     Startup_Trap_Handler      ; 13: Reserved
                DCD     Startup_Trap_Handler      ; 14: GPIO Interrupt
                DCD     Startup_Trap_Handler      ; 15: RFI Interrupt
__VectorsEnd
                DCD     0                         ; Enforce alignment to 32 entries



                AREA    |ifx_ram_vectors|, DATA, READONLY
                EXPORT  __RamVectors

; Note: Depending on the product, some of the external interrupts listed below may not be available. Please refer
;       to the product documentation.
__RamVectors    DCD     0                         ; Top of Stack
                DCD     0                         ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     UART_IRQHandler           ; 0:  UART Interrupt
                DCD     RNG_IRQHandler            ; 1:  RNG Interrupt
                DCD     TIM0_IRQHandler           ; 2:  Timer 0 Interrupt
                DCD     TIM1_IRQHandler           ; 3:  Timer 1 Interrupt
                DCD     TIM2_IRQHandler           ; 4:  Timer 2 Interrupt
                DCD     0                         ; 5:  Reserved
                DCD     IMM_IRQHandler            ; 6:  IMM Interrupt
                DCD     SYSTEM_IRQHandler         ; 7:  SYSTEM Interrupt
                DCD     I2C_IRQHandler            ; 8:  I2C Interrupt
                DCD     SCP_IRQHandler            ; 9:  SCP Interrupt
                DCD     CRYPTO_IRQHandler         ; 10: Crypto@2304 Interrupt
                DCD     0                         ; 11: Reserved
                DCD     0                         ; 12: Reserved
                DCD     0                         ; 13: Reserved
                DCD     GPIO_IRQHandler           ; 14: GPIO Interrupt
                DCD     RFI_IRQHandler            ; 15: RFI Interrupt
__RamVectorsEnd
                DCD     0                         ; Enforce alignment to 32 entries        

                AREA    |ifx_startup_code|, CODE, READONLY

                IMPORT  MpuStartupRegionDescriptorArray
                IMPORT  IFX_ISS_Start
                IMPORT  IFX_ISS_Stop

__sec_reset
                DCI.W 0x44FF44FF
; /* Added by Infineon end */
; Reset Handler
Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit

; /* Added by Infineon */
                ; Handle MPU immediately after transition from BOS to FL
                ; Put Module To Config State
                LDR     R0, =MPU_OCC
                LDR     R1, [R0]
                MVN     R1, R1
                STR     R1, [R0]
                ; Disable Mpu
                LDR     R2, =MPU_CTRL
                MOV     R1, #0x0
                STR     R1, [R2]
                ; Setup the regions
                LDR     R1, =MPU_RBAR
                ; Get reference to MPU startup configuration data
                LDR     R0, =MpuStartupRegionDescriptorArray
                ; Make RAM accessible rw for priv and user + make ROM+NVM accessible RW for priv, R for user, make IO area RW for all
                LDM     R0!,{R2-R9}       ; load MPU descriptor 0..3
                STM     R1,{R2-R9}       ; MPU alias can max load 4 descriptors at once
                LDM     R0!,{R2-R7}       ; load MPU descriptor 4..6, note, that MPU region 7 is already set @ Reset_Handler
                STM     R1,{R2-R7}     ; write remaining descriptor in a second STM
                ; Enable Mpu
                LDR     R2, =MPU_CTRL
                MOV     R1, #MPU_CTRL_ENABLE_Msk
                STR     R1, [R2]
                ; Put Module To Operation State
                LDR     R0, =MPU_OCC
                LDR     R1, [R0]
                MVN     R1, R1
                STR     R1, [R0]
                
                ;
                ; Configure SPAU
                ;
                
                ; Load SPAU base address to R0
                LDR     R0, =SPAU_USERDIS0
                
                ; Put module to configuration mode (invert OCC)
                LDR     R1, [R0,#SPAU_OCC-SPAU_USERDIS0]
                MVN     R1, R1
                STR     R1, [R0,#SPAU_OCC-SPAU_USERDIS0]
                
                ; Enable all available peripherals in PRIVILEGED mode
                ; ~(SPAU_AVAIL0) -> SPAU_PRIVDIS0
                LDR     R1, [R0,#SPAU_AVAIL0-SPAU_USERDIS0]
                MVN     R1, R1
                STR     R1, [R0,#SPAU_PRIVDIS0-SPAU_USERDIS0]
                ; ~(SPAU_AVAIL1) -> SPAU_PRIVDIS1
                LDR     R1, [R0,#SPAU_AVAIL1-SPAU_USERDIS0]
                MVN     R1, R1
                STR     R1, [R0,#SPAU_PRIVDIS1-SPAU_USERDIS0]
                
                ; Enable selected peripherals in USER mode
                ; SPAU_USERDIS0
                LDR     R1, = CONF_SPAU_USERDIS0
                STR     R1,  [R0,#SPAU_USERDIS0-SPAU_USERDIS0]
                ; SPAU_USERDIS1
                LDR     R1, = CONF_SPAU_USERDIS1
                STR     R1,  [R0,#SPAU_USERDIS1-SPAU_USERDIS0]

                ; Put module to operation mode (invert OCC)
                LDR     R1, [R0,#SPAU_OCC-SPAU_USERDIS0]
                MVN     R1, R1
                STR     R1, [R0,#SPAU_OCC-SPAU_USERDIS0]
                
                ; End of SPAU configuration

    ;            BL      IFX_ISS_Stop          ; check flow control

                ;
                ; Configure CPAU
                ;
                
                ; Load CPAU base address to R2
                LDR     R2, =CPAU_USERDIS0
                ; Put module to configuration mode (invert OCC)
                LDR     R1, [R2, #CPAU_OCC-CPAU_USERDIS0]
                MVN     R1, R1
                STR     R1, [R2, #CPAU_OCC-CPAU_USERDIS0]
                
                ; Configure user access
                LDR     R3, =CONF_CPAU_USERDIS0
                STR     R3, [R2, #CPAU_USERDIS0-CPAU_USERDIS0]
                
                ; Put module to operation mode (invert OCC)
                LDR     R1, [R2, #CPAU_OCC-CPAU_USERDIS0]
                MVN     R1, R1
                STR     R1, [R2, #CPAU_OCC-CPAU_USERDIS0]

                ; End of CPAU configuration

                LDR     R4, =SystemInit
                BLX     R4

                ; To remove default library's scatterload being linked automatically, do not import the symbol '__main'
                ; Instead define your own __main 

;                IMPORT  IFX_AppNotesInit

                EXPORT  __main
__main
                MOV     R0, #-1                         ; initialize the exceptionflag with -1
                LDR     R1, =ExceptionFlag              ; unexpected exceptions will end in a loop forever
                STR     R0, [R1]

                ; Initialize runtime environment, e.g. resource management
;                LDR     R0, =IFX_AppNotesInit
;                BLX     R0
                ; IFX_AppNotesInit returns in R0 the TCB to be used for 1st task

                ; Restore main stack for handler mode
                LDR     R2, =__Vectors                  ; set  MSP for handler mode
                LDR     R13, [R2,#0]                    ; get default startup stack
				
                ; Goto application
                ; R0 shall still hold a reference to a valid TCB indicating the 1st task
                MOV     R2,#1                           ; set ACTRL.DISMCYCINT bit to disable interruption of LDM / STM instructions
                LDR     R3, =ACTLR
                STR     R2,[R3]
;                LDR     R2, [R0,#0]                     ; set  PSP for main() , R0 shall have reference to SupervisorTaskControlBlock
;                MSR     PSP, R2                         ; set  PSP
;                LDR     R3, [R0,#36]                    ; Get reference to platform specific security context , R0 shall have reference to task control block of startup task, e.g. SupervisorTaskControlBlock
;                LDRB    R2, [R3,#0]                     ; Get type of thread mode from security context
;                ORR     R2, #2                          ; add usage of PSP for task as  mandatory in this IFX AppNote framework
;                MSR     CONTROL, R2                     ; from here on code execution continues with PSP, but still in privileged mode
;                ISB                                     ; Instruction Synchronisation Barrier 
;                LDR     R0,[R0,#4]                      ; get TaskDescriptor: SupervisorTaskDescriptor
;                LDR     LR,[R0,#0]                      ; get task start address : SupervisorTaskDescriptor.TaskStartAddress
;                LDR     R0,[R0,#4]                      ; get auxiliary parameter : SupervisorTaskDescriptor.AuxParam
;                BX      LR
;                ENDP

				IMPORT	os_main
				LDR     R0, =os_main
                BLX     R0	
				ENDP

; Dummy handler for unexpected interrupts/traps during startup
Startup_Trap_Handler    PROC
                B       BSOD
                ENDP

                EXPORT  DebugMon_Handler          [WEAK]
                EXPORT  NMI_Handler               [WEAK]
                EXPORT  HardFault_Handler         [WEAK]
                EXPORT  MemManage_Handler         [WEAK]
                EXPORT  BusFault_Handler          [WEAK]
                EXPORT  UsageFault_Handler        [WEAK]
                EXPORT  SVC_Handler               [WEAK]
                EXPORT  PendSV_Handler            [WEAK]
                EXPORT  SysTick_Handler           [WEAK]
                EXPORT  NrgPriv                   [WEAK]

DebugMon_Handler        PROC
                B       .
                ENDP

; The following weak functions execute Default_Exception_Handler
NMI_Handler             PROC
                        ENDP
HardFault_Handler       PROC
                        ENDP
MemManage_Handler       PROC
                        ENDP
BusFault_Handler        PROC
                        ENDP
UsageFault_Handler      PROC
                        ENDP
SVC_Handler             PROC
                        ENDP
PendSV_Handler          PROC
                        ENDP
SysTick_Handler         PROC
                        ENDP

; In case NRG is not included in build of these IFX_AppNotes, this label will provide a weak symbol to help the linker to resolve the symbol
NrgPriv                 PROC
                        ENDP

                IMPORT  ExceptionFlag
ExceptionAllowed  EQU 0x00
ExceptionOccurred EQU 0x02
; default exception handler stops execution with a loop forever.
;
; In case of Mpu-Driver function IFX_Mpu_ReadMem or IFX_Mpu_WriteMem 
; causes the exception, the handler stores the exception number in 
; the global variable ExceptionFlag    and add 4 to the return address. 
; The return from the exception continues on the next instruction. 
; The mpu driver function can evaluate the exceptionFlag variable and 
; return this information to the caller.  
Default_Exception_Handler PROC

                ;; EXC_RETURN    Description
                ;; ---------------------------------------------------
                ;; 0xFFFFFFF1    Return to Handler mode.
                ;;               Exception return gets state from the main stack.
                ;;               Execution uses MSP after return.
                ;; 0xFFFFFFF9    Return to Thread mode.
                ;;               Exception Return get state from the main stack.
                ;;               Execution uses MSP after return.
                ;; 0xFFFFFFFD    Return to Thread mode.
                ;;               Exception return gets state from the process stack.
                ;;               Execution uses PSP after return.                
                TST     LR, #0x04               ; test return code for stack being used
                MRSEQ   R1, MSP                 ; get main stack pointer
                MRSNE   R1, PSP                 ; get user stack pointer if LR != 0xfffffff9
                                                
                MRS     R0, IPSR
                LDR     R3,    =ExceptionFlag        
                LDRH    R2, [R3,#ExceptionAllowed]    ; get flag of expected exption
                CMP     R2, R0
                BNE     BSOD                    ; is exception expected ? No, goto haning loop, alternatively trigger a security reset

                MOV     R2, #-1                 ; switch off exception handling
                STRH    R0, [R3,#ExceptionOccurred]    ; signal the current IPSR to user's owned ExceptionFlag.ExceptionOccurred
                STRH    R2, [R3,#ExceptionAllowed]    ; signal the current IPSR to user's owned ExceptionFlag.ExceptionOccurred
                                                
                ;  alternative exceptionhandler for MPU-Test
                LDR     R0, [R1, #24]           ; get return address from Stack
                LDRH    R2, [R0]                ; get the op code causing the exception
                AND     R2, R2, #0xf800         ; get the highest bits of the opcode to check if 16 or 32 bit instruction caused the expection
                CMP     R2, #0xe000         
                ADDLS   R0, #2                  ; calculate return address to skip the 16 bit instruction
                ADDHI   R0, #4                  ; calculate return address to skip the 32 bit instruction
                STR     R0, [R1, #24]           ; manifest skipping
                BX      LR                      ; do skip instruction !
                ENDP

                EXPORT BSOD
BSOD
                STMFD   R1,{R4-R11}                ; save all upper context registers to the current MSP

; /** @brief collects the exception information from the various registers
;  **
;  ** registers Rx are holding
;  ** - R0  exception / IRQ number from IPSR - Interrupt Program Status Register
;  ** - R1  stack pointer when exception occurred
;  ** - R2  PC of instruction when exception occurred
;  ** - R3  Fault Address if Memory or Bus fault occurred
;  ** - R4  Configurable Fault Status Register with the three parts MemManage, Bus & Usage Fault Status Register
;  ** - R5  HardFault Status Register
;  **/
                LDR     R2, [R1, #24]           ; get return address from Stack
                LDR     R12, =CPUID             ; get Configurable Fault Status Register at once, holding MemManage, Bus & Usage Fault Status Register
                LDR     R4,[R12,#CFSR-CPUID]    ; get CFSR content

                MOV     R3, #0
                TST     R4, #0x80               ; check bit 7: MMARVALID
                LDRNE   R3, [R12,#MMFAR-CPUID]  ; get MemManage Fault Address Register 

                TST     R4, #0x8000             ; check bit 15: BARVALID
                LDRNE   R3, [R12,#BFAR-CPUID]   ; get Bus Fault Address Register 

                LDR     R5, [R12,#HFSR-CPUID]   ; get  HardFault Status Register
                MOV     R6, LR                  ; save execption return code at R6
;; option: dump collected data to any interface, e.g. UART                
                LDR     R12, =IOBUF             ; get a transmit peripheral base address, e.g. ISO
                BL      WAIT_IO_IDLE            ; avoid collisions on line
                REV16   R0,R0                   ; send IPSR in big endian for readability
                BL      SEND_HALF
                REV     R0, R1                  ; send SP in big endian
                BL      SEND_WORD
                REV     R0, R2                  ; send PC in big endian
                BL      SEND_WORD
                REV     R0, R3                  ; send fault address in big endian
                BL      SEND_WORD
                REV     R0, R4                  ; send CFSR content in big endian
                BL      SEND_WORD
                REV     R0, R5                  ; send HFSR content in big endian
                BL      SEND_WORD
                REV     R0, R6                  ; send Exception Return Code in big endian
                BL      SEND_WORD
;; option: send stack data of exception                
                SUB     R1, #32                 ; collect and send also previously saved registers R4-R11
                MOV     R2, #128/4              ; setup a word counter
SEND_CALL_STACK
                CBZ     R2, WAIT_DATA_SENT
                LDM     R1!, {R0}
                REV     R0, R0                  ; send stack content in big endian
                BL      SEND_WORD               ; 
                SUB     R2,#1                   ; decrement loopcounter
                B       SEND_CALL_STACK
                
WAIT_DATA_SENT
                LDRB    R10,[R12,#0x18]         ; IOSTAT: get TX status
                AND     R10, R10,#3             ; get TDATA & TBUSY
                CMP     R10, #0                 ; FIFO emty & shift register empty ?
                BNE     WAIT_DATA_SENT

;               BKPT    0x00                    ; stop execution here on emulator
                B       .                       ; Loop forever
                B       .                       ; Loop forever
                DCI.W 0x44FF44FF
                
                
WAIT_IO_IDLE    PROC
                MOV     R11,#100
WAIT_IO_IDLE_LOOP
                LDRB    R10,[R12,#0x18]         ; IOSTAT: get RX status
                TST     R10, #0x04              ; get TDATA & TBUSY
                BNE     WAIT_IO_IDLE_LOOP
                TST     R10, #0x08              ; get TDATA & TBUSY
                LDRBNE  R10,[R12,#0x00]         ; IOBUF: read Byte if RDATA set
                SUBS    R11,#1
                BNE     WAIT_IO_IDLE_LOOP
                MOV     R11, #1
                STRH    R11,[R12,#0x14]         ; IOMOD: force UART enable
                
                BX      LR
                ENDP

SEND_WORD       PROC
                LDRB    R10,[R12,#0xc]          ; get TX FIFO capacity
                CMP     R10, #4                 ; at least 2 Bytes free
                BMI     SEND_WORD
                STRH    R0,[R12,#0x04]          ; write lower half
                LSR     R0, R0, #16             ; get upper half word
                STRH    R0,[R12,#0x04]          ; write half
                BX      LR                      ; return to caller
                
                ; intended fall thru
                ENDP
SEND_HALF       PROC
                LDRB    R10,[R12,#0xc]          ; get TX FIFO capacity
                CMP     R10, #2                 ; at least 2 Bytes free
                BMI     SEND_HALF
                STRH    R0,[R12,#0x04]          ; write half
                BX      LR                      ; return to caller
                ENDP
                ALIGN 

Default_Handler PROC

                EXPORT  UART_IRQHandler           [WEAK]
                EXPORT  RNG_IRQHandler            [WEAK]
                EXPORT  TIM0_IRQHandler           [WEAK]
                EXPORT  TIM1_IRQHandler           [WEAK]
                EXPORT  TIM2_IRQHandler           [WEAK]
                EXPORT  IMM_IRQHandler            [WEAK]
                EXPORT  SYSTEM_IRQHandler         [WEAK]
                EXPORT  I2C_IRQHandler            [WEAK]
                EXPORT  SCP_IRQHandler            [WEAK]
                EXPORT  CRYPTO_IRQHandler         [WEAK]
                EXPORT  GPIO_IRQHandler           [WEAK]
                EXPORT  RFI_IRQHandler            [WEAK]

UART_IRQHandler
RNG_IRQHandler
TIM0_IRQHandler
TIM1_IRQHandler
TIM2_IRQHandler
IMM_IRQHandler
SYSTEM_IRQHandler
I2C_IRQHandler
SCP_IRQHandler
CRYPTO_IRQHandler
GPIO_IRQHandler
RFI_IRQHandler

                B       .                       ; Infinite loop

                ENDP

                ALIGN

                END
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; /* Added by Infineon end */

