#ifndef    _BIPMODULEPROCESS_H_
#define    _BIPMODULEPROCESS_H_
#include 	"typedef.h"

/*************************macro define************************************/
#define NULL_ChannelId    0x00    //空的 channelId，即当前无通道打开

/******以下 列出卡片基于BIP 的业务类型 ，所有卡片上的应用的BIP使用同一个缓存文件收取bip数据，以下 业务类型要给对应的C应用程序去处理，其他的则按照以前的业务逻辑继续处理×**/
#define ChannelDataType_NULL    0x00   //表示当前
#define ChannelDataType_HTTPS   0x01
#define ChannelDataType_CATTP   0x02
#define ChannelDataType_OTA     0x03
#define ChannelDataType_DNS     0x04
#define ChannelDataType_RAS     0x05


#define   BIPModule_DataAvailable            					0x09
#define   BIPModule_Channelstatus	        					0x0A



/*************************variable define**********************************/
typedef __packed struct tagBIPModuleStatus
{
	BYTE channelNum;     //当前打开的通道号1-7:有效，00无通道打开
	BYTE channelDataType; //当前打开通道的待处理报文类型 OTA or Http or CATTP
	WORD channelBufferSize;	//bip通道的缓存大小, 由于卡端接收数据是动态接收，该值对卡端没影响，但是终端如何实现不知道，所以此处卡端还是需要严格按照规范实现，一次发送数据的大小不能大于BufferSize, -jiangqingbin-20221206
//	BYTE channelDataLength;  //当前dataavaliable 待接收的BIP数据长度 D6事件用打标记的方式处理，将其指示的待接收数据做为标记的第2字节保留在标记文件中，处理BIPModule_DataAvaiable_Alert标记时再取出来
}BIPModuleStatus;

extern BIPModuleStatus xdata G_BIPModuleStatus;  
extern WORD xdata G_BIPModule_Receive_Length;
extern WORD xdata G_BIPModule_Send_Length;
extern BYTE xdata G_BIPDataAvaiableEvent;     //
#if GP__AMENDMENT__B__SUPPORT_DATA_IN_RAM
extern BYTE xdata G_BIPModule_Session_Data_Buf[0x1D10];
#endif
/*************************function define**********************************/
extern void BIPModuleInit(void);
extern void BIPModuleProcessDataAvaiable(BYTE len);
extern void BIPModuleProcessStatus(void);
extern BYTE BIPModuleOpenChannel(BYTE appType);
extern BYTE BIPModuleSendResponseData(BYTE mode, BYTE* dataBuf);
//extern BYTE BIPModuleSendHttpHandshake(void);
extern BYTE BIPModuleChannelStatus(BYTE cmdLeng, BYTE *cmdBuf);
extern BYTE BIPModuleDataAvaiable(BYTE cmdLeng, BYTE *cmdBuf);
extern BYTE BIPModuleIsProcess(void);
extern void IF_BipChannelStatus(void);

#endif

