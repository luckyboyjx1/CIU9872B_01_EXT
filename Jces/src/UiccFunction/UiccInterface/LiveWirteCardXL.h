#ifndef __REMOTECARDNEW_H_
#define __REMOTECARDNEW_H_
#include "ecpDefine.h"

#define  D_LiveWriteCardTag		 0xE2
#define  D_GetCardInfoTag	     0xE3

#define  OTA_GetCardInfo_Tag	 0xF1
#define  OTA_LiveWriteCard_Tag 0xF2

#define  EF_CARDKEY		0x3F01

#define	Result_Success			0x30
#define	Result_Incomplete   0x31
#define	Result_DecryptErr   0x32
#define	Result_WrongTag     0x33
#define	Result_DataErr      0x40
#define	Result_WriteErr     0x50
#define	Result_MacErr       0xD0

#define Status_Check		0xD1
#define Status_Write		0xD2

#define EF_WriteCardConfig	0x8F00

#define  var_MacKey        0x02
#define  var_PIN1			0x03
#define  var_PIN2			0x04
#define  var_PUK1			0x05
#define  var_PUK2			0x06
#define  var_sendCode	0x07

typedef struct 
{
    u8 tag;

    u8 (*func)(void);

}TAGLIST;

typedef struct 
{
    u8 tag;

    u8 length;

}CHECKLIST;

extern u8 X_A G_NowNumber;
//extern u8 X_A G_setNum;
//extern u8 X_A G_WriteCardStatus;
extern u8 X_A G_LIVEWRITECARDTYPE;
extern u8 X_A G_SendControl;

extern u8 IsEmptyCard(void);
extern u8 FindEmptyNumber(void);
extern void LiveGetCardInfoHandler(u8 ptr);
extern void LiveGetCardInfoExcute(void);
extern void LiveWriteCardHandler(u8 reference, u8 headNo, u8 smsType,  u8 *tpoa);
extern void ClearLociSqn(void);

//extern u8 X_A G_NowNumber;
//extern u8 X_A G_setNum;
//extern u8 X_A G_WriteCardStatus;
//extern u8 X_A G_SendControl ;
// extern u8 CONST WriteError[];
// extern u8 LiveWriteCardSendSMS(u8 result);
//extern u8    X_A     G_LIVEWRITECARDTYPE;//保存当前下发数据的命令类型，是两不一快0x0c,还是现场写卡0X0B

//----------------OTARfm外部声明--------------------------
extern void ClearRFMTmpPackage(memref fileAddr, u8 pageNum);

extern u8 IsRFMReceiveAll(memref fileAddr, u8 packageNum);

#endif
