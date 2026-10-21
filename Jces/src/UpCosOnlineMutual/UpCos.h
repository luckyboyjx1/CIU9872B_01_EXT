#ifndef    _UPCOS_H   
#define	   _UPCOS_H 
//#include    "typedef.h"
#include    "ecpDefine.h"
#include    "SysPlatformConfigProfile.h"
#include "FunImp.h"


//根据芯片字节对齐修改
typedef struct upcosStatus
{
	u8	cosSection;//记录维护A区/B区
	u8	cosupStatus;//维护状态
	u16	pageOffset;//u8	pageOffset;//缓存数据在页内偏移
	u32	groupTotalNum;//总分组数
	u32	groupLen;//分组长度（单个）
	u32	upcosSize;//COSUP大小
	u16	blockNum;//指定的block号
	u16	startOffset;//指定的起始地址在block中的偏移
	u32	blockTotalLen;//待处理数据块总长度
	u32	dataOffset;//已处理到的数据偏移
	u32	groupNum;// 组号
	u32	groupOffset;//
	u16	crc;
	u8	cosUpHASH[34];//COSUP校验值  结构：blockNum(2字节)+哈希值(32字节)
	u8	cosUpHASHRemainBuf[0x40];//缓存非block整数块未计算的数据
	
	u8	cosUpsessionIDLen;
	u8	cosUpsessionID[16]; //会话标识 1字节
	u8	upcosName[0x26];//COSUP标识 取upcos文件名
	u8	remainLen;//上一次未处理完的数据，最多10字节
	
	u8	remainBuf[15];

	//链接参数
}UpCosStatus;
//结构体后面需要有0x100长度用于缓存提示语 UPCOS_REMIND_TEXT

extern u8   X_A  G_upCosNextAction;

extern u8  X_A  G_AuthenticationFlag;			//upcos业务中使用，当前cos的进行鉴权标记
extern u8  X_A  G_StatusNumForUPCOSReport;	//upcos 业务中的开机联网成功后接收到的status个数, todo：此值可以考虑和G_STATUSNUM_FOR_NOTIFICATION整合成一个


#define UPCOS_COS_SECTION				0//相对于COSUP_BACKUP_DATA偏移//指定的block号
#define UPCOS_COSUPSTATUS					UPCOS_COS_SECTION + sizeof(u8)
#define UPCOS_PAGEOFFSET                (UPCOS_COSUPSTATUS+sizeof(u8))
#define UPCOS_GROUP_TOTAL_NUM			UPCOS_PAGEOFFSET+sizeof(u16)//总分组数
#define UPCOS_GROUP_LEN					UPCOS_GROUP_TOTAL_NUM+sizeof(u32)//分组长度（单个）
#define UPCOS_UPCOSSIZE					UPCOS_GROUP_LEN+sizeof(u32)//COSUP大小
#define UPCOS_BLOCK_NUM					UPCOS_UPCOSSIZE + sizeof(u32)//相对于COSUP_BACKUP_DATA偏移//指定的block号
#define UPCOS_START_OFFSET				UPCOS_BLOCK_NUM+sizeof(u16)//指定的起始地址在block中的偏移
#define UPCOS_BLOCK_TOTALLEN			UPCOS_START_OFFSET+sizeof(u16)//待处理数据块总长度
#define UPCOS_DATA_OFFSET				UPCOS_BLOCK_TOTALLEN+sizeof(u32)//已处理到的数据偏移
#define UPCOS_GROUP_NUM					UPCOS_DATA_OFFSET+sizeof(u32)// 组号
#define UPCOS_GROUP_OFFSET				UPCOS_GROUP_NUM+sizeof(u32)//当前处理的数据在组内的偏移
#define UPCOS_CRC						UPCOS_GROUP_OFFSET+sizeof(u32)
#define UPCOS_HASH						UPCOS_CRC+sizeof(u16)
#define UPCOS_HASH_REMAIN				UPCOS_HASH+34
#define UPCOS_COSUP_SESSIONID_LEN		UPCOS_HASH_REMAIN+0x40
#define UPCOS_COSUP_SESSIONID			UPCOS_COSUP_SESSIONID_LEN+sizeof(u8)//会话标识 1字节
#define UPCOS_UPCOSNAME					UPCOS_COSUP_SESSIONID+16//COSUP标识 取upcos文件名
#define UPCOS_REMAINLEN                 (UPCOS_UPCOSNAME+0x26)
#define UPCOS_REMAINBUF                 (UPCOS_REMAINLEN+sizeof(u8))
#define UPCOS_STRUCT_END                (UPCOS_REMAINBUF+0x0F)

#define UPCOS_REMIND_TEXT               (UPCOS_REMAINBUF+0x0F)//用于存提示语  LV结构  ，限制长度不超过0x100字节
#define UPCOS_REMIND_MAXLEN             0x100

#define UPCOS_KEYLEN					0x10//COSUP文件加解密密钥长度

/*--------------最近COSUP维护状态定义--------------*/
#define COSUPSTATUS_StartUpFail                0x12 //启动失败
#define COSUPSTATUS_Loading                    0x21	//加载中
#define COSUPSTATUS_LoadingBrokenByCancel      0x22	//加载中断（用户取消）
#define COSUPSTATUS_LoadingBroken              0x23	//加载中断
#define COSUPSTATUS_FailLoading                0x24	//加载失败
#define COSUPSTATUS_FinishLoading              0x25	//加载完成
#define COSUPSTATUS_ActiveFialByCancel         0x31	//激活失败（用户取消）
#define COSUPSTATUS_ActiveFial                 0x32	//激活失败
#define COSUPSTATUS_ActiveSuccessByConfirm     0x01	//激活成功（用户确认）
#define COSUPSTATUS_ActiveSuccess      		   0x00	//激活成功

#define COSUP_SCHEME	         0x0001    //COS升级方案版本号

#define COSCS_A	         1    //A区COS
#define COSCS_B	         2

#define MODE_FUNCVERSION	     00 //设置功能版本
#define MODE_INCVERSION	         80 //设置增量版本

/*--------------COSUP加载完成后的操作--------------*/
#define COSUPNextAction_Null                           0x00 //初始值
#define COSUPNextAction_ActiveByRst   		   		   0x01	//加载完成，待断电激活
#define COSUPNextAction_ActiveImmediately      		   0x02	//加载完成，立即激活
#define COSUPNextAction_ActiveRemind      		       0x03	//激活提醒
#define COSUPNextAction_ActiveRemind_NoDisplay         0x04	//激活提醒-已激活  该情况不需要提醒 只需要开通道发起报送
#define COSUPNextAction_VersionBack      		       0x05	//版本回退
#define COSUPNextAction_VersionConfirm      		   0x06	//版本确认
/*--------------cosup.xx info--------------*/
#define INFO_CARDISSUER                            0x00 //卡商
#define INFO_CARDTYPE                            0x01 //卡品
#define INFO_COSFUNVERSION                           0x02 //功能版本
#define INFO_COSINCVERSION                           0x03 //增量版本
#define INFO_COSSECTION                           0x04 //空间标识 A/B区域
#define INFO_COSCHECKSUN                           0x05 //校验值
#define INFO_EXTENSION                             0x06 //文件夹扩展名
#define INFO_FILENAME_WITHOUTCHECKSUM              0x07 //文件名,不包含校验值
#define INFO_CHIPTYPE_FLAG                        0x08 //匹配芯片型号标识

/*--------------cosup.xx offset--------------*/
#define COSUP_TOTALLEN_OFFSET                    0x00
#define COSUP_CARDISSUER_OFFSET                   0x01
#define COSUP_CARDTYPE_OFFSET                     0x03
#define COSUP_FUNCVERSION_OFFSET                  0x06
#define COSUP_CHIPTYPE_FLAG_OFFSET                0x0C
#define COSUP_INCVERSION_OFFSET                   0x10
#define COSUP_COSSECTION_OFFSET                   0x16
#define COSUP_CHECKSUM_OFFSET                     0x18
#define COSUP_EXTENSION_OFFSET                    0x20//后缀名

#define COSUP_FILENAME_LEN                        0x26

extern     u8 g_UPCOS_storeDataBuf[];//COS数据缓存页
extern     UpCosStatus X_A G_upCosStatus;
extern     u16  UPCOS_COSMaintenance(void);
extern     u16  UPCOS_GetData(void);
extern     u16  UPCOS_CosLoading(void);
extern     u16   UPCOS_UpdateInit(void);
extern     u16   UPCOS_ResumeUpdate(void);
extern     u16   UPCOS_ActivationRemind(void);
extern     u16   UPCOS_FinishLoading(u8 type);
extern     u16   UPCOS_FailLoading(void);
extern     u16   UPCOS_switchCOS(u16 COS_Version);
extern     u16   UPCOS_versionAffirm(void);
extern     u16   UPCOS_VersionBack(void);
extern     void processD0D1Head(u8 type, PX8 buffer, u8 offset);
extern     u16 UPCOSDecryptData(PM8 decDataBuf, u16 offset);
extern 	   u16  NM_IsUPCOSCommand(void);
extern     void valueToBuff(PX8 buff, u32 value, u8 Len);
extern     void InitUpCOSVar(void);
extern     u16 GetCurCosVersion(void);
extern     u32 GetRemainCodeSize(u8 flag);
extern     u8 GetBreakPoint(void);
extern     void ActiveCOS(void);
extern     u8 SwitchCOS(u16 CosVersion, u8 cosFlag);
extern     u8 GetMaintenanceInfoByFileName(u8 infoType, PX8 info);
extern     u16 UPCOSGetDataSubFunc(PM8 dstBuf);
extern     u16 PaddingActiveReming(PM8 outBuf);
extern     u8 VersionAffirmDisplay(void);
extern     void IF_UPCOS_ExpandHandle(void);
extern     void IF_UPCOS_ExpandHandle_ForCloseHttpChannel(void);
extern     void SetCOSUPStatus(u8 status);
extern     void SetCOSUPNextAction(u8 action);
extern     void STK_UpgradeFunc(void);
//extern     u16 UPCOS_SetCOSVersion(void);
extern     u16 SetCosVersion(u8 cosSection,u8 mode,u16 version);

//extern u8 CONST BackUp_data[];
extern u8 CONST UPCosPara[];



/*****************/
#if !HAL__PLATFORM__SIMULATOR
#define  COSA_START_ENTRY         CHIP_CONFIG_START
#define  COSB_START_ENTRY         BOOT_CONFIG_START


#if UPCOS_COSB
#define COS_SIZE				(COSA_START_ENTRY - COSA_DATA_START)//
#else 
#define COS_SIZE				(COSA_BACKENTRY - BOOT_LOADER_START)//
#endif

#else
#define  COSA_START_ENTRY         ((u32)0x100C7E00) //要与SCT对应上
#define  COSB_START_ENTRY         ((u32)0x100C7F00)
#define  COSA_BACKENTRY           ((u32)0x101F3700) //要与SCT对应上
#define  COSB_BACKENTRY           ((u32)0x101F3800) //要与SCT对应上
#define  COSA_DATA_START          ((u32)0x10009000) //要与SCT对应上

#define COS_SIZE				(COSA_START_ENTRY - COSA_DATA_START)//
#endif

#endif
