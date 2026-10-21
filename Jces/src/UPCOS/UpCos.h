#ifndef    _UPCOS_H   
#define	   _UPCOS_H 
//#include    "typedef.h"
#include    "ecpDefine.h"
#include    "SysPlatformConfigProfile.h"


typedef struct upcosStatus
{
//	u8	cosSection;//记录维护A区/B区
//	u8	blockNum;//指定的block号
	u16	blockNum;//指定的block号,IFX代码区从0x10009000开始，block号应从1000开始
	u16	startOffset;//指定的起始地址在block中的偏移
	u32	blockTotalLen;//待处理数据块总长度
	u32	dataOffset;//已处理到的数据偏移
	u32	groupNum;// 组号
//	u32	groupOffset;//
	u16	crc;
//	u8  cosUpHASH[34];//COSUP校验值  结构：blockNum(2字节)+哈希值(32字节)
//	u8  cosUpHASHRemainBuf[0x40];//缓存非block整数块未计算的数据
	u32	groupTotalNum;//总分组数
	u32	groupLen;//分组长度（单个）
	u32 upcosSize;//COSUP大小
	//u8  cosUpsessionIDLen;
	u8	cosUpsessionID[16]; //会话标识 1字节
	u8	upcosName[0x26];//COSUP标识 取upcos文件名
	
	u8	cosupStatus;//维护状态
	u16	pageOffset;//u8	pageOffset;//缓存数据在页内偏移
	u8	remainLen;//上一次未处理完的数据，最多10字节
	u8	remainBuf[15];

	//链接参数
}UpCosStatus;

extern u8   X_A  G_upCosNextAction;

extern u8  X_A  G_AuthenticationFlag;			//upcos业务中使用，当前cos的进行鉴权标记
extern u8  X_A  G_StatusNumForUPCOSReport;	//upcos 业务中的开机联网成功后接收到的status个数, todo：此值可以考虑和G_STATUSNUM_FOR_NOTIFICATION整合成一个


//#define UPCOS_COS_SECTION				0//相对于COSUP_BACKUP_DATA偏移//指定的block号
//#define UPCOS_BLOCK_NUM					0//相对于COSUP_BACKUP_DATA偏移//指定的block号
//#define UPCOS_START_OFFSET				UPCOS_BLOCK_NUM+sizeof(u16)//指定的起始地址在block中的偏移
//#define UPCOS_BLOCK_TOTALLEN			UPCOS_START_OFFSET+sizeof(u16)//待处理数据块总长度
//#define UPCOS_DATA_OFFSET				UPCOS_BLOCK_TOTALLEN+sizeof(u32)//已处理到的数据偏移
//#define UPCOS_GROUP_NUM					UPCOS_DATA_OFFSET+sizeof(u32)// 组号
//#define UPCOS_GROUP_OFFSET			UPCOS_GROUP_NUM+sizeof(u32)//当前处理的数据在组内的偏移
//#define UPCOS_CRC						UPCOS_GROUP_NUM+sizeof(u32)
//#define UPCOS_HASH					UPCOS_CRC+sizeof(u16)
//#define UPCOS_HASH_REMAIN				UPCOS_HASH+34
//#define UPCOS_GROUP_TOTAL_NUM			UPCOS_CRC+sizeof(u16)//总分组数
//#define UPCOS_GROUP_LEN					UPCOS_GROUP_TOTAL_NUM+sizeof(u32)//分组长度（单个）
//#define UPCOS_UPCOSSIZE					UPCOS_GROUP_LEN+sizeof(u32)//COSUP大小
//#define UPCOS_COSUP_SESSIONID_LEN		UPCOS_UPCOSSIZE+sizeof(u32)
//#define UPCOS_COSUP_SESSIONID			UPCOS_COSUP_SESSIONID_LEN+sizeof(u8)//会话标识 1字节
//#define UPCOS_UPCOSNAME					UPCOS_COSUP_SESSIONID+16//COSUP标识 取upcos文件名
//#define UPCOS_COSUPSTATUS               (UPCOS_UPCOSNAME+0x26)
//#define UPCOS_PAGEOFFSET                (UPCOS_COSUPSTATUS+sizeof(u8))
//#define UPCOS_REMAINLEN                 (UPCOS_PAGEOFFSET+sizeof(u16))
//#define UPCOS_REMAINBUF                 (UPCOS_REMAINLEN+sizeof(u8))
//#define UPCOS_STRUCT_END                (UPCOS_REMAINBUF+0x0F)

//#define UPCOS_REMIND_TEXT               (UPCOS_REMAINBUF+0x0F)//用于存提示语  LV结构  ，限制长度不超过0x100字节
//#define UPCOS_REMIND_MAXLEN             0x100

#define UPCOS_KEYLEN					0x10//COSUP文件加解密密钥长度

/*--------------cosupStatus状态定义--------------*/
#define COSUPSTATUS_NULL                	   0x00 //初始值
#define COSUPSTATUS_UpdateInit                 0x01 //升级初始化成功
#define COSUPSTATUS_Loading                    0x02	//加载中
#define COSUPSTATUS_LoadingBroken              0x03	//加载中断
#define COSUPSTATUS_FailLoading                0x04	//加载失败
#define COSUPSTATUS_FinishLoading              0x05	//加载完成
#define COSUPSTATUS_ActiveSuccess      		   0x06	//激活成功
#define COSUPSTATUS_ActiveFail      		   0x07	//激活失败

//#define COSUP_SCHEME	         0x0001    //COS升级方案版本号
#define COSUP_SCHEME	         0x0100    //COS升级方案版本号

//#define COSCS_A	         1    //A区COS
//#define COSCS_B	         2

#define COS_INVALID	         0x00 //功能版本号无效 ，表示正在下载COS  状态未知
#define COS_VALID	         0x01 //功能版本号有效 ，表示下载的COS已校验CRC成功

/*--------------COSUP加载完成后的操作--------------*/
//#define COSUPNextAction_Null                           					   0x00 //初始值
//#define COSUPNextAction_RefreshAfterCloseHttpChannel   		   		   0x01	//加载完成，关HTTP通道后fresh


/*--------------cosup.xx info--------------*/
#define INFO_CARDISSUER                            0x00 //卡商
#define INFO_CARDTYPE                              0x01 //卡品
#define INFO_COSFUNVERSION                         0x02 //功能版本
#define INFO_COSIDENTIFY                           0x03 //COS识别标识
#define INFO_COSCHECKSUN                           0x04 //校验值
#define INFO_EXTENSION                             0x05 //文件夹扩展名
#define INFO_FILENAME_WITHOUTCHECKSUM              0x06 //文件名,不包含校验值

/*--------------cosup.xx offset--------------*/
#define COSUP_TOTALLEN_OFFSET                     0x00
#define COSUP_CARDISSUER_OFFSET                   0x01
#define COSUP_CARDTYPE_OFFSET                     0x03
#define COSUP_FUNCVERSION_OFFSET                  0x06
#define COSUP_COSIDENTIFY_OFFSET                  0x0C
#define COSUP_CHECKSUM_OFFSET                     0x11
#define COSUP_EXTENSION_OFFSET                    0x19//后缀名

#define COSUP_FILENAME_LEN                        0x1F

extern     u8 g_UPCOS_storeDataBuf[];//COS数据缓存页
extern     UpCosStatus X_A G_upCosStatus;
extern     u8 IF_UPCOS_EnvelopHandle(void);
extern     u8 IF_UPCOS_SSDAppId(PM8 aidBuf,u8 aidLen);
extern     u16  UPCOS_COSMaintenance(void);
//extern     u16  UPCOS_GetData(void);
extern     u16  UPCOS_CosLoading(void);
extern     u16   UPCOS_UpdateInit(void);
extern     u16   UPCOS_FinishLoading(u8 type);
extern     u16 processD0D1Head(u8 type, PX8 buffer, u8 offset);
extern     u16 UPCOSDecryptData(PM8 decDataBuf, u16 offset);
extern 	   u16  NM_IsUPCOSCommand(void);
extern     void InitUpCOSVar(void);
extern     u16 GetCurCosVersion(void);
extern     u32 GetRemainCodeSize(u8 flag);
extern     void ActiveCOS(void);
extern     u8 GetMaintenanceInfoByFileName(u8 infoType, PX8 info);
extern     u16 UPCOSGetDataSubFunc(PM8 dstBuf);
extern     u16 PaddingActiveReming(PM8 outBuf);
extern     void IF_UPCOS_ExpandHandle_ForCloseHttpChannel(void);
extern     u16 UPCOSPrivateCommand(PX8 apduBuffer);
extern     u8 IF_UPCOS_GetPSK(u8* pskBuf, u8 keyversion,u8 sdId);



#define  COS_START_ENTRY         CHIP_CONFIG_START
#define  BOOT_START_ENTRY         BOOT_CONFIG_START



#endif
