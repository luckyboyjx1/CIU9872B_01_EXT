/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCEnvelopHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-28

*Description :该文件主要用于实现基于UICC JAVA API的一些接口函数
*
*History     :
	
author    time      version    description
xuyz   2013/11/21   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "stkhead.h"
#include    "hard_api.h"
#include    "ToolkitRFM.h"
//#include    "JCREMain.h"
//#include    "GPCommand.h"
//#include    "Interpreter.h"
#include    "uicc_interface.h"
#include    "ToolkitAPIPublic.h"
//#include    "main.h"
#include    "STKToolkitInterface.h"

 /*
 *@function     :UICCToolkitPollInterval
 *
 *@description  :拼装主动式命令poll interval
 *
 *@param        :timeUint :间隔类型
 *@param        :timeInterval :间隔持续时间
 *
 *@return       :TRUE
 *
 *@author       :xuyz
 *
 *@history      :V1.0: create by xuyz@20131125
 */
BYTE ToolkitPollInterval(WORD duration)
{
	if(ValueBit(G_TerminalProfile[3],5) == FALSE)
	{
	 	return FALSE;
	}

	InitProactiveCommand(CMD_POLLINTERVAL, 0x00, DEV_ME);

	//记录当前的duration，若新的duration==当前duration，不再发送Poll interval主动式命令
	//G_CUR_DURATION = duration;

	if(duration != 0x00)
	{		
		tempBuf[0] = TLV_DURATION&TLV_INVERT_CR;//duration
		
		tempBuf[1] = 0x02;

		if(duration <= 0xFF)
		{	
			tempBuf[2] = 0x01;	    //'01' seconds;
		
			tempBuf[3] = duration;
		}
		else if(duration <= 0x9F6)
		{
			tempBuf[2] = 0x02;	  // '02' tenths of seconds;
		
			tempBuf[3] = duration/10;
		}
		else 
		{
			tempBuf[2] = 0x00;	  // '00' minutes;
		
			tempBuf[3] = duration/60;
		}

		AddTLV(&ResData[1], &tempBuf[0]);
	}

    STKPerform[0] = TRUE;

	STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 

	return TRUE;
}
/*@Function:    SendPollOffCmd                                            
 * 
 *@Descrip:     函数拼装PollOff命令
 *
 *@See:	        
 *                                                                                                                                     
 *@Param:       NULL 
 *				                                                                                                         
 *@Return:      TRUE: 表示拼装命令成功                                                                                                                                                                                                                                                                 
 * 
 *@Author:      YangJian
 *  
 *@History:     2013/9/26
 *                        
 */

BYTE ToolkitPollOff(void)
{
	if(ValueBit(G_TerminalProfile[3],6) == FALSE)
	{
		return FALSE;
	}

//	G_CUR_DURATION = POLL_NO_DURATION;

	InitProactiveCommand(CMD_POLLINGOFF,0x00,DEV_ME);

	STKPerform[0] = TRUE;

	STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 

	return TRUE;		
}


