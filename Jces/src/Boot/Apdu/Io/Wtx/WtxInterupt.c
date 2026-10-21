#include "WtxTimer.h"
#include "WtxInterupt.h"
#include "7816Util.h"
#include "Io_7816T1.h"
#include "Crypto_API.h"
#include "SysConfig.h"
#include "NvmHal.h"

#if HAL__IO__TIMER

void WtxInterupt_UartRequestWtx(u8 currentProtocol)  SMALL_MODE
{
	u8	I_A ToWtxByte;
#if HAL__IO__ISO7816_T1
	u8		I_A		RNGprocess;
	u8		I_A		RSAprocess;
	u8		I_A		DESprocess;
	u8		I_A		AESprocess;
#endif
	PATCH_BOOT_APDU_WTXINTERRUPT_001();
	
	wtxTimer_WtxTimerStop();
    U8_INC(g_wtxDelay);
	if(g_wtxDelay == g_wtxDelayLimit)
	{
		if(ecpNvm_IsInProgress())
		{
			ecpNvm_WaitForProgress();
		}
#if HAL__IO__ISO7816_T1
		if(APDU_PROTOCOL_T1 == currentProtocol)
		{
			if(ecpIsRngInProgress())
			{
				RNGprocess	= ecpRngSuspend();
			}

			if(ecpIsDesInProgress())
			{
				DESprocess	= ecpDesSuspend();
			}

			if(ecpIsAesInProgress())
			{
				AESprocess	= ecpAesSuspend();
			}

			if(ecpIsRsaInProgress())
			{
				RSAprocess	= ecpRsaSuspend();
			}

			if(ecpNvm_IsInProgress())
			{
				ecpNvm_WaitForProgress();
			}
			iso7816T1_SetWtxm(1);
			iso7816T1_RequestWtx();
			ecpSetT1WtxTimer(g_uartBwtH, g_uartBwtL);
			if(RNGprocess)
			{
				ecpRngRestart(RNGprocess);
			}

			if(DESprocess)
			{
				ecpDesRestart(DESprocess);
			}

			if(AESprocess)
			{
				ecpAesRestart(AESprocess);
			}

			if(RSAprocess)
			{
				ecpRsaRestart(RSAprocess);
			}
		}
		else
#endif
		{
			ToWtxByte	= 0x60;
			ecpUart_TxMulBytes(&ToWtxByte,1 , SEND_60);
			ecpSetT0WtxTimer(g_uartBwtH, g_uartBwtL);
		}
		g_wtxDelay	= 0;
	}
	else
	{
#if HAL__IO__ISO7816_T1
		if(APDU_PROTOCOL_T1 == currentProtocol)
		{
			ecpClearT1WtxTimer();
		}
		else
#endif
		{
			ecpClearT0WtxTimer();
		}
	}
#if HAL__IO__ISO7816_T1
	if(APDU_PROTOCOL_T1 == currentProtocol)
	{
		ecpStartT1WtxTimer();
	}
	else
#endif
	{
		ecpStartT0WtxTimer();
	}
}

#if HAL__IO__ISO14443_TA
void WtxInterupt_RfRequestWtx(void)
{
	u16 	I_A		storeCLKSEL;
	u8		I_A		RNGprocess;
	u8		I_A		RSAprocess;
	u8		I_A		DESprocess;
	u8		I_A		AESprocess;
	
	PATCH_BOOT_APDU_WTXINTERRUPT_002();	
	
	RSAprocess		= 0;
	DESprocess		= 0;
	AESprocess		= 0;
	RNGprocess		= 0;

	wtxTimer_WtxTimerStop();

	if(g_wtxDelay == g_wtxDelayLimit)
	{
		if(ecpIsRngInProgress())
		{
			RNGprocess	= ecpRngSuspend();
		}

		if(ecpIsDesInProgress())
		{
			DESprocess	= ecpDesSuspend();
		}

		if(ecpIsAesInProgress())
		{
			AESprocess	= ecpAesSuspend();
		}

		if(ecpIsRsaInProgress())
		{
			RSAprocess	= ecpRsaSuspend();
		}

		if(ecpNvm_IsInProgress())
		{
			ecpNvm_WaitForProgress();
		}

		storeCLKSEL	= sysConfig_GetCpuCLK();
		sysConfig_SetCpuClkRfCom();

		sysConfig_SetIntBeforeRfWtx();

		iso14443TaL4_RequestWtx();

		sysConfig_SetIntAfterRfWtx();

		sysConfig_SetCpuCLK(storeCLKSEL);

		if(RNGprocess)
		{
			ecpRngRestart(RNGprocess);
		}

		if(DESprocess)
		{
			ecpDesRestart(DESprocess);
		}

		if(AESprocess)
		{
			ecpAesRestart(AESprocess);
		}

		if(RSAprocess)
		{
			ecpRsaRestart(RSAprocess);
		}

		g_wtxDelay	= 0;
	}
	else
	{
		U8_INC(g_wtxDelay);
		ecpClearTaWtxTimer();
	}
	ecpStartTaWtxTimer();
}
#endif   //HAL__IO__ISO14443_TA

#endif	//HAL__IO__TIMER
