/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1 增加接口IF_EnvelopeCommandSTK()和IF_EnvelopeCommand()
			V1.2 修改函数EnvelopeCommandSTK
			V1.3 为满足联通脚本测试需要,添加sleep命令,该命令在11.17有提及 -20100323
			V1.4 修改函数FetchCommand  @20110806 by xuyz
			V1.5 修改函数FetchCommand  @20130922 by wlj,当P3==0x00，FetchCommand函数处理有错
			v1.6 修改函数EnvelopeCommandSTK，当IF_EnvelopeCommandSTK返回为TRUE的时候，直接退出该函数@20131127 by xuyz
			V1.7 修改函数FetchCommand，当主动式命令获取结束后，将STKPerform的值设置为0x02，表示当前主动式命令fetch结束
			     修改函数TerminalResponse。当收到14响应时，将STKPerform的值设置成FALSE,并处理IF_TerminalResponse,并且在STKPerform =FALSE时，对标记进行处理@20140623 by xuyz

				**************for uicc api处理 20151117**************************************
			v1.8  修改函数EnvelopeCommand，修改接口IF_EnvelopeCommand()的调用，同时增加接口函数IF_EnvelopeCommandFinish
			      修改函数TerminalProfile，增加接口StoreProfileData，修改接口函数IF_TerminalProfile的调用
				  修改函数STKTerminalProfile，增加接口StoreProfileData	、IF_STKTerminalProfile	     @20151117 by xuyz for uiccapi

			v1.9 修改函数EnvelopeCommand，屏蔽掉STKPerform = TRUE  的判断，为兼容UICC API测试 @20151202 by xuyz

 */ 
#include <string.h>
#include <uicc_error.h>
#include "typedef.h"
#include "uicc_main.h"
//#include "main.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
//#include "resume.h"
#include "stkhead.h"
//#include "hard_api.h"
#include "stkalert.h"
#include "uicc_interface.h"
#include "STKCATRuntimeEnviorment.h"
#include "ToolkitAPIPublic.h"
#include "Mbms.h"
#include "T0ApduImpl.h"
#include "Cdma.h"
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
#include "SEEUICCInterface.h"
#endif
#if UICC__APP__WHITECARD
#include "WhiteCardBeWrited.h"
#include "LiveWirteCardXL.h"
#endif
#if UICC__AUTH__CDMA__CAVE
#include "cdma.h"
#endif
#if UICC__AUTH__EAP
#include "EapAuthentication.h"
#include "Authentication.h"
#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunAuth
  
  Command format:    00    89    00   00   XX
  
  Data    content:   proative commnad data's                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:      鉴权指令（GBA）
  
  status:			 Code OK!     Test OK!    

  history          V1.0 create funcyion
                   V1.1 modify by xwh 20211126,避免出现false==false的情况
 -------------------------------------------------------------------------------------------------------------------*/    
#if (UICC__AUTH__MBMS || UICC__AUTH__LOCALKEY || UICC__AUTH__EAP)
void RunODDAuth(void)
{
	BYTE idata result;
	UWORD xdata fileAddr;
	
	
	if (G_CurrentAdfName[G_ChannelNum] == FALSE)
	{
		ConditionSatisfied();												//Send 69 85
		return;
	}
	
	if(ValueBit(G_ChvStatus,B_Chv_1_Verified) == FALSE)
	{
		SecuritySatisfied();												//Send 69 82
		return;
	}

#if UICC__ISIM	
	if((G_DF_ID_ISIM != FALSE)&&(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)) // 增加G_DF_ID_ISIM != FALSE判断，否则这里会出现false== false 情况
	{
		fileAddr = SelectTFFile( G_DF_ID_ISIM, EF_IST);
	}
	else
#endif		
	{
		fileAddr = SelectTFFile( G_DF_ID_USIM, EF_UST);
	}
	
	result = 0x00;

	if (fileAddr == FALSE)
	{
		ConditionSatisfied();												//Send 69 85
		return;
	}
	
	switch(P2)
	{
#if UICC__AUTH__EAP
		case 0x00: //add by chentao20220418 supprot EAP
		{
			result = EAPAuthentication();

			if((result == 4)||(result == 3))
			{
				return;
			}

			if(result != FALSE)
			{
				result = G_ResponseLen;
			}
			break;
		}
#endif
	#if UICC__AUTH__MBMS
		case 0x85:  //MBMS
		{
			if(fileAddr != FALSE)
			{
				ReadTFBuf(fileAddr, 0x08, &result, sizeof(BYTE));
			}
			
			if ((result&0x10) != 0x10)
			{
				ConditionSatisfied();											//Send 6985

				return;
			}
			
			result = MBMSAuth();
			
			if((result == 4)||(result == 3))
			{
				return;
			}	
			break;
		}
	#endif
		
	#if  UICC__AUTH__LOCALKEY
		case 0x86:
		{
			if(fileAddr != FALSE)
			{
			#if UICC__ISIM
				if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
				{
					ReadTFBuf(fileAddr, 0x00, &result, sizeof(BYTE));												
					
					if ((result&0x0A) != 0x0A)
					{
						ConditionSatisfied();											//Send 6985

						return;
					}	
				}				
				else
			#endif	
				{
					ReadTFBuf(fileAddr, 0x08, &tempBuf[0], 0x02);
																	
					if (((tempBuf[0]&0x08) != 0x08) ||((tempBuf[1]&0x08) != 0x08))
					{
						ConditionSatisfied();											//Send 6985

						return;
					}	
				}								
			}
		
			result = LocalKeyEstablish();
			
			if((result == 4)||(result == 3))
			{
				return;
			}	
			break;
		}
	#endif
	
		default:
		{
			InsError();
			return;
		}	
	}
	
	if(result == FALSE)
	{
		AuthenticateError();
	}
	else
	{
		CommandFinish(result);
	}
	
}
#endif

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 RunAuth
  
  Command format:    00    88    00   00   XX
  
  Data    content:   proative commnad data's                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:      鉴权指令（GBA）
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    

void RunAuth(void)
{
	if (G_CommandMode == Command_Mode_2G)		
	{
#if(!UICC__CDMA_2G_NOT_SUPPORT)
		if(G_CurrentDfName[G_ChannelNum] == DF_CDMA)
		{
#if UICC__CDMA__SMS__DOWNLOAD
			G_IsCdmaSms = TRUE;
#endif			
			RunCDMAAlgorithm();
		}
		else
#endif
		{
#if UICC__CDMA__SMS__DOWNLOAD
			G_IsCdmaSms = FALSE;
#endif
#if(!UICC__GSM_2G_NOT_SUPPORT)
#if (UICC__AUTH__V1||UICC__AUTH__V2V3)
			if(SUPPORT_V1V2V3)  // add by chentao 20221207 俄罗斯MTS招标需要测试 V1V2V3
			{
				RunGsmAlgorithmA3A8();
			}
			else
			{
				RunGsmAlgorithm2G();
			}
#else
			RunGsmAlgorithm2G();
#endif
#else
			ConditionSatisfied();
#endif
		}
	}
	else	                                     //4G?￡ê?
	{				
		UsimAuthentication(); //Command_Mode_3G	
	}
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 FetchCommand
  
  Command format:    A0    12    00   00   XX
  
  Data    content:   proative commnad data's                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used to transfer an application toolkit command from the sim to the ME.
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    
void FetchCommand(void)
{
 	BYTE idata offset;
	WORD idata sendNum;
	//WORD idata num;
	
	if(P1P2Check() ==FALSE)
	{
		return ;
	}
	
	if(P3 == 0x00)
	{
		sendNum = 0x0100;
	}
	else
	{
		sendNum = (0x00FF&P3);
	}
	                                      									
	if((ResData[2]!= ((ResData[2]>0x7F)? (sendNum  -3):(sendNum - 2)))||(STKPerform[0] != TRUE))
	{
		P3Error();														//send 67 00
		return;
	}
    
	//Delay4etu();              //收转发延迟4 个etu

	//timer0(T0_IS_OFF);

 	//SendByte(INS);
	
	if(ResData[2] >= 0x80)
	{
		ResData[1]=0x81;

		offset = 0x00;
	}
	else
	{
		ResData[1]=0xD0;

		offset = 0x01;
	}

	//for( num=0; num<sendNum; num++)
	//{
	//	SendByte(ResData[num+offset]);										//把主动式命令的数据放到缓冲区发送
	//}

	//timer0(T0_IS_ON);

	STKPerform[0] = 0x02;  ////设置STKPerform为3个值:0x00,0x01,0x02,防止开机 过程被RFM/RAM打断 	

	//CommandFinish(0x00);													//90 00
	ecpMem_Copy(RecData, ResData+offset, sendNum);
	apdu_ApduSetOutgoingAndSend(5,sendNum);
	
	GlobalSetStatus(0x9000);
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 EnvelopeCommand
  
  Command format:    A0    C2    00   00   XX
  
  Data    content:   proative commnad data's                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used to transfer data to the SIM Appliction Toolkit applications int the SIM
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/ 
void EnvelopeCommand(void)
{
		
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return ;
	}


	ReceiveData();
    
    ProcessEnvelopData(); 

#if UICC__FUNCTION__STKAPI
	
	if(SetSTKBusy() == TRUE)
	{
		return;
	}
	else
 #endif
	{
		ProcessPendingMessageAlert();

		CommandFinish(0x00);	
	}
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 STKEnvelopeCommand
  
  Command format:    A0    C2    00   00   XX
  
  Data    content:   proative commnad data's                                                      
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used to transfer data to the SIM Appliction Toolkit applications int the SIM
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    
void EnvelopeCommandSTK(void)
{	
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return;
	} 

	ReceiveData();

#if UICC__FUNCTION__STKAPI
	ToolkitSaveAPPContext();
#endif

	ProcessEnvelopData();

#if UICC__FUNCTION__STKAPI
	ToolkitRestoreAPPContext();
	
	if(G_ISOBusyExceptionFlag == TRUE)
	{
		G_ISOBusyExceptionFlag = FALSE;

		SimAppTkitBusy();
	}
	else	
#endif
	{
		CommandFinish(0x00);
	}
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 TerminalResponse
  
  Command format:    A0    14    00   00   XX
  
  Data    content:   the result of proative commnad                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used to transfer from the ME to the SIM the response to a previously fetched SIM Appliction Toolkit
  
  status:			 Code OK!     Test OK!    
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 -------------------------------------------------------------------------------------------------------------------*/    
void TerminalResponse(void)
{
	UWORD xdata toolkitAppAddr;
	BYTE idata i,recNum,sendPollInterval,tag,pos;
	BYTE xdata tmp[EventList_Length];
#if UICC__APP__WHITECARD
BYTE idata result;
	UWORD idata fileAddr;
#endif
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	u32    X_A   toolkitInfoObj;
#endif

	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return ;
	}
	
	ReceiveData();

	if(STKPerform[0] == 0x02)
	{
		STKPerform[0] = FALSE;
        
#if UICC__FUNCTION__STKAPI
		ClearBit(G_ToolkitSystemProactiveFlag,Bit_Pending);					//收到14命令则中断当前挂起的系统主动式命令流程
 #endif
	}
		
//	IF_TerminalResponse();
#if UICC__APP__WHITECARD
	if(RecData[3] == 0x13)
	{
		result = Get_ResponseResultSMS_TR();
		if(result == SendSMS_SUCCESS)
		{
			if (G_applySMS == 0x01)
			{
				G_applySMS = 2;
			}
			else if (G_applyActivate == 0x01)
			{
				G_applyActivate = 2;
				fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConstructor);
				if(fileAddr == FALSE)
				{
					return ;
				}
				SetTFConst(fileAddr, 0, 0x02, 0x01);//02 重启后发送新号激活短信成功，写卡流程结束
			}
		}
		else if(result == SendSMS_NeedResent)
		{
			if (G_applySMS == 0x01)
			{
				if (G_SMSCounter <= 2)
				{
					G_SMSCounter++;
					SendSMS_WriteCardApply(1);
				}
				else
				{
					G_applySMS = 0;
					cephDisplayTextSingle(0x81, UCS2, (BYTE CONST*)WhiteCardWriteCardErr);
					return ;
				}
			}
			else if (G_applyActivate == 0x01)
			{
				if (G_SMSCounter <= 2)
				{
					G_SMSCounter++;
					SendSMS_NewNumberActivate();
				}
				else
				{
					G_applyActivate = 0;
					return ;
				}
			}
		}
		else
		{
			if (G_applySMS == 0x01)
			{
				G_applySMS = 0;
				cephDisplayTextSingle(0x81, UCS2, (BYTE CONST*)WhiteCardWriteCardErr);
			}
			else if (G_applyActivate == 0x01)
			{
				G_applyActivate = 0;
			}
			return ;
		}
	}
#endif	
#if UICC__FUNCTION_CMCCAPI
	if(RAM_Location_Status_Flag == 0x01)
	{
		pos = FindTag(&RecData[0],TLV_RESULT,P3);
	
		if(RecData[pos+1] < 0x0F)
		{
			RAM_Function_Supported = 0x01;
		}
		else
		{
			RAM_Function_Supported = 0x00;
		}
		RAM_Location_Status_Flag = 0x00;
	}	
#endif	
	
	
	if(STKPerform[0] == FALSE)                                              //开机过程中收到的RFM，先打标记，开机过程结束后在此处触发
	{

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		sendPollInterval = FALSE;

		recNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

		for (i = APP_FIRST; i < recNum; U16_INC(i))
		{
			toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

			if(toolkitAppAddr == ADDR_IS_NULL)
			{
				continue;
			}

			ecpMem_Copy(&tmp[0],MEMtPN8(toolkitAppAddr + AppletStatus_OFFSET),1);

			if(tmp[0] == APPLET_UNLOCKED_SELECTABLE)
			{
				ecpMem_Copy(&tmp[0],MEMtPN8(toolkitAppAddr + EventList_OFFSET),EventList_Length);

				if(ToolkitGetEventBit(tmp,EVENT_STATUS_COMMAND_bit))
				{
					sendPollInterval = TRUE;

					break;
				}
			}
		}

#else
		sendPollInterval = FALSE;

		toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
		
		if(toolkitAppAddr != FALSE)
		{
			recNum =  GetLFRecNumLgth(toolkitAppAddr,FileNum);
			
			for(i=0; i<recNum; i++)
			{			
				tag = ReadLFBufForJavaAccess(toolkitAppAddr,i+1,AppletStatus_OFFSET,&tmp[0],0x01);

				if((tag != FALSE)&&(tmp[0] == APPLET_UNLOCKED_SELECTABLE))
				{
					ReadLFBufForJavaAccess(toolkitAppAddr,i+1,EventList_OFFSET,tmp,EventList_Length);

					if(ToolkitGetEventBit(tmp,EVENT_STATUS_COMMAND_bit))
					{
						sendPollInterval = TRUE;

						break;
					}
				}
				
			}
		}
#endif
		//用例名称:  UICCReEntry_07_05_02 -- TestPurpose: 开机中setupMenu过程中重入，应当先处理开机流程，再处理重入数据的测试
		if(((G_ToolkitSystemProactiveFlag & 0x30) == 0x00)&&(((G_ToolkitSystemProactiveFlag & 0x40) == 0x00)||(sendPollInterval == FALSE)))
		{
			ProcessPendingMessageAlert();
		}			
	}					

	CommandFinish(0x00);
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 TerminalProfile
  
  Command format:    A0    10    00   00   XX
  
  Data    content:   the result of proative commnad                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used by ME to transmit to the SIM its capabilities concerning the SIM Application ToolKit

  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    
void TerminalProfile(void)
{
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return;
	}

	ReceiveData();

	if(P3 >= 0x02)
	{
		if((RecData[1]&0x20)== 0x20)
		{
			SetBit(G_SystemStatus ,B_meSurportUcs2);
		}
		else
		{
			ClearBit(G_SystemStatus ,B_meSurportUcs2);
		}
	}

	if(P3 >= 0x04 )
	{
		if((RecData[3]&0x20) == 0x20)
		{
			SetBit(G_SystemStatus, B_meSurportStk);
		}
	}

	ecpMem_Set(&G_TerminalProfile[0],0x00, sizeof(G_TerminalProfile));

	if((P3 + 1) > sizeof(G_TerminalProfile))
	{
		G_TerminalProfile[0] = sizeof(G_TerminalProfile) - 1;
	}
	else
	{
		G_TerminalProfile[0] = P3;	//first byte is length of  terminalprofile
	}

	ecpMem_Copy(&G_TerminalProfile[1],&RecData[0],G_TerminalProfile[0]);
	
	//5G国密卡-通信脚本log 2020-10-20\通信功能测试脚本_GMNFC_20200910\测试脚本\6.Java功能测试脚本\UICC\2.framework\Applet Triggering\catre.cre_apt_edat.Test_Cre_Apt_Edbt.log
	//要求在TerminalProfile 时重发SetUpEventList
	ecpMem_Set(&G_EventBufferOld[0],0x00, sizeof(G_EventBufferOld));
	
#if UICC__FUNCTION__STKAPI
	JavaToolkitProcess(EVENT_PROFILE_DOWNLOAD_bit,0);

	if(SetSTKBusy() == TRUE)
	{
		return;
	}

	if((STKPerform[0] == TRUE)&&(ValueBit(G_ToolkitSystemProactiveFlag,Bit_Pending) == TRUE))
	{
		STKPerform[0] = FALSE;//在收到terminalprofle时，如果之前有91xx未取的setupmenu等系统主动式命令。将其丢弃
	}

    G_ToolkitSystemProactiveFlag = 0x70;  //打上标记Bit_PowerOnSetupEventList  Bit_PowerOnSetupMenu	  Bit_PowerOnPollInterval
	
	if((G_STKBusy == FALSE)&&(STKPerform[0] == FALSE))   //如果当前在主动式命令中，则 Index_ProactiveHandler依旧无效
	{
		if(G_TerminalProfile[0] != 0x00)   //102241  The ProactiveHandler shall not be available if the Terminal Profile command has not yet been processed by the CAT Runtime Environment.
		{                                  //If the CAT session ends prior to an applet triggering, the applet will be triggered at the next CAT session.										
			JavaToolkitProcessProactiveHandlerEvent();	 //测试用例cre_apt_epha ,在应用1的menuSelection中触发后，12数据还未退出，发送了应用2的menuselection要求触发应用，从应用2的menuselction退出后 If the CAT session ends prior to an applet triggering, the applet will be triggered at the next CAT session	
		}
	}
#endif
	
	CommandFinish(0x00);													//91 xx /90 00
	
	G_OpenControl = TRUE;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 STKTerminalProfile
  
  Command format:    A0    10    00   00   XX
  
  Data    content:   the result of proative commnad                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used by ME to transmit to the SIM its capabilities concerning the SIM Application ToolKit

  status:			 2008-3-7

 -------------------------------------------------------------------------------------------------------------------*/    
void STKTerminalProfile(void)
{
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return;
	}

	ReceiveData();

	if(P3 >= 0x02)
	{
		if((RecData[1]&0x20)== 0x20)
		{
			SetBit(G_SystemStatus ,B_meSurportUcs2);
		}
		else
		{
			ClearBit(G_SystemStatus ,B_meSurportUcs2);
		}
	}

	if(P3 >= 0x04 )
	{
		if((RecData[3]&0x20) == 0x20)
		{
			SetBit(G_SystemStatus, B_meSurportStk);
		}
	}

	ecpMem_Set(&G_TerminalProfile[0],0x00, sizeof(G_TerminalProfile));
	//5G国密卡-通信脚本log 2020-10-20\通信功能测试脚本_GMNFC_20200910\测试脚本\6.Java功能测试脚本\UICC\2.framework\Applet Triggering\catre.cre_apt_edat.Test_Cre_Apt_Edbt.log
	//要求在TerminalProfile 时重发SetUpEventList
	ecpMem_Set(&G_EventBufferOld[0],0x00, sizeof(G_EventBufferOld));

	if((P3 + 1) > sizeof(G_TerminalProfile))
	{
		G_TerminalProfile[0] = sizeof(G_TerminalProfile) - 1;
	}
	else
	{
		G_TerminalProfile[0] = P3;	//first byte is length of  terminalprofile
	}

	ecpMem_Copy(&G_TerminalProfile[1],&RecData[0],G_TerminalProfile[0]);
    
	GlobalSetStatus(0x9000);

#if UICC__FUNCTION__STKAPI
	ToolkitSaveAPPContext();
    
	JavaToolkitProcess(EVENT_PROFILE_DOWNLOAD_bit,0);

	SetSTKBusy();

	ToolkitRestoreAPPContext();
#endif

}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 Sleep Command
  
  Command format:    A0    FA    00   00   00
  
  Data    content:   the result of proative commnad                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       根据11.17 ,添加sleep命令-add by xyz 20100323 14:17 

  status:			 2008-3-7

 -------------------------------------------------------------------------------------------------------------------*/    
void Sleep(void)
{
	if(P1P2P3Check(0x00,FALSE) ==FALSE)
	{
		return ;
	}
	
	CommandFinish(0x00);
}
