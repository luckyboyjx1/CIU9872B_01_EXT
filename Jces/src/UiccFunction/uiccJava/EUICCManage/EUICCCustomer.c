/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC 个人版的操作的一些流程，基于规范 <SGP22_V2.2. pdf> 
*
*@history     :V1.0 :create @ 20181106  
*/

#include "EUICCCustomer.h"
#include "EUICCCustomerPublic.h"
#include "EUICCProfile.h"
#include "EUICCMain.h"
#include "interpreter.h"
#include "ASN1Operation.h"
#include "EUICCInterface.h"
#include "EUICCCustomerCertificate.h"
#include "SCP03Realize.h"
#include "EUICCGPDataOpertion.h"
#include "ProfileInterpreter.h"
#include "ProfileGPCommand.h"
#include "T0ApduImpl.h"
#include "cardManager.h"
#include "SEEUICCInterface.h"
#include  "EUICCPersonalAPDU.h"
#include  "Stkhead.h"
#include  "EUICCInterface.h"
#include  "Crypto_Constant.h"
#include "platform_api.h"
#include "object.h"
#include  "Interface_eSEPrivateAPI.h"
#include "EUICCCustomerCertificate.h"
#include  "VMAccessEeprom.h"
#include "ob_manager.h"
#include "euiccConfig.h"
#include "crypto_impl.h"
#include "profileApi.h"
#include "EUICCRAM.h"

#if (EUICC_ONLY || SE_AND_EUICC)
#if RSP_EUICC
RSPSessionContext xdata G_RSPSessionContext;

CustomerStoreData xdata G_CustomerStoreDataApdu;
SendFileExtendBackData xdata G_SendFileExtendBackDataStruct;

/*
*@function     :CUSTOMER_SendSingleBYTEResult 
*
*@description  :发送RSP接口执行的响应结果
*
*@see          :
*
*@param        :WORD ResponseTag       响应结果对应的TAG
*@param        :BYTE result            响应结果
*
*@return       :void
*
*@RAM Resources:RecData
*
*@author       :xuxiaojun@20181116
*
*@hisotry      :created by xuxiaojun@20181116
*/
void CUSTOMER_SendSingleBYTEResult(WORD ResponseTag, BYTE result)
{
	SetWord((BYTE*)&RecData[0],ResponseTag);
	
	RecData[2] = 0x03;
		
	RecData[3] = 0x80;
	
	if((ResponseTag == TAG_LIST_NOTIFICATION) || (ResponseTag == TAG_RETRIEVE_NOTIFICATION_LIST))
	{
		RecData[3] = 0x81;
	}
	
	RecData[4] = 0x01;
	
	RecData[5] = result;   
	
	SetOutgoingAndSend(0x05,RecData[2] + 3);
}
/*
*@function     :CUSTOMER_GetEuiccConfiguredAddresses (80E2910003 BF3C00)
*
*@description  :This function retrieves the Root SM-DS and if configured the Default SM-DP+ address from  the eUICC. Both addresses are coded as FQDN. 
*
*@see          :<SGP22_V2.2 > 5.7.3  Function (ES10a): GetEuiccConfiguredAddresses
*
*@param        :void 
*
*@return       :void
*
*@test         :自测通过
*
*@RAM Resources:tempBuf[0..102]，仅在该函数内部有效
*
*@author       :liguiyu@20181109
*
*@hisotry      :V1.0 created by liguiyu@20181109
*              :V1.1 modify by xuxiaojun@20190704 修改函数可以发送的字节数不受RecData的限制
*/
void CUSTOMER_GetEuiccConfiguredAddresses(void)
{	
	UWORD xdata addr;
	WORD idata totalLen, offset;
	BYTE idata i,dataLen;	
	
	if((RecData[2] != 0x00) || (P3 != 0x03)) // BF3C00
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	addr = GetAddrByIndexAddr(EUICCRootSMDSAddress_Ref_Addr);
	
	if(addr == ADDR_IS_NULL)//必须存在RootSMDSAddr
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	totalLen = 0;
	
	offset = 0x05; // EUICCTempDataBuf的偏移
	
	for(i=0 ; i< 2 ;i ++)
	{
		addr = GetAddrByIndexAddr(EUICCSMDPDefaultAddress_Ref_Addr + i*sizeof(WORD));
		
		if(addr == ADDR_IS_NULL)  //DP地址可能不存在, 最大值为Default_DP_Address_MAX_LEN
		{
			continue;
		}
		
		addr = get_array_data(MEMtPF8(addr));

		dataLen = VMReadByte(addr);  //获取地址的实际长度
		
		if(dataLen != 0x00)  //DS地址一定不为0，在卡片初始化时设置
		{
			totalLen += ASN1_PaddingBERTlvTagAndLen(&tempBuf[totalLen],(0x80 | i),dataLen);
			
			VMReadBuf(&tempBuf[totalLen],addr + 1,dataLen); // 由于个人化时只支持单条指令，所以DP和DS的最大长度都只能是0xFF
			
			totalLen += dataLen;

			VMWriteBuf(EUICCTempDataBuf + offset, tempBuf, totalLen); //两次一起缓存在tempBuf中的话，可能会越界: 8081FF V + 8181FF V一共0x204字节. 不会导致EUICCTempDataBuf越界
						
			offset += totalLen;
			
			totalLen = 0;
		}
	}
	
	totalLen = offset - 5;
	
	i = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0], 0xBF3C, totalLen);
	
	VMWriteBuf(EUICCTempDataBuf + 5 - i, tempBuf, i);
	
	SetSendFileExtendInfo(EUICCTempDataBuf + 5 - i, i + totalLen);		
}

/*
*@function     :CUSTOMER_SetDefaultDpAddress 
*
*@description  :This function is used to update the default SM-DP+ address.  
*
*@see          :<SGP22_V2.2 > 5.7.4  Function (ES10a): SetDefaultDpAddress. 参数格式BF3F L 80 LV
*
*@param        :void
*
*@return       :void
*
*@test         :自测通过
*
*@RAM Resources:tempBuf，数据只在该函数内部有效
*
*@author       :by liguiyu@20181109 
*
*@hisotry      :V1.0 created by liguiyu@20181109
*@hisotry      :V1.1 modify by xuxiaojun@20230314: 由于defalut DP地址的最大能力为Default_DP_Address_MAX_LEN字节，为了减少空间分配次数，在ISDR初始化时按照最大长度只创建一次即可
*/
void CUSTOMER_SetDefaultDpAddress(void)
{
	BYTE idata offset,result,len1,len2;
	UWORD xdata tmpLen1,tmpLen2,dataAddr;

	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)  // 支持级联
	{
		dataAddr = (UWORD)RecData;
	}
	else
	{
		dataAddr = EUICCTempDataBuf;
	}
	
	if(VMReadByte(dataAddr + 2) == 0x00) // BF3F00
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	len1 = ASN1_GetBERTlvLen(dataAddr+2,(UWORD*)&tmpLen1);	
	
	if((tmpLen1 + len1 + 2) != G_CustomerStoreDataApdu.storeDataLen)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	offset = 2 + len1; // 偏移到第1个子TAG
	
	if(VMReadByte(dataAddr + offset) != 0x80)  // 子TAG不正确
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_DEFAULT_DPADDR, RESULT_UNDEFINED_ERROR);	
		
		return;
	}
		
	len2 = ASN1_GetBERTlvLen((dataAddr + offset + 1),(UWORD*)&tmpLen2); // 获取TAG 80的长度
	
	if((tmpLen2 > Default_DP_Address_MAX_LEN) || (tmpLen1 != (tmpLen2 + len2 + 1)))  //包含了多个tag, <RSP_EUICC测试_管理指令测试_管理指令异常测试\SetDefaultDpAddress_TSE02.html>
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_DEFAULT_DPADDR, RESULT_UNDEFINED_ERROR);	
		
		return;
	}
	
	result = SetDefaultDpAddress_MemoryReset((BYTE *)(dataAddr + offset + 1 + len2), tmpLen2);		
	
	CUSTOMER_SendSingleBYTEResult(TAG_SET_DEFAULT_DPADDR, result);	
}
/*
*@function    : GetEuiccInfo_ExtCardResource
*
*@description :拼装EUICCINfo2中的extCardResource 字段
*
*param        :BYTE * outBuf :输出数据缓存buf
*
*@return      :BYTE len
*
*@author      :
*
*@
*/
BYTE GetEuiccInfo_ExtCardResource(BYTE * outBuf)
{
	UWORD xdata cardResource;
	
	outBuf[0] = TAG_ExtCardResource; //extCardResource 84 xx 810100 8204 xx 8302 xx 共0x0D个字节
						
	outBuf[1] = 0x0D;
			
	outBuf[2] = 0x81;
	
	outBuf[3] = 0x01;
	
	outBuf[4] = 0x00;	//The "number of installed application" value field of extCardResource SHALL be set to '00'.
	
	cardResource = GPGetCardRemainMemory(GPFlash_Mode); //Free non volatile memory
	
	outBuf[5] = 0x82;
	
	outBuf[6] = 0x04;
	
	outBuf[7] = (cardResource>> 24)&0xFF;
	
	outBuf[8] = (cardResource>> 16)&0xFF;
	
	outBuf[9] = (cardResource>> 8)&0xFF;
	
	outBuf[10] = (cardResource)&0xFF;

	cardResource = GPGetCardRemainMemory(GPRam_Mode); //Free volatile memory
	
	outBuf[11] = 0x83;
	
	outBuf[12] = 0x02;
	
	outBuf[13] = (cardResource>> 8)&0xFF;
	
	outBuf[14] = (cardResource)&0xFF;
	
	return 0x0F;
}
/*
*@function     :CUSTOMER_GetEUICCInfo 
*
*@description  :This function gets the eUICC Information as defined in section ?0. This function MAY be called at any time
*
*@see          : <SGP22_V2.2 > 5.7.8  Function (ES10b): GetEUICCInfo
*              	
				BF20 XX  value  ------------------EUICCInfo1 ::= [32] SEQUENCE
					   82 XX  value --------------svn [2] VersionType
					   A9 XX  value --------------euiccCiPKIdListForVerification [9] SEQUENCE OF SubjectKeyIdentifier
							  04 XX  value--------SubjectKeyIdentifier(KeyIdentifier ::= OCTET STRING)
							  04 XX  value--------SubjectKeyIdentifier(KeyIdentifier ::= OCTET STRING)
							  ……
					   AA XX  value --------------euiccCiPKIdListForSigning [10] SEQUENCE OF SubjectKeyIdentifier -- List of CI Public Key Identifier supported on the eUICC for signature creation
							  04 XX  value -------SubjectKeyIdentifier(KeyIdentifier ::= OCTET STRING)
							  04 XX  value -------SubjectKeyIdentifier(KeyIdentifier ::= OCTET STRING)	
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:tempBuf 占用的最大长度为euiccInfo_section3的长度
*
*@hisotry      :V1.0 created by liguiyu@20190114
*              :V1.1 modify by xuxiaojun@20190614 代码优化，去掉局部变量euiccInfo_section1
*/
void CUSTOMER_GetEUICCInfo(void)
{
	BYTE idata octerNum;
	WORD idata outTag;
	WORD idata dataLen,PKIDForVerificationLen,PKIDForSigningLen;
	UWORD xdata addr;
	
	if(RecData[2] != 0)// 输入数据为BF20 00 /BF22 00
	{
		ThrowError(SW_WRONG_DATA);
	}
  
	outTag = (((WORD)RecData[0] << 8)&0xFF00)|RecData[1];
	
	if(outTag == 0xBF20)	//BF20  tag ,	retrieve EUICCInfo1
	{			
		addr = EUICCINFO2_SECTION1;
		
		if(addr == ADDR_IS_NULL)
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);  //没有执行个人化指令
		}

		VMReadBuf(tempBuf, addr+1, EUICCINFO2_SECTION1_LEN);//LV结构

//		if(tempBuf[0] != 0x81)
//		{
//			ThrowError(SW_CONDITIONS_NOT_SATISFIED);  //没有执行个人化指令
//		}
		
		dataLen = tempBuf[6] + 2;// 获取svn 所占用的结构体大小
		
		ecpMem_Move(&tempBuf[0], (BYTE *)&tempBuf[5],dataLen);
		
		WriteEepromNonAtomic(EUICCTempDataBuf + 5,(BYTE *)&tempBuf[0],dataLen);//从EUICCTempDataBuf + 5开始写svn [2] VersionType ，预留 BF 20 XX XX XX 5字节大tag和len
		
		//该函数内部越界的话会抛异常， 可以容纳的最大CI证书个数是0x19，否则就会越界. PKIDForVerificationLen的最大值为0x1F4
		PKIDForVerificationLen = GetEuiccCiPKIdList(&tempBuf[0], TEMP_BUF_SIZE, CIPKID_ForVerification);// A9 XX euiccCiPKIdListForVerification [9] SEQUENCE OF SubjectKeyIdentifier  ,for verification

		if((5 + dataLen + PKIDForVerificationLen) > EUICCTempDataBufSize) //modify by xuxiaojun@20230314 增加越界检查， PKIDForVerificationLen的最大值为0x19*0x14=0x1F4，所以也不会越界
		{
			ThrowError(Card_Ability_Exception);
		}

		WriteEepromNonAtomic(EUICCTempDataBuf + 5 + dataLen,&tempBuf[0],PKIDForVerificationLen);
		
		dataLen += PKIDForVerificationLen;
						
		PKIDForSigningLen = GetEuiccCiPKIdList(&tempBuf[0], TEMP_BUF_SIZE, CIPKID_ForSignature);// AA xx  euiccCiPKIdListForSigning [10] SEQUENCE OF SubjectKeyIdentifier   , for signature creation

		if((5 + dataLen + PKIDForSigningLen) > EUICCTempDataBufSize)//modify by xuxiaojun@20230314 增加越界检查， PKIDForSigningLen的最大值为0x19*0x14=0x1F4，所以也不会越界
		{
			ThrowError(Card_Ability_Exception);
		}

		WriteEepromNonAtomic(EUICCTempDataBuf + 5 + dataLen ,&tempBuf[0],PKIDForSigningLen);
		
		dataLen += PKIDForSigningLen;//总长度
		
		octerNum = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0],outTag,dataLen);
	
		VMWriteBuf(EUICCTempDataBuf + 5 - octerNum,&tempBuf[0],octerNum);
	
		SetSendFileExtendInfo(EUICCTempDataBuf + 5 - octerNum, dataLen + octerNum);//从EUICCTempDataBuf + 5 - octerNum处发响应数据  长度为dataLen + octerNum
	}
	else	// BF22 tag   ,  retrieve EUICCInfo2
	{
		dataLen = GetEuiccInfo2SubFunc(EUICCTempDataBuf, EUICCTempDataBufSize); //modify by xuxiaojun@20230314 增加越界检查，该函数内部会抛异常

		SetSendFileExtendInfo(EUICCTempDataBuf, dataLen);
	}
}
/*
*@function     :CUSTOMER_ListNotification 
*
*@description  :This function is used by the LPA to list all available pending notifications from an eUICC before retrieving a specific notification.
*
*@see          :<SGP22_V2.2 > 5.7.9  Function: (ES10b): ListNotification
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:tempBuf
*
*@author       :xuxiaojun@20181114
*
*@hisotry      :created by xuxiaojun@20181114
*              :modify by xuxiaojun@20230329 根据《代码评审问题跟踪表  - 20230327》由于EPX平台的VMWriteBuf支持E2到E2的写操作，所以去掉对tempBuf的借用及越界判断
*/
void CUSTOMER_ListNotification(void)
{
	UWORD xdata notifyAddr,notifyDataAddr;
	WORD idata notifyRef, euiccDataBufOffset, notifyLen;
	BYTE idata profileManagementOperation, lengthOfLen, notificationType;
	ASN1Info xdata tempASN1Info;
	
	if(((RecData[2]+3) != P3) || (RecData[2] > 0x04))   //BF2B L  确保数据域只有BF28 tag，避免出现类似CLA IN P1 P2 P3 BF2800 BF2200的情况
	{
		CUSTOMER_SendSingleBYTEResult(TAG_LIST_NOTIFICATION, RESULT_UNDEFINED_ERROR);
		
		return;
	}
	
	if(RecData[2] == 0x00) // BF2800  If profileManagementOperation data object is omitted, the eUICC SHALL return all stored notifications whatever their type.
	{
		RecData[6] = 0xF0; 
	}
	else if((RecData[3] != 0x81) || ((RecData[4]+2) != RecData[2]))
	{
		CUSTOMER_SendSingleBYTEResult(TAG_LIST_NOTIFICATION, RESULT_UNDEFINED_ERROR);
		
		return;
	}
	else if((RecData[4] == 0x00) || ((RecData[4] == 0x01) && (RecData[5] == 0x00))) // BF28 02 8100 或 BF28 03 810100
	{
		SetWord((BYTE*)&RecData[0],TAG_LIST_NOTIFICATION);    // 测试用例 < TC_eUICC_ES10b_ListNotification_10_Nominal_No_Notifications_available> 要求在这种情况下返回空列表
		
		RecData[2] = 0x02;
		
		RecData[3] = 0xA0;
		
		RecData[4] = 0x00;
		
		SetOutgoingAndSend(0x05,RecData[2] + 3);
		
		return;
	}
	if(RecData[6] == 0x00) // If profileManagementOperation data object indicates no event (all bits set to 0), the eUICC SHALL return an empty list or undefinedError(127)
	{
		CUSTOMER_SendSingleBYTEResult(TAG_LIST_NOTIFICATION, RESULT_UNDEFINED_ERROR);
		
		return;
	}
	
	profileManagementOperation = 0x00;
	
	notifyRef = REF_NULL;

	euiccDataBufOffset = 0x09;               //预留9个字节出来
	
	notifyLen = 0;
	
	notifyAddr = GetAddrAndIndexByIndexAddr(EUICCNotificationsRef_Addr,&notifyRef);      //获取到通知列表中第一个通知的引用以及第一个通知数据存储的地址
			
	while(notifyRef != REF_NULL)
	{
		profileManagementOperation = VMReadByte(GetNotificationList_profileManagementOperationAddr(notifyAddr));//VMReadByte(notifyAddr + NotificationList_profileManagementOperation_Offset);
		
		notificationType = VMReadByte(GetNotificationList_notificationTypeAddr(notifyAddr));//VMReadByte(notifyAddr + NotificationList_notificationType_Offset);
		
		notifyLen = memReadU16(MEMtPM8(GetNotificationList_notificationDataSizeAddr(notifyAddr)));//VMReadBuf((BYTE*)&notifyLen,notifyAddr + NotificationList_notificationDataSize_Offset,2);
		
		notifyDataAddr = GetNotificationList_notificationData_ArrayAddr(notifyAddr);

		ASN1_InitTLVInfo((ASN1Info*)&tempASN1Info,notifyDataAddr, notifyLen);
				
		if(((RecData[6]&profileManagementOperation) == profileManagementOperation)&&(notifyLen != 0x00))        // 找到对应类型的通知，拼装BF2F数据，从Notification中查找
		{
			if((profileManagementOperation == NOTIFICATION_EVENT_Install) && (notificationType == NOTIFICATION_TYPE_FINALRESULT))// 从BF37 --- BF27 --- BF2F找到BF2F在起始在通知中的起始偏移
			{					
				ASN1_FindTLV((ASN1Info*)&tempASN1Info, 0x37, FALSE, 0x01);   //BF 37
				
				ASN1_SetNextDepthToFindTLV((ASN1Info*)&tempASN1Info);
				
				ASN1_FindTLV((ASN1Info*)&tempASN1Info, 0x27, FALSE, 0x01);   //BF 27
				
				ASN1_SetNextDepthToFindTLV((ASN1Info*)&tempASN1Info);
				
				ASN1_FindTLV((ASN1Info*)&tempASN1Info, 0x2F, FALSE, 0x01);   //BF 2F					
			}
			else                                                            //30 --- BF2F找到BF2F的起始在通知中的起始偏移
			{
				ASN1_FindTLV((ASN1Info*)&tempASN1Info, 0x30, FALSE, 0x01);   //0x30
				
				ASN1_SetNextDepthToFindTLV((ASN1Info*)&tempASN1Info);
				
				ASN1_FindTLV((ASN1Info*)&tempASN1Info, 0x2F, FALSE, 0x01);   //BF 2F			
			}
			
			notifyLen = tempASN1Info.curTLVLen; //tempASN1Info.curTLVLen = tempASN1Info.curTagLen + tempASN1Info.curLenLen + tempASN1Info.curValueLen;

			if((euiccDataBufOffset + notifyLen) > EUICCTempDataBufSize)
			{
				break; //modify by xuxiaojun@20230314: 可能会导致EUICCTempDataBuf越界,如果越界则返回前面已经写入的数据，后续的通知数据舍弃
			}

			VMWriteBuf(EUICCTempDataBuf + euiccDataBufOffset, (BYTE *)(tempASN1Info.bufferAddr + tempASN1Info.curTagOff), notifyLen);               //将单条BF2F写入EUICCTempDataBuf

			euiccDataBufOffset += notifyLen;
		}
		
		notifyAddr = GetAddrAndIndexByIndexAddr(GetNotificationList_nextNotificationRefAddr(notifyAddr),&notifyRef);//GetAddrAndIndexByIndexAddr(notifyAddr + NotificationList_nextNotificationRef_Offset,&notifyRef);
			
	}
	
	if(euiccDataBufOffset == 0x09)      //没有找到, The list SHALL be empty if there are no pending notification matching the filtering criteria. 
	{
		SetWord((BYTE*)&RecData[0],TAG_LIST_NOTIFICATION);
		
		RecData[2] = 0x02;
		
		RecData[3] = 0xA0;
		
		RecData[4] = 0x00;
		
		SetOutgoingAndSend(0x05,RecData[2] + 3);
		
		return;
	}
	
	euiccDataBufOffset -= 9;         //所有BF2F的总长度
	
	lengthOfLen = ASN1_PaddingBERTlvTagAndLen(&tempBuf[5],0xA0,euiccDataBufOffset);                               //将A0L拼装在tempBuf[5]起始位置，并返回A0L占用的字节数, A082XXXX
	
	notifyLen = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0],TAG_LIST_NOTIFICATION,euiccDataBufOffset + lengthOfLen);  //将0xBF28L拼装在tempBuf[0]起始位置，并返回0xBF28L占用的字节数, BF2882XXXX
	
	ecpMem_Move(&tempBuf[notifyLen], &tempBuf[5], lengthOfLen);                                                       //将A0L连接在BF28L后，BF28从tempBuf[0]开始
	
	VMWriteBuf(EUICCTempDataBuf + (9 - lengthOfLen - notifyLen), tempBuf, lengthOfLen + notifyLen);
	
	SetSendFileExtendInfo(EUICCTempDataBuf + (9 - lengthOfLen - notifyLen), euiccDataBufOffset + lengthOfLen + notifyLen);//从EUICCTempDataBuf偏移为(9 - lengthOfLen - notifyLen)的位置发送长度为euiccDataBufOffset + lengthOfLen + notifyLen的响应数据	
}
/*
*@function     :CUSTOMER_RetrieveNotificationsList 
*
*@description  :This  function  retrieves the  list  of Pending  notifications for  installed  Profiles  including  their confirmation required and the related data.
*
*@see          :<SGP22_V2.2 > 5.7.10  Function (ES10b): RetrieveNotificationsList
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:获取通知成功的情况下，一条通知的长度会超过RecData的大小，所以需要将响应数据拼装在EUICCTempData中，然后再发送
*
*@author       :xuxiaojun@20181114
*
*@hisotry      :created by xuxiaojun@20181114
*              :modify by xieyong@20210827 在个人化预制接近0x7FFF长度的证书测试中响应数据长度大于2字节，修改euiccDataBufOffset为4字节，
*              								将A0L拼装在tempBuf[7]起始位置
*              BF2B 00   ------------------------------------获取所有的
*              BF2B 02 A000 ---------------------------------需要抛错，CHOICE类型必须有值             响应应该为 BF2B 03 81017F                                  
*              BF2B 05 A103 800101---------------------------需要抛错，TAG编码错误                    响应应该为 BF2B 03 81017F                                  
*              BF2B 05 A003 800101---------------------------根据seqNumber获取
*              BF2B 06 A004 81020640-------------------------根据profileManagementOperation获取
*              BF2B 09 A007 800101 81020640------------------需要抛错，CHOICE类型只能选择一项         响应应该为 BF2B 03 81017F                                  
*/
void CUSTOMER_RetrieveNotificationsList(void)
{
	UWORD xdata notifyLen,seqNumber, notifyAddr,notifyAddrCount[NOTIFICATION_MAX_NUMBER],euiccDataBufOffset;
	WORD idata notifyRef;
	BYTE idata lengthOfLen, indicate;
	WORD xdata notifyLenCount[NOTIFICATION_MAX_NUMBER];
	//NotificationListHead xdata newNorificationHead;

//	WORD idata nextNotificationRef;
//	UWORD xdata notifySeqNumber;
	BYTE idata profileManagementOperation;
//	BYTE idata notificationType;               // 记录当前条通知的类型是final result还是other
//	WORD idata euiccEumRef;                    // 只有通知类型为OtherSignedNotification时该字段才有效
	WORD idata notificationDataSize;

	if((RecData[2]+3) != P3)   //BF2B L  确保数据域只有BF2B tag，避免出现类似CLA IN P1 P2 P3 BF2100 BF2200的情况
	{
		CUSTOMER_SendSingleBYTEResult(TAG_RETRIEVE_NOTIFICATION_LIST, RESULT_UNDEFINED_ERROR);
		
		return;
	}
	
	if(RecData[2] != 0)
	{
		if((RecData[3] != 0xA0) || ((RecData[4] + 2) != RecData[2])) // 确保BF2B的子TAG只能是A0，且A0的长度正确
		{
			CUSTOMER_SendSingleBYTEResult(TAG_RETRIEVE_NOTIFICATION_LIST, RESULT_UNDEFINED_ERROR);
		
			return;
		}
		
		if(RecData[4] == 0x00)
		{
			RecData[2] = 0x00;   //BF2B 02 A000	也处理成获取卡内所有通知
		}
		else if((RecData[6]+2) != RecData[4])
		{
			CUSTOMER_SendSingleBYTEResult(TAG_RETRIEVE_NOTIFICATION_LIST, RESULT_UNDEFINED_ERROR);
		
			return;
		}
	}		
			
	indicate = 0x00;     //bit0为1表示根据seqNumber获取；bit1为1表示根据profileManagementOperation获取通知
	
	seqNumber = 0x00;
	
	if(RecData[2] == 0x00)
	{
		RecData[8] = 0xF0;   //BF2B 00  需要获取所有的通知	
		
		indicate |= 0x02;    
	}
	else
	{		
		switch(RecData[5])
		{
			case 0x80:     //根据sequence number查找, BF2B 05 A003 800101
			{
				indicate |= 0x01;
				
				seqNumber = ASN1_CheckIntegerLV((BYTE*)&RecData[6]);
	
				if(seqNumber == 0xFFFFFFFF)  // Interger编码错误
				{
					CUSTOMER_SendSingleBYTEResult(TAG_RETRIEVE_NOTIFICATION_LIST, RESULT_UNDEFINED_ERROR);
					return;
				}
				
				break;
			}
			case 0x81:     //根据notification type查找，BF2B 05 A003 81020780/81020640/81020520/81020410
			{
				if(RecData[6] != 0x02)
				{
					
				}
				
				if((RecData[6] == 0x01) && (RecData[7] == 0x00))
				{
					RecData[8] = 0x00;  //返回空列表 <TC_eUICC_ES10b_RetrieveNotificationsList_11_Nominal_Retrieve_by_Notification_Type_for_Install_Notifications>
					
					indicate |= 0x02;
				}
				else if(RecData[6] == 0x02)
				{
					indicate |= 0x02;
				}
				else
				{
					CUSTOMER_SendSingleBYTEResult(TAG_RETRIEVE_NOTIFICATION_LIST, RESULT_UNDEFINED_ERROR);
					
					return;
				}
				
				break;
			}
			
			default:   //其他 非法TAG
			{
				CUSTOMER_SendSingleBYTEResult(TAG_RETRIEVE_NOTIFICATION_LIST, RESULT_UNDEFINED_ERROR);
				
				return;
			}
		}
	}
	
	notifyRef = REF_NULL;
	
	notifyAddr = GetAddrAndIndexByIndexAddr(EUICCNotificationsRef_Addr,&notifyRef); //获取到通知列表中第一个通知的引用以及第一个通知结构存储的地址
	
	euiccDataBufOffset = 0;
	
	lengthOfLen = 0x00;
	
	ecpMem_Set(notifyAddrCount, ADDR_IS_NULL, sizeof(notifyAddrCount));
	
	ecpMem_Set(notifyLenCount, 0x00, sizeof(notifyLenCount));

	ecpMem_Set((BYTE*)&G_SendFileExtendBackDataStruct,0,sizeof(SendFileExtendBackData));
	
	while(notifyRef != REF_NULL)
	{
		//VMReadBuf((BYTE *)&newNorificationHead, notifyAddr, sizeof(newNorificationHead));
		notificationDataSize = memReadU16(MEMtPM8(GetNotificationList_notificationDataSizeAddr(notifyAddr)));
		
		if(notificationDataSize == 0x00)
		{
			break;
		}
				
		if(VMReadByte(GetNotificationList_notificationTypeAddr(notifyAddr)) == NOTIFICATION_TYPE_OTHER)//通知内容 30 L V NotificationList_notificationDataSize_Offset存的是通知内容中除了EUM EUICC证书以外的总长度，并不是一个完整通知的长度
		{
			lengthOfLen = ASN1_GetBERTlvLen(GetNotificationList_notificationData_ArrayAddr(notifyAddr) + 1,(UWORD*)&notifyLen);//ASN1_GetBERTlvLen(notifyAddr + NotificationList_notificationData_Offset + 1,(UWORD*)&notifyLen);
			
			notifyLen += (1 + lengthOfLen);
		}
		else // NOTIFICATION_TYPE_FINALRESULT 通知内容 BF2B L V，NotificationList_notificationDataSize_Offset存的是整个通知的实际长度
		{			
			notifyLen = notificationDataSize;
		}	
		
		if((indicate & 0x01) == 0x01)       //根据seqNume获取
		{
			if(seqNumber == VMReadUword(GetNotificationList_seqNumberAddr(notifyAddr)))
			{
				euiccDataBufOffset += notifyLen;
				
				G_SendFileExtendBackDataStruct.notifiy[G_SendFileExtendBackDataStruct.notifiyNum] = notifyRef;
				
				G_SendFileExtendBackDataStruct.notifiyNum ++;//通知个数加1
												
				break;
			}
		}
		else if((indicate & 0x02) == 0x02)  //根据profileManagementOperation获取
		{
			profileManagementOperation = VMReadByte(GetNotificationList_profileManagementOperationAddr(notifyAddr));

			if((RecData[8] & profileManagementOperation) == profileManagementOperation)
			{
				euiccDataBufOffset += notifyLen;
				
				G_SendFileExtendBackDataStruct.notifiy[G_SendFileExtendBackDataStruct.notifiyNum] = notifyRef;
				
				G_SendFileExtendBackDataStruct.notifiyNum ++;//通知个数加1
			}
		}
		else
		{
			//针对白盒测试添加 实际不会出现该情况，异常已在前面判断
		}
			
		notifyAddr = GetAddrAndIndexByIndexAddr(GetNotificationList_nextNotificationRefAddr(notifyAddr),&notifyRef);
	}
	
	if(euiccDataBufOffset == 0)   //卡内没有通知 The list SHALL be empty if there are no pending notifications matching the filtering criteria
	{
		SetWord((BYTE*)&RecData[0],TAG_RETRIEVE_NOTIFICATION_LIST);
		
		RecData[2] = 0x02;
		
		RecData[3] = 0xA0;
		
		RecData[4] = 0x00;
		
		SetOutgoingAndSend(0x05,RecData[2] + 3);
		
		return;
	}
		
	lengthOfLen = ASN1_PaddingBERTlvTagAndLen(&RecData[7],0xA0,euiccDataBufOffset); //将A0L拼装在tempBuf[5]起始位置，并返回A0L占用的字节数, A082XXXX
	
	notifyLen = ASN1_PaddingBERTlvTagAndLen(&RecData[0],TAG_RETRIEVE_NOTIFICATION_LIST,euiccDataBufOffset + lengthOfLen);  //将0xBF28L拼装在tempBuf[0]起始位置，并返回0xBF28L占用的字节数, BF2882XXXX

	ecpMem_Move(&RecData[notifyLen], &RecData[7], lengthOfLen); //将A0L连接在BF2BL后，BF2B从tempBuf[0]开始
	
	VMWriteBuf(EUICCTempDataBuf,RecData,lengthOfLen+notifyLen);//第一部分要发送的数据 
	
	G_SendFileExtendBackDataStruct.backAddr = EUICCTempDataBuf;
	
	G_SendFileExtendBackDataStruct.len = lengthOfLen+notifyLen;
	
	G_SendFileExtendBackDataStruct.totalLen = euiccDataBufOffset + lengthOfLen + notifyLen;
	
	G_SendFileExtendBackDataStruct.currentChannel = G_CurSelectedChannel;
	
	if(G_SendFileExtendBackDataStruct.totalLen > 0x100)
	{
		ThrowError(0x6100);
	}
	else
	{
		ThrowError(0x6100|G_SendFileExtendBackDataStruct.totalLen);
	}

}

/*
*@function     :CUSTOMER_RemoveNotificationFromList 
*
*@description  :This function informs the eUICC that: 
*			    1.  A specific Notification has been sent to the recipient address; and 
*				2.  The  eUICC  SHALL  remove  such  Notification  from  the  Pending  Notifications List
*
*@see          :<SGP22_V2.2 > 5.7.11  Function (ES10b): RemoveNotificationFromList 
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:拼装后的响应数据直接放在RecData中发送出去
*
*@author       :xuxiaojun@20181113
*
*@hisotry      :created by xuxiaojun@20181113
*测试说明      :1. 找不到sequence一致的；2.删除头；3删除尾；4.删除中间
*/
void CUSTOMER_RemoveNotificationFromList(void)
{
	UWORD xdata seqNumber, notifyAddr,previousAddr,tmpSeqNumber;
	WORD idata notifyRef, nextRef, notifyLen;
	BYTE idata result, transactionTag;
	
	if((RecData[3] != 0x80) || ((RecData[2]+3) != P3) || ((RecData[4]+2) != RecData[2]))   // BF30 L 80 L V，确保数据域只有该TAG
	{
		CUSTOMER_SendSingleBYTEResult(TAG_REMOVE_NOTIFICATIOV_FROM_LIST, RESULT_UNDEFINED_ERROR);
		return;
	}
	
	seqNumber = ASN1_CheckIntegerLV((BYTE*)&RecData[4]);
	
	if(seqNumber == 0xFFFFFFFF)  // Interger编码错误
	{
		CUSTOMER_SendSingleBYTEResult(TAG_REMOVE_NOTIFICATIOV_FROM_LIST, RESULT_UNDEFINED_ERROR);
		return;
	}
	
	result = RESULT_NOTHINGTODELETE;  // nothingToDelete(1)
	
	previousAddr = EUICCNotificationsRef_Addr;

	notifyAddr = GetAddrAndIndexByIndexAddr(EUICCNotificationsRef_Addr,&notifyRef);      //获取到通知列表中第一个通知的引用以及第一个通知数据存储的地址
	//previousAddr = notifyAddr;

	while(notifyRef != REF_NULL)
	{
		tmpSeqNumber = VMReadUword(GetNotificationList_seqNumberAddr(notifyAddr));

		//VMReadBuf((BYTE *)&tmpSeqNumber, notifyAddr + NotificationList_seqNumber_Offset, sizeof(UWORD) );
						
		if(seqNumber == tmpSeqNumber)
		{
			notifyLen = memReadU16(MEMtPM8(GetNotificationList_notificationDataSizeAddr(notifyAddr)));//VMReadBuf((BYTE*)&notifyLen,notifyAddr + NotificationList_notificationDataSize_Offset,2);
			
			if(notifyLen == 0)//通知已被释放
			{
				break;
			}
				
			transactionTag = IF_BeginTransaction();
						
			GetAddrAndIndexByIndexAddr(GetNotificationList_nextNotificationRefAddr(notifyAddr),&nextRef);
			
			if(previousAddr == EUICCNotificationsRef_Addr)
			{
				memWriteU16Atomic(MEMtPM8(previousAddr),nextRef);//VMWriteBuf(previousAddr + NotificationList_nextNotificationRef_Offset,(BYTE *)&nextRef,sizeof(WORD)); // 将下一个通知链接到上一个通知
			}
			else
			{
				memWriteU16Atomic(MEMtPM8(GetNotificationList_nextNotificationRefAddr(previousAddr)),nextRef);//VMWriteBuf(previousAddr + NotificationList_nextNotificationRef_Offset,(BYTE *)&nextRef,sizeof(WORD)); // 将下一个通知链接到上一个通知
			}

			memWriteU16Atomic(MEMtPM8(GetNotificationList_notificationDataSizeAddr(notifyAddr)),0);//VMSetBuf(notifyAddr + NotificationList_notificationDataSize_Offset, 0x00 ,sizeof(WORD));
			
			memWriteU16Atomic(MEMtPM8(GetNotificationList_nextNotificationRefAddr(notifyAddr)),REF_NULL);//VMSetBuf(notifyAddr + NotificationList_nextNotificationRef_Offset, REF_NULL ,sizeof(WORD));
			
			Notification_GetValidNum(&previousAddr, &notifyAddr);//notifyAddr为最后一条通知的地址，previousAddr为最后一条有效的通知的地址
			
			memWriteU16Atomic(MEMtPM8(GetNotificationList_nextNotificationRefAddr(notifyAddr)),notifyRef);//VMWriteBuf(notifyAddr + NotificationList_nextNotificationRef_Offset,(BYTE *)&notifyRef,sizeof(WORD)); // 将本次被移除的通知链接到通知列表，变成最后一个通知存储区
			
			IF_CommitTransaction(transactionTag);
			
			result = RESULT_OK_;
			
			break;
		}
		else
		{
			previousAddr = notifyAddr;
			
			notifyAddr = GetAddrAndIndexByIndexAddr(GetNotificationList_nextNotificationRefAddr(previousAddr) ,&notifyRef);
		}
	}
	
	CUSTOMER_SendSingleBYTEResult(TAG_REMOVE_NOTIFICATIOV_FROM_LIST, result);
}

/*
*@function     : CUSTOMER_LoadCRL 
*
*@description  :This function is used to transfer a CRL to the eUICC. It is optionally supported by the Device and the eUICC, as indicated by their corresponding capabilities
*
*@see          : <SGP22_V2.2 > 5.7.12  Function (ES10b): LoadCRL
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@author       :
*
*@hisotry      :
*/
void CUSTOMER_LoadCRL(void)
{
	ThrowError(SW_INS_NOT_SUPPORTED);
}
/*
*@function     : CUSTOMER_GetProfilesInfo 
*
*@description  :This  function  retrieves the  list  of  Profile  information  for  installed  Profiles  including  their current  state  (Enabled/Disabled)  and  their  associated  Profile  Metadata.  This  function  MAY also be used to retrieve this information for a particular Profile
*
*@see          : <SGP22_V2.2 > 5.7.15  Function (ES10c): GetProfilesInfo 
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@author       :
*
*@hisotry      :
*/
//modify by xieyong@20220914  GetProfilesInfo增加0xBF22tag
//If no tag list is present, the eUICC SHALL return the default ProfileInfo: the ProfileInfo data objects marked with (*) for each Profile matching the selection criterion.
BYTE code defaultTagList[] = {0x0A,0x5A,0x4F,0x9F,0x70,0x90,0x91,0x92,0x93,0x94,0x95,0xB6,0xB7,0xB8,0x99
#if RSP_SGP22_BF22
		,0xBF,0x22
#endif
};
						//默认时仅需拼装0x0A个tag，数组后4个tag用来当做顺序模板
void CUSTOMER_GetProfilesInfo(void)
{
	BYTE * tagList;
	BYTE idata i,tempOffset,compareLen;
	WORD idata offset,dataLen;	
	BYTE idata profileNumber, mode;
	BYTE xdata profileIndex[PROFILE_MAX_NUMBER];
	
	if(IF_ProfielChangeOngoing() == TRUE)
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);//If there is a Profile state change ongoing (i.e. during an enable, disable or eUICCMemoryReset command requiring a REFRESH) and the Profile state is requested, the eUICC SHALL terminate the command with the status word '69 85' (Conditions of use not satisfied).
	}
		
	if(P3 != (RecData[2] + 3))	//检查tag和len的合法性
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	profileNumber = 0;
	
	ecpMem_Set(&profileIndex[0],0x00,PROFILE_MAX_NUMBER);
	
	offset = 0x03;	//指向BF2D后的第一个tag 
	
	if((RecData[2] == 0x00) || (RecData[3] == 0x5C))//列出所有isd-p的信息  BF2D00  或者 BF2D Len 5C xx xx  ，个人版规范：5.7.15 Example of use: 1和3
	{	
		profileNumber = GetValidProfileNumber(profileIndex);
		
	}
	else //根据searchCriteria [0] CHOICE列出特定isd-p的信息，后续如果有5C tag根据5C列出，没有5C则按默认*列出
	{
		if(RecData[offset] != TAG_SearchCriteria)	//searchCriteria [0]
		{
			ThrowError(SW_WRONG_DATA);   //如果RecData[2] != 0,  RecData[3]只能是0x5C或者0xA0
		}
		
		offset += 2;//指向A0 tag的内容  tag "4F"或"5A"或"95"
		
		if(RecData[offset] == TAG_SearchCriteria_AID) //根据aid查找  tag "0x4F"
		{			
			profileIndex[0] = GetProfileIndexByAID(&RecData[offset + 2],RecData[offset+1]);
					
			if(ProfileIndexIsValid(profileIndex[0]))
			//if(profileIndex[0] != ILLEGAL_PROFILEINDEX)
			{
				profileNumber = 1;
			}
		}
		else if(RecData[offset] == TAG_ICCID)//根据ICCID查找 tag "0x5A"
		{
			profileIndex[0] = GetProfileIndexByICCID(&RecData[offset + 2]);

			//if(profileIndex[0] != ILLEGAL_PROFILEINDEX)
			if(ProfileIndexIsValid(profileIndex[0]))
			{
				profileNumber = 1;
			}

		}
		else if(RecData[offset] == TAG_PROFILECLASS)//根据profileClass查找
		{
			profileNumber = GetProfileIndexByProfileClass(RecData[offset + 2], profileIndex);
		}
		else //根据《代码评审问题跟踪表  - 20230327》讨论决定：searchCriteria是4F\5A\95之外的其它值时，COS按照未查找到符合条件的ISD-P来处理
		{
			profileNumber = 0;
		}
	
		offset += (RecData[offset+1] + 2);//指向下一个tag ,5C
	}
	
	if(profileNumber == 0) //没有符合查找条件的isd-p
	{
		tempOffset = ASN1_PaddingBERTlvTagAndLen(&RecData[0],0xBF2D,0x02);
		
		RecData[tempOffset++] = 0xA0;
		
		RecData[tempOffset++] = 0x00;
		
		SetOutgoingAndSend(0x05,tempOffset);
		
		return ;
	}
	
	
	if((RecData[offset] == 0x5C)&&(RecData[offset + 1] != 0x00)&&((offset + RecData[offset+1] + 2) == P3)) //列出所有符合条件的isd-p的5C tag指定信息, 5C 00时期望返回所有profile信息 <eSIM_Test_CMCC_additional-LS\ProfileInfoListTest03.html>
	{
		ecpMem_Copy(&tempBuf[0],defaultTagList,sizeof(defaultTagList));
		
		tempBuf[0] = sizeof(defaultTagList) - 1;
		
		ResetTagList(&RecData[offset + 1],tempBuf);//重置TagList的顺序  根据规范中响应数据格式排序  RecData[offste + 1]// 指向tagList 的L字段		
		
		mode = GET_PROFILE_INFO_TAG_LIST;
	}
 	else 
	{
		ecpMem_Copy(&RecData[offset + 1],&defaultTagList[0],sizeof(defaultTagList));//列出所有isd-p的默认*指定信息  LV结构
		
		mode = GET_PROFILE_INFO_DEF_LIST;
	}
	
	tagList = &RecData[offset + 1];
	
	/*
	-------BF2D XX XX XX: VALUE                                           //ProfileInfoListResponse ::= [45] CHOICE
	----------------------A0 XX XX XX: VALUE                              //profileInfoListOk SEQUENCE OF ProfileInfo
	--------------PROFILE1------------ E3 XX XX XX: VALUE                 //ProfileInfo ::= [PRIVATE 3] SEQUENCE
	                                   ------------ 4F XX: VALUE
	                                   ------------ 9F 70 XX : VALUE
	                                   ----------OTHER TAG LIST DATA 
	--------------PROFILE2------------ E3 XX XX XX: VALUE
	                                   ------------ 4F XX: VALUE
	                                   ------------ 9F 70 XX : VALUE
	                                   ----------OTHER TAG LIST DATA 
	--------------PROFILE3------------ E3 XX XX XX: VALUE
	                                   ------------ 4F XX: VALUE
	                                   ------------ 9F 70 XX : VALUE
	                                   ----------OTHER TAG LIST DATA 
	
	…………………………………ROFILE N
                                        	
	
	*/
	offset = 0x09; //偏移开 BF 2D XX XX XX A0 XX XX XX E3 XX XX XX ,指向第一个E3	
	
	for(i = 0;i < profileNumber;i++)	//拼装数据  每个isd-p 一个E3 tag
	{
		offset += GetProfileInfoDataByTagList(EUICCTempDataBuf + offset, EUICCTempDataBufSize-offset, tagList,profileIndex[i], mode); //modify by xuxiaojun@20230314: 增加边界检查，越界返回0
	}
	
	dataLen = offset - 9;
	
	compareLen = ASN1_PaddingBERTlvTagAndLen(&tempBuf[5],0xA0,dataLen);//复用变量compareLen  拼装A0 XX XX XX 
	
	tempOffset = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0],0xBF2D,dataLen + compareLen);//复用变量tempOffset
	
	if(tempOffset < 5)
	{
		ecpMem_Move(&tempBuf[tempOffset],&tempBuf[5],compareLen);
	}
	
	VMWriteBuf(EUICCTempDataBuf + 9 - (compareLen + tempOffset),&tempBuf[0],(compareLen + tempOffset));//将BF2D len + A0 len写入
	
	SetSendFileExtendInfo(EUICCTempDataBuf + 9 - (compareLen + tempOffset), compareLen + tempOffset + dataLen); //从EUICCTempDataBuf偏移为(9 - lengthOfLen - notifyLen)的位置发送长度为euiccDataBufOffset + lengthOfLen + notifyLen的响应数据		
}
/*

*@function     : CUSTOMER_ChangeProfileNoReset
*
*@description  :在设备不重启的情况下切换profile需要调用这个函数切换全局变量和一些配置

*@see          :
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by chentao@20230210
*         */
void CUSTOMER_ChangeProfileNoReset(BYTE targetProfileIndex, BYTE mode, BYTE refreshFlag)
{
	G_CurEnabledProfileIndex = targetProfileIndex;

	G_CurExecutingProfileIndex = targetProfileIndex;

	SEEUICCVariableInit(mode);

	CloseChannelAfterProfileChange(refreshFlag);// Close all logical channels which still have an application of the currently enabled Profile selected, 未考虑SE应用

	IF_ChangeProfileLTEInfoNoReset(mode);
}

/*
*@function     : CUSTOMER_EnableProfile 
*
*@description  :This function is used to enable a Profile on the eUICC. The function makes the target Profile enabled, and disables implicitly the currently Enabled Profile
*
*@see          : <SGP22_V2.2 > 5.7.16  Function (ES10c): EnableProfile 
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@20181128
*              :V1.1 modify by xuxiaojun@20190326 调整异常判断的先后顺序，<TY_Offline_RSP_ManageCmdTest \ EnableProfile_Case4_TSE10>
*              :V1.2 modify by xuxiaojun@20190520 如果是RESET模式的话，要设备重启之后，才能实际切换. 暂时只考虑了一次enable之后就等待reset的情况，没有考虑enable1, enable2然后设备再reset的情况
*/
void CUSTOMER_EnableProfile(void)
{
	BYTE idata commandCase3,offset;
	BYTE idata refreshFlag;
	BYTE idata targetProfileIndex;
	BYTE idata targetProfileClass, currentProfileClass;
	BYTE idata lastOperationalProfile;
	BYTE idata isSupportProfileChangeMode;
	
	refreshFlag = FALSE;
	
	commandCase3 = FALSE;
	
	isSupportProfileChangeMode = FALSE;
	
	if(P3 != (RecData[2] + 3))//长度判断
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(P1 == 0x90) // EnableProfile时需要支持 Alternative Case 3 Command
	{
		commandCase3 = TRUE;
	}
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *1. profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *2. GPload过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *3. GPstoredata过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 */
	if((IF_IsProfileLoadingSession() == TRUE)||(IF_CapLoaingInCurrnetProfile() == TRUE)||(IF_MoreCommandsInCurrnetProfile()==TRUE))   // add by chentao@20230306 profiel 下载过程中不允许EnableProfile
	{
		if(commandCase3)
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE, RESULT_UNDEFINED_ERROR);
		}
		return;
	}


	if((STKPerform[0] == TRUE) ||( G_STKBusy != FALSE))  //if there is a proactive session ongoing, the eUICC SHALL terminate the EnableProfile command with error "catBusy".
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_CATBUSY);		//"cat busy"
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_CATBUSY);
		}
		
		return;
	}
		
	offset = 0x03;//指向tag profileIdentifier CHOICE ---“A0”

	if(RecData[offset] != 0xA0)
	{		
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);		//Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_UNDEFINED_ERROR); // <TY_Offline_RSP_ManageCmdTest\EnableProfile_Case4_TSE09>
		}
		
		return;
	}
	
	offset += 0x02;//指向profileIdentifier内的tag字段
	
	if(RecData[offset] == TAG_SearchCriteria_AID)
	{
		targetProfileIndex = GetProfileIndexByAID(&RecData[offset+2], RecData[offset+1]);
	}
	else if(RecData[offset] == TAG_ICCID)
	{
		targetProfileIndex = GetProfileIndexByICCID(&RecData[offset+2]);
	}
	else
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);		//Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_ICCID_NOT_FOUND);
		}
		
		return;
	}
	
	offset += (RecData[offset+1] + 2);//指向refreshFlag 的tag
	
	if((offset + 3) != P3) // 还有一些其他的TAG存在, RSP_EUICC测试_管理指令测试_管理指令异常测试\EnableProfile_Case4_TSE10.html
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);//'6A 82': Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_UNDEFINED_ERROR);
		}
		return ;
	}

	//if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(targetProfileIndex) == FALSE)
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);//'6A 82': Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_ICCID_NOT_FOUND);
		}
		return ;
	}
	
	refreshFlag = RecData[offset+2];

	if(GetProfileState(targetProfileIndex) != PROFILE_STATE_DISABLED)  //Verify that the target Profile is in the Disabled state.
	{
		if(commandCase3)//报错返回 CUSTOMER_SendSingleBYTEResult
		{
			ThrowError(PROFILE_NOT_ALLOWED);		//'69 85': Profile not in disabled state or command not allowed by Profile Policy Rules or re-enabling wrong Profile
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_PROFILE_NotINDisabledState);
		}
		return;
	}
	
	targetProfileClass = GetSingleBYTEProfileMeta(targetProfileIndex, MetaDataProfileClass_INDEX,2) ;  //获取目标profile的类别
	
	lastOperationalProfile = JcvmConfig_GetRSPLastOperationaProfile();//VMReadByte(EUICCLastOperationaProfile_Addr) ;

	//if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)  //当前存在有效的profile
	if(ProfileIndexIsValid(G_CurEnabledProfileIndex))//当前存在有效的profile
	{
		currentProfileClass = GetSingleBYTEProfileMeta(G_CurEnabledProfileIndex, MetaDataProfileClass_INDEX,2) ;  //获取当前enable的profile的类别

		if((targetProfileClass == PROFILE_CLASS_PROVISIONING) && (currentProfileClass == PROFILE_CLASS_OPERATIONAL)) // <eSIM_Test_additional/补充测试/Enabled_Provisioning_Profile_Regardless_of_currently_enabled_Operational_Profile_PPR1>
		{
			//不需要检测PPR1是否允许 Provisioning Profile and Operational Profile Policies. Provisioning Profiles SHALL still be usable, even if the currently enabled Operational Profile is subject to Profile Policy Rule 'ppr1'
		}
		else if(targetProfileClass != PROFILE_CLASS_TEST)   //If the target Profile is not a Test Profile: check if the Profile Policy Rules of the currently Enabled Profile allow its disablings
		{
			if((GetSingleBYTEProfileMeta(G_CurEnabledProfileIndex,MetaDataProfilePolicyRules_INDEX,3)&PPRIDS_ppr1) == PPRIDS_ppr1)  //不允许失效当前profile
			{
				if(commandCase3)//出错返回
				{
					ThrowError(PROFILE_NOT_ALLOWED);	//'69 85': Profile not in disabled state or command not allowed by Profile Policy Rules or re-enabling wrong Profile
				}
				else
				{
					CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_DISALLOWEDByPolicy);
				}
				return;
			}
		}
		
		if(currentProfileClass == PROFILE_CLASS_TEST) //If the currently Enabled Profile is a Test Profile，that the target Profile is (another) Test Profile, or  if an Operational Profile was in Enabled state before the (first) Test Profile was enabled, that the target Profile is this Operational Profile. 
		{
			if( targetProfileClass != PROFILE_CLASS_TEST )
			{
				//if((lastOperationalProfile != ILLEGAL_PROFILEINDEX)&&(targetProfileIndex != lastOperationalProfile))
				if((ProfileIndexIsValid(lastOperationalProfile))&&(targetProfileIndex != lastOperationalProfile))
				{
					if(commandCase3)//出错返回
					{
						ThrowError(PROFILE_NOT_ALLOWED);	//'69 85': Profile not in disabled state or command not allowed by Profile Policy Rules or re-enabling wrong Profile
					}
					else
					{
						CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_WRONGProfileReenabling);
					}
					return;
				}
			}			
		}
	}

	//SEUICC_SynSEForSingleProfile(targetProfileIndex,ModeSyn_CheckTag);
	
	if(refreshFlag == FALSE) // If the refreshFlag is not set: the eUICC SHALL reset the PIN status;the eUICC SHALL disable the currently Enabled Profile (if any), Enable the target Profile and implicitly select the MF on the basic logical channel;The eUICC SHALL return OK to the LPAd
	{
		ProfileChangeEnable_GernerateNotifyAndSetState(targetProfileIndex,targetProfileClass, MODE_PROFILE_CHANGE);

		CUSTOMER_ChangeProfileNoReset(targetProfileIndex, MODE_PROFILE_CHANGE,refreshFlag);

		if(!commandCase3)  // the response data SHALL not be present 发送61xx以后可能不取响应 跳回主流程 但是要求激活profile成功 所以把发响应放在最后执行
		{
			CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_OK_);//STEP4:  The eUICC SHALL return OK to the LPAd
		}
		return;
	}
	else
	{
		if( IF_CATIsTnitialised() == FALSE) //add by chentao @20230209 SGP22V3.0 5.7.16 Function (ES10c): EnableProfile If CAT is not initialised, the eUICC SHALL return an error code commandError.
		{
			if(commandCase3)
			{
				ThrowError(PROFILE_NOT_ALLOWED); // 6985
			}
			else
			{
				CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_COMMANDERROR);
			}
			return;
		}
	}

	if(!commandCase3)  // the response data SHALL not be present
	{
		CUSTOMER_SendSingleBYTEResult(TAG_ENABLE_PROFILE,RESULT_OK_);//STEP4:  The eUICC SHALL return OK to the LPAd
	}
		
	isSupportProfileChangeMode = IF_TerminaprofileSupportEUICCChangeMode();  //卡片支持 eUICC Profile State Change" mode 
	
	offset = FALSE;
	
	if(isSupportProfileChangeMode == TRUE)  // refreshFlag是ASN.1的boolean编码
	{
		offset = IF_SendRefreshCommandOfeUICCProfileStateChange(); // 发送 the eUICC SHALL send the REFRESH command in "eUICC Profile State Change" mode (if supported by the Device) , 收到失败的terminame response 直接退出即可
		
		if(offset == FALSE)
		{
			return;
		}

		if(GetProfileEntryRef(targetProfileIndex) == REF_NULL)  // modify by chentao @20230114  正在激活的profile被删除了直接退出   enable profile发送refresh 收到12 后收到删除指令
		{
			return;
		}

		ProfileChangeEnable_GernerateNotifyAndSetState(targetProfileIndex,targetProfileClass, MODE_PROFILE_CHANGE);

		CUSTOMER_ChangeProfileNoReset(targetProfileIndex, MODE_PROFILE_CHANGE,refreshFlag);

		return;
	}

	ProfileChangeEnable_GernerateNotifyAndSetState(targetProfileIndex, targetProfileClass, MODE_RESET);

	//发refresh	收到了14，但结果不是CMD_SUCUSS 复位不切----方案：Profile state change is ongoing状态变为FALSE，profile激活相关标记还原，  那么已经生成的激活通知不处理
	if(IF_SendRefreshCommandOfUICCResetMode() == FALSE)//正常情况发送fresh 后终端重启，后面代码不会执行，只有终端不重启的情况会执行下面的代码
	{
		// add by chentao@20230209  if the reset Proactive command send fail, then need to fallback
		JcvmConfig_SetUpdataEnableProfileIndex(ILLEGAL_PROFILEINDEX);

		JcvmConfig_SetRSPLastOperationaProfile(lastOperationalProfile);
	}
}
/*
*@function     : CUSTOMER_DisableProfile
*
*@description  :This function is used to disable a Profile on the eUICC.
*
*@see          : <SGP22_V2.2 > 5.7.17  Function (ES10c): DisableProfile
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@20181128
*              :V1.1 modify by xuxiaojun@20190322 根据用例<TY_Offline_RSP_ManageCmdTest\DisableProfile_TSE10.html>调整一下抛异常的顺序
*              :V1.2 modify by xuxiaojun@20190520 如果是RESET模式的话，要设备重启之后，才能实际切换. 暂时只考虑了一次disable之后就等待reset的情况，没有考虑disable1, disable2然后设备再reset的情况
*/
void CUSTOMER_DisableProfile(void)
{
	BYTE idata commandCase3,offset;
	BYTE idata refreshFlag;
	BYTE idata targetProfileIndex,profileIndex;
	BYTE idata isSupportProfileChangeMode;
	
	refreshFlag = FALSE;
	
	commandCase3 = FALSE;
	
	isSupportProfileChangeMode = FALSE;
	
	if(P3 != (RecData[2] + 3))//长度判断
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(P1 == 0x90) // DisableProfile时需要支持 Alternative Case 3 Command
	{
		commandCase3 = TRUE;
	}
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *1. profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *2. GPload过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *3. GPstoredata过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 */
	if((IF_IsProfileLoadingSession() == TRUE)||(IF_CapLoaingInCurrnetProfile() == TRUE)||(IF_MoreCommandsInCurrnetProfile() == TRUE))   // add by chentao@20230306 profiel 下载过程中不允许DisableProfile
	{
		if(commandCase3)
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE, RESULT_UNDEFINED_ERROR);
		}
		return;
	}

	if((STKPerform[0] == TRUE) ||( G_STKBusy != FALSE))  //if there is a proactive session ongoing, the eUICC SHALL terminate the EnableProfile command with error "catBusy".
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_CATBUSY);		//"cat busy"
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_CATBUSY);
		}
		
		return;
	}
	
	offset = 0x03;//指向tag profileIdentifier CHOICE ---“A0”
	
	if(RecData[offset] != 0xA0)
	{		
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);		//Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_UNDEFINED_ERROR);
		}
		
		return;
	}
	
	offset += 0x02;//指向profileIdentifier内的tag字段
	
	if(RecData[offset] == TAG_SearchCriteria_AID)
	{
		targetProfileIndex = GetProfileIndexByAID(&RecData[offset+2], RecData[offset+1]);
	}
	else if(RecData[offset] == TAG_ICCID)
	{
		targetProfileIndex = GetProfileIndexByICCID(&RecData[offset+2]);
	}
	else
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);		//Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_ICCID_NOT_FOUND);
		}
		
		return;
	}
	
	offset += (RecData[offset+1] + 2);//指向refreshFlag 的tag
	
	if((offset + 3) != P3) // 还有一些其他的TAG存在, RSP_EUICC测试_管理指令测试_管理指令异常测试\DisableProfile_TSE10.html
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);//'6A 82': Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_UNDEFINED_ERROR);
		}
		return ;
	}

	//if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(targetProfileIndex) == FALSE)
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);//'6A 82': Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_ICCID_NOT_FOUND);
		}
		return ;
	}
	
	refreshFlag = RecData[offset+2];
	
	if(GetProfileState(targetProfileIndex) != PROFILE_STATE_ENABLED)  //Verify that the target Profile is in the Enabled state
	{
		if(commandCase3)//报错返回 CUSTOMER_SendSingleBYTEResult
		{
			ThrowError(PROFILE_NOT_ALLOWED);		//'69 85': Profile not in enabled state or command not allowed by Profile Policy Rules
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_PROFILE_NotINEnabledState);//profileNotInEnabledState
		}
		return;
	}
	
	if(GetSingleBYTEProfileMeta(G_CurEnabledProfileIndex, MetaDataProfileClass_INDEX,2) != PROFILE_CLASS_TEST) //If the currently Enabled Profile is not a Test Profile: check if the Profile Policy Rules of the target Profile allows its disabling.
	{
		if((GetSingleBYTEProfileMeta(targetProfileIndex,MetaDataProfilePolicyRules_INDEX,3)&PPRIDS_ppr1) == PPRIDS_ppr1)  //不允许失效当前profile
		{
			if(commandCase3)//出错返回
			{
				ThrowError(PROFILE_NOT_ALLOWED);	//'69 85': Profile not in disabled state or command not allowed by Profile Policy Rules or re-enabling wrong Profile
			}
			else
			{
				CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_DISALLOWEDByPolicy);
			}
			return;
		}
	}

	if(IF_CATIsTnitialised() == FALSE) //modify by xxj@20230321:  根据《新一代ESIM产品质量整改方案-20230301-讨论后new》 终端不发terminal profile的情况下，如果收到disable profile时，不管refresh有没有置上，都按照没有置上来处理
	{
		refreshFlag = FALSE;
	}

	if(refreshFlag == FALSE)
	{
		profileIndex = ProfileChangeDisable_GernerateNotifyAndSetState(targetProfileIndex, MODE_PROFILE_CHANGE);

		CUSTOMER_ChangeProfileNoReset(profileIndex, MODE_PROFILE_CHANGE,refreshFlag);

		if(!commandCase3)
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_OK_);//STEP4:  The eUICC SHALL return OK to the LPAd
		}
				
		return ;
	}
	
	if(!commandCase3)
	{
		CUSTOMER_SendSingleBYTEResult(TAG_DISABLE_PROFILE,RESULT_OK_);//STEP4:  The eUICC SHALL return OK to the LPAd
	}
	
	isSupportProfileChangeMode = IF_TerminaprofileSupportEUICCChangeMode();  //卡片支持 eUICC Profile State Change" mode
	
	offset = FALSE;
	
	if(isSupportProfileChangeMode == TRUE)
	{
		offset = IF_SendRefreshCommandOfeUICCProfileStateChange(); //发送 the eUICC SHALL send the REFRESH command in "eUICC Profile State Change" mode (if supported by the Device) , 收到失败的terminame response 直接退出即可
		
		if(offset == FALSE)
		{
			return;
		}

		if(GetProfileEntryRef(targetProfileIndex) == REF_NULL) // modify by chentao @20230114  正在失效的profile被删除了直接退出   disable profile发送refresh 收到12 后收到删除指令
		{
			return;
		}

		profileIndex = ProfileChangeDisable_GernerateNotifyAndSetState(targetProfileIndex, MODE_PROFILE_CHANGE);
		CUSTOMER_ChangeProfileNoReset(profileIndex, MODE_PROFILE_CHANGE,refreshFlag);

		return ;
	}
	// modify by陈涛@20230209
	profileIndex = ProfileChangeDisable_GernerateNotifyAndSetState(targetProfileIndex, MODE_RESET);  //不用进行MF 状态的切换，后续卡片会直接复位

	//收到了14，但结果不是CMD_SUCUSS 复位不切
	//方案：Profile state change is ongoing状态变为FALSE，profile激活相关标记还原，  那么已经生成的激活通知不处理
	if(IF_SendRefreshCommandOfUICCResetMode() == FALSE)//正常情况发送fresh 后终端重启，后面代码不会执行，只有终端不重启的情况会执行下面的代码
	// add by chentao@20230306 if terminal is not support reset Proactive command
	{
		if(profileIndex == ILLEGAL_PROFILEINDEX)
		{
			JcvmConfig_SetUpdataDisableProfileIndex(ILLEGAL_PROFILEINDEX);
		}
		else
		{
			JcvmConfig_SetUpdataEnableProfileIndex(ILLEGAL_PROFILEINDEX);
		}

	}
}

/*
*@function     : CUSTOMER_DeleteProfile 
*
*@description  :This function deletes a Profile from eUICC. This function can be used at any time by the LPAd. The Profile to be deleted can be identified by ISD-P AID or ICCID.. 
*
*@see          : <SGP22_V2.2 > 5.7.18  Function (ES10c): DeleteProfile
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@author       :xuyz
*
*@hisotry      :
*/
void CUSTOMER_DeleteProfile(void)
{
	BYTE idata commandCase3,offset;
	BYTE idata targetProfileIndex;
	BYTE idata lastOperationalProfile;
	BYTE idata transactionTag,checkLen;
	WORD idata fileName[PROFILEDELETE_BACKUPFILESYSTEM];

	commandCase3 = FALSE;
	
	if(P1 == 0x90) // DeleteProfile时需要支持 Alternative Case 3 Command
	{
		commandCase3 = TRUE;
	}
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *1. profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *2. GPload过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *3. GPstoredata过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 */
	if((IF_IsProfileLoadingSession() == TRUE)||(IF_CapLoaingInCurrnetProfile() == TRUE)||(IF_MoreCommandsInCurrnetProfile()==TRUE))   // add by chentao@20230112 profiel 下载过程中不允许DeleteProfile
	{
		if(commandCase3)
		{
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DELETE_PROFILE, RESULT_UNDEFINED_ERROR);
		}
		return;
	}


	offset = 3;// point the data Of BF33
	
	if(RecData[offset] == TAG_SearchCriteria_AID)
	{
		checkLen = 0x10;
	}
	else if(RecData[offset] == TAG_ICCID)
	{
		checkLen = 0x0A;
	}
	else 
	{
		checkLen = 0x00;   // 用于标识 tag 错误
	}
	
	if((P3 < 5) || (RecData[2] != (checkLen+2))||(RecData[offset+1] != checkLen))   //目前认为profile AID 一定只能是16字节
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);		//Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DELETE_PROFILE, RESULT_UNDEFINED_ERROR);
		}
		
		return;
	}
	
	if(RecData[offset] == TAG_SearchCriteria_AID)
	{
		
		targetProfileIndex = GetProfileIndexByAID(&RecData[offset+2], RecData[offset+1]);
	}
	else //if(RecData[offset] == TAG_ICCID)
	{		
		targetProfileIndex = GetProfileIndexByICCID(&RecData[offset+2]);
	}


	//if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(targetProfileIndex) == FALSE)
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_FOUND);//'6A 82': Profile not found
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DELETE_PROFILE,RESULT_ICCID_NOT_FOUND);
		}
		return ;
	}
	
	
	if(GetProfileState(targetProfileIndex) == PROFILE_STATE_ENABLED)  //f Profile is in Enabled state or the Profile Policy Rules does not allow deletion of the Profile, the eUICC SHALL return an erro
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_ALLOWED); //69 85': Profile not in disabled state or command not allowed by Profile Policy Rules. 
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DELETE_PROFILE,RESULT_PROFILE_NotINDisabledState);
		}
		return ;
	}

	if((GetSingleBYTEProfileMeta(targetProfileIndex,MetaDataProfilePolicyRules_INDEX,3)&PPRIDS_ppr2) == PPRIDS_ppr2)  //Indicator for PPR2 'Deletion of this Profile is not allowed'
	{
		if(commandCase3)
		{
			ThrowError(PROFILE_NOT_ALLOWED); //69 85': Profile not in disabled state or command not allowed by Profile Policy Rules. 
		}
		else
		{
			CUSTOMER_SendSingleBYTEResult(TAG_DELETE_PROFILE,RESULT_DISALLOWEDByPolicy);
		}
		return ;
	}
	
	transactionTag = IF_BeginTransaction();
		
	lastOperationalProfile = JcvmConfig_GetRSPLastOperationaProfile();//VMReadByte(EUICCLastOperationaProfile_Addr) ;
	
	if((lastOperationalProfile != ILLEGAL_PROFILEINDEX)&&(lastOperationalProfile == targetProfileIndex))
	{
		JcvmConfig_SetRSPLastOperationaProfile(ILLEGAL_PROFILEINDEX);//VMSetBuf(EUICCLastOperationaProfile_Addr,ILLEGAL_PROFILEINDEX,1);
	}
	
	Notification_GenerateOtherOperation(NOTIFICATION_DELETE, targetProfileIndex);
	
	VMSetBuf(PROFILE_VALIDTAG_CFG + targetProfileIndex, PROFILE_DELETELAG, 1);  
	
	IF_CommitTransaction(transactionTag);
	
	IF_ProfielDeletebackFileName(fileName);

	DeleteProfileDataByProfileIndex(targetProfileIndex, MODE_DELETE_PROFILE);
	
	IF_ProfielDeleteinit(fileName);

//	打垃圾回收标记 ？GarbageCollectorAndHeapPieceCleanUp();
	
	if(!commandCase3)
	{
		CUSTOMER_SendSingleBYTEResult(TAG_DELETE_PROFILE,RESULT_OK_);
	}
	
}
/*
*@function     : CUSTOMER_eUICCMemoryReset 
*
*@description  :This function deletes selected subsets of the Profiles stored on an eUICC regardless of their enabled status or any Profile Policy Rule
*
*@see          : <SGP22_V2.2 > 5.7.19  Function (ES10c): eUICCMemoryReset  P3 BF34 L 82 02 xxxx 
*                              3.3.2  eUICC Memory Reset          3.3.3  eUICC Test Memory Reset 
*@param        :
*
*@return       :void
*
*@RAM Resources:借用tempBuf[0..]，只在该函数内部有效
*
*@author       :xuxiaojun
*
*@hisotry      :created by xuxiaojun@20190102
*              :modify by xwh@20200529 调整删除profile的顺序，减少memoryReset过程中碎片整理次数
			   :modify by LCZ@20200628 添加设置UpdateChangeProfileInitPrivateFile私有文件初始化标志,GarbageCollectorAndHeapPieceCleanUp/HeapPieceCleanUp/SEEUICCVariableInit调用初始化,
*/
void CUSTOMER_eUICCMemoryReset(void)
{
	BYTE idata curEnableIndex, index,lastOperationaindex;
	BYTE idata resetOptions, profileClass;
	BYTE idata refreshTag, transactionTag;
	BYTE idata result;
	UWORD idata addr;
	BYTE xdata deleteProfileIndex[PROFILE_MAX_NUMBER+1];
	WORD idata ref;
	WORD idata fileName[PROFILEDELETE_BACKUPFILESYSTEM];
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *1. profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *2. GPload过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *3. GPstoredata过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 */
	if((IF_IsProfileLoadingSession() == TRUE)||(IF_CapLoaingInCurrnetProfile() == TRUE)||(IF_MoreCommandsInCurrnetProfile()==TRUE)) // add by chentao@20230112 profile 下载过程中不允许memoryReset
	{
		CUSTOMER_SendSingleBYTEResult(TAG_EUICCMEMORYRESET,RESULT_UNDEFINED_ERROR);

		return;
	}

	if((RecData[3] != 0x82) || (RecData[4] != 0x02) || (RecData[2] != 0x04) || ((RecData[2]+3) != P3))   // BF34 L 82 02 xxxx，确保数据域只有该TAG
	{
		CUSTOMER_SendSingleBYTEResult(TAG_EUICCMEMORYRESET,RESULT_UNDEFINED_ERROR);		 // undefinedError(127)
			
		return;
	}
	
	resetOptions = RecData[6];                           // deleteOperationalProfiles(0), deleteFieldLoadedTestProfiles(1), resetDefaultSmdpAddress(2)
	
	curEnableIndex = JcvmConfig_GetProfileEnableIndex();
	
	if(curEnableIndex != ILLEGAL_PROFILEINDEX)  //当前有激活的profile
	{
		if((STKPerform[0] == TRUE) ||( G_STKBusy != FALSE))  //if there is a proactive session ongoing, the eUICC SHALL terminate the EnableProfile command with error "catBusy". 
		{
			if(((resetOptions & 0x80) != 0) || ((resetOptions & 0x40) != 0)) // add by chentao @20230308 有主动式命令在执行的情况，可以执行SetDefaultDpAddress
			{
				CUSTOMER_SendSingleBYTEResult(TAG_EUICCMEMORYRESET,RESULT_CATBUSY);

				return;
			}
		}
	}

	transactionTag = IF_BeginTransaction(); // In case of any other error during the command execution, the command SHALL stop and SHALL leave the eUICC and the involved Profiles in their original states prior to command execution.

	result = RESULT_NOTHINGTODELETE;
	
	ecpMem_Set(deleteProfileIndex,0,PROFILE_MAX_NUMBER + 1);
	
	refreshTag = FALSE;


	lastOperationaindex = JcvmConfig_GetRSPLastOperationaProfile();

	for(index = 0; index < PROFILE_MAX_NUMBER; index++)   // 这里如果全部放在事务中的话，事务能力会不会不够？
	{
		if(VMReadByte(PROFILE_VALIDTAG_CFG + index) != PROFILE_VALIDFLAG)
		{
			continue;
		}
		
		profileClass = GetSingleBYTEProfileMeta(index, MetaDataProfileClass_INDEX,2) ;  //获取profile的类别
		
		if(((profileClass == PROFILE_CLASS_OPERATIONAL)&&((resetOptions & 0x80) == 0x80))   // Operational Profiles
			|| ((profileClass == PROFILE_CLASS_TEST)&&((resetOptions & 0x40) == 0x40)))     // Test Profiles that were not pre-installed，目前没有区分test profile预置还是后下载，全部当成后下载来处理的
		{			
			if(curEnableIndex == index)  // If there was an Enabled Profile, the ISD-R SHALL send a REFRESH proactive command to the Device.
			{
				refreshTag = TRUE;
				
				JcvmConfig_SetProfileEnableIndex(ILLEGAL_PROFILEINDEX);

				JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);//memoryreset时候 如果当前有激活Profile
			}

			if(lastOperationaindex == index)
			{
				JcvmConfig_SetRSPLastOperationaProfile(ILLEGAL_PROFILEINDEX);// add by chentao @20230323 记录的LastOperationa profile被删除了 需要修改配置区
			}

			deleteProfileIndex[0] ++;
			deleteProfileIndex[deleteProfileIndex[0]] = index;
				
			VMSetBuf(PROFILE_VALIDTAG_CFG + index, PROFILE_DELETELAG, 1);     // 在本次循环中，将打标记放在一个事务中，如果中途发生断电，需要调用DeleteProfileResume函数将删除操作执行完毕
			                                                                  // non-reversible way in case of external interruptions (e.g. power loss): the eUICC SHALL continue the processing of that command upon the next eUICC power on.
			Notification_GenerateOtherOperation(NOTIFICATION_DELETE, index);  // For each deleted Profile, the eUICC SHALL generate as many Notifications as configured in its Profile Metadata (notificationConfigurationInfo) in the format of OtherSignedNotification.
			
			result = RESULT_OK_;
		}
	}
	
	if((resetOptions & 0x20) == 0x20)    // resetDefaultSmdpAddress(2)
	{
		//addr = GetAddrByIndexAddr(EUICCSMDPDefaultAddress_RefInitial_Addr);  //Lmax LV 由于Default SM-DP+ address个人化时是设置成initial value， 所以这里这两个地址都是存在的，且Default SM-DP+ address的空间足够
		ref = memReadU16(MEMtPM8(EUICCSMDPDefaultAddress_RefInitial_Addr));//LV结构
		
		if(ref != REF_NULL)
		{
			addr = get_array_data(MEMtPF8(locateObjectInMemory(ref)));

			RecData[0] = VMReadByte(addr);
			
			VMReadBuf(&RecData[1], addr + 1, RecData[0]);
		}
		else
		{
			RecData[0]  = 0;
		}
		
		result = SetDefaultDpAddress_MemoryReset(&RecData[1], RecData[0]);	
	}	
	
	if(result == RESULT_UNDEFINED_ERROR)
	{
		IF_AbortTransaction(transactionTag);
	}
	else
	{
		IF_CommitTransaction(transactionTag);
	}

	if(result == RESULT_OK_)  //只有当数据执行成功后，才去进行删除等操作
	{
		if(refreshTag != TRUE) //modify by chentao@20230313不删除当前激活的profile，需要备份文件信息
		{
			IF_ProfielDeletebackFileName(fileName);
		}

		G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;

		for(index = 0; index < deleteProfileIndex[0]; index++)
		{
			DeleteProfileDataByProfileIndex(deleteProfileIndex[deleteProfileIndex[0]-index], MODE_DELETE_PROFILE);
		}

		if(refreshTag != TRUE) //不删除当前激活的profile，需要还原文件信息
		{
			IF_ProfielDeleteinit(fileName);
		}
	}

	CUSTOMER_SendSingleBYTEResult(TAG_EUICCMEMORYRESET,result);

	if(result == RESULT_OK_)
	{
		if(refreshTag == TRUE)
		{
// add by chentao@20230209 CUSTOMER_eUICCMemoryReset 删除所有profile后，初始化将所有变量切到SE   begin
			CUSTOMER_ChangeProfileNoReset(ILLEGAL_PROFILEINDEX, MODE_MEMORY_RESET, refreshTag);
// add by chentao@20230209 CUSTOMER_eUICCMemoryReset 删除所有profile后，初始化将所有变量切到SE   end
			IF_SendRefreshCommandOfUICCResetMode();
		}
	}
}
/*
*@function     :CUSTOMER_GetEID 
*
*@description  :This function gets the EID from the eUICC. 该条指令只能是BF3E035C015A
*
*@see          :<SGP22_V2.2 > 5.7.20  Function (ES10c): GetEID 
*
*@param        :void
*
*@return       :void
*
*@test         :自测通过
*
*@RAM Resources:响应数据直接放在RecData中发送出去
*
*@author       :xuxiaojun@20181112
*
*@hisotry      :created by xuxiaojun@20181112
*/
void CUSTOMER_GetEID(void)
{
	if((P3 != 0x06) || (RecData[2] != 0x03) || (RecData[3] != 0x5C) || (RecData[4] != 0x01) || (RecData[5] != 0x5A))   // BF3E 03 5C015A，确保数据域只有该TAG
	{
		ThrowError(SW_WRONG_DATA);               // tagList [APPLICATION 28] Octet1  -- tag '5C', the value SHALL be set to '5A'
	}
	
	RecData[0] = 0xBF;
	
	RecData[1] = 0x3E;
	
	RecData[2] = 0x12;
	
	RecData[3] = 0x5A;
	
	VMReadBuf((BYTE*)&RecData[4], EUICCEIDInfo_Addr, 0x11); //这一步获取时，已经假定是发卡状态，且EID数据已经被写入成功。EID的合法性在写入时进行判断，所以这里只进行读取操作，不判断合法性	
	
	SetOutgoingAndSend(0x05,RecData[2] + 3);	
}
/*
*@function     :CUSTOMER_SetNickname 
*
*@description  :This function is used to add or update a Profile Nickname associated to one Profile present on-card
*
*@see          :<SGP22_V2.2 > 5.7.21  Function (ES10c): SetNickname   BF29
*              SetNicknameRequest ::= [41] SEQUENCE { -- Tag 'BF29' 
*                 iccid Iccid, 
*                 profileNickname [16] UTF8String (SIZE(0..64)) 
*              }
*
*@param        :void
*
*@return       :void
*
*@test         :自测通过
*
*@RAM Resources:tempBuf[0...], 数据长度 > 0xFF时，需要用tempBuf缓存EUICCTempDataBuf的数据后写到E方
*
*@author       :xuxiaojun@20181116
*
*@hisotry      :created by xuxiaojun@20181116
*              :modify by xuxiaojun@20190708 书写规范化，用Profile_NickName_MAX_LEN替换256；优化代码
*              :modify by xuxiaojun@20221012 缺陷412的处理，反复setnickname时抛6A84的问题，修改方案，第一次setNickName时就创建一个最大对象，后续只需要修改对象内容。存储格式为TLV
*/
void CUSTOMER_SetNickname(void)
{
	BYTE idata transactionTag, offset, lengthOfLen, profileIndex;
	WORD idata nickNameRef;
	UWORD xdata nickNameAddr, metaDataAddr, dataStartAddr, totalLen, /*freeNickNameLen, */allocNickNameLen, curNickNameSize;
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *1. profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *2. GPload过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *3. GPstoredata过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 */
	if((IF_IsProfileLoadingSession() == TRUE)||(IF_CapLoaingInCurrnetProfile() == TRUE)||(IF_MoreCommandsInCurrnetProfile()==TRUE)) // add by chentao@20230112 profile 下载过程中不允许SetNickname
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME,RESULT_UNDEFINED_ERROR);

		return;
	}

	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)
	{
		dataStartAddr = (UWORD)RecData;
	}
	else
	{
		dataStartAddr = EUICCTempDataBuf; //测试用例：联通buglist/SetNicknameTest02 有测profileNickname的长度为256的情况
	}
	
	offset = ASN1_GetBERTlvLen(dataStartAddr+2, (UWORD*)&totalLen); 
	
	if(totalLen == 0x00)
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_UNDEFINED_ERROR);
			
		return;
	}
	
	offset += 2;                // 偏移到BF29的V域, totalLen是BF29域的总长度
	
	totalLen += offset;         // 指令的全部长度 BF29 L 5ALV 90 LV	
	
	if(totalLen != G_CustomerStoreDataApdu.storeDataLen)          // 不能存在BF29以外的其他TAG
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_UNDEFINED_ERROR);
			
		return;
	}
	
	if((VMReadByte(dataStartAddr+offset) != TAG_ICCID) || (VMReadByte(dataStartAddr+offset+1) != ICCID_LEN))   // ICCID  is  defined  according  to  ITU-T  recommendation  E.118 [21] , with the derivation that up to 20 digits are allowed.
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_UNDEFINED_ERROR);   // 这里是否需要判断ICCID的长度必须是10字节？
		
		return;
	}

	profileIndex = GetProfileIndexByICCID((BYTE*)(dataStartAddr+offset+2));

	if(ProfileIndexIsValid(profileIndex) == FALSE)
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_ICCID_NOT_FOUND);
		return;
	}
	
	offset += (VMReadByte(dataStartAddr+offset+1) + 2); //偏移到TAG_NickName(0x90) TAG_ICCID + ICCID_LEN + VALUE
	
	if((offset == totalLen) || ((offset + 2 )> totalLen)) // 只有ICCID，或者存在其他数据，但是数据无效
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_ICCID_NOT_FOUND);
		
		return;
	}
		
	if(VMReadByte(dataStartAddr+offset) != TAG_NickName) //不管是set, update, remove，TAG_NickName都必须存在
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_ICCID_NOT_FOUND);
		
		return;
	}
	
	metaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex*sizeof(WORD));

	nickNameAddr = GetMetaData_profileNickNameRefValueAddr(metaDataAddr);//NickNameRef数组的内容指向的地址

	lengthOfLen = ASN1_GetBERTlvLen(dataStartAddr+offset+1, (UWORD*)&curNickNameSize); //获取0x90的L域占用的字节数以及值，用于后续空间分配
	
	if(curNickNameSize > Profile_NickName_MAX_LEN)
	{
		CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_UNDEFINED_ERROR);			
			
		return;
	}
		
	if(curNickNameSize != 0) // In case the new value is an empty string, the Profile Nickname SHALL be removed 指令数据域编码如下：BF290e5A0a989209012143658709F59000
	{
//		allocNickNameLen = (allocNickNameLen + lengthOfLen + 1);                  //需要分配的空间大小
		allocNickNameLen = (Profile_NickName_MAX_LEN + 2 + 1);          //modify by xuxiaojun@20221011: 缺陷412， 修改方案，避免每次setNickName时都需要释放再分配，直接分配一个最大值，然后每次直接修改值即可

		transactionTag = IF_BeginTransaction();

		if(nickNameAddr == ADDR_IS_NULL) // 由于最大值已经被定义为Profile_NickName_MAX_LEN，可以考虑在一开始的时候按照最大值创建。不过会出现卡内不存在这个的情况时导致空间浪费
		{
			nickNameAddr = MakeProfileArrayObject(getISDPISDRContext(ISDP_PkgId,ISDP_APPID),allocNickNameLen,&nickNameRef,profileIndex);

			if(nickNameAddr == ADDR_IS_NULL)
			{
				CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_UNDEFINED_ERROR);

				IF_AbortTransaction(transactionTag);

				return;
			}

			memWriteU16Atomic(MEMtPM8(GetMetaData_profileNickNameRefAddr(metaDataAddr)), nickNameRef);  // 把引用写入
		}

		VMReadBuf(&tempBuf[0], dataStartAddr+offset, (curNickNameSize + lengthOfLen + 1)); //数据源可能是EUICCTempDataBuf，可能是RecData, 所以先缓存到tempBuf中.  NickName的len + NickName长度的len + tag的len, 前面已经判断allocNickNameLen不大于0x100，所以这里不会越界

		VMWriteBuf(nickNameAddr, (BYTE*)&tempBuf[0], (curNickNameSize + lengthOfLen + 1)); // 把数据写入

		IF_CommitTransaction(transactionTag);
	}
	else
	{
		memWriteU16Atomic(MEMtPM8(GetMetaData_profileNickNameRefAddr(metaDataAddr)), REF_NULL);
	}
	
	CUSTOMER_SendSingleBYTEResult(TAG_SET_NICK_NAME, RESULT_OK_);
}
/*
*@function     : CUSTOMER_GetRAT 
*
*@description  :This function retrieves the Rules Authorisation Table (RAT) from the eUICC. It can be called at any time
*
*@see          : <SGP22_V2.2 > 5.7.22  Function (ES10b): GetRAT   
*
*@param        :void
*
*@return       :void, 发送出去的响应数据格式如下
*              BF43 61
*                   A0 5F
*                      30 1D 80020640 A1133011800392F928810447534D418204525350FF 82020780
*                      30 11 80020640 A1073005800392F948 82020780
*                      30 14 80020520 A10B3009800392EEEE81008200 820100
*                      30 15 80020560 A10B30098003EEEEEE81008200 82020780 
*
*@RAM Resources:未考虑RAT长度超过0xFF的情况，所以响应数据直接放在RecData中发送出去
*
*@author       :xuxiaojun@20181108
*
*@hisotry      :created by xuxiaojun@20181108
*/
void CUSTOMER_GetRAT(void)
{
	WORD idata PPARRef, RATOffset, PPARTotalLen,curPPARArrayRef;
	UWORD xdata curPPARAddr, curPPARArrayAddr;
	BYTE idata currentPPARLen;   
	
	if((RecData[2] != 0) || (P3 != 3))         // BF43 00
	{
		ThrowError(SW_WRONG_DATA);             // SGP.22 5.7.22  Function (ES10b): GetRAT  -- No input data
	}
	
	RATOffset = 0x09; // BF43 82XXXX A0 82XXXX
	
	curPPARAddr = EUICCRATRef_Addr;   
	
	PPARRef = memReadU16(MEMtPM8(EUICCRATRef_Addr));//REF_NULL;
	
	while(PPARRef != REF_NULL)  //找到RAT链表中最后一个PPAR
	{
		curPPARAddr = locateObjectInMemory(PPARRef);//RATTableRef对象的地址

		curPPARArrayAddr = GetObjectItemAddr(curPPARAddr,PPARArray_INDEX);//存PPARArray[]的地址

		curPPARArrayRef = memReadU16(MEMtPM8(curPPARArrayAddr));

		curPPARArrayAddr = locateObjectInMemory(curPPARArrayRef);//PPARArray[]数组地址 指向数组头

		currentPPARLen = get_array_length(curPPARArrayAddr);

		curPPARArrayAddr = get_array_data(MEMtPM8(curPPARArrayAddr));

		if(currentPPARLen > sizeof(tempBuf))
		{
			continue; //modify by xuxiaojun@20230314:当前ppar长度超过tempBuf大小
		}
		VMReadBuf(tempBuf,curPPARArrayAddr,currentPPARLen);

		if((RATOffset + currentPPARLen) > EUICCTempDataBufSize)
		{
			break; //modify by xuxiaojun@20230314: EUICCTempDataBuf越界，舍弃后续的PPAR
		}

		VMWriteBuf(EUICCTempDataBuf + RATOffset, tempBuf, currentPPARLen);

		RATOffset += currentPPARLen;

		curPPARAddr = GetObjectItemAddr(curPPARAddr,nextRATTable_INDEX);//下一个RATTable

		PPARRef = memReadU16(MEMtPM8(curPPARAddr));
	}
	
	PPARTotalLen = RATOffset - 0x09;
	
	currentPPARLen = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0], 0xA0, PPARTotalLen); //AUTOMATIC TAGS ，根据规范 ITU-T X.680 ，AUTOMATIC TAGS为context- specific 类型 ，而且RAT是结构化编码，tag number从0开始，所以此处 tag 为0xA0： BIT8,BIT7 是标签类，10表示上下文特定；BIT6是编码类型，为1表示结构化编码（0表示原始编码）；BIT5..BIT1是标签编号00
	
	VMWriteBuf(EUICCTempDataBuf + RATOffset - PPARTotalLen - currentPPARLen , tempBuf, currentPPARLen);
	
	PPARTotalLen += currentPPARLen;
	
	currentPPARLen = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0], TAG_GET_RAT, PPARTotalLen);
	
	VMWriteBuf(EUICCTempDataBuf + RATOffset - PPARTotalLen - currentPPARLen , tempBuf, currentPPARLen);
	
	PPARTotalLen += currentPPARLen;
	
	SetSendFileExtendInfo(EUICCTempDataBuf + RATOffset - PPARTotalLen, PPARTotalLen);	
}

/*
*@function     :CUSTOMER_GetEUICCChallenge 
*
*@description  :This function initiates a RSP session between an RSP Server and the ISD-R
*
*@see          :<SGP22_V2.2 > 5.7.7  Function (ES10b): GetEUICCChallenge   BF2E00
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:No
*
*@author       :xuyz
*
*@hisotry      :V1.0 :create @20190102
*/
void CUSTOMER_GetEUICCChallenge(void)
{
	BYTE idata offset;

	/**
	 *modify by chentao@20230315: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *1. profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *2. GPload过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 *3. GPstoredata过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作
	 */
	if(IF_CapLoaingInCurrnetProfile() || IF_MoreCommandsInCurrnetProfile())   // 在当前激活的profile下进行卡片内容管理时，不允许开启一个新的下载profile流程（COS内部方案 无规范来源）
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
			
	if(P3 != 0x03)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(RecData[2] != 0x00)   //BF 2E 00
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_NULL)   //The LPA SHOULD NOT initiate a new Profile download and installation procedure while there is an active download RSP session, IF
	{	
		/*
		 需求： 会话被打断时，已经产生了otPK/otSK.EUICC.ECKA，但是还未使用，此时将otPK/otSK.EUICC.ECKA + SMDP OID保存下来，供下次会话使用
		
		 处理： 该处的需求不需要特殊处理，在 InitialiseSecureChannel  中使用完之后，将  EUICC_OpKeyBuf 的长度 字段清空即可，不需要做额外处理	
		*/

		if(G_RSPSessionContext.sessionSate >= RSPSessionState_ConfigureISDP)  //当前已经在创建ISD-P 设置参数等操作，需要回滚这些操作
		{
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
			{
				GPLoadInit();//需要调restore回收正在下载的包？
			}
			
			DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);  //回滚ISD-P   

//	打垃圾回收标记 ？GarbageCollectorAndHeapPieceCleanUp();
			
		}
		
	}
	
	ecpMem_Set((BYTE *)&G_RSPSessionContext, 0x00, sizeof(RSPSessionContext));  //重新清空session context

	G_RSPSessionContext.downloadProfileIndex = ILLEGAL_PROFILEINDEX;

	G_RSPSessionContext.channelNum = 0xFF;

#if EUICC__FUNCTION__GUOMI_eSIM
	G_CertType = Cert_ECC; //将G_CertType恢复初始值
#endif

	offset = 0;
	
	RecData[offset++] = 0xBF;
	
	RecData[offset++] = 0x2E;
	
	RecData[offset++] = 0x12;    //L
	
	RecData[offset++] = 0x80;   //AUTOMATIC TAGS ，根据规范 ITU-T X.680 ，AUTOMATIC TAGS为context- specific 类型 ，tag number从0开始，所以此处 tag 为0x80
	
	RecData[offset++] = 0x10;  
	
	Random_number_generator(&RecData[offset], 0x10,ALG_SECURE_RANDOM);
	
	G_RSPSessionContext.sessionSate = RSPSessionState_Init;

	G_RSPSessionContext.channelNum = selected_channel;

	ecpMem_Copy((BYTE *)&G_RSPSessionContext.channllengeTransactionId[0], &RecData[offset], 0x10);   //拷贝10字节的随机数
	
	offset += 0x10;
	
	SetOutgoingAndSend(0x05,offset);	
}
/*
*@function     : CUSTOMER_AuthenticateServer 
*
*@description  :This function performs the authentication of the RSP Server by the eUICC
*
*@see          : <SGP22_V2.2 > 5.7.13  Function (ES10b): AuthenticateServer 
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:No
*
*@author       :xuyz
*
*@hisotry      :V1.0 create @20190102 
*              :V1.1 modify by xuxiaojun@20190621 代码书写规范
*              :V1.2 modify by xuxiaojun@20190711 增加ctxParams1Len长度为0的非法处理
*/
BYTE code authServerSeqTag[0x06] = {0x30,0x5F, 0x37, 0x04,0x30, 0xA0}; //tag 30: serverSigned1  + tag 5F37: serverSignature1  + tag 04:euiccCiPKIdToBeUsed  + tag 30 : serverCertificate +tag A0 ctxParams1
void CUSTOMER_AuthenticateServer(void)
{
	BYTE idata i,j,flag,tmp;
	BYTE idata tmpTag[2];
	WORD idata certifiLen;
	WORD xdata dataOff[5] ;  //dataOff[0] - dataOff[4]分别表示serverSigned1 serverSignature1 euiccCiPKIdToBeUsed serverCertificate ctxParams1在输入数据中的偏移
	UWORD xdata  offset,tmpLen,dataStartAddr, certifiAddr,errorAddr,ctxParams1Len;
	
	BYTE xdata serverChallange[0x12], getCERTRole[0x08];
		
	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)
	{
		dataStartAddr = (UWORD)RecData;
	}
	else
	{
		dataStartAddr = EUICCTempDataBuf;
	}
	
	dataStartAddr += 2;  //point the length of BF38
	
	tmp = ASN1_GetBERTlvLen(dataStartAddr, (UWORD*)&tmpLen);  //BF38 的长度
	
	errorAddr = dataStartAddr+ tmp;  //指向serverSigned1 ServerSigned1字段的起始位置
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_Init)   //原则上 AuthenticateServer 命令只能在 收到 Get Challenge 之后执行
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,AuthenticateError_noSessionContext,0xBF38);
		return; 
	}
	
	offset = tmp;  //指向第一个数据
	
	for(i=0,j =0; i< 5; i++, j++)
	{
		flag = 0x00;
		
		VMReadBuf(tmpTag, dataStartAddr+offset, 0x02);
		
		if(tmpTag[0] != authServerSeqTag[j])   //AuthenticateServerRequest中的有的tag不对
		{
			AuthenticateServerPrepareDownload_SendResponseError(errorAddr,AuthenticateError_undefinedError,0xBF38);
			return; 
		}
		if(tmpTag[0] == 0x5F)
		{
			if(tmpTag[1] != 0x37)
			{
				AuthenticateServerPrepareDownload_SendResponseError(errorAddr,AuthenticateError_undefinedError,0xBF38);
				return; 
			}
			
			j++;
			
			flag = 0x01;
		}
		dataOff[i] = offset;
		
		tmp = ASN1_GetBERTlvLen(dataStartAddr + offset + 1 + flag, (UWORD*)&tmpLen);
		
		offset += (tmp + tmpLen + 1 + flag);  
	}
	
	for(i=0 ; i<5 ; i++)
	{
		if(dataOff[i] == 0x00)   //必选tag 有不存在的
		{
			AuthenticateServerPrepareDownload_SendResponseError(errorAddr,AuthenticateError_undefinedError,0xBF38);
			return;
		}
		
	}
	
	tmp = ASN1_GetBERTlvLen((UWORD)(dataStartAddr+dataOff[4]+1), (UWORD*)&ctxParams1Len);  //获取ctxParams1 字段数据体的长度
	
	if(ctxParams1Len == 0x00) 
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,AuthenticateError_undefinedError,0xBF38);
		return;
	}
	
	ctxParams1Len += (tmp + 1);  //ctxParams1Len为整个ctxParams1的长度，包括TLV的总长度
	
	if(ctxParams1Len > Common_Auth_CtxParams1_MAX_TLV_LEN)  //卡片暂时不支持超过255字节长度的ctxParams1的字段
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,AuthenticateError_undefinedError,0xBF38);
		return;
	}
	
	tmp = AuthenticateServer_CheckServerSigned1(dataStartAddr + dataOff[0], serverChallange);  
	
	if(tmp != RESPONSE_NoError)
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,tmp,0xBF38);
		return;
	}
	
	tmp = ASN1_GetBERTlvLen(dataStartAddr+dataOff[3]+1, (UWORD*)&tmpLen);  //Certificate  证书体的长度
	
	certifiLen = tmpLen + tmp + 1;
	
#if EUICC__FUNCTION__GUOMI_eSIM
	if(RESPONSE_NoError == TBSCertificate_GetField((BYTE *)(dataStartAddr + dataOff[3]), certifiLen, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm, &tempBuf[0])) //CERT.DPpb.SM2SIG  取出AlgorithmIdentifier.algorithm字段的值
	{
		if((sizeof(SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI) == tempBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[0], &tempBuf[1], tempBuf[0])))
		{
			G_CertType = Cert_GUOMI;
		}
	}
#endif

	tmp = Certificate_VerifyValidity((BYTE *)(dataStartAddr + dataOff[3]), certifiLen, RSPROLE_DP_AUTH);  //在authenserver中参数为RSPROLE_DP_AUTH 或RSPROLE_DS_AUTH 都表示同样的意思，因为此时还无法区分下行业务是DP 的还是DS 的
	
	if(tmp != RESPONSE_NoError)  //证书无效，退出 ，Verify the validity of the RSP Server Certificate for authentication (using the ECASD service), as described in section 4.5.2.2 using the PK.CI.ECDSA identified by the Authority Key Identifier contained in the RSP Server Certificate
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,tmp,0xBF38);
		return;
	}
	
	certifiAddr = ADDR_IS_NULL;
	
	tmp = AuthenticateServer_CheckEuiccCiPKIdToBeUsed(dataStartAddr + dataOff[2] , &certifiAddr);  //certifiAddr 从非空开始为有效期
	
	if(tmp != RESPONSE_NoError) // verify that euiccCiPKIdToBeUsed is supported and related credentials are available for signing
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,tmp,0xBF38);
		return;
	}
	
	tmp = ASN1_GetBERTlvLen(dataStartAddr+dataOff[0]+1, (UWORD*)&tmpLen);  //serverSigned1  证书体的长度
	
	tmpLen += (tmp + 1);  //serverSigned1字段的总长度
	
	i  = ASN1_GetBERTlvLen(dataStartAddr+dataOff[1]+2, (UWORD*)&offset);   //serverSignature1
		
	tmp = AuthenPrepareDownload_VerifyDpSignature((BYTE *)(dataStartAddr + dataOff[3]), certifiLen, dataStartAddr + dataOff[0], tmpLen ,dataStartAddr + dataOff[1]+2+i, offset, RSPROLE_DP_AUTH);

	if(tmp != RESPONSE_NoError)
	{
		AuthenticateServerPrepareDownload_SendResponseError(errorAddr,tmp,0xBF38);
		return;
	}
	
	G_RSPSessionContext.sessionSate = RSPSessionState_Auth;
	
	TBSCertificateExtensions_GetField((BYTE *)(dataStartAddr + dataOff[3]), certifiLen, id_ce_certificatePolicies,tempBuf,NULL);  //4.5.2.2 The extension 'Certificate Policies' SHALL be set with one of the following OID: id-rspRole-dp-auth' to indicate an SM-DP+ Certificate for authentication with the eUICC.

	if(tempBuf[0] == 0x07)
	{
		ecpMem_Copy(&getCERTRole[0], &tempBuf[0], 8);
	}

	tmpLen = AuthenticateServer_SendResponseSuccess(dataStartAddr+dataOff[4], ctxParams1Len, serverChallange, certifiAddr);//内部会使用到G_RSPSessionContext，所以该流程不能放到后面做 只能在清G_RSPSessionContext前拼装
	
	if(RSPROLE_DS_AUTH == Certificate_GetCERTRole(getCERTRole))  // .6.2  Event Retrieval 使用DSauth   ,此时的流程不属于prepare准备阶段，需要清空rsp会话的一些状态信息。测试用例：UL套件TC_4_2_18_2_5_01_TC_eUICC_ES10b_AuthenticateServer_SM_DS_BRP
	{
		RSP_ClearSession();
	}

	if(tmpLen >= 0x100)
	{
		ThrowError(0x6100);
	}
	else
	{
		ThrowError(0x6100|tmpLen);
	}
	
}

/*
*@function     : CUSTOMER_PrepareDownload 
*
*@description  :This function initiates a Bound Profile Package download after a successful authentication of an SM-DP+
*
*@see          : <SGP22_V2.2 > 5.7.5  Function (ES10b): PrepareDownload  
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:
*
*@author       :xuyz
*
*@hisotry      :V1.0 create @20180102
*              :V1.1 modify by xuxiaojun@20190709 增加hashCc Octet32 OPTIONAL的长度非法性判断
*               V1.2 modify by chentao@20200811 修改是否支持EUICC__REUSE_OTPK可配置
*/
BYTE code prepareDownloadTag[0x04] = {0x30, 0x00,  0x04, 0x30};
void CUSTOMER_PrepareDownload(void)
{
	BYTE idata tmp,i,tag[2],result, tagLen;
	WORD idata off, DPPbcertLen;
	WORD idata dataOff[4];  //dataOff[0] :  smdpSigned2's off  dataOff[1] :  smdpSignature2's off dataOff[2] :  hashCc's off dataOff[3] :  smdpCertificate's off
	UWORD xdata tmpLen,requestTotalLen,dataAddr;
	
	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)
	{
		dataAddr = (UWORD)RecData;
	}
	else
	{
		dataAddr = EUICCTempDataBuf;
	}
	
	tmp = ASN1_GetBERTlvLen(dataAddr + 2, (UWORD*)&requestTotalLen);  //获取PrepareDownloadRequest的总长度
	
	ecpMem_Set((BYTE *)dataOff, 0x00, sizeof(dataOff));   //清空各字段的偏移
	
	dataOff[0] = 0x02 + tmp;  //dataOffset[0] 表示the first tag
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_Auth)
	{		
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],ErrorCode_noSessionContext, 0xBF21);
				
		return;
	}
		
	tagLen = 0x00;
	
	for(off = 0,i = 0; (off<requestTotalLen)&&(i<4);  )
	{
		tagLen = ASN1_GetBERTlvTag((UWORD)(dataAddr + dataOff[i]), tag);
		
		if(tagLen == 0x02)  //两字节的tag
		{
			if((i != 0x01) ||(tag[0] != 0x5F) || (tag[1] != 0x37))
			{
			    AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);
				
				return;
			}	
		}
		else
		{			
			if(i == 0x01)
			{
			    AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);
				
				return;
			}
			
			if(prepareDownloadTag[i] != tag[0])
			{
				if(i == 0x02)
				{
					i++;
					dataOff[3] = dataOff[2];
					dataOff[2] = 0;
					continue;
				}
				else
				{
					AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);
				
					return;
				}
			}	
		}
		
		tmp = ASN1_GetBERTlvLen(dataAddr + dataOff[i] + tagLen, (UWORD*)&tmpLen);
		
		if((i == 2)&&(tmpLen != PrepareDownload_hashCc_Len)) // hashCc Octet32 OPTIONAL, -- Hash of confirmation code
		{
			AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);
				
			return;
		}
		
		if(i < 3)
		{
			dataOff[i+1] = (dataOff[i] + tmp + tmpLen + tagLen);
		}
		
		off += (tmp + tmpLen + tagLen);
			
		i++;
	}
	
	if(off != requestTotalLen)   //存在多余字段
	{
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);
				
		return;
	}
	
	for(i=0; i< 4; i++)
	{
		if((i != 2)&&(dataOff[i] == 0x00))   //必须存在的tag不存在
		{
			AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);
				
			return;
		}
	}
	
	result = PrepareDownload_VerifySmdpSigned2(dataAddr+dataOff[0],tag);	// tag[0] Indicates if the Confirmation Code is required 
																			// tag[1] = 0, generate a new one-time key pair ,else ```
	if( result != RESPONSE_NoError)
	{
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],result, 0xBF21);
		
		return;
	}
	
	if((tag[0] != FALSE)&&(dataOff[2] == 0))//Indicates if the Confirmation Code is required 
	{
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],DownloadErrorCode_undefinedError, 0xBF21);//Verify that the Hashed Confirmation Code is provided by the LPAd if the Confirmation Code Required Flag is set in smdpSigned2
		
		return ;
	}
	
	tmp = ASN1_GetBERTlvLen(dataAddr+dataOff[3]+1, (UWORD*)&tmpLen);  //获取证书的长度域，和 证书长度本身所占的字节数
	
	DPPbcertLen = tmpLen+tmp+1; //证书总长度 包含tag 30
	
	result = Certificate_VerifyValidity((BYTE *)(dataAddr+dataOff[3]), DPPbcertLen,RSPROLE_DP_PB);  //验证证书相关数据的有效性：Verify the validity of the CERT.DPpb.ECDSA (using the ECASD service) as defined in section 4.5.2.2. 
	
	if(result != RESPONSE_NoError)  //证书有效性验证失败
	{
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],result, 0xBF21);
		
		return;
	}
	
	tmp = ASN1_GetBERTlvLen(dataAddr+dataOff[0]+1, (UWORD*)&tmpLen);
	
	tmpLen += (tmp+1);  // 字段的总长度 加上30 tag字段的长度
	
	tmp = ASN1_GetBERTlvLen(dataAddr+dataOff[1]+2, (UWORD*)&requestTotalLen);  //smdpSignature2数据体的长度
	
	result = AuthenPrepareDownload_VerifyDpSignature((BYTE *)(dataAddr+dataOff[3]), DPPbcertLen, dataAddr+dataOff[0], tmpLen,dataAddr+dataOff[1]+2+tmp, requestTotalLen,RSPROLE_DP_PB);
	
	if(result != RESPONSE_NoError)
	{
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],result, 0xBF21);
		
		return;
	}


	TBSCertificateExtensions_GetField((BYTE *)(dataAddr + dataOff[3]), DPPbcertLen, id_ce_subjectAltName, &tempBuf[128],NULL);
	if (EUICC__REUSE_OTPK)
	{
		if(tag[1] == 1)//下行中带了一组临时密钥 密钥值和已缓存的密钥一致   还需要比较OID
		{
			if(NvmCmp((BYTE *)(EUICC_OpKeyBuf + EUICC_OpKeyOID_Off), &tempBuf[128], tempBuf[128]+1) != EQUAL)//比较上次产生密钥时的OID和本次CERT DP ECDSA的OID是否一致 如果不一致 需要重新产生密钥对
			{
				tag[1] = 0; //需要产生密钥对
			}
		}
	}
	else
	{
		tag[1] = 0;
	}

	if(tag[1] == 0)	//产生密钥对
	{
		TBSCertificate_GetField((BYTE *)(dataAddr + dataOff[3]), DPPbcertLen,FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters,&tempBuf[0]); //CERT.DPpb.ECDSA  取出曲线参数 1字节  LV结构 tempBuf[1]存曲线参数

#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
			tempBuf[2] = 0x04;

			SM2_ECP_GetKey(&tempBuf[68], &tempBuf[3], false);
		}
		else
#endif
		{
			if(GenECCKeyPair(&tempBuf[2], &tempBuf[68], tempBuf[1]) != TRUE)
			{
				AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],ErrorReason_installFailedDueToUnknownError, 0xBF21);

				return ;
			}
		}
		
		tempBuf[0] = 99 + tempBuf[128] + 1; //totalLen(1字节)    --> pkref(1字节) + pkLen(1字节) +  publicValue(64字节) + skLen(1字节) + skValue(32字节) + smOIDLen(1字节) + smOIDValue(最大41字节长)
										 //tempBuf[1] 曲线参数
		tempBuf[2] = 64;				 //pkLen(1字节) 
										 //pk  64字节
		tempBuf[67] = 32;				 //skLen(1字节)
										 //sk  32字节
		tempBuf[100] = tempBuf[128];		 //smOIDLen(1字节)
		
		ecpMem_Move(&tempBuf[101],&tempBuf[129],tempBuf[128]);	 //OID
		
		VMWriteBuf(EUICC_OpKeyBuf,&tempBuf[0],tempBuf[0] + 1);//将曲线参数 OPKey 和 OID 一起写入E2中
	}
	
	requestTotalLen = PrepareDownload_GenerateEuiccSigned2(dataAddr,&tempBuf[0],dataOff);//Generate euiccSigned2

	
	VMReadBuf(&tempBuf[requestTotalLen], (dataAddr + dataOff[1]),67);//将smdpSignature2拼装到euiccSigned2后面       euiccSignature2 SHALL apply on the concatenated data objects euiccSigned2 and smdpSignature2.
	
	tmpLen =  PrepareDownload_GenerateEuiccSignature2(tempBuf,requestTotalLen + 67 ,&tempBuf[requestTotalLen]);//Generate the euiccSignature2  with the SK.EUICC.ECDSA related to the CERT.EUICC.ECDSA as requested by SM-DP+.
	
	if(tmpLen == FALSE)//计算的时候出错
	{
		AuthenticateServerPrepareDownload_SendResponseError(dataAddr+dataOff[0],ErrorReason_installFailedDueToUnknownError, 0xBF21);
			
		return ;
	}
	
	requestTotalLen += tmpLen;
	
	G_RSPSessionContext.sessionSate = RSPSessionState_Prepare;
	
	RecData[0] = 0xBF;
	RecData[1] = 0x21;
	RecData[2] = 0x81; //EuiccSigned2 大于64字节 EuiccSignature2 大于64字节
	RecData[3] = requestTotalLen + 3;
	RecData[4] = 0xA0;  //tag  -- PrepareDownloadResponseOk ::= SEQUENCE
	RecData[5] = 0x81;
	RecData[6] = requestTotalLen;
	
	ecpMem_Copy(&RecData[7],tempBuf,requestTotalLen);
	
	SetOutgoingAndSend(0x05,(short)(requestTotalLen+7)); 
	
}
/*
*@function     : CUSTOMER_CancelSession 
*
*@description  :This function allows to cancel an on-going RSP session on the eUICC and to get a signed data structure that is necessary for the LPAd to request the same session cancellation to the RSP Server.
*              :根据规范 描述，CancelSession 命令可能发生在下载过程的任一时机，所以要考虑profile相关资源的回收
*
*@see          : <SGP22_V2.2 > 5.7.14  Function (ES10b): CancelSession    3.1.3  Profile Download and Installation BF41 L 80LV 8101V
*
*@param        :
*
*@return       :void
*
*@RAM Resources:
*
*@author       :lx @20190110
*
*@hisotry      :modify by xuxiaojun@20190709 对于该条指令总长度的合法性进行判断
*
*@note		   :此函数使用tempBuf前128个字节来缓存数据             
*/
void CUSTOMER_CancelSession(void)
{
	BYTE idata len[2], offset, cancelSessionReason;	
	
	if(G_RSPSessionContext.sessionSate <= RSPSessionState_Init)   //测试用例TC_eUICC_ES10b_CancelSession_ErrorCase_01_Error_No_on_going_RSP_session 
	{
        CancelSessionSendErrorResult(DownloadErrorCode_invalidTransactionId) ; 
		
		return ;
	}
	
	if(G_CustomerStoreDataApdu.storeDataLen > 24) // 24 = BF41 L 8010V 8101V 的最大有效字节数
	{
		CancelSessionSendErrorResult(DownloadErrorCode_invalidTransactionId) ; 
		
		return ;
	}
		
	offset = 0;

	len[0] = RecData[offset+2]; //len[0]表示BF41的length ，len[1]表示后续80 81 tag的长度的和
	
	offset += 3; //偏移到80 tag
	
	if(RecData[offset] == 0x80)
	{
		len[1] = RecData[offset+1]+2;
		
		if((RecData[offset+1] != G_RSPSessionContext.transactionIdLen) || (EQUAL != ecpMem_Compare(&RecData[offset+2], &G_RSPSessionContext.channllengeTransactionId[0], G_RSPSessionContext.transactionIdLen))) //todo： 验证 指令后 transactionId 和上下文中transaId一致
		{
			CancelSessionSendErrorResult(DownloadErrorCode_invalidTransactionId) ;  // transactionId 和上下文中transaId不一致
			return ;
			
		}
		
		offset += (RecData[offset+1] + 2); //偏移到81 tag
		
		if(RecData[offset] == 0x81)
		{
			len[1] += (RecData[offset+1]+2);
			
			if(len[0] != len[1]) //todo ： 判断数据的完整性，即 Tag 'BF41' 的 L =后续80 81 tag的和
			{
				CancelSessionSendErrorResult(DownloadErrorCode_undefinedError) ;
	
				return ;
			}
			
			cancelSessionReason = RecData[offset+2];
		}
		else
		{
			CancelSessionSendErrorResult(DownloadErrorCode_undefinedError) ;
	
			return ; //第二个子tag不是81
		}
	}
	else
	{
		CancelSessionSendErrorResult(DownloadErrorCode_invalidTransactionId) ;//第一个子tag不是transactionID 80
	
		return ;
	}
		
	len[0] = PaddingCancelSessionResponse(cancelSessionReason);
		
	if(len[0] == FALSE)
	{
		CancelSessionSendErrorResult(DownloadErrorCode_undefinedError) ; //CI PKID 对应ci链上的EUICC 证书不存在，抛错
		return ;
	}
	
	if(G_RSPSessionContext.sessionSate == RSPSessionState_Prepare)  //SGP22 3.1.5  However, in the event that this does occur, the eUICC SHALL discard its session state (including generated eUICC challenge, any downloaded Profile Metadata, Profile contents, and Profile Installation Result) with the possible exception that an unused otPK/otSK.EUICC.ECKA MAY be stored for future retry
	{
		//todo :会话被打断时，已经产生了otPK/otSK.EUICC.ECKA，但是还未使用，此时将otPK/otSK.EUICC.ECKA + SMDP OID保存下来，供下次会话使用
	}
	else
	{
		if(G_RSPSessionContext.sessionSate >= RSPSessionState_ConfigureISDP)  //当前已经在创建ISD-P 设置参数等操作，需要回滚这些操作 
		{			
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
			{
				GPLoadInit();
			}
			
			DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);//todo :回滚ISD-P  回滚profile等相关数据

//	打垃圾回收标记 ？GarbageCollectorAndHeapPieceCleanUp();

		}
	}

	RSP_ClearSession();  //Discard the on-going RSP session context.

	SetOutgoingAndSend(0x05,len[0]);
}

/*
*@function     : CUSTOMER_InitialiseSecureChannel 
*
*@description  :This function is used by the SM-DP+ to open a new RSP session with the target eUICC
*
*@see          : <SGP22_V2.2 > 5.5.1  Function: InitialiseSecureChannel   
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:NO
*
*@hisotry      V1.0 :created by xuyunzhen@20190104
*              V1.1 :modify by liguiyu@20190424 SCP03会话密钥的ICVcounter初始化成0，每次解密前再+1，避免个人版和设备版分别在算完CMAC+1和算完RMAC+1
*              V1.2 :modify by xuxiaojun@20190621 代码书写规范
*              V1.3 :modify by xuxiaojun@20190709 如果下行的transactionId与G_RSPSessionContext中的不一致的话，返回G_RSPSessionContext中,避免下行的transactionId长度超过16字节而越界的情况
*              V1.4 :modify by chentao@20200811修改可save sessionkey
*/
WORD code smdpOtpkSignTag[2]= {0x5F49, 0x5F37};

void CUSTOMER_InitialiseSecureChannel(void)
{
	BYTE idata tmp, i , sharedInfoLen , shsLen ;
	UWORD idata tmpLen, offset;
	UWORD xdata dataStartAddr;	
	WORD idata backOff[2], tag;
	BYTE idata keyType, keyLen;
	BYTE xdata hostId[1 + InitialiseSecureChanne_hostId_MAX_LEN];  //LV结构

	if (JcvmConfig_GetSEState() == CARD_OP_READY)//预置profile时，不需要执行后续的判断
	{
		G_RSPSessionContext.sessionSate = RSPSessionState_InitiaSecureChannel;
		
		return;
	}
	
	if(G_RSPSessionContext.sessionSate == RSPSessionState_NULL) // <TC_eUICC_ES8_InitialiseSecureChannel_05_Error_InitialiseSecureChannel_request_while_Secure_Channel_Session_is_ongoing >
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		
		return;
	}
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_Prepare)  
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_installFailedDueToUnknownError, ADDR_IS_NULL);
		
		return;
	}
	
	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)
	{
		dataStartAddr = (UWORD)RecData;
	}
	else
	{
		dataStartAddr = EUICCTempDataBuf;
	}
	
	dataStartAddr += 2;
	
	tmpLen = 0x00;
	
	tmp = ASN1_GetBERTlvLen(dataStartAddr, &tmpLen);  //BF36 的长度
	
	dataStartAddr += tmp;  //point to the tag of BF23
	
	tag = VMReadShort(dataStartAddr);  //读取BF23 字段
	
	if(tag != 0xBF23)
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	tmp = ASN1_GetBERTlvLen(dataStartAddr+2, &tmpLen);  //BF23的长度
	
	dataStartAddr += (2 + tmp);  //point to BF23的 value位置
	
	G_CustomerStoreDataApdu.storeDataLen = tmpLen;
	
	VMReadBuf(tempBuf, dataStartAddr, 70);  //先读取67字节的数据，有可能会读到无效的数据，但是如果保证了dataStartAddr 的有效长度一定比70大，则无效数据不处理即可，70 = remoteOpId(3字节) + transactionId(max 18字节) + controlRefTemplate(max 49字节)
	
	offset = 0; 
	
	if((tempBuf[0] != 0x82) ||( tempBuf[1] != 0x01) ||(tempBuf[2] != 0x01)) //remoteOpId 字段非法
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_unsupportedRemoteOperationType, ADDR_IS_NULL);
		
		return;
	}
	
	offset +=3; 
	
	if(tempBuf[3] == 0x80) 
	{
		if( (tempBuf[4] == G_RSPSessionContext.transactionIdLen ) &&(ecpMem_Compare(&tempBuf[5], &G_RSPSessionContext.channllengeTransactionId[0], tempBuf[4]) == EQUAL))
		{
			offset += tempBuf[4] + 2;  //point to  controlRefTemplate
		}
		else
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_invalidTransactionId, ADDR_IS_NULL);
		
			return;
		}
	}
	else // 如果下来的没有transactionID时,那生成的ProfileInstallationResultData中的transactionId是什么？
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_invalidTransactionId, ADDR_IS_NULL);
		
		return;
	}
	
	if(tempBuf[offset] == 0xA6) // A6 L 8001V 8101V 84LV
	{
		if((tempBuf[offset + 2] != 0x80) || (tempBuf[offset + 3] != 0x01) || (tempBuf[offset + 5] != 0x81) || (tempBuf[offset + 6] != 0x01) || (tempBuf[offset + 8] != 0x84) || (tempBuf[offset + 1] != tempBuf[offset + 9] + 8))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
			return;
		}
		
		offset += 2 ;  //point to the  tag of keyType
		
		keyType = tempBuf[offset+2];

#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
			if(keyType != keyType_SM4)
			{
				Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_unsupportedCrtValues, ADDR_IS_NULL);

				return;
			}
		}
		else
#endif
		{
			if(keyType != keyType_AES)
			{
				Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_unsupportedCrtValues, ADDR_IS_NULL);

				return;
			}
		}
		
		offset += 3 ;  //point to the  tag of keyLen
		
		keyLen = tempBuf[offset+2];
		
		if(keyLen != 0x10)
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_unsupportedCrtValues, ADDR_IS_NULL);
			
			return;
		}
		
		offset += 3 ;  //point to the  tag of hostId
		
		ecpMem_Set(hostId, 0x00, sizeof(hostId));
		
		hostId[0] = tempBuf[offset +1];
		
		if((hostId[0] == 0x00) ||(hostId[0] > InitialiseSecureChanne_hostId_MAX_LEN))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
			return;
		}
		
		ecpMem_Copy(&hostId[1] , &tempBuf[offset +2], hostId[0]);
		
		offset += (hostId[0] + 2);  //point to  offset of '5F49
	}
	else
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
		return;
	}
	
	ecpMem_Set((BYTE *)backOff , 0x00, sizeof(backOff));
	
	for(i=0; i<2 ; i ++)
	{	
		tag = VMReadShort(dataStartAddr+offset);  //读取5f49 /5F37字段
	
		if(tag != smdpOtpkSignTag[i])
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_unsupportedRemoteOperationType, ADDR_IS_NULL);
		
			return;
		}
		
		backOff[i] = offset;
		
		tmp = ASN1_GetBERTlvLen(dataStartAddr+offset+2, &tmpLen);
		
		offset += (tmp + tmpLen + 2);  //point to the off of 5F37
	}
	
	for(i=0; i<2 ; i ++)   //smdpOtpk and  smdpSign 有一个不存在
	{
		if( backOff[i]  == FALSE)
		{ 
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_unsupportedRemoteOperationType, ADDR_IS_NULL);
		
			return;
		}
	}
	
	tmp = InitialiseSecureChannel_VerifySmdpSign(dataStartAddr ,backOff[1]); //Verify the SM-DP+ signature using the PK.DPbp.ECDSA; if the signature is invalid the command SHALL be rejected
	
	if(tmp != RESPONSE_NoError )
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, tmp, ADDR_IS_NULL);
		
		return;
	}
#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		tmp = InitialiseSecureChannel_KeyExchangeForSM2(dataStartAddr+backOff[0], &hostId[1], hostId[0], keyLen*3, tempBuf);

		if(tmp != RESPONSE_NoError )
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, tmp, ADDR_IS_NULL);
	
			return;
		}
	
		offset = 0;
	}
	else
#endif
	{
		tmp = InitialiseSecureChannel_CalShS(dataStartAddr+backOff[0] ,tempBuf);  //tempBuf存放V 结构的shs，长度固定为64字节

		if(tmp != RESPONSE_NoError )
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, tmp, ADDR_IS_NULL);

			return;
		}

		shsLen = 64;  //shs，长度固定为64字节

		tempBuf[shsLen] = keyType;

		tempBuf[shsLen + 1] = keyLen;

		ecpMem_Copy(&tempBuf[shsLen + 2], &hostId[0], hostId[0] + 1) ;  // HostID-LV comprises the length and the value field of the HostID given in the input data

		sharedInfoLen = hostId[0] + 3;

		VMReadBuf(&tempBuf[shsLen + sharedInfoLen], EUICCEIDInfo_Addr, EUICCEIDLVLEN);  //读取EID len & Value

		sharedInfoLen  += (tempBuf[shsLen + sharedInfoLen] + 1);

		shsLen = KeyDerivationFunction(tempBuf, ECC_PARA_LEN_256, &tempBuf[64], sharedInfoLen , keyLen *3, &tempBuf[64 + sharedInfoLen], &tempBuf[96 + sharedInfoLen*2 + 4] );
		
		if (shsLen < (keyLen * 3))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_INITIALISESECURECHANNEL, ErrorReason_incorrectInputValues, ADDR_IS_NULL);

			return;
		}
		
		offset = 96 + sharedInfoLen*2 + 4;  //point to the first key Data
	}
	
	ecpMem_Copy(&G_RSPSessionContext.initMacValue[0], &tempBuf[offset], keyLen);    // Initial MAC chaining value
	
	ecpMem_Copy(&G_RSPSessionContext.encKey[0], &tempBuf[offset+keyLen], keyLen);   // S-ENC
	
	ecpMem_Copy(&G_RSPSessionContext.macKey[0], &tempBuf[offset+keyLen*2], keyLen); //S-MAC
	
	//save session key
	if(EUICC__SAVE_SESSIONKEY || Germany_Mobile)
	{
		VMWriteBuf(EUICC_SessionKey,&tempBuf[offset], keyLen*3);
	}
//	else
//	{
//		VMSetBuf(EUICC_SessionKey, 0xff, EUICC_SessionKeyLen);  //fill 0xff if not support save session key
//	}

	ecpMem_Set(&G_RSPSessionContext.icvCounter[0] ,0x00, 0x10);
	
// 	G_RSPSessionContext.icvCounter[0x0F] = 0x01;  //If random key mode is selected by the SM-DP+, the initial MAC chaining value to be used for the first segment of the PPP is provided together with the random key (section 5.5.4) and the encryption counter for ICV calculation is reset to its initial state (i.e. the value on 16 bytes is '00…01').	
	
	G_RSPSessionContext.sessionSate = RSPSessionState_InitiaSecureChannel;
	
	G_RSPSessionContext.channelNum = selected_channel; //modify by xxj@20240327 兼容祥承和ifree，下载profile时，如果进入到BF36时，关通道时需要清空会话和profile数据

	VMSetBuf(EUICC_OpKeyBuf, 0x00, sizeof(BYTE));  //清除otPk等信息

}


/*
*@function     : CUSTOMER_ConfigureISDP 
*
*@description  :This function is used by the SM-DP+ to provide data to the eUICC for configuring the ISD-P
*
*@see          : <SGP22_V2.2 > 5.5.2  Function: ConfigureISDP  
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:tempBuf[380]之后的最大128字节
*
*@author       :xuyz
*
*@hisotry      :V1.0 :create @20181126 假设从BF24开始数据存储在RecData[0]中
*              :V1.1 :modify by lcz@20190905 实现红茶移动需求
*/
//SGP02_V3.2 2.2.3  Identification of Security Domains: AID and TAR & 附录H
BYTE code ISDP_LoadFileAID[0x11]    = {0x10, 0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x0D,0x00};  
BYTE code ISDP_ModuleAID[0x11]      = {0x10, 0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x0E,0x00};  
//BYTE code ISDP_ApplicationAID[0x11] = {0x10, 0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x10,0x00};  //实际的AID 的值，按照index递加
BYTE code ISDP_ApplicationAID[0x11] = {0x10, 0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x12,0x00};  //实际的AID 的值，按照index递加   0x11 预留给红茶移动的，10暂时没用
BYTE code CustomerInstallISDPPara[] = {0x03,0x80,0xC0,0x00,   //Length of Privileges + Privileges
                                       0x06, 0xC9, 0x04, 0x81, 0x02, 0x03,0x70,     //length of  Length of I nstall P arameters field   + Install  P arameters field  .ISDP  支持SCP03协议
	                                   0x00,   //length of token + token value
                                      };

void CUSTOMER_ConfigureISDP(void)
{
	WORD idata decryLen,sw;
	BYTE idata index, tmp,commandLen;
	UWORD xdata dataAddr,totalLen;
#if EUICC__FUNCTION_RedTea_Mobile
	WORD idata redteaTag;    //红茶移动的私有tag
#endif
		
	if(G_RSPSessionContext.sessionSate == RSPSessionState_NULL)  
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		
		return;
	}
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_InitiaSecureChannel)  
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_installFailedDueToUnknownError, ADDR_IS_NULL); 
		
		return;
	}
	
	dataAddr = (UWORD)&RecData[0];
	
	tmp = ASN1_GetBERTlvLen(dataAddr + 1, (UWORD*)&totalLen);  //获取 A0  的 L 长度
	
	dataAddr += tmp + 1;  //point to the 87 tag
	
	decryLen = SCP03tDecryptAndCalMac(dataAddr, dataAddr, totalLen, (SECURITYLEVEL_CMAC|SECURITYLEVEL_CDECRYPTION), &G_RSPSessionContext.macKey[0], 0x10, &G_RSPSessionContext.initMacValue[0],&G_RSPSessionContext.encKey[0],0x10,&G_RSPSessionContext.icvCounter[0]);
	
	if(decryLen == FALSE)
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_scp03tSecurityError, ADDR_IS_NULL); 
		
		return;
	}
	
	tmp = ASN1_GetBERTlvLen(dataAddr+1, (UWORD*)&totalLen);  //获取 87  的 L 长度
	
	dataAddr += tmp +1;  // //point to the BF24 tag
	
	tmp = ASN1_GetBERTlvLen(dataAddr+2, (UWORD*)&totalLen);  //获取 Tag 'BF24  的 L 长度
	
	if((totalLen > 0x80) || ((totalLen == 0x80) && (tmp != 0x01) && (tmp != 0x02)))  //If the length of the SM-DP+ proprietary data exceeds the maximum size, terminate with error 'incorrectInputValues'.  测试用例：TC_4_2_4_2_1_02_TC_eUICC_ES8Plus_ConfigureISDP 有测长度等于0x80的情况
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_incorrectInputValues, ADDR_IS_NULL); //maximum size including tag and length field: 128 bytes
		
		return;
	}
	
	index = GetIdleProfileIndex();
	
	if(index == ILLEGAL_PROFILEINDEX)
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_installFailedDueToInsufficientMemoryForProfile, ADDR_IS_NULL); 
		
		return;//profile已满
	}
	
	ecpMem_Copy(&tempBuf[380], (BYTE *)(dataAddr+tmp+2), totalLen);  //拷贝dpProprietaryData 到tempBuf中，tempBuf一定要为512字节长： 数据结构为TLV 结构

#if EUICC__FUNCTION_RedTea_Mobile
	if(RedTea_Mobile)
	{
		redteaTag = ASN1_FindTagTLV((UWORD)&tempBuf[380],totalLen,0xF8); //红茶移动的私有tag
	}
#endif	

	G_RSPSessionContext.downloadProfileIndex = index;
	
	commandLen = 0; 

	ecpMem_Copy(&RecData[0],(BYTE *)ISDP_LoadFileAID,sizeof(ISDP_LoadFileAID) ) ;
	
	commandLen += sizeof(ISDP_LoadFileAID);
	
	ecpMem_Copy(&RecData[commandLen],(BYTE *)ISDP_ModuleAID,sizeof(ISDP_ModuleAID) ) ;
	
	commandLen += sizeof(ISDP_ModuleAID);

#if EUICC__FUNCTION_RedTea_Mobile
    if(RedTea_Mobile && (redteaTag != TAG_NOT_FOUND))
	{
		if((tempBuf[380+redteaTag+1] < 0x05) || (tempBuf[380+redteaTag+1] > 0x10))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_installFailedDueToUnknownError, ADDR_IS_NULL); 
		
			return;//profile已满	
		}
		ecpMem_Copy(&RecData[commandLen],&tempBuf[380+redteaTag+1],tempBuf[380+redteaTag+1]+1 ) ; //LV
		
		commandLen += (tempBuf[380+redteaTag+1]+1);	

		totalLen -=  (tempBuf[380+redteaTag+1]+2);			
	}
	else
#endif
	{
		ecpMem_Copy(&RecData[commandLen],(BYTE *)ISDP_ApplicationAID,sizeof(ISDP_ApplicationAID) ) ;
		
		commandLen += sizeof(ISDP_ApplicationAID);
	
		RecData[commandLen-2] = 0x12 + index;  //动态修改ISD-P的APPID
	}
	
	ecpMem_Copy(&RecData[commandLen],(BYTE *)CustomerInstallISDPPara,sizeof(CustomerInstallISDPPara) ) ;
	
	commandLen += sizeof(CustomerInstallISDPPara);
	
	CLA = (0x80 | selected_channel);

	INS = 0xE6;  //INS of STORE DATA

	P1 = 0x0C;   //Last block

	P2 = 0x00;
	
	CLA = 0x80|selected_channel;
	
	G_APDU[4] = commandLen;
	
	ecpMem_Copy(G_ApduBuffer,G_APDU,0x05);  //GPinstallforInftall函数会使用P3进行判断处理，所以此处要给G_APDU赋值
	
	bRamBuffer[LOADING_PROFILE_INDEX] = index;

	if(CreateProfileEntry() != 0x9000)//创建ProfileEntry
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_installFailedDueToInsufficientMemoryForProfile, ADDR_IS_NULL);
		return;//profile已满
	}

	SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

	sw = CUSTOMERGP_InstallforInstallISDP(g_bSDId[selected_channel]);

	SEUICC_ResumeISDPAppletTable();

	if(sw != 0x9000)
	{
		if(sw == 0x6A84) //空间不够返回对应状态0x0A  eSIM_Test_additional/压力测试/Profile_DownLoad_pressure_Test_01  创建-P空间不够
		{
			tmp = ErrorReason_installFailedDueToInsufficientMemoryForProfile;
		}
		else
		{
			tmp = ErrorReason_installFailedDueToUnknownError;
		}
		Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, tmp, ADDR_IS_NULL);

		return;//profile已满
	}
	
	if(totalLen != 0x00)
	{
		index = StoreProfileMetadataRef(&tempBuf[380], tempBuf[381] +2, MetaDataDpProprietaryDataRef_INDEX, G_RSPSessionContext.downloadProfileIndex  );   //将 dpProprietaryData 写入到ISDP 参数区域 中，此处不用考虑和创建ISDP在一个事务中， 因为如果创建这里失败，失败了之后发送install结果后续会回收空间
		
		if(index == FALSE)
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_CONFIGUREISDP, ErrorReason_installFailedDueToInsufficientMemoryForProfile, ADDR_IS_NULL); 
			
			return; //空间分配失败，报错返回
		}
	}
	
	G_RSPSessionContext.sessionSate = RSPSessionState_ConfigureISDP;
}

//modify by xieyong@20220914 增加0xBF22,0xBF23tag
BYTE code metaData[]   = {TAG_ICCID, TAG_SERTVICEPROVIDERNAME, TAG_PROFILENAME, TAG_ICONTYPE,TAG_ICON, TAG_PROFILECLASS ,TAG_NOTIIFICATIONCONFIG,TAG_PROFILEOWNER,TAG_PROFILEPOLICYRULES
#if RSP_SGP22_BF22
		,TAG_SERVICESPECIFICDATASTOREDINEUICC,TAG_SERVICESPECIFICDATANOTSTOREDINEUICC
#endif
};
BYTE code metaDataOff[]=
{
		MetaDataIccid_INDEX,//MetaData_Iccid_Offset,
		MetaDataServiceProviderNameRef_INDEX,//MetaData_ServiceProviderNameRef_Offset,
		MetaDataProfileNameRef_INDEX,//MetaData_ProfileNameRef_Offset,
		MetaDataIconType_INDEX,//MetaData_IconType_Offset,
		MetaDataIconRef_INDEX,//MetaData_IconRef_Offset,
		MetaDataProfileClass_INDEX,//MetaData_ProfileClass_Offset,
		MetaDataNotificationConfigurationInfoRef_INDEX,//MetaData_NotificationConfigurationInfoRef_Offset,
		MetaDataProfileOwerRef_INDEX,//MetaData_ProfileOwerRef_Offset,
		MetaDataProfilePolicyRules_INDEX//MetaData_ProfilePolicyRules_Offset};
#if RSP_SGP22_BF22
		,MetaDataserviceSpecificDataStoredInEuicc_INDEX//MetaData_serviceSpecificDataStoredInEuicc_Offset};
#endif
};


/*
*@function     :GetStoreMetadataDataOffset 
*
*@description  :获取storeMetadata中TAG的相对于EUICCTempDataBuf的偏移
*
*@see          : <SGP22_V2.2 > 5.5.3  Function: StoreMetadata
*
*@param        :WORD* dataOffset --- 输出参数，存储各TAG的起始相对于BF25的起始偏移
*
*@return       :WORD --- 数据非法时返回0xFFFF
*
*@hisotry      :V1.0 :created by xuxiaojun@20190521 假设数据起始从BF25 开始,数据存储在EUICCTempDataBuf中
*/
//modify by lgy@20240528 ，原函数在下行tag乱序时处理有问题  ，除了解决这个问题 还兼容处理了2字节tag，包括不识别的tag
//不识别tag插在中间也可以当成一个正常的UNSPECIFIED_TAG
WORD GetStoreMetadataDataOffset(WORD* dataOffset)
{
	BYTE idata tmp, i, j, tag[2], tagLen, lenLen, tagCount, unspecifiedTag;
	WORD idata  offset;
	UWORD xdata  totalLen,lenValue;
	WORD idata tmpDataOffset[sizeof(metaData) + UNSPECIFIED_TAG_NUMBER];
	BYTE idata unspecifiedTagIndex;

	unspecifiedTagIndex = sizeof(metaData);//不识别的tag从标准tag之后存

	tmp = ASN1_GetBERTlvLen(EUICCTempDataBuf + 2,(UWORD*)&totalLen); // BF25 LV

	offset = 0;

	tagCount = 0x00;
	
	ecpMem_Set(tmpDataOffset, 0x00, sizeof(tmpDataOffset));
	
	while((offset < totalLen)&&(tagCount < (sizeof(metaData) + UNSPECIFIED_TAG_NUMBER)))
	{
		tagLen = ASN1_GetBERTlvTag(EUICCTempDataBuf + 2 + tmp + offset, tag);

		lenLen = ASN1_GetBERTlvLen(EUICCTempDataBuf + 2 + tmp + offset + tagLen, (UWORD*)&lenValue);

		tmpDataOffset[tagCount] = (2 + tmp + offset);

		offset += (tagLen + lenLen + lenValue);

		tagCount++;
	}

	if(offset != totalLen)
	{
		return 0xFFFF;
	}

	for(i=0;i<tagCount;i++)
	{
		unspecifiedTag = TRUE;

		tagLen = ASN1_GetBERTlvTag(EUICCTempDataBuf + tmpDataOffset[i], tag);

		for(j=0; j < sizeof(metaData) ;j++)
		{
			if((tagLen == 0x01)&&(tag[0] == metaData[j]))//单tag 并且是标准tag
			{
				dataOffset[j] = tmpDataOffset[i];
				unspecifiedTag = FALSE;//是标准tag
				break;
			}
//#if SGP32_IoT//兼容2tag的不识别tag 不区分是否是SGP32,双tag不是SGP32特有的
			else if((tagLen == 0x02)&&((tag[0]&0x1F) == 0x1F)&&(tag[1] == metaData[j]))//2字节标准tag 并且符合ASN.1编码
			{
				dataOffset[j] = tmpDataOffset[i];//这里存的是tag第1个字节的偏移
				unspecifiedTag = FALSE;//是标准tag
				break;
			}
//#endif
		}

		if((unspecifiedTag == TRUE)&&(unspecifiedTagIndex < (sizeof(metaData) + UNSPECIFIED_TAG_NUMBER)))//标准tag没匹配上 当成不识别tag处理 ，如果超出最大不识别tag个数 直接忽略不报错
		{
			dataOffset[unspecifiedTagIndex++] = tmpDataOffset[i];//这里存的是tag第1个字节的偏移  使用时直接根据ASN1相关函数取TLV
		}

	}

	return offset;
}
/*
* 说明： 在storeMetaData中，什么情况下算作iccid未知，需要探讨一下，目前是按照我的理解来做的。
*@see :  
*/
/*
*@function     :CUSTOMER_StoreMetadata 
*
*@description  :this function is used by the SM-DP+ to provide Profile Metadata of the Profile to the eUICC.
*
*@see          : <SGP22_V2.2 > 5.5.3  Function: StoreMetadata
*              : <SGP.22_v2.2.pdf> 2.5.6  Profile Installation Result  <ICCID (Not provided if the Notification reports an error that has happened before ICCID was known by the eUICC, otherwise it SHALL be present) > 
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:No
*
*@hisotry      :假设数据起始从BF25 开始,统一放在EUICCTempDataBuf
×　　　　　　　:V1.0 :created by xuyunzhen @20190104
*              :V1.1 :modify by xuxiaojun@20190621 修复PPRsVerification调用时tempBuf数据被冲掉的问题
*              :V1.2 :modify by xuxiaojun@20190709 增加边界判断，判断NotificationConfigurationInformation中的notificationAddress是否越界
*/
void CUSTOMER_StoreMetadata(void)
{
	BYTE idata tmp,i, profileClass;
	WORD idata dataOffset[sizeof(metaData) + UNSPECIFIED_TAG_NUMBER]; //dataOffset,分别代表BF25 下各字段的tag所在的偏移
	BYTE xdata iccid[12], ppr[3];            // TLV
	ASN1Info xdata tempASN1Info;
	UWORD xdata totalLen,tmpLen;
	if(G_RSPSessionContext.sessionSate != RSPSessionState_PrepareStoreMetadata)   //并没有收到 Tag and length fields of the sequenceOf88 TLV 
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	tmp = StoreMetaData_ProcessEachMetaData88();
	
	if(tmp == 0xFF)
	{
		return;//88序列还没收全 ，继续接收后续的88 序列	
	}
	else if(tmp != RESPONSE_NoError)   //解析 88 字段时出现错误
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, tmp, ADDR_IS_NULL);  // 此时iccid数据有吗？
		
		return;
	}
	
	tmp = ASN1_GetBERTlvLen(EUICCTempDataBuf + 2,(UWORD*)&totalLen); // BF25 LV
	
	ecpMem_Set((BYTE *)dataOffset, 0x00, sizeof(dataOffset));
	
	if(0xFFFF ==  GetStoreMetadataDataOffset(dataOffset))
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
		return;
	}
	
	if((dataOffset[0] == 0x00) || (dataOffset[1] == 0x00) || (dataOffset[2] == 0x00)) // 必选项缺失的话，抛错
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
	
		return;
	}
	
	for(i=0x00;i<sizeof(metaData);i++)
	{
		tmp = ASN1_GetBERTlvLen( (EUICCTempDataBuf + dataOffset[i] + 1)  , (UWORD*)&tmpLen );
		
		if(i==0x00)
		{
			if(tmpLen != ICCID_LEN)
			{
				Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
	
				return;
			}
			
			VMReadBuf(&iccid[0], EUICCTempDataBuf + dataOffset[i], (1 + tmp + tmpLen));   //为了后面生成通知使用
		}
		
		if((i == 0x01) && (tmpLen > ServiceProviderNameMaxLen)) //serviceProviderName [17] UTF8String (SIZE(0..32))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
	
			return;
		}
		
		if((i == 0x02) && (tmpLen > ProfileNameMaxLen)) //profileName [18] UTF8String (SIZE(0..64))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
	
			return;
		}		
	}
	
	if(GetProfileIndexByICCID((BYTE *)(EUICCTempDataBuf + dataOffset[0] + 2) ) != ILLEGAL_PROFILEINDEX)  //iccid 已存在
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_installFailedDueToIccidAlreadyExistsOnEuicc, iccid);
		
		return;
	}
	
	if(((dataOffset[3] != 0x00)&&(dataOffset[4] == 0x00)) || ((dataOffset[3] == 0x00)&&(dataOffset[4] != 0x00))) //This field SHALL only be present if iconType is present. iconType和icon要么同时存在，要么同时不存在
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, iccid); // <TC_4_2_5_2_1_10_TC_eUICC_ES8Plus_StoreMetadata>
		
		return;
	}
	
	if(dataOffset[3] != 0x00)
	{
		ASN1_GetBERTlvLen((EUICCTempDataBuf + dataOffset[3] + 1), (UWORD*)&tmpLen);
		
		if(tmpLen > 0x01) //iconType的长度不能大于1
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL); //SGP22接口边界值测试:StoreMetadataRequest_BoundaryValueTest_03_01
		
			return;
		}
		
		ASN1_GetBERTlvLen((EUICCTempDataBuf + dataOffset[4] + 1), (UWORD*)&tmpLen); //dataOffset[3] != 0x00时dataOffset[4] != 0x00
		
		if(tmpLen > IconMaxLen)
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL); //SGP22接口边界值测试:StoreMetadataRequest_BoundaryValueTest_04_01
		
			return;
		}
	}	
	
	if((dataOffset[8] != 0x00)&&(dataOffset[7] == 0x00)) //The data object profileOwner is optional. It SHALL be present if the profilePolicyRules data object is present. 如果ppr存在，则profileOwner一定要存在
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, iccid); // <TC_eUICC_ES8_StoreMetadata_08_Error_Profile_Policy_Rule_is_set_but_Profile_Owner_is_not>这种情况的错误码为07/01/7F
		
		return;
	}
	
	if(dataOffset[5] != 0x00)  //profileClass字段存在
	{
		ASN1_GetBERTlvLen((EUICCTempDataBuf + dataOffset[5] + 1), (UWORD*)&tmpLen);
		
		if(tmpLen > 0x01) //profileClass的长度不能大于1
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, ADDR_IS_NULL); //SGP22接口边界值测试:StoreMetadataRequest_BoundaryValueTest_05_01
		
			return;
		}
		
		profileClass = VMReadByte(EUICCTempDataBuf +dataOffset[5] + 2 );
		
		if(profileClass > PROFILE_CLASS_OPERATIONAL)  //PROFILE_CLASS_TEST : 0x00  PROFILE_CLASS_PROVISIONING: 0x01  PROFILE_CLASS_OPERATIONAL:0x02
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_unsupportedProfileClass, iccid);
		
			return;
		}
		
		if(profileClass == PROFILE_CLASS_TEST)  // Test Profile SHOULD NOT contain any Profile Policy Rules. 
		{
			if(dataOffset[8] != 0x00)   //profilePolicyRules [25] PprIds OPTIONAL –- Tag '99' 字段存在
			{
				Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_pprNotAllowed, iccid);
		
				return;
			}
		}
	}
	
	if(dataOffset[6] != 0x00)   //判断NotificationConfigurationInformation中的notificationAddress是否越界   notificationConfigurationInfo存在，结构：B6+L+30+L+80+L(L等于2)+V+81+L+V，分别是notificationConfigurationInfo，NotificationConfigurationInformation，profileManagementOperation，notificationAddress
	{		
		tmp = ASN1_GetBERTlvLen(EUICCTempDataBuf + dataOffset[6] + 1 , (UWORD*)&tmpLen ); //获取notificationConfigurationInfo(B6LV)处L的值
		
		ASN1_InitTLVInfo((ASN1Info *)&tempASN1Info, EUICCTempDataBuf + dataOffset[6] + 1 + tmp, tmpLen); //1表示偏移过B6，用于查找的数据为30LV 30LV...
	
		while(1)          // 循环遍历NotificationConfigurationInformation, 30L 80LV 81LV
		{
			if(FALSE == ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x30 , FALSE, 1))   //找到SEQUENCE OF的最后一个NotificationConfigurationInformation
			{
				break;
			}
			
			ASN1_SetNextDepthToFindTLV((ASN1Info *)&tempASN1Info);
			
			ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x81 , FALSE, 0x01);  //找到NotificationConfigurationInformation中的notificationAddress，如果30存在，则其子TAG中一定存在notificationAddress
				
			if(tempASN1Info.curValueLen > NotificationAddress_MAX_LEN)
			{
				Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_installFailedDueToInsufficientMemoryForProfile, iccid);
	
				return;
			}
			
			ASN1_ReturnFromDepthToFindTLV((ASN1Info *)&tempASN1Info);         //返回到上一级继续查找30
		}
	}
	
	if(dataOffset[7] != 0x00)   // profileOwner存在
	{	
		tmp = ASN1_GetBERTlvLen(EUICCTempDataBuf +dataOffset[7] + 1 , (UWORD*)&tmpLen );
		
		if(tmpLen > ProfileOwner_MAX_LEN)
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, iccid);
		
			return;
		}
			
		VMReadBuf(tempBuf, EUICCTempDataBuf+ dataOffset[7] + 1 + tmp, tmpLen);  //profileOwner的Value到tempBuf，其中前5个字节是MNCMCC字段8003 xxxxxx 
		
		if(StoreMetaData_verifyProfileOwner(&tempBuf[0], tmpLen ) == FALSE ) //In this instance the mccMnc field SHALL not specify any wildcard ('E') digits
		{
//			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_pprNotAllowed, iccid);
			Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, iccid); //中国电信测试要求返回01
		
			return;
		}	
		
		if(dataOffset[8] != 0x00) // profilePolicyRules存在。If the profilePolicyRules data object is not present, all PPR bits of the Profile SHALL be considered zero.
		{
			VMReadBuf(ppr, EUICCTempDataBuf + dataOffset[8] + 1, 0x03);  //获取990100 或 9902xxxx
			
			switch(ppr[0])  // 长度只能是0x01, 0x02
			{
				case 0x01:
				{
					if(ppr[1] != 0x00)
					{
						Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, iccid);
						
						return;
					}
					break;
				}
				case 0x02:
				{
					if(PPRsVerification(ppr[2], tempBuf, tmpLen ) == FALSE) //if PPRs are provided in the Profile Metadata: the Profile Owner data object is present and the PPRs are allowed for the Profile Owner
					{
						Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_pprNotAllowed, iccid);
					
						return;
					}
					
					break;
				}
				default:
				{
					Notification_GenerateProfileInstallResult(BPPCommandId_STOREMETADATA, ErrorReason_incorrectInputValues, iccid);
						
					return;
				}
			}
		}		
	}
	
	for(i=0; i<sizeof(metaData); i++)
	{
		if(dataOffset[i] != 0x00)  //表示存在该字段
		{
			tmp = ASN1_GetBERTlvLen(EUICCTempDataBuf +dataOffset[i] + 1 , (UWORD*)&tmpLen );
			
			tmpLen += tmp + 1;  //该字段所占的总长度
#if RSP_SGP22_BF22
			if(i == 9)
			{
				tmpLen += 1;

				dataOffset[i] -= 1;
			}
			if(i == 0x0A)   //BF23 tag 不需要存
			{
				continue;
			}
#endif

			if((i == 0)||(i ==3) ||(i == 5)||(i == 0x08))  // Iccid,  iconType  profileClass profilePolicyRules
			{
				VMReadBuf(tempBuf, EUICCTempDataBuf + dataOffset[i], tmpLen);
				
				StoreProfileMetadataTLV(tempBuf, tmpLen,  metaDataOff[i], G_RSPSessionContext.downloadProfileIndex);  //更新ICCID
			}
			else
			{
				StoreProfileMetadataRef((BYTE*)(EUICCTempDataBuf + dataOffset[i]), tmpLen, metaDataOff[i],G_RSPSessionContext.downloadProfileIndex);
			}
		}
	}
	
	for(;i < (sizeof(metaData) + UNSPECIFIED_TAG_NUMBER);i++)
	{
		if(dataOffset[i] != 0x00)  //表示存在该字段
		{
			StoreProfileMetadataUnspecifiedTagRef((EUICCTempDataBuf + dataOffset[i]), G_RSPSessionContext.downloadProfileIndex);
		}
	}
	
	G_RSPSessionContext.sessionSate = RSPSessionState_StoreMetadata;
	
}


/*
*@function     : CUSTOMER_ReplaceSessionKeys 
*
*@description  :This function is used to replace the SCP03t session keys (S-ENC and S-MAC) during the loading of a Bound Profile Package by a new set of session keys
*
*@see          : <SGP22_V2.2 > 5.5.4  Function: ReplaceSessionKeys
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:No
*
*@author       :xuyz
*
*@hisotry      :V1.0 :create @20190104 该函数的实现假设数据都缓存在RecData中
*              :V1.1 :modify by xuxiaojun@20190425  SCP03会话密钥的ICVcounter初始化成0，每次解密前再+1，避免个人版和设备版分别在算完CMAC+1和算完RMAC+1
*/
BYTE code ReplaceSessionKeys_TAG[3] = {TAG_INITIALMAC, TAG_PPKENC, TAG_PPKCMAC};

void CUSTOMER_ReplaceSessionKeys(void)
{
	BYTE idata i,offset,tmp,tag;
	BYTE idata dataOffset[3]; //dataOffset[0] : macOffset, dataOffset[1] :ppkEncOffset, dataOffset[2] :ppkCmacoffset;
	UWORD xdata tmpLen,totalLen,dataAddr;

	if(JcvmConfig_GetSEState() == CARD_OP_READY)//预置profile时，不需要执行后续的判断
	{
		G_RSPSessionContext.sessionSate = RSPSessionState_ReplaceSessionKeys;
		
		return;
	}
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_StoreMetadata)   //并没有收到 Tag and length fields of the sequenceOf88 TLV 
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	dataAddr = (UWORD)&RecData[0];
	
	tmp = ASN1_GetBERTlvLen(dataAddr + 1,(UWORD*)&totalLen);
	
	if(P3 != (totalLen + tmp+1))
	{
		ThrowError(SW_WRONG_DATA); 
	}
	
	dataAddr += tmp +1 ; // point to the 87 offset
	
	totalLen = SCP03tDecryptAndCalMac(dataAddr,dataAddr,totalLen, (SECURITYLEVEL_CMAC|SECURITYLEVEL_CDECRYPTION),&G_RSPSessionContext.macKey[0], 0x10, &G_RSPSessionContext.initMacValue[0],&G_RSPSessionContext.encKey[0],0x10,&G_RSPSessionContext.icvCounter[0]);
	
	if(totalLen == FALSE)
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_REPLACESESSIONKEYS, ErrorReason_scp03tSecurityError, ADDR_IS_NULL);
		
		return;
	}
	
	tmp = ASN1_GetBERTlvLen(dataAddr+1, (UWORD*)&totalLen);  //获取 87  的 L 长度
	
	dataAddr += tmp +1;  // //point to the BF26 tag
	
	tmp = ASN1_GetBERTlvLen(dataAddr+2, (UWORD*)&totalLen);  //获取 Tag 'BF26  的 L 长度
	
	if((totalLen != 54)||(tmp != 0x01))  // 54 = 48(3个tag的长度都是16字节长) + 3 个字节的tag + 3 L 所占的字节
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_REPLACESESSIONKEYS, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
		return;
	}
	
	dataAddr += tmp +2; //point to the value of BF26
	
	dataOffset[0] = 0;
	
	for(offset = 0 ,i = 0; (offset < totalLen)&&(i < 3) ; )
	{
		tmp = ASN1_GetBERTlvLen( (UWORD)(dataAddr + dataOffset[i] + 1 ) , (UWORD*)&tmpLen );
		
		tag = VMReadByte(dataAddr + dataOffset[i]);
		
		if( tag != ReplaceSessionKeys_TAG[i])
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_REPLACESESSIONKEYS, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
			return;
		}
		
		if((tmp != 0x01) ||(tmpLen != 0x10 ))
		{
			Notification_GenerateProfileInstallResult(BPPCommandId_REPLACESESSIONKEYS, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
			return;
		
		}
		
		offset +=  tmpLen + 2;
		
		if(i < 2)
		{
			dataOffset[i+1] = dataOffset[i] + tmpLen + 2;  //point to the next tag
		}
	
		i ++;
			
	}
	
	if((offset != totalLen)||(i != 0x03))  //仅且存在3个tag
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_REPLACESESSIONKEYS, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
		return;
		
	}

	VMReadBuf(&G_RSPSessionContext.initMacValue[0], (dataAddr+dataOffset[0]+2), 0x10);
	
	VMReadBuf(&G_RSPSessionContext.encKey[0], (dataAddr+dataOffset[1]+2), 0x10);
	
	VMReadBuf(&G_RSPSessionContext.macKey[0], (dataAddr+dataOffset[2]+2), 0x10);
	
	if(Germany_Mobile) //add by chentao@20220107 德国需求保存sessionkey。这里保存PPK后的key
	{
		VMWriteBuf(EUICC_SessionKey,(BYTE *)(dataAddr+dataOffset[0]+2), 0x10);
		VMWriteBuf(EUICC_SessionKey+0x10,(BYTE *)(dataAddr+dataOffset[1]+2), 0x10);
		VMWriteBuf(EUICC_SessionKey+0x20,(BYTE *)(dataAddr+dataOffset[2]+2), 0x10);
	}

	ecpMem_Set(&G_RSPSessionContext.icvCounter[0] ,0x00, 0x10);
	
//	G_RSPSessionContext.icvCounter[0x0F] = 0x01;  //If random key mode is selected by the SM-DP+, the initial MAC chaining value to be used for the first segment of the PPP is provided together with the random key (section 5.5.4) and the encryption counter for ICV calculation is reset to its initial state (i.e. the value on 16 bytes is '00…01').	
	
	G_RSPSessionContext.sessionSate = RSPSessionState_ReplaceSessionKeys;
}
/*
*@function  : CUSTOMER_PrepareStoreMetadata
*
*@descrption: 处理storeMedata下载过程中的  Tag and length fields of the sequenceOf88 TLV   字段
*
*@param     : void
*
*@return    :void
*
*@author    :xuyz
*
*@history   :V1.0 :create
*/
void CUSTOMER_PrepareStoreMetadata(void)
{
	UWORD idata len;
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_ConfigureISDP)   //并没有收到configISDP命令
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	ASN1_GetBERTlvLen((UWORD)&RecData[1], (UWORD*)&len);
	
	G_RSPSessionContext.sessionSate = RSPSessionState_PrepareStoreMetadata;
	
	G_RSPSessionContext.remainSequence = len;  //剩余序列的长度
	
	G_RSPSessionContext.clearDataLen = 0;  //清空clearDataLen 字段 ，供接收级联storeData时使用
}
/*
*@function  : CUSTOMER_PrepareLoadPE
*
*@descrption: 处理profile下载过程中的  Tag and length fields of the sequenceOf86 TLV  字段
*
*@param     : void
*
*@return    :void
*
*@author    :xuyz
*
*@history   :V1.0 :create 
*/
void CUSTOMER_PrepareLoadPE(void)
{
	UWORD idata len;

	if((G_RSPSessionContext.sessionSate != RSPSessionState_StoreMetadata)&&(G_RSPSessionContext.sessionSate != RSPSessionState_ReplaceSessionKeys))   //并没有收到configISDP命令
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}
	
	if(RecData[1] <= 0x82)
	{
		ASN1_GetBERTlvLen((UWORD)&RecData[1], (UWORD*)&len);
		
		G_RSPSessionContext.remainSequence = len;  //剩余序列的长度
	}
	else
	{
		G_RSPSessionContext.remainSequence = ((((UWORD)(RecData[2]))<<16)&0x00FF0000)+(((WORD)RecData[3]<<8)&0x0000FF00)+RecData[4];
	}
	
	G_RSPSessionContext.sessionSate = RSPSessionState_PreparePEDownload;
	
	G_RSPSessionContext.clearDataLen = 0; 
	
	PEGPGlobalVariableInit();
	
	IF_PEAnayleGlobalVariableInit();
}
/*
*@function   :CUSTOMER_LoadProfilePE
*
*@description:处理PE模块
*
*@param      :void
*
*@return     :void
*
*@author     :xuyz
*
*@history    :modify by xuxiaojun@20190218 由于ProfilePE解析时会进行指令拼装，借用RecData，如果此时下发的指令也在RecData中，则可能会造成数据被冲掉，所以ProfilePE的所有数据都是接收在EUICCTempDataBuf中
*/
void CUSTOMER_LoadProfilePE(void)
{
	BYTE idata tmp;
	UWORD idata lenOf86;
	UWORD xdata dataAddr;
	
	if(G_RSPSessionContext.sessionSate != RSPSessionState_PreparePEDownload)   //并没有收到RSPSessionState_PreparePEDownload命令
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED); // 这种位置抛异常，RspSession是不是应该清掉？
	}
	
	dataAddr = EUICCTempDataBuf;
	
	tmp = ASN1_GetBERTlvLen(dataAddr + 1, &lenOf86);  //获取86tag的总长度
	
	lenOf86 += tmp +1 ;  // 86字段的总长度，包括TLV 
	
	if(G_RSPSessionContext.remainSequence  < lenOf86)  //整个PE 包的剩余长度不够一个86tag
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_LOADPROFILEELEMENTS, ErrorReason_incorrectInputValues, ADDR_IS_NULL);
		
		return;
	}
	
	G_RSPSessionContext.remainSequence -= lenOf86;   //86剩余序列的总长度
	
	lenOf86 = SCP03tDecryptAndCalMac(dataAddr,dataAddr,lenOf86 , (SECURITYLEVEL_CMAC|SECURITYLEVEL_CDECRYPTION),&G_RSPSessionContext.macKey[0], 0x10, &G_RSPSessionContext.initMacValue[0],&G_RSPSessionContext.encKey[0],0x10,&G_RSPSessionContext.icvCounter[0]); // 仅返回明文数据长度，不包括mac和padding字节，但是包括TL
	
	if(lenOf86 == FALSE)
	{
		Notification_GenerateProfileInstallResult(BPPCommandId_LOADPROFILEELEMENTS, ErrorReason_scp03tSecurityError,ADDR_IS_NULL);
		
		return;
	}

	SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

	interpreterPE(dataAddr+1+tmp, lenOf86 - tmp - 1/*,ResData 不能用这个数组 会冲掉STK中是数据*/);  //错误数据存放在ResData中,ResData[0]存放的是长度，目前不支持长度超过0xFF的情况.Notification_GenerateProfileInstallResult中会直接使用ResData中的结果进行拼装
	
	SEUICC_ResumeISDPAppletTable();

	tmp = IF_GetPfoileExcuteState();
	
	if(tmp == FALSE)  //profile执行过程中出错
	{		
		Notification_GenerateProfileInstallResult(BPPCommandId_LOADPROFILEELEMENTS, IF_GetProfileExcuteErrorReason(),ADDR_IS_NULL);
		
		return;
	}
	else if(tmp == 0x02)   //PE已经执行成功了，且PE到达end状态
	{
		//NOTE：此处要区分G_RSPSessionContext.remainSequence 是否 为0 ，分两种情况解释
		//1:如果G_RSPSessionContext.remainSequence  == 0 ，表示PE正好全部处理结束
		//2 :如果G_RSPSessionContext.remainSequence != 0, 表示虽然已经处理到PEEnd，但是后续还有多余的数据，暂时也当作成功处理
	
		Notification_GenerateProfileInstallResult(BPPCommandId_LOADPROFILEELEMENTS, RESPONSE_NoError,ADDR_IS_NULL);		
	}
	else
	{
		if(G_RSPSessionContext.remainSequence == 0x00)  //PE还在执行过程中，但是profile已经不会再有后续数据了，如何处理呢？？ by xuyz
		{
		}
	}	
}

/*
*@function   :CUSTOMER_ReceiveErrorAPDU
*
*@description:收到非法的ISD R命令时的处理流程
*
*@param       :void
*
*@return      :void
*
*@author     :xuyz
*
*@history    :V1.0 :Create@20181203
*/
void CUSTOMER_ReceiveErrorAPDU(void)
{
	if(G_RSPSessionContext.sessionSate == RSPSessionState_NULL)
	{
		ThrowError(SW_INS_NOT_SUPPORTED);
	}
	else
	{
		ThrowError(SW_REFERENCED_DATA_NOT_FOUND);  // SGP2.2 5.7.6 If this function is called when there is no RSP session, or if it is called with a TLV that is not expected according to the structure of the Bound Profile Package, the eUICC SHALL return the status words '6A 88' (Reference data not found) or '69 85' (Conditions of use not satisfied) as the response of the transport command defined in section 5.7.2
	}
}

static code INSLIST ISDRTwoTagApdu[]=
{
	{0x3C,CUSTOMER_GetEuiccConfiguredAddresses},  //Tag 'BF3C'  GetEuiccConfiguredAddresses
	{0x3F,CUSTOMER_SetDefaultDpAddress},          //Tag 'BF3F'   SetDefaultDpAddress
	{0x21,CUSTOMER_PrepareDownload},              //Tag 'BF21   PrepareDownload
	{0x2E,CUSTOMER_GetEUICCChallenge},            //Tag 'BF2E'    GetEUICCChallenge
	{0x20,CUSTOMER_GetEUICCInfo},                 //Tag 'BF20'   GetEUICCInfo
	{0x22,CUSTOMER_GetEUICCInfo},                 //Tag 'BF22'   GetEUICCInfo
    {0x28,CUSTOMER_ListNotification},             // Tag 'BF28' ListNotification
    {0x2B,CUSTOMER_RetrieveNotificationsList},    // Tag 'BF2B' RetrieveNotificationsList
	{0x30,CUSTOMER_RemoveNotificationFromList},   // Tag 'BF30'  RemoveNotificationFromList
    {0x35,CUSTOMER_LoadCRL},                      //Tag 'BF35' LoadCRL
    {0x38,CUSTOMER_AuthenticateServer},           // Tag 'BF38'  AuthenticateServer
    {0x41,CUSTOMER_CancelSession},                //Tag 'BF41'  CancelSession
    {0x2D,CUSTOMER_GetProfilesInfo},              // Tag 'BF2D' GetProfilesInfo
	{0x31,CUSTOMER_EnableProfile},                //Tag 'BF31  EnableProfile
    {0x32,CUSTOMER_DisableProfile},               //Tag 'BF32'   DisableProfile 
    {0x33,CUSTOMER_DeleteProfile},                //Tag 'BF33'   DeleteProfile
    {0x34,CUSTOMER_eUICCMemoryReset},             //Tag 'BF34'  eUICCMemoryReset
    {0x3E,CUSTOMER_GetEID},                       //Tag 'BF3E'  GetEID
    {0x29,CUSTOMER_SetNickname},                  //Tag 'BF29  SetNickname
    {0x43,CUSTOMER_GetRAT},                       //Tag 'BF43' GetRAT
	{0x36,CUSTOMER_InitialiseSecureChannel},      //Tag 'BF36  initialiseSecureChannelRequest
  	{0x00,CUSTOMER_ReceiveErrorAPDU},
};


static code INSLIST ISDRProfileDownloadApdu[]=
{
	{0xA0,CUSTOMER_ConfigureISDP},
	{0xA1,CUSTOMER_PrepareStoreMetadata},
    {0x88,CUSTOMER_StoreMetadata},
    {0xA2,CUSTOMER_ReplaceSessionKeys},
    {0xA3,CUSTOMER_PrepareLoadPE},
	{0x86,CUSTOMER_LoadProfilePE},
    {0x00,CUSTOMER_ReceiveErrorAPDU},
};
/*
*@function   :CUSTOMER_ReceiveStoreData
*
*@description:customer中接收storeData APDU的数据
*
*@param      :void
*
*@return     :TRUE :数据接收完全，可以进行后续处理/FALSE: 数据未接受完全
*
*@author     :xuyz
*
*@history    :V1.0 注意在接收数据的时候就要防止接收的数据溢出， 这样后续就不用判断了
*             modify by xuxiaojun@20190218 由于ProfilePE解析时会进行指令拼装，借用RecData，如果此时下发的指令也在RecData中，则可能会造成数据被冲掉，所以ProfilePE的所有数据都是接收在EUICCTempDataBuf中
*/
BYTE CUSTOMER_ReceiveStoreData(void)
{
	/**
	 *modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>
	 *profile下载过程中被ES10打断，下载过程块号会乱序，下载过程无法继续，
	 *A.级联块号接不上情况，本条指令直接报错，下载终止，状态数据清除。
	 *B.级联一个完整快已经结束，下载不能打断， 这些DeleteProfile  DisableProfile EnableProfile  EuiccMemoryReset  SetNickname指令不允许，ES10里其他指令可以操作。
     */
	if(P2 == 0x00)
	{
		if(G_CustomerStoreDataApdu.blockNum == 0x00) // 无store data进行中 , 指令往下走
		{
			G_CustomerStoreDataApdu.storeDataLen = 0x00;

			G_CustomerStoreDataApdu.blockNum  = 0;
		}
		else
		{
			G_CustomerStoreDataApdu.storeDataLen = 0x00;

			G_CustomerStoreDataApdu.blockNum  = 0;

			RSP_abortLoadingSession(); // 新指令打断了其他指令， 清会话，指令继续往下走
		}
	}
	else
	{
		//The eUICC MAY discard its session state if a Profile switch occurs during a Profile download and installation procedure.
		if(G_CustomerStoreDataApdu.blockNum == 0x00)
		{
			ThrowError(SW_WRONG_DATA);// 当前指令格式错误，抛异常
		}
		else if(P2 != G_CustomerStoreDataApdu.blockNum)
		{
			G_CustomerStoreDataApdu.storeDataLen = 0x00;

			G_CustomerStoreDataApdu.blockNum  = 0;

			ThrowError(SW_WRONG_DATA);// 自身的块号不正确，抛异常
		}
		else // P2 == G_CustomerStoreDataApdu.blockNum
		{
			// 当前指令块号正确，继续往下执行
		}
	}
	// 块号正确。 级联一个完整快已经结束，下载不能打断， 这些DeleteProfile  DisableProfile EnableProfile  EuiccMemoryReset  SetNickname指令不允许，ES10里其他指令可以操作
	if((G_RSPSessionContext.sessionSate == RSPSessionState_PrepareStoreMetadata)) //后续将接收storeMetadata 序列 ,统一放在E2中去处理
	{
		if((G_RSPSessionContext.clearDataLen + G_CustomerStoreDataApdu.storeDataLen + P3) > EUICCTempDataBufSize) // 注意在接收数据的时候就要防止接收的数据溢出
		{
			RSP_abortLoadingSessionThrowError(SW_FILE_FULL);
		}

		VMWriteBuf(EUICCTempDataBuf + G_RSPSessionContext.clearDataLen + G_CustomerStoreDataApdu.storeDataLen, RecData, P3);   //对于多块的命令，暂时更新数据到E2中

		G_CustomerStoreDataApdu.storeDataLen += P3;

		if((P1&0x80) == 0x00)
		{
			G_CustomerStoreDataApdu.blockNum ++;

			return FALSE;  //more blocks，继续接收后续数据
		}
		else
		{
			G_CustomerStoreDataApdu.blockNum = 0x00;

			return TRUE;
		}
	}
	else if(G_RSPSessionContext.sessionSate == RSPSessionState_PreparePEDownload)// 由于ProfilePE解析时会进行指令拼装，借用RecData，如果此时下发的指令也在RecData中，则可能会造成数据被冲掉，所以ProfilePE的所有数据都是接收在EUICCTempDataBuf中
	{
		if((G_CustomerStoreDataApdu.storeDataLen + P3) > EUICCTempDataBufSize) // 注意在接收数据的时候就要防止接收的数据溢出
		{
			RSP_abortLoadingSessionThrowError(SW_FILE_FULL);
		}

		VMWriteBuf(EUICCTempDataBuf + G_RSPSessionContext.clearDataLen + G_CustomerStoreDataApdu.storeDataLen, RecData, P3);   //对于多块的命令，暂时更新数据到E2中

		G_CustomerStoreDataApdu.storeDataLen += P3;

		if((P1&0x80) == 0x00)
		{
			G_CustomerStoreDataApdu.blockNum ++;

			return FALSE;  //more blocks，继续接收后续数据
		}
		else
		{
			G_CustomerStoreDataApdu.blockNum = 0x00;

			return TRUE;
		}
	}
	else
	{
		if((P2 == 0x00) && ((P1&0x80) == 0x80))
		{
			G_CustomerStoreDataApdu.storeDataLen = P3; //直接去处理命令数据即可

			return TRUE;
		}

		if((G_CustomerStoreDataApdu.storeDataLen + P3) > EUICCTempDataBufSize) // 注意在接收数据的时候就要防止接收的数据溢出
		{
			RSP_abortLoadingSessionThrowError(SW_FILE_FULL);
		}

		VMWriteBuf(EUICCTempDataBuf + G_CustomerStoreDataApdu.storeDataLen, RecData, P3);   //对于多块的命令，暂时更新数据到E2中

		G_CustomerStoreDataApdu.storeDataLen += P3;

		if((P1&0x80) == 0x00)
		{
			G_CustomerStoreDataApdu.blockNum ++;

			return FALSE;  //more blocks，继续接收后续数据
		}
		else
		{
			if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)  //如果一个storeData序列虽然可以分多条发送，但是如果总数据长度小于255 ，依旧放在RecData中去处理
			{
				P3 = G_CustomerStoreDataApdu.storeDataLen;

				VMReadBuf(RecData, EUICCTempDataBuf, P3);
			}

			G_CustomerStoreDataApdu.blockNum = 0x00;

			return TRUE;
		}
	}

	return TRUE;
}

/*
*@function    :NM_CUSTOMERISDRStoreData
*
*@description :执行RSP 版本ISD R下的命令，主要包括 5.5 ES8+ 5.7 ES10x
*
*@SEE         :<SGP22_V2.2 >  5.5 ES8+ 5.7 ES10x
*
*@param       :void
*
*@return      :void
*
*@author      :xuyz
*
*@history     :V1.0:create @
*             :V1.1:modify by xuxiaojun@20190603 由于ProfilePE解析时会进行指令拼装，借用RecData，如果此时下发的指令也在RecData中，则可能会造成数据被冲掉，所以ProfilePE的所有数据都是接收在EUICCTempDataBuf中
*			  :V1.2modify by liguiyu@20190715 增加eSE接口函数的调用入口 ，tag eSE_Private_TagFirstByte
*/
void NM_CUSTOMERISDRStoreData(void)
{

	UWORD xdata dataStartAddr;
	BYTE idata tmp,i,tmp2;
	
	if((P1 != 0x11) && (P1 != 0x91) && (P1 != 0x90))   //对于enable disable delete这几条命令而言，当P1 = 90 时，表示当前是case3命令，不需要返回响应 
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(CUSTOMER_ReceiveStoreData() == FALSE)
	{
		return;
	}
	
	if(G_RSPSessionContext.sessionSate == RSPSessionState_PrepareStoreMetadata) 
	{
		dataStartAddr = EUICCTempDataBuf + G_RSPSessionContext.clearDataLen;
	}
	else if(G_RSPSessionContext.sessionSate == RSPSessionState_PreparePEDownload)
	{
		dataStartAddr = EUICCTempDataBuf;
	}
	else if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)
	{
		dataStartAddr = (UWORD)RecData;
	}	
	else
	{
		dataStartAddr = EUICCTempDataBuf;
	}	
	
	tmp = VMReadByte(dataStartAddr);  //读取tag的第一个字节
	
	if((tmp&0x1F) == 0x1F)   //TAG为2字节
	{
		if(tmp == 0xBF)//SGP22 标准api定义的tag
		{
			tmp = VMReadByte(dataStartAddr + 1);   //读取两个tag的第二个字节
			
			for(i=0;  ;i++)
			{
				if((tmp != 0x31)&&(tmp != 0x32)&&(tmp != 0x33)&&(P1 == 0x90)) // 除EnableProfile/DisableProfile/DeleteProfile要求支持case3类型的指令，其他的都不允许支持, 根据< RSP_EUICC测试_管理指令测试_管理指令异常测试\eUICCMemoryReset_TSE05.html>类推
				{                                                                 // <RSP_EUICC测试_管理指令测试_管理指令异常测试\ListNotification_TSE03.html>
					ThrowError(SW_WRONG_DATA);
					
					return;
				}
		
				if((ISDRTwoTagApdu[i].ins == tmp)||(ISDRTwoTagApdu[i].ins == EOL))
				{
					/*SGP22_V2.1: While a Profile state change is ongoing, i.e. a command was sent to the eUICC which
					mandates a REFRESH, but the REFRESH was not yet successfully executed (i.e. no
					Terminal Response with result "command performed successfully" received or reset of the
					eUICC), the eUICC MAY reject any other ES10 command with the status word '69 85'
					(Conditions of use not satisfied). //add by chentao@20230209
					*/
					/*if(IF_ProfielChangeOngoing() == TRUE) // 这个限制提前到了ISDR的process处理
					{
						ThrowError(SW_CONDITIONS_NOT_SATISFIED);

						return;
					}*/
					tmp2 = VMReadByte(dataStartAddr + 1);
					// 20240411移远无人机反馈：重复扫描同一个二维码 下载profile时，第二次下载失败后再执行激活失效时，期望成功实际失败。原因是第二次下载失败后，服务器未给卡片任何数据，导致IF_IsProfileLoadingSession(G_RSPSessionContext.sessionSate >= RSPSessionState_Init)条件一直满足
										// 因此把原方案"profile下载过程中DeleteProfile、DisableProfile、EnableProfile、EuiccMemoryReset、SetNickname指令不允许，ES10里其他指令可以操作"这几条指令采用新方案处理
					if((tmp2 == 0x31) || (tmp2 == 0x32) || (tmp2 == 0x33) || (tmp2 == 0x34) || (tmp2 == 0x29))
					{
						RSP_abortLoadingSession(); // 新指令打断了其他指令， 清会话，指令继续往下走
					}

					ISDRTwoTagApdu[i].func();
					break;
				}                           			
			}
		}
#if  EUICC_FUNC_eSE_INTERFACE
		else if((tmp&eSE_Private_TagFirstByte) == eSE_Private_TagFirstByte)//eSE 私有API定义的TAG为DF或FF
		{
			tmp = VMReadByte(dataStartAddr + 1);   //读取两个tag的第二个字节
			
			if(tmp == 0x7E) // 德国客户需求：保存sessionkey
			{
				CUSTOMER_Getsessionkey();
			}
			else if(tmp == 0x7D)  // 德国客户需求：获取含有冲突应用的profile信息
			{
				CUSTOMER_GetProfileInfo();
			}

			else
			{
				CallEseInterfaceFunc(tmp);
			}
			
			return ;
		}
#endif
		else
		{
			ThrowError(SW_WRONG_DATA);
		}
	}
	else
	{
		if(P1 == 0x90) // 除EnableProfile/DisableProfile/DeleteProfile要求支持case3类型的指令，其他的都不允许支持, 根据< RSP_EUICC测试_管理指令测试_管理指令异常测试\eUICCMemoryReset_TSE05.html>类推
		{
			ThrowError(SW_WRONG_DATA);
			
			return;
		}
	
		for(i=0;  ;i++)
		{
			if((ISDRProfileDownloadApdu[i].ins == tmp)||(ISDRProfileDownloadApdu[i].ins == EOL))
			{
				ISDRProfileDownloadApdu[i].func();
				break;
			}                           			
		}
	}
	
}


/*
*@function   : CUSTOMER_UpdateMetadata
*
*@description: This function allows to update the following Profile Metadata of the targeted Profile
*
*@see        :SGP22_V2.2 5.4.1  Function: UpdateMetadata 
*
*@param      :void
*
*@return     :void
*
*@history    :created by xuyz
*            :modify by xuxiaojun@20190703  代码优化
*            :modify by xuxiaojun@20190709  增加子TAG的边界值判断
*/
//modify by xieyong@20220914 增加0xBF22tag
BYTE code updateMetaTag[] = {TAG_SERTVICEPROVIDERNAME, TAG_PROFILENAME, TAG_ICONTYPE, TAG_ICON,TAG_PROFILEPOLICYRULES
#if RSP_SGP22_BF22
		,TAG_SERVICESPECIFICDATASTOREDINEUICC
#endif
};
BYTE code updateMetaTagOffset[] =
{
	MetaDataServiceProviderNameRef_INDEX,
	MetaDataProfileNameRef_INDEX,
	MetaDataIconType_INDEX,
	MetaDataIconRef_INDEX,
	MetaDataProfilePolicyRules_INDEX
#if RSP_SGP22_BF22
	,MetaDataserviceSpecificDataStoredInEuicc_INDEX
#endif
};
/*
*@function     :CUSTOMER_UpdateMetadata
*
*@description  :This function gets the EID from the eUICC
*
*@see          :<SGP22_V2.2 > 5.7.20  Function (ES10c): GetEID
*
*@param        :void
*
*@return       :void
*
*@test         :自测通过
*
*@RAM Resources:响应数据直接放在RecData中发送出去
*
*@author       :xuxiaojun@20181112
*
*@hisotry      :created by xuxiaojun@20181112
*/
void CUSTOMER_UpdateMetadata(void)
{
	UWORD xdata dataStartAddr,totalLen,tmpLen;
	BYTE idata i,tmp,flag, curPPrs,profileIndex, pprUpdateControl;
	WORD idata offset;
	WORD idata dataOffset[sizeof(updateMetaTag)];   //dataOffset[0] :  serviceProviderName  dataOffset[1] :  profileName     dataOffset[2] : iconType:  dataOffset[3] : icon  :dataOffset[3] : profilePolicyRules
	if(IF_ProfileLoadingOrChangeOngoing())
	{
		G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;

		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}

	if(IF_CapLoaingInCurrnetProfile() || IF_MoreCommandsInCurrnetProfile())
	{
		G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;

		ThrowError(SW_CONDITIONS_NOT_SATISFIED);
	}

	if((P1 != 0x10) && (P1 != 0x90))   //Table 33: UpdateMetadata Command Message
	{
		G_StoreDataReservedProfileIndex = ILLEGAL_PROFILEINDEX;

		ThrowError(SW_INCORRECT_P1P2); //<TY_Offline_RSP_ManageCmdTest\UpdateMetadata_TSE_12_13_14_15>
	}
	
	if(CUSTOMER_ReceiveStoreData() == FALSE)
	{
		return;
	}
	
	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)  // 支持级联
	{
		dataStartAddr = (UWORD)RecData;
	}
	else
	{
		dataStartAddr = EUICCTempDataBuf;
	}
	
	VMReadBuf(tempBuf, dataStartAddr, 0x02);
	
	if((tempBuf[0] != 0xBF) || (tempBuf[1] != 0x2A))
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	profileIndex = G_CurEnabledProfileIndex;
	
	ecpMem_Set(dataOffset, 0x00 ,sizeof(dataOffset));
	
	tmp = ASN1_GetBERTlvLen(dataStartAddr + 2, (UWORD*)&totalLen);
	
	flag = FALSE;
	
	for(i=0, offset = 0; (offset < totalLen)&&(i < sizeof(updateMetaTag) ); )
	{
		if(i == 0x00)
		{
			dataOffset[0] = 0x02 + tmp;  //dataOffset[0] 表示the first tag
		}
		else
		{
			if(flag == FALSE)
			{
				dataOffset[i] = dataOffset[i-1] + tmpLen + tmp + 1;		
			}
		}
		
		flag = FALSE;
#if RSP_SGP22_BF22
		if((VMReadByte(dataStartAddr + dataOffset[i]) == 0xBF)&&(VMReadByte(dataStartAddr + dataOffset[i]+1) == 0x22))   //BF22
		{
			dataOffset[i]++;
			offset++;
		}
#endif
		
		if(VMReadByte(dataStartAddr + dataOffset[i]) == updateMetaTag[i])
		{
			tmp = ASN1_GetBERTlvLen( dataStartAddr + dataOffset[i] + 1  , (UWORD*)&tmpLen );
			
			if((i == 0x02) || (i == 0x04))
			{
				if(tmp != 0x01)  //icon 和ppr 字段的长度都只占一个字节
				{
					ThrowError(SW_WRONG_DATA);
				}
			}
		}
		else
		{
			dataOffset[i + 1] = dataOffset[i];
			
			dataOffset[i] = 0;			
			
			i ++;
			
			flag = TRUE;
			
			continue;
		}
		
		offset +=  tmpLen + tmp + 1;
	
		i ++;	
	}
	
	if(offset != totalLen)  //存在其他多余的信息
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	flag = FALSE;
	
	for(i= 0; i<sizeof(updateMetaTag); i++)
	{
		if(dataOffset[i] != FALSE)
		{
			flag = TRUE;
			
			break;
		}
	}
	
	if(flag == FALSE)   //serviceProviderName', 'profileName', 'iconType', 'icon' and 'profilePolicyRules' fields are all optional. But at least one field SHALL be present
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(dataOffset[0] != 0x00)
	{
		tmp = ASN1_GetBERTlvLen(dataStartAddr + dataOffset[0] + 1 , (UWORD*)&tmpLen );  //获取serviceProviderName 字段的长度
		
		if(tmpLen > ServiceProviderNameMaxLen)
		{
			ThrowError(SW_WRONG_DATA);
		}
	}
	
	if(dataOffset[1] != 0x00)
	{
		tmp = ASN1_GetBERTlvLen(dataStartAddr + dataOffset[1] + 1 , (UWORD*)&tmpLen );  //获取profileName 字段的长度
		
		if(tmpLen > ProfileNameMaxLen)
		{
			ThrowError(SW_WRONG_DATA);
		}
	}
	
	if(dataOffset[2] != 0x00) //获取iconType 字段的长度
	{
		tmp = ASN1_GetBERTlvLen(dataStartAddr + dataOffset[2] + 1 , (UWORD*)&tmpLen );  //获取iconType 字段的长度
		
		if(tmpLen > 0x01)
		{
			ThrowError(SW_WRONG_DATA);
		}
	}
		
	if((dataOffset[2] != 0x00)&&(dataOffset[3] != 0x00))
	{
		tmp = ASN1_GetBERTlvLen(dataStartAddr + dataOffset[3] + 1 , (UWORD*)&tmpLen );  //获取icon 字段的长度
		
		if(tmpLen > IconMaxLen)
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		if(VMReadByte(dataStartAddr + dataOffset[2] + 1)  != 0x00)  //iconType字段的长度不为0 ，但是icon的长度为0 
		{
			if(tmpLen == 0x00)
			{
				ThrowError(SW_WRONG_DATA);
			}
		}
		else         //iconType的长度为0  ，但是icon字段的长度不为0
		{
			if(tmpLen != 0x00)
			{
				ThrowError(SW_WRONG_DATA);
			}
		}
	}
	else if((dataOffset[2] == 0x00)&&(dataOffset[3] == 0x00))  //'iconType' and 'icon' SHALL be both present and contain non-empty values, be both present and contain empty values (i.e. TLV objects with length zero),
	{
		//do nothing
	}
	else
	{
		ThrowError(SW_WRONG_DATA);
	}
	
	if(dataOffset[4] != 0x00)  //存在profilePolicyRules字段
	{
		pprUpdateControl = VMReadByte(dataStartAddr + dataOffset[4] + 1); // 复用pprUpdateControl，后面会将其值覆盖
		
		if(pprUpdateControl == 0x02)  //L
		{
			pprUpdateControl = VMReadByte(dataStartAddr + dataOffset[4] + 3); 
			 
			if((pprUpdateControl & 0x80 ) == 0x80)  // For this operation, the pprUpdateControl bit SHALL be set to zero. A value of one SHALL be treated as an error. 
			{
				ThrowError(SW_FUNC_NOT_SUPPORTED);  // <ES6 (Operator -- eUICC): UpdateMetadata\TC_eUICC_ES6_UpdateMetadata_07_Error_Set_a_pprUpdateControl_value_to_one>
			}
			
			curPPrs = GetSingleBYTEProfileMeta(profileIndex, MetaDataProfilePolicyRules_INDEX,3);
			
			curPPrs = (curPPrs & pprUpdateControl);
		}
		else if(pprUpdateControl <= 0x01)
		{		
			curPPrs = 0x00;  	//清空pprs
		}
		else
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		flag = 5;  //以flag表示bit string的填充字节数 
		
		if((curPPrs&0x20) == 0x00)
		{
			flag ++;
			
			if((curPPrs&0x40) == 0x00)
			{
				flag = 8;  //因为 pprUpdateControl 字段经过这个 操作 之后必须为 0
			}
		}
		tempBuf[0] = 0x02;
		
		tempBuf[1] = flag;
		
		tempBuf[2] = curPPrs;
		
		VMWriteBuf(dataStartAddr + dataOffset[4] + 1, &tempBuf[0], 3);
	}
	
	for(i = 0; i<sizeof(updateMetaTag);  i++)
	{
		if(dataOffset[i] != 0x00)
		{
			tmp = ASN1_GetBERTlvLen(dataStartAddr + dataOffset[i] + 1 , (UWORD*)&tmpLen );
			
			tmpLen += tmp + 1;  //该字段所占的总长度
			
#if RSP_SGP22_BF22
			if(i == 5)
			{
				tmpLen += 1;

				dataOffset[i] -= 1;
			}
#endif

			//VMReadBuf(tempBuf, dataStartAddr + dataOffset[i], tmpLen);
			
			if((i == 2) ||(i == 4))  //存储 iconType 和 pprs 的TLV
			{
				VMReadBuf(tempBuf, dataStartAddr + dataOffset[i], tmpLen);

				StoreProfileMetadataTLV(tempBuf, tmpLen , updateMetaTagOffset[i], profileIndex);
			}
			else
			{
				StoreProfileMetadataRef((BYTE*)(dataStartAddr + dataOffset[i]) ,tmpLen, updateMetaTagOffset[i], profileIndex );
			}
		}
	}
}

/*
*@function   : CUSTOMER_ISDR_GetResponse
*
*@description: 处理ISD-R的取响应 INS = C0 由于目前遇到的响应数据超过256字节的基本都是 ISD-R的管理指令，为了兼容多通道交互发指令 且不影响下一次C0取数据的情况，
				将ISD-R的响应单独加一条INS处理，不影响原有的收发机制，目前只考虑卡片最多与1个LPA交互的情况，即除了与LPA交互的通道，其他通道不会去选择ISD-R对其发送APDU
*
*@see        :
*
*@param      :void
*
*@return     :void
*
*@author     :V1.0 by lgy@20190703
*
*
*@history    :V1.0
*/
void ISDR_GetResponse(void)
{

	WORD idata recOffset,sendLen;//recOffset声明成word型 是因为它表示的是下一次取数据时存放在RecData中的偏移，如果第一次刚好取了0x100字节 recOffset就会等于0x100
	BYTE idata findTag;
	UWORD xdata euiccCertAddr,eumCertAddr,notifyAddr;
	SendFileExtendBackData xdata backStruct;
	WORD  idata euiccEumRef;//, eumRef,euiccRef;

	if(G_SendFileExtendBackDataStruct.totalLen == 0)
	{
		ThrowError(SW_WRONG_LENGTH); //没有响应值可以取
	}

	ecpMem_Copy((BYTE*)&backStruct,(BYTE*)&G_SendFileExtendBackDataStruct,sizeof(SendFileExtendBackData));//备份信息，防止数据不够发6CXX的情况

	findTag = FALSE;

	euiccCertAddr = ADDR_IS_NULL;
	eumCertAddr = ADDR_IS_NULL;
	notifyAddr = ADDR_IS_NULL;
	recOffset = 0;

	sendLen = P3;//本次要取的数据长度

	if(P3 == 0)
	{
		sendLen = 0x100;
	}

	if(G_SendFileExtendBackDataStruct.totalLen < sendLen)//要取的数据大于剩余总长度 直接发6CXX出去
	{
		ThrowError(0x6C00|G_SendFileExtendBackDataStruct.totalLen);

		return ;
	}

	while(1)//查找本次需要发送的数据
	{
		switch(G_SendFileExtendBackDataStruct.curretnSendStep)
		{
			case SendStep_BackAddr:

				if(G_SendFileExtendBackDataStruct.len > (sendLen-recOffset))//剩余长度足够本次发送，且不需要修改下一次查找数据的起始地址
				{
					VMReadBuf(&RecData[recOffset],G_SendFileExtendBackDataStruct.backAddr+G_SendFileExtendBackDataStruct.backOffset,(sendLen-recOffset));

					G_SendFileExtendBackDataStruct.len  -= (sendLen-recOffset);

					G_SendFileExtendBackDataStruct.backOffset += (sendLen-recOffset);

					G_SendFileExtendBackDataStruct.totalLen -= (sendLen-recOffset);

					findTag = TRUE;

					break;
				}
				else//拷贝本次剩余数据 ，继续查找下一个step的数据，把(G_SendFileExtendBackDataStruct.len = (sendLen-recOffset))的情况在else里处理 是为了取完数据后设置下一次查找的起始地址和长度等信息
				{
					VMReadBuf(&RecData[recOffset],G_SendFileExtendBackDataStruct.backAddr+G_SendFileExtendBackDataStruct.backOffset,G_SendFileExtendBackDataStruct.len);

					recOffset += G_SendFileExtendBackDataStruct.len;

					G_SendFileExtendBackDataStruct.totalLen -= G_SendFileExtendBackDataStruct.len;

					G_SendFileExtendBackDataStruct.len = 0;

					G_SendFileExtendBackDataStruct.backOffset = 0;
				}

				if(G_SendFileExtendBackDataStruct.euiccCertRef != REF_NULL)
				{
					euiccCertAddr = get_array_data(MEMtPF8(locateObjectInMemory(G_SendFileExtendBackDataStruct.euiccCertRef)));

					G_SendFileExtendBackDataStruct.len = VMReadShort(euiccCertAddr);//下一段数据的总长度
				}

				G_SendFileExtendBackDataStruct.curretnSendStep = SendStep_EUICCCert;

				if(sendLen == recOffset)//本次在SendStep_BackAddr中查找完所有待发送数据 跳出循环
				{
					findTag = TRUE;

					break;
				}
				/* no break */

			case SendStep_EUICCCert:

				if(G_SendFileExtendBackDataStruct.euiccCertRef != REF_NULL)//证书地址不为0，G_SendFileExtendBackDataStruct.len的值就是本次证书的剩余待取长度
				{
					euiccCertAddr = get_array_data(MEMtPF8(locateObjectInMemory(G_SendFileExtendBackDataStruct.euiccCertRef)))+2;

					if(G_SendFileExtendBackDataStruct.len > (sendLen-recOffset))
					{
						VMReadBuf(&RecData[recOffset],euiccCertAddr+G_SendFileExtendBackDataStruct.backOffset,(sendLen-recOffset));

						G_SendFileExtendBackDataStruct.len -= (sendLen-recOffset);

						G_SendFileExtendBackDataStruct.backOffset += (sendLen-recOffset);

						G_SendFileExtendBackDataStruct.totalLen -= (sendLen-recOffset);

						findTag = TRUE;

						break;
					}
					else//EUICC证书的数据取完 ，继续取EUM证书的数据
					{
						VMReadBuf(&RecData[recOffset],euiccCertAddr+G_SendFileExtendBackDataStruct.backOffset,G_SendFileExtendBackDataStruct.len);

						recOffset += G_SendFileExtendBackDataStruct.len;

						G_SendFileExtendBackDataStruct.totalLen -= G_SendFileExtendBackDataStruct.len;

						G_SendFileExtendBackDataStruct.len = 0;

						G_SendFileExtendBackDataStruct.backOffset = 0;
					}
				}

				if(G_SendFileExtendBackDataStruct.eumCertRef != REF_NULL)
				{
					eumCertAddr = get_array_data(MEMtPF8(locateObjectInMemory(G_SendFileExtendBackDataStruct.eumCertRef)));

					G_SendFileExtendBackDataStruct.len = VMReadShort(eumCertAddr);
				}

				G_SendFileExtendBackDataStruct.curretnSendStep = SendStep_EUMCert;

				if(sendLen == recOffset)
				{
					findTag = TRUE;

					break;
				}

			case SendStep_EUMCert:

				if(G_SendFileExtendBackDataStruct.eumCertRef != REF_NULL)
				{
					//eumCertAddr = G_AddrIndexTablePointer[G_SendFileExtendBackDataStruct.eumCertRef-1].dataAddr + 2;
					eumCertAddr = get_array_data(MEMtPF8(locateObjectInMemory(G_SendFileExtendBackDataStruct.eumCertRef)))+2;

					if(G_SendFileExtendBackDataStruct.len > (sendLen-recOffset))
					{
						VMReadBuf(&RecData[recOffset],eumCertAddr+G_SendFileExtendBackDataStruct.backOffset,(sendLen-recOffset));

						G_SendFileExtendBackDataStruct.len -= (sendLen-recOffset);

						G_SendFileExtendBackDataStruct.backOffset += (sendLen-recOffset);

						G_SendFileExtendBackDataStruct.totalLen -= (sendLen-recOffset);

						findTag = TRUE;

						break;
					}
					else//数据不够 查找下一条通知（如果有通知存在）
					{
						VMReadBuf(&RecData[recOffset],eumCertAddr+G_SendFileExtendBackDataStruct.backOffset,G_SendFileExtendBackDataStruct.len);

						recOffset += G_SendFileExtendBackDataStruct.len;//recOffset定义成BYTE时 如果G_SendFileExtendBackDataStruct.len刚好是100字节时会截断数据

						G_SendFileExtendBackDataStruct.totalLen -= G_SendFileExtendBackDataStruct.len;

						G_SendFileExtendBackDataStruct.len = 0;

						G_SendFileExtendBackDataStruct.backOffset = 0;
					}

				}

				if(G_SendFileExtendBackDataStruct.notifiyNum > 0) //设置下一次数据查找的起始信息
				{
					notifyAddr = locateObjectInMemory(G_SendFileExtendBackDataStruct.notifiy[0]);

					euiccEumRef = memReadU16(MEMtPM8(GetNotificationList_euiccEumRefAddr(notifyAddr)));

					if(VMReadByte(GetNotificationList_notificationTypeAddr(notifyAddr)) == NOTIFICATION_TYPE_OTHER)//30LV ,L包含了2个证书的长度，但是内容没有包含
					{
						if(euiccEumRef != REF_NULL)//正常情况OTHER类型通知一定会存在证书
						{
							eumCertAddr =locateObjectInMemory(euiccEumRef);

							G_SendFileExtendBackDataStruct.euiccCertRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_curCertEUICCArrayAddr(eumCertAddr)));

							G_SendFileExtendBackDataStruct.eumCertRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_curCertEUMArrayAddr(eumCertAddr)));
						}
					}
					else //final result类型通知
					{
						G_SendFileExtendBackDataStruct.euiccCertRef = REF_NULL;

						G_SendFileExtendBackDataStruct.eumCertRef = REF_NULL;
					}

					G_SendFileExtendBackDataStruct.backAddr = GetNotificationList_notificationData_ArrayAddr(notifyAddr);//notifyAddr + NotificationList_notificationData_Offset;

					G_SendFileExtendBackDataStruct.len = memReadU16(MEMtPM8(GetNotificationList_notificationDataSizeAddr(notifyAddr)));//newNorificationHead.notificationDataSize;//通知的长度 不包含证书

					G_SendFileExtendBackDataStruct.notifiyNum--;

					ecpMem_Move((BYTE*)&G_SendFileExtendBackDataStruct.notifiy[0],(BYTE*)&G_SendFileExtendBackDataStruct.notifiy[1],G_SendFileExtendBackDataStruct.notifiyNum*2);

				}
				else
				{
					if(G_SendFileExtendBackDataStruct.totalLen != 0)
					{
						ThrowError(SW_WRONG_LENGTH);//数据异常
					}
				}

				G_SendFileExtendBackDataStruct.curretnSendStep = SendStep_BackAddr;

				if(sendLen == recOffset)
				{
					findTag = TRUE;	//用于跳出while循环
				}

				break;//继续从第一步查找数据

			default:
				ThrowError(SW_WRONG_DATA);
				break;
		}

		if(findTag == TRUE)
		{
			break;
		}
	}

	//GlobalSendData();//发送数据
	//SendDataSingle();
	
	apdu_ApduSetOutgoingAndSend(5,(P3==0)?0x100:P3);

	if(G_SendFileExtendBackDataStruct.totalLen == 0)
	{
		ecpMem_Set((BYTE*)&G_SendFileExtendBackDataStruct,0,sizeof(SendFileExtendBackData));

		G_SendFileExtendBackDataStruct.currentChannel = ILLEGAL_CHANNEL;
	}
	else if(G_SendFileExtendBackDataStruct.totalLen > 0x100)//发送61XX
	{
		ThrowError(0x6100);
	}
	else
	{
		ThrowError(0x6100|G_SendFileExtendBackDataStruct.totalLen);
	}

}
#endif
/*
*@function   : NM_GetISDRSelectOtherResponse
*
*@description: 选择 ISD-R 时，返回响应数据
*
*@pram        :void
*
*@return      :void
*
*@RAM         :tempBuf[0...23], 仅在该函数内部使用
*
*@history     :V1.0 created by xuyunzhen@20190109
*             :V1.1 modify by xuxiaojun@20190614 代码优化，去掉局部变量euiccInfo_section1, 由个人化保证EUICCINFO2_SECTION1必须写入
*/
void NM_GetISDRSelectOtherResponse(void)
{
	WORD idata offset,arraryRef;
	UWORD xdata arrayAddr;
#if RSP_EUICC
	VMReadBuf(&tempBuf[8], EUICCINFO2_SECTION1, EUICCINFO2_SECTION1_LEN);
#else
	ecpMem_Set(&tempBuf[8],0,EUICCINFO2_SECTION1_LEN);
#endif
	G_PersonalProfileState = 0;  //为支持M2M版本的预置profile，在选择ISD R 时，将预置profile的状态会话数据进行清空，供后续使用； 由于不想增加新借口，暂时将该变量的初始化放在这里处理

	offset  = PopOperandStackShort();	//从操作数栈中pop出参数：目标数组偏移

	arraryRef  = PopOperandStackRef();  	//从操作数栈中pop出参数：目标数组

	arrayAddr = (memref)(MEMtPX8(get_array_data(MEMtPF8(locateObjectInMemory(arraryRef))))  + offset);//获取apduBuf数组对象头起始地址

	tempBuf[0] = 0xA5;
	tempBuf[1] = 0x04;
	tempBuf[2] = 0x9F;
	tempBuf[3] = 0x65;
	tempBuf[4] = 0x01;
	tempBuf[5] = 0xFF;
	offset = 6;
#if RSP_EUICC // modify by chentao@20220818 E0 tag is only for RSP
	tempBuf[6] = 0xE0;   //SGP22_V2.2 5.7.1  ISD-R Selection and LPAe Activation
	tempBuf[7] = 0x05;

	ecpMem_Move(&tempBuf[8], &tempBuf[14], 0x05);  //拷贝svnversion

	offset = 13;

	//未拼装 lpaeSupport字段，该字段是可选的
#endif
	VMWriteBuf(arrayAddr,&tempBuf[0],offset);

    PushOperandStackShort(offset);
}
#if  EUICC_FUNC_eSE_INTERFACE
/*
*@function     : CUSTOMER_Getsessionkey
*
*@description  :get session key
*
*@see          :
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:NO
*
*@hisotry      V1.0 :created by chentao@20220107
*
*/
void CUSTOMER_Getsessionkey(void)
{
	BYTE    xdata    ICVBuffer[0x10];
	if(Germany_Mobile)
	{
		APDUBuffer[APDU_DATA_OFFSET] = 0xDF;
		APDUBuffer[APDU_DATA_OFFSET+1] = 0x7E;
		APDUBuffer[APDU_DATA_OFFSET+2] = 0x30;
		memCopyToRam(MEMtPX8(&APDUBuffer[APDU_DATA_OFFSET+3]),MEMtPM8(EUICC_SessionKey), 0x30);

		ecpMem_Set(&ICVBuffer[0],0,0x10); //icv 为全0

		AESCBC((&APDUBuffer[APDU_DATA_OFFSET+3]), 0x30, MEMtPM8(sysConfig_GetBootKeyValueBase()), 0x10, &ICVBuffer[0], 0x10, MODE_ENCRYPT,&APDUBuffer[APDU_DATA_OFFSET+3]); // 加密

		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET,0x33);
	}
	else
	{
		ThrowError(SW_WRONG_DATA);
	}

	return ;
}

/*
*@function     : CUSTOMER_GetProfileInfo
*
*@description  :get profile info
*
*@see          :
*
*@param        :void
*
*@return       :void
*
*@RAM Resources:APDUBuffer
*
*@hisotry      V1.0 :created by xieyong@20220211
*
*/
void CUSTOMER_GetProfileInfo(void)
{
	if(Germany_Mobile)
	{
		BYTE idata len;

		memCopyToRam(&len,MEMtPM8(EUICC_ICCIDAndAID), 0x01);

		if(len == 0x00)
		{
			return;
		}

		APDUBuffer[APDU_DATA_OFFSET] = 0xDF;
		APDUBuffer[APDU_DATA_OFFSET+1] = 0x7D;

		memCopyToRam(MEMtPX8(&APDUBuffer[APDU_DATA_OFFSET+2]),MEMtPM8(EUICC_ICCIDAndAID), len+1);

		SetEepromEX(EUICC_ICCIDAndAID,0x00,len+1);   //获取profile信息后清空所有信息

		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET,len+3);
	}
	else
	{
		ThrowError(SW_WRONG_DATA);
	}

	return;
}
#endif //#if  EUICC_FUNC_eSE_INTERFACE
#endif

