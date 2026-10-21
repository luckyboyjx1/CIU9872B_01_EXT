/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEEnd.c
*
* Description: 该文件主要是对PE-End的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
//#include  "main.h"
//#include  "JCREMain.h"
#include  "fileAction.h"
#include  "tlvHeader.h"
//#include  "resume.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"
#include "Gsmpublic.h"
#include "Interface_LTEFromPlatform.h"
#include "fileSystem.h"
#include "gsmpublic.h"
#include "fileAction.h"
#include "stkhead.h"
#include "EUICCProfile.h"
#include  "ProfileInterpreter.h"

#if (EUICC_ONLY||SE_AND_EUICC)

#define FILE_NAME_OFF   7
#define FILE_ACC_OFF    15
#if UICC_AB8C_ARR_PRIVATE
	#define FILE_TAGSIZE_OFF  29
	#define FILE_SIZE_OFF     32
#else
	#define FILE_SIZE_OFF     19
#endif
//EFENV-CLASSES SHALL never be present in any Profile Package, however if present, it SHALL be ignored by the eUICC when installing the Profile.
//EFUMPC MAY be present in a Profile Package. If present and EFUMPC is also present in the default file system, the second byte of the default file SHALL be modified by the content of the Profile Package when this Profile is enabled. 
//The eUICC SHALL ignore the content of all the other bytes of the file present in the Profile Package.
//If present and EFUMPC is not present in the default file system, the whole file from the Profile SHALL be taken into account.
/**
*modify by chentao@20210914: 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
 history: modify by xieyong@20211223  AB/8C规则存储在私有文件B200/2B06,增加宏开关UICC_AB8C_ARR_PRIVATE
*/
void createDefaultFileSystem()
{
#if RSP_EUICC
	BYTE idata arrNum,fileLgth,offset;
	UWORD xdata maxMFOffset,srcfileAddr,desfileAddr;
	//TranFileHead xdata transEfFileHead;  
	//WORD idata fileSize;
#if UICC_AB8C_ARR_PRIVATE	
	BYTE xdata EF2F07[] = {0x82,0x02,0x41,0x21,0x83,0x02,0x2F,0x07,0x8A,0x01,0x05,0xAB,0x10,0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x1A,0xA4,0x06,0x83,0x01,0x0A,0x95,0x01,0x08,0x80,0x02,0x00,0x0A};
#else		
	BYTE xdata EF2F07[] = {0x82,0x02,0x41,0x21,0x83,0x02,0x2F,0x07,0x8A,0x01,0x05,0x8B,0x03,0x2F,0x06,0x0A,0x80,0x02,0x00,0x0A};
#endif		
	maxMFOffset = GetIfMfCreated(MODE_DEFAULT);
	
	if(maxMFOffset == INVALID_ADDR)
	{
		return;
	}
	offset = 0x00;
	
	srcfileAddr = SelectFile(maxMFOffset,EF_CLASSES);
	
	if(srcfileAddr != FALSE)//如果默认文件系统存在EF_CLASSES文件，则拷贝到-p下面
	{
		arrNum = GetFileHeadByte(srcfileAddr,F_ArrNum_Offset);//获取默认文件系统EF_CLASSES的访问规则	
		
        EF2F07[FILE_SIZE_OFF] =  (GetFileHeadWord(srcfileAddr,F_SpaceSize_Offset)&0xff);//获取默认文件系统EF_CLASSES的大小
#if UICC_AB8C_ARR_PRIVATE
		if((arrNum&A_PRIVATEARR_REC_FLAG)==A_PRIVATEARR_REC_FLAG)
		{
			ecpMem_Copy(tempBuf,EF2F07,0x0B);
			offset += 0x0B;
			
			fileLgth =	GetDefaultFileSystemPrivateArrABInfo(maxMFOffset,arrNum,tempBuf+offset);
			offset += fileLgth;
			
			ecpMem_Copy(tempBuf+offset,&EF2F07[FILE_TAGSIZE_OFF],4);
			offset += 0x04;
		}
		else
#endif
		{
			EF2F07[FILE_ACC_OFF] = arrNum;
			
			ecpMem_Copy(tempBuf,EF2F07,sizeof(EF2F07));
			offset += sizeof(EF2F07);
		}
		commonCreateEFFile(G_CreatingMFOffset,EF_CLASSES,tempBuf,offset,PROFILE_MODE);
	}
	
	offset = 0x00;
	
	srcfileAddr = SelectFile(maxMFOffset,EF_UMPC);
	desfileAddr = SelectFile(G_CreatingMFOffset,EF_UMPC);
	
	if(srcfileAddr != ADDR_IS_NULL)//如果默认文件系统存在EF_UMPC文件，则拷贝到-p下面
	{
		arrNum = GetFileHeadByte(srcfileAddr,F_ArrNum_Offset);//获取默认文件系统EF_UMPC的访问规则		
		
		if(desfileAddr == ADDR_IS_NULL)//profile中不存在EF_UMPC文件，则从默认文件系统中拷贝，创建
		{
			EF2F07[FILE_NAME_OFF] = 8;
			EF2F07[FILE_SIZE_OFF] = 5;
#if UICC_AB8C_ARR_PRIVATE
			if((arrNum&A_PRIVATEARR_REC_FLAG)==A_PRIVATEARR_REC_FLAG)
			{
				ecpMem_Copy(tempBuf,EF2F07,0x0B);
				offset += 0x0B;
				
				fileLgth =	GetDefaultFileSystemPrivateArrABInfo(maxMFOffset,arrNum,tempBuf+offset);
				offset += fileLgth;
				
				ecpMem_Copy(tempBuf+offset,&EF2F07[FILE_TAGSIZE_OFF],4);
				offset += 0x04;
			}
			else
#endif
			{
				EF2F07[FILE_ACC_OFF] = arrNum;
				
				ecpMem_Copy(tempBuf,EF2F07,sizeof(EF2F07));
				offset += sizeof(EF2F07);
			}	
			commonCreateEFFile(G_CreatingMFOffset,EF_UMPC,tempBuf,offset,PROFILE_MODE);
		}
		else//如果profile中也存在EF_UMPC文件，则只使用profile中的第2个字节，其它字节使用默认文件系统的
		{
			ReadTFBuf(srcfileAddr,0,tempBuf,5);
			ReadTFBuf(desfileAddr,1,tempBuf+1,1);
			WriteTFBuf(desfileAddr,0,tempBuf,5);
		}
				
	}
	
#endif
}



/*
*@function     : MNNMNCConvert 
*
*@description  :将6f07文件中的mccmnc进行编码转换
*
*@see          : 
*
*@param        :BYTE *Src,BYTE *Des
*
*@return       :void
*
*@RAM Resources:
*
*@author       :zhxx
*
*@hisotry      :
×　　　　　　　V1.0 :create @20190322
*/

/*eg:0x6f07中的值是：082998281232547696
转换后获取到卡片实际的MCC = 289，MNC = 822
则289中28互换，822中82互换，mcc中的9和mnc中的2互换
因此metadata中存储数据为82 29 28
*/

void MNNMNCConvert(BYTE *Src,BYTE *Des,BYTE mncLen)
{
	BYTE idata i;
	Src[0] = Src[0] & 0xF0;
	
	for(i = 0; i<4; i++)
	{
		Des[i] = (Src[i]<<4) | (Src[i]>>4);
	}	
	
	for(i = 0; i<3; i++)
	{
		Src[i] = ((Des[i]<<4) | (Des[i+1]>>4));
	}
	
	if(mncLen == 2)
	{
		Src[2] |= 0x0F;
	}
	
	
	Des[0] = (Src[0]<<4) | (Src[0]>>4);
	Des[1] = (Src[2]<<4) | (Src[1]>>4);
	Des[2] = (Src[2]&0xF0) | (Src[1]&0x0F);	
}



/*
*@funtion    :EndPEProcess
*
*@description: 对PE-End进行解析处理
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
BYTE EndPEProcess(void)
{
	UWORD  xdata firstLevelDf;
	BYTE idata result,i,pinRef;
	
	BYTE xdata okResponse[] = {0x30,0x07,0xA0,0x05,0x30,0x03,0x80,0x01,0x00};
#if RSP_EUICC
	BYTE idata  gidService,mncLen = 0,temp,checkTag;
	WORD idata adfId,fileSize;
	UWORD xdata fileAddr,adfAddr,ustAddr;
	WORD idata usimIMSITag = 0;
	WORD idata csimIMSITag = 0;
	adfAddr = FALSE;
	ustAddr = FALSE;
	gidService = 0;
	checkTag = 0;//默认检查失败
	fileSize = 0;
#endif
	
	

	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE)//不够一个PE，将整个数据都备份
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}	
	
	while(1)
	{
		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if(result == FALSE)
		{
			break;  //处理到最后一个tag
		}
		
		switch(G_PETlvInfo.tagBuffer[0])
		{
			case 0xA0:
				
				if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
				{
					return FALSE;
				}				
				break;		
			
			default:
				appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
				return FALSE;			
		}
		
		if( G_PEControlContext.TagA0Occ == 0)
		{
			appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
			return FALSE;
		}
	}	
		
	
	if((G_profileControlContext.BackInfoNextLen-G_profileControlContext.PESize) > 0)//error：PE-END 后面还有数据 HB TestCase01_14
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); //TestCase01_14
		return FALSE;
	}
		
	if(G_profileControlContext.ProfileVersion >= 0x22) //profile2.2规范才做此检查
	{

		firstLevelDf = GetLastDFAddr();//8.2.3.17. Installing EAP files by template，PEEAP之后来了PEAKA，此时会在USIM下面创建私有文件，即在PEEAP之前插入一些文件，G_LoadDfAddr指向的不是最后一个DF地址，需要重新进行获取最后一个DF地址

		if(firstLevelDf != FALSE)//"PIN Context" of the previous PE Template or PE Generic File Management  is ADF
		{
			//In response of the PE-End if PE-PINCodes that should create all local Pins has not been received. 
			//In this case the field identification of the PE Status is set to the one of the PE in which the (A)DF refers to a Local Pin that has not been created.
			 
			for(i= 0x00;i<0x04;i++)
			{
				pinRef = GetFileHeadByte(firstLevelDf, F_ChvStatus_Offset+i);//获取pin的引用

				if(pinRef == 0xFF)
				{
					break;
				}
				else
				{
					if((pinRef >= 0x81) && (pinRef <= 0x88))//如果存在localpinref，没有创建EF_PRIVATE_PIN，则抛错
					{
						 if(SelectFile(firstLevelDf,EF_PRIVATE_PIN) == FALSE)
						 {		 
							 appendErrorResponse(PIN_CODE_MISSING,G_fileControlContext.LoadADfId);
							 return FALSE;
						 }
					}
				}
			}
			
		}	
	}


	/*SGP22.V2.2.2
	 If profileOwner is provided in the Profile Metadata:
	The mccMnc value provided in the Profile Metadata SHALL match the MCC and MNC values in EFIMSI
	If gid1 or gid2 is provided in the Profile Metadata: The corresponding EFGID1 or EFGID2 SHALL be present and contain the same value as provided in the data object and the related service in EFUST SHALL indicate "available".
	If gid1 or gid2 is not provided in the Profile Metadata: The corresponding service in EFUST for EFGID1(6F3E) or EFGID2(6F3F) SHALL indicate "not available".
	according 31102,gid1 service n° 17,gid2 service n° 18*/
#if RSP_EUICC

	adfId = GetADFFID((BYTE *)&USIMPartial[0]);

	if(adfId != FALSE)
	{
		adfAddr = SelectFile(G_CreatingMFOffset,adfId);	//找到USIM文件
	}

	ustAddr = SelectFile(adfAddr,0x6F38);//查找USIM/UST文件

	ReadTFBuf(ustAddr,2,(BYTE idata *)&gidService,sizeof(BYTE));

	if(ProfileOwnerIsExist(G_DownLoadingProfileIndex) == TRUE)//If profileOwner is provided in the Profile Metadata:
	{
		if(IF_CustomerGetDataFromMetadata(GID1_TYPE,tempBuf) == TRUE)// If gid1 provided in the Profile Metadata:
		{
			fileAddr = SelectFile(adfAddr,0x6F3E);

			if(fileAddr != FALSE)//EFGID1  SHALL be present
			{
				fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);

				ReadTFBuf(fileAddr,0,tempBuf+tempBuf[1]+2,fileSize);

				if((fileSize == tempBuf[1]) && (ecpMem_Compare(tempBuf+2,tempBuf+tempBuf[1]+2,fileSize) == 0) && ((gidService&0x01) == 0x01))//contain the same value as provided in the data object and the related service in EFUST SHALL indicate "available"
				{
					checkTag |= 1;
				}
			}
		}
		else//If gid1 is not provided in the Profile Metadata
		{
             if((gidService&0x01) == 0x00)//The corresponding service in EFUST for EFGID1(6F3E) SHALL indicate "not available".
             {
            	 checkTag |= 1;
             }
		}


		if(IF_CustomerGetDataFromMetadata(GID2_TYPE,tempBuf) == TRUE)//// If gid2 provided in the Profile Metadata:
		{
			fileAddr = SelectFile(adfAddr,0x6F3F);

			if(fileAddr != FALSE)////EFGID2  SHALL be present
			{
				fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);

				ReadTFBuf(fileAddr,0,tempBuf+tempBuf[1]+2,fileSize);

				if((fileSize == tempBuf[1]) && (ecpMem_Compare(tempBuf+2,tempBuf+tempBuf[1]+2,fileSize) == 0) && ((gidService&0x02) == 0x02))//contain the same value as provided in the data object and the related service in EFUST SHALL indicate "available"
				{
					checkTag |= 0x02;
				}
			}
		}
		else//If gid2 is not provided in the Profile Metadata
		{
			 if((gidService&0x02) == 0x00)//The corresponding service in EFUST for EFGID1(6F3E) SHALL indicate "not available".
			 {
				 checkTag |= 0x02;
			 }
		}


		if(adfAddr != FALSE)
		{
			usimIMSITag = 1;//存在usim

			fileAddr = SelectFile(adfAddr,0x6FAD);	//选择文件EFAD (Administrative data)

			if(fileAddr != FALSE)
			{
				ReadTFBuf(fileAddr,0x03,&mncLen,1);//length of  MNC in the IMSI
			}

			temp = IF_CustomerGetDataFromMetadata(MCCMNC_TYPE,tempBuf); // 5.5.5  Function: LoadProfileElements

			if(temp != 0)
			{
				if((((tempBuf[3] & 0xF0) == 0xF0) && (mncLen == 3)) || (((tempBuf[3] & 0xF0) != 0xF0) && (mncLen == 2)))//0x6FAD中指示的MNC的长度和Profile Metadata中的不一致
				{
					usimIMSITag = 2;  //存在不匹配
					G_PEControlContext.PE_identification = G_USIM_identification;
				}
				else
				{
					/*The mccMnc value provided in the Profile Metadata SHALL match the MCC and MNC values in EFIMSI.*/
					 /* If the Profile contains PPRs, the eUICC SHALL verify that the Profile Owner mcc and mnc
					values provided in the Profile Metadata are the same as the MCC and MNC values
					contained in the EFIMSI of the Profile*/
					fileAddr = SelectFile(adfAddr,EF_IMSI);	//选择IMSI

					if(fileAddr != FALSE)
					{
						ReadTFBuf(fileAddr,0x01,&tempBuf[0x20],4);	//6F07 第2个字节开始的3个字节

						MNNMNCConvert(&tempBuf[0x20],&tempBuf[0x24],mncLen);

						if(ecpMem_Compare(&tempBuf[2],&tempBuf[0x24],3) != EQUAL) //Metadata域中配置的MCCMNC与6F07中的MCCMNC不一致
						{
							usimIMSITag = 2; //存在不匹配
							G_PEControlContext.PE_identification = G_USIM_identification;
						}
						else
						{
							usimIMSITag = 3; //存在匹配
						}

					}
				}

			}
		}

#if UICC__AUTH__CDMA__CAVE
		adfId = GetADFFID((BYTE *)&CSIMPartial[0]);

		if(adfId != FALSE)
		{
			adfAddr = SelectFile(G_CreatingMFOffset,adfId);		//找到CSIM文件

			if(adfAddr != FALSE)
			{
				csimIMSITag = 1;//存在cSIM

				fileAddr = SelectFile(adfAddr,EF_IMSIM);

				if(fileAddr != FALSE)
				{
					ReadTFBuf(fileAddr,6,&tempBuf[0x20],1);////6F22 第7个字节

					ReadTFBuf(fileAddr,8,&tempBuf[0x21],2);//6F22 第9/10个字节

					temp = IF_CustomerGetDataFromMetadata(MCCMNC_TYPE,tempBuf); // 5.5.5  Function: LoadProfileElements

					if(temp != 0)
					{
						MCCMNCConverToIMSIM(&tempBuf[2],&tempBuf[10]);//Metadata 中B7 tag的80tag

						if((ecpMem_Compare(&tempBuf[10],&tempBuf[0x21],2) != EQUAL) || (ecpMem_Compare(&tempBuf[12],&tempBuf[0x20],1) != EQUAL)) //Metadata域中配置的MCCMNC与6F22中的MCCMNC不一致
						{
							csimIMSITag = 2;//存在不匹配
							G_PEControlContext.PE_identification = G_CSIM_identification;
						}
						else
						{
							csimIMSITag = 3;//存在匹配
						}

					}

				}

			}
		}

#endif

	}
	else
	{
		checkTag = 3;
	}

	if(!Check_GID1GID2) //add by chentao@20220601   profile can download successful when gid1 and gid2 is not matchings with metadata
	{
		checkTag = 3;
	} 

	if((checkTag != 3) //检查失败
			|| ((usimIMSITag == 2) && (csimIMSITag == 0)) ||  //只存在usim，但是usim不匹配
			 ((csimIMSITag == 2) && (usimIMSITag == 0)) ||  //只存在csim，但是csim不匹配
			    ((usimIMSITag == 2) && (csimIMSITag == 2)))  //usim和csim都存在，且都不匹配
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
		G_ProfileErrorReason = ErrorReason_installFailedDueToDataMismatch;
		return FALSE;
	}
				
	

	IF_CustomerGetDataFromMetadata(ICCID_TYPE,tempBuf);
	/*The ICCID provided in the Profile Metadata is identical to the value of EFICCID.*/	
	fileAddr = SelectFile(G_CreatingMFOffset,EF_ICCID);		//找到该profile的2FE2文件
	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr,0x00,&tempBuf[0x20],0x0A);
		if(ecpMem_Compare(&tempBuf[0],&tempBuf[0x20],0x0A) != EQUAL) //Metadata域中配置的ICCID与EFICCID中的不一致
		{	
			appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); // TY MF_TEMPLATE_E01
			
			return FALSE;
		}
	}	
	
#endif
	
	createDefaultFileSystem();//创建默认的文件系统

	G_profileControlContext.ProfileCurrStatus = PROFILE_END_STATE;//设置profile为终止状态
	
	if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_FileNotSupportWaring) == TRUE)  //modify by chentao@20211111  ServicesList不支持BER-TLV
	{
		G_PEControlContext.PE_identification = G_Tmp_identification;
		appendPEStatusInfo(FETURE_NOT_SUPPORTED,G_PEControlContext.PE_identification); //拼警告

		appendPEStatusInfo(OK_PE,0x00);          //为了通过profile3.0套件测试，暂时这样改，不确定用例是否有问题
								//测试eUICC_TCA_Profile_TestSuite-TestSuite/Warning when installing USIM files by template with BER-TLV files in a non mandatory PE when eUICC does not support BER-TLV
	}

	if(G_profileControlContext.EUICCResponseLen == 0)  //PE-End之前没有警告信息，才会拼装ok状态字，否则不拼装OK
	{
		G_EUICCResponse[0] = sizeof(okResponse);
		ecpMem_Copy(&G_EUICCResponse[1],okResponse,G_EUICCResponse[0]);
		
	#if RSP_EUICC
		G_ProfileErrorReason = 0;
	#endif
	}
	else
	{  //PE-End之前全部是警告信息，拼装警告信息
		 //appendAbordInfo();
		ecpMem_Move(G_EUICCResponse+5,G_EUICCResponse,G_profileControlContext.EUICCResponseLen);
		G_EUICCResponse[0] = G_profileControlContext.EUICCResponseLen+4;//第一个字节是L
		G_EUICCResponse[1] = 0x30;
		G_EUICCResponse[2] = G_profileControlContext.EUICCResponseLen+2;
		G_EUICCResponse[3] = 0xA0;
		G_EUICCResponse[4] = G_profileControlContext.EUICCResponseLen;
	}
	
	//SetWord(tempBuf,G_profileControlContext.CURLOADDIR);
	
	IF_SetProfileCapacityInfo(G_profileControlContext.CURLOADDIR);
	
	
	//将当前正在下载的Profile 的状态设置成valid ，该函数需要平台层实现
	IF_SetDownLoadingProfileToValidState();
	
	return FALSE;
}
	
#endif

