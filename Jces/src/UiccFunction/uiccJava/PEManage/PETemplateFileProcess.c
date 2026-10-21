
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
#include  "Uicc_interface.h"
#include "DataOrientedCommand.h"

#if (EUICC_ONLY||SE_AND_EUICC)

/*
*@function     :UnknownPETag
*
*@description  :无法识别的PEtag
*
*@see          :
*
*@param        :void
*
*@return       :void
*
*@test         :
*
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */

BYTE UnknownPETag()
{
	appendErrorResponse(PE_NOT_SUPPORTED,0);

	return FALSE;
}
/*
*@function     :checkLFFileContent
*
*@description  :检查线性文件是否有更新
*
*@see          :
*
*@param        :BYTE tempId,
*@param        WORD  filename
*
*@return       :TRUE：文件内容合法
*@return       FALSE：文件内容是默认值，文件没有更新
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
/*BYTE checkLFFileContent(BYTE tempId,WORD  filename)
{
	UWORD xdata fileAddr;
	BYTE idata mandatory,fileSize,i,j,num,recLgth;

		mandatory = TRUE;

		switch(tempId)
		{
			case MF_TEMPLATE_LAST_ID:
				fileAddr = G_CreatingMFOffset;
				break;
			case CD_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,0x7F11);
				break;
			case TELECOM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,0x7F10);
				break;
			case USIM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_USIM);
				break;
			case OPT_USIM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_USIM);
				mandatory = FALSE;
				break;
			case ISIM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_ISIM);
				break;
			case OPT_ISIM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_ISIM);
				mandatory = FALSE;
				break;
			case CSIM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_CSIM);
				break;
			case OPT_CSIM_TEMPLATE_LAST_ID:
				fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_CSIM);
				mandatory = FALSE;
				break;
		}

	fileAddr = SelectLFFile(fileAddr,filename);
	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	num = GetLFRecNumLgth(fileAddr,FileNum);

	recLgth = GetLFRecNumLgth(fileAddr, FileLgth);

	for(i=1;i<= num;i++)
	{
		ReadLFBuf(fileAddr,i,0x00,RecData,recLgth);

		for(j =0; j< recLgth; j++)
		{
			if(RecData[j] != 0xFF)
			{
				return TRUE;
			}
		}
	}

	return FALSE;

}
/*
*@function     :checkTFFileContent
*
*@description  :检查透明文件是否有更新
*
*@see          :
*
*@param        :BYTE tempId,
*@param        WORD  filename
*
*@return       :TRUE：文件内容合法
*@return       FALSE：文件内容是默认值，文件没有更新
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
/*
BYTE checkTFFileContent(BYTE tempId,WORD  filename)
{
	UWORD xdata fileAddr;
	BYTE idata mandatory,fileSize,i;

	mandatory = TRUE;

	switch(tempId)
	{
		case MF_TEMPLATE_LAST_ID:
			fileAddr = G_CreatingMFOffset;
			break;
		case CD_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,0x7F11);
			break;
		case TELECOM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,0x7F10);
			break;
		case USIM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_USIM);
			break;
		case OPT_USIM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_USIM);
			mandatory = FALSE;
			break;
		case ISIM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_ISIM);
			break;
		case OPT_ISIM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_ISIM);
			mandatory = FALSE;
			break;
		case CSIM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_CSIM);
			break;
		case OPT_CSIM_TEMPLATE_LAST_ID:
			fileAddr = SelectFile(G_CreatingMFOffset,G_DF_ID_CSIM);
			mandatory = FALSE;
			break;
	}

	fileAddr = SelectTFFile(fileAddr,filename);
	if(fileAddr == FALSE)
	{
		if(mandatory == TRUE)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);

	ReadTFBuf(fileAddr,0,&RecData[0],fileSize);//todo:fileSize > sizeof(RecData)

	for(i=0;i<= fileSize;i++)
	{
		if(RecData[i] != 0xFF)
		{
			return TRUE;
		}
	}
	return FALSE;

}

BYTE CheckTemplateFile(BYTE tempId,FLASH TemplateFileList * templateFileList)
{
	BYTE idata i;

	for(i=0; ; i++)
	{
		if(templateFileList[i].filename == EOL)
		{
			return TRUE;
		}
		else if(templateFileList[i].fileType == TRANTYPE )
		{
			if(FALSE == checkTFFileContent(tempId,templateFileList[i].filename))
			{
				return FALSE;
			}
			continue;

		}
		else if(templateFileList[i].fileType == LINTYPE )
		{
			if(FALSE == checkLFFileContent(tempId,templateFileList[i].filename))
			{
				return FALSE;
			}
			continue;
		}
		else if(templateFileList[i].fileType == CYCTYPE )
		{

		}
	}
}*/

/*
*@function     :TemplatePreProcess
*
*@description  :模板方式创建文件前处理
*
*@see          :
*
*@param        :BYTE tempId
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
*@hisotry      V1.3  : modify by zhangxiaoxiao @20220531 修改更新文件大小超过缓存文件导致profile下载失败的bug
* */

BYTE TemplatePreProcess(BYTE tempId)
{
	BYTE idata result;
	WORD idata tempParameter,updateDataLen,usedSize;
	UWORD idata backFileAddr;

	if((ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == FALSE) && (ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == FALSE) && (G_PETlvInfo.bufferLen < 0x0F))//来的数据不是完整的，且前面的数据太短，直接备份，返回，TY_OfflineProfileFormatTest/HEAD\异常测试/HEAD_E01很短，但是是完整的，期望不保存，继续执行
	{
		backData(EUICCBackPEBuf,G_PETlvInfo.bufferAddr,  G_PETlvInfo.bufferLen);

		return 0xFF;
	}


	if(G_PEControlContext.C4RemainLen > G_PETlvInfo.bufferLen)  //modify by zhangxiaoxiao @20220531 修改更新文件大小超过缓存文件导致profile下载失败的bug
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
				return FALSE;
			}
		}
		else if((G_fileControlContext.CurrentFileStructure == LINTYPE) || (G_fileControlContext.CurrentFileStructure == CYCTYPE)) //LF/CF
		{
			if(profileUpdateRecord((G_fileControlContext.UpdateOffset / G_fileControlContext.CurrentFileFileRecLen)+1,(G_fileControlContext.UpdateOffset % G_fileControlContext.CurrentFileFileRecLen),(BYTE*)(G_PETlvInfo.bufferAddr),G_PETlvInfo.bufferLen) == FALSE)
			{
				return FALSE;
			}
		}
#if UICC__FUNCTION__BERTLVCmd
	else if(G_fileControlContext.CurrentFileStructure== BERTLVTYPE)
	{
		WriteBERBuf(G_fileControlContext.LoadEFAddr,G_fileControlContext.UpdateOffset,(BYTE*)(G_PETlvInfo.bufferAddr),G_PETlvInfo.bufferLen);

		usedSize = GetFileHeadWord(G_fileControlContext.LoadEFAddr, F_BERTLV_UsedSize);

		usedSize += G_PETlvInfo.bufferLen;

		WritePageCacheEX(G_fileControlContext.LoadEFAddr+F_BERTLV_UsedSize,(BYTE *)&usedSize,sizeof(WORD));
	}
#endif
		else
		{
			return 0xFF;
		}

		G_fileControlContext.LoadEFAddr = backFileAddr;

		G_fileControlContext.UpdateOffset += G_PETlvInfo.bufferLen;
		G_PEControlContext.C4RemainLen -= G_PETlvInfo.bufferLen;
		G_profileControlContext.TotalLen += G_PETlvInfo.bufferLen;

		return 0xFF;
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
				return FALSE;
			}
		}
		else if((G_fileControlContext.CurrentFileStructure == LINTYPE) || (G_fileControlContext.CurrentFileStructure == CYCTYPE)) //LF/CF
		{
			if(profileUpdateRecord((G_fileControlContext.UpdateOffset / G_fileControlContext.CurrentFileFileRecLen)+1,(G_fileControlContext.UpdateOffset % G_fileControlContext.CurrentFileFileRecLen),(BYTE*)(G_PETlvInfo.bufferAddr),G_PEControlContext.C4RemainLen) == FALSE)
			{
				return FALSE;
			}
		}
#if UICC__FUNCTION__BERTLVCmd
	else if(G_fileControlContext.CurrentFileStructure== BERTLVTYPE)
	{
		WriteBERBuf(G_fileControlContext.LoadEFAddr,G_fileControlContext.UpdateOffset,(BYTE*)(G_PETlvInfo.bufferAddr),G_PEControlContext.C4RemainLen);

		usedSize = GetFileHeadWord(G_fileControlContext.LoadEFAddr, F_BERTLV_UsedSize);

		usedSize += G_PEControlContext.C4RemainLen;

		WritePageCacheEX(G_fileControlContext.LoadEFAddr+F_BERTLV_UsedSize,(BYTE *)&usedSize,sizeof(WORD));
	}
#endif
		else
		{
			return 0xFF;
		}

		G_fileControlContext.LoadEFAddr = backFileAddr;

		G_fileControlContext.UpdateOffset += G_PEControlContext.C4RemainLen;
		G_profileControlContext.TotalLen += G_PEControlContext.C4RemainLen;
		//totalLen += (G_UpdateOffset + G_PETlvInfo.findTagLen + G_PETlvInfo.findLenLen);
		initTlvInfo(&G_PETlvInfo,G_PETlvInfo.bufferAddr+G_PEControlContext.C4RemainLen,G_PETlvInfo.bufferLen-G_PEControlContext.C4RemainLen);
		G_PEControlContext.C4RemainLen = 0;
		G_PETlvInfo.levelIndex = 0;
	}



	if(ValueBit(G_PEControlContext.IsExistFlag2,OccurFlag) == TRUE)
	{
		result = VMReadByte(G_PETlvInfo.bufferAddr);

		if((result == Tag_thefileDescriptor) || (result == Tag_thefillFileOffset) || (result == Tag_thefillFileContent))//恰好在值的结束处断开
		{
			G_PETlvInfo.levelIndex = 1;
		}
	}
	else//第一次进来，设置已经进来的标记
	{
		SetBit(G_PEControlContext.IsExistFlag2,OccurFlag);

		getPEID();

		if((tempId == OPT_USIM_TEMPLATE_LAST_ID)||(tempId == PHONEBOOK_TEMPLATE_LAST_ID) ||(tempId == GSM_ACCESS_TEMPLATE_LAST_ID))
		{
			if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_USIM) == FALSE)
			{
				return FALSE;
			}
		}

		if(tempId == USIM_TEMPLATE_LAST_ID)
		{
		  SetBit(G_profileControlContext.IsCreateFlag,CREATE_USIM);
		}
		if(tempId == OPT_USIM_TEMPLATE_LAST_ID)
		{
			if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_OPTUSIM) == TRUE)
			{
				return FALSE;
			}
			 SetBit(G_profileControlContext.IsCreateFlag,CREATE_OPTUSIM);
		}
		if(tempId == CD_TEMPLATE_LAST_ID)
		{
			if(ValueBit(G_profileControlContext.IsCreateFlag,CREATE_DF_CD) == TRUE)
			{
				return FALSE;
			}
			 SetBit(G_profileControlContext.IsCreateFlag,CREATE_DF_CD);
		}

		if(tempId < OPT_USIM_TEMPLATE_LAST_ID)
		{
			SetProfileChannel(G_CreatingMFOffset,DF_MF);
		}
		else if((tempId < ISIM_TEMPLATE_LAST_ID) || (tempId == DF5GS_TEMPLATE_LAST_ID) || (tempId == SAIP_TEMPLATE_LAST_ID) || (tempId == EAP_TEMPLATE_LAST_ID))  //5: Optional USIM EFs     6: 9.5.3 DF Phonebook     7: 9.5.4 DF GSM-ACCESS
		{
			if(PESearchAIDName((BYTE *)&USIMPartial[0]) == FALSE)
			{
				return FALSE;
			}

			VMReadBuf((BYTE *)&tempParameter,G_fileControlContext.LoadADFAddr,sizeof(WORD));

			SetProfileChannel(G_fileControlContext.LoadADFAddr,tempParameter);
		}
		else if((tempId == ISIM_TEMPLATE_LAST_ID)||(tempId == CSIM_TEMPLATE_LAST_ID))
		{
			SetProfileChannel(G_CreatingMFOffset,DF_MF);
		}
		else if(tempId == OPT_ISIM_TEMPLATE_LAST_ID)  //9.6.2 Optional ISIM EFs
		{
			if(PESearchAIDName((BYTE *)&ISIMPartial[0]) == FALSE)
			{
				return FALSE;
			}

			VMReadBuf((BYTE *)&tempParameter,G_fileControlContext.LoadADFAddr,sizeof(WORD));

			SetProfileChannel(G_fileControlContext.LoadADFAddr,tempParameter);
		}
		else if(tempId == OPT_CSIM_TEMPLATE_LAST_ID)
		{
			if(PESearchAIDName((BYTE *)&CSIMPartial[0]) == FALSE)
			{
				return FALSE;
			}

			VMReadBuf((BYTE *)&tempParameter,G_fileControlContext.LoadADFAddr,sizeof(WORD));

			SetProfileChannel(G_fileControlContext.LoadADFAddr,tempParameter);
		}


		if((tempId == OPT_USIM_TEMPLATE_LAST_ID)||(tempId == OPT_ISIM_TEMPLATE_LAST_ID) ||(tempId == OPT_CSIM_TEMPLATE_LAST_ID))
		{
			 ClearBit(G_PEControlContext.IsExistFlag2,DFFlag);
		}
	}

	if(G_PETlvInfo.levelIndex == 1)
	{
		tempParameter = G_PETlvInfo.valueLenBuffer[0]-G_profileControlContext.TotalLen; //层级1的剩余长度
		if(tempParameter > G_PETlvInfo.findNextLen)
		{
			G_PETlvInfo.lenBuffer[0] = 0;
			G_PETlvInfo.offBuffer[0] = 0;
		}
		else
		{
			G_PETlvInfo.lenBuffer[0] = G_PETlvInfo.findNextLen-tempParameter;
			G_PETlvInfo.offBuffer[0] = tempParameter;
			G_PETlvInfo.findNextLen = tempParameter;
		}
	}
	return TRUE;
}
/*
*@function     :MFTemplateProcess
*
*@description  :模板方式创建MF
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.2 Files at MF level
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */

BYTE MFTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(MF_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :CDTemplateProcess
*
*@description  :模板方式创建CD
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.3 DF CD
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */

BYTE CDTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(CD_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :TelTemplateProcess
*
*@description  :模板方式创建TELECOM
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.4 DF TELECOM
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE TelTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(TELECOM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :UsimTemplateProcess
*
*@description  :模板方式创建USIM
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.5.1 Mandatory USIM EFs
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE UsimTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(USIM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :OptialUsimTemplateProcess
*
*@description  :模板方式创建OptialUsim
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.5.2 Optional USIM EFs
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE OptialUsimTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(OPT_USIM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :IsimTemplateProcess
*
*@description  :模板方式创建Isim
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.6.1 Mandatory ISIM EFs
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE IsimTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(ISIM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :OptialIsimTemplateProcess
*
*@description  :模板方式创建OptialIsim
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.6.2 Optional ISIM EFs
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE OptialIsimTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(OPT_ISIM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :phonebookTemplateProcess
*
*@description  :模板方式创建PhoneBook
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.5.3 DF Phonebook
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE phonebookTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(PHONEBOOK_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :GSMAccemplateProcess
*
*@description  :模板方式创建GSMAEECSS
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.5.4 DF GSM-ACCESS
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE GSMAccemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(GSM_ACCESS_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :CsimTemplateProcess
*
*@description  :模板方式创建SCIM
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.7.1 Mandatory CSIM EFs
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE CsimTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(CSIM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :OptialCsimTemplateProcess
*
*@description  :模板方式创建OptialCsim
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.7.2 Optional CSIM EFs
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE OptialCsimTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(OPT_CSIM_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :eapTemplateProcess
*
*@description  :模板方式创建EAP
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.8 EAP
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE eapTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(EAP_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :df5gsTemplateProcess
*
*@description  :模板方式创建DF5GS
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.5.11 DF 5GS
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE df5gsTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(DF5GS_TEMPLATE_LAST_ID);

	return result;
}
/*
*@function     :SaipTemplateProcess
*
*@description  :模板方式创建SAIP
*
*@see          :<eUICC Profile Package:Interoperable Format Technical Specification Version 2.3.1>-9.5.12 DF SAIP
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210625
*
*@hisotry      :created by chentao@20210625
* */
BYTE SaipTemplateProcess(void)
{
	BYTE idata result;

	result = PEFileSystemProcess(SAIP_TEMPLATE_LAST_ID);

	return result;
}

/*
*@function     :TemplatePEEndProcess
*
*@description  :模板方式创建文件结束后处理
*
*@see          :
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210628
*
*@hisotry      :created by chentao@20210628
*@hisotry      :modify by zhxx@20220629 HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg00b3：B9(CSIM)下面没有A6、AE(CSIM模板强制性文件)所以抛异常
* */
BYTE TemplatePEEndProcess(BYTE tempId)
{
	if(ValueBit(G_PEControlContext.IsExistFlag2,PE_Complete) == TRUE)//一个完整的PE执行结束
	{

		if(G_PEControlContext.CurTag == 0)
		{
			appendErrorResponse(INVALID_PARAMETER,G_PEControlContext.PE_identification);
			return FALSE;
		}

		if (contentRequiredCheck() == FALSE)
		{
			appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); //modify by chentao@20211118 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data\11-DF-PE-5GS\\09-PE未提供没有缺省值的参数  期望返回INVALID_REQUEST_FORMAT
			return FALSE;
		}

		if(tempId == CSIM_TEMPLATE_LAST_ID)
		{
			if((ValueBit(G_PEControlContext.IsExistFlag3,A6Tag) == FALSE) || (ValueBit(G_PEControlContext.IsExistFlag3,AETag) == FALSE))
			{
				appendErrorResponse(INVALID_REQUEST_FORMAT,G_PEControlContext.PE_identification); //add by zhxx   20220629 HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg00b3：B9(CSIM)下面没有A6、AE(CSIM模板强制性文件)所以抛异常
				return FALSE;
			}
		}

		if((tempId == MF_TEMPLATE_LAST_ID) || (tempId == USIM_TEMPLATE_LAST_ID) || (tempId == ISIM_TEMPLATE_LAST_ID) || (tempId == CSIM_TEMPLATE_LAST_ID))//CreatedByDefault
		{
			if(createTemplateAfterTagFile(G_PEControlContext.CurTag) == FALSE)
			{
				return FALSE; //错误信息已经拼装，这里不需要再拼装
			}
		}


		if(G_profileControlContext.ProfileVersion >= 0x22)//profile2.2规范才做此检查
		{
			//add by chentao@20211122 中国电信离线脚本测试--pofile结构测试库V2.3.1\\data\data_08_eUICCResponsetype_\\data_08_eUICCResponsetype_05
			if((tempId != OPT_USIM_TEMPLATE_LAST_ID) && (tempId != OPT_ISIM_TEMPLATE_LAST_ID) && (tempId != OPT_CSIM_TEMPLATE_LAST_ID) )
			{
				G_fileControlContext.LoadADfId = G_PEControlContext.PE_identification;
			}
		}
	}

	return TRUE;
}
/*
*@function     :contentRequiredCheck
*
*@description  :模板方式创建文件Content Required的检查
*
*@see          :eUICC Profile Package: Interoperable Format Technical Specification  Version 2.1
*@see          Content Required: If Yes, indicates that there is no default content for the file defined in the template
*@see                             and that it shall be provided when referencing the template in the Profile Package.
*
*@param        :void
*@param
*
*@return       :TRUE
*@return       FALSE
*
*@test         :
*@author       :chentao@20210701
*
*@hisotry      :created by chentao@20210701
* */
BYTE contentRequiredCheck(void)
{
	if(G_profileControlContext.ProfileVersion == 0x21)  //Profile2.1 版本才做此检查
	{
		if(ValueBit(G_PEControlContext.IsExistFlag2,ContentRequired) == TRUE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/*
*@function     :getPEID
*
*@description  : 在模板处理前出错，这时候还没到PEHeader解析，没有获取到PEid，这个函数目的就是获取PEID以拼装错误信息
*
*@see
*
*@param        :void
*@param
*
*@return       :
*
*@test         :
*@author       :chentao@20210707
*
*@hisotry      :created by chentao@20210707
* */
void getPEID(void)
{
	BYTE idata i,len;

	findTLV(&G_PETlvInfo,0xFF,0xFF,1);

	VMReadBuf(tempBuf,G_PETlvInfo.bufferAddr+G_PETlvInfo.findValueOff, G_PETlvInfo.findValueLen);   //根据PEheader的结构，其数据体不会超过256 ，所以此处采用tempBuf一次读取处理

	len = 0;

	for(i=0; i< G_PETlvInfo.findValueLen; )  //Note: 此处代码没有考虑80 81 的顺序
	{
		if(tempBuf[i] == 0x81)  //The identification field is used to uniquely identify a PE within the profile. It will be used for error reporting to the sender of the profile.
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
		}

		i += (len +2);   //PEheader中的tag len均为1字节表示
	}

	initFindTlv(&G_PETlvInfo);
}
#endif


