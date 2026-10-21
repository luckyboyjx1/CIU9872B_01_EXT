#include "tranManager.h"
#include "ecpDefine.h"
#include "context.h"
#include "euiccConfig.h"
#include "profile.h"
#include "JcvmConfig.h"
#include "JcvmDataConfig.h"
#include "object.h"
#include "gc.h"
#include "Io_7816T0.h"
#include "7816Util.h"
#include "7816Channel.h"
#include "SEEUICCInterface.h"
#include "NvmDeallocate.h"
#include "rtrSpace.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "profileApi.h"

#if (EUICC_ONLY||SE_AND_EUICC)
#include "EUICCCustomer.h"

u32 JcvmConfig_GetCurrentExceptionListBase(void)
{
	u8  profileindex;

	profileindex = Euicc_getCurExecProfile();
	//if(profileindex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(profileindex) == FALSE)
	{
		return U32_A_U32(VM_CONFIG_START, CURRENT_EXCEPTION_LIST);
	}
	else
	{
		return JcvmConfig_GetEUICCCurrentExecptionListBase(profileindex);
	}
}

u32 JcvmConfig_GetDtpBase(void)
{
	u8  profileindex;

	profileindex = Euicc_getCurExecProfile();

	if(ProfileIndexIsValid(profileindex) == FALSE)
	{
		return	U32_A_U32(VM_CONFIG_START, dTP);
	}
	else
	{
		return	JcvmConfig_GetEUICCDtpBase(profileindex);
	}
}

u32 JcvmConfig_GetMaxDtpBase(void)
{
	u8  profileindex;

	profileindex = Euicc_getCurExecProfile();

	if(ProfileIndexIsValid(profileindex) == FALSE)
	{
		return U32_A_U32(VM_CONFIG_START, max_dTP);
	}
	else
	{
		return	JcvmConfig_GetEUICCMaxDtpBase(profileindex);
	}
}


u32 JcvmConfig_GetRtpBase(void)
{
	u8  profileindex;

	profileindex = Euicc_getCurExecProfile();

	if(ProfileIndexIsValid(profileindex) == FALSE)
	{
		return U32_A_U32(VM_CONFIG_START, rTP);
	}
	else
	{
		return	JcvmConfig_GetEUICCRtpBase(profileindex);
	}
}


u16 tranMananger_getProfileRtp(u8 profileindex)
{
	u16   tempRtp;

	if (U8_AND_U8(g_transientCache.Flag, RTR_FLAG))
	{
		tempRtp = g_transientCache.RTR_Ptr;
	}
	else
	{
		tempRtp = nvmReadU16(MEMtPM8(JcvmConfig_GetEUICCRtpBase(profileindex)));
	}
	return tempRtp;
}

void Euicc_ConfigDataReset(u16 initialRtp)
{
	JcvmConfig_SetRtp(initialRtp);
	//RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)), tranMananger_getProfileRtp(G_CurExecutingProfileIndex), false);
	
	//if(initialRtp == 0)//创建profileEntry时 ，删PE时 都需要赋初始值，
	{
		JcvmConfig_SetMaxDtp(JcvmConfig_GetEUICCMaxDtpBack());
		memCopyAtomic(MEMtPM8(JcvmConfig_GetDtpBase()), MEMtPM8(JcvmConfig_GetEUICCDtpBackBase()), (SIZE_U16 * NUM_GROUPS));//拷贝SE的RTP
	}

	JcvmConfig_SetCurrentExceptionList(INITIAL_EXCEPTION_TABLE_LIST);
}

void Euicc_BakJCRE(void)
{
//	JcvmConfig_SetEUICCRtpBack(JcvmConfig_GetRtp());
	JcvmConfig_SetEUICCMaxDtpBack(JcvmConfig_GetMaxDtp());
	memCopyAtomic(MEMtPM8(JcvmConfig_GetEUICCDtpBackBase()), MEMtPM8(JcvmConfig_GetDtpBase()), (SIZE_U16 * NUM_GROUPS));
}


/*将某个对象Mark为有效*/
u8 markProfileObject(u16 objectId)
{
	u8		D_A		bitChecker;
	u16		D_A		arrayIndex;
	u8		D_A		bitNumber;

	bitChecker = 1;

	//if (objectId <= MASK_MAP_ENTRY_COUNT)
	if (objectId < MASK_MAP_ENTRY_COUNT)
		return true;

	objectId	= U16_S_U16(objectId, MASK_MAP_ENTRY_COUNT);
	arrayIndex	= U16_D_U8(objectId, 8);
	bitNumber	= U16t8(U16_R_U8(objectId, 8));

	if(arrayIndex > bitMapCount)
	{
		throw_error(0x6F02);
	}

	/*getting the nth power of 2*/
	bitChecker = U8_SHL_U8(bitChecker, bitNumber);
	if(!mapTableTag)
	{
		PX_W_U8(PX8_A_U16(bmp, arrayIndex), U8_OR_U8(PX_R_U8(PX8_A_U16(bmp, arrayIndex)), bitChecker));
	}
	else
	{
	//	if(arrayIndex <= U16_S_U16(PXtMEM(firstBmpEnd), PXtMEM(firstBmp)))
		if(arrayIndex <= U32t16(U32_S_U32(PXtMEM(RamManager_getAlloRamBase(firstBmpEnd)), firstBmp)))
		{
			PX_W_U8(PX8_A_U16(firstBmp, arrayIndex), U8_OR_U8(PX_R_U8(PX8_A_U16(firstBmp, arrayIndex)), bitChecker));
		}
		else
		{
			arrayIndex	= U16_S_U16(arrayIndex, U16_S_U8(U32t16(U32_S_U32(PXtMEM(RamManager_getAlloRamBase(firstBmpEnd)), firstBmp)), 1));// - 1 锟斤拷锟斤拷锟斤拷secondMap锟斤拷锟斤拷锟斤拷锟斤拷  //(lzm modified according [MISA])
			PX_W_U8(PX8_A_U16(secondBmp, arrayIndex), U8_OR_U8(PX_R_U8(PX8_A_U16(secondBmp, arrayIndex)), bitChecker));
		}
	}

	return true;

}/*end markObject*/
#if M2M_EUICC
/*将不属于delete_profile的所有对象Mark为有效，delete_profile即将被删 对象不Mark*/
u8 markProfileObjects(u8 delete_profile)
{
	jref		D_A		scanner;
	u16			D_A		TotalObjectCount;
	TotalObjectCount	= MapTable_GetTopIndex();
	delete_profile = delete_profile& 0x3F;
	do
	{
		deferredObjectCount = 0;
		deferredObjectTableOverflow = false;
		/*
		 This for loop starts from the start of the bitmap and calls markChildren on all
		 objects that have been marked already.
		 */
		for (scanner = MASK_MAP_ENTRY_COUNT; scanner < TotalObjectCount; U16_INC(scanner))
		{
			/*if(!MapTable_IsCreationObject(scanner))
			{
				continue;
			}*/
			if(!MapTable_IsCurrentProfile(scanner, delete_profile))//锟斤拷锟斤拷锟斤拷锟斤拷profile锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
			{
				markProfileObject(scanner);
			}

		}/*end for*/
		/* This loop runs exactly once in almost all cases */
	} while (deferredObjectTableOverflow);/*end do-while*/

	return true;
}/*end markNonRootObjects*/

/*释放PE占用的空间*/
//modify by xuxiaojun@20220916: 下载profile过程中有下载CAP包，此时回滚的话Profile_pkgLocationTable中的引用没有被释放，测试用例：eSIM_Test_M2M_04/eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03的第13个profile
void profilePESweep(u8 deleteProfileIndex ,jref deleteProfileEntryRef)
{
	initializeGC();

	markProfileObjects(deleteProfileIndex);//将不属于deleteProfileIndex的所有对象标记为有效

	markProfileObject(deleteProfileEntryRef);//将profileEntry mark有效
	markDeleteProfileNonRootObjects(deleteProfileIndex);//markNonRootObjects(0);

	profileSweep(deleteProfileIndex);//sweep();//释放无效对象

	resetGarbageCollector();//清变量

	NvmDeallocate_Main(false);//空间搬移
}
#endif
//modify by xuxiaojun@20220916: 下载profile过程中有下载CAP包，此时回滚的话Profile_pkgLocationTable中的引用没有被释放，测试用例：eSIM_Test_M2M_04/eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03的第13个profile
void profileSweep(u8 delete_profile)
{
	jref		D_A		entry_index;
	memref 		D_A		entry_addr;
	u16			D_A		TotalObjectCount;

	TotalObjectCount	= MapTable_GetTopIndex();

	for (entry_index = MASK_MAP_ENTRY_COUNT; entry_index < TotalObjectCount; U16_INC(entry_index))  //end condition: ran out of objects
	{
#if RSP_EUICC
		if(MapTable_IsCurrentProfile(entry_index, delete_profile))
#else
		if((MapTable_IsCurrentProfile(entry_index, delete_profile))&&(!isMarked(entry_index)))
#endif
		{
			entry_addr = MapTable_GetEntityContent(entry_index);

			if (entry_addr != MEM_NULL)
			{
				/*delete the object*/
				//removeObject(entry_index);
				NvmManager_Free(entry_index, MapTable_GetEntryPckId(entry_index));
			}/*end if*/
		}/*end if*/
	}/*end */
	NvmManager_Commit();
}/*end sweep*/

void profile_gc(u8 deleteProfileIndex,u16 entryIndex)
{
	/*mark non-root objects*/
	u8 back;

	if(MapTable_GetEntryStatus(entryIndex)==MAP_TABLE_ENTRY_STATUS_AVAIL)
	{
		return;
	}

	back = G_CurExecutingProfileIndex;

	if(G_CurExecutingProfileIndex != deleteProfileIndex)
	{
		G_CurExecutingProfileIndex = deleteProfileIndex;

		if(ExecuEnvironmentInit(G_CurExecutingProfileIndex) == FALSE)//profileEntry锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷gc
		{
			return ;
		}
	}

	initializeGC();

	//if (markProfileObjects(deleteProfileIndex))
	{
		/*Since marking was successful, start the sweep cycle.*/
		profileSweep(deleteProfileIndex);//改bitmap表状态
	}/*end if*/

	resetGarbageCollector();//清变量

	NvmDeallocate_Main(false);//空间搬移

	if(back != deleteProfileIndex)
	{
		G_CurExecutingProfileIndex = back;

		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	}
}
/*调用该函数前已经将所有不属于deleteProfileIndex的对象mark成了有效对象*/
#define MAX_GC_DEPTH 4     //值同gc_main.c
void markDeleteProfileNonRootObjects(u8 deleteProfileIndex)
{
	jref		D_A		scanner;
	u16			D_A		bitIndex;
	u16			D_A		TotalObjectCount;
	TotalObjectCount	= MapTable_GetTopIndex();

	do
	{
		initializeDeferredObjectTable();
		/*
		 This for loop starts from the start of the bitmap and calls markChildren on all
		 objects that have been marked already.
		 */
		for (scanner = MASK_MAP_ENTRY_COUNT, bitIndex = MASK_MAP_ENTRY_COUNT; bitIndex <= TotalObjectCount; U16_INC(scanner), U16_INC(bitIndex))
		{
			if(!MapTable_IsCreationObject(scanner))
			{
				continue;
			}

			//if ((bitIndex <= MASK_MAP_ENTRY_COUNT) || ((bitIndex > MASK_MAP_ENTRY_COUNT) && isMarked(bitIndex)))
			if(isMarked(bitIndex)&&(MapTable_IsCurrentProfile(scanner, deleteProfileIndex)))
			{

				if (markChildren(scanner, bitIndex, MAX_GC_DEPTH, 0) == 0)
				{
					return ;//false;
				}/*end if*/
			}/*end if*/
		}/*end for*/
		/* This loop runs exactly once in almost all cases */
	} while (deferredObjectTableOverflow);/*end do-while*/

	//return true;
}

#if RSP_EUICC
void Clearresponse(void)
{
	u8 selectchannel;
	selectchannel = U16t8(popShort());

	if(selectchannel == G_SendFileExtendBackDataStruct.currentChannel)
	{
		G_SendFileExtendBackDataStruct.totalLen =0;
	}
}
#endif

#endif

#if JAVA_CARD_UICC		//modify by jiangxiaohui@20220324 分层
static u8 b616crecflag[ISO_MAX_LOGICAL_CHANNELS]; 

// modify by xieyong@20220107 增加一卡一CRC功能

__attribute__( ( section( "LogicBakDataArea" ) ) )
#if RUN_IN_SIMULATOR
u8  LogicBakData[LOGICCHNBAK_SIZE]  = {
	0x00,	
};
#else
u8  CONST LogicBakData[LOGICCHNBAK_SIZE]  = {
	0x00,
};
#endif

PM8 bakLogicalChninfoaddr(u8 bakLogicalChn , u8 type)
{
	memref rtn;

	rtn = U32_A_U32(LOGICCHNBAK_BASEADDR,U8_M_U32(bakLogicalChn,LOGICCHNBAK_BLOCKSIZE));

	switch (type)
	{
		case BAK_RESST:
			rtn = U32_A_U16(rtn,LOGICCHNBAK_RESSTOFFSET);
			break;

		case BAK_DATA:
			break;

		case BAK_CRLST:
			rtn = U32_A_U16(rtn,LOGICCHNBAK_CRLSTOFFSET);
			break;

		case BAK_FLAG:
			rtn = U32_S_U8(U32_A_U16(rtn,EEPAGE_SIZE),0x04);
			break;
		default:
			break;
	}
	return MEMtPM8(rtn);
}


void bakLogicalChninfo(void)
{

	ppIso7816Util_Response_t			ppResponse;
	piso7816T0_Control_t				pT0Control;
	ppIso7816Util_Command_t 			command;

	iso7816Util_GetResponse( &ppResponse );
	pT0Control	= get_g_pT0Control();
	iso7816Util_GetCommand(&command);

	if(((iso7816Channel_GetCurLogicalChn()<ISO_MAX_LOGICAL_CHANNELS) || (channelInfo.channel_status[0] == CHANNEL_OPEN))&&(iso7816Channel_CalLogicalChannel(command->pData[0]) != pT0Control->command->chn))
	{
		memCopyNoAtomic(bakLogicalChninfoaddr(iso7816Channel_GetCurLogicalChn(),BAK_DATA),&ppResponse->pData[pT0Control->ResponseOffset],ppResponse->length);
		memCopyNoAtomic(bakLogicalChninfoaddr(iso7816Channel_GetCurLogicalChn(),BAK_RESST),MEMtPM8(ppResponse),sizeof(pIso7816Util_Response_t));
		memCopyNoAtomic(bakLogicalChninfoaddr(iso7816Channel_GetCurLogicalChn(),BAK_CRLST),MEMtPM8(pT0Control),sizeof(iso7816T0_Control_t));
		b616crecflag[iso7816Channel_GetCurLogicalChn()] = 0;
		memRefresh();
	}
	return;
}

u8 recLogicalChninfo(void)
{
	ppIso7816Util_Response_t			ppResponse;
	piso7816T0_Control_t				pT0Control;
	u8									bakpT0Control[0x20];
	u8									curCmdChannel;
	ppIso7816Util_Command_t 			command;

	pT0Control	= get_g_pT0Control();

	iso7816Util_GetResponse( &ppResponse );
	iso7816Util_GetCommand(&command);

	curCmdChannel = iso7816Channel_CalLogicalChannel(command->pData[0]);

	if(curCmdChannel >= ISO_MAX_LOGICAL_CHANNELS)
	{
		return false;
	}

	ecpMem_Copy(bakpT0Control,pT0Control,sizeof(iso7816T0_Control_t));
	memCopyNoAtomic(MEMtPM8(pT0Control),bakLogicalChninfoaddr(curCmdChannel,BAK_CRLST),sizeof(iso7816T0_Control_t));
	if((pT0Control->command == 0)||(b616crecflag[curCmdChannel] == REC_FLAG))
	{
		ecpMem_Copy(pT0Control,bakpT0Control,sizeof(iso7816T0_Control_t));
		if((channelInfo.channel_status[0] == CHANNEL_CLOSED) && (curCmdChannel==0))
		{
			return true;
		}
		else if(U8_AND_U8(U16t8(getChannelContext(curCmdChannel)), 0xFF)==UICC_APPID)
		{
			//if(((command->pData[0])&0x30) == 0x30) //移动备案  3G UICC测试/UICC 测试/命令测试/StructureOfCommandsAndResponse-6.7.1   APDU: 30C0000000 期望6E00

#if SE_ONLY
				ppResponse->SW1SW2 = 0x6e00;
#else
//EPX-测试列表\eUICC平台基本通信功能测试工程集(RSP)V1.0.6\01_基本通信功能测试.sol\GSM平台11.17测试脚本\DESCRIPTION OF THE FUNCTIONS\GetResponse
				//EPX-测试列表\eUICC平台基本通信功能测试工程集(RSP)V1.0.6\01_基本通信功能测试.sol\GSM平台11.17测试脚本\DESCRIPTION OF THE FUNCTIONS\GetResponse_EnhancedTest

				ppResponse->SW1SW2 = 0x6f00;//暂时先统一成6E00   后续遇到需要6F00的用例直接放开 if else条件
#endif



			ppResponse->length = 0x00;
			return true;
		}
		else
		{
			ppResponse->SW1SW2 = 0x6881;
			ppResponse->length = 0x00;
			return true;
		}
	}
	ecpMem_Copy(pT0Control->iso7816T0_CurHeader,((piso7816T0_Control_t)bakpT0Control)->iso7816T0_CurHeader,T0_COMMAND_HEADER_LENGTH);
	memCopyNoAtomic(MEMtPM8(ppResponse),bakLogicalChninfoaddr(curCmdChannel,BAK_RESST),sizeof(pIso7816Util_Response_t));
	memCopyNoAtomic(&ppResponse->pData[pT0Control->ResponseOffset],bakLogicalChninfoaddr(curCmdChannel,BAK_DATA),ppResponse->length);
	b616crecflag[curCmdChannel] = REC_FLAG;
	return true;
}

void b616crecflag_init(void)
{
	ecpMem_Set(b616crecflag,REC_FLAG,ISO_MAX_LOGICAL_CHANNELS);
}

void IFBakChannelInfo(void)
{
	piso7816T0_Control_t				pT0Control;

	pT0Control	= get_g_pT0Control();

	if(RESP_61 == pT0Control->b6C61)
	{
		if(iso7816Channel_CalLogicalChannel(pT0Control->iso7816T0_CurHeader[ISO7816UTIL_CLA_OFFSET]) != pT0Control->command->chn)
		{
			bakLogicalChninfo();
		}
	}
}

#endif		
u32 JcvmConfig_GetSERtpBase(void)
{
	return U32_A_U32(VM_CONFIG_START, rTP);
}
u16 tranMananger_getSERtp(void)
{
	u16   tempRtp;

	if (U8_AND_U8(g_transientCache.Flag, RTR_FLAG))
	{
		tempRtp = g_transientCache.RTR_Ptr;
	}
	else
	{
		tempRtp = nvmReadU16(MEMtPN8(JcvmConfig_GetSERtpBase()));
	}
	return tempRtp;
}


u16 JcvmConfig_GetdefaultAppID(void)
{
	return nvmReadU16(G_CurExecuEnvironment.defaultAppIDAddr);
}

u32 JcvmConfig_GetCardStateBase(void)
{
	return (memref)G_CurExecuEnvironment.CardStateAddr;
}

u8 JcvmConfig_GetSEState(void)
{
	return nvmReadU8(MEMtPN8(OP_bCardState_JC));
}

u16 JcvmConfig_SetSEState(u8* buffer)
{
		if((buffer[ISO7816UTIL_DATA_OFFSET]!=0x01)&&(buffer[ISO7816UTIL_DATA_OFFSET]!=0x07)&&(buffer[ISO7816UTIL_DATA_OFFSET]!=0x0f)&&(buffer[ISO7816UTIL_DATA_OFFSET]!=0x7f)&&(buffer[ISO7816UTIL_DATA_OFFSET]!=0xff))
		{
			return 0x6984;
		}

		nvmWriteU8Atomic(MEMtPN8(OP_bCardState_JC),buffer[ISO7816UTIL_DATA_OFFSET]);

		return 0x9000;
}
u8 JcvmConfig_GetCardState(void)
{
	return nvmReadU8(MEMtPN8(JcvmConfig_GetCardStateBase()));
}

#if (!SE_ONLY)
u8	Euicc_getCurExecProfile(void)
{
	return G_CurExecutingProfileIndex;
}
#endif



