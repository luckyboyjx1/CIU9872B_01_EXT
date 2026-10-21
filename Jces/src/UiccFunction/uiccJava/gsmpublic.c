/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1  增加对BER-TLV结构的文件取响应
			V1.2  修改函数GetEfInformation，修改在2G模式下，对于映像文件与实体文件取不同的权限值
			V1.3  修改函数GetEfInformation,UsimGetEfInformation,增加对循环文件的特殊处理@20110726 by xuyz
			V1.4  修改函数UsimGetDfInformation()，修改获取电压支持的语句，改为从ATRDefine[]中获取 --chenl @20110921
			V1.5  定义宏BERTLVCmd，将BER-TLV结构的处理放入该宏内 --20111027 by chenl
			V1.6  修改函数 GetFileAcc，修改对于Increase权限的获取处理@20140228 by xuyz
			V1.7  修改函数UsimGetDfInformation，当获取某个df的信息是，其ARRNAME通过获取其文件头配置信息得到@20140508by xuyz
			V1.8  修改函数UsimGetDfInformation，文件头中表示2F或者6F的字节arrName修改为由此字节的bit7表示@20140727 xuyz

			  ****************************************new for UICC API***********************  
			V1.9  修改函数GetFileAcc，支持当6F06不是2F06映射文件时， 可以通过直接选择当前DF下的6F06 获取权限@20151116 by xuyz -for 芬兰文件系统

			V1.10 修改函数GetDfInformation、GetEfInformation、UsimGetDfInformation、UsimGetEfInformation，增加输入参数 @20151117 BY xuyz

			V1.11 修改函数BaseUsimGetDfInformation  BaseUsimGetDfInformation删除宏定义RFM_SEPARATE_PERMIT  @20151201 by xuyz

			V1.12 修改函数GetFileAcc，因为该代码支持RFM权限配置，因此需要对arrName进行转换@20151202 by xuyz
 */

#include  "typedef.h"
#include  "uicc_main.h"
//#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
//#include  "JCREMain.h"
#include  "uicc_interface.h"
#include "Interface_LTEFromPlatform.h"	//remove warning,dengyiwen,2022/9/27
/*
*@Description: 将3G模式下的权限字段转换成2G模式下的权限字段
*
*@function   : FileAccTransate
*
*@param       :AccBuf
*
*@return       :accesstion2G
*
*@history      :V1.0 :create by @20171116
*/
BYTE FileAccTransate(BYTE *AccBuf )
{
	BYTE idata keyStatus;
	
	switch(AccBuf[0])
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
			keyStatus = AccBuf[4];

			break;
		}
		case 0xA0:                            //OR template  
		case 0xAF:                            //AND template
		{
			keyStatus = AccBuf[6];  //OR 或者AND，选择第一个权限,by xwh 20181203

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
	
	return keyStatus;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetFileAcc                                                                                                                   
                                                                                                                                                
  Input parameter:   UWORD fileAddr

                     BYTE * buf
                              					                                                                                                         
  Output parameter:  num                                                                                                               
                                                                                                                                                
  description:       compare the accessConditon can do the action
  
  status:		        
  history:           modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
 -------------------------------------------------------------------------------------------------------------------*/  
void GetFileAcc(UWORD fileAddr,BYTE * buf)
{
	BYTE  idata arrNum,fileLgth,tempPosition,keyStatus,fileType;

	BYTE idata  arrName;
	WORD idata parName;
	UWORD idata arrFileAddr;

    buf[0] = 0x00;

    buf[1] = 0xFF;

    buf[3] = 0xFF;

	buf[2] = 0xF0;  //RFU : 3  11.11 Coding of Access Conditions

    fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
	
	arrNum = GetFileHeadByte(fileAddr,F_ArrNum_Offset);

	arrName = GetFileHeadByte(fileAddr,F_ArrName_Offset);  //为兼容6F06不是2F06映射文件情况下，获取权限，有些国外产品会指定文件系统，且每个DF下的6F06配置的信息不一样，例如芬兰项目	
    
    arrName =  ((arrName&F_ARRNAME_6F)==F_ARRNAME_6F?0x6F:0x2F);

	if((G_CurrentDfName[G_ChannelNum] == DF_MF)||(arrName == 0x2F)) //如果当前df文件是3F00 或 文件中配置的Arr是2F，则权限从3F00/2F06获取
    {
         arrFileAddr = SelectLFFile(DF_MF,EF_ARR);
    }
    else 
    {
        fileAddr = G_CurrentDfAddr[G_ChannelNum];

        while(1)
        {
			 arrFileAddr = SelectFile(fileAddr,0x6F06) ;   //选择当前df下的6F06

			 if(arrFileAddr != FALSE)
			 {
			 	break;
			 }
            parName = GetFileHeadWord(fileAddr, F_SecondParentName_Offset); 

            fileAddr = GetParentOrMapFileAddr(fileAddr,F_ParentName_Offset);  //如果当前DF下不存在6F06 文件，则继续向上查找父DF的权限文件

			 if(parName == DF_MF)	  //DF下的6F06 不存在，且DF的父DF为MF，则选择文件3F00/2F06
            {
				 arrFileAddr = SelectLFFile(DF_MF,EF_ARR);  //3F00/2F06  必须创建
                 break ;
            }
        }
    }

	fileAddr = arrFileAddr;


	if(fileAddr != FALSE)
	{
		//2F06FF和2F06FE两条记录专门给私有文件使用，权限为全部NEV和ADM, by xwh 20190321
		if(arrNum == ARR_RECNUM_NEV)
		{
			 buf[1] = 0xFF;
   			 buf[2] = 0xF0;
			 buf[3] = 0xFF; 
			 return;
		}
		else if(arrNum == ARR_RECNUM_ADM)
		{
			 buf[1] = 0x44;
   			 buf[2] = 0x40;
			 buf[3] = 0x44; 
			 return;
		}
#if UICC_AB8C_ARR_PRIVATE
		if((arrNum&A_PRIVATEARR_REC_FLAG)==A_PRIVATEARR_REC_FLAG)
		{
			fileLgth = GetPrivateArrInfoToExpand(arrNum,tempBuf,tempBuf+MaxUseLen);
		}
		else
#endif
		{
			fileLgth = GetLFRecNumLgth(fileAddr,FileLgth);

			ReadLFBuf(fileAddr,arrNum,0,&tempBuf[0],fileLgth);
		}
	}
    else
    {
        return;
    }

	//for(arrNum =0x00;((tempBuf[arrNum] != 0xFF) && (arrNum < fileLgth));arrNum = arrNum + tempPosition)
	//判断条件位置换一下，防止越界访问
	for(arrNum =0x00;((arrNum < fileLgth)&&(tempBuf[arrNum] != 0xFF));arrNum = arrNum + tempPosition)
	{
		if((tempBuf[arrNum] == 0x80)&&(tempBuf[arrNum+1]==0x01))			//Find the 
		{
            tempPosition = tempBuf[arrNum + 4] ;
			
// 			switch(tempBuf[arrNum+3])
// 			{
// 			 	case 0x90:
// 				{
// 					keyStatus = 0x00;
// 					break;
// 				}
// 				case 0x97:
// 				{
// 					keyStatus = 0x0F;
// 					break;
// 				}
// 				case 0xA4:
// 				{
//                     keyStatus = tempBuf[arrNum+7];

// 					break;
//                 }
// 				case 0xA0:                            //OR template  
// 				case 0xAF:                            //AND template
// 				{
// 					 keyStatus = tempBuf[arrNum+9];  //OR 或者AND，选择第一个权限,by xwh 20181203

// 					break;
// 				}
// 			    default:
// 				{
// 				 	keyStatus = 0x0F;
// 				}
// 			}

// 			if(keyStatus == 0x81)												//add at 2007-04-02
// 			{
// 				keyStatus = 0x02;												//trans 0x81 to 0x02				
// 			}
// 			
// 			if((keyStatus > 0x09) && (keyStatus < 0x0F))
// 			{
// 				keyStatus = keyStatus - 0x06;									//trans 0x0A... to 0x04... 
// 			}

			keyStatus = FileAccTransate(&tempBuf[arrNum+3]);
			
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
// 		  	   if(tempBuf[arrNum + 3] == 0x90)
// 			   {
// 			   	   	keyStatus = 0x00;
// 			   }
// 			   else if(tempBuf[arrNum + 3] == 0x97)
// 			   {
// 			   	   keyStatus = 0x0F; 
// 			   }
// 			   else if(tempBuf[arrNum + 3] == 0xA4)
// 			   {
// 			   	   keyStatus = tempBuf[arrNum + 7];
// 			   }
// 			   else
// 			   {
// 		 	       keyStatus = 0x0F;				
// 			   }
// 	
// 		   	if(keyStatus == 0x81)												//add at 2007-04-02
// 			{
// 				keyStatus = 0x02;												//trans 0x81 to 0x02				
// 			}
// 			
// 			if((keyStatus > 0x09) && (keyStatus < 0x0F))
// 			{
// 				keyStatus = keyStatus - 0x06;									//trans 0x0A... to 0x04... 
// 			}

			keyStatus = FileAccTransate(&tempBuf[arrNum+3]);

			buf[2] = buf[2]&((keyStatus<<4)|0x0F); // (keyStatus<<4)&0xF0;

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
BYTE GetDfInformation(BYTE * dataBuf)
{
	UWORD xdata fileSpareSize,pukAddr;
	BYTE  idata tempResult,keyId,num;

	fileSpareSize = GetHeapBase() - GetMaxProfileMFEnd();	

	dataBuf[0]  = 0x00;         //RFU

	dataBuf[1]  = 0x00;

	if(fileSpareSize > 0xFFFF)
	{
		dataBuf[2]  = 0xFF;                                                                                                                    

  		dataBuf[3]  = 0xFF;
	}
	else
	{
		dataBuf[2]  = (fileSpareSize >> 8)&0xFF;  	//file not allocated size                                                                                                                    

  		dataBuf[3]  = fileSpareSize&0xFF;
	}
	
	dataBuf[4] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;       //file ID

	dataBuf[5] = G_CurrentDfName[G_ChannelNum]&0xFF;

	if(G_CurrentDfName[G_ChannelNum] == DF_MF)
	{
		dataBuf[6] = MF;  
	}
	else
	{
		dataBuf[6] = DF;
	}

	ecpMem_Set((BYTE xdata *)&dataBuf[7],0x00,0x05);        //RFU

	dataBuf[12] = 0x0A;    //followed Length
       
	dataBuf[13]	= 0x13;		//fileCharacteristics	0x13:允许时钟停止，无优先电平，鉴权13/4MHZ，CHV1激活	

	ReadEeprom((BYTE xdata *)&dataBuf[14],G_CurrentDfAddr[G_ChannelNum]+F_ChildDFNum_Offset,sizeof(WORD));
	
	dataBuf[16] = 0x07; //num of CHVs,UNBLOCK CHVs,administartive codes 

	dataBuf[17] = 0x00; //RFU
	
	tempResult = ReadKey(PIN1,&dataBuf[0x20]);
	pukAddr = SelectLFFile(DF_PROFILECFG,EF_PRIVATE_PUK);

	if((tempResult == FALSE) || (pukAddr == FALSE))
	{
		dataBuf[18] = 0x00;
		
		dataBuf[19] = 0x00;

	    dataBuf[13] = dataBuf[13]&0x7F;
	}
	else
	{
		if((dataBuf[0x21]&0x01) == PIN_ENABLED)
		{
  			dataBuf[13] = dataBuf[13]&0x7F;
		}
  		else
		{
  			dataBuf[13] = dataBuf[13]|0x80;
		}

		 ecpMem_Copy(&dataBuf[18],&dataBuf[0x20+KEY_LIMIT_OFF],1);

		 dataBuf[18] = ( dataBuf[18]& 0x0F ) | 0x80 ;//pin1的最大剩余次数
		
		 num = LocateRecordInFile(pukAddr,dataBuf[0x20+KEY_PUKREF_OFF]);//获取pin1引用的puk信息
		
		 ReadLFBuf(pukAddr,num,1,&dataBuf[19],1);

		 dataBuf[19] = ( dataBuf[19]& 0x0F ) | 0x80 ;//获取puk的最大剩余次数
	}

#if SE_ONLY
	keyId = 0x81;

#else
	if(getADFAddr() != FALSE)
	{
		keyId = 0x81;
	}
	else

	{
		keyId = 2;
	}
#endif

	tempResult = ReadKey(keyId,&dataBuf[0x20]);//读取PIN2的信息

	if(tempResult == FALSE)
	{
		dataBuf[20] = 0x00;
		
		dataBuf[21] = 0x00;
	}
	else
	{
		 ecpMem_Copy(&dataBuf[20],&dataBuf[0x20+KEY_LIMIT_OFF],1);

		 dataBuf[20] = ( dataBuf[20]& 0x0F ) | 0x80 ;//获取PIN2的最大剩余次数
		
		 num = LocateRecordInFile(pukAddr,dataBuf[0x20+KEY_PUKREF_OFF]);//获取PIN2引用的puk信息
		
		 ReadLFBuf(pukAddr,num,1,&dataBuf[21],1);
		 
		 dataBuf[21] = ( dataBuf[21]& 0x0F ) | 0x80 ;	//获取PUK的最大剩余次数	
	}

	dataBuf[22] = 0x00;

	return R_DF_LENGTH;
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetEfInformation                                                                                                                   
                                                                                                                                                
  Input parameter:   void
                              					                                                                                                         
  Output parameter:  num                                                                                                               
                                                                                                                                                
  description:       compare the accessConditon can do the action
  
  status:		        

 -------------------------------------------------------------------------------------------------------------------*/  
BYTE GetEfInformation(BYTE * dataBuf)
{
	BYTE idata fileType;
	WORD xdata fileSize,fileUsedSize,maxSize;
	UWORD xdata  CurrentEfHeadAddr;

	dataBuf[0] = 0x00;  //RFU

	dataBuf[1] = 0x00;

	dataBuf[4]  = (G_CurrentEfName[G_ChannelNum] >> 8)&0xFF;

	dataBuf[5]  = G_CurrentEfName[G_ChannelNum] & 0xFF;

	dataBuf[6] = EF;    //type of file

	CurrentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);

    GetFileAcc(CurrentEfHeadAddr,&dataBuf[7]);      

	dataBuf[11] = GetFileHeadByte(CurrentEfHeadAddr, F_Status_Offset);  
	
	if((dataBuf[11]&F_READ_UPDATA_ENBALE) == F_READ_UPDATA_ENBALE)
	{
		dataBuf[11] = (dataBuf[11]&0x01)|0x04;
	}
	else
	{
		dataBuf[11] = dataBuf[11]&0x01;
	}

	dataBuf[12] = 0x02;

	fileType = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);

	if(fileType == TRANTYPE)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);

		dataBuf[13] = UICC_TF; 

		dataBuf[14] = 0x00;  //RFU
	}
#if UICC__FUNCTION__BERTLVCmd
	else if((fileType&0x3F) == BERTLVTYPE)  //BER-TLV
	{

		fileUsedSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_UsedSize);

		maxSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);

		fileSize = maxSize - fileUsedSize;

		dataBuf[13] = 0x04; //BER-TLV

	}
#endif
	else
	{
		dataBuf[14] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecLgth_Offset);
		
		dataBuf[2] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecNum_Offset);

		fileSize = dataBuf[2]*dataBuf[14];
	
		if((fileType&FILETYPE) == CYCTYPE)
		{
			dataBuf[13] = CF;
		}
		else
		{
			dataBuf[13] = LF; 
		}
	}

	dataBuf[2] = (fileSize>>8)&0xFF;

	dataBuf[3] = fileSize&0xFF;
	
	return 0x0F;	
}
#if ((!SE_ONLY) && UICC_AB8C_ARR_PRIVATE)
/*
*created by xieyong@20211223 获取默认文件系统AB规则信息
*/
BYTE GetDefaultFileSystemPrivateArrABInfo(UWORD addr,BYTE arrNum,BYTE * buf)
{
	UWORD idata arrFileAddr;

	arrFileAddr = SelectFile(addr,DF_Toolkit_P);
	if(arrFileAddr == FALSE)
	{
		return FALSE;
	}

	arrFileAddr = SelectFile(arrFileAddr, EF_ARR_PRIVATE);
	if(arrFileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(arrFileAddr, arrNum&A_PRIVATEARR_REC_UNMASK, 0, &buf[0], 2);

	ReadLFBuf(arrFileAddr, arrNum&A_PRIVATEARR_REC_UNMASK, 2, &buf[2], buf[1]);

	return (buf[1]+2);
}
#endif
#if UICC_AB8C_ARR_PRIVATE
/*
*created by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
*/
BYTE GetPrivateArrABInfo(BYTE arrNum,BYTE * buf)
{
	UWORD idata arrFileAddr;
	
	arrFileAddr = SelectLFFile(DF_Toolkit_P,EF_ARR_PRIVATE);

	if(arrFileAddr == FALSE)
	{
		return FALSE;
	}

	ReadLFBuf(arrFileAddr, arrNum&A_PRIVATEARR_REC_UNMASK, 0, &buf[0], 2);	

	ReadLFBuf(arrFileAddr, arrNum&A_PRIVATEARR_REC_UNMASK, 2, &buf[2], buf[1]);	

	return (buf[1]+2);
}
/*
*created by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
*/
BYTE GetPrivateArrInfoToExpand(BYTE arrNum,BYTE * buf,BYTE * bufTemp)
{
	UWORD idata arrFileAddr;
	BYTE  idata recLenLgth,realLen;

	arrFileAddr = SelectLFFile(DF_Toolkit_P,EF_ARR_PRIVATE);

	if(arrFileAddr == FALSE)
	{
		return FALSE;
	}

	recLenLgth = GetLFRecNumLgth(arrFileAddr,FileLgth);

	ReadLFBuf(arrFileAddr, arrNum&A_PRIVATEARR_REC_UNMASK, 0,&bufTemp[0], recLenLgth);		

	if(bufTemp[0] == 0xAB)
	{
		ecpMem_Move(buf,&bufTemp[2],bufTemp[1]);
		return bufTemp[1];
	}
	else if(bufTemp[0] == 0x8C)
	{
		CompactToExpand((BYTE *)&bufTemp[2],(BYTE * )&buf[0]);

		realLen = buf[0];
		ecpMem_Move((BYTE * )&buf[0],(BYTE * )&buf[1],realLen);
		return realLen;
	}
	else
	{
		return FALSE;
	}
}

#endif
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UsimGetDfInformation                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the df information to the gsm buffer
  
  status:			 OK         
  history:           modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
 -------------------------------------------------------------------------------------------------------------------*/    

BYTE UsimGetDfInformation(BYTE *dataBuf)
{
	//UWORD xdata totalFiledSize;						//sum of all the EFs and DFs contained in this DF plus the amount of available memory in this DF
    BYTE idata num,fileNameLgth,i;
	KeyStructer xdata   pinKey;						
	BYTE idata addLen,chvNum,tag;
#if UICC_AB8C_ARR_PRIVATE
	BYTE  idata arrNum;
#endif	
	
	addLen = 0;
	chvNum = 0;

	dataBuf[0] = 0x62;

	num = 0x03;           //82tag

	dataBuf[num] = 0x82;

	dataBuf[num+1] = 0x02;

	ReadEeprom((BYTE xdata *)&dataBuf[num+2],G_CurrentDfAddr[G_ChannelNum]+F_Type_Offset,sizeof(BYTE));
	
	dataBuf[num+2] &= 0x7F;

	dataBuf[num + 3] = 0x21;

	num = num + 4 ;  //END OF File dESCRIPTOR, 83TAG

	dataBuf[num] = 0x83; 

	dataBuf[num + 1] = 0x02;

	dataBuf[num + 2] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;

	dataBuf[num + 3] = G_CurrentDfName[G_ChannelNum]&0xFF;

	num = num + 4 ;	//END OF File identifier  84TAG

	fileNameLgth = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum],F_DIRNum_offset);  //AID name  在2F00中的记录号
	
	if(fileNameLgth != 0x00)											  //IS ADF FILE
	{
		dataBuf[num] = 0x84;

		GetAIDNameInFile(fileNameLgth,&dataBuf[num+1]);	  //AID name:LV

		num = num + dataBuf[num+1]  + 2 ;	 //8A TAG
	}

	if(G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)					//Only for MF,modify at 2007-02-08
	{
		dataBuf[num] = 0xA5;
	
		dataBuf[num + 2] = 0x80;	   									//UICC fileCharacteristics
	
		dataBuf[num + 3] = 0x01;
	
		//dataBuf[num+4] = ((ATRDefine[4]&0x0F)<<4)|0x01;
		dataBuf[num+4] = 0x71;
#if RSP_EUICC     // modify by chentao20220819 87 is only for RSP
		dataBuf[num+1] = 0x06;

		dataBuf[num + 5] = 0x87; //Tag  11.1.1.4.6.8 Supported system commands

		dataBuf[num + 6] = 0x01;

		dataBuf[num + 7] = 0x01; //TERMINAL CAPABILITY is supported

		num = num + 0x08;
#else
		dataBuf[num+1] = 0x03;

		num = num + 0x05;
#endif

	}

	dataBuf[num] = 0x8A;     

	dataBuf[num + 1] = 0x01;

	dataBuf[num + 2] = 0x05;											//card cycle Status ??? CardStatus与LCSI冲突

	num = num + 3;														//END OF LIFE CYCLE STATUS INTEGER

#if UICC_AB8C_ARR_PRIVATE
	arrNum = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrNum_Offset);	
	
	if((arrNum!=ARR_RECNUM_ADM)&&(arrNum!=ARR_RECNUM_NEV)&&((arrNum & A_PRIVATEARR_REC_FLAG) == A_PRIVATEARR_REC_FLAG))
	{		
		GetPrivateArrABInfo(arrNum,&dataBuf[num]);
		num += (dataBuf[num+1]+2);
	}
	else
#endif	
	{
	dataBuf[num] = 0x8B;

	dataBuf[num + 1] = 0x03;

	dataBuf[num+2] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrName_Offset);

	dataBuf[num+2] =  ((dataBuf[num+2]&F_ARRNAME_6F)==F_ARRNAME_6F?0x6F:0x2F);

	dataBuf[num + 3] = 0x06;

	dataBuf[num+4] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrNum_Offset); 

	num = num + 0x05;												//END OF Referenced to expanded format
	}

	dataBuf[num] = 0xC6;	//PS_DO C6 tag

	dataBuf[num+2] = 0x90;

	dataBuf[num+3] = 0x01;

	dataBuf[num+4] = 0x00;
#if SE_ONLY  	//	 eUICC_Profile_Package_Interoperable_Format_Test_V_2_1_2 
	dataBuf[num+5] = 0x95;
	
	dataBuf[num+6] = 0x01;
	
	dataBuf[num+7] = 0x08;	
	addLen = 3;
#endif
	for(i= 0x00;i<0x04;i++)
	{
	  tag = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ChvStatus_Offset+i);

		if(tag == 0xFF)
		{
			break;
		}
		else
		{				
			fileNameLgth = ReadKey(tag,(BYTE*)&pinKey.KeyId);
		
			if(fileNameLgth == TRUE)
			{
				dataBuf[num+(5 + addLen)+(chvNum*3)] = 0x83;		
				dataBuf[num+(6 + addLen)+(chvNum*3)] = 0x01;
				dataBuf[num+(7 + addLen)+(chvNum*3)] = tag;
				
				
#if UICC__FUNCTION__UniversalPin
				//The usage qualifier data object indicated by tag '95' is mandatory for the universal PIN (see clause 9.4.1) and optional for other PINs.
				//set usage qualifier of universal PIN to "do not use"
				if((i==0)&&(tag==0x11))
				{
						if(UICC__SET_UNIVELSALPIN_USE)
						{						
						}
						else
						{
							dataBuf[num+7] = 0x00;	
						}
				}
#endif				
				if((pinKey.KeyStatus&0x01) == PIN_ENABLED)
				{
					dataBuf[num+4] = dataBuf[num+4] + (0x80 >> chvNum);
				}
				
				chvNum ++;
			}
			else
			{
				//break;
				continue;
			}
		}
	}

	dataBuf[num+1] = addLen+3*(chvNum+1);										//L

	num = num + dataBuf[num+1] + 2;

	num-=3;

	/*dataBuf[num] = 0x81;

	dataBuf[num+1] = 0x04;

	totalFiledSize = GetFileHeadUsedSize( G_CurrentDfAddr[G_ChannelNum]);

	totalFiledSize = totalFiledSize - sizeof(FileSystemHead) + GetHeapBase() - GetMaxProfileMFEnd();;

	if(sizeof(UWORD) == 0x04)
	{
		dataBuf[num +2] = (((unsigned long)totalFiledSize) >> 24)&0xFF;

		dataBuf[num +3] = (((unsigned long)totalFiledSize) >> 16)&0xFF;

		dataBuf[num +4] = (totalFiledSize >> 8)&0xFF;

		dataBuf[num +5] = totalFiledSize&0xFF;

		num = num + 2;
	}
	else
	{
		dataBuf[num+1] = 0x02;

	    dataBuf[num +2] = (totalFiledSize >> 8)&0xFF;

		dataBuf[num +3]	= totalFiledSize&0xFF;
	}*/
 	
	//num = num + 0x01;

	if(num > 0x80)
	{
		dataBuf[1] = 0x81;

		dataBuf[2] = num;

		num = num + 3;
	}
	else
	{
		dataBuf[1] = num;

		ecpMem_Copy((BYTE xdata *)&dataBuf[2],(BYTE xdata *)&dataBuf[3],num);

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
  history:           modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
 -------------------------------------------------------------------------------------------------------------------*/    
BYTE UsimGetEfInformation(BYTE *dataBuf)                                 
{
	BYTE idata num,fileType,sfi;
	WORD xdata fileSize,maxSize;
	UWORD xdata CurrentEfHeadAddr;
#if UICC_AB8C_ARR_PRIVATE
	BYTE  idata arrNum;
#endif
	dataBuf[0] = 0x62;

	num = 0x03;

	dataBuf[num] = 0x82;

	ReadEeprom((BYTE idata *)&fileType,G_CurrentEfAddr[G_ChannelNum]+F_Type_Offset,sizeof(BYTE));

	dataBuf[num+2] = (fileType&0x7F); 

	dataBuf[num + 3] = 0x21;									

	if(((dataBuf[num+2]&0x02)==0x02)||((dataBuf[num+2]&0x06)==0x06))			//记录文件
	{
		dataBuf[num+4] = 0x00;
		
		dataBuf[num+5] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecLgth_Offset);

		dataBuf[num+6] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecNum_Offset);		

		dataBuf[num+1] = 0x05;

		num = num + 3;
	}
	else
	{
		dataBuf[num+1] = 0x02;
	}
	
	num = num + 0x04 ;															//文件描述符

	dataBuf[num] = 0x83;

	dataBuf[num + 1] = 0x02;

	dataBuf[num + 2] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;

	dataBuf[num + 3] = G_CurrentEfName[G_ChannelNum]&0xFF;
	
	num = num + 0x04;															//文件标识符

#if UICC__FUNCTION__BERTLVCmd
	if((fileType&FILETYPE) == 0x39)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_UsedSize);

		maxSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);

		dataBuf[num] = 0xA5;

		dataBuf[num+1] = 0x0F;

		dataBuf[num+2] = 0x83;
	
		dataBuf[num+3] = 0x02;

		dataBuf[num+4] =  ((maxSize -fileSize)>>8)&0xFF;

		dataBuf[num+5] =  (maxSize -fileSize)&0xFF;

		dataBuf[num+6] = 0x84;
	
		dataBuf[num+7] = 0x01;

		dataBuf[num+8] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_Detail);
		
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_ReservedSize);

		dataBuf[num+9] = 0x85;
	
		dataBuf[num+10] = 0x02;

		dataBuf[num+11] =  (fileSize>>8)&0xFF;

		dataBuf[num+12] =  fileSize&0xFF;

		dataBuf[num+13] = 0x86;
	
		dataBuf[num+14] = 0x02;

		dataBuf[num+15] =  (maxSize>>8)&0xFF;

		dataBuf[num+16] =  maxSize&0xFF;

		num = num +17;
	
	}
#endif
	CurrentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);
	
	dataBuf[num] = 0x8A ;

	dataBuf[num + 1] = 0x01;

	dataBuf[num+2] = GetFileHeadByte(CurrentEfHeadAddr, F_Status_Offset);

	dataBuf[num+2] = dataBuf[num+2]&0x0F;

	num = num + 0x03;												//END OF Life cycle status integer

#if UICC_AB8C_ARR_PRIVATE
	arrNum = GetFileHeadByte(CurrentEfHeadAddr, F_ArrNum_Offset);	
	
	if((arrNum!=ARR_RECNUM_ADM)&&(arrNum!=ARR_RECNUM_NEV)&&((arrNum&A_PRIVATEARR_REC_FLAG)==A_PRIVATEARR_REC_FLAG))
	{
		GetPrivateArrABInfo(arrNum,&dataBuf[num]);
		num += (dataBuf[num+1]+2);
	}
	else
#endif
	{
		dataBuf[num] = 0x8B;

		dataBuf[num+1] = 0x03;

	 	dataBuf[num+2] = GetFileHeadByte(CurrentEfHeadAddr, F_ArrName_Offset);

		dataBuf[num+2] =  ((dataBuf[num+2]&F_ARRNAME_6F)==F_ARRNAME_6F?0x6F:0x2F);

		dataBuf[num+3] = 0x06;

		dataBuf[num+4] = GetFileHeadByte(CurrentEfHeadAddr, F_ArrNum_Offset);	

		num = num + 5;
	}

	dataBuf[num] = 0x80;

	dataBuf[num +1] = 0x02;
	
	
	if((fileType&FILETYPE) == TRANTYPE)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);
	}
#if UICC__FUNCTION__BERTLVCmd
	else if((fileType&FILETYPE) == 0x39)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_UsedSize);
	}
#endif
	else
	{
		ReadEeprom((BYTE xdata *)&dataBuf[num+2],G_CurrentEfAddr[G_ChannelNum] + F_RecNum_Offset,sizeof(WORD));
		
		fileSize = dataBuf[num+2]*dataBuf[num+3];
	}

	dataBuf[num + 2] = (fileSize>>8)&0xFF;

	dataBuf[num + 3] = fileSize&0xFF;

	num = num + 0x04;
	
	dataBuf[num] = 0x88;																//END OF FileSize

 	sfi = GetFileHeadByte(CurrentEfHeadAddr,F_ShordFid_Offset);

	if(sfi == 0x00)														//SFI,not supoprt
	{		
		dataBuf[num + 1] = 0x00;
		
		num = num + 2 ;			
	}
	else
	{
		dataBuf[num + 1] = 0x01;

		dataBuf[num + 2] = (sfi << 3) & 0xF8;

		num = num + 3 ;
	}

	num = num - 3;																	//End of SFI

	if(num > 0x80)
	{
		dataBuf[1] = 0x81;

		dataBuf[2] = num;

		num = num + 3;
	}
	else
	{
		dataBuf[1] = num;

		ecpMem_Copy((BYTE xdata *)&dataBuf[2],(BYTE xdata *)&dataBuf[3],num);

		num = num + 2 ; 
	} 

	return num;
 }
/*@Function:   SetChannel                                            
 * 
 *@Descrip:    选择文件后设置文件上下文环境，并拼装FCP
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      tagAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V1.0---Create by xyz
 *  		   V1.1---modify by chentao@20211112 选择ADF时给G_CurrentAdfName赋值，否则RunODDAuth函数会有false==false的情况
 *             V1.2----modify by xwh@20211125,V1.1 chentao修改代码与电信测试用例ManageChannel - 3 - 功能测试-PIN状态测试冲突，恢复，改成修改RunODDAuth函数
 *                        
 */
BYTE SetChannel(UWORD fileAddr,WORD fileName,BYTE option)
{
   	BYTE idata fileType;
	BYTE idata returnLgth;
	WORD idata fileNameTemp;
	UWORD xdata fileAddrBak;

	returnLgth = 0x00;    //初始化当前选中文件的响应值返回长度

   	if(fileAddr == 0)
   	{
   		return 0;
   	}

	if(fileName == 0x3F00)
	{
		fileType = ADFTYPE;	
		fileNameTemp = fileName;
	}
	else
	{
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);
		fileNameTemp = GetFileHeadWord(fileAddr,F_Name_Offset);	
	}	

   	if( fileType == ADFTYPE )					//DF  ADF  MF
  	{ 
		fileAddrBak = fileAddr;
		//V1.1 chentao修改代码与电信测试用例ManageChannel - 3 - 功能测试-PIN状态测试冲突，恢复
// 		result = GetFileHeadByte(fileAddr,F_DIRNum_offset); //add by chentao20211112选择ADF时给G_CurrentAdfName赋值

// 		if(result != 0x00)
// 		{
// 			G_CurrentAdfName[G_ChannelNum] = fileName;
// 		}

		if(fileNameTemp != fileName) //表示当前选的是DFlink
		{
			if(fileName != 0x7F21)
			{
				fileAddr = SearchFile(E_FILE_START,fileName);
			}
		}
		
		G_CurrentDfName[G_ChannelNum] = fileName;

		G_CurrentDfAddr[G_ChannelNum] = fileAddr;

		G_CurrentEfName[G_ChannelNum] = FALSE;

		G_CurrentEfAddr[G_ChannelNum] = FALSE;

        //ClearBit(G_SystemStatus,B_cmk);  //复位后校验MF key, 选择MF /DF后不再校验，不清除校验通过状态, by xwh 20180508

		

		if(option == TRUE)
		{
			if(G_CommandMode == Command_Mode_3G)
			{
    			returnLgth = UsimGetDfInformation(RecData);
			}
			else
			{
				returnLgth = GetDfInformation(RecData);
			}
		}

		G_CurrentDfAddr[G_ChannelNum] = fileAddrBak;
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
			if(G_CommandMode == Command_Mode_3G)
			{
   			  	returnLgth = UsimGetEfInformation(RecData);
			}
			else
			{
				returnLgth = GetEfInformation(RecData);	
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
#if 0
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 BaseUsimGetDfInformation                                                                                                                   
                                                                                                                                                
  input parameter:   dataBuf:输出数据缓冲区
  input parameter:   mode:isforAPI or isForPhase                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the df information to the gsm buffer
  
  status:			 OK         
  history:           modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
 -------------------------------------------------------------------------------------------------------------------*/    

BYTE BaseUsimGetDfInformation(BYTE *dataBuf)
{
	//UWORD xdata totalFiledSize;						//sum of all the EFs and DFs contained in this DF plus the amount of available memory in this DF
    BYTE idata num,fileNameLgth,i;
	KeyStructer xdata   pinKey;						
#if UICC_AB8C_ARR_PRIVATE
	BYTE  idata arrNum;
#endif

	dataBuf[0] = 0x62;

	num = 0x03;  

	dataBuf[num] = 0x82;

	dataBuf[num+1] = 0x02;

	ReadEeprom((BYTE xdata *)&dataBuf[num+2],G_CurrentDfAddr[G_ChannelNum]+F_Type_Offset,sizeof(BYTE));
	
	dataBuf[num+2] &= 0x7F;

	dataBuf[num + 3] = 0x21;

	num = num + 4 ;                    										//END OF File dESCRIPTOR

	dataBuf[num] = 0x83;

	dataBuf[num + 1] = 0x02;

	dataBuf[num + 2] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;

	dataBuf[num + 3] = G_CurrentDfName[G_ChannelNum]&0xFF;

	num = num + 4 ;														   //END OF File identifier

	fileNameLgth = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum],F_DIRNum_offset);  //AID name  在2F00中的记录号
	
	if(fileNameLgth != 0x00)											  //IS ADF FILE
	{
		dataBuf[num] = 0x84;

		GetAIDNameInFile(fileNameLgth,&dataBuf[num+1]);	  //AID name:LV

		num = num + dataBuf[num+1]  + 2 ;	
	}

	if(G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)					//Only for MF,modify at 2007-02-08
	{
		dataBuf[num] = 0xA5;
	
		dataBuf[num+1] = 0x03;
	
		dataBuf[num + 2] = 0x80;	   									//UICC fileCharacteristics
	
		dataBuf[num + 3] = 0x01;
	
		//RecData[num+4] = 0x71;											//0x31:允许时钟停止，无优先级别，支持电压级别A、B、C
		//dataBuf[num+4] = ((ATRDefine[4]&0x0F)<<4)|0x01;
		dataBuf[num+4] = 0x71;
								
		num = num + 0x05;
	}

	dataBuf[num] = 0x8A;

	dataBuf[num + 1] = 0x01;

	dataBuf[num + 2] = 0x05;											//card cycle Status ??? CardStatus与LCSI冲突

	num = num + 3;														//END OF LIFE CYCLE STATUS INTEGER
#if UICC_AB8C_ARR_PRIVATE
	arrNum = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrNum_Offset);	
	
	if((arrNum!=ARR_RECNUM_ADM)&&(arrNum!=ARR_RECNUM_NEV)&&(arrNum & A_PRIVATEARR_REC_FLAG) == A_PRIVATEARR_REC_FLAG)
	{
		GetPrivateArrABInfo(arrNum,&dataBuf[num]);
		num += (dataBuf[num+1]+2);
	}
	else
#endif
	{
		dataBuf[num] = 0x8B;

		dataBuf[num + 1] = 0x03;

	//	dataBuf[num + 2] = 0x2F;
		dataBuf[num+2] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrName_Offset);

		dataBuf[num+2] =  ((dataBuf[num+2]&F_ARRNAME_6F)==F_ARRNAME_6F?0x6F:0x2F);

		dataBuf[num + 3] = 0x06;

		dataBuf[num+4] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ArrNum_Offset); 

		num = num + 0x05;												//END OF Referenced to expanded format
	}

	dataBuf[num] = 0xC6;											//PS_DO

	dataBuf[num+2] = 0x90;

	dataBuf[num+3] = 0x01;

	dataBuf[num+4] = 0x00;

	for(i= 0x00;i<0x04;i++)
	{
	   	dataBuf[num+7+(i*3)] = GetFileHeadByte(G_CurrentDfAddr[G_ChannelNum], F_ChvStatus_Offset+i);

		if(dataBuf[num+7+(i*3)] == 0xFF)
		{
			break;
		}
		else
		{
			dataBuf[num+5+(i*3)] = 0x83;
		
			dataBuf[num+6+(i*3)] = 0x01;
		
			fileNameLgth = ReadKey(dataBuf[num+7+(i*3)],(BYTE*)&pinKey.KeyId);
		
			if(fileNameLgth == TRUE)
			{
				if((pinKey.KeyStatus&0x01) == PIN_ENABLED)
				{
					dataBuf[num+4] = dataBuf[num+4] + (0x80 >> i);
				}
			}
			else
			{
				break;
			}
		}
	}

	dataBuf[num+1] = 3*(i+1);										//L

	num = num + dataBuf[num+1] + 2;

	num-=3;

	if(num > 0x80)
	{
		dataBuf[1] = 0x81;

		dataBuf[2] = num;

		num = num + 3;
	}
	else
	{
		dataBuf[1] = num;

		ecpMem_Copy((BYTE xdata *)&dataBuf[2],(BYTE xdata *)&dataBuf[3],num);

		num = num + 2 ; 
	} 

	return num;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 BaseUsimGetEfInformation                                                                                                                   
                                                                                                                                                
  input parameter:   dataBuf:输出数据缓冲区
  input parameter:   mode:isforAPI or isForPhase                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       set the ef information to the gsm buffer
  
  status:			 OK         
  history:           modify by xuweihua@20211106 AB/8C规则存储在私有文件2700/AF06，增加宏开关UICC_AB8C_ARR_PRIVATE
 -------------------------------------------------------------------------------------------------------------------*/    
BYTE BaseUsimGetEfInformation(BYTE *dataBuf)                                 
{
	BYTE idata num,fileType,sfi;
	WORD xdata fileSize,maxSize;
	UWORD xdata CurrentEfHeadAddr;
#if UICC_AB8C_ARR_PRIVATE
	BYTE  idata arrNum;
#endif	

	dataBuf[0] = 0x62;

	num = 0x03;

	dataBuf[num] = 0x82;

	ReadEeprom((BYTE idata *)&fileType,G_CurrentEfAddr[G_ChannelNum]+F_Type_Offset,sizeof(BYTE));

	dataBuf[num+2] = (fileType&0x7F); 

	dataBuf[num + 3] = 0x21;									

	if(((dataBuf[num+2]&0x02)==0x02)||((dataBuf[num+2]&0x06)==0x06))			//记录文件
	{
		dataBuf[num+4] = 0x00;
		
		dataBuf[num+5] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecLgth_Offset);

		dataBuf[num+6] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_RecNum_Offset);		

		dataBuf[num+1] = 0x05;

		num = num + 3;
	}
	else
	{
		dataBuf[num+1] = 0x02;
	}
	
	num = num + 0x04 ;															//文件描述符

	dataBuf[num] = 0x83;

	dataBuf[num + 1] = 0x02;

	dataBuf[num + 2] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;

	dataBuf[num + 3] = G_CurrentEfName[G_ChannelNum]&0xFF;
	
	num = num + 0x04;															//文件标识符

#if UICC__FUNCTION__BERTLVCmd
	if((fileType&FILETYPE) == 0x39)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_UsedSize);

		maxSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);

		dataBuf[num] = 0xA5;

		dataBuf[num+1] = 0x0F;

		dataBuf[num+2] = 0x83;
	
		dataBuf[num+3] = 0x02;

		dataBuf[num+4] =  ((maxSize -fileSize)>>8)&0xFF;

		dataBuf[num+5] =  (maxSize -fileSize)&0xFF;

		dataBuf[num+6] = 0x84;
	
		dataBuf[num+7] = 0x01;

		dataBuf[num+8] = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_Detail);
		
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_ReservedSize);

		dataBuf[num+9] = 0x85;
	
		dataBuf[num+10] = 0x02;

		dataBuf[num+11] =  (fileSize>>8)&0xFF;

		dataBuf[num+12] =  fileSize&0xFF;

		dataBuf[num+13] = 0x86;
	
		dataBuf[num+14] = 0x02;

		dataBuf[num+15] =  (maxSize>>8)&0xFF;

		dataBuf[num+16] =  maxSize&0xFF;

		num = num +17;
	
	}
#endif
	
	CurrentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);
	
	dataBuf[num] = 0x8A ;

	dataBuf[num + 1] = 0x01;

	dataBuf[num+2] = GetFileHeadByte(CurrentEfHeadAddr, F_Status_Offset);

	dataBuf[num+2] = dataBuf[num+2]&0x0F;

	num = num + 0x03;												//END OF Life cycle status integer
#if UICC_AB8C_ARR_PRIVATE
	arrNum = GetFileHeadByte(CurrentEfHeadAddr, F_ArrNum_Offset);	
	
	if((arrNum!=ARR_RECNUM_ADM)&&(arrNum!=ARR_RECNUM_NEV)&&((arrNum & A_PRIVATEARR_REC_FLAG) == A_PRIVATEARR_REC_FLAG))
	{
		GetPrivateArrABInfo(arrNum,&dataBuf[num]);
		num += (dataBuf[num+1]+2);
	}
	else
#endif
	{
		dataBuf[num] = 0x8B;

		dataBuf[num+1] = 0x03;

	 	dataBuf[num+2] = GetFileHeadByte(CurrentEfHeadAddr, F_ArrName_Offset);

		dataBuf[num+2] =  ((dataBuf[num+2]&F_ARRNAME_6F)==F_ARRNAME_6F?0x6F:0x2F);

		dataBuf[num+3] = 0x06;

		dataBuf[num+4] = GetFileHeadByte(CurrentEfHeadAddr, F_ArrNum_Offset);	

		num = num + 5;
	}

	dataBuf[num] = 0x80;

	dataBuf[num +1] = 0x02;
	
	
	if((fileType&FILETYPE) == TRANTYPE)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_SpaceSize_Offset);
	}
#if UICC__FUNCTION__BERTLVCmd
	else if((fileType&FILETYPE) == 0x39)
	{
		fileSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum], F_BERTLV_UsedSize);
	}
#endif
	else
	{
		ReadEeprom((BYTE xdata *)&dataBuf[num+2],G_CurrentEfAddr[G_ChannelNum] + F_RecNum_Offset,sizeof(WORD));
		
		fileSize = dataBuf[num+2]*dataBuf[num+3];
	}

	dataBuf[num + 2] = (fileSize>>8)&0xFF;

	dataBuf[num + 3] = fileSize&0xFF;

	num = num + 0x04;
	
	dataBuf[num] = 0x88;																//END OF FileSize

 /*
	if(G_CurrentSfi == 0x00)														//SFI,not supoprt
	{		
		dataBuf[num + 1] = 0x00;
		
		num = num + 2 ;			
	}
	else
	{
		dataBuf[num + 1] = 0x01;

		dataBuf[num + 2] = (G_CurrentSfi << 3) & 0xF8;

		num = num + 3 ;
	}
 */
 	sfi = GetFileHeadByte(CurrentEfHeadAddr,F_ShordFid_Offset);

	if(sfi == 0x00)														//SFI,not supoprt
	{		
		dataBuf[num + 1] = 0x00;
		
		num = num + 2 ;			
	}
	else
	{
		dataBuf[num + 1] = 0x01;

		dataBuf[num + 2] = (sfi << 3) & 0xF8;

		num = num + 3 ;
	}

	num = num - 3;																	//End of SFI

	if(num > 0x80)
	{
		dataBuf[1] = 0x81;

		dataBuf[2] = num;

		num = num + 3;
	}
	else
	{
		dataBuf[1] = num;

		ecpMem_Copy((BYTE xdata *)&dataBuf[2],(BYTE xdata *)&dataBuf[3],num);

		num = num + 2 ; 
	} 

	return num;
 }
#endif
#if UICC__OPERATOR__CHINAUNICOM
 
 /**
 *@brief     1.7FF0/6F61都不可以写入6F7B文件中 2.如果写入的值与6F7B对应位置内容相同，丢弃更新的值  
 *
 *@param       fileName: 7ff0/6F7b
 *
 *@wuthor      zhxx
 *
 *@history     20201014
*/
 BYTE Update6F7BFile(void)
 {
	UWORD idata fileAddrOPLMNwAcT;
	WORD  idata fileSize,plmnfileSize;
	BYTE  idata fPlmn[3];
	WORD  idata i,j;
	BYTE  idata tag;
	 
	 if((G_CurrentEfName[G_ChannelNum] == 0x6F7B)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7FF0)))
	 {
		plmnfileSize = GetTFSize(G_CurrentEfAddr[G_ChannelNum]);

		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],0,(BYTE *)&RecData[0x50],plmnfileSize) ;
		 
		fileAddrOPLMNwAcT = SelectTFFile(0x7FF0, EF_OPLMNwAcT);  

		if(fileAddrOPLMNwAcT == FALSE)
		{
			return FALSE;
		}
		 
		fileSize = GetTFSize(fileAddrOPLMNwAcT);
		 
		if((P3 % 3) != 0)//如果更新的长度不是3的倍数，需要将后续数据填充为0xFF
		{
			ecpMem_Set(&RecData[P3],0xFF,P3-(P3 % 3));
		}
		 
		for(i=0; i<P3; i=i+3)//循环遍历6F7B文件
		{
			tag = TRUE;
			
			for(j=0;j<fileSize;j=j+5)                                   // fileSize should be less then 0x100，循环遍历6F61文件
			{
				ReadTFBuf(fileAddrOPLMNwAcT,j,&fPlmn[0],0x03);
				
				if((fPlmn[0] != 0xFF)||(fPlmn[1] != 0xFF)||(fPlmn[2] != 0xFF)) //modify by xwh
				{
					if((RecData[i]== fPlmn[0])&&(RecData[i+1]==fPlmn[1])&&(RecData[i+2]==fPlmn[2]))
					{
						return FALSE;	            			
					}
				}	
			}
			
			if(tag == TRUE)
			{
				ecpMem_Copy(&RecData[0x50+P2+i],&RecData[i],((P3-i) >= 3) ? 3 : P3-i);//最后一次更新的长度有可能不是3
			}				
		}
		
		WriteTFBuf(G_CurrentEfAddr[G_ChannelNum],0,(BYTE *)&RecData[0x50],plmnfileSize) ;
		
		return FALSE;
		 		 
	 }
	 
	 return TRUE;
 }
 
 #endif
 
#if (SE_ONLY && UICC__OPERATOR__CHINAMOBILE)
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

#if (!UICC__GSM_2G_NOT_SUPPORT)
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7B)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)||(G_CurrentDfName[G_ChannelNum] == 0x7FF0)))	//134号段
#else
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7B)&&(G_CurrentDfName[G_ChannelNum] == 0x7FF0))	//134号段
#endif		
    {
		plmnfileSize = GetTFSize(G_CurrentEfAddr[G_ChannelNum]);

		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],0,(BYTE *)&RecData[0x50],plmnfileSize) ;

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
	          		ecpMem_Copy(&RecData[0x50+P2+i],&RecData[i],3);
	          	}				
    		}						
		}
		else
		{
			ecpMem_Copy((BYTE *)&RecData[0x50+P2],(BYTE *)&RecData[0],P3);
		}

		WriteTFBuf(G_CurrentEfAddr[G_ChannelNum],0,(BYTE *)&RecData[0x50],plmnfileSize) ;
		
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

BYTE Funtion134Write(void)
{
	return Update6F7BFile(0xFF03);
}




/**
 * @brief 		SmartLumingWrite() 在WRITE BINIARY中处理优选漫游的函数
 * 
 * @author 		XYZ,XWH 
 * 
 * @history 	2009-05-11,根据优选漫游规范修改,"当终端一次向6F7B文件写入多条网络信息（MCC+MNC）时，SIM卡应在滤除出现在6F30文件中的网络信息后，将剩余的信息（如果有剩余信息）写入到6F7B文件中". 
 */
BYTE SmartLumingWrite(void)
{
#if (!UICC__GSM_2G_NOT_SUPPORT)	
   	if((G_CurrentEfName[G_ChannelNum] == EF_PLMNsel)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21))) //不允许本地更新6F30,2008-03-07
    {
		return FALSE;
	}
#endif	
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

#if (!UICC__GSM_2G_NOT_SUPPORT)
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7E)&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)||(G_CurrentDfName[G_ChannelNum] == 0x7FF0))) //EF_LOCI,2008-03-07
#else
	if((G_CurrentEfName[G_ChannelNum] == 0x6F7E)&&( G_CurrentDfName[G_ChannelNum] == 0x7FF0)) //EF_LOCI,2008-03-07
#endif
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],4, &LAI_6F7E[0], 3);              				// MCC,MNC

		fileAddr = SelectTFFile(DF_PHASE,EF_IMSI);

		ReadTFBuf(fileAddr, 1, &LAI_Tmp[0], 3);

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

		fileLgth = (((WORD)P1 << 8)&0xFF00 )|(P2 & 0x00FF); //根据白盒测试结果，增加强制转化，避免移位时溢出
	
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],P3);
	}
}
#endif
 
/*@Function:    CheckCurrentDfForADF                                           
 * 
 *@Descrip:    判断当前DF是不是ADF或者ADF下的DF
 *
 *                                                                                                                                     
 *@param        :void
 *				                                                                                                         
 *@Return:      TRUE(ADF)/FALSE(not ADF)                                                                                                                                                                                                                                                           
 * 
 *@Author:      LCZ 2019-7-3
 *  
 *@History:     V0.0---Create by LCZ 2019-7-3            
 */
 
UWORD CheckCurrentDfForADF()
{
	BYTE idata fileType,i;
	
	WORD xdata fileName[2];
	
	UWORD xdata fileAddr,DFfileAddr;
	
	DFfileAddr = G_CurrentDfAddr[G_ChannelNum];  //G_CurrentDfAddr[G_ChannelNum] 没有FALSE的机会
	
	fileType = GetFileHeadByte(DFfileAddr,F_DIRNum_offset);
	
	if(fileType != 0x00)//当前就是ADF  
	{
		return DFfileAddr;
	}
	
	if((DFfileAddr == E_FILE_START) || (G_CurrentAdfName[G_ChannelNum] == FALSE))
	{
			return FALSE;
	}
		
	
	fileAddr = E_FILE_START;//只支持3F00/7F20/5F3A/4F3A层次
	
	do{	
			ReadEeprom((BYTE *)&fileName[0],DFfileAddr + F_ParentName_Offset,sizeof(fileName));

			for(i = 0x00;i<2;i++)
			{
				fileAddr = SearchFile(fileAddr,fileName[i]);
				
				if(fileAddr != FALSE)
				{
					fileType = GetFileHeadByte(fileAddr,F_DIRNum_offset);
					
					if(fileType != 0x00)
					{
						return fileAddr;
					}
					
				}
				else
				{
					return FALSE;
				}
				
				if((i == 0x01)&&(fileAddr == E_FILE_START)) // 
				{
					return FALSE;
				}
			}
			
			DFfileAddr = fileAddr; //备份DF地址 往上查找  当前DF的上一级DF是fileName[1]
			
	}while((fileName[1] != DF_MF)&&(fileName[0] != DF_MF));  //找到MF结束
	
	return FALSE;
}

