
#ifdef LDFile

#include   "typedef.h"
#include   "eepromdefine.h"
//#include   "hard_api.h"

//////////////文件系统//////////////
#define		LDFile_KeyFile			0x5E01	//key文件(线性)
/*
说明：
	暂定10条记录，每条0x10长度，索引按记录号，DES时仅使用前8字节，3DES时使用16字节	
	每两条记录表示一个版本的密钥：第一条为KIC，第二条为KID 
*/

#define		LDFile_CounterFile		0x5E02	//counter文件（透明）
/*
说明：
	暂定长度为6字节，保存卡内counter+级联批次
*/

#define		LDFile_RelaxFile		0x5E03	//执行缓存文件（透明）
//短信和cattp解密后数据存放文件， 定义短信头解析时用到的宏长度
#define		SMS_Head_CplToPcntr		0x10	//短信头从CPL到PCNTR的长度值是固定的16字节：CPL CHL SPI KIC KID TAR CONTR PCNTR = 16

#define     LDFile_RFMPerso         0x5E04
/*
说明：
	暂定长度为526字节，
	结构为：L(2字节) + SPI(两字节) + KIC(1字节) + KID(1字节) +TAR(3字节) + counter(此次，5字节) + vlaue(N字节，不大于512字节)
*/

//#define		LDFile_TPOAFile			0x5E04	//保存远程文件管理的TPOA地址(线性)
/*
说明：
	暂定1条记录，每条长度为0x11，结构为：L（1字节）+ V（不大于0x10）
*/

//#define		LDFile_DownloadFile		0x5E11	//单条短信下行缓存文件（线性）
/*
说明：
	暂定包含3条记录，暂定每条记录长度为0xA0，
	结构为：L(1字节) + 最新标记(1字节) + RFU(2字节) + CPL······
*/
#define		LDFile_DownloadFileMuli	0x5E12	//级联短信下行缓存文件（线性）
/*
说明：
	暂定包含15条记录，暂定每条记录长度为0xA0，
	结构为：L(1字节) + 批次(1字节) + 总条数(1字节) + 已有条数(1字节) + CPL(或有效数据)······
*/

#define		LDFile_UpFile			EF_ResponseFile	//执行结果缓存文件（透明）
/*
说明：
	暂定长度为512字节，保存执行结果,结构为：RFU(两字节) + L(2字节) + V（结果数据）
*/

#define 	EF_TIMER               0x7E05  //透明文件，初始化为0102030405060708,记录长度为8个字节,存放timer,


#define    EF_FCL                   0x2F08  //EFFCL文件权限控制列表，线性文件，每条记录9字节长，初始化为0



#define   EF_ResponseFile           0x5E20    //LV结构,存储当前的R-APDU(包括执行的命令个数),前两个字节预留，第3.4字节为后续数据长度,该文件初始化为0
//EF_ResponseFile拼装响应数据时的偏移	native平台原有方案，增加宏定义是代码更具有阅读性
#define		RFM_RES_LEN_OFF				23				//写入必须信息  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU =23
#define		RFM_RESPONSE_COMNUM_OFF		25				//写入必须信息  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU + repLenth(2byte)=25
#define		RFM_RES_STATUS_OFF			26				//写入必须信息  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU + repLenth(2byte) + resComNum(1byte) =26
#define		RFM_RES_VALUE_OFF		    28				//写入必须信息  RPL + RHL +TAR +COUNTR + PCNTR +STATUS +CCLength(留8字节) +RFU +RFU + repLenth(2byte) + resComNum(1byte) + resStatus(2byte) =28


//////////////TAR定义//////////////
#define		TAR1_LDFile			0xB0	//远程文件管理

#define		TAR2_COMPRESS		0x00	//压缩型
#define		TAR2_EXTEND			0x01	//扩展型

#define		TAR3_SHARE_FILE		0x02	//共享文件
#define		TAR3_CSIM_ADF		0x20	//CSIM ADF
#define		TAR3_USIM_ADF		0x21	//USIM ADF
#define		TAR3_USIM_ADF_Add	0x00	//USIM ADF
#define		TAR3_ISIM_ADF		0x22	//ISIM ADF
#define		TAR3_SIM			0x11
#define		TAR3_HTTP_RFM		TAR3_USIM_ADF	//HTTP_RFM_TAR B20202 目前默认是USIM?

#define 	ILLEGAL_APPID		0xFF	//非法aid

//////////////宏定义//////////////
#define		LDFILEMLSD				0x02	//最小安全等级
#define		LDFileMaxNum			0x0F	//下行级联缓存文件最大记录条数
#define		LDFileSingleProgess		0x80	//标记单条接收完成
#define		LDFileMuliProgess		0x81	//标记多条接受完成

#define		PoROK					0x00	//指令执行成功
#define		RCCCDSFailed			0x01	//RC/CC/DS校验错误
#define		CNTRLow					0x02	//counter小于现有计数值
#define		CNTRHigh				0x03	//counter大于现有计数值
#define		CNTRBlocked				0x04	//counter锁定
#define		CipheringError			0x05	//解密信息错误
#define		UnSecurityError			0x06	//安全错误
#define		NoMemory				0x07	//存储空间不足
#define		MoreTime				0x08	//申请更多时间（此处主要用于PoR级联）
#define		TARUnknown				0x09	//TAR不明白
#define		OtherError				0x0A	//其他错误


#define   DataBiteOff       0xFF

//////////////SMSDisposal.c//////////////
//extern void LDFilePPHanlder(BYTE * dataBuf);
extern BYTE LDFileSPICheck(BYTE firstSPI);
//extern BYTE LDFileCheckCounter(BYTE counter[5], BYTE firstSPI);
extern BYTE LDFileCheckCounter(BYTE counter[5], BYTE firstSPI,BYTE counterId);

//extern BYTE LDFileCheckRCCCDS(BYTE * sourceBuf, BYTE * MacBuf);
//extern BYTE LDFileCheckRCCCDSPoR(BYTE * headBuf, BYTE * MacBuf, BYTE SPI2, BYTE KID, BYTE kind);
extern void LDFilePoR(BYTE errorTag);
extern BYTE LDFileMuliPPHanlder(BYTE refNumber,BYTE tag);
extern void LDFileCaluteMacMuli(BYTE * sourceHead, BYTE * key, BYTE * mac, UWORD fileAddr2, BYTE mode);
extern BYTE LDFileSMSPoR(BYTE * dataBuf, BYTE * headBuf, BYTE * outBuf, BYTE sum, BYTE num);
extern void LDFileSMSSubmit(BYTE * dataBuf);
extern void LDFileSMSDeliver(BYTE * dataBuf, BYTE errorCode);
//extern BYTE LDFileFindRecord(UWORD fileAddr, BYTE type, BYTE tag);
extern void LDFileProgess(BYTE tag);
//extern BYTE LDFileRecordFind(UWORD fileAddr, BYTE type);
extern BYTE LDFileCPLCheck(BYTE * dataBuf, BYTE type);
extern BYTE LDFileSMSRPS(UWORD fileAddr, BYTE * dataBuf, BYTE tag, BYTE mode);
extern BYTE LDFileFindBegain(UWORD fileAddr, BYTE type);
extern void LDFileClearSMSDownload(BYTE IELa);
extern BYTE LDFileMuliSMSFull(BYTE tag, BYTE sumNum);
extern BYTE LDFileRecordFindSub(BYTE refNumber, BYTE num,BYTE tag);
extern void LDCleanCounter(void);
extern void LDCleanUpdateFile(void);
extern BYTE IsRFMTar(BYTE *tar);
//////////////LDFilecompressCMD.c//////////////
extern BYTE CompressCMDProgress(void);

extern BYTE CATTPExcuteCommand(void);

//extern BYTE CATTPIsRFMReceiveAll(UWORD fileAddr, BYTE packageNum,BYTE tag);
extern BYTE IsRFMReceiveAll(UWORD fileAddr, BYTE packageNum,BYTE tag);


extern BYTE StoreDataSms(BYTE * bufdata,BYTE refNumber,BYTE seqNumber,BYTE totalNumber,BYTE tag);

extern BYTE CATTPChipheRe(BYTE kic,BYTE * pointer,BYTE * result_pointer,BYTE mode,WORD lgth);

extern BYTE CATGetRFMKey(BYTE index ,BYTE *key,BYTE mode);

extern void DesMacFunction(UWORD eepromAddr,BYTE * cpKey,BYTE cpResult[8],WORD nEnterLen,BYTE mode);

//extern void ProcessDeliverMuliResponse(void);
extern INSLIST code remoteRfm_list[];

//////////////全局变量//////////////
extern BYTE xdata G_LDFileDeliver;		//标志是否返回9FXX/9EXX/61XX
extern BYTE xdata G_LDFileDeliverMuli;	//deliver是否级联
extern BYTE xdata G_Source_Type;      //区分数据源 CATTP/Http/SMS
extern WORD xdata G_CurrentRemoteResponseLgth;	//https的下行数据执行完apdu指令后的响应数据长度,长度赋值0xFFFF 作为Http触发短信时候特殊使用
extern UWORD xdata G_CurrentRFMEfAddr; //用来保存102226读取数据过长时候的地址
extern BYTE  xdata G_RFMTypeStatus;		//设置当前的RFM操作状态 
//extern BYTE  xdata G_TARRecIndex[4];			//，第3字节存储的是RFM 的counterIndex


//数据源分类
#define SourceType_NULL  0x00
#define SourceType_SMS   0x01
#define SourceType_CATTP 0x02
#define SourceType_HTTPS 0x03


////////////Variable Tag//////////////////////
#define		LDFileTPOATag			0xA1	//TPOA缓存标志
#define		LDFileDeliverTag		0xA2	//Deliver级联使用

#define MODE_KIC     0x00
#define MODE_KID     0x01



//******************文件操作信息*********************************
#define     ReadFailed              0x00//0xB1
#define     CreateFailed            0x00//0xB2 
#define     ResizeFailed            0x00//0xB3
#define     UpdateFailed            0x00//0xB4
#define     DeleteFailed            0x00//0xB5
#define     DeactiveFaile           0x00//0xB6
#define     ActiveFailed            0x00//0xB7


/*************Response Status Code*****************************************/
#define  POR_OK                 0x00
#define  CCFailed               0x01
#define  CNTRLow                0x02
#define  CNTRHigh               0x03
#define  CNTRBlocked            0x04
#define  CipheringError         0x05
#define  RFMSecurityError  		0x06
#define  InsufficientMemory     0x07
#define  RFMNeedMoreTime        0x08
#define  TARUnknown             0x09
#define  RFMSecurityLevel       0x0A
#define  GPAnnexBNotSupport     0x0B		//https特殊错误，GPAnnexB的宏没有开启
#define  NOTGPAnnexBTritag      0x0C		//不是https的触发tag


//*******************************extended RFM extern *******************************************

extern BYTE xdata G_RFMAllReceived;

extern WORD xdata G_ExtendedRFMResponseLgth;   //当前扩展型RFM响应数据长度
extern BYTE xdata G_CurrentChainingValueTag;   //保存当前操作的ChainingValue
extern WORD xdata G_ErrorActionOffsetLgth[2];   //指示当前最后一个ErrorActionTLV在文件中的偏移
extern BYTE xdata G_EarlyResponseTag ;         //标志当前是否存在early response TAG
extern WORD xdata G_RFMResponseLgth;           //当前响应数据长度
extern BYTE  xdata G_FormatAndType;				//数据的格式  压缩/AA/AE
extern BYTE  xdata G_TARRecIndex[3];				//即tpx平台的G_TARRecIndex[0]
extern BYTE  xdata G_CurrentComNUM;


extern BYTE xdata  G_PORDeliverSubmit;

extern BYTE xdata  G_RFMFileIn2F08;

extern WORD xdata  G_CurrentCommandOffset; 

extern BYTE xdata  G_CurrentComNUM;  

//extern BYTE xdata  G_PORTag;

typedef __packed  struct tagTranFileHead
{
    WORD    FileId;                                 //  2    0
    BYTE    FileType;                               //  1    2
	BYTE    ArrNum;								    //  1    3  EFarr中记录号
    BYTE    ShortFileId;                            //  1    4
    BYTE    ArrName;                                //  1    5
    WORD    SpaceSize;                              //  2    6
    BYTE    UpdateConfignum;                                    //  1    8  
   	BYTE    FileStatus;                             //  1    9
	WORD    MapFile[3];								//  4	 A
}TranFileHead;	


#define TAG_CommandScriptingTemplateDefinite   0xAA
#define TAG_ResponseScriptingTemplateDefinite  0xAB
#define TAG_C_APDU                             0x22
#define TAG_ImmediateAction                    0x81
#define TAG_ErrorAction                        0x82
#define TAG_ScriptChaining                     0x83

#define TAG_R_APDU                             0x23
#define TAG_NumberCommandTLV                   0x80
#define TAG_NumberCAPDU                        0x80
#define TAG_ImmediateActionResponse            0x81
#define TAG_ScriptChainingResponse             0x83
#define TAG_BadFormat                          0x90
#define FLAG_TagNotFound                       0x01
#define FLAG_WrongLgth                         0x02
#define FLAG_LgthNotFound                      0x03
#define Value_SuspensionError                  0x01
#define TAG_ClearResponseFile                  0xAA


#define  Action_ReadBit                        0x00
#define  Action_UpdateBit                      0x01
#define  Action_ResizeBit                      0x02
#define  Action_DeleteBit                      0x03
#define  Action_DeactiveBit                    0x04
#define  Action_ActiveBit                         0x05


#define    TRIPLEDES            0x01
#define    SINGLEDES            0x02

#define    POR_OK  			0x00   //etsi 102 225 table 5
#define    CC_Failed 		0x01
#define    Cipher_Error 	0x05
#define    Securit_Error 	0x06



/*
#define    CNTR_LOW     	0x02
#define    CNTR_HIGH    	0x03
#define    CNTR_BLOCKED 	0x04
#define    More_Time        0x08
#define    TAR_Unkown   	0x09
*/



#define EF_ScriptChainingFile          0x5E21    //intial: 0x00 byte 1:当前Script Chaining Value,,结构体保存了当前context(ScriptValue,current directory, current file, current tag pointer, etc.) and the PIN verification),

typedef __packed  struct extendedRFMScriptContext
{
    BYTE     firstScriptValue;     //记录第一条scriptValue
    BYTE     previousValue;        //记录上一条的chainingValue
    BYTE     pinStatus;
    BYTE     recNum;                               
    WORD     dfName;
    WORD     efName;
    UWORD    dfAddr;
    UWORD    efAddr;
    WORD     secondeParrentName;  
     
}extendedRFMContext;	

//*************************************function********************************
extern extendedRFMContext xdata ChainingValue;
extern BYTE GPRAMRFMExtendResponsePublic(BYTE tag);
extern BYTE ExtendedRFMProcessBadTLV(BYTE errorCode);
extern BYTE ExtendedRFMProcessCAPDU(WORD lgth,WORD offset);
extern BYTE ExtendedRFMProcessScriptChaining(WORD valuelgth,WORD offset);
//extern BYTE ExtendedRFMExcuteCommand(void);
extern BYTE ExtendedRFMProcessErrorAction(void);
extern BYTE ExtendedRFMProcessImmediateAction(WORD lgth ,WORD offset);
extern BYTE RFMAccessCondition(UWORD fileAddr,BYTE action);
extern BYTE RFMAddNewRecordFCL(void);
extern BYTE RFMDeleteRecordFCL(WORD fileName);
extern BYTE RFMFindEmptyRecordFCL(void);
extern BYTE RFMGetCurrentEFPath(BYTE * currentPath);
extern void RemoteReadBinary(void);
extern BYTE CdmaTpoa2GsmTpoa(BYTE offset, BYTE *tpoabuf);
extern void BackDFNameForMoveFalsh(WORD DFFileName[4]);
extern void ReloadDFOrEFAddr(WORD fileName[4]);
extern BYTE ProcessRemoteCheckCAPDU(WORD cplLen);
//extern void IF_ProcessCdmaTpoaToGamTpoa( BYTE pos);
#endif
