#ifndef __IO_SWP_H__
#define	__IO_SWP_H__

#include "ecpDefine.h"
#include "ecpReadWrite.h"
#include "SysConfigProfile.h"

#if BOOT__IO__SWP


/************* following functions are HCI API , used for native methods **************/
extern  void getReceiveBufferHCI(PM8 bufAddr);
extern  void SWP_HCI_Interface_TransactionEvent_6paras(PM8 parametersAddr,u16 parametersOffset,u16 parametersLen,PM8 aidAddr,u16 aidOffset,u16 aidLen);
extern  void SWP_HCI_Interface_TransactionEvent_3paras(PM8 parametersAddr, u16 parametersOffset, u16 parametersLen);
extern  void sendSendDataEventHCI(PM8 dataAddr,u16 dataOffset,u16 dataLen);
extern  void SendWriteXchgDataCommand(PM8 dataAddr,u16 offset,u16 len,u8 timeout);
extern  u8   getParaCommandHCI(u16 paraId);
extern  u8   isHciServiceAvaliable(u16 paraId);
extern  u8   isHeaderHCI(void);
extern  u8   isCompleteHCI(void);
extern  u8   getInstructionTypeHCI(void);
extern  u8   getInstructionHCI(void);
extern  u8   getDataLength(void);
extern  u8   HCI_getPowerMode(void);
extern  u8   getRFTypeHCI(void);
extern  u8   getListenerEvent(void);
extern  u8   connectivity_prepareAndSendCnnEvent(void);


extern  u8   swp_HciNoParaProcess(u8 tType);
extern  u8   swp_HciOneParaProcess(u8 tType,u16 pId);
extern  void swp_HciMulParaProcess(u8 tType,u8 time,u16 length,u16 off,PM8 dataddr);
extern  void swp_GetReceiveBufferHCI(PM8 address);
extern  void swp_Swp_Hci_TransactionEvent_6paras(PM8 parAddr,u16 parOff,u16 parLen,PM8 aidAddress,u16 aidOffset,u16 aidLength);
extern  void swp_Swp_Hci_TransactionEvent_3paras(PM8 parAddr,u16 parOffset,u16 parLen);
	
#else

#define  getReceiveBufferHCI()
#define  SWP_HCI_Interface_TransactionEvent_6paras()
#define  SWP_HCI_Interface_TransactionEvent_3paras()
#define  sendSendDataEventHCI()
#define  SendWriteXchgDataCommand()
#define  getParaCommandHCI()                   0
#define  isHciServiceAvaliable()                0
#define  isHeaderHCI()                          0
#define  isCompleteHCI()                          0
#define  getInstructionTypeHCI()                    0
#define  getInstructionHCI()                    0
#define  getDataLength()                     0
#define  HCI_getPowerMode()                  0
#define  getRFTypeHCI()                    0
#define  getListenerEvent() 0
#define  connectivity_prepareAndSendCnnEvent() 0
#endif


#endif

