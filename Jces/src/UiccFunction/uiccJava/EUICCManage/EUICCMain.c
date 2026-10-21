/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC的一些入口函数及所有全局变量的声明及定义
*
*@history     :V1.0 :create @ 20181106  
*/
#include "EUICCM2MAPDU.h"
#include "EUICCMain.h"
#include "EUICCCustomer.h"
#include "string.h"
#include "hard_api.h"
#include "EUICCPersonalAPDU.h"
#include "EUICCRAM.h"
#include "SEEUICCInterface.h"
//#include "Sha1.h"
#include "object.h"
#include "ob_manager.h"
#include "SCP03Realize.h"
#include "profile.h"
#include "Interface_LTEFromPlatform.h"
#include "EUICCM2MAPDUPublic.h"
#include "EUICCCustomerPublic.h"
#include "profileApi.h"
#include "EUICCProfile.h"

#if (EUICC_ONLY||SE_AND_EUICC)
BYTE  xdata G_HttpExecutingProfileIndex;//HTTP指令的执行区域， close channel时会清掉该变量，该值一旦赋上  每条http指令都需要切到该区域执行 ，暂时不支持twosession且在不同执行区域的情况的情况
BYTE  xdata G_RAMBackExecutingProfileIndex;  //备份RAM （短信模式）的执行区域  checktar过程中可能会从profile切到SE执行，该变量存的是最终执行的区域   FF表示SE区  其他值表示profileIndex
BYTE  xdata G_CurEnabledProfileIndex;   //当前激活的profile index ，RSP和M2M版按照实际值在开机或切换profile后设置

//SHA1_CTX  xdata G_LoadSha1Context;

//BYTE xdata G_ComponetCurrentStep;  //当前组件处理到第几步?
//BYTE xdata G_CapMinorVersion;	//缓存当前正在下载的包的次版本号
//BYTE xdata G_LoadTmpBufOffset;  //有效数据在G_LoadTmpBuf中的偏移
//UWORD xdata G_LoadComponentRemainLen;   //当前正在处理的组件的剩余长度
//BYTE xdata G_LoadRecOffset;  //当前正在处理的APDU数据的偏移
//BYTE xdata G_LoadTmpBuf[20];  //处理组件过程中用于缓存一些临时数据


//BYTE xdata G_CapTagInvalid;   //缓存当前正在处理的tag是有效tag还是非法tag。中国移动tck302要求过滤掉代码中的非法tag

#if EUICC__FUNCTION__GUOMI_eSIM
	BYTE xdata G_CertType ; //用于判断当前证书是国际证书还是国密证书，国际证书为0，国密证书为1，该变量初始值为0，默认为国际证书
#endif
/*
 * EUICCResponse ::= SEQUENCE {
peStatus SEQUENCE OF PEStatus,
profileInstallationAborted NULL OPTIONAL,
statusMessage UTF8String (SIZE (2..64)) OPTIONAL
}

PEStatus ::= SEQUENCE {
status INTEGER {
ok(0), pe-not-supported(1), memory-failure(2),bad-values(3),not-enough-memory(4),invalid-request-format(5), invalid-parameter(6),
runtime-not-supported (7), lib-not-supported (8),template-not-supported (9), feature-not-supported (10),pin-code-missing (11),unsupported-profile-version(31)
},
identification UInt15 OPTIONAL, -- Identification number of the PE triggering the error
additional-information UInt8 OPTIONAL,-- Additional information related to the status code
offset UInt31 OPTIONAL-- Position of the part of the PE generating this status code
}
目前pestatus拼装的信息为：
正常： A0053003800100
正常且带警告： A0XX +
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告1） ---- 13字节，82tag没有拼
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告2） ---- 13字节
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告3） ---- 13字节
				……
//				3003800100 ---- 5字节 ，正常且带警告的情况下 没有这个，peend没看到处理
错误不带警告： A0XX +
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （错误结果） ---- 13字节
		 8100    ------- profileInstallationAborted

错误且带警告：A0XX + 30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告1） ---- 13字节，
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告2） ---- 13字节
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告3） ---- 13字节
				……
				30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告5或错误结果） ---- 13字节
		 8100    ------- profileInstallationAborted

EUICCResponse的拼装结果为：30xx + pestatus ,其他两项没有拼装
*/
BYTE xdata G_EUICCResponse[72]; //LV结构 只能用于下载profile时存结果，不能借给其他模块使用！！！，最多拼装5个警告  ，按错误且带警告的情况开设数组长度 （13*5 + 6（30xx + a0xx + 8100）+ 1(总L)）  EUICC profile下载时用于缓存PEStatus的全局数组，不使用Resdata，使用Resdata会冲掉0通道上的主动式指令

/*
*@function   :EUICCInitGobalVariable
*
*@description: 开机复位时初始化EUICC 相关的全局变量
*
*@param      :void
*
*@return     :void
*
*@test       :
*
*@author     :xuyz
*
*@history    :V1.0 :create @20181108
*/
void EUICCInitGobalVariable(void)
{

	BYTE idata isUpdateEnableIndex;//,disableIndex;
	BYTE idata loop_counter;
	
#if M2M_EUICC
	G_DownLoadingProfileIndex = ILLEGAL_PROFILEINDEX;

	G_M2MKeySetState = FALSE;
	
	ecpMem_Set(G_M2MStoreData, 0xFF, sizeof(G_M2MStoreData));
#endif

	ecpMem_Set(G_EUICCResponse,0,sizeof(G_EUICCResponse));

	G_CAWrite = 0x00;
	
	G_CALength = 0x00;
	
	G_PersonalProfileState = 0;	
	G_PersonalProfileIndex = ILLEGAL_PROFILEINDEX;

#if EUICC__FUNCTION__GUOMI_eSIM
	G_CertType = Cert_ECC;
#endif

#if RSP_EUICC
	RSP_InitGobalVariable();
#endif

	G_BackCurEnableProfileIndex = ILLEGAL_PROFILEINDEX;
	
	G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;
	
	G_IsResumeISDPInfoMode = FALSE;
	
	isUpdateEnableIndex = JcvmConfig_GetUpdataEnableProfileIndex();
	
	//if(isUpdateEnableIndex != ILLEGAL_PROFILEINDEX) // 复位之前执行了enable profile的操作，refresh模式是reset模式，但是没有执行reset之前又把该profile给删掉了，导致该profile的entry不存在，程序却按照存在的情况继续往下走，访问了非法地址i
	if(ProfileIndexIsValid(isUpdateEnableIndex))
	{

		if(GetProfileEntryRef(isUpdateEnableIndex) == REF_NULL)  // 复位之前执行了enable profile的操作，refresh模式是reset模式，但是没有执行reset之前又把该profile给删掉了，导致该profile的entry不存在，程序却按照存在的情况继续往下走，访问了非法地址i
		{
			isUpdateEnableIndex = ILLEGAL_PROFILEINDEX;  // modify by chentao 20220209 激活复位后需要激活的profile不存在的话直接回到SE
		}

		JcvmConfig_SetProfileEnableIndex(isUpdateEnableIndex);

		JcvmConfig_SetUpdataEnableProfileIndex(ILLEGAL_PROFILEINDEX);// profile状态已切换完成，清除该状态
	}
#if RSP_EUICC 	//	个人版失效Profile以后 可能会存在 当前没有激活的profile的情况 ，此时需要清空PROFILE_ENABLEINDEX_CFG
	else
	{
		isUpdateEnableIndex = JcvmConfig_GetUpdataDisableProfileIndex();
		
		//if(isUpdateEnableIndex != ILLEGAL_PROFILEINDEX)
		if(ProfileIndexIsValid(isUpdateEnableIndex))
		{
			JcvmConfig_SetUpdataDisableProfileIndex(ILLEGAL_PROFILEINDEX);

			JcvmConfig_SetProfileEnableIndex(ILLEGAL_PROFILEINDEX);
		}
	}
#endif
	
	G_CurEnabledProfileIndex = JcvmConfig_GetProfileEnableIndex();

	if(ProfileIndexIsValid(G_CurEnabledProfileIndex) == FALSE)// add by chentao @20230313 G_CurEnabledProfileIndex如果为非法值，初始化到SE区，避免坏卡
	{
		G_CurEnabledProfileIndex = ILLEGAL_PROFILEINDEX;
		JcvmConfig_SetProfileEnableIndex(ILLEGAL_PROFILEINDEX);
	}

	G_CurExecutingProfileIndex = G_CurEnabledProfileIndex; //卡片第一次上电时，bRamBuffer还没有创建
	G_NextExecutingProfileIndex = G_CurEnabledProfileIndex; //

	G_RAMBackExecutingProfileIndex = G_CurEnabledProfileIndex;  // add by chentao 初始化20230221
	G_HttpExecutingProfileIndex = G_CurEnabledProfileIndex;
	for(loop_counter = 0; loop_counter < MAX_LOGICAL_CHANNELS; U8_INC(loop_counter))
	{
		setChannelProfileIndex(loop_counter, G_CurEnabledProfileIndex);
	}

	SEEUICCVariableInit(MODE_RESET);
}

/**/
UWORD GetArrayDataAddrByRefAddr(UWORD RefAddr)
{
	WORD  idata eidArrayRef;
	UWORD  xdata eidArrayAddr;

	eidArrayRef = memReadU16(MEMtPM8(RefAddr));

	if(eidArrayRef == REF_NULL)
	{
		return ADDR_IS_NULL;
	}

	eidArrayAddr = locateObjectInMemory(eidArrayRef);

	return get_array_data(MEMtPF8(eidArrayAddr));
}
#endif

/*
*@fucntion    :InitCardConfigArea
*
*@description :在卡片第一次下卡时，用于初始化卡片配置区的flash值
*
*@param       :void
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20181227
*             :V1.1 :modify by xuxiaojun@20190614  删除重复的初始化操作
*/
void InitCardConfigArea(void)
{
//#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH   //该操作在clearAllEeprom中实现，这里不再调用,by xwh 20230105
//	BYTE  idata num;
//	BYTE  idata RamBuffer[PFPageSize];
//#endif
#if (EUICC_ONLY||SE_AND_EUICC)
	JcvmConfig_SetProfileEnableIndex(ILLEGAL_PROFILEINDEX);

	JcvmConfig_SetUpdataEnableProfileIndex(ILLEGAL_PROFILEINDEX);

	JcvmConfig_SetUpdataDisableProfileIndex(ILLEGAL_PROFILEINDEX);

	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileNoInit);

	//SetEeprom(EUICCLastOperationaProfile_Addr, 0x00 ,EUICCConfigInfo_SIZE);//将EUICC Config Area配置区统一清成0 ，后续再根据不同配置项设置特殊值

	SetEeprom(PROFILE_VALIDTAG_CFG, 0x00 ,ProfileConfigInfo_SIZE);      //将Profile配置区统一清成0 ，后续再根据不同配置项设置特殊值

	SetEeprom(PROFILE_VALIDTAG_CFG, PROFILE_INVALIDFLAG, PROFILE_MAX_NUMBER);  //初始化所有profile的状态为invalid

	SetEeprom(PROFILE_MFOFFSET_CFG,0xFF,PROFILE_MAX_NUMBER*sizeof(UWORD));//PROFILE文件系统首地址初始化

	JcvmConfig_SetRSPLastOperationaProfile(ILLEGAL_PROFILEINDEX);//SetEeprom(EUICCLastOperationaProfile_Addr,   ILLEGAL_PROFILEINDEX,  0x01);

	SetEeprom(EUICC_OpKeyBuf, 0x00, sizeof(BYTE));  //清除otPk等信息	

	SetEeprom(EUICC_AuthenTmpBuf, 0x00, EUICC_AuthenTmpSize);//将EUICC_AuthenTmpBuf的值清成0	

#if M2M_EUICC
	//SetEeprom(M2MFallBackProfileIndex, ILLEGAL_PROFILEINDEX,  4);  //初始化 M2MFallBackProfileIndex 到  M2MUpdateEnableProfileIndex  区域的字段

	JcvmConfig_SetM2MFallBackProfileIndex(ILLEGAL_PROFILEINDEX);//M2MFallBackProfileIndex
	JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(ILLEGAL_PROFILEINDEX);
	JcvmConfig_SetM2MLastDisableProfileIndex(ILLEGAL_PROFILEINDEX);
	JcvmConfig_SetM2MNotificationType(NOTIFICATION_FIRST_TIME);
#endif

#endif
#if UICC_FUCTION_PRIVATEAUTH
	SetEeprom(E_TAG_AUTHDATA_UPDATE_ADDR,PROFILE_STATE_AUTH_NOT_UPDATE,PROFILE_MAX_NUMBER);  //初始化更新鉴权参数指令为“未执行”
#endif
	SetEeprom(RSPDefaultFileSystem_Addr, 0xFF,  4);  //初始化RSP 的默认文件系统字段
#if UICC_FUCTION_PRIVATEAUTH	
	SetEeprom(E_MFKEYADDR,0x0A,0x01);   //设置MF key初始次数为10次
	SetEeprom(E_MFKEYADDR+1,0xFF,0x08);  //设置MF key to FF

	SetEeprom(E_MANAGERKEYADDR,0x0A,0x01);   //设置Manager key初始次数为10次
	SetEeprom(E_MANAGERKEYADDR+1,0xFF,0x08);  //设置MF key to FF
#endif	
//#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH  //该操作在clearAllEeprom中实现，这里不再调用,by xwh 20230105
//	ecpMem_Set(RamBuffer,0x00,PFPageSize);
//	for(num = 1; num<= Num_HeFlash_Pages;num++)	
//	{		 			
//		WriteHEFlashPage(HE_FLASH_BASE+(num-1)*PFPageSize,RamBuffer);
//	}
//	SetEeprom(ADDR_FILENO_HEFLASH,0x00,0x01);
//#endif	

	SetEeprom(E_CUR_DURATION,0x00,0x02); //poll interval 初始化
}
