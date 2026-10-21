;/*****************************************************************************
; * @file:    startup_SLCx7_C40.inc
; * @purpose: This file contains SPAU and CPAU configuration on unprivileged (=user)
; *           level for different products.
; *           Configuration may be adjusted by modifying CONF_SPAU_USERDIS0,
; *           CONF_SPAU_USERDIS1 and CONF_CPAU_USERDIS0 below.
; *           Access is enabled for peripherals which are listed.
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
; * (C)Copyright INFINEON TECHNOLOGIES All rights reserved
; ******************************************************************************/


  IF :DEF:SLCx7V14 || :DEF:SLCx7V15
CONF_SPAU_USERDIS0 EQU (:NOT:(SPAU_USERDIS0_UART_Msk     :OR: \
                              SPAU_USERDIS0_PSU_Msk      :OR: \
                              SPAU_USERDIS0_CLK_PM_Msk   :OR: \
                              SPAU_USERDIS0_SYS_Msk      :OR: \
                              SPAU_USERDIS0_ANA_Msk      :OR: \
                              SPAU_USERDIS0_USCP_Msk     :OR: \
                              SPAU_USERDIS0_RNG_Msk))
CONF_SPAU_USERDIS1 EQU (:NOT:(SPAU_USERDIS1_TIM1_Msk     :OR: \
                              SPAU_USERDIS1_TIM0_Msk     :OR: \
                              SPAU_USERDIS1_RAM0MAIL_Msk :OR: \
                              SPAU_USERDIS1_RAM0USER_Msk :OR: \
                              SPAU_USERDIS1_GPIO0_Msk))
; Note: Access to NVM0 User memory is always allowed and is listed here only for consistency
CONF_CPAU_USERDIS0 EQU (:NOT:(CPAU_USERDIS0_NVM0USER_Msk :OR: \
                              CPAU_USERDIS0_NVM0SFR_Msk))
  ELIF  :DEF:SLCx7V11a || :DEF:SLCx7V16
CONF_SPAU_USERDIS0 EQU (:NOT:(SPAU_USERDIS0_I2C_Msk      :OR: \
                              SPAU_USERDIS0_RFI_Msk      :OR: \
                              SPAU_USERDIS0_UART_Msk     :OR: \
                              SPAU_USERDIS0_PSU_Msk      :OR: \
                              SPAU_USERDIS0_CLK_PM_Msk   :OR: \
                              SPAU_USERDIS0_SYS_Msk      :OR: \
                              SPAU_USERDIS0_ANA_Msk      :OR: \
                              SPAU_USERDIS0_RNG_Msk      :OR: \
                              SPAU_USERDIS0_CRC_Msk      :OR: \
                              SPAU_USERDIS0_SCP_Msk))
CONF_SPAU_USERDIS1 EQU (:NOT:(SPAU_USERDIS1_TIM2_Msk     :OR: \
                              SPAU_USERDIS1_TIM1_Msk     :OR: \
                              SPAU_USERDIS1_TIM0_Msk     :OR: \
                              SPAU_USERDIS1_RAM0MAIL_Msk :OR: \
                              SPAU_USERDIS1_RAM0USER_Msk :OR: \
                              SPAU_USERDIS1_GPIO0_Msk))
; Note: Access to NVM0 User memory is always allowed and is listed here only for consistency
CONF_CPAU_USERDIS0 EQU (:NOT:(CPAU_USERDIS0_ROM0_Msk     :OR: \
                              CPAU_USERDIS0_NVM0USER_Msk :OR: \
                              CPAU_USERDIS0_NVM0SFR_Msk))
  ELSE
    error unknown tool identifier
  ENDIF

  END
