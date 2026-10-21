/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件

			V1.1 修改ProcessPendingMessageAlert函数	   @20130922 wlj 针对RFM的标记处理

			V1.2  修改ProcessPendingMessageAlert函数   @20130925 wlj 针对RFM的标记处理

			V1.3 修改ProcessPendingMessageAlert函数，当前的主动式命令如果没有获取或其14响应没有收到的话，则不对标记进行处理@20140623 by xuyz
 */
#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "ceph.h"
#include  "uicc_interface.h"
#include "ToolkitAPIPublic.h"
#include "ToolkitRFM.h"
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 PendingMessageAlert
  
  Input parameter:   void					                     

  Output parameter:	 void
                                                                                                                                                
  Description:       对OTA进行标记处理
  
  Status:			 OK
  
  Author:			 zhuli
  
  Testor:			 Caibo
  
  Status :			 OK    

 -------------------------------------------------------------------------------------------------------------------*/    
void PendingMessageAlert(BYTE ch,BYTE refNumber)
{
	UWORD idata fileAddr;
	WORD  idata offset;
	WORD  idata fileSize;
	BYTE  idata buf[2];

	fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_STKAlert);	
	
	if(fileAddr == FALSE)
	{
		return ;
	}

	fileSize = GetTFSize(fileAddr);

	if(ch == D_SetUpMenuTag)                                        //若为SetUpMenu标志，只设置一个，防止多次SetUpMenu
	{
		for(offset=0; offset<fileSize; offset = offset + 2)
		{
			ReadTFBuf(fileAddr, offset, &buf[0], sizeof(buf));
	
			if(buf[0] == D_SetUpMenuTag)
			{
				return;
			}
		}
	}

	for(offset=0; offset<fileSize; offset=offset +2)
	{
		ReadTFBuf(fileAddr, offset, &buf[0], sizeof(buf));
	        
        if((buf[0] == 0xFF)&&(buf[1] == 0xFF))
        {     
            buf[0] = ch;

            buf[1] = refNumber;
            
            WriteTFBuf(fileAddr, offset, &buf[0], sizeof(buf));

            return;

         }
 	}

	return;
}
/*
*@description :清除卡片上的所有标记，一般用于开机时
*
*@function    :ClearingAllMessageAlert
*
*@param       :void
*
*@return      :void
*
*@history
*/
/*void ClearingAllMessageAlert(void)
{
	UWORD xdata fileAddr;
	WORD  idata fileSize;

	fileAddr = SelectTFFile(DF_OTA, EF_STKAlert);

	if(fileAddr == FALSE)
	{
		return ;
	}	
	
	fileSize = 	GetTFSize(fileAddr);

	SetTFConst(fileAddr, 0, 0xFF, fileSize);  //6ea7文件一般不会超过0x100，所以此处直接用SetTFConst进行文件的清除操作
}没有被调用*/
/*
*@function    :ClearingMessageAlert
*
*@description :清除标记文件6EA7中的指定标记
*
*@param       :alert
*
*@return      :void
*
*@history     :v1.0:create by xuyz
*/
/*
void ClearingMessageAlert(BYTE ch)
{
	UWORD xdata fileAddr;
	WORD  idata offset;
	WORD  idata fileSize;
	BYTE  idata buf[2];

	fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_STKAlert);

	if(fileAddr == FALSE)
	{
		return ;
	}

	fileSize = 	GetTFSize(fileAddr);

	for(offset=0; offset<fileSize; offset += 2)
	{
		ReadTFBuf(fileAddr, offset, &buf[0], sizeof(buf));
		
		if(buf[0] == ch)
		{
			SetTFConst(fileAddr, offset, 0xFF, sizeof(buf));

			return;
		}	
		
	}
}
*/
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ProcessPendingMessageAlert
  
  input parameter:   void
					                     

  output parameter:	 void
                                                                                                                                                
  description:       Process OTA2 pending message
  
  status:		     modify by xuxiaojun@20170725: 该函数用于EnvelopeCommand和TerminalResponse函数中，所以不用带CACHE的读写
  
 -------------------------------------------------------------------------------------------------------------------*/    
void ProcessPendingMessageAlert(void)
{
	BYTE  xdata ch[2];
	UWORD xdata fileAddr;
	WORD  xdata alertSize;
	WORD  xdata alertPoint;
	
	if(STKPerform[0] != FALSE)	   //若当前有主动式命令的没有被获取或其14响应还没有收到，则不执行标记
	{
		return;
	}

 	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_STKAlert);  	//注意,STK 等私有文件一定要放在文件系统的第一个目录，因为java卡创建过程中，可能涉及到EEPROM的移动，而导致执行标记的过程中，标记被移动.

	if(fileAddr == FALSE)
	{
		return;
	}

	alertSize = GetTFSize(fileAddr);
	
	while(1)
	{
		for(alertPoint=0x00; alertPoint<alertSize; alertPoint=alertPoint+2)
		{			
			if(ReadTFBuf(fileAddr, alertPoint, (BYTE *)&ch, sizeof(ch))==FALSE)//远程删除DF_2700，fetch之后，执行本地指令选择2700，选不到文件，删除成功，此时执行TerminalResponse时卡片无响应,增加返回值的判断，如果地址为空，终止执行by yjr20220913
			{
				return;
			}

			if(ch[0] != 0xFF)
			{
				break;
			}
		}

		if(alertPoint >= alertSize)											//没有找到标记
		{     
			return;
		}

		SetTFConst(fileAddr, alertPoint, 0xFF, sizeof(ch));

		switch(ch[0])
		{	
			
			case D_SetUpMenuTag:
				IF_SetUpMenu();	 //在进入该步时，CACHE已经提交
				break;

			default:
				IF_ProcessPendingMessageAlert(ch[0],ch[1]);	
				break;
		}
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 InitMessageAlert
  
  Input parameter:   void					                     

  Output parameter:	 void
                                                                                                                                                
  Description:       对标志进行初始化(防止打上标志后断电,复位后等系统主动性命令处理完后回去处理断电前打上的标志.目前讨论的结果是只有收满短信的标志不清空,其余的标志清空)
                     目前使用标志是在处理标志函数都是在UICC层,初始化函数放在IF_InitSystem/UICCInition 中初始化即可,后面修改 可能需要移动到第一条指令里面初始化HandleFirstCommand
  
  Author:			xwh 20200720

 -------------------------------------------------------------------------------------------------------------------*/
void InitMessageAlert(void)
 {
	 UWORD idata fileAddr;
	 WORD  idata offset,fileSize;
	 BYTE   idata updatetag;  //有需要清空的tag
	 
	fileAddr = SelectPrivateTFFile(DF_Toolkit, EF_STKAlert);
	
	if(fileAddr == FALSE)
	{
		return ;
	}

	fileSize = GetTFSize(fileAddr);
	
	if(fileSize > sizeof(tempBuf))                                        //正常情况下 文件大小不会超过 tempBuf 大小
	{
		return ;
	}
	
	updatetag = FALSE; //不需要更新alert 文件
	
	ReadTFBuf(fileAddr, 0x00,tempBuf,fileSize);
	
	for(offset = 0x00;offset <fileSize;offset += 0x02)
	{		
		if(tempBuf[offset] == 0xFF)
		{
			continue;
		}	
		else if((tempBuf[offset] != TYPE_SMS_ENV_MULTI)&&(tempBuf[offset] != TYPE_SMS_ENV_SINGLE)&&(tempBuf[offset] != TYPE_SMS_UPD_MULTI)&&(tempBuf[offset] != TYPE_SMS_UPD_SINGLE))
		{
			tempBuf[offset]   = 0xFF;      			
			tempBuf[offset+1] = 0xFF;				
			updatetag = TRUE;			
		}
		else
		{
			continue;
		}							
	}
	
	if(updatetag == TRUE)
	{
		WriteTFBuf(fileAddr, 0x00,tempBuf,fileSize);
	}	 
 }
