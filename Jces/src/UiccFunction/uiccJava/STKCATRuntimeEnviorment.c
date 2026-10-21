/************************************************************************************************************************************************************************************************
*Copyright(C) 2013- ,Eastcompeace.CO.,Ltd
* 
*FileName    :UICCProactiveHandler.c
*
*Author      :CAIBO
*
*Version     :V1.0
*
*Date        :2013-10-30

*Description :该文件主要用于实现CAT Runtime Enviorment,主要包括应用触发等
*
*History     :
	
author    time      version    description
ZZJ   2013/11/27   V1.0       创建该文件
************************************************************************************************************************************************************************************************/
#include    "fileAction.h"
#include    "gsmpublic.h"
#include    "hard_api.h"
//#include    "transaction.h"
#include    "ToolkitRFM.h"
#include    "fileSystem.h"
//#include    "JCREMain.h"
#include    "STKCATRuntimeEnviorment.h"
#include    "STKFileView.h"
#include    "uicc_interface.h"
#include    "ToolkitAPIPublic.h"
//#include    "main.h"
//#include    "ChannelManagement.h"
#include    "stkhead.h"

#if UICC__FUNCTION__SIMAPI
#include    "SimApi.h"
#endif

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
#include "SEEUICCInterface.h"
#endif

#if UICC__FUNCTION__STKAPI
/*
*@description：对指定 的 toolkit按照优先级排序，如果优先级一样，则按照安装顺序排序
*
*@function   : SortApplistEvent
*
*@param      :BYTE *buf
*@param      :BYTE num
*
*@return     :void
*
*@author     :xuyz
*
*@history     :@
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
void SortApplistEvent(BYTE *buff, BYTE num)
{
	BYTE idata i,j;
	BYTE idata tmp,MatchedRec,channelId;
	BYTE xdata eventlist[2],appbuf[20];//和getApplistByEvent传参数大小一样  两个应用都有外部更新事件时候  会冲掉tempBuf;
	WORD idata LastestInstallSeq,InstallSeq;

#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	UWORD xdata toolkitAppAddr;
	
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{		
		return;
	}
#endif

	for(i=1;  i<=num;  i++)
	{
		for(j=num; j>i;j--)
		{		  
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(buff[j-1])),buff[j-1],AppPriority_OFFSET,&eventlist[0],0x01);
			ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(buff[j-2])),buff[j-2],AppPriority_OFFSET,&eventlist[1],0x01);
#else
			ReadLFBufForJavaAccess(toolkitAppAddr,buff[j-1],AppPriority_OFFSET,&eventlist[0],0x01);
			ReadLFBufForJavaAccess(toolkitAppAddr,buff[j-2],AppPriority_OFFSET,&eventlist[1],0x01);
#endif

			if(eventlist[0] < eventlist[1])  // 01:优先级最高  FF：优先级最低
			{
				tmp = buff[j-1];

				buff[j-1] = buff[j-2] ;

				buff[j-2] = tmp;
			}
		}
	}
	
	//查找若连续几个应用 优先级一样，则按照安装顺序进行排序
	for(i=0; i< num; )
	{	   
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(buff[i])),buff[i],AppPriority_OFFSET,&eventlist[0],0x01);
#else
		ReadLFBufForJavaAccess(toolkitAppAddr,buff[i],AppPriority_OFFSET,&eventlist[0],0x01);
#endif
		for(j=(i+1),MatchedRec=0; j<num; j++)
		{			
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(buff[j])),buff[j],AppPriority_OFFSET,&eventlist[1],0x01);
#else
			ReadLFBufForJavaAccess(toolkitAppAddr,buff[j],AppPriority_OFFSET,&eventlist[1],0x01);
#endif
			if(eventlist[0] == eventlist[1])
			{
				MatchedRec ++;
			}
			else
			{
				break;
			}
		}

		if(MatchedRec >0)  //MatchedRec表示 i后面有几个app的优先级和它一样，如果有，则按照安装时间再进行排序
		{
			ecpMem_Copy(&appbuf[0],&buff[i],(MatchedRec+1));  //先将记录备份到tempBuf中，

			for(channelId = MatchedRec; channelId >0; channelId--)
			{
				for(j=0;   j<channelId;    j++)
				{					
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
					ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(appbuf[j])),appbuf[j],InstallSequence_OFFSET,(BYTE *)&InstallSeq,sizeof(WORD));

					ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(appbuf[j+1])),appbuf[j+1],InstallSequence_OFFSET,(BYTE *)&LastestInstallSeq,sizeof(WORD));
#else
					ReadLFBufForJavaAccess(toolkitAppAddr,appbuf[j],InstallSequence_OFFSET,(BYTE *)&InstallSeq,sizeof(WORD));

					ReadLFBufForJavaAccess(toolkitAppAddr,appbuf[j+1],InstallSequence_OFFSET,(BYTE *)&LastestInstallSeq,sizeof(WORD));
#endif
					if(InstallSeq <= LastestInstallSeq)
					{
						tmp = appbuf[j];

						appbuf[j] =appbuf[j+1] ;

						appbuf[j+1] = tmp;
					}	
				}
			}

			ecpMem_Copy(&buff[i],&appbuf[0],(MatchedRec+1));  //将信息还原，

			i += (MatchedRec+1);
		}
		else
		{
			i++;
		}
	}
}
/*@Function:    getApplistByEvent                                            
 * 
 *@Descrip:     通过事件确定所触发应用的AID
 *
 *@See:	        ts_143019v050600p.pdf  
 *                                                                                                                                     
 *@Param:       eventbit ：下行对应事件
 *				identifier	：项目标识符，menuselection(helprequest时有效)
 *				buff ：存储待出发应用对应的6F80记录号          
 *@Return:      待触发应用个数                                                                                                                                                                                                                                                                   
 * 
 *@Author:      LJ 2013.06.20
 *  
 *@History:     V0.0---Create by LJ 2013.06.20
 *              V1.1-- 增加对逻辑通道的处理，优化代码，xyz 2013.09.16   
 *              V1.2---增加对EVENT_EXTERNAL_FILE_UPDATE和EVENT_REMOTE_FILE_UPDATE的处理 by zzj@20131205
 *              V1.3---对于EVENT_PROACTIVE_HANDLER_AVAILABLE_bit事件，若当前应用注册了该事件，则获取记录号之后，直接清除该事件，参考UICC API测试脚本TS 102268 \Cre_Apt_Epha
 *              :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 */
BYTE getApplistByEvent(BYTE eventbit,BYTE identifier,BYTE buff[20])   //注意该函数内部会判断buff所指示的数组的大小
{
	BYTE idata i,j,tmp,num,channelId,entryApprecNum;
	BYTE xdata eventlist[EventList_Length];

#if UICC__FUNCTION__UICCAPI
	BYTE idata recNum, recLgth, blkNum_DF,blkNum_EF;
	UWORD  xdata toolkitPathInfoAddr;
#endif
	BYTE idata state;//LV结构AID
	UWORD xdata tookitMenuEntryAddr;
	BYTE idata  tookitMenuEntryRecNum,toolkitAppRecNum;
	UWORD xdata toolkitAppAddr;
	u16    X_A      bFreeEntryIndex;
	u8     X_A  tmpDf[0x0A];  // path 长度为A
	u8     X_A  tmpEf[0x0A];  // path 长度为A

#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
		return FALSE;
	}
	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);
#endif

	channelId = 0x00;
    
	entryApprecNum = FALSE;

	if((eventbit == EVENT_MENU_SELECTION_bit )||(eventbit == EVENT_MENU_SELECTION_HELP_REQUEST_bit ))
 	{
		tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);

		if(tookitMenuEntryAddr == FALSE)
		{
			return FALSE;
		}

		tookitMenuEntryRecNum = GetLFRecNumLgth(tookitMenuEntryAddr, FileNum);

		for(i=1;i<= tookitMenuEntryRecNum;i++)
		{
			ReadLFBuf(tookitMenuEntryAddr,i,MenuEntryStatus_OFFSET,&tempBuf[0],1);

			if(tempBuf[0] == 0xFF)
			{
				continue;
			}
		
			 ReadLFBuf(tookitMenuEntryAddr,i,ItemIdentifier_OFFSET,&tempBuf[0],1);

			 if(identifier == tempBuf[0])
			 {
			 	if(eventbit == EVENT_MENU_SELECTION_HELP_REQUEST_bit)			  //判断该MENU是否支持HelpSupported
				{
					ReadLFBuf(tookitMenuEntryAddr,i,HelpSupported_OFFSET,&tempBuf[0],1);

					if(tempBuf[0] == FALSE)
					{
						return FALSE;
					}
				}

  				ReadLFBuf(tookitMenuEntryAddr,i,AppRecNum_OFFSET,&entryApprecNum,1);

			 	break;
			 }
		}
		if(i > tookitMenuEntryRecNum)
		{
			return FALSE;
		}

		if(entryApprecNum != FALSE)
		{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			buff[0] = getIndexByID(entryApprecNum);//该方案，menu文件记录中存的是APPID
#else
			buff[0] = entryApprecNum;
#endif
		    return 1;
		}

		return 0;

 	}

	if((eventbit == EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit)||(eventbit == EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit))
	{ 
		ReadTFBuf(G_EnvelopHanderAddr,0x00,&tempBuf[0],ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength);

		num = FindTag(&tempBuf[0],0x38,ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength);  //找到tag TLV_CHANNELSTATUS

		channelId = tempBuf[num+1]&0x07;	    
	}
	else  if(eventbit == EVENT_TIMER_EXPIRATION_bit)
	{
		ReadTFBuf(G_EnvelopHanderAddr,0x00,&tempBuf[0],ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength);  

		num = FindTag(&tempBuf[0],0x24,ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength);  //找到tag Timer Identifier

		channelId = tempBuf[num+1];	  
	}
	else if(eventbit == EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit)
	{
	 	ReadTFBuf(G_EnvelopHanderAddr,0x00,&tempBuf[0],ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength);

		num = FindTag(&tempBuf[0],0x41,ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength);  //找到tag Service Record tag

		channelId = tempBuf[num+2]&0x07;
	}
	else //白盒测试
	{
	}

	num = 0;

	if((eventbit == EVENT_FORMATTED_SMS_PP_ENV_bit)||(eventbit == EVENT_FORMATTED_SMS_PP_UPD_bit)||(eventbit == EVENT_FORMATTED_SMS_CB_bit))
	{		
		 i = ToolkitfindSpecifiedTARValue(G_ToolkitTar);   //查找该tar值是否在指定的应用中存在,在install时就已经判断了，某个tar在所有的应用中是唯一的

		 if(i != FALSE)
		 {
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			 toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));
#endif
			 ReadLFBufForJavaAccess(toolkitAppAddr,i,AppletStatus_OFFSET,&state,sizeof(BYTE));
			
			if(state != APPLET_UNLOCKED_SELECTABLE)
			{
				return FALSE;
			}

			ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,eventlist,sizeof(eventlist));

			if(ToolkitGetEventBit(eventlist,eventbit) == TRUE)
			{
				buff[num++] = i;
			}
		 }

		 return num;
          		   
	} 
	else if(eventbit == EVENT_PROACTIVE_HANDLER_AVAILABLE_bit)
	{
		buff[num++] = identifier;
		
		return num;
	}	
#if UICC__FUNCTION__UICCAPI
	else if(eventbit == EVENT_REMOTE_FILE_UPDATE_bit)
	{
		buff[num++] = identifier ;  // 触发前已经用tempBuf缓存待触发的应用index

		return num;
	}
	else if(eventbit == EVENT_EXTERNAL_FILE_UPDATE_bit)
	{	
		ecpMem_Set(tmpDf, 0xFF, sizeof(tmpDf));
		ecpMem_Set(tmpEf, 0xFF, sizeof(tmpEf));

		ecpMem_Copy(&tmpDf[0],&tempBuf[7],tempBuf[5]-3);		 //查找其父目录是否有注册FILE_UPDATE事件
		ecpMem_Copy(&tmpEf[0],&tempBuf[7],tempBuf[5]-1);		 //查找ef是否有注册FILE_UPDATE事件
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		recNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

		for (i = APP_FIRST; i < recNum; U16_INC(i))//查找注册了事件EVENT_EXTERNAL_FILE_UPDATE_bit，并且设置的file—path与当前DF/EF相同的应用
		{
			toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

			if(toolkitAppAddr == ADDR_IS_NULL)//不是toolkit应用
			{
				continue;
			}

			ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,eventlist,sizeof(eventlist));

			if(ToolkitGetEventBit(eventlist,eventbit) == FALSE)//没有注册该事件
			{
				continue;
			}

			if((SearchFileList(&tmpDf[0],i, MODE_EXTERNAL_FILE_LIST) == FALSE)&&(SearchFileList(&tmpEf[0],i, MODE_EXTERNAL_FILE_LIST) == FALSE))
			{
				continue;
			}

			buff[num++] = i;
		}
#else
		toolkitPathInfoAddr =  SelectLFFile(DF_Toolkit_P, EF_ToolkitAppPathInfo);		
		
//		ecpMem_Set(&tempBuf[100],0xFF,20);										 //tempBuf[100]~tempBuf[109]保存filelist ，初始化为全0xFF
//
//		ecpMem_Move(&tempBuf[100],&tempBuf[7],tempBuf[5]-3);						 //查找其父目录是否有注册FILE_UPDATE事件
//
//		ecpMem_Move(&tempBuf[110],&tempBuf[7],tempBuf[5]-1);						 //查找当前文件是否有注册FILE_UPDATE事件

		if((SearchFileList(&tmpDf[0],&blkNum_DF) == FALSE)&&(SearchFileList(&tmpEf[0],&blkNum_EF) == FALSE))
		{
			return 0;
		}
	
		recNum = GetLFRecNumLgth(toolkitPathInfoAddr, FileNum);

		recLgth = GetLFRecNumLgth(toolkitPathInfoAddr, FileLgth)>>1;		  //记录前半段注册EVENT_EXTERNAL_FILE_UPDATE，后半段注册EVENT_REMOTE_FILE_UPDATE

		tmp = (eventbit == EVENT_EXTERNAL_FILE_UPDATE_bit)? 0: recLgth;		  
		
		for(i = 1;i<=recNum;i++)
		{
			
			ReadLFBuf(toolkitPathInfoAddr, i, tmp ,&tempBuf[128], recLgth);		  //记录内容为LV结构
			for(j = 1;j<=tempBuf[128];j++ )
			{
				if((tempBuf[128+j]==blkNum_DF) || (tempBuf[128+j]==blkNum_EF))									  //若找到对应的注册信息，则将对应记录号放到待触发App列表（EF_TookitAppPathInfo的记录号与6f80文件记录号一一对应）
				{
					ReadLFBuf(toolkitAppAddr,i,EventList_OFFSET,eventlist,sizeof(eventlist));

					if(ToolkitGetEventBit(eventlist,eventbit) == TRUE)
					{
						buff[num++] = i;

						break;
					}
				}
			}
		}
#endif
	}
#endif
	else
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
		tmp = TRUE;

		for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i)) //查找事件匹配记录
#else
		for(i=1;i<=toolkitAppRecNum;i++)	 //查找事件匹配记录
#endif
		{

#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

			if(toolkitAppAddr == ADDR_IS_NULL)
			{
				continue;
			}
#endif			
			/*tmp = */ReadLFBufForJavaAccess(toolkitAppAddr,i,AppletStatus_OFFSET,&state,sizeof(BYTE));

			if(/*(tmp == FALSE)||*/(state!= APPLET_UNLOCKED_SELECTABLE))
			{
				continue;
			}
	
			ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,eventlist,sizeof(eventlist));
			
			if(ToolkitGetEventBit(eventlist,eventbit) == TRUE)
			{
				if((eventbit == EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit)||(eventbit == EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit))
				{
					ReadLFBufForJavaAccess(toolkitAppAddr,i,AllocateChannelCount_OFFSET,&tmp,0x01);	  //获取当前channel设置
	
					if(ValueBit(tmp,channelId) == TRUE)
					{
						buff[num++] = i;
					}
				}
				else if(eventbit == EVENT_TIMER_EXPIRATION_bit)
				{
					ReadLFBufForJavaAccess(toolkitAppAddr,i,AllocateTimerCount_OFFSET,&tmp,0x01);	  //获取当前Timer Identifier设置
	
					if(ValueBit(tmp,(channelId-1)) == TRUE)
					{
						buff[num++] = i;
					}
				}
				else if(eventbit == EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit)
				{
					ReadLFBufForJavaAccess(toolkitAppAddr,i,CurrentService_OFFSET,&tmp,0x01);	  //获取当前service设置
	
					if(ValueBit(tmp,channelId) == TRUE)
					{
						buff[num++] = i;
					}
				}
				else
				{
// 					if(eventbit == EVENT_PROACTIVE_HANDLER_AVAILABLE_bit)
// 					{
// 						ToolkitClearEventBit(eventlist,eventbit);

// 					    WriteLFBuf(toolkitAppAddr,i,EventList_OFFSET,eventlist,sizeof(eventlist));
// 					}

					buff[num++] = i;
				}
			}
			
			if(num >= 20)  //20代表 分配给buff指针的数组的大小
		    {
		   		break;
		    }
			
		}
	}
	if(num == 0)
    {
       return 0;
    }
	
	SortApplistEvent(buff, num);

   //先按照优先级进行排序，将级别最高的放在前面，
   
//    for(i=1;  i<=num;  i++)
//    {
//    	  for(j=num; j>i;j--)
// 	  {
// 	  	    ReadLFBuf(toolkitAppAddr,buff[j-1],AppPriority_OFFSET,&eventlist[0],0x01);

// 			ReadLFBuf(toolkitAppAddr,buff[j-2],AppPriority_OFFSET,&eventlist[1],0x01);

// 			if(eventlist[0] < eventlist[1])  // 01:优先级最高  FF：优先级最低
// 			{
// 				tmp = buff[j-1];

// 				buff[j-1] = buff[j-2] ;

// 				buff[j-2] = tmp;
// 			}
// 	  }
//    }
//    //查找若连续几个应用 优先级一样，则按照安装顺序进行排序
//    for(i=0; i< num; )
//    {
//    		ReadLFBuf(toolkitAppAddr,buff[i],AppPriority_OFFSET,&eventlist[0],0x01);

//    		for(j=(i+1),MatchedRec=0; j<num; j++)
// 		{
// 			ReadLFBuf(toolkitAppAddr,buff[j],AppPriority_OFFSET,&eventlist[1],0x01);

// 			if(eventlist[0] == eventlist[1])
// 			{
// 				MatchedRec ++;
// 			}
// 			else
// 			{
// 				break;
// 			}
// 		}

// 		if(MatchedRec >0)  //MatchedRec表示 i后面有几个app的优先级和它一样，如果有，则按照安装时间再进行排序
// 		{
// 			ecpMem_Copy(&tempBuf[0],&buff[i],(MatchedRec+1));  //先将记录备份到tempBuf中，

// 			for(channelId = MatchedRec; channelId >0; channelId--)
// 			{
// 				for(j=0;   j<channelId;    j++)
// 				{
// 				 	ReadLFBuf(toolkitAppAddr,tempBuf[j],InstallSequence_OFFSET,(BYTE *)&InstallSeq,sizeof(WORD)); 

// 					ReadLFBuf(toolkitAppAddr,tempBuf[j+1],InstallSequence_OFFSET,(BYTE *)&LastestInstallSeq,sizeof(WORD)); 
// 					
// 					if(InstallSeq <= LastestInstallSeq)
// 					{
// 						tmp = tempBuf[j];

// 						tempBuf[j] =tempBuf[j+1] ;
// 	
// 						tempBuf[j+1] = tmp;
// 					}	
// 				}
// 			}

// 			ecpMem_Copy(&buff[i],&tempBuf[0],(MatchedRec+1));  //将信息还原，

// 			i += (MatchedRec+1);
// 		}
// 		else
// 		{
// 			i++;
// 		}
//    }

  if((eventbit == EVENT_FORMATTED_SMS_PP_ENV_bit)||
     (eventbit == EVENT_FORMATTED_SMS_PP_UPD_bit)||
	 (eventbit == EVENT_FORMATTED_SMS_CB_bit)||
	 (eventbit == EVENT_CALL_CONTROL_BY_SIM_bit)||
	 (eventbit == EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit)||
	 (eventbit == EVENT_TIMER_EXPIRATION_bit)||
	 (eventbit == EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit)||
	 (eventbit == EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit)||
	 (eventbit == EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit))		 	//某些事件只能触发一个应用
	 {
	 	return 0x01;
	 }
	 else
	 {
	 	return num;
	 }
}
/*
*@description:初始化 EnvelopReponse的hanler有效性
*
*@param      :event
*
*@return     :void
*
*@history    :
*/
void ToolkitInitEnvelopResponseHandler(BYTE eventbit) 
{
	 ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;

	  if((eventbit == EVENT_FORMATTED_SMS_PP_ENV_bit)||
	   (eventbit == EVENT_UNFORMATTED_SMS_PP_ENV_bit)||
	   (eventbit == EVENT_CALL_CONTROL_BY_SIM_bit)||
	   (eventbit == EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit)||
	   (eventbit == EVENT_UNRECOGNIZED_ENVELOPE_bit))
	   {
	   	  	ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = TRUE;
	   }
}
/*
*@function      :Toolkit_transLocalEventTypeToStandardEvent
*
*@description   :将event转换成标准event
*
*@param         :localEvent
*
*@return        :standardEvent
*
*@author        :xuyz
*
*@history       :
*/
jbyte Toolkit_transLocalEventTypeToStandardEvent(BYTE localEvent)
{
	jbyte standardEvent;

	standardEvent = localEvent;

	if(localEvent == EVENT_UNRECOGNIZED_ENVELOPE_bit)
	{
		standardEvent = EVENT_UNRECOGNIZED_ENVELOPE;
	}
	else if((localEvent >= EVENT_PROACTIVE_HANDLER_AVAILABLE_bit)&&(localEvent <= EVENT_MEMORY_FAILURE_bit ))
	{
		standardEvent += 83;   //123-129
	}
	else //白盒测试
	{
	}

    return standardEvent;
}
/*
*@function      :HanlderInitBeforeProcessToolkit
*
*@description   :在调用processToolkit之前，卡片需要做的一些初始化动作
*
*@param          :eventBit
*
*@return         :void
*
*@history        :V1.0
*/
void HanlderInitBeforeProcessToolkit(BYTE eventbit)
{
	ToolkitHandlerList[Index_EnvelopeHandler].HandlerAvailable = TRUE;
	ToolkitHandlerList[Index_ProactiveHandler].HandlerAvailable = FALSE;
	ToolkitHandlerList[Index_ProResHandler].HandlerAvailable = FALSE;

	if(G_TerminalProfile[0] != 0x00)	 //see ts102268 5.5.1.1.1.2:The ProactiveHandler shall not be available if the Terminal Profile command has not yet been processed by the CAT Runtime Environment
	{
		ToolkitHandlerList[Index_ProactiveHandler].HandlerAvailable = TRUE;
		ToolkitHandlerList[Index_ProResHandler].HandlerAvailable = TRUE;  //TS 102268 5.5.1.2.1.3 CRRC2:The ProactiveResponseHandler shall not be available if the ProactiveHandler is not available.
	}

    ToolkitHandlerList[Index_EnveResHandler].TLVListLength = 0x00;
	ToolkitHandlerList[Index_ProactiveHandler].TLVListLength = 0x00;
    ToolkitHandlerList[Index_ProResHandler].TLVListLength = 0x00;

 	ClearHandlerCurrentTLV(Index_EnvelopeHandler);
	ClearHandlerCurrentTLV(Index_EnveResHandler);
	ClearHandlerCurrentTLV(Index_ProactiveHandler);
	ClearHandlerCurrentTLV(Index_ProResHandler);

	if((eventbit == EVENT_FIRST_COMMAND_AFTER_ATR_bit)||(eventbit == EVENT_APPLICATION_DESELECT_bit))
	{
		/*The ProactiveHandler and the ProactiveResponseHandler shall not be available at the invocation of the
          processToolkit method of the toolkit applet on the EVENT_FIRST_COMMAND_AFTER_SELECT event.*/
		ToolkitHandlerList[Index_ProactiveHandler].HandlerAvailable = FALSE;
		ToolkitHandlerList[Index_ProResHandler].HandlerAvailable = FALSE;
	}
	if( (eventbit == EVENT_FIRST_COMMAND_AFTER_ATR_bit)||(eventbit == EVENT_PROFILE_DOWNLOAD_bit) ||(eventbit == EVENT_STATUS_COMMAND_bit)||(eventbit == EVENT_PROACTIVE_HANDLER_AVAILABLE_bit))
	{
		ToolkitHandlerList[Index_EnvelopeHandler].HandlerAvailable = FALSE;
	}
}
/*@Function:    SetToolkitContext                                            
 * 
 *@Descrip:     执行javatoolkit应用之前配置临时数据 
 *
 *@See:	        ts_143019v050600p.pdf  
 *                                                                                                                                     
 *@Param:       RecordNum :	待触发应用对应6F80文件记录号
 *              
 *              eventbit ：事件源在事件列表中bit标志
 *						                                                                                                  
 *@Return:      0x01:SIM API mode /0x02:UICC API mode/FALSE 配置失败                                                                                                                                                                                                                                                               
 * 
 *@Author:     
 *  
 *@History:     modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
 *                                           用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
 *              modify by lgy@20220708  #if UICC__STKAPI_TOOLKITINFO_IN_HEAP时 参数      RecordNum含义为AppEntry的下标
 */
BYTE SetToolkitContext(BYTE RecordNum,BYTE eventbit)
{
	BYTE idata trigerMode,appstatus;
	UWORD xdata toolkitAppAddr;

	jbyte  idata event = Toolkit_transLocalEventTypeToStandardEvent(eventbit);
	
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(RecordNum));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{
	   return FALSE;
	}
#endif
	ReadLFBufForJavaAccess(toolkitAppAddr,RecordNum,AppletStatus_OFFSET,&appstatus,1);

	if(appstatus != APPLET_UNLOCKED_SELECTABLE)
	{
		return FALSE;//app处于selectable状态时才允许触发 20140318
	}

	ReadLFBufForJavaAccess(toolkitAppAddr,RecordNum,TrigerMode_OFFSET,&trigerMode,1);

	//G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] = trigerMode;

	G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] = event;

	G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel] = RecordNum;

	HanlderInitBeforeProcessToolkit(eventbit);

#if UICC__FUNCTION__SIMAPI
	if(trigerMode == SIMAPI_Mode)
	{
		Toolkit_InitSIMView(G_CurSelectedChannel,0xFF); 	//SIM API 模式下，仅可访问MF下的DF 和EF
	}
    #endif
	return trigerMode;

}
/*
*@funtion    :publicToolkitEventTrigerApplet
*
*@description: 该函数主要用于对于某个event而言，来触发应用. 该函数会进入JCVM，从JCVM出来时提交CACHE (CallToolkitApplet中完成)
*
*@param     : eventbit   identifier
*
*@return    :TRUE:有应用触发/FALSE: 无应用触发
*
*@history   :V1.0:create by xuyz@20140124
*/
BYTE publicToolkitEventTrigerApplet(BYTE eventbit,BYTE identifier)
{
	BYTE idata i,num;
	BYTE xdata Applist[20];
	BYTE idata trigerMode;
	BYTE idata spi[2];
	BYTE idata backCurrentApprecNum;
	SBYTE idata backCurrentEvent;

    G_TriggerAppNumber = FALSE;   //清空该事件所触发的应用个数
    
	G_ISOBusyExceptionFlag = FALSE;
	
	backCurrentApprecNum = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];
	
	backCurrentEvent = G_ToolkitContext.CurrentEvent[G_CurSelectedChannel];
	
	num = getApplistByEvent(eventbit,identifier,Applist); //该函数还未进入JAVA层

	if(num == 0)                                         //add by zzj@20131205
	{
		return FALSE;
	}
    
	G_TriggerAppNumber = num;  //设置该应用触发的实际应用个数

	ToolkitInitEnvelopResponseHandler(eventbit);

	trigerMode = FALSE;

	for(i = 0;i < num;i++)
	{	
		trigerMode =  SetToolkitContext(Applist[i],eventbit);
		 
		if(trigerMode == FALSE)
		{
			continue;
		}

		CallToolkitApplet(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel],G_ToolkitContext.CurrentEvent[G_CurSelectedChannel],trigerMode); //在该函数中进入JCVM，该函数返回时CACHE已经提交
	 	
		ProcessNoShariableFileForToolkit(trigerMode);
    }
		
	if(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_FORMATTED_SMS_PP_ENV)
	{
		if(G_PostTag == FALSE)  
		{
			G_PostTag = TRUE;  
			
			ReadTFBuf(G_EnvelopHanderAddr,G_formatedSPIOffset,&spi[0], 0x02);

			if(((spi[1] & 0x20) == 0x20)&&(G_STKBusy != FALSE)) //如果当前要发送submit Por且处于STK 中，则不发送
			{
		
			}
			else
			{
				Toolkit_ResponsePack(POR_OK,PORMODE_AUTO_EVERESPONSE,ToolkitHandlerList[Index_EnveResHandler].TLVListLength, 0x00);//modify by jiangqingbin-20220303, 增加一个传入参数，表示http短信触发结果，不再使用原来的复用SW1，SW2的方式 	
			}   //31.048  Test Area Reference: ANA_INS_INMS.  TestCase 17

		}
	}    

	//modify by zhxx@20230509,电信外场反馈6F00问题，http方式，执行storedata指令，会进入到应用里面，该应用注册了123事件，除了stk之后，发送91xx（provide local information），
	//之后清掉G_ToolkitContext.CurrentApprecNum然后执行第二个storedata指令，通过接口读取文件时，由于G_ToolkitContext.CurrentApprecNum是0，抛了6F00
	//修改方案，将G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]先备份起来，事件触发结束进行还原

	G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel] = backCurrentApprecNum; //toolkit应用执行完成，清除该变量，已区分非共享文件被toolkit操作还是普通的Java Applet	，此处清除G_ToolkitContext.CurrentApprecNum目的是支持普通Java Applet处理
    
	G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] = backCurrentEvent;
    
	//G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] = 0x00;	

	G_TriggerAppNumber = FALSE; 

	return TRUE;
}
/*
*@description:判断当前是否存在应用注册了event ProactiveHandlerAvailable
*
*@function    :isProactiveHandlerAvailableExist
*
*@param        :void
*
*@return       :TRUE/FALSE
*
*@history       :1.0:create by xuyz
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE isProactiveHandlerAvailableExist(BYTE buf[20])
{
	BYTE idata i, tmp,state;
	BYTE idata num,recNum;
	BYTE xdata eventlist[EventList_Length];
	
	UWORD xdata toolkitAppAddr;
	
	num = 0;

#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP

	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);
	
	if(toolkitAppAddr == FALSE)
	{
		return FALSE;
	}

	recNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(i=1;i<= recNum; i++)	 //查找事件匹配记录
#else
	recNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for (i = APP_FIRST; i < recNum; U16_INC(i))
#endif
	{
#if    UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr  = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}
#endif
		ReadLFBufForJavaAccess(toolkitAppAddr,i,AppletStatus_OFFSET,&state,sizeof(BYTE));

		if(state!= APPLET_UNLOCKED_SELECTABLE)
		{
			continue;
		}

		ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,eventlist,sizeof(eventlist));

		if(ToolkitGetEventBit(eventlist,EVENT_PROACTIVE_HANDLER_AVAILABLE_bit))
		{
			if(buf != NULL)
			{
				buf[num] = i;
			}
			
			num ++;	

			if(num >= 20)
			{
				break;     //最多支持20 个应用同时注册了procative handler事件 
			}
		}
	}
	
	return num;
}
/*@Function:    JavaToolkitProcess                                            
 * 
 *@Descrip:     执行javaToolkit应用入口函数
 *
 *@See:	        ts_102241  
 *                                                                                                                                     
 *@Param:       eventbit ：事件源在事件列表中bit标志
 *				
 *				index ：被触发应用在EF_EF_MenuEntryIndex中对应的记录号，只有menuselection事件时该参数有效                                                                                                     
 *@Return:      NULL                                                                                                                                                                                                                                                                
 * 
 *@Author:        
 *  
 *@History:   V1.1:注意：对于EVENT_PROACTIVE_HANDLER_AVAILABLE_bit事件，若在APP1 中触发了app2，触发app2时，app2注册了VENT_PROACTIVE_HANDLER_AVAILABLE事件，那么是在app2触发结束后触发VENT_PROACTIVE_HANDLER_AVAILABLE还是在app1上一个事件触发结束后触发VENT_PROACTIVE_HANDLER_AVAILABLE呢   
              V1.2：If the CAT session ends prior to an applet triggering, the applet will be triggered at the next CAT session. 这句话没了解什么意思？
 *                        
 */
void JavaToolkitProcess(BYTE eventbit,BYTE identifier)
{	
	G_PostTag = FALSE;  //初始化当前事件的post信息

	G_ToolkitContext.CurrentEvent[G_CurSelectedChannel]  = EVENT_InitValue;  //初始化该全局变量，在STK中触发应用时，由于并没有清除该值，如果新的触发没有产生，则本函数内的后续操作还是会使用原值进行判断

	publicToolkitEventTrigerApplet(eventbit,identifier);
	
	if(G_ISOBusyExceptionFlag == TRUE)
	{
		return;
	}
	
	/*CRRN1: The applet is triggered by the EVENT_PROACTIVE_HANDLER_AVAILABLE once it has
	  registered to this event and once the ProactiveHandler is available and all the Toolkit Applets registered to the
	  previous event have been triggered and have returned from the processToolkit() invocation
	*/
		//CMCC API 中的APP A，在menuselction中触发EVENT_FORMATTED_SMS_PP_ENV，没有退出menuselction时获取了proactivehandler，抛异常，重新设置了事件EVENT_PROACTIVE_HANDLER_AVAILABLE，要求在获取menuslection的14之后触发EVENT_PROACTIVE_HANDLER_AVAILABLE
	
	JavaToolkitProcessProactiveHandlerEvent();
}


/*
*function     :ToolkitSaveAPPContext
*
*@description:在app1中触发app2时，缓存app1的现场
*
*@param     :void
*
*@return    :void
*
*@history   :V1.0 :create by zzj@20131128
 *          :v1.1:modify by zhxx@20230104,CMCC测试：39.Buglist测试/SIM快捷实网测试Status重入问题
*/
void ToolkitSaveAPPContext(void)
{
	UWORD xdata fileAddr;
	WORD idata i;
	
	#define tmp (tempBuf+180) //为了修改IF_ExternalFileUpdate  这个函数先判断有没有支持的事件 移动函数位置 这里使用tempBuf的位置修改
	
	if(G_STKBusy==FALSE)       //在STK里面触发应用才需要保存和恢复现场，by xwh 20140320
	{
		return;
	}

	//fileAddr = SelectTFFile(DF_Toolkit,EF_ToolkitAppContext);
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitAppContext);
	
	if(fileAddr == FALSE)
	{
		 return ;
	}

	ReadTFBuf(fileAddr, 0x00,&tmp[0] ,sizeof(BYTE));

	if(tmp[0] == 0x81)
	{
		return;
	}

	tmp[0] = 0x81;

	tmp[1] = G_EnvelopeTag;

	 i = 0x02;

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_JCREMainProcessMode,sizeof(G_JCREMainProcessMode));

	 i += sizeof(G_JCREMainProcessMode);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_TriggerAppNumber,sizeof(G_TriggerAppNumber));

	 i += sizeof(G_TriggerAppNumber);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_ISOBusyExceptionFlag,sizeof(G_ISOBusyExceptionFlag));

	 i += sizeof(G_ISOBusyExceptionFlag);

#if UICC__FUNCTION__SIMAPI
	 ecpMem_Copy(&tmp[i], (BYTE *)&G_SIMCurrentDfName[0],sizeof(G_SIMCurrentDfName[0]));	//因为C2只能在通道0过来，所以这里简单备份通道0的数据即可，供SIMView使用

	 i += sizeof(G_SIMCurrentDfName[0]);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_SIMCurrentEfName[0],sizeof(G_SIMCurrentEfName[0]));

	 i += sizeof(G_SIMCurrentEfName[0]);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_SIMCurrentDfAddr[0],sizeof(G_SIMCurrentDfAddr[0]));

	 i += sizeof(G_SIMCurrentDfAddr[0]);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_SIMCurrentEfAddr[0],sizeof(G_SIMCurrentEfAddr[0]));

	 i += sizeof(G_SIMCurrentEfAddr[0]);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_SIMCurrentRecNum[0],sizeof(G_SIMCurrentRecNum[0]));

	 i += sizeof(G_SIMCurrentRecNum[0]);
	 
	 ecpMem_Copy(&tmp[i], (BYTE *)&G_SIMFileSystemMode[0],sizeof(G_SIMFileSystemMode[0]));

	 i += sizeof(G_SIMFileSystemMode[0]);
    #endif

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_ToolkitContext,sizeof(G_ToolkitContext));

	 i += sizeof(G_ToolkitContext);
	 
//	 ecpMem_Copy(&tmp[i], (BYTE *)&ToolkitHandlerList[Index_ProactiveHandler].TLVListLength,sizeof(WORD));

//	 i += sizeof(WORD);  //中国移动NFC2.0项目20160415/ GP2.2.1_UICC_ComplianceTests(移动版)-20141112/ReEntry_test/SIMReEntry_09_01_01 脚本要求两次调用send函数，且第一次send调用时触发了新的应用，因此需要将该现场保存下来

	 ecpMem_Copy(&tmp[i], (BYTE *)&ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer,sizeof(ToolkitHandlerList));

	 i += sizeof(ToolkitHandlerList);//by yanjr20220811 原有的代码没有备份EnveResHandler的状态，下行短信触发应用sim快捷3.5，期望91xx，在fech之后，来了status，又触发了应用，此时将EnveResHandler的状态置为false，status结束之后没有恢复，导致TerminalResponse时，EnveResHandler不可用，期望的91xx没有发出来

//	 ecpMem_Copy(&tmp[i], (BYTE *)&ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength,sizeof(WORD));

//	 i += sizeof(WORD);

//	 ecpMem_Copy(&tmp[i], (BYTE *)&ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset,sizeof(WORD));

//	 i += sizeof(WORD);

	 ecpMem_Copy(&tmp[i], (BYTE *)&G_PostTag,sizeof(BYTE));

	 i += sizeof(BYTE);

	 //WriteTFBuf(fileAddr,0x01,&tmp[1],(i-1));
	 WriteTFBuf(fileAddr,0x00,&tmp[0],i);

	 //G_EnvelopHanderAddr  = SelectTFFile(DF_Toolkit,EF_EnvelopSecondHandlerBuf);
	 G_EnvelopHanderAddr  = SelectPrivateTFFile(DF_Toolkit,EF_EnvelopSecondHandlerBuf);

	 ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer = (BYTE *)(G_EnvelopHanderAddr + sizeof(TranFileHead));		
	
	 ToolkitHandlerList[Index_EnvelopeHandler].HandlerBufferSize = GetTFSize(G_EnvelopHanderAddr);

	 //SetTFConst(fileAddr,0x00,0x81,sizeof(BYTE));

}
/*
*function     :ToolkitRestoreAPPContext
*
*@description:从app2中返回时，恢复app1的现场
*
*@param     :void
*
*@return    :void
*
*@history   :V1.0 :create by zzj@20131128
*           :v1.1:modify by zhxx@20230104,CMCC测试：39.Buglist测试/SIM快捷实网测试Status重入问题
*/

void ToolkitRestoreAPPContext(void)
{
	UWORD xdata fileAddr;
	WORD idata i;
	
	if(G_STKBusy==FALSE)         //在STK里面触发应用才需要保存和恢复现场，by xwh 20140320
	{
		return;
	}

	//fileAddr = SelectTFFile(DF_Toolkit,EF_ToolkitAppContext);
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitAppContext);
	
	if(fileAddr == FALSE)
	{
		 return ;
	}

	ReadTFBuf(fileAddr, 0x00,&tempBuf[0] ,sizeof(BYTE));

	if(tempBuf[0] != 0x81)
	{
		return;
	}

	ReadTFBuf(fileAddr, 0x01,&G_EnvelopeTag,sizeof(BYTE));

	i = 0x02;

	 ReadTFBuf(fileAddr,i, (BYTE *)&G_JCREMainProcessMode,sizeof(G_JCREMainProcessMode));

	 i += sizeof(G_JCREMainProcessMode);

     ReadTFBuf(fileAddr,i, (BYTE *)&G_TriggerAppNumber,sizeof(G_TriggerAppNumber));

	 i += sizeof(G_TriggerAppNumber);
	
	 ReadTFBuf(fileAddr,i, (BYTE *)&G_ISOBusyExceptionFlag,sizeof(G_ISOBusyExceptionFlag));

	 i += sizeof(G_ISOBusyExceptionFlag);
	
#if UICC__FUNCTION__SIMAPI
	ReadTFBuf(fileAddr,i, (BYTE *)&G_SIMCurrentDfName[0],sizeof(G_SIMCurrentDfName[0]));

	 i += sizeof(G_SIMCurrentDfName[0]);

	ReadTFBuf(fileAddr,i, (BYTE *)&G_SIMCurrentEfName[0],sizeof(G_SIMCurrentEfName[0]));

	 i += sizeof(G_SIMCurrentEfName[0]);

	 ReadTFBuf(fileAddr,i, (BYTE *)&G_SIMCurrentDfAddr[0],sizeof(G_SIMCurrentDfAddr[0]));

	 i += sizeof(G_SIMCurrentDfAddr[0]);

	  ReadTFBuf(fileAddr,i, (BYTE *)&G_SIMCurrentEfAddr[0],sizeof(G_SIMCurrentEfAddr[0]));

	 i += sizeof(G_SIMCurrentEfAddr[0]);

	 ReadTFBuf(fileAddr,i, (BYTE *)&G_SIMCurrentRecNum[0],sizeof(G_SIMCurrentRecNum[0]));

	 i += sizeof(G_SIMCurrentRecNum[0]);
	 
	 ReadTFBuf(fileAddr,i, (BYTE *)&G_SIMFileSystemMode[0],sizeof(G_SIMFileSystemMode[0]));

	 i += sizeof(G_SIMFileSystemMode[0]);
    #endif

	 ReadTFBuf(fileAddr,i, (BYTE *)&G_ToolkitContext,sizeof(G_ToolkitContext));

	 i += sizeof(G_ToolkitContext);
	 
//	  ReadTFBuf(fileAddr,i, (BYTE *)&ToolkitHandlerList[Index_ProactiveHandler].TLVListLength,sizeof(WORD));

//	 i += sizeof(WORD);  //中国移动NFC2.0项目20160415/ GP2.2.1_UICC_ComplianceTests(移动版)-20141112/ReEntry_test/SIMReEntry_09_01_01 脚本要求两次调用send函数，且第一次send调用时触发了新的应用，因此需要将该现场保存下来

	 ReadTFBuf(fileAddr,i, (BYTE *)&ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer,sizeof(ToolkitHandlerList));

	 i += sizeof(ToolkitHandlerList);

//	  ReadTFBuf(fileAddr,i, (BYTE *)&ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVLength,sizeof(WORD));

//	 i += sizeof(WORD);

//	 ReadTFBuf(fileAddr,i, (BYTE *)&ToolkitHandlerList[Index_EnvelopeHandler].CurrentTLVValueOffset,sizeof(WORD));

//	 i += sizeof(WORD);

	 ReadTFBuf(fileAddr,i, (BYTE *)&G_PostTag,sizeof(BYTE));

	  i += sizeof(BYTE);

	 //G_EnvelopHanderAddr  = SelectTFFile(DF_Toolkit,EF_EnvelopeHandlerBuf);
	  G_EnvelopHanderAddr  = SelectPrivateTFFile(DF_Toolkit,EF_EnvelopeHandlerBuf);

	 ToolkitHandlerList[Index_EnvelopeHandler].HandlerBuffer = (BYTE *)(G_EnvelopHanderAddr + sizeof(TranFileHead));		
	
	 ToolkitHandlerList[Index_EnvelopeHandler].HandlerBufferSize = GetTFSize(G_EnvelopHanderAddr);

	 SetTFConst(fileAddr,0x00,0x00,sizeof(BYTE));
}

/*
*@description: 查找比当前应用优先级一样，但是安装顺序前一点的应用，如果没有则返回本次优先级下最新安装的应用
*
*@function    :JavaTookitProcessSamePriority
*
*@param       :BYTE * buff
*@param       :BYTE num
*@param       :BYTE curAppId,6F80的记录号
*
*@return      :return resultAppId: 当前待触发的proactiveHandler 应用
*
*@author      :xuyz
*
*@history    :V1.0 :create @20180910
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE JavaTookitProcessSamePriority(BYTE * buff, BYTE num, BYTE curAppId)
{
	BYTE idata i;
	BYTE idata curPri, tmpPri;
	WORD idata tmpInstallSeq,InstallSeq;
	BYTE idata firstAppId;
	
	UWORD xdata toolkitAppAddr;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(curAppId));
#else
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{		
		return FALSE;
	}
#endif
	firstAppId = FALSE;

	ReadLFBufForJavaAccess(toolkitAppAddr,curAppId,AppPriority_OFFSET, &curPri ,0x01);  //读取当前应用的优先级
	
	ReadLFBufForJavaAccess(toolkitAppAddr,curAppId,InstallSequence_OFFSET,(BYTE *)&InstallSeq,sizeof(WORD));   //读取当前应用的安装顺序
	
	for(i=0; i<num ; i++)
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(buff[i]));
#endif
		ReadLFBufForJavaAccess(toolkitAppAddr,buff[i],AppPriority_OFFSET, &tmpPri ,0x01);  //读取应用的优先级
	
		ReadLFBufForJavaAccess(toolkitAppAddr,buff[i],InstallSequence_OFFSET,(BYTE *)&tmpInstallSeq,sizeof(WORD));   //读取应用的安装顺序
		
		if(tmpPri == curPri)   //优先级相同时，检查安装顺序
		{
			if(firstAppId == FALSE)
			{
				firstAppId =  buff[i];  //登记当前优先级最高的那个应用，如果找不到比当前应用安装顺序旧的，则返回最新的
			}
			
			if(tmpInstallSeq <= InstallSeq)  //比当前应用早安装的
			{
				return buff[i];
			}
			
		}
	}
	
	return firstAppId;

}
/*
*@description: 找到当前待触发的EVENT_PROACTIVE_HANDLER_AVAILABLE的应用
*
*@function   :JavaToolkitFindProactiveHandlerApp
*
*@param      :BYTE currentAppId   ,6F80的记录号
*
*@return     :BYTE newAppId  ，6F80的记录号
*
*@author     :xuyz
*
*@history     :V1.1: v1.0 create @20180910
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE JavaToolkitFindProactiveHandlerApp(BYTE currentAppId)
{
	BYTE idata num;
	WORD idata resultApp;
	BYTE xdata Applist[20];
	BYTE xdata eventlist[EventList_Length];
	UWORD xdata toolkitAppAddr;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
	{		
		return FALSE;
	}
#endif
	num = isProactiveHandlerAvailableExist(Applist);   //登记 当前 注册了 proactive handler事件的应用, 最多支持20个
	
	if(num == 0)
	{
		return FALSE;
	}
	
	SortApplistEvent(Applist, num);  //先按照优先级从高到低进行排序，且相同优先级内部按照安装顺序从最新到最旧进行排序
	
	if(currentAppId == FALSE)  //第一次触发proactive handler应用
	{
		resultApp = Applist[0];  //返回优先级最高的 且最新安装的那一个应用
	}
	else
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(currentAppId)),currentAppId,AppPriority_OFFSET,&eventlist[0],0x01);  //获取当前应用的优先级

		ReadLFBufForJavaAccess(GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(Applist[0])),Applist[0],AppPriority_OFFSET,&eventlist[1],0x01);    //获取本次注册了procative事件的应用的最高优先级
#else
		ReadLFBufForJavaAccess(toolkitAppAddr,currentAppId,AppPriority_OFFSET,&eventlist[0],0x01);  //获取当前应用的优先级
		
		ReadLFBufForJavaAccess(toolkitAppAddr,Applist[0],AppPriority_OFFSET,&eventlist[1],0x01);    //获取本次注册了procative事件的应用的最高优先级
#endif
		if(eventlist[0] == eventlist[1])  //优先级相同，则先获取比它安装早的应用，然后再获取本次优先级下最新的应用
		{
			resultApp = JavaTookitProcessSamePriority(Applist, num, currentAppId);
			
			if(resultApp == FALSE)//if(resultApp == ILLEGAL_APPID)
			{
				return FALSE;
			}
		}
		else
		{
			resultApp = Applist[0];  
		}
	}
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr  =  GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(resultApp));
#endif
	ReadLFBufForJavaAccess(toolkitAppAddr,resultApp,EventList_OFFSET,eventlist,sizeof(eventlist));
   
	ToolkitClearEventBit(eventlist,EVENT_PROACTIVE_HANDLER_AVAILABLE_bit);
   
	WriteLFBufForJavaAccess(toolkitAppAddr,resultApp,EventList_OFFSET,eventlist,sizeof(eventlist));
		
	return resultApp;  
	
	
}
/*
*@description:  针对123 EVENT_PROACTIVE_HANDLER_AVAILABLE  事件进行处理
*
*@function   :JavaToolkitProcessProactiveHandlerEvent
*
*@param      : void
*
*@return     : void
*
*@author     :xuyz
*
*@history    :V1.0 :create @20180910
*/
void JavaToolkitProcessProactiveHandlerEvent(void)
{
	BYTE idata currentAppRecNo,eventbit;

	currentAppRecNo = FALSE;
	
	
	while(1)
	{
		if((G_STKBusy == FALSE)&&(STKPerform[0] == FALSE)&&(G_TerminalProfile[0] != 0x00))   //如果当前在主动式命令中，则 Index_ProactiveHandler依旧无效
		{	
			//只有当EVENT_PROACTIVE_HANDLER_AVAILABLE有效时，才触发事件EVENT_PROACTIVE_HANDLER_AVAILABLE_bit，但是此处使用判断G_TerminalProfile[0] != 0x00，因为若前面没有事件被触发时，ToolkitHandlerList[Index_ProactiveHandler].HandlerAvailable不会被设置成TRUE			
			if(isProactiveHandlerAvailableExist(NULL) == 0)                      
			{
					break;  //CMCC API cmccconstants 6.6.2.5 在触发当前的proactiveHandlerAvaiable事件发送主动式命令时，又注册了EVENT_PROACTIVE_HANDLER_AVAILABLE_bit，等待退出上一个主动式命令后重新触发by xuyz
			}
			currentAppRecNo = JavaToolkitFindProactiveHandlerApp(currentAppRecNo);
			
			if(currentAppRecNo == FALSE)
			{
				break;
			}

			 eventbit = EVENT_PROACTIVE_HANDLER_AVAILABLE_bit;  //测试用例cre_apt_epha ,在应用1的menuSelection中触发后，12数据还未退出，发送了应用2的menuselection要求触发应用，然后 卡片复位，要求在下一次 会话时重新触发

			 publicToolkitEventTrigerApplet(eventbit,currentAppRecNo);  //If the CAT session ends prior to an applet triggering, the applet will be triggered at the next CAT session
		}
		else
		{
			break;
		}
	}
}

#endif
