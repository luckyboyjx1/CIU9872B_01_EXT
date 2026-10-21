/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   Interface_PlatformFromLTE
 *
 * Description: 该文件是主要存放一些平台层向LTE 层申请资源，以及逻辑控制的一些接口函数，由LTE层人员负责实现函数内部功能，平台层人员负责提供接口说明等
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
 /************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   Interface_PlatformFromLTE
 *
 * Description: 该文件是主要存放一些平台层向LTE 层申请资源，以及逻辑控制的一些接口函数，由LTE层人员负责实现函数内部功能，平台层人员负责提供接口说明等
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/
#include "Interface_PlatformFromLTE.h"
#include "Interface_LTEFromPlatform.h"
#include "Interpreter.h"
#include "cardManager.h"
#include "SEEUICCInterface.h"
#include "object.h"
#include "ob_manager.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "SCPInterface.h"
#include "opFunction.h"
#include "profileApi.h"
#include "BootLifeCycle.h"

#if GP__AMENDMENT__B__DNS_Resolution
#include "DNS.h"
#endif


u16 X_A G_LTEDirFileID; //当前激活的PROFILE的DIRFILEID,通常是2F00

#if (EUICC_ONLY||SE_AND_EUICC)

/*
*@function   :IF_SupportContactlessByProfileIndex
*
*@description:判断profileIndex所指示的profile是否支持Contactless
*
*@param      :u8 profileIndex --- profile Index
*
*@return     :支持则返回TRUE / 不支持返回FALSE
*
*@hisotry    :created by 
*            :modify by xuxiaojun@20190606 如果profileHeader中未指示该profile是否支持非接，则默认采用平台的配置。平台支持则返回TURE；平台不支持则返回FALSE. 目前该平台是支持非接的。
*            :modify by zhangxiaoxiao@20190611 根据euiccInfo_section2中信息判断是否支持非接
*            :modify by xuxiaojun@20190617 是否支持非接与profile本身没有关系，所有把参数去掉,后续可以考虑将该函数的返回值用全局变量保存
*/
/*u8 CONST euiccInfo_section2[19] = {
0x85,0x03,   0x01,0x7E,0x3A,                 //uiccCapability:         T: 85   L:04    V:  
0x86,0x03,   0x09,0x02,0x00,	                  //ts102241Version:        T: 86   L:03    根据others\test\02 外部测试\eUICC test RSP 20190107\eSIM_Test_CMCC.sol中的配置进行修改
0x87,0x03,   0x02,0x03,0x00,                      //globalplatformVersion : T: 87   L:03    V:2.3.0
0x88,0x02,   0x04,0xD0                            //rspCapability           T: 88   L:02    V:04 90         	
};*/
u8 IF_SupportContactlessByProfileIndex(void)
{
#if RSP_EUICC
	
	memref X_A addr;
	u16 I_A ref;
	u8 I_A cfgByte;

	addr = GetAddrAndIndexByIndexAddr(EUICCINFO2_SECTION2,&ref);//读取EUICCINFO2_SECTION2信息
	
	if(addr == ADDR_IS_NULL)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);  //没有执行个人化指令
	}
	
	cfgByte = VMReadByte(addr+1);//读取第一个字节
	
	if(cfgByte != 0x85)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	cfgByte = VMReadByte(addr+4);
	
	return ValueBit(cfgByte,7);	
#else
	return TRUE;				//M2M版本不对非接检查
#endif
	
}
#endif

/*
*@function   :IF_GetADFByAID
*
*@description:该函数用于判断一张普通卡的应用AID和ADF的AID是否重复
*
*@param      :u8* aidBuf --- AID数据
*@param      :u8 aidLen  --- aidBuf中有效数据长度
*
*@return     :不存在时返回ILLEGAL_PKGID / 存在时返回0x2F00文件的记录号
*
*@hisotry    :
*/
#if JAVA_CARD_UICC
void IF_GetADFByAID(void)
{
	u8 I_A result;
	u8 X_A temp[0x11];
	u16 I_A offset,arraryRef;
	memref X_A arrayAddr;
	
	temp[0] = PopOperandStackShort();

	offset  = PopOperandStackShort();

	arraryRef  = popReference();

	arrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(arraryRef)))  + offset;//获取apduBuf数组对象头起始地址

	ecpMem_Copy(temp+1,arrayAddr,temp[0]);
	
	result = GetAIDNumber(temp,FALSE);	//判断ADF aid在2F00中是否存在,如果存在，则返回记录号
	
	if(result == FALSE)
	{
		pushShort(U8tS16(ILLEGAL_PKGID));
	}
	else
	{
		pushShort(U8tS16(result));
	}
	
}
#endif
/*
*@description:该函数主要用于处理在开机复位接收第一个命令头之后，所进行的一些初始化动作,如处理SIM (uicc )API的一些handler初始化	,接下来会进入到命令分发阶段
*
*@param      :u8 mode --- MODE_PROFILE_CHANGE表示profile发生切换但是没有Reset后的第一条指令;
*                           MODE_RESET表示Reset后的第一条指令
*
*@return     :void
*
*@history    :V1.0 create by xuyz
*
*             V1.1 在HandleFirstCommand调用UICCToolkitInit和UICCInition，uicc_main中不再调用，修改联通保险箱测试不同通道冲掉全局变量的问题，by xwh 20210929
*			  V1.2 调用CheckHeFlashRecordResume，对于强更新文件区可能出现的弱写页进行擦除或者实写,by xwh 20211117
*/
void HandleFirstCommand(u8 mode)
{
#if JAVA_CARD_UICC	
	u8 I_A i,index;
	memref X_A entryAddr;

	/*if(mode == MODE_RESET)   // modify by chentao @20230308 HandleFirstCommand 函数中除MoveEepromResume以外的操作挪到私有指令后处理
	{
		MoveEepromResume();

		StartSecureMoveEepromMapTableExtend();//map表扩展过程中，断电，继续文件系统的搬移

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
		CheckHeFlashRecordResume();  //对于强更新文件区可能出现的弱写页进行擦除或者实写,by xwh 20211117
#endif
	}*/

#if (EUICC_ONLY||SE_AND_EUICC)
	index = JcvmConfig_GetProfileEnableIndex();

	//if(index == ILLEGAL_PROFILEINDEX)//没有激活的profile，设置默认值
	if(ProfileIndexIsValid(index) == FALSE)
	{
		G_LTEDirFileID = 0x2F00;
		//G_LTECardCFG = 0xff;//全部支持
	}
	else
	{
		entryAddr = GetProfile_LTEAblityAddr(GetISDPObjectAddr(index));//GetProfile_LTEAblityAddr(GetProfileEntryAddr(index));//

		G_LTEDirFileID = memReadU16(MEMtPM8(entryAddr));

		//G_LTECardCFG=buf[2];
	}
#else
	G_LTEDirFileID = 0x2F00; //SE_ONLY
#endif

	G_CurMFFileStart = GetMFFileStart();

	if(mode == MODE_RESET)
	{
		if(G_CardStatus != D_CardEmpty)
		{
			//FileAdmiResume();	  //去掉E4、D4这两条指令后，无需这两条指令的恢复机制

			IF_InitNoShareableFile();
		}

		ecpMem_Fill(&G_TerminalProfile[0],0x00, sizeof(G_TerminalProfile));   //避免应用的install方法中调用Toolkit_profileCheckPara3时，该变量没有初始化, 印度rjio的第三方应用就是这样干的

#if (EUICC_ONLY||SE_AND_EUICC)
		G_LpaSendRefresh = LPANoRefreshMode;//用于LPA有主动性命令,14还没回来,LPA主动关闭通道的场景使用  放在这里是因为切换profile会调用这个初始化函数,G_LpaSendRefresh初始化后还需要使用
#endif
	}
	else
	{
		//SGP.22_V2.2 5.7.16,If the refreshFlag is not set,the eUICC SHALL reset the PIN status
		if(G_CardStatus != D_CardEmpty)
		{
			ReadPinStatus();                                                      	//读pin的状态
		}
	}

	G_CommandMode = Command_Mode_First;

	//G_DATARECEIVED = 0x00;

	G_ToolkitSystemProactiveFlag = FALSE; //SIMReEntry_01_01_01执行installforinstall时，执行到Toolkit_initMenuEntry函数时，InitMenuEntryPublic的SetBit(G_ToolkitSystemProactiveFlag,Bit_SetUpMenu);时，G_ToolkitSystemProactiveFlag未初始化

	G_PollIntervalRecNum = FALSE;
//modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP，删除 该变量
//	G_CUR_DURATION = POLL_NO_DURATION; //SIM API测试用例中的api_2_mep_checs在用例连续执行的情况下，执行到该用例时installforinstall指令时死卡，在GPInstallforInstall函数中重入之后进入RequestPollIntervalPublic函数中，G_CUR_DURATION没有初始化。同样在该函数中还有全局变量G_STKBusy。

	G_STKBusy = FALSE;
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = FALSE;
#endif
	ecpMem_Fill(STKPerform,0x00,sizeof(STKPerform)); // UICC API测试用例中的cre_mha_pahd用例选择测试应用时执行到UICCToolkit_GetHandlerStatus时死卡

	G_ChannelNum = 0x00;    //复位之后直接执行开通道指令，会调用IF_AppSetChannelContext

	G_SystemStatus = FALSE;

	ecpMem_Fill(&G_CurrentReponse[0],0x00,sizeof(G_CurrentReponse));

	//G_VolatileByteArrayIsCalled = FALSE;

	for(i = 0x00;i<0x04;i++)         //《SE访问控制应用测试_工程集\6.1.1》步骤2选择AC应用成功后，步骤3发送指令选择5031
	{
	   	G_CurrentAdfName[i] = FALSE;

		G_CurrentDfName[i] = FALSE;

		G_CurrentDfAddr[i] = FALSE;

		G_CurrentEfName[i] = FALSE;

		G_CurrentEfAddr[i] = FALSE;
 #if UICC__FUNCTION__BERTLVCmd//       #ifdef BERTLVCmd                               //BERTLV结构
		G_BERTLV_OperateTag[i] = FALSE;

 		G_BERTLV_LastOffset[i]= FALSE ;

 		G_BERTLV_CurrentOffset[i] =FALSE;

		G_PreviousEfAddr[i] = FALSE;
#endif
		G_CurrentRecNum[i] = 0;
	}

 	//if(mode == MODE_PROFILE_CHANGE)
 	//{
 	//	SetChannel(E_FILE_START,0x3F00,FALSE);
 	//}

	G_RAMSessionSdId = APP_NULL;

	G_EnvelopHanderAddr = SelectPrivateTFFile(DF_Toolkit,EF_EnvelopeHandlerBuf);

	G_OpenControl = 0;

	InitRemoteManagevariable();

#if (GP__AMENDMENT__B__FOR_MASK || UICC__FUNCTION__CATTP)
	BIPModuleInit();
#endif

#if GP__AMENDMENT__B__FOR_MASK
	InitHttpVariable();
	
	#if GP__AMENDMENT__B__DNS_Resolution	
	DNSResolutionInitVariable();		//增加上电时dns参数初始化
#endif
#endif

#if UICC__FUNCTION__CATTP
	InitCattpVariable();
#endif

#if M2M_EUICC
	G_DownLoadingProfileIndex = ILLEGAL_PROFILEINDEX;

	G_NotificationSmsRetryTime = NOTIFICATION_RETRY_TIMES;          //通知SMS重发次数，暂时设置为3次，可能需要可配置

	G_IsSendHttpsNotification = FALSE;

	G_RefreshAfterCloseBIP = FALSE;

	ecpMem_Fill(G_IMEI_FOR_NOTIFICATION,0x00,sizeof(G_IMEI_FOR_NOTIFICATION));

	G_STATUSNUM_FOR_NOTIFICATION = 0x00;
	G_STATUSNUM_FOR_REFRESH = 0x00;          //refresh重发2次

	G_Handle_Notification_Tag = FALSE;

	G_NotificationTimerId = 0x00;

	G_NetworkStatus = LocationStatus_NULL;
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
	InitPEManageVariable();
#endif  // end of #if (EUICC_ONLY||SE_AND_EUICC)

#ifdef  COS_Update
	eSEPrivateInit(); //eSE初始化
#endif

#if High_Update_InRAM_New
	InitUpdateInRAMVariable();
#endif	

    UICCToolkitInit();			//如果在uicc——main里的热复位，热复位后还是停留在for死循环中，所以此处要将UICC变量全部清空//   uicc api相关全局变量初始化操作
	
    UICCInition();  //UICCInition会调用IF_InitSystem，该函数里面又会调用InitJavaMenuIdInfo，初始化菜单式会用到6F81 6F82的地址，所以要先初始化UICCToolkitInit
#endif
}
/*
*added by chentao @20230308 HandleFirstCommand 函数中除MoveEepromResume以外的操作挪到私有指令后处理
* 该函数只能用于上电复位之后
*/
void TelecomVarInit(void)
{
#if (EUICC_ONLY||SE_AND_EUICC)
	G_CurEnabledProfileIndex = ILLEGAL_PROFILEINDEX;
#endif
	G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;

	if (BootLifeCycle_IsCardInit())
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		EUICCInitGobalVariable();
#else
		ExecuEnvironmentInit(ILLEGAL_PROFILEINDEX); //SE_ONLY
#endif
		HandleFirstCommand(MODE_RESET);
	}
}
/*
*function    : IF_GetFileSystemEndAddr
*
*@description:  获取文件系统的结束地址

*
*@param      :void

*
*@return     :FileSystem 的结束地址
*
*@history    :V1.0 by xuyz @20190310
*/
#if JAVA_CARD_UICC	
memref IF_GetFileSystemEndAddr(void)
{
	return (GetMaxProfileMFEnd());
}
#endif

/*
*function    :IF_GetScp81KeyVersionNum
*
*@description:当前通过RAM方式执行M2M_DeleteCommand_FinaliseISDRhandover指令时，需要知道当前正在使用的密钥是否是保留不删除的密钥
*@see        :
*
*
*@return     :u8 keyVersionNum(当前使用的scp81 密钥版本)/FF(当前不是SCP81会话 或者不需要使用SCP81密钥)
*
*@history    :V1.0 created by jiangqb @20190508
*
*
*/
u8 IF_GetScp81KeyVersionNum(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	u8 I_A keyversion;

	memref X_A FileAddr;

	if (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)
	{
		FileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);

		if (FileAddr == FALSE)
		{
			return ILLEGAL_KEYVERSION;
		}

		ReadLFBuf(FileAddr, HTTP_TriggerPara_SecurityPara_Index, 2, &keyversion, sizeof(u8));		//keyversion复用表示长度

		keyversion += 2 + 1 + 1;

		ReadLFBuf(FileAddr, HTTP_TriggerPara_SecurityPara_Index, keyversion, (u8*)&keyversion, sizeof(u8));

		return keyversion;
	}
	else
	{
		return ILLEGAL_KEYVERSION;
	}

#else

	return ILLEGAL_KEYVERSION;

#endif
}
/*
*function    :IF_GetScp81TriggerSD
*
*@description:获取当前建立的HTTP链接的SD的APPID
*
*@see        :
*
*@return     :void
*
*@history    :V1.0 created by xuxiaojun @20191125
*
*/
u8 IF_GetScp81TriggerSD(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	if (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)
	{
		return G_HTTPSStatus.TriggeredSD;
	}
	else
	{
		return APP_NULL;
	}

#else
	return APP_NULL;
#endif
}


/*
*@description:JCREMain_RAMMode模式下获取当前的SDID
*
*@function   :IF_GPGetRAMAPDUSDId
*
*@param      :void
*
*@return     :当前RAM APDU在哪个SD下面执行
*
*@author  :xuyz
*
*@history :V1.0 create by xuyz
*         :V1.1 modify by jiangqingbin @20190222 isdr的模式增加
*         :V1.2 modify by jiangqingbin @20190302 gpinstallcommand中添加对http来的指令判断当前sd是http指定的sd
*         :V1.3 modify by xuxiaojun@20190415 增加ISDP模式
*/
u8 IF_GPGetRAMAPDUSDId(void)
{
#if UICC__FUNCTION__STKAPI
	//memref X_A fileAddr;
	//u8 I_A sdId;
#if M2M_EUICC
	if(G_TARRecIndex[0] == TarType_ISDR)
	{
		if(G_M2MStoreData[1] != ILLEGAL_PROFILEINDEX)//通过ISDR installfor personal 给ISDP建立秘钥
		{
			return ISDP_APPID;
		}
		return ISDR_APPID;
	}

	if(G_TARRecIndex[0] == TarType_ISDP)
	{
		return ISDP_APPID;
	}
#endif
	
#if GP__AMENDMENT__B__FOR_MASK		//同步tpx平台代码，当RAM模式指令时，http会话中需要返回的是http头指定的目标安全域SDid, 这个点TPX平台很早就修改了，当时TPX移植到EPX时遗漏了这个点
	if(IF_IsHTTPCommand() == TRUE)
	{
		return G_HTTPSStatus.TargetedSD;
	}
#endif
	
	if(G_TARRecIndex[0] != TarType_RAM)
	{
		return APP_NULL;
	}

	return G_TARRecIndex[1];//对于RAM 该值存的是目标SDID

    #else
	return APP_NULL;
    #endif
}

/*
*function    :IF_IsHTTPCommand
*
*@description:判断当前指令是否是通过SCP81通道下发的
*

*@see        :
*
*@return     :TRUE表示是;FALSE表示不是
*
*@history    :V1.0 created by xuxiaojun @20191125
*
*/
u8 IF_IsHTTPCommand(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	if(G_Source_Type == SourceType_HTTPS)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

#else
	return FALSE;
#endif
}

/*
*@function    :SelectSuccessJavaApp()
*
*@description :在选择应用时，如果当前选中的是某个ToolkitApp，则将其在6F80中的记录号保存在全局变量中，以便于process方法会调用UICC Acess中的Select等API函数，进行文件权限判断
*
*@param       :void
*
*@return      :void
*
*@history     :V1.0 create by xuyz
*			  :V1.1 不要考虑在选择UICC ADF时，会重复进入该函数进行赋值；因为如果两次都是选中的UICC ADF，则代码逻辑不会走到该函数中
*			  :V1.2 要考虑当前在Toolkit的 STK 中，进入该函数，G_ToolkitContext的上下文如果处理？如CurrentEvent的值为EVENT_InitValue，表示当前不能获取handler，那么在STK中发生了选择A4，可能会改变CurrentEvent的值
*             :V1.3 modify by xuweihua@20190317  增加函数SetClockTag，对于7816，选择application之后再提高芯片频率
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制
*/
#if JAVA_CARD_UICC
void SelectSuccessJavaApp(void)
{
/*#if UICC__FUNCTION__STKAPI
	u8 I_A currentChannel;
	u8 I_A aidlen,RecordNum;
	u8 X_A aidBuf[16];
	memref X_A toolkitAppAddr;

	if(INS == 0xA4)//对于A4来说，执行的一定是G_CurSelectedChannel通道上的应用,有可能是新开通的，也有可能是原始通道
	{
		currentChannel = G_CurSelectedChannel;

		//IF_SetClockTag(); //Select Application 成功后， 设置G_SetClockFinished标记，在 Sendresponse后调用 MGR_SetClockConfig 提高始终频率
	}
	else  //70指令，只有当前是开通道，并且有可选择的应用才会进入该函数，即 RecData[OpenChannelId_Offset] 一定是有效值
	{
		currentChannel = RecData[OpenChannelId_Offset];
	}

	aidlen = GetAIDDataByAppID(U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF),aidBuf);

	RecordNum =  UICCToolkitFindSpecifiedAID(aidBuf,aidlen);   //假设当前选中的是toolkit应用，允许在toolkit应用的process方法中执行simView uiccView的指令

	G_ToolkitContext.CurrentApprecNum[currentChannel] = RecordNum; //如果Record为false,因为文件权限不可能满足，所有的文件操作均会失败，此处的赋值不影响

	G_ToolkitContext.CurrentEvent[currentChannel] = EVENT_InitValue;  //	非法事件

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(RecordNum != FALSE)	//通过SIMAPI UICCAPI访问文件，由于其访问权限存在于toolkitAppAddr文件中，对于install参数中不带CA或EA的情况，RecordNum为false，权限必定不会满足
	{
		//ReadToolkitAppData(toolkitAppAddr,RecordNum,TrigerMode_OFFSET,&G_ToolkitContext.CurrentTrigeMode[currentChannel],1);

#if UICC__FUNCTION__SIMAPI
		Toolkit_InitSIMView(currentChannel,0xFF);  // 成功选择某个java应用，时，统一将SIMView的fileContext信息进行初始化，由于是各通道分别初始化，所以对流程不会有影响
		//事务断电测试中 CutPowerTest_19_1 中，安装的应用带了CA 参数，但是却没有实现toolkit接口，其在process中调用了SIMAccess方法，识别成了UICCAPI 模式，此处需确认,注意接口NativeImpl_EndOfRegister的实现
#endif
	}
	else  //非toolkit应用也可以调用SIMView,uiccView的接口，只是权限错误 ？ by xuyz
	{
#if UICC__FUNCTION__SIMAPI
		//G_ToolkitContext.CurrentTrigeMode[currentChannel] = SIMAPI_Mode;  //初始化触发模式，当RecordNum= FALSE时，默认触发模式为SIMAPI模式，因为文件读写权限不满足，所以不会影响

		Toolkit_InitSIMView(currentChannel,0x00);; 	  //uicc api的上下文是应用自己保存；而SIM View的上下文需要这里初始化之后，再拷贝给API 的中间变量 ,在选择文件时，如果G_CurrentDfAddr地址无效，则文件会选择失败
#else
		G_ToolkitContext.CurrentTrigeMode[currentChannel] = UICCAPI_Mode;
#endif
	}
#endif*/

	u16 I_A aidLen,aidOffSet,aidBuf;

	u8 I_A channelId;

	memref X_A aidAddr;

	aidLen = PopOperandStackShort();

	aidOffSet = PopOperandStackShort();

	aidBuf =  popReference();

	channelId =  (u8)popReference();

	aidAddr = get_array_data(MEMtPF8(locateObjectInMemory(aidBuf)))  + aidOffSet;//获取apduBuf数组对象头起始地址

	//aidAddr = locateObjectInMemory(aidBuf);

	IF_SelectSuccessJavaApp(channelId,MEMtPM8(aidAddr),aidLen);
}
#endif
/*
*@function    : SetbSDId
*
*@description : 根据指定应用的权限值设置g_bSDId[selected_channel]
*
*@param       :
*
*@return      : void
*@history     :added by https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin/code @20210816
*@history     :move this function to cardManager.c
*/
//void SetbSDId()

/*
*@function   :IF_SetHttpScriptState
*
*@description:设置http返回数据的状态, RAM模式下的tarGet-sd不是触发sd的时候指令内部判断是否支持该指令，参考附录B-> 3.3.3.2-> Secure Channel Protocol Usage
*			 :测试用例 T_020_HTTP_NoSeg_RAM_SCP02_FailUnwrap_1 和 T_020_HTTP_NoSeg_RAM_SCP02_FailUnwrap_3
*
*@param      :void
*
*@return     :void
*
*@history    :created by jiangqb@20190308
*/
void IF_SetHttpScriptState(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	if (G_Source_Type == SourceType_HTTPS)
	{
		if(MOBILE_Operator == OPERATOR_CHINATELECOM)
		{
			G_HTTPSStatus.httpException = HTTP_SEC_ERR;
		}
	}
#endif
}


/*
*function    :IF_IsRAMActiveApp
*
*@description:判断当前传入的appId, 是否在远程业务中正在被使用
*
*@para       :u8 appId	: 输入参数, sd或者app的id
*
*@return     :true表示是; false表示不是
*
*@history    :V1.0 created by jiangqingbin @20221012
*@history    :V1.1 created by jiangqingbin @20221104 - 代码评审后修改, 不判断G_TARRecIndex[2]更合理, 并且方便后续代码分层, BYTE/TRUE/FALSE分别修改为u8/true/false
*
*/
u8 IF_IsRAMActiveApp(u8 appId)
{
#if GP__AMENDMENT__B__FOR_MASK
	if ((appId == G_HTTPSStatus.TargetedSD)
		|| (appId == G_HTTPSStatus.TriggeredSD)
		|| (appId == G_HTTPSStatus.TriggeredAppId)
		|| (appId == G_HTTPSStatus.RAMSessionSdId))
	{
		return true;
	}
#endif

#if JAVA_CARD_UICC	
	if ((appId == G_RAMSessionSdId) || (appId == g_SCP02SDID))
	{
		return true;
	}
#endif
	return false;
}


#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@function   :IF_PEAnayleGlobalVariableInit
*
*@description:该函数用于初始化所有与PE解析相关的全局变量。这些变量在当前profile下载结束之后，下一次profile下载开始之前，必须全部清掉
*
*@param       :void
*
*@return      :void
*
*@hisotry     :created by zhangxiaoxiao@20190327
*/
void IF_PEAnayleGlobalVariableInit(void)
{
	ContactlessProtocolParameterInit();
	
	InitPEManageVariable();
}

/*
*@function   :IF_SendRefreshCommandOfUICCResetMode
*
*@description:发送UICC Reset Mode模式的Refresh命令
*
*@param       :无
*
*@return      :ture
*
*@author       :
*
*@hisotry      V1.1 删除参数 mode  by chentao@20231123:
*/
#if RSP_EUICC
u8 IF_SendRefreshCommandOfUICCResetMode()
{
	if(IF_TerminalprofileSupportRefresh())
	{
		G_LpaSendRefresh = LPAReseteMode;  //modify by chentao@20231123  02.eSIM_Test_RSP.sol]<30.eSIM_plan_admend_test><eSIM方案修正测试><eSIM_plan_admend_test_05_03~05_04>
		 //1 通道执行memsetReset 删除了激活的profile，需要切换profileCOS主动关闭了1通道，同时1需要发送refresh (910B), 在发送了91XX还没有来12的情况，执行了00A40410 选择USIM ADF走到这里，结束了主动式命令，导致910B这个主动式命令丢了

		
		G_NextAPDUFlag = FALSE;//当前指令已处理完 需要发响应，不需要再处理APDU

	 	InitProactiveCommand(CMD_REFRESH,REFRESH_MODE_UICC_RESET,DEV_ME);	
	 	
	 	//// modify bu chentao@20230303
	 	//收到了14 结果是CMD_SUCUSS，但是卡片不复位----方案：Profile state change is ongoing状态认为是TRUE。 不允许执行ES10指令，仍然在当前激活的profile下运行
	 	if(CMD_Success == SendProactiveCommand())
	 	{
	 		return TRUE;
	 	}
	}
	
	clearHttpRetryFlag();

	G_LpaSendRefresh = LPANoRefreshMode;

	return FALSE;
}
/*
*@function   :IF_SendRefreshCommandOfeUICCProfileStateChange
*
*@description:发送eUICC Profile State Change模式的Refresh命令
*
*@param      :void
*
*@return     :u8 result : 主动式命令结果 ：成功返回TRUE/失败返回FALSE
*
*@hisotry    :created by xuweihua@20190418
*            :modify by xuxiaojun@20190618 优化代码
*/
u8 IF_SendRefreshCommandOfeUICCProfileStateChange(void)
{
	if(IF_TerminaprofileSupportEUICCChangeMode() == TRUE)
	{
		G_LpaSendRefresh = LPAChangeMode;
		
		G_NextAPDUFlag = FALSE;//当前指令已处理完 需要发响应，不需要再处理APDU

	 	InitProactiveCommand(CMD_REFRESH,REFRESH_MODE_PROFILE_STATE_CHANGE,DEV_ME);	
	 	
	 	if(CMD_Success == SendProactiveCommand())
	 	{
	 		G_LpaSendRefresh = LPANoRefreshMode;  // add by chentao @20230114 refersh 结束激活过程完成
	 		return TRUE;
	 	}
	}
	
	G_LpaSendRefresh = LPANoRefreshMode;
 	
	return FALSE;
}
 /*
*@function  :IF_TerminaprofileSupportEUICCChangeMode
*
*description:判断当前terminal profile命令是否支持 "eUICC Profile State Change" mode
×
×param      :void
*
*@return    :TRUE: 支持/FALSE :不支持
*
*@history   :created by xuweihua@20190418
*/
u8 IF_TerminaprofileSupportEUICCChangeMode(void)
{
	if((G_TerminalProfile[0] > 33)&&(G_TerminalProfile[34] & 0x20))
	{
		return TRUE;
	}
	
	return FALSE;
}
/*
*@function  :IF_CATIsTnitialised
*
*description:判断终端是否有发送terminal profile指令
×
×param      :void
*
*@return    :TRUE: 支持/FALSE :不支持
*
*@history   :created by chentao@20230209
*/
u8 IF_CATIsTnitialised(void)
{

	if(G_TerminalProfile[0] ==0)
	{
		return FALSE;
	}

	return TRUE;
}
/*
*@function  :IF_TerminalprofileSupportRefresh
*
*description:判断当前terminal profile命令是否支持Proactive UICC: REFRESH
×
×param      :void
*
*@return    :TRUE: 支持/FALSE :不支持
*
*@history   :created by chentao@20230209
*/
u8 IF_TerminalprofileSupportRefresh(void)
{
	if((G_TerminalProfile[0] >= 3)&&(G_TerminalProfile[3]&0x80))
	{
		return TRUE;
	}

	return FALSE;
}
 /*
*@function   :IF_ChangeProfileLTEInfoNoReset
*
*description :profile发生切换，但是没有reset时，更改lte层相关的全局变量
×
×param       :void
*
*@return     :void
*
*@history    :created by zhengdongmei@20190104
*            :modify by zhangxiaoxiao@20190319
*            ：IF_RSPReturnMainFormSTK  添加中断STK处理  LPA下载Profile时候关闭通道的特殊处理
*            ：//添加初始化  UICCToolkitInit/UICCInition初始化  防止Profile1切换到Profile2相关的变量还是Profile1状态的
*/
void IF_ChangeProfileLTEInfoNoReset(u8 mode)
{
	HandleFirstCommand(mode);
	
	UICCToolkitInit();			//   uicc api相关全局变量初始化操作

	UICCInition();  //UICCInition会调用IF_InitSystem，该函数里面又会调用InitJavaMenuIdInfo，初始化菜单式会用到6F81 6F82的地址，所以要先初始化UICCToolkitInit		
	
	clearHttpRetryFlag();
	
	//IF_RSPReturnMainFormSTK();
}
#endif

/*
*@function   :IF_GetPfoileExcuteState
*
*@description:解析PE过程中,获取PE解析的执行状态
*
*@param      :void
*
*@return     :PROFILE_INIT_STATE: profile执行失败 / PROFILE_CREATING_STATE: profile执行成功，但是还没有执行到PEEND / PROFILE_END_STATE: profile执行成功，且已执行到PEEND部分
*
*@history    :created by zhangxiaoxiao@20190222
*            :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
u8 IF_GetPfoileExcuteState(void)
{
	return G_profileControlContext.ProfileCurrStatus;
}

/*
*function:IF_DeleteFilesystemByProfileIndex
*
*@description:该函数主要用于删除profile后清除文件系统，整个与profileIndex相关的文件系统都需要清掉 需要考虑断电问题
				平台层会根据合适的时机调用，如果是在下载profile过程中出错 只有进入到解析PE后才会创建文件系统 未到PE解析流程不需要调用该函数
*
*@param      : void
*
*@return     : void
*
*@author     :create by lgy
*
*@history    :V1.0: crreate
*
*             V1.1: 实现函数功能，考虑了删除过程中断电的问题, by xwh 20190227
*
*             V1.2: 增加判断，地址非法，说明该profile 的MF 已经被删除了，直接退出， by xwh 20190301
*
*             V1.3: 首先将EF_ToolkitApp 和EF_ToolkitMenuEntry 文件动态申请的空间释放, by xwh 20190318
*/
void IF_DeleteFilesystemByProfileIndex(u8 profileIndex)
{
	memref X_A profileMFOffset,pOffset,findOffset,maxSize;
	u8  X_A buf[PROFILE_MAX_NUMBER];
	u8  X_A indexOffsetBuf[5*(PROFILE_MAX_NUMBER-1)+1];                              //num(1) + (profileIndex(1) + profileMFOffset(4))*(PROFILE_MAX_NUMBER-1)
	u8  I_A i,num;
	memref X_A dstAddr,srcAddr;
	u8  I_A transActioinFlag;

	profileMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + profileIndex*sizeof(memref));

	//地址非法，说明该profile 的MF 已经被删除了，直接退出
	if((profileMFOffset == 0xFFFFFFFF)||(profileMFOffset == 0x00000000))
	{
		return;
	}

	VMReadBuf(buf,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态

	ecpMem_Fill(&indexOffsetBuf[0],0x00,sizeof(indexOffsetBuf));                              //需要移动数据的Profile 个数，初始化成0
	
	for(i= 0; i< PROFILE_MAX_NUMBER ; i++)                                               //以下操作把需要移动数据的Profile 的信息，按照MFOffset 从小到大的顺序存储在indexOffsetBuf 中
	{
		pOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(memref));
		
		if((buf[i] == PROFILE_INVALIDFLAG)||(pOffset == 0xFFFFFFFF)||(pOffset == 0x00000000)||(pOffset <= profileMFOffset))                           //无效的Profile 或者当前要删除的Profile ,不处理
		{
			continue;	
		}
  
		for(num=0; num < indexOffsetBuf[0]; num++)
		{
			findOffset = BuildUWord(&indexOffsetBuf[1+5*num+1]);

			if(pOffset < findOffset)
			{
				ecpMem_Move(&indexOffsetBuf[1+5*num+5],&indexOffsetBuf[1+5*num],(indexOffsetBuf[0]-num)*5);         //向后移动5 个字节

				break;
			}
		}

		indexOffsetBuf[1+5*num] = i;
		
		SetUWord(&indexOffsetBuf[2+5*num],pOffset);	

		indexOffsetBuf[0]++;				
	}

	if(indexOffsetBuf[0] != 0x00)                                                          //有需要移动数据的Profile 
	{
		dstAddr = profileMFOffset;                                                                                 //移动数据的目标地址

		findOffset = BuildUWord(&indexOffsetBuf[1+5*(indexOffsetBuf[0]-1)+1]);      //最后一个Profile的MF offset

		maxSize = GetFileHeadUsedSize(findOffset);                                               //最后一个Profile的size

		srcAddr = BuildUWord(&indexOffsetBuf[2]);                                              //第一个Profile的MF offset

		maxSize = findOffset + maxSize - srcAddr;                                              //需要移动数据的长度
                            
		for(num=0; num < indexOffsetBuf[0]; num++)
		{
			findOffset = BuildUWord(&indexOffsetBuf[1+5*num+1]);

			findOffset -= (srcAddr - dstAddr);                                                                                                                //修改移动数据后的MF 起始地址

			SetUWord(&indexOffsetBuf[2+5*num],findOffset);	                                                     		
		}

		IF_CommitTransaction(isInProgress());
		
		transActioinFlag = IF_BeginTransaction();
#define moveDateBuf  (tempBuf + 100)
		ecpMem_Set(moveDateBuf,0x00,SecureMoveStuct_LengthAll);

		moveDateBuf[SecureMoveEepromTag_Offset] = D_EEPMoveTag_FAST;

		SetUWord(&moveDateBuf[SecureMoveEepromSrcAddr_Offset],srcAddr);
		SetUWord(&moveDateBuf[SecureMoveEepromDesAddr_Offset], dstAddr);
		SetUWord(&moveDateBuf[SecureMoveEepromMoveLen_Offset],maxSize);
		
		VMWriteBuf(E_SECUREMOVE_START, moveDateBuf, SecureMoveStuct_LengthAll);	
#undef moveDateBuf
		for(num=0; num < indexOffsetBuf[0]; num++)
		{						
			VMWriteBuf(PROFILE_MFOFFSET_CFG+ indexOffsetBuf[1+5*num]*sizeof(memref),&indexOffsetBuf[1+5*num+1],sizeof(memref));
		}

		VMSetBuf(PROFILE_MFOFFSET_CFG + profileIndex*sizeof(memref), 0xFF, 4);   //清空当前MF 的偏移
#if UICC_FUCTION_PRIVATEAUTH
		VMSetBuf(E_TAG_AUTHDATA_UPDATE_ADDR + profileIndex, PROFILE_STATE_AUTH_NOT_UPDATE, 1);            //修改"修改鉴权参数"状态为"未执行"(每个Profile 一个状态)
#endif
		IF_CommitTransaction(transActioinFlag);

		StartSecureMoveEeprom(0,0,0);
	}
	else
	{
		VMSetBuf(PROFILE_MFOFFSET_CFG + profileIndex*sizeof(memref), 0xFF, 4);
#if UICC_FUCTION_PRIVATEAUTH
		VMSetBuf(E_TAG_AUTHDATA_UPDATE_ADDR + profileIndex, PROFILE_STATE_AUTH_NOT_UPDATE, 1);                //修改"修改鉴权参数"状态为"未执行"(每个Profile 一个状态)
#endif
	}
}
/*
*@function   :IF_GetProfileExcuteErrorReason
*
*@description:在个人版profile下载过程中，获取profile解析过程中的错误类型 用于拼装通知时使用
ErrorReason ::= INTEGER { incorrectInputValues(1), invalidSignature(2), 
							invalidTransactionId(3), unsupportedCrtValues(4)，
							unsupportedRemoteOperationType(5), unsupportedProfileClass(6), 
							scp03tStructureError(7), scp03tSecurityError(8), 
							installFailedDueToIccidAlreadyExistsOnEuicc(9), 
							installFailedDueToInsufficientMemoryForProfile(10), 
							installFailedDueToInterruption(11), installFailedDueToPEProcessingError (12), 
							installFailedDueToDataMismatch(13), testProfileInstallFailedDueToInvalidNaaKey(14), 
							pprNotAllowed(15), installFailedDueToUnknownError(127) }
*
*@param      :void
*
*@return      :errorReson 
*
*@author      :lgy
*
*@history      :V1.0
*/
#if RSP_EUICC
u8 IF_GetProfileExcuteErrorReason(void)
{
	return G_ProfileErrorReason;
}
#endif
				
/*
*function    : IF_GetFileSystemUsedSize
*
*@description:  获取文件系统的已占用空间
				
*
*@param      :void

*
*@return     :FileSystem 的已占用空间
*
*@history    :V1.0 by xuyz @20190310
*/
memref IF_GetFileSystemUsedSize(void)
{
	return (IF_GetFileSystemEndAddr() - ADDR_PRIVATE_FILE_START);	
}
/*
*function    : IF_GetRAMProfileIndex
*
*@description:  获取当前执行RAM的ProfileIndex
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by lgy@20190318
*/
u8 IF_GetRAMProfileIndex(void)
{
#if M2M_EUICC
	if(G_TARRecIndex[0] == TarType_ISDP)
	{
		return G_TARRecIndex[1];
	}
#endif	
	return ILLEGAL_PROFILEINDEX;
}

	
/*
*function    :IF_SetRefreshFlag
*
*@description:当前执行了profile切换的相关操作，后续需要发送refresh(Reset)指令，此处将标记打上
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 created by xxxx @20190417
*/
#if M2M_EUICC
void IF_M2M_SetRefreshFlag(void)
{
	PendingMessageAlert(D_RefreshTag,0);	
}

/*
*function    :IF_M2M_SetNotificationPara
*
*@description:通知确认时对相关全局变量清空操作
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 created by jiangqb @20190426
*/
void IF_M2M_NotificationConfirmation(void)			
{
	G_IsSendHttpsNotification = FALSE;	

	G_NotificationTimerId &= 0x0F;

	TimerModuleReaseTimerId(&G_NotificationTimerId);
}


/*
*function    :IF_GetScp80KeyVersionNum
*
*@description:当前通过RAM方式执行M2M_DeleteCommand_FinaliseISDRhandover指令时，需要知道当前正在使用的密钥是否是保留不删除的密钥
*@see        :
*
*@return     :u8 keyVersionNum(当前使用的scp80 KIC密钥版本)/FF(当前不是SCP80会话 或者不需要使用SCP80密钥)
*
*@history    :V1.0 created by xxxx @20190508           
*/
u8 IF_GetScp80KeyVersionNum(void)
{
	return G_RFMRAMKeyversion;
}
#endif

/*
*function    :IF_CurrentTARIsMNOSD
*
*@description:判断当前RAM的TAR所指示的APP是否为MNOSD，如果是的话则返回TRUE，否则返回FALSE; 其中RAM包含两种情况：一种是短信格式的RAM，一种是HTTP格式的RAM
*@see        :
*
*@param      :void
*
*@return     :TRUE表示当前RAM的执行者是MNOSD/FALSE表示当前RAM的执行者是SSD
*
*@history    :V1.0 created by jiangqb @20190508     
*注意        :实现时，请考虑短信格式的RAM, HTTP格式的RAM, CATTP格式的RAM
*/
u8 IF_CurrentTarSDIsMNOSD(void)
{
	if(G_TARRecIndex[1] == ISD_APPID)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
*function    :IF_GetScp80Dek
*
*@description:当前通过RAM方式执行put key指令时，可能需要用到对应的DEK来解密得到明文密钥.
*@see        :<GPC_SE_Configuration_v1.0.pdf>When the targeted Security Domain has a SCP80 Key Set, the security of the remote APDU command string is unwrapped by the Security Domain itself and processed as specified in [2].  Any attempt to initiate a SCP02 Secure Channel within the command script shall be rejected
*            :If PUT KEY or STORE DATA is used within a Secure Channel according to SCP80, the provisions of ETSI TS 102 226 [2]  shall apply. For creating or updating keys with a KVN greater than '0F', the DEK of the Secure Cha nnel Key Set shall be used. It shall not be possible to use the DEK key type single DES to update or create triple DES keys or RSA private keys, if any
*            :<ts_102226v130000p.pdf>
*
*@param      :u8* dek    输出参数 --- 获取到的DEK密钥数据16字节，后续如果发现DEK的数据长度超过16字节时，该接口及使用到DEK的地方都需要改动
*@param      :u8 keyLen  输入参数 --- 读取到dek中有效数据的长度
*
*@return     :void
*
*@history    :V1.0 created by xxxx @20190430
*
*             V1.1 实现函数，注意dek数组的长度必须保证能放下kik key，by xwh 20190505
*/
u8 IF_GetScp80Dek(u8* dek,u8 keyLen)
{
	return (GetKicKidKikKey((G_RFMRAMKeyversion<<4)&0xF0,dek,TYPE_KEY_KIK,keyLen));
}
/*
*function    :IF_IsProfileLoadingSession
*
*@description:当前是否在Profile下载过程中，对于个人版 只要进行了GetEuiccChallenge就认为进入了下载流程 （不考虑GetEuiccChallenge是为了认证DS服务器的情况），对于设备版 执行了第一条86指令认为进入了下载profile流程
*@see        :该函数的目的是为了限制在下载profile过程中又执行非Profile内部的安装 load cap 和删除等指令
*
*@param      :void
*
*@return     :u8 TRUE/FALSE
*
*@history    :V1.0 created by lgy @20190710
*
* 
*/
u8 IF_IsProfileLoadingSession(void)
{
#if RSP_EUICC
	if(G_RSPSessionContext.sessionSate >= RSPSessionState_Init)
	{
		return TRUE;
	}
#else
	if(G_DownLoadingProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		return TRUE;
	}
#endif
	
	return FALSE;
}
/*
*function    :IF_ProfielChangeOngoing
*
*@description:当前是否正在进行profile切换
*@see        :
*
*@param      :void
*
*@return     :u8 TRUE/FALSE
*
*@history    :V1.0 created by lgy @20190712
*
* 
*/
u8 IF_ProfielChangeOngoing(void)
{
#if RSP_EUICC
	if(G_LpaSendRefresh != LPANoRefreshMode)// LPAChangeMode和LPAReseteMode这两种模式 会发送主动式指令 中间可能会收到其他指令 导致某些指令在主动式命令之间处理
	{
		return TRUE;
	}
#endif

	return FALSE;
}
/*
*function    :IF_ProfileLoadingOrChangeOngoing
*
*@description:当前是否处于profile下载过程中，或者处于profile状态切换过程中
*
*@param      :void
*
*@return     :TRUE表示处于profile下载过程中，或者处于profile状态切换过程中；FALSE表示不在这两个过程中
*
*@history    :V1.0 created by xuxiaojun @20230308
*/
u8 IF_ProfileLoadingOrChangeOngoing(void)
{
	u8 result = 0x00;

	result = IF_IsProfileLoadingSession();

	if(result == FALSE)
	{
		result = IF_ProfielChangeOngoing();
	}

	return result;
}
/*
*function    :IF_IsGetResponseOngoing
*
*@description:判断当前是否处于RSP获取数据的C0过程中
*
*@param      :void
*
*@return     :true表示当前处于该过程中；否则返回false
*
*@history    :V1.0 created by xuxiaojun @20230330
*/
u8 RSP_IsGetResponseOngoing(void)
{
	if(G_SendFileExtendBackDataStruct.currentChannel != ILLEGAL_CHANNEL)
	{
		return TRUE;
	}

	return FALSE;
}
/*
*function    :IF_CapLoaingInCurrnetProfile
*
*@description:判断当前是否处于当前激活profile下的load过程中
*
*@param      :void
*
*@return     :TRUE表示处于当前激活profile下的load过程中；FALSE表示不在这个过程中
*
*@history    :V1.0 created by xuxiaojun @20230308
*/
u8 IF_CapLoaingInCurrnetProfile(void)
{
#if RSP_EUICC
	if((PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)&&(G_RSPSessionContext.sessionSate == RSPSessionState_NULL))   //在当前激活的profile下进行卡片内容管理时，不允许开启一个新的下载profile流程（COS内部方案 无规范来源）
	{
		return TRUE;
	}
#endif

	return FALSE;
}
/*
*function    :IF_MoreCommandsInCurrnetProfile
*
*@description:判断当前是否处于当前激活profile下的级联CHAINING_PUTKEY/CHAINING_INSTALL/CHAINING_DELETE/CHAINING_STOREDATA过程中
*
*@param      :void
*
*@return     :TRUE表示处于当前激活profile下的级联过程中；FALSE表示不在这个过程中
*
*@history    :V1.0 created by xuxiaojun @20230308
*/
u8 IF_MoreCommandsInCurrnetProfile(void)
{
	//当前是下载profile内部的store data, 因为有的profile内的store data指令不规范
	if((bRamBuffer[PROFILE_APDU_MODE] == 1)&&(g_moreCommands == CHAINING_STOREDATA))
	{
		return FALSE;
	}

	return (g_moreCommands == CHAINING_CLEAR)?FALSE:TRUE;
}
/*
* modify by xxj@20240128 将RSP_ClearSession中删除正在下载profile的操作提到外面，原因是放在函数内部存在以下问题
*                        1.如果当前下载过程未结束时终端复位会导致垃圾回收时虚拟机还没有初始化，死卡； 
*                        2.调用RSP_ClearSession之前以及删除了当前正在下载的profile，再次调用出现无效操作
*/
void RSP_CloseChannel(u8 managedChannel)
{
	u8 profileIndex;

	profileIndex = getChannelProfileIndex(managedChannel);

	if((G_CurExecutingProfileIndex != profileIndex)&&(getChannelContext(managedChannel)!=NULL_CONTEXT))
	{
		G_CurExecutingProfileIndex = profileIndex;
		ExecuEnvironmentInit(G_CurExecutingProfileIndex);// 关通道 需要调用desect方法
	}
	setNextProfileIndex(ILLEGAL_PROFILEINDEX); //clear

	if(G_RSPSessionContext.channelNum == managedChannel) // 表示当前RSPSession会话所在的通道被关闭 //modify by xxj@20240327 兼容祥承和ifree，下载profile时，如果进入到BF36时，关通道时需要清空会话和profile数据
	{
		if((G_RSPSessionContext.downloadProfileIndex != ILLEGAL_PROFILEINDEX)&&(VMReadByte(PROFILE_VALIDTAG_CFG + G_RSPSessionContext.downloadProfileIndex) == PROFILE_LOADINGFLAG))  // add bychentao@20240102--30.eSIM_plan_admend_test\用例名称:  eSIM_plan_admend_test_17_01  profile下载过程中86没有发送完LPA关闭通道，需要回收空间
		{
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
			{
				GPLoadInit();
			}

			DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);//回滚ISD-P  回滚profile等相关数据

			RSP_ClearSession();
		}
	}
}
/*
*function    :IF_ProfielDeletebackFileName
*
*@description:删除profile时候备份当前通道的 DF/Ef name, Memory Reset时不需要：因为Memory Reset时如果有激活的profile，则会设备复位
*@see        :
*
*@param      :fileName  当前逻辑通道的DF/EF name
*
*@return     :void
*
*@history    :V1.0 created by LCZ @20200609
*
*
*/
void IF_ProfielDeletebackFileName(u16 fileName[PROFILEDELETE_BACKUPFILESYSTEM])
{
	BackDFNameForMoveFalsh(fileName);
}
/*
*function    :IF_ProfielDeleteinit
*
*@description:删除profile时候变量等初始化, Memory Reset时不需要：因为Memory Reset时如果有激活的profile，则会设备复位
*@see        :
*
*@param      :fileName  当前逻辑通道的DF/EF name
*
*@return     :void
*
*@history    :V1.0 created by LCZ @20200609
*
*
*/
void IF_ProfielDeleteinit(u16 fileName[PROFILEDELETE_BACKUPFILESYSTEM])
{
	G_CurMFFileStart = GetMFFileStart();

	SEEUICCVariableInit(MODE_RESET);

	ReloadDFOrEFAddr(fileName);

	G_EnvelopHanderAddr = SelectPrivateTFFile(DF_Toolkit,EF_EnvelopeHandlerBuf);

}
/*
*function    :IF_RedTea_StoreDataTagBF7F
*
*@description:红茶移动的处理函数
*@see        :
*
*@param      :void
*
*@return     :
*
*@history    :V1.0 created by LCZ @20190718
*
*/
#if RSP_EUICC
#if EUICC__FUNCTION_RedTea_Mobile
void IF_RedTea_StoreDataTagFF7F(void)
{
	CheckStoreDataTagFF7F();
}

/*
*function    :IF_RedTea_Fallback
*
*@description:红茶移动的强制Fallback技术接口实现
* 
*@see        :TPX3.0_EUICC\doc\03 需求规范\红茶移动\FallbackStoredata - V2.docx   DF20 03 800100 enableProvisioningProfile
*
*@param      :void
*
*@return     :
*
*@history    :V1.0 created by xuxiaojun@20190725
*
*/
/**********Red Tea接口Fallback Response 的Result 宏定义************/

#define RedTea_Fallback_OK                                         0x00
#define RedTea_Fallback_PROFILE_NOT_IN_DISABLED_STATE              0x01
#define RedTea_Fallback_PROFILE_NOT_FOUND                          0x02
#define RedTea_Fallback_COMMAND_NOT_SUPPORTED                      0x03
#define RedTea_Fallback_COMMAND_FAILED                             0x04
#define RedTea_Fallback_UNDEFINED_ERROR                            0x05

/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void IF_RedTea_Fallback(void)
{
	u8 X_A enableProvisiong[6];
	u8 I_A profileNum, lastOperationalProfile, currentProfileClass;
	u8 X_A profileIndex[PROFILE_MAX_NUMBER];
	
	enableProvisiong[0] = 0xFF;	
	enableProvisiong[1] = 0x20;	
	enableProvisiong[2] = 0x03;	
	enableProvisiong[3] = 0x80;	
	enableProvisiong[4] = 0x01;	
	enableProvisiong[5] = 0x00;
	
	if((P3 != sizeof(enableProvisiong)) || (ecpMem_Compare(RecData, enableProvisiong, P3) != EQUAL))
	{
		CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_COMMAND_NOT_SUPPORTED);
		
		return;
	}
	
	if((P1 != 0x91) || (P2 != 0x00))
	{
		CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_COMMAND_NOT_SUPPORTED);
		
		return;
	}
	
	if((STKPerform[0] == TRUE) ||( G_STKBusy != FALSE))  //if there is a proactive session ongoing, the eUICC SHALL terminate the EnableProfile command with error "catBusy". 
	{
		CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_COMMAND_NOT_SUPPORTED);
				
		return;
	}
	
	profileNum = GetProfileIndexByProfileClass(PROFILE_CLASS_PROVISIONING, profileIndex);
	
	if(profileNum == 0)
	{
		CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_PROFILE_NOT_FOUND);
		
		return;
	}
	
	if(profileNum > 1)
	{
		CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_COMMAND_FAILED);
		
		return;
	}
	
	if(GetProfileState(profileIndex[0]) != PROFILE_STATE_DISABLED)
	{
		CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_PROFILE_NOT_IN_DISABLED_STATE);
		
		return;
	}
	
	lastOperationalProfile = JcvmConfig_GetRSPLastOperationaProfile();//VMReadByte(EUICCLastOperationaProfile_Addr) ;
	
	if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)  //当前存在有效的profile
	{
		currentProfileClass = GetSingleBYTEProfileMeta(G_CurEnabledProfileIndex, MetaDataProfileClass_INDEX,2) ;  //获取当前enable的profile的类别
		
		if(currentProfileClass == PROFILE_CLASS_TEST) //If the currently Enabled Profile is a Test Profile，that the target Profile is (another) Test Profile, or  if an Operational Profile was in Enabled state before the (first) Test Profile was enabled, that the target Profile is this Operational Profile. 
		{
			if((lastOperationalProfile != ILLEGAL_PROFILEINDEX)&&(profileIndex[0] != lastOperationalProfile))
			{
				CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_COMMAND_FAILED);
				
				return;
			}
		}			
	}
	
	//SEUICC_SynSEForSingleProfile(profileIndex[0],ModeSyn_CheckTag);
	
	CUSTOMER_SendSingleBYTEResult(0xFF20,RedTea_Fallback_OK);
	
	ProfileChangeEnable_GernerateNotifyAndSetState(profileIndex[0], PROFILE_CLASS_PROVISIONING, MODE_RESET);
	
	IF_SendRefreshCommandOfUICCResetMode(); //发送refresh 主动式命令 ，reset mode
}
#endif
#endif	

#endif //endif  #if (EUICC_ONLY||SE_AND_EUICC)

#if GPAPI_SUPPORT_SECURECHANNELINTERFACE_WITH_SCP8x
/*
*function    :IF_GetScp80SecurityLevel
*
*@description:当前通过RAM方式执行SecureChannel.getSecurityLevel时，需要获取到secure channel security level
*@see        :<UICC_Configuration_v2.0.pdf>
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 created by xxxx @20210128
*/
u8 IF_GetScp80SecurityLevel()
{
	u16 I_A lev;

	lev = NO_SECURITYLEVEL;

	if((G_SPIKICKID[0]&0x03) == 0x02)    // SPI1  SPI2 KIC KID  SPI.B1.b2 is set to 1, SPI.B1.b1 is set to 0
	{
		lev = SECURITYLEVEL_AUTHENTICATED | SECURITYLEVEL_CMAC;
	}

	if((G_SPIKICKID[0]&0x04) == 0x04) // C_DECRYPTION: bit SPI.B1.b3 is set to 1
	{
		lev |= SECURITYLEVEL_CDECRYPTION;
	}

	if((G_SPIKICKID[1]&0x0C) == 0x08) // R_MAC: bit SPI.B2.b4 is set to 1, SPI.B2.b3 set to 0
	{
		lev |= SECURITYLEVEL_RMAC;
	}

	if((G_SPIKICKID[1]&0x10) == 0x10) // R_ENCRYPTION: bit SPI.B2.b5 is set to 1
	{
		lev |= SECURITYLEVEL_RENCRYPTION;
	}
	return lev;
}
/*
*function    :IF_GetScp80SecurityLevel
*
*@description:当前通过RAM方式执行SecureChannel.getSecurityLevel时，需要获取到secure channel security level
*@see        :<UICC_Configuration_v2.0.pdf> <GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf>
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 created by xxxx @20210128
*/

u8 IF_GetScp81SecurityLevel(void)
{
	u16 I_A lev;

	lev = NO_SECURITYLEVEL;
#if GP__AMENDMENT__B__FOR_MASK
	switch(G_TLSSecurityParameters.cipherSuite)
	{
		case TLS_PSK_WITH_NULL_SHA:
		case TLS_PSK_WITH_NULL_SHA256:
			lev = SECURITYLEVEL_AUTHENTICATED | SECURITYLEVEL_CMAC | SECURITYLEVEL_RMAC;
			break;
		case TLS_PSK_WITH_3DES_EDE_CBC_SHA:
		case TLS_PSK_WITH_AES_128_CBC_SHA:
		case TLS_PSK_WITH_AES_128_CBC_SHA256:
			lev = SECURITYLEVEL_AUTHENTICATED | SECURITYLEVEL_CMAC | SECURITYLEVEL_CDECRYPTION | SECURITYLEVEL_RMAC | SECURITYLEVEL_RENCRYPTION;
			break;
		default:
			break;
	}
#endif	
	return lev;
}

#endif




/*@Function:    IF_UpdateSeidFile
 *
 *@Descrip:     对于USIM卡v3.0+ (国密卡)产品，SEID文件必须存在，编码格式遵循《中国移动用户卡应用基础能力技术规范》，且SEID文件中的值需与GetData指令获取的SEID的值一致。
 *
 *@See:	        ts_143019v050600p.pdf 6.2
 *
 *@Param:       tag   ---
 *              seidBuf---RFU
 *
 *@Return:      NULL
 *
 *@Author:      XWH 20220810
 *
 *@History:     V0.0---Create by XWH 20220810
 *
 *
 */
void IF_UpdateSeidFile(u16 tag,u8* seidBuf)
{
#if UICC__OPERATOR__CHINAMOBILE
	UWORD idata fileAddr;

	if((tag == 0x0044)&&(seidBuf[0] == 0x93)&&(seidBuf[1] == 0x0A))
	{
		fileAddr = SelectTFFile(0x3F00,0x2FF0);

		if(fileAddr != FALSE)
		{
			WriteTFBuf(fileAddr,0x00,&seidBuf[2],seidBuf[1]);
		}
	}
#endif
}
/*
 @Function:   IF_GetDCEPID
 *
 *@Descrip:   移动产品，出厂时不写ICCID,storedata无法预置ICCID的值，getdata时从2FE2文件中读取IDDIC
 *
 *@See:	    中国移动USIM 3.0+产品技术要求 V1.0.5
 *
 *@Param:     PX8 buff --- out,ICCID文件值反转之后
 *
 *@Return:    void
 *
 *@History:   V0.0---Create by yjr@20221013
*/
void IF_GetDCEPID(PX8 buff)
{
#if UICC__OPERATOR__CHINAMOBILE
	memref fileAddr;
	u32    ICCIDSize;
	u8     result,i;

	fileAddr = SelectTFFile(DF_MF, EF_ICCID);

	if (fileAddr == FALSE)
	{
		throw_error(SW_REFERENCED_DATA_NOT_FOUND);
	}

	result = ReadTFBuf(fileAddr,0,buff,0x0A);

	if(result == FALSE)
	{
		return;
	}

	for(i=0;i<0x0A;i++)
	{
		buff[i] = (buff[i]<<4)|(buff[i]>>4);
	}
#endif
}

/*
*@function     :IsUiccApp
*
*@description  :If is UICC app
*
*@see          :
*
*@param        :NULL
*
*@return       :TRUE:is UICC app/FALSE:not UICC app
*
*
*@author       :xwh@20220519
*
*@hisotry      :created by xwh@20220519
*				：modify by jiangqingbin-20221206, 同步 EPX\v2.1\BRANCH_MODULE\RSP_hongcha_STM_ST33G1M2\DEVELOP\的代码, 由于本平台进行了代码分层,本函数需要在java/native的接口函数中分别定义
*              
*/
u8 IsUiccApp(u8  channelNum)
{
#if JAVA_CARD_UICC	
	if(U8_AND_U8(U16t8(getChannelContext(channelNum)), 0xFF)==UICC_APPID)	
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else
	return TRUE;
#endif
}



/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	ExtendMapTable                                                                                                                
                                                                                                                                                
  Input parameter: void
                                                                                              
  Output parameter: void                                                                                                  
                                                                                                                                                
  Description:   当index已经满时，用来扩展map表，一次扩展一页，文件系统往后移动一页

* @author       :zhxx@20220714
*
* @hisotry      :created by zhxx@20220714
*/    
#if UICC_NEW_FILESYSTEM

void ExtendMapTable(void)
{	
	u32 fileSystemStart,fileSystemDestAddr,fileSystemTotalSize,mapNum,defaultFileSystemAddr;
	MapTableExtend mapExtend;
	
#if (EUICC_ONLY || SE_AND_EUICC)
	u8 xdata temp[PROFILE_MAX_NUMBER];
	u32 currentMFOffset;
	u8 i;
#endif
	
	fileSystemStart = ADDR_PRIVATE_FILE_START;//获取文件系统的起始源地址
	
	fileSystemDestAddr = (fileSystemStart+EEPAGE_SIZE);//文件系统的起始目的地址，向后移动一页（0x200）
	
	defaultFileSystemAddr = VMReadUword(RSPDefaultFileSystem_Addr);//get RSPDefaultFileSystem_Addr
	
	fileSystemTotalSize = GetFileSystemTotalSize();//获取文件系统的总大小，即移动的数据总长度
	
	mapNum = sysConfig_GetMaptable_num();//获取map表的页数
	
	mapNum++;  	
	
	ecpMem_Fill(&mapExtend,0,sizeof(mapExtend));
	
	mapExtend.newMapNum = mapNum;
	
	if(fileSystemTotalSize > 0)
	{		
		mapExtend.moveTag = D_EEPMoveTag;
		mapExtend.srcAddr = fileSystemStart + fileSystemTotalSize;
		mapExtend.destAddr = fileSystemDestAddr + fileSystemTotalSize;
		mapExtend.moveLen = fileSystemTotalSize;
		
		mapExtend.newDefaultFileSystemAddr = defaultFileSystemAddr + EEPAGE_SIZE;
		//VMReadBuf((BYTE*)mapExtend.ProfileNewMFOffset,PROFILE_MFOFFSET_CFG,PROFILE_MAX_NUMBER*sizeof(UWORD)); 
		
#if (EUICC_ONLY || SE_AND_EUICC)
		VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER); 
		
		for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
		{		  
			if(temp[i] != PROFILE_INVALIDFLAG) //判断profile是否有效
			{
				currentMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(UWORD)); 
				
				if((currentMFOffset != 0xFFFFFFFF)&&(currentMFOffset != 0x00000000))
				{					
					mapExtend.ProfileNewMFOffset[i] = currentMFOffset + EEPAGE_SIZE;					
				}
			}
		}
#endif
		WriteEepromEX(E_MAPEXTEND_START, (BYTE*)&mapExtend, sizeof(mapExtend));
		
		G_CurMFFileStart += EEPAGE_SIZE;//将当期激活的proifle的MF地址增大一页
	
	    G_CreatingMFOffset += EEPAGE_SIZE;//downloading的proifle的MF地址增大一页
		
				
			
	}
	else//创建文件系统之前map表已经不够，只需要扩展map表即可
	{
		sysConfig_SetMaptable_num(mapExtend.newMapNum);
	}
	
		
	StartSecureMoveEepromMapTableExtend();
	
	
}
#endif



/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	IF_GetAvailableMapIndex                                                                                                                
                                                                                                                                                
  Input parameter: u16 i,u16 * para_gNvmMap_AppStartIndex,u16 * para_gNvmMap_TopIndex,u16 * para_gNvmMap_LastFindIndex,	u1 isFind
                                                                                              
  Output parameter: i                                                                                                  
                                                                                                                                                
  Description:   get available map index

* @author       :zhxx@20230109
*
* @hisotry      :created by zhxx@20230109
*/
u16		IF_GetAvailableMapIndex(u16 i,u16 * para_gNvmMap_AppStartIndex,u16 * para_gNvmMap_TopIndex,u16 * para_gNvmMap_LastFindIndex,	u1 isFind)
{
	#if UICC_NEW_FILESYSTEM
		
		if(((i -(*para_gNvmMap_AppStartIndex))<U8_M_U16(MAP_BITMAP_ENTRY, MAP_BITMAP_SIZE)) && (AllocSpaceEnough(HAL_EEPAGE_SIZE) == TRUE))//bitmap enough,且剩余空间大于一页，才进行扩展
		{
			ExtendMapTable();//如果找不到index 项，扩展map表
		
			if(false == isFind)
			{
				U16_INC(para_gNvmMap_TopIndex); //TODO mwp: g_mapTopIndex 要做边界测试。
			}

			*para_gNvmMap_LastFindIndex	= i;
		}
		else//bitmap not enough
		{
			 i	= MAP_TABLE_ILLEGAL_INDEX;
		}
		
		
	#else
		
		   i	= MAP_TABLE_ILLEGAL_INDEX; //TODO mwp: 如果找不到index 项，要不抛出异常，要不返回NULL(0).
	#endif
		
		return i;
}
