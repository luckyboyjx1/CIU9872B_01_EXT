/*
 * Io_Uart.c
 *
 *  Created on: 2015年1月12日
 *      Author: linguoming
 */

#include "Io_Uart.h"
#include "IoBuffer.h"
#include "BootNoAreaInitRam.h"
#include "SysHal.h"
#include "SysConfig.h"
#include "JcvmConfig.h"
#include "MixMode.h"

#if HAL__IO__ISO7816_UART

static u8							X_A	g_current_D;
static u16 						    X_A	g_current_F;
static UartSatus					X_A	g_uartSatus;

/* Fi/F and Di/D conversion tables */
static u16 CONST  iso7816Util_FiTable[16] =
{
	372,    /* 0  */
	372,    /* 1  */
	558,    /* 2  */
	744,    /* 3  */
	1116, /* 1116, // 4  //fmax > 8MHz ! */
	1488, /* 1488, // 5  //fmax > 8MHz ! */
	1860, /* 1860, // 6  //fmax > 8MHz ! */
	0xffff, /* 7 = RFU */
	256, /* 8 = RFU */
	512,    /* 9  */
	768,    /* 10 */
	1024, /* 1024, // 11  //fmax > 8MHz ! */
	1536, /* 1536, // 12  //fmax > 8MHz ! */
	2048, /* 2048, // 13  //fmax > 8MHz ! */
	0xffff, /* 14 */
	0xffff  /* 15 */
};

static  u8 CONST iso7816Util_DiTable[16] =
{
	0xff, /* 0 = RFU  */
	1, /* 1 */
	2, /* 2 */
	4, /* 3 */
	8, /* 4 */
	16, /* 5 */
	32, /* 6 */
	64, /* 7 */
	12, /* 8 */
	20, /* 9 */
    0xff,
	0xff,
	0xff,
	0xff, /* 13 = RFU */
	0xff, /* 14 = RFU */
	0xff  /* 15 = RFU */
};

#define	MAX_ATR_LEN					30

void IoUart_InitVar(void)
{
	ecpMem_Set(MEMtPX8(&g_uartSatus), 0xFF, sizeof(UartSatus));
	g_current_D					= 1;
	g_current_F					= 372;
}

u8 const ATRDefineALL[21]={0x9F,0x95,0x80,0x3F,0xC7,0x82,0x80,0x31,0xE0,0x73,0xFE,0x21,0x1B,0x57,0x31,0x86,0x60,0xA0,0x01,0x01,0x02};

//modify by chentao@20200811 send special ATR when ATRHistoryLen = 0

static void IoUart_SendRemainATR(u8 isWarmReset)    SMALL_MODE
{
	u8		D_A		ATRCounter;
	PN8		D_A		currentATR;
	PN8		D_A		currentATRHis;
	u8		D_A		currentATRInterfaceLen;
	u8		D_A		currentATRHistoryLen;
	u8		D_A		tckValue;
	u8		D_A		isTckPresent;
	u8		D_A		i;
	u8		X_A		AtrBuffer[MAX_ATR_LEN];

	ATRCounter		= 0;
	isTckPresent	= 0;

	if (isWarmReset)
	{
		currentATRInterfaceLen	= sysConfig_GetWarmAtrIfLen();
		currentATRHistoryLen	= sysConfig_GetWarmAtrHistLen();
		currentATR				= MEMtPN8(sysConfig_GetWarmAtrIfBase());
		currentATRHis			=	MEMtPN8(sysConfig_GetWarmAtrHistBase());
		if(sysConfig_IsWarmAtrTtrTckPresent())
		{
			isTckPresent = 1;
		}
	}
	else
	{
		currentATRInterfaceLen	= sysConfig_GetColdAtrIfLen();
		currentATRHistoryLen	= sysConfig_GetColdAtrHistLen();
		currentATR				= MEMtPN8(sysConfig_GetColdAtrIfBase());
		currentATRHis			=	MEMtPN8(sysConfig_GetColdAtrHistBase());
		if(sysConfig_IsColdAtrTtrTckPresent())
		{
			isTckPresent = 1;
		}
	}

	if(currentATRHistoryLen == 0) //send established atr
	{
			ecpMem_Copy(&AtrBuffer[ATRCounter], &ATRDefineALL[0], sizeof(ATRDefineALL));
		  ATRCounter					= U16t8(U8_A_U8(ATRCounter, sizeof(ATRDefineALL)));
	}
	else
	{
		//set T0
		AtrBuffer[ATRCounter]		= U8_OR_U8(U8_AND_U8(PR_R_U8(&currentATR[ATRCounter]), 0xF0), currentATRHistoryLen);
		U8_INC(ATRCounter);
		U8_DEC(currentATRInterfaceLen);

		//copy interface bytes
		ecpMem_Copy(&AtrBuffer[ATRCounter], &currentATR[ATRCounter], currentATRInterfaceLen);
		ATRCounter					= U16t8(U8_A_U8(ATRCounter, currentATRInterfaceLen));

		//send history bytes
		ecpMem_Copy(&AtrBuffer[ATRCounter], &currentATRHis[0], currentATRHistoryLen);
		ATRCounter					= U16t8(U8_A_U8(ATRCounter, currentATRHistoryLen));
	}

	//set TCK
	if(isTckPresent)
	{
		tckValue = AtrBuffer[0];

		for (i = 1; i < ATRCounter; U8_INC(i))
		{
			tckValue			= U8_XOR_U8(tckValue, AtrBuffer[i]);
		}

		AtrBuffer[ATRCounter]	= tckValue;

		U8_INC(ATRCounter);
	}
	
#if UICC__FUNCTION_CMCCAPI
	getOnlineStateInit();	
#endif
	//RAM_ATR_Flag = 0x01;

	ecpUart_AtrDelay();
	ecpUart_SendRemaindAtr(MEMtPX8(AtrBuffer), ATRCounter, SEND_ATR);
	MixMode_SetInterfaceActivedFlag(ISO7816_INTERFACE);

}

static void IoUart_setDF(u8 FiDi)
{
    u8		Fi;
    u8		Di;

    Fi = U8_AND_U8(U8_SHR_U8(FiDi, 4), 0x0F);
    Di = U8_AND_U8(FiDi, 0x0F);

	g_current_D = iso7816Util_DiTable[Di];
	g_current_F = iso7816Util_FiTable[Fi];
}

void IoUart_Init(void)
{
	u8			D_A		isWarmReset;


	IoUart_InitVar();
	ecpUart_Init();
	
	ecpUart_TsDelay();

	if(!sysConfig_UartInverseConvention())
	{
		ecpUart_SendDirectTs();
	}
	else
	{
		ecpUart_SendinverseTs();
	}

	if(sysConfig_IsWarmReset())
	{
		isWarmReset = true;
	}
	else
	{
		isWarmReset = false;
	}
	IoUart_SendRemainATR(isWarmReset);
//     IoUart_SetATRDoneFlag();
    sysConfig_SetCpuClkCb();//switch SYSCLK to high level
#if BOOT__IO__SWP
	G_bCLKSwitchValid = 0x01;
#endif
	ecpSysHal_ManageSetClockConfig(0);
	
	ecpUart_EnRxIrq();
}

boolean	IoUart_IsPpsAllowed(void)
{
	if(true == g_uartSatus.ppsAllowed)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void IoUart_DisablePps(void)
{
	g_uartSatus.ppsAllowed	= false;
}

u8	Io_Uart_AnalysPps0(u8 pps0)
{
	u8	i;
	u8	len;
	
	len		= 0;
	
	for(i = 0; i < 3; U8_INC(i))
	{
		if(U8_AND_U8(pps0, U8_SHL_U8(0x10, i)))
		{
			U8_INC(len);
		}
	}
	
	return len;
}

boolean IoUart_Pps(PX8 ppsBuffer, u8 len)
{
	boolean		result;
	u8 			pps0;
	u8			offset;
	u8			pck;

	g_uartSatus.ppsAllowed		= true;
	result						= true;
	pck							= 0xFF;
	pps0						= ppsBuffer[1];
	pps0						= U8_SHR_U8(pps0, 4);
	U8_DEC(len);

	//get PCK
	for(offset = 1; offset < len; U8_INC(offset))
	{
		pck						= U8_XOR_U8(pck, ppsBuffer[offset]);
	}

	if(pck != ppsBuffer[offset])
	{
		result = false;
	}

	if(0 != U8_AND_U8(ppsBuffer[1], 0x80))
	{
		result = false;
	}
    
	if(0 != U8_AND_U8(ppsBuffer[1], 0x01))
	{
		if(!HAL__IO__ISO7816_T1)
		{
		     result = false;
		}
	}
	if(true == result)
	{

		if (U8_AND_U8(ppsBuffer[1], 0x0e) || (U8_AND_U8(ppsBuffer[1], 0x70) && (!U8_AND_U8(ppsBuffer[1], 0x10))))
		{
			result = false;
		}

		if (U8_AND_U8(pps0, 0x01))
		{
	
			if (ecpUart_CheckFiDi(ppsBuffer[2]) == 0)
			{
				result = false;
			}
			else
			{
				//PPS successful
				result = true;
			}
		}
		else
		{
			//Default IO speed was requested (1 ETU = 372 T)Send response
			result = true;
		}
	}
    ecpUart_GuardTime();
	return result;
}

void IoUart_SetPps0(u8 pps0)
{
	#if HAL__IO__ISO7816_T1
	if(U8_AND_U8(pps0, 0x0f) == 0x01)
	{
		g_uartSatus.isT0Protocol = false;
	}
	#endif
}

void IoUart_SetPps1(u8 pps1)
{
	ecpUart_SetFiDi(pps1);
	IoUart_setDF(pps1);
}

boolean IoUart_isT0Protocol(void)
{
	boolean result;

	if(true == g_uartSatus.isT0Protocol)
	{
		result	= true;
	}
	else
	{
		result	= false;
	}

	return result;
}

u8 IoUart_GetD(void)
{
	return g_current_D;
}

u16 IoUart_GetF(void)
{
	return g_current_F;
}

/****************************************************************************
  Function:      IoUart_Busy
  Description:   7816正在接收或者发送
  Input:           
  Output:         
  Return:        
  Others:                            
****************************************************************************/
u8 IoUart_Busy(void)
{
    if (ecpUart_TxRxRunning() == 1)
	{
		return 1;
	}
    if(ecpUart_WarnReset())
    {
        return 1;
    }
    return 0;
}


u8 IoUart_isActived()
{
	return MixMode_GetInterfaceActivedFlag(ISO7816_INTERFACE);
	//return RAM_ATR_Flag;
}

void IoUart_ClearFifo(void)
{
	ecpUart_ClearFifo();
}

// u8 IoUart_GetATRDoneFlag(void)
// {
//     return g_bATR_Done;
// }

// u8 IoUart_SetATRDoneFlag(void)
// {
//     g_bATR_Done = 1;
// }



#endif
