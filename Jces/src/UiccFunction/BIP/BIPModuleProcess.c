/************************************************************************************************************
* Copyright (C), 2014, Eastcompeace.Co.,Ltd.(www.eastcompeace.com).
* All rights reserved.
*
* File name:   BIPModuleProcess.c
*
* Description: 该文件主要用于存放BIP相关的文件
*
* auther：     xxxx    xxxx.xx.xx
*
* history：    V1.0:该文件目前的方案约定对于COS层的BIP 业务数据，一个时刻只允许支持一个业务开通通道，即OTA、CATTP、HTTP 的业务不可以同时打开通道

				    要考虑标记在卡片复位时的清除。即6EA7文件中的标记清除
*
************************************************************************************************************/
#include "BIPModuleProcess.h"
#include "stkhead.h"
#include "gsmpublic.h"
#include "fileAction.h"
//#include "Main.h"
#include "HttpProcess.h"
#include "Interface_AnnexB.h"
#include "TlsMainFlow.h"
#include "filesystem.h"
//#include "JCREMain.h"

#if UICC__FUNCTION__CATTP
#include "Interface_CATTP.h"
#endif

#if GP__AMENDMENT__B__DNS_Resolution
#include "Interface_DNS.h"
#endif
#if JAVA_CARD_UICC
#include "Stkalert.h"
#endif

BIPModuleStatus xdata G_BIPModuleStatus;   //标示当前通道是否打开、当前通道打开上待处理的业务类型 
WORD xdata G_BIPModule_Receive_Length;
WORD xdata G_BIPModule_Send_Length;

#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
//0x1D10 = 0x800+0x800+0x800+0x410+0x100 (分别对应 BIPModule_Receive_SIZE+BIPModule_Send_SIZE+HTTP_CommandData_SIZE+HTTPSResponseData_SIZE+HTTPNextURIPara_SIZE)
BYTE xdata G_BIPModule_Session_Data_Buf[0x1D10];	
#endif

#if UICC__FUNCTION__CATTP
BYTE xdata G_BIPDataAvaiableEvent;     //当前是否收到了dataAvalable事件
#endif


/*
*@description: 对于COS 应用支持BIP 的业务场景，对BIP 的相关变量进行初始化.该初始化操作考虑在开机复位、通道关闭等场景下调用
*
*@function   : BIPModuleInit
*
*@param       :void
*
*@return      :void
*
*@author      :lx
*
*@hisotry     :V1.0 ：create @20180411
*/
void BIPModuleInit(void)
{	
	ecpMem_Set((BYTE *)&G_BIPModuleStatus, 0x00, sizeof(G_BIPModuleStatus)); //初始化全局变量	
#if UICC__FUNCTION__CATTP
	G_BIPDataAvaiableEvent = FALSE;
#endif
}
/*
*@description::https会话开启时，检查https的openchannel参数中的必选的tag是否都具有
*@See         :ts_101220v110100p.pdf / 7.2  Assigned TLV tag values
*@function   :BIPModuleOpenChannelMandatoryTagCompalete
*
*@param      :BYTE *srcBuf	openchannel的参数 tlv...tlv格式
*@param      ：BYTE scrLen  openchannel数组的长度
*
*@return     :true 标识所有必选tag都存在， 反之是不存在， 目前实现的方案是必选的tag是：35,39,3c，3e (102223规范只说明35和39是必选， 测试部要求判断3c和3e也必须存在)
*
*@history    :creat by Jiangqb@20200722
*/
BYTE BIPModuleOpenChannelMandatoryTagCompalete(BYTE *srcBuf, BYTE scrLen)
{
	if(FALSE == FindTag(srcBuf, 0x35, scrLen)) //Bearer description tag
	{
		return FALSE;
	}

	if(FALSE == FindTag(srcBuf, 0x39, scrLen))//Buffer size tag
	{
		return FALSE;
	}

	if(FALSE == FindTag(srcBuf, 0x3c, scrLen))//UICC/terminal interface transport level tag
	{
		return FALSE;
	}

	if(FALSE == FindTag(srcBuf, 0x3e, scrLen))//Other address (data destination address) tag
	{
		return FALSE;
	}

	return TRUE;
}
/*
*@descritpion: 根据指定的参数进行开通道操作，通道打开成功后，对G_BIPModuleStatus中的变量进行赋值；如果当前有正在打开的通道，则不允许重新开通通道
*
*@function   : BIPModuleOpenChannel
*
*@param      : BYTE appType :当前期望开通的BIP 业务类型
*
*@return      :CMD_Success:通道开通成功/ other:通道开通失败
*
*@author      :lx
*
*@history     :V1.0 ：create @20180412
*             :V1.1 modify by xuxiaojun@20180530   通过接口获取开通道的相关参数，进入该函数时，假设所有的开通道参数都是合法的
*             :V1.2 modify by xuxiaojun@20200622   不知道之前的代码为什么修改，暂时还原成之前的版本
*             :V1.3 modify by xuxiaojun@20220309   merged from EPX产业化\STM_ST33G1M2\Branch\RSP\code, 支持DNS功能
	OPEN CHANNEL related to packet data service bearer：
	Bearer description  											'35' or 'B5' 	M
	Buffer size 													'39' or 'B9' 	M
	Network Access Name  											'47' or 'C7'  	O
	Other address (local address)									'3E' or 'BE' 	O
	Text String (User login)  										'0D' or '8D' 	O
	Text String (User password)  									'0D' or '8D' 	O
	UICC/terminal interface transport level  						'3C' or 'BC'	O	//PORT  端口号的值转换到16进制  如;端口号 29173  -->  0x71F5  --> BC 03 02 71F5  (02指类型是端口号)
	Data destination address  										'3E' or 'BE' 	C	//IP地址 IP地址转为16进制 如IP:103.7.64.150 --> 0x67 07 40 96  -->  3E 05 2167074096 (21指ip地址)
	Text Attribute  												'50' or 'D0' 	C
	Frame Identifier  												'68' or 'E8' 	O
	
*/
BYTE BIPModuleOpenChannel(BYTE appType)
{
	BYTE idata result, tag, lenOfUserLogin;		
	WORD idata len;

	len = 0;
	
	if(G_BIPModuleStatus.channelDataType != ChannelDataType_NULL)//表示当前bip通道打开，约定卡端一个时刻只能打开一个通道
	{
		return ExitStkPerform;
	}
	
	InitProactiveCommand(CMD_OPENCHANNEL,0x01,DEV_ME); //开通道	  CATTP 要01?
	
	switch(appType)
	{
		case ChannelDataType_HTTPS:
		{
			len = IF_AnnexB_GetOpenChannelParaForHttp((BYTE *)&RecData[0]);
			
			if (len == 0x00)
			{
				return ExitStkPerform;
			}
			
			break;
		}
#if UICC__FUNCTION__CATTP
		case ChannelDataType_CATTP:
		{ 		
			len = IF_GetOpenChannelParaForCATTP((BYTE *)&RecData[0]);
			
			if(len == 0x00)
			{
				return ExitStkPerform;
			}
			break;
		}
#endif
#if GP__AMENDMENT__B__DNS_Resolution
		case ChannelDataType_DNS:
		{
			len = IF_DNS_GetOpenChannelPara((BYTE *)&RecData[0]);

			break;
		}

		case ChannelDataType_RAS://该case不可以合并到ChannelDataType_HTTPS
		{
			IF_AnnexB_TLSVariableInit();

			len = IF_RAS_GetOpenChannelPara((BYTE *)&RecData[0]);

			break;
		}
#endif
		case ChannelDataType_OTA:
		{
			return ExitStkPerform;
			//break;
		}
		default : 			  //此处为了白盒测试添加
			return ExitStkPerform;
	}
	
	if (!BIPModuleOpenChannelMandatoryTagCompalete(RecData, len))		//检查https开通到时的必选tag是否都存在
	{
		return ExitStkPerform;
	}

	tag = 0x81;
	
	//移动15所测试需求openchannel时子tag的顺序参考规范
	while(tag != 0)
	{
		result = FindTag(&RecData[0], tag, len);

		if ((result != 0) && (tag != 0x81) && (tag != 0x82))
		{
			AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[result-1]);  	//假设所有的T都是单字节，所有的L都是单字节
			
			if (tag == 0x0D)						//假设两个0Dtag(User login 和 User password), 是紧接的，如果不是则当前方案有bug
			{
				lenOfUserLogin = RecData[result];
				
				if ((RecData[result+1+lenOfUserLogin]&0x7F) == 0x0D)		
				{
					AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[result+1+lenOfUserLogin]);  	//假设所有的T都是单字节，所有的L都是单字节
				}
			}
		}
		
		switch(tag)
		{
			case 0x81:					//移动https测试，下行短信中有可能包含有 81(命令细节) tag
				tag = 0x82;
			
				if (result != 0)
				{
					ecpMem_Copy(ResData+3, RecData+result-1, 5);
				}
				
				break;
			case 0x82:					//移动https测试，下行短信中有可能包含有 82(设备标识) tag
				tag = 0x85;
			
				if (result != 0)
				{
					ecpMem_Copy(ResData+8, RecData+result-1, 4);
				}
				
				break;
			case 0x85:					//移动测试要求有85tag
				tag = 0x35;
				break;
			case 0x35:					//Bearer description  											'35' or 'B5' 
				tag = 0x39;
				break;
			case 0x39:					//Buffer size 													'39' or 'B9' 
				tag = 0x47;
				break;
			case 0x47:					//Network Access Name  											'47' or 'C7'
				tag = 0x0D;
				break;
			case 0x0D:					//Text String (User login) /Text String (User password) 		'0D' or '8D' 
				tag = 0x3C;
				break;
			case 0x3C:					//UICC/terminal interface transport level  						'3C' or 'BC'		//PORT  端口号的值转换到16进制  如;端口号 29173  -->  0x71F5  --> BC 03 0271F5  (02指端口号)
				tag = 0x3E;
				break;
			case 0x3E:					//Data destination address  									'3E' or 'BE' 		//IP地址 IP地址转为16进制 如IP:103.7.64.150 --> 0x67 07 40 96  -->  3E 05 2167074096 (21指ip地址)
				tag = 0x50;
				break;
			case 0x50:					//Text Attribute  												'50' or 'D0' 
				tag = 0x68;				
				break;
			case 0x68:					//Frame Identifier  											'68' or 'E8' 
				tag = 0x00;				
				break;	

			default:
				return ExitStkPerform;
		}			
	}

	result = SendProactiveCommand();
	
	if(result == CMD_Success)
	{
		result = FindTag(&RecData[0], TLV_CHANNELSTATUS, P3);
		
		if((result > 0) && ((RecData[result+1] & 0x80) == 0x80))
		{
			G_BIPModuleStatus.channelNum = (RecData[result+1] & 0x07); //取channel ID
			
			G_BIPModuleStatus.channelDataType = appType;
			
			//:modify by jiangqingbin@20221206 张永林根据《tpx/epx 平台中所有涉及到 http 流程中的疑问讨论会议》做出的扩展, 根据规范《ts_102223v150300p.pdf》8.6节, bip的发送数据时, 需要判断终端中的缓存数据大小是否会越界bipBufferSize, 并由此确定senddata的Command Qualifier字节的值, 脚本(移动5G内测脚本/BugList/BugList_12_01/02)
			result = FindTag(&RecData[0], TLV_BUFFERSIZE, P3);

			if (result != 0)
			{
				G_BIPModuleStatus.channelBufferSize = BuildWord(&RecData[result+1]);
			}
			else
			{
				return ExitStkPerform;
			}

			G_BIPModule_Send_Length = 0x00; //ClearFileFirstWord(DF_BIPModule,EF_BIPModule_Send);

			G_BIPModule_Receive_Length = 0x00; //ClearFileFirstWord(DF_BIPModule,EF_BIPModule_Receive);
		
			switch(appType)
			{
				case ChannelDataType_HTTPS:
				{
					IF_AnnexB_TLSVariableInit();
					
					break;
				}
				case ChannelDataType_CATTP:
				{
// 					IF_CATTP_ariableInit();
					
					break;
				}
				case ChannelDataType_OTA:
				{
					break;
				}
				default : 			  //此处为了白盒测试添加
					break;
			}
			return CMD_Success;
		}
		
		return ExitStkPerform;	//openchannel响应数据没有通道号时判断为执行失败
	}
	else
	{
		//函数 SendProactiveCommand 内部可能会对其非 CMD_Success 的 TLV_RESULT 值进行另外赋值，所以此处需要重新获取该真实的TLV_RESULT响应值
		//异常测试脚本用例：电信备案测试：T_026_HTTP_RFM_FirstOpenChannelFail_1X_Retry
		result = FindTag(&RecData[0],TLV_RESULT,P3);
	
		if (result == FALSE)
		{
			return ExitStkPerform;
		}
		else
		{
			return RecData[result+1];
		}
	}
}

/*
*@descritpion: 将存放于EF_BIPModule_Send中的响应数据在打开bip模块中打开的通道ID上发送出去
*
*@function   : BIPModuleSendData
*
*@param      : BYTE mode     --- 标识待发送数据是在RAM中还是在E2中
*@param      : BYTE* dataBuf --- 待发送数据，只有mode为RAM时才有效, 其中dataBuf[0,1]表示待发送数据的长度
*
*@return     :是否成功发送
*
*@author     :lx
*
*@history    :V1.0 :create @20180411
*            :V1.1 :modify by xuxiaojun@20190610  修改一次BIP SEND的最大长度为0xFF(dataLength = 240), 因为电信测试时，测试工具不支持9100的解析。
*			 :v1.2 :modify by jiangqb@20191230 为节省代码空间，优化整合 函数BIPModuleSendResponseData和函数BIPModuleSendHttpHandshake
*			 ：v1.3 :modify by jiangqb@20200424 增加发数据前判断当前通道是否开启
*			 ：v1.4 :modify by jiangqingbin@20221206 张永林根据《tpx/epx 平台中所有涉及到 http 流程中的疑问讨论会议》做出的扩展, 根据规范《ts_102223v150300p.pdf》8.6节, 
*						bip的发送数据时, 需要判断终端中的缓存数据大小是否会越界bipBufferSize, 并由此确定senddata的Command Qualifier字节的值, 脚本(移动5G内测脚本/BugList/BugList_12_01/02)
*/
BYTE BIPModuleSendResponseData(BYTE mode, BYTE* dataBuf)
{
	WORD idata sendLength; //待发送数据的长度	
	WORD idata sendOffset;			
	UWORD xdata bipSendAddr;
	WORD idata bipBufferSize;
	WORD idata nextSendDataLength;
	
	BYTE idata result, mode2;
	
	if(G_BIPModuleStatus.channelDataType == ChannelDataType_NULL)		//通道被关闭
	{
		return CMD_MISSINGINFORMATION;
	}

#define dataLength 240
	mode2 = mode;
	
	bipBufferSize = 0;
//	bipSendAddr = SelectPrivateTFFile(DF_BIPModule,EF_BIPModule_Send);
	bipSendAddr = (UWORD)BIPModule_Send_ADDR_START;
	
	if (mode2 == DataInEep)
	{
//		sendLength = ReadTFFileWord(bipSendAddr);
		sendLength = G_BIPModule_Send_Length;
	}
	else
	{
		sendLength = BuildWord(dataBuf);
		
		if (sendLength > dataLength)		//一次senddata发送不完，分两次以上，此时把数据写入文件，防止重入时冲掉特tempBuf
		{
			mode2 = DataInEep;
		
//			WriteTFFileValue(bipSendAddr, 2, dataBuf+2, sendLength, sendLength);
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
			ecpMem_Copy(bipSendAddr+2, &dataBuf[2], sendLength);
#else
            WriteEepromWithoutProtect(bipSendAddr+2, &dataBuf[2], sendLength);
#endif
			G_BIPModule_Send_Length = sendLength;
		}		
	}
	
	sendOffset = 0x02;	
		
	do
	{
		InitProactiveCommand(CMD_SENDDATA, 0x01, (BYTE)(G_BIPModuleStatus.channelNum | 0x20));
					
		RecData[0] = TLV_CHANNELDATA;
			
		RecData[1] = ((sendLength > dataLength)?dataLength:sendLength);
		
 		if (mode2 == DataInEep)
 		{
 			//ReadTFBuf(bipSendAddr, sendOffset, &RecData[2], RecData[1]);
 			ReadEeprom(&RecData[2], bipSendAddr + sendOffset, RecData[1]);
 		}
 		else
 		{
 			ecpMem_Copy(&RecData[2], dataBuf+sendOffset, RecData[1]);
 		}
		
		AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[0]);

		sendOffset += RecData[1];
		
		sendLength -= RecData[1];

		if(sendLength > 0)
		{
			bipBufferSize += RecData[1];

			nextSendDataLength = ((sendLength > dataLength)?dataLength:sendLength);

			if ((bipBufferSize+nextSendDataLength) > G_BIPModuleStatus.channelBufferSize)
			{
				//加上下一次待发送的数据长度nextSendDataLength将会超过bip的缓存, 此处需要先发一次立刻发送的标记，终端将立刻向服务器发送bip缓存中的数据,参考《ts_102223.pdf》8.6-Command details
				bipBufferSize = 0;
			}
			else
			{
				ResData[7] = 0;		//中间块, 此时数据发到终端中, 终端不会立刻外发数据，而是缓存在手机
			}
		}
			
		result = SendProactiveCommand();			//CMD_Success
		
		if (result != CMD_Success)
		{
			#undef  dataLength
			return result;
		}
									 
	}while (sendLength > 0x00);	    

//	SetTFConst(bipSendAddr, 0x00, 0x00, 0x02); //清空发送缓存的length

#undef  dataLength
	
	return result;
}

BYTE BIPModuleTransB7ToMaxLen(BYTE B7Len)
{
	return ((B7Len > 0xED)?0xED:B7Len);
}

/*
*@descritpion:在收到C2的DataAaliable(D6) event后，进行 BIP 数据的接收;在数据接收完成后，根据不同的业务类型，分发给不同的业务去处理
*
*@function   :BIPModuleProcessDataAvaiable
*
*@param      :void
*
*@return     :void
*
*@history    :V1.0 ：create @20180411
*            :V1.1 : modify by xuxiaojun@20180707 在收到C2的DataAvailable(D6) event后，下来一次14数据就处理一次，尽量减少BIP 接收能力的限制。 印度rjio测试卡，通过D6事件一次下来30K的数据
*            :V1.2 : modify by xuxiaojun220180716 优化BIP处理速度，如果BIP Receive文件的剩余空间足够容纳下一次要接收的数据，则继续接收；否则，处理之后再接收
*            :V1.3 : modify by xuxiaojun@20180901 增加参数，用于表示当前处理的D6事件所指示的3701XX中XX的值。收到D6事件之后，打标记BIPModule_DataAvaiable_Alert时，标记的第2个字节用来表示当前D6指令所指示的待接收数据长度3701XX的XX. 主要是用于处理D6过程中又来一个D6事件 <201808311731_LYF LS 5013_Launch Browser(PUSH_Download)_Success_No PoR at Server_No Refresh_Latest Solution.txt>
*            :V1.4 : modify by jiangqingbin@20210508 增加配置，bip会话中下一次要接收的数据大于0xed时，是否需要强制转换为只接收0xed的长度
*            :V1.5 : modify by xuxiaojun@20220309   merged from EPX产业化\STM_ST33G1M2\Branch\RSP\code, 支持DNS功能
*/
void BIPModuleProcessDataAvaiable(BYTE len)
{
	BYTE idata result,channelDataLen, flag, channelValidDataLen ;
	WORD idata offset, bipReveiveSize;
	UWORD xdata bipReceiveAddr;
	BYTE idata readTag;
	
	if(G_BIPModuleStatus.channelDataType == ChannelDataType_NULL)		//通道被关闭
	{
		return;
	}
	
	flag = 0;
	
//	bipReceiveAddr = SelectPrivateTFFile(DF_BIPModule, EF_BIPModule_Receive);
	bipReceiveAddr = (UWORD)BIPModule_Receive_ADDR_START;
	
//	bipReveiveSize = GetTFSize(bipReceiveAddr);
	bipReveiveSize = BIPModule_Receive_SIZE;
	
	channelDataLen = len;  //该全局变量的值是在收到C2的D6事件的处理时进行赋值，后续处理会根据14指令的B7 TAG来指示是否还有后续数据，每接收一次14则处理一次
	
	channelValidDataLen = BIPModuleTransB7ToMaxLen(channelDataLen);

	if((channelDataLen > 0xED)&&(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Bip_ReceiveData_ProactiveCommand_B7_Transfer))) //0表示不需要转换14中的B7所指示的长度，1表示需要转换
	{
		channelDataLen = 0xED;
	}
	
// 	//中国电信http测试脚本需要返回格式  期望响应值: D00C810301420082028121X701FF
// 	if(channelDataLen > 0xED) //理论上来说，这个值可以随意，但是该值+必要的TAG之后不能大于0xFF
// 	{
// 		channelDataLen = 0xED;
// 	}

	offset = 0;

	ecpMem_Set(tempBuf, 0x00, 2);

	readTag = TRUE;	
	
	while(channelDataLen > 0x00)
	{
		if(readTag == TRUE)
		{
			offset = G_BIPModule_Receive_Length;
		}
		
		offset += 0x02; //空出接收缓存中第一个L的位置
		
		InitProactiveCommand(CMD_RECEIVEDATA, 0x00, (BYTE)(G_BIPModuleStatus.channelNum | 0x20));

		RecData[0] = TLV_CHANNELDATALENGTH;

		RecData[1] = 0x01;

		RecData[2] = channelDataLen;

		AddTLV((BYTE *)&ResData[1], (BYTE *)&RecData[0]);
	
		result = SendProactiveCommand(); //发送主动式命令接收数据		//receive data
		
		if(result == CMD_Success)
		{
			result = FindTag(&RecData[0], TLV_CHANNELDATA, P3); //找到data的TAG
			
			if (result > 0x00)		//TLSSecurityParameters.serverSeqNum < 1 是握手信息，加快速度不用写文件
			{
				if (IF_HTTPS_HandShake() && (flag == 0))//if((G_TLSSecurityParameters.serverSeqNum < 1) && (G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS))			//handshake， 并且默认握手期间的数据小于 sizeof(tempBuf)
				{					
					SetWord(&tempBuf[0], RecData[result]);
					
					ecpMem_Move(tempBuf+2, RecData+result+1, RecData[result]);
					
					offset = 2;
				}
				else
				{
//					WriteTFBuf(bipReceiveAddr, offset, &RecData[result+1], RecData[result]); //将data拷贝至接收数据缓存		
					if((offset + RecData[result]) <= BIPModule_Receive_SIZE)
					{
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
						ecpMem_Copy(bipReceiveAddr + offset ,  &RecData[result+1],RecData[result]);
#else
						WriteEepromWithoutProtect(bipReceiveAddr + offset ,  &RecData[result+1],RecData[result]);
#endif
					}
					else
					{
						return;
					}
				}				
				
				offset += RecData[result];			
			}
			
			result = FindTag(&RecData[0], TLV_CHANNELDATALENGTH, P3); //找到下次需要接收数据的长度的TAG
			
			if(result > 0x00)
			{
				channelDataLen = RecData[result+1];
				
				if ((IF_HTTPS_HandShake()) && (channelDataLen > 0) && (flag == 0))
				{
					flag = 1;
					
//					WriteTFBuf(bipReceiveAddr, 2, tempBuf+2, BuildWord(tempBuf));
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
					ecpMem_Copy(bipReceiveAddr + 2 ,  tempBuf+2, BuildWord(tempBuf));
#else
					WriteEepromWithoutProtect(bipReceiveAddr + 2 ,  tempBuf+2, BuildWord(tempBuf));
#endif
				}
				
				if((channelDataLen > 0xED)&&(ValueBit(JcvmConfig_GetAmendmentBConfig(), BIT_GP__AMENDMENT__B__Bip_ReceiveData_ProactiveCommand_B7_Transfer))) //0表示不需要转换14中的B7所指示的长度，1表示需要转换
				{
					channelDataLen = 0xED;
				}
			}
			else
			{
				channelDataLen = 0;
			}

			channelValidDataLen = BIPModuleTransB7ToMaxLen(channelDataLen);
	
			if (offset >= 0x02)    //考虑到BIP接收能力问题，这里采用收到一条处理一条的方式，处理当前通过14接收到的数据
			{	
				offset -= 0x02;							
			}

			if((offset < (bipReveiveSize - 2 - channelValidDataLen)) && (channelValidDataLen > 0))
			{
				readTag = FALSE;
				continue;
			}

			if (offset >0x00)    
			{					
				if ((IF_HTTPS_HandShake()) && (flag == 0))//
				{
					//表示握手期间不写文件
				}
				else
				{
					G_BIPModule_Receive_Length = offset;
					readTag = TRUE;
				}		
			}
			
			switch(G_BIPModuleStatus.channelDataType) //所有卡片上的应用的BIP使用同一个缓存文件收取bip数据，以下 业务类型要给对应的C应用程序去处理，其他的则按照以前的业务逻辑继续处理
			{
#if GP__AMENDMENT__B__FOR_MASK
 				case ChannelDataType_HTTPS: //业务类型为HTTP
					
					if ((IF_HTTPS_HandShake()) && (flag == 1))
					{	
						if (offset < sizeof(tempBuf))
						{
							SetWord(&tempBuf[0], offset);
							
//							ReadTFBuf(bipReceiveAddr, 2, tempBuf+2, offset);
							ReadEeprom(tempBuf+2, bipReceiveAddr + 0x02, offset);
							
//							ClearFileFirstWord(DF_BIPModule, EF_BIPModule_Receive);
							G_BIPModule_Receive_Length = 0x00;
						}
						else
						{
							return;	//握手接收到的数据大于tempBuf，暂时做成无法处理
						}
					}
				
 					IF_HTTPDataAvaiable(channelValidDataLen, offset);
					
 					break;
#endif

#if UICC__FUNCTION__CATTP
				case ChannelDataType_CATTP: //业务类型为CATTP
					IF_CATTPDataAvaiable(offset);
					break;
#endif
				case ChannelDataType_OTA: //业务类型为OTA
					break;
#if GP__AMENDMENT__B__DNS_Resolution
				case ChannelDataType_DNS:
				{
					IF_DNSDataAvaiable(channelDataLen, offset); //此时处理的是DNS Queries的下行数据
					break;
				}
#endif				
				default: //白盒测试添加
					break;
			}
			
		}
		else
		{
				//联通天谕测试， 机制测试\RetryPolicy--TestScript_N001 期望在发送receive失败后关闭通道并开启timer
				IF_AnnexB_ChannelStatus();
			offset = 0x00;
			
			break;
		}
	}
}

/*
*@description:处理通道状态下行事件函数.如关闭当前打开的通道等动作
*
*@function   :BIPModuleProcessStatus
*
*@param      :void
*
*
*@Returns    :void
*
* @author 	 :lx
*
*@history    :V1.0 ：create @20180411
*            :V1.0.1: modify by xuxiaojun@20180829 增加手机兼容性测试，印度RJIO测试卡20180827122212_LYF LS5013_RAM+RFM_APM ON during BIP session and APM OFF after Retry Exhaust(PULL_Activation).txt。手机飞行模式打开之后，卡片发送的close channel的状态为3A，但是后须要求通道打开成功。所以这里需要兼容，将BIP相关信息清掉
*
*/
void BIPModuleProcessStatus(void)
{		
	if(G_BIPModuleStatus.channelDataType == ChannelDataType_NULL)		//通道被关闭
	{
		return;
	}

	InitProactiveCommand(CMD_CLOSECHANNEL, 0x00, (BYTE)(G_BIPModuleStatus.channelNum | 0x20)); //关闭通道
	
	SendProactiveCommand();	
	
	BIPModuleInit();
	
	return;
}

/*
*@description:处理通道状态下行事件函数.需要判断当前event的指令是否正确，正确后才能打标记进行后续业务逻辑的处理：如关闭当前卡端已打开的通道，清空变量等 
*
*@function   :BIPModuleChannelStatus
*
*@param      :BYTE cmdLeng: Length (A+B+C+D) 
*param       :BYTE *cmdBuf:  point to the first sub tag of Event download  Command
*
*
*@Returns    :TRUE:event command合法，打上对应的标记进行后续处理/FALSE:EVENT指令不合法
*
*@history    :V1.0 ：created by lixin@20180412
*             V1.1   modify by xuxiaojun@20180622  102223规范中Channel status 的byte3的bit7,bit8为0时表示要关闭通道
*/
BYTE BIPModuleChannelStatus(BYTE cmdLeng, BYTE *cmdBuf)
{
	BYTE idata pos;
	
	pos = FindTag(cmdBuf, TLV_CHANNELSTATUS, cmdLeng);
	
	if (G_BIPModuleStatus.channelNum == (cmdBuf[pos+1] & 0x07))
	{
		if(G_BIPModuleStatus.channelDataType == ChannelDataType_NULL) //当前没有执行过BIP OPEN channel操作, SIMAPI/API_2_ENH_GCID指令为A0C200000DD60B99010A8202828138020605时要求可以触发应用
		{
			return FALSE;
		}
			
		if(((cmdBuf[pos+1] & 0x07) == 0x00) || ((cmdBuf[pos+1] & 0xC0) == 0x00))  //bit7,8要求关通道
		{
			PendingMessageAlert(BIPModule_Channelstatus_Alert,0x00); //通道关闭打标记
			
			return TRUE;
		}
		
		return FALSE;
	}
	
	return FALSE;
}

/*
*@descritpion: 在收到C2的DataAaliable(D6) event时，判断event数据的有效性，判断当前event的通道ID 是不是已开通业务通道的ID
*
*@function   : BIPModuleDataAvaiable
*
*@param      :BYTE cmdLeng: Length (A+B+C+D) 
*param       :BYTE *cmdBuf:  point to the first sub tag of Event download  Command
*
*@Returns    :TRUE:event command合法，打上对应的标记进行后续处理/FALSE:EVENT指令不合法
*
*@see        :《11.14ts_101267v081800p》11.10
*
*@author 	 :lx
*
*@history    :V1.0 ：create @20180411
*            :V1.1 : modify by xuxiaojun@20180901 收到D6事件之后，打标记BIPModule_DataAvaiable_Alert时，标记的第2个字节用来表示当前D6指令所指示的待接收数据长度3701XX的XX. 主要是用于处理D6过程中又来一个D6事件 <201808311731_LYF LS 5013_Launch Browser(PUSH_Download)_Success_No PoR at Server_No Refresh_Latest Solution.txt>
*
*/
BYTE BIPModuleDataAvaiable(BYTE cmdLeng, BYTE *cmdBuf)
{		
	BYTE idata pos1, pos2, pos3;

	pos1 = FindTag(cmdBuf, TLV_DEVICEIDENTITY, cmdLeng);
	
	pos2 = FindTag(cmdBuf, TLV_CHANNELSTATUS, cmdLeng);
	
	pos3 = FindTag(cmdBuf, TLV_CHANNELDATALENGTH, cmdLeng);

	if ((pos1 == FALSE)||(pos2 == FALSE)||(pos3 == FALSE)) //缺少tag
	{
		return FALSE;
	}
	
	if (G_BIPModuleStatus.channelNum == (cmdBuf[pos2+1] & 0x07)) //判断当前event的通道ID 是不是已开通业务通道的ID
	{
#if UICC__FUNCTION__CATTP
		G_BIPDataAvaiableEvent = TRUE;  //特意为电信CATTP 业务增设的此tag，该变量判断完后即丢弃，在下一条命令即无效
#endif

#if GP__AMENDMENT__B__FOR_MASK
	#if !JAVA_CARD_UICC
		if ((G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS) && (STKPerform == FALSE))
	#else
		if ((G_BIPModuleStatus.channelDataType == ChannelDataType_HTTPS) && (STKPerform[0] == FALSE))		//https操作为了加快速度，不在stk中的可以直接收取数据处理，不用打标记写文件操作
	#endif		
		{
			BIPModuleProcessDataAvaiable(cmdBuf[pos3+1]);
		}
		else
#endif
		{
			PendingMessageAlert(BIPModule_DataAvaiable_Alert, cmdBuf[pos3+1]); //打标记
		}
		
		return TRUE;
	}
	
	return FALSE;
}

/*
*@description: 判断当前指令所指示的event是否是BIP event，如果是，则进行当前BIP的处理，如果不是，则继续后续业务逻辑的处理； event数据已存放于RecData中 
*            : D60E 190109 02028281 38028100 370156
*
*@function    :BIPModuleIsProcess
*
*@param       :void
*
*@return      :TRUE: 当前event命令已被截断，由对应业务进行BIP处理/FALSE: 不满足当前应用的BIP处理流程，继续将数据分发出去
*
*@author      :xuyz
*
*@history     :V1.0 ：create @20180410.该函数的处理默认eventcommand里面的tag是严格按照标准中列出的顺序进行排序的
*/
BYTE BIPModuleIsProcess(void)
{
	BYTE idata pos, bak;
	BYTE idata result;
	
	if(RecData[1] == 0x81)
	{
		pos = 3;
	}
	else
	{
		pos = 2;  //偏移到D6的数据域
	}

	result = FindTag(&RecData[pos],TLV_EVENTLIST,RecData[pos - 1]);
	
	if(result == FALSE)		   //point to  event list tag
	{
		return FALSE;
	}

	bak = pos;
	
	pos = pos + result + 1;
	
	result = FALSE;
	
	switch(RecData[pos] )  //point to the  Event list  字段
	{
		case BIPModule_DataAvailable:
			
			result = BIPModuleDataAvaiable(RecData[bak - 1],&RecData[bak] ); //L V结构
		
			break;
		
		case BIPModule_Channelstatus:
			
			result = BIPModuleChannelStatus(RecData[bak - 1],&RecData[bak] ); //L V结构
		
			break;
		
		default : //白盒测试添加
			break;
	}
	
	return result;
}

