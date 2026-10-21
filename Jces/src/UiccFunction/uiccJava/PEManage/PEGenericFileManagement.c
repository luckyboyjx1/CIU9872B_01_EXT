
/************************************************************************************************************
* Copyright (C), 2017, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   PEGenericFileManagement.c
*
* Description: 该文件主要是对PE-GenericFileManagement的所有处理
*
* auther：     zxx    20170623
*
* history：    V1.0
*
************************************************************************************************************/


#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileAction.h"
#include  "gsmpublic.h"
#include  "fileSystem.h"
#include  "tlvHeader.h"
#include  "ProfileInterpreter.h"
#include  "profileApi.h"
#include  "EUICCCustomerPublic.h"
#include  "Interface_LTEFromPlatform.h"
#include "stkhead.h"
#include  "VMAccessEeprom.h"
#include "DataOrientedCommand.h"

#if (EUICC_ONLY||SE_AND_EUICC)
/*
*@funtion    :makeDescript
*
*@description: 根据文件的描述信息，获取文件的类型及记录长度
*
*@param      :TlvInfo * tlv  --待分析的tlv
*             
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
void makeDescript(void)
{
	BYTE idata fileDescript,result;
	
	result = FindTag(tempBuf,0x82,G_TempLen);

	fileDescript = VMReadByte((UWORD)(tempBuf+result+1));
	
	if (((fileDescript & MFTYPE) == MFTYPE) && ((fileDescript&0x01) == 0x00))  //MF/DF
	{ 
		G_fileControlContext.CurrentFileStructure = (fileDescript & MFTYPE);
	}
	else if ((fileDescript & FILETYPE) == TRANTYPE)  //TF
	{ 
		G_fileControlContext.CurrentFileStructure = (fileDescript & FILETYPE);
	}
	else if (((fileDescript & FILETYPE) == LINTYPE)||((fileDescript& FILETYPE) == CYCTYPE))   //LF/CF
	{
	
		G_fileControlContext.CurrentFileStructure = (fileDescript & FILETYPE);
		
		G_fileControlContext.CurrentFileFileRecLen = VMReadShort((UWORD)(tempBuf+result+3));
	}
#if UICC__FUNCTION__BERTLVCmd
	else if (((fileDescript & FILETYPE) == 0x39))   //LF/CF
	{	
			G_fileControlContext.CurrentFileStructure = BERTLVTYPE;
	}
	else
	{
		//白盒测试添加
	}		
#endif
}


/*
*@funtion    :makePInfoData
*
*@description: 根据102226规范，拼装标准的pin属性信息，并替换G_TempTlvInfo中的pin属性信息
*
*@param      :BYTE fPstdTag  --pin属性的tag
*             
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*                 V1.1: modify by xwh 20170713,因为目标TLV 在RAM 地址中，  函数replaceFindTlv 调用修改成replaceFindTlvInRam,简化处理流程
*                 V1.2: modify by chentao C6是TLV格式需要拷贝数据到临时buffer
*/
#define MAX_PIN_NUM							26
BYTE makePInfoData(BYTE fPstdTag)
{
	//BYTE xdata buf[20];  //c6 13 0181828384858687880A0B0C0D0E8A8B8C8D8E
	
	BYTE idata pinSDNum,i,j;
	BYTE xdata pinBuf[MAX_PIN_NUM*3+5];// 26*3 + 5 = 83 ,最大需要83字节
	
	i = 0;//i=180;
	
	pinSDNum = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff); //复用变量读取value 判断是否是TLV格式

	if(pinSDNum == 0x90)//是TLV格式不能直接退出，需要将Profile数据拷贝到临时buffer
	{
		replaceFindTlvInRam(fPstdTag,(BYTE*)G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff, G_PETlvInfo.findTLVLen);
		return TRUE;
	}

	pinSDNum = G_PETlvInfo.findValueLen;
	
	if(pinSDNum > MAX_PIN_NUM)//最大26个pinRef--0x01~0x08/0x81~0x88/0x0a~0x0e/0x8a~0x8e
	{
		return FALSE;
	}
	
	pinBuf[i++] = fPstdTag;
	
	pinBuf[i++] = pinSDNum*3+3;
	
	pinBuf[i++] = 0x90;
	
	pinBuf[i++] = 0x01;
	
	pinBuf[i++] = (0xFF<<(8-pinSDNum))&0xFF;
	
	for(j = 0;j<pinSDNum;j++)
	{
		pinBuf[i++] = 0x83;
		
		pinBuf[i++] = 1;

		pinBuf[i] = VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff+j);
		

		//0xc6引用的pinRef必须是有效的范围1~8,0x81~0x88,0xa~0xe,0x8a~0x8e,否则，返回错误
		if(!(((pinBuf[i] >= 0x01)&&(pinBuf[i]<=0x08)) || ((pinBuf[i] >= 0x81)&&(pinBuf[i]<=0x88))|| ((pinBuf[i] >= 0x0A)&&(pinBuf[i]<=0x0E)) || ((pinBuf[i] >= 0x8A)&&(pinBuf[i]<=0x8E))))
		{
			return 	FALSE;
		}
		
		i++;
	}	
	
	replaceFindTlvInRam(fPstdTag,&pinBuf[0],i);
	
	return TRUE;
}


/*
*@funtion    :makeFileAttrInfo
*
*@description: 根据102226规范，拼装标准的文件属性信息，并替换G_TempTlvInfo中的文件属性信息
*
*@param      :BYTE fPstdTag  --文件属性的tag
*             
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 :modify by xwh 20170713,因为目标TLV 在RAM 地址中，  函数replaceFindTlv 调用修改成replaceFindTlvInRam,简化处理流程
*            :V1.2 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP 
*/
void makeFileAttrInfo(BYTE fAttrTag)
{
	BYTE xdata buf[5];
	BYTE idata i=0;
	
	buf[i++] = fAttrTag;
	
	buf[i++] = 3;
	
	if(G_PETlvInfo.findValueLen == 1)
	{
		if(G_fileControlContext.LoadDFName == 0x3F00)//EF ARR ID implicitly known from the context, i.e. '2F06' within the MF and '6F06' otherwise
		{
			SetWord(&buf[i], 0x2f06);
		}
		else
		{
			SetWord(&buf[i], 0x6f06);
	  }		
		i += 2;
	  
	  
	}

	 

	VMReadBuf(&buf[i],G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff,G_PETlvInfo.findValueLen);	
	
	i += G_PETlvInfo.findValueLen;
	
	replaceFindTlvInRam(fAttrTag,&buf[0],i);	
	  
}


/*
*@funtion    :MCCMNCConverToIMSIM
*
*@description: 将MCCMNC转换成6F22文件中第7，9， 10 个字节
    
*@see         例如MCCMNC为460 03 转换后第7个字节为5C，第9和10字节为6701
			  例如MCCMNC为299 81 转换后第7个字节为46，第9和10字节为BC00
*
*@param      :Src --- storemeta中的3个字节的MCCMNC
*             
*@return     :Des -- 转换后6F22文件中第7，9， 10 个字节
*
*@author     :chentao
*
*@history    :V1.0 :create by chentao@20181107
*/
#if UICC__AUTH__CDMA__CAVE
void MCCMNCConverToIMSIM(BYTE *Src,BYTE *Des)  //92F918
{
	BYTE idata i,mnc,a,b,c;
	WORD idata mcc;
	
	for(i = 0; i<3; i++)
	{
		Des[i] = (Src[i]<<4) | (Src[i]>>4); //299F81
	}	
	
	a = Des[0] >>4;
	b = Des[0] & 0x0F;
	c = Des[1] >>4;
	
	if(a == 0)
	{
		a = 10;
	}
	if(b == 0)
	{
		b = 10;
	}
	if(c == 0)
	{
		c = 10;
	}
	
	mcc = a *100 + b *10 + c - 111;	
	
	Des[1]=((mcc)>>8)&0xFF; 
	Des[0]=(mcc)&0xFF;
	
	a = (Des[2] >>4) & 0x0F;
	b = Des[2] & 0x0F;
	
	
	if(a == 0)
	{
		a = 10;
	}
	if(b == 0)
	{
		b = 10;
	}
	
	mnc =  a *10 + b - 11;
	Des[2] = mnc;
}
#endif



/*
*@funtion    :makeUpdate
*
*@description: 根据文件类型及偏移，对文件进行更新
*@see          SGP.22-V2.1---5.5.5 Function: LoadProfileElements
*
*@param      :void
*             
*@return     :TRUE/0x80/FALSE  更新成功/数据不匹配错误/数据结构错误 HB_Offline_Profile_Test==>14-PROFILE : Profile_FulSeg0025
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
			 :V1.2 :update by chentao@20171207
			 :V1.3 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*/
BYTE makeUpdate(void)
{
	BYTE fileType;
	UWORD   xdata backAddr;
	WORD idata remainLen,updateDataLen,usedSize;
#if RSP_EUICC
	BYTE idata temp;
#endif
	
	remainLen = G_PETlvInfo.bufferLen-G_PETlvInfo.findValueOff;
	
	backAddr = G_fileControlContext.LoadEFAddr;
	fileType = GetFileHeadByte(G_fileControlContext.LoadEFAddr,F_Status_Offset);

	if((fileType&F_MAP_FILE) == F_MAP_FILE)// 如果更新的是link 文件，则实际更新的是实体文件
	{
		G_fileControlContext.LoadEFAddr = GetParentOrMapFileAddr(G_fileControlContext.LoadEFAddr, F_MapFile_Offset);
		G_fileControlContext.CurrentFileFileRecLen	=  GetFileHeadByte(G_fileControlContext.LoadEFAddr,F_RecLgth_Offset);
	}
	
	if(remainLen >= G_PETlvInfo.findValueLen)	//更新数据完整	
	{
		updateDataLen = G_PETlvInfo.findValueLen;
	}
	else
	{
		updateDataLen = remainLen;
		G_PEControlContext.C4RemainLen = G_PETlvInfo.findValueLen - remainLen;
	}
	
	
#if RSP_EUICC
	temp = IF_CustomerGetDataFromMetadata(ICCID_TYPE,tempBuf);
	/*The ICCID provided in the Profile Metadata is identical to the value of EFICCID.*/
	if((G_fileControlContext.LoadEFName == EF_ICCID) && (G_PEControlContext.C4RemainLen == 0))//要更新的数据是完整的
	{
		if(G_PETlvInfo.findValueLen == temp)
		{		
			VMReadBuf(&tempBuf[0x20],(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),G_PETlvInfo.findValueLen);		
			
			if(ecpMem_Compare(&tempBuf[0],&tempBuf[0x20],G_PETlvInfo.findValueLen) != EQUAL) //Metadata域中配置的ICCID与EFICCID中的不一致
			{
				return 0x80; //数据不匹配错误	
			}
		}		
	}
	
	if(G_fileControlContext.LoadEFName == EF_CLASSES)//EFENV-CLASSES SHALL never be present in any Profile Package, however if present, it SHALL be ignored by the eUICC when installing the Profile.
	{
		return TRUE;
	}
	
#endif	
	
	
	if(G_fileControlContext.CurrentFileStructure == TRANTYPE)//TF
	{     
		if(profileUpdateBinary(G_fileControlContext.UpdateOffset,(BYTE*)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),updateDataLen) == FALSE)
		{
			return FALSE;//profile结果错误
		}
	}
	else if((G_fileControlContext.CurrentFileStructure == LINTYPE) || (G_fileControlContext.CurrentFileStructure == CYCTYPE)) //LF/CF
	{
		if(profileUpdateRecord((G_fileControlContext.UpdateOffset / G_fileControlContext.CurrentFileFileRecLen)+1,(G_fileControlContext.UpdateOffset % G_fileControlContext.CurrentFileFileRecLen),(BYTE*)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),updateDataLen) == FALSE)
		{
			return FALSE;//profile结果错误
		}
	}
#if UICC__FUNCTION__BERTLVCmd
	else if(G_fileControlContext.CurrentFileStructure== BERTLVTYPE)
	{
	    WriteBERBuf(G_fileControlContext.LoadEFAddr,G_fileControlContext.UpdateOffset,(BYTE*)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),updateDataLen);

		usedSize = GetFileHeadWord(G_fileControlContext.LoadEFAddr, F_BERTLV_UsedSize);

		usedSize += updateDataLen;

		WritePageCacheEX(G_fileControlContext.LoadEFAddr+F_BERTLV_UsedSize,(BYTE *)&usedSize,sizeof(WORD));
	}
#endif
	else
	{
		return TRUE;
	}
	
	G_fileControlContext.UpdateOffset += updateDataLen;
	
	G_fileControlContext.LoadEFAddr = backAddr;
	
	return TRUE;
}

/*
*@funtion    :makeFileLinkPath
*
*@description: 拼装标准的文件映射信息，并替换G_TempTlvInfo中的文件映射信息
*
*@param      :BYTE linkPathTag        --文件映射的tag
*             
*@return     :void
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*                 V1.1: modify by xwh 20170713,因为目标TLV 在RAM 地址中，  函数replaceFindTlv 调用修改成replaceFindTlvInRam,简化处理流程
*/
void makeFileLinkPath(BYTE linkPathTag)//修改映射文件信息，服务器来的是C7LV，需要修改为8FLV
{
	BYTE xdata buf[DF_Depth*2+2];
	
	VMReadBuf(&buf[0],G_PETlvInfo.bufferAddr+G_PETlvInfo.findTagOff,G_PETlvInfo.findTLVLen);
	
	buf[0] = 0x8F;
	
	replaceFindTlvInRam(linkPathTag,&buf[0],G_PETlvInfo.findTLVLen);	
}



/*
*@funtion    :GFMPEProcess
*
*@description: 对PE-GenericFileManagement进行解析处理
*
*@param      :
*             
*@return     :BYTE    --TRUE:继续执行
*                     --FALSE:终止执行，直接返回
*
*@author     :zxx
*
*@history    :V1.0 :create by zxx@20170623
*            :V1.1 : modify by xwh 20170711,优化程序执行流程，提高执行效率
*            :V1.2 :modify by xwh 20170711,优化程序执行流程，收到62 后直接创建文件，下一级的处理不再需要
*            :V1.3 :modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
*            :V1.4 : modify by zhxx@20220419,同步EPX产业化/INFENION_SLM97/Branch/huawei代码,HB_Offline_Profile_Test/Profile_FulSeg0047,when create adf,82,83tag should in order
*/
//extern BYTE code USIMPartialAID[] ;

//extern BYTE code USIMPartialAID1[] ;
BYTE GFMPEProcess(void)
{
	BYTE  idata result,pos, temp;
	UWORD xdata backFileAddr;
	BYTE idata recNum,tempRec;
	
	recNum = 0;
	
	if(G_CreatingMFOffset == 0xFFFFFFFF)//profileHeader还没有来,就来了GFMPEProcess,MF_TEMPLATE_E08会测到
	{
		appendErrorResponse(INVALID_REQUEST_FORMAT,0);
		return FALSE;
	}

	if((ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == FALSE) && (G_PETlvInfo.bufferLen < 0x0F))//第一次进来，且前面的数据太短，直接备份，返回
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);	
		
		return TRUE;
	}
	
	
	if(G_PEControlContext.C4RemainLen > G_PETlvInfo.bufferLen)
	{
		backFileAddr = G_fileControlContext.LoadEFAddr;//映射文件会改变G_CurrentEfAddr[G_ChannelNum]，因此需要先备份起来
		
		if((GetFileHeadByte(G_fileControlContext.LoadEFAddr,F_Status_Offset)&F_MAP_FILE) == F_MAP_FILE)// 如果更新的是link 文件，则实际更新的是实体文件
		{
			G_fileControlContext.LoadEFAddr = GetParentOrMapFileAddr(G_fileControlContext.LoadEFAddr, F_MapFile_Offset);
			G_fileControlContext.CurrentFileFileRecLen	=  GetFileHeadByte(G_fileControlContext.LoadEFAddr,F_RecLgth_Offset);
		}
		
		if(G_fileControlContext.CurrentFileStructure == TRANTYPE)
		{
			if(profileUpdateBinary(G_fileControlContext.UpdateOffset,(BYTE*)(G_PETlvInfo.bufferAddr),G_PETlvInfo.bufferLen) == FALSE)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
		}
		else if((G_fileControlContext.CurrentFileStructure == LINTYPE) || (G_fileControlContext.CurrentFileStructure == CYCTYPE)) //LF/CF
		{
			if(profileUpdateRecord((G_fileControlContext.UpdateOffset / G_fileControlContext.CurrentFileFileRecLen)+1,(G_fileControlContext.UpdateOffset % G_fileControlContext.CurrentFileFileRecLen),(BYTE*)(G_PETlvInfo.bufferAddr),G_PETlvInfo.bufferLen) == FALSE)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
		}
		else
		{
			return TRUE;
		}
		
		G_fileControlContext.LoadEFAddr = backFileAddr;
		
		G_fileControlContext.UpdateOffset += G_PETlvInfo.bufferLen;
		G_PEControlContext.C4RemainLen -= G_PETlvInfo.bufferLen;
		G_profileControlContext.TotalLen += G_PETlvInfo.bufferLen;
		
		return TRUE;
	}
	else if(G_PEControlContext.C4RemainLen > 0)
	{
		backFileAddr = G_fileControlContext.LoadEFAddr;//映射文件会改变G_CurrentEfAddr[G_ChannelNum]，因此需要先备份起来
		
		if((GetFileHeadByte(G_fileControlContext.LoadEFAddr,F_Status_Offset)&F_MAP_FILE) == F_MAP_FILE)// 如果更新的是link 文件，则实际更新的是实体文件
		{
			G_fileControlContext.LoadEFAddr = GetParentOrMapFileAddr(G_fileControlContext.LoadEFAddr, F_MapFile_Offset);
			G_fileControlContext.CurrentFileFileRecLen	=  GetFileHeadByte(G_fileControlContext.LoadEFAddr,F_RecLgth_Offset);
		}	
		
		if(G_fileControlContext.CurrentFileStructure == TRANTYPE)
		{
			if(profileUpdateBinary(G_fileControlContext.UpdateOffset,(BYTE*)(G_PETlvInfo.bufferAddr),G_PEControlContext.C4RemainLen) == FALSE)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
		}
		else if((G_fileControlContext.CurrentFileStructure == LINTYPE) || (G_fileControlContext.CurrentFileStructure == CYCTYPE)) //LF/CF
		{
			if(profileUpdateRecord((G_fileControlContext.UpdateOffset / G_fileControlContext.CurrentFileFileRecLen)+1,(G_fileControlContext.UpdateOffset % G_fileControlContext.CurrentFileFileRecLen),(BYTE*)(G_PETlvInfo.bufferAddr),G_PEControlContext.C4RemainLen) == FALSE)
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
		}
		else
		{
			return TRUE;
		}
		
		G_fileControlContext.LoadEFAddr = backFileAddr;
		
		G_fileControlContext.UpdateOffset += G_PEControlContext.C4RemainLen;
		G_profileControlContext.TotalLen += G_PEControlContext.C4RemainLen;
		//totalLen += (G_fileControlContext.UpdateOffset + G_PETlvInfo.findTagLen + G_PETlvInfo.findLenLen);
		initTlvInfo(&G_PETlvInfo,G_PETlvInfo.bufferAddr+G_PEControlContext.C4RemainLen,G_PETlvInfo.bufferLen-G_PEControlContext.C4RemainLen);
		G_PEControlContext.C4RemainLen = 0;
		G_PETlvInfo.levelIndex = 2;	
	}

	if(ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == FALSE)//第一次进来
	{
	   SetProfileChannel(G_CreatingMFOffset,DF_MF);	                    //The default selection at the beginning of this PE is as follows:Current DF: MF,Current EF: no selection
	   SetBit(G_PEControlContext.IsExistFlag2,OccurFlag);
	}
    else
	{
		tempRec = VMReadByte(G_PETlvInfo.bufferAddr);
	
		if((tempRec == 0x80) || (tempRec == 0x62) || (tempRec == 0x81) || (tempRec == 0x02))//恰好在值的结束处断开
		{
			G_PETlvInfo.levelIndex = 2;
		}

#if PROFILE_GENERATE_CREATEFILE_MORE30

		if(tempRec == 0x30)
		{
			G_PETlvInfo.levelIndex = 1;
		}
#endif

	}		

	
	if(G_PETlvInfo.levelIndex == 2)
	{
		G_PETlvInfo.lenBuffer[0] = 0;
		G_PETlvInfo.lenBuffer[1] = 0;
		if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == TRUE)//这是最后一批A1数据
		{
			G_PETlvInfo.findNextLen = G_PETlvInfo.valueLenBuffer[1]-G_profileControlContext.TotalLen;

#if PROFILE_GENERATE_CREATEFILE_MORE30
			if((G_PETlvInfo.valueLenBuffer[0] - G_PETlvInfo.valueLenBuffer[1]) > 4)//包含有多个0x30
			{
				G_PETlvInfo.lenBuffer[1] = G_PETlvInfo.bufferLen - G_PETlvInfo.findNextLen;//逐层回退时，需要给第一层级的nextLen赋值
				G_PETlvInfo.offBuffer[1] = G_PETlvInfo.valueLenBuffer[1] - G_profileControlContext.TotalLen; //逐层回退时，需要给第一层级的nextOff赋值
			}
#endif

		}
		else//此时A1还没有来完整，中间的A1数据
		{
#if PROFILE_GENERATE_CREATEFILE_MORE30
			if((G_PETlvInfo.valueLenBuffer[0] - G_PETlvInfo.valueLenBuffer[1]) > 4)//包含有多个0x30
			{
				if((G_PETlvInfo.valueLenBuffer[1]-G_profileControlContext.TotalLen) >= G_PETlvInfo.bufferLen)//30下面有多个81，02的情况，此时来的数据只是30的一部分
				{
					G_PETlvInfo.findNextLen = G_PETlvInfo.bufferLen;
				}
				else//此次来的是这个30的最后一部分数据
				{
					G_PETlvInfo.findNextLen = G_PETlvInfo.valueLenBuffer[1]-G_profileControlContext.TotalLen;
					G_PETlvInfo.lenBuffer[1] = G_PETlvInfo.bufferLen - G_PETlvInfo.findNextLen;//逐层回退时，需要给第一层级的nextLen赋值
					G_PETlvInfo.offBuffer[1] = G_PETlvInfo.findNextLen;  //逐层回退时，需要给第一层级的nextOff赋值
				}
			}
			else//标准结构，只包含一个0x30
#endif
			{
				G_PETlvInfo.findNextLen = G_PETlvInfo.bufferLen;
			}
		}
		
	}
		
	result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);	
	
	
  while(result)
  {
//		ecpMem_Set(tempbuffer,0,sizeof(tempbuffer));
		if((G_PETlvInfo.levelIndex)==0)
		{
			if(G_PETlvInfo.tagBuffer[0] != 0xA0)
			{
				if( G_PEControlContext.TagA0Occ == 0)
				{
					appendErrorResponse(INVALID_PARAMETER,0);
					return FALSE;
				}	
			}
			
			if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_thePEHeader)
			{
				if(CheckTagOcc(G_PEControlContext.TagA0Occ++) == FALSE)
				{
					return FALSE;
				}
				
				if (PEHeaderAnalyze()== FALSE)  //处理PEHeader信息
				{
					return FALSE;
				}				
			} 
			else if(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_theGFMPEfileManagementCMD)
			{
				setVerticalFind(&G_PETlvInfo);
			}
			else
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
		}
		else if((G_PETlvInfo.levelIndex)==1)
		{
			if(G_PETlvInfo.tagBuffer[0] == Tag_theGFMPEfileManagementCMD)
			{
				if (G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_fileManagementListTag)
				{
					setVerticalFind(&G_PETlvInfo);
					
					G_TempLen = 0;
					
					G_profileControlContext.TotalLen = 0;
				} 
			}
			else
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					return FALSE;
			}
		}
		else if((G_PETlvInfo.levelIndex)==2)
		{
			if((G_PETlvInfo.tagBuffer[0] == Tag_theGFMPEfileManagementCMD)&&(G_PETlvInfo.tagBuffer[1] == Tag_fileManagementListTag))
			{	
				if((G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_filePath) || (G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_createFCP) || (G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex] == Tag_fillFileOffset))								
				{
					if(G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == FALSE)//处理的tag不完整
					{
						backData( EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findTagOff,G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff);	
		
						return TRUE;
					}
				}				
				
				if(G_PETlvInfo.isCompleteBuffer[G_PETlvInfo.levelIndex] == TRUE)
				{
					G_profileControlContext.TotalLen += G_PETlvInfo.findTLVLen;
				}
				else
				{
					G_profileControlContext.TotalLen += (G_PETlvInfo.bufferLen-G_PETlvInfo.findTagOff);
				}
			    				
				switch(G_PETlvInfo.tagBuffer[G_PETlvInfo.levelIndex])
				{
					case Tag_filePath:	
						
						if((G_PETlvInfo.findValueLen % 2) != 0)//长度必须是偶数
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
							return FALSE;
						}	
						
						if(profileSelectByPath((BYTE*)(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff),G_PETlvInfo.findValueLen) == FALSE)
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
					
							return FALSE;
						}					
						break;
					
					case Tag_createFCP:					
						setVerticalFind(&G_PETlvInfo);
																		
						G_TempLen = 0;
						
						if(appendFindValue(&G_PETlvInfo,FromValue) == FALSE)
						{
							return FALSE;
						}

						if(replaceGivenTLV() == FALSE)
						{
							appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
					
							return FALSE;
						}
						
						 pos = FindTag(tempBuf,Tag_fDept,G_TempLen); //tag 82
					   if(pos != 1)//HB_Offline_Profile_Test/Profile_FulSeg0047,when create adf,82,83tag should in order
						 {
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						
								return FALSE;
						 }

						makeDescript();
						pos = FindTagOffset(tempBuf,Tag_fId,G_TempLen,1);
						if(BuildWord(tempBuf+pos+1) == 0x3F00)
						{
							if (profileModifyMFInfo(tempBuf,G_TempLen) == FALSE) //resize上一个File中的文件，即resize MF文件，因为MF文件在安装ISD-P的时候已经创建
							{
								appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
								return FALSE;
							}
							SetBit(G_profileControlContext.IsCreateFlag,CREATE_MF);
						}
						else
						{
							pos = FindTag(tempBuf,0x84,G_TempLen);

							if(pos != FALSE)
							{
								tempRec = GetAIDNumber((tempBuf+pos),TRUE); //2F00 里面没有的ADF，主动添加进去
								if(tempRec == FALSE) //为了应付中国电信甜瑜离线脚本测试，CSIM ADF在2F00里面没有导致选择CSIM失败的问题
								{
									recNum = WriteDIRNum(tempBuf+pos);
								}
							}

							tempRec = profileCreateFile(tempBuf,G_TempLen,PROFILE_MODE_GENERIC);
							
							if(processFileResult(tempRec) == FALSE)
							{
								return FALSE;
							}
							
							if(recNum != 0)
							{
								VMSetBuf(G_fileControlContext.LoadDFAddr + F_ShordFid_Offset,recNum,sizeof(BYTE));
								recNum = 0;
							}

						}
						

						if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_USIM) == FALSE)
						{
							if(PESearchAIDName((BYTE *)&USIMPartial[0]) == TRUE)
							{
								SetBit(G_profileControlContext.IsCreateFlag,CREATE_USIM);
							}
						}
								
						G_fileControlContext.UpdateOffset = 0;

						break;
					
					case Tag_fillFileOffset://改变updateOff的值	
                        
						if(G_PETlvInfo.findValueLen > 2)
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);//fillFileOffset UInt16,
					
							return FALSE;
						}
						
					    if(G_PETlvInfo.findValueLen == 2)
						{
					    	G_fileControlContext.UpdateOffset += VMReadShort(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
						}
						else
						{
							G_fileControlContext.UpdateOffset += VMReadByte(G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff);
						}
						break;
										
					case Tag_fillFileContent: //对LF/CF/TF文件进行更新		
						temp = makeUpdate();
						if((temp & 0x0F) == FALSE)//数据不匹配错误/数据更新错误 HB_Offline_Profile_Test==>14-PROFILE : Profile_FulSeg0025
						{
							appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
#if RSP_EUICC
							if((temp & 0x80) == 0x80)
							{
								G_ProfileErrorReason = ErrorReason_installFailedDueToDataMismatch;
							}							
#endif
							return FALSE;
						}

						break;	

					default:
						appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
						return FALSE;
						//break;
				}
			}
			else
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification);
				return FALSE;
			}
		}

		result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);
		
		if((result == FALSE) && (G_PETlvInfo.findNextLen > 0))//数据在tag或者长度处断开，直接备份返回
		{
			backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff, G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
	
			return TRUE;
		}
			
		while (result == FALSE)
		{
			if(setReturnFind(&G_PETlvInfo) == FALSE)
			{
				break;
			}
			
			result = findTLV(&G_PETlvInfo,0xFF,0xFF,1);  

            if((result == FALSE) && (G_PETlvInfo.findNextLen > 0))//数据在tag或者长度处断开，直接备份返回
			{
				backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff,  G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
				
				return TRUE;
			}			
		}	
	}
	

	if(G_PETlvInfo.findNextLen > 0)
	{
		
		backData(EUICCBackPEBuf, G_PETlvInfo.bufferAddr + G_PETlvInfo.findNextOff, G_PETlvInfo.bufferLen-G_PETlvInfo.findNextOff);	
		
		return TRUE;
	}
			    

	if(G_profileControlContext.ProfileVersion >= 0x22)//profile2.2规范才做此检查
	{
		//if(G_fileControlContext.LoadADFAddr != 0)//"PIN Context" of the previous PE Template or PE Generic File Management  is ADF
		//{
		G_fileControlContext.LoadADfId = G_PEControlContext.PE_identification;
		//}
	}
	
	return TRUE;
}

#endif

