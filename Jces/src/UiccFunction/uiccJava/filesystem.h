/*

 * Copyright (c) 2005,eastcompeace

 * All rights reserved.

 * MODEL: EM TG96-3G/TCG96-3G EM ATMEL SAMSUANG INFINEON,RENESAS,SOLOMON  

 * MOTOCOS V5.01

 * USIM AND SIM  RELEASE:2005-2-25

 * AUTHER:	蔡勃

 */
#ifndef    _fileSystem_h_
#define    _fileSystem_h_
#include "profile.h"
//-------------------------------------------------------------------------------------------------
//                                       文件表识的定义
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//                                       文件表识的定义
//-------------------------------------------------------------------------------------------------
#define     DF_MF       	0x3F00
//#define		TF_ATR			0x2F01
#define     EF_ARR      	0x2F06
//#define     EF_KEY      	0x2E10
#define     EF_UMPC      	0x2F08                    //EFUMPC,没有profile 时的默认文件
#define     EF_CLASSES      0x2F07                    //EFENV-CLASSES,没有profile 时的默认文件

#define		EF_CARDCFG		0x2E02			//


#define 	EF_AuthCFG_TPX		0x2E00	
#define 	EF_Ki_TPX		    0x6F1B
//------------------ROOT下私有文件--------------------
//#define 
#define DFProfileId 0xA900
#define TemplateFileId 0x9A70
#define PrivateTemplateFileId  0x9A6F
#define SpecPrivateTemplateFileId  0x9A6E
#define TemplateFileId_2_1 0x9A60

#define   DF_Toolkit          	       0xA800//(2800)    

#define   EF_EnvelopeCBMessage         0x8A0A //(0x6F78)
#define   EF_ToolkitAppContext         0x8A04 //(0x6F7C)    //保存当前APP的上下文，tag：0x81，该文件后续内容有效
#define   EF_ToolkitRFMSmsHead	       0x8A01 //(0x6F7E)    //,该文件初始化为全0,LF，每条记录格式如下：smsType refrenceNumber EVENTID L 设备标示(TLV)  地址(TLV)  8B-udl ,主要用于存储多条短信的短信头信息
#define   EF_ToolkitRFMSms             0x8A02 //(0x6F7F)    //smstype ,批次,,总数,索引 后续数据长度,数据
#define   EF_EnvelopeHandlerBuf        0x8A08 //(0x6F90)   //存储EnvelopHandler数据
#define   EF_EnvelopSecondHandlerBuf   0x8A09 //(0x6F91)  //该文件主要用于在app1中触发APP2时,缓存app2的envelop数据
#define   EF_RAMExtendResponse         0x8A05 //(0x6F93)   //LV 结构，L：2字节，后续存储响应数据，TF文件，文件初始化为0
//#define   EF_SSDMenuInfo               0x6F94   // 线性结构 存储SD下载过程中EA参数中的菜单信息   APPID + 菜单个数（最大10） + 最大菜单个数* 2    共22字节一条纪录 
#define   EF_HTTPRFM                   0x8A06 //(0x6F95)  //该文件转移到226模块中 //文件大小为0x12 用于存储HTTP的RFM过程中的DfAddr/EfAddr/DfName/EfName/AdfName/RecNum/CommandMode等，该文件的初始值为0
#define   EF_PorDataFile               0x8A07 //(0x6FFF)   //该文件主要用于缓存POR数据
#define   EF_STKAlert                  0x8A0C //(0x6EA7)    //标志文件   两字节一个标志
#define   EF_ScriptChainingFile        0x8A0D //0x5E21    //intial: 0x00 byte 1:当前Script Chaining Value,,结构体保存了当前context(ScriptValue,current directory, current file, current tag pointer, etc.) and the PIN verification),

#define   EF_SWPPrivateConfig         0x4E83  //SWP配置文件

#define	  EF_CSMS_Head			       0x8A0B //(0xA106) //缓存C网短信头内容
#define   TempBufoffset                 256        //开始存放 User Data位置
#define   RFM_CSMS_refrence                     0x00                            //批次
#define   RFM_CSMS_TotalNum                 (RFM_CSMS_refrence + 1)             //总数
#define   RFM_CSMS_DeviceIdentities         (RFM_CSMS_TotalNum + 1)
#define   RFM_CSMS_MessageType             (RFM_CSMS_DeviceIdentities + 4)
#define   RFM_CSMS_Teleservice_Identifier  (RFM_CSMS_MessageType +1)
#define   RFM_CSMS_Originating_Address     (RFM_CSMS_Teleservice_Identifier +4)
#define   RFM_GSM_Originating_Address      (RFM_CSMS_Originating_Address + 0x10)
#define   RFM_CSMS_Message_Identifier      (RFM_GSM_Originating_Address + 0x10)
#define   RFM_CSMS_Message_Display_Mode      (RFM_CSMS_Message_Identifier + 5)        //Message Display Mode  TLV 
#define   RFM_CSMS_MSG_ENCODING            (RFM_CSMS_Message_Display_Mode + 3)
#define   RFM_CSMS_LAST_BEARER_LEN          (RFM_CSMS_MSG_ENCODING + 1)

/**************************以下为8A07(6FFF) EF_PorDataFile的描述**************************************************************/
//更新当前会话使用的SCP80时候,发送POR时候要使用更新前的key，发送POR后使用新的key,所以在执行RAM操作的时候把更新前的key暂时保存到这个POR文件,执行完POR清空
//#define  SCP80Keyoffset            0x60   //一般来说 POR文件会>0x60+33 +33  存储格式TLV 00(Kic)/01(kid)
#define  SCP80Keyoffset            (BIPModule_Send_SIZE - 0x80)  //使用BIPModule_Send_ADDR_START 最后一段作为保存旧的key 的地址, modify by xuweihua@20210625: 优化空间,EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop

/**************************以下为8A05(6F93) EF_RAMExtendResponse的描述**************************************************************/
//#define ramExtendResLen_offset    0x00			  //文件数据偏移
#define ramExtendResValue_offset  0x02 
/**************************以下为0x8A06(6F95) EF_HTTPRFM的描述**************************************************************/
#define HTTP_RFM_Count_Offset                      0x00           //BYTE类型，表示当前执行的HTTP RFM的指令条数
#define HTTP_RFM_DfAddr_Offset                     0x01           //UWORD类型 
#define HTTP_RFM_EfAddr_Offset                     0x05           //UWORD类型 
#define HTTP_RFM_DfName_Offset                     0x09           //WORD类型
#define HTTP_RFM_EfName_Offset                     0x0B           //WORD类型
#define HTTP_RFM_ADfName_Offset                    0x0D           //WORD类型
#define HTTP_RFM_RecNum_Offset                     0x0F           //BYTE类型
#define HTTP_RFM_CommandMode_Offset                0x10           //BYTE类型
#define HTTP_RFM_Channel_Offset                    0x11           //BYTE类型

//-------P下的私有文件 
#define DF_PROFILECFG                    0xB100	
#define EF_PRIVATE_PUK                   0x1B01
#define EF_PRIVATE_PIN                   0x1B02  //私有文件模板tag：0xA5，local pin
#define EF_AUTHRECDATA                   0x1B03
#define EF_EAPAUTHDATA					 0x1B04//缓存EAP鉴权数据    Nonce(16)+MK(20)+K_encr(16)+K_auth(16) =68


//每个ADF下面都需要创建，私有文件模板tag：0xA0
//#define EF_PRIVATE_PIN                   0x1B02
#define EF_KI       	                 0x1B1B//0x6F1B  

//私有文件模板tag：0xA1
//USIM/ISIM都需要创建
#define EF_PRIVATE_AUTHCFG               0x1B10  //2E00 	//用于保存鉴权时候配置（1字节）、OP内容和鉴权常数R和C
#define	EF_SQNCheck		                 0x1B11 //0x2E01			//用于保存鉴权成功时候的SQN的内容，32（记录数）×6（字节数）
#define EF_AuthMaxCounter                0x1B1D//0x6F1F
#define EF_LOCALKEY_ID                   0x1B06
#define EF_GBA_KS                        0x1B05

#define EF_AUTH_MODE                     0x1B04
//#define EF_GBA_KS_ISIM                   0x1B07  // add by chentao@20211111 USIM  ISIM 鉴权参数分开存储

//私有文件模板tag：0xA2
//只有USIM需要创建
#define     EF_MUK_SUCUSED_USIM          0x1B0D
#define     EF_MSK_KV_USIM               0x1B0E


//TOOKIT相关的_P下私有文件
#define   DF_Toolkit_P          	   0xB200 //(2800)
//#define   EF_RAMAPDUTars               0X2B7B //(0x6F7B)	 //主要用于保存TAR 000000 或B00000-BFFFFF之间的tar信息,记录内容初始化为全FF
#define   EF_ToolkitApp       	       0x2B80 //(0x6F80)	 //该文件主要用于存放安装Toolkit App时的一些参数信息，如Timers Channels  MenuItems MSL TARs AccessDomain 等
#define   EF_ToolkitMenuEntry          0x2B81 //(0x6F81)	 //该文件主要用于保存菜单信息， 初始化为全FF
#define   EF_MenuEntryIndex            0x2B82 //(0x6F82)
#define   EF_ToolkitFileList           0x2B83 //(0x6F83)   //支持透明文件，初始化为全FF,该文件大小为10的倍数，其中10表示当前卡片最大支持4级DF+1级EF(即10个字节)
#define   EF_ToolkitAppPathInfo        0x2B84 //(0x6F84)   //记录文件，初始化为全0，记录长度为2的倍数，其记录个数等于6F80的记录个数，该文件的文件内容 格式为LV  +LV ,L表示fileNumer,前21个字节表示extralFile 的更新信息,后21个字节表示remote fileUpdte的信息
#define	  EF_RFM_TAR			       0x2B05 //(0x4E05) //文件结构： tar(3字节)+mslConfig(1字节)+counterfig(1字节)+counter(5字节)，该文件初始化为全FF
#define   EF_SSDMenuInfo               0x2B94 //(0x6F94)   // 线性结构 存储SD下载过程中EA参数中的菜单信息   APPID + 菜单个数（最大10） + 最大菜单个数* 2    共22字节一条纪录 
#define   EF_RFMFileUpdate             0x8A18   //保存RFM 操作中文件的Update Increase信息
#define	  EF_ARR_PRIVATE		       0x2B06   //AB/8C规则存放私有文件

//added by xuweihua@20210710  2B80文件放在专门的目录BB00下，并放在文件系统最后面，当下载/删除文件时，2B80记录数可以动态增加/减少. 
//用UICC__FUNCTION__2B80_AT_FILEEND开关进行控制,不会影响以前的功能
#if UICC__FUNCTION__2B80_AT_FILEEND
#define   DF_Toolkit_P_Ex          	   0xBB00
#else
#define   DF_Toolkit_P_Ex              DF_Toolkit_P
#endif

//#define 	EF_AuthCFG		0x2E00			//用于保存鉴权时候配置（1字节）、OP内容和鉴权常数R和C
//#define		EF_CARDCFG		0x2E02			//卡片设置文件
#define		EF_ICCID		0x2FE2		
#define   EF_UMPC     0x2F08
#define		EF_WIBMenuName	0x2001
#define		EF_WIBMenu		0x2100
//#define		EF_APPSQNCheck  0x2EF1  //用来保存鉴权成功时候的SQN的内容，32（记录数）*6（字节数）


//----------------------------DF 7F25--------------------//
#define   DF_CDMA               0x7F25
//#define   EF_CdmaKi         	0x6F1B
#define   EF_PRL_SMS            0x6FFF
#define   EF_PRL_UPDATEFILE     0x6F30
#define	  EF_UIMID				0x6F42
#define	  EF_RUIMID				0x6F31
#define   EF_ESNME				0x6F38
#define	  EF_IMSIM				0x6F22
#define   EF_IMSIT				0x6F23
#define   EF_TMSI				0x6F24
#define   EF_CDMA_SST			0x6F32
#define   EF_CDMA_ACC           0x6F2C


#define EF_CHAP_SS  			        0x1B80 // 6E80
#define EF_CHAP_SS_TABLE	 	        0x1B81 // 6E81
#define MN_HA_SS_TABLE           	    0x1B82 // 6E82
#define EF_ComputerIp            	    0x1B83 // 6E83
#define MN_AAA_SS_TABLE          	    0x1B84 // 6E84
#define	EF_SSD				            0x1BBB //0x6FBB

#define		DF_TELECOM		0x7F10
#define		EF_SMSP			0x6F42
#define   EF_ADN			0x6F3A
#define   EF_SMS			0x6F3C
#define 	EF_BDN			0X6F4D  
#define   DF_PHASE    	0x7F20
#define   EF_SST      	0x6F38
#define   EF_WRONGKI  	0x0010
#define   EF_RAND     	0x0020
#define   EF_IMSI     	0x6F07
#define		EF_PLMNsel		0x6F30
#define	    EF_STKName		0x6F54
#define		EF_FPLMN		0x6F7B
#define		EF_LOCI			0x6F7E
#define		EF_134			0xFF03
#define     EF_ACC      	0x6F78
#define     EF_BCCH      	0x6F74
#define		EF_SPN			0x6F46
#define		EF_EPSNSC       0x6FE4
#define     EF_AD     	    0x6FAD

#if UICC__OPERATOR__CHINAUNICOM
#define   EF_OPLMNwAcT   0x6F61
#define    EF_LoginNetFlag  0xFF00  //根据联通现场写卡规范，定义的登网标识文件
#endif

#define     EF_MSK_USIM            0x6FD7
#define     EF_MUK_USIM            0x6FD8

#define		EF_IMPI			0x6F02
#define		EF_IST			0x6F07
#define		EF_GBABP		0x6FD5
#define		EF_GBANL		0x6FD7

#define     EF_GBABP_USIM   0x6FD6
#define		EF_GBANL_USIM	0x6FDA

#define		EF_UST			0x6F38
#define		EF_EST			0x6F56


#define     EF_AFL          0xAF01


#define		DF_GSM			0x7F20 
#define	    DF_DCS1800		0x7F21
//---------------------OTA------------------------//
#define		DF_STK				0x2700
#define   	DF_OTA				0x2700
#define		EF_Menu				0x6F18


//#define	  	EF_TAR				0x6EA2
//#define	  	EF_OTASecurity		0x6EA3
//#define	  	EF_OTAScript		0x6EA4
//#define   	EF_SubmitSyn        0x6EA5
//#define   	EF_ServiceList		0x6EA6		//OTA RECOMMEND FILE
//#define   	EF_RuntimeReg		0x6EA8		//OTA实时登记文件
//#define		EF_OTA_KEY			0x6EA9
//#define   	EF_CNTRFile			0x6EAA
//#define   	EF_OTAConfig		0x6EAB
//#define   	EF_TPDA				0x6EAC
//#define   	EF_DownLoadFile		0x6EAE
//#define		EF_OTABipTempFile 	0x6EAF		//缓存BIP通道接收数据
//#define	  	EF_LocalInfoConfig	0x6ED1		//位置服务
//#define		EF_RemoteCardLockCfg	0x6EE1		//远程短信锁卡
//#define 	STKNAME             0x6F19
//#define		EF_OTAEventRegister	0x6EB0		//事件注册登记文件
//#define		EF_OTAEventContent	0x6EB1		//事件下载缓冲数据内容
//#define		EF_BrandConfig		0x6EC1		//品牌配置文件
//#define		EF_BrandCmdData		0x6EC2		//品牌下行缓冲文件
//#define		EF_BRANDINFO		0x6FFE	//品牌身份认证信息文件
//#define     EF_OTA_Tar          0x4E01
//#define		EF_JS_FETION		0x6D01

//------------------SPB Group---------------------//
//#define 	EF_ADNIndex				0x6E01
//#define 	EF_ADNGroup				0x6E02
//#define 	EF_NameIndex			0x6E04
//#define 	EF_SECURE_EEPMOVE		0x6E03
//#define 	EF_Protect				0x6E05
//#define 	EF_PROTECTION_FILE		0x6E06
//#define 	EF_SPBCfg				0x6E07
//#define     DF_SPBV3			    0x2700

//#define 	DF_AdnFile			0x7F10
//#define 	EF_AdnFile1			0x6FB0
//#define 	EF_AdnFile2			0x6FB1
//#define 	EF_AdnFile3			0x6FB2
//#define 	EF_AdnFile4			0x6FB3

//#define   DF_UICCLTE_PRIVATE          	       0x2800
//#define   EF_AUTH_MODE                         0x2A01

/******************CATTP******************/
#define     DF_CATTP               0xA300
#define 	EF_CATTPPUSH           0x3A01  // 记录文件,两条,每条长度为FF ,保存BIP 和CAT-TP数据，初始化为全FF
#define     EF_PDUFILE             0x3A02  //存储当前PDU中的数据字段 ,大小为一条SDU的最大值,初始化为0，存放数据格式UsedSpace(WORD),SEQ1(WORD),LEN1(WORD),DATA1 ,SEQ2(WORD),LEN2(WORD),DATA.....。每次开机或数据处理完毕，该文件清0
#define     EF_EACKSEQ			   0x3A03  //存放OUT OF SEQ，LV结构LBYTE，V  word+word+word ,已按序排列，初始为0
#define     EF_CATTPDuanDian       0x3A04   //5字节，第一字节保存写数据tag ，后续字节保存tar值
#define     EF_CATTPSave           0x3A05  //缓存当前收到的PRL数据totalUsedSpace index1 usedspace data1 index2 usedspace data2...

#define     PRL_SMS_SIZE           0x900
/*************************BIP**************************************/
#define   DF_BIPModule                0xA400
//#define   EF_BIPModule_Receive        0x4A01       //存放bip的接收数据， LV 结构， 在BIP通道打开成功之后清
//#define   EF_BIPModule_Send           0x4A02       //存放bip的发送数据，LV 结构， 在BIP通道打开成功之后清


/***************** 实现HTTP功能时私有文件定义 ************************/
#define  DF_TLS                  0xA100		//GP_AnnexB_DIR		
#define  EF_TLSsessionData       0x1A01  	//EF_TLSsessionData	//该文件用于会话记录，cos内部使用

#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
#define  EF_TLSKey 				 0x1A02		//EF_TLSKey			//该文件用于保存TLS握手成功之后的相关密钥，包括客户端的MAC KEY，服务器的MAC KEY，客户端的ENC KEY,服务器的ENC KEY,客户端的IV，服务器的IV
#endif

#define  EF_HTTPTriggerPara      0x1A03		//EF_HTTPTriggerPara	//记录文件，记录1存储Connection parameters(84), 记录2存储Security parameters(85)，记录3存储Retry policy parameters(86)，记录4存储Administration Host parameter(89-8a)，记录5存储Agent ID parameter(89-8b)，记录6存储Administration URI parameter(89-8C),该文件不用清，每次下来触发SMS时会重新写入
//#define  EF_HTTPSCommandData     0x1A04  	//EF_HTTPSCommandData 	//存放http解密后的明文APDU指令流, 用于给到226模块处理的指令数据 //BIP开通道成功之后/HTTP会话结束之后清除
#define  EF_HTTPSResponseData    0x1A05  	//EF_HTTPSResponseData	//存放APDU执行后的响应数据，不能和sms的响应数据复用同一个文件，因为euicc-M2M-HB离线脚本7通知流程用例 中有在http会话进行的时候来sms短信

#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
#define  EF_HTTPTwoSession		 0x1A08	 //用于存储当前HTTP会话过程中，再次下来的触发参数，包括触发模式，触发参数, 不支持TWO SESSION时不需要创建该文件
#endif

#if COS_UPDATE_ONLINE_MUTUAL
#define  EF_HTTPUPCOS		 0x1A09	 //用于存储当前HTTP会话过程中，接收到携带http参数的upcos维护指令，包括触发模式，触发参数, 不支持 upcos 时不需要创建该文件
#endif

/**************************以下为 EF_HTTPRFM的描述**************************************************************/
#define HTTP_RFM_Count_Offset                      0x00           //BYTE类型，表示当前执行的HTTP RFM的指令条数
#define HTTP_RFM_DfAddr_Offset                     0x01           //UWORD类型 
#define HTTP_RFM_EfAddr_Offset                     0x05           //UWORD类型 
#define HTTP_RFM_DfName_Offset                     0x09           //WORD类型
#define HTTP_RFM_EfName_Offset                     0x0B           //WORD类型
#define HTTP_RFM_ADfName_Offset                    0x0D           //WORD类型
#define HTTP_RFM_RecNum_Offset                     0x0F           //BYTE类型
#define HTTP_RFM_CommandMode_Offset                0x10           //BYTE类型
#define HTTP_RFM_Channel_Offset                    0x11           //BYTE类型

#define PETARNum_Offset                 0x00   //tarNum 在tempBuf中的存储偏移
#define PETARValue_Offset               0x01   //tarValue在tempBuf中的起始偏移
#define PETarMSL_Offset                 0x10     //One byte
#define PEAID_Offset                    0x11   //LV结构，最大17字节
#define PEADFAid_Offset                 0x22     //LV结构，最大17字节
#define PEAIDAccessDomain_Offst         0x33    //最大5字节
#define PEAIDAdminAccessDomain_Offset   0x38     //最大5字节
#define PEUICCAccessDomain_Offset       0x3D     //最大5字节
#define PEUICCAdminAccessDomain_Offset  0x42     //最大5字?
#define PEsecurityDomainID_Offset       0x47     //关联的安全域 默认是MOSD
#endif
