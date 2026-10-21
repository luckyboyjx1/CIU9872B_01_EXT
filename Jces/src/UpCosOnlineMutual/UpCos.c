/*
*@author     : lgy
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放UPCOS的操作流程，基于中国移动UPCOS相关规范
*
*@history     :V1.0 :create @ 20220104
*/

#include "UpCos.h"
#include "profile.h"
#include "Apdu.h"
#include "uicc_main.h"
#include "filesystem.h"
#include "stkhead.h"
#include "platform_api.h"
#include "NvmHal.h"
#include "cardManager.h"
#include  "ToolkitAPIPublic.h"
#include  "ToolkitRFM.h"
#include "dataManager.h"
#include "nativemethod_impl.h"
#include "JcvmConfig.h"
#if  GP__AMENDMENT__B__FOR_MASK
#include "Interface_AnnexB.h"
#include "HttpProcess.h"
#endif

#include "ecpDefine.h"

#include "ceph.h"
#include "UploadInfoFunction.h"
#include "filesystem.h"
#include "fileAction.h"
#include "FunImp.h"
#include "opFunction.h"
#include "SecurityDomainData.h"
#include "KeySet.h"
#if  COS_UPDATE_ONLINE_MUTUAL

UpCosStatus X_A G_upCosStatus;//该变量的值需要更新到E2中
u8   X_A  G_upCosNextAction;
u8 g_UPCOS_remainEncData[8];//缓存需要和下一分组一起解密的密文数据，格式：L(1字节) + V(0~7字节)

BYTE X_A G_AuthenticationFlag;		//upcos业务中使用，当前cos已进行鉴权并成功的标记
BYTE X_A G_StatusNumForUPCOSReport;	//upcos 业务中的开机联网成功后接收到的status个数, todo：此值可以考虑和G_STATUSNUM_FOR_NOTIFICATION整合成一个



/*
*@function   :checkEntry
*
*@description: 初始化
*
*@param      :
*@param      :
*
*@return     :成功： 1  失败：0
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create by lgy 
*/
#if !RUN_IN_SIMULATOR
u8 checkEntry(PN32 entryAddr)
{
	PN32			D_A			addrCheck;
	u8				D_A			i;
	u8              D_A         result;

	result    =  1;
	addrCheck = (PN32)((u8 *)entryAddr + sizeof(RealEntry));

	i = sizeof(RealEntry) >> 2;
	for(; i != 0 ; i--)
	{
		if((*entryAddr == 0)||(*entryAddr == 0xFFFFFFFF)|| (*entryAddr != *addrCheck))
		{
			result = 0;
			break;
		}
		entryAddr ++;
		addrCheck ++;
	}

	return result;
}
#endif

/*
*@function   :InitUpCOSVar
*
*@description: 初始化
*
*@param      :
*@param      :
*
*@return     :void
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220215
*/
void InitUpCOSVar(void)
{
	G_upCosNextAction = UPCOSConfig_GetUPCOSNextAction();

	ecpMem_Copy(MEMtPM8(&G_upCosStatus.cosSection),COSUP_BACKUP_DATA,sizeof(UpCosStatus));

	G_StatusNumForUPCOSReport = 0;

	G_AuthenticationFlag = FALSE;
	
	COS_UPDATE_COSLoading = FALSE;
#if RUN_IN_SIMULATOR
	memCopyAtomic(MEMtPM8(COSUP_PARA_BASE), MEMtPM8(&UPCosPara[0]), EEPAGE_SIZE);//更新到E方
#endif

#if !RUN_IN_SIMULATOR
	if(checkEntry((PN32)COSA_START_ENTRY) == 1)
	{
		if(UPCOSConfig_GetActiveCos() != COSCS_A)
		{
			UPCOSConfig_SetActiveCos(COSCS_A);
		}
	}
	else
	{
		if(UPCOSConfig_GetActiveCos() != COSCS_B)
		{
			UPCOSConfig_SetActiveCos(COSCS_B);
		}
	}
#endif
}
/*
*@function   :SetCosVersion
*
*@description: 设置配置区的COS版本号
*
*@param      :cosSection 要设置去区域
*@param      :mode  设置功能版本/增量版本
*@param      :version  版本号
*@return     :status
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220412
*/
u16 SetCosVersion(u8 cosSection,u8 mode,u16 version)
{
	if(cosSection == COSCS_A)
	{
		if(mode == MODE_FUNCVERSION)
		{
			UPCOSConfig_SetUPCOSAFunVersion(version);
		}
		else
		{
			UPCOSConfig_SetUPCOSAIncVersion(version);
		}
	}
	else if(cosSection == COSCS_B)
	{
		if(mode == MODE_FUNCVERSION)
		{
			UPCOSConfig_SetUPCOSBFunVersion(version);
		}
		else
		{
			UPCOSConfig_SetUPCOSBIncVersion(version);
		}
	}
	else
	{
		return 0x6A80;
	}

	return 0x9000;
}
/*
E0D2000X02 XXXX
P1指示要设置的版本类型 00表示funcVersion ，80表示IncVersion
P2指定要设置的COS区域，1表示设置COSA    2表示设置COSB
P3 = 2，数据域为2字节COS版本号 ，对应UPCOS需求规范描述的COS主版本
*/
//u16 UPCOS_SetCOSVersion(PX8 apduBuffer)
//{
//	u16    X_A   version;
//	
//	if(apduBuffer[APDU_P3_OFFSET] != 0x02)
//	{
//		return 0x6700;
//	}
//	
//	apdu_ApduSetIncomingAndReceive();
//	
//	version = BuildWord(&apduBuffer[APDU_DATA_OFFSET]);
//	
//	return SetCosVersion(apduBuffer[APDU_P2_OFFSET],apduBuffer[APDU_P1_OFFSET],version);
//}
/*
*@function   :SetCOSUPStatus
*
*@description: 设置COSUP升级状态
*
*@param      :
*@param      :
*
*@return     :void
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220215
*/
void SetCOSUPStatus(u8 status)
{
	G_upCosStatus.cosupStatus = status; //加载完成
	memWriteU8NoAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_COSUPSTATUS), G_upCosStatus.cosupStatus);
}
/*设置COS下一步操作*/
void SetCOSUPNextAction(u8 action)
{
	G_upCosNextAction = action;

	UPCOSConfig_SetUPCOSNextAction(G_upCosNextAction);//G_upCosNextAction写E2 ，报送后清成COSUPNextAction_Null
}
/*
*@function   :GetCurCosVersion
*
*@description: 获取当前激活的COS版本
*
*@param      :
*
*@return     :curCosVersion
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220217
*/
u16 GetCurCosVersion(void)
{
	u8       X_A      activeTag;

	activeTag = UPCOSConfig_GetActiveCos();

	if(activeTag == COSCS_A)
	{
		return UPCOSConfig_GetUPCOSAFunVersion();		//当前激活功能版本号  2
	}
	else
	{
		return UPCOSConfig_GetUPCOSBFunVersion();		//当前激活功能版本号  2
	}

}

/*
*@function   :GetRemainCodeSize
*
*@description: 获取可以使用的CODE空间
*
*@param      :flag
*@param      :
*
*@return     :remain size
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220215
*/
u32 GetRemainCodeSize(u8 flag)
{
	u32      X_A        size = 0;
	u32      X_A        tmp;

	ecpMem_Copy(MEMtPM8(&G_upCosStatus),COSUP_BACKUP_DATA,sizeof(UpCosStatus));

	if(flag == COSCS_A)//获取A区可用空间
	{
		if(UPCOSConfig_GetActiveCos() == COSCS_A)//当前激活的是A区
		{
			size = 0;
		}
		else//当前激活的是B
		{
			if(G_upCosStatus.cosSection == COSCS_A)//正在维护A区
			{
				tmp = U32_M_U32((G_upCosStatus.groupNum-1),G_upCosStatus.groupLen);
				size = U32_S_U32(COS_SIZE,tmp);
			}
			else //没有维护  或者 维护B区加载完后激活到了B区
			{
				size = COS_SIZE;
			}
		}
	}
	else//获取B区可用空间
	{
		if(UPCOSConfig_GetActiveCos() == COSCS_B)//当前执行的是B区
		{
			size = 0;
		}
		else//当前激活的是A区
		{
			if(G_upCosStatus.cosSection == COSCS_B)//正在维护B区
			{
				tmp = U32_M_U32(G_upCosStatus.groupNum -1,G_upCosStatus.groupLen);
				size = U32_S_U32(COS_SIZE,tmp);
			}
			else  //没有维护  或者 维护A区加载完后激活到了A区
			{
				size = COS_SIZE;
			}
		}
	}

	return  size;
}

u8 GetBreakPoint(void)
{
	if(((G_upCosStatus.groupNum - 1) == G_upCosStatus.groupTotalNum )||		//已处理块数 = 总快数
		((G_upCosStatus.groupNum == 0)&&(G_upCosStatus.groupTotalNum == 0)))
	{
		return 0; //没有断点
	}

	return G_upCosStatus.cosSection;
}
/*
*@function   :GetDeviceType
*
*@description: 获取设备类型
*
*@param      :
*@param      :
*
*@return     :0：物联网设备  /1：人联网设备
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220218
*/
u8 GetDeviceType(void)
{
	return UPCOSConfig_GetUPCOSDeviceType();
}

/*
*@function   :GetMaintenanceInfoByFileName
*
*@description: 从cosup.xx中获取COS维护信息
*
*@param      :u8 infoType  数据类型
*@param（out）:PX8 info
*
*@return     :u8 len  获取的数据长度
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220218
*/
u8 GetMaintenanceInfoByFileName(u8 infoType, PX8 info)
{
	u8    X_A    len;
	u8    X_A    tmp[9];

	ecpMem_Copy(&G_upCosStatus.upcosName[0],COSUP_BACKUP_DATA + UPCOS_UPCOSNAME,sizeof(G_upCosStatus.upcosName));

	if(G_upCosStatus.upcosName[COSUP_TOTALLEN_OFFSET] != (COSUP_FILENAME_LEN -1))
	{
		return FALSE;
	}

	switch(infoType)
	{
		case INFO_CARDISSUER://卡商代码
			info[0] = G_upCosStatus.upcosName[COSUP_CARDISSUER_OFFSET];
			len = 1;
			break;
		case INFO_CARDTYPE:
			ecpMem_Copy(&info[0], &G_upCosStatus.upcosName[COSUP_CARDTYPE_OFFSET],  2);
			len = 2;
			break;

		case INFO_COSFUNVERSION://COS主版本  01.00 转为ASCII码后存的值是30312E3030,是否需要转成BCD码输出？
			ecpMem_Copy(&tmp[1], &G_upCosStatus.upcosName[COSUP_FUNCVERSION_OFFSET],  2);
			ecpMem_Copy(&tmp[3], &G_upCosStatus.upcosName[COSUP_FUNCVERSION_OFFSET+3],  2);
			tmp[0] = 4;
			StringToInt(&tmp[0],&tmp[0]);

			info[0] =  tmp[1];
			info[1] =  tmp[2];
			len = tmp[0];
			break;

		case INFO_COSINCVERSION://COS增量版本 同COSUP_INCVERSION_OFFSET   xx.xx -->
			ecpMem_Copy(&tmp[1], &G_upCosStatus.upcosName[COSUP_INCVERSION_OFFSET],  2);
			ecpMem_Copy(&tmp[3], &G_upCosStatus.upcosName[COSUP_INCVERSION_OFFSET+3],  2);
			tmp[0] = 4;
			StringToInt(&tmp[0],&tmp[0]);

			info[0] =  tmp[1];
			info[1] =  tmp[2];
			len = tmp[0];
			break;
		case INFO_CHIPTYPE_FLAG:
			if(G_upCosStatus.upcosName[COSUP_CHIPTYPE_FLAG_OFFSET] == 0x30)
			{
				info[0] = 0x00;//不需要精确匹配芯片
			}
			else
			{
				info[0] = 0x01;
			}
			len = 1;
			break;

		case INFO_COSSECTION://空间标识 A区/B区
			if(G_upCosStatus.upcosName[COSUP_COSSECTION_OFFSET] == 'a')
			{
				info[0] = COSCS_A;// A区COS
			}
			else if(G_upCosStatus.upcosName[COSUP_COSSECTION_OFFSET] == 'b')
			{
				info[0] = COSCS_B;//B区
			}
			else//if(G_upCosStatus.upcosName[COSUP_COSSECTION_OFFSET] == '0')
			{
				info[0] = 0x00;//不匹配代码空间
			}
			len = 1;
			break;

		case INFO_COSCHECKSUN://校验值  ,ASCii -> Hex
			ecpMem_Copy(&tmp[1], &G_upCosStatus.upcosName[COSUP_CHECKSUM_OFFSET],  8);
			tmp[0] = 8;
			StringToInt(&tmp[0],&tmp[0]);
			ecpMem_Copy(info,&tmp[1],tmp[0]);
			len = tmp[0];
			break;

		case INFO_EXTENSION://扩展名
			ecpMem_Copy(info, &G_upCosStatus.upcosName[COSUP_EXTENSION_OFFSET],  6);
			len = 6;
			break;
		case INFO_FILENAME_WITHOUTCHECKSUM:
			ecpMem_Copy(info, &G_upCosStatus.upcosName[COSUP_CARDISSUER_OFFSET],  COSUP_CHECKSUM_OFFSET-2);
			ecpMem_Copy(&info[COSUP_CHECKSUM_OFFSET-2], &G_upCosStatus.upcosName[COSUP_EXTENSION_OFFSET],  6);
			len = COSUP_CHECKSUM_OFFSET-2 + 6;
			break;
		default :
			len = 0;
			break;
	}

	return  len;
}
/*
*@function   :SwitchCOS
*
*@description: COS切换，切换到新下载的COS区域
*
*@param      :CosVersion:  指定的功能版本
*@param      :cosFlag:  COSCS_A/COSCS_B
*
*@return     :TRUE/FALSE
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220218
*/
u8 SwitchCOS(u16 CosVersion, u8 cosFlag)
{
	u8   X_A   result = FALSE;
	
//	if (isInProgress())//在事务中切COS会导致ENTRY写成功了，启动新COS后进了新的COS ，后续事务恢复又把ENTRY信息还原回去旧值 ，此时ENTRY和ActiveCos标记都是旧COS的，但实际在新COS下执行
//	{
//		return FALSE;
//	}
	
	UPCOSConfig_SetActiveCos(cosFlag);

#if !RUN_IN_SIMULATOR
	//todo: 将ENTRY处的地址处理一下  
	
	COS_UPDATE_COSLoading = TRUE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
	
	if(cosFlag == COSCS_A)//切到A区
	{		
		if(checkEntry((PN32)COSA_BACKENTRY) == 1)
		{
			ecpMem_Copy(MEMtPM8(&g_UPCOS_storeDataBuf[0]),COSA_BACKENTRY,EEPAGE_SIZE);
			ecpNvm_WriteCache_NoCheck(MEMtPM8(COSA_START_ENTRY), MEMtPM8(&g_UPCOS_storeDataBuf[0]), EEPAGE_SIZE);
			memset(MEMtPM8(&g_UPCOS_storeDataBuf[0]),0,EEPAGE_SIZE);
			ecpNvm_WriteCache_NoCheck(MEMtPM8(COSB_START_ENTRY), MEMtPM8(&g_UPCOS_storeDataBuf[0]), EEPAGE_SIZE);
			result = TRUE;
		}
	}
	else
	{
		if(checkEntry((PN32)COSB_BACKENTRY) == 1)
		{
			ecpMem_Copy(g_UPCOS_storeDataBuf,COSB_BACKENTRY,EEPAGE_SIZE);
			ecpNvm_WriteCache_NoCheck(MEMtPM8(COSB_START_ENTRY), MEMtPM8(&g_UPCOS_storeDataBuf[0]), EEPAGE_SIZE);
			memset(MEMtPM8(&g_UPCOS_storeDataBuf[0]),0,EEPAGE_SIZE);
			ecpNvm_WriteCache_NoCheck(MEMtPM8(COSA_START_ENTRY), MEMtPM8(&g_UPCOS_storeDataBuf[0]), EEPAGE_SIZE);
			result = TRUE;
		}
	}
	
	COS_UPDATE_COSLoading = FALSE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
#endif
	return  result;
}

/*
*@function   :PaddingActiveReming
*
*@description: 拼装激活提示语
*
*@param      :PM8 outBuf
*@param      :
*
*@return     :u16 len
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220218
*/
#define DEVICE_IOT      0
#define DEVICE_IOP      1
BYTE code  UPCOS_ActivateWarn1[] = {0x65,0xB0,0x72,0x48,0x67,0x2C,0x00,0x43,0x00,0x4F,0x00,0x53,0x00,0x55,0x00,0x50,0x65,0x87,0x4E,0xF6,0xFF,0x08};//新版本COSUP文件（
//BYTE code  UPCOS_ActivateWarn2[] = {0x00,0x34,0x00,0x5F,0x00,0x30,0x00,0x30,0x00,0x5F,0x00,0x30,0x00,0x31,0x00,0x2E,0x00,0x30,0x00,0x30,0x00,0x5F,0x00,0x30,0x00,0x5F,0x00,0x31,0x00,0x5F,0x00,0x30,0x00,0x30,0x00,0x2E,0x00,0x30,0x00,0x30,0x00,0x5F,0x00,0x30,0x00,0x2E,0x00,0x75,0x00,0x70,0x00,0x63,0x00,0x6F,0x00,0x73};//4_00_01.00_0_1_00.00_0.upcos
BYTE code  UPCOS_ActivateWarn3[] = {0xFF,0x09,0x5D,0xF2,0x51,0xC6,0x59,0x07,0x5C,0x31,0x7E,0xEA,0xFF,0x0C,0x66,0x2F,0x54,0x26,0x73,0xB0,0x57,0x28,0x6F,0xC0,0x6D,0x3B,0xFF,0x1F};//）已准备就绪，是否现在激活？

u16 PaddingActiveReming(PM8 outBuf)
{
	u8     X_A     tmpLen,tmpLen2;

	tmpLen = sizeof(UPCOS_ActivateWarn1);

	ecpMem_Copy(&outBuf[0], UPCOS_ActivateWarn1, tmpLen);
	outBuf[tmpLen]  = GetMaintenanceInfoByFileName(INFO_FILENAME_WITHOUTCHECKSUM,&outBuf[1+tmpLen]);

	AsciiToUCS2(&outBuf[tmpLen],&outBuf[tmpLen]);

	tmpLen2 = outBuf[tmpLen];
	ecpMem_Move(&outBuf[tmpLen], &outBuf[1+tmpLen], tmpLen2);

	ecpMem_Copy(&outBuf[tmpLen+tmpLen2], UPCOS_ActivateWarn3,  sizeof(UPCOS_ActivateWarn3));

	return (tmpLen + tmpLen2 + sizeof(UPCOS_ActivateWarn3));
}
/*
*@function   :ActiveCOS
*
*@description: 加载完成后 激活新的COS
*
*@param      :
*@param      :
*
*@return     :
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220218
*/
#define DEVICE_IOT      0
#define DEVICE_IOP      1
void ActiveCOS(void)
{
	u8     X_A     result = 0;
	u16    X_A     cosFunVersion;
	u8     X_A     cosFlag,tmp[2];

	GetMaintenanceInfoByFileName(INFO_COSFUNVERSION,MEMtPM8(&tmp[0]));
	cosFunVersion = BuildWord(&tmp[0]);
	GetMaintenanceInfoByFileName(INFO_COSSECTION,&cosFlag);

	if(GetDeviceType() == DEVICE_IOT)//物联网设备
	{
		//处理Entry
		if(SwitchCOS(cosFunVersion,cosFlag) == TRUE)
		{
			SetCOSUPStatus(COSUPSTATUS_ActiveSuccess); //激活成功
		}
		else
		{
			SetCOSUPStatus(COSUPSTATUS_ActiveFial); //激活失败
			return;
		}

		if(G_upCosNextAction == COSUPNextAction_ActiveImmediately)//立即激活
		{
			InitProactiveCommand(CMD_REFRESH,REFRESH_MODE_UICC_RESET,DEV_ME);
			result = SendProactiveCommand(); //refresh - reset，会重新上电，不会走后面的流程

			if(result == CMD_Success)//refresh成功不会走到这里  而是直接复位，
			{
				G_upCosNextAction = COSUPNextAction_Null;//中国移动COS更新测试脚本/COS全量升级流程测试(天喻)/用户取消激活流程/COSUP_Fullupdate_Cancel_N002 用户2次取消激活后 第三次确认激活 refresh成功了但是卡片没有复位
			}
		}
		else //COSUPNextAction_ActiveByRst 复位激活
		{
			//等待复位
		}
	}
	else //人联网设备
	{
		if(G_upCosNextAction == COSUPNextAction_ActiveByRst)//加载完成 复位激活    ，不需要提醒用户
		{
			G_upCosNextAction = COSUPNextAction_Null;

			if(SwitchCOS(cosFunVersion,cosFlag) == FALSE)
			{
				SetCOSUPStatus(COSUPSTATUS_ActiveFial); //激活失败
				return;
			}
			else
			{
				SetCOSUPStatus(COSUPSTATUS_ActiveSuccess); //
				return;
			}
		}
		else//加载完成，立即激活   /激活提醒指令
		{
		//提示用户升级数据加载完成，是否立即激活。
			memset(tempBuf,0,sizeof(tempBuf));

			tempBuf[0] = PaddingActiveReming(&tempBuf[1]);

			result = cephDisplayTextSingle(0x81, UCS2, tempBuf);

			if(result == CMD_Success)//用户选择激活
			{
				if(SwitchCOS(cosFunVersion,cosFlag) == FALSE)
				{
					SetCOSUPStatus(COSUPSTATUS_ActiveFial); //激活失败
					return;
				}
				else
				{
					SetCOSUPStatus(COSUPSTATUS_ActiveSuccessByConfirm); //激活成功（由用户确定）

					InitProactiveCommand(CMD_REFRESH,REFRESH_MODE_UICC_RESET,DEV_ME);
					result = SendProactiveCommand(); //refresh - reset，会重新上电，不会走后面的流程

					if(result == CMD_Success)//refresh成功不会走到这里  而是直接复位，
					{
						G_upCosNextAction = COSUPNextAction_Null;//中国移动COS更新测试脚本/COS全量升级流程测试(天喻)/用户取消激活流程/COSUP_Fullupdate_Cancel_N002 用户2次取消激活后 第三次确认激活 refresh成功了但是卡片没有复位  此处不报送 等复位报送
					}
				}

			}
			else //if (result == CMD_USERTERMINATESESSION)该情况直接返回 ExitStkPerform  用户不激活，写完状态后则直接return
			{
				SetCOSUPStatus(COSUPSTATUS_ActiveFialByCancel); //激活失败（被用户取消）
			}
		}

	}
}
/*
*@function     :IF_UPCOS_ExpandHandle
*
*@description  :UPCOS处理完指令后续的一些操作，如关闭BIT通道后发主动式指令，收到数据短信提醒激活COS等
*
*@see          :
*
*@param        :void
*@param        :void
*
*@return       :void
*
*@RAM Resources:
*
*@author       :
*
*@history      :created by lgy@20220224
*/
void IF_UPCOS_ExpandHandle(void)
{
	u8    X_A   result,activeTag;
	u16   X_A   cosFunVersion;

	if(G_upCosNextAction != COSUPNextAction_Null)
	{
		if((G_upCosNextAction&0x7F) == COSUPNextAction_VersionConfirm)
		{
			if(G_upCosNextAction == COSUPNextAction_VersionConfirm)//无新版本时  只提示信息 不报送
			{
				VersionAffirmDisplay();
			}
			else
			{
				result = VersionAffirmDisplay();//有新版本时

				if(result == CMD_Success)//用户点击确认（升级），开通道
				{
					IF_UPCOS_PendingHTTPAlert();
				}
			}
		}
		else if(G_upCosNextAction == COSUPNextAction_ActiveRemind_NoDisplay)//指定的标识已激活 ，不需要重新激活   只报送 不提示信息
		{
			IF_UPCOS_PendingHTTPAlert();
		}
		else if(G_upCosNextAction == COSUPNextAction_VersionBack)//版本回退 如果指定的回退版本已经是激活的  在指令中直接反9000  不会走到该流程
		{
			G_upCosNextAction = COSUPNextAction_Null; //清掉变量 本次不报送，上电后2个status后报送

			InitProactiveCommand(CMD_REFRESH,REFRESH_MODE_UICC_RESET,DEV_ME);
			result = SendProactiveCommand(); //refresh - reset，会重新上电，不会走后面的流程
		}
		else
		{
			ActiveCOS();//能从 ActiveCOS 函数返回到此处的情况都要求立刻报送, 不能到此处的情况是在ActiveCOS中进行了复位
			if(G_upCosNextAction != COSUPNextAction_Null)//COSUPNextAction_Null的情况为发送REFRESH以后收到执行成功（卡片并没有直接复位），此时等待复位后收到2个status才开通道报送
			{
				IF_UPCOS_PendingHTTPAlert();
			}
		}
	}

}
/*
*@function     :IF_UPCOS_ExpandHandle_ForCloseHttpChannel
*
*@description  :UPCOS处理完指令后续的一些操作，如关闭BIT通道后发主动式指令，关闭通道后中断下载，收到数据短信提醒激活COS等
*
*@see          :
*
*@param        :void
*@param        :void
*
*@return       :void
*
*@RAM Resources:
*
*@author       :
*
*@history      :created by lgy@20220412
*/
void IF_UPCOS_ExpandHandle_ForCloseHttpChannel(void)
{
	if(G_upCosStatus.cosupStatus == COSUPSTATUS_Loading)//加载中关通道
	{
		SetCOSUPStatus(COSUPSTATUS_LoadingBroken); //加载中断
	}
	else
	{
		IF_UPCOS_ExpandHandle();//UPCOS上报等
	}
}
/*
*@function     :COSMaintenance
*
*@description  :COS维护
*
*@see          :参考《中国移动SIM卡软件升级原型验证—COS空中升级方案说明_2021112201.pdf》7.1章节
*
*@param        :void
*@param        :void
*
*@return       :void
*
*@RAM Resources:RecData
*
*@author       :
*
*@history      :created by lgy@20220104
*/
u16  UPCOS_COSMaintenance(void)
{	
	u8    p2;
	u16   result = 0x6A80;
	
	p2 = APDUBuffer[APDU_P2_OFFSET];
	
	//根据P1确定数据是否收齐，等收齐再处理  ，只能在0通道？
	
	switch(p2)
	{
		case  0xFF:		//升级初始化
			result = UPCOS_UpdateInit();
			break;
		
		case  0xFE:		//升级续传
			result = UPCOS_ResumeUpdate();
			break;		
		
		case  0xFD:		//加载完成 待断电激活
		case  0xFC:		//加载完成 立即断电激活
			result = UPCOS_FinishLoading(p2);			
			break;	
		
		case  0xFB:		//加载失败
			result = UPCOS_FailLoading();
			break;	
		
		case  0xFA:		//激活提示 短信下发
			result = UPCOS_ActivationRemind();
			break;	

		case  0xEF:		//回退到指定版本COS
			result = UPCOS_VersionBack();
			break;	

		case  0xEE:		//版本确认
			result = UPCOS_versionAffirm();
			break;		

		default		:
			break;
		
	}
	
	return  result;
}

/*
*@function     :GetChipType
*
*@description  :获取芯片型号
*
*@see          :
*
*@param        :destBuf
*
*@return       :len
*
*@author       :
*
*@history      :created by lgy@20220214
*/
u16  GetChipType(u8* destBuf)
{
	return 0;
}
/*
*@function     :UPCOSGetDataSubFunc
*
*@description  :UPCOS_GetData子函数
*
*@see          :
*
*@param        :void
*@param        :void
*
*@return       :void   发送拼装好的数据
*
*@RAM Resources:RecData
*
*@author       :
*
*@history      :created by lgy@20220222
*/
u16 UPCOSGetDataSubFunc(PM8 dstBuf)
{
	u16		dataOff;
	u16		seidLen;
	u16		chipTypeLen;
	u16		sessionIDLen;
	u16		upcosNameLen;
	u16		responseLen;
	u8      tmp = 0;

	ecpMem_Copy(MEMtPM8(&G_upCosStatus.cosSection),COSUP_BACKUP_DATA,sizeof(UpCosStatus));

	//数据拼装
	dstBuf[0] = 0x00;
	dstBuf[1] = 0x01;

	dataOff = 2;
	seidLen = getSeId(&dstBuf[dataOff + 1]);//标识数据(SEID)  XX
	dstBuf[dataOff++] = seidLen;		//标识长度  1


	dataOff = dataOff + seidLen;
	dstBuf[dataOff] = 0x34;		//卡商代码默认34  GetMaintenanceInfoByFileName(INFO_CARDISSUER,&dstBuf[dataOff++]);取不到值时反回34
	GetMaintenanceInfoByFileName(INFO_CARDISSUER,&dstBuf[dataOff++]);

	if(GetMaintenanceInfoByFileName(INFO_CARDTYPE,&dstBuf[dataOff])== FALSE)//valueToBuff(&dstBuf[dataOff], 0x3031, 2);		//卡品代码  2
	{
		valueToBuff(&dstBuf[dataOff], 0x0000, 2);//上一次有维护信息，从维护信息里区卡片代码，无维护信息返回默认值0
	}

	dataOff = dataOff + 2;

	if((GetMaintenanceInfoByFileName(INFO_CHIPTYPE_FLAG,&tmp) == FALSE)||(tmp = 0))//无维护信息 或 不需要匹配芯片
	{
		chipTypeLen = 0;
	}
	else
	{
		chipTypeLen = GetChipType(&dstBuf[dataOff+2]);		//芯片型号  XX
	}
	valueToBuff(&dstBuf[dataOff], chipTypeLen, 2);		//芯片型号长度  2
	dataOff = dataOff + 2 + chipTypeLen;

	dstBuf[dataOff++] = UPCOSConfig_GetActiveCos();		//当前COS激活标志  1

	valueToBuff(&dstBuf[dataOff], GetCurCosVersion(), 2);		//当前激活功能版本号  2

	dataOff = dataOff + 2;
	valueToBuff(&dstBuf[dataOff],COS_SIZE , 4);		//COSCS-A空间大小  4
	valueToBuff(&dstBuf[dataOff + 4],GetRemainCodeSize(COSCS_A) , 4);		//COSCS-A可用空间大小  4
	valueToBuff(&dstBuf[dataOff + 8],UPCOSConfig_GetUPCOSAFunVersion() , 2);		//COSCS-A代码功能版本号 2

	dataOff = dataOff + 10;
	valueToBuff(&dstBuf[dataOff], COS_SIZE, 4);		//COSCS-B空间大小  4
	valueToBuff(&dstBuf[dataOff + 4], GetRemainCodeSize(COSCS_B), 4);		//COSCS-B可用空间大小  4
	valueToBuff(&dstBuf[dataOff + 8], UPCOSConfig_GetUPCOSBFunVersion(), 2);		//COSCS-B代码功能版本号 2

	dataOff = dataOff + 10;
	dstBuf[dataOff++] = GetBreakPoint();		//断点标识  1 	00：没有断点。 01：A区断点； 02：B区断点

	sessionIDLen = G_upCosStatus.cosUpsessionIDLen;
	dstBuf[dataOff++] = sessionIDLen;		//最近会话标识长度  1
	memcpy(&dstBuf[dataOff], &G_upCosStatus.cosUpsessionID[0], sessionIDLen);		//最近会话标识数据  XX

	dataOff = dataOff + sessionIDLen;
	upcosNameLen = G_upCosStatus.upcosName[0];
	dstBuf[dataOff++] = upcosNameLen;		//最近COSUP标识长度  1
	memcpy(&dstBuf[dataOff], &G_upCosStatus.upcosName[1], upcosNameLen);		//最近COSUP标识  XX

	dataOff = dataOff + upcosNameLen;
	dstBuf[dataOff++] = G_upCosStatus.cosupStatus;		//最近COSUP状态  1

	valueToBuff(&dstBuf[dataOff], G_upCosStatus.upcosSize, 4);		//原生代码大小  4
	valueToBuff(&dstBuf[dataOff + 4], G_upCosStatus.groupLen, 4);		//分组大小  4
//	if(G_upCosStatus.groupTotalNum == 0)
	{
		valueToBuff(&dstBuf[dataOff + 8], G_upCosStatus.groupTotalNum, 4);		//分组总数  4
	}
//	else
//	{
//		valueToBuff(&dstBuf[dataOff + 8], G_upCosStatus.groupTotalNum - 1, 4);		//分组总数  4
//	}

	if(G_upCosStatus.groupNum == 0)
	{
		valueToBuff(&dstBuf[dataOff + 12], G_upCosStatus.groupNum, 4);		//原生代码已下载块数  4
	}
	else
	{
		valueToBuff(&dstBuf[dataOff + 12], G_upCosStatus.groupNum - 1, 4);		//原生代码已下载块数  4
	}

	responseLen = dataOff + 16;

	return responseLen;
}
/*
*@function     :UPCOS_GetData
*
*@description  :现场数据采集
*
*@see          :参考《中国移动SIM卡软件升级原型验证—COS空中升级方案说明_2021112201.pdf》7.2章节
*
*@param        :void
*@param        :void
*
*@return       :void   发送拼装好的数据
*
*@RAM Resources:RecData
*
*@author       :
*
*@history      :created by lgy@20220104
*/
u16  UPCOS_GetData(void)
{
	// P1P2P3合法性判断
	u8    	p1,p2,p3;
	u16		responseLen;

	p1 = APDUBuffer[APDU_P1_OFFSET];
	p2 = APDUBuffer[APDU_P2_OFFSET];
	p3 = APDUBuffer[APDU_P3_OFFSET];

	if((p1 != 0xFF) || (p2 != 0xFE) || (p3 != 0x00))
	{
		throw_error(SW_WRONG_P1P2);
	}

	responseLen = UPCOSGetDataSubFunc(RecData);

	SetRemoteProcessRes(0x05, responseLen);

	return 0x9000;
}


/*
*@function   :UPCOSUpdateInit
*
*@description: 升级初始化，从头开始
*
*@param      :void
*
*@return     :0x9000/0x6A80
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220106
*/
u16 UPCOS_UpdateInit(void)
{
	u8     X_A   offset,transActioinFlag;
	
	offset = 0;
	
	if(BuildWord(&RecData[offset]) != COSUP_SCHEME)//COS升级方案版本不等于0x0001
	{
		SetCOSUPStatus(COSUPSTATUS_StartUpFail); //加载完成
		throw_error(0x6A80);//P2 的值为 FF（升级初始化，从头开始）但数据域与规范要求的格式不一致时应返回 6A80
	}
	
	ecpMem_Set(MEMtPM8(&G_upCosStatus.cosSection), 0x00, sizeof(UpCosStatus));//全局变量G_upCosStatus初始化成0

	offset += 2; //指向会话标识长度	
	if((RecData[offset] == 0)||(RecData[offset] > 0x10))//天喻使用的长度为16字节  东信2字节，COS目前开的数组为16字节
	{
		SetCOSUPStatus(COSUPSTATUS_StartUpFail); //升级启动失败
		throw_error(0x6A80);
	}
	G_upCosStatus.cosUpsessionIDLen = RecData[offset];
	memcpy(&G_upCosStatus.cosUpsessionID[0], &RecData[offset+1], RecData[offset]);
	
	offset = offset+RecData[offset]+1;//指向COSUP标识长度

	if(RecData[offset] != (COSUP_FILENAME_LEN-1))
	{
		SetCOSUPStatus(COSUPSTATUS_StartUpFail); //加载失败
		throw_error(0x6A80);
	}

	ecpMem_Copy(&G_upCosStatus.upcosName[0], &RecData[offset], RecData[offset]+1);

	if(G_upCosStatus.upcosName[COSUP_COSSECTION_OFFSET] == 'a')
	{
		G_upCosStatus.cosSection = COSCS_A;// A区COS
	}
	else if(G_upCosStatus.upcosName[COSUP_COSSECTION_OFFSET] == 'b')
	{
		G_upCosStatus.cosSection = COSCS_B;//B区
	}
	else//if(G_upCosStatus.upcosName[COSUP_COSSECTION_OFFSET] == '0')
	{
		throw_error(0x6985);//G_upCosStatus.cosSection = 0x00;//不匹配代码空间  暂时不支持
	}

	if(UPCOSConfig_GetActiveCos() == G_upCosStatus.cosSection)// 不能自己升级自己
	{
		throw_error(0x6985);
	}

	offset = offset+RecData[offset]+1;//指向COSUP大小	
	G_upCosStatus.upcosSize = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向分组大小
	G_upCosStatus.groupLen = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向分组总数
	G_upCosStatus.groupTotalNum = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向起始组号
	G_upCosStatus.groupNum = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向链接参数，用于报送
	
	g_UPCOS_remainEncData[0] = 0x00;//初始化长度

	transActioinFlag = IF_BeginTransaction();
	//处理链接参数
	if(IF_UPCOS_HTTPSAnalyseTriggerPara(&RecData[offset]) != TRUE)
	{
		SetCOSUPStatus(COSUPSTATUS_StartUpFail); //加载shibai
		throw_error(0x6A80);
	}

	G_upCosStatus.cosupStatus = (COSUPSTATUS_Loading); //加载中

	SetCosVersion(G_upCosStatus.cosSection,MODE_FUNCVERSION,0x00);//升级前 清除版本信息，避免升级过程中把该版本的COS内容改变了（有可能是没升级完的COS）版本没有修改，版本回退等操作会认为是一个合法的COS
	SetCosVersion(G_upCosStatus.cosSection,MODE_INCVERSION,0x00);

	memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_COS_SECTION), MEMtPM8(&G_upCosStatus.cosSection), sizeof(G_upCosStatus));//更新到E方
	
	IF_CommitTransaction(transActioinFlag);

	return 0x9000;
}

/*
*@function   :UPCOSResumeUpdate
*
*@description: 升级续传
*
*@param      :void
*
*@return     :0x9000/0x6984
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220105
*/
u16 UPCOS_ResumeUpdate(void)
{
	u8 offset;
	
	memCopyAtomic(MEMtPM8(&G_upCosStatus.cosSection), MEMtPM8(COSUP_BACKUP_DATA + UPCOS_COS_SECTION), sizeof(UpCosStatus));//从E方读数据到全局变量G_upCosStatus，14是最后4个成员的长度

	if((G_upCosStatus.cosupStatus != COSUPSTATUS_Loading)
		&&(G_upCosStatus.cosupStatus != COSUPSTATUS_LoadingBroken)
		&&(G_upCosStatus.cosupStatus != COSUPSTATUS_LoadingBrokenByCancel))//增加状态机，判断卡片是否处于加载COS状态。--收到 COSUS 发送的升级续传指令时， USIM 卡的当前状态不是 COSUP 加载中的状态，应处理失败返回 6985(功能条件不满足)
	{
		throw_error(0x6985);
	}
	
	offset = 0;
	
	if(BuildWord(&RecData[offset]) != COSUP_SCHEME)
	{
		throw_error(0x6A80);//P2 的值为 FE（升级续传）但数据域与规范要求的格式不一致时应返回 6A80
	}
	
	offset += 2; //指向会话标识长度

	if((RecData[offset] > 0x10)||(RecData[offset] == 0x00))
	{
		throw_error(0x6A80);//内部开的数组最大支持0x10字节
	}

	if((G_upCosStatus.cosUpsessionIDLen != RecData[offset]) || (ECP_CMP_RESULT_NOT_EQUAL == memCompareEqual(&G_upCosStatus.cosUpsessionID[0], &RecData[offset+1], RecData[offset])))//比较会话标识
	{
		throw_error(0x6A80);
	}

	offset = offset + RecData[offset] + 1;//指向COSUP标识长度
	
	if((G_upCosStatus.upcosName[0] != RecData[offset]) || (ECP_CMP_RESULT_NOT_EQUAL == memCompareEqual(&G_upCosStatus.upcosName[1], &RecData[offset+1], RecData[offset])))//COSUP标识不相等
	{
		throw_error(0x6A80);
	}
	offset = offset + RecData[offset] + 1;//指向COSUP大小
	
	if((G_upCosStatus.upcosSize != BuildUWord(&RecData[offset])) 
		|| (G_upCosStatus.groupLen != BuildUWord(&RecData[offset+4])) || (G_upCosStatus.groupTotalNum != BuildUWord(&RecData[offset+8])))
	{
		throw_error(0x6A80);
	}
	
	offset += 12;//指向起始组号
	
	if(BuildUWord(&RecData[offset]) != (G_upCosStatus.groupNum))//当为升级续传时，起始组号为“已经下载组数+1”
	{
		throw_error(0x6985);
	}
	
	//G_upCosStatus.groupNum = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向链接参数，用于报送
	
	g_UPCOS_remainEncData[0] = 0x00;//续传时重新初始化长度

	//处理链接参数
	IF_UPCOS_HTTPSAnalyseTriggerPara(&RecData[offset]);
	

//	memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_COS_SECTION), MEMtPM8(&G_upCosStatus.cosSection), sizeof(UpCosStatus));//将G_upCosStatus更新到E方
	
	return 0x9000;
}

/*
*@function   :UPCOS_ActivationRemind
*
*@description: 激活提醒
*
*@param      :void
*
*@return     :0x9000/0x6985/other SW status
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220106
*/
u16 UPCOS_ActivationRemind(void)
{
	u8      X_A     offset,transActioinFlag,tmp[2];
	u16     X_A     version;
	offset = 0;
	
	if(BuildWord(&RecData[offset]) != COSUP_SCHEME)
	{
		throw_error(0x6A80);//P2 的值为 FE（升级续传）但数据域与规范要求的格式不一致时应返回 6A80
	}
	
	offset += 2; //指向会话标识长度
	if((G_upCosStatus.cosUpsessionIDLen != RecData[offset]) || (ECP_CMP_RESULT_NOT_EQUAL == memCompareEqual(&G_upCosStatus.cosUpsessionID[0], &RecData[offset+1], RecData[offset])))//会话标识不相等
	{
		throw_error(0x6A80);
	}

	offset = offset + RecData[offset] + 1;//指向COSUP标识长度
	if((G_upCosStatus.upcosName[0] != RecData[offset]) || (ECP_CMP_RESULT_NOT_EQUAL == memCompareEqual(&G_upCosStatus.upcosName[1], &RecData[offset+1], RecData[offset])))//COSUP标识不相等
	{
		throw_error(0x6985);
	}

	offset = offset + RecData[offset] + 1;//指向链接参数

	if(IF_UPCOS_HTTPSAnalyseTriggerPara(&RecData[offset]) == FALSE)//处理链接参数
	{
		throw_error(0x6A80);
	}

	transActioinFlag = IF_BeginTransaction();

	GetMaintenanceInfoByFileName(INFO_COSFUNVERSION, MEMtPM8(&tmp[0]));//主版本
	version =  BuildWord(&tmp[0]);
	if(version == GetCurCosVersion())
	{
		SetCOSUPNextAction(COSUPNextAction_ActiveRemind_NoDisplay);//只报送 不发display
	}
	else
	{
		SetCOSUPNextAction(COSUPNextAction_ActiveRemind);//执行到本函数，用于已经取消过一次激活，此时通过短信下发激活提醒, 如果用户第二次选择取消激活，卡再次打上标记需要开启http会话,建立报送链接
	}

	if(G_Source_Type == SourceType_SMS)
	{
		PendingMessageAlert(D_UPCOS_ACTIVEREMIND,0);//打激活提醒标记  提醒用户激活
	}
	IF_CommitTransaction(transActioinFlag);

	return 0x9000;
}

/*
*@function   :UPCOS_FinishLoading
*
*@description: 加载完成
*
*@param      :0xFD：加载完成，待断电激活。当为人联网设备时，通过提示用户，由用户确定是否激活；
*             0xFC：加载完成，立即复位激活。当为人联网设备时，通过提示用户，由用户确定是否激活；
*
*@return     :0x9000/0x6985/other SW status
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220107
*/
u16 UPCOS_FinishLoading(u8 type)
{
	u8     X_A    transActioinFlag;
	u16    X_A    version,incVersion;
	u8     X_A    tmp[2];

	if(ECP_CMP_RESULT_NOT_EQUAL == memCompareEqual(&G_upCosStatus.cosUpHASH[2], &RecData[2], 32))//读取计算COSUP的HASH值，将结果和数据域的HASH值比较
	{
		SetCOSUPStatus(COSUPSTATUS_FailLoading);
		throw_error(0x6985);//CR6:收到 COSUS 发送的加载完成指令，且 cos 校验值与实际不一致，应处理失败返回 6985
	}

	GetMaintenanceInfoByFileName(INFO_COSCHECKSUN, MEMtPM8(&tmp[0]));//
	if(G_upCosStatus.crc != BuildWord(&tmp[0]))
	{
		SetCOSUPStatus(COSUPSTATUS_FailLoading);
		throw_error(0x6985);//
	}
	transActioinFlag = IF_BeginTransaction();
	
	//GetMaintenanceInfoByFileName(INFO_COSSECTION, MEMtPM8(&cosSection));// A/B区
	GetMaintenanceInfoByFileName(INFO_COSFUNVERSION, MEMtPM8(&tmp[0]));//主版本
	version = BuildWord(&tmp[0]);

	GetMaintenanceInfoByFileName(INFO_COSINCVERSION, MEMtPM8(&tmp[0]));//增量版本
	incVersion = BuildWord(&tmp[0]);

	SetCosVersion(G_upCosStatus.cosSection,MODE_FUNCVERSION,version);
	SetCosVersion(G_upCosStatus.cosSection,MODE_INCVERSION,incVersion);

//#if !RUN_IN_SIMULATOR
//	//todo:将入口地址写到备份区 ,该操作放到编译和工具里实现

//	ecpMem_Copy(tempBuf,CHIP_CONFIG_START,EEPAGE_SIZE);
//	memCopyAtomic(MEMtPM8(COSA_BACKENTRY),tempBuf,EEPAGE_SIZE);

//	ecpMem_Copy(tempBuf,BOOT_CONFIG_START,EEPAGE_SIZE);
//	memCopyAtomic(MEMtPM8(COSB_BACKENTRY),tempBuf,EEPAGE_SIZE);

//	if(G_upCosStatus.cosSection == COSCS_A)
//	{
//		memFillAtomic(MEMtPM8(CHIP_CONFIG_START), 0, EEPAGE_SIZE);
//	}
//	else
//	{
//		memFillAtomic(MEMtPM8(BOOT_CONFIG_START), 0, EEPAGE_SIZE);
//	}
//#endif

	if(BuildWord(&RecData[0]) != COSUP_SCHEME)//先校验HASH值 如果哈希值校验通过 说明COS加载进去的数据是OK的 ，先把COS版本号等内容写进配置区   剩下的就是激活操作
	{
		SetCOSUPNextAction(COSUPNextAction_Null);

		SetCOSUPStatus(COSUPSTATUS_ActiveFial);
		IF_CommitTransaction(transActioinFlag);
		throw_error(0x6A80);//数据域与规范要求的格式不一致时应返回 6A80
	}

	SetCOSUPStatus(COSUPSTATUS_FinishLoading); //加载完成

	if(type == 0xFD)//加载完成 待断电激活
	{
		SetCOSUPNextAction(COSUPNextAction_ActiveByRst);
	}
	else//加载完成 立即复位激活
	{
		SetCOSUPNextAction(COSUPNextAction_ActiveImmediately);
	}

	IF_CommitTransaction(transActioinFlag);

	return 0x9000;
}

/*
*
*@function   :UPCOS_FailLoading
*
*@description: 加载失败
*
*@param      :
*
*@return     :0x9000/0x6A80/other SW status
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220111
*/
u16 UPCOS_FailLoading(void)
{
	if(P3 != 0x00)
	{
		throw_error(0x6A80);//P2 的值为 FB（加载失败）但数据域与规范要求的格式不一致时应返回 6A80
	}
	
	SetCOSUPStatus(COSUPSTATUS_FailLoading); //加载失败
	
	return 0x9000;
}

/*
*@function   :UPCOS_VersionBack
*
*@description: 回退到指定版本COS
*
*@param      :
*
*@return     :0x9000/0x6984/6985
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220112
*/
u16 UPCOS_VersionBack(void)
{
	u16    X_A    offset;
	u16    X_A    funcVersion;
	u8     X_A    activeCos;
	
	offset = 0;

	if(BuildWord(&RecData[offset]) != COSUP_SCHEME)
	{
		throw_error(0x6A80);//P2 的值为 （版本回退）但数据域与规范要求的格式不一致时应返回 6A80
	}

	offset = 2;//指向功能版本

	funcVersion = BuildWord(&RecData[offset]);

	if(funcVersion == 0x0000)//没有加载任何COS时 配置区的值为0，为避免funcVersion与配置区比较值EQUAL
	{
		throw_error(0x6984);
	}

	if(GetCurCosVersion() == funcVersion)//当前已经是指定版本
	{
		return 0x9000;
	}
	
	if(UPCOSConfig_GetUPCOSAFunVersion() == funcVersion)
	{
		activeCos = COSCS_A;
	}
	else if(UPCOSConfig_GetUPCOSBFunVersion() == funcVersion)
	{
		activeCos = COSCS_B;
	}
	else//if((UPCOSConfig_GetUPCOSAFunVersion() != funcVersion)&&(UPCOSConfig_GetUPCOSBFunVersion() != funcVersion))//
	{
		throw_error(0x6984);
	}

	offset += 2;//处理链接参数
	if(IF_UPCOS_HTTPSAnalyseTriggerPara(&RecData[offset]) != TRUE)
	{
		throw_error(0x6A80);
	}

	if(SwitchCOS(funcVersion,activeCos) == TRUE)
	{
		SetCOSUPStatus(COSUPSTATUS_ActiveSuccess); //激活成功  HTTP关通道后发refresh reset后正在切换 / 掉电后切换
	}
	else
	{
		SetCOSUPStatus(COSUPSTATUS_ActiveFial); //激活失败
		return 0x6985;
	}
	
	SetCOSUPNextAction(COSUPNextAction_VersionBack);//回退版本时不用发起报送,  回退成功后重新激活入网后会收到status指令，累计2个status时再开启http链接开启报送

	
	return 0x9000;
}
/*
*@function   :VersionAffirmDisplay
*
*@description: 发送版本确认提示语
*
*@param      :void
*
*@return     :
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220224
*/
u8 VersionAffirmDisplay(void)
{
	u8    X_A     result;

	//memset(tempBuf,0,sizeof(tempBuf));

	ecpMem_Copy(MEMtPM8(&tempBuf[0]),COSUP_BACKUP_DATA+UPCOS_REMIND_TEXT,1);//todo：处理提示语内容
	ecpMem_Copy(MEMtPM8(&tempBuf[1]),COSUP_BACKUP_DATA+UPCOS_REMIND_TEXT + 1,tempBuf[0]);//todo：处理提示语内容

	result = cephDisplayTextSingle(0x81, UCS2, tempBuf);

	SetCOSUPNextAction(COSUPNextAction_Null);

	return  result;
}
/*
*@function   :UPCOS_versionAffirm
*
*@description: 版本确认
*
*@param      :void
*
*@return     :0x9000/0x6984/6985
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220111
*/
u16 UPCOS_versionAffirm(void)
{
	u8      X_A    offset, newVersionFlag;
	u8      X_A    backOff,tmpLen,backLen;
//	u8      X_A    backOff1 = 0;
	
	offset = 0;
	
	if(BuildWord(&RecData[offset]) != COSUP_SCHEME)
	{
		throw_error(0x6A80);//P2 的值为 EE（版本确认）但数据域与规范要求的格式不一致时应返回 6A80
	}
	
	offset += 2;
	
	newVersionFlag = RecData[offset++];

	if((newVersionFlag != 0x00)&&(newVersionFlag != 0x01))
	{
		throw_error(0x6A80);
	}

	tmpLen = RecData[offset++];
	backLen = tmpLen;//提示语长度
	backOff = offset; //提示语内容

	offset += tmpLen;//指向COSUP标识长度
	
	if(newVersionFlag == 0)//无新版本
	{
		if((RecData[offset] != 0)||(offset >= P3))//COSUP标识长度字段必须存在
		{
			throw_error(0x6A80);
		}
	}
	else
	{
	//	backOff1 = offset;//todo：备份COSUP标识长度偏移
		offset += (RecData[offset]+1);//指向链接参数

		if(IF_UPCOS_HTTPSAnalyseTriggerPara(&RecData[offset]) == FALSE)//处理链接参数
		{
			throw_error(0x6A80);
		}

	}

	ecpMem_Copy(MEMtPM8(&tempBuf[1]),&RecData[backOff],backLen);//todo：处理提示语内容

	tempBuf[0]  =  backLen;

//    if(backOff1 != 0)//有新版本时
//    {
//		AsciiToUCS2(&RecData[backOff1],&RecData[backOff1]);//todo：处理COSUP标识, 拼装到准备display的数组中
//
//		if((backLen+RecData[backOff1]) >= UPCOS_REMIND_MAXLEN)
//		{
//			throw_error(0x6A80);
//		}
//
//		ecpMem_Copy(MEMtPM8(&tempBuf[1+backLen]),&RecData[backOff1+1],RecData[backOff1]);//todo：处理提示语内容后半部分
//
//		tempBuf[0]  +=  RecData[backOff1];
//    }

	memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA+UPCOS_REMIND_TEXT), MEMtPM8(&tempBuf[0]), tempBuf[0] + 1);//

	if(G_Source_Type == SourceType_SMS)//HTTP下来的数据会在关通道以后再处理
	{
		PendingMessageAlert(D_UPCOS_VERSIONAFFIRM,0);//打标记 提醒用户
	}

	if(newVersionFlag == 1)//本条指令后display，断电后无后续操作 不写E2
	{
		G_upCosNextAction = COSUPNextAction_VersionConfirm|0x80;
	}
	else
	{
		G_upCosNextAction = COSUPNextAction_VersionConfirm;
	}

	return 0x9000;
	
}
/*检查是否更新的是用户区*/
u8 checkUPCOSUserAddr(u32 addr)
{
	if(((addr >= EEPROM_BASE)&&(addr < sysConfig_GetNvmTop()))||(addr >= COSUP_BACKUP_DATA))
	{
		return 1;
	}
	return 0;
}
/*
*@function     :UPCOS_CosLoading
*
*@description  :COS加载
*
*@see          :参考《中国移动SIM卡软件升级原型验证—COS空中升级方案说明_2021112201.pdf》7.3章节
*
*@param        :void
*@param        :void
*
*@return       :0x9000
*
*@RAM Resources:RecData
*
*@author       :lx
*
*@history      :V1.0 :create @20220124
*/
u8 g_UPCOS_storeDataBuf[EEPAGE_SIZE];//COS数据缓存页
//u8 const UPCOS_KEY[]={0x95,0x62,0xBB,0xC6,0xD3,0x80,0x9E,0xC0,0xDB,0x70,0xC0,0x13,0xB6,0xF8,0x22,0xEC};//COS数据解密密钥
u16 UPCOS_CosLoading(void)
{
#define GROUPNUM_LEN  4
#define HEAD_LENGTH  16 //E0D0指令的长度（头和体），例如：E0D000XX06 xxxxxxxxxxxx xxxxxxxxxx
#define UPDATE_STATUS_LEN  112//UpCosStatus中dataOffset到cosUpHASHRemainBuf的长度，即收齐一条E0D0指令时需要保存的status的长度
#define srcDataBuf  (APDUBuffer+5)

	u32 groupNum, writeAddr;
	u8 groupLen, remainBuffer[HEAD_LENGTH];
	u16 offset;//当前处理数据在srcDataBuf中的偏移
	u32 len;//当前block本次可处理的COS数据的长度
	u16 writeLen;//实写数据的长度
	u16 dataLen;//可处理数据的最大长度
	u8 remainDataLen;//上一分组未处理数据的长度
	
	groupNum = BuildUWord(&APDUBuffer[APDU_DATA_OFFSET]);
	groupLen = APDUBuffer[APDU_P3_OFFSET]-GROUPNUM_LEN;

	if((G_upCosStatus.groupNum != groupNum)//《中国移动COS升级功能测试规范20211123》:收到 COSUS 发送的加载升级包指令块号不连续时应返回 6A80
		||(groupNum > G_upCosStatus.groupTotalNum)//块号超过初始化设置的最大分组数 //天喻/现场数据采集/COSUP_APDU_Getdata_N002、COSUP_APDU_Getdata_N003
	//	||(groupLen > G_upCosStatus.groupLen))//数据长度超过初始化设置的分组大小
		||((groupLen != G_upCosStatus.groupLen)&&(groupNum != G_upCosStatus.groupTotalNum)))//只允许最后一个分组的长度与升级初始化指定的长度不一致
	{
		SetCOSUPStatus(COSUPSTATUS_FailLoading); //加载失败
		throw_error(0x6A80);
	}

	if((APDUBuffer[APDU_P2_OFFSET] != 0x00) && (APDUBuffer[APDU_P2_OFFSET] != 0x80))
	{
		SetCOSUPStatus(COSUPSTATUS_FailLoading); //加载失败
		throw_error(0x6A80);
	}

	if(G_upCosStatus.groupOffset == 0)
	{
		if(groupNum != G_upCosStatus.groupTotalNum)
		{
			HASHUpdate(&RecData[GROUPNUM_LEN], groupLen, HASH_UPDATE, &G_upCosStatus.cosUpHASH[0], &G_upCosStatus.cosUpHASHRemainBuf[0], ALG_SHA_256);//带IV计算sha256中间值
		}
		else//最后一个分组
		{
			HASHDoFinal(&RecData[GROUPNUM_LEN], groupLen, &G_upCosStatus.cosUpHASH[2], HASH_UPDATE, &G_upCosStatus.cosUpHASH[0], &G_upCosStatus.cosUpHASHRemainBuf[0], ALG_SHA_256);//计算最终的哈希值
		}

//		if(G_upCosStatus.endFlag == 1)//已经处理过E0D1
//		{
//			G_upCosStatus.groupNum++;//(groupNum-1)即为已下载块数，此操作方便服务器判断已下载块数
//
//			memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_GROUP_NUM), MEMtPM8(&G_upCosStatus.groupNum), sizeof(u32));//更新groupNum到E方
//			memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_HASH), MEMtPM8(&G_upCosStatus.cosUpHASH[0]), (sizeof(G_upCosStatus.cosUpHASH)+sizeof(G_upCosStatus.cosUpHASHRemainBuf)));//更新cosUpHASH和cosUpHASHRemainBuf到E方
//
//			return 0x9000;
//		}
	}

	remainDataLen = g_UPCOS_remainEncData[0];

	dataLen = UPCOSDecryptData(MEMtPM8(srcDataBuf), (G_upCosStatus.groupOffset + GROUPNUM_LEN));//对数据进行3DES解密

	offset = 0;

	if(G_upCosStatus.remainLen > 0)//有因命令头不完整而未处理完的数据
	{
		memCopyAtomic(MEMtPM8(&remainBuffer[0]), MEMtPM8(&G_upCosStatus.remainBuf[0]), G_upCosStatus.remainLen);//将不完整命令头的前一部分先放入缓存中

		len = (HEAD_LENGTH - G_upCosStatus.remainLen);//复用len，缓存不完整命令头的后一部分的长度

		memCopyAtomic(MEMtPM8(&remainBuffer[G_upCosStatus.remainLen]), MEMtPM8(&srcDataBuf[offset]), len);//将收到的指令中不完整命令头的后一部分在缓存中与前一部分进行拼装

		if(remainBuffer[APDU_INS_OFFSET] == 0xD0)
		{
			G_upCosStatus.groupOffset = (len - remainDataLen);

			processD0D1Head(0xD0, remainBuffer, 0x00);
		}
		else if(remainBuffer[APDU_INS_OFFSET] == 0xD1)
		{
			processD0D1Head(0xD1, remainBuffer, 0x00);

			return 0x9000;
		}

		offset += len;
		G_upCosStatus.remainLen = 0;
	}

	while(offset < dataLen) //APDUBuffer中的数据没有处理完
	{
		if(G_upCosStatus.blockTotalLen == G_upCosStatus.dataOffset)
		{
			if(srcDataBuf[offset] == 0xE0)
			{
				if((offset+HEAD_LENGTH) > dataLen)//E0D0/E0D1命令头不完整，先放入缓存，等待下一分组收到时一起处理
				{
					G_upCosStatus.remainLen = (dataLen - offset);

					memcpy(&G_upCosStatus.remainBuf[0], &srcDataBuf[offset], G_upCosStatus.remainLen);//处理命令头不完整的情况

					break;
				}

				if(srcDataBuf[offset+APDU_INS_OFFSET] == 0xD0)
				{
					G_upCosStatus.groupOffset += (HEAD_LENGTH - remainDataLen);

					processD0D1Head(0xD0, srcDataBuf, offset);
				}
				else if(srcDataBuf[offset+APDU_INS_OFFSET] == 0xD1)
				{
					processD0D1Head(0xD1, srcDataBuf, offset);

					return 0x9000;
				}

				offset += HEAD_LENGTH;//COS数据在srcDataBuf中的偏移
			}
			else
			{
				throw_error(0x6A80);
			}
		}

		len = (G_upCosStatus.blockTotalLen - G_upCosStatus.dataOffset - G_upCosStatus.pageOffset);//当前block剩余数据的长度

		if((len + offset) >= dataLen)//当前block剩余数据的长度大于或等于当前分组未处理数据的COS数据长度
		{
			len = (dataLen - offset);//本次可处理的COS数据
		}

		if(((len+G_upCosStatus.pageOffset) >= EEPAGE_SIZE) || ((G_upCosStatus.dataOffset + G_upCosStatus.pageOffset + len) == G_upCosStatus.blockTotalLen))//缓存将满 或 该条EOD0即将收完
		{
			if((len+G_upCosStatus.pageOffset) < EEPAGE_SIZE)//该条EOD0即将收完并且缓存未满1页
			{
				writeLen = (len+G_upCosStatus.pageOffset);
			}
			else
			{
				writeLen = EEPAGE_SIZE;
			}

			G_upCosStatus.groupOffset = (writeLen - G_upCosStatus.pageOffset - remainDataLen);//由于缓存（大小为1页）内数据是按8字节整数倍放入的，remainDataLen最大为7，因此G_upCosStatus.groupOffset不会为负数

			ecpMem_Copy(&g_UPCOS_storeDataBuf[G_upCosStatus.pageOffset], &srcDataBuf[offset], (writeLen-G_upCosStatus.pageOffset));//凑成一页或将即将收完的E0D0数据写入缓存

			writeAddr = G_upCosStatus.blockNum*0x10000+G_upCosStatus.startOffset+G_upCosStatus.dataOffset;//E方目的地址

			if(((writeAddr & EEPAGE_MASK) != 0)||(checkUPCOSUserAddr(writeAddr)))
			{
				SetCOSUPStatus(COSUPSTATUS_FailLoading); //加载失败
				throw_error(0x6984);//E方目的地址不是页起始  / 不是ROM区
			}
				
			COS_UPDATE_COSLoading = TRUE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
			
			ecpNvm_WriteCache_NoCheck(MEMtPM8(writeAddr), MEMtPM8(&g_UPCOS_storeDataBuf[0]), writeLen);
				//ecpNvm_WriteCache(MEMtPM8(writeAddr), MEMtPM8(&g_UPCOS_storeDataBuf[0]), writeLen);//将COS数据写入E方
			COS_UPDATE_COSLoading = FALSE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区

			G_upCosStatus.crc = Get_crc16WithIV(G_upCosStatus.crc, MEMtPM8(&g_UPCOS_storeDataBuf[0]), writeLen);//计算当前数据的CRC

			G_upCosStatus.dataOffset += writeLen;

			if(G_upCosStatus.groupOffset == groupLen) //当前分组的数据都被写进了E方，此时直接备份下一分组的groupOffset和groupNum，发生续传时可避免重复处理当前分组的数据
			{
				G_upCosStatus.groupOffset = 0;
				G_upCosStatus.groupNum++;
			}

			if(G_upCosStatus.blockTotalLen == G_upCosStatus.dataOffset)//收齐一条E0D0
			{
				memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_DATA_OFFSET), MEMtPM8(&G_upCosStatus.dataOffset), UPDATE_STATUS_LEN);//更新状态值到E方
			}

			offset += (writeLen-G_upCosStatus.pageOffset);
			len -= (writeLen-G_upCosStatus.pageOffset);//本条指令剩余的可处理的当前Block的数据

			G_upCosStatus.pageOffset = 0;

			memCopyAtomic((g_UPCOS_storeDataBuf), MEMtPM8(&srcDataBuf[offset]), len); //将剩余的数据拷贝至缓存
		}
		else
		{
			memCopyAtomic((g_UPCOS_storeDataBuf + G_upCosStatus.pageOffset), MEMtPM8(&srcDataBuf[offset]), len);
		}

		G_upCosStatus.pageOffset += len;
		offset += len;

		remainDataLen = 0;
	}

	if(G_upCosStatus.groupNum == groupNum)
	{
		G_upCosStatus.groupOffset = 0;
		G_upCosStatus.groupNum++;
	}

	return 0x9000;
}


/*
*@function   :processD0D1Head
*
*@description: 处理E0D0/E0D1头数据
*
*@param      :u8 type  	  0xD0/0xD1
*@param      :PX8 buffer  D0/D1所在的数组
*@param      :u8 offset   在buffer中的偏移
*
*@return     :void
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220218
*/
void processD0D1Head(u8 type, PX8 buffer, u8 offset)
{
	if(type == 0xD0)
	{
		G_upCosStatus.blockNum = BuildWord(&buffer[offset+APDU_P1_OFFSET]);//更新bank号

		G_upCosStatus.startOffset = BuildWord(&buffer[offset+APDU_DATA_OFFSET]);//更新起始地址在block中的偏移

		G_upCosStatus.blockTotalLen = BuildUWord(&buffer[offset+APDU_DATA_OFFSET+2]);//更新待处理数据块总长度

		G_upCosStatus.dataOffset = 0;

		memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_BLOCK_NUM), MEMtPM8(&G_upCosStatus.blockNum), (sizeof(G_upCosStatus.blockNum)+sizeof(G_upCosStatus.startOffset)+sizeof(G_upCosStatus.blockTotalLen)+sizeof(G_upCosStatus.dataOffset)));//更新blockNum到dataOffset的状态值到E方
	}
	else
	{
		if(G_upCosStatus.crc != BuildWord(&buffer[offset+APDU_DATA_OFFSET]))//处理E0D1数据中的CRC，和当前CRC进行比较
		{
			throw_error(0x6A80);//CRC错误
		}

		G_upCosStatus.groupNum++;//(groupNum-1)即为已下载块数，此操作方便服务器判断已下载块数
		G_upCosStatus.groupOffset = 0;//该分组已处理完成，将groupOffset清0

//		G_upCosStatus.endFlag = 1;//打上已处理E0D1标记
//		memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_ENDFLAG), MEMtPM8(&G_upCosStatus.endFlag), 1);//更新endFlag到E方
	}

	memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_HASH), MEMtPM8(&G_upCosStatus.cosUpHASH[0]), (sizeof(G_upCosStatus.cosUpHASH)+sizeof(G_upCosStatus.cosUpHASHRemainBuf)));//更新cosUpHASH和cosUpHASHRemainBuf到E方
	memCopyAtomic(MEMtPM8(COSUP_BACKUP_DATA + UPCOS_GROUP_NUM), MEMtPM8(&G_upCosStatus.groupNum),(sizeof(G_upCosStatus.groupNum)+sizeof(G_upCosStatus.groupOffset)));//更新groupNum和groupOffset到E方
}


/*
*@function     :UPCOSDecryptData
*
*@description  :COS加载密文解密
*
*@see          :
*
*@param        :PM8 decDataBuf		保存输出的明文
*@param        :u16 offset			当前处理数据在RecData中的偏移
*
*@return       :u16 decLen			最大可解密长度
*
*@RAM Resources:RecData
*
*@author       :lx
*
*@history      :V1.0 :create @20220316
*/
u16 UPCOSDecryptData(PM8 decDataBuf, u16 offset)
{
	u16 decLen;//最大可解密长度
	u8 remainEncDataBuf[8],UPCOS_KEY[16];//缓存当前分组未解密的数据，格式：L(1字节) + V(0~7字节)
	
	UPCOSConfig_GetUPCOSUPKeyValue(&UPCOS_KEY[0]);
	decLen = (APDUBuffer[APDU_P3_OFFSET] - offset + g_UPCOS_remainEncData[0]);//复用decLen，计算上一分组未处理数据和本次分组未处理数据的总长度
	remainEncDataBuf[0] = (decLen % 8);//计算本次分组需要缓存的数据长度（用于下一分组）
	decLen = (decLen - remainEncDataBuf[0]);//获取最大可解密长度

	if(remainEncDataBuf[0] != 0x00)
	{
		memcpy(&remainEncDataBuf[1], &RecData[APDUBuffer[APDU_P3_OFFSET]-remainEncDataBuf[0]], remainEncDataBuf[0]);//缓存密文
	}
	
	memmove(&decDataBuf[g_UPCOS_remainEncData[0]], &RecData[offset], (decLen-g_UPCOS_remainEncData[0]));
	memcpy(&decDataBuf[0], &g_UPCOS_remainEncData[1], g_UPCOS_remainEncData[0]);//拼装上一分组未处理数据和本次分组数据
	
//	remainDataLen[0] =  g_UPCOS_remainEncData[0];
	//PX_W_U8((PX8)remainDataLen, g_UPCOS_remainEncData[0]);

	memcpy(&g_UPCOS_remainEncData[0], &remainEncDataBuf[0], (remainEncDataBuf[0]+1));//将该分组剩余未解密的数据拷贝到全局数组，待收到下一分组再一起处理

	SymCipherDofinal(MEMtPM8(&decDataBuf[0]), decLen, MEMtPM8(&decDataBuf[0]), MEMtPM8(UPCOS_KEY), U8_M_U8(UPCOS_KEYLEN,8), NULL, NULL, ALG_DES_ECB_NOPAD, ECB_Decrypt, 0x00, ALG_DES);//从&srcDataBuf[0]开始对decLen长度的密文进行解密

	return decLen;
}

u16 NM_IsUPCOSCommand()
{
	u8      X_A     b_INS,b_P1;
	u16     X_A     sw;

	sw           =      0x0000;
	b_INS        =      APDUBuffer[APDU_INS_OFFSET];
	b_P1         =      APDUBuffer[APDU_P1_OFFSET];

	if(((b_INS == 0xCA)&&(b_P1 == 0xFF))||((b_INS == 0xE2)&&(b_P1 == 0xFF))||(b_INS == 0xEA))
	{
		switch(b_INS)
		{
			case  0xCA://现场数据采集
				sw = UPCOS_GetData();
				break;
			case  0xE2://COSUP加载
				sw = UPCOS_CosLoading();
				break;
			case 0xEA://COS维护
				sw  =  UPCOS_COSMaintenance();
				break;

			default:
				break;
		}
	}
	return sw;
}
//LV LV LV ...
BYTE code STK_UpgradeMenuText[] = {
	//查看当前版本	0		12
	0x0C,0x67,0xE5,0x77,0x0B,0x5F,0x53,0x52,0x4D,0x72,0x48,0x67,0x2C,
	//查询可升级版本	13		14
	0x0E,0x67,0xE5,0x8B,0xE2,0x53,0xEF,0x53,0x47,0x7E,0xA7,0x72,0x48,0x67,0x2C,
	//当前程序的版本为 		28		14
	0x0E,0x5F,0x53,0x52,0x4D,0x7A,0x0B,0x5E,0x8F,0x72,0x48,0x67,0x2C,0x4E,0x3A,
	//信息上报成功	43		12
	0x0C,0x4F,0xE1,0x60,0x6F,0x4E,0x0A,0x62,0xA5,0x62,0x10,0x52,0x9F,
	//信息上报失败	56	12
	0x0C,0x4F,0xE1,0x60,0x6F,0x4E,0x0A,0x62,0xA5,0x59,0x31,0x8D,0x25,
	//您的SIM卡系统已是最新版本！	69	30		
	0x1E,0x60,0xA8,0x76,0x84,0x00,0x53,0x00,0x49,0x00,0x4D,0x53,0x61,0x7C,0xFB,0x7E,0xDF,0x5D,0xF2,0x66,0x2F,0x67,0x00,0x65,0xB0,0x72,0x48,0x67,0x2C,0xFF,0x01,
	//新版本	100	6
	0x06,0x65,0xB0,0x72,0x48,0x67,0x2C,
	//文件可提供升级，是否现在升级？	107	30
	0x1E,0x65,0x87,0x4E,0xF6,0x53,0xEF,0x63,0xD0,0x4F,0x9B,0x53,0x47,0x7E,0xA7,0xFF,0x0C,0x66,0x2F,0x54,0x26,0x73,0xB0,0x57,0x28,0x53,0x47,0x7E,0xA7,0xFF,0x1F,
};


/*
*@function   :DisplayCurEdition
*
*@description: 查询当前激活的固件版本号
*
*@test       :
*
*@author     :lhl
*
*@history    :V1.0 :create @20220215
*/
void DisplayCurEdition(void)
{
	u16 cosVersion;
	
	tempBuf[0] = 0x00;
	
	AddValue(tempBuf,(BYTE*)&STK_UpgradeMenuText[28]);
	
	cosVersion = GetCurCosVersion();//按BCD编码。即如果功能版本号为“12.01”时，编码为：第一个字节：0001 0010；第二个字节：0000 0001。转为UCS2为  0031 0032 002E 0030 0031

	valueToBuff(&tempBuf[101], cosVersion, 2);

	tempBuf[100] = 0x02;

	IntToString(&tempBuf[100], &tempBuf[100]);//input :LV ,output LV, L=L*2
	memmove(&tempBuf[104], &tempBuf[103],2);
	tempBuf[103] = 0x2E;//"."
	tempBuf[100] = 0x05;

	AsciiToUCS2(&tempBuf[100], &tempBuf[100]);
	AddValue(tempBuf,(BYTE*)&tempBuf[100]);
	cephDisplayTextSingle(0x81,UCS2,(BYTE *)tempBuf);
}
/*上发版本查询短信*/
BYTE  UICCSubmitToCOSUS(void)
{
	
	u8 result, offset, i, seidLen,backOffset;
	u8 rngbuf[4];
	u8 keyContext[0x10];
	u16	DESSrcDataLen,SrcLen,keyLen;
	u32 fileAddr;
    u8 sdId ,aidLen,keyVersion;
    u8 sdAid[16],CNTR[5];

    aidLen = UPCOSConfig_GetCOSUMSSDAIDLen();
    UPCOSConfig_GetUPCOSUMSSDAIDValue(&sdAid[0],aidLen);

    sdId  = GetAppIdByAIDFromAIDList(&sdAid[0],aidLen);

    if(ILLEGAL_APPID == sdId)
    {
    	return FALSE;
    }

    keyVersion =  UPCOSConfig_GetUPCOSUMSSDSubmitKeyVersion();//keySet_getMaxKeyVersion(MEMtPN8(sdDataObj), SCP_80);

    keyLen = getScp80Key(sdId, keyVersion, TYPE_KEY_KIC,  PNtMEM(&keyContext[0]));

    if(keyLen == 0)
    {
    	return FALSE;
    }

	offset = 0x00;
	
	RecData[offset++] = 0x00;		//UDL		1

	seidLen = getSeId(&RecData[offset + 1]);		//标识数据(SEID)  XX
	RecData[offset++] = seidLen;			//标识长度  1 
	
	offset = offset + seidLen;

	UPCOSConfig_GetCOSUMSSDCNTR(&CNTR[0]);
	memcpy((BYTE *)&RecData[offset], CNTR, 0x05);

	IncreatCNTR((BYTE *)CNTR);			//计数器		5
	UPCOSConfig_SetCOSUMSSDCNTR(&CNTR[0]);

	offset += 0x05;									
	Random_number_generator(rngbuf, 0x04, ALG_PSEUDO_RANDOM);		//随机数		4
	memcpy((BYTE *)&RecData[offset], rngbuf, 0x04);
	
	offset += 0x04;
	RecData[offset++] = TYPE_KEY_KID;		//密钥组别TYPE_KEY_KID		1

	backOffset = offset;//指向命令类型
	RecData[offset++] = 0x01;		//命令类型		1
	
	RecData[offset] = UPCOSGetDataSubFunc(&RecData[offset+1]);		//命令参数长度 + 命令行参数		XX
	
	offset += (RecData[offset] + 1);

	SrcLen = (offset -1);//UDL长度（不包括MAC部分）
	DESSrcDataLen = ((SrcLen + 8)/8)*8;

	RecData[offset++] = 0x80;
	for(i = 1 ;i<(DESSrcDataLen-SrcLen);i++)//padding 80 00 00 00 ---
	{
		RecData[offset++] = 0x00;
	}

	TripleDES((BYTE *)&RecData[1], keyContext, (BYTE *)&tempBuf[200], CBC_Encrypt, DESSrcDataLen, 0x10); //TYPE_KEY_KIC
	
	ecpMem_Move(&RecData[backOffset+8],&RecData[backOffset],SrcLen - (backOffset-1));

	ecpMem_Copy(&RecData[backOffset],&tempBuf[200+DESSrcDataLen-8], 0x08);//MAC
//
	RecData[0] = SrcLen + 8;
	
	//拼接 	TPDU_Header
	fileAddr = SelectTFFile(DF_STK, EF_Uploadconfig);		//TPDA
	ReadTFBuf(fileAddr, OFFSET_TPDA, &tempBuf[0], 1);
	ReadTFBuf(fileAddr, OFFSET_TPDA+1, &tempBuf[1], tempBuf[0]);
	
	offset = tempBuf[0] + 1;
	tempBuf[offset++] = 0x00;	//PID
	tempBuf[offset++] = 0x04; //DCS
	
	memcpy((BYTE *)&tempBuf[offset], (BYTE *)&RecData[0], RecData[0]+1);
	
	tempBuf[0] += (2 + RecData[0]+1);

	result = cephSendShortMessageSingle(0X00,NULL,NULL,(BYTE*)&tempBuf[0]);
	
	if(result == ExitStkPerform)
	{
		return FALSE;
	}
	return TRUE;
}

BYTE code ReferUpgraedEditionInfo(void)
{
	BYTE idata result;
	
	//Todo:COSUM主动上发查询版本信息给服务器
	result = UICCSubmitToCOSUS();
	
	if(result==TRUE)
	{
		cephDisplayTextSingle(0x81,UCS2,(BYTE *)&STK_UpgradeMenuText[43]);		//信息上报成功
	}
	else
	{
		cephDisplayTextSingle(0x81,UCS2,(BYTE *)&STK_UpgradeMenuText[56]);	//信息上报失败	
		return FALSE;
	}
	
	return TRUE;
}
/*程序升级菜单处理*/
void STK_UpgradeFunc(void)
{
	BYTE idata result;
	while(1)
	{
		result = cephSelectItemSingle(0x00, UCS2, NULL, (BYTE *)STK_UpgradeMenuText, 26); //菜单项选择
			
		if((result == BackMove)||(result == ExitStkPerform))
		{
			return ;
		}
		
		result = GetResponseItemIdentifier();
		
		switch(result)
		{
			case 1:
				DisplayCurEdition();//显示当前固件版本号
			
				break;
			
			case 2:
				result = ReferUpgraedEditionInfo();//版本查询（主动发短信给服务器）
			
				if(result == false)
				{
					return;
				}
				break;
				
			default:
				return;
		}
	}	
}

#endif

