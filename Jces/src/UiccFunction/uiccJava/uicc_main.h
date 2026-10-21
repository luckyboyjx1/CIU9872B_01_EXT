#ifndef  _uicc_main_h_
#define  _uicc_main_h_
//#include "typedef.h"
#include "hard_api.h"
#include "variable_tag.h"
#include "global_variable.h"
#include "EUICCMain.h"
#include "Interface_PlatformFromLTE.h"
#include "FunctionSwitch.h"
#include "profile.h"

#if UICC_AB8C_ARR_PRIVATE
#define    A_PRIVATEARR_REC_FLAG      0xC0
#define    A_PRIVATEARR_REC_UNMASK    0x3F
#define    STANDARD_ARR_MAX_NUMBER  0xBF    //2F06支持的最大记录数
#endif

#define    ARR_RECNUM_ADM           0xFE
#define    ARR_RECNUM_NEV           0xFF

#define DecToAsc(x)   x += (x < 0x0A) ? 0x30:0x37;

#define BuildWord(buf)    MEM_R_U16((memref)buf) //((WORD)(((WORD)(buf)[0]<<8)|(buf)[1]))

#define BuildUWord(buf)   MEM_R_U32((memref)buf) // ((UWORD)(((UWORD)(buf)[0]<<24)|((UWORD)(buf)[1]<<16)|((UWORD)(buf)[2]<<8)|(buf)[3]))

#define SetWord(buf,value)	do{(buf)[0]=((value)>>8)&0xFF; (buf)[1]=(value)&0xFF; }while(0)

#define SetUWord(buf,value)	MEM_W_U32((memref)buf,value) //do{(buf)[0]=((value)>>24)&0xFF; (buf)[1]=((value)>>16)&0xFF; (buf)[2]=((value)>>8)&0xFF; (buf)[3]=(value)&0xFF; }while(0)

#define OPERATOR_DEFAULT           0x07   //默认设置成标准模式
#define OPERATOR_CHINAMOBILE   0x00
#define OPERATOR_CHINAUNICOM   0x01
#define OPERATOR_CHINATELECOM  0x02
//#define OPERATOR_GERMANY  		0x03

#define OPERATOR_EUICC__REUSE_OTPK        0x80 // 复用运营商配置的最高字节配置是否支持reuse otpk
#define OPERATOR_EUICC__SUPPORT_DFLINK    0x40 // 复用运营商配置的一个字节配置是否支持DFLink

//#define SPECIAL_CHINAUNICOM     0x01   // 当前工程对于联通特殊需求的处理
#define ResSave_Len                       0xFF //目前最大支持255字节长度响应数据在各通道上交互获取

#define VM_MAX_LOGICAL_CHANNELS       MAX_LOGICAL_CHANNELS //(0x08)
#define VM_MAX_LOGICAL_CHANNELS_CT    ISO_MAX_LOGICAL_CHANNELS//(0x04)

// modify by xuxiaojun@20220318 分层, 该变量属于Boot层
//extern BYTE xdata RAM_ATR_Flag;//modify by zhangxiaoxiao@20210721: 将全局变量RAM_ATR_Flag提到开关UICC__FUNCTION_CMCCAPI外面，用于判断是否发送ATR

#if UICC__FUNCTION_CMCCAPI
extern BYTE xdata RAM_Function_Supported;
extern BYTE xdata RAM_Online_Flag;
extern BYTE xdata RAM_Location_Status_Flag;//是否支持event
#endif


//地址分配
//非共享文件信息存储区
/*****************************BIP_FILE_START*************************************/
//#define  CARDCFG_START                                 (EUICC_CFG_END + 0x01)
//#if UICC_FUCTION_PRIVATEAUTH
//#define  E_TAG_AUTHDATA_UPDATE_ADDR                CARDCFG_START      //"更新鉴权参数"状态，PROFILE_MAX_NUMBER字节(每个profile 一个字节)

////#define  CARDCFG_KeepChaining     (E_TAG_AUTHDATA_UPDATE_ADDR + PROFILE_MAX_NUMBER)
////#define  E_MFKEYADDR                               (CARDCFG_KeepChaining + 1)           //MF key ststus(1) + MF key(8)

//#define  E_MFKEYADDR                               (E_TAG_AUTHDATA_UPDATE_ADDR + PROFILE_MAX_NUMBER)
//#define  E_MANAGERKEYADDR                    (E_MFKEYADDR+0x10)      //Manager key ststus(1) + Manager key(8)
//#define  E_CUR_DURATION                      (E_MANAGERKEYADDR + 0x10)//2字节  保存 poll Interval时间间隔
//#else
//#define  E_CUR_DURATION                      CARDCFG_START           //2字节  保存 poll Interval时间间隔
//#endif


//#define  BIPModule_Receive_ADDR_START     ((E_CUR_DURATION + 2 + EEPAGE_MASK)&NOR_EEPAGE_MASK) //必须是页起始

//EUICC_CFG_END is page start
#define  BIPModule_Receive_ADDR_START     ((EUICC_CFG_END + EEPAGE_MASK)&NOR_EEPAGE_MASK) //必须是页起始
#define  BIPModule_Receive_SIZE                 0xA00  //整数页

#define  BIPModule_Send_ADDR_START         (BIPModule_Receive_ADDR_START + BIPModule_Receive_SIZE)// 页起始
#define  BIPModule_Send_SIZE                     0xA00 //整数页

#define  HTTP_CommandData_ADDR_START       (BIPModule_Send_ADDR_START + BIPModule_Send_SIZE)// 页起始
#define  HTTP_CommandData_SIZE                    0xC00 //整数页

//#define  SMS_SEND_PORDATA_BUFFER       BIPModule_Send_ADDR_START    //EF_PorDataFile文件不再需要，共用这块区域, by xwh 20210628
//#define  SMSPorData_Send_SIZE                     BIPModule_Send_SIZE //整数页


#if UICC_NEW_FILESYSTEM

#define  E_MAPEXTEND_START   (HTTP_CommandData_ADDR_START + HTTP_CommandData_SIZE) 
#define  E_MAPEXTEND_Size     (EEPAGE_SIZE)

#define  E_SECUREMOVE_START  (E_MAPEXTEND_START + E_MAPEXTEND_Size)     //必须是页起始
#define  E_SECUREMOVE_Size       SecureMoveStuct_LengthAll

#else

#define  E_SECUREMOVE_START  (HTTP_CommandData_ADDR_START + HTTP_CommandData_SIZE)     //必须是页起始
#define  E_SECUREMOVE_Size       SecureMoveStuct_LengthAll

#endif



#define  CARDCFG_START                                 (E_SECUREMOVE_START+E_SECUREMOVE_Size)
#if UICC_FUCTION_PRIVATEAUTH
#define  E_TAG_AUTHDATA_UPDATE_ADDR                CARDCFG_START      //"更新鉴权参数"状态，PROFILE_MAX_NUMBER字节(每个profile 一个字节)

//#define  CARDCFG_KeepChaining     (E_TAG_AUTHDATA_UPDATE_ADDR + PROFILE_MAX_NUMBER)
//#define  E_MFKEYADDR                               (CARDCFG_KeepChaining + 1)           //MF key ststus(1) + MF key(8)

#define  E_MFKEYADDR                               (E_TAG_AUTHDATA_UPDATE_ADDR + PROFILE_MAX_NUMBER)
#define  E_MANAGERKEYADDR                    (E_MFKEYADDR+0x10)      //Manager key ststus(1) + Manager key(8)
#define  E_CUR_DURATION                      (E_MANAGERKEYADDR + 0x10)//2字节  保存 poll Interval时间间隔
#else
#define  E_CUR_DURATION                      CARDCFG_START           //2字节  保存 poll Interval时间间隔
#endif


//#define E_FILE_NoShareableRec (E_SECUREMOVE_START+E_SECUREMOVE_Size)
#define E_FILE_NoShareableRec (E_CUR_DURATION + 2)
#define E_FILE_NoShareableSize 0x200

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
#define  ADDR_FILENO_HEFLASH (E_FILE_NoShareableRec + E_FILE_NoShareableSize)
#define  CARDCFG_COSVersion  (ADDR_FILENO_HEFLASH + 1)
#else
#define  CARDCFG_COSVersion		  (E_FILE_NoShareableRec + E_FILE_NoShareableSize) //LV 结构
#endif
#define  CARDCFG_COSVersionLen		0x10 //暂时固定为10

#if   High_Update_InRAM_New
#define  Config_High_Update_InRAM_num  	(CARDCFG_COSVersion + CARDCFG_COSVersionLen) //记录强更新RAM的分配个数	
#define  Config_High_Update_InRAM_Size  (Config_High_Update_InRAM_num + 1) //记录强更新RAM的分配大小 文件大小+2
#define  RSPDefaultFileSystem_Addr                   (Config_High_Update_InRAM_Size +1 )
#else
#define  RSPDefaultFileSystem_Addr                   	(CARDCFG_COSVersion + CARDCFG_COSVersionLen)
#endif

#define  E_NAFID_OLD_RECORD                             (RSPDefaultFileSystem_Addr + 4)//GBANAFAuthenticate更新NAFID时，存储最近更新的记录号，以便于覆盖，大小2 byte，ISIM(1)+USIM(1)

//私有文件区
#define ADDR_PRIVATE_FILE_START    (memref)(MapTable_GetEndAddr()+EEPAGE_SIZE) //(E_NAFID_OLD_RECORD + 2)私有文件系统和map表之间预留一页

#define     UICC_CFG_END     (memref)((E_NAFID_OLD_RECORD + 2 +0xFF)& 0xFFFFFF00)

#define E2P_IMAGE_START				UICC_CFG_END//(memref)(COS_CONFIG_START + CONFIG_END_ALIGN)


extern UWORD xdata G_CurMFFileStart;
//文件区
//#define E_FILE_START         (ADDR_PRIVATE_FILE_START + 0x1000)  //暂定
#define  E_FILE_START              G_CurMFFileStart

//Card Specification v2.2 Version 1.0.1 ¨C Amendment E :4.5 Preloaded ECC Curve Parameters -> Table 4-3: Key Parameter Reference Values
#define ECCCure_P_256_KeyParaRef                0x00
#define ECCCure_P_384_KeyParaRef                0x01
#define ECCCure_P_521_KeyParaRef                0x02
#define ECCCure_brainpoolP256r1_KeyParaRef      0x03
#define ECCCure_brainpoolP256t1_KeyParaRef      0x04
#define ECCCure_brainpoolP384r1_KeyParaRef      0x05
#define ECCCure_brainpoolP384t1_KeyParaRef      0x06
#define ECCCure_brainpoolP512r1_KeyParaRef      0x07
#define ECCCure_brainpoolP512t1_KeyParaRef      0x08
#define ECCCure_RP256V1_KeyParaRefF             0x40

#if EUICC__FUNCTION__GUOMI_eSIM
	#define ECCCure_SM2_KeyParaRefF             	0x41
#endif

/***************************************************/
#define ECC_PARA_LEN_256                        0x20

#define     NOShareableFile_Length        0x0D
#define     NOShareableFile_Number        0x10

//#define E_FILE_NoShareableRec 0

//#define    ISD_APPID          0x00
#define    TEMP_BUF_SIZE      512

#define D_DeleteFile		0x40
#define D_Resize			0x80
#define D_CreateFile        0x20


#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
#define PFPageSize                               EEPAGE_SIZE
#define Num_HeFlash_Pages                        16
#define MaxNum_HeFlash_Files                     14                       //支持创建14个强更新文件
#define MaxNum_HeFlash_Group                     8                  //每组最大记录数（分两组，每组8条记录）
#define Num_HeFlashPage_ManageBytes              9                 //管理字节数，CRC  2bytes + counter 4 bytes + EfName 2 bytes + FileNo 1 byte
#define RamBuffer_CrcOffset                      (PFPageSize -2)    //CRC  2bytes
#define RamBuffer_CounterOffset                  (PFPageSize -6)     //counter 4 bytes 
#define RamBuffer_EfNameOffset                   (PFPageSize -8)     //EfName 2 bytes
#define RamBuffer_FileNoOffset                   (PFPageSize -9)      //FileNo 1 byte
#endif
/*
V1.0 :modify @20140727 by xuyz ,增加以下宏定义
#define		F_RFM_PERMIT_ALL		0x1F
#define		F_RFM_PERMIT_NONE		0x80

#define     F_ARRNAME_6F            0x80
#define     F_ARRNAME_2F            0x00
*/
/*
 * 各种结构的定义
*/
typedef struct 
{
    BYTE ins;
    void (*func)(void);
}INSLIST;

typedef __packed struct tagPathInfo							//备份当前路径,modify at 2007-02-08
{
	WORD  fileId[3];
	BYTE  recPointer;							
	BYTE  sfi;										
}PathInfo;


//-------------------------------------------------------------------------------------------------//
//-------------------------------File System (MF,ADF,DDF)的文件头结构-----------------------------------------------//
//-------------------------------------------------------------------------------------------------//
typedef __packed  struct tagFileSystemHead
{
   WORD   FileId;                                  // 2     0      2    0
   BYTE   FileType;                                // 1     2      1    2
   BYTE   ArrNum;						           // 1     3      1    3
   BYTE   DIRNum;                                  // 1     4      1    4  ;ADF name在DIR文件中的记录号
   BYTE   ArrName;                                 // 1     5      1    5
   BYTE   LocalPinIndex;
   BYTE   FillByte;                                // 为了4字节对齐，填充一个WORD
   UWORD  MaxSize;                                 // 4     6      2    6
   UWORD  UsedSize;                                // 4     A      2    8
   BYTE   ChildDfNum;                              // 1     E      1    A
   BYTE   ChildEfNum;                              // 1     F      1    B
   WORD   ParentName[2];	                       // 4     10     4    C 
   BYTE   ChvStatus[4];                            // 4	    14     4    10
   BYTE   Rfu;
   BYTE	  KeyLmit;
   BYTE   MF_Key[8];
}FileSystemHead;

typedef __packed  struct tagSingleFileHead
{  
   UWORD  UsedSize;                                 //4     4      2    2
   BYTE   ChildDfNum;                               //1     8      1    4
   BYTE   ChildEfNum;                               //1     9      1    5
}SingleFileHead;
//-------------------------------------------------------------------------------------------------//
//-------------------------------二进制 的文件头结构-----------------------------------------------//
//-------------------------------------------------------------------------------------------------//
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
//-------------------------------------------------------------------------------------------------//
//-------------------------------线形固定/循环记录 的文件头结构------------------------------------//
//-------------------------------------------------------------------------------------------------//
typedef __packed  struct tagRecordFileHead
{
    WORD    FileId;                                 //  2    0
    BYTE    FileType;                               //  1    2
	BYTE    ArrNum;									//  1    3
    BYTE    ShortFileId;                            //  1    4
    BYTE    ArrName;                                //  1    5
    BYTE    RecNumber;                              //  1    6
	BYTE	RecLength;                              //  1    7
	BYTE	UpdateConfignum;                           //  1    8
    BYTE    FileStatus;                             //  1    9
	WORD    MapFile[3];								//  4    A
}RecordFileHead;						    		//LINER FIXED AND CYCIC FILE  head define
//---------------------------------------------------------------------------------------------------

#if UICC__FUNCTION__BERTLVCmd
//*****************************add BER-TLV structrue EF ***********************************************
typedef __packed  struct tagBERTLVFileHead
{
	WORD   	FileId;                           
	BYTE    FileType;                        
	BYTE    ArrNum;					
	BYTE    ShortFileId;                     
	BYTE    ArrName;                         
	WORD  	MaxSize;                         
	BYTE    FileDetails;                     
	BYTE    FileStatus;                      
	WORD    MapFile[3];					
	WORD  	UsedSize;                          
	WORD  	ReservedSize;                     
}BERTLVFileHead;

extern BERTLVFileHead xdata  bertlvFileHead;
#endif

//-------------------------------------------------------------------------------------------------//
//-------------------------------CHV/PUK结构设计--------------------------------------------------//
//------------------------------------------------------------------------------------------------//
typedef __packed  struct tagKeyStructer
{
 	BYTE    KeyId;	            //1    0
	BYTE    KeyStatus;			//1    1
	BYTE    KeyLimit;			//1	   2
	BYTE    KeyContent[8];		//8	   4
	BYTE    pukRef;
}KeyStructer;


//pin相关信息
#define PIN_LEN           12
#define KEY_ID_OFF         0
#define KEY_STATUS_OFF     1
#define KEY_LIMIT_OFF      2
#define KEY_CONTENT_OFF    3
#define KEY_PUKREF_OFF     11


typedef __packed  struct tagPukStructer
{
 	BYTE    PukId;	           
	BYTE    PukLimit;			
	BYTE    PukContent[8];		 	
}PukStructer;



//puk相关信息
#define PUK_LEN    0x0A
#define PUK_ID_OFF        0
#define PUK_LIMIT_OFF     1
#define PUK_VALUE_OFF     2


typedef __packed  struct tagFileIdType
{
   WORD   FileId;                                  // 2     0      2    0
   BYTE   FileType;                                // 1     2      1    2
}FileIdType;

#define DF_Depth         0x04
/*
typedef __packed struct tagResizeAntiTearing
{
	BYTE 	Tag;
	UWORD	ParentAddr[DF_Depth];
	UWORD	NewUsedSize[DF_Depth];
	UWORD	SrcAddr;
	UWORD	DataLen;
	UWORD	FileAddr;
	BYTE    FileType;
	BYTE    FileInfo[0x40];

}NewResizeAntiTearing;
*/

//modify by qiuping@2021-11-11  
//经验证，在结构体内部使用开关对编译无影响，俩种情况生成的hex文件一致,因此将#if (EUICC_ONLY || SE_AND_EUICC) 开关还原到结构体内部
typedef __packed struct newtagResizeAntiTearing  //????????????size?0x80 ,?????????0x80
{
	UWORD	ParentAddr[DF_Depth];
	UWORD	NewUsedSize[DF_Depth];
	UWORD	NewMaxSize[DF_Depth];
	BYTE    FileInfo[0x40];

#if (EUICC_ONLY || SE_AND_EUICC)
  BYTE    ProfileNum;                            //?????profile???
  BYTE    ProfileIndex[PROFILE_MAX_NUMBER];        //profile index
  UWORD  ProfileNewMFOffset[PROFILE_MAX_NUMBER]; //??Profile MF offset
#endif
}NewResizeAntiTearing;



#if UICC_NEW_FILESYSTEM
typedef __packed struct mapTableExtend  //
{	
    UWORD  srcAddr;
	UWORD  destAddr;
	UWORD  moveLen;
#if (EUICC_ONLY || SE_AND_EUICC)
	UWORD  ProfileNewMFOffset[PROFILE_MAX_NUMBER];
#endif
	UWORD  newDefaultFileSystemAddr;
	BYTE   moveTag;	
	BYTE   newMapNum;
	BYTE   RFU1;
	BYTE   RFU2;

}MapTableExtend;
#endif




#define     F_BERTLV_UsedSize           ( 5*sizeof(WORD)+ 6*sizeof(BYTE))
#define     F_BERTLV_ReservedSize     	( 6*sizeof(WORD)+ 6*sizeof(BYTE))
#define     F_BERTLV_Detail            	 ( 2*sizeof(WORD)+ 4*sizeof(BYTE))
//----------------------------------------------------------------------------------------------------
#define    ByteToWord(high,low)         ((((WORD)(high) << 8) & 0xFF00) | ((WORD)(low) & 0x00FF))
//-------------------------------------------------------------------------------------------------//
//-------------------------------文件头结构的偏移--------------------------------------------------//
#if 0
#define    F_Name_Offset           0x00
#define    F_Type_Offset           sizeof(WORD)
#define    F_ArrNum_Offset		   (sizeof(WORD)+sizeof(BYTE))
#define    F_ShordFid_Offset       (sizeof(WORD)+2*sizeof(BYTE))
#define    F_DIRNum_offset  	   (sizeof(WORD)+2*sizeof(BYTE))
#define    F_ArrName_Offset        (sizeof(WORD)+3*sizeof(BYTE)) 
#define    F_RecNum_Offset         (sizeof(WORD)+4*sizeof(BYTE)) 
#define	   F_MSize_Offset	       (sizeof(WORD)+4*sizeof(BYTE))
#define	   F_SpaceSize_Offset	   (sizeof(WORD)+4*sizeof(BYTE))
#define    F_RecLgth_Offset        (sizeof(WORD)+5*sizeof(BYTE))
#define	   F_Pointer_Offset        (sizeof(WORD)+6*sizeof(BYTE))
#define    F_Status_Offset         (sizeof(WORD)+7*sizeof(BYTE))
#define    F_MapFile_Offset        (sizeof(WORD)+8*sizeof(BYTE))

#define    F_UsedSize_Offset          (sizeof(WORD)+4*sizeof(BYTE)+sizeof(UWORD))
#define    F_ChildDFNum_Offset        (sizeof(WORD)+4*sizeof(BYTE)+2*sizeof(UWORD))
#define    F_ChildEFNum_Offset        (sizeof(WORD)+5*sizeof(BYTE)+2*sizeof(UWORD))
#define    F_ParentName_Offset        (sizeof(WORD)+6*sizeof(BYTE)+2*sizeof(UWORD))
#define    F_SecondParentName_Offset  (2*sizeof(WORD)+6*sizeof(BYTE)+2*sizeof(UWORD))
#define    F_ChvStatus_Offset        (3*sizeof(WORD)+6*sizeof(BYTE)+2*sizeof(UWORD))

#define	   F_KLMT_Offset			(3*sizeof(WORD)+11*sizeof(BYTE)+2*sizeof(UWORD))
#define	   F_KEY_Offset				(3*sizeof(WORD)+12*sizeof(BYTE)+2*sizeof(UWORD))

#define    F_BaudStatus_Offset		F_KLMT_Offset

#define    E_MK_START		  	    (E_FILE_START+(3*sizeof(WORD)+12*sizeof(BYTE)+2*sizeof(UWORD))) 
#endif
#define    F_Name_Offset                        0x00
#define    F_Type_Offset                         (F_Name_Offset+sizeof(WORD))
#define    F_ArrNum_Offset                     (F_Type_Offset+sizeof(BYTE))
#define    F_DIRNum_offset  	               (F_ArrNum_Offset+sizeof(BYTE))
#define    F_ArrName_Offset                   (F_DIRNum_offset+sizeof(BYTE))
#define    F_LocalPinIndex_offset             (F_ArrName_Offset + 1)
#define	   F_MSize_Offset	                       (F_ArrName_Offset+sizeof(BYTE)+sizeof(WORD))  //跳过填充WORD
#define    F_UsedSize_Offset                   (F_MSize_Offset+sizeof(UWORD))
#define    F_ChildDFNum_Offset              (F_UsedSize_Offset+sizeof(UWORD))
#define    F_ChildEFNum_Offset              (F_ChildDFNum_Offset+sizeof(BYTE))
#define    F_ParentName_Offset              (F_ChildEFNum_Offset+sizeof(BYTE))
#define    F_SecondParentName_Offset  (F_ParentName_Offset+sizeof(WORD))
#define    F_ChvStatus_Offset                 (F_SecondParentName_Offset+sizeof(WORD))
#define	   F_AlgID_Offset			(F_ChvStatus_Offset+4*sizeof(BYTE))
#define	 F_KLMT_Offset			        (F_ChvStatus_Offset+5*sizeof(BYTE))
#define	 F_KEY_Offset		               (F_KLMT_Offset+sizeof(BYTE))
#define    F_BaudStatus_Offset		  F_KLMT_Offset


//EF文件宏定义
#define    F_ShordFid_Offset       (F_ArrNum_Offset+sizeof(BYTE))
#define    F_Status_Offset           (F_ShordFid_Offset+5*sizeof(BYTE))
#define    F_MapFile_Offset         (F_Status_Offset+sizeof(BYTE))

//LF文件宏定义
#define    F_RecNum_Offset         (F_ArrName_Offset+sizeof(BYTE)) 
#define    F_RecLgth_Offset         (F_RecNum_Offset+sizeof(BYTE))
#define	 F_Pointer_Offset          (F_RecLgth_Offset+sizeof(BYTE))

//TF文件宏定义
#define	 F_SpaceSize_Offset     (F_ArrName_Offset+sizeof(BYTE)) 



//#define    E_MK_START		  	    (E_FILE_START+(3*sizeof(WORD)+12*sizeof(BYTE)+2*sizeof(UWORD))) 

#define	   F_MapAndRead_Offset		F_ShordFid_Offset   //复用F_ShordFid_Offset字节的最高bit表示文件不允许被读和映射
#define    File_MapAndRead             0x80
//-------------------------------------------------------------------------------------------------//
//-------------------------------读写记录文件模式的定义--------------------------------------------//
//-------------------------------------------------------------------------------------------------//
#define 	FRM_BGN_FRD         0x00
#define 	FRM_LST_BFD         0x01
#define 	FRM_CUR_FRD         0x02
#define 	FRM_CUR_BFD         0x03
#define 	NEXT_RECORD         0x02
#define 	PRIVOUS_RECORD      0x03
#define 	ABSOLUTE_CURRENT    0x04
/*
 * 文件属性操作
*/
#define    FILE_INVALIDATE	   0x00
#define	   FILE_REINVALIDATE   0x01
#define    PIN                 0x00
#define    PIN1                0x01
#define    PIN2                0x81						//correspond to usim
/*
 * CHV状态定义
 */
#define    PIN_BLOCKED         	0
#define    PIN_DISABLED        	0
#define    PIN_UNBLOCKED       	1
#define    PIN_ENABLED         	1
//#define    PIN_LIMIT_OFFSET     2
 
//-----------------------------------------------------------------------------------------------------//
//----------------------------文件类型的模式定义-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
#define    FILETYPE             0x3F
#define    MFTYPE               0x38
#define    ADFTYPE              0x38
#define    DFTYPE               0x38
#define    MF                   0x01
#define    DF                   0x02
#define    EF                   0x04
#define    TRANTYPE             0x01
#define    LINTYPE              0x02
#define    CYCTYPE              0x06        
#define    EFKEY                0x0C
#define    BERTLVTYPE            0x39

//Structure of file for phase2 
#define    UICC_TF                   0x00
//modif by guobing@20240411,与英飞凌库中的宏定义重复
#define    LF                   0x01
#define    CF                   0x03 
//-----------------------------------------------------------------------------------------------------//
//----------------------------生命周期的定义-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
#define		D_CardEmpty				0x00
#define		D_CardInit				0x01
#define		D_CardDefaultFile		0x03
#define		D_CardProfileFile		0x07

    
#define		R_DF_LENGTH				0x17
#define 	R_EF_LENGTH				0x0F

#define     F_MAP_FILE				0x80
#define		F_READ_UPDATA_ENBALE	0x40
#define		F_RFM_PERMIT			0x20
/*****************************************************/
#define		F_RFM_PERMIT_ALL		0x3F	//Bit 0 ：Read/Search
											//Bit 1 ：Update
											//Bit 2 ：Write
											//Bit 3 ：Deactivate
											//Bit 4 ：Activate
											//Bit 5 ：increase
											//Bit 6 ：RFU
											//Bit7 ：RFU

#define		F_RFM_PERMIT_NONE		0x80

#define     F_ARRNAME_6F            0x80
#define     F_ARRNAME_2F            0x00
//-----------------------------------------------------------------------------------------------------//
//----------------------------错误信息的定义-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
#define     E_Parameter         		0x01
#define     E_FileSize          		0x02
#define     E_DfNameExist       		0x03
#define     E_PINMissing        		0x04
#define     E_ConditionNotSat   		0x05
#define     E_InvalidRequest    		0x06
#define     E_FileNotSupportWaring		0x07   // add by xieyong@20211115 拼装警告使用
#define     E_NotSupport        		E_FileNotSupportWaring +1 //modify by chentao@20211126 因为有这个判断if(fileMaxSize <= E_NotSupport)，条件满足认为是创建文件失败，所以E_NotSupport必须错误的最大值
//-----------------------------------------------------------------------------------------------------//
//----------------------------文件操作的定义-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
#define    A_CreateDF		   0x04
#define    A_CreateEF          0x02
#define    A_DeleteEF 		   0x01
#define    A_IncreaseFile      0x32
#define    A_ResizeFile        0xD4
#define    A_DeleteFile        0x40
#define    A_TerminateFile     0x20
#define    A_ActivateFile      0x10
#define    A_DeactivateFile    0x08
#define    A_WriteFile		   0x04
#define    A_UpdateFile		   0x02
#define    A_ReadFile		   0x01   
//-----------------------------------------------------------------------------------------------------//
//----------------------------全局变量的声明-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//   
extern WORD   xdata   G_GPErrorCode;
//extern   	BYTE    xdata    G_APDU[5];
#define     G_APDU  APDUBuffer
#define     RecData (APDUBuffer+5)
#define     G_ApduBuffer APDUBuffer

//extern      BYTE    xdata    G_616CStatus[MAX_LOGICAL_CHANNELS][8];
extern      BYTE    xdata    G_NextAPDUFlag;
#define    G_CurSelectedChannel    selected_channel
//extern      BYTE    xdata    G_CurSelectedChannel;
//extern      BYTE    xdata    RecData[256];
#if RUN_IN_SIMULATOR
extern      BYTE    xdata    tempBuf[TEMP_BUF_SIZE];
#else
extern __attribute__((aligned(4))) BYTE    xdata    tempBuf[TEMP_BUF_SIZE];
#endif
extern      BYTE    xdata    VARIABLEBUFFER[0x200];
extern      BYTE    xdata    G_StatusResponse[2];
extern      BYTE    xdata    G_LocalPinStatus[8];   //LocalPin Status 81~88

extern      BYTE    xdata    G_ChannelNum;
extern   	WORD    xdata    G_CurrentDfName[4];
extern      BYTE    xdata    G_StatusResponse[2];
extern   	WORD    xdata    G_CurrentAdfName[4];
extern   	UWORD   xdata    G_CurrentDfAddr[4];
extern   	UWORD   xdata    G_CurrentEfAddr[4];
extern   	WORD    xdata    G_CurrentEfName[4];
extern   	BYTE    xdata    G_CurrentRecNum[4];
extern   	WORD    xdata    G_CurrentReponse[4];
extern   	BYTE	xdata	 G_CurrentSfi;								//add at 2007-02-07
extern   	BYTE    xdata    ResData[256];
extern   	BYTE    xdata    G_SystemStatus;
extern		BYTE	xdata	 G_CardStatus;
extern    BYTE    xdata    G_UniversalPinStatus;			//Universal PIN status
extern   	BYTE    xdata    G_ChvStatus;
extern      WORD    xdata    G_AdmStatus;
extern      BYTE    xdata    G_KeyStatus;
//extern      WORD    xdata    G_BlockNum; //deleted by chentao@20210626 EPX产业化\INFENION_SLM97\Develop
extern      BYTE    xdata    STKPerform[2];
extern      WORD    xdata    G_STKBusy;
extern 		BYTE    xdata    G_Post9EFlag;  //BIT0:1表示当前状态字为9EXX ,else 0为9FXX
extern      BYTE    xdata    G_CommandMode;
extern      BYTE    xdata    G_UIDShuaKa;   //表示UID刷卡标记
extern      BYTE    xdata    G_M1ShuaKa;    //表示M1刷卡标记
extern      BYTE    xdata    G_CPUShuaKa;   //表示CPU刷卡标记
//extern      BYTE    xdata    G_A4Control;

extern      BYTE    xdata    G_TerminalProfile[0x25]; 
extern 		BYTE    xdata    G_OpenControl;  //开机控制变量， 标识开机流程，初始化上电是0， 收到terminalprofile后置为1
//extern      BYTE    xdata    G_BakRecDataTag;
extern UWORD xdata G_CreatingMFOffset;

extern 		WORD    xdata    G_DF_ID_USIM;

//#if UICC__AUTH__CDMA__CAVE //deleted by zhangxiaoxiao@20210601: EPX产业化/THC_THD89/Branch/EPX_THC_Mobile5G/Develop
extern 		WORD    xdata    G_DF_ID_CSIM;
//#endif

#if UICC__ISIM
extern 	WORD   xdata   G_DF_ID_ISIM;
#endif

extern    BYTE code EUICC_CAPACITY[3];
extern BYTE  xdata G_LpaSendRefresh;


#if RSP_EUICC
extern  BYTE code VersionRSP[4];
#elif M2M_EUICC
extern BYTE code VersionM2M[4];
#endif


#if RSP_EUICC
extern BYTE code PatchVersionRSP[2];
#elif M2M_EUICC
extern BYTE code PatchVersionM2M[2];
#endif

extern  BYTE		xdata g_securityState;

#if UICC__FUNCTION_HIGHUPDATE_IN_HEFLASH
extern BYTE xdata G_UpdateCycleRecPointer;
#endif
/***********function define *****************************/
extern    void   NativeUICC_Main(void);
extern    void   UICCInition(void);
extern    void   SetCardStatus(void);
extern    void ReselectPrivateFile(void);
extern    void setChannelNum(BYTE cla);

extern BYTE    xdata   G_EventBufferOld[17];//LV结构, added by weipingjiao@20210419 EPX产业化\INFENION_SLM97\Develop

//---------------------------network---------------------------
#define  GSM_NETWORK       0x01
#define  CDMA_NETWORK      0x02
//==============================================================================
//    Bit Manipulation macros
//==============================================================================
#define     ClearBit(a,b)       (a) &= ~(1 << (b))
#define     SetBit(a,b)         (a) |= (1 << (b))
#define     ValueBit(a,b)      (((a) >> (b))&0x01)     //使用该表达式，代码大约减少300bytes
//===============================================================================
//	Bit define macros
//===============================================================================
#define    B_cManagerKey    	0                                                // the Manager key is or not varify
#define    B_init				1                                                // the system is or not init
#define    B_cmk				2                                                // the MF key is or not varify
#define    B_RfmPerform			3
#define    B_needCopy			4
#define    B_meSurportUcs2		5
#define    B_needChange			6
#define    B_meSurportStk		7

//===============================================================================
// Chv_Status macros define-Global Pin
//===============================================================================
#define    B_Chv_1_Verified 	0                                      // the bit of tag for chv 1 verified
#define    B_Chv_2_Verified 	1                                      // the bit of tag for chv 2 verified
#define    B_Chv_3_Verified 	2
#define    B_Chv_4_Verified 	3
#define    B_Chv_5_Verified 	4
#define    B_Chv_6_Verified 	5
#define    B_Chv_7_Verified 	6
#define    B_Chv_8_Verified 	7
//===============================================================================
// LocalPin_Status macros define-
//===============================================================================
#define   B_LocalPin_1_Verified 	0
#define   B_LocalPin_2_Verified 	1
#define   B_LocalPin_3_Verified 	2
#define   B_LocalPin_4_Verified 	3
#define   B_LocalPin_5_Verified 	4
#define   B_LocalPin_6_Verified 	5
#define   B_LocalPin_7_Verified 	6
#define   B_LocalPin_8_Verified 	7
//====================================================================================
// ADM_Status mocros define
//=====================================================================================
#define    B_Adm_0_Verified 	3                                      // the bit of tag for adm 0 verified
#define    B_Adm_1_Verified 	4                                      // the bit of tag for adm 1 verified
#define    B_Adm_2_Verified 	5                                     // the bit of tag for adm 2 verified
//---------------------------------------------------------------------------------------//
#define 	ALWAYS     0x00
#define 	CHV1       0x01
#define 	CHV2       0x02
//#define 	RFU        0x03
#define 	ADM0       0x04
#define 	ADM1       0x05
#define 	ADM2       0x06
#define 	NEVER      0x07
//----------------------------发送上行的承载模式-----------------------------------------------------------//
#define	Mode_EF		0x02		//根据配置文件
#define	Mode_SMS	0x00		//短信模式
#define	Mode_BIP	0x01		//BIP模式
//-----------------------------命令模式---------------------------------------------//
#define Command_Mode_First		0x00
#define Command_Mode_2G   		0x02
#define Command_Mode_3G   		0x03
//-----------------------------------------------------------------------------------------//
#define MAIN_MODE  0x00			 //指令是从主流程入口的
#define STK_MODE   0x01			 //指令是stk入口的

/***********************用于LPA有主动性命令,14还没回来,LPA主动关闭通道的场景使用 ****************************/
#define LPANoRefreshMode                                        0x00
#define LPAChangeMode                                           0x01
#define LPAReseteMode                                           0x02

//#define LPAChangeModeWithCloseChannel                           0x11
//#define LPAReseteModeWithCloseChannel                           0x12

//全局变量的宏定义
#define     CLA        G_APDU[0]
#define     INS        G_APDU[1]
#define     P1         G_APDU[2]
#define     P2         G_APDU[3]
#define     P3         G_APDU[4]
//#define 	PPSS 	   G_APDU[0]
//#define		PPS0	   G_APDU[1]
//#define		PPS1	   G_APDU[2]
//#define		PPS2	   G_APDU[3]
//#define		PCK		   G_APDU[4]
#define     SW1        G_StatusResponse[0]
#define     SW2        G_StatusResponse[1]
#endif
