/*
*@HISTORY:    V1.0: 创建源文件
*
*             V1.1: 删除文件中的无效代码
*/
#include "typedef.h"
#include "fileSystem.h"
//#include "eepromDefine.h"
#include "uicc_main.h"
#include "fileAction.h"
#include "ToolkitRFM.h"
#include "uicc_main.h"
//#include "transaction.h"
#include "SimApi.h"
//#include "Interpreter.h"
#include "ToolkitAPIPublic.h"
//#include "main.h"
#include "platform_api.h"
#include "T0ApduImpl.h"
#if UICC__FUNCTION__SIMAPI
#include "uicc_error.h"		//remove warning,dengyiwen,2022/9/27
/*
*@description:拼装post公共函数
*
*@param     :statusType
*
*@return    :void
*
*@history
*/
void Toolkit_publicPostFunction(BYTE statusType, BYTE apiMode)
{
	WORD idata postLen;
	BYTE xdata spi[0x02];
	BYTE idata result,backStatusType;	
	BYTE idata value;
	
	if((apiMode == SIMAPI_Mode)&&(statusType != 0x9E)&&(statusType != 0x9F))
	{
		ThrowToolkitException(BAD_INPUT_PARAMETER_SIM);	 	//throw exception: BAD_LENGTH

		return;    //FWK_MHA_ERHD_1.java 测试中调用了postAsBERTLV((byte)0x33,SW1_RP_ACK);，期望返回ToolkitException.HANDLER_NOT_AVAILABLE异常，所以参数异常放在handler异常之后判断
	}
	
	value = statusType;
	
	if((apiMode == SIMAPI_Mode) || (G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_CALL_CONTROL_BY_NAA))
	{
		value = TRUE;	   //In case of CALL_CONTROL, the CAT Runtime Environment has to set the boolean value always to true.
	}
	
	backStatusType = statusType;

	if(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == EVENT_FORMATTED_SMS_PP_ENV)
	{
		ReadTFBuf(G_EnvelopHanderAddr,G_formatedSPIOffset,&spi[0], 0x02);

		if((spi[1] & 0x20) != 0x00)
		{
			if(G_STKBusy != FALSE)
			{
				G_PostTag = TRUE;

				ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;

				UiccOrSimThrowToolkitException(HANDLER_NOT_AVAILABLE_SIM, apiMode);
			}
		}

		Toolkit_ResponsePack(POR_OK,PORMODE_EVERESPONSE,ToolkitHandlerList[Index_EnveResHandler].TLVListLength, 0x00);//modify by jiangqingbin-20220303, 增加一个传入参数，表示http短信触发结果，不再使用原来的复用SW1，SW2的方式 

		ReadTFBuf(G_EnvelopHanderAddr,G_formatedSPIOffset,&spi[0], 0x02);

		if ((spi[1] & 0x20) != 0x00)	    //PoR response shall be sent using SMS-SUBMIT ，91XX
		{
			G_PostTag = TRUE;               //todo:对于SMS_SUBMIT,数据体内容来源于遵照03.48格式	  ,将response数据拷贝到resdata		 	
            
			ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;
            
			return;
		}	
	}

	postLen = ToolkitHandlerList[Index_EnveResHandler].TLVListLength;

	if(postLen == 0x00)
	{
		if(value == TRUE)
		{
			ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;
		}
		else
		{
			GlobalSetStatus(0x6200);

			SendResponseSingle();//为处理toolkitApp post后直接send主动式指令的情况，post后都会先接收一个命令头，在send处理指令，不直接发状态字。 ————102268 Api_2_Erh_Post

			G_NextAPDUFlag = GPGetCurrenteChannel()+1;//接收到下一条指令

			ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;

			G_PostTag = TRUE;	
		}
		return;
	}
	
	if(postLen > 0x100)
	{	
		if(apiMode == UICCAPI_Mode)
		{
			ThrowAPDUException(BAD_LENGTH); //uicc api / api_2_erh_poab测试用例
		}
		else
		{
			ThrowToolkitException(BAD_INPUT_PARAMETER_SIM);
		}

		return;	
	}

	if((backStatusType !=0x9E)&&(G_CommandMode == Command_Mode_3G))  //测试部脚本： uicc_configuration_additional_test_工程集\SECURED\mslTestApp_1_1 测试中，当前命令是80C2XXXXXX 触发应用时调用了SIM API 的post期望返回61XX，故作此修改
	{
		 statusType = 0x61;
	}
	else
	{
		if(apiMode == UICCAPI_Mode)
		{
			if(value == TRUE)
			{
				statusType = 0x9F;
			}
			else
			{
				statusType = 0x9E;
			}
		}
	}
	
	SW1 = statusType;

	SW2 = postLen;
	
	if((value != TRUE)&&(G_CommandMode == Command_Mode_3G))  //中国电信脚本 UICCAPI脚本，Api_2_Erh_Poab APDU: 80C2000009010782028381FF0101期望先返回6200 ，再发送00C0取响应
	{
		GlobalSetStatus(0x6200);
	}
	if((G_CommandMode == Command_Mode_3G)&& (backStatusType == 0x9E))//测试部脚本： uicc_configuration_additional_test_工程集\SECURED\GP补充\EVENT_CALL_CONTROL_return_Error_or_false_Test_01_01 测试中，当前命令是80C2XXXXXX 触发应用时调用了SIM API 的post，statusType为9E,期望返回6200，故作此修改
	{
		GlobalSetStatus(0x6200);
	}
	G_CurrentReponse[G_ChannelNum] = postLen;

_SENDandRec:
	apdu_ApduSetOutgoingAndSend(5, G_CurrentReponse[G_ChannelNum]);

 	if(SW1 == 0x61)
	{
		GlobalSetStatus(0x9000);
	}
	
	SendResponseSingle();

	result = PostAfterReceiveCommandHeader();

	if(result == ILLEGAL_CHANNEL)  //0xFF :ILLEGAL_CHANNEL
	{
		GlobalSetStatus(0x6E00);

		goto _SENDandRec;
	}

	if(result == FALSE)  //当前接收的是其他通道的命令，则保存当前的C0响应数据
	{
		ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;

		G_PostTag = TRUE;  //默认响应数据已被发送

		G_NextAPDUFlag = GPGetCurrenteChannel()+1;//接收到下一条指令

		return;
	}
	
	if(result == 0x02) //当前接收的是同一通道的命令，但是不是C0，则打断当前C0指令
	{
		G_NextAPDUFlag = GPGetCurrenteChannel()+1;//接收到下一条指令
	}
	else
	{   
		if(P3 == 0x00)
		{
			if(G_CurrentReponse[G_ChannelNum] != 0x100)
			{
				if(G_CommandMode == Command_Mode_3G)
				{
					SW1 = 0x6C;
				}
				else
				{
					SW1 = 0x67;
				}
				
				SW2 = postLen;

				goto _SENDandRec;
			}
		}
		else if(P3 > G_CurrentReponse[G_ChannelNum])
		{
			if(G_CommandMode == Command_Mode_3G)
			{
				SW1 = 0x6C;
			}
			else
			{
				SW1 = 0x67;
			}

			SW2 = postLen;

			goto _SENDandRec;
		}

		//SendData();	 //ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer指向RecData
		SendDataSingle();
		
		if(G_CommandMode == Command_Mode_3G)		
		{
			if((P3 < G_CurrentReponse[G_ChannelNum])&&(P3 != 0x00))
			{
				G_CurrentReponse[G_ChannelNum] = G_CurrentReponse[G_ChannelNum] - P3;

				ecpMem_Copy(&RecData[0],&RecData[P3],G_CurrentReponse[G_ChannelNum]);

				SW1 = statusType;

				SW2 = 	G_CurrentReponse[G_ChannelNum];

				goto _SENDandRec;	//和UICC post的处理保持一致
			}    //\中国移动NFC2.0项目20160415\GP2.2.1_UICC_ComplianceTests(移动版)-20141112\ReEntry_test\SUPPORTSIMAPI\SIMReEntry_06_06_01 如果post的C0未取完,期望 返回91XX ,而不是9fxx
		}
			 
		/*if((apiMode == SIMAPI_Mode)&&(backStatusType == 0x9E))
		{
			if(G_CommandMode == Command_Mode_3G)
			{		
				GlobalSetStatus(0x6200);

				SendResponseSingle();

				G_NextAPDUFlag = GPGetCurrenteChannel()+1;//接收到下一条指令
			}	
		    
		}*/
	}

    ToolkitHandlerList[Index_EnveResHandler].HandlerAvailable = FALSE;

	G_PostTag = TRUE;
}
/*
@description: Pepares the Envelope response. Should be used with Envelope SMS-PP Data Download
*
*@param     :statusType :the status to be sent to the ME (SW1_RP_ACK or SW1_RP_ERROR) 
*
*@return     :void
*
*
*@history:   1: After a call to the post method the handler is no longer available.
             2: After the first invocation of the ProactiveHandler.send method the EnvelopeResponseHandler is no more available
*
*
*@exception:
			ToolkitException - with the following reason codes: 
				HANDLER_NOT_AVAILABLE if the handler is busy 
			javacard.framework.APDUException - with the following reason code: 
				BAD_LENGTH if the resulting response length is greater than 256 and the response data has to be retrieved by the GET RESPONSE command.
*/
void Toolkit_Post(void)
{
	jbyte idata statusType;
	
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	statusType = PopOperandStackShort();

	ToolkitAvaiableOfEnvelopResponse(apiMode);

	handler_check_available(Index_EnveResHandler, apiMode);	   //检查handler是否有效

	Toolkit_publicPostFunction(statusType, apiMode);
}

/*
@description: Prepare the Envelope response in a BER TLV structure. Should be used with Envelope Call Control by SIM or MO Short Message Control by SIM. The tag value is to be used to set the Result for Call Control and MO Short Message Control by the SIM.
*
*@param     :statusType :the status to be sent to the ME (SW1_RP_ACK or SW1_RP_ERROR)
*@param     :tag - the BER Tag to be used at the beginning of the SIMPLE_TLV list.  
*
*@return     :void
*
*
*@history:   1: After a call to the post method the handler is no longer available.
             2: After the first invocation of the ProactiveHandler.send method the EnvelopeResponseHandler is no more available
*
*
*@exception:
			ToolkitException - with the following reason codes: 
				HANDLER_NOT_AVAILABLE if the handler is busy 
			javacard.framework.APDUException - with the following reason code: 
				BAD_LENGTH if the resulting response length is greater than 256 and the response data has to be retrieved by the GET RESPONSE command.
*/
void Toolkit_PostAsBERTLV(void)
{
	BYTE xdata tmp[0x04];
	jbyte idata statusType; 
	jbyte idata tag;
	jshort idata apiMode;
	
	apiMode = PopOperandStackShort();
	
	tag = PopOperandStackShort();
    
	statusType = PopOperandStackShort(); 	

	ToolkitAvaiableOfEnvelopResponse(apiMode);

	handler_check_available(Index_EnveResHandler, apiMode);//检查handler是否有效,handlerde无效的情况下不应该改变HandlerBuffer的值,by xwh 20140218
	 	
	tmp[0x01] = tag;

	if(ToolkitHandlerList[Index_EnveResHandler].TLVListLength >0x7F)
	{
		tmp[2] = 0x81;

		tmp[3] = ToolkitHandlerList[Index_EnveResHandler].TLVListLength;

		tmp[0] = 0x03; 
	}
	else
	{
		tmp[2] =  ToolkitHandlerList[Index_EnveResHandler].TLVListLength;

		tmp[0] = 0x02;
	}

	ecpMem_Move(&ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer[tmp[0]], &ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer[0],ToolkitHandlerList[Index_EnveResHandler].TLVListLength);

	ecpMem_Copy(&ToolkitHandlerList[Index_EnveResHandler].HandlerBuffer[0], &tmp[1],tmp[0]);

	ToolkitHandlerList[Index_EnveResHandler].TLVListLength += tmp[0];

	Toolkit_publicPostFunction(statusType, apiMode);
}
#endif
