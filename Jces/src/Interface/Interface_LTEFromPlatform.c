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
#include "Interface_LTEFromPlatform.h"
#include "Interpreter.h"
#include "Method.h"
#include "Object.h"
#include "rtrSpace.h"
#include "SEEUICCInterface.h"
#include "cardManager.h"
#include "anomaly.h"
#include "opFunction.h"
#include "SCPInterface.h"
#include "BootLifeCycle.h"
#include "keyGroup.h"
#include "keySet.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "SecurityDomainData.h"
#include "profileApi.h"

u8 CONST USIMPartial[8] = {0x07, 0xA0,0x00,0x00,0x00,0x87,0x10,0x02};  //101220  附录E

u8 CONST CSIMPartial[8] = {0x07, 0xA0,0x00,0x00,0x03,0x43,0x10,0x02};  //101220  附录E

u8 CONST ISIMPartial[8] = {0x07, 0xA0,0x00,0x00,0x00,0x87,0x10,0x04};  //101220  附录E

#if UICC__OPERATOR__CHINAMOBILE
/*
*@function   : IsUsimADF
*
*@description:判断当前AID是否是USIM adf的AID
*
*@param      :
*
*@return     :TRUE/FALSE
*
*@history    :V1.0:create by lgy@2020 - 11-23
*/
u8 IsUsimADF(u8 * aidBuf, u16 offset,u8 aidLen)
{
	u8 X_A aid[0x11];

	aid[0] = aidLen;
	ecpMem_Copy(&aid[1],&aidBuf[offset],aidLen);

	if((ecpMem_Compare(&USIMPartial[1],&aidBuf[offset],USIMPartial[0]) == 0) && (GetAIDNumber(aid,FALSE) != FALSE))//ADF USIM 的aid，
	{
		return TRUE;
	}
	return FALSE;
}
#endif
#if JAVA_CARD_UICC
/*
*@function   : GetIfMfCreated
*
*@description:返回私有文件区或者default文件区MF地址， 若没有创建，返回INVALID_ADDR
*
*@param      :u8 mode --- MODE_PRIVATE表示获取私有文件系统起始地址 / MODE_DEFAULT表示获取默认文件系统起始地址
*
*@return     :memref mfOffset --- 对应的文件系统存在的话就返回该文件系统的起始地址；否则返回INVALID_ADDR
*
*@history    :V1.0:create by xwh@20190306
*            :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
memref GetIfMfCreated(u8 mode)
{
	FileIdType X_A fileHead;

	if((mode != MODE_PRIVATE)&&(mode != MODE_DEFAULT))
	{
		return INVALID_ADDR;
	}
	
	if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)
	{
		return INVALID_ADDR;
	}

	ReadEEPromCache((u8 *)&fileHead,ADDR_PRIVATE_FILE_START,sizeof(fileHead));

	if((fileHead.FileId == DF_MF)&&((fileHead.FileType&MFTYPE)==MFTYPE))                                            //ò??-′′?¨á? Private File System
	{
		if(mode == MODE_PRIVATE)
		{
			return ADDR_PRIVATE_FILE_START;
		}
		else
		{
			return VMReadUword(RSPDefaultFileSystem_Addr);
		}
	}

	return INVALID_ADDR;
}

/*
*@function   :GetMaxProfileMFEnd
*
*@description:获取最后一个Profile的文件系统结束地址（最后一个Profile的3F00地址 + UsedSize）
*
*@param      :void
*
*@return     :memref mfOffset --- 最后一个Profile的文件系统结束地址
*
*@history    :V1.0:create by xwh@20190114
*             V1.1:更新函数，若Default文件区已创建，profile在Default文件区之后创建, modify by xwh 20190306
*/
memref GetMaxProfileMFEnd(void)
{	
	u8  I_A i;
	u8  X_A temp[PROFILE_MAX_NUMBER];
	memref X_A maxMFOffset,maxMFOffset2,usedSize,currentMFOffset;
	
	maxMFOffset = GetIfMfCreated(MODE_PRIVATE);
	
	if(maxMFOffset == INVALID_ADDR)                 //没有创建Private文件区，此时不允许下载profile
	{
		return ADDR_PRIVATE_FILE_START;
	}
	
	maxMFOffset2 = GetIfMfCreated(MODE_DEFAULT);
	
	if(maxMFOffset2 != INVALID_ADDR)               //Default文件区已创建
	{
		maxMFOffset = maxMFOffset2;
	}
	
#if (EUICC_ONLY||SE_AND_EUICC)
	VMReadBuf(temp,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态

	for(i= 0; i< PROFILE_MAX_NUMBER ; i++)
	{		  
		//if(temp[i] == PROFILE_VALIDFLAG)
		if(temp[i] != PROFILE_INVALIDFLAG)
		{
			currentMFOffset = VMReadUword(PROFILE_MFOFFSET_CFG + i*sizeof(memref));
			
			if((currentMFOffset > maxMFOffset)&&(currentMFOffset != 0xFFFFFFFF))
			{
				maxMFOffset = currentMFOffset;
			}
		}
	}
#endif
	usedSize = GetFileHeadUsedSize(maxMFOffset);
	
	return (maxMFOffset+usedSize);
}

/*
*@function   :GetFileSystemTotalSize
*
*@description:获取文件系统占用的总空间
*
*@param      :void
*
*@return     :memref 文件系统占用的总空间
*
*@history    :V1.0:create by lgy@2020-11-17
*
*/
memref GetFileSystemTotalSize(void)
{
	return(GetMaxProfileMFEnd() - ADDR_PRIVATE_FILE_START);
}
/*
*@description:SCP80key更新时候 如果和当前使用的key会话一致  需要保存更新前的key 发POR时候使用  此函数 借用发POR之前缓存key  位置 SCP80Keyoffset
*
*@param      :void
*
*@return     :void
*
*@history    :create by LCZ @20190119
*             modify by xuweihua@20210625: 优化空间， EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*
*/
void IF_SaveSCP80KeyValueBeforeUpdate(void)
{
	u8 I_A num;
	memref X_A keyAddr;

	G_NewSCPKey	 = FALSE;    //初值

	tempBuf[1] = GetKicKidKikKey((G_RFMRAMKeyversion<<4),&tempBuf[2],TYPE_KEY_KIC,0);  //key LV 结构  (G_RFMRAMKeyversion<<4)  GetKicKidKikKey 取会右移4

	if(tempBuf[1] != FALSE) // KIC存在
	{
		num = tempBuf[1] +2;

		tempBuf[0] = TYPE_KEY_KIC;
	}
	else
	{
		num = 0x00;
	}

	tempBuf[num+1] = GetKicKidKikKey((G_RFMRAMKeyversion<<4),&tempBuf[num+2],TYPE_KEY_KID,0);  //key LV 结构

	if(tempBuf[num+1] != FALSE)      //KID存在
	{
		tempBuf[num] = TYPE_KEY_KID;

		num += tempBuf[num+1] + 2;
	}
	else
	{

	}

	if(num != FALSE)
	{
		//keyAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);

		//if(keyAddr != FALSE)
		//{
		//	WriteTFBuf(keyAddr,SCP80Keyoffset,&tempBuf[0],num);

		//	G_NewSCPKey = GetSCP80KeyType((G_RFMRAMKeyversion<<4),ADDR_IS_NULL,ADDR_IS_NULL);//TRUE;
		//}
		keyAddr = BIPModule_Send_ADDR_START;		
		WriteEeprom(keyAddr+SCP80Keyoffset,  &tempBuf[0],num); 
		G_NewSCPKey = GetSCP80KeyType((G_RFMRAMKeyversion<<4),ADDR_IS_NULL,ADDR_IS_NULL);//TRUE;
	}
}
/*
*@Function:    GetEnabledProfileFileStart
 *
 *@Descrip:     获取当前enabled的Profile的MF起始地址(如果当前没有enable 的Profile,返回INVALID_ADDR)
 *
 *@See:	        ts_102222
 *
*@Param:        offset     ---> 写EF_PRIVATE_AUTHCFG文件的偏移
 *@Param:       value      ---> 要写的数据
 *@param        len        ---> 数据长度
 *
 *@Return:      void
 *
 *@author     :xwh
 *
 *@history    :V1.0:create by xwh@20190111
 */
memref GetMFFileStart(void)
{
	u8  I_A index = ILLEGAL_PROFILEINDEX;
	memref I_A mfAddr,mfAddr1,mfAddr2;
#if (EUICC_ONLY||SE_AND_EUICC)
	index = JcvmConfig_GetProfileEnableIndex();
#endif
	if(index == ILLEGAL_PROFILEINDEX)
	{
		mfAddr = INVALID_ADDR;

		G_CardStatus=D_CardEmpty;

		mfAddr1 = GetIfMfCreated(MODE_PRIVATE);

		if(mfAddr1 != INVALID_ADDR)                                            //已经创建了 Private File System
		{
			mfAddr = mfAddr1;

			G_CardStatus=D_CardInit;

			mfAddr2 = GetIfMfCreated(MODE_DEFAULT);

			if(mfAddr2 != INVALID_ADDR)                                                                                                                      //预制了果通文件系统
			{
				mfAddr = mfAddr2;
			}
		}
	}

#if (EUICC_ONLY||SE_AND_EUICC)
	else                                                                                                                                                     //已经创建了 Profile
	{
		mfAddr =  VMReadUword(PROFILE_MFOFFSET_CFG + index*sizeof(memref));

		G_CardStatus = D_CardProfileFile;
	}
#endif

	return mfAddr;
}
#endif // JAVA_CARD_UICC
#if (EUICC_ONLY||SE_AND_EUICC)
//u8 X_A G_NewSCPKey;     // add chentao 更新SCP80key后，发响应值需要用更新前的key，发完响应后用新的key

/*
*@function    :IF_GetDownloadProfileIndex
*
*@description :在PE执行过程中,获取当前正在执行操作的profile Index
*
*@param        :void
*
*@return       :u8 profileIndex
*
*@author       :xuyz
*
*@history      :V1.0 :create @20190308
*/
u8 IF_GetDownloadProfileIndex(void)
{
#if RSP_EUICC
	return G_RSPSessionContext.downloadProfileIndex;
#else
	return  G_DownLoadingProfileIndex;  //在RAM 时已经给G_DownLoadingProfileIndex 正确赋值了
#endif
}


/*
*@function   :SetDownLoadingProfileMFOffset
*
*@description:将当前正在下载的Profile 的mfOffset 写入PROFILE_MFOFFSET_CFG
*
*@param      :memref mfOffset --- 输入参数, 当前正在下载的profile的文件系统起始地址
*
*@return     :void
*
*@history    :V1.0:create by xwh@20190111
*/
void SetDownLoadingProfileMFOffset(memref mfOffset)
{
	//if(G_DownLoadingProfileIndex != ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(G_DownLoadingProfileIndex))
	{
		VMWriteUword(PROFILE_MFOFFSET_CFG + G_DownLoadingProfileIndex*sizeof(memref),mfOffset);
	}
}
 
/*@Function   :SetDownLoadingProfilePOL1                                           
 * 
 *@description:设置当前正在下载的Profile 的POL 1
 *
 *@See        :	        
 *                                                                                                                                     
 *@Param      :u8 POL1 --- POL1的值
 *				                                                                                                         
 *@Return     :void                                                                                                                                                                                                                                                     
 * 
 *@history    :V1.0:create by xwh@20190111        
 */ 
#if M2M_EUICC
void SetDownLoadingProfilePOL1(u8 POL1)
{
	memref X_A entryAddr;

	if(G_DownLoadingProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		entryAddr = GetProfile_M2M_POL1Addr(GetISDPObjectAddr(G_DownLoadingProfileIndex));//GetProfile_M2M_POL1Addr(GetProfileEntryAddr(G_DownLoadingProfileIndex));//
		memWriteU8Atomic(MEMtPM8(entryAddr),POL1);//VMSetBuf(PROFILE_POL1 + G_DownLoadingProfileIndex,POL1,1);
	}
}
#endif

/*@Function:GetProfileIndexByISDPTar                                           
 * 
 *@Descrip :获取tarBuf所指示的ISDP的Index
 *                                                                                                                                            
 *@Param   :u8* tarBuf --- ISD-P的TAR值
 *				                                                                                                         
 *@Return  :不存在返回PROFILE_INVALIDFLAG;否则返回对应的profile index
 * 
 *@history :V1.0:create by xwh@20190111 
 */ 
u8 GetProfileIndexByISDPTar(u8* tarBuf)
{
#if M2M_EUICC
	return  IsISDP_Tar(tarBuf,NORMAL_CHECK_ISDPTAR);
#endif
	return ILLEGAL_PROFILEINDEX;
}



/*
*@funtion    :IF_M2M_ISDP_Process86
*
*@description:参考：SGP02_V3.1.pdf， 4.1.3.3节  84tag时候或者86Tag 解析到PEEND时候 清空SCP03T
*
*@param      :u8 appid   --- 建立安全通道的 ISD_P APPID
*@param      :memref buffer --- eepromAddr [86 Lcc V MAC]
*@param      :u16 totallen--- L[86 Lcc V MAC]
*@param      :u8 *outPut --- [86 00] / [9F46 01 XX]
*
*@return     :void
*
*@history    :V1.0 created by xuyunzhen@20190221 实现该函数
*            :V1.1 modify by xuxiaojun@20190326 修改lenOf86的类型由WORD修改成UWORD
*            :V1.2 modify by liguiyu@20190401 修改解析PE时传入的86tag长度值
*            :V1.3 modify by liguiyu@20190423 scp03会话的ICV counter统一放在下行解密和算CMAC里面+1
*            :V1.3 modify by xuweihua@20190522 G_DownLoadingProfileIndex在86中再赋值
*            :V1.4 modify by xuweihua@20190527 上一个profile下载没有结束，又开始一个profile下载，此时将上一个profile回滚，开始新的profile下载 
             :V1.5 modify by licaizhang@20200513 添加GPClearSecureSessionInfo 为了兼容先来84xx 85xx 830101 86xx的情况,在830101时候不清空scp03t会话
*/
#if M2M_EUICC
void IF_M2M_ISDP_Process86(u8 appid, memref buffer, u16 totallen, u8 *outPut)
{
	u16 I_A decrytLen;
	memref X_A lenOf86;
	u8 I_A tmp;
	u8 I_A securityLevel,firstLoadingFlag;

	firstLoadingFlag = FALSE;
					
	if((G_DownLoadingProfileIndex == ILLEGAL_PROFILEINDEX) && (VMReadByte(PROFILE_VALIDTAG_CFG + IF_GetRAMProfileIndex()) == PROFILE_VALIDFLAG)) //M2M_IndependenceTest_06_01 -- TestPurpose:一个ISD-P下面只能下载一个profile
	{
		outPut[0] = 0x9F; //In case of an error, tag '9F46' is used.
		outPut[1] = 0x46; 
		outPut[2] = 0x01; 
		outPut[3] = 0x01;
		
		return; 
	}	

	if(G_DownLoadingProfileIndex == ILLEGAL_PROFILEINDEX)          //一个新的profile下载
	{
		if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)  //在当前激活的profile下进行卡片内容管理时，不允许开启一个新的下载profile流程（COS内部方案 无规范来源）
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);  
		}
	
		G_DownLoadingProfileIndex = G_TARRecIndex[1];

		IF_PEAnayleGlobalVariableInit(); //解析profile之前进行全局变量的初始化操作

		bRamBuffer[LOADING_PROFILE_INDEX] = G_DownLoadingProfileIndex;

		firstLoadingFlag = TRUE;
	}
	else if(G_DownLoadingProfileIndex != IF_GetRAMProfileIndex())  //上一个profile下载没有结束，又开始一个profile下载，此时将上一个profile回滚，开始新的profile下载 
	{
		IF_M2M_DeleteProfilePE(G_DownLoadingProfileIndex);			
	
		G_DownLoadingProfileIndex = IF_GetRAMProfileIndex();

		IF_PEAnayleGlobalVariableInit(); //解析profile之前进行全局变量的初始化操作

		bRamBuffer[LOADING_PROFILE_INDEX] = G_DownLoadingProfileIndex;

		firstLoadingFlag = TRUE;
	}
	else
	{

	}
	
	securityLevel = channelInfo.security_level[selected_channel];

	if((U8_AND_U8(securityLevel, SCF_AUTHENTICATED) != SCF_AUTHENTICATED))
	{
		outPut[0] = 0x9F; //In case of an error, tag '9F45' is used.
		outPut[1] = 0x46; 
		outPut[2] = 0x01; 
		outPut[3] = 0x02;
		
		return; 
	}
		
	tmp = ASN1_GetBERTlvLen(buffer + 1, &lenOf86);  //获取86tag的总长度
	
	if(lenOf86 < 16)//cmac 8字节 ，数据+填充 至少8字节
	{
		outPut[0] = 0x9F; //'01': error in length or structure of command data
		outPut[1] = 0x46; 
		outPut[2] = 0x01; 
		outPut[3] = 0x01;
		
		return ;
	}
	
	decrytLen = SCP03tDecryptAndCalMac(buffer,EUICCTempDataBuf,totallen,SECURITYLEVEL_CMAC|SECURITYLEVEL_CDECRYPTION,(u8 *)g_current_SCP_MAC_KEY, g_SCP_macKeyLen, (u8 *)g_current_SCP_LAST_MAC,(u8 *)g_current_SCP_SES_KEY, g_SCP_sesKeyLen,g_SCP_ICV_Counter);
	
	if(decrytLen == FALSE)
	{
		outPut[0] = 0x9F; //In case of an error, tag '9F45' is used.
		outPut[1] = 0x46; 
		outPut[2] = 0x01; 
		outPut[3] = 0x02;
		
		return ;
	}
	
	SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

	if(firstLoadingFlag == TRUE)
	{
		//Euicc_ConfigDataReset(JcvmConfig_GetEUICCRTP_ProfileInitialBack());
		RamManager_alloc(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)), tranMananger_getProfileRtp(G_CurExecutingProfileIndex), false);
	//	firstLoadingFlag = FALSE;
	}

	interpreterPE(EUICCTempDataBuf+1+tmp, decrytLen - 1- tmp,&outPut[1]);  //错误数据存放在outPut中  LV结构
	
	SEUICC_ResumeISDPAppletTable();

	ecpMem_Move(&outPut[1],&outPut[0],outPut[0]+1);
	
	tmp = IF_GetPfoileExcuteState();
	
	outPut[0] = 0x86;
	
	if(tmp == PROFILE_CREATING_STATE)//创建过程中不需要拼装响应
	{
		outPut[1] = 0x00;	//If there is no response data field, the length is ‘00’, and no R-MAC shall be generated
	}
	else//正常/异常结束都需要拼装响应
	{				
		SCP03tEncryptionAndRMAC(&outPut[2],outPut[1], SECURITYLEVEL_RMAC|SECURITYLEVEL_RENCRYPTION, outPut,(u8 *)g_current_SCP_RMAC_KEY, g_SCP_rmacKeyLen, (u8 *)g_current_SCP_LAST_MAC,(u8 *)g_current_SCP_SES_KEY, g_SCP_sesKeyLen,g_SCP_ICV_Counter);

		GPClearSecureSessionInfo(/*appid*/);   //清空scp03t会话  为了兼容外场测试先来 84xx  85xx  83010186xx的情况,830101时候不清空scp03t会话

		//SGP.02_v3.2_updated.pdf的Annex K : Script Chaining implementation (Informative)
		bRamBuffer[LOADING_PROFILE_INDEX] = ILLEGAL_PROFILEINDEX;
	}
}
/*
*@funtion    :IF_M2M_ISDP_Process85
*
*@description:参考：SGP02_V3.1.pdf， 4.1.3.3节，“85“ Tag处理
*
*@param      :u8 ssdId     --- 建立安全通道的 ISD_P APPID
*@param      :u8 *buffer   --- [85 Lcc V MAC]  -- V: Security level(1), Host cryptogram(8), MAC(8)
*@param      :u8 bufferLen --- L[85 Lcc V MAC]
*@param      :u8 *outPut   --- outPut -- [85 00] / [9F45 01 XX]
*
*@return     :void
*
*@history    :V1.0 created by xuyunzhen@20190221 实现该函数
*            :V1.1 modify by LCZ@20190627 修改扩展格式84/85长度判断位置, 移动到Process85函数中判断， 防止外层数据读越界。同步修改Process85
*/
void IF_M2M_ISDP_Process85(u8 ssdId, u8 *buffer, u8 bufferLen, u8 *outPut)
{
	u16 I_A sw;
	
	//3-从buffer中获取 ”85“ tag 参数
	if(buffer[2] != 0x33)//SGP02 4.1.3.3节 : The security level shall be set to '33': "C DECRYPTION, R ENCRYPTION, C MAC, and R MAC".
	{
		outPut[0] = 0x9F;
		outPut[1] = 0x45;
		outPut[2] = 0x01;
		outPut[3] = 0x02;//'01': error in length or structure of command data
		return;
	}

	APDUBuffer[OFFSET_CLA] = 0x84;
	APDUBuffer[OFFSET_INS] = 0x82;
	APDUBuffer[OFFSET_P1] = buffer[0x02]; // security lever
	APDUBuffer[OFFSET_P2] = 0x00;
	APDUBuffer[OFFSET_LC] = 0x10;
	ecpMem_Move(&APDUBuffer[OFFSET_CDATA], &buffer[0x03], 0x10);

	g_SCP_SCP03t_Flag = 1;

	sw = JcvmProcessRemoteAppManageAPDU();

	g_SCP_SCP03t_Flag = 0;

	if(sw == SW_NO_ERROR)
	{
		outPut[0] = 0x85;
		outPut[1] = 0x00;
	}
	else
	{
		outPut[0] = 0x9F; //In case of an error, tag '9F45' is used.
		outPut[1] = 0x45; 
		outPut[2] = 0x01; 
		outPut[3] = 0x02; // '02': security error
				
		return;
	}
	
	if(G_DownLoadingProfileIndex == IF_GetRAMProfileIndex())
	{
		IF_M2M_DeleteProfilePE(G_DownLoadingProfileIndex);			
	}
}

/*
*@funtion    :IF_M2M_ISDP_SubProcess87
*
*@description:参考：SGP02_V4.5.pdf， 4.1.3.3节，“87“ Tag处理
*
*@param      :BYTE *buffer   --- 80LV 81LV 82LV 83LV
*@param      :BYTE bufferLen --- L
*
*@return     :BYTE   ---  0:correct/ '01': error in length or structure of command data/'02': security error 
*
*@history    :V1.0 created by zhxx@20220701 实现该函数
*/
u8 IF_M2M_ISDP_SubProcess87(u8 * buffer, u16 totallen)
{
	u16 idata off,offInitMacChainValue,offPPKEncValue,offPPKMacValue,offPPKRmacValue,decrytLen;
	u8 idata transactionStarted;

	transactionStarted = 0;

	off = 0;

	if(buffer[off++] != 0x80)//point to tag80
	{
		return 1;
	}

	if(buffer[off] != 0x10)//point to len 80
	{
		return 1;
	}


	offInitMacChainValue = off+1;

	off += (buffer[off]+1);

	if(buffer[off++] != 0x81)//point to tag81
	{
		return 1;
	}

	if((buffer[off] != 0x10) && (buffer[off] != 0x20) && (buffer[off] != g_SCP_sesKeyLen))//point to len 81,new keys are of same length as the old keys
	{
		return 1;
	}


	offPPKEncValue = off+1;

	off += (buffer[off]+1);

	if(buffer[off++] != 0x82)//point to tag82
	{
		return 1;
	}

	if((buffer[off] != 0x10) && (buffer[off] != 0x20)&& (buffer[off] != g_SCP_macKeyLen))//point to len 82，new keys are of same length as the old keys
	{
		return 1;
	}


	offPPKMacValue = off+1;

	off += (buffer[off]+1);

	if(buffer[off++] != 0x83)//point to tag83
	{
		return 1;
	}

	if((buffer[off] != 0x10) && (buffer[off] != 0x20)&& (buffer[off] != g_SCP_rmacKeyLen))//point to len 83，new keys are of same length as the old keys
	{
		return 1;
	}


	offPPKRmacValue = off+1;

	off += (buffer[off]+1);

	if(off != totallen)
	{
		return 1;
	}

	if (!isInProgress())
	{
		memBeginTransaction(SINGLE_APDU_PROCESS);
		transactionStarted = 1;
	}
	
	//the encryption counter for ICV calculation is reset to its initial state (i.e. the value on 16 bytes is '00…01').
	ecpMem_Set(g_SCP_ICV_Counter,0,SCP03_ICV_COUNTER_LENGTH);


	//all keys (S-ENC, S-MAC and S-RMAC) have to be replaced. This command doesn't allow to replace only a part of the session keys.
	memCopyAtomic(MEMtPM8(g_current_SCP_LAST_MAC), &buffer[offInitMacChainValue], 0x10); // modify by chentao@20221008: g_current_SCP_LAST_MAC可能是RAM，可能是E2

	memCopyAtomic(MEMtPM8(g_current_SCP_SES_KEY), &buffer[offPPKEncValue], buffer[offPPKEncValue-1]);

	memCopyAtomic(MEMtPM8(g_current_SCP_MAC_KEY), &buffer[offPPKMacValue], buffer[offPPKMacValue-1]);

	memCopyAtomic(MEMtPM8(g_current_SCP_RMAC_KEY), &buffer[offPPKRmacValue], buffer[offPPKRmacValue-1]);

	if(transactionStarted)
	{
		memCommitTransaction();
	}

	return 0;
}
/*
*@funtion    :IF_M2M_ISDP_Process87
*
*@description:参考：SGP02_V4.5.pdf， 4.1.3.3节，“87“ Tag处理
*
*@param      :BYTE ssdId     --- 建立安全通道的 ISD_P APPID
*@param      :BYTE *buffer   --- [87 Lcc V MAC]  -- V: 80LV 81LV 82LV 83LV
*@param      :BYTE bufferLen --- L[87 Lcc V MAC]*
*@return     :BYTE   ---  0:correct/ '01': error in length or structure of command data/'02': security error 
*
*@history    :V1.0 created by zhxx@20220701 实现该函数
*/
u8 IF_M2M_ISDP_Process87(u8 appid, memref buffer, u16 totallen)
{
	u16 idata decrytLen;
	u8 idata tmp,result;
	memref xdata lenOf87;

	tmp = ASN1_GetBERTlvLen(buffer + 1, &lenOf87);  //获取87tag的总长度

	if(lenOf87 < 16)//cmac 8字节 ，数据+填充 至少8字节
	{
		return 1;
	}

	decrytLen = SCP03tDecryptAndCalMac(buffer,EUICCTempDataBuf,totallen,SECURITYLEVEL_CMAC|SECURITYLEVEL_CDECRYPTION,(BYTE *)g_current_SCP_MAC_KEY, g_SCP_macKeyLen, (BYTE *)g_current_SCP_LAST_MAC,(BYTE *)g_current_SCP_SES_KEY, g_SCP_sesKeyLen,g_SCP_ICV_Counter);

	if(decrytLen == FALSE)
	{
		return 2;
	}

	ReadEeprom(&RecData[0], EUICCTempDataBuf,decrytLen);

	result = IF_M2M_ISDP_SubProcess87(RecData+1+tmp, decrytLen-1-tmp);

	return result;

}
/*
*@funtion    :IF_M2M_ISDP_Process84
*
*@description:参考：SGP02_V3.1.pdf， 4.1.3.3节，“84“Tag处理
*
*@param      :u8 ssdId     --- 建立安全通道的 ISD_P APPID
*@param      :u8 *buffer   --- 84 Lcc V] V = Key version number(1) + Key identifier(1) + Host challenge(8)
*@param      :u8 bufferLen --- L[84 Lcc V]
*@param      :u8 *outPut   --- [84 20 V] / [9F44 01 XX]
*
*@return     :void
*
*@history    :V1.0 created by xuyunzhen@20190221 实现该函数
*            :V1.1 modify by LCZ@20190627 修改扩展格式84/85长度判断位置, 移动到Process84函数中判断， 防止外层数据读越界。同步修改Process84
*/
void IF_M2M_ISDP_Process84(u8 ssdId, u8 *buffer, u8 bufferLen, u8 *outPut)
{
	u16 I_A sw;
	
	if(buffer[3] != 0x00)  //Key identifier ：  Mandatory (and value must be ‘00’, consistently with SCP03)
	{
		outPut[0] = 0x9F; //In case of an error, tag '9F44' is used.
		outPut[1] = 0x44; 
		outPut[2] = 0x01; 
		outPut[3] = 0x03; //'01': error in length or structure of command data
				
		return;
	}
	
	APDUBuffer[OFFSET_CLA] = 0x80;
	APDUBuffer[OFFSET_INS] = 0x50;
	APDUBuffer[OFFSET_P1] = buffer[2];//KVN
	APDUBuffer[OFFSET_P2] = buffer[3];//KID
	APDUBuffer[OFFSET_LC] = 0x08;
	ecpMem_Move(&APDUBuffer[OFFSET_CDATA], &buffer[0x04], 0x08);

	sw = JcvmProcessRemoteAppManageAPDU();

	if((sw&0xFF00) != 0x6100)
	{
		outPut[0] = 0x9F; //In case of an error, tag '9F44' is used.
		outPut[1] = 0x44; 
		outPut[2] = 0x01; 
		outPut[3] = 0x03; //'01': error in length or structure of command data
	}
	else
	{
		outPut[0] = 0x84;
		outPut[1] = sw&0xFF;
		
		ecpMem_Copy(&outPut[2],&RecData[0],sw&0xFF);
	}	
}
/*
*@function :IF_UpdateConnectPara

*@desciption: 根据profileHeader中的信息更新连接参数
   
*@param    :u8 *inBuf
@param     :u16 inLen
*
*@return    :执行结果
*
*@author    :zhxx
*
*@history    :V1.0 :create by zhxx@20190123
*/
u8 IF_UpdateConnectPara(u8 *inBuf,u16 inLen)
{
	//NOTE: 连接参数的有效性此处未判断，
	
	if(inLen > 0xFF)  //暂时不支持大于255字节的链接参数的更新，如果后续要支持超过255长度的连接参数的更新，要同步修改M2M_StoreConnectivityParameters
	{
		return FALSE;
	}
	
	M2M_StoreConnectivityParameters(G_DownLoadingProfileIndex,inBuf, inLen);
	
	return TRUE;
}

#endif

/*
*@function  :IF_GetDataFromMetadata
*
*@desciption:获取正在下载的profile的metadata中的信息: 比如iccid、gid、mccmnc、ppr
*
*@param     :u8 type    --- 获取数据的指示类型
*@param     :u8 *outPut --- 输出参数，对应类型的期望格式数据。目前上层传入的都是tempBuf
*
*@return    :outPut中有效数据的长度(取ICCID ，PPR_TYPE时反长度 其他TRUE/FALSE )
*
*@history   :V1.0 :create by zhxx@20190123
             V1.1  add functionImpl Code by xuyz@20190228
*            V1.2  增加获取当前正在下载的profile class功能。在这种情况下，outPut没有意义 
*            V1.3  modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
#if RSP_EUICC
u8 IF_CustomerGetDataFromMetadata(u8 type,u8 *outPut)
{
	u16 I_A result;
	
	result = FALSE;
	
	switch(type)
	{
		case GID1_TYPE:        //Note： 根据SGP22_V2.2的规范 ，If gid1 or gid2 is provided in the Profile Metadata: The corresponding EFGID1 or EFGID2SHALL be present and contain the same value as provided in the data object and the related service in EFUST SHALL indicate "available".
		{
			result = GetProfileOwnerSubField(G_DownLoadingProfileIndex,OperatorId_GID1_TAG,outPut,0);  //tempBuf存储的是TLV 结构的:即数据为81XX or 82 XX
	
			if(result != FALSE)
			{			
				return TRUE;  //GID_TYPE目前profile的实现仅要求返回TRUE/FALSE 即可,后续可调整
			}
			
			break;
		}

		case GID2_TYPE:        //Note： 根据SGP22_V2.2的规范 ，If gid1 or gid2 is provided in the Profile Metadata: The corresponding EFGID1 or EFGID2SHALL be present and contain the same value as provided in the data object and the related service in EFUST SHALL indicate "available".
		{
			result = GetProfileOwnerSubField(G_DownLoadingProfileIndex,OperatorId_GID2_TAG,outPut,0);  //tempBuf存储的是TLV 结构的:即数据为81XX or 82 XX

			if(result != FALSE)
			{
				return TRUE;  //GID_TYPE目前profile的实现仅要求返回TRUE/FALSE 即可,后续可调整
			}

			break;
		}

		case TYPE_PROFILE_CLASS:
		{
			return GetSingleBYTEProfileMeta(G_DownLoadingProfileIndex, MetaDataProfileClass_INDEX,2);
		}
		
		case MCCMNC_TYPE:
		{
			result = GetProfileOwnerSubField(G_DownLoadingProfileIndex,OperatorId_MCCMNC_TAG,outPut,0); //此处根据外界调用关系，outPut 期望的是TLV 结构的数据
		
//			if(result != FALSE)
//			{
//				ecpMem_Move(outPut, tempBuf, result);
//			}
			
			break;
		}
		case ICCID_TYPE:
		{
			result = GetProfileMetadaTLV(G_DownLoadingProfileIndex,MetaDataIccid_INDEX,outPut);  //outPut里面存放的数据是TLV 格式，但是该函数要求返回的V
			
			if(result == FALSE)
			{
				return FALSE;
			}			
			result = outPut[1]; 
			
			ecpMem_Move(&outPut[0], &outPut[2], result);
			
			break;
		}
		case PPR_TYPE:
		{
//			result = GetProfileMetadaTLV(G_DownLoadingProfileIndex,MetaDataProfilePolicyRules_INDEX,outPut);  //outPut里面存放的数据是BYTE profilePolicyRules[4];但是该函数要求返回的V
//
//			if(result == FALSE)
//			{
//				return FALSE;
//			}
//
//			result = 0x01;
//
//			outPut[0] = outPut[3];
			
			outPut[0] = GetSingleBYTEProfileMeta(G_DownLoadingProfileIndex,MetaDataProfilePolicyRules_INDEX,3);

			result = 0x01; 

			break;
		}
		default: 			  //此处为了白盒测试添加
			break;			
	}
	
	return result;
	
}



/*
*@function    :IF_Customer_IMSISatisfyCondition
*
*@description :IMSI是否满足Section 8.3.2.2 of 3GPP TS 34.108中的条件      
*File size: 9 bytes
*Default values: Byte 1 (DEC): 8
*Bytes 2 to 9 (HEX): 09 10 10 ** ** ** ** **
*                    49 24 10 ** ** ** ** ** (for Band VI and Band IX)
"*" indicates any number between 0 and 9 subject to the restriction that IMSI mod 1000 (i.e. bytes 7, 8 and 9) lies in
one of the following ranges:
- 063 to 125, 189 to 251, 315 to 377, 441 to 503, 567 to 629, 693 to 755, 819 to 881 or 945 to 999.
*
*@param       :void
*
*@return      TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190718
*/
u8 IF_Customer_IMSISatisfyCondition()
{
	memref X_A fileAddr,lastNum;
	u8 X_A IMSIBuf[9];
	u16 I_A usimId;
	
	usimId = GetADFFID((u8 *)&USIMPartial[0]);
				
	if(usimId != FALSE)
	{
		fileAddr = SelectTFFile(usimId,EF_IMSI);		//找到USIM文件
		if(fileAddr != FALSE)
		{
			ReadTFBuf(fileAddr,0,IMSIBuf,sizeof(IMSIBuf));
			if(((IMSIBuf[1] == 0x09) && (IMSIBuf[2] == 0x10) && (IMSIBuf[3] == 0x10)) || ((IMSIBuf[1] == 0x49) && (IMSIBuf[2] == 0x24) && (IMSIBuf[3] == 0x10)))
			{
				lastNum =  (((IMSIBuf[6]<<16)&0xFF0000)|((IMSIBuf[7]<<8)&0xFF00)|IMSIBuf[8]);
				if(((lastNum >= 63) && (lastNum <= 125)) || ((lastNum >= 189) && (lastNum <= 251)) || ((lastNum >= 315) && (lastNum <= 377)) || ((lastNum >= 441) && (lastNum <= 503)) 
					|| ((lastNum >= 567) && (lastNum <= 629)) || ((lastNum >= 693) && (lastNum <= 755)) || ((lastNum >= 819) && (lastNum <= 881)) || ((lastNum >= 945) && (lastNum <= 999)))
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}
	}
	
	return FALSE;
		
}

/*
*@function  :IF_CustomerCheckTestProfile
*
*@desciption:个人版中检查TestProfile是否满足SGP2.2中2.4.5.3 Test Profile，鉴权的要求
*
*@param     :void
*
*@return    :TRUE/FALSE
*
*@history   :V1.0 :create by zhxx@20190722
            
*/
u8 IF_CustomerCheckTestProfile()
{
	u8 I_A profileClass,i;
	
	profileClass = IF_CustomerGetDataFromMetadata(TYPE_PROFILE_CLASS,NULL);
	
	if(profileClass == PROFILE_CLASS_TEST)
	{		
		if(IF_Customer_IMSISatisfyCondition() == FALSE)
		{

			ecpMem_Set(tempBuf+G_PETlvInfo.findValueLen,0,0x0C);//All bits except the lowest 32 bits set to zero.
			if(ecpMem_Compare(tempBuf,tempBuf+G_PETlvInfo.findValueLen,0x0c) != EQUAL)
			{
				for(i = 0;i < 0x10;i++)
				{
					tempBuf[G_PETlvInfo.findValueLen+i] = i;
				}
				if(ecpMem_Compare(tempBuf,tempBuf+G_PETlvInfo.findValueLen,0x10) != EQUAL)//'00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F' (default K value of Test USIM as defined in Section 8.2 of 3GPP TS 34.108 [62]).
				{
					return FALSE;
				}
			}

		}
	}
	
	return TRUE;
	
}

#endif

/*
*@function :IF_PEUpdateCATTPPersonalisation

*@desciption: 更新8.6 Security domains 中的8.6.7  CAT_TP personalisation参数到正在下载的profile的指定字段中
   
*@param    :u16 catTpMaxSduSize
@param     :u16 catTpMaxPduSize
*
*@return    :void
*
*@history    :V1.0 :create by zhxx@20190423
*/
void IF_PEUpdateCATTPPersonalisation(u16 catTpMaxSduSize, u16 catTpMaxPduSize)
{
	PEUpdateCATTPPersonalisation(catTpMaxSduSize,catTpMaxPduSize);
}

/*
*@function   : IF_PEUpdateContactlessProtocolParameters
*
*@description: 更新8.6 Security domains 中的8.6.6  contactlessProtocolParameters 参数到正在下载的profile的指定字段中
*
*@param      :u8 *contactlessProtocolParameters :LV结构 ,值域分别是 'Initial Contactless Activation State'0x81, Contactless protocol Type State(0x89)  Protocol Data type A(0x86),  Protocol Data type B(0x87), Protocol Data type F (0x88), Continuous Processing(0x84)
*
*@return     :TRUE/FALSE
*
*@author     :zhxx
*
*@history    :Note ： 
			 Protocol Data Type A 
			 Protocol Data Type  B 的处理要考虑，
			 V1.0 :create by zhxx@20190709
*/
//u8 IF_PEUpdateContactlessProtocolParameters(u8 *contactlessProtocolParameters)
//{
//	return PEUpdateOpenPersoData(contactlessProtocolParameters);
//}

/*
*@function   : IF_PEUpdateRestrictParameter
*
*@description: 更新8.6 Security domains 中的8.6.6  restrictParameter 到正在下载的profile的指定字段中
*
*@param      :u8 restrictParameter
*
*@return     :void
*
*@author     :zhxx
*
*@history  	 V1.0 :create by zhxx@20190709
*/
//void IF_PEUpdateRestrictParameter(u8 restrictParameter)
//{
//	VMSetBuf(PROFILE_Restrict_D9Para + IF_GetDownloadProfileIndex(), restrictParameter, 1); //更新restrictParameter到正在下载profile的PROFILE_Restrict_D9Para处
//}

/*
*@function  :IF_SetProfileCapacityInfo

*@desciption: profile执行结束后，设置lte相关的能力信息
*
*@param     :u8 *inBuf
@param      :u16 inLen
*
*@return    :void
*
*@history   :V1.0 :create by zhxx@20190123
*/
void IF_SetProfileCapacityInfo(u16 DIR)
{
	//if(G_DownLoadingProfileIndex != ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(G_DownLoadingProfileIndex))
	{
		memWriteU16Atomic(MEMtPM8(GetProfile_LTEAblityAddr(GetISDPObjectAddr(G_DownLoadingProfileIndex))),DIR);//memWriteU16Atomic(MEMtPM8(GetProfile_LTEAblityAddr(GetProfileEntryAddr(G_DownLoadingProfileIndex))),DIR);//
	}
}
/*
*@function  :IF_SetDownLoadingProfileToValidState

*@desciption: profile执行结束后，设置profile的状态为有效状态
*
*@param     :void
*
*@return    :void
*
*@history   :V1.0 :create by zhxx@20190123
*/
void IF_SetDownLoadingProfileToValidState(void)
{
	//if(G_DownLoadingProfileIndex != ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(G_DownLoadingProfileIndex))
	{
		SetProfileInfoAfterLoadPE(G_DownLoadingProfileIndex);	
		RamManager_free(MEMtPX8(rtrSpace_getRtrSpaceEntry(PROFILE_EXECUTION_SPACE_ID)));
		G_DownLoadingProfileIndex = ILLEGAL_PROFILEINDEX;
	}
}
/*
*@function    :IF_Get_EUICCEID
*
*@description :获取EID内容
*
×@param       :u8 *identification: LV结构
*
*@return      :void
*
*@author      :licaizhang
*
*@history     :
*/
void IF_Get_EUICCEID(u8 *identification)
{
	VMReadBuf(identification , EUICCEIDInfo_Addr, EUICCEIDLVLEN);
}
/*
*@function  :IF_ExcuteProfileGPCommand
*
*@desciption:处理Profile中与GP 相关的指令，如PutKey StoreData中，在进入该函数之前，数据体已经拼装在RecData中，APDU 头放于 CLA INS P1 P2 P3中
*
*@param     :void
*
*@return    :statusWord
*
*@history   :V1.0 :create by zhxx@20190123
*/

u16 IF_ExcuteProfileGPCommand(u8 excuteSdId)
{
	return PEExcuteGPCommand(excuteSdId);
}
/*
*@function  :IF_GPGetPkgIDByPkgAID

*@desciption:在当前正在下载的profile里，通过包的AID获取到该包对应的PKGID
*@param     :u8* aidBuf --- 缓存包的AID字节
*@param		:u8 aidLen  --- aidBuf中有效数据长度
*@param		:u8 majorVersion  --- 包的最大版本号
*@param		:u8 minorVersion  --- 包的最小版本号
*@param		:u8 mode          --- 包的最小版本号
*
*@return    :找到则返回对应的pkgId，否则返回ILLEGAL_PKGID
*
*@history    :V1.0 :create by zhxx@20190325
*/

u8 IF_GPGetPkgIDByPkgAID(u8* aidBuf, u8 aidLen,u8 majorVersion, u8 minorVersion/*,u8 mode*/)
{
	u8 I_A pkgId = ILLEGAL_PKGID;
	
    pkgId = findPkgID(aidBuf,0,aidLen,U8_OR_U8(U8_SHL_U8(majorVersion, 4), minorVersion));

	return pkgId;
}

/*@Function  :IF_GetAnalyzeAppIdByAIDList                                           
 * 
 *@Descrip   :获取当前正在下载的profile中aidBuf所指示的appId
 *
 *@See       :	        
 * 
 *@Param:    :u8 *aidBuf --- aid值
 *@param     :u8 aidLen  --- aidBuf中有效数据长度
 *				                                                                                                         
 *@return    :如果存在则返回对应的appid，否则返回ILLEGAL_APPID                                                                                                                                                                                                                                                           
 * 
 *@History:     V1.0---Create by zhxx 20190123             
 */ 
u8 IF_GetAnalyzeAppIdByAIDList(u8 *aidBuf,u8 aidLen)
{
	return GetProfileAppIdByAID(aidBuf,aidLen,G_DownLoadingProfileIndex);
}

/*@Function :IF_SetAnalyzeExtraditeSDID                                           
 * 
 *@Descrip  :设置PE解析过程中的G_profileGPState.peExtraditeSDID
 *
 *@See      :	        
 * 
 *@Param    :u8 appid --- 解析PE过程中得到的appId
 *				                                                                                                         
 *@Return   :void                                                                                                                                                                                                                                                            
 *  
 *@History  :V1.0---Create by zhxx 20190228      
 */ 
void IF_SetAnalyzeExtraditeSDID(u8 appid)
{
	G_profileGPState.peExtraditeSDID = appid;
}
/*@Function :IF_SetAnalyzeLifecycle                                           
 * 
 *@Descrip  :设置解析过程中的应用的lifeCycle
 *
 *@See      :	        
 * 
 *@Param    :u8 lifeCycle --- 应用的生命周期状态
 *				                                                                                                         
 *@Return   :void                                                                                                                                                                                                                                                            
 *  
 *@History  :V1.0 Create by zhxx 20190320      
 */ 
void IF_SetAnalyzeLifecycle(u8 lifeCycle)
{
	G_profileGPState.peLifeCycle = lifeCycle;
}
/*@Function:IF_SetKeyCounter                                           
 * 
 *@Descrip :设置当前正在解析PE的key counter的初始值 
 *
 *@See     :	        
 * 
 *@param   :u8* counter --- counter值
 *@param   :u8 couterLen--- counter中有效数据长度
 *				                                                                                                         
 *@Return  :设置成功返回TRUE否则返回FALSE                                                                                                                                                                                                                                                            
 *  
 *@History :V1.0 Create by zhxx 20190123             
 */ 
void IF_SetKeyCounter(u8* counter,u8 couterLen)
{
	G_profileGPState.keyCounter[0] = couterLen;
	
	ecpMem_Copy(&G_profileGPState.keyCounter[1], counter, couterLen);
}

/*@Function:IF_InitKeyCounter                                           
 * 
 *@Descrip :解析PE时初始化key counter
 *
 *@See     :
 * 
 *@Param   :void
 *				                                                                                                         
 *@Return  :void                                                                                                                                                                                                                                                            
 * 
 *@History :V1.0---Create by zhxx 20190123             
 */ 
void IF_InitKeyCounter(void)
{
	ecpMem_Fill(&G_profileGPState.keyCounter[0], 0x00, sizeof(G_profileGPState.keyCounter));
}

#if M2M_EUICC
/*
*@function   :IF_M2M_GetEnableISDPHttpConnectPara
*
*@description:获取当前激活的isdp的http链接参数,注意不是函数IF_GetHttpTriggerParaBySDID()中的链接参数，而是通过ES8中接口(3A07)Connectivity Parameters Update获得的isdp发送通知的http链接参数
*
*@param      :u8 *para --- 输出参数，
*
*@return     :返回获取到的链接数据的长度
*
*@author     :
*
*@history    :created by jiangqingbin@20190121
*/
u8 IF_M2M_GetEnableISDPHttpConnectPara(u8 *para)
{
	u8 I_A len;
	u16 I_A paraRef;
	memref I_A paraAddr;

	paraAddr = GetProfileConnectivityParaAddr(G_CurEnabledProfileIndex);//GetAddrAndIndexByIndexAddr(PROFILE_METADATAREF+ G_CurEnabledProfileIndex * sizeof(u16), &paraRef);

	if(paraAddr == ADDR_IS_NULL)
	{
		return 0x00;
	}

	len = VMReadByte(paraAddr);

	VMReadBuf(para, paraAddr+1, len);			//返回格式是 TLV

	return len;
}
#endif
/*
*@function   :IF_UpdateICCIDToMetaData
*
*@description:LTE更新ICCID时候同步更新MetaData
*
*@param      :BYTE dataLen --- iccid中有效数据长度
*@param      :BYTE * indata  --- iccid的值
*@param      :BYTE * offset  --- 修改iccid的偏移
*@param      :WORD fileName  --- 更新的文件ID
*@return     :void
*
*@author     :LCZ_2019-4-1
*
*@history    :created by xuxiaojun@20190402 如果修改的是默认文件系统的ICCID，当前是没有激活的profile的
*@history    V1.1 modify by chentao@20230105 修改只更新2Fe2两个字节导致metadata中ICCID长度改成两个字节的bug
*/
void IF_UpdateFileToMetaData(u8 dataLen,u8 * indata,u8 offset,u16 fileName)
{
#if RSP_EUICC
	memref X_A metaDataAddr,fileAddr;
	u8 X_A tmpData[11],mncLen;
	
	//if(G_CurEnabledProfileIndex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(G_CurEnabledProfileIndex) == FALSE)
	{
		return;
	}
	
	if(fileName == EF_ICCID)//同步ICCID
	{
		metaDataAddr = GetProfileMetaDataAddr(G_CurEnabledProfileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + G_CurEnabledProfileIndex*sizeof(u16));

		metaDataAddr = GetMetaData_iccidAddr(metaDataAddr) + 1;//+= (MetaData_Iccid_Offset + 1);   // 走到这里时metaDataAddr肯定是合法值
	
		VMReadBuf(&tmpData[0], metaDataAddr, ICCID_LEN+1 ); //read ICCID，LV结构，L的值为0x0A   // modify by chentao@20230105 修改只更新2Fe2两个字节导致metadata中ICCID长度改成两个字节的bug
		
		ecpMem_Copy(&tmpData[1+offset], indata,dataLen); // 替换需要修改的字节，其它字节保持不变
		
		VMWriteBuf(metaDataAddr, tmpData, ICCID_LEN + 1);
	}
	else if(fileName == EF_IMSI)//同步MCCMNC
	{
		
		fileAddr = SelectTFFile(G_DF_ID_USIM,0x6FAD);
		
		if(fileAddr == FALSE)
		{
			return;
		}
		
		ReadTFBuf(fileAddr,0x03,&mncLen,1);//length of  MNC in the IMSI
		
		MNNMNCConvert(&indata[1],&tmpData[2],mncLen);
		
		tmpData[0] = 0x80;
		
		tmpData[1] = 3;
		
		//metaDataAddr = GetMetaData_profileOwerRefValueAddr(metaDataAddr) + 1;//+= (MetaData_ProfileOwerRef_Offset + 1);
		
		GetProfileOwnerSubField(G_CurEnabledProfileIndex,OperatorId_MCCMNC_TAG,tmpData,1);
		
	}
	/*else if(fileName == 0x6F3E)//同步GID1
	{
		tempBuf[0] = 0x81;
		tempBuf[1] = dataLen;
		ecpMem_Copy(tempBuf,indata,dataLen);
		GetProfileOwnerSubField(G_CurEnabledProfileIndex,OperatorId_GID1_TAG,tempBuf,1);
	}
	else if(fileName == 0x6F3F)//同步GID2
	{
		tempBuf[0] = 0x82;
		tempBuf[1] = dataLen;
		ecpMem_Copy(tempBuf,indata,dataLen);
		GetProfileOwnerSubField(G_CurEnabledProfileIndex,OperatorId_GID2_TAG,tempBuf,1);
	}
	else if(fileName == 0x6FAD)//同步MCCMNC
	{
		
	}*/
	else
	{
		
	}
			
#endif	
}

/*
*@function   : IF_ProfileChangeMechanism
*
*@description: Profile发生切换时或者失去网络连接时需要做的一些操作 ，ROLL-BACK/FALL-BACK
*
			 : 卡片在执行EnableProfile以后如果失去网络连接 会先进行ROLL BACK，如果ROLL BACK以后依然连不上网 则进行FALL BACK操作
*            : 卡片在没有进行任何Profile切换的时候 重启手机连不上网 ，进行FALL BACK操作

*@return     :  TRUE/FALSE 表示需要REFRESH/不需要REFRESH
*
*@author     :lgy@20190417
*
*@history    :created by lgy@20190402
*/

#if M2M_EUICC
u8 IF_ProfileChangeMechanism(void)
{
	u8 I_A lastDisabledProfileIndex,notificationType,fallBackProfileIndex;
	u8 I_A curPOL1;
	
	notificationType = JcvmConfig_GetM2MNotificationType();
	
	if(notificationType == NOTIFICATION_RROFILE_CHANGE_SUCCEEDED)//切换Profile后 refresh后网络连接不成功 ，ROLL BACK
	{
		lastDisabledProfileIndex =  JcvmConfig_GetM2MLastDisableProfileIndex();
		
		if(lastDisabledProfileIndex == ILLEGAL_PROFILEINDEX)//正常情况下 进行了enable操作 这里的值不可能是非法值,如果是进行了disable操作导致连不上网 则表示fallback属性的profile也没办法连网
		{
			return TRUE;// reset重新使用FALLBACK的去尝试
		}			
		
		JcvmConfig_SetM2MNotificationType(NOTIFICATION_RROFILE_CHANGE_FAILED_AND_ROLLBACK);//VMWriteBuf(M2MNotificationType,&Temp[1],1);     //ROLL BACK,同时把notificationType改成NOTIFICATION_RROFILE_CHANGE_FAILED_AND_ROLLBACK

		JcvmConfig_SetUpdataEnableProfileIndex(lastDisabledProfileIndex);//Temp[2] = lastDisabledProfileIndex; 							//UpdateEnableProfileIndex

		return TRUE;
	}
	//else if((notificationType == NOTIFICATION_RROFILE_CHANGE_FAILED_AND_ROLLBACK)||(notificationType == NOTIFICATION_NON))// ROLL BACK以后还是无法连网 或者Profile突然无法连网
	//Profile突然无法连网， 这种情况不做Fall back
	else if(notificationType == NOTIFICATION_RROFILE_CHANGE_FAILED_AND_ROLLBACK)
	{
		fallBackProfileIndex = JcvmConfig_GetM2MFallBackProfileIndex();
		
		lastDisabledProfileIndex = ILLEGAL_PROFILEINDEX;
		
		curPOL1 = GetProfilePOL1(G_CurEnabledProfileIndex);
	
		if((curPOL1 & POL1_DISABLE_NOT_ALLOWED) == POL1_DISABLE_NOT_ALLOWED)  // Verify that POL1 of the currently Enabled Profile allows its disabling 
		{
			lastDisabledProfileIndex = G_CurEnabledProfileIndex;  
		}		
		
		//FALL BACL,同时把notificationType改成NOTIFICATION_RROFILE_CHANGE_AFTER_FALLBACK
		JcvmConfig_SetM2MFallBackDisabledNoAllowProfileIndex(lastDisabledProfileIndex);//Temp[0] = lastDisabledProfileIndex;		 //M2MFallBackDisabledNoAllowProfileIndex  ,M2MLastDisableProfileIndex ,M2MNotificationType
		JcvmConfig_SetM2MNotificationType(NOTIFICATION_RROFILE_CHANGE_AFTER_FALLBACK);//Temp[2] = NOTIFICATION_RROFILE_CHANGE_AFTER_FALLBACK;				//M2MNotificationType
		JcvmConfig_SetUpdataEnableProfileIndex(fallBackProfileIndex);//Temp[3] = fallBackProfileIndex; 									//UpdateEnableProfileIndex
		
		return TRUE;
	}
	else //fall back以后依然没有网络  正常不会出现这种情况 
	{
		return FALSE;
	}

}

/*
*@function   :IF_GetNotifitionType
*
*@description:获取通知类型
*
*@param      :void
*
*@return     :void
*
*@author     :LCZ_2019-4-1
*
*@history    :created by xuxiaojun@20190402
*/
u8 IF_GetNotifitionType(void)
{
	return JcvmConfig_GetM2MNotificationType();
}
/*
*@function   :IF_M2M_IncreaseNotificateNumerber
*
*@description:M2M的通知序列号+1
*
*@param      :void
*
*@return     :void
*
*@history    :created by xuxiaojun@20190505
*/
void IF_M2M_IncreaseNotificateNumerber(void)
{
	memref X_A seqNumber;
	
	seqNumber = VMReadUword(EUICCNotificationSeqNumber);
	
	if(seqNumber < 0xFFFF)  // When a Notification Confirmation has been successfully received by the SM-SR, the eUICC shall increment the sequence number for the next notification
	{
		seqNumber++;
	}
	else                   //设备版的seqNumber仅支持两个字节
	{
		seqNumber = 0;
	}
	
	VMWriteUword(EUICCNotificationSeqNumber, seqNumber); 
}
#endif

/*
*@function   :IF_M2M_DeleteProfilePE
*
*@description:删除profileIndex指定的profile的pe数据，该函数只对M2M版本有效
*
*@pram       :u8 profileIndex --- profile的Index, 该profile的PROFILE_VALIDTAG_CFG对应的状态肯定是PROFILE_LOADINGFLAG
*
*@return     :void
*
*@history    :created by xuxiaojun@20190517
*/
void IF_M2M_DeleteProfilePE(u8 profileIndex)
{
#if M2M_EUICC
	if(PROFILE_LOADINGFLAG == VMReadByte(PROFILE_VALIDTAG_CFG + profileIndex))
	{
		//if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//pe解析过程中如果在下载CAP包 ，此时PE下载发生了错误 先将Load包的一些变量清掉 结束下载cap包流程
		//{
			//GPLoadInit();
		//}
		
		DeleteProfileDataByProfileIndex(profileIndex, MODE_DELETE_PROFILE_PE); 
		
		//	打垃圾回收标记 ？GarbageCollectorAndHeapPieceCleanUp();

		G_DownLoadingProfileIndex = ILLEGAL_PROFILEINDEX;
	}
#endif	
}
/*
*@function   :IF_RSPReturnMainFormSTK
*
*@description:使用:LPA切换Profile时候有主动性命令,发送eUICC Profile State Change模式的Refresh命令,14还没回复LPA关闭了通道,
 强制延迟打断STK的操作等,判断14返回结果后打断,即调用SendProactiveCommand后  判断14结果做完各种初始化后调用IF_RSPReturnMainFormSTK
*
*@pram       :
*
*@return     :void
*
*@history    :created by LCZ 2019-7-9
*@history    modify by xwh@20220530 修改德国测试enable profile后发送Refresh命令，14还没回复LPA就关闭通道，BF2D返回6985的bug
*/
/*void IF_RSPReturnMainFormSTK(void)
{
#if RSP_EUICC
	WORD idata status; //modify chentao 20220530 定义成BYTE 修改德国测试enable profile后发送Refresh命令，14指令有的读卡器会无响应
	if(G_LpaSendRefresh == LPAChangeModeWithCloseChannel)
	{
		G_LpaSendRefresh = LPANoRefreshMode;
		
		if(G_NextAPDUFlag == FALSE)
		{
			//GlobalReceiveCommand();

			//GlobalSndResponseAndRcvCommand();//发送状态字及接收指令			
			
			status = (SW1<<8)|SW2; //modify by xwh@20220530 修改德国测试enable profile后发送Refresh命令，14还没回复LPA就关闭通道，BF2D返回6985的bug

			apdu_SendStatusAndReceiveCommand(status);
							 
			G_NextAPDUFlag = GPGetCurrenteChannel()+1;	  //收到下一条合法APDU 命令，标记标上
			
			ThrowErrorBackToMain(Priv_EXCEPTION);//抛出私有异常，借助该异常，回退到应用主流程中,遗留问题：Priv_EXCEPTION异常只能回退到触发流程起始的位置，还是无法回到主流程，需要在触发时，根据tag再次抛异常，结束UICC流程
		}
	}
	
	G_LpaSendRefresh = LPANoRefreshMode;
#endif
}*/
/*
*@function   :IF_GetCATTPPara
*
*@description:获取Profile里面的CATTP PDU/SDU Size
*
*@pram       :u8 size[2]  返回的PDU SIZE/SDU SIZE
*@pram       :type          00获取SDU SIZE/01 获取PDU Size
*
*@return     :void
*
*@history    :created by LCZ 2019-7-15
*/
void IF_GetCATTPPara(u8 *size,u8 type)
{
	u8 I_A buf[2];

	//if(G_CurEnabledProfileIndex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(G_CurEnabledProfileIndex) == FALSE)
	{
		return;
	}
	if(type == 0x00)
	{
		VMReadBuf(buf,PROFILE_CATTPPara + G_CurEnabledProfileIndex*sizeof(memref),sizeof(buf));  //SDU
	}
	else
	{
		VMReadBuf(buf,PROFILE_CATTPPara + G_CurEnabledProfileIndex*sizeof(memref)+2,sizeof(buf));  //PDU
	}

	if((buf[0] != 0x00) || (buf[1] != 0x00))  //如果非法值 则不修改size,size传进来时候已有默认值
	{
		ecpMem_Copy(size,buf,sizeof(buf));
	}

}

/*
*@function   :IF_ChangeProfileInitPrivateFile
*
*@description:Profile发生改变时候初始化私有文件 防止上一个Profile的私有文件值影响下一个Profile的(比如上一个Profile存在还没收齐的级联短信)
*
*@pram       :void
*
*@return     :void
*
*@history    :created by LCZ 20206028
*/
u16 CONST InitPrivateFile[3] ={EF_EnvelopeCBMessage,EF_ToolkitRFMSmsHead,EF_ToolkitRFMSms};
void IF_ChangeProfileInitPrivateFile(void)
{
	u8 I_A recNum,i,j,recLgth,temp[2];
	memref X_A PrivateFileAddr;

	for(j = 0x00; j < 3;j++) //3对应 InitPrivateFile文件个数
	{
		PrivateFileAddr = SelectPrivateLFFile(DF_Toolkit,InitPrivateFile[j]); //CB文件

		if(PrivateFileAddr != FALSE)
		{
			recNum = GetLFRecNumLgth(PrivateFileAddr,FileNum);

			recLgth = GetLFRecNumLgth(PrivateFileAddr,FileLgth);

			for(i=1 ;i <= recNum; i++)
			{
				ReadLFBuf(PrivateFileAddr, i,0x00,&temp[0],2);

				if((temp[0] == 0x00)&&(temp[1] == 0x00))
				{
					continue;
				}

				if(j == 0x00) //CB 文件清空两字节就好
				{
					SetLFConst(PrivateFileAddr, i,0x00,0x00,2);
				}
				else
				{
					SetLFConst(PrivateFileAddr, i,0x00,0x00,recLgth); //短信文件 清空整条记录
				}

			}
		}

	}
#if GP__AMENDMENT__B__FOR_MASK
	PrivateFileAddr = SelectPrivateTFFile(DF_TLS, EF_TLSsessionData); //TLS 文件

	if(PrivateFileAddr != FALSE)
	{
		ReadTFBuf(PrivateFileAddr, TLS_OpenCloseNormal_Offset, &temp[0], 0x01); //05 TLS_OpenCloseNormal_Offset

		if(temp[0] == 0x01)
		{
			SetTFConst(PrivateFileAddr, TLS_OpenCloseNormal_Offset, 0x00,0x01);
		}
	}
#endif

}
#endif // #if (EUICC_ONLY||SE_AND_EUICC)
/**
*modify by xuxiaojun@20220314 从platform_api.c中移动到这里来
*modify by xuxiaojun@20220528 对于支持SCP11的SD，如果SCP11 key完整的话，需要将SD切换到personal状态。 测试用例：AmendH测试脚本-SCP11c   Annex_B_scenario_02_DM_without_autoRestore_Taes16B_Raes16B_bb_02_11
*modify by xxj@20230512: 修正缺陷487时，该函数需要做适当的修改，返回值由TRUE修改为当前查找到的scp_type的第一组密钥的key version
*/
u8 GPSSDExistCompleteSecureKey(u8 sdId,u8 scp_type)
{
	memref X_A sdEntryAddr,sdDataObj,keySetObj;
	jref I_A sdRef,sdDataRef,keyGroupRef,keySetRef;
	PN8 keySetField;
	u8 I_A allTag;

	//不能再本函数内部这样判断，因为GPSSDExistCompleteSecureKey在多个地方调用，下行ram的sd初始化ProcessRAMinit时不能存在以下这个判断
//	if (scp_type == SCP)	 //当前平台默认所有ssd都支持的协议(scp02 or scp03)，不用判断其key是否完整，  gp2.2默认所有ssd支持scp02，gp2.3默认所有ssd支持scp03
//	{
//		return TRUE;
//	}

	sdEntryAddr = getEntryByAppId(sdId);

	if(sdEntryAddr != MEM_NULL)
	{
		sdRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(sdEntryAddr), theApplet_INDEX)));

		sdDataRef = sdData_getSDdataRef(sdRef);
		sdDataObj = locateObjectInMemory(sdDataRef);

		keyGroupRef = sdData_getKeyGroupRef(MEMtPN8(sdDataObj),scp_type, FALSE);

		if(keyGroupRef != REF_NULL)
		{
			keySetField = MEMtPN8(keyGroup_getKeySetField(MEMtPF8(locateObjectInMemory(keyGroupRef))));
			keySetRef = memReadU16(keySetField);

			while(keySetRef != REF_NULL)
			{
				keySetObj = locateObjectInMemory(keySetRef);

				//scp81key只存在两支key，属于format3的格式，format3的keyAllSet(allTag)字段无意义,此时只需要判断scp81存在两支key即可-by-jiangqingbin-20210618
				if (scp_type == SCP_81)
				{
					allTag = keySet_getKeyNum(keySetObj);

					if (allTag == 2)
					{
						return keySet_getKeyVersion(keySetObj); //return TRUE;
					}
				}
				else if(scp_type == SCP_11) // todo: 如何判断SCP11 KEY完整？
				{
					allTag = keySet_getKeyNum(keySetObj);//todo 这里判断完整性是不是得考虑当前sd支持SCP11a、SCP11b、SCP11c的情况，以及key的存在情况

					return keySet_getKeyVersion(keySetObj); //return TRUE;
				}
				else
				{
					allTag = keySet_getkeyAllTag(MEMtPN8(keySetObj));

					if((allTag&0x07) == 0x07)
					{
						return keySet_getKeyVersion(keySetObj); //return TRUE;
					}
				}

				keySetField = get_object_field_ref(MEMtPF8(keySetObj), nextKeySet_INDEX);//getNextKeySetHand(keySetObj);
				keySetRef = memReadU16(keySetField);
			}
		}

	}

	return FALSE;
}
