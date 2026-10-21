#ifndef    _TLSMIANFLOW_H_
#define    _TLSMIANFLOW_H_
#include   "typedef.h"
#include   "profile.h"
#include   "Crypto_Constant.h"

/*********************macro define*****************************/
#define TLS_Version10       0x0301
#define TLS_Version11       0x0302
#define TLS_Version12       0x0303
#define TLS_DefaultVersion  TLS_Version12				//5G-SIMCard

#define TLS_SESSION_KEY_LEN  128

#define TLS_MaterSecret_Len   48    //根据TLS1.0 1.1 1.2规范, mastersecret长度固定是48字节

#define TLS_ServerRandom_Len  32    //根据TLS1.0 1.1 1.2规范, random长度固定为32字节
#define TLS_ClientRandom_Len  32    //根据TLS1.0 1.1 1.2规范, random长度固定为32字节
#define TLS_VerifyData_Len    12

#define DataInRam		    1
#define DataInEep		    2

#define ClientUpMsg_Mode     0
#define ServerDownMsg_Mode   1

//mac mode 
#define TLS_OTHER_BLOCK    0x00
#define TLS_FIRST_BLOCK    0x01
#define TLS_LAST_BLOCK     0x80
#define TLS_ONLY_BLOCK     0x81


//文件EF_TLSsessionData的TLS_OpenCloseNormal_Offset偏移位子的值的宏定义
#define Https_Session_End   	0x00
#define Https_Session_Running   0x01

//EncryMode/DecryMode
#define TLS_EncryMsg_Mode   0x00
#define TLS_DecryMsg_Mode   0x01

/*****************TLS 层的状态字 可以用这个异常来表示TLS层的Alert Value <Alert Protocol>**********************************/
#define  TLS_RECORD_OK                                      0x00

#define  TLS_CLOSE_NOTIFY 								    0x00    			 //通道关闭
#define  TLS_UNEXPECTED_MESSAGE							    0x0A   			 //非期望的信息， 即不是14,15,16,17,  This alert is always fatal
#define  TLS_BAD_RECORD_MAC								    0x14   			 //mac错误                            This message is always fatal
#define  TLS_DECRYPTION_FAILED							    0x15   			 //解密失败
#define  TLS_RECORD_OVERFLOW								0x16   			 //数据溢出                           This message is always fatal
#define  TLS_DECOMPRESSION_FAILURE						    0x1E   			 //解压失败                           This message is always fatal
#define  TLS_HANDSHAKE_FAILURE							    0x28   			 //握手失败                           This is a fatal error
#define  TLS_NO_CERTIFICATE_RESERVED 					    0x29   			 //没有证书
#define  TLS_BAD_CERTIFICATE								0x2A   			 //错误证书
#define  TLS_UNSUPPORTED_CERTIFICATE						0x2B   			 //不支持的证书
#define  TLS_CERTIFICATE_REVOKED							0x2C   			 //被撤销的证书
#define  TLS_CERTIFICATE_EXPIRED							0x2D	 		 //已过期的证书
#define  TLS_CERTIFICATE_UNKNOWN							0x2E   			 //未知的证书			
#define  TLS_ILLEGAL_PARAMETER							    0x2F   		     //非法的参数                          This message is always fatal
#define  TLS_UNKNOWN_CA									    0x30   			 //未知的CA                            This message is always fatal
#define  TLS_ACCESS_DENIED								    0x31   			 //拒绝访问                            This message is always fatal
#define  TLS_DECODE_ERROR								    0x32   			 //解码错误
#define  TLS_DECRYPT_ERROR								    0x33   			 //解密错误
#define  TLS_EXPORT_RESTRICTION_RESERVED					0x3C   			 //输出限制
#define  TLS_PROTOCOL_VERSION							    0x46   			 //协议版本
#define  TLS_INSUFFICIENT_SECURITY						    0x47   			 //不足够的安全
#define  TLS_INTERNAL_ERROR								    0x50   			 //内部错误
#define  TLS_USER_CANCELED								    0x5A   			 //用户取消
#define  TLS_NO_RENEGOTIATION							    0x64  			 //没有重协商
#define  TLS_UNSUPPORTED_EXTENSION							0x6E          	 //不是期望的扩展格式


/******** TLSSecurityParameters.tlsRecordState的状态变迁 ********/
#define TLS_NULL_STATE	                       				0x0
#define TLS_SEND_CLIENT_HELLO_STATE            				0x1
#define TLS_RECEIVE_SERVER_HELLO_STATE         				0x2
#define TLS_RECEIVE_SERVER_KEY_EXCHANGE_STATE				0x3
#define TLS_RECEIVE_SERVER_HELLO_DONE_STATE    				0x4
#define TLS_SEND_CLIENT_KEY_EXCHANGE_STATE     				0x5
#define TLS_SEND_CLIENT_CHANGE_CIPHER_SPEC_STATE            0x6
#define TLS_SEND_CLIENT_FINISHED_STATE         				0x7
#define TLS_RECEIVE_SERVER_CHANGE_CIPHER_STATE      	    0x8
#define TLS_RECEIVE_SERVER_FINISHED_STATE      				0x9
#define TLS_RECEIVE_APPLICATION_DATA_STATE					0xA
#define TLS_SEND_APPLICATION_DATA_STATE				 		0xB
#define TLS_SEND_ALERT_DATA_STATE	            			0xC
#define TLS_END_STATE 										0xD

#define TLS_RECORD_STATE_MASK                               0x7F
#define BIT_RETRY_WITH_START_TIMER                          0x07 //http session end with channel status, then need start timer, and end session


/******** Alert Protocol <rfc5246TLS1.2.txt.pdf> AlertLevel **********/
#define ALERT_LEVER_WARNING								    0x01     //警告级别
#define ALERT_LEVER_FATAL								   	0x02     //致命级别

/******** TLS ContentType <rfc5246TLS1.2.txt.pdf> 6.2.1 **********/
#define TLS_CHANGE_CIPHER_SPECS_PROTOCOL                    0x14
#define TLS_ALERT_PROTOCOL                                  0x15
#define TLS_HANDSHAKE_PROTOCOL                              0x16
#define TLS_APPLICATION_DATA_PROTOCOL                       0x17

/**** Handshake Protoco HandshakeType <rfc5246TLS1.2.txt.pdf> ****/
#define HandshakeType_HelloRequest                          0x00
#define HandshakeType_ClientHello			                0x01		
#define HandshakeType_ServerHello			                0x02		
#define HandshakeType_ServerKeyExchange		                0x0C	
#define HandshakeType_ServerHelloDone            			0x0E		
#define HandshakeType_ClientKeyExchange                 	0x10				
#define HandshakeType_ClientFinished            			0x14		
#define HandshakeType_ServerFinished             			0x14		

/**************** TLS Record和HandShake的协议头长度 ****************/
#define TLS_RecordHead_LENGTH                                5   //protocol(1byte) + version(2byte) + len(2byte)
#define TLS_HandShakeHead_LENGTH                             4   //HandshakeType msg_type + uint24 length

/****************** 目前卡片中支持的ciphersuites ******************/
#define TLS_PSK_WITH_NULL_NULL 				0x0000
#define TLS_PSK_WITH_NULL_SHA 				0x002C    
#define TLS_PSK_WITH_3DES_EDE_CBC_SHA 		0x008B     // <rfc4279-PSK.txt.pdf>
#define TLS_PSK_WITH_AES_128_CBC_SHA  		0x008C     // <rfc4279-PSK.txt.pdf>
#define TLS_PSK_WITH_NULL_SHA256 			0x00B0
#define TLS_PSK_WITH_AES_128_CBC_SHA256 	0x00AE
#define TLS_PSK_WITH_AES_128_GCM_SHA256 	0x00A8


/************EF_TLSsessionData文件描述***************************************/
#define TLS_RetryRemainNum_Offset         		0	    //用于保存http的当前剩余重试次数 (WORD)		//由于5gcmcc需求速度， 所以减少写文件操作， 重发次数修改到重发的时候再减少和操作
#define TLS_CurrentTriggerSD_Offset       		2	    //用于保存当前https会话的sdid
#define TLS_CurrentTriggerAppid_Offset			3		//用于保存当前https会话的应用的id
#define TLS_CurrentTriggerMode_Offset			4		//用于保存当前https会话触发模式
#define TLS_CurrentExecutingProfileIndex_Offset	5		//用于保存当前https执行的区域  added at 20210406
#define TLS_OpenCloseNormal_Offset        		6     	//用于记录HTTP会话标记。1为有https会话在进行，0为没有
#define	Http_TargetAppId_LvOff			 		7		//本次下行数据的target app 的aid， L(1BYTE)V格式

#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
/************EF_HTTPTwoSession(A100/1A08) 文件描述***************************************/
#define HTTP_SecondSession_ParaLen_Offset          0x00            //用于保存第2个HTTP触发请求的参数长度，参数为0表示当前没有需要触发的会话，该值在open channel之后清掉
#define HTTP_SecondSession_TriggerMode_Offset      0x02		       //用于保存第2个HTTP触发请求的触发模式，是SMS触发，还是API触发
#define HTTP_SecondSession_TriggerSD_Offset        0x03		       //用于保存第2个HTTP触发请求的APP ID
#define HTTP_SecondSession_ExecutingProfileIndex_Offset         0x04		       //用于保存第2个HTTP触发请求的执行区域  added at 20210406
#define HTTP_SecondSession_Para_Offset             				0x05            //用于保存第2个HTTP触发请求的参数
#endif




/*********EF_TLSKey文件描述*****************************/
#define  Tls_KeyLen_Off                   0x00    //6字节，分别表示 client_wirteMacKey 、server_wirteMacKey、client_wirteKey、client_wirteKey、client_IV_value、server_IV_value的长度
#define  Client_WirteMacKey_Value_Off     0x06
#define  TLS_ServerRandom_Offset          Client_WirteMacKey_Value_Off  //复用Client_WirteMacKey_Value_Off, 根据TLS1.0 1.1 1.2规范,random长度固定为32字节
#define  Server_WirteMacKey_Value_Off     0x26
#define  TLS_ClientRandom_Offset          Server_WirteMacKey_Value_Off  //复用Server_WirteMacKey_Value_Off, 根据TLS1.0 1.1 1.2规范,random长度固定为32字节
#define  Client_WirteKey_Value_Off        0x46
#define  Server_WirteKey_Value_Off        0x66
#define  Client_IV_Value_Off              0x86
#define  Server_IV_Value_Off              0xA6


/************EF_HTTPTriggerPara文件描述***************************************/
#define HTTP_TriggerPara_ConnectionPara_Index      0x01           //第一条记录存储Connection parameters(84), LV格式
#define HTTP_TriggerPara_SecurityPara_Index        0x02           //第二条记录存储Security parameters(85), LV格式
#define HTTP_TriggerPara_RetryPara_Index           0x03           //第三条记录存储Retry policy parameters(86), LV格式
#define HTTP_TriggerPara_HostPara_Index            0x04           //第四条记录存储Administration Host parameters(89-8A), LV格式
#define HTTP_TriggerPara_AgentIdPara_Index         0x05           //第五条记录存储Agent Id parameters(89-8B), LV格式
#define HTTP_TriggerPara_URIPara_Index             0x06           //第六条记录存储Administration URI parameters(89-8C), LV格式
#define HTTP_TriggerPara_NextURIPara_Index         0x07           //第七条记录存储当前服务器下发的HTTP头中的next uri， LV格式
//#define HTTP_TriggerPara_ContentType_Index         0x08           //第八条记录是移动5G sim 项目中通过api-scp81connection.Open()触发https时传入的数据，用于在https握手成功后上行数据的connectType域, LV格式

/*****************************HTTP触发短信中的参数************************************/
#define HTTP_TriggerPara_Connection_TAG      0x84
#define HTTP_TriggerPara_Security_TAG        0x85 
#define HTTP_TriggerPara_Retry_TAG           0x86
#define HTTP_TriggerPara_Post_TAG            0x89
#define HTTP_TriggerPara_AdmHost_TAG         0x8A 
#define HTTP_TriggerPara_AgentId_TAG         0x8B  
#define HTTP_TriggerPara_AdmURI_TAG          0x8C


/************EF_HTTPSCommandData 文件复用于tls握手偏移描述***************************************/
#if GP__AMENDMENT__B__TLS_MasterKey_IN_FILE
#define TLS_MasterSecret_Offset       			0		//用于保存MasterSecret
#endif
#define TLS_HandshakeMsgLen_Offset     			150		//用于保存HandshakeMsgLen
#define TLS_HandshakeMsgValue_Offset   			152		//用于保存HandshakeMsg

#define TLS_ClientRandom_TempBufOffset        			440		//用于保存ClientRandom
#define TLS_ServerRandom_TempBufOffset        			480		//用于保存ServerRandom

/****************constent define********************************/
extern BYTE code LABEL_KEY_EXPANSION[];
extern BYTE code LABEL_MASTER_SECRET[];
extern BYTE code LABEL_CLIENT_FINISHED[];
extern BYTE code LABEL_SERVER_FINISHED[];
//extern BYTE code Label_Tls0301_CipherSuit[];		//tls1.0/1.1版本支持的算法集
extern BYTE code Label_Tls_Extand[];				//tls扩展字段
//extern BYTE code Label_Tls0303_CipherSuit[];		//tls扩展字段

#if (!GP__AMENDMENT__B__TLS_MasterKey_IN_FILE)
extern BYTE xdata G_TLSMaterSecretBuf[TLS_MaterSecret_Len];
#endif

#if (!GP__AMENDMENT__B__TLS_SessionKey_IN_FILE)
extern BYTE xdata G_TLSSessionKeyBuf[TLS_SESSION_KEY_LEN];
#endif

#if (!GP__AMENDMENT__B__HAND_SHAKE_MSG_IN_FILE)
extern BYTE xdata G_HandShakeHashMD5Buf[MD_ICV_SIZE+2];//0x12					//当前已update进去的数据计算到的hash,根据hash算法计算方案，前两字节是blockNum，使用时定义数组SHA256_ICV_SIZE+2大小即可，中间计算过程不用做任何赋值，hash计算函数内部会处理
extern BYTE xdata G_HandShakeHashMD5RemainDataBuf[MD_BLOCK_SIZE+1];//0x41		//上一次update数据进去时的剩余的不足一块的数据的剩余长度，
extern BYTE xdata G_HandShakeHashSha256Buf[SHA256_ICV_SIZE+2];//0x22			//当前已update进去的数据计算到的hash,根据hash算法计算方案，前两字节是blockNum，使用时定义数组SHA256_ICV_SIZE+2大小即可，中间计算过程不用做任何赋值，hash计算函数内部会处理
extern BYTE xdata G_HandShakeHashSha256RemainDataBuf[SHA256_BLOCK_SIZE+1];//0x41//上一次update数据进去时的剩余的不足一块的数据的剩余长度，
#endif

/*****************global variable define************************/
typedef __packed struct tagTLSSecurityParameters
{
	BYTE    tlsRecordState;
	//BYTE    tlsPSKVersion;        //进行会话时保存所使用的PSK 的版本号
	WORD    tlsProtcolVersion;    //进行会话时使用的TLS协议版本
	WORD    cipherSuite;          //本次会话serverhello中指示的算法
	BYTE    cipherAlgorithm;      //本次会话协商的加密算法
	BYTE    enckeyLength;         //加密key
	BYTE    blockLength;          //加密算法的块长度, == IV长度
	BYTE    macAlgorithm;         //本次tls协商的mac算法
	BYTE    macKeyLength;         //mac key长度
	BYTE    macLength;            //本次会话期望的mac长度
	WORD    clientSeqNum;         //RFC5246: Each connection state contains a sequence number,规范要求6字节长，目前假定2字节够用
	WORD    serverSeqNum;         //RFC5246: Each connection state contains a sequence number,规范要求6字节长，目前假定2字节够用
	BYTE    clientChangeCipher;   //客户端的change cipher中的值
	BYTE    serverChangeCipher;   //服务器下发的change cipher中的值
	BYTE    tlsException;         //TLS层数据异常，0表示正常
	BYTE    tlsAlertLever;        //TLS层warning or fatal
	WORD    TLSEachLength;     	  //每次上行tls-record的最大长度
} TLSSecurityParameters;          //一共17字节

extern TLSSecurityParameters xdata G_TLSSecurityParameters;
/*****************function define**********************************/
extern void TLSInitVariable(void);

extern BYTE TLSSendClientHelloMsg(void);
extern WORD TLSSendClientKeyExchangeMsg(WORD offSet);
extern WORD TLSSendChangeCipherSpecMsg(WORD offSet);
extern WORD TLSSendClientFinishedMsg(WORD offSet);
extern void TLSAnalyzeCipherSuit(WORD cipherSuite);
extern BYTE TLSProcessServerHello(BYTE *severHelloBuf, WORD serverHelloLen);





extern void TLSSaveHandShakeMsg(BYTE *handMsgBuf, BYTE handMsglen );
extern BYTE TLSTransRecordStateToHandShakeType(BYTE tlsRecordState);
extern void TLSProcessRecieveHandShakeMsg(void);
extern BYTE TLSVerifyServerFinishedMsg(BYTE *msgBuf, WORD msgLen);
extern void TLSHandProcess(BYTE channelDataLen, WORD offset);
extern BYTE TLSProcessRecieveApplicationDataMsg(BYTE channelDataLen, WORD offset);
extern void TLSApplicationDataDecryVerifyMac(UWORD fileAddr, WORD curDataOff, WORD recordLen);
extern BYTE TLSPatchApplicationOutData(void);
extern void TLSSendAlertMsg(void);


extern void saveNextUriForRetry(void);
extern BYTE TLSClientUpMsgGnerateIV(void);
extern BYTE TLSServerDownMsgExistIV(void);
extern void setTLSSecurityPara(BYTE RecordState,BYTE tException,BYTE AlertLever);
#endif

