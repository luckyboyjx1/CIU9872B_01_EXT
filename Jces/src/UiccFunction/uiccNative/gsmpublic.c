/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件

 			v1.1  增加对BER-TLV结构的文件取响应

			V1.2  修改函数GetEfInformation，修改在2G模式下，对于映像文件与实体文件取不同的权限值
			
			V1.3 修改函数GetEfInformation,UsimGetEfInformation,增加对循环文件的特殊处理@20110726 by xuyz
			V1.4  修改函数UsimGetDfInformation()，修改获取电压支持的语句，改为从ATRDefine[]中获取 --chenl @20110826
			V1.5 修改函数 GetFileAcc，修改对于2G命令 下 获取 Increase权限的处理 @20140228 by xuyz
			V1.6 修改函数Update6F7BFile，增加在特殊需求下若创建FF04文件，则除6F30下的网络数据不被更改之外，增加FF04文件内的网络数据也不被更改的处理---20140418 by xwh
			V1.7 修改函数Update6F7BFile， SmartLumingRead，因为6F7B ,6F7E在ADF（7FF0） 与7F20下为映射文件，所以在更新7FF0，7F20下的这两个文件时，都需要对网络进行过滤;同时要注意，USIM ADF的AID必须为7FF0，否则需要自行修改代码或重新提交COS V1.32 ->1.33@20140630 by xuyz
			V1.8 修改函数SmartLumingRead，当6F30里面的MCC MNC为全FF时 ，继续搜索下一个与6F7E中的数据比较@20140704 by xuyz @1.34 ->1.35
			v1.9 修改函数UsimGetEfInformation，若某个文件的Arrname不是2F,6F ，若其在MF 下，则默认其Arrname为2F,否则其ArrName为6F ，@20141119 by xuyz
			V2.0 修改函数GetEFInformation,UsimGetEfInformation,修正映射文件状态总与原文件状态一致的错误  @20170728 by yangjian 
 */
#include  "typedef.h"
#include  "Interface_LTEFromBoot.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "interface.h"
#include "uicc_error.h"

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetFileAcc                                                                                                                   
                                                                                                                                                
  Input parameter:   UWORD fileAddr

                     BYTE * buf
                              					                                                                                                         
  Output parameter:  num                                                                                                               
                                                                                                                                                
  description:       compare the accessConditon can do the action
  
  status:		        

 -------------------------------------------------------------------------------------------------------------------*/  
void GetFileAcc(UWORD fileAddr,BYTE * buf)
{
	BYTE  idata arrNum,fileLgth,tempPosition,keyStatus,fileType;

    buf[0] = 0x00;

    buf[1] = 0xFF;

    buf[3] = 0xFF;

	buf[2] = 0xF0;

    fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
	
	arrNum = GetFileHeadByte(fileAddr,F_ArrNum_Offset);
    
    fileAddr = SelectLFFile(DF_MF,EF_ARR);

	if(fileAddr != FALSE)
	{
		fileLgth = GetLFRecNumLgth(fileAddr,FileLgth);

		ReadLFBuf(fileAddr,arrNum,0,&tempBuf[0],fileLgth);
	}
    else
    {
        return;
    }

  for(arrNum =0x00;((arrNum < fileLgth) && (tempBuf[arrNum] != 0xFF));arrNum = arrNum + tempPosition)
	{
		if((tempBuf[arrNum] == 0x80)&&(tempBuf[arrNum+1]==0x01))			//Find the 
		{
            tempPosition = tempBuf[arrNum + 4] ;
			
			switch(tempBuf[arrNum+3])
			{
			 	case 0x90:
				{
					keyStatus = 0x00;
					break;
				}
				case 0x97:
				{
					keyStatus = 0x0F;
					break;
				}
				case 0xA4:
				{
                    keyStatus = tempBuf[arrNum+7];

					break;
                }
			    default:
				{
				 	keyStatus = 0x0F;
				}
			}

			if(keyStatus == 0x81)												//add at 2007-04-02
			{
				keyStatus = 0x02;												//trans 0x81 to 0x02				
			}
			
			if((keyStatus > 0x09) && (keyStatus < 0x0F))
			{
				keyStatus = keyStatus - 0x06;									//trans 0x0A... to 0x04... 
			}

			if((tempBuf[arrNum+2]&A_ReadFile)==A_ReadFile)                    	//Read
            {
                buf[1] =buf[1]&((keyStatus<<4)|0x0F);
            }

            if((tempBuf[arrNum+2]&A_UpdateFile)==A_UpdateFile)                   //Update
            {
                buf[1] = buf[1]&(keyStatus|0xF0);
            }

            if((tempBuf[arrNum+2]&A_ActivateFile)==A_ActivateFile)               //active
            {
                buf[3] = buf[3]&((keyStatus<<4)|0x0F);
            }

            if((tempBuf[arrNum+2]&A_DeactivateFile)==A_DeactivateFile)           //deActive
            {
                buf[3] = buf[3]&(keyStatus|0xF0);
            }
		}
        else if((tempBuf[arrNum] == 0x84)&&(tempBuf[arrNum+1]==0x01))
        {
   
        	  if((tempBuf[arrNum + 2] == 0x32)&&(fileType == CYCTYPE)) //当前文件是 循环文件 ，配置 循环文件的权限 
			  {
			  	   if(tempBuf[arrNum + 3] == 0x90)
				   {
				   	   	keyStatus = 0x00;
				   }
				   else if(tempBuf[arrNum + 3] == 0x97)
				   {
				   	   keyStatus = 0x0F; 
				   }
				   else if(tempBuf[arrNum + 3] == 0xA4)
				   {
				   	   keyStatus = tempBuf[arrNum + 7];
				   }
				   else
				   {
     			 	    keyStatus = 0x0F;				
				   }

				   	if(keyStatus == 0x81)												//add at 2007-04-02
					{
						keyStatus = 0x02;												//trans 0x81 to 0x02				
					}
					
					if((keyStatus > 0x09) && (keyStatus < 0x0F))
					{
						keyStatus = keyStatus - 0x06;									//trans 0x0A... to 0x04... 
					}

					buf[2] = (keyStatus<<4)&0xF0;

                    buf[0] = 0x40;
			  }
        }
		else
		{
			 return;
		}

		tempPosition = 5 + tempBuf[arrNum+ 4];
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetDfInformation                                                                                                                   
                                                                                                                                                
  Input parameter:   void
                              					                                                                                                         
  Output parameter:  num                                                                                                               
                                                                                                                                                
  description:       compare the accessConditon can do the action
  
  status:		        

 -------------------------------------------------------------------------------------------------------------------*/  
BYTE GetDfInformation(void)
{
	UWORD xdata fileSpareSize;
	BYTE  idata tempResult;

	fileSpareSize = GetNVMAvailMemory();

	RecData[0]  = 0x00;         //RFU

	RecData[1]  = 0x00;


   //total memory space available
	if(fileSpareSize > 0xFFFF)
	{
		RecData[2]  = 0xFF;                                                                                                                    

  		RecData[3]  = 0xFF;
	}
	else
	{
		RecData[2]  = (fileSpareSize >> 8)&0xFF;  	//file not allocated size                                                                                                                    

  		RecData[3]  = fileSpareSize&0xFF;
	}

	IF_GetDfInformation();
	
	RecData[4] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;       //file ID

	RecData[5] = G_CurrentDfName[G_ChannelNum]&0xFF;

	if(G_CurrentDfName[G_ChannelNum] == DF_MF)
	{
		RecData[6] = MF;  
	}
	else
	{
		RecData[6] = DF;
	}

	memset((BYTE *)&RecData[7],0x00,0x05);        //RFU

	RecData[12] = 0x0A;    //followed Length
       
	RecData[13]	= 0x13;		//fileCharacteristics	0x13:允许时钟停止，无优先电平，鉴权13/4MHZ，CHV1激活	

	ReadEeprom((BYTE *)&RecData[14],G_CurrentDfAddr[G_ChannelNum]+F_ChildDFNum_Offset,sizeof(WORD));
	
	RecData[16] = 0x07; //num of CHVs,UNBLOCK CHVs,administartive codes 

	RecData[17] = 0x00; //RFU
	
	tempResult = ReadKey(PIN1,&RecData[0x20]);

	if(tempResult == FALSE)
	{
		RecData[18] = 0x00;
		
		RecData[19] = 0x00;

	    RecData[13] = RecData[13]&0x7F;
	}
	else
	{
		if((RecData[0x21]&0x01) == PIN_ENABLED)
		{
  			RecData[13] = RecData[13]&0x7F;
		}
  		else
		{
  			RecData[13] = RecData[13]|0x80;
		}

		 memcpy(&RecData[18],&RecData[0x20+PIN_LIMIT_OFFSET],2);

		 RecData[18] = ( RecData[18]& 0x0F ) | 0x80 ;

		 RecData[19] = ( RecData[19]& 0x0F ) | 0x80 ;
	}

	tempResult = ReadKey(PIN2,&RecData[0x20]);

	if(tempResult == FALSE)
	{
		RecData[20] = 0x00;
		
		RecData[21] = 0x00;
	}
	else
	{
		 memcpy(&RecData[20],&RecData[0x20+PIN_LIMIT_OFFSET],2);

		 RecData[20] = ( RecData[20]& 0x0F ) | 0x80 ;
		 
		 RecData[21] = ( RecData[21]& 0x0F ) | 0x80 ;		
	}

	RecData[22] = 0x00;

	return R_DF_LENGTH;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetEfInformation                                                                                                                   
                                                                                                                                                
  Input parameter:   void
                              					                                                                                                         
  Output parameter:  num                                                                                                               
                                                                                                                                                
  description:       compare the accessConditon can do the action
  
  status:		        

 -------------------------------------------------------------------------------------------------------------------*/  
BYTE GetEfInformation(void)
{
	BYTE idata fileType;
	WORD xdata fileSize;
	UWORD xdata  currentEfHeadAddr;

	RecData[0] = 0x00;  //RFU

	RecData[1] = 0x00;

	RecData[4]  = (G_CurrentEfName[G_ChannelNum] >> 8)&0xFF;

	RecData[5]  = G_CurrentEfName[G_ChannelNum] & 0xFF;

	RecData[6] = EF;    //type of file

	currentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);

    GetFileAcc(currentEfHeadAddr,&RecData[7]);      

	RecData[11] = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset); 
	
	if((RecData[11]&F_READ_UPDATA_ENBALE) == F_READ_UPDATA_ENBALE)
	{
		RecData[11] = (RecData[11]&0x01)|0x04;
	}
	else
	{
		RecData[11] = RecData[11]&0x01;
	}

	RecData[12] = 0x02;

	fileType = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);

	if(fileType == TRANTYPE)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);

		RecData[13] = TF; 

		RecData[14] = 0x00;  //RFU
	}
	else
	{
		RecData[14] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecLgth_Offset);
		
		RecData[2] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecNum_Offset);

		fileSize = RecData[2]*RecData[14];
	
		if((fileType&FILETYPE) == CYCTYPE)
		{
			RecData[13] = CF;
		}
		else
		{
			RecData[13] = LF; 
		}
	}

	RecData[2] = (fileSize>>8)&0xFF;

	RecData[3] = fileSize&0xFF;
	
	return 0x0F;	
}
#ifndef SingleSIM



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimGetDfInformation                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the df information to the gsm buffer
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE UsimGetDfInformation(void)
{
	UWORD xdata totalFiledSize;						//sum of all the EFs and DFs contained in this DF plus the amount of available memory in this DF
    BYTE idata num,fileNameLgth,i;
	KeyStructer xdata   pinKey;						

	RecData[0] = 0x62;

	num = 0x03;  

	RecData[num] = 0x82;

	RecData[num+1] = 0x02;

	ReadEeprom((BYTE *)&RecData[num+2],G_CurrentDfAddr[G_ChannelNum]+F_Type_Offset,sizeof(BYTE));
	
	RecData[num+2] &= 0x7F;

	RecData[num + 3] = 0x21;

	num = num + 4 ;                    										//END OF File dESCRIPTOR

	RecData[num] = 0x83;

	RecData[num + 1] = 0x02;

	RecData[num + 2] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;

	RecData[num + 3] = G_CurrentDfName[G_ChannelNum]&0xFF;

	num = num + 4 ;														   //END OF File identifier

	fileNameLgth = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum],F_DIRNum_offset);  //AID name  在2F00中的记录号
	
	if(fileNameLgth != 0x00)											  //IS ADF FILE
	{
		RecData[num] = 0x84;

		GetAIDNameInFile(fileNameLgth,&RecData[num+1]);	  //AID name:LV

		num = num + RecData[num+1]  + 2 ;	
	}

	if(G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)					//Only for MF,modify at 2007-02-08
	{
		RecData[num] = 0xA5;
	
		RecData[num+1] = 0x03;
	
		RecData[num + 2] = 0x80;	   									//UICC fileCharacteristics
	
		RecData[num + 3] = 0x01;
	
		//RecData[num+4] = 0x71;											//0x31:允许时钟停止，无优先级别，支持电压级别A、B、C
		RecData[num+4] = 0x71;//((ATRDefine[4]&0x0F)<<4)|0x01;
								
		num = num + 0x05;
	}

	RecData[num] = 0x8A;

	RecData[num + 1] = 0x01;

	RecData[num + 2] = 0x05;											//card cycle Status ??? CardStatus与LCSI冲突

	num = num + 3;														//END OF LIFE CYCLE STATUS INTEGER

	RecData[num] = 0x8B;

	RecData[num + 1] = 0x03;

	RecData[num + 2] = 0x2F;

	RecData[num + 3] = 0x06;

	RecData[num+4] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrNum_Offset); 

	num = num + 0x05;												//END OF Referenced to expanded format

	RecData[num] = 0xC6;											//PS_DO

	RecData[num+2] = 0x90;

	RecData[num+3] = 0x01;

	RecData[num+4] = 0x00;

	for(i= 0x00;i<0x04;i++)
	{
	   	RecData[num+7+(i*3)] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ChvStatus_Offset+i);

		if(RecData[num+7+(i*3)] == 0xFF)
		{
			break;
		}
		else
		{
			RecData[num+5+(i*3)] = 0x83;
		
			RecData[num+6+(i*3)] = 0x01;
		
			fileNameLgth = ReadKey(RecData[num+7+(i*3)],(BYTE*)&pinKey.KeyId);
		
			if(fileNameLgth == TRUE)
			{
				if((pinKey.KeyStatus&0x01) == PIN_ENABLED)
				{
					RecData[num+4] = RecData[num+4] + (0x80 >> i);
				}
			}
			else
			{
				break;
			}
		}
	}

	RecData[num+1] = 3*(i+1);										//L

	num = num + RecData[num+1] + 2;
	
	//num -= 3;                                 //这行代码多余，by xwh 20220519

#if NATIVE_CARD_UICC
 /*For a DF, the "total file size" represents the sum of the "total file sizes" of all the Efs and DFs contained in this DF plus
the amount of available memory in this DF. The size of the structural information of the selected DF itself is not
included. */
	RecData[num] = 0x81;	

	RecData[num+1] = 0x04;

	totalFiledSize = GetFileHeadUword( G_CurrentDfAddr[G_ChannelNum],F_UsedContentSize_Offset)+GetFileHeadUword( G_CurrentDfAddr[G_ChannelNum],F_UsedHeadSize_Offset);	

	totalFiledSize = totalFiledSize - sizeof(FileSystemHead) + GetNVMAvailMemory();

	if(sizeof(UWORD) == 0x04)
	{
		RecData[num +2] = (((unsigned long)totalFiledSize) >> 24)&0xFF;

		RecData[num +3] = (((unsigned long)totalFiledSize) >> 16)&0xFF;

		RecData[num +4] = (totalFiledSize >> 8)&0xFF;

		RecData[num +5] = totalFiledSize&0xFF;

		num = num + 2;
	}
	else
	{
		RecData[num+1] = 0x02;

	    RecData[num +2] = (totalFiledSize >> 8)&0xFF;

		RecData[num +3]	= totalFiledSize&0xFF;
	}
 	
	num = num + 0x01;
#endif

	if(num > 0x80)
	{
		RecData[1] = 0x81;

		RecData[2] = num;

		num = num + 3;
	}
	else
	{
		RecData[1] = num;

		memmove((BYTE *)&RecData[2],(BYTE *)&RecData[3],num);

		num = num + 2 ; 
	} 

	return num;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimGetEfInformation                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the ef information to the gsm buffer
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    
BYTE UsimGetEfInformation(void)                                 
{
	BYTE idata num,fileType;
	WORD xdata fileSize;
	UWORD xdata currentEfHeadAddr;

	RecData[0] = 0x62;

	num = 0x03;

	RecData[num] = 0x82;

	ReadEeprom((BYTE *)&fileType,G_CurrentEfAddr[G_ChannelNum]+F_Type_Offset,sizeof(BYTE));

	RecData[num+2] = (fileType&0x7F); 

	RecData[num + 3] = 0x21;									

	if(((RecData[num+2]&0x02)==0x02)||((RecData[num+2]&0x06)==0x06))			//记录文件
	{
		RecData[num+4] = 0x00;
		
		RecData[num+5] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecLgth_Offset);

		RecData[num+6] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecNum_Offset);	
		
		RecData[num+1] = 0x05;

		num = num + 3;
	}
	else
	{
		RecData[num+1] = 0x02;
	}

	currentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);
	
	num = num + 0x04 ;															//文件描述符

	RecData[num] = 0x83;

	RecData[num + 1] = 0x02;

	RecData[num + 2] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;

	RecData[num + 3] = G_CurrentEfName[G_ChannelNum]&0xFF;
	
	num = num + 0x04;															//文件标识符
					  				
	RecData[num] = 0x8A ;

	RecData[num + 1] = 0x01;

	RecData[num+2] = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);

	RecData[num+2] = RecData[num+2]&0x0F;

	num = num + 0x03;												//END OF Life cycle status integer

	RecData[num] = 0x8B;

	RecData[num+1] = 0x03;

 	RecData[num+2] = GetFileHeadByte(currentEfHeadAddr, F_ArrName_Offset); //modify @20140727  bit7表示当前arr是2F还是6F
	
	RecData[num+2] =  ((RecData[num+2]&F_ARRNAME_6F)==F_ARRNAME_6F?0x6F:0x2F);	//modify @20140727  bit7表示当前arr是2F还是6F
 	
	/*
 	if((RecData[num+2] != 0x2F)&&(RecData[num+2] != 0x6F))
 	{
 		if(G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)
 		{
 			RecData[num+2] = 0x2F;
 		}
 		else
 		{
 			RecData[num+2] = 0x6F;
 		}
 	}
	*/

	RecData[num+3] = 0x06;

	RecData[num+4] = GetFileHeadByte(currentEfHeadAddr, F_ArrNum_Offset);	

	num = num + 5;

	RecData[num] = 0x80;

	RecData[num +1] = 0x02;
	
	
	if((fileType&FILETYPE) == TRANTYPE)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);
	}
	else
	{
		ReadEeprom((BYTE *)&RecData[num+2],G_CurrentEfAddr[G_ChannelNum] + F_RecNum_Offset,sizeof(WORD));
		
		fileSize = RecData[num+2]*RecData[num+3];
	}

	RecData[num + 2] = (fileSize>>8)&0xFF;

	RecData[num + 3] = fileSize&0xFF;

	num = num + 0x04;
	
	RecData[num] = 0x88;																//END OF FileSize

	if(G_CurrentSfi == 0x00)														//SFI,not supoprt
	{		
		RecData[num + 1] = 0x00;
		
		num = num + 2 ;			
	}
	else
	{
		RecData[num + 1] = 0x01;

		RecData[num + 2] = (G_CurrentSfi << 3) & 0xF8;

		num = num + 3 ;
	}

	num = num - 3;																	//End of SFI

	if(num > 0x80)
	{
		RecData[1] = 0x81;

		RecData[2] = num;

		num = num + 3;
	}
	else
	{
		RecData[1] = num;

		memcpy((BYTE *)&RecData[2],(BYTE *)&RecData[3],num);

		num = num + 2 ; 
	} 

	return num;
 }
#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetChannel                                                                                                                   
                                                                                                                                                
  input parameter:   fileAddr:
  					 fileName:
					 option:	TRUE/FALSE                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the ef information to the gsm buffer
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE SetChannel(UWORD fileAddr,WORD fileName,BYTE option)
{
  BYTE idata fileType;
	BYTE idata returnLgth;

	returnLgth = 0x00;    //初始化当前选中文件的响应值返回长度

  fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

  if(fileType == ADFTYPE)					//DF  ADF  MF
  { 
		G_CurrentDfName[G_ChannelNum] = fileName;

		G_CurrentDfAddr[G_ChannelNum] = fileAddr;

		G_CurrentEfName[G_ChannelNum] = FALSE;

		G_CurrentEfAddr[G_ChannelNum] = FALSE;                              

		if(option == TRUE)
		{
#ifndef SingleSIM
				if(G_CommandMode == Command_Mode_3G)
				{
    				returnLgth = UsimGetDfInformation();
				}
				else
#endif
				{
					returnLgth = GetDfInformation();
				}
		}

  }
  	else 
	{
   		G_CurrentEfName[G_ChannelNum] = fileName;

   		G_CurrentEfAddr[G_ChannelNum] = fileAddr;

		if((fileType == CYCTYPE)&&(G_CommandMode == Command_Mode_2G))
		{
		   G_CurrentRecNum[G_ChannelNum] = 1;
		}
		else
		{
		   G_CurrentRecNum[G_ChannelNum] = 0x00;
		}

		if(option == TRUE)
		{
#ifndef SingleSIM
			if(G_CommandMode == Command_Mode_3G)
			{
   			  	returnLgth = UsimGetEfInformation();
			}
			else
#endif
			{
				returnLgth = GetEfInformation();	
			}
		}

	}

	return  returnLgth;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetCurrentADFAid                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the ef information to the gsm buffer
  
  status:			 OK         

 -------------------------------------------------------------------------------------------------------------------*/  
BYTE GetCurrentADFAid(WORD fileName,BYTE * buf)
{
    BYTE  idata fileNameLgth;
    UWORD xdata fileAddr;

    fileAddr = SelectFile(E_FILE_START,fileName);

    if(fileAddr == FALSE)
    {
        return FALSE;
    }

   	fileNameLgth = GetFileHeadByte(fileAddr,F_DIRNum_offset);  //AID name  在2F00中的记录号
	
	if(fileNameLgth != 0x00)											  //IS ADF FILE
	{
		buf[0] = 0x84;

		GetAIDNameInFile(fileNameLgth,&buf[1]);  //LV

		return (buf[1]+2);
	}

	return FALSE;
}

#if NATIVE_CARD_UICC
/*
*@description:在E_COS_ConfigData区查找指定的tag
*
*@param      :tag    ---> 要查找的tag
*             outByte ---> 存放找到的数据(1字节)，如果为NULL，不读取数据
*@return     :offset ---> tag的偏移
*
*@history    :create by xwh 20180404
*/

WORD FindCOSConfigTag(BYTE tag,BYTE * outByte)
{
	WORD  idata offset;
	BYTE  idata buf[2];

	for(offset = 0; offset < (E_ConfigData_LEN-2); )                                   
	{
		ReadEeprom(&buf[0],E_COS_ConfigData+offset,sizeof(buf));

		if(buf[0] == PRIVATETAG_NULL)       
		{
			if(tag == PRIVATETAG_NULL)                                 //查找空闲地址
			{
				return offset;
			}
			else
			{
				return OFFSET_NULL;
			}
		}

		if(buf[0] == tag)
		{
			if(outByte != NULL)
			{
				ReadEeprom(&outByte[0],E_COS_ConfigData+offset+2,1);//读取Value，为了防止越界使用，只 支持读一个字节
			}
		
			return offset;
		}
		else
		{
			offset += (buf[1]+2);             //Skip current TLV
		}
	}

	return OFFSET_NULL;
}
#endif





