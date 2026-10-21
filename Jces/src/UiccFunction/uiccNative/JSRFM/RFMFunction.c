/*-------------------------------------------------------------------------------------------------------//
  Copyright (c) 2003,eastcompeace

  All rights reserved.

  MODEL:EMOSYN TG96-3G

  MOTOCOS V4.10

  PHASE 2 + STK(WIB)  RELEASE:2003-2-25
  
  V1.0   创建源文件

 --------------------------------------------------------------------------------------------------------*/

#include  "typedef.h"
#include  "main.h"
#include  "fileAction.h"
#include  "string.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "error.h"
#include  "rfm.h"
#include  "Authentication.h"
#include  "fileSystem.h"
#include  "resume.h"
#include  "hard_api.h"
#include  "arithmatic.h"
#include  "interface.h"
#include  "Transaction.h"


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 SaveTPOAtoFile                                                                                                                   
                                                                                                                                                
  input parameter:   smscBuf: SMSC,LV
                     tpoaBuf: TPOA,LV                                                                  
  					                                                                                                         
  output parameter:  void                                                                                                                
                                                                                                                                                
  description:       把下行数据的SMSC和TPOA的内容保存在文件中，结构LV
  
  author: 			 XuWeiHua

  status: 			 need test

  status:			 OK
---------------------------------------------------------------------------------------------------------------------------------------------------*/

BYTE SaveTPOAtoFile(BYTE * smscBuf,BYTE * tpoaBuf)
{
	UWORD idata fileAddr;

	fileAddr = SelectLFFile(DF_RFM, EF_RFM_ScaTpoa);	 //	   EF_SCA_AND_TPOA 记录文件，记录长度：24；一条记录的结构：sca(12字节 LV结构) tpoa(12字节LV结构)

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	if(smscBuf != NULL)
	{			
		WriteLFBuf(fileAddr, G_TempTarNo, 0x00, &smscBuf[0], smscBuf[0]+1);		
	}
	else
	{
		SetLFConst(fileAddr, G_TempTarNo, 0x00, 0x00, 1);
	}
	
	WriteLFBuf(fileAddr, G_TempTarNo, 12, &tpoaBuf[0], (tpoaBuf[0]/2+tpoaBuf[0]%2+2));
	
	return TRUE;
}



BYTE CheckTar(BYTE * tarPointer)
{
	UWORD idata fileAddr;
    BYTE idata k,recNum;
	BYTE  idata i,num;
    BYTE xdata tem[3];
	BYTE xdata tar[5];
	
    fileAddr = SelectLFFile(DF_RFM,EF_USIMTAR);									 

	if(fileAddr!=FALSE)
	{
		recNum = GetLFRecNumLgth(fileAddr,FileNum);

		for(k=1;k<=recNum;k++)
		{
			ReadLFBuf(fileAddr,k,0,(BYTE *)&tem[0],3);
			
			if(memcmp(&tarPointer[0],(BYTE *)&tem[0],3) == EQUAL)
			{
				G_TarTypeMode = USIMMODE;											
				break;	
			}			
		}				
	}
	
	fileAddr = SelectLFFile(DF_RFM,EF_RFM_Tar);
		
	if(fileAddr != FALSE)
	{
		num = GetLFRecNumLgth(fileAddr,FileNum);

		for(i=1;i<=num;i++)
		{
			ReadLFBuf(fileAddr,i,0,(BYTE *)&tar[0],sizeof(tar));
		
			if(memcmp(&tarPointer[0],(BYTE *)&tar[1],0x03) == EQUAL)
			{
				G_TempTarNo = i;

				G_TempSc = tar[4];					
	
				return RFM_TAR;
				
			}
		}	
	}		


	if((tarPointer[0] == 0x00)&&(tarPointer[1] == 0x00)&&(tarPointer[2]==0x00))  //compare tar value with the tar file		
	{
		G_TempTarNo = 1;

		G_TempSc = 1;
		
		return RFM_TAR;
	}

    G_TempTarNo = 1;							

	G_TempSc = 1;
	
	return ErrorTAR ;
}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 STKUpdateRecord
  
  Command format:    A0    DC   XX   XX   XX
  
  Data    content:   the concent of The file                                                       
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       This function update a string of bytes from the current Liner EF

  status:			 Code OK!     Test OK!    

 -------------------------------------------------------------------------------------------------------------------*/
void STKUpdateRecord(void)
{
	BYTE idata recordNo,result;
	BYTE idata tag,fileType,need_commit;

	recordNo = ComputeRecord(A_UpdateFile);

	if(recordNo == FALSE )
  {
		return ;
	}


	if(P2 != ABSOLUTE_CURRENT)
	{
		G_CurrentRecNum[G_ChannelNum] = recordNo;                                                               //after a successful read,the current pointer must change to the new position;
	}

	result = ReceiveData();

	if(result == TRUE)
	{
		need_commit = IF_BeginTransaction();
		
		tag = WriteLFBuf(G_CurrentEfAddr[G_ChannelNum],recordNo,0,(BYTE  *)&RecData[0],P3);
	
		fileType = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);
	
		if((P2 != ABSOLUTE_CURRENT) && ((fileType & 0x07) == CYCTYPE))
		{	
			tag = tag & 	SetEepromEX(getFileContentAddr(G_CurrentEfAddr[G_ChannelNum]),recordNo,1);
			
			if(tag == FALSE)
			{
				MemoryError();													//65 81/92 40
			
				if(need_commit == TRUE)
				{
					TransactionAbort();
				}	
		
				return;
			}
    }		
		
    TransactionCommit(need_commit); 
		
		
		/*
   	if(ValueBit(G_SystemStatus, B_RfmPerform) == TRUE)
		{				
			if((G_CurrentDfName[G_ChannelNum] == 0x2700)&&((G_CurrentEfName[G_ChannelNum] == 0x6F18)||(G_CurrentEfName[G_ChannelNum] == 0x6F1E)))		//2700/6F1E
			{
				PendingMessageAlert(D_SetUpMenuTag, 0x00);
			}		
		}	
		*/
	}	

	CommandFinish(0x00);               
}


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 WIBRFMHandlerD0                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                   
  					                                                                                                         
  output parameter:  void                                                                                                                
                                                                                                                                                
  description:       下行RFM数据短信的主动式命令的执行

  Author: 			 zhuli

  status:			 OK
---------------------------------------------------------------------------------------------------------------------------------------------------*/

void RFMHandlerD0(void)
{
	BYTE  idata ch,buf[2];
	UWORD xdata dataAddr;
	WORD  xdata usedSpace;
	

	dataAddr = SelectTFFile(DF_STK, EF_WIBTempFile);

	if(dataAddr == FALSE)
	{
		return ;
	}

		//当芯片为小端，修正错误
	//ReadTFBuf(dataAddr, 0, (BYTE *)&usedSpace, sizeof(WORD));
	/*总是以大端法存储*/
	ReadTFBuf(dataAddr, 0, &buf[0], sizeof(WORD));
	usedSpace = BuildWORD(&buf[0]);

	ReadTFBuf(dataAddr,CommandStartNum,&ch,1);

	while((ch == 0xD0)&&(CommandStartNum < usedSpace))
	{
		ResData[0] = 0xD0;

		ReadTFBuf(dataAddr, CommandStartNum, &ResData[1], 0xFF);

		if(ResData[1] != 0xD0)
		{
			break;
		}

		if(ResData[2] > 0x80)                   //in this case,len is "818X" or "8X"?
		{
			memmove(&ResData[2],&ResData[3],ResData[3]+1);

			CommandStartNum = CommandStartNum + ResData[3] + 3;
		}
		else
		{
			CommandStartNum = CommandStartNum + ResData[2] + 2;
		}
	
		SendProactiveCommand();
	}

	SetTFConst(dataAddr, 0x00, 0x00, 0x10);

	CommandStartNum = 0x00;
}


/*-------------------------------------------------------------------------------------------------------//

  Function name:	ExcuteCommand
 
  Input parameter:	commandbuffer:    that is store the command string

 					commandlength:    that indicator the command's length

 					
 Output parameter:  void
 
 Description:       

 -------------------------------------------------------------------------------------------------------------------*/
static code INSLIST rfm_list[]= 
{ 
  {0xA4,SelectCommand},
  {0xD6,UpdateBinary},
  {0xDC,STKUpdateRecord},
  {0xA2,SeekCommand},
  {0x32,IncreaseCommand},
  {0x20,VerifyChangeUnblockChv},
  {0x04,FileStatusHandler},
  {0x44,FileStatusHandler},
  {0xB0,ReadBinary},
  {0xB2,ReadRecord}, 
  {0xC0,GetResponseCommand},
  {EOL,InsError},
};

BYTE RFMExcuteCommand(void)    //here,commandbuffer is the same with global var ResData[]
{
  	UWORD xdata RFMDFAddr,RFMEFAddr,fileAddr;
  	WORD  xdata RFMDFName,RFMEFName,cpLgth,num;
  	BYTE  xdata RFMCurrentRecord;

	RFMDFAddr = G_CurrentDfAddr[G_ChannelNum];                        //a data copy

  	RFMEFAddr = G_CurrentEfAddr[G_ChannelNum];

  	RFMDFName = G_CurrentDfName[G_ChannelNum];

  	RFMEFName = G_CurrentEfName[G_ChannelNum];

  	RFMCurrentRecord = G_CurrentRecNum[G_ChannelNum];

	SetBit(G_SystemStatus,B_RfmPerform);

	G_CurrentComNUM = 0;
	
	num = 0;

	fileAddr = SelectTFFile(DF_STK, EF_WIBTempFile);	//not inclue the file head 

	if(fileAddr == FALSE)
	{
		return FALSE;
	}
											
	//如果为小端修正错误										
	//ReadTFBuf(fileAddr, 0, (BYTE *)&cpLgth, sizeof(WORD));	//FileStartNum是文件的偏移。去掉MAC后，文件内容向后偏移4个字节。
	/*总是以大端法存储*/
	ReadTFBuf(fileAddr, 0, &RecData[0], sizeof(WORD));
	cpLgth = BuildWORD(&RecData[0]);

	SW1 = 0x90;

	/*When the Command Packet security has been verified successfully the ?°command 
	session?± begins. The current RFM file pointer shall then implicitly have selected 
	the MF directory. DT??GSM ????7F10->5F3A->4F3Aoó￡?RFM A0A40000027F20A0A40000026F07ê§°üμ??êìa---BY XWH 20121011*/
		
	if(G_TarTypeMode == USIMMODE)	
	{
		G_CurrentDfName[G_ChannelNum] = DF_ADF;	
		G_CurrentAdfName[G_ChannelNum] = DF_ADF;
		G_CurrentDfAddr[G_ChannelNum] = SelectFile(E_FILE_START, G_CurrentAdfName[G_ChannelNum]); 	
	}
	else
	{
		G_CurrentDfAddr[G_ChannelNum] = E_FILE_START;  
  		G_CurrentDfName[G_ChannelNum] = DF_MF;	
	}
	
	while((CommandStartNum < (cpLgth+2)) && ((SW1 == 0x90)||(SW1 == 0x9F)||(SW1 == 0x61)||((SW1 == 0x62)&&(SW2 == 0x83))) )		//select无效文件时返回6283
  	{
		SetBit(G_COMMANDPATH,B_CMDINIT); 	
		
		ReadTFBuf(fileAddr,CommandStartNum,(BYTE *)&G_APDU[0],0x05);

		if(G_APDU[0] == 0xD0)		                                            //主动式命令
		{
		    PendingMessageAlert(D_RFMProactiveCommandTag,0x00);
			if(SW1 == 0x90)
			{
				SW1 = 0x90;																								//此情况设定状态字的默认值(修正测试中发现的902E问题)
				SW2 = 0x00;
			}
			break;
		} 

		if (INS == 0xC0)		//GetResponse 特殊处理
	 	{
			if(P3 == 0x00)				//若为00C0000000指令,直接取出全部响应
			{
				P3 = SW2;
			}
			
			if(P3<=SW2)					//所取数据不能大于返回数据长度
			{
				GetResponseCommand();
				G_CurrentComNUM = G_CurrentComNUM + 1;
			}
		
		   	break;
	 	}
		
		if((INS==0xB2)&&(P3==0x00))  //当P3 = 00时，读取整条数据长度
		{
			if(G_CurrentEfAddr[G_ChannelNum] != FALSE)
			{
				 P3 = GetLFRecNumLgth(G_CurrentEfAddr[G_ChannelNum],FileLgth);	
			}
		}
		
		CommandStartNum = CommandStartNum + 5 ;

		G_CurrentComNUM = G_CurrentComNUM + 1;

		if((CommandStartNum+P3)>(cpLgth+2)&&((INS == 0xD6)||(INS == 0xDC)))
		{
			P3Error();

			break;
		}

		ReadTFBuf(fileAddr,CommandStartNum,(BYTE *)&RecData[0],P3);
		
		if((INS != 0xB0)&&(INS != 0xB2))
		{
			CommandStartNum = CommandStartNum + P3;
		}

		G_ChannelNum = CLA&0x03;

		if (((CLA&0xFC) != 0x80)&&((CLA&0xF0) != 0x00)&&(CLA != 0xA0))
		{
			ClaError();

			continue;
		}

		if (G_CommandMode == Command_Mode_3G)
		{
			if(ValueBit(G_ChannelStatus,G_ChannelNum) == FALSE)
			{
				ChannelNotSupport();							//通道不支持

				continue;
			}
		}

			
	
		for(num=0;;num++)
        { 
	 		if((rfm_list[num].ins == INS)||(rfm_list[num].ins == EOL))
 	        {				
				if(rfm_list[num].ins == EOL)  //INS 非法
	        	{
	        		InsError();      		
	        		break;
	        	}
				
				rfm_list[num].func();

                break;
			}                           												//perform the command now!
        }      
		
		if ((INS == 0xB0) || (INS == 0xB2) || (INS == 0xC0))			//case2	
		{		
			if (CommandStartNum != (cpLgth+2))	//case2不在最后一条，直接退出不执行
			{
				break;
			}
		} 

    }

	ClearBit(G_SystemStatus,B_RfmPerform);
                                 
  	G_CurrentDfAddr[G_ChannelNum] = RFMDFAddr  ;                    //get the data of stored before

  	G_CurrentEfAddr[G_ChannelNum] = RFMEFAddr  ;

  	G_CurrentDfName[G_ChannelNum] = RFMDFName  ;

  	G_CurrentEfName[G_ChannelNum] = RFMEFName  ;

  	G_CurrentRecNum[G_ChannelNum] = RFMCurrentRecord ;

	G_CurrentReponse[G_ChannelNum] = 0x00;

    return POP_OK;

}


/*----------------------------------------------------------------------------------------------------------                          

 Function name :	ResponsePack                                                                                                                   
                                                                                                                                                
 Input parameter:   errorCode

										                                                                                                                                               
 Output parameter:  TRUE
 					FALSE                                                                                                              
                                                                                                                                               
 Description:       
 
 status:			need Test
 
 author:            XuWeiHua	        
  ------------------------------------------------------------------------------------------------------------------------------*/                   
 
void ResponsePack(BYTE errorCode)				                 //result[0] is modified to ResData[0],for emulate
{   
	UWORD idata dataAddr,tpoaFileAddr;
	BYTE  idata responseLgth;
    BYTE  idata tar[3],spiLow;
	BYTE  idata i,resultStatus;	
	BYTE  idata status,offset;
	BYTE  idata recNum;

	dataAddr = SelectTFFile(DF_STK, EF_WIBTempFile);

	if(dataAddr == FALSE)
	{
		return;
	}

	ReadTFBuf(dataAddr, 0x04, (BYTE *)&spiLow, sizeof(BYTE));	                                    //Read SPI1,SPI2,KIC,KID	                        

	if(((spiLow & 0x03 ) == 0x03)||((spiLow & 0x03 ) == 0x00))	        //no POR reply or reserved
	{  
		return;
	}
	
	if((errorCode == 0x00) && ((spiLow & 0x03 ) == 0x02) && ((SW1 == 0x90) || (SW1 == 0x9F) || (SW1 == 0x61)))	//POR required only when an error has occured
	{  
		return ;
	}

	ResData[0] = 0x02 ;     													//UDHL

	ResData[1] = 0x71 ;     													//UDH_high

	ResData[2] = 0x00 ;     													//UDH_low

	ReadTFBuf(dataAddr, 0x07, &ResData[6], 8);	                //Read tar and counter

	if((errorCode!=0x00)&&(errorCode!=ERROR_CNTR_LOW)&&(errorCode!=ERROR_CNTR_HIGH)&&(errorCode!=ERROR_CNTR_BLCED))	   	//这些情况下,上发counter置为全0
	{
		memset(&ResData[9],0x00,5);	
	}

	responseLgth = 0x10;    												    //from UDHL to Response status code 

	ResData[0x0F] = errorCode ;	

	ResData[0x05] = 0x0A ;

	if(errorCode == 0x00)
	{
		ResData[0x10] = G_CurrentComNUM ;                  						//before compute the mac, we place the data behind mac here

		ResData[0x11] = SW1;

		ResData[0x12] = SW2;

		responseLgth = responseLgth + 3 ;

		if(ValueBit(G_SystemStatus,B_needCopy)== TRUE)
		{
			if(P3>POR_DATA_MAXLENGTH)	//if TP-UDL > 140bytes,errorCode is 0x07
			{
				ResData[0x0F] = ERROR_MEMERY;	
				responseLgth -= 3;
			}
			else
			{
				
				if(P3 == 0x00)								  //此时P3==00,表示长度超出范围(最大POR_DATA_MAXLENGTH)
				{
					ResData[0x0F] = ERROR_MEMERY;	
					responseLgth -= 3;
				}
				else
				{
					memcpy(&ResData[0x13],&RecData[0],P3);		
					responseLgth = responseLgth + P3;	
				}									
			}
		}
	}

	ResData[0x0E] = 0x00;

	ResData[3] = 0x00;

	ResData[4] = responseLgth - 5;

	if((spiLow & 0x20 ) == 0x00)	        //using SMS-DELIVER-REPORT 
	{  
		memcpy((BYTE *)&RecData[0x00],(BYTE *)&ResData[0],responseLgth);
	
		if(G_CommandMode == Command_Mode_2G)
		{
			SW1 = (errorCode == 0x00)?0x9F:0x9E;
		}
		else
		{
			SW1 = 0x61;
		}

		SW2 = responseLgth;	

		G_ResponseStatus = 1;

		return ;
	}

	memcpy((BYTE *)&RecData[0x01],(BYTE *)&ResData[0],responseLgth);

	RecData[0] = responseLgth;
	
	InitProactiveCommand(CMD_SENDSHORTMESSAGE,Q_SUIMT,DEV_NETWORD);
	
	tpoaFileAddr = SelectLFFile(DF_RFM, EF_RFM_ScaTpoa);	 	//EF_SCA_AND_TPOA 记录文件，记录长度：24；一条记录的结构：sca(12字节 LV结构) tpoa(12字节LV结构)
	
	offset = ResData[2] + 3;
	
	resultStatus = FALSE;
		
	dataAddr = SelectLFFile(DF_MF, EF_RFM_PorConfig);
	
	if(dataAddr != FALSE)
	{
		recNum = GetLFRecNumLgth(dataAddr,FileNum);
		
		for(i = 1;i <= recNum;i++)
		{
			ReadLFBuf(dataAddr,i,0,(BYTE *)&tar[0],0x03);
	
			if(((tar[0] == 0xFF)&&(tar[1] == 0xFF)&&(tar[2] == 0xFF))||((memcmp(&tar[0],(BYTE *)&RecData[0x07],0x03) == EQUAL)))
			{
				resultStatus = TRUE;
	
				break;	
			}
		}		
	}

	if(errorCode == ERROR_WRONGTAR)			  				
	{
		i = 1;											
		resultStatus = TRUE;	
	}
				
	ResData[offset] = TLV_ADDRESS;                                                        //tpsca tag
		
	if(resultStatus == TRUE)												
	{
		ReadLFBuf(dataAddr,i,0x0F,(BYTE *)&ResData[offset + 1],0x01);
	}
	else
	{
		 ResData[offset + 1] = 0xFF;
	}
	
	if(ResData[offset + 1] == 0xFF)	                 //	SCA value is "FF....FF"											//need copy tp_sca
	{
		if(tpoaFileAddr != FALSE)
		{
			ReadLFBuf(tpoaFileAddr, G_ReTarNo, 0, &ResData[offset + 1], sizeof(BYTE));
		}
	
		if((ResData[offset + 1] == 0xFF)||(ResData[offset + 1] == 0x00))						//no tp_sca
		{
			status = 0x00;
		}
		else
		{
			ReadLFBuf(tpoaFileAddr, G_ReTarNo, 1, &ResData[offset + 2], ResData[offset + 1]);
	
			status = ResData[offset + 1] + 2;
		}
	}
	else
	{
		ReadLFBuf(dataAddr,i,0x10,(BYTE *)&ResData[offset + 2],0x0B);
	
		status = ResData[offset + 1] + 2;
	}
		
	if(status != 0x00)
	{
		AddTLV((BYTE *)&ResData[1],(BYTE *)&ResData[offset]);
	
		offset = ResData[2] + 3;
	}								
		 
	ResData[offset + 1] = TLV_SMSTPDU;                                       //tpdu tag
	
	ResData[offset + 2] = 0x02;										  //fixed length
											
	ReadLFBuf(dataAddr,i,30,(BYTE *)&ResData[offset + 3],0x01); //first byte of submit		   //mti
	
	ResData[offset + 4] = 0x00;										  //fixed MR
	
	if(resultStatus == TRUE)		
	{				
		ReadLFBuf(dataAddr,i,3,(BYTE *)&ResData[offset + 5],0x01);				   //add TP_DA	
	}
	else
	{
		ResData[offset + 5] = 0xFF;
	}
	
	if(ResData[offset + 5] == 0xFF)									//TPDA value is "FF....FF",need copy tp_oa
	{
		if(tpoaFileAddr != FALSE)
		{
			ReadLFBuf(tpoaFileAddr, G_ReTarNo, 12, &ResData[offset + 5], 12);
		}				
	}
	else
	{
		ReadLFBuf(dataAddr,i,4,(BYTE *)&ResData[offset + 6],0x0B);	
	}
	
	if((ResData[offset + 5] == 0xFF)||(ResData[offset + 5] == 0x00))	
	{
		return ;
	}
	
	status = ResData[offset + 5]/2+ResData[offset + 5]%2 + 2 ;
	
	ResData[offset + 2] = ResData[offset + 2] + status;
						
	ReadLFBuf(dataAddr,i,30,(BYTE *)&status,0x01);							  //read mti
	
	if((status & 0x18) == 0x00)
	{
		ReadLFBuf(dataAddr,i,27,(BYTE *)&ResData[ResData[offset + 2] + offset + 3],0x02);  //pid,dcs
	
		ResData[offset + 2] = ResData[offset + 2] + 2;
	}
	else
	{
		ReadLFBuf(dataAddr,i,27,(BYTE *)&ResData[ResData[offset + 2] + offset + 3],0x03);  //pid,dcs,VP
	
		ResData[offset + 2] = ResData[offset + 2] + 3;
	}
	
	memcpy((BYTE *)&ResData[ResData[offset + 2] + offset + 3],(BYTE *)&RecData[0],RecData[0]+1);
	
	ResData[offset + 2] = ResData[offset + 2] + RecData[0] + 1;
	
	status = AddTLV((BYTE *)&ResData[1],(BYTE *)&ResData[offset + 1]);
	
	if(status == FALSE)
	{
		return ;
	}
	
	i = 0x03;
	
	while(1)
	{
		resultStatus = SendProactiveCommand();
	
		i = i - 1;
				
		if((resultStatus == CMD_Success)||(i == 0x00))
		{	 			
			return;
		}
	}
	
}


/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 RFMDownLoadExcute                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                   
  					                                                                                                         
  output parameter:  TRUE/FALSE                                                                                                                
                                                                                                                                                
  description:       下行的WIB的数据短信的执行,普通短信做RFM

  author: 			 zhuli

  status:			 OK
---------------------------------------------------------------------------------------------------------------------------------------------------*/
void RFMDownLoadExcute(BYTE refNumber)
{
	BYTE  xdata result;

	result = WIBDecryptANDCheck(refNumber);

	if(result == 0xFE)
	{
		return;
	}

	if(result != 0xFF)
	{
		ResponsePack(result);

		if(result!=ERROR_WRONGTAR)
		{
			PendingMessageAlert(D_RFMFinishTag,FALSE);
		}
	
		return;	
		
	}

	result = RFMExcuteCommand();

	ResponsePack(result);

	if((SW1 != 0x90)&&(SW1 != 0x9F)&&(SW1 != 0x61))
	{			
		PendingMessageAlert(D_RFMFinishTag,FALSE);
	}
	else
	{
		PendingMessageAlert(D_RFMFinishTag,TRUE);
	}
	 
	return;
}


void ClearSMSBufFile(void)
{
	BYTE idata recNum,i;
	UWORD idata  fileAddr;

  fileAddr = SelectTFFile(DF_STK, EF_WIBDownLoadFile);

	if(fileAddr == FALSE)
	{
		return;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	for(i=1;i<=recNum;i++)
	{
		SetLFConst(fileAddr,i,0x00,0xFF,0x10);		//清空缓存文件
	}
	

}
