
/*	
#include "resume.h"
#include "Stkhead.h"
#include "Authentication.h"
#include "Interface.h"
#include "JcreMain.h"
#include "Transaction.h"
#include "Gsmpublic.h"
#include "ToolkitApiPublic.h"
*/

#include <string.h>
#include <uicc_error.h>
//#include "GPApplet.h"
//#include "GPSecurityDomain.h"
#include "uicc_main.h"
//#include  "Main.h"
#include "SuspendUicc.h"
#include "FileAction.h"
#include "FileSystem.h"
#include "Gsmpublic.h"
#include "Authentication.h"
//#include "BIPModuleProcess.h"
//#include "ChannelManagement.h"
#include "object.h"
#include "opFunction.h"

//SUSPENDSTATUS xdata UiccStatus[4];

#if UICC__AUTH__5G__SUSPEND
#define TOKEN_LEN  9


/*
*@description: SUSPENDUICC
*
*@param      :80 76 00/01 00 LC
							P1:00 SUSPEND UICC
							   01 RESUME UICC
*
*@returns   :void
*
*@history   :create by ZDM@2019/07/08
*/
void SuspendUicc(void)
{
	UWORD fileAddr;
	BYTE token[9],tmp,i,privi,appId;
	UWORD xdata addr;
	
	if((P1!=0x00)&&(P1!=0x01))
	{
		P1P2Error();
		return;
	}
	if(P2!=0x00)
	{
		P1P2Error();
		return;
	}
	if((P1==0x00)&&(P3!=0x04)) //suspend uicc
	{
			P3Error();
			return;
	}
	if((P1==0x01)&&(P3!=0x08)) //resume uicc,相关状态丢失
	{
			SetEepromEX(JcvmConfig_GetE_SUSPEND_TOKENBase()	,0x00,SUSPENDUICC_TAGLEN); //清空所有TAG和状态
			P3Error();
			return;
	}

	ReceiveData();
	
	fileAddr = SelectTFFile(DF_MF, EF_UMPC);
	
	if(fileAddr==FALSE)
	{
		ConditionSatisfied();
		return;
	}
	
	ReadTFBuf(fileAddr,0x02,&tmp,0x01);//Bit 2: '0': UICC does not support the UICC suspension procedure
																				  //'1': UICC supports the UICC suspension procedure.	
	if((tmp&0x02)==0x00)
	{
		ConditionSatisfied();
		return;
	}
	if(P1==0x00) //suspend uicc
	{
		if((STKPerform[0]!=0x00)) //有新打开的逻辑通道或者在STK中
		{
			ConditionSatisfied();
			return;
		}
		
		if (G_BIPModuleStatus.channelNum!=0x00) //如果有BIP通道打开，不能执行SUSPEND UICC
		{
			ConditionSatisfied();
			return;
		}
		
		for(i=1;i<VM_MAX_LOGICAL_CHANNELS_CT;i++) //如果有安全通道，则也不能执行SUSPEND
		{
#if (EUICC_ONLY||SE_AND_EUICC)
			if(getChannelProfileIndex(i) == G_CurExecutingProfileIndex)
#endif
			{
				if(getChannelState(i) == CHANNEL_OPEN)
				{
					appId = getChannelContext(i) & 0xFF;

					sdEntryAddr = getSdEntryObj(appId);
					objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));
					if (objRef != MEM_NULL)
					{
						privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));
					}
					else
					{
						continue;
					}
					privi = PX_R_U8(privArrayAddr);

					 if((privi & 0x80) == 0x80)  //指定的app是SD
					 {
	//						addr = GetSDSessionInfoBySdID(appId);
	//
							//if(((SDSessionInfo*)addr)->SessionCommandTag == ExternalAuth_Excuted)
							if((channelInfo.compulsory_Session_Security_Level[i] & SCF_AUTHENTICATED) == SCF_AUTHENTICATED)
							{
								ConditionSatisfied();
								return;
							}
					 }
				}
			}
		}
		//Random_number_generator(&token[1],0x08);
		Random_Number_Generator_Get(&token[1],0x08);
		
		token[0]=0x01;
		
		WriteEepromEX(JcvmConfig_GetE_SUSPEND_TOKENBase()	,&token[0],TOKEN_LEN);
		
		G_SuspendUiccTag=TRUE;
		
		ecpMem_Copy(&RecData[2],&token[1],8);
		
		for(i=0;i<4;i++)
		{
		
			tempBuf[i*OFF_LEN+CurrentDfAddr_OFF]=(G_CurrentDfAddr[i]&0xFF000000)>>24;
			tempBuf[i*OFF_LEN+CurrentDfAddr_OFF+1]=(G_CurrentDfAddr[i]&0x00FF0000)>>16;
			tempBuf[i*OFF_LEN+CurrentDfAddr_OFF+2]=(G_CurrentDfAddr[i]&0x0000FF00)>>8;
			tempBuf[i*OFF_LEN+CurrentDfAddr_OFF+3]=(G_CurrentDfAddr[i]&0x000000FF);
			
			tempBuf[i*OFF_LEN+CurrentEfAddr_OFF]=(G_CurrentEfAddr[i]&0xFF000000)>>24;
			tempBuf[i*OFF_LEN+CurrentEfAddr_OFF+1]=(G_CurrentEfAddr[i]&0x00FF0000)>>16;
			tempBuf[i*OFF_LEN+CurrentEfAddr_OFF+2]=(G_CurrentEfAddr[i]&0x0000FF00)>>8;
			tempBuf[i*OFF_LEN+CurrentEfAddr_OFF+3]=(G_CurrentEfAddr[i]&0x000000FF);
			
			tempBuf[i*OFF_LEN+CurrentAdfName_OFF]=(G_CurrentAdfName[i]&0xFF000000)>>24;
			tempBuf[i*OFF_LEN+CurrentAdfName_OFF+1]=(G_CurrentAdfName[i]&0x00FF0000)>>16;
			tempBuf[i*OFF_LEN+CurrentAdfName_OFF+2]=(G_CurrentAdfName[i]&0x0000FF00)>>8;
			tempBuf[i*OFF_LEN+CurrentAdfName_OFF+3]=(G_CurrentAdfName[i]&0x000000FF);
			
			tempBuf[i*OFF_LEN+CurrentDfName_OFF]=(G_CurrentDfName[i]&0xFF00)>>8;
			tempBuf[i*OFF_LEN+CurrentDfName_OFF+1]=(G_CurrentDfName[i]&0x00FF);
			
			tempBuf[i*OFF_LEN+CurrentEfName_OFF]=(G_CurrentEfName[i]&0xFF00)>>8;
			tempBuf[i*OFF_LEN+CurrentEfName_OFF+1]=(G_CurrentEfName[i]&0x00FF);				

			tempBuf[i*OFF_LEN+ChannelStatus_OFF]=ChannelManagement[i].status;
			
			tempBuf[i*OFF_LEN+appSelected_OFF]=ChannelManagement[i].appSelected;
			
			tempBuf[i*OFF_LEN+context_OFF]=ChannelManagement[i].context;
			
			tempBuf[i*OFF_LEN+pinStatus_OFF]=G_ChvStatus;			
			
		}
		
		WriteEepromEX(JcvmConfig_GetE_SUSPENDSTATUSBase(),tempBuf,OFF_LEN*4); // modify by xuxiaojun@20211009 如果VM_CONFIG_START不是0的话， 读取的E_SUSPENDSTATUS地址是不对的
		
		G_InvildCommandTag=FALSE; //如果不断电，再次执行SUSPENDUICC，重新计该TAG.
		
		CommandFinish(0x0A);
	}
	
	if(P1==0x01) //resume UICC
	{
		
		if(G_TerminalCap==FALSE)
		{
				ConditionSatisfied();
				return;
		}
		
		if(G_SuspendUiccTag==FALSE) //没有执行SUSPENDUICC指令或者在执行RESUME指令前，执行了其他非规定指令，卡片返回6985
		{
				ConditionSatisfied();
				return;
		}
		if(G_InvildCommandTag==TRUE)
		{
				SetEepromEX(JcvmConfig_GetE_SUSPEND_TOKENBase()	,0x00,SUSPENDUICC_TAGLEN); //清空所有TAG和状态
				ConditionSatisfied();
				return;
		}
		ReadEeprom(&token[0],JcvmConfig_GetE_SUSPEND_TOKENBase()	,TOKEN_LEN);
		
		if(token[0]!=0x01)
		{
			ConditionSatisfied();
			return;
		}
		
		if(ecpMem_Compare(&RecData[0],&token[1],8)!=EQUAL)
		{
			SetEepromEX(JcvmConfig_GetE_SUSPEND_TOKENBase()	,0x00,SUSPENDUICC_TAGLEN); //清空所有TAG和状态
			SecuritySatisfied();
			return;
		}
		
		ReadEeprom(tempBuf,JcvmConfig_GetE_SUSPENDSTATUSBase(),OFF_LEN*4); // modify by xuxiaojun@20211009 如果VM_CONFIG_START不是0的话， 读取的E_SUSPENDSTATUS地址是不对的
		
		for(i=0;i<4;i++)
		{

			G_CurrentDfAddr[i]=((((UWORD)(tempBuf[i*OFF_LEN+CurrentDfAddr_OFF]))<<24)&0xFF000000)+
										((((UWORD)(tempBuf[i*OFF_LEN+CurrentDfAddr_OFF+1]))<<16)&0x00FF0000)+
										(((WORD)tempBuf[i*OFF_LEN+CurrentDfAddr_OFF+2]<<8)&0x0000FF00)+tempBuf[i*OFF_LEN+CurrentDfAddr_OFF+3];
			
			G_CurrentEfAddr[i]=((((UWORD)(tempBuf[i*OFF_LEN+CurrentEfAddr_OFF]))<<24)&0xFF000000)+
										((((UWORD)(tempBuf[i*OFF_LEN+CurrentEfAddr_OFF+1]))<<16)&0x00FF0000)+
										(((WORD)tempBuf[i*OFF_LEN+CurrentEfAddr_OFF+2]<<8)&0x0000FF00)+tempBuf[i*OFF_LEN+CurrentEfAddr_OFF+3];
			
			G_CurrentAdfName[i]=((((UWORD)(tempBuf[i*OFF_LEN+CurrentAdfName_OFF]))<<24)&0xFF000000)+
										((((UWORD)(tempBuf[i*OFF_LEN+CurrentAdfName_OFF+1]))<<16)&0x00FF0000)+
										(((WORD)tempBuf[i*OFF_LEN+CurrentAdfName_OFF+2]<<8)&0x0000FF00)+tempBuf[i*OFF_LEN+CurrentAdfName_OFF+3];;
			G_CurrentDfName[i]=(((WORD)tempBuf[i*OFF_LEN+CurrentDfName_OFF]<<8)&0x0000FF00)+tempBuf[i*OFF_LEN+CurrentDfName_OFF+1];
			G_CurrentEfName[i]=(((WORD)tempBuf[i*OFF_LEN+CurrentEfName_OFF]<<8)&0x0000FF00)+tempBuf[i*OFF_LEN+CurrentEfName_OFF+1];
			G_CurrentRecNum[i]=tempBuf[i*OFF_LEN+CurrentRecNum_OFF];
			if(i!=0)
			{
				ChannelManagement[i].status=tempBuf[i*OFF_LEN+ChannelStatus_OFF];
			}
			else
			{
				ChannelManagement[i].status=CHANNEL_STATUS_OPEN; //0通道始终打开
			}
			ChannelManagement[i].appSelected=tempBuf[i*OFF_LEN+appSelected_OFF];
			ChannelManagement[i].context=tempBuf[i*OFF_LEN+context_OFF];			
			
		}
		G_ChvStatus=tempBuf[pinStatus_OFF];
		
		SetEepromEX(JcvmConfig_GetE_SUSPEND_TOKENBase()	,0x00,SUSPENDUICC_TAGLEN); //清空所有TAG和状态
		
		G_SuspendUiccTag=FALSE;
		
		CommandFinish(0x00);
	} 
}

#endif




