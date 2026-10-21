/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.22

 * MODEL: 	All model

 * RELEASE: 2020-3-2

 * AUTHER:	杨健

 * HISTROY:	V1.0  创建源文件,用于中国电信平台
 			V1.1  修改IF_AccessCondition等函数,支持中国电信234G卡新增RFM权限判断需求

         
					
*/

#include  "typedef.h"
#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include  "error.h"
#include  "arithmatic.h"
#include  "Authentication.h"
#include  "ceph.h"
#include  "interface.h"
#include "Transaction.h"
#ifdef High_Update_FOR_Test
#include  "PrivateCommandTest.h"
#endif


#ifdef LDFile
#include "LDFile.h"
#endif
#ifdef CDMA
#include  "cdma.h"
#endif
#ifdef MD5
#include  "md5.h"
#endif
#ifdef CATTP
#include  "CatTp.h"
#endif

BYTE    xdata 	 G_MenuIndex;
BYTE    xdata    G_NextMenuIndex;
BYTE    xdata    G_NextPageTag;      //标记是否需要显示下一页
BYTE    xdata    G_BackMenuIndex;    //保存第一页的最后一个标识符

BYTE 	xdata	 G_SMItemLen;
BYTE 	xdata	 G_SMLastMenuID;

BYTE code  prePage[]={0x8F,0x08,D_PrePageItem,0x80,0x4E, 0x0A,0x4E,0x00, 0x98, 0x75};

BYTE code  lasPage[]={0x8F,0x08,D_NextPageItem,0x80,0x4E, 0x0B,0x4E,0x00,0x98,0x75};

BYTE code recordIsEmpty[] = {10,0x8B,0xB0,0x5F,0x55,0x4E,0x3A,0x7A,0x7A,0xFF,0x01}; 

//unsigned char idata G_chipVar1 _at_ 0x99;			//该IRAM(0x98,0x99)空间不允许用户COS占用,芯片内部维护
//unsigned char idata G_chipVar2 _at_ 0x98;	
 

//BYTE xdata VARIABLEBUFFER[0x300];	  //若要求OTA功能默认大小为0x300
BYTE xdata VARIABLEBUFFER[0x100];

BYTE	xdata  G_TerminalProfile[0x20];

#ifdef ATR_PSEUDO	
BYTE code ATR_PSEUDOFORUSER[15]={0x80,0x31,0xE0,0x73,0xFE,0x21,0x1B,0x57,0x37,0x86,0x60,0x9E,0x00,0x80,0x00};
#endif							



/**
 * @brief 		SAMSUNG设置EEPROM地址的接口
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 20:00
 */
void IF_EepromAddressSetting(void)
{	
#ifdef ST33G384	
	  G_CODE_SIZE = 0x27F00;						//CODE区大小，此处根据需求调整
#else
		G_CODE_SIZE = 0x16000;						//CODE区大小，此处根据需求调整
#endif		
		
#ifdef High_Update_Function
	//G_High_Update_NUM = 0x28;					//根据需求设置,不需要使用时配置为0
	G_High_Update_NUM = 0x00;	
#else
	G_High_Update_NUM = 0x00;
#endif
}

/**
 * @brief 		设置ATR中的应用版本号字段
 *
 * @param 		appVerId
 *
 * @author 		WG
 *
 * @history 	add comment by WG, @071220 20:00
 */
void IF_SendATR(BYTE lgth)
{
#ifdef ATR_PSEUDO	
	BYTE idata tag;
	UWORD idata fileAddr;
#endif	
	
	ResData[lgth]=0;   //应用版本号
	
	ResData[lgth+1]=0;	

#ifdef ATR_PSEUDO	

				
	fileAddr = SelectTFFile(DF_MF,TF_ATR);								

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr,0,(BYTE *)&tag,0x01);

		if(tag == 0x80)
		{
			ReadTFBuf(fileAddr,1,(BYTE *)&ResData[5],sizeof(ATRDefine)-3);
			return;
		}
	}	
	//memcpy(&ResData[5],(BYTE CONST *)&ATR_PSEUDOFORUSER[0],sizeof(ATR_PSEUDOFORUSER));
	
#endif

}

/**
 * @brief 		
 * @author 		
 *  			
 */
void IF_ATRChange(void)
{


}


BYTE IF_ActiveAppCommand(void)
{		
	return FALSE;
}


void IF_ProcessApp(void)
{
	return;
}


BYTE IF_SelectCommandP2Validity(void)
{
	if((P2&0x93) != 0x00)
	{
		P1P2Error();					//Send 6B 00
		return FALSE;
	}

	return TRUE;

}


BYTE IF_SelectCommandP2Validity2(void)
{
	if((P2&0x80)==0x80)
	{
		P1P2Error();					//Send 6B 00
		return FALSE;
	}

	return TRUE;

}







BYTE IF_ParentDFFCP(UWORD fileAddr,WORD fileName)
{
	BYTE  idata responseLgth;
	
	if(G_A4Control == FALSE)
	{
		responseLgth =  SetChannel(fileAddr,fileName,TRUE);
	}
	else
	{
		responseLgth = G_A4Control;
	}
			
	if((P3 == 0x00)||(P3 >responseLgth))
	{	
		RepeatPreviousCmdHeader(responseLgth);
	}
	else
	{
		SendData();
				
		if(G_A4Control == FALSE)
		{
			CommandFinish(0x00);
		}
		else
		{
			memcpy(&RecData[0],&RecData[P3],responseLgth-P3);
				
			CommandFinish(responseLgth- P3);
		}
	}
			
	return TRUE;
}

/**
 * @brief 		main()中的系统变量和参数初始化的接口
 *
 * @author 		XuQingsheng
 *
 * @history 	created by XuQingsheng, @071220 20:00
 */
void IF_InitSystem(void)
{
	InitVariable();
	
	STKPerform = FALSE;
	
	G_AdmStatus = 0x00;

	//G_NetWorkMode = CDMA_NETWORK;		

	G_SMItemLen = 0x00;
	
	G_AppControl = 0x40;				//bit7置1

	G_OpenControl = 0x00;
	
	STKControl = TRUE;

	G_STKBusy = 0x00; 
	
	memset(G_TerminalProfile, 0x00,sizeof(G_TerminalProfile));
	
	InitUpdateVariable();

	G_DF_ID_USIM = 0x7FF0;

	G_DF_ID_CSIM = 0x7FF1;	    //卡片支持CSIM时使用

  	G_SetUpEventListSent = FALSE;

	//G_Cur_IMSI_RecNo = 0x01;                              

#ifdef LDFile	
	G_EarlyResponseTag = 0x00;
	
	G_CompactExtendedTag = 0x00;

	G_LDFileDeliver = FALSE;

	G_LDFileDeliverMuli = FALSE;  

	G_ExtendedRFMComNUM = 0x00;
	
	G_RFMAllReceived = FALSE;
#endif

#ifdef CDMA		
	G_CdmaStatus = FALSE;

	G_P1 = 0x00;						  		
	
    memset(&G_P2[0],0x00,sizeof(G_P2));

#endif


#ifdef MD5
	memset(PADDING, 0x00, sizeof(PADDING));

	PADDING[0] = 0x80;
#endif

#ifdef CATTP

	G_PushCommand = FALSE;
		
	G_BipPrlStatus = 0x00;

	G_EventTag = FALSE;

	memset((BYTE *)&CATTPRecord, 0x00 ,sizeof(CATTPRecord));

	ClearVariable(VaribleTag_BackPDU);

	G_CATBIPChannelId = FALSE;
	
	G_prePduNoData = FALSE;
	
	G_SDUARRIVAL = FALSE;
	
	G_CATTPOperation = FALSE ;
	
	G_CATTPSndNextSeqNB = FALSE;
	
	G_CATTPCurrentTotalNumber = FALSE;  	//当前尚未收到PRL数据

	G_CATTPPreNumber = FALSE;            //记录上一条的PRL短信序号

	G_LASTSDU_NB = FALSE;      					//sdu最后的一块SEQ

	G_LASTSDU_Flag = FALSE;  					//判断当前是否收到了SDU的最后一块
	
#endif


#ifdef PRL
	G_EPRL_PROCING = FALSE;
#endif

}


/**
 * @brief 		main()中的系统变量和参数初始化的接口
 *
 * @author 		zdm
 *				开机过程中，读完PIN状态后的接口文件。目前是开机后的第一条指令中处理

 * @history 	created by zdm, @20160817
 */
void  IF_AfterReadPin(void)
{
	BYTE  xdata tem[2];	
	WORD idata offset;
			 	
	offset = FindCOSConfigTag(Tag_USIMAlgorithm_Type,&tem[0]);

	if(offset == OFFSET_NULL)	  		//没找到相关配置
	{
		G_USIMAlgorithmType = AlgorithmType_MilenageNORMAL;			
	}
	else
	{
		G_USIMAlgorithmType = tem[0];				
	}	
	
}


/**
 * @brief 		IF_SelectFile，
 *
 * @author 		可以通过此接口修改多电话本多短信
 *
 * @history 	created by zdm,@081030
 */
WORD IF_SelectFile(WORD fileName)
{
	return 	fileName;
}


/**
 * @brief 		对扩展指令的处理，可自行添加扩展指令
 *
 * @author 		xyz
 *
 * @history 	created by zxl, @071220 20:01
 */
void IF_ReceiveAPDU(void)
{
	switch (INS)
	{
		case 0x88:
#ifdef CSIM
			RunAuth();
#else
			RunAlgorithm();
#endif
			break;

#ifdef CDMA
		case 0x84:
			UpdateSSD();
			break;
		case 0x82:
			ConfirmSSD();
			break;
		case 0x8A:
			BaseStationChallenge();
			break;
		case 0x8E:
			GenerateKey();
			break;
		case 0xDE:
			StoreESNMe();
			break;
#endif
#ifdef MD5
		case 0x80:
			RunMD5Algorithm();
			break;
#endif
#ifdef LDFile		
		case 0xE1:		
			LDCleanCounter();
			break;
#endif


#ifdef DELRESIZEFILE
		case 0xE4:
		   DeleteFile();
		   break;
		case 0xD4:
		   ResizeFile();
		   break;
#endif
		case 0xDD:		
			CalCRC();
			break;
		
#ifdef High_Update_FOR_Test
		case 0xA5:
			TFFileUpdateForTest();
			break;
		case 0xAA:		
			PersoRead_Update_Config();
			break;
		case 0xAB:		
			PersoRead_Update_START();
			break;
		case 0xAC:		
			PersoHighUpdate_TEST();
			break;
#endif

		default:
			InsError();
			break;
	}
}

/**
 * @brief 		ReceiveData()收到数据后写EEPROM前的接口函数，能处理11.17命令的特殊功能需求；
				注意：不允许改变RecData的内容
 *
 * @Returns		TRUE: 继续执行后续操作
 *				FALSE: 终止后续操作
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
BYTE IF_ReceiveData(void)
{
	return TRUE;
}

/**
 * @brief 		SendData()发送数据前的接口函数，能处理11.17命令的特殊功能需求
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
void IF_SendData(void)
{

}

/**
 * @brief 		CommandFinish()执行前写EEPROM后的接口函数，能处理11.17命令的特殊功能需求
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
void IF_ApduFinish(void)
{	

}

/**
 * @brief 		处理服务器下发的各种下行数据的接口。如：各省份的特定品牌需求
 * 
 * @param 		tar :TAR最后一个字节
 * @param 		tarPos :RecData[tarPos]==0xB0
 * @param 		udPos :RecData[udPos]==UDL
 * 
 * @author 		WG 
 * 
 * @history 	add comment by WG, @071220 22:37
 */
void IF_PP_Download_Hanlder(BYTE tar,BYTE tarPos, BYTE udPos)
{
	switch (tar)
	{
		default:
			break;	
	}

	return;
}
/**
 * @brief 		处理不带有UDHL的下行数据;
 * 
 * @param 		udPos :RecData[udPos]==UDL
 * 
 * @author 		xuqs 
 * 
 * @history 	add comment by xuqs, @071220 22:37
 */
void IF_NO_UDHL_Hanlder(BYTE udPos)
{
}

/**
 * @brief 		对下行数据短信进行处理，根据不同的TAR类型执行应用或者存储
 *
 * @author 		xyz
 *
 * @history 	created  by xyz, @080215
 */
BYTE PP_Download_Hanlder(void)
{
	BYTE idata ptr;
	BYTE idata result;
	BYTE idata udPos;
	
#ifdef LDFile
	BYTE idata midPtr;
#endif

	
#ifdef CDMASMS                        
	udPos = GetCDMAUDPos();
#endif

	if(udPos != FALSE)			//ponit to UDL				
	{
		ptr = udPos + 1;       //have the UDHL

		result = FindTag(&RecData[ptr+1], 0x70, RecData[ptr]); //027000

		if(result != FALSE)
		{
#ifdef	LDFile	
			midPtr = ptr + RecData[ptr] + 3 + 5;		//point to the TAR
						
				if(IsRFMTar(&RecData[midPtr]) == TRUE)  // 当前为RFM tar ，注意B00011 和B00111是否是电信RFM 要求的tar
				{
					if(RecData[udPos+1] == 0x02)
					{
						StoreDataSms(&RecData[udPos],0xFF,1,1,LDFileSingleProgess);
					}
					else
					{
						StoreDataSms(&RecData[udPos],RecData[udPos+4],RecData[udPos+6],RecData[udPos+5],LDFileMuliProgess);	
					}
					
					return TRUE;
				}
#endif
		
		}
		else
		{
#ifdef	LDFile
				StoreDataSms(&RecData[udPos],RecData[udPos+4],RecData[udPos+6],RecData[udPos+5],LDFileMuliProgess);
				
				return TRUE;
#endif
		}	
		
	}
	
}




/**
 * @brief 		this function is to process EVENT download
 *
 * @return		TRUE/FALSE
 *
 * @author 		XuWeiHua
 *
 * @history 	created  by xuqs, @081215
 */
BYTE EventDownload(void)
{
	BYTE  idata pos;
	BYTE  idata result;

	if(RecData[1] == 0x81)
	{
		pos = 3;
	}
	else
	{
		pos = 2;
	}

	if((RecData[pos]& 0x7F) != 0x19)		                   	//event tag
	{
		return FALSE;
	}

	pos = pos + 2;	                                          		//event offset

	switch(RecData[pos])
	{
#ifdef CATTP
		case 0x09:		
			if(G_CATTPOperation == TRUE)  //#ifdef CATTP
			{
				result = CATDataAvailable(pos + 5); 
			}
			break;

		case 0x0A:	
			result = ChannelStatus(pos+5);
			break;
#endif

#ifdef MULTI_IMSI
		case 0x03:
			ProcessLocationStatusEvent();
			break;
#endif
		default:
			result=TRUE;
			break;
	}

	return result ;

}



/**
 * @brief 		This function is to send SetUpEventList command,可根据需求添加发送事件
 *
 * @return		
 *
 * @author 		
 *
 * @history 	
 */

void SetUpEventList(void)
{

	tempBuf[0] = 0x02;

	tempBuf[1] = 0x09;

	tempBuf[2] = 0x0A;

	InitProactiveCommand(CMD_SETUPEVENT,0x00,DEV_ME);

	RecData[0] = 0x99;

	memcpy((BYTE *)&RecData[1], &tempBuf[0], tempBuf[0]+1);

	AddTLV((BYTE *)&ResData[1], (BYTE CONST *)(BYTE *)&RecData[0]);	
}




/**
 * 		建立菜单
 *      需要建立实际应用菜单时(多IMSI),可选择此函数
 *
 * @history 	created by XuQingsheng, @071220 20:01
 */
void SetUpMenu(void)
{
/*	
	UWORD idata fileAddr;
	BYTE idata i,j,total;
	
	
	STKPerform = FALSE;
	
	if(ValueBit(G_SystemStatus,B_meSurportStk) == FALSE)  
	{	
		return;
	}	
	
	G_NextPageTag = FALSE;
	
	G_MenuIndex = 1;
	
	G_SMItemLen = 0x00;

	InitProactiveCommand(CMD_SETUPMENU,Q_SETUPMENU,DEV_ME);

	fileAddr = SelectTFFile(DF_TELECOM,EF_SUME);
	
	if(fileAddr != FALSE)
	{
		RecData[0] = 0x85;
	
		ReadTFBuf(fileAddr,1,(BYTE *)&RecData[1],1);

		ReadTFBuf(fileAddr,2,(BYTE *)&RecData[2],RecData[1]);
	
		AddTLV((BYTE *)&ResData[1],(BYTE *)&RecData[0]);
	}

	fileAddr = SelectLFFile(DF_OTA,EF_Menu);
	
	if (fileAddr == FALSE)
	{
		return;	
	}
	

	total = GetItemIDinEFMenu(fileAddr, tempBuf);

	for(i = 0xFA; i < 0xFF; i++)	   		   //FB-FF为完全固化目录
	{
		for(j =  1; j <= total; j++)
		{
			if (tempBuf[j] == (i+1))
			{
				AddMenuItem(fileAddr, j);		
				break;
			}
		}
	}

   
	for(i = 0x01; i < 0xFA; i++)
	{
		for(j =  1; j <= total; j++)
		{
			if (tempBuf[j] == i)
			{
				if(AddMenuItem(fileAddr, j)==FALSE)
				{
				   	AddTLV(&ResData[1], (BYTE *)&lasPage[0]);					

					G_SMLastMenuID = i;	
					
					STKPerform = TRUE;
					
					G_BackMenuIndex = G_MenuIndex;
	
					return;			
				}

				break;
			}
		}
	}
	
	
	if(FindTag(&ResData[3],0x8F,ResData[2]) != FALSE) 
	{			
		STKPerform = TRUE;
	}
	*/
}



/**
 * @brief 		处理菜单选择结果，请根据需求修改
 * 
 * @Returns   	CMD_Success
 * 
 * @author 		WG 
 * 
 * @history 	created by XuQingsheng, @071220 20:01
 */
BYTE MenuSelection(void)
{
	//BYTE  xdata menu[5];
	//UWORD xdata fileAddr;
	BYTE  xdata num;

	InitVariable();	

	num = RecData[1]>0x80?3:2;

	num = num + RecData[num+1]+2;    								//the number of item identifier

	if(((RecData[num]&0x7F) == 0x10)&&(RecData[num+1] == 0x01))
	{
		;
	}

	return CMD_Success;
}


#ifdef LDFile

/**
 * @brief	Sumbit方式发送上行PoR
 *
 * @param	dataBuf:用户数据及长度
 * 
 * @author	PL
 * 
 * @history	created at 20121029
 */
void LDFileSMSSubmit(BYTE * dataBuf)
{
	BYTE xdata address[16];
	BYTE idata i,tmp;
	
	memset(RecData, 0x00, sizeof(RecData));
		
	RecData[0] = ReadVarible(LDFileTPOATag, (BYTE *)&RecData[1]);	//读取TPOA

#ifdef CDMASMS	
	if (RecData[2] != 0x81)		//增加81
	{
		if (RecData[2] != 0x91)
		{
			RecData[0]++;
			
			memmove((BYTE *)&RecData[3], (BYTE *)&RecData[2], (RecData[0]-2));
		}
		
		RecData[2] = 0x81;
	}
#endif
	
	for (i = 0x03; i <= RecData[0]; i++)	//高地4位翻转
	{
		tmp = RecData[i] & 0x0F;
		
		RecData[i] = (RecData[i] >> 0x04) & 0x0F;
		
		RecData[i] = ((tmp << 0x04) & 0xF0) | RecData[i];
	}
		
	RecData[++RecData[0]] = 0x7F;		//PID
	
	RecData[++RecData[0]] = 0xF6;		//DCS
	
	RecData[0]++;
	
	memcpy((BYTE *)&RecData[RecData[0]], (BYTE *)&dataBuf[0], (dataBuf[0] + 1));	//UDL+UD
	
	RecData[0] += dataBuf[0];
	
	memcpy(tempBuf, RecData, (RecData[0] + 1));
	
	if (FALSE != LoadSCAddress((BYTE *)&address[0]))	                   	// 读短信中心号码
	{
		//commQualify=0x80表示“the  CHARi  field  in  the  User  Data  Subparameter  contains  a  User  Data  Header”
		cephSendShortMessageSingle(0x80, NULL, (BYTE *)address, (BYTE *)tempBuf);	
	}
	else
	{
		//commQualify=0x80表示“the  CHARi  field  in  the  User  Data  Subparameter  contains  a  User  Data  Header”
		cephSendShortMessageSingle(0x80, NULL, NULL, (BYTE *)tempBuf);		//发送上行短信
	}
}

#endif






/**
 * @brief 		ProcessPendingMessageAlert的接口函数，用于处理扩展的标记
 *
 * @param 		pendingTag :已经打上的标记
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @081220 22:37
 */
void IF_ProcessPendingMessageAlert(BYTE pendingTag,BYTE exaInfo)
{
#ifdef LDFile
	G_LDFileDeliver = FALSE;		//标记为不返回9FXX/61XX/9EXX
	
	G_LDFileDeliverMuli = FALSE;
	
	ClearVariable(LDFileDeliverTag);		//清除deliver级联时标记
#endif	
	
	
	switch (pendingTag)
	{
#ifdef PRL
		case D_PRLDisplayTag:
			ShowPRLInfo();
			break;
#endif
		
#ifdef CATTP
			case D_ChannelStatus:
				 tempBuf[1] = 0x01;  //Location status  TS102 223 p8.25
	
				 tempBuf[2] = 0x03;  
					
			     cephSetUpEvenListSingle(&tempBuf[1]);
				
				 cephCloseChannelSingle(0x00, G_ChannelStatusID);    //通道连接发生错误，通道关闭
				
				
				if(G_ChannelStatusID == G_CATBIPChannelId)  //当前失去链接的通道正是之前进行CATTP链接的通道
				{
					CATTPRleaseTimer();
					
					clearCatTPRecord();
				}
				break;
			case D_CATTPDataAvaiable:
				BIPDataAvaiable();	
				break;
				
			case D_RFMSucessTag: //执行安全命令包			
				LDFileProgess(pendingTag);        //公用一套
				break;
			case D_CATTPProcessSDU:
				CATTPProcessSDUData();
			    CATTPClearFile(LDFile_RelaxFile);
				break;
			case D_TimerExpiration:
	
				tempBuf[0] =ReadVarible(VaribleTag_BackPDU,&tempBuf[1]);
	
				if(tempBuf[0] != 0x00)
				{
					G_RetryTimers --; 
	
					if(G_RetryTimers == FALSE)
					{
						CATTPRleaseTimer();
	
						tempBuf[1] = 0x01;  //Location status  TS102 223 p8.25
	
						tempBuf[2] = 0x03;  
							
					    cephSetUpEvenListSingle(&tempBuf[1]);
				
						cephCloseChannelSingle(0x00, G_CATBIPChannelId);    //3次没收到服务器的响应，则关闭通道
	
						clearCatTPRecord();
	
						ClearVariable(VaribleTag_BackPDU);

						break;
					}
	
					cephSendDataSingle(0x01,G_CATBIPChannelId,&tempBuf[0]);
	
					CATTPRleaseTimer();
	
					CATTPStartTimer(0x00);
				}
				break;
#endif		

	
#ifdef LDFile
		case LDFileSingleProgess:		//单条RFM处理
		case LDFileMuliProgess:			//多条RFM处理
		{
			LDFileProgess(pendingTag);				
			break;
		}
#endif		
		
		default:
			break;
	}
	

}

/**
 * @brief 		RFM 指令接口函数
 * 
 * @param       recNo:当前被更新文件的记录号
 *
 * @Returns   	VOID
 *
 * @author 		XWHUA
 *
 * @history 	created by XWH
 */

void IF_UpdateRecord(BYTE recNo)
{
	
#ifdef  High_Update_Function
	IF_Switch_High_Update_START();
	  
#endif
}

BYTE IF_UpdateBinary(void)
{	
#ifdef  High_Update_Function
	IF_Switch_High_Update_START();  
#endif

	return FALSE;

}

 /*
 *@description:如果存在过程调用，则获取当前变量所指示的实际变量名字
 *
 *@param      :	variablename:变量名
 *
 *@returns    :返回当前变量名所代表的实际变量名
 *
 *@history    :create by xyz@20100531 13:40
 */
BYTE IF_GetVaribleAddr(BYTE variablename)
{
	return variablename;
}

 /*
 *@description:判断当前变量名是否有效
 *
 *@param      :variablename :变量名称
 *
 *@returns    :TRUE:名称有效/FALSE：变量名无效
 *
 *@history    :create by xyz@20100531 13:43
 */
BYTE IF_WriteVariable(BYTE variablename)
{
	return TRUE;
}


/**
 * @brief 		IF_TerminalResponse接口，可在此处理如开机导航等功能；	
 *				注意：在STK中的TerminalResponse不能使用该接口
 * 
 * @Returns   	void
 * 
 * @author 		xuyz 
 * 
 * @history 	created by xuyz
 */
void IF_TerminalResponse(void)
{			
	return;
}

/**
 * @brief 		开机TerminalProfile接口，可在此处理开机时发送主动式命令等功能；
 *				注意：在STK中的TerminalProfile不能使用该接口	
 * 
 * @Returns   	void
 * 
 * @author 		xuyz 
 * 
 * @history 	created by xuyz
 */
void IF_TerminalProfile(void)
{
#ifdef LDFile
	UWORD idata  fileAddr;
#endif
	
	if((P3 + 1) > sizeof(G_TerminalProfile))
	{
		G_TerminalProfile[0] = sizeof(G_TerminalProfile) - 1;
	}
	else
	{
		G_TerminalProfile[0] = P3;	//first byte is length of  terminalprofile
	}

	memcpy((BYTE *)&G_TerminalProfile[1],(BYTE *)&RecData[0],G_TerminalProfile[0]);	

#ifdef LDFile	
	fileAddr = SelectTFFile(DF_OTA,EF_ScriptChainingFile);
	
	if(fileAddr != FALSE)
	{	
		ReadTFBuf(fileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
		
		if((ChainingValue.firstScriptValue == 0x01)||((ChainingValue.firstScriptValue == 0x11)&&(ChainingValue.previousValue == 0x03)))
		{
			SetTFConst(fileAddr ,0x00,0x00,sizeof(ChainingValue));  
		}

		ExtendedRFMUpdateResponseTLV(TAG_ClearResponseFile);
	}
#endif



}

/**
 * @brief 		EnvelopeCommand指令接口函数	
 * 
 * @Returns   	WORD型，0xFF01表示不继续处理后续指令/其他值表示低字节有效，继续处理后续指令
 * 
 * @author 		XWH
 * 
 * @history 	created by XWH
 */
WORD IF_EnvelopeCommand(void)
{
	BYTE idata result;
	
	result = 0x00;	

	switch(RecData[0])
	{
		case 0xD1:
			PP_Download_Hanlder();			 
			break;
		case 0xD3:
			MenuSelection();
			break;	
		case 0xD6:
			EventDownload();
			break;
#ifdef CATTP		
		case 0xD7:						// #ifdef CATTP
			TimerExpiration();
			break;	
#endif		
		default:
			break;
	}

	return result;
}

/**
 * @brief 		EnvelopeCommandSTK指令接口函数	
 * 
 * @Returns   	TRUE:不再执行后续指令 /FALSE：继续执行后续命令
 * 
 * @author 		XWH
 * 
 * @history 	created by XWH
 */
BYTE IF_EnvelopeCommandSTK(void)
{
	BYTE idata result;

	result = FALSE;

	if(RecData[0] == 0xD1)
	{
		PP_Download_Hanlder();
	}
	else if(RecData[0] == 0xD6)
	{
		EventDownload();
	}
#ifdef CATTP	
	else if(RecData[0] == 0xD7)
	{
		TimerExpiration();
	}
#endif	
	else
	{
		SimAppTkitBusy();
	}

	return result;
}


/**
 * @brief 		GetDfInformation()的接口函数
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
void IF_GetDfInformation(void)
{

}

/**
 * @brief 		StatusCommand指令接口函数	
 * 
 * @Returns   	VOID
 * 
 * @author 		XWH
 * 
 * @history 	created by XWH
 */
 
void IF_StatusCommand(void)
{
	return;
}

/**
 * @brief 		STKStatusCommand指令接口函数	
 * 
 * @Returns   	VOID
 * 
 * @author 		XWH
 * 
 * @history 	created by XWH
 */
 
void IF_STKStatusCommand(void)
{
	return;
}



/**
 * @brief 		MAIN函数中，接收完5字节APDU头后的接口函数
 *
 * @RETURN		TRUE：不继续执行接口处的后续程序，重新接收APDU
 				FALSE:继续执行接口处的后续程序，继续执行该APDU
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @100127 20:01
 */
BYTE IF_MainReceiveApdu(void)
{	
#ifdef LDFile
	UWORD xdata fileAddr;
	BYTE idata num;
	
	if (INS != 0xC0)		//说明放弃级联读	
	{
		while (G_LDFileDeliverMuli != FALSE)		//上行级联并使用deliver时，上行批次管理
		{
			fileAddr = SelectTFFile(DF_OTA, LDFile_RFMPerso);
			
			if (fileAddr == FALSE)
			{
				break;
			}
			
			ReadTFBuf(fileAddr, 0x00, (BYTE *)&num, 0x01);
			
			num++;
			
			WriteTFBuf(fileAddr, 0x00, (BYTE *)&num, 0x01);
			
			break;
		}
		
		G_LDFileDeliver = FALSE;
		
		G_LDFileDeliverMuli = FALSE;
	}
#endif
	return FALSE;
}

/**
 * @brief 		ReceiveData中接收数据之前的接口函数
 *
 * @Returns		TRUE:继续执行ReceiveData()中的接收数据流程
 				FALSE:禁止执行ReceiveData()中的接收数据流程
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @081220 20:01
 */
BYTE IF_BeforeReceiveData(void)
{
#ifdef LDFile
	if(	G_CompactExtendedTag != 0x00)  //当前处于RFM中操作，不接收数据
	{
		return FALSE;
	}
#endif	
	
	return TRUE;
}


/*
   中国移动M2M专网写号功能使用
*/
void IF_STKTerminalResponse(void)
{
 	return;
}

/*
   
*/
void IF_ResumeResponseMode(void)
{
 	return;
}

/*
   中国移动M2M专网写号功能使用
*/
BYTE IF_LiveCardBusy(void)
{
 	return FALSE;
}



/*
*@description:	AccessCondition函数的接口函数,中国电信平台使用
*
*@param	:fileAddr :文件地址
*@param :action: A_CreateDF/A_DeleteFile/A_ReadFile/A_UpdateFile等
*
*@return: 0xFF:继续执行后续命令
*		   TRUE/FALSE:当前权限验证通过/or不通过
*
*@author:  xuyz
*
*@history: V1.0：create @20150422
*/
BYTE IF_AccessCondition(UWORD fileAddr,BYTE action)
{
#ifdef LDFile
    BYTE idata localResult,rfmResult;
	WORD idata spaceSize;
	WORD  idata binaryFileLgth;


	if((ValueBit(G_SystemStatus,B_RfmPerform) == TRUE) &&(G_CompactExtendedTag != 0x00))	//TS102226规范
	{
		 if(INS == 0xB2)
		 {
			if(P3 == 0x00)  //根据ts102226,P3 为0时，可以读取整条记录
			{
				P3 = GetLFRecNumLgth(fileAddr,FileLgth);
			}
		}	
		else if(INS == 0xB0)
		{
			if(P3 == 0x00)
			{
				spaceSize = GetTFSize(fileAddr);

				binaryFileLgth = ((P1<<8)&0xFF00)|(P2&0x00FF);

				if(spaceSize <( binaryFileLgth+0x100))  // 当剩余有效数据不足0x100时，返回剩余有效数据	;否则返回0x100字节
				{
					P3 = spaceSize - binaryFileLgth; 
				}
			}
		}

		rfmResult = RFMAccessCondition(fileAddr,action);		 //此处改动为新备案的234G卡需求		

		if(G_RFMFileIn2F08 == TRUE)								//表示该文件已有在2F08中配置,按照2F08的要求返回
		{
			return 	rfmResult;
		}
		else													//该文件没有在2F08中配置,则要参考3F00/2F06的权限设置
		{
			localResult = GetAcessPurview(fileAddr,action);	

			if((localResult==FALSE)||(localResult==0x97))
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		
		}

		//return RFMAccessCondition(fileAddr,action);

	}
#endif	
	
	return 0xFF;
}









/*
   中国电信RFM功能使用
*/
BYTE IF_LDFileDeliver(void)
{
#ifdef LDFile	
	if(G_LDFileDeliver == TRUE)	 //#ifdef LDFile
	{
		return TRUE;
	}
#endif
   return FALSE;				
}


/**
 * @brief 		设定指定项目标识符
 *
 * @author 		XuQingsheng
 * 
 * @history 	created by XuQingsheng, @071220 20:01
 */
void SetSpcPrjItem(void)
{
	if(G_NextPageTag == TRUE)
	{
		RecData[2] = G_NextMenuIndex;  //下一页时，项目标识符从1开始，暂时做此处理

		G_NextMenuIndex ++;
	}
	else
	{
		RecData[2] = G_MenuIndex;
	}
}



/**
 * @brief 		处理SetupMenu第二页
 *
 * @Returns		BackMove/ExitStkPerform:退出或者上一页
 *				    其他值：选中菜单在6F18中的记录号
 *
 * @author 		xuqs 
 * 
 * @history 	created by xuqs, @080703 10:01
 */
BYTE SetupMenuNextPage(void)
{
/*	
	BYTE idata i, j,total;
	BYTE idata result;
	UWORD idata fileAddr;
	
	G_NextPageTag = TRUE ;

	G_MenuIndex = G_BackMenuIndex;

	G_NextMenuIndex = 1;

	G_SMItemLen = 0x00;

	InitProactiveCommand(CMD_SELECTITEM,0x00,DEV_ME);

	fileAddr = SelectLFFile(DF_STK,EF_Menu);


	total = GetItemIDinEFMenu(fileAddr, tempBuf);

	AddTLV(&ResData[1], (BYTE *) &prePage[0]);	

	for (i = G_SMLastMenuID; i < 0xFC; i++)
	{
		for (j = 1; j <= total; j++)
		{
			if (tempBuf[j] == i)
			{
				AddMenuItem(fileAddr, j);
			}  

		}
	}
	result = SendProactiveCommand();

	if (result != CMD_Success)
	{
		return result;
	}

	result = GetResponseItemIdentifier();

	if (result == D_PrePageItem)
	{
		return ExitStkPerform;
	}

	return result;	
	*/
}




/**
 * @brief 		添加菜单项目
 * 
 * @param 		fileAddr: EF_6F18 
 * @param 		index: Item Identifier 
 * 
 * @Returns		TRUE:继续组装SetupMenu的Item项
 *				    FALSE:组装完毕
 *
 * @author 		
 * 
 * @history 	
 */
BYTE AddMenuItem(UWORD fileAddr, BYTE index)
{	
/*	
	ReadLFBuf(fileAddr,index,0x00,&RecData[1],GetLFRecNumLgth(fileAddr,FileLgth));

	if((RecData[1] != 0x04) ||((RecData[5]&0x80) != 0x80)||(RecData[6] == 0x00))  //?????00?,???
	{
		return TRUE;
	}


    RecData[0] = 0x8F;

	RecData[1] = 0x01;
	
	SetSpcPrjItem();

	SetLFConst(fileAddr,index,0x01, G_MenuIndex,sizeof(BYTE));

	G_MenuIndex ++; 

	AddVlaue((BYTE *)&RecData[1], (BYTE *)&RecData[6]);
	
	if ((G_SMItemLen + RecData[1]) <= 0xB9)
	{
		G_SMItemLen += RecData[1];	
	} 
	else
	{	
		return FALSE;
	}

	AddTLV((BYTE *)&ResData[1], (BYTE  *)&RecData[0]);

	return TRUE;
	*/
}




/**
 * @brief 	扫描EF_6F18,将所有的ITEM_ID读取到输出中
 *
 * @param       fileAddr: EF_6F18
 * @param       output: array, id1 id2 id3
 * 	
 */
BYTE GetItemIDinEFMenu(UWORD fileAddr, BYTE * output)
{
/*	
    BYTE idata i, total;

    total = GetLFRecNumLgth(fileAddr, FileNum);

    for (i=0x01; i<=total; i++)
    {
        ReadLFBuf(fileAddr, i, 0x02, &output[i], sizeof(BYTE));
    }

    return i; // ITEM ID?? 
*/	
}


/*
*@description:	更新敏感数据区的数据
*
*@param	:BYTE *buffer :  要更新的数据buffer
*@param :WORD off  ：要更新的偏移
*@param :WORD len  ：要更新的长度
*@param : mode：0：更新KI   1：更新鉴权参数
*
*@return: BYTE  :TRUE/FALSE	  更新成功或者失败
*
*@author:  zhxx
*
*@history: V1.0：create by zhxx @20200330
*/
/*
BYTE IF_UpdateSecureData(BYTE *buffer,WORD off,WORD len,BYTE mode)
{
   	if(mode == 0) //更新KI
	{
	   if(len != KI_Len)
	   {
	      return FALSE;
	   }

	   return WriteEepromEX(E_SECURE_DATA + KI_OFF,buffer,KI_Len);
	}
	else if(mode == 1) //更新鉴权参数
	{
       if(off + len > AUTH_CFG_Len)
	   {
	      return FALSE;
	   }
	   return WriteEepromEX(E_SECURE_DATA + AUTH_CFG_OFF + off,buffer,len);
	}

	return FALSE;
}
*/

/*
 *@description:SecureDataStore 根据EFName判断当前文件是否是敏感文件，如果是，返回该文件在敏感数据区的地址
 *
 *@param:      WORD efName -- EF Id
  *
 *@Returns     敏感数据区的地址/FALSE
 *
 *@history     create by zhxx at 20200401
*/
UWORD SecureDataStore(WORD efName,WORD* index)
{
#ifdef SENSITIVEDATA																	//不支持多IMSI功能时才使用此值
   index[0] =  FindFreeIndexFromAddrIndexTable();

	 if((G_CurrentDfName[G_ChannelNum] == DF_MF) && (efName == EF_ICCID))  //3F00/2FE2
	 {
	      return E_SECURE_DATA + ICCID_OFF;
	 }
	 else if((G_CurrentDfName[G_ChannelNum] == DF_PHASE) && (efName == EF_IMSI))
	 {
		 return E_SECURE_DATA + IMSI1_OFF;
	 }
	 else if((G_CurrentDfName[G_ChannelNum] == DF_ADF) && (efName == EF_IMSI))
	 { 
		 return E_SECURE_DATA + IMSI2_OFF;
	 }
	 else if((G_CurrentDfName[G_ChannelNum] == DF_MF) && (efName == EF_AuthCFG))   //3F00/2E00
	 {
		 return E_SECURE_DATA + AUTH_CFG_OFF;
	 }
	 else if(((G_CurrentDfName[G_ChannelNum] == DF_PHASE) && (efName == EF_KI)) || ((G_CurrentDfName[G_ChannelNum] == DF_MF) && (efName == EF_Single4G_KI)))  //7F20/6F1B || 3F00/2E1B
	 {     
		 return E_SECURE_DATA + KI_OFF;
	 }
	 else
	 {
	     return FALSE;
	 }	 
#else
	 return FALSE;
#endif	 
	 
}




/*
 *	@brief  IF_MoveFlashResume,该接口可配置是否调用Del,Resize断电恢复功能
 *	@author yangjian

*/
void IF_MoveFlashResume(void)
{
	if(G_CardStatus == D_CardInit)
	{

#ifdef DELRESIZEFILE
		 MoveHugeEepromRunning();
#endif
	}

}

/*
 *	@brief  IF_CreateDFFile,该接口可配置调用哪个创建DF函数(要看是否支持DelFile,发卡后创建等功能)
 *	@author yangjian
 *  @param	:UWORD fileUseSize: 创建文件大小  
 *  @param :WORD fileName:  创建文件ID

*/
UWORD IF_CreateDFFile(UWORD fileUseSize,WORD fileName)
{
	UWORD xdata  fileMaxSize;

#ifdef SECURE_CREATE	
	fileMaxSize = CreateDFFileSECURE(fileUseSize,fileName);	
#else 
	fileMaxSize = CreateDFFile(fileUseSize,fileName);
#endif

	return 	fileMaxSize;


}


/*
 *	@brief  IF_CreateEFFile,该接口可配置调用哪个创建EF函数(要看是否支持DelFile,发卡后创建等功能)
 *	@author yangjian
 *  @param	:UWORD fileUseSize: 创建文件大小  
 *  @param :WORD fileName:  创建文件ID

*/
UWORD IF_CreateEFFile(UWORD fileUseSize,WORD fileName)
{
	UWORD xdata  fileMaxSize;

#ifdef SECURE_CREATE	
	fileMaxSize = CreateEFFileSECURE(fileUseSize,fileName);	
#else 
	fileMaxSize = CreateEFFile(fileUseSize,fileName);
#endif

	return 	fileMaxSize;	

}

/*
  *@brief  IF_CreateFileAccessCondition,该接口支持配置特殊的创建文件权限,默认返回FALSE(表示不需要特殊配置)
  *@author yangjian
  *  @param	:UWORD fileAddr: 文件头地址 
  *  @param :BYTE operAction: 文件操作类型

   *@return:  FALSE: 表示正常流程(默认)
 			  TRUE:  根据需求特别处理

*/
BYTE IF_CreateFileAccessCondition(UWORD fileAddr,BYTE operAction)
{	
#ifdef LDFile															 
	BYTE idata 	fileType;

	//if((operAction == A_CreateDF)||(operAction == A_CreateEF)) 
	if(((operAction == A_CreateDF)||(operAction == A_CreateEF)) && (ValueBit(G_SystemStatus,B_RfmPerform) == TRUE))  
	{
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if( fileType == ADFTYPE )
		{
			return TRUE;
		}
	}
#endif
	return FALSE;
	
}




/*
 *@brief  IF_VMHeapAlloc,创建文件时存储空间分配接口函数
 *@author yangjian
 *@Param:   size  :分配的存储空间的大小
 *@param    index :分配的有效的地址引用

 *@return:  实际分配文件体大小
*/
UWORD IF_VMHeapAlloc(UWORD size)
{
	UWORD xdata assignSize;

#ifdef SECURE_CREATE
	assignSize = size + Heap_ConfigBytes_Len;// 实际分配的大小
#else
	assignSize = size;
#endif

	return assignSize;	
}



/*
  *@brief  IF_CmdINS,根据INS的值,执行特定指令的处理

 *@author  yangjian
 *@Param:   insByte  :指令INS的值
 

 *@return:  FALSE: 表示正常指令流程(默认)
 			TRUE:  根据需求执行特别流程

*/
BYTE IF_CmdINS(BYTE insByte)
{	
#ifdef CSIM
   	if((insByte == 0x84)&&(G_CommandMode == Command_Mode_2G))						  //CDMA中也有0x84指令，因此用CLA来区分
	{
		return TRUE;				  
	}
#endif	

	return FALSE;
}


/*
  *@brief  IF_CommandAttemper,可根据需求配置是否拒绝非激活的操作模式	

 *@author  yangjian
 
 *@return:  

*/
BYTE IF_CommandAttemper(void)
{
#ifdef CSIM
	if(INS != 0x80)
	{
		if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xFC) != 0x00)&&(CLA != 0xA0)))		//拒绝非激活的操作模式							
		{
			ClaError();

			SendResponse();

			return FALSE;
		}
	}
	else
	{
		if((CLA&0xFC) != 0x80)
		{
			ClaError();
	
			SendResponse();
	
			return FALSE;
		}

	}

	return TRUE;

#else
	if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xFC) != 0x00)&&(CLA != 0xA0)))		//拒绝非激活的操作模式							
	{
		ClaError();

		SendResponse();

		return FALSE;
	}

	return TRUE;
#endif
}






/*
 *@brief  IF_CDMAGSM,可通过接口配置中国电信CSIM卡功能(不需要该功能时直接返回)

 *@author  yangjian

*/
void  IF_CDMAGSM(void)
{
	if((G_CurrentDfName[G_ChannelNum] == DF_PHASE)&&(G_CurrentEfName[G_ChannelNum] == 0x6FAE))
	{
		G_NetWorkMode = GSM_NETWORK;	//COS默认为C网，如果终端读取6FAE文件，则转换为G网
	}
	else if((G_CurrentDfName[G_ChannelNum] == DF_CDMA)&&(G_CurrentEfName[G_ChannelNum] == 0x6F39))
	{
		G_NetWorkMode = CDMA_NETWORK;	//COS默认为C网，如果终端读取6F39文件，则转换为C网
	}	

}

/*
 *@brief  IF_VerifyChv,校验PIN指令接口,	可通过该接口配置中国电信CSIM卡特殊需求

 *@author  yangjian

*/
BYTE IF_VerifyChv(void)
{
#ifdef CSIM						
		if(((P2==0x00)||(P2>0x06))) 																//CHV||ADM
		{
			P1P2Error();	                                            //6B 00
			return FALSE;
		}		
				
		if(P2 == 0x03)
		{
			P1P2Error();	                                            //6B 00
			return FALSE;
		}
		else if(P2>0x03)
		{
			P2 = P2 + 0x06;
		}		
		else													
		{
			if(P2 == 0x02)
			{
				P2 = 0x81;										
			}
		}

		return TRUE;
#else			
		if((P2==0x00)||(P2>0x05))
		{
			P1P2Error();	                                            //6B 00
			return FALSE;
		}

		return TRUE;
			
#endif

}

/*
 *@brief  IF_VerifyChv2,校验PIN指令接口,可通过该接口配置中国电信CSIM卡特殊需求

 *@author  yangjian

*/
void IF_VerifyChv2(BYTE mode)
{
#ifndef CSIM
	if((mode == mode_Verify) && (P2 > 0x02))
	{
		P2 = P2 + 0x07;
	}
#endif

	if((mode == mode_Unblock) && (P2 == 0x00))
	{
		P2 = 0x01;
	}

	if(P2 == 0x02)
	{
		P2 = 0x81;										//PIN2
	}
}



/*
 *@brief  IF_IncreaseCommand,Increase指令接口,可通过该接口配置中国电信CSIM卡特殊需求

 *@author  yangjian

*/
BYTE	IF_IncreaseCommand(void)
{
#ifdef CSIM
	if(((G_CommandMode == Command_Mode_2G)&&(P3 != 0x03)&&(P3 != 0x02))||((G_CommandMode == Command_Mode_3G)&&((P3 == 0x00)||(P3>0x7F))))
  	{
		P3Error();                                                 
		return FALSE;
	} 
#else		 
	if(P3 != 0x03)
	{
		P3Error();				 
		return FALSE;	
	}
#endif

	return TRUE;

}


/*
 *@brief  IF_UpdateRecordPRL,PRL指令接口,可通过该接口配置中国电信CSIM卡的PRL功能需求,不需要PRL功能时默认直接返回

 *@author  yangjian	


*/

void IF_UpdateRecordPRL(BYTE recNo)
{
#ifdef PRL
	if((RecData[0] == 0x03)&&(G_CurrentEfName[G_ChannelNum] == EF_SMS)&&((G_CurrentDfName[G_ChannelNum] == G_DF_ID_CSIM)||(G_CurrentDfName[G_ChannelNum] == DF_CDMA)))//ifdef PRL
	{
		UpdatePLMNList(&RecData[0],recNo);		
	}
#endif


}

/*
 *@brief  CATTP初始化,PRL断电恢复/Timer初始化

 *@author  LCZ  20201221



*/
void IF_ResumePrlData(void)
{
#ifdef CATTP
	ResumePrlData();
#endif 	
}












