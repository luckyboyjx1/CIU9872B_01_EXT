
/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :RFMFile.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :20167-13

*Description :该文件主要用于实现基于03.48所实现的RFM功能，后期可能 在此基础上扩展支持TS10225规范所要求的RFM 功能
*
*History     :
	
author    time      version    description
xuyz    2016/7/13   V1.0       创建该文件
************************************************************************************************************************************************************************************************/

#include "CompactManage.h"
#include "uicc_main.h"
#include "gsmpublic.h"
#include "fileAction.h"
#include "uicc_error.h"
#include "fileSystem.h"
#include "ToolkitRFM.h"
//#include "stkalert.h"
//#include "ToolkitAPIPublic.h"
#include "BIPModuleProcess.h"
#include "ExpandedManage.h"
//#include "STKRemoteAppManagement.h"
#include "HttpProcess.h"
#include "Interface_AnnexB.h"
//#include "STKCATRuntimeEnviorment.h"
//#include "uicc_interface.h"
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#include "TlsMainFlow.h"
#include "GPSCP81KeyManagement.h"
#endif
//#include "platform_api.h"
#include "opFunction.h"
//#include "profileApi.h"
#include "SCPInterface.h"
#if UICC__AUTH__CDMA__CAVE
#include "CDMA.h"
#endif

#if UICC__FUNCTION__CATTP
#include "CatTp.h"
#endif
#include "Interface_LTEFromBoot.h"
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
#if UICC__FUNCTION__RFM
INSLIST code remoteRfm_list[]= 
{ 
  {0xA4,SelectCommand},   //CASE 4 OR CASE 3 command ,返回9000时,为CASE 3 ,else为CASE 4指令
  {0xD6,UpdateBinary},
  {0xDC,UpdateRecord},
  {0xA2,SeekCommand},     //CASE 4 OR CASE 3 command, 返回9000时,为CASE 3 ,else为CASE 4指令
  {0x32,IncreaseCommand}, //CASE 4 command
  {0x20,VerifyChangeUnblockChv},
  {0x24,VerifyChangeUnblockChv},
  {0x26,ChvHandle},
  {0x28,ChvHandle},
  {0x2C,VerifyChangeUnblockChv},
  {0x04,FileStatusHandler},
  {0x44,FileStatusHandler},
  {0xB0,ReadBinary},    //case 2
  {0xB2,ReadRecord},    //case 2
#if NATIVE_CARD_UICC
  {0xD4,ResizeFile},							//CLA in RFM is 0xA0
  {0xE0,CreateFile},
  {0xE4,DeleteFile},
#endif  
#if UICC__FUNCTION__CATTP
   {0xEC,PushCommand},
#endif
		
#if UICC__AUTH__5G__SUSPEND
	{0x76,SuspendUicc},
#endif
	
#if UICC__AUTH__5G__SUPI
	{0x78,GetIdentity},
#endif

  {EOL,InsError},
};
#endif
/*
*@function: InitRemoteManagevariable
*
*@description :变量初始化，上电后第一条指令调用，防止变量未初始化就使用
*
*@param     :fileAddr
*@param     :action
*
*@return    :TRUE/FALSE
*
*@author    :xwh
*
*@history   :V1.0:create by xwh 20190305
*/
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
void InitRemoteManagevariable(void)
{
#if UICC_RFM_FILE_UPDATE_EVENT
	UWORD xdata updateFileAddr;
	
	updateFileAddr = SelectLFFile(DF_Toolkit_P,EF_RFMFileUpdate);
	
	if (updateFileAddr != FALSE)		////迁移自tpx2.0平台， 进入RFM时，清除EF_RFMFileUpdate文件内容
	{
		ClearFileConst(updateFileAddr,0x00);
	}
#endif	
	G_Source_Type = SourceType_NULL; //防止IF_SetHttpScriptState接口在NM_DecryptAndCalMac中还没初始化就使用
	G_SpecialRFM = FALSE;

	G_ErrorActionOffsetLgth[0] = 0;	
	G_ErrorActionOffsetLgth[1] = 0;	
}
#endif

/*
*@function: RFMAccessContion(keyStatus);
*
*@description :对于RFM 操作判断当前条件是否满足
*
*@param     :fileAddr
*@param     :action
*
*@return    :TRUE/FALSE
*
*@author    :xuyz
*
*@history   :V1.0:create by xuyz@20170724
*/
//BYTE RFMAccessContion(UWORD  fileAddr, BYTE action)
//{
//#if (EUICC_ONLY||SE_AND_EUICC)
//	BYTE xdata accessDomain[0x05]; 
//	BYTE xdata tmp[0x04];
//	BYTE idata aidLen,keyStatus;
//	WORD idata offset;
//	BYTE idata adminFlag,result;
//	
//	keyStatus = GetAcessPurview(fileAddr,action);

//	if(keyStatus == FALSE)
//	{
//		return FALSE;
//	}

//	fileAddr = SelectLFFile(DF_Toolkit_P,EF_RFM_TAR);
//	
//	ReadLFBuf(fileAddr,G_TARRecIndex[1],PEADFAid_Offset,&aidLen,0x01);
//	
//	adminFlag = FALSE;
//	
//	if((INS == 0xE0)||(INS == 0xE4)||(INS == 0xD4))  //CREATE FILE /DELETE FILE /RESIZE FILE
//	{
//		adminFlag = TRUE;
//	}
//	
//	
//	if(aidLen == 0x00)  //访问的事UICC access
//	{
//		offset = PEUICCAccessDomain_Offset;
//	}
//	else  //访问的是adf access
//	{
//		offset = PEAIDAccessDomain_Offst;
//	}
//	
//	ReadLFBuf(fileAddr,G_TARRecIndex[1],offset + adminFlag*0x05 ,accessDomain,0x05);
//	
//	if(accessDomain[0] == 0x00)
//	{
//		return FALSE;
//	}
//	else if(accessDomain[0] == 0x01)
//	{
//		if(accessDomain[1] == 0x00)   //full access	to fileSystem
//		{
//			if(keyStatus==0x97)
//			{
//				return FALSE;
//			}
//			
//			return TRUE;
//		}
//		else if(accessDomain[1] == 0xFF)  //No access to fileSystem
//		{
//			return FALSE;
//		}
//	}
//	else if(accessDomain[0] == 0x04)
//	{
//		if(accessDomain[1] == 0x02)
//		{
//			if(keyStatus == 0x90)  //always
//			{
//				return ValueBit(accessDomain[4],2);	
//			}
//			else if(keyStatus <= 0x08)  //PIN1 :APP1 - APP8
//			{
//				return ValueBit(accessDomain[2],(keyStatus -1));
//			}
//			else if(((keyStatus&0xF0)==0xC0)||((keyStatus&0xF0)==0xD0))	  //OR  or AND 
//			{
//				memset(tmp, 0x00,sizeof(tmp));

//				if((keyStatus&0x01) == 0x01)  //PIN1
//				{
//					tmp[0] = ValueBit(accessDomain[2],0);	
//				}

//				if((keyStatus&0x02) == 0x02)   //PIN2
//				{
//					tmp[1]  = ValueBit(accessDomain[4],3);	
//				}

//				if((keyStatus&0x04) == 0x04)  //ADM1
//				{
//					tmp[2]  = ValueBit(accessDomain[3],0);	
//				}

//				if((keyStatus&0x08) == 0x08)  //ADM2
//				{
//					tmp[3]  = ValueBit(accessDomain[3],1);	
//				}

//				for(result =0 ;result <4 ;result ++)
//				{
//					if(ValueBit(keyStatus,result))
//					{
//						if((keyStatus&0xF0)==0xC0)  //OR
//						{
//							if(tmp[result] == TRUE)
//							{
//								return TRUE;
//							}
//						}
//						else 
//						{
//							if(tmp[result] == FALSE)
//							{
//								return FALSE;
//							}
//						}
//					}
//				}

//				if((keyStatus&0xF0)==0xC0)  //OR
//				{
//					return FALSE;
//					
//				}
//				else 
//				{
//					return TRUE;
//				
//				}

//			}
//			else if(((keyStatus&0xF0)==0x80)&&(keyStatus<0x8A))	   //pin2
//			{
//				return ValueBit(accessDomain[4],3);
//			}   
//			else if((keyStatus&0x0F) >= 0x0A)	//0A-0E:ADM1- ADM5  8A-8E: ADM6 -ADM10
//			{
//				if((keyStatus >= 0x0A)&&(keyStatus < 0x0F))	 // A-E:ADM1 ADM2 ADM3 ADM4 ADM5
//				{
//					return ValueBit(accessDomain[3],(keyStatus -0x0A));	
//				}
//				else if(keyStatus == 0x8A)
//				{
//					return ValueBit(accessDomain[3],5);	
//				}
//				else  if(keyStatus == 0x8B)
//				{
//					return ValueBit(accessDomain[3],6);	
//				}
//				else  if(keyStatus == 0x8C)
//				{
//					return ValueBit(accessDomain[3],7);	
//				}
//				else  if(keyStatus == 0x8D)
//				{
//					return ValueBit(accessDomain[4],0);	
//				}
//				else  if(keyStatus == 0x8E)
//				{
//					return ValueBit(accessDomain[4],1);	
//				}			
//			}
//			else
//			{
//				return FALSE;
//			}
//		}
//		else
//		{
//			return FALSE;
//		}
//	}
//	else if(accessDomain[0] == 0x03)           //APDU access mechanism, 只用于 SIM API 模式
//	{
//		if(accessDomain[1] == 0x01)
//		{
//			if(keyStatus == 0x90)  //always
//			{
//				return ValueBit(accessDomain[3],0);	 //Byte 2, b1
//			}
//			else if(keyStatus <= 0x08)  //PIN1 :APP1 - APP8
//			{
//				return ValueBit(accessDomain[3],1); //Byte 2, b2
//			}
//			else if(((keyStatus&0xF0)==0x80)&&(keyStatus<0x8A))	   //pin2
//			{
//				return ValueBit(accessDomain[3],2); //Byte 2, b3
//			}   
//			else if((keyStatus&0x0F) >= 0x0A)	//0A-0E:ADM1- ADM5  8A-8E: ADM6 -ADM10
//			{
//				if((keyStatus >= 0x0A)&&(keyStatus <= 0x0D))	 // A-D:ADM1 ADM2 ADM3 ADM4 ,对应于SIM  ADM 0-3
//				{
//					return ValueBit(accessDomain[3],(keyStatus -0x06)); //Byte 2, b5-b8
//				}
//				else if(keyStatus == 0x0E) //E, ADM 5,对应于SIM  ADM 4
//				{
//					return ValueBit(accessDomain[2],0); //Byte 1, b1
//				}
//				else if((keyStatus >= 0x8A)&&(keyStatus <= 0x8E)) //8A-8E,ADM6 -ADM10,对应于SIM  ADM 5-9
//				{
//					return ValueBit(accessDomain[2],(keyStatus&0x0F)-0x09);	//Byte 1, b2-b6
//				}	
//			}
//		}
//	}	
//#else
//	BYTE  idata fileType;
//	WORD  idata keyStatus;            //为了支持或/与权限，keyStatus扩展为WORd类型

//	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
//	
//	keyStatus = GetFileHeadByte(fileAddr, F_ArrName_Offset);  
//	
//	if(fileType == ADFTYPE)
//	{
//		/*
//		Bit 0 : DELETE FILE (child)   - Not used
//		Bit 1 : CREATE FILE (EF creation)
//		Bit 2 : CREATE FILE (DF creation)
//		Bit 3 : DEACTIVATE FILE   -Not used
//		Bit 4 : ACTIVATE FILE     -Not used
//		Bit 5 : TERMINATE CARD USAGE (MF), TERMINATE DF  -Not used
//		Bit 6 : DELETE FILE (self)
//		*/
//		return TRUE;
//	}
//	else
//	{
//		/*
//		Bit 0 ：Read/Search    
//		Bit 1 ：Update
//		Bit 2 ：Delete
//		Bit 3 ：Deactivate
//		Bit 4 ：Activate
//		Bit 5 ：Increase
//		Bit 6 ：Resize 权限 ，如果EF文件时通过RFM 创建，则EF文件自动获取了RFM 下的Delete/Resize权限
//		*/
//		if(action == A_ResizeFile)
//		{
//			return ((keyStatus&0x40) == 0x40);
//		}
//		else if(action == A_DeleteFile)	
//		{
//			return ((keyStatus&0x04) == 0x04);
//		}	
//		else if(action == A_IncreaseFile)
//		{
//			return ((keyStatus&0x20) == 0x20);		
//		}
//		else
//		{
//			return ((keyStatus&action) == action);
//		}		
//	}
//#endif
//	return  FALSE ;
//}
/*
*@description :RFM中处理Read Binary操作
*
*@param       :void
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20160713
*/
/*
void RemoteReadBinary(void)
{
	BYTE idata actionTag;
	WORD idata fileLgth ;
	TranFileHead xdata transefFile;


	actionTag = ComputeBinary(A_ReadFile);

	if(actionTag == FALSE)
	{
		return ;
	}

	fileLgth = ((P1 << 8)&0xFF00 )|(P2 & 0x00FF);

	if(P3==0x00) //lgth是256个字节
	{	
		ReadEEPromCache((BYTE xdata *)&transefFile.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(TranFileHead));
	
		if(transefFile.SpaceSize <( fileLgth+0x100))  // 当剩余有效数据不足0x100时，返回剩余有效数据
		{
			P3 = transefFile.SpaceSize - fileLgth; 
			
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],P3);		
			
			GlobalSetStatus(0x9000);
		}
		else
		{
			GlobalSetStatus(0x9000);//POR时候读取
			//同步5g移动代码，一次返回不完所有数据时需要抛出状态字62F1, 用例：CMCC_RFM_RAM_test/RFM_TEST/移动RFM测试(3G)/5.2 REMOTE FILE MANAGEMENT FOR SIM/5.2.2 Coding of the command/5.2.2.2 Output command/OutputCommand_P300_ReadBinary_100_bytes_00
			if(JcvmConfig_GetOPerator() == OPERATOR_CHINAMOBILE)
			{
				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],0x80);

				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth+0x80,(BYTE *)&RecData[0x80],0x80);

				if(transefFile.SpaceSize > ( fileLgth+0x100))
				{
					GlobalSetStatus(0x62F1);
				}
			}
		}
		
	}
	else
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0x00],P3);
		GlobalSetStatus(0x9000);
	}				                                                                
}
*/
 /*@Function:   setCurDfNameAndAddrbyTAR                                            
 * 
 *@Descrip:    根据G_TARRecIndex的值设置G_CurrentDfName 和G_CurrentDfAddr
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      none      
 *              						                                                                                                  
 *@Return:     none
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180809
 *                        
 */
#if UICC__FUNCTION__RFM
BYTE setCurDfNameAndAddrbyTAR(void)
{
	UWORD xdata tmpfileAddr;
	
    WORD idata fileName;
	
	tmpfileAddr = SelectLFFile(DF_Toolkit_P,EF_RFM_TAR);
	
	ReadLFBuf(tmpfileAddr,G_TARRecIndex[1],PEADFAid_Offset,&tempBuf[20],0x11);
	
	if(tempBuf[20] == 0x00)  //当前访问的是UICC下的文件
	{
		G_CurrentDfName[G_ChannelNum] = DF_MF; 
		
		G_CurrentDfAddr[G_ChannelNum] = E_FILE_START;		
	}
	else     //当前访问的是某个ADF 下的文件
	{
		tmpfileAddr = SelectFileByName(E_FILE_START,&tempBuf[21],tempBuf[20]);
		
		if(tmpfileAddr == FALSE)    //SIM 时候
		{
			G_CurrentDfName[G_ChannelNum] = DF_MF; 
		
			G_CurrentDfAddr[G_ChannelNum] = E_FILE_START;
			
			return TRUE;
		}
			
		ReadEEPromCache((BYTE *)&fileName,tmpfileAddr,sizeof(WORD));
		
		G_CurrentDfName[G_ChannelNum] = fileName;
		
		G_CurrentDfAddr[G_ChannelNum] = tmpfileAddr;
		
		G_CurrentAdfName[G_ChannelNum] = fileName;	
	}
	return TRUE;
}
#endif
#if JAVA_CARD_UICC
/*
*@description:SCP80key更新时候 如果和当前使用的key会话一致  获取更新前的key
*            :需要在附加数据写到POR文件之前使用,保存的Key借用了POR文件SCP80Keyoffset位置开始,读取出来的Value，保存到ResData
*@param      :buff  缓存key
*@param      :mode 获取 kic  MODE:获取  kid   MODE:读取key到全局变量 ResData防止写附加数据时候冲掉keyvalue
*@param      :
*@return     :void
*
*@history    :create by LCZ @20190119
*            modify by xuweihua@20210625: 优化空间, \EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
*
*/
void GetSCP80KeyValueBeforeUpdate(BYTE *buff,BYTE mode)
{
#if UICC__FUNCTION__RAM
	BYTE offset;
	UWORD xdata keyAddr;
	
	if(mode ==MODE_KIC_KID)//获取KIC+KID 到Res
	{
		//keyAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);	
		//ReadTFBuf(keyAddr, SCP80Keyoffset, ResData, 68);  //缓存TLV结构 按照AES 32字节  最大68字节

		keyAddr = (UWORD)BIPModule_Send_ADDR_START;
		ReadEeprom(ResData, keyAddr+SCP80Keyoffset, 68);
	}
	
	else if(mode == MODE_KEY_KIC)  //获取kic  kic有的话 只会缓存在一开始位置   上行如果加密 肯定存在kic 和kid
	{
		if((ResData[0] == TYPE_KEY_KIC) && ((ResData[1]%8)==0x00))
		{
			memcpy(buff, &ResData[0x02], ResData[1]);								//拷贝value
		}
	}
	
	else //(mode ==MODE_KEY_KID)  //获取kid  (可能只有kid,或者kickid)    //如果上行只是算mac则可能只存在 kid
	{
		offset = 0x00;
		
		if(ResData[0] == TYPE_KEY_KIC)
		{
			offset = ResData[1] +2;  //到kid Tag位置
		}
		
		if((ResData[offset] ==TYPE_KEY_KID)&&((ResData[offset+1]%8)==0x00))
		{
			memcpy(buff, &ResData[offset+2], ResData[offset+1]);								//拷贝value
		}
	}
	return;
#endif
}
#endif
/*
*@description:处理一条RFM APDU(完整的),对于扩展格式 则是处理一条22 tag里面的APDU
*
*@param      :fileAddr:待处理数据的文件地址
*@param      :commandOffset: 命令数据在文件中的起始偏移
*@param      :dataLen  :待处理的命令数据的长度
*
*@return     :TRUE 执行成功/FALSE 执行失败
*
*@history    :create by LCZ @20181115
*
*/
#if UICC__FUNCTION__RFM
BYTE RFMApduProcess(UWORD fileAddr,WORD commandOffset, WORD dataLen)
{
	BYTE idata num,Len,curChannel;
//不需要电信宏，都要判断是否为0 通道	
//#ifdef CHINATELECOM
	// 00A4000C026F0701B0000002 实际值与期望值不同。实际值：026986期望值：026E00/026AXX/0268XX
	curChannel = CLA&0x03;

	//RFM只能在0 通道执行
	if(curChannel != 0)
	{
		SW1 = 0x6E;
		SW2 = 0x00;
		return CipheringError;	
	}
//#endif	
	if(G_CommandMode == Command_Mode_First)					//第一个APDU命令
	{
		if (CLA == 0xA0)
		{
			G_CommandMode = Command_Mode_2G;					//当前为2G操作模式
		}
		else 
		{
			G_CommandMode = Command_Mode_3G;					//当前为3G操作模式
		}	
	}
	
	G_ChannelNum = CLA&0x03;           //
	
	if( ((G_CommandMode == Command_Mode_2G)^(CLA==0xA0)) || (((CLA&0xFC) != 0x80)&&((CLA&0xFC) != 0x00)&&(CLA != 0xA0)))	 //CLA 不符合规范，返回 "6E00"
	{
		GlobalSetStatus(0x6E00);
		return CipheringError;
	}

	if (IF_RfmApduModeCheck())
	{
		return CipheringError;
	}

	if(G_Source_Type == SourceType_HTTPS)
	{
		if (G_SpecialRFM)
		{
			GlobalSetStatus(0x6985);
			return CipheringError;
		}
	}
	
	if(JcvmConfig_GetOPerator() == OPERATOR_CHINATELECOM)	
	{
		if ((G_CommandMode == Command_Mode_2G)&&(P2 != 0x00)&&(INS == 0xA4))
		{
			GlobalSetStatus(0x6B00);
			return CipheringError;
		}
	}	
	
	if(G_APDU[0] == G_APDU[1])  //CLA == INS ,退出
	{
		return FALSE;
	}
	if ((P3 != 0x00) && ((INS != 0xB0) && (INS != 0xB2) && (INS != 0xC0)))	//case3,case4		
	{
		if((P3 + 5)<=dataLen)   //数据完整
		{
			Len = P3;
		}
		else                   //2210 80F240021B 4F05A0000005595C024F53
		{
			Len = dataLen -5;
		}
		
		memset(&RecData[Len],0,256-Len);    //清除APDU后的数据
		
//		ReadTFBuf(fileAddr, commandOffset+5, (BYTE *)RecData, Len);		//获取数据
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(RecData, fileAddr + commandOffset+5, Len);
		}
		else
		{
			ReadTFBuf(fileAddr, commandOffset+5, RecData, Len);		//获取数据
		}
	}
	
	if((INS==0xB2)&&(P3==0x00))  //当P3 = 00时，读取整条数据长度
	{
		if(G_CurrentEfAddr[G_ChannelNum] != FALSE)
		{
			P3 = GetLFRecNumLgth(G_CurrentEfAddr[G_ChannelNum],FileLgth);	
		}
	}
	
	if(INS == 0xA4) // modify by yangjian@20221205
	{
			if( (G_CommandMode == Command_Mode_3G)&&(P1 == 0x04))  //SELECT不支持 P1 = 04的情况,中国电信备案脚本要求
			{
				SW1 = 0x6A;
				SW2 = 0x86;
				return POR_OK;
			}
	}
						
	for(num = 0; ; num ++)
	{		
		if((remoteRfm_list[num].ins == INS)||(remoteRfm_list[num].ins == EOL))
		{	
			if(remoteRfm_list[num].ins == EOL)  //INS 非法
			{
				InsError();
				
				return FALSE;
			}
						
			remoteRfm_list[num].func();

			break;
		}  
	}

	return POR_OK;	
}
#endif
/*
*@description:处理一条RAM APDU(完整的),对于扩展格式 则是处理一条22 tag里面的APDU
*
*@param      :fileAddr:待处理数据的文件地址
*@param      :commandOffset: 命令数据在文件中的起始偏移
*@param      :dataLen  :待处理的命令数据的长度
*
*@return     :TRUE 执行成功/FALSE 执行失败
*
*@history    :create by LCZ @20181115
*
*/
#if UICC__FUNCTION__RAM
BYTE RAMApduProcess(UWORD fileAddr,WORD commandOffset, WORD dataLen)
{
	WORD idata sw;
	BYTE idata currentP3;
	
	if(G_APDU[0] == G_APDU[1])  //CLA == INS ,退出
	{
		return CipheringError;
	}
	
// 	if( ((INS != 0xC0)&&(INS!=0xCA))||((INS == 0xCA)&&(P1==0xFF)&&(P2==0x1F)))        //GET DATA只有FF1F才接收数据		
// 	{	
// 		ReadTFBuf(fileAddr,commandOffset+5,&RecData[0],P3);
// 	}
	
	if(INS != 0xC0)        //GET DATA只有FF1F才接收数据	EUICC CA时候有其他TAG 带命令体 修改	
	{
		if((P3 + 5)<=dataLen)   //数据完整
		{
			currentP3 = P3;
		}
		else             //2210 80F240021B 4F05A0000005595C024F53
		{
			currentP3 = dataLen -5;
		}
		
		memset(&RecData[currentP3],0,256-currentP3);    //清除APDU后的数据
		
//		ReadTFBuf(fileAddr,commandOffset+5,&RecData[0],currentP3);
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(&RecData[0], fileAddr + commandOffset+5, currentP3);
		}
		else
		{
			ReadTFBuf(fileAddr,commandOffset+5,&RecData[0],currentP3);
		}
	}
	
	currentP3 = P3;    //备份P3  执行时候带mac会改变P3的值
	
	sw = NativeProcessRemoteAppManageAPDU();   //判断当前RAM APDU命令是否由native COS层直接处理
	
/*	if((INS == 0xD8)&&((P1&0x7F)!=0x00)&&(RecData[0]==G_RFMRAMKeyversion)) //更新的Key和当前使用的key一样 需要保存好旧的值
	{
		IF_SaveSCP80KeyValueBeforeUpdate();
	}

	if(sw == FALSE)
	{			
		sw = JcvmProcessRemoteAppManageAPDU();
	}*/

	GlobalSetStatus(sw);
	
	P3 = currentP3;
	
	return POR_OK;
}
#endif

#if UICC__FUNCTION__RAM
/*
*@return     :初始化是否成功
*
*@history    :create by LCZ @20191102
*                 :modify by xuxiaojun@20210222 扩展的RAM指令中，带有83TAG时，8050/8482指令建立的安全信息，可以跨短信使用。g_SCP02SDID在清会话时会被清掉
*/
/*                                                                                    
* @Function    :IF_ChinaTelecom_RealNetWorkSSD
*  
* @Discription :判断当前的SCP80是不是电信的实网测试 ，如果下行已经被scp80保护 返回TRUE
*
* @See         :
*                                                                                                                                                
* @Parameter   :BYTE sdId --- SCP80的TAR指示的SD                                                                   					                                                                                                             
*
* @Return      :BYTE
*                                                                                                                                                        
* @History	   :chentao
*/ 
BYTE IF_ChinaTelecom_RealNetWorkSSD(BYTE sdId)
{
#if UICC__OPERATOR__CHINATELECOM
// 	if((G_SPIKICKID[0]&0x04) == 0)  //不需要解密 下行没有被保护
// 	{
// 		return FALSE;
// 	}
// 	else
// 	{
// 		return TRUE;
// 	}
	BYTE xdata realSSD1[0x09] = {0xA0,0x00,0x00,0x01,0x51,0x53,0x50,0x41,0x02};
	//BYTE xdata realSSD2[0x10] = {0xF0,0x00,0x00,0x00,0x00,0x01,0x90,0x77,0x7F,0x02,0x01,0x00,0x00,0x00,0x00,0x83};

	//if((sdId == GetAppIdByAIDFromAIDList(realSSD1, sizeof(realSSD1))) || (sdId == GetAppIdByAIDFromAIDList(realSSD2, sizeof(realSSD2))))
//	if(sdId == GetAppIdByAIDFromAIDList(realSSD1, sizeof(realSSD1) ))
//	{
//		if(INS == 0xD8)
//		{
//			return TRUE;
//		}
//	}
	
	return FALSE;
#else
	return FALSE;
#endif
	
	
}

BYTE ProcessRAMinit(void)
{
	BYTE idata 	sdId,assoicateId, tmpAppId;
	WORD idata keyFlag;
//	UWORD xdata EF_RAM_fileAddr;
	
	G_RAMSessionSdId = ILLEGAL_APPID;

	if(G_CurrentChainingValueTag == FALSE)
	{
		//g_SCP02SDID = ILLEGAL_APPID;//该变量含义与G_RAMSessionSdId 后续考虑合并成一个   SCP02_SCP80_SecureChannel_encryptData_Test_01(Application processData),需要清会话时，该变量会被同时清掉
	}

	keyFlag = FALSE;

	G_GPErrorCode = 0x9000;  //清除状态字

#if M2M_EUICC
	if((G_TARRecIndex[0] == TarType_ISDR))
	{
		sdId = 	ISDR_APPID;
	}
	else if(G_TARRecIndex[0] == TarType_ISDP)
	{
		sdId = ISDP_APPID;
	}
	else
#endif
	{
		sdId = G_TARRecIndex[1];
	}

	
//	if((GPSSDExistCompleteSCP80Key(sdId) == TRUE)
//	||(GPSSDExistCompleteSecureKey(sdId,SCP_02) == TRUE)
//#if GP__AMENDMENT__D
//	||(GPSSDExistCompleteSecureKey(sdId,SCP_03) == TRUE)
//#endif
//	)  //modified by dingzy for scp03t, Security Domain having one complete Key Set for a Secure Channel;
	{
		keyFlag = TRUE;
	}

	assoicateId = GetAssoiatedSDByAppId(sdId);

	if(keyFlag == FALSE)	  //Security Domain with no Secure Channel Key Set
	{
		if(assoicateId == G_TARRecIndex[2])
		{
			//do nothing   If the closest ascendant is the associated Security Domain the remote APDU command string is trusted and processed by the targeted Security Domain according to

		}
		else
		{
			G_RAMSessionSdId = assoicateId;
			
			if(G_RAMSessionSdId != 0xFF)
			{
				GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);	 //清除安全会话信息
			}
		}
	}
	else   //Security Domain having one complete Key Set for a Secure Channel;
	{
		if(sdId == G_TARRecIndex[2]) // 1: When the targeted Security Domain has a SCP80 Key Set, the security of the remote APDU command string is unwrapped by the Security Domain itself and processed as specified in [2]. Any attempt to initiate a SCP02 Secure Channel within the command script shall be rejected.
		{

		}
		else    //When the targeted Security Domain does not have a SCP80 Key Set, the remote APDU command string is not trusted. The Security Domain may use its own SCP02 Secure Channel to secure each command received in the remote APDU command string
		{
			//
			if(IF_ChinaTelecom_RealNetWorkSSD(sdId) == TRUE)
			{
				//电信实网测试  数据已经被scp80加密过  不需要再开scp02
			}
			else
			{
				G_RAMSessionSdId = sdId;
			}	
			
		}	
	}
	
	return sdId;
}


/*                                                                                    
* @Function     	    :HttpProcessRAMinit
*  
* @Discription        :ram-apdu执行前的初始化
*
* @See                :附录b-3.3.3.2章节
*                                                                                                                                                
* @Parameter          :void                                                                   					                                                                                                             
*
* @Return             :void
*                                                                                                                                                        
* @History		      :V1.1：creat by jiangqb-20190710	
*                     :V1.1 modify by xuxiaojun@20191125  修改BUG: if the Security Domain unwrapping the remote APDU command string is the Security Domain associated with the targeted Security Domain, then the remote APDU command string is trusted and processed by the targeted Security Domain.
*
*/ 
BYTE HttpProcessRAMinit(void)
{
#if GP__AMENDMENT__B__FOR_MASK
	BYTE idata 	assoicateId;
	WORD idata keyFlag;
	
	G_RAMSessionSdId = ILLEGAL_APPID;
//	g_SCP02SDID = ILLEGAL_APPID; //M2M下载移动profile后相关测试/HTTPS同步bug测试/TPX3.0/T_011_HTTP_Install_Delete_Personal_SSD_Normal
	keyFlag = FALSE;

	G_GPErrorCode = 0x9000;  //
		
//	if((GPSSDExistCompleteSCP81Key(G_HTTPSStatus.TargetedSD) == TRUE)
//		||(GPSSDExistCompleteSecureKey(G_HTTPSStatus.TargetedSD, SCP_02) == TRUE)
//		#if GP__AMENDMENT__D
//		||(GPSSDExistCompleteSecureKey(G_HTTPSStatus.TargetedSD, SCP_03) == TRUE)
//		#endif
//		)  //modified by dingzy for scp03t, Security Domain having one complete Key Set for a Secure Channel;
	{
		keyFlag = TRUE;
	}

	assoicateId = GetAssoiatedSDByAppId(G_HTTPSStatus.TargetedSD);
	
	if(G_HTTPSStatus.TriggeredSD == G_HTTPSStatus.TargetedSD)
	{
		/*	
		When the targeted Security Domain is the one unwrapping the remote APDU command string, then the 
		remote APDU command string is trusted and processed. Any attempt to initiate a Secure Channel session 
		(according to another Secure Channel Protocol) within the remote APDU command string shall be rejected	
		*/
	}
	else
	{			
		if(keyFlag == FALSE)	  //If the targeted Security Domain does not have any Secure Channel Key Se
		{
			if(assoicateId == G_HTTPSStatus.TriggeredSD)	 //1?áaμ?°2è?óò?íê??á?°óμóD??°2è?óò
			{
				//do nothing  GP_AnnexB  3.3.3.2  Secure Channel Protocol Usage 
				/*					
					if the Security Domain unwrapping the remote APDU comman d string is the Security Domain 
					associated with the targeted Security Domain, then the remote APDU command string is trusted 
					and processed by the targeted Security Domain.
				*/
			}
			else
			{
				G_RAMSessionSdId = assoicateId;	
				
				//同步 5g 代码，此处不用清空会话， 测试用例：移动5G附录B内测脚本/LOG/TelecomNetWorkTest01
//				if(G_RAMSessionSdId != 0xFF)
//				{
//					GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);
//				}
			}
		}
		else   // If the targeted Security Domain has at least one complete Key Set  for a Secure Channel Protocol, the remote APDU command string is not trusted, and:
		{	
			G_RAMSessionSdId = G_HTTPSStatus.TargetedSD;  //μ±?°μ???±êsd′??ú°2è?key￡?
		}
	}
	
	return G_HTTPSStatus.TargetedSD;
#else
	return ILLEGAL_APPID;
#endif
}
#endif
/*
*@funtion    :ProcessRemoteCheckRAPDU 判断C——APDU数据是否完整
*
*@description:
*
*@param      :cplLen	--	数据长度
*
*@return     :TRUE 完整/FALSE 不完整
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181213      create
*/
BYTE ProcessRemoteCheckCAPDU(WORD cplLen)
{
	if(G_Data_Type == DataType_RFM)
	{
		if ((P3 != 0x00) && ((INS != 0xB0) && (INS != 0xB2) && (INS != 0xC0)))	//case3,case4
		{
			if((cplLen) < (P3+5))
			{
				return FALSE;
			}
		}
	}
	else
	{
		if( ((INS != 0xC0)&&(INS!=0xCA))||((INS == 0xCA)&&(P1==0xFF)&&(P2==0x1F)))        //GET DATA只有FF1F才接收数据		
		{	
			if((cplLen) < (P3+5))	  	//数据长度小于一条指令，退出
			{
				return FALSE;
			}	
		}
	}
	
	return TRUE;
}



/*
*@funtion    :ProcessRFMinit
*
*@description:rfm操作时初始化相关变量
*
*@param      :
*
*@return     :void
*
*@history    :V1.0 creat by jiangqb @20200327
*/
#if UICC__FUNCTION__RFM
void ProcessRFMinit(WORD *fileName)
{
#if UICC_RFM_FILE_UPDATE_EVENT
	UWORD xdata updateFileAddr;
	
	updateFileAddr = SelectLFFile(DF_Toolkit_P,EF_RFMFileUpdate);
	
	if (updateFileAddr != FALSE)		////迁移自tpx2.0平台， 进入RFM时，清除EF_RFMFileUpdate文件内容
	{
		ClearFileConst(updateFileAddr,0x00);
	}
#endif		
	BackDFNameForMoveFalsh(fileName);        //备份DF name  防止RFM执行过程中会改变当前DF EFAddress，RFM执行完后 还原错的值  
	
	setCurDfNameAndAddrbyTAR();
	
	G_CurrentEfName[G_ChannelNum] = FALSE;

	G_CurrentEfAddr[G_ChannelNum] = FALSE;
	
	SetBit(G_SystemStatus, B_RfmPerform);	//设置RFM 状态
}
#endif


/*
*@funtion    :ProcessCompactFun
*
*@description:解释压缩格式 
*
*@param      :fileAddr	--	命令的地址
*							dataOffset	--	命令的偏移
*							dataLen  -- 命令的长度
*
*@return     :void
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181115      create
*/
BYTE ProcessCompactFun(UWORD fileAddr,WORD commandOffset, WORD cplLen,BYTE mode)
{
	BYTE idata RFMCurrentRecord,RFMCommandMode,RFMSFI;
	WORD idata RFMADFName;                        //如果对于卡片支持多个ADF的情况下，一定要备份ADF name,否则如果在RFM 中修改了ADF name但是没有恢复，在通道中选择7FFF时，可能会出问题
	WORD  idata fileName[4];                      //前3备份DF name  4备份 EF name
	
	BYTE idata GetResponseTag;
	WORD idata resultState;
	BYTE    idata    RFMChvStatus;			//PIN1 status

#if !SE_ONLY
	BYTE    xdata    RFMLocalPinStatus[8];
#endif

	WORD    idata    RFMAdmStatus;
#if  UICC_SUPPORT_RFMHANDLER_D0	
	BYTE idata d0Num;
#endif	
	
	if(G_ChannelNum != 0x00)  //根据GSM11.11 TS102221，C2指令的CLA 只能为A0\80,因此一定是在通道0执行C2命令
	{
		return POR_OK;
	}
	
	//sdId = ILLEGAL_APPID;
	GlobalSetStatus(0x9000);
	
	G_FormatAndType = FormatType_Compat;   //压缩格式

    G_CurrentComNUM = 0x00;
	
	G_CurrentRemoteResponseLgth = 0x00;
	
	G_CurrentReponse[G_ChannelNum] = 0x00; //防止正常流程下G_CurrentReponse的数据没有被获取，被RFM 获取,该值在新的一轮中会被清成0
	
	RFMCommandMode = G_CommandMode;
	
	G_CommandMode = Command_Mode_First;
	
	RFMSFI = G_CurrentSfi;
		
	RFMADFName = G_CurrentAdfName[G_ChannelNum];      //备份ADF name

	RFMCurrentRecord = G_CurrentRecNum[G_ChannelNum]; 
	
//	getStatusCommandFlag = FALSE;
	
	GetResponseTag = 0x00;   //对于压缩格式 只有B0或者CA等才需要返回响应数据   但是每次都要保存上一次执行响应长度 等到GetResponse取出来
	
	RFMChvStatus = G_ChvStatus;

#if !SE_ONLY
	memcpy(RFMLocalPinStatus, &G_LocalPinStatus[0], 0x08);
#endif
	RFMAdmStatus = G_AdmStatus;
	
	if(G_Source_Type == SourceType_HTTPS)
	{
		ReadEeprom(G_APDU, fileAddr + commandOffset, 0x05);
	}
	else
	{
		ReadTFBuf(fileAddr, commandOffset, G_APDU, 0x05);	//获取APDU
	}
#if UICC__FUNCTION__RFM
	if(G_Data_Type == DataType_RFM)
	{
		ProcessRFMinit(fileName);
	}
	else
#endif		
	{
#if UICC__FUNCTION__RAM		
		if (G_Source_Type == SourceType_HTTPS)
		{
			//sdId = HttpProcessRAMinit();
			HttpProcessRAMinit();
		}
		else
		{
			//sdId = ProcessRAMinit();
			ProcessRAMinit();
		}
#endif		
	}
	
	resultState = POR_OK;
	
#if UICC__FUNCTION__RFM	
	if(G_Source_Type == SourceType_HTTPS)
	{
		IF_AnnexB_HTTPRFM(HTTP_RFM_MODE_READ);
	}
#endif
	
	//电信测试脚本5G 补充 RFM, T_UnblockPIN_PORSubmit 63C2 要求继续向下执行
	while(  (cplLen > 0) && ((SW1 == 0x90)||(SW1 == 0x9F) || (SW1 == 0x61)|| (SW1 == 0x63))) //添加63  联通测试校验pin指令返63Cx 期望能执行往下执行	
	{		
		if(cplLen<5)
		{
#if GP__AMENDMENT__B__FOR_MASK
			 if((G_Source_Type == SourceType_HTTPS)&&(mode != RemoteDataEndBlock))
			 {
				 IF_ProcessRemoteMoveFalshForHTTP(fileAddr,commandOffset, cplLen);
				 
				  G_CurrentComNUM ++;  //当前执行的指令个数加1
			 }	
#endif			 
			break;
		}
#if GP__AMENDMENT__B__FOR_MASK				
		if(G_Source_Type == SourceType_HTTPS)
		{
			ReadEeprom(G_APDU, fileAddr + commandOffset, 0x05);
		}
		else
#endif			
		{
			ReadTFBuf(fileAddr, commandOffset, G_APDU, 0x05);	//获取APDU
		}
#if UICC_SUPPORT_RFMHANDLER_D0			
		if((G_APDU[0] == 0xD0)&&((G_APDU[1] == 0x81)||(G_APDU[1] < 0x80)))		  //?????
		{		    
			CommandStartNum = commandOffset;

			d0Num = 0x01;

			if(G_APDU[1]==0x81)
			{
				commandOffset += (G_APDU[2]+3);
				cplLen -= (G_APDU[2]+3);
			}
			else
			{
				commandOffset += (G_APDU[1]+2);
				cplLen -= (G_APDU[1]+2);
			}

			if(cplLen > 3)
			{
				ReadTFBuf(fileAddr, commandOffset, G_APDU, 3);	                           //??APDU

				if((G_APDU[0] == 0xD0)&&((G_APDU[1] == 0x81)||(G_APDU[1] < 0x80)))
				{
					d0Num++;                                                       //????2??????
				}
			}

			PendingMessageAlert(TYPE_RFMProactiveCommandTag,d0Num);

			break;		    
		} 
#endif
		G_CurrentComNUM ++;  //当前执行的指令个数加1
		
		IF_AnnexB_ChangeComNUM(ComNUM_INCREASE);	//HttpApduNum 跟随G_CurrentComNUM递增

//
//		if(INS == 0xF2)
//		{
//			getStatusCommandFlag = TRUE;  //当前命令中有Get Status命令
//		}
		
		if(INS == 0xC0)
		{
			GetResponseTag = 0x01;  //有响应数据
		
			if((P1 != 0x00)||(P2 != 0x00))
			{
				GlobalSetStatus(0x6A86);
				
				break;
			}
			
			 if(G_CurrentRemoteResponseLgth == 0x00)
			 {
				GlobalSetStatus(0x6700);				 
				P3  = 0x00;
				break;
			 }
			 if(P3 == 0x00)
			 {
				//P3 = G_CurrentRemoteResponseLgth;   //获取全部响应数据
			 }
			 else if(P3 < G_CurrentRemoteResponseLgth)
			 {
				G_CurrentRemoteResponseLgth = P3;   //获取P3数据
			 }
			 else
			 {
				//	白盒测试添加   P3 小于或等于G_NativeRAMResponseLen时 ，根据实际的P3值确定响应长度 
			 }

//			 if((G_RAMGetStatus[NEXT_MATCH_INDEX] != 0x00)&&(getStatusCommandFlag == TRUE))  //uicc_configuration_additional_test_工程集\SECURED\GP补充测试\GP_TEST\getstatus补充测试\ GetStatusAddition_4_2
//			 {
//				 SW1 = 0x63;    //uicc_configuration_additional_test_工程集\GP补充测试\GP_TEST\getstatus补充测试\GetStatusAddition_1_2 GetStatusAddition_2_1  GetStatusAddition_2_2 3_1 3_2 4_1,GetStatus会话维持到下次
//					SW2 = 0x10;   //对于compact模式来说，因为c0一定是最后一条，所以当G_RAMGetStatus[NEXT_MATCH_INDEX] != 0时，说明前一次一定执行了GetStatus
//			 }
			 if((SW1==0x63)&&(SW2 == 0x10))
			 {
				 //get status 发6310
			 }
			 else
			 {
				GlobalSetStatus(0x9000);
			}
			
			cplLen -= 0x05; //减去00C0000000长度

			 break;
		}
#if UICC__FUNCTION__RAM		
		if(G_Data_Type == DataType_RAM)	
		{
		
//			if((G_RAMCAPInstallTag != CAPCOMPONENT_InvalidTag)&&(INS != 0xE8)) //此处无测试用例支撑，按照普通APDU的处理实现@2017118 by xuyz
//			{    // 中国移动NFC2.0项目20160415/GP2.2.1_UICC_ComplianceTests(移动版)-20141112/uicc_configuration_additional_test/MultiChannelTest_SECURED/MultiChannelTest_10_1、MultiChannelTest_10_2
//				if(G_RAMCAPInstallTag == sdId)// 中国移动NFC2.0项目20170116/GP2.2.1_UICC_ComplianceTests(移动版)-20141112/uicc_configuration_additional_test/MultiChannelTest_SECURED/MultiChannelTest_11_1 / Transaction 1:  ISD的RAM Load流程不会被另一个SD的其他非下载RAM操作打断
//				{
//					GPLoadInit();  //当前 RAM 正在执行load过程,打断
//				}
//
//			}

			//if( (bRamBuffer[INSTALL_FOR_LOAD_TO_SDID] == sdId) && (INS != 0xE8))
			//{
				//bRamBuffer[INSTALL_FOR_LOAD_TO_SDID] = 0;

				//GPLoadInit();
			//}
		}
#endif
	
		if(ProcessRemoteCheckCAPDU(cplLen) == FALSE)
		{			
			 if ((G_Source_Type == SourceType_HTTPS) && (mode != RemoteDataEndBlock))
			 {
				IF_ProcessRemoteMoveFalshForHTTP(fileAddr,commandOffset, cplLen);	
					
				IF_AnnexB_ChangeComNUM(ComNUM_DECREASE);	// HttpApduNum 减1是因为当前的apdu指令data域不完整，前面对HttpApduNum已进行+1，此处需要-1，返回http继续接收数据，然后再进来本函数处理指令
			 }

			 break ;
		}
			
#if UICC__FUNCTION__RFM		
		if(G_Data_Type == DataType_RFM)
		{			
//			if(ProcessRemoteCheckCAPDU(cplLen) == FALSE)
//			{			
//				 if ((G_Source_Type == SourceType_HTTPS) && (mode != RemoteDataEndBlock))
//				 {
//					IF_ProcessRemoteMoveFalshForHTTP(fileAddr,commandOffset, cplLen);					 
//				 }

//				 break ;
//			}

			resultState = RFMApduProcess(fileAddr, commandOffset, cplLen);
			
			commandOffset += 0x05;		//向下偏移
			
			cplLen -= 0x05; //剩余有效数据长度减少5字节
			
			if ((P3 != 0x00) && ((INS != 0xB0) && (INS != 0xB2) && (INS != 0xC0)))
			{
				commandOffset += P3;
				
				cplLen -= P3;
			}
			
#if GP__AMENDMENT__B__FOR_MASK	//todo，该问题如何修改需要进一步讨论，中国电信native平台备案脚本发现的问题, http会话中当指令抛错后， 目前方案是不再执行后续的apdu
			 if (G_Source_Type == SourceType_HTTPS)
			 {
				 //经检查, tpx和epx都有以下问题，后续需要同步，包括RFM和RAM模式，简单格式和扩展格式, 并且该问题在SMS,CATTP时如何处理？
				 //目前只有http的apdu有可能不完整(包括apdu不完整和apdu完整但不是最后一块), resultState退出本函数时仅http对返回值判断;	//用例：通用AmendMentB测试/RFM/T_024_HTTP_Seg_RFM_USIM
				if ((SW1 != 0x90) && (SW1 != 0x9F) && (SW1 != 0x61) && (SW1 != 0x62) && (SW1 != 0x63))	//指令异常时该判断是否合理？
				{
					resultState = GPAnnexBError;	//此处赋值时为了本函数返回值用于外部函数判断APDU指令执行结果
					break;
				}
			 }
#endif				
			
			if(resultState != POR_OK)
			{
				break ;
			}
			
			if((INS == 0xA4) && (SW1 == 0x62) && (SW2 == 0x83))	//文件失效特殊处理
			{
				if(cplLen != 0x00)  //A4后面还有APDU未执行，则将6283的状态进行转换，以便可能通过C0取数据
				{
					if(G_CurrentReponse[G_ChannelNum] == 0x00)  //当前无响应数据
					{
						GlobalSetStatus(0x9000);
					}
					else
					{
						SW1 = 0x61;
						SW2 = G_CurrentReponse[G_ChannelNum];  //将6283的状态字，切换成61XX，因为在A4之后报6283允许后续通过C0取响应数据
					}
				}					
				else
				{
					break;   //如果最后一条是A4命令，则返回6283
				}

			}
			if((INS == 0xB0)||(INS==0xB2))
			{
				if((SW1 == 0x90)||(SW1 == 0x62))  //B0 B2命令执行成功
				{
					if(INS == 0xB0)
					{
						if(SW1 == 0x62)
						{
							G_CurrentRemoteResponseLgth = 0x100;  //256字节的响应数据
						}
						else 
						{
							if(P3  == 0x00)  //返回剩余所有数据
							{
								G_CurrentRemoteResponseLgth = GetTFSize(G_CurrentEfAddr[G_ChannelNum])- (WORD)((((WORD)P1 << 8)&0xFF00 )|P2); 
								
								if(G_CurrentRemoteResponseLgth > 0xFF)   //二进制文件大于255字节，并且要求读出全部内容时，在Toolkit_ResponsePack 函数中读文件
								{
									//G_CurrentRFMEfAddr = G_CurrentEfAddr[G_ChannelNum] + sizeof(TranFileHead)+ (WORD)((((WORD)P1 << 8)&0xFF00 )|P2); //保存实际读取数据开始地址
									G_CurrentRFMEfAddr = G_CurrentEfAddr[G_ChannelNum]; // modify by xuweihua@20220514
									G_CurrentRFMEfOffset = (WORD)((((WORD)P1 << 8)&0xFF00 )|P2);
								}
							}
							else
							{
								G_CurrentRemoteResponseLgth = P3; 
							}
						}						
					}
					else
					{
						G_CurrentRemoteResponseLgth = P3;
					}
					
					GetResponseTag = 0x01;  //有响应数据
					
					break;  //The GET RESPONSE and any case 2 command (i.e. READ BINARY, READ RECORD) shall only occur once in a command string and, if present, shall be the last command in the string.
				} 
				else
				{
					break;
				}	
			}
			
			if((SW1 == 0x61) || (SW1 == 0x9F))
			{
				G_CurrentRemoteResponseLgth = G_CurrentReponse[G_ChannelNum];
			}

		}
		else
#endif			
		{
#if UICC__FUNCTION__RAM			
//			if(ProcessRemoteCheckCAPDU(cplLen) == FALSE)
//			{			
//				if((G_Source_Type == SourceType_HTTPS)&&(mode != RemoteDataEndBlock))
//				{
//					IF_ProcessRemoteMoveFalshForHTTP(fileAddr,commandOffset, cplLen);
//				}

//				break ;
//			}
			
			resultState = RAMApduProcess(fileAddr, commandOffset, cplLen);
			
			commandOffset += 0x05;		//向下偏移
			
			cplLen -= 0x05; //剩余有效数据长度减少5字节
			
			if ( ((INS != 0xC0)&&(INS!=0xCA)) || ((INS == 0xCA)&&(P1==0xFF)&&(P2==0x1F)) )
			{
				commandOffset += P3;
				
				cplLen -= P3;
			}	
			
#if GP__AMENDMENT__B__FOR_MASK	//todo，该问题如何修改需要进一步讨论，中国电信native平台备案脚本发现的问题, http会话中当指令抛错后， 目前方案是不再执行后续的apdu
			 if (G_Source_Type == SourceType_HTTPS)
			 {
				 //经检查, tpx和epx都有以下问题，后续需要同步，包括RFM和RAM模式，简单格式和扩展格式
				 //目前只有http的apdu有可能不完整(包括apdu不完整和apdu完整但不是最后一块), resultState退出本函数时仅http对返回值判断;	//用例：通用AmendMentB测试/RFM/T_024_HTTP_Seg_RFM_USIM
				if ((SW1 != 0x90) && (SW1 != 0x9F) && (SW1 != 0x61) && (SW1 != 0x62) && (SW1 != 0x63))	//指令异常时该判断是否合理？
				{
					resultState = GPAnnexBError;	//此处赋值时为了本函数返回值用于外部函数判断APDU指令执行结果
					break;
				}
			 }
#endif		
			if(resultState != POR_OK)
			{
				break ;
			}
			
			if(INS == 0xCA)    //电信测试RAM时候 会有多条CA指令 要求只能执行一条  RAM_getdata_1221       目前测试到的是: 80CAFF210080CAFF1F124F10A0000000090005FFFFFFFF895001000100C0000000     如果CA C0 CA C0这种格式的话 需要修改     
			{
				if(cplLen != 0x00)
				{
					if(G_Source_Type == SourceType_HTTPS)
					{
						ReadEeprom(&G_APDU[0], fileAddr + commandOffset, 0x05);
					}
					else
					{
						ReadTFBuf(fileAddr,commandOffset,&G_APDU[0],0x05); //读取 CLA INS P1 P2 P3	
					}
					if(INS != 0xC0)
					{
						cplLen = 0x00;
						
						G_CurrentRemoteResponseLgth = 0x00;
					}
				}

			}
			if((cplLen == 0)&&(SW1 == 0x61))
			{
				if((INS == 0xCA)&&(P3 == 0x00)) //The GET RESPONSE and any case 2 command (i.e. READ BINARY, READ RECORD) shall only occur once in a command string and, if present, shall be the last command in the string.
				{
					if(SW2 == 0x00)
					{
						SW2 = 0xFF; //暂时不支持6100数据长度的获取
					}
					
					GetResponseTag = 0x01;  //有响应数据
					
					G_CurrentRemoteResponseLgth = SW2;

					GlobalSetStatus(0x9000);//对于RAM APDU而言，GP 指令里面并不存在C0指令，所以当最后一条APDU 有响应数据时，则取响应，测试例子： N3_TEE\develop\others\test\移动NFC2.0送检工程集20151113\MultiChannel_20150330_明文版\天喻\19g/步骤4,该脚本没有检查响应数据
				}
				else     //对于普通的installforinstall函数，如果指令返回6101 ，则响应状态字为6101 ，没有响应数据  其他都是Case 4指令 如果需要相应数据 只能后面接  GET RESPONSE
				{
					G_CurrentRemoteResponseLgth = 0x00;
				}
			}
			else
			{
				G_CurrentRemoteResponseLgth = 0x00;   //清除P3 ，后续如果有C0指令，会重新赋值P3,使用P3作为响应数据长度  ,Toolkit_ResponsePack
			}

			if(SW1 == 0x61)          //如果需要响应数据 则后面要接 GET RESPONSE  ,case2指令在这行代码前面已经判断
			{
				if(SW2 == 0x00)
				{
					SW2 = 0xFF; //暂时不支持6100数据长度的获取
				}

				G_CurrentRemoteResponseLgth = SW2;
			}
			else if(SW1 == 0x90)
			{
				;
			}
//			else if((SW1 ==0x63)&&(SW2 == 0x10))
//			{
//				G_CurrentRemoteResponseLgth = IF_GetRAMResWORDLen();
//			}
			else
			{
				if(G_Source_Type == SourceType_HTTPS)			//执行出错，指令清空
				{
					cplLen = 0;
				}
				
				 break;
			}
#endif
		}	
	}
	
	if(GetResponseTag == 0x00)
	{
		G_CurrentRemoteResponseLgth = 0x00;
	}
#if GP__AMENDMENT__B__FOR_MASK	
	if(G_Source_Type == SourceType_HTTPS)     //保存当前Http RFM上下文
	{
		if((cplLen == 0x00)&&(mode != RemoteDataEndBlock)) //传进来的都是完整的指令 对于压缩格式 则要清空文件长度
		{
			IF_ProcessRemoteMoveFalshForHTTP(fileAddr,commandOffset, cplLen);
		}
		//同步5G移动代码， 指令执行出错是不应该把执行指令个数减1， 用例：移动 握奇 LOG AmendmentB测试脚本-5G/http/rfm/T_011_HTTP_NonSeg_SSD_Target_RFM_UICC
//		if((cplLen != 0x00) && (resultState == POR_OK) && ((SW1 == 0x90)||(SW1 == 0x9F) ||(SW1 == 0x61)) ) //指令断开时候 指令个数减去不完整的1
//		{
//			G_CurrentComNUM --;
//		}
#if UICC__FUNCTION__RFM		
		IF_AnnexB_HTTPRFM(HTTP_RFM_MODE_WRITE);
#endif		
	}
#endif	
	G_CommandMode = RFMCommandMode;
	
	G_ChvStatus = RFMChvStatus;

#if !SE_ONLY
	memcpy(&G_LocalPinStatus, &RFMLocalPinStatus[0], 0x08);
#endif

	G_AdmStatus = RFMAdmStatus;
#if UICC__FUNCTION__RFM	
	if(G_Data_Type == DataType_RFM)
	{		
		ClearBit(G_SystemStatus,B_RfmPerform);		//清除RFM状态
	
		G_CurrentAdfName[G_ChannelNum] = RFMADFName;  

		ReloadDFOrEFAddr(fileName);
		
		G_CurrentSfi = RFMSFI;
		
		G_CurrentRecNum[G_ChannelNum] = RFMCurrentRecord;
	}
	else
#endif		
	{
#if UICC__FUNCTION__RAM		
		if(G_RAMSessionSdId != ILLEGAL_APPID)		//0xFF
		{
			GPClearSecureSessionInfo(/*G_RAMSessionSdId*/);	 //RAM APDU执行成功，清除安全会话信息

			G_RAMSessionSdId = ILLEGAL_APPID;
		}
#endif		
	}

	return resultState;
		
}
/*
*@funtion    :RemoteAPDUProcessFunction 
*
*@description:处理RFM RAM函数的统一入口/ SMS/CATTP/Http等调用
*
*@param      :fileAddr	--	命令的地址
*			 :dataOffset	--	命令的偏移
*			 :dataLen  -- 命令的长度(对于CATTP/sms长度是CPL得来)
*            :mode--主要是为了压缩格式时候 http能判断出来是否是最后一块数据 拼装POR数据
*			 :*isTriHttp --- 传入参数，sms时传入一个指针，函数内对该指针位置的变量进行赋值，返回http触发短信的解析结果
*
*@return     :void
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20181115      create
*            :V1.1 对于type == 0x81 http触发短信，设置G_FormatAndType为简单格式，同时设置G_CurrentComNUM、G_CurrentRemoteResponseLgth、SW1、SW2的值， by LCZ 20190424
*			 :V1.2 modify by jiangqb @20191122,  增加当简单格式最后一块数据是0(即https最后下行数据是300D0A0D0A)时的特殊处理
*			 :V1.3 modify by jiangqb @20210906,	 增加函数的一个传入参数，用于是http触发短信时写入触发短信的解析结果
*/
BYTE RemoteAPDUProcessFunction(UWORD fileAddr,WORD commandOffset, WORD cplLen, BYTE mode,BYTE *isTriHttp)
{
	BYTE idata type,result;
	BYTE xdata cpl[0x03];
	BYTE idata cplHeadLen,pcntr;
	WORD idata DataLen;
	
	result = POR_OK;
#if UICC__FUNCTION__CATTP
	if((G_Source_Type == SourceType_SMS) || (G_Source_Type == SourceType_CATTP))
#else
	if(G_Source_Type == SourceType_SMS)
#endif	
	{
		ReadTFBuf(fileAddr,commandOffset,&cpl[0], 0x03);

		cplHeadLen =  ToolkitGetCPLLength(cpl[0]);

		if(cpl[0] < 0x0F)  //CPL为2字节,不是标准的BER-TLV编码
		{
			DataLen = (((WORD)cpl[0]<<8)&0xFF00)|cpl[1];
		}
		else if(cpl[0] < 0x80)
		{
			DataLen = cpl[0];
		}
		else if(cpl[0] == 0x81)
		{
			DataLen = cpl[1];
		}
		else //82
		{
			DataLen = (((WORD)cpl[1]<<8)&0xFF00)|cpl[2];
		}
		
		ReadTFBuf(fileAddr,commandOffset+cplHeadLen,&cpl[0], 0x01);    //CHL

		ReadTFBuf(fileAddr,commandOffset+cplHeadLen+13,&pcntr, 0x01);    //PCNTR;  pcntr会不会 不解密时候 随机写个值？？

		commandOffset = commandOffset + cplHeadLen + cpl[0]+1;  //跳过CPL CHL SPI KIC KID TAR CNTR PCNTR MAC

		DataLen = DataLen-(pcntr+cpl[0]+1);  //命令数据的实际长度

	}
#if GP__AMENDMENT__B__FOR_MASK	
	else if(G_Source_Type == SourceType_HTTPS)
	{
		DataLen = cplLen;
	}
#endif	
	else
	{
		return POR_OK;
	}
	
//	ReadTFBuf(fileAddr, commandOffset, &type, sizeof(type));
#if GP__AMENDMENT__B__FOR_MASK	
	if(G_Source_Type == SourceType_HTTPS)
	{
		ReadEeprom(&type, fileAddr + commandOffset, sizeof(type));	
	}
	else
#endif		
	{	
		ReadTFBuf(fileAddr, commandOffset, &type, sizeof(type));
#if GP__AMENDMENT__B__FOR_MASK		
		if (isTriHttp != NULL)
		{	
			//对传入的参数该值进行赋值，外部判断是否是http的触发短信
			*isTriHttp = IF_GP_GPAnnexB_TriggerSMS_Check(fileAddr, commandOffset,DataLen);
			
			if (*isTriHttp != NOTGPAnnexBTritag)
			{
				return POR_OK; 
			}
		}
#endif		
	}
	
	if (DataLen > 0) 
	{
#if UICC__FUNCTION__RFMRAM_EXTEND
		if((type & 0x03)!= 0x00)  //5.3 Automatic application data format detection
		{
			result=ProcessExtendFun(fileAddr,commandOffset,DataLen,mode);
#if M2M_EUICC
			if((result != POR_OK)&&(G_TARRecIndex[0] == TarType_ISDP))
			{
				if(G_DownLoadingProfileIndex != ILLEGAL_PROFILEINDEX)
				{
					IF_M2M_DeleteProfilePE(G_DownLoadingProfileIndex);									
				}
			}
#endif
		}
		else
#endif			
		{
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)			
			result=ProcessCompactFun(fileAddr,commandOffset,DataLen,mode);
#endif			
		}
	}
	else
	{
		GlobalSetStatus(0x9000);
	
		G_CurrentComNUM = 0x00;
	}
	
#if GP__AMENDMENT__B__FOR_MASK
	if (G_Source_Type == SourceType_HTTPS)
	{
		if ((mode == RemoteDataEndBlock) || (result != POR_OK)) 		 //Http下来的 并且是最后一块 或者执行出错 则整理数据给Http使用
		{
			if (((type & 0x03) == 0x00) && (cplLen == 0))	//最后一次https下行数据是300d0a0d0a
			{
				IF_AnnexB_GetHttpsCompactAPDURes();
			}

			IF_AnnexB_ProcessRemoteAppendDataForPor(mode);		//mode == RemoteDataEndBlock
		}
		else
		{	
			if ((type & 0x03) == 0x00)
			{			
				IF_AnnexB_SaveHttpsCompactAPDURes();
			}		
		}
	}
#endif
	
	if((result != POR_OK) || (mode == RemoteDataEndBlock)) //执行出错  或者最后一个协议数据的结束  这个变量理论上是要Http宏隔开的 但实际在102226使用 所以不隔开
	{
		G_ChainingTagRec = FALSE;     //  不能放在ProcessExtendFun 进去时候清空
	}

	return result;
}
/*
*@funtion    :GetTlvTandLBytes
*
*@description: 获取一个TLV的TL的字节数
*
*@param      : dataBuf		--		TLV的数据地址
							 
*            
*@return     :void
*
*@author     :duan ss
*
*@history    :V1.0 :create by duan ss@20170623
*/
BYTE GetTlvTandLBytes(BYTE * dataBuf)
{
	if (dataBuf[1] == 0x81)
	{
		return 3;
	}
	else if (dataBuf[1] == 0x82)
	{
		return 4;
	}
	else
	{
		return 2;
	}
}



/*
*@funtion    :GetTlvLValue
*
*@description: 获取一个TLV中V的值
*
*@param      : dataBuf		--		TLV的数据地址
							 
*            
*@return     :void
*
*@author     :duan ss
*
*@history    :V1.0 :create by duan ss@20170623
*/
WORD GetTlvLValue(BYTE * dataBuf)
{
	if (dataBuf[1] == 0x81)
	{
		return dataBuf[2];
	}
	else if (dataBuf[1] == 0x82)
	{
		return BuildWord(&dataBuf[2]);
	}
	else if (dataBuf[1] == 0x00)
	{
		return 0;
	}
	else
	{
		return dataBuf[1];
	}
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 FindUnFormatTag

  input parameter:   dataPointer

  					 tag

  output parameter:  The offset of the tag that is fineded on the tag/FALSE

  description:       Search the tag in the dataPointer, if success return the position of the length,

                     else return FALSE  主要针对查找不规则tag增加的，比如C网的08 tag后面即使接着0x81，他的长度字节也是一个字节 0x81就是长度字节

  Code status:        OK

  test status:        OK

 -------------------------------------------------------------------------------------------------------------------*/

BYTE FindUnFormatTag(BYTE * dataPointer,BYTE tag,BYTE lgth)
{
	WORD idata i,dataLgth;

	dataLgth = 0x0000 + lgth;

	for(i = 0; i < dataLgth; )
	{

		if((dataPointer[i]&0x7F) == (tag&0x7F))
		{
			return ((i&0xFF)+1);			
		}
	
		i =i + dataPointer[i+1]+2;
	}

	return FALSE;
}

#if UICC__CDMA__SMS__DOWNLOAD
BYTE code TP_PID_DCS_SCTS[] = {0x7F,0xF6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
/*
*@funtion    :CdmaTransGsm 
*
*@description:处理Cdma短信,转成G网去加解密或者非格式的直接拼凑去触发应用
*
*@param      :
*
*@return     :TRE(数据有错或者已经触发应用)/FALSE(不是Cdma短信等)
*
*@author     :LCZ
*
*@history    :V1.0 LCZ @20190109      create
*/

BYTE CdmaTransGsm(void)
{
	BYTE idata ptr,offset,TagLen,UserDataTagLen,UnFormtOffset;
	BYTE idata LenNote3,LenNote2,LenNote1,TpoaLen,TpoaLenoffset;
	BYTE xdata *resultBuf;
	UWORD xdata cdmafileAddr;
	
	G_IsCdmaSms = FALSE;
	
	resultBuf = &tempBuf[TempBufoffset - 17];   //用来保存原始TPOA

	ptr = FindTag(&RecData[0],0xD1,P3);
	
	if(ptr == FALSE)         //格式有错
	{
		return TRUE;
	}
	
	offset = FindTag(&RecData[ptr+1],0x02,P3);//Device Identities

	if(offset == FALSE)         //格式有错
	{
		return TRUE;
	}
	else
	{
		memcpy(&tempBuf[RFM_CSMS_DeviceIdentities],&RecData[offset + ptr],0x04);      //Device Identities 
	}
	
	offset = ptr+1;       //D1的下一个 tag
	
	ptr = FindTag(&RecData[offset],0xC8,P3);//ptr+1  到下一个tag
	
	if(ptr == FALSE)         //不是cdma短信
	{
		return FALSE;
	}
	else
	{
		memcpy(&tempBuf[RFM_CSMS_MessageType],&RecData[offset + ptr+1],0x05);      //SMS Point to Point(1) + Teleservice Identifier(4)
	}
	
	TagLen = RecData[ptr+offset];   //C8 tag Len
	
	offset += ptr +1 + 1; //到 Teleservice Identifier  tag  (1+1)(Len +   Message Type)
	
	ptr = FindUnFormatTag(&RecData[offset],0x02,TagLen);//Originating Address
	
	if(ptr == FALSE)         //格式错误
	{
		return TRUE;
	}
	else
	{
		TpoaLen = RecData[offset + ptr];
		
		TpoaLenoffset = offset + ptr-1;  //保存TPOA开始位置
		
		memcpy(&tempBuf[RFM_CSMS_Originating_Address],&RecData[offset + ptr-1],TpoaLen + 2); //保存C网的Originating Address  TLV
	}
	
	ptr = FindUnFormatTag(&RecData[offset],0x08,TagLen);//Bearer Data
	
	if(ptr == FALSE)         //格式错误
	{
		return TRUE;
	}
	else
	{
		tempBuf[RFM_CSMS_LAST_BEARER_LEN] = RecData[offset + ptr];//保存 Bearer Data 的长度  单条则为长度,级联则最后一条长度
		
		TagLen = RecData[ptr+offset];   //08 tag Len
		
		offset += ptr +1; //到 Message Identifier  tag
		
		memcpy(&tempBuf[RFM_CSMS_Message_Identifier],&RecData[offset],0x08); //Message Identifier(5) + Message Display Mode(3)
	}
	
	ptr = FindUnFormatTag(&RecData[offset],0x01,TagLen);//User Data 
	
	if(ptr == FALSE)         //格式错误
	{
		return TRUE;
	}
	else
	{
		UserDataTagLen = RecData[offset + ptr];      //User Data Length
		
		tempBuf[RFM_CSMS_MSG_ENCODING] = RecData[offset + ptr + 1]; //+1  User Data Length    MSG_ENCODING Tag
		
		ShiftDataUncoil(&RecData[offset + ptr+1], UserDataTagLen, &tempBuf[TempBufoffset], 5); 	// 左移5个bit，去掉MSG_ENCODING  UserData 保存到 tempBuf[TempBufoffset]开始   这时候tempBuf[TempBufoffset]也是GSM UserData 长度 
		
		UserDataTagLen = tempBuf[TempBufoffset];
		
		if((tempBuf[TempBufoffset] >= 0x03)&&(tempBuf[TempBufoffset+1] == 0x02)&&(tempBuf[TempBufoffset+2] == 0x70)&&(tempBuf[TempBufoffset+3] == 0x00)) 
		{
			tempBuf[RFM_CSMS_refrence] = 0x01;
			tempBuf[RFM_CSMS_TotalNum] = 0x01;
		}
		else if((tempBuf[TempBufoffset] >= 0x03)&& ((tempBuf[TempBufoffset+1] == 0x07) || (tempBuf[TempBufoffset+1] == 0x05))  && ((tempBuf[TempBufoffset+2] == 0x00)&&(tempBuf[TempBufoffset+3] == 0x03)) )
		{
			tempBuf[RFM_CSMS_refrence] = tempBuf[TempBufoffset+4];     //批次    // 070003 XXXX01 7000  /050003 XXXXXX  格式短信 中间的条数 /050003 XXXX01 非格式短信 多条的时候 
			tempBuf[RFM_CSMS_TotalNum] = tempBuf[TempBufoffset+5];     //总数
		}
		else             //无格式数据  直接拼装到6F90 然后退出 
		{
			UnFormtOffset = 0x01;
			
			memcpy(&RecData[UnFormtOffset],&tempBuf[RFM_CSMS_DeviceIdentities],0x04);  //Device Identities
			
			UnFormtOffset += 0x04;
                        
            RecData[UnFormtOffset++] = (0xC8&0x7F); // C8 tag
			
			LenNote3 = UserDataTagLen + 1;     //1  MSG_ENCODING 
			
			if(LenNote3 >0x7F) //无格式 能多条？单条时候的长度 不会超过两字节   01 tag Len
			{
				LenNote3 = 0x02;      // 81XX
			}
			else
			{
				LenNote3 = 0x01;      // XX
			}
				
			LenNote2 = UserDataTagLen + 1 + LenNote3 + 1 + 8;//(UserDataTagLen + 1 + LenNote3 + 1 )整个01 tag长度  + 8(Message Identifier(5) + Message Display Mode(3))
			
			LenNote1 = LenNote2;
			
			TagLen = LenNote2;              //保存08 tag长度
			
// 			if(LenNote2 >0x7F)          // 08 tag Len
// 			{
// 				LenNote2 = 0x02;
// 			}
// 			else
			{
				LenNote2 = 0x01;      // 08长度 固定为一个字节
			}
			
			LenNote1 += LenNote2 + 1 + 5 + TpoaLen +2 ; // (LenNote2 + 1) 08 Tag   (SMS Point to Point(1) + Teleservice Identifier(4))  5  (TpoaLen +2) TPOA
			
			if(LenNote1 >0x7F)
			{
				RecData[UnFormtOffset++] = 0x81;
				
				RecData[UnFormtOffset++] = LenNote1;
			}
			else
			{
				RecData[UnFormtOffset++] = LenNote1;
			}
			
		    memcpy(&RecData[UnFormtOffset],&tempBuf[RFM_CSMS_MessageType],0x05);  //SMS Point to Point(1) + Teleservice Identifier(4)
			
			UnFormtOffset += 0x05;
			
		    memcpy(&RecData[UnFormtOffset],&tempBuf[RFM_CSMS_Originating_Address],TpoaLen+2); //Originating Address  TLV
                        
            UnFormtOffset += (TpoaLen+2);
			
			RecData[UnFormtOffset++] = 0x08;         //Bearer Data  
			
			RecData[UnFormtOffset++] = TagLen;    //   Bearer Data  Len
			
			memcpy(&RecData[UnFormtOffset],&tempBuf[RFM_CSMS_Message_Identifier],0x08);
			
			UnFormtOffset += 0x08;
			
			RecData[UnFormtOffset++] = 0x01;
			
			if(LenNote3 == 0x02)
			{
				RecData[UnFormtOffset++] = 0x81;
                            
                RecData[UnFormtOffset++] = UserDataTagLen + 1;     //1  MSG_ENCODING 
			}
			else
			{
				
				RecData[UnFormtOffset++] = UserDataTagLen + 1;     //1  MSG_ENCODING 
			}
			
			RecData[UnFormtOffset++] = ((tempBuf[RFM_CSMS_MSG_ENCODING]>>3)&0x1F);  //MSG_ENCODING 
			
			memcpy(&RecData[UnFormtOffset],&tempBuf[TempBufoffset+1],UserDataTagLen); // +1 偏移到数据
			
			UnFormtOffset += UserDataTagLen;
			
			WriteTFBuf(G_EnvelopHanderAddr, 0x00, &RecData[1], (UnFormtOffset-1));
		
			ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = (UnFormtOffset-1);
		
			G_EnvelopeTag = RecData[0];
		
			JavaToolkitProcess(EVENT_UNFORMATTED_SMS_PP_ENV_bit,0);
			
			return TRUE;
		}			
	}
	
	UnFormtOffset = 0x01;         //开始转G网
		
	CdmaTpoa2GsmTpoa(TpoaLenoffset, (BYTE *)&resultBuf[0x01]);
	
	if((resultBuf[1] %2) == 0x00)  // 04 06  123456
	{
		 resultBuf[0] = (resultBuf[1]/2);  //Len 字符格式 TPOA字符
	}
	else
	{
		resultBuf[0] = (resultBuf[1]/2) + 1;
	}
	
	//tempBuf[0]+1        :User
	//TP_PID              ：7F     
	//TP_DCS              ：F6
    //TP_SCTS             ：FFFFFFFFFFFFFF            7+2
	//1                   :GSM TPOA 格式字节  0x81
	//(resultBuf[0]+1+1)  :TPOA
	
	LenNote1 = tempBuf[TempBufoffset]+1+7+2+1+(resultBuf[0]+1+1); // 8B Tag Len
	
	if(LenNote1 >0x7F)
	{
		LenNote2 = 0x2;
	}
	else
	{
		LenNote2 = 0x01;
	}
	
	LenNote3 = LenNote2 + 1 + LenNote1 +4; //4  Device Identities    D1 tag Len

    if(LenNote3 > 0x7F)
	{
		RecData[UnFormtOffset++] = 0x81;
		RecData[UnFormtOffset++] = LenNote3;
		
		P3 = LenNote3 +3;
	}
	else
	{
		RecData[UnFormtOffset++] = LenNote3;
		
		P3 = LenNote3 +2;
	}
	
	memcpy(&RecData[UnFormtOffset],&tempBuf[RFM_CSMS_DeviceIdentities],0x04); //Device Identities
	
	UnFormtOffset += 0x04;

	RecData[UnFormtOffset++] = 0x8B;     // SMS TPDU标记
	
	if(LenNote1 >0x7F)
	{
		RecData[UnFormtOffset++] = 0x81;
		RecData[UnFormtOffset++] = LenNote1;
	}
	else
	{
		RecData[UnFormtOffset++] = LenNote1;
	}
	
	RecData[UnFormtOffset++] = 0x40; // TPOA
	
	RecData[UnFormtOffset++] = resultBuf[1]; // 字符个数
	
	RecData[UnFormtOffset++] = 0x81; //格式字符
	
	for(offset = 0x01;offset<=resultBuf[0];offset++)  //高低字节 对调  转成G网的TPOA   CDMAtpoa变量保存的是原始的(C网的是左移2,G网的是 高低对调)
	{
		RecData[UnFormtOffset++] = (resultBuf[offset+1]<<4 | resultBuf[offset+1]>>4);	// +1 到字符
	}
	
	memcpy(&tempBuf[RFM_GSM_Originating_Address],&RecData[UnFormtOffset-(resultBuf[0]+2)-1],resultBuf[0]+3);  //转成G网后的tpoa 也保存到CSMS短信头 好解密时候对比看当前解密的是不是C短信
	
	tempBuf[RFM_GSM_Originating_Address] = resultBuf[0] + 2;////len of OA +TPOA类型 +TPOA字符个数 = 总的字节数  比如 05 0681214365
	
	memcpy(&RecData[UnFormtOffset],(BYTE *)&TP_PID_DCS_SCTS,sizeof(TP_PID_DCS_SCTS)); //TP_PID_DCS_SCTS
	
	UnFormtOffset += sizeof(TP_PID_DCS_SCTS);
	
	memcpy(&RecData[UnFormtOffset],(BYTE *)&tempBuf[TempBufoffset],tempBuf[TempBufoffset]+1); //User Data
	
	G_IsCdmaSms = TRUE;
	
	cdmafileAddr = SelectPrivateLFFile(DF_Toolkit,EF_CSMS_Head);
	
   if(cdmafileAddr == FALSE)
	{	
		return TRUE;		
	}
	
	ptr =ToolkitFindRFMCSMS_Head(MODE_FIND_RECNO ,tempBuf[RFM_CSMS_refrence],tempBuf[RFM_CSMS_TotalNum],0x00,&tempBuf[RFM_GSM_Originating_Address]);//查找该批次的短信 有没有在A106文件中存储过  如果有存过 无需存第二次 ，将 Bearer Data 的长度修改成本条短信的Bearer Data 长度 
	
	if(ptr != FALSE)//该短信不是第一条  不需要重复存储 
	{
		WriteLFBuf(cdmafileAddr,ptr,RFM_CSMS_LAST_BEARER_LEN, &tempBuf[RFM_CSMS_LAST_BEARER_LEN],1);
	}
	else
	{
		
		ptr = ToolkitFindRFMCSMS_Head(MODE_FIND_NOUSE,0x00,0x00,0x00,NULL);
		
		if(ptr !=FALSE)
		{             
			WriteLFBuf(cdmafileAddr,ptr,0x00, &tempBuf[RFM_CSMS_refrence],(RFM_CSMS_LAST_BEARER_LEN - RFM_CSMS_refrence+1));
		}
		else
		{
			return TRUE;
		}            
	}

	return FALSE;
		
}
#endif
#endif
