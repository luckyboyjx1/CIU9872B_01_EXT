
#include "global_variable.h"
#include "FunEntryConfig.h"
#include "ecpOperation.h"
#include "target.h"
#include "bootPlaformConfigProfile.h"
#include "Boot.h"
#include "FunImp.h"
#if ((COS_UPDATE_ONLINE_BY_NativeOS||COS_UPDATE_ONLINE_MUTUAL)&& (!BOOTLOADER_PRO))
#include "EUICCMain.h"
#include "platform_api.h"
#include "object.h"
#include "stkhead.h"
#include "opFunction.h"
#include "keySet.h"
#endif
extern void BusFault_Handler(void);
#if BOOTLOADER
__attribute__( ( section( "FunEntry_data" ) ) )
FunctionEntry_t	CONST	FuncEntry	={
#if (SUPPORT_COS_UPDATE && (!BOOTLOADER_PRO) && (!UPCOS_COSB))
	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
    NULL,
	NULL,
	NULL,
	NULL,
	NULL,

#endif
	
};


__attribute__( ( section( "FunBootEntry_data" ) ) )

FunctionEntry_t	CONST	FuncBootEntry	={
#if (SUPPORT_COS_UPDATE && (BOOTLOADER_PRO || UPCOS_COSB))
	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif
};
__attribute__( ( section( "CosABackEntry_data" ) ) )

FunctionEntry_t	CONST	FuncBackEntry	={
#if (COS_UPDATE_ONLINE_MUTUAL||COS_UPDATE_ONLINE_BY_NativeOS)&&(!BOOTLOADER_PRO)&&(!UPCOS_COSB)
	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif
};


__attribute__( ( section( "CosBBackEntry_data" ) ) )

FunctionEntry_t	CONST	FuncBootBackEntry	={
//#if (UPCOS_COSB)
#if (COS_UPDATE_ONLINE_MUTUAL && UPCOS_COSB)
	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,

	resetInit_Process,
	appMain_Process,
	IrqEntry1_Process,
	IrqEntry2_Process,
	IrqEntry3_Process,
	IrqEntry4_Process,
	IrqEntry5_Process,
	IrqEntry6_Process,
	IrqEntry7_Process,
	IrqEntry8_Process,
	IrqEntry9_Process,
	
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif
};

#endif

#if  COS_UPDATE_ONLINE_MUTUAL
__attribute__((section("COSUPBackUp_data"))) 
u8 CONST BackUp_data[EEPAGE_SIZE] = {0x00};

__attribute__((section("UPCosPara_data")))
u8 CONST UPCosPara [EEPAGE_SIZE] = {
  /*UPGRADE_EN_MODE               			0x01(0x01)*/
	0x01,	
	/*UPCOS_ENC_KEY[]           0x10(0x10)*/
	0x95,0x62,0xBB,0xC6,0xD3,0x80,0x9E,0xC0,0xDB,0x70,0xC0,0x13,0xB6,0xF8,0x22,0xEC,
	/*COSUM_SSD_CNTR           0x05(0x05)*/
	0x00,0x00,0x00,0x00,0x00,
	/*COSUM_SSD_AID_LEN*/
	0x10,
	/*COSUM_SSD_AID*/
	0xD1,0x56,0x00,0x01,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x01,
	/*COSUM_SSD  SCP80key Version for Send Message*/ //用于主动发短信查询版本
	0x01,
	/*DEVICE TYPE 设备类型，默认人联网设备*/
	0x01,
	/*UPCOSA_FUNVERSION_OFFSET*/
	0x01,0x00,
	/*UPCOSA_INCVERSION_OFFSET*/
	0x00,0x00,
	/*UPCOSB_FUNVERSION_OFFSET*/
	0x00,0x00,
	/*UPCOSB_INCVERSION_OFFSET*/
	0x00,0x00,
	/*COSUP_NEXT_ACTION_OFFSET*/
	0x00,
	/*ACTIVE_COS_OFFSET*/
	0x01,
	/**/
	0xff,0xff


};

typedef struct TagControl{	//定义同Boot.c
	u16 tagName;             //store/get data P1P2 tag 
	u8  property;            //RFU //Tag property contain bit/Byte tyep , life Cycle etc
	u8  length;              //tag len  //when using bit to config, then this byte will be reused for bit maping.
	u16 offsetAddr;          //Tag offset Address
}ConfigTagList;

#define RFU  0
static const ConfigTagList UPCos_ConfigTaglist[]=
{
	{0x0000,  RFU,   0x10, UPCOS_KEY_OFFSET,}, 
	{0x0001,  RFU,   0x05, COSUM_SSD_CNTR_OFFSET,}, 
	{0x0002,  RFU,   0x01, COSUM_SSD_AIDLEN_OFFSET,}, 
	{0x0003,  RFU,   0x10, COSUM_SSD_AID_OFFSET,}, 
	{0x0004,  RFU,   0x01, COSUM_SSD_KEYVERSION_OFFSET,} ,
	{0x0005,  RFU,   0x01, UPCOS_DEVICETYPE_OFFSET,} ,
	{0x0006,  RFU,   0x02, UPCOSA_FUNVERSION_OFFSET,} ,
	{0x0007,  RFU,   0x02, UPCOSA_INCVERSION_OFFSET,} ,
	{0x0008,  RFU,   0x02, UPCOSB_FUNVERSION_OFFSET,} ,
	{0x0009,  RFU,   0x02, UPCOSB_INCVERSION_OFFSET,} ,
//	{0x000A,  RFU,   0x01, COSUP_NEXT_ACTION_OFFSET,} ,//该标记不需要私有指令操作
	{0x000A,  RFU,   0x01, ACTIVE_COS_OFFSET} ,
	{0x000B,  RFU,   0x01, UPGRADE_EN_MODE} ,

	/*RFU*/
	{0xFFFF,  RFU,   0x02, 0xFFFF,} 
	
};

#else

#if COS_UPDATE_ONLINE_BY_NativeOS||BOOTLOADER

__attribute__( ( section( "BootLoaderPara_data" ) ) )
u8 CONST BootLoaderPara [EEPAGE_SIZE] = {

	/*FileSystem_StartAddr*/
	0x00,0x00,0x00,0x00,
	/*Private FileSystem Addr*/
	0x00,0x00,0x00,0x00,
	/*BIPModule_Receive_ADDR_START*/
	0x00,0x00,0x00,0x00,	
	/*DownLoad_COS_ENKEY           0x10*/
	0x95, 0x62, 0xBB, 0xC6, 0xD3, 0x80, 0x9E, 0xC0, 0xDB, 0x70, 0xC0, 0x13, 0xB6, 0xF8, 0x22, 0xEC,
    /*UPGRADE_EN_MODE               			0x01*/
	0x01,
	/*COS Identity*/  //用于与xx.cosup文件的COS识别标识匹配，匹配成功才可以升级，该标识是为了区分xx.cosup文件是否可以写到该卡（sct能对应才不会坏卡）
	0x00,0x01,
	/*COS_FUNVERSION*/
	 0x01,0x00,
	/*UPCOSFunVersionFlag*/
	 0x01,
	/*CARD_PRODUCT 卡品代码*/
	0x30,0x32,
	/*COSUM_SSD_AID_LEN*/
	0x10,
	/*COSUM_SSD_AID*/ //用于管理在线升级的SD AID 
	0xD1,0x56,0x00,0x01,0x01,0x00,0x01,0x60,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,	
	/*UPGRADE_COSUM_SSD_APPID*/
	0xFF,
	/*COSUM_SSD  SCP80key Version for RAM*/ //用于GETDATA 或短信触发建立HTTP会话
	0x01,	
	/*COSUM_SSD  SCP80key - kicLen*/
	0x10,
	/*COSUM_SSD  SCP80key - kic -- max 24 bytes*/
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*COSUM_SSD  SCP80key - kidLen*/
	0x10,
	/*COSUM_SSD  SCP80key - kid -- max 24 bytes*/
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*COSUM_SSD  RAM TAR--3bytes*/
	0xCF,0xFF,0xF0,
	/*UPGRADE_COSUM_SSD_MSL*/
	0x00,  //
	/*COSUM_SSD  RAM key counter addr -- 4bytes*/
	0x00,0x00,0x00,0x00,
	/*COSUM_SSD  SCP81key Version for RAM(COS UP)*/ //用于升级过程中建立HTTP链接时分散会话秘钥的KEY VERSION
	0x40,
	/*COSUM_SSD  SCP81key - PSK Len*/ 
	0x10,
	/*COSUM_SSD  SCP81key - PSK -- max 32 bytes*/ 
	0x88,0x7F,0x65,0x8F,0xD4,0xBA,0xD4,0x93,0x88,0x7F,0x65,0x8F,0xD4,0xBA,0xD4,0x93,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/*UPGRADE_CARD_SEID_LEN*/
	0x10,
	/*esim卡时为EID，16字节；普通卡为ICCID，10字节*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	
	/*end flag*/
	0xFF
};

typedef struct TagControl{	//定义同Boot.c
	u16 tagName;             //store/get data P1P2 tag 
	u8  property;            //RFU //Tag property contain bit/Byte tyep , life Cycle etc
	u8  length;              //tag len  //when using bit to config, then this byte will be reused for bit maping.
	u16 offsetAddr;          //Tag offset Address
}ConfigTagList;

#define RFU  0
static const ConfigTagList UPCos_ConfigTaglist[]=
{
	{0x0000,  RFU,   0x10, UPGRADE_DOWNLOAD_KEY,}, //保留tag与移动的UPCOS一致
//	{0x0001,  RFU,   0x05, COSUM_SSD_CNTR_OFFSET,}, 
	{0x0002,  RFU,   0x01, UPGRADE_COSUM_SSD_AID_LEN,}, 
	{0x0003,  RFU,   0x10, UPGRADE_COSUM_SSD_AID,}, 
	{0x0004,  RFU,   0x01, UPGRADE_COSUM_SSD_SCP80KVN,} ,
//	{0x0005,  RFU,   0x01, UPCOS_DEVICETYPE_OFFSET,} ,
//	{0x0006,  RFU,   0x02, UPCOSA_FUNVERSION_OFFSET,} ,
//	{0x0007,  RFU,   0x02, UPCOSA_INCVERSION_OFFSET,} ,
//	{0x0008,  RFU,   0x02, UPCOSB_FUNVERSION_OFFSET,} ,
//	{0x0009,  RFU,   0x02, UPCOSB_INCVERSION_OFFSET,} ,
//	{0x000A,  RFU,   0x01, ACTIVE_COS_OFFSET} ,
	{0x000B,  RFU,   0x01, UPGRADE_EN_MODE} ,
	{0x000C,  RFU,   0x02, UPGRADE_COS_Identity} ,
	{0x000D,  RFU,   0x02, UPGRADE_COS_FUNVERSION} ,
	{0x000E,  RFU,   0x03, UPGRADE_COSUM_SSD_TAR} ,
	{0x000F,  RFU,   0x01, UPGRADE_COSUM_SSD_SCP81KVN} ,
	{0x0010,  RFU,   0x01, UPGRADE_COSUM_SSD_MSL} ,

	/*RFU*/
	{0xFFFF,  RFU,   0x02, 0xFFFF,} 
	
};
#endif

#endif

#if (SUPPORT_COS_UPDATE)&&(!BOOTLOADER_PRO)
/*
*@function   :UPCOSPrivateCommand
*
*@description: 私有指令 E0D2 TAG + P3 
*
*@param      :
*@param      :
*
*@return     :void
*
*@test       :
*
*@author     :lgy
*
*@history    :V1.0 :create @20220412
*/
u16  UPCOSPrivateCommand(PX8 apduBuffer)
{
	#define   MODE_GET     0x80
	#define   MODE_SET     0x00	
	#define   MASK_        0x7F	
	
	u8    X_A    funcType,isStoreData;
	u8    X_A    tagH,tagL,dataLength;
	u8    X_A    i,tagMaxLen;
	u32   X_A    configDataAddress;
	
	if(g_securityState != SECURITY_STATE_MFKEY_VERIFIED)
	{
		return 0x6985;
	}

	funcType    = U8_AND_U8(apduBuffer[APDU_P1_OFFSET],MODE_GET);
	tagH		= U8_AND_U8(apduBuffer[APDU_P1_OFFSET],MASK_);
	tagL		= apduBuffer[APDU_P2_OFFSET];
	dataLength	= apduBuffer[APDU_P3_OFFSET];

	if(MODE_SET == funcType)
	{
		isStoreData		= true;
		apdu_ApduSetIncomingAndReceive();
	}
	else
	{
		isStoreData		= false;
	}
	
	//find Tag config Control information by loop
	for(i = 0; ; U8_INC(i))
	{
		if( UPCos_ConfigTaglist[i].tagName == U16_OR_U16(U16_SHL_U16(tagH, 8), tagL) )
		{
			configDataAddress = U32_A_U16(COSUP_PARA_BASE, UPCos_ConfigTaglist[i].offsetAddr);
			tagMaxLen = UPCos_ConfigTaglist[i].length;
			break;
		}
		if( UPCos_ConfigTaglist[i].tagName == 0xFFFF)
		{
			return BOOT_INS_NOT_SUPPORTED;         //not find
		}
	}
	
	if(dataLength > tagMaxLen)
	{
		return 0x6700;
	}

	if(isStoreData == true)
	{
		nvmCopyAtomic(MEMtPM8(configDataAddress), MEMtPM8(&apduBuffer[APDU_DATA_OFFSET]), dataLength);				
	}
	else
	{
		memCopyToRam(MEMtPX8(&apduBuffer[APDU_DATA_OFFSET]),MEMtPM8(configDataAddress), tagMaxLen);
		apdu_ApduSetOutgoingAndSend(APDU_DATA_OFFSET, tagMaxLen);
	}
	
	return 0x9000;
}
#endif

#if SUPPORT_COS_UPDATE
void valueToBuff(PX8 buff, u32 value, u8 Len)
{
	u8		off = 0;
	if(Len == 4)
	{
		buff[U8_INC(off)] = U32t8(U32_SHR_U8(value, 24));
		buff[U8_INC(off)] = U32t8(U32_SHR_U8(value, 16));
		buff[U8_INC(off)] = U32t8(U32_SHR_U8(value, 8));
		buff[U8_INC(off)] = U32t8(value);
	}
	if(Len == 2)
	{
		buff[U8_INC(off)] = U32t8(U32_SHR_U8(value, 8));
		buff[U8_INC(off)] = U32t8(value);
	}
}
#endif

#if COS_UPDATE_ONLINE_BY_NativeOS

/*
*@function     :UPCOS_GetData
*
*@description  :
*
*@see          :参考《NativeOS在线升级方案设计.doc》
*
*@param        :void
*@param        :void
*
*@return       :拼装好的数据长度61xx
*
*@RAM Resources:RecData
*
*@author       :
*
*@history      :created by lgy@20220510
*/
u16  UPCOS_GetData(void)
{
	u8     X_A     offset,len;
	u16    X_A     tmpValue;
	
	APDUBuffer[APDU_DATA_OFFSET]   =   UPCOS_SCHEME_VERSION>>8;//UPCOS_SCHEME_VERSION
	APDUBuffer[APDU_DATA_OFFSET + 1]   =   (UPCOS_SCHEME_VERSION&0xFF);
	
#if !BOOTLOADER_PRO  //主COS
#if SE_ONLY //获取ICCID
	APDUBuffer[APDU_DATA_OFFSET + 2] = getSeId(&APDUBuffer[APDU_DATA_OFFSET + 3]);
#else //获取EID
	APDUBuffer[APDU_DATA_OFFSET + 2] = nvmReadU8(MEMtPM8(EUICCEIDInfo_Addr));
	ecpMem_Copy(MEMtPN8(&APDUBuffer[APDU_DATA_OFFSET + 3]), MEMtPN8(EUICCEIDInfo_Addr+1), APDUBuffer[APDU_DATA_OFFSET + 2]);//	
#endif
#else  //BOOTLOADER OS
	APDUBuffer[APDU_DATA_OFFSET + 2] = UPCOSConfig_GetCardSEIDLen();
	UPCOSConfig_GetCardSEID(MEMtPN8(&APDUBuffer[APDU_DATA_OFFSET + 3]),APDUBuffer[APDU_DATA_OFFSET + 2]);
#endif
	offset     =    U8_A_U8(APDU_DATA_OFFSET,3);
	offset     =   U8_A_U8(APDUBuffer[APDU_DATA_OFFSET + 2],offset);
	
	APDUBuffer[offset++]    =  0x34;//卡商代码
	tmpValue = UPCOSConfig_GetCardProduct();
	valueToBuff(&APDUBuffer[offset],tmpValue,2);//卡品代码
	offset     =   U8_A_U8(2,offset);
	
	tmpValue = UPCOSConfig_GetUPCOSFunVersion();
	valueToBuff(&APDUBuffer[offset],tmpValue,2);
	offset     =   U8_A_U8(2,offset);
#if !BOOTLOADER_PRO  //主COS
	APDUBuffer[offset++]    =  0x01;//卡片状态 = 1 处于主COS
#else
	APDUBuffer[offset++]    =  0x00;//卡片状态 = 1 处于主BOOTLOADER
#endif
	len = (offset - APDU_DATA_OFFSET);
	
	return (0x6100 + len);
}
#if !BOOTLOADER_PRO   //主COS支持  BOOTLOADER OS不支持
/*
*@function     :UPCOS_SwitchToLoader
*
*@description  :将卡片从主COS状态切到 NativeOS BOOTLOADER，在切换之前将一些应用相关的数据更新到配置区 让NativeOS可以快速访问
*
*@see          :参考《NativeOS在线升级方案设计.doc》
*
*@param        :void
*@param        :void
*
*@return       :0x9000/error
*
*@RAM Resources:RecData
*
*@author       :
*
*@history      :created by lgy@20220510
*/
u16  UPCOS_SwitchToLoader(void)
{
	u8    X_A    backArray[0x100],aid[16];
	u8    X_A    scp80KVN,scp81KVN;
	u32   X_A    countrAddr,sdDataObject,keySetObjcet;
	u8    X_A    sdId,kicLen,kidLen,aidLen,pskLen;
	u16   X_A    keySetRef,result;
	
#define TYPE_KEY_KIC           0x01	 //KIC key type
#define TYPE_KEY_KID           0x02	 //KID key type
	
	if((P1 != 0x00)||(P2 != 0x00))
	{
		return 0x6A86;
	}
	
	aidLen = UPCOSConfig_GetCOSUMSSDAIDLen();
	UPCOSConfig_GetUPCOSUMSSDAIDValue(aid,aidLen);
	sdId = GPGetAppletIDByAppletAID(aid,aidLen);
	
	if(sdId == ILLEGAL_APPID)
	{
		return 0x6982;
	}
	
	ecpMem_Copy(MEMtPN8(backArray), MEMtPN8(COSUP_PARA_BASE), 0x100);
	
	valueToBuff(&backArray[UPGRADE_BIPMODULE_RECEIVE_ADDR],BIPModule_Receive_ADDR_START,4);//BIP收数据的地址
	
	backArray[UPGRADE_COSUM_SSD_APPID]  =  sdId;
	
	scp80KVN = UPCOSConfig_GetUPCOSUMSSDScp80KeyVersion();
	scp81KVN = UPCOSConfig_GetUPCOSUMSSDScp81KeyVersion();
		
	kicLen = keyData_getKeyData(sdId, scp80KVN, TYPE_KEY_KIC, PNtMEM(&backArray[UPGRADE_COSUM_SSD_SCP80KIC]));//查找特定version 的密钥
	kidLen = keyData_getKeyData(sdId, scp80KVN, TYPE_KEY_KID, PNtMEM(&backArray[UPGRADE_COSUM_SSD_SCP80KID]));//查找特定version 的密钥
	
	if((kicLen == 0)||(kidLen == 0))
	{
		return 0x6982;
	}
	sdDataObject		=	(getSdDataObjBySdid(sdId));
	keySetRef			=	keySet_getKeySet(MEMtPN8(sdDataObject), scp80KVN);
	keySetObjcet		=	(locateObjectInMemory(keySetRef));
	//jref getSCP80KeySEQ(u8 sdId, u8 keyVersion, PX8 counterppointer);
	countrAddr = PFtMEM(get_object_field_ref(keySetObjcet, sequenceCounter_INDEX));//PFtMEM(getSeqCounterField(MEMtPN8(keySetObjcet)));
	
	valueToBuff(&backArray[UPGRADE_COSUM_SSD_COUNTR_ADDR],countrAddr,4);
	
	backArray[UPGRADE_COSUM_SSD_SCP80KIC_LEN] = kicLen;
	backArray[UPGRADE_COSUM_SSD_SCP80KID_LEN] = kidLen;
	
	pskLen = keyData_getKeyData(sdId, scp81KVN, 0x01, PMtMEM(&backArray[UPGRADE_COSUM_SSD_PSK]));
	
	if(pskLen == 0)
	{
		return 0x6982;
	}
	
	backArray[UPGRADE_COSUM_SSD_PSK_LEN] = pskLen;
	
#if SE_ONLY //获取ICCID
	backArray[UPGRADE_CARD_SEID_LEN] = getSeId(&backArray[UPGRADE_CARD_SEID]);
#else //获取EID
	backArray[UPGRADE_CARD_SEID_LEN] = nvmReadU8(MEMtPM8(EUICCEIDInfo_Addr));
	ecpMem_Copy(MEMtPN8(&backArray[UPGRADE_CARD_SEID]), MEMtPN8(EUICCEIDInfo_Addr+1), backArray[UPGRADE_CARD_SEID_LEN]);//	
#endif
	
	memCopyAtomic(MEMtPM8(COSUP_PARA_BASE), MEMtPM8(backArray),0x100);
	
	result = switchToEcpLoader(0);
	
	if(result == 0x9000)
	{	
	    InitProactiveCommand(CMD_REFRESH,0x04,DEV_ME);
		SendProactiveCommand();//91XX
	}
	else
	{
		return result;
	}
		
	return 0x9000;
}
#endif
#endif


//__weak int app_main(void)
//{

//	return 0;
//}

//__weak void TIMER0_IRQHandler(void)
//{

//}

//__weak void TIMER1_IRQHandler(void)
//{

//}

//__weak void TIMER2_IRQHandler(void)
//{

//}

//__weak void TIMER3_IRQHandler(void)
//{

//}

//__weak void GPIO_IRQHandler(void)
//{

//}

//__weak void SPI_IRQHandler(void)
//{

//}

//__weak void I2C_IRQHandler(void)
//{

//}

//__weak void UART_IRQHandler(void)
//{

//}

