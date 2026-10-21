
/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   TlsMainFlow.c
*
* Description: 该文件主要用于存放TLS 层算法相关的函数
*
* @see        :<rfc5246.txt.pdf>
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0 :create @20180411 ,该文档主要存放用于处理TLS的主流程
				    
*
************************************************************************************************************/
#include "DNS.h"
#include "opFunction.h"
#include "SecurityDomainData.h"
#include "TlsMainFlow.h"
#include "FileAction.h"
#include "fileSystem.h"
#include "TlsAlgorithm.h"
#include "HttpProcess.h"
#include "Interface_AnnexB.h"
#include "ToolkitRFM.h"
#include "ToolkitApiPublic.h"
#include "StkCatRuntimeEnviorment.h"
#include "BIPModuleProcess.h"
#include "Gsmpublic.h"
#include "ExpandedManage.h"
#include "CompactManage.h"
#include "platform_api.h"
#include "JcvmConfig.h"
#include "uicc_interface.h"
#include "uicc_M2M.h"
#include "hard_api.h"
#include "cardManager.h"
#include "T0ApduImpl.h"
#include "bc_array.h"
#include "Interface_DNS.h"
#include "ob_manager.h"
#include "Interface_TimerModule.h"
#include "ASN1Operation.h"

#if GP__AMENDMENT__B__DNS_Resolution
DNSByMEConnectStatus xdata G_DNSByMEConnectStatus;
DNSIPConnectStatus xdata G_DNSIPConnectStatus;
RASIPConnectStatus xdata G_RASIPConnectStatus;
DNSResolutionProcedureState xdata G_DNSResolutionProcedureState;
/*
*@description :初始化DNS Resolution Procedure过程中涉及到的一些全局变量
*
*@see         :
*
*@function    :DNSResolutionInitVariable
*
*@param       :void
*
*@return      :
*
*@history     V1.0 :created by xxx@xxxxxx
*/
void DNSResolutionInitVariable(void)
{
	ecpMem_Fill((BYTE *)&G_DNSByMEConnectStatus, 0x00, sizeof(G_DNSByMEConnectStatus));

	ecpMem_Fill((BYTE *)&G_DNSIPConnectStatus, 0x00, sizeof(G_DNSIPConnectStatus));

	ecpMem_Fill((BYTE *)&G_RASIPConnectStatus, 0x00, sizeof(RASIPConnectStatus));

	ecpMem_Fill((BYTE *)&G_DNSResolutionProcedureState, 0x00, sizeof(DNSResolutionProcedureState));
}
/*
*@description :
*
*@see         :
*
*@function    :fetchDNSIPListFromME
*
*@param       :void
*
*@return      :
*
*@history     V1.0 :created by xxx@xxxxxx
*/
u8 fetchDNSIPListFromME(void)
{
	return FALSE;
}
/*
*@description :判断当前的设备是否支持DNS-From-ME机制
*
*@see         :
*
*@function    :isSupportDNSFromMEMechanism
*
*@param       :void
*
*@return      :TRUE表示支持; FALSE表示不支持
*
*@history     V1.0 :created by xuxiaojun@20211228
*/
u8 isSupportDNSFromMEMechanism()
{
	return FALSE;
}
/*
*@description :获取DNS IP Address List。 进入该函数时curState = DNSResolutionProcedureState_CONNECT_DNS_IP
*
*@see         : GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf  3.11.7  DNS IP Address List
*
*@function    :fetchingDNSIPList
*
*@param       :BYTE mode --- 表示当前进入这个函数的模式是Timer Expiration后再次进来，还是初次进来
*
*@return      :
*
*@history     V1.0 :created by xuxiaojun@20211228
*/
BYTE fetchingDNSIPList(void)
{
	u8 idata fetchResult;
	PN8		X_A		sdDataObj;
	jref	X_A  	bufferRef;

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	bufferRef = sdData_getDNS_F9_bufferRef(sdDataObj);

	if(isSupportDNSFromMEMechanism() == TRUE) //支持DNS-From-ME mechanism
	{
		// todo 只有第一次近来的时候，才需要执行这两行
		if(G_DNSIPConnectStatus.resume ==  FALSE)
		{
			G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_Fetch_DNS_IP_List_from_ME;

			G_DNSByMEConnectStatus.tryCounter = InitializeDNSFromMETryCounter(); //todo Initialize DNS By ME Try Counter
		}

		fetchResult = fetchDNSIPListFromME(); // 如果获取成功的话，对应的数据与DNS IP Address List的存储在同一片区域,可能会出现两种情况：新建对象 或者 更新对象内容

		bufferRef = sdData_getDNS_F9_bufferRef(sdDataObj);

		if(fetchResult == NOK_WITH_No_Suitable_Answer)
		{
			if(G_DNSIPConnectStatus.tryCounter > 0)
			{
				G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_Fetch_DNS_IP_List_from_ME;

				DNSByME_StartTimer();

				return RETRY_POLICY;
			}
			else
			{
				if(bufferRef == NULL_REF)
				{
					return UICC_STOP;
				}
				else
				{
					return OK_WITH_DNS_IP_List_Cache_Non_Empty; // BACK TO [DNS2] WITH OK Non Empty DNS IP List Cache
				}
			}
		}
		else
		{
			if(bufferRef == NULL_REF)
			{
				return OK_WITH_DNS_IP_List_Cache_Empty; // BACK TO [DNS2] WITH OK Non Empty DNS IP List Cache
			}
			else
			{
				return OK_WITH_DNS_IP_List_Cache_Non_Empty; // BACK TO [DNS2] WITH OK Non Empty DNS IP List Cache
			}
		}
	}
	else
	{
		if(bufferRef == NULL_REF)
		{
			return UICC_STOP; // Empty
		}
		else
		{
			return OK_WITH_DNS_IP_List_Cache_Non_Empty; // BACK TO [DNS2] WITH OK Non Empty DNS IP List Cache
		}
	}
}

/*
*@description :获取用于dns会话的FQDN的值
*
*@see         :3.10.2  DNS Queries to DNS Server
*
*@function    :getDNSFQDN
*
*@param       :BYTE * FQDNBuf	输出值, FQDN的值, 
*@param       :BYTE  FQDNLen	输入值, FQDNBuf的可用空间, 
*
*@return      :获取到的FQDN的长度
*
*@history     V1.0 :created by jiangqingbin@20220106
*/
WORD getDNSFQDN(BYTE * FQDNBuf, BYTE FQDNLen)
{
	UWORD xdata triggerParaFileAddr;
	WORD  idata bufferLen;
	
	PN8		X_A		sdDataObj;
	jref	X_A  	bufferRef;
	memref	X_A  	bufferAddr;
	memref	X_A  	bufferField;
	
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  
	
	bufferLen = ReadLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_FQDN_Index, ResData);
	
	if (bufferLen == 0)	//表示 触发sms 中的触发参数没有带FQDN参数, 此时需要找预置的FQDN参数
	{	
		sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));
		
		bufferRef = sdData_getDNS_D6_bufferRef(sdDataObj);
		
		if (bufferRef == NULL_REF)
		{
			return 0;
		}

		bufferAddr = locateObjectInMemory(bufferRef);

		bufferLen = get_array_length(bufferAddr);
	
		bufferField = get_array_data(MEMtPF8(bufferAddr));
		
		if (bufferLen > FQDNLen)	//数据过长无法读取	
		{
			return 0;
		}
		
		ecpMem_Copy(FQDNBuf, bufferField, bufferLen); 		
	}
	
	return bufferLen;
}



/*
*@description :执行DNS Queries to DNS Server过程，建立RAS IP Address List
*
*@see         :3.10.2  DNS Queries to DNS Server
*
*@function    :performDNSQueries
*
*@param       :void
*
*@return      :void
*
*@history     V1.0 :created by jiangqingbin@20220106
*/
u8 performDNSQueries(void)
{
	WORD idata offSet;
	BYTE  idata result;
	
	offSet = 2;
		
	////dns head 部分
	//head id， 规范没有找到规定事务id的值， 此处直接沿用二部的LTE2.0方案
	tempBuf[offSet++] = 0x3D;
	tempBuf[offSet++] = 0x34;
	
	//head flag，每个bit都有含义，此处默认使用0100
	tempBuf[offSet++] = 0x01;
	tempBuf[offSet++] = 0x00;
	
	//head question num，即请求的问题个数，此处默认为请求ip地址，所以是0001
	tempBuf[offSet++] = 0x00;
	tempBuf[offSet++] = 0x01;
	
	//head answer num, 回答的个数，此处在下行数据中才会有效
	tempBuf[offSet++] = 0x00;
	tempBuf[offSet++] = 0x00;
	
	//head answer number of name server resource， 回答时才有效， 权威名称服务器的个数
	tempBuf[offSet++] = 0x00;
	tempBuf[offSet++] = 0x00;
	
	//head answer number of additional, 回答时才有效，附加信息的个数
	tempBuf[offSet++] = 0x00;
	tempBuf[offSet++] = 0x00;
	
	////dns Question 部分， 即请求的问题部分， 请求问题的NAME(LV格式，即FQDN)，TYPE, CLASS
	//Question name， 即DNS参数(00f5)中(D6)FQDN,
	result = getDNSFQDN(tempBuf+offSet, 0xFF);
	
	if (result == 0)	//找不到FQDN时无法进行DNS会话
	{
		return FALSE;		
	}
	
	offSet += result;
	
	//tempBuf[offSet++] = 0x00;		//注释本行, 因为一个完整的 FQDN 的最后一个字节必须是 00

	//Question type, DNS查询请求的资源类型，通常是域名IP地址类型， 并此处参考LTE2.0赋值为0001
	tempBuf[offSet++] = 0x00;
	tempBuf[offSet++] = 0x01;
	
	//Question class，地址类型，此处参考LTE2.0赋值为0001(0001是互联网ip地址)
	tempBuf[offSet++] = 0x00;
	tempBuf[offSet++] = 0x01;
		
	//bipSendAddr = SelectTFFile(DF_BIPModule, EF_BIPModule_Send);
	
	//WriteTFFileValue(bipSendAddr, 2, tempBuf, offSet, offSet);
	
	SetWord(&tempBuf[0], (offSet-2));
	
	if (BIPModuleSendResponseData(DataInRam, tempBuf)) 		 //发送数据不成功的处理
	{
		return FALSE;			
	}
	
	return TRUE;
}

/*
*
*@description :保存通过 DNS 获取到的新的 RAS IP Address List
*
*@see         :3.10.2  DNS Queries to DNS Server
*
*@function    :storeRASIPList
*
*@param       :void
*
*@return      :void
*
*@history     V1.0 :created by jiangqingbin@20220106
*/
void storeRASIPList( BYTE * rasIPListBuf, BYTE rasIPListLen)
{
	PN8		X_A		sdDataObj;
	jref	X_A  	arrayRef;
	memref	X_A  	arrayField;
	
	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));
		
	arrayRef = sdData_getDNS_F7_bufferRef(sdDataObj);
	
	if(arrayRef != REF_NULL) //前面已经分配过，打上回收标记, 后续直接申请新的空间
	{	
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
		{
			bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
		}

	}
	
	if (rasIPListLen == 0)
	{
		sdData_setDNS_F7_bufferRef(sdDataObj, REF_NULL);	//清空
		
		return;		
	}
	
	arrayRef = _newarrayObject(AType_Byte, rasIPListLen, BIT_NEED_REFRESH);

	sdData_setDNS_F7_bufferRef(sdDataObj, arrayRef);
	
	arrayField = get_array_data(MEMtPF8(locateObjectInMemory(arrayRef)));

	nvmCopyAtomic(MEMtPM8(arrayField),&rasIPListBuf[0], rasIPListLen);
}



/*
*@description :执行DNS Server to DNS Queries 过程，解析下行的数据， 提取 RAS IP Address List并保存
*
*@see         :3.10.2  DNS Queries to DNS Server
*
*@function    :ProcessDNSResponse
*
*@param       :void
*
*@return      :成功解析时返回 OK_Empty_RAS_IP_List_Cache/OK_Non_Empty_RAS_IP_List_Cache , 否则返回 NOK_WITH_No_Suitable_Answer;
*
*@history     V1.0 :created by jiangqingbin@20220106
*
DNS MESSAGES format：
	+---------------------+
	|        Header       |	12字节, 6个地段, 具体参考一下例子
	+---------------------+
	|       Question      | 询问的地址,TYPE, CALSS, 具体参考一下例子
	+---------------------+
	|        Answer       | RRs answering the question  (回答问题区域字段)
	+---------------------+
	|      Authority      | RRs pointing toward an authority (权威名称服务器区域字段)
	+---------------------+
	|      Additional     | RRs holding additional information (附加信息区域字段)
	+---------------------+
	
----例如以下数据：

Header:											
	3D34										;事务id (下行的必须和上行的一致)
	8580										;标志Flags
	0001										;查询问题记录数
	0001										;应答资源记录数
	0002										;权威名称服务器记录数
	0002										;附加资源记录数
	
Question:										(Question字段数据必须上下行数据一致)
	066F74616C74650674656C63656C03636F6D00		;想要查询的主机名称,如左边数字的意思是：otalte.telcel.com, 此字段必须是0结束
	0001										;所查询的 type,   通常是type A,   编码是 0001, 即IPv4类型
	0001										;所查询的 class , 通常是Class IN ,编码是 0001, 即internet 网址
	
----------------以下字段在 dns 下行数据时才存在----------------
----------------并且Answer, Authority, Additional的数据格式一致------
----------------以下数据的格式, 每一条资源的数据分别表示：域名(16bit), type(16bit), class(16bit), timer(32bit有效时间单位秒), ip长度(16bit), ip值----
----------------每一条资源的数据编码：
					域名：	16bit, 	最高两个bit为1时表示是指针类型, 后面14个bit表示域名在dns数据中的字节偏移数， C00C(1100 0000 0000 0011), 高两个1表示指针, 其余的值为0x0C, 即本次dns数据的偏移到0x0C的域名的响应数据
					type：	16bit,	必须匹配的上行dns的Question中的type
					class： 16bit,	必须匹配的上行dns的Question中的class
					timer： 32bit,	有效事件, 比如00000E10 表示 3600 秒有效事件
					ip长度：16bit,	后续ip值的字节数, 比如type A(IPV4)必须为4
					ip值：  xxbit,	ip值, 比如type A(IPV4)必须为4字节,  0xBB.0xED.0x08.0x07 -->转为10进制：  187.237.08.07

Answer:		(在目前的业务中,该字段就是dns会话获取到的ras-ip)
	;Answer 的个数对应 Header 的 应答资源记录数
	C00C 0001 0001 00000E10 0004 BBED0807		
	
Authority：	(在目前的业务中,该字段忽略, 后续有可能使用本字段中的数据作为新的 FQDN 再次拼装上行数据询问)
	;Authority 的个数对应 Header 的 权威名称服务器记录数
	C013 0002 0001 00000E10 0010 05646E733169 07 6974656C63656C C01A		
	C013 0002 0001 00000E10 0011 05646E733031 08 616D69676F6B6974 C01A	
	
Additional：(在目前的业务中,该字段暂时忽略, 具体作用还未明确)
	;Additional 个数对应 Header 的 附加资源记录数
	C05B 0001 0001 00000E10 0004 C826D0C8								
	C03F 0001 0001 00000E10 0004 94E99706  
*
*/
BYTE ProcessDNSResponse()
{
	UWORD xdata bipReceiveAddr;
	WORD idata  dnsResponseLen;
	WORD idata  fileOffSet;
	WORD idata  tempBufOffSet;
	BYTE idata  FQDNLen;
	BYTE idata  readLen;
	WORD idata  answerNum;
	BYTE xdata	rasIPListBuf[0x60];
	BYTE idata  rasIPListOffset;
	BYTE idata  i;

//bipReceiveAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Receive);
	bipReceiveAddr = BIPModule_Receive_ADDR_START;
	
//dnsResponseLen = ReadTFFileWord(bipReceiveAddr);
	dnsResponseLen = G_BIPModule_Receive_Length;
	
	BIPModuleProcessStatus();	//关 DNS 通道

	if (dnsResponseLen == 0)
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	readLen = 0xFF;
	
	fileOffSet = 2;

//	ReadTFBuf(bipReceiveAddr, fileOffSet, tempBuf, readLen);	//先读部分数据解析
	ReadEeprom(tempBuf,bipReceiveAddr + fileOffSet,readLen);
	
	tempBufOffSet = 0;
	
	//dns head id 
	if ((tempBuf[tempBufOffSet] != 0x3D) || (tempBuf[tempBufOffSet+1] != 0x34))		//head id check
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	tempBufOffSet += 2;
	
	//dns head flag,  此处只必须置1的bit， 其他可变的没有判断
	if (((tempBuf[tempBufOffSet]&0x80) != 0x80) || ((tempBuf[tempBufOffSet+1]&0x87) != 0x80))
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	tempBufOffSet += 2;														//point to ipnum
	
	//dns head questionNum
	if (((tempBuf[tempBufOffSet]) != 0x00) || ((tempBuf[tempBufOffSet+1]) != 0x01))
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	tempBufOffSet += 2;
	
	//dns head answerNum,响应的 RAS IP 个数
	answerNum = BuildWord(&tempBuf[tempBufOffSet]);
	
	//中间其他的头项暂时不做检查
	
	tempBufOffSet += 6;														//point to FQDN
	
	FQDNLen = getDNSFQDN(RecData, 0xFF);
	
	if (ecpMem_Compare(tempBuf+tempBufOffSet, RecData, FQDNLen) != EQUAL)				//上下行数据的FQDN不一致
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	tempBufOffSet += FQDNLen;
	
	//point to answer type	
	if ((tempBuf[tempBufOffSet] != 0x00) || (tempBuf[tempBufOffSet+1] != 0x01))
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	tempBufOffSet += 2;														//point to answer class
	
	if ((tempBuf[tempBufOffSet] != 0x00) || (tempBuf[tempBufOffSet+1] != 0x01))
	{
		return NOK_WITH_No_Suitable_Answer;
	}
	
	tempBufOffSet += 2;			//point to answer

	ecpMem_Fill(rasIPListBuf, 0x00, sizeof(rasIPListBuf));
	
	rasIPListOffset = 0;
	
	fileOffSet += tempBufOffSet;
		
	i = 1;
	
	while ((i <= answerNum) && (fileOffSet < dnsResponseLen))			//此处是所有响应数据分析完的循环判断
	{	
		if ((fileOffSet + readLen) > (dnsResponseLen+2))
		{
			readLen = dnsResponseLen + 2 - fileOffSet;
		}
		
//	ReadTFBuf(bipReceiveAddr, fileOffSet, tempBuf, readLen);
		ReadEeprom(tempBuf,bipReceiveAddr + fileOffSet,readLen);
		
		fileOffSet += readLen;
		
		tempBufOffSet = 0;
			
		for (; ((i <= answerNum) && (tempBufOffSet < readLen)); i++)	//此处是所有 answer 数据分析完的循环判断
		{	
			if (tempBuf[tempBufOffSet] != 0xC0)							
			{
				return NOK_WITH_No_Suitable_Answer;
			}
			
			tempBufOffSet += 2;												//point to answer type	
			
			if ((tempBuf[tempBufOffSet] == 0x00) && (tempBuf[tempBufOffSet+1] == 0x01))	//type 匹配
			{
				tempBufOffSet += 2;												//point to answer class	
				
				if ((tempBuf[tempBufOffSet] == 0x00) && (tempBuf[tempBufOffSet+1] == 0x01))	//class 匹配
				{
					tempBufOffSet += 2;											//point to live time, 该ip在多长时间内有效(秒)，目前不做时间的判断
					
					tempBufOffSet += 4;											//point to ip data length
					
					if (tempBuf[tempBufOffSet] == 0x00)
					{
						tempBufOffSet++;
						
						if (tempBuf[tempBufOffSet] == 0x04)		//目前主动请求的都是ipv4, 长度都是4
						{
							tempBufOffSet ++;
							
							rasIPListBuf[rasIPListOffset++] = 0x3E;
							
							rasIPListBuf[rasIPListOffset++] = 0x05;
							
							rasIPListBuf[rasIPListOffset++] = 0x21;
		
							ecpMem_Copy(&rasIPListBuf[rasIPListOffset], &tempBuf[tempBufOffSet], 4);
							
							tempBufOffSet += 4;
							
							rasIPListOffset += 4;
						}
						else
						{
							return NOK_WITH_No_Suitable_Answer;
						}
					}
					else
					{
						return NOK_WITH_No_Suitable_Answer;
					}
				}
				else
				{
					return NOK_WITH_No_Suitable_Answer;
				}	
			}
			else
			{
				return NOK_WITH_No_Suitable_Answer;
			}	
		}
	}
	
	storeRASIPList(rasIPListBuf, rasIPListOffset);
	
	if (rasIPListBuf[0] == 0)		//空的 ip list
	{
		return OK_Empty_RAS_IP_List_Cache;
	}
	else
	{		
		return OK_Non_Empty_RAS_IP_List_Cache;
	}
}

/*
*@description :
*
*@see         :Figure B-3:  DNS Resolution [DNS2]
*
*@function    :DNSResolution_DNS2
*
*@param       :void
*
*@return      :返回值的类型有以下几种：
*              RETRY_POLICY
*              STOP
*              DNS_QUERIES
*              NOK_WITH_No_Suitable_Answer
*              OK_Empty_RAS_IP_List_Cache
*              OK_Non_Empty_RAS_IP_List_Cache
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
u8 DNSResolution_DNS2(void)
{
	u8 idata fetchingDNSIPListReslut;

	fetchingDNSIPListReslut = fetchingDNSIPList();

	//fetchingDNSIPListReslut的返回值可能为RETRY_POLICY、STOP、OK_WITH_DNS_IP_List_Cache_Empty、OK_WITH_DNS_IP_List_Cache_Non_Empty
	if(fetchingDNSIPListReslut == OK_WITH_DNS_IP_List_Cache_Non_Empty)
	{
		fetchingDNSIPListReslut = DNSResolution_ConnectDNS(); //可能的返回值为 RETRY_POLICY、DNS_QUERIES、NOK_WITH_No_Suitable_Answer
	}

	return fetchingDNSIPListReslut;
}

/*
*@description : 检查配置是否需要获取ras-ip-list, 并且是否获取成功
*
*@see         :GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf  Figure B-2:  DNS Resolution [DNS1]
*
*@function    :DNSResolution_DNS1
*
*@param       :void
*
*@return      :返回值可能为RETRY_POLICY
*                       STOP
*                       DNS_QUERIES
*                       NOK_WITH_No_Suitable_Answer
*                       OK_Empty_RAS_IP_List_Cache
*                       OK_Non_Empty_RAS_IP_List_Cache
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
u8 DNSResolution_DNS1(void)
{
	u8 idata dns2Result;
	PN8	 X_A   sdDataObj;
	jref I_A   rasListBufferRef;

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	rasListBufferRef = sdData_getDNS_F7_bufferRef(sdDataObj);

	if(G_DNSResolutionProcedureState.required == FALSE) // Check DNS Resolution Required == No, 此时直接去读取 RAS IP List;
	{
		if(rasListBufferRef == NULL_REF) // Check RAS IP List Cache == Empty, 后续需要开启dns业务获取 RAS IP List;
		{
			if(G_DNSResolutionProcedureState.performed == FALSE) // Check DNS Resolution Performed == No, 表示本次session 未开启dns会话
			{
				dns2Result = DNSResolution_DNS2(); // 迭代调用,但是这个时候，DNS的地址是不一样的，需要找到下一个DNS地址
				//dns2Result的值可能为RETRY_POLICY、STOP、DNS_QUERIES、NOK_WITH_No_Suitable_Answer、OK_Empty_RAS_IP_List_Cache、OK_Non_Empty_RAS_IP_List_Cache
				if(dns2Result == OK_Non_Empty_RAS_IP_List_Cache)
				{
					G_DNSResolutionProcedureState.performed = TRUE;

					G_DNSResolutionProcedureState.required = FALSE;
				}

				return dns2Result;
			}
			else  // Check DNS Resolution Performed == Yes, 此时performed为yes表示已经开启过dns会话，此时RAS IP List如果还是空的，即dns获取了空的ras-ip-list, 此时返回状态应该和ras-ip-list不为空时一致,
			{
				G_DNSResolutionProcedureState.performed = TRUE; //Set DNS Resolution Performed = YES
				G_DNSResolutionProcedureState.required = FALSE; //                   Required = NO
				return OK_Empty_RAS_IP_List_Cache;   // back to main
			}
		}
		else  // Check RAS IP List Cache == Not Empty, 原有的 RAS IP List 不为空, 返回 main尝试使用 RAS IP 开启http通道
		{
			G_DNSResolutionProcedureState.performed = TRUE; //Set DNS Resolution Performed = YES
			G_DNSResolutionProcedureState.required = FALSE; //                   Required = NO
			return OK_Non_Empty_RAS_IP_List_Cache;   // back to main
		}
	}
	else // Check DNS Resolution Required == Yes
	{
		if(G_DNSResolutionProcedureState.performed == FALSE) // Check DNS Resolution Performed == No
		{
			dns2Result = DNSResolution_DNS2(); // 迭代调用,但是这个时候，DNS的地址是不一样的，需要找到下一个DNS地址

			//dns2Result的值可能为RETRY_POLICY、STOP、DNS_QUERIES、NOK_WITH_No_Suitable_Answer、OK_Empty_RAS_IP_List_Cache、OK_Non_Empty_RAS_IP_List_Cache
			if(dns2Result == OK_Non_Empty_RAS_IP_List_Cache)
			{
				G_DNSResolutionProcedureState.performed = TRUE;

				G_DNSResolutionProcedureState.required = FALSE;
			}

			return dns2Result;
		}
		else  // Check DNS Resolution Performed == Yes
		{
			G_DNSResolutionProcedureState.performed = TRUE; //Set DNS Resolution Performed = YES
			G_DNSResolutionProcedureState.required = FALSE; //                   Required = NO
			return OK_Non_Empty_RAS_IP_List_Cache;   // back to main
		}
	}
}
/*
*@description :执行Connect RAS IP的操作，如果open channel成功，则状态切换到curState = ESTABLISH_TLS;否则进行重试
*
*@see         :Figure B-1:  Administration Session started using DNS Resolution [MAIN]
*
*@function    :openChannel_ConnectRASIP
*
*@param       :void
*
*@return      :void
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
u8 openChannel_ConnectRASIP(void)
{
	BYTE idata result;

	result = BIPModuleOpenChannel(ChannelDataType_RAS);

	return result;  //ExitStkPerform;
}
/*
*@description :执行Connect RAS IP的操作，如果open channel成功，则状态切换到curState = ESTABLISH_TLS;否则进行重试
*
*@see         :Figure B-1:  Administration Session started using DNS Resolution [MAIN]
*
*@function    :openChannel_ConnectDNSIP
*
*@param       :void
*
*@return      :void
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
u8 openChannel_ConnectDNSIP(void)
{
	BYTE idata result;

	result = BIPModuleOpenChannel(ChannelDataType_DNS);

	return result;  //ExitStkPerform;
}
/*
*@description :查找下一个可用的RAS IP Address.该函数中对G_RASIPConnectStatus全局变量结构体赋初始值
*
*@see         :Figure B-1:  Administration Session started using DNS Resolution [MAIN]
*
*@function    :SelectNextRASIP
*
*@param       :void
*
*@return      :
*
*@history     V1.0 :created by xuxiaojun@20220106
*/
void SelectNextRASIP(void)
{
	PN8	 X_A   sdDataObj;
	jref I_A   rasListBufferRef;
	memref	X_A rasListBufferAddr;
	memref	X_A rasListBufferField;
	WORD idata rasListBufferLen;
	BYTE idata currIPLen;

	G_RASIPConnectStatus.timerId = 0x00; //初始化为非法值

	G_RASIPConnectStatus.tryCounter = 0x00;

	G_RASIPConnectStatus.resume = FALSE;

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	rasListBufferRef = sdData_getDNS_F7_bufferRef(sdDataObj);

	if(rasListBufferRef != REF_NULL)
	{
		rasListBufferField = sdData_getDNS_F7_bufferField(sdDataObj);

		rasListBufferAddr = locateObjectInMemory(rasListBufferRef);

		rasListBufferLen = U16t8(get_array_length(rasListBufferAddr));

		//todo: ras ip 的选择在 gemalto 规范中有权重, 目前没有实现该功能
		
		if(G_RASIPConnectStatus.ipAddress == MEM_NULL)
		{
			G_RASIPConnectStatus.ipAddress = rasListBufferField;

			//currIPLen = VMReadByte(G_RASIPConnectStatus.ipAddress + 1);

			G_RASIPConnectStatus.tryCounter = InitializeRASIPTryCounter();
		}
		else
		{
			currIPLen = VMReadByte(G_RASIPConnectStatus.ipAddress + 1);

			//偏移到下一个IP地址时, 其地址值还在 list 地址范围内。这种地址的判断方法, 是否合理
			if ((G_RASIPConnectStatus.ipAddress + (currIPLen + 2)) < (rasListBufferField + rasListBufferLen))
			{
				G_RASIPConnectStatus.ipAddress += (currIPLen + 2);

				G_RASIPConnectStatus.tryCounter = InitializeRASIPTryCounter();
			}
			else
			{
				G_RASIPConnectStatus.ipAddress = MEM_NULL;
			}
		}
	}

}
/*
*@description :查找下一个可用的DNS IP Address.该函数中对G_RASIPConnectStatus全局变量结构体赋初始值
*
*@see         :Figure B-1:  Administration Session started using DNS Resolution [MAIN]
*
*@function    :SelectNextDNSIP
*
*@param       :void
*
*@return      :
*
*@history     V1.0 :created by jiangqingbin@xxxxxxxx
*/
void SelectNextDNSIP(void)
{
	PN8	 X_A   sdDataObj;
	jref I_A   dnsListBufferRef;
	memref	X_A dnsListBufferAddr;
	memref	X_A dnsListBufferField;
	WORD idata dnsListBufferLen;
	BYTE idata currIPLen;
	
// todo 这里需要考虑3.11.8  DNS Connection Parameters 'FA'中存在IP地址时，就应该忽略3.11.7  DNS IP Address List 'F9'

	G_DNSIPConnectStatus.timerId = 0x00; //初始化为非法值

	G_DNSIPConnectStatus.tryCounter = 0x00;

	G_DNSIPConnectStatus.resume = FALSE;

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	dnsListBufferRef = sdData_getDNS_F9_bufferRef(sdDataObj);
	
	if(dnsListBufferRef != REF_NULL)
	{
		dnsListBufferField = sdData_getDNS_F9_bufferField(sdDataObj);

		dnsListBufferAddr = locateObjectInMemory(dnsListBufferRef);

		dnsListBufferLen = U16t8(get_array_length(dnsListBufferAddr));

		if(G_DNSIPConnectStatus.ipAddress == MEM_NULL)
		{
			G_DNSIPConnectStatus.ipAddress = dnsListBufferField;

			currIPLen = VMReadByte(G_DNSIPConnectStatus.ipAddress + 1);

			G_DNSIPConnectStatus.tryCounter = InitializeDNSIPTryCounter();
		}
		else
		{
			currIPLen = VMReadByte(G_DNSIPConnectStatus.ipAddress + 1);

			//偏移到下一个IP地址时, 其地址值还在 list 地址范围内。这种地址的判断方法, 是否合理
			if ((G_DNSIPConnectStatus.ipAddress + (currIPLen + 2)) < (dnsListBufferField + dnsListBufferLen))
			{
				G_DNSIPConnectStatus.ipAddress += (currIPLen + 2);

				G_DNSIPConnectStatus.tryCounter = InitializeDNSIPTryCounter();
			}
			else
			{
				G_DNSIPConnectStatus.ipAddress = MEM_NULL;
			}
		}
	}
}


/*
*@description : 在DNS-IP-LIST 不为空时, 准备逐个尝试其中的ip开启dns会话
*
*@see         :GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf  Figure B-3:  DNS Resolution [DNS2]
*
*@function    :DNSResolution_ConnectDNS
*
*@param       :void
*
*@return      :返回值可能为：RETRY_POLICY、DNS_QUERIES、NOK_WITH_No_Suitable_Answer
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
u8 DNSResolution_ConnectDNS(void)
{
	u8 idata querResult;
	u8 idata openChannelResult;

	do		
	{
		if(G_DNSIPConnectStatus.resume == FALSE)
		{
			SelectNextDNSIP(); //怎么样区分，是openChannel_ConnectDNSIP失败导致的再次进入呢？ 这个dns会话失败都是要开启dns重试timer，不用区分不同情况
		}

		if(G_DNSIPConnectStatus.ipAddress == MEM_NULL)// All DNS IP tried
		{
			break;
		}
		else
		{
			openChannelResult = BIPModuleOpenChannel(ChannelDataType_DNS);	//openChannel_ConnectDNSSIP(); 这里的IP地址可能是3.11.8  DNS Connection Parameters 'FA'里的

			if(CMD_Success != openChannelResult)
			{
				if(!ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__OpenChannel_With_3X_Retry)) // 这里是否有必要做成配置
				{
					if((0x30 <= openChannelResult) && (openChannelResult <= 0x3F)) //<102223> Results '3X' indicate that it is not worth the UICC re-trying with an identical command, as it will only get the same response. However, the decision to retry lies with the application.
					{
						G_DNSIPConnectStatus.tryCounter = 0x00;
					}
				}

				if(G_DNSIPConnectStatus.tryCounter > 0)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_DNS_IP;

					DNSConnect_StartTimer();

					return RETRY_POLICY;
				}
				else
				{
					G_DNSIPConnectStatus.resume = FALSE; //循环到SelectNextDNSIP
					
					continue;
				}
			}
			else
			{
				querResult = performDNSQueries();

				if(querResult == TRUE)
				{
					return DNS_QUERIES;
				}
				else //connect DNS 开通道成功，执行performDNSQueries失败，避免影响下一次的open channel ,先将通道关闭
				{
					if(G_DNSIPConnectStatus.tryCounter > 0)
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_DNS_IP;

						DNSConnect_StartTimer();

						return RETRY_POLICY;
					}
					else
					{						
						BIPModuleProcessStatus(); // 开通道成功，执行performDNSQueries失败，避免影响下一次的open channel ,先将通道关闭
						
						G_DNSIPConnectStatus.resume = FALSE; //循环到SelectNextDNSIP, 直接发送连接下一个DNS的open channel，不发start timer
						
						continue;
					}
				}
			}
		}


	}while(G_DNSIPConnectStatus.ipAddress != MEM_NULL);

	//走到这里时， All_DNS_IP_tried == TRUE
	return NOK_WITH_No_Suitable_Answer; // BACK TO [DNS1] NOK No Suitable Answer
}
/*
*@description :建立RAS IP连接，如果一个RAS IP建立连接不成功，则用下一个进行，直到成功建立或者所有的RAS IP遍历完
*
*@see         :Figure B-1:  Administration Session started using DNS Resolution [MAIN]
*
*@function    :DNSResolution_ConnectRAS
*
*@param       :void
*
*@return      :返回值可能为RETRY_POLICY、STOP、OK_Non_Empty_RAS_IP_List_Cache、OK_Empty_RAS_IP_List_Cache
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
u8 DNSResolution_ConnectRAS(void)
{
	u8 idata openChannelResult;

	do
	{
		if(G_RASIPConnectStatus.resume == FALSE) //用这个区分是重试时进入
		{
			SelectNextRASIP();
		}

		if(G_RASIPConnectStatus.ipAddress == MEM_NULL)// All RAS IP tried
		{
			break;
		}
		else
		{
			openChannelResult = BIPModuleOpenChannel(ChannelDataType_RAS); //openChannel_ConnectRASIP();

			if(CMD_Success != openChannelResult)
			{
				if(!ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__OpenChannel_With_3X_Retry)) // 这里是否有必要做成配置
				{
					if((0x30 <= openChannelResult) && (openChannelResult <= 0x3F)) //<102223> Results '3X' indicate that it is not worth the UICC re-trying with an identical command, as it will only get the same response. However, the decision to retry lies with the application.
					{
						G_RASIPConnectStatus.tryCounter = 0x00;
					}
				}

				if(G_RASIPConnectStatus.tryCounter > 0)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_RAS_IP;

					RASConnect_StartTimer();

					return RETRY_POLICY;
				}
				else
				{
					G_RASIPConnectStatus.resume = FALSE;
				}
			}
			else
			{
				G_BIPModuleStatus.channelDataType = ChannelDataType_HTTPS; // RAS连接成功之后，后续进行TLS握手和HTTP指令交互，为了复用之前的代码，将该变量切换到ChannelDataType_HTTPS

				return OK_Non_Empty_RAS_IP_List_Cache;
			}
		}
	}while(G_RASIPConnectStatus.ipAddress != MEM_NULL);  // All RAS IP tried

	//走到这里，说明所有的RAS IP都被尝试过了
	if(G_DNSResolutionProcedureState.performed == FALSE)
	{
		G_DNSResolutionProcedureState.required = TRUE;

		return OK_Empty_RAS_IP_List_Cache; // 这种情况需要重复执行dns1
	}
	else
	{
		if(G_HTTPSStatus.RetryConterRemain > 0)
		{
			G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS1;

			BIPModuleProcessStatus(); // 目前bip只支持一个，走到这里时肯定是DNS过程开的BIP通道
			
			IF_AnnexB_StartTimerAfterUnNormalEnd();

			return RETRY_POLICY;
		}
		else
		{
			return UICC_STOP;
		}
	}
}
/*
*@description :返回Security Domain DNS Resolution Parameters中Force DNS Resolution的值
*
*@see         :3.11  Security Domain DNS Resolution Parameters
*
*@function    :getValueofForceDNSResolution
*
*@param       :void
*
*@return      :Force DNS Resolution的值
*
*@history     :v1.0--created by xuxiaojun@20220102
*/
BYTE getValueofForceDNSResolution()
{
	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
	jref    X_A     bufferRef;
    u8      X_A     D0Buffer[3];

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

	if(bufferRef == NULL_REF)
	{
		return 0x00;
	}

	bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

	ReadEeprom(D0Buffer, (bufferFeild + D0_OFFSET_DNS_D0_D3_buffer), sizeof(D0Buffer));

	if(D0Buffer[0] == TAG_D0_IN_DNS_Resolution_Parameters)
	{
		return D0Buffer[2];
	}
	else
	{
		return 0x00;
	}
}
/*
*@description :获取Security Domain DNS Resolution Parameters中DNS-From-ME Retry Policy Parameters的try counter值
*
*@see         :3.11  Security Domain DNS Resolution Parameters
*
*@function    :InitializeDNSFromMETryCounter
*
*@param       :void
*
*@return      :DNS-From-ME Retry Policy Parameters的try counter值
*
*@history     :v1.0--created by xuxiaojun@20211228
*/
WORD InitializeDNSFromMETryCounter(void)
{
	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
	jref    X_A  	bufferRef;
    u8      X_A     D1Buffer[9];

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

	if(bufferRef == NULL_REF)
	{
		return 0x00;
	}

	bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

	ReadEeprom(D1Buffer, (bufferFeild + D1_OFFSET_DNS_D0_D3_buffer), sizeof(D1Buffer));

	if(D1Buffer[0] == TAG_D1_IN_DNS_Resolution_Parameters)
	{
		return BuildWord(&D1Buffer[2]);
	}
	else
	{
		return 0x00;
	}
}
/*
*@description :获取Security Domain DNS Resolution Parameters中DNS IP Retry Policy Parameters的try counter值
*
*@see         :3.11  Security Domain DNS Resolution Parameters
*
*@function    :InitializeDNSIPTryCounter
*
*@param       :void
*
*@return      :DNS IP Retry Policy Parameters的try counter值
*
*@history     :v1.0--created by xuxiaojun@20211228
*/
WORD InitializeDNSIPTryCounter(void)
{
	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
    u8      X_A     D2Buffer[9];
    jref    X_A  	bufferRef;

	if(ValueBit(JcvmConfig_GetTlsExtandVersion(),BIT_GP__AMENDMENT__B__DNS_Version))	//gemalto dns
	{
		bufferFeild = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara); 
		
		ReadLFBuf(bufferFeild, HTTP_TriggerPara_RetryPara_Index, 2, D2Buffer, 2);
		
		return BuildWord(&D2Buffer[0]);
	}
	else
	{
		sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

		bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

		if(bufferRef == NULL_REF)
		{
			return 0x00;
		}

		bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

		ReadEeprom(D2Buffer, (bufferFeild + D2_OFFSET_DNS_D0_D3_buffer), sizeof(D2Buffer));

		if(D2Buffer[0] == TAG_D2_IN_DNS_Resolution_Parameters)
		{
			return BuildWord(&D2Buffer[2]);
		}
		else
		{
			return 0x00;	//当该tag不存在时初始化为00是否合适？或者说预置时是否需要预置一个初始值
		}
	}
}
/*
*@description :获取Security Domain DNS Resolution Parameters中RAS IP Retry Policy Parameters的try counter值
*
*@see         :3.11  Security Domain DNS Resolution Parameters
*
*@function    :InitializeRASIPTryCounter
*
*@param       :void
*
*@return      :RAS IP Retry Policy Parameters的try counter值
*
*@history     :v1.0--created by xuxiaojun@20211228
*/
WORD InitializeRASIPTryCounter(void)
{
	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
    u8      X_A     D3Buffer[9];
    jref    X_A  	bufferRef;

	if(ValueBit(JcvmConfig_GetTlsExtandVersion(),BIT_GP__AMENDMENT__B__DNS_Version))	//gemalto dns
	{
		bufferFeild = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara);

		ReadLFBuf(bufferFeild, HTTP_TriggerPara_RetryPara_Index, 2, D3Buffer, 2);
		
		return BuildWord(&D3Buffer[0]);
	}
	else
	{
		sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

		bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

		if(bufferRef == NULL_REF)
		{
			return 0x00;
		}

		bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

		ReadEeprom(D3Buffer, (bufferFeild + D3_OFFSET_DNS_D0_D3_buffer), sizeof(D3Buffer));

		if(D3Buffer[0] == TAG_D3_IN_DNS_Resolution_Parameters)
		{
			return BuildWord(&D3Buffer[2]);
		}
		else
		{
			return 0x00;	//当该tag不存在时初始化为00是否合适？或者说预置时是否需要预置一个初始值
		}
	}
}
/*
*@description :获取Retry Policy Parameters的try counter值
*
*@see         :3.8.3  Retry Policy Parameters
*
*@function    :InitializeSessionTryCounter
*
*@param       :void
*
*@return      :Retry Policy Parameters的try counter值
*
*@history     :v1.0--created by xuxiaojun@20211228
*/
WORD InitializeSessionTryCounter(void)
{
	UWORD xdata triggerParaFileAddr;
	BYTE idata retryCounter[2];

	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/1A03存储触发参数

	ReadLFBuf(triggerParaFileAddr, HTTP_TriggerPara_RetryPara_Index, 2, retryCounter, 2);

	return BuildWord(retryCounter);
}

/*
*@description: 当卡片支持DNS功能时，对SMS下行的http的触发参数进行处理
*@see        : <ts_102223v110200p.pdf> 6.6.27 OPEN CHANNEL描述了open channel必须存在的参数
*@function   : DNS_HTTPSAnalyseTriggerPara
*
*@param      : BYTE triggeredSDID        当前被触发的地址
*            : UWORD paraAddr            触发参数的起始地址，从起始地址处的格式为81LV
*
*@return     : TRUE表示参数解析成功；false表示参数解析失败
*
*@history    : jiangqingbin@xxxxxxxx
*
*NOTE        :
*/
BYTE DNS_HTTPSAnalyseTriggerPara(BYTE triggeredSDID, UWORD paraAddr)
{
	return FALSE;
}
/*
**@function   : findRASIPAddressInConnectionPara
*
*@description: 判断触发参数或者SD/ISD的配置参数Connection Parameters中是否存在RAS IP address
*
*@see        : <GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf> 3.3.1  Administration Session Start
*
*@param      : TRUE表示存在; FALSE表示不存在
*
*@return     : TRUE 表示找到84tag参数中的RAS IP address， FALSE表示找不到
*
*@history    : creat by jiangqingbin@20211231
*
*NOTE        :
*/
BYTE findRASIPAddressInConnectionPara(void)
{
	BYTE idata offset, connectionDataLen;
	
	connectionDataLen = GetOpenChannelParaForHttp(&RecData[0]);
	
	offset = FindTag(&RecData[0], 0x3E, connectionDataLen);
	
	if (offset != 0)
	{
		return TRUE;
	}
	
	return FALSE;
}
/*
openchannel：
Bearer description  											'35' or 'B5'
Buffer size 													'39' or 'B9'
Network Access Name  											'47' or 'C7'
Other address (local address)							        '3E' or 'BE'
Text String (User login)  								        '0D' or '8D'
Text String (User password)  							        '0D' or '8D'
UICC/terminal interface transport level  	                    '3C' or 'BC'
Data destination address  								        '3E' or 'BE'
Text Attribute  												'50' or 'D0'
Frame Identifier  												'68' or 'E8'
*/
/*
*@description:获取DNS开通道的相关参数
*@See        :102223
*@function   :GetOpenChannelParaForDNS
*
*@param      :BYTE *para      获取到的open channel相关参数，返回格式为TLV+TLV+TLV格式。文件中存的是什么，返回的就是什么
*
*@return     :void
*
*@history    :jiangqingbin@20211231
*/
BYTE GetOpenChannelParaForDNS(BYTE *para)
{
	UWORD xdata triggerParaFileAddr;
	WORD idata subLen;
	BYTE idata offSet;
	PN8		X_A		sdDataObj;
	jref 	I_A		bufferRef;
	memref	X_A		bufferAddr;
	memref	X_A  	bufferFeild;
	
	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/1A03存储触发参数

	subLen = ReadLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_DNSPara_Index, &para[0]);
	
	if (subLen == 0)	//触发参数中没带dns连接参数，此时获取默认的dns参数
	{
		sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

		bufferRef = sdData_getDNS_FA_bufferRef(sdDataObj);
		
		if(bufferRef == NULL_REF)
		{
			return 0x00;
		}

		bufferFeild = sdData_getDNS_FA_bufferField(sdDataObj);

		bufferAddr = locateObjectInMemory(bufferRef);

		subLen   = U16t8(get_array_length(bufferAddr));

		ReadEeprom(para, bufferFeild, subLen);
	}
	
	offSet = FindTag(&para[0], 0x3E, subLen);
	
	if (offSet == 0)	//不存在ip时到 dns-ip-list中读取
	{
		offSet = VMReadByte(G_DNSIPConnectStatus.ipAddress + 1);		//获取ip的L
		
		ReadEeprom(para+subLen, G_DNSIPConnectStatus.ipAddress, offSet+2);
		
		subLen += offSet+2;
	}
	
	return subLen;
}

/*
*@description:获取DNS开通道的相关参数
*@See        :102223
*@function   :GetOpenChannelParaForRAS
*
*@param      :BYTE *para      获取到的open channel相关参数，返回格式为TLV+TLV+TLV格式。文件中存的是什么，返回的就是什么
*
*@return     :void
*
*@history    :jiangqingbin@20220109
*/
BYTE GetOpenChannelParaForRAS(BYTE *para)
{
	UWORD xdata triggerParaFileAddr;
	WORD idata subLen;
	BYTE idata offSet;

	triggerParaFileAddr = SelectPrivateLFFile(DF_TLS, EF_HTTPTriggerPara);  //A100/1A03存储触发参数

	subLen = ReadLFFileValue(triggerParaFileAddr, HTTP_TriggerPara_ConnectionPara_Index, &para[0]); // 触发时已经查找过一编，并且将值写入的了

	offSet = FindTag(&para[0], 0x3E, subLen);

	if (offSet == 0)	//不存在ip时到 ras-ip-list中读取
	{
		offSet = VMReadByte(G_RASIPConnectStatus.ipAddress + 1);		//获取ip的L

		ReadEeprom(para + subLen, G_RASIPConnectStatus.ipAddress, offSet+2);

		subLen += (offSet+2);
	}

	return subLen;

}
/*
*@function   :IF_DNSByME_StartTimer
*
*@description:Fetch DNS IP List from ME过程中发生重试，需要开启timer
*
*@See        :Figure B-4:  Fetching DNS IP List
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20211229
*/
void DNSByME_StartTimer(void)
{
	BYTE xdata retry[9];

	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
	jref 	I_A		bufferRef;
	BIPModuleProcessStatus(); //20231010 葡萄牙  目前bip只支持一个，走到这里时可能前面有成功的open channel没有关闭，此时需要关闭。否则可能会出现start timer， 定时器终止后尝试open channel直接返回9000的情况

	ecpMem_Fill(retry, 0x00, sizeof(retry));

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	if(ValueBit(JcvmConfig_GetTlsExtandVersion(),BIT_GP__AMENDMENT__B__DNS_Version))	//gemalto dns
	{
		bufferFeild = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara);

		ReadLFBuf(bufferFeild, HTTP_TriggerPara_RetryPara_Index, 0, retry, sizeof(retry));
	}
	else
	{
		bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

		if (bufferRef == REF_NULL)
		{
			return;		//todo：此处获取 ME 的重试参数失败时 是否需要使用 http的86的重试参数
		}

		bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

		ReadEeprom(retry, (bufferFeild + D1_OFFSET_DNS_D0_D3_buffer), sizeof(retry));
	}

//	if(retry[0] == TAG_D1_IN_DNS_Resolution_Parameters)
//	{
		if((G_RASIPConnectStatus.tryCounter > 0) && ((retry[4] == 0x25)||(retry[4] == 0xA5)) && (retry[5] == 0x03))
		{
			IF_TimerModule_StartTimer(&G_DNSByMEConnectStatus.timerId, &retry[5]);
		}

		return;
//	}
//	else
//	{
//		return;
//	}
}
/*
*@function   :DNSByME_RetryFetchingDNSIPListSessionByTimerExpiration
*
*@description:DNS重试开户timer后，计时到之后的处理
*
*@See        :
*
*@param      :BYTE value  标记位的值
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20220102
*/
void DNSByME_RetryFetchingDNSIPListSessionByTimerExpiration(BYTE timerExpirationId)
{
	if(timerExpirationId != G_DNSByMEConnectStatus.timerId)   //只有timerID一致时才重启会话
	{
		return;
	}

	IF_TimerModule_ReleaseTimer(&G_DNSByMEConnectStatus.timerId);

	G_DNSByMEConnectStatus.resume = TRUE;

	G_DNSByMEConnectStatus.tryCounter--;

	DNSResolution_Main();
}
/*
*@function   :IF_DNSConnect_StartTimer
*
*@description:DNS IP Address 连接失败时，需要开启timer
*
*@See        :
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by jiangqingbin@20211231
*/
void DNSConnect_StartTimer(void)
{
	BYTE xdata retry[9];

	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
	jref 	I_A		bufferRef;
    BIPModuleProcessStatus(); //20231010 葡萄牙  目前bip只支持一个，走到这里时可能前面有成功的open channel没有关闭，此时需要关闭。否则可能会出现start timer， 定时器终止后尝试open channel直接返回9000的情况

	ecpMem_Fill(retry, 0x00, sizeof(retry));

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	if(ValueBit(JcvmConfig_GetTlsExtandVersion(),BIT_GP__AMENDMENT__B__DNS_Version))	//gemalto dns
	{
		bufferFeild = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara);

		ReadLFBuf(bufferFeild, HTTP_TriggerPara_RetryPara_Index, 0, retry, sizeof(retry));
	}
	else
	{
		bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

		if (bufferRef == REF_NULL)
		{
			return;		//todo：此处获取 DNS 的重试参数失败时 是否需要使用 http的86的重试参数
		}

		bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

		ReadEeprom(retry, (bufferFeild + D2_OFFSET_DNS_D0_D3_buffer), sizeof(retry));	//此处的偏移是否合理
	}

//	if(retry[0] == TAG_D2_IN_DNS_Resolution_Parameters)
//	{
		if((G_DNSIPConnectStatus.tryCounter > 0) && ((retry[4] == 0x25)||(retry[4] == 0xA5)) && (retry[5] == 0x03))
		{
			IF_TimerModule_StartTimer(&G_DNSIPConnectStatus.timerId, &retry[5]);
		}

		return;
//	}
//	else
//	{
//		return;
//	}
}
/*
*@function   :DNS_RetryDNSSessionByTimerExpiration
*
*@description:DNS重试开户timer后，计时到之后的处理
*
*@See        :
*
*@param      :BYTE value  标记位的值
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20220102
*/
void DNS_RetryDNSSessionByTimerExpiration(BYTE timerExpirationId)
{
	if(timerExpirationId != G_DNSIPConnectStatus.timerId)   //只有timerID一致时才重启会话
	{
		return;
	}

	IF_TimerModule_ReleaseTimer(&G_DNSIPConnectStatus.timerId);

	G_DNSIPConnectStatus.resume = TRUE;

	G_DNSIPConnectStatus.tryCounter--;

	DNSResolution_Main();
}
/*
*@function   :RAS_StartTimer
*
*@description:RAS IP Address连接失败时，需要开启timer
*
*@See        :
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20180725
*/
void RASConnect_StartTimer(void)
{
	BYTE xdata retry[9];

	PN8		X_A		sdDataObj;
	memref	X_A  	bufferFeild;
	jref 	I_A		bufferRef;

	BIPModuleProcessStatus(); //20231010 葡萄牙  目前bip只支持一个，走到这里时可能前面有成功的open channel没有关闭，此时需要关闭。否则可能会出现start timer， 定时器终止后尝试open channel直接返回9000的情况
	ecpMem_Fill(retry, 0x00, sizeof(retry));

	sdDataObj =	MEMtPN8(getSdDataObjBySdid(G_HTTPSStatus.TriggeredSD));

	if(ValueBit(JcvmConfig_GetTlsExtandVersion(),BIT_GP__AMENDMENT__B__DNS_Version))	//gemalto dns
	{
		bufferFeild = SelectPrivateLFFile(DF_TLS,EF_HTTPTriggerPara);

		ReadLFBuf(bufferFeild, HTTP_TriggerPara_RetryPara_Index, 0, retry, sizeof(retry));
	}
	else
	{
		bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

		if (bufferRef == REF_NULL)
		{
			return;		//todo：此处获取 RAS 的重试参数失败时 是否需要使用 http的86的重试参数
		}

		bufferFeild = sdData_getDNS_D0_D3_bufferField(sdDataObj);

		ReadEeprom(retry, (bufferFeild + D3_OFFSET_DNS_D0_D3_buffer), sizeof(retry));
	}

//	if(retry[0] == TAG_D3_IN_DNS_Resolution_Parameters)
//	{
		if((G_RASIPConnectStatus.tryCounter > 0) && ((retry[4] == 0x25)||(retry[4] == 0xA5)) && (retry[5] == 0x03))
		{
			IF_TimerModule_StartTimer(&G_RASIPConnectStatus.timerId, &retry[5]);
		}

		return;
//	}
//	else
//	{
//		return;
//	}

}
/*
*@function   :RAS_RetryHttpSessionByTimerExpiration
*
*@description:RAS重试开户timer后，计时到之后的处理
*
*@See        :
*
*@param      :BYTE value  标记位的值
*
*@return     :void
*
*@history    :V1.0  created by xuxiaojun@20220102
*/
void RAS_RetryHttpSessionByTimerExpiration(BYTE timerExpirationId)
{
	if(timerExpirationId != G_RASIPConnectStatus.timerId)   //只有timerID一致时才重启会话
	{
		return;
	}

	IF_TimerModule_ReleaseTimer(&G_RASIPConnectStatus.timerId);

	G_RASIPConnectStatus.resume = TRUE;

	G_RASIPConnectStatus.tryCounter--;

	DNSResolution_Main();
}

/*
*@function   :DNSResolutionParaCheck
*
*@description:在storedata的时候检查dns的子tag是否存在,
*				bit0为1表示D0/D1/D2/D3tag存在，bit1为1表示D6tag存在，bit2为1表示F7tag存在，bit3为1表示F9tag存在,bit4为1表示FAtag存在
*
*@See        :
*
*@param      :BYTE dnsTag  标记位的值
*
*@return     :返回对应tag存在时的bit位
*
*@history    :V1.0  created by jiangqingbin@20211229
*/
BYTE DNSResolutionParaCheck(BYTE dnsTag)
{
	switch (dnsTag)
	{
		case TAG_D0_IN_DNS_Resolution_Parameters:  //0xD0
			return 0x01;

		case TAG_D1_IN_DNS_Resolution_Parameters:  //0xD1
			return 0x01;

		case TAG_D2_IN_DNS_Resolution_Parameters:  //0xD2
			return 0x01;

		case TAG_D3_IN_DNS_Resolution_Parameters:  //0xD3
			return 0x01;

		case TAG_D6_IN_DNS_Resolution_Parameters:  //0xD6
			return 0x02;

		case TAG_F7_IN_DNS_Resolution_Parameters:  //0xF7
			return 0x04;

		case TAG_F9_IN_DNS_Resolution_Parameters:  //0xF9
			return 0x08;

		case TAG_FA_IN_DNS_Resolution_Parameters:  //0xFA
			return 0x10;
		
		case TAG_D8_IN_DNS_Resolution_Parameters:  //0xFA
			return 0x20;

		default:
			return 0x00;
	}
}

/*
*@function   :isIPCorrect
*
*@description:检查传入数据的ip地址是否是合法的IPv4/IPv6地址
*
*@See        :GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf 3.11.
*
*@param      :BYTE *dataBuffer	IP地址, 3E开头
*@param      :WORD subTagLen   传入ip的数据长度，可以是多个ip的TLVTLV...结构
*
*@return     :TRUE表示传入的数据是tlv结构的ip地址数据，FALSE表示不是
*
*@history    :V1.0----created by jiangqingbin @20211229            
*/
BYTE isIPCorrect(BYTE *dataBuffer, WORD subTagLen)
{
	WORD idata offSet;
	
	offSet = 0;
	
	while (offSet < subTagLen)
	{
		if ((dataBuffer[offSet] & 0x7F) != 0x3E)	//not ip tag
		{
			return FALSE;
		}
		
		if ((dataBuffer[offSet+1] != 0x05) && (dataBuffer[offSet+1] != 0x11))	//not ip length
		{
			return FALSE;
		}
		
		if ((dataBuffer[offSet+2] != 0x21) && (dataBuffer[offSet+2] != 0x57))	//not ipv4/ipv6
		{
			return FALSE;
		}
		
		offSet += (2+ dataBuffer[offSet+1]);
	}	

	return TRUE;
}

/*
*@function   :GPStoreSDDNSResolutionParaSub
*
*@description:处理GP附录B中通过store data指令存储Security Domain DNS Resolution Parameters 
*
*@See        :    《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf》 3.11.           
*
*@param      :sdDataObj
*@param      :tag          F4/F5LV 中的子tag: D0/D1/D2/D3/D6/F7/F9/FA
*@param      :dataBuffer   子tag的 TAG 起始下标    
*
*@return     :BYTE  返回子tag的 TLV 总长度
*
*@history    :V1.0----created by jiangqingbin @20211229            
*/
BYTE GPStoreSDDNSResolutionParaSub(PN8 sdDataObj, BYTE tag, BYTE * dataBuffer)
{
	WORD idata subTagLen, heapLen, tagRef, tmpRef, offset;
	PX8	 X_A   tempAmendBPtr;
	u16  I_A   index;
	u8	 X_A	tmpD0D3Buf[LENGTH_OF_DNS_D0_D3_buffer];		//临时存放D0D1D2D3数据的bug
	
	offset = 1;					//offset point to L
	
	subTagLen = dataBuffer[offset++];		
	
	if(subTagLen == 0x81)
	{				
		subTagLen = dataBuffer[offset++];
		
		heapLen = subTagLen + 3;			
	}
	else
	{	
		heapLen = subTagLen + 2;
	}
	
	switch(tag)
	{
		//D0/D1/D2/D3放在同一个位置存储
		case TAG_D0_IN_DNS_Resolution_Parameters:
		case TAG_D1_IN_DNS_Resolution_Parameters:
		case TAG_D2_IN_DNS_Resolution_Parameters:
		case TAG_D3_IN_DNS_Resolution_Parameters:
		{
			ecpMem_Fill(tmpD0D3Buf, 0x00, sizeof(tmpD0D3Buf));
			
			heapLen = 0;
			
			if(dataBuffer[heapLen] == TAG_D0_IN_DNS_Resolution_Parameters)
			{
				ecpMem_Copy(tmpD0D3Buf+D0_OFFSET_DNS_D0_D3_buffer, dataBuffer+heapLen, D1_OFFSET_DNS_D0_D3_buffer);
				
				heapLen += D1_OFFSET_DNS_D0_D3_buffer;				//heapLen point to next tag		
			}
			if(dataBuffer[heapLen] == TAG_D1_IN_DNS_Resolution_Parameters)
			{
				ecpMem_Copy(tmpD0D3Buf+D1_OFFSET_DNS_D0_D3_buffer, dataBuffer+heapLen, DNS_PARA_D1_D2_D3_PER_TLV_LENGTH);
				
				heapLen += DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;		//heapLen point to next tag
			}
			if(dataBuffer[heapLen] == TAG_D2_IN_DNS_Resolution_Parameters)
			{
				ecpMem_Copy(tmpD0D3Buf+D2_OFFSET_DNS_D0_D3_buffer, dataBuffer+heapLen, DNS_PARA_D1_D2_D3_PER_TLV_LENGTH);
				
				heapLen += DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;		//heapLen point to next tag
			}
			if(dataBuffer[heapLen] == TAG_D3_IN_DNS_Resolution_Parameters)
			{
				ecpMem_Copy(tmpD0D3Buf+D3_OFFSET_DNS_D0_D3_buffer, dataBuffer+heapLen, DNS_PARA_D1_D2_D3_PER_TLV_LENGTH);
				
				heapLen += DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;		//heapLen point to next tag
			}
			
			subTagLen = LENGTH_OF_DNS_D0_D3_buffer;					//D0D1D2D3 固定申请30字节的空间
			
			index = DNS_D0_D3_buffer_INDEX;

			offset = 0;		//D0/D1/D2/D3 存储格式是 TLVTLV...

			break;
		}
		case TAG_D6_IN_DNS_Resolution_Parameters:
		{
			index = DNS_D6_buffer_INDEX;

			break;
		}
		case TAG_F7_IN_DNS_Resolution_Parameters:
		{
			index = DNS_F7_buffer_INDEX;

			break;
		}
		case TAG_F9_IN_DNS_Resolution_Parameters:
		{
			index = DNS_F9_buffer_INDEX;

			break;
		}
		case TAG_FA_IN_DNS_Resolution_Parameters:
		{
			index = DNS_FA_buffer_INDEX;

			break;
		}

		case TAG_D8_IN_DNS_Resolution_Parameters:
		{
			index = DNS_D8_buffer_INDEX;
			break;
		}
		default: // 不识别tag的处理
			return heapLen;
	}
	
	tagRef =  nvmReadU16(get_object_field_ref(sdDataObj, index));

	if(tagRef != REF_NULL) //前面已经分配过，则需要把上一次分配的释放掉
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
		{
			bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
		}

	}
	
	if(heapLen == 2) //子tag的长度为0,eg: D0/D1/D2/D3 都不存在时
	{
		tmpRef = REF_NULL;
	}
	else
	{
		tmpRef	= _newarrayObject(AType_Byte, subTagLen, BIT_NEED_REFRESH);
		
		tempAmendBPtr = MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(tmpRef))));

		if ((index == DNS_F7_buffer_INDEX) || (index == DNS_F9_buffer_INDEX))	//check ras/dns ip list tlv format
		{
			if (isIPCorrect(&dataBuffer[offset], subTagLen) == FALSE)
			{
				return heapLen;
			}
		}
		
		if (index == DNS_D0_D3_buffer_INDEX)
		{
			nvmCopyAtomic(MEMtPM8(tempAmendBPtr),&tmpD0D3Buf[0],  U8t16(subTagLen));
		}
		else
		{
			nvmCopyAtomic(MEMtPM8(tempAmendBPtr),&dataBuffer[offset],  U8t16(subTagLen));
		}	
	}

	nvmWriteU16Atomic(get_object_field_ref(sdDataObj, index), tmpRef); //将新的引用写入

	return heapLen;
}

/*
*@function   :GPStoreSDDNSResolutionPara
*
*@description:处理GP附录B中通过store data指令存储Security Domain DNS Resolution Parameters 
*
*@See        :     《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf》 3.11.      
*
*@param      :sdDataObj         --- 需要被存储AnnexB参数的SDID
*@param      :BYTE * dataBuffer --- F4LV/F5LV， 假设这里的L是单字节      
*
*@return     :void
*
*@history    :V1.0----created by jiangqingbin @20211229            
*/
void GPStoreSDDNSResolutionPara(PN8 sdDataObj, BYTE * dataBuffer)
{
	BYTE idata tagExist, transActioinFlag, tag;
	WORD idata offset, heapLen;
	UWORD xdata length;
	
	tag = dataBuffer[0];
	
	offset = 1+ ASN1_GetBERTlvLen((UWORD)&dataBuffer[1], &length);//length = dataBuffer[1]; ，offset指向V
	
	transActioinFlag = FALSE;
	
	tagExist = 0; //bit0为1表示D0/D1/D2/D3tag存在，bit1为1表示D6tag存在，bit2为1表示F7tag存在，bit3为1表示F9tag存在,bit4为1表示FAtag存在,bit5为1表示D8tag存在
	
	if(length == 0) 
	{
		if(tag == 0xF5) // If a  sub - TLV is not present, no creation/ update shall be performed for the corresponding SD Administration Session Parameter
		{
			return;
		}
		
		transActioinFlag = IF_BeginTransaction(); //该处若采用事物写，则需要判断当前是否已开启了事物，如果没有开启，则开启一个新的事物
		
		sdData_setDNS_D0_D3_bufferRef(sdDataObj, REF_NULL);
		sdData_setDNS_D6_bufferRef(sdDataObj, REF_NULL);      // Parameters may be removed all at once using tag '85' with a null length
		sdData_setDNS_F7_bufferRef(sdDataObj, REF_NULL);
		sdData_setDNS_F9_bufferRef(sdDataObj, REF_NULL);
		sdData_setDNS_FA_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
		{
			bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
		}

		IF_CommitTransaction(transActioinFlag);
		
		return;		
	}	
	
	transActioinFlag = IF_BeginTransaction();

	length += offset;

	while(offset < length)
	{		
		tagExist |= DNSResolutionParaCheck(dataBuffer[offset]);
		
		heapLen = GPStoreSDDNSResolutionParaSub(sdDataObj, dataBuffer[offset], (BYTE*)&dataBuffer[offset]);

		offset += heapLen;
	}
		
	if(tag != 0xF5)
	{
		if((tagExist&0x01) == 0x00)  //D0/D1/D2/D3都缺失
		{
			sdData_setDNS_D0_D3_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x02) == 0x00)  //D6缺失
		{
			sdData_setDNS_D6_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x04) == 0x00)  //F7缺失
		{
			sdData_setDNS_F7_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x08) == 0x00)  //F9缺失
		{
			sdData_setDNS_F9_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x10) == 0x00)  //FA缺失
		{
			sdData_setDNS_FA_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
		
		if((tagExist&0x20) == 0x00)  //D8缺失
		{
			sdData_setDNS_D8_bufferRef(sdDataObj, REF_NULL);
#if (EUICC_ONLY||SE_AND_EUICC)
			if(bRamBuffer[PROFILE_APDU_MODE] == FALSE)
#endif
			{
				bRamBuffer[GC_Requested] = 1; // 如果在profile下载过程中，该标记被置上，则对应的store data指令执行完毕，会进入GC流程，死卡
			}
		}
	}

	IF_CommitTransaction(transActioinFlag);
}


/*
*@function   :GPGetSDDNSResolutioParaF4
*
*@description:处理GP 附录B中通过GET data指令获取Security Domain DNS Resolution Parameters , tag为0xF4
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf》 3.11       
*
*@param      :PN8 sdDataObj ---  SD 对象
*@param      :BYTE * dataBuffer---  获取到的响应数据，格式为F4LV
*            
*@return     :WORD --- dataBuffer中有效数据长度
*
*@history    :V1.0----created by jiangqingbin @20211229    
*/
WORD GPGetSDDNSResolutioParaF4(PN8 sdDataObj, BYTE * dataBuffer)
{
	u8 			arrayLen;
	jref 		bufferRef;
	memref		bufferFeild;
	memref 		bufferAddr;
	u8          tmp[5];
	u16         dataBufferOffset;
	u8          offset;
	u8          i;
	u8          tag;
	u8          dnsVersion;
	
	offset = 0;

	dataBufferOffset = 0;
	
	dnsVersion = ValueBit(JcvmConfig_GetTlsExtandVersion(), BIT_GP__AMENDMENT__B__DNS_Version);	//gemalto dns version

	for(i=0;i<5;i++)
	{
		switch(i)
		{
			case 0:
			{
				if (dnsVersion)
				{
					bufferRef = sdData_getDNS_D6_bufferRef(sdDataObj);
					
					tag = TAG_D6_IN_DNS_Resolution_Parameters;
				}
				else
				{
					bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);
				
					tag = TAG_D0_IN_DNS_Resolution_Parameters;
				}
				break;
			}
			case 1:
			{
				if (dnsVersion)
				{
					bufferRef = sdData_getDNS_F7_bufferRef(sdDataObj);
					
					tag = TAG_F7_IN_DNS_Resolution_Parameters;
				}
				else
				{
					bufferRef = sdData_getDNS_D6_bufferRef(sdDataObj);
				
					tag = TAG_D6_IN_DNS_Resolution_Parameters;
				}
				
				break;
			}
			case 2:
			{
				if (dnsVersion)
				{
					bufferRef = sdData_getDNS_D8_bufferRef(sdDataObj);
					
					tag = TAG_D8_IN_DNS_Resolution_Parameters;
				}
				else
				{
					bufferRef = sdData_getDNS_F7_bufferRef(sdDataObj);
				
					tag = TAG_F7_IN_DNS_Resolution_Parameters;
				}
				break;
			}
			case 3:
			{
				bufferRef = sdData_getDNS_F9_bufferRef(sdDataObj);
				
				tag = TAG_F9_IN_DNS_Resolution_Parameters;
				
				break;
			}
			case 4:
			{
				bufferRef = sdData_getDNS_FA_bufferRef(sdDataObj);
				
				tag = TAG_FA_IN_DNS_Resolution_Parameters;
				
				break;
			}

		}


		if(bufferRef != REF_NULL)
		{
			bufferAddr = locateObjectInMemory(bufferRef);

			arrayLen   = U16t8(get_array_length(bufferAddr));

			if (tag != TAG_D0_IN_DNS_Resolution_Parameters)
			{
				offset = ASN1_PaddingBERTlvTagAndLen(tmp, tag, arrayLen);		

				if(dataBuffer != NULL)
				{
					ecpMem_Copy(&dataBuffer[dataBufferOffset], tmp, offset);
				}
			}
			
			dataBufferOffset += offset;

			bufferFeild = get_array_data(MEMtPF8(bufferAddr));

			if(dataBuffer != NULL)
			{
				ecpMem_Copy(&dataBuffer[dataBufferOffset], bufferFeild, arrayLen);
				
				if (tag == TAG_D0_IN_DNS_Resolution_Parameters) 
				{
					offset = 0;
					
					if (dataBuffer[offset] == TAG_D0_IN_DNS_Resolution_Parameters)	//D0 存在
					{
						offset += D1_OFFSET_DNS_D0_D3_buffer;	
					}
					else
					{
						arrayLen -= D1_OFFSET_DNS_D0_D3_buffer;
						
						ecpMem_Move(&dataBuffer[offset], &dataBuffer[offset+D1_OFFSET_DNS_D0_D3_buffer], arrayLen-offset);	//数据往前移
					}
					
					if (dataBuffer[offset] == TAG_D1_IN_DNS_Resolution_Parameters)	//D1 存在
					{
						offset += DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;		
					}
					else
					{
						arrayLen -= DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;
						
						ecpMem_Move(&dataBuffer[offset], &dataBuffer[offset+DNS_PARA_D1_D2_D3_PER_TLV_LENGTH], arrayLen-offset);	//数据往前移
					}
					
					if (dataBuffer[offset] == TAG_D2_IN_DNS_Resolution_Parameters)	//D2 存在
					{
						offset += DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;		
					}
					else
					{
						arrayLen -= DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;
						
						ecpMem_Move(&dataBuffer[offset], &dataBuffer[offset+DNS_PARA_D1_D2_D3_PER_TLV_LENGTH], arrayLen-offset);	//数据往前移
					}
					
					if (dataBuffer[offset] == TAG_D3_IN_DNS_Resolution_Parameters)	//D3 存在
					{
						offset += DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;		
					}
					else
					{
						arrayLen -= DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;
						
						ecpMem_Move(&dataBuffer[offset], &dataBuffer[offset+DNS_PARA_D1_D2_D3_PER_TLV_LENGTH], arrayLen-offset);	//数据往前移
					}
					
					arrayLen = offset;
				}
			}

			dataBufferOffset += arrayLen;
		}

	}
	
	return dataBufferOffset;
}



/*
*@function   :GPGetSDDNSResolutioParaF5
*
*@description:处理GP 附录B中通过GET data指令获取Security Domain DNS Resolution Parameters, tag为0xF5
*
*@See        :《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf》3.11  
*
*@param      :PN8 sdDataObj ---  sd 的对象
*@param      :BYTE * dataBuffer---  既是输入也是输出，获取F5时，输入数据应该为5c01D0/5c01D1/5c01D2/5c01D3/5c01D6/5c01F7/5c01F9/5c01FA，返回时数据格式为 LV
*
*@return     :WORD --- dataBuffer中有效数据长度
*
*@history    :V1.0----created by jiangingbin @20211229
*/
WORD GPGetSDDNSResolutioParaF5(PN8 sdDataObj, BYTE * dataBuffer)
{
	u8 			arrayLen;
	jref 		bufferRef;
	memref		bufferFeild;
	memref 		bufferAddr;
	u8          tmp[5];
	u16         dataBufferOffset;
	u8          offset;
	u8          tag;
	u8          dnsD0D3[LENGTH_OF_DNS_D0_D3_buffer];
	
	offset = 0;
	dataBufferOffset = 0;
		
	if((P3 != 0x03) || (RecData[0] !=  0x5C) || (RecData[1] !=  0x01) ||
			((RecData[2] != 0xD0) && (RecData[2] != 0xD1) && (RecData[2] != 0xD2) && (RecData[2] != 0xD3) && (RecData[2] != 0xD6) && (RecData[2] != 0xD8) && (RecData[2] != 0xF7) && (RecData[2] != 0xF9)&& (RecData[2] != 0xFA)))
	{
		ThrowError(SW_WRONG_DATA);
	}
		
	switch(RecData[2])
	{
		case TAG_D0_IN_DNS_Resolution_Parameters:
		case TAG_D1_IN_DNS_Resolution_Parameters:
		case TAG_D2_IN_DNS_Resolution_Parameters:
		case TAG_D3_IN_DNS_Resolution_Parameters:
		{
			bufferRef = sdData_getDNS_D0_D3_bufferRef(sdDataObj);

			tag = TAG_D0_IN_DNS_Resolution_Parameters;

			break;
		}
		case TAG_D6_IN_DNS_Resolution_Parameters:
		{
			bufferRef = sdData_getDNS_D6_bufferRef(sdDataObj);

			tag = TAG_D6_IN_DNS_Resolution_Parameters;

			break;
		}
		case TAG_D8_IN_DNS_Resolution_Parameters:
		{
			bufferRef = sdData_getDNS_D8_bufferRef(sdDataObj);

			tag = TAG_D8_IN_DNS_Resolution_Parameters;

			break;
		}
		case TAG_F7_IN_DNS_Resolution_Parameters:
		{
			bufferRef = sdData_getDNS_F7_bufferRef(sdDataObj);

			tag = TAG_F7_IN_DNS_Resolution_Parameters;

			break;
		}
		case TAG_F9_IN_DNS_Resolution_Parameters:
		{
			bufferRef = sdData_getDNS_F9_bufferRef(sdDataObj);

			tag = TAG_F9_IN_DNS_Resolution_Parameters;

			break;
		}
		case TAG_FA_IN_DNS_Resolution_Parameters:
		{
			bufferRef = sdData_getDNS_FA_bufferRef(sdDataObj);

			tag = TAG_FA_IN_DNS_Resolution_Parameters;

			break;
		}
		// 不会出现非法值的情况
	}
	if(bufferRef != REF_NULL)
	{
		bufferAddr = locateObjectInMemory(bufferRef);

		arrayLen   = U16t8(get_array_length(bufferAddr));

		if (tag != TAG_D0_IN_DNS_Resolution_Parameters)
		{
			offset = ASN1_PaddingBERTlvTagAndLen(tmp, tag, arrayLen);

			ecpMem_Copy(&dataBuffer[dataBufferOffset], tmp, offset);
			
			dataBufferOffset += offset;
		}

		bufferFeild = get_array_data(MEMtPF8(bufferAddr));

		switch(RecData[2])
		{
			case TAG_D0_IN_DNS_Resolution_Parameters:
				bufferFeild += D0_OFFSET_DNS_D0_D3_buffer;
				arrayLen = D1_OFFSET_DNS_D0_D3_buffer;
				break;
			
			case TAG_D1_IN_DNS_Resolution_Parameters:
				bufferFeild += D1_OFFSET_DNS_D0_D3_buffer;
				arrayLen = DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;
				break;
			
			case TAG_D2_IN_DNS_Resolution_Parameters:
				bufferFeild += D2_OFFSET_DNS_D0_D3_buffer;
				arrayLen = DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;
				break;
			
			case TAG_D3_IN_DNS_Resolution_Parameters:
				bufferFeild += D3_OFFSET_DNS_D0_D3_buffer;
				arrayLen = DNS_PARA_D1_D2_D3_PER_TLV_LENGTH;
				break;
		}		
			
		ecpMem_Copy(&dataBuffer[dataBufferOffset], bufferFeild, arrayLen);
			
		dataBufferOffset += arrayLen;
				
	}
	else
	{
		dataBuffer[dataBufferOffset++] = RecData[2];
		dataBuffer[dataBufferOffset++] = 0x00;
	}
	
	return dataBufferOffset;
}


/*
*@function   :GPGetSDDNSResolutionPara
*
*@description:处理GP附录B中通过get data指令获取Security Domain DNS Resolution Parameters
*
*@see        :     《GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.12.pdf》 3.11.       
*
*@param      :sdDataObj         --- 输入，对应的SDID
*@param      :BYTE tag          --- 输入，0xF4或0xF5
*@param      :BYTE * dataBuffer --- 输出，获取到的响应数据
*
*@return     :WORD --- dataBuffer中的有效数据长度
*
*@history    :V1.0----created by jiangqingbin @20211230
*/
WORD GPGetSDDNSResolutionPara(PN8 sdDataObj, WORD tag, BYTE * dataBuffer)
{
	WORD idata len;
	
	if(tag == 0xF4)
	{
		len = GPGetSDDNSResolutioParaF4(sdDataObj, dataBuffer);
	}
	else
	{
		len = GPGetSDDNSResolutioParaF5(sdDataObj, dataBuffer);
	}
	
	return len;
}
/*
*@description :
*
*@see         :Figure B-1:  Administration Session started using DNS Resolution [MAIN]
*
*@function    :DNSResolution_Main
*
*@param       :void
*
*@return      :void
*
*@history     V1.0 :created by xuxiaojun@20220102
*/
void DNSResolution_Main(void)
{
	u8 idata dns1Result;
	u8 idata dnsQuerResult;
	u8 idata connectResult;
	u8 idata fetchingDNSIPListReslut;

	while(TRUE)
	{
		switch(G_DNSResolutionProcedureState.curState)
		{
			case DNSResolutionProcedureState_Fetch_DNS_IP_List_from_ME:
			{
				fetchingDNSIPListReslut = fetchingDNSIPList(); // 如果该函数内部执行了start timer的操作，这个函数如何退出？第一次进入该函数时，curState = DNSResolutionProcedureState_CONNECT_DNS_IP，函数内部可以将该值修改为DNSResolutionProcedureState_Fetch_DNS_IP_List_from_ME或者DNSResolutionProcedureState_END

				if(fetchingDNSIPListReslut == RETRY_POLICY)
				{
					return;
				}
				else if(fetchingDNSIPListReslut == UICC_STOP)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;
				}
				else if(fetchingDNSIPListReslut == OK_WITH_DNS_IP_List_Cache_Non_Empty)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_DNS_IP;
				}
				else // OK_WITH_DNS_IP_List_Cache_Empty
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;
				}

				break;
			}
			case DNSResolutionProcedureState_CONNECT_DNS_IP:
			{
				connectResult = DNSResolution_ConnectDNS(); //返回值可能为RETRY_POLICY、DNS_QUERIES、NOK_WITH_No_Suitable_Answer

				if(connectResult == RETRY_POLICY)
				{
					return;
				}
				else if(connectResult == DNS_QUERIES)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS_QUERIES;

					return;
				}
				else //if(connectResult == NOK_WITH_No_Suitable_Answer)
				{
					if(G_HTTPSStatus.RetryConterRemain > 0)
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS1;

						IF_AnnexB_StartTimerAfterUnNormalEnd();

						return;
					}
					else
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;

						break;
					}
				}

				break;
			}
			case DNSResolutionProcedureState_DNS_QUERIES:
			{
				dnsQuerResult = ProcessDNSResponse();

				if(dnsQuerResult == NOK_WITH_No_Suitable_Answer)
				{
					if(G_DNSIPConnectStatus.tryCounter > 0)
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_DNS_IP;

						DNSConnect_StartTimer();

						return;
					}
					else
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_DNS_IP;

						break;
					}
				}
				else if(dnsQuerResult == OK_Non_Empty_RAS_IP_List_Cache)
				{
					G_DNSResolutionProcedureState.performed = TRUE;

					G_DNSResolutionProcedureState.required = FALSE;

					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_RAS_IP;
				}
				else // dnsQuerResult == OK_Empty_RAS_IP_List_Cache
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END; // STOP
				}
				break;
			}
			case DNSResolutionProcedureState_DNS1:
			{
				dns1Result = DNSResolution_DNS1(); //DNSResolution_DNS1内部循环，为了方便直观的看到状态切换的过程 ，状态切换在DNSResolution_DNS1返回之后

				if(dns1Result == RETRY_POLICY) // 这种情况，状态已经在内部进行切换
				{
					return;
				}
				else if(dns1Result == DNS_QUERIES)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS_QUERIES;

					return;
				}
				else if(dns1Result == UICC_STOP)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;
				}
				else if(dns1Result == NOK_WITH_No_Suitable_Answer)
				{
					if(G_HTTPSStatus.RetryConterRemain > 0)
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS1;

						IF_AnnexB_StartTimerAfterUnNormalEnd();

						return;
					}
					else
					{
						G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;

						break;
					}
				}
				else if(dns1Result == OK_Empty_RAS_IP_List_Cache)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;
				}
				else// if(dns1Result == OK_Non_Empty_RAS_IP_List_Cache)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_CONNECT_RAS_IP;
				}
				break;
			}
			case DNSResolutionProcedureState_CONNECT_RAS_IP:
			{
				connectResult = DNSResolution_ConnectRAS(); //内部循环，且在内部进行状态切换

				if(connectResult == RETRY_POLICY)
				{
					return;
				}
				else if(connectResult == UICC_STOP)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_END;
					break;
				}
				else if(connectResult == OK_Non_Empty_RAS_IP_List_Cache)
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_ESTABLISH_TLS;
					break;
				}
				else if(connectResult == OK_Empty_RAS_IP_List_Cache) // 这种情况直接执行DNS1，而不需要session try counter减1
				{
					G_DNSResolutionProcedureState.curState = DNSResolutionProcedureState_DNS1;
					break;
				}
				break;
			}
			case DNSResolutionProcedureState_ESTABLISH_TLS:
			{
				TLSInitVariable();

				G_TLSSecurityParameters.tlsRecordState = TLS_SEND_CLIENT_HELLO_STATE;

				TLSHandProcess(0, 0); //该函数内部TLS握手成功之后会将状态切换到DNSResolutionProcedureState_HTTP_EXCHANGE

				return;
			}
			case DNSResolutionProcedureState_HTTP_EXCHANGE: // 进入TLSHandProcess循环之后，就无法再进入这里了
			{
				break;
			}
			case DNSResolutionProcedureState_END:
			{
				DNSResolutionInitVariable();
				return;
			}
			default:
				return;
		}
	}
}
#endif
