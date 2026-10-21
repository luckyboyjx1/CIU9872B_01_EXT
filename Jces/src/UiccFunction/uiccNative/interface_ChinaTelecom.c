/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.22

 * MODEL: 	All model

 * RELEASE: 2020-3-2

 * AUTHER:	杨健

 * HISTROY:	V1.0  创建源文件,用于中国电信平台
			V1.1  LDFileSMSSubmit函数中,改为只有C网上行才支持CDMA转换(宏CDMASMS隔开)	
			V1.2  增加支持C网上下行短信的处理,用宏CDMASMS区分不同的PP_Download_Hanlder函数		

         
					
*/

#include  "typedef.h"
#include  "uicc_main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include "uicc_error.h"
#include  "arithmatic.h"
#include  "Authentication.h"
#include  "ceph.h"
#include  "interface.h"
//#include "Transaction.h"
#include "TimerModuleProces.h"
#include "BIPModuleProcess.h"
#ifdef High_Update_FOR_Test
#include  "PrivateCommandTest.h"
#endif
#ifdef CDMA
#include  "cdma.h"
#endif

#ifdef CATTP
#include  "CatTp.h"
#endif
#ifdef MULTI_IMSI
#include "MultiImsi.h"
#endif
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#include "Interface_AnnexB.h"
#endif

#include  "md5.h"
#include  "ToolkitRFM.h"
#if COS_UPDATE_ONLINE_BY_NativeOS
#include  "FunEntryConfig.h"
#include  "UpCos.h"
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

#ifdef MULTI_IMSI
BYTE  xdata G_TerminalData[0x1C];
BYTE  xdata G_StatusEventList;
BYTE  xdata G_Cur_IMSI_RecNo;    
#endif


 

BYTE xdata VARIABLEBUFFER[0x80];

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
/*void IF_EepromAddressSetting(void)
{
	//G_CODE_SIZE = 0x20000;						//CODE区大小，此处根据需求调整
	 G_CODE_SIZE = 0x16000;						//CODE区大小，此处根据需求调整
		
#ifdef High_Update_Function
	G_High_Update_NUM = 0x00;	
#else
	G_High_Update_NUM = 0x00;
#endif
}*/

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
	
//	if(G_A4Control == FALSE)
//	{
//		responseLgth =  SetChannel(fileAddr,fileName,TRUE);
//	}
//	else
//	{
//		responseLgth = G_A4Control;
//	}
//			
//	if((P3 == 0x00)||(P3 >responseLgth))
//	{	
//		RepeatPreviousCmdHeader(responseLgth);
//	}
//	else
//	{
//		SendData();
//				
//		if(G_A4Control == FALSE)
//		{
//			CommandFinish(0x00);
//		}
//		else
//		{
//			memcpy(&RecData[0],&RecData[P3],responseLgth-P3);
//				
//			CommandFinish(responseLgth- P3);
//		}
//	}
//			
//	return TRUE;
	
	//变量G_A4Control不再使用，这里不再根据G_A4Control的值做不同处理,by xwh 20220330
	responseLgth =  SetChannel(fileAddr,fileName,TRUE);
	
	apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, responseLgth);
				
	GlobalSetStatus(0x9000);
	
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

	G_SMItemLen = 0x00;
	
	G_AppControl = 0x40;				//bit7置1

	G_OpenControl = 0x00;
	
	STKControl = TRUE;

	G_STKBusy = 0x00; 
	
	memset(G_TerminalProfile, 0x00,sizeof(G_TerminalProfile));
	
	InitUpdateVariable();

	G_DF_ID_USIM = 0x7FF0;


	G_DF_ID_CSIM = 0x7FF1;	    


  G_SetUpEventListSent = FALSE;                        

#if GP__AMENDMENT__B__FOR_MASK		
	InitHttpVariable();
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
		
//	G_BipPrlStatus = 0x00;

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

#ifdef MULTI_IMSI
    G_StatusEventList = 0x01;
		
		G_Cur_IMSI_RecNo = 0x01;                              //使用默认数据0x01
#endif

}


/**
 * @brief 		main()中的系统变量和参数初始化的接口
 *
 * @author 		zdm
 *				开机过程中，读完PIN状态后的接口文件。目前是开机后的第一条指令中处理

 * @history 	created by zdm, @20160817
 */
//void  IF_AfterReadPin(void)
//{
//	BYTE  xdata tem[2];	
//	WORD idata offset;
//			 	
//	offset = FindCOSConfigTag(Tag_USIMAlgorithm_Type,&tem[0]);

//	if(offset == OFFSET_NULL)	  		//没找到相关配置
//	{
//		G_USIMAlgorithmType = AlgorithmType_MilenageNORMAL;			
//	}
//	else
//	{
//		G_USIMAlgorithmType = tem[0];				
//	}	
//	
//#ifdef MULTI_IMSI
//	ProcessImsiFplmnScychronze();
//#endif	
//	
//}


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
		
		case 0xE1:									//添加中国电信清除counter私有指令
			LDCleanCounter();
			break;
		
#if NATIVE_CARD_UICC
#ifdef DELRESIZEFILE
		case 0xE4:
		   DeleteFile();
		   break;
		case 0xD4:
		   ResizeFile();
		   break;
#endif
	#endif
	#if NATIVE_CARD_UICC
		case 0xDD:		
			CalCRC();
			break;
		#endif
		
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



#ifdef CDMASMS
/**
 * @brief 		用于C网下行格式短信的PP_Download_Hanlder
 *
 * @author 		
 *
 * @history 	
 */
BYTE PP_Download_Hanlder(void)
{
	BYTE idata ptr;
	BYTE idata result;
	BYTE idata udPos;
	BYTE idata rfmStatus;			   //表示RFM短信接收状况	
	BYTE idata midPtr;
                
	udPos = GetCDMAUDPos();					//C网短信格式转换

	if(udPos != FALSE)			//ponit to UDL				
	{
		ptr = udPos + 1;       //have the UDHL

		result = FindTag(&RecData[ptr+1], 0x70, RecData[ptr]); //027000

		if(result != FALSE)
		{
#ifdef	LDFile	
			midPtr = ptr + RecData[ptr] + 3 + 5;		//point to the TAR
						
				if(IsRFMTar(&RecData[midPtr]) == TRUE)  //当前为RFMTAR
				{
					if(RecData[udPos+1] == 0x02)
					{
						rfmStatus = StoreDataSms(&RecData[udPos],0xFF,1,1,LDFileSingleProgess);
					}
					else
					{
						rfmStatus = StoreDataSms(&RecData[udPos],RecData[udPos+4],RecData[udPos+6],RecData[udPos+5],LDFileMuliProgess);	
					}
		
					if(rfmStatus == 0xFF)					//表示RFM短信已收满
					{
						G_RFMAllReceived = TRUE;
					}
					
					return TRUE;
				}
#endif


#ifdef CATTP
		   	if(((RecData[midPtr] == 0xB2) && (RecData[midPtr+1] == 0x01) && (RecData[midPtr+2] == 0x00))
			   ||((RecData[midPtr] == 0xBF) && (RecData[midPtr+1] == 0x01) && (RecData[midPtr+2] == 0xFF))
			   ||((RecData[midPtr] == 0xBF) && (RecData[midPtr+1] == 0x02) && (RecData[midPtr+2] == 0xFF))
			   ||((RecData[midPtr] == 0xBF) && (RecData[midPtr+1] == 0x03) && (RecData[midPtr+2] == 0xFF)))
			   {
					 
					 StoreDataSms(&RecData[udPos],0xFE,1,1,D_RFMSucessTag);
			   		return TRUE;
			   }
#endif
		
		}
		else
		{
#ifdef	LDFile
				rfmStatus = StoreDataSms(&RecData[udPos],RecData[udPos+4],RecData[udPos+6],RecData[udPos+5],LDFileMuliProgess);
								
				if(rfmStatus == 0xFF)					//表示RFM短信已收满
				{
					G_RFMAllReceived = TRUE;
				}
					
				
				return TRUE;
#endif
		}	
		
	}
	
	return TRUE;
}



#else
/**
 * @brief 		处理数据短信的UDHI字节(各省服务器的定义不一致)，以确定该短信是否含03.48头
 * 
 * @param 		BYTE udhi :UDHI字节
 *
 * @return		FALSE:不含03.48头 / TRUE:含03.48头
 * 
 * @author 		WG 
 * 
 * @history 	created  by xuqs, @080215
 */
BYTE Check_UDHI(BYTE udhi)
{
	//if ((udhi & 0x60) != 0x00)
	if ((udhi & 0x40) != 0x00)
	{
		return TRUE;
	}    
 	else
 	{
 		return FALSE;
 	}
}
void PP_Download_Hanlder(void)
{
	BYTE idata ptr;
	BYTE idata result;
	BYTE idata udPos,pos00;
	BYTE idata smscPtr, deviceIDPtr, tpduPtr;
	WORD idata offset;

#if UICC__APP__LIVECARD
	BYTE idata udhl, chl, buf[1];
	UWORD xdata fileAddr;
#endif

	BYTE xdata tpoa[0x10];  //GSM03.40中9.2.2.1中要求TP-OA的长度为2-12BYTE，该变量保存为LV结构，数据域为TPOA字符个数+TPOA类型+TPOA
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)	
#if UICC__CDMA__SMS__DOWNLOAD
	if(CdmaTransGsm())
	{
		return;
	}
#endif  
#endif
	if(RecData[1] == 0x81)
	{
	   if((RecData[2]+3) != P3 )	//GSM11.14  7.1.2 Structure of ENVELOPE   测试用例MultiChannelTest_9_1
	   {
	      return;   
	   } 	  
	}
	else
	{
	   if((RecData[1]+2) != P3 )	//GSM11.14  7.1.2 Structure of ENVELOPE   测试用例CodeReview_45_1
	   {
	      return;   
	   }	   
	}
	ptr = FindTag(&RecData[0],0xD1,P3);	

	deviceIDPtr = 0;

	for(offset = ptr+1; offset < RecData[ptr]+ptr+1;)  //判断后续所有TLV的长度是否等于一个完整APDU的长度
	{
		smscPtr = 0;

		if(RecData[offset+1] == 0x81)	 //point to len,仅支持0x81	
		{
			smscPtr = 1;
		}

		deviceIDPtr += RecData[offset+1+smscPtr]+2+smscPtr;	

		offset += RecData[offset+1+smscPtr]+2+smscPtr;
	}

	if(deviceIDPtr != RecData[ptr])
	{
		return;
	}
	
	deviceIDPtr = FindTag(&RecData[ptr+1], TLV_DEVICEIDENTITY, RecData[ptr]); //GSM11.14  7.1.2 Structure of ENVELOPE、12.7 Device identities

	if(FALSE == deviceIDPtr)
	{
	   return;
	}

	deviceIDPtr += (ptr + 1);

	if(RecData[deviceIDPtr] != 0x02)
	{
	   return;
	}

	smscPtr = FindTag(&RecData[ptr+1], TLV_ADDRESS, RecData[ptr]);   //todo：此处判断当前短信内容是不是指定的服务器下发，若不是，则不处理该短信  

	if(FALSE != smscPtr) //短信中心地址是O
	{
	   	smscPtr = smscPtr + ptr + 1;	                                   //offset of SMSC Length 
	} 
// 	else  //将该代码注释掉的原因是：移动NFC2.0送检工程集20151113\SIM_API测试用例_20150204_明文版\api_1_ssy_gets用例中，没有短信数据中心也要求可以测过。
// 	{
// 		return; //中国移动NFC2.0项目20160415/GP2.2.1_UICC_ComplianceTests(移动版)-20141112/uicc_configuration_additional_test/CodeReview_SECURED/CodeReview_45_1的步骤9，指令中将短信中心TAG写成了66，期望返回9000
// 	}
	
	tpduPtr = FindTag(&RecData[ptr+1],TLV_SMSTPDU,RecData[ptr]);

	if(tpduPtr == FALSE)
	{
		return;
	}

	ptr =  tpduPtr + ptr; 
	
	result = 4 + (RecData[ptr+3]/ 2) + (RecData[ptr+3]&0x01);				//Len of OA

	tpoa[0] = (RecData[ptr+3]/ 2) + (RecData[ptr+3]&0x01) + 2;  //len of OA +TPOA类型 +TPOA字符个数 = 总的字节数,如TPOA为 0C81106540001000
	
	if(tpoa[0] > (sizeof(tpoa)-1))//判断数组越界，避免tpoa[0]>0x0F时执行memcpy(&tpoa[1], &RecData[ptr+3],tpoa[0]);的异常
	{
		return;
	}
	
	memcpy(&tpoa[1], &RecData[ptr+3],tpoa[0]);   //暂时缓存下行短信的TPOA,包括 TPOA类型，比较的时候不需要比较该字节, 则存储进的数据为 08 0C 81 106540001000,81字节不需要比较

	udPos = ptr + result + 10; 											// OA len, TPI/NPI, PI, DC, Service Stamp

	if(RecData[ptr + 1] != (3 + (RecData[ptr+3]/ 2) + (RecData[ptr+3]&0x01) + 10 + RecData[udPos]))	 //判断UDL是否正确，RecData[udPos]指向UDL位置,测试用例CodeReview_45_1
	{
	    return;
	}

	if((RecData[ptr+2]&0x03) != 0x00)
	{
		return;
	}

	if(Check_UDHI(RecData[ptr+2])!=FALSE)
	{
#if UICC__APP__LIVECARD
		if((RecData[ptr+3]%2) == 0x00)
		{
			 buf[0] = RecData[ptr+3]/2+2;
		}
		else
		{
			  buf[0] = (RecData[ptr+3]+1)/2+2;
		}

		fileAddr = SelectLFFile(0x2700,0x6F03);

		WriteLFBuf(fileAddr,1,0x00,&buf[0],1);  //

		WriteLFBuf(fileAddr,1,0x01,&RecData[ptr+3],buf[0]);

#endif

 		ptr = udPos + 1; 

#if UICC__APP__LIVECARD
        if(RecData[udPos]!=0x00)
		{
		    udhl = RecData[ptr];   //
		}
		else
		{
		   return;
		}

#endif

#if UICC__APP__WHITECARD
		result = FindTagFullMatch(&RecData[ptr+1], 0x0A, RecData[ptr]); 	
		if(result != FALSE)   //白卡入网应用的单条短信,只能是0x0a，不能是0x8a
		{
			ToolkitSaveFirstFomatData(udPos,EVENT_FORMATTED_SMS_PP_ENV_bit,tpoa);  //udpos指向UDL	
			return;
		}
#endif
        result = FindTagFullMatch(&RecData[ptr+1], 0x70, RecData[ptr]); 	
			        
		if(result != FALSE)
		{ 
		    //00/70之外可能还有其他 UDH element，这里不判断长度，提高兼容性,
//			if((RecData[ptr] != 0x02) && (RecData[ptr] != 0x07))	//GSM03.48Table 7和Table 6；IEILA字段错误
//			{
//			   return;
//			}
			 
            ptr = ptr + RecData[ptr] + 3;									//skip CPL	,point to the CHL

#if UICC__APP__LIVECARD
			chl = RecData[ptr];
#endif

			ptr = ptr + 5;

			if((RecData[ptr] == 0xB0) && (RecData[ptr+1] == 0x00))	//check TAR 2/3
			{

#if UICC__APP__LIVECARD
				if(RecData[ptr+2] == OTA_GetCardInfo_Tag)
				{
					if((chl != 0x0D) || (udhl != 0x02))//
					{
						return;
					}
				}
				else if(RecData[ptr+2] == OTA_LiveWriteCard_Tag)
				{
					if((udhl != 0x07 )
					 || (chl != 0x11)
					 || (memcmp(&RecData[ptr-4], (BYTE *)fixSPIKICKID, sizeof(fixSPIKICKID)) != EQUAL))//
					{
						return;
					}
				}
#endif


				switch(RecData[ptr+2])
				{

#if UICC__APP__LIVECARD
				case OTA_GetCardInfo_Tag:
						LiveGetCardInfoHandler(ptr+9);
						return;
			   case OTA_LiveWriteCard_Tag:

						ToolkitSaveFirstFomatData(udPos,EVENT_FORMATTED_SMS_PP_ENV_bit,tpoa);

						return;
#endif

					default:
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
						ToolkitSaveFirstFomatData(udPos,EVENT_FORMATTED_SMS_PP_ENV_bit,tpoa);
#endif
						return;
				}

			}
			else
			{
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
				ToolkitSaveFirstFomatData(udPos,EVENT_FORMATTED_SMS_PP_ENV_bit,tpoa);
#endif	
				return;
			}
		}
		else
		{
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)			
			result = FindTagFullMatch(&RecData[ptr+1], 0x00, RecData[ptr]); 	 //Concatenated short messages, 8-bit reference number	

			if(result != FALSE)
			{
				ToolkitSaveRFMHead(RecData[ptr+1+result+1],EVENT_UNFORMATTED_SMS_PP_ENV_bit,udPos,TYPE_SMS_ENV_MULTI,tpoa);  //只有对于index为1的时候，才会更新eventId	
				ToolkitSaveRFMSMS(udPos,0x00,TYPE_SMS_ENV_MULTI,tpoa);
				return;				
			}	
#endif			
		}
	}
	
#if UICC__FUNCTION__STKAPI
	ptr = FindTag(&RecData[0],0xD1,P3);

	WriteTFBuf(G_EnvelopHanderAddr, 0x00, &RecData[ptr+1], RecData[ptr]);

	ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = RecData[ptr];

	G_EnvelopeTag = RecData[0];

	JavaToolkitProcess(EVENT_UNFORMATTED_SMS_PP_ENV_bit,0);
#endif
	
}
#endif
//注释该函数，多平台统一，该值使用配置的方式。 jiangqingbin-20220514
//BYTE JcvmConfig_GetAmendmentBTriggerPor(void)
//{
//#if UICC__OPERATOR__CHINATELECOM
//	return 0;
//#endif	
//#if UICC__OPERATOR__CHINAMOBILE
//	return 3;
//#endif	
//}

/*清remote模式的安全会话 ，注意：如果ram里面的指令不是非0通道会有问题  比如：8150xxxxxx 8582xxxxxx*/
void GPClearSecureSessionInfo(/*u8  sdId*/)
{
//	if(G_RAMSessionSdId == ILLEGAL_APPID)
//	{
//		return ;
//	}

//	closeSecuritychannel();
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
	BYTE  idata pos,bak;
	BYTE  idata result;

	result = TRUE;							//置初始值
	
	if(RecData[1] == 0x81)
	{
		pos = 3;
	}
	else
	{
		pos = 2;						//D60B (pos)99010A 82028281 B802 0105
	}	
	
	result = FindTag(&RecData[pos],TLV_EVENTLIST,RecData[pos - 1]);		//#define TLV_EVENTLIST							0x99
	
	if(result == FALSE)		   //point to  event list tag
	{
		return FALSE;
	}
	
	bak = pos;
	
	pos = pos + result + 1;
	
	result = FALSE;                                        		//event offset

	switch(RecData[pos])
	{
 #if ((CATTP) || (GP__AMENDMENT__B__FOR_MASK))
		case 0x09:		

			result = BIPModuleDataAvaiable(RecData[bak-1], &RecData[bak]); 		
			break;

		case 0x0A:	
			
			result = BIPModuleChannelStatus(RecData[bak-1], &RecData[bak]);
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

	AddTLV((BYTE *)&ResData[1], (BYTE  *)(BYTE *)&RecData[0]);	
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
	

	total = GetItemIDinEFMenuTelecom(fileAddr, tempBuf);

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
//	BYTE  xdata num;

//	InitVariable();	

//	num = RecData[1]>0x80?3:2;

//	num = num + RecData[num+1]+2;    								//the number of item identifier

//	if(((RecData[num]&0x7F) == 0x10)&&(RecData[num+1] == 0x01))
//	{
//		;
//	}

	return CMD_Success;
}

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
	switch (pendingTag)
	{
		case TYPE_SMS_ENV_MULTI:
		case TYPE_SMS_ENV_SINGLE:
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)			
			ToolkitRFMExcute(pendingTag,exaInfo);		
			break;
#endif		
#ifdef PRL
		case D_PRLDisplayTag:
			ShowPRLInfo();
			break;
#endif

#if GP__AMENDMENT__B__FOR_MASK			
		case D_SCP81TriggerTag:
			HTTPSessionStart();
			break;
#endif
		
		case BIPModule_Channelstatus_Alert:
#if GP__AMENDMENT__B__FOR_MASK			
			if (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)	//http
			{
				IF_AnnexB_ChannelStatus();
			}
			else	//cattp
#endif
			{
#ifdef CATTP
				 tempBuf[1] = 0x01;  //Location status  TS102 223 p8.25
	
				 tempBuf[2] = 0x03;  
					
				 cephSetUpEvenListSingle(&tempBuf[1]);
				
				 //cephCloseChannelSingle(0x00, G_ChannelStatusID);    //通道连接发生错误，通道关闭
				BIPModuleProcessStatus();	 //通道连接发生错误，通道关闭
							
				if(G_ChannelStatusID == G_CATBIPChannelId)  //当前失去链接的通道正是之前进行CATTP链接的通道
				{
					TimerModuleReaseTimerId(&G_CattpTimerId);
					
					clearCatTPRecord();
				}
#endif
			}

			break;
		case BIPModule_DataAvaiable_Alert: //D_CATTPDataAvaiable:
#if GP__AMENDMENT__B__FOR_MASK	
			if (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS)
			{
				BIPModuleProcessDataAvaiable(exaInfo);	//http
			}
			else
#endif
			{
#ifdef CATTP
				BIPDataAvaiable();						//cattp
#endif
			}
			break;
			
//		case D_RFMSucessTag: //执行安全命令包			//0xa3
//			LDFileProgess(pendingTag);        //公用一套
//			break;
#ifdef CATTP
		case D_CATTPProcessSDU:
			CATTPProcessSDUData();
			CATTPClearFile(LDFile_RelaxFile);
			break;
#endif
		case TimerModule_TimerExpiration_Alert:
			
#if GP__AMENDMENT__B__FOR_MASK			
			IF_AnnexB_RetryHttpSessionByTimerExpiration(exaInfo);
#endif
#ifdef CATTP
			IF_CATTP_RetryCattpsessionByTimerExpiration(exaInfo);
#endif		
			break;
		
#ifdef MULTI_IMSI
			case D_SendRefresh03:		       //发送Refresh 03主动式命令	
				cephRefreshSingle(0x03);
				break;
				
			case D_SendProvideLocalInfo00:	      //发送ProvideLocalInfo 00主动式命令	
				ProcessProvideLocalInfoResponse();
				break;
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
#ifdef MULTI_IMSI
	//Set Up Menu的Terminal response返回后发送SendSetUpEventList
	if((RecData[3] == CMD_SETUPMENU)&&(G_SetUpEventListSent == FALSE)&&(G_StatusEventList==0x01))	
	{
		G_SetUpEventListSent = TRUE;
		SendSetUpEventList();	
	}
#endif
	
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
 	UWORD xdata fileAddr;

#ifdef CATTP	
	BYTE  idata i,recNo;
    BYTE  xdata tmp[4];
#endif	
#ifdef MULTI_IMSI
	if((P3 + 1) > sizeof(G_TerminalData))
	{
		G_TerminalData[0] = sizeof(G_TerminalData) - 1;
	}
	else
	{
		G_TerminalData[0] = P3;                                           	//first byte is length of  terminalprofile
	}

	memcpy((BYTE *)&G_TerminalData[1],(BYTE *)&RecData[0],G_TerminalData[0]);
	
	G_StatusEventList = G_TerminalData[5]&0x01;	
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
	
	
#ifdef CATTP

    fileAddr = SelectPrivateLFFile(DF_Toolkit, EF_STKAlert);          //=============================增加开机把6EA7写为FF  

	if(fileAddr!=FALSE)
	{
		SetTFConst(fileAddr ,0x00,0xFF,GetTFSize(fileAddr));
	}
	
	fileAddr = SelectTFFile(DF_STK, LDFile_DownloadFileMuli);	//=============================增加开机把5E12写为00
	
	if(fileAddr!=FALSE)
	{	
		recNo = GetTFSize(fileAddr); 
	
		for(i=0x01;i<=recNo;i++)
		{
			ReadLFBuf(fileAddr,i,0,&tmp[0],sizeof(tmp));

			if((tmp[0] == 0x00)&&(tmp[1] == 0x00)&&(tmp[2] == 0x00)&&(tmp[3] == 0x00))
	    {
		   continue;
	    }

			SetLFConst(fileAddr,i, 0x00, 0x00, 0x04);
		
		}  
  }  

#endif
	
	SetUpMenu();	
	
#ifdef MULTI_IMSI			
	if((STKPerform == FALSE)&&(G_SetUpEventListSent == FALSE)&&(G_StatusEventList==0x01))
	{
		G_SetUpEventListSent = TRUE;
		SendSetUpEventList();	
	}
#else
	if((STKPerform == FALSE)&&(G_SetUpEventListSent == FALSE))
	{

		G_SetUpEventListSent = TRUE;
		TerProSendSetUpEventList();	
			
	}
#endif	
	G_OpenControl = TRUE;
	//IF_AnnexB_StartTimerAfterRestartHandset();	
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
		case 0xD7:						
			TimerModuleProcessTimerExpiration();
			break;			
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
//#ifdef CATTP	
	else if(RecData[0] == 0xD7)
	{
		TimerModuleProcessTimerExpiration();
	}
//#endif	
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
   中国电信RFM功能使用
*/
BYTE IF_LDFileDeliver(void)
{
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


	total = GetItemIDinEFMenuTelecom(fileAddr, tempBuf);

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
	
	return TRUE;
	

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

	if((RecData[1] != 0x04) ||((RecData[5]&0x80) != 0x80)||(RecData[6] == 0x00))  
	{
		return TRUE;
	}


    RecData[0] = 0x8F;

	RecData[1] = 0x01;
	
	SetSpcPrjItem();

	SetLFConst(fileAddr,index,0x01, G_MenuIndex,sizeof(BYTE));

	G_MenuIndex ++; 

	AddValue((BYTE *)&RecData[1], (BYTE *)&RecData[6]);
	
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
	
	return TRUE;
	
}


/**
 * @brief 	扫描EF_6F18,将所有的ITEM_ID读取到输出中
 *
 * @param       fileAddr: EF_6F18
 * @param       output: array, id1 id2 id3
 * 	
 */
BYTE GetItemIDinEFMenuTelecom(UWORD fileAddr, BYTE * output)
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
	
	return TRUE;
}


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

#if NATIVE_CARD_UICC
		 MoveHugeEepromRunning();
#endif
	}

}

#if NATIVE_CARD_UICC
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
#endif

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
	return FALSE;
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
 *@brief  CATTP初始化 PRL断电恢复/Timer初始化

 *@author  LCZ  20201221



*/
void IF_ResumePrlData(void)
{
#ifdef CATTP
	ResumePrlData();
#endif 	
}


/**
 * @brief 		该函数发送Set Up Event List命令
 * 
 * @param 		NONE
 * 
 * @Returns   	TRUE/FALSE
 * 
 * @author 		XWH
 * 
 * @history 	add comment by xwh, @20150418
 */
void TerProSendSetUpEventList(void)
{
//#ifdef CATTP											 //仅在需要CATTP功能时需要SetUpEventList
	InitProactiveCommand(CMD_SETUPEVENT,0x00,DEV_ME);

	tempBuf[0] = TLV_EVENTLIST&0x7F;  // Event List Tag	
	tempBuf[1] = 0x02 ;
	tempBuf[2] = 0x09 ;
	tempBuf[3] = 0x0A ;

	AddTLV((BYTE xdata *)&ResData[1],(BYTE *)&tempBuf[0]);
	STKPerform = TRUE;
//#endif
}

/*
*@description: 判断当前tar是否为移动下行报文格式的tar值 ，如 OTA RFM ，远程写卡等 
			   对于这种报文的下行tar，按照移动安全报文格式去解密,同时对下行报文进行打标记 处理
*
*@function     :IF_CheckChimaMobileTar(BYTE *tar)
*
*@param        :BYTE * TAR:3字节 
*
*@return        :TRUE:为移动安全报文支持的tar值 /false:为标准格式的tar值 
*
*@history        :V1.0 :create by xuyz
*/
#if UICC__OPERATOR__CHINAMOBILE
BYTE IF_CheckChinaMobileTar(BYTE * tar)
{			
#if UICC__APP__WHITECARD
	if((tar[0] == 0xB0)&&(tar[1] == 0x01)&&(tar[2] == 0x2B))  //中国移动白卡入网应用
	{
		return TRUE;
	}
#endif
	
#if UICC__APP__LIVECARD
	
	if((tar[0] == 0xB0)&&(tar[1] == 0x00)&&(tar[2] == OTA_LiveWriteCard_Tag)) //现场写卡
	{	
		return TRUE;      //清除短信	
	}
	
	if((tar[0] == 0xB0)&&(tar[1] == 0x00)&&(tar[2] == OTA_GetCardInfo_Tag)) //现场写卡
	{	
		return TRUE;   //清除短信	
	}
	
#endif	
	return FALSE;
}
#endif

/*×@function     :IF_CheckTarSupportSingleDES

*@description: 在处理SMS的下行数据中，判断在某些tar情况下，需要检查下行报文或上行POR不能支持单DES 算法
×
×@param      :spiKicKid[4]: SPI1 SP2 KIC KID
*
*@return     :FALSE: 当前tar不能支持单des,tar类型由G_TARRecIndex[0]指示，其取值有  TarType_RAM、TarType_RFM 、TarType_ToolkitApplet
							/TRUE:不用判断是不是支持单DES
*
*@history
*/
BYTE IF_CheckTarSupportSingleDES(BYTE * spiKicKid)
{                                                        
#if UICC__FUNCTION__SINGLEDES
	return TRUE;
#else
	if(((((spiKicKid[2]&0x0C) == 0x00)||((spiKicKid[2]&0x0C) == 0x0C))&&(((spiKicKid[0]&0x04) == 0x04)||((spiKicKid[1]&0x10) == 0x10))&&(((spiKicKid[2]&0x03) == 0x01) ||((spiKicKid[2]&0x03) == 0x00)))     //102225 KIC(spiKicKid[2]&0x03) == 0x01 单DES 的各种条件
	||(((spiKicKid[3]&0x0C) == 0x00)&&(((spiKicKid[3]&0x03) == 0x01) || ((spiKicKid[3]&0x03) == 0x00))&&( (((spiKicKid[0]&0x03) != 0x00)&&((spiKicKid[0]&0x03) != 0x01)) ||( ((spiKicKid[1]&0x0C) != 0x00)&&((spiKicKid[1]&0x0C) != 0x04) ))) )  //102225 KID ((spiKicKid[3]&0x03) == 0x01) mac前提
	{
#if UICC__OPERATOR__CHINATELECOM
		return FALSE;
#else	
#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
		if((G_TARRecIndex[0] == TarType_RAM)||(G_TARRecIndex[0] == TarType_RFM))  //RAM RFM不能支持单des
		{
			return FALSE;
		}	
		else  //TOOLKIT
#endif		
		{
			return TRUE;
		}
#endif		
	}
	return TRUE;
#endif 	
}
//BYTE JcvmConfig_GetOPerator(void)
//{
//#if UICC__OPERATOR__CHINAMOBILE
//	return OPERATOR_CHINAMOBILE;
//#elif UICC__OPERATOR__CHINAUNICOM
//	return OPERATOR_CHINAUNICOM;
//#else
//	return OPERATOR_CHINATELECOM;
//#endif
//}
/*
*@description : 判断是否是RAM TAR（用于COSUP功能）
*
*@function      : ToolkitFindSDTars
*
*@param        : BYTE *tarBuf --- 待查找的tar值，3字节
*
*@return        : ISD_APPID(RAM) or ILLEGAL_APPID(not RAM)
*
*@HISTORY
*/
#if UICC__FUNCTION__RAM
BYTE ToolkitFindSDTars(BYTE *tarBuf)
{
#if COS_UPDATE_ONLINE_BY_NativeOS
	u8  tar[3];

	UPCOSConfig_GetUPCOSUMSSDTar(&tar);
	
	if((tarBuf[0] == tar[0])&&(tarBuf[1] == tar[1])&&(tarBuf[2] == tar[2])) //COSUP功能指定的TAR
	{
		return UPCOSConfig_GetUPCOSUMSSDAPPID();
	}
#endif
	return ILLEGAL_APPID;
}
#endif
/*
*@description : 判断是否是RAM TAR（用于COSUP功能）
*
*@function      : ToolkitFindHttpsTriggerTars
*
*@param        : BYTE *tarBuf --- 待查找的tar值，3字节
*
*@return        : ISD_APPID(RAM) or ILLEGAL_APPID(not RAM)
*
*@HISTORY
*/
BYTE ToolkitFindHttpsTriggerTars(BYTE *tarBuf)
{
	BYTE  tar[3];
	UWORD fileAddr;
	
	fileAddr = SelectLFFile(DF_Toolkit_P,EF_HttpsTriggle_TAR);

	if(fileAddr != FALSE)
	{
		ReadLFBuf(fileAddr,1,HTTPS_TAR_Offset,tar,3);
		
		//if((tarBuf[0] == 0x00)&&(tarBuf[1] == 0x00)&&(tarBuf[2] == 0x00))//http触发短信
		if((tarBuf[0] == tar[0])&&(tarBuf[1] == tar[1])&&(tarBuf[2] == tar[2]))//http触发短信
		{
			return TRUE;
		}
	}
		
	return FALSE;
}

/*@Function:    CheckRAMMinimumSecurityLevel                                           
 * 
 *@Descrip:     判断下行RAM 信息是否满足Minimum Security Level
 *
 *@See:	        TS 23.048
 *                                                                                                                                     
 *@Param:       spi0  --- SPI第一字节，与msl进行比较
 *              				                                                                                                         
 *@Return:      POR_OK(检查通过)/RFMSecurityLevel（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131105
 *  
 *@History:     V0.0---Create by XWH 20131105
 *
 *              V0.1---Modify by XWH 20140302,MSL Level支持多种模式 
*               V0.2---Modify by LCZ去掉2800/6F79文件,porfile下来的只有minispi
 */ 
BYTE CheckRAMMinimumSecurityLevel(BYTE spi)
{
	memref      X_A     sdTarbufferAddr,sdDataObj;
//	jref        X_A     sdTarbufferRef;
	u8          X_A     minispi;
	UWORD       X_A     fileAddr;
#if UICC__FUNCTION__STKAPI
	sdDataObj =  sdData_getsdDataObjByAppId(G_TARRecIndex[1]);

/*	sdTarbufferRef = sdData_getSD_Tar_bufferRef(sdDataObj) ;//调用该函数前 RAM TAR已经被找到 所以这里肯定有值

	if(sdTarbufferRef == REF_NULL)
	{
		return POR_OK;
	}*/

	sdTarbufferAddr = sdData_getSD_Tar_bufferField(MEMtPM8(sdDataObj));

	minispi = nvmReadU8(MEMtPM8(sdTarbufferAddr + sdMslValue_Offset));
#else
	minispi = 0x12;//默认值
	
#if COS_UPDATE_ONLINE_BY_NativeOS	
	if(IF_UPCOS_EnvelopHandle() == TRUE)
	{
		minispi = UPCOSConfig_GetUPCOSUMSSDMSL();//默认值
	}
	else
#endif
	{
#if GP__AMENDMENT__B__FOR_MASK		
		if(G_TARRecIndex[0] == TarType_HTTPS)
		{
			fileAddr = SelectLFFile(DF_Toolkit_P,EF_HttpsTriggle_TAR);

			if(fileAddr != FALSE)
			{
				ReadLFBuf(fileAddr,1,HTTPS_MSL_Offset,&minispi,1);
			}
		}
#endif		
	}
#endif	
	if(((spi&0x03)>=(minispi&0x03))&&((spi&0x04)>=(minispi&0x04))&&((spi&0x18)>=(minispi&0x18)))
	{
		return POR_OK;
	}

	return RFMSecurityLevel;

/*	UWORD xdata fileAddr;
	BYTE idata minispi;
	
	fileAddr = SelectLFFile(DF_Toolkit_P,EF_RAMAPDUTars);

	if(fileAddr == FALSE)
	{
		return POR_OK;
	}
	
	minispi = 0x00;

	ReadLFBuf(fileAddr,G_TARRecIndex[1],sdMslValue_Offset,(BYTE *)&minispi,0x01);     //minispi

	if(((spi&0x03)>=(minispi&0x03))&&((spi&0x04)>=(minispi&0x04))&&((spi&0x18)>=(minispi&0x18)))
	{
		return POR_OK;
	}
	
	return RFMSecurityLevel;
*/
}	
/*
*@description: 获取CATTP 指定的密钥
*
*@param
*
*@description
*
@history
*/	//注释本函数，native的rfm-key不再保存在文件中
//BYTE GetRFMRAMKey(BYTE index ,BYTE *key,BYTE mode)
//{
//	BYTE idata result;
//	UWORD xdata fileAddr;
//	
//	fileAddr = SelectLFFile(DF_STK,LDFile_KeyFile);
//	
//	if(fileAddr == FALSE)
//	{
//		return FALSE;
//	}

//	index = index*2;

//	if(mode == MODE_KIC)
//	{
//		index = index-1;
//	}

//	result = ReadLFBuf(fileAddr,index,0x00,key,0x10);
//	
//	if(result == FALSE)
//	{
//		return FALSE;
//	}
//	
//	return 0x10;	
//}