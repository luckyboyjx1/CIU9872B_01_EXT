/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   ProfileInterpreter.c
*
* Description: 该文件主要是PE解析的入口
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/

#include "PETemplateFileProcess.h"
#include  "typedef.h"
#include  "fileAction.h"
#include  "gsmpublic.h"
#include  "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"
#include "fileSystem.h"
#include "Interface_LTEFromPlatform.h"
#include "Interface_PlatformFromLTE.h"
#include "stkhead.h"
#include "STKFileView.h"
#include "VMAccessEeprom.h"
#include "SEEUICCInterface.h"
#include "ToolkitRFM.h"

#if (EUICC_ONLY||SE_AND_EUICC)

//PE 全局TAG


//BYTE    xdata G_PEIndex,G_PETag;//当前PE的索引，当前PE的tag

PEControlContext xdata G_PEControlContext;
ProfileControlContext xdata  G_profileControlContext;
FileControlContext xdata G_fileControlContext;


TlvInfo xdata G_PETlvInfo;//解析PE的信息
WORD  xdata G_TempLen;//tempBuf的数据长度

#if RSP_EUICC
BYTE xdata G_ProfileErrorReason;
WORD xdata G_USIM_identification,G_CSIM_identification; // G_Tmp_identification add by xieyong 记录拼警告的PE ID
#endif

WORD xdata G_Tmp_identification;

BYTE code G_Common_Template_ID[] = {0x67,0x81,0x0F,0x01,0x02};
//Identification number of PE

//PE HEADER 中的变量



// FILESYSTEM 变量

//BYTE xdata G_startTag,G_curTag;//起始tag/当前tag



TlvInfo xdata G_TemplateTlvInfo;//模版信息








//PE SECURITYDOMAIN 



BYTE xdata instanceAID[0x11]; // add by chentao 20220426 // add by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
BYTE xdata extraditeSDAID[0x11]; // add by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量


//BYTE xdata G_AlgorithmID; //zhhj G_AlgorithmID==1 millenage ,G_AlgorithmID==2 tuak 
//BYTE code thePEHeaderTag[] = {0xA0};
// /*-- if set, indicate that the support of this PE is mandatory
//	* NULL OPTIONAL,
//	*/
//BYTE code thePEHeaderMandated[] = {0xA0,0x80};
////-- Identification number of this PE,UInt15
//BYTE code thePEHeaderIdentification[] = {0xA0,0x81};



#define  WAIT_TAG	    0
#define  WAIT_LEN1		1
#define  WAIT_LEN2		2
#define  WAIT_LEN3		3
#define  WAIT_LEN4		4
#define  WAIT_DATA		5


/**
*modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
static code PELIST pelist[]=
{
	{0xA0,ProfileHeaderProcess},		//0xA0
	{0xA1,GFMPEProcess},          		//0xA1
	{0xA2,PINCodesPEProcess}, 	 		//0xA2
	{0xA3,PUKCodesPEProcess},			//0xA3
	{0xA4,AKAParameterPEProcess},		//0xA4
	{0xA5,CDMAParameterPEProcess},		//0xA5
	{0xA6,SecurityDomainPEProcess},		//0xA6
	{0xA7,RFMPEProcess},				//0xA7
	{0xA8,ApplicationPEProcess},		//0xA8
	{0xA9,NonStandardPEProcess},		//0xA9
	{0xAA,EndPEProcess},				//0xAA

	/* PEs related to file system creation using templates defined in this specification */	
	{0xB0,MFTemplateProcess},
	{0xB1,CDTemplateProcess},
	{0xB2,TelTemplateProcess},
	{0xB3,UsimTemplateProcess},
	{0xB4,OptialUsimTemplateProcess},
	{0xB5,IsimTemplateProcess},
	{0xB6,OptialIsimTemplateProcess},
	{0xB7,phonebookTemplateProcess},
	{0xB8,GSMAccemplateProcess},
	{0xB9,CsimTemplateProcess},
	{0xBA,OptialCsimTemplateProcess},
	{0xBB,eapTemplateProcess},
	{0xBC,df5gsTemplateProcess},
	{0xBD,SaipTemplateProcess},
	{EOL, UnknownPETag},
};


/*
*@funtion    :PreAnalyseCreateFile
*
*@description: 下载profile中发现有文件需要移动，将后面的profile数据预解析一下，如果发现后续有移动的文件，将该文件的空间合并到一起移动，减少一条指令中移动文件的次数，优化移动速度
*
*@param      :void
*
*@return     :UWORD modifyLen，合并之后，需要移动的数据长度
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx 20230328
*@history    :V1.1:modify by zhxx@20240320预解析的数据里面有DF的话，已经预留了空间，当创建这个DF的时候，没有判断当前MF的maxSize和usedSize，导致又一次移动，导致文件系统乱掉了
*/
UWORD PreAnalyseCreateFile()
{
	TlvInfo xdata preAnalyseTlvInfo;
	BYTE idata result,pos,fileType;
    UWORD xdata fileSize,modifyLen;

	modifyLen = 0;

	fileSize = 0;

	//mfMaxSize = GetFileHeadWord(G_CreatingMFOffset, F_MSize_Offset);

	//currentDFUsedSize = GetFileHeadWord(G_LoadDFAddr, F_UsedSize_Offset);
	memCopyNoAtomic((PM8)&preAnalyseTlvInfo.bufferAddr,(PM8)&G_PETlvInfo.bufferAddr,sizeof(TlvInfo));//将正在进行profile解析的上下文信息拷贝到preAnalyseTlvInfo中，继续解析

	result = findTLV(&preAnalyseTlvInfo,0xFF,0xFF,1);

	if((result == FALSE) && (preAnalyseTlvInfo.findNextLen > 0))//数据在tag或者长度处断开，直接备份返回
	{
		return modifyLen;
	}

	while (result == FALSE)
	{
		if(setReturnFind(&preAnalyseTlvInfo) == FALSE)
		{
			return modifyLen;
		}

		result = findTLV(&preAnalyseTlvInfo,0xFF,0xFF,1);

		if((result == FALSE) && (preAnalyseTlvInfo.findNextLen > 0))//数据在tag或者长度处断开，直接备份返回
		{
			return modifyLen;
		}
	}

	 while(result)
	 {
		if((preAnalyseTlvInfo.levelIndex)==1)
		{
			if (preAnalyseTlvInfo.tagBuffer[preAnalyseTlvInfo.levelIndex] == Tag_fileManagementListTag)
			{
				setVerticalFind(&preAnalyseTlvInfo);
			}
			else
			{
				return modifyLen;
			}

		}
		else if((preAnalyseTlvInfo.levelIndex)==2)
		{

			if((preAnalyseTlvInfo.tagBuffer[preAnalyseTlvInfo.levelIndex] == Tag_filePath) || (preAnalyseTlvInfo.tagBuffer[preAnalyseTlvInfo.levelIndex] == Tag_createFCP) || (G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_fillFileOffset))
			{
				if(preAnalyseTlvInfo.isCompleteBuffer[preAnalyseTlvInfo.levelIndex] == FALSE)//处理的tag不完整
				{
					return modifyLen;
				}
			}

			switch(preAnalyseTlvInfo.tagBuffer[preAnalyseTlvInfo.levelIndex])
			{
				case Tag_filePath://选择其它路径，退出

					return modifyLen;

				case Tag_createFCP://同一目录下的创建文件，进行合并
					VMReadBuf(tempBuf, preAnalyseTlvInfo.bufferAddr + preAnalyseTlvInfo.findValueOff , preAnalyseTlvInfo.findValueLen);
					pos = FindTag(tempBuf,0x82,preAnalyseTlvInfo.findValueLen);              //File Type
					fileType = tempBuf[pos+1]&FILETYPE;

					pos = FindTag(tempBuf,0x80,preAnalyseTlvInfo.findValueLen);//find file size

					if(pos != FALSE)
					{
						if(tempBuf[pos] == 0x01)				  //UICCAPI Api_2_Tkr_RgfeSo 测到 800103 ，需要支持01长度的fileSize
						{
							fileSize = tempBuf[pos+1];
						}
						else if(tempBuf[pos] == 0x02)
						{
							fileSize = BuildWord(&tempBuf[pos+1]);
						}
					}

					pos = FindTag(tempBuf,0xC7,preAnalyseTlvInfo.findValueLen);//是否是映射文件

					if(pos != FALSE)
					{
						fileSize = 0;
					}

					if(fileType == ADFTYPE)
					{
						return modifyLen; //modify by zhxx@20240320,DF直接退出，避免嵌套,逻辑更简单，但是性能可能会降低，[02.eSIM_Test_RSP.sol]<12.eSIM_BugList><外场测试发现的bug><BugList_34_01>
						//modifyLen += (sizeof(FileSystemHead));
					}
					else if((fileType&0x37) == CYCTYPE)   //循环文件
					{
						if(pos != FALSE)//映射文件
						{
							modifyLen += (sizeof(RecordFileHead));//只增加文件头
						}
						else
						{
							modifyLen += (fileSize + sizeof(RecordFileHead) + 1);//注意，循环的内容一定要再多增加一个字节
						}
					}
				#if UICC__FUNCTION__BERTLVCmd
					else if((fileType&0x3F) == 0x39)//bertlv类型文件
					{
//						if(pos != FALSE)//映射文件
//						{
//							modifyLen += (fileSize + sizeof(BERTLVFileHead));
//						}
//						else
//						{
//							modifyLen += (fileSize + fileSize + sizeof(BERTLVFileHead));
//						}
						return modifyLen; //modify by zhxx@20240321,bertlv文件大小的获取比较复杂，遇到这种类型的文件直接返回不累加了

					}
				#endif
					else//记录文件/透明文件
					{
						modifyLen += (fileSize + sizeof(TranFileHead));
					}

					break;

				case Tag_fillFileOffset://改变updateOff的值，不处理，继续解析
				case Tag_fillFileContent: //对LF/CF/TF文件进行更新，不处理，继续解析
					break;
				default:
					return modifyLen;
			}

		}

		result = findTLV(&preAnalyseTlvInfo,0xFF,0xFF,1);

		if((result == FALSE) && (preAnalyseTlvInfo.findNextLen > 0))//数据在tag或者长度处断开，直接备份返回
		{
			return modifyLen;
		}

		while (result == FALSE)
		{
			if(setReturnFind(&preAnalyseTlvInfo) == FALSE)//返回上一层级
			{
				break;
			}

			result = findTLV(&preAnalyseTlvInfo,0xFF,0xFF,1);

			if((result == FALSE) && (preAnalyseTlvInfo.findNextLen > 0))//数据在tag或者长度处断开，直接备份返回
			{
				return modifyLen;
			}
		}
	}

	return modifyLen;
}

/*
EUICCResponse ::= SEQUENCE {
	peStatus SEQUENCE OF PEStatus,
	profileInstallationAborted NULL OPTIONAL,
	statusMessage UTF8String OPTIONAL
}*/

/*
*@funtion    :InitPEManageVariable
*
*@description: profile解析器变量初始化，上电后第一条指令调用，防止变量未初始化就使用
*
*@param      :void
*            
*@return     :void
*
*@author     :xwh
*
*@history    :V1.0 :create by xwh 20190305
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void InitPEManageVariable(void)
{

	ecpMem_Set((BYTE *)&G_PEControlContext,0x00,sizeof(G_PEControlContext));

	ecpMem_Set((BYTE *)&G_profileControlContext,0x00,sizeof(G_profileControlContext));

	ecpMem_Set((BYTE *)&G_fileControlContext,0x00,sizeof(G_fileControlContext));

	ecpMem_Set((BYTE *)(G_profileControlContext.PEID), 0xFF, sizeof(G_profileControlContext.PEID));

	//G_PEIndex = 0x00;
#if RSP_EUICC
	G_ProfileErrorReason = 0;
	G_USIM_identification = 0;
	G_CSIM_identification = 0;
	G_Tmp_identification = 0; // add by xieyong@20211115
#endif

	G_TempLen = 0;
	G_PETlvInfo.bufferLen = 0;
	G_CreatingMFOffset = INVALID_ADDR;
	G_profileControlContext.CURLOADDIR = 0x2F00;

}
/*
*@funtion    :appendPEStatusInfo
*
*@description: 用来拼装PE STATUS，拼装结果是30[8001XX 8101XX](出错)/30[800100](正确)
*
*@param      :BYTE type          -- PE执行的状态,eg:lib-not-supported(8),ok(0)
*             BYTE identifier    --出错的PE头中的identifier，正确的话没有该项，传递的参数为0
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170802
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*            :V1.2:modify by zhxx@20221204,增加配置O_RESP_OFFSET的处理
*/
void appendPEStatusInfo(BYTE type,WORD identifier)
{
	BYTE idata idLen;
	
#define  PESTATUS_WARMING_LEN   13//拼装一条警告需要的长度  30XX 8001XX + 81XXXX(最多4字节)+ 8302xxxx （警告） ---- 13字节，82tag没有拼

	if(identifier != 0)// if else和下面拼装警告相对应
	{
		if((G_profileControlContext.EUICCResponseLen+PESTATUS_WARMING_LEN) > (sizeof(G_EUICCResponse) - 7))//7是预留给L 30xx a0xx 8100 的
		{
			return;//不继续拼装警告
		}
	}
	else
	{
		if((G_profileControlContext.EUICCResponseLen + 9) > (sizeof(G_EUICCResponse) - 7))//4是if(identifier == 0)时拼装的响应长度，如果后续有增加 需要响应的做调整
		{
			return;//不继续拼装警告
		}
	}
	G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = 0x30;
	if(identifier != 0)
	{//出错或者警告
		if(identifier > 0x7F)
		{
			idLen = 2;
		}
		else
		{
			idLen = 1;
		}
		if(O_RESP_OFFSET)
		{
			G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = idLen + 9;
		}
		else
		{
			G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = idLen + 5;
		}
				
		SetWord(G_EUICCResponse+G_profileControlContext.EUICCResponseLen,0x8001);
		G_profileControlContext.EUICCResponseLen += 2;
		G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = type; //eg:lib-not-supported(8),ok(0)
		G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = 0x81;
		G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = idLen;
		if(identifier > 0x7F)
		{
			SetWord(&G_EUICCResponse[G_profileControlContext.EUICCResponseLen],identifier);
			G_profileControlContext.EUICCResponseLen += 2;
		}
		else
		{
			G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = identifier;
		}		
		
		if(O_RESP_OFFSET)
		{
			SetWord(G_EUICCResponse+G_profileControlContext.EUICCResponseLen,0x8302);//offset UInt31 OPTIONAL -- Position of the part of the PE generating this status code
			G_profileControlContext.EUICCResponseLen += 2;
			SetWord(G_EUICCResponse+G_profileControlContext.EUICCResponseLen,G_PETlvInfo.findTagOff);
			G_profileControlContext.EUICCResponseLen += 2;
		}
		
	}
	else
	{
		if(O_RESP_OFFSET)
		{
			G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = 7;
		}
		else
		{
			G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = 3;
		}
		
		SetWord(G_EUICCResponse+G_profileControlContext.EUICCResponseLen,0x8001);
		G_profileControlContext.EUICCResponseLen += 2;
		G_EUICCResponse[G_profileControlContext.EUICCResponseLen++] = type; //eg:lib-not-supported(8),ok(0)
		
		if(O_RESP_OFFSET)
		{
			SetWord(G_EUICCResponse+G_profileControlContext.EUICCResponseLen,0x8302);//offset UInt31 OPTIONAL -- Position of the part of the PE generating this status code
			G_profileControlContext.EUICCResponseLen += 2;
			SetWord(G_EUICCResponse+G_profileControlContext.EUICCResponseLen,G_PETlvInfo.findTagOff);
			G_profileControlContext.EUICCResponseLen += 2;
		}
	}
	
}


/*
*@funtion    :appendAbordInfo，拼装的结果为：A0[30[]30[]]8100			//8001xx 标识错误状态字， 8101xx 标识第几个PE出错
*
*@description: 用来拼状错误的EUICC RESPONSE
*
*@param      :void         

*@return     :void
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190623
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void appendAbordInfo()
{
	G_EUICCResponse[G_profileControlContext.EUICCResponseLen] = 0x81;//profileInstallationAborted
	G_EUICCResponse[G_profileControlContext.EUICCResponseLen+1] = 0;
			
	ecpMem_Move(G_EUICCResponse+5,G_EUICCResponse,G_profileControlContext.EUICCResponseLen+2);
	G_EUICCResponse[0] = G_profileControlContext.EUICCResponseLen+6;//第一个字节是L
	G_EUICCResponse[1] = 0x30;
	G_EUICCResponse[2] = G_profileControlContext.EUICCResponseLen+4;
	G_EUICCResponse[3] = 0xA0;
	G_EUICCResponse[4] = G_profileControlContext.EUICCResponseLen;
}

/*
*@funtion    :appendErrorResponse，拼装的结果为：A0[30[]30[]]8100			//8001xx 标识错误状态字， 8101xx 标识第几个PE出错
*
*@description: 用来拼状错误的EUICC RESPONSE
*
*@param      :BYTE type          -- PE执行的状态,eg:lib-not-supported(8)
*             BYTE identifier    --出错的PE头中的identifier，正确的话没有该项，传递的参数为0
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170802
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/

void appendErrorResponse(BYTE type,WORD identifier)
{
	
	G_profileControlContext.ProfileCurrStatus = PROFILE_INIT_STATE;

	appendPEStatusInfo(type,identifier);
	
    appendAbordInfo();

#if M2M_EUICC
	IF_M2M_DeleteProfilePE(G_DownLoadingProfileIndex);
#endif

#if RSP_EUICC
	G_ProfileErrorReason = ErrorReason_installFailedDueToPEProcessingError;	
#endif

}



/*
*@funtion    :backData
*
*@description: PE解析到内层的tlv不完整时，将不完整的tlv备份起来
*
*@param       :UWORD desAddr
*@param       :UWORD srcAddr
*@param       :WORD dataLen
*@return     :void
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190124
*            :V1.1 :modify by zhxx@20220419 同步EPX产业化/INFENION_SLM97/Branch/huawei代码，PEApp/processData很长，超过PE缓存能力的处理
*/
BYTE backData(UWORD desAddr,UWORD srcAddr, WORD dataLen)
{
	if(VMCopyDataToAddr(desAddr,EUICCBackPEBufSize,srcAddr,dataLen,MODE_PROTECT_WRITE) == FALSE)
	{
		return FALSE;
	}

	G_profileControlContext.BackInfoLen = dataLen;

	return TRUE;
}


/*
*@funtion    :initPE
*
*@description: 每个PE解析前的初始化操作
*
*@param      :void
*@return     :void
*
*@author     :zhxx
*
*@history    :V1.0 :create by zhxx@20190124
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void initPE(BYTE PEtag)
{
	//通用初始化
	G_PEControlContext.IsExistFlag2 = 0;
	G_PEControlContext.IsExistFlag3 = 0;
	G_PEControlContext.TagA0Occ = 0;
	G_profileControlContext.TotalLen = 0;
		
	if((PEtag >= FILE_TEMPLATE_STATE_TAG) && (PEtag <= FILE_TEMPLATE_END_TAG))
	{
		G_fileControlContext.FirstDFName = FALSE;
		G_fileControlContext.FirstDFAddr = FALSE;
		SetBit(G_PEControlContext.IsExistFlag2,DFFlag);
		G_PEControlContext.FileDescriptorTimes = 0;
		G_PEControlContext.CurTag = 0;
	}
	else if(PEtag == 0xA0)//GFMPE的初始化
	{
		G_PEControlContext.C4RemainLen = 0;//updateBinary的剩余数据长度
	}
	else if(PEtag == 0xA6)//PESD初始化
	{
		PEGPGlobalVariableInit();
		G_fileControlContext.LoadDFAddr = 0;
	}
	else if(PEtag == 0xA8)//PEApplication初始化
	{
		G_PEControlContext.C4RemainLen = 0;//Application包的剩余数据长度
		G_PEControlContext.BlockNum = 0;
		G_PEControlContext.ApplicationLoadPackageOff = 0;
		PEGPGlobalVariableInit();
		G_fileControlContext.LoadDFAddr = 0;
	}
}
/*
*@funtion    :PEProcess
*
*@description: PE 模块解析
*
*@param
*
*@return     :void
*
*@author     :chentao
*
*@history    :V1.0 :create by chentao@20210621
*/
BYTE  PEProcess(UWORD dataBuffer,WORD dataOff,WORD datalen,BYTE PEtag)
{
	BYTE idata temp,i,status;
	WORD idata bacDirFileId;
	UWORD xdata bacFileAddr;

	status = FALSE;

	if(G_profileControlContext.BackInfoLen == 0)
	{
		initTlvInfo(&G_PETlvInfo,dataBuffer+dataOff,datalen);
	}
	else
	{
		if(VMCopyDataToAddr(dataBuffer-G_profileControlContext.BackInfoLen,EUICCBackPEBufSize,EUICCBackPEBuf,G_profileControlContext.BackInfoLen, MODE_PROTECT_WRITE)  == FALSE)
		{
			appendErrorResponse(MEMORY_FAILURE_PE,G_PEControlContext.PE_identification);
			return FALSE;
		}

		temp = G_PETlvInfo.levelIndex;

		initTlvInfo(&G_PETlvInfo,dataBuffer-G_profileControlContext.BackInfoLen,G_profileControlContext.BackInfoLen+datalen);

		G_PETlvInfo.levelIndex = temp;

		G_profileControlContext.BackInfoLen = 0;
	}

	//backupFilesystemInfo
	bacFileAddr = G_CurMFFileStart;		//备份当前激活的profile的MF的偏移
	bacDirFileId = G_LTEDirFileID;     //备份当前激活的profile的dir文件id

	G_CurMFFileStart = G_CreatingMFOffset;	//将G_CurMFFileStart指向将正在下载的profile
	G_LTEDirFileID = G_profileControlContext.CURLOADDIR;//将G_LTEDirFileID指向正在下载的profile

	for(i=0;;i++)
	{
		if((pelist[i].PETag  == PEtag) || (pelist[i].PETag == EOL))
		{
			status = pelist[i].func();
			break;
		}
	}

	G_CurMFFileStart = bacFileAddr;	//恢复当前激活的profile的G_CurMFFileStart
	G_LTEDirFileID = bacDirFileId; //恢复当前激活的profile的G_LTEDirFileID

	return status;
}
/*
*@funtion    :interpreterPE
*
*@description: profile解析入口，用于派发给各个PE进行处理
*
*@param      :UWORD dataBuffer -- profile明文数据
*             WORD dataLen    --profile明文数据长度
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void interpreterPE(UWORD dataBuffer,WORD dataLen/*,BYTE *outPut*/)
{	
	BYTE idata status,temp;
	WORD idata dataOff;

	dataOff = 0;
	
	if(G_profileControlContext.ProfileCurrStatus == PROFILE_END_STATE)
	{
		return ;                            //若profile是已经创建结束，不处理，不需要拼装响应数据
	}
		
	while(dataLen > 0)
	{
		switch(G_profileControlContext.LoaderState)
		{
			case WAIT_TAG:
				
				G_PEControlContext.PETag = VMReadByte(dataBuffer+dataOff);

				if(G_PEControlContext.PETag == 0xA0)//profileHeader在0xa0处就断开的情况，在此时就处理，把状态置上，放在ProfileHeaderProcess中进行处理就晚了
				{
					if(G_profileControlContext.ProfileCurrStatus == PROFILE_CREATING_STATE) //PE header 重复
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,0);

						return;
					}
						
					G_profileControlContext.ProfileCurrStatus = PROFILE_CREATING_STATE;//profile进入创建状态
				}
				
				initPE(G_PEControlContext.PETag);
				dataOff++;
				dataLen--;
				G_profileControlContext.LoaderState = WAIT_LEN1;
				break;
			case WAIT_LEN1:				
				temp = VMReadByte(dataBuffer+dataOff);
				if(temp == 0x81)
				{
					G_profileControlContext.LoaderState = WAIT_LEN2;
				}
				else if(temp == 0x82)
				{
					G_profileControlContext.LoaderState = WAIT_LEN3;
				}
				else
				{
					G_profileControlContext.PESize = temp;
					G_profileControlContext.LoaderState = WAIT_DATA;
				}
				dataOff++;
				dataLen--;
				break;
		    case WAIT_LEN2:
		    	G_profileControlContext.PESize = VMReadByte(dataBuffer+dataOff);
		    	G_profileControlContext.LoaderState = WAIT_DATA;
				dataOff++;
				dataLen--;
				break;
			case WAIT_LEN3:
				G_profileControlContext.PESize = VMReadByte(dataBuffer+dataOff);
				G_profileControlContext.LoaderState = WAIT_LEN4;
			    dataOff++;
				dataLen--;
				break;
			case WAIT_LEN4:
				temp = VMReadByte(dataBuffer+dataOff);
				G_profileControlContext.PESize = ((G_profileControlContext.PESize<<8) | temp);
				G_profileControlContext.LoaderState = WAIT_DATA;
			    dataOff++;
				dataLen--;
				break;
			case WAIT_DATA:		
				G_profileControlContext.BackInfoNextLen = dataLen;
				if(G_profileControlContext.PESize <= dataLen)//一批数据包含一个或者多个PE,即PE是完整的
				{
					ClearBit(G_PEControlContext.IsExistFlag2,Mandated);
					SetBit(G_PEControlContext.IsExistFlag2,PE_Complete);
					
					status = PEProcess(dataBuffer,dataOff,G_profileControlContext.PESize,G_PEControlContext.PETag);
					
					if(status == FALSE)
					{  							
						return;		//1、profile在执行过程中产生错误，拼好响应数据到outPut中，直接返回,需要拼装响应数据 2、执行到end，拼装好数据到outPut中，直接返回
					}

					dataOff += G_profileControlContext.PESize;
					dataLen -= G_profileControlContext.PESize;
					G_profileControlContext.PESize = 0;
					G_profileControlContext.LoaderState = WAIT_TAG;
				}
				else//一批数据只包含部分PE,即PE是不完整的
				{
					status = PEProcess(dataBuffer,dataOff,dataLen,G_PEControlContext.PETag);
										
					G_profileControlContext.PESize -= dataLen;

					return;
				}
				break;
			default: 			  //此处为了白盒测试添加
				break;
		}
	}
	
}



/*
*@function   : PEHeaderAnalyze
*
*@description: 解析PE组件中的PEHeader，如mf-header   telecom-header  cd-header等
*              PEHeader ::= SEQUENCE { 
*                mandated NULL OPTIONAL, -- if set, indicate that the support of this PE is mandatory 
*               identification UInt15 -- Identification number of this PE 
*              }
*
*@参考规范     :《eUICC Profile Package Interoperable Format Technical Specification v2.0.pdf》 8.1.3
*
*@初始化条件  :G_PETlvInfo.findValueOff 指向PEHeader中mandated 所在的偏移,G_PETlvInfo.findValueLen为该组件的总长度         
*
*@param       :void
*
*@rerurn      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20170711
*             :V1.1:modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE PEHeaderAnalyze(void)
{
	BYTE idata i,len,j,tag81;
	WORD id;
	
	tag81 = 0;
	
	if(G_profileControlContext.ProfileCurrStatus != PROFILE_CREATING_STATE)//8.2.1.4. Installing profile without ProfileHeader PE, eUICC responses with PEStatus (5) invalid-request-format.
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,0);
		return FALSE;
	}
	VMReadBuf(tempBuf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);   //根据PEheader的结构，其数据体不会超过256 ，所以此处采用tempBuf一次读取处理
	
	for(i=0; i< G_PETlvInfo.findValueLen; )  //Note: 此处代码没有考虑80 81 的顺序
	{
		if(tempBuf[i] == 0x80)  //mandated NULL, 8.1.3: The mandated field is used to indicate that the support of this PE is mandatory for the installation of this profile. If the eUICC does not support the following PE, it shall abort the processing of the profile and return an error to the sender of the profile.
		{
			len = tempBuf[i+1];
			
			if(len != 0)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
			
			SetBit(G_PEControlContext.IsExistFlag2,Mandated);
		}
		else if(tempBuf[i] == 0x81)  //The identification field is used to uniquely identify a PE within the profile. It will be used for error reporting to the sender of the profile.
		{
			len = tempBuf[i+1];
			if(len == 2)
			{
				G_PEControlContext.PE_identification = BuildWord(&tempBuf[i+2]);
			}
			else if(len == 1)
			{
				G_PEControlContext.PE_identification = tempBuf[i+2];
			}
			else
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
			
			for(j =0; j<sizeof(G_profileControlContext.PEID); j+=2)
			{
				id =  BuildWord(&G_profileControlContext.PEID[j]);
				if(id == 0xFFFF)
				{
					break;
				}
				if(G_PEControlContext.PE_identification == id)
				{
					if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_PEID_CHECK) == 0x01)
					{
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//add by chentao@20220421 Germany profile has  the same PE id

						return FALSE;
					}

				}				
			}	
			
			if(j == sizeof(G_profileControlContext.PEID))// add chentao@20220804 
			{
				j = 0;
			}
			
			SetWord(&G_profileControlContext.PEID[j], G_PEControlContext.PE_identification);
			
			tag81 = 1;
		}
		else  
		{
			//return;//PEheader中的非法tag
			appendErrorResponse(INVALID_REQUEST_FORMAT,0);
			return FALSE;
		}
		
		i += (len +2);   //PEheader中的tag len均为1字节表示
		
	}	
	
	if(tag81 == 0)
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,0);
		return FALSE;
	}
	return TRUE;
}

#endif
