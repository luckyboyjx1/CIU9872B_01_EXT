/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   hard_api.c
 *
 * Description: 该文件主要定义底层相关的功能函数
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/

#include "typedef.h"
#include "hard_api.h"
#include "uicc_main.h"
#include "interface.h"
#include "gsmpublic.h"
#include "platform_api.h"
#include "keyGroup.h"
#include "keySet.h"
#include "object.h"
#include "ToolkitAPIPublic.h"
#include "packageloc.h"
#include "stack.h"
#include "ToolkitRFM.h"
#include "method.h"
#include "STKRemoteAppManagement.h"
#include "EUICCMain.h"
#include "Crypto_API.h"
#include "RSA_API.h"
#include "EllipseCurve_API.h"
#include "EUICCCustomerPublic.h"
#include "EUICCCustomerCertificate.h"
#include "EUICCProfile.h"
#include "VMAccessEeprom.h"
#include "T0ApduImpl.h"
#include "interpreter.h"
#include "SecurityDomainData.h"
#include "KeyLogicManager.h"
#include "HttpProcess.h"
#include "profileApi.h"
#include "opFunction.h"
#include "SEEUICCInterface.h"
#include "SCPInterface.h"
/*
*@function    : IF_AppSetChannelContext
*
*@description : 设置App的通道信息（从0通道开通道，新通道的隐式appID根据参数、权限查，从辅助逻辑通道上打开通道，新通道的隐式应用是正在开通道的通道上应用）
*@param       : *AppId 新打开的通道的默认appid在appletEntry中的index
*@param       : channelId   通道号
*@param       : contextId	上下文
*
*@return      :
*
*@history     :
*/
void IF_AppSetChannelContext(BYTE cmdChannel, BYTE channelId, BYTE appletIndex)
{
	BYTE idata bakChannel,uiccAppId;

	uiccAppId = GetUICCAppId();

	//if(ILLEGAL_APPID == ILLEGAL_APPID)//没有安装UICCAPP
	if(uiccAppId == ILLEGAL_APPID)//没有安装UICCAPP
	{
		return;
	}

	if(appletIndex == getIndexByID(uiccAppId))//新开的通道隐式选择UICC
	{
		//if(INS == 0x70)	 //通道管理
		//{
			bakChannel = G_ChannelNum;

			G_ChannelNum = channelId;

			if((cmdChannel!=0)&&((getChannelContext(cmdChannel) & 0xFF)==uiccAppId)) //当前通道是UICC应用下开通新的通道，需要拷贝通道信息
			{
				SetChannel(G_CurrentDfAddr[cmdChannel], G_CurrentDfName[cmdChannel], FALSE);	//发出命令的逻辑通道的当前DF应被选为当前DF

				G_CurrentAdfName[G_ChannelNum] = G_CurrentAdfName[cmdChannel];
			}
			else
			{
				SetChannel(E_FILE_START,0x3F00,FALSE);
			}

			G_ChannelNum = bakChannel;
		//}
	}

}

#define  MODE_GetEF_ToolkitAppRef     0//方案变动 该模式不再使用
#define  MODE_SetEF_ToolkitAppRef     1//该模式没有用到，直接在Java层赋值了
#define  MODE_SetToolkitAppletInfoToApplet       2
#define  MODE_SetSD_MenuInfoRef       3     //将SD_Menu_Info[]的引用写入SDDATA

/*
*@function    : GetSetToolkitInfoRef
*
*@description : 获取或者设置ToolkitInfoRef
*@param       :
*@param       :
*@param       :
*
*@return      :
*
*@history     :modify by zhxx@20221204
*/
u16 GetSetToolkitInfoRef(void)
{
    u8    X_A    mode;
    u16   X_A    ref,theApplet;
    PN8   X_A    appObjAddr;
	  u32   X_A    sdDataObj;

    mode        =     popShort();
    ref         =     PopOperandStackRef();
    theApplet   =     PopOperandStackRef();

    switch(mode)
    {
//    	case MODE_GetEF_ToolkitAppRef:
//    		ref =  G_EF_ToolkitAppRef;
//    		break;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
       	case MODE_SetToolkitAppletInfoToApplet:

       		if(G_ToolkitAppletInfoRef != REF_NULL)
       		{
				appObjAddr			= 	MEMtPN8(locateObjectInMemory(theApplet));
				nvmWriteU16Atomic(MEMtPN8(get_object_field_ref(MEMtPF8(appObjAddr), toolkitInfo_INDEX)), G_ToolkitAppletInfoRef);
       		}
       		G_ToolkitAppletInfoRef = REF_NULL;
       		G_ToolkitAppIndex = ILLEGAL_APPID;

        	break;
#endif
       	case MODE_SetSD_MenuInfoRef:
			if(G_SD_MenuInfoRef != REF_NULL)
			{
				sdDataObj = sdData_getsdDataObjByAppId(cc&0xFF);

				memWriteU16Atomic(MEMtPM8(get_object_field_ref(sdDataObj, SD_Menu_Info_INDEX)),G_SD_MenuInfoRef);
			}
			break;

    	default:
    		ref = REF_NULL;
    		break;
    }

    return  ref;
}

#define  MAKESELETABLE_MODE     0
#define  SETSTATUS_MODE         1
void ToolkitAppLifecycleChange(void)
{
	u8     I_A   mode,appId;
	u16    D_A   result;

	mode = popShort();
	appId = popShort();

#if UICC__FUNCTION__STKAPI
	if(mode == MAKESELETABLE_MODE)//install for make selectable
	{
		result = NativeImpl_InstallForMakeSelectable(appId);
	}
	else//set status
	{
		result = NativeImplSetStatus(appId);
	}

	if(result != 0x9000)
	{
		ThrowError(0x6A80);
	}
#endif

}

/*应用可能通过process方法访问文件系统  进应用前设置文件相关信息*/
void IF_SelectSuccessJavaApp(BYTE channel, BYTE*aidBuf,BYTE aidlen)
{

#if UICC__FUNCTION__STKAPI
	u8     I_A    recordNum;

	recordNum =  UICCToolkitFindSpecifiedAID(aidBuf,aidlen);   //假设当前选中的是toolkit应用，允许在toolkit应用的process方法中执行simView uiccView的指令

	G_ToolkitContext.CurrentApprecNum[channel] = recordNum; //如果Record为false,因为文件权限不可能满足，所有的文件操作均会失败，此处的赋值不影响

	G_ToolkitContext.CurrentEvent[channel] = EVENT_InitValue;  //	非法事件

	if(recordNum != FALSE)	//通过SIMAPI UICCAPI访问文件，由于其访问权限存在于toolkitAppAddr文件中，对于install参数中不带CA或EA的情况，RecordNum为false，权限必定不会满足
	{
		//ReadLFBufForJavaAccess(toolkitAppAddr,RecordNum,TrigerMode_OFFSET,&G_ToolkitContext.CurrentTrigeMode[currentChannel],1);

#if UICC__FUNCTION__SIMAPI
		Toolkit_InitSIMView(channel,0xFF);  // 成功选择某个java应用，时，统一将SIMView的fileContext信息进行初始化，由于是各通道分别初始化，所以对流程不会有影响
		//事务断电测试中 CutPowerTest_19_1 中，安装的应用带了CA 参数，但是却没有实现toolkit接口，其在process中调用了SIMAccess方法，识别成了UICCAPI 模式，此处需确认,注意接口NativeImpl_EndOfRegister的实现
#endif
	}
	else  //非toolkit应用也可以调用SIMView,uiccView的接口，只是权限错误 ？ by xuyz
	{
#if UICC__FUNCTION__SIMAPI
		//G_ToolkitContext.CurrentTrigeMode[currentChannel] = SIMAPI_Mode;  //初始化触发模式，当RecordNum= FALSE时，默认触发模式为SIMAPI模式，因为文件读写权限不满足，所以不会影响

		Toolkit_InitSIMView(channel,0x00);; 	  //uicc api的上下文是应用自己保存；而SIM View的上下文需要这里初始化之后，再拷贝给API 的中间变量 ,在选择文件时，如果G_CurrentDfAddr地址无效，则文件会选择失败
#else
		//G_ToolkitContext.CurrentTrigeMode[channel] = UICCAPI_Mode;
#endif
	}
#endif
}

/*获取UICC APP的APPID, 后续可以考虑写入配置区 优化性能*/
// modify by xuxiaojun@20220318 分层 该函数被移动到cardManager.c
//u16 getAppIDByIndex(u8 index)	SMALL_MODE

BYTE GetUICCAppId(void) // 可以与GetAppIdByAIDFromAIDList 合成一个
{
#if 0
	u8   		I_A   		i,bFreeEntryIndex;
	u8         D_A          buf[16];
	
	u8   arrayLen;
	jref I_A  aid;
	PN8  X_A  aidArray;

	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	for (i = 1; i < bFreeEntryIndex; U16_INC(i))
	{
		aid = getAppletAid(i);
		if(aid != APP_INDEX_NULL)
		{
			aidArray = MEMtPF8(locateObjectInMemory(aid));
			arrayLen = get_array_length(aidArray);
			if(arrayLen == sizeof(UICCAID))
			{
				aidArray = MEMtPN8(get_array_data(aidArray));
				ecpMem_Copy(MEMtPN8(&buf[0]), aidArray, arrayLen);
				if(ecpMem_Compare(buf,UICCAID,sizeof(UICCAID)) == EQUAL)
				{
					return getAppIDByIndex(i);
				}
			}
		}
	}
	return ILLEGAL_APPID;
#else
	return UICC_APPID;
#endif
}

/*在某个通道选中ADF以后 如果通道上选中的应用不是UICC ，需要选择UICC 并且设置通道的文件信息*/
static u8 CONST UICCAID[0x10] = {0xA0,0x00,0x00,0x05,0x33,0x80,0x00,0xFF,0x86,0xFF,0xFF,0x89,0x00,0x00,0x00,0xF7};
void IF_SelectADFSucess(BYTE channel)
{
	u8   I_A   	bakChannel;

	bakChannel = G_ChannelNum;

	G_ChannelNum = channel;

	SetChannel(E_FILE_START,0x3F00,FALSE);  //防止在BackFilePath函数中 读异常

    G_ChannelNum = bakChannel;

    APDUBuffer[4] = 0x10;

    ecpMem_Copy(&APDUBuffer[5],UICCAID,0x10);//将UICC APP拷贝到APDUBuffer进行选择操作
}
BYTE GetAIDDataByAppID(BYTE appID, BYTE* buf)
{
	u16   I_A  theAppletIndex,arrayLen;
	jref I_A  aid;
	PN8  X_A  aidArray;

	if(appID == ILLEGAL_APPID)
	{
		return FALSE;
	}

	theAppletIndex = getIndexByID(appID);//appEntryTable中的下标

	if(theAppletIndex == APP_INDEX_NULL)
	{
		return 0;
	}
	aid = getAppletAid(theAppletIndex);

	aidArray = MEMtPF8(locateObjectInMemory(aid));

	arrayLen = get_array_length(aidArray);

	aidArray = MEMtPN8(get_array_data(aidArray));

	ecpMem_Copy(MEMtPN8(&buf[0]), aidArray, arrayLen);

    return arrayLen;
}

WORD GetAppletRefByAppId(BYTE appId)
{
	u32  D_A  appEntryAddr;
	u16  D_A  objRef;

	appEntryAddr = getEntryByAppId(appId);

	if(appEntryAddr == MEM_NULL)//modify by zhxx@20230215,45.uicc_configuration_additional_test_工程集1/SECURED/GP补充测试/install_before_register_createSCP81Connection_Test01:Transaction 1:  正常情况测试-在install中，register之前调用createSCP81Connection，安装报6A88
	{
		return REF_NULL;
	}

	objRef = PX_R_U16(get_object_field_ref(appEntryAddr, theApplet_INDEX));

	return objRef;
}

BYTE GetAssoiatedSDByAppId(BYTE appId)
{
	u8 idata tmpAppId;
	
	tmpAppId = getAssociateSDId(appId);
	
	if(tmpAppId == ILLEGAL_APPID) // SD associated with itself
	{
		tmpAppId = appId;
	}
	
	return tmpAppId;
}

void GPRunIntoJavaFunction(BYTE mode)
{
	//PF8	 D_A	mainMethodAddr;		//remove warning,dengyiwen,2022/9/27

	G_JCREMainProcessMode = mode;

	if(mode == JCREMain_RAMMode)
	{
		bRamBuffer[REMOTE_SCP_KEY_VERSION] = G_RFMRAMKeyversion;

		if(IF_IsHTTPCommand() == TRUE)
		{
			//bRamBuffer[REMOTE_SCP_SDID] = G_HTTPSStatus.TargetedSD; //电信测试  T_018_HTTP_NoSeg_RAM_MultipleCommandSession
		}
		else
		{
			bRamBuffer[REMOTE_SCP_SDID] = G_TARRecIndex[2];
		}

		GPSubCallFunction(UICCRAMAPDUProcessMain_METHOD_ADDR, UICC_PKG_ID, JCRE_CONTEXT);

		bRamBuffer[REMOTE_SCP_KEY_VERSION] = 0xFF;
	}
	else
	{
		GPSubCallFunction(TriggerToolkitApiMain_METHOD_ADDR, UICC_PKG_ID, JCRE_CONTEXT);
	}

	G_JCREMainProcessMode = JCREMain_INITMode;
}

void GPSubCallFunction(UWORD staticMethodOff, BYTE pkgId, WORD context)
{
	memref   D_A      staticMethodAddr;

	staticMethodAddr = PFtMEM(PF8_A_U16(MEMtPF8(getPackageComponentLocation(pkgId, COMPONENT_METHOD)), staticMethodOff));

	callmethod(MEMtPF8(staticMethodAddr), pkgId, context);

	run();

	popFrame(); //恢复当前上下文	
}
// modify by xuxiaojun@20220318 分层 该函数被移动到Interface_LTEFromPlatform.c
//BYTE GPSSDExistCompleteSecureKey(BYTE sdId,u8 scp_type)
void GPLoadInit(void)
{
	GPSubCallFunction(ClearProfileLoadingPkg_METHOD_OFFSET, IMPL_PKG_ID,JCRE_CONTEXT);//清掉正在下载的包
}
/*清remote模式的安全会话 ，注意：如果ram里面的指令不是非0通道会有问题  比如：8150xxxxxx 8582xxxxxx*/
void GPClearSecureSessionInfo(/*u8  sdId*/)
{
	if(G_RAMSessionSdId == ILLEGAL_APPID)
	{
		return ;
	}

	closeSecuritychannel(SC_TERMINATED);
}

/*
*@function   :GetAppPrivileges
*
*@description:根据应用ID获取应用权限
*
*@param      : appId  应用ID
               pri    存放appId所指应用的权限
			   len    指定读取的权限长度
*
*@return     : 获取权限成功返回权限的字节数1或3,失败返回FALSE
*
*@author     :xxj
*
*@history    :create by xxj@20200220 -该函数在被调用时，要保证appId一定是一个有效的应用ID,备注： 其调用处已经经过了初步检查
*/
void GetAppPrivileges(BYTE appId, BYTE* pri, BYTE len)
{
	u16 	bPrivRef;
	memref  enTry;
	memref	bPrivArray;
	
	enTry	   	=	getEntryByAppId(appId);

	//check SSD privilege
	bPrivRef = memReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(enTry), bPriv_INDEX)));
	bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
	
	ecpMem_Copy(pri, MEMtPM8(bPrivArray), len);
}

//BYTE GetAppIdByAIDFromAIDList(BYTE *aidValue,BYTE aidLen) 该函数被移到opFunction.c中

/*
*@function   : GetAppIdByRef
*
*@description:
*
*@param      :
*
*@return     : appId
*
*@author     :
*
*@history    
*/
BYTE GetAppIdByRef(WORD appletRef)
{
	u8   		I_A   	   i,bAppID;
	u8          I_A        bFreeEntryIndex;
	memref	        D_A		   entry;
	u16			I_A        theAppletRef;
	memref	    X_A		   theAppTable;

	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	
	theAppTable = getAppTable();

	for (i = 1; i < bFreeEntryIndex; U16_INC(i))
	{
		entry = locateObjectInMemory(PN_R_U16(U32_A_U8(theAppTable , U8_M_U8(i , SIZE_REF))));
		
		theAppletRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
		
		if(appletRef == theAppletRef)
		{
			bAppID = memReadU8(MEMtPN8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX)));
			return bAppID;
		}
	}

	return ILLEGAL_APPID;
}
/*获取RAM的目标SD的appletEntry*/
WORD GetRAMProcesser(void)
{
	BYTE  idata  appId;

	appId  =  IF_GPGetRAMAPDUSDId();

	if(appId == ILLEGAL_APPID)
	{
		return REF_NULL;
	}

	return GetAppletRefByAppId(appId);

}
/*清counter 根据测试需求清对应的counter */
void ClearAllCounter(void)
{
	BYTE xdata COUNTER[5];

	ecpMem_Set(COUNTER,0,sizeof(COUNTER));

	setAllKeyCounterForSCP80(ISD_APPID, 5, COUNTER);
}
/*
*@function   : GetAddrAndIndexByIndexAddr
*
*@description:  addr是某个表中的地址(SDAPP_ENTRY_TABLE/APPLET_ENTRY_TABLE等存索引的地址)，该地址处存的是ADDR_INDEX_TABLE中的index；
				返回NewAddrIndexTable中相应索引处存储的地址  index不合法或地址无效均返回0xFFFFFF;(第一个包的组件地址为0，所以不以0作为非法时的返回值)
*				若addr处的index是非法索引 返回的地址是0xFFFFFF,这里仅将数据返回 不对地址的内容进行判断
*@param      : addr
*
@return      ：UWORD   addr
*
*@history    : added by lgy @20160913

*/
UWORD GetAddrAndIndexByIndexAddr(UWORD addr, WORD* indexRef)
{
	WORD idata index;

	index = memReadU16(MEMtPM8(addr));//ReadEEPromCache((BYTE *)&index, addr, sizeof(WORD));   //从表中（addr处）读index

//	if(index == 0x0000)
//	{
//		if(indexRef != NULL)
//		{
//			indexRef[0] = REF_NULL;
//		}
//
//		return ADDR_IS_NULL;
//	}
//	else
//	{
//		if(indexRef != NULL)
//		{
//			indexRef[0] = index;
//		}
//
//		return locateObjectInMemory(index);
//	}
	indexRef[0] = index;
	return locateObjectInMemory(index);
}
/*
*@function   : GetAddrAndIndexByIndexAddr
*
*@description:  addr是某个表中的地址(SDAPP_ENTRY_TABLE/APPLET_ENTRY_TABLE等存索引的地址)，该地址处存的是ADDR_INDEX_TABLE中的index；
				返回NewAddrIndexTable中相应索引处存储的地址  index不合法或地址无效均返回0xFFFFFF;(第一个包的组件地址为0，所以不以0作为非法时的返回值)
*				若addr处的index是非法索引 返回的地址是0xFFFFFF,这里仅将数据返回 不对地址的内容进行判断
*@param      : addr
*
@return      ：UWORD   addr
*
*@history    : added by lgy @20160913

*/
UWORD GetAddrByIndexAddr(UWORD addr)
{
	WORD idata index;

	index = memReadU16(MEMtPM8(addr));

	return  locateObjectInMemory(index);

//	if((index == 0x0000)/*||(index == 0xFFFF)*/)
//	{
//		return ADDR_IS_NULL;
//	}
//	else
//	{
//		return  locateObjectInMemory(index);
//	}
}


#if  EUICC_FUNC_eSE_INTERFACE
void CallEseInterfaceFunc(BYTE tag)
{
#if EUICC__FUNCTION_RedTea_Mobile
	if(tag == 0x7F) //红茶移动私有tag
	{
		IF_RedTea_StoreDataTagFF7F();
		return;
	}
	
	if(tag == 0x20)//Fallback  tag
	{
		IF_RedTea_Fallback();
		return;
	}
#endif

}
#endif
// modify by xuxiaojun@20220318 分层 该函数被移动到SEEUICCInterface.c
//BYTE Cmac(BYTE *key, BYTE keylen, BYTE *M, WORD MLen, BYTE * headMsg, BYTE headMsgLen, BYTE *T)
