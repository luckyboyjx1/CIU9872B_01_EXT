/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCRemoteAppManagement.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-22

*Description :该文件主要用于实现基于TS10226 第8章节所描述的RAM功能，主要包括load,install,remove application以及set status ,get status,get data等命令，这些命令的执行是在GP平台的安全域的控制之下的;
*             程序暂时没有验证下行报文中DAP的正确性
*
*History     :
	
author    time      version    description
xuyz   2013/10/22   V1.0       创建该文件
************************************************************************************************************************************************************************************************/

#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "Arithmatic.h"
#include    "uicc_interface.h"
#include    "Object.h"
#include    "ToolkitAPIPublic.h"
#include    "STKRemoteAppManagement.h"
#include    "ToolkitRFM.h"
#include    "filesystem.h"
#include    "opFunction.h"
#include    "securityDomainData.h"
#include	"ob_manager.h"
#include    "bc_array.h"
#include 	"packageloc.h"
#include 	"SEEUICCInterface.h"
#include	"EUICCM2MAPDUPublic.h"

#if UICC__FUNCTION__STKAPI
#include "dataManager.h"	//remove warning,dengyiwen,2022/9/27
#include "reference.h"
/*@function:    GetLastInstallSequence                                            
 * 
 *@descrip:     获取当前新安装应用的安装时间 = (最后一个应用的安装时间 + 1)
 *
 *@see:	        no 
 *                                                                                                                                     
 *@param:       void 
 *				                                                                                                         
 *@return:      LastestInstallSeq                                                                                                                                                                                                                                                                    
 * 
 *@author:      LJ 20131023
 *  
 *@history:     V1.0---Create by LJ 20131023     
 *           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
 *                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能                      
 */
WORD GetLastInstallSequence(void)
{
	WORD xdata LastestInstallSeq,InstallSeq;
	BYTE idata i,result;
	UWORD xdata toolkitAppAddr;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	BYTE  indexNum;
	LastestInstallSeq = 0;
	indexNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for(i=0;i<indexNum;i++)
	{
		toolkitAppAddr   = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}

		ReadLFBufForJavaAccess(toolkitAppAddr,i,InstallSequence_OFFSET,(BYTE *)&InstallSeq,sizeof(WORD));

		if((InstallSeq >= LastestInstallSeq)&&(InstallSeq != 0xFFFF))
		{
			LastestInstallSeq = InstallSeq;
		}
	}
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	LastestInstallSeq = 0;

	for(i=1;;i++)
	{
		result = ReadLFBufForJavaAccess(toolkitAppAddr,i,InstallSequence_OFFSET,(BYTE *)&InstallSeq,sizeof(WORD));

		if(result == FALSE)
		{
		   break;
		}

		if((InstallSeq >= LastestInstallSeq)&&(InstallSeq != 0xFFFF))
		{
			LastestInstallSeq = InstallSeq;
		}
	} 
#endif
	return 	(LastestInstallSeq+1);
}

/*
*@function   :  ToolkitFindSDTars
*
*@description: 查找sd Tar是否已经安装过
*
*@param       :tarValue:当tarValue为空时，表示查找一条空记录，否则表示查找特定的tars是否在某条记录中
*
*@return      :FALSE/Offset
*
*@author      ：xuyz
*/
BYTE ToolkitFindSDTars(BYTE *tar)
{
	u8         X_A    bFreeEntryIndex,index;
	jref       X_A    bPrivRef,SDdataRef;
	memref 	   X_A    entry,theAppTable,bPrivArray,sdDataObj,sdTarBuffer;
	u8         X_A    appId,tarNum,k,tmp[3];

	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable = getAppTable();

	for (index = APP_FIRST; index < bFreeEntryIndex; U16_INC(index))
	{
		entry = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(index, SIZE_REF))));
		appId = memReadU8(MEMtPN8(get_object_field_ref(entry, bAppID_INDEX)));

		if(appId == APP_NULL)
		{
			continue;
		}

		bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));

		bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
		if((U8_AND_U8(memReadU8(MEMtPM8(bPrivArray)), SECUREDOMAIN_PRIVI_TAG) != SECUREDOMAIN_PRIVI_TAG)
			||(appId == ILLEGAL_APPID))
		{
			continue;
		}

		SDdataRef = dataManager_getSdDataRef(appId);

		if (REF_NULL == SDdataRef)
		{
			continue;
		}

		sdDataObj = locateObjectInMemory(SDdataRef);

		sdTarBuffer = sdData_getSDTarbufferAddr((PX8)sdDataObj);

		if(sdTarBuffer == ADDR_IS_NULL)//没有TAR
		{
			continue;
		}

		tarNum = memReadU8(MEMtPM8(sdTarBuffer + sdTarNumber_offset));

		for(k = 0;k < tarNum;k++)
		{
			VMReadBuf(&tmp[0],sdTarBuffer + sdTarValue_offset + k*3,sizeof(tmp));
			if(ecpMem_Compare(&tmp[0], tar, 0x03) == EQUAL)
			{
				return memReadU8(MEMtPM8(sdTarBuffer + sdAppId_offset));//返回SD的appid
			}
		}

	}
	return ILLEGAL_APPID;

/*
	BYTE idata j;
	BYTE idata recNum;

	UWORD xdata fileAddr;
	BYTE idata tarNum,k;
	BYTE xdata tmp[0x04]; 
    

	fileAddr = SelectLFFile(DF_Toolkit_P,EF_RAMAPDUTars);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(j = 1; j <= recNum; j++)
	{
		ReadLFBuf(fileAddr, j ,0x00,&tmp[0],sizeof(BYTE));

		if( tar == NULL)   // 当tarValue为空时，表示查找一条空记录 E:\N3_TEE\develop\others\test\中国联通NFC-SWP测试工程集测试脚本20150818\中国联通NFC-SWP测试工程集测试脚本20150817\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\uicc_configuration_additional_test_工程集.uce\CodeReview_43_2
		{
			if(tmp[0] == 0xFF)	  //空记录
			{
				return j;
			}
			else
			{
				continue;
			}
		}
		else
		{
			if(tmp[0] == 0xFF)	  //空记录
			{
				continue ;
			}
		}

		ReadLFBuf(fileAddr, j , sdTarNumber_offset,&tarNum,sizeof(BYTE));
		
		for(k = 0x00; k < tarNum ; k++)
		{
			ReadLFBuf(fileAddr, j , sdTarValue_offset+k*3,&tmp[0],sizeof(tmp));
			
			 if(ecpMem_Compare(&tmp[0], tar, 0x03) == EQUAL)
			 {
			 	return j;
			 }
		}
	}

	return FALSE;
	*/
}
/*
*@funtion     :UICCNativeImpl_RAMIsValidForTARs

*@description :该函数主要用来处理判断当前install (for install)中的tars是否已经分配过
*
*@see         :TS 102 226,chapter 8(8.2.1.3.7)
*
*@param       :*tarValues
*@param       :tarLgth  :3的整数倍
*
*@return      :FALSE: APDU中的tars已经分配/TRUE:APDU中的tars都没有分配过
*
*@author      :xuyz
*
*@history     :V1.0: create by xuyz @2013-10-23
*             :V1.1: modify by chentao@EPX产业化\INFENION_SLM97\Develop
*
*@others      :
*/
BYTE  UICCNativeImpl_RAMIsValidForTARs(BYTE *tarValue, BYTE tarLgth)
{
	BYTE  idata num,i,tarExit,j;
#if SE_AND_EUICC
	BYTE  idata backProfileId,k;
	BYTE  xdata occur[PROFILE_MAX_NUMBER];
	BYTE  xdata buf[PROFILE_MAX_NUMBER];
	UWORD xdata backFileSystemAddr;
#endif
	
	if(tarLgth == 0x00)
	{
		return TRUE;
	}

	num = 1;
	tarExit = FALSE;

#if SE_AND_EUICC
	backFileSystemAddr = G_CurMFFileStart;
	backProfileId = G_CurExecutingProfileIndex;
	ecpMem_Set(occur,0,sizeof(occur));

	if(G_CurExecutingProfileIndex == ILLEGAL_PROFILEINDEX)//在SE区安装应用 ，检查所有profile
	{
		VMReadBuf(buf,PROFILE_VALIDTAG_CFG,PROFILE_MAX_NUMBER);    //获取当前所有profile的状态
		j = 0;
		for(k = 0; k < PROFILE_MAX_NUMBER;k++)
		{
			if(buf[k] == PROFILE_VALIDFLAG)
			{
				num++;//add 有效profile个数
				occur[j++] = k;//profileIndex
			}
		}

	}
	else//在profile区安装应用 只需要检查SE区
	{
		num = 2;
		occur[0] = ILLEGAL_PROFILEINDEX;
	}
#endif

	for(j = 0; j < num; j++)//复用J
	{
#if SE_AND_EUICC
		if(j > 0)//j =0 表示当前正在执行的区域，先查找当前区域 ， 再切换到其他区域查找
		{
			G_CurExecutingProfileIndex = occur[j-1];
			ExecuEnvironmentInit(G_CurExecutingProfileIndex);
			if(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX)//如果是SE安装应用 ，切到profile区查找 需要将文件系统地址一起切换
			{
				G_CurMFFileStart = VMReadUword(PROFILE_MFOFFSET_CFG + G_CurExecutingProfileIndex*sizeof(UWORD));
			}
		}
#endif
		for(i = 0x00; i < tarLgth ; i += 0x03)
		{
			if(ToolkitFindSDTars(&tarValue[i]) != ILLEGAL_APPID)  //当前安装的Tar是否在sdTar中已存在
			{
				tarExit = TRUE;//return FALSE;
				break;
			}

			if(ToolkitfindSpecifiedTARValue(&tarValue[i]) != FALSE)  //当前TAR值已经在其他的toolkit app中分配过了
			{
				tarExit = TRUE;//return FALSE;
				break;
			}

			if(FindRFMTars(&tarValue[i]) != FALSE)  //当前TAR值已经在其他的rfm中分配过了
			{
				tarExit = TRUE;//return FALSE;
				break;
			}
	#if M2M_EUICC
			if(IsISDP_Tar(&tarValue[i],NORMAL_CHECK_ISDPTAR) != ILLEGAL_PROFILEINDEX)//当前TAR值和ISD-P ISD-R的是否重复
			{
				tarExit = TRUE;//return FALSE;
				break;
			}

			if(IsISDR_ECASD_Tar(&tarValue[i]) != FALSE)
			{
				tarExit = TRUE;//return FALSE;
				break;
			}
	#endif
		}

		if(tarExit == TRUE)
		{
			break;
		}
	}
#if SE_AND_EUICC
	if(backProfileId != G_CurExecutingProfileIndex)
	{
		G_CurExecutingProfileIndex = backProfileId;
		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
		if(G_CurMFFileStart != backFileSystemAddr)
		{
			G_CurMFFileStart = backFileSystemAddr;
		}
	}
#endif
	 if(tarExit == TRUE)
	 {
		 return FALSE;
	 }

	 return TRUE;
}
#if UICC__FUNCTION__UICCAPI
/*
*@funtion     :UICCNativeImpl_ProcessRAMAccessDomain

*@description :	处理EA数据里面的81,82 tag
*
*@see         :TS 102 226,chapter 8(8.2.1.3.2.2.2)
*
*@param       :BYTE *buf  : 缓存当前数据的buf : 即EA的data 域
*@param       :bufLgth	  : 即EA的数据长度
*@param       :tag       : 0x81: UICC Access Application specific parameters / 0x82 : UICC Administrative Access Application specific parameters
*
*@return      :TRUE:数据成功执行,已缓存到tempBuf的正确位置 /FALSE:数据体有问题
*
*@author      :xuyz
*
*@history     :V1.0 : create by xuyz
*
*@others      :1:该函数的数据最后缓存在tempBuf中
*/
BYTE UICCNativeImpl_ProcessRAMAccessDomain(BYTE *buf, BYTE bufLgth, BYTE tag)
{
	BYTE idata AccessDomainOffset,AccessDomainLgth;
	BYTE idata flag,fileAccessDomainOffset;
	BYTE idata aidLgth,DAPLgth,accessDomainLen;
	BYTE idata arrNum;
	BYTE xdata adfAID[0x11];

	AccessDomainOffset = FindTag(&buf[0], tag, bufLgth);   //查找tag 81

	if(AccessDomainOffset == FALSE)
	{
		return TRUE;
	}

	AccessDomainLgth = 	buf[AccessDomainOffset ++];

	if(tag == 0x81)
	{
		fileAccessDomainOffset = ADFAccessDomainValue_OFFSET;
	}
	else
	{
		fileAccessDomainOffset = AdminAccessDomainValue_OFFSET;
	}

	if(AccessDomainOffset == FALSE)
	{
		return TRUE;
	}

	flag = FALSE;

	arrNum = 0x00;

	while(AccessDomainLgth > 0x00)
	{
		aidLgth = buf[AccessDomainOffset ++];  	//AccessDomainOffset point to Empty UICC file system AID OR ADF AID

		adfAID[0] =  aidLgth;

		ecpMem_Copy(&adfAID[1], &buf[AccessDomainOffset], aidLgth);

		AccessDomainLgth -= (aidLgth + 1);

		AccessDomainOffset += aidLgth; //point to Length of Access Domain

		accessDomainLen = buf[AccessDomainOffset++];  //Access Domain for UICC file system OR ADF AID

		if((accessDomainLen != 0x00)&&(accessDomainLen != 0x01)&&(accessDomainLen != 0x03)&&(accessDomainLen != 0x04))	 //判断 acces Domain lgth的有效性
		{
			return FALSE;
		}

		if(accessDomainLen > 0x00)
		{
			if((buf[AccessDomainOffset] != 0x00)&&(buf[AccessDomainOffset] != 0x01)&&(buf[AccessDomainOffset] != 0x02)&&(buf[AccessDomainOffset] != 0xFF))	 // 00 ;01 XXXX;02 XXXXXX; FF  or no
			{
				return FALSE;
			}
		}

		if((flag == FALSE)&&(aidLgth == 0x00))		 //Length of UICC file system AID
		{	
			if(tag == 0x81)
			{
				tempBuf[MFAccessDomainLength_OFFSET] = accessDomainLen;   // tempBuf 在使用前已经全部初始为0了 
				
				ecpMem_Copy(&tempBuf[MFAccessDomainContent_OFFSET],&buf[AccessDomainOffset],accessDomainLen);

				goto _NEXT_;	

			}
			else
			{
				arrNum = 0x00;
			}
		}
		else if((aidLgth >= 0x05)&&(aidLgth <= 0x10))
		{
			arrNum = GetAIDNumber(adfAID,FALSE);	   	//判断ADF aid在2F00中是否存在,如果存在，则返回记录号

			if(arrNum == 0x00)
			{
				return FALSE;
			}						
		}
		else
		{
			return FALSE;
		}

	   	tempBuf[fileAccessDomainOffset ] = arrNum;  //2F00中的记录号		

		tempBuf[fileAccessDomainOffset +1] = accessDomainLen;

		ecpMem_Copy(&tempBuf[fileAccessDomainOffset+2],&buf[AccessDomainOffset],accessDomainLen);

		fileAccessDomainOffset += 0x06;	  //每个权限信息有6个字节表示：ArrNum，lgth，AccessDomainPara,AccessDomainData

_NEXT_:
		AccessDomainOffset += (accessDomainLen);

		AccessDomainLgth -= (accessDomainLen + 1);

		DAPLgth = buf[AccessDomainOffset++];

		AccessDomainOffset +=  DAPLgth ;

		AccessDomainLgth -= (DAPLgth + 1);	

		flag = TRUE; //ADF AID不允许长度为0	
	}

	if(tag == 0x81)
	{
		tempBuf[ADFAccessDomainLgth_OFFSET] = (fileAccessDomainOffset-ADFAccessDomainValue_OFFSET);
	}
	else
	{
		tempBuf[AdminAccessLgth_OFFSET] = (fileAccessDomainOffset-AdminAccessDomainValue_OFFSET);
	}

	return TRUE;
}
#endif
/*
*@funtion    : InstallforInstall_RAMSD(&buf[InstallfieldOffset], InstallfieldLength);   
*
*@description: 该函数主要用于处理安装sd时存在CA或ea参数的情况
*
*@param     :&buf[0]:for EA: UICC Toolkit Application specific parameters			           for CA: SIM file access and toolkit Application specific Parameters
*@param     :installFieldLength :Length of UICC Toolkit Application specific parameters field 	       Length of SIM file access and toolkit application specific parameters field
*@param     :tagFlag: 0x01:EA TAG存在 / 0x02:CA tag 存在
*@param     :appId:应用的appId
*
*@return     :SWStstus
*
*@author     :xuyz
*
*@history    :V1.0 :create by xuyz
*/

WORD InstallforInstall_RAMSD(BYTE *paraFiled, BYTE paraLen,BYTE tagFlag,BYTE appId)
{
	BYTE idata offset;
	BYTE idata tmpLen,tmpValue;
	//UWORD xdata fileAddr;

	ecpMem_Set(tempBuf, 0x00, sizeof(tagRAMAPDUTarsInfo));  //清空特定文件的某一记录

	tempBuf[sdAppId_offset] = appId;

	offset = 0;

	if((tagFlag&0x02) == 0x02) //先处理CA tag中Length of Access Domain field ，使其和EA tag中的数据对齐
	{
		tmpLen = paraFiled[offset ++];  //point to Length of Access Domain field

		offset += tmpLen;  //point to Priority level of the Toolkit application instance

		paraLen -= (tmpLen + 1);  //CA tag中除AccessDomain外的剩余数据长度
	}
	
	 if(paraLen < 4)  //参数： Priority level of the Toolkit application instance、  Maximum number of timers allowed for this application instance、Maximum text length for a menu entry、Maximum number of menu entries allowed for this application必须存在instance
	 {
		 return 0x6A80;	
	 }

    offset ++; //point toMaximum number of timers allowed for this applet instance

	paraLen --;  //包括timer字节 在内的长度
	

	offset ++; //point to Maximum text length for a menu entry

	paraLen --;  //包括text length字节在内的长度
	

	offset ++; //point to Maximum number of menu entries allowed for this application instance

	paraLen --;  //paralen包括 meunuentry number字节在内的长度
	

	tmpValue = paraFiled[offset] ; //获取 Maximum number's Value
	 
	if(paraLen < (tmpValue* 2 + 1))  //剩余数据长度不够
	{
		return 0x6A80;
	}
	
	if((paraLen-2*tmpValue-1) > 0) //23048规范和SIMAPI测试脚本中安装参数最大通道数可选
	{
		offset += (2*tmpValue+1);   //point to Maximum number of channels for this application instance

		paraLen -= (2*tmpValue+1);   //剩余数据长度，包括channel长度
		

		offset ++;   //point to Length of Minimum Security Level field
		paraLen --;  //剩余数据长度，包括length OF msl的长度
		

		if(paraLen > 0)  //MSL 存在
		{			
// 			ecpMem_Copy(&tempBuf[sdMslValue_Offset], &paraFiled[offset],paraFiled[offset] +1);
// 			
// 			if(tempBuf[sdMslValue_Offset] > 0x02)  //msl暂时只支持 01 mini的情况，所以其长度一定不会超过2
// 			{
// 				return 0x6A80;
// 			}
			
			if(paraFiled[offset] == 0x00) // MSL  LEN  = 0x00
			{
				tempBuf[sdMslValue_Offset] = 0x00;
			}
			else
			{		
				ecpMem_Copy(&tempBuf[sdMslValue_Offset], &paraFiled[offset+2],1);//值拷贝minispi 不要Len 指示符
			}
			
			if(paraFiled[offset] > 0x02)  //msl暂时只支持 01 mini的情况，所以其长度一定不会超过2
			{
				return 0x6A80;
			}
			
			if((paraLen -1-paraFiled[offset]) > 0x00)//23048规范和SIMAPI测试脚本中安装参数TAR可选
			{
				offset += (paraFiled[offset] +1); //point to Length of TAR Value(s) field
				
				if((paraFiled[offset]%0x03) != 0x00) //TAR length 不是3的整数倍
				{
					return 0x6A80;
				}
				
				tempBuf[sdTarNumber_offset]	 = (paraFiled[offset]/3);   //为和海外平台保持一致，这里存储的时候也修改成tar的数目
			}	
		}
		else  //If the Maximum number of channels field is included in the command data then the Length of Minimum Security Level field shall also be included.
		{
			return 0x6A80; 
		}
		
	}

	if(tempBuf[sdTarNumber_offset] == 0x00)   //此处考虑是否要使用sd的appID中代表的Tars
	{
		tempBuf[sdAppId_offset] = ILLEGAL_APPID;
		SaveSDTarTemp(tempBuf);

		return 0x9000;
	}
	else
	{
		ecpMem_Copy(&tempBuf[sdTarValue_offset], &paraFiled[offset+1],tempBuf[sdTarNumber_offset]*3);
	}

	if(UICCNativeImpl_RAMIsValidForTARs(&tempBuf[sdTarValue_offset], tempBuf[sdTarNumber_offset]*3) == FALSE)  //判断tars是否已经分配过
	{
		return 0x6A80;
	}

	SaveSDTarTemp(tempBuf);

	return 0x9000;
	//return  SaveSdRAMTar(tempBuf, SD_TAR_INFO_LEN, appId);
}
/*
*@funtion    :SaveSDTarTemp
*
*@description: 临时存储SD的TAR到ISD_DefaultTarInfo，创建SD后再存到SDObjet里
*
*
*@return     :
*
*@author     :lgy
*
*@history    :V1.0 :create by lgy
*/

void SaveSDTarTemp(BYTE *paraFiled)
{
	//memref      X_A   tarBufAddr;
	//jref        X_A   tarBufRef;

	VMWriteBuf(PNtMEM(&bRamBuffer[SD_TAR_INFO_OFF]),paraFiled,SD_TAR_INFO_LEN);
}
/*
*@funtion    :SaveSdRAMTar
*
*@description: 给SD分配数组 存TAR信息，为了后续处理方便 ，数组长度按最大值分配 0x0c字节
*
*@param     :paraFiled 已经拼装好的TAR信息  tagRAMAPDUTarsInfo
*@param     :paraLen  信息长度
*@param     :appId:应用的appId
*
*@return     :SWStstus
*
*@author     :lgy
*
*@history    :V1.0 :create by lgy
*@history    :V1.1 :modify by xuxiaojun 不调用get_array_data，可以减少_newarrayObject中提交一次cache
*/

WORD SaveSdRAMTar(BYTE *paraFiled, BYTE paraLen,BYTE appId)
{
	memref      X_A   sdDataObj,tarBufAddr;
    jref        X_A   tarBufRef;
    u8          X_A   tmp[SD_TAR_INFO_LEN];

    if(paraLen > SD_TAR_INFO_LEN)
    {
    	return 0x6A80;
    }

    ecpMem_Set(tmp,0,sizeof(tmp));

    VMReadBuf(tmp,PNtMEM(paraFiled),paraLen);

    if(tmp[sdAppId_offset] == ILLEGAL_APPID)// 安装参数下来的EA/ CA中不包含SDTAR
    {
    	return 0x9000;
    }

	tarBufRef =	_newarrayObject(AType_Byte, SD_TAR_INFO_LEN, BIT_NEED_REFRESH);

	tarBufAddr = get_array_data(MEMtPF8(locateObjectInMemory(tarBufRef)));

	VMWriteBuf(tarBufAddr,tmp,SD_TAR_INFO_LEN);

	sdDataObj = sdData_getsdDataObjByAppId(appId);

	memWriteU16Atomic(MEMtPM8(get_object_field_ref(sdDataObj, SD_Tar_buffer_INDEX)),tarBufRef);

	return 0x9000;
}


/*
*@funtion     :NativeImpl_InstallForMakeSelectable

*@description :	该函数主要用来处理 install (for make selectable)APDU命令时，将数据区域与ToolkitApp相关的参数缓存在指定的文件中
*
*@see         :TS 102 226,chapter 8(8.2.1.3.2)
*
*@param       :buffer：install (for install) 的 命令数据,包括5字节的APDU Header +APDU Data
*
*@return      :0x9000/0x6A80/other SW status
*
*@author      :xwh
*
*@history     :V1.0: create by xwh @20140317
*
*@others      :1：暂时仅支持install (for make selectable)为单条的情况
*             :V1.0.1: 修改toolkit应用被selectable后的MenuEntryStatus_OFFSET的状态。同时，如果该toolkit应用有菜单，则SetBit(G_ToolkitSystemProactiveFlag,Bit_SetUpMenu);
*                      该需求来源于印度RJIO测试卡项目：《20180806161512_LYF LS 4510_10K Applet Download+Install(HTTP_PUSH)_then Make selectable_Not visible on Card but at server it is successfull_PoR OK_Activation c.txt》
*                                                       《20180807123412_LYF LS 5013_10K Applet Download+Install then make selectable(PUSH)_seccess_Activation card_other vendor card.txt》
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
WORD NativeImpl_InstallForMakeSelectable(BYTE appId)
{
	BYTE  idata recNum,appstatus,i;
	BYTE  xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr,toolkitMenuEntryAddr;

	if((APDUBuffer[2]&0x08)==0)	  //not For make selectable
	{
		return 0x9000;
	}
	/*
	if((buf[4]<4)||(buf[7]<5)||(buf[7]>16))	  //Aid length is 5-16
	{
		return 0x6A80;
	}

	recNum = UICCToolkitFindSpecifiedAID(&buf[8],buf[7]);
*/
	recNum = GetToolkitEntryAppNumByAppId(appId);

	if(recNum == FALSE)	 //当前应用不是Toolkit App
	{
		return 0x9000;   
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(recNum));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{
		return 0x9000;
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,AppletStatus_OFFSET,&appstatus,0x01);
	
	SetBit(appstatus,APP_SELECTABLE_BIT);

	WriteLFBufForJavaAccess(toolkitAppAddr,recNum,AppletStatus_OFFSET,&appstatus,sizeof(BYTE));

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,tmp,sizeof(tmp));

	for(i= EVENT_EVENT_DOWNLOAD_MT_CALL_bit;i<=EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit;i++)
	{
		if(i == EVENT_STATUS_COMMAND_bit)
		{
			continue;
		}
		
		if(ToolkitGetEventBit(&tmp[0], i)==TRUE)
		{
			SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
			break;	
		}	
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,MaxEntryCount_OFFSET,tmp,sizeof(BYTE));  //获取该toolkit应用中菜单的个数
	
	if(tmp[0] > 0)
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);
		
		toolkitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);       //复用toolkitAppAddr变量
	
		appstatus = GetLFRecNumLgth(toolkitMenuEntryAddr, FileNum);                //复用appstatus变量
		
		//修改MenuEntryStatus_OFFSET的状态
		for(i = 0x01;  i <= appstatus; i++)
		{
			if(ReadLFBuf(toolkitMenuEntryAddr, i , MenuEntryStatus_OFFSET, tmp ,0x02) != FALSE)   //当前记录为非空记录
			{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
				if(appId == tmp[AppRecNum_OFFSET])    //找到当前应用
#else
				if(recNum == tmp[AppRecNum_OFFSET])    //找到当前应用
#endif
				{
					tmp[0] = tmp[0] |Menu_SELECTABLE;  //只有install make selectable后 entyr才显示20140317
					
					WriteLFBuf(toolkitMenuEntryAddr,i,MenuEntryStatus_OFFSET,tmp,0x01);  //修改MenuEntryStatus_OFFSET的值
					
					break;
				}
			}
		}
	}
	
	
	return 0x9000;
}

/*
*@funtion :   FindSpecifiedSDTar
*
*@description ：在RAM tARS中查找指定的sdId
*
*@param      :sd
*
*@return    TRUE/FALSE
*/

//BYTE  FindSpecifiedSDTar(BYTE appId)
//{
//	UWORD xdata fileAddr;
//	BYTE idata recNum,j,sdId;
//
//	fileAddr = SelectLFFile(DF_Toolkit_P,EF_RAMAPDUTars);
//
//	if(fileAddr == FALSE)
//	{
//		return FALSE;
//	}
//
//	recNum = GetLFRecNumLgth(fileAddr,FileNum);
//
//	for(j=1 ; j<= recNum; j++)
//	{
//		ReadLFBuf(fileAddr,j,sdAppId_offset,&sdId, sizeof(BYTE));
//
//		if(sdId == ILLEGAL_APPID)
//		{
//			continue;
//		}
//
//		if(sdId == appId)
//		{
//			return j;
//		}
//	}
//
//	return FALSE;
//}

/*
*@funtion     :NativeImpl_Delete

*@description :该函数主要用于在删除卡片上应用时，若该应用为toolkit app,则删除相对应的文件2800/6F80 6F81 6F82 6f83 6f84等
*
*@see         :TS 102 226,chapter 8(8.2.1.1)
*
*@param       :apdu：

*@return      :0x9000/other SW status
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz
*@history     :V1.1 :create by yjr@20210402 参考102241 V16 void requestPollInterval(short duration)
*																	throws ToolkitException,
*																		TransactionException
*应用在申请PollInterval时，如果申请的值不是0，应该返duration的最小值，如果申请的值是0，需要和当前其他应用申请的最小值比较，在决定发polloff或者pollinterval，
*测试用例uicc_configuration_additional_test_工程集/GP补充测试/GP TEST/GP 补充Multi_app_requestPollInterval_Test01
**           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*            :modify by xuweihua@20211113如果toolkitNum 大于EF_ToolkitAppPathInfo 的记录个数，说明EF_ToolkitAppPathInfo文件记录个数不够，报错
*@others      :其他说明，如该函数使用的限制条件等
*/
WORD NativeImpl_Delete(BYTE appId)
{
#if UICC__FUNCTION__STKAPI
	BYTE idata recNum,result,i,j,aidLgth;
	BYTE xdata buf[0x10];
	BYTE idata num;
	BYTE idata eventListTag;
#if UICC__FUNCTION__UICCAPI
	UWORD xdata toolkitFileListAddr;
	UWORD  xdata toolkitPathInfoAddr;
#endif
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum;
	UWORD xdata toolkitMenuEntryIndexAddr;
	UWORD xdata toolkitAppAddr;
	WORD xdata minDuration,appDuration;

    BYTE idata k,recLgth,recamount,offset;

	CacheFlush();//删包和应用的模式 删模块entry后 apptable内容会发生移动 被删除的entryIndex后的数据都往前移动了  数据在cache，获取AID时会访问apptable等

	aidLgth =  GetAIDDataByAppID(appId,buf);

	recNum = UICCToolkitFindSpecifiedAID(buf,aidLgth);   //删除2800/6F80中的数据

	if(recNum == FALSE)  //当前应用在2800/6F80下查找不到,即该应用不是tollkit app ,继续执行后续GP流程
	{
		return 0x9000;
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(recNum));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{
		return 0x9000;
	}

	eventListTag = 0x00;

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,EventList_OFFSET,buf,EventList_Length);

	for(j = EVENT_EVENT_DOWNLOAD_MT_CALL_bit; j<= EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit; j++)
	{
		if((j == EVENT_STATUS_COMMAND_bit)||(j == 24)||(j == 30)||(j == 31)||(j == 33)) //24 :EVENT_FORMATTED_SMS_CB_bit //30 31 33  reserved by 3GPP
		{
			continue;
		}
        
		if(ToolkitGetEventBit(&buf[0], j))
		{
			eventListTag = TRUE;
			break;
		}
	}

	if(ToolkitGetEventBit(&buf[0], EVENT_STATUS_COMMAND_bit)) //Cre_Pcs_Spco：testCase12:delete app1，发送poll off主动式命令
	{
		minDuration = FindMinDuration(recNum);
		
		ReadLFBufForJavaAccess(toolkitAppAddr,recNum,PollDuration_OFFSET,(BYTE *)&appDuration, sizeof(WORD)); 
		
		if(appDuration < minDuration)
		{
			if(minDuration == (MAXVALUE_POLL_DURATION + 1))
			{
				SetG_ToolkitSystemProactiveFlag(Bit_PollOff);
				
				appDuration = POLL_NO_DURATION;
				
				VMWriteBuf(E_CUR_DURATION,(BYTE*)&appDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 
			}
			else
			{
				SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);

				SetBit(G_PollIntervalRecNum,(recNum-1));

				VMWriteBuf(E_CUR_DURATION,(BYTE*)&minDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 						
			}
			
		}
		
		//SetG_ToolkitSystemProactiveFlag(Bit_PollOff);
	}
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	result = 0xFF;

	result = WriteLFBufForJavaAccess(toolkitAppAddr,recNum,0x00,&result,sizeof(BYTE));  //清空当前记录,清空首字节为FF

	if(result == FALSE)  //写flash失败
	{
		return 0x6581;
	}
#endif
#if UICC__FUNCTION__2B80_AT_FILEEND
	// 删除应用时，如果删除的是EF_ToolkitApp 文件最后一条记录，则将记录数减一,by xwh 20210710
	AddOrDeleteToolkitAppRecord(recNum);
#endif
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);
	
	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);
	
	ReadTFBuf(toolkitMenuEntryIndexAddr, 0x00,&tempBuf[0],sizeof(BYTE));	 //整理2800/6F82文件

	ReadTFBuf(toolkitMenuEntryIndexAddr, 0x01,&tempBuf[1],tempBuf[0]);

	num = 0x00;

   	for(i = 0x01;  i <= tookitMenuEntryRecNum; i++)
	{
   		ReadLFBuf(tookitMenuEntryAddr, i , 0x00, buf ,sizeof(buf));

		if(buf[0] != 0xFF)  //当前记录为非空记录
		{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			if(appId == buf[AppRecNum_OFFSET])
#else
			if(recNum == buf[AppRecNum_OFFSET])    //删除应用时，若当前应用之前申请了duration,则此时应标记发送poll off主动式命令
#endif
			{
				result = 0xFF;

				//删除应用时，若当前应用之前申请了duration,则此时应标记发送poll off主动式命令

				result = WriteLFBuf(tookitMenuEntryAddr,i,0x00,&result,sizeof(BYTE)); //清空当前记录,清空首字节为FF
			
				if(result == FALSE)  //写flash失败
				{
					return 0x6581;
				}	
			
			    num ++; 

				for(j = 1; j <= tempBuf[0]; j ++)
				{
					if(tempBuf[j] == i )
					{						
						ecpMem_Move(&tempBuf[j],&tempBuf[j+1],tempBuf[0]-j);

						tempBuf[0]--;

   						break;
					}
				}
			}
		}
	}

	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);

	result = WriteTFBuf(toolkitMenuEntryIndexAddr, 0, tempBuf, tempBuf[0]+1);  //将修改过后的2800/6F82文件内容写回

	if(result == FALSE)	 //flash写入失败
	{
	 	return 0x6581;	   
	}
#if UICC__FUNCTION__UICCAPI
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitFileListAddr = SelectTFFile(DF_Toolkit_P, EF_ToolkitFileList);
	
	toolkitPathInfoAddr =  SelectLFFile(DF_Toolkit_P, EF_ToolkitAppPathInfo);
	
	recLgth =  GetLFRecNumLgth(toolkitPathInfoAddr, FileLgth);
	
	offset = recLgth>>1;         // recLgth>>1 EVENT_REMOTE_FILE_UPDATE偏移开始
	
	recamount = GetLFRecNumLgth(toolkitPathInfoAddr, FileNum);			  
	
	if(recamount >= recNum)
	{
		ReadLFBuf(toolkitPathInfoAddr, recNum , 0x00, tempBuf ,recLgth);
		
		ecpMem_Copy(&tempBuf[tempBuf[0]+1],&tempBuf[offset+1],tempBuf[offset]);
		
		tempBuf[0] += tempBuf[offset];
		
		for(i=1;i<=tempBuf[0];i++)
		{
			result = FALSE;  //result 此时用于判断第i个索引所表示的filelist是否被其他应用所占用，如果没有被其他应用占用，则删除该块的filelist

			for(j=1;j<=recamount;j++)
			{
				if(j == recNum)
				{
					continue;
				}

				ReadLFBuf(toolkitPathInfoAddr,j,0,&tempBuf[50],recLgth);
				
				ecpMem_Copy(&tempBuf[tempBuf[50]+1],&tempBuf[50+offset+1],tempBuf[50+offset]);
				
				tempBuf[50] += tempBuf[50+offset];
	
				for(k=1;k<=tempBuf[50];k++) 
				{
					if(tempBuf[50+k] == tempBuf[i])
					{	
						result = TRUE;																	//改块filelist仍然有效，不需要清空

						break;
					}		
				}
			}

			if(result == FALSE)												   //没找到相同的filelist
			{
				ecpMem_Set(&tempBuf[100],0xFF,10);												//该块filelist无效，清空

				result = WriteTFBuf(toolkitFileListAddr,(tempBuf[i]-1)*10,&tempBuf[100],10);
	
				if(result == FALSE)
				{
				//抛出异常 TransactionException
				}
			}
		}
											
		ecpMem_Set(tempBuf, 0x00,recLgth);	   	//清空对应的filelistinf

		result = WriteLFBuf(toolkitPathInfoAddr,recNum,0x00,&tempBuf[0],recLgth);

		if(result == FALSE)
		{
		//抛出异常 TransactionException
		}
	}
#endif
#endif
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	recNum = appId;
#endif
	ProcessNoShareableFileAfterDeleteApplet(recNum);
	
	if(num > 0)
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);
	}

	if(eventListTag == TRUE)
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
	}

	return 0x9000;	
#endif
}
/*
*@funtion     :UICCNativeImpl_GetStatus

*@description :RAM处理getStatus指令
*
*@see         :TS 102 226,chapter 8(8.2.1.6)
*
*@param       :
*
*@return      :
*
*@author      :
*
*@history     :
*
*@others      :其他说明，如该函数使用的限制条件等
*/

//WORD UICCNativeImpl_GetStatus(BYTE *currentLen)
//{
//	BYTE idata i,j,offset;
//	BYTE xdata tmp[5];
//	BYTE idata menuEntryNum,menuEntryIndex;
//	BYTE idata appRecNum,appstatus;
//	UWORD xdata tookitMenuEntryAddr;
//	UWORD xdata toolkitMenuEntryIndexAddr;
//	UWORD xdata toolkitAppAddr;
//
//	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);
//
//	if (toolkitMenuEntryIndexAddr == FALSE)
//	{
//		return 0x9000;
//	}
//	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
//
//	toolkitAppAddr = SelectLFFile(DF_Toolkit_P, EF_ToolkitApp);
//
//	if(toolkitAppAddr == FALSE)
//	{
//		return 0x9000;
//	}
//
//	menuEntryNum = 0x00;
//
//	ReadTFBuf(toolkitMenuEntryIndexAddr,0,&menuEntryNum,1);
//
//	offset = 0x04; //响应数据格式为"EA"+len +"80"+len+value
//
//	for(i = 0x01; i <= menuEntryNum; i++)
//	{
//		ReadTFBuf(toolkitMenuEntryIndexAddr,i,&menuEntryIndex,0x01);
//
//		if ((menuEntryIndex&0x80) == 0x80) //本地菜单信息不显示
//		{
//			continue;
//		}
//
//		ReadLFBuf(tookitMenuEntryAddr,i,0x00,&tmp[0],sizeof(tmp));
//
//		if(tmp[0] == 0xFF)
//		{
//			continue;
//		}
//
//		appRecNum = tmp[AppRecNum_OFFSET]; //菜单有效，找到6F80对应记录
//
//		if(appRecNum == FALSE)
//		{
//			continue;
//		}
//
//		ReadLFBufForJavaAccess(toolkitAppAddr,appRecNum ,AppletStatus_OFFSET,&appstatus ,1);
//
//// 		if(appstatus != APPLET_UNLOCKED_SELECTABLE)  // 102226  8.2.1.6.1 Menu parameters    不管菜单状态和应用状态 都要显示
//// 		{
//// 			continue;
//// 		}
//
//		tempBuf[offset+1] = tmp[ItemIdentifier_OFFSET];  //item identifier
//
//		tempBuf[offset+2] = (tmp[MenuEntryStatus_OFFSET]&0x01)?0x01:0x00;//菜单是否enable，1表示enable
//
//		ReadLFBufForJavaAccess(toolkitAppAddr,appRecNum ,MaxEntryCount_OFFSET, tmp,sizeof(tmp));//读取6F80 中position和itemid
//
//		for(j = 0;j<tmp[0];j++)
//		{
//			if((tmp[1+2*j+1]) == tempBuf[offset+1])	//查找匹配的position
//			{
//				tempBuf[offset] = tmp[1+2*j];
//				break;
//			}
//		}
//
//		if(j == tmp[0])//未找到匹配的position
//		{
//			continue;
//		}
//
//		offset +=3;
//	}
//
//	tempBuf[0x03] = offset - 4;
//
//	tempBuf[0x02] = 0x80;
//
//	tempBuf[0x01] = tempBuf[0x03]+2;
//
//	tempBuf[0x00] = 0xEA;
//
//   	if(tempBuf[0x03] != 0x00)  //有菜单存在
//	{
//		if(((*currentLen)+tempBuf[0x01]+2) > G_CommandResponseLength)  //响应数据超过了0x100
//		{
//			return 0x6A84;
//		}
//
//		AddTLV(&RecData[0],&tempBuf[0]);
//
//		*currentLen += (tempBuf[0x01]+2);
//	}
//
//	return 0x9000;
//}

/*
*@funtion     :ToolkitRAM_GetData

*@description :
*
*@see         :TS 102 226,chapter 8(8.2.1.7)
*
*@param       :
*
*@return      :
*
*@author      :
*
*@history     :V1.0 : 删除该函数中的ReveriveData函数的调用，RAM中不需要执行收数据的操作@20180208
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others      :其他说明，如该函数使用的限制条件等
*/
WORD ToolkitRAM_GetData(void)
{
   BYTE idata recNum,len;
   BYTE idata offset;
   UWORD xdata toolkitAppAddr;

    if((P1 != 0xFF)||(P2 != 0x1F))
	{
	    return 0x00;
	}
	
	if(RecData[0] != 0x4F)
	{
		return 0x6A80;
	}
	
	if((CLA & 0x80) == 0x80)        //  GP 2.2.1  11.3.3.1 Data Field Returned in the Response Message  要去CLA最高位为1  返回TLV
	{
		offset = 0x03;
	}
	else                        //123048  A.1.1.2.1  仅要求返回V数据
	{
		offset = 0x00;
	}
	recNum = UICCToolkitFindSpecifiedAID(&RecData[2],RecData[1]);

	if(recNum==FALSE)   //Referenced data not found
	{
		return 0x6A88;	
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(recNum));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{
		return 0x6A88;
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,MaxEntryCount_OFFSET,&len,1); //读最大菜单数

	ReadLFBufForJavaAccess(toolkitAppAddr,recNum,MenuEntryPosition_OFFSET,&RecData[offset],len *2);

	if(offset != 0x00)
	{
		RecData[0] = P1;

		RecData[1] = P2;

		RecData[2] = 2*len;
	}

	G_CurrentRemoteResponseLgth	= 2*len + offset;

	return  0x6100+ G_CurrentRemoteResponseLgth;
}


/*
*@funtion     :NativeImpl_InstallForInstall

*@description :	该函数主要用来处理 install (for install)APDU命令时，将数据区域与ToolkitApp相关的参数缓存在指定的文件中
*
*@see         :TS 102 226,chapter 8(8.2.1.3.2)
*
*@param       :buf  （L（EF TLV的总长度） + EF LV + EA LV） 为了兼容平台层，第一个字节L是EF的TLV总长度  EA追加在后面                   //buffer：install (for install) 的 命令数据,包括5字节的APDU Header +APDU Data
*@param       :appId : 当前安装的应用的appId
*@param       :sdMode: 0x80 :当前安装的应用为安全域，else：当前安装的应用为普通应用
*
*@return      :0x9000/0x6A80/other SW status
*
*@author      :xuyz
*
*@history     :V1.0: create by xuyz @201310-23
*             :V1.1: modify by xuxiaojun@20180702 修改参数长度为81XX的情况：EUICC在电信研究院测试出来的BUG同步修改
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*           :modify by chentao@EPX产业化\INFENION_SLM97\Develop
*           ：modify by liguiyu@20221204,toolkitApplet方案修改
*
*@others      :1：暂时仅支持install (for install)为单条的情况
*/
WORD NativeImpl_InstallForInstall(BYTE * buf,WORD context, BYTE sdMode)
{
	BYTE idata executableModuleOffset,instanceAidOffset,instanceAidLen;
	BYTE idata InstallfieldOffset,InstallfieldLength;
	BYTE idata tagOffset,tagLength;
	BYTE idata CATagOffset,EATagOffset;
	BYTE idata tagFlag,i,j;
	WORD idata MaxInstallSequence;
	BYTE idata ToolkitAppSpecificParaOffset,ToolkitAppSpecificParaLgth;
	BYTE idata result,tarLength,installMode;
	BYTE idata apprecNum,appId;
	UWORD xdata toolkitAppAddr;
#if UICC__FUNCTION__UICCAPI
	BYTE idata EATagLgth,serviceLen;
#endif
	EATagOffset = 0;
	appId = (context&0xFF);
	
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif

#if UICC__FUNCTION__UICCAPI
	EATagLgth = FALSE;
	
	serviceLen = 0;

#endif	
	PXB_W_U8(PNtMEM(&bRamBuffer[SD_TAR_INFO_OFF]),ILLEGAL_APPID);//初始化 默认不带TAR  该函数后续需要添加对ISDP的TAR处理 处理后bRamBuffer[SD_TAR_INFO_OFF]赋值FF 避免注册时调用setSDTarInfo再次处理
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	G_ToolkitAppletInfoRef = REF_NULL;
	G_ToolkitAppIndex = ILLEGAL_APPID;
#endif
	G_SD_MenuInfoRef = REF_NULL;

	tagFlag = 0x00;

	tarLength = 0x00;

	installMode = APDUBuffer[2];

	ToolkitAppSpecificParaOffset = FALSE;

	ToolkitAppSpecificParaLgth = 0x00;

	executableModuleOffset =  APDUBuffer[5]+6;  //point to Length of Executable Module AID

	instanceAidOffset = executableModuleOffset + APDUBuffer[executableModuleOffset]+1;	  //point to Length of Application AID

	instanceAidLen = APDUBuffer[instanceAidOffset++];

	InstallfieldOffset = 0;

	InstallfieldLength = buf[InstallfieldOffset];//EF 总长度
	InstallfieldOffset = InstallfieldLength + 1;//指向EA len

	if(buf[InstallfieldOffset] != 0x00)//处理EA参数
	{
#if UICC__FUNCTION__UICCAPI
		tagFlag |= 0x01;

		EATagOffset = InstallfieldOffset;//指向EA len

		EATagLgth = buf[EATagOffset++];

		InstallfieldLength += (EATagLgth + 2);//参数总长度

		ToolkitAppSpecificParaOffset = FindTag(&buf[EATagOffset], 0x80,EATagLgth);   //查找tag 80

		if(ToolkitAppSpecificParaOffset != FALSE)
		{
			ToolkitAppSpecificParaOffset += EATagOffset;

			ToolkitAppSpecificParaLgth = buf[ToolkitAppSpecificParaOffset ++];
		}
		#else
		return 0x6A80;
		#endif
	}

	InstallfieldOffset = 0;

	tagOffset = 0;

	if(buf[InstallfieldOffset] != 0)//EF tag的总长度
	{		
		tagLength = buf[InstallfieldOffset];

		tagOffset ++;

		CATagOffset = FindTag(&buf[InstallfieldOffset+tagOffset], 0xCA,tagLength);   //查找tag CA

		if(CATagOffset != FALSE)
		{
			tagFlag |= 0x02;	

			ToolkitAppSpecificParaOffset =  InstallfieldOffset+tagOffset + CATagOffset; //指向 CA tag lgth在buf中的偏移

			ToolkitAppSpecificParaLgth = buf[ToolkitAppSpecificParaOffset++];
		}
	}

	if((tagFlag &0x03) == 0x03)  //CA & EA 同时存在
	{
		return 0x6A80;
	}

#if M2M_EUICC
	if((appId == ISDP_APPID)&&(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX)) //M2M ISD-P ，没有EA CA 后续直接把AID中指定字节当TAR ,有EA CA正常处理
	{
		return ProcessISDPTarInfo(buf,ToolkitAppSpecificParaOffset,ToolkitAppSpecificParaLgth,tagFlag);
	}
#endif

	if((tagFlag &0x03) == 0x00)	//CA & EA都不存在
	{
		return 0x9000;
	}
	
	if(sdMode == 0x80)  // 当前安装的应用是安全域，则若该安全域有支持CA或EA参数，则表明该安全域支持RAM APDU
	{	
		if(ToolkitAppSpecificParaOffset != FALSE)
		{	    
			InstallforInstall_StorSDMenuPara(&buf[ToolkitAppSpecificParaOffset], ToolkitAppSpecificParaLgth,context);//将SD的EA  CA参数写入文件，GP Getstatus  会获取SD的菜单信息
			
			return InstallforInstall_RAMSD(&buf[ToolkitAppSpecificParaOffset], ToolkitAppSpecificParaLgth,tagFlag,appId);  //处理安装sd里面存在的EA或CA参数
		}
		else
		{
			return 0x9000;  //没有配置参数，卡片正常返回
		}
	}
	/*else
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		//Toolkit applet is not allowed install under SE,by xwh 20200611
		if(G_CurExecutingProfileIndex == ILLEGAL_PROFILEINDEX)
		{
			return 0x6985;
		}
#endif
	}*/
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	apprecNum = UICCToolkitFindSpecifiedAID(NULL , 0x00);

	if(apprecNum == 0x00)
	{
#if UICC__FUNCTION__2B80_AT_FILEEND		
		//在EF_ToolkitApp 文件记录不足时，增加一条记录,by xwh 20210710
		apprecNum = AddOrDeleteToolkitAppRecord(0);

		if(apprecNum == 0x00)
#endif			
		{	
			return 0x6A80;
		}
	}
#endif
	ecpMem_Set(&tempBuf[0], 0x00, sizeof(tempBuf));

	SetBit(tempBuf[AppletStatus_OFFSET],APP_UNLOCK_BIT);

	if(installMode&0x08)	  //For make selectable
	{
		SetBit(tempBuf[AppletStatus_OFFSET],APP_SELECTABLE_BIT);
	}
	
	tempBuf[AppId_OFFSET] = appId;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel] = apprecNum;
#else
	G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel] = U16t8(nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex));//无实际意义，entry还没有创建 ，应用注册以后使用才有意义
#endif
	MaxInstallSequence = GetLastInstallSequence();

	ecpMem_Copy(&tempBuf[InstallSequence_OFFSET],(BYTE *)&MaxInstallSequence ,sizeof(WORD));

	if(ToolkitAppSpecificParaOffset != FALSE)  //当前存在CA tag 或 80 tag
	{
		if((tagFlag&0x03) == 0x02)  // 处理CA tag
		{			
			tempBuf[MFAccessDomainLength_OFFSET] =  buf[ToolkitAppSpecificParaOffset ++];	 //Length of Access Domain field
	
			ecpMem_Copy(&tempBuf[MFAccessDomainContent_OFFSET],&buf[ToolkitAppSpecificParaOffset],tempBuf[MFAccessDomainLength_OFFSET]);
	
			ToolkitAppSpecificParaOffset += tempBuf[MFAccessDomainLength_OFFSET];  //指向优先级 

			ToolkitAppSpecificParaLgth -= (tempBuf[MFAccessDomainLength_OFFSET] + 1);  //CA tag中除AccessDomain外的剩余数据长度
		}

	     if(ToolkitAppSpecificParaLgth < 4)  //参数： Priority level of the Toolkit application instance、  Maximum number of timers allowed for this application instance、Maximum text length for a menu entry、Maximum number of menu entries allowed for this application必须存在instance
		 {
			 return 0x6A80;	
		 }
		tempBuf[AppPriority_OFFSET] = buf[ToolkitAppSpecificParaOffset];

		if(tempBuf[AppPriority_OFFSET] == 0x00)  //app优先级无效,       移动 电信5G 都期望可以成功安装应用，暂时统一处理（同TPX）
		{
//			 if(JcvmConfig_GetOPerator() == OPERATOR_CHINAMOBILE)
//			 {
				 tempBuf[AppPriority_OFFSET] = 0xFF; //E:\N3_TEE\develop\others\test\移动NFC2.0送检工程集20151113\MultiChannel_20150330_明文版\多通道并发处理机制测试_工程集.uce\东信\13m 中 卡片初始化，下载安装java1应用时，其优先级为00，期望安装成功,将其设置成FF 最低优先级
//			 }
//             else
//             {
//				return 0x6A80;
//             }
		}

		ToolkitAppSpecificParaOffset ++ ; // point to Maximum number of timers allowed for this application instance

		ToolkitAppSpecificParaLgth --;

		tempBuf[MaxTimerCount_OFFSET] = buf[ToolkitAppSpecificParaOffset];

		if(tempBuf[MaxTimerCount_OFFSET] > 0x08)
		{
			return 0x6A80;		
		}

		ToolkitAppSpecificParaOffset ++; // point to Maximum text length for a menu entry

		ToolkitAppSpecificParaLgth --;

		tempBuf[MaxEntryTextLen_OFFSET] = buf[ToolkitAppSpecificParaOffset];

		ToolkitAppSpecificParaOffset ++; //point to Maximum number of menu entries allowed for this applet instance

		ToolkitAppSpecificParaLgth --;

        tempBuf[MaxEntryCount_OFFSET] = buf[ToolkitAppSpecificParaOffset];
		
		if(ToolkitAppSpecificParaLgth < (tempBuf[MaxEntryCount_OFFSET] * 2 + 1))  //剩余数据长度不够
		{
			return 0x6A80;
		}

		ecpMem_Copy(&tempBuf[MenuEntryPosition_OFFSET],&buf[ToolkitAppSpecificParaOffset+1],2*tempBuf[MaxEntryCount_OFFSET]);

		for(i = 0 ;i < tempBuf[MaxEntryCount_OFFSET]; i ++)					//检查是否存在非法的menuId
		{
			if (tempBuf[MenuEntryPosition_OFFSET+2*i+1] >= 0x80)
			{
				return 0x6A80;	
			}
			
			if(tempBuf[MenuEntryPosition_OFFSET+2*i+1] != 0x00)	  //判断当前menuId是否已经被分配 脚本Cre_Tin_Itid会测试在安装应用2时会出现其menuId与APP1的menuId重复,安装失败
			{
				if(ToolkitFindSpecifiedMenuIdForInstall(tempBuf[MenuEntryPosition_OFFSET+2*i+1]) != FALSE)	 
				{
					return 0x6A80;	
				}
			}
		}

		for(i = 0 ;i < tempBuf[MaxEntryCount_OFFSET]; i ++)		//检查当前是否有重复的非0 的menuId
		{
			for(j = i+1; j <tempBuf[MaxEntryCount_OFFSET] ; j++)
			{
				if(tempBuf[MenuEntryPosition_OFFSET+2*i+1] != 0x00)
				{
					if(tempBuf[MenuEntryPosition_OFFSET+2*i+1] ==tempBuf[MenuEntryPosition_OFFSET+2*j+1] )
					{
						return 0x6A80;
					}
				}
				else
				{
					break;
				}
			}
		}
	
		if(ToolkitAppSpecificParaLgth-(2*tempBuf[MaxEntryCount_OFFSET]+1)>0) //23048规范和SIMAPI测试脚本中安装参数最大通道数可选
		{
			ToolkitAppSpecificParaOffset += 2*tempBuf[MaxEntryCount_OFFSET]+1 ;

			ToolkitAppSpecificParaLgth -= (2*tempBuf[MaxEntryCount_OFFSET]+1);

			tempBuf[MaxChannelCount_OFFSET] = buf[ToolkitAppSpecificParaOffset];

			if(tempBuf[MaxChannelCount_OFFSET] > 0x07)
			{
				return 0x6A80;
			}

			ToolkitAppSpecificParaOffset ++;
			
			ToolkitAppSpecificParaLgth --;

			if(ToolkitAppSpecificParaLgth  > 0x00)//23048规范和SIMAPI测试脚本中安装参数MSL可选
			{
				tempBuf[MSLLen_OFFSET]  = buf[ToolkitAppSpecificParaOffset];
	
				ecpMem_Copy(&tempBuf[MSLCoding_OFFSET], &buf[ToolkitAppSpecificParaOffset+1],tempBuf[MSLLen_OFFSET]);
				
				ToolkitAppSpecificParaLgth -= (1+tempBuf[MSLLen_OFFSET]);  //包括tar和剩余数据的长度

				if(ToolkitAppSpecificParaLgth  > 0x00)//23048规范和SIMAPI测试脚本中安装参数TAR可选
				{
					ToolkitAppSpecificParaOffset += (tempBuf[MSLLen_OFFSET]  + 1);  //point to Length of TAR Value(s) field
	
					tempBuf[TARLength_OFFSET] =  buf[ToolkitAppSpecificParaOffset];
					
					tarLength =  buf[ToolkitAppSpecificParaOffset];
	
					if((tempBuf[TARLength_OFFSET]%0x03) != 0x00) //TAR length 不是3的整数倍
					{
						return 0x6A80;
					}	

					if(ToolkitAppSpecificParaLgth < (tarLength + 1))   //剩余数据不够包括tar的全部数据，报错
					{
						return 0x6A80; 
					}
#if UICC__FUNCTION__UICCAPI
                    serviceLen = ToolkitAppSpecificParaLgth - tarLength -1; 
#endif
					
				}
			}
			else  //If the Maximum number of channels field is included in the command data then the Length of Minimum Security Level field shall also be included.
			{
				return 0x6A80; 
			}
		}
		else
		{
			tempBuf[MaxChannelCount_OFFSET] = 0x07;
		}

		if(tempBuf[TARLength_OFFSET] == 0x00)  //TS 102226 8.2.1.3.2.7 Coding of TAR Value(s) field
		{	//E:\N3_TEE\develop\others\test\中国联通NFC-SWP测试工程集测试脚本20150818\中国联通NFC-SWP测试工程集测试脚本20150817\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\ETST_102_Test_工程集.uce\TAR_repetition_01_01
			if(instanceAidLen >= 0x0F)	  //If the length of TAR Value(s) is zero, the TAR may be taken out of the AID if any. 实例AID的长度为15字节时，13-15表示TAR
			{
				tempBuf[TARLength_OFFSET] = 0x03;

				ecpMem_Copy(&tempBuf[TARValues_OFFSET], &APDUBuffer[instanceAidOffset+12],tempBuf[TARLength_OFFSET]);  //AID 的第13 字节 - 第15字节为TAR
			}
		}
		else
		{
			ecpMem_Copy(&tempBuf[TARValues_OFFSET], &buf[ToolkitAppSpecificParaOffset+1],tempBuf[TARLength_OFFSET]);
		}
	
		if(UICCNativeImpl_RAMIsValidForTARs(&tempBuf[TARValues_OFFSET], tempBuf[TARLength_OFFSET]) == FALSE)  //判断tars是否已经分配过
		{
			return 0x6A80;
		}

		if((tagFlag&0x03) == 0x02)  //SIMAPI   处理CA tag ，写入文件
		{
			tempBuf[TrigerMode_OFFSET] = SIMAPI_Mode;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			G_ToolkitAppletInfoRef =  AddEFToolkitAppInfo(context,tempBuf,ToolkitApp_Info_Size);
			if(G_ToolkitAppletInfoRef == REF_NULL)
			{
				return 0x6A84;
			}
			G_ToolkitAppIndex = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];
#else
			//G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] = SIMAPI_Mode;//在initMenuentry中可能会用到该变量，需要在此处赋值 20130304 lj
	
			result = WriteLFBufForJavaAccess(toolkitAppAddr,apprecNum,0x00,tempBuf,GetLFRecNumLgth(toolkitAppAddr, FileLgth));
	
			if(result == FALSE)  //写flash失败
			{
				return 0x6581;
			}		
#endif
			return 0x9000;

		}
		
#if UICC__FUNCTION__UICCAPI
		if(serviceLen > 0)  //存在Maximum number of services for this application instance
		{	

			ToolkitAppSpecificParaOffset += (tarLength  + 1); // point to Maximum number of services for this application instance

			tempBuf[MaxServiceCount_OFFSET] =  buf[ToolkitAppSpecificParaOffset];
			
			if(tempBuf[MaxServiceCount_OFFSET] > 0x08)   //The total number of services allocated for all the applets shall not exceed 8.
			{
				return 0x6A80;
			}	
		}
		#endif
			
	}

#if UICC__FUNCTION__UICCAPI
	if(UICCNativeImpl_ProcessRAMAccessDomain(&buf[EATagOffset],EATagLgth,0x81) == FALSE)  //查找并处理81tag
	{
		return 0x6A80;
	}

	if(UICCNativeImpl_ProcessRAMAccessDomain(&buf[EATagOffset],EATagLgth,0x82) == FALSE)  //查找并处理82tag
	{
		return 0x6A80;
	}

	tempBuf[TrigerMode_OFFSET] = UICCAPI_Mode;

	//G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] = UICCAPI_Mode;//在initMenuentry中可能会用到该变量，需要在此处赋值 20130304 lj
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	G_ToolkitAppletInfoRef =  AddEFToolkitAppInfo(context,tempBuf,ToolkitApp_Info_Size);

	if(G_ToolkitAppletInfoRef == REF_NULL)
	{
		return 0x6A84;
	}
	G_ToolkitAppIndex = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];
#else
	result = WriteLFBufForJavaAccess(toolkitAppAddr,apprecNum,0x00,tempBuf,GetLFRecNumLgth(toolkitAppAddr, FileLgth));

	if(result == FALSE)  //写flash失败
	{
		return 0x6581;
	}
#endif

#endif
	return 0x9000;	
}
/*
*@funtion    :FindMenuInfoAddrForSD 
*
*@description:在2800/6F94下找一条指定记录  
*
*@param      :appId:应用的appId   ,输入指定appID,返回该appId占用的记录所在地址 ，ILLEGAL_APPID表示找一条空记录并返回地址 
*
*@return     :FileAddr
*
*@author     :lgy
*
*@history    :V1.0 :copy from TPX3.0 and modify by xuxiaojun@20190805
*/
//UWORD FindMenuInfoAddrForSD(BYTE appId,BYTE* recNo)
//{
//    UWORD xdata sdMenuAddr;
//
//    BYTE idata recNum,i,tmp;
//
//	sdMenuAddr = SelectLFFile(DF_Toolkit_P,EF_SSDMenuInfo);
//
//    if(sdMenuAddr ==  FALSE)
//    {
//        return ADDR_IS_NULL;
//    }
//
//    recNum = GetLFRecNumLgth(sdMenuAddr,FileNum);
//
//    for(i = 0; i < recNum; i++)//找一条空纪录 或者指定记录
//    {
//        ReadLFBuf(sdMenuAddr,i+1,0x00,&tmp,sizeof(BYTE));
//
//        if(tmp == appId)
//        {
//            *recNo = i + 1;
//            return sdMenuAddr;
//        }
//    }
//
//    *recNo = FALSE;
//
//    return  ADDR_IS_NULL;
//}
/*
*@funtion    :InstallforInstall_StorSDMenuPara 
*
*@description:该函数主要用于处理安装sd时存在EA参数的情况，存储菜单信息 GETSTATUS时返回 
*
*@param      :BYTE *paraFiled   参考102226   8.2.1.3.2.2.1 UICC Toolkit Application specific parameters field
*@param      :BYTE paraLen
*@param      :appId:SD的appId
*
*@return     :void
*
*@author     :lgy
*
*@history    :V1.0 :copy from TPX3.0 by xuxiaojun@20190805
*@history    ：V1.1 ：modify by lgy@20220720,SDMenuPara放在SecurityDomainData/SD_Menu_Info，不放文件
*/
void InstallforInstall_StorSDMenuPara(BYTE *paraFiled, BYTE paraLen,WORD context)
{
    UWORD  X_A  sdMenuAddr;
    BYTE  X_A   recNum,appId;
    WORD  X_A   backCC;

    appId = (context&0xFF);
    
 /*   sdMenuAddr = FindMenuInfoAddrForSD(ILLEGAL_APPID,&recNum);
    
    if(sdMenuAddr ==  ADDR_IS_NULL)
    {
        return;
    }
*/
    if((paraLen < 4)||(paraFiled[3] == 0)||(paraFiled[3] > 10))//菜单个数为0 或超过10个 忽略    ,(paraLen < 4)限制条件是为了保证paraFiled[3]不越界，该值存的是菜单个数
    {
        return ;
    }
    
    backCC = cc;
    cc = context;

    G_SD_MenuInfoRef =	_newarrayObject(AType_Byte, SD_MENU_INFO_LEN, BIT_NEED_REFRESH);
    cc = backCC;

    sdMenuAddr    =     get_array_data(MEMtPF8(locateObjectInMemory(G_SD_MenuInfoRef)));

    if(sdMenuAddr ==  ADDR_IS_NULL)
    {
        return ;
    }
    
    ecpMem_Set(&tempBuf[0],0x00,22);
    
    tempBuf[0] = appId;
    
    tempBuf[1] = paraFiled[3];
    
    ecpMem_Copy(&tempBuf[2],&paraFiled[4],paraFiled[3]*2);
    
    WriteEepromAtomic(sdMenuAddr,&tempBuf[0],SD_MENU_INFO_LEN);	 // WriteLFBuf(sdMenuAddr,recNum,0x00,&tempBuf[0], GetLFRecNumLgth(sdMenuAddr,FileLgth));
}
/*
*@funtion    :GetStatusSDMenuInfo 
*
*@description:该函数主要用于获取SSD的menuInfoGETSTATUS指令EA tag ，SD的EA参数存放于文件DF_Toolkit/EF_SSDMenuInfo 
*
*             参考102226   8.2.1.3.2.2.1 UICC Toolkit Application specific parameters field

*@param      :appId:SD的appId
*@param      :BYTE* outBuf
*
*@return     :BYTE InfoLen  返回拼装的EA tag总长度
*
*@author     :lgy
*
*@history    :V1.0 :copy from TPX3.0 by xuxiaojun@20190805
*@history    :V1.1:modify by liguiyu@20221204，toolkitApplet的信息放在应用中
*/
void GetStatusSDMenuInfo(void)
{
	UWORD xdata sdDataObj,sdMenuInfoAddr;
	BYTE idata recNo,offset,len,i;
	BYTE idata appId;
	UWORD xdata arrayAddr;
	
	WORD idata bufferPtr,arraryRef,SDMenuInfoRef;
#define   MENU_NUM_OFFSET    0x01
	bufferPtr  = PopOperandStackShort();	//从操作数栈中pop出参数：目标数组偏移
	arraryRef  = PopOperandStackRef();  	//从操作数栈中pop出参数：目标数组
	appId = PopOperandStackRef();

	offset = 0;

	sdDataObj = sdData_getsdDataObjByAppId(appId);

	SDMenuInfoRef = memReadU16(MEMtPM8(get_object_field_ref(sdDataObj, SD_Menu_Info_INDEX)));

//	sdMenuInfoAddr = get_array_data(MEMtPN8(locateObjectInMemory(SDMenuInfoRef)));//FindMenuInfoAddrForSD(appId,&recNo);
					
	if(SDMenuInfoRef == REF_NULL)//if((sdMenuInfoAddr == FALSE)||(recNo == FALSE))//安装时不带EA参数
	{
//		tempBuf[offset++] = 0xEA; //Append5CTagListContext函数中已经拼装了EA
		tempBuf[offset++] = 0x02;
		tempBuf[offset++] = 0x80;
		tempBuf[offset++] = 0x00;

	}
	else
	{
		sdMenuInfoAddr = get_array_data(MEMtPN8(locateObjectInMemory(SDMenuInfoRef)));

		len = memReadU16(MEMtPN8(sdMenuInfoAddr + MENU_NUM_OFFSET));//ReadLFBuf(sdMenuInfoAddr,recNo,MENU_NUM_OFFSET,&len,sizeof(BYTE));//获取菜单个数 存在len变量中   菜单个数为0时不会存到文件 不会走到该流程
		
//		tempBuf[offset++] = 0xEA;//Append5CTagListContext函数中已经拼装了EA
		
		tempBuf[offset++] = len*3 + 2;
		tempBuf[offset++] = 0x80;
		tempBuf[offset++] = len*3;
		
		for(i = 0;i < len ;i++)
		{
			ecpMem_Copy(&tempBuf[offset],MEMtPN8(sdMenuInfoAddr + 2 + i*2),2);//ReadLFBuf(sdMenuInfoAddr,recNo,2 + i*2,&tempBuf[offset],2);
			
			offset += 2;
			
			tempBuf[offset++] = 0x00; //MENU STATUS
		}
	}
	
	arrayAddr = (memref)(MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(arraryRef))))  + bufferPtr);//获取apduBuf数组对象头起始地址

	VMWriteBuf(arrayAddr,&tempBuf[0],offset);

	PushOperandStackShort(offset);
}
/*
*@funtion    :ToolKitGetAppMenuStatusByRecNo 
*
*@description:在EF_TookitEntry中查找 EF_ToolkitApp文件中第toolkitAppRecNo条记录表示的APP的菜单信息 
			  该函数查找的是EA/CA参数中Menu Identifier为0的菜单信息 ，zeroMenuIdNum表示第几个Menu Identifier为0的菜单
*             Menu Identifier 为0时 会在InitMenuEntry时分配menuId, 分配的ID范围在80~FF ，原则上按从小到大的顺序分配	
*
*             参考102226   8.2.1.3.2.2.1 UICC Toolkit Application specific parameters field

*@param      :BYTE  zeroMenuIdNum  ,需要查找的第几个id为0的菜单
*@param      :BYTE  toolkitAppRecNo ，6F80的记录号 --待查找的菜单所在的记录号,6F80 ToolkitAPP改成HEAP方案以后，传APPID作为参数
*
*@return     :BYTE MenuStatus  菜单状态 0/实际状态
*
*@author     :lgy
*
*@history    :V1.0 :copy from TPX3.0 by xuxiaojun@20190805
*@history    modify by lgy@20220719 6F80 ToolkitAPP改成HEAP方案以后，传APPID作为参数
*/
BYTE ToolKitGetAppMenuStatusByRecNo(BYTE zeroMenuIdNum,  BYTE toolkitAppRecNo)
{
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata zeroIdNo,i,tookitMenuEntryRecNum,tmp[3];
	
	zeroIdNo = 1;
	
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	if(tookitMenuEntryAddr == ADDR_IS_NULL)
	{
		return 0;
	}
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);
	
	for(i=1;i<tookitMenuEntryRecNum;i++)
	{
		ReadLFBuf(tookitMenuEntryAddr, i , MenuEntryStatus_OFFSET, tmp,sizeof(tmp));
		
		if(tmp[ItemIdentifier_OFFSET] <= 0x7F) //非0菜单 由下行参数指定的id
		{
			continue;
		}
		
		if(tmp[AppRecNum_OFFSET] == toolkitAppRecNo)//cos为toolkitAppRecNo表示的应用分配的菜单id
		{
			if(zeroIdNo == zeroMenuIdNum)
			{
				return (tmp[MenuEntryStatus_OFFSET]&0x01)?0x01:0x00;
			}
			
			zeroIdNo ++ ;
		}
	}
	
	return 0;
}

/*
*@funtion    :GetStatusToolkitAppMenuInfo 
*
*@description:该函数主要用于获取ToolkitApp的menuInfoGETSTATUS指令EA tag 
*
*             参考102226   8.2.1.3.2.2.1 UICC Toolkit Application specific parameters field

*@param      :appId:ToolkitApp的appId
*@param      :BYTE* outBuf
*
*@return     :BYTE InfoLen  返回拼装的EA tag总长度
*
*@author     :lgy
*
*@history    :V1.0 :copy from TPX3.0 by xuxiaojun@20190805
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void GetStatusToolkitAppMenuInfo(void)
{
	BYTE idata i,offset,zeroMenuIdNum,toolkitAppRecNo,MenuStatus;
	BYTE xdata buf[20];
	BYTE idata menuEntryNum,menuEntryIndex;
	UWORD xdata tookitMenuEntryAddr;
	UWORD xdata toolkitAppAddr;			

	BYTE idata appId;
	UWORD xdata arrayAddr;

	WORD idata bufferPtr,arraryRef;

	bufferPtr  = PopOperandStackShort();	//从操作数栈中pop出参数：目标数组偏移
	arraryRef  = PopOperandStackRef();  	//从操作数栈中pop出参数：目标数组
	appId = PopOperandStackRef();

    buf[0] = GetAIDDataByAppID(appId,&buf[1]);
    
    toolkitAppRecNo = UICCToolkitFindSpecifiedAID(&buf[1],buf[0]); //找到appId 在6F80的记录号 
    
    offset = 0;

    arrayAddr = (memref)MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(arraryRef))))  + bufferPtr;//获取apduBuf数组对象头起始地址

    if(toolkitAppRecNo == FALSE)	//	安装应用时没有带EA /CA参数
    {
//    	tempBuf[offset++] = 0xEA;
    	tempBuf[offset++] = 0x02;
    	tempBuf[offset++] = 0x80;
    	tempBuf[offset++] = 0x00;
    }
    else
    {
		tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(toolkitAppRecNo));
#else
		toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
		menuEntryNum = 0x00;
	
		ReadLFBufForJavaAccess(toolkitAppAddr,toolkitAppRecNo,MaxEntryCount_OFFSET,&menuEntryNum,1);//获取这个应用的总菜单个数

		ReadLFBufForJavaAccess(toolkitAppAddr,toolkitAppRecNo,MenuEntryPosition_OFFSET,&buf[0],menuEntryNum*2);//每个应用 最大支持10个菜单 将该应用所有菜单的Position和identify信息读出

		zeroMenuIdNum = 1;

		offset = 0x03;

		for(i = 0; i < menuEntryNum; i++)
		{
			tempBuf[offset++] = buf[i*2];
			tempBuf[offset++] = buf[i*2 + 1];

			if((buf[i*2 + 1]) == 0)//menuId 为0
			{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
				MenuStatus = ToolKitGetAppMenuStatusByRecNo(zeroMenuIdNum,  appId);//找appId所表示的Toolkit应用 第zeroMenuIdNum个menuId为0的菜单信息，这些菜单被分配的id原则上是从小到大分配的，值为80~FF之间
#else
				MenuStatus = ToolKitGetAppMenuStatusByRecNo(zeroMenuIdNum,  toolkitAppRecNo);//找toolkitAppRecNo记录中 第zeroMenuIdNum个menuId为0的菜单信息，这些菜单被分配的id原则上是从小到大分配的，值为80~FF之间
#endif
				zeroMenuIdNum++;
			}
			else//在6F81找到对应的记录号 （如果EA/CA中已经为应用分配了menuId ，值一定在1-7F范围）
			{
				menuEntryIndex = ToolkitFindSpecifiedMenuId(buf[i*2+1]);//根据menuId 返回6F81的记录号

				if(menuEntryIndex != FALSE)
				{
					ReadLFBuf(tookitMenuEntryAddr,menuEntryIndex,MenuEntryStatus_OFFSET,&MenuStatus,1);//获取菜单状态
                    //电信5G 项目      ， 中国电信(国密项目)4G NFC卡备案基本通信功能测试/7.Java功能测试脚本/RAM测试/144 RAM_getstatus_1621_2
					MenuStatus = (MenuStatus&0x01)?0x01:0x00;//菜单是否enable，1表示enable; 统一一种方式处理

				}
				else
				{
					MenuStatus = 0;
				}
			}
			tempBuf[offset++] = MenuStatus;
		}

		tempBuf[0x02] = offset - 3;
		tempBuf[0x01] = 0x80;
		tempBuf[0x00] = tempBuf[0x02]+2;
//		tempBuf[0x00] = 0xEA;
    }

    VMWriteBuf(arrayAddr,&tempBuf[0],offset);

	PushOperandStackShort(offset);

}

/*
*@funtion     :NativeImpl_SetStatus

*@description :set status中调用该函数设置6F80/6F81状态
*
*@see         :TS 102 226,chapter 8(8.2.1.2)
*
*@param       :BYTE *apdu
*
*@return      :
*
*@author      :
*
*@history     :V1.1 :create by yjr@20210402 参考102241 V16 void requestPollInterval(short duration)
*																	throws ToolkitException,
*																		TransactionException
*应用在申请PollInterval时，如果申请的值不是0，应该返duration的最小值，如果申请的值是0，需要和当前其他应用申请的最小值比较，在决定发polloff或者pollinterval，
*测试用例uicc_configuration_additional_test_工程集/GP补充测试/GP TEST/GP 补充Multi_app_requestPollInterval_Test01
*
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能

*
*@others      :其他说明，如该函数使用的限制条件等
*             1：注意，锁住某应用时，如果当前应用之前申请了duration，则发送poll off主动式命令；解锁时，则发送poll interval主动式命令 ,测试用例Cre_Pcs_Spco
*             2：MAC是否为4字节？？？
*/
WORD NativeImplSetStatus(BYTE appId)
{
//	BYTE idata aidLen;
	BYTE idata appstatus,entrystatus;
	BYTE idata ApprecNum,entryApprec,recNum,result,num,flag;
	BYTE idata eventListTag,j;
	BYTE xdata buf[0x11];
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum;
	UWORD xdata toolkitAppAddr;
	WORD xdata sysduration,appDuration,minDuration;
	BYTE* apdu;

	apdu = APDUBuffer;

	if((apdu[2]&0xC0) != 0x40)   //P1 :statusType&0xC0 = 0x40, Indicate Application
	{
		return 0x9000;
	}
	/*
	aidLen = apdu[4];   //当CLASS为84 时，带mac；为80时，数据域支包括AID

	ApprecNum = UICCToolkitFindSpecifiedAID(&apdu[5],aidLen);
*/
	ApprecNum = GetToolkitEntryAppNumByAppId(appId);

	if(ApprecNum == FALSE)	 //当前应用不是Toolkit App
	{
		return 0x9000;
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(ApprecNum));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	ReadLFBufForJavaAccess(toolkitAppAddr,ApprecNum,AppletStatus_OFFSET,&appstatus,0x01);

	if(ValueBit(appstatus, APP_SELECTABLE_BIT) == FALSE)
	{
		return 0x6985;//应用没有selectable 禁止操作。
	}

	if((apdu[3]&0x80) == 0x80)  //LOCKED state
	{
		 ClearBit(appstatus,APP_UNLOCK_BIT);      		 //appstatus  =  APPLET_LOCKED;

		 entrystatus = Menu_LOCKED|0x7F;	    //= 0x7F
	}
	else
	{

		 SetBit(appstatus,APP_UNLOCK_BIT);                  //appstatus =   APPLET_UNLOCKED;

	     entrystatus = Menu_UNLOCKED|0x7F;	   // = 0xFF
	}

	/*result = */WriteLFBufForJavaAccess(toolkitAppAddr,ApprecNum,AppletStatus_OFFSET,&appstatus,sizeof(BYTE));
/*
	if(result == FALSE)
	{
		return FALSE;      	//抛出事物异常
	}
*/
	eventListTag = 0x00;

	ReadLFBufForJavaAccess(toolkitAppAddr,ApprecNum,EventList_OFFSET,buf,EventList_Length);

	for(j = EVENT_EVENT_DOWNLOAD_MT_CALL_bit; j<= EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit; j++)
	{
		if((j == EVENT_STATUS_COMMAND_bit)||(j == 24)||(j == 30)||(j == 31)||(j == 33)) //24 :EVENT_FORMATTED_SMS_CB_bit //30 31 33  reserved by 3GPP
		{
			continue;
		}
		if(ToolkitGetEventBit(&buf[0], j))
		{
			eventListTag = TRUE;
			break;
		}
	}


	if(ToolkitGetEventBit(&buf[0], EVENT_STATUS_COMMAND_bit))
	{
		ReadLFBufForJavaAccess(toolkitAppAddr,ApprecNum,PollDuration_OFFSET,(BYTE *)&appDuration, sizeof(WORD));
		
		if(ValueBit(appstatus,APP_UNLOCK_BIT)==0)
		{
		   	//SetG_ToolkitSystemProactiveFlag(Bit_PollOff);
			minDuration = FindMinDuration(ApprecNum); 
			
			if(appDuration < minDuration)
			{
				if(minDuration == (MAXVALUE_POLL_DURATION + 1))
				{
					SetG_ToolkitSystemProactiveFlag(Bit_PollOff);
					
					appDuration = POLL_NO_DURATION;
					
					VMWriteBuf(E_CUR_DURATION,(BYTE*)&appDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 
				}
				else
				{
					SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);

					SetBit(G_PollIntervalRecNum,(ApprecNum-1));

					VMWriteBuf(E_CUR_DURATION,(BYTE*)&minDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取 						
				}
				
			}
			
			
			
		}
		else
		{	
		   	//SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);
			
			//SetBit(G_PollIntervalRecNum,(ApprecNum-1));
			
			VMReadBuf((BYTE*)&sysduration,E_CUR_DURATION,sizeof(WORD));

			if((appDuration < sysduration) ||(sysduration == POLL_NO_DURATION))
			{
				SetG_ToolkitSystemProactiveFlag(Bit_PollInterval);

				SetBit(G_PollIntervalRecNum,(ApprecNum-1));

				VMWriteBuf(E_CUR_DURATION,(BYTE*)&appDuration,sizeof(WORD)); //更改系统的 -Pull Interval 时间间隔 发送主动性命令也是从这个系统配置获取
			}
		}
	}

	num = 0x00;

	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);

	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);


   	for(recNum = 1; recNum<=  tookitMenuEntryRecNum;  recNum++)
	{
   		ReadLFBuf(tookitMenuEntryAddr,recNum,MenuEntryStatus_OFFSET,&flag,sizeof(BYTE));

		if(flag == 0xFF)
		{
			continue;
		}

		ReadLFBuf(tookitMenuEntryAddr,recNum,AppRecNum_OFFSET,&entryApprec,1);
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		if(entryApprec == appId)
#else
		if(entryApprec == ApprecNum)
#endif
		{
			if((flag & 0x80)!= (entrystatus & 0x80))                    //有菜单状态被改变是要做setupmenu
			{
				flag = (entrystatus&0x80)|(flag&0x7F);

				result = WriteLFBuf(tookitMenuEntryAddr,recNum,MenuEntryStatus_OFFSET,&flag,sizeof(BYTE));

				if(result == FALSE)
				{
					return FALSE;     //抛出事物异常
				}
				num ++;
			}
		}
	}

	if(num > 0)
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);
	}

	if(eventListTag == TRUE)
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpEventList);
	}

	return 0x9000;
}

/*
*@function   :NativeImpl_EndOfRegister
*
*@description: 在注册成功之后，如果当前是toolkit应用，则判断其实现了sim.toolkit.ToolkitInterface还是uicc.toolkit.ToolkitInterface，而不是根据CA EA 参数判断当前是SIM API 模式还是UICC API 模式，以提高兼容性
*
*@param      : BYTE appId
*@param       :WORD appletRef
*
*@return     :void
*
*@history    :关联测试用例 ： 中国移动NFC2.0项目20160415/GP2.2.1_UICC_ComplianceTests(移动版)-20141112/uicc_configuration_additional_test/GpBugListAdditionalTest_SECURED/BugListAdditional_09_01
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void NativeImpl_EndOfRegister()
{
	BYTE appId;
	WORD appletRef;

	BYTE idata tmp[2],i;
	UWORD xdata addr;
	BYTE idata trigeMode;
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum;

	appletRef = PopOperandStackShort();
	appId = (BYTE)PopOperandStackShort();

#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)   //文件系统未创建时，执行此函数无意义,如初始化isd会进入到该函数中
	{
		return ;
	}

	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(i=0; i< toolkitAppRecNum; i++)
	{
		ReadLFBufForJavaAccess(toolkitAppAddr, i+1 , AppletStatus_OFFSET, tmp,sizeof(tmp));

		if(tmp[0] == 0xFF)
		{
			continue;
		}

		if(tmp[1] == appId)
		{
			break;
		}
	}

	if(i == toolkitAppRecNum)
	{
		return ;  //当前安装的应用不是toolkit应用，则直接返回
	}
#else
	appId = getIndexByID(appId);

	toolkitAppAddr  =  GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(appId));

	if(toolkitAppAddr == ADDR_IS_NULL)
	{
		return ;//当前安装的应用不是toolkit应用，则直接返回
	}
#endif
	ReadLFBufForJavaAccess(toolkitAppAddr, i+1 , TrigerMode_OFFSET, &trigeMode,sizeof(BYTE));  //还是根据CA 或EA 来判断

	addr = GetObjectHeaderAddr(appletRef);  //要注意如果应用既没有实现SIMToolkit Interface 也没有实现UICCToolkit Interface,此处应该清除所有的与Toolkit安装的信息的，

	if(isinstanceof(MEMtPN8(addr), AType_Object,MEMtPF8(resolveReferenceAddress(SIMToolkitInterface_Interface_Addr, SIM_TOOLKIT_PKG_ID, COMPONENT_CLASS))) == TRUE)
	{
		trigeMode = SIMAPI_Mode;  //在注意在应用select成功之后，可能也会根据SIM API模式还是UICC API 模式，进行通道信息初始化
	}

	else if(isinstanceof(MEMtPN8(addr), AType_Object, MEMtPN8(resolveReferenceAddress(UICCToolkitInterface_Interface_Addr, UICC_TOOLKIT_PKG_ID, COMPONENT_CLASS))) == TRUE)
	{
		trigeMode = UICCAPI_Mode;
	}

	else
	{
		//保留原值不变
	}

	SetLFConstForJavaAccess(toolkitAppAddr, i+1 , TrigerMode_OFFSET, trigeMode,sizeof(BYTE));

    return ;
}
#endif
