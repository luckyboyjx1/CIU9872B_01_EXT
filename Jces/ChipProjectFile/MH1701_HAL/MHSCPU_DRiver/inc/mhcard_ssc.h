/**************************************************************************//**
 * @file     mhcard_ssc.h
 * @brief    
 * @version  V1.00
 * @date     6. April 2022
 *
 * @note
 *
 ******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
#ifndef __MHCARD_SSC_H
#define __MHCARD_SSC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"

/*******************SSC Struct Reference**************************/

typedef struct
{
      __IO uint32_t  RES0[(0x1C - 0) >> 2];//0x00~0x18
      __IO uint32_t  INT_STAT;//0x1C
      __IO uint32_t  ATT_CR;//0x20
      __IO uint32_t  PD_RNG_INI;//0x24
      __IO uint32_t  ST_EN;//0x28
      __IO uint32_t  ST_ISR;//0x2C
      __IO uint32_t  RES1[(0x98 - 0x2C) >> 2];//0x30~0x9C
      __IO uint32_t  VTOR_REMAP;//0x9C
      __IO uint32_t  DITHER;//0xA0
      __IO uint32_t  POL_CR;//0xA4
      __IO uint32_t  RES2;//0xA8
      __IO uint32_t  RES3;//0xAC
      __IO uint32_t  RES4;//0xB0
      __IO uint32_t  ST_END_FALG;//0xB4
      __IO uint32_t  RES5;//0xB8
      __IO uint32_t  RES6;//0xBC
      __IO uint32_t  IFLUSH_CR;//0xC0
      __IO uint32_t  GLITCH_CLR;//0xC4
} SSC_TypeDef;

#define SSC_BASE                            (MHCARD_APB_BASE + 0x3000)
#define SSC                                 ((SSC_TypeDef *)SSC_BASE)  

#define SSC_INT_STAT_IT_Mask                ((uint32_t)0x1FFFF)
                                       
/************************* BIT Defination of INT_STAT **********************************/
#define SSC_INT_STAT_BACKUP_INT_Mask         BIT(17)
#define SSC_INT_STAT_TRNG_INT_Mask           BIT(16)
#define SSC_INT_STAT_MESH_INT_Mask           BIT(15)
#define SSC_INT_STAT_LD_INT_Mask             BIT(14)
#define SSC_INT_STAT_RC_FDH_INT_Mask         BIT(13)
#define SSC_INT_STAT_RC_FDL_INT_Mask         BIT(12)
#define SSC_INT_STAT_ISO14443_FDH_INT_Mask   BIT(11)
#define SSC_INT_STAT_ISO14443_FDL_INT_Mask   BIT(10)
#define SSC_INT_STAT_ISO7816_FDH_INT_Mask    BIT(9)
#define SSC_INT_STAT_ISO7816_FDL_INT_Mask    BIT(8)
#define SSC_INT_STAT_TDH_INT_Mask            BIT(7)
#define SSC_INT_STAT_TDL_INT_Mask            BIT(6)
#define SSC_INT_STAT_GDE_INT_Mask            BIT(5)
#define SSC_INT_STAT_GDI_INT_Mask            BIT(4)
#define SSC_INT_STAT_VDIH_INT_Mask           BIT(3)
#define SSC_INT_STAT_VDIL_INT_Mask           BIT(2)
#define SSC_INT_STAT_VDEH_INT_Mask           BIT(1)
#define SSC_INT_STAT_VDEL_INT_Mask           BIT(0)
#define SSC_INT_STAT_ALL_Mask                ((uint32_t)0x0003FFFF)
#define IS_SSC_INT(INT)                      ((((INT) & ~SSC_INT_STAT_ALL_Mask) == 0x00) && ((INT) != 0x00))


/************************* BIT Defination of ATT_CR **********************************/
#define SSC_ATT_CR_VDEL     	            BIT(0)     /* External Low Voltage detectctor alarm */
#define SSC_ATT_CR_VDEH     	            BIT(1)     /* External High Voltage detectctor alarm */
#define SSC_ATT_CR_VDIL     	            BIT(2)      /* Internal Low Voltage detectctor alarm */
#define SSC_ATT_CR_VDIH     	            BIT(3)      /* Internal High Voltage detectctor alarm */
#define SSC_ATT_CR_GDI          	        BIT(4)      /* Internal Voltage Glitch Detector alarm */
#define SSC_ATT_CR_GDE              	    BIT(5)      /* External Voltage Glitch Detector alarm */ 
#define SSC_ATT_CR_TDL     	                BIT(6)      /* Low Temperature Detector alarm */   
#define SSC_ATT_CR_TDH           	        BIT(7)      /* High Temperature Detector alarm */
#define SSC_ATT_CR_7816_FDL                 BIT(8)      /* 7816 Clock Low Frequence Detector alarm */
#define SSC_ATT_CR_7816_FDH                 BIT(9)      /* 7816 Clock High Frequence Detector alarm */
#define SSC_ATT_CR_14443_FDL                BIT(10)      /* 14443 Clock Low Frequence Detector alarm */
#define SSC_ATT_CR_14443_FDH                BIT(11)      /* 14443 Clock High Frequence Detector alarm */
#define SSC_ATT_CR_RC_FDL                   BIT(12)      /* OSC Clock Low Frequence Detector alarm */
#define SSC_ATT_CR_RC_FDH                   BIT(13)      /* OSC Clock High Frequence Detector alarm */
#define SSC_ATT_CR_LD        		        BIT(14)      /* Light Detector alarm */
#define SSC_ATT_CR_SHELIDING                BIT(15)     /* active sheilding alarm */
#define SSC_ATT_CR_TRNG                     BIT(16)      /* TRNG alarm */  
#define SSC_ATT_CR_BACKUP_PAR               BIT(17)     /* backup register and parity check alarm */
#define SSC_ATT_CR_ALL                      ((uint32_t)0x3FFFF)
#define IS_SSC_ATT_CR(CR)                   (((CR) & ~SSC_ATT_CR_ALL) == 0 && ((CR) != 0x00))


/*******************BIT Defination of ST_EN *************************/
#define SSC_ST_EN_RNG_ANA_Mask              BIT(18)
#define SSC_ST_EN_RSV_Mask                  BIT(17)
#define SSC_ST_EN_TRNG_Mask                 BIT(16)
#define SSC_ST_EN_MASH_Mask                 BIT(15)
#define SSC_ST_EN_LD_Mask          		    BIT(14)
#define SSC_ST_EN_RC_FDH_Mask               BIT(13)
#define SSC_ST_EN_RC_FDL_Mask  		        BIT(12)
#define SSC_ST_EN_14443_FDH_Mask            BIT(11)
#define SSC_ST_EN_14443_FDL_Mask            BIT(10)
#define SSC_ST_EN_7816_FDH_Mask             BIT(9)
#define SSC_ST_EN_7816_FDL_Mask             BIT(8)
#define SSC_ST_EN_TDH_Mask                  BIT(7)
#define SSC_ST_EN_TDL_Mask         		    BIT(6)
#define SSC_ST_EN_GDE_Mask         		    BIT(5)
#define SSC_ST_EN_GDI_Mask         	        BIT(4)
#define SSC_ST_EN_VDIH_Mask                 BIT(3)
#define SSC_ST_EN_VDIL_Mask                 BIT(2)
#define SSC_ST_EN_VDEH_Mask                 BIT(1)
#define SSC_ST_EN_VDEL_Mask                 BIT(0)
#define SSC_ST_EN_ALL_Mask                  ((uint32_t)0x0005FFFF)
#define IS_SSC_ST_EN(ST_EN)                 ((((ST_EN) & ~SSC_ST_EN_ALL_Mask) == 0x00) && ((ST_EN) != 0x00))

/*******************BIT Defination of ST_ISR **********************/
#define SSC_ST_ISR_TRNG_Mask                BIT(16)
#define SSC_ST_ISR_MASH_Mask                BIT(15)
#define SSC_ST_ISR_LD_Mask                  BIT(14)
#define SSC_ST_ISR_RC_FDH_Mask              BIT(13)
#define SSC_ST_ISR_RC_FDL_Mask              BIT(12)
#define SSC_ST_ISR_14443_FDH_Mask           BIT(11)
#define SSC_ST_ISR_14443_FDL_Mask           BIT(10)
#define SSC_ST_ISR_7816_FDH_Mask            BIT(9)
#define SSC_ST_ISR_7816_FDL_Mask            BIT(8)
#define SSC_ST_ISR_TDH_Mask                 BIT(7)
#define SSC_ST_ISR_TDL_Mask                 BIT(6)
#define SSC_ST_ISR_GDE_Mask                 BIT(5)
#define SSC_ST_ISR_GDI_Mask                 BIT(4)
#define SSC_ST_ISR_VDIH_Mask                BIT(3)
#define SSC_ST_ISR_VDIL_Mask                BIT(2)
#define SSC_ST_ISR_VDEH_Mask                BIT(1)
#define SSC_ST_ISR_VDEL_Mask                BIT(0)
#define SSC_ST_ISR_ALL_Mask                 ((uint32_t)0x0001FFFF)
#define IS_SSC_ST_ISR(ST_ISR)               ((((ST_ISR) & ~SSC_ST_ISR_ALL_Mask) == 0x00) && ((ST_ISR) != 0x00))	


/*****************BIT Defination of ST_END_FALG*******************/
/*if 1,Bypass Enabled*/
#define SSC_ST_END_FALG               BIT(0)

/*****************BIT Defination of GLITCH_CLR*******************/
/*if 1,Bypass Enabled*/
#define SSC_GLITCH_CLR_GDE               BIT(1)
#define SSC_GLITCH_CLR_GDI               BIT(0)

/*****************BIT Defination End*************************/


#define SSC_DETECTOR_VDEL                (0x01UL << 0 )
#define SSC_DETECTOR_VDEH                (0x01UL << 1 )
#define SSC_DETECTOR_VDIL                (0x01UL << 2 )
#define SSC_DETECTOR_VDIH                (0x01UL << 3 )
#define SSC_DETECTOR_GDI                 (0x01UL << 4 )
#define SSC_DETECTOR_GDE                 (0x01UL << 5 )
#define SSC_DETECTOR_TDL                 (0x01UL << 6 )
#define SSC_DETECTOR_TDH                 (0x01UL << 7 )

#define SSC_DETECTOR_ISO7816_FDL         (0x01UL << 8 )
#define SSC_DETECTOR_ISO7816_FDH         (0x01UL << 9)
#define SSC_DETECTOR_ISO14443_FDL        (0x01UL << 10)
#define SSC_DETECTOR_ISO14443_FDH        (0x01UL << 11)
#define SSC_DETECTOR_RC_FDL              (0x01UL << 12)
#define SSC_DETECTOR_RC_FDH              (0x01UL << 13)

#define SSC_DETECTOR_LD                  (0x01UL << 14)
#define SSC_DETECTOR_MESH                (0x01UL << 15)
#define SSC_DETECTOR_TRNG                (0x01UL << 16)
#define SSC_DETECTOR_BACKUP              (0x01UL << 17)
#define SSC_DETECTOR_RNG                 (0x01UL << 18)
#define SSC_DETECTOR_ALL                 (0x0007FFFFUL)



typedef enum
{
	SSC_SENSOR_CPUReset  = 0,
	SSC_SENSOR_NoResponse = 1,		
	SSC_SENSOR_Interrupt = 3,
}SSC_SENSOR_RespModeTypeDef;
#define IS_SSC_SENSOR_RESP_MODE(Mode)				((Mode) == SSC_SENSOR_CPUReset  ||\
													 (Mode) == SSC_SENSOR_Interrupt ||\
													 (Mode) == SSC_SENSOR_Interrupt)	



/**
  * @method	SSC_GetITStatus
  * @brief	
  * @param	SSC_IT
  * @retval ITStatus
  */
ITStatus SSC_GetITStatus(uint32_t SSC_IT);

/**
  * @method	SSC_ClearITPendingBit
  * @brief	
  * @param	SSC_IT
  * @retval void
  */
void SSC_ClearITPendingBit(uint32_t SSC_IT);


/**
  * @method	SSC_SelfCheckConfig
  * @brief	
  * @param	
  * @param	
  * @retval void
  */
void SSC_SelfCheckConfig(uint32_t SSC_SelfCheck, FunctionalState NewState);

FlagStatus SSC_GetSelfCheckStatus(uint32_t SSC_SelfCheck);


void SSC_ClearSelfCheckStatus(uint32_t SSC_SelfCheck);


void SSC_SecureEnable(void);

void  SSC_SecureDisable(void);

void  SSC_IflushEnable(void);

void SSC_IflushDisable(void);


uint32_t SSC_GetEnable(uint32_t u32Det);
uint32_t SSC_GetAttack(uint32_t u32Det);
uint32_t SSC_ClearAttack(uint32_t u32Det);
uint32_t SSC_GetRespMode(uint32_t u32Det);
uint32_t SSC_GetSelfCheckEnable(uint32_t u32Det);
uint32_t SSC_SetSelfCheckEnable(uint32_t u32Det);
uint32_t SSC_GetSelfCheckResult(uint32_t u32Det);
uint32_t SSC_ClearSelfCheckResult(uint32_t u32Det);



#ifdef __cplusplus
}
#endif

#endif 

/**************************      (C) COPYRIGHT    *****END OF FILE****/
