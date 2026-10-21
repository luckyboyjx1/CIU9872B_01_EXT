#ifndef _CDMA_H_
#define _CDMA_H_ 
#include "typedef.h"
#define LFSR_A LFSR[0]
#define LFSR_B LFSR[1]   
#define LFSR_C LFSR[2]
#define LFSR_D LFSR[3]
#define LOMASK                   0x0F
#define HIMASK                   0xF0
#define AAV                      0xC7
#define IS_LAST                  0xFF

#define  CHECK_MODE 	0x01
#define  OVERRIDE_MODE 	0x02

#define  TAG_PRL_DISPLAYSTR      0xE1
extern BYTE  xdata ESN[4];
extern BYTE  xdata LFSR[4];
extern BYTE  xdata SAVED_LFSR[4];
extern BYTE  xdata SAVED_RAND[4];
extern BYTE  xdata Register[0x10];
extern WORD  xdata SAVED_OFFSET_1;
extern WORD  xdata SAVED_OFFSET_2;
extern BYTE  xdata SSD_A_NEW[8];
extern BYTE  xdata SSD_B_NEW[8];
extern BYTE  xdata SAVED_DATA[3];
extern BYTE  xdata MIN1[3];
extern BYTE  xdata MIN2[2];
extern BYTE  xdata RANDBS[4];												//must store
extern DWORD xdata AuthBs;
extern BYTE  xdata G_EPRL_PROCING;

extern BYTE xdata G_P1;
extern BYTE xdata G_P2[2];

#define B_BSrun 	0
#define B_USrun 	1
#define B_KeyOk 	2


typedef __packed struct headPRLSMSFILE
{  
	BYTE    TotleNum;
    BYTE    StoreNum;                               //  1    0
    WORD    UsedSpace;                           //  2    2
   
}PRLSMSFILEHEAD;

extern BYTE xdata G_CdmaStatus;	

extern BYTE InitCave(void);

extern void UpdateSSD(void);

extern  void ConfirmSSD(void);

extern void BaseStationChallenge(void);

extern void StoreESNMe(void);

extern void GenerateKey(void);

extern void CdmaAuthmetic(void);

extern BYTE UTKPPDownLoad(void) ; 

extern DWORD Auth_Signature(const BYTE RAND_CHALLENGE[4],const BYTE AUTH_DATA[3],const BYTE *SSD_AUTH,const BYTE SAVE_REGISTERS);

extern BYTE TransBitFomat(BYTE *input,BYTE len,BYTE *output);

extern BYTE Decode7Bit(BYTE *in ,BYTE length,BYTE *out);

extern void SendESNInfomation(void);

extern void GSMtoDTMF(BYTE * input, BYTE * output);

extern void Key_VPM_Generation(BYTE * SSD_B,BYTE * cmeakey,BYTE * VPM);

extern void CAVE(const int number_of_rounds, int * offset_1, int * offset_2);

extern BYTE storePRLData(BYTE  * receiveSms,UWORD fileHeadAddr);

extern WORD PRLcrcFromEeprom(UWORD fileHeadAddr,BYTE * resultOut);

extern BYTE WritePRLData(UWORD writeFileAddr,BYTE * userData,WORD fileSize);

extern void UpdateSMS(BYTE * SMSstr,BYTE * UserData,BYTE len,BYTE savebit);

extern void ShowPRLInfo(void);

extern BYTE IsPRLSMS(BYTE *str,WORD pos);

extern BYTE UpdatePLMNList(BYTE * str,BYTE recNo);

extern void ShiftDataPlus(BYTE *input, BYTE len, BYTE *output, BYTE pos);

extern void CDMAAuth(void);

extern BYTE InitCave(void);

extern void ShiftDataUncoil(BYTE *input,BYTE len,BYTE *output,BYTE pos);

extern BYTE GetCDMAUDPos(void);

extern void WritePRLDisplayText(BYTE * UserData);

extern BYTE TransSMSFomat(void);

extern void formatDisplay(BYTE * inputBuffer,BYTE * outBuffer);

extern void RunCDMAAlgorithm(void);

extern void  Run3GAccessAKA(void);

extern void CDMADataAvaiable(void);

extern void SSD_Generation(const BYTE * RANDSSD, const BYTE * A_key);

extern WORD PRLcrc(WORD iniData, BYTE *ptr, WORD count);

extern BYTE cdmaAsciiToDTMF(BYTE input);

extern BYTE CdmaTpoa2GsmTpoa(BYTE offset, BYTE *tpoabuf);

#endif
