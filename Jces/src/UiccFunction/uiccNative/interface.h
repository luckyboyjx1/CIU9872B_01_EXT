#ifndef _INTERFACE_H_
#define _INTERFACE_H_
#include "typedef.h"

#define	RecEmpty		0xFD
#define FunItemID    0xFC

//************************************开机导航***************** 
#define		OpenNavigationFinish		0x08		//BIT 4 @ 开机导航显示返回
#define		SpecSelectItemSwitch		0x80		//BIT 8 @ 特殊SelectItem(显示一级目录)

//UNDER 2700 一卡多号
//#define	 	EF_CARDNUMBER		0x6F0C			//存放号码名称文件（DF2700）
//
#define		SolidifyNum					0x05	//该变量的值取决于各省份需求菜单的固化更名目录的个数

#define		AppRecomendMenuId			0xB7
//#define		AppRecomendMenuId			0xAF	//"业务推荐"的菜单ID ，对应6F18 中的Item项 原值0xAF
#define     AppManageMenuId             0xFC    //"应用管理"的菜单ID ，对应6F18 中的Item项

#define     AppIndusryManageMenuID      0xF8    //江苏"企业管理"的菜单ID

#define		D_BrandId					0x00	//EF_BRANDINFO中品牌标识的偏移	
#define		D_LevelId					0x02	//EF_BRANDINFO中级别标识的偏移
#define		D_DataContent				0x04	//EF_BRANDINFO中数据内容的偏移

#define		D_CrcValue				    0x00    //下行数据的CRC值，2bytes
#define		D_UserLevel					0x03	//82    EF_BrandCmdData中的用户级别偏移
#define		D_BrandDisplay				0x06	//83	EF_BrandCmdData中的品牌显示内容的偏移
#define		D_UserLevelDisplay			0x3A	//84	EF_BrandCmdData中的用户级别显示内容的偏移
#define		D_AppId						0x4A	//86	EF_BrandCmdData中的业务标识的偏移
#define		D_UserInf					0x4F	//87	EF_BrandCmdData中的用户资料信息的偏移
#define		D_VipCardNum				0x7F	//88	EF_BrandCmdData中的VIP卡号的偏移

#define		D_OtaKey_BaseRecNo		0x01	//OTA key 在密钥文件EF_OTA_KEY中的记录号
#define		D_OtaRFMKey_BaseRecNo	0x06	//OTA RFM key 在密钥文件EF_OTA_KEY中的记录号
#define		D_BrandKey_BaseRecNo		0x0B	//大唐品牌 key 在密钥文件EF_OTA_KEY中的记录号
#define		D_ZJKey_BaseRecNo			0x10	//其他品牌 key 在密钥文件EF_OTA_KEY中的记录号
#define		D_GDRemoteKey_BaseRecNo	0x15	//广东远程写卡 key 在密钥文件EF_OTA_KEY中的记录号
#define		D_OTPKey_BaseRecNo		0x1a	//OTP key 在密钥文件EF_OTA_KEY中的记录号
#define		D_OTPRFMKey_BaseRecNo	0x1F	//OTP RFM  key 在密钥文件EF_OTA_KEY中的记录号
#define		D_OTPAppKey_BaseRecNo	0x24	//OTP数据同步,应用开通, key 在密钥文件EF_OTA_KEY中的记录号
#define		D_OTPAdKey_BaseRecNo		0x29	//OTP广告更新key 在密钥文件EF_OTA_KEY中的记录号


////////////Variable Tag//////////////////////
#define		LDFileTPOATag			0xA1	//TPOA缓存标志
#define		LDFileDeliverTag		0xA2	//Deliver级联使用

/*--------------------------------------------------------------------------------------------*/
extern BYTE	 xdata	G_OpenControl;		//开机控制变量。
										//BIT1:通知更新事件列表；
										//BIT3:开机显示返回；
										//BIT4:开机导航显示返回
										//BIT5:鉴权成功;
										//BIT6:SetupMenu返回；
										//BIT7:开机显示完成；
										//BIT8:特殊SelectItem(显示一级目录SetUpMenu)
										//其他：RFU
extern BYTE xdata G_AppControl;	   		//开机执行OTA应用控制变量
										//bit0:   是否执行SetupMenu成功
										//bit1:   是否更新6F7E成功
										//bit2:   是否可以执行OTA应用

extern BYTE xdata G_RFMMAC;

extern UWORD   xdata G_MenuAddr;


extern BYTE xdata G_BigResendTag;

extern BYTE	xdata G_StatusCount;		//BIT4-BIT1:表示status个数
										//BIT8-BIT5:表示开机短信的重发次数

extern BYTE xdata G_OpenEnhance;		

//extern BYTE xdata G_HTTPRFMReceive;

#define BIT_COUNT_STATUS                0    //可以开始计数status
#define BIT_SENDING_SETUPEVENT          1	 //已发送Set Up Event List
#define BIT_SENDING_PROVIDELOCAL        2	 //已发送PROVIDE LOCAL
#define BIT_SENDING_POLLINTERVAL        3	 //已发送POLL INTERVAL
#define BIT_SENDING_POLLOFF             4	 //已发送POLL OFF
#define BIT_SENDING_LOGGINGSMS          5    //已发送登录信息
#define BIT_SUPPORT_POLLINTERVAL        7    //是否支持PollInterVal



extern BYTE xdata G_SetUpEventListSent;
extern BYTE xdata G_NotUpdateFileTag;
extern BYTE xdata G_WriteFlashTimes;
extern BYTE xdata G_ProtectFileID[0x30];

extern BYTE xdata G_K[3];

extern BYTE xdata G_USIMAlgorithmType;

extern BYTE xdata  VARIABLEBUFFER[0x80];

extern BYTE	xdata  G_TerminalProfile[0x20];

extern BYTE  G_CATTPOpenError[10];

extern WORD    xdata    G_DF_ID_USIM;
extern WORD    xdata    G_DF_ID_CSIM;

extern BYTE code ATR_PSEUDOFORUSER[15];


extern BYTE  xdata G_TerminalData[0x1C];
extern BYTE  xdata G_StatusEventList;
extern BYTE  xdata G_Cur_IMSI_RecNo; 




//-------------提示语----------------------
extern BYTE code  liebiaokong[];

extern BYTE code recordIsEmpty[];

extern BYTE code  prePage[];

extern BYTE code  lasPage[];

extern BYTE code RoamingTxt[];

extern BYTE code  menuManager[];

extern BYTE code fetion_app_offset[];

extern BYTE code fetionAid[];




//////////////////////////////////////////////////////

extern void IF_AfterReadPin(void);

extern WORD IF_SelectFile(WORD fileName);

extern void IF_PP_Download_Hanlder(BYTE tar, BYTE tarPos, BYTE udPos);

extern void IF_EepromAddressSetting(void);

extern void IF_SendATR(BYTE lgth);

extern void MEPowerOnDisplay(void);

extern BYTE IF_ReceiveData(void);

extern void IF_SendData(void);

extern void IF_ReceiveAPDU(void);

extern void IF_ApduFinish(void);

extern void IF_ProcessPendingMessageAlert(BYTE pendingTag,BYTE exaInfo);

extern void IF_UpdateRecord(BYTE recNo);

extern BYTE IF_UpdateBinary(void);	

extern BYTE IF_ParentDFFCP(UWORD fileAddr,WORD fileName);  

extern BYTE IF_ActiveAppCommand(void);

extern BYTE OTAPageListInformation(BYTE type, BYTE* pages, BYTE numPage, BYTE * menu);

extern BYTE OTAGetNameList(BYTE menuId, BYTE type,BYTE num,BYTE * buf, BYTE onOff);

extern BYTE GroupMemberList(BYTE index, BYTE num, BYTE * buf);

extern BYTE OTAGetListItem(BYTE menuId, UWORD fileAddr, BYTE type);

extern BYTE IF_GetVaribleAddr(BYTE variablename);

extern BYTE IF_WriteVariable(BYTE variablename);

extern void IF_TerminalResponse(void);

extern void IF_TerminalProfile(void);

extern WORD IF_EnvelopeCommand(void);

extern void IF_RFMPorFunction(BYTE result);

extern BYTE IF_EnvelopeCommandSTK(void);

extern void IF_SetDefaultBaud(BYTE * baud);

extern void IF_StatusCommand(void);

extern void IF_STKStatusCommand(void);

extern void IF_GetDfInformation(void);

extern BYTE SwitchActivePhone(BYTE phoneNum,BYTE mode);

extern void DoubleNumber(void);

extern BYTE IF_OTAPPHander(BYTE * dataptr);

extern void IF_OTAPPHanlder_Register(void);

extern void IF_OTAPPHander_DelAid(BYTE *dataptr);

extern void IF_OTAAppDelNotice(BYTE *aid);

extern BYTE IF_AccessDefined(BYTE viewIndex,WORD fileName);

extern BYTE IF_BeforeReceiveData(void);

extern BYTE IF_MainReceiveApdu(void);

extern void IF_ResumeResponseMode(void);

extern BYTE IF_NotUpdateFile(WORD dfName, WORD efName);

extern BYTE IsRoaming(void);       //判断是否处于漫游状态

extern void IF_STKTerminalResponse(void);

extern BYTE IF_ActiveAppCommand(void);

extern void IF_ProcessApp(void);

extern BYTE IF_SelectCommandP2Validity(void);

extern BYTE IF_SelectCommandP2Validity2(void);

extern BYTE IF_AccessCondition(UWORD fileAddr,BYTE action);

extern BYTE IF_LDFileDeliver(void);


extern BYTE FindMenuFileRecNum(BYTE menuId);

extern void UnicomSetUpMenu(void);

extern void UnicomResetIMSI(void);

extern BYTE IfUpdate6F7BAllow(void);


///////////////////////////////TD APP////////////////////////////////
extern void MyIdentifier(void);
extern BYTE Imvip(void);
extern void activityInfoTD3G(void);
extern void SIMNavigation(void);
extern void mobileStrongBox(void);
extern void SimInfor(void);

extern BYTE IF_OTAPPHandlerMac(BYTE cmdType);

extern BYTE IF_UpdateOTADownloadSet(void);

extern BYTE IF_RunAlgorithm(void);

extern BYTE TransICCIDFormat(UWORD fileAddr,BYTE * ptr);

extern WORD crc_calculate(WORD iniData,BYTE *ptr,WORD count);

extern void VerifySecurity(void);

extern void AnalyseSecurity(void);

extern void ResponsePack(BYTE errorCode);

extern BYTE IF_LiveCardBusy(void);

extern void ClearCattpMessageAlert(void);

extern BYTE GWCheckTar(BYTE * tarPointer,BYTE * outBuf);

extern void IF_MoveFlashResume(void);

extern UWORD IF_CreateDFFile(UWORD fileUseSize,WORD fileName);

extern UWORD IF_CreateEFFile(UWORD fileUseSize,WORD fileName);

extern BYTE IF_CreateFileAccessCondition(UWORD fileAddr,BYTE operAction);

extern UWORD IF_VMHeapAlloc(UWORD size);

extern BYTE IF_CmdINS(BYTE insByte);

extern BYTE IF_CommandAttemper(void);

extern BYTE IF_IOTServiceCmd(void);

extern BYTE IF_IOTGetResponseCommand(void);

extern void IF_CDMAGSM(void);

extern BYTE IF_VerifyChv(void);

extern void IF_VerifyChv2(BYTE mode);

extern BYTE	IF_IncreaseCommand(void);

extern void IF_UpdateRecordPRL(BYTE recNo);

extern void IF_ATRChange(void);

extern void IF_ResumePrlData(void);

extern void TerProSendSetUpEventList(void);


#endif
