/*

 * Copyright (c) 2008,eastcompeace

 * All rights reserved.

 * VERSION: V1.26

 * MODEL: 	All model

 * RELEASE: 2008-12-18

 * AUTHER:	郑冬梅

 * HISTROY:	V1.0 创建源文件	
 			v1.1 删除功能 MORENUMBER ，目前在国外3G平台已无该功能;并增加为实现UICC API的接口 @20151113 by xuyz
			V1.2 修改函数IF_ProcessPendingMessageAlert，删除TYPE_SMS_POR_SEND  D_RfmProactiveCommandTag的处理@20151203by xuyz
			V1.3 修改IF_ProcessNoShareableFile，删除对卡片状态的判断，在调用该函数的外部判断是否满足进入该函数的条件。修改非共享文件失选方案，将失选文件的时机放在JAVA应用失选之后
 						
*/

#include <uicc_error.h>
#include  "typedef.h"
#include  "uicc_main.h"
#include  "ceph.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include "Crypto_Constant.h"
#include  "ceph.h"
#include  "interface.h"
#include  "uicc_interface.h"
#include  "FunctionSwitch.h"
#include "stack.h"
#include "SCPInterface.h"
#include "keySet.h"
#include "BIPModuleProcess.h"
#include "cdma.h"
#include "opFunction.h"
#include "Object.h"
#include "Io_Swp.h"
#include "SEEUICCInterface.h"
#if UICC__FUNCTION__STKAPI  //支持Toollkit功能
#include "ToolkitRFM.h"
#include "STKRemoteAppManagement.h"
#include "STKCATRuntimeEnviorment.h"
#include "STKFileView.h"
#include "ToolkitAPIPublic.h"
#include "casdStoreKey.h"
#include "Interface_LTEFromPlatform.h"
#include "TimerModuleProces.h"
#include "cdma.h"
#include "object.h"
#include "ExpandedManage.h"
#include "T0ApduImpl.h"		//remove warning,dengyiwen,2022/9/27
#include "DNS.h"
#if UICC__FUNCTION__UICCAPI
#include "UICCToolkitAPI.h"
#endif
#endif

#if COS_UPDATE_ONLINE_MUTUAL
#include "FunEntryConfig.h"
#include "UpCos.h"
#endif

#include "Interface_AnnexB.h"
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#include "GPAnnexbAdminSessionPara.h"
#endif

#if UICC_USATTEST
#include "ceph.h"
#endif

#include 	"Authentication.h"

#if M2M_EUICC
#include "uicc_M2M.h"
#endif

#if UICC__FUNCTION__CATTP
#include "CatTp.h"
#include "Interface_CATTP.h"
#include "BIPModuleProcess.h"
#endif
#if M2M_ESx
#include "EUICCM2MAPDU.h"
#endif

#if UICC__AUTH__5G__SUSPEND
#include "SuspendUicc.h"
#endif
#include "Arithmatic.h"

#if UICC__APP__WHITECARD
#include "WhiteCardBeWrited.h"
#include "LiveWirteCardXL.h"

BYTE xdata AppWriteResult;
BYTE xdata G_StatusSendSMS;
BYTE xdata G_StatusFirstTime;
BYTE xdata G_StatusSendSMSCounter;
BYTE xdata G_auth;
BYTE xdata G_applySMS;
BYTE xdata G_applyActivate;
BYTE xdata G_SMSCounter;
//BYTE xdata G_FirstAPDU;
#endif


extern void RunAuth(void);//no head file
extern void dataOrder(BYTE size,BYTE *Buf);//no head file
#define OFFSET_ITEMINDEX            150
#define OFFSET_NEXTACTION           100
#define OFFSET_ICONLIST             50
#define OFFSET_TEXTATTRIBUTELIST    100
#define OFFSET_AlphaIcon            230
#define OFFSET_AlphaTextAttribut    234
#define OFFSET_OTAMenuSort          258   //最多可支持的6F18的记录数为sizeof(tempBuf) [511-259]/3 = 84,即最多支持84条有效的6F18记录排序,LV结构

//DWORD   xdata    G_CODE_SIZE;
BYTE   xdata G_SMItemLen;
BYTE   xdata G_IconNum;
BYTE   xdata G_JavaMenuIndex;
BYTE   xdata G_JavaMenuIndexBak;
WORD xdata G_BlockOffset;
BYTE xdata G_NetworkStatus;

BYTE 	xdata	 G_SMLastNum;	//翻页时tempBuf中第一个未排好序的序号
BYTE 	xdata	 G_BackUpK;
BYTE    xdata    G_BackMenuIndex;    //保存第一页的最后一个标识符
BYTE    xdata 	 G_MenuIndex;
BYTE    xdata    G_NextPageTag;      //标记是否需要显示下一页 
BYTE    xdata    G_NextMenuIndex;

#if High_Update_InRAM_New  //不能和High_Update_Function_RAM  同时打开,此开关可以和High_Update_In_HEFlash同时使用
BYTE    xdata G_High_Update_InRAM_new[40];
#endif

BYTE code  prePage[]={0x8F,0x08,D_PrePageItem,0x80,0x4E, 0x0A,0x4E,0x00, 0x98, 0x75}; //菜单上显示“上一页”的编码
BYTE code  lasPage[]={0x8F,0x08,D_NextPageItem,0x80,0x4E, 0x0B,0x4E,0x00,0x98,0x75}; //菜单上显示“下一页”的编码

#if UICC__AUTH__5G__SUPI

BYTE code fileList[]={0x09,0x01,0x3F,0x00,0x7F,0xFF,0x5F,0xC0,0x4F,0x0A};
BYTE xdata G_SUPILEN;

#endif
#if UICC__AUTH__5G__SUSPEND
BYTE xdata G_TerminalCap; //开机是否执行了CAPBILITY的指令
BYTE xdata G_SuspendUiccTag; //是否执行了SUSPENDUICC命令，执行了RESUME 后为FALSE
BYTE xdata G_InvildCommandTag;//在RESUME UICC前，是否执行了非规定的指令（除A4,B0,B2,AA）
#endif
/*
*@description :设置全局的E方地址
*
*@param       :void
*
*@return      :void
*
*@history     :
*/
void IF_EepromAddressSetting(void)
{
    #if defined(S3M228A)
	//G_CODE_SIZE = 0x21000;
    #elif defined(S3FS9FG)
	//G_CODE_SIZE = 0x24800;
	#elif defined(ST33G1M)
	//G_CODE_SIZE = 0x70000;
	#else
	//G_CODE_SIZE = 0x60000;
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
void IF_SendATR(BYTE appVerId[2])
{
	appVerId[0] = 0x00;		//该数组值是对应的应用版本号，根据应用赋值

	appVerId[1] = 0x50;
}
#if UICC__OPERATOR__CHINAUNICOM
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UnicomSetIMSI
  
  Command format:    A0    F4  00   00   12  
  
  Data    content:   IMSI数据1+IMSI数据2    
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:      中国联通现场写卡APDU指令:
					 如果是2G电信智能卡,则IMSI数据1和IMSI数据2是完全一样的,是写入7F20/6F07的数据，若IMSI数据1和IMSI数据2不一致,则IMSI数据写入失败,卡片返回"9804"
					 如果是3G电信智能卡,则IMSI数据1是写入7F20/6F07的数据,IMSI数据2是写入7FF0/6F07的数据

  status:			 TEST    

 -------------------------------------------------------------------------------------------------------------------*/    

void UnicomSetIMSI(void)
{
	UWORD xdata fileAddr;
	UWORD xdata fileAddr2,fileAddr3;

	if (P3 != 0x16)
	{
		P3Error();
		return;
	}

	ReceiveData();

	ecpMem_Set(&tempBuf[0],0xFF,0x09);

	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);

	fileAddr2 = SelectTFFile(0x7FF0, EF_IMSI);			

	ReadTFBuf(fileAddr, 0x00, &ResData[0], 0x09);

	ReadTFBuf(fileAddr2, 0x00, &ResData[10], 0x09);

	if ((ecpMem_Compare(&ResData[0], &tempBuf[0], 0x09) != EQUAL) || (ecpMem_Compare(&ResData[10], &tempBuf[0], 0x09) != EQUAL)) //7F20/6F07或7FF0/6F07非FF
	{
		GlobalSetStatus(0x9804);							//非白卡返回安全性错误
		return;
	}	

	if (fileAddr != FALSE)
	{
		WriteTFBuf(fileAddr, 0x00, &RecData[4], 0x09);	
	}
	
	if (fileAddr2 != FALSE)
	{	
		WriteTFBuf(fileAddr2, 0x00, &RecData[13], 0x09);
	}
	
	fileAddr = SelectTFFile(0x7FF0, 0x5FC0);
	
	if(fileAddr != FALSE)
	{
		fileAddr3 =  SelectFile(fileAddr, 0x4F0A);
	}
	
	if (fileAddr3 != FALSE)//更新Routing_Indicator
	{	
		WriteTFBuf(fileAddr3, 0x00, &RecData[0], 4);
	}

	CommandFinish(0x00);

}



/*
 *@Function: RsaCaculateNE_ND
 *
 *@Descrip:  :RSA计算  Result = A^D mod N /Result = A^E mod N, A为待运算数据，Result为输出	 ，在该函数中用同一个参量  In_OutPut
 *
 *@See:
 *
 *@Param:    BYTE* Module_N	   模的地址
 *
 *@Param:    BYTE* KeyValue_ND 私钥D或公钥E的地址

 *@Param:    BYTE* In_OutPut   待加密数据/加密后输出数据

 *@Param:    WORD KeyLgth	   私钥D或公钥E的长度 （E的长度一般为3，也可以等于模长(前面补0)，D的长度等于模长）

 *@Param: 	 WORD dataLgth     待加密数据长度 （等于RSA密钥对的模长）-- ST芯片底层api暂时只支持长度为16字节的整数倍密钥对的运算（后续可能会将模长不为16字节整数倍的密钥对拼装成16字节整数倍后再调用底层api运算--暂未实行）

 *@Return:   FALSE（失败） / TRUE（成功）
 *
 *@Author:   ligy @20141201
 *
 *@History:  V1.0--
 */
BYTE RsaCaculateNE_ND(BYTE* Module_N, BYTE* KeyValue_ED, BYTE* In_OutPut, WORD KeyLgth ,WORD dataLgth)
{
	
	
	PX8					ptrDataOut;
	u16					OutLength;

  u8 xdata dataIn[0x100];
	
	if(dataLgth != 0x80)
	{
		return FALSE;
	}

	//1024bit public key operation
	RSA_storeMEKey(MEMtPM8(KeyValue_ED), MEMtPM8(Module_N), KeyLgth, 0x80, TYPE_RSA_PUBLIC);
	RSA_Set_InData(dataIn);
	ecpMem_Copy(dataIn, In_OutPut, dataLgth);
	RSA_Set_InDataLength(dataLgth);
	RSASTDCore();

	OutLength =  RSA_Get_OutDataLength();
	ptrDataOut = RSA_Get_OutData();
	if(OutLength != 0x80)
	{
		return FALSE;
	}

	ecpMem_Copy(In_OutPut,ptrDataOut,dataLgth);

	return TRUE;
}



/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 UnicomResetIMSI
  
  Command format:    A0    F4  00   01   08  
  
  Data    content:   data  
                                                                                                                                                
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:       中国联通现场写卡IMSI重置
					 通过该指令恢复USIM卡的IMSI文件(3F00/7F20/6F07和3F00/7FF0/6F07)的内容为0xFF FF...FFh 
					 

  status:			    

 -------------------------------------------------------------------------------------------------------------------*/    
void UnicomResetIMSI(void)
{
	UWORD xdata fileAddr,fileAddr3;
	BYTE idata tag;
	BYTE xdata key[8];
	KeyStructer xdata  pinKey;
	PukStructer xdata	 pukInfo;
	
	if (P3 != 0x08)
	{
		P3Error();
		return;
	}

	ReceiveData();
	
	fileAddr = SelectTFFile(DF_MF, EF_LoginNetFlag);
	
	ReadTFBuf(fileAddr, 0, &tag, 1);
	if(tag == 0x01)//卡片曾经成功登陆过网络
	{
		//SW1 = 0X98;
		//SW2 = 0X10;
		InvalidationContradiction();

		return;
	}
	
	fileAddr = SelectTFFile(G_DF_ID_USIM, EF_KI);
	
	ReadTFBuf(fileAddr, 8, &key[0], 8);
	
	ReadKey(PIN2, (BYTE *)&pinKey.KeyId);
	
	ReadOrWritePukInfo(pinKey.pukRef,(BYTE*)&pukInfo.PukId,Read_Key);
	
	ecpMem_Copy(&ResData[0], (BYTE *)&pukInfo.PukContent[0], 8);
	
	SingleDES(&ResData[0], &key[0], &ResData[0], CBC_Encrypt, 8);	
	
	if(ecpMem_Compare(&ResData[0], &RecData[0], 0x08) != EQUAL)//PUK2比对失败
	{
		GlobalSetStatus(0x9804);
		return;
	}
	
	fileAddr = SelectTFFile(DF_PHASE, EF_IMSI);
	
	SetTFConst(fileAddr, 0, 0xFF, 9);
	
	fileAddr = SelectTFFile(0x7FF0, EF_IMSI);
	
	SetTFConst(fileAddr, 0, 0xFF, 9);
	
	fileAddr = SelectTFFile(0x7FF0, 0x5FC0);
	
	if(fileAddr != FALSE)
	{
		fileAddr3 =  SelectFile(fileAddr, 0x4F0A);
	}
	
	if(fileAddr3 != FALSE)
	{
		SetTFConst(fileAddr3, 0, 0xF0, 1);
		SetTFConst(fileAddr3, 1, 0xFF, 3);//恢复为F0FFFFFF
	}
		
	
	CommandFinish(0x00);
}

/**
 * @brief 		IF_UpdateBinary
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
#if UICC__OPERATOR__CHINAUNICOM
	UWORD idata fileAddr;
   
	if(G_CurrentEfName[G_ChannelNum]==0x6F7E)	//对STK流程中更新文件的动作，不执行应用
	{
		if(RecData[P3-1] == 0x00)
		{		
			fileAddr = SelectTFFile(DF_MF, EF_LoginNetFlag);
			
			if(fileAddr == FALSE)
			{
				return FALSE;
			}
	
			SetTFConst(fileAddr, 0, 0x01, 1);		  //更新标识位，表示已经登录过网络
				
		}
	}
    return FALSE;	
#endif
	
	return TRUE;

}

u16 getKeyDataBykeyVersionAndType(u8 sdId, u8 keyVersion, u8 keyId, u8 keyType, BYTE* keyDataBuffer)
{
	jref				keySetRef;
	PN8					sdDataObject;
	PN8					keySetObjcet;
	PR32				keyDataAField;
	u16 keyLen = 0;	

	sdDataObject		=	MEMtPN8(getSdDataObjBySdid(sdId));	
	keySetRef			=	keySet_getKeySet(sdDataObject, keyVersion);
	if (REF_NULL == keySetRef)
	{
		return keyLen;
	}
	keySetObjcet		=	MEMtPN8(locateObjectInMemory(keySetRef));
	//keyType				=	keySet_getKeyType(keySetObjcet, keyId, NO_ERROR);

	if (REF_NULL == keyType)
	{
		return keyLen;
	}

	keyLen		= getKeyDataByIdforSCP80(keySetObjcet, keyId, keyType, MEMtPR32(&keyDataAField));

	if(keyDataBuffer != NULL)
	{
		memCopyNoAtomic(keyDataBuffer, MEMtPM8(keyDataAField), keyLen);

		memRefresh();
	}

	return keyLen;
}

#endif
/**
 * @brief 		对扩展指令的处理，可自行添加扩展指令
 *
 * @author 		xyz
 *
 * @history 	created by zxl, @071220 20:01
 */
void IF_ReceiveAPDU(void)
{
#if UICC__OPERATOR__CHINAUNICOM	
	BYTE idata i;
	UWORD xdata keyAddr;
	WORD idata keyLen;
		BYTE  xdata exponent[0x04] = {0x00, 0x00, 0x00, 0x00};
	BYTE  xdata keyValue[0x10];
#endif
	
	switch (INS)
	{
		case 0x88:
#if UICC__APP__WHITECARD		
			G_auth = 0x01;
#endif			
			RunAuth();
			break;
#if (UICC__AUTH__MBMS || UICC__AUTH__LOCALKEY)
		case 0x89:
			RunODDAuth();
			break;
#endif
		
#if UICC__OPERATOR__CHINAUNICOM		//中国联通现场写卡APDU指令，目前仅支持USIM写卡指令
	    case 0xF4:
//			if(CLA != 0xA0)		//现场写卡规范中CLA为0xA0
//			{
//				ClaError();
//				break;
//          }


			if(((CLA== 0xA0)|| (CLA== 0x00))&&(P1 == 0x00)&&(P2 == 0xF0))
			{
				#define modulus tempBuf   //局部变量压栈会占用过多的SP 空间
				
				if(P3 != 0x88)
				{
					P3Error();
					return; 
				}
	
				ReceiveData();
				
				keyLen = getKeyDataBykeyVersionAndType(ISD_APPID, KEY_TOKEN_ID1, KEY_ID_01, KEY_TYPE_RSA_PIVATE, (BYTE*)modulus); //查找ISD RSA TOKEN KEY的密钥

				if(keyLen == 0)
				{
					ReferencedDataNotFound();
					return;         //如果当前sdID没有put token key应该如果操作
				}
				
				keyLen = getKeyDataBykeyVersionAndType(ISD_APPID, KEY_TOKEN_ID1, KEY_ID_01, KEY_TYPE_RSA_PUBLIC, (BYTE*)exponent);//eg:获取的值为01000100
				
				if(keyLen == 0)
				{
					ReferencedDataNotFound();
					return;         //如果当前sdID没有put token key应该如果操作
				}
				
				if(FALSE == RsaCaculateNE_ND(modulus, exponent, &RecData[0x00], keyLen ,0x80))
				{
					SecuritySatisfied();
					 return;
				}
				
				keyLen = getKeyDataBykeyVersionAndType(ISD_APPID, KEY_RECEIPT_ID1, KEY_ID_01, 0x80, keyValue);//keyValue[0..15] 存储token key
				if(keyLen == 0)
				{
					ReferencedDataNotFound();
					return;         //如果当前sdID没有put token key应该如果操作
				}
				
				ecpMem_Set(modulus,0,8);
				
				TripleDES(&RecData[0x00], keyValue, modulus,  CBC_Init_Encrypt, 0x80, 0x10);
				
				if(ecpMem_Compare(&modulus[0x80-8],&RecData[0x80],0x08) != EQUAL)
				{
					SecuritySatisfied();
					
					return;
				}
				
				GlobalSetStatus(0x9000);				
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
#if UICC__AUTH__CDMA__CAVE
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
		case 0x80:
			RunMD5Algorithm();
			break;
#endif
#if UICC__APP__WHITECARD			
//		case 0xB5:
//			SDK_GetInformation();
//			break;
//		case 0xA5:
//			SDK_WhiteCardWriteCardHandler();
//			return;
//		case 0xA9:
//			SDK_GetWriteCardCommandResult();
//			break;
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
	if(INS == 0xD6)
	{
#if (SE_ONLY && UICC__OPERATOR__CHINAMOBILE)
		if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE) //优选漫游and 134号段处理
		{
#if   !NOSMARTLUMING
			return SmartLumingWrite();
#else
			return  Funtion134Write();
#endif
		}
#endif
		
#if UICC__OPERATOR__CHINAUNICOM
	    if(ValueBit(G_SystemStatus,B_RfmPerform) == FALSE)//更新6F7B文件时，不允许和6f61文件的内容相同
		 {
			 return Update6F7BFile();			 
		 }
#endif

	}
	else
	{
		return TRUE;
	}

	return TRUE;
}


#if UICC__FUNCTION_CMCCAPI
void LocationStatusProcessCMCCAPI(void)
{
	BYTE  idata pos;
	
	pos = FindTag(&RecData[2], 0x9B,RecData[1]);
	
	if(pos != FALSE)
	{
		//PendingMessageAlert(LocationStatus_Alert,RecData[2+pos+1]);
		if(RecData[2+pos+1] == 0x00)
		{
			RAM_Online_Flag = 0x01;
		}
		else
		{
			RAM_Online_Flag = 0x00;
		}
		
	}		
}





#endif

/*
*@function   :ProcessEnvelopData

*@description:处理ENVELOP 数据,若该envelop数据为toolkit app触发数据，则打标记，在processAlert中处理
*
*@see         :
*
*@param		  :void
*
*@return	  :
*
*@author      :ZZJ
*
*@history	  :V1.0 :create by ZZJ@20131128
*/
BYTE ProcessEnvelopData(void)
{
#if UICC__FUNCTION__STKAPI
	BYTE idata eventBit;
	BYTE  idata pos,offset,length;
	BYTE idata result;
    #endif
	switch(RecData[0])	
	{
		case 0xD1:
			PP_Download_Hanlder(); 
			return TRUE;
					 
		case 0xD3:
			MenuSelection();
			return TRUE;
		
#if UICC__FUNCTION__STKAPI
		case 0xD2:
			CB_Download_Hanlder();
			return TRUE;

		case 0xD4:          //Call control tag
			eventBit = EVENT_CALL_CONTROL_BY_SIM_bit ;		
			break;

		case 0xD5:  //MO Short message control tag (if (MOSMcontrol issupported)
			eventBit =  EVENT_MO_SHORT_MESSAGE_CONTROL_BY_SIM_bit;
			break;

		case 0xD7:
			eventBit = EVENT_TIMER_EXPIRATION_bit;
//#if M2M_EUICC
//			result = FindTag(&RecData[2],0x24,RecData[1]);
		
//			if(RecData[result+3] == (G_NotificationTimerId&0x0F))
//			{                
//                ProcessResendNotification();
//			}			
//#endif			
			if(TimerModuleProcessTimerExpiration())
			{
				return TRUE;
			}
			else
			{
				break;
			}

		case 0xD6:  //	Event download tag
		{
			if (BIPModuleIsProcess() == TRUE)	//http and cattp
			{
				return TRUE;
			}
			
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
		
			pos = pos + 2;	 

			switch(RecData[pos])
			{
    #if UICC_USATTEST
				case 0x00:
					result = MTCallEvent(pos-2);
					return FALSE;
				case 0x01:
					result = CallConnected(pos-2);
					return FALSE;
				case 0x02:
					result = CallDisConnect(pos-2);
					return FALSE;
				case 0x03:
					result = LocationEvent(pos-2);
					return FALSE;
				case 0x04:
					result = UserActiveEvent();
					return FALSE;
				case 0x05:
					result = IdleSrcEvent();
					return FALSE;
				case 0x08:
					result = BrowserTerminateEvent();
					return FALSE;
				case 0x09:
					USATDataAvailable(pos + 5);
					return FALSE;
				case 0x0A:
					ChannelStatusEvent();
					return FALSE;
	#else
				case MT_call:
					eventBit = EVENT_EVENT_DOWNLOAD_MT_CALL_bit;
					break;
				case Call_connected:
					eventBit = EVENT_EVENT_DOWNLOAD_CALL_CONNECTED_bit;
					break;
				case Call_disconnected:
					eventBit = EVENT_EVENT_DOWNLOAD_CALL_DISCONNECTED_bit;
					break;
				case Location_status:
					eventBit = EVENT_EVENT_DOWNLOAD_LOCATION_STATUS_bit;
#if M2M_EUICC
					LocationStatusProcess();
#endif	
				
#if UICC__FUNCTION_CMCCAPI					
					LocationStatusProcessCMCCAPI();
#endif					
					break;
				case User_activity:
					eventBit = EVENT_EVENT_DOWNLOAD_USER_ACTIVITY_bit;
					break;
				case Idle_screen_available:
					eventBit = EVENT_EVENT_DOWNLOAD_IDLE_SCREEN_AVAILABLE_bit;
					break;
				case Card_reader_status:
					eventBit= EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS_bit;
					break;
				case Language_selection:
					eventBit = EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION_bit;
					break; 
				case Browser_Termination:
					eventBit = EVENT_EVENT_DOWNLOAD_BROWSER_TERMINATION_bit;
					break;
				case Data_available:
					eventBit = EVENT_EVENT_DOWNLOAD_DATA_AVAILABLE_bit;
					break;
				case Channel_status:
					eventBit = EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit;
					break;
				
				case Network_Rejection:
					eventBit = RecData[pos]+13;
					break;

				case Access_Technology_Change_single:											   
					 eventBit = EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_bit;
					 break;
				case Display_parameters_changed:
					 eventBit = EVENT_EVENT_DOWNLOAD_DISPLAY_PARAMETER_CHANGED_bit;
					 break;
				case Network_Search_Mode_Change:
					 eventBit = EVENT_EVENT_DOWNLOAD_NETWORK_SEARCH_MODE_CHANGE_bit;
					 break;
				case Browsing_status:				 						 						 						 						 	
					 eventBit = EVENT_EVENT_DOWNLOAD_BROWSING_STATUS_bit;   
					 break;	
				case Frames_Information_Change:
					 eventBit = EVENT_EVENT_DOWNLOAD_FRAMES_INFORMATION_CHANGED_bit;
					 break;
				case HCI_connectivity_event:
					 eventBit = EVENT_EVENT_DOWNLOAD_HCI_CONNECTIVITY_bit;
					 break;	
				case Local_connection:
					 eventBit = EVENT_EVENT_DOWNLOAD_LOCAL_CONNECTION_bit;
					 break;
				case Contactless_state_request:
#if BOOT__IO__SWP
					result = FindTag(&RecData[2],TAG_CONTACTLESS_STATE_REQUEST,RecData[1]);//modify by linshanwen@20210809: 修改HCI API问题, EPX_SUITE/CHIP/HED_CIU98M25/HED_CIU98M25_5G_SWP

					if (result != FALSE)
					{
						if (RecData[2+result+1] == 0x00)
						{
							//'00' = Enable contactless;
							JcvmConfig_SetRfCommunicationSwitch(RF_COMMUNICATION_SWITCH_ON);
						}
						else if (RecData[2+result+1] == 0x01)
						{
							//'01' = Disable contactless
							JcvmConfig_SetRfCommunicationSwitch(RF_COMMUNICATION_SWITCH_OFF);
						}
						
						//'02' = Get contactless state;
					}
//					if((result != FALSE)&&(RecData[2+result+1]<2))
//					{											
//						JcvmConfig_SetRfCommunicationSwitch(RecData[2+result+1]);						
//					}
#endif
					 eventBit = EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit;
					 break;
					 
				case Access_Technology_Change_multiple:
					 eventBit = EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_MULTIPLE_bit;
					 break;	
				default:
					return FALSE;
#endif
            }
			
		} 
		break;
#if M2M_ESx
		case 0xE4: //add by chentao@20220928 The Local Enable and Local Disable of a Profile with the Emergency Profile Attribute set is realised by using the ENVELOPE command with a dedicated Tag E4
			result = M2MLocalEnvelopCommand();
			RecData[0] = 0x80;
			RecData[1] = 01;
			RecData[2] = result;
			SetOutgoingAndSend(0x05,3);
			return TRUE;
#endif
		default:
			eventBit = EVENT_UNRECOGNIZED_ENVELOPE_bit;
			break;       
        #else
     	default:
		     return TRUE;
        #endif
	}	
	
#if UICC__FUNCTION__STKAPI
	offset = (RecData[1] == 0x81?3:2);

	length = P3 - offset;

    result = WriteTFBuf(G_EnvelopHanderAddr, 0, &RecData[offset], length);

	if(result == FALSE)
	{
	   return FALSE;
	}

	ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = length;

	G_EnvelopeTag = RecData[0];

	JavaToolkitProcess(eventBit,0);	          //不再打标记，直接处理,by XWH 20131010
    #endif
	return TRUE;
} 
/**
 * @brief 		ProcessPendingMessageAlert的接口函数，用于处理扩展的标记
 *
 * @param 		pendingTag :已经打上的标记
 * @param 		refrence :
 *
 * @author 		xuqs
 *
 * @history 	add comment by xuqs, @081220 22:37
 *              modify by xuxiaojun@20220309   merged from EPX产业化\STM_ST33G1M2\Branch\RSP\code, 支持DNS功能
 */
void IF_ProcessPendingMessageAlert(BYTE pendingTag,BYTE refrence)
{
#if  UICC_USATTEST
BYTE idata pos;
#endif
	
	switch (pendingTag)
	{
#if UICC__FUNCTION__STKAPI
		case TYPE_SMS_ENV_MULTI:
		case TYPE_SMS_ENV_SINGLE:
			ToolkitRFMExcute(pendingTag,refrence);
			break;
#endif

#if GP__AMENDMENT__B__FOR_MASK
		case D_SCP81TriggerTag:
			HTTPSessionStart();
			break;
#endif

		case BIPModule_DataAvaiable_Alert:    //0x40 ′|àíBIP?￡?é?Dμ?data avaiableevent ?￡?÷òaê??óê?êy?Yμè′|àí
			BIPModuleProcessDataAvaiable(refrence);
			break;
		
		case BIPModule_Channelstatus_Alert:
			IF_BipChannelStatus();
			break;

		case TimerModule_TimerExpiration_Alert:	
#if M2M_EUICC			
			IF_ProcessResendNotification(refrence);
#endif
#if GP__AMENDMENT__B__FOR_MASK
			IF_AnnexB_RetryHttpSessionByTimerExpiration(refrence);
#endif
#if UICC__FUNCTION__CATTP
			IF_CATTP_RetryCattpsessionByTimerExpiration(refrence);
#endif
#if GP__AMENDMENT__B__DNS_Resolution
			DNS_RetryDNSSessionByTimerExpiration(refrence);
			RAS_RetryHttpSessionByTimerExpiration(refrence);
			DNSByME_RetryFetchingDNSIPListSessionByTimerExpiration(refrence);
#endif
			break;

#if UICC__FUNCTION__CATTP		
		case D_CATTPProcessSDU:
			CATTPProcessSDUData();
			CATTPClearFile(EF_PDUFILE);
			break;
		case CATTPPOR:
			IF_SendPDU();
			CATTPStartTimer();
			break;
					
#endif
#if UICC_SUPPORT_RFMHANDLER_D0		
		    case TYPE_RFMProactiveCommandTag:
				RFMHandlerD0(refrence);
			    break;
	#endif	
#if UICC__APP__LIVECARD
			case D_GetCardInfoTag:
			{
				LiveGetCardInfoExcute();
				break;
			}
  #endif

#if  UICC_USATTEST
		case D_USATTEST_DataAvailable:
			cephReceiveDataSingle2(0x01, GSMDEFAULT, NULL, (BYTE  *)&data7_21[0]);
	 		cephReceiveDataSingle2(0x01, GSMDEFAULT, NULL, (BYTE  *)&data7_21[0]);
	 		cephReceiveDataSingle2(0x01, GSMDEFAULT, NULL, (BYTE  *)&data7_21[0]);
	 		cephReceiveDataSingle2(0x01, GSMDEFAULT, NULL, (BYTE  *)&data7_21[0]);
	 		cephReceiveDataSingle2(0x01, GSMDEFAULT, NULL, (BYTE  *)&data7_21[0]);
	 		cephCloseChannelSingle(0x00, 0x01);
	 		cephDisplayTextSingle(0x80,UCS2,(BYTE *)&data7_21[2]);
			break;
		case D_USATTEST_MTCallEvent_1:
			cephDisplayTextSingle(0x80,UCS2,(BYTE  *)&tempBuf[0]);
			break;
		case D_USATTEST_MTCallEvent_2:
			cephDisplayTextSingle(0x80,UCS2,(BYTE  *)mthj);
			break;
		case D_USATTEST_MTCallEvent_3:
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_CallConnected_1:
			cephDisplayTextSingle(0x81, UCS2, (BYTE  *)&eventdisplay[0]);
			cephSetupEventListSingle(NULL);
			break;
		case D_USATTEST_CallConnected_2:
			cephDisplayTextSingle(0x81, UCS2, (BYTE  *)&eventdisplay[50]);
			cephSetupEventListSingle(NULL);
			break;
		case D_USATTEST_CallConnected_3:
			cephDisplayTextSingle(0x81, UCS2, (BYTE  *)&eventdisplay[75]);
			cephSetupEventListSingle(NULL);
			break;
		case D_USATTEST_CallConnected_4:
			cephSetupEventListSingle((BYTE  *)&EventlistSrting[5]);
			G_USATEventContral = 0x07;
			break;
		case D_USATTEST_CallConnected_5:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&mthjylj[0]);
			break;
		case D_USATTEST_CallConnected_6:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&mohjylj[0]);
			break;
		case D_USATTEST_CallConnected_7:
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_CallDisConnect_1:
			cephDisplayTextSingle(0x81, UCS2, (BYTE  *)&eventdisplay[25]);
			break;
		case D_USATTEST_CallDisConnect_2:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&hjcx3[0]);
			break;
		case D_USATTEST_CallDisConnect_3:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&hjcx2[0]);
			break;
		case D_USATTEST_CallDisConnect_4:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&hjcx1[0]);
			break;
		case D_USATTEST_CallDisConnect_5:
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_USATLocationEvent:
			cephSetupEventListSingle((BYTE  *)locationeventlist);
			break;

		case D_USATTEST_LocationEvent:
			cephDisplayTextSingle(0x80, UCS2, &tempBuf[0]);
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_UserActiveEvent:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&useractive[0]);
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_IdleSrcEvent:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&idlesrc[0]);
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_BrowserTerminateEvent:
			cephDisplayTextSingle(0x80, UCS2, (BYTE  *)&browser[0]);
			cephSetupEventListSingle(NULL);
			break;

		case D_USATTEST_ChannelStatusEvent_1:
			cephGetChannelStatusSingle(0xFF);

			pos = FindTag(&RecData[0], 0xB8,P3);

			if (pos != FALSE)
			{
				if (RecData[pos+2] == 0x05)
				{
					cephDisplayTextSingle(0x80, UCS2, (BYTE *)channeldroplink);
				}
			}

			break;

		case D_USATTEST_ChannelStatusEvent_3:
			cephDisplayTextSingle(0x80, UCS2, (BYTE *)channelstatusdis);
			cephCloseChannelSingle(0x00,1);
			cephSetupEventListSingle(NULL);
			break;
        #endif

#if UICC__AUTH__5G__SUPI
		case D_RefreshUpdateRoutingID:
			 cephRefreshSingle(0x01, (BYTE  *)fileList);
			 break;
#endif
#if M2M_EUICC
		case LocationStatus_Alert:
			HandleDefaultNotification(refrence);
			break;

		case D_RefreshTag:				
			sendRefreshComm();			
			break;
#endif		
#if COS_UPDATE_ONLINE_MUTUAL
		case  D_UPCOS_ACTIVEREMIND:
		case  D_UPCOS_VERSIONAFFIRM:
			IF_UPCOS_ExpandHandle();
			break;
#endif
		default:
			break;
	}
	
	return;
}
/**
 * @brief 		StatusCommand指令接口函数	
 * 
 * @Returns   	false:继续执行后续指令/TRUE:不再执行后续命令，直接返回当前设置的状态字
 * 
 * @author 		XWH
 * 
 * @history 	created by XWH
 */
 
BYTE IF_StatusCommand(void)
{
#if UICC__APP__WHITECARD
	UWORD idata fileAddr;
	BYTE idata ch[2];
#endif
	//BYTE idata ch[2];
	//UWORD xdata fileAddr;
#if M2M_EUICC
	BYTE  idata result;
#endif
	
#if UICC__FUNCTION__STKAPI
	 JavaToolkitProcess(EVENT_STATUS_COMMAND_bit,0);

	if(SetSTKBusy() == TRUE)
	{
		return TRUE;
	}
	 //有POR数据(diliver)要发出,直接退出，不处理下面的WIB数据,by xwh 20150112
	 //if(G_CurrentRFMResponse != FALSE)
	 //{
	 //	return FALSE;
	 //}
#endif
	
#if COS_UPDATE_ONLINE_MUTUAL
	//IF_UPCOSCreaBipLinkByStatus();	//打上标记需要开启http会话，用于报送
	if (UPCOSConfig_GetUPCOSNextAction() != COSUPNextAction_Null)	//upcos 存在报送通知业务需求
	{
		if ((G_StatusNumForUPCOSReport < 0x02) && (G_AuthenticationFlag == TRUE))	//表示需要的报送连接未开启过 并且 连网鉴权成功
		{
			G_StatusNumForUPCOSReport ++;

			//根据需求，2次status后开启bip通道建立连接(中国移动SIM卡软件升级原型验证—COS空中升级方案说明_2021112201.pdf 第6章)
			if(G_StatusNumForUPCOSReport == 0x02)
			{
				IF_UPCOS_PendingHTTPAlert();
			}
		}		
	}
		
	
#endif

#if M2M_EUICC
	if(((G_STATUSNUM_FOR_REFRESH&0xF0) != 0x00)&&((G_STATUSNUM_FOR_REFRESH&0x0F) != 0x00))
	{
		InitProactiveCommand(CMD_REFRESH,(G_STATUSNUM_FOR_REFRESH>>4)&0x0F,DEV_ME);
		result = SendProactiveCommand();

		if((result&0x20)!=0x20) //ts 102223  8.12.0 : Results '2X' indicate to the UICC that it may be worth re-trying the command at a later opportunity:
		{
			G_STATUSNUM_FOR_REFRESH = 0x00;
		}
		else
		{
			G_STATUSNUM_FOR_REFRESH --;
		}
		return FALSE;
	}

	
	if(G_STATUSNUM_FOR_NOTIFICATION == 0xFF) 		
	{
		return FALSE;
	}
	else
	{
		if ((G_NetworkStatus == LocationStatus_Limited_Service) || (G_NetworkStatus == LocationStatus_NO_Service))					//service 是 limited时 使用相同的全局变量 G_STATUSNUM_FOR_NOTIFICATION 进行计数 10 个status
		{
			G_STATUSNUM_FOR_NOTIFICATION ++;

			if(G_STATUSNUM_FOR_NOTIFICATION == 0x0A)
			{
				G_STATUSNUM_FOR_NOTIFICATION = 0xFF;

				G_NetworkStatus = LocationStatus_NULL;

				HandleDefaultNotification(LocationStatus_Limited_AND_NOService_10_Status);
			}
			return FALSE;
		}

		if(IF_GetNotifitionType() != 0x00)
		{
			G_STATUSNUM_FOR_NOTIFICATION ++;

			if(G_STATUSNUM_FOR_NOTIFICATION == 0x0A)
			{
				G_STATUSNUM_FOR_NOTIFICATION = 0xFF;
				
				HandleDefaultNotification(LocationStatus_10_Status);
				
				return FALSE;
			}
		}
	}
#endif	
#if UICC__APP__WHITECARD
	if( (G_StatusSendSMS == 0x00) &&  (G_StatusFirstTime == 0x00) && (G_auth == 1) )
	{
		fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConfig); // modify by xuweihua@20220107
		ReadTFBuf(fileAddr, 2, &ch[0], 1);
		
		if((ch[0]&0x80) == 0x00)//only APP write card
		{
			return FALSE;
		}
		
		fileAddr = SelectTFFile(0x7F20, 0x6F7E);
		if(fileAddr == FALSE)
		{
			return FALSE;
		}
		ReadTFBuf(fileAddr, 0x0A, &tempBuf[0], 1);
		fileAddr = SelectTFFile(0x7FF0, 0x6FE3);
		if(fileAddr == FALSE)
		{
			return FALSE;
		}
		ReadTFBuf(fileAddr, 0x11, &tempBuf[1], 1);
		if((tempBuf[0] != 0x00) && (tempBuf[1] != 0x00))
		{
			return FALSE;
		}
		G_StatusSendSMS = 0;
		G_StatusSendSMSCounter ++;
		if (GetWriteCardFlag() == FALSE)//未写卡
		{
			G_StatusFirstTime = 0x01;
			SendSMS_WriteCardApply(0);
		}
		else
		{
			fileAddr = SelectTFFile(DF_STK, EF_WhiteCardConstructor);
			ReadTFBuf(fileAddr, 0, &ch[0], 1);
			if(fileAddr == FALSE)
			{
				return FALSE;
			}
			if (ch[0] == 0x01)//已注册，未上发新号激活短信
			{
				G_StatusFirstTime = 0x01;
				SendSMS_NewNumberActivate();
			}
		}
		return FALSE;
	}
#endif
	return FALSE;
}
/*
*@descriptim:  //其他Items Next Action Indicator/Icon identifier/Item Icon identifier list/Text Attribute/Item Text Attribute List长度
*
*@function     :CheckSetUpConfigInfoLength
*
*@param        :void
*
*@return     :Length
*
*@history    :
*/
BYTE CheckSetUpConfigInfoLength(void)
{
	BYTE idata CheckLength;
	
	CheckLength = 0x00;
	
	if((RecData[OFFSET_NEXTACTION]&0x7F) == TAG_ITEMS_NEXT_ACTION_INDICATOR)					//拼装预先缓存好的数据
	{
		
		CheckLength += RecData[OFFSET_NEXTACTION+1] + 2;
	}
	
	if((tempBuf[OFFSET_AlphaIcon]&0x7F) == TAG_ICON_IDENTIFIER)
	{
		
		CheckLength += tempBuf[OFFSET_AlphaIcon+1] + 2;
	}


	if(((tempBuf[OFFSET_ICONLIST]&0x7F) == TAG_ITEM_ICON_IDENTIFIER_LIST)&&(G_IconNum!=0))
	{
		
		CheckLength += tempBuf[OFFSET_ICONLIST+1] + 2;
	}
	
	if((tempBuf[OFFSET_AlphaTextAttribut]&0x7F) == TAG_TEXT_ATTRIBUTE_TAG)
	{
		
		CheckLength += tempBuf[OFFSET_AlphaTextAttribut+1] + 2;
	}
	
	if((tempBuf[OFFSET_TEXTATTRIBUTELIST]&0x7F) == TAG_ITEM_TEXT_ATTRIBUTE_LIST)
	{
		
		CheckLength += tempBuf[OFFSET_TEXTATTRIBUTELIST+1] + 2;
	}

	return CheckLength;

}
/**
 * @brief 		ProcessPendingMessageAlert中处理stepMenu指令	
 * 
 * @Returns   	VOID
 * 
 * @author 		ZDM 
 * 
 * @history 	created by ZDM
 */
void IF_SetUpMenu(void)
{
	BYTE  idata result;

	result = SetUpMenu();

	if(result == TRUE)
	{
		SendProactiveCommand();
	}
}
/*
*@description:对 java菜单进行拼装
*
*@function    :AddJavaMenuItem
*
*@param        :void
*
*@return       :FALSE
*
*@author       :
*/
BYTE AddJavaMenuItem(void)
{
	BYTE idata AddCheckLength,flag;
	WORD idata i;
	WORD idata TotalLen;
	UWORD xdata tookitMenuEntryAddr;
	UWORD xdata toolkitMenuEntryIndexAddr;

	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);
	
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	ReadTFBuf(toolkitMenuEntryIndexAddr,0,&RecData[OFFSET_ITEMINDEX],1);          //获取item_index到RecData 150-249
	
	ReadTFBuf(toolkitMenuEntryIndexAddr,1,&RecData[OFFSET_ITEMINDEX+1],RecData[OFFSET_ITEMINDEX]);
	
	//RecLen = GetLFRecNumLgth(tookitMenuEntryAddr,FileLgth);

	for(i = G_JavaMenuIndex; i <= RecData[OFFSET_ITEMINDEX];i++, G_JavaMenuIndex++)
	{		
		tempBuf[0] = TLV_ITEM;
		tempBuf[1] = 0x01;
	
		flag = ReadLFBuf(tookitMenuEntryAddr,RecData[OFFSET_ITEMINDEX+i],0x00,&RecData[0],GetLFRecNumLgth(tookitMenuEntryAddr,FileLgth));

		if(flag == FALSE)  //空记录，跳过
	      {
			continue;
		}

		tempBuf[2] = RecData[ItemIdentifier_OFFSET];  //item identifier	;  

		if(RecData[MenuEntryStatus_OFFSET] != 0xC1)//unlock selectable enable时才显示菜单
		{
			continue;
		}
		
		if(RecData[HelpSupported_OFFSET] == TRUE)                                //若用菜单需要显示Help信息，Command Qualifier最高位置一 see 102223 8.6
		{
			ResData[7] |= 0x80;
		}

		AddValue((BYTE xdata *)&tempBuf[1], (BYTE xdata *)&RecData[MenuEntryTextLen_OFFSET]);

		RecData[OFFSET_NEXTACTION+2+RecData[OFFSET_NEXTACTION+1]] = RecData[NextAction_OFFSET];    //拼装NextAction信息
		RecData[OFFSET_NEXTACTION+1] ++;

		if(RecData[NextAction_OFFSET] != 0)                                                        //当任一Item的Next Action有效，则该字段有效
		{
			RecData[OFFSET_NEXTACTION] = TAG_ITEMS_NEXT_ACTION_INDICATOR;
		}

		if(RecData[IconQulifier_OFFSET] != ICONQULIFIER_SELFEXPLANATORY)                           //The Icon list qualifier transmitted to the Terminal will be 'icon is not self explanatory' if one of the applet registered prefers this qualifier.  See 102241_Annex_A_HTML 关于initMenuEntry相关描述
		{
		   	tempBuf[OFFSET_ICONLIST+2] = ICONQULIFIER_NOTSELFEXPLANATORY;
		}

		if((RecData[IconIdentifier_OFFSET] == 0)&&(RecData[IconIdentifier_OFFSET+1] == 0)) 
		{
			tempBuf[OFFSET_ICONLIST] = 0x00;                                                   //either none of the icons in this list are displayed, or for each item its related icon is displayed.  See 102223 8.32
		}
		else
		{
			tempBuf[OFFSET_ICONLIST+2+(tempBuf[OFFSET_ICONLIST+1])] = RecData[IconIdentifier_OFFSET+1]; //UICC只有一个Img文件，IconIdentifier不会大于0xFF，因此只取低字节的值. 参考Each icon identifier addresses a record in EFIMG as defined in TS 131 102 [6]. See 102223 8.32
			tempBuf[OFFSET_ICONLIST+1] ++;
			G_IconNum ++;
		}
		
		if((RecData[TextAttribute_OFFSET] != 0x00)||(RecData[TextAttribute_OFFSET+1] != 0x00)||(RecData[TextAttribute_OFFSET+2] != 0x03)||(RecData[TextAttribute_OFFSET+3] != 0x90))    //若TEXT ATTRIBUTE不为默认值，则该字段有效
		{
			tempBuf[OFFSET_TEXTATTRIBUTELIST] = TAG_ITEM_TEXT_ATTRIBUTE_LIST;
			ecpMem_Copy(&tempBuf[OFFSET_TEXTATTRIBUTELIST+2+tempBuf[OFFSET_TEXTATTRIBUTELIST+1]], &RecData[TextAttribute_OFFSET], 4);    //拼装TEXT ATTRIBUTE LIST
		}
		else
		{						
			tempBuf[OFFSET_TEXTATTRIBUTELIST+tempBuf[OFFSET_TEXTATTRIBUTELIST+1]+2] = 0x00; //拼装OFFSET_TEXTATTRIBUTELIST信息				
			tempBuf[OFFSET_TEXTATTRIBUTELIST+tempBuf[OFFSET_TEXTATTRIBUTELIST+1]+3] = 0x00;				
			tempBuf[OFFSET_TEXTATTRIBUTELIST+tempBuf[OFFSET_TEXTATTRIBUTELIST+1]+4] = 0x03;				
			tempBuf[OFFSET_TEXTATTRIBUTELIST+tempBuf[OFFSET_TEXTATTRIBUTELIST+1]+5] = 0x90;
		}
		
		tempBuf[OFFSET_TEXTATTRIBUTELIST+1] += 4;
		
		AddCheckLength = CheckSetUpConfigInfoLength();      //其他Items Next Action Indicator/Icon identifier/Item Icon identifier list/Text Attribute/Item Text Attribute List长度

				
		if ((G_SMItemLen + (tempBuf[1]+2)) +AddCheckLength<= (255-sizeof(lasPage)))
		{
			G_SMItemLen += (tempBuf[1]+2);	
		} 
		else
		{		
			return FALSE;
		}
	
		AddTLV((BYTE xdata *)&ResData[1], (BYTE xdata *)&tempBuf[0]);		
	}
	
	return TRUE;

}
/*
*@description:初始化Java菜单信息，如：初始化一些属性信息，ICON、NextAction、TEXTATTRIBUTELIST等, 将当前有效的JavaMenuId缓存起来
*
*@function   :JavaMenuStartConfig
*
*@param      :void
*
*@author     :
*
*@history    :
*/
void JavaMenuStartConfig(void)
{	
	RecData[OFFSET_NEXTACTION] = 0;     //拼装Items Next Action Indicator                    //初始化Next Action,默认indicator为0,该字段无效.当任一Item存在Next Action，则该字段有效
    
	RecData[OFFSET_NEXTACTION+1] = 0;   //length

	tempBuf[OFFSET_ICONLIST] = TAG_ITEM_ICON_IDENTIFIER_LIST&TLV_INVERT_CR;                  //初始化ITEM ICON,默认indicator为0x1F,该字段有效.当任一Item不存在ITEM ICON，则该字段无效      see 102223 8.32
    
	tempBuf[OFFSET_ICONLIST+1] = 0x01;  //length
    
	tempBuf[OFFSET_ICONLIST+2] = ICONQULIFIER_SELFEXPLANATORY;  //Icon list qualifier
	
	tempBuf[OFFSET_TEXTATTRIBUTELIST] = 0;     //拼装Items TEXT ATTRIBUTE LIST Indicator     //初始化TEXT ATTRIBUTE,默认indicator为0,该字段无效.当任一Item存在TEXT ATTRIBUTE，则该字段有效
    
	tempBuf[OFFSET_TEXTATTRIBUTELIST+1] = 0;   //length
	
	G_IconNum = 0x00;	
}

/**
 * @brief 		读取6F18中 目录ID、菜单类型、记录号，3个一组按目录ID由小到大排序存在tempBuf中
 *
 * @return		void
 *
 * @author 		ChenL
 *
 * @history 	
 */
 #if  UICC_USATTEST
/*
  Function name :	 OTASortApplicationID
  
  input parameter:   list : LV format save the application id 's 	
  					 mode:	application id 的长度
   
  output parameter:	 无
                                                                                                                                                
  description:     	 sort application id in the list in ascendant format
  					 AID由小到大排序
  
  author:			D
  
  status:					      
 */    

void SortApplicationID(BYTE * list, BYTE mode)
{
	BYTE idata i, j;
	BYTE xdata tmp[3];

	for (i=0; i<(list[0]-1); i++)
	{
		for(j=0; j<(list[0]-i-1); j++)
		{
			if(memcmp_p(&list[(j*mode) + 1], &list[(j+1) * mode + 1] , mode) > 0)
			{
				ecpMem_Copy(tmp, &list[(j*mode)+1], mode);

				ecpMem_Move(&list[(j*mode)+1],&list[(j+1)*mode+1], mode);

				ecpMem_Copy(&list[(j+1)*mode+1] , tmp, mode);
			}
		}
	}
}
/*
*@function :SortMenuId
*
*@description:对不同类型的ota菜单进行排序
*
*@param      :UWORD fileAddr
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0 :create by xuyz@20170908
*/
void SortMenuId(UWORD fileAddr)
{
	BYTE idata i, j,validNum;
	BYTE idata recLgth,recNum;
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);
	
	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	validNum = 0x00;
	
	for(i=1,j=1; i<=recNum; i++)
	{
		ReadLFBuf(fileAddr,i,0x00,&RecData[1],recLgth);

		if(((RecData[1]==0xFF)||(RecData[5]&0x80) != 0x80)||(RecData[6] == 0x00))  //目录名称长度为0x00时，不显示
		{
			continue;
		}
		
		tempBuf[OFFSET_OTAMenuSort+j] = RecData[3];  //目录ID
		
		tempBuf[OFFSET_OTAMenuSort+j+1] = RecData[1]; //菜单类型
		
		tempBuf[OFFSET_OTAMenuSort+j+2] = i;		//记录号	
		
		validNum ++ ;
		
		j+=3;
	}

	tempBuf[OFFSET_OTAMenuSort] = validNum;	//要排序的目录ID个数

	//目录ID由小到大排序
	SortApplicationID(&tempBuf[OFFSET_OTAMenuSort], 3);	 //将6F18中目录ID按照从小到大排序，那么对于可更名固化应用等，6F18中的记录号已经按照ID 大小排好序了
}
#endif
/*
 * @brief 		添加菜单项目，菜单的启用和禁用等属性请参看EF_6F18的说明
 * 
 * @param 		fileAddr: EF_6F18 
 * @param 		index: Item Identifier 
 * 
 * @Returns		TRUE:继续组装SetupMenu的Item项
 *				FALSE:组装完毕
 *
 * @author 		XuQingsheng
 * 
 * @history 	created by XuQingsheng, @071220 20:01
 */
 #if   UICC_USATTEST
BYTE AddMenuItem(UWORD fileAddr, BYTE index)
{	
	ReadLFBuf(fileAddr,index,0x00,&RecData[1],GetLFRecNumLgth(fileAddr,FileLgth));

	if(((RecData[1]==0xFF)||(RecData[5]&0x80) != 0x80)||(RecData[6] == 0x00))  //目录名称长度为0x00时，不显示
	{
		return TRUE;
	}

	RecData[0] = 0x8F;

	RecData[1] = 0x01;
	
	USATSetSpcPrjItem();
	
	SetLFConst(fileAddr,index,0x01, RecData[2],sizeof(BYTE));
	
	AddValue((BYTE xdata *)&RecData[1], (BYTE  *)&RecData[6]);

	AddTLV((BYTE xdata *)&ResData[1], (BYTE  *)&RecData[0]);
	
	return TRUE;
 }
#else
BYTE AddMenuItem(UWORD fileAddr)
{	
	BYTE idata menuIndex,AddCheckLength;
	
	BYTE idata recNum,recLgth,i;
		
	recNum = GetLFRecNumLgth(fileAddr,FileNum);
	
	recLgth = GetLFRecNumLgth(fileAddr,FileLgth);
	
	if(G_NextPageTag == TRUE)
	{
		G_MenuIndex = G_BackMenuIndex;
	}
	else
	{
		G_MenuIndex = 1;
	}
	
	for(i=G_MenuIndex;i<=recNum;i++)
	{
	
		ReadLFBuf(fileAddr, i, 0x00, &RecData[1], recLgth);
		
		if(((RecData[1] == 0xFF) || ((RecData[5]&0x80) != 0x80)||(RecData[6] == 0x00)))//目录名称长度为0x00时，不显示
		{
			continue;
		}
	
		RecData[0] = 0x8F;	
		
		//OTASetSpcPrjItem();

		RecData[1] = 0x01;
		
		AddValue((BYTE *)&RecData[1], (BYTE *)&RecData[6]);
		
		RecData[OFFSET_NEXTACTION+2+RecData[OFFSET_NEXTACTION+1]] = 0;    //拼装NextAction信息
		RecData[OFFSET_NEXTACTION+1] ++;
		
		AddCheckLength = CheckSetUpConfigInfoLength();      //其他Items Next Action Indicator/Icon identifier/Item Icon identifier list/Text Attribute/Item Text Attribute List长度
		
		if ((G_SMItemLen + (RecData[1] + 2) + AddCheckLength) <= (255 - sizeof(lasPage))) //判断整个主动式命令长度加起来不超过255 bylicaizhang20220907
		{                                             
			G_SMItemLen += (RecData[1] + 2);	
		} 
		else
		{	
			return FALSE;
		}
		
		if(G_NextPageTag == TRUE)
		{
			menuIndex = G_BackMenuIndex + G_NextMenuIndex - 1;
			
			G_NextMenuIndex ++;
		}
		else
		{
			menuIndex = G_MenuIndex;
			
			G_MenuIndex ++; 
		}

		menuIndex = (menuIndex|D_NativeMeun);
		
		RecData[2] = menuIndex;
		
		SetLFConst(fileAddr,i,0x01, menuIndex,sizeof(BYTE)); 

		AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[0]);
	}
	return TRUE;	
}
#endif

/**
 * @brief 		添加菜单项目公共函数
 * 
 * @param 		fileAddr: EF_6F18 
 * @param 		i: 
 				j:
				k:
 * 
 * @Returns		TRUE/FALSE
 *
 * @author 		XuQingsheng
 * 
 * @history 	created by XuQingsheng, @071220 20:01
 */
 #if  UICC_USATTEST
BYTE AddMenuPublic(UWORD fileAddr, WORD i, BYTE j, BYTE k)
{
	for(; i<=tempBuf[OFFSET_OTAMenuSort]; i++, j+=3)
	{
		 if(((k==0) && (tempBuf[OFFSET_OTAMenuSort+j+1]==0x04) && (tempBuf[OFFSET_OTAMenuSort+j]>=0xA0) && (tempBuf[OFFSET_OTAMenuSort+j]<=0xB7)) ||  //可更名固化应用
		 	((k==2) && (tempBuf[OFFSET_OTAMenuSort+j+1]==0x04) && (tempBuf[OFFSET_OTAMenuSort+j]>=0xB8) && (tempBuf[OFFSET_OTAMenuSort+j]<=0xFF)) ||  //>全网OTA一级应用，本地OTA一级应用 全网行业OTA一级应用  本地行业OTA一级应用
		    ((k==3) && (tempBuf[OFFSET_OTAMenuSort+j+1]==0x04) && (tempBuf[OFFSET_OTAMenuSort+j]>=0x01) && (tempBuf[OFFSET_OTAMenuSort+j]<=0x9E)) ||  //全网OTA目录 本地OTA目录 全网行业OTA目录  本地行业OTA目录
			((k==4) && (tempBuf[OFFSET_OTAMenuSort+j+1]==0x06))                                                                                       //完全固化目录
		   )
 
		 {
		 	if(AddMenuItem(fileAddr, tempBuf[OFFSET_OTAMenuSort+j+2]) == FALSE)  //表示第一页无法展示所有菜单项，需要有第二页
			{
				if(ResData[6] == CMD_SETUPMENU)  //在第二次拼装菜单式，不需要拼装下一页 
				{												
					AddTLV(&ResData[1], (BYTE *)&lasPage[0]);					
		
					G_SMLastNum = i;		

					G_BackUpK = k;
								
					STKPerform[0] = TRUE;	

					G_BackMenuIndex = G_MenuIndex;
					
					STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 	
				}
				
				G_JavaMenuIndexBak = G_JavaMenuIndex;
				
				return FALSE;	
			}
		 }
	}

	return TRUE;
}
#else
BYTE AddMenuPublic(UWORD fileAddr, WORD i, BYTE j, BYTE k)
{
	//BYTE    xdata prelastPage[10];           //把前一页 上一页数组的数据复制出来


	if(AddMenuItem(fileAddr)==FALSE)
	{
		if(ResData[6] == CMD_SETUPMENU)  //在第二次拼装菜单式，不需要拼装下一页 
		{
			
			AddTLV(&ResData[1], (BYTE *)&lasPage[0]);					

			G_SMLastNum = i;		

			G_BackUpK = k;
						
			STKPerform[0] = TRUE;	

			G_BackMenuIndex = G_MenuIndex;
			
			STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 	
		}
		
		G_JavaMenuIndexBak = G_JavaMenuIndex;
		
		return FALSE;	
	}
			
		
	return TRUE;
	
}
#endif

/*
*@descriptim:  拼装setupmenu中的配置字节
*
*@function     :SetUpConfigInfo
*
*@param        :void
*
*@return     :void
*
*@history    :
*/
void AddSetUpConfigInfo(void)
{
	if((RecData[OFFSET_NEXTACTION]&0x7F) == TAG_ITEMS_NEXT_ACTION_INDICATOR)					//拼装预先缓存好的数据
	{
		AddTLV((BYTE  *)&ResData[1], (BYTE  *)&RecData[OFFSET_NEXTACTION]);
	}
	
	if((tempBuf[OFFSET_AlphaIcon]&0x7F) == TAG_ICON_IDENTIFIER)
	{
		AddTLV((BYTE  *)&ResData[1], (BYTE  *)&tempBuf[OFFSET_AlphaIcon]);
	}

	if(((tempBuf[OFFSET_ICONLIST]&0x7F) == TAG_ITEM_ICON_IDENTIFIER_LIST)&&(G_IconNum!=0))
	{
		AddTLV((BYTE  *)&ResData[1], (BYTE  *)&tempBuf[OFFSET_ICONLIST]);
	}
	
	if((tempBuf[OFFSET_AlphaTextAttribut]&0x7F) == TAG_TEXT_ATTRIBUTE_TAG)
	{
		AddTLV((BYTE  *)&ResData[1], (BYTE  *)&tempBuf[OFFSET_AlphaTextAttribut]);
	}
	
	if((tempBuf[OFFSET_TEXTATTRIBUTELIST]&0x7F) == TAG_ITEM_TEXT_ATTRIBUTE_LIST)
	{
		AddTLV((BYTE  *)&ResData[1], (BYTE  *)&tempBuf[OFFSET_TEXTATTRIBUTELIST]);
	}	
}
/*
*@funtion     :	SetUpMenu

*@description : 建立菜单
*
*@see         : 102223
*
*@param       : void
*
*@return      : TRUE/FALSE
*
*@author      : ZZJ
*
*@history     : V1.0:create by zzj @20131125
*
*@others      : 函数未对上发数据长度超过255时进行处理 ，无翻页显示功能V1.0
*/
  
//tempBuf 0-49    缓存单条Item的TLV
//tempBuf 50-99   缓存ICONLIST TLV
//tempBuf 100-229 缓存TextAttributeList TLV
//tempBuf 230-233 缓存AlphaIcon TLV
//tempBuf 234-249 缓存AlphaTextAttribut TLV
//RecData 0-99    缓存Menuentry内容
//RecData 100-149 缓存NextAction TLV
//RecData 150-250 缓存ItemIndex
 										
BYTE SetUpMenu(void)
{
	UWORD xdata fileAddr;
	BYTE idata i,j,k,haveEntry,result;
	WORD idata len;                      //指令长度	
#if UICC__FUNCTION__STKAPI
	UWORD xdata tookitMenuEntryAddr;
	UWORD xdata toolkitMenuEntryIndexAddr;
#endif	

	UWORD xdata NTMenuAddr;
	
	ecpMem_Set(&tempBuf[0], 0x00,sizeof(tempBuf));	  //JavaMenuStartConfig中会初始化tempBuf的值
	
#if UICC__FUNCTION__STKAPI	 //SIM API 的测试中，TerminalProfile不支持SetupMenu，但是测试用例发了C2(D3)要求触发对应的应用，因此要先进行G_CurrentJavaMenuId的赋值操作:即先发送10指令返回9000，再直接发送C2(D3)
	             //如SIMAPI 中的api_1_usy_getfb 测试步骤1
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);
	
	if((tookitMenuEntryAddr== FALSE)||(toolkitMenuEntryIndexAddr == FALSE))
	{
		return FALSE;
	}
	
	JavaMenuStartConfig();                                                    //拼凑java菜单前的配置
#endif
	
	if(ValueBit(G_SystemStatus, B_meSurportStk) == FALSE)
	{
		return FALSE;
	}
	//初始化翻页菜单的变量 bylicaizhang20220907
	G_BackUpK = 0;
	
	G_NextMenuIndex = 0;
	
	G_BackMenuIndex = 0;
	
	NTMenuAddr = SelectLFFile(DF_STK,EF_Menu);   //电信没有本地菜单
  
	G_NextPageTag = FALSE;
	
	G_MenuIndex = 1;
	G_JavaMenuIndex = 0x01;  //setupMenu时从EntryIndex文件的起始开始,第一字节为L ，所有有效数据偏移从1开始
	
	G_JavaMenuIndexBak = 0x01;
	
	G_SMItemLen = 0;	

	InitProactiveCommand(CMD_SETUPMENU,Q_SETUPMENU,DEV_ME);
	
	G_SMItemLen += 0x0C;       //InitProactiveCommand长度
	
	fileAddr = SelectTFFile(DF_TELECOM,EF_STKName);   //注意：根据规范，SIM CARD下的6F54 位于7F20下；UICC card的6F54位于7F10下，对于UICC 卡片而言，这两个是映射关系
	
	if(fileAddr == FALSE)
	{
		fileAddr = SelectTFFile(DF_GSM,EF_STKName);
	}


	if(fileAddr != FALSE)
	{
		len = GetTFSize(fileAddr);	 //注意：6F54的文件大小若超过一个字节所能表示的长度，此处需要修改

		ReadTFBuf(fileAddr,0,&RecData[0],len);

		if((RecData[0]&TLV_INVERT_CR) == (TAG_ALPHA_IDENTIFIER&TLV_INVERT_CR))
		{
			RecData[0]|=0x80;
			AddTLV(&ResData[1], &RecData[0]);
		}
		else
		{
			RecData[0] = TAG_ALPHA_IDENTIFIER;
			RecData[1] = 0;
			AddTLV(&ResData[1], &RecData[0]);
		}

		i = RecData[1]+2;
		
		if((RecData[i]&TLV_INVERT_CR) == (TAG_ICON_IDENTIFIER&TLV_INVERT_CR))
		{
			RecData[i]&=TLV_INVERT_CR;
			ecpMem_Copy(&tempBuf[OFFSET_AlphaIcon], &RecData[i], 4);
			
			i+=4;
		}
		
		if((RecData[i]&TLV_INVERT_CR) == (TAG_TEXT_ATTRIBUTE_TAG&TLV_INVERT_CR))
		{
			RecData[i]&=TLV_INVERT_CR;
			ecpMem_Copy(&tempBuf[OFFSET_AlphaTextAttribut], &RecData[i], 6);
		}
	} 
	else
	{
		RecData[0] = TAG_ALPHA_IDENTIFIER;
		RecData[1] = 0;
		AddTLV(&ResData[1], &RecData[0]);
	}
	
	G_SMItemLen = ResData[2] + 2;

	#if UICC_USATTEST
	SortMenuId(NTMenuAddr);   
	#endif
			
	for(k=0;k<5;k++) //k=0表示可更名固化应用；1表示java应用；2表示全网OTA一级应用，本地OTA一级应用 全网行业OTA一级应用  本地行业OTA一级应用；3表示全网OTA目录 本地OTA目录 全网行业OTA目录  本地行业OTA目录；4表示完全固化目录
	{
      #if UICC_USATTEST
		if(k == 1)   //对java应用进行排序 
		#else
		if(k == 0)
		#endif
		{
			result = AddJavaMenuItem();
			
			if(result == FALSE)   //需要展示下一页
			{
				 AddTLV(&ResData[1], (BYTE*)&lasPage[0]);					

				 G_SMLastNum = G_JavaMenuIndex;		

				 G_BackUpK = k;
							
				 STKPerform[0] = TRUE; //在置标记STKPerform[0] = true时，一定要登记STKPerform[1]的值。

				 STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 
				 
				 G_JavaMenuIndexBak = G_JavaMenuIndex;
				 
				 AddSetUpConfigInfo();				 

				 return TRUE;
			}
		}
#if !UICC_USATTEST
		else if(k == 1)
		{
			i=1,j=1;
			
			if(NTMenuAddr == FALSE)   //6F18不存在 本地菜单不拼装
			{
				continue;
			}

			result = AddMenuPublic(NTMenuAddr, i, j, k);

			if(result == FALSE) //需要展示下一页
			{
				AddSetUpConfigInfo();
				
				//ecpMem_Copy(&G_CurrentJavaMenuIdBak[0], &G_CurrentJavaMenuId[0], sizeof(G_CurrentJavaMenuId)); //进入下一页时，G_CurrentJavaMenuId数据通过G_CurrentJavaMenuIdBak获取
				
				return TRUE;
			}
		}
#endif
#if UICC_USATTEST
		else
		{
			i=1,j=1;
			
			if(NTMenuAddr == FALSE)
			{
				continue;
			}

			result = AddMenuPublic(NTMenuAddr, i, j, k);

			if(result == FALSE) //需要展示下一页
			{
				AddSetUpConfigInfo();
				
				//ecpMem_Copy(&G_CurrentJavaMenuIdBak[0], &G_CurrentJavaMenuId[0], sizeof(G_CurrentJavaMenuId)); //进入下一页时，G_CurrentJavaMenuId数据通过G_CurrentJavaMenuIdBak获取
				
				return TRUE;
			}
		}
    #endif		
	}
	
				
	haveEntry=FindTag(&ResData[3],0x8F,ResData[2]);

	if(haveEntry == FALSE)	//无菜单项时，8F字段为8F00(8F为必选字段，必须存在),不拼装icon字段  See 102223 6.6.7 
	{
#if UICC__FUNCTION__STKAPI				
		if(ValueBit(G_ToolkitSystemProactiveFlag,Bit_PowerOnSetupMenu) == TRUE)
		{
			return FALSE;    //在会话开始时，如当前没有有效菜单，则不发送setupMenu 主动式命令  ,UICCAPI TEST_CRE_TIN_NBME 要求不发无item的setupmenu
		}
#endif
		
		RecData[0] = TLV_ITEM;
		RecData[1] = 0x00;
		AddTLV((BYTE xdata *)&ResData[1], (BYTE xdata *)&RecData[0]);			

	  tempBuf[OFFSET_ICONLIST] = 0x00;
	}

	AddSetUpConfigInfo();
	
	STKPerform[0] = TRUE; //在置标记STKPerform[0] = true时，一定要登记STKPerform[1]的值。

	STKPerform[1] = G_ChannelNum+1;  //登记当前STK的通道信息，支持java卡多通道处理 

	return TRUE;
}
/**
 * @brief 		建立菜单第二页。菜单排序的规则请查看操作手册对EF_6F18的描述
 * 
 * @author 		XuQingsheng 
 * 
 * @history 	created by XuQingsheng, @071220 20:01
 * 修改：增加下一页拼加菜单时，需要拼本地菜单bylicaizhang20220907
 */
BYTE SetupMenuNextPage(void)
{
	BYTE idata result,k,i,j;
	UWORD idata NTMenuAddr,fileAddr;
	

	G_SMItemLen = 0;
	
	G_JavaMenuIndex = G_JavaMenuIndexBak;
	G_NextPageTag = TRUE ;
	G_MenuIndex = G_BackMenuIndex;

	G_NextMenuIndex = 1;
	InitProactiveCommand(CMD_SELECTITEM,0x00,DEV_ME);
	
	G_SMItemLen += 0x0C;       //InitProactiveCommand长度

	fileAddr = SelectLFFile(DF_STK,EF_Menu);

	AddTLV(&ResData[1], (BYTE*) &prePage[0]);  //拼装上一页 Item
	
	G_SMItemLen += sizeof(prePage);
 #if UICC_USATTEST
	SortMenuId(fileAddr);
 #endif
	
	for(k=G_BackUpK;k<5;k++)
	{
		#if UICC_USATTEST
		if(k==1)
		#else
		if(k == 0)   //对java应用进行排序 
		#endif
		{
			result = AddJavaMenuItem();
			
			if(result == FALSE) //表示第二页也显示不全所有剩余菜单
			{					
				break;
			}			
		}
		#if !UICC_USATTEST
		else if(k==1)
		{
			i = 1;
			j = 1;
			
			if(fileAddr == FALSE)   //6F18不存在 本地菜单不拼装
			{
				continue;
			}
			
			result = AddMenuPublic(fileAddr, i, j, k);

			if(result == FALSE)  //最大只显示两页，表示第二页也显示不全所有剩余菜单
			{
				break;
			}
		}	
		#endif
    #if UICC_USATTEST
		else
		{
			if(k == G_BackUpK)
			{
				i = G_SMLastNum;
				j = (G_SMLastNum-1)*3+1;
			}
			else
			{
				i = 1;
				j = 1;
			}
			
			if(fileAddr == FALSE)   //6F18不存在 本地菜单不拼装
			{
				continue;
			}			
			result = AddMenuPublic(fileAddr, i, j, k);

			if(result == FALSE)  //最大只显示两页，表示第二页也显示不全所有剩余菜单
			{
				break;
			}
		}
#endif
	}

	result = SendProactiveCommand();

	if (result != CMD_Success)
	{
		return result;
	}

	result = GetResponseItemIdentifier();

	return result;	
}

/*
*@description:判断当前的C2指令是不是要触发java应用   处理menuselecttion中的java应用操作
*
*@function      :ProcessJavaMenuSelect
*
*@param      :BYTE selectId
*
*@return     :true:  当前操作的是java菜单  /FALSE: 当前操作的不是java菜单
*
*@author     :
*/
BYTE ProcessJavaMenuSelect(BYTE selectId)
{
	BYTE  idata j,itemidptr,result,identifier,ptr,length,offset;
	BYTE idata currentIndexTotal,entryNum,flag;
	BYTE idata tmp[3];
	UWORD xdata tookitMenuEntryAddr;
	UWORD xdata toolkitMenuEntryIndexAddr;

	toolkitMenuEntryIndexAddr = SelectTFFile(DF_Toolkit_P,EF_MenuEntryIndex);
	
	tookitMenuEntryAddr = SelectLFFile(DF_Toolkit_P,EF_ToolkitMenuEntry);
	
	if((toolkitMenuEntryIndexAddr == FALSE)||(tookitMenuEntryAddr == FALSE))
	{
		return FALSE;
	}
	
	ReadTFBuf(toolkitMenuEntryIndexAddr,0,&currentIndexTotal,1);      //读取Entry总数
	
	for(j=1;  j<=currentIndexTotal;  j++)        //添加这个是为了  能直接出发应用  比如：UICC API/api_2_tkr_sevt步骤13复位后，直接发送D3要求可以触发java应用
	{			
		ReadTFBuf(toolkitMenuEntryIndexAddr,j,&entryNum,sizeof(BYTE));  //当前menu在6F81中的记录号
		
		flag = ReadLFBuf(tookitMenuEntryAddr,entryNum,MenuEntryStatus_OFFSET,&tmp[0],3);

		if(flag == FALSE) //空记录，跳过
		{
			continue;
		}
		
		if(tmp[MenuEntryStatus_OFFSET] != 0xC1)//unlock selectable enable时才显示菜单
		{
			continue;
		}
		
		if(selectId == tmp[2])
		{
			break;
		}
   }	

   if(j > currentIndexTotal)
   {
	   return FALSE;
   }
   
	ptr = FindTag(&RecData[0],0xD3,P3);
   
	if(ptr == FALSE) //当前在第二页，选择菜单时，是select item指令，类似801400000F    81030124008202828183010090010B
	{
		itemidptr = FindTag(&RecData[0], TLV_ITEMIDENTIFIER, P3); //90TAG长度所在偏移

		identifier = RecData[itemidptr+1];	

		itemidptr--; //兼容RecData[ptr+1+itemidptr]判断，当ptr存在时的+1
	}
	else  //当前在第一页，选择菜单时，是MENU SELETION指令，类似80C2000009    D3078202018190014F
	{
		itemidptr = FindTag(&RecData[ptr+1], TLV_ITEMIDENTIFIER, RecData[ptr]);

		identifier = RecData[ptr+1+itemidptr+1];
	}

	if((itemidptr != FALSE)&&(RecData[ptr+1+itemidptr] == 0x01))
	{				
		if(ptr == FALSE) //当前在第二页，选择菜单时，是SELET ITEM指令
		{
			offset = RecData[1]+2;  //偏移到TAG 82的起始位置
		}
		else
		{
			offset = (RecData[1] == 0x81?3:2);  //偏移到TAG 82的起始位置
		}

		length = P3 - offset;

		result = WriteTFBuf(G_EnvelopHanderAddr, 0, &RecData[offset], length);			  //初始化 EnvelopHander

		if(result == FALSE)
		{
			 return FALSE;
		}

		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = length;

		G_EnvelopeTag = 0xD3;

		if(ptr == FALSE) //当前在第二页，选择菜单时，是SELET ITEM指令
		{
// 			itemidptr = FindTag(&RecData[0], TLV_HELPREQUEST, P3);	 //HelpRequest，见11.14  12.21节 Help request tag = 0x15
// 					
// 			if(itemidptr != FALSE)
// 			{
// 				itemidptr--;
// 			}
			itemidptr = FALSE;        //select item 时候并没有   HelpRequest
		}
		else
		{
			itemidptr = FindTag(&RecData[ptr+1], TLV_HELPREQUEST, RecData[ptr]);	 //HelpRequest，见11.14  12.21节 Help request tag = 0x15
		}

		if((itemidptr != FALSE)&&(RecData[ptr+1+itemidptr] == 0x00))
		{
			JavaToolkitProcess(EVENT_MENU_SELECTION_HELP_REQUEST_bit,identifier);
		}
		else
		{
			JavaToolkitProcess(EVENT_MENU_SELECTION_bit,identifier);
		}

		return TRUE;
	}
		 
	return FALSE;
}
/*------------------------------------------------------------------------------------------------------------                                                                                                                                               
  Function name :	MenuSelection                                                                                                                   
                                                                                                                                                
  input parameter:   void                                                                    
  					                                                                                                         
  output parameter:  void                                                                                                                 
                                                                                                                                                
  description:       This function is to add the tlv structure to the buffer of have data :the buffer must have data of thv structure           
 --------------------------------------------------------------------------------------------------------------*/
BYTE MenuSelection(void)
{
	BYTE  idata num,result;	

#if (UICC__OPERATOR__CHINAMOBILE || UICC_USATTEST)
	UWORD xdata fileAddr;
	BYTE  idata menu[5],i;
#endif

#if UICC__OPERATOR__CHINAMOBILE
	BYTE  idata recNum,simInfo;
#endif


	num = GetResponseItemIdentifier();
	
	if(num == FALSE)
	{
		return FALSE;
	}
	
	result = ProcessJavaMenuSelect(num);
	
	if(G_STKBusy != FALSE)	  //当前在主动式命令中收到了C2 MenuSelecttion,则不进行菜单选择操作，只是用来触发java应用
	{			
		return FALSE;
	}
	else
	{
		if(result == TRUE)   //已经执行了java菜单
		{
			return FALSE;
		}
	
		if (num == D_NextPageItem)							//一级菜单的第二页
		{
_NextPage_:
			num= SetupMenuNextPage();  

			if ((num== BackMove)||(num== ExitStkPerform)||(num== D_PrePageItem))
			{
				G_NextPageTag = FALSE;

				return CMD_Success;
			}
			
			num = GetResponseItemIdentifier();
			
			if(ProcessJavaMenuSelect(num) == TRUE)  //翻页选中的也是java菜单
			{
				//return  FALSE;
				goto _NextPage_;
			}
		}
#if UICC__OPERATOR__CHINAMOBILE		
		if(num >= D_NativeMeun)   //移动的usim卡信息
		{
			fileAddr = SelectLFFile(DF_STK,EF_Menu);
			
			recNum = GetLFRecNumLgth(fileAddr,FileNum);
			
			simInfo = 0xFF;
			
			for(i=1; i<=recNum; i++)
			{
				ReadLFBuf(fileAddr, i, 0x01, &menu[0], 0x01);
				
				if(menu[0] == num)
				{
					ReadLFBuf(fileAddr, i, 0x02, (BYTE*)&simInfo, 0x01);
					break;
				}
				else
				{
					return FALSE;
				}
			}
		#if UICC__APP__SIMINFO
			if(simInfo == G_SimImformationMenuId) //移动sim卡信息
			{
				SimInfor();
			}
		#endif
		}
#endif		
		
 #if UICC_USATTEST
	   fileAddr = SelectLFFile(DF_STK,EF_Menu) ; 
		
		for(i = 0; i < GetLFRecNumLgth(fileAddr, FileNum); i++)                              
		{
			ReadLFBuf(fileAddr, i+1,0x00, &menu[0], sizeof(menu));
 
			if((menu[1] == num) && ((menu[4]&0x80) != 0x00))       //根据项目标识符查找记录号
			{
				break;
			}
		} 
		
		USATTestApp(menu[2]);	//USAT test		
					
// 		if(G_NextPageTag == TRUE)
// 		{
// 			goto _NextPage_;
// 		}
// 		else
// 		{
// 			return 0x00;
// 		}
#endif	

			if(G_NextPageTag == TRUE)
		{
			goto _NextPage_;
		}
		else
		{
			return 0x00;
		}
	}
	
}
/*
*@function   :UICCAPITestJudge

*@description:判断当前是否是测试UICCAPI  由于uiccapi测试时候SetUpEventlist不期望有090A事件 所以加此函数
*
*@see         : TS 102268 Annex B  根据是否存在测试文件来判断
*
*@param		  :void
*
*@return	  :TRUE 当前不是API测试/FALSE 当前是API测试
*
*@author      :LCZ
*
*@history	  :V1.0 :create by LCZ 2019-5-9
*/
BYTE UICCAPITestJudge(void)
{
	UWORD xdata testApiAddr;
	
	testApiAddr = SelectTFFile(0x7F4A, 0x6F01);
	
	if(testApiAddr == FALSE)
	{
		return TRUE;
	}
	
	testApiAddr = SelectTFFile(0x7F4A, 0x5F10);
	
	if(testApiAddr == FALSE)
	{
		return TRUE;
	}
	
	testApiAddr = SelectFile(testApiAddr, 0x4F10);
	
	if(testApiAddr == FALSE)
	{
		return TRUE;
	}
	
	return FALSE;
}

/*
*@function   :IF_CurProfile_Support_HTTPS

*@description:判断当前profile的MNOSD是否支持https
*
*@see         : GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3
*
*@param		  :void
*
*@return	  :TRUE/FALSE
*
*@author      :xwh
*
*@history	  :V1.0 :create by xwh 20201123
*/

BYTE IF_CurProfile_Support_HTTPS(void)
{
#if (RSP_EUICC && GP__AMENDMENT__B__FOR_MASK)	
	u8         X_A    bFreeEntryIndex,index,appID;
	memref 	   X_A    entry,theAppTable,bPrivArray,sdDataObj;	
	jref       X_A    bPrivRef;
	
	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable = getAppTable();
	appID = ILLEGAL_APPID;
	
	for (index = APP_FIRST; index < bFreeEntryIndex; U16_INC(index))
	{
		entry = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(index, SIZE_REF))));

		appID = memReadU8(MEMtPN8(get_object_field_ref(entry, bAppID_INDEX)));

		if(appID == ILLEGAL_APPID)
		{
			continue;
		}

		bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));

		bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));

		if((U8_AND_U8(memReadU8(MEMtPM8(bPrivArray)), SECUREDOMAIN_PRIVI_TAG) != SECUREDOMAIN_PRIVI_TAG)
			||( appID == ILLEGAL_APPID))
		{
			continue;
		}
       if((appID >0) && (appID <5))
	   {
    	   continue;
	   }
		sdDataObj	=	getSdDataObjBySdid(appID);
		
		if(sdDataObj != MEM_NULL)
		{	
			if(GPGetSDAdminSessionPara85(MEMtPN8(sdDataObj), NULL) != 0)
			{
				return TRUE;
			}
		}
	}
#endif		
	return FALSE;
}

/*
*@function   :SetupEvenList

*@description:组装当前支持UIC JAVA API的setupevenlist主动式命令
*
*@see         : TS 102241   6.7.1.2
*
*@param		  :void
*
*@return	  :void
*
*@author      :xuyz
*
*@history	  :V1.0 :create by xuyz@20131121
*              V1.1 :modify by xwh 20201123, 当前profile的MNOSD支持https的情况下，才发送09/0A set up eventlist
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE SetupEvenList(BYTE mode)
{
 	BYTE idata i,j,k,p,tag,apitest; //apitest为了UICCPAI测试时候 不显示090a事件添加
	BYTE xdata tmp[EventList_Length];
	
	BYTE idata evenlistTag;
	UWORD xdata toolkitAppAddr;
	BYTE idata  toolkitAppRecNum; 
	//BYTE  idata httpsTag;
	
#if UICC__FUNCTION_CMCCAPI

	BYTE idata result;

	result = 0xFF;

#endif
	evenlistTag = FALSE;
	
	apitest = UICCAPITestJudge();
	
	if((G_TerminalProfile[0] < 5)||(ValueBit(G_TerminalProfile[5],0) == FALSE))
	{
		return FALSE;
	}

	ecpMem_Set(&tempBuf[0], 0xFF,0x20);
	ecpMem_Set(&tempBuf[50], 0,0x20);

	InitProactiveCommand(CMD_SETUPEVENTLIST,0x00,DEV_ME);

	tempBuf[50] = TLV_EVENTLIST&TLV_INVERT_CR;
    
	tempBuf[51] = 0x00;
	
#if M2M_EUICC
	if((ValueBit(G_TerminalProfile[0x05],4) == TRUE) && apitest)  //Event: Location status
	{
		tempBuf[52+tempBuf[51]] = 0x03;
		tempBuf[51]++;
	}
#endif
	if(apitest)
	{
		if(ValueBit(G_TerminalProfile[0x06],2) == TRUE)  //Event: Data available
		{
			tempBuf[52+tempBuf[51]] = 0x09;
			tempBuf[51]++;
		}

		if(ValueBit(G_TerminalProfile[0x06],3) == TRUE)  //Event: Channel status
		{
			tempBuf[52+tempBuf[51]] = 0x0A;	             
			tempBuf[51]++;
		}	
	}
	
#if UICC__FUNCTION_CMCCAPI

	result = PN_R_U8(MEMtPN8(JcvmConfig_GetOnLineStateBase()));
	
	if(result != 0x01)//API测试，开机不需要发03事件
	{
		if((ValueBit(G_TerminalProfile[5],4) == TRUE))
		{
			tempBuf[52+tempBuf[51]] = 0x03;	             
			tempBuf[51]++;

			RAM_Location_Status_Flag = 0x01;
		}
		else
		{
			RAM_Function_Supported = 0x00;
		}
	}
#endif	

#if UICC__FUNCTION__STKAPI
	evenlistTag = 1;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);	
	
	toolkitAppRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(i=1; i<= toolkitAppRecNum; i++)
#else
	toolkitAppRecNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for(i=0; i< toolkitAppRecNum; i++)
#endif
	{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
		toolkitAppAddr = GetToolkitAppInfoArrayAddr(GetToolkitInfoObj(i));

		if(toolkitAppAddr == ADDR_IS_NULL)
		{
			continue;
		}
#endif
		/*tag = */ReadLFBufForJavaAccess(toolkitAppAddr, i , AppletStatus_OFFSET, tmp,sizeof(tmp));

		if(/*(tag == FALSE)||*/((tmp[AppletStatus_OFFSET] != APPLET_UNLOCKED_SELECTABLE)))
		{
			continue;
		}

		  ReadLFBufForJavaAccess(toolkitAppAddr,i,EventList_OFFSET,&tmp[0], EventList_Length);

		for(j = EVENT_EVENT_DOWNLOAD_MT_CALL_bit; j<= EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit; j++)
		{
			if((j == EVENT_STATUS_COMMAND_bit)||(j == 24)) //24 :EVENT_FORMATTED_SMS_CB_bit //30 31 33  reserved by 3GPP
			{
				continue;
			}

			if(ToolkitGetEventBit(&tmp[0], j))
			{
				if(j <EVENT_STATUS_COMMAND_bit)	  //处理EVENT_EVENT_DOWNLOAD_MT_CALL_bit -  EVENT_EVENT_DOWNLOAD_CARD_READER_STATUS_bit
				{
					k = j-12;		 //0-6
					p = j - 11;

					if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[0x05],p) == TRUE))
					{
						tempBuf[k] = k;	  //k取值范围: 0 - 6 
					}
				}
				else if(j < 24)	   //处理EVENT_EVENT_DOWNLOAD_LANGUAGE_SELECTION_bit - EVENT_EVENT_DOWNLOAD_CHANNEL_STATUS_bit
				{
					k = j- 13;	  //7-0a

					p = j - 20;

					if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[0x06],p) == TRUE))
					{
						tempBuf[k] = k;	  //k取值范围: 7 - A 
					}
				}
				else if(j < 30)		 //处理事件 EVENT_EVENT_DOWNLOAD_ACCESS_TECHNOLOGY_CHANGE_bit  -  EVENT_EVENT_BROWSING_STATUS_bit
				{
					k = j - 14;   // 0B - 0F

					if(j != EVENT_EVENT_DOWNLOAD_BROWSING_STATUS_bit)
					{
						p = j - 21;

						if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[0x06],p) == TRUE))
						{
							tempBuf[k] = k;	  
						}
					}
					else
					{
						if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[25],0) == TRUE))
						{
							tempBuf[k] = k;	  
						}
					}
				}
				else if(j == EVENT_EVENT_DOWNLOAD_FRAMES_INFORMATION_CHANGED_bit)
				{
					k = 0x10;

					if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[25],2) == TRUE))
					{
						tempBuf[k] = k;	  
					}
				}
				else if(j == EVENT_EVENT_DOWNLOAD_CONTACTLESS_STATE_REQUEST_bit)
				{
					k = 0x16;
					
					if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[26],1) == TRUE))
					{
						tempBuf[k] = k;	  
					}	
				}
				else if((j == EVENT_EVENT_DOWNLOAD_HCI_CONNECTIVITY_bit) || (j == EVENT_EVENT_DOWNLOAD_IWLAN_ACCESS_STATUS_bit) || (j == EVENT_EVENT_DOWNLOAD_NETWORK_REJECTION_bit)) // 30 31 32
				{
					//k = 0x13;	   //0x13;
					
				    k = j - 13;  // 0x11 0x12 0x13           
					
					p = j - 27;  // 3 /4 /5

					if((tempBuf[k] == 0xFF)&&(ValueBit(G_TerminalProfile[25],p) == TRUE))
					{
						tempBuf[k] = k;	  
					}
				}
				else //白盒测试
				{
				}	
		
			}
		}
	}
	
	for(i =0;i< 0x20;i ++)
	{
		if(tempBuf[i] != 0xFF)
		{
			if(evenlistTag == TRUE)   //已经配置多09 0A 事件，此处需过滤掉 
			{
#if M2M_EUICC
				if((ValueBit(G_TerminalProfile[0x05],4) == TRUE) && apitest)  //Event: Location status
				{
					if(tempBuf[i]==0x03)
					{
						continue;
					}
				}
#endif
				if(apitest)
				{
					if(ValueBit(G_TerminalProfile[0x06],2) == TRUE) 
					{
						if(tempBuf[i]==0x09)
						{
							continue;
						}
					}
					
					if(ValueBit(G_TerminalProfile[0x06],3) == TRUE)  //Event: Channel status
					{
						if(tempBuf[i]==0x0A)	 //避免事件09，0A重复发送
						{
							continue;
						}
					}
				}				
			}

			#if UICC__FUNCTION_CMCCAPI

				if(result != 0x01)//API测试，开机不需要发03事件
				{
					if((ValueBit(G_TerminalProfile[5],4) == TRUE))
					{
						if(tempBuf[i]==0x03)//避免03事件重复发送
						{
							continue;
						}
					}
				}
			#endif

			//TODO:开机是否发送090a事件
			tempBuf[52+tempBuf[51]] = tempBuf[i];
			tempBuf[51]++;
		}
	}
    #endif


#if UICC__FUNCTION__STKAPI
	//中国电信，要求发送空的SetUp EventList
	//if(MOBILE_Operator != OPERATOR_CHINATELECOM)
	//{
		if((ValueBit(G_ToolkitSystemProactiveFlag,Bit_PowerOnSetupEventList) == TRUE) || (mode == Openchannel_Mode))
		{
			if(tempBuf[51] == 0x00)
			{
				return FALSE;	   //在会话开始时，如当前没有应用注册了event，则不发送主动式命令，测试规范Cre_Mha_Pahd.js，第一个terminalProfile的返回 ;移动用例
			}
		}

	//}

#endif  //电信UICCAPI测试中，此处代码如果加上，Test_Cre_Tin_Chal 测试不过
	//事件排序问题， 中国电信UICC API TEST测试脚本-->FRAMEWORK-->CATREEVENT REGISTRATION--->Test_Cre_Reg_Evtr: 期望菜单由小到大排序
	dataOrder(tempBuf[51], &tempBuf[52]);
		
	if(ecpMem_Compare(tempBuf+51,G_EventBufferOld,sizeof(G_EventBufferOld)) != 0)//检查事件列表有无变化，有的话才发送
	{
		ecpMem_Copy(G_EventBufferOld,tempBuf+51,sizeof(G_EventBufferOld));
	}
	else
	{
		return FALSE;
	}

	AddTLV(&ResData[1], &tempBuf[50] );	

	if(mode == Openchannel_Mode)
	{
		SendProactiveCommand();
	}
	else
	{
		STKPerform[0] = TRUE;

		STKPerform[1] = G_ChannelNum+1;
	}
	
	//G_TimerManagerFlag |= 2;

	return TRUE;
 }
/*
*@function    :IF_EnvelopGetKeyAddrBySDVersion(BYTE sdId, BYTE keyVersion)
*
*@description : 获取SD 下特定的某个 版本的密钥
*
*@param       :sdId
*@param       :keyVersion
*
*@return      :UWORD addr
*
*/
WORD IF_EnvelopGetKeyAddrBySDVersion(BYTE sdId, BYTE keyVersion, BYTE keyId, BYTE *keyBuff)
{
	WORD idata keyLen;
	
	if(keyVersion == 0x00)
	{
		keyVersion = 0x0F;   //在FORMAT短信中，如果当前key的版本号为00，则 指向某组特定的SCP80 密钥 ，SCP80的密钥索引是 01- 0F ,其他的密钥索引会当成SCP02的密钥去处理
	}
	
	keyLen = keyData_getKeyData(sdId, keyVersion, keyId, (UWORD)keyBuff); //查找特定version 的密钥

	return keyLen;
}

/*
*@function    :IF_GetFirstsCP80AESkeyByAppID(BYTE sdId, BYTE keyVersion)
*
*@description : GetFirstsCP80AESkeyByAppID
*
*@param       :sdId
*@param       :keyVersion
*
*@return      :UWORD addr
@ history     :add by cT@20220627
*
*/
BYTE IF_GetFirstSCP80AESkeyByAppID(BYTE sdId, BYTE *keyBuff)
{
	BYTE idata keyLen;
	
	BYTE idata keyVersion = 0;
	
	for(keyVersion = 0x01;keyVersion < 0x10; keyVersion++)
	{
		
		keyLen = keyData_getKeyData(sdId, keyVersion, TYPE_KEY_KID, (UWORD)keyBuff); //查找特定version 的密钥
		
		if(keyLen != 0)
		{
			return keyVersion;
		}
	}
	
	return keyVersion;

}
/*
*@description:从BLOCKDATA文件block header的长度(53820108)
*
*@return     :block header字节数
*
*@author    :create by xwh
*
*@history   :v1.0 create by xwh 20140717
*/
BYTE GetTLLength(UWORD dataAddr)
{
 	BYTE  idata buf[4],tlLength;
	
	ReadEEPromCache(&buf[0],dataAddr,sizeof(buf));

	if(buf[1] < 0x80)             //53 len
	{			           
		tlLength =  2;
	}
	else if(buf[1] == 0x81)       //53 81 len	
	{				       	
		tlLength =  3;
	}
	else if(buf[1] == 0x82)       //53 82 len1 len2
	{							   
		tlLength =  4;
	}
	else
	{
		return FALSE;
	}
	
	return tlLength;	
}
/*
*@description:从BLOCKDATA文件block data的长度(53820108)
*
*@return     :block data字节数
*
*@author    :create by xwh
*
*@history   :v1.0 create by xwh 20140717
*/
WORD GetDataLength(UWORD dataAddr)
{
	BYTE  idata buf[4];
	WORD  idata Length;

	ReadEEPromCache(&buf[0],dataAddr,sizeof(buf));

	if(buf[1] < 0x80)             //53 len
	{			           
		Length = buf[1];
	}
	else if(buf[1] == 0x81)       //53 81 len	
	{			
		Length = buf[2];	       	
	}
	else if(buf[1] == 0x82)       //53 82 len1 len2
	{							   
		Length = (((WORD)buf[2] << 8)&0xFF00) + buf[3];
	}
	else
	{
		return FALSE;
	}				 

	return Length;
}
/*
*@description:保存信息到BLOCKDATA文件中
*
*@return     :TRUE(命令已收齐)/FALSE(命令未收齐)
*
*@author    :create by xwh
*
*@history   :v1.0 create by xwh 20140717
*/
BYTE SaveBlockData(void)
{
	UWORD idata fileAddr;
	BYTE  idata tlLength;
	WORD  idata dataLength;
	
	fileAddr = SelectTFFile(DF_PROFILECFG,EF_AUTHRECDATA);//modify by zhangxiaoxiao@20220428 1、EF_GBA_KS放在USIM,ISIM下面，使用相同的文件ID 1B05 2、EF_AUTHRECDATA放在DF_PROFILECFG下面 3、私有文件模板也要相应的修改

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	if(P1 == P1_FirstBlockData)
	{
		G_BlockOffset = 0x0000;
	}

	WriteTFBuf(fileAddr,G_BlockOffset,RecData,P3);
	G_BlockOffset += P3;

	tlLength = GetTLLength(fileAddr+sizeof(TranFileHead));
	dataLength = GetDataLength(fileAddr+sizeof(TranFileHead));

	if(G_BlockOffset == (tlLength + dataLength))       //all segments of the authentication data are received
	{
		return TRUE;
	}

	return FALSE;
}
/*
*@description:处理INS为0x89时的数据
*
*@return     :TRUE(数据已接收完成)/其他情况需直接返回状态字
*
*@author    :create by yjr
*
*@history   :v1.0 create by xwh 20140717
*/
BYTE ODDAuth(void)
{
	BYTE  idata result;
	
	if((P1 != P1_FirstBlockData)&&(P1 != P1_NextBlockData))
	{
		if(P3 == 0x00)
		{
			if(G_ResponseLen == 0)
			{
				SW1 = 0x90;
			}
			else
			{
				SW1 = 0x6C;
			}			
			SW2 = G_ResponseLen;
			return 3;
		}
		else if(P3 != G_ResponseLen)
		{
			GlobalSetStatus(0x6700);
			
			return 3;
		}
		else
		{
		   ReadVarible(Tag_BakOddData,RecData);
		   SendData();
		   GlobalSetStatus(0x9000);
			
		   return 3;
		}									
	}
	else
	{
		ReceiveData();
		result = SaveBlockData();

		if(result == FALSE)
		{
			//As long as the UICC has not received all segments of the authentication data it shall answer with SW1 SW2 '63 F1'.
			GlobalSetStatus(0x63F1);
			
			return 3;	
		}
			
		return TRUE;
	}				
}

/*
*@description: 针对SIM API ，增加私有event的接口，如果当前event为自定义的私有event，则 在此函数中将私有event转换成COS支持的eventbit，触发应用时，使用eventbit进行触发
*
*@param      :event: 自定义的私有event
*@param       :BYTE *eventBit:存储转换后的ecentBit,可用 eventBit为 48-55，UICC API和SIM API 共用这些bit
*
*@return      :TRUE:当前event是私有event，转换后，直接退出 /FALSE:将event交由COS 处理
*
*@author      : xuyz
*
*@history    :V1.0: create --初始需求：在做移动两不一快，产业部的cap包set 了0xFB 的事件( 实际应用并未使 用到该event)，为兼容;同时据了解，SWP卡支持私有event
*/
BYTE IF_SIMAPIPrivateEventSupport(jbyte event,BYTE * eventBit)
{
	BYTE idata result;
	
    #if  SIMAPI_SUPPORT_123EVENT//联通测试第三项也要支持注册事件EVENT_PROACTIVE_HANDLER_AVAILABLE
	if(event == 0x7B)  //在支持CMCC API 的情况下，SIM toolkit也支持事件EVENT_PROACTIVE_HANDLER_AVAILABLE
	{
		eventBit[0] =  EVENT_PROACTIVE_HANDLER_AVAILABLE_bit;
		
		return TRUE;
	}
    #endif	
	
    #if  VERIFYPINEVENT
	if(event == EVENT_CHV1_VERIFIED)			 //added EVENT_CHV1_VERIFIED 20140813 liguiyu
	{
		* eventBit = EVENT_CHV1_VERIFIED_bit;
		
		return TRUE;
	}
    #endif
	result = FALSE;
	
	return result;
}
/**
 * @brief 		MAIN函数中，接收完5字节APDU头后的接口函数
 *
 * @RETURN		TRUE： 不继续执行Base库中的APDU指令，该函数退出后，重新接收新的APDU
 				FALSE: 继续执行Base库中的后续APDU指令
 *
 * @author 		xuqs
 *
 * @history 	created by xuqs, @100127 20:01
 */
//BYTE IF_MainReceiveApdu(void)
//{
//#if UICC__APP__WHITECARD
//	if (G_FirstAPDU == 0)
//	{
//		G_FirstAPDU = 1;
//		CheckProtection();
//	}
//	if(INS == 0xA4)
//	{
//		if(P1 == 0x04)
//		{
//			if(G_AID_tag == G_ChannelNum)
//			{
//				 G_AID_tag = 0xFF;  //当前通道上收到其他的A4指令，则清除AID tag
//			}
//			if(SDK_selectAID() == TRUE)
//			{
//				return TRUE;
//			}
//		}
//		else
//		{
//			if(G_AID_tag == G_ChannelNum)
//			{
//				ResponseFunction(0x6A,0x86);
//				return TRUE;
//			}
//		}
//	}
//	if(G_AID_tag == G_ChannelNum)	 //当前通道上选中的是白卡入网应用，则不允许处理其他非法命令，可以直接报错
//	{
//		if((INS != 0xA5)&&(INS != 0xB5)&&(INS != 0xA9))
//		{
//			ResponseFunction(0x6D,0x00);
//			return TRUE;
//		}
//		if((INS == 0xA5) || (INS == 0xB5) || (INS == 0xA9))
//		{
//			if(CLA >= 0x04)
//			{
//				ResponseFunction(0x6D,0x00);
//				return TRUE;
//			}
//		}
//	}
//#endif
//	return FALSE;
//}
/*
×@function     :IF_CheckTarSupportSingleDES

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
		if((G_TARRecIndex[0] == TarType_RAM)||(G_TARRecIndex[0] == TarType_RFM))  //RAM RFM不能支持单des
		{
			return FALSE;
		}	
		else  //TOOLKIT
		{
			return TRUE;
		}
#endif		
	}
	return TRUE;
#endif 	
}

/*
*@descritpion:bip的channelStatus事件，当前通道可有是http 或者 cattp， 需要关闭当前bip通道处理
*
*@function   :IF_BipChannelStatus
*
*@param      :void
*
*@return     :void
*
*@history    :jiangqb-20200311
*/
void IF_BipChannelStatus()
{	

	IF_AnnexB_ChannelStatus();
#if UICC__FUNCTION__CATTP
	IF_CATTP_ChannelStatus();
#endif
}



#if (UICC__FUNCTION__UICCAPI | UICC_RFM_FILE_UPDATE_EVENT) 
/*
*@description: 获取当前更新文件的路径，最大支持4级DF+1级EF，返回的是文件的depth
*
*@function   :IF_GetCurrentFilePath
*
*@param      :BYTE * pathInfo: 10字节长
*
*@return     :BYTE pathDep
*
*@author      :xuyz
*
*@history     :xreate by @20180827
*/
BYTE IF_GetCurrentFilePath(BYTE *pathInfo)
{
	BYTE idata pathDep,i;
	WORD idata parentFileName[0x02];
	
	ecpMem_Set(pathInfo, 0xFF, 0x0A);
	
	pathInfo[0] = 0x3F;
	
	pathInfo[1] = 0x00;
	
	pathDep = 1;
	
	ReadEeprom((BYTE *)&parentFileName[0],G_CurrentDfAddr[G_ChannelNum] + F_ParentName_Offset,sizeof(WORD)*2); //读取当前DF 的父目录信息
	
	for(i=0; i<2; i++)
	{
		if(parentFileName[i] != 0x3F00)
		{
			pathInfo[pathDep*2] = (parentFileName[i] >>8)&0xFF;
			
			pathInfo[pathDep*2 + 1] = parentFileName[i]&0xFF;
			
			pathDep ++;
		}
	}
	if(G_CurrentDfAddr[G_ChannelNum] != E_FILE_START)
	{
		pathInfo[pathDep*2] = ((G_CurrentDfName[G_ChannelNum] >> 8)&0xFF);

		pathInfo[pathDep*2 + 1] = (G_CurrentDfName[G_ChannelNum]&0xFF);
		
		pathDep ++;
	}

	pathInfo[pathDep*2] = ((G_CurrentEfName[G_ChannelNum] >> 8)&0xFF);

	pathInfo[pathDep*2 + 1] = (G_CurrentEfName[G_ChannelNum]&0xFF);
	
	pathDep ++;
	
	return pathDep;
}
#endif
#if UICC_RFM_FILE_UPDATE_EVENT
/*
*@description:在处理RFM的Update或Increase操作时，保存当前文件的路径和fileUpdateInfo
*
*@function   :IF_SaveRFMRemoteFileUpdate
*
*@param      :void
*
*@return     :void
*
*@author     :xuyz
*
*@hisotry    :@create 20180827
*/
void IF_SaveRFMRemoteFileUpdate(WORD position,WORD len)
{
	UWORD xdata fileAddr;
	BYTE idata i,tmp,recNum;
	
	fileAddr =  SelectLFFile(DF_Toolkit_P, EF_RFMFileUpdate);  //该线性文件的结构为 1字节的路径深度 + 10字节的filePath(3F00XXXXXX，多余字节补上FF) + 6 字节的 file UpdateInfo Object(TLV 结构)
	
	if(fileAddr == FALSE)   //缓存fileUpdate Info的文件不存在
	{
		return;
	}
	
	ecpMem_Set(&tempBuf[1], 0xFF, 0x10);
	
	tempBuf[0] = IF_GetCurrentFilePath(&tempBuf[1]);
	
	tempBuf[11] = 0x3B;												//File Update Information

	tempBuf[12] = 0x04;											//len

	tempBuf[13] = position>>8;

	tempBuf[14] = position&0xFF;

	tempBuf[15] = len>>8;

	tempBuf[16] = len&0xFF;
	
	recNum =  GetLFRecNumLgth(fileAddr, FileNum);
	
	for(i = 1 ; i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0x00, &tmp, sizeof(BYTE));  //读取第一字节
		
		if(tmp == 0x00)
		{
			break;
		}
	}
	
	if(i > recNum)
	{
		return;
	}
	
	WriteLFBuf(fileAddr, i, 0x00, tempBuf, 0x11);	
} 
/*
*@description: fileList在app中存在
*
*@function    :ExistFileListPath
*
*@param       :BYTE *filePath
*@param       :BYTE pathDep
*
*@return      :void
*
*@author      :xuyz
*
*@history     :modify by xuweihua@20211115 如果toolkitNum 大于EF_ToolkitAppPathInfo 的记录个数，说明EF_ToolkitAppPathInfo文件记录个数不够，报错
*/
BYTE ExistFileListPath(BYTE *filePath, BYTE pathDep, BYTE toolkitAppIndex)
{
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
	u8     X_A  tmpDf[0x0A];  // path 长度为A
	u8     X_A  tmpEf[0x0A];  // path 长度为A

	ecpMem_Set(tmpDf, 0xFF, sizeof(tmpDf));
	ecpMem_Set(tmpEf, 0xFF, sizeof(tmpEf));

	ecpMem_Copy(&tmpDf[0],&filePath[0],(pathDep-1)*2);						 //查找其父目录是否有注册FILE_UPDATE事件
	ecpMem_Copy(&tmpEf[0],&filePath[0],pathDep*2);		 ////查找ef是否有注册FILE_UPDATE事件

	if((SearchFileList(&tmpDf[0],toolkitAppIndex, MODE_REMOTE_FILE_LIST) != FALSE)||(SearchFileList(&tmpEf[0],toolkitAppIndex, MODE_REMOTE_FILE_LIST) != FALSE))
	{
		return TRUE;
	}

#else
	UWORD xdata toolkitPathInfoAddr;
	BYTE xdata tmp[0x0A];  // path 长度为A 和文件 toolkitPathInfoAddr 记录长度/2 取最大值
	BYTE idata flag,blkNum_DF,blkNum_EF,recLgth,j,recNum;
		
	ecpMem_Set(tmp, 0xFF, sizeof(tmp));
			
	ecpMem_Copy(&tmp[0],&filePath[0],(pathDep-1)*2);						 //查找其父目录是否有注册FILE_UPDATE事件
	
	flag = FALSE;

	if(SearchFileList(&tmp[0],&blkNum_DF) == TRUE)
	{
		flag = TRUE;
	}
	
	ecpMem_Set(tmp, 0xFF, sizeof(tmp));
	
	ecpMem_Copy(&tmp[0],&filePath[0],pathDep*2);		 ////查找ef是否有注册FILE_UPDATE事件
	
	if(SearchFileList(&tmp[0],&blkNum_EF) == TRUE)
	{
		flag = TRUE;
	}
	
	if(flag == FALSE)
	{
		return FALSE;
	}
	
	toolkitPathInfoAddr = SelectLFFile(DF_Toolkit_P, EF_ToolkitAppPathInfo);		

	recLgth = GetLFRecNumLgth(toolkitPathInfoAddr, FileLgth)>>1;	
	recNum = GetLFRecNumLgth(toolkitPathInfoAddr, FileNum);	//记录前半段注册EVENT_EXTERNAL_FILE_UPDATE，后半段注册EVENT_REMOTE_FILE_UPDATE

	if(recNum >= toolkitAppIndex)
	{
		ReadLFBuf(toolkitPathInfoAddr, toolkitAppIndex, recLgth ,tempBuf, recLgth);		  //记录内容为LV结构

		for(j = 1;j <= tempBuf[0];j ++ )
		{
			if((tempBuf[j]==blkNum_DF) || (tempBuf[j]==blkNum_EF))									  //若找到对应的注册信息，则将对应记录号放到待触发App列表（EF_TookitAppPathInfo的记录号与6f80文件记录号一一对应）
			{					
				return TRUE;
			}
		}
	}
		
#endif
	return FALSE;
}

/*
*@description:检查登记的RFM 文件是否还存在,如果不存在，则删除该条记录
*
*@参考规范    ：TS102241 In case of an update or increase to a file that  is deleted within the same command string the file shall not be included in the File List TLV for this  event
*
*@function   :RFMUpdateFileDelete
*
*@param      :void
*
*@return      :
*
*@author     :xuyz
*
*@hisotry     :V1.0 :create@20180828
*/
void RFMUpdateFileDelete(void)
{
	BYTE idata i,j,recNum;
	UWORD xdata fileAddr,pathAddr;
	
	fileAddr = SelectLFFile(DF_Toolkit_P, EF_RFMFileUpdate);
	
	recNum =  GetLFRecNumLgth(fileAddr, FileNum);
	
	for(i = 1 ; i <= recNum; i++)
	{
		ReadLFBuf(fileAddr, i, 0x00, RecData, 0x11);  //缓存记录数据,记录长度固定为17字节，不再通过获取文件的记录长度来进行文件的读取
		
		if(RecData[0] == 0x00)
		{
			continue;
		}
		
		pathAddr = E_FILE_START;
	
		for(j=0; j< RecData[0]; j ++)
		{	
			pathAddr =  SelectFile(pathAddr,BuildWord(&RecData[2*j+1]));
			
			if(pathAddr == FALSE)
			{
				break;
			}	
		}
		
		if(pathAddr == FALSE)  //In case of an update or increase to a file that is deleted within the same command string the file shall not be included in the File List TLV for this  event.
		{
			SetLFConst(fileAddr, i, 0x00, 0x00, 0x11); //如果文件已经不存在，则清空本条记录
		}
	}	
}
/*
*@description:查找注册了RFM_File_Update的应用，并将应用记录号拼装在文件中
*
*@function    :SearchEventAppRec
*
*@param      :BYTE *buf:存储记录号的缓存，假设最大不超过20字节
*
*@return     :当前支持该event的总的应用数目
*
*@author     :xuyz
*
*@history    :V1.0 :create @20180828
*           :modify by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
*                                         用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
*/
BYTE SearchEventAppRec(BYTE *buff, BYTE eventbit)
{
	BYTE idata i, tmp, state, num, appRecNum;
	BYTE xdata eventlist[EventList_Length];
	UWORD xdata toolkitAppAddr;
	
	num = 0;
#if !UICC__STKAPI_TOOLKITINFO_IN_HEAP
	toolkitAppAddr = SelectLFFile(DF_Toolkit_P_Ex, EF_ToolkitApp);

	if(toolkitAppAddr == FALSE)
  	{
		return FALSE;
  	}
	
	appRecNum = GetLFRecNumLgth(toolkitAppAddr, FileNum);

	for(i=1; i<=appRecNum;i++)	 //查找事件匹配记录
#else
	appRecNum = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);

	for(i=0; i<appRecNum;i++)	 //查找事件匹配记录
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
			buff[num++] = i;
		}
		
		if(num >= 20)
		{
			break;
		}
	}
	
	return num;
}
#endif
/*处理安装指令中的CA EA参数*/
void ProcessUICCPara(void)
{
    BYTE idata sdMode;

	WORD idata result = 0x9000;
	WORD   context;
	sdMode = popReference();
	context = popReference();

#if UICC__FUNCTION__SIMAPI	  //为方便以后提供lib，将Toolkit_InitSIMView提前到这里
	Toolkit_InitSIMView(G_CurSelectedChannel,0xFF);   //为支持在安装时进行文件的选择、更新等操作 ,如果当前是SIM API，则对上下文进行赋值操作
    #endif

	result = NativeImpl_InstallForInstall(cacheBuffer/*APDUBuffer*/,context,sdMode);

	pushShort(result);
}

/*抛异常 从run函数返回，用于重入时退出run函数*/
void throwPrivateError(void)
{
	throw_error(0x90FF);
}
/*设置Java返回的状态字*/
void SetJavaReturnSW(u16 status)
{
	G_GPErrorCode = status;
}

/*查找需要使用哪个SSD进行SCP80加解密,传入的appID一定是一个SD*/
BYTE SearchClosestSCP80SDId(BYTE appId)
{
	u8 idata associateSDId;

	while(1)
	{
		if(U8_AND_U8(GetCurSDSCP80and02Key(appId),0x80) == 0x80)//GPSSDExistCompleteSecureKey(appId,SCP_80) ?
		{
			return appId;
		}

		associateSDId = getAssociateSDId(appId);

		if(associateSDId == 0xFF)//已经查找到最顶 都没有scp80
		{
			return ILLEGAL_APPID;
		}

		appId = associateSDId;
	}

//	return ISD_APPID;

}

/*选择ADF ，匹配成功返回TRUE 未找到返回FALSE，同时设置状态字errorCode*/
u8 AppAPI_SelectByFileId(PX8 apduBuffer, PX8 errorCode)
{
	u32   D_A   fileAddr;

	//在没有创建任何文件前，E_FILE_START 的值是0xFFFFFFFF，此时直接退出 
	if((E_FILE_START == INVALID_ADDR)||(E_FILE_START == 0))
	{
		return FALSE;
	}

	fileAddr = SelectFileByName(E_FILE_START,&APDUBuffer[5],APDUBuffer[4]);  //SE访问控制应用测试\东信\SELECT DF（7F50）\6.3.1.10,步骤5 ，要求支持部分选择ADF

	if(fileAddr != FALSE)
	{
		return TRUE;//暂时没有处理当前通道上不是UICC APP时选择ADF的情况，该情况需要失选通道上的应用并且将UICCAPP置为当前选择的通道上的应用
	}

//#if UICC__APP__WHITECARD
//	return SDK_selectAID();
// #endif

	return FALSE;
}

void ThrowErrorBackToMain(WORD exceptionType)
{
	bRamBuffer[IS_UICC_JUMP_OFF] = TRUE;
	throw_error(exceptionType);
}
/*
*@description: 触发EVENT_REMOTE_FILE_UPDATE的处理，在RFM POR 发送成功后
*
*@function    :IF_TriggerRemoteFileUpdate
*
*@param       :void
*
*@return      :void
*
*@author      :xuyz
*
*@hisotry     :V1.0 create@20180828 
*			   该函数的调用支持在STK主动式命令模式下触发 
*              该函数占用了缓存 RecData[0]- RecData[0x11] RecData[100]- RecData[100+x]
*				由tpx2.0移植到tpx3.0-20200311-jiangqb
*/
void IF_TriggerRemoteFileUpdate(void)
{
#if UICC_RFM_FILE_UPDATE_EVENT
	
	BYTE idata adfFlag,recNum,lgth;
	BYTE idata i,k,j, backi, appNum,fileType;
	UWORD xdata fileAddr;
	WORD idata adfName;
	BYTE xdata appList[20];
	
	fileAddr = SelectLFFile(DF_Toolkit_P, EF_RFMFileUpdate);
	
	if(fileAddr == FALSE)
	{
		return;
	}
	
	appNum = SearchEventAppRec(appList, EVENT_REMOTE_FILE_UPDATE_bit);
	
	if( appNum == 0)
	{
		//ClearUpRFMFile(fileAddr);
		ClearFileConst(fileAddr,0x00);
		
		return;
	}
	
	RFMUpdateFileDelete();  //RFM 操作中可能会删除文件，删除的文件可能在文件更新列表中，所以要先将删除的文件过滤掉
	
	SortApplistEvent(appList,appNum);  //先按照安装顺序将 注册了 event REMOTE_FILE_UPDATE 的应用 排序好，分别去触发。 检查这些应用有没有注册下列的文件
	
	recNum =  GetLFRecNumLgth(fileAddr, FileNum);
	
	//为了防止G_EnvelopHanderAddr的数据被破坏，此处必须保存现场
	ToolkitSaveAPPContext();	

	
	for(k = 0; k < appNum; k++)
	{
		RecData[100] = 0x00; // 由于缓存有多少条记录文件 被当前应用触发 
		
		adfFlag = FALSE;
		
		adfName = FALSE;
		
		for(i = 1 ; i <= recNum; i++)  //登记属于应用appList[k]的文件列表
		{
			ReadLFBuf(fileAddr, i, 0x00, RecData, 0x11);  //缓存记录数据,文件的长度固定为17字节，不再通过读取文件的长度来获取
			
			if(RecData[0] == 0x00)
			{
				continue;
			}
					
			if(ExistFileListPath(&RecData[1], RecData[0],appList[k]) == TRUE)  //当前更新的文件路径存在于某个app中，记录下来，待后续拼装
			{
				RecData[100] ++;
				
				RecData[100+RecData[100] ] = i;
			}
			else
			{
				continue;
			}
		
			if(adfFlag == FALSE)
			{
				if(RecData[0] > 2)  //文件深度超过了2级
				{
					fileType = GetFileHeadByte(SelectFile(E_FILE_START,BuildWord(&RecData[3])),F_DIRNum_offset);
					
					if(fileType != 0x00)  //属于app的有效路径里有ADF，则将tag保存下来
					{
						adfFlag = TRUE;
						
						adfName = BuildWord(&RecData[3]);
					}
				}
			}
		}	
		
		if(RecData[100] == 0x00)  //当前应用没有注册RFM更新的文件
		{
			continue;
		}
		
		tempBuf[0] = 0x82;						 //Device Identity

		tempBuf[1] = 0x02;

		tempBuf[2] = 0x82;

		tempBuf[3] = 0x81;
		
		i = 4;
		
		if(adfFlag == TRUE)  //当前file update的文件存在于ADF 中,拼装AID TLV
		{
			tempBuf[i++] = 0x2F;	
			
			GetCurrentADFAid(adfName , &tempBuf[i]);

			ecpMem_Move(&tempBuf[i], &tempBuf[i+1], tempBuf[i+1]+1);

			i += tempBuf[i] + 1;
		}
		
		tempBuf[i] = 0x12;  //File List, as defined in TS 102 223 [7].
		
		backi = i+1;  //point to filelistlgth offset
		
		tempBuf[i + 2] = RecData[100];
		
		 i += 3;
		
		lgth = 1;
		
		for(j=1; j <= RecData[100]; j++)  //拼装filelist
		{
			ReadLFBuf(fileAddr, RecData[100+j], 0x00, RecData, 0x0B);  //读取前11字节 dep+ 10字节的filePath
			
			if(RecData[0]>2)
			{
				if((RecData[3] == ((adfName>>8)&0xFF))&&(RecData[4] == (adfName&0xFF))&&(adfFlag == TRUE))
				{
					RecData[3] = 0x7F;
					RecData[4] = 0xFF;
				}
			}
			
			ecpMem_Copy(&tempBuf[i], &RecData[1], RecData[0]*2);
			
			i += RecData[0]*2;
			
			lgth += RecData[0]*2;
		}
		
		tempBuf[backi] = lgth;
		
		for(j=1; j <= RecData[100]; j++)  //拼装filelist
		{
			ReadLFBuf(fileAddr, RecData[100+j], 0x0B, RecData, 0x06);  //读取前11字节:  dep+ 10字节的filePath
			
			ecpMem_Copy(&tempBuf[i], &RecData[0], 6);
			
			i += 6;
		}
		
		fileType = WriteTFBuf(G_EnvelopHanderAddr, 0, &tempBuf[0], i);
		
		if(fileType == FALSE)
		{
			continue;
		}
		
		ToolkitHandlerList[Index_EnvelopeHandler].TLVListLength	  = i;

		JavaToolkitProcess(EVENT_REMOTE_FILE_UPDATE_bit,appList[k]);
	}

	ToolkitRestoreAPPContext();		
	
	//ClearUpRFMFile(fileAddr);
	ClearFileConst(fileAddr,0x00);
	
#endif
}
/*
@description: 设置9300的公共函数
*
*@function  :SetSTKBusy
*
*@param      :void
*
*@return      :void
*
*@author      :xwh
*
*@history     :V1.0:create by xwh 20180428
*/
BYTE SetSTKBusy(void)
{
	if(G_ISOBusyExceptionFlag == TRUE)
	{
		G_ISOBusyExceptionFlag = FALSE;	
		GlobalSetStatus(0x9300);
		return TRUE;
	}

	return FALSE;
}
#if (UICC__FUNCTION_CMCCAPI||UICC__FUNCTION__UICCAPI)
void ClearTheVolatileByteArray(void)
{
	u16   D_A     ref,arrayLen;
	PN8   X_A     refAddr;

	ref = nvmReadU16(MEMtPM8(TheVolatileByteArray_JC));

	if((ref != REF_NULL)&&(ref != 0xFFFF))
	{
		refAddr = MEMtPF8(locateObjectInMemory(ref));

		arrayLen = get_array_length(refAddr);

		refAddr = MEMtPN8(get_array_data(refAddr));

		ecpMem_Set(refAddr,0,arrayLen);
	}
}
#endif


/*@Function:    UiccOrSimThrowToolkitException                                            
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
void UiccOrSimThrowToolkitException(BYTE errorCode, BYTE apiMode)
{
	if(apiMode == UICCAPI_Mode)
	{		
		UICCThrowToolkitException(errorCode);
	}
	else
	{               
		ThrowToolkitException(errorCode);
	} 
}
/*@Function:    UiccOrSimViewThrowException                                            
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
void UiccOrSimViewThrowException(BYTE errorCode, BYTE apiMode)
{
	if(apiMode == UICCAPI_Mode)
	{
		UICCThrowUICCException(errorCode);
	}
	else
	{
		ThrowSIMViewException(errorCode);	
	}
}

/*
*@function     :GetBERTlvLen
*
*@description  :获取BER编码规则的TLV中，L域占用字节数，以及L的值即value占用的字节数
*
*@see          :
*
*@param        :UWORD srcAddr -- 输入参数，指向 BER TLV编码Len的起始即Tag后第一字节，可能是 xx/81 xx /82 xxxx / 83 xxxxxx
*@param        :UWORD* len    -- 输出参数，BER TLV结构中的Len值
*
*@return       :BYTE          -- 返回L域占用的字节数
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@20181113
*              :V1.1 modify by xuxiaojun@20190326  eUICC test RSP 20190301\TY_Offline_RSP_ManageCmdTest \ Download_260K_size_profile 要求下载260K的profile，传入的长度为83 029696
*/
BYTE GetBERTlvLen(UWORD srcAddr,UWORD* len)
{
	BYTE xdata temp[4];
	WORD idata lenNum;

	lenNum = 0x00;

	VMReadBuf(&temp[0],srcAddr,4); //len最大可能为4字节  83 xx xx xx

	*len = GetBerTlvLength(temp,&lenNum);

	return (lenNum-1);
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

/*
*@dexription:处理移动安全报文格式的tar值
*
*@function   :IF_ExcuteChinaMobileFormattedEnvelopSMS
*
*@param      :BYTE *currentOTATar: 当前的OTA 应用tar
*@param      :BYTE refrence:对于单条短信 ，该字段 为处于6F7F 中的记录号，对于多条短信 ，该字段为批次号
*@param       BYTE headNo:该下行数据在6F7E中的记录号
*@param     :BYTE smsType: TYPE_SMS_ENV_SINGLE/TYPE_SMS_UPD_SINGLE/TYPE_SMS_ENV_MULTI/TYPE_SMS_UPD_MULTI
*@pram       :BYTE eventId:用于触发应用
*@param       BYTE * tpoa :LV 结构，如  08 0C 81 106540001000
*
*@return     :void
*
*@history    :V1.0:create by xuyz
			  V1.1:在正式处理TF 数据前，就要清空缓存中的数据
*/

void IF_ExcuteChinaMobileFormattedEnvelopSMS(BYTE *currentOTATar, BYTE reference, BYTE headNo, BYTE smsType, BYTE eventId, BYTE *tpoa)
{	
#if UICC__APP__WHITECARD
	if((currentOTATar[0] == 0xB0)&&(currentOTATar[1] == 0x01)&&(currentOTATar[2] == 0x2B))  //中国移动白卡入网应用
	{
		WhiteCardSingleMsgProcess(reference, headNo, smsType,tpoa);
		ToolkitClearAllData(smsType,reference,tpoa);      //清除短信	,白卡入网应用有断电的测试，所以在执行前，先清除缓存，后续要考虑如果数据收满的情况下，断电缓存怎么办
		SMS_WhiteCardWriteCardHandler();
		return;
	}	
#endif

#if UICC__APP__LIVECARD
	if((currentOTATar[0] == 0xB0)&&(currentOTATar[1] == 0x00)&&(currentOTATar[2] == OTA_LiveWriteCard_Tag))  //OTA RFM的执行过程
	{	
	  LiveWriteCardHandler( reference,  headNo,  smsType, tpoa);
	
	  ToolkitClearAllData(smsType,reference,tpoa);      //清除短信	
		
	  return;
	}
#endif	
		
}

#if UICC__APP__WHITECARD
WORD WhiteCardSingleMsgProcess( BYTE reference, BYTE headNo, BYTE smsType, BYTE *tpoa)
{
	BYTE idata recNo;
	WORD idata offset,udlLength;
	UWORD xdata fileAddr;
	WORD idata cplDataOffset;
	recNo = ToolkitFindSpecifiedRFMSMS(MODE_FIND_RECNO,reference,0x01,smsType,tpoa);     //查找第1条短信
	fileAddr = SelectPrivateLFFile(DF_Toolkit,EF_ToolkitRFMSms);
	ReadLFBuf(fileAddr,recNo,17,&tempBuf[0] ,sizeof(BYTE));
	udlLength = tempBuf[0];
	offset = ToolkitProcessHeader(headNo,udlLength,0x01);
	cplDataOffset = offset;
	ReadLFBuf(fileAddr,recNo,18,&tempBuf[0],udlLength);
	if (udlLength > 0x8C)
	{
		return 0;
	}
	WriteTFBuf(G_EnvelopHanderAddr,offset,&tempBuf[0],udlLength); //更新完整的数据到G_EnvelopHanderAddr文件中
	fileAddr = SelectTFFile(DF_STK, EF_WhiteCardTPOA);
	WriteTFBuf(fileAddr,0, &tpoa[0], tpoa[0]+1);
	return cplDataOffset;
}
#endif

#endif
/*
*@description: 处理级联的unformatSMS,数据已缓存在G_EnvelopHanderAddr中，对于支持UDHI的unformatsms，长度后面补了一个字节的0
*
*@function   :IF_ExcutePrivateUnformatSMS
*
*@param      :WORD cploffset : 数据在G_EnvelopHanderAddr中的偏移
*@param      :BYTE * tpoa
*
*@return     :TRUE:私有tar自己处理了/ FALSE：抛给toolkit applet去处理
*
*@author     :xuyz
*
*@hisotry    :
*/
BYTE IF_ExcutePrivateUnformatSMS(WORD cploffset, BYTE * tpoa) 
{
#if UICC__APP__WHITECARD
	WORD idata dataLgth;
	UWORD xdata fileAddr;
	ReadTFBuf(G_EnvelopHanderAddr,cploffset,&tempBuf[0],0x0A);  //读取CPL (2字节) + CHL(1字节)+ SPI(2字节)+ KIC (1字节)+KID(1字节)+ TAR(3字节)
	if((tempBuf[7] == 0xB0)&&(tempBuf[8] == 0x01)&&(tempBuf[9] == 0x2B))  //白卡入网应用的 tar值
	{
		dataLgth = tempBuf[0]*256+tempBuf[1]+2;
		ReadTFBuf(G_EnvelopHanderAddr,cploffset,&tempBuf[0],dataLgth); 
		fileAddr = SelectTFFile(DF_STK, EF_WhiteCardTPOA);
		if(fileAddr == FALSE)
		{
			return FALSE;
		}
		WriteTFBuf(fileAddr,0, &tpoa[0], tpoa[0]+1);
		SMS_WhiteCardWriteCardHandler();
		return TRUE;
	}
#endif
	return FALSE;
}
/*
*@description :为支持某个native应用模拟java应用，在此接口中增加native APP应用的处理，如果当前AID是由native实现的java应用，则返回TRUE,否则返回false
*
*@function    :IF_SelectNativeJavaAPP
*
*@param        :BYTE *aidBuffer
*@param        :BYTE aidLength
*
*@return        :TRUE: 用native 模拟的java应用/FALSE:不是native 模拟的java应用
*
*@shitory      :
*/
//BYTE IF_SelectNativeJavaAPP(BYTE *aidBuffer,BYTE aidLen)
//{
//#if UICC__APP__WHITECARD
//	UWORD xdata fileAddr;
//	fileAddr = SelectLFFile(DF_STK, EF_WhiteCardAID);
//	if(fileAddr == FALSE)
//	{
//		return FALSE;
//	}
//	ReadLFBuf(fileAddr, 1, 0, &WhiteCardBuf[0], 0x26);
//	if(WhiteCardBuf[0] != aidLen)
//	{
//		return FALSE;
//	}
//	if(ecpMem_Compare(aidBuffer, (BYTE *)&WhiteCardBuf[1], aidLen ) == EQUAL)
//	{
//		return TRUE;	   //支持部分选择白卡入网应用
//	}
//#endif
//	return FALSE;
//}

void IF_UICCInition(void)
{
#if UICC__APP__WHITECARD
	AppWriteResult = 0; 
	G_StatusSendSMS = 0;
	G_StatusFirstTime = 0;
	G_StatusSendSMSCounter = 0;
	G_auth = 0;
	G_SDK_command_step = 0;
	G_applySMS = 0;
	G_applyActivate = 0;
	G_SMSCounter = 0;
	G_AID_tag = 0xFF;
	//G_FirstAPDU = 0;
	//if (G_FirstAPDU == 0)
	{
		//G_FirstAPDU = 1;
		CheckProtection();
	}
	G_personalDataTagM = 0;
	G_personalDataTagO = 0;
	G_Lgth_P3 = 0;
	G_SDK_command_step = 0; 
#endif		
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
// #ifdef UICC_GBA
// 	UWORD idata fileGBAModeAddr;
// 	BYTE idata gbaAuthMode;
// #endif
		
#if UICC__AUTH__5G__SUPI
	if(INS != 0x78)
	{
		G_SUPILEN = 0x00;
	}
#endif
	
#if UICC_SUPPORT_POR_DELIVER_TO_SUBMIT	
	if((G_Deliver_T0_Submit & 0x02)==0x02)
	{
		if((INS == 0x0C0)&&((CLA & 0x03)==0x00))
		{
			Toolkit_ResponsePack(POR_OK,PORMODE_COMMON|PORMODE_RESEND,G_PorAdditionalLen, 0x00);//modify by jiangqingbin-20220303, 增加一个传入参数，表示http短信触发结果，不再使用原来的复用SW1，SW2的方式 
		}
		else         //deliver级联时候，发送状态字9FXX等之后接着不是getResponse,则清空G_Deliver_T0_Submit变量，deliver数据抛弃
		{
			G_Deliver_T0_Submit = 0x00;
		}

		ecpMem_Set(G_TARRecIndex, 0x00, sizeof(G_TARRecIndex));  //deliver 转submit第二次进入POR函数使用到G_TARRecIndex

		G_formatedTPOAOffset = 0;

		G_FormatAndType = FormatType_NULL;					//还原初值
	}

	if(G_CurrentRFMResponse != FALSE)
	{
		if((G_Deliver_T0_Submit & 0x01)==0x01)
		{
			G_Deliver_T0_Submit &=0xFE;          //bit1 标志有级联deliver,还原
			G_Deliver_T0_Submit |= 0x02;         //bit2 标志第二次调用Toolkit_ResponsePack函数
		}
	}
#endif
}
#if UICC__APP__WHITECARD
void WhiteCard_Main(void)
{
	WORD  sw1sw2 = 0x9000;

	G_ChannelNum = selected_channel;
	switch(INS)
	{
		case 0xB5:
			SDK_GetInformation();
			break;
		case 0xA5:
			SDK_WhiteCardWriteCardHandler();
			break;
		case 0xA9:
			SDK_GetWriteCardCommandResult();
			break;

		default :
			ResponseFunction(0x6D,0x00);
			break;
	}

	sw1sw2 = (SW1<<8 | SW2);

	pushShort(sw1sw2);
}
/**
*modify by zhangxiaoxiao@20210728: 短信写卡+写卡冲突+场景/场景测试/仅支持两不一快写卡,打开01通道，在01通道选择白卡入网应用，期望6D00，实际9000，参考TPX5G函数中SDK_selectAID
*/
void WhiteCard_SetState(void)
{
	//if (G_FirstAPDU == 0)
	//{
	//	G_FirstAPDU = 1;
	//	CheckProtection();
	//}

	if(INS == 0xA4)
	{
		if((P1 != 0x04))
		{
			throw_error(0x6A86);
		}

//		if((U8_AND_U8(P2,0xF3) != 0x40)&&(U8_AND_U8(P2,0xF3) != 0x10)&&(U8_AND_U8(P2,0xF3) != 0x00))//白卡入网-APP写卡-空卡不预置KOPC/07parameter
//		{
//			throw_error(0x6A86);
//		}

		if (GetWriteCardSupport(B_BKXK) == FALSE)//短信写卡+写卡冲突+场景/场景测试/仅支持两不一快写卡,打开01通道，在01通道选择白卡入网应用，期望6D00，实际9000，参考TPX5G函数中SDK_selectAID
		{
			throw_error(0x6D00);
		}

		G_SDK_command_step = 0;
		G_AID_tag = selected_channel;//记录白卡写网应用在某个通道被选中 记录的是通道号
	}
}
BYTE code  WHITECARD_AID[] = {0xD1,0x56,0x00,0x01,0x01,0x81,0x01,0x7B,0x00,0x00,0x00,0x00,0xB0,0x01,0x2B,0x02};
BYTE IsWhiteCardApplet(BYTE* aidBuf)
{
	if(memCompare(MEMtPM8(aidBuf), MEMtPM8(WHITECARD_AID), sizeof(WHITECARD_AID)) == ECP_CMP_RESULT_EQUAL)
	{
		return TRUE;
	}
	return FALSE;
}
#endif

// 调整BYTE code GBAAppletAid[16]和BYTE IsGBAApplet(BYTE* aidBuf)函数的位置，解决因为typedef.h的被包含引起的simulator编译不过问题 by xuxiaojun@20211112
#if UICC__OPERATOR__CHINAMOBILE
BYTE code GBAAppletAid[] = {0xD1,0x56,0x00,0x01,0x01,0x81,0x01,0x7B,0x00,0x00,0x00,0x00,0xB0,0x01,0x2C,0x02};

BYTE IsGBAApplet(BYTE* aidBuf)
{
	if(memCompare(MEMtPM8(aidBuf), MEMtPM8(GBAAppletAid), sizeof(GBAAppletAid)) == ECP_CMP_RESULT_EQUAL)
	{
		return TRUE;
	}
	return FALSE;
}
#endif

/*@Function:    UICC_Toolkit_FileView_Init                                            
 * 
 *@Descrip:     3?ê??ˉToolkit_InitSIMView
 *
 *@See:	       áaí¨±￡????DT????oóμ?bug￡??′????oó￡???????è?ó|ó?ê±￡??á??μ±?°í¨μàSIM API?￡ê?μ?FILE VIEW D??￠??3y￡?′￥·￠ó|ó?ê±￡?
 *             SIM API?￡ê?￡?·??ê???t￡?ê1ó?μ±?°í¨μàFILE VIEWμ?D??￠ê±￡??ò2?μ?￡?μ???′￥·￠ó|ó?ê§°ü. uicc_configuration_additional_test_1¤3ì?ˉ\SECURED\GP213?\UICC_SIM_FileView_Test_04
 *                                                                                                                                     
 *@Param:       void
				                                                                                                         
 *@Return:      void                                                                                                                                                                                                                                                                    
 * 
 *@Author:      
 *  
 *@History:     V0.0---Create by yjr20210905         
 */
void UICC_Toolkit_FileView_Init(void)
{
	Toolkit_InitSIMView(GPGetCurrenteChannel(),0xFF);
}
/*@Function:    IsBufAllZeroOrFF                                            
 * 
 *@Descrip:     判断传入的buf是否是全零/FF/其他任意值
 * *                                                                                                                                     
 *@Param:       NULL
 *								                                                                                                         
 *@Return:      NULL                                                                                                                                                                                                                                                                   
 * 
 *@Author:      yjr 2021.12.13
 *  
 *@History:     V0.0---Create by yjr 2021.12.13
 *              V0.1--Modify by lgy @20220411
 */

BYTE IsBufAllZeroOrFF(BYTE * buf,BYTE hex,BYTE len)
{
	BYTE xdata i;
	
	for(i = 0; i < len;i++)
	{
		if(buf[i] != hex)
		{
			return FALSE;
		}
	}

	return TRUE;
}


/*@Function:    IF_WriteGBAKsFile                                            
 * 
 *@Descrip:     为了优化milenage的速度，将写文件的操作放在该指令后面
 * *                                                                                                                                     
 *@Param:       NULL
 *								                                                                                                         
 *@Return:      NULL                                                                                                                                                                                                                                                                   
 * 
 *@Author:      zhxx 2019.11.21
 *  
 *@History:     V0.0---Create by yjr 2021.12.13
 *                         
 */

 void IF_WriteGBAKsFile(void)
 {
	UWORD xdata fileAddr;

	if((INS != 0x88) && (INS != 0xC0))
	{
	     if(IsBufAllZeroOrFF(G_Auth_Rand,0,sizeof(G_Auth_Rand))==FALSE)
		 {
#if UICC__ISIM
			if(G_CurrentAdfName[G_ChannelNum] == G_DF_ID_ISIM)
			{
				fileAddr = SelectTFFile(G_DF_ID_ISIM, EF_GBABP); 
			}
			else	
#endif
			{	
				fileAddr = SelectTFFile(G_DF_ID_USIM, EF_GBABP_USIM);	
			}
			 
			if(fileAddr != FALSE)
			{
				WriteTFBuf(fileAddr, 0x00, &G_Auth_Rand[0], 17);			//更新6FD5中的RAND值
			}
			
			fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_AUTH_MODE);
			
			if(fileAddr != FALSE)
			{
				WriteTFBuf(fileAddr, 0, &G_Auth_Ks[0], sizeof(G_Auth_Ks));			//更新6FD5中的RAND值
			}
			
			 ecpMem_Set(G_Auth_Rand,0,sizeof(G_Auth_Rand));
			 ecpMem_Set(G_Auth_Ks,0,sizeof(G_Auth_Ks));
		 }
	} 
 }
 
 
 /*@Function:    IF_Process91xx                                            
 * 
 *@Descrip:     11.JavaCard事件处理机制检测/3-1-JavaCard事件处理机制检测/SIM API事件处理机制/事件处理机制关联性检测/SIM_N015-SIMN021，处理完C0之后，还要判断有没有主动式命令要发出来
 *               重入测试用例SIMReEntry_01_14_01,0070000001(6101) 00C0000001(91XX),有主动式命令要发，要求发送91XX                                                                                                                                     
 
 *@Param:       WORD status,BYTE INS,BYTE channel,BYTE isUICC
 *								                                                                                                         
 *@Return:      TRUE/FALSE
 * 
 *@Author:      zhxx 2022.11.16
 *  
 *@History:     V0.0---Create by zhxx 2022.11.16
 *                         
 */
WORD IF_Process91xx(WORD status,BYTE ins,BYTE channel,BYTE isUICC)
{
	#if UICC__OPERATOR__CHINAUNICOM
	//11.JavaCard事件处理机制检测/3-1-JavaCard事件处理机制检测/SIM API事件处理机制/事件处理机制关联性检测/SIM_N015-SIMN021，处理完C0之后，还要判断有没有主动式命令要发出来
	if((status == 0x9000)&&(ins == 0xC0)&&(channel==0)&&(isUICC == TRUE))
	{
		CommandFinish(0);
		IF_ProcessSystemProactiveCommand();
		status = SW1*0x100+SW2;
		return status;
	}
	else
	#endif
	{
		//重入测试用例SIMReEntry_01_14_01,0070000001(6101) 00C0000001(91XX),有主动式命令要发，要求发送91XX，by xwh 20200217
		if((status == 0x9000)&&(STKPerform[0] == TRUE)&&(channel==0)&&(isUICC == TRUE))
		{
			CommandFinish(0);
			status = SW1*0x100+SW2;
			return status;
		}	
	}
	
	return status;
}

 

BYTE IF_IsYCTAppletCommand(BYTE cla,BYTE ins)
{
	if(YANG_CHENG_TONG_CFG)
	{
		if(((cla == 0xC4) || (cla == 0x84)) && ((ins == 0xFE)||(ins == 0xFB)))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}
/*@Function:    IF_ProcessProactiveHandlerEvent                                            
 * 
 *@Descrip:     HTTPSessionStart中注册了123事件没有及时触发，在这里增加触发(只能在IF_AnnexB_StartHttpSessionByAPI中调用)
 * *                                                                                                                                     
 *@Param:       NULL
 *								                                                                                                         
 *@Return:      NULL                                                                                                                                                                                                                                                                   
 * 
 *@Author:      xwh 2023.01.04
 *  
 *@History:     V0.0---Create by xwh 2023.01.04
 *                         
 */

void IF_ProcessProactiveHandlerEvent(void)
{
	if((JcvmConfig_GetProcessProactive()&0x01)==0x01)
	{
		JavaToolkitProcessProactiveHandlerEvent();
	}
}
