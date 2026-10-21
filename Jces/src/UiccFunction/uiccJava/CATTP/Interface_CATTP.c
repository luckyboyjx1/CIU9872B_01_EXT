/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   
*
* Description: CATTP相关接口
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0
*
************************************************************************************************************/

#include "FunctionSwitch.h"
#include "Interface_CATTP.h"
#include "CatTp.h"
#include "Ceph.h"
#include "BIPModuleProcess.h"
//#include "Main.h"
#include "ToolkitApiPublic.h"
#include "Stkhead.h"
#include "Gsmpublic.h"
#include "FileAction.h"
#include "fileSystem.h"
#include "ToolkitRfm.h"
#include "ExpandedManage.h"
#include "TimerModuleProces.h"
#include "CompactManage.h"
#include "Interface_LTEFromPlatform.h"
#if UICC__FUNCTION__CATTP
/*
*@function   :IF_CATTP_RetryCattpsessionByTimerExpiration
*
*@description:上一次的CATTP会话异常终止之后，收到timer expiration之后再开启会话，累计尝试次数为
*
*@See        :   
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xxj @20180826         
*/
void IF_CATTP_RetryCattpsessionByTimerExpiration(BYTE timerExpirationId)
{
#if UICC__FUNCTION__CATTP	
	if(timerExpirationId != G_CATTP_TimerIdentifier)   //只有timerID一致时才重启会话
	{
		return;
	}
	
	G_EventTimerExpirationId = FALSE;        //还原初始值
	
	G_BIPDataAvaiableEvent = FALSE;
	
	if(G_BIPModuleStatus.channelNum == FALSE) //当前进行CATTP链接的通道已经关闭  <scenario_01_CATTP_30_6d_04>
	{
		return;
	}
	
	tempBuf[0] =ReadVarible(VaribleTag_BackPDU,&tempBuf[1]);

	if(tempBuf[0] != 0x00)
	{
		G_RetryTimers --; 

		if(G_RetryTimers == FALSE)
		{
			CATTPRleaseTimer();

			//电信测试 5G  Test_PRL_EPRL_OPLMN_3_4  脚本期望发SetUpEvenList
// 			tempBuf[1] = 0x01;  //Location status  TS102 223 p8.25

// 			tempBuf[2] = 0x03;  
// 				
// 		    cephSetUpEvenListSingle(&tempBuf[1]);
#if UICC__OPERATOR__CHINATELECOM
			cephSetUpEvenListSingle(NULL); //下载移动profile测试\6bip\3异常测试重发3次  modify by chentao@20211020
#endif
			cephCloseChannelSingle(0x00, G_BIPModuleStatus.channelNum);    //3次没收到服务器的响应，则关闭通道

			clearCatTPRecord();
			
			BIPModuleInit();

			ClearVariable(VaribleTag_BackPDU);

		//	return;

			return;
		}

		cephSendDataSingle(0x01,G_BIPModuleStatus.channelNum,&tempBuf[0]);

		CATTPRleaseTimer();
		
		//电信测试存在12和14不匹配的情况，这里不做判断 zhhj
		//CATTPStartTimer();
		CATTPStartTimerOnly();
		cephMoreTimeSingle();
	}
#endif	
}
/*
*@function   :IF_HTTPDataAvaiable
*
*@description:接受完PDU 处理PDU arrival events接口
*
*@See        :   
*
*@param      :
*
*@return     :void
*
*@history    :       
*/
void IF_CATTPDataAvaiable(WORD PDULen)
{
#if UICC__FUNCTION__CATTP
//	UWORD xdata bipReceiveAddr;
	
	G_CurrentPDULength = PDULen;
	
	G_BIPDataAvaiableEvent = FALSE;

//	bipReceiveAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Receive);
	
	CATTPPduArrivalEvents();
	
//	if(bipReceiveAddr != FALSE)
//	{
//		SetTFConst(bipReceiveAddr, 0x00, 0x00, 0x02); //初始化接收缓存
//	}

 	//ClearFileFirstWord(DF_BIPModule,EF_BIPModule_Receive);
//	SetEeprom(BIPModule_Receive_ADDR_START, 0x00, 0x02);
	G_BIPModule_Receive_Length = 0x00;
#endif 
}

BYTE IF_CATTP_ChannelStatus(void)
{
#if UICC__FUNCTION__CATTP
	
	if(G_BIPModuleStatus.channelDataType == ChannelDataType_CATTP)
	{
		tempBuf[1] = 0x01;  //Location status  TS102 223 p8.25

		tempBuf[2] = 0x03;  
					
		cephSetUpEvenListSingle(&tempBuf[1]);
				
		cephCloseChannelSingle(0x00, G_BIPModuleStatus.channelNum);    //通道连接发生错误，通道关闭
				
// 		if(G_ChannelStatusID == G_CATBIPChannelId)  //当前失去链接的通道正是之前进行CATTP 链接的通道
// 		{
			CATTPRleaseTimer();
			clearCatTPRecord();
// 		}
		
		G_BIPDataAvaiableEvent = FALSE;
		
		BIPModuleInit();
		return TRUE;
    }

#endif
   return FALSE;	
}
/*
*#function   :IF_GetOpenChannelParaForCATTP
*
*@description:获取CATTP开通道的相关参数
*
*@param      :BYTE *para    获取到的开通道参数的数据TLV+TLV格式
*
*@return     :BYTE 返回para中的有效数据
*
*@history    :created bu xuxiaojun@20180530
*/
BYTE IF_GetOpenChannelParaForCATTP(BYTE *para)
{
#if UICC__FUNCTION__CATTP
	
	BYTE idata i,transportType,flag,bearerType;
	UWORD xdata fileAddr;
	BYTE xdata bipport[0x02];
	BYTE xdata ipAddr[0x08];
	BYTE xdata netAccess[0x10];
	BYTE xdata bearerData[12];
	BYTE CONST * bearPtr;
	BYTE idata Len,tag39;

	fileAddr = SelectPrivateLFFile(DF_CATTP,EF_CATTPPUSH);	

  if(fileAddr == FALSE)
	{
		 return FALSE;
	}  

	tempBuf[0] = ReadLFFileValue1ByteLen(fileAddr,0x01,0x00,&tempBuf[1]);

	G_PDUSize[0x00] = ((MAX_PDU_SIZE>>8)&0xFF);
	
	G_PDUSize[0x01] = (MAX_PDU_SIZE&0xFF);  //buffer size
		
// 	SDUSize[0x00] = ((MAX_SDU_SIZE>>8)&0xFF);
// 	
// 	SDUSize[0x01] = (MAX_SDU_SIZE&0xFF);  //默认max sdu length =MAX_SDU_SIZE


	netAccess[0] = 0x00;

	ipAddr[0] = 0x00;

	bearerData[0] = 0;
	
	transportType = 0;
		
	i = 1;

	flag = FALSE;
	
	tag39 = FALSE;  //默认PDU SIZE从触发短信读取,如果没有的话 ,从Profile配置参数里面查找  都没有的话 使用默认值  MAX_PDU_SIZE
	
	while(i< tempBuf[0])
	{
		switch(tempBuf[i]&0x7F)
		{
			case 0x35:	//8.52 Bearer description
			{
				ecpMem_Copy(bearerData,&tempBuf[i+1],(tempBuf[i+1]+1)>sizeof(bearerData)?sizeof(bearerData):(tempBuf[i+1]+1));
				break;
			}

			case 0x39:  //buffer size  =MAX SDU SIZE  8.55 Buffer size
			{				
//				bufSize[0x00] =tempBuf[i +2];	
//				bufSize[0x01] = tempBuf[i +3];  //buffer size
				
				G_PDUSize[0x00] =tempBuf[i +2];
				
				G_PDUSize[0x01] = tempBuf[i +3];  //buffer size
				
				tag39 = TRUE;
				
				break;
			}

			case 0x3C:   //8.59 UICC/terminal interface transport level
			{			
				transportType = tempBuf[i+2];
	
				bipport[0]  = tempBuf[i +3];  
				bipport[1]  = tempBuf[i +4];  

				flag = TRUE;
				
				break;
			}
			case 0x3e:  //8.58 Other Address
			{
				ecpMem_Copy(ipAddr,&tempBuf[i+1],tempBuf[i+1]+1);
											
				break;
			}
			case 0x47: //8.70 Network access name
			{
				ecpMem_Copy(netAccess, &tempBuf[i+1],tempBuf[i+1]+1); //LV 结构
								
				break;
			}
			default: //白盒测试添加
				break;
		}
		
		i += tempBuf[i+1]+2;	
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	if(tag39 == FALSE)  //触发短信没有来 39 tag则
	{
		IF_GetCATTPPara(G_PDUSize,0x01);  //获取PDU Size
	}
#endif
	if((ipAddr[0] == 0x00)||(flag == FALSE)) //IP地址或IP端口号不存在，则返回错误
	{
		SetLFConst(fileAddr, 0x01,0x00,0x00,sizeof(BYTE));

    	SetLFConst(fileAddr, 0x02,0x00,0x00,sizeof(BYTE));	 //清除存放BIP信息，CATTP信息的文件

		return FALSE;	
	}


	if(bearerData[0] != 0)
	{
		bearerType = bearerData[1];	
		ecpMem_Move(&bearerData[1],&bearerData[2],bearerData[0]-1);
		bearerData[0]--;
		bearPtr = &bearerData[0];
	}
	else
	{
		bearerType = 0x03;	
		bearPtr = NULL;
	}
    //----------------------------------------参数-------------------------------------------------------------
	Len = 0;
	
	para[0] = TLV_BEARERDESCRIPTION;	//bearer description
	
	Len++;
	
	if( bearPtr == NULL )
	{
		para[1] = 0x01;

		para[2] = bearerType;
	}
	else
	{
		para[1] = bearerData[0] + 1;

		para[2] = bearerType;

		ecpMem_Copy(&para[3], (BYTE CONST *)&bearerData[1], bearerData[0]);
	}
	
	Len += para[1]+1;
	
	para[Len] = TLV_BUFFERSIZE;	//Buffer Size

	para[Len+1] = 0x02;

	para[Len+2] = G_PDUSize[0];

	para[Len+3] = G_PDUSize[1];
	
	Len += 4;
	
	if(netAccess[0] != 0x00)
	{
		para[Len] = TLV_NETWORKACCESSNAME;

		ecpMem_Copy( &para[Len+1], &netAccess[0], netAccess[0]+1);
		
		Len += netAccess[0]+2;
	}
	
	para[Len+0] = TLV_SIMMEINTERFACETRANSPORT;

	para[Len+1] = 0x03;

	para[Len+2] = transportType;

	para[Len+3] = bipport[0];

	para[Len+4] = bipport[1];
	
	Len += 5;
	
	para[Len] = 0x3E;		//address

	ecpMem_Copy( &para[Len+1], &ipAddr[0], ipAddr[0]+1);
	
	Len += ipAddr[0]+2;
	
// 	ecpMem_Move(&para[1],&para[0],Len);
// 	
// 	para[0] = Len;
	
	return Len;
#else 
	return 0;
#endif	
}
/*
*@function   :IF_CATTP_ariableInit
*
*@description:openchanel之后的SYN SEND
*
*@See        :   
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0   
*/
void IF_CATTP_ariableInit(void)
{
}
/*
*#function   :IF_CATTPcephMoreTime
*
*@description:根据当前有没有收到Data Avaiable事件返回
*
*@param      :
*            :
*
*@return     :
*
*@history    :
*/
BYTE IF_CATTPcephMoreTime(void)
{
#if UICC__FUNCTION__CATTP
	
	if((G_BIPDataAvaiableEvent == TRUE) || ((G_EventTimerExpirationId == G_CATTP_TimerIdentifier)&&(G_EventTimerExpirationId != FALSE)))
	{
// 		if((G_CATTPSndSEQ != 0x00)&&(G_CATTPSndSEQ == CATTPRecord.CUR_PDU_ACK_NB))
// 		{
// 			G_CATTPEventTag = FALSE;
// 			return TRUE;
// 		}
		
// 		if(G_CATTPSndSEQ == 0x00)
// 		{
			G_BIPDataAvaiableEvent = FALSE;
		    //G_EventTimerExpirationId = FALSE; 执行D7 时间终止tag时候用到
			return TRUE;
// 		}
		

	}
	else
	{
		return FALSE;
	}
#else
	return TRUE;
#endif	
}
/*
*#function   :IF_SendPDU
*
*@description:根据当前有没有收到Data Avaiable事件返回
*
*@param      :
*            :
*
*@return     :
*
*@history    :
*/
#if UICC__FUNCTION__CATTP
void IF_SendPDU(void)
{
	WORD  idata PDULen,ChecksumLen,Checksum;
	UWORD xdata porDataFileAddr;
	BYTE idata CATTPHeaderLen;
	
	CATTPHeaderLen = 0x12;
	
	PDULen = CATTPRecord.SYN_PDU_SIZE_MAX-CATTPHeaderLen;   //除去头长度
	
	porDataFileAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);
	
	if(porDataFileAddr == FALSE)
	{
		return;
	}
	
	ReadTFBuf(porDataFileAddr,0x00,&tempBuf[0],CATTPHeaderLen);
	
	if(G_CurrentSDUForSendLength>PDULen)
	{
		ChecksumLen = PDULen;
		
	}
	else
	{
		ChecksumLen = G_CurrentSDUForSendLength;
		
		tempBuf[0] &= 0xFB;  //最后一条数据 CATTP_HEADER_SEG_BIT为0
	}
	
	tempBuf[10] = (CATTPRecord.SND_NXT_SEQ_NB >>8)&0xFF;	
	tempBuf[11] = CATTPRecord.SND_NXT_SEQ_NB&0xFF;

	WriteTFBuf(porDataFileAddr,0x00,&tempBuf[0],CATTPHeaderLen);
	
	//Checksum = CATTPCalCheckSumAddr(porDataFileAddr,ChecksumLen+CATTPHeaderLen,0x00);    //计算  CATTPRecord.SYN_PDU_SIZE_MAX  Checksum
	
	Checksum = CATTPCalCheckSumAddr(porDataFileAddr,ChecksumLen+CATTPHeaderLen,sizeof(TranFileHead));    //配合函数里面ReadEeprom而修改
	
	tempBuf[16] = (Checksum>>8)&0xFF;
	
	tempBuf[17] = Checksum&0xFF;

	WriteTFBuf(porDataFileAddr,0x00,&tempBuf[0],CATTPHeaderLen);
	
	G_RetryTimers = 0x03;
	
	if(CATTPRecord.SND_NXT_SEQ_NB < (CATTPRecord.SND_UNA_PDU_SEQ_NB + CATTPRecord.SND_WIN_SIZE))   //send ACK DATA  
	{	
		CATTPSendMoreData(porDataFileAddr,ChecksumLen+CATTPHeaderLen,CATTPRecord.channelId);
		
		G_CATTPSndNextSeqNB = TRUE;
	
		CATTPRecord.SND_NXT_SEQ_NB +=1;		
	}

//	WriteTFFileNonAtomic(porDataFileAddr,CATTPHeaderLen,porDataFileAddr+ sizeof(TranFileHead)+CATTPHeaderLen+ChecksumLen,G_CurrentSDUForSendLength-ChecksumLen);
//	SecureMoveEeprom(porDataFileAddr+sizeof(TranFileHead)+CATTPHeaderLen,(porDataFileAddr+ sizeof(TranFileHead)+CATTPHeaderLen+ChecksumLen),G_CurrentSDUForSendLength-ChecksumLen);
	
	VMCopyDataToAddr(porDataFileAddr+sizeof(TranFileHead)+CATTPHeaderLen,GetTFSize(porDataFileAddr)-CATTPHeaderLen,(porDataFileAddr+ sizeof(TranFileHead)+CATTPHeaderLen+ChecksumLen),G_CurrentSDUForSendLength-ChecksumLen,MODE_PROTECT_WRITE);
	
	G_CurrentSDUForSendLength -= ChecksumLen;
	
	if(G_CurrentSDUForSendLength !=0x00)      //还有数据发送  则备份已经发送的SEQ
	{
		G_CATTPSndSEQ = CATTPRecord.SND_NXT_SEQ_NB-1;     //备份SEQ
		
	}
	else
	{
		GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);
	}
}
#endif
/*
*@funtion    :IF_SendPor
*
*@description: SMS下来的触发CATTP Openchannel时候如果要求SMS返POR要等SYN之后才能发CATTP_SYN_SENT
*
*@param      : 
							 
*            
*@return     :TRUE(发POR)/FALSE(不发POR 此时是触发短信)
*
*@author     :LCZ 2019-4-16
*
*/
BYTE IF_SendPor(void)
{
#if UICC__FUNCTION__CATTP	
    if(G_PushCommand == TRUE)
	{
		G_PushCommand = FALSE;
	
		CATTPOpenLink();	
		
		CATTPSavePushPara(G_FormatAndType);    //CATTPPushPOR  发送的数据时手动拼装的ABXX/AFXX 还是要从EF_RAMExtendResponse文件读取？如果从文件读取 不能再这里清空文件
		
		GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile); //清空扩展执行文件 防止扩展格式下来Push指令 然后发送SYN 再去发送POR的时候 这时候数据8001xx23029000等目前是手动拼装的,导致EF_RAMExtendResponse文件数据没有被清
	
		return FALSE;
	}
	else
	{
		return TRUE;  //不是触发短信
	}
#else
	return TRUE;  //	
#endif
}
#endif
