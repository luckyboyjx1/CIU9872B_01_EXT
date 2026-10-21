/*

 * Copyright (c) 2005,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EM ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V5.01

 * USIM AND SIM  RELEASE:2005-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 
			V1.1: 修改函数RepeatPreviousCmdHeader，对为2G模式时，返回67XX
			 	  修改函数SendResponse ，当上一条响应不是GetResponse的命令响应对时，清除G_CurrentResponse的值 @20110808 by  xuyz
			V1.2: 修改函数CommandFinish, 当定义SingleSIM时，选择文件的返回值为61XX	 @2012/1/4	by liuchen	
			V1.3  修改函数SendResponse ，增加00A4 0304XX中的特殊处理 @20130530 by xuyz Version: 1.14->1.15
			V1.4  增加函数P3ReturnRightLength2G,2G模式下A0F2指令P3长度有误时返回67XX  	@20171206 by  yangjian
			V1.5  修改函数CommandFinish,增加G_ResponseStatus为1时的处理,兼容国网RFM应用       @20180510 by  yangjian


 */ 
#include "uicc_main.h"
#include "interface.h"
#include "fileSystem.h"
#include "fileAction.h"


void GlobalSetStatus(WORD sw1sw2)
{
	SW1 = ((sw1sw2>>8)&0xFF);

	SW2 = (sw1sw2&0xFF);
}

#ifndef SingleSIM
void WarningNoInformationGiven(void)
{
	SW1 = 0x62;
	SW2 = 0x00;
}

void SelectedFileInvalid(void)
{
	SW1 = 0x62;
	SW2 = 0x83;
}

void PinRetryCounter(BYTE retryLeft)
{
	SW1 = 0x63;
	SW2 = 0xC0 + (retryLeft & 0x0F);
}

void NoInforStateOfMemoryUnchanged(void)
{	
	SW1 = 0x64;
	SW2 = 0x00;
}
void FunctionNotSupport(void)
{
	SW1 = 0x6A;
	SW2 = 0x81;
}
void RecordNotFound(void)
{
	SW1 = 0x6A;
	SW2 = 0x83;	
}
void P1P2Incorrect(void)
{
	SW1 = 0x6A;
	SW2 = 0x86;
}
#endif



void MemoryError(void)
{
#ifndef SingleSIM
	if (G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x65;
		SW2 = 0x81;
	}
	else
#endif
	{
		SW1 = 0x92;
		SW2 = 0x40;	
	}
}

void P3ReturnRightLength2G(BYTE RightLength)
{
	SW1 = 0x67;
	SW2 = RightLength;
}

void P3Error(void)
{
	SW1 = 0x67;
	SW2 = 0x00;
}

void ChannelNotSupport(void)
{
	SW1 = 0x68;
	SW2 = 0x81;
}

void FileStructureError(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x69;
		SW2 = 0x81;
	}
	else
#endif
	{		
		SW1 = 0x94;
		SW2 = 0x08;
	}	
}

/**
 * @brief 		   	   SecuritySatisfied:file access condition fail error
 *
 * @command format	
 *
 * @Returns		      void
 *
 * @Coder:             		Xiaoyh 
 *
 * @history 	       
 */
void SecuritySatisfied(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x69;
		SW2 = 0x82;
	}
	else
#endif
	{	
		SW1 = 0x98;
		SW2 = 0x04;
	}
}

void SecurityError(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x69;
		SW2 = 0x82;
	}
	else
#endif
	{	
		SW1 = 0x98;
		SW2 = 0x04;
	}
}


void PinBlocked(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x69;
		SW2 = 0x83;
	}
	else
#endif
	{
		SW1 = 0x98;
		SW2 = 0x40;
	}
}

void ReferencedDataInvalid(void)
{
	SW1 = 0x69;
	SW2 = 0x84;	
}

void ConditionSatisfied(void)
{
	SW1 = 0x69;
	SW2 = 0x85;
}

void NoEfSelected(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x69;
		SW2 = 0x86;
	}
	else
#endif
	{
		SW1 = 0x94;
		SW2 = 0x00;
	}
}

void FileNotFound(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x6A;
		SW2 = 0x82;
	}
	else
#endif
	{		
		SW1 = 0x94;
		SW2 = 0x04;
	}
}


void ReferencedDataNotFound(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x6A;
		SW2 = 0x88;
	}
	else
#endif
	{
		SW1 = 0x98;
		SW2 = 0x02;
	}
}

void FileExist(void)
{
	SW1 = 0x6A;
	SW2 = 0x89;
}

void DfNameExist(void)
{
	SW1 = 0x6A;
	SW2 = 0x8A;
}

void P1P2Error(void)
{
	SW1 = 0x6B;
	SW2 = 0x00;
}

void RepeatPreviousCmdHeader(BYTE length)
{
	if(G_CommandMode == Command_Mode_2G)
	{
		SW1 = 0x67;
	}
	else
	{
		SW1 = 0x6C;
	}

	SW2 = length;

	G_CurrentReponse[G_ChannelNum] = length;
}

void InsError(void)
{
	SW1 = 0x6D;
	SW2 = 0x00;
}

void ClaError(void)
{
	SW1 = 0x6E;
	SW2 = 0x00;
}

void TechnicalProblem(void)
{
	SW1 = 0x6F;
	SW2 = 0x00;
}

void SimAppTkitBusy(void)
{
	SW1 = 0x93;
	SW2 = 0x00;
}

void P3InconsistentWithP1P2(void)
{
	SW1 = 0x6A;
	SW2 = 0x87;
}
void IncorrentParameter(void)
{
	SW1 = 0x6A;
	SW2 = 0x80;
}
void AddressInvilid(void)
{
#ifndef SingleSIM
	if(G_CommandMode == Command_Mode_3G)
	{
		SW1 = 0x6A;
		SW2 = 0x83;	
	}
	else
#endif
	{
		SW1 = 0x94;
		SW2 = 0x02;
	}
}

void InvalidationContradiction(void)
{
	SW1 = 0x98;
	SW2 = 0x10;
} 

void MaxReached(void)
{
	SW1 = 0x98;
	SW2 = 0x50;
}

void AuthenticateError(void)
{
	SW1 = 0x98;
	SW2 = 0x62;
}


void NoEnoughSpace(void)
{
	SW1 = 0x6A;
	SW2 = 0x84;
}

void GSMSecurityContextNotSupported(void)
{
	SW1 = 0x98;
	SW2 = 0x64;
}



void CommandFinish(BYTE length)
{
	UWORD idata porDataFileAddr;
	
	IF_ApduFinish();
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)	
	if(G_CurrentRFMResponse != FALSE)		 //	G_CurrentRFMResponse该全局变量设置的意义在于，C2以CommandFinish结束，而此时如果有RAM APDU/RFM APDU需要返回响应数据，则无法利用CommandFinish(00)
	{
		porDataFileAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);

		if(porDataFileAddr != FALSE)
		{
			ReadTFBuf(porDataFileAddr,0,&RecData[0],G_CurrentRFMResponse); 
		}

		if(G_CommandMode == Command_Mode_2G)
		{
			if((G_Post9EFlag&0x01) == 0x00)
			{
				SW1 = 0x9F;
			}
			else
			{
				SW1 = 0x9E;
			}
			
			SW2 = G_CurrentRFMResponse;

#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
			if((G_Deliver_T0_Submit & 0x02)==0x02)
			{
				G_CurrentReponse[G_ChannelNum] = G_CurrentRFMResponse;
			}
#endif
		}
		else
		{		
			if((G_Post9EFlag&0x01) == 0x01)	  	//按照102221,C.1.7,POR deliver error 应该在00C20000XX后返回“6200”，而不是在00C00000XX后返回“6200”，by xwh 20141128
			{
				GlobalSetStatus(0x6200);					
			}
			else
			{
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT			
				if((G_Deliver_T0_Submit & 0x02)==0x02)
				{
					SW1 = 0x61;
					SW2 = G_CurrentRFMResponse;
					G_CurrentReponse[G_ChannelNum] = G_CurrentRFMResponse;
				}
				else
#endif
				{
					GlobalSetStatus(0x9000);
				}
			}			
		}

		G_Post9EFlag  &= 0xFE;  //为方便以后扩展使用，CommandFinish中只使用到了bit0 ，则将bit0 清空即可

		if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
		{
			apdu_ApduSetOutgoingAndSend(5, G_CurrentRFMResponse);
		}
		else
		{
			G_CurrentReponse[G_ChannelNum] = G_CurrentRFMResponse;
		}

		G_CurrentRFMResponse = FALSE;

		////SW1SW2=9000的情况下,可能需要发送主动式命令,因此这里不退出,在下面的代码中将SW1SW2修改成91XX,by xwh 20191205
		if((SW1 == 0x90)&&(SW2 == 0x00))
		{

		}
		else
		{
			return;
		}
	}
#endif
	if(G_ResponseStatus == 1)
	{
		G_ResponseStatus = 0;

		G_CurrentReponse[G_ChannelNum] = SW2;
	}	
	else if((STKPerform == TRUE)&&(length == 0x00)&&((CLA&0x03) == 0x00))							//STK BUFFER HAVE DATA TO SEND 
	{
		SW1 = 0x91;

		SW2 = ResData[2]>0x7F?ResData[2]+3:ResData[2]+2;
	}
	else											//PHASE 2 COMMAND FINISH  
	{
		if( length == 0x00)
		{
			SW1 = 0x90;								//PHASE 2 COMMAND FINISH AND NO DATA WANT TO SEND
			SW2 = length;
			G_CurrentReponse[G_ChannelNum] = 0x00;
		}
 		else										//PHASE 2 COMMAND FINISH AND HAVE DATA WANT TO SEND
		{
			if(G_CommandMode == Command_Mode_2G)
			{
				SW1 = 0x9F;
				SW2 = length;
					
				if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
				{
					apdu_ApduSetOutgoingAndSend(5, length);
				}
			}
			else
			{				
				SW1 = 0x61;
				SW2 = length;
				
				if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
				{
					apdu_ApduSetOutgoingAndSend(5, length);

					GlobalSetStatus(0x9000);
				}				
			}

			if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)
			{
				G_CurrentReponse[G_ChannelNum] = length;
			}
		}

		//SW2 = length;
	}												//THE NUMBER OF DATA OF BUFFER WANT OT SNED
}

void SendResponse(void)
{
	if( (SW1 != 0x61)&&(SW1 != 0x9F)&&(SW1!=0x62)&&(SW1 != 0x9E))
	{
		if((INS == 0xC0)&&((SW1 == 0x6C)||(SW1 == 0x67))&&(SW2 != 0x00))
		{
			//do nothing for GetResponse
		}   
		else
		{
			G_CurrentReponse[G_ChannelNum] = 0x00;
		}
	}
	//if(JcvmConfig_GetOPerator() != OPERATOR_CHINAUNICOM)
	//{
	//	if((INS == 0xA4)&&(SW1 == 0x6C))  //针对选择命令中的 00 A4 03 04 XX的情况
	//	{
	//		G_A4Control = SW2;
	//	}
	//	else
	//	{
	//		G_A4Control = FALSE;
	
	//	}
	//}
	GlobalSetStatus((SW1<<8)|SW2);	   //G_CommandStatus = 	StatusHaveSet，在该状态下才会发送状态字
		
	GlobalSendResponse();			  //发送状态字后 G_CommandStatus = WaitingCommand;
	
}

/**
 * @brief 		   	   P1P2Check:phase 2, P1 or P2 is zero or not.
 *
 * @command format	
 *
 * @Returns		      FALSE/TRUE
 *
 * @Coder:             		Xiaoyh 
 *
 * @history 	       
 */
BYTE P1P2Check(void)
{
	if((P1 != 0x00) || (P2 != 0x00) )
	{
		P1P2Error();													//Send 6B 00
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief 		   	   P1P2Check:phase 2, P1 or P2 is zero ,judge the P3 is the same with value.
 *
 * @command format	
 *
 * @Returns		      FALSE/TRUE
 *
 * @Coder:             		Xiaoyh 
 *
 * @history 	       
 */
BYTE P1P2P3Check(BYTE value, BYTE nCondition)
{
	if(P1P2Check() ==FALSE) 
	{
		return FALSE;
	}
	
	if(nCondition == FALSE)
	{
		if( P3 != value )
	   {
			P3Error();													//Send   67 00
			
			return FALSE;
		}   
	}
	else
	{
		if( P3 == value ) 
		{
			P3Error();													//Send   67 00
			
			return FALSE;
		}
	}
	
	return TRUE;
}

/**
 * @brief 		   	   ChvContradiction:chv status error
 *
 * @command format	
 *
 * @Returns		      void
 *
 * @Coder:             		Xiaoyh 
 *
 * @history 	       
 */
void ChvContradiction(void)
{
	if(G_CommandMode == Command_Mode_2G)
	{
		SW1 = 0x98;
		SW2 = 0x08;
	}
#ifndef SingleSIM
	else
	{
		ReferencedDataInvalid();
	}
#endif	
}

void FileNoFind(void)
{
	SW1 = 0x94;
	SW2 = 0x04;
}


void FileInconsistent(void)
{
	SW1 = 0x94;
	SW2 = 0x08;
}

void CDMAError(void)
{
	SW1 = 0x98;
	SW2 = 0x34;
}


void KeyCheckSumNotValid(void)
{
	SW1 = 0x94;
	SW2 = 0x85;
}
