/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC RAM相关的一些操作，比如ISD R RAM操作或ISD P RAM操作等
*
*@history     :V1.0 :create @ 20181106  
*/

#include "EUICCRAM.h"
#include "EUICCM2MAPDU.h"
#include "interpreter.h"
#include "SCP03Realize.h"
#include "EUICCCustomer.h"
#include "EUICCMain.h"
#include "EUICCProfile.h"
#include "cardManager.h"
#include "ToolkitRFM.h"
#include "ToolkitAPIPublic.h"
#include "T0ApduImpl.h"
#include "nativemethod_impl.h"
#include "profileApi.h"

#if (EUICC_ONLY||SE_AND_EUICC)

BYTE xdata G_StoreDataReservedProfileIndex;  //登记保留的ISD-P Index
//WORD xdata G_RamResponseWordLen;              //保存执行RAM时响应数据超过255时的响应长度
/*
*@function   :GPInstallForPersonnalization
*
*@description:处理GP中的GPInstallForPersonnalization,参考GP221  6.7 、7.3.3 章节，及Mapping_guildline规范
*
*@param      :void
*
*@return     :void
*
*@author     :xuyz
*
*@history    :v1.0:create @20141017 by xuyz
*            :v1.1:modify @20190322 by xuxiaojun <TY_Offline_RSP_ManageCmdTest\UpdateMetadata_TSE_12_13_14_15>install for personalization指令传入的AID为当前激活的profile的ISD-P的AID，期望可以成功
*            :v1.1:modify @20190509 by xuxiaojun 避免MNOSD以外的SSD来更新ISD-P参数
*/
BYTE code ReservedISDPAID[] = {0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x0F,0x00};

BYTE  EUICCPreInstallForPersonnalization(void)
{
	BYTE xdata isdPAid[0x11];
	
	G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;
	
	if(FALSE == IF_CurrentTarSDIsMNOSD())
	{
		return FALSE;
	}
	
	if((P1&0x7F) != P1_INSTALLFORPERSONALIZATION)
	{
		return FALSE;
	}
	
	if((RecData[0] != 0x00)||(RecData[1] != 0x00))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(P3 != (RecData[2] + 0x06)) 
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(RecData[2] == 0x10 )  //有可能是  ReservedISDPAID， 也可能是 当前激活的profile的ISD-P的AID
	{	
		if((RecData[P3 - 1] != 0x00)||(RecData[P3 -2] != 0x00)||(RecData[P3 -3] != 0x00))  //该指令的后面3字节不为0
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		isdPAid[0] = GetProfileAIDByProfileIndex(G_CurEnabledProfileIndex, &isdPAid[1]); // 如果G_CurEnabledProfileIndex为FF时，返回值为0
		
		//modify by xuxiaojun@20230307: 将长度相等的判断提到前面，避免出现isdPAid[0]为0时还会进行无效的memcmp操作
		if(((RecData[2] == isdPAid[0])&&(ecpMem_Compare(&RecData[3],&isdPAid[1],RecData[2]) == EQUAL)) || ((RecData[2] == sizeof(ReservedISDPAID))&&(ecpMem_Compare(&RecData[3],&ReservedISDPAID[0],RecData[2]) == EQUAL)))
		{
			if(IF_ProfileLoadingOrChangeOngoing() == TRUE)//当前开启了下载profile会话 ，下载profile过程不允许插入当前激活profile下的卡片管理指令 (COS内部方案，无规范来源)
			{
				ThrowError(SW_CONDITIONS_NOT_SATISFIED);

				return TRUE;
			}
			
			if(IF_CapLoaingInCurrnetProfile() == TRUE)
			{
				ThrowError(SW_CONDITIONS_NOT_SATISFIED);

				return TRUE;
			}

			if(IF_MoreCommandsInCurrnetProfile() == TRUE)
			{
				ThrowError(SW_CONDITIONS_NOT_SATISFIED);

				return TRUE;
			}

			G_StoreDataReservedProfileIndex = G_CurEnabledProfileIndex;
			
			RecData[0] = 0x00;

			SetRemoteProcessRes(0x05, 1);

		    ThrowError(0x6101); 
			
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
/*
*@function   : EUICC_PreRAMCommand
*
*@description: 在EUICC 过程中,需要提前处理的RAM 命令 ,主要处理通过MNOSD的TAR对ISDP进行个人化操作（ES6接口）
*
*@param      :void
*
*@return     :TRUE: 命令已经被提前处理/FALSE: 命令没有被提前处理
*
*@author     :xuyz
*
*@history    :
*/
BYTE EUICC_PreRAMCommand(void)
{		
	switch(INS)
	{
		case 0xE6: //install for personalisation
			return EUICCPreInstallForPersonnalization();
	
		case 0xE2:
			if(ProfileIndexIsValid(G_StoreDataReservedProfileIndex))
			//if(G_StoreDataReservedProfileIndex != ILLEGAL_PROFILEINDEX)  //之前执行了INSTALL FOR PERSONALTION的命令，且是为 ISD-P 执行的，则直接处理即可（以下指令是给ISDP的）
			{
				if(ADDR_IS_NULL == GetProfileEntryAddr(G_StoreDataReservedProfileIndex)) //modify by xxj@20230330 可能在install for personal和该指令之间插入了euiccMemoryReset导致profile被删掉
				{
					G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;

					ThrowError(SW_CONDITIONS_NOT_SATISFIED);
				}

				if((P1&0x80) == 0x80)
				{
					G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;
				}
				
#if RSP_EUICC
				CUSTOMER_UpdateMetadata();	 // 在CUSTOMER_UpdateMetadata函数内部判断TAG是否属于updateMetadata指令
#elif M2M_EUICC
				if(M2M_ISDPStoreData() == FALSE)//之前执行了INSTALL FOR PERSONALTION的命令，且是为 ISD-P 执行的，则直接处理
				{
					ThrowError(SW_WRONG_DATA);
				}
#endif			
				return TRUE;
			}
			else
			{
#if M2M_EUICC
				if((G_TARRecIndex[0] != TarType_ISDP)&&(G_TARRecIndex[0] != TarType_ISDR))//其他普通SD的tar 不支持EUICC指令
				{
					if(((P1 & 0x08) == 0x08)&&(RecData[0] == 0x3A))//storeData 指令的DGI 3AXX均为M2M相关指令
					{
						ThrowError(SW_WRONG_DATA);
					}
				}
#endif
				return FALSE;//继续分发
			}
		
		default:

			return FALSE;//未被拦截的指令 分发给实际的SD处理
	}
}
/*
*@function   : NM_IsRAMProcessed
*
*@description:判断当前命令是否在该本地方法中执行 内部只能处理通过MNOSD的TAR对ISDP进行个人化操作，其他情况反FALSE 继续后续处理
*
*@param      :void
*
*@return     :TRUE: RAM APDU 在该函数中执行 /FALSE: 退出该函数，进行后续的处理
*
*@author     :xuyz
*
*@history     :V1.0 :create @20190304
*/
void NM_IsRAMProcessed(void)
{
	BYTE idata result;
	
	result = EUICC_PreRAMCommand();
		
	PushOperandStackShort(result);
}

#endif
