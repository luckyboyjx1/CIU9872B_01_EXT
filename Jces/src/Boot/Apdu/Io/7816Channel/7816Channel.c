#include "7816Channel.h"
#include "7816Util.h"
#include "BootRam.h"
#include "ecpDefine.h"
#include "SysConfig.h"
#include "global_variable.h"
#include "uicc.h" // modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if BOOT__IO__ISO7816

#pragma _7816_CHANNEL_RAM_START

static u8	X_A  g_curLogicalChn;

_7816_CHANNEL_RAM_END

#define LOGICAL_CHN_MASK  			0x03
#define EXTEND_LOGICAL_CHN_MASK  	0x0F
#define APDU_TYPE_MASK    			0xF0

#define	LOGICAL_CHN_TYPE_16			0x02
#define	LOGICAL_CHN_TYPE_4			0x01
#define	LOGICAL_CHN_TYPE_RFU		0x00

static u8 CONST LOGICAL_CHN_INFO_EX[] =
{
	LOGICAL_CHN_TYPE_4,		//(0x00 == theAPDUType) -- %b0000 uuzz (Type 4) last or only command in chain
	LOGICAL_CHN_TYPE_4,		//(0x10 == theAPDUType) -- %b0001 uuzz (Type 4) not last command in chain
	
	LOGICAL_CHN_TYPE_RFU,	//(0x20 == theAPDUType) -- %b0010 uuuu RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x30 == theAPDUType) -- %b0011 uuuu RFU
	
	LOGICAL_CHN_TYPE_16,	//(0x40 == theAPDUType) -- %b0100 zzzz (Type 16) last or only command in chain, no SM
	LOGICAL_CHN_TYPE_16,	//(0x50 == theAPDUType) -- %b0101 zzzz (Type 16) not last command in chain, no SM
	LOGICAL_CHN_TYPE_16,	//(0x60 == theAPDUType) -- %b01y0 zzzz (Type 16) last or only command in chain, with SM
	LOGICAL_CHN_TYPE_16,	//(0x70 == theAPDUType) -- %b01y1 zzzz (Type 16) not last command in chain, with SM
	
	LOGICAL_CHN_TYPE_4,		//(0x80 == theAPDUType) -- %b1000 uuzz (Type 4) last or only command in chain
	LOGICAL_CHN_TYPE_4,		//(0x90 == theAPDUType) -- %b1001 uuzz (Type 4) not last command in chain
	LOGICAL_CHN_TYPE_4,		//(0xA0 == theAPDUType) -- %b1010 uuzz (Type 4) last or only command in chain
	LOGICAL_CHN_TYPE_4,		//(0xB0 == theAPDUType) -- %b1011 uuzz (Type 4) not last command in chain
	
	LOGICAL_CHN_TYPE_16,	//(0xC0 == theAPDUType) -- %b1100 zzzz (Type 16) last or only command in chain. no SM
	LOGICAL_CHN_TYPE_16,	//(0xD0 == theAPDUType) -- %b1101 zzzz (Type 16) not last command in chain, no SM
	LOGICAL_CHN_TYPE_16,	//(0xE0 == theAPDUType) -- %b11y0 zzzz (Type 16) last or only command in chain, with SM
	LOGICAL_CHN_TYPE_16		//(0xF0 == theAPDUType) -- %b11y1 zzzz (Type 16) not last command in chain, with SM
};

static u8 CONST LOGICAL_CHN_INFO[] =
{
	LOGICAL_CHN_TYPE_4,		//(0x00 == theAPDUType) -- %b0000 uuzz (Type 4) last or only command in chain
	
	LOGICAL_CHN_TYPE_RFU,	//(0x10 == theAPDUType) -- %b0001 uuzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x20 == theAPDUType) -- %b0010 uuuu RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x30 == theAPDUType) -- %b0011 uuuu RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x40 == theAPDUType) -- %b0100 zzzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x50 == theAPDUType) -- %b0101 zzzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x60 == theAPDUType) -- %b0110 zzzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0x70 == theAPDUType) -- %b0111 zzzz RFU
	
	LOGICAL_CHN_TYPE_4,		//(0x80 == theAPDUType) -- %b1000 uuzz (Type 4) last or only command in chain
	LOGICAL_CHN_TYPE_4,		//(0x90 == theAPDUType) -- %b1001 uuzz (Type 4) not last command in chain
	LOGICAL_CHN_TYPE_4,		//(0xA0 == theAPDUType) -- %b1010 uuzz (Type 4) last or only command in chain
	
	LOGICAL_CHN_TYPE_RFU,	//(0xB0 == theAPDUType) -- %b1011 uuzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0xC0 == theAPDUType) -- %b1100 zzzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0xD0 == theAPDUType) -- %b1101 zzzz RFU
	LOGICAL_CHN_TYPE_RFU,	//(0xE0 == theAPDUType) -- %b1110 zzzz RFU
	LOGICAL_CHN_TYPE_RFU	//(0xF0 == theAPDUType) -- %b1111 zzzz RFU
};

void iso7816Channel_Init(void)
{
	g_curLogicalChn	= 0;
}


u8 iso7816Channel_CalLogicalChannel(u8 cla)
{
	u8 							D_A		theAPDUType;
	u8							D_A		LogicalChn;

	PATCH_BOOT_APDU_7816CHANNEL_001();


	theAPDUType				= U8_AND_U8(cla, APDU_TYPE_MASK) >> 4;

	// Verify if CLA byte contains channel information, according
	// to ISO 7816-4 section 5.4
	if(BOOT_CONFIG_EXTENDED_LOGICAL_CHANNEL)
	{
		theAPDUType 		= LOGICAL_CHN_INFO_EX[theAPDUType];
	}
	else
	{
		theAPDUType 		= LOGICAL_CHN_INFO[theAPDUType];
	}

	if (LOGICAL_CHN_TYPE_16 == theAPDUType)
	{
		LogicalChn		= U8_A_U8(U8_AND_U8(cla, EXTEND_LOGICAL_CHN_MASK), 4);
	}
	else if(LOGICAL_CHN_TYPE_4 == theAPDUType)
	{
		LogicalChn 		= U8_AND_U8(cla, LOGICAL_CHN_MASK);
	}
	else
	{
		LogicalChn 		= 0;
	}

	return LogicalChn;

}

void iso7816Channel_SetLogicalChannel(void)
{
	PX8							D_A		commandBuffer;
	ppIso7816Util_Command_t		D_A		comand;
	
	PATCH_BOOT_APDU_7816CHANNEL_001();
	
	iso7816Util_GetCommand( &comand );
	
	commandBuffer 			= comand->pData;

	g_curLogicalChn = iso7816Channel_CalLogicalChannel(commandBuffer[0]);
	
	comand->chn				= g_curLogicalChn;

	return;
}

u8 iso7816Channel_GetCurLogicalChn(void)
{
	return g_curLogicalChn;
}

#endif
