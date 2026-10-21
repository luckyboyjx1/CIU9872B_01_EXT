/****************************************************
* Copyright (C), 2008-2015, Eastcompeace. Co., Ltd. 
* this code is auto generate by config gen tool
* build time: 2024-06-24 18:25:32
* build by:   liguiyu
******************************************************/ 
#ifndef CHIPDATACONFIG_H_
#define CHIPDATACONFIG_H_

#include "SysConfigProfile.h"
#include "SysConfigTarget.h"

#define    CHIP_CONFIG							1
#define    CHIP_PATCH_CONFIG					1
#define    CHIP_RESERVE_CONFIG					1

#if CONFIG_IN_BUFFER
	extern u8 CONST chipDataConfig[];
#endif//CONFIG_IN_BUFFER


#define     CHIPDATACONFIG_OFFSET_START          (0x000000d0L)

                                   //EEPROM Address  tag  Length  default_Value
#if  CHIP_CONFIG
#define    CHIP_RFU                  0x00d0        //C011   0x02   FFFF

#define    CHIP_CONFIG_LENGTH        0x0002
#endif

#if  CHIP_PATCH_CONFIG
#define    EE_PATCH_SIZE             0x00d2        //FFFF   0x04   All(00)
#define    EXTERNAL_PATCH_TABLE_FLG  0x00d6        //FFFF   0x01   All(FF)
#define    EXTERNAL_PATCH_TABLE_NUM  0x00d7        //FFFF   0x01   All(FF)
#define    EXTERNAL_PATCH_TABLE_ADDR 0x00d8        //FFFF   0x04   All(FF)
#define    PATCH_FLAG_ENABLE         0x00dc        //FFFF   0x6e   All(FF)
#define    DEFAULT_PATCH_TABLE_ADDR  0x014a        //FFFF   0x60   All(FF)

#define    CHIP_PATCH_CONFIG_LENGTH  0x00d8
#endif

#if  CHIP_RESERVE_CONFIG
#define    CHIP_IO_TYPE              0x01aa        //C015   0x01   02
#define    CHIP_I2C_ADDR             0x01ab        //C016   0x01   51
#define    PATCH_ENKEY               0x01ac        //C017   0x10   9562BBC6D3809EC0DB70C013B6F822EC
#define    CHIP_SPI_MODE             0x01bc        //C018   0x01   03
#define    SYSCLK                    0x01bd        //C019   0x01   50
#define    CHIPHARDWARE              0x01be        //C020   0x01   00
#define    BL_UPADTE_ENMODE          0x01bf        //C023   0x01   01
#define    CHIP_RESET_ENBLE          0x01c0        //C024   0x01   01
#define    CHIP_RESERVE              0x01c1        //FFFF   0x3f   All(FF)

#define    CHIP_RESERVE_CONFIG_LENGTH0x0056
#endif


#define     CHIPDATACONFIG_OFFSET_END          (0x000001ffL)

#endif	    /*CHIPDATACONFIG_H_*/



