#ifndef    _HTTPPROCESS_H_
#define    _HTTPPROCESS_H_

#include "typedef.h"
#include "CompactManage.h"

/***********global variable***************************************/
#define CHUNKED_BUFSIZE           0x07  //G_ChunkedSizeBuf[0]表示后续有效数据个数，用于临时缓存lcrlf, vcrlf, 30crlfcrlf被截断时的情况

#define HTTPTriggeredByAPPUnDone     0x81  // 表示应用调用了API接口，但是COS还没有调用IF_AnnexB_StartHttpSessionByAPI来实际触发HTTP会话
#define HTTPTriggeredByAPPHasDone    0x01  // COS调用了IF_AnnexB_StartHttpSessionByAPI触发HTTP会话，该函数只能被调用一次

#define HTTPTriggeredNull                      0
#define HTTPTriggeredBySMS                     1
#define HTTPTriggeredByHTTPAdministration      2
#define HTTPTriggeredBySCP81Connection         3
#define HTTPTriggeredByUPCOS		           	   4
    
#define HTTPTriggeredParaCheck			           1	//仅检查参数符合TLV格式
#define HTTPTriggeredParaAnaly			           2	//参数解析并写入会话文件EF_HTTPTriggerPara

typedef  struct httpsDataStatus
{
	BYTE httpHeadExit;               //表示当前解析的HTTP头是否存在，是否完整，一个bit表示一个含义
	BYTE httpCommandType;            //取值范围是HTTP_Command_Type_RAM/HTTP_Command_Type_RFM，表示当前http指令类型是RAM/RFM，卡端回复一次则将该值清空
	BYTE httpException;              //取值范围是HTTP_OK/HTTP_HEAD_ERROR/HTTP_HEAD_FINISHE，表示当前解析的数据格式是否正确
	BYTE httpDataStatus;             //取值范围是HTTP_DataType_HEAD/HTTP_DataType_BODY/HTTP_DataType_END，表示当前正在处理的阶段，是在处理HEAD，还是在处理BODY，还是已经处理完毕
	BYTE httpContentType;            //取值范围是HTTP_ContentType_LVMode/HTTP_ContentType_ChunkedMode，表示当前数据格式的类型是LV结构还是chuncked结构
	BYTE httpChunkedContentStatus;	//取值范围是HTTP_Content_Beginning/HTTP_Content_Processing/HTTP_Content_IsEnding/HTTP_Content_End，表示当前处理的chuncked结构数据的状态，是到了哪个阶段，只有类型是chuncked时才有意义
	BYTE HttpApduNum;				//已经执行的https下行指令个数
	union  
	{
		WORD contentLength;   		//Content_LVMode ,模式下的数据长度
		WORD chunkedSize;     		// chunked 模式下的数据长度
	}contentLen;
	BYTE TargetedSD;							//用于保存当前服务器下行的X- Admin- Targeted - Application所指示的ADF， RAM
	BYTE TriggeredSD;						  	//用于保存当前有效的85参数的来源sdid，也就是触发短信所指示的SD		//或者用于保存触发HTTP API的应用的SD
	BYTE TriggeredAppId;						//用于保存应用触发https时的appid
	BYTE HTTPTimerId;							//http的timerid
	BYTE HTTPIsResume;						//该值为1标识当前是https重连操作
	BYTE HTTPTriggerMode;						//该值在第一条指令时初始化为0,标识当前https会话的触发方式
	BYTE IsHTTPCommandExecueError;			//用于标示HTTP指令执行时是否出现错误，TRUE表示执行执行失败;FALSE表示执行正常。 主要用于HTTP执行执行失败但是并没有收到300D0A0D0A时继续接收数据的情况
//	BYTE HttpAPDUFlag;						//用于标记当前的指令是通过http下行得到的, //使用远程处理模块的全局变量G_Source_Type替换
//	BYTE HttpPorMode;						//http下行的apdu数据的por数据是存放DataInRam/DataInEep中，此值是为了加快处理速速，在数据较少时减少写文件操作，
	BYTE ChunkedSizeBuf[CHUNKED_BUFSIZE];		//LV结构， 假设最大支持6个字节的chunked size字段，即4个ASICII的长度和两个字节的CR LF ，该buf同时也用于缓存end标识 CRLF 或0 CRLF CRLF?
   
	WORD HttpChunkYetReadLen;					//chunk模式时当前块已经分析了多少字节
	WORD RetryConterRemain;				 	//表示当前的剩余重试次数，在IF_Init_In_FirstCommand中初始化为0. 在HTTP触发参数解析时赋值
	BYTE PreviousTargetedSD;        		//上一个http的头中的target的sdid值,参考规范 GPC_2.2_B_...__HTTP_v1.1.3.pdf 的3.6章节, 记录http会话中开启的CommandSession(scp02/scp03)的sdid的值，用于在切换targetApp/close bip channel后进行关闭对应的CommandSession
	BYTE RAMSessionSdId;					//记录当前需要开启 CommandSession的 sdid
}HTTPSDataStatus;			//G_HTTPSStatus

extern HTTPSDataStatus xdata G_HTTPSStatus;   //存储某个http会话的状态信息
extern BYTE xdata G_HTTP_APITriggerTag;             // 用于标记标记API方式下来的触发请求，有没有实际执行过HTTP触发，即有没有执行过IF_AnnexB_StartHttpSessionByAPI(对于同一个API触发请求，该接口函数只允许被触发一次). 初始值为HTTPTriggeredByAPPHasDone
extern WORD xdata G_HTTP_CommandData_Length;
extern BYTE code SCP81_CONTENTTYPE_REQUEST_VALUE[33];

//根据电信测试需求：中国电信用户卡需求白皮书-OTA下载功能分册(2021.V1).pdf 的 5.8.9章节，一条HTTPS协议上行响应报文体至少要求支持1K字节长度。HTTPS上行报文体大于1K时，要求每个
//chunked块大小不能超过1K，如1.5K上行报文体要求拆分为1K、0.5K两个块进行发送。上行报文体小于或等于1K时，要求在一个chunked块内发送
#define CHUNKED_SIZE 0x400					//chunk模式时上行数据的块长度


#define HTTP_HEAD_VERSION_EXIST              0x01  //bit0为1表示收到了http/x.x
#define HTTP_HEAD_PROTOCOL_EXIST             0x02  //bit1为1表示收到了HTTP_PROTOCOL_HEAD
#define HTTP_HEAD_URI_EXIST                  0x04  //bit2为1表示收到了HTTP_NEXT_URI  // The Remote Administration Server explicitly ends the session by sending an HTTP response with no “ X - Admin- Next- URI ”   header and with an empty body
#define HTTP_HEAD_CONTENT_TYPE_EXIST         0x08  //bit3为1表示收到了CONTENT_TYPE
#define HTTP_HEAD_TARGETED_APP_EXIST         0x10  //bit4为1表示收到了TARGETED APPLICATION,  X - Admin- Targeted - Application 在<GPC_2.2_B_Remote_Application_Mgmt_over_HTTP_v1.1.3.pdf>描述了这个域存在或不存在时的处理方式
#define HTTP_HEAD_CONTENT_LENGTH_EXIST       0x20  //bit5为1表示收到了Content-Length或都Transfer-Encoding


#define HTTP_Command_Type_NULL              DataType_NULL
#define HTTP_Command_Type_RFM               DataType_RFM 
#define HTTP_Command_Type_RAM              DataType_RAM
#define HTTP_Command_Type_SCP81			   0x81	//定义下行数据https头的scp81格式

#define HTTP_OK                              0x01
#define HTTP_HEAD_ERROR                      0x02   //表示http头的数据错误, 比如HEADID_XAdmin_Protocol值不正确, HEADID_Content_Type既不是RAM也不是RFM
#define HTTP_UNKNOWN_APP_ERROR               0x03   //X - Admin- Targeted - Applicatio 错误，不需要发警告，不需要终止会话
#define HTTP_NOT_SD_ERROR                    0x04   //X - Admin- Targeted - Applicatio 错误，不需要发警告，不需要终止会话
#define HTTP_SESSION_FINISH                  0x05   //HTTP/1.1 204 No Content CRLF  HTTP头要求终止会话
#define HTTP_BODY_ERROR                      0x06   //HTTP数据体的格式不正确
#define HTTP_RETRY_EXHAUSTED                 0x07   //重试次数被耗尽			//该重试次数耗尽的宏定义为07，不能修改，因为要和java代码对应
#define HTTP_SEC_ERR                 		 0x08   //HTTP/1.1 204 No Content CRLF  HTTP头要求终止会话
#define HTTP_Continue                 		 0x09   //HTTP/1.1 204 有 Content CRLF 值为0，并且有nextUri

//以下Scp81ConnectionStatus状态量  根据接口 SCP81ConnectionStatus定义,接口NM_SCP81ConnectionStatusGetDetails获取的状态量
#define SCP81_CONNECTION_ESTABLISHED         0x0001		//链接建立，握手通过后的状态
#define SCP81_CONNECTION_SETUP_FAILED        0x0002		//链接建立失败，握手失败后的状态
#define SCP81_COMMUNICATION_ERROR       	 0x0003		//数据交互过程中的安全错误时的状态
#define SCP81_CONNECTION_CLOSED        		 0x0004		//链接关闭，正常被通知结束会话时的状态
#define SCP81_SERVER_SWITCH        		 	 0x0005		//服务器切换，RAM、RFM互相切换时的状态
#define SCP81_RECEIVE_DATA        			 0x0006		//接收到数据并正确解析出明文数据后的状态
#define SCP81_CONNECTION_SEND        		 0x0007		//本地标记， 记录当前cos正在发送数据的状态，还没有接收到新的下行数据

#define HTTP_DataType_HEAD                   0x00
#define HTTP_DataType_END                    RemoteDataEndBlock
#define HTTP_DataType_BODY                   HTTP_DataType_END+1


#define TriDataExit				0x0
#define TriDataNotExit				0x1


#define HTTP_ContentType_Null	             0x00
#define HTTP_ContentType_LVMode              0x01
#define HTTP_ContentType_ChunkedMode         0x02

#define HTTP_ChunkedContent_Beginning         0x01    //chunkedsize还没收全
#define HTTP_ChunkedContent_Processing        0x02    //chunkedsize解析完，当前正在读取chunked数据
#define HTTP_ChunkedContent_IsEnding          0x03    //判断chunk数据尾部的CR LF 是否存在，此方案支持HTTPS层的任意分段
#define HTTP_ChunkedContent_End               0x04    //chunk结束



/*******************HTTP POST HEADID define***********************/
#define HEADID_POST                0x00
#define HEADID_HTTP_VERSION        0x01
#define HEADID_HOST                0x02
#define HEADID_XAdmin_Protocol     0x03
#define HEADID_XAdmin_From         0x04
#define HEADID_Content_Type        0x05
#define HEADID_Content_Length      0x06
#define HEADID_Transfer_Encoding   0x07
#define HEADID_XAdminScript_Status 0x08
#define HEADID_XAdmin_Resume       0x09
#define HEADID_XAdmin_Next_URI     0x0A
#define HEADID_XAdmin_TarApp       0x0B
#define HEADID_CRLF                0x0C
#define HEADID_OTHERS              0x0D         //除了以上head以外的头， Notice that the implementation shall not reject an HTTP POST response for the only reason that it contains an HTTP header field not described in this document.
#define HEADID_NOT_FULL            0x0F

/*****************************************************************/
#define CR        			0x0D
#define Lf        			0x0A
#define SPACE     			0x20     // 空格
#define SLASH     			0x2F     // 左划线
#define ZERO 				0x30


/*****************************HTTP触发短信中的参数是否存在************************************/
#define HTTP_TriggerPara_Connection_Eixt     0x01    //bit0为1表示Connection  Parameters存在
#define HTTP_TriggerPara_Security_Eixt       0x02    //bit1为1表示Security  Parameters存在
#define HTTP_TriggerPara_Retry_Eixt          0x04    //bit2为1表示Retry  Parameters存在
#define HTTP_TriggerPara_AdmHost_Eixt        0x08    //bit3为1表示Administration Host  Parameters存在
#define HTTP_TriggerPara_AgentId_Eixt        0x10    //bit4为1表示Agent Id  Parameters存在
#define HTTP_TriggerPara_AdmURI_Eixt         0x20    //bit5为1表示Administration URI  Parameters存在

/**********************function define***************************/
extern void InitHttpVariable(void);
extern BYTE HttpsSessionFileCheck(void);

extern WORD HTTPPatchPostHeadData(BYTE headId, BYTE * outBuf);
extern void HttpProtocolInit(void);
extern void HTTPPostInit(void);
extern void Scp81ConnectionReport(WORD SCP81Status);
extern BYTE HTTPPendingSessionTrigger(void);
extern BYTE HttpPatchPostResponseData(void);
extern void HTTPSAnalyseHead(void);
extern BYTE HexStrToAscStr(BYTE * hexBuf, BYTE * acsiiBuf, BYTE dataLgth);
extern BYTE AsciiToHexInt(BYTE * src, BYTE srcLen, BYTE * dst);
extern WORD HTTPSAnalyseChunkBody(BYTE *chunkBody, WORD chunkLen);
extern BYTE HTTPSAnalyseTriggerPara(BYTE TriggeredSDID, UWORD paraAddr,BYTE mode);
extern BYTE HTTPTriggerConnectionParaProc(BYTE * destBuf, BYTE destBufLen, WORD maxSize, BYTE * srcBuf, BYTE srcBufLen);
extern void HttpSessionReport(BYTE httpException);
extern BYTE HTTPSessionStart(void);
extern BYTE GetOpenChannelParaForHttp(BYTE *para);
extern WORD HTTPAnalyseHeadSub(BYTE dataLen);
extern BYTE HTTPAnalyseBodySub(BYTE dataLen);
extern UWORD AsciiToDec(BYTE *asciiBuf, BYTE asciiLgth);
extern void HttpRAMTargetAppiddefaultProcess(void);
extern BYTE checkHttpTargetAppid(BYTE mode, BYTE * appIdBuf, BYTE appIdLen);
extern void SecondHttpSessionProcess(void);
extern BYTE writeParaAddrPublic(BYTE tag,BYTE subTag,BYTE index);
extern BYTE saveHttpsParaForSession(BYTE triggeredSD,BYTE tag, UWORD srcAddr, BYTE mode);
extern BYTE GetHttpHeadID(BYTE * offsetRecData, BYTE dataLen);
extern BYTE TrimSpaceAtHeadOrEnd(BYTE * asciiBuffer, WORD *startAddr, WORD *endAddr, WORD length);

extern BYTE MemcmpWithoutUpperOrLower(BYTE *buffer1, BYTE *buffer2, BYTE bufLen);
extern BYTE GetHTTPTriggeredSD(BYTE appID);
#endif
