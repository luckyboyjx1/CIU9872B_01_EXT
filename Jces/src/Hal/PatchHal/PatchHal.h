#ifndef __PATCH_HAL_H__
#define	__PATCH_HAL_H__

#include "ecpDefine.h"
#include "ecpOperation.h"
#include "ecpReadWrite.h"
//#include "SysConfig.h"
#include "SysConfigProfile.h"
#include "SysConfigData.h"
#include "chipDataConfig.h"



#define PATCH_SECURITY_STATUS_NOT_SATISFIED 0x6982
#define APDU_P2_OFFSET 											3
#define APDU_P3_OFFSET											4
#define PATCH_WRONG_LENGTH									0x6700
#define PATCH_MEMORY_NOT_ENOUGH							0x6A84
#define PATCH_NO_ERROR											0x9000
#define MARK_ARRY_SIZE											4
#define PATCH_EEP_START								        ((u32)&Image$$ER_PATCH_START$$Base) //0x100F9000
#define MAX_PATCH_EEP_SIZE									 0x2800
#define PATCH_EEP_TOP										(PATCH_EEP_START + MAX_PATCH_EEP_SIZE)
#define PATCH_CONDITIONS_NOT_SATISFIED			0x6985
#define APDU_DATA_OFFSET										5
#define APDU_P1_OFFSET											2
#define APDU_HEAD_LEN												5

#define   OFFSET_ADDR_PATCH		0x05			
#define   OFFSET_DATA_PATCH		0x09     
#define 	OFFSET_DATA_PATCH_LIB			0x0B 
#define   OFFSET_PATCH_DATA_ADDR		0x11
#define   OFFSET_PATCH_LENGTH				0x15
#define   OFFSET_PATCH_CRC					0x17

#define PATCH_MEMORY_NOT_ENOUGH				    0x6A84
#define PATCH_DATA_ERROR				    0x6A83
#define	 EE_PATCH_SIZE_LENGTH								0x04
#define  WITH_PATCH_LENGTH                  0x006c

extern void nativeMethod_sepatch_init(void);
extern void nativeMethod_sepatch_download(void);
extern void nativeMethod_sepatch_InstallandRegister(void);
extern void nativeMethod_sepatch_updatelib(void);
//extern void Image$$ER_PATCH_START$$Base;

extern void nativeMethod_decrypto(void);

#if HAL__PATCH

/*===========================================================================*
 * patch Defines:
 *===========================================================================*/
extern  u8	patch_inrom;

#define  EXTERNAL_PATCH_TABLE_EXIST     					0x5A

#define	 EE_PATCH_SIZE_LENGTH								0x04
#define	 EXTERNAL_PATCH_TABLE_FLG_LENGTH					0x01
#define	 EXTERNAL_PATCH_TABLE_NUM_LENGTH					0x01
#define	 EXTERNAL_PATCH_TABLE_ADDR_LENGTH					0x04

	
#define	 PATCH_INDEX_LENGTH									0x02
#define	 PATCH_ADDRESS_LENGTH								0x04

#define	 PATCH_TABLE_INDEX_LENGTH							(PATCH_INDEX_LENGTH + PATCH_ADDRESS_LENGTH)

#define  DEFAULT_PATCH_TABLE_NUM        					16
#define  DEFAULT_PATCH_TABLE_LENGTH							U8_M_U8(DEFAULT_PATCH_TABLE_NUM, PATCH_TABLE_INDEX_LENGTH)


//Patch flag	                                           
#define         PATCH_FLAG_BLOCK01							(COS_CONFIG_START + PATCH_FLAG_ENABLE)  //+ 0x01            FF       
#define 		PATCH_FLAG_001				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x01))
#define 		PATCH_FLAG_002				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x02))
#define 		PATCH_FLAG_003				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x04))
#define 		PATCH_FLAG_004				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x08))
#define 		PATCH_FLAG_005				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x10))
#define 		PATCH_FLAG_006				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x20))
#define 		PATCH_FLAG_007				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x40))
#define 		PATCH_FLAG_008				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK01) & 0x80))
#define         PATCH_FLAG_BLOCK02							(PATCH_FLAG_BLOCK01 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_009				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x01))
#define 		PATCH_FLAG_010				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x02))
#define 		PATCH_FLAG_011				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x04))
#define 		PATCH_FLAG_012				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x08))
#define 		PATCH_FLAG_013				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x10))
#define 		PATCH_FLAG_014				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x20))
#define 		PATCH_FLAG_015				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x40))
#define 		PATCH_FLAG_016				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK02) & 0x80))
#define         PATCH_FLAG_BLOCK03							(PATCH_FLAG_BLOCK02 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_017			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x01))
#define 		PATCH_FLAG_018				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x02))
#define 		PATCH_FLAG_019				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x04))
#define 		PATCH_FLAG_020				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x08))
#define 		PATCH_FLAG_021				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x10))
#define 		PATCH_FLAG_022				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x20))
#define 		PATCH_FLAG_023				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x40))
#define 		PATCH_FLAG_024				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK03) & 0x80))
#define         PATCH_FLAG_BLOCK04							(PATCH_FLAG_BLOCK03 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_025			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x01))
#define 		PATCH_FLAG_026				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x02))
#define 		PATCH_FLAG_027				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x04))
#define 		PATCH_FLAG_028				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x08))
#define 		PATCH_FLAG_029				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x10))
#define 		PATCH_FLAG_030				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x20))
#define 		PATCH_FLAG_031				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x40))
#define 		PATCH_FLAG_032				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK04) & 0x80))
#define         PATCH_FLAG_BLOCK05							(PATCH_FLAG_BLOCK04 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_033			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x01))
#define 		PATCH_FLAG_034				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x02))
#define 		PATCH_FLAG_035				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x04))
#define 		PATCH_FLAG_036				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x08))
#define 		PATCH_FLAG_037				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x10))
#define 		PATCH_FLAG_038				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x20))
#define 		PATCH_FLAG_039				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x40))
#define 		PATCH_FLAG_040				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK05) & 0x80))
#define         PATCH_FLAG_BLOCK06							(PATCH_FLAG_BLOCK05 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_041			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x01))
#define 		PATCH_FLAG_042				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x02))
#define 		PATCH_FLAG_043				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x04))
#define 		PATCH_FLAG_044				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x08))
#define 		PATCH_FLAG_045				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x10))
#define 		PATCH_FLAG_046				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x20))
#define 		PATCH_FLAG_047				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x40))
#define 		PATCH_FLAG_048				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK06) & 0x80))
#define         PATCH_FLAG_BLOCK07							(PATCH_FLAG_BLOCK06 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_049			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x01))
#define 		PATCH_FLAG_050				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x02))
#define 		PATCH_FLAG_051				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x04))
#define 		PATCH_FLAG_052				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x08))
#define 		PATCH_FLAG_053				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x10))
#define 		PATCH_FLAG_054				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x20))
#define 		PATCH_FLAG_055				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x40))
#define 		PATCH_FLAG_056				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK07) & 0x80))
#define         PATCH_FLAG_BLOCK08							(PATCH_FLAG_BLOCK07 + 1)    //+ 0x01            FF       
#define 		PATCH_FLAG_057			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x01))
#define 		PATCH_FLAG_058				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x02))
#define 		PATCH_FLAG_059				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x04))
#define 		PATCH_FLAG_060				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x08))
#define 		PATCH_FLAG_061				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x10))
#define 		PATCH_FLAG_062				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x20))
#define 		PATCH_FLAG_063				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x40))
#define 		PATCH_FLAG_064				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK08) & 0x80))
#define         PATCH_FLAG_BLOCK09							(PATCH_FLAG_BLOCK08 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_065			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x01))
#define 		PATCH_FLAG_066				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x02))
#define 		PATCH_FLAG_067				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x04))
#define 		PATCH_FLAG_068				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x08))
#define 		PATCH_FLAG_069				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x10))
#define 		PATCH_FLAG_070				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x20))
#define 		PATCH_FLAG_071				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x40))
#define 		PATCH_FLAG_072				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK09) & 0x80))
#define         PATCH_FLAG_BLOCK10							(PATCH_FLAG_BLOCK09 + 1)   //+ 0x01            FF       
#define 		PATCH_FLAG_073			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x01))
#define 		PATCH_FLAG_074				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x02))
#define 		PATCH_FLAG_075				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x04))
#define 		PATCH_FLAG_076				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x08))
#define 		PATCH_FLAG_077				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x10))
#define 		PATCH_FLAG_078				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x20))
#define 		PATCH_FLAG_079				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x40))
#define 		PATCH_FLAG_080				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK10) & 0x80))
#define         PATCH_FLAG_BLOCK11							(PATCH_FLAG_BLOCK10 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_081			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x01))
#define 		PATCH_FLAG_082				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x02))
#define 		PATCH_FLAG_083				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x04))
#define 		PATCH_FLAG_084				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x08))
#define 		PATCH_FLAG_085				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x10))
#define 		PATCH_FLAG_086				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x20))
#define 		PATCH_FLAG_087				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x40))
#define 		PATCH_FLAG_088				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK11) & 0x80))
#define         PATCH_FLAG_BLOCK12							(PATCH_FLAG_BLOCK11 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_089			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x01))
#define 		PATCH_FLAG_090				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x02))
#define 		PATCH_FLAG_091				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x04))
#define 		PATCH_FLAG_092				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x08))
#define 		PATCH_FLAG_093				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x10))
#define 		PATCH_FLAG_094				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x20))
#define 		PATCH_FLAG_095				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x40))
#define 		PATCH_FLAG_096				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK12) & 0x80))
#define         PATCH_FLAG_BLOCK13							(PATCH_FLAG_BLOCK12 + 1)    //+ 0x01            FF       
#define 		PATCH_FLAG_097			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x01))
#define 		PATCH_FLAG_098				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x02))
#define 		PATCH_FLAG_099				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x04))
#define 		PATCH_FLAG_100				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x08))
#define 		PATCH_FLAG_101				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x10))
#define 		PATCH_FLAG_102				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x20))
#define 		PATCH_FLAG_103				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x40))
#define 		PATCH_FLAG_104				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK13) & 0x80))
#define         PATCH_FLAG_BLOCK14							(PATCH_FLAG_BLOCK13 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_105			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x01))
#define 		PATCH_FLAG_106				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x02))
#define 		PATCH_FLAG_107				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x04))
#define 		PATCH_FLAG_108				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x08))
#define 		PATCH_FLAG_109				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x10))
#define 		PATCH_FLAG_110				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x20))
#define 		PATCH_FLAG_111				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x40))
#define 		PATCH_FLAG_112  			  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK14) & 0x80))
#define         PATCH_FLAG_BLOCK15							(PATCH_FLAG_BLOCK14 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_113			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x01))
#define 		PATCH_FLAG_114				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x02))
#define 		PATCH_FLAG_115				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x04))
#define 		PATCH_FLAG_116				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x08))
#define 		PATCH_FLAG_117				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x10))
#define 		PATCH_FLAG_118				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x20))
#define 		PATCH_FLAG_119				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x40))
#define 		PATCH_FLAG_120				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK15) & 0x80))
#define         PATCH_FLAG_BLOCK16							(PATCH_FLAG_BLOCK15 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_121			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x01))
#define 		PATCH_FLAG_122				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x02))
#define 		PATCH_FLAG_123				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x04))
#define 		PATCH_FLAG_124				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x08))
#define 		PATCH_FLAG_125				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x10))
#define 		PATCH_FLAG_126				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x20))
#define 		PATCH_FLAG_127				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x40))
#define 		PATCH_FLAG_128				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK16) & 0x80))
#define         PATCH_FLAG_BLOCK17							(PATCH_FLAG_BLOCK16 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_129			  	  				(!((*(u8  *)PATCH_FLAG_BLOCK17) & 0x01))
#define 		PATCH_FLAG_130				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x02))
#define 		PATCH_FLAG_131				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x04))
#define 		PATCH_FLAG_132				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x08))
#define 		PATCH_FLAG_133				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x10))
#define 		PATCH_FLAG_134				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x20))
#define 		PATCH_FLAG_135				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x40))
#define 		PATCH_FLAG_136				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK17) & 0x80))
#define         PATCH_FLAG_BLOCK18							(PATCH_FLAG_BLOCK17+ 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_137			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x01))
#define 		PATCH_FLAG_138				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x02))
#define 		PATCH_FLAG_139				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x04))
#define 		PATCH_FLAG_140				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x08))
#define 		PATCH_FLAG_141				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x10))
#define 		PATCH_FLAG_142				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x20))
#define 		PATCH_FLAG_143				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x40))
#define 		PATCH_FLAG_144				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK18) & 0x80))
#define         PATCH_FLAG_BLOCK19							(PATCH_FLAG_BLOCK18 + 1)    //+ 0x01            FF       
#define 		PATCH_FLAG_145			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x01))
#define 		PATCH_FLAG_146				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x02))
#define 		PATCH_FLAG_147				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x04))
#define 		PATCH_FLAG_148				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x08))
#define 		PATCH_FLAG_149				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x10))
#define 		PATCH_FLAG_150				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x20))
#define 		PATCH_FLAG_151				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x40))
#define 		PATCH_FLAG_152				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK19) & 0x80))
#define         PATCH_FLAG_BLOCK20							(PATCH_FLAG_BLOCK19 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_153			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x01))
#define 		PATCH_FLAG_154				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x02))
#define 		PATCH_FLAG_155				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x04))
#define 		PATCH_FLAG_156				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x08))
#define 		PATCH_FLAG_157				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x10))
#define 		PATCH_FLAG_158				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x20))
#define 		PATCH_FLAG_159				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x40))
#define 		PATCH_FLAG_160				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK20) & 0x80))
#define         PATCH_FLAG_BLOCK21							(PATCH_FLAG_BLOCK20 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_161			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x01))
#define 		PATCH_FLAG_162				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x02))
#define 		PATCH_FLAG_163				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x04))
#define 		PATCH_FLAG_164				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x08))
#define 		PATCH_FLAG_165				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x10))
#define 		PATCH_FLAG_166				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x20))
#define 		PATCH_FLAG_167				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x40))
#define 		PATCH_FLAG_168				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK21) & 0x80))
#define         PATCH_FLAG_BLOCK22							(PATCH_FLAG_BLOCK21 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_169			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x01))
#define 		PATCH_FLAG_170				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x02))
#define 		PATCH_FLAG_171				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x04))
#define 		PATCH_FLAG_172				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x08))
#define 		PATCH_FLAG_173				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x10))
#define 		PATCH_FLAG_174				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x20))
#define 		PATCH_FLAG_175				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x40))
#define 		PATCH_FLAG_176				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK22) & 0x80))
#define         PATCH_FLAG_BLOCK23							(PATCH_FLAG_BLOCK22 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_177			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x01))
#define 		PATCH_FLAG_178				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x02))
#define 		PATCH_FLAG_179				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x04))
#define 		PATCH_FLAG_180				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x08))
#define 		PATCH_FLAG_181				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x10))
#define 		PATCH_FLAG_182				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x20))
#define 		PATCH_FLAG_183				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x40))
#define 		PATCH_FLAG_184				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK23) & 0x80))
#define         PATCH_FLAG_BLOCK24							(PATCH_FLAG_BLOCK23 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_185			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x01))
#define 		PATCH_FLAG_186				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x02))
#define 		PATCH_FLAG_187				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x04))
#define 		PATCH_FLAG_188				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x08))
#define 		PATCH_FLAG_189				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x10))
#define 		PATCH_FLAG_190				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x20))
#define 		PATCH_FLAG_191				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x40))
#define 		PATCH_FLAG_192   	 		  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK24) & 0x80))
#define         PATCH_FLAG_BLOCK25							(PATCH_FLAG_BLOCK24 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_193			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x01))
#define 		PATCH_FLAG_194				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x02))
#define 		PATCH_FLAG_195				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x04))
#define 		PATCH_FLAG_196				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x08))
#define 		PATCH_FLAG_197				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x10))
#define 		PATCH_FLAG_198				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x20))
#define 		PATCH_FLAG_199				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x40))
#define 		PATCH_FLAG_200				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK25) & 0x80))
#define         PATCH_FLAG_BLOCK26							(PATCH_FLAG_BLOCK25 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_201			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x01))
#define 		PATCH_FLAG_202				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x02))
#define 		PATCH_FLAG_203				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x04))
#define 		PATCH_FLAG_204				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x08))
#define 		PATCH_FLAG_205				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x10))
#define 		PATCH_FLAG_206				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x20))
#define 		PATCH_FLAG_207				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x40))
#define 		PATCH_FLAG_208				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK26) & 0x80))
#define         PATCH_FLAG_BLOCK27							(PATCH_FLAG_BLOCK26 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_209			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x01))
#define 		PATCH_FLAG_210				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x02))
#define 		PATCH_FLAG_211				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x04))
#define 		PATCH_FLAG_212				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x08))
#define 		PATCH_FLAG_213				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x10))
#define 		PATCH_FLAG_214				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x20))
#define 		PATCH_FLAG_215  			  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x40))
#define 		PATCH_FLAG_216				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK27) & 0x80))
#define         PATCH_FLAG_BLOCK28							(PATCH_FLAG_BLOCK27 + 1)    //+ 0x01            FF       
#define 		PATCH_FLAG_217			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x01))
#define 		PATCH_FLAG_218				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x02))
#define 		PATCH_FLAG_219				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x04))
#define 		PATCH_FLAG_220				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x08))
#define 		PATCH_FLAG_221				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x10))
#define 		PATCH_FLAG_222				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x20))
#define 		PATCH_FLAG_223				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x40))
#define 		PATCH_FLAG_224				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK28) & 0x80))
#define         PATCH_FLAG_BLOCK29							(PATCH_FLAG_BLOCK28 + 1)     //+ 0x01            FF       
#define 		PATCH_FLAG_225			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x01))
#define 		PATCH_FLAG_226				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x02))
#define 		PATCH_FLAG_227				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x04))
#define 		PATCH_FLAG_228				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x08))
#define 		PATCH_FLAG_229				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x10))
#define 		PATCH_FLAG_230				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x20))
#define 		PATCH_FLAG_231				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x40))
#define 		PATCH_FLAG_232				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK29) & 0x80))
#define         PATCH_FLAG_BLOCK30							(PATCH_FLAG_BLOCK29 + 1)   //+ 0x01            FF       
#define 		PATCH_FLAG_233			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x01))
#define 		PATCH_FLAG_234				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x02))
#define 		PATCH_FLAG_235				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x04))
#define 		PATCH_FLAG_236				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x08))
#define 		PATCH_FLAG_237				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x10))
#define 		PATCH_FLAG_238				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x20))
#define 		PATCH_FLAG_239				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x40))
#define 		PATCH_FLAG_240				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK30) & 0x80))
#define         PATCH_FLAG_BLOCK31							(PATCH_FLAG_BLOCK30 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_241			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x01))
#define 		PATCH_FLAG_242				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x02))
#define 		PATCH_FLAG_243				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x04))
#define 		PATCH_FLAG_244				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x08))
#define 		PATCH_FLAG_245				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x10))
#define 		PATCH_FLAG_246				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x20))
#define 		PATCH_FLAG_247				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x40))
#define 		PATCH_FLAG_248				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK31) & 0x80))
#define         PATCH_FLAG_BLOCK32							(PATCH_FLAG_BLOCK31 + 1)   //+ 0x01            FF       
#define 		PATCH_FLAG_249			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x01))
#define 		PATCH_FLAG_250				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x02))
#define 		PATCH_FLAG_251				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x04))
#define 		PATCH_FLAG_252				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x08))
#define 		PATCH_FLAG_253				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x10))
#define 		PATCH_FLAG_254				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x20))
#define 		PATCH_FLAG_255				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x40))
#define 		PATCH_FLAG_256				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK32) & 0x80))
#define         PATCH_FLAG_BLOCK33							(PATCH_FLAG_BLOCK32 + 1) //+ 0x01            FF       
#define 		PATCH_FLAG_257			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x01))
#define 		PATCH_FLAG_258				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x02))
#define 		PATCH_FLAG_259				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x04))
#define 		PATCH_FLAG_260				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x08))
#define 		PATCH_FLAG_261				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x10))
#define 		PATCH_FLAG_262				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x20))
#define 		PATCH_FLAG_263				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x40))
#define 		PATCH_FLAG_264				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK33) & 0x80))
#define         PATCH_FLAG_BLOCK34							(PATCH_FLAG_BLOCK33 + 1) //+ 0x01            FF       
#define 		PATCH_FLAG_265			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x01))
#define 		PATCH_FLAG_266				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x02))
#define 		PATCH_FLAG_267				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x04))
#define 		PATCH_FLAG_268				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x08))
#define 		PATCH_FLAG_269				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x10))
#define 		PATCH_FLAG_270				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x20))
#define 		PATCH_FLAG_271				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x40))
#define 		PATCH_FLAG_272				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK34) & 0x80))
#define         PATCH_FLAG_BLOCK35							(PATCH_FLAG_BLOCK34 + 1) //+ 0x01            FF       
#define 		PATCH_FLAG_273			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x01))
#define 		PATCH_FLAG_274				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x02))
#define 		PATCH_FLAG_275				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x04))
#define 		PATCH_FLAG_276				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x08))
#define 		PATCH_FLAG_277				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x10))
#define 		PATCH_FLAG_278				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x20))
#define 		PATCH_FLAG_279				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x40))
#define 		PATCH_FLAG_280				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK35) & 0x80))
#define         PATCH_FLAG_BLOCK36							(PATCH_FLAG_BLOCK35 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_281			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x01))
#define 		PATCH_FLAG_282				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x02))
#define 		PATCH_FLAG_283				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x04))
#define 		PATCH_FLAG_284				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x08))
#define 		PATCH_FLAG_285				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x10))
#define 		PATCH_FLAG_286				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x20))
#define 		PATCH_FLAG_287				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x40))
#define 		PATCH_FLAG_288				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK36) & 0x80))
#define         PATCH_FLAG_BLOCK37							(PATCH_FLAG_BLOCK36 + 1) //+ 0x01            FF       
#define 		PATCH_FLAG_289			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x01))
#define 		PATCH_FLAG_290				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x02))
#define 		PATCH_FLAG_291				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x04))
#define 		PATCH_FLAG_292				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x08))
#define 		PATCH_FLAG_293				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x10))
#define 		PATCH_FLAG_294				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x20))
#define 		PATCH_FLAG_295				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x40))
#define 		PATCH_FLAG_296				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK37) & 0x80))
#define         PATCH_FLAG_BLOCK38							(PATCH_FLAG_BLOCK37 +1) //+ 0x01            FF       
#define 		PATCH_FLAG_297			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x01))
#define 		PATCH_FLAG_298				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x02))
#define 		PATCH_FLAG_299				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x04))
#define 		PATCH_FLAG_300				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x08))
#define 		PATCH_FLAG_301				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x10))
#define 		PATCH_FLAG_302				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x20))
#define 		PATCH_FLAG_303				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x40))
#define 		PATCH_FLAG_304				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK38) & 0x80))
#define         PATCH_FLAG_BLOCK39							(PATCH_FLAG_BLOCK38 + 1) //+ 0x01            FF       
#define 		PATCH_FLAG_305			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x01))
#define 		PATCH_FLAG_306				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x02))
#define 		PATCH_FLAG_307				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x04))
#define 		PATCH_FLAG_308				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x08))
#define 		PATCH_FLAG_309				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x10))
#define 		PATCH_FLAG_310				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x20))
#define 		PATCH_FLAG_311				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x40))
#define 		PATCH_FLAG_312				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK39) & 0x80))
#define         PATCH_FLAG_BLOCK40							(PATCH_FLAG_BLOCK39 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_313			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x01))
#define 		PATCH_FLAG_314				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x02))
#define 		PATCH_FLAG_315				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x04))
#define 		PATCH_FLAG_316				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x08))
#define 		PATCH_FLAG_317				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x10))
#define 		PATCH_FLAG_318				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x20))
#define 		PATCH_FLAG_319				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x40))
#define 		PATCH_FLAG_320				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK40) & 0x80))
//EDEP_Patch
#define         PATCH_FLAG_BLOCK41							(PATCH_FLAG_BLOCK40 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_321			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x01))
#define 		PATCH_FLAG_322				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x02))
#define 		PATCH_FLAG_323				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x04))
#define 		PATCH_FLAG_324				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x08))
#define 		PATCH_FLAG_325				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x10))
#define 		PATCH_FLAG_326				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x20))
#define 		PATCH_FLAG_327				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x40))
#define 		PATCH_FLAG_328				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK41) & 0x80))
#define         PATCH_FLAG_BLOCK42							(PATCH_FLAG_BLOCK41 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_329			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x01))
#define 		PATCH_FLAG_330				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x02))
#define 		PATCH_FLAG_331				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x04))
#define 		PATCH_FLAG_332				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x08))
#define 		PATCH_FLAG_333				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x10))
#define 		PATCH_FLAG_334				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x20))
#define 		PATCH_FLAG_335				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x40))
#define 		PATCH_FLAG_336				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK42) & 0x80))
#define         PATCH_FLAG_BLOCK43							(PATCH_FLAG_BLOCK42 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_337			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x01))
#define 		PATCH_FLAG_338				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x02))
#define 		PATCH_FLAG_339				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x04))
#define 		PATCH_FLAG_340				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x08))
#define 		PATCH_FLAG_341				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x10))
#define 		PATCH_FLAG_342				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x20))
#define 		PATCH_FLAG_343				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x40))
#define 		PATCH_FLAG_344				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK43) & 0x80))
#define         PATCH_FLAG_BLOCK44							(PATCH_FLAG_BLOCK43 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_345			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x01))
#define 		PATCH_FLAG_346				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x02))
#define 		PATCH_FLAG_347				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x04))
#define 		PATCH_FLAG_348				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x08))
#define 		PATCH_FLAG_349				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x10))
#define 		PATCH_FLAG_350				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x20))
#define 		PATCH_FLAG_351				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x40))
#define 		PATCH_FLAG_352				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK44) & 0x80))
#define         PATCH_FLAG_BLOCK45							(PATCH_FLAG_BLOCK44 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_353			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x01))
#define 		PATCH_FLAG_354				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x02))
#define 		PATCH_FLAG_355				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x04))
#define 		PATCH_FLAG_356				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x08))
#define 		PATCH_FLAG_357				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x10))
#define 		PATCH_FLAG_358				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x20))
#define 		PATCH_FLAG_359				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x40))
#define 		PATCH_FLAG_360				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK45) & 0x80))
#define         PATCH_FLAG_BLOCK46							(PATCH_FLAG_BLOCK45 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_361			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x01))
#define 		PATCH_FLAG_362				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x02))
#define 		PATCH_FLAG_363				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x04))
#define 		PATCH_FLAG_364				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x08))
#define 		PATCH_FLAG_365				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x10))
#define 		PATCH_FLAG_366				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x20))
#define 		PATCH_FLAG_367				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x40))
#define 		PATCH_FLAG_368				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK46) & 0x80))
#define         PATCH_FLAG_BLOCK47							(PATCH_FLAG_BLOCK46 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_369			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x01))
#define 		PATCH_FLAG_370				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x02))
#define 		PATCH_FLAG_371				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x04))
#define 		PATCH_FLAG_372				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x08))
#define 		PATCH_FLAG_373				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x10))
#define 		PATCH_FLAG_374				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x20))
#define 		PATCH_FLAG_375				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x40))
#define 		PATCH_FLAG_376				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK47) & 0x80))
#define         PATCH_FLAG_BLOCK48							(PATCH_FLAG_BLOCK47 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_377			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x01))
#define 		PATCH_FLAG_378				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x02))
#define 		PATCH_FLAG_379				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x04))
#define 		PATCH_FLAG_380				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x08))
#define 		PATCH_FLAG_381				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x10))
#define 		PATCH_FLAG_382				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x20))
#define 		PATCH_FLAG_383				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x40))
#define 		PATCH_FLAG_384				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK48) & 0x80))
#define         PATCH_FLAG_BLOCK49							(PATCH_FLAG_BLOCK48 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_385			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x01))
#define 		PATCH_FLAG_386				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x02))
#define 		PATCH_FLAG_387				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x04))
#define 		PATCH_FLAG_388				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x08))
#define 		PATCH_FLAG_389				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x10))
#define 		PATCH_FLAG_390				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x20))
#define 		PATCH_FLAG_391				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x40))
#define 		PATCH_FLAG_392				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK49) & 0x80))
#define         PATCH_FLAG_BLOCK50							(PATCH_FLAG_BLOCK49 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_393			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x01))
#define 		PATCH_FLAG_394				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x02))
#define 		PATCH_FLAG_395				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x04))
#define 		PATCH_FLAG_396				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x08))
#define 		PATCH_FLAG_397				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x10))
#define 		PATCH_FLAG_398				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x20))
#define 		PATCH_FLAG_399				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x40))
#define 		PATCH_FLAG_400				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK50) & 0x80))


//PBOC_Patch
#define         PATCH_FLAG_BLOCK51							(PATCH_FLAG_BLOCK50 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_401			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x01))
#define 		PATCH_FLAG_402				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x02))
#define 		PATCH_FLAG_403				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x04))
#define 		PATCH_FLAG_404				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x08))
#define 		PATCH_FLAG_405				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x10))
#define 		PATCH_FLAG_406				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x20))
#define 		PATCH_FLAG_407				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x40))
#define 		PATCH_FLAG_408				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK51) & 0x80))
#define         PATCH_FLAG_BLOCK52							(PATCH_FLAG_BLOCK51 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_409			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x01))
#define 		PATCH_FLAG_410				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x02))
#define 		PATCH_FLAG_411				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x04))
#define 		PATCH_FLAG_412				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x08))
#define 		PATCH_FLAG_413				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x10))
#define 		PATCH_FLAG_414				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x20))
#define 		PATCH_FLAG_415				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x40))
#define 		PATCH_FLAG_416				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK52) & 0x80))
#define         PATCH_FLAG_BLOCK53							(PATCH_FLAG_BLOCK52 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_417			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x01))
#define 		PATCH_FLAG_418				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x02))
#define 		PATCH_FLAG_419				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x04))
#define 		PATCH_FLAG_420				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x08))
#define 		PATCH_FLAG_421				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x10))
#define 		PATCH_FLAG_422				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x20))
#define 		PATCH_FLAG_423				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x40))
#define 		PATCH_FLAG_424				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK53) & 0x80))
#define         PATCH_FLAG_BLOCK54							(PATCH_FLAG_BLOCK53 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_425			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x01))
#define 		PATCH_FLAG_426				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x02))
#define 		PATCH_FLAG_427				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x04))
#define 		PATCH_FLAG_428				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x08))
#define 		PATCH_FLAG_429				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x10))
#define 		PATCH_FLAG_430				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x20))
#define 		PATCH_FLAG_431				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x40))
#define 		PATCH_FLAG_432				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK54) & 0x80))
#define         PATCH_FLAG_BLOCK55							(PATCH_FLAG_BLOCK54 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_433			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x01))
#define 		PATCH_FLAG_434				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x02))
#define 		PATCH_FLAG_435				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x04))
#define 		PATCH_FLAG_436				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x08))
#define 		PATCH_FLAG_437				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x10))
#define 		PATCH_FLAG_438				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x20))
#define 		PATCH_FLAG_439				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x40))
#define 		PATCH_FLAG_440				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK55) & 0x80))
#define         PATCH_FLAG_BLOCK56							(PATCH_FLAG_BLOCK55 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_441			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x01))
#define 		PATCH_FLAG_442				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x02))
#define 		PATCH_FLAG_443				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x04))
#define 		PATCH_FLAG_444				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x08))
#define 		PATCH_FLAG_445				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x10))
#define 		PATCH_FLAG_446				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x20))
#define 		PATCH_FLAG_447				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x40))
#define 		PATCH_FLAG_448				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK56) & 0x80))
#define         PATCH_FLAG_BLOCK57							(PATCH_FLAG_BLOCK56 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_449			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x01))
#define 		PATCH_FLAG_450				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x02))
#define 		PATCH_FLAG_451				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x04))
#define 		PATCH_FLAG_452				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x08))
#define 		PATCH_FLAG_453				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x10))
#define 		PATCH_FLAG_454				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x20))
#define 		PATCH_FLAG_455				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x40))
#define 		PATCH_FLAG_456				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK57) & 0x80))
#define         PATCH_FLAG_BLOCK58							(PATCH_FLAG_BLOCK57 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_457			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x01))
#define 		PATCH_FLAG_458				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x02))
#define 		PATCH_FLAG_459				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x04))
#define 		PATCH_FLAG_460				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x08))
#define 		PATCH_FLAG_461				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x10))
#define 		PATCH_FLAG_462				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x20))
#define 		PATCH_FLAG_463				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x40))
#define 		PATCH_FLAG_464				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK58) & 0x80))
#define         PATCH_FLAG_BLOCK59							(PATCH_FLAG_BLOCK58 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_465			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x01))
#define 		PATCH_FLAG_466				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x02))
#define 		PATCH_FLAG_467				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x04))
#define 		PATCH_FLAG_468				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x08))
#define 		PATCH_FLAG_469				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x10))
#define 		PATCH_FLAG_470				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x20))
#define 		PATCH_FLAG_471				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x40))
#define 		PATCH_FLAG_472				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK59) & 0x80))
#define         PATCH_FLAG_BLOCK60							(PATCH_FLAG_BLOCK59 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_473			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x01))
#define 		PATCH_FLAG_474				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x02))
#define 		PATCH_FLAG_475				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x04))
#define 		PATCH_FLAG_476				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x08))
#define 		PATCH_FLAG_477				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x10))
#define 		PATCH_FLAG_478				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x20))
#define 		PATCH_FLAG_479				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x40))
#define 		PATCH_FLAG_480				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK60) & 0x80))
#define         PATCH_FLAG_BLOCK61							(PATCH_FLAG_BLOCK60 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_481			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x01))
#define 		PATCH_FLAG_482				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x02))
#define 		PATCH_FLAG_483				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x04))
#define 		PATCH_FLAG_484				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x08))
#define 		PATCH_FLAG_485				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x10))
#define 		PATCH_FLAG_486				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x20))
#define 		PATCH_FLAG_487				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x40))
#define 		PATCH_FLAG_488				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK61) & 0x80))
#define         PATCH_FLAG_BLOCK62							(PATCH_FLAG_BLOCK61 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_489			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x01))
#define 		PATCH_FLAG_490				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x02))
#define 		PATCH_FLAG_491				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x04))
#define 		PATCH_FLAG_492				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x08))
#define 		PATCH_FLAG_493				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x10))
#define 		PATCH_FLAG_494				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x20))
#define 		PATCH_FLAG_495				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x40))
#define 		PATCH_FLAG_496				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK62) & 0x80))
#define         PATCH_FLAG_BLOCK63							(PATCH_FLAG_BLOCK62 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_497			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x01))
#define 		PATCH_FLAG_498				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x02))
#define 		PATCH_FLAG_499				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x04))
#define 		PATCH_FLAG_500				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x08))
#define 		PATCH_FLAG_501				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x10))
#define 		PATCH_FLAG_502				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x20))
#define 		PATCH_FLAG_503				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x40))
#define 		PATCH_FLAG_504				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK63) & 0x80))
#define         PATCH_FLAG_BLOCK64							(PATCH_FLAG_BLOCK63 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_505			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x01))
#define 		PATCH_FLAG_506				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x02))
#define 		PATCH_FLAG_507				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x04))
#define 		PATCH_FLAG_508				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x08))
#define 		PATCH_FLAG_509				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x10))
#define 		PATCH_FLAG_510				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x20))
#define 		PATCH_FLAG_511				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x40))
#define 		PATCH_FLAG_512				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK64) & 0x80))
#define         PATCH_FLAG_BLOCK65							(PATCH_FLAG_BLOCK64 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_513			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x01))
#define 		PATCH_FLAG_514				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x02))
#define 		PATCH_FLAG_515				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x04))
#define 		PATCH_FLAG_516				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x08))
#define 		PATCH_FLAG_517				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x10))
#define 		PATCH_FLAG_518				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x20))
#define 		PATCH_FLAG_519				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x40))
#define 		PATCH_FLAG_520				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK65) & 0x80))
#define         PATCH_FLAG_BLOCK66							(PATCH_FLAG_BLOCK65 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_521			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x01))
#define 		PATCH_FLAG_522				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x02))
#define 		PATCH_FLAG_523				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x04))
#define 		PATCH_FLAG_524				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x08))
#define 		PATCH_FLAG_525				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x10))
#define 		PATCH_FLAG_526				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x20))
#define 		PATCH_FLAG_527				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x40))
#define 		PATCH_FLAG_528				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK66) & 0x80))
#define         PATCH_FLAG_BLOCK67							(PATCH_FLAG_BLOCK66 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_529			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x01))
#define 		PATCH_FLAG_530				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x02))
#define 		PATCH_FLAG_531				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x04))
#define 		PATCH_FLAG_532				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x08))
#define 		PATCH_FLAG_533				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x10))
#define 		PATCH_FLAG_534				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x20))
#define 		PATCH_FLAG_535				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x40))
#define 		PATCH_FLAG_536				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK67) & 0x80))
#define         PATCH_FLAG_BLOCK68							(PATCH_FLAG_BLOCK67 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_537			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x01))
#define 		PATCH_FLAG_538				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x02))
#define 		PATCH_FLAG_539				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x04))
#define 		PATCH_FLAG_540				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x08))
#define 		PATCH_FLAG_541				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x10))
#define 		PATCH_FLAG_542				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x20))
#define 		PATCH_FLAG_543				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x40))
#define 		PATCH_FLAG_544				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK68) & 0x80))
#define         PATCH_FLAG_BLOCK69							(PATCH_FLAG_BLOCK68 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_545			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x01))
#define 		PATCH_FLAG_546				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x02))
#define 		PATCH_FLAG_547				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x04))
#define 		PATCH_FLAG_548				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x08))
#define 		PATCH_FLAG_549				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x10))
#define 		PATCH_FLAG_550				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x20))
#define 		PATCH_FLAG_551				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x40))
#define 		PATCH_FLAG_552				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK69) & 0x80))
#define         PATCH_FLAG_BLOCK70							(PATCH_FLAG_BLOCK69 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_553			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x01))
#define 		PATCH_FLAG_554				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x02))
#define 		PATCH_FLAG_555				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x04))
#define 		PATCH_FLAG_556				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x08))
#define 		PATCH_FLAG_557				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x10))
#define 		PATCH_FLAG_558				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x20))
#define 		PATCH_FLAG_559				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x40))
#define 		PATCH_FLAG_560				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK70) & 0x80))
#define         PATCH_FLAG_BLOCK71							(PATCH_FLAG_BLOCK70 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_561			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x01))
#define 		PATCH_FLAG_562				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x02))
#define 		PATCH_FLAG_563				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x04))
#define 		PATCH_FLAG_564				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x08))
#define 		PATCH_FLAG_565				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x10))
#define 		PATCH_FLAG_566				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x20))
#define 		PATCH_FLAG_567				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x40))
#define 		PATCH_FLAG_568				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK71) & 0x80))
#define         PATCH_FLAG_BLOCK72							(PATCH_FLAG_BLOCK71 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_569			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x01))
#define 		PATCH_FLAG_570				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x02))
#define 		PATCH_FLAG_571				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x04))
#define 		PATCH_FLAG_572				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x08))
#define 		PATCH_FLAG_573				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x10))
#define 		PATCH_FLAG_574				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x20))
#define 		PATCH_FLAG_575				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x40))
#define 		PATCH_FLAG_576				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK72) & 0x80))
#define         PATCH_FLAG_BLOCK73							(PATCH_FLAG_BLOCK72 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_577			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x01))
#define 		PATCH_FLAG_578				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x02))
#define 		PATCH_FLAG_579				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x04))
#define 		PATCH_FLAG_580				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x08))
#define 		PATCH_FLAG_581				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x10))
#define 		PATCH_FLAG_582				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x20))
#define 		PATCH_FLAG_583				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x40))
#define 		PATCH_FLAG_584				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK73) & 0x80))
#define         PATCH_FLAG_BLOCK74							(PATCH_FLAG_BLOCK73 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_585			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x01))
#define 		PATCH_FLAG_586				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x02))
#define 		PATCH_FLAG_587				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x04))
#define 		PATCH_FLAG_588				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x08))
#define 		PATCH_FLAG_589				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x10))
#define 		PATCH_FLAG_590				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x20))
#define 		PATCH_FLAG_591				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x40))
#define 		PATCH_FLAG_592				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK74) & 0x80))
#define         PATCH_FLAG_BLOCK75							(PATCH_FLAG_BLOCK74 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_593			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x01))
#define 		PATCH_FLAG_594				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x02))
#define 		PATCH_FLAG_595				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x04))
#define 		PATCH_FLAG_596				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x08))
#define 		PATCH_FLAG_597				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x10))
#define 		PATCH_FLAG_598				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x20))
#define 		PATCH_FLAG_599				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x40))
#define 		PATCH_FLAG_600				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK75) & 0x80))
#define         PATCH_FLAG_BLOCK76							(PATCH_FLAG_BLOCK75 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_601			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x01))
#define 		PATCH_FLAG_602				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x02))
#define 		PATCH_FLAG_603				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x04))
#define 		PATCH_FLAG_604				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x08))
#define 		PATCH_FLAG_605				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x10))
#define 		PATCH_FLAG_606				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x20))
#define 		PATCH_FLAG_607				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x40))
#define 		PATCH_FLAG_608				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK76) & 0x80))
#define         PATCH_FLAG_BLOCK77							(PATCH_FLAG_BLOCK76 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_609			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x01))
#define 		PATCH_FLAG_610				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x02))
#define 		PATCH_FLAG_611				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x04))
#define 		PATCH_FLAG_612				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x08))
#define 		PATCH_FLAG_613				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x10))
#define 		PATCH_FLAG_614				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x20))
#define 		PATCH_FLAG_615				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x40))
#define 		PATCH_FLAG_616				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK77) & 0x80))
#define         PATCH_FLAG_BLOCK78							(PATCH_FLAG_BLOCK77 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_617			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x01))
#define 		PATCH_FLAG_618				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x02))
#define 		PATCH_FLAG_619				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x04))
#define 		PATCH_FLAG_620				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x08))
#define 		PATCH_FLAG_621				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x10))
#define 		PATCH_FLAG_622				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x20))
#define 		PATCH_FLAG_623				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x40))
#define 		PATCH_FLAG_624				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK78) & 0x80))
#define         PATCH_FLAG_BLOCK79							(PATCH_FLAG_BLOCK78 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_625			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x01))
#define 		PATCH_FLAG_626				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x02))
#define 		PATCH_FLAG_627				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x04))
#define 		PATCH_FLAG_628				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x08))
#define 		PATCH_FLAG_629				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x10))
#define 		PATCH_FLAG_630				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x20))
#define 		PATCH_FLAG_631				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x40))
#define 		PATCH_FLAG_632				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK79) & 0x80))
#define         PATCH_FLAG_BLOCK80							(PATCH_FLAG_BLOCK79 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_633			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x01))
#define 		PATCH_FLAG_634				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x02))
#define 		PATCH_FLAG_635				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x04))
#define 		PATCH_FLAG_636				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x08))
#define 		PATCH_FLAG_637				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x10))
#define 		PATCH_FLAG_638				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x20))
#define 		PATCH_FLAG_639				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x40))
#define 		PATCH_FLAG_640				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK80) & 0x80))
#define         PATCH_FLAG_BLOCK81							(PATCH_FLAG_BLOCK80 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_641			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x01))
#define 		PATCH_FLAG_642				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x02))
#define 		PATCH_FLAG_643				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x04))
#define 		PATCH_FLAG_644				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x08))
#define 		PATCH_FLAG_645				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x10))
#define 		PATCH_FLAG_646				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x20))
#define 		PATCH_FLAG_647				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x40))
#define 		PATCH_FLAG_648				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK81) & 0x80))
#define         PATCH_FLAG_BLOCK82							(PATCH_FLAG_BLOCK81 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_649			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x01))
#define 		PATCH_FLAG_650				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x02))
#define 		PATCH_FLAG_651				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x04))
#define 		PATCH_FLAG_652				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x08))
#define 		PATCH_FLAG_653				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x10))
#define 		PATCH_FLAG_654				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x20))
#define 		PATCH_FLAG_655				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x40))
#define 		PATCH_FLAG_656				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK82) & 0x80))
#define         PATCH_FLAG_BLOCK83							(PATCH_FLAG_BLOCK82 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_657			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x01))
#define 		PATCH_FLAG_658				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x02))
#define 		PATCH_FLAG_659				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x04))
#define 		PATCH_FLAG_660				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x08))
#define 		PATCH_FLAG_661				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x10))
#define 		PATCH_FLAG_662				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x20))
#define 		PATCH_FLAG_663				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x40))
#define 		PATCH_FLAG_664				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK83) & 0x80))
#define         PATCH_FLAG_BLOCK84							(PATCH_FLAG_BLOCK83 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_665			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x01))
#define 		PATCH_FLAG_666				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x02))
#define 		PATCH_FLAG_667				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x04))
#define 		PATCH_FLAG_668				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x08))
#define 		PATCH_FLAG_669				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x10))
#define 		PATCH_FLAG_670				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x20))
#define 		PATCH_FLAG_671				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x40))
#define 		PATCH_FLAG_672				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK84) & 0x80))
#define         PATCH_FLAG_BLOCK85							(PATCH_FLAG_BLOCK84 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_673			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x01))
#define 		PATCH_FLAG_674				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x02))
#define 		PATCH_FLAG_675				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x04))
#define 		PATCH_FLAG_676				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x08))
#define 		PATCH_FLAG_677				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x10))
#define 		PATCH_FLAG_678				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x20))
#define 		PATCH_FLAG_679				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x40))
#define 		PATCH_FLAG_680				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK85) & 0x80))
#define         PATCH_FLAG_BLOCK86							(PATCH_FLAG_BLOCK85 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_681			  	  				(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x01))
#define 		PATCH_FLAG_682				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x02))
#define 		PATCH_FLAG_683				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x04))
#define 		PATCH_FLAG_684				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x08))
#define 		PATCH_FLAG_685				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x10))
#define 		PATCH_FLAG_686				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x20))
#define 		PATCH_FLAG_687				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x40))
#define 		PATCH_FLAG_688				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK86) & 0x80))


#define         PATCH_FLAG_BLOCK87							(PATCH_FLAG_BLOCK86 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_689				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x01))
#define 		PATCH_FLAG_690				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x02))
#define 		PATCH_FLAG_691				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x04))
#define 		PATCH_FLAG_692				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x08))
#define 		PATCH_FLAG_693				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x10))
#define 		PATCH_FLAG_694				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x20))
#define 		PATCH_FLAG_695				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x40))
#define 		PATCH_FLAG_696				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK87) & 0x80))
#define         PATCH_FLAG_BLOCK88							(PATCH_FLAG_BLOCK87 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_697				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x01))
#define 		PATCH_FLAG_698				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x02))
#define 		PATCH_FLAG_699				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x04))
#define 		PATCH_FLAG_700				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x08))
#define 		PATCH_FLAG_701				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x10))
#define 		PATCH_FLAG_702				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x20))
#define 		PATCH_FLAG_703				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x40))
#define 		PATCH_FLAG_704				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK88) & 0x80))
#define         PATCH_FLAG_BLOCK89							(PATCH_FLAG_BLOCK88 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_705				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x01))
#define 		PATCH_FLAG_706				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x02))
#define 		PATCH_FLAG_707				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x04))
#define 		PATCH_FLAG_708				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x08))
#define 		PATCH_FLAG_709				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x10))
#define 		PATCH_FLAG_710				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x20))
#define 		PATCH_FLAG_711				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x40))
#define 		PATCH_FLAG_712				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK89) & 0x80))
#define         PATCH_FLAG_BLOCK90							(PATCH_FLAG_BLOCK89 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_713				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x01))
#define 		PATCH_FLAG_714				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x02))
#define 		PATCH_FLAG_715				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x04))
#define 		PATCH_FLAG_716				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x08))
#define 		PATCH_FLAG_717				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x10))
#define 		PATCH_FLAG_718				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x20))
#define 		PATCH_FLAG_719				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x40))
#define 		PATCH_FLAG_720				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK90) & 0x80))
#define         PATCH_FLAG_BLOCK91							(PATCH_FLAG_BLOCK90 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_721				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x01))
#define 		PATCH_FLAG_722				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x02))
#define 		PATCH_FLAG_723				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x04))
#define 		PATCH_FLAG_724				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x08))
#define 		PATCH_FLAG_725				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x10))
#define 		PATCH_FLAG_726				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x20))
#define 		PATCH_FLAG_727				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x40))
#define 		PATCH_FLAG_728				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK91) & 0x80))
#define         PATCH_FLAG_BLOCK92							(PATCH_FLAG_BLOCK91 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_729				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x01))
#define 		PATCH_FLAG_730				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x02))
#define 		PATCH_FLAG_731				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x04))
#define 		PATCH_FLAG_732				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x08))
#define 		PATCH_FLAG_733				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x10))
#define 		PATCH_FLAG_734				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x20))
#define 		PATCH_FLAG_735				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x40))
#define 		PATCH_FLAG_736				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK92) & 0x80))
#define         PATCH_FLAG_BLOCK93							(PATCH_FLAG_BLOCK92 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_737				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x01))
#define 		PATCH_FLAG_738				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x02))
#define 		PATCH_FLAG_739				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x04))
#define 		PATCH_FLAG_740				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x08))
#define 		PATCH_FLAG_741				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x10))
#define 		PATCH_FLAG_742				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x20))
#define 		PATCH_FLAG_743				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x40))
#define 		PATCH_FLAG_744				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK93) & 0x80))
#define         PATCH_FLAG_BLOCK94							(PATCH_FLAG_BLOCK93 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_745				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x01))
#define 		PATCH_FLAG_746				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x02))
#define 		PATCH_FLAG_747				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x04))
#define 		PATCH_FLAG_748				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x08))
#define 		PATCH_FLAG_749				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x10))
#define 		PATCH_FLAG_750				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x20))
#define 		PATCH_FLAG_751				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x40))
#define 		PATCH_FLAG_752				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK94) & 0x80))
#define         PATCH_FLAG_BLOCK95							(PATCH_FLAG_BLOCK94 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_753				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x01))
#define 		PATCH_FLAG_754				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x02))
#define 		PATCH_FLAG_755				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x04))
#define 		PATCH_FLAG_756				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x08))
#define 		PATCH_FLAG_757				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x10))
#define 		PATCH_FLAG_758				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x20))
#define 		PATCH_FLAG_759				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x40))
#define 		PATCH_FLAG_760				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK95) & 0x80))
#define         PATCH_FLAG_BLOCK96							(PATCH_FLAG_BLOCK95 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_761				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x01))
#define 		PATCH_FLAG_762				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x02))
#define 		PATCH_FLAG_763				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x04))
#define 		PATCH_FLAG_764				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x08))
#define 		PATCH_FLAG_765				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x10))
#define 		PATCH_FLAG_766				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x20))
#define 		PATCH_FLAG_767				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x40))
#define 		PATCH_FLAG_768				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK96) & 0x80))
#define         PATCH_FLAG_BLOCK97							(PATCH_FLAG_BLOCK96 + 1)  //+ 0x01            FF       
#define 		PATCH_FLAG_769				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x01))
#define 		PATCH_FLAG_770				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x02))
#define 		PATCH_FLAG_771				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x04))
#define 		PATCH_FLAG_772				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x08))
#define 		PATCH_FLAG_773				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x10))
#define 		PATCH_FLAG_774				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x20))
#define 		PATCH_FLAG_775				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x40))
#define 		PATCH_FLAG_776				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK97) & 0x80))
#define         PATCH_FLAG_BLOCK98							(PATCH_FLAG_BLOCK97 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_777				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x01))
#define 		PATCH_FLAG_778				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x02))
#define 		PATCH_FLAG_779				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x04))
#define 		PATCH_FLAG_780				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x08))
#define 		PATCH_FLAG_781				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x10))
#define 		PATCH_FLAG_782				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x20))
#define 		PATCH_FLAG_783				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x40))
#define 		PATCH_FLAG_784				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK98) & 0x80))
#define         PATCH_FLAG_BLOCK99							(PATCH_FLAG_BLOCK98 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_785				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x01))
#define 		PATCH_FLAG_786				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x02))
#define 		PATCH_FLAG_787				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x04))
#define 		PATCH_FLAG_788				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x08))
#define 		PATCH_FLAG_789				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x10))
#define 		PATCH_FLAG_790				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x20))
#define 		PATCH_FLAG_791				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x40))
#define 		PATCH_FLAG_792				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK99) & 0x80))
#define         PATCH_FLAG_BLOCK100							(PATCH_FLAG_BLOCK99 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_793				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x01))
#define 		PATCH_FLAG_794				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x02))
#define 		PATCH_FLAG_795				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x04))
#define 		PATCH_FLAG_796				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x08))
#define 		PATCH_FLAG_797				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x10))
#define 		PATCH_FLAG_798				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x20))
#define 		PATCH_FLAG_799				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x40))
#define 		PATCH_FLAG_800				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK100) & 0x80))
#define         PATCH_FLAG_BLOCK101							(PATCH_FLAG_BLOCK100 + 1)  //+ 0x01            FF  
#define 		PATCH_FLAG_801				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x01))
#define 		PATCH_FLAG_802				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x02))
#define 		PATCH_FLAG_803				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x04))
#define 		PATCH_FLAG_804				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x08))
#define 		PATCH_FLAG_805				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x10))
#define 		PATCH_FLAG_806				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x20))
#define 		PATCH_FLAG_807				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x40))
#define 		PATCH_FLAG_808				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK101) & 0x80))
#define         PATCH_FLAG_BLOCK102							(PATCH_FLAG_BLOCK101 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_809				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x01))
#define 		PATCH_FLAG_810				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x02))
#define 		PATCH_FLAG_811				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x04))
#define 		PATCH_FLAG_812				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x08))
#define 		PATCH_FLAG_813				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x10))
#define 		PATCH_FLAG_814				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x20))
#define 		PATCH_FLAG_815				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x40))
#define 		PATCH_FLAG_816				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK102) & 0x80))
#define         PATCH_FLAG_BLOCK103							(PATCH_FLAG_BLOCK102 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_817				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x01))
#define 		PATCH_FLAG_818				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x02))
#define 		PATCH_FLAG_819				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x04))
#define 		PATCH_FLAG_820				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x08))
#define 		PATCH_FLAG_821				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x10))
#define 		PATCH_FLAG_822				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x20))
#define 		PATCH_FLAG_823				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x40))
#define 		PATCH_FLAG_824				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK103) & 0x80))
#define         PATCH_FLAG_BLOCK104							(PATCH_FLAG_BLOCK103 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_825				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x01))
#define 		PATCH_FLAG_826				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x02))
#define 		PATCH_FLAG_827				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x04))
#define 		PATCH_FLAG_828				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x08))
#define 		PATCH_FLAG_829				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x10))
#define 		PATCH_FLAG_830				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x20))
#define 		PATCH_FLAG_831				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x40))
#define 		PATCH_FLAG_832				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK104) & 0x80))
#define         PATCH_FLAG_BLOCK105							(PATCH_FLAG_BLOCK104 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_833				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x01))
#define 		PATCH_FLAG_834				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x02))
#define 		PATCH_FLAG_835				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x04))
#define 		PATCH_FLAG_836				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x08))
#define 		PATCH_FLAG_837				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x10))
#define 		PATCH_FLAG_838				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x20))
#define 		PATCH_FLAG_839				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x40))
#define 		PATCH_FLAG_840				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK105) & 0x80))
#define         PATCH_FLAG_BLOCK106							(PATCH_FLAG_BLOCK105 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_841				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x01))
#define 		PATCH_FLAG_842				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x02))
#define 		PATCH_FLAG_843				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x04))
#define 		PATCH_FLAG_844				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x08))
#define 		PATCH_FLAG_845				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x10))
#define 		PATCH_FLAG_846				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x20))
#define 		PATCH_FLAG_847				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x40))
#define 		PATCH_FLAG_848				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK106) & 0x80))
#define         PATCH_FLAG_BLOCK107							(PATCH_FLAG_BLOCK106 + 1)  //+ 0x01            FF   
#define 		PATCH_FLAG_849				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x01))
#define 		PATCH_FLAG_850				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x02))
#define 		PATCH_FLAG_851				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x04))
#define 		PATCH_FLAG_852				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x08))
#define 		PATCH_FLAG_853				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x10))
#define 		PATCH_FLAG_854				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x20))
#define 		PATCH_FLAG_855				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x40))
#define 		PATCH_FLAG_856				  	  			(!((*(u8 FAR *)PATCH_FLAG_BLOCK107) & 0x80))

extern void PatchHandle(u16 flag);

#define PATCH_METHOD(flag1, flag2) 						\
//if(true == flag1)      									\
//{		                    							\
//	PatchHandle(flag2); 								\
//}           

#else

#define PATCH_METHOD(flag1, flag2)

#endif //  PATCH_OS_ON_OFF

extern  void ecpPatch_initBeforAts(void);
extern  void ecpPatch_initAfterAts(void);
extern  void ecpPatch_BackupOperation(PX8 tempData1,PX8 tempData2);
extern  void ecpPatch_RecoveryOperation(u16 tempData1,u16 tempData2);
/***************Patch for Boot.c*************************/
    #define PATCH_MAIN_001()                                       PATCH_METHOD(PATCH_FLAG_001, (u16)1)  
    #define PATCH_BOOT_001()                                       PATCH_METHOD(PATCH_FLAG_002, (u16)2)  
    #define PATCH_BOOT_002()                                       PATCH_METHOD(PATCH_FLAG_003, (u16)3)  
    #define PATCH_BOOT_003()                                       PATCH_METHOD(PATCH_FLAG_004, (u16)4)  
    #define PATCH_BOOT_004()                                       PATCH_METHOD(PATCH_FLAG_005, (u16)5)  
    #define PATCH_BOOT_005()                                       PATCH_METHOD(PATCH_FLAG_006, (u16)6)  
    #define PATCH_BOOT_006()                                       PATCH_METHOD(PATCH_FLAG_007, (u16)7)  
    #define PATCH_BOOT_007()                                       PATCH_METHOD(PATCH_FLAG_008, (u16)8)  
    #define PATCH_BOOT_008()                                       PATCH_METHOD(PATCH_FLAG_009, (u16)9)  
    #define PATCH_BOOT_009()                                       PATCH_METHOD(PATCH_FLAG_010, (u16)10) 
    #define PATCH_BOOT_010()                                       PATCH_METHOD(PATCH_FLAG_011, (u16)11) 
    #define PATCH_BOOT_011()                                       PATCH_METHOD(PATCH_FLAG_012, (u16)12) 
    #define PATCH_BOOT_012()                                       PATCH_METHOD(PATCH_FLAG_013, (u16)13) 
    #define PATCH_BOOT_013()                                       PATCH_METHOD(PATCH_FLAG_014, (u16)14) 
    #define PATCH_BOOT_014()                                       PATCH_METHOD(PATCH_FLAG_015, (u16)15) 
    #define PATCH_BOOT_015()                                       PATCH_METHOD(PATCH_FLAG_016, (u16)16) 
    #define PATCH_BOOT_016()                                       PATCH_METHOD(PATCH_FLAG_017, (u16)17) 
    #define PATCH_BOOT_017()                                       PATCH_METHOD(PATCH_FLAG_018, (u16)18) 
    #define PATCH_BOOT_018()                                       PATCH_METHOD(PATCH_FLAG_019, (u16)19) 
    #define PATCH_BOOT_019()                                       PATCH_METHOD(PATCH_FLAG_020, (u16)20) 
    #define PATCH_BOOT_020()                                       PATCH_METHOD(PATCH_FLAG_021, (u16)21) 
    #define PATCH_BOOT_021()                                       PATCH_METHOD(PATCH_FLAG_022, (u16)22) 
    #define PATCH_BOOT_022()                                       PATCH_METHOD(PATCH_FLAG_023, (u16)23) 
    #define PATCH_BOOT_023()                                       PATCH_METHOD(PATCH_FLAG_024, (u16)24) 
	
	
/***************Patch for Boot_APDU--------->7816Channel.c	*************************/
    #define PATCH_BOOT_APDU_7816CHANNEL_001()                      PATCH_METHOD(PATCH_FLAG_025, (u16)25) 
	
/***************Patch for Boot_APDU--------->7816Util.c			*************************/
    #define PATCH_BOOT_APDU_7816UTIL_001()                         PATCH_METHOD(PATCH_FLAG_026, (u16)26) 
    #define PATCH_BOOT_APDU_7816UTIL_002()                         PATCH_METHOD(PATCH_FLAG_027, (u16)27) 
    #define PATCH_BOOT_APDU_7816UTIL_003()                         PATCH_METHOD(PATCH_FLAG_028, (u16)28) 
    #define PATCH_BOOT_APDU_7816UTIL_004()                         PATCH_METHOD(PATCH_FLAG_029, (u16)29) 
    #define PATCH_BOOT_APDU_7816UTIL_005()                         PATCH_METHOD(PATCH_FLAG_030, (u16)30) 
    #define PATCH_BOOT_APDU_7816UTIL_006()                         PATCH_METHOD(PATCH_FLAG_031, (u16)31) 
					
/***************Patch for Boot_APDU--------->Io7816T0.c			*************************/
    #define PATCH_BOOT_APDU_IO7816T0_001()                         PATCH_METHOD(PATCH_FLAG_032, (u16)32) 
    #define PATCH_BOOT_APDU_IO7816T0_002()                         PATCH_METHOD(PATCH_FLAG_033, (u16)33) 
    #define PATCH_BOOT_APDU_IO7816T0_003()                         PATCH_METHOD(PATCH_FLAG_034, (u16)34) 
    #define PATCH_BOOT_APDU_IO7816T0_004()                         PATCH_METHOD(PATCH_FLAG_035, (u16)35) 
    #define PATCH_BOOT_APDU_IO7816T0_005()                         PATCH_METHOD(PATCH_FLAG_036, (u16)36) 
    #define PATCH_BOOT_APDU_IO7816T0_006()                         PATCH_METHOD(PATCH_FLAG_037, (u16)37) 
    #define PATCH_BOOT_APDU_IO7816T0_007()                         PATCH_METHOD(PATCH_FLAG_038, (u16)38) 
    #define PATCH_BOOT_APDU_IO7816T0_008()                         PATCH_METHOD(PATCH_FLAG_039, (u16)39) 
	
/***************Patch for Boot_APDU--------->Io7816T1.c			*************************/	
    #define PATCH_BOOT_APDU_IO7816T1_001()                         PATCH_METHOD(PATCH_FLAG_040, (u16)40) 
    #define PATCH_BOOT_APDU_IO7816T1_002()                         PATCH_METHOD(PATCH_FLAG_041, (u16)41) 
    #define PATCH_BOOT_APDU_IO7816T1_003()                         PATCH_METHOD(PATCH_FLAG_042, (u16)42) 
    #define PATCH_BOOT_APDU_IO7816T1_004()                         PATCH_METHOD(PATCH_FLAG_043, (u16)43) 
    #define PATCH_BOOT_APDU_IO7816T1_005()                         PATCH_METHOD(PATCH_FLAG_044, (u16)44) 
    #define PATCH_BOOT_APDU_IO7816T1_006()                         PATCH_METHOD(PATCH_FLAG_045, (u16)45) 
    #define PATCH_BOOT_APDU_IO7816T1_007()                         PATCH_METHOD(PATCH_FLAG_046, (u16)46) 
    #define PATCH_BOOT_APDU_IO7816T1_008()                         PATCH_METHOD(PATCH_FLAG_047, (u16)47) 
    #define PATCH_BOOT_APDU_IO7816T1_009()                         PATCH_METHOD(PATCH_FLAG_048, (u16)48) 
    #define PATCH_BOOT_APDU_IO7816T1_010()                         PATCH_METHOD(PATCH_FLAG_049, (u16)49) 
    #define PATCH_BOOT_APDU_IO7816T1_011()                         PATCH_METHOD(PATCH_FLAG_050, (u16)50) 

/***************Patch for Boot_APDU--------->Io14443TaL3.c	*************************/		
    #define PATCH_BOOT_APDU_IO14443TAL3_001()                      PATCH_METHOD(PATCH_FLAG_051, (u16)51) 
    #define PATCH_BOOT_APDU_IO14443TAL3_002()                      PATCH_METHOD(PATCH_FLAG_052, (u16)52) 
    #define PATCH_BOOT_APDU_IO14443TAL3_003()                      PATCH_METHOD(PATCH_FLAG_053, (u16)53) 
    #define PATCH_BOOT_APDU_IO14443TAL3_004()                      PATCH_METHOD(PATCH_FLAG_054, (u16)54) 
    #define PATCH_BOOT_APDU_IO14443TAL3_005()                      PATCH_METHOD(PATCH_FLAG_055, (u16)55) 
    #define PATCH_BOOT_APDU_IO14443TAL3_006()                      PATCH_METHOD(PATCH_FLAG_056, (u16)56) 
    #define PATCH_BOOT_APDU_IO14443TAL3_007()                      PATCH_METHOD(PATCH_FLAG_057, (u16)57) 
    #define PATCH_BOOT_APDU_IO14443TAL3_008()                      PATCH_METHOD(PATCH_FLAG_058, (u16)58) 
    #define PATCH_BOOT_APDU_IO14443TAL3_009()                      PATCH_METHOD(PATCH_FLAG_059, (u16)59) 
    #define PATCH_BOOT_APDU_IO14443TAL3_010()                      PATCH_METHOD(PATCH_FLAG_060, (u16)60) 
    #define PATCH_BOOT_APDU_IO14443TAL3_011()                      PATCH_METHOD(PATCH_FLAG_061, (u16)61) 
    #define PATCH_BOOT_APDU_IO14443TAL3_012()                      PATCH_METHOD(PATCH_FLAG_062, (u16)62) 
    #define PATCH_BOOT_APDU_IO14443TAL3_013()                      PATCH_METHOD(PATCH_FLAG_063, (u16)63) 
    #define PATCH_BOOT_APDU_IO14443TAL3_014()                      PATCH_METHOD(PATCH_FLAG_064, (u16)64) 
    #define PATCH_BOOT_APDU_IO14443TAL3_015()                      PATCH_METHOD(PATCH_FLAG_065, (u16)65) 
    #define PATCH_BOOT_APDU_IO14443TAL3_016()                      PATCH_METHOD(PATCH_FLAG_066, (u16)66) 
    #define PATCH_BOOT_APDU_IO14443TAL3_017()                      PATCH_METHOD(PATCH_FLAG_067, (u16)67) 

/***************Patch for Boot_APDU--------->Io14443TaL4.c	*************************/		
    #define PATCH_BOOT_APDU_IO14443TAL4_001()                      PATCH_METHOD(PATCH_FLAG_068, (u16)68) 
    #define PATCH_BOOT_APDU_IO14443TAL4_002()                      PATCH_METHOD(PATCH_FLAG_069, (u16)69) 
    #define PATCH_BOOT_APDU_IO14443TAL4_003()                      PATCH_METHOD(PATCH_FLAG_070, (u16)70) 
    #define PATCH_BOOT_APDU_IO14443TAL4_004()                      PATCH_METHOD(PATCH_FLAG_071, (u16)71) 
    #define PATCH_BOOT_APDU_IO14443TAL4_005()                      PATCH_METHOD(PATCH_FLAG_072, (u16)72) 
    #define PATCH_BOOT_APDU_IO14443TAL4_006()                      PATCH_METHOD(PATCH_FLAG_073, (u16)73) 
    #define PATCH_BOOT_APDU_IO14443TAL4_007()                      PATCH_METHOD(PATCH_FLAG_074, (u16)74) 
    #define PATCH_BOOT_APDU_IO14443TAL4_008()                      PATCH_METHOD(PATCH_FLAG_075, (u16)75) 
    #define PATCH_BOOT_APDU_IO14443TAL4_009()                      PATCH_METHOD(PATCH_FLAG_076, (u16)76) 
    #define PATCH_BOOT_APDU_IO14443TAL4_010()                      PATCH_METHOD(PATCH_FLAG_077, (u16)77) 
    #define PATCH_BOOT_APDU_IO14443TAL4_011()                      PATCH_METHOD(PATCH_FLAG_078, (u16)78) 
    #define PATCH_BOOT_APDU_IO14443TAL4_012()                      PATCH_METHOD(PATCH_FLAG_079, (u16)79) 
    #define PATCH_BOOT_APDU_IO14443TAL4_013()                      PATCH_METHOD(PATCH_FLAG_080, (u16)80) 

/***************Patch for Boot_APDU--------->WtxInerrupt.c	*************************/			
    #define PATCH_BOOT_APDU_WTXINTERRUPT_001()                     PATCH_METHOD(PATCH_FLAG_081, (u16)81) 
    #define PATCH_BOOT_APDU_WTXINTERRUPT_002()                     PATCH_METHOD(PATCH_FLAG_082, (u16)82) 
	
/***************Patch for Boot_APDU--------->WtxTimer.c     *************************/		
    #define PATCH_BOOT_APDU_WTXTIMER_001()                         PATCH_METHOD(PATCH_FLAG_083, (u16)83) 

/***************Patch for Boot_APDU--------->Apdu.c         *************************/	
    #define PATCH_BOOT_APDU_APDU_001()                             PATCH_METHOD(PATCH_FLAG_084, (u16)84) 
    #define PATCH_BOOT_APDU_APDU_002()                             PATCH_METHOD(PATCH_FLAG_085, (u16)85) 
    #define PATCH_BOOT_APDU_APDU_003()                             PATCH_METHOD(PATCH_FLAG_086, (u16)86) 
    #define PATCH_BOOT_APDU_APDU_004()                             PATCH_METHOD(PATCH_FLAG_087, (u16)87) 
    #define PATCH_BOOT_APDU_APDU_005()                             PATCH_METHOD(PATCH_FLAG_088, (u16)88) 
    #define PATCH_BOOT_APDU_APDU_006()                             PATCH_METHOD(PATCH_FLAG_089, (u16)89) 
    #define PATCH_BOOT_APDU_APDU_007()                             PATCH_METHOD(PATCH_FLAG_090, (u16)90) 
    #define PATCH_BOOT_APDU_APDU_008()                             PATCH_METHOD(PATCH_FLAG_091, (u16)91) 
    #define PATCH_BOOT_APDU_APDU_009()                             PATCH_METHOD(PATCH_FLAG_092, (u16)92) 
    #define PATCH_BOOT_APDU_APDU_010()                             PATCH_METHOD(PATCH_FLAG_093, (u16)93) 
    #define PATCH_BOOT_APDU_APDU_011()                             PATCH_METHOD(PATCH_FLAG_094, (u16)94) 
	
/***************Patch for Boot_APDU--------->Io.c	         *************************/	
    #define PATCH_BOOT_APDU_IO_001()                               PATCH_METHOD(PATCH_FLAG_095, (u16)95) 
    #define PATCH_BOOT_APDU_IO_002()                               PATCH_METHOD(PATCH_FLAG_096, (u16)96) 
    #define PATCH_BOOT_APDU_IO_003()                               PATCH_METHOD(PATCH_FLAG_097, (u16)97) 
    #define PATCH_BOOT_APDU_IO_004()                               PATCH_METHOD(PATCH_FLAG_098, (u16)98) 

/***************Patch for Boot_Crypto--------->EllipseCurve_API.c ******************/	
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_001()               PATCH_METHOD(PATCH_FLAG_099, (u16)99) 
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_002()               PATCH_METHOD(PATCH_FLAG_100, (u16)100)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_003()               PATCH_METHOD(PATCH_FLAG_101, (u16)101)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_004()               PATCH_METHOD(PATCH_FLAG_102, (u16)102)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_005()               PATCH_METHOD(PATCH_FLAG_103, (u16)103)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_006()               PATCH_METHOD(PATCH_FLAG_104, (u16)104)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_007()               PATCH_METHOD(PATCH_FLAG_105, (u16)105)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_008()               PATCH_METHOD(PATCH_FLAG_106, (u16)106)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_009()               PATCH_METHOD(PATCH_FLAG_107, (u16)107)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_010()               PATCH_METHOD(PATCH_FLAG_108, (u16)108)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_011()               PATCH_METHOD(PATCH_FLAG_109, (u16)109)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_012()               PATCH_METHOD(PATCH_FLAG_110, (u16)110)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_013()               PATCH_METHOD(PATCH_FLAG_111, (u16)111)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_014()               PATCH_METHOD(PATCH_FLAG_112, (u16)112)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_015()               PATCH_METHOD(PATCH_FLAG_113, (u16)113)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_016()               PATCH_METHOD(PATCH_FLAG_114, (u16)114)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_017()               PATCH_METHOD(PATCH_FLAG_115, (u16)115)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_018()               PATCH_METHOD(PATCH_FLAG_116, (u16)116)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_019()               PATCH_METHOD(PATCH_FLAG_117, (u16)117)
    #define PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_020()               PATCH_METHOD(PATCH_FLAG_118, (u16)118)

/***************Patch for Boot_Crypto--------->HASH_API.c   *************************/	
    #define PATCH_BOOT_HASH_API_001()                              PATCH_METHOD(PATCH_FLAG_119, (u16)119)
    #define PATCH_BOOT_HASH_API_002()                              PATCH_METHOD(PATCH_FLAG_120, (u16)120)
    #define PATCH_BOOT_HASH_API_003()                              PATCH_METHOD(PATCH_FLAG_121, (u16)121)
    #define PATCH_BOOT_HASH_API_004()                              PATCH_METHOD(PATCH_FLAG_122, (u16)122)
    #define PATCH_BOOT_HASH_API_005()                              PATCH_METHOD(PATCH_FLAG_123, (u16)123)

/***************Patch for Boot_Crypto--------->HMAC_API.c	  *************************/	
    #define PATCH_BOOT_HMAC_API_001()                              PATCH_METHOD(PATCH_FLAG_124, (u16)124)
    #define PATCH_BOOT_HMAC_API_002()                              PATCH_METHOD(PATCH_FLAG_125, (u16)125)
    #define PATCH_BOOT_HMAC_API_003()                              PATCH_METHOD(PATCH_FLAG_126, (u16)126)
	
/***************Patch for Boot_Crypto--------->RSA_API.c	  *************************/	
    #define PATCH_BOOT_RSA_API_001()                               PATCH_METHOD(PATCH_FLAG_127, (u16)127)
    #define PATCH_BOOT_RSA_API_002()                               PATCH_METHOD(PATCH_FLAG_128, (u16)128)
    #define PATCH_BOOT_RSA_API_003()                               PATCH_METHOD(PATCH_FLAG_129, (u16)129)
    #define PATCH_BOOT_RSA_API_004()                               PATCH_METHOD(PATCH_FLAG_130, (u16)130)
    #define PATCH_BOOT_RSA_API_005()                               PATCH_METHOD(PATCH_FLAG_131, (u16)131)
    #define PATCH_BOOT_RSA_API_006()                               PATCH_METHOD(PATCH_FLAG_132, (u16)132)
    #define PATCH_BOOT_RSA_API_007()                               PATCH_METHOD(PATCH_FLAG_133, (u16)133)
    #define PATCH_BOOT_RSA_API_008()                               PATCH_METHOD(PATCH_FLAG_134, (u16)134)
    #define PATCH_BOOT_RSA_API_009()                               PATCH_METHOD(PATCH_FLAG_135, (u16)135)
    #define PATCH_BOOT_RSA_API_010()                               PATCH_METHOD(PATCH_FLAG_136, (u16)136)
	
/***************Patch for Boot_Crypto--------->Sym_API.c	  *************************/	
    #define PATCH_BOOT_SYM_API_001()                               PATCH_METHOD(PATCH_FLAG_137, (u16)137)
    #define PATCH_BOOT_SYM_API_002()                               PATCH_METHOD(PATCH_FLAG_138, (u16)138)
    #define PATCH_BOOT_SYM_API_003()                               PATCH_METHOD(PATCH_FLAG_139, (u16)139)
    #define PATCH_BOOT_SYM_API_004()                               PATCH_METHOD(PATCH_FLAG_140, (u16)140)
    #define PATCH_BOOT_SYM_API_005()                               PATCH_METHOD(PATCH_FLAG_141, (u16)141)
    #define PATCH_BOOT_SYM_API_006()                               PATCH_METHOD(PATCH_FLAG_142, (u16)142)
    #define PATCH_BOOT_SYM_API_007()                               PATCH_METHOD(PATCH_FLAG_143, (u16)143)
    #define PATCH_BOOT_SYM_API_008()                               PATCH_METHOD(PATCH_FLAG_144, (u16)144)
    #define PATCH_BOOT_SYM_API_009()                               PATCH_METHOD(PATCH_FLAG_145, (u16)145)
    #define PATCH_BOOT_SYM_API_010()                               PATCH_METHOD(PATCH_FLAG_146, (u16)146)
    #define PATCH_BOOT_SYM_API_011()                               PATCH_METHOD(PATCH_FLAG_147, (u16)147)
    #define PATCH_BOOT_SYM_API_012()                               PATCH_METHOD(PATCH_FLAG_148, (u16)148)
    #define PATCH_BOOT_SYM_API_013()                               PATCH_METHOD(PATCH_FLAG_149, (u16)149)
    #define PATCH_BOOT_SYM_API_014()                               PATCH_METHOD(PATCH_FLAG_150, (u16)150)
    #define PATCH_BOOT_SYM_API_015()                               PATCH_METHOD(PATCH_FLAG_151, (u16)151)

/***************Patch for Boot_ErrorHandle--------->BootErrorHandle.c	  *************/	
    #define PATCH_BOOT_ERRORHANDLE_001()                           PATCH_METHOD(PATCH_FLAG_152, (u16)152)

/***************Patch for Boot_LifeCycle--------->BoorLifeCycle.c	 ******************/	
    #define PATCH_BOOT_LIFECYCLE_001()                             PATCH_METHOD(PATCH_FLAG_153, (u16)153)
    #define PATCH_BOOT_LIFECYCLE_002()                             PATCH_METHOD(PATCH_FLAG_154, (u16)154)
	
/***************Patch for Boot_Memory--------->MemBackUpServe.c	*********************/	
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_001()                 PATCH_METHOD(PATCH_FLAG_155, (u16)155)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_002()                 PATCH_METHOD(PATCH_FLAG_156, (u16)156)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_003()                 PATCH_METHOD(PATCH_FLAG_157, (u16)157)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_004()                 PATCH_METHOD(PATCH_FLAG_158, (u16)158)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_005()                 PATCH_METHOD(PATCH_FLAG_159, (u16)159)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_006()                 PATCH_METHOD(PATCH_FLAG_160, (u16)160)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_007()                 PATCH_METHOD(PATCH_FLAG_161, (u16)161)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_008()                 PATCH_METHOD(PATCH_FLAG_162, (u16)162)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_009()                 PATCH_METHOD(PATCH_FLAG_163, (u16)163)
    #define PATCH_BOOT_MEMORY_MEMBACKUPSERVE_010()                 PATCH_METHOD(PATCH_FLAG_164, (u16)164)
	
/***************Patch for Boot_Memory--------->MemNvmOperation.c	*******************/	
    #define PATCH_BOOT_MEMORY_MEMNVMOPERATION_001()                PATCH_METHOD(PATCH_FLAG_165, (u16)165)
    #define PATCH_BOOT_MEMORY_MEMNVMOPERATION_002()                PATCH_METHOD(PATCH_FLAG_166, (u16)166)
    #define PATCH_BOOT_MEMORY_MEMNVMOPERATION_003()                PATCH_METHOD(PATCH_FLAG_167, (u16)167)
    #define PATCH_BOOT_MEMORY_MEMNVMOPERATION_004()                PATCH_METHOD(PATCH_FLAG_168, (u16)168)
/***************Patch for Boot_Memory--------->MemNvmOperationServe.c	***************/	
    #define PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_001()              PATCH_METHOD(PATCH_FLAG_169, (u16)169)
    #define PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_002()              PATCH_METHOD(PATCH_FLAG_170, (u16)170)
    #define PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_003()              PATCH_METHOD(PATCH_FLAG_171, (u16)171)
    #define PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_004()              PATCH_METHOD(PATCH_FLAG_172, (u16)172)
	
/***************Patch for Boot_Memory--------->MemBackUp.c	*************************/	
    #define PATCH_BOOT_MEMORY_MEMBACKUP_001()                      PATCH_METHOD(PATCH_FLAG_173, (u16)173)
    #define PATCH_BOOT_MEMORY_MEMBACKUP_002()                      PATCH_METHOD(PATCH_FLAG_174, (u16)174)
	
/***************Patch for Boot_Memory--------->MemGetReusedRam.c	*******************/	
    #define PATCH_BOOT_MEMORY_MEMGETREUSERAM_001()                 PATCH_METHOD(PATCH_FLAG_175, (u16)175)

/***************Patch for Boot_Memory--------->MemOperation.c	***************/	
    #define PATCH_BOOT_MEMORY_MEMOPERATION_001()                   PATCH_METHOD(PATCH_FLAG_176, (u16)176)

/***************Patch for Boot_Mifare--------->Mifare.c	*****************************/	
    #define PATCH_BOOT_MIFARE_001()                                PATCH_METHOD(PATCH_FLAG_177, (u16)177)
    #define PATCH_BOOT_MIFARE_002()                                PATCH_METHOD(PATCH_FLAG_178, (u16)178)
    #define PATCH_BOOT_MIFARE_003()                                PATCH_METHOD(PATCH_FLAG_179, (u16)179)
    #define PATCH_BOOT_MIFARE_004()                                PATCH_METHOD(PATCH_FLAG_180, (u16)180)
    #define PATCH_BOOT_MIFARE_005()                                PATCH_METHOD(PATCH_FLAG_181, (u16)181)
    #define PATCH_BOOT_MIFARE_006()                                PATCH_METHOD(PATCH_FLAG_182, (u16)182)
	
/***************Patch for Boot_Patch--------->Patch.c	*******************************/	
    #define PATCH_BOOT_PATCH_001()                                 PATCH_METHOD(PATCH_FLAG_183, (u16)183)
	
/***************Patch for Boot_SecureOperation--------->SecureCrypto.c	   **********/	
    #define PATCH_BOOT_SECURECRYPTO_001()                          PATCH_METHOD(PATCH_FLAG_184, (u16)184)
    #define PATCH_BOOT_SECURECRYPTO_002()                          PATCH_METHOD(PATCH_FLAG_185, (u16)185)
    #define PATCH_BOOT_SECURECRYPTO_003()                          PATCH_METHOD(PATCH_FLAG_186, (u16)186)
    #define PATCH_BOOT_SECURECRYPTO_004()                          PATCH_METHOD(PATCH_FLAG_187, (u16)187)
    #define PATCH_BOOT_SECURECRYPTO_005()                          PATCH_METHOD(PATCH_FLAG_188, (u16)188)
    #define PATCH_BOOT_SECURECRYPTO_006()                          PATCH_METHOD(PATCH_FLAG_189, (u16)189)

/***************Patch for Boot_SecureOperation--------->SecureOperation.c	***********/	
    #define PATCH_BOOT_SECUREOPERATION_001()                       PATCH_METHOD(PATCH_FLAG_190, (u16)190)
    #define PATCH_BOOT_SECUREOPERATION_002()                       PATCH_METHOD(PATCH_FLAG_191, (u16)191)
    #define PATCH_BOOT_SECUREOPERATION_003()                       PATCH_METHOD(PATCH_FLAG_192, (u16)192)

/***************Patch for ConfigFile_Boot-------->bootConfigMethod.c ****************/	

/***************Patch for ConfigFile_Hal--------->halDataConfig.c	  *****************/	

/***************Patch for ConfigFile_VM--------->jcvmConfigMethod.c	*****************/	

/***************Patch for ConfigFile_VM--------->jcvmDataConfig.c	  ****************/	

/***************Patch for JCVM_Chnmanager--------->chn_manager.c	  ****************/	
    #define PATCH_JCVM_CHNMANAGER_001()                            PATCH_METHOD(PATCH_FLAG_193, (u16)193)
    #define PATCH_JCVM_CHNMANAGER_002()                            PATCH_METHOD(PATCH_FLAG_194, (u16)194)
    #define PATCH_JCVM_CHNMANAGER_003()                            PATCH_METHOD(PATCH_FLAG_195, (u16)195)
    #define PATCH_JCVM_CHNMANAGER_004()                            PATCH_METHOD(PATCH_FLAG_196, (u16)196)
    #define PATCH_JCVM_CHNMANAGER_005()                            PATCH_METHOD(PATCH_FLAG_197, (u16)197)
    #define PATCH_JCVM_CHNMANAGER_006()                            PATCH_METHOD(PATCH_FLAG_198, (u16)198)
    #define PATCH_JCVM_CHNMANAGER_007()                            PATCH_METHOD(PATCH_FLAG_199, (u16)199)
	
/***************Patch for JCVM_Init--------->JCVMConfig.c	         *****************/	
    #define PATCH_JCVM_INIT_JCVMCONFIG_001()                       PATCH_METHOD(PATCH_FLAG_200, (u16)200)
    #define PATCH_JCVM_INIT_JCVMCONFIG_002()                       PATCH_METHOD(PATCH_FLAG_201, (u16)201)
    #define PATCH_JCVM_INIT_JCVMCONFIG_003()                       PATCH_METHOD(PATCH_FLAG_202, (u16)202)
    #define PATCH_JCVM_INIT_JCVMCONFIG_004()                       PATCH_METHOD(PATCH_FLAG_203, (u16)203)
    #define PATCH_JCVM_INIT_JCVMCONFIG_005()                       PATCH_METHOD(PATCH_FLAG_204, (u16)204)
	
/***************Patch for JCVM_Init--------->JCVMInit.c	          ******************/	
    #define PATCH_JCVM_INIT_JCVMINIT_001()                         PATCH_METHOD(PATCH_FLAG_205, (u16)205)

/***************Patch for JCVM_Mask--------->mask.c	     	**************************/	
		
	
/***************Patch for JCVM_Memory--------->memory.c	      **********************/	
    #define PATCH_JCVM_MEMORY_MEMORY_001()                         PATCH_METHOD(PATCH_FLAG_206, (u16)206)
    #define PATCH_JCVM_MEMORY_MEMORY_002()                         PATCH_METHOD(PATCH_FLAG_207, (u16)207)
    #define PATCH_JCVM_MEMORY_MEMORY_003()                         PATCH_METHOD(PATCH_FLAG_208, (u16)208)
    #define PATCH_JCVM_MEMORY_MEMORY_004()                         PATCH_METHOD(PATCH_FLAG_209, (u16)209)
    #define PATCH_JCVM_MEMORY_MEMORY_005()                         PATCH_METHOD(PATCH_FLAG_210, (u16)210)
    #define PATCH_JCVM_MEMORY_MEMORY_006()                         PATCH_METHOD(PATCH_FLAG_211, (u16)211)
    #define PATCH_JCVM_MEMORY_MEMORY_007()                         PATCH_METHOD(PATCH_FLAG_212, (u16)212)
    #define PATCH_JCVM_MEMORY_MEMORY_008()                         PATCH_METHOD(PATCH_FLAG_213, (u16)213)
    #define PATCH_JCVM_MEMORY_MEMORY_009()                         PATCH_METHOD(PATCH_FLAG_214, (u16)214)
    #define PATCH_JCVM_MEMORY_MEMORY_010()                         PATCH_METHOD(PATCH_FLAG_215, (u16)215)
    #define PATCH_JCVM_MEMORY_MEMORY_011()                         PATCH_METHOD(PATCH_FLAG_216, (u16)216)
    #define PATCH_JCVM_MEMORY_MEMORY_012()                         PATCH_METHOD(PATCH_FLAG_217, (u16)217)
    #define PATCH_JCVM_MEMORY_MEMORY_013()                         PATCH_METHOD(PATCH_FLAG_218, (u16)218)
    #define PATCH_JCVM_MEMORY_MEMORY_014()                         PATCH_METHOD(PATCH_FLAG_219, (u16)219)
    #define PATCH_JCVM_MEMORY_MEMORY_015()                         PATCH_METHOD(PATCH_FLAG_220, (u16)220)
    #define PATCH_JCVM_MEMORY_MEMORY_016()                         PATCH_METHOD(PATCH_FLAG_221, (u16)221)
    #define PATCH_JCVM_MEMORY_MEMORY_017()                         PATCH_METHOD(PATCH_FLAG_222, (u16)222)
    #define PATCH_JCVM_MEMORY_MEMORY_018()                         PATCH_METHOD(PATCH_FLAG_223, (u16)223)
    #define PATCH_JCVM_MEMORY_MEMORY_019()                         PATCH_METHOD(PATCH_FLAG_224, (u16)224)
    #define PATCH_JCVM_MEMORY_MEMORY_020()                         PATCH_METHOD(PATCH_FLAG_225, (u16)225)
    #define PATCH_JCVM_MEMORY_MEMORY_021()                         PATCH_METHOD(PATCH_FLAG_226, (u16)226)
    #define PATCH_JCVM_MEMORY_MEMORY_022()                         PATCH_METHOD(PATCH_FLAG_227, (u16)227)

/***************Patch for JCVM_NativeAppApi--------->AppAPI.c	      **********************/	


/***************Patch for JCVM_NativeAppApi--------->PbocAPI.c	    **********************/	


/***************Patch for JCVM_OpenPlatform--------->CardManager.c	**********************/	
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_001()              PATCH_METHOD(PATCH_FLAG_228, (u16)228)
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_002()              PATCH_METHOD(PATCH_FLAG_229, (u16)229)
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_003()              PATCH_METHOD(PATCH_FLAG_230, (u16)230)
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_004()              PATCH_METHOD(PATCH_FLAG_231, (u16)231)
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_005()              PATCH_METHOD(PATCH_FLAG_232, (u16)232)
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_006()              PATCH_METHOD(PATCH_FLAG_233, (u16)233)
    #define PATCH_JCVM_OPENPLATFORM_CARDMANAGER_007()              PATCH_METHOD(PATCH_FLAG_234, (u16)234)

/***************Patch for JCVM_OpenPlatform--------->ContentManager.c	********************/
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_001()           PATCH_METHOD(PATCH_FLAG_235, (u16)235)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_002()           PATCH_METHOD(PATCH_FLAG_236, (u16)236)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_003()           PATCH_METHOD(PATCH_FLAG_237, (u16)237)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_004()           PATCH_METHOD(PATCH_FLAG_238, (u16)238)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_005()           PATCH_METHOD(PATCH_FLAG_239, (u16)239)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_006()           PATCH_METHOD(PATCH_FLAG_240, (u16)240)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_007()           PATCH_METHOD(PATCH_FLAG_241, (u16)241)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_008()           PATCH_METHOD(PATCH_FLAG_242, (u16)242)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_009()           PATCH_METHOD(PATCH_FLAG_243, (u16)243)
    #define PATCH_JCVM_OPENPLATFORM_CONTENTMANAGER_010()           PATCH_METHOD(PATCH_FLAG_244, (u16)244)
		
/***************Patch for JCVM_OpenPlatform--------->CCM.c	******************************/
    #define PATCH_JCVM_OPENPLATFORM_CVM_001()                      PATCH_METHOD(PATCH_FLAG_245, (u16)245)
    #define PATCH_JCVM_OPENPLATFORM_CVM_002()                      PATCH_METHOD(PATCH_FLAG_246, (u16)246)
    #define PATCH_JCVM_OPENPLATFORM_CVM_003()                      PATCH_METHOD(PATCH_FLAG_247, (u16)247)
	
/***************Patch for JCVM_OpenPlatform--------->KeyManager.c	************************/
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_001()               PATCH_METHOD(PATCH_FLAG_248, (u16)248)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_002()               PATCH_METHOD(PATCH_FLAG_249, (u16)249)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_003()               PATCH_METHOD(PATCH_FLAG_250, (u16)250)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_004()               PATCH_METHOD(PATCH_FLAG_251, (u16)251)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_005()               PATCH_METHOD(PATCH_FLAG_252, (u16)252)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_006()               PATCH_METHOD(PATCH_FLAG_253, (u16)253)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_007()               PATCH_METHOD(PATCH_FLAG_254, (u16)254)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_008()               PATCH_METHOD(PATCH_FLAG_255, (u16)255)
    #define PATCH_JCVM_OPENPLATFORM_KEYMANAGER_009()               PATCH_METHOD(PATCH_FLAG_256, (u16)256)
	
/***************Patch for JCVM_OpenPlatform--------->Loader.c	****************************/
    #define PATCH_JCVM_OPENPLATFORM_LOADER_001()                   PATCH_METHOD(PATCH_FLAG_257, (u16)257)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_002()                   PATCH_METHOD(PATCH_FLAG_258, (u16)258)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_003()                   PATCH_METHOD(PATCH_FLAG_259, (u16)259)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_004()                   PATCH_METHOD(PATCH_FLAG_260, (u16)260)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_005()                   PATCH_METHOD(PATCH_FLAG_261, (u16)261)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_006()                   PATCH_METHOD(PATCH_FLAG_262, (u16)262)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_007()                   PATCH_METHOD(PATCH_FLAG_263, (u16)263)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_008()                   PATCH_METHOD(PATCH_FLAG_264, (u16)264)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_009()                   PATCH_METHOD(PATCH_FLAG_265, (u16)265)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_010()                   PATCH_METHOD(PATCH_FLAG_266, (u16)266)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_011()                   PATCH_METHOD(PATCH_FLAG_267, (u16)267)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_012()                   PATCH_METHOD(PATCH_FLAG_268, (u16)268)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_013()                   PATCH_METHOD(PATCH_FLAG_269, (u16)269)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_014()                   PATCH_METHOD(PATCH_FLAG_270, (u16)270)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_015()                   PATCH_METHOD(PATCH_FLAG_271, (u16)271)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_016()                   PATCH_METHOD(PATCH_FLAG_272, (u16)272)
    #define PATCH_JCVM_OPENPLATFORM_LOADER_017()                   PATCH_METHOD(PATCH_FLAG_273, (u16)273)
	
/***************Patch for JCVM_OpenPlatform--------->OpFunction.c	************************/
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_001()               PATCH_METHOD(PATCH_FLAG_274, (u16)274)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_002()               PATCH_METHOD(PATCH_FLAG_275, (u16)275)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_003()               PATCH_METHOD(PATCH_FLAG_276, (u16)276)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_004()               PATCH_METHOD(PATCH_FLAG_277, (u16)277)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_005()               PATCH_METHOD(PATCH_FLAG_278, (u16)278)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_006()               PATCH_METHOD(PATCH_FLAG_279, (u16)279)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_007()               PATCH_METHOD(PATCH_FLAG_280, (u16)280)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_008()               PATCH_METHOD(PATCH_FLAG_281, (u16)281)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_009()               PATCH_METHOD(PATCH_FLAG_282, (u16)282)
    #define PATCH_JCVM_OPENPLATFORM_OPFUNCTION_010()               PATCH_METHOD(PATCH_FLAG_283, (u16)283)

/***************Patch for JCVM_OpenPlatform--------->OPInstaller.c	**********************/
    #define PATCH_JCVM_OPENPLATFORM_OPINSTALLER_001()              PATCH_METHOD(PATCH_FLAG_284, (u16)284)
    #define PATCH_JCVM_OPENPLATFORM_OPINSTALLER_002()              PATCH_METHOD(PATCH_FLAG_285, (u16)285)
    #define PATCH_JCVM_OPENPLATFORM_OPINSTALLER_003()              PATCH_METHOD(PATCH_FLAG_286, (u16)286)
    #define PATCH_JCVM_OPENPLATFORM_OPINSTALLER_004()              PATCH_METHOD(PATCH_FLAG_287, (u16)287)
    #define PATCH_JCVM_OPENPLATFORM_OPINSTALLER_005()              PATCH_METHOD(PATCH_FLAG_288, (u16)288)
	
/***************Patch for JCVM_OpenPlatform--------->UtilMethod.c	*************************/

/***************Patch for JCVM_RamManager--------->dtrSpace.c	*****************************/
    #define PATCH_JCVM_RAMMANAGER_DTRSPACE_001()                   PATCH_METHOD(PATCH_FLAG_289, (u16)289)
    #define PATCH_JCVM_RAMMANAGER_DTRSPACE_002()                   PATCH_METHOD(PATCH_FLAG_290, (u16)290)
    #define PATCH_JCVM_RAMMANAGER_DTRSPACE_003()                   PATCH_METHOD(PATCH_FLAG_291, (u16)291)
    #define PATCH_JCVM_RAMMANAGER_DTRSPACE_004()                   PATCH_METHOD(PATCH_FLAG_292, (u16)292)
    #define PATCH_JCVM_RAMMANAGER_DTRSPACE_005()                   PATCH_METHOD(PATCH_FLAG_293, (u16)293)
	
/***************Patch for JCVM_RamManager--------->ramAllocate.c	*****************************/
    #define PATCH_JCVM_RAMMANAGER_RAMALLOCATE_001()                PATCH_METHOD(PATCH_FLAG_294, (u16)294)
    #define PATCH_JCVM_RAMMANAGER_RAMALLOCATE_002()                PATCH_METHOD(PATCH_FLAG_295, (u16)295)
    #define PATCH_JCVM_RAMMANAGER_RAMALLOCATE_003()                PATCH_METHOD(PATCH_FLAG_296, (u16)296)
	
/***************Patch for JCVM_RamManager--------->ramDeallocate.c	*****************************/
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_001()              PATCH_METHOD(PATCH_FLAG_297, (u16)297)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_002()              PATCH_METHOD(PATCH_FLAG_298, (u16)298)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_003()              PATCH_METHOD(PATCH_FLAG_299, (u16)299)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_004()              PATCH_METHOD(PATCH_FLAG_300, (u16)300)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_005()              PATCH_METHOD(PATCH_FLAG_301, (u16)301)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_006()              PATCH_METHOD(PATCH_FLAG_302, (u16)302)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_007()              PATCH_METHOD(PATCH_FLAG_303, (u16)303)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_008()              PATCH_METHOD(PATCH_FLAG_304, (u16)304)
    #define PATCH_JCVM_RAMMANAGER_RAMDEALLOCATE_009()              PATCH_METHOD(PATCH_FLAG_305, (u16)305)

/***************Patch for JCVM_RamManager--------->RamManager.c	*********************************/
    #define PATCH_JCVM_RAMMANAGER_RAMMANAGER_001()                 PATCH_METHOD(PATCH_FLAG_306, (u16)306)
    #define PATCH_JCVM_RAMMANAGER_RAMMANAGER_002()                 PATCH_METHOD(PATCH_FLAG_307, (u16)307)
    #define PATCH_JCVM_RAMMANAGER_RAMMANAGER_003()                 PATCH_METHOD(PATCH_FLAG_308, (u16)308)
		
/***************Patch for JCVM_Vm--------->crypto_impl.c	***************************************/
    #define PATCH_JCVM_VM_CRYPTO_IMPL_001()                        PATCH_METHOD(PATCH_FLAG_309, (u16)309)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_002()                        PATCH_METHOD(PATCH_FLAG_310, (u16)310)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_003()                        PATCH_METHOD(PATCH_FLAG_311, (u16)311)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_004()                        PATCH_METHOD(PATCH_FLAG_312, (u16)312)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_005()                        PATCH_METHOD(PATCH_FLAG_313, (u16)313)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_006()                        PATCH_METHOD(PATCH_FLAG_314, (u16)314)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_007()                        PATCH_METHOD(PATCH_FLAG_315, (u16)315)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_008()                        PATCH_METHOD(PATCH_FLAG_316, (u16)316)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_009()                        PATCH_METHOD(PATCH_FLAG_317, (u16)317)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_010()                        PATCH_METHOD(PATCH_FLAG_318, (u16)318)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_011()                        PATCH_METHOD(PATCH_FLAG_319, (u16)319)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_012()                        PATCH_METHOD(PATCH_FLAG_320, (u16)320)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_013()                        PATCH_METHOD(PATCH_FLAG_321, (u16)321)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_014()                        PATCH_METHOD(PATCH_FLAG_322, (u16)322)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_015()                        PATCH_METHOD(PATCH_FLAG_323, (u16)323)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_016()                        PATCH_METHOD(PATCH_FLAG_324, (u16)324)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_017()                        PATCH_METHOD(PATCH_FLAG_325, (u16)325)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_018()                        PATCH_METHOD(PATCH_FLAG_326, (u16)326)
    #define PATCH_JCVM_VM_CRYPTO_IMPL_019()                        PATCH_METHOD(PATCH_FLAG_327, (u16)327)

/***************Patch for JCVM_Vm--------->except.c	**********************************************/
    #define PATCH_JCVM_VM_EXCEPT_001()                             PATCH_METHOD(PATCH_FLAG_328, (u16)328)
    #define PATCH_JCVM_VM_EXCEPT_002()                             PATCH_METHOD(PATCH_FLAG_329, (u16)329)
    #define PATCH_JCVM_VM_EXCEPT_003()                             PATCH_METHOD(PATCH_FLAG_330, (u16)330)
	
/***************Patch for JCVM_Vm--------->gc_main.c	*********************************************/
    #define PATCH_JCVM_VM_GC_MAIN_001()                            PATCH_METHOD(PATCH_FLAG_331, (u16)331)
    #define PATCH_JCVM_VM_GC_MAIN_002()                            PATCH_METHOD(PATCH_FLAG_332, (u16)332)
    #define PATCH_JCVM_VM_GC_MAIN_003()                            PATCH_METHOD(PATCH_FLAG_333, (u16)333)
    #define PATCH_JCVM_VM_GC_MAIN_004()                            PATCH_METHOD(PATCH_FLAG_334, (u16)334)
    #define PATCH_JCVM_VM_GC_MAIN_005()                            PATCH_METHOD(PATCH_FLAG_335, (u16)335)
    #define PATCH_JCVM_VM_GC_MAIN_006()                            PATCH_METHOD(PATCH_FLAG_336, (u16)336)
    #define PATCH_JCVM_VM_GC_MAIN_007()                            PATCH_METHOD(PATCH_FLAG_337, (u16)337)

/***************Patch for JCVM_Vm--------->grt.c			*********************************************/
    #define PATCH_JCVM_VM_GRT_001()                                PATCH_METHOD(PATCH_FLAG_338, (u16)338)
	
/***************Patch for JCVM_Vm--------->interpreter.c	*****************************************/
    #define PATCH_JCVM_VM_INTERPRETER_001()                        PATCH_METHOD(PATCH_FLAG_339, (u16)339)
    #define PATCH_JCVM_VM_INTERPRETER_002()                        PATCH_METHOD(PATCH_FLAG_340, (u16)340)
	
/***************Patch for JCVM_Vm--------->method.c				*****************************************/
    #define PATCH_JCVM_VM_METHOD_001()                             PATCH_METHOD(PATCH_FLAG_341, (u16)341)
    #define PATCH_JCVM_VM_METHOD_002()                             PATCH_METHOD(PATCH_FLAG_342, (u16)342)
    #define PATCH_JCVM_VM_METHOD_003()                             PATCH_METHOD(PATCH_FLAG_343, (u16)343)

/***************Patch for JCVM_Vm--------->nativemethod_impl.c		**********************************/
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_001()                  PATCH_METHOD(PATCH_FLAG_344, (u16)344)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_002()                  PATCH_METHOD(PATCH_FLAG_345, (u16)345)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_003()                  PATCH_METHOD(PATCH_FLAG_346, (u16)346)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_004()                  PATCH_METHOD(PATCH_FLAG_347, (u16)347)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_005()                  PATCH_METHOD(PATCH_FLAG_348, (u16)348)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_006()                  PATCH_METHOD(PATCH_FLAG_349, (u16)349)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_007()                  PATCH_METHOD(PATCH_FLAG_350, (u16)350)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_008()                  PATCH_METHOD(PATCH_FLAG_351, (u16)351)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_009()                  PATCH_METHOD(PATCH_FLAG_352, (u16)352)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_010()                  PATCH_METHOD(PATCH_FLAG_353, (u16)353)
    #define PATCH_JCVM_VM_NATIVEMETHOD_IMPL_011()                  PATCH_METHOD(PATCH_FLAG_354, (u16)354)

/***************Patch for JCVM_Vm--------->ob_manager.c				**************************************/
    #define PATCH_JCVM_VM_OB_MANAGER_001()                         PATCH_METHOD(PATCH_FLAG_355, (u16)355)
    #define PATCH_JCVM_VM_OB_MANAGER_002()                         PATCH_METHOD(PATCH_FLAG_356, (u16)356)
    #define PATCH_JCVM_VM_OB_MANAGER_003()                         PATCH_METHOD(PATCH_FLAG_357, (u16)357)
    #define PATCH_JCVM_VM_OB_MANAGER_004()                         PATCH_METHOD(PATCH_FLAG_358, (u16)358)

/***************Patch for JCVM_Vm--------->object.c				******************************************/
    #define PATCH_JCVM_VM_OBJECT_001()                             PATCH_METHOD(PATCH_FLAG_359, (u16)359)
    #define PATCH_JCVM_VM_OBJECT_002()                             PATCH_METHOD(PATCH_FLAG_360, (u16)360)
	
/***************Patch for JCVM_Vm--------->packageloc.c				***************************************/
    #define PATCH_JCVM_VM_PACKAGELOC_001()                         PATCH_METHOD(PATCH_FLAG_361, (u16)361)
    #define PATCH_JCVM_VM_PACKAGELOC_002()                         PATCH_METHOD(PATCH_FLAG_362, (u16)362)
    #define PATCH_JCVM_VM_PACKAGELOC_003()                         PATCH_METHOD(PATCH_FLAG_363, (u16)363)
    #define PATCH_JCVM_VM_PACKAGELOC_004()                         PATCH_METHOD(PATCH_FLAG_364, (u16)364)
    #define PATCH_JCVM_VM_PACKAGELOC_005()                         PATCH_METHOD(PATCH_FLAG_365, (u16)365)
	
/***************Patch for JCVM_Vm--------->stack.c				******************************************/	
    #define PATCH_JCVM_VM_STACK_001()                              PATCH_METHOD(PATCH_FLAG_366, (u16)366)
    #define PATCH_JCVM_VM_STACK_002()                              PATCH_METHOD(PATCH_FLAG_367, (u16)367)
	
/***************Patch for HAL_Crypto--------->CRC_HAL.c		******************************************/		
    #define PATCH_HAL_CRYPTO_CRC_HAL_001()                         PATCH_METHOD(PATCH_FLAG_368, (u16)368)
    #define PATCH_HAL_CRYPTO_CRC_HAL_002()                         PATCH_METHOD(PATCH_FLAG_369, (u16)369)
	
/***************Patch for HAL_Crypto--------->CRYPTO_SOFTMD.c		************************************/		
    #define PATCH_HAL_CRYPTO_SOFTMD_001()                          PATCH_METHOD(PATCH_FLAG_370, (u16)370)
	
/***************Patch for HAL_Crypto--------->HASH_SHA1_HAL.c		************************************/		
    #define PATCH_HAL_CRYPTO_HASH_SHA1_HAL_001()                   PATCH_METHOD(PATCH_FLAG_371, (u16)371)
    #define PATCH_HAL_CRYPTO_HASH_SHA1_HAL_002()                   PATCH_METHOD(PATCH_FLAG_372, (u16)372)
    #define PATCH_HAL_CRYPTO_HASH_SHA1_HAL_003()                   PATCH_METHOD(PATCH_FLAG_373, (u16)373)
	
/***************Patch for HAL_Crypto--------->HASH_SHA224_HAL.c		**********************************/		
    #define PATCH_HAL_CRYPTO_HASH_SHA224_HAL_001()                 PATCH_METHOD(PATCH_FLAG_374, (u16)374)
    #define PATCH_HAL_CRYPTO_HASH_SHA224_HAL_002()                 PATCH_METHOD(PATCH_FLAG_375, (u16)375)
    #define PATCH_HAL_CRYPTO_HASH_SHA224_HAL_003()                 PATCH_METHOD(PATCH_FLAG_376, (u16)376)
	
/***************Patch for HAL_Crypto--------->HASH_SHA256_HAL.c		**********************************/		
    #define PATCH_HAL_CRYPTO_HASH_SHA256_HAL_001()                 PATCH_METHOD(PATCH_FLAG_377, (u16)377)
    #define PATCH_HAL_CRYPTO_HASH_SHA256_HAL_002()                 PATCH_METHOD(PATCH_FLAG_378, (u16)378)
    #define PATCH_HAL_CRYPTO_HASH_SHA256_HAL_003()                 PATCH_METHOD(PATCH_FLAG_379, (u16)379)

/***************Patch for HAL_Crypto--------->RNG_HAL.c		******************************************/		
    #define PATCH_HAL_CRYPTO_RNG_HAL_001()                         PATCH_METHOD(PATCH_FLAG_380, (u16)380)
	
/***************Patch for HAL_Crypto--------->RSA_HAL.c		******************************************/		
    #define PATCH_HAL_CRYPTO_RSA_HAL_001()                         PATCH_METHOD(PATCH_FLAG_381, (u16)381)
    #define PATCH_HAL_CRYPTO_RSA_HAL_002()                         PATCH_METHOD(PATCH_FLAG_382, (u16)382)
    #define PATCH_HAL_CRYPTO_RSA_HAL_003()                         PATCH_METHOD(PATCH_FLAG_383, (u16)383)
    #define PATCH_HAL_CRYPTO_RSA_HAL_004()                         PATCH_METHOD(PATCH_FLAG_384, (u16)384)
	
/***************Patch for HAL_Crypto--------->Sym_AES_HAL.c		**************************************/		
    #define PATCH_HAL_CRYPTO_SYM_AES_HAL_001()                     PATCH_METHOD(PATCH_FLAG_385, (u16)385)
    #define PATCH_HAL_CRYPTO_SYM_AES_HAL_002()                     PATCH_METHOD(PATCH_FLAG_386, (u16)386)
    #define PATCH_HAL_CRYPTO_SYM_AES_HAL_003()                     PATCH_METHOD(PATCH_FLAG_387, (u16)387)
	
/***************Patch for HAL_Crypto--------->Sym_DES_HAL.c		**************************************/		
    #define PATCH_HAL_CRYPTO_SYM_DES_HAL_001()                     PATCH_METHOD(PATCH_FLAG_388, (u16)388)
    #define PATCH_HAL_CRYPTO_SYM_DES_HAL_002()                     PATCH_METHOD(PATCH_FLAG_389, (u16)389)
    #define PATCH_HAL_CRYPTO_SYM_DES_HAL_003()                     PATCH_METHOD(PATCH_FLAG_390, (u16)390)
    #define PATCH_HAL_CRYPTO_SYM_DES_HAL_004()                     PATCH_METHOD(PATCH_FLAG_391, (u16)391)

/***************Patch for HAL_DataOperation--------->ecpDefine.c		*********************************/
/***************Patch for HAL_IoHal--------->RfHAL.c		*********************************************/
    #define PATCH_HAL_IOHAL_RFHAL_001()                            PATCH_METHOD(PATCH_FLAG_392, (u16)392)
    #define PATCH_HAL_IOHAL_RFHAL_002()                            PATCH_METHOD(PATCH_FLAG_393, (u16)393)
    #define PATCH_HAL_IOHAL_RFHAL_003()                            PATCH_METHOD(PATCH_FLAG_394, (u16)394)
    #define PATCH_HAL_IOHAL_RFHAL_004()                            PATCH_METHOD(PATCH_FLAG_395, (u16)395)
    #define PATCH_HAL_IOHAL_RFHAL_005()                            PATCH_METHOD(PATCH_FLAG_396, (u16)396)
    #define PATCH_HAL_IOHAL_RFHAL_006()                            PATCH_METHOD(PATCH_FLAG_397, (u16)397)
    #define PATCH_HAL_IOHAL_RFHAL_007()                            PATCH_METHOD(PATCH_FLAG_398, (u16)398)
    #define PATCH_HAL_IOHAL_RFHAL_008()                            PATCH_METHOD(PATCH_FLAG_399, (u16)399)
    #define PATCH_HAL_IOHAL_RFHAL_009()                            PATCH_METHOD(PATCH_FLAG_400, (u16)400)
    #define PATCH_HAL_IOHAL_RFHAL_010()                            PATCH_METHOD(PATCH_FLAG_401, (u16)401)
	
/***************Patch for HAL_IoHal--------->UartHAL.c		*******************************************/
    #define PATCH_HAL_IOHAL_UARTHAL_001()                          PATCH_METHOD(PATCH_FLAG_402, (u16)402)
    #define PATCH_HAL_IOHAL_UARTHAL_002()                          PATCH_METHOD(PATCH_FLAG_403, (u16)403)
    #define PATCH_HAL_IOHAL_UARTHAL_003()                          PATCH_METHOD(PATCH_FLAG_404, (u16)404)
    #define PATCH_HAL_IOHAL_UARTHAL_004()                          PATCH_METHOD(PATCH_FLAG_405, (u16)405)
    #define PATCH_HAL_IOHAL_UARTHAL_005()                          PATCH_METHOD(PATCH_FLAG_406, (u16)406)
    #define PATCH_HAL_IOHAL_UARTHAL_006()                          PATCH_METHOD(PATCH_FLAG_407, (u16)407)
    #define PATCH_HAL_IOHAL_UARTHAL_007()                          PATCH_METHOD(PATCH_FLAG_408, (u16)408)
    #define PATCH_HAL_IOHAL_UARTHAL_008()                          PATCH_METHOD(PATCH_FLAG_409, (u16)409)


/***************Patch for HAL_IoHal--------->WtxTimerHAL.c		***************************************/
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_001()                      PATCH_METHOD(PATCH_FLAG_410, (u16)410)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_002()                      PATCH_METHOD(PATCH_FLAG_411, (u16)411)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_003()                      PATCH_METHOD(PATCH_FLAG_412, (u16)412)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_004()                      PATCH_METHOD(PATCH_FLAG_413, (u16)413)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_005()                      PATCH_METHOD(PATCH_FLAG_414, (u16)414)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_006()                      PATCH_METHOD(PATCH_FLAG_415, (u16)415)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_007()                      PATCH_METHOD(PATCH_FLAG_416, (u16)416)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_008()                      PATCH_METHOD(PATCH_FLAG_417, (u16)417)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_009()                      PATCH_METHOD(PATCH_FLAG_418, (u16)418)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_010()                      PATCH_METHOD(PATCH_FLAG_419, (u16)419)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_011()                      PATCH_METHOD(PATCH_FLAG_420, (u16)420)
    #define PATCH_HAL_IOHAL_WTXTIMERHAL_012()                      PATCH_METHOD(PATCH_FLAG_421, (u16)421)

/***************Patch for HAL_MifareHal--------->MifareHAL.c		*************************************/
    #define PATCH_HAL_MIFAREHAL_001()                              PATCH_METHOD(PATCH_FLAG_422, (u16)422)
    #define PATCH_HAL_MIFAREHAL_002()                              PATCH_METHOD(PATCH_FLAG_423, (u16)423)
    #define PATCH_HAL_MIFAREHAL_003()                              PATCH_METHOD(PATCH_FLAG_424, (u16)424)
    #define PATCH_HAL_MIFAREHAL_004()                              PATCH_METHOD(PATCH_FLAG_425, (u16)425)
	
/***************Patch for HAL_NvmHal--------->MifareHAL.c		*************************************/
    #define PATCH_HAL_NVMHAL_001()                                 PATCH_METHOD(PATCH_FLAG_426, (u16)426)
    #define PATCH_HAL_NVMHAL_002()                                 PATCH_METHOD(PATCH_FLAG_427, (u16)427)
    #define PATCH_HAL_NVMHAL_003()                                 PATCH_METHOD(PATCH_FLAG_428, (u16)428)
	
/***************Patch for HAL_PatchHal--------->PatchHAL.c		***************************************/	
/***************Patch for HAL_SecOperationHal--------->SecOperationHal.c		*************************/	

/***************Patch for HAL_SystemConfig--------->SysHal.c		*************************************/
    #define PATCH_HAL_SYSTEMCONFIG_SYSHAL_001()                    PATCH_METHOD(PATCH_FLAG_429, (u16)429)
    #define PATCH_HAL_SYSTEMCONFIG_SYSHAL_002()                    PATCH_METHOD(PATCH_FLAG_430, (u16)430)
	
/***************Patch for ChipHal--------->St_rfInit.c		*************************************/	
    #define PATCH_CHIPHAL_ST_RFINIT_001()                          PATCH_METHOD(PATCH_FLAG_431, (u16)431)
    #define PATCH_CHIPHAL_ST_RFINIT_002()                          PATCH_METHOD(PATCH_FLAG_432, (u16)432)


///////////////////////////////////////////////////////////////////////////////////////////////////////	
/***************Patch for Pboc--------->ecpPBOC_Alg.c								**********************************/		
    #define PATCH_PBOC_ALG_001()                                   PATCH_METHOD(PATCH_FLAG_433, (u16)433)
    #define PATCH_PBOC_ALG_002()                                   PATCH_METHOD(PATCH_FLAG_434, (u16)434)
    #define PATCH_PBOC_ALG_003()                                   PATCH_METHOD(PATCH_FLAG_435, (u16)435)
    #define PATCH_PBOC_ALG_004()                                   PATCH_METHOD(PATCH_FLAG_436, (u16)436)
    #define PATCH_PBOC_ALG_005()                                   PATCH_METHOD(PATCH_FLAG_437, (u16)437)
    #define PATCH_PBOC_ALG_006()                                   PATCH_METHOD(PATCH_FLAG_438, (u16)438)
	                                                                                                                                                                                                                             
	                                                                                                                                                                                                                                                                           
/***************Patch for Pboc--------->ecpPBOC_AppCommand.c								***************************/	                                                                                                                           
    #define PATCH_PBOC_APPCMD_001()                                PATCH_METHOD(PATCH_FLAG_439, (u16)439)
    #define PATCH_PBOC_APPCMD_002()                                PATCH_METHOD(PATCH_FLAG_440, (u16)440)
    #define PATCH_PBOC_APPCMD_003()                                PATCH_METHOD(PATCH_FLAG_441, (u16)441)
    #define PATCH_PBOC_APPCMD_004()                                PATCH_METHOD(PATCH_FLAG_442, (u16)442)
    #define PATCH_PBOC_APPCMD_005()                                PATCH_METHOD(PATCH_FLAG_443, (u16)443)
    #define PATCH_PBOC_APPCMD_006()                                PATCH_METHOD(PATCH_FLAG_444, (u16)444)
    #define PATCH_PBOC_APPCMD_007()                                PATCH_METHOD(PATCH_FLAG_445, (u16)445)
    #define PATCH_PBOC_APPCMD_008()                                PATCH_METHOD(PATCH_FLAG_446, (u16)446)
    #define PATCH_PBOC_APPCMD_009()                                PATCH_METHOD(PATCH_FLAG_447, (u16)447)
    #define PATCH_PBOC_APPCMD_010()                                PATCH_METHOD(PATCH_FLAG_448, (u16)448)
    #define PATCH_PBOC_APPCMD_011()                                PATCH_METHOD(PATCH_FLAG_449, (u16)449)
    #define PATCH_PBOC_APPCMD_012()                                PATCH_METHOD(PATCH_FLAG_450, (u16)450)
    #define PATCH_PBOC_APPCMD_013()                                PATCH_METHOD(PATCH_FLAG_451, (u16)451)
    #define PATCH_PBOC_APPCMD_014()                                PATCH_METHOD(PATCH_FLAG_452, (u16)452)
    #define PATCH_PBOC_APPCMD_015()                                PATCH_METHOD(PATCH_FLAG_453, (u16)453)
    #define PATCH_PBOC_APPCMD_016()                                PATCH_METHOD(PATCH_FLAG_454, (u16)454)
    #define PATCH_PBOC_APPCMD_017()                                PATCH_METHOD(PATCH_FLAG_455, (u16)455)
	                                                                                                                                                                                                                                                                           
	                                                                                                                                                                                                                                                                           
/***************Patch for Pboc--------->ecpPBOC_CmdApi.c								********************************/		                                                                                                           
    #define PATCH_PBOC_CMDAPI_001()                                PATCH_METHOD(PATCH_FLAG_456, (u16)456)
    #define PATCH_PBOC_CMDAPI_002()                                PATCH_METHOD(PATCH_FLAG_457, (u16)457)
    #define PATCH_PBOC_CMDAPI_003()                                PATCH_METHOD(PATCH_FLAG_458, (u16)458)
    #define PATCH_PBOC_CMDAPI_004()                                PATCH_METHOD(PATCH_FLAG_459, (u16)459)
    #define PATCH_PBOC_CMDAPI_005()                                PATCH_METHOD(PATCH_FLAG_460, (u16)460)
    #define PATCH_PBOC_CMDAPI_006()                                PATCH_METHOD(PATCH_FLAG_461, (u16)461)
    #define PATCH_PBOC_CMDAPI_007()                                PATCH_METHOD(PATCH_FLAG_462, (u16)462)
    #define PATCH_PBOC_CMDAPI_008()                                PATCH_METHOD(PATCH_FLAG_463, (u16)463)
    #define PATCH_PBOC_CMDAPI_009()                                PATCH_METHOD(PATCH_FLAG_464, (u16)464)
    #define PATCH_PBOC_CMDAPI_010()                                PATCH_METHOD(PATCH_FLAG_465, (u16)465)
    #define PATCH_PBOC_CMDAPI_011()                                PATCH_METHOD(PATCH_FLAG_466, (u16)466)
    #define PATCH_PBOC_CMDAPI_012()                                PATCH_METHOD(PATCH_FLAG_467, (u16)467)
    #define PATCH_PBOC_CMDAPI_013()                                PATCH_METHOD(PATCH_FLAG_468, (u16)468)
    #define PATCH_PBOC_CMDAPI_014()                                PATCH_METHOD(PATCH_FLAG_469, (u16)469)
    #define PATCH_PBOC_CMDAPI_015()                                PATCH_METHOD(PATCH_FLAG_470, (u16)470)
    #define PATCH_PBOC_CMDAPI_016()                                PATCH_METHOD(PATCH_FLAG_471, (u16)471)
    #define PATCH_PBOC_CMDAPI_017()                                PATCH_METHOD(PATCH_FLAG_472, (u16)472)
    #define PATCH_PBOC_CMDAPI_018()                                PATCH_METHOD(PATCH_FLAG_473, (u16)473)
    #define PATCH_PBOC_CMDAPI_019()                                PATCH_METHOD(PATCH_FLAG_474, (u16)474)
    #define PATCH_PBOC_CMDAPI_020()                                PATCH_METHOD(PATCH_FLAG_475, (u16)475)
    #define PATCH_PBOC_CMDAPI_021()                                PATCH_METHOD(PATCH_FLAG_476, (u16)476)
    #define PATCH_PBOC_CMDAPI_022()                                PATCH_METHOD(PATCH_FLAG_477, (u16)477)
    #define PATCH_PBOC_CMDAPI_023()                                PATCH_METHOD(PATCH_FLAG_478, (u16)478)
    #define PATCH_PBOC_CMDAPI_024()                                PATCH_METHOD(PATCH_FLAG_479, (u16)479)
    #define PATCH_PBOC_CMDAPI_025()                                PATCH_METHOD(PATCH_FLAG_480, (u16)480)
    #define PATCH_PBOC_CMDAPI_026()                                PATCH_METHOD(PATCH_FLAG_481, (u16)481)
    #define PATCH_PBOC_CMDAPI_027()                                PATCH_METHOD(PATCH_FLAG_482, (u16)482)
    #define PATCH_PBOC_CMDAPI_028()                                PATCH_METHOD(PATCH_FLAG_483, (u16)483)
    #define PATCH_PBOC_CMDAPI_029()                                PATCH_METHOD(PATCH_FLAG_484, (u16)484)
                                                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_Fs.c								*************************************/                                                                                                                             
    #define PATCH_PBOC_FS_001()                                    PATCH_METHOD(PATCH_FLAG_485, (u16)485)
    #define PATCH_PBOC_FS_002()                                    PATCH_METHOD(PATCH_FLAG_486, (u16)486)
    #define PATCH_PBOC_FS_003()                                    PATCH_METHOD(PATCH_FLAG_487, (u16)487)
    #define PATCH_PBOC_FS_004()                                    PATCH_METHOD(PATCH_FLAG_488, (u16)488)
    #define PATCH_PBOC_FS_005()                                    PATCH_METHOD(PATCH_FLAG_489, (u16)489)
    #define PATCH_PBOC_FS_006()                                    PATCH_METHOD(PATCH_FLAG_490, (u16)490)
    #define PATCH_PBOC_FS_007()                                    PATCH_METHOD(PATCH_FLAG_491, (u16)491)
    #define PATCH_PBOC_FS_008()                                    PATCH_METHOD(PATCH_FLAG_492, (u16)492)
    #define PATCH_PBOC_FS_009()                                    PATCH_METHOD(PATCH_FLAG_493, (u16)493)
    #define PATCH_PBOC_FS_010()                                    PATCH_METHOD(PATCH_FLAG_494, (u16)494)
    #define PATCH_PBOC_FS_011()                                    PATCH_METHOD(PATCH_FLAG_495, (u16)495)
    #define PATCH_PBOC_FS_012()                                    PATCH_METHOD(PATCH_FLAG_496, (u16)496)
    #define PATCH_PBOC_FS_013()                                    PATCH_METHOD(PATCH_FLAG_497, (u16)497)
    #define PATCH_PBOC_FS_014()                                    PATCH_METHOD(PATCH_FLAG_498, (u16)498)
    #define PATCH_PBOC_FS_015()                                    PATCH_METHOD(PATCH_FLAG_499, (u16)499)
    #define PATCH_PBOC_FS_016()                                    PATCH_METHOD(PATCH_FLAG_500, (u16)500)
                                                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_GlobalVar.c								*********************************/                                                                                                                         
    #define PATCH_PBOC_GLOBALVAR_001()                             PATCH_METHOD(PATCH_FLAG_501, (u16)501)
    #define PATCH_PBOC_GLOBALVAR_002()                             PATCH_METHOD(PATCH_FLAG_502, (u16)502)
    #define PATCH_PBOC_GLOBALVAR_003()                             PATCH_METHOD(PATCH_FLAG_503, (u16)503)
    #define PATCH_PBOC_GLOBALVAR_004()                             PATCH_METHOD(PATCH_FLAG_504, (u16)504)
	                                                                                                                                                                                                                                                                           
/***************Patch for Pboc--------->ecpPBOC_Main.c								*************************************/	                                                                                                                           
    #define PATCH_PBOC_MAIN_001()                                  PATCH_METHOD(PATCH_FLAG_505, (u16)505)
    #define PATCH_PBOC_MAIN_002()                                  PATCH_METHOD(PATCH_FLAG_506, (u16)506)
    #define PATCH_PBOC_MAIN_003()                                  PATCH_METHOD(PATCH_FLAG_507, (u16)507)
    #define PATCH_PBOC_MAIN_004()                                  PATCH_METHOD(PATCH_FLAG_508, (u16)508)
    #define PATCH_PBOC_MAIN_005()                                  PATCH_METHOD(PATCH_FLAG_509, (u16)509)
    #define PATCH_PBOC_MAIN_006()                                  PATCH_METHOD(PATCH_FLAG_510, (u16)510)
    #define PATCH_PBOC_MAIN_007()                                  PATCH_METHOD(PATCH_FLAG_511, (u16)511)
    #define PATCH_PBOC_MAIN_008()                                  PATCH_METHOD(PATCH_FLAG_512, (u16)512)
    #define PATCH_PBOC_MAIN_009()                                  PATCH_METHOD(PATCH_FLAG_513, (u16)513)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_Mem.c								*************************************/	                                                                                                                           
    #define PATCH_PBOC_MEM_001()                                   PATCH_METHOD(PATCH_FLAG_514, (u16)514)
    #define PATCH_PBOC_MEM_002()                                   PATCH_METHOD(PATCH_FLAG_515, (u16)515)
    #define PATCH_PBOC_MEM_003()                                   PATCH_METHOD(PATCH_FLAG_516, (u16)516)
    #define PATCH_PBOC_MEM_004()                                   PATCH_METHOD(PATCH_FLAG_517, (u16)517)
    #define PATCH_PBOC_MEM_005()                                   PATCH_METHOD(PATCH_FLAG_518, (u16)518)
    #define PATCH_PBOC_MEM_006()                                   PATCH_METHOD(PATCH_FLAG_519, (u16)519)
    #define PATCH_PBOC_MEM_007()                                   PATCH_METHOD(PATCH_FLAG_520, (u16)520)
    #define PATCH_PBOC_MEM_008()                                   PATCH_METHOD(PATCH_FLAG_521, (u16)521)
    #define PATCH_PBOC_MEM_009()                                   PATCH_METHOD(PATCH_FLAG_522, (u16)522)
    #define PATCH_PBOC_MEM_010()                                   PATCH_METHOD(PATCH_FLAG_523, (u16)523)
    #define PATCH_PBOC_MEM_011()                                   PATCH_METHOD(PATCH_FLAG_524, (u16)524)
    #define PATCH_PBOC_MEM_012()                                   PATCH_METHOD(PATCH_FLAG_525, (u16)525)
    #define PATCH_PBOC_MEM_013()                                   PATCH_METHOD(PATCH_FLAG_526, (u16)526)
    #define PATCH_PBOC_MEM_014()                                   PATCH_METHOD(PATCH_FLAG_527, (u16)527)
    #define PATCH_PBOC_MEM_015()                                   PATCH_METHOD(PATCH_FLAG_528, (u16)528)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_PerInit.c								*************************************/	                                                                                                                   
    #define PATCH_PBOC_PERINIT_001()                               PATCH_METHOD(PATCH_FLAG_529, (u16)529)
    #define PATCH_PBOC_PERINIT_002()                               PATCH_METHOD(PATCH_FLAG_530, (u16)530)
    #define PATCH_PBOC_PERINIT_003()                               PATCH_METHOD(PATCH_FLAG_531, (u16)531)
    #define PATCH_PBOC_PERINIT_004()                               PATCH_METHOD(PATCH_FLAG_532, (u16)532)
    #define PATCH_PBOC_PERINIT_005()                               PATCH_METHOD(PATCH_FLAG_533, (u16)533)
    #define PATCH_PBOC_PERINIT_006()                               PATCH_METHOD(PATCH_FLAG_534, (u16)534)
    #define PATCH_PBOC_PERINIT_007()                               PATCH_METHOD(PATCH_FLAG_535, (u16)535)
    #define PATCH_PBOC_PERINIT_008()                               PATCH_METHOD(PATCH_FLAG_536, (u16)536)
    #define PATCH_PBOC_PERINIT_009()                               PATCH_METHOD(PATCH_FLAG_537, (u16)537)
    #define PATCH_PBOC_PERINIT_010()                               PATCH_METHOD(PATCH_FLAG_538, (u16)538)
    #define PATCH_PBOC_PERINIT_011()                               PATCH_METHOD(PATCH_FLAG_539, (u16)539)
    #define PATCH_PBOC_PERINIT_012()                               PATCH_METHOD(PATCH_FLAG_540, (u16)540)
    #define PATCH_PBOC_PERINIT_013()                               PATCH_METHOD(PATCH_FLAG_541, (u16)541)
    #define PATCH_PBOC_PERINIT_014()                               PATCH_METHOD(PATCH_FLAG_542, (u16)542)
    #define PATCH_PBOC_PERINIT_015()                               PATCH_METHOD(PATCH_FLAG_543, (u16)543)
    #define PATCH_PBOC_PERINIT_016()                               PATCH_METHOD(PATCH_FLAG_544, (u16)544)
    #define PATCH_PBOC_PERINIT_017()                               PATCH_METHOD(PATCH_FLAG_545, (u16)545)
    #define PATCH_PBOC_PERINIT_018()                               PATCH_METHOD(PATCH_FLAG_546, (u16)546)
    #define PATCH_PBOC_PERINIT_019()                               PATCH_METHOD(PATCH_FLAG_547, (u16)547)
    #define PATCH_PBOC_PERINIT_020()                               PATCH_METHOD(PATCH_FLAG_548, (u16)548)
    #define PATCH_PBOC_PERINIT_021()                               PATCH_METHOD(PATCH_FLAG_549, (u16)549)
    #define PATCH_PBOC_PERINIT_022()                               PATCH_METHOD(PATCH_FLAG_550, (u16)550)
    #define PATCH_PBOC_PERINIT_023()                               PATCH_METHOD(PATCH_FLAG_551, (u16)551)
    #define PATCH_PBOC_PERINIT_024()                               PATCH_METHOD(PATCH_FLAG_552, (u16)552)
    #define PATCH_PBOC_PERINIT_025()                               PATCH_METHOD(PATCH_FLAG_553, (u16)553)
    #define PATCH_PBOC_PERINIT_026()                               PATCH_METHOD(PATCH_FLAG_554, (u16)554)
    #define PATCH_PBOC_PERINIT_027()                               PATCH_METHOD(PATCH_FLAG_555, (u16)555)
    #define PATCH_PBOC_PERINIT_028()                               PATCH_METHOD(PATCH_FLAG_556, (u16)556)
    #define PATCH_PBOC_PERINIT_029()                               PATCH_METHOD(PATCH_FLAG_557, (u16)557)
    #define PATCH_PBOC_PERINIT_030()                               PATCH_METHOD(PATCH_FLAG_558, (u16)558)
    #define PATCH_PBOC_PERINIT_031()                               PATCH_METHOD(PATCH_FLAG_559, (u16)559)
    #define PATCH_PBOC_PERINIT_032()                               PATCH_METHOD(PATCH_FLAG_560, (u16)560)
    #define PATCH_PBOC_PERINIT_033()                               PATCH_METHOD(PATCH_FLAG_561, (u16)561)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_PlatformApi.c					*************************************/	                                                                                                                                                                                                                                                                                                        
    #define PATCH_PBOC_PLATFORMAPI_001()                           PATCH_METHOD(PATCH_FLAG_562, (u16)562)
    #define PATCH_PBOC_PLATFORMAPI_002()                           PATCH_METHOD(PATCH_FLAG_563, (u16)563)
    #define PATCH_PBOC_PLATFORMAPI_003()                           PATCH_METHOD(PATCH_FLAG_564, (u16)564)
    #define PATCH_PBOC_PLATFORMAPI_004()                           PATCH_METHOD(PATCH_FLAG_565, (u16)565)
    #define PATCH_PBOC_PLATFORMAPI_005()                           PATCH_METHOD(PATCH_FLAG_566, (u16)566)
    #define PATCH_PBOC_PLATFORMAPI_006()                           PATCH_METHOD(PATCH_FLAG_567, (u16)567)
    #define PATCH_PBOC_PLATFORMAPI_007()                           PATCH_METHOD(PATCH_FLAG_568, (u16)568)
    #define PATCH_PBOC_PLATFORMAPI_008()                           PATCH_METHOD(PATCH_FLAG_569, (u16)569)
    #define PATCH_PBOC_PLATFORMAPI_009()                           PATCH_METHOD(PATCH_FLAG_570, (u16)570)
    #define PATCH_PBOC_PLATFORMAPI_010()                           PATCH_METHOD(PATCH_FLAG_571, (u16)571)
    #define PATCH_PBOC_PLATFORMAPI_011()                           PATCH_METHOD(PATCH_FLAG_572, (u16)572)
    #define PATCH_PBOC_PLATFORMAPI_012()                           PATCH_METHOD(PATCH_FLAG_573, (u16)573)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_TDA.c					**********************************************/                                                                                                                                            
    #define PATCH_PBOC_TDA_001()                                   PATCH_METHOD(PATCH_FLAG_574, (u16)574)
    #define PATCH_PBOC_TDA_001_1()                                 PATCH_METHOD(PATCH_FLAG_575, (u16)575)
    #define PATCH_PBOC_TDA_001_2()                                 PATCH_METHOD(PATCH_FLAG_576, (u16)576)
    #define PATCH_PBOC_TDA_002()                                   PATCH_METHOD(PATCH_FLAG_577, (u16)577)
    #define PATCH_PBOC_TDA_003()                                   PATCH_METHOD(PATCH_FLAG_578, (u16)578)
    #define PATCH_PBOC_TDA_004()                                   PATCH_METHOD(PATCH_FLAG_579, (u16)579)
    #define PATCH_PBOC_TDA_005()                                   PATCH_METHOD(PATCH_FLAG_580, (u16)580)
    #define PATCH_PBOC_TDA_006()                                   PATCH_METHOD(PATCH_FLAG_581, (u16)581)
    #define PATCH_PBOC_TDA_007()                                   PATCH_METHOD(PATCH_FLAG_582, (u16)582)
    #define PATCH_PBOC_TDA_008()                                   PATCH_METHOD(PATCH_FLAG_583, (u16)583)
    #define PATCH_PBOC_TDA_009()                                   PATCH_METHOD(PATCH_FLAG_584, (u16)584)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpPBOC_TLV.c					**********************************************/                                                                                                                                            
    #define PATCH_PBOC_TLV_001()                                   PATCH_METHOD(PATCH_FLAG_585, (u16)585)
    #define PATCH_PBOC_TLV_002()                                   PATCH_METHOD(PATCH_FLAG_586, (u16)586)
    #define PATCH_PBOC_TLV_003()                                   PATCH_METHOD(PATCH_FLAG_587, (u16)587)
    #define PATCH_PBOC_TLV_004()                                   PATCH_METHOD(PATCH_FLAG_588, (u16)588)
    #define PATCH_PBOC_TLV_005()                                   PATCH_METHOD(PATCH_FLAG_589, (u16)589)
    #define PATCH_PBOC_TLV_006()                                   PATCH_METHOD(PATCH_FLAG_590, (u16)590)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->ecpQPBOC_APPCommand.c	**********************************************/                                                                                                                                                                    
    #define PATCH_QPBOC_APPCMD_001()                               PATCH_METHOD(PATCH_FLAG_591, (u16)591)
    #define PATCH_QPBOC_APPCMD_002()                               PATCH_METHOD(PATCH_FLAG_592, (u16)592)
    #define PATCH_QPBOC_APPCMD_003()                               PATCH_METHOD(PATCH_FLAG_593, (u16)593)
    #define PATCH_QPBOC_APPCMD_004()                               PATCH_METHOD(PATCH_FLAG_594, (u16)594)
    #define PATCH_QPBOC_APPCMD_005()                               PATCH_METHOD(PATCH_FLAG_595, (u16)595)
    #define PATCH_QPBOC_APPCMD_006()                               PATCH_METHOD(PATCH_FLAG_596, (u16)596)
    #define PATCH_QPBOC_APPCMD_007()                               PATCH_METHOD(PATCH_FLAG_597, (u16)597)
    #define PATCH_QPBOC_APPCMD_008()                               PATCH_METHOD(PATCH_FLAG_598, (u16)598)
    #define PATCH_QPBOC_APPCMD_009()                               PATCH_METHOD(PATCH_FLAG_599, (u16)599)
    #define PATCH_QPBOC_APPCMD_010()                               PATCH_METHOD(PATCH_FLAG_600, (u16)600)
    #define PATCH_QPBOC_APPCMD_011()                               PATCH_METHOD(PATCH_FLAG_601, (u16)601)
    #define PATCH_QPBOC_APPCMD_012()                               PATCH_METHOD(PATCH_FLAG_602, (u16)602)
    #define PATCH_QPBOC_APPCMD_013()                               PATCH_METHOD(PATCH_FLAG_603, (u16)603)
    #define PATCH_QPBOC_APPCMD_014()                               PATCH_METHOD(PATCH_FLAG_604, (u16)604)
    #define PATCH_QPBOC_APPCMD_015()                               PATCH_METHOD(PATCH_FLAG_605, (u16)605)
    #define PATCH_QPBOC_APPCMD_016()                               PATCH_METHOD(PATCH_FLAG_606, (u16)606)
    #define PATCH_QPBOC_APPCMD_017()                               PATCH_METHOD(PATCH_FLAG_607, (u16)607)
    #define PATCH_QPBOC_APPCMD_018()                               PATCH_METHOD(PATCH_FLAG_608, (u16)608)
    #define PATCH_QPBOC_APPCMD_019()                               PATCH_METHOD(PATCH_FLAG_609, (u16)609)
    #define PATCH_QPBOC_APPCMD_020()                               PATCH_METHOD(PATCH_FLAG_610, (u16)610)
                                                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_Algorithm.c	**********************************************/                                                                                                                                                                    
    #define PATCH_EDEP_ALG_001()                                   PATCH_METHOD(PATCH_FLAG_611, (u16)611)
    #define PATCH_EDEP_ALG_002()                                   PATCH_METHOD(PATCH_FLAG_612, (u16)612)
    #define PATCH_EDEP_ALG_003()                                   PATCH_METHOD(PATCH_FLAG_613, (u16)613)
    #define PATCH_EDEP_ALG_004()                                   PATCH_METHOD(PATCH_FLAG_614, (u16)614)
    #define PATCH_EDEP_ALG_005()                                   PATCH_METHOD(PATCH_FLAG_615, (u16)615)
    #define PATCH_EDEP_ALG_006()                                   PATCH_METHOD(PATCH_FLAG_616, (u16)616)
    #define PATCH_EDEP_ALG_007()                                   PATCH_METHOD(PATCH_FLAG_617, (u16)617)

                                                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_app.c	*****************************************************/                                                                                                                                                                     
    #define PATCH_EDEP_APP_001()                                   PATCH_METHOD(PATCH_FLAG_618, (u16)618)
    #define PATCH_EDEP_APP_002()                                   PATCH_METHOD(PATCH_FLAG_619, (u16)619)
    #define PATCH_EDEP_APP_003()                                   PATCH_METHOD(PATCH_FLAG_620, (u16)620)
    #define PATCH_EDEP_APP_004()                                   PATCH_METHOD(PATCH_FLAG_621, (u16)621)
    #define PATCH_EDEP_APP_005()                                   PATCH_METHOD(PATCH_FLAG_622, (u16)622)
    #define PATCH_EDEP_APP_006()                                   PATCH_METHOD(PATCH_FLAG_623, (u16)623)
    #define PATCH_EDEP_APP_007()                                   PATCH_METHOD(PATCH_FLAG_624, (u16)624)
    #define PATCH_EDEP_APP_008()                                   PATCH_METHOD(PATCH_FLAG_625, (u16)625)
    #define PATCH_EDEP_APP_009()                                   PATCH_METHOD(PATCH_FLAG_626, (u16)626)
    #define PATCH_EDEP_APP_010()                                   PATCH_METHOD(PATCH_FLAG_627, (u16)627)
    #define PATCH_EDEP_APP_011()                                   PATCH_METHOD(PATCH_FLAG_628, (u16)628)
    #define PATCH_EDEP_APP_012()                                   PATCH_METHOD(PATCH_FLAG_629, (u16)629)
    #define PATCH_EDEP_APP_013()                                   PATCH_METHOD(PATCH_FLAG_630, (u16)630)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_CmdInterface.c	**********************************************/                                                                                                                                                                    
    #define PATCH_EDEP_CMDINTERFACE_001()                          PATCH_METHOD(PATCH_FLAG_631, (u16)631)
    #define PATCH_EDEP_CMDINTERFACE_002()                          PATCH_METHOD(PATCH_FLAG_632, (u16)632)
    #define PATCH_EDEP_CMDINTERFACE_003()                          PATCH_METHOD(PATCH_FLAG_633, (u16)633)
    #define PATCH_EDEP_CMDINTERFACE_004()                          PATCH_METHOD(PATCH_FLAG_634, (u16)634)
    #define PATCH_EDEP_CMDINTERFACE_005()                          PATCH_METHOD(PATCH_FLAG_635, (u16)635)
    #define PATCH_EDEP_CMDINTERFACE_006()                          PATCH_METHOD(PATCH_FLAG_636, (u16)636)
    #define PATCH_EDEP_CMDINTERFACE_007()                          PATCH_METHOD(PATCH_FLAG_637, (u16)637)
    #define PATCH_EDEP_CMDINTERFACE_008()                          PATCH_METHOD(PATCH_FLAG_638, (u16)638)
    #define PATCH_EDEP_CMDINTERFACE_009()                          PATCH_METHOD(PATCH_FLAG_639, (u16)639)
    #define PATCH_EDEP_CMDINTERFACE_010()                          PATCH_METHOD(PATCH_FLAG_640, (u16)640)
    #define PATCH_EDEP_CMDINTERFACE_011()                          PATCH_METHOD(PATCH_FLAG_641, (u16)641)
    #define PATCH_EDEP_CMDINTERFACE_012()                          PATCH_METHOD(PATCH_FLAG_642, (u16)642)
    #define PATCH_EDEP_CMDINTERFACE_013()                          PATCH_METHOD(PATCH_FLAG_643, (u16)643)
    #define PATCH_EDEP_CMDINTERFACE_014()                          PATCH_METHOD(PATCH_FLAG_644, (u16)644)
    #define PATCH_EDEP_CMDINTERFACE_015()                          PATCH_METHOD(PATCH_FLAG_645, (u16)645)
    #define PATCH_EDEP_CMDINTERFACE_016()                          PATCH_METHOD(PATCH_FLAG_646, (u16)646)
    #define PATCH_EDEP_CMDINTERFACE_017()                          PATCH_METHOD(PATCH_FLAG_647, (u16)647)
    #define PATCH_EDEP_CMDINTERFACE_018()                          PATCH_METHOD(PATCH_FLAG_648, (u16)648)
    #define PATCH_EDEP_CMDINTERFACE_019()                          PATCH_METHOD(PATCH_FLAG_649, (u16)649)
    #define PATCH_EDEP_CMDINTERFACE_020()                          PATCH_METHOD(PATCH_FLAG_650, (u16)650)
    #define PATCH_EDEP_CMDINTERFACE_021()                          PATCH_METHOD(PATCH_FLAG_651, (u16)651)
    #define PATCH_EDEP_CMDINTERFACE_022()                          PATCH_METHOD(PATCH_FLAG_652, (u16)652)
/***************Patch for Pboc--------->EDEP_Construction_App.c	**********************************************/                                                                                                                                                                    
    #define PATCH_EDEP_CONSTRUCTION_APP_001()                      PATCH_METHOD(PATCH_FLAG_653, (u16)653)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_Eeprom_func.c	**************************************************/                                                                                                                                                                
    #define PATCH_EDEP_EE_FUNC_001()                               PATCH_METHOD(PATCH_FLAG_654, (u16)654)
    #define PATCH_EDEP_EE_FUNC_002()                               PATCH_METHOD(PATCH_FLAG_655, (u16)655)
    #define PATCH_EDEP_EE_FUNC_003()                               PATCH_METHOD(PATCH_FLAG_656, (u16)656)
    #define PATCH_EDEP_EE_FUNC_004()                               PATCH_METHOD(PATCH_FLAG_657, (u16)657)
    #define PATCH_EDEP_EE_FUNC_005()                               PATCH_METHOD(PATCH_FLAG_658, (u16)658)
    #define PATCH_EDEP_EE_FUNC_006()                               PATCH_METHOD(PATCH_FLAG_659, (u16)659)
    #define PATCH_EDEP_EE_FUNC_007()                               PATCH_METHOD(PATCH_FLAG_660, (u16)660)
    #define PATCH_EDEP_EE_FUNC_008()                               PATCH_METHOD(PATCH_FLAG_661, (u16)661)
    #define PATCH_EDEP_EE_FUNC_009()                               PATCH_METHOD(PATCH_FLAG_662, (u16)662)
    #define PATCH_EDEP_EE_FUNC_010()                               PATCH_METHOD(PATCH_FLAG_663, (u16)663)
    #define PATCH_EDEP_EE_FUNC_011()                               PATCH_METHOD(PATCH_FLAG_664, (u16)664)
                                                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_EP_EXTENSION.c	**************************************************/                                                                                                                                                                
    #define PATCH_EDEP_EXTENSION_001()                             PATCH_METHOD(PATCH_FLAG_665, (u16)665)
    #define PATCH_EDEP_EXTENSION_002()                             PATCH_METHOD(PATCH_FLAG_666, (u16)666)
    #define PATCH_EDEP_EXTENSION_003()                             PATCH_METHOD(PATCH_FLAG_667, (u16)667)
    #define PATCH_EDEP_EXTENSION_004()                             PATCH_METHOD(PATCH_FLAG_668, (u16)668)
    #define PATCH_EDEP_EXTENSION_005()                             PATCH_METHOD(PATCH_FLAG_669, (u16)669)
    #define PATCH_EDEP_EXTENSION_006()                             PATCH_METHOD(PATCH_FLAG_670, (u16)670)
    #define PATCH_EDEP_EXTENSION_007()                             PATCH_METHOD(PATCH_FLAG_671, (u16)671)
                                                                                                                                                                                                                                        
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_fs.c	*************************************************************/                                                                                                                                                             
    #define PATCH_EDEP_FS_001()                                    PATCH_METHOD(PATCH_FLAG_672, (u16)672)
    #define PATCH_EDEP_FS_002()                                    PATCH_METHOD(PATCH_FLAG_673, (u16)673)
    #define PATCH_EDEP_FS_003()                                    PATCH_METHOD(PATCH_FLAG_674, (u16)674)
    #define PATCH_EDEP_FS_004()                                    PATCH_METHOD(PATCH_FLAG_675, (u16)675)
    #define PATCH_EDEP_FS_005()                                    PATCH_METHOD(PATCH_FLAG_676, (u16)676)
    #define PATCH_EDEP_FS_006()                                    PATCH_METHOD(PATCH_FLAG_677, (u16)677)
    #define PATCH_EDEP_FS_007()                                    PATCH_METHOD(PATCH_FLAG_678, (u16)678)
    #define PATCH_EDEP_FS_008()                                    PATCH_METHOD(PATCH_FLAG_679, (u16)679)
    #define PATCH_EDEP_FS_009()                                    PATCH_METHOD(PATCH_FLAG_680, (u16)680)
    #define PATCH_EDEP_FS_010()                                    PATCH_METHOD(PATCH_FLAG_681, (u16)681)
    #define PATCH_EDEP_FS_011()                                    PATCH_METHOD(PATCH_FLAG_682, (u16)682)
    #define PATCH_EDEP_FS_012()                                    PATCH_METHOD(PATCH_FLAG_683, (u16)683)
    #define PATCH_EDEP_FS_013()                                    PATCH_METHOD(PATCH_FLAG_684, (u16)684)
    #define PATCH_EDEP_FS_014()                                    PATCH_METHOD(PATCH_FLAG_685, (u16)685)
    #define PATCH_EDEP_FS_015()                                    PATCH_METHOD(PATCH_FLAG_686, (u16)686)
    #define PATCH_EDEP_FS_016()                                    PATCH_METHOD(PATCH_FLAG_687, (u16)687)
    #define PATCH_EDEP_FS_017()                                    PATCH_METHOD(PATCH_FLAG_688, (u16)688)
    #define PATCH_EDEP_FS_018()                                    PATCH_METHOD(PATCH_FLAG_689, (u16)689)
    #define PATCH_EDEP_FS_019()                                    PATCH_METHOD(PATCH_FLAG_690, (u16)690)
    #define PATCH_EDEP_FS_020()                                    PATCH_METHOD(PATCH_FLAG_691, (u16)691)
    #define PATCH_EDEP_FS_021()                                    PATCH_METHOD(PATCH_FLAG_692, (u16)692)
                                                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_main.c	*************************************************************/                                                                                                                                                             
                                                                                                                                                                                                                                                                                   
    #define PATCH_EDEP_MAIN_001()                                  PATCH_METHOD(PATCH_FLAG_693, (u16)693)
    #define PATCH_EDEP_MAIN_002()                                  PATCH_METHOD(PATCH_FLAG_694, (u16)694)
    #define PATCH_EDEP_MAIN_003()                                  PATCH_METHOD(PATCH_FLAG_695, (u16)695)
    #define PATCH_EDEP_MAIN_004()                                  PATCH_METHOD(PATCH_FLAG_696, (u16)696)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_pin_key.c	**********************************************************/                                                                                                                                                                
                                                                                                                                                                                                                                                                                   
    #define PATCH_EDEP_PINKEY_001()                                PATCH_METHOD(PATCH_FLAG_697, (u16)697)
    #define PATCH_EDEP_PINKEY_002()                                PATCH_METHOD(PATCH_FLAG_698, (u16)698)
    #define PATCH_EDEP_PINKEY_003()                                PATCH_METHOD(PATCH_FLAG_699, (u16)699)
    #define PATCH_EDEP_PINKEY_004()                                PATCH_METHOD(PATCH_FLAG_700, (u16)700)
    #define PATCH_EDEP_PINKEY_005()                                PATCH_METHOD(PATCH_FLAG_701, (u16)701)
    #define PATCH_EDEP_PINKEY_006()                                PATCH_METHOD(PATCH_FLAG_702, (u16)702)
    #define PATCH_EDEP_PINKEY_007()                                PATCH_METHOD(PATCH_FLAG_703, (u16)703)
    #define PATCH_EDEP_PINKEY_008()                                PATCH_METHOD(PATCH_FLAG_704, (u16)704)
    #define PATCH_EDEP_PINKEY_009()                                PATCH_METHOD(PATCH_FLAG_705, (u16)705)
    #define PATCH_EDEP_PINKEY_010()                                PATCH_METHOD(PATCH_FLAG_706, (u16)706)
    #define PATCH_EDEP_PINKEY_011()                                PATCH_METHOD(PATCH_FLAG_707, (u16)707)
    #define PATCH_EDEP_PINKEY_012()                                PATCH_METHOD(PATCH_FLAG_708, (u16)708)
    #define PATCH_EDEP_PINKEY_013()                                PATCH_METHOD(PATCH_FLAG_709, (u16)709)
    #define PATCH_EDEP_PINKEY_014()                                PATCH_METHOD(PATCH_FLAG_710, (u16)710)
    #define PATCH_EDEP_PINKEY_015()                                PATCH_METHOD(PATCH_FLAG_711, (u16)711)
    #define PATCH_EDEP_PINKEY_016()                                PATCH_METHOD(PATCH_FLAG_712, (u16)712)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_record_func.c	********************************************************/                                                                                                                                                          
                                                                                                                                                                                                                                                                                   
    #define PATCH_EDEP_RECORD_FUNC_001()                           PATCH_METHOD(PATCH_FLAG_713, (u16)713)
    #define PATCH_EDEP_RECORD_FUNC_002()                           PATCH_METHOD(PATCH_FLAG_714, (u16)714)
    #define PATCH_EDEP_RECORD_FUNC_003()                           PATCH_METHOD(PATCH_FLAG_715, (u16)715)
    #define PATCH_EDEP_RECORD_FUNC_004()                           PATCH_METHOD(PATCH_FLAG_716, (u16)716)
    #define PATCH_EDEP_RECORD_FUNC_005()                           PATCH_METHOD(PATCH_FLAG_717, (u16)717)
    #define PATCH_EDEP_RECORD_FUNC_006()                           PATCH_METHOD(PATCH_FLAG_718, (u16)718)
    #define PATCH_EDEP_RECORD_FUNC_007()                           PATCH_METHOD(PATCH_FLAG_719, (u16)719)
    #define PATCH_EDEP_RECORD_FUNC_008()                           PATCH_METHOD(PATCH_FLAG_720, (u16)720)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_TJYKT.c	**************************************************************/                                                                                                                                                            
    #define PATCH_EDEP_TJYKT_001()                                 PATCH_METHOD(PATCH_FLAG_721, (u16)721)
    #define PATCH_EDEP_TJYKT_002()                                 PATCH_METHOD(PATCH_FLAG_722, (u16)722)
    #define PATCH_EDEP_TJYKT_003()                                 PATCH_METHOD(PATCH_FLAG_723, (u16)723)
    #define PATCH_EDEP_TJYKT_004()                                 PATCH_METHOD(PATCH_FLAG_724, (u16)724)
                                                                                                                                                                                                                                                                                   
/***************Patch for Pboc--------->EDEP_YCT_specail.c	********************************************************/                                                                                                                                                          
    #define PATCH_EDEP_YKT_001()                                   PATCH_METHOD(PATCH_FLAG_725, (u16)725)
    #define PATCH_EDEP_YKT_002()                                   PATCH_METHOD(PATCH_FLAG_726, (u16)726)

/***************Patch for Pboc--------->BackUp For Use	************************************************************/
    #define PATCH_PBOC_BACKUP_001()                                PATCH_METHOD(PATCH_FLAG_727, (u16)727)
    #define PATCH_PBOC_BACKUP_002()                                PATCH_METHOD(PATCH_FLAG_728, (u16)728)
    #define PATCH_PBOC_BACKUP_003()                                PATCH_METHOD(PATCH_FLAG_729, (u16)729)
    #define PATCH_PBOC_BACKUP_004()                                PATCH_METHOD(PATCH_FLAG_730, (u16)730)
    #define PATCH_PBOC_BACKUP_005()                                PATCH_METHOD(PATCH_FLAG_731, (u16)731)
    #define PATCH_PBOC_BACKUP_006()                                PATCH_METHOD(PATCH_FLAG_732, (u16)732)
    #define PATCH_PBOC_BACKUP_007()                                PATCH_METHOD(PATCH_FLAG_733, (u16)733)
    #define PATCH_PBOC_BACKUP_008()                                PATCH_METHOD(PATCH_FLAG_734, (u16)734)
    #define PATCH_PBOC_BACKUP_009()                                PATCH_METHOD(PATCH_FLAG_735, (u16)735)
    #define PATCH_PBOC_BACKUP_010()                                PATCH_METHOD(PATCH_FLAG_736, (u16)736)
    #define PATCH_PBOC_BACKUP_011()                                PATCH_METHOD(PATCH_FLAG_737, (u16)737)
    #define PATCH_PBOC_BACKUP_012()                                PATCH_METHOD(PATCH_FLAG_738, (u16)738)

/***************Patch for EID--------->ecpEID_Main.c	********************************************************/       
    #define PATCH_EID_MAIN_001()                                   PATCH_METHOD(PATCH_FLAG_739, (u16)739)
    #define PATCH_EID_MAIN_002()                                   PATCH_METHOD(PATCH_FLAG_740, (u16)740)

/***************Patch for EID--------->ecpEID_Fs.c	********************************************************/       
    #define PATCH_EID_FS_001()                                     PATCH_METHOD(PATCH_FLAG_741, (u16)741)
    #define PATCH_EID_FS_002()                                     PATCH_METHOD(PATCH_FLAG_742, (u16)742)
    #define PATCH_EID_FS_003()                                     PATCH_METHOD(PATCH_FLAG_743, (u16)743)
    #define PATCH_EID_FS_004()                                     PATCH_METHOD(PATCH_FLAG_744, (u16)744)
    #define PATCH_EID_FS_005()                                     PATCH_METHOD(PATCH_FLAG_745, (u16)745)
    #define PATCH_EID_FS_006()                                     PATCH_METHOD(PATCH_FLAG_746, (u16)746)
    #define PATCH_EID_FS_007()                                     PATCH_METHOD(PATCH_FLAG_747, (u16)747)
    #define PATCH_EID_FS_008()                                     PATCH_METHOD(PATCH_FLAG_748, (u16)748)
    #define PATCH_EID_FS_009()                                     PATCH_METHOD(PATCH_FLAG_749, (u16)749)
    #define PATCH_EID_FS_010()                                     PATCH_METHOD(PATCH_FLAG_750, (u16)750)
	
    #define PATCH_EID_FS_011()                                     PATCH_METHOD(PATCH_FLAG_751, (u16)751)
    #define PATCH_EID_FS_012()                                     PATCH_METHOD(PATCH_FLAG_752, (u16)752)
    #define PATCH_EID_FS_013()                                     PATCH_METHOD(PATCH_FLAG_753, (u16)753)
/***************Patch for EID--------->ecpEID_CmdApi.c	********************************************************/       
    #define PATCH_EID_CMDAPI_001()                                 PATCH_METHOD(PATCH_FLAG_754, (u16)754)
    #define PATCH_EID_CMDAPI_002()                                 PATCH_METHOD(PATCH_FLAG_755, (u16)755)
    #define PATCH_EID_CMDAPI_003()                                 PATCH_METHOD(PATCH_FLAG_756, (u16)756)
    #define PATCH_EID_CMDAPI_004()                                 PATCH_METHOD(PATCH_FLAG_757, (u16)757)
    #define PATCH_EID_CMDAPI_005()                                 PATCH_METHOD(PATCH_FLAG_758, (u16)758)
    #define PATCH_EID_CMDAPI_006()                                 PATCH_METHOD(PATCH_FLAG_759, (u16)759)
    #define PATCH_EID_CMDAPI_007()                                 PATCH_METHOD(PATCH_FLAG_760, (u16)760)
    #define PATCH_EID_CMDAPI_008()                                 PATCH_METHOD(PATCH_FLAG_761, (u16)761)
    #define PATCH_EID_CMDAPI_009()                                 PATCH_METHOD(PATCH_FLAG_762, (u16)762)
    #define PATCH_EID_CMDAPI_010()                                 PATCH_METHOD(PATCH_FLAG_763, (u16)763)
    #define PATCH_EID_CMDAPI_011()                                 PATCH_METHOD(PATCH_FLAG_764, (u16)764)
    #define PATCH_EID_CMDAPI_012()                                 PATCH_METHOD(PATCH_FLAG_765, (u16)765)
    #define PATCH_EID_CMDAPI_013()                                 PATCH_METHOD(PATCH_FLAG_766, (u16)766)
    #define PATCH_EID_CMDAPI_014()                                 PATCH_METHOD(PATCH_FLAG_767, (u16)767)
    #define PATCH_EID_CMDAPI_015()                                 PATCH_METHOD(PATCH_FLAG_768, (u16)768)
    #define PATCH_EID_CMDAPI_016()                                 PATCH_METHOD(PATCH_FLAG_769, (u16)769)
    #define PATCH_EID_CMDAPI_017()                                 PATCH_METHOD(PATCH_FLAG_770, (u16)770)
    #define PATCH_EID_CMDAPI_018()                                 PATCH_METHOD(PATCH_FLAG_771, (u16)771)
    #define PATCH_EID_CMDAPI_019()                                 PATCH_METHOD(PATCH_FLAG_772, (u16)772)
    #define PATCH_EID_CMDAPI_020()                                 PATCH_METHOD(PATCH_FLAG_773, (u16)773)
    #define PATCH_EID_CMDAPI_021()                                 PATCH_METHOD(PATCH_FLAG_774, (u16)774)
    #define PATCH_EID_CMDAPI_022()                                 PATCH_METHOD(PATCH_FLAG_775, (u16)775)
    #define PATCH_EID_CMDAPI_023()                                 PATCH_METHOD(PATCH_FLAG_776, (u16)776)
    #define PATCH_EID_CMDAPI_024()                                 PATCH_METHOD(PATCH_FLAG_777, (u16)777)
    #define PATCH_EID_CMDAPI_025()                                 PATCH_METHOD(PATCH_FLAG_778, (u16)778)
    #define PATCH_EID_CMDAPI_026()                                 PATCH_METHOD(PATCH_FLAG_779, (u16)779)
    #define PATCH_EID_CMDAPI_027()                                 PATCH_METHOD(PATCH_FLAG_780, (u16)780)
    #define PATCH_EID_CMDAPI_028()                                 PATCH_METHOD(PATCH_FLAG_781, (u16)781)
    #define PATCH_EID_CMDAPI_029()                                 PATCH_METHOD(PATCH_FLAG_782, (u16)782)
	
/***************Patch for EID--------->ecpEID_AppCommand.c	********************************************************/       
    #define PATCH_EID_APPCMD_001()                                 PATCH_METHOD(PATCH_FLAG_783, (u16)783)
    #define PATCH_EID_APPCMD_002()                                 PATCH_METHOD(PATCH_FLAG_784, (u16)784)
    #define PATCH_EID_APPCMD_003()                                 PATCH_METHOD(PATCH_FLAG_785, (u16)785)
    #define PATCH_EID_APPCMD_004()                                 PATCH_METHOD(PATCH_FLAG_786, (u16)786)
    #define PATCH_EID_APPCMD_005()                                 PATCH_METHOD(PATCH_FLAG_787, (u16)787)
    #define PATCH_EID_APPCMD_006()                                 PATCH_METHOD(PATCH_FLAG_788, (u16)788)
    #define PATCH_EID_APPCMD_007()                                 PATCH_METHOD(PATCH_FLAG_789, (u16)789)
    #define PATCH_EID_APPCMD_008()                                 PATCH_METHOD(PATCH_FLAG_790, (u16)790)
    #define PATCH_EID_APPCMD_009()                                 PATCH_METHOD(PATCH_FLAG_791, (u16)791)
    #define PATCH_EID_APPCMD_010()                                 PATCH_METHOD(PATCH_FLAG_792, (u16)792)
    #define PATCH_EID_APPCMD_011()                                 PATCH_METHOD(PATCH_FLAG_793, (u16)793)
    #define PATCH_EID_APPCMD_012()                                 PATCH_METHOD(PATCH_FLAG_794, (u16)794)
    #define PATCH_EID_APPCMD_013()                                 PATCH_METHOD(PATCH_FLAG_795, (u16)795)
    #define PATCH_EID_APPCMD_014()                                 PATCH_METHOD(PATCH_FLAG_796, (u16)796)
    #define PATCH_EID_APPCMD_015()                                 PATCH_METHOD(PATCH_FLAG_797, (u16)797)
    #define PATCH_EID_APPCMD_016()                                 PATCH_METHOD(PATCH_FLAG_798, (u16)798)
    #define PATCH_EID_APPCMD_017()                                 PATCH_METHOD(PATCH_FLAG_799, (u16)799)
    #define PATCH_EID_APPCMD_018()                                 PATCH_METHOD(PATCH_FLAG_800, (u16)800)
    #define PATCH_EID_APPCMD_019()                                 PATCH_METHOD(PATCH_FLAG_801, (u16)801)
    #define PATCH_EID_APPCMD_020()                                 PATCH_METHOD(PATCH_FLAG_802, (u16)802)
    #define PATCH_EID_APPCMD_021()                                 PATCH_METHOD(PATCH_FLAG_803, (u16)803)
    #define PATCH_EID_APPCMD_022()                                 PATCH_METHOD(PATCH_FLAG_804, (u16)804)
    #define PATCH_EID_APPCMD_023()                                 PATCH_METHOD(PATCH_FLAG_805, (u16)805)
    #define PATCH_EID_APPCMD_024()                                 PATCH_METHOD(PATCH_FLAG_806, (u16)806)


/***************Patch for JT--------->ECP_JT	********************************************************/     
    #define PATCH_JT_001()                                         PATCH_METHOD(PATCH_FLAG_807, (u16)807)
    #define PATCH_JT_002()                                         PATCH_METHOD(PATCH_FLAG_808, (u16)808)
    #define PATCH_JT_003()                                         PATCH_METHOD(PATCH_FLAG_809, (u16)809)
    #define PATCH_JT_004()                                         PATCH_METHOD(PATCH_FLAG_810, (u16)810)
    #define PATCH_JT_005()                                         PATCH_METHOD(PATCH_FLAG_811, (u16)811)
    #define PATCH_JT_006()                                         PATCH_METHOD(PATCH_FLAG_812, (u16)812)
    #define PATCH_JT_007()                                         PATCH_METHOD(PATCH_FLAG_813, (u16)813)
    #define PATCH_JT_008()                                         PATCH_METHOD(PATCH_FLAG_814, (u16)814)
    #define PATCH_JT_009()                                         PATCH_METHOD(PATCH_FLAG_815, (u16)815)
    #define PATCH_JT_010()                                         PATCH_METHOD(PATCH_FLAG_816, (u16)816)
    #define PATCH_JT_011()                                         PATCH_METHOD(PATCH_FLAG_817, (u16)817)
    #define PATCH_JT_012()                                         PATCH_METHOD(PATCH_FLAG_818, (u16)818)
    #define PATCH_JT_013()                                         PATCH_METHOD(PATCH_FLAG_819, (u16)819)
    #define PATCH_JT_014()                                         PATCH_METHOD(PATCH_FLAG_820, (u16)820)
    #define PATCH_JT_015()                                         PATCH_METHOD(PATCH_FLAG_821, (u16)821)
    #define PATCH_JT_016()                                         PATCH_METHOD(PATCH_FLAG_822, (u16)822)
    #define PATCH_JT_017()                                         PATCH_METHOD(PATCH_FLAG_823, (u16)823)
    #define PATCH_JT_018()                                         PATCH_METHOD(PATCH_FLAG_824, (u16)824)
    #define PATCH_JT_019()                                         PATCH_METHOD(PATCH_FLAG_825, (u16)825)
    #define PATCH_JT_020()                                         PATCH_METHOD(PATCH_FLAG_826, (u16)826)
    #define PATCH_JT_021()                                         PATCH_METHOD(PATCH_FLAG_827, (u16)827)
    #define PATCH_JT_022()                                         PATCH_METHOD(PATCH_FLAG_828, (u16)828)
    #define PATCH_JT_023()                                         PATCH_METHOD(PATCH_FLAG_829, (u16)829)
/***************Patch for Pboc--------->EDEP_Algorithm.c	**********************************************/   	
	#define PATCH_EDEP_ALG_008()								    PATCH_METHOD(PATCH_FLAG_830, (u16)830)
	#define PATCH_EDEP_ALG_009()								    PATCH_METHOD(PATCH_FLAG_831, (u16)831)
	#define PATCH_EDEP_ALG_010()								    PATCH_METHOD(PATCH_FLAG_832, (u16)832)
/***************Patch for HAL--------->Functuin.c			**********************************************/  
	#define PATCH_FUNC_001()										PATCH_METHOD(PATCH_FLAG_833, (u16)833)
	#define PATCH_FUNC_002()										PATCH_METHOD(PATCH_FLAG_834, (u16)834)
	#define PATCH_FUNC_003()										PATCH_METHOD(PATCH_FLAG_835, (u16)835)
	#define PATCH_FUNC_004()										PATCH_METHOD(PATCH_FLAG_836, (u16)836)
	#define PATCH_FUNC_005()										PATCH_METHOD(PATCH_FLAG_837, (u16)837)
	#define PATCH_FUNC_006()										PATCH_METHOD(PATCH_FLAG_838, (u16)838)
	#define PATCH_FUNC_007()										PATCH_METHOD(PATCH_FLAG_839, (u16)839)
	#define PATCH_FUNC_008()										PATCH_METHOD(PATCH_FLAG_840, (u16)840)
/***************Patch for HAL--------->SM3_HAL.c			**********************************************/  
	#define PATCH_SM3_001()										  	PATCH_METHOD(PATCH_FLAG_841, (u16)841)
	#define PATCH_SM3_002()										  	PATCH_METHOD(PATCH_FLAG_842, (u16)842)
	#define PATCH_SM3_003()										  	PATCH_METHOD(PATCH_FLAG_843, (u16)843)
/***************Patch for HAL--------->SM1_HAL.c			**********************************************/ 
	#define PATCH_SM1_001()										  	PATCH_METHOD(PATCH_FLAG_844, (u16)844)
	#define PATCH_SM1_002()										  	PATCH_METHOD(PATCH_FLAG_845, (u16)845)
/***************Patch for HAL--------->SM4_HAL.c			**********************************************/ 
	#define PATCH_SM4_001()										  	PATCH_METHOD(PATCH_FLAG_846, (u16)846)
	#define PATCH_SM4_002()										  	PATCH_METHOD(PATCH_FLAG_847, (u16)847)
	
/***************Patch for PBOC--------->pbocAPI.c			**********************************************/ 	
	#define PATCH_PBOC_API_001()									PATCH_METHOD(PATCH_FLAG_848, (u16)848)
	#define PATCH_PBOC_API_002()									PATCH_METHOD(PATCH_FLAG_849, (u16)849)
	#define PATCH_PBOC_API_003()									PATCH_METHOD(PATCH_FLAG_850, (u16)850)
	#define PATCH_PBOC_API_004()									PATCH_METHOD(PATCH_FLAG_851, (u16)851)
	#define PATCH_PBOC_API_005()									PATCH_METHOD(PATCH_FLAG_852, (u16)852)
	#define PATCH_PBOC_API_006()									PATCH_METHOD(PATCH_FLAG_853, (u16)853)
	#define PATCH_PBOC_API_007()									PATCH_METHOD(PATCH_FLAG_854, (u16)854)
	
/***********Patch for jcvm native appapi----->AppEdepApi.c	**********************************************/ 
	#define PATCH_APPEDEP_API_001() 								PATCH_METHOD(PATCH_FLAG_855, (u16)855)

#endif
