/****************************************************
* Copyright (C), 2008-2015, Eastcompeace. Co., Ltd. 
* this code is auto generate by config gen tool
* build time: 2024-06-24 18:25:32
* build by:   liguiyu
******************************************************/ 
#ifndef SYS_CONFIG_DATA_H_
#define SYS_CONFIG_DATA_H_

#include "SysConfigProfile.h"
#include "SysConfigTarget.h"

#define    CONFIG_IN_BUFFER						1
#define    BOOT_CONFIG							1
#define    HAL_ISO7816_T0_CONFIG				1
#define    HAL_ISO7816_T1_CONFIG				1
#define    HAL_ISO14443_TA_CONFIG			1
#define    HAL_MIFARE_CONFIG					1
#define    HAL_SWP_SUPPORTED_CONFIG				0
#define    HAL_CLK_CONFIG						1
#define    HAL_RESERVE_CONFIG					1

#if CONFIG_IN_BUFFER
	extern u8 CONST sysConfig[];
#endif//CONFIG_IN_BUFFER


#define     SYSCONFIG_OFFSET_START          (0x00000000L)

                                            //EEPROM Address  tag  Length  default_Value
#if  BOOT_CONFIG
#define    SEC_MF_KEY_VALUE                   0x0000        //FFFF   0x10   00112233445566778899AABBCCDDEEFF
#define    SEC_MF_KEY_IV                      0x0010        //FFFF   0x01   00
#define    SEC_MF_KEY_CHECKSUM                0x0011        //FFFF   0x01   00
#define    SEC_MF_KEY_CNTR                    0x0012        //FFFF   0x01   06
#define    SEC_MF_KEY_FLAG                    0x0013        //FFFF   0x01   00
#define    CM_LIFECYCLE                       0x0014        //FFFF   0x02   20DF
#define    PRIVATE_CMD_AUTH_KEY               0x0016        //FFFF   0x08   4142434445464748
#define    E2P_CONFIG_TOP_ADDRESS             0x001e        //0006   0x04   100BAC00
#define    E2P_CONFIG_BACK_UP_PAGE_NUM        0x0022        //0007   0x01   10
#define    CRYPTO_CONFIG                      0x0023        //FFFF   0x01   11
#define    OTHER_CONFIG                       0x0024        //FFFF   0x01   40
#define    E2P_CONFIG_FLASH_PROTECT_PAGE_NUM  0x0025        //FFFF   0x01   06
#define    BOOT_AREA_CONFIG                   0x0026        //FFFF   0x01   03
#define    SEC_SPECIAL_TAG_KEY                0x0027        //FFFF   0x08   0011223344556677
#define    SEC_SPECIAL_TAG_KEY_AUTH_CNTR      0x002f        //FFFF   0x01   08
#define    E2P_CONFIG_TRANSATION_FLAG_NUM     0x0030        //FFFF   0x01   04

#define    BOOT_CONFIG_LENGTH                 0x0031
#endif

#if  HAL_ISO7816_T0_CONFIG
#define    IOCONFIG_UART_WTX                  0x0031        //010C   0x02   0320
#define    IOCONFIG_COLD_IFLEN                0x0033        //0101   0x01   06
#define    IOCONFIG_COLD_IFBYTES              0x0034        //0102   0x0f   9D97803FC7A0000000000000000000
#define    IOCONFIG_COLD_HISTLEN              0x0043        //0103   0x01   0D
#define    IOCONFIG_COLD_HISTBYTES            0x0044        //0104   0x0f   4A434553333150364434304D34FFFF
#define    IOCONFIG_WARM_IFLEN                0x0053        //0105   0x01   06
#define    IOCONFIG_WARM_IFBYTES              0x0054        //0106   0x0f   9D97803FC7A0000000000000000000
#define    IOCONFIG_WARM_HISTLEN              0x0063        //0107   0x01   0D
#define    IOCONFIG_WARM_HISTBYTES            0x0064        //0108   0x0f   4A434553333150364434304D34FFFF
#define    IOCONFIG_UART_CONF                 0x0073        //FFFF   0x01   33
#define    IOCONFIG_UART_CONF_SECOND          0x0074        //FFFF   0x01   01
#define    IOCONFIG_TC2_VALUE                 0x0075        //0109   0x01   0A
#define    IOCONFIG_GUARDTIME_CB              0x0076        //010D   0x02   0005
#define    IOCONFIG_T0_CONFIG                 0x0078        //FFFF   0x01   00

#define    HAL_ISO7816_T0_CONFIG_LENGTH       0x0048
#endif

#if  HAL_ISO7816_T1_CONFIG
#define    IOCONFIG_T1_TB_VALUE               0x0079        //010A   0x01   4D
#define    IOCONFIG_T1_TC_VALUE               0x007a        //010B   0x01   00
#define    IOCONFIG_T1_CONFIG                 0x007b        //FFFF   0x01   00

#define    HAL_ISO7816_T1_CONFIG_LENGTH       0x0003
#endif

#if  HAL_ISO14443_TA_CONFIG
#define    IOCONFIG_TA_WTX                    0x007c        //FFFF   0x03   000001
#define    IOCONFIG_TA_IFLEN                  0x007f        //0205   0x01   04
#define    IOCONFIG_TA_IFBYTES                0x0080        //0206   0x0f   78B384020000000000000000000000
#define    IOCONFIG_TA_USERATQ                0x008f        //0204   0x02   0004
#define    IOCONFIG_TA_USERSAK_COMPLETE       0x0091        //0202   0x01   20
#define    IOCONFIG_TA_USERSAK_INCOMPLETE     0x0092        //0203   0x01   24
#define    IOCONFIG_TA_USERUID                0x0093        //0201   0x0c   01234567FFFFFFFFFFFFFFFF
#define    IOCONFIG_TA_CONF                   0x009f        //FFFF   0x01   3F
#define    IOCONFIG_TA_HISTLEN                0x00a0        //0207   0x01   0D
#define    IOCONFIG_TA_HISTBYTES              0x00a1        //0208   0x0f   4A434553333150364434304D34FFFF
#define    IOCONFIG_TA_CONFIG                 0x00b0        //FFFF   0x01   00

#define    HAL_ISO14443_TA_CONFIG_LENGTH      0x0035
#endif

#if  HAL_MIFARE_CONFIG
#define    IOCONFIG_MIFARE_USERSAK_INCOMPLETE 0x00b1        //0304   0x01   30
#define    IOCONFIG_MIFARE_USERSAK_COMPLETE   0x00b2        //0303   0x01   38
#define    IOCONFIG_MIFARE_USERATQ            0x00b3        //0301   0x02   0200
#define    IOCONFIG_MIFARE_USERUID            0x00b5        //0302   0x0c   01234567FFFFFFFFFFFFFFFF
#define    IOCONFIG_MIFARE_USERUID_CTR        0x00c1        //FFFF   0x01   02
#define    IOCONFIG_MIFARE_DESELECT           0x00c2        //FFFF   0x01   00
#define    IOCONFIG_MIFARE_L3_CONF            0x00c3        //FFFF   0x01   80
#define    IOCONFIG_MIFARE_L4_CONF            0x00c4        //FFFF   0x01   15
#define    IOCONFIG_MIFARE_CONF               0x00c5        //FFFF   0x01   00

#define    HAL_MIFARE_CONFIG_LENGTH           0x0015
#endif

#if  HAL_SWP_SUPPORTED_CONFIG
#define    SWP_TYPEA_ENABLE                                 //0401   0x01   02
#define    SWP_TYPEA_UID_LEN                                //0402   0x01   00
#define    SWP_TYPEA_UID                                    //0403   0x0a   10001D51FFFFFFFFFFFF
#define    SWP_TYPEA_SAK                                    //0404   0x01   20
#define    SWP_TYPEA_ATQA                                   //0405   0x02   0400
#define    SWP_TYPEA_APPDATA_LEN                            //0406   0x01   03
#define    SWP_TYPEA_APPDATA                                //0407   0x0f   901234FFFFFFFFFFFFFFFFFFFFFFFF
#define    SWP_TYPEA_FWI_SFGI                               //0408   0x01   78
#define    SWP_TYPEA_CID_SUPPORT                            //0409   0x01   01
#define    SWP_TYPEA_CLT_SUPPORT                            //FFFF   0x01   00
#define    SWP_TYPEA_DATARATE_MAX                           //040A   0x03   000000
#define    SWP_TYPEB_ENABLE                                 //0410   0x01   02
#define    SWP_TYPEB_PUPI_LEN                               //0411   0x01   04
#define    SWP_TYPEB_PUPI                                   //0412   0x04   00000001
#define    SWP_TYPEB_AFI                                    //0413   0x01   01
#define    SWP_TYPEB_ATQB                                   //0414   0x04   00000070
#define    SWP_TYPEB_HIGHT_LAYER_RESPONSE_LEN               //0415   0x01   00
#define    SWP_TYPEB_HIGHT_LAYER_RESPONSE                   //0416   0x0f   FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#define    SWP_TYPEB_DATARATE_MAX                           //0417   0x03   000001
#define    IOCONFIG_SWP_CONFIG                              //FFFF   0x01   E3

#define    HAL_SWP_SUPPORTED_CONFIG_LENGTH    0x0000
#endif

#if  HAL_CLK_CONFIG
#define    CLK_CONFIG_CB                      0x00c6        //0001   0x01   00
#define    CLK_CONFIG_CL                      0x00c7        //0002   0x01   1C
#define    CLK_CONFIG_CL_COM                  0x00c8        //0003   0x01   18
#define    CLK_CONFIG_RSA_KEYGEN              0x00c9        //0004   0x01   A0
#define    CLK_CONFIG_RSA_OPERATION           0x00ca        //0005   0x01   A0
#define    CLK_CONFIG_CL_EEPROM_PROGRAM       0x00cb        //0008   0x01   1C
#define    HAL_CONFIG                         0x00cc        //FFFF   0x01   00

#define    HAL_CLK_CONFIG_LENGTH              0x0007
#endif

#if  HAL_RESERVE_CONFIG
#define    HAL_RESERVE                        0x00cd        //FFFF   0x03   All(FF)

#define    HAL_RESERVE_CONFIG_LENGTH          0x0003
#endif


#define     SYSCONFIG_OFFSET_END          (0x000000cfL)

#define		COS_CONFIG_OFFSET_START				SYSCONFIG_OFFSET_START
#define		SWP_CONFIG_START					SWP_TYPEA_ENABLE
#define		BOOT_VERSION_SIZE					0x10
extern u8 CONST bootVersion[BOOT_VERSION_SIZE];


#endif	    /*SYS_CONFIG_DATA_H_*/



