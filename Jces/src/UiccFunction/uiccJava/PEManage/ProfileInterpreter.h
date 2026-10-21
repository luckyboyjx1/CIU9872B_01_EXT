#ifndef	 _profileInterpreter_h_
#define  _profileInterpreter_h_
#include   "typedef.h"
//#include   "eepromdefine.h"
#include   "tlvHeader.h"

/*******************PEGenericFileManagement.c中使用的宏定义*/
//Level One tag
#define Tag_thePEHeader                              0xA0

//Level Two tag
#define Tag_thePEHeaderMandated              0x80
#define Tag_thePEHeaderIdentification        0x81

//Level One tag
#define Tag_theGFMPEfileManagementCMD  0xA1

//Level Two tag
#define Tag_fileManagementListTag             0x30

//Level Three tag
#define Tag_filePath                                      0x80
#define Tag_createFCP                                  0x62
#define Tag_fillFileOffset                              0x02
#define Tag_fillFileContent                           0x81


//Level Four tag
#define Tag_GFMfLinkPath                           0xC7

//OCTET STRING (SIZE(1..5)) OPTIONAL,
#define Tag_GFMfDept                                 0x82

/* fileID	OPTIONAL
	For ADFs, the fileID is a temporary value (named temporary file ID in this document) used only during the profile creation. It has to be unique within a profile and is used for referencing files within this ADF using the file path.
*/
#define Tag_GFMfId                                     0x83

/* dfName	OPTIONAL
	Only applies for ADFs
*/
#define Tag_GFMfdfName                           0x84

/* lcsi	
	Coding according to ETSI TS 102 222 [102 222]
	[10] OCTET STRING (SIZE (1)) DEFAULT '05'H,
*/
#define Tag_GFMflcsi                                  0x8A
/* securityAttributesReferenced
	Either containing EF ARR ID[2] + record number[1] or
	record number[1] only and EF ARR ID implicitly known from the
	context, i.e. '2F06' within the MF and '6F06' otherwise
	[11] OCTET STRING OPTIONAL,
*/
#define Tag_GFMfAttr                                0x8B
/* 	efFileSize
	Mandatory for EF file types
	Not allowed for DF files
	[0] OCTET STRING OPTIONAL
*/
#define Tag_GFMfSize                               0x80
/* 	pinStatusTemplateDO
	Not allowed for EF files
	Mandatory for DF/ADF files
	Encoding is defined in ETSI TS 102 221 [102 221]
	[PRIVATE 6] OCTET STRING OPTIONAL,
*/
#define Tag_GFMfPstd                               0xC6

/* 	shortEFID
	Optional for EF file types
	Not allowed for DF files
	[8] OCTET STRING (SIZE (1)) OPTIONAL
*/
#define Tag_GFMfShortID                        0x88
/* 	proprietaryEFInfo
	Optional for EF file types
	Not allowed for DF files
	[5] ProprietaryInfo OPTIONAL
*/
#define Tag_GFMfPInfo                           0xa5

/* 	linkPath
	Specifies the path to the file to which shall be linked,
	also valid for DFs/ADFs. Files within ADFs are addressed
	by the temporary file ID of the respective ADF. For the coding
	see filePath.
	[PRIVATE 7] OCTET STRING OPTIONAL
*/
#define Tag_GFMfLinkPath                      0xC7

//Level Five tag
#define Tag_GFMspecialFileInformationt  0xC0

/* fillPattern, repeatPattern
only one of the parameters may be present. Coding and rules defined within ETSI TS 102 222 [102 222] apply
*/

//fillPattern [PRIVATE 1] OCTET STRING OPTIONAL,
#define Tag_GFMfillPattern                       0xC1

//repeatPattern [PRIVATE 2] OCTET STRING OPTIONAL
#define Tag_GFMrepeatPattern               0xC2




/*******************PEFileSystem.c中使用的宏定义*/
#define Tag_theTemplateID                              0x81

#define Tag_thePEFile                                       0xA2

 /*-- if set, indicate that the support of this PE is mandatory
	* NULL OPTIONAL,
	*/
#define Tag_thePEHeaderMandated                 0x80
//-- Identification number of this PE,UInt15
#define Tag_thePEHeaderIdentification           0x81

//FCP
#define Tag_thefileDescriptor                          0xA1

/* Indicates that this file shall not be created by the eUICC even if present in a PE referencing
 a "Created by Default" template. This flag has no effect for the creation of files in the MF and 
 shall not be used for all the files listed in a "Not Created by Default" template
*/
#define Tag_thefileDoNotCreate                       0x80

#define Tag_thefillFileOffset                             0x82

#define Tag_thefillFileContent                          0x83

//OCTET STRING (SIZE(1..5)) OPTIONAL,
#define  Tag_fDept             0x82 
/* fileID	OPTIONAL
	For ADFs, the fileID is a temporary value (named temporary file ID in this document) used only during the profile creation. It has to be unique within a profile and is used for referencing files within this ADF using the file path.
*/
#define  Tag_fId             0x83
/* dfName	OPTIONAL
	Only applies for ADFs
*/
#define  Tag_fdfName             0x84
/* lcsi	
	Coding according to ETSI TS 102 222 [102 222]
	[10] OCTET STRING (SIZE (1)) DEFAULT '05'H,
*/
#define  Tag_flcsi             0x8A

/* securityAttributesReferenced
	Either containing EF ARR ID[2] + record number[1] or
	record number[1] only and EF ARR ID implicitly known from the
	context, i.e. '2F06' within the MF and '6F06' otherwise
	[11] OCTET STRING OPTIONAL,
*/
#define  Tag_fAttr             0x8B

/* 	efFileSize
	Mandatory for EF file types
	Not allowed for DF files
	[0] OCTET STRING OPTIONAL
*/
#define  Tag_fSize             0x80
/* 	pinStatusTemplateDO
	Not allowed for EF files
	Mandatory for DF/ADF files
	Encoding is defined in ETSI TS 102 221 [102 221]
	[PRIVATE 6] OCTET STRING OPTIONAL,
*/
#define  Tag_fPstd             0xC6
/* 	shortEFID
	Optional for EF file types
	Not allowed for DF files
	[8] OCTET STRING (SIZE (1)) OPTIONAL
*/
#define  Tag_fShortID             0x88
/* 	proprietaryEFInfo
	Optional for EF file types
	Not allowed for DF files
	[5] ProprietaryInfo OPTIONAL
*/
#define  Tag_fPInfo             0xA5

/* 	linkPath
	Specifies the path to the file to which shall be linked,
	also valid for DFs/ADFs. Files within ADFs are addressed
	by the temporary file ID of the respective ADF. For the coding
	see filePath.
	[PRIVATE 7] OCTET STRING OPTIONAL
*/
#define  Tag_fLinkPath        0xC7

#define  HEADER_SERVICE_CONTACTLESS					0x80
#define  HEADER_SERVICE_USIM						0x81
#define  HEADER_SERVICE_ISIM  						0x82
#define  HEADER_SERVICE_CISM						0x83
#define  HEADER_SERVICE_MILENAGE					0x84
#define  HEADER_SERVICE_TUAK128						0x85
#define  HEADER_SERVICE_CAVE						0x86
#define  HEADER_SERVICE_GBA_USIM					0x87
#define  HEADER_SERVICE_GBA_ISIM					0x88
#define  HEADER_SERVICE_MBMS						0x89
#define  HEADER_SERVICE_EAP							0x8a
#define  HEADER_SERVICE_JAVACARD					0x8b
#define  HEADER_SERVICE_MULTOS						0x8c
#define  HEADER_SERVICE_MULTIPLE_USIM				0x8d
#define  HEADER_SERVICE_MULTIPLE_ISIM				0x8e
#define  HEADER_SERVICE_MULTIPLE_CSIM				0x8f
#define  HEADER_SERVICE_TUAK256						0x90
#define  HEADER_SERVICE_USIM_TEST_ALGORITHM			0x91
#define  HEADER_SERVICE_BERTLV     					0x92
#define  HEADER_SERVICE_DFLINK						0x93
#define  HEADER_SERVICE_CATTP      		 			0x94
#define  HEADER_SERVICE_GET_IDENTITY				0x95
#define  HEADER_SERVICE_PROFILE_A_X25519     		0x96
#define  HEADER_SERVICE_PROFILE_B_P256     			0x97

/*
 * 各种结构的定义
*/
typedef struct 
{
    BYTE PETag;
	BYTE (*func)(void);
}PELIST; //modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP


#define PROFILE_INIT_STATE   	0		//去警告，防止重复定义(宏名称INIT_STATE在文件scp11.h中已定义)，宏名称修改为更体现使用含义
#define PROFILE_CREATING_STATE  1		//去警告，宏名称修改为更体现使用含义
#define PROFILE_END_STATE     	2		//去警告，宏名称修改为更体现使用含义



extern BYTE code G_Common_Template_ID[5];

extern TlvInfo xdata G_PETlvInfo;
extern TlvInfo xdata G_TemplateTlvInfo;

extern WORD  xdata G_TempLen;
extern WORD xdata G_USIM_identification,G_CSIM_identification,G_Tmp_identification;


extern BYTE code USIMPartialAID[] ; 
#define EUICCCapacity    0xFFFFFF


//G_IsCreateFlag;
#define CREATE_MF         				0
#define CREATE_USIM       				1
#define CREATE_OPTUSIM    				2
#define CREATE_DF_CD     			    3
#define CREATE_PUK        				4
#define CREATE_GolbalPIN  				5
#define CREATE_LocalPIN   				6
#define CREATE_FileNotSupportWaring  	7

//G_IsExistFlag2
#define FileDescriptorTag        0 //FileDescriptor是否存在
#define DFFlag                   1 //是否是DF
#define FirstDFFlag              2 //是否是第一个DF
#define OccurFlag                3 //出现的次数
#define Mandated                 4 //PE是否是强制的
#define PE_Complete              5 //PE是否是完整的
#define SD_A2                    6 //SD中是否有A2tag
#define ContentRequired          7 //profile中是否需要给文件内容, modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
//G_IsExistFlag3
#define NeedExtradite            0  //add by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
#define A6Tag                    1  //add by zhxx   20220629 HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg00b3：B9(CSIM)下面没有A6(CSIM模板强制性文件)所以抛异常
#define AETag                    2  //add by zhxx   20220629 HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg00b3：B9(CSIM)下面没有AE(CSIM模板强制性文件)所以抛异常

#define    B_CONTACTLESS	0       //Bit 0 ：contactless                                        
#define    B_BERTLV			1       //Bit 1 ：ber-tlv                                        
#define    B_DFLINK			2       //Bit 2 ：dfLink
#define    B_CATTP			3       //Bit 3 ：cat-tp
#define    B_USIM			4       //Bit 4 ：usim
#define    B_ISIM		    5       //Bit 5 isim
#define    B_CSIM			6       //Bit 6 ：csim
#define    B_RFU	        7		//Bit7 ：RFU
//extern BYTE xdata G_CurLoadLTEAUTH;//当前下载的PROFILE的算法属性
#define    B_MILENAGE   	0       //Bit 0 ：milenage                                        
#define    B_TUAK			1       //Bit 1 ：TUAK                                       
#define    B_XOR			2       //Bit 2 ：XOR
#define    B_RFU_AUTH		3       //Bit 3 ：RFU
#define    B_EAP			4       //Bit 4 ：EAP
#define    B_GBA		    5       //Bit 5 GBA
#define    B_MBMS			6       //Bit 6 ：MBMS
#define    B_CAVE		    7		//Bit7 ：CAVE

typedef __packed struct
{
	BYTE TagA0Occ;//PE头出现的次数
	BYTE PETag;
	WORD PE_identification;
	BYTE IsExistFlag2;
	BYTE IsExistFlag3;
	BYTE FileDescriptorTimes;//文件PE中FileDescriptor出现的次数
	WORD C4RemainLen;//load包中C4tag的剩余长度/记录应用包id在profile数据中的偏移，为了断开的情况
	WORD BlockNum; //拼装loadpackage指令的块号
	WORD ApplicationLoadPackageOff;//load包中C4tag的剩余长度/记录应用包id在profile数据中的偏移，为了断开的情况
	BYTE StartTag;
	BYTE CurTag;
	BYTE SUPPORTED_ServicesList[3];//所支持的ServicesList标志位，1为支持
}PEControlContext; //added by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
typedef __packed struct
{
	BYTE  ProfileCurrStatus;//当前profile的状态
	BYTE  LoaderState;//下载PE的状态
	WORD  PESize;//PE的大小/当前处理的字tag长度
	WORD  TotalLen;//PE的大小/当前处理的字tag长度
	BYTE  PEID[100];	//PE id
	WORD  BackInfoLen; //备份不完整PE的长度
	WORD  BackInfoNextLen;//
	WORD  EUICCResponseLen; //响应数据的长度
	WORD CURLOADDIR; //当前下载的PROFILE的DIRFILEID,通常是2F00
	BYTE IsCreateFlag;//标记，具体看每个bit的含义
	BYTE ProfileVersion;
	BYTE IsCreateMNOSD;//用来标记是否是MNO-SD，0表示MNOSD，其它表示SSD
	WORD LoadParaLen;
}ProfileControlContext;//added by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP

typedef __packed struct
{
	UWORD FirstDFAddr;
	WORD FirstDFName;
	UWORD LoadDFAddr;
	UWORD LoadEFAddr;//当前正在下载的DF地址/当前正在下载的EF地址
	UWORD LoadADFAddr;
	WORD LoadADfId;
	WORD LoadDFName;//当前创建的文件的DFname
	WORD LoadEFName;//当前创建的文件的EFname
	WORD UpdateOffset;//当前创建文件的偏移
	BYTE CurrentFileStructure;//当前创建文件的文件类型
	WORD CurrentFileFileRecLen;//当前创建文件的记录长度
}FileControlContext;//added by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP

extern PEControlContext xdata G_PEControlContext;
extern ProfileControlContext xdata G_profileControlContext;
extern FileControlContext xdata G_fileControlContext;

extern BYTE xdata G_ProfileErrorReason;

extern BYTE ProfileHeaderProcess(void);
extern BYTE GFMPEProcess(void);
extern BYTE PINCodesPEProcess(void);
extern BYTE PUKCodesPEProcess(void);
extern BYTE AKAParameterPEProcess(void);
extern BYTE CDMAParameterPEProcess(void);
extern BYTE SecurityDomainPEProcess(void);
extern BYTE RFMPEProcess(void);
extern BYTE ApplicationPEProcess(void);
extern WORD makeInstallForRegistryUpdate(BYTE* buffer ,WORD installParaLen);
extern BYTE NonStandardPEProcess(void);
extern BYTE EndPEProcess(void);
extern void MNNMNCConvert(BYTE *Src,BYTE *Des,BYTE mncLen);

extern BYTE PEFileSystemProcess(BYTE tempId);//modify by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP
extern BYTE backData(UWORD desAddr,UWORD srcAddr, WORD dataLen);
extern BYTE createTemplateAfterTagFile(BYTE curTag);
extern void MCCMNCConverToIMSIM(BYTE *Src,BYTE *Des);	//remove warning,dengyiwen,2022/9/27

#define OK_PE                            0
#define PE_NOT_SUPPORTED              1
#define MEMORY_FAILURE_PE                2
#define BAD_VALUES                    3
#define NOT_ENOUGH_MEMORY             4
#define INVALID_REQUEST_FORMAT        5
#define INVALID_PARAMETER             6
#define RUNTIME_NOT_SUPPORTED         7
#define LIB_NOT_SUPPORT               8
#define TEMPLATE_NOT_SUPPORTED        9
#define FETURE_NOT_SUPPORTED          10
#define PIN_CODE_MISSING              11
#define UNSUPPORTED_PROFILE_VERSION   31


#define  CONTACTLESS_SUPPORTED      0
#define  USIM_SUPPORTED             1
#define  ISIM_SUPPORTED             2
#define  CSIM_SUPPORTED             3
#define  MILENAGE_SUPPORTED         4
#define  TUAK128_SUPPORTED          5
#define  CAVE_SUPPORTED             6
#define  GBA_USIM_SUPPORTED         7 
#define  GBA_ISIM_SUPPORTED         0
#define  MBMA_SUPPORTED             1
#define  EAP_SUPPORTED              2
#define  JAVACARD_SUPPORTED         3
#define  MULTOS_SUPPORTED           4
#define  MULTIPLE_USIM_SUPPORTED    5
#define  MULTIPLE_ISIM_SUPPORTED    6
#define  MULTIPLE_CSIM_SUPPORTED    7
#define  TUAK256_SUPPORTED          0
#define  usim_test_algorithm_SUPPORTED           1
#define  bertlv_SUPPORTED          2
#define  dfLink_SUPPORTED          3




/*********PE_STATE**************/
//去警告，宏定义名称 INIT_STATE 已在本文件227行中定义，并且此处三个宏定义值未使用到，可以注释掉
//#define  INIT_STATE           0
//#define  UNFINISH_STATE       1
//#define  FINISH_STATE         2



extern BYTE createMFFile(void);

extern BYTE createProfilePrivateFile(void);

extern BYTE makePInfoData(BYTE fPstdTag);
extern void makeFileAttrInfo(BYTE fAttrTag);
extern BYTE makeUpdate(void);
extern void makeFileLinkPath(BYTE linkPathTag);

extern WORD makePutKey(BYTE keyId,BYTE keyVersion,WORD keyCompontentLen, BYTE  *keyCompontentData);
extern BYTE GetProfileAppIdByAIDList(BYTE profileIndex, BYTE aidLen, BYTE *aidBuf);
extern WORD makeInstallForInstall(WORD installParaLen);
extern WORD makeInstallForLoad(WORD loadParaLen);
extern WORD makeLoad(void);
extern void makeStoreData(void);

extern BYTE code thePEHeaderTag[];
 /*-- if set, indicate that the support of this PE is mandatory
	* NULL OPTIONAL,
	*/
extern BYTE code thePEHeaderMandated[];
//-- Identification number of this PE,UInt15
extern BYTE code thePEHeaderIdentification[];

/************************************PESecurityDomain.c function define****************************************/
extern void NM_GPProfileInstallCommand(void);
extern void NM_GPProfileLoadCommand(void);
extern void GPProfileInstallForInstall(void);
extern void GPProfileInstallForLoad(void);

/*******************function define In filr ProfileInterpreter.v************************************************/
extern void appendPEStatusInfo(BYTE type,WORD identifier);
extern void appendErrorResponse(BYTE type,WORD identifier);
extern void appendOKResponse(void);
extern void appendAbordInfo(void);
extern void InitPEManageVariable(void);
extern void interpreterPE(UWORD dataBuffer,WORD dataLen/*,BYTE *outPut*/);
extern void SetProfileChannel(UWORD fileAddr,WORD fileName);


extern BYTE PEHeaderAnalyze(void);
#define   PESD_MODE       0x00
#define   PEAPP_MODE      0x01
extern BYTE ApplicationInstanceAnalyze(BYTE mode);
extern BYTE KeyObjectSequenceAnalyze(void);
extern BYTE ApplicationLoadPackageAnalyze(void);

extern BYTE processStatusWord(WORD sw,BYTE option);

extern BYTE CheckTagOcc(BYTE idata tag);
extern BYTE processFileResult(BYTE result);
extern BYTE  PEProcess(UWORD dataBuffer,WORD dataOff,WORD datalen,BYTE PEtag);//added by chentao@20210914 和PE解析优化代码, EPX产业化\STM_ST33G1M2\DEVELOP

extern BYTE xdata instanceAID[0x11];  //add by chentao 20220426  德国profile在安装SD 时候 A2即putkey时断开，导致 自引渡数据被清除导致profile下载失败，所以把局部变量改成全局变量
extern	BYTE xdata extraditeSDAID[0x11];
UWORD PreAnalyseCreateFile(void);
#endif

