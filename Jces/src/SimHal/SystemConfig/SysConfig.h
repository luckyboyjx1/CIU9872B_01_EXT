#ifndef __SYS_CONFIG_H_
#define __SYS_CONFIG_H_

#include "SysConfigTarget.h"
#include "DataOperation.h"
#include "SysConfigProfile.h"
#include "SysHal.h"
#include "SysConfigData.h"


#define TEMP_ARR_BUF_SIZE									0x118
#define TEMP_ARR_ALIGN_SIZE									0x03
#define EEPROM_BUFFER_SIZE									(0x0C + 0x08 + EEPAGE_SIZE + 0x01)
#define COMPACT_BUFFER_SIZE									0

#define BIG_REUSE_BUFFER_SIZE 								(TEMP_ARR_BUF_SIZE + TEMP_ARR_ALIGN_SIZE + EEPROM_BUFFER_SIZE)

#define isInRam(address)									((PMtMEM(address) >= CHIP_RAM_BASE) && (PMtMEM(address) <= CHIP_RAM_LIMIT))
#define isInE2P(address)									((PMtMEM(address) >= EEPROM_BASE) && (PMtMEM(address) <= EEPROM_LIMIT))
#define isInRom(address)									(((PMtMEM(address) >= ROM_BASE) && (PMtMEM(address) <= ROM_LIMIT)))
#define isValidAddress(addr) 			            		((isInRam(addr) || isInE2P(addr) || isInRom(addr)))


extern u8						X_A							bTempArr[TEMP_ARR_BUF_SIZE];
extern u8						X_A							r_memEepromBuffer[EEPROM_BUFFER_SIZE + TEMP_ARR_ALIGN_SIZE];

#define	CONTACT_BASE_POWER								     0
#define CONTACT_LESS_POWER								     1

#define ILLEGAL_CONFIG_DATA_INDEX                            0xFFFF

#define GET_CONFIG_DATA_OFFSET                               0x00
#define GET_CONFIG_DATA_LENGTH                               0x01

#define SYS_CONFIG_GROUP                                     0x00
#define UART_CONFIG_GROUP                                    0x01
#define TA_CONFIG_GROUP                                      0x02
#define MIFARE_CONFIG_GROUP                                  0x03

//brief: b7 No DESELECT response is sent (bit set) to PCD before ATQA is sent.
#define IOCONFIG_MIFARE_L3_DESELECT_RESP                     0x80
//brief: b6 The Layer 3 Activation ist forced to start in IDLE state
#define IOCONFIG_MIFARE_L3_PROTOCOLRESET                     0x40
//brief: b5-b4 SAK selection mask
#define IOCONFIG_MIFARE_L3_SAK_MASK                          0x30
//brief: Use SAK provided in mifareConfigBuf
#define IOCONFIG_MIFARE_L3_SAK_FLEX                          0x00
//brief: Use SAK from P60 Security Row
#define IOCONFIG_MIFARE_L3_SAK_P60                           0x10
//brief: Use SAK provided by MIFARE Plus IP
#define IOCONFIG_MIFARE_L3_SAK_MFP                           0x20
//brief: Use SAK provided by MIFARE DESFire IP
#define IOCONFIG_MIFARE_L3_SAK_MDF                           0x30
//brief: b3-b2 UID selection mask
#define IOCONFIG_MIFARE_L3_UID_MASK                          0x0C
//brief: Use random UID
#define IOCONFIG_MIFARE_L3_UID_RND                           0x00
//brief: Use UID from P60 Security Row
#define IOCONFIG_MIFARE_L3_UID_P60                           0x04
//brief: Use UID provided by MIFARE Plus IP
#define IOCONFIG_MIFARE_L3_UID_MFP                           0x08
//brief: Use UID provided by MIFARE DESFire IP
#define IOCONFIG_MIFARE_L3_UID_MDF                           0x0C
//brief: b1-b0 ATQA selection mask
#define IOCONFIG_MIFARE_L3_ATQA_MASK                         0x03
//brief: Use ATQA provided in mifareConfigBuf
#define IOCONFIG_MIFARE_L3_ATQA_SMEM                         0x00
//brief: Use ATQA from P60 Security Row
#define IOCONFIG_MIFARE_L3_ATQA_P60                          0x01
//brief: Use ATQA provided by MIFARE Plus IP
#define IOCONFIG_MIFARE_L3_ATQA_MFP                          0x02
//brief: Use ATQA provided by MIFARE DESFire IP
#define IOCONFIG_MIFARE_L3_ATQA_MDF                          0x03

//brief: b7 Exit firmware on UART activity
#define IOCONFIG_MIFARE_L4_EXIT_UART                         0x80
//brief: b6 Exit firmware if no RF field present
#define IOCONFIG_MIFARE_L4_EXIT_RF                           0x40
//brief: DESFire IP processes proprietary DESFire and ISO7816-4 commands
#define IOCONFIG_MIFARE_L4_MDF_CMD                           0x20
//brief: L4 protocol activation handled by Card OS, the framework exits after RATS
#define IOCONFIG_MIFARE_L4_PROTO_SELECT                      0x10
//brief: RFU
#define IOCONFIG_MIFARE_L4_ATS_RFU                           0x00
//brief: Use ATS provided by the Card OS in the shared memory
#define IOCONFIG_MIFARE_L4_ATS_COS                           0x04
//brief: Use ATS provided by MIFARE Plus IP
#define IOCONFIG_MIFARE_L4_ATS_MFP                           0x08
//brief: Use ATS provided by MIFARE DESFire IP
#define IOCONFIG_MIFARE_L4_ATS_MDF                           0x0C
#define IOCONFIG_MIFARE_L4_ATS_MASK                          (IOCONFIG_MIFARE_L4_ATS_COS|IOCONFIG_MIFARE_L4_ATS_MFP)
//brief: b1-b0 IP select mask
#define IOCONFIG_MIFARE_L4_IPSEL_MASK                        0x03
//brief: b1-b0 IP select mask
#define IOCONFIG_MIFARE_L4_IPSEL                             0x03
//brief: RFU
#define IOCONFIG_MIFARE_L4_IPSEL_RFU                         0x00
//brief: no IP is activated
#define IOCONFIG_MIFARE_L4_IPSEL_COS                         0x01
//brief: first MIFARE Plus, then MIFARE DESFire
#define IOCONFIG_MIFARE_L4_IPSEL_MFP                         0x02
//brief: first MIFARE DESFire, then MIFARE Plus
#define IOCONFIG_MIFARE_L4_IPSEL_MDF                         0x03

#if HAL__SYSTEM_CONFIG
extern void sysConfig_Init(void);

#define bigReuseBuf                                          r_memEepromBuffer
#define sysConfig_GetTempArrBuffer(bufSize)                  bTempArr
#define sysConfig_GetEepromBuffer(bufSize)                   r_memEepromBuffer

#if !RAM_COMPACT_BUFFER_IN_LOCAL
#define sysConfig_GetRamCompactBuffer(bufSize)               r_memEepromBuffer
#endif //!RAM_COMPACT_BUFFER_IN_LOCAL

#define sysConfig_GetCpuPower()                              ecpSysHal_GetCpuPower()
#define sysConfig_ChipConfigFun(buf, InfoType)               ecpSysHal_ChipConfigFun(buf, InfoType)
#define sysConfig_SeInfoFun(buf,desOff,datalen,infoType)	 ecpSysHal_SeInfoFun(buf,desOff,datalen,infoType)
#define sysConfig_IsWarmReset()                              ecpSysHal_IsWarmReset()
#define sysConfig_IsDirectConversion()                       ecpSysHal_IsDirectConversion()
#define sysConfig_SetCpuCLK(clk)                             ecpSysHal_SetCpuClk(clk)
#define sysConfig_SetCpuClkCb()                              ecpSysHal_SetCpuClk(sysConfig_GetCpuClkCb())
#define sysConfig_SetCpuClkRf()                              ecpSysHal_SetCpuClk(sysConfig_GetCpuClkRf())

#define sysConfig_SetCpuClkRfCom()                           ecpSysHal_SetCpuClk(sysConfig_GetCpuClkRfCom())

//#define sysConfig_SetCpuClkRfEeProgram()                   ecpSysHal_SetCpuClk(sysConfig_GetRfEeProgramClk())


#define sysConfig_GetCpuCLK()                                ecpSysHal_GetCpuClk()
extern u32 sysConfig_GetCpuClkCb(void);
extern u32 sysConfig_GetCpuClkRf(void);
extern u32 sysConfig_GetCpuClkRfCom(void);
extern u32 sysConfig_GetRsaKeyGenClk(void);
extern u32 sysConfig_GetRsaKeyOperation(void);
extern u32 sysConfig_GetRfEeProgramClk(void);

#define sysConfig_SetIntBeforeRfWtx()                        ecpSysHal_SetIntBeforeRfWtx()
#define sysConfig_SetIntAfterRfWtx()                         ecpSysHal_SetIntAfterRfWtx()
#endif

#if HAL__CONFIG_AREA__MEMORY

#define sysConfig_GetBootKeyValueBase()                      U32_A_U32(COS_CONFIG_START,SEC_MF_KEY_VALUE)
#define sysConfig_GetBootKeyValue()                          PN_R_U8(MEMtPN8(sysConfig_GetBootKeyValueBase()))
#define sysConfig_SetBootKeyValue(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetBootKeyValueBase()),value)

#define sysConfig_GetSecMfKeyChecksumBase()                  U32_A_U32(COS_CONFIG_START,SEC_MF_KEY_CHECKSUM)
#define sysConfig_GetSecMfKeyChecksum()                      PN_R_U8(MEMtPN8(sysConfig_GetSecMfKeyChecksumBase()))
#define sysConfig_SetSecMfKeyChecksum(value)                 nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetSecMfKeyChecksumBase()),value)

#define sysConfig_GetSecMfKeyIvBase()                        U32_A_U32(COS_CONFIG_START,SEC_MF_KEY_IV)
#define sysConfig_GetSecMfKeyIv()                            PN_R_U8(MEMtPN8(sysConfig_GetSecMfKeyIvBase()))
#define sysConfig_SetSecMfKeyIv(value)                       nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetSecMfKeyIvBase()),value)

#define sysConfig_GetBootKeyCounterBase()                    U32_A_U32(COS_CONFIG_START,SEC_MF_KEY_CNTR)
#define sysConfig_GetBootKeyCounter()                        PN_R_U8(MEMtPN8(sysConfig_GetBootKeyCounterBase()))
#define sysConfig_SetBootKeyCounter(value)                   nvmWriteU8Atomic(MEMtPN8(sysConfig_GetBootKeyCounterBase()),value)

#define sysConfig_GetBootKeyFlagBase()                       U32_A_U32(COS_CONFIG_START,SEC_MF_KEY_FLAG)
#define sysConfig_GetBootKeyFlag()                           PN_R_U8(MEMtPN8(sysConfig_GetBootKeyFlagBase()))
#define sysConfig_SetBootKeyFlag(value)                      nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetBootKeyFlagBase()),value)

#define sysConfig_GetBootLifeCycleBase()                     U32_A_U32(COS_CONFIG_START,CM_LIFECYCLE)
#define sysConfig_GetBootLifeCycle()                         PN_R_U16(MEMtPN8(sysConfig_GetBootLifeCycleBase()))
#define sysConfig_SetBootLifeCycle(value)                    WriteU16Atomic(MEMtPN8(sysConfig_GetBootLifeCycleBase()),value)

#define sysConfig_GetPrivateCmdAuthKeyBase()                 U32_A_U32(COS_CONFIG_START,PRIVATE_CMD_AUTH_KEY)
#define sysConfig_GetPrivateCmdAuthKey()                     PN_R_U8(MEMtPN8(sysConfig_GetPrivateCmdAuthKeyBase()))
#define sysConfig_SetPrivateCmdAuthKey(value)                nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetPrivateCmdAuthKeyBase()),value)

#define sysConfig_GetNvmTopBase()                            U32_A_U32(COS_CONFIG_START,E2P_CONFIG_TOP_ADDRESS)
#define sysConfig_GetNvmTop()                                PN_R_U32(MEMtPN8(sysConfig_GetNvmTopBase()))
#define sysConfig_SetNvmTop(value)                           nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetNvmTopBase()),value)

#define sysConfig_GetBackUpPageNumBase()                     U32_A_U32(COS_CONFIG_START,E2P_CONFIG_BACK_UP_PAGE_NUM)
#define sysConfig_GetBackUpPageNum()                         PN_R_U8(MEMtPN8(sysConfig_GetBackUpPageNumBase()))
#define sysConfig_SetBackUpPageNum(value)                    nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetBackUpPageNumBase()),value)

#define sysConfig_GetCryptoConfigBase()                      U32_A_U32(COS_CONFIG_START,CRYPTO_CONFIG)
#define sysConfig_GetCryptoConfig()                          PN_R_U8(MEMtPN8(sysConfig_GetCryptoConfigBase()))
#define sysConfig_SetCryptoConfig(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetCryptoConfigBase()),value)

#define sysConfig_GetOtherConfBase()                         U32_A_U32(COS_CONFIG_START,OTHER_CONFIG)
#define sysConfig_GetOtherConf()                             PN_R_U8(MEMtPN8(sysConfig_GetOtherConfBase()))
#define sysConfig_SetOtherConf(value)                        nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetOtherConfBase()),value)

#define sysConfig_GetFlashProtectPageNumBase()               U32_A_U32(COS_CONFIG_START,E2P_CONFIG_FLASH_PROTECT_PAGE_NUM)
#define sysConfig_GetFlashProtectPageNum()                   PN_R_U8(MEMtPN8(sysConfig_GetFlashProtectPageNumBase()))
#define sysConfig_SetFlashProtectPageNum(value)              nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetFlashProtectPageNumBase()),value)

#define sysConfig_GetTransationFlagNumBase()              	 U32_A_U32(COS_CONFIG_START,E2P_CONFIG_TRANSATION_FLAG_NUM)
#define sysConfig_GetTransationFlagNum()                  	 PN_R_U8(MEMtPN8(sysConfig_GetTransationFlagNumBase()))
#define sysConfig_SetTransationFlagNum(value)              	 nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetTransationFlagNumBase()),value)

#define sysConfig_GetBootAreaConfigBase()                    U32_A_U32(COS_CONFIG_START,BOOT_AREA_CONFIG)
#define sysConfig_GetBootAreaConfig()                        PN_R_U8(MEMtPN8(sysConfig_GetBootAreaConfigBase()))
#define sysConfig_SetBootAreaConfig(value)                   nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetBootAreaConfigBase()),value)

#define sysConfig_GetSpecialTagKeyBase()                     U32_A_U32(COS_CONFIG_START,SEC_SPECIAL_TAG_KEY)
#define sysConfig_GetSpecialTagKey()                         PN_R_U8(MEMtPN8(sysConfig_GetSpecialTagKeyBase()))
#define sysConfig_SetSpecialTagKey(value)                    nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetSpecialTagKeyBase()),value)

#define sysConfig_GetSpecialTagAuthCounterBase()             U32_A_U32(COS_CONFIG_START,SEC_SPECIAL_TAG_KEY_AUTH_CNTR)
#define sysConfig_GetSpecialTagAuthCounter()                 PN_R_U8(MEMtPN8(sysConfig_GetSpecialTagAuthCounterBase()))
#define sysConfig_SetSpecialTagAuthCounter(value)            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetSpecialTagAuthCounterBase()),value)
#else

#define sysConfig_GetBootKeyValueBase()                      0
#define sysConfig_GetBootKeyValue()                          0
#define sysConfig_SetBootKeyValue(value)

#define sysConfig_GetSecMfKeyChecksumBase()                  0
#define sysConfig_GetSecMfKeyChecksum()                      0
#define sysConfig_SetSecMfKeyChecksum(value)

#define sysConfig_GetSecMfKeyIvBase()                        0
#define sysConfig_GetSecMfKeyIv()                            0
#define sysConfig_SetSecMfKeyIv(value)

#define sysConfig_GetBootKeyCounterBase()                    0
#define sysConfig_GetBootKeyCounter()                        0
#define sysConfig_SetBootKeyCounter(value)

#define sysConfig_GetBootKeyFlagBase()                       0
#define sysConfig_GetBootKeyFlag()                           0
#define sysConfig_SetBootKeyFlag(value)

#define sysConfig_GetBootLifeCycleBase()                     0
#define sysConfig_GetBootLifeCycle()                         0
#define sysConfig_SetBootLifeCycle(value)

#define sysConfig_GetPrivateCmdAuthKeyBase()                 0
#define sysConfig_GetPrivateCmdAuthKey()                     0
#define sysConfig_SetPrivateCmdAuthKey(value)

#define sysConfig_GetNvmTopBase()                            0
#define sysConfig_GetNvmTop()                                0
#define sysConfig_SetNvmTop(value)

#define sysConfig_GetBackUpPageNumBase()                     0
#define sysConfig_GetBackUpPageNum()                         0
#define sysConfig_SetBackUpPageNum(value)

#define sysConfig_GetCryptoConfigBase()                      0
#define sysConfig_GetCryptoConfig()                          0
#define sysConfig_SetCryptoConfig(value)

#define sysConfig_GetOtherConfBase()                         0
#define sysConfig_GetOtherConf()                             0
#define sysConfig_SetOtherConf(value)

#define sysConfig_GetErrorHandleBase()                       0
#define sysConfig_GetErrorHandle()                           0
#define sysConfig_SetErrorHandle(value)

#define sysConfig_GetBootAreaConfigBase()                    0
#define sysConfig_GetBootAreaConfig()                        0
#define sysConfig_SetBootAreaConfig(value)

#define sysConfig_GetSpecialTagKeyBase()                     0
#define sysConfig_GetSpecialTagKey()                         0
#define sysConfig_SetSpecialTagKey(value)

#define sysConfig_GetSpecialTagAuthCounterBase()             0
#define sysConfig_GetSpecialTagAuthCounter()                 0
#define sysConfig_SetSpecialTagAuthCounter(value)


#endif


#if HAL__IO__ISO7816_T0
#define sysConfig_GetUartWtxBase()                           U32_A_U32(COS_CONFIG_START,IOCONFIG_UART_WTX)
#define sysConfig_GetUartWtx()                               PN_R_U16(MEMtPN8(sysConfig_GetUartWtxBase()))
#define sysConfig_SetUartWtx(value)                          nvmWriteU16Atomic(MEMtPN8(sysConfig_GetUartWtxBase()),value)

#define sysConfig_GetColdAtrIfLenBase()                      U32_A_U32(COS_CONFIG_START,IOCONFIG_COLD_IFLEN)
#define sysConfig_GetColdAtrIfLen()                          PN_R_U8(MEMtPN8(sysConfig_GetColdAtrIfLenBase()))
#define sysConfig_SetColdAtrIfLen(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetColdAtrIfLenBase()),value)

#define sysConfig_GetColdAtrIfBase()                         U32_A_U32(COS_CONFIG_START,IOCONFIG_COLD_IFBYTES)
#define sysConfig_GetColdAtrIf()                             PN_R_U8(MEMtPN8(sysConfig_GetColdAtrIfBase()))
#define sysConfig_SetColdAtrIf(value)                        nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetColdAtrIfBase()),value)

#define sysConfig_GetColdAtrHistLenBase()                    U32_A_U32(COS_CONFIG_START,IOCONFIG_COLD_HISTLEN)
#define sysConfig_GetColdAtrHistLen()                        PN_R_U8(MEMtPN8(sysConfig_GetColdAtrHistLenBase()))
#define sysConfig_SetColdAtrHistLen(value)                   nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetColdAtrHistLenBase()),value)

#define sysConfig_GetColdAtrHistBase()                       U32_A_U32(COS_CONFIG_START,IOCONFIG_COLD_HISTBYTES)
#define sysConfig_GetColdAtrHist()                           PN_R_U8(MEMtPN8(sysConfig_GetColdAtrHistBase()))
#define sysConfig_SetColdAtrHist(value)                      nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetColdAtrHistBase()),value)

#define sysConfig_GetWarmAtrIfLenBase()                      U32_A_U32(COS_CONFIG_START,IOCONFIG_WARM_IFLEN)
#define sysConfig_GetWarmAtrIfLen()                          PN_R_U8(MEMtPN8(sysConfig_GetWarmAtrIfLenBase()))
#define sysConfig_SetWarmAtrIfLen(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetWarmAtrIfLenBase()),value)

#define sysConfig_GetWarmAtrIfBase()                         U32_A_U32(COS_CONFIG_START,IOCONFIG_WARM_IFBYTES)
#define sysConfig_GetWarmAtrIf()                             PN_R_U8(MEMtPN8(sysConfig_GetWarmAtrIfBase()))
#define sysConfig_SetWarmAtrIf(value)                        nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetWarmAtrIfBase()),value)

#define sysConfig_GetWarmAtrHistLenBase()                    U32_A_U32(COS_CONFIG_START,IOCONFIG_WARM_HISTLEN)
#define sysConfig_GetWarmAtrHistLen()                        PN_R_U8(MEMtPN8(sysConfig_GetWarmAtrHistLenBase()))
#define sysConfig_SetWarmAtrHistLen(value)                   nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetWarmAtrHistLenBase()),value)

#define sysConfig_GetWarmAtrHistBase()                       U32_A_U32(COS_CONFIG_START,IOCONFIG_WARM_HISTBYTES)
#define sysConfig_GetWarmAtrHist()                           PN_R_U8(MEMtPN8(sysConfig_GetWarmAtrHistBase()))
#define sysConfig_SetWarmAtrHist(value)                      nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetWarmAtrHistBase()),value)

#define sysConfig_GetUartConfBase()                          U32_A_U32(COS_CONFIG_START,IOCONFIG_UART_CONF)
#define sysConfig_GetUartConf()                              PN_R_U8(MEMtPN8(sysConfig_GetUartConfBase()))
#define sysConfig_SetUartConf(value)                         nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartConfBase()),value)

#define sysConfig_GetUartConfSecondBase()                    U32_A_U32(COS_CONFIG_START,IOCONFIG_UART_CONF_SECOND)
#define sysConfig_GetUartConfSecond()                        PN_R_U8(MEMtPN8(sysConfig_GetUartConfSecondBase()))
#define sysConfig_SetUartConfSecond(value)                   nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartConfSecondBase()),value)

#define sysConfig_GetUartTC2Base()                           U32_A_U32(COS_CONFIG_START,IOCONFIG_TC2_VALUE)
#define sysConfig_GetUartTC2()                               PN_R_U8(MEMtPN8(sysConfig_GetUartTC2Base()))
#define sysConfig_SetUartTC2(value)                          nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartTC2Base()),value)

#define sysConfig_GetGuardTimeBase()                         U32_A_U32(COS_CONFIG_START,IOCONFIG_GUARDTIME_CB)
#define sysConfig_GetGuardTime()                             PN_R_U16(MEMtPN8(sysConfig_GetGuardTimeBase()))
#define sysConfig_SetGuardTime(value)                        nvmWriteU16Atomic(MEMtPN8(sysConfig_GetGuardTimeBase()),value)

#define sysConfig_GetUartT0ConfigBase()                      U32_A_U32(COS_CONFIG_START,IOCONFIG_T0_CONFIG)
#define sysConfig_GetUartT0Config()                          PN_R_U8(MEMtPN8(sysConfig_GetUartT0ConfigBase()))
#define sysConfig_SetUartT0Config(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartT0ConfigBase()),value)

#else
#define sysConfig_GetUartWtxBase()                           0
#define sysConfig_GetUartWtx()                               0
#define sysConfig_SetUartWtx(value)

#define sysConfig_GetColdAtrIfLenBase()                      0
#define sysConfig_GetColdAtrIfLen()                          0
#define sysConfig_SetColdAtrIfLen(value)

#define sysConfig_GetColdAtrIfBase()                         0
#define sysConfig_GetColdAtrIf()                             0
#define sysConfig_SetColdAtrIf(value)

#define sysConfig_GetColdAtrHistLenBase()                    0
#define sysConfig_GetColdAtrHistLen()                        0
#define sysConfig_SetColdAtrHistLen(value)

#define sysConfig_GetColdAtrHistBase()                       0
#define sysConfig_GetColdAtrHist()                           0
#define sysConfig_SetColdAtrHist(value)

#define sysConfig_GetWarmAtrIfLenBase()                      0
#define sysConfig_GetWarmAtrIfLen()                          0
#define sysConfig_SetWarmAtrIfLen(value)

#define sysConfig_GetWarmAtrIfBase()                         0
#define sysConfig_GetWarmAtrIf()                             0
#define sysConfig_SetWarmAtrIf(value)

#define sysConfig_GetWarmAtrHistLenBase()                    0
#define sysConfig_GetWarmAtrHistLen()                        0
#define sysConfig_SetWarmAtrHistLen(value)

#define sysConfig_GetWarmAtrHistBase()                       0
#define sysConfig_GetWarmAtrHist()                           0
#define sysConfig_SetWarmAtrHist(value)

#define sysConfig_GetUartConfBase()                          0
#define sysConfig_GetUartConf()                              0
#define sysConfig_SetUartConf(value)

#define sysConfig_GetUartConfSecondBase()                    0
#define sysConfig_GetUartConfSecond()                        0
#define sysConfig_SetUartConfSecond(value)

#define sysConfig_GetUartTC2Base()                           0
#define sysConfig_GetUartTC2()                               0
#define sysConfig_SetUartTC2(value)

#define sysConfig_GetGuardTimeBase()                         0
#define sysConfig_GetGuardTime()                             0
#define sysConfig_SetGuardTime(value)

#define sysConfig_GetUartT0ConfigBase()                      0
#define sysConfig_GetUartT0Config()                          0
#define sysConfig_SetUartT0Config(value)
#endif


#if HAL__IO__ISO7816_T1

#define sysConfig_GetUartT1TBBase()                          U32_A_U32(COS_CONFIG_START,IOCONFIG_T1_TB_VALUE)
#define sysConfig_GetUartT1TB()                              PN_R_U8(MEMtPN8(sysConfig_GetUartT1TBBase()))
#define sysConfig_SetUartT1TB(value)                         nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartT1TBBase()),value)

#define sysConfig_GetUartT1TCBase()                          U32_A_U32(COS_CONFIG_START,IOCONFIG_T1_TC_VALUE)
#define sysConfig_GetUartT1TC()                              PN_R_U8(MEMtPN8(sysConfig_GetUartT1TCBase()))
#define sysConfig_SetUartT1TC(value)                         nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartT1TCBase()),value)

#define sysConfig_GetUartT1ConfigBase()                      U32_A_U32(COS_CONFIG_START,IOCONFIG_T1_CONFIG)
#define sysConfig_GetUartT1Config()                          PN_R_U8(MEMtPN8(sysConfig_GetUartT1ConfigBase()))
#define sysConfig_SetUartT1Config(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetUartT1ConfigBase()),value)
#else

#define sysConfig_GetUartT1TBBase()                          0
#define sysConfig_GetUartT1TB()                              0
#define sysConfig_SetUartT1TB(value)

#define sysConfig_GetUartT1TCBase()                          0
#define sysConfig_GetUartT1TC()                              0
#define sysConfig_SetUartT1TC(value)

#define sysConfig_GetUartT1ConfigBase()                      0
#define sysConfig_GetUartT1Config()                          0
#define sysConfig_SetUartT1Config(value)

#endif

#if HAL__IO__ISO14443_TA

#define sysConfig_GetTaWtxDelayLimitBase()                   U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_WTX)
#define sysConfig_GetTaWtxDelayLimit()                       PN_R_U8(MEMtPN8(sysConfig_GetTaWtxDelayLimitBase()))
#define sysConfig_SetTaWtxDelayLimit(value)                  nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetTaWtxDelayLimitBase()),value)

#define sysConfig_GetAtsIfLenBase()                          U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_IFLEN)
#define sysConfig_GetAtsIfLen()                              PN_R_U8(MEMtPN8(sysConfig_GetAtsIfLenBase()))
#define sysConfig_SetAtsIfLen(value)                         nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetAtsIfLenBase()),value)

#define sysConfig_GetAtsIfBase()                             U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_IFBYTES)
#define sysConfig_GetAtsIf()                                 PN_R_U8(MEMtPN8(sysConfig_GetAtsIfBase()))
#define sysConfig_SetAtsIf(value)                            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetAtsIfBase()),value)

#define sysConfig_GetTaUserATQBase()                         U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_USERATQ)
#define sysConfig_GetTaUserATQ()                             PN_R_U8(MEMtPN8(sysConfig_GetTaUserATQBase()))
#define sysConfig_SetTaUserATQ(value)                        nvmWriteU16Atomic(MEMtPN8(sysConfig_GetTaUserATQBase()),value)

#define sysConfig_GetTaUsersakCompleteBase()                 U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_USERSAK_COMPLETE)
#define sysConfig_GetTaUsersakComplete()                     PN_R_U8(MEMtPN8(sysConfig_GetTaUsersakCompleteBase()))
#define sysConfig_SetTaUsersakComplete(value)                nvmWriteU8Atomic(MEMtPN8(sysConfig_GetTaUsersakCompleteBase()),value)

#define sysConfig_GetTaUsersakInCompleteBase()               U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_USERSAK_INCOMPLETE)
#define sysConfig_GetTaUsersakInComplete()                   PN_R_U8(MEMtPN8(sysConfig_GetTaUsersakInCompleteBase()))
#define sysConfig_SetTaUsersakInComplete(value)              nvmWriteU8Atomic(MEMtPN8(sysConfig_GetTaUsersakInCompleteBase()),value)

#define sysConfig_GetTAUIDBase()                             U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_USERUID)
#define sysConfig_GetTAUID()                                 PN_R_U8(MEMtPN8(sysConfig_GetTAUIDBase()))
#define sysConfig_SetTAUID(value)                            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetTAUIDBase()),value)

#define sysConfig_GetTaConfBase()                            U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_CONF)
#define sysConfig_GetTaConf()                                PN_R_U8(MEMtPN8(sysConfig_GetTaConfBase()))
#define sysConfig_SetTaConf(value)                           nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetTaConfBase()),value)

#define sysConfig_GetAtsHistLenBase()                        U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_HISTLEN)
#define sysConfig_GetAtsHistLen()                            PN_R_U8(MEMtPN8(sysConfig_GetAtsHistLenBase()))
#define sysConfig_SetAtsHistLen(value)                       nvmWriteU8Atomic(MEMtPN8(sysConfig_GetAtsHistLenBase()),value)

#define sysConfig_GetAtsHistBase()                           U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_HISTBYTES)
#define sysConfig_GetAtsHist()                               PN_R_U8(MEMtPN8(sysConfig_GetAtsHistBase()))
#define sysConfig_SetAtsHist(value)                          nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetAtsHistBase()),value)

#define sysConfig_GetTaConfigBase()                          U32_A_U32(COS_CONFIG_START,IOCONFIG_TA_CONFIG)
#define sysConfig_GetTaConfig()                              PN_R_U8(MEMtPN8(sysConfig_GetTaConfigBase()))
#define sysConfig_SetTaConfig(value)                         nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetTaConfigBase()),value)

#else

#define sysConfig_GetTaWtxDelayLimitBase()                   0
#define sysConfig_GetTaWtxDelayLimit()                       0
#define sysConfig_SetTaWtxDelayLimit(value)

#define sysConfig_GetAtsIfLenBase()                          0
#define sysConfig_GetAtsIfLen()                              0
#define sysConfig_SetAtsIfLen(value)

#define sysConfig_GetAtsIfBase()                             0
#define sysConfig_GetAtsIf()                                 0
#define sysConfig_SetAtsIf(value)

#define sysConfig_GetTaUserATQBase()                         0
#define sysConfig_GetTaUserATQ()                             0
#define sysConfig_SetTaUserATQ(value)

#define sysConfig_GetTaUsersakCompleteBase()                 0
#define sysConfig_GetTaUsersakComplete()                     0
#define sysConfig_SetTaUsersakComplete(value)

#define sysConfig_GetTaUsersakInCompleteBase()               0
#define sysConfig_GetTaUsersakInComplete()                   0
#define sysConfig_SetTaUsersakInComplete(value)

#define sysConfig_GetTAUIDBase()                             0
#define sysConfig_GetTAUID()                                 0
#define sysConfig_SetTAUID(value)

#define sysConfig_GetTaConfBase()                            0
#define sysConfig_GetTaConf()                                0
#define sysConfig_SetTaConf(value)

#define sysConfig_GetAtsHistLenBase()                        0
#define sysConfig_GetAtsHistLen()                            0
#define sysConfig_SetAtsHistLen(value)

#define sysConfig_GetAtsHistBase()                           0
#define sysConfig_GetAtsHist()                               0
#define sysConfig_SetAtsHist(value)

#define sysConfig_GetTaConfigBase()                          0
#define sysConfig_GetTaConfig()                              0
#define sysConfig_SetTaConfig(value)

#endif



#if HAL__IO__MIFARE
#define sysConfig_GetMifareUsersakInCompleteBase()           U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_USERSAK_INCOMPLETE)
#define sysConfig_GetMifareUsersakInComplete()               PN_R_U8(MEMtPN8(sysConfig_GetMifareUsersakInCompleteBase()))
#define sysConfig_SetMifareUsersakInComplete(value)          nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareUsersakInCompleteBase()),value)

#define sysConfig_GetMifareUsersakCompleteBase()             U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_USERSAK_COMPLETE)
#define sysConfig_GetMifareUsersakComplete()                 PN_R_U8(MEMtPN8(sysConfig_GetMifareUsersakCompleteBase()))
#define sysConfig_SetMifareUsersakComplete(value)            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareUsersakCompleteBase()),value)

#define sysConfig_GetMifareUserATQBase()                     U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_USERATQ)
#define sysConfig_GetMifareUserATQ()                         PN_R_U16(MEMtPN8(sysConfig_GetMifareUserATQBase()))
#define sysConfig_SetMifareUserATQ(value)                    nvmWriteU16Atomic(MEMtPN8(sysConfig_GetMifareUserATQBase()),value)

#define sysConfig_GetMifareUserUidBase()                     U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_USERUID)
#define sysConfig_GetMifareUserUid(des)                      memcpy(des, MEMtPN8(sysConfig_GetMifareUserUidBase()), 4)//PN_R_U8(MEMtPN8(sysConfig_GetMifareUserUidBase()))
#define sysConfig_SetMifareUserUid(value)                    nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareUserUidBase()),value)

#define sysConfig_GetMifareUserUidCtrBase()                  U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_USERUID_CTR)
#define sysConfig_GetMifareUserUidCtr()                      PN_R_U8(MEMtPN8(sysConfig_GetMifareUserUidCtrBase()))
#define sysConfig_SetMifareUserUidCtr(value)                 nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareUserUidCtrBase()),value)

#define sysConfig_GetMifareDeselectBase()                    U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_DESELECT)
#define sysConfig_GetMifareDeselect()                        PN_R_U8(MEMtPN8(sysConfig_GetMifareDeselectBase()))
#define sysConfig_SetMifareDeselect(value)                   nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareDeselectBase()),value)

#define sysConfig_GetMifareL3ConfBase()                      U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_L3_CONF)
#define sysConfig_GetMifareL3Conf()                          PN_R_U8(MEMtPN8(sysConfig_GetMifareL3ConfBase()))
#define sysConfig_SetMifareL3Conf(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareL3ConfBase()),value)

#define sysConfig_GetMifareL4ConfBase()                      U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_L4_CONF)
#define sysConfig_GetMifareL4Conf()                          PN_R_U8(MEMtPN8(sysConfig_GetMifareL4ConfBase()))
#define sysConfig_SetMifareL4Conf(value)                     nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareL4ConfBase()),value)

#define sysConfig_GetMifareConfBase()                        U32_A_U32(COS_CONFIG_START,IOCONFIG_MIFARE_CONF)
#define sysConfig_GetMifareConf()                            PN_R_U8(MEMtPN8(sysConfig_GetMifareConfBase()))
#define sysConfig_SetMifareConf(value)                       nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetMifareConfBase()),value)

#else

#define sysConfig_GetMifareUsersakInCompleteBase()           0
#define sysConfig_GetMifareUsersakInComplete()               0
#define sysConfig_SetMifareUsersakInComplete(value)

#define sysConfig_GetMifareUsersakCompleteBase()             0
#define sysConfig_GetMifareUsersakComplete()                 0
#define sysConfig_SetMifareUsersakComplete(value)

#define sysConfig_GetMifareUserATQBase()                     0
#define sysConfig_GetMifareUserATQ()                         0
#define sysConfig_SetMifareUserATQ(value)

#define sysConfig_GetMifareUserUidBase()                     0
#define sysConfig_GetMifareUserUid()                         0
#define sysConfig_SetMifareUserUid(value)

#define sysConfig_GetMifareUserUidCtrBase()                  0
#define sysConfig_GetMifareUserUidCtr()                      0
#define sysConfig_SetMifareUserUidCtr(value)

#define sysConfig_GetMifareDeselectBase()                    0
#define sysConfig_GetMifareDeselect()                        0
#define sysConfig_SetMifareDeselect(value)

#define sysConfig_GetMifareL3ConfBase()                      0
#define sysConfig_GetMifareL3Conf()                          0
#define sysConfig_SetMifareL3Conf(value)

#define sysConfig_GetMifareL4ConfBase()                      0
#define sysConfig_GetMifareL4Conf()                          0
#define sysConfig_SetMifareL4Conf(value)

#define sysConfig_GetMifareConfBase()                        0
#define sysConfig_GetMifareConf()                            0
#define sysConfig_SetMifareConf(value)

#endif

#if HAL__IO__SWP
#define sysConfig_GetIoconfigSwpTypeAEnableBase()            U32_A_U32(COS_CONFIG_START,SWP_TYPEA_ENABLE)
#define sysConfig_GetIoconfigSwpTypeAEnable()                PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeAEnableBase()))
#define sysConfig_SetIoconfigSwpTypeAEnable(value)           nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAEnableBase()),value)

#define sysConfig_GetIoconfigSwpTypeAUidLenBase()            U32_A_U32(COS_CONFIG_START,SWP_TYPEA_UID_LEN)
#define sysConfig_GetIoconfigSwpTypeAUidLen()                PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeAUidLenBase()))
#define sysConfig_SetIoconfigSwpTypeAUidLen(value)           nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAUidLenBase()),value)

#define sysConfig_GetIoconfigSwpTypeAUidValueBase()          U32_A_U32(COS_CONFIG_START,SWP_TYPEA_UID)
#define sysConfig_GetIoconfigSwpTypeAUidValue(des)           memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeAUidValueBase()), sysConfig_GetIoconfigSwpTypeAUidLen())
#define sysConfig_SetIoconfigSwpTypeAUidValue(src,len)       nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAUidValueBase()), src, len)

#define sysConfig_GetIoconfigSwpTypeASakBase()          	 U32_A_U32(COS_CONFIG_START,SWP_TYPEA_SAK)
#define sysConfig_GetIoconfigSwpTypeASak()                   PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeASakBase()))
#define sysConfig_SetIoconfigSwpTypeASak(value)              nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeASakBase()),value)

#define sysConfig_GetIoconfigSwpTypeAAtqaBase()          	 U32_A_U32(COS_CONFIG_START,SWP_TYPEA_ATQA)
#define sysConfig_GetIoconfigSwpTypeAAtqa()                  PN_R_U16(MEMtPN8(sysConfig_GetIoconfigSwpTypeAAtqaBase()))
#define sysConfig_SetIoconfigSwpTypeAAtqa(value)             nvmWriteU16NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAAtqaBase()),value)

#define sysConfig_GetIoconfigSwpTypeAAppDataLenBase()        U32_A_U32(COS_CONFIG_START,SWP_TYPEA_APPDATA_LEN)
#define sysConfig_GetIoconfigSwpTypeAAppDataLen()            PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeAAppDataLenBase()))
#define sysConfig_SetIoconfigSwpTypeAAppDataLen(value)       nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAAppDataLenBase()),value)

#define sysConfig_GetIoconfigSwpTypeAAppDataBase()           U32_A_U32(COS_CONFIG_START,SWP_TYPEA_APPDATA)
#define sysConfig_GetIoconfigSwpTypeAAppData(des)            memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeAAppDataBase()), sysConfig_GetIoconfigSwpTypeAAppDataLen())
#define sysConfig_SetIoconfigSwpTypeAAppData(src,len)        nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAAppDataBase()), src, len)

#define sysConfig_GetIoconfigSwpTypeAFwiBase()               U32_A_U32(COS_CONFIG_START,SWP_TYPEA_FWI_SFGI)
#define sysConfig_GetIoconfigSwpTypeAFwi()            	     PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeAFwiBase()))
#define sysConfig_SetIoconfigSwpTypeAFwi(value)       		 nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeAFwiBase()),value)

#define sysConfig_GetIoconfigSwpTypeACidBase()               U32_A_U32(COS_CONFIG_START,SWP_TYPEA_CID_SUPPORT)
#define sysConfig_GetIoconfigSwpTypeACid()            	     PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeACidBase()))
#define sysConfig_SetIoconfigSwpTypeACid(value)       		 nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeACidBase()),value)

#define sysConfig_GetIoconfigSwpTypeADataRateBase()          U32_A_U32(COS_CONFIG_START,SWP_TYPEA_DATARATE_MAX)
#define sysConfig_GetIoconfigSwpTypeADataRate(des)           memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeADataRateBase()), 3)
#define sysConfig_SetIoconfigSwpTypeADataRate(src,len)       nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeADataRateBase()), src, len)

#define sysConfig_GetIoconfigSwpTypeBEnableBase()            U32_A_U32(COS_CONFIG_START,SWP_TYPEB_ENABLE)
#define sysConfig_GetIoconfigSwpTypeBEnable()                PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeBEnableBase()))
#define sysConfig_SetIoconfigSwpTypeBEnable(value)           nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBEnableBase()),value)

#define sysConfig_GetIoconfigSwpTypeBPupiLenBase()           U32_A_U32(COS_CONFIG_START,SWP_TYPEB_PUPI_LEN)
#define sysConfig_GetIoconfigSwpTypeBPupiLen()               PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeBPupiLenBase()))
#define sysConfig_SetIoconfigSwpTypeBPupiLen(value)          nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBPupiLenBase()), value)

#define sysConfig_GetIoconfigSwpTypeBPupiValueBase()         U32_A_U32(COS_CONFIG_START,SWP_TYPEB_PUPI)
#define sysConfig_GetIoconfigSwpTypeBPupiValue(des)          memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeBPupiValueBase()), sysConfig_GetIoconfigSwpTypeBPupiLen())
#define sysConfig_SetIoconfigSwpTypeBPupiValue(src,len)      nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBPupiValueBase()), src, len)

#define sysConfig_GetIoconfigSwpTypeBAfiBase()               U32_A_U32(COS_CONFIG_START,SWP_TYPEB_AFI)
#define sysConfig_GetIoconfigSwpTypeBAfi()                   PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeBAfiBase()))
#define sysConfig_SetIoconfigSwpTypeBAfi(value)              nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBAfiBase()),value)

#define sysConfig_GetIoconfigSwpTypeBAtqbValueBase()         U32_A_U32(COS_CONFIG_START,SWP_TYPEB_ATQB)
#define sysConfig_GetIoconfigSwpTypeBAtqbValue(des)          memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeBAtqbValueBase()), 4)
#define sysConfig_SetIoconfigSwpTypeBAtqbValue(src,len)      nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBAtqbValueBase()), src, len)

#define sysConfig_GetIoconfigSwpTypeBHighLayerResLenBase()   U32_A_U32(COS_CONFIG_START,SWP_TYPEB_HIGHT_LAYER_RESPONSE_LEN)
#define sysConfig_GetIoconfigSwpTypeBHighLayerResLen()       PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpTypeBHighLayerResLenBase()))
#define sysConfig_SetIoconfigSwpTypeBHighLayerResLen(value)  nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBHighLayerResLenBase()),value)

#define sysConfig_GetIoconfigSwpTypeBHighLayerResDataBase()     U32_A_U32(COS_CONFIG_START,SWP_TYPEB_HIGHT_LAYER_RESPONSE)
#define sysConfig_GetIoconfigSwpTypeBHighLayerResData(des)      memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeBHighLayerResDataBase()), sysConfig_GetIoconfigSwpTypeBHighLayerResLen())
#define sysConfig_SetIoconfigSwpTypeBHighLayerResData(src,len)  nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBHighLayerResDataBase()), src, len)

#define sysConfig_GetIoconfigSwpTypeBDataRateBase()          U32_A_U32(COS_CONFIG_START,SWP_TYPEB_DATARATE_MAX)
#define sysConfig_GetIoconfigSwpTypeBDataRate(des)           memcpy(des, MEMtPN8(sysConfig_GetIoconfigSwpTypeBDataRateBase()), 3)
#define sysConfig_SetIoconfigSwpTypeBDataRate(src,len)       nvmCopyNoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpTypeBDataRateBase()), src, len)


#define sysConfig_GetIoconfigSwpClfModelIdBase()             U32_A_U32(COS_CONFIG_START,IOCONFIG_SWP_CLF_MODEL_ID)
#define sysConfig_GetIoconfigSwpClfModelId()                 PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpClfModelIdBase()))
#define sysConfig_SetIoconfigSwpClfModelId(value)            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpClfModelIdBase()),value)

#define sysConfig_GetIoconfigSwpAmendmentCTestBase()         U32_A_U32(COS_CONFIG_START,IOCONFIG_SWP_AMENDMENT_C_TEST)
#define sysConfig_GetIoconfigSwpAmendmentCTest()             PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpAmendmentCTestBase()))
#define sysConfig_SetIoconfigSwpAmendmentCTest(value)        nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpAmendmentCTestBase()),value)

//bit0: contactless para switch control
//bit1: mifare enable control
#define SWP_CONFIG_CONTACTLESS_PARA_SWITCH_MASK              0x01
#define SWP_CONFIG_MIFARE_ENABLE_MASK                        0x02

#define sysConfig_GetIoconfigSwpConfigBase()                 U32_A_U32(COS_CONFIG_START,IOCONFIG_SWP_CONFIG)
#define sysConfig_GetIoconfigSwpConfig()                     PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpConfigBase()))
#define sysConfig_SetIoconfigSwpConfig(value)                nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetIoconfigSwpConfigBase()),value)

#define sysConfig_GetContactlessParaSwitch()                 (PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpConfigBase())) & 0x01)
#define sysConfig_GetMifareEnable()                          (PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpConfigBase())) & 0x02)

#else

#define sysConfig_GetIoconfigSwpClfModelIdBase()             0
#define sysConfig_GetIoconfigSwpClfModelId()                 0
#define sysConfig_SetIoconfigSwpClfModelId(value)

#define sysConfig_GetIoconfigSwpAmendmentCTestBase()         0
#define sysConfig_GetIoconfigSwpAmendmentCTest()             0
#define sysConfig_SetIoconfigSwpAmendmentCTest(value)

#define sysConfig_GetIoconfigSwpConfigBase()                 0
#define sysConfig_GetIoconfigSwpConfig()                     0
#define sysConfig_SetIoconfigSwpConfig(value)

#define sysConfig_GetContactlessParaSwitch()                 0
#define sysConfig_GetMifareEnable()                          0
#endif

//#define sysConfig_GetContactlessParaSwitch()                 (PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpConfigBase())) & 0x01)
//#define sysConfig_GetMifareEnable()                          (PN_R_U8(MEMtPN8(sysConfig_GetIoconfigSwpConfigBase())) & 0x02)


#if HAL__PATCH


#define sysConfig_GetPatchAreaBase()                         sysConfig_GetEePatchSizeBase()

#define sysConfig_GetEePatchSizeBase()                       U32_A_U32(COS_CONFIG_START,VM_EE_PATCH_SIZE)
#define sysConfig_GetEePatchSize()                           PN_R_U32(MEMtPN8(sysConfig_GetEePatchSizeBase()))
#define sysConfig_SetEePatchSize(value)                      nvmWriteU32Atomic(MEMtPN8(sysConfig_GetEePatchSizeBase()),value)

#define sysConfig_GetexternalPatchTableFlgBase()             U32_A_U32(COS_CONFIG_START,EXTERNAL_PATCH_TABLE_FLG)
#define sysConfig_GetexternalPatchTableFlg()                 PN_R_U8(MEMtPN8(sysConfig_GetexternalPatchTableFlgBase()))
#define sysConfig_SetexternalPatchTableFlg(value)            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetexternalPatchTableFlgBase()),value)

#define sysConfig_GetexternalPatchTableNumBase()             U32_A_U32(COS_CONFIG_START,EXTERNAL_PATCH_TABLE_NUM)
#define sysConfig_GetexternalPatchTableNum()                 PN_R_U8(MEMtPN8(sysConfig_GetexternalPatchTableNumBase()))
#define sysConfig_SetexternalPatchTableNum(value)            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetexternalPatchTableNumBase()),value)

#define sysConfig_GetexternalPatchTableAddrBase()            U32_A_U32(COS_CONFIG_START,EXTERNAL_PATCH_TABLE_ADDR)
#define sysConfig_GetexternalPatchTableAddr()                PN_R_U32(MEMtPN8(sysConfig_GetexternalPatchTableAddrBase()))
#define sysConfig_SetexternalPatchTableAddr(value)           nvmWriteU32NoAtomic(MEMtPN8(sysConfig_GetexternalPatchTableAddrBase()),value)

#define sysConfig_GetPatchFlagEnableBase()                   U32_A_U32(COS_CONFIG_START,PATCH_FLAG_ENABLE)
#define sysConfig_GetPatchFlagEnable()                       
#define sysConfig_SetPatchFlagEnable(value)                  

#define sysConfig_GetDefaultPatchTableAddrBase()             U32_A_U32(COS_CONFIG_START,DEFAULT_PATCH_TABLE_ADDR)
#define sysConfig_GetDefaultPatchTableAddr()                 
#define sysConfig_SetDefaultPatchTableAddr(value)            

#else

#define sysConfig_GetEePatchSizeBase()                       U32_A_U32(COS_CONFIG_START,VM_EE_PATCH_SIZE)
#define sysConfig_GetEePatchSize()                           PN_R_U32(MEMtPN8(sysConfig_GetEePatchSizeBase()))
#define sysConfig_SetEePatchSize(value)                      nvmWriteU32NoAtomic(MEMtPN8(sysConfig_GetEePatchSizeBase()),value)


#define sysConfig_GetexternalPatchTableFlgBase()             0
#define sysConfig_GetexternalPatchTableFlg()                 0
#define sysConfig_SetexternalPatchTableFlg(value)

#define sysConfig_GetexternalPatchTableNumBase()             0
#define sysConfig_GetexternalPatchTableNum()                 0
#define sysConfig_SetexternalPatchTableNum(value)

#define sysConfig_GetexternalPatchTableAddrBase()            0
#define sysConfig_GetexternalPatchTableAddr()                0
#define sysConfig_SetexternalPatchTableAddr(value)

#define sysConfig_GetPatchFlagEnableBase()                   0
#define sysConfig_GetPatchFlagEnable()                       0
#define sysConfig_SetPatchFlagEnable(value)

#define sysConfig_GetDefaultPatchTableAddrBase()             0
#define sysConfig_GetDefaultPatchTableAddr()                 0
#define sysConfig_SetDefaultPatchTableAddr(value)

#endif

#if HAL__CONFIG_AREA__MEMORY

#define sysConfig_GetClkConfigCbBase()                       U32_A_U32(COS_CONFIG_START,CLK_CONFIG_CB)
#define sysConfig_GetClkConfigCb()                           PN_R_U8(MEMtPN8(sysConfig_GetClkConfigCbBase()))
#define sysConfig_SetClkConfigCb(value)                      nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetClkConfigCbBase()),value)

#define sysConfig_GetClkConfigClBase()                       U32_A_U32(COS_CONFIG_START,CLK_CONFIG_CL)
#define sysConfig_GetClkConfigCl()                           PN_R_U8(MEMtPN8(sysConfig_GetClkConfigClBase()))
#define sysConfig_SetClkConfigCl(value)                      nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetClkConfigClBase()),value)

#define sysConfig_GetClkConfigClComBase()                    U32_A_U32(COS_CONFIG_START,CLK_CONFIG_CL_COM)
#define sysConfig_GetClkConfigClCom()                        PN_R_U8(MEMtPN8(sysConfig_GetClkConfigClComBase()))
#define sysConfig_SetClkConfigClCom(value)                   nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetClkConfigClComBase()),value)

#define sysConfig_GetClkConfigRsaKeygenBase()                U32_A_U32(COS_CONFIG_START,CLK_CONFIG_RSA_KEYGEN)
#define sysConfig_GetClkConfigRsaKeygen()                    PN_R_U8(MEMtPN8(sysConfig_GetClkConfigRsaKeygenBase()))
#define sysConfig_SetClkConfigRsaKeygen(value)               nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetClkConfigRsaKeygenBase()),value)

#define sysConfig_GetClkConfigRsaOperationBase()             U32_A_U32(COS_CONFIG_START,CLK_CONFIG_RSA_OPERATION)
#define sysConfig_GetClkConfigRsaOperation()                 PN_R_U8(MEMtPN8(sysConfig_GetClkConfigRsaOperationBase()))
#define sysConfig_SetClkConfigRsaOperation(value)            nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetClkConfigRsaOperationBase()),value)

#define sysConfig_GetClkConfigClEepromProgramBase()          U32_A_U32(COS_CONFIG_START,CLK_CONFIG_CL_EEPROM_PROGRAM)
#define sysConfig_GetClkConfigClEepromProgram()              PN_R_U8(MEMtPN8(sysConfig_GetClkConfigClEepromProgramBase()))
#define sysConfig_SetClkConfigClEepromProgram(value)         nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetClkConfigClEepromProgramBase()),value)

#define sysConfig_GetHalConfigBase()                         U32_A_U32(COS_CONFIG_START,HAL_CONFIG)
#define sysConfig_GetHalConfig()                             PN_R_U8(MEMtPN8(sysConfig_GetHalConfigBase()))
#define sysConfig_SetHalConfig(value)                        nvmWriteU8NoAtomic(MEMtPN8(sysConfig_GetHalConfigBase()),value)

#define sysConfig_GetAttackCounterBase()					 U32_A_U32(COS_CONFIG_START,IFX_ATTACK_COUNTER)
#define sysConfig_GetAttackCounter()						 PN_R_U8(MEMtPN8(sysConfig_GetAttackCounterBase()))
#else

#define sysConfig_GetClkConfigCbBase()                       0
#define sysConfig_GetClkConfigCb()                           0
#define sysConfig_SetClkConfigCb(value)

#define sysConfig_GetClkConfigClBase()                       0
#define sysConfig_GetClkConfigCl()                           0
#define sysConfig_SetClkConfigCl(value)

#define sysConfig_GetClkConfigClComBase()                    0
#define sysConfig_GetClkConfigClCom()                        0
#define sysConfig_SetClkConfigClCom(value)

#define sysConfig_GetClkConfigRsaKeygenBase()                0
#define sysConfig_GetClkConfigRsaKeygen()                    0
#define sysConfig_SetClkConfigRsaKeygen(value)

#define sysConfig_GetClkConfigRsaOperationBase()             0
#define sysConfig_GetClkConfigRsaOperation()                 0
#define sysConfig_SetClkConfigRsaOperation(value)

#define sysConfig_GetClkConfigClEepromProgramBase()          0
#define sysConfig_GetClkConfigClEepromProgram()              0
#define sysConfig_SetClkConfigClEepromProgram(value)

#define sysConfig_GetHalConfigBase()                         0
#define sysConfig_GetHalConfig()                             0
#define sysConfig_SetHalConfig(value)

#define sysConfig_GetAttackCounterBase()                         0
#define sysConfig_GetAttackCounter()                             0
#endif
#define sysConfig_IsTaConfEmvTest()                           U8_AND_U8(sysConfig_GetTaConf(), 0x20)
#define sysConfig_TaIblockNeedNoCheck()                       U8_AND_U8(sysConfig_GetTaConf(), 0x80)
#define IOCONFIG_TA_CONF_USERUID_MASK                        0x01
#define IOCONFIG_TA_CONF_USERSAK_COMPLETE_MASK               0x02
#define IOCONFIG_TA_CONF_USERSAK_INCOMPLETE_MASK             0x04
#define IOCONFIG_TA_CONF_USERATQ_MASK                        0x08
#define IOCONFIG_TA_CONF_UID_LEVEL                           0x40

#define IOCONFIG_MIFARE_CONF_ON_MASK                         0x01
#define IOCONFIG_MIFARE_CONF_OFF_MASK                        0xFE
#define IOCONFIG_MIFARE_CONF_API_MASK                        0x02
#define IOCONFIG_MIFARE_CONF_ATS_MASK                        0x04
#define IOCONFIG_MIFARE_CONF_INIT_MASK                       0x10

#define sysConfig_IsMifareOs()                               U8_AND_U8(sysConfig_GetMifareConf(),IOCONFIG_MIFARE_CONF_ON_MASK)
#define IOCONFIG_MIFARE_CONF_API                             U8_AND_U8(sysConfig_GetMifareConf(),IOCONFIG_MIFARE_CONF_API_MASK)
#define IOCONFIG_MIFARE_CONF_ATS                             U8_AND_U8(sysConfig_GetMifareConf(),IOCONFIG_MIFARE_CONF_ATS_MASK)
#define IOCONFIG_MIFARE_CONF_INIT                            U8_AND_U8(sysConfig_GetMifareConf(),IOCONFIG_MIFARE_CONF_INIT_MASK)

#define sysConfig_UartPPSEnable()                            U8_AND_U8(sysConfig_GetUartConf(), 0x01)
#define sysConfig_IsUartEnterSleepMode()                     U8_AND_U8(sysConfig_GetUartConf(), 0x02)
#define sysConfig_UartDefaultProtocl()                       U8_AND_U8(sysConfig_GetUartConf(), 0x04)
#define sysConfig_UartInverseConvention()                    U8_AND_U8(sysConfig_GetUartConf(), 0x08)
#define sysConfig_IsColdAtrTtrTckPresent()                   U8_AND_U8(sysConfig_GetUartConf(), 0x10)
#define sysConfig_IsWarmAtrTtrTckPresent()                   U8_AND_U8(sysConfig_GetUartConf(), 0x20)
//EID2a?D閿熸枻鎷穉??????0
#define sysConfig_IsUartToSend616CXX()                       U8_AND_U8(sysConfig_GetUartConf(), 0x40)
#define sysConfig_IsUartC0HaveLogicNum()                     U8_AND_U8(sysConfig_GetUartConf(), 0x80)
/*bit 0 exist meaning Test API SendBytes Or SendBytesLong For 00c0 only once*/
#define sysConfig_IsUartToSend61XXforTest()                  U8_AND_U8(sysConfig_GetUartConfSecond(), 0x01)
/*bit 0 exist meaning Test API SendBytes Or SendBytesLong For 00c0 only once*/
#define sysConfig_IsUartToDataBefor61XX()                    U8_AND_U8(sysConfig_GetUartConfSecond(), 0x02)

#define sysConfig_IsTranExceptionOpen()                      U8_AND_U8(sysConfig_GetOtherConf(), 0x10)
#define OTHER_CONFIG_EEP_INT0                                U8_AND_U8(sysConfig_GetOtherConf(), 0x01)
#define OTHER_CONFIG_EEP_INT1                                U8_AND_U8(sysConfig_GetOtherConf(), 0x02)
#define OTHER_CONFIG_TRAN_OVER                               U8_AND_U8(sysConfig_GetOtherConf(), 0x10)
#define OTHER_CONFIG_WAIT_EEP                                U8_AND_U8(sysConfig_GetOtherConf(), 0x20)
#define OTHER_CONFIG_CALL_FVEC                               U8_AND_U8(sysConfig_GetOtherConf(), 0x40)

#define HAL_CONFIG_RF_EEP_PROGRAM_CLK                        U8_AND_U8(sysConfig_GetHalConfig(), 0x01)

//this is used for SM2 encrypt and decrypt, the ciphertext sequence is C1 || C3 || C2.
#define OS_CONF4_SM2_ENC_C132_MODE                           U8_AND_U8(sysConfig_GetCryptoConfig(), 0x01)
//wzq: use for private command(get data) to get all config data
#define ALL_TAG_ENABLE                                       U8_AND_U8(sysConfig_GetCryptoConfig(), 0x02)
#define IOCONFIG_CBCTEST_RECEIVELENTH                        U8_AND_U8(sysConfig_GetCryptoConfig(), 0x04)
#define IOCONFIG_EIDTEST_RECEIVELENTH_OVERLAY                U8_AND_U8(sysConfig_GetCryptoConfig(), 0x08)
#define GUOMI_STANDARD                                       1 //U8_AND_U8(sysConfig_GetCryptoConfig(), 0x10) modify by xuxiaojun@20220913:根据第一次代码评审(20220913).xlsx，将此处直接写成1
//0x20  RSAFAME, 0x00  RSALIB
#define RSALIB_RSAFAME                                       U8_AND_U8(sysConfig_GetCryptoConfig(), 0x20)
#define BLOCK_BOOT_CONFIG                                    U8_AND_U8(sysConfig_GetBootKeyFlag(), 0x80)
#define PRIVATE_COMMAND_DISABLE_CONFIG                       U8_AND_U8(sysConfig_GetBootKeyFlag(), 0x40)

#define OTHER_CONFIG_PATCH_EEP                               U8_AND_U8(PN_R_U8(PATCH_AREA_CONFIG_ADDR), 0x04)
#define OTHER_CONFIG_PATCH_SERVICE_SEL                       U8_AND_U8(PN_R_U8(PATCH_AREA_CONFIG_ADDR), 0x08)
#define OTHER_CONFIG_DATA_PATCH_WRITE                        U8_AND_U8(PN_R_U8(PATCH_AREA_CONFIG_ADDR), 0x80)

#define BOOT_CONFIG_EXTENDED_LOGICAL_CHANNEL                 U8_AND_U8(sysConfig_GetBootAreaConfig(), 0x01)
#define BOOT_CONFIG_SPECIAL_TAG_VALID_CHECK                  U8_AND_U8(sysConfig_GetBootAreaConfig(), 0x02)
#define BOOT_CONFIG_TA_PROLOGUE_UNSIGNED_VALUE_ON            U8_AND_U8(sysConfig_GetBootAreaConfig(), 0x04)

#endif
