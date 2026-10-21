/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	

 * HISTORY: V1.0 创建源文件
			V1.1 修改函数SmartLumingRead,取IMSI值的方式改变
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





/**
 *@brief       当终端一次向6F7B文件写入多条网络信息（MCC+MNC）时，SIM卡应在滤除出现在6F30/FF03文件中的网络信息后，将剩余的信息（如果有剩余信息）写入到6F7B文件中". 
 *
 *@param       fileName: FF03/6F30
 *
 *@wuthor      XYZ
 *
 *@history     20090820
*/
BYTE Update6F7BFile(WORD fileName)
{
	UWORD idata fileAddr,fileAddrExtern;
	WORD  idata fileSize,plmnfileSize,fileSizeExtern;
	BYTE  idata fPlmn[3];
	WORD  idata i,j;
	BYTE  idata tag;

#ifdef 	SingleSIM 
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7B)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)))	//134号段
#else
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7B)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)||(G_CurrentDfName[G_ChannelNum] == 0x7FF0)))	//134号段
#endif
    {
		plmnfileSize = GetTFSize(G_CurrentEfAddr[G_ChannelNum]);

		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],0,(BYTE *)&RecData[0x50],plmnfileSize);

		fileAddr = SelectTFFile(DF_PHASE, fileName);                  // FPLMN不允许更新成0x6F30/0xFF03中数据

		fileAddrExtern = FALSE;

		if(fileName == EF_PLMNsel)
		{
			fileAddrExtern = SelectTFFile(DF_PHASE,0xFF04);
		}
	
		if(fileAddr != FALSE)
		{
			fileSize = GetTFSize(fileAddr);
			
			for(i=0;i<P3;i=i+3)
    		{
    			tag = TRUE;
    			
    			for(j=0;j<fileSize;j=j+3)                                   // fileSize should be less then 0x100
				{
					ReadTFBuf(fileAddr,j,&fPlmn[0],0x03);
    				
					if((fPlmn[0] != 0xFF)||(fPlmn[1] != 0xFF)||(fPlmn[2] != 0xFF)) //modify by xwh
					{
					    if((RecData[i]== fPlmn[0])&&(RecData[i+1]==fPlmn[1])&&(RecData[i+2]==fPlmn[2]))
			 			{
			 				tag = FALSE;
			 				break;	            			
		          		}
					}	
	          	}

				if((fileAddrExtern != FALSE)&&(tag == TRUE))
				{
					fileSizeExtern = GetTFSize(fileAddrExtern);

					for(j=0;j<fileSizeExtern;j=j+3)                                   // fileSize should be less then 0x100
					{
						ReadTFBuf(fileAddrExtern,j,&fPlmn[0],0x03);
	    				
						if((fPlmn[0] != 0xFF)||(fPlmn[1] != 0xFF)||(fPlmn[2] != 0xFF)) //modify by xwh
						{
						    if((RecData[i]== fPlmn[0])&&(RecData[i+1]==fPlmn[1])&&(RecData[i+2]==fPlmn[2]))
				 			{
				 				tag = FALSE;
				 				break;	            			
			          		}
						}	
		          	}	
				}
	          	
	          	if(tag == TRUE)
	          	{
	          		memcpy(&RecData[0x50+P2+i],&RecData[i],3);
	          	}				
    		}						
		}
		else
		{
			memcpy((BYTE *)&RecData[0x50+P2],(BYTE *)&RecData[0],P3);
		}

		WriteTFBuf(G_CurrentEfAddr[G_ChannelNum],0,(BYTE *)&RecData[0x50],plmnfileSize);
		
		return FALSE;
	}

	return TRUE;
}


/**
 * @brief 		Funtion134Write() 在WRITE BINIARY中处理134号段的函数
 * 
 * @author 		XYZ,XWH 
 * 
 * @history 	2009-05-11,"当终端一次向6F7B文件写入多条网络信息（MCC+MNC）时，SIM卡应在滤除出现在6F30文件中的网络信息后，将剩余的信息（如果有剩余信息）写入到6F7B文件中". 
 */
/*
BYTE Funtion134Write(void)
{
	return Update6F7BFile(0xFF03);
}
*/

/**
 * @brief 		SmartLumingWrite() 在WRITE BINIARY中处理优选漫游的函数
 * 
 * @author 		XYZ,XWH 
 * 
 * @history 	2009-05-11,根据优选漫游规范修改,"当终端一次向6F7B文件写入多条网络信息（MCC+MNC）时，SIM卡应在滤除出现在6F30文件中的网络信息后，将剩余的信息（如果有剩余信息）写入到6F7B文件中". 
 */

BYTE SmartLumingWrite(void)
{
   	if((G_CurrentEfName[G_ChannelNum] == EF_PLMNsel)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21))) //不允许本地更新6F30,2008-03-07
    {
		return FALSE;
	}
	
	return Update6F7BFile(EF_PLMNsel);
}


/**
 * @brief 		SmartLumingRead() 在READ BINIARY中处理优选漫游的函数
 * 
 * @author 		XYZ,XWH 
 * 
 * @history 	2009-05-11,根据优选漫游规范修改,"当终端一次向6F7B文件写入多条网络信息（MCC+MNC）时，SIM卡应在滤除出现在6F30文件中的网络信息后，将剩余的信息（如果有剩余信息）写入到6F7B文件中". 
 */

void SmartLumingRead(void)
{
	BYTE idata LAI_6F7E[3];
	BYTE idata LAI_6F07[3];
	BYTE idata LAI_Tmp[3];
	BYTE idata LAI_6F30[3];
	UWORD idata fileAddr;
	WORD idata fileSize,fileLgth;
	BYTE idata i;

#ifdef 	SingleSIM
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7E)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21))) 
#else
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7E)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)||(G_CurrentDfName[G_ChannelNum] == 0x7FF0))) //EF_LOCI,2008-03-07
#endif
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],4, &LAI_6F7E[0], 3);              				// MCC,MNC

		ReadEeprom(&LAI_Tmp[0],E_SECURE_DATA + IMSI1_OFF + 1,3); 	

		LAI_6F07[0] = ((LAI_Tmp[1]<<4)&0xF0) | ((LAI_Tmp[0]>>4)&0x0F); 				// 64

		LAI_6F07[1] = 0xF0 | ((LAI_Tmp[1]>>4)&0x0F);                   				// F0

		if((LAI_6F7E[0] != LAI_6F07[0])||((LAI_6F7E[1]&0x0F) != (LAI_6F07[1]&0x0F)))
		{
			fileAddr = SelectTFFile(DF_PHASE,EF_PLMNsel);

			if(fileAddr == FALSE)
			{
				return;
			}

			fileSize = GetFileHeadWord( fileAddr,F_SpaceSize_Offset);	
			
			for(i=1;i<=(BYTE)(fileSize/3);i++)
			{
				ReadTFBuf(fileAddr, i*3-3, &LAI_6F30[0], 3);
				
				if((LAI_6F30[0] == 0xFF)&&(LAI_6F30[1] == 0xFF)&&(LAI_6F30[2] == 0xFF))
				{
					continue;
				}

				if((LAI_6F7E[0] == LAI_6F30[0])&&((LAI_6F7E[1]&0x0F) == (LAI_6F30[1]&0x0F)))
				{
					fileAddr = SelectTFFile(DF_PHASE, 0x6F7E);

					WriteTFBuf(fileAddr,4,LAI_6F30,3);

					break;
				}
			}
		}

		fileLgth = ((P1 << 8)&0xFF00 )|(P2 & 0x00FF);
	
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],P3);
	}
}