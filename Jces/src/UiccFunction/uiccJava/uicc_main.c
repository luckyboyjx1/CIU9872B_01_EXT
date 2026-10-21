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
			v1.8 删除函数PpsProcess()，将该函数移到底层hardapi.c --chenl @20110808
				 修改函数main()，删除超号用的断电保护函数MoveEepromResume()，因国外平台无超号功能
				 G_CurrentReponse的定义由BYTE改为WORD，便于支持0x100的输出  @20110806 by xuyz
			V1.9 定义宏BERTLVCmd，将BER-TLV结构的处理放入该宏内 --20111027 by chenl
			V1.10删除E4 DeleteFile和D4 ResizeFile这两条指令以减小代码体积，一般需求中不会用到这两条指令，如以后有需求要用到这两条指令再添加 --chenl @20111101

		    V1.11 增加G_RFMPretreatment,G_A4Control的定义，修改SystemInit函数，针对RFM标记处理和A4指令做出修改		 @20130922 wlj

			V1.12 去掉G_RFMPretreatment的定义 @20130925 wlj

			V1.13 增加全局变量G_STKBusy，该全局变量用于当前是否处于SendProcativeCommand命令中.@20140404 by xuyz

			V1.14 修改函数ChangeBaud，增加切换TA1功能中对95波特率的支持，modify by zengxiao@20140718

			V1.15 修改函数SystemInit与main,只在2G模式下执行FDN过程		modify by yangjian@20150210

			V1.16 修改函数SystemInit，删除E_SYSTEM_STATUS的使用
				  去掉不使用的UpdateFile指令							@20150430  by yangjian


 */
#include <uicc_error.h>
#include    "typedef.h"
#include    "uicc_main.h"
#include    "Apdu.h"
//#include     "main.h"
#include    "gsmpublic.h"
#include    "string.h"
//#include    "resume.h"
#include    "gsmpublic.h"
#include	"stkhead.h"
#include	"fileAction.h"
#include	"fileSystem.h"
//#include 	"hard_api.h"
#include    "uicc_interface.h"
#include    "Interpreter.h"
#include  "Authentication.h"
//#include  "transaction.h"
#include  "ToolkitAPIPublic.h"
#include	"STKToolkitInterface.h"
#include	"stkalert.h"
//#if UICC__FUNCTION__BERTLVCmd
#include    "DataOrientedCommand.h"
//#endif
#include    "STKCATRuntimeEnviorment.h"
#include "T0ApduImpl.h"
#include "interface.h"
#include "PrivateCommand.h"
#include "anomaly.h"
#include "SEEUICCInterface.h"

#if GP__AMENDMENT__B__FOR_MASK
#include "Interface_AnnexB.h"
#endif
#include  "cardManager.h"
#include "Supi.h"
#include "SuspendUicc.h"
#include "ToolkitRFM.h"
#include "TimerModuleProces.h"
#include "PrivateCommand.h"
#if UICC__AUTH__CDMA__CAVE
	#include "cdma.h"
#endif
#if UICC__APP__WHITECARD
#include "WhiteCardBeWrited.h"
#include "LiveWirteCardXL.h"
#endif

UWORD xdata G_CreatingMFOffset;        //当前正在创建的Profile的MF offset
UWORD xdata G_CurMFFileStart;    //当前Profile下的3F00起始地址
WORD   xdata   G_GPErrorCode;
//BYTE    xdata    G_APDU[5];
//BYTE    xdata    G_616CStatus[MAX_LOGICAL_CHANNELS][8];
BYTE    xdata    G_NextAPDUFlag;
#if RUN_IN_SIMULATOR
BYTE    xdata    tempBuf[TEMP_BUF_SIZE];
#else
__attribute__((aligned(4))) BYTE    xdata    tempBuf[TEMP_BUF_SIZE];
#endif
BYTE    xdata    VARIABLEBUFFER[0x200];
BYTE    xdata    G_StatusResponse[2];

#if SE_ONLY
BYTE    xdata    G_KeyStatus;
#else
BYTE    xdata    G_LocalPinStatus[8];   //LocalPin Status 81~88
#endif


WORD    xdata    G_CurrentAdfName[4];
WORD    xdata    G_CurrentDfName[4];
UWORD   xdata    G_CurrentDfAddr[4];
WORD    xdata    G_CurrentEfName[4];
UWORD   xdata    G_CurrentEfAddr[4];
BYTE    xdata    G_CurrentRecNum[4];
WORD    xdata    G_CurrentReponse[4];



BYTE	xdata	 G_CurrentSfi;              //?							
BYTE    xdata    ResData[256];
BYTE    xdata    G_ChannelNum;
BYTE    xdata    G_SystemStatus;
BYTE	xdata	 G_CardStatus;
BYTE    xdata    G_UniversalPinStatus;			//Universal PIN status
BYTE    xdata    G_ChvStatus;			//PIN1 status
WORD    xdata    G_AdmStatus;			//ADM status,cos支持ADM1- ADM10, 需求来源： 移动NFC2.0 SWP卡要求AC应用下的AC文件的更新权限是ADM10 ，所以COS 增加了ADM4-ADM10的校验支持
BYTE    xdata    STKPerform[2];  //STKSTKPerform[0] :TRUE:表示当前主动式命令拼装结束 0x02：表示当前主动式命令fetch结束 FALSE:表示已经收到当前主动式命令的14响应,
        						 //STKPerform[1]只有在STKPerform[0]为TRUE 的时候才有效 ,记录当前的STK 的通道信息 .用作在UICC 命令commandFinish(0)时，如果之前其他通道有STK 待发送，而当前通道上的响应字为0x9000时，不会发送91XX出去
BYTE    xdata    G_CommandMode;         //命令模式：2G/3G
BYTE	xdata	 G_OpenControl;			//开机控制变量。
#if UICC__FUNCTION_CMCCAPI
BYTE    xdata    G_UIDShuaKa;   //表示UID刷卡标记
#if UICC__FUNCTION__RECORD_NFC_NEW
BYTE    xdata    G_CPUShuaKa;    //表示CPU刷卡标记
BYTE    xdata    G_M1ShuaKa;    //表示M1刷卡标记
#endif
#endif
//BYTE    xdata    G_A4Control;
//BYTE    xdata    G_BakRecDataTag;


#if UICC__FUNCTION__BERTLVCmd
BYTE    xdata G_BERTLV_OperateTag[4];     //current tag（Retrieve or Set ）
WORD    xdata G_BERTLV_LastOffset[4];     //current previous offset
WORD    xdata G_BERTLV_CurrentOffset[4];  //current offset
WORD    xdata G_BERTLV_CurrentPointer[4];  //current tag point
UWORD   xdata G_PreviousEfAddr[4];        //modify current file pointer
#endif

WORD    xdata    G_STKBusy;	    //表示当前是否处于STK中，FALSE：表示当前未处于STK中/others：表示当前正在STK中，且该值表示STK 的通道信息/APPID;原来G_STKBusy的取值只有TRUE/FALSE两种，为了兼容在STK 中处理Java指令，将G_STKBusy扩展成标记当前哪个通道上的STK操作
BYTE    xdata    G_Post9EFlag;  //BIT0:1表示当前状态字为9EXX ,else 0为9FXX
BYTE    xdata    G_OpenControl;  //开机控制变量， 标识开机流程，初始化上电是0， 收到terminalprofile后置为1
BYTE    xdata    G_TerminalProfile[0x25];  //开机保存terminalProfile值,lv结构
WORD    xdata    G_DF_ID_USIM;

//#if UICC__AUTH__CDMA__CAVE //deleted by zhangxiaoxiao@20210601: EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
WORD    xdata    G_DF_ID_CSIM;
//#endif

#if UICC__ISIM
WORD    xdata    G_DF_ID_ISIM;
#endif
// modify by xuxiaojun@20220318 分层, 该变量属于Boot层
//BYTE xdata RAM_ATR_Flag; //modify by zhangxiaoxiao@20210721: 将全局变量RAM_ATR_Flag提到开关UICC__FUNCTION_CMCCAPI外面，用于判断是否发送ATR

#if UICC__FUNCTION_CMCCAPI
BYTE xdata RAM_Function_Supported;
BYTE xdata RAM_Online_Flag;
BYTE xdata RAM_Location_Status_Flag;//是否支持event
#endif
BYTE    xdata   G_EventBufferOld[17];//LV结构

#if (EUICC_ONLY||SE_AND_EUICC)
BYTE  xdata G_LpaSendRefresh; //用于LPA有主动性命令,14还没回来,LPA主动关闭通道的场景使用
#endif
#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
BYTE xdata G_UpdateCycleRecPointer;
#endif
#if UICC__AUTH__5G__SUSPEND
#include "SuspendUicc.h"
#endif

/*
*@function : ReselectPrivateFile
*
*@description:在RFM 进行delete resize create中可能会存在直接处理MF/EF DF 而私有目录位于这些文件后，那么全局变量G_EnvelopHanderAddr等会出错
*
*@param       :void
*
*@return       :void
*
*@history       :V1.0 :create by xuyz
*/
void ReselectPrivateFile(void)
{
	/*    //此处考虑的有缺陷，G_EnvelopHanderAddr的处理不一定正确，暂时不处理
#if UICC__FUNCTION__STKAPI
	if(G_STKBusy == FALSE)
	{
		G_EnvelopHanderAddr = SelectPrivateLFFile(DF_Toolkit,EF_EnvelopeHandlerBuf);
	}
	else   //在STK里面选择第二个buf
	{
		 G_EnvelopHanderAddr  = SelectPrivateLFFile(DF_Toolkit,EF_EnvelopSecondHandlerBuf);
	}

	if(G_EnvelopHanderAddr != FALSE)  //进入到该函数时，一定是在UICC中，所以ToolkitHandlerList全局变量一定已经被初始化
	{
		 ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer = (BYTE *)(G_EnvelopHanderAddr + sizeof(TranFileHead));

		 ToolkitHandlerList[Index_EnvelopeHandler].HandlerBufferSize = GetTFSize(G_EnvelopHanderAddr);
	}
#endif
*/
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 SystemInit

  Input parameter:   void

  Output parameter:  void

  Description:       根据卡片的状态，对系统变量设置相应的状态

  Coder:

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
void UICCInition(void)
{
#if UICC__AUTH__5G__SUSPEND
	BYTE idata tag;
#endif
	ecpMem_Set(ResData,0x00,sizeof(ResData));

	G_DF_ID_USIM = GetADFFID((BYTE *)&USIMPartial[0]);

#if UICC__AUTH__CDMA__CAVE
	G_DF_ID_CSIM = GetADFFID((BYTE *)&CSIMPartial[0]);	    //卡片支持CSIM
#endif

#if UICC__ISIM
    G_DF_ID_ISIM = GetADFFID((BYTE *)&ISIMPartial[0]);
#endif
	G_GBA_AUTH_Status = 0;  //初始化ISIM 鉴权变量
	
	ecpMem_Set(G_Auth_Rand,0,sizeof(G_Auth_Rand));//优化鉴权性能（88/C0指令不写文件，其他指令再写）,移动备案性能测试 by yjr20211213
	
	ecpMem_Set(G_Auth_Ks,0,sizeof(G_Auth_Ks));

	G_Post9EFlag = FALSE;   //该变量在CommandFinish 中使用，在CommandMode_2G模式下其bit0为1 ：表示当前发送状态响应字为9Exx;bit0为0 ，表示当前发送9FXX

	G_OpenControl = 0x00;

	G_JavaMenuIndex = 0;

	//配置menuId
	G_SMItemLen = 0;

	G_ChannelNum = 0x00;

	G_ChvStatus = 0x00;
#if UICC__FUNCTION__UniversalPin	
	G_UniversalPinStatus = 0x00;
#endif
	G_AdmStatus = 0x00;

	//G_A4Control = 0x00;

	ecpMem_Set(roundKeys,0x00,ROUND_KEYS_SIZE);

//	ecpMem_Set(state,0x00,sizeof(state));


	G_CommandMode = Command_Mode_First;

#if SE_ONLY
	G_KeyStatus = 0;
#else
	ecpMem_Set(G_LocalPinStatus,0x00,sizeof(G_LocalPinStatus));
#endif

	if(G_CardStatus != D_CardEmpty)
	{
		ReadPinStatus();                                                      	//读PIN状态
	}

	ecpMem_Set(&G_CurrentRecNum[0],0x00,sizeof(G_CurrentRecNum));

	ecpMem_Set(&G_CurrentReponse[0],0x00,sizeof(G_CurrentReponse));

	ecpMem_Set(STKPerform,0x00,sizeof(STKPerform));

    G_CurrentSfi = 0;

	//G_BakRecDataTag = FALSE;

	G_ResponseLen = 0x00; //MBMS 脚本测试  6.36

#if UICC__AUTH__5G__SUPI
		G_SUPILEN=0;
//		G_LONGLEN=0;
#endif
	ecpMem_Set(&G_EventBufferOld[0],0x00, sizeof(G_EventBufferOld)); //初始化
#if UICC__AUTH__5G__SUSPEND
		G_TerminalCap=TRUE; //两次terminalCAPbility的值相同
		G_InvildCommandTag=FALSE;

		ReadEeprom(&tag,E_SUSPEND_TOKEN,0x01); //是否执行过UICCSUSPEND指令
		if(tag==0x01)
		{
			G_SuspendUiccTag=TRUE;

		}
		else
		{
			G_SuspendUiccTag=FALSE;
		}
#endif
#if UICC__AUTH__CDMA__CAVE
	InitCDMAVariable();
#endif

#if TIMERMODULE
	G_EventTimerExpirationId = FALSE;
#endif
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT          //级联deliver变量初始化

	G_Deliver_T0_Submit = 0x00;
	
	G_PorAdditionalLen = 0x00;
	
	G_Back_EnvelopHanderAddr = G_EnvelopHanderAddr;
	
#endif
#if  UICC__AUTH__ANTICLON
	G_RandomClone = 0x00;        //2G防克隆全局变量初始化  // add by chentao 20221207 俄罗斯MTS招标需要测试 V1V2V3
#endif
#if UICC__CDMA__CATTP__PRL
	ResumePrlData();    //PRL断电恢复
#endif 
	//IF_UICCInition();
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

#if UICC__FUNCTION_CMCCAPI
void getOnlineStateInit(void)
{
	RAM_Function_Supported = 0x00;
	
	RAM_Online_Flag = 0x00;		

//	RAM_ATR_Flag = 0x00; // modify by xuxiaojun@20220318 分层, 该变量属于Boot层，在其他地方初始化
		
	RAM_Location_Status_Flag = 0x00;
}
#endif


/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 CommandAttemper

  input parameter:   inslist

  output parameter:   void

  description:       COS的主控程序的命令调度部分功能

  Coder:             xuqs

  Status:

 -------------------------------------------------------------------------------------------------------------------*/
void CommandAttemper(FLASH INSLIST *inslist,BYTE mode)
{
	BYTE idata i;

	G_ChannelNum = CLA&0x03;

#if UICC__AUTH__CDMA__MD5
	if(INS != 0x80)
	{
		if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xFC) != 0x00)&&(CLA != 0xA0))||((INS==0x78)&&((CLA&0xFC) != 0x80)) || (((G_CommandMode == Command_Mode_3G))&&(INS==0x88)&&((CLA&0x80) == 0x80)))		//拒绝非激活的操作模式							
		{
			ClaError();

			if((mode == STK_MODE)&&(G_NextAPDUFlag == FALSE))
			{
				SendResponse();
			}

			return;
		}
	}
	else
	{
		/*if((CLA&0xFC) != 0x80)   // modify by chentao@20211012
		{
			ClaError();
	
			SendResponse();
	
			return;
		}*/

	}
#else
	if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xFC) != 0x00)&&(CLA != 0xA0))||((INS==0x78)&&((CLA&0xFC) != 0x80)))		//拒绝非激活的操作模式							
	{
		ClaError();

		if((mode == STK_MODE)&&(G_NextAPDUFlag == FALSE))
		{
			SendResponse();
		}

		return;
	}
#endif
	//注意：由于java层也有70指令，因此UICC不再处理manage channel指令，UICC应用所收到的通道命令都默认是有效的通道，这个需要处理的是通道上的DF信息的设置

	//if((INS != 0xC0)&&(G_616CStatus[G_ChannelNum][0] == 0x9F))	//如果当前通道需要取响应，但是收到的却不是C0，则清除当前通道的响应数据
	//{
	//	ecpMem_Set(G_616CStatus[G_ChannelNum], 0x00, 0x08);
	//}
#if UICC__AUTH__5G__SUSPEND//卡片在执行UICCRESUME前，只能执行A4(P1!=04),B0,D0,AA指令，如果执行了其他指令，则清除保存的状态，按现有状态进行
	if(G_SuspendUiccTag==TRUE)
	{
		if(((INS==0xA4)&&(P1!=0x04))||(INS==0xB0)||(INS==0xB2)||(INS==0xAA)||(INS==0x76)||(INS==0xC0))
		{

		}
		else //如果执行了其他指令，所有记录的状态清空
		{
			//ecpMem_Set((BYTE *)&UiccStatus,0x00,sizeof(UiccStatus));

			G_InvildCommandTag=TRUE;
		}
	}

#endif

//	if (IF_MainReceiveApdu() == TRUE)
//	{
//		return;
//	}

	for(i=0;;i++)
	{
	    if((inslist[i].ins == INS)||(inslist[i].ins == EOL))
	    {
			inslist[i].func();

			if((mode == STK_MODE)&&(G_NextAPDUFlag == FALSE))
			{
				SendResponse();
			}

			break;
	    }
	}
}
/*@Function:    IF_ProcessSystemProactiveCommand                                            
 * 
 *@Descrip:     处理系统主动式命令
 *				The CAT Runtime Environment shall send its system proactive command(s) as soon as no proactive session is ongoing
 *				and after all the Toolkit applets registered to the current events have been triggered and have returned from the
 *				processToolkit() method invocation.
                该函数位于处理完main APU之后，发送SW1SW2之前
 *
 *@See:	        102241 6.7.1
 *                                                                                                                                     
 *@Param:       NULL
 *								                                                                                                         
 *@Return:      NULL                                                                                                                                                                                                                                                                   
 * 
 *@Author:      zzj 2013.09.25
 *  
 *@History:     V0.0---Create by zzj 2013.11.28
				V1.1---Create by LCZ 20200623 处理完标志G_ToolkitSystemProactiveFlag有可能重新赋值(比如更新6F54或者注册事件)所以使用goto
 *                  V1.2---在IF_ProcessSystemProactiveCommand函数中调用JavaToolkitProcessProactiveHandlerEvent ( 修改某些情况下123 事件应用没有被触发的问题,by xwh 20210412 )                  
 *           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 */
void IF_ProcessSystemProactiveCommand(void)
{
#if UICC__FUNCTION__STKAPI
	BYTE idata result, bit;
	BYTE idata i,recNum;
	WORD idata duration;
	BYTE xdata tmp[EventList_Length];
	UWORD xdata toolkitAppAddr;
_SystemProactiveFlagAgain_:
	result = FALSE;
    
	if((SW1 != 0x90)||(SW2 != 0x00)||(ValueBit(G_ToolkitSystemProactiveFlag,Bit_Pending)))//当指令返回值为9000，且没有系统主动式命令在执行，可触发新的系统主动式命令
	{
		return;
	}
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return;
	}
#endif
	for(bit = 0; bit < 7; bit++)  //检查是否有系统主动式指令需要发送，目前仅低4位有效
	{
		if(ValueBit(G_ToolkitSystemProactiveFlag,bit))
		{  
			if((bit == Bit_SetUpMenu)||(bit == Bit_PowerOnSetupMenu))
			{
				result = SetUpMenu();
			}
			else if((bit == Bit_SetUpEventList)||(bit == Bit_PowerOnSetupEventList))
			{
				if(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_Setupeventlist) == SUPPORT_SETUPEVENTLIST)
				{
					result = SetupEvenList(TerminalProfile_Mode);
				}
			}
			else if((bit == Bit_PollInterval)||(bit == Bit_PowerOnPollInterval))
			{				
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
				recNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
				for(i=0; i<recNum; i++)
#else
				recNum = GetFileHeadByte(toolkitAppAddr, F_RecNum_Offset);
				for(i=1; i<=recNum; i++)
#endif
				{
				    if(bit == Bit_PowerOnPollInterval)	 //开机的时候是否只发送一个Poll interval
					{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
				    	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

				    	if(toolkitAppAddr == ADDR_IS_NULL)
				    	{
				    		continue;
				    	}
#endif
				    	ReadLFBufForJavaAccess(toolkitAppAddr,i,AppletStatus_OFFSET,&tmp[0],0x01);

						if(tmp[0] == APPLET_UNLOCKED_SELECTABLE)
						{
							ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,tmp,EventList_Length);

							if(ToolkitGetEventBit(tmp,EVENT_STATUS_COMMAND_bit))
							{
								//ReadLFBuf(toolkitAppAddr,i+1,PollDuration_OFFSET,(BYTE *)&duration, sizeof(WORD));
								VMReadBuf((BYTE*)&duration,E_CUR_DURATION,sizeof(WORD));//使用系统的poll Interval时间间隔

								result = ToolkitPollInterval(duration);
	
								break;
							}
						}
					}
					else
					{
						if(ValueBit(G_PollIntervalRecNum,i) == TRUE)
						{
							ClearBit(G_PollIntervalRecNum,i) ;
	
							//ReadLFBuf(toolkitAppAddr,i+1,PollDuration_OFFSET,(BYTE *)&duration, sizeof(WORD));
							VMReadBuf((BYTE*)&duration,E_CUR_DURATION,sizeof(WORD));
	
							result = ToolkitPollInterval(duration);	
	
							break;
						}
					}
				}		
			}
			else if(bit == Bit_PollOff)
			{
				result = ToolkitPollOff();
			}
			else
			{
				
			}  

			if(bit == Bit_PollInterval)
			{
				if(G_PollIntervalRecNum == FALSE)
				{
					ClearBit(G_ToolkitSystemProactiveFlag,bit); 				 
				}
			}
			else
			{
				ClearBit(G_ToolkitSystemProactiveFlag,bit); 
			}
	
			if(result == TRUE)											   //标识已被处理，清除对应标志位
			{			
				SW1 = 0x91;												   //重新对SW1 SW2赋值

				SW2 = (ResData[2] > 0x7F) ? (ResData[2]+3) : (ResData[2]+2);
		
				SetG_ToolkitSystemProactiveFlag(Bit_Pending); 		   //置位Bit_Pending ，表示当前有系统主动式指令正在执行

				break;
			}
		}
	}

	for(i=0;i<2;i++)
	{
		if((SW1 == 0x90)&&(SW2 == 0x00)&&(G_ToolkitSystemProactiveFlag == 0x00)&&((G_TerminalProfile[0])!= 0x00))
		{
			if(i == 0)
			{
				ProcessPendingMessageAlert();
			}
			else
			{
				JavaToolkitProcessProactiveHandlerEvent();
			}
					
			CommandChangeSW(0x00);
			
			if((SW1 == 0x90)&&(SW2 == 0x00)&&(G_ToolkitSystemProactiveFlag != 0x00))
			{
				goto _SystemProactiveFlagAgain_;
			}
		}
	} 
    #endif
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
	{0xE0,CreateFile},
	{0xE4,DeleteFile},
	{0x84,UsimGetChallenge},
	{0xD4,ResizeFile},
	{0xAA,TerminalCapability},
#if UICC__AUTH__5G__SUPI
	{0x78,GetIdentity},
#endif
#if UICC__AUTH__5G__SUSPEND
	{0x76,SuspendUicc},
#endif
#if UICC__FUNCTION__BERTLVCmd
	{0xCB,RetrieveData},
	{0xDB,SetData},
#endif
#if SE_ONLY
	{0xF0,UpdateFile},
#endif
	{0xF7,PrivateFileCheck},				//Reload key
	{0xF8,ReloadKey},				//Reload key

#if (EUICC_ONLY || SE_AND_EUICC)
	{0xF9,UpdatePriveFile},
#endif

  	{EOL,IF_ReceiveAPDU},
};

/*
*@description: uisim main的主流程处理函数
*
*@function   :NativeUICC_Main
*
*@param      :
*
*@return     :void
*
*@history    :V1.0:修改在main函数中进行数据的接收，如果当前数据是A4或70数据，则返回主流程进行
*             V1.1 在HandleFirstCommand调用UICCToolkitInit和UICCInition，uicc_main中不再调用，修改联通保险箱测试不同通道冲掉全局变量的问题，by xwh 20210929
*            V1.1 : IF_UICCInition 必须在NativeUICC_Main调用 测试用例 白卡入网-APP写卡-空卡不预置KOPC traversalforapdu_n004, by xwh 20210930
*/
void NativeUICC_Main(void)
{
	BYTE idata curChannel;
	BYTE idata firstMain;

	G_NextAPDUFlag = G_CurSelectedChannel+1;  //第一次进入native uicc_main函数时必须是已经收到过命令头

	firstMain = TRUE;  //表示由外界进入main函数

	CacheFlush();  //由JAVA层(虚拟机)进入NativeUICC_Main，后续需要处理APDU指令

	if(G_CommandMode == Command_Mode_First)					//第一个APDU命令
	{
		//UICCToolkitInit();			//   uicc api相关全局变量初始化操作

        //UICCInition();  //UICCInition会调用IF_InitSystem，该函数里面又会调用InitJavaMenuIdInfo，初始化菜单式会用到6F81 6F82的地址，所以要先初始化UICCToolkitInit
		IF_UICCInition();
		UICC_Toolkit_FileView_Init();
	}

	for(;;)
	{
		curChannel = GPGetCurrenteChannel();

		if(curChannel== ILLEGAL_CHANNEL)   //通道非法，直接发送6E00
		{
			G_NextAPDUFlag = FALSE;

			if(G_CommandMode == Command_Mode_2G)  //在第一次进入uicc_main时，通道肯定已经是成功打开的
			{
				GlobalSetStatus(0x6E00);
			}
			else
			{
				//GlobalSetStatus(0x6881);  //3G模式下应该返回6881，2G模式下应该返回6E00
				GlobalSetStatus(SW_CLA_NOT_SUPPORTED); //<移动rsp下载profile后测试脚本\加密 UICC测试脚本_6Cxx升级版+Enable PIN升级 For eSIM\命令测试\StructureOfCommandsAndResponse-6.7.1.js>
			}

			firstMain = FALSE;

			GlobalSendResponse();

			continue;
		}

		if(curChannel >= ISO_MAX_LOGICAL_CHANNELS)  //当前收到的通道数据为非接通道的数据，则跳回jcvm去重新派发
		{
			G_NextAPDUFlag = curChannel+1;

			ThrowErrorBackToMain(Priv_EXCEPTION);  //抛异常，通过该异常机制返回JCVM 虚拟机主流程去处理，该异常的抛出不会有任何实际意义
		}

		if((INS == 0x70) || ((INS == 0xA4)&&(P1 == 0x04)) )//对于70 A404指令，此时数据jcvm处理的命令，跳到虚拟机层重新处理,但是要注意第一次就是由00A404命令进来的，不能再跑出去了
		{
			if(firstMain == FALSE)   //表示当前的A404指令是在该for循环中收到的，需要跳回到主流程重新进行派发,对于70指令而言，不会在外界收到70命令才进来的
			{
				G_NextAPDUFlag = curChannel+1;

				ThrowErrorBackToMain(Priv_EXCEPTION);  //抛异常，通过该异常机制返回JCVM 虚拟机主流程去处理，该异常的抛出不会有任何实际意义
			}
			else//A4是从Java进来的  该情况只有选择ADF时或者未找到需要选择的Java应用时会分发进来
			{
				if(INS == 0x70)
				{
					if((CLA&0xF0) != 0x00)
					{
						ThrowError(SW_CLA_NOT_SUPPORTED);//中国联通 联通2G3G指令测试\M2M 03 USIM卡2G3G基本指令测试2-关闭智能调整.script ManageChannel测试要求 A070000001  返回6E00, 结合102221 70仅支持0X 4X 6X 其中(4X 6X 是支持大于4个通道的情况下 )，GP221 70是支持0X 4X(4X也是通道 > 4的情况下使用)
					}
				}

				else if((CLA == 0xA0) && (INS == 0xA4)&&(P1 == 0x04)) //联通测试0xA0 A4 04 00 02 7F20  期望返回6B00
				{
					if(ValueBit(G_SystemStatus, B_RfmPerform) == FALSE)
					{
						if(P1P2P3Check(0x02, FALSE) ==FALSE)
						{
							G_NextAPDUFlag = FALSE;
							firstMain = FALSE;
							//GlobalSetStatus(0x6A82);
							SendResponse();
							continue;
						}
					}
				}
				else if(SelectFileByName(E_FILE_START,&APDUBuffer[5],APDUBuffer[4]) == FALSE)
				{
//                  #if UICC__APP__WHITECARD
//					    if (SDK_selectAID() == FALSE)
//                   #endif
					    {
					    	G_NextAPDUFlag = FALSE;
							firstMain = FALSE;
							GlobalSetStatus(0x6A82);
							SendResponse();
							continue;
					    }

				}
			}
		}

		if(channelInfo.channel_status[curChannel] == CHANNEL_CLOSED)
		{
			if(G_CommandMode == Command_Mode_2G)  //在第一次进入uicc_main时，通道肯定已经是成功打开的
			{
				GlobalSetStatus(0x6E00);
			}
			else
			{
				GlobalSetStatus(0x6881);  //3G模式下应该返回6881，2G模式下应该返回6E00
			}

			firstMain = FALSE;

			GlobalSendResponse();

			continue;
		}

		if((getChannelContext(curChannel)&0xFF) != UICC_APPID)
		{
			G_NextAPDUFlag = curChannel+1;

			ThrowErrorBackToMain(Priv_EXCEPTION);   //抛异常，通过该异常机制返回JCVM 虚拟机主流程去处理，该异常的抛出不会有任何实际意义
		}

		G_CurSelectedChannel = curChannel; 
			
		G_NextAPDUFlag = FALSE;

		firstMain = FALSE;  //第一次进来该函数时，前面的判断条件其实不会执行

		if(INS == 0xFE)
		{
			Initializtion();

			SendResponse();

			continue;
		}
		else
		{
			if(G_CardStatus == D_CardEmpty)
			{
				ConditionSatisfied();

				SendResponse();

				continue;
			}
		}


		if(G_CommandMode == Command_Mode_First)					//第一个APDU命令
		{
			if(CLA == 0xA0)
			{
				G_CommandMode = Command_Mode_2G;					//当前为2G操作模式

				SW1 = 0x9F;

				FdnProgcess();													 		//FDN 过程
			}
			else if (((CLA&0xFC) == 0x80)||((CLA&0xF0) == 0x00)) //白盒测试需要，将if语句移动到下面
			{
				G_CommandMode = Command_Mode_3G;	// 暂时修改为除了A0 模式外，其他的都认为是3G 模式

				SW1 = 0x61;
			}

			SetChannel(E_FILE_START,0x3F00,FALSE);

			G_ToolkitSystemProactiveFlag = FALSE;  //在ｄｅｌｅｔｅ　Ｐａｃｋａｇｅ时可能会对该G_ToolkitSystemProactiveFlag变量进行赋值，此处清除之前的赋值操作，重新开始，否则第一条命令就有可能发送主动式命令

#if UICC__FUNCTION__STKAPI
      JavaToolkitProcess(EVENT_FIRST_COMMAND_AFTER_ATR_bit,0);//在该函数内部进入JAVA 虚拟机，从虚拟机出来时CACHE已经提交  该事件同EVENT_FIRST_COMMAND_AFTER_SELECT
#endif

			if(INS == 0xC0)  //对于USIM卡片而言，由于命令派发由jcvm处理，jcvm对于00A40404XX的APDU会收取其数据域，为防止收到的数据在这里被覆盖，因此，只有在第一条指令为C0时，才取响应
			{
				G_CurrentReponse[G_ChannelNum] = SetChannel(E_FILE_START,0x3F00,TRUE);	//设置MF的信息

				SW2 = G_CurrentReponse[G_ChannelNum];

				//apdu_SendStatusAndReceiveCommand中会清掉RecData的数据，因此要保存在VariableBuffer中
				//tempBuf[0] = G_CurrentReponse[G_ChannelNum];
				//ecpMem_Copy(&tempBuf[1],&RecData[0],tempBuf[0]);

				//WriteVariable(Tag_BakRecData,&tempBuf[0]);
				//G_BakRecDataTag = TRUE;
				
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

		IF_WriteGBAKsFile();//优化鉴权性能（88/C0指令不写文件，其他指令再写）,移动备案性能测试 by yjr20211213
		
		CommandAttemper(inslist,MAIN_MODE);	 //执行main函数中的APDU

		IF_ProcessSystemProactiveCommand();
#if GP__AMENDMENT__B__FOR_MASK
		IF_AnnexB_StartTimerAfterRestartHandset();	                        //判断是否需要开机后重启https会话

		IF_AnnexB_StartHttpSessionByAPI();
#endif
        if(G_NextAPDUFlag == FALSE)  //由于UICC 中的C2命令可能会触发post，post中会接收到新的命令才退出，所以当收到下一条命令时，不能发送响应字?  若 没有该判断UICC_API测试api_2_erh_poab测试会死卡
		{
			SendResponse();
		}

#if !SE_ONLY
        if(G_CurExecutingProfileIndex != G_CurEnabledProfileIndex)//RAM时将执行区域切到了SE ，此时将执行区域切回到profile 继续执行下一条指令 ，UICC下默认操作当前激活的profile，只有RAM才会执行到SE区
        {
        	G_RAMBackExecutingProfileIndex = G_CurEnabledProfileIndex;
        	G_CurExecutingProfileIndex = G_CurEnabledProfileIndex;
        	ExecuEnvironmentInit(G_CurExecutingProfileIndex);
        }
#endif
		continue;
	}

}
