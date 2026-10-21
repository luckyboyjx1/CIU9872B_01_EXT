/*
 * chipPatch.h
 *
 *  Created on: 2015年1月4日
 *      Author: jiangxiaohui
 */

#ifndef CHIPPATCH_H_
#define CHIPPATCH_H_

#define			PATCH_FLAG_START							0x00
#define			PATCH_FLAG_INIT                   			(PATCH_FLAG_START					+ 0x02)			//0x02(0x02)				FFFF
#define			EE_PATCH_TABLE    				  			(PATCH_FLAG_INIT					+ 0x02)			//0x7E(0x04)				ALL FF
#define			EE_PATCH_LENGTH					  			(EE_PATCH_TABLE						+ 0x7E)			//0x1C(0x82)				ALL FF
#define			PATCH_ENTRY_SEL								(EE_PATCH_LENGTH					+ 0x1C)			//0x02(0x9E)				FFFF
#define			EE_PATCH_SERVICE							(PATCH_ENTRY_SEL					+ 0x02)			//0x18(0xA0)				ALL FF(注意如果EE_PATCH_SERVICE的地址改了，WatchExeptonIsR必须对应地修改)
#define	        PATCH_AREA_CONFIG							(EE_PATCH_SERVICE					+ 0x18)			//0x01(0xB8)				0x00
#define			PATCH_DYNAMIC_AREA							(PATCH_AREA_CONFIG					+ 0x01)			//0x0c(0xb9)

//#define			PATCH_AREA_LENGTH							0xC5

#define	ecpCiu_getPatchFlagStartBase(patchAreaPtr)			(patchAreaPtr + PATCH_FLAG_START)
#define	ecpCiu_getPatchFlagInitBase(patchAreaPtr)			(patchAreaPtr + PATCH_FLAG_INIT)
#define	ecpCiu_getPatchEntryTable(patchAreaPtr)				(patchAreaPtr + EE_PATCH_TABLE)
#define	ecpCiu_getPatchLenthTable(patchAreaPtr)				(patchAreaPtr + EE_PATCH_LENGTH)

#endif /* CHIPPATCH_H_ */
