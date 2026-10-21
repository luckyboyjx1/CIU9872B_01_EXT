/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   SEEUICCInterface.c
*
* Description: 由于该平台支持SE&EUICC管理机制，该文件主要存放SE与EUICC共同使用的一些接口函数 ，
*
* author：     xxxx    xxxx.xx.xx
*
* history：    V1.0
*
************************************************************************************************************/
#include "SEEUICCInterface.h"
#include "interpreter.h"
#include "cardManager.h"
#include "rtrSpace.h"
#include "tranManager.h"
#include "object.h"
#include "ob_manager.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "Interface_LTEFromPlatform.h"
#include "profileApi.h"

#if SE_ONLY // modify by xuxiaojun@20220315 从EUICCMain.c中移动到这个文件来，因为如果不支持通信功能的话，放在原来的位置编译不过
ExecuEnvironment const G_CurExecuEnvironment = {

	MEMtPN8(AM_theAppTable_JC), //
	MEMtPN8(AM_bFreeEntryIndex_JC),
	MEMtPN8(iAppIdFlag_JC),
	MEMtPN8(iMutiFlag_JC),
	MEMtPN8(iExtraditonFlag_JC),
	MEMtPN8(PM_f_pkgTable_JC), //??PackageMgrCommit,???Pkg_Tabel_Object_Cash
	MEMtPF8(PM_pkgStatus_JC),
	MEMtPF8(PACKAGE_LOCATION_TABLE_ADDRESS),
	MEMtPN8(PM_packageContextTable_JC),
	MEMtPN8(PM_appletPkgCount_JC),
	MEMtPN8(PA_DefaultAppID_JC),
	MEMtPN8(OP_bCardState_JC)
};
#else
ExecuEnvironment X_A G_CurExecuEnvironment;
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
u8  X_A G_CurExecutingProfileIndex;
u8  X_A G_NextExecutingProfileIndex;
u8  X_A G_BackCurEnableProfileIndex;  //备份当前激活的profileIndex，在新安装profile过程中使用
u8  X_A G_IsResumeISDPInfoMode;      //判断是否要恢复profile的配置区域
#endif

/*切换包表相关的全局变量到执行的区域 输入：0xFF表示切到SE区执行  其他表示切到对应的profile区执行*/
u8 ExecuEnvironmentInit(u8 ProfileIndex)
{
#if (EUICC_ONLY||SE_AND_EUICC)
	memref      X_A     profileEntryAddr;
	
	if(ProfileIndex == ILLEGAL_PROFILEINDEX)
	{
		G_CurExecuEnvironment.AppTable = MEMtPN8(AM_theAppTable_JC); //
		G_CurExecuEnvironment.bFreeEntryIndex = MEMtPN8(AM_bFreeEntryIndex_JC);
		G_CurExecuEnvironment.AppIdFlag = MEMtPN8(iAppIdFlag_JC);
		G_CurExecuEnvironment.MutiFlag = MEMtPN8(iMutiFlag_JC);
		G_CurExecuEnvironment.ExtraditonFlag = MEMtPN8(iExtraditonFlag_JC);
		G_CurExecuEnvironment.pkgTable = MEMtPN8(PM_f_pkgTable_JC); //参考PackageMgrCommit，是否用Pkg_Tabel_Object_Cash
		G_CurExecuEnvironment.pkgStatus = MEMtPF8(PM_pkgStatus_JC);
		G_CurExecuEnvironment.pkgLocationTable = MEMtPF8(PACKAGE_LOCATION_TABLE_ADDRESS);
		G_CurExecuEnvironment.packageContextTable = MEMtPN8(PM_packageContextTable_JC);
		G_CurExecuEnvironment.appletPkgCount = MEMtPN8(PM_appletPkgCount_JC);
		G_CurExecuEnvironment.defaultAppIDAddr = MEMtPN8(PA_DefaultAppID_JC);
		G_CurExecuEnvironment.CardStateAddr = MEMtPN8(OP_bCardState_JC);

	}
	else
	{
		profileEntryAddr = GetProfileEntryAddr(ProfileIndex);

		if(profileEntryAddr == ADDR_IS_NULL)
		{
			return  FALSE;
		}
		G_CurExecuEnvironment.AppTable = MEMtPF8(GetProfile_theAppTableAddr(profileEntryAddr));
		G_CurExecuEnvironment.bFreeEntryIndex = MEMtPF8(GetProfile_bFreeEntryIndexAddr(profileEntryAddr));
		G_CurExecuEnvironment.AppIdFlag = MEMtPF8(GetProfile_iAppIdFlagAddr(profileEntryAddr));
		G_CurExecuEnvironment.MutiFlag = MEMtPF8(GetProfile_iMutiFlagAddr(profileEntryAddr));
		G_CurExecuEnvironment.ExtraditonFlag = MEMtPF8(GetProfile_iExtraditonFlagAddr(profileEntryAddr));
		G_CurExecuEnvironment.pkgTable = MEMtPF8(GetProfile_pkgTableAddr(profileEntryAddr));
		G_CurExecuEnvironment.pkgStatus = MEMtPF8(GetProfile_pkgStatusAddr(profileEntryAddr));
		G_CurExecuEnvironment.pkgLocationTable = MEMtPF8(GetProfile_pkgLocationTableValueAddr(profileEntryAddr));
		G_CurExecuEnvironment.packageContextTable = MEMtPF8(GetProfile_packageContextTableAddr(profileEntryAddr));
		G_CurExecuEnvironment.appletPkgCount = MEMtPF8(GetProfile_appletPkgCountAddr(profileEntryAddr));
		G_CurExecuEnvironment.defaultAppIDAddr = MEMtPN8(GetProfile_defaultAppIDAddr(profileEntryAddr));
		G_CurExecuEnvironment.CardStateAddr = MEMtPN8(GetProfile_bCardStateAddr(profileEntryAddr));
	}
#endif

	APP_Table_Cash = REF_NULL;
	Pkg_Tabel_Cash = REF_NULL;

	return TRUE;
}
#if (EUICC_ONLY||SE_AND_EUICC)
u8 EUICC_GetEID(PX8 outBuf)
{
	u8    X_A     len;
	len     =     nvmReadU8(MEMtPM8(EUICCEIDInfo_Addr));
	VMReadBuf((BYTE*)&outBuf[0], EUICCEIDInfo_Addr + 1, len); 
	return len;	
}
/*
*@function   :SEEUICCVariableInit
*
*@description:初始化SE 的一些全局变量,在开机或切换profile，进入新的profile安装状态时，该函数要被调用
*
*@param      :u8 mode --- 当前是什么模式下调用该接口
*
*@return     :void
*
*@author     :xuyz
*
*@history    :V1.0 create 21
*            :V1.1 modify by xuxiaojun@20190625 profile切换时通道信息初始化
*/
void SEEUICCVariableInit(u8 mode)
{
	u8 I_A isChangeProfileInitFile;

	isChangeProfileInitFile = JcvmConfig_GetUpdateChangeProfileInitPrivateFileIndex();

	if(isChangeProfileInitFile == PROFILE_PrivateFileInit)
	{
		IF_ChangeProfileInitPrivateFile();

		JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileNoInit);   //初始化需要初始化文件的标志为无效标志
	}

	ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	
	if(mode == MODE_PROFILE_CHANGE)  // profile发生状态切换之后，需要对可能的GP全局变量重新赋值, 参考GPVariableInit函数. 需要考虑如果当前在通道1上选择ISD-R之后，往SE区域进行操作的情况
	{
		//if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)
		if(ProfileIndexIsValid(G_CurEnabledProfileIndex))
		{
			if(RamManager_refHasBeenAllocated(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_ENABLE_SPACE_ID))) != INVALID_ALLOC_RAM_ADDRESS)
			{
				RamManager_free(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_ENABLE_SPACE_ID)));
			}
			RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_ENABLE_SPACE_ID)), tranMananger_getProfileRtp(G_CurEnabledProfileIndex), false);
		}
		else
		{
			RamManager_free(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_ENABLE_SPACE_ID)));
		}

		//	ChannelManagerInit(G_CurSelectedChannel);// Close all logical channels which still have an application of the currently enabled Profile selected, 未考虑SE应用
	}

}

/*
*@function :SEUICC_BackISDPAppletTable,
*
*@function :恢复ISD-P的信息，该函数在profile下载过程中不能调用（在pelist[G_PEIndex].func()前后调用，进入pe解析后所有包表信息已切到正在下载的profile，无需再切一次，否则还原时会提前把G_CurExecutingProfileIndex还原回去）
*
*@param    :void
*
*@return   :void
*
*@author   :xuyz
*
*@history  :V1.0
*/
void SEUICC_BackISDPAppletTable(u8 newIndex)
{
	if(newIndex == G_CurExecutingProfileIndex)
	{
		return;
	}
	G_BackCurEnableProfileIndex = G_CurExecutingProfileIndex;
	
	G_IsResumeISDPInfoMode = TRUE;
	
	G_CurExecutingProfileIndex = newIndex;
	
	SEEUICCVariableInit(MODE_DOWNLOAD_PROFILE);	
}
/*
*@function :SEUICC_ResumeISDPAppletTable,
*
*@function :恢复ISD-P的信息，该函数一般与 SEUICC_BackISDPAppletTable 配合使用
*
*@param    :void
*
*@return   :void
*
*@author   :
*
*@history  :V1.0 该函数放在java代码的finally语句，是因为在installforinstall过程中，可能会抛异常，此时也有可能要恢复ISD-P信息
*/
void SEUICC_ResumeISDPAppletTable(void)
{
	if(G_IsResumeISDPInfoMode == TRUE)
	{
		G_CurExecutingProfileIndex = G_BackCurEnableProfileIndex;
		
		G_IsResumeISDPInfoMode = FALSE;
		
		SEEUICCVariableInit(MODE_DOWNLOAD_PROFILE);	
	}
}
/*
*@function   : SEUICC_CheckISDRECASDAID
*
*@description: 检查安装应用的AID 不能和ISD R ECASD ISDP等重复
*
*@param    :   u8 appId
×@param    ：  u8 *aidValue
*@param    :   u8 aidLen
*
*@return    :  void
*
*@author    :  xuyz
*
*shitory
*/
u8 CONST ForbidenPreAID[] =  {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00};                //SGP02_V3.2 2.2.3  Identification of Security Domains: AID and TAR
u8 CONST ECASD_TAR[] = {0x00,0x00,0x02};
void SEUICC_CheckISDRECASDAID(void)
{
	u8						AidLen;
	u16 					AidObj,bufRef,off;
	memref 					AidAdd;
	u8                      tmp[0x10];

	AidLen    =  popShort();
	off    =  popShort();
	bufRef =  popShort();
	AidObj =  popShort();

	if(AidObj != REF_NULL)//注册时调用 传AID对象
	{
		AidAdd = locateObjectInMemory(AidObj);
		AidAdd = locateObjectInMemory(memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(AidAdd), AIDARRAY_INDEX_OF_AID))));
		AidLen = U16t8(get_array_length(MEMtPF8(AidAdd)));
		off = 0;

	}
	else//安装时调用 传数组
	{
		AidAdd = locateObjectInMemory(bufRef);
	}

	  //其他应用不能使用ISDP ISDR ECASD等应用的AID和tar值

	if(AidLen == 0x10)
	{
		AidAdd = U32_A_U16(get_array_data(MEMtPF8(AidAdd)),off);

		VMReadBuf(tmp, (memref)AidAdd, 0x10);

		if((ecpMem_Compare(tmp,(u8 *)ForbidenPreAID, 13) == EQUAL) && (ecpMem_Compare(&tmp[12],(u8 *)ECASD_TAR, 3) == EQUAL))
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED); // WQ_Offline_Test\TC_AID_Allocation_Not_ISDR_ECASD_ISDP_AIDS_Error_01:AID与ECASD冲突时期望返回状态字6a88/6a80/6985
													 // HB_Offline_Test/8安全性测试/Tele_TC5_AID_Allocation_TEST_01  用MNO SD的tar 安装一个应用 ，aid和ECASD的值相同时期望返回6985
		}

		if((ecpMem_Compare(tmp,(u8 *)ForbidenPreAID, 13) == EQUAL) && (tmp[15] == 0x00) )
		{
			ThrowError(SW_WRONG_DATA); // M2M版本UL套件测试用例：《TC_CV_5_AIDAllocation_03_Error_Case_Unauthorized_ECASD_AID_17_11_03》
													 // WQ_Offline_Test\TC_AID_Allocation_Not_ISDR_ECASD_ISDP_AIDS_Error_01:AID与ISD-R/ISD-P冲突时返回状态字6a88/6a80
		}

	}

}
/*
*@function   :IF_CheckSurviveKey
*
*@description:M2M_DeleteCommand_FinaliseISDRhandover的子函数，用于判断需要被保留的KEY是否是当前建立安全通道所使用的key，用key version判断
*@see        :Check that all keys of the key set(s) used for setting up the current secure channel are among the keys not to be deleted. For SCP81, this also includes the key set used for the push SM. If that check fails, the command is terminated without deleting any key.
*
*@param      :u8 keyVersion[2]  指定的key的版本号
*
*@return     :TRUE 是当前建立安全通道所使用的key；FALSE 不是
*
*@author     :xuxiaojun
*
*@history    :created by xuxiaojun@20181226
*/
#if M2M_EUICC

u8 IF_CheckSurviveKey(u8 keyVersion[2])
{
	u8 I_A currentKeyVersionNum;
	
	currentKeyVersionNum = IF_GetScp80KeyVersionNum();
	
	if(currentKeyVersionNum != 0xFF)//当前有正在使用的scp80密钥
	{
		if((currentKeyVersionNum != keyVersion[0]) && (currentKeyVersionNum != keyVersion[1]))//当前使用的密钥不是需要保留（不删除）的密钥 
		{
			return FALSE;
		}
	}
	
	currentKeyVersionNum = IF_GetScp81KeyVersionNum();
	
	if(currentKeyVersionNum != 0xFF)//当前有正在使用的scp81密钥
	{
		if((currentKeyVersionNum != keyVersion[0]) && (currentKeyVersionNum != keyVersion[1]))//当前使用的密钥不是需要保留（不删除）的密钥 
		{
			return FALSE;
		}
	}
		
	return TRUE;
}
#endif

#endif
/**
*modify by xuxiaojun@20220315 从platform_api.c文件中移动到该文件中，否则去掉UICC功能之后，编译不过
*/
u8 Cmac(u8 *key, u8 keylen, u8 *M, u16 MLen, u8 * headMsg, u8 headMsgLen, u8 *T)
{
	u8 X_A IVBuf[AES_BLOCKSIZE];

	ecpMem_Set(IVBuf, 0x00, AES_BLOCKSIZE);    //初始向量是Zero block

	symCmacAlgorithmCal(MEMtPM8(headMsg), headMsgLen, MEMtPM8(M), MLen, MEMtPM8(T), MEMtPM8(key), U8_M_U8(keylen,8), MEMtPM8(IVBuf), CBC_MAC_Encrypt, ALG_AES);

	return AES_BLOCKSIZE;
}
