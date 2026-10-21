/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCToolkitPublic.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-23

*Description :该文件主要用于存放实现Toolkit API过程中的一些初始化函数和部分公共函数
*
*History     :
	
author    time      version    description
xuyz   2013/10/23   V1.0       创建该文件
xuyz   2016/11/04   V1.1       修改函数UICCToolkitInit，增加部分全局变量的初始化
************************************************************************************************************************************************************************************************/
#include	"fileAction.h"
#include	"gsmpublic.h"
#include	"hard_api.h"
#include	"fileSystem.h"
#include	"interface.h"
#include	"ToolkitRFM.h"
#include	"VMAccessEeprom.h"
#include	"FunctionSwitch.h"
#include	"STKFileView.h"
#include  "uicc_interface.h"
#include  "Object.h"
#include  "FireWall.h"
#include  "ToolkitAPIPublic.h"
#include  "STKAlert.h"
#include  "T0ApduImpl.h"
#include  "platform_api.h"
#include  "anomaly.h"
#include  "STKFileView.h"
#include "TimerModuleProces.h"
#include "Compactmanage.h"
#include "BIPModuleProcess.h"
#include "ob_manager.h"
#if UICC__FUNCTION__SIMAPI
#include	"SIMToolkitConstant.h"
#include	"SimApi.h"
#endif

#if UICC__FUNCTION__UICCAPI
#include	"UICCToolkitAPI.h"
#include	"UICCToolkitConstants.h"
#include	"USATEnvelopeHandler.h"
#endif
#include "cardManager.h"
#if UICC__FUNCTION__CATTP	
	#include "CatTp.h"
#endif

#if M2M_EUICC
#include "EUICCM2MAPDU.h"
#include "SEEUICCInterface.h"
#endif

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
#include "SEEUICCInterface.h"
#endif

#if UICC__FUNCTION__STKAPI
UWORD   xdata    G_EnvelopHanderAddr;
BYTE    xdata    G_ToolkitTar[0x03];         //缓存当前format_CB,format_update format_env的tar值
BYTE    xdata    G_SPIKICKID[4];
BYTE    xdata    G_TARRecIndex[3];		     //G_TARRecIndex[0]: tarType ,G_TARRecIndex[1]:tar在对应文件中的记录号(对于RAM，存的是TAR对应的SD的appid)，G_TARRecIndex[2]：scp80SDId: 对于Toolkit 或ram操作，保存当前需要使用到的SCP80 的sdId
BYTE    xdata    G_RFMRAMKeyversion;         //缓存当前RAM短信使用的RAM密钥版本
BYTE    xdata    G_formatedSPIOffset;        //缓存当前format ENV Data 的spi在6F90中的偏移
BYTE    xdata    G_formatedTPOAOffset;       //缓存当前format ENV Data 的TPOA 在6F90中的偏移
BYTE    xdata    G_CurrentRFMResponse;       //通过getresponse需要获取的当前POR的数据长度
BYTE    xdata    G_EnvelopeTag;    			//用于保存当前的Envelope BER-TLV tag ，C2指令的第一字节
BYTE    xdata    G_ToolkitSystemProactiveFlag; //判断当前toolkit是否要处理系统的proactiveCommand;	 bit0:setupmenu bit1:setupEvenlist bit2:poll interval bit3:poll off
//BYTE    xdata    G_PowerOnProfielDone;  //判断当前是否已经 执行了开机的主动式命令seupmenu setupeVENTLIST POLL INTERVAL
WORD    xdata    G_TrigerToolkitRef;    //保存待触发toolkit应用的ref
ToolkitContext xdata G_ToolkitContext;
ToolkitHandler xdata ToolkitHandlerList[5];
UWORD   xdata    G_BERTLVEditHandlerContext;
UWORD  xdata     G_PollIntervalRecNum;  //用于保存当前需要进行poll interval的toolkit app在6F80中的记录号，一个bit表示一条记录，因此最大允许支持32个toolkitApp   
BYTE   xdata     G_TriggerAppNumber;  //当前event触发的toolkit app的个数
BYTE   xdata     G_ISOBusyExceptionFlag;  //判断当前是否要向终端发送由toolkit app抛出的9300异常
BYTE   xdata      G_PostTag;    //判断对于formatted_ENV_PP事件，当前是否已发送POR，false:当前未发送POR ,TRUE:当前已发送POR 
//BYTE   xdata     G_VolatileByteArrayIsCalled;
//WORD   xdata     G_CUR_DURATION;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
WORD    xdata    G_ToolkitAppletInfoRef;//G_EF_ToolkitAppRef;
BYTE    xdata    G_ToolkitAppIndex;//正在安装的Toolkit应用的apptabelEntry Index
#endif
WORD    xdata    G_SD_MenuInfoRef;
/**
 * 空间优化，将SetBit(G_ToolkitSystemProactiveFlag,Bit_SetUpMenu);封成一个函数,节约空间288字节
 */
void SetG_ToolkitSystemProactiveFlag(u16 bit)
{
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == FALSE) //下载profile过程中不发送系统主动式命令
#endif
	{
		SetBit(G_ToolkitSystemProactiveFlag,bit);	  	//todo:打上setupmenu 标记,等待发送setup menu主动式命令
	}
}
/*
*@description :在post过程中接收到新的命令时，判断接收到的命令是否是UICC同一通道的命令，进行不同的处理 
*
*@param       :void
*
*@return       :void
*
*@
*/
BYTE PostAfterReceiveCommandHeader(void)
{
	BYTE idata curChannel;
	
	curChannel = GPGetCurrenteChannel();  //获取当前指令的新的通道信息

	if(curChannel == ILLEGAL_CHANNEL)
	{
		return ILLEGAL_CHANNEL;
	}

    if(curChannel == G_CurSelectedChannel)  //新接收命令如果和之前的命令在同一通道，则直接分发，由UICC 自行处理
    {								
        if(G_APDU[1] == 0xC0)//如果接收到的是0xC0命令，返回TRUE
        {
            return TRUE;
        }
        else
        {			
            return 0x02;				
        }			
    }
    else  //如果新接收的命令不是同一通道， 则保存当前UICC 通道的响应数据
    {		
        //G_616CStatus[G_CurSelectedChannel][0] = 0x9F;
        
        //G_616CStatus[G_CurSelectedChannel][1] = SW1;
        
       // G_616CStatus[G_CurSelectedChannel][2] = SW2;
        
       // G_616CStatus[G_CurSelectedChannel][3] = 0x00;

      //  WritePageCacheEX(ResponseDataSave_Addr +G_CurSelectedChannel*ResSave_Len ,(BYTE *)&RecData[0], SW2);	 	//更新数据到指定通道的缓存中,
        
        return FALSE;						
    }

}
/*
@description: 复位时，清空toolkit应用已分配的通道信息和serveice信息
*
*@function  :ToolkitClearChannleAndService
*
*@param      :void
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz@20170920
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void ToolkitClearChannleAndService(void)
{
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum,i,tmp,result;
	u16  X_A    index;
	
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
		
	if(toolkitAppAddr == FALSE)
	{	
		return;	
	}

	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);
	
	for(i=1; i<= toolkitAppRecNum; i++)	  //复位时，清除所有应用已分配的通道ID 
#else
	index  =  nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	for (i = APP_FIRST; i < index; U16_INC(i))
#endif
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}

		ReadLFBufForJavaAccess(toolkitAppAddr,i,AllocateChannelCount_OFFSET,&tmp,sizeof(BYTE));
#else
		result = ReadLFBufForJavaAccess(toolkitAppAddr,i,AllocateChannelCount_OFFSET,&tmp,sizeof(BYTE));

		if(result == FALSE)
		{
			continue;
		}
#endif
		if((tmp != 0x00)&&(tmp != 0xFF))  //之前该应用分配了通道
		{
			SetLFConstForJavaAccess(toolkitAppAddr,i,AllocateChannelCount_OFFSET,0x00,0x01);
		
		}
		
		ReadLFBufForJavaAccess(toolkitAppAddr,i,CurrentService_OFFSET,&tmp,sizeof(BYTE));
		
		if((tmp != 0x00)&&(tmp != 0xFF))  //之前该应用分配了serviceId
		{
			SetLFConstForJavaAccess(toolkitAppAddr,i,CurrentService_OFFSET,0x00,0x01);
		}
	}
	
}
/*
*@funtion     :	UICCToolkitInit

*@description :该函数主要用于在开机时初始化Toolkit APP 所使用到的一些全局变量
*
*@see         :no
*
*@param       :void
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz @20131023
*
*@others      :其他说明，如该函数使用的限制条件等
*             该函数是在NativeUICC_Main中调用，是从JAVA层过来，所以该函数中的文件操作需要用带CACHE的
*/
void UICCToolkitInit(void)
{
	BYTE idata tmp;
	UWORD xdata fileAddr,fileAddr2;
	
	G_ToolkitFileSystemMode = UICCFileSystem;  //初始化全局变量
	
	G_ToolkitCurrentRecNum = 0x00;
	
	G_ToolkitCurrentDfName = 0x00;
	
	G_ToolkitCurrentEfName = 0x00;
	
	G_ToolkitCurrentDfAddr = FALSE;
	
	G_ToolkitCurrentEfAddr = FALSE;

/*
	BAK_ToolkitFileSystemMode_PROFILE = UICCFileSystem;
	BAK_ToolkitCurrentRecNum_PROFILE = 0x00;
	BAK_ToolkitCurrentDfName_PROFILE = 0x00;
	BAK_ToolkitCurrentEfName_PROFILE = 0x00;
	BAK_ToolkitCurrentDfAddr_PROFILE = FALSE;
	BAK_ToolkitCurrentEfAddr_PROFILE = FALSE;

	BAK_ToolkitFileSystemMode_SE = UICCFileSystem;
	BAK_ToolkitCurrentRecNum_SE = 0x00;
	BAK_ToolkitCurrentDfName_SE = 0x00;
	BAK_ToolkitCurrentEfName_SE = 0x00;
	BAK_ToolkitCurrentDfAddr_SE = FALSE;
	BAK_ToolkitCurrentEfAddr_SE = FALSE;
*/			
	G_RAMSessionSdId = ILLEGAL_APPID;
	
	ecpMem_Set(G_ToolkitTar,0x00,sizeof(G_ToolkitTar));

	ecpMem_Set(G_TARRecIndex, 0x00, sizeof(G_TARRecIndex));

//	G_ToolkitSystemProactiveFlag = FALSE;  //全局变量初始化 , 在HandleFirstCommand中已经初始化了

//	G_PowerOnProfielDone = FALSE;   // 这个变量是在哪里被用到的

	TimerModuleInit();     // Timer初始化

	InitVariableRemoteModule(); //RFM变量初始化
	
	InitVariable(); //     要在CATTPModuleInit前初始化
	
	BIPModuleInit();
#if UICC__FUNCTION__CATTP	
	CATTPModuleInit();     //CATTP
#endif

	InitMessageAlert(); //开机清空标志操作

	G_PollIntervalRecNum = FALSE;

	G_CurrentRFMResponse = FALSE;

	G_ISOBusyExceptionFlag = FALSE;

#if UICC__FUNCTION__UICCAPI    //只有在UICCAPI 中才会用到该变量
	G_ToolkitFileContextEvent = 0x03;//0x00:永久对象，0x01:clear on reset,0x02:clear on deselect,0x03空闲 20140313 lj增加该变量初始化
#endif
	
	G_RFMRAMKeyversion = 0xFF;

	//GPRAMAPDUExtendResponse(TAG_ClearRamExtendRes);

	G_formatedSPIOffset = 0;

	G_formatedTPOAOffset = 0;

	G_EnvelopeTag = 0x00;

	G_TrigerToolkitRef = REF_NULL;

	G_BERTLVEditHandlerContext = 0;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	G_ToolkitAppletInfoRef  = REF_NULL;
	G_ToolkitAppIndex = ILLEGAL_APPID;
#endif

	G_SD_MenuInfoRef = REF_NULL;
//	G_TriggerAppNumber = FALSE;   // JCREInit函数中已经对该变量进行了初始化

	ecpMem_Set((BYTE *)&G_ToolkitContext,0x00,sizeof(G_ToolkitContext));	 // 20140313 lj增加该变量初始化,处理非共享文件时，处于UICC applet下，该值要为0
	
	ecpMem_Set((BYTE *)ToolkitHandlerList, 0x00,sizeof(ToolkitHandlerList));
	
	if(G_EnvelopHanderAddr != FALSE)  //在函数中HandleFirstCommand已经选择了文件
	{		
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer = (BYTE *)(G_EnvelopHanderAddr + sizeof(TranFileHead));	 
        
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerBufferSize = GetTFSize(G_EnvelopHanderAddr);
        
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerAvailable  = FALSE; 
		
		//ClearingMessageAlert(D_SetUpMenuTag);

		fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitAppContext);
			
		if(fileAddr != FALSE)
		{
			ReadTFBuf(fileAddr,0x00,&tmp,sizeof(BYTE));
			
			if(tmp != 0x00)
			{		
				SetTFConst(fileAddr,0x00,0x00,sizeof(BYTE));
			}
		}		
	}
	else
	{  
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer = NULL;		
        
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerBufferSize = 0;
        
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerAvailable  = FALSE;	
	}

	ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = 0;
    
	ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength = 0;
    
	ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset = 0;

	ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer = RecData;
    
	ToolkitHandlerList[Index_EnveResHandler].HandlerBufferSize =  256;
    
	ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable  = FALSE;

    ToolkitHandlerList[Index_ProactiveHandler].HandlerBuffer = ResData+3;  
    
	ToolkitHandlerList[Index_ProactiveHandler].HandlerBufferSize =  253;
    
	ToolkitHandlerList[Index_ProactiveHandler].HandlerAvailable  = FALSE;

	ToolkitHandlerList[Index_ProResHandler].HandlerBuffer = RecData;
    
	ToolkitHandlerList[Index_ProResHandler].HandlerBufferSize =  256;
    
	ToolkitHandlerList[Index_ProResHandler].HandlerAvailable  = FALSE;

	G_PollIntervalRecNum = FALSE;

	G_PostTag = FALSE;
	
	G_ToolkitCurrentAdfName = 0x0000;    //初始化以下变量，防止未初始化的变量使用造成死卡,by xwh 20160127
	
	ToolkitClearChannleAndService();

#if UICC__FUNCTION__SIMAPI
	Toolkit_InitSIMView(0xFF,0xFF);
#endif

//开机时检查EF_ToolkitRFMSmsHead和EF_ToolkitRFMSms，如果已经没有空闲记录，则清空文件，防止RFM文件被占满无法处理的情况,by xwh 20180629
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
	
	fileAddr2 = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
	
	if((fileAddr != FALSE)&&(fileAddr2 != FALSE))
	{
			if((ToolkitFindSpecifiedRFMSMS(MODE_FIND_NOUSE,0x00,0x00,0x00, NULL)==FALSE)||(ToolkitFindSpecifiedHeaderNum(MODE_FIND_NOUSE, 0x00, 0x00,NULL)==FALSE))
			{
				ClearFileConst(fileAddr,0x00);
				ClearFileConst(fileAddr2,0x00);

				fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_STKAlert);
				
				if(fileAddr != FALSE)
				{
				    ClearFileConst(fileAddr,0xFF);		
				}					
			}
	}

}
/*@Function:    handler_check_currenttlv                                            
 * 
 *@Descrip:     检查currenttlv是否有效
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:       handlerIndex---hanlder索引
 *				                                                                                                         
 *@Return:      TRUE成功/FALSE失败                                                                                                                                                                                                                                                                    
 * 
 *@Author:      LJ 20131016
 *  
 *@History:     V0.0---Create by LJ 20131016          
 */

void handler_check_currenttlv(BYTE handlerIndex, BYTE apiMode)
{
	if(ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset == 0xFFFF)
	{	
		if((apiMode == UICCAPI_Mode)&&(handlerIndex == Index_BERTLVEditHandler))
		{
			UICCToolkitWriteBERTLVHandlerContext();
		}
		
		UiccOrSimThrowToolkitException(UNAVAILABLE_ELEMENT_SIM, apiMode);	
	}
}

/*
 *@function     : Toolkit_getValueBytePublic
 *
 *@description  :在当前hander的当前TLV 中返回指定偏移的字节
 *
 *@param        :handlerIndex
 *@param        :offset
 *
 *@return       :BYTE/无效值：-1(即：0xFFFF )
 *
 *@author       :
 *
 */
 BYTE Toolkit_getValueBytePublic(BYTE handlerIndex,WORD offset, BYTE apiMode)
 {
	UWORD xdata dstBuffer;

	handler_check_currenttlv(handlerIndex, apiMode);

	if(offset>=(ToolkitHandlerList[handlerIndex].CurrentTLVLength))
	{
		UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode);
	}

	dstBuffer= (UWORD)(ToolkitHandlerList[handlerIndex].HandlerBuffer+ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset);

    return VMReadByte(dstBuffer+offset);	
 }

/*
 *@function   :Toolkit_getFirstByteOfSpecifiedTag
 *
 *@description: 查找当前handlerIndex中第一次出现的某个特定的tag ,并返回当前TLV的第一字节

 *@param      : tag
 *
 *@return     :the first byte
 *
 *@author     :create by xuyz
 *
 *@history    :V1.0 :create by xuyz
 */
 BYTE Toolkit_getFirstByteOfSpecifiedTag(BYTE handlerIndex, BYTE tag,BYTE apiMode)
 {		
	BYTE idata identifier;

 	ToolkitFindTLVPublic(handlerIndex,tag,1, apiMode);

	handler_check_currenttlv(handlerIndex, apiMode);

	if(ToolkitHandlerList[handlerIndex].CurrentTLVLength == 0x00)
	{
		UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode); 
	}
	
	identifier = Toolkit_getValueBytePublic(handlerIndex,0, apiMode);	

	return(identifier);	
 }

/*
*@function     :findSpecifiedTARValue
*
*@description :检验当前的某个tar在所有安装的toolkit app中是否存在
*
*@param        :tarValue[0x03]
*
*@return       :recNo:返回tar所属应用的记录号
*
*@author       :xuyz
*
*@history      :V1.0 :create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*           :modify by liguiyu@20220705   ,将Toolkit应用的信息放在HEAP中动态分配，返回值改为APPentry的index ，非法时返回FALSE （index = 0与ISD模块entry重叠，但是ISD不是Toolkit应用 不可能存在ToolkitInfo，所以可以将FALSE认为是没有找到，）
*/
BYTE  ToolkitfindSpecifiedTARValue(BYTE *tar)
{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	u16      X_A     i,bFreeEntryIndex;
	u32      X_A     toolkitInfoObj,ToolkitAppInfoAddr;
	u8       X_A     tmp[0x10];  //暂定6F80文件支持 5个TARs: 3*5+1
	u8       X_A     k;

	bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
	{
		toolkitInfoObj = GetToolkitInfoObj(i);

		if(toolkitInfoObj == ADDR_IS_NULL)
		{
			continue;
		}

		ToolkitAppInfoAddr = GetToolkitAppInfoArrayAddr(toolkitInfoObj);

		ecpMem_Copy(&tmp[0],ToolkitAppInfoAddr+TARLength_OFFSET,sizeof(tmp));

		for(k = 0x01; k < tmp[0] ; k += 0x03)
		{
			 if(ecpMem_Compare(&tmp[k], tar, 0x03) == EQUAL)
			 {
				return i;
			 }
		}
	}

	return FALSE;//ILLEGAL_APPID;
#else
	BYTE idata k,j;
	BYTE xdata tmp[0x10];  //暂定6F80文件支持 5个TARs: 3*5+1
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum; 
		
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{	
		return FALSE;	
	}
	
	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(j = 1; j <= toolkitAppRecNum; j++)
	{
		ReadLFBuf(toolkitAppAddr, j ,0x00,&tmp[0],sizeof(BYTE));

		if( tmp[0] == 0xFF)  //非空记录
		{
			continue;
		}

	    ReadLFBuf(toolkitAppAddr, j , TARLength_OFFSET,&tmp[0],sizeof(tmp));

		for(k = 0x01; k < tmp[0] ; k += 0x03)
		{
			 if(ecpMem_Compare(&tmp[k], tar, 0x03) == EQUAL)
			 {
			 	return j;
			 }
		}
	}

	return FALSE;
#endif
}
/*@Function:    handler_check_available                                            
 * 
 *@Descrip:     检查Toolkit hanlder有效性
 *
 *@See:	        ts_143019v050600p.pdf
 *                                                                                                                                     
 *@Param:       handlerIndex---hanlder索引
				                                                                                                         
 *@Return:      TRUE成功/FALSE失败                                                                                                                                                                                                                                                                    
 * 
 *@Author:      LJ 20131011
 *  
 *@History:     V0.0---Create by LJ 20131011          
 */

void handler_check_available(BYTE handlerIndex,BYTE apiMode)
{
	if(ToolkitHandlerList[handlerIndex].HandlerAvailable == FALSE)
	{
	    UiccOrSimThrowToolkitException(HANDLER_NOT_AVAILABLE_SIM, apiMode);
		return;
	}
}

/*@Function:    ClearHandlerCurrentTLV                                            
 * 
 *@Descrip:     清除handler的currentTLV状态
 *
 *@See:	        ts_143019v050600p.pdf
 *                                                                                                                                     
 *@Param:       handlerIndex---hanlder索引
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                                   
 * 
 *@Author:      LJ 20131013
 *  
 *@History:     V0.0---Create by LJ 20131013           
 */
void  ClearHandlerCurrentTLV(BYTE handlerIndex)
{
    ToolkitHandlerList[handlerIndex].CurrentTLVLength = 0xFFFF;
    
	ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset = 0xFFFF;
}

 /*
*@description:从handler里的指定偏移，拷贝一定长度的数据到dstBuffer指定偏移处 ,在调用该函数前，数据的有效性判断已经处理了
*
*@param      :byte handlerIndex
*@param      :word handlerOffset;
*@param      :UWORD dstBuffer
*@param      :word dstOffset
*@param      :WORD copyLength
*
*@return     :void
*
*@author     :xuyz
*
*@history	 :V1.0 :create by xuyz@20131030
*/

void Toolkit_copyHandlerBufferDataToDstBuffer(BYTE handlerIndex,WORD handlerOffset,objref_t dstBuffer,WORD dstOffset,WORD copyLength)
{
	UWORD xdata handlerBuffer;
	UWORD xdata dstAddr;

	if(copyLength == 0x00)
	{
		return;
	}
    
	handlerBuffer = (UWORD)ToolkitHandlerList[handlerIndex].HandlerBuffer+ handlerOffset; //可能为RAM，可能为E2

	dstAddr = GetValidArrayDataAddr(dstBuffer,dstOffset,copyLength);//GetArrayBufferAddress(dstBuffer) + dstOffset;                              //可能为RAM，可能为E2
    
   VMWriteEepromOrRam((BYTE *)dstAddr,(BYTE *)handlerBuffer, copyLength);
}
/*
*@description:  根据appid获取toolkit应用在EF_ToolkitApp文件中的记录号
* *
*@function     :GetToolkitEntryAppNumByAppId
*
*@param         :BYTE appId
*
*@return        toolkitAppNum ，toolkit应用放HEAP以后，返回的是appentry的index，且该应用是toolkit时才反index
*
*@history      :create by lgy@20191204
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE GetToolkitEntryAppNumByAppId(BYTE appID)
{
	BYTE idata  toolkitAppRecNum;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	BYTE idata i,tmp,appId;
	UWORD xdata toolkitAppAddr;

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return FALSE;
	}

	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(i=1; i <=  toolkitAppRecNum;  i++)
	{
		ReadLFBufForJavaAccess(toolkitAppAddr,i,0x00,&tmp,sizeof(BYTE));

		if(tmp == 0xFF) //找到 空记录
		{
			continue;
		}

		ReadLFBufForJavaAccess(toolkitAppAddr,i,AppId_OFFSET,&appId,sizeof(BYTE));

		if(appId == appID)
		{
		   	return  i;
		}
	}
#else
	toolkitAppRecNum		= U16t8(getIndexByID(appID));  //正在安装的应用注册toolkit事件，此时可能还没有创建entry,直接用GetCurrentToolkitEntryArr()获取地址

	if(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(toolkitAppRecNum)) != ADDR_IS_NULL)//是toolkit应用才会创建该数组
	{
		return toolkitAppRecNum;
	}
	else
	{
		if(G_ToolkitAppletInfoRef != REF_NULL)//正在安装的应用，未调用register方法，联通备案测试脚本，11.JavaCard事件处理机制检测/3-1-JavaCard事件处理机制检测/SIM API事件处理机制/SIM_STATUS_COMMAND,安装应用的时候调用RequestPollIntervalPublic接口，此时还没有register，还没有创建entry，导致返回null
		{
			return G_ToolkitAppIndex;//G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];
		}
	}
#endif
	return FALSE;
}
/*
*@description:  由于在处理 ToolkitRegistry等操作时，entry对象为JCRE上下文，实际上要操作的是调用entry的toolkit应用，而不是当前被触发的应用。
                海外平台有测试到：触发toolkit应用A后，在应用A中调用了getshareable(B)，然后去修改B 的菜单名称及状态等
*
*@function     :GetCurrentToolkitEntryAppNum
*
*@param         :void
*
*@return        toolkitAppNum
*
*@history      :create by xuyz@20171127
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE GetCurrentToolkitEntryAppNum(void)
{
	BYTE idata entryId = getPreviousStackContext()&0xFF;

	return GetToolkitEntryAppNumByAppId(entryId);
}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
u32 GetCurrentToolkitEntryArr(void)
{
	u8       X_A    appid;
	u16      X_A    index;
	u32      X_A    toolkitInfoObj,toolkitAppArr;

	appid  =   getPreviousStackContext()&0xFF;

	index = getIndexByID(appid);

	if(index == 0xFFFF)//正在安装的应用注册toolkit事件，此时可能还没有创建entry
	{
		if((appid != bRamBuffer[APP_IN_PROCESS])||(G_ToolkitAppletInfoRef == REF_NULL))//正在安装的应用注册toolkit事件，此时可能还没有创建entry
		{
			return ADDR_IS_NULL;
		}

//		toolkitAppArr			= 	locateObjectInMemory(G_EF_ToolkitAppRef);
//
//		return   get_array_data(MEMtPF8(toolkitAppArr));

		index = G_ToolkitAppIndex;//正在安装的toolkit应用
	}

	toolkitInfoObj = GetToolkitInfoObj(index);

	if(toolkitInfoObj == ADDR_IS_NULL)
	{
		return ADDR_IS_NULL;
	}

	return GetToolkitAppInfoArrayAddr(toolkitInfoObj);

}
#endif
/*
*@funtion     :	UICCToolkitFindSpecifiedAID

*@description :该函数主要用于在6F80/6F83文件中查找特定的纪录，如空记录，或指定AID的记录
*
*@see         :no
*
*@param       :*aidBuf :NULL，表示查找空记；else查找指定AID的记录
*@param       :aidLgth

*
*@return      :TRUE:特定记录已找到/false：未找到指定记录
*
*@author      :xuyz
*
*@history     :V1.0:create by xuyz @20131024
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*
*@others      :其他说明，如该函数使用的限制条件等
*/
BYTE UICCToolkitFindSpecifiedAID(BYTE *aidBuf, BYTE aidLgth)
{
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	BYTE idata i,tmp;
	BYTE xdata buf[18];
	UWORD xdata toolkitAppAddr;
#endif
	BYTE idata  appId,toolkitAppRecNum;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	appId = GPGetAppletIDByAppletAID(aidBuf,aidLgth);

	if(appId != ILLEGAL_APPID)
	{
		toolkitAppRecNum = U16t8(getIndexByID(appId));

		if(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(toolkitAppRecNum)) !=  ADDR_IS_NULL)
		{
			return toolkitAppRecNum;
		}
	}
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return FALSE;
	}
	 
	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(i=1; i <=  toolkitAppRecNum;  i++)
	{	
		ReadLFBufForJavaAccess(toolkitAppAddr,i,0x00,&tmp,sizeof(BYTE));

		if(tmp == 0xFF) //找到 空记录
		{	
			if(aidBuf == NULL)
			{
				return i;
			}

			continue;
		}

		ReadLFBufForJavaAccess(toolkitAppAddr,i,AppId_OFFSET,&appId,sizeof(BYTE));
		
		buf[0] = GetAIDDataByAppID(appId,&buf[1]);

		if((aidLgth == buf[0])&&(NvmCmp(aidBuf,buf+1,aidLgth) == EQUAL))
		{		
		   	return  i;		
		}
	}
#endif
	return FALSE;
}

/*
*@description:查找当前EF_TookitEntry中特定AID和对应的menuID所存在的记录号
*
*@param      : menuId
*
*@returns    :recordId/FALSE
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*            
*/
BYTE ToolKitFindAppRecordNumById(BYTE menuId)
{
	BYTE idata i,recNum,flag;
	BYTE xdata tmp[3] ;
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum;
	
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	recNum = GetCurrentToolkitEntryAppNum();
#else
	recNum = getPreviousStackContext()&0xFF;//当前appid
#endif
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);

	for(i=1; i<= tookitMenuEntryRecNum; i++)
	{
		flag = ReadLFBuf(tookitMenuEntryAddr, i , MenuEntryStatus_OFFSET, tmp,sizeof(tmp));

		if(flag == FALSE)
		{
			continue;
		}

		if((tmp[0] == 0xFF)||((tmp[0]& Menu_UNLOCKED) == Menu_LOCKED))  //NULL or APPLET_LOCKED
		{
			continue;
		}

		if((tmp[AppRecNum_OFFSET] == recNum)&&(tmp[ItemIdentifier_OFFSET] == menuId))
		{
				return i;
		}
	}

	return FALSE;
}

#if UICC__FUNCTION__UICCAPI
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
/*@function:    SearchFileList
 *
 *@descrip:     查询FileList
 *
 *@see:
 *
 *@param:       filelist---需要查找的文件路径
 *@param:		index	---appTableentry 的index

 *
 *@return:      TRUE 找到index处应用注册的 file事件是否与filelist相同 / FALSE 找不到相同的filelist
 *
 *@author:      lgy20220716
 *
 *@history:     V1.0---
 */
BYTE SearchFileList(BYTE* filelist, BYTE index, BYTE mode)
{
	jref     X_A   appletRef;
	jref     X_A   toolkitInfoRef;
	u32      X_A   toolkitInfoObj,FileListObj,FileListArr;
	u8       X_A   fileListTemp[10];

	toolkitInfoObj = GetToolkitInfoObj(index);

	FileListObj = GetFirstToolkitEventFileListObj(toolkitInfoObj, mode);

	if(FileListObj == ADDR_IS_NULL)
	{
		return FALSE;
	}

	while(FileListObj != ADDR_IS_NULL)
	{
		FileListArr = GetToolkitEventFileListArr(FileListObj);

		ReadEeprom(&fileListTemp[0],FileListArr,sizeof(fileListTemp));

		if(ecpMem_Compare(fileListTemp,filelist,10)==EQUAL)
		{
			return TRUE;
		}

		FileListObj = GetNextToolkitEventFileListObj(FileListObj);
	}

	return FALSE;
}
#else
/*@function:    SearchFileList                                            
 * 
 *@descrip:     查询FileList
 *
 *@see:	       
 *                                                                                                                                     
 *@param:       filelist---需要查找的文件路径
 *@param:		blkNum	---需要返回的块号，如果能找到相同的filelist，则返回该filelist的块号，否则，返回空闲块号

 *				                                                                                                         
 *@return:      TRUE 找到相同的filelist / FALSE 找不到相同的filelist                                                                                                                                                                                                                                                                    
 * 
 *@author:      ZZJ 20131119
 *  
 *@history:     V0.0---Create by ZZJ 20131119           
 */

BYTE SearchFileList(BYTE* filelist, BYTE* blkNum)
{
	BYTE xdata fileListTemp[10];
	BYTE idata blk,spaceblk;
	WORD idata totalBlkNum;
	UWORD xdata toolkitFileListAddr;

	toolkitFileListAddr = SelectTFFile(DF_Toolkit_P, EF_ToolkitFileList); 
	
	spaceblk = 0;
	
	totalBlkNum = GetTFSize(toolkitFileListAddr)/10;
	
	for(blk=1;blk<=totalBlkNum;blk++)
	{
		ReadTFBuf(toolkitFileListAddr,(blk-1)*10,fileListTemp,10);
		
		if((spaceblk == 0)&&(fileListTemp[0]==0xFF))
		{
			spaceblk = blk;
		}
		
		if(ecpMem_Compare(fileListTemp,filelist,10)==EQUAL)
		{
			*blkNum =  blk;
			
			return TRUE;
		}
	}
	
	*blkNum = spaceblk;
	
	return FALSE;
}
#endif
#endif
/*
*function     :CopySrcDataToArray
*
*@description:
*
*@param     :
*
*@return    :
*
*@history   :
*/
void CopySrcDataToArray(BYTE* srcbuff,jref arrayref,jshort offset,jshort length)
{
	UWORD xdata arrayaddr;

	arrayaddr = GetValidArrayDataAddr(arrayref,offset,length);//GetArrayBufferAddress(arrayref)+offset;

	VMWriteBuf((UWORD)arrayaddr,(BYTE *)srcbuff,length);
}

/*
*function     :CopyArrayDataToDest
*
*@description:
*
*@param     :
*
*@return    :
*
*@history   :
*/
void CopyArrayDataToDest(BYTE * destaddr,jref arrayref,jshort offset,jshort length)
{
	UWORD xdata arrayaddr;	

	arrayaddr = GetArrayBufferAddress(arrayref)+offset;

	if(IsEepromAddr(arrayaddr))
	{
		ReadEEPromCache(destaddr,arrayaddr,length);
	}
	else
	{
		ecpMem_Move(destaddr,(BYTE *)arrayaddr,length);
		//ecpMem_Move(destaddr,(BYTE *)arrayaddr,length);
	}	
}

/*
*@description: Looks for the Secured Data from the Command Packet in the first SMS TPDU  or Cell Broadcast Page Simple TLV contained in the Envelope handler. This can
*
*@param     : mode: 0x00-return secureDataOffset/ 0x01:return secureDataLen
*
*@return    :secureDataOffset/secureDataLen
*
*@author    :create by xuyz
*
*@history   :V1.0 :Create by xuyz@20131012
*/
WORD ToolkitSubGetSecureDataLenOffset(BYTE mode)
{
	BYTE idata tpoaNum;
	WORD idata offset,secureDataLen,cpl;
	BYTE idata udhl,chl,pntr;
	WORD idata MID;
	BYTE idata udhi;
	jshort idata apiMode = PopOperandStackShort();

	if(G_EnvelopeTag == 0xD2)   //Cell Broadcast Page Simple TLV
	{
		ToolkitFindTLVPublic(Index_EnvelopeHandler,TAG_CELL_BROADCAST_PAGE,1, apiMode);
	
		handler_check_currenttlv(Index_EnvelopeHandler, apiMode);

		offset = 0;

		offset += 2;    //skip Serial Number(2),point to MID

		MID = VMReadShort((UWORD)((ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer)+(ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset)+offset));
		
		offset += 4;	 //skip Message Identifie(2),DCS(1),page number(1),point to cpl

		if(mode == MODE_MESSAGE_OFFSET)
		{
			return offset;
		}

		if(mode == MODE_MESSAGE_LENGTH)
		{
			secureDataLen = (ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength)-offset; 
	
			return secureDataLen;
		}

		if ((MID < CBS_FORMATTED_MIDMIN)||(MID > CBS_FORMATTED_MIDMAX)) 
		{ 
			UiccOrSimThrowToolkitException(UNAVAILABLE_ELEMENT_SIM, apiMode);
		}

		cpl = VMReadShort((UWORD)((ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer)+(ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset)+offset));

		offset += 2;	 //CPL(2)

		chl = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset, apiMode);

		pntr = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset+13, apiMode);
        
		offset += (chl+1);
		
		if(mode == MODE_SECURE_OFFSET)
		{
			return offset;
		}

		secureDataLen = cpl - (chl+1) - pntr; 

		return(secureDataLen);
	}
	else if(G_EnvelopeTag == 0xD1)	 //SMS TPDU
	{
		ToolkitFindTLVPublic(Index_EnvelopeHandler,TAG_SMS_TPDU,1, apiMode);   //
	
		udhi = Toolkit_getValueBytePublic(Index_EnvelopeHandler,0, apiMode);
				
		tpoaNum = Toolkit_getValueBytePublic(Index_EnvelopeHandler,1, apiMode);
	
		offset = 12+(tpoaNum+1)/2;   //1+1+1+(tpoaNum+1)/2+1+1+7; //跳过MTI,TPOA字符个数，TON/NPI,TPOA,PID,DCS,TP_SCTS，指向UDL
			
		offset++;                                  //指向UDHL
					
		if(Check_UDHI(udhi)==TRUE)
		{
			udhl = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset, apiMode);

			offset+=(1+udhl);           	           //跳过UDHL,UDH,指向CPL
		}
		
		if(mode == MODE_MESSAGE_OFFSET)           //It is the offset of the first byte after the UDH, if any.
		{
			return offset;
		}

		if(mode == MODE_MESSAGE_LENGTH)
		{
			secureDataLen = (ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength)-offset; 
	
			return secureDataLen;
		}

		if((G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_UNFORMATTED_SMS_PP_ENV)||(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_UNFORMATTED_SMS_PP_UPD_bit))
		{
			UiccOrSimThrowToolkitException(UNAVAILABLE_ELEMENT_SIM,apiMode);
		}	

		offset += 2;                                 //跳过CPL,指向CHL
	
		chl = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset, apiMode);
	
		pntr = Toolkit_getValueBytePublic(Index_EnvelopeHandler,offset+13, apiMode);
	
		offset+=(1+chl);                     //跳过chl,ch,指向secure data
	
		if(mode == MODE_SECURE_OFFSET)
		{
			return offset;
		}
	
		secureDataLen = (ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength)-offset-pntr; 
	
		return secureDataLen;
	}
	else  //USSD String TLV
	{
		//Send an Unrecognized Envelope with neither TPDU TLV nor Cell Broadcast Page TLV, then call the method. ToolkitException.UNAVAILABLE_E LEMENT
		if((mode == MODE_MESSAGE_OFFSET)||(mode == MODE_MESSAGE_LENGTH))
		{
			UiccOrSimThrowToolkitException(UNAVAILABLE_ELEMENT_SIM,apiMode);
		}
		
		return 0xFFFF;		
	}	
}
/*
*@description:获取当前TPUDL的偏移
*
*
*@return     :offset
*
*@author      :create by xuyz
*
*@history     :V1.0: create by xuyz@20131012
*/
WORD Toolkit_subTPUDLOffset(void)
{
	BYTE idata tpoaNum;
	WORD idata offset;
	jshort idata apiMode = PopOperandStackShort();

	ToolkitFindTLVPublic(Index_EnvelopeHandler,TAG_SMS_TPDU,1,apiMode);

	tpoaNum = Toolkit_getValueBytePublic(Index_EnvelopeHandler,1, apiMode);

	offset = 12+(tpoaNum+1)/2;		//	 1+1+1+(tpoaNum+1)/2+1+1+7; //MTI,TPOA字符个数，TON/NPI,TPOA,PID,DCS,TP_SCTS

	return(offset);	
}
/*
*@description:判断当前envelopResponse的事件是正确
*
*@param      :void
*
*@return     :true:tag有效/false:tag 无效
*
*@history
*/
void ToolkitAvaiableOfEnvelopResponse(BYTE apiMode)
{
	switch(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel])
	{
		case  EVENT_CALL_CONTROL_BY_NAA:
		case  EVENT_FORMATTED_SMS_PP_ENV:
		case  EVENT_UNFORMATTED_SMS_PP_ENV:
		case  EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM:
    	case  EVENT_UNRECOGNIZED_ENVELOPE:   
			return ;
		default:
			UiccOrSimThrowToolkitException(HANDLER_NOT_AVAILABLE_SIM, apiMode);
			return;	
	}	
} 

/*@Function:    check_profile_available                                            
 * 
 *@Descrip:     检查profile数据是否有效
 *
 *@See:	        ts_143019v050600p.pdf/ts102241
 *                                                                                                                                     
*@apimode   : 该函数为simapi和usimapi公共函数，apimode表示调用siaapi调用或usimapi调用
 *				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                                  
 * 
 *@Author:      LJ 20131016
 *  
 *@History:     V0.0---Create by LJ 20131016
 *@history     :V1.1: modify by lj@20140225，添加参数apimode           
 */

void check_profile_available(BYTE apiMode)
{
	if(G_TerminalProfile[0] == 0x00)
	{	
		UiccOrSimThrowToolkitException(ME_PROFILE_NOT_AVAILABLE_SIM, apiMode);
	}
}


/*
*@description:检查当前index在terminal profile是否为TRUE
*
*@param      :jshort: index
*@return      :TRUE/FALSE
*
*A@author     :create by xuyz
*
*@history     :V1.0: create by xuyz
*/
BYTE Toolkit_publicProfileCheck1(jshort index,BYTE apiMode)
{
	BYTE   sIndex,sBit;

	if(index < 0x00)
	{
		UiccOrSimThrowToolkitException(BAD_INPUT_PARAMETER_SIM, apiMode);
	}

	check_profile_available(apiMode);

	if((index/8) >= G_TerminalProfile[0])
	{
		return 0x00;
	}
	else
	{
		sIndex = index/8;	   //第sIndex个字节
		sBit  = index&0x07;    //字节中的第n个bit

		return ValueBit(G_TerminalProfile[1+sIndex],sBit);
	}	
}

/**
 *@fucntion    : ProactivecmdPermission
 *
 *@description : 发主动式命令之前framework检查是否被允许
 *
 *@see          :TS 102 241 :interface ProactiveHandler
 *
 *@param        :void
 *
 *@author       : ZZJ
 *
 *@hisotry      :V1.0 :create by ZZJ@20131030
 *           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 */

BYTE ProactivecmdPermission(void)
{
   BYTE  idata id,i,tmp[2];
   UWORD xdata toolkitAppAddr;

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
   toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]));
#else
   toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
   if(toolkitAppAddr == FALSE)
	{	
		return FALSE;	
	}
	 if(G_CurContext == getChannelContext(G_CurSelectedChannel)/*ChannelManagement[G_CurSelectedChannel].context*/)
	 {
		 return FALSE;   //因为只有在(U)SIM应用中才会触发应用，send函数必须在triger applet的processToolkit中才能被调用，所以此时的G_CurContext一定不能等于通道上下文，如果等于则认为在Toolkit的Process方法中调用了send函数，程序不允许这样操作，报错
	 }
	
   if((ResData[6] == CMD_SETUPMENU)||(ResData[6] == CMD_SETUPEVENTLIST)||(ResData[6] == CMD_POLLINTERVAL)||(ResData[6] == CMD_POLLINGOFF))
   {
	    return FALSE;	  //不允许应用层触发一些主动式命令
   }
   else if(ResData[6] == CMD_TIMERMANAGEMENT)
   {
       tmp[0] = FindTag(&ResData[3],TLV_TIMERIDENTIFIER,ToolkitHandlerList[Index_ProactiveHandler].TLVListLength);

	   if(tmp[0] == FALSE)
	   {
	   	   return FALSE;
	   }

      id =  ResData[3 + tmp[0] + 1];//timerid

	  ReadLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],AllocateTimerCount_OFFSET,&tmp[0],1);

	  if(ValueBit(tmp[0],(id-1)) != TRUE)
	  {
	     return FALSE;   //禁止通过未设置的timerid来发timermanage主动式命令
	  }
   }
   else if((ResData[6] == CMD_CLOSECHANNEL)||(ResData[6] == CMD_SENDDATA)||(ResData[6] == CMD_RECEIVEDATA))
   {
   	   id = ResData[11] - 0x20;  //获取close channeld的通道ID 
       
	   ReadLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],AllocateChannelCount_OFFSET,&tmp[0],1);

	   if(ValueBit(tmp[0],id) != TRUE)
	   {
		  return FALSE;		   //逻辑通道未开通，禁止通过该通道发送命令
	   }

	   return id;
	   
   }
   else if(ResData[6] == CMD_OPENCHANNEL)
   {
       id = 0;

       ReadLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],MaxChannelCount_OFFSET,&tmp[0],2);

	   for(i = 1;i < 8;i++)
	   {
	      if(ValueBit(tmp[1], i) == TRUE)
		  {
		     id++;
		  }
	   }

	   if(tmp[0] <= id)
	   {
	   		    
		     return FALSE;   //逻辑通道已满，禁止继续开通
	   }
   }
   else if(ResData[6] == CMD_DECLARE_SERVICE)
   {
   		tmp[0] = FindTag(&ResData[3],0x41,ToolkitHandlerList[Index_ProactiveHandler].TLVListLength);

	   if(tmp[0] == FALSE)
	   {
	   	   return FALSE;
	   }
	   id =  ResData[3 + tmp[0] + 2];//serviceid

   	   ReadLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],AllocateService_OFFSET,&tmp[0],2);
		
	   if(ValueBit(tmp[0],id) != TRUE)  //当前serviceId没有分配给该应用
	   {
	   		return FALSE;
	   }	

	   if((ResData[7]&0x01) == 0x00)  //add a new service to the terminal service database;
	   {
	   		if(ValueBit(tmp[1],id) == TRUE)	    //当前服务已经启用
			{
				return FALSE;
			}
	   }
	   else		    //delete a service from the terminal service database.
	   {
	   	  	if(ValueBit(tmp[1],id) != TRUE)	    //当前服务暂未启用
			{
				return FALSE;
			}
	   }

	   return (id+1);  //返沪值为1-8
   }
   else //白盒测试
   {
   }

   return TRUE;
}
/*
*@description: 若收到基于open channel or close channel成功的termialResponse，则对相应的应用注册或清除channelId
*
*@param 	 :Id 
*@param      :mode :0x00：设置相应的channelId/serviceId /0x01：清除channelId/serviceId
*@param      :offset: AllocateChannelCount_OFFSET/CurrentService_OFFSET
*
*@return  	 :TRUE
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE SetToolkitChannelServiceIdStatus(BYTE Id,  BYTE mode,WORD offset)
{
	BYTE idata tmp;
	UWORD xdata toolkitAppAddr;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
   toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
#endif
	if(toolkitAppAddr == FALSE)
	{	
		return TRUE;	
	}
	
	ReadLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],offset,&tmp,0x01);	  //获取当前channel/service设置
	
	if(mode == 0x00)     //mode只有两个值：0x00和0x01，去掉编译警告,by xwh 20140618
	{
		SetBit(tmp,Id);
	}
	else//  if(mode == 0x01)
	{
		ClearBit(tmp,Id);
	}

	WriteLFBufForJavaAccess(toolkitAppAddr,G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],offset,&tmp,0x01);

	return TRUE;
}
/*@Function:    check_stk_busy                                            
 * 
 *@Descrip:     当处于STK流程时，不允许对Res进行操作
 *
 *@See:	        ts_143019v050600p.pdf
 *                                                                                                                                     
 *				                                                                                                         
 *@Return:      TRUE成功/FALSE失败                                                                                                                                                                                                                                                                    
 * 
 *@Author:      LJ 20131021
 *  
 *@History:     V0.0---Create by LJ 20131021         
 */
void checkStkBusy(BYTE apiMode)
{
	//STKPerform!=FALSE的情况下，MENU_SELECTION可以打断当前STK流程进入菜单(防止无14返回无法进入菜单的情况)
	if((G_STKBusy != FALSE)||((STKPerform[0]!=FALSE)&&(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] != EVENT_MENU_SELECTION_HELP_REQUEST_bit)&&(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] != EVENT_MENU_SELECTION_bit)))
	{
		UiccOrSimThrowToolkitException(HANDLER_NOT_AVAILABLE_SIM, apiMode);
		return;
	}
}
/*@Function:    Toolkit_subGetOtherDataLength                                            
 *
 *@description:返回第一个指定tag除第一字节的后续数据长度,主要针对testString  resultAddtionalData
 *
 *@param      :tag
 *@return     :TextStringLength
 *
 *@author      :create by xuyz
 *
 *@history     :V1.0 :Create by xuyz@20131012

*/
WORD Toolkit_subGetOtherDataLength(BYTE tag)
{
	WORD idata length;
	
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	ToolkitFindTLVPublic(Index_ProResHandler,tag,1, apiMode);

	handler_check_currenttlv(Index_ProResHandler, apiMode);
	
    length =(ToolkitHandlerList[Index_ProResHandler].CurrentTLVLength > 0)?(ToolkitHandlerList[Index_ProResHandler].CurrentTLVLength - 1):0x00;

    return(length);	
}

/*@Function:    Toolkit_copyTagDataToDstBuffer                                            
 *
 *@description:从当前response data中第一个指定tag的offset偏移处,拷贝后续所有的数据到dstbuff中
 *
 *@param      :	objref_t dstBuffer,
 *@param      : jshort dstOffset, 
 *@param      :jshort dstLength
 *@param      :byte tag
 *@param      :BYTE offset
 
 *@return     :void
 *
 *@author     :create by xuyz
 *
 *@history    :V1.0:create by xuyz@20131012
*/
jshort Toolkit_copyTagDataToDstBuffer(BYTE tag,BYTE offset)
{
	jref dstBuffer; 
	jshort dstOffset; 
	jshort dstLength;
	jshort apiMode;

	apiMode =  PopOperandStackShort();
	dstLength =  PopOperandStackShort();
	dstOffset =  PopOperandStackShort();
	dstBuffer =  PopOperandStackShort();
	
	ToolkitFindTLVPublic(Index_ProResHandler,tag,1, apiMode);

	handler_check_currenttlv(Index_ProResHandler, apiMode);

	if(dstLength>(ToolkitHandlerList[Index_ProResHandler].CurrentTLVLength - offset)) //长度去掉一字节result
	{
	    UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode);
	}

	check_bounds_interval_buffer(dstBuffer, dstOffset, dstLength);	  //异常：ArrayIndexOutOfBoundsException

	Toolkit_copyHandlerBufferDataToDstBuffer(Index_ProResHandler,ToolkitHandlerList[Index_ProResHandler].CurrentTLVValueOffset+offset,dstBuffer,dstOffset,dstLength);
	
	return(dstOffset+dstLength);	
}
/*
 *@description:从当前handler中当前TLV的特定偏移处的数据拷贝到目标buf中
 *
 *@param      :handlerIndex
 *@param      :	valueOffset
 *@param      :dstBuffer
 *@param      :dstOffset
 *@param      :dstLength
 *@param      :mode

 *@return     :void
 *
 *@author     :create by xuyz
 *
 *@history    :V1.0 :create by xuyz
 */
 jshort Toolkit_BaseCopyDataToDstBuffer(jbyte handlerIndex, jshort valueOffset, objref_t dstBuffer, jshort dstOffset, jshort dstLength,BYTE mode, BYTE apiMode)
 {
 	handler_check_available(handlerIndex, apiMode);   //检查hanlder是否有效
 	 
	handler_check_currenttlv(handlerIndex, apiMode);	 //抛出异常	 UNAVAILABLE_ELEMENT

 	check_bounds_interval_buffer(dstBuffer, dstOffset, dstLength);	  //异常：ArrayIndexOutOfBoundsException
			
	if(mode != 0x01)   //Toolkit_findAndCopyValue_3 不需要进行以下判断，该函数的拷贝长度为ToolkitHandlerList[handlerIndex].CurrentTLVLength
	{
		if((valueOffset<0)||(valueOffset >= ToolkitHandlerList[handlerIndex].CurrentTLVLength)||(dstLength>ToolkitHandlerList[handlerIndex].CurrentTLVLength)||((valueOffset+dstLength)>ToolkitHandlerList[handlerIndex].CurrentTLVLength))
		{
			if(mode != 0x00) //Toolkit_copyValue 不需要进行以下操作
			{
				ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset = 0xFFFF;
			}

			if((apiMode == UICCAPI_Mode)&&(handlerIndex == Index_BERTLVEditHandler))
			{
				UICCToolkitWriteBERTLVHandlerContext();
			}

			UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode);			
		}
	}

	Toolkit_copyHandlerBufferDataToDstBuffer(handlerIndex,ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset+valueOffset,dstBuffer,dstOffset,dstLength);
	
	return((jshort)(dstOffset+dstLength));			
 }
 /*
 *@function    :ToolkitFindTLVPublic
 *
 *@description :在当前handerIndex指定的TLV 列表中查找指定的出现次数的tag,偏移从0开始
 *
 *@param     :handlerIndex
 *@param     :tag
 *@param     :occurrence
 *
 *@return    :TLV_FOUND_CR_SET：TLV_FOUND_CR_NOT_SET 成功后，设置当前tlvOFFSET和TLVLength/无效值：
 *
 *@author    :create by xuyz
 *
 *@HISTORY	 :V1.0: create by xuyz
 *           :V1.1: modify by XWH
 *           :V1.2: modify by LIUJIE
 */
 BYTE ToolkitFindTLVPublic(BYTE handlerIndex,BYTE tag,BYTE occurrence,BYTE apiMode)
 {
 	WORD idata tagLength;
 	WORD idata offset;
	BYTE idata tmpNumber;
	BYTE xdata tmp[0x04];	 //tmp[0] :tag tmp[1]:lgth or other bytes	
	UWORD xdata dstBuffer;

	if(occurrence == 0x00)
	{
		if(handlerIndex == Index_BERTLVEditHandler)
		{
			UICCToolkitWriteBERTLVHandlerContext();
		}
		
		ClearHandlerCurrentTLV(handlerIndex);               //if not successful,no TLV is selected	
		
		UiccOrSimThrowToolkitException(BAD_INPUT_PARAMETER_SIM,apiMode);
	}

	tagLength = 0x00;

	tmpNumber = 0x01;

	ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset = 0xFFFF;    //初始化偏移

	dstBuffer= (UWORD)(ToolkitHandlerList[handlerIndex].HandlerBuffer);

	for(offset = 0;offset<ToolkitHandlerList[handlerIndex].TLVListLength;)
	{
		VMReadBuf((BYTE *)&tmp[0],dstBuffer+offset,4);

		/*
		if(tmp[1] < 0x80)
		{
			offset +=  2;
			tagLength = tmp[1];
		}
		else if(tmp[1] == 0x81)
		{			
			tagLength = tmp[2];			
			offset += 3;
		}
		else if(tmp[1] == 0x82)
		{							   
			tagLength = (((WORD)tmp[2] << 8)&0xFF00) + tmp[3];
			offset += 4;
		}
		else   //支持最大2字节length(82XXXX),超出2字节当作异常处理
		{
			ClearHandlerCurrentTLV(handlerIndex);               //if not successful,no TLV is selected	
		
			if(G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] == UICCAPI_Mode)
		    {
				UICCThrowToolkitException(BAD_INPUT_PARAMETER);	  
			}
#if UICC__FUNCTION__SIMAPI
			else
			{
				ThrowToolkitException(BAD_INPUT_PARAMETER_SIM);
			}
#endif
		}
 */
		//白俄罗斯项目在 SAMSUNG NOTE3上发现手机兼容性问题，在SENDUSSD后，14的响应数据中0D tag为9A 的时候，前面没有带81，导致卡片不能处理
		//如下所示：A0140000A8 810301120082028281830100  0D 9A 00DA30F9CDD6A3CB6EF77B4EEFD55CB89B1CE40C8FD1E939BBEC4EE7C320F81BE47EB7CBF2BA0E0473C1607290B4BC6687DB6117E84987CBC3F6934E11F3A962B1980A371B815A206B797E02ADC3F430FB7D06B5EBFAFC3ACC3EB9F3681032497D5A41FA3048C682E140F2BA5821F325E7F4B73C9D0F83DEE2FC18EDCEA341F6F21C3D4697D521D08DDC82C9592072985D2EB7602CD84DFE2203
    //因此做以下修改
		if(tmp[1] == 0x81)  //
		{			
			tagLength = tmp[2];			
			offset += 3;
		}
		else if(tmp[1] == 0x82)
		{							   
			tagLength = (((WORD)tmp[2] << 8)&0xFF00) + tmp[3];
			offset += 4;
		}
		else    
		{
				offset +=  2;
				tagLength = tmp[1];  
		}
		
		if((tmp[0]&TAG_SET_NO_CR) == (tag&TAG_SET_NO_CR))	    
		{
			if(tmpNumber == occurrence)
			{
				ToolkitHandlerList[handlerIndex].CurrentTLVLength =  tagLength;
				ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset = offset;
	
				if((tmp[0]&TAG_SET_CR) == 0x00)
				{
					return TLV_FOUND_CR_NOT_SET;
				}
				else
				{
					return TLV_FOUND_CR_SET;
				}
			}
			tmpNumber ++;														     
		}
																		   
		offset += tagLength ;	

	}

	ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset = 0xFFFF;

	return TLV_NOT_FOUND;	 
 }
/*
 *@description:从当前handler中当前TLV的特定偏移处的数据与目标buf中的数据进行比较
 *
 *@param      :handlerIndex
 *@param      :	valueOffset
 *@param      :dstBuffer
 *@param      :dstOffset
 *@param      :dstLength
 *@param      :mode

 *
 *@return     :void
 *
 *@author     :create by xuyz
 *
 *@history    :V1.0 :create by xuyz
 */
 jbyte Toolkit_BaseCompareDstBuffer(jbyte handlerIndex, jshort valueOffset, objref_t compareBuffer, jshort compareOffset, jshort compareLength,BYTE mode, BYTE apiMode)
 {
 	jbyte idata result;
	UWORD xdata handlerBufferData;
	DWORD xdata dstBuffer;

	handler_check_available(handlerIndex, apiMode);

	handler_check_currenttlv(handlerIndex, apiMode);

	if(mode != 0x01)	//Toolkit_findAndCompareValue_3(jbyte handlerIndex, jbyte tag, objref_t compareBuffer, jshort compareOffset)，不需要进行以下判断
	{
		if((valueOffset<0)||(valueOffset >= ToolkitHandlerList[handlerIndex].CurrentTLVLength)||(compareLength>ToolkitHandlerList[handlerIndex].CurrentTLVLength)||((valueOffset+compareLength)>ToolkitHandlerList[handlerIndex].CurrentTLVLength))
		{
			if(mode != 0x00)   //Toolkit_compareValue ，不需要重置当前TLV
			{
				ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset = 0xFFFF;
			}

			if((apiMode == UICCAPI_Mode)&&(handlerIndex == Index_BERTLVEditHandler))
			{
				UICCToolkitWriteBERTLVHandlerContext();
			}

			UiccOrSimThrowToolkitException(OUT_OF_TLV_BOUNDARIES_SIM, apiMode); 
		}
	}

	dstBuffer = GetValidArrayDataAddr(compareBuffer, compareOffset, compareLength);	  //异常：ArrayIndexOutOfBoundsException	

	handlerBufferData = (UWORD)ToolkitHandlerList[handlerIndex].HandlerBuffer+ ToolkitHandlerList[handlerIndex].CurrentTLVValueOffset+valueOffset;	

	result = NvmCmp((BYTE *)dstBuffer, (BYTE *)handlerBufferData, compareLength);

	return(result);	
 }
#endif
/************************************************************
* func name: JavaRAM_Main
* para : 无
* discript:java主入口
* return ：无
************************************************************/
WORD JcvmProcessRemoteAppManageAPDU(void)
{
   	WORD idata sw = 0;
#if M2M_EUICC
   	BYTE idata backProfileIndex = G_CurExecutingProfileIndex;
#endif
   	ecpMem_Copy(APDUBuffer,G_APDU,5);
	
	G_GPErrorCode = 0x9000;  
	
	//UICC 层不会开启事务
	if(TransactionGetDepth() == TRUE)  //在进入java层，如果开启了事务，但是事务没有提交，则在退出 java层时abort 事务
	{
		TransactionAbort(); 
	}
#if M2M_EUICC
	if(((G_TARRecIndex[0] == TarType_ISDP)&&(G_TARRecIndex[1] != G_CurExecutingProfileIndex))
	||(G_TARRecIndex[0] == TarType_ISDR))//ISDR storedata 给ISDP 或者ISDR自己的RAM
	{
		if(G_TARRecIndex[0] == TarType_ISDP)
		{
			G_CurExecutingProfileIndex = G_TARRecIndex[1];//G_TARRecIndex[1]是profileIndex
		}
		else //if(G_TARRecIndex[0] == TarType_ISDR)  //(G_M2MStoreData[1] ！= ILLEGAL_PROFILEINDEX)时表示通过ISDR执行了installforpersonal,给ISDP建立秘钥 需要将执行区域切到ISDP
		{
			if((G_M2MStoreData[1] != ILLEGAL_PROFILEINDEX)&&(APDUBuffer[1] != 0xE2)) // installfor personal(ISDR-->ISDP)/storedata 3A01 /storedata 3A02 不连续，打断storedata ，直接交给ISDR处理
			{
				ecpMem_Set(G_M2MStoreData, 0xFF, sizeof(G_M2MStoreData));

				G_M2MKeySetState = FALSE;

				g_moreCommands = CHAINING_CLEAR;
			}

			G_CurExecutingProfileIndex = G_M2MStoreData[1];// (G_M2MStoreData[1] == ILLEGAL_PROFILEINDEX)时 表示ISDR自己的RAM ，需要将执行区域切到SE
		}

		ExecuEnvironmentInit(G_CurExecutingProfileIndex); //ISDP的普通RAM不涉及DTR RTR 不切换
	}

#endif

	GPRunIntoJavaFunction(JCREMain_RAMMode);
	
	if(TransactionGetDepth() == TRUE)  //在进入java层，如果开启了事务，但是事务没有提交，则在退出 java层时abort 事务
	{
		TransactionAbort();    //比如应用installforinstall中，在安装时会开启事务，如果安装失败，java层没有abort事务，在此处ablort
	}

#if M2M_EUICC
	if(backProfileIndex != G_CurExecutingProfileIndex)
	{
		G_CurExecutingProfileIndex = backProfileIndex;
		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	}
#endif

	sw = G_GPErrorCode;

	G_GPErrorCode = 0x9000;

	return sw;
}

/************************************************************
* func name: GetArrayAddress
* para : 无
* discript:	获取指定的java数组的元素存放地址
* return ：	数组第一个元素的地址
************************************************************/
DWORD GetArrayBufferAddress(jref refrence)
{
	PF8  X_A  objAddr;

	objAddr  = MEMtPM8(locateObjectInMemory(refrence));//获取对象的地址

	objAddr = MEMtPF8(get_array_data(objAddr));

	return (DWORD)objAddr;
}
/*
*@function   : ThrowSIMViewException
*
*@description: 该函数主要用于COS层抛出TS43 019 所指定的SIMViewException
*
*@param      : reason
*
*@return     :void
*
*@author
*/
void ThrowSIMViewException(jshort reason)
{
	ThrowError(SIM_VIEW_EXCEPTION_MASK|reason);
}
/*
*@function   : ThrowToolkitException
*
*@description: 该函数主要用于COS层抛出TS43 019 所指定的Toolkit Exception
*
*@param      : reason
*
*@return     :void
*
*@author
*/
void ThrowToolkitException(jshort reason)
{
	ThrowError(SIM_TOOLKIT_EXCEPTION_MASK|reason);
}
/*
*@function   : UICCThrowToolkitException
*
*@description: 该函数主要用于COS层抛出TS102 241 所指定的Toolkit Exception
*
*@param      : reason
*
*@return     :void
*
*@author
*/
void UICCThrowToolkitException(jshort reason)
{
	ThrowError(TOOLKIT_EXCEPTION_MASK|reason);
}
/*
*@function   : UICCThrowUICCException
*
*@description: 该函数主要用于COS层抛出TS102 241 所指定的UICC Exception
*
*@param      : reason
*
*@return     :void
*
*@author
*/
void UICCThrowUICCException(jshort reason)
{
	ThrowError(UICC_EXCEPTION_MASK|reason);
}

/*
*@function   : UICCThrowAdminException
*
*@description: 该函数主要用于COS层抛出TS102 241 所指定的Admin Exception
*
*@param      : reason
*
*@return     :void
*
*@author
*/
void UICCThrowAdminException(jshort reason)
{
	ThrowError(ADMIN_EXCEPTION_MASK|reason);
}

/*
*@function   : UICCThrowSystemException
*
*@description: 抛出系统异常
*
*@param      : reason
*
*@return     :void
*
*@history    :
*/
void UICCThrowSystemException(jshort reason)
{
	ThrowError(SYSTEM_EXCEPTION_MASK|reason);
}

/*
*@function   : ThrowAPDUException
*
*@description: 抛出APDU系统异常
*
*@param      : reason
*
*@return     :void
*
*@history    :
*/
void ThrowAPDUException(jshort reason)
{
	ThrowError(APDU_EXCEPTION_MASK|reason);
}

/*
*@funtion    : check_bounds_interval
*
*@description:	检查当前数组的有效性，如 objref是否为NULL; offset  ,length 是否《0 ，其长度是否大于buffer本身的长度
*				注意：  该函数操作的都是BYTE型数组 ，length是指字节数
*@param      : objef
*@param      : offset
*@param      : length
*
*@return     :exception
*
*@author     :
*
*@history    :
*/
void check_bounds_interval_buffer(WORD objref,SWORD offset,SWORD length)
{
	u16 D_A   array_Len;
	u32 D_A   array_ref ;

	if(objref == REF_NULL)
	{
		throw_error(NULL_POINTER_EXCEPTION);
	}

	array_ref =  locateObjectInMemory(objref);

	array_Len = get_array_length(array_ref);

	if((offset < 0)||(length < 0)||((offset + length) > array_Len))
	{
		throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
	}
}
/*
*@function : GetValidArrayDataAddr
*
*@description:判断当前数组对象的有效性，然后返回待操作的物理地址
*
*@param      :WORD arrayRef
*@param      :SWORD soffset
*@param      :SWORD sLENGTH
*
*@returb     :operateAddr
*/
UWORD GetValidArrayDataAddr(WORD arrayRef, SWORD soffset,SWORD  sLength)
{
	UWORD xdata operateAddr;
	
	check_bounds_interval_buffer(arrayRef,soffset,sLength);
    
    operateAddr = GetArrayBufferAddress(arrayRef) + soffset;  

	return operateAddr;
}
/*
*@function     :
*
*@description  :根据当前AID调用其相应的getShareInterfaceObjecti及processToolkit接口函数
*
*@param        :AID
*@param        :AIDLen
*@param        :event
*@param        :trigerMode :0x01:SIM ToolkitApp mode/0x02: UICC ToolkitAPP mode
*
*@return       :TRUE/FALSE
*
*@author       :
*
*@history	   :modify by xuxiaojun@20200720 增加G_TrigerToolkitRef的清除操作，避免触发应用G_TrigerToolkitRef被赋上有效值，但是后续却没有清除，导致下一个是NM_scp81ConnectionOpen或NM_GPRequestHTTPAdministrationSession时，triggeredAppId取值错误
*              :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                            用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*@others       :getShareInterfaceObjecti的参数如下(NULL,0X01)
*/
BYTE CallToolkitApplet(BYTE recNum,SBYTE event,BYTE trigerMode)
{
	BYTE idata appId;
	UWORD xdata toolkitAppAddr;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{	
		return TRUE;	
	}

	ReadLFBuf(toolkitAppAddr,recNum,AppId_OFFSET,&appId,sizeof(BYTE));
#else
	appId  =  getAppIDByIndex(recNum);//执行到CallToolkitApplet recNum指向的应用一定是一个Toolkit应用，不需要做其他判断
#endif
	G_TrigerToolkitRef = GetAppletRefByAppId(appId);	 //为兼容在触发应用app1 里触发app2，在触发app2 时，需要保存该变量

	if(TransactionGetDepth() == TRUE)		            //At the invocation of the  processToolkit()  method there shall be no transaction in progress.
	{
		TransactionAbort();
	}

	GPRunIntoJavaFunction(trigerMode);                  //如果在processToolkit中有未捕获的异常，则不再往外抛出，参考用例UICC API :Cre_Exh_Imtg.在processToolkit中throw new NullpointerException, 期望返回9000

	if(TransactionGetDepth() == TRUE)		            //Termination of a Toolkit Applet (return from the  processToolkit()  method): -   A pending Toolkit Applet transaction is aborted. 
	{
		TransactionAbort();
	}
	else
	{
		CacheFlush();
	}

	G_TrigerToolkitRef = NULL_REF;

	return TRUE;
}


/*
*@description:判断数组中的对应bit是否置1
*
*@param      :buf ,bit
*
*@return     :TRUE/FALSE
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz
*/
BYTE GetBufBit(BYTE *buf, BYTE bit)
{
	BYTE i,index;

	i = bit/8;  //在数组中的第几字节

	index = bit%8; // 字节中的第几bit(bit0 -bit7)

	return ValueBit(buf[i],index);
}
/*
*@description:将数组中的某一bit置1
*
*@param      :buf ,bit
*
*@return      :void
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz   
*/
void SetBufBit(BYTE *buf, BYTE bit)
{
	BYTE i,index;

	i = bit/8;  //在数组中的第几字节

	index = bit%8; // 字节中的第几bit(bit0 -bit7)

	SetBit(buf[i],index);
}
/*
*@description:清除数组中的某一bit
*
*@param      :buf ,bit
*
*@return      :void
*
*@author     :create by xuyz
*
*@hisoty     :V1.0: create by xuyz   
*/
void ClearBufBit(BYTE *buf, BYTE bit)
{
	BYTE i,index;

	i = bit/8;  //在数组中的第几字节

	index = bit%8; // 字节中的第几bit(bit0 -bit7)

	ClearBit(buf[i],index);
}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP

/*
*@description:AddEFToolkitAppInfo,新增加一个toolkitApplet应用的信息
*
*@param      :WORD newCC,BYTE* srcData,WORD arrayLen
*
*@return      :WORD
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu
*/
WORD AddEFToolkitAppInfo(WORD newCC,BYTE* srcData,WORD arrayLen)
{
	WORD  backCC,ref,ToolkitAppletInfoRef;
	UWORD toolkitAppAddr;

	backCC = cc;
	cc = newCC;

	ToolkitAppletInfoRef = newObject(TOOLKIT_APPLETINFO_CLASS_OFFSET,UICC_PKG_ID,BIT_NEED_NOTHING);
	if(MAP_TABLE_ILLEGAL_INDEX == ToolkitAppletInfoRef)
	{
		cc = backCC;
		return REF_NULL;
	}
	ref = _newarrayObject(AType_Byte,arrayLen , BIT_NEED_REFRESH);//get_array_data直接访问物理地址，对象头信息在cache时会出错
	cc = backCC;

	if(MAP_TABLE_ILLEGAL_INDEX == ref)
	{
		return REF_NULL;
	}

	toolkitAppAddr = locateObjectInMemory(ref);
	toolkitAppAddr = get_array_data(MEMtPF8(toolkitAppAddr));

	memCopyAtomic(MEMtPM8(toolkitAppAddr),srcData, arrayLen);

	toolkitAppAddr = locateObjectInMemory(ToolkitAppletInfoRef);

	nvmWriteU16Atomic(MEMtPN8(get_object_field_ref(MEMtPF8(toolkitAppAddr), EF_ToolkitApp_INDEX)),ref);

	return ToolkitAppletInfoRef;

}


/*
*@description: GetToolkitInfoObj,获取toolkitinfo对象
*
*@param      :BYTE beFreeIndex
*
*@return      :UWORD
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
UWORD GetToolkitInfoObj(BYTE beFreeIndex)
{
	jref appletRef;
	jref toolkitInfoRef;
	u32 toolkitInfoObj;
	PN8	appletObject;
	memref entry,theAppTable;


	if((beFreeIndex == G_ToolkitAppIndex)&&(G_ToolkitAppletInfoRef != REF_NULL))
	{
		return locateObjectInMemory(G_ToolkitAppletInfoRef);
	}

	theAppTable = getAppTable();
	entry = locateObjectInMemory(PN_R_U16(U32_A_U8(theAppTable , U8_M_U8(beFreeIndex , SIZE_REF))));
	//entry				    =	getEntryByAppId(beFreeIndex);

	if(entry == MEM_NULL)
	{
		return ADDR_IS_NULL;
	}
	appletRef 				= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));

	if(appletRef == REF_NULL)// isd uicc isdr ecasd   在addentry的时候就分配的appid ，此时能根据AppId找到entry，但是没有applet对象，profile下安装mnosd会调该函数
	{
		return ADDR_IS_NULL;
	}

	appletObject			= 	MEMtPN8(locateObjectInMemory(appletRef));
	toolkitInfoRef			=	PN_R_U16(get_object_field_ref(MEMtPF8(appletObject),toolkitInfo_INDEX));

	if(REF_NULL != toolkitInfoRef)
	{
		toolkitInfoObj = locateObjectInMemory(toolkitInfoRef);
	}
	else
	{
		return ADDR_IS_NULL;
	}

	return toolkitInfoObj;
}

/*
*@description: GetToolkitAppInfoArrayAddr,获取ToolkitAppInfo数组地址
*
*@param      :UWORD toolkitInfoObj
*
*@return      :UWORD
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
UWORD GetToolkitAppInfoArrayAddr(UWORD toolkitInfoObj)
{
	jref X_A    ToolkitAppInfoRef;
	u32  X_A    ToolkitAppInfoAddr;

	if(toolkitInfoObj == ADDR_IS_NULL)//toolkitInfoObj存在就一定会创建byte []   ToolkitAppInfo;//6F80
	{
		return ADDR_IS_NULL;
	}

	ToolkitAppInfoRef 			= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(toolkitInfoObj), EF_ToolkitApp_INDEX)));
	ToolkitAppInfoAddr			= 	locateObjectInMemory(ToolkitAppInfoRef);

	ToolkitAppInfoAddr          =   get_array_data(MEMtPF8(ToolkitAppInfoAddr));

	return ToolkitAppInfoAddr;
}


/*
*@description: GetFirstToolkitEventFileListObj,获取第一个ToolkitEvent文件列表对象
*
*@param      :UWORD toolkitInfoObj,BYTE mode
*
*@return      :UWORD
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
UWORD GetFirstToolkitEventFileListObj(UWORD toolkitInfoObj,BYTE mode)
{
	jref  X_A     ToolkitEventFileListRef;

	if(toolkitInfoObj == ADDR_IS_NULL)//toolkitInfoObj存在就一定会创建byte []   ToolkitAppInfo;//6F80
	{
		return ADDR_IS_NULL;
	}

	if(mode == MODE_EXTERNAL_FILE_LIST)
	{
		ToolkitEventFileListRef = nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(toolkitInfoObj), ExternalEventFileList_INDEX)));
	}
	else
	{
		ToolkitEventFileListRef = nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(toolkitInfoObj), RemoteEventFileList_INDEX)));
	}

	if(ToolkitEventFileListRef != REF_NULL)
	{
		return   locateObjectInMemory(ToolkitEventFileListRef);
	}

	return ADDR_IS_NULL;
}


/*
*@description: GetToolkitEventFileListArr,获取ToolkitEvent文件列表地址
*
*@param      :UWORD ToolkitEventFileListObj
*
*@return      :UWORD
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
UWORD GetToolkitEventFileListArr(UWORD ToolkitEventFileListObj)
{
	jref   X_A    FileListRef;
	u32    X_A    FileListArrAddr;

	if(ToolkitEventFileListObj == ADDR_IS_NULL)//ToolkitEventFileListObj存在就一定会创建byte []   ToolkitFileList;
	{
		return ADDR_IS_NULL;
	}

	FileListRef 			= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(ToolkitEventFileListObj), ToolkitFileList_INDEX)));
	FileListArrAddr			= 	locateObjectInMemory(FileListRef);

	FileListArrAddr          =   get_array_data(MEMtPF8(FileListArrAddr));

	return FileListArrAddr;
}


/*
*@description: GetNextToolkitEventFileListObj,获取下一个ToolkitEvent文件列表对象
*
*@param      :UWORD ToolkitEventFileListObj
*
*@return      :UWORD
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
UWORD GetNextToolkitEventFileListObj(UWORD ToolkitEventFileListObj)
{
	jref   X_A    FileListRef;
	u32    X_A    FileListArrAddr;

	if(ToolkitEventFileListObj == ADDR_IS_NULL)//toolkitInfoObj存在就一定会创建byte []   ToolkitAppInfo;//6F80
	{
		return ADDR_IS_NULL;
	}

	FileListRef 			= 	nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(ToolkitEventFileListObj), nextFileList_INDEX)));
	FileListArrAddr			= 	locateObjectInMemory(FileListRef);

	return FileListArrAddr;
}


/*
*@description: SetToolkitEventFileListObj,设置ToolkitEvent文件列表对象
*
*@param      :UWORD toolkitInfoObj,u16 ref,BYTE mode
*
*@return      :void
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
void SetToolkitEventFileListObj(UWORD toolkitInfoObj,u16 ref,BYTE mode)
{
	u16   X_A    ToolkitEventFileListRef;
	u32   X_A    addr,ToolkitEventFileListObj;

	if(mode == MODE_EXTERNAL_FILE_LIST)
	{
		addr = get_object_field_ref(toolkitInfoObj, ExternalEventFileList_INDEX);
	}
	else
	{
		addr = get_object_field_ref(toolkitInfoObj, RemoteEventFileList_INDEX);
	}

	ToolkitEventFileListRef = nvmReadU16(MEMtPN8(addr));

	ToolkitEventFileListObj = locateObjectInMemory(ToolkitEventFileListRef);

	while(ToolkitEventFileListObj != ADDR_IS_NULL)
	{
		addr = get_object_field_ref(ToolkitEventFileListObj, nextFileList_INDEX);
		ToolkitEventFileListObj = GetNextToolkitEventFileListObj(ToolkitEventFileListObj);
	}

	nvmWriteU16Atomic(MEMtPN8(addr),ref);
}
/*删除与fileList匹配的文件路径所在数组*/
/*
*@description: clearToolkitEventFileList,清除ToolkitEvent文件列表对象
*
*@param      :UWORD toolkitInfoObj,PN8 fileList,BYTE mode
*
*@return      :void
*
*@author     :create by liguiyu
*
*@hisoty     :V1.0:  create by liguiyu@20221204
*/
void clearToolkitEventFileList(UWORD toolkitInfoObj,PN8 fileList,BYTE mode)
{
	u16   X_A    NextFileListRef;
	u32   X_A    addr,ToolkitEventFileListObj,ToolkitEventFileListArr,nextAddr;
	u8    X_A    fileListArr[10];

	if(mode == MODE_EXTERNAL_FILE_LIST)
	{
		addr = get_object_field_ref(toolkitInfoObj, ExternalEventFileList_INDEX);
	}
	else
	{
		addr = get_object_field_ref(toolkitInfoObj, RemoteEventFileList_INDEX);
	}

	NextFileListRef = nvmReadU16(MEMtPN8(addr));

	ToolkitEventFileListObj = locateObjectInMemory(NextFileListRef);

	while(ToolkitEventFileListObj)
	{
		ToolkitEventFileListArr = GetToolkitEventFileListArr(ToolkitEventFileListObj);

		ecpMem_Copy(&fileListArr[0],ToolkitEventFileListArr,sizeof(fileListArr));

		nextAddr = get_object_field_ref(ToolkitEventFileListObj, nextFileList_INDEX);

		NextFileListRef 	= 	nvmReadU16(MEMtPN8(nextAddr));

		if(ecpMem_Compare(fileListArr,fileList,10)==EQUAL)
		{
			nvmWriteU16Atomic(MEMtPN8(addr), NextFileListRef);

			return;
		}

		addr =  nextAddr;

		ToolkitEventFileListObj =  locateObjectInMemory(NextFileListRef);

	}
}
#endif

