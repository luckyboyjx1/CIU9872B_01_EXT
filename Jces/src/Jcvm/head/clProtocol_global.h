#ifndef __CLPROTOCOL_GLOBAL_H__
#define __CLPROTOCOL_GLOBAL_H__

/**
 * @file clProtocol_global.h
 * @brief Header file with global variable definition.
 * @author Joerg Poeltl (IFAT DCGR CC CL78)
 * @version 1.0
 * @date 2008-04-04 - file created
 */

extern u8 bClAnticollCfg1;               /*!< variable defined within AnticollTypeA.asm */
#ifdef L_MIFARE
  extern u8 bClAnticollCfg2;               /*!< variable defined within AnticollTypeA.asm */
#endif
//extern CLPROTOCOL_STATE eClProtocolState;
//extern CLPROTOCOL_CONFIG_PARAMS eClProtocolParams;
extern u8 bClProtocolExtParam;
extern u8 *pbCommBuffer;
extern u8 bAppState;
extern u8 bFirstATRByteSentBySTS;
extern u8  bHCRClkOffFlag;

#define WTX_CALLBACK_FUNC_ADDR_ADDR     0x0020
#define EXT_CLK_VALUE                   13560L
#define FWI_MAX                         0x0E 
#define FWI_DEFAULT                     0x04
#define FWI_WTXPROG_LIMIT               0x05 

#define CLPROTOCOL_DLSM_ANTICOLL        0x01
#define CLPROTOCOL_DLSM_RX              0x02
#define CLPROTOCOL_DLSM_TXRX            0x03
#define CLPROTOCOL_DLSM_PRORX           0x04
#define CLPROTOCOL_DLSM_PROTX           0x05
#define CLPROTOCOL_DLSM_PROTXCHAIN      0x06
#define CLPROTOCOL_DLSM_PROTXRE         0x07
#define CLPROTOCOL_DLSM_PROTX_ACK       0x08

#ifdef L_ISO18092
  #define CLPROTOCOL_DLSM_MODE0         0x0B
  #define CLPROTOCOL_DLSM_MODE0_TX      0x0C
  #define CLPROTOCOL_DLSM_MODE0_RX      0x0D
  #define CI_FAIL                       0xFFFF
  #define MAX_SUPP_SERV                 0x02 //supported Services of Tag
  #define ISO18092_MAX_FRAME_SIZE       0xFF
#endif // L_ISO18092



#define PPS_PPS1_AVAIL    0x10
#define PPS1_RFU_MASK     0xF0
#define PPS1_RX_MASK      0x03
#define PPS1_TX_MASK      0x0C
#define ATTRIB_RX_MASK    0x30
#define ATTRIB_TX_MASK    0xC0

#define EXTFRAMESIZE_MAX      0xAA
#define BLOCK_WRITE_SIZE      16
#define CLKU_OPM_CLKSCL_MAX_VALUE   (0x1F) /**< Max possible value for the iClkScale */

#define DEFAULT_COMM_BUFFER           0xFFFFA200 /**< Address of the default communication buffer used to send/receive data (RFI_RAM by default)*/
#define DEFAULT_COMM_BUFFER_SIZE      0x0100 /**< Size of the default communication buffer (RFI_RAM) */
#define HFS_ACTIVATION_LIMIT          0x0100 /**< Maximum frame size that can be handled without HFS activation */
#define MIN_APP_DATA_BUFFER_LENGTH    0x0100 /**< Minimum size of the Application Data buffer */

#define IMM_RSTTRG_WR           0x0001

#define MACBLOCKSIZE            16
#define MACPADDING              0x00
#define DECBLOCKSIZE            16
#define MINHEADER               7
#define APDUNOTCOMPLETE         0
#define APDUCOMPLETE            1
#define APDUCOMPLETE_EXIT       2


#define CLPROTOCOL_WTX_SEND             0x00
#define CLPROTOCOL_WTX_CHECK            0x01
#define CLPROTOCOL_WTX_RX               0x02

#define CLPROTOCOL_EXTPARAM_INIT        0x00
#define CLPROTOCOL_EXTPARAM_HFS         0x01
#define CLPROTOCOL_EXTPARAM_ACM         0x02
#define CLPROTOCOL_EXTPARAM_NBLOCK      0x80

#define BCLANTICOLLCFG1_DESELECT        0x40

#define BCLANTICOLLCFG1_TYPEA_CL2       0x01
#define BCLANTICOLLCFG1_TYPEA_CL3       0x02
#define BCLANTICOLLCFG1_TYPEA_RFU       0x04
#define BCLANTICOLLCFG1_HALTRXTX        0x08
#define BCLANTICOLLCFG1_TYPEA_CL2_EN    0x10
#define BCLANTICOLLCFG1_TYPEA_CL3_EN    0x20
#define BCLANTICOLLCFG1_TYPEA_HALT      0x80
#define BCLANTICOLLCFG1_TYPEA_IDLE      0x00

#ifdef L_MIFARE
  #define BCLANTICOLLCFG2_TYPEA_MIFAVAIL  0x01
  #define BCLANTICOLLCFG2_TYPEA_MIF4K     0x04
  #define BCLANTICOLLCFG2_TYPEA_MIF_MEM   0x08
  #define BCLANTICOLLCFG2_TYPEA_MIF_RB0   0x10
  #define BCLANTICOLLCFG2_TYPEA_MIF_NOEMU 0x20
  #define BCLANTICOLLCFG2_TYPEA_MIF_SCAEN 0x40
#endif

#define BCLANTICOLLCFG1_TYPEB_IDLE      0x00


// ALLOWED STATES
#define ALLOWED_I_BLOCK                 0x80
#define ALLOWED_R_BLOCK_ACK_CBN         0x40
#define ALLOWED_R_BLOCK_ACK_WBN         0x20
#define ALLOWED_R_BLOCK_NAK_CBN         0x10
#define ALLOWED_R_BLOCK_NAK_WBN         0x08


// STATE CODING
#define I_BLOCK         0x02  //I-Block:        000x xx1x
#define R_BLOCK_ACK     0xA2  //R-Block-ACK:    1010 x01x
#define R_BLOCK_NAK     0xB2  //R-Block-NAK:    1011 x01x
#define S_BLOCK_DES     0xC2  //S-Block:        1100 x010
#define S_BLOCK_PARAM   0xF0  //S-Block:        1111 x000
#define S_BLOCK_WTX     0xF2  //S-Block:        1111 x010

// Bitmasks for decoding the several state
#define M_PPS           0xD0  //PPS command:    1101 0000
#define M_I_BLOCK       0xE2  //I-BLOCK:        1110 0010
#define M_R_BLOCK       0xF6  //R-BLOCK:        1111 0110
#define M_S_BLOCK       0xF7  //S-BLOCK:        1111 0111
#define M_CHAINING      0x10  //Chaining:       0001 0000
#define M_CID           0x08  //CID following:  0000 1000
#define M_NAD           0x04  //NAD following:  0000 0100
#define M_BLOCK_NUMBER  0x01  //BLOCKNUMBER:    0000 0001
#define M_CID_BYTE      0xFF  //CID Byte:       1111 1111
#define M_POWER_LEVEL   0x00  //Power Level Indication:           0000 0000
#define M_WTXM          0x3F  //Waiting Time eXtension Multipier  0011 1111
#define M_WTXM_RFU      0xC0  //Waiting Time RFU values           1100 0000

// Maximum Baudrates for CL protocols
#define MAX_BR_TX_TYPE_A      RFAPI_BR_TX_6_8M
#define MAX_BR_TX_TYPE_B      RFAPI_BR_TX_6_8M
#define MAX_BR_TX_TYPE_F      RFAPI_BR_TX_212K

#define MAX_BR_RX_TYPE_A      RFAPI_BR_RX_1_7M
#define MAX_BR_RX_TYPE_B      RFAPI_BR_RX_1_7M
#define MAX_BR_RX_TYPE_F      RFAPI_BR_RX_212K

// ISO 14443 A constants
#define MAX_FSCI        0x0C
#define MAX_FSDI        0x0C

#define APPSTATE_FASTPERSO    1

/*!
 * <add description here!>
 */
typedef struct tdCLPROTOCOL_PROLOGUE
{
u8  bPCB;                              /*!< PCB of last received block. */
u8  bCID;                              /*!< CID of last received block. value = 0xFF if no CID was send. */
u8  bNAD;                              /*!< NAD of last received block. value = 0xFF if no NAD was send.*/
} CLPROTOCOL_PROLOGUE;

//#define CLPROTOCOL_PROLOGUE_NOCID 0xFF    /*!< no CID was send within last received block */
//#define CLPROTOCOL_PROLOGUE_NONAD 0xFF    /*!< no NAD was send within last received block */

//CLPROTOCOL_STATUS far clProtocol_DatalinkStatemachine(u8 *pbAppDataTX, u16 wAppDataLenTX, u8 *pbAppDataRX, u16 *pwAppDataLenRX, u16 wAppDataMaxRXLen, u8 bCmd);
//CLPROTOCOL_STATUS far clAnticoll(u16 *pwCommLength);
u8 clAnticollTypeA(u8 *clUID);
u8 clAnticollTypeB(u8 *pbATQB, u8 bAFI, u8 *pbParam);


// utility
void Mem2Comm(u8 *pbSrc, u8 *pbDest, u16 wDestOffset, u16 wSize);
void Comm2Mem(u8 *pbSrc, u16 wSrcOffset, u8 *pbDest, u16 wSize);
void StartWTXTimer(u8 bWTXM);
void StopWTXTimer(void);
void ISR_clProtocol_SendWTX_Callback(void);
#ifdef SLE78
  void far EnablePFD(void);
  void far DisablePFD(void);
#endif

// common defines for SFRs
#define LINT_CTRL_MAX                 0x12


#endif  /* __CLPROTOCOL_GLOBAL_H__ */
