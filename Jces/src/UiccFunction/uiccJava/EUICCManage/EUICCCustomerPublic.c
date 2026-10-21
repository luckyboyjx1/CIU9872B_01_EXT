/*
*@author     : lgy
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC 个人版的操作的一些公共函数
*
*@history     :V1.0 :create @ 20181113
*/

#include "EUICCCustomerPublic.h"
#include "ASN1Operation.h"
#include "EUICCCustomer.h"
#include "EUICCMain.h"
#include "interpreter.h"
#include "Exception.h"
#include "EUICCProfile.h"
#include "EUICCCustomerCertificate.h"
#include "T0ApduImpl.h"
#include "SCP03Realize.h"
#include "EUICCInterface.h"
#include "cardManager.h"
#include "SEEUICCInterface.h"
#include "Platform_api.h"
#include "object.h"
#include "ob_manager.h"
#include "opfunction.h"
#include "VMAccessEeprom.h"
#include "euiccConfig.h"
#include "filesystem.h"
#include "fileAction.h"
#include "Interface_LTEFromPlatform.h"
#include "gsmpublic.h"
#include  "ProfileInterpreter.h"
#include "profileApi.h"

#include "gsmpublic.h"	//remove warning,dengyiwen,2022/9/27
#include "ProfileInterpreter.h"	//remove warning,dengyiwen,2022/9/27

#if RSP_EUICC
/*
*@fucntion    : ProfileChangeDisable_GernerateNotifyAndSetState
*
*@description : Disable profile ，包括产生通知，设置对应的profile状态信息
*
*@param       : BYTE targetProfileIndex
*@param       : BYTE mode               --- 当前执行了disable profile操作之后, refresh的模式:MODE_RESET表示需要复位，所有的环境切换要等到复位之后才启用
*
*@return       :BYTE newEnableProfileIndex
*
*@author      :xuyz
			  :V1.1 LCZ 2020 0628 添加设置UpdateChangeProfileInitPrivateFile私有文件初始化标志,在SEEUICCVariableInit函数里面根据标志初始化
*
*@history     :
*/
BYTE ProfileChangeDisable_GernerateNotifyAndSetState(BYTE targetProfileIndex, BYTE mode)
{
	BYTE idata enableProfileIndex,lastOperationalProfile;
	BYTE idata transactionTag;
	
	transactionTag = IF_BeginTransaction();
		
	enableProfileIndex = ILLEGAL_PROFILEINDEX;
		
	if(GetSingleBYTEProfileMeta(targetProfileIndex, MetaDataProfileClass_INDEX,2) == PROFILE_CLASS_TEST)//step4: if the target Profile is a Test Profile and an Operational Profile was in Enabled state before the (first) Test Profile was enabled: enable this Operational Profile. If this Operational Profile was deleted while the Test Profile was enabled, no error SHALL be generated and the function execution SHALL continue
	{
		lastOperationalProfile = JcvmConfig_GetRSPLastOperationaProfile();//VMReadByte(EUICCLastOperationaProfile_Addr) ;
		
		//if(lastOperationalProfile != ILLEGAL_PROFILEINDEX)
		if(ProfileIndexIsValid(lastOperationalProfile))
		{
			//JcvmConfig_SetRSPLastOperationaProfile(ILLEGAL_PROFILEINDEX);//20230328 代码评审修改，这里不清除，删除清除即可，后续也不用还原
			
			enableProfileIndex = lastOperationalProfile;		
		}	
	}
	
	//if(enableProfileIndex != ILLEGAL_PROFILEINDEX)
	if(ProfileIndexIsValid(enableProfileIndex))
	{
		Notification_GenerateOtherOperation( NOTIFICATION_ENABLE , enableProfileIndex);
	}
	
	Notification_GenerateOtherOperation( NOTIFICATION_DISABLE, targetProfileIndex);
		
	if(mode == MODE_RESET)
	{
		if(enableProfileIndex == ILLEGAL_PROFILEINDEX) 
		{
			JcvmConfig_SetUpdataDisableProfileIndex(targetProfileIndex);
		}
		else
		{
			JcvmConfig_SetUpdataEnableProfileIndex(enableProfileIndex);
		}
	}
	else
	{
		JcvmConfig_SetProfileEnableIndex(enableProfileIndex);
	}
	
	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	IF_CommitTransaction(transactionTag);
	
	return enableProfileIndex;
}
/*
*@fucntion    : ProfileChangeEnable_GernerateNotifyAndSetState
*
*@description : enable profile ，包括产生通知，设置对应的profile状态信息
*
*@param       : BYTE targetProfileIndex
*@param       : BYTE targetProfileClass
*@param       : BYTE mode               --- 当前执行了enable profile操作之后, refresh的模式:MODE_RESET表示需要复位，所有的环境切换要等到复位之后才启用
*
*@return       :void
*
*@author      :xuyz
*
*@history     :
*/
void ProfileChangeEnable_GernerateNotifyAndSetState(BYTE targetProfileIndex, BYTE targetProfileClass, BYTE mode)
{
	BYTE idata transactionTag;
	
	transactionTag = IF_BeginTransaction();

	if(mode == MODE_RESET)
	{
		JcvmConfig_SetUpdataEnableProfileIndex(targetProfileIndex);
	}
	else
	{
		JcvmConfig_SetProfileEnableIndex(targetProfileIndex);
	}
	
	JcvmConfig_SetUpdateChangeProfileInitPrivateFileIndex(PROFILE_PrivateFileInit);

	if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		if((GetSingleBYTEProfileMeta(G_CurEnabledProfileIndex, MetaDataProfileClass_INDEX,2) == PROFILE_CLASS_OPERATIONAL)&&( targetProfileClass == PROFILE_CLASS_TEST ))// 如果本次是从operationalProfile切换到Test模式，需要记录operationalProfile
		{
			JcvmConfig_SetRSPLastOperationaProfile(G_CurEnabledProfileIndex);//VMSetBuf(EUICCLastOperationaProfile_Addr,G_CurEnabledProfileIndex,1);
		}
		
		Notification_GenerateOtherOperation(NOTIFICATION_DISABLE, G_CurEnabledProfileIndex);//产生disable的通知
	}
	
	Notification_GenerateOtherOperation( NOTIFICATION_ENABLE, targetProfileIndex);
		
	IF_CommitTransaction(transactionTag);
}
/*
*@function    : Notification_GetValidNum
*
*@see         :
*
*@description :获取卡片上已经存在的有效通知的个数，并返回最后一个有效通知数据的存储地址
*
*@param       :UWORD* addr --- 输出参数，当前卡片内已经存在的最后一个有效通知的地址
*@param       :UWORD* addr --- 输出参数，当前卡片内已经存在的最后一个通知存储区的地址
*
*@return      :UWORD --- 卡片内最后一个有效通知的数据存储地址
*
*@RAM         :无
*
*@author      :xuxiaojun@20181123
*
*@history     :created by xuxiaojun@20181123
*/
BYTE Notification_GetValidNum(UWORD* addr, UWORD* addr2)
{
	UWORD xdata notifyAddr, lastAddr;
	WORD idata notifyRef, notifyLen;
	BYTE idata count;
	
	count = 0x00;
	
	lastAddr = EUICCNotificationsRef_Addr;

	if(addr != NULL)
	{
		addr[0] = EUICCNotificationsRef_Addr;
	}

	notifyAddr = GetAddrAndIndexByIndexAddr(EUICCNotificationsRef_Addr,&notifyRef);      //获取到通知列表中第一个通知的引用以及第一个通知数据存储的地址

	while(notifyAddr != ADDR_IS_NULL)
	{
		notifyLen = memReadU16(MEMtPM8(GetNotificationList_notificationDataSizeAddr(notifyAddr)));//VMReadBuf((BYTE*)&notifyLen,notifyAddr + NotificationList_notificationDataSize_Offset,2);
		
		if(notifyLen != 0x00)
		{
			if(addr != NULL)
			{
				addr[0] = notifyAddr;
				
				count++;
			}
		}
		
		if(addr2 != NULL)
		{
			addr2[0] = notifyAddr;
		}
		
		lastAddr = notifyAddr;
		
		notifyAddr = GetAddrAndIndexByIndexAddr(GetNotificationList_nextNotificationRefAddr(lastAddr),&notifyRef);
	}
	
	return count;	
}

/*
*@function    :GenerateNotificationMetadataTLV
*
*@see         :SGP22_V2.2 SECTION 3.5   5.7.9-5.7.11
*
*              NotificationMetadata ::= [47] SEQUENCE { -- Tag 'BF2F' 
*                    seqNumber [0] INTEGER, 
*                    profileManagementOperation [1] NotificationEvent, Only one bit SHALL be set to 1 
*                    notificationAddress UTF8String, -- FQDN to forward the notification 
*                    iccid Iccid OPTIONAL 
*              }
*
*@description :生成NotificationMetadata的TLV数据
*
*@param       :BYTE operation        --- profile操作类型
*@param       :BYTE notificationType --- 当前生成的通知类型，是final result / other
*@param       :UWORD addr            --- notificationAddress的地址，该地址处的数据格式为TLV /对于安装该字段仅存储地址V
*@param       :WORD notificationLen  --- notification的总长度，包含TLV，是NotificationConfigurationInformation的notificationAddress的起始偏移;  L的值最大设置为0x7F，在storeMetadata时就已经进行了合法性判断
*@param       :UWORD iccidAddr       --- iccid的地址，该地址处的数据格式为TLV, 该地址为profile的MetaData_Iccid_Offset偏移起始
*
*@return      :BYTE --- BF2F的TLV的总长度，如果返回0则表示数组发生越界   BF2F L 8005V 8102V 307FV 5A0AV     
*
*@RAM         :tempBuf[0..0x9d] 借用最多时的情况是BF2F 8099 8005V 8102V 308180V 5A0AV，该函数内不会越界  
*
*@author      :xuxiaojun@20181123
*
*@history     :created by xuxiaojun@20181123
*注意：        此处tempBuf的值并没有被取出，该函数调用结束后，tempBuf的数据还是有效的。所以，这里需要考虑如何处理tempBuf中的数据，将其存储在哪里？
*/
BYTE GenerateNotificationMetadataTLV(BYTE operation, BYTE notificationType, UWORD notificationAddress, WORD notificationLen, UWORD iccidAddr)
{
	UWORD xdata seqNumber;
	BYTE idata offset,tagLen;
	BYTE xdata tag[5];
	
	offset = 0; 
                     	
	seqNumber = VMReadUword(EUICCNotificationSeqNumber);// seqNumber [0] INTEGER  80 L V
	
	tempBuf[offset++] = 0x80;
	
	ASN1_GenIntegerLV(seqNumber, &tempBuf[offset]);
	
	offset += (tempBuf[offset] + 1);
	
	tempBuf[offset++] = 0x81;                            // profileManagementOperation [1] NotificationEvent, Only one bit SHALL be set to 1
	
	tempBuf[offset++] = 0x02;
	
	switch(operation)
	{
		case NOTIFICATION_EVENT_Install:
		{
			tempBuf[offset++] = 0x07;
			break;
		}
		case NOTIFICATION_EVENT_ENABLE:
		{
			tempBuf[offset++] = 0x06;
			break;
		}
		case NOTIFICATION_EVENT_DISABLE:
		{
			tempBuf[offset++] = 0x05;
			break;
		}
		default: // NOTIFICATION_EVENT_DELETE:
		{
			tempBuf[offset++] = 0x04;
			break;
		}
	}
	
	tempBuf[offset++] = operation; 
	
	if((operation == NOTIFICATION_EVENT_Install )&&(notificationType == NOTIFICATION_TYPE_FINALRESULT))
	{
		tempBuf[offset ++] = 0x0C;  

		tempBuf[offset ++] = notificationLen;		
		
		VMReadBuf(&tempBuf[offset], notificationAddress, notificationLen);//notificationAddress UTF8String, -- FQDN to forward the notification
	}
	else
	{
		VMReadBuf(&tempBuf[offset], notificationAddress, notificationLen);//notificationAddress UTF8String, -- FQDN to forward the notification
		
		tempBuf[offset] = 0x0C;                                           //通过store data存储时，notificationAddress的T采用的是自动编码0x81，通知中采用的是通用编码0x0C
	}
	
	offset += notificationLen;
	
	if(iccidAddr != ADDR_IS_NULL)
	{
		VMReadBuf(&tempBuf[offset], iccidAddr, 12);                           //ICCID  TLV
	
		offset += 12;
	}
	
	tagLen = ASN1_PaddingBERTlvTagAndLen(tag,0xBF2F, offset);            
	
	ecpMem_Move(&tempBuf[tagLen], tempBuf, offset);
	
	ecpMem_Copy(tempBuf, tag, tagLen);
	
	return (offset+tagLen);
}

/*
*@function    :Notification_HeapAlloc
*
*@see         :
*
*@description :给需要增加的通知分配空间，如果空间不够，则删除掉序列号最小的通知，直到所有的通知被释放完。如果空间还是不够则返回ADDR_IS_NULL
*
*@param       :WORD notificationDataSize --- 输入参数，通知的TLV的最大总长度
*@param       :BYTE operation            --- 输入参数，当前产生通知的操作
*@param       :WORD euiccEumRef          --- 输入参数，当前通知中所使用的EUICCEUM证书链的引用
*@param       :BYTE notificationType     --- 输入参数，当前产生通知的类型： final result / other notification
*@param       :BYTE mode                 --- 输入参数，当前调用该函数的模式：NOTIFICATION_HEAPALLOC_MODE_PRE表示预分配 / NOTIFICATION_HEAPALLOC_MODE_NOR表示后续生成
*
*@return      :UWORD                     --- 分配成功则返回通知对应的NotificationList结构的起始地址/失败返回ADDR_IS_NULL
*
*@RAM         :无
*
*@author      :xuxiaojun@20190103
*
*@history     :created by xuxiaojun@20190103
               modify by xuyz@20190111 该函数负责分配空间并更新一定的数据，且将新分配的通知添加到通知列表中去
*/
UWORD Notification_HeapAlloc(WORD notificationDataSize ,BYTE operation, WORD euiccEumRef, BYTE notificationType, BYTE mode)
{
//	NotificationListHead xdata newNorificationHead;
	WORD idata firstNotificationRef;
	UWORD xdata lastAddr,seqNumber,allocAddr, preAddr;
	WORD idata  notificationRef;
	BYTE idata notificationNum;
	
	allocAddr = ADDR_IS_NULL;
	
	//ecpMem_Set(&newNorificationHead, 0x00, sizeof(newNorificationHead));
	
//	if(mode == NOTIFICATION_HEAPALLOC_MODE_PRE)
//	{
//		allocAddr = GetAddrByIndexAddr(EUICCNotificationsRef_Addr);
//
//		if(allocAddr != ADDR_IS_NULL)
//		{
//			return allocAddr;
//		}
//
//		preAddr = EUICCNotificationsRef_Addr;
//
//		for(notificationNum = 0x00; notificationNum < NOTIFICATION_MAX_NUMBER; notificationNum++)
//		{
//			allocAddr = VMCOSHeapAlloc(NotificationMaxSIZE, &notificationRef, CardMemory_Mode);  // 参考NotificationList，每次按最大值分配
//
//			if(allocAddr == ADDR_IS_NULL)  // 预分配时卡片空间就不够了
//			{
//				return ADDR_IS_NULL;
//			}
//
//			VMWriteBuf(allocAddr,(BYTE *)&newNorificationHead,sizeof(newNorificationHead));  //更新通知的头列表信息
//
//			VMWriteBuf(preAddr, (BYTE*)&notificationRef, sizeof(WORD));
//
//			preAddr = allocAddr;//该地址实际是allocAddr + NotificationList_nextNotificationRef_Offset，用于存储下一次分配的通知的引用
//		}
//
//		return allocAddr;
//	}
	
	if((notificationDataSize /*+ sizeof(NotificationListHead)*/) > NotificationMaxSIZE)
	{
		return ADDR_IS_NULL;
	}
	
	notificationNum = Notification_GetValidNum(&preAddr, &lastAddr); // 获取卡内最后一个有效通知的地址，以及最后一个通知存储区域的地址
	
	if(notificationNum == NOTIFICATION_MAX_NUMBER) // 通知已满，则复用最先生成的通知
	{
		preAddr = GetAddrAndIndexByIndexAddr(EUICCNotificationsRef_Addr, &firstNotificationRef);                        //获取到第一个通知的引用和存储地址
		
		notificationRef = memReadU16(MEMtPM8(GetNotificationList_nextNotificationRefAddr(preAddr)));//ReadEEPromCache((BYTE*)&notificationRef, preAddr + NotificationList_nextNotificationRef_Offset, sizeof(WORD));  //获取第二个通知的引用
		
		memWriteU16Atomic(MEMtPM8(EUICCNotificationsRef_Addr),notificationRef);//VMWriteBuf(EUICCNotificationsRef_Addr, (BYTE*)&notificationRef, sizeof(WORD));                                  //把第二个通知的引用写到EUICCNotificationsRef_Addr处
		
		allocAddr = preAddr;
		
		notificationRef = firstNotificationRef;
		
		memWriteU16Atomic(MEMtPM8(GetNotificationList_nextNotificationRefAddr(lastAddr)),notificationRef);//VMWriteBuf(lastAddr, (BYTE*)&notificationRef, sizeof(WORD));  //将本条通知链接到通知列表的尾部，并且修改本条通知的下一个引用为空引用
		
		memWriteU16Atomic(MEMtPM8(GetNotificationList_nextNotificationRefAddr(allocAddr)),REF_NULL);
	}
	else //通知未满 直接使用 不需要修改链接关系
	{
		if(preAddr == EUICCNotificationsRef_Addr)
		{
			allocAddr = GetAddrByIndexAddr(preAddr);
		}
		else
		{
			allocAddr = GetAddrByIndexAddr(GetNotificationList_nextNotificationRefAddr(preAddr));
		}
	}	
	
	seqNumber = VMReadUword(EUICCNotificationSeqNumber);

	memWriteU32Atomic(MEMtPM8(GetNotificationList_seqNumberAddr(allocAddr)),seqNumber);
	//newNorificationHead.seqNumber = seqNumber;
	memWriteU8Atomic(MEMtPM8(GetNotificationList_profileManagementOperationAddr(allocAddr)),operation);
	//newNorificationHead.profileManagementOperation = operation;
	memWriteU8Atomic(MEMtPM8(GetNotificationList_notificationTypeAddr(allocAddr)),notificationType);
	//newNorificationHead.notificationType = notificationType;
	memWriteU16Atomic(MEMtPM8(GetNotificationList_notificationDataSizeAddr(allocAddr)),notificationDataSize);
	//newNorificationHead.notificationDataSize = notificationDataSize;
	
	if((notificationType == NOTIFICATION_TYPE_OTHER)&&(euiccEumRef != REF_NULL))
	{
		memWriteU16Atomic(MEMtPM8(GetNotificationList_euiccEumRefAddr(allocAddr)),euiccEumRef);
		//newNorificationHead.euiccEumRef = euiccEumRef;
	}
	else
	{
		memWriteU16Atomic(MEMtPM8(GetNotificationList_euiccEumRefAddr(allocAddr)),REF_NULL);
	}
	
	//VMWriteBuf(allocAddr,(BYTE *)&newNorificationHead,sizeof(newNorificationHead));  //更新通知的头列表信息
	
	seqNumber ++;
	
	if(seqNumber == 0xFFFFFFFF)   // 这里增加边界限制
	{
		seqNumber = 0x00;
	}
	
	VMWriteUword(EUICCNotificationSeqNumber, seqNumber);           // Each time a new Notification is generated by the eUICC, the Sequence Number SHALL be incremented 
	
	return allocAddr;
}

/*
*@function    : Notification_GenerateOtherOperation
*
*@see         :SGP22_V2.2 SECTION 3.5   5.7.9-5.7.11
*              OtherSignedNotification ::= SEQUENCE { 
*                    tbsOtherNotification NotificationMetadata,  
*                    euiccNotificationSignature [APPLICATION 55] OCTET STRING, -- eUICC signature of tbsOtherNotification, Tag '5F37' 
*                    euiccCertificate Certificate,  -- eUICC Certificate (CERT.EUICC.ECDSA) signed by the EUM  
*                    eumCertificate Certificate   -- EUM Certificate (CERT.EUM.ECDSA) signed by the requested CI 
*              }
*
*@description : 对profile的disable enable delete操作产生对应的notification
*
*@param       : BYTE operation :NOTIFICATION_EVENT_ENABLE /  NOTIFICATION_EVENT_DISABLE / NOTIFICATION_EVENT_DELETE
*@param       : BYTE profileIndex :当前待产生通知的profile索引
*
*@return      :void
*
*@RAM         :tempBuf
*
*@author      :xuxiaojun@20181123
*
*@history     :created by xuxiaojun@20181123
*             :modify by xuxiaojun@20180612   修改通知存储和发送方案：生成通知时，先计算好一条完整证书的长度，并将除euiccCertificate + eumCertificate之外的信息写入到通知中
*注意         :delete profile时，通知生成需要放在Metadata释放之前，否则就找不到对应的数据
*/
void Notification_GenerateOtherOperation(BYTE operation, BYTE profileIndex)
{
	UWORD xdata notificationAddr, len;
	UWORD xdata configInfoAddr, metaDataAddr, euiccCertificateAddr, eumCertificateAddr, listAddr;
	WORD idata  euiccNotificationSignatureLen, euiccCertificateLen, eumCertificateLen, maxLen;
	WORD idata offset, lengthOfLen, ref;
	ASN1Info xdata tempASN1Info;
	BYTE idata transactionTag;

#if EUICC__FUNCTION__GUOMI_eSIM
	G_CertType = GetCertType(profileIndex);
#endif

	metaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex*sizeof(WORD));  //metaData不存在
	
	if(metaDataAddr == ADDR_IS_NULL)
	{
		return;  //理论上来说不会走到这里来的，因为成功的profile下载肯定会执行storeMetaData指令
	}
		
	configInfoAddr = GetMetaData_notificationConfigurationInfoRefValueAddr(metaDataAddr);//GetAddrByIndexAddr(metaDataAddr + MetaData_NotificationConfigurationInfoRef_Offset);   	// Recipient Address，该地址处的数据结构为B6 L 30LV 30LV...
	
	if(configInfoAddr == ADDR_IS_NULL)
	{
		return;  // 执行StoreMetadata时，notificationConfigurationInfo是OPTIONAL，所以这里可能为null
	}
	
	lengthOfLen = ASN1_GetBERTlvLen(configInfoAddr+1, (UWORD*)&len);                           //获取MetaData_NotificationConfigurationInfoRef_Offset数据的L, 存储时会保证TLV 格式的正确性
	
	ASN1_InitTLVInfo((ASN1Info *)&tempASN1Info,configInfoAddr + 1 + lengthOfLen, len);         //1表示偏移过B6，用于查找的数据为30LV 30LV...
	
	ref = REF_NULL;
	
	while(1)          // 循环遍历NotificationConfigurationInformation, 30L 80LV 81LV
	{
		if(FALSE == ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x30 , FALSE, 1))   //找到SEQUENCE OF的最后一个NotificationConfigurationInformation
		{
			break;
		}
		
		ASN1_SetNextDepthToFindTLV((ASN1Info *)&tempASN1Info);
		
		ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x80 , FALSE, 0x01);      //找到NotificationConfigurationInformation中的profileManagementOperation 8002 XXXX
		
		offset = VMReadByte(tempASN1Info.bufferAddr + tempASN1Info.curValueOff + 1);                //借用offset
		
		if((offset&operation) == operation)
		{
			listAddr = GetProfile_PkIdRefValueAddr(GetISDPObjectAddr(profileIndex));//GetProfile_PkIdRefValueAddr(GetProfileEntryAddr(profileIndex));////GetAddrByIndexAddr(PROFILE_PKIDREF + profileIndex*sizeof(WORD)); // 获取当前profile的pkid的存储地址，格式为LV, L为1字节
			
			if(listAddr == ADDR_IS_NULL)  //当前profile的pkid不存在，正常的话不会出现这种情况，数据的存储格式为LV,L为1字节
			{
				return;
			}
			
			listAddr = Certificate_SearchEUICCCerficateByCIPKID(listAddr + 1, VMReadByte(listAddr), &ref);
			
			euiccCertificateAddr = Certificate_GetEUMEUICCCerInfoByListAddr(listAddr, EUICC_Certificate_Mode);  //获取EUICC证书的数据地址,存储格式为2字节的证书TLV总长度 + 完整的证书数据 TLV
			
			euiccCertificateLen = VMReadShort(euiccCertificateAddr);  
			
			eumCertificateAddr = Certificate_GetEUMEUICCCerInfoByListAddr(listAddr, EUM_Certificate_Mode);    //获取EUM证书的数据地址,存储格式为2字节的证书TLV总长度 + 完整的证书数据 TLV
			
			eumCertificateLen = VMReadShort(eumCertificateAddr);          //证书的长度写入时是字节流写入，所以这里不会有大小端问题  
		
			ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x81 , FALSE, 0x01);  //找到notificationAddress，如果30存在，则其子TAG中一定存在notificationAddress
			
			len = GenerateNotificationMetadataTLV(operation, NOTIFICATION_TYPE_OTHER, tempASN1Info.bufferAddr + tempASN1Info.curTagOff, tempASN1Info.curTLVLen, GetMetaData_iccidAddr(metaDataAddr)/*metaDataAddr + MetaData_Iccid_Offset*/);  // BF2F 该函数调用结束后，tempBuf的数据还是有效的。所以，这里需要考虑如何处理tempBuf中的数据，将其存储在哪里？
			
			if(len == 0)  //GenerateNotificationMetadataTLV内部发生了越界，抛异常 or return FALSE
			{
				return;
			}
			
			maxLen = len + 67 + euiccCertificateLen + eumCertificateLen;     //len是NotificationMetadata的TLV长度; 67是euiccNotificationSignature的TLV的长度; euiccCertificateLen是euiccCertificate的TLV长度; eumCertificateLen是eumCertificate的TLV长度;
			
			lengthOfLen = ASN1_PaddingBERTlvTagAndLen(&tempBuf[len],0x30,maxLen);//填充最外层的30L, 此时tempBuf[0..len-1]存放的是BF2F LV, 所以，这里借用tempBuf[len..]缓存最外层的30L
			
			transactionTag = IF_BeginTransaction();
						
			notificationAddr = Notification_HeapAlloc(len + 67 + lengthOfLen, operation, ref, NOTIFICATION_TYPE_OTHER, NOTIFICATION_HEAPALLOC_MODE_NOR);    // 先分配本次待增加通知的NotificationList的地址
			
			if(notificationAddr == ADDR_IS_NULL)
			{
				IF_AbortTransaction(transactionTag);
				
				return;
			}
			
			notificationAddr = GetNotificationList_notificationData_ArrayAddr(notificationAddr);//notificationAddr += NotificationList_notificationData_Offset;   // 偏移到通知数据的T位置
			
			offset = 0; 
			
			WriteEepromNonAtomic(notificationAddr + offset, &tempBuf[len], lengthOfLen);   // 把最外层的30L写入, 30L，事务中分配的空间不受保护
			
			offset += lengthOfLen;
			
			WriteEepromNonAtomic(notificationAddr + offset, &tempBuf[0], len);             // 把NotificationMetadata的TLV写入, BF2F LV, 事务中分配的空间不受保护
			
			euiccNotificationSignatureLen = AuthenticateServer_CalEuiccSignature1(notificationAddr + offset, len, listAddr, tempBuf); // 计算euiccNotificationSignature并返回长度euiccNotificationSignatureLen,  euiccNotificationSignature SHALL apply on the tbsOtherNotification data object
			
			offset += len;
			
			WriteEepromNonAtomic(notificationAddr + offset, tempBuf, euiccNotificationSignatureLen);   // 把euiccNotificationSignature的TLV写入, 5F37 L V, 事物中分配的空间不受事务保护
			
			IF_CommitTransaction(transactionTag);
		}
		
		ASN1_ReturnFromDepthToFindTLV((ASN1Info *)&tempASN1Info);         //返回到上一级继续查找30
	}	
}
/*
*@function    :GenerateFinalResultTLV
*
*@see         :SGP22_V2.2 SECTION 3.5   5.7.9-5.7.11
*              ProfileInstallationResultData ::= [39] SEQUENCE { -- Tag 'BF27' 
*                  transactionId[0] TransactionId, -- The TransactionID generated by the SM-DP+ 
*                  notificationMetadata[47] NotificationMetadata, 
*                  smdpOid OBJECT IDENTIFIER, -- SM-DP+ OID (same value as in CERT.DPpb.ECDSA) 
*                  finalResult [2] CHOICE { 
*                     successResult SuccessResult,
*                     errorResult ErrorResult 
*                  } 
*              }
*
*              SuccessResult ::= SEQUENCE { 
*                 aid [APPLICATION 15] OCTET STRING (SIZE (5..16)), -- AID of ISD-P 
*                 simaResponse OCTET STRING -- contains (multiple) 'EUICCResponse' as defined in [5] 
*              }
*
*              ErrorResult ::= SEQUENCE { 
*                 bppCommandId BppCommandId, 
*                 errorReason ErrorReason, 
*                 simaResponse OCTET STRING OPTIONAL -- contains (multiple) 'EUICCResponse' as defined in [5] 
*              }
*
*              EUICCResponse ::= SEQUENCE { 
*                 peStatus SEQUENCE OF PEStatus, 
*                 profileInstallationAborted NULL OPTIONAL, 
*                 statusMessage UTF8String OPTIONAL 
*              }
*
*              PEStatus ::= SEQUENCE { 
*                  status INTEGER { ok(0), pe-not-supported(1), memory-failure(2),bad-values(3), not-enough-memory(4),invalid-request-format(5), invalid-parameter(6), runtime-not-supported (7), lib-not-supported (8), 
*                                  template-not-supported (9), feature-not-supported (10),  unsupported-profile-version(31)}, 
*                  identification UInt15 OPTIONAL, -- Identification number of the PE triggering the error 
*                  additional-information UInt8 OPTIONAL -- Additional information related to the status code 
*              }
*
*@description :拼装ProfileInstallationResultData中finalResult的TLV数据，并返回TLV的总长度
*
*@param       :BYTE result        --- 当前安装操作的执行结果
*@param       :BYTE bppCommandId  --- 当前安装操作的ID， 当result为0x00时，该参数无意义
*@param       :BYTE* simaResponse --- PE解析的结果，格式为LV，根据规范定义，这里的L肯定不会超过1字节的表示范围.
*                                     只有bppCommandId为BPPCommandId_LOADPROFILEELEMENTS时才有意义，安装成功时，该值肯定有意义
*
*@return      :BYTE               --- 数据越界则返回0; / 否则返回TLV的总长度，该函数中限制TLV总长度不会超过0x7F
*
*@author      :xuxiaojun@20181126
*
*@history     :created by xuxiaojun@20181126
*注意         :该函数中还没有处理installFailedDueToDataMismatch， testProfileInstallFailedDueToInvalidNaaKey的情况
*/
//BYTE code SuccessSimaResponse[]={0x04,0x09,0x30,0x07,0xA0,0x05,0x30,0x03,0x80,0x01,0x00};  //SuccessResult中simaResponse的值是固定的, 11字节
BYTE GenerateFinalResultTLV(BYTE result, BYTE bppCommandId, BYTE* simaResponse)
{
	BYTE idata len, offset;
	
	offset = 0;
	
	if(result == 0x00)    //SuccessResult
	{	
		len = (24 + simaResponse[0]);  // A2L(2字节) + A0L(2字节) + 4F10V(18字节) + simaResponse(simaResponse[0] + 2)
		
		if(len > 0x7F) // 这里暂时这样定义，理论上该值的长度没有限制，但是实际中不会这么大
		{
			return FALSE;
		}
		
		tempBuf[offset++] = 0xA2;
		
		tempBuf[offset++] = (len - 2);
		
		tempBuf[offset++] = 0xA0;
		
		tempBuf[offset++] = (len - 4);
		
		tempBuf[offset++] = 0x4F;        // aid [APPLICATION 15] OCTET STRING (SIZE (5..16)), -- AID of ISD-P
		
		//这里的tempBuf不会越界
		tempBuf[offset] = GetProfileAIDByProfileIndex(G_RSPSessionContext.downloadProfileIndex, &tempBuf[offset+1]);
		
		offset += 0x11;  
		
		tempBuf[offset++] = 0x04;
		
		ecpMem_Copy(&tempBuf[offset], simaResponse, simaResponse[0]+1);  // simaResponse OCTET STRING -- contains (multiple) 'EUICCResponse' as defined in [5]
		
		return len;
	}
	
	if((bppCommandId == BPPCommandId_LOADPROFILEELEMENTS) && (result != ErrorReason_incorrectInputValues) && (result != ErrorReason_scp03tSecurityError)) // 下载profile过程中出现错误时，除了这两类reason之外的reason才需要拼装simaResponse
	{
		len = (12 + simaResponse[0]);  // A2L(2字节) + A1L(2字节)  + 8001xx(3字节) + 8101xx(3字节) + simaResponse(simaResponse[0] + 2)
	}
	else
	{
		len = 10;  // A2L(2字节) + A1L(2字节) + 8001xx(3字节) + 8101xx(3字节)
	}
	
	if(len > 0x7F) // 这里暂时这样定义，理论上该值的长度没有限制，但是实际中不会这么大
	{
		return FALSE;
	}
	
	tempBuf[0] = 0xA2;
	
	tempBuf[1] = len-2;
	
	tempBuf[2] = 0xA1;
	
	tempBuf[3] = len-4;
	
	tempBuf[4] = 0x80;
	
	tempBuf[5] = 0x01;
	
	tempBuf[6] = bppCommandId;

	tempBuf[7] = 0x81;
	
	tempBuf[8] = 0x01;
	
	tempBuf[9] = result;
	
	if((bppCommandId == BPPCommandId_LOADPROFILEELEMENTS) && (result != ErrorReason_incorrectInputValues) && (result != ErrorReason_scp03tSecurityError)) // 下载profile过程中出现错误时，除了这两类reason之外的reason才需要拼装simaResponse
	{
		tempBuf[10] = 0x82;
		
		ecpMem_Copy(&tempBuf[11], simaResponse, simaResponse[0]+1);
	}
	
	return len;
}
/*
*@function    :GetFinalResultTLVLen
*
*@see         :SGP22_V2.2 SECTION 3.5   5.7.9-5.7.11
*              ProfileInstallationResultData ::= [39] SEQUENCE { -- Tag 'BF27' 
*                  transactionId[0] TransactionId, -- The TransactionID generated by the SM-DP+ 
*                  notificationMetadata[47] NotificationMetadata, 
*                  smdpOid OBJECT IDENTIFIER, -- SM-DP+ OID (same value as in CERT.DPpb.ECDSA) 
*                  finalResult [2] CHOICE { 
*                     successResult SuccessResult,
*                     errorResult ErrorResult 
*                  } 
*              }
*
*              SuccessResult ::= SEQUENCE { 
*                 aid [APPLICATION 15] OCTET STRING (SIZE (5..16)), -- AID of ISD-P 
*                 simaResponse OCTET STRING -- contains (multiple) 'EUICCResponse' as defined in [5] 
*              }
*
*              ErrorResult ::= SEQUENCE { 
*                 bppCommandId BppCommandId, 
*                 errorReason ErrorReason, 
*                 simaResponse OCTET STRING OPTIONAL -- contains (multiple) 'EUICCResponse' as defined in [5] 
*              }
*
*              EUICCResponse ::= SEQUENCE { 
*                 peStatus SEQUENCE OF PEStatus, 
*                 profileInstallationAborted NULL OPTIONAL, 
*                 statusMessage UTF8String OPTIONAL 
*              }
*
*              PEStatus ::= SEQUENCE { 
*                  status INTEGER { ok(0), pe-not-supported(1), memory-failure(2),bad-values(3), not-enough-memory(4),invalid-request-format(5), invalid-parameter(6), runtime-not-supported (7), lib-not-supported (8), 
*                                  template-not-supported (9), feature-not-supported (10),  unsupported-profile-version(31)}, 
*                  identification UInt15 OPTIONAL, -- Identification number of the PE triggering the error 
*                  additional-information UInt8 OPTIONAL -- Additional information related to the status code 
*              }
*
*@description :获取ProfileInstallationResultData中finalResult的TLV数据的总长度
*
*@param       :BYTE result        --- 当前安装操作的执行结果
*@param       :BYTE bppCommandId  --- 当前安装操作的ID， 当result为0x00时，该参数无意义
*@param       :BYTE* simaResponse --- PE解析的结果，格式为LV，根据规范定义，这里的L肯定不会超过1字节的表示范围.
*                                     只有bppCommandId为BPPCommandId_LOADPROFILEELEMENTS时才有意义，安装成功时，该值肯定有意义
*
*@return      :BYTE  返回TLV的总长度   假设不超过1字节
*
*@author      :lgy
*
*@history     :created by @20180113
*/
BYTE GetFinalResultTLVLen(BYTE result, BYTE bppCommandId, BYTE* simaResponse)
{
	BYTE idata len;
	BYTE xdata AID[17];
	
	if(result == 0x00)    //SuccessResult
	{			
		AID[0] = GetProfileAIDByProfileIndex(G_RSPSessionContext.downloadProfileIndex, &AID[1]);  
		
		len = AID[0] + 2 + simaResponse[0] + 2 + 2 + 2;	//finalResult [2] CHOICE (2字节) + SuccessResult ::= SEQUENCE(2字节) 
		
		if(len > 0x7F)  // 理论上该值的长度没有限制，但是实际中不会这么大 这里暂时限制长度不能超过7F
		{
			return 0;
		}
		
		return len;
	}
	
	if((bppCommandId == BPPCommandId_LOADPROFILEELEMENTS) && (result != ErrorReason_incorrectInputValues) && (result != ErrorReason_scp03tSecurityError)) // 下载profile过程中出现错误时，除了这两类reason之外的reason才需要拼装simaResponse
	{
		len = (12 + simaResponse[0]);  // A2L(2字节) + A1L(2字节)  + 8001xx(3字节) + 8101xx(3字节) + simaResponse(simaResponse[0] + 2)
	}
	else
	{
		len = 10;  // A2L(2字节) + A1L(2字节) + 8001xx(3字节) + 8101xx(3字节)
	}
	
	if(len > 0x7F) // 这里暂时这样定义，理论上该值的长度没有限制，但是实际中不会这么大
	{
		return FALSE;
	}
		
	return len;
}
/*
*@function    :Notification_GenerateProfileInstallResult
*
*@see         :SGP22_V2.2 SECTION 3.5   5.7.9-5.7.11 <2.5.6  Profile Installation Result>
*
*              ProfileInstallationResult ::= [55] SEQUENCE { -- Tag 'BF37' 
*                  profileInstallationResultData [39] ProfileInstallationResultData, 
*                  euiccSignPIR EuiccSignPIR 
*              }
*
*              ProfileInstallationResultData ::= [39] SEQUENCE { -- Tag 'BF27' 
*                  transactionId[0] TransactionId, -- The TransactionID generated by the SM-DP+ 
*                  notificationMetadata[47] NotificationMetadata, 
*                  smdpOid OBJECT IDENTIFIER, -- SM-DP+ OID (same value as in CERT.DPpb.ECDSA) 
*                  finalResult [2] CHOICE { 
*                     successResult SuccessResult,
*                     errorResult ErrorResult 
*                  } 
*              }
*
*              EuiccSignPIR ::= [APPLICATION 55] OCTET STRING -- Tag '5F37', eUICC’s signature
*
*@description :对profile的INSTALL操作产生对应的RESULT
               该函数中使用了G_EUICCResponse存储profile的安装结果，传入时就已经将结果存放在G_EUICCResponse中了
*
*@param       :BYTE result       --- 当前安装操作的执行结果
*@param       :BYTE bppCommandId --- 当前安装操作的ID
*@param       :BYTE * iccid      --- 当前执行StoreMetadata时，指令中下来的iccid数据 TLV，该参数只有在bppCommandId == BPPCommandId_STOREMETADATA时才有效，其他时候该参数没有意义
*
*@return      :BYTE              --- TRUE表示通知增加成功 / FALSE表示通知增加失败
*
*@author      :xuxiaojun@20181126
*
*@history     :created by xuxiaojun@20181126
*注意         :该函数实现时，使用了分段拼装数据，比如GenerateNotificationMetadataTLV、GenerateFinalResultTLV等，在拼装函数实现过程中使用了tempBuf，且数据未取出
*              这里需要考虑如何把数据连在一起
*/
BYTE Notification_GenerateProfileInstallResult(BYTE bppCommandId, BYTE result, BYTE * iccid)
{
	UWORD xdata metaDataAddr, notificationAddr,allocAddr;
	WORD idata lenOfNotify, maxLen, ref;
	BYTE xdata transactionId[0x12];  // TransactionId ::= OCTET STRING (SIZE(1..16)) 
// 	BYTE xdata iccid[12];            // TLV
	BYTE idata smdpOIDLen,transactionTag,FinalResultTLVLen;
	WORD idata offset;
	BYTE idata tagBf27Len,tagBf37Len;
	BYTE xdata tagBf27[5],tagBf37[5]; 
	
	if(JcvmConfig_GetSEState() == CARD_OP_READY) //不需要判断CLA(CLA可能已经被修改了),by xwh 20200730(xuxiaojun已确认)
	{
		if(RESPONSE_NoError != result)
		{
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
			{
				GPLoadInit();
			}

			DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);

			//这里是预置prfoile阶段，所以就不清空当前上下文了，节省代码量
			ThrowError(SW_CONDITIONS_NOT_SATISFIED);
		}
		else
		{
			RSP_ClearSession();//added by xuxiaojun@20230317: 清空当前上下文， 预制的profile没办法激活，一直在downing状态

			return TRUE;
		}
	}
	
	transactionId[0] = 0x80;
	
	transactionId[1] = G_RSPSessionContext.transactionIdLen;
	
	ecpMem_Copy((BYTE*)&transactionId[2], G_RSPSessionContext.channllengeTransactionId, G_RSPSessionContext.transactionIdLen);
	
	metaDataAddr = ADDR_IS_NULL;
	
	if(bppCommandId < BPPCommandId_STOREMETADATA)
	{
		lenOfNotify = GenerateNotificationMetadataTLV(NOTIFICATION_EVENT_Install, NOTIFICATION_TYPE_FINALRESULT, (UWORD)&G_RSPSessionContext.serverAddress[1], G_RSPSessionContext.serverAddress[0], ADDR_IS_NULL);// BF2F 该函数调用结束后，tempBuf的数据还是有效的。
	}
	else if(bppCommandId == BPPCommandId_STOREMETADATA)
	{
		//假设RecData中的数据没有发生变化，从RecData中解析中iccid
		
		lenOfNotify = GenerateNotificationMetadataTLV(NOTIFICATION_EVENT_Install, NOTIFICATION_TYPE_FINALRESULT, (UWORD)&G_RSPSessionContext.serverAddress[1], G_RSPSessionContext.serverAddress[0], (UWORD)&iccid[0]);// BF2F 该函数调用结束后，tempBuf的数据还是有效的。所以，这里需要考虑如何处理tempBuf中的数据，将其存储在哪里？
	}	
	else //(bppCommandId > BPPCommandId_STOREMETADATA)
	{
		metaDataAddr = GetProfileMetaDataAddr(G_RSPSessionContext.downloadProfileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + G_RSPSessionContext.downloadProfileIndex*sizeof(WORD));  //执行到这里时，StoreMetadata已经执行成功，所以metaDataAddr不会为null
		
		lenOfNotify = GenerateNotificationMetadataTLV(NOTIFICATION_EVENT_Install, NOTIFICATION_TYPE_FINALRESULT, (UWORD)&G_RSPSessionContext.serverAddress[1], G_RSPSessionContext.serverAddress[0], GetMetaData_iccidAddr(metaDataAddr)/*metaDataAddr + MetaData_Iccid_Offset*/);// BF2F 该函数调用结束后，tempBuf的数据还是有效的。所以，这里需要考虑如何处理tempBuf中的数据，将其存储在哪里？
	}
	
	smdpOIDLen = VMReadByte(EUICC_AuthenTmpBuf + TempSIMOID_Off);                     //读取AutherServer中的SM-DP OID的长度，到这里时该值肯定是合法的
	
	FinalResultTLVLen = GetFinalResultTLVLen(result, bppCommandId, G_EUICCResponse);
	
	maxLen = (transactionId[1] + 2) + lenOfNotify + (smdpOIDLen + 2) + FinalResultTLVLen ;//bf27子tag的总长度 ，不包含tag len字段    ，借用&tempBuf[lenOfNotify] 该值无意义 只是为了取出BF27 + L字段的长度
	
	tagBf27Len = ASN1_PaddingBERTlvTagAndLen(&tagBf27[0],0xBF27,maxLen);//BF27 + L字段后的长度

	maxLen += (tagBf27Len + 67) ; //加上euiccSignPIR EuiccSignPIR ，此时maxLen 为profileInstallationResultData + euiccSignPIR的总长度，不包含tag BF37 + L字段
	
	tagBf37Len = ASN1_PaddingBERTlvTagAndLen(&tagBf37[0],0xBF37,maxLen);//BF37 + L字段的长度 ，借用&tempBuf[lenOfNotify] 该值无意义 只是为了取出BF37 + L字段的长度
	
	maxLen += tagBf37Len;
	
	transactionTag = IF_BeginTransaction();
		
	allocAddr = Notification_HeapAlloc(maxLen ,NOTIFICATION_EVENT_Install, REF_NULL, NOTIFICATION_TYPE_FINALRESULT, NOTIFICATION_HEAPALLOC_MODE_NOR);  // 参考NotificationList
	
	if(allocAddr == ADDR_IS_NULL)
	{
		IF_AbortTransaction(transactionTag);
		
		if(RESPONSE_NoError != result)
		{
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
			{
				GPLoadInit();
			}

			DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);
		}

		RSP_ClearSession(); //清空当前上下文 // add chentao@20230115

		return FALSE;
	}
	
	notificationAddr =  GetNotificationList_notificationData_ArrayAddr(allocAddr);//allocAddr + NotificationList_notificationData_Offset;   // 偏移到通知数据的T位置
			
	offset = tagBf27Len + tagBf37Len; // 用于存储通知的最外层BF37L/ BF27 L , V的值直接写入	

	VMWriteBuf(notificationAddr + offset, transactionId, transactionId[1] + 2);   // 把transactionId的TLV写入, 80LV
	
	offset += (transactionId[1] + 2);
	
	VMWriteBuf(notificationAddr + offset, tempBuf, lenOfNotify);                  // 把NotificationMetadata的TLV写入, BF2F LV
	
	offset += lenOfNotify;
	
	tempBuf[0] = 0x06;
	
	tempBuf[1] = smdpOIDLen;
	
	VMReadBuf(&tempBuf[2], EUICC_AuthenTmpBuf + TempSIMOID_Off + 1, smdpOIDLen);
	
	VMWriteBuf(notificationAddr + offset, tempBuf, smdpOIDLen + 2);              // 把smdpOid的TLV写入, 06 LV
	
	offset += (smdpOIDLen + 2);	
				
	lenOfNotify = GenerateFinalResultTLV(result, bppCommandId, G_EUICCResponse);         //获取finalResult的TLV并返回TLV的总长度 A2LV.  这里是假设PE解析的结果存储在G_EUICCResponse中, 假设GenerateFinalResultTLV的总长度最大为
	
	VMWriteBuf(notificationAddr + offset, tempBuf, lenOfNotify);                 // 把finalResult的TLV写入, A2LV
	
	offset += lenOfNotify;	
	
	VMWriteBuf(notificationAddr + tagBf37Len, &tagBf27[0], tagBf27Len);                // 把BF27 L写入      
	
	metaDataAddr = Certificate_SearchEUICCCerficateByCIPKID(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off + 1, VMReadByte(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off), &ref);//复用metaDataAddr, 获取当前profile的pkid的存储地址，格式为LV, L为1字节
	
	lenOfNotify = AuthenticateServer_CalEuiccSignature1(notificationAddr + tagBf37Len, offset - tagBf37Len, metaDataAddr, tempBuf);  // 计算euiccNotificationSignature并返回长度euiccNotificationSignatureLen
	
	VMWriteBuf(notificationAddr + offset, tempBuf, lenOfNotify);                 // 把EuiccSignPIR的TLV写入, 5F37 LV
	
	VMWriteBuf(notificationAddr, &tagBf37[0], tagBf37Len);                       // 把最外层的BF37 L写入

	VMSetBuf(EUICC_AuthenTmpBuf, 0x00, EUICC_AuthenTmpSize);                     //将EUICC_AuthenTmpBuf清掉

	IF_CommitTransaction(transactionTag);
	
	if(RESPONSE_NoError == result)
	{
		Notification_GenerateOtherOperation(NOTIFICATION_INSTALL , G_RSPSessionContext.downloadProfileIndex);  // 安装成功后先生成final result， 再可能生成other notification, 对应的测试用例<TC_eUICC_ES10b_RetrieveNotificationsList_08_Nominal_Retrieve_by_Sequence_Number_for_Install_OtherSignedNotification_and_Enable_Notifications>
	}
	else
	{
		if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
		{
			GPLoadInit();
		}

		DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);
	}
		
	RSP_ClearSession(); //清空当前上下文
	
	SetSendFileExtendInfo(notificationAddr, maxLen);
	
	return TRUE;
}
/*
*@function     : GetEuiccCiPKIdList
*
*@description  :CUSTOMER_GetEUICCInfo子函数，用于获取CIPKID for Verification/ForSignature 
*
				拼装的数据为 euiccCiPKIdListForVerification [9] SEQUENCE OF SubjectKeyIdentifier
						  或 euiccCiPKIdListForSigning [10] SEQUENCE OF SubjectKeyIdentifier
*
*@see          : <SGP22_V2.2 > 5.7.8  Function (ES10b): GetEUICCInfo 
*
*@param        :BYTE* OutBufAddr ---输出参数, RAM. 目前上层调用传入的是tempBuf ,程序里应避免数据冲掉的问题
*@param        :BYTE freeLen     ---输入参数, OutBufAddr中可用数据长度
*@param        :BYTE mode        ---输入参数, CIPKID_ForSignature/CIPKID_ForVerification
*
*@return       :WORD --- OutBufAddr中有效数据的长度
*
*@RAM Resources:
*
*@hisotry      :V1.0 created by lgy@
*              :V1.1 modify by xuxiaojun@20190614 增加越界处理
*/	
WORD GetEuiccCiPKIdList(BYTE* OutBufAddr, WORD freeLen, BYTE mode)
{
	UWORD xdata EuiccCAInfoAddr,CIPkIdAddr;
	WORD idata totleLen ,certLen, ref;
	BYTE idata tag,octerNum;
	BYTE xdata temp[CACIList_PkIdCI_MAX_Len + 2];
	BYTE xdata tempTL[5]; //缓存TL，用于判断是否会越界
	
	totleLen = 0;

	if(mode == CIPKID_ForSignature)
	{
		tag = TAG_EuiccCiPKIdListForSigning; // AA
	}
	else
	{
		tag = TAG_EuiccCiPKIdListForVerification; // A9
	}
	
	EuiccCAInfoAddr = GetAddrByIndexAddr(EUICCCARef_Addr);// 第一组CI证书
	
	if(EuiccCAInfoAddr == ADDR_IS_NULL)
	{
		OutBufAddr[0] = tag;
		
		OutBufAddr[1] = 0x00;
		
		return 2;
	}	
	
	while(EuiccCAInfoAddr)
	{						
		CIPkIdAddr = CACIListGet_pkIdCIArrayValueAddr(EuiccCAInfoAddr);//GetAddrByIndexAddr(EuiccCAInfoAddr + CACIList_PkIdCIRef_Offset);
		
		certLen = VMReadByte(CIPkIdAddr); // 1字节长度, 个人化时要求其值不超过20字节，否则就抛错
		
		if(mode == CIPKID_ForSignature)
		{											
			if(Certificate_SearchEUICCCerficateByCIPKID(CIPkIdAddr + 1, certLen, &ref) == ADDR_IS_NULL)// CI-->EUM-->EUICC存在
			{
				EuiccCAInfoAddr = GetAddrByIndexAddr(CACIListGet_nextCIListRefAddr(EuiccCAInfoAddr));//GetAddrByIndexAddr(EuiccCAInfoAddr + CACIList_NextListRef_Offset);
				continue;	
			}			
		}			
	
		temp[0] = 0x04;
		
		temp[1] = certLen; // 1字节长度 
		
		if(((certLen + 2) > (freeLen - totleLen)))
		{
			break; //ThrowError(Card_Ability_Exception); // 这里是抛异常合适，还是break合适？调用处传入的OutBufAddr是tempBuf，一个CI的PK是0x14字节，所以这里约可以获取到0x19个证书信息
		}
		
		octerNum = ASN1_PaddingBERTlvTagAndLen(tempTL,tag, totleLen + certLen + 2); //把TL填入时的越界判断提前，避免数据读取完成后发现TL加入之后的越界

		if((octerNum + totleLen + certLen + 2) > freeLen)
		{
			break;
		}

		VMReadBuf(&temp[2],CIPkIdAddr + 1,certLen); //L + V (1字节L ，pkid)   每个pkid的len都是0x14，根据规范X509 4.2.1.2  keyid生成规则 The keyIdentifier is composed of the 160-bit SHA-1 hash of the value of the BIT STRING subjectPublicKey (excluding the tag, length, and number of unused bits). Other methods of generating unique numbers are also acceptable（假设最大长度不超过1字节）						
		
		ecpMem_Copy(OutBufAddr + totleLen,&temp[0],certLen + 2); //从OutBufAddr + 3开始拼数据 预留tag 81 L 这3个字节；    为避免有多个CIList的时候 总数据过长 每获取一个PKID就写入输出缓存
		
		totleLen += (certLen + 2); 
		
		EuiccCAInfoAddr = GetAddrByIndexAddr(CACIListGet_nextCIListRefAddr(EuiccCAInfoAddr));//GetAddrByIndexAddr(EuiccCAInfoAddr + CACIList_NextListRef_Offset);
	}
	
	octerNum = ASN1_PaddingBERTlvTagAndLen(temp,tag, totleLen);
	
	ecpMem_Move(OutBufAddr + octerNum, OutBufAddr, totleLen);
	
	ecpMem_Copy(OutBufAddr,&temp[0],octerNum);
		
	totleLen += octerNum;
	
	return totleLen;
}


/*
*@function     : IsAccessConditionAllowed
*
*@description  :判断文件的访问条件是否允许
**
*@param        :UWORD fileAddr --   文件地址
*@param        :BYTE action  --  操作(read,update......)
*@return       :BYTE :     TRUE/FALSE
*@author       :zhxx@20210428
*
*@hisotry      :V1.0  EPX产业化\STM_ST33G1M2\DEVELOP
*/

BYTE IsAccessConditionAllowed(UWORD fileAddr,BYTE action)
{
		UWORD xdata arrFileAddr;
		BYTE idata arrNum,fileLgth,tempPosition,status;
		BYTE xdata arrRecContent[0x40];
		KeyStructer xdata chvStuct;
#if UICC_AB8C_ARR_PRIVATE	
		BYTE xdata arrRecTemp[0x42];
#endif
	    arrFileAddr = SelectLFFile(DF_MF,EF_ARR);
	
		if(arrFileAddr == FALSE)
		{
			return FALSE; //modify by lixin@20230313: 3F00->2F06不存在时返回FALSE
		}

	    arrNum = GetFileHeadByte(fileAddr, F_ArrNum_Offset);

	  	//2F06FF和2F06FE两条记录专门给私有文件使用，权限为全部NEV和ADM, by xwh 20190321
	    if(arrNum == ARR_RECNUM_NEV)
		{
			return FALSE;
		}
		else if(arrNum == ARR_RECNUM_ADM)
		{
			return FALSE;
		}

#if UICC_AB8C_ARR_PRIVATE
		if((arrNum&A_PRIVATEARR_REC_FLAG)==A_PRIVATEARR_REC_FLAG)
		{
			fileLgth = GetPrivateArrInfoToExpand(arrNum,arrRecContent,arrRecTemp);
		}
		else
#endif
		{
			fileLgth = GetLFRecNumLgth(arrFileAddr,FileLgth);
			ReadLFBuf(arrFileAddr,arrNum,0,&arrRecContent[0],fileLgth);
		}
		
		for(arrNum =0x00; ((arrNum < fileLgth)&&(arrRecContent[arrNum] != 0xFF)); arrNum = arrNum + 5+ tempPosition)
		{
			if((arrRecContent[arrNum]!=0x80)&&(arrRecContent[arrNum]!=0x84)&&(arrRecContent[arrNum+1]!=0x01))
			{
				return FALSE;
			}

			tempPosition = arrRecContent[arrNum+4];

			if((((arrRecContent[arrNum+2]&action) == action)&&(arrRecContent[arrNum] == 0x80)&&(action != A_IncreaseFile)&&(action != A_ResizeFile))||((arrRecContent[arrNum]==0x84)&&((action == A_IncreaseFile)||(action == A_ResizeFile))&&(arrRecContent[arrNum+2]==action)))
			{
				switch(arrRecContent[arrNum+3])
				{
					case 0x90:
						return TRUE;

					case 0x97:
						return FALSE;

					case 0xA4:
					{
						if((arrRecContent[arrNum+5]!=0x83)&&(arrRecContent[arrNum+6]!=0x01))
						{
							return FALSE;
						}

						status = ReadKey(arrRecContent[arrNum +4+3],(BYTE xdata *)&chvStuct.KeyId);

						if(status == TRUE)
						{
							if((chvStuct.KeyStatus&0x01)==PIN_DISABLED)//设置glocal pin1的状态
							{
								return TRUE;
							}
						}

						return GetKeyRefenseStatus(arrRecContent[arrNum +4+3]);
					}

					default:
					{
						return  FALSE;
					}
				}
			}
		}

		return FALSE;

}


/*
*@function     : GetProfileOwnerInfo
*
*@description  :拼装ProfileOwnerInfo，当profile metadata中没有ProfileOwnerInfo时，根据条件是否允许，从文件中获取mccmnc，gid1，gid2，拼装ProfileOwnerInfo
**
*@param        :输出参数，BYTE* buf --- 拼装数据存放的数组
*@param        :输入参数，BYTE freeLen --- buf中可用的字节数
*@param        :输入参数，BYTE profileIndex --- profile index
*
*@return       :WORD off     本次拼装的数据总长度
*
*@author       :zhxx@20210428
*
*@hisotry      :V1.0
*@hisotry      ：modify by xuxiaojun@20230313 增加参数freeLen，用于数据写入buf时的越界判断
*/
WORD GetProfileOwnerInfo(BYTE* buf, WORD freeLen, BYTE profileIndex)
{
	BYTE idata mncLen;
	UWORD xdata fileAddr,backMFAddr;
	WORD idata fileSize,off,usimADFFID;
	BYTE xdata mccmncBuf[7];
	
	backMFAddr = E_FILE_START;
	
	E_FILE_START = VMReadUword(PROFILE_MFOFFSET_CFG + profileIndex*sizeof(UWORD));

	usimADFFID = GetADFFID((BYTE *)&USIMPartial[0]);  // add by chentao 20230215 选择对应profile的usim

	mncLen = 0;

	off = 0;

	//The profileOwner data object can only be returned if Profile Owner has been provided in Profile Metadata or if files EFIMSI, EFGID1 or EFGID2 are not PIN protected.

	if(freeLen < 2)
	{
		E_FILE_START = backMFAddr;

		return 0x00; // buf越界
	}

	buf[off++] = 0xB7;//profileOwner Tag

	buf[off++] = 0;//profileOwner Len

	fileAddr = SelectTFFile(usimADFFID,EF_AD);	//选择文件EFAD (Administrative data)
	
	if(fileAddr == FALSE)
	{
		E_FILE_START = backMFAddr;

		return 0;
	}

	ReadTFBuf(fileAddr,0x03,&mncLen,1);//length of  MNC in the IMSI

	fileAddr = SelectTFFile(usimADFFID,EF_IMSI);	//选择IMSI
	
	if(fileAddr == FALSE)
	{
		E_FILE_START = backMFAddr;

		return 0;
	}

	if(IsAccessConditionAllowed(fileAddr,A_ReadFile) == TRUE)
	{
		ReadTFBuf(fileAddr,0x01,&mccmncBuf[0],4);	//6F07 第2个字节开始的3个字节

		MNNMNCConvert(&mccmncBuf[0],&mccmncBuf[4],mncLen);//将IMSI装换为MCCMNC

		if((off + 2 + 3) > freeLen)
		{
			E_FILE_START = backMFAddr;

			return 0x00; // buf越界
		}

		buf[off++] = 0x80; //mccMnc Tag

		buf[off++] = 0x03; //mccMnc Len

		ecpMem_Copy(buf+off,&mccmncBuf[4],3); //mccMnc value

		off += 3;
	}

	fileAddr = SelectTFFile(usimADFFID,0x6F3E);

	if(fileAddr != FALSE)
	{
		if(IsAccessConditionAllowed(fileAddr,A_ReadFile) == TRUE)
		{
			fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);

			if((off + 2 + fileSize) > freeLen)
			{
				E_FILE_START = backMFAddr;

				return 0x00; // buf越界
			}

			buf[off++] = 0x81;//gid1 Tag

			buf[off++] = fileSize&0xFF; ////gid1 Len

			ReadTFBuf(fileAddr,0,buf+off,fileSize); ////gid1 Value

			off += fileSize;
		}

	}

	fileAddr = SelectTFFile(usimADFFID,0x6F3F);

	if(fileAddr != FALSE)
	{
		if(IsAccessConditionAllowed(fileAddr,A_ReadFile) == TRUE)
		{
			fileSize = GetFileHeadWord(fileAddr, F_SpaceSize_Offset);

			if((off + 2 + fileSize) > freeLen)
			{
				E_FILE_START = backMFAddr;

				return 0x00; // buf越界
			}

			buf[off++] = 0x82; //gid2 Tag

			buf[off++] = fileSize&0xFF; //gid2 Len

			ReadTFBuf(fileAddr,0,buf+off,fileSize); //gid2 Value

			off += fileSize;
		}

	}

	buf[1] = off-2;
	
	E_FILE_START = backMFAddr;

	return (off == 2) ? 0 : off;
}

/*
*@function     : GetProfileInfoDataByTagList
*
*@description  :CUSTOMER_GetProfilesInfo子函数，用于获取单个ProfilesInfo
*
*@see          : 5.7.15 Function (ES10c): GetProfilesInfo
*
*@param        :UWORD outAddr --- 拼装数据存放的地址
*@param        :WORD freeLen  --- outAddr允许存放的数据长度
*@param        :BYTE* tagList --- 需要拼装的tag  LV结构
*@param        :BYTE  profileIndex ---  需要拼装数据的profileIndex
*@return       :BYTE mode --- 数据拼装是按照GET_PROFILE_INFO_TAG_LIST模式还是GET_PROFILE_INFO_DEF_LIST模式
*
*@RAM Resources: tempBuf[0]开始使用，长度不确定 ，用于缓存多个tag的数据，内部调用的子函数也会用到tempBuf，使用前需要先将tempBuf中数据存下来
*
*@author       :lgy@20181117
*
*@hisotry      :V1.0
*@hisotry      :V1.1 增加参数freeLen用于越界判断，如果越界则忽略当前profile信息
*/
WORD GetProfileInfoDataByTagList(UWORD outAddr, WORD freeLen, BYTE* tagList,BYTE profileIndex, BYTE mode)
{
	UWORD xdata profileMetaDataAddr,tempAddr,tempOffset, tmpLen;
	WORD idata offset;
	BYTE idata i,refOffset,octerNum,j,tagLen,tag[2];
	BYTE xdata temp[4];
	WORD idata totalLen, iconOff;
	WORD idata tmpRef;
	
	offset = 4;  //E3 XX XX XX VALUE
	
	tempOffset = 0;
		
	profileMetaDataAddr = GetProfileMetaDataAddr(profileIndex);//GetAddrByIndexAddr(PROFILE_METADATAREF + profileIndex*sizeof(WORD));
	
	if(profileMetaDataAddr == ADDR_IS_NULL)
	{
		return 0x00;
	}
	
	for(i = 1; i <= tagList[0];i++)
	{
		if(tagList[i] == 0x5A)	//ICCID  TLV
		{
			tmpLen = ASN1_GetBERTlvData(GetMetaData_iccidAddr(profileMetaDataAddr),&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);

			if(tmpLen == 0x00)
			{
//				return 0x00; //该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				tempOffset += tmpLen;
			}
			
			continue;			
		}
		else if(tagList[i] == 0x4F)//ISD-P AID, tag '4F' (*)
		{
			if((tempOffset + 1 + 0x11) > sizeof(tempBuf))
			{
//				return 0x00; //该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				tempBuf[tempOffset++] = 0x4F;

				tempBuf[tempOffset] = GetProfileAIDByProfileIndex(profileIndex, &tempBuf[tempOffset + 1]);

				tempOffset += (tempBuf[tempOffset] + 1);
			}
			
			continue;
		}
		else if((tagList[i] == 0x9F)&&(tagList[i + 1] == 0x70))
		{
			if((tempOffset + 4) > sizeof(tempBuf))
			{
//				return 0x00; //该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				tempBuf[tempOffset++] = 0x9F;
				tempBuf[tempOffset++] = 0x70;
				tempBuf[tempOffset++] = 0x01;//Len
				tempBuf[tempOffset++] = GetProfileState(profileIndex);//VMReadByte(PROFILE_STATE_CFG + profileIndex);
			}
			i++; // tag 为2字节  需要再+1 指向下一个tag
			
			continue;
		}
		else if(tagList[i] == 0x90)//Profile Nickname, tag '90' (*) 
		{						
			refOffset = MetaDataProfileNickNameRef_INDEX;//MetaData_ProfileNickNameRef_Offset;
		}
		else if(tagList[i] == 0x91)//Service provider name, tag '91' (*)
		{
			refOffset = MetaDataServiceProviderNameRef_INDEX;//MetaData_ServiceProviderNameRef_Offset;
		}
		else if(tagList[i] == 0x92)//Profile name, tag '92' (*)
		{
			refOffset = MetaDataProfileNameRef_INDEX;//MetaData_ProfileNameRef_Offset;
		}
		else if(tagList[i] == 0x93)
		{
			if(VMReadByte(GetMetaData_iconTypeAddr(profileMetaDataAddr) + 1) != 0)// 该tag是可选项 可能不存在/*profileMetaDataAddr + MetaData_IconType_Offset*/
			{
				tmpLen = ASN1_GetBERTlvData(GetMetaData_iconTypeAddr(profileMetaDataAddr),&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);/*profileMetaDataAddr + MetaData_IconType_Offset*/

				if(tmpLen == 0x00) 
				{
//					return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
				}
				else
				{
					tempOffset += tmpLen;
				}
			}
			continue;		
		}
		else if(tagList[i] == 0x94)//Icon
		{												
			tempAddr = GetMetaData_iconRefValueAddr(profileMetaDataAddr);//GetAddrByIndexAddr(profileMetaDataAddr + MetaData_IconRef_Offset);
			
			if(tempAddr ==  ADDR_IS_NULL)
			{
				continue;  //The list SHALL be empty if no Profile is installed or if no Profile matches the given search criteria.
			}
			
			if((offset + tempOffset) > freeLen)
			{
//				return 0; //该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				VMWriteBuf(outAddr + offset,tempBuf,tempOffset); //写icon前把之前拼装在tempBuf的所有数据写到E2中

				offset += tempOffset; //距离outAddr的偏移

				octerNum = ASN1_GetBERTlvLen(tempAddr+1,(UWORD*)&tempOffset); //获取icon的总长度

				//以下代码可以用 VMWriteEepromOrRam((BYTE*)outAddr + offset, (BYTE*)tempAddr, tempOffset + octerNum + 1);  offset += (tempOffset + octerNum + 1);替代
				
				totalLen = tempOffset + 1 + octerNum;
				
				if((offset + totalLen) > freeLen)
				{
//					return 0; //该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
				}
				else
				{
					for(iconOff = 0 ;iconOff < totalLen;  )
					{
						if((totalLen - iconOff) > sizeof(tempBuf))
						{
							tempOffset =  sizeof(tempBuf);
						}
						else
						{
							tempOffset = totalLen - iconOff;
						}

						VMReadBuf(tempBuf, tempAddr+iconOff, tempOffset);

						VMWriteBuf(outAddr +offset +iconOff, tempBuf,tempOffset);

						iconOff += tempOffset;
					}

					offset += totalLen;
				}
			}
			tempOffset = 0;  //初始化为0  后续tag从tempBuf[0]开始缓存在RAM中
			
			continue;
			
		}
		else if(tagList[i] == 0x95)//Profile Class, tag '95' (*)
		{
			tmpLen = ASN1_GetBERTlvData(/*profileMetaDataAddr + MetaData_ProfileClass_Offset*/GetMetaData_profileClassAddr(profileMetaDataAddr),&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);

			if(tmpLen == 0x00) 
			{
//				return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				tempOffset += tmpLen;
			}
			continue;
		}
		else if(tagList[i] == 0xB6)//Notification Configuration Info, tag 'B6'
		{
			refOffset = MetaDataNotificationConfigurationInfoRef_INDEX;//MetaData_NotificationConfigurationInfoRef_Offset;
		}
		else if(tagList[i] == 0xB7)//Profile Owner, tag 'B7'
		{
			refOffset = MetaDataProfileOwerRef_INDEX;//MetaData_ProfileOwerRef_Offset;
		}
		else if(tagList[i] == 0xB8)//SM-DP+ proprietary data, tag 'B8'
		{
			refOffset = MetaDataDpProprietaryDataRef_INDEX;//MetaData_DpProprietaryDataRef_Offset;
		}
		else if(tagList[i] == 0x99)//Profile Policy Rules, tag '99'  ,testclass的profile是否要返回该字段？by  xuyz
		{
			if(VMReadByte(GetMetaData_profilePolicyRulesAddr(profileMetaDataAddr) + 3) != 0)// 该tag是可选项 可能不存在/*profileMetaDataAddr + MetaData_ProfilePolicyRules_Offset*/
			{
				tmpLen = ASN1_GetBERTlvData(GetMetaData_profilePolicyRulesAddr(profileMetaDataAddr),&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);/*profileMetaDataAddr + MetaData_ProfilePolicyRules_Offset*/

				if(tmpLen == 0x00) 
				{
//					return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
				}
				else
				{
					tempOffset += tmpLen;
				}
			}
			continue;
		}
#if RSP_SGP22_BF22
		else if((tagList[i] == 0xBF)&&(tagList[i+1]) == 0x22)
		{
			refOffset = MetaDataserviceSpecificDataStoredInEuicc_INDEX;
		}
#endif
		else
		{
			for(j = 0x00; j < UNSPECIFIED_TAG_NUMBER; j++)
			{
				tempAddr = GetMetaData_unspecifiedTagValueAddr(profileMetaDataAddr) + j*sizeof(WORD);

				tmpRef = memReadU16(MEMtPM8(tempAddr));

				if(tmpRef != REF_NULL)
				{
					tempAddr = locateObjectInMemory(tmpRef);

					tempAddr = get_array_data(MEMtPN8(tempAddr));

					tagLen = ASN1_GetBERTlvTag(tempAddr, tag);

					if((tagLen == 0x01)&&(tagList[i] == tag[0]))
					{
						tmpLen = ASN1_GetBERTlvData(tempAddr,&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);

						if(tmpLen == 0x00) 
						{
//							return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
						}
						else
						{
							tempOffset +=tmpLen;
						}

						break;
					}
					else if((tagLen == 0x02)&&(tagList[i] == tag[0])&&(tagList[i+1] == tag[1]))
					{
						tmpLen = ASN1_GetBERTlvData(tempAddr,&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);

						if(tmpLen == 0x00) 
						{
//							return 0x00;
						}
						else
						{
							tempOffset += tmpLen;
						}

						i++;

						break;
					}
				}
			}
			continue;
		}
		
		tempAddr = GetMetaData_ArrayItemAddr(profileMetaDataAddr , refOffset);//GetAddrByIndexAddr(profileMetaDataAddr + refOffset);
		
		if(tempAddr != ADDR_IS_NULL)
		{
			tmpLen = ASN1_GetBERTlvData(tempAddr,&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);

			if(tmpLen == 0x00) 
			{
//				return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				tempOffset += tmpLen;
			}
		}
		else if(refOffset == MetaDataProfileOwerRef_INDEX)//according SGP22.V2.2.2:The profileOwner data object can only be returned if Profile Owner has been provided in Profile Metadata or if files EFIMSI, EFGID1 or EFGID2 are not PIN protected.
		{
			tmpLen = GetProfileOwnerInfo(&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset, profileIndex);

			if(tmpLen == 0x00) 
			{
//				return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
			}
			else
			{
				tempOffset += tmpLen;
			}
		}
	}
	
	if(mode == GET_PROFILE_INFO_DEF_LIST)
	{
		for(j = 0x00; j < UNSPECIFIED_TAG_NUMBER; j++)
		{
			tempAddr = GetMetaData_unspecifiedTagValueAddr(profileMetaDataAddr) + j*sizeof(WORD);

			tmpRef = memReadU16(MEMtPM8(tempAddr));

			if(tmpRef != REF_NULL)
			{
				tempAddr = locateObjectInMemory(tmpRef);

				tempAddr = get_array_data(MEMtPN8(tempAddr));

				tmpLen = ASN1_GetBERTlvData(tempAddr,&tempBuf[tempOffset], TEMP_BUF_SIZE - tempOffset);

				if(tmpLen == 0x00)
				{
//					return 0x00;//该条件成立时表示越界导致的数据获取失败，继续拼下一个TAG的数据
				}
				else
				{
					tempOffset += tmpLen;
				}
			}
		}
	}
		
	if(tempOffset != 0)
	{
		if((offset + tempOffset) > freeLen)
		{
//			return 0x00; //该条件成立时表示越界导致的数据获取失败，不写入
		}
		else
		{
			VMWriteBuf(outAddr + offset, tempBuf, tempOffset); //剩余总长度写入目的地址

			offset += tempOffset;
		}
	}
	
	if((offset -4 ) == 0)//拼装的总长度为0 
	{
		return 0;
	}
	
	ecpMem_Set(temp,0,4);
	
 	tempOffset = ASN1_PaddingBERTlvTagAndLen(temp,TAG_ProfileInfo,offset-4);//数据总长度是offset-4 ,预留了4字节 tag len
	
	VMWriteBuf(outAddr,temp,tempOffset);
	
	if(tempOffset < 4) // 前面拼装的数据从outAddr + 4 开始写入的 如果tag和len字节数少于4  需要将数据往前移
	{
		VMWriteEepromOrRam((BYTE*)outAddr + tempOffset, (BYTE*)outAddr + 4, offset-4);//该函数内部需要使用tempBuf
	}
	
	return  (tempOffset + offset - 4);
}

/*
*@function     :GetForbiddenProfilePolicyRules
*
*@description  :获取forbiddenProfilePolicyRules
*
*@see          :A PPR is 'forbidden' when there is no PPAR related to this PPR. In addition, PPR1 is 'forbidden' if an Operational Profile is currently installed on the eUICC
*
*@param        :BYTE* buf --- 输出数据，格式为0x99 L V
*
*@return       :BYTE      --- buf中有效数据的长度
*
*@RAM Resources:
*
*@author       :xuxiaojun@20181119
*
*@test         :.R_R
*               C0C7000000 ;期望返回990100
*               C0C801001F 301D80020640A1133011800392F928810447534D418204525350FF82020780  ;set ppr1 only
*               C0C7000000 ;期望返回990205A0 / 99020520
* 
*               C0C8010016 301480020520A10B3009800392EEEE81008200820100                    ;set ppr2 only
*               C0C7000000 ;期望返回990206C0 / 99020640
*
*               C0C8010017 301580020560A10B30098003EEEEEE8100820082020780                  ;set ppr1 and ppr2
*               C0C7000000 ;期望返回99020780 / 990100
*
*               C0C8010017 301580020640A10B30098003EEEEEE8100820082020780                  ;set ppr1 first
*               C0C8010017 301580020520A10B30098003EEEEEE8100820082020780                  ;set ppr2 second
*               C0C7000000 ;期望返回99020780 / 990100
*
*@hisotry      :created by xuxiaojun@20181119
*/
BYTE GetForbiddenProfilePolicyRules(BYTE* buf)
{
	BYTE idata bitFlag, offset, size;	
	WORD idata PPARRef,curPPARArrayRef;
	UWORD xdata curPPARAddr,curPPARArrayAddr;
	ASN1Info xdata tempASN1Info;
	BYTE idata pprid[2];
	
	curPPARAddr = EUICCRATRef_Addr;
	
	PPARRef = memReadU16(MEMtPM8(EUICCRATRef_Addr));//获取到RAT列表中第一个PPAR的引用
	
	bitFlag = 0x00;
	
	while(PPARRef != REF_NULL)
	{
		curPPARAddr = locateObjectInMemory(PPARRef);//RATTableRef对象的地址

		curPPARArrayAddr = GetObjectItemAddr(curPPARAddr,PPARArray_INDEX);//存PPARArray[]的地址

		curPPARArrayRef = memReadU16(MEMtPM8(curPPARArrayAddr));

		curPPARArrayAddr = locateObjectInMemory(curPPARArrayRef);//PPARArray[]数组地址 指向数组头

		size = get_array_length(curPPARArrayAddr);

		curPPARArrayAddr = get_array_data(MEMtPF8(curPPARArrayAddr));
		
		ASN1_InitTLVInfo((ASN1Info *)&tempASN1Info,curPPARArrayAddr, size);
		
		if(FALSE == ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x30 , FALSE, 0x01))  //查找ProfilePolicyAuthorisationRule
		{
			return FALSE;
		}
		
		ASN1_SetNextDepthToFindTLV((ASN1Info *)&tempASN1Info);
		
		if((FALSE == ASN1_FindTLV((ASN1Info *)&tempASN1Info, 0x80 , FALSE, 0x01)) || (tempASN1Info.curValueLen != 0x02))  //查找pprIds
		{
			return FALSE;
		}
		
		VMReadBuf(pprid, tempASN1Info.bufferAddr + tempASN1Info.curValueOff, sizeof(pprid));
		
// 		if((pprid[1] & PPRIDS_pprUpdateControl) == PPRIDS_pprUpdateControl)      // PPRIDS_pprUpdateControl现在是如何配置？ 
// 		{
// 			bitFlag |= PPRIDS_pprUpdateControl;                 
// 		}
		
		if((pprid[1] & PPRIDS_ppr1) == PPRIDS_ppr1)
		{
			bitFlag |= PPRIDS_ppr1;                
		}
		
		if((pprid[1] & PPRIDS_ppr2) == PPRIDS_ppr2)
		{
			bitFlag |= PPRIDS_ppr2;                
		}
		
		curPPARAddr = GetObjectItemAddr(curPPARAddr,nextRATTable_INDEX);//下一个RATTable;
		
		PPARRef = memReadU16(MEMtPM8(curPPARAddr));
	}
	
	if(0 != GetProfileIndexByProfileClass(PROFILE_CLASS_OPERATIONAL, NULL))   // In addition, PPR1 is 'forbidden' if an Operational Profile is currently installed on the eUICC
	{
//		bitFlag |= PPRIDS_ppr1;
		ClearBit(bitFlag, 6);
	}
	
	offset = 7;
	
	if(bitFlag != 0)
	{ 
		bitFlag = ((~(bitFlag >> 5))<<5);  

		for(offset = 5; offset < 7; offset++)
		{
			if(0x01 == ValueBit(bitFlag, offset))
			{
				break;
			}
		}		
	}	
	
	ClearBit(bitFlag,0x07); // PPRIDS_pprUpdateControl现在配置成0
	
	buf[0] = 0x99;
	
	if(offset == 7)   // 表示forbiddenProfilePolicyRules不存在
	{		
		return 0;
	}

	buf[1] = 0x02;
	
	buf[2] = offset;
	
	buf[3] = bitFlag;
	
	return (buf[1] + 2);
}
/*
*@function   :PPRsVerification
*
*@description:在profile的安装过程中验证 如store metadata 验证PPRs的有效性. profileOwner与profilePolicyRules同时存在时才会进入该函数。进入该函数之前，已经使用StoreMetaData_verifyProfileOwner验证了OperatorId的正确性
*
*@SEE        :SGP22_V2.2  2.9.3.1  PPRs Verification: Profile installation time
*             OperatorId ::= SEQUENCE { 
*                 mccMnc OCTET STRING (SIZE(3)), -- MCC&MNC coded as 3GPP TS 24.008 
*                 gid1 OCTET STRING OPTIONAL, -- referring to content of EF GID1 (file identifier '6F3E') in 3GPP TS 31.102 [54] 
*                 gid2 OCTET STRING OPTIONAL  -- referring to content of EF GID2 (file identifier '6F3F') in 3GPP TS 31.102 [54] 
*             }
*
*@param      :BYTE PPRs           --- ppr值
*@param      :BYTE *OperatorId    --- profileOwner的值域，不包括TL，格式如：8003EEEEEE(mccMnc) 8100(gid1) 8200(gid2)
*@param      :BYTE OperatorIdLen  --- profileOwner的长度
*
*@return     :TRUE:PPRS验证通过 /FALSE: :PPRS验证不通过
*
*@author     :xuxiaojun@20181129
*
*@test       :
*@history    :V1.0 -- created by xuxiaojun@20181129
*注意        :函数中假设gid1和gid2的vaule域的长度不会超过0x7F
*/
BYTE PPRsVerification(BYTE ppr, BYTE  *OperatorId, BYTE OperatorIdLen)
{
	BYTE idata lengthOfPPARLen, i, ppridInRAT, tag;	
	WORD idata PPARRef, gid1Flag, gid2Flag, gidFlag, gidInRAT;
	UWORD xdata curPPARAddr, curPPARArrayAddr, PPARVauleAddr, curOperatorValueAddr, currentPPARLen;
	BYTE idata pprid[2], mccMnc[3];	
	UWORD xdata  gidLen, allOperatorLen, curOperatorLen;
	BYTE idata lengthOfAllOperatorLen;
	BYTE idata lengthOfCurOperatorLen;
	BYTE idata offset, operationalProfile;
	
	ecpMem_Set(pprid, 0xFF, 0x02);//初始化为0xFF，pprid[0]表示是否需要判断PPR1, pprid[1]表示是否需要判断PPR2，0xFF表示不需要判断。 0表示校验不通过，1表示校验通过
	
	operationalProfile = GetProfileIndexByProfileClass(PROFILE_CLASS_OPERATIONAL, NULL);//确定卡内是否存在Operational Profile.  In addition, PPR1 is 'forbidden' if an Operational Profile is currently installed on the eUICC
	
	if((ppr & PPRIDS_ppr1) == PPRIDS_ppr1) //确定profilePolicyRules包含了PPR1
	{
		pprid[0] = FALSE;
	}
	
	if((ppr & PPRIDS_ppr2) == PPRIDS_ppr2) //确定profilePolicyRules包含了PPR2
	{
		pprid[1] = FALSE;
	}

	if((pprid[0] == 0xFF) && (pprid[1] == 0xFF))  // 不需要验证
	{
		return TRUE;
	}
		
	gid1Flag = TAG_NOT_FOUND;
	
	gid1Flag = ASN1_FindTagTLV((UWORD)&OperatorId[0], OperatorIdLen, 0x81);  //查找gid1, OperatorId[gid1Flag]处是0x81
	
	gid2Flag = TAG_NOT_FOUND;
	
	gid2Flag = ASN1_FindTagTLV((UWORD)&OperatorId[0], OperatorIdLen, 0x82);  //查找gid2, OperatorId[gid2Flag]处是0x82
	
	curPPARAddr = EUICCRATRef_Addr;
	
	PPARRef = memReadU16(MEMtPM8(EUICCRATRef_Addr));
	
	while(PPARRef != REF_NULL)   //在查找RAT中PPR时，不用考虑格式的正确性，正确性由个人化保证
	{		
		curPPARAddr = locateObjectInMemory(PPARRef);//RATTableRef对象的地址

		curPPARArrayAddr = GetObject_ArrayItemAddr(curPPARAddr,PPARArray_INDEX);//PPARArray[]数据体的地址

		lengthOfPPARLen = ASN1_GetBERTlvLen(curPPARArrayAddr + 1, (UWORD*)&currentPPARLen);  //值域的编码格式为： 30 1D 80020640(ppr) A1133011800392F928810447534D418204525350FF(operatorID) 82020780(UserConsent)

		PPARVauleAddr = curPPARArrayAddr + 1 + lengthOfPPARLen;                  //80020640(ppr) A1133011800392F928810447534D418204525350FF(operatorID) 82020780(UserConsent)
		
		ppridInRAT = VMReadByte(PPARVauleAddr + 3);                                                          //RAT中的PPR肯定存在，所以偏移是固定的，不需要查找。3字节是800206
		
		lengthOfAllOperatorLen = ASN1_GetBERTlvLen(PPARVauleAddr + 5, (UWORD*)&allOperatorLen);               //5字节是80020640 A1
		
		offset = 0;   //用于记录operatorId与第一个operatorId起始的偏移
		
		while(allOperatorLen > 0)    // 循环处理多个OperatorId, 第一个30的起始偏移是nextPPARAddr + RATTableList_PPARValue_Offset + 1 + lengthOfPPARLen + 5 + lengthOfOperatorLen，所有30的总长度为operatorLen
		{
			lengthOfCurOperatorLen = ASN1_GetBERTlvLen(PPARVauleAddr + 5 + lengthOfAllOperatorLen + offset + 1, (UWORD*)&curOperatorLen); 
			
			curOperatorValueAddr = PPARVauleAddr + 5 + lengthOfAllOperatorLen + offset + 1 + lengthOfCurOperatorLen;  //800392F928 810447534D41 8204525350FF(operatorID)

			VMReadBuf(mccMnc, curOperatorValueAddr + 2, sizeof(mccMnc));
				
			tag = FALSE;
			
			for(i = 0; i<3; i++)   //先比较mccMnc是否一致，由于OperatorId中不允许有E，所以这里就不需要判断OperatorId中是否存在E的情况了
			{
				if((((mccMnc[i] &0x0F) != 0x0E)&&((mccMnc[i]&0x0F) != (OperatorId[2+i]&0x0F))) || 
					(((mccMnc[i] &0xF0) != 0xE0)&&((mccMnc[i]&0xF0) != (OperatorId[2+i]&0xF0))))
				{
					tag = TRUE;   //借用tag来表示是否需要结束本次的operatorId判断，偏移到当前PPAR的下一个operatorID
					
					break;   
				}
			}
				
			if(tag == TRUE)
			{
				offset += (1 + lengthOfCurOperatorLen + curOperatorLen);            //偏移到下一个operatorId
					
				allOperatorLen -= (1 + lengthOfCurOperatorLen + curOperatorLen);
				
				continue;     //结束当前operatorID的查找，继续查找下一个operator
			}
			 
			for(i = 0; i<2; i++)  //mccMnc验证通过之后，判断gid1 和 gid2是否可以验证通过
			{
				switch(i)
				{
					case 0x00:
					{
						tag = 0x81;
						
						gidFlag = gid1Flag;
						
						break;
					}
					case 0x01:
					{
						tag = 0x82;
						
						gidFlag = gid2Flag;
						
						break;
					}
					default: 			  //此处为了白盒测试添加
						break;
				}
				
				gidInRAT = ASN1_FindTagTLV(curOperatorValueAddr, curOperatorLen, tag); // 查找30LV下的81LV 82LV)//  An omitted gid1 or gid2 value in the PPAR SHALL only match a profileOwner field where the corresponding gid1 or gid2 value is absent.
				
				if(gidInRAT == TAG_NOT_FOUND) 
				{
					if(gidFlag == TAG_NOT_FOUND) // An omitted gid1 or gid2 value in the PPAR SHALL only match a profileOwner field where the corresponding gid1 or gid2 value is absent.
					{
						continue;
					}
					else
					{
						tag = TRUE;
						
						break;   // 跳出 for(i = 0; i<2; i++)循环，偏移到下一个operatorId
					}
				}
				
				ASN1_GetBERTlvLen(curOperatorValueAddr + gidInRAT + 1, (UWORD*)&gidLen);  //gid的长度不会超过0x7F
				
				if(gidLen == 0) // The gid1 or gid2 data objects can be wildcard-ed by setting an empty value (length zero).
				{
// 					if(gidFlag == TAG_NOT_FOUND)  // profileOwner的gid不存在，或者存在但是长度为0，都认为是通配的情况
// 					{
// 						tag = TRUE;
// 						
// 						break;   //如果gid1验证失败，则继续查找下一个OperatorId
// 					}
				}
				else  // If present in the PPAR, a non-empty gid1 or gid2 value SHALL exactly match the corresponding value in the profileOwner field
				{
					if(gidFlag == TAG_NOT_FOUND)  // profileOwner中的gid1不存在则认为不匹配;
					{
						tag = TRUE;
						
						break;   //如果gid1验证失败，则继续查找下一个OperatorId
					}
					
					if(((OperatorId[gidFlag + 1]) != 0) &&
						( ((OperatorId[gidFlag + 1]) != gidLen) || 
						  (EQUAL != NvmCmp((BYTE*)&OperatorId[gidFlag+2], (BYTE*)(curOperatorValueAddr + gidInRAT + 2), gidLen))))
					{
						tag = TRUE;
						
						break;   //如果gid1验证失败，则继续查找下一个OperatorId
					}
				}
			}	
				
			if(tag == TRUE)
			{
				offset += (1 + lengthOfCurOperatorLen + curOperatorLen);            //偏移到下一个operatorId
					
				allOperatorLen -= (1 + lengthOfCurOperatorLen + curOperatorLen);
				
				continue;     //结束当前operatorID的查找，继续查找下一个operator
			}
				
			if(((ppridInRAT & PPRIDS_ppr1) == PPRIDS_ppr1) && ((ppr & PPRIDS_ppr1) == PPRIDS_ppr1) && (operationalProfile == 0x00))  // ppr1验证通过, <RSP_EUICC测试_PPR规则测试\PPR_Rules_Test_Error_TSE01.html>
			{
				pprid[0] |= TRUE;
			}
			
			if(((ppridInRAT & PPRIDS_ppr2) == PPRIDS_ppr2) && ((ppr & PPRIDS_ppr2) == PPRIDS_ppr2))  // ppr2验证通过
			{
				pprid[1] |= TRUE;
			}

			if((pprid[0] != FALSE) && (pprid[1] != FALSE))  // PPR验证通过
			{
				return TRUE;
			}
			
			offset += (1 + lengthOfCurOperatorLen + curOperatorLen);            //偏移到下一个operatorId
					
			allOperatorLen -= (1 + lengthOfCurOperatorLen + curOperatorLen);
		}	

		curPPARAddr = GetObjectItemAddr(curPPARAddr,nextRATTable_INDEX);//下一个RATTable;
		
		PPARRef = memReadU16(MEMtPM8(curPPARAddr));
		
	} // while(PPARRef != REF_NULL)， 所有的PPAR遍历完
	
	return FALSE;
}
/*
*@function   :StoreMetaData_verifyProfileOwner
*
*@description:在storeMetaData中验证profileOwer的合法性，进入到该函数时，OperatorId必须是存在的
*             In this instance the mccMnc field SHALL not specify any wildcard ('E') digits.  Coding of mccMnc: contains MCC (3 digits) and MNC (2 or 3 digits) on 3 bytes coded as in 3GPP TS 24.008 [32]. For instance, an Operator identified by 246 for the MCC and 81 for the MNC will be coded as bytes 1 to 3: '42' 'F6' '18'
*
*             OperatorId ::= SEQUENCE { 
*                 mccMnc OCTET STRING (SIZE(3)), -- MCC&MNC coded as 3GPP TS 24.008 
*                 gid1 OCTET STRING OPTIONAL, -- referring to content of EF GID1 (file identifier '6F3E') in 3GPP TS 31.102 [54] 
*                 gid2 OCTET STRING OPTIONAL  -- referring to content of EF GID2 (file identifier '6F3F') in 3GPP TS 31.102 [54] 
*             }
*
*@param      :BYTE *OperatorId    --- profileOwner的值域，不包括TL，格式如：8003EEEEEE(mccMnc) 8100(gid1) 8200(gid2)
*@param      :BYTE OperatorIdLen  --- profileOwner的长度  
*
*@return     :TRUE:验证通过 /FALSE:验证失败
*@test       :.R_R
*             C0C700000A 80031234568101028200(9000)                                   
*             C0C700000A 80031E34568101018200(6985)   
*             C0C700000A 8003E234568101018200(6985)                                  
*             C0C700000A 800312E4568101018200(6985)  
*             C0C700000A 8003123E568101018200(6985)  
*             C0C700000A 80031234E68101018200(6985)  
*             C0C700000A 800312345E8101018200(6985) 
*
*@author     :xuxiaojun@20181203
*
*@history    :created by xuxiaojun@20181203
*/
BYTE StoreMetaData_verifyProfileOwner(BYTE  *OperatorId, BYTE OperatorIdLen)
{
	if((OperatorIdLen < 0x05) || (OperatorId[0] != 0x80) || (OperatorId[1] != 0x03) ||
		((OperatorId[2]&0xF0) == 0xE0) || ((OperatorId[2]&0x0F) == 0x0E)||
	    ((OperatorId[3]&0xF0) == 0xE0) || ((OperatorId[3]&0x0F) == 0x0E)||
	    ((OperatorId[4]&0xF0) == 0xE0) || ((OperatorId[4]&0x0F) == 0x0E))
	{
		return FALSE;
	}
	return TRUE;
}
/*
*@function    :StoreMetaData_ProcessEachMetaData88
*
*@description:处理storeMetada中的 88 序列
*
*@param      :void
*
*@return     :RESPONSE_NoError/ 0xFF/ other error
*
*@author     :xuyz
*
*@history    :
*/
BYTE StoreMetaData_ProcessEachMetaData88(void)
{
	BYTE idata tmp;
	WORD idata offset,tmpLen;
	UWORD xdata dataStartAddr, lenof88;
	
	tmp = ASN1_GetBERTlvLen(EUICCTempDataBuf + G_RSPSessionContext.clearDataLen+1, &lenof88);
	
	if(lenof88 < 0x08)  //有效数据不足一个mac字段
	{
		return ErrorReason_scp03tSecurityError;
	}
	lenof88 = SCP03tDecryptAndCalMac(EUICCTempDataBuf + G_RSPSessionContext.clearDataLen,EUICCTempDataBuf + G_RSPSessionContext.clearDataLen, lenof88+tmp+1, SECURITYLEVEL_CMAC,&G_RSPSessionContext.macKey[0], 0x10, &G_RSPSessionContext.initMacValue[0],&G_RSPSessionContext.encKey[0],0x10,&G_RSPSessionContext.icvCounter[0]);
	
	if(lenof88 == FALSE)
	{
		return ErrorReason_scp03tSecurityError;
	}
	
	dataStartAddr = EUICCTempDataBuf + G_RSPSessionContext.clearDataLen + tmp+1;  //88 序列中有效数据的起始位置
	
	for(offset = 0 ; offset < lenof88; )
	{
		if((lenof88-offset) >= sizeof(tempBuf))
		{
			tmpLen = sizeof(tempBuf);
		}
		else
		{
			tmpLen = lenof88-offset;
		}
		
		VMReadBuf(tempBuf, dataStartAddr + offset ,tmpLen);
		
		VMWriteBuf(EUICCTempDataBuf + G_RSPSessionContext.clearDataLen + offset, tempBuf , tmpLen); // 这里EUICCTempDataBuf不会越界，因为在外层收数据时已经做了越界判断
		
		offset += tmpLen;
	}
	
	G_RSPSessionContext.remainSequence -= G_CustomerStoreDataApdu.storeDataLen;   
	
	G_RSPSessionContext.clearDataLen += (lenof88-tmp-1);    //明文数据的总长度	,lenof88 是包括前面88的TL的长度的
	
	if(G_RSPSessionContext.remainSequence == 0x00)  //storeMedata接收完整，进行storeMetaData的正式处理
	{
		return RESPONSE_NoError;
	}
	else
	{
		return 0xFF;  //继续接收后面的88 tag
	}
}
/*
*@function   : PrepareDownload_VerifySmdpSigned2
*
*@description: 检查PrepareDownload 命令中 smdpSigned2 字段的有效性，如transactionId 是否是会话中的，指定的字段是否全部存在
*
*@see         :SGP22 5.7.5  Function (ES10b): PrepareDownloa
*
*@param       :UWORD dataAddr  :SmdpSigned2 的起始地址，30 L ....
*@param       :BYTE dataBuf[2] ; 缓存 ccRequiredFlag 和是否需要产生新的otKey
*
*@return      :RESPONSE_NoError/ other result
*
*@author       :
*
*@hisotry      :V1.0 modify by xixiaojun@20190709 代码书写规范
*@hisotry      V1.1 modify by chentao@20200811 修改是否支持EUICC__REUSE_OTPK可配置
*/
BYTE PrepareDownload_VerifySmdpSigned2(UWORD dataAddr, BYTE *dataBuf)
{
	BYTE idata tmp,tag[2],startTmp;
	WORD idata dataOff;
	UWORD xdata totalLen,tmpLen;

	dataBuf[0] = FALSE;
	
	dataBuf[1] = FALSE;  //  初始化dataBuf中的数据
	
	startTmp = ASN1_GetBERTlvLen((UWORD)(dataAddr+1), (UWORD*)&totalLen);
	
	dataOff = startTmp + 1;  //point to the first flag
	
	tag[0] = VMReadByte(dataAddr + dataOff);
	
	if(tag[0] == 0x80)  //transactionId Tag
	{
		tmp = ASN1_GetBERTlvLen((UWORD)(dataAddr+dataOff+1), (UWORD*)&tmpLen);
		
		if((tmpLen > TransactionIDMaxLen)||(tmpLen == 0x00) ||(tmp != 0x01))
		{
			return DownloadErrorCode_undefinedError;
		}
		
		if((NvmCmp((BYTE *)(dataAddr+dataOff+2),&G_RSPSessionContext.channllengeTransactionId[0], tmpLen) == EQUAL)&&(G_RSPSessionContext.transactionIdLen == tmpLen))
		{
			dataOff += (tmp + tmpLen + 1);  //poinit to the next flag  ccRequiredFlag BOOLEAN, --Indicates if the Confirmation Code is required 
		}
		else
		{
			return DownloadErrorCode_invalidTransactionId;
		}
	}
	else
	{
		return DownloadErrorCode_undefinedError;
	}
	
	if((dataOff - startTmp -1) >= totalLen)   //boolean tag不存在了
	{
		return DownloadErrorCode_undefinedError;
	}
	
	tag[0] = VMReadByte(dataAddr+dataOff);
	
	if(tag[0] == 0x01)  //ccRequiredFlag BOOLEAN, --Indicates if the Confirmation Code is required
	{
		tmp = ASN1_GetBERTlvLen((UWORD)(dataAddr+dataOff+1), (UWORD*)&tmpLen);
		
		if((tmpLen != 0x01) ||(tmp != 0x01))
		{
			return DownloadErrorCode_undefinedError;
		}
		
		dataBuf[0] = VMReadByte(dataAddr+dataOff+2);
		
		dataOff += (tmp + tmpLen + 1);
	}
	else
	{
		return DownloadErrorCode_undefinedError;
	}
	if (EUICC__REUSE_OTPK)
	{
		if((dataOff - startTmp -1) >= totalLen)   //tag '5F49'不存在了
		{
			dataBuf[1] = FALSE;  //本次需要重新产生一组key
		}
		else
		{
			 VMReadBuf(tag, dataAddr+dataOff, 0x02);  //读取两字节的tag
			
			if((tag[0] == 0x5F)&&(tag[1] == 0x49))
			{
				tmp = ASN1_GetBERTlvLen((UWORD)(dataAddr+dataOff+2), (UWORD*)&tmpLen);

				dataOff += (3 + tmp);  //point to the value   密钥格式为04 + value
				
				tmpLen -= 1;	// 密钥值的长度减去填充字节长度

				if(VMReadByte(EUICC_OpKeyBuf) != 0x00)
				{
					tmp = VMReadByte(EUICC_OpKeyBuf+2)  ;//读取公钥的长度 ,还应该检查OID是否也是关联的oid

					if((tmp == tmpLen) &&( NvmCmp((BYTE *)(EUICC_OpKeyBuf+3),(BYTE *)(dataAddr+dataOff),tmpLen) == EQUAL))
					{
						dataBuf[1] = TRUE;  //不需要重新产生临时密钥
					}
					else
					{
						dataBuf[1] = FALSE; //需要重新产生临时密钥
					}
				}
				else
				{
					dataBuf[1] = FALSE;  //没有缓存临时密钥需要进行比较
				}

			}
			else
			{
				return DownloadErrorCode_undefinedError;
			}
		}
	}
	else
	{
		dataBuf[1] = FALSE;  //需要重新产生临时密钥
	}

	return RESPONSE_NoError;
}

/*
×@function   : AuthenPrepareDownload_VerifyDpSignature
*
*@see        :5.7.13  Function (ES10b): AuthenticateServer 验证serverSignature1签名/ 5.7.5  Function (ES10b): PrepareDownload中验证 smdpSignature2
*
*@description: 验证签名值,使用DPpb的公钥和对应的曲线参数
*
*@param      :BYTE *certificateAddr    --- DP证书地址,用于函数内部获取公钥
*@param      :WORD certificateLen      --- DP证书长度,TLV的总长度
*@param      :UWORD signatureBufAddr   --- 待签名的数据，存放在RAM中
*@param      :BYTE signatureBufLen     --- 待签名的数据长度
×@param      :UWORD signatureValueAddr --- 待验证的签名值
*@param      :BYTE signatureValueLen   --- 待验证的签名值长度
*@param      :BYTE mode :RSPROLE_DP_AUTH/RSPROLE_DP_PB
×
×@return     :TRUE:签名值验证成功/FALSE: 签名值验证失败
*
*@author     :xuyz
*/
BYTE AuthenPrepareDownload_VerifyDpSignature(BYTE *certificateAddr,WORD certificateLen, UWORD signatureBufAddr ,BYTE signatureBufLen,UWORD signatureValueAddr, BYTE signatureValueLen , BYTE mode)
{
	BYTE idata result;
	
	if(signatureValueLen != 64)
	{
		return ErrorReason_invalidSignature;
	}
	
	VMReadBuf(RecData,signatureBufAddr,  signatureBufLen); 
	
	if(mode == RSPROLE_DP_PB)  //需要拼装euiccSignature1.
	{	
		VMReadBuf(&RecData[signatureBufLen],EUICC_AuthenTmpBuf + TempeuiccSignature1_Off,  67);
		
		signatureBufLen += 67;
	}
	
//	SHA256DoFinalSingleCall_Ex((UWORD)&RecData[0],  signatureBufLen ,  &tempBuf[0]); //tempBuf[0-32] 存储  sha256签名
	
	TBSCertificate_GetField(certificateAddr, certificateLen, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey, &tempBuf[64]);  //提取证书里面的公钥值, tempBuf[64]是LV结构, 前面已经判断了证书的合法性，所以这里不需要再对返回值进行判断
		
	TBSCertificate_GetField(certificateAddr, certificateLen, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters,&tempBuf[33]);   //提取证书里面的公钥曲线参数, 前面已经判断了证书的合法性，所以这里不需要再对返回值进行判断

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		result = VerifySignForSM2(&tempBuf[65], RecData, signatureBufLen, (BYTE *)signatureValueAddr);
	}
	else
#endif
	{
		result = VerifySignForECDSA(&tempBuf[65], 64, tempBuf[34], RecData, signatureBufLen ,(BYTE *)signatureValueAddr, (BYTE *)(signatureValueAddr+0x20), 64, ALG_ECDSA_SHA_256);
	}
	
	if(result == TRUE)
	{
		return RESPONSE_NoError;
	}
	else
	{
		return ErrorReason_invalidSignature;
	}
}
/*
*@function   : InitialiseSecureChannel_VerifySmdpSign
*
*@descruotion: InitialiseSecureChannelRequest命令中验证DP的签名值
*
*@param       :UWORD dataStartAddr:数据的起始偏移，InitialiseSecureChannelRequest中BF23的值域的起始地址，目前上层传入的为&RecData[xx]
*@param       :WORD smdpSignOff: smdpSign字段在dataStartAddr中的偏移位置，指向 tag '5F37', 同时也是remoteOpId + transactionId + controlRefTemplate + smdpOtpk的总长度
*
*@return     : RESPONSE_NoError/ otherError
*
*@RAM         :tempBuf[0..31]存储sha256签名, tempBuf[32..95]存储待验证的签名值
*
*@author     :xuyz
*
*@history    :V1.0 create @20181205
*注意         :由于该函数的调用者在调用完该函数之后，RecData中数据依然有效，所以该函数中不允许使用RecData作为缓存
*/
BYTE InitialiseSecureChannel_VerifySmdpSign(UWORD dataStartAddr,WORD smdpSignOff)
{
	BYTE idata pkRef,result;
	UWORD idata signLen, tempSignLen;
	BYTE idata pkValue[64], signature[64];
	
	if((smdpSignOff + 4 + 64) > sizeof(tempBuf)) //modify by xuxiaojun@20230306: 先进行越界判断，再拷贝数据，避免数据被冲掉
	{
		return ErrorReason_incorrectInputValues;
	}

	VMReadBuf(tempBuf, dataStartAddr, smdpSignOff);  //读取 remoteOpId transactionId controlRefTemplate smdpOtpk 到tempBuf中
	
	signLen = smdpSignOff;
	
	tempBuf[signLen ++] = 0x5F;
	tempBuf[signLen ++] = 0x49;
	tempBuf[signLen ++] = 0x41;
	tempBuf[signLen ++] = 0x04;  //04表示非压缩编码
	
	VMReadBuf(&tempBuf[signLen],EUICC_OpKeyBuf + EUICC_OpKeyPK_Off,  64);  //读取64字节的临时euiccOtpk
	
	signLen += 64;
	
	tempSignLen = signLen;

//	SHA256DoFinalSingleCall_Ex((UWORD)&tempBuf[0],  signLen ,  &tempBuf[0]); //tempBuf[0..31] 存储  sha256签名
	
	pkRef =  ASN1_GetBERTlvLen(dataStartAddr+smdpSignOff +2, (UWORD*)&signLen);
	
	if(signLen != 64)  //签名值字段必须为64字节
	{
		return ErrorReason_incorrectInputValues;
	}
	
	VMReadBuf(signature, dataStartAddr + smdpSignOff + 2 + pkRef, 64);  //读取待验证的签名值到signature[0...63]中
	
	pkRef = VMReadByte(EUICC_AuthenTmpBuf + TempPKDPpb_Off);  //第一字节存储 pkref ，即公钥的曲线参数
	
	VMReadBuf(pkValue, EUICC_AuthenTmpBuf + TempPKDPpb_Off +1, 64);  //读取 PK.DPbp.ECDSA 到缓存 pkValue[0]开始的位置

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		result = VerifySignForSM2(pkValue, &tempBuf[0], tempSignLen, signature);
	}
	else
#endif
	{
		result = VerifySignForECDSA(pkValue, 64, pkRef, &tempBuf[0], tempSignLen ,signature, signature+32, 64, ALG_ECDSA_SHA_256);
	}
	
	if(result == TRUE)
	{
		return RESPONSE_NoError;
	}
	else
	{
		return ErrorReason_invalidSignature;
	}
}
/*
*@function    : InitialiseSecureChannel_CalShS
*
*@description : 在 InitialiseSecureChannel 中计算shs ，用于后续提取key
*
*@see         :SGP22_V2.2 Annex G  Key Derivation Process (Normative) 
*
*@param       :UWORD smdpOtpkAddr --- 输入参数 5F49的TAG起始地址，point to the tag of smdpOtpk [APPLICATION 73] OCTET STRING  5F49, 5F49 41 04 V
*@param       :BYTE *outBuf : tempBuf来的: LV结构，存放shs的长度和L， 对于ECC256 而言，长度固定为64字节了
*
*@return      : BYTE errorCode
*
*@author      :xuyz
*
*@history      :V1.0 :create @20180103
*/
BYTE InitialiseSecureChannel_CalShS(UWORD smdpOtpkAddr, BYTE *outBuf)
{
	BYTE idata tmp, pkRef;
	UWORD idata tmpLen;
	
	tmp = ASN1_GetBERTlvLen(smdpOtpkAddr+2, (UWORD*)&tmpLen);
	
	if((tmp != 0x01) || (tmpLen != 65) || (VMReadByte(smdpOtpkAddr + 3) != 0x04))  //仅支持非压缩的公钥，当公钥的第一字节为04时，表示当前是非压缩模式，公钥字节本身是64字节长
	{
		return ErrorReason_incorrectInputValues;
	}
	
	pkRef = VMReadByte(EUICC_OpKeyBuf + EUICC_OpKeyPara_Off);          //获取 otPK.EUICC.ECKA/otSK.EUICC.ECKA 的曲线参数
	
	VMReadBuf(tempBuf, smdpOtpkAddr + 2 + tmp + 1, 64);                //读取64字节的  otPK.DP.ECKA, 1是偏移掉0x04
	
	VMReadBuf(&tempBuf[64], EUICC_OpKeyBuf + EUICC_OpKeySK_Off , 32);  //读取 32 字节的  otSK.EUICC.ECKA
	
	tmp = calcSHS(tempBuf, 64 , pkRef , &tempBuf[64], 32, &tempBuf[128]) ;  
	
	if(tmp == FALSE)
	{
		return ErrorReason_incorrectInputValues;
	}
	
	ecpMem_Move(outBuf, &tempBuf[128], tmp);
	
	return RESPONSE_NoError;
}
/*
*@function   : AuthenticateServer_CheckServerSigned1
*
*@description:在AuthenticateServer中检查字段是否正确，并将对应的数据存储在rsp会话上下文中
*
*@param      :UWORD dataAddr: ServerSigned1 的起始地址，即30开始的位置
*
*@return1     :RESPONSE_NoError/ OTHER ERRORS
*@return2     :serverChallangeBuf存储的是serverChallenge  Octet16 ,即 84 10 xxxxxxxxxx
*
*@author      :xuyz
*
*@history     :
*/
BYTE AuthenticateServer_CheckServerSigned1(UWORD dataAddr, BYTE *serverChallangeBuf)
{
	BYTE idata tmp;
	UWORD idata tmpLen;
	WORD idata offset;
	
	tmp = ASN1_GetBERTlvLen(dataAddr+1, (UWORD*)&tmpLen);  //读取 ServerSigned1 序列中有效数据的总长度
	
	if(tmpLen > sizeof(tempBuf))
	{
		return AuthenticateError_undefinedError;
	}
	
	VMReadBuf(tempBuf, dataAddr+tmp+1, tmpLen);
	
	offset = 0; //point to the  tag of transactionId
	
	if(tempBuf[0] != 0x80)  // transactionId not exist
	{
		return AuthenticateError_undefinedError;
	}
	
	if((tempBuf[1] == 0x00) ||( tempBuf[1] > TransactionIDMaxLen))
	{
		return AuthenticateError_undefinedError;
	}
	
	offset += tempBuf[1] +2 ; //ponit to the tag of euiccChallenge
	
	if(tempBuf[offset] != 0x81)  // euiccChallenge 
	{
		return AuthenticateError_euiccChallengeMismatch;
	}
	
	if(tempBuf[offset +1] != 0x10)
	{
		return AuthenticateError_euiccChallengeMismatch;  // <eSIM_Test_CMCC_additional-LS\CommonMutualAuthenticationTest03.html>
	}	

	if(ecpMem_Compare(&tempBuf[offset+2], &G_RSPSessionContext.channllengeTransactionId[0],0x10) != EQUAL)	//比较euiccChallenge
	{
		return AuthenticateError_euiccChallengeMismatch;
	}
	
	offset += 18 ; //ponit to the tag of serverAddress
	
	if(tempBuf[offset] != 0x83)  
	{
		return AuthenticateError_undefinedError;
	}
	
	if((tempBuf[offset + 1] == 0x00) ||( tempBuf[offset + 1] > ServerAddressMaxLen))  //ServerAddressMaxLen为G_RSPSessionContext.serverAddress 可存储的服务器地址
	{
		return AuthenticateError_undefinedError;
	}
	
	ecpMem_Copy(&G_RSPSessionContext.serverAddress[0], &tempBuf[offset + 1], tempBuf[offset + 1] +1);  //缓存serverAddress 供后续拼装installresult时使用
	
	offset += tempBuf[offset + 1] +2 ; //ponit to the tag of serverChallenge
	
	if(tempBuf[offset] != 0x84)  // serverChallenge 
	{
		return AuthenticateError_undefinedError;  // <eSIM_Test_CMCC_additional-LS\CommonMutualAuthenticationTest03.html>;
	}
	
	if(tempBuf[offset +1] != 0x10)
	{
		return AuthenticateError_undefinedError;  // <eSIM_Test_CMCC_additional-LS\CommonMutualAuthenticationTest03.html>;
	}	
	
	G_RSPSessionContext.transactionIdLen = tempBuf[1];
	
	ecpMem_Copy(&G_RSPSessionContext.channllengeTransactionId[0], &tempBuf[2],tempBuf[1] );  //保存transactionId到rsp上下文中
	
	ecpMem_Copy(serverChallangeBuf, &tempBuf[offset], 0x12);  //拷贝18字节的 serverChallenge 字段到tempBuf中
	
	return RESPONSE_NoError;
}
/*
*@function   :AuthenticateServer_CheckEuiccCiPKIdToBeUsed
*
*@description:在AuthenticateServer中检查字段 EuiccCiPKIdToBeUsed 是否正确
*
*@param      :UWORD dataAddr         --- 输入参数，SubjectKeyIdentifier的起始地址，即04 tag 开始的位置
*@param      :WORD ×UICCCertifiIndex --- 输出参数，返回EuiccCiPKIdToBeUsed所对应的EUICC证书链的起始地址
*
*@return     :EuiccCiPKIdToBeUsed验证通过返回RESPONSE_NoError / 否则，返回OTHER ERRORS
*
*@author     :xuyz
*
*@history    :modify by xuxiaojun@20190726 调整euiccCiPKIdToBeUsed缓存越界的判断位置
*/
BYTE AuthenticateServer_CheckEuiccCiPKIdToBeUsed(UWORD dataAddr, UWORD * UICCCertifiAddr)
{
	BYTE idata tmp;
	WORD idata ref;
	UWORD xdata tmpLen;
	
	tmp = ASN1_GetBERTlvLen(dataAddr + 1, (UWORD*)&tmpLen);  //读取 euiccCiPKIdToBeUsed 序列中有效数据的总长度
	
	if(tmpLen > (TempeuiccSignature1_Off - TempCIPKIdToBeUsed_Off - 1)) //euiccCiPKIdToBeUsed 超过了缓存
	{
		return AuthenticateError_ciPKUnknown;
	}
	
	dataAddr += (tmp + 1);                           //point to the value of CiPKIdToBeUsed
	
	UICCCertifiAddr[0] = Certificate_SearchEUICCCerficateByCIPKID(dataAddr, tmpLen, &ref);
	
	if(UICCCertifiAddr[0] == ADDR_IS_NULL)
	{
		return AuthenticateError_ciPKUnknown;
	}
	
	tempBuf[0] = tmpLen;
	
	VMReadBuf(&tempBuf[1] ,dataAddr,  tmpLen);  //读取euiccCiPKIdToBeUsed 字段	
	
	VMWriteBuf(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off, tempBuf, tmpLen + 1);
		
	return RESPONSE_NoError;
}
/*
*@function   : AuthenticateServerPrepareDownload_SendResponseError
*
*@description: 在AuthenticateServer/PrepareDownload 指令中遇到错误的状态时，返回对应的响应字段
×
×@param      :UWORD dataAddr: point to the offset of  SmdpSigned2 ,即30tag所在的位置
*@param      :BYTE errorCode
*@para,      :WORD startTag
*
*@return     void
*
*@author     :
*
*@history    :modify by xuxiaojun@20190709 代码书写规范
*            :modify by xuxiaojun@20190711  增加transactionId的长度为0时的异常处理
*/
BYTE code transacionIdTag[0x02] = {0x30, 0x80};
void AuthenticateServerPrepareDownload_SendResponseError(UWORD dataAddr, BYTE errorCode,WORD startTag)
{
	BYTE idata i;
	BYTE idata tmp;
	BYTE xdata tlv[0x04];
	UWORD idata tmpLen;
	
	RSP_ClearSession(); //清空当前上下文
	
	for(i =0 ;i<2 ;i ++)
	{
		tmp = VMReadByte(dataAddr);
		
		if(tmp != transacionIdTag[i])  //根本不存在ServerSigned1 or transactionId
		{
			ThrowError(SW_WRONG_DATA); 
		}
		
		tmp = ASN1_GetBERTlvLen(dataAddr + 1, (UWORD*)&tmpLen); 
		
		if(i == 0x00)
		{
			dataAddr += (tmp + 1);  //point to the tag of transactionId 
		}
	}
	
	if((tmpLen > TransactionIDMaxLen)||(tmpLen == 0x00))
	{
		ThrowError(SW_WRONG_DATA); 
	}
	
	VMReadBuf(tempBuf, dataAddr, tmpLen+tmp+1);  //读取 transactionId 到RecData中，TLV结构
	
	tmpLen += (tmp + 1);
	
	tempBuf[tmpLen ++ ] = 0x02; //the tag of authenticateErrorCode
	
	tempBuf[tmpLen ++ ] = 0x01;
	
	tempBuf[tmpLen ++ ] = errorCode;
	
	tmp = ASN1_PaddingBERTlvTagAndLen(tlv, 0xA1, tmpLen);   //拼装choicetag
	
	ecpMem_Move(&tempBuf[tmp],tempBuf,tmpLen);
	
	ecpMem_Copy(tempBuf, tlv, tmp);
	
	tmpLen += tmp; 
	
	tmp = ASN1_PaddingBERTlvTagAndLen(RecData, startTag, tmpLen);
	
	ecpMem_Copy(&RecData[tmp], tempBuf, tmpLen);
	
	tmpLen += tmp;
	
	SetOutgoingAndSend(0x05,(short)tmpLen);   	
}
/*
*@function   :EUICC_SendSingleChannelResponse 
*
*@description: 发送响应数据，不支持多通道交互和机制
×
×@param      :WORD sendLen,数据已存放在RecData中
*
*@return     :void
*
*@author     :
*/
//void EUICC_SendSingleChannelResponse(WORD sendLen)
//{
//	BYTE idata curChannel;
//	WORD idata sw = 0x6100;
//	
//_sendErrorStatus:
//	G_APDULr = sendLen;

//	if(G_APDULr < 256)  //为支持G_APDULr超过256的情况，对于响应数据超过256的情况，发送6100
//	{
//		sw =sw|G_APDULr;
//	}
//	
//	GlobalSetStatus(sw);

//	GlobalSndResponseAndRcvCommand(); //发送61XX并接收下一条指令

//	curChannel = GPGetCurrenteChannel();  //获取当前指令的新的通道信息

//	if(ILLEGAL_CHANNEL ==  curChannel)  //通道非法，重新收取命令
//	{
//		sw = 0x6881;

//		goto _sendErrorStatus;
//	}

//	if(curChannel == G_CurSelectedChannel)  //新收的命令和61XX在同一通道上，如果是C0 则继续往下走；否则的话，则打断当前流程
//	{		
//		if(G_APDU[1] == 0xC0)//如果接收到的是0xC0命令，返回TRUE
//		{
//			if((G_APDU[2] != 0x00) || (G_APDU[3] != 0x00)) //UPCARD测试，根据11.11规范中，GET RESPONSE指令的P1和P2必须为0000
//			{
//				ThrowError(SW_INCORRECT_P1P2);
//			}
//			
//			if(G_APDU[4] == 0) //7816-4: If the Le field contains only bytes set to '00', then all the available bytes should be returned within the limit of 256 for a short Le field, or 65 536 for an extended Le field. 
//			{
//				if(G_APDULr != 256)  //标注 ：虽然按照7816-4 ，当00C000000 的P3为0时，要求返回全部数据 ，但是在实际 测试中发现，如果卡端的实际响应值<256字节 ，这是终端发送00C000000，卡片会死卡，
//				{                    //因此修改成现在的处理方式，当实际响应值不足256字节时，发送00C0000000,返回6CXX，告诉终端实际的响应值.
//					 sw = 0x6C00|G_APDULr;		
//					
//					 goto  _sendErrorStatus;
//				}
//				else
//				{
//					G_APDULe = G_APDULr;
//				}
//			}
//			else
//			{
//				G_APDULe = G_APDU[4];
//		
//				if(G_APDULr < G_APDULe)//C0指令期望返回数据长度不正确，要求重发
//				{				
//					sw = 0x6C00|G_APDULr;				
//					
//					goto  _sendErrorStatus;
//				}
//			}
//		}
//		else
//		{
//			G_NextAPDUFlag = curChannel+1;//在当前通道 上接收到的非C0指令，则打断当前流程

//			ThrowError(APDU_EXCEPTION_NO_T0_GETRESPONSE);
//		}
//	}
//	else
//	{
//		G_NextAPDUFlag = curChannel+1;
//		
//		ThrowError(APDU_EXCEPTION_NO_T0_GETRESPONSE);
//	}
//	
//	if(G_APDULe == 0x100)
//	{
//		P3 = 0;
//	}
//	else
//	{
//		P3 = G_APDULe;
//	}
//	
//	GlobalSendData();
//	
//	G_APDULr -= G_APDULe;
//	
//	if(G_APDULr != 0x00)
//	{
//		goto _sendErrorStatus;
//	}
//}
/*
*@function   : AuthenticateServer_SendResponseSuccess
*
*@description: 发送成功的AuthenticateServer的响应
*
*@param        UWORD ctxParams1Addr
*@param        BYTE ctxParams1Len
*@param        BYTE *serverChallange
*@param        WORD caListAddr
*
*@return       :WORD totalLen 响应值总长度
*
*@author      :xyz
*
*@history     :v1.1 modify by lgy@20190705
*@history     :V1.2 modify by xxj@20230314 增加越界判断
*/
WORD AuthenticateServer_SendResponseSuccess(UWORD ctxParams1Addr, BYTE ctxParams1Len, BYTE *serverChallange, UWORD caListAddr)
{
	BYTE idata level1, level2,tmp, signatureLen;
	BYTE xdata tagLevel1[0x05], tagLevel2[0x04];
	WORD idata offset,euiccCertRef,eumCertRef;
	UWORD xdata CerAddr;
	WORD xdata CerLen[3];
	WORD idata totalLen;
	
	VMReadBuf(RecData, ctxParams1Addr ,ctxParams1Len );  //先将ctxParams1 备份在 RecData中
	
	offset = 0;
	
	tempBuf[offset ++ ] = 0x80;  //TransactionId tag
	
	tempBuf[offset ++ ] = G_RSPSessionContext.transactionIdLen;
	
	ecpMem_Copy(&tempBuf[offset], G_RSPSessionContext.channllengeTransactionId, G_RSPSessionContext.transactionIdLen);  //拼装tranactionId tag
	
	offset += G_RSPSessionContext.transactionIdLen;
	
	tempBuf[offset ++ ] = 0x83;  //serverAddress tag
	
	ecpMem_Copy(&tempBuf[offset], &G_RSPSessionContext.serverAddress[0], G_RSPSessionContext.serverAddress[0] + 1);
	
	offset += G_RSPSessionContext.serverAddress[0] + 1;
	
	ecpMem_Copy(&tempBuf[offset], &serverChallange[0], 0x12);  //拼装serverChallenge字段
	
	offset += 0x12;
	
	VMWriteBuf(EUICCTempDataBuf + 13, tempBuf, offset);   //更新数据到EUICCTempDataBuf中，12是预留出来的字节后续可以使用非原子写的方式进行更新，提高速度，因为此处不需要进行事务保护
	
	// 该函数内部可能会抛异常
	offset += GetEuiccInfo2SubFunc(EUICCTempDataBuf+13+offset, EUICCTempDataBufSize - (13 + offset));//更新euiccInfo2到EUICCTempDataBuf++12+offset的起始位置，并返回总长度
	
	if((13+offset + ctxParams1Len) > EUICCTempDataBufSize)
	{
		return 0x00; //ThrowError(Card_Ability_Exception);
	}

	VMWriteBuf(EUICCTempDataBuf+13+offset, RecData, ctxParams1Len);  //更新ctxParams1Len字段到E2中
	
	offset +=  ctxParams1Len;
	
	tmp = ASN1_PaddingBERTlvTagAndLen(RecData, 0x30,  offset);  //拼装EuiccSigned1的SEQUENCE tag
	
	VMWriteBuf(EUICCTempDataBuf + 13-tmp, RecData, tmp);   //更新大tag到E2中., 30 L V
	
	offset += tmp;  //EuiccSigned1字段的总长度，包括TLV
	
	signatureLen = AuthenticateServer_CalEuiccSignature1(EUICCTempDataBuf + 13 - tmp,offset, caListAddr, RecData);  //计算签名值
	
	VMWriteBuf(EUICCTempDataBuf + 13 - tmp + offset, RecData, signatureLen); 
	
	offset += signatureLen;  //EuiccSigned1 + euiccSignature1 字段的总长度
	
	ecpMem_Set((BYTE *)&CerLen[0] , 0x00, sizeof(CerLen)) ; //清空证书长度字段
	
	CerLen[0] = offset;//EuiccSigned1 + euiccSignature1 字段的总长度
	
	euiccCertRef = VMReadShort(CAEUMAndEUICCListGet_curCertEUICCArrayAddr(caListAddr));

	CerAddr = CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(caListAddr);//GetAddrAndIndexByIndexAddr(caListAddr + CAEUMAndEUICCList_curCertEUICCRef_Offset,&euiccCertRef);
	
	CerLen[1] = VMReadShort(CerAddr);  //获取EUICC证书的长度
	
	eumCertRef = VMReadShort(CAEUMAndEUICCListGet_curCertEUMArrayAddr(caListAddr));

	CerAddr = CAEUMAndEUICCListGet_curCertEUMArrayValueAddr(caListAddr);//GetAddrAndIndexByIndexAddr(caListAddr + CAEUMAndEUICCList_curCertEUMRef_Offset, &eumCertRef);
	
	CerLen[2] = VMReadShort(CerAddr);  //获取EUM证书的长度
	
	totalLen = CerLen[0]  + CerLen[1] + CerLen[2];  //AuthenticateResponseOk序列的总长度
	
	level2 = ASN1_PaddingBERTlvTagAndLen(tagLevel2, 0xA0,  totalLen);  
	
	totalLen  += level2;
	
	level1 = ASN1_PaddingBERTlvTagAndLen(tagLevel1, 0xBF38,  totalLen);  
	
	totalLen += level1;
		
	ecpMem_Copy(RecData, tagLevel1, level1);  //拷贝0xBF38 L
	
	ecpMem_Copy(&RecData[level1], tagLevel2, level2); //拷贝 0xA0 L
	
	VMWriteBuf(EUICCTempDataBuf + 13 - tmp - level1 - level2,RecData,level1 + level2);
	
	ecpMem_Set((BYTE*)&G_SendFileExtendBackDataStruct,0,sizeof(SendFileExtendBackData));
	
	G_SendFileExtendBackDataStruct.backAddr = EUICCTempDataBuf + 13 - tmp - level1 - level2;
	
	G_SendFileExtendBackDataStruct.currentChannel = G_CurSelectedChannel;
	
	G_SendFileExtendBackDataStruct.totalLen = totalLen;
	
	G_SendFileExtendBackDataStruct.len = (totalLen - CerLen[1] - CerLen[2]);
	
	G_SendFileExtendBackDataStruct.euiccCertRef = euiccCertRef;
	
	G_SendFileExtendBackDataStruct.eumCertRef = eumCertRef;
			
	return totalLen;
	
}
/*
*@function   : AuthenticateServer_CalEuiccSignature1
*
*@description: AuthenticateServer计算euiccSignature1的签名，其使用SK.EUICC.ECDSA进行签名运算，同时将签名数据 euiccSignature1 更新到临时缓存 TempeuiccSignature1_Off 中
*
*@param      :UWORD signDataAddr --- 输入参数，待签名的源数据的起始地址，可以是E2逻辑地址，也可以是RAM
*@param      :WORD signDataLen   --- 输入参数，待签名的源数据的长度
*@param      :UWORD euiccListAddr--- 输入参数，pkciused所对应EUICC链表CAEUMAndEUICCList的地址
*@param      :BYTE *sigoutBuf    --- 输出参数，生成的签名数据，包括TLV 5F37 L r s
*
*@return     :WORD signOutLen    --- 签名数据的长度
*
*@author     :xuyz
*
*@history    :V1.0 :该函数使用了tempBuf的资源，支持传入数据sigoutBuf也是 tempBuf
*/
WORD AuthenticateServer_CalEuiccSignature1(UWORD signDataAddr, WORD signDataLen, UWORD euiccListAddr, BYTE *sigoutBuf)
{
	BYTE idata skRef;
	UWORD xdata skEuiccAddr;
	
//	SHA256DoFinalSingleCall_Ex((UWORD)signDataAddr,  signDataLen , &tempBuf[0]); //tempBuf[0-32] 存储  sha256签名
	
	skEuiccAddr = CAEUMAndEUICCListGet_skEUICCArraysValueAddr(euiccListAddr);//euiccListAddr + CAEUMAndEUICCList_skEUICCValueOffset;
	
	skRef = VMReadByte(CAEUMAndEUICCListGet_parameterEUICCAddr(euiccListAddr));//VMReadByte(euiccListAddr+CAEUMAndEUICCList_parameterEUICCRefOffset);  //读取sk的曲线参数引用
	
	VMReadBuf(&tempBuf[32] , skEuiccAddr +1 , 32);  //读取32字节的私钥  SK是LV 结构 共33字节

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		CalSignValueForSM2(&tempBuf[32], (BYTE *)signDataAddr, signDataLen, &tempBuf[128]); //SM2签名
	}
	else
#endif
	{
		CalSignValueForECDSA(&tempBuf[32],32, skRef, (BYTE *)signDataAddr, signDataLen, &tempBuf[128]);  //输出64字节的签名值
	}
	
	ecpMem_Move(&sigoutBuf[3], &tempBuf[128], 64);
	
	sigoutBuf[0] = 0x5F;
	sigoutBuf[1] = 0x37;
	sigoutBuf[2] = 0x40;
	
	VMWriteBuf(EUICC_AuthenTmpBuf + TempeuiccSignature1_Off, sigoutBuf, 67);  //此处只是缓存数据，后续可以考虑将此字段的更新变更成非原子写，前提是位于一段完整页上
	
	return 67;  //返回67字节的签名值
		
}
/*
×@function   : PrepareDownload_GenerateEuiccSigned2
*
*@description: 在 PrepareDownload  指令操作中产生相应数据的EuiccSigned2
*
*@param       :UWORD dataAddr  下行中的数据源 
*@param       :BYTE* OutBuf: 
*@param       :WORD* tagOffset ,输入数据 smdpSigned2 SmdpSigned2,
										 smdpSignature2 [APPLICATION 55] OCTET STRING,
										 hashCc Octet32 OPTIONAL,
										 smdpCertificate Certificate 相对于dataAddr的偏移量
*
*return       :Len 拼装数据的总长度
*
*@author      :lgy
*
*@history     :modify by xuxiaojun@201709 修改BUG，书写规范
*/
BYTE PrepareDownload_GenerateEuiccSigned2(UWORD dataAddr,BYTE* OutBuf,WORD tagOffset[4])
{
	BYTE idata startTmp,dataOff,tempLen;
	UWORD idata totalLen;
	
	startTmp = ASN1_GetBERTlvLen((UWORD)(dataAddr+tagOffset[0]+1), (UWORD*)&totalLen); //获取SmdpSigned2内容的总长度
	
	dataOff = tagOffset[0] + startTmp+1;  //point to transactionId Tag
	
	tempLen = VMReadByte(dataAddr+dataOff+1);// transactionId Len
	
	VMReadBuf(&OutBuf[3],dataAddr+dataOff,tempLen+2);	//拷贝 transactionId  TLV结构
	
	totalLen = tempLen + 2;
	
	OutBuf[3 + totalLen] = 0x5F;
	
	OutBuf[3 + totalLen + 1] = 0x49;
	
	totalLen += 2;
	
	OutBuf[3 + totalLen] = 0x41; 	//euiccOtpk ==  L + 04 + 公钥 
	
	OutBuf[3 + totalLen + 1] = 0x04;
	
	totalLen += 2;//point to key value
	
	VMReadBuf(&OutBuf[3 + totalLen],EUICC_OpKeyBuf + EUICC_OpKeyPK_Off,64);
	
	totalLen += 64;
	
	if(tagOffset[2] != 0)//hashCc
	{
		VMReadBuf(&OutBuf[3 + totalLen],dataAddr+tagOffset[2], PrepareDownload_hashCc_Len + 2);//hashCc Octet32 OPTIONAL  下行有该字段时 响应中拼装该字段，该字段在ccRequiredFlag设置时 必须存在，已检查过合法性
		
		totalLen += 34;
	}
	
	if(totalLen >= 0x80)
	{
		OutBuf[0] = 0x30;
		
		OutBuf[1] = 0x81;
		
		OutBuf[2] = totalLen;
		
		totalLen += 3;
	}
	else
	{
		OutBuf[0] = 0x30;
		
		OutBuf[1] = totalLen;
		
		ecpMem_Move(&OutBuf[2],&OutBuf[3],totalLen);
		
		totalLen += 2;
	}
	
	return totalLen;
	
}
/*
×@function   : PrepareDownload_GenerateEuiccSignature2
*
*@description: 在 PrepareDownload  指令操作中产生相应数据的EuiccSignature2
				euiccSignature2 SHALL be created using the SK.EUICC.ECDSA and verified using the PK.EUICC.ECDSA as described in section 2.6.7.2. 
				euiccSignature2 SHALL apply on the concatenated data objects euiccSigned2 and smdpSignature2.
*
*@param       :BYTE* dataBuf  待签名数据
*@param       :BYTE dataLen   待签名数据长度
*@param       :BYTE* OutBuf:  输出 RecData足够缓存PrepareDownload的结构 这里传入的是RECDATA
*
*return       :Len 拼装数据的总长度 包括tag和签名值
*
*RAM资源      ：借用tempBuf[448]~tempBuf[511]   448--479 用于缓存euicc私钥，480--511用于缓存哈希
*
*@author      :lgy @20190106
*
*@history     ：V1.0
*/ 
BYTE PrepareDownload_GenerateEuiccSignature2(BYTE* dataBuf,BYTE dataLen ,BYTE* OutBuf)
{
	UWORD xdata euiccCertAddr;
	WORD idata ref;
	
	euiccCertAddr = Certificate_SearchEUICCCerficateByCIPKID(EUICC_AuthenTmpBuf+TempCIPKIdToBeUsed_Off + 1, VMReadByte(EUICC_AuthenTmpBuf+TempCIPKIdToBeUsed_Off), &ref);	//	前面流程已经验证过 这里一定可以找到EUICC证书
	
//	SHA256DoFinalSingleCall_Ex((UWORD)dataBuf,  dataLen ,  &RecData[0]); //RecData[0] - 32用来缓存哈希值
	
	VMReadBuf(&RecData[32],CAEUMAndEUICCListGet_skEUICCArraysValueAddr(euiccCertAddr) + 1,32);//VMReadBuf(&RecData[32],euiccCertAddr + CAEUMAndEUICCList_skEUICCValueOffset + 1,32);//借用RecData 在该业务流程 此处RecData数据已经不需要 可以用来做缓存

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		CalSignValueForSM2(&RecData[32], dataBuf, dataLen, &OutBuf[3]); //SM2签名
	}
	else
#endif
	{
		//if(CalSignValueForECDSA(&RecData[32], 32, VMReadByte(euiccCertAddr + CAEUMAndEUICCList_parameterEUICCRefOffset),&RecData[0],32,&OutBuf[3]) == FALSE)
		if(CalSignValueForECDSA(&RecData[32], 32, VMReadByte(CAEUMAndEUICCListGet_parameterEUICCAddr(euiccCertAddr)), dataBuf, dataLen, &OutBuf[3]) == FALSE)
		{
			return 0;
		}
	}
	
	OutBuf[0] = 0x5F;
	OutBuf[1] = 0x37;
	OutBuf[2] = 0x40;
	
	return 0x43;
	
}
/*
×@function   :GetEuiccInfo2SubFunc
*
*@description:获取EUICC INFO2的信息 

*@param       :BYTE* outBuf  输出数据
*@param       :freeLen --- OutBuf中可用字节数
*
*return       :outBuf中有效数据的长度 
*
*RAM资源      ：
*
*@history     :V1.0 created by lgy @20190108
*             :V1.1 modify by xuxiaojun@20190614 优化代码，去掉局部变量euiccInfo_section1, 由个人化保证EUICCINFO2_SECTION1必须写入
*             :V1.2 modify by chentao@20200811 修改支持Category不配置，Category是 optional
*             :V1.3 modify by xuxiaojun@20230314 增加越界判断
*/ 
WORD GetEuiccInfo2SubFunc(UWORD OutBuf, WORD freeLen)
{
	WORD idata dataLen,PKIDForVerificationLen,PKIDForSigningLen;
	WORD idata WriteOffset,WriteLen,nextOffsetStart;
	BYTE xdata tmp[4];
	BYTE idata octerNum, categoryLen, section3Len;
	UWORD idata addrSection1, addrSection2,addrCategory, addrSection3;
	
	dataLen = 0;
	WriteOffset = 0x05;//预留的BF22 82XXXX
	WriteLen = 0;      //已写入E2的长度
	
	addrSection1 = EUICCINFO2_SECTION1;
	addrSection2 = EUICCINFO2_SECTION2;
	addrSection3 = EUICCINFO2_SECTION3;
	addrCategory = EUICCINFO2_CATEGORY;

	/*if((addrSection1 == ADDR_IS_NULL)||(addrSection2 == ADDR_IS_NULL)||(addrSection3 == ADDR_IS_NULL)||(VMReadByte(addrCategory)!= 0x03))
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);  //没有执行个人化指令
	}*/

	if((addrSection1 == ADDR_IS_NULL)||(addrSection2 == ADDR_IS_NULL)||(addrSection3 == ADDR_IS_NULL))//euiccCategory OPTIONAL
	{
		ThrowError(SW_CONDITIONS_NOT_SATISFIED);  //没有执行个人化指令
	}

	VMReadBuf(&tempBuf[0], addrSection1 + 1, EUICCINFO2_SECTION1_LEN); // tempBuf[0...14]
	
	dataLen = EUICCINFO2_SECTION1_LEN;
				
	dataLen += GetEuiccInfo_ExtCardResource(&tempBuf[dataLen]);   // tempBuf[15...29]
	
	octerNum = VMReadByte(addrSection2);
	
	VMReadBuf(&tempBuf[dataLen], addrSection2 + 1, octerNum); // tempBuf[30...48]
	
	dataLen += octerNum;
		
	//该函数内部对tempBuf越界进行了判断，越界时返回已经获取到的数据
	PKIDForVerificationLen = GetEuiccCiPKIdList(&tempBuf[dataLen], TEMP_BUF_SIZE - dataLen, CIPKID_ForVerification);// A9 XX euiccCiPKIdListForVerification [9] SEQUENCE OF SubjectKeyIdentifier  ,for verification

	dataLen += PKIDForVerificationLen;
	
	if(dataLen >= 0x100)//总长度大于2字节  tag BF22 + LEN 为5字节
	{
		if((WriteOffset + dataLen) > freeLen)// 这里的OutBuf传入的为EUICCTempDataBuf，走到这里越界的可能性几乎不存在，如果程序异常走到了这里的话，直接抛异常
		{
			ThrowError(Card_Ability_Exception);
		}

		VMWriteBuf(OutBuf + WriteOffset,&tempBuf[0],dataLen);//从OutBuf偏移为5处开始写数据 后续不会造成E2移动
		
		WriteOffset += dataLen;
		
		WriteLen = dataLen;
		
		nextOffsetStart = 0;// 下一次tempBuf从偏移0开始使用

	}
	else
	{
		nextOffsetStart = dataLen;		//数据接着当前偏移继续拼装
	}

	//该函数内部对tempBuf越界进行了判断，越界时返回已经获取到的数据
	PKIDForSigningLen = GetEuiccCiPKIdList(&tempBuf[nextOffsetStart], TEMP_BUF_SIZE - nextOffsetStart, CIPKID_ForSignature);// AA xx  euiccCiPKIdListForSigning [10] SEQUENCE OF SubjectKeyIdentifier   , for signature creation
	
	dataLen += PKIDForSigningLen;
	
	octerNum = GetForbiddenProfilePolicyRules(tmp);//后续数据还有ForbiddenPPR ，euiccInfo_euiccCategory ，euiccInfo_section3
	
	//categoryLen = 0x03;//VMReadByte(addrCategory);
	categoryLen = VMReadByte(addrCategory);
	
	section3Len = VMReadByte(addrSection3);
	
	if((dataLen + octerNum + categoryLen + section3Len) >= 0x100)//此种情况 可能前面已经写过一次数据到OutBuf
	{
		if((WriteOffset + dataLen - WriteLen) > freeLen)
		{
			ThrowError(Card_Ability_Exception);
		}

		VMWriteBuf(OutBuf + WriteOffset,&tempBuf[0],dataLen - WriteLen);//避免tempBuf溢出 先将数据写一部分进去，WriteOffset可能是5 可能是上一次写到的末地址
		
		WriteOffset += (dataLen - WriteLen); 
		
		VMReadBuf(&tempBuf[0], addrCategory + 1, categoryLen);//数据写入E2后可以从tempBuf[0]开始缓存category + section3数据是为了减少写次数，提高写速度，但是可能也限制了能力
		
		ecpMem_Copy(&tempBuf[categoryLen], tmp, octerNum);
		
		if((categoryLen + octerNum + section3Len) > sizeof(tempBuf))
		{
			ThrowError(Card_Ability_Exception); //这里抛异常状态继续保持
		}

		VMReadBuf(&tempBuf[categoryLen + octerNum], addrSection3 + 1, section3Len);

		if((WriteOffset + categoryLen + octerNum + section3Len) > freeLen)
		{
			ThrowError(Card_Ability_Exception);
		}

		VMWriteBuf(OutBuf + WriteOffset,tempBuf,categoryLen + octerNum + section3Len);
		
		dataLen += (octerNum + categoryLen + section3Len);//数据总长度
				
	}
	else//此种情况 还没有数据写到OutBuf  ，直接在tempBuf后面拼装数据
	{
		if((nextOffsetStart + PKIDForSigningLen + categoryLen + octerNum + section3Len) > sizeof(tempBuf))
		{
			ThrowError(Card_Ability_Exception); //这里抛异常状态继续保持
		}

		VMReadBuf(&tempBuf[nextOffsetStart + PKIDForSigningLen], addrCategory + 1, categoryLen);
		
		ecpMem_Copy(&tempBuf[nextOffsetStart + PKIDForSigningLen + categoryLen], tmp, octerNum);
		
		VMReadBuf(&tempBuf[nextOffsetStart + PKIDForSigningLen + categoryLen + octerNum], addrSection3 + 1, section3Len);
		
		dataLen += (octerNum + categoryLen + section3Len);
		
		if(dataLen > 0x7F)
		{			
			VMWriteBuf(OutBuf + 4,&tempBuf[0],dataLen);//BF22 81 L + DATA
		}
		else
		{			
			VMWriteBuf(OutBuf + 3,&tempBuf[0],dataLen);//BF22 + L + DATA
		}
	
	}

	octerNum = ASN1_PaddingBERTlvTagAndLen(&tempBuf[0],0xBF22,dataLen);
	
	VMWriteBuf(OutBuf ,tempBuf,octerNum);
	
	return (dataLen + octerNum);
}
/*
*@fuction    :SetDefaultDpAddress_MemoryReset
*
*@description:在命令eUICCMemoryReset SetDefaultDpAddress中设置DP 的缺省地址
*
*@param      :BYTE *dpAddress  : V结构，可能在RAM中，也可能在E2
*@param      :BYTE dpAddressLen: dpAddress中有效数据长度
*
*@return     :RESULT_OK_/others
*
*@author     :xuyz
*
*@history    :V1.0 :create @20190109
*            :V1.1 :modify by xuxiaojun@20190704 增加参数用于表示dpAddress中的有效数据长度
*            :V1.2 :modify by xuxiaojun@20230311 为了减少空间分配次数，同时由于Default_DP_Address_MAX_LEN有定义，所以在一开始就按最大地址来分配空间，后续直接往里面写值
*/
BYTE SetDefaultDpAddress_MemoryReset(BYTE *dpAddress, BYTE dpAddressLen)
{
	WORD idata defaultDpAddrRef;
	BYTE idata result,transactionTag;
	UWORD xdata defaultDpAddr;
	
	result = RESULT_OK_;
	
	defaultDpAddr = GetAddrAndIndexByIndexAddr(EUICCSMDPDefaultAddress_Ref_Addr,&defaultDpAddrRef);
	
	if(defaultDpAddr != ADDR_IS_NULL)
	{
		if(dpAddressLen != 0x00)	//	需要更新  If the provided UTF8 string is not empty, it SHALL constitute the new default SM-DP+ address.
		{						
			tempBuf[0] = dpAddressLen;

			VMReadBuf(&tempBuf[1], (UWORD)dpAddress, dpAddressLen); // 当dpAddressLen大于0xFB的时候，dpAddress是E2，不能直接调用VMWriteBuf写

			VMWriteBuf(get_array_data(MEMtPF8(defaultDpAddr)), tempBuf, dpAddressLen + 1);  //更新LV      //VMWriteBuf(defaultDpAddr + 1, tempBuf, dpAddressLen + 1);
		}
		else	//需要清空内容  If the provided UTF8 string is empty, an existing default SM-DP+ address SHALL be removed.
		{
			VMSetBuf(get_array_data(MEMtPF8(defaultDpAddr)), 0, 1); //清掉L
		}
	}
	else
	{
		return RESULT_UNDEFINED_ERROR;
	}
	
	return result;
}
/*
*@fuction    :CancelSessionSendErrorResult
*
*@description:在命令CancelSession中发送错误状态字
*
*@param      : BYTE errorCode
*
*@return     :NULL
*
*@author     :lgy
*
*@history    :V1.0 @20190111
*/
void CancelSessionSendErrorResult(BYTE errorCode)
{
	RecData[0] = 0xBF;

	RecData[1] = 0x41;
	
	RecData[2] = 0x03;
	
	RecData[3] = 0x81;
	
	RecData[4] = 0x01;
	
	RecData[5] = errorCode;
	
	SetOutgoingAndSend(0x05,6); 
}
/*
*@fuction    :PaddingCancelSessionResponse
*
*@description:拼装CancelSession的响应值
*
*@param      : BYTE errorCode
*
*@return     :NULL
*
*@author     :lgy
*
*@history    :V1.0 @20190111
*/
BYTE PaddingCancelSessionResponse(BYTE cancelSessionReason)
{
	BYTE idata offset,tempLen;
	WORD idata ref;
	UWORD xdata caAddr;
	
	offset = 0; //进入指令的处理流程，即解析 CancelSessionRequest ，拼装 CancelSessionResponse 	
	
	RecData[0] = 0xBF;
	
	RecData[1] = 0x41;
	
	RecData[3] = 0xA0; //ResponseOK tag
	
	RecData[5] = 0x30; //EuiccCancelSessionSigned tag
	
	RecData[7] = 0x80; //transactionId tag
	
	RecData[8] = G_RSPSessionContext.transactionIdLen;
	
	offset=9;
	
	ecpMem_Copy(&RecData[offset], &G_RSPSessionContext.channllengeTransactionId[0], G_RSPSessionContext.transactionIdLen);
		
	offset += G_RSPSessionContext.transactionIdLen;
	
	RecData[offset++] = 0x81;//smdpOid tag
	
	VMReadBuf(&RecData[offset], EUICC_AuthenTmpBuf+TempSIMOID_Off, VMReadByte(EUICC_AuthenTmpBuf+TempSIMOID_Off)+1);
	
	offset += (RecData[offset]+1);
	
	RecData[offset++] = 0x82;//CancelSessionReason tag
	
	RecData[offset++] = 0x01;
	
	RecData[offset++] = cancelSessionReason;
		
	RecData[6] = offset-7; //EuiccCancelSessionSigned 长度
			
//	SHA256DoFinalSingleCall_Ex((UWORD)&RecData[5], RecData[6]+2, &tempBuf[0]); //tempBuf[0...31]存放EuiccCancelSessionSigned数据的hash值
	
	caAddr = Certificate_SearchEUICCCerficateByCIPKID(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off + 1, VMReadByte(EUICC_AuthenTmpBuf + TempCIPKIdToBeUsed_Off), &ref); //通过CI PKID 查找属于该ci链上的EUICC 证书是否存在，并返回证书链的地址
	
	if(caAddr == ADDR_IS_NULL)
	{
                //CI PKID 对应ci链上的EUICC 证书不存在，抛错		
		return FALSE;
	}
	
	VMReadBuf(&tempBuf[32], CAEUMAndEUICCListGet_skEUICCArraysValueAddr(caAddr)+1, 32); //EUICC证书的SK//VMReadBuf(&tempBuf[32], caAddr+CAEUMAndEUICCList_skEUICCValueOffset+1, 32); //EUICC证书的SK

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		tempLen = CalSignValueForSM2(&tempBuf[32], &RecData[5], RecData[6]+2, &tempBuf[64]); //SM2签名
	}
	else
#endif
	{
		tempLen = CalSignValueForECDSA(&tempBuf[32], 32, VMReadByte(CAEUMAndEUICCListGet_parameterEUICCAddr(caAddr)), &RecData[5], RecData[6]+2, &tempBuf[64]); //签名//CalSignValueForECDSA(&tempBuf[32], 32, VMReadByte(caAddr+CAEUMAndEUICCList_parameterEUICCRefOffset), &tempBuf[0], 32, &tempBuf[64]); //签名
	}
	
	RecData[offset++] = 0x5F;
	
	RecData[offset++] = 0x37;
	
	RecData[offset++] = 0x40;
	
	ecpMem_Copy(&RecData[offset], &tempBuf[64], tempLen);
	
	offset += tempLen;
	
	RecData[2] = offset-3; //BF41总长度
	
	RecData[4] = offset-5; //ResponseOK长度
	
	return offset;
}

/*
*@fuction    :ResetTagList
*
*@description:GetProfilesInfo 子函数，需要根据ProfileInfo ::=结构顺序拼装响应数据
*
*@param      : BYTE* tagList   LV结构，需要调整顺序的源数据
*@param      : BYTE *FormatTaglist  参考模板，tagList根据这个模板调整顺序
*
*@return     :NULL  调整顺序后tagList里面的tag长度可能会变，该函数自动过滤了FormatTaglist中不存在的tag
*
*@author     :lgy
*
*@history    :V1.0 @20190117
*/
void ResetTagList(BYTE* tagList,BYTE *FormatTaglist)
{
	BYTE xdata tempList[0x10]; //规范中列出的tag只有14个 这里只排列14个tag  //modify by xieyong@20221202 增加BF22tag,数组增大空间
	BYTE idata i,j,k,m;
	BYTE xdata unspecifiedTag[UNSPECIFIED_TAG_NUMBER*2];
	
	k = 0;
	ecpMem_Set(tempList,0,sizeof(tempList));
	ecpMem_Set(unspecifiedTag,0,sizeof(unspecifiedTag));
	
	for(i = 0; i < FormatTaglist[0];i++)//该循环需要遍历完 FormatTaglist是参考模板
	{
		for(j = 0; j < tagList[0]; )
		{
			if(tagList[j+1] == FormatTaglist[i+1])
			{
				if(FormatTaglist[i+1] == 0x9F)
				{
					if(tagList[j+2] == 0x70)
					{
						tempList[k++] = 0x9F;
						tempList[k++] = 0x70;
						i++; 		      // j不需要加 如果需要比较的tag是9F70 本次流程结束 继续从模板找下一个tag		
						tagList[j+1] = 0; //把已经找到的模板中的tag清掉 剩下不是规范中定义的tag
						tagList[j+2] = 0;
						break;
					}
					else //9FXX 除了9F70以为的值 
					{
						j += 2;; //for(j = 0; j < tagList[0]; j++)
					}
					
				}
#if RSP_SGP22_BF22
				else if(FormatTaglist[i+1] == 0xBF)
				{
					if(tagList[j+2] == 0x22)
					{
						tempList[k++] = 0xBF;
						tempList[k++] = 0x22;
						i++; 		      // j不需要加 如果需要比较的tag是9F70 本次流程结束 继续从模板找下一个tag
						tagList[j+1] = 0; //把已经找到的模板中的tag清掉 剩下不是规范中定义的tag
						tagList[j+2] = 0;
						break;
					}
					else //
					{
						j += 2;; //for(j = 0; j < tagList[0]; j++)
					}

				}
#endif
				else
				{
					tempList[k++] = FormatTaglist[i+1];
					tagList[j+1] = 0; //把已经找到的模板中的tag清掉 剩下不是规范中定义的tag
					break;  //找到tag 跳出循环继续下一个 不一定遍历完tagList[]
				}
			}
			else
			{
				if((tagList[j+1]&0x1F) == 0x1F)//双字节tag需要偏移2字节
				{
					j += 2; 
				}
				else
				{
					j += 1;
				}
			}

		}
	}
	
	m = 0x00;
	
	for(j = 0; j < tagList[0]; j++)
	{
		if((tagList[j+1] != 0x00)&&(m < sizeof(unspecifiedTag)))
		{
			unspecifiedTag[m++] = tagList[j+1];
		}
	}	
	
	ecpMem_Copy(&tagList[1],&tempList[0],k);
	
	ecpMem_Copy(&tagList[1+k], &unspecifiedTag[0], m);
	
	tagList[0] = (k+m);
}
/*
*@fuction    :CloseChannelAfterProfileChange
*
*@description:Close all logical channels which still have an application of the currently enabled Profile selected, 未考虑SE应用
				当profile发生切换时（激活 失效），profile内部的APP如果有被某个通道选中 需要关闭该通道 ，当前执行指令的通道选中的是ISD-R 该通道不会被关闭，基本逻辑通道也不能关闭 
*
*@param      :refreshFlag   -- false  不带refreshFlag的情况需要关闭的是选中当前激活profile的应用的通道
*@param                     --true  带refreshFlag需要关闭所有的非0通道
*
*@return     :NULL
*
*@author     :lgy
*
*@history    :V1.0 @20190710
*            :V1.1 modify by weipingjiao@20210419 EPX产业化\INFENION_SLM97\Develop
*            :V1.2 modify by liguiyu @20220627 BRANCH_MODULE/RSP_hongcha_STM_ST33G1M2  eSIM_Test_additional/方案评审补充测试/MNO_SD_Privilege_Test02卡片只有4个通道时（只有ISO，没有SPI和SWP等）
*			 :V1.3 chentao@20231110 增加参数refreshFlag
*
*
*/
void CloseChannelAfterProfileChange(BYTE refreshFlag)
{

	u8		D_A		i_iter;
	u8      D_A		appID;
	u16     D_A		backChannelContext;
	u8      D_A		channelProfileIndex;

	for (i_iter = 0; i_iter < MAX_LOGICAL_CHANNELS; U8_INC(i_iter))
	{
		backChannelContext = getChannelContext(i_iter);
		appID = U8_AND_U8(U16t8(backChannelContext), 0xFF);

		channelProfileIndex = getChannelProfileIndex(i_iter);
		if(i_iter == 0)
		{
			if(appID == UICC_APPID) // UICC close and open
			{
				setChannelContext(i_iter, NULL_CONTEXT, FALSE);
				setChannelContext(i_iter, backChannelContext, FALSE);
	//			setChannelProfileIndex(i_iter, G_CurExecutingProfileIndex); // TY_OfflineProfileFormatTest/App_load_N01
			}
		}
		else
		{
			if(refreshFlag) //modify by chentao@20231107  北京ifree测试激活profile后没有关闭通道，期望卡片关
			{
				setChannelContext(i_iter, NULL_CONTEXT, TRUE);//放在最后的话，PX_W_U16(&channelInfo.selected_context[U8_M_U8(i_iter,2)],NULL_CONTEXT);将通道上下文设成了NULL，调用该函数时无法将dtr释放 后续在通道上选中应用时，会导致dtr的spaceID不够       eSIM_Test_additional/方案评审补充测试/MNO_SD_Privilege_Test02卡片只有4个通道时（只有ISO，没有SPI和SWP等）会测出问题，如果卡片有5个以上通道可能会规避问题导致测不出来
				PX_W_U16(&channelInfo.selected_context[U8_M_U8(i_iter,2)],NULL_CONTEXT);
				setSecurityLevel(i_iter ,SCF_NO_AUTHENTICATED);
				channelInfo.compulsory_Session_Security_Level[i_iter]= SCF_NO_AUTHENTICATED;
				channelInfo.channel_status[i_iter]		= CHANNEL_CLOSED;
				channelInfo.sercrity_channel_number[i_iter] = 0x00;
			}
			else
			{
				if(channelProfileIndex != ILLEGAL_PROFILEINDEX)
				{
					setChannelContext(i_iter, NULL_CONTEXT, TRUE);//放在最后的话，PX_W_U16(&channelInfo.selected_context[U8_M_U8(i_iter,2)],NULL_CONTEXT);将通道上下文设成了NULL，调用该函数时无法将dtr释放 后续在通道上选中应用时，会导致dtr的spaceID不够       eSIM_Test_additional/方案评审补充测试/MNO_SD_Privilege_Test02卡片只有4个通道时（只有ISO，没有SPI和SWP等）会测出问题，如果卡片有5个以上通道可能会规避问题导致测不出来
					PX_W_U16(&channelInfo.selected_context[U8_M_U8(i_iter,2)],NULL_CONTEXT);
					setSecurityLevel(i_iter ,SCF_NO_AUTHENTICATED);
					channelInfo.compulsory_Session_Security_Level[i_iter]= SCF_NO_AUTHENTICATED;
					channelInfo.channel_status[i_iter]		= CHANNEL_CLOSED;
					channelInfo.sercrity_channel_number[i_iter] = 0x00;
				}
			}
		}
	}

	// make sure channel 0 is open
	channelInfo.channel_status[0]				= CHANNEL_OPEN;
#if JCVM__IO__SWP
	channelInfo.channel_status[NFC_BASIC_CHANNEL_NUM] = CHANNEL_OPEN;
#endif	

#if JCVM__IO__SPI
	channelInfo.channel_status[SPI_BASIC_CHANNEL_NUM] = CHANNEL_OPEN;
#endif	

}
/*
*@function   :SetSendFileExtendInfo

*@description:设置需要发送的数据相关信息，由于该函数主要用于实现ISD-R发响应长度超过256字节，且被其他通道指令打断后仍可以取响应数据
				这里只设置取响应需要备份的一些信息，并不会马上发送数据（ISD-R的指令都是有输入的，只能先发61XX），发送数据的流程是在ISD-R的C0指令完成的
				对于ISD-R的响应 有4种情况：
				1 指令处理过程发生错误时，响应不超过256字节 直接从RecData发送
				2 响应可能超过256字节 将响应拼装到EuiccTempBuf中发送
				3 鉴权服务器的时候，一部分数据拼装在EuiccTempBuf中中，另外两部分数据分别是EUICC证书和EUM证书
				4 获取通知列表时，从通知存储区域发送，其中通知存储区又包括特定的E2区域和EUICC证书 EUM证书3部分，此时需要发送的通知可能是多条

				该函数处理的是第2种情况
*
×@param       :UWORD bufferAddr --- 待发送数据的EEPROM地址
*@param       :WORD len         --- 待发送数据长度
*
*@return       :void
*
*@author       :lgy
*
*@history      :V1.0 create by lgy@20190703
*/
void SetSendFileExtendInfo(UWORD bufferAddr, WORD len)
{
	ecpMem_Set((BYTE*)&G_SendFileExtendBackDataStruct,0,sizeof(SendFileExtendBackData));
	
	G_SendFileExtendBackDataStruct.backAddr = bufferAddr;
	
	G_SendFileExtendBackDataStruct.currentChannel = G_CurSelectedChannel;
	
	G_SendFileExtendBackDataStruct.totalLen = len;
	
	G_SendFileExtendBackDataStruct.len = len;			
	
	if(len > 0x100)
	{
		ThrowError(0x6100);
	}
	else
	{
		ThrowError(0x6100|len);
	}
}
/*
*@function     : IF_ClearSendFileExtendInfo

*@description  :在收到一条新的APDU以后 根据当前收到指令的通道决定是否需要清除已经设置好ISD-R响应数据信息
				如果在ISD-R取响应的通道收到一条非C0指令，取响应过程被打断 清除相关信息
*
*@param        :void
*@param        :void
*
*@return       :void
*
*@author       :lgy
*
*@history      :V1.0 create by lgy@20190703
*/
void IF_ClearSendFileExtendInfo(void)
{
	BYTE idata curChannel;
	
	curChannel = GPGetCurrenteChannel();  //获取当前指令的新的通道信息
	
	if((curChannel == G_SendFileExtendBackDataStruct.currentChannel)&&(G_APDU[1] != 0xC0))
	{
		ecpMem_Set((BYTE*)&G_SendFileExtendBackDataStruct,0,sizeof(SendFileExtendBackData));
		
		G_SendFileExtendBackDataStruct.currentChannel = ILLEGAL_CHANNEL;
	}
}
/*
*@function     :RSP_ClearSession
*@description  :清除G_RSPSessionContext, 该函数中不允许执行E2操作
*@param        :void
*@return       :void
*@history      :V1.0 create by xuxiaojun@20230317
*              :V1.1 modify by chentao@20240102 30.eSIM_plan_admend_test\用例名称:  eSIM_plan_admend_test_17_01  profile下载过程中86没有发送完LPA关闭通道，需要回收空间
*              :V1.2 modify by xxj@20240128 v1.1的修改放在这里的时候会导致以下问题：1.如果当前下载过程未结束时终端复位会导致垃圾回收时虚拟机还没有初始化，死卡； 2.调用RSP_ClearSession之前以及删除了当前正在下载的profile，再次调用出现无效操作
*/
void RSP_ClearSession(void)
{
	if(G_RSPSessionContext.downloadProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		if(g_moreCommands == CHAINING_STOREDATA)
		{
			g_moreCommands = CHAINING_CLEAR;
		}
	}
	ecpMem_Set((BYTE*)&G_RSPSessionContext, 0x00, sizeof(G_RSPSessionContext));

	G_RSPSessionContext.downloadProfileIndex = ILLEGAL_PROFILEINDEX;

	G_RSPSessionContext.channelNum = 0xFF;
}
/*
*@function     :RSP_abortLoadingSession
*@description  :清除当前正在下载的profile，以及G_RSPSessionContext
*@param        :void
*@return       :void
*@history      :V1.0 create by xuxiaojun@20230314
*/
void RSP_abortLoadingSession(void)
{
	if(G_RSPSessionContext.sessionSate != RSPSessionState_NULL)   //The LPA SHOULD NOT initiate a new Profile download and installation procedure while there is an active download RSP session, IF
	{
		//  A.级联块号接不上情况，本条指令直接报错，下载终止，状态数据清除。
		if(G_RSPSessionContext.sessionSate >= RSPSessionState_ConfigureISDP)  //当前已经在创建ISD-P 设置参数等操作，需要回滚这些操作
		{
			if(PN_R_U8(PM_g_pkgIdInProcess_JC) != ILLEGAL_ID)//如果在profile内部下载cap包 先将正在下载的包的数据清掉
			{
				GPLoadInit();//需要调restore回收正在下载的包？
			}

			DeleteProfileDataByProfileIndex(G_RSPSessionContext.downloadProfileIndex, MODE_DELETE_PROFILE);  //回滚ISD-P
		}
	}

	RSP_ClearSession();
}
/*
*@function     :RSP_abortLoadingSessionThrowError
*@description  :清除当前正在下载的profile，以及G_RSPSessionContext，并抛异常
*@param        :u16 error --- 异常状态字
*@return       :void
*@history      :V1.0 create by xuxiaojun@20230314
*/
void RSP_abortLoadingSessionThrowError(u16 error)
{
	RSP_abortLoadingSession();

	throw_error(error);
}
/*
*@function     :RSP_InitGobalVariable
*@description  :初始化RSP相关的全局变量
*@param        :void
*@return       :void
*@history      :V1.0 create by xuxiaojun@20230330
*/
void RSP_InitGobalVariable(void)
{
	RSP_ClearSession();

	ecpMem_Set((BYTE *)&G_CustomerStoreDataApdu, 0x00, sizeof(G_CustomerStoreDataApdu));

	ecpMem_Set((BYTE *)&G_SendFileExtendBackDataStruct, 0x00, sizeof(G_SendFileExtendBackDataStruct));

	G_SendFileExtendBackDataStruct.currentChannel = ILLEGAL_CHANNEL;
}
#endif

