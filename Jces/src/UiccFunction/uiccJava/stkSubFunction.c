/*

 * Copyright (c) 2002,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EMOSYN  ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V4.03

 * PHASE 2  RELEASE:2003-2-25

 * AUTHER:	蔡勃

 * HISTORY: V1.0 创建源文件

			V1.1 修改SendPractiveCommand函数， 增加ReceiveFirstByte函数  @20130922 wlj

			V1.2  修改函数SendProactiveCommand，当收到14响应时，将STKPerform设置为false @20140623 by xuyz

			v1.3  增加函数GetBufBit、SetBufBit、ClearBufBit，主要用于处理Tookit事件使用@20151116 by xuyz
 */ 

#include <uicc_error.h>
#include  "typedef.h"
#include  "uicc_main.h"
//#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "arithmatic.h"
#include  "ceph.h"
#include  "uicc_interface.h"
#include  "T0ApduImpl.h"
//#include  "ChannelManagement.h"
//#include  "GPPublic.h"
//#include "JCREMain.h"
//#include "Exception.h"
#include "Interpreter.h"
#include "interface.h"
//#include "transaction.h"
//#include "ToolkitAPIPublic.h"
#include "BootLifeCycle.h"
#include "Anomaly.h"
#include "platform_api.h"
#include "SEEUICCInterface.h"
/*
 *@description:无符号比较数组，返回数据比较结果	  , 
 *
 *@function   :CompareUnsignedDataBuf
 *@param      :BYTE *CMP1.
 *@param      :BYTE *CMP2
 *@param      :BYTE len
 *
 *@return     >0 <0 = 0
 *
 *@autho
 */
signed char memcmp_p(BYTE * buf1,BYTE *buf2,BYTE lgth)
{
    BYTE i;
    
    for(i=0;i<lgth;i++)
    {
        if(buf1[i]>buf2[i])
        {
            return 1;
        }
        
        if(buf1[i]<buf2[i])
        {
            return -1;
        }
    }
    return EQUAL;

}
//BYTE IsUiccApp(BYTE contextId)
//{
//	BYTE idata tempAppId;

//	if(BootLifeCycle_GetLifeCycle() < LIFE_CYCLE_CREATE)
//	{
//			return FALSE;
//	}

//	tempAppId = GetUICCAppId();

//	//当前通道的应用是UICC应用
//	if((tempAppId != 0xFF)&&(contextId == tempAppId))
//	{
//		return TRUE;
//	}

//	return FALSE;

//}
/*@function:    SendProactiveCommand                                            
 * 
 *@descrip:     Send the data of proactive command and to pasre the terminal response for returnnig the general result
 *
 *@see:	        TS_11.14           
 *                                                                                                                                     
 *@param:	    void
 *				                                                                                                         
 *@return:      TRUE/FALSE                                                                                                                                                                                                                                                                    
 * 
 *@author:      xxx
 *  
 *@history:     V1.1---因为 12 14 必须在0通道上才能执行，因此暂时认为0通道上必须以UICC为默认应用@by xuyz
 *              V1.2---modify by xwh 20170612,在STK  中，发生了热复位，此时需要跳转到 JAVA  主流程执行
 *              V1.3---修改GP 非接测试问题(在STK 中收到非接指令，用例雅斯拓非接6.1.6.4，期望6A82，实际6E00), by xwh 20210726
 *              V1.4---修改V1.3的问题，在STK 中收到非接指令，不跳回jcvm重新派发，通过在field off中调用SwpDeselect函数来实现,by xwh 20211116
 */
static code INSLIST CephInsList[]=
{ 
//SIM and USIM command
	{0xA4,SelectCommand},
	{0xD6,UpdateBinary},
	{0xB2,ReadRecord},
	{0xDC,UpdateRecord},
  	{0xB0,ReadBinary},
  	{0x04,FileStatusHandler},
  	{0x44,FileStatusHandler},
  	{0x20,VerifyChangeUnblockChv},
  	{0x24,VerifyChangeUnblockChv},
  	{0x26,ChvHandle},
  	{0x28,ChvHandle},
  	{0x2C,VerifyChangeUnblockChv},	
  	{0xF2,STKStatusCommand},
  	{0xC0,GetResponseCommand},
   	{0xC2,EnvelopeCommandSTK},
  	{0x12,FetchCommand},
   	{0xA2,SeekCommand},
  	{0x32,IncreaseCommand},
	{0x10,STKTerminalProfile},
#if UICC__AUTH__5G__SUSPEND
	{0x76,SuspendUicc},
#endif
	
#if UICC__AUTH__5G__SUPI
	{0x78,GetIdentity},
#endif

	{EOL,IF_ReceiveAPDU},
};
/*
*@descritpion: 发送主动式命令
*
*@function   : SendProactiveCommand
*
*@param      : void
*
*@return      :接收到14 指令该函数才会退出
*
*@author      :lx
*
*@history     :V1.0 ：create @
*             :V1.1 modify by chentao@20230109   华勤卡在测试时，当激活或者失效一个profile时，卡片发送了refresh指令后，但在terminal response返回前，有其他指令，如status指令，会造成前面的激活或失效profile失败，并在发送BF2D时，返6985。
*/
BYTE SendProactiveCommand(void)
{
	BYTE idata num;
	BYTE idata abortSTKFlag;
//	PF8	 D_A	mainMethodAddr;
	BYTE idata     channelContext,tempAppId;
	BYTE idata    backChannelId;
	
	u8      X_A     backChannelProfileId,profileIndex;  // 重入情况下需要给G_CurExecutingProfileIndex重新赋值

	if(G_CurSelectedChannel == ILLEGAL_CHANNEL)
	{
		ThrowErrorBackToMain(Priv_EXCEPTION);  //为避免出现异常时造成	ChannelManagement[G_CurSelectedChannel]数组异常，在这里抛出异常
	}

	STKPerform[0] = TRUE ;
	
	STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 

	//G_STKBusy = (((WORD)(G_CurSelectedChannel+1)) << 8 )|(U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF)); //为防止STK在通道0时，无法区分G_STKBusy是有效还是无效，因此在G_STKBusy通道上加1区分,只有在uicc应用才有可能触发STK
	G_STKBusy = (((WORD)(G_CurSelectedChannel+1)) << 8 )|(getChannelContext(G_CurSelectedChannel) & 0xFF); //为防止STK在通道0时，无法区分G_STKBusy是有效还是无效，因此在G_STKBusy通道上加1区分,只有在uicc应用才有可能触发STK

	if(G_NextAPDUFlag == FALSE)
	{  
		CommandFinish(0x00);														//Send 91 XX
	
		SendResponse();
	
		G_NextAPDUFlag = FALSE;  //清除新接收指令的标记
	}

	abortSTKFlag = FALSE;  //是否需要中止当前STK流程，只有在STK 的通道上关闭了通道或者A4命令重新选择了应用，这个时候需要中止当前的STK 操作，该通道上的命令重新回到主流程进行分发
	
	backChannelProfileId = G_CurExecutingProfileIndex;//用于还原当前指令的通道执行环境
	
	for(;;)
	{
		G_CurSelectedChannel = GPGetCurrenteChannel();

//		if(G_NextAPDUFlag == FALSE)
//		{
//			GlobalReceiveCommand();
//
//			AfterReceiveCommandHeader();
//
//		}

#if (EUICC_ONLY||SE_AND_EUICC)
		profileIndex = getChannelProfileIndex(G_CurSelectedChannel);
		if(G_CurExecutingProfileIndex != profileIndex)
		{
			G_CurExecutingProfileIndex = profileIndex; //selected_channel发生变化时，都需要执行该语句
			ExecuEnvironmentInit(G_CurExecutingProfileIndex);			
		}
	
#endif

		if(GPGetCurrenteChannel() == ILLEGAL_CHANNEL)   //通道非法，直接发送6E00
		{
			GlobalSetStatus(0x6881);

			GlobalSendResponse();
			
			G_NextAPDUFlag = FALSE;
			
			continue;
		}
		
		if(G_CommandMode == Command_Mode_First)					//防止在ProfileA切换到ProfileB时候,change模式清空了相关变量,接着ProfileB切换到其他Profile时候 会进入STK 时G_CommandMode变量没赋值
		{
			if(CLA == 0xA0)
			{
				G_CommandMode = Command_Mode_2G;					//当前为2G操作模式

				SW1 = 0x9F;
			}
			else //if (((CLA&0xFC) == 0x80)||((CLA&0xF0) == 0x00)) //白盒测试需要，将if语句移动到下面
			{
				G_CommandMode = Command_Mode_3G;	// 暂时修改为除了A0 模式外，其他的都认为是3G 模式

				SW1 = 0x61;
			}

			SetChannel(E_FILE_START,0x3F00,FALSE);
		}

		if(abortSTKFlag == TRUE)
		{
			G_NextAPDUFlag = GPGetCurrenteChannel()+1;	  //收到下一条合法APDU 命令，标记标上

			G_STKBusy = FALSE;
			
			STKPerform[0] = FALSE;	

#if (EUICC_ONLY||SE_AND_EUICC)
			if(backChannelProfileId != G_CurExecutingProfileIndex)//还原, 执行区域已设置在通道信息中setChannelProfileIndex(selected_channel, G_CurExecutingProfileIndex);
			{
				G_CurExecutingProfileIndex = backChannelProfileId;
				ExecuEnvironmentInit(G_CurExecutingProfileIndex);
			}
#endif

			ThrowErrorBackToMain(Priv_EXCEPTION);  //回到JAVA主流程
		}
		
//		if(STKPerform[0] == 0x02)  //主动式命令已经fetch成功，成功的fetch必须在0通道才能执行，此时切换G_STKBusy为0通道上的数据,默认0通道上必须为UICC应用
//		{
//			G_STKBusy = (0x0100|(ChannelManagement[0].context & 0xFF)); 
//		}
		
		if(((INS == 0xA4)&&(P1 == 0x04))||(INS == 0x70))  //单独处理A4 或 70 命令
		{
			G_NextAPDUFlag = GPGetCurrenteChannel()+1;
			
			backChannelId = G_CurSelectedChannel;//70指令在关通道的时候 会改变selected_channel的值为被关闭的通道号 ，此时需要备份当前执行指令的通道 ，0通道发关通道指令 不能打断其STK的91xx

			G_GPErrorCode = 0x9000;  
			
			GPSubCallFunction(ProcessA470Command_METHOD_ADDR, UICC_PKG_ID, JCRE_CONTEXT);
			
			if(INS == 0x70)
			{
				G_CurSelectedChannel = backChannelId;
			}

			if((INS == 0xA4)&&(G_GPErrorCode == 0x00))  //若当前的选择指令为uicc上的，则直接进入STK 中处理
			{
				G_ChannelNum = CLA&0x03;
				
				SelectCommand();   //此处修改的原因，因为新方案中进入uicc main分发之后一直在里面循环等待，因此STK 中不能再进入uicc的分发机制,久方案兼容新方案								
			}
			else
			{
				GlobalSetStatus(G_GPErrorCode);
			}

			if(getChannelState(((G_STKBusy>>8)&0xFF)-1) == CHANNEL_CLOSED)//(channelInfo.channel_status[((G_STKBusy>>8)&0xFF)-1] == CHANNEL_CLOSED) //STK上的通道已关闭，则中止当前的STK操作
			{
#if (EUICC_ONLY||SE_AND_EUICC)
				 if(G_LpaSendRefresh != LPANoRefreshMode)  // add by chentao@20230114  profile change 过程中收到close指令不退出，等14指令  buglist 16_03
				{

				}
				else
#endif
				{
					abortSTKFlag = TRUE;  //中止当前的STK操作
					STKPerform[0] = FALSE;
				}

			}
			else
			{
				//两次都是操作属于UICC 的A4，则不需要中止当前STK 操作
				//如在UICC应用中触发了STK操作，且在STK 中选择ADF，则不需要中止当前的STK操作
				//如果在其他非UICC 应用中触发了STK，则中止STK
				//if((ChannelManagement[((G_STKBusy>>8)&0xFF)-1].context & 0xFF) == (G_STKBusy&0xFF))
				//G_CurSelectedChannel = GPGetCurrenteChannel();  //对全局变量G_CurSelectedChannel进行有效赋值操作.

				channelContext = getChannelContext(0); //0通道应用

				tempAppId = GetUICCAppId();

				if((tempAppId != 0xFF)&&(U8_AND_U8(U16t8(channelContext),0xFF) == tempAppId))//当前通道上的app是UICC 选中ADF后直接分发给UICC
				{

				}
				else
				{
#if (EUICC_ONLY||SE_AND_EUICC)
					if(G_LpaSendRefresh == LPAChangeMode)  // add by chentao@20230303通道profile状态切换操作，发91XX，0通道选非UICC应用，profile没有切换，正常使用  .修改方案：清全局变量 E2保持不变
					{
						 G_LpaSendRefresh = LPANoRefreshMode;
					}
#endif
					abortSTKFlag = TRUE;
					STKPerform[0] = FALSE;
				}
			}

			GlobalSendResponse();

			continue;
		}
		else
		{
			//G_CurSelectedChannel = GPGetCurrenteChannel();  //对全局变量G_CurSelectedChannel进行有效赋值操作.
						
			if(U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF) == UICC_APPID)  //当前通道上的app是UICC 选中ADF后直接分发给UICC
			{
				G_NextAPDUFlag = FALSE;

				if(INS == 0x14)
				{
					G_ChannelNum = CLA&0x03;
					if(G_ChannelNum != 0) // 收到0通道14才退出STK
					{
						SendResponse();

						continue;
					}

					if(P1P2P3Check(0x00,TRUE) ==FALSE)  
					{
						SendResponse();
		
						continue;
					}
						
					ReceiveData();
			  
				  if(STKPerform[0] == TRUE)		 	//设置STKPerform为3个值:0x00,0x01,0x02,防止开机过程被RFM/RAM打断
					{
						CommandFinish(0x00);														//Send 91 XX
		
						SendResponse();
		
						continue;
					}
		
					G_STKBusy = FALSE;
			
					if(STKPerform[0] == 0x02)		 	//设置STKPerform为3个值:0x00,0x01,0x02,防止开机过程被RFM/RAM打断
					{
						STKPerform[0] = FALSE;	
		
                        #if UICC__FUNCTION__STKAPI
   //                     ClearBit(G_ToolkitSystemProactiveFlag,Bit_Pending);					//收到14命令则中断当前挂起的系统主动式命令流程,  //考虑到手机兼容性，收到应用的terminal response时也终止当前挂起的系统主动式命令，20130925
                        #endif
					}
					
					G_CurSelectedChannel = STKPerform[1]-1;
					
					G_ChannelNum = STKPerform[1]-1;	

					CommandFinish(0x00);													//Send 9000


					if( ResData[6] == RecData[3] )										//compare the response of command que the command send the last before
					{
						num = GetResponseResult();

#if (EUICC_ONLY||SE_AND_EUICC)
						if(backChannelProfileId != G_CurExecutingProfileIndex)//还原 , 执行区域已设置在通道信息中setChannelProfileIndex(selected_channel, G_CurExecutingProfileIndex);
						{
							G_CurExecutingProfileIndex = backChannelProfileId;
							ExecuEnvironmentInit(G_CurExecutingProfileIndex);
						}
#endif
		
						return num;
					}
					else
					{
#if (EUICC_ONLY||SE_AND_EUICC)
						if(backChannelProfileId != G_CurExecutingProfileIndex)//还原 , 执行区域已设置在通道信息中setChannelProfileIndex(selected_channel, G_CurExecutingProfileIndex);
						{
							G_CurExecutingProfileIndex = backChannelProfileId;
							ExecuEnvironmentInit(G_CurExecutingProfileIndex);
						}
#endif
						return ExitStkPerform;											//exit application
					}
				}
		
				CommandAttemper(CephInsList,STK_MODE); //注意增加以下测试用例： 在STK 中由于C2导致了post，且post后接收了新的命令，那么此时响应状态字不能发送出去
				
				continue; 
			}
			else   //进入当前通道的上下文进行命令分发处理
			{
				G_NextAPDUFlag = GPGetCurrenteChannel()+1;
			
				G_GPErrorCode = 0x9000;  

				GPSubCallFunction(ProcessSTKOtherAPDUCommand_METHOD_ADDR, UICC_PKG_ID, JCRE_CONTEXT);
				
				GlobalSetStatus(G_GPErrorCode);
				
				GlobalSendResponse();
				
				continue;
			}
		} 
	}

	return 0;
}
/*-------------------------------------------------------------------------------------------------------//

  Function name:	InitProactiveCommand

  Input parameter:
 					com_type:  type of command 				        	see 11.14 page 89

 					com_qualifier: qualifiers specific to the command  	see 11.14 page 89

 					des_device:   destination device identity          	see 11.14 page 92

 Output parameter:  void

 Description:       init the head of stk_command in the stk buffer:command details ,command device identities

 Status:			OK

 -------------------------------------------------------------------------------------------------------------------*/

void InitProactiveCommand(BYTE commandType,BYTE commandQlify,BYTE desDevice)
{
	ResData[0] = 0xD0;

	ResData[1] = 0xD0;

	ResData[2] = 0x09;

	ResData[3] = 0x81;

	ResData[4] = 0x03;

	ResData[5] = 0x01;

	ResData[6] = commandType;

	ResData[7] = commandQlify;

	ResData[8] = 0x82;

	ResData[9] = 0x02;

	ResData[10]= 0x81;

	ResData[11]= desDevice;
}

/*-------------------------------------------------------------------------------------------------------//

  Function name :	 AddTLV

  input parameter:   TLV_AIM:     be added buffer :the buffer must tlv structure 					        see 11.14 page 44

                     TLV_OBJECT:  want to add object :must the tlv structure

  output parameter:  TRUE/FALSE

  description:       This function is to add the tlv structure to the buffer of have data :the buffer must have data of thv structure

  Status:			OK but need to add function
---------------------------------------------------------------------------------------------------------------------------------------------------*/

BYTE AddTLV(BYTE * TLV_AIM,BYTE  * TLV_OBJECT)
{
    WORD idata length,num; //为避免溢出，修改num类型为WORD

	if(TLV_OBJECT[1]>=0x80)	 	//修改TLV总长度判断条件，len>=0x80需要添加0x81字节
	{
		num = TLV_OBJECT[1] + 0x03;
	}
	else
	{
		num = TLV_OBJECT[1] + 0x02;
	}

    length = TLV_AIM[1] + num;


	if( length > MAX_LENGTH )
	{                                                           //CHECK LENGTH IS OVER FLOWER
		return  FALSE ;
 	}

    if(TLV_OBJECT[1] >= 0x80)
    {
		TLV_AIM[ TLV_AIM[1] + 0x02] = TLV_OBJECT[0] ;

		TLV_AIM[ TLV_AIM[1] + 0x03] = 0x81;

		ecpMem_Copy(&TLV_AIM[ TLV_AIM[1] + 0x04],&TLV_OBJECT[1],num-2);

    }
    else
    {
    	ecpMem_Copy(&TLV_AIM[ TLV_AIM[1] + 0x02],&TLV_OBJECT[0],num);
    }

	TLV_AIM[1] = TLV_AIM[1] + num ;                                                 //LENGTH ADD LENGTH

	return TRUE;
}


/*-------------------------------------------------------------------------------------------------------//

  Function name :	 AddValue

  input parameter:   TLV_AIM:   be added buffer :the buffer must tlv structure 					        see 11.14 page 44

                     LV_OBJECT:  want to add object :must the tlv structure

  output parameter:  TRUE/FALSE

  description:       This function is to add the data structure to lv structure

  Status:			 OK but need to add function
---------------------------------------------------------------------------------------------------------------------------------------------------*/

BYTE AddValue(BYTE * LV_AIM,BYTE * LV_OBJECT)
{
    WORD idata length;

	if(LV_OBJECT[0] == 0x00)
	{
		return TRUE ;
	}

	length = LV_AIM[0] +  LV_OBJECT[0];

	if( length > MAX_LENGTH )
	{                                                           //CHECK LENGTH IS OVER FLOWER
		return  FALSE ;
 	}

	ecpMem_Copy(&LV_AIM[ LV_AIM[0] + 0x01], &LV_OBJECT[1], LV_OBJECT[0]);

	 LV_AIM[0] = length;	                                     //LENGTH ADD LENGTH

	 return TRUE;
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 GetResponseResult
  
  input parameter:   void
   
  output parameter:	 BYTE, 0 = FAIL otherwise reponse result

  description:       Get Response Result 
  					 
  
  status:			Code ok! test ok!			     

 -------------------------------------------------------------------------------------------------------------------*/    

BYTE GetResponseResult(void)
{
	BYTE idata pos;

	pos = FindTag(&RecData[0],TLV_RESULT,P3);

	if (pos == FALSE)
	{
		return ExitStkPerform;
	}
	else
	{
		if(RecData[pos+1] < 0x0F)
		{
			return CMD_Success;
		}
		else if(RecData[pos+1] == 0x11)
		{
			return BackMove;
		}
		else if(RecData[pos+1] == 0x10)
		{
			return ExitStkPerform;
		}
		else
		{
			return (RecData[pos+1]);
		}
	}
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 GetResponseItemIdentifier

  input parameter:   void

  output parameter:	 0 = FAIL, otherwise user selected item identifier

  description:       Get Response Item Identifier


  status:

 -------------------------------------------------------------------------------------------------------------------*/
BYTE GetResponseItemIdentifier(void)
{
	BYTE idata pos,ch;

	if (INS == 0xC2)
	{
		if(RecData[1] == 0x81)
		{
			ch = 0x01;
		}
		else
		{
			ch = 0x00;
		}

		pos = FindTag(&RecData[2+ch],TLV_ITEMIDENTIFIER,RecData[1+ch]);

		if( pos != FALSE)
		{
			return RecData[pos+3+ch];
		}
	}
	else
	{
		pos = FindTag(&RecData[0],TLV_ITEMIDENTIFIER,P3);

		if(pos != FALSE)
		{
			return RecData[pos+1];
		}
	}

	return FALSE;
}
/*@Function:   ReadLFFileValue                                            
 * 
 *@Descrip:    
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      fileAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
WORD ReadLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf)
{
	BYTE idata temp[2];
	WORD idata parameterLen;

	ReadLFBuf(fileAddr, index, 0, temp, 2); //

	parameterLen = BuildWord(temp);
			
	ReadLFBuf(fileAddr, index, 2, &buf[0], parameterLen); //

	return parameterLen;
}

/*@Function:   ReadLFFileValue                                            
 * 
 *@Descrip:    从LF文件读取内容
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      fileAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
BYTE ReadLFFileValue1ByteLen(UWORD fileAddr,BYTE index,BYTE offset,BYTE * buf)
{
	BYTE idata parameterLen;

	ReadLFBuf(fileAddr, index, offset, (BYTE*)&parameterLen, 1); //取Host的长度
			
	ReadLFBuf(fileAddr, index, offset+1, &buf[0], parameterLen); //从文件读取Host的值,长度大于0xFF的暂时不做处理

	return parameterLen;
}

/*@Function:   ClearFileFirstWord                                            
 * 
 *@Descrip:    
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      dfName    
 *@Param:      efName    
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
void ClearFileFirstWord(WORD dfName,WORD efNAme)
{
	UWORD  xdata fileAddr;

	fileAddr = SelectPrivateTFFile(dfName, efNAme);		

	if(fileAddr != FALSE)
	{
		SetTFConst(fileAddr,0x00,0x00, 0x02);;
	}
}
/*------------------------------------------------------------------------------------------------------------                                                                                                                                               
  Function name :	WriteTFFileWord                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function manage the bookmark
  
  status: 			need test
  
  author :          XuWeiHua      
 --------------------------------------------------------------------------------------------------------------*/
void WriteTFFileWord(UWORD fileAddr,WORD len)
{
	BYTE idata temp[2];

	SetWord(temp, len);

	WriteTFBuf(fileAddr, 0x00, temp, 2);
}

/*------------------------------------------------------------------------------------------------------------                                                                                                                                               
  Function name :	ReadTFFileWord                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function manage the bookmark
  
  status: 			need test
  
  author :          XuWeiHua      
 --------------------------------------------------------------------------------------------------------------*/
WORD ReadTFFileWord(UWORD fileAddr)
{
	BYTE idata temp[2];
	WORD  idata len;

	ReadTFBuf(fileAddr, 0x00, temp, 2);

	len = BuildWord(temp);

	return len;
}
/*@Function:   ClearFileFirstWordAddr                                            
 * 
 *@Descrip:    
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      dfName    
 *@Param:      efName    
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
void ClearFileFirstWordAddr(UWORD fileAddr)
{
	SetTFConst(fileAddr,0x00,0x00, 0x02);
}


/*@Function:   WriteLFFileValue                                            
 * 
 *@Descrip:    写入L(2BYTE)V格式数据到记录文件的指定记录
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      fileAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *				V1.1---modify by jiangqb 20191128-增加函数返回值,写入成功返回TRUE, 写文件越界，写入失败时返回false
 *                        
 */
BYTE WriteLFFileValue(UWORD fileAddr,BYTE index,BYTE * buf,WORD length)
{
	BYTE idata temp[2];

	SetWord(temp, length);

	WriteLFBuf(fileAddr, index, 0, temp, 2);
	
	if (!WriteLFBuf(fileAddr, index, 2, buf, length))
	{
		return FALSE;
	}

	return TRUE;
}

/*@Function:   WriteLFFileValue                                            
 * 
 *@Descrip:   
 *
 *@See:	       
 *                                                                                                                                     
 *@Param:      fileAddr      
 *              						                                                                                                  
 *@Return:     BYTE                                                                                                                                                                                                                                                              
 * 
 *@Author:     xwh
 *  
 *@History:    V0.0---Create by xwh 20180801
 *                        
 */
BYTE WriteTFFileValue(UWORD fileAddr,WORD offset,BYTE * buf,WORD length,WORD totalLen)
{
	BYTE idata temp[2];

	SetWord(temp, totalLen);

	WriteTFBuf(fileAddr, 0x00, temp, 2);
	
	if (!WriteTFBuf(fileAddr, offset, buf, length))
	{
		return FALSE;
	}
	
	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------                                                                                                                                               
  Function name :	ReadWORD                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function manage the bookmark
  
  status: 			need test
  
  author :          XuWeiHua      
 --------------------------------------------------------------------------------------------------------------*/
WORD ReadWORD(UWORD addr)
{
	BYTE  idata buf[2];
	
	ReadEeprom(&buf[0],addr,2);	
	
	return BuildWord(&buf[0]);
}

void setChannelNum(BYTE cla)
{
	G_ChannelNum = (cla&0x03);
}

