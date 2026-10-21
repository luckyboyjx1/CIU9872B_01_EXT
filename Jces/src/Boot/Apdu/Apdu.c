#include "Apdu.h"
#include "io.h"
#include "MemOperation.h"
#include "BootNoAreaInitRam.h"
#include "Io_7816T0.h"
#include "Io_7816T1.h"
#include "7816Channel.h"
#include "global_variable.h"
#include "SecureCrypto.h"
#include "SCPAPINativeMethod.h"
#include "Crypto_common_API.h"
#include "BootLifeCycle.h"

#if UICC_FUCTION_PRIVATEAUTH
#include "gwAPI.h"
#endif
//break the code architectural temporarily
//#include "SCPAPINativeMethod.h"

#if BOOT__IO__APDU

static u16 						X_A 		g_apduLE;
static u16 						X_A 		g_apduLR;
static u16 						X_A      	g_apduLc;
static u16 						X_A 		g_curResOffset;
static u16 						X_A 		g_ResLen;
static APDUStatus  				X_A 		g_apduStatus;
static u8 						X_A 		Current_status;
static u16						X_A			io_FrameReceiveLength;
static SETOUTGOING_CALLBACK	*	X_A			g_setOutGoingCallBack;

static void apdu_InitVar(void)
{
	g_apduLE					= 0;
	g_apduLR					= 0;
	g_curResOffset				= 0;
	g_ResLen					= 0;
	Current_status				= 0;
	g_apduLc					= 0;
	io_FrameReceiveLength		= 0;
/*@ -type @*/
	g_setOutGoingCallBack		= APDU_NULL_CALL_BACK;
/*@ +type @*/
	ecpMem_Fill(&g_apduStatus, 0x00 ,sizeof(g_apduStatus));
}

static void apdu_resetAPDU(void)
{
	g_apduLR					= 0;
	Current_status 				= STATE_INITIAL;
	g_curResOffset				= 0;
	g_ResLen					= 0;
	ecpMem_Fill(&g_apduStatus, 0x00 ,sizeof(g_apduStatus));
#if UICC_FUCTION_PRIVATEAUTH
	SetSymGuomiProperty(SYM_GUOMI_PROPERTY_STANDAND);
#endif

//#if (UICC__FUNCTION_CMCCAPI||UICC__FUNCTION__UICCAPI)
//	if (BootLifeCycle_GetLifeCycle() >= LIFE_CYCLE_CREATE) 
//	{
//		ClearTheVolatileByteArray();//CMCCAPI  cmcc_platform/6.7.2.8 要求在指令之间清除通过 CMCCPlatform.getTheVolatileByteArray()方法，获取的数组值，因此增加该方案，每次收到命令头时，都清除挥发性数组的值
//	}
//#endif

}

u8 apdu_GetLogicalChannel(void)
{
	return io_GetLogicalChannel();
}

void apdu_SetCommand(PX8 commandBuf, u16 size)
{
	io_SetCommand(commandBuf, size);
}

/*@ -discast @*/
void apdu_GetCommand( ppApdu_Command_t * ppCommand )
{
	io_GetCommand((ppIso7816Util_Command_t *)ppCommand);
}

void apdu_GetResponse( ppApdu_Response_t *ppResponse )
{
	io_GetResponse((ppIso7816Util_Response_t *)ppResponse);
}
/*@ +discast @*/
u16 apdu_GetResLen(void)
{
	return g_ResLen;
}

void apdu_SetResLen(u16 len)
{
	g_ResLen	= len;
}

void apdu_Init(u8 isDeselect)    SMALL_MODE
{
	io_Init(isDeselect);
	apdu_InitVar();
	RamManager_initVal();
	iso7816Channel_Init();
#if JAVA_CARD_UICC	
	//在7816跳转后需要初始化逻辑通道，防止进入deselect中出现异常modify by guobing@20220809
	channel_init();
#endif	
}

u8 apdu_GetProtocol(void)
{
	return io_GetCurrentProtocol();
}

u8 apdu_getCLAChannel(void)
{
//	u8 channel = io_GetLogicalChannel();
//#if JCVM__IO__SWP
//	if (channel >= SWP_BASIC_CHANNEL_NUM)
//	{
//		return (U8_S_U8(channel, SWP_BASIC_CHANNEL_NUM));
//	}
//	else
//#endif
//#if JCVM__IO__SPI
//	if (channel >= SPI_BASIC_CHANNEL_NUM)
//	{
//		return (U8_S_U8(channel, SPI_BASIC_CHANNEL_NUM));
//	}
//	else
//#endif
	{
		return io_GetLogicalChannel();
	}
}

void apdu_FetchComand(PX8 apduBuffer)
{
	io_FetchCommand(apduBuffer);
    return;
}

void apdu_SendStatusAndReceiveCommand(u16 status)
{
	ppIso7816Util_Command_t  	D_A		sr_PCommand;
	u16							D_A		sr_Fullness;
	u16							D_A		sr_ClearLength;
	// free autoRelease blocks in Ram
	RamManager_autoReleaseRam();

	memRefresh();
	io_SetResPrameter(g_ResLen, g_apduLR, status);

	io_SendStatusAndReceiveCommand(status);

	io_GetCommand( &sr_PCommand );

	g_apduLE				= sr_PCommand->Le;
	g_apduLc 				= sr_PCommand->Lc;
	
	/* clear residual information */
	io_FrameReceiveLength	= sr_PCommand->length;
	sr_Fullness				= sr_PCommand->length;

	if(sr_Fullness <  261)
	{
		sr_ClearLength	= U16_S_U16(261, sr_Fullness);
		/*
		if(g_apduLE == 0)
		{
			sr_Fullness 	= U16_S_U8(sr_Fullness,1);
		}*/
		ecpMem_Fill(&sr_PCommand->pData[sr_Fullness], 0, sr_ClearLength);
	}

	apdu_resetAPDU();
}
#if (JAVA_CARD_UICC||BOOTLOADER_PRO)//modify by jiangxiaohui@20220324 分层
void apdu_SendStatusAndReceiveCommandSingle(u16 status)
{
	ppIso7816Util_Command_t  	D_A		sr_PCommand;
	u16							D_A		sr_Fullness;
	//u16							D_A		sr_ClearLength;
	// free autoRelease blocks in Ram
	RamManager_autoReleaseRam();

	memRefresh();
	io_SetResPrameter(g_ResLen, g_apduLR, status);

	io_SendStatusAndReceiveCommandSingle(status);

	io_GetCommand( &sr_PCommand );

	g_apduLE				= sr_PCommand->Le;
	g_apduLc 				= sr_PCommand->Lc;
	
	/* clear residual information */
	io_FrameReceiveLength	= sr_PCommand->length;
	sr_Fullness				= sr_PCommand->length;

	if(sr_Fullness <  261)
	{
		/*
		sr_ClearLength	= U16_S_U16(261, sr_Fullness);
		
		if(g_apduLE == 0)
		{
			sr_Fullness 	= U16_S_U8(sr_Fullness,1);
		}*/
		//ecpMem_Fill(&sr_PCommand->pData[sr_Fullness], 0, sr_ClearLength);
	}

	apdu_resetAPDU();
}
#endif
void apdu_SetLeLc(void)
{
	ppIso7816Util_Command_t  	D_A		sr_PCommand;
	
	io_GetCommand( &sr_PCommand );

	g_apduLE				= sr_PCommand->Le;
	g_apduLc 				= sr_PCommand->Lc;
}

void apdu_CallResetAPDU(void)
{
	apdu_resetAPDU();
}
u16 apdu_ApduSetIncomingAndReceive(void)
{
	u16						D_A		sRcvDataLen_;
	ppIso7816Util_Command_t			ppCommand;

	sRcvDataLen_ = 0;
	io_GetCommand( &ppCommand );

	// if we've previously called this or setOutgoing() method, then throw an exception
	if ( (g_apduStatus.SET_INCOMING == 1 && (0x0 == getGlobalScpAPI())) || (g_apduStatus.SET_OUTGOING == 1) )
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
	else
	{
		if (APDU_PROTOCOL_T0 == io_GetCurrentProtocol())
		{
			if ((0 == g_apduStatus.SYS_SET_INCOMING) && (0 != g_apduLc))
			{
				sRcvDataLen_	= io_Receive();
				g_apduLc		= U16_S_U16(g_apduLc, sRcvDataLen_);
				g_apduLE		= 256;
			}
			else
			{
				sRcvDataLen_	= ppCommand->Lc;
			}
			if (0 != sRcvDataLen_)
			{
				//  Integrate apdu related flag.
				g_apduStatus.SET_INCOMING = 1;
			}

		}
		else
		{
			if(io_FrameReceiveLength >= 261)
			{
				sRcvDataLen_ = 255;
			}
			else if(io_FrameReceiveLength < 5)
			{
				sRcvDataLen_ = 0;
			}
			else
			{
				if(IOCONFIG_CBCTEST_RECEIVELENTH == 0x04)
				{
					sRcvDataLen_	= U8t16(ppCommand->pData[4]);
				}
				else
				{
					sRcvDataLen_	= U16_S_U8(io_FrameReceiveLength, 5);
				}
				
				//if LE is exist 
				if(U16t8(U16_S_U8(sRcvDataLen_, 1)) == ppCommand->pData[4])
					sRcvDataLen_ = U8t16(ppCommand->pData[4]);

			}
			g_apduLc					= sRcvDataLen_;
			g_apduStatus.SET_INCOMING	= 1;
		}
	}

	Current_status	= STATE_FULL_INCOMING;

	return sRcvDataLen_;
}

void apdu_UndoSetIncomingAndReceive(void)
{
	Current_status					= STATE_INITIAL;
	g_apduStatus.SET_INCOMING		= 0;
	g_apduStatus.SYS_SET_INCOMING	= 1;
}

void apdu_ApduSendBytes(u16 bOff, u16 len)
{
	u16							D_A		Lr;
	ppIso7816Util_Command_t		D_A		command;
	ppIso7816Util_Response_t	D_A		respond;

	io_GetCommand( &command );
	io_GetResponse( &respond );

	Lr			= g_apduLR;

	if (len == 0)
	{
		return;
	}

	if (len > g_apduLR || 0 == g_apduStatus.SET_OUTGOING_LEN || 1 == g_apduStatus.SET_OUTGOING_AND_SEND)
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
	if ((0x8000 == U16_AND_U16(bOff, 0x8000)) || (0x8000 == U16_AND_U16(len, 0x8000)) || (U16_A_U16(bOff , len) > APDU_BUFFER_SIZE))
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_BUFFER_BOUNDS);
	}

	ecpMem_Move(&respond->pData[g_curResOffset], &command->pData[bOff], len);
	g_ResLen		= U32t16(U16_A_U16(g_ResLen , len));
	g_curResOffset	= U32t16(U16_A_U16(g_curResOffset , len));
	Lr				= U16_S_U16(Lr , len);

	//  check interface contacted or contactless
	if (APDU_PROTOCOL_T0 != io_GetCurrentProtocol())
	{
		if (g_apduStatus.SET_INCOMING == 1)
		{
			Current_status = STATE_FULL_OUTGOING;
		}
		else
		{
			Current_status = STATE_PARTIAL_OUTGOING;
		}
	}
	else
	{
		if (Lr == 0)
		{
			Current_status = STATE_FULL_OUTGOING;
			g_apduStatus.SET_INCOMING  = 0;
		}
		else
		{
			Current_status = STATE_PARTIAL_OUTGOING;
			g_apduStatus.SET_INCOMING  = 1;
		}
	}

	return;
}

void apdu_ApduSetOutgoingAndSend(u16 bOff, u16 len)    SMALL_MODE
{
	u8	callBackResult;

	callBackResult	= false;
	
/*@ -type @*/
	if(APDU_NULL_CALL_BACK != g_setOutGoingCallBack)
	{
		callBackResult	= g_setOutGoingCallBack(bOff, len);
	}
/*@ +type @*/
	if(false == callBackResult)
	{
		if (1 == g_apduStatus.SET_OUTGOING  || 1 == g_apduStatus.SET_OUTGOING_LEN )
		{
			bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
		}
		g_apduStatus.SET_OUTGOING			= 1;

		if (len > 256 || (U16_AND_U16(len, 0x8000) == 0x8000))
		{
			bootErrorHandle_ThrowError(APDU_EXCEPTION_BAD_LENGTH);
		}
		g_apduLR = len;

		g_apduStatus.SET_OUTGOING_LEN		= 1;
		apdu_ApduSendBytes(bOff, len);
		g_apduStatus.SET_OUTGOING_AND_SEND	= 1;
	}
}

void apdu_InitSetOutGoingCallBack(SETOUTGOING_CALLBACK * callBackFun)
{
	g_setOutGoingCallBack	= callBackFun;
}

boolean apdu_IsSetIncoming(void)
{
	return g_apduStatus.SET_INCOMING;
}

void apdu_SuspendAllInterrupts(void)
{
	io_SuspendAllInterrupts();
}

void apdu_ResumeAllInterrupts(void)
{
	io_ResumeAllInterrupts();
}


#if BOOT__IO__APDU_API
u8 apdu_GetCurrentState(void)
{
	return Current_status;
}

u16 apdu_getInBlockSize(void)
{
	if (APDU_PROTOCOL_T0 == io_GetCurrentProtocol())
	{
		return 1;
	}
	else if (APDU_PROTOCOL_T1 == io_GetCurrentProtocol())
	{
		return 254;
	}
	else
	{
		return 253;
	}
}

u16 apdu_GetOutBlockSize(void)
{
	u8 protocol = io_GetCurrentProtocol();
	
	if (APDU_PROTOCOL_T0 == protocol)
	{
		return 258;
	}
	else if (APDU_PROTOCOL_T1 == protocol)
	{
		return 254;
	}
	else if ( (PROTOCOL_SWP == protocol) || (PROTOCOL_SPI == protocol) || (PROTOCOL_I2C == protocol) )
	{
		return 258;
	}
	else
	{
		return 252;
	}
}

void apdu_WaitExtension(void)
{
	if (1 == g_apduStatus.SET_OUTGOING_NO_CHAINING)
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
}

u16 apdu_SetOutgoing(void)
{
 	if (g_apduStatus.SET_OUTGOING == 1)
	{
 		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
	else
	{
		g_apduStatus.SET_OUTGOING = 1;
		Current_status = STATE_OUTGOING;
	}


#if BOOT__IO__APDU_EXTENDED_LENGTH
	if (1 == g_apduStatus.extendedAPDUFlag)
	{
		return 32767;
	}
	else
#endif
	{
		return g_apduLE;
	}
}

void apdu_SendBytesLong(PM8 ArrayAddr, u16 bOff, u16 len)   SMALL_MODE
{
	u16						D_A		sendLength;
	ppIso7816Util_Command_t	X_A		command;

	sendLength	= 261;
	io_GetCommand(&command);

	if ((g_apduStatus.SET_OUTGOING_LEN == 0) || (g_apduStatus.SET_OUTGOING_AND_SEND == 1))
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
	//  Make sure the outgoing length is positive.
	while (len > 0)
	{
		if (len < sendLength)
		{
			sendLength	= len;
		}
		SC_InCheckBack(ArrayAddr + bOff + len);
		memCopyNoAtomic(command->pData, PM8_A_U16(ArrayAddr, bOff), sendLength);
		SC_InCheckBack(ArrayAddr +  bOff + len);
		apdu_ApduSendBytes(0, sendLength);
		//len -= sendLength;
		//bOff += sendLength;
		len		= U16_S_U16(len , sendLength);
		bOff	= U32t16(U16_A_U16(sendLength , bOff));
	}
	Current_status = STATE_FULL_OUTGOING;
}

/**
 * This method is used to set the data transfer direction to
 * outbound without using BLOCK CHAINING(See ISO 7816-3/4) and to obtain the expected length of response (Le).
 */
u16 apdu_SetOutgoingNoChaining(void)
{
	if (g_apduStatus.SET_OUTGOING == 1)
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
	else
	{
		g_apduStatus.SET_OUTGOING				= 1;
		g_apduStatus.SET_OUTGOING_NO_CHAINING	= 1;
		Current_status = STATE_OUTGOING;
	}

#if BOOT__IO__APDU_EXTENDED_LENGTH
	if (1 == g_apduStatus.extendedAPDUFlag)
	{
		return 32767;
	}
	else
#endif
	{
		return g_apduLE;
	}
}


/**
 * Sets the actual length of response data. Default is 0.
 */
void apdu_SetOutgoingLength(u16 len)    SMALL_MODE
{
	if ((g_apduStatus.SET_OUTGOING == 0) || (g_apduStatus.SET_OUTGOING_LEN == 1))
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}

	if (0x8000 == U16_AND_U16(len, 0x8000))
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_BAD_LENGTH);
	}
	
	if(len > 256)
	{
#if BOOT__IO__APDU_EXTENDED_LENGTH
		if(1 != g_apduStatus.extendedAPDUFlag)
#endif
		{
			bootErrorHandle_ThrowError(APDU_EXCEPTION_BAD_LENGTH);
		}
	}
	
	g_apduLR = len;

	if (APDU_PROTOCOL_TA == io_GetCurrentProtocol())
	{
		//TODO:check (len > 250)
		if (1 == g_apduStatus.SET_OUTGOING_NO_CHAINING && len > 250)
		{
			bootErrorHandle_ThrowError(APDU_EXCEPTION_BAD_LENGTH);
		}
	}
	else if (APDU_PROTOCOL_T1 == io_GetCurrentProtocol())
	{
		//TODO:check (len > 250)
		if (1 == g_apduStatus.SET_OUTGOING_NO_CHAINING && len > 252)
		{
			bootErrorHandle_ThrowError(APDU_EXCEPTION_BAD_LENGTH);
		}
	}
	else
	{
		if (1 == g_apduStatus.SET_OUTGOING_NO_CHAINING && len > 256)
		{
			bootErrorHandle_ThrowError(APDU_EXCEPTION_BAD_LENGTH);
		}
	}

	Current_status = STATE_OUTGOING_LENGTH_KNOWN;
	g_apduStatus.SET_OUTGOING_LEN = 1;
}

/**
 * Gets as many data bytes as will fit without APDU buffer overflow,
 * at the specified offset bOff.&nbsp; Gets all the remaining bytes if they fit.
 */
u16 apdu_ReceiveBytes(u16 bOff)
{
	if (0x8000 == U16_AND_U16(bOff, 0x8000) || bOff > 256)
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_BUFFER_BOUNDS);
	}

	if ((0 == g_apduStatus.SET_INCOMING) || (1 == g_apduStatus.SET_OUTGOING )|| (1 == g_apduStatus.SET_OUTGOING_NO_CHAINING) )
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}
	else
	{

		Current_status = STATE_FULL_INCOMING;
	}

	return 0;
}

u16 apdu_GetIncomingLength(void)
{
	if(0 == g_apduStatus.SET_INCOMING || 1 == g_apduStatus.SET_OUTGOING ||1 == g_apduStatus.SET_OUTGOING_NO_CHAINING)
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}

	return g_apduLc;
}

u8 apdu_GetNAD(void)
{
	if (APDU_PROTOCOL_T0 == io_GetCurrentProtocol())
	{
		return 0;
	}
	else if(APDU_PROTOCOL_T1 == io_GetCurrentProtocol())
	{
		return iso7816T1_GetNad();
	}
	else
	{
		return 0 ;		 //添加非接的NAD
	}
}

u16 apdu_GetOffsetCdata(void)
{
	if(0 == g_apduStatus.SET_INCOMING || 1 == g_apduStatus.SET_OUTGOING ||1 == g_apduStatus.SET_OUTGOING_NO_CHAINING)
	{
		bootErrorHandle_ThrowError(APDU_EXCEPTION_ILLEGAL_USE);
	}

#if BOOT__IO__APDU_EXTENDED_LENGTH
	if (1 != g_apduStatus.extendedAPDUFlag)
	{
		return U16_AND_U16(APDU_CLA_OFFSET, 0x7FFF);
	}
	else
	{
		return U16_AND_U16(APDU_EXT_CDATA_OFFSET, 0x7FFF);
	}
#else
	{
		return U16_AND_U16(APDU_CLA_OFFSET, 0x7FFF);
	}
#endif
}

/**
 * Returns whether the current APDU command is the first or
 * part of a command chain. Bit b5 of the CLA byte if set, indicates that
 * the APDU is the first or part of a chain of commands.
 *
 * Note:
 *
 * This method returns false if the CLA bits (b8,b7,b6) is
 * %b001 which is a CLA encoding reserved for future use(RFU),
 * or if CLA is 0xFF which is an invalid value as defined in
 * the ISO 7816-4:2005 specification.
 *
 *
 * See Runtime Environment Specification, Java Card
 * Platform, Classic Edition, section 4.3 for encoding details.
 *
 *
 * @return true if this APDU is not the last APDU of a
 * command chain, false otherwise.
 * @since 2.2.2
 */
u8 apdu_IsCommandChainingCLA(void)
{
	ppIso7816Util_Command_t		D_A		command;

	io_GetCommand(&command);

	if (!apdu_IsValidCLA())
	{
		return false;
	}

	return ((U8_AND_U8(command->pData[APDU_CLA_OFFSET], APDU_CHAINING_MASK) == 0x10) ? true : false);
}


/**
 * Returns whether the current APDU command CLA byte is
 * valid. The CLA byte is invalid if the CLA bits (b8,b7,b6) is %b001, which
 * is a CLA encoding reserved for future use(RFU), or if CLA is 0xFF which
 * is an invalid value as defined in the ISO 7816-4:2005 specification.
 *
 * See Runtime Environment Specification, Java Card
 * Platform, Classic Edition, section 4.3 for encoding details.
 *
 *
 * @return true if this APDU CLA byte is valid,
 * false otherwise.
 * @since 3.0
 */
u8 apdu_IsValidCLA(void)
{
	ppIso7816Util_Command_t		D_A		command;

	io_GetCommand(&command);

	return ((U8_AND_U8(command->pData[APDU_CLA_OFFSET], 0xE0) == 0x20) || (command->pData[APDU_CLA_OFFSET] == 0xFF)) ? false:true;
}

/**
 * Returns whether the current APDU command CLA byte
 * corresponds to an interindustry command as defined in ISO 7816-4:2005
 * specification. Bit b8 of the CLA byte if 0, indicates
 * that the APDU is an interindustry command.
 *
 * @return true if this APDU CLA byte corresponds to an ISO
 * interindustry command, false otherwise.
 * @since 2.2.2
 */
u8 apdu_IsISOInterindustryCLA(void)
{
	ppIso7816Util_Command_t		D_A		command;
	io_GetCommand(&command);

	return (U8_AND_U8(command->pData[APDU_CLA_OFFSET], APDU_ISOCLASS_MASK) == 0x00) ? true : false;
}

/**
 * Returns true if the encoding of the current
 * APDU command based on the CLA byte indicates secure
 * messaging. The secure messaging information is in bits (b4,b3) for
 * commands with origin channel numbers 0-3, and in bit b6 for origin
 * channel numbers 4-19.
 *
 * Note:
 *
 * This method returns false if the CLA bits (b8,b7,b6) is
 * %b001 which is a CLA encoding reserved for future use(RFU),
 * or if CLA is 0xFF which is an invalid value as defined in
 * the ISO 7816-4:2005 specification.
 *
 *
 * See Runtime Environment Specification, Java Card
 * Platform, Classic Edition, section 4.3 for encoding details.
 *
 *
 * @return true if the secure messaging bit(s) is(are)
 * nonzero, false otherwise
 * @since 2.2.2
 */
u8 apdu_IsSecureMessagingCLA(void)
{
	u8										secureMsgFlag;
	boolean									temp;
	ppIso7816Util_Command_t				D_A	command;

	io_GetCommand(&command);

	temp	= ((U8_AND_U8(command->pData[APDU_CLA_OFFSET], APDU_ISType16CLA_MASK) == 0x40) ? true : false);

	if (!apdu_IsValidCLA())
	{
		return false;
	}
	if (temp)
	{
		secureMsgFlag	= U8_AND_U8(command->pData[APDU_CLA_OFFSET], APDU_SM_MASK_TYPE16);
	}
	else
	{
		secureMsgFlag	= U8_AND_U8(command->pData[APDU_CLA_OFFSET], APDU_SM_MASK_TYPE4);
	}

	if (secureMsgFlag == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

#endif	//BOOT__IO__APDU_API

void apdu_SendStatus(u16 status)
{
	io_SetResPrameter(g_ResLen, g_apduLR, status);

	wtxTimer_WtxTimerStop();

	//io_SendStatus(status);
	iso7816T0_SendStatus();
}

#endif	//BOOT__IO__APDU
