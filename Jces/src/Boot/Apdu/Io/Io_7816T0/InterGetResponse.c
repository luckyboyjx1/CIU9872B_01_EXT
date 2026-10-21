/*
 * InterGetResponse.c
 *
 *  Created on: 2015Äê2ÔÂ2ÈÕ
 *      Author: linguoming
 */

#include "InterGetResponse.h"
#include "IoBuffer.h"
#include "Io_7816T0.h"
#include "7816Util.h"
#include "SysConfigTarget.h"
#include "7816Channel.h"

#if BOOT__CHANNEL__GET_RESPONSE_DATA

u8						g_secResBuffer[RESPONSE_BUFFER_SIZE];
iso7816T0_Control_t		g_secT0Control;
pIso7816Util_BufPck_t	g_secBufPck;
static u8               g_NeedChnGetResponse;

memref					g_t0ControlTbl[MAX_INTER_CHN];
memref					g_t0BufPckTbl[MAX_INTER_CHN];

static void InterGetRes_Construct(u8 chn)
{
	boolean					isSetNew;
	u8                      preChn;
	piso7816T0_Control_t	pDefaultT0Control;
	ppIso7816Util_BufPck_t	pDefaultBufPck;

	piso7816T0_Control_t	pNewT0Control;
	ppIso7816Util_BufPck_t	pNewBufPck;

	piso7816T0_Control_t	pCurT0Control;

	isSetNew					= false;
	
	iso7816T0_GetT0Control(&pCurT0Control);
	preChn = iso7816Channel_GetCurLogicalChn();
    if((preChn != chn) && (RESP_NO616C != pCurT0Control->b6C61))
	{
		g_NeedChnGetResponse = 1;
		if(preChn != DEFAULT_INDEX)
		{
			if(g_t0ControlTbl[preChn] == NULL_TBL_ENTRY)
			{
				pNewT0Control				= &g_secT0Control;
				pNewBufPck					= &g_secBufPck;

				pDefaultT0Control			= (piso7816T0_Control_t)g_t0ControlTbl[DEFAULT_INDEX];
				pDefaultBufPck				= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[DEFAULT_INDEX];


				ecpMem_Copy(pNewT0Control, pDefaultT0Control, sizeof(iso7816T0_Control_t));
				ecpMem_Copy(pNewBufPck, pDefaultBufPck, sizeof(pIso7816Util_BufPck_t));

				pNewT0Control->command		= &pNewBufPck->command;
				ecpMem_Copy(g_secResBuffer,pNewBufPck->reponse.pData,pNewBufPck->reponse.length);
				pNewBufPck->reponse.pData	= g_secResBuffer;

				g_t0ControlTbl[preChn] 		= PXtMEM(pNewT0Control);
				g_t0BufPckTbl[preChn]		= PXtMEM(pNewBufPck);
			}
		}
		if(chn != DEFAULT_INDEX)
		{
			if(g_t0ControlTbl[chn] == NULL_TBL_ENTRY)
			{
				pNewT0Control				= &g_secT0Control;
				pNewBufPck					= &g_secBufPck;

				pDefaultT0Control			= (piso7816T0_Control_t)g_t0ControlTbl[DEFAULT_INDEX];
				pDefaultBufPck				= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[DEFAULT_INDEX];


				ecpMem_Copy(pNewT0Control, pDefaultT0Control, sizeof(iso7816T0_Control_t));
				ecpMem_Copy(pNewBufPck, pDefaultBufPck, sizeof(pIso7816Util_BufPck_t));

				pNewT0Control->command		= &pNewBufPck->command;
				ecpMem_Copy(g_secResBuffer,pNewBufPck->reponse.pData,pNewBufPck->reponse.length);
				pNewBufPck->reponse.pData	= g_secResBuffer;
			}
				g_t0ControlTbl[chn] 		= PXtMEM(pNewT0Control);
				g_t0BufPckTbl[chn]			= PXtMEM(pNewBufPck);
				isSetNew					= true;
		}
		else
		{
			pNewT0Control				= (piso7816T0_Control_t)g_t0ControlTbl[DEFAULT_INDEX];
			pNewBufPck					= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[DEFAULT_INDEX];
			isSetNew					= true;
		}
	}
	else
	{
		if(g_NeedChnGetResponse == 0x0)
		{
			pNewT0Control				= (piso7816T0_Control_t)g_t0ControlTbl[DEFAULT_INDEX];
		    pNewBufPck					= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[DEFAULT_INDEX];
		    isSetNew					= true;
		}
		else
		{
			pNewT0Control				= (piso7816T0_Control_t)g_t0ControlTbl[chn];
			pNewBufPck					= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[chn];
			isSetNew					= true;
		}
	}

	if(true == isSetNew)
	{
		iso7816T0_SetT0Control(pNewT0Control);
		iso7816Util_SetBufPck(pNewBufPck, UART_BUF_PCK_TYPE);
	}
}

void InterGetRes_Init(void)
{
	piso7816T0_Control_t	toControl;
	ppIso7816Util_BufPck_t	toBufPck;

	iso7816T0_GetT0Control(&toControl);
	iso7816Util_GetBufPck(&toBufPck, UART_BUF_PCK_TYPE);
    g_NeedChnGetResponse = 0x0;
	ecpMem_Set(g_t0ControlTbl, NULL_TBL_ENTRY, sizeof(g_t0ControlTbl));
	ecpMem_Set(g_t0BufPckTbl, NULL_TBL_ENTRY, sizeof(g_t0BufPckTbl));

	g_t0ControlTbl[DEFAULT_INDEX] 	= PXtMEM(toControl);
	g_t0BufPckTbl[DEFAULT_INDEX]	= PXtMEM(toBufPck);

}

static u8 InterGetRes_GetChn616C(void)
{
	u8                      i;
	piso7816T0_Control_t	T0Control;
	piso7816T0_Control_t	pT0ChnControl;
	
	for(i = DEFAULT_INDEX ; i < MAX_INTER_CHN ; U8_INC(i))
	{
		if(g_t0ControlTbl[i] != NULL_TBL_ENTRY)
		{
			T0Control = (piso7816T0_Control_t)g_t0ControlTbl[i];
			iso7816T0_SetT0Control(T0Control);
			iso7816T0_GetT0Control(&pT0ChnControl);
			if(RESP_NO616C != pT0ChnControl->b6C61)
			{
				return 0x1;
			}
	    }
	}
	return 0x0;
}

void InterGetRes_Realease(void)
{
	u8						curChn;
	u8                      chnNum;
	piso7816T0_Control_t	T0Control;
	ppIso7816Util_BufPck_t	BufPck;

	curChn		= iso7816Channel_GetCurLogicalChn();
	if(curChn > ISO_MAX_LOGICAL_CHANNELS)
	{
		curChn	= ISO_MAX_LOGICAL_CHANNELS;
	}
	if(g_NeedChnGetResponse == 0x1)
	{
		if(0x0 == InterGetRes_GetChn616C())
		{
			g_NeedChnGetResponse = 0x0;
			for(chnNum = 1 ; chnNum < MAX_INTER_CHN ; U8_INC(chnNum))
			{
				g_t0ControlTbl[chnNum] = NULL_TBL_ENTRY;
				g_t0BufPckTbl[chnNum]  = NULL_TBL_ENTRY;
			}
			T0Control				= (piso7816T0_Control_t)g_t0ControlTbl[DEFAULT_INDEX];
			BufPck					= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[DEFAULT_INDEX];

			iso7816T0_SetT0Control(T0Control);
			iso7816Util_SetBufPck(BufPck, UART_BUF_PCK_TYPE);
		}
		else
		{
			T0Control				= (piso7816T0_Control_t)g_t0ControlTbl[curChn];
			BufPck					= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[curChn];

			iso7816T0_SetT0Control(T0Control);
			iso7816Util_SetBufPck(BufPck, UART_BUF_PCK_TYPE);
		}
	}
	else
	{
		T0Control				= (piso7816T0_Control_t)g_t0ControlTbl[DEFAULT_INDEX];
		BufPck					= (ppIso7816Util_BufPck_t)g_t0BufPckTbl[DEFAULT_INDEX];
		iso7816T0_SetT0Control(T0Control);
		iso7816Util_SetBufPck(BufPck, UART_BUF_PCK_TYPE);
	}
}

void InterGetRes_Main(u8 cla)
{
	u8						newChn;

	newChn				= iso7816Channel_CalLogicalChannel(cla);
    if(newChn > ISO_MAX_LOGICAL_CHANNELS)
	{
	     newChn	= ISO_MAX_LOGICAL_CHANNELS;
	}

	InterGetRes_Construct(newChn);
}

#endif
