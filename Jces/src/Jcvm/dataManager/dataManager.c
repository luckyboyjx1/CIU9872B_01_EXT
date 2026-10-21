#include "cardManager.h"
#include "opFunction.h"
#include "dataManager.h"
#include "object.h"
#include "error.h"
#include "tagNode.h"
#include "securityDomainData.h"
#include "KeyManagerOld.h"
#include "opFunction.h"
#include "context.h"
#include "ecpDefine.h"
#include "anomaly.h"
#include "global_variable.h"
#include "nativemethod_impl.h"
#include "tranAllocate.h"
#include "JcvmRam.h"
#include "CgmManager.h"
#include "KeyLogicManager.h"
#include "casdStoreKey.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "SCP11.h"
#include "SCPInterface.h"
//modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11，将宏定义移动到dataManager.h中
#include "Interface_LTEFromPlatform.h"

#if GP__DATA_MANAGER__FOR_MASK

u8			g_handle0070;
u8			g_isLink;
u8			g_storeDataSequence[MAX_LOGICAL_CHANNELS];
u8			g_BitTagValue[MAX_LOGICAL_CHANNELS];
u8			g_storeDataState[MAX_LOGICAL_CHANNELS];
u8			g_storeDataOffset[MAX_LOGICAL_CHANNELS];
u16			g_storeDataRemaindLen[MAX_LOGICAL_CHANNELS];
u16			g_curStoreDataTag;
u16			g_curStoreDataTagOffset[MAX_LOGICAL_CHANNELS];
u16			g_curStoreDataTagLength[MAX_LOGICAL_CHANNELS];
u16			g_curStoreDataDgiLength[MAX_LOGICAL_CHANNELS];
memref		g_tagArrayContent[MAX_LOGICAL_CHANNELS];


void dataManager_init(void)
{
    g_handle0070						= 0;
	g_isLink                            = 0;
	ecpMem_Set(g_BitTagValue, 0, sizeof(moreDataFlag));
	ecpMem_Set(g_storeDataSequence, 0, sizeof(g_storeDataSequence));
	ecpMem_Set(g_curStoreDataDgiLength, 0 , sizeof(g_curStoreDataDgiLength));
}

void dataManager_reset(void)
{
	g_storeDataSequence[selected_channel] 			= 0;
	g_handle0070						  			= 0;
	g_curStoreDataDgiLength[selected_channel]		= 0;
	if(CHAINING_STOREDATA == g_moreCommands) //add by xuxj@20230418--26.eUICC_SE_TestSE_Test/SE和Profile相互影响性测试/SE_Profile_InterrelateTest_04
	{
		g_moreCommands = CHAINING_CLEAR;
	}
}

void dataManager_abortStoreData(void)
{
	dataManager_reset();
	if(isInProgress())
	{
		abortTransaction();
	}
}
/*
 *modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11 将dataManager_abortStoreData()放开，之前不清楚为什么注释掉
 */
void dataManager_abortStoreDataThrowError(u16 error)
{
	dataManager_abortStoreData();	//gp2.3-UL-DS测试中，有用例测试到存储多个tag时，有任意一个tag检查和写入失败，则全部tag都应该不写入
	throw_error(error);
}

u16 dataManager_calcTagAttri(u8 is0070Group)
{
	u16	tempAttri;
	
	tempAttri		= 0;
	
	if(true == is0070Group)
	{
		tempAttri	= U16_OR_U16(tempAttri, IS_0070_GROUP_TAG);
	}
	
	return tempAttri;
}

/*=====================================================handle store data=============================================================*/
/*===================================================================================================================================*/
static void dataManager_decreaseDgiLength(u16 length)
{
		if(DGI_HANDLE_FINISH != g_curStoreDataDgiLength[selected_channel])
		{
			g_curStoreDataDgiLength[selected_channel]	= U16_S_U16(g_curStoreDataDgiLength[selected_channel], length);
		}
}
/*
 *modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
static boolean dataManager_handleMappingDGI()
{
	u16 curStoreDataTag = 0;
	curStoreDataTag		= PX_R_U16(&APDUBuffer[g_storeDataOffset[selected_channel]]);
	switch(curStoreDataTag)
	{

//#if GP__AMENDMENT__B__FOR_MASK
		case 0x00B9:
#if GP__SCP__SCP11__FOR_MASK
			if((PX_R_U8(g_current_SCP11SessionPara_sessionType) == SCP11_C))// modify by xuxiaojun@20220927： RAM优化，g_current_SCP11SessionPara用全局变量，不使用结构体
			{
				closeSecuritychannel(SC_TERMINATED);// modify by liweimin

				dataManager_abortStoreDataThrowError(SW_CONDITIONS_NOT_SATISFIED);
			}
#endif
//			return true;
//#endif
#if GP__AMENDMENT__B__FOR_MASK
//		case 0x0085:
		case 0x00F5:
#endif
		case 0x0070:
		case 0x00CF: // Mapping_guidelines_v1.0.1-Final.pdf,  Key Derivation Data
//#if GP__KEY_MANAGER__STORE_KEY
		case DES_SECURE_CHANNEL_KEY: // 8F01 Mapping_guidelines_v1.0.1-Final.pdf,  Secure Channel DES keys
		case DAP_VERIFICATION_KEY:   // 9102 Mapping_guidelines_v1.0.1-Final.pdf,  DAP Verification key information
//#endif
#if  VM_WITH_DCEP_ZZH
		case 0x00D1:   //mergerd from FINANCE_TMC_THD89: modify by ydd@20220610
#endif
		case 0x9F70: // Mapping_guidelines_v1.0.1-Final.pdf,  Card Life Cycle State

			//g_handle0070						= false;
			g_storeDataState[selected_channel]	= STORE_DATA_GET_DGI;
			return true;
	}
	return false;
}
static u8 dataManager_getStoreDataInitState(u8 p1)
{
	#define		DATA_FIELD_FORMAT_MASK		0x18
	#define		DGI_FORMAT					0x08
	#define		BER_TLV_FORMAT				0x10
	
	if((DGI_FORMAT == U8_AND_U8(DATA_FIELD_FORMAT_MASK , p1)) && (DGI_HANDLE_FINISH == g_curStoreDataDgiLength[selected_channel]))
	{
		g_handle0070		= false;
		return STORE_DATA_GET_DGI;
	}
	/*else if(BER_TLV_FORMAT == (DATA_FIELD_FORMAT_MASK & p1))
	{
		return STORE_DATA_GET_TAG1;
	}
	else if(DGI_HANDLE_FINISH == g_curStoreDataDgiLength)
	{
		g_handle0070		= false;
		return STORE_DATA_GET_DGI;
	}*/
	else
	{
		return STORE_DATA_GET_TAG1;
	}
}

void dataManager_checkSequence(u8 p2)
{
	#define	LAST_BLOCK		0x80
	#define  BIT0_TAG		0x01
	//  if p2 is not Sequence and p2 is not checked before, then throw error
	if((p2 != g_storeDataSequence[selected_channel]) && (true != dataManager_getBitTag(BIT0_TAG)))
	{
		dataManager_abortStoreDataThrowError(SW_INCORRECT_P1P2);
	}
}

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
void cms2ac_dataManager_checkSequence(u8 p1)
	{
		if((0 != bRamBuffer[INSTALL_FOR_PERSONERLIZE]) && ((cc & APPLET_MASK) != bRamBuffer[INSTALL_FOR_PERSONERLIZE]))
		{
			if((p1!=0x00) && (p1!=0x80) && (p1 != 0x60))
			{
				dataManager_abortStoreDataThrowError(SW_INCORRECT_P1P2);
			}
		}
		else
		{
			if((p1!=0x00) && (p1!=0x80))
			{
				dataManager_abortStoreDataThrowError(SW_INCORRECT_P1P2);
			}
		}	
	}
#endif
static u8 dataManager_isLink(u8 p1)
{
	#define		BIT0_TAG		0x01
	if(LAST_BLOCK == U8_AND_U8(p1 , LAST_BLOCK))
	{
		dataManager_setBitTag(BIT0_TAG);
		g_storeDataSequence[selected_channel]	= 0;

		return false;
	}
	else
	{
		if(false == dataManager_getBitTag(BIT0_TAG))
		{
			 U8_INC(g_storeDataSequence[selected_channel]);
			dataManager_setBitTag(BIT0_TAG);
		}
		return true;
	}
}
/**
 * 该函数用于检测TAG只能单独存在，不允许被嵌入其他的DGI中
 */
static void dataManager_checkSingleCommandTag(u8 remaindLen)
{
	//the tags should be included in one store data command
	switch(g_curStoreDataTag)
	{
		case 0x0070:
		case 0x00CF: // Mapping_guidelines_v1.0.1-Final.pdf,  Key Derivation Data
		case 0x0102: // Mapping_guidelines_v1.0.1-Final.pdf,  DAP Verification key related data
		case 0x7F01: // Mapping_guidelines_v1.0.1-Final.pdf,  DES key related data
#if GP__KEY_MANAGER__STORE_KEY
		case DES_SECURE_CHANNEL_KEY: // 8F01  Mapping_guidelines_v1.0.1-Final.pdf,  Secure Channel DES keys
		case DAP_VERIFICATION_KEY:   // 9102  Mapping_guidelines_v1.0.1-Final.pdf, DAP Verification key information
#endif
		case 0x9F70: // Mapping_guidelines_v1.0.1-Final.pdf,  Card Life Cycle State
//#if VGP_CONFIG
		case 0x9F67: // Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf: 8 字节  IC Pre-Personalizer Data
		case 0x9F66: // Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf: 8 字节  IC Personalizer Data
		case 0xDF30:
//#endif
			//接收完命令后才进行判断
			if (U8_AND_U8(APDUBuffer[APDU_P1_OFFSET] , LAST_BLOCK) == LAST_BLOCK)
			{
				if(U8t16(remaindLen) < g_curStoreDataTagLength[selected_channel])
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_LENGTH);
				}
			}
			break;
		default:
			break;
	}
}

static void dataManager_checkTagLength(u8 remaindLen)
{
	//the tags has the constant length
	switch(g_curStoreDataTag)
	{
		case 0x00CF:
			if(0x0A != g_curStoreDataTagLength[selected_channel])
			{
				dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
			}
			break;
		case 0x9F70:
			if(0x01 != g_curStoreDataTagLength[selected_channel])
			{
				dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
			}
			break;
		case 0x004F:
		if((0x05 > g_curStoreDataTagLength[selected_channel]) || (0x10 < g_curStoreDataTagLength[selected_channel]))
		{
			dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
		}
		break;
		case 0x00CD:
		if(0x01 != g_curStoreDataTagLength[selected_channel])
		{
			dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
		}
	}
}
/*
 *The command shall be rejected if such data objects are presented in DGI '0070' or presented in BER-TLV format.
 */
static u8 dataManager_storeData0070TagAnalyse(void)
{
	if(g_handle0070)
	{
		//the tags is not support
		switch(g_curStoreDataTag)
		{
			case 0x009F:
			case 0x00C0:
			case 0x00C1: //GPC_Specification_v2.3.1.pdf  Tag 'C1': Sequence Counter of the default Key Version Number
			case 0x00E0:
			case 0x00CF:
			case 0x00C2: //GPC_Specification_v2.3.1.pdf  Confirmation Counter
			case 0x00D3: //GPC_Specification_v2.3.1.pdf  Tag 'D3': Current Security Level
			case 0x2F00: //GPC_Specification_v2.3.1.pdf  List of Applications associated with the Security Domain
			case 0xFF21: //GPC_Specification_v2.3.1.pdf  Extended Card Resources Information available for Card Content Management
				dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
				break;
		}
	}
	return STORE_DATA_SELF_HANDLE_TAG;
}
static void dataManager_storeDataCheckIllegalTag(void)
{
		//the tags is not support
	switch(g_curStoreDataTag)
	{
		case 0x0000:
		case 0x00FF:
		case 0x00C0:
		case 0x00C1:
		case 0x00E0:
//#if GP__KEY_MANAGER__STORE_KEY
		case 0x0102: // Mapping_guidelines_v1.0.1-Final.pdf,  DAP Verification key related data
//#endif
		case 0x7F01:
	#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		case 0xFF20:
		case 0x004E:
		case 0x00E2:
		case 0x2F00:
		case 0x2F16:
	#endif
			dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
			break;
	#if (GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK | GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK)
		case 0x2F12:
			if(JcvmConfig_GetCardState() >= CARD_SECURED)
				dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
			break;
	#endif		
		
	}
}

static u8 dataManager_storeDataDGIandTagAnalyse(u8 remaindLen)
{

	dataManager_checkSingleCommandTag(remaindLen);
	dataManager_checkTagLength(remaindLen);
	dataManager_storeDataCheckIllegalTag();

	if(0x0070 == g_curStoreDataTag)
	{
		g_handle0070		= true;
		//begin multi apdu transaction
		return STORE_DATA_GET_TAG1;
	}

	//the tags are handled in java layer
	switch(g_curStoreDataTag)
	{
		case 0x004F:
		case 0x00A0:
		case 0x00A1:
		case 0x00CD:
		case 0x9F70:

		case 0x9F66: //Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0:  6.13.2.3  Data field sent in the command message, 在JAVA层处理
		case 0x9F67: //Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0:  6.13.2.3  Data field sent in the command message, 在JAVA层处理
		
		case 0x7F21:
		case 0x00AE:
		case 0x00DE:
		case 0x8010:
		case 0x00B8:
#if		GP__AMENDMENT__A__FOR_MASK
		case 0x00A6:
#endif
#if GP__KEY_MANAGER__STORE_DGI_KEY            
		case KEY_INF_DAT_DGI: // 00B9
		case RSA_N_DGI: // 0010
		case RSA_D_DGI: // 8112
		case KS_DGI:    // 8113
		case RSA_CRT_IQP_DGI: // 8121
		case RSA_CRT_DQ_DGI:  // 8122
		case RSA_CRT_DP_DGI:  // 8123
		case RSA_CRT_Q_DGI:   // 8124
		case RSA_CRT_P_DGI:   // 8125
#endif        
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK || GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK 
		case 0x2F14:
#endif
			return STORE_JAVA_TAG;
	}

	return STORE_DATA_GET_TAG_ARRAY_AFEILD;
}

static u8 dataManager_selfHandleTag(PN8 sdDataObj)
{
	switch(g_curStoreDataTag)
	{
#if GP__KEY_MANAGER__STORE_KEY
		case DES_SECURE_CHANNEL_KEY:  //8F01  Mapping_guidelines_v1.0.1-Final.pdf,  Secure Channel DES keys
		case DAP_VERIFICATION_KEY:    //9102  Mapping_guidelines_v1.0.1-Final.pdf, DAP Verification key information
			storeKey(sdDataObj);
			return STORE_DATA_FINISH;
#endif
		default:
			break;
	}
	
	return STORE_DATA_GET_LENGTH;
}

static memref dataManager_getTagArrayAfield(jref firstTagNodeRef, u16 tag, u16 tagLength, u8 is0070Group, u8 isAppend)
{
	jref			tagArrayRef;
	u16				tagAttri;
	PN8		tagArrayObj;
	memref			tagArrayAField;

#if ! GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u16				tagArrayMostLength;
#endif

	tagAttri		= dataManager_calcTagAttri(is0070Group);
	tagArrayRef		= tagNode_getTagArray(firstTagNodeRef, tag, tagLength, tagAttri, MEM_NULL, isAppend);
	if(REF_NULL == tagArrayRef)
	{
		dataManager_abortStoreDataThrowError(SW_REFERENCED_DATA_NOT_FOUND);
	}
	tagArrayObj			= MEMtPN8(locateObjectInMemory(tagArrayRef));
	tagArrayAField		= get_array_data(tagArrayObj);

#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	tagArrayMostLength	= get_array_length(tagArrayObj);
	tagArrayMostLength	= U16_S_U16(tagArrayMostLength, TAG_ARRAY_HEADER_SIZE);
	if(tagLength > tagArrayMostLength)
	{
		dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
	}
#endif

	return tagArrayAField;
}

static u8 dataManager_storeValue(memref tagArrayContent, u8 p1)
{
	u8				nextState;
	u16				storeLength;

	if(g_storeDataRemaindLen[selected_channel] > g_curStoreDataTagLength[selected_channel])
	{
		g_storeDataRemaindLen[selected_channel]		= U16_S_U16(g_storeDataRemaindLen[selected_channel], g_curStoreDataTagLength[selected_channel]);
		storeLength									= g_curStoreDataTagLength[selected_channel];
		nextState									= dataManager_getStoreDataInitState(p1);
	}
	else if(g_storeDataRemaindLen[selected_channel] == g_curStoreDataTagLength[selected_channel])
	{
		storeLength									= g_curStoreDataTagLength[selected_channel];
		nextState									= STORE_DATA_FINISH;
	}
	else
	{
		g_curStoreDataTagLength[selected_channel]	= U16_S_U16(g_curStoreDataTagLength[selected_channel], g_storeDataRemaindLen[selected_channel]);
		storeLength									= g_storeDataRemaindLen[selected_channel];
		nextState									= NEXT_STORE_DATA;
	}


	memCopyAtomic(MEMtPM8(U32_A_U16(tagArrayContent, g_curStoreDataTagOffset[selected_channel])),
					MEMtPM8(PX8_A_U16(APDUBuffer, g_storeDataOffset[selected_channel])),
					storeLength);

	memRefresh();

	//对于USIM卡v3.0+ (国密卡)产品，SEID文件必须存在，编码格式遵循《中国移动用户卡应用基础能力技术规范》，且SEID文件中的值需与GetData指令获取的SEID的值一致。by xwh 20220811
	IF_UpdateSeidFile(g_curStoreDataTag,MEMtPM8(PX8_A_U16(APDUBuffer, g_storeDataOffset[selected_channel])));

	g_curStoreDataTagOffset[selected_channel]				= U32t16(U16_A_U16(g_curStoreDataTagOffset[selected_channel],storeLength));
	dataManager_decreaseDgiLength(storeLength);

	if(NEXT_STORE_DATA == nextState)
	{
		g_storeDataOffset[selected_channel]				= APDU_DATA_OFFSET;
	}
	else
	{
		g_storeDataOffset[selected_channel]               = U32t8(U8_A_U16(g_storeDataOffset[selected_channel], g_curStoreDataTagLength[selected_channel]));
	}

	return nextState;
}

static u16 dataManager_getDgiOrTagLength(u8 isDgi)
{
	u16		lengthSize;
	u16 	tempLength;
	
	#define		SIZE_82_LENGTH			2
	#define		SIZE_81_LENGTH			1
	
	tempLength = 0;
	lengthSize = 0;
	
	if(true == isDgi)
	{
		if(APDUBuffer[g_storeDataOffset[selected_channel]] == 0xFF) // GP Systems Scripting Language 1.1.0 [FINAL] - DGI format.pdf    B. TLV Length Coding 当Length < 255时用1字节表示，否则用3字节表示(起始字节为ff)
		{
			if(g_storeDataRemaindLen[selected_channel] < SIZE_82_LENGTH)
			{
				dataManager_abortStoreDataThrowError(SW_WRONG_LENGTH);
			}
			U8_INC(g_storeDataOffset[selected_channel]);
			tempLength										= PX_R_U16(&APDUBuffer[g_storeDataOffset[selected_channel]]);
			g_storeDataOffset[selected_channel]				= U16t8(U8_A_U8(g_storeDataOffset[selected_channel], SIZE_82_LENGTH));
			g_storeDataRemaindLen[selected_channel]			= U16_S_U8(g_storeDataRemaindLen[selected_channel], U8_A_U8(SIZE_82_LENGTH , 1));

		}
		else
		{
			tempLength						= PX_R_U8(&APDUBuffer[g_storeDataOffset[selected_channel]]);
			U8_INC(g_storeDataOffset[selected_channel]);
			U16_DEC(g_storeDataRemaindLen[selected_channel]);

		}
		g_curStoreDataDgiLength[selected_channel]		= tempLength;
	}
	else
	{
		if(0x82 == APDUBuffer[g_storeDataOffset[selected_channel]])
		{
			if(g_storeDataRemaindLen[selected_channel] < SIZE_82_LENGTH)
			{
				dataManager_abortStoreDataThrowError(SW_WRONG_LENGTH);
			}
			U8_INC(g_storeDataOffset[selected_channel]);
			tempLength										= PX_R_U16(&APDUBuffer[g_storeDataOffset[selected_channel]]);
			g_storeDataOffset[selected_channel]				= U16t8(U8_A_U8(g_storeDataOffset[selected_channel], SIZE_82_LENGTH));
			g_storeDataRemaindLen[selected_channel]			= U16_S_U8(g_storeDataRemaindLen[selected_channel], U8_A_U8(SIZE_82_LENGTH , 1));
			lengthSize										= U8t16(U8_A_U8(SIZE_82_LENGTH , 1));
		}
		else if(0x81 == APDUBuffer[g_storeDataOffset[selected_channel]])
		{
			if(g_storeDataRemaindLen[selected_channel] < SIZE_81_LENGTH)
			{
				dataManager_abortStoreDataThrowError(SW_WRONG_LENGTH);
			}
			U8_INC(g_storeDataOffset[selected_channel]);
			tempLength										= PX_R_U8(&APDUBuffer[g_storeDataOffset[selected_channel]]);
			g_storeDataOffset[selected_channel]				= U16t8(U8_A_U8(g_storeDataOffset[selected_channel], SIZE_81_LENGTH));
			g_storeDataRemaindLen[selected_channel]			= U16_S_U8(g_storeDataRemaindLen[selected_channel], U8_A_U8(SIZE_81_LENGTH , 1));
			lengthSize										= U8t16(U8_A_U8(SIZE_82_LENGTH , 1));
		}
		else if(APDUBuffer[g_storeDataOffset[selected_channel]] < 0x80)
		{
			tempLength						= PX_R_U8(&APDUBuffer[g_storeDataOffset[selected_channel]]);
			U8_INC(g_storeDataOffset[selected_channel]);
			U16_DEC(g_storeDataRemaindLen[selected_channel]);
			lengthSize						= 1;
		}
		else
		{
			#if UICC__OPERATOR__CHINAMOBILE//GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
			  dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
			#else
				dataManager_abortStoreDataThrowError(SW_WRONG_LENGTH);
			#endif
		}
		
		dataManager_decreaseDgiLength(lengthSize);
	}
	
	//剩余数据长度和标签长度匹配问题
	#if GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
	  if((tempLength > g_storeDataRemaindLen[selected_channel]) && (LAST_BLOCK == (APDUBuffer[APDU_P1_OFFSET] & LAST_BLOCK)))
	  {
         dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
      }
	
	#endif
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	if(g_isLink)
	{
		if((tempLength != g_storeDataRemaindLen[selected_channel] ) && (tempLength < g_storeDataRemaindLen[selected_channel]))
		{
			dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
		}
	}
	else
	{
		if(tempLength != g_storeDataRemaindLen[selected_channel])
		{
			dataManager_abortStoreDataThrowError(SW_WRONG_DATA);	
		}
	}
#endif
	return tempLength;
}
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
u8 dataManager_storeData(PN8 sdDataObj)
{
	u8	isDgi;
	u8			ins;
	u8			p1;
	u8			p2;
	u16			Lc;
	jref 		firstTagNodeRef;
	PN8	tagArrayAfield;
#if UICC__OPERATOR__CHINAMOBILE
	u8          offset,i;
#endif
    // modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11 将此处的宏定义移动到dataManager.h
	isDgi					= false;
	ins						= APDUBuffer[APDU_INS_OFFSET];
	p1						= APDUBuffer[APDU_P1_OFFSET];
	p2						= APDUBuffer[APDU_P2_OFFSET];
	Lc						= U8t16(APDUBuffer[APDU_P3_OFFSET]);
	firstTagNodeRef			= sdData_getFirstTagNode(sdDataObj);
	tagArrayAfield          = 0;
	
	//TODO: 不限制SWP关机刷卡时的电流，临时解决方法，后续需要修正
	if ( (p1 == U16t8(0xEC)) && (p2 == U16t8(0xEC)) )
	{
		return 0;
	}
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	cms2ac_dataManager_checkSequence(p1);
#endif
	dataManager_setBitTag(BIT0_TAG);//将该标记位重新置上
	dataManager_checkSequence(p2);  //级联store data的P2

	g_isLink				= dataManager_isLink(p1); // false indicates last block; true indicates more blocks

	if((0 != bRamBuffer[INSTALL_FOR_PERSONERLIZE]) && ((U16_AND_U16(cc, APPLET_MASK)) != U8t16(bRamBuffer[INSTALL_FOR_PERSONERLIZE])))
	{
		return STORE_DATA_FINISH;
	}
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK || GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
if(ins & JAVA_STORE_DATA)
{
	 g_storeDataState[selected_channel] = STORE_DATA_GET_TAG_ARRAY_AFEILD ;
	 APDUBuffer[APDU_INS_OFFSET] = ins & ~JAVA_STORE_DATA ;
}
else
#endif
{
	if(U8_AND_U8(ins, CUR_STORE_DATA))
	{
		if(0 == g_storeDataRemaindLen[selected_channel])
		{
			g_storeDataState[selected_channel]		= STORE_DATA_FINISH;
		}
		else
		{
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					g_storeDataRemaindLen[selected_channel]			= U16_S_U16(g_storeDataRemaindLen[selected_channel], g_curStoreDataTagLength[selected_channel]);
					dataManager_decreaseDgiLength(g_curStoreDataTagLength[selected_channel]);
#endif
			    g_storeDataOffset[selected_channel]					= U32t8(U8_A_U16(g_storeDataOffset[selected_channel], g_curStoreDataTagLength[selected_channel]));
		}
	}
	else
	{
		g_storeDataRemaindLen[selected_channel]		= Lc;
		g_storeDataOffset[selected_channel]			= APDU_DATA_OFFSET;
	}
	
	if((0 != p2) && ((g_storeDataState[selected_channel] == STORE_DATA_VALUE) || (g_storeDataState[selected_channel] == STORE_JAVA_TAG)))
	{
		
	}
	else
	{
		g_storeDataState[selected_channel] = dataManager_getStoreDataInitState(p1);
	}
}

	while(g_storeDataRemaindLen[selected_channel] > 0)
	{
		switch(g_storeDataState[selected_channel])
		{
			case STORE_DATA_GET_TAG1:
			{
				if (U8_AND_U8(APDUBuffer[APDU_P1_OFFSET] , LAST_BLOCK) == LAST_BLOCK)
				{
					//it must include TL,if it is TLV structure
					if((g_storeDataRemaindLen[selected_channel] < TAG1_LENGTH) || (g_storeDataRemaindLen[selected_channel] == TAG1_LENGTH))
					{
						dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
					}
				}
				if(dataManager_handleMappingDGI()) // 处理DGI格式
				{
					break;
				}
				g_curStoreDataTag					= PX_R_U8(&APDUBuffer[g_storeDataOffset[selected_channel]]);
#if GP__AMENDMENT__B__FOR_MASK
				if((0x85 == g_curStoreDataTag) || (0xA5 == g_curStoreDataTag))
				{
					#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
					if(checkSCP11cSTOREDATATagInBF20(g_curStoreDataTag, isDgi) == false)
					{
						//If the script attempts to perform an operation that is not allowed, the command triggering this operation shall
						//be rejected with a status word of '6985', the SCP11c session shall be terminated, and all objects with a
						//“pending completion” mark shall be deleted

						closeSecuritychannel(SC_TERMINATED);// modify by liweimin
						
						dataManager_abortStoreDataThrowError(SW_CONDITIONS_NOT_SATISFIED);
					}
					#endif
					GPStoreSDAdminSessionPara(sdDataObj, &APDUBuffer[g_storeDataOffset[selected_channel]]);

					return STORE_DATA_FINISH;
				}
#endif

#if GP__AMENDMENT__B__DNS_Resolution
				if((0xF4 == g_curStoreDataTag) || (0xF5 == g_curStoreDataTag))
				{
					GPStoreSDDNSResolutionPara(sdDataObj, &APDUBuffer[g_storeDataOffset[selected_channel]]);

					return STORE_DATA_FINISH;
				}
#endif
				//todo:遗留问题，此处附录a和附录F都支持时，附录a的dgi-00A6和附录F的tag-A6的区分问题
#if	GP__SCP__SCP11__FOR_MASK
				if(0xA6 == g_curStoreDataTag)
				{
					SCP11AnalyzeStoreData(sdDataObj);

					if (g_storeDataState[selected_channel] == NEXT_STORE_DATA)
					{
						break;		//级联，后续还有apdu指令来
					}
					else
					{
						return STORE_DATA_FINISH;	//非级联，保存完成
					}
				}
#endif
				U8_INC(g_storeDataOffset[selected_channel]);
				U16_DEC(g_storeDataRemaindLen[selected_channel]);
				dataManager_decreaseDgiLength(1);
				isDgi								= false;
				if(DOUBLE_TAG == U16_AND_U16(DOUBLE_TAG , g_curStoreDataTag) || (0x2F == g_curStoreDataTag))
				{
					U8_INC(g_storeDataState[selected_channel]);
				}
		/*	#if CMS2AC
				else if(0x00 == g_curStoreDataTag)
				{
					U8_INC(g_storeDataState);
				}
			#endif*/
#if UICC__OPERATOR__CHINAMOBILE
				else if((0x00 == g_curStoreDataTag)&&(0x44 == PX_R_U8(&APDUBuffer[g_storeDataOffset[selected_channel]])))//增加移动的需求storedata支持0044tag，modify by xuxiaojun@20210716
				{
					U8_INC(g_storeDataState[selected_channel]);
//					U8_INC(g_storeDataOffset[selected_channel]);
//					U16_DEC(g_storeDataRemaindLen[selected_channel]);
				}
#endif
				else
				{
					g_storeDataState[selected_channel]				= STORE_DATA_ANALYSE_0070TAG;
				}
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					g_storeDataState[selected_channel]				= STORE_DATA_GET_TAG2;
			#endif
				break;
			}
			case STORE_DATA_GET_TAG2:
			{
				U8_DEC(g_storeDataOffset[selected_channel]);
				if((g_storeDataRemaindLen[selected_channel] < TAG2_LENGTH) || (g_storeDataRemaindLen[selected_channel] == TAG2_LENGTH))
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
				}
				if(dataManager_handleMappingDGI())     //处理MAPPING配置中的DGI
				{
					break;
				}
				U8_INC(g_storeDataOffset[selected_channel]);
				g_curStoreDataTag					= U16_SHL_U8(g_curStoreDataTag , 8);
				g_curStoreDataTag					= U16_OR_U16(g_curStoreDataTag , PX_R_U8(&APDUBuffer[g_storeDataOffset[selected_channel]]));
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				if(CARD_ISSUER_DAT == g_curStoreDataTag)//NFCSim1.0的0044和0045，共用同一个tag 0044
					U16_DEC(g_curStoreDataTag);
			#endif
				U8_INC(g_storeDataOffset[selected_channel]);
				U16_DEC(g_storeDataRemaindLen[selected_channel]);
				dataManager_decreaseDgiLength(1);
				g_storeDataState[selected_channel]					= STORE_DATA_ANALYSE_0070TAG;
				break;
			}
			case STORE_DATA_GET_DGI:
			{
				if((g_storeDataRemaindLen[selected_channel] < DGI_LENGTH) || (g_storeDataRemaindLen[selected_channel] == DGI_LENGTH))
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
				}
				g_curStoreDataTag									= PX_R_U16(&APDUBuffer[g_storeDataOffset[selected_channel]]);
				g_storeDataOffset[selected_channel]					= U16t8(U8_A_U8(g_storeDataOffset[selected_channel], DGI_LENGTH));
				g_storeDataRemaindLen[selected_channel]				= U16_S_U8(g_storeDataRemaindLen[selected_channel], DGI_LENGTH);
				isDgi												= true;
				U8_INC(g_storeDataState[selected_channel]);
				break;
			}
			case STORE_DATA_ANALYSE_0070TAG:
			{
#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
				if(checkSCP11cSTOREDATATagInBF20(g_curStoreDataTag, isDgi) == false)
				{
					//If the script attempts to perform an operation that is not allowed, the command triggering this operation shall
					//be rejected with a status word of '6985', the SCP11c session shall be terminated, and all objects with a
					//“pending completion” mark shall be deleted

					closeSecuritychannel(SC_TERMINATED);// modify by liweimin

					dataManager_abortStoreDataThrowError(SW_CONDITIONS_NOT_SATISFIED);
				}
#endif
				g_storeDataState[selected_channel]					= dataManager_storeData0070TagAnalyse();
				break;
			}
			case STORE_DATA_SELF_HANDLE_TAG:
			{
				g_storeDataState[selected_channel]					= dataManager_selfHandleTag(sdDataObj);
				break;
			}
			case STORE_DATA_GET_LENGTH:
			{
				g_curStoreDataTagLength[selected_channel]			= dataManager_getDgiOrTagLength(isDgi);
				g_curStoreDataTagOffset[selected_channel]			= 0; //这里为什么清0?
				U8_INC(g_storeDataState[selected_channel]);

				if((U8_AND_U8(APDUBuffer[APDU_P1_OFFSET] , LAST_BLOCK) == LAST_BLOCK)//子tag长度不为0  总长度为0
					&&(g_curStoreDataTagLength[selected_channel] > g_storeDataRemaindLen[selected_channel]))
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
				}
#if M2M_EUICC
				if((getCurrentSDId() == ISDP_APPID)&&(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX)&&(g_curStoreDataTagLength[selected_channel] < 1))
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_DATA);//对于ISDP ，在masterDelete时 需要带42 45 5F20（强制tag且长度为1~n），对于其他SD 有测到storedata 4200 4500等情况 期望成功 所以仅针对ISDP做限制
				}
#endif
				if((g_curStoreDataTagLength[selected_channel] == 0x00)&&(g_storeDataRemaindLen[selected_channel] == 0x00))//modify by xuxiaojun@20220621: TLV的L不允许为0，否则抛异常6A80/6700  gpBFCCompliance208: ds_APDU_storeData_OPR_11_82
				{
					dataManager_checkSingleCommandTag(0x00);
					dataManager_checkTagLength(0x00);
					dataManager_storeDataCheckIllegalTag();
				}
				break;
			}
			case STORE_DATA_ANALYSE_TAG:
			{
				g_storeDataState[selected_channel]					= dataManager_storeDataDGIandTagAnalyse(U16t8(g_storeDataRemaindLen[selected_channel]));
				break;
			}
			case STORE_DATA_GET_TAG_ARRAY_AFEILD:
			{
#if UICC__OPERATOR__CHINAMOBILE   //中国移动用户卡应用基础能力技术规范v4.0.1(报批稿).pdf  标签2F12用来获取用户卡的版本信息, 数据格式见5-2。
				if(g_curStoreDataTag == 0x2F12)
				{
					offset = FindTag(&APDUBuffer[g_storeDataOffset[selected_channel]], 0xA1, g_curStoreDataTagLength[selected_channel]); //用户卡类型标识标签A1, 必须存在，且长度为2

					if((offset == FALSE) || (APDUBuffer[g_storeDataOffset[selected_channel]+offset] != 0x02))
					{
						dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
					}

					for(i =2 ; i<= 7; i++)
					{
						offset = FindTag(&APDUBuffer[g_storeDataOffset[selected_channel]], 0xA0+i, g_curStoreDataTagLength[selected_channel]);//A2:COS商代码标签,A3:产品版本标签 A4://gp版本标签 A5:COS版本标签 A6:用户卡芯片商标签 A7:用户卡芯片型号标签

						if(offset == FALSE)
						{
							dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
						}
					}
				}
#endif
				if (!isInProgress())
				{
					begin_transaction(MULTIL_APDU_PROCESS);
				}
				tagArrayAfield						= MEMtPN8(dataManager_getTagArrayAfield(firstTagNodeRef, g_curStoreDataTag, g_curStoreDataTagLength[selected_channel], g_handle0070, true));
				U8_INC(g_storeDataState[selected_channel]);
				break;
			}
			case STORE_DATA_INIT_STORE_VALUE:
			{
				g_tagArrayContent[selected_channel]	= PXtMEM(tagNode_getTagArrayContent(tagArrayAfield));				
				tagNode_setTagArrayLength(tagArrayAfield, g_curStoreDataTagLength[selected_channel]);
				U8_INC(g_storeDataState[selected_channel]);
				break;
			}
			case STORE_DATA_VALUE:
			{
				g_storeDataState[selected_channel]					= dataManager_storeValue(g_tagArrayContent[selected_channel], p1);
				break;
			}
			case STORE_JAVA_TAG:
			{
				if ( (g_curStoreDataTag == 0x0085) || (g_curStoreDataTag == 0x00F5) )
				{
					g_curStoreDataTagLength[selected_channel] = 0;
				}

				if(g_storeDataRemaindLen[selected_channel] < g_curStoreDataTagLength[selected_channel])
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_LENGTH);
				}
#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK || !GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK
				else
				{
					g_storeDataRemaindLen[selected_channel]			= U16_S_U16(g_storeDataRemaindLen[selected_channel], g_curStoreDataTagLength[selected_channel]);
					dataManager_decreaseDgiLength(g_curStoreDataTagLength[selected_channel]);
				}
#endif
				APDUBuffer[APDU_INS_OFFSET]			= U8_OR_U8(APDUBuffer[APDU_INS_OFFSET] , CUR_STORE_DATA);
				return STORE_JAVA_TAG;
			}
			case NEXT_STORE_DATA:
			{
				if(false == g_isLink)
				{
					dataManager_abortStoreDataThrowError(SW_WRONG_DATA);
				}
				else
				{
					g_storeDataState[selected_channel]			= STORE_DATA_VALUE;
				}
				return NEXT_STORE_DATA;
			}
			case STORE_DATA_FINISH:
			{
				commit_transaction();
				if(false == g_isLink)
				{
					dataManager_reset();
				}
				return STORE_DATA_FINISH;
			}
		}
	}

	return g_storeDataState[selected_channel];
}
/*===================================================================================================================================*/
/*===================================================================================================================================*/

/*=====================================================handle get data===============================================================*/
/*===================================================================================================================================*/
/**
 * modify by xuxiaojun@20220427 根据分层代码 第一次代码评审(20220415).xlsx 将GetFileSystemTotalSize()宏定义为0
 */
u8 getCardResoure(u16 curGetDataTag, PX8 tempBuff, u8 appNum)
{
	u8  off;
	u32 iFreeSize_;

	off = 0;

	tempBuff[U8_INC(off)] = 0x81;  //Number of installed application tag
	tempBuff[U8_INC(off)] = 0x01;

	tempBuff[U8_INC(off)] = appNum;  //install application number

	tempBuff[U8_INC(off)] = 0x82;   //Free Non Volatile Memeory tag
	tempBuff[U8_INC(off)] = 0x04;

#if GP__CGM__FOR_MASK
	if ((curGetDataTag == CARD_REAL_RESOURCE)
		||(g_bCheckFlag == CHECK_CLOSE))
	{
		iFreeSize_ = NvmManager_GetAvailMemory();
	}
	else
	{
		iFreeSize_ = CgmField_getSpaceInfo(APP_FIRST, SD_CGM_REMAIN_NVM_SIZE);

		iFreeSize_ = U32_S_U32(iFreeSize_,GetFileSystemTotalSize());
	}
#else
	iFreeSize_ = NvmManager_GetAvailMemory();
#endif

	tempBuff[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 24));
	tempBuff[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 16));
	tempBuff[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 8));
	tempBuff[U8_INC(off)] = U32t8(iFreeSize_);

	tempBuff[U8_INC(off)] = 0x83;   //Free Volatile Memeory tag
	tempBuff[U8_INC(off)] = 0x02;   //Free Volatile Memeory tag

#if GP__CGM__FOR_MASK
	if ((curGetDataTag == CARD_REAL_RESOURCE)
		||(g_bCheckFlag == CHECK_CLOSE))
	{
		iFreeSize_ = tranAllocate_rtrAvailable();
	}
	else
	{
		iFreeSize_ = CgmField_getSpaceInfo(0, SD_CGM_REMAIN_RAM_SIZE) - RamManager_getUsedDtrSize();
	}
#else
	iFreeSize_ = tranAllocate_rtrAvailable();
#endif
	tempBuff[U8_INC(off)] = U32t8(U32_SHR_U8(iFreeSize_, 8));
	tempBuff[U8_INC(off)] = U32t8(U32_AND_U32(iFreeSize_, 0xFF));

	return off;
}

/*
	modify by xieyong@20211210  电信5G GP平台测试 接触获取SEID测试添加
    modify by xwh 20220111 	当为实卡时，SEID取值与ICCID的值相同。当为空卡时，SEID取值为空卡序列号的前9个字节+第11个字节 <中国电信TSM业务技术方案-密钥分册2021v1.doc>5.6节
	modify by qp 20230208 增加中国移动usim卡获取seid
*/
#if JAVA_CARD_UICC
u16 getSeId(PX8 tempBuff)
{
	memref fileAddr;
	u32    seIdSize;
	u8     result;
	
#if UICC__OPERATOR__CHINAMOBILE

	fileAddr = SelectTFFile(DF_MF, 0x2FF0);
	
	if (fileAddr == FALSE)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
	
	seIdSize =  GetTFSize(fileAddr);
	
	result = ReadTFBuf(fileAddr,0,tempBuff,seIdSize);
	
	if(result == FALSE)
	{
		return 0;
	}
	
	
#else
	
	fileAddr = SelectTFFile(DF_MF, EF_ICCID);
	
	if (fileAddr == FALSE)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}
	
	seIdSize =  GetTFSize(fileAddr);
	
	result = ReadTFBuf(fileAddr,0,tempBuff,seIdSize);
	
	if(result == FALSE)
	{
		return 0;
	}
	
	if((IsBufAllZeroOrFF(tempBuff,0x00,0x0A))||(IsBufAllZeroOrFF(tempBuff,0xFF,0x0A)))
	{
		//当为空卡时，SEID取值为空卡序列号的前9个字节+第11个字节
		fileAddr = SelectTFFile(DF_MF, 0x2F02);
		
		if (fileAddr == FALSE)
		{
			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
		}
		
		ReadTFBuf(fileAddr,0,tempBuff,0x09);
		ReadTFBuf(fileAddr,0x0A,tempBuff+9,0x01);
	}
	
#endif
	return seIdSize;	
	
}
#endif
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
static void dataManager_getDataCheckIllegalTag(u16 curGetDataTag, u8 cla) {

	switch(curGetDataTag)
	{
		case 0x00E2:
		case 0x2F12:
			if(g_bSDId[selected_channel] != 0)
			{
				throw_error(SW_INCORRECT_P1P2);
			}
			break;
	}

	switch(curGetDataTag)
	{
		case 0x00C0:
		case 0x00C2: // Confirmation Counter
		case 0x2F10:
		case 0x2F12:
		case 0x2F14:
			if((0x00 == U8_AND_U8(APDUBuffer[APDU_CLA_OFFSET] , 0x80)) || (SCF_NO_AUTHENTICATED == channelInfo.security_level[selected_channel]))
			{
				throw_error(SW_SECURITY_STATUS_NOT_SATISFIED);
			}
			break;
	}
#if GP__SCP__SCP11__FOR_MASK
	switch(curGetDataTag)
	{
		case 0xBF21: // GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf   SCP11 certificate store
		case 0x0083: // GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf
			break;
	}
#endif
	//the tags is not support
	switch (curGetDataTag)
	{
		case 0x004f:		   // tag:0x4f card manager AID  mapping 6.3.2
		case 0x9F66:// Note that for the Issuer Security Domain,parameters '9F66', '9F67', '00CD' and '004F' shall cause a response of '6A88' to be returned
		case 0x00CD:
		case 0x9F67:
			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
			break;
	}

	/*
	if(CONFIG_VGP)
	{

	}
	else
	{
		switch (curGetDataTag)
		{
			case 0x00C1:
				throw_error(SW_REFERENCED_DATA_NOT_FOUND);
				break;
		}
	}
	*/
}
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
static u8 dataManager_getDataTagAnalyse(u16 curGetDataTag, u8 cla)
{

	dataManager_getDataCheckIllegalTag(curGetDataTag, cla);

	switch (curGetDataTag)
	{
		case 0x00A0:
		case 0x7F21:
		case 0x2F16:
		case 0xBF30:
		case 0xFF1F:
			return GET_DATA_THE_JAVA_TAG;
		default:
			break;
	}

	return GET_DATA_THE_C_TAG;
}

u32 dataManager_getDataValue(PX8 buf, jref firstTagNodeRef, u16 tag, u16 curContentOffset, u16 maxLen, u8 is0070group,u8 isSend)
{
	u16 				copyLength;
	u16 				tagLength;
	u16					tagAttri;
	jref				tagArrayRef;
	PN8					tagArrayAField;

	tagAttri			= dataManager_calcTagAttri(is0070group);
	tagArrayRef			= tagNode_getTagArray(firstTagNodeRef, tag, 0, tagAttri, MEM_NULL, false);
	if(REF_NULL == tagArrayRef)
	{
#if UICC__OPERATOR__CHINAMOBILE
		if((tag == CARD_ISSUER_NUM) || (tag == CARD_ISSUER_DAT) || (tag == CARD_ID) || (tag == CARD_DATA))
		{
			return 0x00;
		}
		else
#endif
		{
			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
		}
	}
	tagArrayAField		= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(tagArrayRef))));
	tagLength			= tagNode_getTagArrayLength(tagArrayAField);

	copyLength			= U16_S_U16(tagLength, curContentOffset);
	
	if(true == isSend)
	{
		if(copyLength > maxLen)
		{
			copyLength		= maxLen;
		}
		ecpMem_Copy(buf, PN8_A_U16(tagNode_getTagArrayContent(tagArrayAField), curContentOffset), copyLength);
		memRefresh();
	}
	else
	{
		if(copyLength > maxLen)
		{
			throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
		}
		memCopyAtomic(buf, tagNode_getTagArrayContent(tagArrayAField), copyLength);
	}

	return (U32_OR_U32(U32_SHL_U8(copyLength, GET_SEND_LENGTH_SHIFT), U16t32(tagLength)));
}

static void dataManager_sendOutData(u16 curGetDataTag, u16 curGetDataLength, u16 sendLength, u8 isReturnTag, u8 isReturnLen, u8 isSendFirstData)
{
	u8					sendStartOffset;
	u8					tagLengthSize;
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	boolean				dooubleTag_handle;
#endif
	#define				FIRST_TAG_MASK				0xFF00
	#define				FIRST_TAG_ZERO				0x0000
	#define				FIRST_TAG_2F				0x2F00
	#define				IS_DOUBLE_TAG				0x1F00
	#define				DOUBLE_TAG_SIZE				2
	#define				SINGLE_TAG_SIZE				1
	
	#define				LENGTH_82_OFFSET			2
	#define				LENGTH_82_VALUE_OFFSET		3
	#define				LENGTH_82_SIZE				3

	#define				LENGTH_81_OFFSET			3
	#define				LENGTH_81_VALUE_OFFSET		4
	#define				LENGTH_81_SIZE				2

	#define				LENGTH_7F_VALUE_OFFSET		4
	#define				LENGTH_7F_SIZE				1
	
	#define				LENGTH_NONE_OFFSET			5
	#define				LENGTH_NONE_SIZE			0
	#define				BIT1_TAG					0x02		
	#define				BIT1_TAG3					0x04
	
	if((true == isSendFirstData) && (false == dataManager_getBitTag(BIT1_TAG)))//级联的时候，后续数据不允许出现TL
	{
		if(true == isReturnLen)
		{
			if(curGetDataLength > 0xFF)
			{
				APDUBuffer[LENGTH_82_OFFSET]		= 0x82;
				PX_W_U16(&APDUBuffer[LENGTH_82_VALUE_OFFSET], curGetDataLength);
				sendStartOffset						= LENGTH_82_OFFSET;
				tagLengthSize						= LENGTH_82_SIZE;
			}
			else if(curGetDataLength > 0x7F)
			{
				APDUBuffer[LENGTH_81_OFFSET]		= 0x81;
				APDUBuffer[LENGTH_81_VALUE_OFFSET]	= U16t8(curGetDataLength);
				sendStartOffset						= LENGTH_81_OFFSET;
				tagLengthSize						= LENGTH_81_SIZE;
			}
			else
			{
				APDUBuffer[LENGTH_7F_VALUE_OFFSET]	= U16t8(curGetDataLength);
				sendStartOffset						= LENGTH_7F_VALUE_OFFSET;
				tagLengthSize						= LENGTH_7F_SIZE;
			}
		}
		else
		{
			sendStartOffset							= LENGTH_NONE_OFFSET;
			tagLengthSize							= LENGTH_NONE_SIZE;
		}
		
		if(true == isReturnTag)
		{
			if((IS_DOUBLE_TAG == U16_AND_U16(IS_DOUBLE_TAG, curGetDataTag)) || (CARD_ID == curGetDataTag) || (FIRST_TAG_2F == U16_AND_U16(curGetDataTag, FIRST_TAG_MASK)))
			{
				sendStartOffset						= U8_S_U8(sendStartOffset, DOUBLE_TAG_SIZE);
				tagLengthSize						= U16t8(U8_A_U8(tagLengthSize, DOUBLE_TAG_SIZE));
				PX_W_U16(&APDUBuffer[sendStartOffset], curGetDataTag);
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				dooubleTag_handle					= true;
#endif
			}
			else
			{									
				sendStartOffset					= U8_S_U8(sendStartOffset, SINGLE_TAG_SIZE);
				tagLengthSize					= U16t8(U8_A_U8(tagLengthSize, SINGLE_TAG_SIZE));
				APDUBuffer[sendStartOffset]		= U16t8( curGetDataTag );
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				dooubleTag_handle 			= false;
#endif
			}
			
#if	GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			if(dooubleTag_handle == false)
			{
				sendStartOffset						= U8_S_U8(sendStartOffset, SINGLE_TAG_SIZE);
				tagLengthSize						= U8_A_U8(tagLengthSize, SINGLE_TAG_SIZE);
				PX_W_U16(&APDUBuffer[sendStartOffset], curGetDataTag);
			}
			else
			{
				dooubleTag_handle	= false;
			}
				
#endif
		}
		
		sendLength									= U32t16(U16_A_U8(sendLength, tagLengthSize));
		/*if(bRamBuffer[PROCESS_LEVEL] != ROCESS_LEVEL_OTA)
		{	
			apdu_SetOutgoing();
			apdu_SetOutgoingLength(sendLength);
		}*/
	}
	else
	{
		sendStartOffset		= APDU_DATA_OFFSET;
	}
	//检测得到数据长度合法性
// #if !CMS2AC
// 	if(0x00 == sendLength)
// 	{
// 		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
// 	}
// #endif
		

	//apdu_SetOutgoing();
	//apdu_SetOutgoingLength(sendLength);
#if  APP_BYD_CONFIG
	if(curGetDataTag == SE_ID_BYD)//modify by xuxiaojun@20220701 比亚迪的SEID需求中，获取时不期望带有TL，只期望返回24字节的value
	{
		sendStartOffset = U8_A_U8(sendStartOffset, 0x02);
		sendLength = U8_S_U8(sendLength, 0x02);
	}
#endif

	apdu_ApduSetOutgoingAndSend(U8t16(sendStartOffset), sendLength);
	
}
//extern u8 aidToAppId(u8 theAid[]);
/**
*modify by xuxiaojun@20220309   merged from EPX产业化\STM_ST33G1M2\Branch\RSP\code, 支持DNS功能
*/
u8 dataManager_getData(PN8 sdDataObj)
{
	u8 		isReturnLen;
	u8		isReturnTag;
	u8		isGetDataFinish;
	u8		isSendFirstData;
	u8		cla;
	u16		sendLength;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u16		Is_GetDataLinkTag;
	u16		backcurGetDataTag;
#endif
	u16 	curGetDataTag;

	u16 	curGetDataLength;
	u16 	curContentOffset;
	u16		sendMaxLengthtemp;
	jref 	firstTagNodeRef;
	u32		tempLength;
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	u32		tempSendLength;
	u8		maxKeyVersion;
#endif
	PX8		tempBuff;


//*************************************************************used by get key data********************************************************	
	u16					curKeyVersion;
//*****************************************************************************************************************************************
	
#if  VM_WITH_DCEP_ZZH
	u8 buf[17];

	u8 walletAidBuf[0x0A] = {0x09, 0xA0,0x00,0x00,0x08,0x72,0x53,0x00,0x00,0x00};
#endif

	#define				SEND_FIRST_DATA				0
	#define				SEND_SUBSEQUENCE_DATA		1
	#define				SEND_MAX_LEN_ONE_DATA		U8_S_U8(255, GET_DATA_START_OFFSET)
	#define				SEND_MAX_LEN_ONE_DATA_RMAC	0xF2
	#define				ISGETDATALINKFLAG			0x8000
	#define				GET_DATA_START_OFFSET		5

	#define				BIT1_TAG					0x02
	#define				BIT1_TAG2					0xFD
	#define				BIT1_TAG3					0x04
	#define				BIT1_TAG4					0xFB
//*************************************************************used by get key data********************************************************	
	curKeyVersion		= 0xFFFF;
	//curKeyVersion		= keySet_getDefaultKeyVersion(sdDataObj);
//*****************************************************************************************************************************************

	isReturnLen			= true;
	isReturnTag			= true;
	isGetDataFinish		= false;
	isSendFirstData		= true;
	cla					= APDUBuffer[APDU_CLA_OFFSET];
	curGetDataTag		= U16_OR_U16(U16_SHL_U8(U8t16(APDUBuffer[APDU_P1_OFFSET]), 8), APDUBuffer[APDU_P2_OFFSET]);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	backcurGetDataTag 	= curGetDataTag;
#endif

#if  VM_WITH_DCEP_ZZH //  mergerd from FINANCE_TMC_THD89:支持硬件钱包的两个TAG
	if(curGetDataTag == 0xDF30)
	{
		buf[0] = GetAIDDataByAppID(getCurrentSDId(),&buf[1]);

		if((buf[0] == walletAidBuf[0]) && (memCompare(MEMtPM8(buf),MEMtPM8(walletAidBuf),U8_A_U8(walletAidBuf[0], 0x01)) == ECP_CMP_RESULT_EQUAL))
		{

		}
		else
		{
			throw_error(SW_REFERENCED_DATA_NOT_FOUND);
		}

	}
	if((curGetDataTag == 0xD1) || (curGetDataTag == 0xDF30))
	{
		sdDataObj = MEMtPN8(sdData_getsdDataObjByAppId(ISD_APPID));  //mergerd from FINANCE_TMC_THD89: modify by ydd@20220610
	}
#endif

	firstTagNodeRef		= sdData_getFirstTagNode(sdDataObj);
	curGetDataLength	= 0;
	curContentOffset    = 0;

	if (GET_DATA_THE_JAVA_TAG == dataManager_getDataTagAnalyse(curGetDataTag, cla))
	{
		return GET_DATA_THE_JAVA_TAG;
	}

	while(true != isGetDataFinish)
	{
		if(true == isSendFirstData)
		{
			curContentOffset	= 0;
			tempBuff				= &APDUBuffer[GET_DATA_START_OFFSET];

		}
		else
		{
			tempBuff				= &APDUBuffer[APDU_CLA_OFFSET];
		}

		switch(curGetDataTag)
		{
			case 0x9F7F: // added by xuxiaojun@20220621  Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf: Table 7 - Card Production Life Cycle Data
			{
				memCopyAtomic(tempBuff, MEMtPN8(JcvmConfig_GetCplcDataConfBase()), PRODUCTION_LIFECYCLE_LENGTH);
				sendLength = PRODUCTION_LIFECYCLE_LENGTH;
				curGetDataLength = sendLength;
				break;
			}
#if  APP_BYD_CONFIG			
			case SE_ID_BYD: // 比亚迪的SE ID需求：SEID长度为24字节，前16字节为分散因子信息(SEID_BYD)，后8字节为随机数信息(随机生成)；
			{
				memCopyAtomic(tempBuff, MEMtPN8(JcvmConfig_GetSEIDBydBase()), 0x10);
				Random_number_generator(MEMtPX8(&tempBuff[0x10]), CARD_CHALLENGE_LENGTH, ALG_SECURE_RANDOM);
				sendLength = 0x18;
				curGetDataLength = sendLength;
				break;
			}
#endif
//			case MENU_PARAMETER:
//			{
//				u8 app_id = aidToAppId(&APDUBuffer[6]);
//
//				if(app_id == 0xFF)
//					throw_error(SW_REFERENCED_DATA_NOT_FOUND);
//
//				sendLength = get_menuInfo((u8 *)tempBuff, 0, false, app_id);
//
//				curGetDataLength = sendLength;
//
//				if(sendLength == 0)
//					throw_error(SW_REFERENCED_DATA_NOT_FOUND);
//
//				break;
//			}
#if GP__AMENDMENT__B__FOR_MASK
		case 0x85:
		case 0xA5:
		{
			sendLength = GPGetSDAdminSessionPara(sdDataObj, curGetDataTag, tempBuff);
			curGetDataLength = sendLength;
			break;
		}
#endif
#if GP__AMENDMENT__B__DNS_Resolution
		case 0xF4:
		case 0xF5:
		{
			sendLength = GPGetSDDNSResolutionPara(sdDataObj, curGetDataTag, tempBuf);
			curGetDataLength = sendLength;
			break;
		}
#endif
#if GP__SCP__SCP11__FOR_MASK //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
			case 0xBF21: // GET DATA (ECKA Certificate) Command
			{
				if(true == isSendFirstData)
				{
					firstTagNodeRef = NULL_REF;
				}
				else if (firstTagNodeRef == NULL_REF)	//读数据的索引已清空，表示已读取完
				{
					break;

				}

				sendLength = SCP11GetECKACert(sdDataObj, tempBuff, SEND_MAX_LEN_ONE_DATA, &firstTagNodeRef, curContentOffset);

				curGetDataLength = sendLength;

				isReturnTag				= false;
				isReturnLen				= false;
				break;
			}
			case 0x0083: // GET DATA (CA-KLOC KID-KVN) Command
			{
				sendLength = SCP11GetKIDandKVNByCAID(sdDataObj, tempBuff, SEND_MAX_LEN_ONE_DATA);
				curGetDataLength = sendLength;
				break;
			}
#endif
			case CARD_E2E_SPACE://FF20
		#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			{
				u32 freesize = NvmManager_GetAvailMemory();

			#if GP__VERSION__FOR_MINI_MASK
				freesize = (freesize > U16t32(0xFFFF)) ? U16t32(0xFFFF) : freesize;
			#else
				freesize = (freesize > 0x7FFF) ? U16t32(0x7FFF) : freesize;
			#endif
				tempBuff[0]   = U32t8(U32_SHR_U8(freesize, 8));
				tempBuff[1] 	 = U32t8(freesize);
				tempBuff[2] 	 = getAppletNum();
				sendLength   = 3;
				curGetDataLength = 3;
				break;
			}
		#endif
			case CARD_REAL_RESOURCE: //EF20
			case CARD_RESOURCE_DATA: //FF21
				sendLength				= U8t16(getCardResoure(curGetDataTag, tempBuff, getAppletNum()));
				curGetDataLength  		= sendLength;
				break;
			case KEY_INFO: //00E0
				sendLength				= sdData_getKeyInfo(tempBuff, SEND_MAX_LEN_ONE_DATA, sdDataObj, &curKeyVersion);
				curGetDataLength  		= sendLength;
				break;
			case CARD_KEYVERSION: //00E2
				PX_W_U8(tempBuff, 1);
				sendLength				= 1;
				curGetDataLength		= 1;
				break;
			case CARD_APP_INFO: //2F00
			case CAED_APP_INFO1://2F01
				if(apdu_IsSetIncoming() == 0) //if the cla of get data command is 0x84，the data had been received in checkIsAuthenticated
				{                             //test case: eUICC_SE_Test/SE_Test/SE和Profile独立性测试/SE_Profile_IndependenceTest_11
					apdu_ApduSetIncomingAndReceive();
				}
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK  //2F00级联的话，后面只能跟2F01不能插入其他指令		
				if(((0x80 == U8_AND_U8(handle_2F00Tag[selected_channel] , 0x80)) && (CAED_APP_INFO1 == curGetDataTag) && (handle_2F00TagLink[selected_channel] == U8_AND_U8(handle_2F00Tag[selected_channel] , 0x0F)))
						|| ((0x00 == U8_AND_U8(handle_2F00Tag[selected_channel] , 0x80)) && (CARD_APP_INFO == curGetDataTag)))
				{
					tempSendLength          = getAppInfoCMS2AC(g_bSDId[selected_channel], tempBuff, &curGetDataLength);
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					Is_GetDataLinkTag		= U16_AND_U16(tempSendLength , ISGETDATALINKFLAG);
#endif
					sendLength		      	= U16_AND_U16(tempSendLength , 0x7FFF);
					isReturnTag				= true;
					isReturnLen				= true;
				}
				else
				{
					throw_error(SW_INCORRECT_P1P2);
				}
				
			#else
				sendLength				= getAppInfo(g_bSDId[selected_channel], tempBuff, &curGetDataLength);
				isReturnTag				= false;
				isReturnLen				= false;
			#endif
				break;
			case KEY_SEQUENCE: //00C0

			#if GP__KEY_MANAGER
				sendLength 				= U8t16(getDefaultKeyCounter(sdDataObj, tempBuff));
//			GetCounterKey(0,0,0);
//			getCounter(PX8_A_U16(tempBuff, sendLength));
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				maxKeyVersion = keySet_getMaxKeyVersion(sdDataObj, 0x80);
				if(maxKeyVersion!=0xFF)
				{
					getSCP80KeySEQ(g_bSDId[selected_channel], maxKeyVersion, PX8_A_U16(tempBuff, 2));
					sendLength 				= U16_A_U8(sendLength, 5);
				}
			#endif
				curGetDataLength		= sendLength;
			#endif
				break;
			case VGP_KEY_SEQUENCE: //00C1
			#if GP__KEY_MANAGER
				sendLength				= U8t16(getDefaultKeyCounter(sdDataObj, tempBuff));
				isReturnTag				= false;
				isReturnLen				= false;
			#endif
				break;
			case SE_CSL: //00D3
				PX_W_U8(tempBuff, channelInfo.security_level[selected_channel]);
				sendLength				= 1;
				curGetDataLength		= 1;
				break;
#if JAVA_CARD_UICC
			case CARD_SEID:
				sendLength              = getSeId(tempBuff);
				curGetDataLength		= sendLength;
				break;
#endif
			default:
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				if(CARD_ISSUER_DAT == curGetDataTag)//NFCSim1.0的0044和0045，共用同一个tag 0044
					curGetDataTag--;
			#endif
				if(U8_AND_U8(channelInfo.security_level[selected_channel] , 0x10))
				{	
					sendMaxLengthtemp			= 	SEND_MAX_LEN_ONE_DATA_RMAC;	
				}
				else
				{
					sendMaxLengthtemp			= 	U8t16(SEND_MAX_LEN_ONE_DATA);
				}
				
				if(BIT1_TAG3 == dataManager_getBitTag(BIT1_TAG3))
				{
					U16_DEC(curGetDataTag);
					curContentOffset = sendMaxLengthtemp;
				}
				tempLength				= dataManager_getDataValue(tempBuff, firstTagNodeRef, curGetDataTag, curContentOffset, sendMaxLengthtemp, g_handle0070, true);
				sendLength				= U32t16(U32_SHR_U8(tempLength, GET_SEND_LENGTH_SHIFT));
				curGetDataLength		= U16_AND_U16(U32t16(tempLength), GET_TAG_DATA_MASK);
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				curGetDataTag = backcurGetDataTag; 
			#endif
				break;
		}

		if(curGetDataTag == CARD_WALLETAPPSEID)
		{
			IF_GetDCEPID(tempBuff); // 根据移动需求，替换掉getData时ICCID对应位置的数据
		}

		curContentOffset	= U32t16(U16_A_U16(curContentOffset, sendLength));
		
		if(0x00 == cla) //7816
		{
			isReturnTag		= false;
			isReturnLen		= false;
		}
		
		if(curGetDataLength == 0x00)
		{
			if((curGetDataTag == 0x42)||(curGetDataTag == 0x45)||(curGetDataTag == 0x66)||(curGetDataTag == 0x44))
			{
				if(CONFIG_GP_FOR_BFC || CONFIG_GP_FOR_MAPPING_TEST)// merged from EPX\v2.1\BRANCH_MODULE\FINANCE_TMC_THD89
				{
					throw_error(SW_REFERENCED_DATA_NOT_FOUND);//modify by xuxiaojun@20220426 gpMGCompliance103  ds_APDU_getData_12_a9_f1
				}
			}
		}
		
		dataManager_sendOutData(curGetDataTag, curGetDataLength, sendLength, isReturnTag, isReturnLen, isSendFirstData);

		#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		if((sendLength < sendMaxLengthtemp ) && (Is_GetDataLinkTag != ISGETDATALINKFLAG))
		{
			isGetDataFinish 											= true;
			handle_2F00Tag[selected_channel]		= U8_AND_U8(handle_2F00Tag[selected_channel], 0x0F);
			dataManager_getBitTag(BIT1_TAG2);
			dataManager_getBitTag(BIT1_TAG4);
		}
		else
		{
			isGetDataFinish 						= false;
			Is_GetDataLinkTag						= false;
			handle_2F00Tag[selected_channel]		= U8_OR_U8(handle_2F00Tag[selected_channel], 0x80);
			U8_INC(handle_2F00Tag[selected_channel]);
			dataManager_setBitTag(BIT1_TAG);
			dataManager_setBitTag(BIT1_TAG3);
		}
		isSendFirstData	= false;

		
		if(isGetDataFinish == false)
		{
			throw_error(SW_MORE_GETDATA);
		}
		#endif


#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		if(sendLength < U8t16(SEND_MAX_LEN_ONE_DATA))
		{
			isGetDataFinish = true;
		}
		isSendFirstData		= false;
#endif
	}

	return GET_DATA_FINISH;
}
/*===================================================================================================================================*/
/*===================================================================================================================================*/

/*=====================================================used by java layer============================================================*/
/*===================================================================================================================================*/
static u16 dataManager_getTagContent(jref firstTagNodeRef, jref desRef, u16 offset, u16 tag, u16 is0070GroupTag)
{
	u16			maxLength;
	PN8	desObj;
	PX8	desAfield;
	
	desObj			= MEMtPN8(locateObjectInMemory(desRef));
	maxLength		= get_array_length(PXtMEM(desObj));
	maxLength		= U16_S_U16(maxLength, offset);
	desAfield		= MEMtPN8(get_array_data(desObj));
	
	maxLength		= U32t16(dataManager_getDataValue(PX8_A_U16(desAfield, offset), firstTagNodeRef, tag, 0, maxLength, is0070GroupTag, false));
	
	return maxLength;
}

static void dataManager_setTagContent(jref firstTagNodeRef, jref desRef, u16 offset, u16 tag, u16 tagContentLength, u16 is0070GroupTag)
{
	u8	transactionStarted;
	u16			maxLength;
	PN8	desObj;
	PX8	desAfield;
	PN8	tagArrayAfield;
	PN8	tagArrayContent;
	
	transactionStarted	= false;
	desObj				= MEMtPN8(locateObjectInMemory(desRef));	// pop arrayref
	maxLength			= get_array_length(PXtMEM(desObj));
	desAfield			= MEMtPN8(get_array_data(desObj));
	
	tagArrayAfield		= MEMtPN8(dataManager_getTagArrayAfield(firstTagNodeRef, tag, tagContentLength, is0070GroupTag, false));
	tagArrayContent		= tagNode_getTagArrayContent(tagArrayAfield);
	
	if(U16_A_U16(offset, tagContentLength) > maxLength)
	{
		throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}
	
	if (!isInProgress())
	{
		begin_transaction(SINGLE_APDU_PROCESS);
		transactionStarted	= true;
	}
	
	tagNode_setTagArrayLength(tagArrayAfield, tagContentLength);
	memCopyAtomic(tagArrayContent, PX8_A_U16(desAfield, offset), tagContentLength);
	
	if(true == transactionStarted)
	{
		commit_transaction();
	}
}
#endif

void dataManager_dataManagerFunction(void) {
	u8 				funcType;
	jref			sdRef;
	PN8				sdDataObj;
	u8 				is0070GroupTag;
	u16				tagAttri;
	u16				LenOrOffset;
	u16				dataTag;
	u16				tagContentLength;
	jref			tempRef;
	jref			desRef;
	jref			firstTagNodeRef;

	#define			NULL_RETURN					0

	#define			REGISTER_DATA			   	0
	#define			STORE_DATA				   	1
	#define			GET_DATA_JAVA_TAG		   	2
	#define			GET_JAVA_TAG_OFFSET		   	3
	#define			GET_JAVA_TAG_LENGTH		   	4
	#define			GET_DATA				   	5
	#define			GET_TAG_CONTENT				6
	#define			SET_TAG_CONTENT				7
	#define         ABORT_STORE_DATA            8 //modify by xuxiaojun@20220725 如果store key抛异常之后，C层直接调用的throw_error会导致store data的相关状态未清掉。所以在JAVA层的storeData(Applet sd)函数的ISOException异常进行了捕获

	funcType			= U16t8(popShort());
	tagContentLength	= popShort();
	tempRef				= popReference();
	desRef				= popReference();
	is0070GroupTag		= U16t8(popShort());
	LenOrOffset			= popShort();
	dataTag				= popShort();
	sdRef				= popReference();

	sdDataObj			= MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(sdRef)));
	firstTagNodeRef		= sdData_getFirstTagNode(sdDataObj);

#if GP__DATA_MANAGER__FOR_MASK
	switch (funcType) {
		case REGISTER_DATA:			
			tagAttri			= dataManager_calcTagAttri(is0070GroupTag);
			tagNode_getTagArray(firstTagNodeRef, dataTag, LenOrOffset, tagAttri, tempRef, true);
			pushShort(NULL_RETURN);
			break;
		case STORE_DATA:
			pushShort(U8t16(dataManager_storeData(sdDataObj)));
			break;
		case GET_DATA_JAVA_TAG:
			pushShort(getCurStoreDataTag());
			break;
		case GET_JAVA_TAG_OFFSET:
			pushShort(getCurStoreDataOffset());
			break;
		case GET_JAVA_TAG_LENGTH:
			pushShort(getCurStoreDataTagLength());
			break;
		case GET_DATA:
			pushShort(U8t16(dataManager_getData(sdDataObj)));
			break;
		case GET_TAG_CONTENT:
			pushShort(dataManager_getTagContent(firstTagNodeRef, desRef, LenOrOffset, dataTag, is0070GroupTag));
			break;
		case SET_TAG_CONTENT:
			dataManager_setTagContent(firstTagNodeRef, desRef, LenOrOffset, dataTag, tagContentLength, is0070GroupTag);
			pushShort(NULL_RETURN);
			break;
		case ABORT_STORE_DATA:
			dataManager_abortStoreDataThrowError(dataTag);
			pushShort(NULL_RETURN);
			break;
		default:
			break;
	}
#else
	if(GET_DATA == funcType)
	{
		pushShort(U8t16(dataManager_getData(sdDataObj)));
		return ;
	}

	pushShort(STORE_JAVA_TAG);
#endif
}
jref dataManager_getSdDataRef(u8 sdId)
{
	jref			sdRef;
	jref			sdDataRef;
	memref			entry;
	PN8				sdObj;

	entry			=		getEntryByAppId(sdId);
	sdRef 			= 		nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
	sdObj				= MEMtPN8(locateObjectInMemory(sdRef));
	sdDataRef 			= memReadU16(get_object_field_ref(MEMtPF8(sdObj), sdData_INDEX));//sdData_getSdDataRef(sdObj);

	return 	sdDataRef;
}
void dataManager_getCardRecognitionData(void)
{
	jref			sdId;
	u16 			tagLength;
	u16				offSet;
	PM8				arrayAddr;

	offSet = popShort(); //offSet    待输出数据所在数组的偏移
	arrayAddr = MEMtPM8(U32_A_U16(getJavaObjectDataArray(), offSet)); //arrayAddr     输出数据的数组.
	sdId = popShort(); //sdId    当前选择的SD

	tagLength = getCardRecognitionData(sdId, arrayAddr);

	pushShort(tagLength);
}

/**
  获取并拼装CardRecognitionData
*
* @param sdId    当前选择的SD
*
* @param arrayAddr     输出数据的数组.
*
*return ：CardRecognitionData的长度
*
*/
u16 getCardRecognitionData(u8 sdId, PX8 arrayAddr)
{
	PN8				sdDataObj;
	jref			tagArrayRef;
	jref 			firstTagNodeRef;
	PN8				tagArrayAField;
	u16 			tagLength;
	u16				offSet;

	jref			sdDataRef;

	sdDataRef 	= dataManager_getSdDataRef(sdId);

	if(sdDataRef == REF_NULL)
	{
		return 0;
	}
	sdDataObj 		= 		MEMtPN8(locateObjectInMemory(sdDataRef));

	firstTagNodeRef		= sdData_getFirstTagNode(sdDataObj);
	tagArrayRef			= tagNode_getTagArray(firstTagNodeRef, 0x0066, 0, 2, MEM_NULL, false);
	if(REF_NULL == tagArrayRef)
	{
		return 0;
	}
	tagArrayAField		= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(tagArrayRef))));
	tagLength			= tagNode_getTagArrayLength(tagArrayAField);
#if JAVA_CARD_UICC
	ecpMem_Copy(tempBuf, tagNode_getTagArrayContent(tagArrayAField), tagLength); //从66tag的数据中找73tag
	offSet = FindTag(tempBuf, 0x73, tagLength);
	if(offSet == FALSE)
	{
		return 0;
	}

	tagLength = tempBuf[offSet]+2;

	ecpMem_Copy(arrayAddr, &tempBuf[offSet-1], tagLength);
#else
	ecpMem_Copy(arrayAddr, tagNode_getTagArrayContent(tagArrayAField), tagLength);
#endif
	return tagLength;
}

u16 GetSDGeneraldata(u8* buf, u16 tag, memref sdDataObj, u16 maxLen, u8 is0070group)
{
	u16 I_A firstTagNodeRef;
	u16 I_A tagAttri;
	u16 I_A tagArrayRef;
	memref X_A tagArrayAField;
	u16 I_A tagLength;

	firstTagNodeRef		= sdData_getFirstTagNode((PN8)sdDataObj);

	tagAttri			= dataManager_calcTagAttri(is0070group);
	tagArrayRef			= tagNode_getTagArray(firstTagNodeRef, tag, 0, tagAttri, MEM_NULL, false);

	if(REF_NULL == tagArrayRef)
	{
		return 0x00;
	}

	tagArrayAField		= get_array_data(MEMtPN8(locateObjectInMemory(tagArrayRef)));
	tagLength			= tagNode_getTagArrayLength(tagArrayAField);

	if(tagLength > maxLen)
	{
		return 0x00; //throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}

	memCopyAtomic(buf, tagNode_getTagArrayContent(MEMtPN8(tagArrayAField)), tagLength);

	return tagLength;
}

/*===================================================================================================================================*/
/*===================================================================================================================================*/
