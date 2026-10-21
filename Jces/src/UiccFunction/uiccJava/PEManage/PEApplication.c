
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEApplication.c
*
* Description: 该文件主要是对PE-Application的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
#include  "fileAction.h"
#include "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "cardManager.h"
#include "ToolkitRFM.h"
#include "ProfileGPCommand.h"
#include "VMAccessEeprom.h"
#include "opFunction.h"

#if (EUICC_ONLY||SE_AND_EUICC)

/*********************PE-Application START*****************************/

//// ApplicationLoadPackage OPTIONAL,
//BYTE code theApplicationPELoadBlock[]={0xA1};

//// [APPLICATION 15] ApplicationIdentifier,
//BYTE code loadPackageAIDTag []={0xA1,0x4F};
//// [APPLICATION 15] ApplicationIdentifier OPTIONAL,
//BYTE code APPsecurityDomainAIDTag []={0xA1,0x4F};
//// [PRIVATE 6] OCTET STRING OPTIONAL,
//BYTE code nonVolatileCodeLimitC6Tag []={0xA1,0xC6};
//// [PRIVATE 7] OCTET STRING OPTIONAL,
//BYTE code volatileDataLimitC7Tag []={0xA1,0xC7};
//// [PRIVATE 8] OCTET STRING OPTIONAL,
//BYTE code nonVolatileDataLimitC8Tag []={0xA1,0xC8};
//// [PRIVATE 1] OCTET STRING OPTIONAL,
//BYTE code hashValueTag []={0xA1,0xC1};
//// [PRIVATE 4] OCTET STRING
//BYTE code loadBlockObjectTag []={0xA1,0xC4};
///***************************************************************************************************/
//// SEQUENCE OF ApplicationInstance OPTIONAL
////LIST... ?TAG ????
//BYTE code theApplicationPEinstanceList []={0xA2};
////[7] UInt8 DEFAULT 8ance ::[]=SEQUENCE
//BYTE code APPapplicationInstTag []={0xA2,0x30};
////[APPLICATION 15] ApplicationIdentifier,
//BYTE code APPapplicationLoadPackageAIDTag []={0xA2,0x30,0x4F};
////[APPLICATION 15] ApplicationIdentifier,
//BYTE code APPclassAIDTag []={0xA2,0x30,0x4F};
//// [APPLICATION 15] ApplicationIdentifier,
//BYTE code APPinstanceAIDTag []={0xA2,0x30,0x4F};
////[APPLICATION 15] ApplicationIdentifier OPTIONAL,
//BYTE code APPextraditeSecurityDomainAIDTag []={0xA2,0x30,0x4F};
////[2] OCTET STRING,
//BYTE code APPapplicationPrivilegesTag []={0xA2,0x30,0x82};
////[3] OCTET STRING (SIZE(1)) DEFAULT '07'H,
//BYTE code APPlifeCycleStateTag []={0xA2,0x30,0x83};
///* Coding according to GP Life Cycle State.  */
////[PRIVATE 9] OCTET STRING,
//BYTE code APPapplicationSpecificParametersC9Tag []={0xA2,0x30,0xC9};
///***************************************************************************************************/
//// [PRIVATE 15] ApplicationSystemParameters OPTIONAL,
//BYTE code APPsystemSpecificParametersTag []={0xA2,0x30,0xEF};
////[PRIVATE 7] OCTET STRING OPTIONAL,
//BYTE code APPvolatileMemoryQuotaC7Tag []={0xA2,0x30,0xEF,0xC7};
////[PRIVATE 8] OCTET STRING OPTIONAL,
////BYTE code nonVolatileMemoryQuotaC8Tag []={0xA2,0x30,0xEF,0xC8};
////[PRIVATE 11] OCTET STRING OPTIONAL,
////BYTE code globalServiceParametersTag []={0xA2,0x30,0xEF,0xCB};
////[PRIVATE 15] OCTET STRING OPTIONAL,
////BYTE code implicitSelectionParameterTag []={0xA2,0x30,0xEF,0xCF};
////[PRIVATE 23] OCTET STRING OPTIONAL,
////BYTE code APPvolatileReservedMemoryTag []={0xA2,0x30,0xEF,0xD7};
////[PRIVATE 24] OCTET STRING OPTIONAL,
////BYTE code APPnonVolatileReservedMemoryTag []={0xA2,0x30,0xEF,0xD8};
////[PRIVATE 10] OCTET STRING OPTIONAL,
////BYTE code ts102226SIMFileAccessToolkitParameterTag []={0xA2,0x30,0xEF,0xCA};//TODO
////[0] TS102226AdditionalContactlessParameters OPTIONAL,
////BYTE code ts102226AdditionalContactlessParametersTag []={0xA2,0x30,0xEF,0xB0};//TODO

///* Parameters for contactless applications encoded according to TS 102 226 */
////OCTET STRING 
////BYTE code protocolParameterDataTag []={0xA2,0x30,0xEF,0xB0,0x04};//TODO

///* Coded according to Contactless Protocol Parameters Structure as defined in GP Amd. C */
////[PRIVATE 25] OCTET STRING OPTIONAL, 
////BYTE code contactlessProtocolParametersTag []={0xA2,0x30,0xEF,0xD9};
///* Coded according to User Interaction Parameters Structure as defined in GP Amd. C */
//// [PRIVATE 26] OCTET STRING OPTIONAL  
////BYTE code APPuserInteractionContactlessParametersTag []={0xA2,0x30,0xEF,0xDA};


///***************************************************************************************************/
//	//[PRIVATE 10] UICCApplicationParameters OPTIONAL,
//BYTE code APPapplicationParametersTag []={0xA2,0x30,0xEA};
////[0] OCTET STRING OPTIONAL,
////BYTE code APPuiccToolkitApplicationSpecificParametersFieldTag []={0xA2,0x30,0xEA,0x80};
////[1] OCTET STRING OPTIONAL,
////BYTE code APPuiccAccessApplicationSpecificParametersFieldTag []={0xA2,0x30,0xEA,0x81};
//// [2] OCTET STRING OPTIONAL
////BYTE code APPuiccAdministrativeAccessApplicationSpecificParametersFieldTag []={0xA2,0x30,0xEA,0x82};

////SEQUENCE OF OCTET STRING OPTIONAL  LIST... ?TAG ????
//BYTE code processAPPDataListTag []={0xA2,0x30,0x30};

//BYTE code processAPPDataTag []={0xA2,0x30,0x30,0x04};


/*
*@function   : ApplicationInstanceAnalyze
*
*@description: 解析 8.7.3 ApplicationInstance 组件
*             ApplicationInstance ::= SEQUENCE 
*             { 
*               applicationLoadPackageAID [APPLICATION 15] ApplicationIdentifier,
*               classAID [APPLICATION 15] ApplicationIdentifier, 
*               instanceAID [APPLICATION 15] ApplicationIdentifier, 
*               extraditeSecurityDomainAID [APPLICATION 15] ApplicationIdentifier OPTIONAL, 
*               applicationPrivileges [2] OCTET STRING, 
*               lifeCycleState [3] OCTET STRING (SIZE(1)) DEFAULT '07'H, // Coding according to GP Life Cycle State. 
*               applicationSpecificParametersC9 [PRIVATE 9] OCTET STRING, 
*               systemSpecificParameters [PRIVATE 15] ApplicationSystemParameters OPTIONAL, 
*               applicationParameters [PRIVATE 10] UICCApplicationParameters OPTIONAL, 
*               processData SEQUENCE OF OCTET STRING OPTIONAL
*             }
*
*@参考规范    :《eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf》 8.7.3
*              :Note: 按照ASN.1规范，SEQUENCE是一个有序集合，不考虑数据乱序过来的情况
*
*@初始化条件  :G_PETlvInfo.findValueOff 指向ApplicationInstance中的applicationLoadPackageAID 所在的偏移,G_PETlvInfo.findValueLen为该组件的总长度         
*
*@param       :BYTE mode :0x00: PESD_MODE/ 0x01 :PEAPP_MODE 普通应用
*
*@rerurn      :BYTE appId
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20170711
*/
BYTE code MNOsdModuleAID[] = {0xA0,0x00,0x00,0x01,0x51,0x00,0x00,0x00};
BYTE code MNOsdPackageAID[] = {0xA0,0x00,0x00,0x01,0x51,0x01,0x00};

//BYTE code sdModuleAID[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//BYTE code sdPackageAID[] = {0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x61,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  //aid根据 SIMalliance-eUICC-Profile-Package-Interoperable-Format-Test-Specification-Version-2.0.pdf进行配置




/*
*@function   : checkRFMAID
*
*@description: 检查应用/SSD的AID和RFM的实例/ADF的AID是否相同
*
*@param       :BYTE * instanceAID : 应用/SSD的AID
*@param       :BYTE aidLen :应用/SSD的AID的长度
*
*@return      :TRUE/FALSE
*
8@author      :zhxx
*
*@history     :V1.0 :create by zhxx@20190401
*/


BYTE checkRFMAID(BYTE* instanceAID,BYTE aidLen)
{
	UWORD xdata fileAddr;
	BYTE idata i,num,recLgth;
	
	fileAddr = SelectFile(G_CreatingMFOffset,DF_Toolkit_P);
	
	if(fileAddr == FALSE)
	{
		return TRUE;
	}
	
	fileAddr = SelectFile(fileAddr,EF_RFM_TAR);
	
	if(fileAddr == FALSE)
	{
		return TRUE;
	}

	num = GetLFRecNumLgth(fileAddr,FileNum);

	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);
	
	for(i=1;i<= num;i++) //检查应用/SSD的AID和RFM的实例/ADF的AID是否相同
	{
		ReadLFBuf(fileAddr,i,0x00,RecData,recLgth);
		if((aidLen == RecData[PEAID_Offset]) && (ecpMem_Compare(&RecData[PEAID_Offset+1],instanceAID,aidLen) == 0))
		{
			return FALSE;
		}
		
		if((aidLen == RecData[PEADFAid_Offset]) && (ecpMem_Compare(&RecData[PEADFAid_Offset+1],instanceAID,aidLen) == 0))
		{
			return FALSE;
		}
	}
	
	return TRUE;
}



/*
*@funtion    :processStatusWord
*
*@description: 根据GP指令返回的状态字拼装profile的响应
*
*@param      :WORD sw      --      gp指令的状态字
*@param      :BYTE option  --      是否是load指令
*
*@return     :BYTE   --  TRUE/FALSE/2
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190328
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE processStatusWord(WORD sw,BYTE option)
{
	BYTE idata sw1;
	
	if((sw == SW_REFERENCED_DATA_NOT_FOUND) || (sw == SW_SECURITY_STATUS_NOT_SATISFIED) || (sw == SW_WRONG_DATA)) //modify by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data\06-CatTpParameters\\data\06-CatTpParameters_05
	{
		appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);//1、Error when load a PE-Applet4 and bad library is provided.2、TC_8_2_2_PE_App_ErrorHash_Error
					
		return FALSE;
	}
	
	if(sw == SW_FILE_FULL)
	{
		appendErrorResponse(NOT_ENOUGH_MEMORY,G_PEControlContext.PE_identification);
		
		//if(option == 2)//installforinstall指令才做此操作，为了兼容测试,无论下载还是安装，只要SW_FILE_FULL错误，都需要抛ErrorReason_installFailedDueToInsufficientMemoryForProfile错误
		{
	#if RSP_EUICC
			G_ProfileErrorReason = ErrorReason_installFailedDueToInsufficientMemoryForProfile;	
    #endif		
		}			
		return FALSE;
		
	}
	
	sw1 = (BYTE)(sw >> 8);
	if((sw1 != 0x90) && (sw1 != 0x61))
	{
		if(option == TRUE)			
		{
			appendErrorResponse(LIB_NOT_SUPPORT,G_PEControlContext.PE_identification);//load指令
		}
		else
		{
			appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
		}
		
		return FALSE;
	}
	return TRUE;
}
/*
*@funtion    :ProcessDataProcess
*
*@description:当30/04数据很长，从04后面断开的处理
*
*@param      :WORD processDataValueLen,WORD processDataValueOff
*
*@return     :BYTE   --  TRUE/FALSE/2/3   TRUE:所有04tag执行完且执行成功  FALSE:执行失败   2:处理了部分04tag，不完整的部分备份先备份起来，接收下一批的数据继续处理  3:04tag已经处理完，后面接着处理上一层级的30tag(有多个应用实例的安装)
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20220105
*/
BYTE ProcessDataProcess(WORD processDataValueOff,WORD processDataValueLen)
{
	BYTE idata result;
    WORD idata sw;

	TlvInfo xdata tempASN1Info;

    if(processDataValueLen != FALSE)   //正式处理processData SEQUENCE
	{

		initTlvInfo((TlvInfo*)&tempASN1Info,G_PETlvInfo.bufferAddr+processDataValueOff, processDataValueLen);

		initFindTlv(&tempASN1Info);

		while(1)
		{
			result = findTLV(&tempASN1Info,0xFF,0xFF,1);

			if(result == false)
			{
				if(tempASN1Info.findNextLen > 0)//04TLV不是完整的
				{
					backData(EUICCBackPEBuf, tempASN1Info.bufferAddr + tempASN1Info.findNextOff, tempASN1Info.bufferLen-tempASN1Info.findNextOff);
					return TRUE;
				}
				else
				{
					break;
				}
			}

			if(tempASN1Info.isCompleteBuffer[0] == FALSE)//处理的04tag不完整
			{
				backData(EUICCBackPEBuf,tempASN1Info.bufferAddr + tempASN1Info.findTagOff,  tempASN1Info.bufferLen-tempASN1Info.findTagOff);
				return TRUE;//将04前半部分数据备份起来，收齐下一批数据，继续执行
			}

			if(tempASN1Info.tagBuffer[0] == 0x04)
			{
				VMReadBuf(G_APDU,tempASN1Info.bufferAddr+tempASN1Info.findValueOff,5);	//读取APDU头  TY_OfflineProfileFormatTest/App_Install_N10

				VMReadBuf(RecData,tempASN1Info.bufferAddr+tempASN1Info.findValueOff+5,tempASN1Info.findValueLen-5);//读取APDU头之外的长度

				bRamBuffer[INSTALL_FOR_PERSONERLIZE] = G_profileGPState.peInstallAppId;
				bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = U8_OR_U8(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL], U8_SHL_U8(1, selected_channel));

				sw = IF_ExcuteProfileGPCommand(MNOSD_APPID);

				if(processStatusWord(sw,FALSE) == FALSE)
				{
					return FALSE;//执行失败
				}
			}
			else if(tempASN1Info.tagBuffer[0] == 0x30)//返回到上一层级的0x30
			{
				initTlvInfo((TlvInfo*)&G_PETlvInfo,tempASN1Info.bufferAddr+tempASN1Info.findTagOff, tempASN1Info.findTLVLen);

				initFindTlv(&G_PETlvInfo);

				G_PETlvInfo.levelIndex = 1;

				return 2;//接着执行上一层级的0x30
			}
			else
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//Error when load a PE-Applet4 and bad library is provided.
				return FALSE;//执行失败
			}
		}
	}

    return TRUE;  //04已经完全处理完毕，接着执行下一个PE
}
/*
*@funtion    :ApplicationInstanceAnalyze
*
*@description:解析PE-SD或者PE-Application的ApplicationInstance
*
*@param      :BYTE mode  --   PE-SD模式/PE-Application模式
*
*@return     :BYTE   --  TRUE/FALSE
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190328
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*            :V1.2 :modify by chentao@20211022 contaceless para 处理重复了，底层函数installParaProcessFuction有处理，所以这里的删除
*            :V1.3 :modify by zhxx@20220419 同步EPX产业化/INFENION_SLM97/Branch/huawei代码，PEApp/processData很长，超过PE缓存能力的处理，另外profile中的数据EF/D9,DA,A0需要转换成GP标准的tag
              V1.4 modify by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
*/
BYTE ApplicationInstanceAnalyze(BYTE mode)
{
	BYTE idata aidOccurce;
	BYTE idata result,lifeCycle,result1;
	WORD idata processDataValueOff,processDataValueLen,installParaLen, sw;
	BYTE xdata privi[0x03];
	//UWORD xdata totalLen;
	BYTE xdata temp[2];
	BYTE xdata AIDbuf[0x10];

	aidOccurce = FALSE;  //标记ApplicationIdentifier出现的次数
	
	processDataValueLen = FALSE;
	
	processDataValueOff = FALSE;  //初始化变量，去除警告
	
	setVerticalFind(&G_PETlvInfo); // TLV嵌套，设置垂直查找信息
	
	G_TempLen = 0; 
	
	installParaLen = FALSE;   //初始化安装参数长度
	
	ecpMem_Set(temp,0,sizeof(temp));
	
	//totalLen = 0;		//not used
	
	P1 = 0x0C;
	
	IF_SetAnalyzeLifecycle(APP_STATE_SELECTABLE);//DEFAULT '07'H, /* Coding according to GP Life Cycle State. */
			
	while (1)
	{	
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
		    if(G_PETlvInfo.findNextLen > 0)
			{
				backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	

				return TRUE;
			}
			else
			{
				break;
			}
		}
		switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
		{
			case 0x4F:
			{
				if(aidOccurce == 0x04)  //出现了5次 4F tag ，error
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}


				 if(MNO_SELF_EXTRADITE == 0)//根据profile规范：For the MNO-SD instance, no value shall be provided for the "extraditeSecurityDomainAID" parameter: the MNO-SD shall be associated with itself and shall not be subject to extradition, as indicated in the GlobalPlatform Card Specification [GP CS].
				{
						if((G_profileControlContext.IsCreateMNOSD ==0) && (aidOccurce == 0x03) && (PESD_MODE == mode)) // MNoSD不能关联到自身,WQ_Offline_Profile_Test/TC_7_1_3_PE_SD_MNOSDExtradite_Error有测到
						{
							appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);//标准的处理流程
					return FALSE;
				}

				 }


				if(AIDLenInvalid(G_PETlvInfo.findValueLen))
				{
					appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
					return FALSE;
				}
				if(aidOccurce == 0x03)
				{
					//8.2.7.7 Error when loading Application PE and the lifecycle of SD is not PERSONALISED
					extraditeSDAID[0] = G_PETlvInfo.findValueLen;
					VMReadBuf(&extraditeSDAID[1],(UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff), G_PETlvInfo.findValueLen);
					
					result = IF_GetAnalyzeAppIdByAIDList(&extraditeSDAID[1],G_PETlvInfo.findValueLen);
					
					if(result == ILLEGAL_APPID)
					{
						if((mode != PESD_MODE) || (ecpMem_Compare(extraditeSDAID, instanceAID, G_PETlvInfo.findValueLen+1) != 0x00))
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);

							return FALSE;
						}
						else
						{
							if(MNO_SELF_EXTRADITE == 0)//不支持MNOSD自引渡，标准规范处理流程
							{
								//needExtradite[0] = TRUE;
								SetBit(G_PEControlContext.IsExistFlag3,NeedExtradite); //add by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
							}
							else
							{
								if(G_profileControlContext.IsCreateMNOSD > 0)//MNO-SD自引渡时，不需要拼装makeinstallforextradite指令,只有SSD自引渡时才需要拼装该指令，
								{
									SetBit(G_PEControlContext.IsExistFlag3,NeedExtradite);//add by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
								}
							}

						}
					}
					
					IF_SetAnalyzeExtraditeSDID(result);
				}
				else
				{
					VMReadBuf(&AIDbuf[0],(UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff), G_PETlvInfo.findValueLen);
					if(checkRFMAID(&AIDbuf[0],G_PETlvInfo.findValueLen) == FALSE)//检查应用/SSD的AID和RFM的实例/ADF的AID是否相同
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					
						return FALSE; 
					}
					if(aidOccurce == 0x02)
					{
						instanceAID[0] = G_PETlvInfo.findValueLen;
						ecpMem_Copy(&instanceAID[1], AIDbuf, G_PETlvInfo.findValueLen);
					}
				}
				
				if((mode == PESD_MODE) && (G_profileControlContext.IsCreateMNOSD == 0))  //采用卡端预置的ISD 的包AID 和可执行模块AID
				{
					if(aidOccurce == 0x00)  //拷贝SSD 的包AID 到tempBuf[0]起始的位置
					{
						tempBuf[G_TempLen++] = 0x4F;
						tempBuf[G_TempLen++] = sizeof(MNOsdPackageAID);//在安装MNO-SD 时，不采用下行报文中的可执行模块AID和包AID，采用卡商自定义的SSD 模块AID 和 包AI
						
						ecpMem_Copy(&tempBuf[G_TempLen],&MNOsdPackageAID,sizeof(MNOsdPackageAID));
						
					    G_TempLen += sizeof(MNOsdPackageAID);
					    bRamBuffer[PROFILE_INSTALL_MNOSD_FLAG] = TRUE;
					
					}
					else if(aidOccurce == 0x01)  //拷贝SSD 的可执行模块AID 到tempBuf[0]起始的位置
					{						
						tempBuf[G_TempLen++] = 0x4F;
						tempBuf[G_TempLen++] = sizeof(MNOsdModuleAID); //拷贝SSD 的可执行模块AID 到tempBuf[0]起始的位置
						
						ecpMem_Copy(&tempBuf[G_TempLen],&MNOsdModuleAID,sizeof(MNOsdModuleAID));
						
					    G_TempLen += sizeof(MNOsdModuleAID);
						
					}
					else
					{
						if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
						{
							return FALSE;
						}
					}
					
				}
				else
				{
					if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
					{
						return FALSE;
					}
				}
				
				aidOccurce ++;
				
				break;

			}
			case 0x82:     // applicationPrivileges [2] OCTET STRING, 
			{
				VMReadBuf(privi,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen); 
				
				if((G_PETlvInfo.findValueLen != 0x03) && (G_PETlvInfo.findValueLen != 0x01))
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				if(mode == PESD_MODE) 
				{
					if((privi[0] & 0x80)!= 0x80)   //当前处理的是PE -SD ，但是应用的权限却不是SD
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
				else
				{
					if((privi[0] & 0x80)!= 0x0)   //当前安装的是PE- APP，但是权限却是SD 的
					{
						appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
						return FALSE;
					}
				}
				
				if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
				{
					return FALSE;
				}
				temp[0]++;
				break;
			}
			case 0x83:   //lifeCycleState [3] OCTET STRING (SIZE(1)) DEFAULT '07'H, // Coding according to GP Life Cycle State
			{
				if(G_PETlvInfo.findValueLen != 0x01)
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
				
				lifeCycle = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
			
				if(lifeCycle == APPLET_INSTALLED)
				{
					P1 = 0x04;
				}
				IF_SetAnalyzeLifecycle(lifeCycle);
				temp[1]++;				
				break;
			}
			case 0xC9:  //applicationSpecificParametersC9 [PRIVATE 9] OCTET STRING,    拷贝TLV
				if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)//applicationPrivileges
				{
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}
				if(mode == PESD_MODE)
				{
					// modify by 陈涛20220802 德国profile C9参数没有82tag希望能自迁移成功  australia_telastra_fail_log_2022_07_07(明文profile).txt
					result1 = FindTag(tempBuf+G_TempLen-G_PETlvInfo.findValueLen,0x82,G_PETlvInfo.findValueLen);
					if(result1 == FALSE)//测试用例：德国外场问题profile下载测试_RSP/Only_load_Porifle_20220707
					{
						tempBuf[G_TempLen-G_PETlvInfo.findValueLen-1]+= 3;
						tempBuf[G_TempLen++] = 0x82;
						tempBuf[G_TempLen++] = 0x01;
						tempBuf[G_TempLen++] = 0xF8;
						installParaLen += 3;
					}
				}

				installParaLen += G_PETlvInfo.findTLVLen;

				break;
			case 0xEF:
			case 0xEA:  //applicationParameters
			{
				if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)//applicationPrivileges
				{
					//appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					return FALSE;
				}

//eUICC_Profile_Package_Interoperable_Format_Test_V_2_3/ TC_8_2_7_12_Installing_profile_with_contactless_eUICC_Mandatory_service_selected_and_userInteractionContactlessParameters_eUICC_reports_error,不支持非接的话需要抛错，支持非接的话GP层处理
#if !GP__AMENDMENT__C__FOR_MASK
				if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == 0xEF)
				{
					result1 = FindTag(tempBuf+G_TempLen-G_PETlvInfo.findValueLen,0xD9,G_PETlvInfo.findValueLen);//profile:EF/contactlessProtocolParameters [PRIVATE 25] = 0xD9
					if(result1 != FALSE)
					{
						  appendErrorResponse(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification);
						  return FALSE;
						//tempBuf[result1+G_TempLen-G_PETlvInfo.findValueLen-1] = 0xA0;//GPAmdC:Contactless Protocol Parameters = 0xA0
					}
					result1 = FindTag(tempBuf+G_TempLen-G_PETlvInfo.findValueLen,0xDA,G_PETlvInfo.findValueLen);//profile:EF/userInteractionContactlessParameters [PRIVATE 26] = 0xDA
					if(result1 != FALSE)
					{
						  appendErrorResponse(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification);
						  return FALSE;
						//tempBuf[result1+G_TempLen-G_PETlvInfo.findValueLen-1] = 0xA1;//GPAmdC:User Interaction Parameters = 0xA1
					}

					result1 = FindTag(tempBuf+G_TempLen-G_PETlvInfo.findValueLen,0xA0,G_PETlvInfo.findValueLen);//profile:EF/ts102226AdditionalContactlessParameters [0] = 0xA0
					if(result1 != FALSE)
					{
						  appendErrorResponse(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification);
						  return FALSE;
						//tempBuf[result1+G_TempLen-G_PETlvInfo.findValueLen-1] = 0xB0;//GPAmdC:(Assigned to ETSI) = 0xB0
					}
				}
#endif

			
				installParaLen += G_PETlvInfo.findTLVLen;
				
				break;
			}
			case 0x30: //为为何processData的 tag是 0x30 ，没有找到来源，30后面还有04tag
			{
				processDataValueOff = G_PETlvInfo.findValueOff;  	//登记processData SEQUENCE OF OCTET STRING OPTIONAL 所处的偏移和数据长度
				
				if(G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == FALSE)//0x30不是完整的
				{
					processDataValueLen = G_PETlvInfo.bufferLen-G_PETlvInfo.findValueOff;
				}
				else//0x30是完整的
				{
					processDataValueLen = G_PETlvInfo.findValueLen;
				}
				break;
			}
			default:
			{
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					
				return FALSE;
			}
		}
				
	}	
	
	if((temp[0] != 1) || (temp[1] > 1))////The applicationPrivileges object is mandatory and it shall be an OCTET STRING.
	{
		appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);//Error when load a PE-Applet4 and bad library is provided.
				
		return FALSE;
	}	
	
	sw = makeInstallForInstall(installParaLen);
	
	bRamBuffer[PROFILE_INSTALL_MNOSD_FLAG] = FALSE;

	if(processStatusWord(sw,2) == FALSE)
	{
		return FALSE;
	}

	result1 = ProcessDataProcess(processDataValueOff,processDataValueLen);//处理04tag


	setReturnFind(&G_PETlvInfo);

	return result1;  //TRUE/FALSE/2
}

/*
*@function  : ApplicationLoadPackageAnalyze
*
*@description: 分析PE-Application 中的loadBlock ApplicationLoadPackage 组件,该组件可选
*
*              ApplicationLoadPackage ::= SEQUENCE 
*               { 
*                      loadPackageAID [APPLICATION 15] ApplicationIdentifier, 
*                      securityDomainAID [APPLICATION 15] ApplicationIdentifier OPTIONAL, 
*                      nonVolatileCodeLimitC6 [PRIVATE 6] OCTET STRING OPTIONAL, 
*                      volatileDataLimitC7 [PRIVATE 7] OCTET STRING OPTIONAL, 
*                      nonVolatileDataLimitC8 [PRIVATE 8] OCTET STRING OPTIONAL,  
*                      hashValue [PRIVATE 1] OCTET STRING OPTIONAL, 
×                      loadBlockObject [PRIVATE 4] OCTET STRING 
*                }
*@参考规范    :《eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf》 8.7.2
*              :Note: 按照ASN.1规范，SEQUENCE是一个有序集合，不考虑数据乱序过来的情况
*
*@初始化条件  :G_PETlvInfo.findValueOff 指向ApplicationLoadPackage中的loadPackageAID 所在的偏移,G_PETlvInfo.findValueLen为该组件的总长度         
*
*@param       :void
*
*@rerurn      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20170711
*             :V1.1:modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE ApplicationLoadPackageAnalyze(void)
{
	BYTE idata result,occur4FTag;
	WORD idata sw;
	BYTE xdata extraditeSecurityDomainAID[16];
	
	occur4FTag = 0;
	
	if(G_PETlvInfo.levelIndex == 0)
	{
		G_TempLen = 0; 
		
		G_profileControlContext.LoadParaLen = FALSE;

		setVerticalFind(&G_PETlvInfo); // TLV嵌套，设置垂直查找信息
	}
		
	while (1)
	{	
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			if(G_PETlvInfo.findNextLen > 0)
			{			
			    backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PEControlContext.ApplicationLoadPackageOff,  G_PETlvInfo.bufferLen-G_PEControlContext.ApplicationLoadPackageOff);
				G_PETlvInfo.levelIndex = 0;
				return 2;
			}
			else
			{
				break;
			}		
		}
		
		if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] != 0xC4)
		{
			if(G_PETlvInfo.findNextLen == 0)
			{
				backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PEControlContext.ApplicationLoadPackageOff,  G_PETlvInfo.bufferLen-G_PEControlContext.ApplicationLoadPackageOff);
				G_PETlvInfo.levelIndex = 0;
				return 2;
			}
			
			G_profileControlContext.TotalLen += G_PETlvInfo.findTLVLen;
		}
		
		switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
		{
			case 0x4F:   //第一次出现的4F tag为loadPackageAID，第二次出现的4F  tag为 securityDomainAID ，第一次的必选，第二次可选
			{
                if(AIDLenInvalid(G_PETlvInfo.findValueLen))
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
		            return FALSE;
				}	
				
                if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
				{
					return FALSE;
				}
								
				if(occur4FTag == 1)
				{
					VMReadBuf(&extraditeSecurityDomainAID[0],(UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff), G_PETlvInfo.findValueLen);
					result = IF_GetAnalyzeAppIdByAIDList(&extraditeSecurityDomainAID[0],G_PETlvInfo.findValueLen);
					if(result == ILLEGAL_APPID)
					{						
						appendErrorResponse(BAD_VALUES,G_PEControlContext.PE_identification);
					
		                return FALSE; 
					}

					IF_SetAnalyzeExtraditeSDID(result);
				}
				else
				{
					occur4FTag++;
				}				
			
				break; 
			}
			case 0xC6:  //nonVolatileCodeLimitC6			
			case 0xC8:  //nonVolatileDataLimitC8
			{
                if((G_PETlvInfo.findValueLen != 2) && (G_PETlvInfo.findValueLen != 4))	
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
		            return FALSE;
				}					
				
                if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
				{
					return FALSE;
				}
			
				G_profileControlContext.LoadParaLen += G_PETlvInfo.findTLVLen;
			
				break;
			}
			case 0xC7:  //8.2.7.2 If O_MEMORY_LIMIT , the eUICC responds with PEStatus (4) not-enough-memory and the identification of PE-Application-2, or with PEStatus different from (0) and identification of PE-Application-2.
			{
				if((G_PETlvInfo.findValueLen != 2) && (G_PETlvInfo.findValueLen != 4))	
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
		            return FALSE;
				}
				
				VMReadBuf(&RecData[0],(UWORD)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff), G_PETlvInfo.findValueLen);
				
				if((G_PETlvInfo.findValueLen == 2) && (RecData[0] == 0xFF) && (RecData[1] == 0xFF))
				{
					appendErrorResponse(NOT_ENOUGH_MEMORY,G_PEControlContext.PE_identification);
					
		            return FALSE; //For volatileDataLimitC7 with value ‘FFFF’ the eUICC reports error status.
				}
				
				if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
				{
					return FALSE;
				}
			
				G_profileControlContext.LoadParaLen += G_PETlvInfo.findTLVLen;
			
				break;
			}
			case 0xC1:  //hashValue
			{	
				//TODO:hash的长度为多少				
				if(appendFindValue(&G_PETlvInfo,FromTag) == FALSE)
				{
					return FALSE;
				}
			
				break;
			}
			case 0xC4: //loadBlockObject
			{
				if(occur4FTag == 0)
				{         
					appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					
		            return FALSE;//一个4F tag都不存在 
				}
				
				sw = makeInstallForLoad(G_profileControlContext.LoadParaLen);
				
				if(processStatusWord(sw,FALSE) == FALSE)					
				{
					return FALSE;
				}
				
				G_profileControlContext.LoadParaLen = 0;
				
				sw = makeLoad();
				
				if(processStatusWord(sw,TRUE) == FALSE)					
				{
					return FALSE;
				}
				
				break;
			}
			default:
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);//Error when load a PE-Applet4 and bad library is provided.
					
		    return FALSE;
		}
	}
	
	setReturnFind(&G_PETlvInfo);  //回退到上一级
	
	return TRUE;
}



/*
*@funtion    :appendLVApdu
*
*@description: 从G_TempTlvInfo查找tag，追加到destBuffer中，拼装的格式是LV结构
*
*@param      : BYTE* destBuffer   --需要追加的目的buffer
*              BYTE  tag          --待查找的tag
*              WORD* curOff       --buffer的起始偏移
*              BYTE  occur        --待查找的次数
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
void appendLVApdu(BYTE* destBuffer,BYTE tag,WORD *curOff,BYTE occur)
{
	BYTE idata result;	
	
	result = FindTagOffset(tempBuf,tag,G_TempLen,occur);
	
	if(result)
	{
		ecpMem_Copy(&destBuffer[*curOff],(BYTE*)(tempBuf+result),tempBuf[result]+1);
		
		*curOff += (tempBuf[result]+1);
	}
	else
	{
		destBuffer[*curOff] = 0;	
		*curOff += 1;
	}
}


/*
*@funtion    :appendTLVApdu
*
*@description: 从G_TempTlvInfo查找tag，追加到destBuffer中，拼装的格式是TLV结构
*
*@param      : BYTE* destBuffer   --需要追加的目的buffer
*              BYTE  tag          --待查找的tag
*              WORD* curOff       --buffer的起始偏移
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
void appendTLVApdu(BYTE* destBuffer,BYTE tag,WORD *curOff)
{
	BYTE idata result;	
		
	result = FindTag(tempBuf,tag,G_TempLen);
	
	if(result)
	{
		if(tempBuf[result] > 0x80)//长度超过了0x80，目前仅支持这种情况
		{
			ecpMem_Copy(&destBuffer[*curOff],(BYTE*)(tempBuf+result-2),tempBuf[result]+3);
			
			*curOff += (tempBuf[result]+3);
		}
		else
		{
			ecpMem_Copy(&destBuffer[*curOff],(BYTE*)(tempBuf+result-1),tempBuf[result]+2);
			
			*curOff += (tempBuf[result]+2);
		}
		
	}
}


/*
*@funtion    :makeInstallForLoad
*
*@description: 根据GP2.2.1规范，拼装标准的installForLoad指令到APDUBuffer中
*
*@param      :WORD installParaLen   --安装参数的长度
*@param      :BYTE occurNums   :4F tag出现的次数
*
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
WORD makeInstallForLoad(WORD loadParaLen)
{
	WORD idata curOff,status;
	
	curOff = 0;
	
	appendLVApdu(RecData,0x4F,&curOff,1);//Load File AID
	
	appendLVApdu(RecData,0x4F,&curOff,2);//Security Domain AID
	
	appendLVApdu(RecData,0xC1,&curOff,1);//Load File Data Block Hash
	
	RecData[curOff++] = loadParaLen+2;
	
	RecData[curOff++] = 0xEF;
	
	RecData[curOff++] = loadParaLen;
	
	appendTLVApdu(RecData,0xC6,&curOff);//installPara
	
	appendTLVApdu(RecData,0xC7,&curOff);//installPara
	
	appendTLVApdu(RecData,0xC8,&curOff);//installPara
	
	RecData[curOff++] = 0;//TOKEN
	
	CLA = (0x80 | selected_channel);
	
	INS = 0xE6;
	
	P1 = 0x02;
	
	P2 = 0x00; 
	
	P3 = curOff;
	
	status = IF_ExcuteProfileGPCommand(MNOSD_APPID);
	
	return status;
	
}

/**
*modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
WORD makeRemainLoad(void)
{
	WORD idata lastBlockLen,lastBlockCnt,status ;
	WORD idata i;
	WORD idata backC4Remain = G_PEControlContext.C4RemainLen;
	
	if(G_PEControlContext.C4RemainLen > G_PETlvInfo.bufferLen) //下载应用的剩余数据比该批来的PE总长度还要大
	{
		lastBlockLen = G_PETlvInfo.bufferLen % 255; //LOAD 最后一条APDU 数据的长度

		lastBlockCnt = G_PETlvInfo.bufferLen / 255;  //LOAD APDU的数目

		G_PEControlContext.C4RemainLen -= G_PETlvInfo.bufferLen;
	}
	else
	{
		lastBlockLen = G_PEControlContext.C4RemainLen % 255; //LOAD 最后一条APDU 数据的长度

		lastBlockCnt = G_PEControlContext.C4RemainLen / 255;  //LOAD APDU的数目
		
		G_PEControlContext.C4RemainLen = 0;
	}
	
	if(lastBlockLen == 0)//如果是该批数据是255的整数倍，则将最后一个255数据放在循环外面进行拼装，这样可以减少一次P3是0长度的load指令的拼装
	{
		lastBlockLen = 0xFF;
		
		lastBlockCnt--;
	}
	
	
	for(i=0;i<lastBlockCnt;i++)
	{
		CLA = (0x80 | selected_channel);
	
	    INS = 0xE8;
	    P1 = 0;
		P2 = (G_PEControlContext.BlockNum&0xFF);
			
		VMReadBuf(&RecData[0], G_PETlvInfo.bufferAddr+i*255, 255);
		
		P3 = 255;
		status = IF_ExcuteProfileGPCommand(MNOSD_APPID);

		G_PEControlContext.BlockNum++;
		
		if(status != 0x9000)  //LOAD命令执行失败
		{
			return status;
		}
	}
	
	CLA = (0x80 | selected_channel);  // add by  chentao@20240301 load 执行完P1变了，需要重新赋值
	INS = 0xE8;
	if(G_PEControlContext.C4RemainLen == 0)
	{
		P1 = 0x80;
	}	
	else
	{
		P1 = 0x00;
	}
	
	P2 = (G_PEControlContext.BlockNum&0xFF);
	
	VMReadBuf(&RecData[0], G_PETlvInfo.bufferAddr+i*255, lastBlockLen);
	P3 = lastBlockLen;
	
	status = IF_ExcuteProfileGPCommand(MNOSD_APPID);
			
	if(G_PEControlContext.C4RemainLen == 0)//最后一块儿
	{
		initTlvInfo(&G_PETlvInfo,G_PETlvInfo.bufferAddr+backC4Remain,G_PETlvInfo.bufferLen-backC4Remain);	
		G_PEControlContext.BlockNum = 0;
	}
	else
	{
		G_PETlvInfo.bufferLen = 0;
		G_PEControlContext.BlockNum++;
	}
	
	return status;
}

/*
*@funtion    :makeLoad
*
*@description: 根据GP2.2.1规范，拼装标准的Load指令到APDUBuffer中
*
*@param      : void
*
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
WORD makeLoad(void)
{
	WORD idata lastBlockLen,lastBlockCnt,status ;
	WORD idata remainLen;
	remainLen = G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff;
	
	if(remainLen >= G_PETlvInfo.findTLVLen)//数据完整
	{
		lastBlockLen = G_PETlvInfo.findTLVLen % 255; //LOAD 最后一条APDU 数据的长度
	
	    lastBlockCnt = G_PETlvInfo.findTLVLen / 255;  //LOAD APDU的数目

	}
	else//数据不完整
	{
		lastBlockLen = remainLen%255;
		lastBlockCnt = remainLen/255;	
		G_PEControlContext.C4RemainLen = G_PETlvInfo.findTLVLen - remainLen;
	}
	
	if(lastBlockLen == 0)//如果是该批数据是255的整数倍，则将最后一个255数据放在循环外面进行拼装，这样可以减少一次P3是0长度的load指令的拼装
	{
		lastBlockLen = 0xFF;
		
		lastBlockCnt--;
	}
		
	
	
	for(;G_PEControlContext.BlockNum<lastBlockCnt;G_PEControlContext.BlockNum++)
	{
		CLA = (0x80 | selected_channel);
	
	    INS = 0xE8;
	    P1 = 0;
		P2 = (G_PEControlContext.BlockNum&0xFF);
		
		VMReadBuf(&RecData[0], G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff+G_PEControlContext.BlockNum*255, 255);
		
		P3 = 255;
		
		status = IF_ExcuteProfileGPCommand(MNOSD_APPID);
		
		if(status != 0x9000)  //LOAD命令执行失败
		{
			return status;
		}
	}
	
	INS = 0xE8;
	if(G_PEControlContext.C4RemainLen == 0)
	{
		P1 = 0x80;
	}
	else
	{
	    P1 = 0x00;
	}
	
	CLA = (0x80 | selected_channel);

	P2 = (G_PEControlContext.BlockNum&0xFF);
	
	VMReadBuf(&RecData[0], G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff+G_PEControlContext.BlockNum*255, lastBlockLen);
	P3 = lastBlockLen;
	
	status = IF_ExcuteProfileGPCommand(MNOSD_APPID);
	
	
	if(G_PEControlContext.C4RemainLen == 0)//最后一块儿,将G_PEControlContext.BlockNum清0
	{
		G_PEControlContext.BlockNum = 0;
	}
	else
	{
		G_PEControlContext.BlockNum ++;
	}

	return status;
}



/*
*@funtion    :makeInstallForInstall
*
*@description: 根据GP2.2.1规范，拼装标准的InstallForInstall指令到APDUBuffer中
*
*@param      : WORD installParaLen  --安装参数的长度
*
*@return     : WORD status
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*/
WORD makeInstallForInstall(WORD installParaLen)
{
	WORD idata curOff ;
	WORD idata status;

	
	if(installParaLen == 0) 
	{
		return FALSE;
	}
		
	curOff = 0;
	
	appendLVApdu(RecData,0x4F,&curOff,1);//Executable Load File AID
	
	appendLVApdu(RecData,0x4F,&curOff,2);//Executable Module AID
	
	appendLVApdu(RecData,0x4F,&curOff,3);//Application AID
		
	appendLVApdu(RecData,0x82,&curOff,1);//Privileges
	
	if(installParaLen > 0xFF)  //BER-TLV格式  modify by chentao@20221117 修改边界处理 0x100需要两个 字节处理，不然会被截断成0,0x80目前统一处理成0x81 0x80
	{
		RecData[curOff++] = 0x82;
		
		RecData[curOff++] = (installParaLen >> 8)&0xFF;
		
		RecData[curOff++] = installParaLen&0xFF;
	}
	else if(installParaLen > 0x7F)
	{
		RecData[curOff++] = 0x81;
		
		RecData[curOff++] = installParaLen&0xFF;
	}
	else
	{
		RecData[curOff++] = installParaLen;
	}
	
	appendTLVApdu(RecData,0xC9,&curOff);//installPara
	
	appendTLVApdu(RecData,0xEF,&curOff);//installPara
	
	appendTLVApdu(RecData,0xEA,&curOff);//installPara
	
	RecData[curOff++] = 0;//TOKEN
	
	CLA = (0x80 | selected_channel);
	
	INS = 0xE6;
	
	P2 = 0x00;
	
	if(curOff < 0x100)
	{
		P3 = curOff;
	}
	else//暂时不支持大于0x100的安装指令
	{
		return 0x6985;
	}
		
	status = IF_ExcuteProfileGPCommand(MNOSD_APPID);
	
	return status;
		
}

/*
*@funtion    :ApplicationPEProcess
*
*@description: 对PE-Application进行解析处理
*
*@param      :
*             
*@return     :BYTE    --TRUE:继续执行
*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE ApplicationPEProcess(void)
{
	BYTE idata result,temp,result1;
	WORD idata sw,tempLen;
	BYTE xdata instanceAID[0x11];
	BYTE xdata extraditeSDAID[0x11];
	//BYTE idata needExtradite;
	
	result = TRUE;
	ecpMem_Set(instanceAID, 0x00, 0x11);
	ecpMem_Set(extraditeSDAID, 0x00, 0x11);
	//needExtradite = FALSE;
		
	if((ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == FALSE) && (G_PETlvInfo.bufferLen < 0x0F))//第一次进来，且前面的数据太短，直接备份，返回
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}
	
	if(G_PEControlContext.C4RemainLen > 0)
	{
		sw = makeRemainLoad();
		if(processStatusWord(sw,TRUE) == FALSE)
		{
			return FALSE;
		}
	}
	
	if(G_PETlvInfo.bufferLen == 0)//继续接收PEApplication中load包的剩余数据
	{
		return TRUE;
	}  
	
	if(ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == TRUE)
	{
		temp = VMReadByte(G_PETlvInfo.bufferAddr);
	
		if(temp == 0x30)//恰好在值的结束处断开
		{
			G_PETlvInfo.levelIndex = 1;
		}

		if(temp == 0x04)//红茶移动，德国profile测试发现的问题，安装应用30下面有好多0x04tag，分多批数据下发
		{
			result1 = ProcessDataProcess(0,G_PETlvInfo.bufferLen);

			if(result1 != 2)//等于2，表明04已经处理完，接着处理上一层级的30
			{
				return result1;
			}
		}
	}
	else
	{
		SetBit(G_PEControlContext.IsExistFlag2,OccurFlag);
	}
	
	if(G_PETlvInfo.levelIndex == 1)//对于case 0xA1: 只有子tag4F-C1才会赋值G_profileControlContext.TotalLen，到C4里面以后不会赋值 再次进入该函数会先sw = makeRemainLoad();处理数据，如果数据不够会返回，如果cap包数据结束 会initTLVinfo,所以这里tempLen的赋值没有问题
	{
		tempLen = G_PETlvInfo.valueLenBuffer[0]-G_profileControlContext.TotalLen; //层级1的剩余长度（层级1所有tag剩余总长度） （层级0的某个tag的剩余待处理长度）
		if(tempLen > G_PETlvInfo.findNextLen)
		{
			G_PETlvInfo.lenBuffer[0] = 0;//层级0的当前tag的下一个Tag本次需要处理的长度，因为本次数据不足以处理完当前tag 所以本次不需要处理下一个tag
			G_PETlvInfo.offBuffer[0] = 0;//层级0的当前tag的下一个Tag在源数据中的偏移，因为本次数据不足以处理完当前tag 所以本次不需要处理下一个tag
		}
		else
		{
			G_PETlvInfo.lenBuffer[0] = G_PETlvInfo.findNextLen-tempLen;//层级0的当前tag的下一个tag本次需要处理的长度（源数据总长度 - 层级0当前tag剩余长度）
			G_PETlvInfo.offBuffer[0] = tempLen;//层级0的当前tag的下一个tag在源数据的偏移
			G_PETlvInfo.findNextLen = tempLen;//层级0的当前tag本次需要处理的长度
		}
	}		
	
	while(1)
	{
		if(G_PETlvInfo.levelIndex == 0)
		{
			result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		}
			
		if(result == FALSE)
		{
			if(G_PETlvInfo.findNextLen > 0)
			{
				backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
				
				return TRUE;
			}
			else
			{
				break;  //处理到最后一个tag
			}			
		}		
		
		
		if(G_PETlvInfo.tagBuffer[0] != 0xA0)
		{
			if( G_PEControlContext.TagA0Occ == 0)
			{
				appendErrorResponse(INVALID_PARAMETER,0);
				return FALSE;
			}	
		}
		
		switch(G_PETlvInfo.tagBuffer[0])
		{
			case 0xA0:  //process sd-Header
				
				if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
				{
					return FALSE;
				}
			
				if(G_profileControlContext.IsCreateMNOSD == 0)
				{
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//Installing profile with PE-Application before PE-SecurityDomain, eUICC reports error. eUICC responses with PEStatus (5) invalid-request-format or PEStatus (6) invalid-parameter
					return FALSE;
				}
				break;
					
			case 0xA1:  //loadBlock ApplicationLoadPackage OPTIONAL,	
                
				if(G_PETlvInfo.levelIndex == 0)
				{
					G_profileControlContext.TotalLen = 0;
					G_PEControlContext.ApplicationLoadPackageOff = G_PETlvInfo.findTagOff;
				}
				
				temp = ApplicationLoadPackageAnalyze();
			    if(temp == FALSE)
				{
					return FALSE;
				}
                else if(temp == 2)  //数据不完整
				{
					return TRUE;
				}					
				break;
			
			case 0xA2:  // instanceList SEQUENCE OF ApplicationInstance OPTIONAL
				
				if(G_PETlvInfo.levelIndex == 0)
				{
					setVerticalFind(&G_PETlvInfo); // TLV嵌套，设置垂直查找信息
					G_profileControlContext.TotalLen = 0;
				}
				
				while (1)
				{	
					result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
					
					if(result == FALSE)
					{
						if(G_PETlvInfo.findNextLen > 0)
						{
							backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff, G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
							return TRUE;
						}
						else
						{
							break;
						}
					}
											
					if(G_PETlvInfo.isCompleteBuffer[1] == FALSE)//处理的30tag不完整,且未超出备份区能力，则返回，继续接收下一批数据
					{
						if(backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findTagOff, G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff) == TRUE)//如果A2/30/30数据很长，备份能力不够的话，继续执行
						{
							return TRUE;
						}
					}					
					
					G_profileControlContext.TotalLen += G_PETlvInfo.findTLVLen;
					
					switch(G_PETlvInfo.tagBuffer[1])
					{
						case 0x30:
						{
							if(ApplicationInstanceAnalyze(PEAPP_MODE) == FALSE)
							{
								return FALSE;					
							}
														
							break;
						}
						default: //白盒测试添加
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);// add by chentao@20230207  eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03   PE里的子tag超出规范定义
							return FALSE;
					}
				}
				
				setReturnFind(&G_PETlvInfo);
										
				break;
				
				default: //其余的tag要报错
					appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);// add by chentao@20230207  eUICC_M2M_additonal/压力测试/Profile_DownLoad_pressure_Test_03   PE里的子tag超出规范定义
		            return FALSE;
		}

	}
	
	return TRUE;
}

/*
*@funtion    :makeInstallForRegistryUpdate
*
*@description: 根据GP2.2.1规范，拼装标准的InstallForRegistryUpdate指令到APDUBuffer中
*
*@param      : WORD installParaLen  --安装参数EF的长度
*
*@return     : WORD status
*
*@author     :zxx
*
*@history    :V1.0 :create by lgy@20210219
*/
WORD makeInstallForRegistryUpdate(BYTE* buffer ,WORD installParaLen)
{
	WORD idata curOff ;
	WORD idata status;


	if(installParaLen == 0)
	{
		return 0x9000;
	}

	ecpMem_Set(RecData,0,256);
	curOff = 4;

	if((installParaLen+2) > 0x7F)
	{
		RecData[curOff++] = 0x81;
	}

	if(installParaLen > 0x7F)
	{
		RecData[curOff++] = installParaLen + 3;
	}
	else
	{
		RecData[curOff++] = installParaLen + 2;
	}

	RecData[curOff++] = 0xEF;

	if(installParaLen > 0x7F)
	{
		RecData[curOff++] = 0x81;
	}
	RecData[curOff++] = installParaLen;

	ecpMem_Copy(&RecData[curOff],buffer,installParaLen);

	curOff += installParaLen;

	CLA = (0x80 | selected_channel);

	INS = 0xE6;
	P1 = 0x40;
	P2 = 0x00;

	if(curOff < 0x100)
	{
		P3 = curOff;
	}
	else//暂时不支持大于0x100的安装指令
	{
		return 0x6985;
	}

	status = IF_ExcuteProfileGPCommand(MNOSD_APPID);

	return status;

}

#endif

