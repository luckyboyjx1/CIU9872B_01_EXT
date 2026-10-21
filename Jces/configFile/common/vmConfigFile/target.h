/*****************************************************************************
 target.h

 config the target information , the target information must sym to the mask information

 w h
 eastcompeace Technology Corporation

 History:
 2007,6,23	.Created
 *****************************************************************************/
//  modify by xieyong@20220107 增加一卡一CRC相关定义

#ifndef __TARGET_H__
#define __TARGET_H__


#include "DataOperation.h"
#include "SysConfigData.h"
#include "profile.h"

//#define 	EEPROM_PARA_DATA_FIELD_START					COS_CONFIG_START
//#define 	EEPROM_PARA_DATA_FIELD_END						(EEPROM_PARA_DATA_FIELD_START + 0xC0)
#if GP__AMENDMENT__C__FOR_MASK
#define     AMDC_CONFIG_DATA_START                          VM_CONFIG_END
#define     AMDC_CONFIG_DATA_1                              AMDC_CONFIG_DATA_START
#define     AMDC_CONFIG_DATA_2                              (AMDC_CONFIG_DATA_START + EECFG_SIZE)
#define     AMDC_CONFIG_DATA_END                            (AMDC_CONFIG_DATA_START + 0x500)

#define     CONFIG_DATA_END                              	(AMDC_CONFIG_DATA_END)

#else

#define     CONFIG_DATA_END                              	(CARDINIT_CONFIG_BASE)

#endif


#if HAL__PLATFORM__SIMULATOR
	#define		PHY_RAM_BASE									((memref)(CHIP_RAM_BASE & 0xffff0000))//((memref)(0x20000000)) //解决编译问题, 将JCVM_RAM_LIMIT替换为CHIP_RAM_LIMIT，JCVM_RAM_BASE为CHIP_RAM_BASE
	#define 	JC_EEPROM_BASE									((memref)0x00000000UL)
	#define 	LOGICCHNBAK_BASEADDR							((memref)&LogicBakData)
	#define 	LOGICCHNBAK_SIZE								0x800

	#define 	SCP_DATA_AREA_START								((memref)&scpData)
	#define     SCP_DATA_AREA_SIZE					        	0x00

	#define     JC_SE_INFO_SATRT                                ((memref)&seInfo)
	#define     JC_SE_INFO_SIZE						        	0x00

	#define     JC_CRC_SATRT                                	 ((memref)(&CRCData))
	#define     JC_CRC_SIZE						        		 0x00

	#define     CODECRC_START						     		((memref)(&ROMSTACK))
	#define     CODECRC_SIZE						         	0x00

	#define     CONFIGUECRC_START								((memref)(&EEPROMSTACK))
	#define		CONFIGUECRC_SIZE								0x600

	#define     USEREEPROMCRC_START						    	((memref)(&EEPROMSTACK[CONFIGUECRC_SIZE]))
	#define     USEREEPROMCRC_SIZE						    	0x00
	#define     BOOTLOADERCRC_START								((memref)(&RAMSTACK))
	#define		BOOTLOADERCRC_SIZE								0x00
#else
	extern void Image$$ER_SCP_DATAAREA$$Base;
#if JAVA_CARD_UICC //modify by jiangxiaohui@20220324 分层
	#define SCP_DATA_AREA_START		((memref)&SCP_DATA_AREA)
#else
	#define SCP_DATA_AREA_START		((memref)&scpData)
#endif
	
	extern void Image$$ER_LOGICCHN_BAK_AREA$$Base;


	#define LOGICCHNBAK_BASEADDR	((u32)&Image$$ER_LOGICCHN_BAK_AREA$$Base)
	#define LOGICCHNBAK_SIZE		0x800
	//#define 	ROMBase 										((memref)&Image$$ER_JC_MASKS$$Base)
	#define		PHY_RAM_BASE									((memref)(0x20000000))
	#define 	JC_EEPROM_BASE									((memref)0x00000000UL)
	extern void Image$$ER_SEINFO_START$$Base;
	#define     JC_SE_INFO_SATRT                                ((memref)&Image$$ER_SEINFO_START$$Base)
	#define     JC_SE_INFO_SIZE						        	EEPAGE_SIZE

	extern void Image$$ER_CRCAREA_START$$Base;
	#define     JC_CRC_SATRT                                	((memref)&Image$$ER_CRCAREA_START$$Base)
	#define     JC_CRC_SIZE						        		 EEPAGE_SIZE

	extern void Image$$ER_ROM1$$Base;
	extern void Image$$HAL_CONFIG_DATA_FILE$$Base;
	extern void Image$$JCVM_CONFIG_DATA_FILE$$Limit;
	extern void Image$$ER_BOOTLOADER_HAL_CONFIG_DATA$$Base;
	extern void Image$$ER_FUNENTRYCONFIG$$Base;
	extern void Image$$ER_FUNBOOTENTRYCONFIG$$Base;
	extern void Image$$ER_FUNBOOTENTRYCONFIG$$Limit;
	
	extern void Image$$ER_STACK_RAM$$Base;
	extern void	Image$$ER_AN_APDUBUFFER_RAM$$Limit;

	extern void Image$$ER_TRANS_DATAAREA$$Base;
	extern void Image$$ER_PATCH_START$$Base;

	#define     CODECRC_START						     	((u32)&Image$$ER_ROM1$$Base)
	#define     CODECRC_SIZE						         (((u32)&Image$$HAL_CONFIG_DATA_FILE$$Base)-CODECRC_START)

	#define     CONFIGUECRC_START							((u32)&Image$$HAL_CONFIG_DATA_FILE$$Base)
	#define		CONFIGUECRC_SIZE							0x600

	#define     USEREEPROMCRC_START						    CONFIGUECRC_START+CONFIGUECRC_SIZE
	#define     USEREEPROMCRC_SIZE						    (((u32)BACK_UP_AREA_START)-(CONFIGUECRC_START+CONFIGUECRC_SIZE))

	#define     BOOTLOADERCRC_START							((u32)&Image$$ER_FUNBOOTENTRYCONFIG$$Base)
	#define		BOOTLOADERCRC_SIZE							(((u32)&Image$$ER_PATCH_START$$Base)-((u32)&Image$$ER_FUNBOOTENTRYCONFIG$$Base))
#endif
//#define     E2P_CONFIG_TOP_ADDRESS                          0x001E7FFFL//((memref)&Image$$ER_JC_MASKS$$Base)

//给jvm用的eeprom区，包括map表和java heap
//对应的Java层E2BASE配置为 E2PBase + 0xF00(E2P_IMAGE_START的大小)
//#define 	E2PBase											(COS_CONFIG_START + CONFIG_DATA_END)
//#define 	E2PSize											((E2P_CONFIG_TOP_ADDRESS) - E2PBase)

#define 	RTR_SEGMENT_SIZE          						(RAMSize - (MAX_TEMPORARY_OBJECTS * TEMP_OBJECT_HEADER_LENGTH) - (dtrSize * logicalChannelNums))//(NVM_READ_U16_NOCACHE(NON_JCRE_RTR_SIZE))	//(RAMSize - (MAX_TEMPORARY_OBJECTS * TEMP_OBJECT_HEADER_LENGTH) - (dtrSize * logicalChannelNums))


/*rom的空间分配策略
	块一：mask
	块二：code		
	块三：start code:	CODE_START	0X8000
  	mask + 空隙 + code + 空隙
*/
//#define 	ROM_AREA_START									((memref)(0x10061000UL))	//芯片的总rom空间的开始


/*大ram buf分配方案,将几个大buf连成一个巨大的buf,使第一个buf的结尾与其它buf复用
buf顺序是：
rsa buf		:尾部复用
bresbuffer	:完全复用
btemp buf	:完全复用
java ram buf   :头部复用
*/

#define 	RSA_RAM_SIZE									480

#define 	B_TEMP_BUF_SIZE									280
#define 	BRESBUFFER_SIZE									(264+8)

#define 	JC_RAM_BASE										((memref)0x20000000)
#define		PHY_RAM_END										0x20008000//ram的物理末地址

#define 	JAVA_RAM_REUSE_SIZE								0X4c4	//设小一点，可加快rsa速度,但是rsaram中的的非 java ram空间要大于RTR_SEGMENT_SIZE大小，以使gc过程中的ram复用能正常进行
#define 	BTEMP_BUF_REUSE_SIZE							B_TEMP_BUF_SIZE
#define 	BRESBUFFER_REUSE_SIZE							(BRESBUFFER_SIZE)
#define 	RSA_RAM_REUSE_SIZE								(JAVA_RAM_REUSE_SIZE + B_TEMP_BUF_SIZE + BRESBUFFER_SIZE)			 //1772

#define 	BIGBUF_SIZE										(RSA_RAM_SIZE)

#define 	RSA_RAM_NON_REUSE_SIZE							(RSA_RAM_SIZE - RSA_RAM_REUSE_SIZE)	//要确保该值不能小于crypto data的大小，当前crypto data区设置为260字节
#define 	BIGBUF_REUSE_OFFSET								RSA_RAM_NON_REUSE_SIZE		

//===========================================================================
//pointer size is needed for word align pointers into buffers
//===================================================== ======================

#define 	EEPAGE_SIZE_192                 				0x10E	//0x100 +0x0E(原子写会多一个头信息)
#define 	EEPROM_PAGE_SIZE               					0x100

#define 	EECELL_SIZE										0x04
#define 	EECELL_MASK										0x0003
#define 	NOR_EECELL_MASK									0xFFFFFFFC	//4 byte aligned
//===========================================================================
//Compile time flag that determines if Locals are pre-initialized to 0xFFFF
//whenever a new stack frame is created, during runtime.
//===========================================================================
#define LOCALS_ARE_FFFF 									0
#define LOCALS_ARE_0000 									0
#define UNINITIALIZED_MEMORY_CELL							0xff

#define STACKSIZE 											800
#define APDU_BUFFER_SIZE  									520
//#define APDU_DATA_SIZE    									256
#define RAM_BUFFER_SIZE	  									RAM_BUFFER_END//增加EUICC需要的变量 modify by xuxiaojun@20220329 移动到global_variable.h
#define SHORT_BUFFER_SIZE 									SHORT_BUFFER_END // modify by xuxiaojun@20220329 移动到global_variable.h
#define CACHE_BUFFER_SIZE 									260

#if !JCVM__RAM__DTR__RTR__NEW
//0xa17 = 2583 减去这80个字节 2503为DTR  RTR使用
#define MAX_TEMPORARY_OBJECTS     							8
#define TEMP_OBJECT_HEADER_LENGTH 							10
#endif

#define TEMPBUFFER_SIZE										24
#define	CARD_CHALLENGE_SIZE									8

#define	VM_CONFIG_START										COS_CONFIG_START
#if (!JAVA_CARD_UICC)
#define E2P_IMAGE_START										(memref)(COS_CONFIG_START + CONFIG_END_ALIGN)
#endif


//===========================================================================
//	physics Addr config
//===========================================================================

#define 		RAMBase   									CHIP_RAM_BASE//start of DTR∷RTR and temporary object space

#define 		RAMSize   									((memref)(CHIP_RAM_LIMIT-CHIP_RAM_BASE))//size of DTR∷RTR and temporary object space


#define			TR_OBJ_TABLE_SIZE							600

#if JCVM__RAM__DTR__RTR__NEW
#if JCVM__VM__RMI
	//0xa17 = 2583 减去这80个字节 2503为DTR  RTR使用
	#define MAX_TEMPORARY_OBJECTS     							8
	#define TEMP_OBJECT_HEADER_LENGTH 							10
#else
	//#define MAX_DTR_RAM_CHANNELS     							(SWP_MAX_LOGICAL_CHANNELS + ISO_MAX_LOGICAL_CHANNELS)
	#define MAX_DTR_RAM_CHANNELS     							(NFC_MAX_LOGICAL_CHANNELS + ISO_MAX_LOGICAL_CHANNELS)
	#define MAX_DTR_RAM_RESERVE 								270

	#define MAX_TEMPORARY_OBJECTS     							MAX_DTR_RAM_CHANNELS
	#define TEMP_OBJECT_HEADER_LENGTH 							MAX_DTR_RAM_RESERVE
#endif
#endif

#define         BOOT_CONFIG_RESERVE_SIZE                    0x00             /*use for more boot config*/
#define         CARD_INIT_PAGE_JUSTIFY_SIZE                 0x00             /*use for CARD INIT 配置开始地址的页对齐 */
#define         FILE_ID_SYS_MAP_SIZE                        0xDD
#define 	    CPLC_DATA_SIZE                              0x30
#define 		HAL_BOOT_CONFIG_END							SYSCONFIG_OFFSET_END

#endif

#define 		APP_END  						        	0x90 //中国移动5G备案需求 80个包（部分包带可执行模块） + 80个实例       中国电信需要load48个包修改
#define 		ON_CARD_PKG_MAX  				    		110  //中国电信需要load48个包修改  (NPACKAGES + 48)，必须小于mask.cfg里的配置值MaxPackageNum？
#define 		ON_CARD_APPLET_PKG_MAX  					80

//#define         PROFILE_PKG_NUM                             0x40//ProfileEntry.java中定义
