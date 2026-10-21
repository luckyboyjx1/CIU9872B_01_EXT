#ifndef THD89FUNCTION_H_
#define THD89FUNCTION_H_
//#include "optspace.h"
#include "ecpDefine.h"
#include "RNG_API.h"
#include "chipHead.h"
#include "stdint.h"

typedef struct{
	unsigned char* p;
	unsigned short length;
}BigNum_t;


#define SupportFiTypeNum  3
#define SupportDiTypeNum  9
#define FWINUM   15
#define BWINUM   10

void RFIdleMode(unsigned char RFclk);
void clkInit(unsigned char CTclk, unsigned char minFS);
void erasePageWithCheck(PN8 eew_Des);
void ecpNvm_FlashBackUp_RollBackPage(void);
u8 ProgPageWithCheck(PN8 ram_Src, PN8 eew_Des);
u8 ProgPartWithCheck(PN8 ram_Src, PN8 eew_Des, u16 eew_length);
u32 nvmLowPowerConfigBegin(void);
void nvmLowPowerConfigEnd(u32 config);
//__asm unsigned long disableIRQ(void);
//__asm void restoreIRQ(unsigned long backup);


extern void chipSM2GenKeyPair(PM8 privateKey, PX8 tempPublicKey);
extern void chipSM2GetKey(PX8 prikey, PX8 publickey, u8 isgenkeypair);
extern void chipSM2Sign(PX8 E_Buff, PX8 privteKeyBuffer, PM8 rs);
extern u8 chipSM2Verify(PM8 E, PM8 publicKey, PM8 rs) SMALL_MODE;
extern void chipSM2Encrypt(PX8 inBuff, u16 inLen, PM8 publickeyBuff, PM8 outBuff);
extern u8 chipSM2Decrypt(PM8 inBuff, u16 inLen, PM8 privatekeybuff, PM8 outBuff);
extern void ecpAModB(PX8 APoint, PX8 BPoint, PX8 CPoint, u16 byteLen);

#define NUM_OF_WORDS_IN_256BIT_FRAME (256 / (sizeof( uint16_t ) * 8))
extern u8 currRNGMode; 
extern u32 TRNGLocalCacheBuffer[NUM_OF_WORDS_IN_256BIT_FRAME<<2];
extern TRNG_Context TRNGLocalCache;

extern void Rng_Init();
extern u32 Rng_GetRandomWord( u32 * pReadOutWordInv );
extern u16 Trng_GetRandomWord(TRNG_Context * TRNGCache);
extern void delayNclock(unsigned int etu_num);
	
#define TMC_NVMCON_HVTYP 		0x30000000
#define TMC_NVMCON_RETRY 		0x00300000
#define TMC_NVMCON_PVTYP 		0x00008000
#define TMC_NVMCON_PVEN 		0x00004000
#define TMC_NVMCON_PTYP 		0x00000F00
#define TMC_NVMCON_EVTYP 		0x00000080
#define TMC_NVMCON_EVEN 		0x00000040
#define TMC_NVMCON_EREN 		0x00000010
#define TMC_NVMCON_ETYP 		0x0000000F

#define THD89_PKESTS_HALTED			0x04
#define THD89_PKECON2_CONTINUE		0x02


//NVM buffer write
#define PROG_BUF_WORD_WRITE			0x00010000
#define ERASE_BUF_WORD_WRITE		0x00030000

//NVM erase
#define SECTOR_ERASE				0x00000005
#define EARSE_MAX_TIMES				4

//NVM program
#define PAGEROW_WORD_PROG			0x00000400



#define sysConfig_GetEGTandRetransmitBase()                         U32_A_U32(COS_CONFIG_START,THD89_EGT_RETRANSMIT_COUNTER)
#define sysConfig_GetEGTandRetransmit()                             PN_R_U8(MEMtPN8(sysConfig_GetEGTandRetransmitBase()))
#define sysConfig_SetEGTandRetransmit(value)                        nvmWriteU8Atomic(MEMtPN8(sysConfig_GetEGTandRetransmitBase()),value)

#define sysConfig_GetErrorSignalBase()                              U32_A_U32(COS_CONFIG_START,THD89_ERROR_SIGNAL)
#define sysConfig_GetErrorSignal()                                  PN_R_U8(MEMtPN8(sysConfig_GetErrorSignalBase()))
#define sysConfig_SetErrorSignal(value)                             nvmWriteU8Atomic(MEMtPN8(sysConfig_GetErrorSignalBase()),value)

#define sysConfig_GetFDTBase()                      				U32_A_U32(COS_CONFIG_START,THD89_FDT_SET)
#define sysConfig_GetFDT()                            				PN_R_U8(MEMtPN8(sysConfig_GetFDTBase()))
#define sysConfig_SetFDT(value)                       				nvmWriteU8Atomic(MEMtPN8(sysConfig_GetFDTBase()),value)

#define sysConfig_GetMifareSelectBase()                      		U32_A_U32(COS_CONFIG_START,THD89_MIFARE_SELECT)
#define sysConfig_GetMifareSelect()                            		PN_R_U8(MEMtPN8(sysConfig_GetMifareSelectBase()))
#define sysConfig_SetMifareSelect(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMifareSelectBase()),value)

#define sysConfig_GetMifareAtqaLBase()                      		U32_A_U32(COS_CONFIG_START,THD89_MIFARE_ATQAL)
#define sysConfig_GetMifareAtqaL()                            		PN_R_U8(MEMtPN8(sysConfig_GetMifareAtqaLBase()))
#define sysConfig_SetMifareAtqaL(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMifareAtqaL()),value)

#define sysConfig_GetMifareAtqaHBase()                      		U32_A_U32(COS_CONFIG_START,THD89_MIFARE_ATQAH)
#define sysConfig_GetMifareAtqaH()                            		PN_R_U8(MEMtPN8(sysConfig_GetMifareAtqaHBase()))
#define sysConfig_SetMifareAtqaH(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMifareAtqaH()),value)

#define sysConfig_GetMifareSAKBase()                      			U32_A_U32(COS_CONFIG_START,THD89_MIFARE_SAK)
#define sysConfig_GetMifareSAK()                            		PN_R_U8(MEMtPN8(sysConfig_GetMifareSAKBase()))
#define sysConfig_SetMifareSAK(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMifareSAK()),value)

#define sysConfig_GetMifareInSAKBase()                      		U32_A_U32(COS_CONFIG_START,THD89_MIFARE_INSAK)
#define sysConfig_GetMifareInSAK()                            		PN_R_U8(MEMtPN8(sysConfig_GetMifareInSAKBase()))
#define sysConfig_SetMifareInSAK(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMifareInSAK()),value)

#define sysConfig_GetL3CONFIGATQALBase()                      		U32_A_U32(COS_CONFIG_START,THD89_L3CONFIG_ATQAL)
#define sysConfig_GetL3CONFIGATQAL()                            	PN_R_U8(MEMtPN8(sysConfig_GetL3CONFIGATQALBase()))
#define sysConfig_SetL3CONFIGATQAL(value)                       	nvmWriteU8Atomic(MEMtPN8(sysConfig_GetL3CONFIGATQAL()),value)

#define sysConfig_GetL3CONFIGATQAHBase()                      		U32_A_U32(COS_CONFIG_START,THD89_L3CONFIG_ATQAH)
#define sysConfig_GetL3CONFIGATQAH()                            	PN_R_U8(MEMtPN8(sysConfig_GetL3CONFIGATQAHBase()))
#define sysConfig_SetL3CONFIGATQAH(value)                       	nvmWriteU8Atomic(MEMtPN8(sysConfig_GetL3CONFIGATQAH()),value)

#define sysConfig_GetL3CONFIGSAKBase()                      		U32_A_U32(COS_CONFIG_START,THD89_L3CONFIG_SAK)
#define sysConfig_GetL3CONFIGSAK()                            		PN_R_U8(MEMtPN8(sysConfig_GetL3CONFIGSAKBase()))
#define sysConfig_SetL3CONFIGSAK(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetL3CONFIGSAK()),value)

#define sysConfig_GetL3CONFIGINSAKBase()                      		U32_A_U32(COS_CONFIG_START,THD89_L3CONFIG_INSAK)
#define sysConfig_GetL3CONFIGINSAK()                            	PN_R_U8(MEMtPN8(sysConfig_GetL3CONFIGINSAKBase()))
#define sysConfig_SetL3CONFIGINSAK(value)                       	nvmWriteU8Atomic(MEMtPN8(sysConfig_GetL3CONFIGINSAK()),value)

#define sysConfig_GetMifareADDRBase()                      			U32_A_U32(COS_CONFIG_START,THD89_MIFARE_ADDR)
#define sysConfig_GetMifareADDR()                            		PN_R_U32(MEMtPN32(sysConfig_GetMifareADDRBase()))
#define sysConfig_SetMifareADDR(value)                       		nvmWriteU32Atomic(MEMtPN8(sysConfig_GetMifareADDR()),value)

#define sysConfig_GetSecuritySetBase()                      		U32_A_U32(COS_CONFIG_START,THD89_SECURITY_SET)
#define sysConfig_GetSecuritySet()                            		PN_R_U32(MEMtPN32(sysConfig_GetSecuritySetBase()))
#define sysConfig_SetSecuritySet(value)                       		nvmWriteU32Atomic(MEMtPN8(sysConfig_GetSecuritySet()),value)

#define sysConfig_GetT1TimerConifgBase()                      		U32_A_U32(COS_CONFIG_START,THD89_T1_TIMERCONFIGVALUE)
#define sysConfig_GetT1TimerConifg()                            	PN_R_U32(MEMtPN32(sysConfig_GetT1TimerConifgBase()))
#define sysConfig_SetT1TimerConifg(value)                       	nvmWriteU32Atomic(MEMtPN8(sysConfig_GetT1TimerConifg()),value)

#define sysConfig_GetT0TimerConifgBase()                      		U32_A_U32(COS_CONFIG_START,THD89_T0_TIMERCONFIGVALUE)
#define sysConfig_GetT0TimerConifg()                            	PN_R_U32(MEMtPN32(sysConfig_GetT0TimerConifgBase()))
#define sysConfig_SetT0TimerConifg(value)                       	nvmWriteU32Atomic(MEMtPN8(sysConfig_GetT0TimerConifg()),value)

#define sysConfig_GetTATimerConifgBase()                      		U32_A_U32(COS_CONFIG_START,THD89_TA_TIMERCONFIGVALUE)
#define sysConfig_GetTATimerConifg()                            	PN_R_U32(MEMtPN32(sysConfig_GetTATimerConifgBase()))
#define sysConfig_SetTATimerConifg(value)                       	nvmWriteU32Atomic(MEMtPN8(sysConfig_GetTATimerConifg()),value)

#define sysConfig_GetGetATRwaitBase()                      			U32_A_U32(COS_CONFIG_START,THD89_ATRWAIT)
#define sysConfig_GetATRwait()                            	     	PN_R_U8(MEMtPN8(sysConfig_GetGetATRwaitBase()))
#define sysConfig_SetGetATRwait(value)                       		nvmWriteU8Atomic(MEMtPN8(sysConfig_GetATRwait()),value)

#define sysConfig_GetMifareBLOCK0DATABase()                      	U32_A_U32(COS_CONFIG_START,THD89_MIFARE_BLOCK0DATA)

#define sysConfig_GetMifareBLOCK0FLAGBase()                      	U32_A_U32(COS_CONFIG_START,THD89_MIFARE_BLCOK0FLAG)
#define sysConfig_GetMifareBLOCK0FLAG()                            	PN_R_U8(MEMtPN32(sysConfig_GetMifareBLOCK0FLAGBase()))
#define sysConfig_SetMifareBLOCK0FLAG(value)                       	nvmWriteU8Atomic(MEMtPN8(sysConfig_GetMifareBLOCK0FLAG()),value)


#define B2L(x) (*(x+3)*0x1000000 + *(x+2)*0x10000 + *(x+1)*0x100 + *(x))
#endif /* THD89FUNCTION_H_ */
