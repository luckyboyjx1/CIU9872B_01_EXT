/****************************************************
* Copyright (C), 2008-2015, Eastcompeace. Co., Ltd. 
* this code is auto generate by config gen tool
* .\output\halDataCinfig.c
*    build time: 2024-06-24 18:25:32
*    build by:   liguiyu
******************************************************/ 
#include "SysConfigProfile.h"
#include "SysConfigTarget.h"
#include "ecpDefine.h"
#include "SysConfigData.h"

#if CONFIG_IN_BUFFER
__attribute__( ( section( "sysConfigDataArea" ) ) )

u8 CONST sysConfig[]  = {
#if  BOOT_CONFIG
         /*SEC_MF_KEY_VALUE                       0x10(0x0000)*/
          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 
          
         /*SEC_MF_KEY_IV                          0x01(0x0010)*/
          0x00, 
         /*SEC_MF_KEY_CHECKSUM                    0x01(0x0011)*/
          0x00, 
         /*SEC_MF_KEY_CNTR                        0x01(0x0012)*/
          0x06, 
         /*SEC_MF_KEY_FLAG                        0x01(0x0013)*/
          0x00, 
         /*CM_LIFECYCLE                           0x02(0x0014)*/
          0x20, 0xDF, 
         /*PRIVATE_CMD_AUTH_KEY                   0x08(0x0016)*/
          0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 
         /*E2P_CONFIG_TOP_ADDRESS                 0x04(0x001e)*/
          0x00, 0x87, 0xE4, 0x00,
         /*E2P_CONFIG_BACK_UP_PAGE_NUM            0x01(0x0022)*/
          0x10, 
         /*CRYPTO_CONFIG                          0x01(0x0023)*/
          0x11, 
         /*OTHER_CONFIG                           0x01(0x0024)*/
          0x40, 
         /*E2P_CONFIG_FLASH_PROTECT_PAGE_NUM      0x01(0x0025)*/
          0x06, 
         /*BOOT_AREA_CONFIG                       0x01(0x0026)*/
          0x03, 
         /*SEC_SPECIAL_TAG_KEY                    0x08(0x0027)*/
          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
         /*SEC_SPECIAL_TAG_KEY_AUTH_CNTR          0x01(0x002f)*/
          0x08, 
         /*E2P_CONFIG_TRANSATION_FLAG_NUM         0x01(0x0030)*/
          0x04, 
#endif

#if  HAL_ISO7816_T0_CONFIG
         /*IOCONFIG_UART_WTX                      0x02(0x0031)*/
          0x03, 0x20, 
         /*IOCONFIG_COLD_IFLEN                    0x01(0x0033)*/
          0x06, 
         /*IOCONFIG_COLD_IFBYTES                  0x0f(0x0034)*/
          0x9D, 0x96, 0x80, 0x3F, 0xC7, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
         /*IOCONFIG_COLD_HISTLEN                  0x01(0x0043)*/
          0x0D, 
         /*IOCONFIG_COLD_HISTBYTES                0x0f(0x0044)*/
          0x4A, 0x43, 0x45, 0x53, 0x33, 0x31, 0x50, 0x36, 0x44, 0x34, 0x30, 0x4D, 0x34, 0xFF, 0xFF, 
         /*IOCONFIG_WARM_IFLEN                    0x01(0x0053)*/
          0x06, 
         /*IOCONFIG_WARM_IFBYTES                  0x0f(0x0054)*/
          0x9D, 0x96, 0x80, 0x3F, 0xC7, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
         /*IOCONFIG_WARM_HISTLEN                  0x01(0x0063)*/
          0x0D, 
         /*IOCONFIG_WARM_HISTBYTES                0x0f(0x0064)*/
          0x4A, 0x43, 0x45, 0x53, 0x33, 0x31, 0x50, 0x36, 0x44, 0x34, 0x30, 0x4D, 0x34, 0xFF, 0xFF, 
         /*IOCONFIG_UART_CONF                     0x01(0x0073)*/
          0x33, 
         /*IOCONFIG_UART_CONF_SECOND              0x01(0x0074)*/
          0x01, 
         /*IOCONFIG_TC2_VALUE                     0x01(0x0075)*/
          0x0A, 
         /*IOCONFIG_GUARDTIME_CB                  0x02(0x0076)*/
          0x00, 0x05, 
         /*IOCONFIG_T0_CONFIG                     0x01(0x0078)*/
          0x00, 
#endif

#if  HAL_ISO7816_T1_CONFIG
         /*IOCONFIG_T1_TB_VALUE                   0x01(0x0079)*/
          0x4D, 
         /*IOCONFIG_T1_TC_VALUE                   0x01(0x007a)*/
          0x00, 
         /*IOCONFIG_T1_CONFIG                     0x01(0x007b)*/
          0x00, 
#endif

#if  HAL_ISO14443_TA_CONFIG
         /*IOCONFIG_TA_WTX                        0x03(0x007c)*/
          0x00, 0x00, 0x01, 
         /*IOCONFIG_TA_IFLEN                      0x01(0x007f)*/
          0x04, 
         /*IOCONFIG_TA_IFBYTES                    0x0f(0x0080)*/
          0x78, 0xB3, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
         /*IOCONFIG_TA_USERATQ                    0x02(0x008f)*/
          0x00, 0x04, 
         /*IOCONFIG_TA_USERSAK_COMPLETE           0x01(0x0091)*/
          0x20, 
         /*IOCONFIG_TA_USERSAK_INCOMPLETE         0x01(0x0092)*/
          0x24, 
         /*IOCONFIG_TA_USERUID                    0x0c(0x0093)*/
          0x01, 0x23, 0x45, 0x67, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
         /*IOCONFIG_TA_CONF                       0x01(0x009f)*/
          0x3F, 
         /*IOCONFIG_TA_HISTLEN                    0x01(0x00a0)*/
          0x0D, 
         /*IOCONFIG_TA_HISTBYTES                  0x0f(0x00a1)*/
          0x4A, 0x43, 0x45, 0x53, 0x33, 0x31, 0x50, 0x36, 0x44, 0x34, 0x30, 0x4D, 0x34, 0xFF, 0xFF, 
         /*IOCONFIG_TA_CONFIG                     0x01(0x00b0)*/
          0x00, 
#endif

#if  HAL_MIFARE_CONFIG
         /*IOCONFIG_MIFARE_USERSAK_INCOMPLETE     0x01(0x00b1)*/
          0x30, 
         /*IOCONFIG_MIFARE_USERSAK_COMPLETE       0x01(0x00b2)*/
          0x38, 
         /*IOCONFIG_MIFARE_USERATQ                0x02(0x00b3)*/
          0x02, 0x00, 
         /*IOCONFIG_MIFARE_USERUID                0x0c(0x00b5)*/
          0x01, 0x23, 0x45, 0x67, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
         /*IOCONFIG_MIFARE_USERUID_CTR            0x01(0x00c1)*/
          0x02, 
         /*IOCONFIG_MIFARE_DESELECT               0x01(0x00c2)*/
          0x00, 
         /*IOCONFIG_MIFARE_L3_CONF                0x01(0x00c3)*/
          0x80, 
         /*IOCONFIG_MIFARE_L4_CONF                0x01(0x00c4)*/
          0x15, 
         /*IOCONFIG_MIFARE_CONF                   0x01(0x00c5)*/
          0x00, 
#endif

#if  HAL_SWP_SUPPORTED_CONFIG
         /*SWP_TYPEA_ENABLE                       0x01*/
          0x02, 
         /*SWP_TYPEA_UID_LEN                      0x01*/
          0x00, 
         /*SWP_TYPEA_UID                          0x0a*/
          0x10, 0x00, 0x1D, 0x51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
         /*SWP_TYPEA_SAK                          0x01*/
          0x20, 
         /*SWP_TYPEA_ATQA                         0x02*/
          0x04, 0x00, 
         /*SWP_TYPEA_APPDATA_LEN                  0x01*/
          0x03, 
         /*SWP_TYPEA_APPDATA                      0x0f*/
          0x90, 0x12, 0x34, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
         /*SWP_TYPEA_FWI_SFGI                     0x01*/
          0x78, 
         /*SWP_TYPEA_CID_SUPPORT                  0x01*/
          0x01, 
         /*SWP_TYPEA_CLT_SUPPORT                  0x01*/
          0x00, 
         /*SWP_TYPEA_DATARATE_MAX                 0x03*/
          0x00, 0x00, 0x00, 
         /*SWP_TYPEB_ENABLE                       0x01*/
          0x02, 
         /*SWP_TYPEB_PUPI_LEN                     0x01*/
          0x04, 
         /*SWP_TYPEB_PUPI                         0x04*/
          0x00, 0x00, 0x00, 0x01, 
         /*SWP_TYPEB_AFI                          0x01*/
          0x01, 
         /*SWP_TYPEB_ATQB                         0x04*/
          0x00, 0x00, 0x00, 0x70, 
         /*SWP_TYPEB_HIGHT_LAYER_RESPONSE_LEN     0x01*/
          0x00, 
         /*SWP_TYPEB_HIGHT_LAYER_RESPONSE         0x0f*/
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
         /*SWP_TYPEB_DATARATE_MAX                 0x03*/
          0x00, 0x00, 0x01, 
         /*IOCONFIG_SWP_CONFIG                    0x01*/
          0xE3, 
#endif

#if  HAL_CLK_CONFIG
         /*CLK_CONFIG_CB                          0x01(0x00c6)*/
          0x1F, 
         /*CLK_CONFIG_CL                          0x01(0x00c7)*/
          0x1C, 
         /*CLK_CONFIG_CL_COM                      0x01(0x00c8)*/
          0x18, 
         /*CLK_CONFIG_RSA_KEYGEN                  0x01(0x00c9)*/
          0xA0, 
         /*CLK_CONFIG_RSA_OPERATION               0x01(0x00ca)*/
          0xA0, 
         /*CLK_CONFIG_CL_EEPROM_PROGRAM           0x01(0x00cb)*/
          0x1C, 
         /*HAL_CONFIG                             0x01(0x00cc)*/
          0x00, 
#endif

#if  HAL_RESERVE_CONFIG
         /*HAL_RESERVE                            0x03(0x00cd)*/
          0xFF, 0xFF, 0xFF, 
#endif


};

#endif /*CONFIG_IN_BUFFER*/


u8 CONST bootVersion[BOOT_VERSION_SIZE] =
{
	0x53,0x4C,0x4D,0x31,0x37,0x5F,0x45,0x50,0x58,0x5F,0x56,0x31,0x2E,0x30,0x2E,0x30 //SLM17_EPX_V1.0.0
};



