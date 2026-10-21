#ifndef _TIMERMODULEPROCES_H_
#define _TIMERMODULEPROCES_H_
#include "typedef.h"
#include "profile.h"

/*************************macro define ***********************************/

#define Start_Timer		        0x00
#define Close_Timer		        0x01


//TS102223 8.37 Timer identifier 
#define TimerMinNum		        0x01
#define TimerMaxNum		        0x08

#define TimerType_NotUsed           0xFF
#define TimerType_Used              0x01
#define TimerType_CardResetClear    0x00   //该类型的timer，在卡片复位时，自动释放
//01-FE用于表示在toolkitAPP所占用的timerId ，由应用自己释放或者删除的应用的时候释放，该字段存储的是6F80的记录号，因此6F80最大支持255条记录


#define   TimerModule_TimerExpiration_Alert    0x42
/***************global define ********************************************/
extern BYTE xdata G_EventTimerExpirationId;

#if TIMERMODULE_TIMERID_IN_RAM
extern BYTE xdata G_UsedTimerId;
#endif

//extern BYTE xdata G_CurrentTimerId;          //当前的timerId
/***************************file define************************************************/
#define DF_Timer                    0xA500
#if TIMERMODULE_TIMERID_IN_FILE
#define EF_TimerId                  0x5A01   //TF文件， 8字节长，若对应的字节不为ff，则意味着该字节所指示 的 timerId已被占用 ，profile私有模板文件，每个profile一个
#endif
#define EF_Timer_SMSSendNotice   	0x5A02 	//可选文件，用于M2M版本发送通知时的重发间隔即重发次数

/******************function define******************************************************/
#define TimerModuleTimerIdInValid(timerID) ((timerID < TimerMinNum) || (timerID > TimerMaxNum))
extern void TimerModuleInit(void);
extern BYTE TimerModuleAllocTimerId(void);
extern BYTE TimerModuleReaseTimerId(BYTE* timerId);
extern BYTE TimerManagementProactive(BYTE commQualify,BYTE timerTd, BYTE * timerValue);
extern BYTE TimerModuleDeactivateTimer(BYTE timerId);
extern BYTE TimerModuleProcessTimerExpiration(void);
extern BYTE TimerModuleStartTimer(BYTE timerId, BYTE * timerValue);



#endif

