/**
 * @file NVM.h
 * @brief Defines and definition of configurable contactless settings.
 * @author Joerg Poeltl (IFAT DCGR CC CL78)
 */

#include "typejava.h"
#include "halCryptoProfile.h"
/*#ifdef SLE78
  #define EEDATA_SIZE                 256U//65536U
#else // SLE77
  #define EEDATA_SIZE                 32768U
#endif*/

#define NVM_PAGESIZE                256
#define NVM_SECTORSIZE              8192U


/* As NVM values can be changed by users, it may happen that configuration values
   are used that prevent the AppNote from performing a correct activation. Therefore,
   fallback values are defined which should be used in case the present values prefent 
   the AppNote from a correct activation. */

#if 0
#define NVM_CONFIG_FALLBACK_MAX_COMMUNICATION_BUFFER  0x0100
#define NVM_CONFIG_FALLBACK_TYPEA_ATS_LENGTH          0x0E
#define NVM_CONFIG_FALLBACK_TYPEA_ATS                 {0x0E, 0x78, 0x77, 0x84, 0x03, 0x66, 'R', 'W', 'E', '2', 'P', 'R', 'O', 'M'}

#define EECFG_APPLICATION                     0
#define EECFG_APPLICATION_SIZE                17
#define EECFG_APPLICATION_COMTYPE             (EECFG_APPLICATION + 0)
#define EECFG_APPLICATION_HCOMTYPE            (EECFG_APPLICATION + 1)
#define EECFG_APPLICATION_PROTOCOL_T0         (EECFG_APPLICATION + 2) 
#define EECFG_APPLICATION_PROTOCOL_DCLB       (EECFG_APPLICATION + 3) 
#define EECFG_APPLICATION_WTXMARGINNVMPROG    (EECFG_APPLICATION + 4)
#define EECFG_APPLICATION_WTXMARGINMIN        (EECFG_APPLICATION + 6)
#define EECFG_APPLICATION_MAXAPDUBUF          (EECFG_APPLICATION + 8)
#define EECFG_APPLICATION_WTXMODE             (EECFG_APPLICATION + 10)
#define EECFG_APPLICATION_POWERLEV            (EECFG_APPLICATION + 11)
#define	EECFG_APPLICATION_PROTOCOL_CL		  (EECFG_APPLICATION + 12)
#define EECFG_APPLICATION_RFCFGSECT           (EECFG_APPLICATION + 14)
#define EECFG_APPLICATION_CB_STARTUP_MODE     (EECFG_APPLICATION + 15)
#define EECFG_APPLICATION_RFCLK_SIGN          (EECFG_APPLICATION + 16)


#define EECFG_TYPEA                           (EECFG_APPLICATION + EECFG_APPLICATION_SIZE)
#define EECFG_TYPEA_SIZE                      44  
#define EECFG_TYPEA_UIDRANDOM                 (EECFG_TYPEA + 0)
#define EECFG_TYPEA_UIDLEN                    (EECFG_TYPEA + 1)
#define EECFG_TYPEA_UID                       (EECFG_TYPEA + 2)
#define EECFG_TYPEA_ATSLEN                    (EECFG_TYPEA + 14)
#define EECFG_TYPEA_ATS                       (EECFG_TYPEA + 15)
#define EECFG_TYPEA_ATS_LEN                   (EECFG_TYPEA_ATS + 0)
#define EECFG_TYPEA_ATS_FMT                   (EECFG_TYPEA_ATS + 1)
#define EECFG_TYPEA_ATS_TA1                   (EECFG_TYPEA_ATS + 2)
#define EECFG_TYPEA_ATS_TB1                   (EECFG_TYPEA_ATS + 3)
#define EECFG_TYPEA_ATS_TC1                   (EECFG_TYPEA_ATS + 4)
#define EECFG_TYPEA_ATS_HIS                   (EECFG_TYPEA_ATS + 5)
#define EECFG_TYPEA_UPDATE_TYPEA_FDT_ADJ      (EECFG_TYPEA + 35)
#define EECFG_TYPEA_ATQA				      (EECFG_TYPEA + 39)
#define EECFG_TYPEA_SAK				          (EECFG_TYPEA + 41)
#define	MIFARE_SAK_1K						  (EECFG_TYPEA + 42) 
#define	MIFARE_SAK_4K						  (EECFG_TYPEA + 43)	    


#define EECFG_TYPEB                           (EECFG_TYPEA + EECFG_TYPEA_SIZE)
#define EECFG_TYPEB_SIZE                      30
#define EECFG_TYPEB_MBLI                      (EECFG_TYPEB + 0)
#define EECFG_TYPEB_AFI                       (EECFG_TYPEB + 1)
#define EECFG_TYPEB_PUPIRANDOM                (EECFG_TYPEB + 2)
#define EECFG_TYPEB_ATQBLEN                   (EECFG_TYPEB + 3)
#define EECFG_TYPEB_ATQB                      (EECFG_TYPEB + 4)
#define EECFG_TYPEB_PUPI                      (EECFG_TYPEB + 5)
#define EECFG_TYPEB_ATQB_AFI                  (EECFG_TYPEB_PUPI + 4)         //ATQB: AFI,CRC1,CRC2,APPLICATIONNUMBER
#define EECFG_TYPEB_CRC1                      (EECFG_TYPEB_PUPI + 5)
#define EECFG_TYPEB_CRC2                      (EECFG_TYPEB_PUPI + 6)
#define EECFG_TYPEB_APPLICATIONNUMBER         (EECFG_TYPEB_PUPI + 7)
#define EECFG_TYPEB_BITRATE                   (EECFG_TYPEB_APPLICATIONNUMBER  + 1)
#define EECFG_TYPEB_MAXFRAMESIZE              (EECFG_TYPEB_BITRATE + 1)
#define EECFG_TYPEB_FWI_ADC_FO                (EECFG_TYPEB_MAXFRAMESIZE +1)
#define EECFG_TYPEB_EXTENDED_ATQB             (EECFG_TYPEB_FWI_ADC_FO + 1)
#define EECFG_TYPEB_HISTORICAL_BYTES          (EECFG_TYPEB_EXTENDED_ATQB + 1)
#define EECFG_TYPEB_TYPEBPARA_MAX             (EECFG_TYPEB_HISTORICAL_BYTES + 13)  

#define EECFG_TYPEF                           (EECFG_TYPEB + EECFG_TYPEB_SIZE)
#define EECFG_TYPEF_SIZE                      (96)
#define EECFG_TYPEF_SDI1                      (EECFG_TYPEF + 0)
#define EECFG_TYPEF_SDI2                      (EECFG_TYPEF + 16)
#define EECFG_TYPEF_SYDI                      (EECFG_TYPEF + 32)
#define EECFG_TYPEF_MANID                     (EECFG_TYPEF + 48)
#define EECFG_TYPEF_UB0                       (EECFG_TYPEF + 64)
#define EECFG_TYPEF_TIM_CORR_VAL              (EECFG_TYPEF + 80)
#define EECFG_TYPEF_NVM_PM_ADJ_NOM            (EECFG_TYPEF + 82)
#define EECFG_TYPEF_NVM_PM_ADJ_LP             (EECFG_TYPEF + 83)
#define EECFG_TYPEF_NVM_PM_ADJ_ALP            (EECFG_TYPEF + 84)

#define EECFG_DCLB_HCR                        (EECFG_TYPEF + EECFG_TYPEF_SIZE)
#define EECFG_DCLB_HCR_SIZE                   0
#define EECFG_DCLB_HCR_CLKU_HCR               (EECFG_DCLB_HCR + 0)
#define EECFG_DCLB_HCR_CLKU_CURLIM_CHANGE_HCR (EECFG_DCLB_HCR + 1)
#define EECFG_DCLB_HCR_DEFAULT_CURLIM_SETTING (EECFG_DCLB_HCR + 2)                        
#define EECFG_DCLB_HCR_DEFAULT_COMM_AFTER_HCR (EECFG_DCLB_HCR + 3)
#define EECFG_DCLB_HCR_CLF_RX_BUFFER_SIZE     (EECFG_DCLB_HCR + 4)
#define EECFG_DCLB_HCR_SE_TX_BYTES            (EECFG_DCLB_HCR + 5)
#define EECFG_DCLB_HCR_SE_TX_MAX_BYTES        (EECFG_DCLB_HCR + 6)               
#define EECFG_DCLB_HCR_TX_OFFSET_NVMDATA      (EECFG_DCLB_HCR + 7)  
#define	EECFG_DCLB_HCR_THRESHOLD_LEVEL        (EECFG_DCLB_HCR + 9) 


#define EECFG_RFCFG                           (EECFG_DCLB_HCR + EECFG_DCLB_HCR_SIZE)
#define EECFG_RFCFG_SIZE                      128
#endif


#define EECFG_TPA_TLVDATA			         0x00  //(EECFG_RFCFG + EECFG_RFCFG_SIZE)
#define EECFG_TPA_TLVDATA_SIZE				 49

#define EECFG_TPA_TLVMASK			         (EECFG_TPA_TLVDATA + EECFG_TPA_TLVDATA_SIZE)
#define EECFG_TPA_TLVMASK_SIZE				 49

#define EECFG_TPB_TLVDATA			         (EECFG_TPA_TLVMASK + EECFG_TPA_TLVMASK_SIZE)
#define EECFG_TPB_TLVDATA_SIZE				 49

#define EECFG_TPB_TLVMASK			         (EECFG_TPB_TLVDATA + EECFG_TPB_TLVDATA_SIZE)
#define EECFG_TPB_TLVMASK_SIZE				 49



// Other constants for accessing NVM config data

// Offsets for ATS access in EECFG
#define EECFG_TYPEA_ATS_OFFSET_T0     1         /**< Offset of T0 byte in ATS from ATS start address in NVM*/
#define EECFG_TYPEA_ATS_OFFSET_TA     2         /**< Offset of T1 byte in ATS from ATS start address in NVM*/
#define EECFG_TYPEA_ATS_OFFSET_TB     3         /**< Offset of T2 byte in ATS from ATS start address in NVM*/
#define EECFG_TYPEA_ATS_OFFSET_TC     4         /**< Offset of T3 byte in ATS from ATS start address in NVM*/

// Offsets for ATQB access in EECFG
#define EECFG_TYPEB_ATQB_OFFSET_PROTOCOL_INFO     9                                           /**< Offset of Protocol Info block in the ATQB from ATQB start address in NVM*/
#define EECFG_TYPEB_ATQB_OFFSET_BIT_RATES         EECFG_TYPEB_ATQB_OFFSET_PROTOCOL_INFO       /**< Offset of bit rates byte in the ATQB from ATQB start address in NVM*/
#define EECFG_TYPEB_ATQB_OFFSET_FRAME_SIZE        (EECFG_TYPEB_ATQB_OFFSET_PROTOCOL_INFO + 1)   /**< Offset of maximum frame size byte in the ATQB from ATQB start address in NVM*/

//#define EECFG_SIZE	(EECFG_APPLICATION_SIZE + EECFG_TYPEA_SIZE + EECFG_TYPEB_SIZE + EECFG_TYPEF_SIZE + EECFG_DCLB_HCR_SIZE + EECFG_RFCFG_SIZE + EECFG_TPA_TLVDATA_SIZE + EECFG_TPA_TLVMASK_SIZE + EECFG_TPB_TLVDATA_SIZE + EECFG_TPB_TLVMASK_SIZE)
#define EECFG_SIZE		(EECFG_TPA_TLVDATA_SIZE + EECFG_TPA_TLVMASK_SIZE + EECFG_TPB_TLVDATA_SIZE + EECFG_TPB_TLVMASK_SIZE)

//extern u8 const bEEDATA[EEDATA_SIZE/2];
#if RUN_IN_SIMULATOR
extern u8 bEECFG1[EECFG_SIZE];
extern u8 bEECFG2[EECFG_SIZE];
#else
extern u8 const bEECFG1[EECFG_SIZE];
extern u8 const bEECFG2[EECFG_SIZE];
#endif
extern u8 *bEECFG;
extern u8 direction;
//extern u8 EFCFG_FLAG;
extern u8 const Atr0[35];
extern u16 const WTXT0[1];
extern u8 const  FEATUREBLOCKBYTES[5];
//extern u8 const DCLB_SE_N[EEDATA_SIZE/2];
