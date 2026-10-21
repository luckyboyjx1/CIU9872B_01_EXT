/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.22

 * MODEL: 	All model

 * RELEASE: 2020-3-2

 * AUTHER:	杨健

 * HISTROY:	V1.0  创建源文件,用于中国移动卡片
	  					
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
#ifdef GH_CARD
#include "authorization.h"
#endif



#ifdef High_Update_FOR_Test
#include  "PrivateCommandTest.h"
#endif


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
		G_CODE_SIZE = 0x12000;						//CODE区大小，此处根据需求调整
#endif	
	
	
		
#ifdef High_Update_Function
	//G_High_Update_NUM = 0x28;					//默认0x28(40页),满足中国移动需求
	G_High_Update_NUM = 0x00;
#else
	G_High_Update_NUM = 0x00;					//根据需求设置,不需要使用时配置为0
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
#ifdef SingleSIM
			ReadTFBuf(fileAddr,1,(BYTE *)&ResData[3],sizeof(ATRDefine)-1);
#else			
			ReadTFBuf(fileAddr,1,(BYTE *)&ResData[5],sizeof(ATRDefine)-3);
#endif
			return;

		}
	}	

	
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

/**
 * @brief 		IF_ActiveAppCommand接口,执行指令后发送状态字前执行操作;默认返回FALSE
 * @author 		yangjian
 *  			
 */

BYTE IF_ActiveAppCommand(void)
{		
	return FALSE;
}


/**
 * @brief 		IF_ProcessApp接口,执行指令发状态字后处理应用操作(中国联通有需求使用)
 * @author 		yangjian
 *  			
 */

void IF_ProcessApp(void)
{
	return;
}


/**
 * @brief 		IF_SelectCommandP2Validity接口,兼容联通平台与其它平台需求
 * @author 		yangjian
 *  			
 */

BYTE IF_SelectCommandP2Validity(void)
{
#ifdef UnicomSET
	if((P2&0xB3) != 0x00)
#else
	if((P2&0x93) != 0x00)
#endif
	{
		P1P2Error();					//Send 6B 00
		return FALSE;
	}

	return TRUE;

}
/**
 * @brief 		IF_SelectCommandP2Validity2接口,兼容联通平台与其它平台需求
 * @author 		yangjian
 *  			
 */


BYTE IF_SelectCommandP2Validity2(void)
{
#ifdef UnicomSET
	if((P2 & 0xB0) != 0x00)
#else
	if((P2&0x80)==0x80)
#endif
	{
		P1P2Error();					//Send 6B 00
		return FALSE;
	}

	return TRUE;

}
/**
 * @brief 		IF_ParentDFFCP接口,兼容联通平台与其它平台需求
 * @author 		yangjian
 *  			
 */

BYTE IF_ParentDFFCP(UWORD fileAddr,WORD fileName)
{
	BYTE  idata responseLgth;

#ifdef 	UnicomSET
		responseLgth =  SetChannel(fileAddr,fileName,TRUE);	
		
		G_UnicomResponseLgth = responseLgth;	

		return FALSE;
#else		
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
#endif	

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
#ifdef 	UnicomSET
	G_AppControl = 0x40;				//初始化时bit7置1,此控制变量在中国联通平台中使用
#endif

#ifdef	GH_CARD
	G_Authorization_Status=FALSE;

	G_ReadLOCI=FALSE;
#endif

	G_OpenControl = 0x00;			    //控制变量供应用开发使用,初始化为0
	
	G_STKBusy = 0x00; 					//STK控制变量,初始化为0
	
	memset(G_TerminalProfile, 0x00,sizeof(G_TerminalProfile));
	
	InitUpdateVariable();

	G_DF_ID_USIM = 0x7FF0;

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
#ifdef GH_CARD
			if(CompareGHCardAuth() == FALSE)
			{
				break;
			}
#endif


#ifdef SingleSIM
#ifdef V3
		RunGSMAlgorithmV3();
#else
		RunGsmAlgorithm(); 	
#endif				
#else 
		RunAlgorithm();			
#endif		

			break;

#ifdef ChinaMobileSET
		case 0xF0:
			UpdateFile();
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
#ifdef UnicomSET
		case 0xF4:					//中国联通现场写卡APDU指令,目前仅支持USIM写卡指令

			if(CLA != 0xA0)		//现场写卡规范中CLA为A0
			{
				ClaError();
				break;
            }
			if(P1 != 0x00)
			{
				P1P2Error();
			}
			else
			{
				if(P2 == 0x00)			//写卡
				{
					UnicomSetIMSI();
				}
				else if(P2 == 0x01)//重置IMSI
				{
					UnicomResetIMSI();
				}
				else
				{
					P1P2Error();
				}	
			}
			
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
	UWORD xdata fileAddr;
	BYTE idata recordNo;
#ifdef IOT_PLMN_FB	
	BYTE idata result;
#endif	

	if(INS == 0xDC) //phone update
	{		
		if((G_CurrentDfName[G_ChannelNum] == 0x7F10)&&(G_CurrentEfName[G_ChannelNum] == 0x6F3A))
		{
			if(P2 == ABSOLUTE_CURRENT)
			{
				if(P1 == 0x00)
				{
					recordNo = G_CurrentRecNum[G_ChannelNum];
				}
				else
				{
					recordNo= P1;
				}
			}
			else
			{
				recordNo = G_CurrentRecNum[G_ChannelNum];
			}	
		
			fileAddr = SelectLFFile(0x7F10,0x5F3A);

			fileAddr = SelectFile(fileAddr,0x4F41);

			if ((fileAddr != FALSE)&&(G_CommandMode == Command_Mode_2G))
			{
				SetLFConst(fileAddr, recordNo, 0x00, 0x01, 0x01);
			}
		}
	}
	else if (INS == 0xD6)
	{
		if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)
		{		
		
#ifdef IOT_PLMN_FB
			result  = IfUpdate6F7BAllow();
		
			if(result == FALSE)
			{
				return FALSE;
			}			
#endif
#ifdef ChinaMobileSET									//只有中国移动平台有此功能
			return  SmartLumingWrite ()	;
#endif
#ifdef Unicom6F7B
			return Update6F7BFile_LD(0x6F61);
#endif
		
		}
	}
	else
	{
		return TRUE;
	}

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
#ifdef GH_CARD
	UWORD xdata	tempAddr;
#endif

	if(INS == 0xB0)
	{
#ifdef ChinaMobileSET									//只有移动平台有优选漫游功能		
		SmartLumingRead();	//优选漫游
#endif


#ifdef GH_CARD
		if((G_CurrentEfName[G_ChannelNum] == EF_LOCI)
		&&((G_CurrentDfName[G_ChannelNum] == 0x7F20)||(G_CurrentDfName[G_ChannelNum] == 0x7F21)||(G_CurrentDfName[G_ChannelNum] == 0x7FF0)||(G_CurrentDfName[G_ChannelNum] == 0x7FFF))
		&&(G_ReadLOCI == FALSE))	//第一次读取EF_LOCI
		{
			tempAddr = SelectTFFile(DF_MF,GH_SWITCH_FILE);	//开关文件，初始值为0x01

			if(tempAddr != FALSE)
			{
				ReadTFBuf(tempAddr,0x00,(BYTE *)&ResData[0],0x01);
		
				if(ResData[0] == 0x01)
				{
					RecData[7] = 0x00;
	
					RecData[8] = 0x00;
			
					RecData[9] = 0xFF;
			
					RecData[10] = 0x01;
	
					G_ReadLOCI=TRUE;
				}
			}
		}
#endif

	}
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
	if(INS ==0x10)
	{
#ifdef UnicomSET
		if((G_AppControl&0x40)==0x40)			//已通过复位
		{
			G_AppControl = 0x80;  		//开机后bit8置1，其它位为0
		}
		
		UnicomSetUpMenu();
#endif
	}
	
	
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
#ifdef GH_CARD_GD
	GHActiveProcess(udPos);
#endif

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
	BYTE idata smscPtr;

	ptr = FindTag(&RecData[0],0xD1,P3);

	smscPtr = FindTag(&RecData[ptr+1], TLV_ADDRESS, RecData[ptr]);

	if(smscPtr != FALSE)
	{
		smscPtr = smscPtr + ptr + 1;					//offset of SMSC Length
	}

	ptr = FindTag(&RecData[ptr+1],TLV_SMSTPDU,RecData[ptr]) + ptr;

	if(ptr == FALSE)
	{
		return FALSE;
	}

	result = 4 + (RecData[ptr+3]/ 2) + (RecData[ptr+3]&0x01);		//Len of TPOA

	udPos = ptr + result + 10;						// TPOA len, TPI/NPI, PI, DC, Service Stamp

	if((RecData[ptr+2]&0x03) != 0x00)	   		// TP_MTI should be "SMS-DELIVER"
	{
		return FALSE;
	}

	if ((RecData[ptr+2] & 0x60) != 0x00)   		//UDHI :have the UDHL							
	{
		ptr = udPos + 1;		////point to  信息标识长度

		result = FindTag(&RecData[ptr+1], 0x70, RecData[ptr]);

		if(result != FALSE)
		{
			ptr = ptr + RecData[ptr] + 3;		//point to the CPL

			ptr = ptr + 5;						//point to TAR
		
			if((RecData[ptr] == 0xB0) && (RecData[ptr+1] == 0x00))	//check TAR 2/3
			{
				switch(RecData[ptr+2])					//check TAR 3/3 ,根据TAR类型的不同，在此处自行扩展
				{					

					default:
						IF_PP_Download_Hanlder(RecData[ptr+2],ptr,udPos);
					break; 
				}		
			}
			else
			{
				return FALSE;
			}
		}
	}
	else
	{
		IF_NO_UDHL_Hanlder(udPos);
	}

	return TRUE;

}


/**
 * @brief 		处理菜单选择结果,请根据需求修改
 * 
 * @Returns   	CMD_Success
 * 
 * @author 	    yangjian	 
 * 
 * @history 	
 */
BYTE MenuSelection(void)
{

	InitVariable();	

	return CMD_Success;
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
	BYTE  idata result;

	result = FALSE;

	return result;

}


/**
 * @brief 	This function is to send SetUpEventList command,可根据需求添加发送事件
 		
 * @author 	 yangjian	
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
 * @brief 		添加菜单项目，菜单的启用和禁用等属性请参看EF_6F18的说明
 *
 * @param 		fileAddr: EF_6F18
 * @param 		index: Item Identifier
 *
 * @author 		XuQingsheng
 *
 * @history 	created by XuQingsheng, @071220 20:01
 */
void AddMenuItem(UWORD fileAddr, BYTE index)
{
  	ReadLFBuf(fileAddr,index,0x00,&RecData[1],GetLFRecNumLgth(fileAddr,FileLgth));

	if(RecData[1] != 0x06)
	{
		return;
	}

    RecData[0] = 0x8F;

	RecData[1] = 0x01;

	RecData[2] = index;

	AddVlaue((BYTE *)&RecData[1], (BYTE CONST *)&RecData[6]);

	AddTLV((BYTE *)&ResData[1], (BYTE CONST *)&RecData[0]);
}



/**
 * @brief 		建立菜单，根据 'SolidifyNum' 确定固化菜单个数。菜单排序的规则请查看操作手册对EF_6F18的描述
 *
 * @author 		XuQingsheng
 *
 * @history 	created by XuQingsheng, @071220 20:01
 */
void SetUpMenu(void)
{
	UWORD xdata fileAddr;
	BYTE idata num;
	WORD idata i;

	if(ValueBit(G_SystemStatus, B_meSurportStk) == FALSE)  // 若当前terminal profile不支持setupmenu主动式命令，则不进行拼装
	{
		return;
	}
	
	InitProactiveCommand(CMD_SETUPMENU,Q_SETUPMENU,DEV_ME);

    fileAddr = SelectTFFile(DF_PHASE,EF_STKName); //注意：根据规范，SIM CARD下的6F54 位于7F20下；UICC card的6F54位于7F10下

	if(fileAddr != FALSE)
	{
		ReadTFBuf(fileAddr,0,(BYTE *)&RecData[0],1);

		if((RecData[0]!=0x85)&&(RecData[0]!=0x05))
		{
			return;
		}

		RecData[0] = 0x85 ;

		ReadTFBuf(fileAddr,1,(BYTE *)&RecData[1],1);

		ReadTFBuf(fileAddr,2,(BYTE *)&RecData[2],RecData[1]);

		AddTLV((BYTE *)&ResData[1], (BYTE CONST *)&RecData[0]);
	}
	else
	{
		return;
	}

	fileAddr = SelectLFFile(DF_STK,EF_Menu);

	if (fileAddr == FALSE)
	{
		return;
	}

	for(i = 0x01;i <= SolidifyNum; i ++)	//可更名固化
	{
	 	AddMenuItem(fileAddr, i);
	}

	num = FindTag(&ResData[3],0x8F,ResData[2]);

	if(num==0)
	{
		 RecData[0]=0x8F;

		 RecData[1]=0x00;
	
		AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[0]);
	}
	
	STKPerform = TRUE;
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
		
#ifdef GH_CARD_GD
		case D_GH_Display1:
			cephDisplayTextSingle(0x01, UCS2, (BYTE *)GHResultDisplay1);
			break;
		case D_GH_Display2:
			cephDisplayTextSingle(0x01, UCS2, (BYTE *)GHResultDisplay2);
			break;
		case D_GH_Display3:
			cephDisplayTextSingle(0x01, UCS2, (BYTE *)GHResultDisplay3);
			break;
		case D_GH_Refresh:
			InitProactiveCommand(CMD_REFRESH, 0x03, DEVICE_ME);
			STKPerform = TRUE ;
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

/**
 * @brief 		IF_UpdateBinary,写透明文件后的数据处理
 * 
 * @param       
 *
 * @Returns   	
 *
 * @author 		
 *
 * @history 	
 */

BYTE IF_UpdateBinary(void)
{
#ifdef UnicomSET
	UWORD idata fileAddr;
   
	if(G_CurrentEfName[G_ChannelNum]==0x6F7E)	//对STK流程中更新文件的动作，不执行应用		
	{
		if(RecData[P3-1] == 0x00)
		{		
			G_AppControl |= 0x02;	//更新6F7E成功	

			fileAddr = SelectTFFile(DF_MF, 0xFF00);
			
			if(fileAddr == FALSE)
			{
				return FALSE;
			}
	
			SetTFConst(fileAddr, 0, 0x01, 1);		  //更新标识位，表示已登录过网络
				
		}
	}	
#endif
	
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
	if(RecData[3] == CMD_SETUPMENU)
	{
#ifdef UnicomSET		
		if((GetResponseResult() == CMD_SUCCESS)&&(G_AppControl == 0x80))
		{
				G_AppControl |= 0x01;					//开机后的SetupMenu执行完成，
				G_AppControl = (G_AppControl&0x7F);		 //完成后bit8置0
				ProcessPendingMessageAlert();
		}
#endif
	}
	else
	{;}	
	
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
#ifdef GH_CARD_GD
	UWORD xdata fileAddr;
	BYTE idata status[2];
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
	
#ifdef ChinaMobileSET
#ifndef GH_CARD
		SetUpMenu();
#endif
#endif

#ifdef GH_CARD
#ifdef GH_CARD_GD
	fileAddr = SelectTFFile(DF_TELECOM, GH_KeyStatus);

	ReadTFBuf(fileAddr, 0x00, status, 0x02);

	if(status[0] == FALSE)		//公话卡认证密钥为空，进入绑定流程
	{
		G_Authorization_Status = TRUE;
		
		GHActiveCall();		//呼叫特服号码
	}
	else
	{
		if(G_Authorization_Status == FALSE)
		{
			Authorization();	//认证流程
		}
		else
		{
			if(status[1] == FALSE)		//验证码标识
			{
				cephDisplayTextSingle(0x01, UCS2, (BYTE *)GHDisplayText1);	//欢迎使用中国移动G3信息机！
			}
			else
			{
				GHOpenProcess();		//开通流程
			}
		}	
	}	
#else
	Authorization();
#endif		//end of define GH_CARD_GD
	

#ifdef NEEDSETUPMENU
#ifdef GH_CARD_GD
	if((status[0] != FALSE) && (G_Authorization_Status == TRUE))
	{
		SetUpMenu();		//已绑定成功，且认证通过后，在正常登网/号码开通流程后，进行SetUpMenu	
	}
#else
	SetUpMenu();
#endif		//end of define GH_CARD_GD		
#endif		//end of define NEEDSETUPMENU

#endif		//end of define GH_CARD


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

#ifdef UnicomSET													  //联通需求
	if(STKPerform == TRUE)
	{
		SimAppTkitBusy();											
		return TRUE;
	}
#endif

	result = FALSE;

	if(RecData[0] == 0xD1)
	{
		PP_Download_Hanlder();
	}
	else if(RecData[0] == 0xD6)
	{
		EventDownload();
	}
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


void IF_ResumeResponseMode(void)
{

}

/*
 * @brief 	IF_STKTerminalResponse,STK菜单中收到14响应后的处理接口
 *
 * 		 		   
*/

void IF_STKTerminalResponse(void)
{
 	return;
}


/*
 *  @brief 	IF_LiveCardBusy,提供给中国移动空中写卡功能使用,不使用时默认返回FALSE
 * 
*/
BYTE IF_LiveCardBusy(void)
{
 	return FALSE;
}

#ifdef UnicomSET

/*
 * @brief 	UnicomSetUpMenu,中国联通建立菜单函数

 */
void UnicomSetUpMenu(void)
{
	UWORD xdata fileAddr;

	InitProactiveCommand(CMD_SETUPMENU,Q_SETUPMENU,DEV_ME);

    fileAddr = SelectTFFile(DF_PHASE,EF_STKName);

	if(fileAddr != FALSE)
	{
		RecData[0] = 0x85;
	
		ReadTFBuf(fileAddr,1,(BYTE *)&RecData[1],1);

		ReadTFBuf(fileAddr,2,(BYTE *)&RecData[2],RecData[1]);
	
		AddTLV((BYTE *)&ResData[1],(BYTE *)&RecData[0]);
	}		
	
	RecData[0] = 0x8F;		  									//没有目录显示时
	RecData[1] = 0x00;

	AddTLV((BYTE *)&ResData[1],(BYTE *)&RecData[0]);
	
	
	STKPerform = TRUE;
}

/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UnicomSetIMSI
  
  Command format:    A0    F4  00   00   12  
  
  Data    content:   IMSI数据1+IMSI数据2    
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       中国联通现场写卡APDU指令：
					 如果是2G电信智能卡，则IMSI数据1和IMSI数据2是完全一样的，是写入7F20/6F07的数据。若IMSI数据1和IMSI数据2不一致，则IMSI数据写入失败，卡片返回"9804"。
					 如果是3G电信智能卡，则IMSI数据1是写入7F20/6F07的数据，IMSI数据2是写入7FF0/6F07的数据

  status:			 TEST    

 -------------------------------------------------------------------------------------------------------------------*/    

void UnicomSetIMSI(void)
{
	UWORD xdata fileAddr;
	UWORD xdata fileAddr2;
	
	if (P3 != 0x12)
	{
		P3Error();
		return;
	}

	ReceiveData();

	memset(&tempBuf[0],0xFF,0x09);

	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);

	fileAddr2 = SelectTFFile(DF_ADF, EF_IMSI);

	ReadTFBuf(fileAddr, 0x00, &ResData[0], 0x09);

	ReadTFBuf(fileAddr2, 0x00, &ResData[10], 0x09);

  if ((memcmp(&ResData[0], &tempBuf[0], 0x09) != EQUAL) || (memcmp(&ResData[10], &tempBuf[0], 0x09) != EQUAL)) //7F20/6F07或7FF0/6F07非全FF				
	{
		SecuritySatisfied();							//非白卡返回安全性错误
		return;
	}
	
	if (fileAddr != FALSE)
	{
		WriteTFBuf(fileAddr, 0x00, &RecData[0], 0x09);	
	}
	
	if (fileAddr2 != FALSE)
	{	
		WriteTFBuf(fileAddr2, 0x00, &RecData[9], 0x09);
	}

	CommandFinish(0x00);

}


/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UnicomResetIMSI
  
  Command format:    A0    F4  00   01   08  
  
  Data    content:   data  
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       中国联通现场写卡IMSI重置
					 通过该指令恢复USIM卡的IMSI文件（3F00/7F20/6F07和3F00/7FF0/6F07）的内容为0xFF FF…FFh 
					 

  status:			    

 -------------------------------------------------------------------------------------------------------------------*/    
void UnicomResetIMSI(void)
{
	UWORD xdata fileAddr;
	BYTE idata tag;
	BYTE xdata key[8];
	KeyStructer xdata  pinKey;
	
	if (P3 != 0x08)
	{
		P3Error();
		return;
	}

	ReceiveData();
	
	fileAddr = SelectTFFile(DF_MF, 0xFF00);
	
	ReadTFBuf(fileAddr, 0, &tag, 1);
	if(tag == 0x01)//卡片曾经成功登陆过网络
	{
		//SW1 = 0X98;
		//SW2 = 0X10;
		InvalidationContradiction();

		return;
	}
	
	fileAddr = SelectTFFile(DF_PHASE, EF_KI);
	
	ReadTFBuf(fileAddr, 8, &key[0], 8);
	
	ReadKey(PIN2, (BYTE *)&pinKey.KeyId);
	
	memcpy(&ResData[0], (BYTE *)&pinKey.IscContent[0], 8);
	
	SingleDES(&ResData[0], &key[0], &ResData[0], CBC_Encrypt, 8);	
	
	if(memcmp(&ResData[0], &RecData[0], 0x08) != EQUAL)//PUK2比对失败
	{
		SecurityError();
		return;
	}
	
	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);
	
	SetTFConst(fileAddr, 0, 0xFF, 9);
	
	fileAddr = SelectTFFile(DF_ADF, EF_IMSI);
	
	SetTFConst(fileAddr, 0, 0xFF, 9);
		
	CommandFinish(0x00);
}
#endif


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
	return 0xFF;
}


/*
   中国电信RFM功能使用
*/
BYTE IF_LDFileDeliver(void)
{
   return FALSE;				
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
	return FALSE;
}


/*
  *@brief  IF_CommandAttemper,可根据需求配置是否拒绝非激活的操作模式	

 *@author  yangjian
 
 *@return:  

*/
BYTE IF_CommandAttemper(void)
{
	if (((G_CommandMode == Command_Mode_2G)^(CLA==0xA0))||(((CLA&0xFC) != 0x80)&&((CLA&0xFC) != 0x00)&&(CLA != 0xA0)))		//拒绝非激活的操作模式							
	{
		ClaError();

		SendResponse();

		return FALSE;
	}

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
#ifdef SENSITIVEDATA
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
 *@brief  IF_UpdateRecordPRL,PRL指令接口,可通过该接口配置中国电信CSIM卡的PRL功能需求,不需要PRL功能时默认直接返回

 *@author  yangjian	


*/

void IF_UpdateRecordPRL(BYTE recNo)
{

}


/*
 *@brief  CATTP初始化 PRL断电恢复/Timer初始化

 *@author  LCZ  20201221



*/
void IF_ResumePrlData(void)
{
	
}



/*
 *@brief  IF_CDMAGSM,可通过接口配置中国电信CSIM卡功能(不需要该功能时直接返回)

 *@author  yangjian

*/
void  IF_CDMAGSM(void)
{
	return;
}

/*
 *@brief  IF_VerifyChv,校验PIN指令接口,	可通过该接口配置中国电信CSIM卡特殊需求

 *@author  yangjian

*/

BYTE IF_VerifyChv(void)
{			
	if((P2==0x00)||(P2>0x05))
	{
		P1P2Error();	                                            //6B 00
		return FALSE;
	}

	return TRUE;
			
}

/*
 *@brief  IF_VerifyChv2,校验PIN指令接口,可通过该接口配置中国电信CSIM卡特殊需求

 *@author  yangjian

*/
void IF_VerifyChv2(BYTE mode)
{
	if((mode == mode_Verify) && (P2 > 0x02))
	{
		P2 = P2 + 0x07;
	}

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
BYTE IF_IncreaseCommand(void)
{		 
	if(P3 != 0x03)
	{
		P3Error();				 
		return FALSE;	
	}

	return TRUE;

}



