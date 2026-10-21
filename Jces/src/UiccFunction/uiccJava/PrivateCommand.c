/************************************************************************************************************
 * Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
 * All rights reserved.
 *
 * File name:   PrivataCommand.c
 *
 * Description: 该文件管理私有指令
 *
 * auther：     xxxx    xxxx.xx.xx
 *
 * history：    V1.0
 *
 ************************************************************************************************************/

#include "typedef.h"
#include "T0ApduImpl.h"
#include "uicc_main.h"
#include "fileAction.h"
#include "ToolkitAPIPublic.h"
#include "gsmpublic.h"
#include "Uicc_error.h"
#include "Apdu.h"
#include "Filesystem.h"
#include "BootLifeCycle.h"
#include "PrivateCommand.h"
#include "Boot.h"
#include "profileApi.h"
#include "cardManager.h"
#include "ProfileInterpreter.h"
#include "euiccConfig.h"
#include "PrivateCommand.h"
#if UICC_FUCTION_PRIVATEAUTH
 /*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 VerifyMFKeyPrivate
  
  Command format:    C0    FC    00   00   08  MF KEY
  					  
  Data    content:    VerifyManagenerKey:	Current DF KEY(8)     
  					                                                                                                                                                                                                    
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:          校验 MF  下的  MF  KEY   					 
  
  status:			 Code OK!     Test OK!    

  修正执行了该操作之后，波特率变回到未下载文件系统之前的值：未下载文件系统时波特率为95，文件系统下载完成后将波特率修改为96，这个时候复位执行校验MFKEY操作成功后，波特率变回95. modify by xuxiaojun
  modify by chentao@EPX产业化\INFENION_SLM97\Develop
 -------------------------------------------------------------------------------------------------------------------*/
WORD ReloadVerifyManagenerKey(BYTE * apduBuffer)
{	
	BYTE idata limit,mode;
	BYTE * cData = &apduBuffer[APDU_DATA_OFFSET];
		
	if((apduBuffer[APDU_P1_OFFSET]!= 0x00)||((apduBuffer[APDU_P2_OFFSET]!= 0x00)&&(apduBuffer[APDU_P2_OFFSET]!= 0x01)))
	{
		GlobalSetStatus(0x6B00);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

    //mode = ((apduBuffer[APDU_INS_OFFSET] == PRIVATE_INS_VERIFYRELOADMFKEY) ? mode_MFKey : mode_ManagerKey);
	
	mode = mode_ManagerKey; //只有ManagerKey

    if(apduBuffer[APDU_P2_OFFSET]== 0x01)  //reload key
    {
    	mode |= mode_ReloadKey;
    }
	
    if(((mode&0x80)==mode_VerifyKey)&&(apduBuffer[APDU_P3_OFFSET] != 0x08))
	{
		GlobalSetStatus(0x6700);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2); 
	}
	
	if(((mode&0x80)==mode_ReloadKey)&&(apduBuffer[APDU_P3_OFFSET] != 0x10))
	{
		GlobalSetStatus(0x6700);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2); 
	}

	GlobalReceiveData();
	
    ReadEeprom((BYTE xdata *)&cData[0x20],E_MANAGERKEYADDR,0x09);

	if((cData[0x20]&0x0F) == 0x00)
	{
		GlobalSetStatus(0x9840);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2); 
	}
	
	if(ecpMem_Compare((BYTE xdata *)&cData[0],(BYTE  *)&cData[0x21],0x08) == EQUAL)
	{						
		if((mode&0x80)==mode_ReloadKey)
		{
			WriteEeprom((E_MANAGERKEYADDR+1),(BYTE *)&cData[0x08],0x08);
		}
		else	//mode_VerifyKey
		{
			SetBit(G_SystemStatus,B_cManagerKey);
			
			limit =0x0A;

			WriteEeprom(E_MANAGERKEYADDR,&limit,0x01);
		}
		
		GlobalSetStatus(0x9000);
	}
	else
	{		
		if((mode&0x80)==mode_VerifyKey)
		{
			ClearBit(G_SystemStatus,B_cManagerKey);
		}	
		limit = cData[0x20] - 1;
		
		WriteEeprom(E_MANAGERKEYADDR,(BYTE  *)&limit,sizeof(BYTE));
		  														
		SW1 = 0x63;
		SW2 = 0xC0 + (limit & 0x0F);
	}
	return (WORD)(((WORD)(SW1)<<8)|SW2); 
}

/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 UpdateAuthData

  Command format:    E0    D6    00   00   7D  AUTH DATA

  Data    content:    VerifyManagenerKey:	Current DF KEY(8)

  input parameter:   void

  output parameter:  void

  description:          校验 MF  下的  MF  KEY

  status:			 Code OK!     Test OK!

  修正执行了该操作之后，波特率变回到未下载文件系统之前的值：未下载文件系统时波特率为95，文件系统下载完成后将波特率修改为96，这个时候复位执行校验MFKEY操作成功后，波特率变回95. modify by xuxiaojun
 -------------------------------------------------------------------------------------------------------------------*/
WORD UpdateAuthData(BYTE * apduBuffer)
{
	BYTE * cData = &apduBuffer[APDU_DATA_OFFSET];
	BYTE idata index = ILLEGAL_PROFILEINDEX;
	UWORD xdata adfUSIMAddr,fileAddrOpc,fileAddrKi;
	WORD usimFid;
	//BYTE xdata temp[6],buf[7];
	BYTE idata tag;

	if(((apduBuffer[APDU_P1_OFFSET]!= 0x00)&&(apduBuffer[APDU_P1_OFFSET]!= 0x01))||(apduBuffer[APDU_P2_OFFSET]!= 0x00))
	{
		GlobalSetStatus(0x6B00);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	if(((apduBuffer[APDU_P1_OFFSET]== 0x00)&&(apduBuffer[APDU_P3_OFFSET]!= 0x7E))||((apduBuffer[APDU_P1_OFFSET]== 0x01)&&(apduBuffer[APDU_P3_OFFSET]!= 0x00)))
	{
		GlobalSetStatus(0x6700);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	if(BootLifeCycle_GetLifeCycle()< LIFE_CYCLE_INIT)
	{
		GlobalSetStatus(0x6985);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	index = JcvmConfig_GetProfileEnableIndex();

	if(index == ILLEGAL_PROFILEINDEX)
	{
		GlobalSetStatus(0x6985);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}
#endif
	if(ValueBit(G_SystemStatus,B_cManagerKey) == FALSE)
	{
		GlobalSetStatus(0x6982);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	if(apduBuffer[APDU_P1_OFFSET]== 0x00)  //更新鉴权数据
	{
		ReadEeprom((BYTE *)&tag,E_TAG_AUTHDATA_UPDATE_ADDR+index,0x01);

		if(tag != PROFILE_STATE_AUTH_NOT_UPDATE)
		{
			GlobalSetStatus(0x6985);
			
			return (WORD)(((WORD)(SW1)<<8)|SW2);
		}

		GlobalReceiveData();
	}
	else                                  //关闭更新鉴权数据指令
	{
		SetEeprom(E_TAG_AUTHDATA_UPDATE_ADDR+index,PROFILE_STATE_AUTH_UPDATED,sizeof(BYTE));

		GlobalSetStatus(0x9000);

		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	usimFid = GetADFFID((BYTE *)&USIMPartial[0]);

	adfUSIMAddr = SelectLFFile(DF_MF,usimFid);

	if(adfUSIMAddr == FALSE)
	{					
		GlobalSetStatus(0x6A82);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	fileAddrKi = SelectFile(adfUSIMAddr,EF_KI);

	if(fileAddrKi == FALSE)
	{	
		GlobalSetStatus(0x6A82);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	//EF_KI是映射文件，说明该AKA PE 是mapping 到其他AKA 的，这种情况下不允许用私有APDU更新鉴权参数
	if((GetFileHeadByte(fileAddrKi,F_Status_Offset) & F_MAP_FILE) == F_MAP_FILE)
	{
		GlobalSetStatus(0x6985);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	fileAddrOpc = SelectFile(adfUSIMAddr, EF_PRIVATE_AUTHCFG);

	if(fileAddrOpc == FALSE)
	{	
		GlobalSetStatus(0x6A82);
		
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}

	//ecpMem_Set(temp,0x00,6);
	//ReadTFBuf(fileAddrOpc,103,&buf[0],7);   //Read L,delta

	//文件中的delta是FF,APDU 中的delta不是FF，不允许更新
	//文件中的L是全0,APDU 中的L不是全0，不允许更新
	//2020-07-03 经过讨论去掉以下限制
	//if(((buf[6]==0xFF)&&(cData[OFFSET_UPDATEAUTHDATA_DELTA]!=0xFF))||((ecpMem_Compare(&buf[0],temp,6)==EQUAL)&&(ecpMem_Compare((BYTE *)&cData[OFFSET_UPDATEAUTHDATA_L],temp,6)!=EQUAL)))
	//{
	//	SW1 = 0x69;																//send  6700
	//	SW2 = 0x85;
	//	return (WORD)(((WORD)(SW1)<<8)|SW2);
	//}

	VMWriteBuf((adfUSIMAddr + F_AlgID_Offset),(BYTE*)&cData[OFFSET_UPDATEAUTHDATA_ALGOID],sizeof(BYTE));

	WriteLFBuf(fileAddrKi,1,0,(BYTE *)&cData[OFFSET_UPDATEAUTHDATA_KI],16);

	WriteTFBuf(fileAddrOpc,1,(BYTE *)&cData[OFFSET_UPDATEAUTHDATA_OPC],16+80+5);                     //update Opc,R,C
	WriteTFBuf(fileAddrOpc,1+16+80+5+1,(BYTE *)&cData[OFFSET_UPDATEAUTHDATA_L],6+1+1);     //update L,delta,Reslen

	GlobalSetStatus(0x9000);

	return (WORD)(((WORD)(SW1)<<8)|SW2);
}

#endif

#if (EUICC_ONLY || SE_AND_EUICC)
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 UpdateOpcFile

  Command format:

  description:      UpdateOpcFile

  status:			 Code OK!     Test OK!
       
  history:                   V 1.1 modify  chentao@20240104  新增参数off

 -------------------------------------------------------------------------------------------------------------------*/

BYTE UpdateOpcFile(BYTE * dataBuf, WORD off,BYTE len)
{
	UWORD idata fileAddr;
	BYTE idata result;
	fileAddr = FALSE;

	fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_PRIVATE_AUTHCFG);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	result=WriteTFBuf(fileAddr,off,dataBuf,len);// modify  chentao@20240104 

	return result;
}


/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 updateSqnFile

  Command format:

  description:       updateSqnFile

  status:			 Code OK!     Test OK!

 -------------------------------------------------------------------------------------------------------------------*/
BYTE updateSqnFile(BYTE * dataBuf, BYTE len)
{
	UWORD idata fileAddr;
	BYTE idata i,recNum,result;

	fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_SQNCheck);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	recNum = GetLFRecNumLgth(fileAddr,FileNum);

	for(i = 1 ;i <= recNum;i++)
	{
		result=WriteLFBuf(fileAddr,i,0,dataBuf,len);

		if(result == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}




/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 updateAuthCounter

  Command format:

  description:       updateAuthCounter

  status:			 Code OK!     Test OK!

 -------------------------------------------------------------------------------------------------------------------*/
BYTE updateAuthCounter(BYTE * dataBuf, BYTE len)
{

	UWORD idata fileAddr;
	BYTE idata result;

	fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],EF_AuthMaxCounter);

	if(fileAddr == FALSE)
	{
		return FALSE;
	}

	result=WriteTFBuf(fileAddr,0x00,&dataBuf[0],len);

	return result;
}
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 UpdatePriveFile

  Command format:    C0    F9    00   P2   P3

  Data    content:   更新私有文件内容

  input parameter:   P2:代表更新文件类型

  status:			ZDM
 -------------------------------------------------------------------------------------------------------------------*/

void UpdatePriveFile(void)
{
	BYTE idata result;
	UWORD idata bakLoadAdfAddr;

	result = JcvmConfig_GetSEState();//VMReadByte(CARD_LIFE_CYCLE);

    if(result > CARD_OP_READY)   //锁卡后要校验MFKEY
    {
		//if (ValueBit(G_SystemStatus,B_cmk) == FALSE)
    	if(SECURITY_STATE_MFKEY_VERIFIED != g_securityState)
	    {
			SecuritySatisfied();
			return ;
		}
    }
	ReceiveData();

	result = FALSE;
	
	//更新G网数据
	if(P1 == 0x00) // modify by EPX产业化/STM_ST33G1M2/DEVELOP
	{
		switch (P2)
		{
			case 0x01: //UPDATE KI
			{
				if((P3!=0x08)&&(P3!=0x10)&&(P3!=0x20)) //TUAK 的长度可能是0x20
				{
					P3Error();													//Send   67 00
					return ;
				}
				result=setKI(NORMALMODE,&RecData[0], P3);

				break;
			}
			case 0x02: //UPDATEOPC
			{
				result=UpdateOpcFile(&RecData[0],0, P3); //modify  chentao@20240104 从第一个配置字节开始更新

				break;
			}
			case 0x82: //UPDATEOPC
			{
				result=UpdateOpcFile(&RecData[0], 1,P3);//modify  chentao@20240104 从第二个字节即OPC的值开始更新

				break;
			}
			case 0x03: //更新sqn文件
			{
				if(P3 != 6)
				{
					P3Error();													//Send   67 00
					return ;
				}
				result = updateSqnFile(&RecData[0], P3);
				break;
			}
			case 0x04: //
			{
				if(P3 != 4)
				{
					P3Error();													//Send   67 00
					return ;
				}
				result = updateAuthCounter(&RecData[0], P3);
				break;
			}
			default:
				result=TRUE;
				break;
		}
	}
	//更新C网数据
	else if(P1 == 0x01)//modify by EPX产业化/STM_ST33G1M2/DEVELOP
	{
		bakLoadAdfAddr = G_fileControlContext.LoadADFAddr;
		
		switch(P2)
		{			
			case 0x81:	//authenticationKey		
				if(P3 != 0x08) 
				{
					P3Error();													
					return ;
				}
				
				setKI(NORMALMODE,RecData, P3);
				result=TRUE;
				break;
			case 0x82:  //ssd OCTET STRING (SIZE (16)) OPTIONAL	
				if(P3 != 16)
				{
					P3Error();													
					return ;
				}
				
				result=TRUE;
				break;
			case 0x83: // hrpdAccessAuthenticationData OCTET STRING (SIZE (2..32)) OPTIONAL,
				if((P3 < 2) || (P3 > 32))
				{
					P3Error();													
					return ;
				}
								
				ecpMem_Move(RecData+2,RecData,P3);
				G_fileControlContext.LoadADFAddr = G_CurrentDfAddr[G_ChannelNum];
				setHRPD(RecData, P3);	
				result=TRUE;
				break;
			case 0x84: //simpleIPAuthenticationData OCTET STRING (SIZE (3..483)) OPTIONAL
			   
				if((P3 < 3) || (P3 > 254))
				{
					P3Error();													
					return ;
				}			
							
				ecpMem_Move(RecData+2,RecData,P3);
				G_fileControlContext.LoadADFAddr = G_CurrentDfAddr[G_ChannelNum];
				setCHAP(RecData, P3);
				result=TRUE;				
				break;
			case 0x85: //mobileIPAuthenticationData OCTET STRING (SIZE (12..957)) OPTIONAL 
			
				if((P3 < 12) || (P3 > 254))
				{
					P3Error();													
					return ;
				}

				ecpMem_Move(RecData+2,RecData,P3);
				G_fileControlContext.LoadADFAddr = G_CurrentDfAddr[G_ChannelNum];
				setMobileIpSs(RecData, P3);	
				result=TRUE;
				break;
			default:
				P1P2Error();
				return ;
		}
		
		G_fileControlContext.LoadADFAddr = bakLoadAdfAddr;
	}
	else
	{
	
	}
				
	if(result==FALSE)
	{
		MemoryError();
		return;
	}
	
	CommandFinish(0x00);
}

/**
*added by EPX产业化/STM_ST33G1M2/DEVELOP
*/
u16 getSessionkey(BYTE * apduBuf)
{
	if(EUICC__SAVE_SESSIONKEY)
	{
		memCopyToRam(MEMtPX8(&apduBuf[APDU_DATA_OFFSET]),MEMtPM8(EUICC_SessionKey), apduBuf[APDU_P3_OFFSET]);
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, apduBuf[APDU_P3_OFFSET]);
		return BOOT_NO_ERROR;
	}

	return BOOT_INS_NOT_SUPPORTED;
}

#endif
/*-------------------------------------------------------------------------------------------------------------------

  Function name :	 SetCOSVersionData

  Command format:    C0    F9    00   00/01   0D  (电信给的预置值) LV结构
									  0x01:配置联通的ATR 版本号。如果设置成联通的类型，则ATR发送固定的版本信息。该为联通的备案需求


  input parameter:   void

  output parameter:  void

  description:          电信要求的cos版本号配置信息写入/读取是 选择ISD-R之后 用公开的私有指令80E900XX00获取
  -------------------------------------------------------------------------------------------------------------------*/

WORD SetCOSVersionData(BYTE * apduBuffer)
{
	BYTE idata result;
	BYTE * cData = &apduBuffer[APDU_DATA_OFFSET];

	result = JcvmConfig_GetSEState();//VMReadByte(CARD_LIFE_CYCLE);

	 if(result > CARD_OP_READY)   //锁卡后要校验MFKEY bug 405
	{
		//if (ValueBit(G_SystemStatus,B_cmk) == FALSE)
		if(SECURITY_STATE_MFKEY_VERIFIED != g_securityState)
		{
			GlobalSetStatus(0x6982);																//send  6700
			return (WORD)(((WORD)(SW1)<<8)|SW2);
		}
	}

	if(apduBuffer[APDU_P2_OFFSET] == 0x00)
	{
		if(apduBuffer[APDU_P3_OFFSET] > (CARDCFG_COSVersionLen+1))
		{
			GlobalSetStatus(0x6700);
			
			return (WORD)(((WORD)(SW1)<<8)|SW2);

		}

		GlobalReceiveData();

		if(apduBuffer[APDU_P3_OFFSET] > CARDCFG_COSVersionLen)
		{
			GlobalSetStatus(0x6A80);
			return (WORD)(((WORD)(SW1)<<8)|SW2);
		}

		WriteEeprom(CARDCFG_COSVersion,(BYTE  *)&cData[0x00],apduBuffer[APDU_P3_OFFSET]); //写入LV		
	}
//	else if(apduBuffer[APDU_P2_OFFSET] == 0x03) //P2 == 0x03
//	{
//		if(apduBuffer[APDU_P3_OFFSET] != 0x01)
//		{
//			SW1 = 0x67;																//send  6700
//			SW2 = 0x00;
//			return (WORD)(((WORD)(SW1)<<8)|SW2);
//		}
//
//		GlobalReceiveData();
//
//		SetEeprom(CARDCFG_KeepChaining,cData[0x00],0x01); //设置值 0x11 则清空会话信息  其他值则保持会话信息
//	}
	else
	{

	}

	GlobalSetStatus(0x9000);
	
	return (WORD)(((WORD)(SW1)<<8)|SW2);

}

#if  EUICC__FUNCTION_RedTea_Mobile
/*-------------------------------------------------------------------------------------------------------------------                                                                                       

  Function name :	 SetRedTeaRootAESKEY
  
  Command format:    E0    FB    00   00 xx(LV结构)
									
  input parameter:   void                                                                
  					                                                                                                         
  output parameter:  void                                                                                                           
                                                                                                                                                
  description:  红茶移动写根密钥，LV结构 	
  History:  created by LCZ
   -------------------------------------------------------------------------------------------------------------------*/

WORD SetRedTeaRootAESKEY(BYTE * apduBuffer)
{
	BYTE * cData = &apduBuffer[APDU_DATA_OFFSET];
	
	if(!RedTea_Mobile)
	{
		GlobalSetStatus(0x6A80);
		return (WORD)(((WORD)(SW1)<<8)|SW2);
	}
	
	if(g_securityState != SECURITY_STATE_MFKEY_VERIFIED)//校验MFkey
	{
			GlobalSetStatus(0x6985);
			return (WORD)(((WORD)(SW1)<<8)|SW2);			
	}

	if(apduBuffer[APDU_P3_OFFSET] > 0x21)//最大长度0x21
	{
			GlobalSetStatus(0x6700);																//send  6700
			return (WORD)(((WORD)(SW1)<<8)|SW2);		
	}
	
	GlobalReceiveData();

	WriteEeprom(CARDCFG_RedTeaAesKey,(BYTE  *)&cData[0x00],apduBuffer[APDU_P3_OFFSET]); //写入LV		
	
	GlobalSetStatus(0x9000);

	return (WORD)(((WORD)(SW1)<<8)|SW2);
	
}
#endif
