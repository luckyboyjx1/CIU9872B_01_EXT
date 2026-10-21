#include "Io_7816T0.h"
#include "SysConfig.h"
#include "UartHal.h"
#include "WtxTimer.h"
#include "BootErrorHandle.h"
#include "BootNoAreaInitRam.h"
#include "Io_Uart.h"
#include "MixMode.h"
#include "boot.h"
#include "InterGetResponse.h"
#include "7816Channel.h"
#include "Io_Uart.h"
#include "euiccConfig.h"
#include "Io.h"
#include "uicc.h" // modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 



#if HAL__IO__ISO7816_T0

static  iso7816T0_Control_t			X_A		g_defaultT0Control;
static	piso7816T0_Control_t		X_A		g_pT0Control;


static void iso7816T0_Clr(void)
{
	g_pT0Control->sw1sw2			= 0;
	g_pT0Control->ResponseOffset	= 0;
	g_pT0Control->isDataReceived	= false;
	g_pT0Control->b6C61				= RESP_NO616C;
}

static u16 iso7816T0_SendData(PX8 dataBuf, u16 datalen, u8 burst)    SMALL_MODE
{
	ecpUart_TxMulBytes(&g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_INS_OFFSET], 1, SEND_PRC_BYTE);
	ecpUart_TxMulBytes(dataBuf, datalen, SEND_DATA);
	return 0;
}

static u16 iso7816T0_SendSW1SW2(u8 sw1, u8 sw2)
{
	u8				sw1sw2[2];

	sw1sw2[0] = sw1;
	sw1sw2[1] = sw2;
	ecpUart_GuardTime();	
	ecpUart_TxMulBytes(MEMtPR8(sw1sw2), 2, SEND_STATUS);

	return 0;
}

static void iso7816T0_send616cxx(u16 len)    SMALL_MODE
{
	// Set SW1SW2 as 61xx. For Case 2E and 4E, set to 6100 if more
	// than 256 bytes are to be sent in the out bound direction.
	if (len >= MAX_LE)
	{
		if(RESP_61 == g_pT0Control->b6C61)
		{
			g_pT0Control->sw1sw2	= SW_BYTES_REMAINING_00;
		}
		else
		{
			g_pT0Control->sw1sw2	= SW_CORRECT_LENGTH_00;
		}
	}
	else
	{
		if(RESP_61 == g_pT0Control->b6C61)
		{
			g_pT0Control->sw1sw2 	= U32t16(U16_A_U16(SW_BYTES_REMAINING_00, U16_AND_U16(len, 0x00FF)));
		}
		else
		{
			g_pT0Control->sw1sw2	= U32t16(U16_A_U16(SW_CORRECT_LENGTH_00, U16_AND_U16(len, 0x00FF)));
		}
	}

	if(RESP_61 == g_pT0Control->b6C61)
	{
		iso7816T0_SendSW1SW2(0x61, U16t8(U16_AND_U16(g_pT0Control->sw1sw2, 0x00ff)));
	}
	else
	{
		//此处需要增加开关NATIVE_CARD_UICC, 因为在native卡的时候, 也需要以下判断, (脚本：NativeOs测试脚本/GSM平台11.17测试脚本/DESCRIPTION OF THE FUNCTIONS/CompareFileInfor), -jiangqingbin-20221129
#if (JAVA_CARD_UICC || NATIVE_CARD_UICC)
		//A0C00000XX,XX长度不符合时返回67XX而不是6CXX
		//if((U8_AND_U8(U16t8(getChannelContext(selected_channel)),0xFF) == UICC_APPID)&&(g_pT0Control->iso7816T0_CurHeader[0] == 0xA0)&&(g_pT0Control->iso7816T0_CurHeader[1] == 0xC0))
		if(IsUiccApp(selected_channel)&&(g_pT0Control->iso7816T0_CurHeader[0] == 0xA0)&&(g_pT0Control->iso7816T0_CurHeader[1] == 0xC0))	//同步EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP的修改 - -jiangqingbin-20221129
		{
			iso7816T0_SendSW1SW2(0x67, U16t8(U16_AND_U16(g_pT0Control->sw1sw2, 0x00ff)));
		}
		else
#endif
		{
			iso7816T0_SendSW1SW2(0x6c, U16t8(U16_AND_U16(g_pT0Control->sw1sw2, 0x00ff)));
		}
	}
}

static boolean iso7816T0_IsSend616C(u16 Le, u16 Lr)
{
	boolean		result;

	if(RESP_NO616C == g_pT0Control->b6C61)
	{
		if ((true == g_pT0Control->isDataReceived) || (Lr != Le))
		{
			if((true != g_pT0Control->isDataReceived) && (Lr != Le))
			{
				/*if(sysConfig_IsUartToSend616CXX())
				{
					g_pT0Control->b6C61 = RESP_6C;
				}
				else*/
				{
					if(/*(Le == 256) ||*/ (Lr > Le))//case 4s
					{
						g_pT0Control->b6C61 = RESP_61;
					}
					else
					{
						if(Lr != Le)
						{
							g_pT0Control->b6C61 = RESP_6C;	   //???????
						}
					}
				}
			}
			else
			{
				g_pT0Control->b6C61 = RESP_61;
			}

			result			= true;
		}
		else
		{
			result			= false;
		}
	}
	else
	{
		result			= true;
	}

	return result;
}

static u8 iso7816T0_CheckGetResponseLen(u16 len)
{
	u16		newLen;
	u16		expLen;
	u8		result;

	expLen				= len;
	newLen				= g_pT0Control->command->Le;

	if(newLen == 0)
	{
		expLen			= MAX_LE;
	}
	else
	{
		expLen			= newLen;
	}

	if(expLen > len)
	{
		g_pT0Control->b6C61 = RESP_6C;
		result	= false;
	}
	else
	{
		result	= true;
	}

	if(true == result)
	{
		if((RESP_6C == g_pT0Control->b6C61) && (newLen != len))
		{
			//result	= false;   //根据测试用例UICC测试->命令测试->SelectFile-6.8.1.1，6C2B 收到 00A4030405 后，发送5字节数据，返回6126,by xwh 20191114
			g_pT0Control->b6C61 = RESP_61;
		}
	}

	return result;
}
void iso7816T0_Clrb6C61(void)
{
	PX8									tempBuf;

	tempBuf		= g_pT0Control->iso7816T0_CurHeader;

	if(tempBuf[ISO7816UTIL_INS_OFFSET] != 0xC0)
	{
		if(RESP_61 == g_pT0Control->b6C61)
		{
			iso7816T0_Clr();
		}
	}
}
//static u8 iso7816T0_IsGetResponse(void)
u8 iso7816T0_IsGetResponse(void)
{
	PX8									tempBuf;		//这种局部变量名不需要改？-jiangqingbin-20221206
	u8									result;
	u8									preCmdChannel;
	u8									curCmdChannel;
	ppIso7816Util_Response_t			ppResponse;		//同步 EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP的修改	-jiangqingbin-20221203

	result		= true;
	tempBuf		= g_pT0Control->iso7816T0_CurHeader;
	iso7816Util_GetResponse( &ppResponse );

	preCmdChannel = iso7816Channel_CalLogicalChannel(g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET]);

	curCmdChannel = iso7816Channel_CalLogicalChannel(tempBuf[ISO7816UTIL_CLA_OFFSET]);

	if(RESP_61 == g_pT0Control->b6C61)
	{
		//上一条命令返回61xx或6cxx，当前命令的ins 是0xc0的时候
		//Modified by Ding to deal with getresponse command with Logicalnumber, 01c00000Lc 20130604
		if(0x00 != sysConfig_IsUartC0HaveLogicNum())
		{
			if(PXB_R_U32(&tempBuf[ISO7816UTIL_CLA_OFFSET]) != 0X00C00000)
			{
				result = false;
			}
		}
		else
		{
#if (JAVA_CARD_UICC || NATIVE_CARD_UICC)			//CLA不相等，要返回6E00	//此处需要增加开关 NATIVE_CARD_UICC, 因为native卡也有多通道测试-jiangqingbin-20221203 (脚本： UICC通用测试)
			if(tempBuf[ISO7816UTIL_INS_OFFSET] != 0xC0)
			{
				bakLogicalChninfo();
				iso7816T0_Clr();
				return false;
			}

			if(curCmdChannel != preCmdChannel)
			{
				bakLogicalChninfo();
#if !BOOTLOADER_PRO		//同步 EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP的修改	-jiangqingbin-20221203
				return recLogicalChninfo();
#else
				ppResponse->SW1SW2 = 0x6e00;
				ppResponse->length = 0x00;
				return TRUE;
#endif
			}
			
			//CLA不相等，要返回6E00
			//if(U8_AND_U8(U16t8(getChannelContext(curCmdChannel)), 0xFF)==UICC_APPID)		//同步 EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP的修改	-jiangqingbin-20221203
			if(IsUiccApp(selected_channel))
			{
				if((g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET] == 0xA0)&&(tempBuf[ISO7816UTIL_CLA_OFFSET] != 0xA0))
				{
					result = false;
				}

				if((PX_R_U32(&g_pT0Control->iso7816T0_PreHeader) != 0x00000000)&&(g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET] != 0xA0)&&(tempBuf[ISO7816UTIL_CLA_OFFSET] == 0xA0))
				{
					if(g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_INS_OFFSET] != 0x80) //MD5指令8080XXXXXX，允许A0C0获取响应
					{
						result = false;
					}
				}

				if (((G_CommandMode == Command_Mode_2G)^(tempBuf[ISO7816UTIL_CLA_OFFSET]==0xA0))||(((tempBuf[ISO7816UTIL_CLA_OFFSET]&0xFC) != 0x80)&&((tempBuf[ISO7816UTIL_CLA_OFFSET]&0xFC) != 0x00)&&(tempBuf[ISO7816UTIL_CLA_OFFSET] != 0xA0)))
				{
					return false;
				}

				G_CurrentReponse[G_ChannelNum] = 0x00;
			}
 #else
 			if(tempBuf[ISO7816UTIL_INS_OFFSET] != 0xC0)
 			{
				result = false;
 			}
 #endif			
			if((PXB_R_U16(&tempBuf[ISO7816UTIL_P1_OFFSET]) != 0X0000 ) && (PXB_R_U16(&tempBuf[ISO7816UTIL_P1_OFFSET]) != PXB_R_U16(&g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_P1_OFFSET])))
			{
				result = false;
			}
		}
	}
	else if(RESP_6C == g_pT0Control->b6C61)
	{
		if(PX_R_U32(&g_pT0Control->iso7816T0_CurHeader) != PX_R_U32(&g_pT0Control->iso7816T0_PreHeader))
		{
			result = false;
		}
	}
#if (JAVA_CARD_UICC || NATIVE_CARD_UICC) 			//CLA不相等，要返回6E00	////此处需要增加开关NATIVE_CARD_UICC, 因为native卡也有多通道测试-jiangqingbin-20221203 (脚本： UICC通用测试)
	else if(tempBuf[ISO7816UTIL_INS_OFFSET] == 0xC0)
	{
		//ins 为 0xc0 去取不同的逻辑通道的数据

	    preCmdChannel = iso7816Channel_CalLogicalChannel(g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET]);
		//if(U8_AND_U8(U16t8(getChannelContext(preCmdChannel)), 0xFF)==UICC_APPID)
		if(IsUiccApp(preCmdChannel))		//同步 EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP的修改	-jiangqingbin-20221203
		{
			setChannelNum(tempBuf[ISO7816UTIL_CLA_OFFSET]);
			G_CurrentReponse[G_ChannelNum] = 0x00;
		}

		//if(U8_AND_U8(U16t8(getChannelContext(curCmdChannel)), 0xFF)==UICC_APPID)
		if(IsUiccApp(curCmdChannel))
		{
			if (((G_CommandMode == Command_Mode_2G)^(tempBuf[ISO7816UTIL_CLA_OFFSET]==0xA0))||(((tempBuf[ISO7816UTIL_CLA_OFFSET]&0xFC) != 0x80)&&((tempBuf[ISO7816UTIL_CLA_OFFSET]&0xFC) != 0x00)&&(tempBuf[ISO7816UTIL_CLA_OFFSET] != 0xA0)))
			{
				return false;
			}
		}

	#if RSP_EUICC
		if((curCmdChannel != preCmdChannel)&&(G_SendFileExtendBackDataStruct.backAddr == 0))
	#else
		if(curCmdChannel != preCmdChannel)
	#endif
		{
			#if !BOOTLOADER_PRO		//同步 EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP的修改	-jiangqingbin-20221203
				return recLogicalChninfo();
			#endif
		}

		result = false;
	}
#endif
	else
	{
		result = false;
	}
    if(false == result)
	{
		iso7816T0_Clr();
	}
	return result;
}

static void iso7816T0_Pps(PX8 commandBuf)
{
	u8		D_A	pps0;
	u8		D_A i;
	u8		D_A ppsByteOffset;
	u8		D_A len;

	//receive PPS0
	MixMode_UartRxMulBytes(&commandBuf[1], 1, RECEIVE_COMMAND);
	pps0				= commandBuf[1];
	ppsByteOffset		= 2;
	len					= Io_Uart_AnalysPps0(pps0);

	//receive PPS1, PPS2 and PPS3 according PPS0
	for(i = 0; i < len; U8_INC(i))
	{
		MixMode_UartRxMulBytes(&commandBuf[ppsByteOffset], 1, RECEIVE_COMMAND);
		U8_INC(ppsByteOffset);
	}

	//receive PCK
	MixMode_UartRxMulBytes(&commandBuf[ppsByteOffset], 1, RECEIVE_COMMAND);
	U8_INC(ppsByteOffset);

	if(true == IoUart_Pps(commandBuf, ppsByteOffset))
	{
		ecpUart_TxMulBytes(&commandBuf[0], ppsByteOffset, SEND_PPS);
		IoUart_SetPps0(pps0);
		if(U8_AND_U8(pps0, 0x10))
		{
			IoUart_SetPps1(commandBuf[2]);
		}
	}
	else
	{
		PXB_W_U32(&commandBuf[0], ERROR_PPS_RESPONSE);
		ecpUart_TxMulBytes(&commandBuf[1], 3, SEND_PPS);
	}

	return;
}

void iso7816T0_GetT0Control(piso7816T0_Control_t * toControl)
{
	*toControl						= g_pT0Control;
}

void iso7816T0_SetT0Control(piso7816T0_Control_t toControl)
{
	g_pT0Control					= toControl;
}

void iso7816T0_Init(void)
{
	ecpUart_T0Init();
	g_pT0Control					= &g_defaultT0Control;
	iso7816T0_Clr();
#if JAVA_CARD_UICC
	b616crecflag_init();
#endif
	iso7816Util_GetCommand(&g_pT0Control->command);
	ecpMem_Set(&g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_CLA_OFFSET], 0, sizeof(g_pT0Control->iso7816T0_CurHeader));
	ecpMem_Set(&g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET], 0, sizeof(g_pT0Control->iso7816T0_PreHeader));
	InterGetRes_Init();
}

void iso7816T0_RecieveCommand(void)
{
	u8	cmdBuf[ISO7816UTIL_HEADER_LENGTH];
	
	g_pT0Control->isPpsCommandReceived	= false;
	MixMode_UartRxMulBytes(&cmdBuf[ISO7816UTIL_CLA_OFFSET], 1, RECEIVE_COMMAND);

	if((IO_UART_PPS_BYTE == cmdBuf[ISO7816UTIL_CLA_OFFSET]) && (true == IoUart_IsPpsAllowed()))
	{
		iso7816T0_Pps(MEMtPX8(cmdBuf));
		
		g_pT0Control->isPpsCommandReceived	= true;
//		g_Iso7816PpsFirstReceiveCommand= true;
		
		wtxTimer_WtxTimerInit();
//		ecpUart_SetRxLength(0x05);
//		ecpSysHal_ManageSetClockConfig(0);
		
	}
	else
	{
		MixMode_UartRxMulBytes(&cmdBuf[ISO7816UTIL_INS_OFFSET], U8_S_U8(ISO7816UTIL_HEADER_LENGTH, 1), RECEIVE_COMMAND);
		IoUart_DisablePps();
		InterGetRes_Main(cmdBuf[ISO7816UTIL_CLA_OFFSET]);
		ecpMem_Copy(&g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET], &g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_CLA_OFFSET], ISO7816UTIL_HEADER_LENGTH);
		ecpMem_Copy(&g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_CLA_OFFSET], &cmdBuf[ISO7816UTIL_CLA_OFFSET], ISO7816UTIL_HEADER_LENGTH);
		iso7816Util_ResetParser();
		iso7816Util_ParseBytes(&cmdBuf[ISO7816UTIL_CLA_OFFSET], ISO7816UTIL_HEADER_LENGTH, PH_ISO7816L4_FLAG_BYPASSPARSER);
		ecpUart_GuardTime();
	}
}

void iso7816T0_FetchCommand(PX8 buf)
{
	ecpMem_Copy(buf, g_pT0Control->iso7816T0_CurHeader, ISO7816UTIL_HEADER_LENGTH);
}

u16 iso7816T0_Receive(void)
{
	u16						D_A		dataLength;
	PX8						D_A		commandBuffer;

	g_pT0Control->isDataReceived	= true;
	commandBuffer 				= g_pT0Control->command->pData;
	dataLength					= g_pT0Control->command->Lc;

	if(0 == dataLength)
	{
		return dataLength;
	}
	
	MixMode_UartReadyRx(dataLength);
	//change receive flag = RECEIVE_DATA befort send INS.SPI INT function occupy time ,avoid  7816 into INT function:flag == RECEIVE CMD.
	MixMode_UartReadyRxData();
	ecpUart_TxMulBytes(&g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_INS_OFFSET], 1, SEND_ACK_BYTE);
	MixMode_UartRxMulBytes(&commandBuffer[ISO7816UTIL_DATA_OFFSET], dataLength, RECEIVE_DATA);

	iso7816Util_ParseBytes(&commandBuffer[ISO7816UTIL_DATA_OFFSET],
							dataLength,
							U8_OR_U8(PH_ISO7816L4_FLAG_BYPASSPARSER, PH_ISO7816L4_FLAG_T0_REV_CMD));

	return dataLength;
}

u16 iso7816T0_Transmit(void)
{
	u16						D_A			len;
	u16									Le;
	PX8									responseBuffer;
	ppIso7816Util_Response_t			respond;

	iso7816Util_GetResponse( &respond );

	g_pT0Control->isDataReceived	= false;
	len							= respond->length;
	Le							= g_pT0Control->command->Le;
	responseBuffer				= respond->pData;
	if(len >= ISO7816UTIL_STATUS_BYTE_LENGTH)
	{
		len							= U16_S_U8(len, ISO7816UTIL_STATUS_BYTE_LENGTH);
	}

	if(true == iso7816T0_CheckGetResponseLen(len))
	{
		Le					= g_pT0Control->command->Le;

		if(len > Le)
		{
			iso7816T0_SendData(&responseBuffer[g_pT0Control->ResponseOffset], Le, ACK_INS);
			len				= U16_S_U16(len, Le);
		}
		else
		{
			iso7816T0_SendData(&responseBuffer[g_pT0Control->ResponseOffset], len, ACK_INS);
			Le				= U16_S_U16(Le, len);
			len				= 0;
		}

		g_pT0Control->ResponseOffset	= U16t8(U8_A_U16(g_pT0Control->ResponseOffset, Le));

		g_pT0Control->command->Le 	= Le;

		//respond->length				= len;
		respond->length				= len + 2;     //respond->length 要包括SW1SW2 的长度,by xwh 20191114

		//此处需要增加开关NATIVE_CARD_UICC, 因为在native卡的时候, 也需要以下判断, (脚本：NativeOs测试脚本/GSM平台11.17测试脚本/LOGICAL MODEL/File identifier), -jiangqingbin-20221129
#if (JAVA_CARD_UICC || NATIVE_CARD_UICC)	//CLA不相等，要返回6E00	
		//根据SIM API测试用例fwk_pcs_igco，9F17之后收到A0C0000006后返回9000而不是9F11,by xwh 20191126
		if((g_pT0Control->iso7816T0_CurHeader[0] == 0xA0)&&(g_pT0Control->iso7816T0_CurHeader[1] == 0xC0))
		{
			respond->length = 0x02;
		}
#endif
	}

	return 0;
}

void iso7816T0_SendStatus(void)
{
	boolean						D_A		isSendProcessByte;
	u16							D_A		resLen;
	u16							D_A		status;
	ppIso7816Util_Response_t	D_A		ppResponse;

	iso7816Util_GetResponse( &ppResponse );

	status = ppResponse->SW1SW2;

//此处需要增加开关NATIVE_CARD_UICC, 因为在native卡的时候需要以下判断, 状态字是9F17的时候需要以下判断成立, 否则将跳过以下判断继续往后执行时, 状态字将被强行切换为6117,导致选文件指令A0A4000002 3F00时抛状态字6117, -jiangqingbin-20221129
#if (JAVA_CARD_UICC || NATIVE_CARD_UICC)	
	//if((status == 0x6283)||(status == 0x6200)||((status&0xFF00) == 0x6100)||((status&0xFF00) == 0x9F00)||((status&0xFF00) == 0x9E00))
	if((status == 0x6283)||(status == 0x6200)||((status&0xFF00) == 0x6100)||((status&0xFF00) == 0x6C00)||((status&0xFF00) == 0x6700)||((status&0xFF00) == 0x9F00)||((status&0xFF00) == 0x9E00))
	{
		//if(U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF)==UICC_APPID)
		if(IsUiccApp(selected_channel))	//native卡时没有定义getChannelContext, 参考之前ST红茶移动的修改 同步代码源于：EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP		-jiangqingbin-20221129
		{
			iso7816T0_Clr();
			InterGetRes_Realease();
			iso7816T0_SendSW1SW2(HIGH(status), LOW(status));

			if(ppResponse->length > ISO7816UTIL_STATUS_BYTE_LENGTH)
			{
				if((status&0xFF00) == 0x6C00)
				{
					g_pT0Control->b6C61 = RESP_6C;
				}
				else
				{
					g_pT0Control->b6C61 = RESP_61;
				}

			}
			
			ppResponse->SW1SW2 = 0x9000;
			return;
		}				
	}
#endif	
	if(ppResponse->length > ISO7816UTIL_STATUS_BYTE_LENGTH)
	{
		resLen						= U16_S_U8(ppResponse->length, ISO7816UTIL_STATUS_BYTE_LENGTH);
		isSendProcessByte			= iso7816T0_IsSend616C(g_pT0Control->command->Le, ppResponse->lr);
		if(true == isSendProcessByte)
		{
			iso7816T0_send616cxx(resLen);
		}
		else
		{
			iso7816T0_SendData(&ppResponse->pData[g_pT0Control->ResponseOffset], resLen, ACK_INS);
		}
	}
	else
	{
		isSendProcessByte			= false;
	}

	if(false == isSendProcessByte)
	{
		iso7816T0_Clr();
		InterGetRes_Realease();

#if JAVA_CARD_UICC		
		
		status = IF_Process91xx(status,g_pT0Control->iso7816T0_CurHeader[1],io_GetLogicalChannel(),(U8_AND_U8(U16t8(getChannelContext(0)), 0xFF)==UICC_APPID));
#endif
		
		iso7816T0_SendSW1SW2(HIGH(status), LOW(status));
	}	
}

u8 iso7816T0_HandleCommand(void)
{
	u8							D_A		isHandleCommandRetrurn;

	if(true == iso7816T0_IsGetResponse())
	{
		iso7816T0_Transmit();
		iso7816Channel_SetLogicalChannel();
		isHandleCommandRetrurn	= false;
		wtxTimer_WtxTimerStop();
	}
	else
	{
		if(true == g_pT0Control->isPpsCommandReceived)
		{
			isHandleCommandRetrurn	= false;
		}
		else
		{
			isHandleCommandRetrurn	= true;
			
			//ecpMem_Copy(&g_pT0Control->iso7816T0_PreHeader[ISO7816UTIL_CLA_OFFSET], &g_pT0Control->iso7816T0_CurHeader[ISO7816UTIL_CLA_OFFSET], ISO7816UTIL_HEADER_LENGTH);
		}
	}

	return isHandleCommandRetrurn;
}

u32 iso7816T0_GetT0WtxTimerValue(void)
{
	u32		D_A    value;
	u16		D_A    bwiValue;

	bwiValue	= U8t16(sysConfig_GetUartTC2());
#if HAL__IO__TIMER_MODE
	value		= U32_M_U32(U16t32(sysConfig_GetUartWtx()), U16t32(U16_M_U16(IoUart_GetF(), bwiValue)));
#else
	value		= U16_M_U32(sysConfig_GetUartWtx(), U8_M_U16(IoUart_GetD(), bwiValue));
#endif

	return ecpDivideT0WtxTimerFrequency(value);
}

piso7816T0_Control_t get_g_pT0Control(void)
{
	return g_pT0Control;
}

boolean iso7816T0_IsSend616C_ext(u16 Le, u16 Lr)
{
	return iso7816T0_IsSend616C(Le,Lr);
}

#endif	//
