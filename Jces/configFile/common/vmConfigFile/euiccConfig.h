#ifndef __EUICC_CONFIG_H_
#define __EUICC_CONFIG_H_

#include "JcvmConfig.h"
#include "SysConfigTarget.h"

//extern void Image$$ER_LOGICCHN_BAK_AREA$$Base;

#define BAK_PROFILEINDEX		0xFE

// modify by xieyong@20220107 增加一卡一CRC功能
/*if HAL__PLATFORM__SIMULATOR
#define LOGICCHNBAK_BASEADDR	0x0011B200 //修改成与.sct文件中的LR_LOGICCHN_BAK_AREA一致，否则 uicc_configuration_additional_test_工程集1\GP补充测试\reEntry_logic_channel_Case4return6100_Test_01  恢复卡片时执行删除会死卡,原因是该值被定义在了用户数据空间的内部
#else
#define LOGICCHNBAK_BASEADDR	(u32)&Image$$ER_LOGICCHN_BAK_AREA$$Base   //0x0011B200 //修改成与.sct文件中的LR_LOGICCHN_BAK_AREA一致，否则 uicc_configuration_additional_test_工程集1\GP补充测试\reEntry_logic_channel_Case4return6100_Test_01  恢复卡片时执行删除会死卡,原因是该值被定义在了用户数据空间的内部
#endif
*/
//以下代码最初是 李宇桐 (epx2.0-20200521)上传的， 最初定义就是EEPAGE_SIZE， 后面的三者相加不清楚为何是注释状态。   目前发现问题是当EEPAGE_SIZE时0x100时，其大小空间小于了(LOGICCHNBAK_DATASIZE + LOGICCHNBAK_RESSTSIZE + LOGICCHNBAK_CRLSTSIZE)， 导致备份数据时有数据被覆盖掉。 -jiangqingbin -20220829
#define	LOGICCHNBAK_BLOCKSIZE	0x200//EEPAGE_SIZE//(LOGICCHNBAK_DATASIZE + LOGICCHNBAK_RESSTSIZE + LOGICCHNBAK_CRLSTSIZE)

#define LOGICCHNBAK_DATASIZE	(256+2)
#define LOGICCHNBAK_RESSTSIZE	sizeof(pIso7816Util_Response_t)
#define LOGICCHNBAK_CRLSTSIZE	sizeof(iso7816T0_Control_t)

#define LOGICCHNBAK_RESSTOFFSET	(0 + LOGICCHNBAK_DATASIZE)
#define LOGICCHNBAK_CRLSTOFFSET	(LOGICCHNBAK_RESSTOFFSET + LOGICCHNBAK_RESSTSIZE)

#define LOGICCHANNELERROR		0xFF
#define LOGICCHANNELE_EXT_BOUND 0x03

#define	BAK_RESST				0
#define BAK_DATA				1
#define BAK_CRLST				2
#define BAK_FLAG				3

#define REC_FLAG				0X5A




extern void Euicc_ConfigDataReset(u16 initialRtp);
extern void Euicc_BakJCRE(void);
extern u8	Euicc_getCurExecProfile(void);

extern u16 JcvmConfig_GetdefaultAppID(void);
extern u32 JcvmConfig_GetCardStateBase(void);
extern u8 JcvmConfig_GetCardState(void);
void profile_gc(u8 deleteProfileIndex,u16 entryIndex);
#if JAVA_CARD_UICC
extern void bakLogicalChninfo(void);
extern u8 recLogicalChninfo(void);
extern void Clearresponse(void);
extern void b616crecflag_init(void);
#endif
extern u16 JcvmConfig_SetSEState(u8* buffer);


extern u8 JcvmConfig_GetSEState(void);

extern void IFBakChannelInfo(void);
extern u8 markProfileObject(u16 objectId);
extern u8 markProfileObjects(u8 delete_profile);
extern void markDeleteProfileNonRootObjects(u8 deleteProfileIndex);
extern void profilePESweep(u8 deleteProfileIndex ,jref deleteProfileEntryRef);

#endif
