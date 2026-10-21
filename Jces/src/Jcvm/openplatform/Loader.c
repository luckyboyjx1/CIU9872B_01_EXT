#include "CgmManager.h"
#include "loader.h"
#include "contentManager.h"
#include "packageloc.h"
#include "bc_array.h"
#include "bc_object.h"
#include "object.h"
#include "ob_manager.h"
#include "nativemethod_impl.h"
#include "bc_object.h"
#include "mask.h"
#include "memory.h"
#include "utilMethod.h"
#include "grt.h"
#include "except.h"
#include "global_variable.h"
#include "SysConfigData.h"
#include "cardmanager.h"
#include "JcvmRam.h"
#include "NvmMap.h"

#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#define INIT  0

#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_LOADER_RAM_START
#endif
/*@ -namechecks @*/
/****************************************used in Import Component only******************************/
static u8 X_A f_pkgIndex;

/****************************************used in static method only*********************************/
static u16		X_A  f_aiCount;
static u16 		X_A	f_sc_offset;
static u8  		X_A	f_aiType;
static u16 		X_A	f_aiOffset;
static u16 		X_A	f_ndOffset;
static u16 		X_A	f_aiByteCount;
static u16 		X_A	f_aiElementCount;
static u16 		X_A	f_ndByteCount;
static u16 		X_A	f_seg2Count;
static jref 	X_A	f_objectArray;
static memref 	X_A	f_objectArrayAddr;


#define CLASS_LINK_START					0
#define CLASS_LINK_INIT						1
#define CLASS_LINK_INTERFACE_INIT			2
#define CLASS_LINK_INTERFACE				3
#define CLASS_LINK_INTERFACE_REMOTE			4
#define CLASS_LINK_FIELD					5
#define CLASS_LINK_PUBLIC_VIRTUAL_INIT		6
#define CLASS_LINK_PUBLIC_VIRTUAL			7
#define CLASS_LINK_PACKAGE_VIRTUAL_INIT		8
#define CLASS_LINK_PACKAGE_VIRTUAL			9
#define CLASS_LINK_IMP_INTERFACE_INIT		10
#define CLASS_LINK_IMP_INTERFACE			11
#define CLASS_LINK_REMOTE_INFO_INIT			12
#define CLASS_LINK_REMOTE_INFO				13


u8  	g_publicMehtodCount;
u8		g_packageMethodCount;
u8		g_implementInterCount;

#if JCVM__VM__RMI
u8		g_remoteInterfaceCount;
u8		g_maxRemoteInterface;
#endif

u8		g_interfaceCount;
u8		g_isLenEnough;
u8		g_isResolvPublic;
u16		g_ClassOffset;
memref	g_CurClassAddr;
memref	g_superClassAddr;
memref	g_superclassCompAddr;




/*****************************************************************************
 * File:  loader.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,11,14	.created
 Original author: wang hui
 *****************************************************************************/
static boolean verifyAPDUOrder()
{
	//g_previous_cap_tag is the previous component tag

	if (g_cap_tag == g_previous_cap_tag)
	{
		return true;
	}
	// skip if first time
	if ((g_previous_cap_tag < CAP_COMPONENT_HEADER) && (g_cap_tag == CAP_COMPONENT_HEADER))
	{
		return true;
	}
	//component arrives late? 
	if (bLoadOrderArr[g_cap_tag] < bLoadOrderArr[g_previous_cap_tag])
	{
		setLoaderException(ERROR_COMP_ORDER);
		return false;
	}
	/*
	 * component arrives early (any gap)?
	 * (!! update the following if download order changes!! )
	 */
	if (bLoadOrderArr[g_cap_tag] > U16t8(U8_A_U8(bLoadOrderArr[g_previous_cap_tag], 1)))
	{
		// applet component absent? class component immediately follows the applet component
		if (g_cap_tag == CAP_COMPONENT_CLASS)
		{
			// OK if the flag indicates the absence of an applet
			if (U8_AND_U8(g_capFlags, ACC_APPLET) == 0x00)
			{
				return true;
			}
		}
		// applet component absent constantpool component immediately follows the export component
		else if (g_cap_tag == CAP_COMPONENT_CONSTANTPOOL)
		{
			// OK if the flag indicates the absence of export
			if (U8_AND_U8(g_capFlags, ACC_EXPORT) == 0x00)
			{
				return true;
			}
		}
		else if (g_cap_tag == CAP_COMPONENT_DEBUG){
					return true;
		}
		else
		{
			// an illegal "gap" is found
			setLoaderException(ERROR_COMP_ORDER);
			return false;
		}
	}
	// previous component downloaded completely?
//	if (!g_loadComplete[g_previous_cap_tag])
	if (!U16_AND_U16(U16_SHR_U8(g_loadComplete, g_previous_cap_tag), 0x01))
	{
		setLoaderException(ERROR_COMP_ORDER);
		return false;
	}
	return true;
}

static void saveBytes(u16 offset, u16 count)
{
	u16		D_A		i;

	g_bytesLeft	= true;
	for (i = 0; i < count; U16_INC(i))
	{
		g_savedBytes[g_savedBytesCount] = g_buffer[offset];
		U8_INC(g_savedBytesCount);
		U16_INC(offset);
		U16_DEC(g_dataSize);
	}
}

static void resetSegmentProcessor()
{
	if (g_bytesLeft)
	{
		g_bytesLeft = false;
		// save the bytes left to the end of the APDU header
		while (g_savedBytesCount > 0)
		{
			U16_INC(g_dataSize);
			U16_DEC(g_dataOffset);
			U8_DEC(g_savedBytesCount);
			g_buffer[g_dataOffset]			= g_savedBytes[g_savedBytesCount];
			// set it to the initial value
			g_savedBytes[g_savedBytesCount] = 0xff;
		}
	}
	g_more = true;
}

u8 checkComponentSize()
{
	u16 compareSize;

	switch(g_cap_tag)
	{
		case CAP_COMPONENT_CLASS:
			compareSize = class_component_Size;
			break;
		case CAP_COMPONENT_METHOD:
			compareSize = method_component_Size;
			break;
		case CAP_COMPONENT_EXPORT:
			compareSize = export_component_Size;
			break;
		default:
			return 1;
	}

	if(g_componentSize <= compareSize)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/*
 * modifed by xxj@20210328: 增加对全局变量的清0.
 * 如果下载包的过程发生异常，应该在哪里清0？
 */
static void cmpLoad()
{
	u1	B_A		doCommit;

	doCommit = false;

	g_offset = g_dataOffset;

	g_previous_cap_tag = g_cap_tag;
	
	f_linkers[g_cap_tag].process();

	if (g_componentSize <= g_dataSize)
	{
		f_linkers[g_cap_tag].postProcess();
		
//		if ((g_lastBlock == (u8) 0x80)
//		&& (g_loadComplete[CAP_COMPONENT_REFERENCELOCATION] == true))
		if ((g_lastBlock == U32t8(0x80))
		&& (U16_AND_U16(U16_SHR_U8(g_loadComplete, CAP_COMPONENT_REFERENCELOCATION), 0x01) == 0x01))
		{
			doCommit = false;
			if (!isInProgress())
			{
				begin_transaction(SINGLE_APDU_PROCESS);
				doCommit = true;
			}
			
			//appMgrcommit();
			PackageMgrCommit();

			if (doCommit)
			{
				commit_transaction();
			}

			G_PkgComponentAddrStartAddr = 0;
			G_PkgComponentAddrEndAddr   = 0;
		}
	}
}

void setLoaderStatus()
{
	resetLinker();
	g_LoaderState	= U8_AND_U8(U16t8(popShort()), 0xff);
}

#if !GP__DAP__FOR_MASK
u16 resolveE2C4command(void)
{
	u16	lc;

	g_resolvResult				= FINISH;
	g_offset					= U16_A_U16(g_offset, 1);
	lc							= APDUBuffer[OFFSET_LC];

	if(0x83 == APDUBuffer[g_offset])
	{
		lc						= U16_S_U16(lc, 5);
		g_offset				= U16_A_U16(g_offset, 4);
	}
	else if(0x82 == APDUBuffer[g_offset])
	{
		lc						= U16_S_U16(lc, 4);
		g_offset				= U16_A_U16(g_offset, 3);
	}
	else if(0x81 == APDUBuffer[g_offset])
	{
		lc						= U16_S_U16(lc, 3);
		g_offset				= U16_A_U16(g_offset, 2);
	}
	else
	{
		lc						= U16_S_U16(lc, 2);
		g_offset				= U16_A_U16(g_offset, 1);
	}

	ecpMem_Copy(&APDUBuffer[OFFSET_CDATA], &APDUBuffer[g_offset], lc);

	APDUBuffer[OFFSET_LC]	= lc;

	return (SW_NO_ERROR);
}
#else
u16 resolveE2C4command(void)
{
	u16		D_A		l_apduOffsetLimit;
	u16		D_A		l_dataLenth;
	
	g_resolvResult		= INIT;
	l_apduOffsetLimit	= U8t16(U8_A_U8(U8_AND_U8(PX_R_U8(&APDUBuffer[OFFSET_LC]), 0xff), OFFSET_CDATA));
	
	while(g_offset < l_apduOffsetLimit)
	{
		switch(f_currentState)
		{
			case E2_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) == 0xE2)
				{
					g_E2C4tag		= 0xE2;
					U16_INC(g_offset);
					U8_INC(f_currentState);
				}
				else if(PX_R_U8(&APDUBuffer[g_offset]) == 0xC4)
				{
					f_currentState	= C4_TAG;
				}
#if GPC_2_3_1
				else if(PX_R_U8(&APDUBuffer[g_offset]) == 0xD3)
				{
					f_currentState = D3_TAG;
				}
#endif
				else if (APDUBuffer[g_offset] == 0xD4)
				{
					f_currentState = D4_TAG;			
				}
				else
				{
					return(SW_WRONG_DATA);
				}
				break;
			case E2_SIZE_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) == 0x81)
				{
					U16_INC(g_offset);
					f_currentState	= E2_SIZE_ONEBYTE;
					break;
				}
				else if(PX_R_U8(&APDUBuffer[g_offset]) < 0x7F)
				{
					g_E2Size		= PX_R_U8(&APDUBuffer[g_offset]);
				}
				else
				{
					;
				}
				U16_INC(g_offset);
				f_currentState		= SD_TAG;
				break;
			case E2_SIZE_ONEBYTE:
				g_E2Size			= PX_R_U8(&APDUBuffer[g_offset]);
				U16_INC(g_offset);
				f_currentState		= SD_TAG;
				break;
			case SD_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) != 0x4F)
				{
					return(SW_WRONG_DATA);
				}
				U16_INC(g_offset);
				U8_INC(f_currentState);
				break;
			case SD_LEN:
				if(PX_R_U8(&APDUBuffer[g_offset]) > 16)
				{
					return(SW_WRONG_DATA);
				}
				g_aidLength			= PX_R_U8(&APDUBuffer[g_offset]);
				g_aidOffset			= 0;
				f_currentState		= SD_VALUE;
				U16_INC(g_offset);
				break;
			case SD_VALUE:
				if (g_aidOffset != g_aidLength)
				{
					PXB_W_U8(PX8_A_U8(MEMtPX8(g_OPbTemp), g_aidOffset), PX_R_U8(&APDUBuffer[g_offset]));
					U8_INC(g_aidOffset);
					U16_INC(g_offset);
			
					if(g_aidOffset == g_aidLength)
					{
						f_currentState = SIGDATA_TAG;
					}
				}
				break;
			case SIGDATA_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) != 0xC3)
				{
					return(SW_WRONG_DATA);
				}
				U8_INC(f_currentState);
				U16_INC(g_offset);
				break;
			case SIGDATA_SIZE_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) == 0x81)
				{
					U16_INC(g_offset);
					f_currentState	= SIGDATA_SIZE_ONEBYTE;
					break;
				}
				else if(PX_R_U8(&APDUBuffer[g_offset]) < 0x7F)
				{
					g_SigDataSize	= PX_R_U8(&APDUBuffer[g_offset]);
					g_E2Size		= U8_S_U8(g_E2Size, 4);	//4F , g_aidLength, C3, g_SigDataSize	
				}
				else
				{
					;
				}
				U16_INC(g_offset);
				f_currentState		= SIGDATA_VALUE;
				g_sigDataOffset		= 0;
				break;
			case SIGDATA_SIZE_ONEBYTE:
				g_SigDataSize		= PX_R_U8(&APDUBuffer[g_offset]);
				g_E2Size			= U8_S_U8(g_E2Size, 5);	//4F, g_aidLength ,C3, 81 ,g_SigDataSize
				/*
				if(APDUBuffer[g_offset] != SIGDATALEN)
				{
					return(SW_WRONG_DATA);
				}*/
				U16_INC(g_offset);
				f_currentState		= SIGDATA_VALUE;
				g_sigDataOffset		= 0;
				break;
			case SIGDATA_VALUE:
				if(g_E2Size != U16t8(U8_A_U8(g_aidLength, g_SigDataSize)))
				{
					return(SW_WRONG_DATA);
				}
				if (g_sigDataOffset != g_SigDataSize)
				{
					PXB_W_U8(PX8_A_U8(MEMtPX8(g_OPbTempDAP), g_sigDataOffset), PX_R_U8(&APDUBuffer[g_offset]));
					U8_INC(g_sigDataOffset);
					U16_INC(g_offset);
					
					if(g_sigDataOffset == g_SigDataSize)
					{
						f_currentState	= E2_TAG;
						g_resolvResult	= GOAHEAD;
						return (SW_NO_ERROR);
					}
				}
				break;
			case C4_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) == 0xC4)
				{
					g_E2C4tag			= 0xC4;
					f_currentState		= C4_SIZE_TAG;
				}
				else if(PX_R_U8(&APDUBuffer[g_offset]) == 0xE2)
				{
					g_E2C4tag			= 0xE2;
					f_currentState		= E2_SIZE_TAG;
				}
				else
				{
					g_E2C4tag			= 0x00;
					return(SW_WRONG_DATA);
				}
				U16_INC(g_offset);
				break;
#if GPC_2_3_1
			case D3_TAG:
				f_currentState 		= D3_LEN;
				U16_INC(g_offset);
				break;
			case D3_LEN:
				g_icvsize			= APDUBuffer[g_offset];
				g_icvoffset			= 0;
				f_currentState 		= D3_VALUE;
				U16_INC(g_offset);
				break;
			case D3_VALUE:
				if(g_icvoffset != g_icvsize)
				{
					g_icvbuf[g_icvoffset]	= APDUBuffer[g_offset];
					U8_INC(g_icvoffset);
					U16_INC(g_offset);

					if(g_icvoffset == g_icvsize)
					{
						f_currentState 		= E2_TAG;

					}
				}
				break;
#endif
			case D4_TAG: 
				g_E2C4tag = 0xD4;
				f_currentState = D4_SIZE_TAG;
				U16_INC(g_offset);
				break;
			case D4_SIZE_TAG:
			case C4_SIZE_TAG:
				if(PX_R_U8(&APDUBuffer[g_offset]) == 0x83)
				{
					U16_INC(g_offset);
					f_currentState		= C4_SIZE_THREEBYTE;
					g_C4LenOffset		= 0;
					break;
				}
				if(PX_R_U8(&APDUBuffer[g_offset]) == 0x82)
				{
					U16_INC(g_offset);
					f_currentState		= C4_SIZE_TWOBYTE;
					g_C4LenOffset		= 0;
					break;
				}
				else if(PX_R_U8(&APDUBuffer[g_offset]) == 0x81)
				{
					U16_INC(g_offset);
					f_currentState		= C4_SIZE_ONEBYTE;
					break;
				}
				else if(PX_R_U8(&APDUBuffer[g_offset]) < 0x7F)
				{
					PXB_W_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H]), PXB_R_U8(&APDUBuffer[g_offset]));
				}
				else
				{
					;
				}
				U16_INC(g_offset);
				f_currentState = C4_VALUE;
				break;
			case C4_SIZE_ONEBYTE:
				PXB_W_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H]), PXB_R_U8(&APDUBuffer[g_offset]));
				U16_INC(g_offset);
				f_currentState = C4_VALUE;
				break;
			case C4_SIZE_TWOBYTE:
				if (g_C4LenOffset < 2)
				{
					PXB_W_U8(PX8_A_U8(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_L]), g_C4LenOffset), PX_R_U8(&APDUBuffer[g_offset]));
					U8_INC(g_C4LenOffset);
					U16_INC(g_offset);
					
					if(g_C4LenOffset == 2)
					{
						f_currentState = C4_VALUE;
					}
				}
				break;
			case C4_SIZE_THREEBYTE:
				if (g_C4LenOffset < 3)
				{
					PXB_W_U8(PX8_A_U8(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H]), U8_A_U8(g_C4LenOffset,1)), PX_R_U8(&APDUBuffer[g_offset]));//C4总长度占3字节的时候  用2个short表示长度  前1个short只用1字节
					U8_INC(g_C4LenOffset);
					U16_INC(g_offset);

					if(g_C4LenOffset == 3)
					{
						f_currentState = C4_VALUE;
					}
				}
				break;
			case C4_VALUE:
				l_dataLenth				= U16_S_U16(l_apduOffsetLimit, g_offset);
				ecpMem_Copy(MEMtPX8(&APDUBuffer[OFFSET_CDATA]), MEMtPX8(&APDUBuffer[g_offset]), l_dataLenth);
				PX_W_U8(&APDUBuffer[OFFSET_LC], U16t8(l_dataLenth));  //(lzm modified according [MISA])
				g_offset				= l_apduOffsetLimit;  //(lzm modified according [MISA])
				g_resolvResult			= GOAHEAD;
				f_currentState			= E2E4_FINISH;
				break;
			case E2E4_FINISH:
				g_resolvResult			= FINISH;
				U8_INC(f_currentState);
				break;
			default:
				return (SW_WRONG_DATA);
		}
	}
	
	if(g_resolvResult == INIT)
	{
		g_resolvResult = NEXTAPDU;
	}
	
	return (SW_NO_ERROR);
	
}
#endif

void load()
{
	//g_buffer		= APDUBuffer;
	g_lastBlock		= g_buffer[OFFSET_P1];
	
	g_dataSize		= U16_AND_U16(PX_R_U8(&APDUBuffer[OFFSET_LC]), 0x00ff); // bytes of avairable data in apdubuffer
	g_dataOffset	= OFFSET_CDATA;// offset of avairable data in apdubuffer 
	

	if(!OS_C4_NOCHECK)
	{
		if(g_dataSize > PXB_R_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H])))
		{
			//setLoaderException(ERROR_ABORTED);
			bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
			setLoaderException(SW_WRONG_DATA);
			return;
		}
		else
		{
			//bShortBuffer[LOAD_C4_VALUE] = bShortBuffer[LOAD_C4_VALUE] - g_dataSize;
			PXB_W_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H]), (PXB_R_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H])) - g_dataSize));
		}
		
		if(U8_AND_U8(g_lastBlock, 0x80) == 0x80)
		{
			if(PXB_R_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H])) != 0)
			{
				//setLoaderException(ERROR_ABORTED);
				bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
				setLoaderException(SW_WRONG_DATA);
				return;
			}
		}
	}


	while (g_dataSize > 0)
	{

		switch (g_LoaderState)
		{
		case WAIT_4_TAG:
			if (PXB_R_U16(MEMtPX8(&bShortBuffer[INSTALLER_EXCEPTION])) != NO_EXCEPTION)
			{
				bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
				return;
			}
			g_cap_tag		= PX_R_U8(&g_buffer[g_dataOffset]);
			U16_INC(g_dataOffset);

			if ((g_cap_tag < CAP_COMPONENT_HEADER) || (g_cap_tag > CAP_COMPONENT_DEBUG)||(g_loadComplete&(0x01<<g_cap_tag)))
			{
				g_cap_tag = CAP_COMPONENT_DESCRIPTOR;
			}
			if (!verifyAPDUOrder())
			{
				bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
				return;
			}
			f_linkers[g_cap_tag].init();
			U16_DEC(g_dataSize);
			g_LoaderState	= WAIT_4_LEN_H;
			break;
		case WAIT_4_LEN_H:
			if(g_cap_tag == CAP_COMPONENT_DEBUG){
				if (U4_SIZE > g_dataSize)
				{
					saveBytes(g_dataOffset, g_dataSize);
					return;
				}
				resetSegmentProcessor();
//				g_componentSize = PX_R_U8(&g_buffer[g_dataOffset]);
//				U16_INC(g_dataOffset);
//				U16_DEC(g_dataSize);
//
//				g_componentSize = U32_A_U16(U32_SHL_U8(g_componentSize, 8), PX_R_U8(&g_buffer[g_dataOffset]));
//				U16_INC(g_dataOffset);
//				U16_DEC(g_dataSize);
//
//				g_componentSize = U32_A_U32(g_componentSize, U32_SHL_U8(PX_R_U8(&g_buffer[g_dataOffset]), 24));
//				U16_INC(g_dataOffset);
//				U16_DEC(g_dataSize);
//
//				g_componentSize = U32_A_U32(g_componentSize, U32_SHL_U8(PX_R_U8(&g_buffer[g_dataOffset]), 16));
//				U16_INC(g_dataOffset);
//				U16_DEC(g_dataSize);
				g_componentSize = PX_R_U16(&g_buffer[g_dataOffset]);
				g_dataOffset += 2;
				g_dataSize	 -= 2;
				g_LoaderState = WAIT_4_DATA;

				if(g_componentSize == 0)
				{
					g_componentSize = PX_R_U16(&g_buffer[g_dataOffset]);
					g_dataOffset += 2;
					g_dataSize	 -= 2;
				}
				if(!OS_C4_NOCHECK)
				{
					if(!checkComponentSize())
					{
						bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
						setLoaderException(SW_WRONG_DATA);
						return;
					}
				}
			}else{

				g_componentSize = PX_R_U8(&g_buffer[g_dataOffset]);
				U16_INC(g_dataOffset);
				U16_DEC(g_dataSize);
				g_LoaderState	= WAIT_4_LEN_L;
			}
			break;
		case WAIT_4_LEN_L:
			g_componentSize	= U32_A_U16(U32_SHL_U8(g_componentSize, 8), PX_R_U8(&g_buffer[g_dataOffset]));
			U16_INC(g_dataOffset);
			U16_DEC(g_dataSize);
			g_LoaderState = WAIT_4_DATA;
			if(!OS_C4_NOCHECK)
			{
				if(!checkComponentSize())
				{
					bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
					setLoaderException(SW_WRONG_DATA);
					return;
				}
			}
			break;
		case WAIT_4_DATA:
			resetSegmentProcessor();
			
			//check:
			if(!OS_C4_NOCHECK)
			{
				if (g_componentSize > U32_A_U16(PXB_R_U32(MEMtPX8(&bShortBuffer[LOAD_C4_VALUE_H])), g_dataSize))
				{
					bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
					setLoaderException(SW_WRONG_DATA);
					return;	
				}
			}
			
			g_max			= U32t16(U16_A_U16(g_dataOffset, g_dataSize));
			if (g_componentSize == g_dataSize)
			{
				// apdu buffer contains one component
				cmpLoad();
				g_componentSize	= 0;
				g_LoaderState	= WAIT_4_TAG;
				return;
			}
			else if (g_componentSize > g_dataSize)
			{
				// apdu buffer contains parts of one component
				cmpLoad();
				g_componentSize	= U32_S_U16(g_componentSize, g_dataSize);
				return;
			}
			else
			{
				// apdu buffer contains several components
				g_max			= U32t16(U16_A_U16(g_dataOffset, g_componentSize));
				cmpLoad();
				g_dataSize		= U16_S_U16(g_dataSize, g_componentSize);
				g_dataOffset 	= U32t16(U16_A_U16(g_dataOffset, g_componentSize));
				g_componentSize = 0;
				g_LoaderState	= WAIT_4_TAG;
			}
			break;
		default:
			break;
		}//end switch
	}//end while
}



/*****************************************************************************
 component method
 *****************************************************************************/
// ram Variables
void resetLinker()
{
//	jbyte i;
	// reset globals
	g_previous_cap_tag		= 0;
	g_cap_tag               = 0;
	g_bytesLeft				= false;
	g_savedBytesCount		= 0;
	g_staticReferenceCount	= 0;
	f_currentState			= 0;

	// reset global arrays
/*	for (i = 0; i < INSTALLER_MAX; i++)
	{
		g_loadComplete[i] = false;
	}		 */
	g_loadComplete			= 0;
#if GPC_2_3_1
	g_icvsize               = 0;
#endif
	class_component_offset	= 0;
	method_component_offset = 0;
	export_component_offset = 0;
	constantpool_componentoffset = 0;
#if GP__DAP__FOR_MASK
	/*@ -disspecopt @*/
	g_OPbTemp		= get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPN8(OP_bTempArr_JC)))));
	g_OPbTempDAP	= get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPN8(OP_bTempDAP_JC)))));
	/*@ +disspecopt @*/
#endif
}

void componentInit()
{
	f_currentState = STATE_READY;
}

boolean postProcess()
{
//	g_loadComplete[g_cap_tag] = true;
	g_loadComplete	= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, g_cap_tag));
	return true;
}



/*****************************************************************************
 header Component method
 *****************************************************************************/
void headerComponentInit()
{
	PXB_W_U8(&bRamBuffer[INSTALLER_STATE_INDEX], INSTALLER_STATE_LOADING);
	g_aidOffset							= 0;
	f_currentState						= STATE_READY;
	g_more								= true;
}
/*
 * modified by xuxiaojun@20210414 1. 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 *                                2. 调整newObject和initAID和newPackageEntry，减少ceche提交的次数
 */
boolean headerComponentProcess()
{
	jref		D_A		theAid, pe;
	//PN8			D_A		packageAddr;  // remove warning,dengyiwen,2022/9/22

	while (g_more && (g_offset < g_max))
	{
		switch (f_currentState)
		{
		case STATE_READY:
			// a short is split in two APDUs
			if (isSplitValue(U4_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				return true;
			}
			if(PXB_R_U32(&g_buffer[g_offset]) != CAP_MAGIC)
			{
				setLoaderException(ERROR_CAP_MAGIC);
				return false;
			}
			g_offset	= U32t16(U16_A_U8(g_offset, 4));
			U8_INC(f_currentState);
			break;
			
		case STATE_MINOR:
			g_capMinor = g_buffer[g_offset]; //case STATE_MINOR:
			U16_INC(g_offset);
			if (g_capMinor > CAP_MINOR)
			{
				setLoaderException(ERROR_CAP_MINOR);
				return false;
			}
			U8_INC(f_currentState);
			break;
			
		case STATE_MAJOR:
			g_capMajor = g_buffer[g_offset]; //case STATE_MAJOR:
			U16_INC(g_offset);
			if (g_capMajor != CAP_MAJOR)
			{
				setLoaderException(ERROR_CAP_MAJOR);
				return false;
			}
			U8_INC(f_currentState);
			break;
			
		case STATE_FLAGS:
			g_capFlags = g_buffer[g_offset]; //case STATE_FLAGS:
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
			
		case STATE_PKG_MINOR:
			g_pkgMinor = g_buffer[g_offset]; //case STATE_PKG_MINOR:
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
			
		case STATE_PKG_MAJOR:
			g_pkgMajor = g_buffer[g_offset]; //case STATE_PKG_MAJOR:
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
			
		case STATE_AID_LENGTH:
			g_aidLength = g_buffer[g_offset]; //case STATE_AID_LENGTH:
			U16_INC(g_offset);
			// this fixed the gop install for load, if test TCK ,this's not ok
			if (g_aidLength != PXB_R_U8(&bRamBuffer[LOAD_PKG_LEN_OFF]))
			{
				setLoaderException(SW_CONDITIONS_NOT_SATISFIED);
				return false;
			}
			U8_INC(f_currentState);
			break;
			
		case STATE_AID_DATA:
			if (g_aidOffset != g_aidLength)
			{
				if (PXB_R_U8(&bRamBuffer[U8_A_U8(LOAD_PKG_AID_OFF, g_aidOffset)]) != g_buffer[g_offset])
				{
					setLoaderException(SW_WRONG_DATA);
					return false;
				}
				U8_INC(g_aidOffset);
				U16_INC(g_offset);
				if(g_aidOffset == g_aidLength)
				{
					//check if the package contains applets, that we have enough room for another applet package
					if (U8_AND_U8(g_capFlags, ACC_APPLET) == ACC_APPLET)
					{/*@ -disspecopt @*/
						if (nvmReadU8(G_CurExecuEnvironment.appletPkgCount) >= ON_CARD_APPLET_PKG_MAX)
						{
							setLoaderException(ERROR_ON_CARD_PKG_MAX_EXCEEDED);
							//InstallerException.throwIt(Errors.ON_CARD_APPLET_PKG_MAX_EXCEEDED);
							return false;
						}
						else
						{
							//if the package is an applet package, we have to
							// add it to the contexts table.
							addAppletPackage(PN_R_U8(PM_g_pkgIdInProcess_JC));
						}
					}/*@ +disspecopt @*/
					U8_INC(f_currentState);
				}
			}
			break;

		case HC_STATE_PKGNAMELEN:
			if (g_capMinor == 0x02)
			{
				g_pkgNameLength		= g_buffer[g_offset];
				g_pkgName			= _newarrayObject(AType_Byte, g_pkgNameLength, BIT_NEED_REFRESH);
				g_pkgNameBuffer		= get_array_data(MEMtPN8(locateObjectInMemory(g_pkgName)));
				g_nameOffset		= 0;
				U16_INC(g_offset);
				U8_INC(f_currentState);
			}
			break;

		case HC_STATE_PKGNAME:
			if(g_nameOffset != g_pkgNameLength)
			{
				nvmWriteU8NoAtomic(PN8_A_U8(MEMtPN8(g_pkgNameBuffer), g_nameOffset), g_buffer[g_offset]);
				U8_INC(g_nameOffset);
				U16_INC(g_offset);
				break;
			}
			break;
			
		default:
			break;
		}//end switch
	}//end while

	if (!hasMoreAPDU())
	{
		//beginTransaction();
		theAid = initAID(bRamBuffer, LOAD_PKG_AID_OFF, g_aidLength);
		pe		= newPackageEntry(theAid, g_pkgMajor, g_pkgMinor, g_capMinor); // 调整pe数据的写顺序为低地址到高地址
//		if (g_capMinor == 0x02)
//		{
//			packageAddr	= MEMtPN8(locateObjectInMemory(pe));
//			store_object_S_filed(packageAddr, pkgName_INDEX, g_pkgName);
//			store_object_B_filed(packageAddr, pkgNamelength_INDEX, g_pkgNameLength);
//		}
		/*@ -disspecopt @*/nvmWriteU16Atomic(MEMtPN8(PM_g_newPackage_JC), pe);/*@ +disspecopt @*/
		//commitTransaction();
	//	g_loadComplete[CAP_COMPONENT_HEADER] = true;
		g_loadComplete	= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_HEADER));
	}
	
	return true;
}

/*****************************************************************************
 Directory Component method
 *****************************************************************************/
// ram Variables only used in header component
void DirectoryComponentInit()
{
	f_currentState = STATE_READY;
}
/*
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 * modified by xxj@20210414 调整G_CurExecuEnvironment.pkgLocationTable写的位置，减少cache提交次数
 */
boolean DirectoryComponentProcess()
{
	u8		D_A		newPackageIdentifier;
	u8		D_A		low_version;
	memref	D_A		pkg_info_ref;
	memref  D_A		pkg_info_addr;
	u8		D_A		temBuffer[24];
	u32     D_A     offset;

	low_version	= 0;

	if ((g_capMajor < 2) || ((g_capMajor == 2) && (g_capMinor < 2)))
	{
		low_version = 1;
	}
	
	while (g_more && (g_offset < g_max))
	{
		if(f_currentState < DERETORY_FIN)
		{
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				return true;
			}
		}
		
		switch(f_currentState)
		{
			case HEADER_SIZE:
			case DERETORY_SIZE:
			case APPLET_SIZE:
			case IMPORT_SIZE:
				g_offset	= U32t16(U16_A_U8(g_offset, 2));// 2byte component size ,8byte hearder directory applet and import component size not used.
				U8_INC(f_currentState);
				break;
				
			case CONSTANT_SIZE:
				constantpool_component_Size	= getShort(g_buffer,g_offset);
				g_offset					= U32t16(U16_A_U8(g_offset, 2));
				U8_INC(f_currentState);
				break;
				
			case CLASS_SIZE:
				class_component_Size		= getShort(g_buffer,g_offset);
				g_offset					= U32t16(U16_A_U8(g_offset, 2));
				U8_INC(f_currentState);
				break;
			
			case METHOD_SIZE:
				method_component_Size		= getShort(g_buffer,g_offset);
				g_offset					= U32t16(U16_A_U8(g_offset, 2));// 2byte method_component_Size and 2byte staticfield component size and reflocation component size .
				U8_INC(f_currentState);
				break;
				
			case STATICFILED_SIZE:
			case REFLOCATION_SIZE:
				g_offset					= U32t16(U16_A_U8(g_offset, 2));// 2byte component size ,8byte hearder directory applet and import component size not used.
				U8_INC(f_currentState);
				break;
				
			case EXPORT_SIZE:
				export_component_Size		= getShort(g_buffer,g_offset);
				g_offset					= U32t16(U16_A_U8(g_offset, 2));
				U8_INC(f_currentState);
				break;
				
			case DESCRIPTOR_SIZE:
				g_offset					= U32t16(U16_A_U8(g_offset, 2));// 2byte component size ,8byte hearder directory applet and import component size not used.
				if (low_version)
				{
					f_currentState			= U16t8(U8_A_U8(2, f_currentState));  //go to STATIC_IMA_SIZE
				}
				else
				{
					U8_INC(f_currentState);
				}
				break;
				
			case DEBUG_SIZE:
				g_offset					= U32t16(U16_A_U8(g_offset, 2));
				U8_INC(f_currentState);
				break;
			
			case STATIC_IMA_SIZE:
				staticfield_component_Size	= getShort(g_buffer,g_offset); // there is static image_size
				g_offset					= U32t16(U16_A_U8(g_offset, 2));
				U8_INC(f_currentState);
				/* no break */
			
			case DERETORY_FIN:
				//beginTransaction();
				/*@ -disspecopt @*/newPackageIdentifier		= PN_R_U8(PM_g_pkgIdInProcess_JC);/*@ +disspecopt @*/


//				class_component_Addresses		= NvmManager_Allocate(U16t32(class_component_Size), MAP_ENTRY_BLOCK_PCK_ID);
//				method_component_Addresses		= NvmManager_Allocate(U16t32(method_component_Size), MAP_ENTRY_BLOCK_PCK_ID);
//				staticfield_component_Addresses = NvmManager_Allocate(U16t32(staticfield_component_Size), MAP_ENTRY_BLOCK_PCK_ID);
//				export_component_Addresses 		= NvmManager_Allocate(U16t32(export_component_Size), MAP_ENTRY_BLOCK_PCK_ID);
//
//				if (class_component_Addresses != ILLEGAL_ADDRESS \
//					&& method_component_Addresses != ILLEGAL_ADDRESS \
//					&& staticfield_component_Addresses != ILLEGAL_ADDRESS \
//					&& export_component_Addresses != ILLEGAL_ADDRESS)
//				{
					//nvmFillAtomic(class_component_Addresses, chunkSize, 0);


				pkg_info_ref = NvmManager_Allocate(U32_M_U32(PKGLOC_COMPONENT_COUNT, U8_A_U8(SIZE_MEMREF, SIZE_U16)) + class_component_Size + method_component_Size + staticfield_component_Size + export_component_Size, MAP_ENTRY_BLOCK_PCK_ID);

					if (pkg_info_ref == REF_NULL)
					{
						throw_error(SYSTEMEXCEPT_NO_RESOURCE);
						return false;
					}

					/* Fill out package location information for each component */
					offset = U32_M_U32(PKGLOC_COMPONENT_COUNT, U8_A_U8(SIZE_MEMREF, SIZE_U16));
					PXB_W_U32(&temBuffer[0], offset); // class_component offset
					PXB_W_U16(&temBuffer[4], class_component_Size);

					offset += class_component_Size;
					PXB_W_U32(&temBuffer[6], offset); // method_component offset
					PXB_W_U16(&temBuffer[10],method_component_Size );

					offset += method_component_Size;
					PXB_W_U32(&temBuffer[12], offset); // method_component staticfield
					PXB_W_U16(&temBuffer[16], staticfield_component_Size);

					offset += staticfield_component_Size;
					PXB_W_U32(&temBuffer[18], offset); // export_component staticfield
					PXB_W_U16(&temBuffer[22], export_component_Size);

					pkg_info_addr = MapTable_GetEntityContent(pkg_info_ref); // 该函数偏移过了对象头

					nvmCopyNoAtomic(MEMtPN8(pkg_info_addr), temBuffer, 24);

					memRefresh();

					/* Add entry into the table */
					/*@ -disspecopt @*/
					nvmWriteU32Atomic(PN8_A_U16(MEMtPN8(G_CurExecuEnvironment.pkgLocationTable), U8_M_U8(newPackageIdentifier, SIZE_MEMREF)), pkg_info_ref);
					/*@ +disspecopt @*/

					offset = U32_M_U32(PKGLOC_COMPONENT_COUNT, U8_A_U8(SIZE_MEMREF, SIZE_U16));
					class_component_Addresses			= pkg_info_addr + offset;

					offset += class_component_Size;
					method_component_Addresses			= pkg_info_addr + offset;

					offset += method_component_Size;
					staticfield_component_Addresses 	= pkg_info_addr + offset;

					offset += staticfield_component_Size;
					export_component_Addresses			= pkg_info_addr + offset;

					G_PkgComponentAddrStartAddr = pkg_info_addr;

					G_PkgComponentAddrEndAddr = G_PkgComponentAddrStartAddr + MapTable_GetEntitySize(MEMtPN8(MapTable_GetEntryAddr(pkg_info_ref))) - MAP_ENTRY_ENTITY_HEAER_SIZE; // MapTable_GetEntitySize获取的结果包含了对象头
//				}
//				else
//				{
//					throw_error(SYSTEMEXCEPT_NO_RESOURCE);
//					return false;
//				}
				//commitTransaction();
				// done!
				//g_loadComplete[CAP_COMPONENT_DIRECTORY] = true;
				g_loadComplete		= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_DIRECTORY));
				U8_INC(f_currentState);
				break;
				
			case IGNORE_SIZE:
				g_offset			= g_max;
				break;
				
			default:
				break;
			}
	}
	
	
	return true;
}

/*****************************************************************************
 Applet Component method
 *****************************************************************************/
// ram Variables only used in header component
// process states 
void AppletComponentInit()
{
	f_currentState	= AC_STATE_COUNT;
	g_more			= true;
}
/*
 * modified by xuxiaojun@20210414 调整newObject和initAID和newPackageEntry，减少ceche提交的次数
 */
boolean AppletComponentProcess()
{
	u16		D_A		installOffset;
	u8		D_A		pkgContext;
	jref	D_A		aid;
	PN8		D_A		methodAddress;

	while (g_more && (g_offset < g_max))
	{
		switch (f_currentState)
		{
		case AC_STATE_COUNT:
			g_aidCount		= g_buffer[g_offset];
			if(g_aidCount >= 128)
			{
				setLoaderException(ERROR_EXCEPTION);
				return false;
			}
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case AC_STATE_LENGTH:
			g_aidLength		= g_buffer[g_offset];
			g_aidOffset		= 0;
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case AC_STATE_AID:
			// more aid bytes?
			if (g_aidOffset < g_aidLength)
			{
				g_appletAID[g_aidOffset] = g_buffer[g_offset];
				U8_INC(g_aidOffset);
				U16_INC(g_offset);
				break;
			}
			U8_INC(f_currentState);
			break;
		case AC_STATE_OFFSET:
			// a short is split in two APDUs
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				g_more		= false;
				break;
			}
			installOffset	= getShort(g_buffer,g_offset);
			g_offset		= U32t16(U16_A_U8(g_offset, 2));
			//beginTransaction();
			aid = initAID(MEMtPN8(g_appletAID), 0, g_aidLength);

			if (installOffset > METHOD_COMP_LIMIT)
			{
				methodAddress	= PN8_A_U32(MEMtPN8(method_component_Addresses), installOffset);
				//add to the global reference table and get the reference back.
				installOffset	= addReference(methodAddress, PN_R_U8(PM_g_pkgIdInProcess_JC));
				if (installOffset == 0)
				{
					setLoaderException(ERROR_EXCEPTION);
					return false;
				}
			}
			pkgContext			= U16t8(getPkgContext(PN_R_U8(PM_g_pkgIdInProcess_JC)));
			addAppletInfo(aid, pkgContext, installOffset);
			//commitTransaction();
			// more AID?
			U8_DEC(g_aidCount);
			if (g_aidCount > 0)
			{
				f_currentState = AC_STATE_LENGTH;
				break;
			}
			U8_INC(f_currentState);
			break;
		default:
			break;
		}//end switch
	}//end while
	if (hasMoreAPDU())
	{
		return true; // to get another APDU
	}
	// done!
//	g_loadComplete[CAP_COMPONENT_APPLET] = true;
	g_loadComplete = U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_APPLET));
	return true;
}

/*****************************************************************************
 ImportComponent method
 *****************************************************************************/

void ImportComponentInit()
{
	f_currentState		= IC_STATE_COUNT;
	f_pkgIndex			= 0;
	
	ecpMem_Set(f_pkgIDs, 0xFF, 16);
	g_more				= true;
}

boolean ImportComponentProcess()
{
	u8		D_A		m_b1;
	
	while (g_more && g_offset < g_max)
	{
		switch (f_currentState)
		{
		case IC_STATE_COUNT:
			g_aidCount = g_buffer[g_offset]; //STATE_COUNT:
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case IC_STATE_MINOR:
			g_pkgMinor = g_buffer[g_offset]; //STATE_MINOR
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case IC_STATE_MAJOR:
			g_pkgMajor = g_buffer[g_offset]; //STATE_MAJOR
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case IC_STATE_LENGTH:
			g_aidLength = g_buffer[g_offset];
			g_aidOffset = 0;
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case IC_STATE_AID:
			g_appletAID[g_aidOffset] = g_buffer[g_offset];
			U16_INC(g_offset);
			U8_INC(g_aidOffset);
			if (g_aidOffset == g_aidLength)
			{
				m_b1 = findPkgID(MEMtPX8(&g_appletAID[0]),  0, g_aidLength, U8_OR_U8(U8_SHL_U8(g_pkgMajor, 4), g_pkgMinor));
				// m_b1 is the internal package identifier
				if (m_b1 == ILLEGAL_ID)
				{
					setLoaderException(ERROR_IMPORT_NOT_FOUND);
					return false;
				}
				f_pkgIDs[f_pkgIndex] = m_b1;
				U8_INC(f_pkgIndex);
				// if have more importpkg ?
				U8_DEC(g_aidCount);
				if (g_aidCount > 0)
				{
					f_currentState = IC_STATE_MINOR;
				}
				else
				{
					U8_INC(f_currentState);
				}
			}
			break;
		default:
			break;
		} //end switch
	} //end while

	if (hasMoreAPDU())
	{
		return true; // to get another APDU
	}
	setImportInfo(MEMtPX8(&f_pkgIDs[0]), 0, f_pkgIndex);
	// done!
//	g_loadComplete[CAP_COMPONENT_IMPORT] = true;
	g_loadComplete = U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_IMPORT));
	return true;
}

/*****************************************************************************
 ConstantComponentInit

 modified by xuxiaojun@20210414 constantpool_component_Size小于0x100时用RAM来存储constantpool_component，可以减少分配与回收的次数，提升性能
 *****************************************************************************/
void ConstantComponentInit()
{
	componentInit();
	//beginTransaction();

#if GP__CGM__FOR_APPPARACHECK
//linguoming:config C7C8
	CgmInit_changeFlag(NO_CHECK);
#endif
    
	constantpool_component_Size				= U16_D_U8(constantpool_component_Size, 2);
    
#if CONSTANT_POOL_COMPONENT_IN_RAM // modify by xuxiaojun@20220606 用开关控制CONSTANTPOOL_COMPONENT是使用RAM还是EEP
	if(constantpool_component_Size <= sizeof(CONSTANTPOOL_COMPONENT))
    {
        constantpool_component_Addresses = PXtMEM(CONSTANTPOOL_COMPONENT);
    }
    else
#endif    
	{
        
		constantpool_component_Addresses		= U16t32(NvmManager_Allocate(U16t32(constantpool_component_Size), MAP_ENTRY_BLOCK_PCK_ID));

		if (constantpool_component_Addresses != ILLEGAL_ADDRESS)
		{/*@ -disspecopt @*/
			nvmWriteU32Atomic(MEMtPN8(PM_g_tempMemoryAddress_JC), constantpool_component_Addresses);
			nvmWriteU16Atomic(MEMtPN8(PM_g_tempMemorySize_JC), constantpool_component_Size);
		}/*@ +disspecopt @*/
		else
		{
			throw_error(SYSTEMEXCEPT_NO_RESOURCE);
		}

		constantpool_component_Addresses		= MapTable_GetEntityContent(U32t16(constantpool_component_Addresses));
	}   
#if GP__CGM__FOR_APPPARACHECK
//linguoming:config C7C8
	CgmInit_changeFlag(CHECK_LOAD);
#endif
	//commitTransaction();
}
/*
 *  modified by xuxiaojun@20210414 constantpool_component_Size小于0x100时用RAM来存储constantpool_component，可以减少分配与回收的次数，提升性能
 */
boolean ConstantComponentProcess()
{
	u16			D_A		m_s1;
	u16			D_A		bTempOff;
	PN8			D_A		m_addr; 
	PN8			D_A		methodAddress;
	PN8			D_A		m_superClassAddr;
	u16			D_A		methodRef, m_superClassRef, m_instanceSize;
	PN8			D_A		classCompAddress;
	u8			D_A		tempRL;

	bTempOff = 0;
	while (g_more && (g_offset < g_max))
	{
		switch (f_currentState)
		{
		case 0: // counter
			// a short is split in two APDUs
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				return true;
			}
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			break;

		case 1: // tag
			m_count = PX_R_U8(&APDUBuffer[g_offset]);
			U16_INC(g_offset);
			break;

		case 2: // b1
			m_base = PX_R_U8(&APDUBuffer[g_offset]);
			U16_INC(g_offset);
			break;

		case 3: // b2
			k_count = PX_R_U8(&APDUBuffer[g_offset]);
			U16_INC(g_offset);
			break;

		case 4: // b3
			k_base				= PX_R_U8(&APDUBuffer[g_offset]);
			U16_INC(g_offset);
			classCompAddress	= MEMtPN8(class_component_Addresses);
			switch (m_count)
			{
			case CONSTANT_CLASSREF: // tag 01 CONSTANT_CLASSREF
				if (isU8External(m_base))
				{
					m_base	= f_pkgIDs[U8_AND_U8(m_base, MASK_HIGH_BIT_OFF)]; // f_pkgIDs in the INMPORT COMPONENT
					m_s1	= getReference(MEMtPN8(getPackageComponentLocation(m_base, COMPONENT_EXPORT)), k_count, 0, FLAG_CLASS);
				}
				else
				{
					m_s1	= makeShort(U8t16(m_base), k_count);
				}
				break;
			case CONSTANT_INSTANCEFIELDREF: // tag 02 CONSTANT_INSTANCEFIELDREF
				if (isU8External(m_base))
				{
					m_base				= f_pkgIDs[U8_AND_U8(m_base, MASK_HIGH_BIT_OFF)];
					m_s1				= getReference(MEMtPN8(getPackageComponentLocation(m_base, COMPONENT_EXPORT)), k_count, 0, FLAG_CLASS);
					//get the real address
					m_addr				= MEMtPN8(getReferenceAddress(m_s1));
					classCompAddress	= MEMtPN8(getClassComponentBaseAddr(m_addr));
				}
				else
				{
					m_addr				= PN8_A_U16(MEMtPN8(class_component_Addresses), makeShort(U8t16(m_base), k_count));
				}
				// store the token to the low byte of a short value token is the offset to a 16 bit cell
				m_superClassRef			= memReadU16(PN8_A_CONST(m_addr, OFFSET_CLASS_SUPER_REF));
				if (isU16External(m_superClassRef))
				{
					m_superClassAddr	= MEMtPN8(getReferenceAddress(m_superClassRef));
				}
				else
				{
					m_superClassAddr	= PN8_A_U16(classCompAddress, m_superClassRef);
				}
				//comError
//				m_instanceSize = (jshort) (nvmReadU8((memref)(m_superClassAddr + OFFSET_CLASS_DEC_INST_SIZE)) & 0xff);
				m_instanceSize			= U32t16(U8t16(memReadU8(PN8_A_CONST(m_superClassAddr, OFFSET_CLASS_DEC_INST_SIZE))));
				//comError
//				m_instanceSize += (jshort) (((jshort) k_base) & 0xff);
				m_instanceSize			= U32t16(U16_A_U8(m_instanceSize, k_base));
				if (m_instanceSize > INSTANCE_MAX)
				{
					setLoaderException(ERROR_INSTANCE_MAX_EXCEEDED);
					return false;
				}
				m_s1 = makeShort(0, U16t8(m_instanceSize));
				break;
			case CONSTANT_VIRTUALMETHODREF:// tag 03 CONSTANT_VIRTUALMETHODREF
				if (isU8External(m_base))
				{
					m_base				= f_pkgIDs[U8_AND_U8(m_base, MASK_HIGH_BIT_OFF)];
					m_s1				= getReference(MEMtPN8(getPackageComponentLocation(m_base, COMPONENT_EXPORT)), k_count, 0, FLAG_CLASS);
					//get the real address

					m_addr				= MEMtPN8(getReferenceAddress(m_s1));
					classCompAddress	= MEMtPN8(getClassComponentBaseAddr(m_addr));
				}
				else
				{
					m_addr				= PN8_A_U16(MEMtPN8(class_component_Addresses), makeShort(U8t16(m_base), k_count));
				}

				methodRef				= getVirtualMethodReference(m_addr, k_base, classCompAddress);

				if (isU16External(methodRef))
				{
					//get the absolute address of the virtual method
					m_addr				= MEMtPN8(getReferenceAddress(methodRef));
				}
				else
				{
					//otherwise it is an offset inside the method component
					m_addr				= PN8_A_U16(MEMtPN8(method_component_Addresses), methodRef);
				}
				//
				// store method nargs to the high byte
				// store method token to the low byte
				//
				if (U8_AND_U8(memReadU8(m_addr), ACC_EXTENDED) == ACC_EXTENDED)
			   {
				//comError
		//		m_s1 = makeShort((jbyte)nvmReadU8((memref)(m_addr + OFFSET_EXTENDED_METHOD_NARGS)),k_base);
					tempRL				= memReadU8(PN8_A_CONST(m_addr, OFFSET_EXTENDED_METHOD_NARGS));
					m_s1				= makeShort(U8t16(tempRL),k_base);
				}
				else
				{
						//comError
					// get rid of the lower 4 bit max_locals
				//	m_s1 = makeShort((jbyte) ((nvmReadU8((memref)(m_addr + OFFSET_METHOD_NARGS)) >> (jbyte)4) & 0x0F),k_base);
					tempRL				= memReadU8(PN8_A_CONST(m_addr, OFFSET_METHOD_NARGS));
					tempRL				= U8_AND_U8(U8_SHR_U8(tempRL, 4), 0x0F);
					m_s1				= makeShort(U8t16(tempRL),k_base);
				}
				break;
			case CONSTANT_SUPERMETHODREF: // tag 04 CONSTANT_SUPERMETHODREF
				if (isU8External(m_base))
				{
					m_base				= f_pkgIDs[U8_AND_U8(m_base, MASK_HIGH_BIT_OFF)];
					m_s1				= getReference(MEMtPN8(getPackageComponentLocation(m_base, COMPONENT_EXPORT)), k_count, 0, FLAG_CLASS);
					//get the real address
					m_addr				= MEMtPN8(getReferenceAddress(m_s1));
					classCompAddress	= MEMtPN8(getClassComponentBaseAddr(m_addr));
				}
				else
				{
					m_addr				= PN8_A_U16(MEMtPN8(class_component_Addresses), makeShort(U8t16(m_base), k_count));
				}
				m_superClassRef			= memReadU16(PN8_A_CONST(m_addr, OFFSET_CLASS_SUPER_REF));
				if (isU16External(m_superClassRef))
				{
					//if it is an external class then this is a reference.
					//get the absolute address from the global reference table.
					m_superClassAddr	= MEMtPN8(getReferenceAddress(m_superClassRef));
					classCompAddress	= MEMtPN8(getClassComponentBaseAddr(m_superClassAddr));
				}
				else
				{
					m_superClassAddr	= PN8_A_U16(classCompAddress, m_superClassRef);
				}
				m_s1					= getVirtualMethodReference(m_superClassAddr, k_base, classCompAddress);
				break;
			case CONSTANT_STATICFIELDREF: // tag 05 CONSTANT_STATICFIELDREF
				if (!isU8External(m_base))
				{
					m_s1				= makeShort(U8t16(k_count), k_base);
				}
				else
				{
					// converter from package token to package ID
					m_base				= f_pkgIDs[U8_AND_U8(m_base, MASK_HIGH_BIT_OFF)];
					// return the address from the given package's export component
					//comError
					//m_s1 = getReference(getPackageComponentLocation(m_base, COMPONENT_EXPORT), (jshort) (k_count & 0xFF), (jshort) (k_base & 0xFF), FLAG_FIELD);
					m_s1				= getReference(MEMtPN8(getPackageComponentLocation(m_base, COMPONENT_EXPORT)),  k_count, k_base, FLAG_FIELD);
				}
				break;
			case CONSTANT_STATICMETHODREF: // tag 06 CONSTANT_STATICMETHODREF
				if (!isU8External(m_base))
				{
					m_s1				= makeShort(U8t16(k_count), k_base);
					if (m_s1 > METHOD_COMP_LIMIT)
					{
						methodAddress	= PN8_A_U16(MEMtPN8(method_component_Addresses), m_s1);
						//using offset here because that is what is used in the switch statement
						m_s1			= addReference(methodAddress, PN_R_U8(PM_g_pkgIdInProcess_JC));
						if (m_s1 == 0)
						{
							setLoaderException(ERROR_EXCEPTION);
							return false;
						}
					}
				}
				else
				{
					// converter from package token to package ID
					m_base				= f_pkgIDs[U8_AND_U8(m_base, MASK_HIGH_BIT_OFF)];
					// return the address from the given package's export component
					//comError
					//m_s1 = getReference(getPackageComponentLocation(m_base, COMPONENT_EXPORT), (jshort) (k_count & 0xFF), (jshort) (k_base & 0xFF), FLAG_METHOD);
					m_s1				= getReference(MEMtPN8(getPackageComponentLocation(m_base, COMPONENT_EXPORT)), k_count, k_base, FLAG_METHOD);
				}
				break;
			default:
				setLoaderException(ERROR_COMP_TAG);
				return false;
			}
			bTempArr[bTempOff]			= U16t8(U16_SHR_U8(m_s1, 8));
			U16_INC(bTempOff);
			bTempArr[bTempOff]			= U16t8(m_s1);
			U16_INC(bTempOff);
			break;
		default:
			break;
		}

		U8_INC(f_currentState);
		if (f_currentState > 4)
			f_currentState = 1;
	}
	if (bTempOff > 0)
	{
#if CONSTANT_POOL_COMPONENT_IN_RAM        // modify by xuxiaojun@20220606 用开关控制CONSTANTPOOL_COMPONENT是在RAM里还是EEP里
		if(constantpool_component_Addresses == PXtMEM(CONSTANTPOOL_COMPONENT))
		{
			memCopyToRam(PX8_A_U16(MEMtPX8(constantpool_component_Addresses), constantpool_componentoffset), bTempArr, bTempOff); // 往常量池地址里写数据
		}
		else
#endif        
		{
			nvmCopyNoAtomic(PN8_A_U16(MEMtPN8(constantpool_component_Addresses), constantpool_componentoffset), bTempArr, bTempOff); // 往常量池地址里写数据
		}
		constantpool_componentoffset	= U32t16(U16_A_U16(constantpool_componentoffset, bTempOff));
	}
	return true;
}

boolean ConstantComponentPostProcess()
{
//	g_loadComplete[CAP_COMPONENT_CONSTANTPOOL] = true;
	g_loadComplete						= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_CONSTANTPOOL));
	return true;
}

u16 getVirtualMethodReference(PF8 classAddr, u8 methodToken, PF8 classCompAddress) REENTRANT
{
	u16			superRef;
	boolean		m_goSuper;
	u16			m_pubMethodCount;
	u16			m_offset;
	u8			m_base_method;
	boolean		isExternalMehtod;
	PF8			superAddress;

	m_goSuper					= false;
	isExternalMehtod			= false;

	isExternalMehtod			= isHighbitOn(methodToken);
	if (isExternalMehtod == true)
	{
		m_goSuper				= memReadU8(PF8_A_CONST(classAddr, OFFSET_CLASS_PKG_METH_BASE)) > U8_AND_U8(methodToken, MASK_HIGH_BIT_OFF);
	}
	else
	{
		m_base_method			= memReadU8(PF8_A_CONST(classAddr, OFFSET_CLASS_PUB_METH_BASE));
		m_pubMethodCount		= U8t16(memReadU8(PF8_A_CONST(classAddr, OFFSET_CLASS_PUB_METH_COUNT)));
		//m_goSuper = ((u8) nvmReadU8((u8 *)(classAddr + OFFSET_CLASS_PUB_METH_BASE)) > methodToken);
		if((m_base_method > methodToken) || (U16t8(U8_A_U16(m_base_method, m_pubMethodCount)) <= methodToken))
		{
			m_goSuper			= 1;
		}
	}


	// method defined in the super class?
	if (m_goSuper)
	{
		superRef				= memReadU16(PF8_A_CONST(classAddr, OFFSET_CLASS_SUPER_REF));
		if (isU16External(superRef))
		{
			//just in case it is an external class, what we have is a reference
			//in the global reference table and not an address
			superAddress		= MEMtPF8(getReferenceAddress(superRef));
			classCompAddress	= MEMtPF8(getClassComponentBaseAddr(superAddress));
		}
		else
		{
			superAddress		= PF8_A_U16(classCompAddress, superRef);
		}
		return getVirtualMethodReference(superAddress, methodToken, classCompAddress);
	}
	else
	{
		//comError
//		m_pubMethodCount = (jshort) (nvmReadU8((memref)(classAddr + OFFSET_CLASS_PUB_METH_COUNT)) & 0xff);
		m_pubMethodCount		= U8t16(memReadU8(PF8_A_CONST(classAddr, OFFSET_CLASS_PUB_METH_COUNT)));
		m_offset				= OFFSET_CLASS_PUB_METH_TABLE;
		// package method token has the highbit on
		if (isExternalMehtod == true)
		{
			m_offset			= U32t16(U16_A_U16(m_offset, U32t16(U8_M_U16(U2_SIZE, m_pubMethodCount))));
			methodToken			= U8_AND_U8(methodToken, MASK_HIGH_BIT_OFF);
			m_base_method		= memReadU8(PF8_A_CONST(classAddr, OFFSET_CLASS_PKG_METH_BASE));
		}
		else
		{
			m_base_method		= memReadU8(PF8_A_CONST(classAddr, OFFSET_CLASS_PUB_METH_BASE));
		}
		/*
		 * "The value of an index into this table must be equal
		 *  to the value of the virtual/package method token of
		 *  the indicated method, minus the value of the
		 *  public/package_method_table_base item."  -JCVM spec.
		 */
		m_offset				= U32t16(U16_A_U16(m_offset, U8_M_U8(U8_S_U8(methodToken, m_base_method), U2_SIZE)));
		return memReadU16(PF8_A_U16(classAddr, m_offset));

	}
}

/**
 * resolve the reference and write the address
 */
u16 resolve(PF8 address, u16 offset)		SMALL_MODE
{
	u16		I_A		m_addr;
	u16		I_A		m_s1;
	u16		I_A		b2;
	u8		I_A		b1;

	m_addr		= 0;
	m_s1		= memReadU16(PF8_A_U16(address, offset));
	//if the address is internal, we return the same address
	//and do not need to do anything with it. Otherwise, we get
	//the reference back from the global reference table and that
	//is stored.
	if (isU16External(m_s1))
	{
		b1		= U16t8(U16_SHR_U8(m_s1, 8));
		b1		= f_pkgIDs[U8_AND_U8(b1, MASK_HIGH_BIT_OFF)]; // f_pkgIDs in the INMPORT COMPONENT
		//comError
		//b2 = (jshort) (m_s1 & 0xff);
		b2		= U16_AND_U16(m_s1, 0xff);
		m_addr	= getReference(MEMtPN8(getPackageComponentLocation(b1, COMPONENT_EXPORT)), b2, 0, FLAG_CLASS);
		nvmWriteU16NoAtomic(PN8_A_U16(address, offset), m_addr);
	}
	else
	{
		m_addr = m_s1;
	}
	return m_addr;
}

u8 checkOffset(u16 offset, u16 len)
{
	if(U16_S_U16(class_component_offset, offset) < len)
	{
		g_isLenEnough	= false;
		return false;
	}
	else
	{
		return true;
	}
}


u16 linkInterface(PF8 m_classCompAddr, PF8 m_curClassAddr, u16 m_offset, u8 isResolve)
{
	u8			m_b1;
	u8			m_interfaceCount;

	// read the first byte of the class_info
	m_b1 = nvmReadU8(MEMtPN8(PN8_A_CONST(m_curClassAddr ,OFFSET_CLASS_INFO)));
	// save interface_count for later use
	m_interfaceCount = U8_AND_U8(m_b1 ,	MASK_INTERFACE_COUNT);

	if (isInterface(m_b1))
	{
		for (; g_interfaceCount < m_interfaceCount; U8_INC(g_interfaceCount))
		{
			if(false == checkOffset(m_offset, 2))
			{
				return m_offset;
			}

			// link superinterfaces
			if(true == isResolve)
			{
				resolve(m_classCompAddr, m_offset);
			}

			m_offset = U32t16(U16_A_U8(m_offset, 2));
		}
		
		if (isRemote(m_b1))
		{
			throw_error(SYSTEMEXCEPT_RMI_RESOURCE);
			
			/*m_tempOffset	= m_offset;
			m_offset = fixupRemoteInfo(m_offset, m_b1, m_classCompAddr);
			if(m_tempOffset == m_offset)
			{
				return m_offset;
			}

			f_currentState			= CLASS_LINK_INTERFACE_REMOTE;*/
		}
		else
		{
			f_currentState			= CLASS_LINK_INIT;
		}
	}
	else
	{
		f_currentState			= CLASS_LINK_FIELD;
	}

	return m_offset;
}

void linkClassFiled(PF8 m_curClassAddr, u8 sectionResolve)
{
	u16		m_s1;
	u16		m_superClassRef;

	if(isU8External(nvmReadU8(PF8_A_CONST(MEMtPF8(g_CurClassAddr), OFFSET_CLASS_SUPER_REF))))
	{
		if(true == sectionResolve)
		{
			g_isResolvPublic	= true;
		}
		else
		{
			g_isResolvPublic	= false;
			return;
		}
	}
	else
	{
		if(true == sectionResolve)
		{
			g_isResolvPublic	= true/*false*/;
			//return;
		}
		else
		{
			g_isResolvPublic	= false/*true*/;
			return;
		}
	}
	// link super class address
	//change required: Check internal ... external
	m_superClassRef = resolve(MEMtPF8(m_curClassAddr), OFFSET_CLASS_SUPER_REF);

	if (isU16External(m_superClassRef))
	{
		//get the real address
		g_superClassAddr		= getReferenceAddress(m_superClassRef);
		g_superclassCompAddr	= getClassComponentBaseAddr(MEMtPF8(g_superClassAddr));
	}
	else
	{
		g_superClassAddr		= U16_A_U32(m_superClassRef, class_component_Addresses);
		g_superclassCompAddr	= class_component_Addresses;
	}
	/*
	 * update the first reference index according to instance size
	 * of the super class
	 */
	m_s1 = U8t16(U8_A_U8(nvmReadU8(MEMtPN8(U32_A_U8(g_superClassAddr, OFFSET_CLASS_DEC_INST_SIZE))),	nvmReadU8(MEMtPN8(PF8_A_CONST(m_curClassAddr, OFFSET_CLASS_REF_INDEX)))));
	nvmWriteU8Atomic(MEMtPN8(PF8_A_U8(m_curClassAddr, OFFSET_CLASS_REF_INDEX)),  U16t8(m_s1));
	/*
	 * link instance size
	 * assuming internal super class is seen
	 *
	 * add super class's instance size to current class's declared
	 * instance size
	 */

	m_s1 = U8t16(U8_A_U8(nvmReadU8(MEMtPN8(U32_A_U8(g_superClassAddr, OFFSET_CLASS_DEC_INST_SIZE))),	nvmReadU8(MEMtPN8(PF8_A_CONST(m_curClassAddr, OFFSET_CLASS_DEC_INST_SIZE)))));

	// m_s1 is the calculated instance size
	if (m_s1 > INSTANCE_MAX)
	{
		setLoaderException(ERROR_INSTANCE_MAX_EXCEEDED);
		return;
	}
	nvmWriteU8Atomic(MEMtPN8(PF8_A_U8(m_curClassAddr, OFFSET_CLASS_DEC_INST_SIZE)),  U16t8(m_s1));
	return;
}
/*
 * modifed by xxj@20210328:每次缓存一页之后再实写
 */
u16 linkPublicVirtrual(PF8 m_superClassAddr, PF8 m_superclassCompAddr, PF8 m_classCompAddr, u16 m_offset, u8 isResolve)
{
	u16		m_s1;
	PM8		methodAddress;
	u16		m_offsetbak;
	u8		X_A		tmp[0x100];

	m_offsetbak = m_offset;
	// link public virtual method table
	for (; g_publicMehtodCount < m_count; U8_INC(g_publicMehtodCount))
	{
		if(false == checkOffset(m_offset, 2))
		{
			break;
		}

		if(true == isResolve)
		{
			m_s1 = nvmReadU16(MEMtPN8(PF8_A_U16(m_classCompAddr ,m_offset)));
			// special case: imported method
			if (m_s1 == 0xffff)
			{
				/*
				 * copy the external virtual method reference
				 *
				 * i is both the index and the methodToken
				 */
				m_s1 = getVirtualMethodReference(m_superClassAddr, U8_A_U8(g_publicMehtodCount, m_base), m_superclassCompAddr);
				// write it (back) to the public virtual method table
			}
			else
			{
				/*
				 * we need to go to the global reference table and add the method to
				 * the global reference table because this value will be used to link
				 * classes extending from this class.
				 */
				// calculate the physical address: m_s1 is then method offset
				methodAddress = MEMtPM8(U32_A_U16(method_component_Addresses ,U16_AND_U16(m_s1, 0xffff)));

				//add to the global reference table and get the reference back.
				/*@ -disspecopt @*/m_s1 = addReference(methodAddress, nvmReadU8(MEMtPN8(PM_g_pkgIdInProcess_JC)));/*@ +disspecopt @*/

				if (m_s1 == 0)
				{
					// 如果前面有m_s1需要写的话，这里抛异常，前面的写是否需要实写呢？
					throw_error(SYSTEMEXCEPT_NO_RESOURCE);
				}
			}
			PXB_W_U16(&tmp[m_offset - m_offsetbak],m_s1);
		}
		m_offset = U32t16(U16_A_U8(m_offset, 2));
	}
	if( (true == isResolve) && (m_offsetbak != m_offset) )
	{
		nvmCopyAtomic(MEMtPN8(PM8_A_U16(m_classCompAddr, m_offsetbak)), tmp, m_offset - m_offsetbak);
	}
	if(g_publicMehtodCount == m_count)
	{
		f_currentState		= CLASS_LINK_PACKAGE_VIRTUAL_INIT;
	}
	return m_offset;
}
/*
 * modifed by xxj@20210328:每次缓存一页之后再实写
 */
u16 linkPackageVirtual(PF8 m_classCompAddr, u16 m_offset, u8 isResolve)
{
	u16			m_s1;
	PM8			methodAddress;
	u16		m_offsetbak;
	u8		X_A		tmp[EEPAGE_SIZE];

	m_offsetbak = m_offset;

	for (; g_packageMethodCount < m_count; U8_INC(g_packageMethodCount))
	{
		if(false == checkOffset(m_offset, 2))
		{
			break;
		}
		if(true == isResolve)
		{
			m_s1 = nvmReadU16(PF8_A_U16(m_classCompAddr, m_offset));
			if (U16_AND_U16(m_s1, 0xffff) > METHOD_COMP_LIMIT)
			{
				//get the absolute address
				methodAddress = MEMtPM8(U32_A_U16(method_component_Addresses, U16_AND_U16(m_s1, 0xffff)));
				//add to the global reference table
				/*@ -disspecopt @*/m_s1 = addReference(methodAddress, nvmReadU8(MEMtPN8(PM_g_pkgIdInProcess_JC)));/*@ +disspecopt @*/
				if (m_s1 == 0)
				{
					setLoaderException(ERROR_EXCEPTION);
					return false;
				}
			}
			//put the reference in the class structure
			PXB_W_U16(&tmp[m_offset-m_offsetbak],m_s1);
		}
		m_offset = U32t16(U16_A_U8(m_offset, 2));
	}

	if((true == isResolve) && (m_offset != m_offsetbak))
	{
		nvmCopyAtomic(PM8_A_U16(m_classCompAddr, m_offsetbak), tmp, m_offset - m_offsetbak);
	}
	if(g_packageMethodCount == m_count)
	{
		f_currentState		= CLASS_LINK_IMP_INTERFACE_INIT;
	}
	return m_offset;
}

u16 linkImplementInterface(PF8 m_classCompAddr, u16 m_offset, u8 isResolve)
{
	u16		l_temp;

	for (; g_implementInterCount < m_count; U8_INC(g_implementInterCount))
	{
		if(false == checkOffset(m_offset, OFFSET_CLASS_INTERFACE_INFO_COUNT))
		{
			return m_offset;
		}

		l_temp =  U8t16(nvmReadU8(PF8_A_U16(m_classCompAddr, U16_A_U16(m_offset, OFFSET_CLASS_INTERFACE_INFO_COUNT))));

		if(false == checkOffset((m_offset), U16_A_U8(l_temp, 3)))
		{
			return m_offset;
		}

		if(true == isResolve)
		{
			resolve(m_classCompAddr,  U16_A_U16(m_offset, OFFSET_CLASS_INTERFACE_INFO_CLASS_REF));
		}

		// m_b1 is the index count
		m_offset = U32t16(U16_A_U16(m_offset, U16_A_U8 (l_temp, 3)));
	}

	f_currentState		= CLASS_LINK_REMOTE_INFO_INIT;
	
	return m_offset;
}

#if JCVM__VM__RMI
u16 fixupRemoteInfo(u16 offset, u8 classFlag, PF8 classCompAddr)	SMALL_MODE
{
	u16 D_A	 remoteMethodCount, i, classNameSize, tempLength, oriOffset;
	
	oriOffset		= offset;
	
	if (isInterface(classFlag))
	{
		//if the interface is remote, it must have name data with it.
		//read the name length and advance the offset by name length size
		//and the one byte that is required to hold the name length.
		//comError
		tempLength = U8_A_U8(memReadU8(PF8_A_U16(classCompAddr, offset)), 1);

		if(false == checkOffset(offset, tempLength))
		{
			return offset;
		}
		offset	= U16_A_U8(offset, tempLength);
		return offset;
	}
	else
	{
		//get remote method count
		remoteMethodCount	= U8t16(memReadU8(PF8_A_U16(classCompAddr, offset)));
		if(false == checkOffset(offset, 1))
		{
			return oriOffset;
		}
		U16_INC(offset);
		for (i = 0; i < remoteMethodCount; U16_INC(i))
		{
			//advance the offset to the size of remote method information
			if(false == checkOffset(offset, REMOTE_METHOD_INFO_SIZE))
			{
				return oriOffset;
			}
			offset	= U8_A_U16(REMOTE_METHOD_INFO_SIZE, offset);
		}
		//if the class is remote, it may have anti-hash collision string in it.
		//read that and add it's length and length data (one byte) to the offset
		offset			= U8_A_U16(U8_A_U8(memReadU8(PF8_A_U16(classCompAddr, offset)), 1), offset);
		if(false == checkOffset(offset, tempLength))
		{
			return oriOffset;
		}
		offset = U16_A_U8(offset, tempLength);
		//Remote class structure also has name in it, we have to advance the offset by amount of
		//of bytes a name takes
		classNameSize	= U8t16(memReadU8(PF8_A_U16(classCompAddr, offset)));
		//advance the offset by class name size and it's size byte
		tempLength	= U16_A_U8(classNameSize, 1);
		
		if(false == checkOffset(offset, tempLength))
		{
			return oriOffset;
		}
		offset = U16_A_U8(offset, tempLength);
		//get the implemented remote interface count
		g_remoteInterfaceCount		= 0;
		g_maxRemoteInterface			= memReadU8(PF8_A_U16(classCompAddr, offset));
		if(false == checkOffset(offset, 1))
		{
			return oriOffset;
		}
		U16_INC(offset);
		
		return offset;
	}
}


u16 linkRemoteInfo(PF8 m_classCompAddr, u16 m_offset, u8 isResolve)
{
	for (; g_remoteInterfaceCount < g_maxRemoteInterface; g_remoteInterfaceCount++)
	{
		if(false == checkOffset(m_offset, CLASS_REF_SIZE))
		{
			return m_offset;
		}
		if(true == isResolve)
		{
			resolve(m_classCompAddr, U16_A_U16(m_offset, OFFSET_CLASS_INTERFACE_INFO_CLASS_REF));
		}
		m_offset = U16_A_U8(m_offset, CLASS_REF_SIZE);
	}

	f_currentState	= CLASS_LINK_INIT;

	return m_offset;
}
#endif


void linkClassComponent(u8 sectionResolve)
{
	u8  	m_b1;
	u16		m_s1;

	switch(f_currentState)
	{
		case CLASS_LINK_START:
			if (g_capMinor == 2)
			{
				//2.2 class component may have signature pool in them.
				if(false == checkOffset(g_ClassOffset, 2))
				{
					return;
				}
				m_s1 			= nvmReadU16(MEMtPN8(class_component_Addresses));

				if(false == checkOffset(g_ClassOffset, U16_A_U8(m_s1, 2)))
				{
					return;
				}
				g_ClassOffset	= U32t16(U16_A_U16(g_ClassOffset, U32t16(U16_A_U8(m_s1, 2)))); //two byte added for size data
			}
			f_currentState		= CLASS_LINK_INIT;
			break;
		case CLASS_LINK_INIT:
			g_CurClassAddr		= U32_A_U16(class_component_Addresses, g_ClassOffset);
			f_currentState		= CLASS_LINK_INTERFACE_INIT;
			break;
		case CLASS_LINK_INTERFACE_INIT:
			m_b1				= nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_INFO)));
			if (isInterface(m_b1))
			{
				if(false == checkOffset(g_ClassOffset, 1))
				{
					return;
				}
				U16_INC(g_ClassOffset);
			}
			g_interfaceCount	= 0;
			f_currentState		= CLASS_LINK_INTERFACE;
			break;
		case CLASS_LINK_INTERFACE:
			g_ClassOffset		= linkInterface(MEMtPF8(class_component_Addresses), MEMtPF8(g_CurClassAddr), g_ClassOffset, sectionResolve);
			break;

		case CLASS_LINK_INTERFACE_REMOTE:
			throw_error(SYSTEMEXCEPT_RMI_RESOURCE);
			//g_ClassOffset		= linkRemoteInfo(MEMtPF8(class_component_Addresses), g_ClassOffset, sectionResolve);
			break;
		
		case CLASS_LINK_FIELD:
			if(false == checkOffset(g_ClassOffset, OFFSET_CLASS_PUB_METH_TABLE))
			{
				return;
			}

			linkClassFiled(MEMtPF8(g_CurClassAddr), sectionResolve);
			f_currentState		= CLASS_LINK_PUBLIC_VIRTUAL_INIT;
			break;
		case CLASS_LINK_PUBLIC_VIRTUAL_INIT:
			m_base 				= nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_PUB_METH_BASE)));
			m_count 			= nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_PUB_METH_COUNT)));
			g_ClassOffset 		= U32t16(U16_A_U8(g_ClassOffset, OFFSET_CLASS_PUB_METH_TABLE));
			g_publicMehtodCount	= 0;
			f_currentState		= CLASS_LINK_PUBLIC_VIRTUAL;
			break;
		case CLASS_LINK_PUBLIC_VIRTUAL:
			g_ClassOffset		= linkPublicVirtrual(MEMtPF8(g_superClassAddr), MEMtPF8(g_superclassCompAddr), MEMtPF8(class_component_Addresses), g_ClassOffset, g_isResolvPublic);
			break;
		case CLASS_LINK_PACKAGE_VIRTUAL_INIT:
			// link package virtual method table
			m_base = nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_PKG_METH_BASE)));
			// get package virtual method count
			m_count = nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_PKG_METH_COUNT)));
			if (U16_A_U16(U8t16(m_base) ,U8t16(m_count)) > PACKAGE_METHOD_MAX)
			{
				setLoaderException(ERROR_ON_CARD_PKG_MAX_EXCEEDED);
			}
			g_packageMethodCount	= 0;
			f_currentState			= CLASS_LINK_PACKAGE_VIRTUAL;
			break;
		case CLASS_LINK_PACKAGE_VIRTUAL:
			g_ClassOffset		= linkPackageVirtual(MEMtPF8(class_component_Addresses), g_ClassOffset, sectionResolve);
			break;
		case CLASS_LINK_IMP_INTERFACE_INIT:
			m_count 			= U8_AND_U8(nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_INFO))), MASK_INTERFACE_COUNT);
			g_implementInterCount	= 0;
			f_currentState		= CLASS_LINK_IMP_INTERFACE;
			break;
		case CLASS_LINK_IMP_INTERFACE:
			g_ClassOffset		= linkImplementInterface(MEMtPF8(class_component_Addresses), g_ClassOffset, sectionResolve);
			break;

		case CLASS_LINK_REMOTE_INFO_INIT:
			if (isRemote(nvmReadU8(MEMtPN8(U32_A_U8(g_CurClassAddr, OFFSET_CLASS_INFO)))))
			{
				throw_error(SYSTEMEXCEPT_RMI_RESOURCE);
				/*tempOffset		= g_ClassOffset;
				g_ClassOffset	= fixupRemoteInfo(g_ClassOffset, m_b1, MEMtPF8(class_component_Addresses));
				if(tempOffset == g_ClassOffset)
				{
					return;
				}
				else
				{
					f_currentState	= CLASS_LINK_REMOTE_INFO;
				}*/
			}
			else
			{
				f_currentState	= CLASS_LINK_INIT;
			}
			break;
		case CLASS_LINK_REMOTE_INFO:
			throw_error(SYSTEMEXCEPT_RMI_RESOURCE);
			//g_ClassOffset		= linkRemoteInfo(MEMtPF8(class_component_Addresses), g_ClassOffset, sectionResolve);
			break;
		
	}
}


void ClassComponentInit(void)
{
	g_isResolvPublic		= true;
	g_ClassOffset			= 0;
	g_CurClassAddr			= class_component_Addresses;
	f_currentState			= CLASS_LINK_START;
}

boolean ClassComponentProcess()
{
	g_isLenEnough		= true;
	nvmCopyNoAtomic(PN8_A_U16(MEMtPN8(class_component_Addresses), class_component_offset), PX8_A_U16(g_buffer, g_offset), U16_S_U16(g_max, g_offset));
	class_component_offset	= U32t16(U16_A_U16(class_component_offset, U16_S_U16(g_max, g_offset)));

	while((g_ClassOffset < class_component_offset) && (true == g_isLenEnough))
	{
		linkClassComponent(true);
	}

	return true;

}

boolean ClassComponentpostProcess()
{
	ClassComponentInit();

	while(g_ClassOffset < class_component_Size)
	{
		linkClassComponent(false);
	} // while

	// Done!
	g_loadComplete	= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_CLASS));
	return true;
}

/*****************************************************************************
 //method component
 *****************************************************************************/
boolean MethodComponentProcess()
{
	nvmCopyNoAtomic(PN8_A_U16(MEMtPN8(method_component_Addresses), method_component_offset), PX8_A_U16(g_buffer, g_offset), U16_S_U16(g_max, g_offset));
	memRefresh();
	method_component_offset	= U32t16(U16_A_U16(method_component_offset, U16_S_U16(g_max, g_offset)));
	return true;
}

boolean MethodComponentpostProcess()
{
	u16			D_A		m_offset;
	u16         D_A     i;
	u16			D_A     catchTypeIndex;
	PN8			D_A		m_compAddress;
	u16			D_A		m_handlerCount;

	m_offset		= 0; 
	i				= 0;
	m_compAddress	= MEMtPN8(method_component_Addresses);
	//
	// link exception handlers
	//
	//comError
//	m_handlerCount = (u16) (nvmReadU8(m_compAddress) & 0xff);
	m_handlerCount	= U8t16(nvmReadU8(m_compAddress));
	U16_INC(m_offset);
	//to take care of the finally block... we go through the whole exception handler info
	//to find out where there are any catch_type_indexes which are 0 (0 represents finally)
	//if we do find a zero we change it to FFFF since 0 now is a valid index inside the class component
	// of the CAP file.
	//Since the 0 in the Method component's exception_handler_info is not an index in
	//the constant pool this change will not affect any other section of the installer.
	//The only change required will be inside cref where it will have to recognize FFFF as the
	//"finally block" not 0.
	m_offset	= U32t16(U16_A_U8(m_offset, OFFSET_EXCEPTION_HANDLER_CATCH_TYPE_INDEX));
	if (m_handlerCount > 0)
	{
		for (i = 0; i < m_handlerCount; U16_INC(i))
		{
			catchTypeIndex	= nvmReadU16(PN8_A_U16(m_compAddress, m_offset));
			if (catchTypeIndex == 0)
			{
				nvmWriteU16NoAtomic(PN8_A_U16(m_compAddress, m_offset), 0xFFFF);
			}
			//following statement would bring it to the next value of catch_type_index
			m_offset	= U32t16(U16_A_U8(m_offset, EXCEPTION_HANDLER_INFO_SIZE));
		}
		//beginTransaction();
		/**
		 * addExcTable needs to do the following now:
		 * 1. Allocate space for new entry in the exception table linked list based on the 16/32 bit
		 * implementation.
		 * 2. To the newly allocated entry add the address to the method component of the current
		 * package.
		 * 3. Finally add the exception table to already existing linked list of exception tables.
		 * The parameter needed to complete the above 3 tasks is just pkgid. At native level we
		 * already know the address of the method component for this package, since we are doing
		 * allocation at native level we will know the size required there as well
		 */
		addexceptiontable(PN_R_U8(PM_g_pkgIdInProcess_JC));
		//commitTransaction();
	}
//	g_loadComplete[CAP_COMPONENT_METHOD] = true;
	g_loadComplete	= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_METHOD));
	return true;
}
/*****************************************************************************
 StaticFieldComponent  method
 *****************************************************************************/
/*#define BYTE_AS_TRUE 0x01		   */

void StaticFieldComponentInit()
{
	f_currentState			= SC_STATE_READY;
	g_staticReferenceCount	= 0;
	f_ndOffset				= 0;
	f_sc_offset				= 0;
	g_more					= true;
}
/*
 * modified by xuxiaojun@20210414 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 */
boolean StaticFieldComponentprocess()
{
	u8			D_A		this_state, pkgContext;
	u8			D_A		temp;
	u16			D_A		count;
	u16			D_A		copyLen;
	PN8			D_A		newPackageAddr;

	this_state	= SC_STATE_READY;
	temp		= 0;
	pkgContext	= U16t8(getPkgContext(PN_R_U8(PM_g_pkgIdInProcess_JC)));

	// hasmore
	while (g_more && g_offset < g_max)
	{
		switch (f_currentState)
		{
		case SC_STATE_READY:
		case SC_STATE_IMAGE_SIZE:
		case SC_STATE_REF_COUNT:
		case SC_STATE_AI_COUNT:
		case SC_STATE_DEFAULT_COUNT:
		case SC_STATE_AI_INFO_TYPE:
		case SC_STATE_AI_INFO_COUNT:
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				this_state		= f_currentState;
				f_currentState	= SC_STATE_NEXT; // do not update state
				break;
			}
			U8_INC(f_currentState);
			if ((f_currentState == SC_STATE_AI_INFO_TYPE) && (U16_AND_U16(f_aiCount, 0x8000) == 0x8000 || (f_aiCount == 0)))
			{
				f_currentState	= SC_STATE_DEFAULT_COUNT;
			}
			break;
		case SC_STATE_AI_INFO_VALUES:
			// more array_init_info.values?
			if (f_aiByteCount != 0)
			{
				break; // remain in the same state
			}


			// more array_init_info?
			if (f_aiCount > 0)
			{
				f_currentState = SC_STATE_AI_INFO_TYPE; // repeat AI
			}
			else
			{
				// a short is split in two APDUs?
				if (isSplitValue(U2_SIZE))
				{
					saveBytes(g_offset, U16_S_U16(g_max, g_offset));
					this_state			= f_currentState;
					f_currentState		= SC_STATE_NEXT; // do not update state
					break;
				}
				U8_INC(f_currentState);
			}
			break;
		case SC_STATE_ND_VALUES:
			// more non_default_values?
			if (f_ndOffset >= f_ndByteCount)
			{
				U8_INC(f_currentState); // go to the next state
			} // else remain in the same state
			break;
		case SC_STATE_ND_COUNT:
			// next is ND_VALUES u1 or DONE u0
			if (f_ndByteCount == 0)
			{
				f_currentState	= SC_STATE_DONE;
				break;
			}
			/* no break */
		default:
			U8_INC(f_currentState); // go to the next state
			break;
		}//end first switch

		switch (f_currentState)
		{
		case SC_STATE_IMAGE_SIZE:
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			break;
		case SC_STATE_REF_COUNT:
			g_staticReferenceCount = getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			break;
		case SC_STATE_AI_COUNT:
			// get array_init_count
			f_aiCount	= getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			f_seg2Count = U16_S_U16(g_staticReferenceCount, f_aiCount);
			break;
		case SC_STATE_AI_INFO_TYPE:
			U16_DEC(f_aiCount);
			// get array_init_info type
			f_aiType 	= g_buffer[g_offset];
			U16_INC(g_offset);
			// be ready for a new array
			f_aiOffset	= 0;
			break;
		case SC_STATE_AI_INFO_COUNT:
			// get array_init_info (byte) count
			f_aiByteCount		= getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			f_aiElementCount	= f_aiByteCount;
			// create an array object

			cc	= U16_SHL_U8(U8t16(pkgContext), 8);
			if(0xFF == pkgContext)
			{
				cc	= U16_OR_U16(cc, PN_R_U8(PM_g_pkgIdInProcess_JC));
			}
			g_isResolveStaticField	= 1; 

			switch (f_aiType)
			{
			case TYPE_BOOLEAN:
				f_objectArray = _newarrayObject(AType_Boolean, f_aiElementCount, BIT_NEED_NOTHING);
				break;
			case TYPE_BYTE:
				f_objectArray = _newarrayObject(AType_Byte, f_aiElementCount, BIT_NEED_NOTHING);
				break;
			case TYPE_SHORT:
				/*+
				 * calculate the array element count by dividing
				 * the count by the data type size
				 -*/
				f_aiElementCount	= U16_D_U8(f_aiByteCount, U2_SIZE);
				f_objectArray = _newarrayObject(AType_Short, f_aiElementCount, BIT_NEED_NOTHING);
				break;
			case TYPE_INT:
				/*
				 * calculate the array element count by dividing
				 * the count by the data type size
				 */
				f_aiElementCount	= U16_D_U8(f_aiByteCount, U4_SIZE);
			#ifdef 	INTEGER_MODE
					f_objectArray = _newarrayObject(AType_Int, f_aiElementCount, BIT_NEED_NOTHING);
			#else
					setLoaderException(ERROR_INTEGER_UNSUPPORTED);
			#endif
				break;

			default:
				break;
			}
			
			g_isResolveStaticField	= 0;
			cc						= JCRE_CONTEXT;

			f_objectArrayAddr = locateObjectInMemory(f_objectArray);
			break;
		case SC_STATE_AI_INFO_VALUES:
			// initialize an element in the newly created array object
			if (f_aiByteCount != 0)
			{
				if (U16_A_U16(g_offset, f_aiByteCount) < g_max)
				{
					copyLen	= f_aiByteCount;
				}
				else
				{
					copyLen = U16_S_U16(g_max, g_offset);
				}
				nvmCopyNoAtomic(PN8_A_U16(MEMtPN8(PFtMEM(PF8_A_CONST(MEMtPF8(f_objectArrayAddr), PA_DataArray_START))), f_aiOffset), MEMtPM8(&g_buffer[g_offset]), copyLen); //nvmCopyNoAtomic(PN8_A_U16(MEMtPN8(get_array_data(MEMtPF8(f_objectArrayAddr))), f_aiOffset), MEMtPM8(&g_buffer[g_offset]), copyLen);
				g_offset		= U32t16(U16_A_U16(g_offset, copyLen));
				f_aiOffset 		= U32t16(U16_A_U16(f_aiOffset, copyLen));
				f_aiByteCount 	= U16_S_U16(f_aiByteCount, copyLen);
				
			}

			if (f_aiByteCount == 0)
			{
				// store the array object reference
				if(isInE2P(staticfield_component_Addresses))
				{
					nvmWriteU16NoAtomic(PN8_A_U16(MEMtPN8(staticfield_component_Addresses), f_sc_offset), f_objectArray);
				}
				// advance the offset
				f_sc_offset	= U32t16(U16_A_U8(f_sc_offset, U2_SIZE));
			}
			break;
		case SC_STATE_DEFAULT_COUNT:
			// get default_value_count
			count		= getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			//Do this segment2 fixing-up work before segment3
			if(isInE2P(staticfield_component_Addresses))
			{
				nvmFillNoAtomic(PN8_A_U16(MEMtPN8(staticfield_component_Addresses), f_sc_offset), U16t8(NULL_REF), U32t16(U16_A_U16(U32t16(U8_M_U16(U2_SIZE, f_seg2Count)), count)));
			}
			f_sc_offset	= U32t16(U16_A_U16(f_sc_offset, U32t16(U16_A_U16(U32t16(U8_M_U16(U2_SIZE, f_seg2Count)), count))));
			// store (initialize to) zeros
			break;
		case SC_STATE_ND_COUNT:
			// get non_defalt_value_count
			f_ndByteCount	= getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			break;
		case SC_STATE_ND_VALUES:
			// store (initialize) one byte of non_default_values
			if (f_ndOffset <= f_ndByteCount)
			{
				//modified by linguoming for transplanting to the NXP platform
				temp	= g_buffer[g_offset];
				if(isInE2P(staticfield_component_Addresses))
				{
				nvmWriteU8NoAtomic(PN8_A_U16(MEMtPN8(staticfield_component_Addresses), f_sc_offset), temp);
				}
				U16_INC(f_sc_offset);
				U16_INC(g_offset);
			}
			U16_INC(f_ndOffset);
			break;
		case SC_STATE_NEXT:
			f_currentState = this_state;
			/*  no break */
		case SC_STATE_DONE:
			g_more = false;
			break;
		default:
			break;
		} //end second switch
	} // while


	// more?
	if (hasMoreAPDU())
	{
		return true; // to get another APDU
	}
	//set the static referece count in the package table entry
	/*@ -disspecopt @*/newPackageAddr = MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPN8(PM_g_newPackage_JC))));/*@ +disspecopt @*/
	nvmWriteU16NoAtomic(MEMtPN8(get_object_field_ref(newPackageAddr,pkgStaticReferenceCount_INDEX)), g_staticReferenceCount);
	/*
	 For garbage collection of pre-initialized arrays
	 */
	f_objectArray	= 0x0000;
	// done!
//	g_loadComplete[CAP_COMPONENT_STATICFIELD] = true;
	g_loadComplete = U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_STATICFIELD));
	return true;
}

/*****************************************************************************
 ExportComponent  method
 *****************************************************************************/
boolean ExportComponentProcess()
{
	u16		D_A		g_data_start;

	g_data_start	= 0;
	g_data_start	= g_offset;
	// hasmore
	while (g_more && g_offset < g_max)
	{
		switch (f_currentState)
		{
		case EC_STATE_READY:
			U8_INC(f_currentState);
			break;
		case EC_STATE_CLASS_COUNTER:
			k_count = g_buffer[g_offset];
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case EC_STATE_CLASS_OFF:
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				U16_DEC(g_max);
				g_more	= false;
				break;
			}
			if (k_count == 0)
			{
				g_more	= false;
				break;
			}
			U8_DEC(k_count);
			if (!resolveAndAddReference(PX8_A_U16(g_buffer, g_offset), MEMtPN8(class_component_Addresses)))
				return false;

			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			U8_INC(f_currentState);
			break;
		case EC_STATE_SF_COUNT:
			m_count		= g_buffer[g_offset];
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case EC_STATE_SM_COUNT:
			m_base		= g_buffer[g_offset];
			U16_INC(g_offset);
			U8_INC(f_currentState);
			break;
		case EC_STATE_SF_OFFSETS:
			if (m_count == 0)
			{
				U8_INC(f_currentState);
				break;
			}
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				U16_DEC(g_max);
				g_more	= false;
				break;
			}
			if (!resolveAndAddReference(PX8_A_U16(g_buffer, g_offset), MEMtPN8(staticfield_component_Addresses)))
				return false;

			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			U8_DEC(m_count);
			break;
		case EC_STATE_SM_OFFSETS:
			if (m_base == 0)
			{
				f_currentState = EC_STATE_CLASS_OFF;
				break;
			}
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				U16_DEC(g_max);
				g_more	= false;
				break;
			}
			if (!resolveAndAddReference(PX8_A_U16(g_buffer, g_offset), MEMtPN8(method_component_Addresses)))
				return false;

			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			U8_DEC(m_base);
			break;
		default:
			g_more = false;
			break;

		}
	}
	if(isInE2P(export_component_Addresses))
	{
		nvmCopyNoAtomic(PN8_A_U16(MEMtPN8(export_component_Addresses), export_component_offset), PX8_A_U16(g_buffer, g_data_start), U16_S_U16(g_max, g_data_start));
	}
	export_component_offset	= U32t16(U16_A_U16(export_component_offset, U16_S_U16(g_max, g_data_start)));
	return true;
}

boolean ExportComponentpostProcess()
{
	// Done!
//	g_loadComplete[CAP_COMPONENT_EXPORT] = true;
	g_loadComplete	= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_EXPORT));
    memRefresh();
	return true;
}

boolean resolveAndAddReference(PX8 addr, PN8 componentAddresses)		SMALL_MODE
{
	//read the reference
	u16		D_A		ref;
	//add the ref to the component base address and put
	//that in the global reference table to get the reference
	u16		D_A		ref2;

	ref		= getShort(addr, 0);
	ref2	= addReference(PN8_A_U16(componentAddresses, ref), PN_R_U8(MEMtPN8(PM_g_pkgIdInProcess_JC)));

	if (ref2 == 0)
	{
		setLoaderException(ERROR_EXCEPTION);
		return false;
	}
	//put that in the export component
	addr[0]	= U16t8(U16_AND_U16(U16_SHR_U8(ref2, 8), LOW_BYTE_OF_SHORT));
	addr[1]	= U16t8(U16_AND_U16(ref2, LOW_BYTE_OF_SHORT));
	return true;
}

u16 getReference(PN8 exportAddr, u16 classToken, u16 token, u8 mode)
{
	u16		I_A		m_fieldCount;
	u16		I_A		m_methodCount, i;
	u16		I_A		m_offset;
	u16				classCount;

	m_offset	= OFFSET_EXPORT_COUNT;
	// get class_count
	//comError
//	jshort classCount = (jshort) (nvmReadU8((memref)(exportAddr + m_offset)) & 0xff);  //jbyte 的0xFF  转化为jshort 是0xffff
	
	/**
	 * This is necessary because a CAP file verified by the verifier
	 * can still refer to unsupported/unpermitted external tokens
	 */
	classCount = U8t16(nvmReadU8(PN8_A_U16(exportAddr, m_offset)));

	if (classToken >= classCount)
	{
		setLoaderException(ERROR_IMPORT_CLASS_NOT_FOUND);
		return 0xFF;
	}
	U16_INC(m_offset); // export count is 1

	// advance the offset
	for (i = 0; i < classToken; U16_INC(i))
	{
		//comError
//		m_fieldCount = (jshort)(nvmReadU8((memref)(exportAddr + m_offset + OFFSET_EXPORT_INFO_STATIC_FIELD_COUNT)) & 0xff);
		m_fieldCount	= U8t16(nvmReadU8(PN8_A_U16(exportAddr, U8_A_U16(OFFSET_EXPORT_INFO_STATIC_FIELD_COUNT, m_offset))));
		//comError
//		m_methodCount = (jshort)(nvmReadU8((memref)(exportAddr + m_offset + OFFSET_EXPORT_INFO_STATIC_METHOD_COUNT)) & 0xff);
		m_methodCount	= U8t16(nvmReadU8(PN8_A_U16(exportAddr, U8_A_U16(OFFSET_EXPORT_INFO_STATIC_METHOD_COUNT, m_offset))));
		m_offset		= U32t16(U16_A_U16(m_offset, U8_A_U16(4, U8_M_U16(2, U16_A_U16(m_fieldCount, m_methodCount))))); // cell size = 2 // 2 for class address, 2 for counts
	}

	//comError
//	m_fieldCount = (jshort) (nvmReadU8((memref)(exportAddr + m_offset + OFFSET_EXPORT_INFO_STATIC_FIELD_COUNT)) & 0xff);
	m_fieldCount		= U8t16(nvmReadU8(PN8_A_U16(exportAddr, U8_A_U16(OFFSET_EXPORT_INFO_STATIC_FIELD_COUNT, m_offset))));
	//comError
//	m_methodCount = (jshort) (nvmReadU8((memref)(exportAddr + m_offset + OFFSET_EXPORT_INFO_STATIC_METHOD_COUNT)) & 0xff);
//	m_methodCount = (jshort)nvmReadU8((memref)(exportAddr + m_offset + OFFSET_EXPORT_INFO_STATIC_METHOD_COUNT)) & 0xff;
	if (mode == FLAG_FIELD)
	{
		// m_offset will be where the static field address is
		m_offset	= U32t16(U16_A_U16(m_offset, U8_A_U16(4, U8_M_U16(2, token)))); // skip class_offset & counts
		// token is the index
	}
	else if (mode == FLAG_METHOD)
	{
		// m_offset will be where the static method address is
		m_offset = U32t16(U16_A_U16(m_offset, U32t16(U8_A_U16(4, U16_A_U16(U32t16(U8_M_U16(2, m_fieldCount)), U32t16(U8_M_U16(2, token))))))); // skip class_offset & counts
		// skip field offsets
		// token is the index
	}
	return nvmReadU16(PN8_A_U16(exportAddr, m_offset));
}

/*****************************************************************************
 ReferenceLocationComponent  method
 *****************************************************************************/
#define f_byteIndexCount  class_component_offset
#define f_byte2IndexCount method_component_offset
#define f_N               export_component_offset
#define f_jumpOffset 	  constantpool_componentoffset

void ReferenceLocationComponentInit()
{
	f_jumpOffset		= 0;
	f_currentState		= RL_STATE_READY;
	f_byteIndexCount	= 0;
	f_byte2IndexCount	= 0;
	f_N					= 0;
}

boolean ReferenceLocationComponentprocess()
{
	u8			D_A		this_status;
	u16			D_A		m_s1;

	this_status			= 0;
	while (g_more && g_offset < g_max)
	{
		switch (f_currentState)
		{
		case RL_STATE_BYTE_INDEX_COUNT:
			// skip if count is zero
			// a short that follows is split in two APDUs?
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				this_status		= f_currentState;
				f_currentState	= RL_STATE_NEXT; // do not update state
				break;
			}
			f_byteIndexCount	= getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			if (f_byteIndexCount == 0)
			{
				U8_INC(f_currentState); // skip to the next state
			}
			U8_INC(f_currentState); // go to the next state
			break;

		case RL_STATE_BYTE2_INDEX_COUNT:
			// skip if count is zero
			// a short that follows is split in two APDUs?
			if (isSplitValue(U2_SIZE))
			{
				saveBytes(g_offset, U16_S_U16(g_max, g_offset));
				this_status		= f_currentState;
				f_currentState	= RL_STATE_NEXT; // do not update state
				break;
			}
			f_byteIndexCount	= getShort(g_buffer,g_offset);
			g_offset	= U32t16(U16_A_U8(g_offset, 2));
			// initialize f_jumpOffset to method block offset
			f_jumpOffset = 0;
			if (f_byteIndexCount == 0)
			{
				U8_INC(f_currentState); // skip to the next state
			}
			U8_INC(f_currentState); // go to the next state
			break;

		case RL_STATE_BYTE2_INDICES:
		case RL_STATE_BYTE_INDICES:
			m_s1				= U8t16(g_buffer[g_offset]);
			U16_INC(g_offset);
			if (m_s1 >= 0xff)
			{
				U16_INC(f_N);
				U16_DEC(f_byteIndexCount);
				break;
			}
			f_jumpOffset		= U32t16(U16_A_U16(f_jumpOffset, U32t16(U16_A_U16(U32t16(U8_M_U16(255, f_N)), m_s1))));
			f_N					= 0;

			if (f_currentState == RL_STATE_BYTE_INDICES)
			{
				// read one byte as the constant pool index
				m_s1	= U8t16(nvmReadU8(PN8_A_U16(MEMtPN8(method_component_Addresses), f_jumpOffset)));
			}
			else
			{
				// read one short as the constant pool index
				m_s1	= nvmReadU16(PN8_A_U16(MEMtPN8(method_component_Addresses), f_jumpOffset));
			}

			m_s1	= U32t16(U8_M_U16(CP_CELL_SIZE, m_s1));	/* + U2_SIZE*/ // the U2_SIZE is constant pool counter
			
			if (f_currentState == RL_STATE_BYTE_INDICES)
			{
				// byte index fixup
				nvmWriteU8NoAtomic(PN8_A_U16(MEMtPN8(method_component_Addresses), f_jumpOffset), U16t8(nvmReadU16(PN8_A_U16(MEMtPN8(constantpool_component_Addresses), m_s1)))); // 常量池里的数据往方法组建中写
			}
			else
			{
				// byte2 index fixup
				nvmWriteU16NoAtomic(PN8_A_U16(MEMtPN8(method_component_Addresses), f_jumpOffset), nvmReadU16(PN8_A_U16(MEMtPN8(constantpool_component_Addresses), m_s1)));
			}

			// special case: multiple items in offset_to_byte2_indices
			U16_DEC(f_byteIndexCount);		  //wzq: f_byteIndexCount-- -> U16_DEC
			if (f_byteIndexCount > 0)
			{
				break; // to stay in the same state
			}
			
			if(f_currentState == RL_STATE_BYTE_INDICES)
			{
				f_currentState	= RL_STATE_BYTE2_INDEX_COUNT; // go to the next state
			}
			else
			{
				U8_INC(f_currentState); // go to the next state
			}
			break;

		case RL_STATE_NEXT:
			f_currentState		= this_status;
			/* no break */
			
		case RL_STATE_DONE:
			g_more = false;
			break;
		default:
			U8_INC(f_currentState);
			break;
		} // end second switch
	} // while

	// more?
	if (hasMoreAPDU())
	{
		return true; // to get another APDU
	}
	// done!
//	g_loadComplete[CAP_COMPONENT_REFERENCELOCATION] = true;
	g_loadComplete						= U16_OR_U16(g_loadComplete, U16_SHL_U8(0x01, CAP_COMPONENT_REFERENCELOCATION));
	PXB_W_U8(&bRamBuffer[INSTALLER_STATE_INDEX], INSTALLER_STATE_READY);
	return true;
}
/*@ +namechecks @*/
