#ifndef _OTA3HEAD_H_
#define _OTA3HEAD_H_


#define 	TYPE_App_Name   		    0x02   //	0x22
#define 	TYPE_Menu_Dele  	    	0x08   //	0x28
#define 	TYPE_Download_List			0x15
#define 	TYPE_Adn_List				0x00
#define 	TYPE_Sms_List				0x03

#define 	D_MaxServiceNum				30		//信息同步时，每条上行短信中最大的服务索引个数

#define     MaxProcess                  3       //卡片最多支持过程的3级调用

///////////////////////////OFFSET macro definition[6F18]///////////////////////////////////
#define 	MENU_OFFSET_TYPE				0x00
#define 	MENU_OFFSET_ID					0x01
#define 	MENU_OFFSET_MENUID				0x02
#define 	MENU_OFFSET_PAGE_NUMBER			0x03
#define 	MENU_OFFSET_MENU_ATTRIBUTE		0x04
#define 	MENU_OFFSET_NAME				0x05

#define 	R_OFFSET_TYPE				0
#define 	R_OFFSET_AID				1
#define 	R_OFFSET_APP_SIZE 			4
#define 	R_OFFSET_APP_NAME 			6

/////////////////////////operation mode macro definition////////////////////////////
//操作类型的定义，按照远程目录管理下行类型定义
#define 	ACCESS_ENABLE_CATALOG		0x00	//启用目录
#define 	ACCESS_DISABLE_CATALOG		0x01	//禁用目录
#define 	ACCESS_READ			0x02	//读操作
#define 	ACCESS_DELETE_CATALOG		0x03	//删除目录
#define 	ACCESS_WRITE			0x04	//写操作
#define 	ACCESS_SHOW_MENAGE_MENU		0x05
#define 	ACCESS_HIDE_MENAGE_MENU		0x06
#define		ACCESS_LOCATE			0x07

#define		D_EnableApp				0x80	
#define		D_DelTagApp				0x20
#define		D_DelNotFinishTagApp	0x30
#define		D_AllAID				0x00
#define		D_AcitveAID				0x80
#define     D_ListTagApp            0x22        //显示应用列表。不包括禁用或已删除的应用
#define     D_CloseApp              0x24        //应用关闭
#define     D_DelMoTagApp           0x28        //已删除或允许MO删除

#define		D_CNTRA				0x00
#define		D_CNTRB				0x01
#define		D_CNTRD				0x04

#define		D_OTAAppDown		0x01
#define		D_OTAAppUpdate		0x09
#define		D_OTAInterDown		0x0A




//////////////TAR定义//////////////
#define		OTA_APP_TAR			0x10
#define		OTA_RFM_TAR			0x1F
#define		ZJ_BrandChange_TAR	0x1E
#define		BrandChange_TAR		0x11

//////////////////////////tag macro definition/////////////////////////////////////
//#define 	ID_MENUMANAGE 			0xFC
#define 	ID_MENUMANAGE 			0x11
#define		ID_NEXTPAGE				0xFD
#define		ID_LASTPAGE				0xFE
#define 	ID_CONTINUE_DELETE		0xF0
#define 	ID_FINISH_DELETE		0xF1
#define 	ID_BACK					0xF2
/////////////////////////Variable Tag for COS///////////////////////
#define     OTASynchronizTag         	0xA1
#define		MenuSearchResult			0xA2
#define		SearchKeyWordTag			0xA3
#define		OTABigDataResult			0xA4
#define		OTADownloadedAID			0xA5
#define		OTAMenuIdTag				0xA6
#define		OTAAidTag					0xA7
#define		OTATPDATag					0xA8
#define     OTAIpTag                	0xA9
#define		OTAAppDelNoticeTag			0xAA
#define     OTADirectoryNotice      	0xAB
#define     OTAAPPOpenClose        		0xAC
#define     OTADowUpdateAPPTag      	0xAE
#define     LocalOTAAppDelNoticeTag     0xAF
#define     LocalDisableEnableTag       0xB0
#define     OTANoDownloadAidTag         0xB2
#define     LocalOTAAidFirstTag         0xB3
#define     LocalOTAAidSecondTag        0xB4

///////////////////////KEY TYPE////////////////////////////////////////////////////
#define		D_OTAKey					0x01	//OTA上下行计算mac的key 
#define		D_RFMKey					0x06	//OTA远程文件更新使用的key 
#define		D_ZJ_BrandKey				0x10	//其他品牌 key 
#define		D_BrandKey					0x0B	//大唐品牌 key 

#define  		D_WriteCardKey        			0x15   	//广东远程写卡 key 

#define		D_OTPKey					0x1a	//OTP上下行计算mac的key 
#define		D_OTPRfmKey			    	0x1f	//OTP远程文件更新使用的key 
#define		D_OTPAppKey				0x24	//OTP数据同步,应用开通,使用的key
#define		D_OTPAdKey				0x29	//OTP广告更新,使用的key 
//////////////////////////other macro definition//////////////////////////////////
#define 	MO_Download				0x01		//MO下载请求
#define 	List_Update				0x02		//主动列表更新请求
#define     BigData_ACK             0x06        //大数据接收确认
#define 	Menu_Search				0x07		//菜单搜索
#define		D_AppClose				0x0A		//应用关闭
#define		D_AppOpen				0x0B		//应用开通

#define		D_TmpBufStart			232

#define 	D_NullRecord				0x00
#define 	D_FullRecord				0xFF
///////////////////////////Error Type Definition/////////////////////////////////
#define		ErrorNoApp			0xFFFF
#define		ErrorNoNextPage		0xFF

#define		ERROR				0xFF
//////////////////////////OTA command type//////////////////////////////////////
#define     OTA_EVENTANALYSE                    0xF6          
#define		OTA_EVENTREG						0xF7
#define     OTA_CALLUPLINKCOMMAND               0xF8
#define     OTA_RETURN                          0xF9
#define 	OTA_BACKWARD						0xFB
#define     OTA_CALLPROCESS                     0xFC
#define 	OTA_JMP								0xFE
#define 	OTA_FUNCTIONCALL					0xFF

#define 	OTA2_EX_PAGEDISPADN					0x01
#define 	OTA2_EX_ADNTOASCII					0x02
#define 	OTA2_EX_ASCIITOADN					0x03
#define 	OTA2_EX_LOCATIONINFOTOASCII			0x04			
#define 	OTA2_EX_SETPOSITIONSERVICE			0x05
#define 	OTA2_EX_SAVETOEDITBUF				0x06
#define 	OTA2_EX_COPYVAR						0x07
#define 	OTA2_EX_DISPLAYVAR					0x08
#define 	OTA2_EX_DELETEOTAVARCONTENT			0x09
#define 	OTA2_EX_FINDNULLOTAVAR				0x0A
#define 	OTA2_EX_LEFTTEXPAND					0x0B
#define 	OTA2_EX_RIGHTEXPAND					0x0C
#define 	OTA2_EX_STRINGCOMPARE				0x0D
#define 	OTA2_EX_SUBSTRING					0x0E
#define 	OTA2_EX_ASCIITOUCS2					0x0F
#define 	OTA2_EX_REPLACECHAR					0x10
#define 	OTA2_EX_JMPZERO						0x11
#define 	OTA2_EX_JMPFROMEDITBUF				0x12
#define 	OTA2_EX_ADDOPERATION				0x13
#define 	OTA2_EX_SUBOPERATION				0x14
#define 	OTA2_EX_MODOPERATION				0x15
#define 	OTA2_EX_SENDTAB						0x16

#define OTA3_EX_ARRAY_COMPARE					0x17
#define OTA3_EX_ARRAY_FILL						0x18
#define OTA3_EX_INIT_PROACTIVE_COMMAND			0x19
#define OTA3_EX_APPEND_ARRAY					0x1A
#define OTA3_EX_APPEND_TLV						0x1B
#define OTA3_EX_CHECK_PROFILE					0x1C
#define OTA3_EX_COPY_VALUE_BY_TAG				0x1D
#define OTA3_EX_GET_GENERAL_RESULT				0x1E
#define OTA3_EX_SEND							0x1F
#define OTA3_EX_READ_BINARY						0x20
#define OTA3_EX_READ_RECORD						0x21
#define OTA3_EX_SELECT							0x22
#define OTA3_EX_UPDATE_BINARY					0x23
#define OTA3_EX_UPDATE_RECORD					0x24
#define OTA3_EX_EXTRACT_SMS_DATA				0x25
#define OTA3_EX_VERIFY_CHV_PUK					0x26
#define OTA3_EX_VARPAD_OPERATION				0x27	
#define OTA3_EX_KEY_OPERATION					0x28	
#define OTA3_EX_RANDOM_DATA_OPERATION			0x29
#define OTA3_EX_ENCRYPT_DECRYPT_OPERATION		0x2A
#define OTA3_EX_ABORT_TRANSACTION				0x2B
#define OTA3_EX_BEGIN_TRANSACTION				0x2C
#define OTA3_EX_COMMIT_TRANSACTION				0x2D
#define OTA3_EX_PKI_DIGITAL_SIGNATURE_OPERATION	0x2E
#define OTA3_EX_PKI_SIGNATURE_VERIFY_OPERATION	0x2F
#define OTA3_EX_DATA_HASHING_OPERATION			0x30	
#define OTA3_EX_LOAD_KEY_OPERATION				0x31
#define OTA3_EX_GET_AID							0x32	
#define OTA3_EX_STRING_MERGE					0x33
#define OTA3_EX_EXTRACT_ADN_DATA				0x34
#define OTA3_EX_BIT_OPERATION					0x35

#define OTA3_EX_LINK_FILE						0x36
#define OTA3_EX_ERASE_FILE						0x37
#define OTA3_EX_GET_BUFFER_LENGTH				0x38
#define OTA3_EX_HEX_TO_ASCII					0x39
#define OTA3_EX_ARRAY_COPY						0x3A
#define OTA3_EX_COUNT_OPERATION					0x3B

#define OTA3_EX_VIP_Businesscard				0x40
#define OTA3_EX_SPB3							0x41
#define OTA3_EX_PROMOTION						0x43

///////////////////////////BIP  macro definition/////////////////////////////////
#define BIPCurrentClosed		0x00			//通道已关闭
#define BIPCurrentOpened		0x01			//通道已打开
#define BIPNeedClosed			0x03			//通道已由卡端打开，且由卡端负责关闭




#define   EF_DownLoadFile		0x6EAE


//////////////////////////STK Alert Tag Definition/////////////////////////////////
#define		D_OTAUserRegistTag			0x04		
#define		D_DownListUpdateTag			0x0F		
#define		D_DownListDeleteTag			0x10		
#define		D_PushListClearTag			0x11		
#define		D_OTAPushUpdateTag			0x12		
#define		D_InterDownSucessTag		0xA0		
#define		D_OTASynchronizTag			0xA1		
#define		D_SetUpMenuTag				0xA2		
#define		D_RFMSucessTag			    0xA3			
#define		D_BrandChangeConfirmTag		0xA8		
#define		D_OpenDisplayTag			0xA9			
#define		D_OTARegisterTag			0xAB		
#define		D_BrandUpdateTag			0xAC		
#define		D_DataDownSucessTag			0xAD			
#define		D_MenuSearchTag				0xAF		
#define		D_OTABigDataResultTag		0xB0		
#define		D_BipPushOpen				0xB6		
#define		D_BipPushClose				0xB7		//BIP close
#define		D_CloseChannelTag			0xBA
#define		D_DataAvaiable				0xBB
#define		D_EventDataDownload			0xBC
#define		D_OTASetUpEventList			0xCD
#define		D_GDOTAEventRun				0xBE       	
#define		D_OTAAppDelNoticeTag	    0xC4        
#define     D_DirectoryNoticeTag        0xC5        
#define     D_OTAAppOpenCloseTag        0xC6        
#define		D_PopUpAppliction		    0xC7        
#define     D_UpdateDowloadConfirm      0xC8        
#define     D_PushListConfirm           0xC9        
#define     D_DownListUpdateConfirm     0xCA         
#define     D_DisableEnbleAppConfirm    0xCB        
#define     D_UpdateIPSmsNumber         0xBD        
#define     D_DisplayMenuManagement     0xCE       
#define		D_ChannelStatus				0xE5



//////////////////////////OTA STRUCT///////////////////////////////////////////////////

typedef __packed struct tagOTAResume
{
	WORD   destntnOffset;
	WORD   sourceOffset;
	WORD    lgth;
}OTARESUME;

////////////////////////////OTA Glouble//////////////////////////////////////////////
extern WORD  xdata  G_AppOffset;

extern WORD  xdata  G_STKAppOffset;

extern BYTE  xdata G_BigDataLastNum;				//记录大数据下载的分组最后一条或者重传的最后一条

extern BYTE  xdata G_BigDataGroupNum;				//记录大数据下载的分组的Index，下载成功时候要置零

extern UWORD xdata G_ByteCodeAddr;

extern BYTE  xdata G_HandleResponse;

extern BYTE  xdata G_Pid[3];

extern BYTE  xdata G_Cpid[3];

extern BYTE  xdata G_Command;

extern BYTE  xdata G_CmdIndex; 

extern UWORD xdata G_AppFileAddr;

extern UWORD xdata G_ServerListFileAddr;

extern BYTE xdata G_FileNum;

extern BYTE xdata G_PageListResult;

extern BYTE xdata G_OpenChannel;

extern BYTE xdata G_BIPChannelId;

extern BYTE xdata G_BIPDataLgth;

extern BYTE xdata G_ChannelStatusID;

extern BYTE xdata  G_CounterType;

extern BYTE xdata G_BIPPushCntr;

extern BYTE	 xdata  G_IsUpdateEventList;

extern BYTE  xdata 	G_EventAID[3];

extern BYTE  xdata  G_EventTag;

extern BYTE  xdata  G_PopUpAID[3];						//弹出应用的应用索引

extern BYTE  xdata  G_MTPerformResult; 

extern BYTE  xdata G_TerminalProfile[0x20];

extern BYTE	 xdata G_PPoint; 

extern BYTE  xdata G_PFlowStack[MaxProcess][4];

extern UWORD xdata G_PAddrStack[MaxProcess];

extern PathInfo xdata FilePathInfo;

extern BYTE    xdata    G_RFMTag;

extern UWORD xdata G_BipDataAddr;

///////////////////////////////////////////const variable//////////////////////////////
///////////////////////////提示语及菜单/////////////////////
extern BYTE code fasongshibai[];

extern BYTE code shanchuchenggong[];

extern BYTE code notenoughspace[];

extern BYTE code DeletionMgrMenu[23];

extern BYTE code  guanjianzi[];

extern BYTE code userSelect3[19];

extern BYTE code userSelectDefault[];
#ifndef LOCALOTA
extern BYTE code recommendation[18];
#else
extern BYTE code recommendation[10];
#endif

extern BYTE code downloadsuccess[];

extern BYTE code muluzhong[];

extern BYTE code muluDownloadSuccess[];

extern BYTE code noFitNum[];

extern BYTE code newrecommd[];

extern BYTE code newdownlist[];

extern BYTE code cleardownlist[];

extern BYTE code clearpushlist[];

extern BYTE code regsucce[];

extern BYTE code rigsterInfor[];

extern BYTE code downSuccess[];

extern BYTE code deleted[];

extern BYTE code updateSuccsee[];

extern BYTE code updateRequest[];
//------------------------------ota3menuManage.c--------------------------
extern BYTE code appClose[];

extern BYTE code MenuSet[0x1F];

extern BYTE code ParameterSet[0x1C];

extern BYTE code inputPIN1[];

extern BYTE code PINError[];

extern BYTE code inputIP[];

extern BYTE code IPError[];

extern BYTE code inputTPDA[];

extern BYTE code SetSuccess[];

extern BYTE code closeChannel[];

extern BYTE code closeAPP[];

extern BYTE code closeMuluApp[];

extern BYTE code closeAPPMulu[];
///////////////////////////////////////////OTAPublic//////////////////////////////
extern BYTE MenuAcess(BYTE menuId, BYTE offset, BYTE * buf, BYTE mode, BYTE menuType,UWORD fileAddr);

extern BYTE OTAMenuAccess(BYTE menuId, BYTE offset, BYTE *buf, BYTE mode, BYTE menuType);

extern WORD OTAGetAppOffset(BYTE CONST aid[3]);

extern WORD GetFreeAppSpaceSize(void);

extern BYTE OTADisplayDownloadSuccess(void);

extern void OTAProcessInterApp(void);

extern BYTE OTASubmitSynchronizeInfo(void);

extern void SendSubmitInfoSyn(BYTE aidCount,BYTE menuCount,BYTE SubmitType);

extern BYTE OTADataReceivedAck(void);

extern void PBOCAuth(BYTE key[16],BYTE sourceKey[8]);

extern BYTE OtaRfmMac(UWORD dataAddr,BYTE  cpKey[8],BYTE  cpResult[8],BYTE totalNum,BYTE packageNum);		//new

extern BYTE RfmDataInfo(UWORD addr,BYTE pIndex, BYTE bIndex);

extern BYTE OTAGetAid(BYTE menuId, BYTE searchType,BYTE num,BYTE * buf,BYTE onOff);

extern BYTE OTAGetAdnList(BYTE starRecNum,BYTE index,BYTE * buf);

extern BYTE GroupMemberList(BYTE index, BYTE num, BYTE * buf); 

extern void OTASetupCall(void);

extern void OTASynMenuName(void);

extern void SynAppMenuName(UWORD appAddr,UWORD menuAddr,BYTE synType);

extern BYTE OTASendUplinkCommandInBIPFormat(BYTE * uplinkData);

extern BYTE DataAvailable(BYTE pos);

extern void OTADataAvaiable(void);

extern void OTAPopUpApp(void);

extern BYTE AccessDefined(UWORD fileAddr,BYTE action);

extern void DeleteAppUnderMenu(BYTE menuId);

extern void ProcessOTAApp(BYTE aid[3]);
////////////////////////////OTACommon///////////////////////////////////

/////////////////////////////recommend /////////////////////////////////
#define	RegStatus_Get	0x00
#define	RegStatus_Set	0x01

extern void OTARecommendation(void);

extern BYTE GetSetRegStatus(WORD dfName, WORD efName, BYTE mode);

extern BYTE OTARigsterProgess(void);

/////////////////////////////menumanagement /////////////////////////////////
extern void OTAMenuHandler(BYTE * menuId);

extern BYTE OTAGetProcessPermission(BYTE item);

extern BYTE OTAServiceMgtMenu(BYTE menuId);

extern BYTE OTAMenuManageDownload(BYTE menuId);

extern BYTE OTAMenuManageDelete(BYTE menuId);

extern  BYTE OTAMenuManageDeleteCatalog(BYTE menuId);

extern BYTE OTAListRefresh(BYTE menuId);

extern BYTE OTAMenuManageSearch(BYTE menuId);

extern BYTE OTAMenuManageClose(BYTE menuId);

extern void OTADownloadSet(void);

extern void UpdateOTADownloadSet(void);

extern void UpdateDownloadConfig(BYTE configByte);

extern BYTE UpdeTPDA(void);

extern BYTE UpdateIp(void);

extern void OTABipPushHandler(BYTE type);

extern BYTE OTAOpenChannelHandle(void);

extern BYTE OTAVerfiryPIN(void);

////////////////////////////PP DownLoad API///////////////////////////////////////////

extern BYTE OTAPPHanlder(BYTE *dataptr);

extern BYTE OTAVarifyCNTR(BYTE cntr[5], BYTE cntrType, BYTE optType);

extern BYTE OTAGetKey(BYTE radom[4], BYTE key[16], BYTE keyType);

extern void singleDesMac(BYTE  * dataBuf,BYTE  cpKey[8],BYTE  cpResult[8],WORD len,BYTE mode); 

extern BYTE OTACaluteMac(BYTE * dataBuf, BYTE mac[4], BYTE key[16], WORD len, BYTE type);

extern BYTE OTACaluteBIPMac(BYTE * dataBuf, BYTE mac[4], BYTE key[16], WORD len);

extern void TripleDES_3Key(BYTE * pbSrcStr,BYTE * keyContext,BYTE * pbTgtStr, BYTE bMod, BYTE lgth);

extern BYTE OTARFMExcute(void);

extern void OTAMenuSearch(void);

extern BYTE OTAIsMenuActive(BYTE menuid);

extern BYTE ExcuteCommand(UWORD dataAddr, BYTE rfmDataInfo[3]);

extern void SaveRFMDataSms(BYTE ptr);

extern void SaveBIPRFMDataSms(BYTE ptr);

extern BYTE OTASendRFMResult(BYTE result, BYTE packageNum);

extern BYTE OTASaveDownLoadData(BYTE * dataBuf, BYTE dataType,BYTE pointer);

extern BYTE IsDataSmsReceived(BYTE index, BYTE largeCntr, BYTE * dataBuf);

extern void CheckLargeSms(BYTE * resultBuf, BYTE * dataBuf, BYTE largeCntr,BYTE index);

extern BYTE IF_OTAMgtMenuSelectItem(BYTE menuId);

extern BYTE OTADownListUpdate(BYTE * cmd);

extern BYTE IF_RecommendationSelectItem(void);

extern BYTE IF_RecommendationSetItem(BYTE item);
////////////////////////////Uplink API///////////////////////////////////////////

extern BYTE OTASubmitMenuSearch(BYTE pageNum, BYTE menuId, BYTE *keyWords);

extern void IncreatCounter(BYTE *cntr,BYTE counterLen);

////////////////////////////////Brand Change API////////////////////////////////
extern BYTE BrandPPHanlder(BYTE pos);

extern BYTE BrandChangeRegisterStatus(BYTE type);

extern BYTE BrandChangeRegConfirmation(BYTE * ud);

////////////////////////////////////////////////////////////////////////////////////
extern void GetUplinkMessage(BYTE pos, BYTE *cmdData ,BYTE mode);

extern BYTE OTASendUplinkCommandMessage(BYTE tpdaIndex,  BYTE *cmdData);

extern BYTE OTAMODownloadCmdData(BYTE *aid, BYTE *cmd);

extern void OTADeleteApp(BYTE IsDownloadList, BYTE type);

extern void OTAPureAppScript(UWORD securityFileAddr, WORD tmpOffset);

extern WORD OTASearchAID(BYTE menuId, BYTE searchType,BYTE onOff);

extern void OTAAppDataResume(void);

extern void OTADelAppFromServerList(BYTE index,BYTE * databuf);

extern void OTAAddAppTOServerList(BYTE type, BYTE index, BYTE *databuf, BYTE replace);

extern void WriteAppToList(BYTE recNum,BYTE *databuf,BYTE type);

extern BYTE OTALocatIndexInServerListFile(BYTE menuId);

extern BYTE OTAMenuAddItem(BYTE * buf,BYTE mode);

extern BYTE OTAGetSearchResultList(BYTE num, BYTE *buf);

extern void OTAClearCurrentSession(void);

extern BYTE OTARegisterCurrentSession(BYTE parameter[3], BYTE oprationType);

extern BYTE OTAVerifyCurrentSession(BYTE Parameter[3], BYTE operationType);

extern void IF_NO_UDHL_Hanlder(BYTE udPos);
//-----------------------OTA3_Interface.c-----------------------
extern void IF_OTASendSMS(void);

extern BYTE IF_OTAExFuntion(void);

extern void IF_OtaSetupCall(void);

extern  BYTE IF_OTAServiceMgtMenu(BYTE item,BYTE menuId);

extern BYTE IF_SetOtaRecommendationPidDcs(BYTE pidDcs[2]);

extern BYTE IF_OTAExExFuntion(void);

extern BYTE IF_OTADownloadSetItem(void);

extern BYTE IF_OTADownloadSetMenu(BYTE item);

extern BYTE ChannelStatus(BYTE pos);
//----------------OTA_AppExcute---------------------------//
extern void OTADisplayText(BYTE offset);

extern WORD OTAFindCmd(WORD offset,BYTE mode);

extern void OTAFindNextPID(BYTE pidLocal[3]);

extern void OTAFindPreviousPID(BYTE pidLocal[3]);

extern WORD OTAFindProcess(void);

extern void OTAGetInput(void);

extern BYTE OTAGetText(UWORD byteCodeAddr,BYTE tag,BYTE * ch);

extern BYTE OTAGetVariable(BYTE index,BYTE * des);

extern void OTAJmp(void);

extern void OTALaunchBrowser(void);

extern WORD OTALocateVarPos(BYTE idName );

extern void OTAPlayTone(void);	

extern void OTAProvideLocalInformation(void);

extern void OTARefresh(void);

extern BYTE OTASeekConst(BYTE id, BYTE * des);

extern BYTE OTASeekVariable(BYTE idName, BYTE * des);

extern WORD OTALocateVarConstPos(BYTE idName,BYTE mode);

extern BYTE OTASeekConstVar(BYTE id, BYTE * des,BYTE initialIndex);

extern void OTASelectItem(void);

extern void OTASendSMS(void);

extern void OTASendSsOrUssd(BYTE cmdType, BYTE tlvTag);

extern void OTASetupCall(void);

extern BYTE OTAStartRun(void);

extern void OTAOpenChannel(void);

extern void OTACloseChannel(void);

extern void OTAReceiveData(void);

extern void OTASendData(void);

extern void OTAGetChannelStatus(void);

extern void OTACallUpLinkCommand(void);

extern void OTACallProcess(void);

extern void OTAReturn(void);

//------------------OTAPlugIn---------------------//
extern void OTAAdnToAscii(void);

extern void OTAAsciiToAdn(void);

extern void OTAAsciiToUcs2(void);

extern void OTACopyVar(void);

extern void OTADeleteVarContent(void);

extern void OTADisPlayVar(void);

extern void OTAFindNullVar(void);

extern void OTAJmpFormEditbuf(void);

extern void OTAJmpZero(void);

extern void OTALocationInforToAscii(void);

extern void OTAModOperation(void);

extern void OTAPageDispAdn(void);

extern void OTAPlugInFuction(void);

extern void OTAReplaceChar(void);

extern void OTATextPand(BYTE tag);

extern void OTASaveToEditBuf(void);

extern void OTASendTab(void);

extern void OTASetpositionService(void);

extern void OTAStringCompare(void);

extern void OTASubAddOperation(BYTE command);

extern void OTASubString(void);

extern BYTE OTAUpdateVariable(BYTE idName, BYTE * src);

extern void OTA3ArrayCompare(void);

extern void OTA3ArrayFill(void);

extern void OTA3GetAID(void);

extern void OTA3StringMerge(void);

extern void OTA3BitOperation(void);

extern void OTA3InitProactiveCommand(void);

extern void OTA3AppendArray(void);

extern void OTA3AppendTLV(void);

extern void OTA3CheckProfile(void);

extern void OTA3CopyValueByTag(void) ;

extern void OTA3GetGeneralResult(void);

#define OTA3Send	SendProactiveCommand

extern void OTA3ReadBinary(void);

extern void OTA3ReadRecord(void);

extern void OTA3Select(void);

extern void OTA3UpdateBinary(void);

extern void OTA3UpdateRecord(void);

extern BYTE ComputePluginRecord(BYTE action,BYTE *cmd,BYTE recordNo);

extern void OTA3ExtractSMSData(void);

extern void OTA3VerifyCHVPUK(void);

extern void OTA3ExtractADNData(void);

extern void OTA3VarPadOperation(void);

extern void OTA3RandomDataOperation(void);

extern void OTA3EncryptDecryptOperation(void);

extern void OTA3KeyOperation(void);

////////////////////////////OTA3新增内容///////////////////////////////

extern void OTA3HexToAscii(void);

extern void OTA3CountOperation(void);

extern void OTA3EraseFile(void);

extern UWORD GetLinkEFAddr(BYTE tmp);

extern void OTA3LinkFile(void);

extern void OTA3GetBufferLength(void);

extern void OTA3ArrayCopy(void);

extern void OTA3EventRegister(void);

extern void OTAGdEventDownloadData(void);

extern void OTAGDSaveAppData(void);
	
extern void OTASaveEventData(BYTE type);

extern void OTAEventRun(void);

extern void OTAEventDataReg(void);

extern void OTARemoveEventReg(BYTE  aid[3], BYTE IsSetupEventList);

extern BYTE MoveTFContent(UWORD fileAddr, WORD srcOffset, WORD desOffset, WORD len);


//------------------------------------本地OTA---------------------------
#ifdef LOCALOTA
extern void LocalUpdateIPSmsNumberConfirm(void);

extern void LocalDownListPushListIPConfirm(BYTE pendingTag);

extern void LocalDisplayDownListUpdate(void);

extern BYTE LocalUpdateDowloadAPPConfirm(void);

extern BYTE LocalOTAAppDelNoticeConfirm(void);

extern BYTE LocalOTADisableEnableConfirm(void);

extern BYTE LocalMenuManageConfirm(void);

extern BYTE LocalAPPOpenCloseConfirm(void);

extern void ReadUpdateLocalJSPara(void);

extern void OTA3EventAnalyse(void);

extern void LocalOTA3SubAPI(void);

extern void subDisplayText(BYTE offset);

extern void ReadUpdateCounter(void);

extern void UpdateRegister(void);

extern void ReadOTAPara(void);

extern BYTE xdata G_MenuId[2];

extern BYTE xdata G_DeleteItem;

extern BYTE code menuListUpdate[];

extern BYTE code listUpdate[];

extern BYTE code localRegsucce[];

extern BYTE code UpdateParameterSet[30];

extern BYTE code EndAID[];

extern BYTE xdata BIPRfmIndex;

extern BYTE xdata G_LevelApp;

extern BYTE xdata G_NextPageTag; 
#endif
#endif


