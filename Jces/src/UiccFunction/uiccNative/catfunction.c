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
			V1.4 修改函数EnvelopeCommandSTK，修改当前函数无法发送9300的问题20131128 by xuyz
			V1.5 修改函数EnvelopeCommand，如果函数IF_EnvelopeCommand返回TRUE，则不改变原流程，继续执行；若返回FALSE，则退出  20141009 by zx
			V1.6 修改函数EnvelopeCommand,增加调用接口函数IF_LDFileDeliver(),兼容中国电信RFM功能			    --20190115  by yangjian
			V1.7 函数EnvelopeCommand移动到AuthConfig.c,开放代码给应用开发

 */ 
#include <string.h>
#include "typedef.h"
#include "uicc_main.h"		   
#include "uicc_error.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
//#include "resume.h"
#include "stkhead.h"
//#include "hard_api.h"
#include "interface.h"



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
	WORD idata sendNum,num;
	
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
	                                      									
	if(ResData[2]!= (ResData[2]>0x7F?(sendNum-3):(sendNum-2)))
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

	STKPerform = 0x02;  ////设置STKPerform为3个值:0x00,0x01,0x02,防止开机 过程被RFM/RAM打断 	

	G_STKBusy = 0x02;  			//已发出主动式命令，但没有"14"响应
	
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
  
  					若IF_EnvelopeCommand返回0xFF01，则退出/若返回其他值，则继续执行

 -------------------------------------------------------------------------------------------------------------------*/ 

void EnvelopeCommand(void)
{
	WORD idata result;
	BYTE idata status;
	
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return ;
	}
	
	if(STKPerform == TRUE)
	{
		SimAppTkitBusy();											//Send 93 00
		return;
	}		
	

	ReceiveData();

	result = IF_EnvelopeCommand();

	if(result == 0xFF01)
	{
		return;
	}

	if(G_STKBusy == 0x00)			  						//只有不在STK流程中,才去执行标记
	{
		ProcessPendingMessageAlert();
	}

	status = IF_LDFileDeliver();

	if(status == TRUE)
	{
		return; 
	}

	CommandFinish(result);
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
	BYTE idata result,result2;
	
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return;
	} 

	ReceiveData();
	
	result = IF_EnvelopeCommandSTK();
	
	result2 = IF_LiveCardBusy();

	if((result2 == TRUE)||(result == TRUE))
	{
		return;
	}


	CommandFinish(0x00);
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 TerminalResponse
  
  Command format:    A0    14    00   00   XX
  
  Data    content:   the result of proative commnad                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function is to Used to transfer from the ME to the SIM the response to a previously fetched SIM Appliction Toolkit
  
  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/    
void TerminalResponse(void)
{
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return ;
	}
	
	ReceiveData();

	if(STKPerform == 0x02)
	{
		STKPerform = FALSE;

		IF_TerminalResponse();
	}
		
	if(STKPerform == FALSE)
	{
		ProcessPendingMessageAlert();
	}	

	G_STKBusy = 0x00;			//已收到“14”响应，G_STKBusy返回0x00

	CommandFinish(0x00);
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 TerminalProfileCommon                                                                                                                   
                                                                                                                                                
  Input parameter:   void
                              					                                                                                                         
  Output parameter:  TRUE/FALSE                                                                                                               
                                                                                                                                                
  description:       TerminalProfile函数的共用部分代码
  
  status:		        

 -------------------------------------------------------------------------------------------------------------------*/
BYTE TerminalProfileCommon(void)
{
	
	if(P1P2P3Check(0x00,TRUE) ==FALSE)
	{
		return  FALSE;
	}

	ReceiveData();

	if((RecData[1]&0x20)== 0x20)
	{
		SetBit(G_SystemStatus ,B_meSurportUcs2);
	}
	else
	{
		ClearBit(G_SystemStatus ,B_meSurportUcs2);
	}

	if((RecData[3]&0x20) == 0x20)
	{
		SetBit(G_SystemStatus, B_meSurportStk);
	}

	return TRUE;
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

	if(TerminalProfileCommon() == FALSE)
	{
		return ;
	}

	if (G_CurrentDfName[G_ChannelNum] == DF_CDMA)  
 	{
 		G_NetWorkMode = CDMA_NETWORK;
 	}

	IF_TerminalProfile();

	CommandFinish(0x00);													//91 xx /90 00
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
	if(TerminalProfileCommon() == FALSE)
	{
		return;
	}

	SW1 = 0x90;

	SW2 = 0x00;	
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
		return;
	}
	
	CommandFinish(0x00);
}


