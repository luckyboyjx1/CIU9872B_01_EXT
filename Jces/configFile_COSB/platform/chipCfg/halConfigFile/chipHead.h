/*
 * chipHead.h
 *
 *  Created on: 2014年9月29日
 *      Author: jiangxiaohui
 *  the head is user for add CMOS chip's own lib head, register head, or other
 */

#ifndef CHIPHEAD_H_
#define CHIPHEAD_H_

/*
 * chip header
 *
 */
#include "IFX_Framework.h"
#include "IFX_DbC.h"

#ifdef SLE70
#include "intrinsic_sle70.h"
#endif

#include "IFX_Imm.h"
#include "IFX_Interrupts.h"

#include "IFX_Clk_Pwr.h"
//#include "IFX_Swp.h"
//#include "IFX_Swp_Reg.h"
//#include "IFX_Timer.h"

//#ifdef SLE90
////#include "fw_sle90.h"
//#include "IFX_Bos.h"
//#endif

//#include "IFX_Trng.h"
//#include "IFX_Uart_Reg.h"
//#include "IFX_Timer.h"
//#include "IFX_Memory.h"
//#include "IFX_Nvm.h"

//#include "IFX_Scp.h"
#include "IFX_StdInt90.h"
#include <ACL37-Crypto2304T-C65-Types.h>
#include <ACL37-Crypto2304T-C65-Base.h>
#include <ACL37-Crypto2304T-C65-Rsa4k.h>
#include <ACL37-Crypto2304T-C65-Toolbox.h>
#include <ACL37-Crypto2304T-C65-Ecc.h>
#include "curve25519_dh.h"

#include "UartHal.h"
#include "Alg_SM.H"
extern  void EraseRAM( PX32 Begin, u32 CountLoop );

// erase (CountRest * 4 bytes) .
extern  void FineEraseRAM( PX32 Begin, u32 CountRest );


extern STATUS HARDWEAR_E2P_Fill(PN8 desAddr, UINT8 value, UINT32 Size);

typedef struct EXCEPTION_FLAG
{
    volatile INT16 ExceptionAllowed;    ///< signals to exception handle that the next exception is expected , initialized with  -1 - no exception allowed
    volatile INT16 ExceptionOccurred;   ///< signals to caller which fault occurred during executionhandle , initialized with  -1 - no exception occurred
} EXCEPTION_FLAG; 

extern EXCEPTION_FLAG ExceptionFlag; 
//#include "System_Section.h"
//#include "FunEntryConfig.h"

//#include "sfr.h"
//#include "halinter.h"
//#include "ecpDefine.h"
//#include "halinter.h"
//#include "core_cm0.h"
//#include "THD89Function.h"
//#include "nvmhal.h"
//#include "cryptolib.h"
//#include "CryptoSULib.h"
//#include "SystemClockLib.h"
//#include "SecurityUtils.h"
//#include "cryptoSM4Lib.h"
//#include "NvmLib.h"
//#include "cryptolib.h"
//#include "CryptoSULib.h"
//#include "CryptoSM7Lib.h"
//#include "SecurityUtils.h"
//#include "optspace.h"
//#include "THD89CoreFunction.h"
//#include "CryptoECCPtDeCompress.h"

#include "optspace.h"
//#include "THD89CoreFunction.h"
#include "NvmMap.h"

//#include "Alg_SM.h"
//#include "euiccconfig.h"
#define CL_REREST_FLAG       0xA5
#define CT_REREST_FLAG       0xA3
#define SPI_REREST_FLAG      0xA9
#define VM_REREST_FLAG       0xAB
#define I2C_REREST_FLAG      0xAD

//#define GW_LIB				0
//COS状态下，修改BL的MFkey、ENKEY、MODE
#define 	BOOTLOADER_CONFIG_BASE		0x100F3000
#define 	BLMFKEY_BASE_ADDR_OFFSET  	0x0000
#define 	BLENKEY_BASE_ADDR_OFFSET 	0x0200
#define   	BLMODE_BASE_ADDR_OFFSET 	0x0210
#define   	BLSPIMODE_BASE_ADDR_OFFSET  0x0211
#define     BLVERSION_BASE_ADDR_OFFSET  0x0212

//#define GW_LIB_TEST	
#define _7816_WORK_FLAG (1<<0)
#define _SPI_WORK_FLAG	(1<<1)
#define _I2C_WORK_FLAG	(1<<2)

//extern void rtnBackup(u8 resetStatus);
//extern void BackupPC(void);
//extern void _HalRamCopy(u8 *pbDest, u8 *pbSrc, u16 wLen);
//extern void GetInterfaceStatus(void);
//extern void IdleMode(void);
//extern u8 CTinit(void);
//extern INTERFACESTATUS interfacestatus;
//extern u8 COMBUF[384];
//extern u8 IsColdResetFlag;	
//extern void initM1DefaultData(u8 mode, u32 ulAddr, u16 *space);
//extern u8 cfgCrypto1Init(u8 mode,u32 ulNVMAddr);
//extern u8 M1LibSak;
//extern u8 M1LibInsak;
//extern u8 M1LibATQA_b9_b16;
//extern u8 M1LibATQA_b1_b8;
//extern PX8 M1Libuid;
//extern u32 E2StartAddr;
//extern u8 MifareFlagFlag;
//extern u8 RATS_BUF[1];
//extern u8 CreatMifareFlag;
//extern u8 T1_ERRLEN_COUNT;
//extern void GetChipUID(PX8 uid,u8 uidlevel);
//extern memref dealMAPAddress(MapEntry_t  mapEntry, u16 index);
//extern u8 T0_GetEventNeedHandle(void);

//extern u16 bkval;
//extern u32 timernum;
//extern u8 timer0div;
//extern u32 timerdiv;
//extern u8 DeselectFlag;
//extern u8 ProtocolWorkFlag;
//#define     OTP_SWITCH        0x14000200     //      OTP switch address	

//#define SYM_SM7_ENC				0
//#define SYM_SM7_DEC				1
#define SYM_SM4_ENC				0
#define SYM_SM4_DEC				1
//#define SYM_SM1_ENC				0
//#define SYM_SM1_DEC				1

//#define ALG_OK 					0xAAAA
//#define ALG_ERROR 				0x5555

#define ALG_SYM_OK 				0xAA
//#define ALG_SYM_ERROR 			0x55

//#define ALG_RNG_OK				0xAA

#define SYM_MK_SIZE				8
#define SYM_TK_SIZE				8

//// Register Define area
//#define OPENALL8_1   0xff
//#define OPENALL16_1  0xffff
//#define OPENALL32_1  0xffffff
//#define OPENALL8_0   0x00
//#define OPENALL16_0  0x00
//#define OPENALL32_0  0x00
//#define CLOSEALL8_1  0xff
//#define CLOSEALL16_1 0xffff
//#define CLOSEALL32_1 0xffffff
//#define CLOSEALL8_0  0x00
//#define CLOSEALL16_0 0x00
//#define CLOSEALL32_0 0x00



////IDLECON0
//#define THD89_ISO7816SLEEP_EN  0x01
//#define THD89_SPISLEEP_EN      0x04
//#define THD89_GPIOSLEEP_EN     0x08
//#define THD89_RFSLEEP_EN       0x20

////CLKIOCON
//#define THD89_ISO7816_CLK_EN  0x01	
//#define THD89_SPI_CLK_EN      0x04
//#define THD89_GPIO_CLK_EN     0x08
//#define THD89_RF_CLK_EN       0x20

////INTIOCON
//#define THD89_ISO7816_INT_EN 0x01
//#define THD89_SPI_INT_EN 0x04
//#define THD89_GPIO_INT_EN 0x08
//#define THD89_RF_INT_EN 0x20

////IOPULLSEL0
//#define THD89_CLK_WEEKPULL 0x01
//#define THD89_RST_WEEKPULL 0x02
//#define THD89_SIO_WEEKPULL 0x04
//#define THD89_GPIO0_WEEKPULL 0x08
//#define THD89_GPIO1_WEEKPULL 0x10
//#define THD89_GPIO2_WEEKPULL 0x20
//#define THD89_GPIO3_WEEKPULL 0x40

////IOPULLSEL2
//#define THD89_CLK_STRONGPULL 0x01
//#define THD89_RST_STRONGPULL 0x02
//#define THD89_SIO_STRONGPULL 0x04
//#define THD89_GPIO0_STRONGPULL 0x08
//#define THD89_GPIO1_STRONGPULL 0x10
//#define THD89_GPIO2_STRONGPULL 0x20
//#define THD89_GPIO3_STRONGPULL 0x40

////CTATRMSK
//#define THD89_ATR3B_STS_MSK 0x01
//#define THD89_RSTRISE_STS_MSK 0x02

////CLKMEMCON
//#define THD89_RAM1_CLK_EN 0x08

////CTTCON
//#define THD89_ETUTIMER_STATR 0x01
//#define THD89_ETUTIMER_TMOD_0 0x00
//#define THD89_ETUTIMER_TMOD_1 0x02
//#define THD89_ETUTIMER_TMOD_2 0x04
//#define THD89_ETUTIMER_TF     0x08

////CTATRWAIT CFG
//#define THD89_ATRCON 0x01

////CTATRSTS
//#define THD89_ATR3B_STS 0x01
//#define THD89_RSTRISE_STS 0x02

////CTSTS
//#define THD89_TXBUF_EMPTY 0x01
//#define THD89_RXBUF_FULL 0x02
//#define THD89_PARITYERROR  0x04
//#define THD89_OVERRUNERROR  0x08

////CTDMACON
//#define THD89_TX_START 0x01
//#define THD89_RX_START 0x02
//#define THD89_ETUAFTERRX_START 0x04
//#define THD89_TXTORX_START 0x08

////CTDMASTS
//#define THD89_TXOVER 0x01
//#define THD89_RXOVER 0x02
//#define THD89_RXNEWBYTE 0x04

////CTCON
//#define THD89_INTORXSTS 0x00
//#define THD89_INTOTXSTS 0x20

////CTATRCON
//#define THD89_AUTOSEND3B 0x01

////CTMSK
//#define THD89_TXBUFEMPTYINT 0x01    
//#define THD89_RXBUFFULLINT  0x02
//#define THD89_PERREINT		0x04
//#define THD89_OERREINT 		0x08

////CTDMAMSK
//#define THD89_DMATXOV_DISEN       0x01
//#define THD89_DMARXOV_DISEN       0x02
//#define THD89_DMARXNEWBYTE_DISEN  0x04


////RFADEMCON
//#define THD89_DEM_EN_H 0x12
//#define THD89_DEM_EN_L 0x11

////RFTRSTA
//#define THD89_RXFIN 0x01
//#define THD89_CRCERR 0x02
//#define THD89_DATOVR 0x08
//#define THD89_PERR 0x20
//#define THD89_TXFIN 0x80

////RFCKCON
//#define THD89_ANTICLK_EN 0x04
//#define THD89_CLKPOWERCONTRL_EN 0x10
//#define THD89_SYSCLKPOWERCONTRL_EN 0X20

////RFTRMODSET
//#define THD89_INTORFTX 0x04
//#define THD89_INTORFRX 0x08

////RFTRCONSET
//#define THD89_STARTANTICOLLISION 0x01
//#define THD89_STARTRXORTX 0x02
//#define THD89_HALTSTS 0x10
//#define THD89_IDLECONENBEFORETXRX 0x80

////RFTRMSK
//#define THD89_RXFINMSK 0x01
//#define THD89_CRCERRMSK 0x02
//#define THD89_DATOVRMSK 0x08
//#define THD89_FERRMSK 0x10
//#define THD89_PERRMSK 0x20
//#define THD89_TXFINMSK 0x80

////RFCRCCON
//#define THD89_JUDGECRCWHENRX  0x40
//#define THD89_GENERATECRCWHENTX 0x80

////CTTMSK
//#define THD89_ETUTIMER_TFMSK 0x01 

////CLKTRCON
//#define THD89_TIMER0_EN 0x01
//#define THD89_TIMER1_EN 0x02
//#define THD89_TIMER2_EN 0x04

////CLKTR0SEL
//#define THD89_SELSYSCLK   0x00
//#define THD89_SEL7816CLK  0x01
//#define THD89_SEL14443CLK 0x07	

////TIMER0CON
//#define THD89_TIMER_START 0x01
//#define THD89_UNLOCK_OV0  0x04
//#define THD89_T0_IDLE_EN  0x08
//#define THD89_T0_CTN      0x10
//#define THD89_T0_RTN      0x20

////INTTRCON
//#define THD89_TIMER0INT_EN 0x01
//#define THD89_TIMER1INT_EN 0x02
//#define THD89_TIMER2INT_EN 0x04

////ABORTTRCON
//#define THD89_TIMER0BREAK_EN 0x01

////RSTTRCON
//#define THD89_TIMER0_RST 0x01
//#define THD89_TIMER1_RST 0x02
//#define THD89_TIMER2_RST 0x04

////RFPOWSTA

//#define THD89_POWPOS    0x01
//#define THD90_RFPOWSTS  0x02


//#define THD89_RFCASA_CASCADE_LEVEL_0			0x00
//#define THD89_RFCASA_CASCADE_LEVEL_1			0x04
//#define THD89_RFCASA_CASCADE_LEVEL_2			0x08
//#define THD89_RFCASA_CURRENTCAS_LEVEL_0			0x00
//#define THD89_RFCASA_CURRENTCAS_LEVEL_1			0x01
//#define THD89_RFCASA_CURRENTCAS_LEVEL_2			0x02

typedef struct
{
	
	void (*Alg_SM7_Process_entry) (unsigned int ,unsigned int ,unsigned char *,unsigned char *,unsigned char *,unsigned char *,unsigned int );
	void (*Alg_SM4_Process_entry)	(unsigned char ,unsigned char ,unsigned char *,unsigned char *,unsigned char *,unsigned char *,unsigned int );
	void (*Alg_SM1_Process_entry) (unsigned char , unsigned char , int , unsigned char , unsigned char *, unsigned char *, unsigned char *, unsigned int , unsigned char *, unsigned char *);
	 unsigned char (*Alg_GetVersion_entry)(unsigned char*);
	u32 a[32-4];
}FunctionEntry;

extern FunctionEntry const FunctionList;


//#define SCP_CTRL_FB_MODE_ECB                     0x01                           /* Feed Back Mode */
//#define SCP_CTRL_FB_MODE_CBC                     0x02                           /* Feed Back Mode */
//#define SCP_CTRL_FB_MODE_BLD                     0x03                           /* Feed Back Mode */


#define CLIB_FAILED(CL_RESULT) ((CL_RESULT != CLIB_STATUS_SUCCESS_UNSPECIFIC)\
                             && (CL_RESULT != CLIB_STATUS_WARNING_CLNGBITMDF))

/// helper for initially loading a clong structure
#define LOADTCLONGNEW(var, MyDataMemory, MyCookie, MySetID)                     \
                  var.super_.BytesAllocated = sizeof(MyDataMemory);          \
                  var.super_.BitLength      = 8*sizeof(MyDataMemory);        \
                  var.super_.Data           = ((UINT8 *)(MyDataMemory));     \
                  var.Cookie                = MyCookie;                      \
                  var.SetID                 = MySetID;

#define NUMBER_OF_WORDS( n ) (( n + (SIZE_OF_WORD*8-1) ) / (SIZE_OF_WORD*8) )    
#define LOADTCLONG(var, MyBYTES, MyBITLEN, MyPMEM, MyCookie, MySetID)\
                  var.super_.BytesAllocated = (MyBYTES);\
                  var.super_.BitLength = (MyBITLEN);\
                  var.super_.Data = ((unsigned char *)(MyPMEM));\
                  var.Cookie = MyCookie;\
                  var.SetID = MySetID;

//const UINT8 zero_mem1[4] = { 0x00, 0x00, 0x00, 0x00 }; 

//__svc_indirect(2) setControlRegPrivMode(int Function, u32 value);

#define ALG_OK 					0xAAAA
#define ALG_ERROR 				0x5555
#endif /* CHIPHEAD_H_ */
