/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1 修改函数GetResponseCommand，SelectCommand，ComputeRecord,ComputeBinary	 ，由于添加了BER-TLV 文件引起
			V1.2 增加函数STKStatusCommand()，用于SendProactiveCommand();并开设接口IF_STKStatusCommand()和IF_StatusCommand()
			V1.3 修改函数GetResponseCommand，修改当GetResponse发送时，若the sequence is not respected,则返回6F00
			V1.4 修改函数SeekCommand,修改对于循环文件，查找到符合条件的记录时，将当前记录设置为查找到记录的物理记录号
				 修改部分函数关于SFI的判断，短文件标识允许的范围为01-30
				 修改函数FileStatusHandler，对于USIM测试，失效的文件只允许操作deactivate 和select命令，对于激活文件，允许重复激活
				 修改函数SelectCommand、FileStatusHandler,根据TS10221规范，select by path from MF or select by path from current DF,终端不应该使用当前DF的标识符，且终端不应该use an empty data field
				 修改函数SelectCommand，当重复选择当前应用时，应用的安全状态需要被更新：PIN2未校验-联通脚本 -at 20100323
			V1.5 修改函数SelectCommand，修改当终端select by path from MF时，the terminal shall not use the 3F00
			V1.6 修改函数IncreaseCommand，增加对SFI的判断条件
			V1.7 修改函数SelectCommand()，删除选择AID时，对(P3%2) == 0x01，返回P3错误的问题 --chenl @20110808
				 修改函数GetRecordNo,ComputeRecord,SeekCommand，增加对循环文件的特殊操作 @20110726 by xuyz
				 修改函数StatusPublic，增加2G模式下，对P3 为0时的情况处理
				 修改函数GetResponseCommand,该函数只允许和指定的命令对出现 ，在其他命令响应中会清除G_CurrentResponse的值 @20110806 by xuyz
			V1.8 定义宏BERTLVCmd，将BER-TLV结构的处理放入该宏内 --20111027 by chenl
			V1.9 修改SelectCommand函数 @20130922 wlj
			V2.0 修改StatusCommand函数 @20130922 xwh
			V2.1 增加ISIM鉴权功能，需定义宏(#define ISIM) --xuqs @20140408
			V2.2 修改函数SelectByFileName，该函数仅用于处理在选择当前DF 的 子文件、父文件及兄弟文件，不再处理MF及ADF的选择,
			      原因：WIB 里面会出现FID为0000的情况，此时若ADF未选择，则会到MF下选择FID为0000的文件，导致文件选不到 @20140521 by xuyz
			      修改函数SelectCommand，单独处理MF及7FFF的选择@20140521 by XUYZ V1.10->1.11
			V2.3 修改SelectCommand，当fileName为7FFF时，替换为G_CurrentAdfName[G_ChannelNum] @20140619 by xuyz V1.11->1.12
			V2.4 修改RunAlgorithm, 函数中增加接口IF_RunAlgorithm,兼容VerifyICCID功能       	@20150428 by yangjian

			******************************new for uicc api***************************

			V2.5: 修改函数 SelectCommand，增加接口 IF_SelectByDFName的调用
			      修改函数StatusCommand，修改接口函数IF_StatusCommand的调用
				  修改函数STKStatusCommand，修改接口函数IF_STKStatusCommand的调用
				  修改函数UpdateBinary，UpdateRecord 增加接口IF_ExternalFileUpdate
				  修改函数GetResponseCommand，修改对状态字6700 6100的处理
				  增加函数TerminalCapability

			V2.6 修改函数StatusPublic，修改对于GetDfInformation UsimGetDfInformation的调用

			V2.7 修改函数IncreaseCommand，增加variable tag: Tag_BakIncreaseResponseData的使用，考虑到要在该指令中触发Tollkit应用，则需要将缓存数据保存在variable中·201511123 by xuyz

			V2.8 修改函数SelectCommand，增加IF_ProcessNoShareableFile处的调用 @20151201 by xuyz

 *
 */
 
#include "uicc_main.h"	
//#include  "main.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include <string.h>
#include <uicc_error.h>
//#include "resume.h"
#include "stkhead.h"
#include "Authentication.h"
#include "uicc_interface.h"
//#include "JCREMain.h"
//#include "transaction.h"
#include "gsmpublic.h"
#if UICC__FUNCTION__BERTLVCmd
#include "DataOrientedCommand.h"
#endif
#if UICC__AUTH__5G__SUPI
#include  "stkalert.h"
#include "Supi.h"
#include "cardManager.h"
#endif
#if UICC__AUTH__5G__SUSPEND
#include "SuspendUicc.h"
#endif
#include "Crypto_Constant.h"
#include "ToolkitAPIPublic.h"
#include "EUICCInterface.h"
#include "STKCATRuntimeEnviorment.h"
#include "ceph.h"
#include "Interface_LTEFromPlatform.h"
#include "ToolkitRFM.h"
#include "T0ApduImpl.h"
#include "euiccConfig.h"	//remove warning,dengyiwen,2022/9/21
#include "fileAction.h"		//remove warning,dengyiwen,2022/9/27
#include "EapAuthentication.h"
/*
*@description:  通过文件ID选择文件
*
*@param      :fileName  :待选择文件的文件ID
*
*@returns   :fileAddr/FALSE
*
*@history   :create by xyz@20100707 16:37
*/
UWORD SelectByFileName(WORD fileName)
{
	BYTE idata type;
	WORD idata  fileParentName;
	UWORD xdata fileAddr;
	
	if(G_CurrentDfAddr[G_ChannelNum] == FALSE)
	{
		return FALSE;
	}

/*	if((fileName == DF_MF) || (fileName == G_CurrentAdfName[G_ChannelNum]))
	{		
		fileAddr = SelectFile(E_FILE_START,fileName);              			//是否为MF或当前应用的ADF              
	}
	else */
	{
		fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName);     //是否是当前DF下的文件

		if(fileAddr == FALSE)
		{
			fileParentName = GetFileHeadWord(G_CurrentDfAddr[G_ChannelNum], F_SecondParentName_Offset);
		
			fileAddr = GetParentOrMapFileAddr(G_CurrentDfAddr[G_ChannelNum],F_ParentName_Offset);  //是否为当前DF的父文件
		
			if(fileParentName != fileName)
			{
				fileAddr = SelectFile(fileAddr,fileName);
				
				if(fileAddr != FALSE)                                		//是否是当前文件的兄弟文件
				{
				    type = GetFileHeadByte(fileAddr,F_Type_Offset);
					
					if(type !=  DFTYPE)
					{
						fileAddr = FALSE;
					}
				}
		    }
		}
	}
	
	
	return fileAddr;
}
/*
*@function   : IF_SelectByDFName
*
*@description:用于在成功选择指定ADF后，SetChannnel之前
*
*@param      :void
*
*@return     :false:继续执行后续操作，如setchannel等/TRUE：中止后续函数流程
*
*@history    :V1.0 :create by xuyz@20131129
*/
BYTE IF_SelectByDFName(void)
{
	BYTE idata lgth;
	BYTE idata result;
#if UICC__AUTH__EAP
	if((P2&0x60) != 0x40) // add 20220418 add EAP init
	{
		IF_EapSystemInit();
	}
#endif

#if UICC__FUNCTION__STKAPI
	result = FALSE;

	if(	G_CurrentAdfName[G_ChannelNum] != 0x00)  //触发EVENT_APPLICATION_DESELECT
	{
		if(G_STKBusy != FALSE)   //当前在STK 中进行了应用的选择，可能会触发应用，因此要先把当前上下文备份一下
		{
			ToolkitSaveAPPContext();
		}
		lgth = GetCurrentADFAid(G_CurrentAdfName[G_ChannelNum],&tempBuf[0]);

		tempBuf[0] = 0x2F;  //AID tag ts 101220
		
		WriteTFBuf(G_EnvelopHanderAddr, 0x00, &tempBuf[0],lgth);

		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = lgth;

		JavaToolkitProcess(EVENT_APPLICATION_DESELECT_bit,0x00);

		if(SetSTKBusy() == TRUE)
		{
			result = TRUE;
		}
		
		if(G_STKBusy != FALSE)   //当前在STK 中进行了应用的选择，可能会触发应用,此处要把备份的上下文恢复
		{
			ToolkitRestoreAPPContext();
		}
	
	}
#endif

	return result;
}
/*
*@description: 通过路径选择文件
*
*@function   :SelectFileByPath
*
*@param       :void
*
*@return      : fileAddr/ FALSE
*
*@author      :create by xuyz
*/
UWORD SelectFileByPath(UWORD fileAddr,WORD *filename)
{
	BYTE idata num;
	WORD idata fileName;
	 
	for(num = 0x00;num < P3;num = num + 2)
	{
		 fileName = BuildWord(&RecData[num]);

		 if((fileName == 0x7FFF))
		 {
			fileName = G_CurrentAdfName[G_ChannelNum];
		 }
		
		 fileAddr = SelectFile(fileAddr,fileName);
		 
		 if(fileAddr == FALSE)
		 {
			  FileNotFound();
			 break;
		 }
		 else
		 {
			   if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x00, NULLAPI_Mode) == FALSE)  //若当前选中的是某个非共享文件，且该文件已经被其他通道或 fileView对象所占用，则返回6a82
			  {
					ConditionSatisfied();	 				// 69 85
					fileAddr = FALSE;
					break;
			  }
			  
			  *filename = fileName;

			  SetChannel(fileAddr,fileName,FALSE);
		 }
	 }
	 
	 return fileAddr;
}


/**
 * @brief 		       SelectCommand:This function selects a file according to the methods rules
 *
 * @command format 	   A0 A4 00 00 02 /0X A4 00(01,03,04,08,09) XX XX
 *
 * @Returns		       void
 *
 * @history 	        modify by chentao@EPX产业化\INFENION_SLM97\Develop
 */
void SelectCommand(void)
{
	UWORD xdata fileAddr;
	BYTE  idata responseLgth,recNum;
	WORD  idata fileName;
	PathInfo xdata filePathInfo;
	UWORD xdata actualFileAddr;
	BYTE idata type;
	//BYTE idata num;
#if SE_ONLY			//remove warning,dengyiwen,2022/9/27
	BYTE idata  status,i;		
	KeyStructer xdata chvStuct;
#endif

	if(G_CommandMode == Command_Mode_3G)    //0x00
	{
		if((P1!=0x00)&&(P1 != 0x01)&&(P1 != 0x03)&&(P1 != 0x04)&&(P1 != 0x08)&&(P1 != 0x09))
		{
			P1P2Error();
			return;
		}

		BackFilePath(&filePathInfo);	      	//备份当前路径
	}
	else		  //2G
	{
		if(ValueBit(G_SystemStatus, B_RfmPerform) == FALSE)	   //0xA0 A4 00 00 02
		{
			if(P1P2P3Check(0x02, FALSE) ==FALSE)
			{
				return;
			}	
		}
		else
		{
			if(((P1 != 0x00)&&(P1 != 0x01)&&(P1 != 0x03)&&(P1 != 0x04)&&(P1 != 0x08)&&(P1 != 0x09)) || ((P2 & 0x04) == 0x04))
			{				
				
				P1P2Error();
				return;
				
			}	
		}
	}
	
    //if((P1 != 0x03)&&(G_DATARECEIVED == 0))     //对2G情况下，P1，P2为0； 对于3G情况下，P1= 03时，为当前期望输出的数据长度，此时不应接受数据
	//P1==04的情况，数据已经在Dispatcher1中接收了，这里不再需要接收,by xwh 20191127
    if((P1 != 0x03)&&(P1 != 0x04))
    {
    	ReceiveData();
		//G_DATARECEIVED = 1;
	}

	fileName = FALSE;
	
	fileAddr = FALSE;
	
	if(P1 == 0x00)                        		//select by FID 
	{
		if(P3 == 0x02)							//modify by qiuping@2021-11-05 中国电信SWP国密测试工程列表-电信测试.sol:CTCC_Test/5G补充RFM/cmd_uicc/T_SELECT_ERROR_PORSubmit
		{										//当P1==0x00,P3==0X02时P2==0x24,0x2c,0x64,0x6c时脚本期望返回6B00
			if((P2&0xB3) != 0x00)
			{
				P1P2Error();					//Send 6B 00

				return;
			}
			
		    //rfm/func/cmd/uicc/T_SELECT_ERROR_PORSubmit   P2为错误值0   期望值：016B00/016A86
			//3G 指令才做以上判断
			if((G_CommandMode == Command_Mode_3G)&&((P2&0x04) == 0x00))
			{
				P1P2Error();					//Send 6B 00

				return;
			}
			
			fileName =(((WORD)RecData[0] << 8)&0xFF00) +  RecData[1];

			if((fileName == DF_MF) || (fileName == 0x7FFF))
            {
               if(fileName == DF_MF)
				{
					fileAddr = E_FILE_START;
				}
				else  //选择7fff,即选择USIM ADF
				{
					fileName = G_CurrentAdfName[G_ChannelNum];

					if(G_CurrentAdfName[G_ChannelNum] == FALSE)  //当前并未选中ADF
					{
						fileAddr = FALSE;
					}
					else
					{
						fileAddr = SelectFile(E_FILE_START, G_CurrentAdfName[G_ChannelNum]); 
					}
				}             
            }
            else
            {
            	fileAddr = SelectByFileName(fileName);	
            }

		
		}
		else if(P3 == 0x00)
		{
		 	if(P2 == 0x0C)                //直接选中MF，无数据返回
			{								
				fileAddr = E_FILE_START;

				fileName = DF_MF;	
	        }
			else
			{
				P3Error();				// Send 67 00 or 6A 87 P2error
				
				return;                                                						
			}		 
		}
		else
		{
			 P3InconsistentWithP1P2();       		//Send 6A 87

			 return;
		}
	}
	else if(P1 == 0x01)               				//选择当前DF的子DF文件
	{
		if((P2&0x93)!= 0x00)
		{
			P1P2Error();							//Send 6B 00
			
			return;
		}
		
		if(P3 != 0x02)                  
		{
			P3Error();
			
			return;
		}
		
		fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];
		
		fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName);    
		
		if(fileAddr != FALSE)              			//判断所选中的文件类型
		{
			type = GetFileHeadByte(fileAddr,F_Type_Offset);
			
			if(type !=  DFTYPE)
			{
				fileAddr = FALSE;
			}
		}
	}
	else if(P1 == 0x03)                				//选择当前DF的父DF文件
	{
		// ReadEeprom(&lteCardCfg,CARDCFG_LIANTONGATR, 0x01);	    //联通测试判断
		
		 if((P2&0x93)!= 0x00)
		 {
			  P1P2Error();							//Send 6B 00

			  return;
		 }

    	 if(G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)
    	 {
    	 	FileNotFound();		                  			//Send 6A 82	
	
			return;
    	 }
		 
		 
		//if((lteCardCfg == 0x00)&&(P3 != 0x00))        //联通蝴蝶结  期望00a4030400 选择上一级DF  00a4030416则是返回数据
		 if(P3 != 0x00)        //移动脚步也是P3 ！=0x00  返FCP
		{
			fileName = G_CurrentDfName[G_ChannelNum];
			
			fileAddr = G_CurrentDfAddr[G_ChannelNum];
			
		}
		else
		{
			 fileName = GetFileHeadWord(G_CurrentDfAddr[G_ChannelNum], F_SecondParentName_Offset);

			 fileAddr = GetParentOrMapFileAddr(G_CurrentDfAddr[G_ChannelNum],F_ParentName_Offset);
		}
 	}
	else if(P1 == 0x04)                  //select by DF name(ADF)
	{
		
		 if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)       // 102226  7.1 Commands  The SELECT command shall not include the selection by DF name corresponding to P1='04' in the Command Parameters of SELECT (see ETSI TS 102 221 [2]).
		 {
			 GlobalSetStatus(0x6A86);
			 return;
		 }
		 
		 if((P2&0xB0) != 0)             
		 {
			 P1P2Error();
			 return;
		 }

		if((P3==0x00)||(P3>0x10))
		 {
		  	P3Error();					//67 00												
			return;
		 }

		if((P2&0x02) == 0x02)  			//Next occurrence or Previous occurrence
		{
			 FileNotFound();			//because  exist  only one ADF
				
			 return;
		}
		 
		if((P2&0x60) == 0x40)  			//Application session Termination 
		{
//			G_CurrentAdfName[G_ChannelNum] = 0;	  //为兼容UICC API的处理，将此部分代码放到后面去 :UICC API中需要触发ADF_DESELCET事件

			fileAddr = E_FILE_START;

			fileName = 0x3F00;
		}
		else
		{
			fileAddr = SelectFileByName(E_FILE_START,&RecData[0],P3);
			
			if(fileAddr == FALSE)
			{
				 FileNotFound();		//Send 6A 82
				
				 return;
			}
			
			ReadEeprom((BYTE idata *)&fileName,fileAddr,sizeof(WORD));

#if SE_ONLY
			if(G_CurrentAdfName[G_ChannelNum] == fileName)
			{

				for(i = 0x81;i<=0x88;i++)//local Pin 1-8
				{
					status = ReadKey(i,(BYTE xdata *)&chvStuct.KeyId);

					if(status == TRUE)
					{
						if((chvStuct.KeyStatus&0x01) != PIN_DISABLED)//如果localpin不是disabled，才允许清除localpin的状态
						{
							ClearBit(G_KeyStatus,((i&0x7F)-1));//将localpin更新为未校验状态。2010-03-23；联通脚本
						}
					}
				}

			}
#else
			
			recNum = GetFileHeadByte(fileAddr,F_LocalPinIndex_offset);

            // modify by zhangxiaoxiao@20221204:			
			//1、创建3F00时，文件头的LocalPinIndex赋初值1
            //2、ReadPinStatus时，遍历localpin，需要遍历文件系统的整个DF，判断LocalPinIndex是否为0，localpin存在于DF/ADF下面
			if((G_CurrentAdfName[G_ChannelNum] == fileName) && ((recNum >= 1) && (recNum <= sizeof(G_LocalPinStatus))))								// 重复选择当前激活的ADF时（P2的bit7bit6=00）意为复位当前ADF，PIN的校验状态要随之更新。若存在多个应用时，该处还要考虑
			{
				ClearLocalPinStatus(fileAddr,&G_LocalPinStatus[recNum-1]);
			}
#endif
			
		}

		IF_SelectByDFName();	//该函数中会根据G_CurrentAdfName[G_ChannelNum]的值进行判断处理，因此 对于(P2&0x60) == 0x40的情况，要在该函数处理结束后，才清掉G_CurrentAdfName[G_ChannelNum] 的值

		if((P2&0x60) == 0x40)  			//Application session Termination 
		{
			G_CurrentAdfName[G_ChannelNum] = 0;
		}
		else
		{
			G_CurrentAdfName[G_ChannelNum] = fileName;
		}
	}
	else     							//select by path from MF or current DF
	{
		if((P2&0x93)!=0x00)
		{
			P1P2Error();
			return;
		}

		if((P3%2) == 0x01)
		{
			 P3Error();
			 return;
		}

        fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];

		if(P1 == 0x08)
		{	 
			if(fileName == 0x3F00)   				//IN case of select path from MF ,the terminal shall not use the FID of MF at the begining of the path
			{
			#if UICC__OPERATOR__CHINATELECOM
				IncorrentParameter();                  //中国电信rfm/func/cmd/uicc/T_SELECT_Path_PORSubmit  期望值：016A80
			#elif UICC__OPERATOR__CHINAUNICOM		//中国联通 华弘测试脚本/RFM(SMS)/SIM/004_SIM_SELECT,P1P2=0800,路径中带有3F00，期望9404
				FileNotFound();
			#else
				ConditionSatisfied();			    // 69 85
			#endif
				return;
			}
			
            fileAddr = E_FILE_START;

			IF_ProcessNoShareableFile(fileAddr,DF_MF, 0x00,NULLAPI_Mode);

            SetChannel(fileAddr,DF_MF,FALSE);
        }
		else
		{	 
		 	 if(fileName == 0x7FFF)      				//in case of "select by path from current DF",the terminal shall not use 7FFF at the begining of the path
		 	 {

				ConditionSatisfied();				// 69 85

				return;
		 	 }
		 	 fileAddr = G_CurrentDfAddr[G_ChannelNum];
		}

		if(fileName == G_CurrentDfName[G_ChannelNum])  	//select by path from MF or current DF,the terminal shall not use the identity of the current DF
		{
			if(G_CommandMode == Command_Mode_3G)
			{
				
				ResumeFilePath(&filePathInfo);              //restriction 3 need consider 2010-03-23
			}
			FileNotFound();		                  		//Send 6A 82	
	
			return;
		}
		
// 		for(num=0x00;num<P3;num=num+2)
// 		{
//              fileName = (((WORD)RecData[num]<<8)&0xFF00) + RecData[num+1];

//              if((fileName == 0x7FFF))
//              {
//                 fileName = G_CurrentAdfName[G_ChannelNum];
//              }
// 			
// 			 fileAddr = SelectFile(fileAddr,fileName);
// 			 
// 			 if(fileAddr == FALSE)
// 			 {
// 				 break;
// 			 }
// 			 else
// 			 {
// 			 	   if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x00) == FALSE)  //若当前选中的是某个非共享文件，且该文件已经被其他通道或 fileView对象所占用，则返回6a82
// 				  {
// 				  		fileAddr = FALSE;
// 						break;
// 				  }

// 				  SetChannel(fileAddr,fileName,FALSE);
//  			 }
// 		 }

		fileAddr = SelectFileByPath(fileAddr,(WORD *)&fileName);
	}

	if(fileAddr == FALSE)                         		//若文件未找到，则恢复备份路径
	{
		if(G_CommandMode == Command_Mode_3G)
		{
			ResumeFilePath(&filePathInfo);
		}
		FileNotFound();		                  			//Send 6A 82	
	
		return;
	}

	if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x00,NULLAPI_Mode) == FALSE)  //若当前选中的是某个非共享文件，且该文件已经被其他通道或 fileView对象所占用，则返回6a82
	{
		if(G_CommandMode == Command_Mode_3G)   //2G apdu模式下选择的文件全部是共享文件
		{
			ResumeFilePath(&filePathInfo);
		}
		 ConditionSatisfied();	 				// 69 85
		 				
		return;	
	}
	
	if(((P2&0x0C)==0x04)||(G_CommandMode == Command_Mode_2G))    //return FCP template
	{
// #ifdef CHINAUNICOM	
// 		responseLgth =  SetChannel(fileAddr,fileName,TRUE);		
// #else
		if(P1 != 0x03)
		{
			responseLgth =  SetChannel(fileAddr,fileName,TRUE);
		}
		if(P1 == 0x03)
		{
			//if(G_A4Control == FALSE)
			//{
				responseLgth =  SetChannel(fileAddr,fileName,TRUE);
			//}
			//else
			//{
			//	responseLgth = G_A4Control;
			//}
			
			//if((P3 == 0x00)||(P3 >responseLgth))
			{	
				//RepeatPreviousCmdHeader(responseLgth);
				apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, responseLgth);
				
				GlobalSetStatus(0x9000);
			}
			/*else
			{
				SendData();
				
				if(G_A4Control == FALSE)
				{
					CommandFinish(0x00);
				}
				else
				{
					ecpMem_Copy(&RecData[0],&RecData[P3],responseLgth-P3);
				
					CommandFinish(responseLgth- P3);
				}
			}
			*/
			return;
		}

// #endif
	}
	else
	{
		responseLgth =  SetChannel(fileAddr,fileName,FALSE);
	}

	CommandFinish(responseLgth);								 //send 9f 19																//send 9f 19


	if(G_CommandMode == Command_Mode_3G)
	{
#if UICC__FUNCTION__BERTLVCmd
		InitBERTLVTagIndicator();
#endif

		type = GetFileHeadByte(fileAddr,F_Type_Offset);										
		
		if(type != ADFTYPE)										 //没有判断DF的状态
		{
			actualFileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);	//获取文件的实际地址，防止文件时映射文件，而修改到了映射文件的状态
			
			type = GetFileHeadByte(actualFileAddr,F_Status_Offset);						
			
			if((type&0x01) == FILE_INVALIDATE)												
			{
				SelectedFileInvalid();							 //62 83
			}
		}
	}
}

/**
 * @brief 		       StatusCommand的公共子函数
 *
 * @Returns		       void
 *
 * @history 	       
 */
BYTE StatusPublic(void)
{  
	BYTE idata lgth;

	ecpMem_Set(RecData, 0x00,256); //由于RecData是指针，所以sizeof(RecData)的值不是256

	if((P1>0x02)||((P2 != 0x00)&&(P2!=0x01)&&(P2!=0x0C)))
	{
        P1P2Error();
		return FALSE;
	} 
	
	if(((P1 == 1) || (P1 == 2)) && (G_CurrentAdfName[G_ChannelNum] == FALSE))//Current application is initialized in the terminal/The terminal will initiate the termination of the current application
	{
		P1P2Error();
		return FALSE;
	}

	if(G_CommandMode == Command_Mode_2G)
	{	
		if(P1P2Check() ==FALSE)
		{
			return FALSE;
		}

		if(( P3 > R_DF_LENGTH)||(P3 == 0x00))    //增加P3 为0时的处理
		{
			P3ReturnRightLength2G(R_DF_LENGTH);
			return FALSE;
		}
	}
	
	if((P2 == 0x0C)&&(P3 != 0x00))
	{
		P3Error();
		return FALSE;
	}

	if(P2 != 0x0C)
	{
		if(P2 == 0x00)
		{
			if (G_CommandMode == Command_Mode_2G)
			{
				lgth = GetDfInformation(RecData);
			}
			else
			{
				lgth = UsimGetDfInformation(RecData);
			}
		}

		else
		{
			lgth = GetCurrentADFAid(G_CurrentAdfName[G_ChannelNum],(BYTE xdata *)&RecData[0]);
		}

        if(lgth == FALSE)
        {
            P1P2Error();

            return FALSE;                        
        }

		if((P3 == 0x00)||(P3 > lgth))               
		{
		  	RepeatPreviousCmdHeader(lgth);
			
		  	return FALSE;
		}
		
		SendData();
	}
	return TRUE;	
}

/**
 * @brief 		       StatusCommand:This function returns information concerning the current directory or current application
 *
 * @command format 	   A0 F2 00 00 lgth /8X F2 00(01,02) 00(01,0C) XX
 *
 * @Returns		       void
 *
 * @history 	       
 */
void StatusCommand(void)
{
	BYTE idata result;

 	result = StatusPublic();

	if (result == TRUE)
	{
		if(IF_StatusCommand() == TRUE)          //Status指令正常执行时才触发applet,by xwh 20150112
		{
			return;
		}
		
		ProcessPendingMessageAlert();
		
		CommandFinish(0x00);
	}
}

/**
 * @brief 		       STKStatusCommand:This function returns information concerning the current directory or current application
 *
 * @command format 	   A0 F2 00 00 lgth /8X F2 00(01,02) 00(01,0C) XX
 *
 * @Returns		       void
 *
 * @history 	       
 */
void STKStatusCommand(void)
{
	BYTE idata result;

 	result = StatusPublic(); 

	if (result == TRUE)
	{
		//Status指令正常执行时才触发applet,by xwh 20150112
		/*if(IF_STKStatusCommand() == TRUE)
		{
			return;
		}*/
		ToolkitSaveAPPContext();

	    JavaToolkitProcess(EVENT_STATUS_COMMAND_bit,0);

		if(SetSTKBusy() == TRUE)
		{
			return;
		}

		ToolkitRestoreAPPContext();
		
		CommandFinish(0x00);
	}
}

/**
 * @brief 		   	   ComputeBinary:This function is to compare the file condition is do or not
 *
 * @command format 	   
 *
 * @Returns		       TRUE/FALSE
 *
 * @history 	       
 */
BYTE ComputeBinary(BYTE action)
{
	BYTE     idata actionTag;
	WORD     idata binaryFileLgth,spaceSize;
	WORD	 idata lgth;

	lgth = P3;

	if (P3 == 0x00)
	{
		if(action==A_ReadFile)
		{
			lgth = 0x100;
		}
		else
		{
			P3Error();
			return FALSE;
		}
	}

  	if(((P1&0xE0)==0x80)&&(G_CommandMode == Command_Mode_3G))      
	{
		actionTag = P1 &0x1F;      //b1-b5 :SFI (01-30)
		
		if(actionTag == 0x1F)
		{
			P1P2Error();

			return FALSE;
		}
		
		P1 = 0x00;
  	}
	else
	{
		actionTag = 0x00;
	}

	actionTag = ComputeAll(action,actionTag,1);	  //判断当前选中的EF文件权限、状态、类型是否符合条件
		
	if(actionTag == FALSE) 
	{
		return FALSE;
	}
	
	spaceSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum],F_SpaceSize_Offset);	//获取透明文件的空间大小
	
	binaryFileLgth = (((WORD)P1<<8)&0xFF00)|(P2&0x00FF);
                                           
	if(spaceSize <= binaryFileLgth)
	{
		P1P2Error();	
														//send 6B 00
		return FALSE;
	}                                                                                                                                                                                   
	
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
	{
		if((action == A_ReadFile)&&(P3 == 0x00))   //对于RFM 操作而言，P3 = 0,返回透明文件剩余全部数据
		{
// 			if(spaceSize <( binaryFileLgth+0x100))  // 当剩余有效数据不足0x100时，返回剩余有效数据
// 			{
// 				P3 = spaceSize - binaryFileLgth; 
// 			}
// 			else
// 			{
// 				P3 = 0x100;
// 			}
			return TRUE;
		}
	}
	
	if(spaceSize < (binaryFileLgth + lgth))
	{
		if((action == A_ReadFile)&&(G_CommandMode == Command_Mode_3G))
		{
			RepeatPreviousCmdHeader((BYTE)(spaceSize-binaryFileLgth));	
		}
		else
		{			
			P3Error();												   //67 00
		}

		return FALSE;
	}

	return TRUE;
}
/**
 * @brief 		       ReadBinaryCommand:This function reads a string of bytes from the current transparent EF
 *
 * @command format 	   A0 B0 high low lgth /0X B0 high low lgth
 *
 * @Returns		       void
 *
 * @history 	       modify by chentao@EPX产业化\INFENION_SLM97\Develop
 *                     modify by zhxx,GET IDENTITY_附录C/安全条件检查/GET IDENTITY_6.1.8,5FC0/4F07或者5FD0/4F01文件都做检查，兼容性更好
 */
void ReadBinary(void)
{
	BYTE idata actionTag;
	WORD idata fileLgth ;
	WORD idata fileSize ;
#if UICC__AUTH__5G__SUPI
	UWORD xdata fileAddr;
	BYTE idata result[2];
	WORD xdata fileName[2];
#endif

	actionTag = ComputeBinary(A_ReadFile);

	if(actionTag == FALSE)
	{
		return ;
	}

#if UICC__AUTH__5G__SUPI
	if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
	{
		fileAddr = SelectTFFile( G_DF_ID_USIM, EF_UST);

		result[0] = 0;
		result[1] = 0;
		ReadTFBuf(fileAddr, 0x0F, result, sizeof(result));		
				
		ReadEeprom((BYTE *)&fileName[0],G_CurrentDfAddr[G_ChannelNum] + F_ParentName_Offset,sizeof(fileName)); 
//modify by chentao@20211103 SE or EUICC 都是用4F07
	//	modify by zhxx,GET IDENTITY_附录C/安全条件检查/GET IDENTITY_6.1.8,5FC0/4F07或者5FD0/4F01文件都做检查，兼容性更好
//#if SE_ONLY
		if((fileName[1] == G_DF_ID_USIM)&&(((G_CurrentDfName[G_ChannelNum]==DF_5GS)&&(G_CurrentEfName[G_ChannelNum]==EF_SUCI_CALC_INFO)) 
			|| ((G_CurrentDfName[G_ChannelNum]==DF_SAIP)&&(G_CurrentEfName[G_ChannelNum]==EF_SUCI_CALC_INFO_USIM))))
//#else
		//if((fileName[1] == G_DF_ID_USIM)&&(G_CurrentDfName[G_ChannelNum]==DF_SAIP)&&(G_CurrentEfName[G_ChannelNum]==EF_SUCI_CALC_INFO_USIM))
//#endif
		{
			if((((result[0]&0x18) == 0x18)|| ((result[0]&0x08) == 0x00)))
			{	
				 ConditionSatisfied();//联通需求，EF_SUCI_CALC_INFO文件不能读的时候，抛6985/6283
				return;
			}
		}	

		
		if(((G_CurrentDfName[G_ChannelNum]==DF_PHASE)||(G_CurrentDfName[G_ChannelNum]==G_DF_ID_USIM))&&(G_CurrentEfName[G_ChannelNum]==EF_IMSI) && ((result[1]&0x02) == 0x02))
		{
			ConditionSatisfied();//联通需求，文件不能读的时候，抛6985/6283
			return;		
		}
	}
#endif	
	fileLgth = (((WORD)P1 << 8)&0xFF00 )|(P2 & 0x00FF);

	if(P3==0x00) 					//lgth是256个字节
	{
		//同步移动5g代码，在p3为0时是获取文件大小即所有长度，而不是0x100，并且当大于0x100抛出状态字0x62f1，用例：5G项目测试工程集V1.0.19/CMCC_RFM_RAM_test/RFM_TEST/移动RFM测试(3G)/5.2 REMOTE FILE MANAGEMENT FOR SIM/5.2.2 Coding of the command/5.2.2.2 Output command/OutputCommand_P300_ReadBinary_100_bytes_00
		if (ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
		{
			fileSize = GetTFSize(G_CurrentEfAddr[G_ChannelNum]);

			if ((fileSize-fileLgth) <= 0x100)		//剩余数据小于0x100
			{
				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE xdata *)&RecData[0], fileSize-fileLgth);
				
				GlobalSetStatus(0x9000);
			}
			else									//剩余数据大于0x100
			{
#if UICC__OPERATOR__CHINAMOBILE			
				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE xdata *)&RecData[0],0x100);
				
				GlobalSetStatus(0x62F1);
#endif				
			}			
		}
		else
		{
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE xdata *)&RecData[0],0x100);
		}
	}
	else
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE xdata *)&RecData[0x00],P3);
	}	

	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
	{
		if((SW1 == 0x62)&&(SW2 == 0xF1)) //移动P3为0时候文件长度超过0x100  状态字直接62f1
		{
			return;
		}
		
		GlobalSetStatus(0x9000);
	}
	else
	{
		SendData();
		CommandFinish(0x00);			//send 90 00                                                                     
	}
}


/*
*@function   :IF_ExternalFileUpdate

*@description:文件更新完成后，触发ExternalFileUpdate事件
*
*@see         :	102241
*
*@param		  :position: offset for transparent file
*						 recNum for record  file
*			   len:		 the number of bytes updated
*
*@return	  :FALSE:继续执行后续流程/TRUE:终止后续函数流程,直接返回在当前函数中设置的状态字
*
*@author      :ZZJ
*
*@history	  :V1.0 :create by ZZJ@20131202
              :V1.1 :1： 该函数中最好不要使用到Recdata[0]字段，因为在IF_APDUFInish中有可能会再次使用RECDATA的值进行处理
                     2： 该函数会去触发应用getApplistByEvent，在触发函数中EVENT_EXTERNAL_FILE_UPDATE_bit 会使用到&tempBuf[100]- 119 ，所以中间的函数中不能使用tempBUf[100 -119]在前期缓存数据 V1.1 BY XUYZ@20180115
*             :V1.2 :印度RJIO需求中，要求可以在RFM中触发toolkit应用。 《20180801185012_LYF LS 4510_send CAFA file through (OTA SMS)_PoR OK_URL not POP UP.txt》

*/
#if UICC__FUNCTION__UICCAPI
BYTE IF_ExternalFileUpdate(WORD position,WORD len)
{
	BYTE idata result;
	BYTE idata i, fileType,pathDep;
	UWORD xdata fileAddr;
	BYTE idata buf[2],Applist[20];
	
	if(G_EnvelopHanderAddr == FALSE)
	{
		return FALSE;
	}
	//RFM触发的更新文件不触发applet
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
	{
	#if UICC_RFM_FILE_UPDATE_EVENT	
		IF_SaveRFMRemoteFileUpdate(position, len);
	#endif
		
		return FALSE;
	}
	
	result = FALSE;
	
	//为了防止G_EnvelopHanderAddr的数据被破坏，此处必须保存现场
// 	ToolkitSaveAPPContext();																  //可能在应用中触发 ，需要保护现场

	tempBuf[0] = 0x82;						 //Device Identity

	tempBuf[1] = 0x02;

	tempBuf[2] = 0x82;

	tempBuf[3] = 0x81;

	tempBuf[4] = 0x12;
	
	pathDep = IF_GetCurrentFilePath(&tempBuf[7]);
	
	tempBuf[6] = 0x01;		//File num	

	tempBuf[5] = pathDep *2 +1;	//Length (X) of bytes following
	
	i = 7 + pathDep *2;   //point to the next Tag offset
	
	fileType = 0;
	
	if(pathDep > 2)  //有2级及2级以上的DF
	{
		fileAddr = SelectFile(E_FILE_START,(((tempBuf[9])<<8)|tempBuf[10]));    	//如果当前EF从属与某个ADF,则该ADF NAME 换成7FFF
		
		fileType = GetFileHeadByte(fileAddr,F_DIRNum_offset);
		
		if(fileType != 0x00)
		{			
			ecpMem_Copy(&buf[0], &tempBuf[9], 2);
			
			tempBuf[9] = 0x7F;

			tempBuf[10]	= 0xFF;
			
			tempBuf[i++] = 0x2F;											//ADF   AID
		
			GetCurrentADFAid(((((WORD)buf[0])<<8)|buf[1]),&tempBuf[i]);

			ecpMem_Move(&tempBuf[i], &tempBuf[i+1], tempBuf[i+1]+1);

			i += tempBuf[i] + 1;
		}		
	}
	
	tempBuf[i] = 0x3B;												//File Update Information

	tempBuf[i+1] = 0x04;											//len

	tempBuf[i+2] = position>>8;

	tempBuf[i+3] = position&0xFF;

	tempBuf[i+4] = len>>8;

	tempBuf[i+5] = len&0xFF;

	i += 6;
	
	if(fileType != 0x00)   //getApplistByEvent使用到 tempBuf[9]  所以提前赋值 查找完后还原
	{
		tempBuf[9] = buf[0];
			
		tempBuf[10]	= buf[1];
	}		
	
	result = getApplistByEvent(EVENT_EXTERNAL_FILE_UPDATE_bit,0x00,Applist);  //这里没有判断RFM？

	if(result == 0x00)     //先判断有没有对应的事件 否则每次更新文件都更新6F90等  6F90文件寿命会先达到
	{
		return FALSE;
	}
	
	if(fileType != 0x00)
	{	
		tempBuf[9] = 0x7F;

		tempBuf[10]	= 0xFF;
	}

	//为了防止G_EnvelopHanderAddr的数据被破坏，此处必须保存现场
 	ToolkitSaveAPPContext();																  //可能在应用中触发 ，需要保护现场
	
	result = WriteTFBuf(G_EnvelopHanderAddr, 0, &tempBuf[0], i);

	if(result != FALSE)
    {
		result = FALSE;
		
		if(fileType != 0x00)						//为了与FileList列表信息进行比对，把7FFF换回Adf的fileID
		{
			//tempBuf[9] = ((G_CurrentAdfName[G_ChannelNum]>>8)&0xFF);

			//tempBuf[10]	= G_CurrentAdfName[G_ChannelNum]&0xFF;
			
			tempBuf[9] = buf[0];

			tempBuf[10]	= buf[1];
		}

		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = i;

// 		if(ValueBit(G_SystemStatus, B_RfmPerform) == FALSE)
// 		{
			JavaToolkitProcess(EVENT_EXTERNAL_FILE_UPDATE_bit,0);
// 		}
// 		else 
// 		{
// 			JavaToolkitProcess(EVENT_REMOTE_FILE_UPDATE_bit,0);
// 		}
		
		result = SetSTKBusy();
	}
	
	ToolkitRestoreAPPContext();		

	return result;	
} 

#endif
/**
 * @brief 		       UpdataBinaryCommand:This function updates the current transparent EF with a string of bytes
 *
 * @command format 	   A0 D6 high low lgth /0X D6 high low lgth
 *
 * @Returns		       void
 *
 * @history 	       modify by chentao@EPX产业化\INFENION_SLM97\Develop
 *                   V1.1---兼容写卡组件，当更新3F00/2E00时，实际更新USIM ADF/1B10,by xwh 20210106
 *                   V1.2--modify by zhxx,GET IDENTITY_附录C/安全条件检查/GET IDENTITY_6.1.8,5FC0/4F07或者5FD0/4F01文件都做检查，兼容性更好
 */
void UpdateBinary(void)
{
	BYTE idata  actionTag,tag;
	WORD idata 	fileLgth;
	UWORD xdata fileAddr;
#if UICC__AUTH__5G__SUPI
	BYTE idata result[2],tmp[4],tmp1[4];//根据31.102规范4F0A文件大小只有4个字节
	WORD xdata fileName[2];
#endif

	actionTag = ComputeBinary(A_UpdateFile);

	if(actionTag == FALSE)
	{
		return ;
	}

#if UICC__AUTH__5G__SUPI

	if(JcvmConfig_GetSEState() == CARD_SECURED)//锁卡状态
	{
		if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
		{

			fileAddr = SelectTFFile( G_DF_ID_USIM, EF_UST);

			result[0] = 0;
			result[1] = 0;
			ReadTFBuf(fileAddr, 0x0F, result, sizeof(result));

			ReadEeprom((BYTE *)&fileName[0],G_CurrentDfAddr[G_ChannelNum] + F_ParentName_Offset,sizeof(fileName));
			//modify by chentao@20211103 SE or EUICC 都是用4F07
			//modify by zhxx,GET IDENTITY_附录C/安全条件检查/GET IDENTITY_6.1.8,5FC0/4F07或者5FD0/4F01文件都做检查，兼容性更好
//#if SE_ONLY
		if((fileName[1] == G_DF_ID_USIM)&&(((G_CurrentDfName[G_ChannelNum]==DF_5GS)&&(G_CurrentEfName[G_ChannelNum]==EF_SUCI_CALC_INFO)) 
			|| ((G_CurrentDfName[G_ChannelNum]==DF_SAIP)&&(G_CurrentEfName[G_ChannelNum]==EF_SUCI_CALC_INFO_USIM))))
//#else
			//if((fileName[1] == G_DF_ID_USIM)&&(G_CurrentDfName[G_ChannelNum]==DF_SAIP)&&(G_CurrentEfName[G_ChannelNum]==EF_SUCI_CALC_INFO_USIM))
//#endif
			{
				if((((result[0]&0x18) == 0x18)|| ((result[0]&0x08) == 0x00)))
				{
					 ConditionSatisfied();//联通需求，EF_SUCI_CALC_INFO不能读的时候，抛6895/6283
					return;
				}
			}

			if(((G_CurrentDfName[G_ChannelNum]==DF_PHASE)||(G_CurrentDfName[G_ChannelNum]==G_DF_ID_USIM))&&(G_CurrentEfName[G_ChannelNum]==EF_IMSI) && ((result[1]&0x02) == 0x02))
			{
				ConditionSatisfied();//联通需求，EF_IMSI不能读的时候，抛6895/6283
				return;
			}
		}

	}

#endif



	fileLgth = (((WORD)P1 << 8)&0xFF00 )|(P2 & 0x00FF);


	tag = ReceiveData();

#if SE_ONLY
	if(tag == TRUE)
#endif
	{
#if UICC__AUTH__5G__SUPI	

		if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)&&(G_CurrentDfName[G_ChannelNum] == DF_5GS)&&(G_CurrentEfName[G_ChannelNum] ==EF_ROUTING_INDICATOR)) //远程更新5FC0/4F0A的特殊处理
		{
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],0x00,(BYTE xdata *)&tmp[0],0x04);
		}
		
#endif
#if UICC__OPERATOR__CHINAMOBILE		
		//兼容写卡组件，当更新3F00/2E00时，实际更新USIM ADF/1B10,by xwh 20210106
		if((G_CurrentDfName[G_ChannelNum] == DF_MF)&&(G_CurrentEfName[G_ChannelNum] == EF_AuthCFG_TPX)) 
		{
			fileAddr = SelectTFFile(G_DF_ID_USIM,EF_PRIVATE_AUTHCFG);			
			actionTag = WriteTFBuf(fileAddr,fileLgth,(BYTE xdata *)&RecData[0],P3); 
		}
		else
#endif			
		{
			actionTag = WriteTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE xdata *)&RecData[0],P3); 
		}
	
		if (actionTag == FALSE)
		{
			MemoryError();											//65 81/92 40

			return;
		}
#if UICC__OPERATOR__CHINAUNICOM		
		IF_UpdateBinary(fileLgth);//modify by zhxx@20230330,CTCC 1.1.B code review ,UNICOM need,更新标识位，表示已经登录过网络,UnicomResetIMSI need
#endif
	}
#if UICC__AUTH__5G__SUPI
	if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)&&(G_CurrentDfName[G_ChannelNum] == DF_5GS)&&(G_CurrentEfName[G_ChannelNum] ==EF_ROUTING_INDICATOR)) //远程更新5FC0/4F0A的特殊处理
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],0x00,(BYTE xdata *)&tmp1[0],0x04);
		if((ecpMem_Compare(&tmp[0], &tmp1[0], 0x04) != EQUAL))
		{								
			PendingMessageAlert(D_RefreshUpdateRoutingID,0);//值有改变时才进行rfresh
		}
	}
	#endif
#if (EUICC_ONLY||SE_AND_EUICC)
	//更新3F00/ICCID,或者USIM/6F07，USIM/GID1，USIM/GID2时候同步更新到MetaData中
	if(((G_CurrentDfName[G_ChannelNum] == DF_MF) && (G_CurrentEfName[G_ChannelNum] == EF_ICCID)) || ((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM) && ((G_CurrentEfName[G_ChannelNum] == EF_IMSI) /*|| (G_CurrentEfName[G_ChannelNum] == 0x6F3E) || (G_CurrentEfName[G_ChannelNum] == 0x6F3F)*/)))
	{
		IF_UpdateFileToMetaData(P3,(BYTE xdata *)&RecData[0],fileLgth, G_CurrentEfName[G_ChannelNum]);
	}
#endif
	if(G_CommandMode == Command_Mode_3G)   //3G下分配服务列表或修改EF_EST文件（enabling/disabling FDN服务）均有可能改变FDN服务状态 ，需要同步到2G模式
	{
#if (!UICC__GSM_2G_NOT_SUPPORT)		
		if((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)&&((G_CurrentEfName[G_ChannelNum] ==EF_EST)||(G_CurrentEfName[G_ChannelNum] ==EF_UST)))
		{
			tag = IsFdnServiceActiveInUSIM();	  //获取fdn服务在USIM模式下的状态
			
			UsimFdnSYNCtoSIM(tag);				//同步到SIM
		}
#endif		
	}
	else
	{
#if (!UICC__GSM_2G_NOT_SUPPORT)		
		 if((G_CurrentDfName[G_ChannelNum] == DF_PHASE )&&(G_CurrentEfName[G_ChannelNum] == EF_SST))//2G模式下分配服务列表或 激活/失效 EF_ADN文件 可能改变FDN状态 需要同步到3G模式
		 {
			 tag = IsFdnServiceActiveInSIM();	   //获取fdn服务在SIM模式下的状态
				 
			 SimFdnSYNCtoUSIM(tag);			 	 //同步的USIM
		 }
#endif		 
	}
	
	if((INS == 0xD6)&&(G_CurrentEfName[G_ChannelNum] == 0x6F54)&&(G_CurrentDfName[G_ChannelNum] == 0x7F10)) //UICC API:测试用例Cre_Pcs_Spco---CRRN3: During a CAT session the CAT Runtime Environment shall send a SET UP MENU system proactive command whenever the EFSUME file under the DFTELECOM file is updated as defined in TS 102 222 [7].
	{
		SetG_ToolkitSystemProactiveFlag(Bit_SetUpMenu);
	}
	
#if UICC__FUNCTION__UICCAPI
  	if(IF_ExternalFileUpdate(fileLgth, P3) == FALSE)
	{
  		CommandFinish(0x00);                                            //SEND 90 00
	}
#else
	CommandFinish(0x00);   
#endif
}
/*
*@function   :GetPhyNum

*@description:获取记录/循环文件的物理记录号
*
*@see         :	102221
*
*@param		  :BYTE action --     A_UpdateFile/ A_ReadFile
*			   UWORD fileAddr--	  所操作的文件地址
*			   BYTE mode--		PRIVOUS_RECORD,NEXT_RECORD,ABSOLUTE_CURRENT
*              BYTE accRecord -- 指令中传进来的记录号，ABSOLUTE_CURRENT模式会使用到
               BYTE* currentRec --  相对记录号的地址
*@return	  :物理记录号
*
*@author      :zhxx
*
*@history	  :V1.0 :create by zhxx@20200730          

*/
BYTE GetPhyNum(BYTE action,UWORD fileAddr,BYTE mode,BYTE accRecord,BYTE* currentRec)
{
	BYTE idata logicFirst,recNum,phyNum,maxRecNum,fileType;
	
	fileType = (GetFileHeadByte(fileAddr, F_Type_Offset) & 0x07);
	
	logicFirst = 0;
	
	if(fileType == CYCTYPE) //只有循环文件才会使用到logicFirst
	{
		logicFirst = getCycleFPoint(fileAddr); //获取基址，即逻辑上的第一条记录的记录号
	}	
	
	maxRecNum = GetLFRecNumLgth(fileAddr,FileNum);  //获取文件的总记录个数
	
	if((fileType == CYCTYPE) && (action == A_UpdateFile))//For a cyclic EF the record containing the oldest data is updated, the record pointer is set to this record and this record becomes record number 1.
	{
		recNum = 1;
		
		phyNum = logicFirst - 1;

		if(phyNum == 0)
		{
			phyNum = maxRecNum;
		}
	}
	else
	{
		recNum = GetRecordNo(fileAddr,fileType, mode, accRecord,*currentRec); 
		
		if(fileType == CYCTYPE)
		{
			phyNum = ((logicFirst + recNum - 1) > maxRecNum) ? (logicFirst+recNum-1 - maxRecNum): (logicFirst+recNum-1);
		}
		else
		{
			phyNum = recNum;
		}
	}	
	
	if((recNum == FALSE) || (phyNum == FALSE))
	{
		return FALSE;
	}
	
	if(mode != ABSOLUTE_CURRENT)
	{
		*currentRec = recNum;
	}
		
	return phyNum;
}

/*
*@description:根据记录模式获取记录文件或循环文件的相对记录号
* @param      UWORD fileAddr -- 所操作文件地址
*@param      :BYTE type--文件类型：循环 or 线形
*@param      :BYTE mode :PRIVOUS_RECORD,NEXT_RECORD,ABSOLUTE_CURRENT
*@param      :BYTE record: 传入的记录号，ABSOLUTE_CURRENT才会用到
*@param     :BYTE currentRec --  逻辑记录号
*
*@history  :create by zhxx @20200730
*/
BYTE GetRecordNo(UWORD fileAddr,BYTE type,BYTE mode,BYTE record,BYTE currRec)
{		
	BYTE idata isLinearFile, maxRecNum, rtnNo;

	maxRecNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	if(maxRecNum == 0) return 0;

	isLinearFile = (type == LINTYPE);

	switch(mode & 0x0F) {
		case NEXT_RECORD: 				
			if(currRec == 0x00)
				rtnNo = 1;
			else 
				rtnNo = ((currRec+1) <= maxRecNum) ? (currRec+1) : (isLinearFile ? 0 : 1);
			break;

		case PRIVOUS_RECORD:			
			if(currRec == 0x00)
				rtnNo = maxRecNum;
			else 
				rtnNo = ((currRec-1) > 0) ? (currRec-1) : (isLinearFile ? 0 : maxRecNum);
			break;

		default: //ABSOLUTE_CURRENT
			if(record == 0x00) //CURRENT
				rtnNo = (currRec>=1)&&(currRec<=maxRecNum) ? currRec : 0x00;
			else //ABSOLUTE
				rtnNo = (record>=1)&&(record<=maxRecNum) ? record : 0x00;
			break;
	}
		
	return rtnNo;		
}
/**
 * @brief 		   	   ComputeRecord:This function is to compare the file condition is do or not
 *
 * @param              A_ReadFile/A_UpdateFile 	   
 *
 * @Returns		       recNo/FALSE
 *
 * @history 	       
 */
BYTE ComputeRecord(BYTE action)
{
	RecordFileHead xdata  fileHead;
	BYTE idata tag,phyNum,currentRec;
	
	tag = (G_CommandMode == Command_Mode_2G)?P2:(P2&0x07);

	if((tag != NEXT_RECORD)&&(tag != PRIVOUS_RECORD)&&(tag != ABSOLUTE_CURRENT))
	{
		P1P2Error();														//Send 6B 00
		return FALSE;
	}

	if((P1 != 0x00)&&((tag == NEXT_RECORD) || (tag == PRIVOUS_RECORD)))
	{
		if(G_CommandMode == Command_Mode_3G)  								//2G在此情况下要求不解释P1，参考GSM11.11
		{
			P1P2Error();													//Send 6B 00
			return FALSE;		
		}
	} 

	if((P2&0xF8)!= 0x00)  //2G模式下P2 为02 03 04 ，前面已经判断过，所以此处一定是3G 的代码
	{
		tag = (P2>>3)&0x1F;
		
		if(tag == 0x1F)
		{
			P1P2Error();
			return FALSE;
		}
		
		P2  = (P2 &0x07);
	}
	else
	{
		tag =0x00;
	}
	
	tag =ComputeAll(action,tag,0);
		
	if(tag == FALSE)
	{
		return FALSE;
	}

	ReadEeprom((BYTE xdata *)&fileHead.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(RecordFileHead));
	
	if((action == A_UpdateFile)&&((fileHead.FileType&0x37) == CYCTYPE) && (P2 !=PRIVOUS_RECORD))
   	 {
		if(G_CommandMode == Command_Mode_2G)
		{
			P1P2Error();													//Send 6B 00												//94 08
		}
		else
		{
			FileStructureError();											//69 81
		}
		return FALSE;
	}       
	
	if( P2 == ABSOLUTE_CURRENT)
	{	
		if(P1 > fileHead.RecNumber)
		{
			AddressInvilid();													//94 02	

			return FALSE;
		}
	}
	
	currentRec = G_CurrentRecNum[G_ChannelNum];

	phyNum =  GetPhyNum(action,G_CurrentEfAddr[G_ChannelNum],P2,P1,&currentRec);

	if(phyNum == FALSE)
	{
		AddressInvilid();		//94  02
												
		return FALSE;
	}
    
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
	{
		if((action == A_ReadFile)&&(P3 == 0x00))   //对于RFM 操作而言，P3 = 0,返回记录文件整条记录内容
		{
			P3 = fileHead.RecLength;
			return TRUE;
		}
	}
	

    if((P3 == 0x00)&&(G_CommandMode == Command_Mode_3G))
    {
   #if (!UICC__OPERATOR__CHINAUNICOM)
		if(action == A_UpdateFile)
		 {
			P3Error();	
		 }
		 else//4-1-E_RFM(SMS)_UICC/UICC/017_UICC_UpdateRecord,00A4000C027F1000A4000C026F3A 00DC010400(RFM) 实际值与期望值不同。期望值:036C1C 实际值:036700
	#endif
		 {
			 RepeatPreviousCmdHeader(fileHead.RecLength);
		 }

        return FALSE;
    }
	

	if(P3 != fileHead.RecLength)
	{
		P3Error();														//Send 67 00

		return FALSE;
	}	

	G_CurrentRecNum[G_ChannelNum] = currentRec;

	return phyNum;	
}

/**
 * @brief 		       ReadRecordCommand:This function read one record from current fixed EF or cyclic EF
 *
 * @command format 	   A0 B2 recNo mode lgth /0X B2 recNo mode lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void ReadRecord(void)
{
	BYTE idata phyNum;

	phyNum = ComputeRecord(A_ReadFile);

	if(phyNum == FALSE)
	{
		return;
	}

	ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],phyNum,0,(BYTE xdata *)&RecData[0],P3);	//get the content of the current record.
	
    SendData();	
	
	CommandFinish(0x00);
}

/**
 * @brief 		       UpdateRecordCommand:This function update one record from current fixed EF or cyclic EF
 *
 * @command format 	   A0 DC recNo mode lgth /0X DC recNo mode lgth
 *
 * @Returns		       void
 *
 * @history 
 *                   V1.1---兼容写卡组件，当更新7F20/6F1B时，实际更新USIM ADF/1B1B,by xwh 20210106	       
 */
//extern BYTE ToolkitProcessSMSUpdate(void);

void UpdateRecord(void)
{
	BYTE idata lgth,i,phyNum,transActioinFlag;
	BYTE idata tag,tmp,fileType;
	WORD idata fileId;
	UWORD xdata fileAddr;
		
	phyNum = ComputeRecord(A_UpdateFile);

	if(phyNum == FALSE)
	{
		return;
	}

	ReceiveData();

#if UICC__OPERATOR__CHINATELECOM
	if((G_CurrentEfName[G_ChannelNum] == EF_SQNCheck) && (G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)) //
	{
		SetGBAAuthMode(AUTH_MODE_GBA_CLEAR);
	}
#endif

#if UICC__OPERATOR__CHINATELECOM
#if !CDMA_2G_NOT_SUPPORT
	if((RecData[0] == 0x03)&&(G_CurrentEfName[G_ChannelNum] == EF_SMS)&&((G_CurrentDfName[G_ChannelNum] == G_DF_ID_CSIM)||(G_CurrentDfName[G_ChannelNum] == DF_CDMA)))//ifdef PRL
#else
	if((RecData[0] == 0x03)&&(G_CurrentEfName[G_ChannelNum] == EF_SMS)&&(G_CurrentDfName[G_ChannelNum] == G_DF_ID_CSIM) )//ifdef PRL
#endif
	{
			UpdatePLMNList(&RecData[0],phyNum);
	}
#endif


	if((G_CurrentDfName[G_ChannelNum] == 0x7F10)&&(G_CurrentEfName[G_ChannelNum] == 0x6F3A))
	{
		fileAddr = SelectLFFile(0x7F10,0x5F3A);		

		if(fileAddr != FALSE)
		{
		fileAddr =  SelectFile(fileAddr,0x4F30);  //查找5F3A下EFPBR的地址

		//这里需要同步的EFPBC一般默认存储在4F30的第一条记录，因为文件系统中一般配置成第一条记录所指示的电话本与7F10/6F3A具有映射关系.
		//若需求指示其他电话本与7F10/6F3A有映射关系，此处需要修改，请咨询COS部门

			if(fileAddr != FALSE)  //读取EFPBR的第一条记录，获取对应的EFPBC 的文件ID 。（因为第一条记录对应的号簿和7F10/6F3A有映射关系）.
			{
				lgth = GetLFRecNumLgth(fileAddr,FileLgth);

				ReadLFBuf(fileAddr,0x01,0x00,&tempBuf[0],lgth);

				for(i = 0x00; i< lgth; i++)  //过滤掉0xFF字段
				{
					if(tempBuf[i] == 0xFF)
					{
						break;
					}
				}

				lgth = i; //实际有效数据长度为i

				i = FindTag(&tempBuf[0],0xA8,lgth); //返回A8长度的偏移位置

				if(i != FALSE)
				{
					lgth = tempBuf[i];

					tmp = FindTag(&tempBuf[i+1],0xC5,lgth); //C5tag中包含的是EFPBC的信息

					if(tmp != FALSE)
					{
						fileId = (((WORD)tempBuf[i+1+tmp+1]) <<8)|(tempBuf[i+1+tmp+2]);

						fileAddr = SelectLFFile(0x7F10,0x5F3A);

						fileAddr = SelectFile(fileAddr,fileId);

						if ((fileAddr != FALSE)&&(G_CommandMode == Command_Mode_2G))
						{
						SetLFConst(fileAddr, phyNum, 0x00, 0x01, 0x01);
						}
					}
				}
			}
		}
	}
#ifdef DATASEC
	if((G_CurrentDfName[G_ChannelNum] == 0x7F20)&&(G_CurrentEfName[G_ChannelNum] == 0x6F1B))
	{
		for(i=0;i<P3;i++)
		{
			RecData[i]=RecData[i]^6;
		}
	}
#endif

	transActioinFlag = FALSE;
	
	transActioinFlag = IF_BeginTransaction();
	
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = IF_UpdateCycleRecPointer(G_CurrentEfAddr[G_ChannelNum]);	
#endif

#if UICC__OPERATOR__CHINAMOBILE		
		//兼容写卡组件，当更新7F20/6F1B时，实际更新USIM ADF/1B1B,by xwh 20210106
		if((G_CurrentDfName[G_ChannelNum] == DF_PHASE)&&(G_CurrentEfName[G_ChannelNum] == EF_Ki_TPX)) 
		{
			fileAddr = SelectTFFile(G_DF_ID_USIM,EF_KI);						
			tag = WriteLFBuf(fileAddr,phyNum,0,(BYTE xdata *)&RecData[0],P3);
		}
		else
#endif			
		{
			tag = WriteLFBuf(G_CurrentEfAddr[G_ChannelNum],phyNum,0,(BYTE xdata *)&RecData[0],P3);
	  }
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(G_UpdateCycleRecPointer == FALSE)
#endif
	{
		fileType = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);
		
		if((P2 != ABSOLUTE_CURRENT) && ((fileType & 0x07) == CYCTYPE))//只有循环文件才会更新循环记录指针
		{	
			VMSetBuf(getCycleFPointAddr(G_CurrentEfAddr[G_ChannelNum]),phyNum,sizeof(BYTE));	
		}
	}
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = FALSE;
#endif
	if(tag == FALSE)
	{
		MemoryError();													//65 81/92 40
		
		IF_AbortTransaction(transActioinFlag);

		return;
	}
	
	IF_CommitTransaction(transActioinFlag);
	
#if UICC__FUNCTION__STKAPI
	if(G_CurrentEfName[G_ChannelNum] == EF_SMS)	 //USIMAPI Test_ufw_Inda测试，更新ADF1下6F3C文件触发EVENT_FORMATTED_SMS_PP_UPD事件，取消对DF的判断 lj 20140214
	{
	   if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)					//RFM的update SMS操作不触发SMSUpdate event，by xwh 20131122
	   {
	   		ToolkitProcessSMSUpdate();	   	   
	   }
	}
#endif

#if UICC__FUNCTION__UICCAPI
	if(IF_ExternalFileUpdate(phyNum,P3) == FALSE)
	{
		CommandFinish(0x00);
	}
#else
	CommandFinish(0x00);
#endif
}
/**
 * @brief 		      SeekCommand:This function searchs a specific pattern from current fixed EF or cyclic EF
 *
 * @command format 	   A0 A2 00 type/mode lgth /0X A2 recNo mode lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void SeekCommand(void)
{
	signed char step;
	BYTE idata tag;
	BYTE idata recNumStart;	

	BYTE idata offset;
	BYTE idata length;
	BYTE * searchString;

	RecordFileHead xdata fileHead;

	if( ((G_CommandMode == Command_Mode_2G) && ((P1!= 0x00)||(((P2>>4)&0x0F)>0x01) || ((P2&0x0F)>0x03) ) )
		||(( ((P2&0xF8) == 0xF8)||((P2&0x04) == 0x00) )&&(G_CommandMode == Command_Mode_3G)))
	{
		P1P2Error();												//6B 00
		return ;
	}

	if( (G_CommandMode == Command_Mode_3G)&&((P2&0xF8) != 0x00))	//使用 SFI
	{
		tag = (P2 >> 3)&0x1F;										//tag as SFI
		
		if(tag == 0x1F)
		{
			P1P2Error();
			return;
		}
	}
	else
	{
		tag =0x00;
	}
	
	tag=ComputeAll(A_ReadFile,tag,4);
		
	if(tag == FALSE) 
	{
		return ;
	}
	
	ReadEeprom((BYTE xdata *)&fileHead.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(RecordFileHead));

	if((G_CommandMode == Command_Mode_2G)&&((P3>0x10)||(P3>fileHead.RecLength)||(P3 == 0x00)))        //check the range of P3.
	{
		P3Error();                                             //67 00
		return ;
	}

	if(G_CommandMode == Command_Mode_3G)
	{	
		if(P1 > fileHead.RecNumber)
		{
			P1P2Error();

			return;
		}
	}  
	
  
	ReceiveData();
	
	step = 1;                                            //默认为forward

	offset = 0x00;											//默认是0偏移            

	tag = FALSE;											//默认是不使用first occurrence方式

	searchString = &RecData[0];								

	length = P3;
	
	if(G_CommandMode == Command_Mode_2G)                   //2G模式search
	{
		switch(P2&0x0F)
		{
			case FRM_BGN_FRD :
			{ 
				recNumStart = 0x01;

				break;
			}
			case FRM_LST_BFD : 
			{
				step = -1;

				recNumStart = fileHead.RecNumber;
			
				break;
			}
			case FRM_CUR_FRD : 
			{
				recNumStart = (G_CurrentRecNum[G_ChannelNum] == 0x00) ? 0x01:(G_CurrentRecNum[G_ChannelNum]+0x01);

				break;
			}
			default : 
			{
				step = -1;									//backward

				recNumStart = (G_CurrentRecNum[G_ChannelNum] == 0x00) ? (fileHead.RecNumber):(G_CurrentRecNum[G_ChannelNum]-0x01);

				break;
			}
		}
	}
	else													//3G模式search
	{
		if((P2&0x06) == 0x04)									//SIMPLE MODE
		{
			if(P1 == 0x00)                                      //current record
			{
				if(G_CurrentRecNum[G_ChannelNum] == 0x00)
				{
					AddressInvilid();							//6A 83
					return;
				}
	
				recNumStart = G_CurrentRecNum[G_ChannelNum];
			}
			else
			{
				recNumStart = P1;
			}
	
			if((P3 == 0x00) || (P3 > fileHead.RecLength))
			{
				P3Error();										//67 00
				return; 
			}
	
			if((P2&0x01) == 0x01)								//backward
			{
				step = -1;
			}
	
		}
		else if((P2&0x07) == 0x06)								//ENHANCED MODE		
		{
			if((P3 < 0x03) || (P3 > (fileHead.RecLength+0x02))) //data = search indication + search  string
			{
				P3Error();										//67 00
				return;
			}
	
			if(((RecData[0]&0x04) == 0x00) || ((RecData[0]&0xF0) != 0x00)) 
			{
				IncorrentParameter();							//6A 80
	
				return;			
			}
	
			if((RecData[0]&0x08) == 0x08)						//After First Occurrence search
			{
				tag = TRUE;
			}
			else												//Offset search
			{
				if(RecData[1] + (P3-2) > fileHead.RecLength)
				{
					IncorrentParameter();						//6A 80
					return;
				}
				
				offset = RecData[1];
			}
	
			if((RecData[0]&0x02) == 0x00)						//ABSOLUTE
			{
				if(P1 == 0x00)
				{
					if(G_CurrentRecNum[G_ChannelNum] == FALSE)
					{
						AddressInvilid();						//6A 83
						return;
					}
	
					recNumStart = G_CurrentRecNum[G_ChannelNum];
				}
				else   
				{
					recNumStart = P1;
				}
	
				if((RecData[0]&0x01) == 0x01)					//backward
				{
					step = -1;
				}				
			}
			else                                               //forward or  backward
			{
				if(P1 != 0x00)
				{
					P1P2Error();								//6b 00
					return;
				}
				
				if((RecData[0]&0x03) == 0x03)                   //backward
				{
					step = -1;
				}
				if(G_CurrentRecNum[G_ChannelNum] == FALSE)		//未指定
				{
					if(step == 0x01)                              //forward
					{
						recNumStart = 0x01;
					}
					else
					{					
						recNumStart = fileHead.RecNumber;
					}
				}
				else                                                     //CF
				{
					if((fileHead.FileType&0x07) == LINTYPE)
					{
						recNumStart = G_CurrentRecNum[G_ChannelNum] + step;  //search函数保证了 G_CurrentRecNum[G_ChannelNum] == fileHead.RecNumber||0x01时的错误返回
						if((recNumStart > (fileHead.RecNumber)) || (recNumStart == 0))  // // add by chentao@20230518  增加异常判断
						{
							if(G_CommandMode == Command_Mode_2G)
							{
								 FileNotFound();                           //94 04
							}
							else
							{
									AddressInvilid();							//6A 83
							}
							return;
						}
					}
					else//循环文件
					{
						recNumStart = G_CurrentRecNum[G_ChannelNum];
						
						if(step == 1)
						{
							recNumStart = ((recNumStart == (fileHead.RecNumber))?0x01 : (recNumStart +1));	
						}
						else
						{
							recNumStart = (recNumStart == 0x01?(fileHead.RecNumber) : (recNumStart-1));
						}
					}
				}
			}
			searchString = &RecData[2];
	
			length = P3 -0x02;
		}
		else									//PROPRIETARY, FUNCTION NOT SUPPORT
		{
			FunctionNotSupport();				//6A 81
			return;
		}
	}

	if(G_CommandMode == Command_Mode_2G)
	{
		tag = SearchStarting(recNumStart, fileHead.RecNumber, step);

		 tempBuf[0] = tag;
	}
	else	//Command_Mode_3G
	{
		tag = SearchContaining(step,recNumStart,offset,searchString,length,tag);
	}

	if(tag == 0x00)
	{
		if(G_CommandMode == Command_Mode_2G)
		{
			 FileNotFound();                           //94 04
		}

		else
		{
	    	AddressInvilid();							//6A 83
		}

		return;
	}			
	
	G_CurrentRecNum[G_ChannelNum] = tempBuf[0];

	if(G_CommandMode == Command_Mode_2G)
	{
		if((( P2 >> 4 )&0x0F) == 0x01)
		{
			RecData[0] = tempBuf[0];

			CommandFinish(0x01);
		}
		else
		{ 
		    CommandFinish(0x00);
		}   			             
	}
	else
	{
		ecpMem_Copy(&RecData[0],&tempBuf[0],tag);

		CommandFinish(tag);							//USIM 用61 XX 代替 SIM 中的 9F XX  
	}
}
/**
 * @brief 		      IncreaseCommand:This function add the value to the last increase/update record of the current CF
 *
 * @command format 	   A0 32 00 00 03 /0X 32 00/SFI 00 lgth(1~127bytes)
 *
 * @Returns		       void
 *
 * @history 	       
 */
void IncreaseCommand(void)
{
	RecordFileHead xdata  cyclicEf;
	BYTE idata tag,fistNum,secondNum;
	WORD xdata totalValue;
	BYTE idata middleValue,RecIndicator,transActioinFlag;

	transActioinFlag = FALSE;

	if(P2 != 0x00)
	{
		P1P2Error();                                              //6B 00
		return;
	}  
	

	if ((P3>0x7F)||(P3==0x00))
	{
		P3Error();
		return;	
	}         

	if (G_CommandMode == Command_Mode_2G)
	{
#if !UICC__OPERATOR__CHINATELECOM
		if(P3 != 0x03)
		{
			P3Error();				 //6700

			return;	
		}
#endif
		if(P1 != 0x00)
		{
			P1P2Error();  		       //6B 00
			                                            
			return;
		}
	}
	

	if((P1&0xE0)==0x80)
	{	
		P1 = P1 & 0x1F;
		
		if((P1 ==0x1F)||(P1 == 0x00))  //SFI for 01 - 30
		{
			P1P2Error();

			return;
		}	
	}
	else // if(P1 != 0x00)
	{
		if(P1 != 0x00)
		{
			P1P2Error();

			return;
		}
	}


	tag = ComputeAll(A_IncreaseFile,P1,3);
		
	if(tag == FALSE) 
	{
		return ;
	}

	ReadEeprom((BYTE xdata *)&cyclicEf.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(RecordFileHead));

	RecIndicator = getCycleFPoint(G_CurrentEfAddr[G_ChannelNum]);

 	if((G_CommandMode == Command_Mode_3G)&&(cyclicEf.RecLength > 0x7F))
	{
    	P3Error();

   		return;
	} 

	
	ReceiveData();

	ecpMem_Copy((BYTE xdata *)&RecData[0x80],(BYTE xdata *)&RecData[0],P3);

	ReadEeprom((BYTE xdata *)&RecData[0],getCycleFPointAddr(G_CurrentEfAddr[G_ChannelNum])+ (RecIndicator-1)*cyclicEf.RecLength + 1,cyclicEf.RecLength);//读取当前记录指针所指记录值

	fistNum = cyclicEf.RecLength<=P3?cyclicEf.RecLength:P3;

	secondNum = cyclicEf.RecLength>P3?cyclicEf.RecLength:P3;

	middleValue = 0x00;

	for(tag = 1;tag <= fistNum; tag++)
	{   
		totalValue = middleValue + RecData[cyclicEf.RecLength-tag] + RecData[0x80-tag+P3] ;

		RecData[cyclicEf.RecLength-tag] = 0xFF & totalValue;

		middleValue = (totalValue >> 8) & 0x01;
	}

	if(cyclicEf.RecLength < P3)										//record lgth < P3
	{
		for(tag = fistNum;tag < secondNum;tag++) 
		{
			totalValue = middleValue  +  RecData[0x80+secondNum - 1 - tag ] ;

			RecData[0x80+secondNum - 1 - tag ] = 0xFF & totalValue;

			if(RecData[0x80+secondNum - 1 - tag ]>0x00)				//max value reached
			{
				MaxReached();										//98 50

				return;
			}

			middleValue = (totalValue >> 8) & 0x01;
		}
	}
	else															//record lgth >= P3
	{
		for(tag = fistNum;tag < secondNum;tag++) 
		{
			totalValue = middleValue  +  RecData[secondNum - 1 - tag ] ;

			RecData[secondNum - 1 - tag ] = 0xFF & totalValue;

			middleValue = (totalValue >> 8) & 0x01;
		}
	} 
 
	if( middleValue == 0x01 )
	{
		MaxReached();												//98 50
		return;
	}                                                   

	ecpMem_Copy((BYTE xdata *)&RecData[cyclicEf.RecLength],(BYTE xdata *)&RecData[0x80],P3);

	if(RecIndicator == 1)
	{
		RecIndicator = cyclicEf.RecNumber;
	}
	else
	{
		RecIndicator = RecIndicator - 1;
	}

	G_CurrentRecNum[G_ChannelNum] = 1;
	
	transActioinFlag = IF_BeginTransaction();;
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = IF_UpdateCycleRecPointer(G_CurrentEfAddr[G_ChannelNum]);	
#endif		
	tag = WriteLFBuf(G_CurrentEfAddr[G_ChannelNum],RecIndicator,0,(BYTE xdata *)&RecData[0],cyclicEf.RecLength);
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	if(G_UpdateCycleRecPointer == FALSE)
#endif
	{	
		VMSetBuf(getCycleFPointAddr(G_CurrentEfAddr[G_ChannelNum]),RecIndicator,sizeof(BYTE));
	}
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
	G_UpdateCycleRecPointer = FALSE;
#endif	
	if(tag == FALSE)
	{ 
		MemoryError();													//65 81/92 40
		
		IF_AbortTransaction(transActioinFlag);

		return;
	}

	IF_CommitTransaction(transActioinFlag);

#if UICC__FUNCTION__UICCAPI
	tempBuf[0] = ReadVarible(Tag_BakIncreaseResponseData,&tempBuf[1]);

	if(tempBuf[0] == FALSE)	//为防止出现Increase指令中触发Toolkit应用，且该Toolkit应用要发送主动式命令，主动式命令未结束前又接收到Increase指令，则有可能冲掉该buf，则目前只支持一级increase中出发App
	{
		tempBuf[0] = P3;

		ecpMem_Copy(&tempBuf[1],&RecData[0x80],P3);

		WriteVariable(Tag_BakIncreaseResponseData,&tempBuf[0]);	 //将RecData[0x80]后的数据备份起来

		//if(IF_ExternalFileUpdate(RecIndicator,cyclicEf.RecLength) == TRUE)	 //该函数中可能会冲掉RecData的数据
		if(IF_ExternalFileUpdate(G_CurrentRecNum[G_ChannelNum],cyclicEf.RecLength) == TRUE)	 //该函数中可能会冲掉RecData的数据 // modify chentao@20240118 20.远程文件更新事件测试\Event_Remote_File_Update_Test
		{
			ClearVariable(Tag_BakIncreaseResponseData);	 //Toolkit busy , 清除当前缓存

		   	return;
		}
	
		tempBuf[0] = ReadVarible(Tag_BakIncreaseResponseData,&tempBuf[1]);	 	//恢复RecData数据

		ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],RecIndicator,0,(BYTE xdata *)&RecData[0],cyclicEf.RecLength);	 //读取响应数据的前半段

		ecpMem_Copy(&RecData[cyclicEf.RecLength],&tempBuf[1],tempBuf[0]);

		ClearVariable(Tag_BakIncreaseResponseData);	   //清除tag变量
	
		CommandFinish(cyclicEf.RecLength + tempBuf[0]);
	}
	else     //Tag_BakIncreaseResponseData 已经备份了数据，说明当前Increase是从STK中进来的，为简化操作 ，则不再触发应用
#endif
	{
		CommandFinish(cyclicEf.RecLength + P3);
	}
}

/**
 * @brief 		      This function  invalidates  or Rehabilidate the EF
 *
 * @command format 	   A0 04 00 00 00 /0X 04 mode 00 data
 * @command format 	   A0 44 00 00 00 /0X 44 mode 00 data
 *
 * @Returns		       void
 *
 * @history 	       
 */
void FileStatusHandler(void)
{
	BYTE idata fileStatus;
	BYTE idata tempTag;
	BYTE idata fileAccStatus,Status;

	UWORD xdata fileAddr;
	WORD  xdata fileName;
	PathInfo xdata filePathInfo;
	//BYTE idata i;

	
	UWORD xdata actualFileAddr;

	if(G_CommandMode == Command_Mode_2G)
	{
		if(P1P2P3Check(0x00,FALSE) ==FALSE)
		{
			return ;
		}
	}
	else										//Command_Mode_3G
	{
		if(P2 != 0x00)  
		{
			P1P2Error();						//6B 00
			return;
		}
	
		BackFilePath(&filePathInfo);	     	//备份当前路径	

		ReceiveData();
	}

	fileAddr = FALSE;  //初始化fileAddr，该值未初始化时，按照逻辑流程，不会影响代码执行

	if(P1 == 0x00)                                   
	{

		if((P3 != 0x00)&&(P3 != 0x02))
		{
			P3Error(); 								//6700
			return ;
		}

		
		if((P3 == 0x00)&&(G_CurrentEfAddr[G_ChannelNum] == FALSE))
		{
			NoEfSelected();				  			//69 86
		
			return;
		}

		if(P3 == 0x02)
		{
			fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];

		   	fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName);

		   	if(fileAddr != FALSE)
		   	{
				tempTag = GetFileHeadByte(fileAddr,F_Type_Offset);				//路径选择判断文件类型
				
				if(tempTag == DFTYPE)
				{
					NoEfSelected();											//69 86									
					return;
				}
				
			   if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x00, NULLAPI_Mode) == FALSE)  //若当前选中的是某个非共享文件，且该文件已经被其他通道或 fileView对象所占用，则返回6a82
			   {
					 ConditionSatisfied();	 				// 69 85
									
					return;	
			   }				
				SetChannel(fileAddr,fileName,FALSE);
		   	}
			else
			{
			 	FileNotFound();												//6A 82
				return;
			}
		}	

	}
	else if((P1==0x08)||(P1==0x09))
	{
		if((P3 == 0x00)||((P3%0x02) == 0x01))
		{
			P3Error(); 														//send 6700
			return ;
		}

		fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];
		
		if(P1 == 0x08)
		{
			if(fileName == 0x3F00)   					//IN case of select path from MF ,the terminal shall not use the FID of MF at the begining of the path
			{
				ConditionSatisfied();					// 69 85
				return;
			}
			
		 	fileAddr = E_FILE_START;
		}

		if(P1 == 0x09)
		{
			if(fileName == 0x7FFF)						// In the case of "select by path from current DF", the terminal shall not use the special file-ID '7FFF' at the beginning of the path.TS 102.221 P55.2010-03-10
			{
				ConditionSatisfied();					// 69 85
				return;
			}
			
		 	fileAddr = G_CurrentDfAddr[G_ChannelNum]; 
		}

		if((fileName == G_CurrentDfName[G_ChannelNum])&&(P1 == 0x09))		// In the case of "select by path from MF" or "select by path from current DF", the terminal shall not use the file identity of the current DF.TS 102.221 P55.2010-03-10
		{
			FileNotFound();													//6A 82,restriction 3 also need consider - 2010-03-23
			
			return;
		}		
			
// 		for(i=0x00;i<P3;i=i+2)
// 		{
// 			fileName = (((WORD)RecData[i]<<8)&0xFF00) + RecData[i+1];

// 			if(fileName == 0x7FFF)
//             {
// 				fileName = G_CurrentAdfName[G_ChannelNum];
//             }
// 			
// 		   	fileAddr = SelectFile(fileAddr,fileName);

// 		   	if(fileAddr != FALSE)
// 		   	{
// 			   if(IF_ProcessNoShareableFile(fileAddr,fileName, 0x00) == FALSE)  //若当前选中的是某个非共享文件，且该文件已经被其他通道或 fileView对象所占用，则返回6a82
// 			   {
// 					 ConditionSatisfied();	 				// 69 85
// #ifndef SingleSIM
// 					if(G_CommandMode == Command_Mode_3G)
// 					{
// 						ResumeFilePath(&filePathInfo);
// 					}
// #endif									
// 					return;	
// 			   }
// 				SetChannel(fileAddr,fileName,FALSE);
// 		   	}
// 			else
// 			{
// 			 	FileNotFound();												//6A 82
// 				return;
// 			}
// 		}
		fileAddr = SelectFileByPath(fileAddr,(WORD *)&fileName);
		
		if(fileAddr == FALSE)
		{

			if(G_CommandMode == Command_Mode_3G)
			{
				ResumeFilePath(&filePathInfo);
			}
			return;
		}		
		
	}
	else
	{
		P1P2Error();
		return;
	}


	if(INS == 0x04)
	{
		fileAccStatus = A_DeactivateFile;
        
		Status = FILE_INVALIDATE;
	}
	else
	{
		fileAccStatus = A_ActivateFile;
        
		Status = FILE_REINVALIDATE;
	}

	tempTag = AccessCondition(G_CurrentEfAddr[G_ChannelNum],fileAccStatus);       //access condition

	if(tempTag == FALSE)
	{
		if(G_CommandMode == Command_Mode_3G)
		{
			IF_CleanShareableFile(G_CurrentEfAddr[G_ChannelNum],G_CurrentEfName[G_ChannelNum]);
			ResumeFilePath(&filePathInfo);
		}

		
		SecuritySatisfied();			 					//send 69 82
			
		return;
	}                                    
     
	actualFileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);	
	
	fileStatus = GetFileHeadByte(actualFileAddr, F_Status_Offset);

	if((fileStatus&0x01) == Status)  						//已失效的文件只允许执行select和activate命令
	{
		if(G_CommandMode == Command_Mode_2G)
		{
			InvalidationContradiction();					//send 98 10
			
			return;
		}
		else
		{
		#if UICC__OPERATOR__CHINATELECOM
			IF_CleanShareableFile(G_CurrentEfAddr[G_ChannelNum],G_CurrentEfName[G_ChannelNum]);
			ResumeFilePath(&filePathInfo);

			if(INS == 0x04)
			{
				SelectedFileInvalid();
			}
			else
			{
			       //已激活的文件不允许重复激活
				//电信测试脚本rfm/func/cmd/uicc/T_ActivateFile_Normal_PORSubmit				
 
					GlobalSetStatus(0x6900);     //command not allowed  No information given
			}
			
			return;			
		
		#else
		   	if(INS == 0x04)
			{
				IF_CleanShareableFile(G_CurrentEfAddr[G_ChannelNum],G_CurrentEfName[G_ChannelNum]);
				
				ResumeFilePath(&filePathInfo);

				SelectedFileInvalid();
				
				return;
			}

		#endif	
	    }
	}
     
	if(fileAccStatus == A_DeactivateFile)
	{
		tempTag = SetEepromEX(actualFileAddr+ F_Status_Offset,fileStatus&0xFE,sizeof(BYTE)); 
	}
	else
	{             
		tempTag = SetEepromEX(actualFileAddr + F_Status_Offset,(fileStatus | FILE_REINVALIDATE),sizeof(BYTE));
	}

	if(tempTag ==FALSE)
	{
		if(G_CommandMode == Command_Mode_2G)
		{
			MemoryError();													//92 40
		}
		else
		{
			IF_CleanShareableFile(G_CurrentEfAddr[G_ChannelNum],G_CurrentEfName[G_ChannelNum]);
			
			ResumeFilePath(&filePathInfo);		

			NoInforStateOfMemoryUnchanged();								//64 00
		}

		
		return;
	}
#if (!UICC__GSM_2G_NOT_SUPPORT)	
	if((G_CommandMode == Command_Mode_2G)&&(G_CurrentDfName[G_ChannelNum] == DF_TELECOM )&&(G_CurrentEfName[G_ChannelNum] == EF_ADN))	 //激活/失效 EF_ADN文件 可能改变 2G下 FDN状态 需要同步到3G模式
	{
		tempTag = IsFdnServiceActiveInSIM();				  //获取fdn服务状态
	 
		SimFdnSYNCtoUSIM(tempTag);		 					  //同步到USIM
	}
#endif	  
	CommandFinish(0x00);   
}
/**
 * @brief 		       GetResponseCommand:This function is to the return the data's of the first command
 *
 * @command format 	   A0 C0 00 00 lgth /0X C0 00 00 lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void GetResponseCommand(void)
{

	WORD idata sendNum;

	if(P3 == 0x00)
	{
		sendNum = 0x100;
	}
	else
	{
		sendNum = P3;
	}

 	if(P1P2Check() ==FALSE)
	{
		return;
	}

	//if(G_616CStatus[G_ChannelNum][0] ==  0x00)
	{
		if((SW1 !=0x61)&&(SW1 != 0x9F)&&(SW1!=0x62)&&(SW1 !=0x6C)&&(SW1 != 0x67)&&(SW1 != 0x9E))
		{
			TechnicalProblem();											//Send 6F 00

			return;
		}
	}

	if(sendNum > G_CurrentReponse[G_ChannelNum] )
	{
		if(G_CurrentReponse[G_ChannelNum] == FALSE)
		{
			TechnicalProblem();										//Send 6F 00
		}
		else
		{
			RepeatPreviousCmdHeader(G_CurrentReponse[G_ChannelNum]);    //6CXX						
		}
		return;
	}

	//if(G_616CStatus[G_ChannelNum][0] == 0x9F)
	//{
	//	ReadEEPromCache((BYTE *)&RecData[0],ResponseDataSave_Addr +G_ChannelNum*ResSave_Len+G_616CStatus[G_ChannelNum][3] , sendNum);

	//	G_616CStatus[G_ChannelNum][3] += sendNum;
	//}
	
	//if(G_BakRecDataTag == TRUE)
	//{
	//	ReadVarible(Tag_BakRecData,&RecData[0]);
	//	G_BakRecDataTag = FALSE;
	//}

	SendDataSingle();

	if(G_CommandMode == Command_Mode_2G)
	{
		CommandFinish(0x00);										  //Send 90 00/91xx
	}
	else															  //Command_Mode_3G
	{
		//if(G_616CStatus[G_ChannelNum][0] !=  0x9F)
		{
			if(G_CurrentReponse[G_ChannelNum]>sendNum)
			{
				ecpMem_Copy(&RecData[0],&RecData[P3],G_CurrentReponse[G_ChannelNum]-P3);
			}
		}

		CommandFinish(G_CurrentReponse[G_ChannelNum]-P3);	          //Send 90 00/91xx
	}
}
/**
 * @brief 		       UsimGetChallengeCommand:This function is used to create a random number
 *
 * @command format 	   0X 84 00  00 notpresent
 *
 * @Returns		       void
 *
 * @history 	       
 */
void UsimGetChallenge(void)
{
	if(P1P2P3Check(0x00,TRUE) ==FALSE)  
	{
		return;
	}
	
	Random_Number_Generator_Get((BYTE xdata *)&RecData[0],P3);
	
	SendData();
#if UICC__OPERATOR__CHINATELECOM
	GlobalSetStatus(0x9000);
#else
	CommandFinish(0x00);
#endif
}
/**
 * @brief 		       TerminalCapability:手机不支持SWP接口时执行相应操作
 *
 * @command format 	   0X AA 00  00 XX
 *
 * @Returns		       void
 *
 * @history modify by xuxiaojun@20211009 如果VM_CONFIG_START不是0的话， 读取的E_TERMINAL_CAPBILITY地址是不对的	       
 */
void TerminalCapability(void)
{
	if((P1!=0x00)||(P2!=0x00))
	{
		P1P2Error();
		return;	
	}

	ReceiveData(); 	
#if BOOT__IO__SWP
	TerminalCapabilityProcess(RecData);
#endif
#if UICC__AUTH__5G__SUSPEND
	ReadEeprom(&RecData[0x80],JcvmConfig_GetE_TERMINAL_CAPBILITYBase(),P3);
	
	if(RecData[0x80]==0xA9)//保存了CAPBILTY的内容
	{
		if(ecpMem_Compare(&RecData[0x80],&RecData[0x00],P3)!=EQUAL)
		{
			G_TerminalCap=FALSE;//两次terminalCapbity的值不同
			WriteEepromEX(JcvmConfig_GetE_TERMINAL_CAPBILITYBase(),&RecData[0],P3);
		}
	}
	else
	{	
		WriteEepromEX(JcvmConfig_GetE_TERMINAL_CAPBILITYBase(),&RecData[0],P3);
	}	
#endif
	
	CommandFinish(0x00);
}
/**
 * @brief 	: 该函数用于判断当前EF文件的类型、状态、权限是否符合特定条件
 *
 * @param 	: action:A_ReadFile,A_UpdateFile,A_IncreaseFile
 *			: nByShortFID: 是否通过短文件标示查找文件(0:否，非0 ：是)
 *		    : nfile: OperateType: 0x00:Record 0x01:Binary,0x02:BER-TLV,0x03:Increase 0x04: Seek
 *
 * @Returns : TRUE/FALSE
 *
 * @Coder:             		Xiaoyh 
 *
 * @history 	       
 */
BYTE ComputeAll(BYTE action, BYTE nByShortFID,BYTE nfile)
{
	BYTE idata tag;	
	BYTE idata fileType,fileStatus;
	UWORD xdata fileAddr;
	UWORD xdata actualFileAddr;

	if(nByShortFID != 0)//用short file ID取文件
	{
		fileAddr = SelectFileByShortId(G_CurrentDfAddr[G_ChannelNum],nByShortFID);

		if(fileAddr == FALSE)
			{
		#if UICC__OPERATOR__CHINATELECOM
				FileNotFound();
				//电信测试-东信5G 新增功能测试log(WD 工具)\WorkDirecory\rfm/func/cmd/uicc/T_Increase_Error_PORSubmit 期望值：026a82
				//电信测试-东信5G 新增功能测试log(WD 工具)\WorkDirecory\rfm/func/cmd/uicc/T_ReadBinary_Error_PORSubmit 期望值：026a82
				//电信测试-东信5G 新增功能测试log(WD 工具)\WorkDirecory\rfm/func/cmd/uicc/T_ReadRecord_Error_PORSubmit 期望值：026a82
		#else
				NoEfSelected();											//69 86
		#endif

				return FALSE;
			}
	}
	else
	{
		fileAddr = G_CurrentEfAddr[G_ChannelNum];

		if(fileAddr == FALSE)
		{
			NoEfSelected();											//69 86

			return FALSE;
		}

	}

	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
	
	actualFileAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);

	fileStatus	= GetFileHeadByte(actualFileAddr,F_Status_Offset);
	
	tag= TRUE;
	
	switch(nfile)		//判断文件类型
	{
		case 0:
			if((fileType != LINTYPE)&&(fileType != CYCTYPE )) //线形记录文件或者循环记录文件
			{
				tag =FALSE;
			}		
			break;
		case 1:
			if(fileType != TRANTYPE)		//透明文件
			{
				tag =FALSE;
			}		
			break;
#if UICC__FUNCTION__BERTLVCmd
		case 2:
			if(fileType != BERTLVTYPE)                 //BER TLV 文件
			{
				tag =FALSE;
			}		
			break;
#endif
		case 3:
			if(fileType != CYCTYPE )	//循环记录文件
			{
				tag =FALSE;
			}	
			break;	
		case 4:
			if(fileType != LINTYPE)        // 2G 为线形文件，3G为线形文件或循环文件              
			{
				if((G_CommandMode == Command_Mode_2G) ||((G_CommandMode == Command_Mode_3G)&&(fileType != CYCTYPE)) )
				{
					tag =FALSE;
				}
			}  	
			break;
		default:		
			break;	
	}

	if(tag == FALSE)
	{
		FileStructureError();           //69 81
		
		return FALSE;
	}

	if((fileStatus&F_READ_UPDATA_ENBALE) == 0x00)  //失效文件时，不可读写
	{
		if((fileStatus&0x01) == FILE_INVALIDATE)		//判断文件是否有效
		{
			if(G_CommandMode == Command_Mode_2G)
			{
				InvalidationContradiction();                                    //98 10
			}
			else
			{
				ReferencedDataInvalid();					//69 84
			}
			return FALSE;
		}
    }
	
   	tag = AccessCondition(fileAddr,action);		//判断访问条件是否满足

   	if( tag == FALSE ) 
	{
		SecuritySatisfied();
		
		return FALSE;
	}    	

	return TRUE;
}
