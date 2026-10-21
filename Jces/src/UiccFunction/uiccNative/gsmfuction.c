/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			V1.1 修改函数GetResponseCommand，SelectCommand，ComputeRecord,ComputeBinary	 ，由于添加了BER-TLV 文件引起
			V1.2 增加函数STKStatusCommand()，用于SendProactiveCommand();并开设接口IF_STKStatusCommand()和IF_StatusCommand()
			V1.3 修改函数GetResponseCommand，修改当GetResponse发送时，若the sequence is not respected,则返回6F00
			V1.4 修改函数SeekCommand,修改对于循环文件，查找到符合条件的记录时，将当前记录设置为查找到记录的物理记录号
				 修改部分函数关于SFI的判断，短文件标识允许的范围为01-30
				 修改函数FileStatusHandler，对于USIM测试，失效的文件只允许操作deactivate 和select命令，对于激活文件，允许重复激活
				 修改函数SelectCommand、FileStatusHandler,根据TS10221规范，select by path from MF or select by path from current DF,终端不应该使用当前DF的标识符，且终端不应该use an empty data field
				 修改函数SelectCommand，当重复选择当前应用时，应用的安全状态需要被更新：PIN2未校验-联通脚本 -at 20100323
			V1.5 修改函数SelectCommand，修改当终端select by path from MF时，the terminal shall not use the 3F00
			V1.6 修改函数IncreaseCommand，增加对SFI的判断条件
			v1.7 删除函数RunGsmAlgorithmA3A8 20110616  by xyz @16:03
			V1.8 修改函数GetRecordNo,SeekCommand，增加对循环文件的特殊操作 @20110726 by xuyz
			
			V1.9 修改函数StatusPublic，增加2G模式下P3 为0时的判断
				 修改函数GetResponseCommand,该函数只允许和指定的命令对出现 ，在其他命令响应中会清除G_CurrentResponse的值 @20110808 by xuyz 
		
		
			v1.10 对于3G选择命令，当P1 == 0x03时，P3 长度为实际期望输出数据长度@20130529 by  xuyz Version 1.37->1.38
			      当选择MF的父DF时，返回6A82 ，VERSION  1.39-〉1.40 ·20130530 BY xuyz
			      
			V1.11 修改函数SelectByFileName，该函数仅用于处理在选择当前DF 的 子文件、父文件及兄弟文件，不再处理MF及ADF的选择,
			      原因：WIB 里面会出现FID为0000的情况，此时若ADF未选择，则会到MF下选择FID为0000的文件，导致文件选不到 @20140521 by xuyz
			      修改函数SelectCommand，单独处理MF及7FFF的选择@20140521 by XUYZ V1.40 ->1.41
			 
			V1.12 :修改函数SelectCommand，当fileName = 7FFF时，替换fileName为 G_CurrentAdfName[G_ChannelNum] @20140619 V1.41->1.42,备注：该次修改与V1.11共同构成一次完整的修改
			
			V1.13 :修改函数ReadRecord，对于7F20/6F1B文文件，密文输出 @20141116 by  xuyz
				   修改函数UsimGetChallenge，当P3 = 0x10，当做Superkey的信息操作@20141116 by xuyz
				   
			V1.14 修改函数ReadBinary，ReadRecord，当某个文件的ArrName配置成非2F且非6F时，则不允许读取该文件;对于EF_KI EF_AuthCFG 密文读取@20141119 by xuyz
				  修改函数UpdateRecord，当SUperkey已经重载了，即不为全FF时 ，则不允许通过DC指令去更新EF_KEY文件
			V1.15 修改函数ReadBinary，当读取文件EF_AuthCFG的内容时，要求偏移为0x00,否则返回0x6B00   @20141126   by wgl
			V1.16 修改函数FileStatusHandler与UpdateBinary,添加FDN同步功能      @20160113   by yangjian
			V1.17 修改函数SelectCommand,FileStatusHandler,ComputeAll,修正映射文件状态总与原文件状态一致的错误  @20170728 by yangjian 
			V1.18 添加宏DATASEC，仅在需要敏感数据保护功能时才加该宏                     @20180511 by yangjian 
			V1.19 修改ReadRecord指令,敏感数据保护功能上增加对3F00/2E1B文件的保护(电信单4G的KI文件) 		@20190821 by yangjian
			V1.20 修改UpdateBinary函数,宏DF_ADF使用全局变量G_DF_ID_USIM替代,增加代码兼容性      @20221206 by yangjian
			
 */
#include "uicc_main.h"		   
#include "uicc_error.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "gsmpublic.h"
#include <string.h>
//#include "resume.h"
#include "stkhead.h"
#include "Authentication.h"
#include "interface.h"
#include  "arithmatic.h"
//#include "Transaction.h"
#include "Interface_LTEFromBoot.h"



/*
*@description:  通过文件ID选择文件
*
*@param      :fileName  :待选择文件的文件ID
*
*@returns   :fileAddr/FALSE
*
*@history   :create by xyz@20100707 16:37
*/
UWORD SelectByFileName(WORD fileName)
{
	BYTE idata type;
	WORD idata  fileParentName;
	UWORD xdata fileAddr;
	
	fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName);     //是否是当前DF下的文件

	if(fileAddr == FALSE)
	{
			fileParentName = GetFileHeadWord(G_CurrentDfAddr[G_ChannelNum], F_SecondParentName_Offset);
		
			fileAddr = GetParentOrMapFileAddr(G_CurrentDfAddr[G_ChannelNum],F_ParentName_Offset);  //是否为当前DF的父文件
		
			if(fileParentName != fileName)
			{
				fileAddr = SelectFile(fileAddr,fileName);
				
				if(fileAddr != FALSE)                                		//是否是当前文件的兄弟文件
				{
				   type = GetFileHeadByte(fileAddr,F_Type_Offset);
					
					if(type !=  DFTYPE)
					{
						fileAddr = FALSE;
					}
				}
		  }
	}
	
	
	
	return fileAddr;
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 ClearLocalPinStatus                                                                                                                   
                                                                                                                                                
  input parameter:   fileAddr    当前ADF地址
					
					 *pinstatus  对应的LocalPin
  output parameter:  void                                                                                                              
                                                                                                                                                
  description:       判断当前LocalPin是否失效,如果没有失效,则清空Pin状态,用于SelectCommand重选ADF时候使用

	*@history    :V1.0 :create by LCZ 20210408

 -------------------------------------------------------------------------------------------------------------------*/ 
#if !NATIVE_CARD_UICC
void ClearLocalPinStatus(UWORD fileAddr,BYTE *pinstatus)
{
	BYTE  idata i,j,reclen;
	UWORD xdata fileAddrlocalpin;
	KeyStructer xdata chvStuct;
	
	fileAddrlocalpin =  SelectFile(fileAddr,EF_PRIVATE_PIN);
	
	if(fileAddrlocalpin != FALSE)
	{
		reclen = GetLFRecNumLgth(fileAddrlocalpin,FileLgth);
		
		for(i= 0x81;i<=0x88;i++)
		{
			j = LocateRecordInFile(fileAddrlocalpin,i);	 //查找特定类型的PIN

			if(j != FALSE)
			{
				ReadLFBuf(fileAddrlocalpin,j,0,(BYTE xdata *)&chvStuct.KeyId,reclen);
				
				if((chvStuct.KeyStatus&0x01) == PIN_ENABLED)//设置MF下面每个ADF下的 local pin1-8的状态
				{
					ClearBit(*pinstatus,(i&0x0F)-0x01);
				}
			}
		}		
	}
		
}
#endif

/**
 * @brief 		       SelectCommand:This function selects a file according to the methods rules
 *
 * @command format 	   A0 A4 00 00 02 /0X A4 00(01,03,04,08,09) XX XX
 *
 * @Returns		       void
 *
 * @history 	       
 */
void SelectCommand(void)
{
	UWORD xdata fileAddr;
	UWORD xdata currentEfHeadAddr;
	BYTE  idata num,responseLgth,type,status;
	WORD  idata fileName;
	PathInfo xdata filePathInfo;
#if !NATIVE_CARD_UICC	
	BYTE  idata recNum;
#endif
#if NATIVE_CARD_UICC    
    BYTE  idata i;
    KeyStructer xdata chvStuct;
#endif    
	
	STKControl = TRUE;
	
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在A4指令中，只有BIT0应该被置位。
	
	if((G_CommandMode == Command_Mode_2G)&&(ValueBit(G_SystemStatus, B_RfmPerform) == FALSE))   //0xA0 A4 00 00 02
	{
		if(P1P2P3Check(0x02, FALSE) ==FALSE)
		{
			return;
		}	
	}
	else if((G_CommandMode == Command_Mode_2G)&&(ValueBit(G_SystemStatus, B_RfmPerform) == TRUE))
	{
		if((P1 == 0x04)&&(P3 == 0x02))
		{
				P1P2Error();													//Send 6B 00
				return;
		}
	}	
#ifndef SingleSIM
	else if(G_CommandMode == Command_Mode_3G)    //0x00
	{
		if((P1!=0x00)&&(P1 != 0x01)&&(P1 != 0x03)&&(P1 != 0x04)&&(P1 != 0x08)&&(P1 != 0x09))
		{
			P1P2Error();
			return;
		}

		BackFilePath(&filePathInfo);	      	//备份当前路径
	}
#endif	
    else
	{;}


    if(P1 != 0x03)    //对2G情况下，P1，P2为0； 对于3G情况下，P1= 03时，为当前期望输出的数据长度，此时不应接受数据
    {
    
		ReceiveData();
	}

	if(P1 == 0x00)                        		//select by FID 
	{
		if(P3 == 0x02)
		{
			
			status = IF_SelectCommandP2Validity();

			if(status == FALSE)
			{
			 	return;	
			}


			fileName =((((WORD)RecData[0]) << 8)&0xFF00) +  RecData[1];
						
			if( (G_CommandMode == Command_Mode_2G)&&(G_FDNProcessed == 0x00) )		  //FDN过程	
			{
				if(fileName == DF_PHASE)
				{
					FdnProgcess(GSMFDN);						        //为了防止3G下6F07/6F7E文件被失效，FDN失效文件过程只在2G模式下进行
		
					G_FDNProcessed = TRUE;
				}
		
				else if(fileName == DF_CDMA) // #ifdef CDMA
				/*If the R-UIM supports the FDN feature (FDN allocated and activated) a special mechanism shall
				 exist in the R-UIM which invalidates EF
				IMSI_T, EF IMSI_M and EF TMSI once during each
				CDMA session. This mechanism shall be invoked by the R-UIM automatically if FDN is 
				enabled. This invalidation shall occur at least before the next command following selection  
				of either EF FDN is enabled when the ADN is invalidated or not activated.*/
				{
					FdnProgcess(CDMAFDN);
		
					G_FDNProcessed = TRUE;
				}
			
			}


			if((fileName == DF_MF) || (fileName == 0x7FFF))
      		{
         		if(fileName == DF_MF)
				{
					fileAddr = E_FILE_START;
				}
				else  //选择7fff,即选择USIM ADF
				{
					fileName = G_CurrentAdfName[G_ChannelNum];
					
					if(G_CurrentAdfName[G_ChannelNum] == FALSE)  //当前并未选中ADF
					{
						fileAddr = FALSE;
					}
					else
					{
						fileAddr = SelectFile(E_FILE_START, G_CurrentAdfName[G_ChannelNum]); 
					}
				}             
            }
            else
            {
            	fileAddr = SelectByFileName(fileName);	
            }
		
		}
		else if(P3 == 0x00)
		{
		 	if(P2 == 0x0C)                //直接选中MF，无数据返回
			{								
				fileAddr = E_FILE_START;

				fileName = DF_MF;	
	        }
			else
			{
				P3Error();				// Send 67 00 or 6A 87 P2error
				
				return;                                                						
			}		 
		}
		else
		{
			 P3InconsistentWithP1P2();       		//Send 6A 87

			 return;
		}
	}
#ifndef SingleSIM
	else if(P1 == 0x01)               				//选择当前DF的子DF文件
	{

	    status = IF_SelectCommandP2Validity();

		if(status == FALSE)
		{
			return;	
		}

		
		if(P3 != 0x02)                  
		{
			P3Error();
			
			return;
		}
		
		fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];
		
		fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName);    
		
		if(fileAddr != FALSE)              			//判断所选中的文件类型
		{
			type = GetFileHeadByte(fileAddr,F_Type_Offset);
			
			if(type !=  DFTYPE)
			{
				fileAddr = FALSE;
			}
		}
	}
	else if(P1 == 0x03)                				//选择当前DF的父DF文件
	{

	     status = IF_SelectCommandP2Validity();

		 if(status == FALSE)
		 {
			return;	
		 }

#if NATIVE_CARD_UICC

    	 if((G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)&&(G_A4Control == FALSE))  //当前DF为MF时，返回6A82
		 #else
				 if(G_CurrentDfAddr[G_ChannelNum] == E_FILE_START)
			#endif
    	 {
		 
    	 	FileNotFound();		                  			//Send 6A 82	
	
			return;
    	 }
    	 
		 //if((lteCardCfg == 0x00)&&(P3 != 0x00))        //联通蝴蝶结  期望00a4030400 选择上一级DF  00a4030416则是返回数据
		 if(P3 != 0x00)        //移动脚步也是P3 ！=0x00  返FCP
		 {
			fileName = G_CurrentDfName[G_ChannelNum];
			
			fileAddr = G_CurrentDfAddr[G_ChannelNum];			
		 }
		 else
		 {
			fileName = GetFileHeadWord(G_CurrentDfAddr[G_ChannelNum], F_SecondParentName_Offset);

			fileAddr = GetParentOrMapFileAddr(G_CurrentDfAddr[G_ChannelNum],F_ParentName_Offset);
		 }
 	}
	else if(P1 == 0x04)                  //select by DF name(ADF)
	{
		 if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)       // 102226  7.1 Commands  The SELECT command shall not include the selection by DF name corresponding to P1='04' in the Command Parameters of SELECT (see ETSI TS 102 221 [2]).
		 {
			 SW1 = 0x6A;
			 SW2 = 0x86;
			 return;
		 }
		
		 status = IF_SelectCommandP2Validity2();

		 if(status == FALSE)
		 {
			return;	
		 }
		 
		 /*
		 if((P2&0x80)==0x80)             
		 {
			 P1P2Error();
			 return;
		 } */

		 //if((P3==0x00)||(P3>0x10)||((P3%2) == 0x01))
		if((P3==0x00)||(P3>0x10))
		{
			P3Error();					//67 00												
			return;
		}

		if((P2&0x02) == 0x02)  			//Next occurrence or Previous occurrence
		{
			 FileNotFound();			//because  exist  only one ADF
				
			 return;
		}
		 
		if((P2&0x60) == 0x40)  			//Application session Termination 
		{
			G_CurrentAdfName[G_ChannelNum] = 0;

			fileAddr = E_FILE_START;

			fileName = 0x3F00;
		}
		else
		{
			fileAddr = SelectFileByName(E_FILE_START,&RecData[0],P3);
			
			if(fileAddr == FALSE)
			{
				 FileNotFound();		//Send 6A 82
				
				 return;
			}
			
			ReadEeprom((BYTE *)&fileName,fileAddr,sizeof(WORD));
			
	#if NATIVE_CARD_UICC
			if(G_CurrentAdfName[G_ChannelNum] == fileName)
			{

				for(i = 0x81;i<=0x88;i++)//local Pin 1-8
				{
					status = ReadKey(i,(BYTE xdata *)&chvStuct.KeyId);

					if(status == TRUE)
					{
						if((chvStuct.KeyStatus&0x01) != PIN_DISABLED)//如果localpin不是disabled，才允许清除localpin的状态
						{
					
							ClearBit(G_KeyStatus,((i&0x7F)));	//修正清除PIN2状态错误,脚本为UICC补充测试-PIN2状态检测
						}
					}
				}

			}
#else
			
			recNum = GetFileHeadByte(fileAddr,F_DIRNum_offset);
			
			if((G_CurrentAdfName[G_ChannelNum] == fileName) && (recNum != 0))								// 重复选择当前激活的ADF时（P2的bit7bit6=00）意为复位当前ADF，PIN的校验状态要随之更新。若存在多个应用时，该处还要考虑
			{
				ClearLocalPinStatus(fileAddr,&G_LocalPinStatus[recNum-1]);
			}
#endif
			
			G_CurrentAdfName[G_ChannelNum] = fileName;
		}
	}
#endif
	else     							//select by path from MF or current DF
	{
		
		 status = IF_SelectCommandP2Validity();

		 if(status == FALSE)
		 {
			return;	
		 }

		if((P3%2) == 0x01)
		{
			 P3Error();
			 return;
		}

        fileName = ((RecData[0]<<8)&0xFF00) + RecData[1];

		if(P1 == 0x08)
		{	 
			if(fileName == 0x3F00)   				//IN case of select path from MF ,the terminal shall not use the FID of MF at the begining of the path
			{
				ConditionSatisfied();				// 69 85
				return;
			}
			
            fileAddr = E_FILE_START;

            SetChannel(fileAddr,DF_MF,FALSE);
        }
		else
		{	 
		 	 if(fileName == 0x7FFF)      				//in case of "select by path from current DF",the terminal shall not use 7FFF at the begining of the path
		 	 {
		 	 	ConditionSatisfied();	 				// 69 85				
				return;
		 	 }
		 	 fileAddr = G_CurrentDfAddr[G_ChannelNum];
		}

		if(fileName == G_CurrentDfName[G_ChannelNum])  	//select by path from MF or current DF,the terminal shall not use the identity of the current DF
		{
			if(G_CommandMode == Command_Mode_3G)
			{
				ResumeFilePath(&filePathInfo);              //restriction 3 need consider 2010-03-23
			}
			
			FileNotFound();		                  		//Send 6A 82	
	
			return;
		}
		
		for(num=0x00;num<P3;num=num+2)
		{
             fileName = (((WORD)RecData[num]<<8)&0xFF00) + RecData[num+1];

       		if((fileName == 0x7FFF))
       		{
				fileName = G_CurrentAdfName[G_ChannelNum];
       		}
			
			fileAddr = SelectFile(fileAddr,fileName);
			 
			 if(fileAddr == FALSE)
			 {
				 break;
			 }
			 else
			 {
				  SetChannel(fileAddr,fileName,FALSE);
 			 }
		 }
	}

	if(fileAddr == FALSE)                         		//若文件未找到，则恢复备份路径
	{
#ifndef SingleSIM
		if(G_CommandMode == Command_Mode_3G)
		{
			ResumeFilePath(&filePathInfo);
		}
#endif
		FileNotFound();		                  			//Send 6A 82	
	
		return;
	}
	
	
	if(((P2&0x0C)==0x04)||(G_CommandMode == Command_Mode_2G))    //return FCP template
	{
		if(P1 != 0x03)
		{
			responseLgth =  SetChannel(fileAddr,fileName,TRUE);
		}
		
		if(P1 == 0x03)
		{
			#if NATIVE_CARD_UICC
		   status = IF_ParentDFFCP(fileAddr,fileName);					//中国移动与中国联通平台处理有区别

		   if(status == TRUE)
		   {
		   		return;
		   }
			 #else
			 responseLgth =  SetChannel(fileAddr,fileName,TRUE);
			 apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, responseLgth);
				
				GlobalSetStatus(0x9000);
			 return;
			 
			 #endif
		}
	}
	else
	{
		responseLgth =  SetChannel(fileAddr,fileName,FALSE);
	}

   	if(G_UnicomResponseLgth!=0x00)
	{
		CommandFinish(G_UnicomResponseLgth);

		G_UnicomResponseLgth = 0x00;
	}
	else
	{
		CommandFinish(responseLgth);								 //send 9f 19
	}
														
#ifndef  SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{

		type = GetFileHeadByte(fileAddr,F_Type_Offset);										
		
		if(type != ADFTYPE)										 //没有判断DF的状态
		{
			currentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);
			
			type = GetFileHeadByte(currentEfHeadAddr,F_Status_Offset);
									
			if((type&0x01) == FILE_INVALIDATE)												
			{
				SelectedFileInvalid();							 //62 83
			}
		}
	}
#endif
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在A4指令中，只有BIT0应该被置位。
}

/**
 * @brief 		       StatusCommand的公共子函数
 *
 * @Returns		       void
 *
 * @history 	       
 */
BYTE StatusPublic(void)
{  
	BYTE idata lgth;

	memset(RecData, 0x00,sizeof(RecData));
	
	if((P1>0x02)||((P2 != 0x00)&&(P2!=0x01)&&(P2!=0x0C)))
	{
        P1P2Error();
		return FALSE;
	} 

		if(((P1 == 0x01) || (P1 == 0x02)) && (G_CurrentAdfName[G_ChannelNum] == FALSE))  //联通F1_TC1 ManageChannel测试,Current application is initialized in the terminal/The terminal will initiate the termination of the current application
	{
		P1P2Error();
		return FALSE;
	}
	
	if(G_CommandMode == Command_Mode_2G)
	{	
		if(P1P2Check() ==FALSE)
		{
			return FALSE;
		}

		if(( P3 > R_DF_LENGTH)||(P3 == 0x00))  //增加不支持P3 == 0X00的判断
		{
			//P3Error();
			P3ReturnRightLength2G(R_DF_LENGTH);
			return FALSE;
		}
	}

#ifndef SingleSIM
	if((P2 == 0x0C)&&(P3 != 0x00))
	{
		P3Error();
		return FALSE;
	}
#endif

	if(P2 != 0x0C)
	{
		if(P2 == 0x00)
		{
			if (G_CommandMode == Command_Mode_2G)
			{
				lgth = GetDfInformation();
			}
#ifndef SingleSIM
			else
			{
				lgth = UsimGetDfInformation();
			}
#endif
		}
#ifndef SingleSIM
		else
		{
			lgth = GetCurrentADFAid(G_CurrentAdfName[G_ChannelNum],(BYTE *)&RecData[0]);
		}


        if(lgth == FALSE)
        {
            P1P2Error();

            return FALSE;                        
        }
#endif
		if((P3 == 0x00)||(P3 > lgth))               
		{
		  	RepeatPreviousCmdHeader(lgth);
			
		  	return FALSE;
		}

		SendData();
	}	
	return TRUE;	
}

/**
 * @brief 		       StatusCommand:This function returns information concerning the current directory or current application
 *
 * @command format 	   A0 F2 00 00 lgth /8X F2 00(01,02) 00(01,0C) XX
 *
 * @Returns		       void
 *
 * @history 	       
 */
void StatusCommand(void)
{	
	BYTE idata result;
	
	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在F2指令中，只有BIT0应该被置位。

 	result = StatusPublic();

	if (result == TRUE)
	{
		IF_StatusCommand();
		
		CommandFinish(0x00);
	}
	
	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在F2指令中，只有BIT0应该被置位。
}

/**
 * @brief 		       STKStatusCommand:This function returns information concerning the current directory or current application
 *
 * @command format 	   A0 F2 00 00 lgth /8X F2 00(01,02) 00(01,0C) XX
 *
 * @Returns		       void
 *
 * @history 	       
 */
void STKStatusCommand(void)
{
	BYTE idata result;

 	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，只有BIT0应该被置位。
	
	result = StatusPublic(); 

	IF_STKStatusCommand();

	if (result == TRUE)
	{
		CommandFinish(0x00);
	}

//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查,只有BIT0应该被置位。
}

/**
 * @brief 		   	   ComputeBinary:This function is to compare the file condition is do or not
 *
 * @command format 	   
 *
 * @Returns		       TRUE/FALSE
 *
 * @history 	       
 */
BYTE ComputeBinary(BYTE action)
{
	BYTE     idata actionTag;
	WORD     idata binaryFileLgth,spaceSize;
	WORD	 idata lgth;

	
#ifndef SingleSIM
  	if(((P1&0xE0)==0x80)&&(G_CommandMode == Command_Mode_3G))      
	{
		actionTag = P1 &0x1F;      //b1-b5 :SFI (01-30)
		
		if(actionTag == 0x1F)
		{
			P1P2Error();

			return FALSE;
		}
		
		P1 = 0x00;
  	}
	else
#endif
	{
		actionTag = 0x00;
	}

	actionTag = ComputeAll(action,actionTag,1);	  //判断当前选中的EF文件权限、状态、类型是否符合条件
		
	if(actionTag == FALSE) 
	{
		return FALSE;
	}
	
	spaceSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum],F_SpaceSize_Offset);	//获取透明文件的空间大小
	
	binaryFileLgth = (((WORD)P1<<8)&0xFF00)|(P2&0x00FF);


	lgth = P3;

	if (P3 == 0x00)
	{
		if(action==A_ReadFile)
		{
			lgth = 0x100;
		}
		else
		{
			P3Error();
			return FALSE;
		}
	}
   
	
	if(spaceSize <= binaryFileLgth)
	{
		P1P2Error();	
														//send 6B 00
		return FALSE;
	}     
	
																											//不需要在此平台支持国际卡RFM功能,此处代码不需要
	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
	{
		if((action == A_ReadFile)&&(P3 == 0x00))   //对于RFM 操作而言，P3 = 0,返回透明文件剩余全部数据
		{
//			if((spaceSize-binaryFileLgth)<0x100)    // 当剩余有效数据不足0x100时，返回剩余有效数据	;否则返回0x100字节
//			{
//					P3 = spaceSize-binaryFileLgth;	
//					return TRUE;		
//			}	
			return TRUE;
		}
	}
	
	if(spaceSize < binaryFileLgth + lgth)
	{
#ifndef SingleSIM
		if((action == A_ReadFile)&&(G_CommandMode == Command_Mode_3G))
		{
			RepeatPreviousCmdHeader((BYTE)(spaceSize-binaryFileLgth));	
		}
		else
#endif
		{			
			P3Error();												   //67 00
		}

		return FALSE;
	}

	return TRUE;
}
/**
 * @brief 		       ReadBinaryCommand:This function reads a string of bytes from the current transparent EF
 *
 * @command format 	   A0 B0 high low lgth /0X B0 high low lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void ReadBinary(void)
{
	BYTE idata actionTag;
	WORD idata fileLgth;
//	BYTE idata arrName;	
	BYTE idata tempResult;
	WORD idata spaceSize;
	


//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在B0指令中，只有BIT0应该被置位。
	actionTag = ComputeBinary(A_ReadFile);

	if(actionTag == FALSE)
	{
		return ;
	}

	/*																																									//取消这个功能，for test
	arrName =  GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_ArrName_Offset);
	
	if((arrName != 0x2F)&&(arrName != 0x6F))
	{		
		SecuritySatisfied();
		
		return ;
	}
	*/
	
	fileLgth = (((WORD)P1 << 8)&0xFF00 )|(P2 & 0x00FF);

	if(P3==0x00) 					//lgth是256个字节
	{
	    if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
		{
			spaceSize = GetFileHeadWord(G_CurrentEfAddr[G_ChannelNum],F_SpaceSize_Offset);	//获取透明文件的空间大小
		
			if(spaceSize <( fileLgth+0x100))  // 当剩余有效数据不足0x100时，返回剩余有效数据
			{
				P3 = spaceSize - fileLgth; 
				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],P3);		
			}
			else
			{
				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],0x80);
				ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth+0x80,(BYTE *)&RecData[0x80],0x80);
				
				if(spaceSize ==( fileLgth+0x100))
				{
					SW1 = 0x90;
					SW2 = 0x00; 
				}
				else
				{
					SW1 = 0x62;
					SW2 = 0xF1;  //剩余数据被切断
				}
			}					
		}
		else
		{
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],0x80);
			ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth+0x80,(BYTE *)&RecData[0x80],0x80);	
		}
	}
	else
	{
		ReadTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0x00],P3);
	}	
	
	if((G_CurrentDfName[G_ChannelNum] == DF_MF)&&(G_CurrentEfName[G_ChannelNum] == EF_AuthCFG)) //该文件长度（包括填充后不能超过200）
	{		
		if((fileLgth%0x6F)!=0x00)		 //因为只加密OPC段数据，所以为了保证偏移正确（偏移1 byte）的字节数解密
		{
			P1P2Error();
			return;
		}	
		if(G_SuperKeyAccess != SuperKeyAccess_PassVerify)
		{
			SecuritySatisfied();
		
			return ;
		}

		tempResult = ReadKey(EncryKey_Type,&RecData[200]); //RecData[204]开始的16字节是Encry Key密钥
		
	    if(tempResult == FALSE)
	    {
	    	ReferencedDataInvalid();

	    	return;
	    }
    	
    	 	
    	TripleDES(&RecData[0x01],&RecData[0xCC],&RecData[0x01],CBC_Encrypt,0x10,16);  //OPC加密输出
    	
	}	
	
	IF_CDMAGSM();
	
	SendData();
	
	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在B0指令中，只有BIT0应该被置位。

	CommandFinish(0x00);			//send 90 00       
		                                                              
}

/**
 * @brief 		       UpdataBinaryCommand:This function updates the current transparent EF with a string of bytes
 *
 * @command format 	   A0 D6 high low lgth /0X D6 high low lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void UpdateBinary(void)
{
	BYTE idata  actionTag,tag;
	WORD idata 	fileLgth;
	
	actionTag = ComputeBinary(A_UpdateFile);

	if(actionTag == FALSE)
	{
		return ;
	}

	fileLgth = (((WORD)P1 << 8)&0xFF00 )|((WORD)P2 & 0x00FF);
	
	tag = ReceiveData();
	
	if(tag == TRUE)
	{
		actionTag = WriteTFBuf(G_CurrentEfAddr[G_ChannelNum],fileLgth,(BYTE *)&RecData[0],P3); 
	
		if (actionTag == FALSE)
		{
			MemoryError();											//65 81/92 40
	
			return;
		}

		tag = IF_UpdateBinary();

		if(tag == TRUE)		  						//当要执行应用时，退出
		{
			return;
		}		
	}
	
	if(G_CommandMode == Command_Mode_3G)   //3G下分配服务列表或修改EF_EST文件（enabling/disabling FDN服务）均有可能改变FDN服务状态 ，需要同步到2G模式
	{
		if((G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)&&((G_CurrentEfName[G_ChannelNum] ==EF_EST)||(G_CurrentEfName[G_ChannelNum] ==EF_UST)))
		{
			tag = IsFdnServiceActiveInUSIM();	  //获取fdn服务在USIM模式下的状态
			
			UsimFdnSYNCtoSIM(tag);				//同步到SIM
		}
	}
	else
	{
		 if((G_CurrentDfName[G_ChannelNum] == DF_PHASE )&&(G_CurrentEfName[G_ChannelNum] == EF_SST))//2G模式下分配服务列表或 激活/失效 EF_ADN文件 可能改变FDN状态 需要同步到3G模式
		 {
			 tag = IsFdnServiceActiveInSIM();	   //获取fdn服务在SIM模式下的状态
				 
			 SimFdnSYNCtoUSIM(tag);			 	 //同步的USIM
		 }
	}

  	CommandFinish(0x00);                                            //SEND 90 00
}
/*
*@function   :GetPhyNum

*@description:获取记录/循环文件的物理记录号
*
*@see         :	102221
*
*@param		  :BYTE action --     A_UpdateFile/ A_ReadFile
*			   UWORD fileAddr--	  所操作的文件地址
*			   BYTE mode--		PRIVOUS_RECORD,NEXT_RECORD,ABSOLUTE_CURRENT
*              BYTE accRecord -- 指令中传进来的记录号，ABSOLUTE_CURRENT模式会使用到
               BYTE* currentRec --  相对记录号的地址
*@return	  :物理记录号
*
*@author      :zhxx
*
*@history	  :V1.0 :create by zhxx@20200730          

*/
BYTE GetPhyNum(BYTE action,UWORD fileAddr,BYTE mode,BYTE accRecord,BYTE* currentRec)
{
	BYTE idata logicFirst,recNum,phyNum,maxRecNum,fileType;
	
	fileType = (GetFileHeadByte(fileAddr, F_Type_Offset) & 0x07);
	
	logicFirst = 0;
	
	if(fileType == CYCTYPE) //只有循环文件才会使用到logicFirst
	{
		logicFirst = getCycleFPoint(fileAddr); //获取基址，即逻辑上的第一条记录的记录号
	}	
	
	maxRecNum = GetLFRecNumLgth(fileAddr,FileNum);  //获取文件的总记录个数
	
	if((fileType == CYCTYPE) && (action == A_UpdateFile))//For a cyclic EF the record containing the oldest data is updated, the record pointer is set to this record and this record becomes record number 1.
	{
		recNum = 1;
		
		phyNum = logicFirst - 1;

		if(phyNum == 0)
		{
			phyNum = maxRecNum;
		}
	}
	else
	{
		recNum = GetRecordNo(fileAddr,fileType, mode, accRecord,*currentRec); 
		
		if(fileType == CYCTYPE)
		{
			phyNum = ((logicFirst + recNum - 1) > maxRecNum) ? (logicFirst+recNum-1 - maxRecNum): (logicFirst+recNum-1);
		}
		else
		{
			phyNum = recNum;
		}
	}	
	
	if((recNum == FALSE) || (phyNum == FALSE))
	{
		return FALSE;
	}
	
	if(mode != ABSOLUTE_CURRENT)
	{
		*currentRec = recNum;
	}
		
	return phyNum;
}
/*
*@description:根据记录模式获取记录文件或循环文件的相对记录号
* @param      UWORD fileAddr -- 所操作文件地址
*@param      :BYTE type--文件类型：循环 or 线形
*@param      :BYTE mode :PRIVOUS_RECORD,NEXT_RECORD,ABSOLUTE_CURRENT
*@param      :BYTE record: 传入的记录号，ABSOLUTE_CURRENT才会用到
*@param     :BYTE currentRec --  逻辑记录号
*
*@history  :create by zhxx @20200730
*/
BYTE GetRecordNo(UWORD fileAddr,BYTE type,BYTE mode,BYTE record,BYTE currRec)
{		
	BYTE idata isLinearFile, maxRecNum, rtnNo;

	maxRecNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	if(maxRecNum == 0) return 0;

	isLinearFile = (type == LINTYPE);

	switch(mode & 0x0F) {
		case NEXT_RECORD: 				
			if(currRec == 0x00)
				rtnNo = 1;
			else 
				rtnNo = ((currRec+1) <= maxRecNum) ? (currRec+1) : (isLinearFile ? 0 : 1);
			break;

		case PRIVOUS_RECORD:			
			if(currRec == 0x00)
				rtnNo = maxRecNum;
			else 
				rtnNo = ((currRec-1) > 0) ? (currRec-1) : (isLinearFile ? 0 : maxRecNum);
			break;

		default: //ABSOLUTE_CURRENT
			if(record == 0x00) //CURRENT
				rtnNo = (currRec>=1)&&(currRec<=maxRecNum) ? currRec : 0x00;
			else //ABSOLUTE
				rtnNo = (record>=1)&&(record<=maxRecNum) ? record : 0x00;
			break;
	}
		
	return rtnNo;		
}
/**
 * @brief 		   	   ComputeRecord:This function is to compare the file condition is do or not
 *
 * @param              A_ReadFile/A_UpdateFile 	   
 *
 * @Returns		       recNo/FALSE
 *
 * @history 	       
 */
BYTE ComputeRecord(BYTE action)
{
	EFileHead xdata  fileHead;
	BYTE idata tag,currentRec,phyNum;

	tag = (G_CommandMode == Command_Mode_2G)?P2:(P2&0x07);

	if((tag != NEXT_RECORD)&&(tag != PRIVOUS_RECORD)&&(tag != ABSOLUTE_CURRENT))
	{
		P1P2Error();														//Send 6B 00
		return FALSE;
	}

#ifndef SingleSIM
	if((P1 != 0x00)&&(tag == NEXT_RECORD || tag == PRIVOUS_RECORD))
	{
		if(G_CommandMode == Command_Mode_3G)  								//2G在此情况下要求不解释P1，参考GSM11.11
		{
			P1P2Error();													//Send 6B 00
			return FALSE;		
		}
	} 

	if((P2&0xF8)!= 0x00)
	{
		tag = (P2>>3)&0x1F;
		
		if(tag == 0x1F)
		{
			P1P2Error();
			return FALSE;
		}
		
		P2  = (P2 &0x07);
	}
	else
#endif
	{
		tag =0x00;
	}
	
	tag =ComputeAll(action,tag,0);
		
	if(tag == FALSE)
	{
		return FALSE;
	}

	
	ReadEeprom((BYTE *)&fileHead.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(EFileHead));
	
	if((action == A_UpdateFile)&&((fileHead.FileType&0x37) == CYCTYPE) && (P2 !=PRIVOUS_RECORD))
   	 {
		if(G_CommandMode == Command_Mode_2G)
		{
			P1P2Error();													//Send 6B 00												//94 08
		}
#ifndef SingleSIM
		else
		{
			FileStructureError();											//69 81
		}
#endif														
		return FALSE;
	}       
	
	if( P2 == ABSOLUTE_CURRENT)
	{

			if(P1 > fileHead.efSize.recInfo[0])
			{
				AddressInvilid();													//94 02	

				return FALSE;
			}
		
	}
	
	currentRec = G_CurrentRecNum[G_ChannelNum];

	phyNum =  GetPhyNum(action,G_CurrentEfAddr[G_ChannelNum],P2,P1,&currentRec);

	if(phyNum == FALSE)
	{
		AddressInvilid();		//94  02
												
		return FALSE;
	}
	

#ifndef ELECTRONIC	
	
	
#ifndef SingleSIM    
    if((P3 == 0x00)&&(G_CommandMode == Command_Mode_3G))
    {
        RepeatPreviousCmdHeader(fileHead.efSize.recInfo[1]);

        return FALSE;
    }       
#endif
	if(P3 != fileHead.efSize.recInfo[1])
	{
		P3Error();														//Send 67 00

		return FALSE;
	}	
	
	
	
	
#else

	if(G_CommandMode == Command_Mode_3G)                              //电气特性测试需要add by xyz@2010-01-21
	{
		if(P3 != fileHead.RecLength )
		{
			 RepeatPreviousCmdHeader(fileHead.RecLength);
			 
			 return FALSE;
		}
		
	}  
	else
	{
		if(P3 != fileHead.RecLength)
		{
			P3Error();														//Send 67 00

			return FALSE;
		}	
	}
	
#endif
  
    G_CurrentRecNum[G_ChannelNum] = currentRec;
  
	return phyNum;	
}

/**
 * @brief 		       ReadRecordCommand:This function read one record from current fixed EF or cyclic EF
 *
 * @command format 	   A0 B2 recNo mode lgth /0X B2 recNo mode lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void ReadRecord(void)
{
	BYTE idata phyNum;
	BYTE idata tempResult;

	
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在A4指令中，只有BIT0应该被置位。	

	phyNum = ComputeRecord(A_ReadFile);

	if(phyNum == FALSE)
	{
		return;
	}
	
	/*																																							//取消这个功能,for test
	arrName =  GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_ArrName_Offset);
	
	if((arrName != 0x2F)&&(arrName != 0x6F))
	{		
		SecuritySatisfied();		
		return ;
	}
	*/
	
	ReadLFBuf(G_CurrentEfAddr[G_ChannelNum],phyNum,0,(BYTE *)&RecData[0],P3);	//get the content of the current record.
	
		
	if(((G_CurrentEfName[G_ChannelNum] == EF_KI)&&(G_CurrentDfName[G_ChannelNum] == DF_PHASE))
	||((G_CurrentEfName[G_ChannelNum] == EF_Single4G_KI)&&(G_CurrentDfName[G_ChannelNum] == DF_MF))
	||((G_CurrentEfName[G_ChannelNum] == EF_KI)&&(G_CurrentDfName[G_ChannelNum] == G_DF_ID_USIM)))
	{
		if(G_SuperKeyAccess != SuperKeyAccess_PassVerify)
		{
			SecuritySatisfied();
		
			return ;
		}

		tempResult = ReadKey(EncryKey_Type,&RecData[0x50]); //RecData[0x54]开始的16字节是Encry Key密钥
		   
	    if(tempResult == FALSE)
	    {
	    	ReferencedDataInvalid();
	    	return;
	    }
    
    	if((P3%8) != 0x00)
    	{
    		P3Error();
    		return;
    	}
    	
    	 	
    	TripleDES(&RecData[0x00],&RecData[0x54],&RecData[0x00],CBC_Encrypt,P3,16);  //Ki密文输出,KI一定是8的倍数
    	
	}
	else if((G_CurrentEfName[G_ChannelNum] == EF_KEY)&&(G_CurrentDfName[G_ChannelNum] == DF_MF) )
	{		 	
    	
		if(G_SuperKeyAccess != SuperKeyAccess_PassVerify)
		{
			SecuritySatisfied();
		
			return ;
		}	
		tempResult = ReadKey(EncryKey_Type,&RecData[0x50]); //RecData[0x54]开始的16字节是Encry Key密钥

	    if(tempResult == FALSE)
	    {
	    	ReferencedDataInvalid();
	    	return;
	    }

		TripleDES(&RecData[0x04],&RecData[0x54],&RecData[0x04],CBC_Encrypt,0X10,16);  //对密钥value进行加密
	}

		
    SendData();	
	
	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在A4指令中，只有BIT0应该被置位。
	
	CommandFinish(0x00);
}

/**
 * @brief 		       UpdateRecordCommand:This function update one record from current fixed EF or cyclic EF
 *
 * @command format 	   A0 DC recNo mode lgth /0X DC recNo mode lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void UpdateRecord(void)
{
	BYTE idata phyNum,need_commit;
	BYTE idata tag,fileType,result;
	BYTE idata tempResult;
	UWORD xdata fileAddr,tempAddr;
	BYTE idata lgth,i,tmp;
	WORD idata fileId;

	
	if((G_CurrentDfName[G_ChannelNum] == DF_MF)&&(G_CurrentEfName[G_ChannelNum] == EF_KEY)) //若superKey已经更新，不允许通过DC指令去更新EF_Key文件
	{		
		memset(&tempBuf[0x00],0x00,0x20);

		memset(&tempBuf[0x20],0xFF,0x10);
		
		tempResult = ReadKey(SuperKey_Type,&tempBuf[0]); //tempBuf[4]开始的16字节是Super Key密钥
	    
	    if((tempResult != FALSE)&&(memcmp(&tempBuf[4],&tempBuf[0x20],0x10) != EQUAL))
	    {
	    	ReferencedDataInvalid();

	    	return;
	    }	    	   
	}
	
		
	phyNum = ComputeRecord(A_UpdateFile);

	if(phyNum == FALSE)
	{
		return;
	}			

	result = ReceiveData();

	if(result == TRUE)
	{
		IF_UpdateRecordPRL(phyNum);
		
		if((G_CurrentDfName[G_ChannelNum] == 0x7F10)&&(G_CurrentEfName[G_ChannelNum] == 0x6F3A)&&(G_CommandMode == Command_Mode_2G))//电话本同步功能
		{
	
			fileAddr = SelectLFFile(0x7F10,0x5F3A);

			tempAddr =  SelectFile(fileAddr,0x4F30);  //查找5F3A下EFPBR的地址

            //这里需要同步的EFPBC一般默认存储在4F30的第一条记录，因为文件系统中一般配置成第一条记录所指示的电话本与7F10/6F3A具有映射关系.若需求指示其他电话本与7F10/6F3A有映射关系，此处需要修改，请咨询COS部门

			if(tempAddr != FALSE)  //读取EFPBR的第一条记录，获取对应的EFPBC 的文件ID 。（因为第一条记录对应的号簿和7F10/6F3A有映射关系）.
			{
				lgth = GetLFRecNumLgth(tempAddr,FileLgth);

				ReadLFBuf(tempAddr,0x01,0x00,&tempBuf[0],lgth);

				for(i = 0x00; i< lgth; i++)  //过滤掉0xFF字段
				{
					if(tempBuf[i] == 0xFF)
					{
						break;
					}
				}

				lgth = i; //实际有效数据长度为i

				i = FindTag(&tempBuf[0],0xA8,lgth); //返回A8长度的偏移位置

				if(i != FALSE)
				{
					lgth = tempBuf[i];

				  tmp = FindTag(&tempBuf[i+1],0xC5,lgth); //C5tag中包含的是EFPBC的信息

					if(tmp != FALSE)
					{
						fileId = (((WORD)tempBuf[i+1+tmp+1]) <<8)|(tempBuf[i+1+tmp+2]);

						fileAddr = SelectFile(fileAddr,fileId);

						if (fileAddr != FALSE)
						{
							SetLFConst(fileAddr, phyNum, 0x00, 0x01, 0x01);
						}
					}
				}
			}
		}
		
		need_commit = IF_BeginTransaction();		
	
		tag = WriteLFBuf(G_CurrentEfAddr[G_ChannelNum],phyNum,0,(BYTE *)&RecData[0],P3);

		fileType = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Type_Offset);//获取文件类型	

		if((P2 != ABSOLUTE_CURRENT) && ((fileType & 0x07) == CYCTYPE))	//循环文件才会更新记录指针
		{	
//		   tag = tag & 	SetEepromEX(getFileContentAddr(G_CurrentEfAddr[G_ChannelNum]),phyNum,1);
//			
//		   if(tag == FALSE)
//		   {
//			    MemoryError();
//			   
//				if(need_commit == TRUE)
//				{
//					TransactionAbort();
//				}		
//				return; 
//			}
		  //这里调用VMSetBuf（memFillAtomic，原子写），不能调用SetEepromEX（nvmFillNoAtomic，非原子写），解决循环文件断电问题,by xwh 20220530
			VMSetBuf(getCycleFPointAddr(G_CurrentEfAddr[G_ChannelNum]),phyNum,sizeof(BYTE));							
	   }
		
		if(tag == FALSE)
		{
			MemoryError();
		   
			IF_AbortTransaction(need_commit);
			
			return; 
		}
		
	   IF_CommitTransaction(need_commit); 
	
	   IF_UpdateRecord(phyNum);
	}

	CommandFinish(0x00);
}
/**
 * @brief 		      SeekCommand:This function searchs a specific pattern from current fixed EF or cyclic EF
 *
 * @command format 	   A0 A2 00 type/mode lgth /0X A2 recNo mode lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void SeekCommand(void)
{
	signed char idata step;
	BYTE idata tag;
	BYTE idata recNumStart;	
	BYTE idata offset;
	BYTE idata length;								
	BYTE * searchString;
	EFileHead xdata fileHead;


	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在A2指令中，只有BIT0应该被置位。

	if( ((G_CommandMode == Command_Mode_2G)&&((P1!= 0x00)||((P2>>4)&0x0F)>0x01||(P2&0x0F)>0x03) )
		||(( ((P2&0xF8) == 0xF8)||((P2&0x04) == 0x00) )&&(G_CommandMode == Command_Mode_3G)))
	{
		P1P2Error();												//6B 00
		return ;
	}                                       								

#ifndef SingleSIM
	if( (G_CommandMode == Command_Mode_3G)&&((P2&0xF8) != 0x00))	//使用 SFI
	{
		tag = (P2 >> 3)&0x1F;										//tag as SFI
		
		if(tag == 0x1F)
		{
			P1P2Error();
			return;
		}
	}
	else
#endif
	{
		tag =0x00;
	}
	
	tag=ComputeAll(A_ReadFile,tag,4);
		
	if(tag == FALSE) 
	{
		return ;
	}
	
	ReadEeprom((BYTE *)&fileHead.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(EFileHead));

	if((G_CommandMode == Command_Mode_2G)&&((P3>0x10)||(P3>fileHead.efSize.recInfo[1])||(P3 == 0x00)))        //check the range of P3.
	{
		P3Error();                                             //67 00
		return ;
	} 
 
#ifndef SingleSIM 
	if(G_CommandMode == Command_Mode_3G)
	{
	
		if(P1 > fileHead.efSize.recInfo[0])
		{
			P1P2Error();
			
			return;
		}
		
	}   
#endif
	
	ReceiveData();
	
	step = 1;                                               //默认为forward
	
	offset = 0x00;											//默认是0偏移            

	tag = FALSE;											//默认是不使用first occurrence方式

	searchString = &RecData[0];								

	length = P3;
	
	if(G_CommandMode == Command_Mode_2G)                   //2G模式search
	{
		switch(P2&0x0F)
		{
			case FRM_BGN_FRD :
			{ 
				recNumStart = 0x01;

				break;
			}
			case FRM_LST_BFD : 
			{
				step = -1;

				recNumStart = fileHead.efSize.recInfo[0];
			
				break;
			}
			case FRM_CUR_FRD : 
			{
				recNumStart = (G_CurrentRecNum[G_ChannelNum] == 0x00) ? 0x01:(G_CurrentRecNum[G_ChannelNum]+0x01);

				break;
			}
			default : 
			{
				step = -1;									//backward

				recNumStart = (G_CurrentRecNum[G_ChannelNum] == 0x00) ? (fileHead.efSize.recInfo[0]):(G_CurrentRecNum[G_ChannelNum]-0x01);

				break;
			}
		}
	}
#ifndef SingleSIM
	else													//3G模式search
	{
		if((P2&0x06) == 0x04)									//SIMPLE MODE
		{
			if(P1 == 0x00)                                      //current record
			{
				if(G_CurrentRecNum[G_ChannelNum] == 0x00)
				{
					AddressInvilid();							//6A 83
					return;
				}
	
			
			recNumStart = G_CurrentRecNum[G_ChannelNum];
	
			}
			else
			{
				recNumStart = P1;
			}
	
			if(P3 == 0x00 || P3 > fileHead.efSize.recInfo[1])
			{
				P3Error();										//67 00
				return; 
			}
	
			if((P2&0x01) == 0x01)								//backward
			{
				step = -1;
			}
	
		}
		else if((P2&0x07) == 0x06)								//ENHANCED MODE		
		{
			if((P3 < 0x03) || (P3 > (fileHead.efSize.recInfo[1]+0x02))) //data = search indication + search  string
			{
				P3Error();										//67 00
				return;
			}
	
			if((RecData[0]&0x04) == 0x00 || (RecData[0]&0xF0) != 0x00) 
			{
				IncorrentParameter();							//6A 80
	
				return;			
			}
	
			if((RecData[0]&0x08) == 0x08)						//After First Occurrence search
			{
				tag = TRUE;
			}
			else												//Offset search
			{
				if(RecData[1] + (P3-2) > fileHead.efSize.recInfo[1])
				{
					IncorrentParameter();						//6A 80
					return;
				}
				
				offset = RecData[1];
			}
	
			if((RecData[0]&0x02) == 0x00)						//ABSOLUTE
			{
				if(P1 == 0x00)
				{
					if(G_CurrentRecNum[G_ChannelNum] == FALSE)
					{
						AddressInvilid();						//6A 83
						return;
					}
	
			
			        recNumStart = G_CurrentRecNum[G_ChannelNum];
			
				}
				else   
				{
					recNumStart = P1;
				}
	
				if((RecData[0]&0x01) == 0x01)					//backward
				{
					step = -1;
				}				
			}
			else                                               //forward or  backward
			{
				if(P1 != 0x00)
				{
					P1P2Error();								//6b 00
					return;
				}
				
				if((RecData[0]&0x03) == 0x03)                   //backward
				{
					step = -1;
				}
				if(G_CurrentRecNum[G_ChannelNum] == FALSE)		//未指定
				{
					if(step == 0x01)                              //forward
					{
						recNumStart = 0x01;
					}
					else
					{
					
					recNumStart = fileHead.efSize.recInfo[0];
					
					}
				}
				else if((fileHead.FileType&0x07) == LINTYPE)
				{
					recNumStart = G_CurrentRecNum[G_ChannelNum] + step;  //search函数保证了 G_CurrentRecNum[G_ChannelNum] == fileHead.RecNumber||0x01时的错误返回
				}
				else                                                     //CF
				{
					//视G_CurrentRecNum[G_ChannelNum])为物理记录号，得到对应的逻辑号
						recNumStart = G_CurrentRecNum[G_ChannelNum];
				
					if(step == 0x01)                                   //forward                   
					{
						recNumStart = ((recNumStart == (fileHead.efSize.recInfo[0]))?0x01 : (recNumStart +1));	
					}
					else                                               //backward
					{
						recNumStart = (recNumStart == 0x01?(fileHead.efSize.recInfo[0]) : (recNumStart-1));
					}
				}
			}
			searchString = &RecData[2];
	
			length = P3 -0x02;
		}
		else									//PROPRIETARY, FUNCTION NOT SUPPORT
		{
			FunctionNotSupport();				//6A 81
			return;
		}
	}
#endif

	if(G_CommandMode == Command_Mode_2G)
	{
		tag = SearchStarting(recNumStart, fileHead.efSize.recInfo[0], step);

		 tempBuf[0] = tag;
	}
#ifndef SingleSIM
	else	//Command_Mode_3G
	{
		tag = SearchContaining(step,recNumStart,offset,searchString,length,tag);
	}
#endif

	if(tag == 0x00)
	{
		if(G_CommandMode == Command_Mode_2G)
		{
			 FileNotFound();                           //94 04
		}
#ifndef SingleSIM
		else
		{
	    	AddressInvilid();							//6A 83
		}
#endif
		return;
	}			
	

	G_CurrentRecNum[G_ChannelNum] = tempBuf[0];


	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在A2指令中，只有BIT0应该被置位。
	
	if(G_CommandMode == Command_Mode_2G)
	{
		if((( P2 >> 4 )&0x0F) == 0x01)
		{
			RecData[0] = tempBuf[0];

			CommandFinish(0x01);
		}
		else
		{ 
		    CommandFinish(0x00);
		}   			             
	}
#ifndef SingleSIM
	else
	{
		memcpy(&RecData[0],&tempBuf[0],tag);

		CommandFinish(tag);							//USIM 用61 XX 代替 SIM 中的 9F XX  
	}
#endif
}
/**
 * @brief 		      IncreaseCommand:This function add the value to the last increase/update record of the current CF
 *
 * @command format 	   A0 32 00 00 03 /0X 32 00/SFI 00 lgth(1~127bytes)
 *
 * @Returns		       void
 *
 * @history 	       
 */
void IncreaseCommand(void)
{
	EFileHead xdata  cyclicEf;
	BYTE idata result,need_commit;
	BYTE idata tag,fistNum,secondNum;
	WORD xdata totalValue;
	BYTE idata middleValue,RecIndicator;

	 if((G_CommandMode == Command_Mode_3G)&&((CLA&0xFC) != 0x80))	  		//3G模式下,CLA必须为8X
	{
		//ClaError();
		InsError();	 				//其它卡商(恒宝)做法
		return;
	}
	
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在32指令中，只有BIT0应该被置位。

	if(P2 != 0x00)
	{
		P1P2Error();                                              //6B 00
		return;
	}                                                          
	 
	if ((P3>0x7F)||(P3==0x00))
	{
		P3Error();
		return;	
	}         

	if (G_CommandMode == Command_Mode_2G)
	{		
	 	result = IF_IncreaseCommand();
		
		if(result == FALSE)
		{
			return;
		}
		
		if(P1 != 0x00)
		{
			P1P2Error();  		       //6B 00
			                                            
			return;
		}
		
	}
 #ifndef SingleSIM
	if((P1&0xE0)==0x80)
	{	
		P1 = P1 & 0x1F;
		
		if((P1 ==0x1F)||(P1 == 0x00))  //SFI for 01 - 30
		{
			P1P2Error();

			return;
		}	
	}
	else if(P1 != 0x00)
	{
		P1P2Error();

		return;
	}
	else
	{;}
 #endif

	tag = ComputeAll(A_IncreaseFile,P1,3);
		
	if(tag == FALSE) 
	{
		return ;
	}

	ReadEeprom((BYTE *)&cyclicEf.FileId,G_CurrentEfAddr[G_ChannelNum],sizeof(EFileHead));

    RecIndicator = getCycleFPoint(G_CurrentEfAddr[G_ChannelNum]);


#ifndef SingleSIM	
 	if((G_CommandMode == Command_Mode_3G)&&(cyclicEf.efSize.recInfo[1] > 0x7F))
	{
    	P3Error();

   		return;
	}        
#endif
		
	ReceiveData();

	memcpy((BYTE *)&RecData[0x80],(BYTE *)&RecData[0],P3);

	ReadEeprom((BYTE *)&RecData[0],getFileContentAddr(G_CurrentEfAddr[G_ChannelNum]) + (RecIndicator-1)*cyclicEf.efSize.recInfo[1]+1,cyclicEf.efSize.recInfo[1]);//读取当前记录指针所指记录值

	fistNum = cyclicEf.efSize.recInfo[1]<=P3?cyclicEf.efSize.recInfo[1]:P3;

	secondNum = cyclicEf.efSize.recInfo[1]>P3?cyclicEf.efSize.recInfo[1]:P3;

	middleValue = 0x00;

	for(tag = 1;tag <= fistNum; tag++)
	{   
		totalValue = middleValue + RecData[cyclicEf.efSize.recInfo[1]-tag] + RecData[0x80-tag+P3] ;

		RecData[cyclicEf.efSize.recInfo[1]-tag] = 0xFF & totalValue;

		middleValue = (totalValue >> 8) & 0x01;
	}
 	for(tag = fistNum;tag < secondNum;tag++) 
	{
		fistNum  = 0x80 + secondNum - 1 - tag;
		 
		if(cyclicEf.efSize.recInfo[1] >=P3)	
		{
			fistNum -=  0x80;
		}
	
		totalValue = middleValue  +  RecData[fistNum ] ;

		RecData[fistNum ] = 0xFF & totalValue;

		if(cyclicEf.efSize.recInfo[1]<P3)
		{
			if(RecData[fistNum ]>0x00)				//max value reached
			{
				MaxReached();
				return;
			}
		}		
		middleValue = (totalValue >> 8) & 0x01;
	}
 	

	if( middleValue == 0x01 )
	{
		MaxReached();												//98 50
		return;
	}                                                   

	memcpy((BYTE *)&RecData[cyclicEf.efSize.recInfo[1]],(BYTE *)&RecData[0x80],P3);

	if(RecIndicator == 1)
	{
		RecIndicator = cyclicEf.efSize.recInfo[0];
	}
	else
	{
		RecIndicator = RecIndicator - 1;
	}

	G_CurrentRecNum[G_ChannelNum] = 1;
	
	need_commit = IF_BeginTransaction();
	
	tag = WriteLFBuf(G_CurrentEfAddr[G_ChannelNum],RecIndicator,0,(BYTE *)&RecData[0],cyclicEf.efSize.recInfo[1]);

	//SetEepromEX(getFileContentAddr(G_CurrentEfAddr[G_ChannelNum]),RecIndicator,1);
	//这里调用VMSetBuf（memFillAtomic，原子写），不能调用SetEepromEX（nvmFillNoAtomic，非原子写），解决循环文件断电问题,by xwh 20220530
	VMSetBuf(getCycleFPointAddr(G_CurrentEfAddr[G_ChannelNum]),RecIndicator,sizeof(BYTE));
	
	if(tag == FALSE)
	{
		MemoryError();													//65 81/92 40
		IF_AbortTransaction(need_commit);
		return;
	}
	

	IF_CommitTransaction(need_commit); 

	
	//G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在32指令中，只有BIT0应该被置位。	

	CommandFinish(cyclicEf.efSize.recInfo[1] + P3);
}

/**
 * @brief 		      This function  invalidates  or Rehabilidate the EF
 *
 * @command format 	   A0 04 00 00 00 /0X 04 mode 00 data
 * @command format 	   A0 44 00 00 00 /0X 44 mode 00 data
 *
 * @Returns		       void
 *
 * @history 	       
 */
void FileStatusHandler(void)
{
	BYTE idata fileStatus;
	BYTE idata i,tempTag;
	BYTE idata fileAccStatus,Status;
	UWORD xdata fileAddr;
	WORD  xdata fileName;
	PathInfo xdata filePathInfo;
	UWORD xdata  currentEfHeadAddr;

	
	if(G_CommandMode == Command_Mode_2G)
	{
		if(P1P2P3Check(0x00,FALSE) ==FALSE)
		{
			return ;
		}
	}
#ifndef SingleSIM
	else										//Command_Mode_3G
	{
		if(P2 != 0x00)  
		{
			P1P2Error();						//6B 00
			return;
		}
	
		BackFilePath(&filePathInfo);	     	//备份当前路径	

		ReceiveData();
	}
#endif
	
	if(P1 == 0x00)                                   
	{
#ifndef SingleSIM
		if((P3 != 0x00)&&(P3 != 0x02))
		{
			P3Error(); 								//6700
			return ;
		}
#endif		
		if((P3 == 0x00)&&(G_CurrentEfAddr[G_ChannelNum] == FALSE))
		{
			NoEfSelected();				  			//69 86
		
			return;
		}
#ifndef SingleSIM		
		if(P3 == 0x02)
		{
			fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];

		   	fileAddr = SelectFile(G_CurrentDfAddr[G_ChannelNum],fileName);

		   	if(fileAddr != FALSE)
		   	{
				tempTag = GetFileHeadByte(fileAddr,F_Type_Offset);				//路径选择判断文件类型
				
				if(tempTag == DFTYPE)
				{
					NoEfSelected();											//69 86									
					return;
				}
				
				SetChannel(fileAddr,fileName,FALSE);
		   	}
			else
			{
			 	FileNotFound();												//6A 82
				return;
			}
		}
#endif		                                                                    
	}
#ifndef SingleSIM
	else if((P1==0x08)||(P1==0x09))
	{
		if((P3 == 0x00)||((P3%0x02) == 0x01))
		{
			P3Error(); 														//send 6700
			return ;
		}

		fileName = (((WORD)RecData[0]<<8)&0xFF00) + RecData[1];
		
		if(P1 == 0x08)
		{
			if(fileName == 0x3F00)   					//IN case of select path from MF ,the terminal shall not use the FID of MF at the begining of the path
			{
				ConditionSatisfied();					// 69 85
				return;
			}
			
		 	fileAddr = E_FILE_START;
		}

		if(P1 == 0x09)
		{
			if(fileName == 0x7FFF)						// In the case of "select by path from current DF", the terminal shall not use the special file-ID '7FFF' at the beginning of the path.TS 102.221 P55.2010-03-10
			{
				ConditionSatisfied();					// 69 85
				return;
			}
			
		 	fileAddr = G_CurrentDfAddr[G_ChannelNum]; 
		}

		if((fileName == G_CurrentDfName[G_ChannelNum])&&(P1 == 0x09))		// In the case of "select by path from MF" or "select by path from current DF", the terminal shall not use the file identity of the current DF.TS 102.221 P55.2010-03-10
		{
			FileNotFound();													//6A 82,restriction 3 also need consider - 2010-03-23
			
			return;
		}		
			
		for(i=0x00;i<P3;i=i+2)
		{
			fileName = (((WORD)RecData[i]<<8)&0xFF00) + RecData[i+1];

			if(fileName == 0x7FFF)
            {
				fileName = G_CurrentAdfName[G_ChannelNum];
            }
			
		   	fileAddr = SelectFile(fileAddr,fileName);

		   	if(fileAddr != FALSE)
		   	{
				SetChannel(fileAddr,fileName,FALSE);
		   	}
			else
			{
			 	FileNotFound();												//6A 82
				return;
			}
		}
	}
	else
	{
		P1P2Error();
		return;
	}
#endif
	if(INS == 0x04)
	{
		fileAccStatus = A_DeactivateFile;
		Status = FILE_INVALIDATE;
	}
	else
	{
		fileAccStatus = A_ActivateFile;
		Status = FILE_REINVALIDATE;
	}

	tempTag = AccessCondition(G_CurrentEfAddr[G_ChannelNum],fileAccStatus);       //access condition

	if(tempTag == FALSE)
	{
#ifndef SingleSIM
		if(G_CommandMode == Command_Mode_3G)
		{
			ResumeFilePath(&filePathInfo);
		}
#endif		
		SecuritySatisfied();			 					//send 69 82
			
		return;
	}                                    
       
	currentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);

    fileStatus = GetFileHeadByte(currentEfHeadAddr, F_Status_Offset);
	
	//fileStatus = GetFileHeadByte(G_CurrentEfAddr[G_ChannelNum], F_Status_Offset);

	if((fileStatus&0x01) == Status)  						//已失效的文件只允许执行select和activate命令
	{
		if(G_CommandMode == Command_Mode_2G)
		{
			InvalidationContradiction();					//send 98 10
			
			return;
		}
#ifndef SingleSIM
		else
		{
			if(INS == 0x04)
			{
				ResumeFilePath(&filePathInfo);

				SelectedFileInvalid();
				
				return;
			}
		}
#endif		
	}
     
	if(fileAccStatus == A_DeactivateFile)
	{
		//tempTag =  AccessEepromEX(currentEfHeadAddr + F_Status_Offset,NULL,fileStatus&0xFE,sizeof(BYTE), (EEP_Set)|(EEP_NoSec));	 //EEP_NoSec模式可以写安全数据区，因为这两条指令的使用频率比较低
		tempTag = SetEepromEX(currentEfHeadAddr + F_Status_Offset,fileStatus&0xFE,sizeof(BYTE));
	}
	else
	{             
		//tempTag =  AccessEepromEX(currentEfHeadAddr + F_Status_Offset,NULL,(fileStatus | FILE_REINVALIDATE),sizeof(BYTE), (EEP_Set|(EEP_NoSec)));
		tempTag = SetEepromEX(currentEfHeadAddr + F_Status_Offset,(fileStatus | FILE_REINVALIDATE),sizeof(BYTE));
	}

	if(tempTag ==FALSE)
	{
		if(G_CommandMode == Command_Mode_2G)
		{
			MemoryError();													//92 40
		}
#ifndef SingleSIM
		else
		{
			ResumeFilePath(&filePathInfo);		

			NoInforStateOfMemoryUnchanged();								//64 00
		}
#endif		
		return;
	}
	
//#ifdef 	 NEWFDN
	 if((G_CommandMode == Command_Mode_2G)&&(G_CurrentDfName[G_ChannelNum] == DF_TELECOM )&&(G_CurrentEfName[G_ChannelNum] == EF_ADN))	 //激活/失效 EF_ADN文件 可能改变 2G下 FDN状态 需要同步到3G模式
	 {
		 tempTag = IsFdnServiceActiveInSIM();				  //获取fdn服务状态
		 
		 SimFdnSYNCtoUSIM(tempTag);		 					  //同步到USIM
	 }

//#endif  	
	
	  
	CommandFinish(0x00);   
}

/**
 * @brief 		       GetResponseCommand:This function is to the return the data's of the first command
 *
 * @command format 	   A0 C0 00 00 lgth /0X C0 00 00 lgth
 *
 * @Returns		       void
 *
 * @history 	       
 */
void GetResponseCommand(void)
{
	WORD idata sendNum;

	if(P3 == 0x00)
	{
		sendNum = 0x100;
	}
	else
	{
		sendNum = P3;
	}

 	if(P1P2Check() ==FALSE)
	{
		return;
	}

	//if(G_616CStatus[G_ChannelNum][0] ==  0x00)
	{
		if((SW1 !=0x61)&&(SW1 != 0x9F)&&(SW1!=0x62)&&(SW1 !=0x6C)&&(SW1 != 0x67)&&(SW1 != 0x9E))
		{
			TechnicalProblem();											//Send 6F 00

			return;
		}
	}

	if(sendNum > G_CurrentReponse[G_ChannelNum] )
	{
		if(G_CurrentReponse[G_ChannelNum] == FALSE)
		{
			TechnicalProblem();										//Send 6F 00
		}
		else
		{
			RepeatPreviousCmdHeader(G_CurrentReponse[G_ChannelNum]);    //6CXX						
		}
		return;
	}

	SendDataSingle();

	if(G_CommandMode == Command_Mode_2G)
	{
		CommandFinish(0x00);										  //Send 90 00/91xx
	}
	else															  //Command_Mode_3G
	{
		//if(G_616CStatus[G_ChannelNum][0] !=  0x9F)
		{
			if(G_CurrentReponse[G_ChannelNum]>sendNum)
			{
				memcpy(&RecData[0],&RecData[P3],G_CurrentReponse[G_ChannelNum]-P3);
			}
		}

		CommandFinish(G_CurrentReponse[G_ChannelNum]-P3);	          //Send 90 00/91xx
	}
}

/**
 * @brief 		       UsimManageChannelCommand:This function opens and closes logical channels
 *
 * @command format 	   0X 70 00(80)  00(01,02,03) 00
 *
 * @Returns		       void
 *
 * @history 	       
 */
#ifndef SingleSIM
void UsimManageChannel(void)
{
	BYTE idata i;
	
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在70指令中，只有BIT0应该被置位。
	
	if(((P1 != 0x00) && (P1 != 0x80))||(P2 > 0x03))
	{
		FunctionNotSupport();							//6A 81
		return;
	}

	if(P1 == 0x00)										//Open logical channel
	{
		if(P2 != 0x00)
		{
			P1P2Incorrect();							//6A 86
			return;
		}
		
		if(P3 != 0x01)
		{
			RepeatPreviousCmdHeader(0x01);				//6C 01
			return;
		}

		for(i=0x01;i<0x04;i++)
		{
			if(ValueBit(G_ChannelStatus,i) == FALSE)
			{
				break;
			}
		}

		if(i != 0x04)
		{			
			SetBit(G_ChannelStatus,i);

			RecData[0] = i;

			G_ChannelNum = i;

			if ((CLA&0x03) == 0x00)			 			//基本通道
			{
				SetChannel(E_FILE_START,DF_MF,FALSE);
			}
			else 										//否则, 新通道的"current DF"和"current ADF"与前一个通道相同.
			{
				SetChannel(G_CurrentDfAddr[(CLA&0x03)], G_CurrentDfName[(CLA&0x03)], FALSE);	//发出命令的逻辑通道的当前DF应被选为当前DF

				G_CurrentAdfName[G_ChannelNum] = G_CurrentAdfName[CLA&0x03]; 
			}
						   	
			SendData();

			CommandFinish(0x00);			
		}
		else											//通道已经全部打开
		{
			FunctionNotSupport();						//6A 81 
		}
	}    
	else                                                //关闭逻辑通道
	{
		if(P3 != 0x00)                      			//close channel not data Present
		{
			P3Error();
			return;
		}
		if(P2 == 0x00)                        			//通道0 总是有效的，并且不可以被关闭
		{
			FunctionNotSupport();						//6A 81
			return;
		}
		
		if(ValueBit(G_ChannelStatus,P2) == FALSE)
		{
			WarningNoInformationGiven();				//62 00
			return;		
		}

		ClearBit(G_ChannelStatus,P2);

		CommandFinish(0x00);
	} 
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在70指令中，只有BIT0应该被置位。
}
#endif
/**
 * @brief 		       UsimGetChallengeCommand:This function is used to create a random number
 *
 * @command format 	   0X 84 00  00 notpresent
 *
 * @Returns		       void
 *
 * @history 	       
 */
void UsimGetChallenge(void)
{
	if(P1P2P3Check(0x00,TRUE) ==FALSE)  
	{
		return;
	}
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在84指令中，只有BIT0应该被置位。
	
	Random_Number_Generator_Get((BYTE *)&RecData[0],P3);
	
	if(P3 == 0x10)
	{
		G_SuperKeyAccess = SuperKeyAccess_GetChallange;   //当前正在获取验证SuperKey的随机数
		
		
		memcpy(&G_SuperKeyRamdomBuf[0],&RecData[0],P3);	
    //memcpy(&RecData[0x20],&RecData[0],P3);			
	
	}
	
	SendData();
	
//	G_COMMANDPATH =G_COMMANDPATH&0x01; //防止擦写失误，对APDU指令序列进行检查，在84指令中，只有BIT0应该被置位。
	
	CommandFinish(0x00);
}

/**
 * @brief 	: 该函数用于判断当前EF文件的类型、状态、权限是否符合特定条件
 *
 * @param 	: action:A_ReadFile,A_UpdateFile,A_IncreaseFile
 *			: nByShortFID: 是否通过短文件标示查找文件(0:否，非0 ：是)
 *		    : nfile: OperateType: 0x00:Record 0x01:Binary,0x02:BER-TLV,0x03:Increase 0x04: Seek
 *
 * @Returns : TRUE/FALSE
 *
 * @Coder:             		Xiaoyh 
 *
 * @history 	       
 */
BYTE ComputeAll(BYTE action, BYTE nByShortFID,BYTE nfile)
{
	BYTE idata tag;	
	BYTE idata fileType,fileStatus;
	UWORD xdata fileAddr;
	UWORD xdata currentEfHeadAddr;

#ifndef SingleSIM	
	if(nByShortFID != 0)//用short file ID取文件
	{
		fileAddr = SelectFileByShortId(G_CurrentDfAddr[G_ChannelNum],nByShortFID);
	}
	else
#endif
	{
		fileAddr = G_CurrentEfAddr[G_ChannelNum];
	}
	
	if(fileAddr == FALSE)
	{		
		NoEfSelected();											//69 86

		return FALSE;
	} 
	
	fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

	currentEfHeadAddr = SearchFile(G_CurrentDfAddr[G_ChannelNum], G_CurrentEfName[G_ChannelNum]);			

	fileStatus	= GetFileHeadByte(currentEfHeadAddr,F_Status_Offset);
	
	//fileStatus	= GetFileHeadByte(fileAddr,F_Status_Offset);
	
	tag= TRUE;
	
	switch(nfile)		//判断文件类型
	{
		case 0:
			if((fileType != LINTYPE)&&(fileType != CYCTYPE )) //线形记录文件或者循环记录文件
			{
				tag =FALSE;
			}		
			break;
		case 1:
			if(fileType != TRANTYPE)		//透明文件
			{
				tag =FALSE;
			}		
			break;
		case 3:
			if(fileType != CYCTYPE )	//循环记录文件
			{
				tag =FALSE;
			}	
			break;	
		case 4:
			if(fileType != LINTYPE)        // 2G 为线形文件，3G为线形文件或循环文件              
			{
				if((G_CommandMode == Command_Mode_2G) ||((G_CommandMode == Command_Mode_3G)&&(fileType != CYCTYPE)) )
				{
					tag =FALSE;
				}
			}  	
			break;
		default:		
			break;	
	}

	if(tag == FALSE)
	{
		FileStructureError();           //69 81
		
		return FALSE;
	}

	if((fileStatus&F_READ_UPDATA_ENBALE) == 0x00)  //失效文件时，不可读写
	{
		if((fileStatus&0x01) == FILE_INVALIDATE)		//判断文件是否有效
		{
			if(G_CommandMode == Command_Mode_2G)
			{
				InvalidationContradiction();                                    //98 10
			}
#ifndef SingleSIM
			else
			{
				ReferencedDataInvalid();					//69 84
			}
#endif
			return FALSE;
		}
	}

   	tag = AccessCondition(fileAddr,action);		//判断访问条件是否满足

   	if( tag == FALSE ) 
	{
		SecuritySatisfied();
		
		return FALSE;
	}    	

	return TRUE;
}

