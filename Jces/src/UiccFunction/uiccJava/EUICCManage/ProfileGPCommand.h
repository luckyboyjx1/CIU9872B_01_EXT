#ifndef    _PROFILEGPCOMMAND_H_
#define    _PROFILEGPCOMMAND_H_
#include   "typedef.h"

typedef __packed struct 
{
	BYTE peInstallAppId;   //登记当前PE正在安装的应用的APPid
	BYTE peExtraditeSDID;  //登记当前PE正在安装的应用的extraditeSecurityDomainAID
	BYTE peLifeCycle;      //登记当前PE正在安装的应用的lifeCycleState
	BYTE keyCounter[6];    //登记当前PE正在执行的Key Personalisation中key counter的LV，至于该值是否与对应的key type对应，到具体的putkey指令中再进行判断
}profileGPState;

extern profileGPState xdata G_profileGPState;

/**********************FUNCTION DEFINE****************************************/
extern void ContactlessProtocolParameterInit(void);
extern void PEGPGlobalVariableInit(void);
extern BYTE PEUpdateOpenPersoData(BYTE *contactlessProtocolParameters);
extern void PEUpdateCATTPPersonalisation(WORD catTpMaxSduSize, WORD catTpMaxPduSize);
extern BYTE PEMNOSDIsCreated(void);

extern WORD PEExcuteGPCommand(BYTE excuteSdId);
extern WORD CreateProfileEntry(void);
#endif
