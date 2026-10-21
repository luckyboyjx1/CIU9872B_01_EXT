/*
 * CgmField.c
 *
 *  Created on: 2015年11月5日
 *      Author: liuhanren
 */

#include "CgmManager.h"
#include "cardManager.h"
#include "CgmField.h"
#include "BootLifeCycle.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#include "Interface_LTEFromPlatform.h"

#if (!GP__CGM__FOR_MASK&&GP__CGM__FOR_APPPARACHECK)
/*
* 由CGM flag的值和CGM_Mask求出对应项的偏移，
* CGMarray数组结构参考JAVA层的AppTableEntry
* CGMflag :
* CGM_Mask : c7/c8/d7/d8/82/83 mask
* isMaxCGMorRemainCGM : true maxCGM, flase remainCGM, 只有在8283情况下isMaxCGMorRemainCGM才能为true
* 返回CGM_Mask对应的参数在CGMarray里的偏移
*
*  CGMarray 数组的结构如下(中间有不存在的数据时,后面的数据往前移)：
*
*  reserveRAM： D7的值, 4字节长度, CGMflag的bit0为1时存在;
*  reserveNVM： D8的值, 4字节长度, CGMflag的bit1为1时存在;
*  remainCGMram： 82的剩余可用值, 4字节长度, CGMflag的bit2为1时存在;
*  maxCGMram： 82的最大值(安装参数指定), 4字节长度, CGMflag的bit2为1时存在;
*  remainCGMnvm： 83的剩余可用值, 4字节长度, CGMflag的bit3为1时存在;
*  maxCGMnvm： 83的最大值(安装参数指定), 4字节长度, CGMflag的bit3为1时存在;
*  quotaNVM： C8的值, 4字节长度, CGMflag的bit6为1时存在;
*  quotaRAM： C7的值, 4字节长度, CGMflag的bit7为1时存在;
*
*/
u8 CgmField_ArrayOffset(u8 CGMflag, u8 CGM_Mask, boolean isMaxCGMorRemainCGM)
{
	u8 offset;

	offset = 0;
	switch(CGM_Mask)
	{
		case HASC7_MASK:
			if(U8_AND_U8(CGMflag, HASC8_MASK))
			{
				offset = U8_A_U8(offset,CGM_Mask_PerPara_Len);
			}
			/* no break */
		case HASC8_MASK:
			if(U8_AND_U8(CGMflag, HAS83_MASK))
			{
				offset = U8_A_U8(offset,2*CGM_Mask_PerPara_Len);	//偏移过83的 两个 nvm（maxCGMnvm + remainCGMnvm） 缓存，每个4字节
			}
			/* no break */
		case HAS83_MASK:
			if(isMaxCGMorRemainCGM)
			{
				offset = U8_A_U8(offset,CGM_Mask_PerPara_Len);		//isMaxCGMorRemainCGM 为true时获取的是maxCGMnvm，偏移过remainCGMnvm
				isMaxCGMorRemainCGM = false;
			}
			if(U8_AND_U8(CGMflag, HAS82_MASK))
			{
				offset = U8_A_U8(offset,2*CGM_Mask_PerPara_Len);	//偏移过82的 两个 ram（maxCGMram + remainCGMram） 缓存，每个4字节
			}
			/* no break */
		case HAS82_MASK:
			if(isMaxCGMorRemainCGM)
			{
				offset = U8_A_U8(offset,CGM_Mask_PerPara_Len);		//isMaxCGMorRemainCGM 为true时获取的是maxCGMram，偏移过remainCGMram
				isMaxCGMorRemainCGM = false;
			}
			if(U8_AND_U8(CGMflag, HASD8_MASK))
			{
				offset = U8_A_U8(offset,CGM_Mask_PerPara_Len);
			}
			/* no break */
		case HASD8_MASK:
			if(U8_AND_U8(CGMflag, HASD7_MASK))
			{
				offset = U8_A_U8(offset,CGM_Mask_PerPara_Len);
			}
			/* no break */
		case HASD7_MASK:
			;  //the offset is 0
			break;
	}
	return offset;
}

u32 CgmField_get8283Value(u8 sdId, u8 cgmFlag, PN8 sdEntryAddr, u8 cgm_mask, boolean isMaxCGMorRemainCGM)
{
	u8 				sdId_temp;
	PN8 			cgmArrayAddr;

	while(!U8_AND_U8(cgmFlag, cgm_mask))
	{
		sdId_temp   = getAppAssociatedSdId(sdEntryAddr);
		if(sdId_temp == sdId)  //注意，自关联的，不一定是ISD
		{
			sdId 	= 0;
		}
		else
		{
			sdId 	= sdId_temp;
		}

		sdEntryAddr = MEMtPN8(getSdEntryObj(sdId));
		cgmFlag 	= CgmField_getCGMFlag(sdEntryAddr);
	}
	cgmArrayAddr 	= MEMtPN8(CgmField_getArrayObj(sdEntryAddr));	
	
	//if(U8_AND_U8(cgm_mask, HAS82_MASK))
	//{
	//	return CgmField_getNvmSizeFromCGMarray(cgmArrayAddr, CgmField_ArrayOffset(cgmFlag, cgm_mask, isMaxCGMorRemainCGM));
	//}
	//else
	{
		return CgmField_getNvmSizeFromCGMarray(cgmArrayAddr, CgmField_ArrayOffset(cgmFlag, cgm_mask, isMaxCGMorRemainCGM));
	}
}

//modify by  jiangqingbin -20211223--去除没使用到的参数
u32 CgmField_getSpaceInfo(u8 sdId, u8 type)
{
	u8 					cgmFlag;
	u32 				result;
	PN8 				sdEntryAddr;
	PN8 				cgmArrayAddr;
	u32                 fileSize = 0;

	result 				= 0xFFFFFFFF; //若没有对应项，则返回0xFFFFFFFF 	(兼容原方案)
//	if(g_bCheckFlag == CHECK_CLOSE)
//	{
//		return result;
//	}

	getAppTable();
	sdEntryAddr 		= MEMtPN8(getSdEntryObj(sdId));
	cgmFlag 			= CgmField_getCGMFlag(sdEntryAddr);
	cgmArrayAddr 		= MEMtPN8(CgmField_getArrayObj(sdEntryAddr));

	switch(type)
	{
		case SD_MANAGE_RAM_SIZE:
		case SD_REMAIN_RAM_CHECK:

		result 			= CgmField_get8283Value(sdId, cgmFlag, sdEntryAddr, HAS82_MASK, false);
		if (SD_REMAIN_RAM_CHECK == type)
		{
			if (result < CgmField_CacheC7())  //这是install for load中的C7
			{
				result  = 1;
			}
			else
			{
				result  = 0;
			}
		}
		break;

	case SD_MANAGE_NVM_SIZE:
	case SD_REMAIN_NVM_CHECK:

		result 			= CgmField_get8283Value(sdId, cgmFlag, sdEntryAddr, HAS83_MASK, false);
//revised by weipingjiao@20230112#if SE_ONLY  //SE_ONLY时  ISD的CGM可作为卡片CGM ，真正可用的CGM需要减去文件系统空间 ，为了减少修改文件系统时频繁写E2修改CGM，在这里判断真正的可用空间 ，带EUICC功能时 每个Profile下都有一个ISD ，需要重新考虑CGM方案
			if((sdId == APP_FIRST)&& (JcvmConfig_GetSEState() > CARD_OP_READY))
			{
				fileSize = GetFileSystemTotalSize();
			}
//#endif
		if (SD_REMAIN_NVM_CHECK == type)
		{
			if (U32_S_U32(result,fileSize) < CgmField_CacheC8()) //保存的是install for load中的C6+C8
			{
				result  = 1;
			}
			else
			{
				result  = 0;
			}
		}
		break;

	case SD_CGM_REMAIN_RAM_SIZE:
		if(U8_AND_U8(cgmFlag, HAS82_MASK))
		{
			result 		= CgmField_getRemain82(cgmArrayAddr, cgmFlag);
		}
		break;
	case SD_CGM_REMAIN_NVM_SIZE:
		if(U8_AND_U8(cgmFlag, HAS83_MASK))
		{
			result 		= CgmField_getRemain83(cgmArrayAddr, cgmFlag);
		}
		break;
	case SD_CGM_RAM_SIZE:
		if(U8_AND_U8(cgmFlag, HAS82_MASK))
		{
			result 		= CgmField_getMax82(cgmArrayAddr, cgmFlag);
		}
		break;
	case SD_CGM_NVM_SIZE:
		if(U8_AND_U8(cgmFlag, HAS83_MASK))
		{
			result 		= CgmField_getMax83(cgmArrayAddr, cgmFlag);
		}
		break;

	case SD_CGM_RAM_SIZE_90:
		    result 		= CgmField_get8283Value(sdId, cgmFlag, sdEntryAddr, HAS82_MASK, true);
		break;

	case SD_CGM_NVM_SIZE_8F:
			result 		= CgmField_get8283Value(sdId, cgmFlag, sdEntryAddr, HAS83_MASK, true);
		break;
	}
	return result;
}

void CgmField_getEntryInfo(u16 context)
{
	u8 					  cgmFlag;
	u8					  lastVmPagId;
	u16					  appIndex;
	PN8					  cgmArrayAddr;
	jref				  objectId;
	lastVmPagId			  = 0;
	appIndex			  = getIndexByID(U16t8(U16_AND_U16(context, APPLET_MASK)));
	if(0xFFFF == appIndex)
	{
		return;
	}

	objectId			  = nvmReadU16(MEMtPN8(U32_A_U16(APP_Table_Cash, U16_M_U16(appIndex, SIZE_REF))));

	APP_Entry_Object_Cash = locateObjectInMemory(objectId);

	if(MEM_NULL != APP_Entry_Object_Cash)
	{
		cgmArrayAddr 	  = MEMtPN8(CgmField_getArrayObj(MEMtPF8(APP_Entry_Object_Cash)));
		cgmFlag 		  = CgmField_getCGMFlag(MEMtPF8(APP_Entry_Object_Cash));

		if(U8_AND_U8(cgmFlag, HASC8_MASK))
		{
			g_sC8Value 	  = CgmField_getC8(cgmArrayAddr, cgmFlag);
		}
		else
		{
			g_sC8Value 	  = 0xFFFFFFFF;
		}

		if(U8_AND_U8(cgmFlag, HASC7_MASK))
		{
			g_sC7Value    = CgmField_getC7(cgmArrayAddr, cgmFlag);
		}
		else
		{
			g_sC7Value 	  = 0xFFFFFFFF;
		}

		if(U8_AND_U8(cgmFlag, HASD7_MASK))
		{
			g_sD7Value    = CgmField_getD7(cgmArrayAddr, cgmFlag);
		}
		else
		{
			g_sD7Value    = 0;
		}

		if(U8_AND_U8(cgmFlag, HASD8_MASK))
		{
			#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
			if(g_sC8Value == 0xFFFFFFFF)
			{
				g_sD8Value = 0;
			}
			else
			{
				g_sD8Value = g_sC8Value;
			}
			#else
			g_sD8Value     = CgmField_getD8(cgmArrayAddr, cgmFlag);
			#endif
		}
		else
		{
			g_sD8Value 	   = 0;
		}

		CgmInit_keepVarValue(&lastVmPagId, lastVMPkg);
		CgmInit_consumeVar();
		CgmManager_calaConsumeSpace(context, MEMtPX32(&g_UsedNVM), MEMtPX32(&g_UsedRAM), false, CGM_GET_ALL);
		CgmInit_keepVarValue(&lastVMPkg, lastVmPagId);
		g_OldUsedRAM		   =	g_UsedRAM;
		g_OldUsedNVM	       = 	g_UsedNVM;
	}
}

memref CgmField_getArrayObj(PF8 entryAddr)
{
	u16 			objRef;
	memref 			cgmArrayAddr;

	cgmArrayAddr	= MEM_NULL;
	objRef = memReadU16(get_object_field_ref(entryAddr, CGMarray_INDEX));
	if (objRef != MEM_NULL)
	{
		cgmArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));
	}

	return cgmArrayAddr;
}
/**
 * modify by xuxiaojun@20220211 根据现在的方案，安装参数中C7C8为全0时，需要将其修改为全FF；D7D8为会全FF时，需要将其修改为全0.
 */
u32 CgmField_GetValue(PR8 buffer, u8 offset, u8 tagLen, u8 cgm_mask, u8 isInstallForLoad)
{
	u32 			cgmValue;

	cgmValue 		= 0;

	if(OS_C7C8_NOCHECK) //OS_C7C8_NOCHECK为1表示不做C7C8的检查
	{
		if((HASC7_MASK == cgm_mask)||(HASC8_MASK == cgm_mask))
		{
			if(TRUE == isInstallForLoad)
			{
				cgmValue = 0;
			}
			else
			{
				cgmValue = 0xFFFFFFFF;
			}
			return cgmValue;
		}
	}

	if(tagLen == TAG_LEN_2)
	{
		cgmValue = PX_R_U16(&buffer[offset]);
		#if	((UICC__OPERATOR__CHINAMOBILE || CHINA_MOBILE_DESIGNATED_REQUIREMENT) || UICC__OPERATOR__CHINATELECOM || SE_AND_EUICC) //移动5g  电信5G   uicc_configuration_additional_test/SECURED/GP补充/INSTALL_D7D8_length_Test_01   EF08 D7020200 D8028000期望成功
		//nothing
		#else
 		if((0 != U32_AND_U32(cgmValue, 0x00008000)) && (0xFFFF != U32t16(cgmValue)))
 		{
 			CgmManager_error(SW_WRONG_DATA);
 		}
		#endif

		#if	UICC__OPERATOR__CHINAMOBILE
 		if(0xFFFF == U32t16(cgmValue)/* || 0 == U32t16(cgmValue)*/) //NFC 2.0 DTR_RTR_RAM测试_工程集/1_12, 对于0000不做特殊处理
		#else
 		if(((0xFFFF == U32t16(cgmValue)) || (0 == U32t16(cgmValue)))&&((cgm_mask != HAS82_MASK)&&(cgm_mask != HAS83_MASK)))//c7 c8 = 0 当成FFFF处理 ,EUICC RSP 相关测试  、 中国电信5G SWP备案测试    CTCC_Test/NFC功能/边界测试-接触
		#endif																												//82  83 cgm tag不做特殊处理，不能把0当成FFFF 否则会当成需要为SD分配FFFF大小的CGM值
		{
			/* For each tag(C6C7C8 of install for load and C7C8 of install for install) that
			 * contains a value of 'FFFF', the implementation is not check for the available space.
			 * (2[1].1.1_Mapping_guidelines_v1.0.1-Final.pdf)*/
			if(TRUE == isInstallForLoad)
			{
				cgmValue = 0;
			}
			else
			{
				if((HASC7_MASK == cgm_mask)||(HASC8_MASK == cgm_mask))
				{
					cgmValue = 0xFFFFFFFF;
				}

				if((HASD7_MASK == cgm_mask)||(HASD8_MASK == cgm_mask))
				{
					cgmValue = 0;
				}

			}
		}

	}
	else if(tagLen == TAG_LEN_4)
	{
		cgmValue = PX_R_U32(&buffer[offset]);

		#if	((UICC__OPERATOR__CHINAMOBILE || CHINA_MOBILE_DESIGNATED_REQUIREMENT) || UICC__OPERATOR__CHINATELECOM || SE_AND_EUICC)
		//nothing
		#else
/*		if((cgmValue <= 0x00007FFF)
			   ||U8_AND_U8(cgm_mask, CGM_2BYTE_MASK))*/
#if (!GPC_2_3_1)			   
		//revised by dengyiwen INSTALL_D7D8_length_Test_01(错误个数: 24),D7 D8 support four bytes length from GP2.3.1
		if(U8_AND_U8(cgm_mask, CGM_2BYTE_MASK))
		{
			CgmManager_error(SW_WRONG_DATA);
		}
#endif
		#endif

		#if	UICC__OPERATOR__CHINAMOBILE
 		if(0xFFFFFFFF == U32t16(cgmValue)/* || 0 == U32t16(cgmValue)*/) //NFC 2.0 DTR_RTR_RAM测试_工程集/1_12, 对于0000不做特殊处理
		#else
 		if(((0xFFFFFFFF == U32t16(cgmValue)) || (0 == U32t16(cgmValue)))&&((cgm_mask != HAS82_MASK)&&(cgm_mask != HAS83_MASK)))//c7 c8 = 0 当成FFFF处理 ,EUICC RSP 相关测试  、 中国电信5G SWP备案测试    CTCC_Test/NFC功能/边界测试-接触
		#endif																												//82  83 cgm tag不做特殊处理，不能把0当成FFFF 否则会当成需要为SD分配FFFF大小的CGM值
		{
			/* For each tag(C6C7C8 of install for load and C7C8 of install for install) that
			 * contains a value of 'FFFF', the implementation is not check for the available space.
			 * (2[1].1.1_Mapping_guidelines_v1.0.1-Final.pdf)*/
			if(TRUE == isInstallForLoad)
			{
				cgmValue = 0;
			}
			else
			{
				if((HASC7_MASK == cgm_mask)||(HASC8_MASK == cgm_mask))
				{
					cgmValue = 0xFFFFFFFF;
				}

				if((HASD7_MASK == cgm_mask)||(HASD8_MASK == cgm_mask))
				{
					cgmValue = 0;
				}

			}
		}
	}
	else
	{
		CgmManager_error(SW_WRONG_DATA);
	}
	return cgmValue;
}

/*
*	现阶段8283一般是同时存在或不存在，但为了以后可扩展性，这里任然把8283
*	分开处理。
* 返回值： true -- 正常结束
*          false -- 抛异常了
*/
void CgmField_set8283Value(PN8 cgmArrayAddr,u8 cgmFlag, u32 cgmRam, u32 cgmNvm, u8 set_mask)
{
	switch(set_mask)
	{
		case INIT_CGM_WHEN_INSTALL_SD://init the maxCGM and remainCGM value
			if(U8_AND_U8(cgmFlag, HAS82_MASK))
			{
				CgmField_setMax82(cgmArrayAddr,cgmFlag,cgmRam);
				CgmField_setRemain82(cgmArrayAddr,cgmFlag,cgmRam);
			}
			if(U8_AND_U8(cgmFlag, HAS83_MASK))
			{
				CgmField_setMax83(cgmArrayAddr,cgmFlag,cgmNvm);
				CgmField_setRemain83(cgmArrayAddr,cgmFlag,cgmNvm);
			}
			break;

		case UPDATA_SD_CGM_SIZE:
			if((U8_AND_U8(cgmFlag, HAS82_MASK)) == HAS82_MASK)
			{
				CgmField_setMax82(cgmArrayAddr,cgmFlag,cgmRam);
			}
			if((U8_AND_U8(cgmFlag, HAS83_MASK)) == HAS83_MASK)
			{
				CgmField_setMax83(cgmArrayAddr,cgmFlag,cgmNvm);
			}
			break;

		case UPDATA_SD_REMAIN_CGM_SIZE:
			if((U8_AND_U8(cgmFlag, HAS82_MASK)) == HAS82_MASK)
			{
				CgmField_setRemain82(cgmArrayAddr,cgmFlag,cgmRam);
			}
			if((U8_AND_U8(cgmFlag, HAS83_MASK)) == HAS83_MASK)
			{
				CgmField_setRemain83(cgmArrayAddr,cgmFlag,cgmNvm);
			}
			break;
		case INIT_CGM_WHEN_INSTALL_MNOSD://init the maxCGM and remainCGM value
			CgmInit_ValueForISD();
			break;
	}
	return;
}

u32	 CgmField_AlignSize(u32 origSize)
{
	u32				alignSize;
	u32				temp;

	temp			= U32_R_U8(origSize, 4);
	if(temp != 0)
	{
		alignSize 	= U8_A_U32(U8_S_U8(4, temp), origSize);
	}
	else
	{
		alignSize	= origSize;
	}

	return  alignSize;
}

#endif
