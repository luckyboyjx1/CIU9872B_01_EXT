/*

 * Copyright (c) 2005,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EM ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V5.01

 * USIM AND SIM  RELEASE:2005-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
			V1.1: 修改函数RepeatPreviousCmdHeader，对为2G模式时，返回67XX
 				  修改函数SendResponse ，当上一条响应不是GetResponse的命令响应对时，清除G_CurrentResponse的值 @20110806 by  xuyz
		    V1.2  修改函数SendResponse函数，兼容A4指令的处理 @20130922 wlj

    ***********************************FOR uicc apdu**************************************************************************************
			V1.3   修改函数CommandFinish，增加POST操作@20151201 by xuyz

 */ 
//#include "main.h"
#include "uicc_main.h"
#include "uicc_interface.h"
#include "T0ApduImpl.h"
//#include "transaction.h"
#if UICC__FUNCTION__STKAPI  //支持Toollkit功能
#include "ToolkitRFM.h"
#include "fileSystem.h"
#include "fileAction.h"
#include "STKRemoteAppManagement.h"
#include "STKCATRuntimeEnviorment.h"
#include "STKFileView.h"
#include "ToolkitAPIPublic.h"

#if UICC__FUNCTION__UICCAPI
#include "UICCToolkitAPI.h"
#endif

#endif

void GlobalSetStatus(WORD sw1sw2)
{
	SW1 = ((sw1sw2>>8)&0xFF);

	SW2 = (sw1sw2&0xFF);
}

void SelectedFileInvalid(void)
{
	GlobalSetStatus(0x6283);
}

void NoInforStateOfMemoryUnchanged(void)
{
	GlobalSetStatus(0x6400);
}


void PinRetryCounter(BYTE retryLeft)
{
	SW1 = 0x63;
	SW2 = 0xC0 + (retryLeft & 0x0F);
}


void MemoryError(void)
{
	if (G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6581);
	}
	else
	{
		GlobalSetStatus(0x9240);
	}
}

void P3Error(void)
{
	GlobalSetStatus(0x6700);
}

void FileStructureError(void)
{
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6981);
	}
	else
	{
		GlobalSetStatus(0x9408);
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
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6982);
	}
	else
	{
		GlobalSetStatus(0x9804);
	}
}

void PinBlocked(void)
{
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6983);
	}
	else
	{
		GlobalSetStatus(0x9840);
	}
}

void ReferencedDataInvalid(void)
{
	GlobalSetStatus(0x6984);
}
void ConditionSatisfied(void)
{
	GlobalSetStatus(0x6985);
}

void NoEfSelected(void)
{
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6986);
	}
	else
	{
		GlobalSetStatus(0x9400);
	}
}

void IncorrentParameter(void)
{
	GlobalSetStatus(0x6A80);
}

void FunctionNotSupport(void)
{
	GlobalSetStatus(0x6A81);
}

void FileNotFound(void)
{
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6A82);
	}
	else
	{
		GlobalSetStatus(0x9404);
	}
}

void NoEnoughSpace(void)
{
	GlobalSetStatus(0x6A84);
}
#if UICC__FUNCTION__BERTLVCmd
void P1P2Incorrect(void)
{
	GlobalSetStatus(0x6A86);
}
#endif
void P3InconsistentWithP1P2(void)
{
	GlobalSetStatus(0x6A87);
}

void ReferencedDataNotFound(void)
{
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6A88);
	}
	else
	{
		GlobalSetStatus(0x9802);
	}
}

void FileExist(void)
{
	GlobalSetStatus(0x6A89);
}

void DfNameExist(void)
{
	GlobalSetStatus(0x6A8A);
}

void P1P2Error(void)
{
	GlobalSetStatus(0x6B00);
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
	GlobalSetStatus(0x6D00);
}

void ClaError(void)
{
	GlobalSetStatus(0x6E00);
}

void TechnicalProblem(void)
{
	GlobalSetStatus(0x6F00);
}

void SimAppTkitBusy(void)
{
	GlobalSetStatus(0x9300);
}

void AddressInvilid(void)
{
	if(G_CommandMode == Command_Mode_3G)
	{
		GlobalSetStatus(0x6A83);
	}
	else
	{
		GlobalSetStatus(0x9402);
	}
}

void InvalidationContradiction(void)
{
	GlobalSetStatus(0x9810);
} 

void MaxReached(void)
{
	GlobalSetStatus(0x9850);
}

void AuthenticateError(void)
{
	GlobalSetStatus(0x9862);
}

void GSMSecurityContextNotSupported(void)
{
	GlobalSetStatus(0x9864);
}
/*
*#function    :CommandChangeSW
*
*@description :修改SW状态字 CommandFinish子函数(某些情况只需要修改状态字 不需要调用IF_ApduFinish)则可以调用这个函数
*              :将CommandFinish的部分代码提出来的原因是因为 开机重入IF_ProcessSystemProactiveCommand里面调用CommandFinish 会造成更新6F54等打上两次标志
*@param       :length:CommandFinish的参数
*
*return       ：将要发送给终端的数据长度
*
*@author      :LCZ
*
*@hisotry     :V1.0 :create by LCZ@20200628
*/
void CommandChangeSW(BYTE length)
{
	UWORD xdata porDataFileAddr;

#if UICC__FUNCTION__STKAPI
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

    if((STKPerform[0] == TRUE)&&(length == 0x00))							//STK BUFFER HAVE DATA TO SEND 
	{
		if((G_STKBusy != FALSE)&&(G_ChannelNum == (((G_STKBusy>>8)&0xFF)-1)))  
		{
			SW1 = 0x91;	//在STK 中重新发送新的APDU 命令的状态字时，如果是同一通道的，则继续发送91XX,否则发送0x9000

			SW2 = (ResData[2]>0x7F)?(ResData[2]+3):(ResData[2]+2);
		}
		else if((STKPerform[1]-1) == G_ChannelNum )	//兼容开机的STK 过程中如果收到其他通道的命令，各自处理其状态字
		{
			SW1 = 0x91;	  ////E:\N3_TEE\develop\branches\ChinaUnicomUicc\test\中国联通NFC-SWP测试工程集测试脚本20160321\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\ReEntry_test_工程集\SIMReEntry_07_09_01

			SW2 = (ResData[2]>0x7F)?(ResData[2]+3):(ResData[2]+2);
		}
		else	
		{
			GlobalSetStatus(0x9000);
		}
	}
	else											//PHASE 2 COMMAND FINISH  
	{
		if( length == 0x00)
		{
			GlobalSetStatus(0x9000);                          //PHASE 2 COMMAND FINISH AND NO DATA WANT TO SEND
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
	}												//THE NUMBER OF DATA OF BUFFER WANT OT SNED
}
/*
*#function    :CommandFinish
*
*@description :这里主要针对某个特殊情况，对当前响应数据的长度进行修改，该函数存放于IF_ApduFinish的后面，有疑问，请联系COS开发人员
*
*@param       :length:CommandFinish的参数
*
*return       ：将要发送给终端的数据长度
*
*@author      :xuyz
*
*@hisotry     :V1.0 :create by xuyz@20131223
*/
void CommandFinish(BYTE length)
{	
	IF_ApduFinish();

	CommandChangeSW(length);											
}


/*@function:    SendResponse                                            
 * 
 *@descrip:     发送状态字
 *
 *@see:	        TS_11.11             
 *                                                                                                                                     
 *@param:	    void
 *				                                                                                                         
 *@return:      void                                                                                                                                                                                                                                                                   
 * 
 *@author:      
 *  
 *@history:     V1.1---Modify by xwh 20170526,去掉对CacheFlush 的调用， 因为在GlobalSendResponse 中已经调用了  CacheFlush
 */
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
	//if(MOBILE_Operator != OPERATOR_CHINAUNICOM)
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
		GlobalSetStatus(0x9808);
	}
	else
	{
		ReferencedDataInvalid();
	}	

}
/*
*/
void P3ReturnRightLength2G(BYTE length)
{
	SW1 = 0x67;
	SW2 = length;
}

#if (UICC__AUTH__CDMA__CAVE || UICC__AUTH__CDMA__MD5)
void CDMAError(void)
{
	GlobalSetStatus(0x9834);
} 
#endif
