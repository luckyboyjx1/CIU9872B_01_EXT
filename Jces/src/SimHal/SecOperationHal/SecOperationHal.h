/*
 * SecOperationHal.h
 *
 *  Created on: 2014Äê11ÔÂ26ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef SECOPERATIONHAL_H_
#define SECOPERATIONHAL_H_

#include "halCryptoProfile.h"
#include "SysConfigProfile.h"
#include "ecpDefine.h"
//#include "cardManager.h"

/* Error codes */
#define ECP_ERR_OK              0xA95C
#define ECP_ERR_FAILURE         0xCA8E


extern PM8						X_A		g_RSA_IV;
extern u8						X_A		g_keyTureByteLen;
extern u16                      X_A     g_CodeFlowControlFlag;

typedef struct
{
	PX8 dataBuff;
	u16 Len;
	u16 maskType;
	u8 maskValue;
	u8 checksum;
} SC_DataMaskParaStruct;

typedef struct
{
	PM8 pbParmPQ;
	PM8 pbParmDP1;
	PM8 pbParmDQ1;
	PM8 pbParmInvQ;
	u8 dPLength;
	u8 dQLength;
	u8 QinvLength;
} RSACRTKeyDPQRefStore;

extern RSACRTKeyDPQRefStore rsaCRTKeydPQRef;

#define DATA_EN_DE_MASK                             0x00FF
#define DATA_TYPE_MASK                              0xFF00


#define DATA_TYPE_KEYSET                            0xA500
#define DATA_TYPE_PIN                               0x5A00
#define DATA_TYPE_DES                               0xC300
#define DATA_TYPE_RSA                               0x3C00

#define DATA_MASK_ENC                               0x005A
#define DATA_MASK_DEC                               0x00A5

#define SEC_STEP_COUNT_0      0x00000000
#define SEC_STEP_COUNT_1      0x00000001
#define SEC_STEP_COUNT_2      0x00000003
#define SEC_STEP_COUNT_3      0x00000007
#define SEC_STEP_COUNT_4      0x0000000F
#define SEC_STEP_COUNT_5      0x0000001F
#define SEC_STEP_COUNT_6      0x0000003F
#define SEC_STEP_COUNT_7      0x0000007F
#define SEC_STEP_COUNT_8      0x000000FF
#define SEC_STEP_COUNT_9      0x000001FF
#define SEC_STEP_COUNT_10     0x000003FF
#define SEC_STEP_COUNT_11     0x000007FF
#define SEC_STEP_COUNT_12     0x00000FFF
#define SEC_STEP_COUNT_13     0x00001FFF
#define SEC_STEP_COUNT_14     0x00003FFF
#define SEC_STEP_COUNT_15     0x00007FFF
#define SEC_STEP_COUNT_16     0x0000FFFF
#define SEC_STEP_COUNT_17     0x0001FFFF
#define SEC_STEP_COUNT_18     0x0003FFFF
#define SEC_STEP_COUNT_19     0x0007FFFF
#define SEC_STEP_COUNT_20     0x000FFFFF
#define SEC_STEP_COUNT_21     0x001FFFFF
#define SEC_STEP_COUNT_22     0x003FFFFF
#define SEC_STEP_COUNT_23     0x007FFFFF
#define SEC_STEP_COUNT_24     0x00FFFFFF
#define SEC_STEP_COUNT_25     0x01FFFFFF
#define SEC_STEP_COUNT_26     0x03FFFFFF
#define SEC_STEP_COUNT_27     0x07FFFFFF
#define SEC_STEP_COUNT_28     0x0FFFFFFF
#define SEC_STEP_COUNT_29     0x1FFFFFFF
#define SEC_STEP_COUNT_30     0x3FFFFFFF
#define SEC_STEP_COUNT_31     0xFFFFFFFF

#define WITH_HAL_SAFETY								0

#if WITH_HAL_SAFETY
	#if CODE_SIGNATURE
		#define SC_startCodeDataSignature(config)





		#define SC_stopCodeSignature()
		#define SC_disableCodeSignature()
		#define SC_checkGCodeDataSignature(cValue)

		#define SC_checkLCodeDataSignature(cValue)
	#else
		#define SC_startCodeDataSignature(config)
		#define SC_stopCodeSignature()
		#define SC_disableCodeSignature()
		#define SC_checkGCodeDataSignature(cValue)
		#define SC_checkLCodeDataSignature(cValue)
	#endif


	//bit0: JSCR_CTRL_JSCR_ACTIVATE_BIT       JSCR activation
	//bit1: JSCR_CTRL_JSCR_JAM_ENABLE_BIT	  Jamming enable
	//bit2: JSCR_CTRL_JSCR_SMT_CONFIG_BIT     Current Smoothing Configuration    0-8.5mA  1-17.5mA
	#define SC_activeJSCR()
	#define SC_checkActiveOfJSCR()
	#define SC_terminateCard()	    setCardState(g_bISDId[selected_channel], CARD_TERMINATED);
	#define SC_checkActiveShield()
	#define SC_checkLaserConfig()
	#define SC_activeLaser()
	#define SC_activeSensor()
	#define SC_checkSensorConfig()
	#define SC_activeFlushCPUprefetchBuffer()
	#define SC_deactiveFlushCPUprefetchBuffer()
	#define SC_checkActiveOfPrefetchBuffer()
	#define SC_checkSecIO()
	#define SC_checkClkConfig()
	#define SC_checkSecCtrlReg()
	#define SC_checkNesCLKConfig()
	#define SC_activeNVMParityCheck()
	#define SC_deactiveNVMParityCheck()
	extern void SC_laserTest(void);
	#define	SC_NVMParityIsActive(void)
#else
	#define SC_startCodeDataSignature(config)
	#define SC_stopCodeSignature(void)
	#define SC_disableCodeSignature(void)
	#define SC_checkGCodeDataSignature(cValue)
	#define SC_checkLCodeDataSignature(cValue)
	#define SC_disEDESPowerSaveMode(void)
	#define SC_disNESPowerSaveMode(void)
	#define SC_activeEDESPowerSaveMode(void)
	#define SC_activeNESPowerSaveMode(void)
	#define	SC_activeJSCR(void)
	#define SC_checkActiveOfJSCR(void)
	#define SC_checkActiveShield(void)
	#define SC_checkLaserConfig(void)
	#define SC_activeLaser(void)
	#define SC_activeSensor(void)
	#define SC_checkSensorConfig(void)
	#define	SC_activeFlushCPUprefetchBuffer(void)
	#define	SC_deactiveFlushCPUprefetchBuffer(void)
	#define SC_checkActiveOfPrefetchBuffer(void)
	#define SC_checkSecIO(void)
	#define SC_checkClkConfig(void)
	#define SC_checkSecCtrlReg(void)
	#define	SC_checkNesCLKConfig(void)
	#define SC_activeNVMParityCheck()
	#define SC_deactiveNVMParityCheck()
	#define	SC_laserTest(void)
	#define SC_terminateCard(void)
	#define	SC_NVMParityIsActive(void)			true
#endif

#if L_COUNTER_CODE_FLOW_CTRL
	#define SC_localCodeFlagInit(v,initValue)     v = initValue;
	#define SC_localCodeFlagInc(v)                v++;
	#define SC_localCodeFlagSet(v,t)              v = t;
	#define SC_localCodeFlagCheck(v,t)            if(v != t)\
												  {\
													faultDetected();\
											      }
#else
	#define SC_localCodeFlagInit(v,initValue)
	#define SC_localCodeFlagInc(v)
	#define SC_localCodeFlagSet(v,t)
	#define SC_localCodeFlagCheck(v,t)
#endif

#if G_COUNTER_CODE_FLOW_CTRL
	#if RAM_COUNTER
		extern u32 g_sensitiveState;
		#define SC_codeFlowRamCnterInit()			g_sensitiveState = SEC_STEP_COUNT_0;
		#define SC_codeFlowRamSetCnter(value)		g_sensitiveState = value;
		#define SC_codeFlowRamCnterCheck(target)	if(g_sensitiveState != target)\
													{\
														faultDetected();\
													}
	#endif
	#if HARDWARE_COUNTER
		#define SC_codeFlowCnterInit()			r_SEC_STEP_COUNT_CLEAR_EN_BIT(SEC_STEP_COUNT_CLEAR_EN_BIT) = 1;\
												r_SEC_STEP_COUNT_INC = r_SEC_STEP_COUNT_INC;
		#define SC_codeFlowSetCnter(value)		r_SEC_STEP_COUNT_INC = value;
		#define SC_codeFlowCnterCheck(target)	if(r_SEC_STEP_COUNT_INC != target)\
												{\
													faultDetected();\
												}
	#endif
#else
	#define SC_codeFlowCnterInit()
	#define SC_codeFlowSetCnter(value)
	#define SC_codeFlowCnterCheck(target)
	#define SC_codeFlowRamCnterInit()
	#define SC_codeFlowRamSetCnter(value)
	#define SC_codeFlowRamCnterCheck(target)
#endif


#if G_COUNTER_CODE_FLOW_CTRL
extern void checkCodeFlowControlFlag(u8 algType);
#define initCodeFlowControlFlag()					(g_CodeFlowControlFlag = 0)
#define incrementCodeFlowControlFlag()				(g_CodeFlowControlFlag++)
#define decrementCodeFlowControlFlag()				(g_CodeFlowControlFlag--)
#else
#define checkCodeFlowControlFlag(algType)			algType
#define initCodeFlowControlFlag()
#define incrementCodeFlowControlFlag()
#define decrementCodeFlowControlFlag()


#endif

#if PARAMETER_INTEGRITY_CHECK
	#define SC_parameterIntegrityCheck(paraStruct)       	if(U32t8(SC_calChecksum((PX32)&paraStruct, (sizeof(paraStruct)) - 1)) != paraStruct.checksum)\
															{\
																faultDetected();\
															}
	#define SC_getParameterChecksum(paraStruct)         	paraStruct.checksum = U32t8(SC_calChecksum((PX32)&paraStruct, (sizeof(paraStruct)) - 1));
#else
	#define SC_parameterIntegrityCheck(paraStruct)
	#define SC_getParameterChecksum(paraStruct)
#endif

#if RNG_OUT_BUFF_CHECK
	#define SC_initRngResult(buff,len)			memWriteU8NoAtomic(buff, 0xFF);memWriteU8NoAtomic(&buff[1], 0xFF);
	#define SC_checkRngResult(buff,len)        	if((memReadU8(buff) == 0xFF) && (memReadU8(&buff[1]) == 0xFF))\
												{\
													faultDetected();\
												}
#else
	#define SC_initRngResult(buff,len)
	#define SC_checkRngResult(buff,len)
#endif

#if DOUBLE_CHECK
	#define SC_doubleCheckEqual(a,b)                 if((a ^ b) != 0)\
													 {\
														faultDetected();\
													 }
												
#else
	#define SC_doubleCheckEqual(a,b)
#endif

#if RANDOM_DELAY_ENABLE
	extern void randomDelay(void);
#else
	#define randomDelay()
#endif
#if	SC_EDC_CHECK_SWITCH_ON
extern u8 edcFlag;
extern void SC_deactiveEDC(void);
extern void SC_activeEDC(void);
#else
#define 	SC_deactiveEDC()
#define 	SC_activeEDC()													 
#endif													 
extern void reset_card(void);
																					 
#if WITH_SECURE_DATA_STORAGE
   #if RUN_IN_SIMULATOR
     #define SC_calChecksum( paraAddress,  paraSize)			0
  #else
      extern u32 SC_calChecksum(PX32 paraAddress, u32 paraSize);
   #endif
#else
	#define SC_calChecksum( paraAddress,  paraSize)			0
#endif


#if ATTACK_COUNTER_AUDIT

	extern u16 checkAttackCtr(void);
#else
	#define checkAttackCtr()    ECP_ERR_OK
#endif


	extern u16 SC_DataMask(SC_DataMaskParaStruct dataMaskPara);
	extern void SC_KEYSETUnMask(PM8 keyData, PM8 outBuffer);
	extern void SetupShares(u16 num_bytes, u8* plain_bytes, u8* share_1, u8* share_2);
	//#define	 SetupShares( num_bytes, plain_bytes,  share_1,  share_2)
	extern u16 SC_rsaKeyMask(PX8 result, PM8 d1_Addr,u16 d1_len, PM8 d2_Addr);

	#define SC_FUNCTION_EXC_INIT_STATUS()
	#define SC_FUNCTION_EXC_SET_STATUS(status)
	#define SC_FUNCTION_EXC_CHECK_STATUS()
	#define SC_FUNCTION_EXC_SET_STATUS_START()
	#define SC_FUNCTION_EXC_SET_STATUS_PROCESS()
	#define SC_FUNCTION_EXC_SET_STATUS_END()
	//#define SC_FUNCTION_EXC_SET_STATUS_START()								SC_FUNCTION_EXC_SET_STATUS(KEY_FUNCTION_EXC_STATE_START)
	//#define SC_FUNCTION_EXC_SET_STATUS_PROCESS()							SC_FUNCTION_EXC_SET_STATUS(KEY_FUNCTION_EXC_STATE_PROCESS)
	//#define SC_FUNCTION_EXC_SET_STATUS_END()								SC_FUNCTION_EXC_SET_STATUS(KEY_FUNCTION_EXC_STATE_END)
#endif /* SECOPERATIONHAL_H_ */
