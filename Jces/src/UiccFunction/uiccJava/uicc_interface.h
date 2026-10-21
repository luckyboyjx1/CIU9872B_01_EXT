#ifndef _UICC_INTERFACE_H_
#define _UICC_INTERFACE_H_

#define POR_OK       0x00

#define    memcpy_G      ecpMem_Copy

#define	RecEmpty     0xFD
#define FunItemID    0xFC

//************************************开机导航***************** 
#define		OpenNavigationFinish		0x08		//BIT 4 @ 开机导航显示返回
#define		SpecSelectItemSwitch		0x80		//BIT 8 @ 特殊SelectItem(显示一级目录)

//UNDER 2700 一卡多号
//#define	 	EF_CARDNUMBER		0x6F0C			//存放号码名称文件（DF2700）
//
#define		SolidifyNum					0x03	//该变量的值取决于各省份需求菜单的固化更名目录的个数

//#define		AppRecomendMenuId			0xFF	
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
#define		D_CrcValue				    0x00    //下行数据的CRC值，2bytes
#define		D_UserLevel					0x03	//82    EF_BrandCmdData中的用户级别偏移
#define		D_BrandDisplay				0x06	//83	EF_BrandCmdData中的品牌显示内容的偏移
#define		D_UserLevelDisplay			0x3A	//84	EF_BrandCmdData中的用户级别显示内容的偏移
#define		D_AppId						0x4A	//86	EF_BrandCmdData中的业务标识的偏移
#define		D_UserInf					0x4F	//87	EF_BrandCmdData中的用户资料信息的偏移
#define		D_VipCardNum				0x7F	//88	EF_BrandCmdData中的VIP卡号的偏移


#define		D_CrcValue				    0x00    //下行数据的CRC值，2bytes
#define		D_UserLevel					0x03	//82    EF_BrandCmdData中的用户级别偏移
#define		D_BrandDisplay				0x06	//83	EF_BrandCmdData中的品牌显示内容的偏移
#define		D_UserLevelDisplay			0x3A	//84	EF_BrandCmdData中的用户级别显示内容的偏移
#define		D_AppId						0x4A	//86	EF_BrandCmdData中的业务标识的偏移
#define		D_UserInf					0x4F	//87	EF_BrandCmdData中的用户资料信息的偏移
#define		D_VipCardNum				0x7F	//88	EF_BrandCmdData中的VIP卡号的偏移

#define     OTA_RFM                     0xB0    //当前RFM是OTARFM
#define     OTP_RFM                     0xE0    //当前RFM是OTPRFM
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

extern BYTE	xdata G_SMItemLen;
extern BYTE xdata G_IconNum;
extern BYTE xdata G_NetworkStatus;
/************************************function define*************************/
extern void IF_SendATR(BYTE appVerId[2]);

extern void IF_ReceiveAPDU(void);

extern BYTE IF_ReceiveData(void);

extern void StoreProfileData(void);

extern BYTE ProcessEnvelopData(void);

extern BYTE IF_STKStatusCommand(void);

extern BYTE IF_StatusCommand(void);

extern void IF_ProcessPendingMessageAlert(BYTE pendingTag,BYTE refrence);

extern void IF_SetUpMenu(void);

extern BYTE MenuSelection(void);

extern BYTE SetUpMenu(void);

extern void PP_Download_Hanlder(void);

extern BYTE Check_UDHI(BYTE udhi);

extern void IF_ProcessSystemProactiveCommand(void);

extern BYTE IF_SelectByDFName(void);

extern BYTE IF_ExternalFileUpdate(WORD position,WORD len);

extern void ProcessNoShariableFileForToolkit(jshort trigerMode);

extern void IF_InitNoShareableFile(void);

extern BYTE IF_ProcessNoShareableFile(UWORD fileAddr,WORD fileName,BYTE mode,BYTE apiMode);

extern void IF_ProcessNoShareableFileListAfterDeleteFile(void);

extern BYTE NoShareableFileIsValidAndSetBit(UWORD fileAddr,WORD fileName,BYTE mode,BYTE apiMode);

extern void ProcessNoShareableFileAfterDeleteApplet(BYTE apprec);

extern BYTE IF_AccessConditionResizeFile(UWORD fileAddr,BYTE mode);

extern BYTE GetNoShareableFilePath(UWORD efAddr,WORD efName,UWORD dfAddr,WORD dfName,BYTE *buf);

extern void IF_NoShareableFileAfterDeSelectApplet(BYTE channel);

#define     TerminalProfile_Mode        				0x0			
#define     Openchannel_Mode                    0x1    //openchannel

extern BYTE SetupEvenList(BYTE mode);

extern BYTE CheckTar(BYTE *inputtar,BYTE *tarType);

extern BYTE GetKicKidKikKey(BYTE keyinfo,BYTE *buff,BYTE keytype,BYTE readKeyLen);

extern BYTE ToolkitCalculateMac(UWORD eeAddr,BYTE spi,BYTE kic,BYTE kid,BYTE cplLength,BYTE ccLength,WORD length,BYTE * resultBuffer);

extern BYTE FormattedRFMSMSCheckCounter(BYTE spi,BYTE * counterPointer)  ;

//extern BYTE IF_ApduFinishLength(BYTE length);

extern BYTE FindRFMTars(BYTE *tarBuf);

extern BYTE CheckRFMMinimumSecurityLevel(BYTE * spikickid);

extern BYTE IF_SIMAPIPrivateEventSupport(jbyte event,BYTE * eventBit);

extern BYTE IF_transLocalEventTypeToStandardEvent(BYTE event,jbyte * standardEvent);      //增加判断校验pin事件的接口

extern BYTE IF_CheckTarSupportSingleDES(BYTE * spiKicKid);

extern BYTE IF_GetPPDownloadKeyVersion(BYTE *processResult,BYTE * spiKicKid);

extern BYTE IF_RFMRAM_SameTar(BYTE * spiKicKid, BYTE cplLength);

//extern UWORD IF_EnvelopGetKeyAddrBySDVersion(BYTE sdId, BYTE keyVersion);
extern WORD IF_EnvelopGetKeyAddrBySDVersion(BYTE sdId, BYTE keyVersion, BYTE keyId, BYTE *keyBuff);

extern BYTE IF_CheckChinaMobileTar(BYTE * tar);

extern BYTE IF_HttpsTransactionGoOn(void);//added by xuweihua@20210625: 8A07文件映射到1A05,为了避免数据冲掉，https处理过程中短信先缓存暂时不处理

extern BYTE IF_ExcutePrivateFormattedEnvelopSMS(BYTE *currentOTATar, BYTE reference, BYTE headNo, BYTE smsType, BYTE eventId, BYTE *tpoa);

extern BYTE OTAGetListItem(BYTE menuId, UWORD fileAddr, BYTE type);

extern BYTE OTAPageListInformation(BYTE type, BYTE* pages, BYTE numPage, BYTE * menu);

extern BYTE IF_OTAPPHander(BYTE * dataptr);

extern void IF_OTAPPHanlder_Register(void);

extern void IF_OTAPPHander_DelAid(BYTE *dataptr);

extern void IF_OTAAppDelNotice(BYTE *aid);

extern BYTE IF_AccessDefined(BYTE viewIndex,WORD fileName);

extern void IF_PP_Download_Hanlder(BYTE tar, BYTE tarPos, BYTE udPos);

extern WORD WhiteCardSingleMsgProcess( BYTE reference, BYTE headNo, BYTE smsType, BYTE *tpoa);

extern BYTE IF_MainReceiveApdu(void);

extern BYTE IF_SelectNativeJavaAPP(BYTE *aidBuffer,BYTE aidLen);

extern void WhiteCard_Main(void);
extern BYTE IsWhiteCardApplet(BYTE* aidBuf);

extern BYTE IF_ToolkitSaveFirstFomatData(BYTE udh);

extern BYTE IF_ToolkitSaveRFMSMS(BYTE udh);

extern void IF_TerminalProfile(void);

extern BYTE IF_EnvelopeCommand(void);

extern void ProcessUICCPara(void);

extern void throwPrivateError(void);

extern BYTE SearchClosestSCP80SDId(BYTE appId);

extern u8 AppAPI_SelectByFileId(PX8 apduBuffer, PX8 errorCode);

extern void ThrowErrorBackToMain(WORD exceptionType);

extern BYTE IF_SendPor(void);

extern void IF_TriggerRemoteFileUpdate(void);

extern BYTE GetTLLength(UWORD dataAddr);

extern WORD GetDataLength(UWORD dataAddr);

extern BYTE IF_GetCurrentFilePath(BYTE *pathInfo);

extern void RunODDAuth(void);

extern BYTE SetSTKBusy(void);

extern BYTE ODDAuth(void);

extern void SetJavaReturnSW(u16 status);

extern void ClearTheVolatileByteArray(void);

extern BYTE GetBERTlvLen(UWORD srcAddr,UWORD* len);

extern void IF_UICCInition(void);

extern void IF_ApduFinish(void);

extern void UICC_Toolkit_FileView_Init(void);

extern void ResumePrlData(void);
//-------------提示语----------------------
extern BYTE code  liebiaokong[];

extern BYTE code recordIsEmpty[];

extern BYTE code  prePage[];

extern BYTE code  lasPage[];

extern BYTE code  menuManager[];

extern BYTE code fetion_app_offset[];

extern BYTE code fetionAid[];

extern void IF_WriteGBAKsFile(void);

extern WORD IF_Process91xx(WORD status,BYTE ins,BYTE channel,BYTE isUICC);
/******************Menu Tag Define*******************************/
#define ICONQULIFIER_SELFEXPLANATORY            0x00

#define ICONQULIFIER_NOTSELFEXPLANATORY         0x01

#define ICONIDENTIFIER_DEFAULT                  0x01

#define ICONIDENTIFIER_NOICON                   0x00
//-------------------------------------------------------------------------------------
// ------------------------ Simple-TLV Constants -------------------------
/** Simple-TLV : Command Details tag                             = 0x01 */
#define TAG_COMMAND_DETAILS                      0x01 
/** Simple-TLV : Device Identities tag                           = 0x02 */
#define TAG_DEVICE_IDENTITIES                    0x02 
/** Simple-TLV : Result tag                                      = 0x03 */
#define TAG_RESULT                               0x03 
/** Simple-TLV : Duration tag                                    = 0x04 */
#define TAG_DURATION                             0x04 
/** Simple-TLV : Alpha Identifier tag                            = 0x05 */
#define TAG_ALPHA_IDENTIFIER                     0x05 
/** Simple-TLV : Address tag                                     = 0x06 */
#define TAG_ADDRESS                              0x06 
/** Simple-TLV : Capability Configuration Parameters tag         = 0x07 */
#define TAG_CAPABILITY_CONFIGURATION_PARAMETERS  0x07 
/** Simple-TLV : Called Party Subaddress tag                     = 0x08 */
#define TAG_CALLED_PARTY_SUBADDRESS              0x08 
/** Simple-TLV : SS String tag                                   = 0x09 */
#define TAG_SS_STRING                            0x09 
/** Simple-TLV : USSD String tag                                 = 0x0A */
#define TAG_USSD_STRING                          0x0A 
/** Simple-TLV : SMS TPDU tag                                    = 0x0B */
#define TAG_SMS_TPDU                             0x0B 
/** Simple-TLV : Cell Broadcast Page tag                         = 0x0C */
#define TAG_CELL_BROADCAST_PAGE                  0x0C 
/** Simple-TLV : Text String tag                                 = 0x0D */
#define TAG_TEXT_STRING                          0x0D 
/** Simple-TLV : Tone tag                                        = 0x0E */
#define TAG_TONE                                 0x0E 
/** Simple-TLV : Item tag                                        = 0x0F */
#define TAG_ITEM                                 0x0F 
/** Simple-TLV : Item Identifier tag                             = 0x10 */
#define TAG_ITEM_IDENTIFIER                      0x10 
/** Simple-TLV : Response Length tag                             = 0x11 */
#define TAG_RESPONSE_LENGTH                      0x11 
/** Simple-TLV : File List tag                                   = 0x12 */
#define TAG_FILE_LIST                            0x12 
/** Simple-TLV : Location Information tag                        = 0x13 */
#define TAG_LOCATION_INFORMATION                 0x13 
/** Simple-TLV : IMEI tag                                        = 0x14 */
#define TAG_IMEI                                 0x14 
/** Simple-TLV : Help Request tag                                = 0x15 */
#define TAG_HELP_REQUEST                         0x15 
/** Simple-TLV : Network Measurement Results tag                 = 0x16 */
#define TAG_NETWORK_MEASUREMENT_RESULTS          0x16 
/** Simple-TLV : Default Text tag                                = 0x17 */
#define TAG_DEFAULT_TEXT                         0x17 
/** Simple-TLV : Items Next Action Indicator tag                 = 0x18 */
#define TAG_ITEMS_NEXT_ACTION_INDICATOR          0x18 
/** Simple-TLV : Event List tag                                  = 0x19 */
#define TAG_EVENT_LIST                           0x19 
/** Simple-TLV : Cause tag                                       = 0x1A */
#define TAG_CAUSE                                0x1A 
/** Simple-TLV : Location Status tag                             = 0x1B */
#define TAG_LOCATION_STATUS                      0x1B 
/** Simple-TLV : Transaction Identifier tag                      = 0x1C */
#define TAG_TRANSACTION_IDENTIFIER               0x1C 
/** Simple-TLV : BCCH Channel List tag                           = 0x1D */
#define TAG_BCCH_CHANNEL_LIST                    0x1D 
/** Simple-TLV : Icon Identifier tag                             = 0x1E */
#define TAG_ICON_IDENTIFIER                      0x1E 
/** Simple-TLV : Item Icon Identifier list tag                   = 0x1F */
#define TAG_ITEM_ICON_IDENTIFIER_LIST            0x1F 
/** Simple-TLV : Card Reader status tag                          = 0x20 */
#define TAG_CARD_READER_STATUS                   0x20 
/** Simple-TLV : Card ATR tag                                    = 0x21 */
#define TAG_CARD_ATR                             0x21 
/** Simple-TLV : C-APDU tag                                      = 0x22 */
#define TAG_C_APDU                               0x22 
/** Simple-TLV : R-APDU tag                                      = 0x23 */
#define TAG_R_APDU                               0x23 
/** Simple-TLV : Timer Identifier tag                            = 0x24 */
#define TAG_TIMER_IDENTIFIER                     0x24 
/** Simple-TLV : Timer Value tag                                 = 0x25 */
#define TAG_TIMER_VALUE                          0x25 
/** Simple-TLV : Date-Time and Time Zone tag                     = 0x26 */
#define TAG_DATE_TIME_AND_TIME_ZONE              0x26 
/** Simple-TLV : Call Control requested action tag               = 0x27 */
#define TAG_CALL_CONTROL_REQUESTED_ACTION        0x27 
/** Simple-TLV : AT Command tag                                  = 0x28 */
#define TAG_AT_COMMAND                           0x28 
/** Simple-TLV : AT Response tag                                 = 0x29 */
#define TAG_AT_RESPONSE                          0x29 
/** Simple-TLV : BC Repeat Indicator tag                         = 0x2A */
#define TAG_BC_REPEAT_INDICATOR                  0x2A 
/** Simple-TLV : Immediate response tag                          = 0x2B */
#define TAG_IMMEDIATE_RESPONSE                   0x2B 
/** Simple-TLV : DTMF string tag                                 = 0x2C */
#define TAG_DTMF_STRING                          0x2C 
/** Simple-TLV : Language tag                                    = 0x2D */
#define TAG_LANGUAGE                             0x2D 
/** Simple-TLV : Timing Advance tag                              = 0x2E */
#define TAG_TIMING_ADVANCE                       0x2E 
/** Simple-TLV : Browser Identity tag                            = 0x30 */
#define TAG_BROWSER_IDENTITY                     0x30 
/** Simple-TLV : URL tag                                         = 0x31 */
#define TAG_URL                                  0x31 
/** Simple-TLV : Bearer tag                                      = 0x32 */
#define TAG_BEARER                               0x32 
/** Simple-TLV : Provisioning Reference File tag                 = 0x33 */
#define TAG_PROVISIONING_REFERENCE_FILE          0x33 
/** Simple-TLV : Browser Termination Cause tag                   = 0x34 */
#define TAG_BROWSER_TERMINATION_CAUSE            0x34 
/** Simple-TLV : Bearer Descrition tag                           = 0x35 */
#define TAG_BEARER_DESCRIPTION	              0x35 
/** Simple-TLV : Channel Data tag                                = 0x36 */
#define TAG_CHANNEL_DATA		              0x36 
/** Simple-TLV : Channel Data length tag                         = 0x37 */
#define TAG_CHANNEL_DATA_LENGTH	              0x37 
/** Simple-TLV : Channel Status tag                              = 0x38 */
#define TAG_CHANNEL_STATUS		              0x38 
/** Simple-TLV : Buffer Size tag                                 = 0x39 */
#define TAG_BUFFER_SIZE		              0x39 
/** Simple-TLV : Card reader identifier tag                      = 0x3A */
#define TAG_CARD_READER_IDENTIFIER               0x3A 
/** Simple-TLV : SIM/ME interface transport level tag            = 0x3C */
#define TAG_SIM_ME_INTERFACE_TRANSPORT_LEVEL     0x3C 
/** Simple-TLV : Other address (data destination address)        = 0x3E */
#define TAG_OTHER_ADDRESS                        0x3E 
/** Simple-TLV : Network Access Name                             = 0x47 */
#define TAG_NETWORK_ACCESS_NAME                  0x47 


/** Simple-TLV : mask to set the CR flag of any Simple-TLV tag   = 0x80 */
#define TAG_SET_CR                               0x80 
/** Simple-TLV : mask to reset the CR flag of any Simple-TLV tag = 0x7F */
#define TAG_SET_NO_CR                            0x7F   
//------------------------------------------------------------------------------------
	
#define TAG_COMMAND_DETAILS                       0x01		   
#define TAG_DEVICE_IDENTITIES                     0x02	
#define TAG_RESULT                                0x03	
#define TAG_DURATION                              0x04	
#define TAG_ALPHA_IDENTIFIER                      0x05	
#define TAG_ADDRESS                               0x06	
#define TAG_CAPABILITY_CONFIGURATION_PARAMETERS   0x07	
#define TAG_CALLED_PARTY_SUBADDRESS               0x08	
#define TAG_TEXT_STRING                           0x0D	
#define TAG_TONE                                  0x0E	
#define TAG_ITEM                                  0x0F	
#define TAG_ITEM_IDENTIFIER                       0x10	
#define TAG_RESPONSE_LENGTH                       0x11	
#define TAG_FILE_LIST                             0x12	
#define TAG_LOCATION_INFORMATION                  0x13	
#define TAG_IMEI                                  0x14	
#define TAG_HELP_REQUEST                          0x15	
#define TAG_NETWORK_MEASUREMENT_RESULTS           0x16	
#define TAG_DEFAULT_TEXT                          0x17	
#define TAG_ITEMS_NEXT_ACTION_INDICATOR           0x18	
#define TAG_EVENT_LIST                            0x19	
#define TAG_LOCATION_STATUS                       0x1B	
#define TAG_ICON_IDENTIFIER                       0x1E	
#define TAG_ITEM_ICON_IDENTIFIER_LIST             0x1F	
#define TAG_CARD_READER_STATUS                    0x20
#define TAG_CARD_ATR                              0x21	
#define TAG_C_APDU                                0x22	
#define TAG_R_APDU                                0x23	
#define TAG_TIMER_IDENTIFIER                      0x24	
#define TAG_DATE_TIME_AND_TIME_ZONE               0x26	
#define TAG_CALL_CONTROL_REQUESTED_ACTION         0x27	
#define TAG_AT_COMMAND                            0x28	
#define TAG_AT_RESPONSE                           0x29	
#define TAG_IMMEDIATE_RESPONSE                    0x2B	
#define TAG_DTMF_STRING                           0x2C	
#define TAG_LANGUAGE                              0x2D	
#define TAG_AID                                   0x2F	
#define TAG_BROWSER_IDENTITY                      0x30	
#define TAG_URL                                   0x31	
#define TAG_BEARER                                0x32	
#define TAG_PROVISION_REFERENCE_FILE              0x33	
#define TAG_BROWSER_TERMINATION                   0x34	
#define TAG_BEARER_DESCRIPTION                    0x35	
#define TAG_CHANNEL_DATA                          0x36	
#define TAG_CHANNEL_DATA_LENGTH                   0x37	
#define TAG_CHANNEL_DATA_STATUS                   0x38	
#define TAG_BUFFER_SIZE                           0x39	
#define TAG_CARD_READER_ID                        0x3A	
#define TAG_UICC_TERMINAL_TRANSPORT_LEVEL         0x3C	
#define TAG_OTHER_DATA_DESTINATION_ADDRESS        0x3E	
#define TAG_ACCESS_TECHNOLOGY                     0x3F	
#define TAG_DISPLAY_PARAMETERS                    0x40	
#define TAG_SERVICE_RECORD                        0x41	
#define TAG_DEVICE_FILTER                         0x42	
#define TAG_SERVICE_SEARCH                        0x43	
#define TAG_ATTRIBUTE_INFORMATION                 0x44	
#define TAG_SERVICE_AVAILABILITY                  0x45	
#define TAG_ESN                                   0x46	
#define TAG_NETWORK_ACCESS_NAME                   0x47	
#define TAG_REMOTE_ENTITY_ADDRESS                 0x49	
#define TAG_TEXT_ATTRIBUTE_TAG                    0x50	
#define TAG_ITEM_TEXT_ATTRIBUTE_LIST              0x51       
#define TAG_CONTACTLESS_STATE_REQUEST             0x53	
#define TAG_IMEISV                                0x62	
#define TAG_BATTERY_STATE                         0x63	
#define TAG_BROWSING_STATUS                       0x64       
#define TAG_NETWORK_SEARCH_MODE                   0x65  
//------------------------------------------------------------------------		
extern BYTE xdata G_IconNum;
extern BYTE xdata G_JavaMenuIndex;
extern BYTE xdata G_JavaMenuIndexBak;
extern WORD xdata   G_BlockOffset;
//***************************************************************************************
#define ALGV1 0x01
#define ALGV2 0x02
#define ALGV3 0x03

extern BYTE code GBAAppletAid[16];
extern BYTE IsGBAApplet(BYTE* aidBuf);
extern BYTE IsBufAllZeroOrFF(BYTE * buf,BYTE hex,BYTE len);

extern BYTE IF_GetFirstSCP80AESkeyByAppID(BYTE sdId, BYTE *keyBuff);
extern BYTE IF_ExcutePrivateUnformatSMS(WORD cploffset, BYTE * tpoa);
extern void IF_ProcessProactiveHandlerEvent(void);
#endif
