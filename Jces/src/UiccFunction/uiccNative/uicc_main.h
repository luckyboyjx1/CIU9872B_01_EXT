#ifndef    _main_h_
#define    _main_h_
#include   "typedef.h"
#include   "eepromdefine.h"
#include   "global_variable.h"
#include   "FunEntryConfig.h"




typedef __packed  struct tagFileIdType
{
   WORD   FileId;                                  // 2     0      2    0
   BYTE   FileType;                                // 1     2      1    2
}FileIdType;

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

#define MODE_PRIVATE 0x01
#define MODE_DEFAULT 0x02

//todo:临时放这里
#define OPERATOR_DEFAULT       0xFF    //默认设置成标准模式
#define OPERATOR_CHINAMOBILE   0x00
#define OPERATOR_CHINAUNICOM   0x01
#define OPERATOR_CHINATELECOM  0x02


#if !HEAD_BODY_TOGETHER
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
   WORD   FileContentRef;	                       // FileContentRef move to hear, for UWORD align,by xwh 20220317
   UWORD  UsedContentSize;                         // 4     6      2    6
   UWORD  UsedHeadSize;                            // 4     A      2    8
   BYTE   ChildDfNum;                              // 1     E      1    A
   BYTE   ChildEfNum;                              // 1     F      1    B
   WORD   ParentName[2];	                       // 4     10     4    C 
   BYTE   ChvStatus[4];                            // 4	    14     4    10
   //WORD    FileContentRef;	
   //BYTE   Rfu;
   //BYTE	  KeyLmit;
   //BYTE   MF_Key[8];
}FileSystemHead;

typedef __packed  struct tagSingleFileHead
{
   UWORD  UsedContentSize;                                  //4     0      2    0
   UWORD  UsedHeadSize;                                 //4     4      2    2
   BYTE   ChildDfNum;                               //1     8      1    4
   BYTE   ChildEfNum;                               //1     9      1    5
}SingleFileHead;

#else


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

#endif


//-------------------------------------------------------------------------------------------------//
//-------------------------------二进制 的文件头结构-----------------------------------------------//
//-------------------------------------------------------------------------------------------------//


typedef __packed  struct tagEFileHead
{
    WORD    FileId;                                 //  2    0
    BYTE    FileType;                               //  1    2
	BYTE    ArrNum;								    //  1    3  EFarr中记录号
    BYTE    ShortFileId;                            //  1    4
    BYTE    ArrName;                                //  1    5
   
    __packed union
	{
	   	WORD    SpaceSize;                              //  2    6
		BYTE    recInfo[2];
	}efSize;

    BYTE    UpdateConfignum;                                    //  1    8    强更新指针索引 1-n 不是强更新文件为0
   	BYTE    FileStatus;                             //  1    9
	WORD    MapFile[3];								//  4	 A
	#if NATIVE_CARD_UICC
	WORD    FileContentRef;                           //文件内容引用
	#endif
}EFileHead;


//-------------------------------------------------------------------------------------------------//
//-------------------------------CHV/PUK结构设计--------------------------------------------------//
//------------------------------------------------------------------------------------------------//
#if NATIVE_CARD_UICC
typedef __packed  struct tagKeyStructer
{
 	BYTE    KeyId;	            //1    0
	BYTE    KeyStatus;			//1    1
	BYTE    KeyLimit;			//1	   2
	BYTE	IscLimit;			//1	   3
	BYTE    KeyContent[8];		//8	   4
	BYTE    IscContent[8];    	//8	   C
}KeyStructer;
#else
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

#endif


#define VM_MAX_LOGICAL_CHANNELS       MAX_LOGICAL_CHANNELS //(0x08)
#define VM_MAX_LOGICAL_CHANNELS_CT    ISO_MAX_LOGICAL_CHANNELS//(0x04)

#define     REF_NULL                0x00 
#define     Heap_ConfigBytes_Len   	6
#define Heap_ValidData_Tag     0xD0   //标识当前heap数据有效
#define Heap_InValidData_Tag   0xFF   //标识当前heap数据无效

#define     F_BERTLV_UsedSize           ( 5*sizeof(WORD)+ 6*sizeof(BYTE))
#define     F_BERTLV_ReservedSize     	( 6*sizeof(WORD)+ 6*sizeof(BYTE))
#define     F_BERTLV_Detail            	 ( 2*sizeof(WORD)+ 4*sizeof(BYTE))
//----------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------//
//-------------------------------文件头结构的偏移--------------------------------------------------//
/////////////////////////////////MF DF header offset-----------------------------------------------//
#if NATIVE_CARD_UICC
#define    F_Name_Offset              0x00
#define    F_Type_Offset              (F_Name_Offset + sizeof(WORD))
#define    F_ArrNum_Offset		      (F_Type_Offset + sizeof(BYTE))
#define    F_DIRNum_offset  	      (F_ArrNum_Offset + sizeof(BYTE))
#define    F_ArrName_Offset           (F_DIRNum_offset + sizeof(BYTE)) 
#define    F_DFContent_Offset         (F_ArrName_Offset + sizeof(BYTE)) 
#define	   F_UsedContentSize_Offset   (F_DFContent_Offset + sizeof(WORD))
#define    F_UsedHeadSize_Offset      (F_UsedContentSize_Offset + sizeof(UWORD))
#define    F_ChildDFNum_Offset        (F_UsedHeadSize_Offset + sizeof(UWORD))
#define    F_ChildEFNum_Offset        (F_ChildDFNum_Offset + sizeof(BYTE))
#define    F_ParentName_Offset        (F_ChildEFNum_Offset + sizeof(BYTE))
#define    F_SecondParentName_Offset  (F_ParentName_Offset + sizeof(WORD))
#define    F_ChvStatus_Offset         (F_SecondParentName_Offset + sizeof(WORD))

/////////////////////////////////EF header offset-----------------------------------------------//
//#define    F_Name_Offset            0x00
//#define    F_Type_Offset            (F_Name_Offset + sizeof(WORD))
//#define    F_ArrNum_Offset		  (F_Type_Offset + sizeof(BYTE))
#define    F_ShordFid_Offset          (F_ArrNum_Offset + sizeof(BYTE))
//#define    F_ArrName_Offset         (F_ShordFid_Offset + sizeof(BYTE)) 
#define	   F_SpaceSize_Offset	      (F_ArrName_Offset + sizeof(BYTE)) 
#define    F_RecNum_Offset            (F_ArrName_Offset + sizeof(BYTE)) 
#define    F_RecLgth_Offset           (F_RecNum_Offset + sizeof(BYTE))
#define	   F_Pointer_Offset           (F_RecLgth_Offset + sizeof(BYTE))
#define    F_Status_Offset            (F_Pointer_Offset + sizeof(BYTE))
#define    F_MapFile_Offset           (F_Status_Offset + sizeof(BYTE))
#define    F_Content_Offset           (F_MapFile_Offset + 3*sizeof(WORD))
//-------------------------------MFKEY结构的偏移--------------------------------------------------//
#define	   F_MFKEYLimit_Offset				0x08

#else

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


#endif

//#define    F_BaudStatus_Offset		F_KLMT_Offset

//#define    E_MK_START		  	    (E_FILE_START+(3*sizeof(WORD)+12*sizeof(BYTE)+2*sizeof(UWORD))) 
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
#define    PIN_LIMIT_OFFSET     2          
 
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
//Structure of file for phase2 
#define    TF                   0x00        
#define    LF                   0x01
#define    CF                   0x03 
//-----------------------------------------------------------------------------------------------------//
//----------------------------生命周期的定义-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
#define		D_CardEmpty				0x00
#define		D_CardCreate			0x01
#define		D_CardInit				0x03
#define     D_CardSecured           0x0F
//#define		D_CardNormal			0x04
//#define		D_CardTerminal			0xC0

    
#define		R_DF_LENGTH				0x17
#define 	R_EF_LENGTH				0x0F

#define     F_MAP_FILE					0x80
#define		F_READ_UPDATA_ENBALE		0x40
#define		F_RFM_PERMIT				0x20


#define		F_RFM_PERMIT_UNICOM			0x30
#define		F_RFM_UPDATA			    0x20
#define		F_RFM_READ				    0x10	//seek同read


#define		F_RFM_PERMIT_ALL		0x1F
#define		F_RFM_PERMIT_NONE		0x80

#define     F_ARRNAME_6F            0x80
#define     F_ARRNAME_2F            0x00

#define	   F_MapAndRead_Offset		F_ShordFid_Offset
#define    File_MapAndRead          0x80
#define    Tag_File_MapAndRead		0x93	

//-----------------------------------------------------------------------------------------------------//
//----------------------------错误信息的定义-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
#define     E_Parameter         0x01
#define     E_FileSize          0x02
#define     E_DfNameExist       0x03
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



#define SQN_SIZE                                192  

#define OPC_SIZE                                111 


#define TranFileHead    EFileHead
#define RecordFileHead  EFileHead
//-----------------------------------------------------------------------------------------------------//
//----------------------------全局变量的声明-----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//   
//extern      BYTE *			APDUBuffer;
#define     G_APDU  APDUBuffer
#define     RecData (APDUBuffer+5)

//extern   	BYTE    xdata    G_APDU[5];
extern      BYTE    xdata    G_ChannelNum;
extern   	WORD    xdata    G_CurrentDfName[4];
extern      BYTE    xdata    G_StatusResponse[2];
extern   	WORD   xdata    G_CurrentAdfName[4];
extern   	UWORD   xdata    G_CurrentDfAddr[4];
extern   	UWORD   xdata    G_CurrentEfAddr[4];
extern   	WORD    xdata    G_CurrentEfName[4];
extern   	BYTE    xdata    G_CurrentRecNum[4];
extern   	WORD    xdata    G_CurrentReponse[4];
extern   	BYTE	xdata	 G_CurrentSfi;								//add at 2007-02-07
//extern   	BYTE    xdata    RecData[256];
extern   	BYTE    xdata    ResData[256];
extern      BYTE    xdata    tempBuf[0x200];
extern   	BYTE    xdata    G_SystemStatus;
extern		BYTE	xdata	 G_CardStatus;
extern   	BYTE    xdata    G_ChvStatus;
extern      BYTE    xdata    G_LocalPinStatus[8];   //LocalPin Status 81~88

extern      BYTE    xdata    G_KeyStatus;
extern      BYTE    xdata    G_AdmStatus;
extern      BYTE    xdata    G_BlockNum;
extern      BYTE    xdata    STKPerform;
extern      BYTE    xdata    G_ChannelStatus;
extern	    BYTE    xdata	 G_ResponseMode;
extern      BYTE    xdata    G_CommandMode;
extern      BYTE    xdata    G_A4Control;
extern      BYTE    xdata    G_UnicomResponseLgth;
extern      BYTE    xdata    G_FirstCommand;
extern      BYTE    xdata    G_SuperKeyAccess;
extern      BYTE   xdata   G_SuperKeyRamdomBuf[0x10]; // added by yangjian@20221205
extern      BYTE 	xdata    G_ResponseStatus;
extern 		BYTE	xdata	 G_OpenControl;			//开机控制变量。
extern      BYTE    xdata G_FDNProcessed;
extern      BYTE    xdata  	G_SetUpEventListSent;
extern      BYTE    xdata   G_NetWorkMode;
extern      BYTE    xdata    G_Post9EFlag;  //BIT0:1表示当前状态字为9EXX ,else 0为9FXX
extern      BYTE    xdata    G_CurrentRFMResponse;       //通过getresponse需要获取的当前POR的数据长度

extern      code BYTE keyedition[2];
extern      code BYTE keydata[128];




extern BYTE xdata G_CATTP_PUSH1;

/*=========================SuperKeyAccess==============================================*/
#define    SuperKeyAccess_NoVerify      0x00
#define    SuperKeyAccess_PassVerify    0x01
#define    SuperKeyAccess_GetChallange  0x02

//================================KeyType=============================================
#define SuperKey_Type   0xA1
#define EncryKey_Type   0xA2  //用于对输出的E2进行加解密

//---------------------------network---------------------------
#define  GSM_NETWORK       0x01
#define  CDMA_NETWORK      0x02
 
//==============================================================================
//    Bit Manipulation macros
//==============================================================================
#define     ClearBit(a,b)       (a) &= ~(1 << (b))
#define     SetBit(a,b)         (a) |= (1 << (b))
//#define     ValueBit(a,b)       (((a) & (1 << (b)))>>(b))
#define      ValueBit(a,b)       (((a) >> (b))&0x01)     //使用该表达式，代码大约减少300bytes
//===============================================================================
//	Bit define macros
//===============================================================================
#define    B_init				1                                                // the system is or not init
#define    B_cmk				2                                                // the DF key is or not varify
#define    B_RfmPerform			3
#define    B_needCopy			4
#define    B_meSurportUcs2		5
#define    B_needChange			6
#define    B_meSurportStk		7
//===============================================================================
// Chv_Status macros define
//===============================================================================
//#if NATIVE_CARD_UICC
//#define    B_Chv_1_Verified 	1                                      // the bit of tag for chv 1 verified
//#else
#define    B_Chv_1_Verified 	0                                      // the bit of tag for chv 1 verified
//#endif
#define    B_Chv_2_Verified 	2                                      // the bit of tag for chv 2 verified
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
#define 	RFU        0x03
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
//全局变量的宏定义
#define     CLA        G_APDU[0]
#define     INS        G_APDU[1]
#define     P1         G_APDU[2]
#define     P2         G_APDU[3]
#define     P3         G_APDU[4]
#define 	PPSS 	   G_APDU[0]
#define		PPS0	   G_APDU[1]
#define		PPS1	   G_APDU[2]
#define		PPS2	   G_APDU[3]
#define		PCK		   G_APDU[4]
#define     SW1        G_StatusResponse[0]
#define     SW2        G_StatusResponse[1]


#define BuildUWord(buf)    ((UWORD)(((UWORD)(buf)[0]<<24)|((UWORD)(buf)[1]<<16)|((UWORD)(buf)[2]<<8)|(buf)[3]))
#define SetUWord(buf,value)	do{(buf)[0]=((value)>>24)&0xFF; (buf)[1]=((value)>>16)&0xFF; (buf)[2]=((value)>>8)&0xFF; (buf)[3]=(value)&0xFF; }while(0)

#define BuildWord(buf)    ((WORD)(((WORD)(buf)[0]<<8)|(buf)[1]))

#define SetWord(buf,value)	do{(buf)[0]=((value)>>8)&0xFF; (buf)[1]=(value)&0xFF; }while(0)


extern void SystemInit_Variable(void);


#define BLOCKNUM 4


#define D_DeleteFile		0x40
#define D_Resize			0x80
#define D_CreateFile        0x20


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

#define INVALID_ADDR              0xFFFFFFFF

//extern DWORD xdata G_CODE_SIZE;

extern unsigned char  xdata PADDING[64];

extern  void InsError(void);

#endif
