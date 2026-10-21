/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	

 * HISTORY: V1.0 创建源文件

 */		
#include  "typedef.h"
#include  "hard_api.h"
#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "interface.h"
#include  "error.h"



#ifdef IOT_PLMN_FB
/**
 * @brief 	   判断要更新的数据是否为全0xFF
 *
 * @Returns    TRUE：  表示更新的值为全0xFF
               FALSE： 表示更新的值不为全0xFF
 * 
 */
/*
BYTE IfAllFF(BYTE *buf,BYTE len)
{
	BYTE idata i;						//最大支持255个字节比较
	
	for(i=0;i<len;i++)
	{
		if(buf[i]!=0xFF)
		{
			return FALSE;
		}
	}
	
	return TRUE;	


}
*/


/**
 * @brief 	根据输入的mccmnc值,若该值在文件IMSI或ADF/6F61,6F62,6FD9,6F30中存在,则返回FALSE,否则返回TRUE
 *
 * @Returns    TRUE：  表示未找到禁止更新的值
               FALSE： 表示已找到禁止更新的值
 * 
 */


BYTE IsMCCMNCPermit(BYTE *mccmnc)
{
	BYTE idata i,j,dataBlockLen;	
	WORD  idata fileSize,fileName;		
	UWORD xdata fileAddr,imsiAddr;
	BYTE  xdata fPlmn[3],tem[3],imsiLocValue[4];	
	

	imsiAddr = 	SelectTFFile(DF_ADF, EF_IMSI); 							//先对比IMSI中的值
	ReadTFBuf(imsiAddr,1,&tem[0],0x03);
	imsiLocValue[0] = (tem[0]>>4)&0x0F;								   //前两个字节转换
	imsiLocValue[1] = tem[1]&0x0F;
	imsiLocValue[2] = (tem[1]>>4)&0x0F;
	imsiLocValue[3] = tem[2];										   //后一个字节直接取值
	
     			    		                                  					
	fPlmn[0]  = mccmnc[0]&0x0F;							   				//前两个字节转换后再比较
	fPlmn[1]  = (mccmnc[0]>>4)&0x0F;
	fPlmn[2]  = mccmnc[1]&0x0F;
						
	if((imsiLocValue[0]== fPlmn[0])&&(imsiLocValue[1]==fPlmn[1])&&(imsiLocValue[2]==fPlmn[2]))
	{ 			
		if((tem[0] != 0xFF)||(tem[1] != 0xFF)||(tem[2] != 0xFF)) 		//连续3个0xFF为无效值
		{
			if(imsiLocValue[3] == mccmnc[2])						//最后一个字节直接比较即可
			{
				return FALSE;								   		//更新6F7B的数据中含有IMSI中的MCCMNC,丢弃该记录
			} 
		} 			
	}
										      							
	for(i=1;i<5;i++)										  	//再查找文件ADF/6F61,6F62,6FD9中是否存在该值
	{			
		if(i == 0x01)
		{
			fileName = EF_EHPLMN;
			dataBlockLen = 0x03;				
		}
		else if(i == 0x02)
		{
			fileName = EF_OPLMNWACT;
			dataBlockLen = 0x05;	
		}
		else if(i == 0x03)
		{
			fileName = EF_HPLMNWACT;
			dataBlockLen = 0x05;	
		}
		else if(i == 0x04)
		{
			fileName = EF_PLMNsel;
			dataBlockLen = 0x03;	
		}
	
		if(i<4)
		{
			fileAddr = SelectTFFile(DF_ADF, fileName); 
		}
		else
		{
			fileAddr = SelectTFFile(DF_PHASE, fileName);
		}
		
		              	
		if(fileAddr != FALSE)
		{
			fileSize = GetTFSize(fileAddr);
					    		                                 				
			for(j=0;j<fileSize;j=j+dataBlockLen)									  
			{
				ReadTFBuf(fileAddr,j,&fPlmn[0],0x03);
    				
				if((fPlmn[0] != 0xFF)||(fPlmn[1] != 0xFF)||(fPlmn[2] != 0xFF)) //连续3个0xFF为无效值
				{
					if((mccmnc[0]== fPlmn[0])&&(mccmnc[1]==fPlmn[1])&&(mccmnc[2]==fPlmn[2]))
			 		{ 			
						return FALSE;								   		//只要找到指定文件中有相同的值,直接返回FALSE,丢弃该值        			
		          	}
				}	
	         }    							 												
		}

	}

	return TRUE;										//都找不到,表示该值允许被更新,返回TRUE



}






/**
 * @brief 		IfUpdate6F7BAllow函数,根据ADF/6F61,6F62,6FD9的值或要更新的值,判断更新6F7B的指令是否需要丢弃
 *
 * @Returns     FALSE： 表示指令需要丢弃 
                TRUE：  表示指令不丢弃,可以正常执行
 *
 * 
 */

BYTE IfUpdate6F7BAllow(void)
{	
	BYTE idata i,result;		
	BYTE xdata mccmncBuf[3];
			
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7B)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)||(G_CurrentDfName[G_ChannelNum] == DF_ADF)))
	{    		
		
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],P2,(BYTE *)&tempBuf[0],P3);				//把需要更新的原数据段读出
			
		for(i=0;i<P3;i=i+3)	
		{
			memcpy(&mccmncBuf[0],&RecData[i],3);				//按顺序取出每一组MCCMNC

			
			result = IsMCCMNCPermit(&mccmncBuf[0]);				
			
			if(result == FALSE)						        //表示在相关文件中找到禁止更新的网段,丢弃该指令
			{
				return FALSE;																
			}
					
		}
		
	}

	return TRUE;


}
#endif
