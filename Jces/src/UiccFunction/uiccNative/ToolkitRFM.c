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

*Description :该文件主要用于存放实现RAM RFM 以及保存SMS ，CB短信的一些函数
*
*History     :
	
author    time      version    description
xuyz   2013/12/06   V1.0       创建该文件
************************************************************************************************************************************************************************************************/

#include    <uicc_error.h>
//#include    "hard_api.h"
#include    "fileAction.h"
#include    "fileSystem.h"
#include    "gsmpublic.h"
#include    "arithmatic.h"
#include    "Crypto_Constant.h"
#include    "stkhead.h"
//#include    "stkalert.h"
#include    "ToolkitRFM.h"
#include    "interface.h"
//#include    "uicc_interface.h"
//#include    "STKCATRuntimeEnviorment.h"
//#include    "STKRemoteAppManagement.h"
//#include    "ToolkitAPIPublic.h"
#include 	"keySet.h"
//#include 	"AES.h"
#include	"KeyLogicManager.h"
//#include	"profileApi.h"
//#include    "uicc_M2M.h" //warning GetIsdpPidDcs
#include    "Interface_AnnexB.h"
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#include "TlsMainFlow.h"
#endif

#if UICC__CDMA__SMS__DOWNLOAD
#include "CDMA.h"
#endif
#include "CompactManage.h"
#include "ExpandedManage.h"
//#include "platform_api.h"
#include "SCPInterface.h"
#include "opFunction.h"
#include "object.h"
#include "SecurityDomainData.h"
#include "Interface_LTEFromBoot.h"

#if UICC__FUNCTION__CATTP
	#include "CatTp.h"
#endif

#if UICC__CDMA__SMS__DOWNLOAD
BYTE xdata G_IsCdmaSms;         //代表当前是不是cdma
BYTE xdata G_cdmaSmsNum;     //cdma总数
BYTE xdata G_cdmaRecNum;     //当前CSMS短信头记录  发POR时候添加tpoa使用
#endif

#if UICC__APP__LIVECARD
	#include "LiveWirteCardXL.h"
#endif

#if COS_UPDATE_ONLINE_BY_NativeOS
#include "FunEntryConfig.h"
#include "UpCos.h"
#endif
//#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)
BYTE    xdata    G_EnvelopeTag;    			//用于保存当前的Envelope BER-TLV tag ，C2指令的第一字节
BYTE    xdata    G_Post9EFlag;  //BIT0:1表示当前状态字为9EXX ,else 0为9FXX
BYTE    xdata    G_CurrentRFMResponse;       //通过getresponse需要获取的当前POR的数据长度
BYTE    xdata    G_formatedSPIOffset;        //缓存当前format ENV Data 的spi在6F90中的偏移
BYTE    xdata    G_ToolkitTar[0x03];         //缓存当前format_CB,format_update format_env的tar值
ToolkitHandler xdata ToolkitHandlerList[5];
BYTE    xdata    G_formatedTPOAOffset;       //缓存当前format ENV Data 的TPOA 在6F90中的偏移
UWORD   xdata    G_EnvelopHanderAddr;
WORD   xdata   G_GPErrorCode;
BYTE    xdata    G_RFMRAMKeyversion;         //缓存当前RAM短信使用的RAM密钥版本
/******************for  RAM APDU*******************************************************************************************/
BYTE    xdata    G_RAMSessionSdId;
//BYTE    xdata    G_RAMGetStatus[19];

//WORD xdata G_CurContext; //
BYTE  xdata G_86errorNum;
BYTE xdata G_CCLength;



//#define D_TmpBufStart 232

BYTE  xdata G_CurrentComNUM;  //保存当前RFM执行的APDU 个数

#if UICC_SUPPORT_CHAGE_DELIVER_POR
BYTE xdata G_BakCommandNumber;
#endif

BYTE    xdata    G_FormatAndType;				//当前数据格式的模式
WORD    xdata    G_CurrentRemoteResponseLgth;	//https的下行数据执行完apdu指令后的响应数据长度

WORD xdata G_CommandResponseLength;

BYTE xdata G_JCREMainProcessMode;
BYTE xdata G_NewSCPKey;     // add chentao 更新SCP80key后，发响应值需要用更新前的key，发完响应后用新的key
UWORD xdata G_CurrentRFMEfAddr; //用来保存102226读取数据过长时候的地址
WORD  xdata G_CurrentRFMEfOffset; //added by xuweihua@20220514 用来保存102226读取数据过长时候的偏移
BYTE xdata G_Source_Type;      //区分数据源 CATTP/Http/SMS
BYTE xdata G_SpecialRFM;
BYTE xdata  G_Data_Type;       //区分数据格式 压缩/AA/AE等
BYTE  xdata G_LAST_REFNUMBER; //批次号 直接用全局变量  不从文件EF_RFMRAMWIBConfig里面或者下行读取
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
BYTE  xdata G_Deliver_T0_Submit;
WORD  xdata G_PorAdditionalLen;
UWORD xdata G_Back_EnvelopHanderAddr;//GP补充测试脚本支持SCP02和KeyManage版20170309/Bug_List_07_01/步骤5 在STK中的deliver转Submit会导致发送第一条0B和后面的submit使用的G_EnvelopHanderAddr地址不一样
#endif
#if UICC_SUPPORT_RFMHANDLER_D0
WORD  xdata CommandStartNum;
#endif

BYTE    xdata    G_TARRecIndex[3];		     //G_TARRecIndex[0]: tarType ,G_TARRecIndex[1]:tar在对应文件中的记录号(对于RAM，存的是TAR对应的SD的appid)，G_TARRecIndex[2]：scp80SDId: 对于Toolkit 或ram操作，保存当前需要使用到的SCP80 的sdId

#if (UICC__FUNCTION__RFM || UICC__FUNCTION__RAM)

/*
*@description:  清除RFM数据counter
*
*@param		 : void
*
*@return	 :void
*
*@history	 :V1.0:create by xuyz@20150427
*/
void LDCleanCounter(void)
{		
	BYTE idata recNum,i;
	UWORD xdata fileAddr;
	
	fileAddr = SelectLFFile(DF_Toolkit_P, EF_RFM_CounterFile);	
	
	if(fileAddr == FALSE)
	{
		CommandFinish(0x00);
		return;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i=1;i<=recNum;i++)
	{
		SetLFConst(fileAddr,i,0x00,0x00,5);	
	}

	CommandFinish(0x00);
 	
}

/*
*@function   :IsBufAllBytesMatch
*
*@description:判断buf的所有字节是否都是checkByte
*
*@See        : 
*
*@param      :void
*@return     :TRUE/FALSE
*
*@history    :V1.0----created by xwh @20201111
*/
BYTE IsBufAllBytesMatch(BYTE * buf,BYTE len,BYTE checkByte)
{
	BYTE  idata i;
	
	for(i=0;i<len;i++)
	{
		if(buf[i] != checkByte)
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

/*
*@function   :IF_InitExtendedRamRfm
*
*@description:支持扩展RFM/RAM时需要初始化的相关变量
*
*@See        : 
*
*@param      :void
*@return     :void
*
*@history    :V1.0----created by LCZ @1090110
*/
void InitVariableRemoteModule(void)
{
	UWORD xdata fileAddr;
	
	fileAddr = SelectPrivateTFFile(DF_Toolkit,EF_ScriptChainingFile); //目前只支持RFM扩展格式  可以选择直接清除
	
	if(fileAddr != FALSE)
	{

		ReadTFBuf(fileAddr, 0x00, (BYTE *)&ChainingValue,sizeof(ChainingValue));
			
		if((ChainingValue.firstScriptValue == 0x01)||((ChainingValue.firstScriptValue == 0x11)&&(ChainingValue.previousValue == 0x03)))
		{
			SetTFConst(fileAddr ,0x00,0x00,sizeof(ChainingValue));  
		}
	}

	G_CurrentRemoteResponseLgth = 0x00; //响应数据长度

	G_FormatAndType = FormatType_NULL;  // AA  AE 压缩等

	G_CurrentComNUM = 0x00; //指令执行条数
#if UICC_SUPPORT_RFMHANDLER_D0	
	CommandStartNum = 0x00;  //RFM主动性命令
#endif
	G_EarlyResponseTag = 0x00;

	//开机时不再初始化 EF_HTTPSResponseData/EF_RAMExtendResponse 文件，SMS 在进入扩展格式处理时初始化,by xwh 20210628
	//GPRAMRFMExtendResponsePublic(TAG_ClearResponseFile);    //清空6F93 RFM执行响应文件: EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
	
	G_LAST_REFNUMBER = 0x00;  //批次号
	
	G_Data_Type = DataType_NULL;        //数据格式  RFM RAM
	
	G_Source_Type = SourceType_NULL; //数据来源
	
	G_SpecialRFM = FALSE;

	G_CurrentRFMEfAddr = FALSE;      //RFM读取数据的地址
	
	G_CurrentRFMEfOffset = 0x0000;
	
	G_86errorNum = 0x00;         
	
	G_NewSCPKey = FALSE; // 更新SCP80key后，发响应值需要用更新前的key，发完响应后用新的key

	G_CurrentChainingValueTag = FALSE;
	
	G_CAPDUResponse = 0x00;      //没有执行过 R_APDU

	G_ChainingTagRec = FALSE;

#if UICC__CDMA__SMS__DOWNLOAD
    G_IsCdmaSms = FALSE;         //代表当前是不是cdma
    G_cdmaSmsNum = 0x00;     //cdma总数
    G_cdmaRecNum = 0x00;     //当前CSMS短信头记录  发POR时候添加tpoa使用
#endif
}

/*@Function:    CheckMinimumSecurityLevel                                           
 * 
 *@Descrip:     判断下行信息是否满足Minimum Security Level
 *
 *@See:	        ts_143019v050600p.pdf,TS 23.048
 *                                                                                                                                     
 *@Param:        spi0  --- SPI第一字节，与msl进行比较
 *@param        :*buf: Minimum SPI1
 *				                                                                                                         
 *@Return:      TRUE(检查通过)/FALSE（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131018
 *  
 *@History:     V0.0---Create by XWH 20131018             
 */ 
BYTE CheckMinimumSecurityLevel(BYTE spi0,BYTE *buf)
{
	/*If the length of the Minimum Security Level field is zero, no minimum security level check shall be performed by the
          receiving entity.*/
	if(buf[0]==0x00)
	{
		return POR_OK;
	} 
	
	if((buf[0]!=0x02)||(buf[1]!=0x01))	 //Minimum SPI1
	{
		return POR_OK;
	}    
	
	/*
	- if SPI1.b2b1 is equal to or greater than MSLD.b2b1 and
	- if SPI1.b3 is equal to or greater than MSLD.b3 and
	- if SPI1.b5b4 is equal to or greater than MSLD.b5b4
	*/
	if(((spi0&0x03)>=(buf[2]&0x03))&&((spi0&0x04)>=(buf[2]&0x04))&&((spi0&0x18)>=(buf[2]&0x18)))
	{
		return POR_OK;
	}
	
	return RFMSecurityLevel;
}
/*@Function:    CheckTpoaEqual                                          
 * 
 *@Descrip:     判断tpoa 是否相同
 *
 *@See:	        ts_143019v050600p.pdf,TS 23.048
 *                                                                                                                                     
 *@Param:        buf
 *@param        :tpoa
 *				                                                                                                         
 *@Return:      TRUE(相同)/FALSE（不同）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131018
 *  
 *@History:     V0.0---Create by XWH 20131018             
 */ 
BYTE CheckTpoaEqual(BYTE * buf,BYTE * tpoa)
{
	return ((buf[0] == tpoa[0])&&(memcmp(&buf[3],&tpoa[3],tpoa[0] -2) == EQUAL));
}
/*
*@description:在当前RFM缓存中查找某特定信息
*
*@param      :mode: 0x00:查找一条空记录	 /返回特定记录号/返回当前短信总数
*@param      :refrence  :批次
*@param      :index   :索引
*@param      :smsType   : TYPE_SMS_ENV_MULTI      0x00	        //Envelope   多条信息 
                          TYPE_SMS_ENV_SINGLE     0x01	 //Envelope   单条信息 
                          TYPE_SMS_UPD_MULTI      0x80	 //Update SMS 多条信息 
                          TYPE_SMS_UPD_SINGLE     0x81	 //Update SMS 单条信息
*@param     BYTE * tpoa
*
*@return   :根据mode处理不同的返回值
*
*@author   :create by xuyz
*
*@history  :V1.0: create by xuyz
*          :V1.1: modify by xuyz 支持不同服务器，同一批次的短信的处理，如果收到的同一服务器，同一批次，则抛弃
*           V1.2: modify by xwh 20171117, 增加MODE_CLEAR_RECORD模式，清除短信头或者短信内容
*/
BYTE ToolkitFindOrClearSpecifiedRFMSMS(BYTE mode ,BYTE refrence,BYTE index,BYTE smsType,BYTE *tpoa)
{
	BYTE xdata tmp[0x20];
	BYTE idata recNum,recLength,i,pos;
	UWORD xdata fileAddr;

	if(mode&MODE_HEADER)
	{
		fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
		pos = 0;
	}
	else
	{
		fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
		pos = 1;
	}

	mode &= 0x7F;

	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	recLength = GetLFRecNumLgth(fileAddr,FileLgth);

	for(i=1 ;i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i,0x00,&tmp[0],sizeof(tmp));

		if(IsBufAllBytesMatch(tmp,4+pos,0x00) == TRUE)
		{
			if(mode == MODE_FIND_NOUSE)
			{
				return i;
			}
		}
		else
		{	
			if(tpoa ==NULL)
			{
				continue;
			}
			if(CheckTpoaEqual(&tmp[3+pos],tpoa)==TRUE)  //服务器地址一致
			{
				if((tmp[0] == smsType)&&(tmp[1] == refrence))
				{
					if(mode == MODE_CLEAR_RECORD)
					{
						SetLFConst(fileAddr,i,0x00,0x00,recLength);
					}				
					else if(mode == MODE_FIND_RECNO)
					{
						if(pos == 0)                                  //Header模式，不比较 index
						{
							return  i;
						}
						else                                             //Data模式，比较Index
						{
							if(tmp[3] == index)
							{
								return  i;
							}
						}
					}
					else
					{
						if(mode == MODE_FIND_TOTAL)  //返回当前批次的短信总数
						{
							return  tmp[2];
						}
					}
				}
			}
		}
	}

	return FALSE;
}
/*
*@description:在当前保存短信头的文件中查找一条空记录,或一条指定记录
*
*@param       :mode: 0x00:空记录 / 0x01:指定记录	
*@param       :invalid           /refrence:  批次号 
*@param       :invalid           /smsType :短信类型	 
×@pram        :BYTE * tpoa  :LV结构
*
*@return      :FALSE/rec
*
*@author      :create by xuyz
*
*@history	  :V1.0:create by xuyz
*/
/*
BYTE ToolkitFindSpecifiedHeaderNum( BYTE mode, BYTE refrence,BYTE smsType,BYTE *tpoa)
{
	BYTE xdata tmp[0x20];
	BYTE idata i,recNo,recNum; 
	UWORD xdata fileAddr;

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);

	if(fileAddr == FALSE)
	{
	   return FALSE;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	recNo = FALSE;

	for(i=1 ;i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i,0x00,&tmp[0],sizeof(tmp));

		//if((tmp[0] == 0x00)&&(tmp[1] == 0x00)&&(tmp[2] == 0x00)&&(tmp[3] == 0x00))
		if(IsBufAllBytesMatch(tmp,4,0x00) == TRUE)
		{
			if(mode == MODE_FIND_NOUSE)
			{
				recNo = i;
				break;
			}
		}
		else
		{
			if(mode == MODE_FIND_NOUSE)
			{
				continue;
			}

			if((tmp[3] == tpoa[0])&&(memcmp(&tmp[6],&tpoa[3],tpoa[0] -2) == EQUAL) )  //若保存的tpoa为  08 0C 81 106540001000，从1065开始比较数据
			{	
				if((mode == MODE_FIND_RECNO)&&(tmp[0] ==smsType)&&(tmp[1] == refrence))
				{
					recNo = i;
					break;
				}
			}
		}
	}

	return recNo;
}
*/
/*
*@description:清除所有SMS文件头和文件内容
*
*@param       :none	 
*
*@return      :none
*
*@author      :create by xwh
*
*@history	  :V1.0:create by xwh 20131120
*/
/*
void ClearSMSHeaderAndData( BYTE refrence,BYTE smsType,BYTE *tpoa)
{
	UWORD xdata fileAddr;
	BYTE  idata recNum,i;
	BYTE  xdata tmp[0x20];

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
	
	if(fileAddr != FALSE)
	{
		recNum =  GetLFRecNumLgth(fileAddr, FileNum);
		
		for(i=1; i<= recNum; i++)	  
		{
			ReadLFBuf(fileAddr, i,0x00,&tmp[0],sizeof(tmp));
			
			if((tmp[0] == TYPE_SMS_ENV_SINGLE)||(tmp[0] == TYPE_SMS_UPD_SINGLE))	   //单条短信，不清除
			{
				continue;
			}
			
			if(ToolkitRFMSMSReceivedAll(tmp[1],tmp[0],&tmp[3]) == TRUE)              //已收满，不清除
			{
				continue;
			}

			if((smsType == TYPE_SMS_ENV_MULTI)||(smsType == TYPE_SMS_UPD_MULTI))     //多条短信情况下，传入的Reference才是有效的
			{
				if(((tmp[3] == tpoa[0])&&(memcmp(&tmp[6],&tpoa[3],tpoa[0]-2) == EQUAL)) && ((tmp[0] == smsType)&&(tmp[1] == refrence)) )    //当前包，不清除
				{
					continue;	
				}
			}
			
			ToolkitClearRFMSms(MODE_CLEAR_RECORD,tmp[1],tmp[0],&tmp[3]);	                 //除以上情况外，可清除

			ToolkitClearRFMSms(MODE_CLEAR_RECORD|MODE_HEADER,tmp[1],tmp[0],&tmp[3]);	
							
			return;			
		}
	}		
}
*/
/*
*@description:在当前RFM缓存中查找某特定信息
*
*@param      :mode: 0x00:查找一条空记录	 /返回特定记录号/返回当前短信总数
*@param      :refrence  :批次
*@param      :index   :索引
*@param      :smsType   : TYPE_SMS_ENV_MULTI      0x00	        //Envelope   多条信息 
                          TYPE_SMS_ENV_SINGLE     0x01	 //Envelope   单条信息 
                          TYPE_SMS_UPD_MULTI      0x80	 //Update SMS 多条信息 
                          TYPE_SMS_UPD_SINGLE     0x81	 //Update SMS 单条信息
*@param     BYTE * tpoa
*
*@return   :根据mode处理不同的返回值
*
*@author   :create by xuyz
*
*@history  :V1.0: create by xuyz
*          :V1.1: modify by xuyz 支持不同服务器，同一批次的短信的处理，如果收到的同一服务器，同一批次，则抛弃
*/
/*
BYTE ToolkitFindSpecifiedRFMSMS(BYTE mode ,BYTE refrence,BYTE index,BYTE smsType,BYTE *tpoa)
{
	BYTE xdata tmp[0x20];
	BYTE idata recNum,recNo,i;
	UWORD xdata fileAddr;

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
	

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	recNo = FALSE;

	for(i=1 ;i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i,0x00,&tmp[0],sizeof(tmp));

		//if((tmp[0] == 0x00)&&(tmp[1] == 0x00)&&(tmp[2] == 0x00)&&(tmp[3] == 0x00)&&(tmp[4] == 0x00))
		if(IsBufAllBytesMatch(tmp,5,0x00) == TRUE)
		{
			if(mode == MODE_FIND_NOUSE)
			{
				return i;

			}
		}
		else
		{
			//mode == MODE_FIND_NOUSE时，tpoa是NULL，此时不应该执行以下比较操作，否则可能代码崩溃,by xwh 20191106
			if(mode == MODE_FIND_NOUSE)
			{
					continue;
			}

			if((tmp[4] == tpoa[0])&&(memcmp(&tmp[7],&tpoa[3],tpoa[0]-2) == EQUAL))   //服务器地址一致
			{
				if((tmp[0] == smsType)&&(tmp[1] == refrence))
				{
					if((mode == MODE_FIND_RECNO)&&(tmp[3] == index))
					{
						return  i;
				
					}
					else
					{
						if(mode == MODE_FIND_TOTAL)  //返回当前批次的短信总数
						{
							return  tmp[2];
						}
					}
				}
			}
		}
	}

	return recNo;
}
*/
#if UICC__CDMA__SMS__DOWNLOAD
/*
*@description:在当前RFM缓存中查找某特定信息
*
*@param      :mode: 0x00:查找一条空记录	 /返回特定记录号/返回当前短信总数
*@param      :refrence  :批次
*@param      :index   :索引
*@param      :smsType   : TYPE_SMS_ENV_MULTI      0x00	        //Envelope   多条信息 
                          TYPE_SMS_ENV_SINGLE     0x01	 //Envelope   单条信息 
                          TYPE_SMS_UPD_MULTI      0x80	 //Update SMS 多条信息 
                          TYPE_SMS_UPD_SINGLE     0x81	 //Update SMS 单条信息
*@param     BYTE * tpoa
*
*@return   :根据mode处理不同的返回值
*
*@author   :create by xuyz
*
*@history  :V1.0: create by xuyz
*          :V1.1: modify by xuyz 支持不同服务器，同一批次的短信的处理，如果收到的同一服务器，同一批次，则抛弃
*/
BYTE ToolkitFindRFMCSMS_Head(BYTE mode ,BYTE refrence,BYTE total,BYTE smsType,BYTE *tpoa)
{
	BYTE xdata tmp[0x35];
	BYTE idata recNum,recNo,i;
	UWORD xdata fileAddr;

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_CSMS_Head);

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	recNo = FALSE;

	for(i=1 ;i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i,0x00,&tmp[0],sizeof(tmp));

		if(IsBufAllBytesMatch(tmp,5,0xFF) == TRUE)
		{
			if(mode == MODE_FIND_NOUSE)
			{
				return i;

			}
		}
		else
		{
			//mode == MODE_FIND_NOUSE时，tpoa是NULL，此时不应该执行以下比较操作，否则可能代码崩溃,by xwh 20191106
			if(mode == MODE_FIND_NOUSE)
			{
				continue;
			}
					
			if(memcmp(&tmp[RFM_GSM_Originating_Address],&tpoa[0],tpoa[0]+1) == EQUAL)   //服务器地址一致
			{
				if((tmp[RFM_CSMS_refrence] == refrence)&&(tmp[RFM_CSMS_TotalNum] == total))   //total  总数     refrence  批次
				{
					if(mode == MODE_FIND_RECNO)
					{
						G_IsCdmaSms = TRUE;

						G_cdmaRecNum = i;

						return  i;				
					}
				}
			}			
		}
	}

	G_IsCdmaSms = FALSE;
	
	return recNo;
}
#endif
/*															    01  recNum
*@description:保存某批次的PP_DOWNLOAD SMS的短信头字段:主要包括 :00  批次,   EVENTID 设备标示  地址标示, 8B-udl
*
*@param1     :refrenceNum:多条短信，该字节为批次号；单条短信，该字节为记录号
*@param2     :eventId	    :EVENT_FORMATTED_SMS_PP_ENV_bit/EVENT_UNFORMATTED_SMS_PP_ENV_bit /0x00
*@param3     :udposOffset:  //指向UDL字段
*@param4     :smsType:    TYPE_SMS_ENV_MULTI      0x00	 //Envelope   多条信息 
                          TYPE_SMS_ENV_SINGLE     0x01	 //Envelope   单条信息 
                          TYPE_SMS_UPD_MULTI      0x80	 //Update SMS 多条信息 
                          TYPE_SMS_UPD_SINGLE     0x81	 //Update SMS 单条信息   
*@param       BYTE *tpoa:LV 结构，字节数
*
*@return     :TRUE/FALSE
*
*@author     :create by xuyz
*
*@history    :V1.0: create by xuyz
*            :V1.1: modify  by XWH
*             V1.2:收到短信时，调用ToolkitClearDifTotalData 函数清除同批次，不同总数的包,防止两个包混淆
*             V1.3: modify by xwh 20170525,修改方案，非标准TLV从6F7F文件移到6F7E文件保存，6F7E文件记录长度可以减小以节省空间，
*                   修改后6F7E文件记录长度设置成0xFF,6F7F文件记录长度设置成0xA0
*/
BYTE  ToolkitSaveRFMHead(BYTE refrence, BYTE eventID,BYTE udposOffset,BYTE smsType, BYTE * tpoa)
{
	BYTE idata emptyNo,ptr1,length,ptr2,ptr3,pos;
	BYTE idata index,total;
	UWORD xdata fileAddr,fileAddrData;
	BYTE idata result,len,ptr,mtiptr;
	BYTE idata refOffset,recLgth;

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
	
	if(fileAddr == FALSE)
	{
	   return FALSE;
	}	   

	memset(tempBuf, 0x00 ,sizeof(tempBuf));
	
	if((smsType==TYPE_SMS_ENV_SINGLE)||(smsType==TYPE_SMS_UPD_SINGLE))	 //对于单条短信,此文件只更新一次,在更新此文件时,要先获取缓存记录文件中的空闲记录
	{
		index = 0x01;

		emptyNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_NOUSE, 0x00, 0x00,NULL);		
	}
	else    //对于多条短信, 先查找之前是否已经有同批次的信息更新  ,否则查找一条空记录进行更新
	{
		pos = FindTagFullMatch(&RecData[udposOffset+2],0x00,RecData[udposOffset+1]);
		
		refOffset = udposOffset + 2 + pos + 1;
	
		if(RecData[refOffset+1] > RecData[refOffset+2])   //下行报文为 ：批次 、短信总数 、短信索引格式，主要为标准的03.48格式报文
		{
			index = RecData[refOffset+2];
			
			total = RecData[refOffset+1];	
		}
		else  //下行报文格式为:批次 、短信索引、短信总数 ，主要为移动OTA报文，如移动远程文件管理、远程写卡等
		{
			index = RecData[refOffset+1];
			
			total = RecData[refOffset+2];	
		}

		ToolkitClearDifTotalData(RecData[refOffset],total,tpoa); //对于同一个服务器来说 收到A50301时清掉A50808包（批次相同，总条数不同，未收满的包）
		
		result = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,RecData[refOffset],index,smsType,tpoa);  //判断当前短信是否已收到过

		if(result != FALSE)		//当前短信已收到
		{
			fileAddrData = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

			if(fileAddrData != FALSE)
			{
				SetLFConst(fileAddrData,result,0,0x00,0x05);
			}
		}
		
		emptyNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_RECNO,refrence,smsType,tpoa);
		
		if(emptyNo == FALSE)  //之前没有更新过同批次的信息
		{
		  emptyNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_NOUSE,0x00,0x00,NULL);  //该短信为该批次的第一条短信,找到一条空记录进行更新
		}
		else
		{
			//sim api测试中 framework fwk_csm_proc。对于同一批次的如果两条短信的86 tag不一样，则以最新一条收到的为主
		}
	}

	if(emptyNo == FALSE)   
	{	
		//ClearSMSHeaderAndData(smsType,refrence,tpoa);		//如果头文件已存满，选择一个没有收满的非当前包，清除头文件和对应的数据文件
		ToolkitClearPackage(refrence,smsType,tpoa);	      //如果头文件已存满，选择一个没有收满的非当前包，清除头文件和对应的数据文件

		emptyNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_NOUSE,refrence,smsType,tpoa);

		if(emptyNo == FALSE) 
		{
			return FALSE;
		}
	}

	if((smsType==TYPE_SMS_UPD_MULTI)||(smsType==TYPE_SMS_UPD_SINGLE))
	{
		len = 0;
	
		tempBuf[18] = TLV_DEVICEIDENTITY; 
		tempBuf[19] = 0x02;
		tempBuf[20] = P1;                                     //Absolute Record Number
		tempBuf[21] = RecData[0];                            //Record Status
		len += 4;

		tempBuf[18+len] = TLV_ADDRESS; 
		
		memcpy(&tempBuf[18+len+1],&RecData[1],RecData[1]+1);      //SMSC
		
		len += (1+RecData[1]+1);

		ptr = 0x01;                                             //to SMSC length
		
		ptr += (RecData[ptr]+1);                                //to MTI
		
		mtiptr = ptr;
		
		ptr ++;        
		//to TPOA字符个数
		ptr += ((RecData[ptr]/2)+(RecData[ptr]%2)+1+1);        //to PID
		
		ptr += (2+7);    


		//针对UISM而言，如果当前更新的文件为某个ADF下面的某个文件，则还需要拼装AID TLV@TS 131130  6.2  ,这里的代码需要补充
		//The AID comprehension TLV is present only if the EFSMS file updated is under an ADF.
		//question :什么时候该拼装该AID TLV呢？？

		tempBuf[18+len] = TLV_SMSTPDU;
		
		tempBuf[18+len+1] =  ptr - mtiptr;
		
		memcpy(&tempBuf[18+len+2],&RecData[mtiptr],ptr - mtiptr);
		
		len += (1+1+(ptr - mtiptr));

		tempBuf[17] = len;   
	}
	else
	{
		ptr1 = FindTag(&RecData[0],0xD1,P3);	   //point to length(0x01 or 0x02)

		ptr2 =  FindTag(&RecData[ptr1+1],0x8B,RecData[ptr1]);	  

		ptr2 += ptr1+1;                                         //point to 0x8B length

		if(RecData[ptr2] > 0x7F)
		{
			ptr3 = 0x02;
		}
		else
		{
			ptr3 = 0x01;
		}

		tempBuf[17] = ptr2-ptr3-1-ptr1;

		memcpy(&tempBuf[18], &RecData[ptr1+1],tempBuf[17]);	//保存8B之前的tag信息,"8202838106058011223344"

		tempBuf[18+tempBuf[17]] = RecData[ptr2-ptr3];  //指向tag 8Bor 0B位置
		
		tempBuf[17] ++;  

		length =  RecData[ptr2] -RecData[udposOffset] - 1;   //8B当前数据的实际长度,保存数据到TP_SCTS 的位置

		tempBuf[18+tempBuf[17]] = length;

		tempBuf[17] ++; 

		memcpy(&tempBuf[18+tempBuf[17]],  &RecData[ptr2+1],length);

		tempBuf[17] += length;    	
	}

	tempBuf[1]  = smsType;

	tempBuf[2] =  refrence;     	//单条---6F7F中记录号  多条---reference number                                                     

	ReadLFBuf(fileAddr,emptyNo,0x02,&tempBuf[3] ,sizeof(BYTE));  //获取文件中存储的EVENTID，只有当收到的短信索引为01时，才对它进行更改

	if(index == 0x01)
	{
		tempBuf[3] = eventID;
	}
	
	memcpy(&tempBuf[4], tpoa,tpoa[0]+1);   //拷贝tpoa
	
	tempBuf[0] = tempBuf[17]+4+13;

	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);	

	if((tempBuf[0] + 1) > recLgth)          //保留最后一字节作为非标准TLV的长度	
	{
		return FALSE;
	}
	
	SetLFConst(fileAddr,emptyNo,recLgth-1,0x00,sizeof(BYTE)); //初始化非标准TLV的长度为00

	result = WriteLFBuf(fileAddr,emptyNo,0x00,&tempBuf[1],tempBuf[0]);

	if(result == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}
/*
*@description:清除当前某个批次的信息
*
*@param      :refrence:仅针对带refrence的情况
*@param      :smsType:短信类型
*@param      :BYTE * tpoa
*
*@return     ：void
*
*@author    :create by xuyz    
*
*@history   :v1.0 create by xuyz
*/
/*
void ToolkitClearRFMSms(BYTE refrence,BYTE smsType,BYTE mode,BYTE *tpoa)
{
	BYTE xdata tmp[0x20];
	BYTE idata i,recNum,tpoaOffset,recLgth;
	UWORD xdata fileAddr;

	if(mode == HEADER_MODE)
	{
		fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
		
		tpoaOffset = 0x03;
	}
	else
	{
		fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
		
		tpoaOffset = 0x04;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);

	for(i= 0x01; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr,i, 0x00,&tmp[0],sizeof(tmp));

		//if((tmp[0] != 0x00)||(tmp[1] != 0x00)||(tmp[2] != 0x00)||(tmp[3] != 0x00)||(tmp[4] != 0x00))
		if(IsBufAllBytesMatch(tmp,5,0x00) == FALSE)
		{
			if((tmp[tpoaOffset] == tpoa[0])&&(memcmp(&tmp[tpoaOffset+3],&tpoa[3],tpoa[0] -2) == EQUAL) ) 
			{
				if((tmp[0] == smsType)&&(tmp[1] == refrence))
				{
					SetLFConst(fileAddr,i,0x00,0x00,recLgth);  //清除整条记录的长度，因为对于EF_ToolkitRFMSms文件，记录结束处可能还缓存有其他数据
				}
			}
		}		
	}
}
*/
/*
*@description:查找和当前批次不一样或者短信类型不一样，且未收齐的短信
*
*@param      :diffcurrentRefrence:当前批次
*@param	     :smsType            :当前短信类型
×@param       :BYTE *tpoa   :LV结构 
*
*@return     :void
*
*@author    :create by xuyz
*
*@history   :V1.1 : -create by xuyz
*            V1.2 : modify by xuwh,增加类型不一样的短信的判断
*/
void ToolkitClearPackage(BYTE diffcurrentRefrence,BYTE smsType, BYTE *tpoa)
{
	BYTE xdata tmp[0x20];
	BYTE idata i,recNum;
	UWORD xdata fileAddr;

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i= 0x01; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr,i, 0x00,&tmp[0],sizeof(tmp));
		
		if(IsBufAllBytesMatch(tmp,5,0x00) == TRUE)
		{
			continue;
		}
		
		if((tmp[0] == TYPE_SMS_ENV_SINGLE)||(tmp[0] == TYPE_SMS_UPD_SINGLE))	   //单条短信，不清除
		{
			continue;
		}
		
		if(ToolkitRFMSMSReceivedAll(tmp[1],tmp[0],&tmp[4]) == TRUE)              //已收满，不清除
		{
			continue;
		}
		
		if(CheckTpoaEqual(&tmp[4],tpoa)==TRUE)  //TPOA相同,批次号相同，类型相同，是同一个包，不清除
		{
				if((tmp[1] == diffcurrentRefrence)&&(tmp[0] == smsType))  
				{
					continue;
				}
		}
		
		ToolkitClearRFMSms(MODE_CLEAR_RECORD,tmp[1],tmp[0],&tmp[4]);	                 //除以上情况外，可清除

		ToolkitClearRFMSms(MODE_CLEAR_RECORD|MODE_HEADER,tmp[1],tmp[0],&tmp[4]);	
							
		return;			
	}	
}
/*
*@description:判断某批次的短信是否收全
*
*@param     :refrence ：批次号
*@param     :smsType ：短信类型
*@param      :tpoa  :Lv结构 
*
*@return 	:TRUE:当前批次收满/false :当前批次未收满
*
*@author    :create by xuyz
*
*@history   :v1.0 create by xuyz
*/
BYTE ToolkitRFMSMSReceivedAll(BYTE refrence,BYTE smsType,BYTE *tpoa)
{
	 BYTE idata total,i;

	 total =  ToolkitFindSpecifiedRFMSMS(MODE_FIND_TOTAL,refrence,0x00,smsType,tpoa); //找到该类型短信对应的批次的短信总条数

	 if(total == 0x00)
	 {
	 	return FALSE;
	 }

	 for(i =1; i<= total; i++)
	 {
	 	if(ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,refrence,i,smsType,tpoa) == FALSE)	   //判断每一条索引数据是否收全
		{
			return FALSE;
		}
	 }

	 return TRUE;
}
/*
*@description: 获取下行数据的spi1 spi2 kic kid tar1 tar2 tar3
*
*@function   :  GetEnvelopFirstRecTar
*
*@param      :  firstNo
*@param      :BYTE *spi
*
*@return
*/
void GetEnvelopFirstRecTar(BYTE firstRecNo, BYTE *spi)
{
	UWORD xdata fileAddr;
	BYTE idata cplLength;

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

	RecData[0] = ReadLFFileValue1ByteLen(fileAddr,firstRecNo,17,&RecData[1]);

	cplLength = ToolkitGetCPLLength(RecData[1]);
	
	memcpy(&spi[0], &RecData[2+cplLength], 0x07);  //spi1 spi2 kic kid tar1 tar2 tar3
}
/*
*@function    :ToolkitEnvelopIsSubmitTar
*
*@description :判断当前接受的数据的tar是否为RFM/RAM tar,如果是Toolkit 数据，则需要立即处理；对于RFM/RAM 不是submit的才需要立即处理，其他的打标记处理
*
*@param       :smsType
*@param       :reference or recNo
*
*@return      :TRUE:下行短信要通过打标记的方式去处理，主要是为了支持在STK中处理RFM /RAM
               FALSE:下行短信可以立即处理，如在STK 中收到了不需要发送sumbit短信的RFM/RAM报文，或者在STK中收到了 TOOLKIT报文
*
*@author      :xuyz
*
*@history    :V1.0 :create by xuyz@20131216
			  V1.1  修改原因 
			   在以下测试用例中
               1  :移动RFM&RAM测试脚本_2G3G结合/移动RFM测试(2G)/5.16_SecurityMechanismResponsePacket  ，步骤51 52
               2：  MultiChannel_20150330_明文版/东信 13g 
			   要求在一个C2产生的STK 中能够处理新的C2命令，要注意备份C2数据的文件大小应该一致才

			  v1.3 :由于下行数据存储的方案增加了存储TPOA字段进行完全匹配 ，因此修改该函数的参数为缓存数据的 第一条记录号@by xuyz
*/
BYTE ToolkitEnvelopIsSubmitTar(BYTE firstRecNo)
{
#if UICC__FUNCTION__STKAPI		
	BYTE idata tarindex;
#endif	
	
	BYTE xdata spi[0x07];
	
	GetEnvelopFirstRecTar(firstRecNo, spi);
#if UICC__OPERATOR__CHINAMOBILE
	if(IF_CheckChinaMobileTar(&spi[4]) == TRUE)
	{
		return TRUE;  //移动报文格式的tar统一采用打标记 处理的方式 
	}
#endif
#if UICC__FUNCTION__STKAPI			
	tarindex = CheckTar(&spi[4],NULL);
#else
    CheckTar(&spi[4],NULL);
#endif	

	//tarindex = TarType_Error 时发送submit POR 也要打标记处理，否则在STK 中发送POR 会造成重入
#if UICC__FUNCTION__STKAPI	
	if(tarindex != TarType_ToolkitApplet)  //rfm,ram,wib
#endif	
	{
		if((spi[1]&0x20) == 0x20)  //需要发送submit POR,那么通过打标记去处理
		{
			return TRUE;  
		}
		else
		{
			return FALSE;  //直接处理下行
		}	
	}
#if UICC__FUNCTION__STKAPI	
	else
	{
		return FALSE;
	}
#endif	
}
/*
*@function     :ToolkitClearAllData
*
*@description  :清除当前批次的所有数据，包括头和数据体之类的
*
*@param        :smsType
*@param        :reference
*
*@return       :void
*
*@author       :xuyz
*
*@history      :V1.0 :create by xuyz
*/
void ToolkitClearAllData(BYTE smsType, BYTE reference,BYTE * tpoa)
{
	UWORD xdata fileAddr;
	BYTE idata headNo,recLgth;

	headNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_RECNO,reference,smsType,tpoa);  //找到当前短信在header中的记录号，可以根据记录号，获取eventid

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
	
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);

	SetLFConst(fileAddr,headNo,0x00,0x00,recLgth);

	if((smsType == TYPE_SMS_ENV_SINGLE)||(smsType == TYPE_SMS_UPD_SINGLE))
	{
		fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
		
		recLgth = GetLFRecNumLgth(fileAddr,FileLgth);

		SetLFConst(fileAddr,reference,0x00,0x00,recLgth);	    //清除记录文件中缓存的RFM信息
	}
	else
	{
		ToolkitClearRFMSms(MODE_CLEAR_RECORD,reference,smsType,tpoa);	 //清除当前批次信息
	}
#if UICC__CDMA__SMS__DOWNLOAD  
    if(G_cdmaRecNum !=FALSE)
    {        
        fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_CSMS_Head);
        SetLFConst(fileAddr,G_cdmaRecNum,0x00,0xFF,0X05);	    //清空前五个字节查找的时候比较前五个字节    
    }
#endif    
}
/*
*@function: ToolkitClearDifTotalData
*
8@description :清除同一个服务器中，同一批次，不同短信条数的数据
*
*@param       :reference
*@param       :BYTE *tpoa
*
*@return      :void
*/
void ToolkitClearDifTotalData(BYTE reference, BYTE total, BYTE *tpoa)
{
	UWORD xdata fileAddr;
	BYTE idata recNum, i,recLgth;
	BYTE xdata tmp[0x20];
	
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

	if(fileAddr == FALSE)
	{
		return ;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);
	
	for(i= 0x01; i<=recNum; i++)	 	//对于同一个服务器来说 收到A50301时清掉A50808包（批次相同，总条数不同，未收满的包）
	{
		ReadLFBuf(fileAddr,i, 0x00,&tmp[0],sizeof(tmp));
		
		if(IsBufAllBytesMatch(tmp,4,0x00) == TRUE)
		{
		}
		else
		{
			if(CheckTpoaEqual(&tmp[4],tpoa)==TRUE)
			{
				if((tmp[1] == reference)&&(tmp[2] != total)&&(tmp[2] != 0x01))	//批次相同，总条数不同，并且不是单条的
				{		
					SetLFConst(fileAddr,i,0x00,0x00,recLgth);		
				}
			}
		}	
	}		
}
/*
*@description:保存当前RFM短信: 该文件的存储格式为: 批次 短信总数 短信索引  ,后续数据长度,后续数据
*
*@param      :udPos:udl处的数据偏移
*@param      :对于单条短信来说,为缓存文件中的记录号
*@param      :smsType: TYPE_SMS_ENV_MULTI   TYPE_SMS_UPD_MULTI	TYPE_SMS_ENV_SINGLE	 TYPE_SMS_UPD_SINGLE
*@param      :BYTE *tpoa :LV结构
*
*@return     :FALSE/TRUE
*
*@author     :create by xuyz@20131011
*
8@history	 :V1.0: create by xuyz
*			  v1.1: modidy by XWH
*             V1.2:modify by xwh 20170525,修改方案，非标准TLV从6F7F文件移到6F7E文件保存，6F7E文件记录长度可以减小以节省空间,
*                  修改后6F7E文件记录长度设置成0xFF,6F7F文件记录长度设置成0xA0
*/
BYTE ToolkitSaveRFMSMS(BYTE udPos,BYTE recNo,BYTE smsType,BYTE *tpoa)
{
	BYTE idata result,len,pos;
	BYTE idata isSingleFlag;
	UWORD xdata fileAddr;
	BYTE idata lgth,ptr,initptr;
	
	BYTE idata index, total;
	BYTE idata refOffset;
#if UICC__FUNCTION__STKAPI   	
	BYTE  idata headRecLgth,dataLgth,headNo,remainLen;
	WORD  idata totalLen;
	UWORD xdata headFileAddr;
#endif	
	
	
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	isSingleFlag = FALSE;
	
	memset(tempBuf, 0x00, sizeof(tempBuf));
	
	pos = FindTagFullMatch(&RecData[udPos+2],0x00,RecData[udPos+1]);

	//if((RecData[udPos+1] == 0x07)||(RecData[udPos+1] == 0x05))     //在保存短信头时已经判断过当前批次的该短信是否已收到过			
	if(pos != FALSE)
	{
		refOffset = udPos + 2 + pos + 1;
		
		if(RecData[refOffset+1] > RecData[refOffset+2])  //标准RFM格式 ：批次、短信总数、短信索引
		{
			index = RecData[refOffset+2];
            
			total = RecData[refOffset+1];
		}
		else  //批次、索引、总数 ，移动的RFM及现场写卡报文
		{
			index = RecData[refOffset+1];
            
			total = RecData[refOffset+2];
		}
		
		ToolkitClearDifTotalData(RecData[refOffset],total,tpoa); //对于同一个服务器来说 收到A50301时清掉A50808包（批次相同，总条数不同，未收满的包）
		
		recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_NOUSE,0x00,0x00,0x00, NULL);  //在缓存文件中查找一条空记录

		if(recNo == FALSE)  //当前缓存已满
		{		
			ToolkitClearPackage(RecData[refOffset],smsType,tpoa);		//todo:清除批次不一样的未收满的短信信息			

			recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_NOUSE,0x00,0x00,0x00, NULL);  //在缓存文件中查找一条空记录

			if(recNo == FALSE)
			{		
				return FALSE;
			}			
		}

		tempBuf[1] = smsType;	 //用于区分当前缓存中的数据带不带批次号
		
		tempBuf[2] = RecData[refOffset];
        
		tempBuf[3] = total;
        
		tempBuf[4] = index;

	}
	else  //单条format短信，即短信头尾027000
	{
		tempBuf[1] = smsType;	//用于区分当前缓存中的数据带不带批次  ，0x01 不带批次号
        
		tempBuf[2] = recNo;
        
		tempBuf[3] = 0x01;
        
		tempBuf[4] = 0x01;

		isSingleFlag = TRUE;
	}

	memcpy(&tempBuf[5],tpoa,tpoa[0]+1);
	
	len =13+ 4 + 1 + RecData[udPos] - (RecData[udPos+1] + 1);                  // length of 00(01) refNumber(recNo),totalNumber seqNumber,L,CPL to data end

	tempBuf[18] = len -5-13;

	udPos = RecData[udPos+1] + udPos+2;

	memcpy(&tempBuf[19],&RecData[udPos],tempBuf[18]);

	result = WriteLFBuf(fileAddr,recNo, 0x00, &tempBuf[1],len);

	if(result == FALSE)
	{
		return FALSE;
	}

	if(RecData[0] == TAG_SMSPP_DOWNLOAD)
	{
		lgth = (RecData[1]==0x81?RecData[2]:RecData[1]);
        
		initptr = (RecData[1]==0x81?3:2);	                                   //initptr指向第一个TLV tag

		ptr = FindTag(&RecData[initptr],TLV_SMSTPDU,lgth) + initptr;       //ptr指向SMSTPDU length	
        
		ptr += (RecData[ptr]+1);	                                       //ptr指向SMSTPDU之后第一个TLV tag
#if UICC__FUNCTION__STKAPI
		if(lgth > (ptr-initptr))                                           //02/06/0B以外的其他TLV存在        	
		{
			remainLen = lgth - (ptr-initptr);                              //02/06/0B以外的其他TLV总长度
			
			headFileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
			
			headNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_RECNO,tempBuf[2],smsType,tpoa);          //找到当前短信在header中的记录号
			
			headRecLgth = GetLFRecNumLgth(headFileAddr,FileLgth);
								
			ReadLFBuf(headFileAddr,headNo,headRecLgth-1, &unFormatLgth,sizeof(BYTE));                 //最后一字节，02/06/0B以外的其他TLV总长度	
			
			ReadLFBuf(headFileAddr,headNo,16, &dataLgth,sizeof(BYTE));
			
			dataLgth += (4+13+1);                                                                     //02/06/0B数据长度
			
			totalLen = dataLgth + (unFormatLgth + 1) + remainLen;
			
			if(totalLen <= headRecLgth)                                                               //文件记录长度足够保存本条短信中的其他TLV，那么保存
			{
				WriteLFBuf(headFileAddr,headNo, headRecLgth-1-unFormatLgth-remainLen,&RecData[ptr],remainLen);
			
				unFormatLgth += remainLen;
			
				SetLFConst(headFileAddr,headNo,headRecLgth-1,unFormatLgth,sizeof(BYTE));
			}						
		}
#endif		
	}

	result = FALSE;

	if(tempBuf[3] != 0x01)
	{
		result = ToolkitRFMSMSReceivedAll(tempBuf[2],smsType,tpoa);
	}

	if((tempBuf[3] == 0x01)||(result == TRUE))  //短信总数为1
	{
		if(isSingleFlag == TRUE)   //打标记，保存记录号
		{		
			if((smsType != TYPE_SMS_UPD_SINGLE)&&(ToolkitEnvelopIsSubmitTar(recNo) == TRUE))	  	//SMS_UPD直接处理，不打标记
			{
				PendingMessageAlert(TYPE_SMS_ENV_SINGLE,recNo);
			}
			else
			{
				ToolkitRFMExcute(smsType,recNo);
			}
		}
		else
		{
			recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,tempBuf[2],0x01,smsType,tpoa);	  //对于多条短信来说，找到当前批次的第一条记录
		
			if((smsType != TYPE_SMS_UPD_MULTI)&&(ToolkitEnvelopIsSubmitTar(recNo) == TRUE))		//SMS_UPD直接处理，不打标记
			{
				PendingMessageAlert(TYPE_SMS_ENV_MULTI,recNo);
			}
			else
			{
				ToolkitRFMExcute(smsType,recNo);
			}
		}
	}
	else
	{
		
	}
	return TRUE;
}
/*
*@description:拼装短信header
*
*@param      :headerNo:记录号，当前批次的短信头信息在6F71中的记录号
*@param      :WORD udlLength 
*@param      :mode :是否要添加027000：0x01：添加/0x00：不添加
*
*@return     :EnvelopBufferOffset
*
**@author    :create by xuyz
*
*@history   :v1.0 create by xuyz
*/
WORD ToolkitProcessHeader(BYTE headerNo,WORD udlLength,BYTE mode)
{
	BYTE idata tp_udl,mtiByte,lenBytes;
	BYTE idata ptr;
	WORD idata offset,blength;
	UWORD xdata fileAddr;	
#if UICC__CDMA__SMS__DOWNLOAD
	UWORD xdata csmsfileAddr;
	BYTE  idata referNum;
	BYTE xdata  tpoa[0x10];
	BYTE xdata  tmp[0x35];
	BYTE idata  userDataLen;
    BYTE idata  Note3length;
#endif
	
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);    //先拼装短信头部分
	
#if UICC__CDMA__SMS__DOWNLOAD
	
	referNum = FALSE;
	
	csmsfileAddr = SelectPrivateLFFile(DF_Toolkit,EF_CSMS_Head);    //cmsm短信头文件
	
	if(csmsfileAddr != FALSE)
	{
		ReadLFBuf(fileAddr,headerNo,0x00,&tempBuf[0],0x02);      //读类型 和批次
		
		if((tempBuf[0] == TYPE_SMS_ENV_SINGLE) || (tempBuf[0] == TYPE_SMS_UPD_SINGLE))
		{
			tempBuf[0] = 0X01;
			
			tempBuf[1] = 0X01;
		}
		else
		{
		  tempBuf[0] = G_cdmaSmsNum;
		}
		
		tpoa[0] = ReadLFFileValue1ByteLen(fileAddr,headerNo,0x03,&tpoa[1]);

		referNum =ToolkitFindRFMCSMS_Head(MODE_FIND_RECNO ,tempBuf[1],tempBuf[0],0x00,tpoa);
		
		if(referNum != FALSE)
		{
		
		}
	}
	
#endif

	tempBuf[0] = ReadLFFileValue1ByteLen(fileAddr,headerNo,16,&tempBuf[1]);

	ptr = FindTag(&tempBuf[1],TLV_SMSTPDU,tempBuf[0]);  //找到8Btag

	mtiByte = tempBuf[1+ptr+1];

	if(mode == 0x00)  //unformatted SMS
	{
#if UICC__CDMA__SMS__DOWNLOAD
        if(G_cdmaSmsNum !=0)
        {
            //  C网短信转G网短信的时候 把 UDHI固定写成了 40， 实际上 C网短信数据中是没有这个字节的  
        }
        else
#endif 
        {
			if(Check_UDHI(mtiByte)==TRUE)  	//根据测试例子Test_Ufw_Csm_Proc, 对于 unformatted SMS 且 udhi 存在，需要在udl 后添加一个字节udhl = 0x00,by xwh 20140214
			{
				udlLength++;
			}
        }
	}

	if(udlLength >= 0x100)	 //根据规范描述，TP_UDL仅占一个字节；see 43.019 or 131130：The original Short Message shall be placed in one SMS TPDU TLV(with TP-UDL field coded on one octet) included in the USATEnvelopeHandler
	{
		tp_udl = 0xFF;
	}
	else
	{
		tp_udl = udlLength;
	}

	offset = 0x00;
#if UICC__CDMA__SMS__DOWNLOAD
	if(referNum != FALSE)
	{
		ReadLFBuf(csmsfileAddr,referNum,0x00,&tmp[0],sizeof(tmp));
		
		memcpy(&RecData[offset],&tmp[RFM_CSMS_DeviceIdentities],0x04);
		
		offset += 0x04;
	}
	else
#endif
	{	
		memcpy(&RecData[offset],&tempBuf[1],(ptr-1));  	//-1(减去首偏移)，+1（加上一字节长）

		offset = ptr-1;
	}

	
#if UICC__CDMA__SMS__DOWNLOAD
	if(referNum != FALSE)
	{
		RecData[offset] = (0xC8&0x7F);		 //  拼凑CSIM API需要的数据	
	}
	else
#endif
	{
		RecData[offset] = tempBuf[ptr];		 //获取8Btag
	}

	offset ++;

#if UICC__CDMA__SMS__DOWNLOAD
	if(referNum != FALSE)
	{
        if((udlLength+1) >0xFF)
        {
            Note3length = 0x03;          //82 XX XX
        }
        else if((udlLength+1) >0x7F)
        {
            Note3length = 0x02;          //81 XX
        }
        else
        {
            Note3length = 0x01;        //XX  就一个字节长度
        }
        blength = udlLength + 1 + Note3length + 1 + 3 + 5 + 2 + tmp[RFM_CSMS_Originating_Address+1] + 2 +4 + 1;
		//udlLength + 1 + Note3length + 1  整个1 tag长度  3 + 5 + 2   ：08 tag的  
	}
 else
#endif
	{
		blength = tempBuf[ptr+1] + udlLength +1;  //8B后续数据长度
	}
	
	lenBytes = SetBerTlvLength(&RecData[offset],blength);
	offset += lenBytes;	

	G_formatedTPOAOffset = offset+1; //下行短信的MTI在文件6F90 中的偏移
#if UICC__CDMA__SMS__DOWNLOAD
	if(referNum != FALSE)
	{		
		memcpy(&RecData[offset],&tmp[RFM_CSMS_MessageType],0x05);  //SMS Point to Point + Teleservice Identifier
		
		offset += 0x05;
		
		memcpy(&RecData[offset],&tmp[RFM_CSMS_Originating_Address],(tmp[RFM_CSMS_Originating_Address +1]+2)); //Originating Address  这里存的数据  为原始数据 TLV结构
		
		offset += (tmp[RFM_CSMS_Originating_Address +1 ]+2);
		
		RecData[offset++] = 0x08;       //Bearer
				
// 		RecData[offset++] = (udlLength + 3 + 5 + userDataLen +2)&0xFF;  //Bearer  Length ; 3 (Message Display Mode  ) 5 (Message Identifier) +userDataLen(01 Tag LEn) Bearer  Length 此处长度为一个字节，单挑短信则为实际长度，级联的话 无意义，保持数据的一致性   
		RecData[offset++] = tmp[RFM_CSMS_LAST_BEARER_LEN];
		memcpy(&RecData[offset],&tmp[RFM_CSMS_Message_Identifier],0x8);
        offset += 0x08;
		
		RecData[offset++] = 0x01;    //User Data Tag
		
		lenBytes = SetBerTlvLength(&RecData[offset],udlLength+1);
		offset += lenBytes;	
		
		RecData[offset++] = ((tmp[RFM_CSMS_MSG_ENCODING] >>3) & 0x1F); //MSG_ENCODING  保留低5位数据
  }
  else
#endif
  {
	memcpy(&RecData[offset],&tempBuf[ptr+2],tempBuf[ptr+1]);
	
	offset +=  tempBuf[ptr+1];

	RecData[offset] = tp_udl;

	offset ++;
  }

	
	if(mode == 0x01)  //formatted SMS,添加 027000
	{		
		RecData[offset] = 0x02;

		RecData[offset+1] =  0x70;

		RecData[offset+2]  = 0x00;	

		offset += 3;
	}
	else                      //unformatted SMS
	{
		//根据测试例子Test_Ufw_Csm_Proc, 对于 unformatted SMS 且 udhi 存在，需要在udl 后添加一个字节udhl = 0x00,by xwh 20140214
		if(Check_UDHI(mtiByte)==TRUE)
		{
			RecData[offset] = 0x00;
			offset++;
		}
	}
	
	WriteTFBuf(G_EnvelopHanderAddr,0x00,&RecData[0],offset); //写数据“82028381......027000”

	return offset;
}
/*
*@description: 保存format SMS PPDowload数据
*
*@param      :udPos	 :UD 数据在RecData中的偏移
*@param      :eventType :EVENT_FORMATTED_SMS_PP_UPD_bit	/EVENT_FORMATTED_SMS_PP_ENV_bit
*@param      :BYTE *tpoa: LV结构
*
*@return     :FALSE/TRUE
*
*@author     :create by xuyz
*
*@history    :V1.0 :create by xuyz
*            :V1.1 :modify by xuyz, 增加参数tpoa，LV结构，其中V 的第一字节为TPOA类型，不参与比较 ，该修改主要用于对于多条短信情况下，区分标准RFM 和移动RFM 的处理，
*/
BYTE ToolkitSaveFirstFomatData(BYTE udPos,BYTE eventType,BYTE *tpoa)
{
	BYTE idata recNo,result,pos;
	BYTE  idata smsType,refNum;

	recNo = FALSE;
	
	pos = FindTagFullMatch(&RecData[udPos+2],0x00,RecData[udPos+1]);
	
	//用是否有00 tag(Concatenated short messages, 8-bit reference number)判断是否是级联，不用长度07判断 
	//if(RecData[udPos+1] == 0x07)	
	if(pos != FALSE)
	{
		smsType = ((RecData[0] == TAG_SMSPP_DOWNLOAD)?TYPE_SMS_ENV_MULTI:TYPE_SMS_UPD_MULTI);
		
		refNum = RecData[udPos+2+pos+1];   //为兼容多条中不同UDH的情况,如SIMEID的需求
		
		result = ToolkitSaveRFMHead(refNum,eventType,udPos,smsType,tpoa);
	}
	else   //单条format短信
	{
		smsType = ((RecData[0] == TAG_SMSPP_DOWNLOAD)?TYPE_SMS_ENV_SINGLE:TYPE_SMS_UPD_SINGLE);
		
		recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_NOUSE,0x00,0x00,0x00, NULL);  //查找一条空记录		

		if(recNo == FALSE)
		{		
			ToolkitClearPackage(0,smsType,tpoa);		//todo:清除批次不一样的未收满的短信信息	,单条短信，Reference = 0		

			recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_NOUSE,0x00,0x00,0x00, NULL);  //查找一条空记录

			if(recNo == FALSE)
			{
				return FALSE;
			}
		}

	   	result = ToolkitSaveRFMHead(recNo,eventType,udPos, smsType,tpoa);
	}	

	if(result == FALSE)	 //保存短信头的文件不存在或更新文件时失败
	{
		return FALSE;
	}
	ToolkitSaveRFMSMS(udPos,recNo,smsType,tpoa);  //recNo仅对单条短信有效

	return TRUE;
}
/*
*@description:拼装unformat短信到envelophander buf中去
*
*@param      :refrence： 批次号
*@param  	 :headerNo: 6F71中的记录号
*@param      : smsType:短信类型
*
*@return     ：void
*
*@author    :create by xuyz
*
*@history   :v1.0 create by xuyz
*/
WORD ToolkitPatchUnformatENVSMS(BYTE refrence, BYTE headerNo,BYTE smsType,BYTE *tpoa)
{
	BYTE idata total,i,recNo,length;
	WORD idata offset,udlLength;
	UWORD xdata fileAddr;
	WORD idata cplDataOffset;
#if UICC__CDMA__SMS__DOWNLOAD	
    BYTE idata cdmarecNo;
#endif

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
	
	total = ToolkitFindSpecifiedRFMSMS(MODE_FIND_TOTAL,refrence,0x00,smsType,tpoa);
	
#if UICC__CDMA__SMS__DOWNLOAD		
		cdmarecNo = ToolkitFindRFMCSMS_Head(MODE_FIND_RECNO ,refrence,total,smsType,tpoa);
		if(cdmarecNo != FALSE)
		{
			G_cdmaSmsNum = total;		
		}
#endif
	udlLength = 0x00;

	for(i=1; i<=total; i++)	    //先计算数据体的总长度
	{
		recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,refrence,i,smsType,tpoa);     //查找第n条短信

		ReadLFBuf(fileAddr,recNo,17,&tempBuf[0] ,sizeof(BYTE));

		udlLength += tempBuf[0];
	}

	offset = ToolkitProcessHeader(headerNo,udlLength,0x00);
	
	cplDataOffset = offset;

	//开始拼装数据体

	for(i=1; i<=total; i++)	    //先计算数据体的总长度
	{
		recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,refrence,i,smsType,tpoa);     //查找第n条短信

		length = ReadLFFileValue1ByteLen(fileAddr,recNo,17,&tempBuf[0]);

		WriteTFBuf(G_EnvelopHanderAddr,offset,&tempBuf[0],length);

		offset += length;
	}

	ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = offset;
	
	return cplDataOffset;
}

/*
*@escription:解密当前的RFM数据  传进来的是E2地址
*
*@param     :KIc    :密钥索引
*@PARAM     :pointer :原始数据,
*@param     :result_pointer	:加密或解密后的数据
*@mode      :mode : Encry or decry
*@param     :length	:加密或解密的数据长度,一定为8的倍数
*
*@return    :TRUE/FALSE
*
*@author    :modify by xuyz
*
*@history   :create by xuyz@20131011
*           :LCZ  添加DES 解密模式
*
*/
BYTE ToolkitDeChipherEe(BYTE kic,UWORD srcDstAddr,WORD srcDstOff,BYTE mode,WORD lgth)
{
	BYTE idata decryptType;
	BYTE xdata keyVlaue[0x20];
	BYTE idata num,j;
	WORD idata offset,len,i;
	BYTE xdata ivBuffer[8];
    BYTE xdata dataPointer[16];
	
#if UICC__FUNCTION__AES		
	BYTE xdata resultData[16];
#endif	
#if UICC__FUNCTION__AES	
	if((kic &0x02) == 0x02)
	{
		decryptType = keyType_AES;
	}
	else
#endif	
	{
		decryptType = keyType_DES;
	}
#if UICC__FUNCTION__STKAPI	
	if(GetSCP80KeyType(kic,TYPE_KEY_KIC,decryptType) == FALSE) 
	{
		return FALSE;
	}
#endif	
	decryptType = GetKicKidKikKey(kic,keyVlaue,TYPE_KEY_KIC,0);

	if(decryptType == FALSE)
	{
		return FALSE;
	}
#if UICC__FUNCTION__AES	
	if((kic &0x02) == 0x02)//AES If b2 b1 = 10 (AES), b4 b3 shall be coded as 00: AES in CBC mode
	{

		if(lgth%16)                         //传进来的长度16整数倍  对于RFM等需要调用前做好数据的移动等处理
		{
			return FALSE;
		}
		
		if(CheckAesKeyLength(decryptType)  == FALSE)
		{
			return FALSE;
		}

		KeyExpansion(keyVlaue,&tempBuf[0x00]);

		memset(keyVlaue,0x00,0x10);   //作为向量的缓存
		
		memset(resultData,0,sizeof(resultData));
		
		memset(dataPointer,0x00,sizeof(dataPointer));
		
		for(i=0x00;i<lgth;i += 0x10)      //这里传进来的都是E2
		{
			ReadTFBuf(srcDstAddr,srcDstOff+i,dataPointer,0x10);
			
			for(j = 0x00;j<0x10;j++)
			{
				if(mode == MODE_DECRYPT)
				{
					resultData[j] = dataPointer[j];
				}
				else
				{
					resultData[j] ^= dataPointer[j];
				}
			}
			if(mode == MODE_DECRYPT)
			{
				AESDecryptForUICC(&resultData[0], &resultData[0], &tempBuf[0x00]);
			}
			else
			{
				AESEncryptForUICC(&resultData[0], &resultData[0], &tempBuf[0x00]);
			}
			
			for(j = 0x00;j<0x10;j++)
			{
				if(mode == MODE_DECRYPT)
				{
					num = dataPointer[j];   //保存解密前的数据 和下一块解密后的数据 异或
					
					resultData[j] ^= keyVlaue[j];
					
					keyVlaue[j] = num;
				}
				else
				{
					resultData[j] = dataPointer[j];
				}
			}
			
			WriteTFBuf(srcDstAddr,srcDstOff+i,resultData,0x10);
		}
			
 		return TRUE;   
	}
#endif
	decryptType = kic &0x0C;                          //get the KIC bit(4-3),that is ,mode
	
	num = GetBlockNumber(lgth,160);
	
	memset(ivBuffer,0,8);
	
	memset(dataPointer,0,8);
	
	for(i=0; i< num;i++)
	{
		offset = i*160;
		len = 160;
		
		if((i==(num-1))&&((lgth%160)!=0))
		{
			len = lgth%160;                          //最后一块字节数
		}
		
		ReadTFBuf(srcDstAddr,srcDstOff+offset,tempBuf,len);
		
		if((decryptType==0)||(decryptType==4)||(decryptType==8)) //CBC
		{
			if(mode == MODE_DECRYPT)        //解密时候保存一块的最后八字节  和下一块解密出来的头八字节异或
			{
				memcpy(ivBuffer,tempBuf+len-8,8);  //
			}
			else
			{
				for(j=0; j<8; j++)
				{
					tempBuf[j] = tempBuf[j]^ivBuffer[j];
				}
			}
		}

		switch (decryptType)
		{
			case 0:	  //single DES CBC
				SingleDES(tempBuf,&keyVlaue[0x00],tempBuf,CBC_Mode|mode,len);
				break;
			case 4:	  //Triple DES in outer-CBC mode using two different keys
				TripleDES(tempBuf,&keyVlaue[0x00],tempBuf,CBC_Mode|mode,len,16);          
				break;
			case 8:	  //Triple DES in outer-CBC mode using three different keys
				TripleDES(tempBuf,&keyVlaue[0x00],tempBuf,CBC_Mode|mode,len,24); 
				break;
			default:  //single DES ECB
				SingleDES(tempBuf,&keyVlaue[0x00],tempBuf,ECB_Mode|mode,len);
				break;
		}
		
		if((decryptType==0)||(decryptType==4)||(decryptType==8))
		{
			if(mode == MODE_DECRYPT)        //
			{
				for(j=0; j<8; j++)
				{
					tempBuf[j] = tempBuf[j]^dataPointer[j];
				}

				memcpy(dataPointer,ivBuffer,8);  //拷贝这次原数据的后八字节,留到下一块和解密后的前八字节异或
			}
			else
			{	
				memcpy(ivBuffer,tempBuf+len-8,8);                               //拷贝最后8字节作为下一块数据的初始向量		
			}
		}
		
		WriteTFBuf(srcDstAddr,srcDstOff+offset,tempBuf,len);
	}

   	return TRUE;                                                     		//REQUEST COMPAREING THE DATA   
}
/*
*@escription:解密当前的RFM数据   传进来的是RAM地址
*
*@param     :KIc    :密钥索引
*@PARAM     :pointer :原始数据,
*@param     :result_pointer	:加密或解密后的数据
*@mode      :mode : Encry or decry
*@param     :length	:加密或解密的数据长度,一定为8的倍数
*
*@return    :TRUE/FALSE
*
*@author    :modify by xuyz
*
*@history   :create by xuyz@20131011
*           :modify by xuweihua@20210310  AES算法key修改成4字节对齐（270改成272）
*
*/
BYTE ToolkitDeChipherRe(BYTE kic,BYTE * pointer,BYTE * result_pointer,BYTE mode,WORD lgth, BYTE last)
{
    BYTE idata decryptType;
	BYTE xdata keyVlaue[0x20]; //目前 AES 32
#if UICC__FUNCTION__AES	
    BYTE idata k,tmp,keytype;	
	WORD idata i;
	BYTE xdata dataPointer[16];
#endif	
	#define TempBuf_Offset_4Bytes_Allign 272
	
	decryptType = GetKicKidKikKey(kic,keyVlaue,TYPE_KEY_KIC,0);

	if(decryptType == FALSE)
	{
		return FALSE;
	}
#if UICC__FUNCTION__AES	
	if((kic &0x02) == 0x02)
	{
		keytype = keyType_AES;
	}
	else
	{
		keytype = keyType_DES;
	}
#endif		

#if UICC__FUNCTION__STKAPI	
	if(GetSCP80KeyType(kic,TYPE_KEY_KIC,keytype) == FALSE)
	{
		return FALSE;
	}
#endif	
#if UICC__FUNCTION__AES	
	if((kic &0x02) == 0x02)//AES If b2 b1 = 10 (AES), b4 b3 shall be coded as 00: AES in CBC mode
	{

		if(lgth%16)                         //传进来的长度16整数倍  对于RFM等需要调用前做好数据的移动等处理
		{
			return FALSE;
		}

 		if((last & Fitst_SMS) == Fitst_SMS) //密钥初始化    CATTPPushPOR会用到tempBuf,所以这里用后面的tempBuf
 		{
			if(CheckAesKeyLength(decryptType) == FALSE)
			{
				return FALSE;
			}
				
			KeyExpansion(keyVlaue,&tempBuf[TempBuf_Offset_4Bytes_Allign]);
 		}
		
		memset(keyVlaue,0x00,0x10);   //作为向量的缓存
		
		memset(dataPointer,0x00,sizeof(dataPointer));
		
		for(i=0x00;i<lgth;i += 0x10)      //这里传进来的都是RAM
		{
			for(k = 0x00;k<0x10;k++)
			{
				if(mode == MODE_DECRYPT)
				{
					dataPointer[k] = pointer[i+k];
				}
				else
				{
					dataPointer[k] ^= pointer[i+k];
				}
			}
			if(mode == MODE_DECRYPT)
			{
				AESDecryptForUICC(&dataPointer[0], &dataPointer[0], &tempBuf[TempBuf_Offset_4Bytes_Allign]);
			}
			else
			{
				AESEncryptForUICC(&dataPointer[0], &dataPointer[0], &tempBuf[TempBuf_Offset_4Bytes_Allign]);
			}
			
			for(k = 0x00;k<0x10;k++)
			{
				if(mode == MODE_DECRYPT)
				{
					tmp = pointer[i+k];   //保存解密前的数据 和下一块解密后的数据 异或
					
					result_pointer[i+k] = dataPointer[k] ^ keyVlaue[k];
					
					keyVlaue[k] = tmp;
				}
				else
				{
					result_pointer[i+k] = dataPointer[k];
				}
			}
		}
			
 		return TRUE;   
	}
#endif
  	decryptType = kic &0x0C;                          //get the KIC bit(4-3),that is ,mode

 	switch (decryptType)
	{
		case 0:	  //single DES CBC
			SingleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],CBC_Mode|mode,lgth);
			break;
		case 4:	  //Triple DES in outer-CBC mode using two different keys
			TripleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],CBC_Mode|mode,lgth,16);          
			break;
		case 8:	  //Triple DES in outer-CBC mode using three different keys
			TripleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],CBC_Mode|mode,lgth,24); 
			break;
		default:  //single DES ECB
			SingleDES(&pointer[0],&keyVlaue[0x00],&result_pointer[0],ECB_Mode|mode,lgth);
			break;
	}

   	return TRUE;                                                     		//REQUEST COMPAREING THE DATA   
}
/*
*@function :   lGenCRC32 
*Polynomial:    X32+X26+X23+X22+X16+X12+X11+X10+X8+X7+X5+X4+X2+X1+X0 
*
*@description :计算CRC32
*
*param         :lOldCRC:初始CRC值
*@param        :ByteVAL :当前参与CRC计算的字节
*
*@return       :currentCRC
*/
unsigned long lGenCRC32( unsigned long lOldCRC,BYTE ByteVal )
{
   DWORD xdata TabVal;
   int   idata j;

   TabVal = ((lOldCRC) ^ ByteVal) & 0xFF;

   for (j=8; j>0; j--)
   {
      if (TabVal & 1)
	  {         
            TabVal = (TabVal >> 1) ^ 0xEDB88320L;
      }   
      else
      {   
           TabVal >>= 1;
       }  
   }

 	return TabVal ^ (((lOldCRC) >> 8) & 0x00FFFFFFL);
}

/*@Function:    IncreatCNTR                                           
 * 
 *@Descrip:     counter加1
 *
 *@See:	        TS 23.048
 *                                                                                                                                     
 *@Param:       cntr: counter
 *              				                                                                                                         
 *@Return:      0(正常)/1（counter 溢出）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131105
 *  
 *@History:     V0.0---Create by XWH 20131105            
 */ 
BYTE IncreatCNTR(BYTE cntr[5])
{
	BYTE idata i;
	WORD idata r;

	r = 0x0001;

	for(i=0x00;i<0x05;i++)
	{
		r = r + cntr[0x04-i];

		cntr[0x04-i] = r&0xFF;

		r = (r>>0x08)&0x0001;
	}

	return (r&0xFF);
}

/*@Function:    CheckSPIRuleForPor                                           
 * 
 *@Descrip:     根据102225  5.1.1 Coding of the SPI 增加判断规则
 *
 *@See:	        
 * 
 *                                                                                                                                     
 *@Param:       SPI1/SPI2
 *              				                                                                                                         
 *@Return:      POR_OK(检查通过)/RFMSecurityError（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *  
 *@History:     V0.0---Create by LCZ 20181217
 *        
 */ 
BYTE CheckSPIRuleForPor(BYTE * spikickid)
{
	if((spikickid[1] & 0x03)!= 0x00)
	{
		if((spikickid[1] & 0x0C) != 0x00)   //If SPI2.b4b3 is not set to 00, then the RC or CC requested for the Response Packet, i.e. SPI2.b4b3, shall be the same as the Command Packet, i.e. SPI1.b2b1.
		{
			if(((spikickid[1] >>2)&0x03) != (spikickid[0] & 0x03))
			{
				return RFMSecurityError;
			}
		}
		
		if((spikickid[1] & 0x10) == 0x10) //Ciphering of the Response Packet, i.e. SPI2.b5 set to '1', is only allowed if the Command Packet has been successfully authenticated (e.g. using CC) and if ciphering was applied to the Command Packet, i.e. SPI1.b3 is set to '1'.              
		{
			if(((spikickid[0] & 0x04)!=0x04) ||((spikickid[0] & 0x03) == 0x00)) //如果上行加密,则要求下行加密并且有mac
			{
				return RFMSecurityError;
			}
		}
	}
	
	return POR_OK;
}
#if UICC__FUNCTION__RFM
/*
*@description : 在RFMtar文件中查找是否存在特定的tar
*
*@function      : FindRFMTars
*
*@param        : BYTE *tarBuf --- 待查找的tar值，3字节
*
*@return        : tarBuf在EF_RFM_TAR文件中的记录号
*
*@HISTORY
*/
BYTE FindRFMTars(BYTE *tarBuf)
{
	UWORD xdata fileAddr;
	BYTE  xdata buf[3];
	BYTE idata num,i,tarNum,k;

	fileAddr = SelectLFFile(DF_Toolkit_P,EF_RFM_TAR);

	if(fileAddr != FALSE)
	{
		num = GetLFRecNumLgth(fileAddr,FileNum);	
	
		for(i=1;i<= num;i++)
		{
			ReadLFBuf(fileAddr,i,PETARNum_Offset,&tarNum,sizeof(BYTE));	
			
			if((tarNum == 0x00)||(tarNum == 0xFF))  //tar len为00 或FF 表示 tar个数无效
			{
				continue;
			}
			for(k = 0x00; k < tarNum ; k ++)
			{
				ReadLFBuf(fileAddr,i,PETARValue_Offset+k*3,&buf[0],sizeof(buf));	
				
				if(memcmp_p(tarBuf,(BYTE *)&buf[0],0x03) == EQUAL)
				{
					return i;	
				}
			}
		}	
	}

	return FALSE;
}
#endif
/*
*@description :根据当前tar值，判断当前下行数据类型	，支持给全局变量 G_TARRecIndex赋值
*
*@param       : BYTE *inputtar --- input, 待查找tar值，3字节
*@param       : BYTE *tarBuf   --- input and output, the value may be NULL or G_TARRecIndex.
*
*@return        : tar Type.
*
*@author       : xuyz
*
*@history       : V1.0  create by xuyz@20131209
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
#if M2M_EUICC
BYTE code ISD_R_TAR[] = {0x00,0x00,0x01};
#endif
BYTE CheckTar(BYTE *inputtar, BYTE *tarBuf)
{
	BYTE  idata tarindex,tarType;
#if UICC__FUNCTION__STKAPI
	BYTE idata appId,tmpAppId;
	UWORD xdata fileAddr;
	UWORD xdata toolkitAppAddr;
#endif	
	
#if !SE_ONLY
	G_RAMBackExecutingProfileIndex = G_CurExecutingProfileIndex;//此时 G_CurExecutingProfileIndex = G_CurEnabledProfileIndex
#endif
	//todo :接口，可在此判断是否为RFM tar  WIB  tar等

	tarType = TarType_Error;  //初始化tar类型
	
	G_Data_Type = DataType_NULL;    //初始化远程指令类型

#if M2M_EUICC
	tarindex = GetProfileIndexByISDPTar(inputtar); //find ISDP TAR,tarindex is profile index add by chentao

	if(tarindex != ILLEGAL_PROFILEINDEX)
	{
		if(tarBuf != NULL)
		{
			G_Data_Type = DataType_RAM;
			tarBuf[0] = TarType_ISDP;
			tarBuf[1] = tarindex;//profileIndex
			tarBuf[2] = ISDR_APPID;
		}

		return TarType_ISDP;
	}

	if(tarType == TarType_Error)
	{
		if(memcmp(inputtar, ISD_R_TAR, sizeof(ISD_R_TAR)) == EQUAL)
		{
			if(tarBuf != NULL)
			{
				G_Data_Type = DataType_RAM;
				tarBuf[0] = TarType_ISDR;
				tarBuf[1] = ISDR_APPID;
				tarBuf[2] = ISDR_APPID;
			}
			return TarType_ISDR;
		}
	}
#endif
#if UICC__FUNCTION__RFM
	if(tarType == TarType_Error)
	{
		tarindex = FindRFMTars(inputtar);  //支持各种RFM tar的处理，包括 03.48/ts102225

		if(tarindex != FALSE)
		{
			tarType = TarType_RFM;
			G_Data_Type = DataType_RFM;
		}
		
	}
#endif
#if GP__AMENDMENT__B__FOR_MASK
	if(tarType == TarType_Error)
	{
		tarindex = ToolkitFindHttpsTriggerTars(inputtar);//Native 卡触发HTTP的 tar
		
		if(tarindex == TRUE)  //表明该Tar是RAM TAR
		{
			tarType = TarType_HTTPS;
			G_Data_Type = DataType_NULL;
		}
	}
#endif	
#if UICC__FUNCTION__RAM
	if(tarType == TarType_Error)
	{
		tarindex = ToolkitFindSDTars(inputtar);//RAM tar，返回的是TAR指向的SDID

		if(tarindex != ILLEGAL_APPID)  //表明该Tar是RAM TAR
		{
			tarType = TarType_RAM;
			G_Data_Type = DataType_RAM;
		}
#if ! SE_ONLY
		else
		{
			if(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX)
			{
				G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;//切到SE区，后续使用key 查找scp80 sdid还需要访问应用 ，切过去以后 等执行完RAM再切回来
				ExecuEnvironmentInit(G_CurExecutingProfileIndex);

				tarindex = ToolkitFindSDTars(inputtar);//RAM tar，返回的是TAR指向的SDID

				if(tarindex != ILLEGAL_APPID)  //表明该Tar是RAM TAR
				{
					tarType = TarType_RAM;
					G_Data_Type = DataType_RAM;
					G_RAMBackExecutingProfileIndex = ILLEGAL_PROFILEINDEX;//se区
				}
				else
				{
					G_CurExecutingProfileIndex = G_RAMBackExecutingProfileIndex;
					ExecuEnvironmentInit(G_CurExecutingProfileIndex);//SE区也没有找到 ，将执行区域切回来
				}
			}

		}
#endif
	}
#endif	
#if UICC__FUNCTION__STKAPI
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(tarType == TarType_Error)
	{
		if(toolkitAppAddr == FALSE)
		{
			return TarType_Error;
		}
	
		tarindex = ToolkitfindSpecifiedTARValue(inputtar);//toolkit tar
	
		if(tarindex != FALSE)
		{	
			ReadLFBuf(toolkitAppAddr, tarindex , AppletStatus_OFFSET, &appId,sizeof(BYTE));

	        if((appId == APPLET_UNLOCKED_SELECTABLE))
			{
				tarType = TarType_ToolkitApplet;   //E:\N3_TEE\develop\branches\ChinaUnicomUicc\test\中国联通NFC-SWP测试工程集测试脚本20160321\GP2.2.1_UICC_ComplianceTests(联通版)-20141112\测试脚本\Bug_List_Contact.uce\Bug_List_20_01 ,要求被触发的应用锁住之后，返回09 状态字
			}			
		}
	}
#endif	
	if((tarBuf != NULL)&&(tarType != TarType_Error))	
	{
		tarBuf[0] = tarType;
		
		tarBuf[1] = tarindex;//TarType_RAM时 该值表示SD的APPID

		if(tarType == TarType_RFM)
		{
#if UICC__FUNCTION__STKAPI			
			tarBuf[2] = ISD_APPID;  //来源：联通SWP操作中的RFM 都采用的是ISD 的SCP80密钥

			fileAddr = SelectLFFile(DF_Toolkit_P,EF_RFM_TAR);

			if(fileAddr != FALSE)
			{
				ReadLFBuf(fileAddr,tarindex,PEsecurityDomainID_Offset,&tarBuf[2],0x01);    //读取关联安全域
			}
#endif			
		}
		else
		{
#if UICC__FUNCTION__STKAPI			
			if(tarType == TarType_RAM)
			{
				appId = tarindex;//RAM TAR时 该值表示SD的APPID
				tarBuf[2] = ISD_APPID;  //来源：联通SWP操作中的RFM 都采用的是ISD 的SCP80密钥
			}			
			else
			{	
				ReadLFBuf(toolkitAppAddr,G_TARRecIndex[1],AppId_OFFSET,&appId,sizeof(BYTE));  //读取该Toolkit应用的appid
                					
			 	tmpAppId = GetAssoiatedSDByAppId(appId);  //获取其关联安全域
			
				tarBuf[2] = SearchClosestSCP80SDId(appId);	
			}
#endif			           									           
		}
		
	}
	return tarType;
}

 /*
*@descirption: 获取当前密钥信息
*
*@param      :	keyinfo:密钥索引字段
*@param      :	buff: 密钥缓存buf,长度为16字节
*@param      :  keytype:密钥类型: KIC OR  KID OR KIK
*
*@return     :void
*
*@author     :create by LIUJIE
*
*@history   :V1.0 :create by LIUJIE
*           :V1.1 :modify by XWH
*/
BYTE GetKicKidKikKey(BYTE keyinfo,BYTE *buff,BYTE keytype,BYTE readKeyLen)
{
	BYTE idata keyIndex;
	WORD idata keyLen;

	keyIndex = (keyinfo&0xF0)>>0x04;   //KeyVersion 
    keyLen = 0;
	
#if M2M_EUICC
	if((G_TARRecIndex[0] == TarType_RFM)||(G_TARRecIndex[0] == TarType_ToolkitApplet)||(G_TARRecIndex[0] == TarType_RAM) || (G_TARRecIndex[0] == TarType_ISDR) || (G_TARRecIndex[0] == TarType_ISDP))  //RAM 或Toolkit App，获取关联的SD的SCP 80密钥
#else
	if((G_TARRecIndex[0] == TarType_RFM)||(G_TARRecIndex[0] == TarType_ToolkitApplet)||(G_TARRecIndex[0] == TarType_RAM)||(G_TARRecIndex[0] == TarType_HTTPS))  //RAM 或Toolkit App，获取关联的SD的SCP 80密钥
#endif
	{
#if UICC__FUNCTION__STKAPI		
		if(G_TARRecIndex[2] == ILLEGAL_APPID)  //RAM 或 Toolkit App没有关联的sd 存在SCP80
		{
			return FALSE;
		}

		keyLen = IF_EnvelopGetKeyAddrBySDVersion(G_TARRecIndex[2], keyIndex, keytype, buff); //查找特定version 的密钥

		if(G_NewSCPKey != FALSE)
		{
			GetSCP80KeyValueBeforeUpdate(buff,keytype);//KEYTYPE和 MODE_KEY_KIC 、MODE_KEY_KID定义一致
		}		
#else
#if COS_UPDATE_ONLINE_BY_NativeOS
		if(IF_UPCOS_EnvelopHandle() == TRUE)
		{
			if(keyIndex != UPCOSConfig_GetUPCOSUMSSDScp80KeyVersion())
			{
				return FALSE;
			}
			
			if(keytype == TYPE_KEY_KIC)
			{
				keyLen = UPCOSConfig_GetUPCOSUMSSDScp80KicLen();
				UPCOSConfig_GetUPCOSUMSSDScp80Kic(&buff[0],keyLen);
			}
			else
			{
				keyLen = UPCOSConfig_GetUPCOSUMSSDScp80KidLen();
				UPCOSConfig_GetUPCOSUMSSDScp80Kid(&buff[0],keyLen);
			}
		}
		else
#endif						
		{		
#if UICC__FUNCTION__RFM			
			keyLen = keyData_getRFMKey(keyIndex,(keytype==TYPE_KEY_KIC)?MODE_KIC:MODE_KID,&buff[0]);               //获取解密key   
#endif			
		}

#endif
		return keyLen;
	}

	return FALSE;	  
}
 /*
*@descirption: 获取AES  Mac Len
*
*@param      :	keyinfo:密钥索引字段
*@param      :  keytype:密钥类型: KIC OR  KID OR KIK
*
*@return     :MAC Len/FALSE
*
*@author     :create by LCZ
*
*@history   :V1.0 :create by LCZ
*/
//BYTE GetAESMACLen(BYTE keyinfo,BYTE keytype)
//{

//	u8					keyType,keyVersion;
//	jref				keySetRef;
//	PN8					sdDataObject;
//	PN8					keySetObjcet;
//	u16                 macLen = 0;

//	keyVersion = (keyinfo&0xF0)>>0x04;   //KeyVersion
//#if M2M_EUICC
//	if((G_TARRecIndex[0] == TarType_RFM)||(G_TARRecIndex[0] == TarType_ToolkitApplet)||(G_TARRecIndex[0] == TarType_RAM) || (G_TARRecIndex[0] == TarType_ISDR) || (G_TARRecIndex[0] == TarType_ISDP))  //RAM 或Toolkit App，获取关联的SD的SCP 80密钥
//#else
//		if((G_TARRecIndex[0] == TarType_RFM)||(G_TARRecIndex[0] == TarType_ToolkitApplet)||(G_TARRecIndex[0] == TarType_RAM))  //RAM 或Toolkit App，获取关联的SD的SCP 80密钥
//#endif
//	{
//		if(G_TARRecIndex[2] == ILLEGAL_APPID)
//		{
//			return FALSE;
//		}

//		sdDataObject		=	MEMtPN8(getSdDataObjBySdid(G_TARRecIndex[2]));
//		keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);

//		if (REF_NULL == keySetRef)
//		{
//			return FALSE;
//		}

//		keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
//		keyType				=	keySet_getKeyType(keySetObjcet, keytype);//keytype是KID

//		if(keyType != KEY_AES_TYPE)
//		{
//			return FALSE;
//		}

//		macLen = keySet_getAesKeyMacLen(keySetObjcet);

//		if((macLen != 0x04) && ((macLen != 0x08)))
//		{
//			return FALSE;
//		}

//		return macLen;
//	}

//	return FALSE;
//}
 /*
*@descirption: 获取GetSCP80KeyType
*
*@param      :	keyinfo:密钥索引字段
*@param      :  keytype:密钥类型: KIC OR  KID OR KIK
*
*@return     :TRUE/FALSE
*
*@author     :create by LCZ
*
*@history   :V1.0 :create by LCZ
             V1.1:修改keyDESorAES参数,传入NULL则是获取KeyType,传入不为NULL则对比,添加下行使用Key和下行数据内容putkey为同一组的判断
*/
#if UICC__FUNCTION__STKAPI
BYTE GetSCP80KeyType(BYTE keyinfo,BYTE kic_kid_kid,BYTE keyDESorAES)
{
	u8					keyType,keyVersion;
	jref				keySetRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;

	keyVersion = (keyinfo&0xF0)>>0x04;   //KeyVersion

#if M2M_EUICC
	if((G_TARRecIndex[0] == TarType_RFM)||(G_TARRecIndex[0] == TarType_ToolkitApplet)||(G_TARRecIndex[0] == TarType_RAM) || (G_TARRecIndex[0] == TarType_ISDR) || (G_TARRecIndex[0] == TarType_ISDP))  //RAM 或Toolkit App，获取关联的SD的SCP 80密钥
#else
		if((G_TARRecIndex[0] == TarType_RFM)||(G_TARRecIndex[0] == TarType_ToolkitApplet)||(G_TARRecIndex[0] == TarType_RAM))  //RAM 或Toolkit App，获取关联的SD的SCP 80密钥
#endif
	{
		if(G_TARRecIndex[2] == ILLEGAL_APPID)
		{
			return FALSE;
		}

		sdDataObject		=	MEMtPN8(getSdDataObjBySdid(G_TARRecIndex[2]));
		keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);

		if (REF_NULL == keySetRef)
		{
			return FALSE;
		}

		keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
		keyType				=	keySet_getKeyType(keySetObjcet, kic_kid_kid);//keytype是KID

		if(keyDESorAES != ADDR_IS_NULL)//比较key type
		{
			if(G_NewSCPKey != FALSE) //此时的条件是下行使用的key和下行包含的数据时putkey更新的是keyversion是同一个
			{
				if(G_NewSCPKey != keyDESorAES)
				{
					return FALSE;
				}
				else
				{
					return TRUE;
				}
			}

			if((keyType >= DES_KEY_TYPE_MIN)&&(keyType < DES_KEY_TYPE_MAX)) //DES key
			{
				if(keyDESorAES != keyType_DES)
				{
					return FALSE;
				}
			}
			else //AES key
			{
				if(keyType != keyDESorAES)
				{
					return FALSE;
				}
			}
		}
		else //获取KEY TYPE
		{
			return keyType;
		}

		return TRUE;
	}

	return FALSE;
}
#endif
/*
*@funtion    :moveOneBitLeft
*
*@description:将 src 的bit序列，左移 1 bit
*
*@param      :BYTE *src    -- 源数据
*             BYTE srcLen  -- 源数据长度
*             BYTE *result -- 移位结果
*
*@return     :void
*
*@author     :dingzy
*
*@history    :V1.0 :create by dingzy@20170616
*/
void moveOneBitLeft(BYTE *src, BYTE srcLen, BYTE *result)
{
	BYTE idata i;
	
	for(i=0; i<(srcLen-1); i++)
	{
		result[i] = (src[i]<<1) | (src[i+1]>>7);
	}
	
	result[srcLen-1] = src[srcLen-1]<<1;
	
	return;
}
/*
*@description:	计算102226 CMAC  SP_800-38B.pdf
*
*@param        :eeAddr  ,数据源
*@param        :ccLength :MAC长度
*@param        :length :数据长度，这个长度并没有减去ccLength的长度
*@param        :keyContext :扩展的密钥
*@param        :resultBuffer :结果数据
*
*@return	   :void
*
*@author       :
*
*@history	   :
*/
//BYTE ToolkitForAESCalculateMac(UWORD eeAddr,BYTE ccLength,WORD length,BYTE cplLength,BYTE *keyContext,BYTE * resultBuffer)
//{
//	BYTE idata i,k;
//	BYTE idata n,headLen;
//  	BYTE idata lastLgth;
//	BYTE xdata keyValue[0x10];				//缓存K  目前长度为0x10
//	BYTE xdata SubkeyValue[0x10];				//缓存Sub Key(K1/K2公用)
//	BYTE xdata aesBuf[16];
//	BYTE xdata cpResult[0x10];
//	WORD xdata time,j;
//	
//	if(length == 0x00)
//	{
//		return TRUE;
//	}
//	
//   //对于下行 则头长度 CPL + 14(CHL + SPI + KIC+KID+TAR+COUNTER+PCNTR )
//   //对于上行 则头长度 027100 CPL(2) +CHL+  TAR+  CNTR +PCNTR+  errorCode  也是14 + CPLLen
//	headLen = 14 + cplLength; 
//	
//	n = (length-ccLength)/AES_BLOCKSIZE;       //长度减去mac ccLength
//	
//	lastLgth = (length-ccLength)%AES_BLOCKSIZE;
//	
//	if(lastLgth != 0x00)
//	{
//		n += 1;
//	}
//	
//	memset(aesBuf,0x00,sizeof(aesBuf));       //暂时用作计算L的初始buf
//	
//	//AESEncrypt(aesBuf, aesBuf , keyContext );  //aesBuf即为 L
//	
//	AESEncryptForUICC(aesBuf, aesBuf,keyContext);
//	
//	moveOneBitLeft(aesBuf,0x10,SubkeyValue);    // if MSB1(L) = 0,then K1=L<<1;else,Else K1 = (L << 1) ⊕ Rb,Rb=R128=0(120)10000111=0x(0000......87)
//	
//	if((aesBuf[0]&0x80) != 0x00)       //计算K1，不管Mlen是不是B的整数倍，都是要先计算K1，Mlen不是B的整数倍的时候，要通过K1计算出K2
//	{
//		SubkeyValue[AES_BLOCKSIZE-1] ^= RB_128BITS;
//	}
//	
//	if(lastLgth != 0x00)       //If Mn* is a complete block,let Mn = K1 ⊕ Mn*; else, let Mn = K2 ⊕ (Mn*||10j),where j = nb-Mlen-1.
//	{
//		i = SubkeyValue[0] & 0x80;        //暂用I
//		
//		memcpy(aesBuf,SubkeyValue,0x10);
//		
//		moveOneBitLeft(aesBuf,0x10,SubkeyValue);  //If MSB1(K1) = 0, then K2 = K1 << 1;Else K2 = (K1 << 1) ⊕ Rb.
//		
//		if(i != 0x00)
//		{
//			SubkeyValue[AES_BLOCKSIZE-1] ^= RB_128BITS;
//		}
//	}
//	
//	j = 0x00;
//	time = 0x00;
//	memset(cpResult,0x00,sizeof(cpResult));
//	memset(keyValue,0x00,0x10);
//	
//	for(i=0x01;i<=n;i++)           //i表示第几块数据，16字节为1块
//	{
//// 		for(;j<length;)
//// 		{
//			if(j<headLen)
//			{
//				if(headLen == 0x10)       //mac字节前面的数据刚好16字节
//				{
//					VMReadBuf(&aesBuf[0],eeAddr+j,headLen);
//					
//					j += headLen;
//					
//					j += ccLength;   //跳过mac字节
//				}
//				else if(headLen<0x10)       //mac字节前面的数据不够16字节
//				{
//					VMReadBuf(&aesBuf[0],eeAddr+j,headLen);   //读取前面的不足16字节的数据
//					
//					VMReadBuf(&aesBuf[headLen],eeAddr+headLen+ccLength,0x10-headLen); //跳过mac字节，拼够16字节最小块
//					
//					j += 0x10;
//					
//					j +=ccLength;
//				}
//				else               //mac前面的数据长度大于0x10,则会读取前面的数据两次 time来控制
//				{
//					if(time == 0x00)
//					{
//						VMReadBuf(&aesBuf[0],eeAddr+j,0x10);
//						
//						j += 0x10;
//						
//						time++;
//					}
//					else
//					{
//						VMReadBuf(&aesBuf[0],eeAddr+j,(headLen-0x10));
//						
//						j = headLen + ccLength;
//						
//						VMReadBuf(&aesBuf[headLen-0x10],eeAddr+j,0x20-headLen);      //当 headLen的长度超过0x10字节的时候,第二次读取前面剩余的headLen-0x10字节,则跳过mac字节后，还需要读取0x10-（headLen-0x10）字节
//						
//						j += 0x20-headLen;
//					}
//				}
//				
//				if(i == n)
//				{
//					for(k=0x00;k<0x10;k++)  //最后一块需要与K1/K2异或，这里K1/K2同一个buf
//					{
//						cpResult[k] ^= SubkeyValue[k];
//					}
//				}						
//				
//			}
//			else
//			{	
//// 				if(time == 0x01)
//// 				{
//// 					VMReadBuf(&aesBuf[0],eeAddr+j,0x20-headLen);      //当 headLen的长度超过0x10字节的时候,第二次读取前面剩余的headLen-0x10字节,则跳过mac字节后，还需要读取0x10-（headLen-0x10）字节
//// 					
//// 					time++;
//// 					
//// 					j += 0x20-headLen;
//// 				}
//// 				else if((time !=0x01) && (i != n))
//				if(i != n)
//				{
//					VMReadBuf(&aesBuf[0],eeAddr+j,0x10);
//					
//					j += 0x10;
//				}
//				else
//				{
//						if(lastLgth != 0x00)     //填充数据，0x80,0x00,0x00。。。。
//						{
//							memset(keyValue,0x00,0x10-lastLgth);
//							
//							keyValue[0] |= 0x80;		
//						}

//						for(k=0x00;k<0x10;k++)  //最后一块需要与K1/K2异或，这里K1/K2同一个buf
//						{
//							cpResult[k] ^= SubkeyValue[k];
//						}
//						
//						VMReadBuf(&aesBuf[0],eeAddr+j,length-j);   //读取剩余的数据
//						
//						if(lastLgth != 0x00) 
//						{					
//							memcpy(&aesBuf[length-j],keyValue,0x10-lastLgth);
//						}
//						
//				}
//							
//			}
//// 		}
//		
//		for(k=0x00;k<0x10;k++)
//		{
//			aesBuf[k] ^= cpResult[k];
//		}
//		
//		AESEncryptForUICC(aesBuf, cpResult , keyContext );  // AES-ECB
//		
//	}
//	
//	memcpy(resultBuffer,cpResult,0x08);    //这里拷贝计算结果  取多长看函数调用

//	return TRUE;	
//}

/*
*@description:比较当前mac是否正确
*
*@param      :kid
*@param      :cplLength	:CPL字段的长度,兼容CPL为BER-TLV个数的数据
*@param      :ccLength  :CC/MAC数据长度
*@param      :offset    : 指向UDL数据的偏移
*@param      :length    :待计算mac的数据的长度
*
*@return     :TRUE:MAC正确/FALSE:下行mac与计算出的mac不相等
*
*@author     :create by xuyz
*
*@history    :V1.0: create by xuyz
*            :modify by xuweihua@20210310  AES算法key修改成4字节对齐（270改成272）
*/
BYTE ToolkitCalculateMac(UWORD eeAddr,BYTE spi,BYTE kic,BYTE kid,BYTE cplLength,BYTE ccLength,WORD length,BYTE * resultBuffer)
{
	BYTE idata headLen,j;
	WORD idata i;
	BYTE xdata cpResult[0x10], cpEnter[0x10];
	BYTE xdata cpKey[0x20];
	DWORD xdata Crc32;
	BYTE  idata time;
	WORD idata offset;
	#define TempBuf_Offset_4Bytes_Allign 272

	headLen = 14 + cplLength;  // CPL CHL SPI KIC KID TAR CNTR PCNTR 的长度

	offset = 0x00;
	
	if((spi&0x03) == 0x01)  //CRC32
	{
		Crc32 = 0xFFFFFFFF;
	
		ReadEeprom(tempBuf,eeAddr+offset,headLen);

		for(j=0; j<headLen; j++)
		{						
			Crc32 = lGenCRC32(Crc32,tempBuf[j]);
		}

		length -= (headLen+ccLength);  //剩余 数据长度 

		offset += (headLen+ccLength);   //剩余数据起始偏移

		for(i=0; i<length;)
		{				
			headLen = ((i + 240 ) > length)?(length - i):240;
							
			ReadEeprom(tempBuf,eeAddr+offset,headLen);

			for(j=0; j<headLen; j++)
			{						
				Crc32 = lGenCRC32(Crc32,tempBuf[j]);
			}

			offset += headLen;
			i += headLen;
		}

		Crc32 = ~Crc32;    
	
	    for(j=0; j<0x04; j++)
	    {
	       cpResult[3-j] = Crc32 >>(j*8);
	    }
	}
#if UICC__FUNCTION__AES	
	else if(((kid&0x02) == 0x02) && ((spi&0x03) == 0x02)) //AES 
	{
		if(GetSCP80KeyType(kid,TYPE_KEY_KID,keyType_AES) == FALSE)
		{
			return FALSE;
		}
 		j = GetKicKidKikKey(kid,&cpKey[0],TYPE_KEY_KID,0);  //key LV 结构
		
		if(CheckAesKeyLength(j)  == FALSE)
		{
			return FALSE;
		}
		
		KeyExpansion(cpKey,&tempBuf[TempBuf_Offset_4Bytes_Allign]);     //用后半部分存扩展密钥

		ToolkitForAESCalculateMac(eeAddr,ccLength,length,cplLength,&tempBuf[TempBuf_Offset_4Bytes_Allign],resultBuffer);
		
		return TRUE;
		
	}
#endif	
	else
	{
		memset(cpResult,0x00,0x08);
		
//		if(GetSCP80KeyType(kid,TYPE_KEY_KID,keyType_DES) == FALSE)
//		{
//			return FALSE;
//		}
	
		j = GetKicKidKikKey(kid,cpKey,TYPE_KEY_KID,0);

		if(j == FALSE)
		{
			return FALSE;
		}
					    				
		for(i=0x00,time=1;i<length;)
		{
			if (i < headLen)		//计算头
			{
				if ((i + 8) > headLen)		//需要从文件中补值
				{			
					ReadEeprom(&cpEnter[0],eeAddr+i,(headLen-i));
									
					ReadEeprom(&cpEnter[headLen-i],eeAddr+headLen+ccLength,(8+i-headLen));
	
					i += ccLength;
				}
				else
				{
					ReadEeprom(&cpEnter[0],eeAddr+i,8);
				}
	
				i += 0x08;
			}
			else if((i == headLen)&&(ccLength!=0))  //若ccLength==0，则不需要跳过CC
			{
				i += ccLength;
	
				continue;
			}
			else if((i+8) > length)
			{
				memset(cpEnter, 0x00, 8);	

				ReadEeprom(&cpEnter[0],eeAddr+i,(length - i));
	
				i += 0x08;
			}
			else
			{			
				ReadEeprom(&cpEnter[0],eeAddr+i,8);

				i += 0x08;
			}
	
			for(j=0;j<0x08;j++)
			{
			  	cpEnter[j]=cpEnter[j] ^ cpResult[j];
		    }
			
			if((kid&0x0C)==0x00)        //DES CBC
			{
				SingleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08); 
			}
			else if ((kid&0x0C)==0x04) //3DES 2 key CBC
			{
				TripleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08,16);
			}
			else if ((kid&0x0C)==0x08) //3DES 3 key CBC
			{
				TripleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08,24);
			}
			else                       //Reserved
			{
				return FALSE;
			}		
			
			time ++;
		}		
	}
	
	if(ccLength != 0x00)
	{
		memcpy(resultBuffer,cpResult,ccLength);
	}
	else
	{
		memcpy(resultBuffer,cpResult,0x08);
	}

	return TRUE;
}
/*
*@function  :CheckCounterSubFunction
*
*@description:  比较下行数据中的counter
*
*@param     :spi
*@param     :couter :文件中存放的counter
*@param     :counterPointer
*
*@return     :compareResult
*
@author     :xuyz
*
*@history   :V1.1 modify 20161124,修改只有对于bit5bi4:10 bit bi411两种模式下自身的counter才加1，然后与下行counter进行比较

*/
BYTE CheckCounterSubFunction(BYTE spi,BYTE *counter, BYTE *counterPointer)
{
	BYTE idata acc,counterMode;
	
	counterMode = spi&0x18; 	 						 //get the bit 5-4 of the SPI high byte ,means countermode

	if((counterMode == 0x10)||(counterMode == 0x18)) 
	{				
		acc = IncreatCNTR(&counter[0]); //03.48 When the counter value reaches its maximum value the counter is blocked .

		if(acc == 0x01)
		{
			return ERROR_CNTR_BLCED ;
		}
		
		if(counterMode == 0x10)                             //SPI high byte ,bit5-4 = 10, higer is ok
		{
			if(memcmp_p(&counterPointer[0],&counter[0],5) < EQUAL)    //ST 软防在使用memcmp进行比较时，假设counterPointer为0xe012358620  &counter[0]为0 ，则比较出的结果为 < 0	,因此使用memcmp_p，不
			{
				return ERROR_CNTR_LOW;
			}
		}
		else 			// SPI high byte ,bit5-4 = 11,only one higher
		{
			if(memcmp_p(&counterPointer[0],&counter[0],5)> EQUAL)	//ST 软防在使用memcmp进行比较时，假设counterPointer为0xe012358620  &counter[0]为0 ，则比较出的结果为 < 0 
			{
				return ERROR_CNTR_HIGH;
			}
			else if(memcmp_p(&counterPointer[0],&counter[0],5) < EQUAL)	 //ST 软防在使用memcmp进行比较时，假设counterPointer为0xe012358620  &counter[0]为0 ，则比较出的结果为 < 0 
			{
				return ERROR_CNTR_LOW;
			}
			else //白盒测试
			{
			}
		}
		
		return POP_OK;	
		
	}
	else                                                      	//no counter available
	{
		return POP_OK;
	}
}
/*@Function:    FormattedRFMSMSCheckCounter                                           
 * 
 *@Descrip:     判断下行RFM 信息counter 是否满足要求
 *
 *@See:	        TS 23.048
 *                                                                                                                                     
 *@Param:       spi0  --- SPI第一字节
 *      
 *                    counterPointer: 下行信息counter
 * 
 *@Return:      TRUE(检查通过)/FALSE（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131105
 *  
 *@History:     V0.0---Create by XWH 20131105  
				V1.1---modify by xuyz,对于toolkit应用，直接比较所使用的sd的counter，该改动暂无测试用例支撑 ，根据对规范的理解进行的修改@20170117
*               V1.2---modify by xuyz,对于profile Counter和key绑定/去掉配置文件EF_RAMAPDUConfig
                V1.3---进行RFM操作时根据SPIKICKID等值的配置获取相应的counter值,用于判断该counter是否合法     @yangjian  2022.12.6
*/ 

BYTE FormattedRFMSMSCheckCounter(BYTE * spiKicKid,BYTE * counterPointer)           
{                                            
	UWORD xdata  fileAddr,counterFileAddr;	
	BYTE idata counterResult,counterId;	
	BYTE  xdata counter[5];	

	counterId = 0x01;			
	counterFileAddr = FALSE;
	fileAddr = FALSE;
	
	if((spiKicKid[0]&0x10) == 0x10)  //bit5bit4:0x10 0x11 .目前只针对这两种情况进行counter的比较
	{
		if(G_TARRecIndex[2] == ILLEGAL_APPID)  //RAM 或 Toolkit App没有关联的sd 存在SCP80
		{
			return ERROR_CNTR_BLCED;
		}
		
#if COS_UPDATE_ONLINE_BY_NativeOS		
		if(IF_UPCOS_EnvelopHandle() == TRUE)
		{
			fileAddr = UPCOSConfig_GetUPCOSUMSSDCounterAddr();
			
			if (fileAddr == FALSE)		//暂时返回安全错误
			{
				return ERROR_CNTR_BLCED;
			}
			ReadEeprom(&counter[0],fileAddr,5);
		}		
		else
#endif
		{
#if GP__AMENDMENT__B__FOR_MASK			
			if(G_TARRecIndex[0] == TarType_HTTPS)
			{
				fileAddr = SelectLFFile(DF_Toolkit_P,EF_HttpsTriggle_TAR);
				
				if(fileAddr == FALSE)		//暂时返回安全错误
				{
					return ERROR_CNTR_BLCED;
				}
				
				ReadLFBuf(fileAddr, 1,HTTPS_COUNTER_Offset, (BYTE *)counter, 0x05);		//读取卡内counter
			}
			else
#endif				
			{
#if UICC__FUNCTION__RFM				
				if(((spiKicKid[2]>>4)&0x0F)!=0x00)									//使用了KIC-Key
				{
					counterId = (spiKicKid[2]>>4)&0x0F;	
				}
				else
				{
					if(((spiKicKid[3]>>4)&0x0F)!=0x00)							//使用了KID-Key
					{
						counterId = (spiKicKid[3]>>4)&0x0F;	
					}	
					else													   //KIC-Key与KID-Key均不使用
					{
						counterId = 0x01; 
					}
				}		
				
				counterFileAddr = SelectLFFile(DF_Toolkit_P, EF_RFM_CounterFile);
		
				if (counterFileAddr == FALSE)		
				{
					return POP_OK;
				}
		
				ReadLFBuf(counterFileAddr, counterId,0, (BYTE *)counter, 0x05);		//读取卡内counter

#endif				
			}
		}
		
		
		counterResult = CheckCounterSubFunction(spiKicKid[0],counter,counterPointer);

		if(counterResult != POP_OK)
		{
			return counterResult;
		}
				
		//counter校验通过后，本地counter更新成下行counter（而不是本地counter+1）, by xwh 20191120
		//setKeyCounterForSCP80(G_TARRecIndex[2], G_RFMRAMKeyversion, SCP80_SEQUENCE_CONTER_SIZE, counterPointer) ;
#if COS_UPDATE_ONLINE_BY_NativeOS		
		if(IF_UPCOS_EnvelopHandle() == TRUE)
		{
			WriteEepromAtomic(fileAddr,counterPointer,5);
		}
		else
#endif
		{
#if GP__AMENDMENT__B__FOR_MASK			
			if(G_TARRecIndex[0] == TarType_HTTPS)
			{
				WriteLFBuf(fileAddr, 1,HTTPS_COUNTER_Offset, (BYTE *)counterPointer, 0x05);	
			}
			else
#endif				
			{
#if UICC__FUNCTION__RFM				
				//WriteLFBuf(fileAddr, G_TARRecIndex[1],PECOUNTER_Offset, (BYTE *)counterPointer, 0x05);
				WriteLFBuf(counterFileAddr, counterId,0, (BYTE *)counterPointer, 0x05);				
#endif				
			}
		}
	}

	return POP_OK ;
}
/*@Function:    CheckRFMMinimumSecurityLevel                                           
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
 */ 
BYTE CheckRFMMinimumSecurityLevel(BYTE * spikickid)
{
	UWORD xdata fileAddr;
	BYTE idata mslIndex;
	
	if(G_TARRecIndex[0] == TarType_RFM)
	{		
		fileAddr = SelectLFFile(DF_Toolkit_P,EF_RFM_TAR);
		
		if(fileAddr == FALSE)
		{
			return POR_OK;
		}
		
		ReadLFBuf(fileAddr,G_TARRecIndex[1],PETarMSL_Offset,&mslIndex,1);
		
		/*
		- if SPI1.b2b1 is equal to or greater than MSLD.b2b1 and
		- if SPI1.b3 is equal to or greater than MSLD.b3 and
		- if SPI1.b5b4 is equal to or greater than MSLD.b5b4
		*/
		if(((spikickid[0]&0x03)>=(mslIndex&0x03))&&((spikickid[0]&0x04)>=(mslIndex&0x04))&&((spikickid[0]&0x18)>=(mslIndex&0x18)))
		{
			return POR_OK;
		}
		
		return RFMSecurityLevel;
	}
	else
	{
		return POR_OK;
	}
}
/*
×@function     :CheckTarSupportSingleDES

*@description: 在处理SMS的下行数据中，判断在某些tar情况下，需要检查下行报文或上行POR不能支持单DES 算法
×
×@param      :spiKicKid[4]: SPI1 SP2 KIC KID
*
*@return     :FALSE: 当前tar不能支持单des,tar类型由G_TARRecIndex[0]指示，其取值有  TarType_RAM、TarType_RFM 、TarType_ToolkitApplet
							/TRUE:不用判断是不是支持单DES
*
*@history    移动到uicc_interface.c文件中，并修改名字为IF_CheckTarSupportSingleDES    by xuweihua@20210203 同步CATTP，PRL代码，CATTP PRL功能可正常测试
*/
//BYTE CheckTarSupportSingleDES(BYTE * spiKicKid)
//{
//#if UICC__FUNCTION__SINGLEDES
//	return TRUE;
//#else
//	if(((((spiKicKid[2]&0x0C) == 0x00)||((spiKicKid[2]&0x0C) == 0x0C))&&(((spiKicKid[0]&0x04) == 0x04)||((spiKicKid[1]&0x10) == 0x10))&&(((spiKicKid[2]&0x03) == 0x01) ||((spiKicKid[2]&0x03) == 0x00)))     //102225 KIC(spiKicKid[2]&0x03) == 0x01 单DES 的各种条件
//	||(((spiKicKid[3]&0x0C) == 0x00)&&(((spiKicKid[3]&0x03) == 0x01) || ((spiKicKid[3]&0x03) == 0x00))&&( (((spiKicKid[0]&0x03) != 0x00)&&((spiKicKid[0]&0x03) != 0x01)) ||( ((spiKicKid[1]&0x0C) != 0x00)&&((spiKicKid[1]&0x0C) != 0x04) ))) )  //102225 KID ((spiKicKid[3]&0x03) == 0x01) mac前提
//	{
//		if((G_TARRecIndex[0] == TarType_RAM)||(G_TARRecIndex[0] == TarType_RFM))  //RAM RFM不能支持单des
//		{
//			return FALSE;
//		}
//		else  //TOOLKIT
//		{
//			return TRUE;
//		}
//	}
//	return TRUE;
//#endif
//}
/*
*@description: 解密format ENV SMS,并将解密后的数据拼装到Envelop handlerbuf中
*
*@param      :refrence :当前批次
*@param      :headerNo : 短信头信息在6F71 中的记录号
*@param      :smsType  :短信雷西国内
*@param      :tar:缓存当前format数据的tar值
*
*@return    :<0x0B: 当前指令执行错误的返回状态字/offset：CPLoffset
*
*@author    :create by xuyz
*
*@hisotry	:V1.0 :create by xuyz
            ：V1.1 modify by lgy     //当加密数据中包含填充值时，padding 数据可以不为0 ，规范没有明确说明 删掉判断pcntr必须为0的相关代码
									   相关测试用例：移动RFM&RAM测试脚本_2G3G结合/CMCC_RFM_test/RFM_TEST(2G)/5.15_SecurityMechanismCommandPacket003
			: V1.2 modify by xuyz   //1：对于toolkit应用，同样判断counter是否有效
                                    //2：增加接口函数IF_GetPPDownloadKeyVersion， 目前针对RAM Toolkit要求KIC KID 的密钥索引必须一致 ，RFM 等暂时未按要求，后续需要重新测试用例 @20170118
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能									
*/
WORD ToolkitExcuteFormattedRFMSMS(BYTE refrence,BYTE headerNo,BYTE smsType,BYTE *tpoa)
{
	BYTE idata recNo,total,i,blocksize,mode;
	BYTE idata cplLength,CcLength;
	BYTE  idata lgth,udl,result;						//一次解包处理的字节长度
	BYTE  idata lastLgth;					//一次解包后剩余的不足8字节的余数
	WORD idata usedSize,offset,nextOffset;
	UWORD xdata fileAddr;
	BYTE idata recLength,chl;
	BYTE idata processResult,pctrn,DecrypReoffset; //DecrypReoffset用来保存加解密剩余数据的偏移等
	BYTE xdata mac[16],cc[16],recNoBuf[18],counter[0x05];
	BYTE xdata spiKicKid[0x04];
	
#if UICC__FUNCTION__STKAPI	
	UWORD xdata toolkitAppAddr;
#endif	
#if UICC__CDMA__SMS__DOWNLOAD
	BYTE idata cdmarecNo;
	
	G_cdmaSmsNum = 0x00;
#endif
	
	cplLength = 0x00;	

	lastLgth = 0x00;

	CcLength = 0x00;
	
	blocksize = 0x08;
	
	processResult = POR_OK; //表示结果正确

	if((smsType == TYPE_SMS_ENV_SINGLE)||(smsType == TYPE_SMS_UPD_SINGLE))
	{
		total = 0x01;
#if UICC__CDMA__SMS__DOWNLOAD		
		cdmarecNo = ToolkitFindRFMCSMS_Head(MODE_FIND_RECNO , 0x01,0x01,0x00,tpoa);
		if(cdmarecNo != FALSE)
		{
			G_cdmaSmsNum = 0x01;		
		}

#endif
	}
	else
	{
		total =  ToolkitFindSpecifiedRFMSMS(MODE_FIND_TOTAL,refrence,0x00,smsType,tpoa);
#if UICC__CDMA__SMS__DOWNLOAD		
		cdmarecNo = ToolkitFindRFMCSMS_Head(MODE_FIND_RECNO ,refrence,total,0x00,tpoa);
		
		if(cdmarecNo != FALSE)
		{
			G_cdmaSmsNum = total;		
		}
#endif
		
	}

	if(total == 0)
	{
		return FALSE;
	}

	usedSize = 0x00;
	
	DecrypReoffset = D_TmpBufStart;          //DES

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);

	recNoBuf[0] = total;

	for(i =1; i<= total; i++)
	{
		if((smsType == TYPE_SMS_ENV_SINGLE)||(smsType == TYPE_SMS_UPD_SINGLE))
		{
			recNo = refrence;
		}
		else
		{
			recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,refrence,i,smsType,tpoa);	
		}

		recNoBuf[i] = recNo;

		//ReadLFBuf(fileAddr, recNo, 17, &RecData[0], sizeof(BYTE));
		RecData[0] = ReadLFFileValue1ByteLen(fileAddr,recNo,17,&RecData[1]);
		
		if(RecData[0] == 0xFF)
		{
			return FALSE;
		}

		if(RecData[0] == 0x00)  //如果没有数据，则继续查找
		{
			continue;	
		}
		
		udl = RecData[0];

		//ReadLFBuf(fileAddr, recNo, 18, &RecData[1], RecData[0]);

		if(i == 0x01)  
		{
			if(RecData[0] <0x0F)
			{
				processResult = RFMSecurityError;
			}
				
			cplLength = ToolkitGetCPLLength(RecData[1]);  
			
			chl = RecData[1+cplLength]; //保存CHL的长度
			
		    memcpy(spiKicKid, &RecData[2+cplLength], 0x04);  //获取SPI1  SPI2 KIC KID

		    //memcpy(G_SPIKICKID, &RecData[2+cplLength], 0x04);  //获取SPI1  SPI2 KIC KID  用于其他地方来判断此时下行算法使用的密钥类型

			memcpy(&G_ToolkitTar[0], &RecData[6+cplLength], 0x03);  //tar   提前到 获取AES Mac Len长度前面,GetAESMACLen要使用到G_TARRecIndex
			
			if(CheckTar(G_ToolkitTar,G_TARRecIndex) == TarType_Error)  //根据tar值，设置当前tar类型
			{
				if(processResult == POR_OK)
				{
					processResult = TARUnknown;
				}
			}			
#if UICC__FUNCTION__AES					
			if(((spiKicKid[0]&0x03) == 0x02)&&((spiKicKid[3]&0x03)==0x02))  //加密校验 
			{
					CcLength = GetAESMACLen(spiKicKid[3],TYPE_KEY_KID);
					
					if((CcLength == FALSE) || (CcLength != (chl -13)))
					{
						//电信测试，key not found / mac length error, 要返回06 error
						if(processResult == POR_OK)
						{
							//Scripts_additional\SCP_80_AES_MAC_length_Test01期望返回01或者05
							processResult = RFMSecurityError;	                    //程序继续往下执行
						}						
					}				
			}
			else
#endif			
			{	
				CcLength = GetCcLength(spiKicKid[0],spiKicKid[3]);
			}
			
			if(CcLength == 0xFF)
			{
				if(processResult == POR_OK)
				{
					processResult = RFMSecurityError;
				}
			}
#if UICC__FUNCTION__AES			
			if(((spiKicKid[2]&0x02) == 0x02)&&((spiKicKid[0] & 0x04) == 0x04)) //添加 加密时候才判断是不是AES 
			{
				blocksize = 0x10;
			}
#endif			
			if(chl != (CcLength + 13)) // SPI KIC KID CNTR PCNTR CC/RC  //移动送检工程集 RFMRAM测试2G3G   CMCC_RFM_test/RFM_TEST(3G)/5.1 IMPLEMENTATION FOR SMS-PP/5.1.5 Security Mechanism for the Command Packet/SecurityMechanismForTheCommandPacket1_00   step12 chl长度与实际不符 要求报错
			{
				if(processResult == POR_OK)
				{
					processResult = RFMSecurityError;	                    //程序继续往下执行
				}
			}
	//		IF_RFMRAM_SameTar(spiKicKid,cplLength); 
			
			memcpy(counter, &RecData[6+cplLength+3], 0x05);  //counter,可能是密文

			/*The Receiving Entity shall check the Minimum Security Level before processing the security of
			the Command Packet. If the check fails, the Receiving Entity shall reject the messages and a Response Packet with the
            'Insufficient Security Level' Response Status Code shall be sent if required.*/
			
			if(processResult == POR_OK)
			{	
				if((G_TARRecIndex[0] == TarType_RAM)||(G_TARRecIndex[0] == TarType_HTTPS))  //表示RAM
				{						
					processResult = CheckRAMMinimumSecurityLevel(spiKicKid[0]);															
				}
#if UICC__FUNCTION__STKAPI				
				else if(G_TARRecIndex[0] == TarType_ToolkitApplet)  //表示Toolkit app
				{
					processResult = POR_OK;
				
					toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

					if(toolkitAppAddr != FALSE)
					{
						ReadLFBuf(toolkitAppAddr,G_TARRecIndex[1],MSLLen_OFFSET,mac,3);
												
						processResult = CheckMinimumSecurityLevel(spiKicKid[0],&mac[0]);						
					}
				}
#endif				
#if M2M_EUICC
				else if(G_TARRecIndex[0] == TarType_ISDR)//SGP.02_v3.1.pdf   
				{
					if(((spiKicKid[0]&0x03)>=0x02)&&((spiKicKid[0]&0x04)>=0x04)&&((spiKicKid[0]&0x18)>=0x10))
					{
						processResult = POR_OK;
					}
					else
					{
						return RFMSecurityLevel;
					}
				}
#endif
#if UICC__FUNCTION__CATTP
			  else if(G_TARRecIndex[0] == TarType_CATTP)  //??CATTP zhhj
			  {
				  processResult = POR_OK;
			  }
#endif			  
			  else   //??others Tar:?RFM tar?
			  {
				  processResult = CheckRFMMinimumSecurityLevel(&spiKicKid[0]);
			  }
			}
#if SE_ONLY   //EUICC rsp 基本通信功能测试   RFM all安全级别配置测试v1.0.0/RFM-ALL/步骤86、SPI:0011,KIC:22,KID:00,CNTR:0000000001:下行:NoRC/CC/DS #No Ciphering #No counter ----->上行:PoRrequired $NoRC/CC/DS $Ciphering 期望报安全错误06，实际没有报错，发了加密的POR出来
			if(JcvmConfig_GetOPerator() != OPERATOR_CHINAMOBILE)//同步移动5g代码，此处不用检查
#endif
			{
				if(CheckSPIRuleForPor(spiKicKid) != POR_OK)
				{
					if(processResult == POR_OK)
					{
						processResult = RFMSecurityError;	                    //记录错误，程序继续往下执行
					}
				}
			}

			if(IF_CheckTarSupportSingleDES(spiKicKid) == FALSE) //不支持单DES
			{
				if(processResult == POR_OK)
				{
					processResult = RFMSecurityError;	                    //记录错误，程序继续往下执行
				}
			}
			
//			if(IF_GetPPDownloadKeyVersion(&processResult,spiKicKid) == FALSE)
			{	//SIM API/api_2_enh_gsdl  中的第5个C2  KIC  KID 分别为00 15， 不一致;   SIM API/api_2_enh_gsdo 中同样存在KIC KID不一致的情况
				
				//在counter验证时，如果counter和key对应，那么需要使用到G_RFMRAMKeyversion ，当报文既不需要解密也不需要计算mac时，如何获取G_RFMRAMKeyversion的值呢？
				//该部分暂无相关测试用例支持.
				if((spiKicKid[0]&0x04) == 0)  //不需要解密
				{
					if(((spiKicKid[0]&0x03) <= 0x01))	//不需要算mac or RC(不需要密钥)，SPI&0X03 = 00 或 01
					{		
							G_RFMRAMKeyversion = spiKicKid[2]>>4;
						
							if((spiKicKid[0]&0x10) == 0x10)  //不加密，不算mac,需要计算mac
							{
								if((spiKicKid[2]&0xF0)!= (spiKicKid[3]&0xF0))  
								{
									if(processResult == POR_OK)
									{
										processResult = RFMSecurityError;
									}
								}
							}
				
					}
					else  //不需要解 密，但是需要进行 mac验证
					{
						G_RFMRAMKeyversion = spiKicKid[3]>>4;	//使用kid的版本
					}
				}			
				else  //需要解密
				{
					G_RFMRAMKeyversion = spiKicKid[2]>>4;	//使用kic的版本
					
					if((spiKicKid[0]&0x03)>= 0x02) //需要解密和算mac，要求kic，kid版本号一致，见ts123.048附录B，lj 20131127
					{
						if((spiKicKid[2]&0xF0)!= (spiKicKid[3]&0xF0))
						{
							if(processResult==POR_OK)
							{
								processResult = RFMSecurityError;
							}
						}
					}
				}
			}	
			
		}
			
		if((spiKicKid[0]&0x04) == 0x04)  //Ciphering
		{
			if(i == 0x01)
			{
#if UICC__FUNCTION__AES				
				if(blocksize == 0x10)
				{
					DecrypReoffset = D_TmpBufStartWithAES;   //AES
				}
				else
#endif				
				{
					DecrypReoffset = D_TmpBufStart;          //DES
				}
				if(total == 1)
				{
					mode = FIRST_LAST_SMS;
				}
				else
				{
					mode = FIRST_FOLLOW_SMS;
				}
				lgth = RecData[0]- 0x08- cplLength;  //CPL CHL SPI KID KID TAR 不参与加密
				
				lastLgth = lgth%blocksize; 
				
				if(lastLgth != 0x00)   //本条不是0x10的整数倍的长度，此数据不参与本次的解密，先保存起来
				{
					lgth = lgth - lastLgth;
						//memmove(&RecData[DecrypReoffset+8], &RecData[udl+1-lastLgth], lastLgth);
					memmove(&RecData[DecrypReoffset+blocksize], &RecData[udl+1-lastLgth], lastLgth);  //保存不完整的数据
				}
				
// 				if(((spiKicKid[2]&0x0C) != 0x0C) && ((spiKicKid[2]&0x01) == 0x01) )  //des in CBC
// 				{
// 					memmove(&RecData[DecrypReoffset], (BYTE xdata *)&RecData[udl+1-lastLgth-8], 8);  //保存本次解密前最后8字节数据
// 				}
				
				memmove(&RecData[DecrypReoffset], (BYTE xdata *)&RecData[udl+1-lastLgth-blocksize], blocksize);  //DES时候不管 ECB还是CBC都保存,AES目前都是CBC
				
				result = ToolkitDeChipherRe(spiKicKid[2],&RecData[9+cplLength],&RecData[9+cplLength],MODE_DECRYPT,lgth,mode);  //从CNTR开始解密

			    if(result == FALSE)
				{				
					if(processResult == POR_OK)
					{
						processResult = RFMSecurityLevel;
					}
					
				}
		
				RecData[0] = lgth + 0x08+cplLength;
	
				memcpy(counter, &RecData[9+cplLength], 0x05);                   //counter

				WriteLFBuf(fileAddr, recNo,17,&RecData[0], RecData[0]+1);		//LV格式记录解密后的文件 CPL,CHL,...

				usedSize = RecData[0];
			
			}
			else
			{
				if(total == i)
				{
					mode = LAST_SMS;
				}
				else
				{
					mode = FOLLOW_SMS;
				}
				
				lgth = udl + lastLgth; 		//本次数据的长度，EF_ToolkitRFMSms文件的记录长度必须预留7字节 ，用于保存上一条短信未计算完成的数据 防止数据溢出 20161122 lgy
				
				if(lastLgth != 0x00)
				{
					memmove((BYTE xdata *)&RecData[1+lastLgth], (BYTE xdata *)&RecData[1], udl); //将上次剩余的未解密的数据和该次数据拼在一起。
					
					//memcpy((BYTE xdata *)&RecData[1], (BYTE xdata *)&RecData[D_TmpBufStart+8], lastLgth);
					
					memcpy((BYTE xdata *)&RecData[1], (BYTE xdata *)&RecData[DecrypReoffset+blocksize], lastLgth);
				}
				
				lastLgth = lgth % blocksize;

				if(lastLgth != 0x00)
				{							
					lgth = lgth - lastLgth;
					
					//memmove(&RecData[D_TmpBufStart+8], &RecData[1+lgth], lastLgth);
					memmove(&RecData[DecrypReoffset+blocksize], &RecData[1+lgth], lastLgth);  //保存不完整的数据
				}

// 				if((spiKicKid[2]&0x02) != 0x02)
// 				{
// 						memcpy((BYTE xdata *)&RecData[D_TmpBufStart+16], (BYTE xdata *)&RecData[1+lgth-8], 8);	//将此次的原始数据的后8个字节放入BUFFER中。
// 				}

				memcpy((BYTE xdata *)&RecData[DecrypReoffset+blocksize*2], (BYTE xdata *)&RecData[1+lgth-blocksize], blocksize);	//将这一次后最小块 保存 CBC时候异或
				
				result = ToolkitDeChipherRe(spiKicKid[2],&RecData[1],&RecData[1],MODE_DECRYPT,lgth,mode);

				if(result==FALSE)
				{
					if(processResult == POR_OK)
					{
						processResult = RFMSecurityLevel;
					}
					
				}
							
				if((((spiKicKid[2]&0x0C) != 0x0C) && ((spiKicKid[2]&0x01) == 0x01)) || ((spiKicKid[2]&0x02) == 0x02))  //des in CBC  或者 AES
				{
					for(result=0; result<blocksize; result++) 											//解密后的头8个字节要做异或操作。
					{
						RecData[1+result] = RecData[1+result] ^ RecData[DecrypReoffset+result];
					}

					memcpy((BYTE xdata *)&RecData[DecrypReoffset], (BYTE xdata *)&RecData[DecrypReoffset+blocksize*2], blocksize); //将这一次最后一小块  移走 好和下一块的异或 并保存下一块的最小块				
				}
						
				SetLFConst(fileAddr, recNo, 17, lgth, sizeof(BYTE));				
			
				WriteLFBuf(fileAddr, recNo,18,&RecData[1], lgth);
				
				usedSize += lgth;				
			}			
		}
		else
		{
			usedSize += RecData[0];
		}	
		
	}

	offset  = 0x00;

	offset = ToolkitProcessHeader(headerNo,usedSize+3,0x01);  //3为027000的长度

	G_formatedSPIOffset = offset + cplLength+1;	    //指向当前envelopdata的SPI字段

	//todo 将解密后的数据写入envelophandlerbuf中，在该结构体中进行mac计算

	usedSize  = 0x00;

	nextOffset = offset;

	for(i =1; i<= total; i++)
	{
		recNo = recNoBuf[i];

		RecData[0] = ReadLFFileValue1ByteLen(fileAddr,recNo,17,&RecData[1]);

		WriteTFBuf(G_EnvelopHanderAddr,nextOffset,&RecData[1],RecData[0]); 

		nextOffset += RecData[0];
		
		usedSize += RecData[0];	
	}

	if(processResult!=POR_OK)
	{
		return processResult;
	}

	//通过检查pctrn的值和padding bytes来判断解密是否成功,by xwh 20140627
	if((spiKicKid[0]&0x04) == 0x04)  //Ciphering
	{
		ReadTFBuf(G_EnvelopHanderAddr,offset+cplLength+1+4+3+5,&pctrn,sizeof(BYTE)); //skip CPL,CHL,SPI 1,SPI2,KIC,KID,TAR,COUNTER
	
		if(pctrn > (blocksize-1))	//padding 数据可以不为0 ，规范没有明确说明 ，相关测试用例：移动RFM&RAM测试脚本_2G3G结合/CMCC_RFM_test/RFM_TEST(2G)/5.15_SecurityMechanismCommandPacket003

		{
			return CipheringError;
		}
	
	}

	//todo :计算mac
	if((spiKicKid[0]&0x03) != 0x00)  //mac or cc	, Digital Signature前面已经排除了
	{
		ReadTFBuf(G_EnvelopHanderAddr,offset + 14 + cplLength,&mac[0],CcLength);	 //拷贝解密后的mac

		memset(cc,0x00,sizeof(cc));
			
		result = ToolkitCalculateMac(FindTFAddr(G_EnvelopHanderAddr,offset,0),spiKicKid[0],spiKicKid[2],spiKicKid[3],cplLength,CcLength,usedSize,cc);
		
		if((result==FALSE)||(memcmp(mac,cc,CcLength)!=EQUAL))
		{					
			return CCFailed;						
		}
	}

	result = FormattedRFMSMSCheckCounter(&spiKicKid[0],counter);   //对于toolkit app的安全报文头，同样检查counter ，因为没有规范说明对于toolkit app的报文头不检查counter的

	if(result != POR_OK)
	{			
		return result;			
	}
	
	
	ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = nextOffset;

	//把EF_ToolkitRFMSmsHead文件中的非02/06/0B TLV读到G_EnvelopHanderAddr中, by xwh 20130830

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);
			
	recLength = GetLFRecNumLgth(fileAddr,FileLgth);
	
	ReadLFBuf(fileAddr, headerNo, recLength-1, &RecData[0], sizeof(BYTE));  //非02/06/0B TLV总长度
	
	if((RecData[0] != 0x00)&&(RecData[0] != 0xFF))
	{
		ReadLFBuf(fileAddr,headerNo,recLength-1-RecData[0],&RecData[1],RecData[0]);

		WriteTFBuf(G_EnvelopHanderAddr,ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength,&RecData[1],RecData[0]); //写"000E0D08000000288705000000000100"

		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength += RecData[0];						
	}

	return offset;  //返回从CPL开始的数据在envelop handler 中的偏移
}
/*
*@description:	计算下行数据的MAC，其数据缓存在tempBuf中，LV结构
*
*@param        :KID  ,
*@param        :macBuf: 缓存mac
*@param        :macLength :待缓存的mac长度
*
*@return	   :void
*
*@author       :create by xuyz
*
*@history	   :V1.0: create by xuyz
*              :modify by xuweihua@20210310  AES算法key修改成4字节对齐（270改成272）
*/
BYTE Toolkit_CalPORMac(BYTE spi,BYTE kid,BYTE *macBuf,BYTE macLength,BYTE * buffer,WORD length)
{
	BYTE idata pcntr,k;
	BYTE xdata cpKey[0x20],cpResult[0x08],cpEnter[0x08];
	WORD idata i,j;
	DWORD xdata Crc32;
	#define TempBuf_Offset_4Bytes_Allign 272
	
	BYTE idata 	kenlen;	
	
//	if((kid & 0x02) == 0x02) //AES If b2 b1 = 10 (AES), b4 b3 shall be coded 00: AES in CBC mode 
//	{
//		kenlen = GetKicKidKikKey(kid,&cpKey[0],TYPE_KEY_KID,0);
//		
//		if(CheckAesKeyLength(kenlen) == FALSE)
//		{
//			return FALSE;
//		}
//		
//		KeyExpansion(&cpKey[0],&tempBuf[TempBuf_Offset_4Bytes_Allign]);     //用后半部分存扩展密钥

//		ToolkitForAESCalculateMac((UWORD)buffer,0x00,length,0x02,&tempBuf[TempBuf_Offset_4Bytes_Allign],macBuf);
//		
//		return TRUE;

//	}	

	if((spi&0x0C) == 0x04)  //CRC32
	{
		Crc32 = 0xFFFFFFFF;	

		for(j=0; j<length; j++)
		{						
			Crc32 = lGenCRC32(Crc32,buffer[j]);
		}

		Crc32 = ~Crc32;    
	
	    for(k=0; k<0x04; k++)
	    {
	       macBuf[3-k] = Crc32 >>(k*8);
	    }

		return TRUE;
	}

	pcntr = GetKicKidKikKey(kid,cpKey,TYPE_KEY_KID,0);

	if(pcntr == FALSE)
	{
		return FALSE;
	}

	pcntr = 0x00;

	if((length&0x07) != 0x00) //计算当前填充数据
	{
		pcntr = 8 - (length &0x07);
	}

	memset(&buffer[length], 0x00, pcntr);

	length += pcntr;

	memset(cpResult,0x00, 0x08);

	for(i = 0x00; i< length; i+=8)
	{
		memcpy(cpEnter,&buffer[i],0x08);
	
		for(k=0;k<0x08;k++)
		{
		  	cpEnter[k]=cpEnter[k] ^ cpResult[k];
	    }
		 
		if((kid&0x0C)==0x00)        //DES CBC
		{
			SingleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08); 
		}
		else if((kid&0x0C)==0x04)   //3DES 2key CBC
		{
			TripleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08,16);
		}
		else if((kid&0x0C)==0x08)   //3DES 3key CBC
		{
			TripleDES(&cpEnter[0],&cpKey[0],&cpResult[0],CBC_Encrypt,0x08,24);
		}
		else  //Reserved
		{
			return FALSE;
		}

	}

	memcpy(macBuf,cpResult,macLength); 	

	return TRUE;
}
/*
*@description:拼装SMS data
*
*@param        :offset  
*@param        :j  
*@param        :smsNum  
*@param        :referNumber  
*
*@return	   :void
*
*@author       :create by xwh 
*
*@history	   :V1.0: create by xwh 20151215,该函数为解决编译器闪退问题而增加
*/
void AddNextMessage(BYTE offset,BYTE j,BYTE smsNum,BYTE referNumber)
{
	if(j<smsNum)  //非最后一条
	{
		tempBuf[0] = ReadVarible(Tag_BakSecondSmsData,&tempBuf[1]);	 //剩余的要发送数据
		RecData[offset] = POR_DATA_MAX_SUBMIT;	
		RecData[offset+1] = 0x05;	
		RecData[offset+2] = 0x00;	
		RecData[offset+3] = 0x03;	
		RecData[offset+4] = referNumber;	  //reference number
		RecData[offset+5] = smsNum;	          //总条数	
		RecData[offset+6] = j;                //当前条数						
		offset += 7;
		
		memcpy(&RecData[offset],&tempBuf[1],POR_DATA_MAX_SUBMIT-6);
		offset += (POR_DATA_MAX_SUBMIT-6);	
		
		tempBuf[1+POR_DATA_MAX_SUBMIT-6-1] = tempBuf[0]-(POR_DATA_MAX_SUBMIT-6);
		WriteVariable(Tag_BakSecondSmsData,&tempBuf[1+POR_DATA_MAX_SUBMIT-6-1]);	
	}
	else         //最后一条
	{
		tempBuf[0] = ReadVarible(Tag_BakSecondSmsData,&tempBuf[1]);
		RecData[offset] = 6+tempBuf[0];	
		RecData[offset+1] = 0x05;	
		RecData[offset+2] = 0x00;	
		RecData[offset+3] = 0x03;	
		RecData[offset+4] = referNumber;	  //reference number
		RecData[offset+5] = smsNum;	          //总条数	
		RecData[offset+6] = smsNum;           //当前条数						
		offset += 7;
		
		memcpy(&RecData[offset],&tempBuf[1],tempBuf[0]);
		offset += tempBuf[0];
	}

	RecData[1] = offset - 2;
}
/*@Function:    GetCcLength                                           
 * 
 *@Descrip:     根据SPI KIC KID 的值获取CC 长度
 *
 *@See:	        ts_143019v050600p.pdf,TS 23.048
 *                                                                                                                                     
 *@Param:        spi0  --- SPI第一字节，与msl进行比较
 *@param        :*buf: Minimum SPI1
 *				                                                                                                         
 *@Return:      TRUE(检查通过)/FALSE（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131018
 *  
 *@History:     V0.0---Create by XWH 20131018             
 */ 
BYTE GetCcLength(BYTE spi1,BYTE kid)
{
	BYTE  idata ccLen;

	if((spi1&0x03) == 0x03)    //Digital 	Signature  ,暂时不支持
	{
		ccLen = 0xFF;
	}
	else if((spi1&0x03) == 0x01)  //Redundancy Check
	{
		ccLen = 0x04;
	}
	else if((spi1&0x03) == 0x02)  //加密校验
	{
		if((kid&0x03)==0x03)	    //9797 MAC 3
		{
			ccLen = 0x04;
		}
		else                        //DES,9797 MAC 4
		{
			ccLen = 0x08;
		}
	}
	else
	{
		ccLen = 0x00;
	}

	return ccLen;
}

/*@Function:    GetBlockNumber                                           
 * 
 *@Descrip:     计算分块数
 *
 *@See:	        ts_143019v050600p.pdf,TS 23.048
 *                                                                                                                                     
 *@Param:        spi0  --- SPI第一字节，与msl进行比较
 *@param        :*buf: Minimum SPI1
 *				                                                                                                         
 *@Return:      TRUE(检查通过)/FALSE（检查不通过）                                                                                                                                                                                                                                                                
 * 
 *@Author:      XWH 20131018
 *  
 *@History:     V0.0---Create by XWH 20131018             
 */ 
BYTE GetBlockNumber(WORD length,WORD size)
{
	return  ((length/size) + ((length%size)==0?0:1));
}
/*-------------------------------------------------------------------------------------------------------------------                                                                                       
* @Function     	    :AddSmsHeaderFromPorConfigFile
*  
* @Discription        :根据配置，从文件EF_POR_HEADER读取TPDA,MTI,PID,DCS
*
* @See                :
*                                                                                                                                                
* @Parameter          :void                                                                  					                                                                                                             
*
* @Return             :void
*                                                                                                                                                        
* @History		        :V1.0 Creat by lgy  
*
*                     :V1.1 初始化COS配置区的内容为全FF 在InitCardInfo 函数中执行，此处不再需要, by xwh 20180504	  
 -------------------------------------------------------------------------------------------------------------------*/  
BYTE AddSmsHeaderFromPorConfigFile(void)
{
	BYTE  idata cclength;	
	BYTE  idata offset;
#if UICC__CDMA__SMS__DOWNLOAD
	UWORD xdata cdmaHeader;
#endif	
	RecData[0] = 0x8B;
	RecData[1] = 0x00;
	offset = 0x02;

#if UICC__CDMA__SMS__DOWNLOAD
	cdmaHeader = FALSE; //初始化 如果文件没建  不会到这里
#endif	

#if UICC__CDMA__SMS__DOWNLOAD
	if((G_IsCdmaSms == TRUE)&&(G_cdmaRecNum !=FALSE))
	{
		cdmaHeader = SelectPrivateLFFile(DF_Toolkit,EF_CSMS_Head);
			
		ReadLFBuf(cdmaHeader,G_cdmaRecNum,RFM_GSM_Originating_Address + 1,&cclength ,sizeof(BYTE));  //直接到tpoa字符个数
			
	}
	else
#endif
	{		
		ReadTFBuf(G_EnvelopHanderAddr,G_formatedTPOAOffset,&cclength ,sizeof(BYTE));   //获取TPOA字符个数	：
	}
	cclength = 2+ (cclength/2) +(cclength&0x01);   //tpoa length TPOA字符个数+ TPOA类型 +TPOA 

	RecData[offset++] = 0x41; //TP_MTI-TP_SRR
	RecData[offset++] = 0x00; //TP_MR
#if UICC__CDMA__SMS__DOWNLOAD
	if((G_IsCdmaSms == TRUE)&&(G_cdmaRecNum !=FALSE))
	{
		ReadLFBuf(cdmaHeader,G_cdmaRecNum,RFM_GSM_Originating_Address + 1,&RecData[offset] ,cclength);	
	}
	else
#endif
	{
		ReadTFBuf(G_EnvelopHanderAddr,G_formatedTPOAOffset,&RecData[offset] ,cclength);   //获取TPOA字符个数
	}
	offset += cclength;
#if M2M_EUICC
	if ( (G_TARRecIndex[0] == TarType_ISDR )	|| (G_TARRecIndex[0] == TarType_ISDP))
	{
		GetIsdpPidDcs(RecData+offset);
		
		offset += 2;		
	}
	else
#endif
	{
		RecData[offset++] = 0x00;  //PID, 根据GSM 03.40  9.2.3.9, Note that for the straightforward case of simple MS-to-SC short message transfer the Protocol Identifier is set to the value 0
		RecData[offset++] = 0xF6;  //DCS	
	}
	

	return offset;
}
/*
*@description:拼装UD header
*
*@param        :headerNo  
*
*@return	   :void
*
*@author       :create by xwh 
*
*@history	   :V1.0: create by xwh 20151215,该函数为解决编译器闪退问题而增加
*/
void AddUdHeader(BYTE * buf,BYTE len,BYTE totalNum,BYTE curNum)
{
	buf[0] = len;	              //UDL
	
	buf[2] = 0x00;	
	buf[3] = 0x03;	
	buf[4] = G_LAST_REFNUMBER;	  //reference number
	buf[5] = totalNum;	          //总条数
	buf[6] = curNum;             //当前条数
	
	if(curNum == 1)
	{
		buf[1] = 0x07;	
		buf[7] = 0x70;	
		buf[8] = 0x00;	
	}
	else
	{
		buf[1] = 0x05;
	}
}
/*
*@description:拼装format短信的POR
*
*@param       :errorCode:短信执行成功:0x00/失败：others value
*@mode        :mode :0x01:addtionalData为：G_CurrentComNUM ,SW1 ,SW2  /0X02：AddtionalData存放在EnvelopResponse中	/mode:0x03:extend mode	/
*
*@return      :void
*
*@author      :create by xuyz
*
*@history     :V1.0: create by xuyz
*
*                  V1.1  modify by xwh 20140217
*
*@others      :本函数对以下情况未处理
               1：因POR RESPONSE存放于&tempBuf[21]开始的位置，所以POR的最大长度只支持(256-21) 个字节,对于mode2 mode4 有可能获取的长度超过限制字节长
			   2：对级联的POR未考虑
			   3:字节addtionalDataLgth可能越界使用,如	addtionalDataLgth = P3+3,当P3 > 0xFD时
			   modify by xuweihua@20210625: 8A07文件映射到1A05,为了避免数据冲掉，https处理过程中短信先缓存暂时不处理
*/
void BuildSmsFunction(BYTE curNum,BYTE totalNum,WORD totalLen,BYTE smsMode)
{
	BYTE  xdata buf[0x0D];
	UWORD xdata porDataFileAddr/*,addrConfig*/;
	BYTE  idata offset,num;
	WORD  idata porAddrOffset;

	//porDataFileAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);
	
	InitProactiveCommand(CMD_SENDSHORTMESSAGE,0x00,DEV_NETWORD);

	//加入SMSC，by xwh 20140324
	buf[0] = TLV_ADDRESS;
	buf[1] = 0xFF; 
#if M2M_EUICC
	if ( (G_TARRecIndex[0] == TarType_ISDR )	|| (G_TARRecIndex[0] == TarType_ISDP))
	{	
		GetIsdpSmsc(&buf[1]);
	}
	else
#endif
	{
		porDataFileAddr = SelectLFFile(DF_TELECOM,EF_SMSP);     //7F10 //TS-Service Centre Address  从6F42文件读取   TP-Destination Address等从下行读取
		
		if(porDataFileAddr == FALSE)
		{
			porDataFileAddr = SelectLFFile(G_DF_ID_USIM,EF_SMSP);	//7FF0
		}
		
		if(porDataFileAddr != FALSE) //不从下行里面获取TS-Service Centre Address
		{
			offset = GetLFRecNumLgth(porDataFileAddr,FileLgth);
			
			ReadLFBuf(porDataFileAddr, 0x01, offset-15, &buf[1], 0x0C); //读取第一条 L
		}
	}
	

	if(buf[1]<=11)	     //SMSC长度最大11字节
	{
		AddTLV(&ResData[1],&buf[0]); 
	}
	
	porDataFileAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);

	offset = AddSmsHeaderFromPorConfigFile();
	
	if(totalNum == 1)
	{
		RecData[offset] = totalLen;  //UDL	
		
		RecData[offset+1] = 0x02;	
		RecData[offset+2] = 0x71;	
		RecData[offset+3] = 0x00;						
		offset += 4;

		ReadTFBuf(porDataFileAddr,3,&RecData[offset],totalLen-3); 
		offset += (totalLen-3);				
	}
	else 	
	{
		if(curNum == 1)
		{
			porAddrOffset = 3;
			
			
			G_LAST_REFNUMBER++;


			AddUdHeader(&RecData[offset] ,POR_DATA_MAX_SUBMIT,totalNum,curNum);
			RecData[offset+7] = 0x71;
			offset += 9;

			ReadTFBuf(porDataFileAddr,porAddrOffset,&RecData[offset],POR_DATA_MAX_SUBMIT-8); 
			offset += (POR_DATA_MAX_SUBMIT-8);			
		}
		else //if(curNum < totalNum)  //非最后一条		
		{
			porAddrOffset = 3+(POR_DATA_MAX_SUBMIT-8)+(curNum-2)*((POR_DATA_MAX_SUBMIT-6));
			
			num = ((curNum < totalNum)?(POR_DATA_MAX_SUBMIT-6):(totalLen-porAddrOffset));

			AddUdHeader(&RecData[offset] ,6+num,totalNum,curNum);
			offset += 7;
			
			ReadTFBuf(porDataFileAddr,porAddrOffset,&RecData[offset],num); 
			offset += num;
		}									
	}
	
	RecData[1] = offset - 2;	
	AddTLV(&ResData[1], &RecData[0]);	
}
/*
*@description:AESDeChipher AES加解密函数 传进来的文件地址 数据长度为 0x10整数倍
*@param       :srcDstAddr  源数据地址
*             :srcDstOff   计算开始的偏移
*@mode        :aesMod      0x00 加密/0x01解密  这里都是CBC模式
*
*@return      :void
*
*@author      :create by LCZ
*
*/
//void AESDeChipher(UWORD srcDstAddr,BYTE srcDstOff, BYTE aesMod, WORD lgth,BYTE kic)
//{
//    BYTE xdata dataPointer[16];
//	BYTE xdata resultData[16];
//	BYTE xdata keyVlaue[33];  // 16  24  32密钥
//	WORD idata num;
//	BYTE idata i;

//	if((lgth%0x10)!=0)				      // 长度不是16的整数倍,退出
//	{
//		return; 
//	}
//	
//	keyVlaue[0] = GetKicKidKikKey(kic,&keyVlaue[1],TYPE_KEY_KIC,0);
//	
//    if(CheckAesKeyLength(keyVlaue[0]) == FALSE)
//	{
//		return;
//	}
//	
//	KeyExpansion(&keyVlaue[1],tempBuf);

//	memset(dataPointer,0x00,sizeof(dataPointer));

//	memset(resultData,0x00,sizeof(resultData));

//	for(num = 0x00; num < lgth; num += 0x10)
//	{

//		ReadTFBuf(srcDstAddr,srcDstOff+num,dataPointer,0x10);
//		
//		for(i = 0x00; i < 0x10; i++)
//		{	
//			if(aesMod == 0x00)     //加密   CBC
//			{
//				dataPointer[i] ^= resultData[i];
//			}
//			else     //解密  0x01
//			{
//				if(num != 0x00)   //保存解密前数据 与下一块解密出来后异或 这里默认IV都是0x00
//				{
//					resultData[i] = dataPointer[i];
//				}
//				
//			}
//		}
//		
//		//CoreCalcAES(&dataPointer[0],&dataPointer[0], aesMod);
//		
//		AESEncryptForUICC(&dataPointer[0],&dataPointer[0],tempBuf);

//		for(i = 0x00; i < 0x10; i++)
//		{
//			if(aesMod == 0x01)  //解密  0x01
//			{
//				
//				dataPointer[i] ^= resultData[i];
//			}
//			else
//			{
//				resultData[i] = dataPointer[i];    //保存计算结果 与下一块异或
//			}
//		}
//		
//		WriteTFBuf(srcDstAddr,srcDstOff+num,dataPointer,0x10);

//	}

//	   
//}
/*
*@description:拼装format短信的POR
*
*@param       :errorCode:短信执行成功:0x00/失败：others value
*@mode        :mode :0x01:addtionalData为：G_CurrentComNUM ,SW1 ,SW2  /0X02：AddtionalData存放在EnvelopResponse中	/mode:0x03:extend mode	/
*
*@return      :void
*
*@author      :create by xuyz
*
*@history     :V1.0: create by xuyz
*
*              V1.1  modify by xwh 20140217
*			   V1.2  modify by jiangqb 20191029， CMCC需求，RAM/RFM测试在 14返回的数据中子tag 830110时不重发
*			   V1.3  modify by xuxiaojun@20200910 if((errorCode != POR_OK) && ((mode & PORMODE_ISDR) != PORMODE_ISDR))代码不能被放在M2M的宏里面
*			   v1.4 modify by jiangqingbin-20220303, 增加一个传入参数，表示http短信触发结果，不再使用原来的复用SW1，SW2的方式
*
*@others      :本函数对以下情况未处理
               1：因POR RESPONSE存放于&tempBuf[21]开始的位置，所以POR的最大长度只支持(256-21) 个字节,对于mode2 mode4 有可能获取的长度超过限制字节长
			   2：对级联的POR未考虑
			   3:字节addtionalDataLgth可能越界使用,如	addtionalDataLgth = P3+3,当P3 > 0xFD时
*/
void Toolkit_ResponsePack(BYTE errorCode,BYTE mode,WORD additionalLen, BYTE httpTriStatus)			                
{
   	WORD  idata UDLLgth,totalLen,fileSize;
	BYTE  xdata spi[0x0D],cc[0x10];
	BYTE  idata pcntr,cclength;	
//	UWORD xdata addrConfig;//,fileAddrSmsc;
	UWORD xdata porDataFileAddr;
	BYTE  idata sendTimes,i,j;
	BYTE  idata smsNum,additionalLenTLLen;
	BYTE  idata tooLong,blockSize;
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
    BYTE  idata SPI2;
	WORD  idata AddLen;
	BYTE idata add_deltosub_TLen;          //备份additionalLenTLLen长度 当扩展模式Deliver转submit时候 需要第一条就把数据写到Por文件，这个时候G_PorAdditionalLen只是备份响应数据长度 additionalLenTLLen这个长度并没有添加，等数据写到Por文件时候加上这个additionalLenTLLen
#endif

	smsNum = 0x01;
	additionalLenTLLen = 0; //默认是压缩格式  此时为0
	tooLong = FALSE;
	
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT	
	add_deltosub_TLen = 0x00;
	
    if((G_Deliver_T0_Submit & 0x02)==0x02)
	{
		G_EnvelopHanderAddr = G_Back_EnvelopHanderAddr;
	}
#endif
	porDataFileAddr = SelectPrivateTFFile(DF_Toolkit,EF_PorDataFile);
	
	if(porDataFileAddr == FALSE)
	{
		return;
	}
	
	fileSize = GetTFSize(porDataFileAddr);

	ReadTFBuf(G_EnvelopHanderAddr,G_formatedSPIOffset,&spi[0], 0x0C);	    //SPI1:spi[0]   SPI2:spi[1] ，KIC KID	TAR(3bytes) COUNTR(5bytes)
																																 
// 	if(((spi[1]&0x03) == 0x00) ||((spi[1]&0x03) == 0x03))  // No PoR reply to the Sending Entity (SE) OR  Reserved
// 	{
// 		return;
// 	}
	
	if((spi[1]&0x03) == 0x00) // Reserved  也发Por     移动测试SPI 1227  期望06 Por
	{
		return;
	}
	
	//SW1不等于90/9F/61,属于错误情况,by xwh 20140508
	if((errorCode == POR_OK) && ((spi[1]&0x03) == 0x02) && ((SW1 == 0x90)||(SW1 == 0x9F)||(SW1 == 0x61)))	//POR required only when an error has occured
	{  
		return;
	}
#if UICC__FUNCTION__RFMRAM_EXTEND	
	//additionalLen是0时，只能是简单格式，不可能是扩展格式,by xwh 20220303
	if(((G_FormatAndType == FormatType_ExtendAA) ||(G_FormatAndType == FormatType_ExtendAE))&&(additionalLen > 0))
	{
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT 
		
		additionalLenTLLen = ExtendResTagADD(additionalLen);
#else
		if(((spi[1]&0x20) == 0x00)&&(additionalLen>POR_DATA_MAX_DELIVER))
		{
			//additionalLen = POR_DATA_MAX_DELIVER;
			
			additionalLen = IF_ExtendResTagChange(POR_DATA_MAX_DELIVER);
		}
		
		additionalLenTLLen = ExtendResTagADD(additionalLen);
#endif
	}
#endif
	blockSize = 0X00;
	
	if((spi[1]&0x10) == 0x10)	 //blockSize  补位 
	{
		blockSize = 0X08;
		
		if((spi[2] & 0x02) == 0x02)
		{
			blockSize = 0x10;
		}
	}
	
	if((additionalLen+additionalLenTLLen+16+8+blockSize+8)>fileSize)          //UDHL 1 + UDH 2 + CPL 2 + CHL 1 + TAR 3 + CNTR 5 + PCNTR 1 + Status Code 1 + CC cclength 8 + ciper padding 8 + mac padding 8
	{
		additionalLen = (fileSize-16-8-blockSize-8);

		GlobalSetStatus(0x62F1);
	}

#if (!UICC_SUPPORT_POR_DELIVER_TO_SUBMIT)	
	if(((spi[1]&0x20) == 0x00)&&(additionalLen>POR_DATA_MAX_DELIVER))     
	{
		if((additionalLen-3) > 0xFF)
		{
			tooLong = TRUE;                                                                                //SMS-DELIVER-REPORT方式，数据长度大于0xFF, 从G_CurrentRFMEfAddr 读取数据
		}
	
		additionalLen = POR_DATA_MAX_DELIVER;
		mode = PORMODE_COMMON;	
		
		GlobalSetStatus(0x62F1);  //中国移动NFC2.0项目20160415/GP2.2.1_UICC_ComplianceTests(移动版)-20141112\Bug_List_Contact\Bug_List_19_01 要求当deliver por太长时，返回62F1 ，无规范支持，根据测试部张永林描述  ，为保证手机兼容性 ，不建议deliver por支持多条的情况，数据太长时，截断，状态字修改成 62F1
	}
#endif
	
	pcntr = 0x00;

	
	//按照GSMA最新规范要求，RFM执行的结果为error的情况下，POR不带MAC，不加密，by xwh 20131218
	//根据31.111,When receiving a secured Command Packet (as specifi ed in TS 31.115 [41]) requesting a Proof of Receipt 
	//(PoR), the UICC shall verify the authenticity of the sender. If the authentication of the sender fails, no further 
	//processing related to the Proof of Receipt shall take place
	//按照以上规范，counter 校验错不属于安全错误，这种情况下应该带MAC ，加密, by xwh 20201022
#if UICC__OPERATOR__CHINATELECOM
	if((errorCode != POR_OK)&&(errorCode != CNTRLow)&&(errorCode != CNTRHigh)&&(errorCode != CNTRBlocked)&& ((mode & PORMODE_ISDR) != PORMODE_ISDR))
#else
	if((errorCode != POR_OK) && ((mode & PORMODE_ISDR) != PORMODE_ISDR))//SGP.02_v3.1.pdf     2.4.3 SMS  规定SPI00 或者39  ‘39’: PoR with CC and encryption.
#endif
	{
		cclength = 0x00;
	}
	else
	{
#if UICC__FUNCTION__AES		
		if(((spi[1]&0x0C) == 0x08)&&((spi[3]&0x03) == 0x02))
		{
			cclength = GetAESMACLen(spi[3],TYPE_KEY_KID);
		}
		else
#endif		
		{
			cclength = GetCcLength((spi[1]>>2)&0x03,0);
		}			
		
		if(cclength == 0xFF)                 //PoR response with DS applied to it ,卡片暂时不支持
		{
			return;
		}
		if((spi[1]&0x10) == 0x10)	 //PoR response shall be ciphered	 
		{
			pcntr = (blockSize-((cclength+7+additionalLenTLLen+additionalLen)&(blockSize-1)))%blockSize;	 //cclength+additionalLen+7 = CNTR(5bytes) PCNTR(1bytes)  StatusCode(1bytes) +cc+addtional			 
		}		
		
	}
	
	totalLen = 1+2+2+1+3+5+1+1+cclength+additionalLen+additionalLenTLLen+pcntr; //UDHL 1 + UDH 2 + CPL 2 + CHL 1 + TAR 3 + CNTR 5 + PCNTR 1 + Status Code 1 + CC cclength + data + padding

	if(((spi[1]&0x20) == 0x20)&&(totalLen>POR_DATA_MAX_SUBMIT))       
	{			
		totalLen -= 3;                                                     //去掉'027100'
		totalLen -= 132;                                                   //第一条短信内容最大132(140-8)'0700030102017100'
		smsNum = 1+ GetBlockNumber(totalLen,134);   //后续短信内容最大134(140-6)  '050003010202'
	}
	
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
	if(((spi[1]&0x20) == 0x00)&&(totalLen>POR_DELIVER_MULTI))       
	{
		G_Deliver_T0_Submit |= 0x01;        //表示deliver长度过长，先发一条不带数据的deliver,131115第七章 Statue Code为0x0B，接着Submit把数据发送出去 G_Deliver_T0_Submit=0x01标志此时deliver响应数据过长
		G_PorAdditionalLen = additionalLen;
		
		G_Back_EnvelopHanderAddr = G_EnvelopHanderAddr;       //此时备份地址    G_Deliver_T0_Submit 02时候还原
		add_deltosub_TLen = additionalLenTLLen;
		
		SPI2 = (spi[1] | 0x20); //
		additionalLen = 0x00;//第一条deliver不带数据  027100001312B000100000000395000B9474FC2A8C0DA077

		additionalLenTLLen = 0x00;

		WriteTFBuf(G_EnvelopHanderAddr, G_formatedSPIOffset+1, &SPI2, 0x01);      //把SPI2 关于deliver/submit的bit为修改
		
	}
#if UICC__OPERATOR__CHINATELECOM 	
	if(!((errorCode != POR_OK)&&(errorCode != CNTRLow)&&(errorCode != CNTRHigh)&&(errorCode != CNTRBlocked))) //和上面的判断mac等对应
#else
	if(errorCode != POR_OK)
#endif
	{
		if((spi[1]&0x10) == 0x10)	 //PoR response shall be ciphered	 
		{
			pcntr = (blockSize-((cclength+7+additionalLenTLLen+additionalLen)&(blockSize-1)))%blockSize;	 //cclength+additionalLen+7 = CNTR(5bytes) PCNTR(1bytes)  StatusCode(1bytes) +cc+addtional			 
		}		
	}
#endif	
	//从tempBuf[3]开始存放 CPL, CHL, TAR, conuter, pctr, status code, cc, data , padding, totalLen为总长度 
	/*see 31115,In order to achieve a modulo 8 length of the data before the RC/CC/DS field in the Response Header, 
	the RPL, the RHL and three octets set to '02' '71' '00', which precede the RPL, shall be included in 
	the calculation of RC/CC/DS if used. These fields shall not be ciphered*/
	totalLen = 0x00;
	
	tempBuf[0] = 0x02;
	tempBuf[1] = 0x71;
	tempBuf[2] = 0x00;  // 计算mac的数据参考规范TS23.048 chapter 6.4,另外TS31115 4.5章节也明确规定027100参与mac计算	
	totalLen += 3;

	UDLLgth = 1+3+5+1+1+cclength+additionalLen+additionalLenTLLen+pcntr; //CHL 1 + TAR 3 + CNTR 5 + PCNTR 1 + Status Code 1 + CC cclength + padding pctr
	tempBuf[3] = (UDLLgth>>8)&0xFF;	     //CPL high
	tempBuf[4] = UDLLgth&0xFF;           //CPL low
	totalLen += 2;

	tempBuf[5] = 0x0A+cclength;          //CHL
	memcpy(&tempBuf[6],&spi[4],0x08);	 //TAR CNTR

	//如果解密错误，此时COUNTER为乱码，发送5字节00作为COUNTER,by xwh 20140627
	//如果安全级别不满足，此时不解密，COUNTER为乱码，发送5字节00作为COUNTER,by xwh 20140627
	if((errorCode == CipheringError)||((errorCode == RFMSecurityError)&&(spi[0]&0x04)))
	{
		memset(&tempBuf[9],0x00,5);
	}

	
	if((errorCode != POR_OK)
			&&(ValueBit(JcvmConfig_GetConfigUicc(), BIT_UICCConfig_Por_Counter_Clear) == POR_COUNTER_CLEAR))      //根据最新102225POR时候状态字不为OK则counter清0或者不发送POR，目前清counter
	{
		switch(errorCode)
		{
			case CNTRLow:
			case CNTRHigh:
			case CNTRBlocked:			// HB TC1_A6_A8_TestProfile1 counter 错期望counter和下行一致
		    //case CCFailed:  // simapi  mac错误也是期望counter 为0x01  fwk_fws_ouda
				break;
			default:
				memset(&tempBuf[9],0x00,5);
			break;
		}
		//memset(&spi[7],0x00,5); //按照ts_102225v120100p.pdf ,4.1 Protocol for generalised secured packets,either send a Response Packet indicating the error cause without any security being applied to the Response Packet and the Counter (CNTR) field set to zero;
	}
	
	totalLen +=9;

	tempBuf[14] = pcntr;                         //PCNTR
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
    if((G_Deliver_T0_Submit & 0x01)==0x01) //deliver级联第一次 状态字0x0B，第二次进来时候G_Deliver_T0_Submit的bit1不为1，则状态字为参数值，第一次进来拼装和附加数据写一次，第二次进来只写拼装数据
    {
		tempBuf[15] = 0x0B;                  //131115第七章 Statue Code为0x0B
	}
	else
#endif
	{
		tempBuf[15] = errorCode;                     //statusCode
	}
	
	totalLen +=2;
	
	memset(&tempBuf[16],0x00,cclength);          //文件中预留CC内容，计算CC时要跳过该字段
	totalLen +=cclength;
	
	WriteTFBuf(porDataFileAddr,0x00,&tempBuf[0],totalLen);      //将第一部分数据写入文件
#if JAVA_CARD_UICC	
	if(((cclength != 0x00) || ((spi[1]&0x10)==0x10)) && (G_NewSCPKey != FALSE)) //如果更新SCP80 key和当前使用的Key version一样 则需要获取更新前的key value
	{
		GetSCP80KeyValueBeforeUpdate(NULL,MODE_KIC_KID);
	}
#endif
	if((mode&0x7F) == PORMODE_COMMON)    //add data
	{
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
		
		if((G_Deliver_T0_Submit & 0x02)==0x02)       //第二次进来时候，由于第一次已经把数据写到上行POR文件，这里直接从上次写入的位置读取数据
		{
			AddLen = 16 + 8 + 8 +8;

			//数据不重叠，可以调用WriteTFFileNonAtomic
			WriteTFFileNonAtomic(porDataFileAddr,totalLen,porDataFileAddr+AddLen+sizeof(TranFileHead),additionalLen);
			
			totalLen +=additionalLen;

			G_Deliver_T0_Submit = 0x00;             //deliver级联  第二次进来时候 这个变量还原初始值0x00		
		}
		else		
#endif
		{
			if(additionalLen >= 0x03)
			{
				tempBuf[16] = G_CurrentComNUM ;   	//CommandNumber								
				tempBuf[17] = SW1;					
				tempBuf[18] = SW2;
				
				WriteTFBuf(porDataFileAddr,totalLen,&tempBuf[16],3); 
				totalLen +=3;

			if((((additionalLen-3)>0xFF)||(tooLong == TRUE))&&(G_CurrentRFMEfAddr!=FALSE))
			{
				fileSize = GetTFSize(porDataFileAddr);
		
				//VMCopyDataToAddr(porDataFileAddr+sizeof(TranFileHead)+totalLen, fileSize-totalLen, (G_CurrentRFMEfAddr), additionalLen-3,MODE_PROTECT_WRITE);
				VMCopyDataToAddr(porDataFileAddr,totalLen,0,G_CurrentRFMEfAddr,G_CurrentRFMEfOffset, additionalLen-3,MODE_PROTECT_WRITE,1,1);// modify by xuweihua@20220514 同时兼容配置区地址、文件头体分开、文件头体连接这三种情况下数据的移动

				totalLen +=(additionalLen-3);

				G_CurrentRFMEfAddr = FALSE;
				G_CurrentRFMEfOffset = 0x0000;
			}
			else
			{

				WriteTFBuf(porDataFileAddr,totalLen,RecData,additionalLen-3); 
				totalLen +=(additionalLen-3);

			}
		}
			else if (additionalLen == 1)
			{
				tempBuf[16] = httpTriStatus;

				WriteTFBuf(porDataFileAddr,totalLen,&tempBuf[16],1); 
				
				totalLen += 1;
			}

#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
			if((G_Deliver_T0_Submit & 0x01)==0x01)      //第一次进来时候把附加数据写到6F86偏移40字节的位置，第二此进来的时候读取出来
			{
				tempBuf[16] = G_CurrentComNUM ;   	//CommandNumber								
				tempBuf[17] = SW1;					
				tempBuf[18] = SW2;
				
				AddLen = 16 + 8 + 8 +8;
				WriteTFBuf(porDataFileAddr,AddLen,&tempBuf[16],3);
				AddLen += 3;
				
				if(((G_PorAdditionalLen-3)>0xFF)&&(G_CurrentRFMEfAddr!=FALSE))
				{
					//数据不重叠，可以调用WriteTFFileNonAtomic
					WriteTFFileNonAtomic(porDataFileAddr,AddLen,G_CurrentRFMEfAddr,G_PorAdditionalLen-3);				
				}
				else
				{
					WriteTFBuf(porDataFileAddr,AddLen,RecData,G_PorAdditionalLen-3); 					
				}
			}
#endif
		}		
		
	}
#if UICC__FUNCTION__RFMRAM_EXTEND	
	else if((mode&0x7F) == PORMODE_EXTEND)    //拼装extend response	Response Scripting template tag
	{
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT

        if((G_Deliver_T0_Submit & 0x01)==0x01)
		{
			ExtendResValueADD(porDataFileAddr,40,G_PorAdditionalLen);
			G_PorAdditionalLen += add_deltosub_TLen;
		}
		else
#endif
		{
			totalLen = ExtendResValueADD(porDataFileAddr,totalLen,additionalLen);
		}
	}
#endif	
	else if(((mode&0x7F) == PORMODE_EVERESPONSE) ||((mode&0x7F) == PORMODE_AUTO_EVERESPONSE ) ) 
	{
		WriteTFBuf(porDataFileAddr,totalLen,ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer,additionalLen); 
		totalLen +=additionalLen;
	}
	else
	{
		
	}
	
	SetTFConst(porDataFileAddr,totalLen,0x00,pcntr+8);
	
	totalLen += pcntr;   //计算CC/RC/DS的数据长度
	
	if(cclength != 0x00)
	{
		ToolkitCalculateMac(getFileContentAddr(porDataFileAddr),(spi[1]>>2),spi[2],spi[3],0x02,cclength,totalLen,cc);
		
		WriteTFBuf(porDataFileAddr,16,cc,cclength); 
	}

	//按照GSMA最新规范要求，RFM执行的结果为error的情况下，POR不带MAC，不加密，by xwh 20131218
	//根据31.111,When receiving a secured Command Packet (as specifi ed in TS 31.115 [41]) requesting a Proof of Receipt 
	//(PoR), the UICC shall verify the authenticity of the sender. If the authentication of the sender fails, no further 
	//processing related to the Proof of Receipt shall take place
	//按照以上规范，counter 校验错不属于安全错误，这种情况下应该带MAC ，加密, by xwh 20201022	
#if UICC__OPERATOR__CHINATELECOM
	if(((errorCode==POR_OK)||((mode & PORMODE_ISDR) == PORMODE_ISDR )||(errorCode == CNTRLow)||(errorCode == CNTRHigh)||(errorCode == CNTRBlocked))&&((spi[1]&0x10)==0x10))   //SGP.02_v3.1.pdf     2.4.3 SMS  规定SPI00 或者39  ‘39’: PoR with CC and encryption.
#else
	if(((errorCode==POR_OK) || ((mode & PORMODE_ISDR) == PORMODE_ISDR ))&&((spi[1]&0x10)==0x10))   //SGP.02_v3.1.pdf     2.4.3 SMS  规定SPI00 或者39  ‘39’: PoR with CC and encryption.
#endif
	{
#if UICC__FUNCTION__AES		
		if((spi[2] & 0x02) == 0x02)
		{
			AESDeChipher(porDataFileAddr,9, 0x00, totalLen-9,spi[2]);
		}
		else
#endif		
		{
			//ToolkitDeChipherRe(spi[2],(BYTE *)(porDataFileAddr+sizeof(TranFileHead)+9),(BYTE *)(porDataFileAddr+sizeof(TranFileHead)+9),ENCRYPT,totalLen-9,FIRST_LAST_SMS);  //从CNTR开始加密, 这里totalLen-6之后如果不是8的倍数，则后面不会执行加密操作，所以抛异常		
			
			ToolkitDeChipherEe(spi[2],porDataFileAddr,9,MODE_ENCRYPT,totalLen-9);
		}
	}

	if(G_NewSCPKey != FALSE)
	{		
		G_NewSCPKey = FALSE;
	}


	if((spi[1]&0x20) == 0x20)  //SMS -SUBMIT
	{
		sendTimes = 0x03;  //重发次数直接定义为0x03
		
		for(j=1;j<=smsNum;j++)
		{
			BuildSmsFunction(j,smsNum,totalLen,SMS_MODE_FIRST);
			
			for(i=1;i<=sendTimes;i++) 
			{
				if(i > 1)
				{
					BuildSmsFunction(j,smsNum,totalLen,SMS_MODE_RESEND);
				}
				
#if UICC__CDMA__SMS__DOWNLOAD				
				TransSMSFomat(NO_SMSC);
#endif
					
				pcntr = SendProactiveCommand();
	
				if ((pcntr == CMD_Success) || (pcntr == ExitStkPerform))		//CMCC需求，RAM/RFM测试在 14返回的数据中子tag 830110时不重发
				{
					break;
				}			
			}
			//第一条发送失败后，不再发送第二条
			if(pcntr != CMD_Success)
			{
				break;
			}
		}
	}
	else  //deliver
	{		
		if((mode&0x7F) == PORMODE_EVERESPONSE)
		{
			ToolkitHandlerList[Index_EnveResHandler].TLVListLength = totalLen;	
					
			ReadTFBuf(porDataFileAddr,0,ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer,totalLen); 
		}
		else
		{
			G_CurrentRFMResponse = totalLen;

			if(errorCode == POR_OK)
			{
				G_Post9EFlag &= 0xFE;		 //用来标示当前要发9EXX还是9FXX,bit0为0表示状态字为9FXX,
			}
			else
			{					    
				G_Post9EFlag |= 0x01;		 //BIT 0为1 表示状态字为9EXX
			}
		}		
	}
	//G_formatedTPOAOffset = 0;	
}	
/*
*@description:执行当前收满的RFM短信
*
*@param     : mode:smsType    or  00
*@param     :     recNo	 :第一条收满的短信在缓存文件中的记录号 
*
@return     :void
*           V1.0---create function
*           V1.1---据全东反馈，没有任何APDU时返回009000是符合规范的，去掉多余的判断,by xwh 20211126
*/
void ToolkitRFMExcute(BYTE smsType,BYTE recNo)
{
	BYTE idata headNo,eventId;
	UWORD xdata fileAddr;
	WORD idata cplOffset;
	BYTE idata errorCode,refrence;
	BYTE xdata tmp[0x20];
	BYTE xdata spi[0x07];  //spi1 spi2 kic kid tar1 tar2 tar3
	BYTE idata isTriHttp;
	
	isTriHttp = NOTGPAnnexBTritag;
	GetEnvelopFirstRecTar(recNo, spi);
	
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
	
	ReadLFBuf(fileAddr,recNo,0x00,tmp,sizeof(tmp));
	
	if((smsType == TYPE_SMS_UPD_SINGLE)||(smsType == TYPE_SMS_ENV_SINGLE))
	{
		refrence = recNo;
	}
	else
	{
		refrence = tmp[1];
	}
	
	headNo = ToolkitFindSpecifiedHeaderNum(MODE_FIND_RECNO,refrence,smsType,&tmp[4]);  //找到当前短信在header中的记录号，可以根据记录号，获取eventid
	
	if(headNo == FALSE)
	{
		return;
	}

	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSmsHead);

	ReadLFBuf(fileAddr,headNo,0x02, &eventId,sizeof(BYTE));
#if UICC__FUNCTION__STKAPI
	if((eventId == EVENT_UNFORMATTED_SMS_PP_ENV_bit) ||(eventId == EVENT_UNFORMATTED_SMS_PP_UPD_bit)) //拼装unformat格式的短信到envelopHandler中
	{
		cplOffset = ToolkitPatchUnformatENVSMS(refrence,headNo,smsType,&tmp[4]);  //拼装envelop数据，触发某个 toolkit应用
	
		ToolkitClearAllData(smsType,refrence,&tmp[4]);		  	//为了防止14不返回时短信头和短信体无法清除，将清除操作提前到这里举行，by xwh 20140702
		
//		if(IF_ExcutePrivateUnformatSMS(cplOffset,&tmp[4]) == TRUE)  //检查unformatsms短信是否需要内部去处理。数据已缓存在G_EnvelopHanderAddr 文件中了
//		{			
//			return;
//		}
		
		G_EnvelopeTag = 0xD1;

		//JavaToolkitProcess(eventId,0);
	}
	else if((eventId == EVENT_FORMATTED_SMS_PP_ENV_bit) ||(eventId == EVENT_FORMATTED_SMS_PP_UPD_bit)) //将format格式的数据拼装到envelophandler buf中
#else
	if(eventId == EVENT_FORMATTED_SMS_PP_ENV_bit) //将format格式的数据拼装到envelophandler buf中
#endif
	{
#if UICC__OPERATOR__CHINAMOBILE
		if(IF_CheckChinaMobileTar(&spi[4]) == TRUE)//处理移动OTA,两步一快功能
		{
			IF_ExcuteChinaMobileFormattedEnvelopSMS(&spi[4],refrence,headNo,smsType,eventId,&tmp[4]);
			
			return ;  //移动报文格式的tar统一采用打标记 处理的方式 
		}	
#endif
		
		cplOffset = ToolkitExcuteFormattedRFMSMS(refrence,headNo,smsType,&tmp[4]);
	
		ToolkitClearAllData(smsType,refrence,&tmp[4]);		  	//为了防止14不返回时短信头和短信体无法清除，将清除操作提前到这里举行，by xwh 20140702

		if(cplOffset <0x0B)     //01-0A	 ,  发送POR
		{			   			  
			errorCode = cplOffset&0x0F;

#if M2M_EUICC
			if(G_TARRecIndex[0] == TarType_ISDR)//  SGP.02_v3.1.pdf /2.4.3 SMS/An incoming SMS for the ISD-R does not meet the security level,it must be rejected by the eUICC and no PoR shall be sent back
			{
				switch(errorCode)
				{
					case CNTRLow:
					case CNTRHigh:
					case CNTRBlocked:
					case RFMNeedMoreTime:
					//case TARUnknown:
						Toolkit_ResponsePack(errorCode,PORMODE_COMMON|PORMODE_RESEND|PORMODE_ISDR,0x00, 0x00);	 
						break;
					default:
						return;
				}
				return;
			}
			else
#endif
			{
				Toolkit_ResponsePack(errorCode,PORMODE_COMMON|PORMODE_RESEND,0x00, 0x00);	  //要注意，如果在STK中接收到了一条Toolkit应用，会不会走到这里？？？ by xuyz
			}
		}
		else  
		{
#if UICC__FUNCTION__STKAPI			
			if(G_TARRecIndex[0] != TarType_ToolkitApplet)  //0表示toolkit applet
#endif			
			{
				//ToolkitProcessRFMRAMWIBApp(cplOffset);
				
				G_Source_Type = SourceType_SMS;
				
				RemoteAPDUProcessFunction(G_EnvelopHanderAddr,cplOffset,0x00,RemoteDataEndBlock, &isTriHttp);
				
				if((G_FormatAndType == FormatType_ExtendAA) || (G_FormatAndType == FormatType_ExtendAE))
				{			
		        	eventId = PORMODE_EXTEND|PORMODE_RESEND;
					
					fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_RAMExtendResponse);
					
					cplOffset = ReadTFFileWord(fileAddr);//ReadTFBuf(fileAddr, ramExtendResLen_offset, (BYTE *)&cplOffset ,0x02);
				}
				else
				{
					eventId = PORMODE_COMMON|PORMODE_RESEND;
					
					cplOffset = G_CurrentRemoteResponseLgth+3;
					
					//if((G_CurrentComNUM == 0x00) && (G_CurrentRemoteResponseLgth == 0x00))
					//使用局部变量isTriHttp来判断是否是http触发短信，因为采用全局变量时有一些情况会考虑不到，避免留下缺陷
					if (isTriHttp != NOTGPAnnexBTritag)
					{
						cplOffset = JcvmConfig_GetAmendmentBTriggerPor();
					}
					//没有任何APDU和主动式命令(CommandStartNum ==0x00),此时不发送command num和SW,by xwh 20211119	(用例:5G测试工程集汇总版/Telecom_ALL/中国电信(国密项目)4G NFC卡备案基本通信功能测试/7.Java功能测试脚本/RAM/155 RAM_6_16xx)
					//只有主动式命令(CommandStartNum !=0x00),此时command num=0，SW=9000,by xwh 20211119				(用例:RFM测试/RFM_Refresh测试)
					//据全东反馈，没有任何APDU时返回009000是符合规范的，去掉以下判断,by xwh 20211126
// #if UICC_SUPPORT_RFMHANDLER_D0					
// 					else if((G_CurrentComNUM == 0x00) && (G_CurrentRemoteResponseLgth == 0x00) && (CommandStartNum == 0x00))
// #else
// 					else if((G_CurrentComNUM == 0x00) && (G_CurrentRemoteResponseLgth == 0x00))
// #endif					
// 					{						
// 						cplOffset = 0;
// 					}
				}
#if UICC__FUNCTION__CATTP
				if(IF_SendPor())
#endif
				{			
					Toolkit_ResponsePack(POR_OK,eventId,cplOffset,isTriHttp);
				}
#if UICC__FUNCTION__STKAPI				
				if(G_Data_Type == DataType_RFM)
				{
					IF_TriggerRemoteFileUpdate();
				}
#endif				
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT
				if(G_Deliver_T0_Submit == 0x00)
#endif
				{
					memset(G_TARRecIndex, 0x00, sizeof(G_TARRecIndex));  //por中 会使用到该变量

					G_FormatAndType = FormatType_NULL;						//还原初值

					G_formatedTPOAOffset = 0;
				}
				
				G_RFMRAMKeyversion = 0xFF;//RFMRAM处理完成后将该变量清为FF 20131128 LJ
			}
#if UICC__FUNCTION__STKAPI			
			else                                                     //触发某个java toolkit
			{								
				G_EnvelopeTag = 0xD1;
				JavaToolkitProcess(eventId,0);
				memset(G_TARRecIndex, 0x00, sizeof(G_TARRecIndex));  //por中 会使用到该变量				
			}
#endif			
		}
	}
	else
	{
		//白盒测试添加  其余event不做处理
	}
}
/*@Function:    ToolkitProcessSMSUpdate                                            
 * 
 *@Descrip:     该函数处理普通短信，包括EVENT_FORMATTED_SMS_PP_UPD, EVENT_UNFORMATTED_SMS_PP_UPD 
 *
 *@See:	        ts_143019v050600p.pdf 6.2 
 *                                                                                                                                     
 *@Param:       NULL 
 *				                                                                                                         
 *@Return:      TRUE---信息已存储/FALSE---空短信或信息格式不合法，未存储                                                                                                                                                                                                                                                                      
 * 
 *@Author:      XWH 20130918
 *  
 *@History:     V0.0---Create by XWH 20130918
 *              V0.1---增加对空短信的判断，by XWH 20130918               
 */
#if UICC__FUNCTION__STKAPI
BYTE ToolkitProcessSMSUpdate(void)
{
	BYTE  idata ptr,mtiptr;
	BYTE  idata result;
	BYTE idata len,udl,tpduLen;
	BYTE xdata tpoa[0x10];
	BYTE xdata Applist[20];
	
	//This event is generated when a Short Message Point to Point (Single or Concatenated) is received by Update Record EFsms APDU(s) and is formatted according to TS 23.048
	if(RecData[0]!=0x03)    //03:message received by MS from network; message to be read
	{
		return FALSE;                                                          
	}

	ptr = 0x01;                                                                //to SMSC length

	if(RecData[ptr]>0x0D)                                                      //SMSC长度非法
	{
		return FALSE;                                                          
	}
	
	ptr += (RecData[ptr]+1);                                                   //to MTI
	
	mtiptr = ptr;

	if((RecData[mtiptr]&0x03)!=0x00)                                           //not SMS-DELIVER type
	{
		return FALSE;
	}

	ptr ++;                                                                    //to TPOA字符个数

	if(RecData[ptr]>20)	                                                       //TPOA长度非法
	{
		return FALSE;                                                         
	}
	
	tpoa[0] = ((RecData[ptr]/2)+(RecData[ptr]%2)+1+1); //len of OA +TPOA类型 +TPOA字符个数 = 总的字节数,如TPOA为 0C81106540001000
	
	memcpy(&tpoa[1],&RecData[ptr], tpoa[0]);   //暂时缓存下行短信的TPOA,包括 TPOA类型，比较的时候不需要比较该字节, 则存储进的数据为 08 0C 81 106540001000,81字节不需要比较
	
	
	ptr += ((RecData[ptr]/2)+(RecData[ptr]%2)+1+1);        //to PID
	ptr += (2+7);   

	result = Check_UDHI(RecData[mtiptr]);
	
	if(result != FALSE)                                                         //unformatted SMS upd
	{
		
		result = FindTagFullMatch(&RecData[ptr+2], 0x70, RecData[ptr+1]);

		if(result != FALSE)                                                         //SMS第一条
		{
			ToolkitSaveFirstFomatData(ptr,EVENT_FORMATTED_SMS_PP_UPD_bit,tpoa);                         //ptr指向UDL
			
			return TRUE;
		}
		else                                                                             //SMS后续条
		{
			//剩余数据用于触发javatoolkit应用
			
			result = FindTagFullMatch(&RecData[ptr+2], 0x00, RecData[ptr+1]);
			
			if(result != 0x00)
			{
				ToolkitSaveRFMHead(RecData[ptr+2+result+1],EVENT_UNFORMATTED_SMS_PP_UPD_bit,ptr,TYPE_SMS_UPD_MULTI,tpoa);  //只有对于index为1的时候，才会更新eventId	
				
				ToolkitSaveRFMSMS(ptr,0x00,TYPE_SMS_UPD_MULTI,tpoa);
				
				return TRUE;
			}
		}

		
	}
	
	len = getApplistByEvent(EVENT_UNFORMATTED_SMS_PP_UPD_bit,0x00,Applist);  
	
	if(len == 0x00)     //先判断有没有对应的事件 否则每次更新文件都更新6F90等  6F90文件寿命会先达到
	{
		return FALSE;
	}
	
	len = 0;

	tempBuf[0] = TLV_DEVICEIDENTITY; 
	tempBuf[1] = 0x02;
	tempBuf[2] = P1;                                     //Absolute Record Number
	tempBuf[3] = RecData[0];                       //Record Status
	len += 4;

	tempBuf[len] = TLV_ADDRESS; 
	memcpy(&tempBuf[len+1],&RecData[1],RecData[1]+1);      //SMSC
	len += (1+RecData[1]+1);
		
	udl = RecData[ptr];
	tpduLen = ptr - mtiptr + 1 + udl; 

	tempBuf[len] = TLV_SMSTPDU;

	if(tpduLen > 0x7F)
	{
		tempBuf[len+1] = 0x81;
		tempBuf[len+2] = tpduLen;
		memcpy(&tempBuf[len+3],&RecData[mtiptr],tpduLen);
		len += (1+1+1+tpduLen);
	}
	else
	{
		tempBuf[len+1] = tpduLen;
		memcpy(&tempBuf[len+2],&RecData[mtiptr],tpduLen);
		len += (1+1+tpduLen);
	}

	WriteTFBuf(G_EnvelopHanderAddr, 0x00, tempBuf, len);

	ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength = len;

	//todo:判断当前是否要触发java toolkit,打标记or直接处理		
	//JavaToolkitProcess(EVENT_UNFORMATTED_SMS_PP_UPD_bit,0);

	return TRUE;
	
}
#endif
/*
 *@function  : NativeProcessRemoteAppManageAPDU
 *
 *@description :该接口主要用于实现由nativecos实现的一些Remote Application Management APDU
 *
 *@param       :void
 *
 *@return      :FALSE:当前指令将在java层处理/else：当前指令在cos层本地已处理
 *
 *@author      :xuyz
 *
 *@history     :V1.0 :create by xuyz@20131211
*/
WORD NativeProcessRemoteAppManageAPDU(void)
{
	WORD idata status;
		
#if COS_UPDATE_ONLINE_BY_NativeOS	
	status  =  NM_IsUPCOSCommand();
	
	if(status != 0)
	{
		return  status;
	}
#endif
	
	switch(INS)
	{
#if UICC__FUNCTION__CATTP
		case 0xEC:      //RAM下来的push指令
			PushCommand();
			status =  0x9000;
			break;
#endif
//		case 0xCA:
//		case 0xCB:			
//			status = ToolkitRAM_GetData();
//			break;		

	    default:
			status = 0x0000;	 	      
	}

   return status;	
}

/*
*@description :处理支持UICC的PP_Download_Handler事件 ;在stk busy时仅处理RFM RAM数据，并且在STKbusy时通过标记处理
*
*@param       :void
*
*@return      :void
*
*@history     :
*/
BYTE code hai_xing_electric[] ={0x30,0x10,0x4E,0x1C,0x4F,0xE1,0x54,0x8C,0x5E,0x73,0x30,0x11};

#if UICC__APP__LIVECARD
BYTE code fixSPIKICKID[] = {0x06, 0x00, 0x05, 0x05};
#endif


#if UICC__OPERATOR__CHINATELECOM__4G
/*
*@description:??è?μ±?°?′DD???tμ??·??
*
*@param      :buf
*
*@return
*
*@history
*/
BYTE RFMGetCurrentEFPath(BYTE * currentPath)
{
	WORD xdata fileName[0x02];
	
	memset(currentPath, 0xFF, 0x08);
	
	if(G_CurrentDfName[G_ChannelNum] != DF_MF)  //μ±?°·??êμ?EFμ???????2?ê?MF￡??ìD?2é?òé???????
	{
		 ReadEeprom((BYTE *)&fileName,G_CurrentDfAddr[G_ChannelNum] + F_ParentName_Offset,2*sizeof(WORD));
		
		 if(fileName[1] != DF_MF)  //èy??????è?êêó?
		 {
		 	currentPath[0] = (fileName[0] >>8)&0xFF;
		 	
		 	currentPath[1] = fileName[0]&0xFF;
		 	
		 	currentPath[2] = (fileName[1] >>8)&0xFF;
		 	
		 	currentPath[3] = fileName[1]&0xFF;
		 	
		 	currentPath[4] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
		 	
		 	currentPath[5] = G_CurrentDfName[G_ChannelNum]&0xFF;
		 	
		 	currentPath[6] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
		 	
		 	currentPath[7] = G_CurrentEfName[G_ChannelNum]&0xFF;
		 	
		 	return 0x06;
		 	
		 }
		 else
		 {
		 	currentPath[0] = (fileName[1] >>8)&0xFF;
		 	
		 	currentPath[1] = fileName[1]&0xFF;
		 	
		 	currentPath[2] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
		 	
		 	currentPath[3] = G_CurrentDfName[G_ChannelNum]&0xFF;
		 	
		 	currentPath[4] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
		 	
		 	currentPath[5] = G_CurrentEfName[G_ChannelNum]&0xFF;
		 	
		 	return 0x04;	 
		 	
		 }		
	}
	else
	{
 	 	currentPath[0] = (G_CurrentDfName[G_ChannelNum] >>8)&0xFF;
	 	
	 	currentPath[1] = G_CurrentDfName[G_ChannelNum]&0xFF;
	 	
	 	currentPath[2] = (G_CurrentEfName[G_ChannelNum]>>8)&0xFF;
	 	
	 	currentPath[3] = G_CurrentEfName[G_ChannelNum]&0xFF;	
	 	
	 	return 0x02;
	}
	
}
/*
*@description: 判断当前正在执行的文件操作是否满足EFFCL文件中所规定的权限
*
*@param      ：efaddr ，action
*
*@return     ：TRUE：允许执行操作/FALSE：权限不满足
*
*@history
*/

BYTE RFMAccessCondition(UWORD fileAddr,BYTE action)
{	
	WORD xdata fileName;
	BYTE idata fileType,i;
	BYTE xdata currentPath[0x08];  //最多支持3级目录，EF FCL文件初始化记录全ff
	UWORD xdata FCLfileAddr;
	BYTE xdata name[0x09];
	
	memset((BYTE *)currentPath, 0xFF, sizeof(currentPath));
	
	i = RFMGetCurrentEFPath(currentPath);
	
	//if(G_CompactExtendedTag == TAR3_SHARE_FILE)  //共享文件系统只能访问MF，MF下的所有DF和EF文件，应用开始运行时，默认当前路径为MF
// 	if ((G_CompactCompressTag == TAR3_SHARE_FILE)		//共享文件
// 		||(G_CompactCompressTag == TAR3_SIM)
// 	 )
	if((G_TARRecIndex[1] == TAR2_SIMFile_System)||(G_TARRecIndex[1] == TAR2_UICCFile_System)) 
	{
		fileName = ((WORD)currentPath[2]<<8)+currentPath[3];
		
		if((fileName == G_DF_ID_CSIM)||(fileName == G_DF_ID_USIM))  // 当前文件为CSIM / USIM / ISIM
		{
			return FALSE;
		}
	}
	
	if(G_CurrentEfName[G_ChannelNum] == FALSE)  //对于删除文件而言，当前拼装的数据经过上述组装可能只是3F000000等结构
	{
		ReadEeprom((BYTE *)&fileName,fileAddr,sizeof(WORD));
		
		currentPath[i] = (fileName>>8)&0xFF;
	 	
	 	currentPath[i+1] = fileName&0xFF;	
	}
	

	if(((action == A_CreateDF)||(action == A_CreateEF))&&(INS == 0xE0))  //创建文件时，不需要在此验证权限
	{
//		if(RFMFindEmptyRecordFCL() == FALSE)  // 创建文件时若FCL文件中没有空记录，则不允许执行创建操作
//		{
//			return FALSE;
//		}
		
		fileType = GetFileHeadByte(fileAddr,F_Type_Offset);

		if( fileType == ADFTYPE )
		{
			return TRUE;
		}
	}
	
	FCLfileAddr = SelectLFFile(DF_MF,EF_FCL);
	
	if(FCLfileAddr == FALSE)
	{
		return TRUE;
	}
	
	if(action == A_ReadFile)
	{
		action = Action_ReadBit;
	}
	else if(action == A_UpdateFile)
	{
		action = Action_UpdateBit;
	}
	else if(action == A_DeleteFile)
	{
		if(INS == 0xD4)  //resize
		{
			action = Action_ResizeBit;
		}
		else
		{
			action = Action_DeleteBit;
		}
	}
	else if(action == A_DeactivateFile)
	{
		action = Action_DeactiveBit;
	}
	else if(action == A_ActivateFile)
	{
		action = Action_ActiveBit;
	}
	
	for(i = 0; i <GetLFRecNumLgth(FCLfileAddr, FileNum) ; i++)
	{
		ReadLFBuf(FCLfileAddr,i+1, 0x00, (BYTE *)name, 0x09);  //读取文件路径ID  
		
		fileName = ((WORD)name[0]<<8)+name[1];
		
		if(fileName != DF_MF)   //卡端获取文件的权限都从MF开始，为防止FCL中某些文件的起始路径不是0x3F00，在此处添加MF一级目录
		{
			memmove(&name[2] ,&name[0],0x06);
			
			name[0] = 0x3F;
			
			name[1] = 0x00;
		}
		
		if(memcmp((BYTE *)name,(BYTE *)currentPath,0x08) == EQUAL)  //在FCL文件中找到当前正在被操作的文件
		{
			return ValueBit(name[0x08], action);
		}
	}
	
	//return FALSE;  //在FCL列表中，未找到相关文件,规范0815版本指令FCL列表中的文件为不可被远程操作的文件，对于不在FCL列表中的文件是否允许远程操作，需求未详细说明，暂时不允许RFM操作不在FCL列表中的文件，待需求明确后修改
	return TRUE;	//不在FCL列表中的文件，都默认允许RFM
}
#endif
/*
*@description:	AccessCondition函数的接口函数
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
// 	WORD idata spaceSize;
// 	WORD     idata binaryFileLgth ;

	if(ValueBit(G_SystemStatus,B_RfmPerform) == TRUE)	//TS102226规范
	{
// 		 if(INS == 0xB2)
// 		 {
// 			if(P3 == 0x00)  //根据ts102226,P3 为0时，可以读取整条记录
// 			{
// 				P3 = GetLFRecNumLgth(fileAddr,FileLgth);
// 			}
// 		}	
// 		else if(INS == 0xB0)
// 		{
// 			if(P3 == 0x00)
// 			{
// 				spaceSize = GetTFSize(fileAddr);

// 				binaryFileLgth = ((P1<<8)&0xFF00)|(P2&0x00FF);

// 				if(spaceSize <=( binaryFileLgth+0x100))  // 当剩余有效数据不足0x100时，返回剩余有效数据	;否则返回0x100字节
// 				{
// 					P3 = spaceSize - binaryFileLgth; 
// 				}
// 			}
// 		}
#if UICC__OPERATOR__CHINATELECOM__4G
		return RFMAccessCondition(fileAddr,action);
#else
		//中国电信5G 不再使用2F08 文件控制RFM 权限, by xwh 20201021
		return 0xFF;
#endif
	}

	return 0xFF;
}
/*-------------------------------------------------------------------------------------------------------//                                                                                                                                               

  Function name :	 RFMHandlerD0                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                   
  					                                                                                                         
  output parameter:  void                                                                                                                
                                                                                                                                                
  description:       ??RFM ??D0 ?????

  Author: 			 xwhua

  status:			 OK

  V1.0                Creat by xwhua 20170413
---------------------------------------------------------------------------------------------------------------------------------------------------*/
#if UICC_SUPPORT_RFMHANDLER_D0
void RFMHandlerD0(BYTE num)
{	
	BYTE  idata buf[3],len,i;

	if(num > 2)
	{
		num = 2;          //最多支持2条主动式命令
	}

	for(i=0;i<num;i++)
	{
		ReadTFBuf(G_EnvelopHanderAddr,CommandStartNum,&buf[0],3);
	
		if(buf[0] != 0xD0)
		{
			break;
		}
		
		len = ((buf[1]==0x81)?(buf[2]+3):(buf[1]+2));
		
		ResData[0] = 0xD0;

		ReadTFBuf(G_EnvelopHanderAddr, CommandStartNum, &ResData[1], len);

		if(ResData[2] == 0x81)                   //in this case,len is "818X" or "8X"?
		{
			memmove(&ResData[2],&ResData[3],ResData[3]+1);
		}
		
		CommandStartNum += len;
	
		SendProactiveCommand();
	}

	SetTFConst(G_EnvelopHanderAddr, 0x00, 0x00, 0x10);

	CommandStartNum = 0x00;
}
#endif
#endif