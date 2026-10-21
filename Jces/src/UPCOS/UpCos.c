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
#include "filesystem.h"
#include "stkhead.h"
#include "NvmHal.h"
#include "cardManager.h"
#include "nativemethod_impl.h"
#include "ecpDefine.h"

#include "ceph.h"
#include "filesystem.h"
#include "fileAction.h"
#include "FunImp.h"
#include "opFunction.h"
#include "MemOperation.h"
#include "Interface_LTEFromBoot.h"
#include "FunEntryConfig.h"
#include "ToolkitRFM.h"
#if  COS_UPDATE_ONLINE_BY_NativeOS
UpCosStatus X_A G_upCosStatus;//该变量的值需要更新到E2中
//u8   X_A  G_upCosNextAction;
u8 g_UPCOS_remainEncData[8];//缓存需要和下一分组一起解密的密文数据，格式：L(1字节) + V(0~7字节)

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
	memset(&G_upCosStatus.blockNum,0,sizeof(G_upCosStatus));
	
	COS_UPDATE_COSLoading = FALSE;
}
/*
*@function     :IF_UPCOS_EnvelopHandle
*
*@description  :判断是否是在线升级的下行数据， TAR是固定的
*
*@see          :
*
*@param        :void
*@param        :void
*
*@return       :TRUE/FALSE
*
*@RAM Resources:
*
*@author       :
*
*@history      :created by lgy@20220412
*/
u8 IF_UPCOS_EnvelopHandle(void)
{	
	u8    X_A    tar[3];
	
	UPCOSConfig_GetUPCOSUMSSDTar(tar);
	
	if((G_ToolkitTar[0] == tar[0])&&(G_ToolkitTar[1] == tar[1])&&(G_ToolkitTar[2] == tar[2]))
	{
		return TRUE;
	}
	
	return FALSE;
}
/*判断是否是升级指定的AID*/
u8 IF_UPCOS_SSDAppId(PM8 aidBuf,u8 aidLen)
{
	BYTE  X_A   buf[16],len;
	
	len  = UPCOSConfig_GetCOSUMSSDAIDLen();
	UPCOSConfig_GetUPCOSUMSSDAIDValue(buf,len);
	
	if(ecpMem_Compare(aidBuf,buf,aidLen) == EQUAL)
	{
		return TRUE;
	}
	
	return FALSE;
}
/*获取PSKKEY 
pskBuf = NULL时 ，表示触发短信时检查PSK是否存在
pskBuf ！= NULL，时，获取PSK的值

return ： PSKLEN /0x00 
*/
u8 IF_UPCOS_GetPSK(u8* pskBuf, u8 keyversion,u8 sdId)
{
	u8    X_A     pskLen = 0;
	
	if((keyversion == UPCOSConfig_GetUPCOSUMSSDScp81KeyVersion())&&(sdId == UPCOSConfig_GetUPCOSUMSSDAPPID()))//BootLoader下仅可以访问SE的特定应用 ，且应用appid是唯一的 ，不管是不是UPCOS业务只要sdid和kvn能匹配就OK，卡上只有一份
	{
		pskLen = UPCOSConfig_GetUPCOSUMSSDPSKLen();
		
		if(pskBuf != NULL)
		{
			UPCOSConfig_GetUPCOSUMSSDPSK(&pskBuf[0],pskLen);	
		}
		/*else //短信触发时判断Key是否存在
		{
			
		}*/		
	}
	
	return pskLen;
	
	/*if(pskBuf == NULL)//短信触发时判断Key是否存在 ,此时TAR还在G_ToolkitTar[3]中
	{
		if(IF_UPCOS_EnvelopHandle() == TRUE)
		{
			if(keyversion != UPCOSConfig_GetUPCOSUMSSDScp81KeyVersion())
			{
				return FALSE;
			}
			
			pskLen = UPCOSConfig_GetUPCOSUMSSDPSKLen();
			
			return pskLen;
			
		}
	}
	else//握手时获取key  ,在触发和握手中间可能有其他下行把G_ToolkitTar[3]冲掉了 ，此时不能根据该值判断
	{
		if((keyversion == UPCOSConfig_GetUPCOSUMSSDScp81KeyVersion())&&(sdId == UPCOSConfig_GetUPCOSUMSSDAPPID()))
		{
			pskLen = UPCOSConfig_GetUPCOSUMSSDPSKLen();
			UPCOSConfig_GetUPCOSUMSSDPSK(&pskBuf[0],pskLen);
			return pskLen;
		}
	}*/
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

//	ecpMem_Copy(&G_upCosStatus.upcosName[0],COSUP_BACKUP_DATA + UPCOS_UPCOSNAME,sizeof(G_upCosStatus.upcosName));

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

		case INFO_COSIDENTIFY: //COS识别标识  ,ASCii -> Hex
			ecpMem_Copy(&tmp[1], &G_upCosStatus.upcosName[COSUP_COSIDENTIFY_OFFSET], 4);
			tmp[0] = 4;
			StringToInt(&tmp[0],&tmp[0]);

			info[0] = tmp[1];
			info[1] = tmp[2];
			len = tmp[0];
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
void ActiveCOS(void)
{	

#if !RUN_IN_SIMULATOR
	//todo: 将ENTRY处的地址处理一下  		
	if(checkEntry((PN32)COSA_BACKENTRY) == 1)
	{
		ecpMem_Copy(MEMtPM8(&g_UPCOS_storeDataBuf[0]),COSA_BACKENTRY,EEPAGE_SIZE);
		ecpNvm_WriteCache_NoCheck(MEMtPM8(COS_START_ENTRY), MEMtPM8(&g_UPCOS_storeDataBuf[0]), EEPAGE_SIZE);
		G_upCosStatus.cosupStatus  = COSUPSTATUS_ActiveSuccess;
	}
	else
	{
		G_upCosStatus.cosupStatus  = COSUPSTATUS_ActiveFail;
	}
#endif
	
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
	//ActiveCOS();//COS激活，该操作放到finishloading,校验COS数据合法后就将Entry写入对应地址，即使HTTP不关通道，断电后也可以回到主COS 
	if(G_upCosStatus.cosupStatus  == COSUPSTATUS_Loading)
	{
		G_upCosStatus.cosupStatus = COSUPSTATUS_LoadingBroken;
	}
	else if(G_upCosStatus.cosupStatus  ==   COSUPSTATUS_ActiveSuccess)
	{
		G_upCosStatus.cosupStatus = COSUPSTATUS_NULL;
		InitProactiveCommand(CMD_REFRESH,REFRESH_MODE_UICC_RESET,DEV_ME);
		SendProactiveCommand(); //refresh - reset，会重新上电，不会走后面的流程
	}
	else
	{
		//todo nothing
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
	u16   result = 0x6A86;
	
	if(APDUBuffer[APDU_P1_OFFSET] != 0x00)
	{
		return result;
	}
	
	p2 = APDUBuffer[APDU_P2_OFFSET];
	
	//根据P1确定数据是否收齐，等收齐再处理  ，只能在0通道？
	
	switch(p2)
	{
		case  0xFF:		//升级初始化
			result = UPCOS_UpdateInit();
			break;
					
		case  0xFD:		//加载完成 
		//case  0xFC:		//加载完成 立即断电激活
			result = UPCOS_FinishLoading(p2);			
			break;	
					
		default:
			break;		
	}
	
	return  result;
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
	u16    X_A   result;
	u8     X_A   version[2],identity[2];
	
	result = 0x9000;
	offset = 0;
	
	if(BuildWord(&RecData[offset]) != COSUP_SCHEME)//COS升级方案版本不等于0x0100
	{
		return 0x6A80;//P2 的值为 FF（升级初始化，从头开始）但数据域与规范要求的格式不一致时应返回 6A80
	}
	
	ecpMem_Set(MEMtPM8(&G_upCosStatus.blockNum), 0x00, sizeof(UpCosStatus));//全局变量G_upCosStatus初始化成0

	offset += 2; //指向会话标识长度	
	if(RecData[offset] != 0x10)//会话标识长度取固定值: 0x10
	{
		return 0x6A80;
	}
//	G_upCosStatus.cosUpsessionIDLen = RecData[offset];
	memcpy(&G_upCosStatus.cosUpsessionID[0], &RecData[offset+1], RecData[offset]);
	
	offset = offset+RecData[offset]+1;//指向COSUP标识长度

	if(RecData[offset] != (COSUP_FILENAME_LEN-1))
	{
		return 0x6A80;
	}

	ecpMem_Copy(&G_upCosStatus.upcosName[0], &RecData[offset], RecData[offset]+1);
	
//	GetMaintenanceInfoByFileName(INFO_COSFUNVERSION, MEMtPM8(&version[0]));//COS功能版本号
//	if(BuildWord(&version[0]) == UPCOSConfig_GetUPCOSFunVersion())//新COS的功能版本号和当前COS的功能版本号相同
//	{
//		return 0x6985;//没必要更新相同COS功能版本号的COS
//	} -----------------不做以上限制的原因是卡片此时已切到BOOTLOADER，如果不让其下载COS的话，卡片将一直处于BOOTLOADER；
	GetMaintenanceInfoByFileName(INFO_COSIDENTIFY, MEMtPM8(&version[0]));
	UPCOSConfig_GetCOSIdentityValue(&identity[0]);
	
	if((identity[0] != version[0])&&(identity[1] != version[1]))
	{
		return 0x6982;
	}
	
	offset = offset+RecData[offset]+1;//指向COSUP大小	
	G_upCosStatus.upcosSize = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向分组大小
	G_upCosStatus.groupLen = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向分组总数
	G_upCosStatus.groupTotalNum = BuildUWord(&RecData[offset]);
	
	offset += 4;//指向起始组号
	G_upCosStatus.groupNum = BuildUWord(&RecData[offset]);
	
	if(G_upCosStatus.groupNum != 0x01)
	{
		return 0x6A80;
	}
	offset += 4;//指向链接参数，用于报送
	
	g_UPCOS_remainEncData[0] = 0x00;//初始化长度

	G_upCosStatus.cosupStatus  = COSUPSTATUS_UpdateInit;

//	UPCOSConfig_SetUPCOSFunVersion(0x00);//升级前 不清除版本信息，加个标记在配置区  ， 升级过程中断后仍可以获取到升级前的版本号
	UPCOSConfig_SetUPCOSFunVersionFlag(0x00); //flag的值为0：当前COS功能版本号对应的COS无效    flag的值为1：当前COS功能版本号对应的COS有效 

	return result;
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
	u8     X_A    tmp[2];
	
	if(G_upCosStatus.cosupStatus  != COSUPSTATUS_FinishLoading)//COS数据下载完毕  （E0D1内部已校验一次CRC）
	{
		G_upCosStatus.cosupStatus  =   COSUPSTATUS_ActiveFail;
		return 0x6985;
	}	
	
	if(BuildWord(&RecData[0]) != COSUP_SCHEME)
	{	
		G_upCosStatus.cosupStatus  =   COSUPSTATUS_ActiveFail;	
		return 0x6A80;//数据域与规范要求的格式不一致时应返回 6A80
	}

	
	if((RecData[2] != 0x10)||(ECP_CMP_RESULT_NOT_EQUAL == memCompareEqual(&G_upCosStatus.cosUpsessionID[0], &RecData[3], 0x10)))//
	{
		G_upCosStatus.cosupStatus  =   COSUPSTATUS_ActiveFail;	
		return 0x6A80;
	}
	
	GetMaintenanceInfoByFileName(INFO_COSCHECKSUN, MEMtPM8(&tmp[0]));//获取COSUP文件的文件名中的CRC值
	if(G_upCosStatus.crc != BuildWord(&tmp[0]))
	{
		G_upCosStatus.cosupStatus  =   COSUPSTATUS_ActiveFail;
		return 0x6985;
	}
	transActioinFlag = IF_BeginTransaction();
	
	GetMaintenanceInfoByFileName(INFO_COSFUNVERSION, MEMtPM8(&tmp[0]));//COS功能版本号
	UPCOSConfig_SetUPCOSFunVersion(BuildWord(&tmp[0]));//写入COS功能版本号	
	UPCOSConfig_SetUPCOSFunVersionFlag(0x01);//打上COS功能版本号标记为有效

	IF_CommitTransaction(transActioinFlag);
	
	COS_UPDATE_COSLoading = TRUE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
	
	ActiveCOS();//COS激活将入口地址写好

	//G_upCosNextAction = COSUPNextAction_RefreshAfterCloseHttpChannel;
	COS_UPDATE_COSLoading = FALSE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
	
	return 0x9000;
}

/*检查是否更新的是代码区*/
u8 checkUPCOSCodeAddr(u32 addr)
{
	if(((addr >= COSA_DATA_START)&&(addr < CHIP_CONFIG_START))
		||((addr >= COSA_BACKENTRY)&&(addr < (COSA_BACKENTRY + EEPAGE_SIZE))))
	{
		return TRUE;
	}
	return FALSE;
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
		return 0x6A80;
	}

	if((G_upCosStatus.cosupStatus != COSUPSTATUS_UpdateInit)&&
		(G_upCosStatus.cosupStatus != COSUPSTATUS_Loading)&&
	    (G_upCosStatus.cosupStatus != COSUPSTATUS_LoadingBroken))//非断电中断HTTP后可以直接接着shengji
	{
		return 0x6985;
	}
	else
	{
		G_upCosStatus.cosupStatus  = COSUPSTATUS_Loading;
	}
	
	if(APDUBuffer[APDU_P2_OFFSET] == 0x00) 
	{
		//todo :nothing
	}
	else if(APDUBuffer[APDU_P2_OFFSET] == 0x80)
	{
		G_upCosStatus.cosupStatus  = COSUPSTATUS_FinishLoading;
	}
	else
	{
		G_upCosStatus.cosupStatus  = COSUPSTATUS_FailLoading;
		return 0x6A86;
	}

	
	
//	if(G_upCosStatus.groupOffset == 0)
//	{
//		if(groupNum != G_upCosStatus.groupTotalNum)
//		{
//			HASHUpdate(&RecData[GROUPNUM_LEN], groupLen, HASH_UPDATE, &G_upCosStatus.cosUpHASH[0], &G_upCosStatus.cosUpHASHRemainBuf[0], ALG_SHA_256);//带IV计算sha256中间值
//		}
//		else//最后一个分组
//		{
//			HASHDoFinal(&RecData[GROUPNUM_LEN], groupLen, &G_upCosStatus.cosUpHASH[2], HASH_UPDATE, &G_upCosStatus.cosUpHASH[0], &G_upCosStatus.cosUpHASHRemainBuf[0], ALG_SHA_256);//计算最终的哈希值
//		}
//	}

	remainDataLen = g_UPCOS_remainEncData[0];

	dataLen = UPCOSDecryptData(MEMtPM8(srcDataBuf), GROUPNUM_LEN);//对数据进行3DES解密

	offset = 0;

	if(G_upCosStatus.remainLen > 0)//有因命令头不完整而未处理完的数据
	{
		ecpMem_Copy(&remainBuffer[0], &G_upCosStatus.remainBuf[0], G_upCosStatus.remainLen);//将不完整命令头的前一部分先放入缓存中

		len = (HEAD_LENGTH - G_upCosStatus.remainLen);//复用len，缓存不完整命令头的后一部分的长度

		ecpMem_Copy(&remainBuffer[G_upCosStatus.remainLen], &srcDataBuf[offset], len);//将收到的指令中不完整命令头的后一部分在缓存中与前一部分进行拼装

		if(remainBuffer[APDU_INS_OFFSET] == 0xD0)
		{
			processD0D1Head(0xD0, remainBuffer, 0x00);
		}
		else if(remainBuffer[APDU_INS_OFFSET] == 0xD1)
		{
			return processD0D1Head(0xD1, remainBuffer, 0x00);			
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
					processD0D1Head(0xD0, srcDataBuf, offset);
				}
				else if(srcDataBuf[offset+APDU_INS_OFFSET] == 0xD1)
				{
					if(processD0D1Head(0xD1, srcDataBuf, offset) != 0x9000)
					{
						G_upCosStatus.cosupStatus  = COSUPSTATUS_FailLoading;
						return 0x6A80;
					}
					
					return 0x9000;
				}

				offset += HEAD_LENGTH;//COS数据在srcDataBuf中的偏移
			}
			else
			{
				G_upCosStatus.cosupStatus  = COSUPSTATUS_FailLoading;
				return 0x6A80;
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

			ecpMem_Copy(&g_UPCOS_storeDataBuf[G_upCosStatus.pageOffset], &srcDataBuf[offset], (writeLen-G_upCosStatus.pageOffset));//凑成一页或将即将收完的E0D0数据写入缓存

			writeAddr = G_upCosStatus.blockNum*0x10000+G_upCosStatus.startOffset+G_upCosStatus.dataOffset;//E方目的地址

			if(((writeAddr & EEPAGE_MASK) != 0)||(!checkUPCOSCodeAddr(writeAddr)))
			{
				G_upCosStatus.cosupStatus  = COSUPSTATUS_FailLoading;
				return 0x6984;//E方目的地址不是页起始  / 不是ROM区
			}
				
			COS_UPDATE_COSLoading = TRUE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
			
			ecpNvm_WriteCache_NoCheck(MEMtPM8(writeAddr), MEMtPM8(&g_UPCOS_storeDataBuf[0]), writeLen);
				//ecpNvm_WriteCache(MEMtPM8(writeAddr), MEMtPM8(&g_UPCOS_storeDataBuf[0]), writeLen);//将COS数据写入E方

			COS_UPDATE_COSLoading = FALSE;//该变量表示COS升级正在写CODE区，升级完以后置为FALSE 不允许更改CODE区
			
			G_upCosStatus.crc = BootLoader_Get_crc16(MEMtPM8(&g_UPCOS_storeDataBuf[0]), writeLen, G_upCosStatus.crc);//计算当前数据的CRC

			G_upCosStatus.dataOffset += writeLen;

			offset += (writeLen-G_upCosStatus.pageOffset);
			len -= (writeLen-G_upCosStatus.pageOffset);//本条指令剩余的可处理的当前Block的数据

			G_upCosStatus.pageOffset = 0;

			ecpMem_Copy(g_UPCOS_storeDataBuf, &srcDataBuf[offset], len); //将剩余的数据拷贝至缓存
		}
		else
		{
			ecpMem_Copy((g_UPCOS_storeDataBuf + G_upCosStatus.pageOffset), &srcDataBuf[offset], len);
		}

		G_upCosStatus.pageOffset += len;
		offset += len;

		remainDataLen = 0;
	}

	G_upCosStatus.groupNum++;

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
*@return     :0x9000/0x6A80
*
*@test       :
*
*@author     :lx
*
*@history    :V1.0 :create @20220218
*/
u16 processD0D1Head(u8 type, PX8 buffer, u8 offset)
{
	if(type == 0xD0)
	{
		G_upCosStatus.blockNum = BuildWord(&buffer[offset+APDU_P1_OFFSET]);//更新bank号

		G_upCosStatus.startOffset = BuildWord(&buffer[offset+APDU_DATA_OFFSET]);//更新起始地址在block中的偏移

		G_upCosStatus.blockTotalLen = BuildUWord(&buffer[offset+APDU_DATA_OFFSET+2]);//更新待处理数据块总长度

		G_upCosStatus.dataOffset = 0;
	}
	else
	{
		if(G_upCosStatus.crc != BuildWord(&buffer[offset+APDU_DATA_OFFSET]))//处理E0D1数据中的CRC，和当前CRC进行比较
		{			
			return 0x6A80;//CRC错误
		}

		G_upCosStatus.groupNum++;//(groupNum-1)即为已下载块数，此操作方便服务器判断已下载块数
	}
	
	return 0x9000;
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

	memcpy(&g_UPCOS_remainEncData[0], &remainEncDataBuf[0], (remainEncDataBuf[0]+1));//将该分组剩余未解密的数据拷贝到全局数组，待收到下一分组再一起处理

	SymCipherDofinal(MEMtPM8(&decDataBuf[0]), decLen, MEMtPM8(&decDataBuf[0]), MEMtPM8(UPCOS_KEY), U8_M_U8(UPCOS_KEYLEN,8), NULL, NULL, ALG_DES_ECB_NOPAD, ECB_Decrypt, 0x00, ALG_DES);//从&srcDataBuf[0]开始对decLen长度的密文进行解密

	return decLen;
}

u16 NM_IsUPCOSCommand()
{
	u8      X_A     b_INS,b_P1,b_P2;
	u16     X_A     sw;

	sw           =      0x0000;
	b_INS        =      APDUBuffer[APDU_INS_OFFSET];
	b_P1         =      APDUBuffer[APDU_P1_OFFSET];
	b_P2         =      APDUBuffer[APDU_P2_OFFSET];

	if(((b_INS == 0xCA)&&(b_P1 == 0xFF)&&(b_P2 == 0xFE))||((b_INS == 0xE2)&&(b_P1 == 0xFF))||(b_INS == 0xEA)||(b_INS == 0xE9))
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

			case 0xE9://切到BOOTLOADER指令，BOOTLOADER不支持
				sw  =  0x6D00;
				break;
			
			default:
				break;
		}
	}
	return sw;
}


#endif

