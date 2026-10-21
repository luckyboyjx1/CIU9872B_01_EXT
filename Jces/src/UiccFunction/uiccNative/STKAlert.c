/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2014-09-26

 * AUTHER:	zengxiao

 * HISTORY: V1.0 创建源文件
            V1.1 修改函数PendingMessageAlert与ProcessPendingMessageAlert,改为一个标记占用2个字节,兼容某些RFM需求   

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
#include  "interface.h"


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 PendingMessageAlert
  
  input parameter:   void					                     

  output parameter:	 void
                                                                                                                                                
  description:       对OTA进行标记处理
  
  status:			 OK
  
  author:			zhuli
  
  testor:			Caibo
  
  status :			OK    

 -------------------------------------------------------------------------------------------------------------------*/    
void PendingMessageAlert(BYTE ch,BYTE refNumber)
{
	UWORD idata fileAddr;
	WORD  idata offset;
	WORD  idata fileSize;
	BYTE  idata buf[2];

	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_STKAlert);	
	
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



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 PendingMessageAlert
  
  input parameter:   void					                     

  output parameter:	 void
                                                                                                                                                
  description:       清除某个标记值
  
  status:			 OK
  
  author:			zhuli
  
  testor:			Caibo
  
  status :			OK    

 -------------------------------------------------------------------------------------------------------------------*/    
void ClearingMessageAlert(BYTE ch)
{
	UWORD xdata fileAddr;
	WORD  idata offset;
	WORD  idata fileSize;
	BYTE  idata buf[2];

	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_STKAlert);

	if(fileAddr == FALSE)
	{
		return;
	}

	fileSize = 	GetTFSize(fileAddr);

	for(offset=0; offset<fileSize; offset=offset+2)
	{
		 ReadTFBuf(fileAddr, offset, &buf[0], sizeof(buf));

			if(buf[0] == ch)
			{
				SetTFConst(fileAddr, offset, 0xFF, sizeof(buf));

				return;
			}	
		
	}	
	
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ProcessPendingMessageAlert
  
  input parameter:   void
					                     

  output parameter:	 void
                                                                                                                                                
  description:       Process OTA2 pending message
  
  status:			     

 -------------------------------------------------------------------------------------------------------------------*/    
void ProcessPendingMessageAlert(void)
{
	BYTE  idata ch[2];
	UWORD xdata fileAddr;
	WORD  idata alertSize;
	WORD  idata alertPoint;
	
	if (STKPerform != FALSE)
	{
		return;
	}

	fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_STKAlert);

	if(fileAddr == FALSE)
	{
		return ;
	}	

	alertSize = GetTFSize(fileAddr);
	
	while(1)
	{
		for(alertPoint=0x00; alertPoint<alertSize; alertPoint=alertPoint+2)
		{
			fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_STKAlert);						//一次循环后,fileAddr可能改变,要重新选择文件
			
			ReadTFBuf(fileAddr, alertPoint, (BYTE *)&ch[0], sizeof(ch));

			if(ch[0] != 0xFF)
			{			
				break;
			}
		}

		if(alertPoint >= alertSize)											//没有找到标记
		{
			IF_ResumeResponseMode();
			
			return;	
		}

		SetTFConst(fileAddr, alertPoint, 0xFF, sizeof(ch));

		switch(ch[0])
		{	
			case D_SetUpMenuTag:  

				SetUpMenu();

				break;
			
			
			default:
					
				IF_ProcessPendingMessageAlert(ch[0],ch[1]);	
						
				break;
		}
	}
	
}

