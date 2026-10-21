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




#if !NATIVE_CARD_UICC
//-------P下的私有文件 
#define DF_PROFILECFG                    0xB100	
#define EF_PRIVATE_PUK                   0x1B01
#define EF_PRIVATE_PIN                   0x1B02

#define EF_PRIVATE_AUTHCFG               0x1B10  //2E00 	//用于保存鉴权时候配置（1字节）、OP内容和鉴权常数R和C
#endif

#define   EF_RAMExtendResponse         0x8A05 //(0x6F93)   //LV ??,L:2??,????????,TF??,??????0
#define   EF_HTTPRFM                   0x8A06 //(0x6F95)  //该文件转移到226模块中 //文件大小为0x12 用于存储HTTP的RFM过程中的DfAddr/EfAddr/DfName/EfName/AdfName/RecNum/CommandMode等，该文件的初始值为0
#define   EF_PorDataFile               0x8A07 //(0x6FFF)   //?????????POR??
#define   EF_ScriptChainingFile        0x8A0D //0x5E21    //intial: 0x00 byte 1:??Script Chaining Value,,????????context(ScriptValue,current directory, current file, current tag pointer, etc.) and the PIN verification),
//#define	  EF_RFM_TAR			       0x2B05 //(0x4E05) //????: tar(3??)+mslConfig(1??)+counterfig(1??)+counter(5??),????????FF
#define   EF_ToolkitRFMSmsHead	       0x8A01 //(0x6F7E)    //,????????0,LF,????????:smsType refrenceNumber EVENTID L ????(TLV)  ??(TLV)  8B-udl ,????????????????
#define   EF_ToolkitRFMSms             0x8A02 //(0x6F7F)    //smstype ,??,,??,?? ??????,??
#define   EF_EnvelopeHandlerBuf        0x8A08

#define   EF_STKAlert                  0x8A0C //(0x6EA7)    //标志文件   两字节一个标志

//-------------------------------------------------------------------------------------------------
//                                       文件表识的定义
//-------------------------------------------------------------------------------------------------
#define     DF_MF       	0x3F00
#define		TF_ATR			0x2F01
#define     EF_ARR      	0x2F06
#define     EF_KEY      	0x2E10

#define 	EF_AuthCFG			0x2E00			//用于保存鉴权时候配置（1字节）、OP内容和鉴权常数R和C

#if NATIVE_CARD_UICC
#define		EF_SQNCheck			0x2E01			//用于保存鉴权成功时候的SQN的内容，32（记录数）×6（字节数）
#else
#define		EF_SQNCheck			0x1B11			//用于保存鉴权成功时候的SQN的内容，32（记录数）×6（字节数）
#endif
#define		EF_CARDCFG			0x2E02			//卡片设置文件
#define   EF_FileNotUpdate    0x2E06
#define		EF_ICCID			    0x2FE2		
#define		EF_WIBMenuName		0x2001
#define		EF_WIBMenu			0x2100

#define		EF_KKID		    	0x2F02          //登陆上行  空卡序列号文件
#define     EF_FileNotUpdate    0x2E06

#define   DF_Toolkit          	       0xA800//(2800)  
#define   DF_Toolkit_P          	   0xB200 //(2800)
#define   EF_ToolkitApp       	       0x2B80 //(0x6F80)	 //该文件主要用于存放安装Toolkit App时的一些参数信息，如Timers Channels  MenuItems MSL TARs AccessDomain 等
#define	  EF_RFM_TAR			       0x2B05 //(0x4E05) //文件结构： tar(3字节)+mslConfig(1字节)+counterfig(1字节)+counter(5字节)，该文件初始化为全FF
#define	  EF_RFM_CounterFile		   0x2B08//counter
#define	  EF_HttpsTriggle_TAR		   0x2B15 //(0x4E05) //文件结构： tar(3字节)+mslConfig(1字节)+counter(5字节)，该文件初始化为全FF

#if UICC__FUNCTION__2B80_AT_FILEEND
#define   DF_Toolkit_P_Ex          	   0xBB00
#else
#define   DF_Toolkit_P_Ex              DF_Toolkit_P
#endif

#define		DF_TELECOM		0x7F10
#define		EF_SMSP			0x6F42
#define     EF_ADN			0x6F3A
#define     EF_SMS			0x6F3C
#define	  	EF_BRANDINFO	0x6FFE

#define     DF_PHASE    	0x7F20
#define	    DF_DCS1800		0x7F21
#define     EF_SST      	0x6F38
#define     EF_WRONGKI  	0x0010
#define     EF_RAND     	0x0020
#define     EF_IMSI     	0x6F07

#if NATIVE_CARD_UICC
#define     EF_KI       	0x6F1B
#else
#define     EF_KI       	0x1B1B
#endif

#define		EF_PLMNsel		0x6F30
#define	  	EF_STKName		0x6F54
#define		EF_FPLMN		0x6F7B
#define		EF_LOCI			0x6F7E
#define		EF_134			0xFF03
#define     EF_ACC      	0x6F78
#define     EF_BCCH      	0x6F74
#define		EF_SPN			0x6F46

#define     EF_OPLMNWACT	0x6F61
#define		EF_HPLMNWACT	0x6F62
#define		EF_EHPLMN		0x6FD9
#define	  EF_RFMFileList		0x9F00


#define     EF_Single4G_KI      0x2E1B


//----------------------------DF 7F25的文件定义--------------------//
#define   DF_CDMA               0x7F25
#if NATIVE_CARD_UICC
#define   EF_Ki         		0x6F1B
#else
#define   EF_Ki         		0x1B1B
#endif
#define   EF_PRL_SMS            0x6FFF
#define   EF_PRL_UPDATEFILE     0x6F30
#define	  EF_UIMID				0x6F42
#define	  EF_RUIMID				0x6F31
#define   EF_ESNME				0x6F38
#define	  EF_IMSIM				0x6F22
#define   EF_IMSIT				0x6F23
#define   EF_TMSI				0x6F24
#if NATIVE_CARD_UICC
#define	  EF_SSD				0x6FBB
#else
#define	  EF_SSD				0x1BBB
#endif
#define   EF_CDMA_SST			0x6F32
#define   EF_CDMA_ACC           0x6F2C
#define	  EF_CDMASST		    0x6F32

#define	  EF_SUME				0x6F54



#define		DF_ADF			0x7FF0
#define		EF_UST			0x6F38
#define		EF_EST			0x6F56

#define		EF_EventTemp	0x6F60//事件下载数据缓冲文件

#define	    EF_Registry		0x2E03
//---------------------短信签名------------------------------
#define	EF_SIGNATURE_CONTENT	0x6F9B
#define	EF_SIGNATURE_CFG		0x6F9A

#define		DF_BIP				0xA400
#define		EF_BipBuffer 	0x9101		//缓存BIP通道接收数据


//---------------------OTA------------------------//
#define		DF_STK				0x2700
#define   	DF_OTA				0x2700
#define		  EF_Menu				0x6F18
//#define	  	EF_TAR				0x6EA2
#define	  	EF_OTASecurity		0x6EA3
#define	  	EF_OTAScript		0x6EA4
#define   	EF_SubmitSyn        0x6EA5
#define   	EF_ServiceList		0x6EA6		//OTA RECOMMEND FILE
//#define		  EF_STKAlert			0x6EA7


#define     EF_OTAAlert         0x6EA7		//联通使用
#define   	EF_RuntimeReg		0x6EA8		//OTA实时登记文件
#define		EF_OTA_KEY			0x6EA9
#define   	EF_CNTRFile			0x6EAA
#define   	EF_OTAConfig		0x6EAB
#define   	EF_TPDA				0x6EAC
//#define   	EF_DownLoadFile		0x6EAE
#define		EF_OTABipTempFile 	0x6EAF		//缓存BIP通道接收数据
#define	  	EF_LocalInfoConfig	0x6ED1		//位置服务
#define		EF_RemoteCardLockCfg	0x6EE1		//远程短信锁卡
#define 	STKNAME             0x6F19
#define		EF_OTAEventRegister	0x6EB0		//事件注册登记文件
#define		EF_OTAEventContent	0x6EB1		//事件下载缓冲数据内容
#define		EF_BrandConfig		0x6EC1		//品牌配置文件
#define		EF_BrandCmdData		0x6EC2		//品牌下行缓冲文件
#define		EF_BRANDINFO		0x6FFE	//品牌身份认证信息文件
#define     EF_OTA_Tar          0x4E01


/*-------------------------RFM user define files----------------------*/
//#define     DF_RFM           	    0x7110
//#define 	EF_DownTempFile		    0x4E03					//2700/4E03
//#define		EF_RFM_Tar			    0x4E05
//#define	    EF_WIBRFM_Key		    0x4E06
//#define		EF_RFM_Counter		    0x4E09
//#define   	EF_RFM_Sc		        0x4E0A
//#define 	EF_RFM_PorConfig 	    0x4E0B		  //POR CONFIGFILE
//#define 	EF_RFM_PorConfig 	    0x6F04		  //POR CONFIGFILE

//#define 	EF_RFM_ScaTpoa 		    0x4E0C		  //the file saved download tpoa and smsc
//#define 	EF_RFM_SUCCESS 	        0x4E10
//#define 	EF_RFM_NOTSUCCESS 		0x4E11


//#define   	EF_WIB_TagFile        0x4E0F        //flag
//#define   	EF_RFM_Msl		        0x4E8A




/***************** 实现HTTP功能时私有文件定义 ************************/
#define  DF_TLS                  	0xA100		//GP_AnnexB_DIR		
#define  EF_TLSsessionData       	0x1A01  	//EF_TLSsessionData	//该文件用于会话记录，cos内部使用
#if GP__AMENDMENT__B__TLS_SessionKey_IN_FILE
#define  EF_TLSKey 					0x1A02		//EF_TLSKey			//该文件用于保存TLS握手成功之后的相关密钥，包括客户端的MAC KEY，服务器的MAC KEY，客户端的ENC KEY,服务器的ENC KEY,客户端的IV，服务器的IV
#endif
#define  EF_HTTPTriggerPara      	0x1A03		//EF_HTTPTriggerPara	//记录文件，记录1存储Connection parameters(84), 记录2存储Security parameters(85)，记录3存储Retry policy parameters(86)，记录4存储Administration Host parameter(89-8a)，记录5存储Agent ID parameter(89-8b)，记录6存储Administration URI parameter(89-8C),该文件不用清，每次下来触发SMS时会重新写入
//#define  EF_HTTPSCommandData     	0x1A04  	//EF_HTTPSCommandData 	//修改为使用页起始的eep地址存储，存放http解密后的明文APDU指令流, 用于给到226模块处理的指令数据 //BIP开通道成功之后/HTTP会话结束之后清除
#define  EF_HTTPSResponseData    	0x1A05  	//EF_HTTPSResponseData	//存放APDU执行后的响应数据，不能和sms的响应数据复用同一个文件，因为euicc-M2M-HB离线脚本7通知流程用例 中有在http会话进行的时候来sms短信
#if GP__AMENDMENT__B__SUPOORT_TWO_SESSIONS
#define  EF_HTTPTwoSession		 	0x1A08	   //用于存储当前HTTP会话过来中，再次下来的触发参数，包括触发模式，触发参数, 不支持TWO SESSION时不需要创建该文件
#endif





/*-------------------------MORE NUM(GSM一卡双号的定义文件)----------------------*/
#define	  	DF_NUMBACKUP		0x7F5F
#define     EF_KC_C             0x6EB0
#define     EF_BCCH_C           0x6EB4
#define     EF_SMSC_C           0x6EB2
#define	 	EF_KI_C				0x6EEB
#define	 	EF_IMSI_GSM_C		0x6EB7
#define	 	EF_ICCID_GSM_C		0x6EBD
#define     EF_LOCI_C			0x6EBE
#define	 	EF_RANDOM_C			0x6E20			
#define     EF_FALSEKI_C        0x6E10
#define	 	EF_ACC_C			0x6EB8
#define		EF_AuthCFG_C		0x6E00
#define	 	EF_CARDNUMBER		0x6F0C			//存放号码名称文件（DF2700）
#define	 	EF_MoreNumCfg		0x6F0B			//存放号码名称文件（DF2700）
#define   EF_AutnCfg_C   0x6EE0			//
#define   EF_AutnSQN_C   0x6EE1			//  

//UNDER 7F10 开机菜单导航开关
#define	   EF_OpenNavigation 0x6FF8
#define    EF_ActivityInfo   0x6FFD

//UNDER 2700 移动保险箱


//---------------V4防克隆--------------------------------------------//

#define   EF_WrongKi    		0x0010			//伪ki(7F20)
#define   EF_Random     		0x0020			//7F20

#endif
