/*

 * Copyright (c) 2006,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EM ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V5.01

 * USIM AND SIM  RELEASE:2005-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件
 			
 			V1.1 添加宏S3FS9AX，适用于三星S3FS9BE/AF/AG/AV系列芯片

			V1.2 修改MAIN():增加接口函数IF_MainReceiveApdu();
			
			V1.3 修改函数HardWareInition，将ResumeData从SystemInit函数中移至此处，为防止当断电所操作的文件和3F00在同一页时，断电恢复不成功
			
			V1.4 修改在复位后收到第一条APDU命令时，将MF默认为当前选中的DF，且对状态字SW1 ，SW2进行初始化
			
			V1.5 根据联通脚本,添加sleep命令 -2010-03-23

			v1.6 增加在未创建MF时,校验COS的crc指令 modify by xyz@20100820 16:18

			v1.7 为兼容ST芯片在擦写异常时发‘3B’的冲突，增加 ST 宏控制  WJN 20101215 10:33

			v1.8 增加宏BERTLVCmd，使用BER-TLV命令功能必须定义此宏

			v1.9 将函数PpsProcess移动到底层，各平台根据芯片特性自行实现该过程 @20110727 by  xuyz
			
			V1.10 修改参数G_CurrentReponse[4]，其定义由BYTE改为WORD @20110808 by xuyz
			V1.11 增加全局变量G_A4Control ，该变量仅用于00A40304XX时，保存当前响应数据长度@20130530 by xuyz
			V1.12 修改函数SystemInit HardWareInition，对于全局变量G_CardStatus的赋值直接通过判断3F00是否存在来获取@20141019 by xuyz
			V1.13 修改函数SystemInit，去掉第一个FdnProgcess @20141010  by xuyz
			V1.14 修改函数SystemInit main，增加全局变量G_SuperKeyAccess的 初始化及使用情况@20141110 by xuyz
			      增加指令85
			V1.15 修改FDN处理流程，修改函数main SystemInit，对于复合卡片或SIM +ota3的卡片而言，只有在2G模式下才做FDN过程，此处移植平台的人员需要测试@20150212 by xuyz
			V1.16 修改函数main，增加支持华大芯片的稳定性方案     @20160621  by yangjian 
			V1.17 升级华大芯片稳定性方案,断电恢复放在收到第一条命令头后做         @20161012  by yangjian 
			
 */ 
 
 
#include    "uicc_main.h"
#include    "uicc_error.h"
#include    "gsmpublic.h"
#include    "string.h"
#include    "gsmpublic.h"
#include	"stkhead.h"
#include    "Authentication.h"
#include	"fileAction.h"
#include	"fileSystem.h"
#include 	"Interface_LTEFromBoot.h"
#include    "interface.h"
#include    "typedef.h"
#include    "eepromdefine.h"
#include    "profile.h"
#include    "APDU.h"
#include    "ToolkitRFM.h"
#include    "resume.h"
//#include "Transaction.h"
//#include "Interface_AnnexB.h"
#ifdef THC228B	
#include    "SC_SFR.h"		
#endif
#if GP__AMENDMENT__B__FOR_MASK	
#include "Interface_AnnexB.h"
#endif
//BYTE *			APDUBuffer;
//BYTE    xdata    G_APDU[5];
BYTE    xdata    G_StatusResponse[2];
WORD    xdata    G_CurrentAdfName[4];
WORD    xdata    G_CurrentDfName[4];
UWORD   xdata    G_CurrentDfAddr[4];
WORD    xdata    G_CurrentEfName[4];
UWORD   xdata    G_CurrentEfAddr[4];
BYTE    xdata    G_CurrentRecNum[4];
WORD    xdata    G_CurrentReponse[4];
BYTE	xdata	 G_CurrentSfi;              						
//BYTE    xdata    RecData[256];
BYTE    xdata    ResData[256];
BYTE    xdata    tempBuf[0x200]; //在global_variable定义
BYTE    xdata    G_ChannelNum;
BYTE    xdata    G_SystemStatus;
BYTE	xdata	 G_CardStatus;
BYTE    xdata    G_ChvStatus;			//PIN1 status
#if NATIVE_CARD_UICC
BYTE    xdata    G_KeyStatus;			//PIN2 status
#else
BYTE    xdata    G_LocalPinStatus[8];   //LocalPin Status 81~88
#endif
BYTE    xdata    G_AdmStatus;			//ADM status
BYTE    xdata    STKPerform;
BYTE    xdata    G_ChannelStatus;
BYTE    xdata    G_CommandMode;         //命令模式：2G/3G
BYTE    xdata    G_AppControl;			 //执行应用控制变量：bit1表示已执行开机情况下已执行SetupMenu
										//					bit2表示已更新7F20/6F7E, bit3表示已可以执行OTA应用
										//				    bit7表示重新复位，只有重新复位后，收到TerminalProfile指令才把bit8置1，兼容联通脚本
										//	                bit8表示收到了TerminalProfile指令，只有开机后产生的SetupMenu，bit1才置1
BYTE    xdata    G_STKBusy;			//G_STKBusy默认为0x00,表示没有进入STK流程
									//G_STKBusy==1表示产生了主动式命令
									//G_STKBusy==2表示已发出了主动式命令，但还没收到“14”响应
BYTE    xdata    G_SuperKeyAccess;  //为增加COS 安全性，只有该全局变量设置为true时，才允许读取权限为NEV 的文件和读E2
BYTE   xdata   G_SuperKeyRamdomBuf[0x10];		//在校验SuperKey时用于缓存产生的随机数,该随机数用于下一条指令的校验操作
BYTE 	xdata    G_ResponseStatus;

BYTE	xdata	 G_OpenControl;			//开机控制变量。
BYTE 	xdata    STKControl;
BYTE    xdata    G_A4Control;
BYTE    xdata    G_UnicomResponseLgth;
BYTE    xdata    G_FirstCommand;


BYTE    xdata    G_NetWorkMode;
BYTE    xdata 	G_FDNProcessed;
BYTE    xdata  	G_SetUpEventListSent;
WORD    xdata    G_DF_ID_USIM;
WORD    xdata    G_DF_ID_CSIM;

UWORD xdata G_CurPrivateMFFileStart;
UWORD xdata G_CurMFFileStart;    //当前Profile下的3F00起始地址
UWORD xdata G_CurBipReceiveAddrStart;


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SystemInit                                                                                                                   
                                                                                                                                                
  Input parameter:   void
                               
  Output parameter:  void                                                                                                               
                                                                                                                                                
  Description:       根据卡片的状态，对系统变量设置相应的状态
 
  Coder:               
  
  history:           V1.0 create file
                     V1.1 增加调用ReadPinStatus函数，by xwh 20220329		          

 -------------------------------------------------------------------------------------------------------------------*/    
void SystemInit_Variable(void)
{
	BYTE idata i;
		
    IF_InitSystem();

	//InitVariable();	
 
	G_ChannelNum = 0x00;

	G_AdmStatus = 0x00;

	G_NetWorkMode = CDMA_NETWORK;		//默认为C网
	
	G_A4Control = 0x00;

	G_UnicomResponseLgth = 0x00;
 
	G_FirstCommand=0x00;
	
	G_FindCH = 0x00;
	
    G_High_Update_NUM = 0x00;	
	G_CommandMode = Command_Mode_First;

	G_FDNProcessed = 0x00;

	for(i = 0x00;i<0x04;i++)
	{
		G_CurrentAdfName[i] = FALSE;
	}

	G_SystemStatus = FALSE;

    G_ChannelStatus = 0x01;		
	
	G_ResponseStatus = 0x00;
	
	G_Bank = 0x00;					

	G_USIMAlgorithmType = AlgorithmType_MilenageNORMAL;	 		//表示使用普通的Milenage算法
   	
  G_SuperKeyAccess = SuperKeyAccess_NoVerify;
	
	memset(G_SuperKeyRamdomBuf, 0x00,sizeof(G_SuperKeyRamdomBuf));

    STKPerform = FALSE;
	
#if NATIVE_CARD_UICC		
	SetNativeUICCCardState();
#else
	G_CurPrivateMFFileStart = UPCOSConfig_GetUPCOSPrivateFileSystemAddr();	//需要先初始化私有文件地址，防止未跑个人化选择的私有文件地址为随机值
	G_CurMFFileStart = GetMFFileStart();
	
#if COS_UPDATE_ONLINE_BY_NativeOS	
	G_CurBipReceiveAddrStart  = UPCOSConfig_GetUPCOSBIPModuleReceiveAddr();
#endif
#endif

#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
	G_CurrentRFMResponse = FALSE;
	
	InitVariableRemoteModule(); //RFM变量初始化
	
	InitRemoteManagevariable();
#endif


	if(G_CardStatus != D_CardEmpty)
	{
		ReadPinStatus();                                                      	//读PIN状态
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)		
		G_EnvelopHanderAddr = SelectPrivateTFFile(DF_Toolkit,EF_EnvelopeHandlerBuf);
#endif		
	}
	
	IF_MoveFlashResume();
}





/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 PresenceTCK                                                                                                                   
                                                                                                                                                
  Input parameter:  * atrString : 指向ATR的T0
                               
  Output parameter:  TRUE/FALSE                                                                                                               
                                                                                                                                                
  Description:		  判断所有的TDi 和T0，仅当T = 0 时不要TCK 其他的情况都要TCK

  Coder:               zxl
  
  Status:			          

 -------------------------------------------------------------------------------------------------------------------*/  
BYTE PresenceTCK(BYTE const * atrString)
{
	BYTE idata pos;
	BYTE idata offset;
	
	pos = 0x00;

	while((atrString[pos]&0x80) == 0x80)					//check all TDi and T0
	{
		offset = 0x01;
		
		if((atrString[pos]&0x40) == 0x40)
		{
			offset ++;
		}

		if((atrString[pos]&0x20) == 0x20)
		{
			offset ++;
		}

		if((atrString[pos]&0x10) == 0x10)
		{
			 offset ++;
		}

		pos = pos + offset;										//pos point to TDi

		if((atrString[pos]&0x0F) != 0x00)					    //if only T=0
		{
			return TRUE;
		}
	}

	return FALSE;											//When no TDi transmi, T=0
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SendAtrProcess                                                                                                                   
                                                                                                                                                
  Input parameter:   void
                               
  Output parameter:  void                                                                                                               
                                                                                                                                                
  Description: 发送ROM 或者ATR文件中的ATR

  			ATR文件内容为LV,V的值不包括TS和TCK

  Coder:               
  
  Status:			          

 -------------------------------------------------------------------------------------------------------------------*/    
/*void SendAtrProcess(void)
{
	BYTE  idata i,atrLen;
	BYTE  idata sendTCK,checkSum,mfStatus;				//TCK使T0到TCK的所有字符异或结果为0
	BYTE  idata temp[3];

	mfStatus = 0x00;			  		//表示无3F00
																
	atrLen = sizeof(ATRDefine);

	memcpy_G((BYTE *)&ResData[0],(BYTE CONST *)&ATRDefine[0],atrLen);
	
	IF_SendATR(atrLen);

	atrLen = atrLen + 2;
	
	ReadEeprom((BYTE *)&temp[0],E_FILE_START,sizeof(temp));

#ifdef LittleEndian
	if((temp[0]!=0x00)||(temp[1]!=0x3F)||((temp[2]&MFTYPE)!=MFTYPE))
#else
	if((temp[0]!=0x3F)||(temp[1]!=0x00)||((temp[2]&MFTYPE)!=MFTYPE))
#endif	
	{
		mfStatus = 0x00;
	}
	else
	{	
		mfStatus = 0x01;							//表示有3F00
	}

	SetATRTA1();

	sendTCK = PresenceTCK((BYTE *)&ResData[0]);
	
	IF_ATRChange();								//调用此接口函数,兼容ATR特殊需求

#ifdef SingleSIM
    for(checkSum = 0x00,i = 0x00;i < atrLen;i++)
	{
	 	checkSum = checkSum ^ ResData[i];

		SendByte(ResData[i]);
	}
    
   if (mfStatus == 0x01)
	{
        checkSum = checkSum ^0x07;
    
		SendByte(0x07);
	}
	else
	{
        checkSum = checkSum ^0x01;
        
		SendByte(0x01);	
	}
#else
	if (mfStatus == 0x01)
	{
		ResData[0x0D] |= 0x07;
	}
	else
	{
		ResData[0x0D] |= 0x01;	
	}

	for(checkSum = 0x00,i = 0x00;i < atrLen;i++)
	{
	 	checkSum = checkSum ^ ResData[i];

		SendByte(ResData[i]);
	}
#endif

	if(sendTCK == TRUE)
	{
		SendByte(checkSum);												    //TCK
	}
	
#ifdef THC228B	
		SCSYS = 0x40;																	//修改电气特性不通过问题,内部使用15MHz
#endif	
	
*/

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 CommandAttemper                                                                                                                   
                                                                                                                                                
  input parameter:   inslist                                          
  					                                                                                                         
  output parameter:   void                                                                                                         
                                                                                                                                                
  description:       COS的主控程序的命令调度部分功能
  
  Coder:             xuqs  			   
  
  Status:        

 -------------------------------------------------------------------------------------------------------------------*/    
void CommandAttemper(FLASH INSLIST *inslist)
{
	BYTE idata i;
	BYTE idata result;

	G_ChannelNum = CLA&0x03;

	result = IF_CommandAttemper();

	if(result == FALSE)
	{
		return;
	}

	if (G_CommandMode == Command_Mode_3G)
	{		
		if(ValueBit(G_ChannelStatus,G_ChannelNum) == FALSE)
		{
			ChannelNotSupport();								//通道不支持
	
			SendResponse();

			return;
		}
	}

	if (IF_MainReceiveApdu() == TRUE)
	{
		return;
	}
			
	for(i=0;;i++)
	{
	    if((inslist[i].ins == INS)||(inslist[i].ins == EOL))
	    {
			result = IF_CmdINS(inslist[i].ins);

			if(result == TRUE)
			{
				continue;	
			}
			
			inslist[i].func();

			result = IF_ActiveAppCommand();			

			if(result == TRUE)						
			{
				return;
			}
					
			SendResponse();				    

			break;
	    }                           			
	}
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 main                                                                                                                   
                                                                                                                                                
  input parameter:   void                                           
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       COS的主控程序，发送ATR，PPS，命令调度
  
  Coder:               			   Tester:
  
  Status:        

 -------------------------------------------------------------------------------------------------------------------*/    
static code INSLIST inslist[]=
{
//SIM and USIM command 
	{0xA4,SelectCommand},
  	{0xD6,UpdateBinary},
	{0xB0,ReadBinary},
  	{0xB2,ReadRecord},
  	{0xDC,UpdateRecord},
  	{0x04,FileStatusHandler},
  	{0x44,FileStatusHandler},
  	{0xF2,StatusCommand},
  	{0xC0,GetResponseCommand},
  	{0xC2,EnvelopeCommand},
  	{0x12,FetchCommand},
  	{0x32,IncreaseCommand},
  	{0x14,TerminalResponse},
  	{0x10,TerminalProfile},
	{0xA2,SeekCommand},
	{0x20,VerifyChangeUnblockChv},		//VerifyChv
	{0x24,VerifyChangeUnblockChv},		//ChangeChv
	{0x26,ChvHandle},					//DisablePIN
	{0x28,ChvHandle},					//EnablePIN
	{0x2C,VerifyChangeUnblockChv},		//UnblockChv
	{0xFA,Sleep}, 
//only USIM command
#if NATIVE_CARD_UICC
	{0xE0,CreateFile},
#endif
#ifndef SingleSIM
	{0x70,UsimManageChannel},
#endif
	{0x84,UsimGetChallenge},
//private command
	#if NATIVE_CARD_UICC		
	{0x85,ReloadVerifySuperKey},
	{0xD8,PersoReadEeprom},
	{0xDA,PersoWriteEeprom},
	#endif
	{0xDF,SetBank},
#if NATIVE_CARD_UICC		
	{0xE2,EepormErase},
	{0xF8,ReloadKey},
#endif
	{0xCA,GetCardInfoData},	
#if NATIVE_CARD_UICC	
	{0xF6,ReloadVerifyManagenerKey},	//ReloadManagenerKey
	{0xFC,ReloadVerifyManagenerKey},	//VerifyManagenerKey
	{0xE8,SetGetCOSCofigData},
#endif	
  	{EOL,IF_ReceiveAPDU},
};


#if defined S3FS9AX || defined S3FW9FX
int OS_Main(void)
#else
int uicc_main(void)
#endif
{ 
	ppApdu_Command_t     D_A    cmd;
	
	apdu_GetCommand(&cmd);
	
	memcpy(APDUBuffer , cmd->pData,5);
	apdu_SetCommand(APDUBuffer, APDU_BUFFER_SIZE);
//	iso7816Util_SetCommand(APDUBuffer, APDU_BUFFER_SIZE, UART_BUF_PCK_TYPE);
	
	for(;;)
	{		

#if NATIVE_CARD_UICC		
		if(INS == 0xE2)
		{
			EepormErase();
			SendResponse();
		    continue;
		}
		else if(INS == 0xE8)
		{
			SetGetCOSCofigData();
			SendResponse();
		    continue;
		}
		else if(INS == 0xCA)
		{
			GetCardInfoData();
			SendResponse();
			continue;	
		}
		else if(INS == 0xE9) //获取卡片剩余空间信息
		{
			GetCardSpaceInfo();
			SendResponse();
			continue;
		}
		else if(INS == 0xF3)//设置卡片生命周期
		{
		   SetCardLifeCycle();
		   SendResponse();
	     continue;
		}
#if UICC__OPERATOR__CHINATELECOM		
		else if(INS == 0xEA)//HTTP 参数 和 RFMKey 设置
		{
		   SetCARDConfig();
		   SendResponse();
	     continue;
		}	
#endif		
		
	
		if(G_CardStatus == D_CardEmpty)										            
		{
			if(INS == 0xFE)	
			{
				Initializtion();				
			}
			else if(INS == 0xDF)
			{
				SetBank();
			}
			else if(INS == 0xD8)
			{
				PersoReadEeprom();
			}
			else if(INS == 0xDA)
			{
				PersoWriteEeprom();
			}
			else if(INS == 0xDD)
			{
				CalCRC();
			}
			else
			{
				ConditionSatisfied();				
			}

			SendResponse();

		  continue;
		}

	#endif	

		if(G_CommandMode == Command_Mode_First)					//第一个APDU命令
		{
			if (CLA == 0xA0)
			{
				G_CommandMode = Command_Mode_2G;					//当前为2G操作模式
				
				SW1 = 0x9F;
			}
			else if (((CLA&0xFC) == 0x80)||((CLA&0xF0) == 0x00))
			{
				G_ChannelNum = CLA&0x03;
				
				if(ValueBit(G_ChannelStatus,G_ChannelNum) == FALSE)
				{
						ChannelNotSupport();								//通道不支持
						SendResponse();
						continue;
				}
									
				G_CommandMode = Command_Mode_3G;					//当前为3G操作模式
				
				SW1 = 0x61;
			}
			else	//for baihe-test
			{;}
			
			SetChannel(E_FILE_START,0x3F00,FALSE);	//设置MF的信息
			
			if(INS == 0xC0)  //对于USIM卡片而言，由于命令派发由jcvm处理，jcvm对于00A40404XX的APDU会收取其数据域，为防止收到的数据在这里被覆盖，因此，只有在第一条指令为C0时，才取响应
			{
				G_CurrentReponse[G_ChannelNum] = SetChannel(E_FILE_START,0x3F00,TRUE);	//设置MF的信息

				SW2 = G_CurrentReponse[G_ChannelNum];

				if(P1P2Check() == FALSE)
				{
					SendResponse();
					continue;
				}
				
				if(P3 == SW2)
				{
					SendDataSingle();
					GlobalSetStatus(0x9000);
				}
				else if(P3 < SW2)
				{
					if(G_CommandMode == Command_Mode_2G)
					{
						SendDataSingle();
						GlobalSetStatus(0x9000);
					}
					else
					{
						SendDataSingle();
						apdu_ApduSetOutgoingAndSend(5+P3,SW2-P3);
						SW1 = 0x61;
						SW2 = SW2-P3;
					}
				}
				else
				{
					apdu_ApduSetOutgoingAndSend(5,SW2);
					SW1 = (G_CommandMode == Command_Mode_2G)?0x67:0x6C;
					SW2 = G_CurrentReponse[G_ChannelNum];
				}
				
				SendResponse();
				continue;
			}
		}

		if((G_SuperKeyAccess == SuperKeyAccess_GetChallange)&&(INS != 0x85))
		{
			G_SuperKeyAccess = SuperKeyAccess_NoVerify;
		}
		
		CommandAttemper(inslist);

		IF_ProcessApp();				

		//G_COMMANDPATH=0x00;
		
#if GP__AMENDMENT__B__FOR_MASK
		IF_AnnexB_StartTimerAfterRestartHandset();	                        //判断是否需要开机后重启https会话
#endif

		continue;
    }
}
