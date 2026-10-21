#ifndef SCP81CONNECTION_H
#define SCP81CONNECTION_H
#include "typedef.h"

typedef __packed struct scp81Connection
{
	//WORD TriggeredAppRef;						//触发scp81会话的app索引
	WORD SCP81ConnectionStatusEventRef;			//链接事件的对象索引
	WORD DataReceivedEventRef;					//接收数据的对象索引
	WORD scp81ConnectionStatus;					//链接状态事件，是收到数据事件还是其他事件
	WORD scp81CopyDataRemainLen;				//标记还有多长的数据要传输给app
} SCP81Connection;         						

extern SCP81Connection xdata G_SCP81Connection;


//***********scp81connection 接口对应的函数************************//
extern void NM_scp81SetConnectionObserver(void);
extern void NM_scp81ConnectionOpen(void);
extern void NM_scp81ConnectionSend(void);
extern void NM_scp81ConnectionClose(void);
extern void NM_scp81getCurrentEventStatus(void);
extern void NM_scp81BakToolkitEvent(void);
extern void NM_scp81RecoverToolkitEvent(void);
extern void NM_scp81ConnectionStatusGetDetails(void);
extern void NM_scp81CopyReceivedData(void);
extern void NM_scp81DiscardRemainingData(void);
extern void NM_scp81GetReceivedDataLength(void);
extern void NM_scp81contentTypeIsCorrect(void);
extern void NM_scp81responseIsComplete(void);
extern void Scp81ConnectionReport(WORD SCP81Status);

#endif
