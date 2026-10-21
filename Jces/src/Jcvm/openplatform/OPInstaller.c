#include "CgmManager.h"
#include "loader.h"
#include "contentManager.h"
#include "packageloc.h"
#include "bc_array.h"
#include "bc_object.h"
#include "object.h"
#include "ob_manager.h"
#include "nativemethod_impl.h"
#include "bc_object.h"
#include "mask.h"
#include "memory.h"
#include "utilMethod.h"
#include "grt.h"
#include "except.h"
#include "global_variable.h"
#include "cardmanager.h"
#include "OPInstaller.h"
#include "opFunction.h"
#include "JcvmConfig.h"
#include "KeyLogicManager.h"
#include "KeyManagerOld.h"
#include "SCP11.h"
#include "SEEUICCInterface.h"
#include "SCPInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

/*****************************************************************************
 * File:  opInstaller.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2009, 8 ,14	.created
 Original author: wang hui
   从EPX3.0同步过来，modify by lixin@@20210816 如果卡内正在执行cap包的下载时，又通过非RAM方式来了一条install for load指令，该条指令期望抛异常.<联通测试的log><2-1-manage channel测试><F2_TC6_ISDAMSSDTest>
 *****************************************************************************/

u16 opInstallForLoad(void) 
{
	u8 					bAidLen_;
	u8					bSysParmTag_;
	u8 					tempLen;
	u16 				offset;
#if GP__CGM__FOR_APPPARACHECK	
	u32 				tempSize;
#endif

	offset 				= OFFSET_CDATA;
	bAidLen_ 			= APDUBuffer[offset];

	U16_INC(offset);

	/*if (JcvmConfig_GetRegistryUpdate() == REGISTRY_UPDATA_DISABLE)
	{
		return SW_INS_NOT_SUPPORTED;
	}*/
	// modify by chentao 20230519 首次修改后ST 和英飞凌平台测试没问题，但是在SCS307测试发现PN_R_U8不带cash读有问题，因为写的时候是在cash里，所以第二次修改将PN_R_U8该为带 cash的读

	if(memReadU8(MEMtPM8(PM_g_pkgIdInProcess_JC)) != ILLEGAL_ID) //bug 484 add by xuxiaojun@20230420 在进行installforlaod时，不允许开启一个load过程（COS内部方案 无规范来源）
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}


	// check aid length accoding to iso7816 
	if (AIDLenInvalid(bAidLen_) || (U8_S_U8(APDUBuffer[OFFSET_LC], bAidLen_) < 5))
	{
		return SW_WRONG_DATA;
	}

	// this used in the heard component process
	bRamBuffer[LOAD_PKG_LEN_OFF] = bAidLen_;
	ecpMem_Copy(&bRamBuffer[LOAD_PKG_AID_OFF], &APDUBuffer[offset], bAidLen_);
	

	// check the pakage is exit.
	if (findPkgID(MEMtPF8(&bRamBuffer[0]), LOAD_PKG_AID_OFF, bAidLen_, 0) != ILLEGAL_ID)
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	offset 	 = U32t16(U16_A_U8(offset, bAidLen_)); // offset to Security Domain AID length indicator
	bAidLen_ = APDUBuffer[offset];
	U16_INC(offset);
	if (bAidLen_ != 0)
	{
		// check aid length accoding to iso7816
		if (AIDLenInvalid(bAidLen_))
		{
			return SW_WRONG_DATA;
		}
		offset 	 = U32t16(U16_A_U8(offset, bAidLen_)); // skip to Security Domain AID length indicator
	}


	// offset to HASH length
	bAidLen_ = APDUBuffer[offset];
	U16_INC(offset);
	//该全局变量没有使用到
//	g_needCheckHash = 0;  
	if (bAidLen_ == U32t8(0x14))
	{
		ecpMem_Copy(&bRamBuffer[HASHBUFER_OFF], &APDUBuffer[offset], bAidLen_);
		offset 	 = U32t16(U16_A_U8(offset, bAidLen_)); // skip to hash block
//		g_needCheckHash = 1;
	}
	else if(bAidLen_!=0)
	{
		return SW_WRONG_DATA;
	}

#if GP__CGM__FOR_APPPARACHECK	
	CgmField_CacheC7() = 0;
	CgmField_CacheC8() = 0;
#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
	p_PkgInfo.loadC6 = 0;
	p_PkgInfo.loadC7 = 0;
	p_PkgInfo.loadC8 = 0;
#endif
#endif

	if (APDUBuffer[U16_INC(offset)] != 0)
	{ // offset to load parameters length indicator
		// check the load parameters tag
		if (APDUBuffer[U16_INC(offset)] == 0xEF)
		{
			//return SW_WRONG_DATA;
			// check the load parameters length
			bAidLen_ = APDUBuffer[offset];
			U16_INC(offset);
			while (bAidLen_ != 0)
			{
			bSysParmTag_ = APDUBuffer[U16_INC(offset)];
			tempLen =  APDUBuffer[U16_INC(offset)];
			
			switch (bSysParmTag_)
			{
			case 0xC6: // Non volatile code space limit,没有判断是否超出limit		
#if GP__CGM__FOR_APPPARACHECK				
				tempSize = CgmField_GetValue(APDUBuffer, offset , tempLen, HASC8_MASK, TRUE);	//C6当C8处理
				CgmField_CacheC8() = U32_A_U32(CgmField_CacheC8(), tempSize);
#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
				p_PkgInfo.loadC6 = tempSize;
#endif		
			
#endif
				break;
				
			case 0xC7: // Volatile data space limit 
#if GP__CGM__FOR_APPPARACHECK				
				CgmField_CacheC7() = CgmField_GetValue(APDUBuffer, offset , tempLen, HASC7_MASK, TRUE);

#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
				p_PkgInfo.loadC7 = CgmField_CacheC7();
#endif
#endif
				break;
			
			case 0xC8: // Non volatile data space limit 
#if GP__CGM__FOR_APPPARACHECK				
				tempSize = CgmField_GetValue(APDUBuffer, offset , tempLen, HASC8_MASK, TRUE);
				CgmField_CacheC8() = U32_A_U32(CgmField_CacheC8(), tempSize);
#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
				p_PkgInfo.loadC8 = tempSize;
#endif
#endif
				break;
			
			default:
				return SW_WRONG_DATA;
			}			
			
			offset   = U32t16(U16_A_U8(offset, tempLen));
			bAidLen_ = U8_S_U8(bAidLen_, U8_A_U8(tempLen, 2));
           }
		}
	}

	// set loader status
	bRamBuffer[INSTALLER_STATE_INDEX] = INSTALLER_STATE_READY;
	resetLinker();
	g_LoaderState = WAIT_4_TAG;
	bRamBuffer[INSTALL_FOR_LOAD] = U8_OR_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_SHL_U8(1, selected_channel));
	PXB_W_U16(&bShortBuffer[INSTALLER_EXCEPTION], NO_EXCEPTION);
	return SW_NO_ERROR;
}


u16 opInstallForInstall(void)
{
	u8 			tmp;	// Pkg AID length
	u16 		bOff;
	u8 			privLength;
	u8 			privBuffer[PRIVMAXBYTES];	
	u8          tagLenLen;
#if !GP__VERSION__FOR_MINI_MASK	
	u8 			C9Len;
	u8 			haveC9;
	u8 			EFLen;
	u8 			praLen;
#if GP__SCP__SCP11__FOR_MASK
	u8			BF20CGMBuf[0x05];
#endif
	u8 			sysTag;
	u8 			tempLen;
	u8 			hasC7;
	u8 			hasC8;
	u8 			hasD7;
	u8 			hasD8;
	u8 			has82;
	u8 			has83;
	
	//u8 installParaLength;
	u16 installParaLimit;

	hasC7 			= 0;
	hasC8 			= 0;
	hasD7 			= 0;
	hasD8 			= 0;
	has82 			= 0;
	has83 			= 0;

#endif
	bOff 			= OFFSET_CDATA;
	tmp 			= APDUBuffer[OFFSET_CDATA];

	if (AIDLenInvalid(tmp))
	{
		return SW_WRONG_DATA;
	}
	
	bOff = U32t16(U16_A_U16(bOff, U8_A_U8(tmp, 1)));	// Offset to Class AID length
	tmp = APDUBuffer[bOff];	// Class AID length
	
	if (AIDLenInvalid(tmp))
	{
		return SW_WRONG_DATA;
	}
	
	bOff = U32t16(U16_A_U16(bOff, U8_A_U8(tmp, 1)));		// Offset to Instance AID length
	tmp = APDUBuffer[bOff];	// Instance AID length
	
	if (AIDLenInvalid(tmp))
	{
		return SW_WRONG_DATA;
	}
	
	bOff = U32t16(U16_A_U16(bOff, U8_A_U8(tmp, 1)));	// Offset to Application privileges length
	tmp = APDUBuffer[bOff];	// Privileges Length
	if ((tmp != 1) && (tmp != 3))
	{
		return SW_WRONG_DATA;
	}
	privLength = tmp;
	bOff 	   = U32t16(U16_A_U8(bOff, 1));
	tmp 	   = APDUBuffer[bOff];	// get Privileges
	ecpMem_Set(privBuffer, 0, PRIVMAXBYTES);
	ecpMem_Copy(privBuffer, MEMtPM8(&APDUBuffer[bOff]), privLength);

	if ((U8_AND_U8(privBuffer[0], 0xC1) == 0x01)		// only Mandated DAP Verification
		|| (U8_AND_U8(privBuffer[0], 0xC0) == 0x40)	// only DAP Verification
		|| (U8_AND_U8(privBuffer[0], 0xB0) == 0x20)	// only Delegated Management
		|| ((U8_AND_U8(privBuffer[0], 0x20) == 0x20) && (U8_AND_U8(privBuffer[1], 0x40) == 0x40))	//Authorized Management and Delegated Management privileges are mutually exclusive;
		|| ((U8_AND_U8(privBuffer[0], 0x20) == 0x20) && (U8_AND_U8(privBuffer[1], 0x20) == 0x20))	//Token Verification and Delegated Management privileges are mutually exclusive;
		|| ((U8_AND_U8(privBuffer[0], 0x20) == 0x20) && (U8_AND_U8(privBuffer[2], 0x80) == 0x80)))  //Receipt Generation and Delegated Management privileges are mutually exclusive;
	{
		return SW_WRONG_DATA;
	}
#if !GP__VERSION__FOR_MINI_MASK
#if GP__CGM__FOR_APPPARACHECK	
	CgmField_CacheD7() = 0;
	CgmField_CacheD8() = 0;
	CgmField_CacheC7() = 0;
	CgmField_CacheC8() = 0;
#endif
	bRamBuffer[SET_INSTALL_PARA] = 0;
	
	bOff = U16_A_U8(bOff, privLength); // offset to install parameter length

	 praLen = APDUBuffer[bOff];
	 if(praLen == 0x81)
	 {
		U16_INC(bOff);
	    praLen = APDUBuffer[bOff]; 
	 }
    installParaLimit = U16_A_U8(bOff, praLen);
    U16_INC(bOff);
	haveC9 = false;
	while (bOff < installParaLimit)
	{
		switch (APDUBuffer[U16_INC(bOff)])
		{
			case 0xC9:
				C9Len = APDUBuffer[U16_INC(bOff)];
				if(C9Len >  0x7F)
				{
					C9Len = APDUBuffer[U16_INC(bOff)];
				}
				haveC9 = true;

	#if		(GP__CGM__FOR_APPPARACHECK && CHINA_MOBILE_DESIGNATED_REQUIREMENT)

				if((privBuffer[0] & 0x80) == 0x80) //SD
				{
					tempLen = 0;
					while(C9Len != 0)
					{
						sysTag = APDUBuffer[bOff++];
						tempLen = APDUBuffer[bOff++];
						switch (sysTag)
						{
							case (u8) 0X49:
								if(tempLen != (u8)0x06)
								{
									return SW_WRONG_DATA;
								}

								CgmField_Cache82() = CgmField_GetValue(APDUBuffer, bOff , 2, HAS82_MASK, FALSE);
								has82 = 1;
								CgmField_Cache83() = CgmField_GetValue(APDUBuffer, U16_A_U8(bOff, 2) , 4, HAS83_MASK, FALSE);
								has83 = 1;

								/*82,83在这置上，JAVA层就不需要修改*/
								bRamBuffer[SET_INSTALL_PARA] |= HAS82_MASK | HAS83_MASK;
							
								break;
							
							default:
								break;
						}
						bOff 	= U16_A_U8(bOff, tempLen);
						C9Len   =U8_S_U8(C9Len, U8_A_U8(tempLen, 2));
						if((s8)0 > (s8)C9Len)
						{
							return SW_WRONG_DATA;
						}
					}
				}
	#endif

				bOff = U16_A_U8(bOff, C9Len);
				break;
			case 0xEF:
				EFLen = APDUBuffer[U16_INC(bOff)];
				if(EFLen == 0x81)
					EFLen = APDUBuffer[U16_INC(bOff)]; //长度超过7F的情况

				tempLen = 0;
				while (EFLen != 0)
				{
					sysTag = APDUBuffer[U16_INC(bOff)];
					tempLen = APDUBuffer[U16_INC(bOff)];
					tagLenLen = 2;
					if(tempLen == 0x81)
					{
						tempLen = APDUBuffer[U16_INC(bOff)];
						U8_INC(tagLenLen);
					}
				
					switch (sysTag)
					{
	#if GP__CGM__FOR_APPPARACHECK
					case 0xC7: // Volatile data space limit
						CgmField_CacheC7() = CgmField_GetValue(APDUBuffer, bOff , tempLen, HASC7_MASK, FALSE);
						hasC7 = 1;

	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
						if(OS_C7C8_NOCHECK)
						{
							CgmField_CacheD7() =  0;
						}
						else
						{
							CgmField_CacheD7() = CgmField_CacheC7();
						}
						hasD7 = 1;
	#endif
						break;
						
					case 0xC8: // Non volatile data space limit
						CgmField_CacheC8() = CgmField_GetValue(APDUBuffer, bOff , tempLen, HASC8_MASK, FALSE);
						hasC8 = 1;

	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
						if(OS_C7C8_NOCHECK)
						{
							CgmField_CacheD8() =  0;
						}
						else
						{
							CgmField_CacheD8() = CgmField_CacheC8();
						}
						hasD8 = 1;
	#endif
						break;

	#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
					//nothing
	#else
					case 0xD7: //Volatile Reserved Memory
						CgmField_CacheD7() = CgmField_GetValue(APDUBuffer, bOff , tempLen, HASD7_MASK, FALSE);
						hasD7 = 1;
						break;

					case 0xD8:
						CgmField_CacheD8() = CgmField_GetValue(APDUBuffer, bOff , tempLen, HASD8_MASK, FALSE);
						hasD8 = 1;
						break;

					case 0x82: //Cumulative Granted Volatile Memory
						CgmField_Cache82() = CgmField_GetValue(APDUBuffer, bOff , tempLen, HAS82_MASK, FALSE);

#if GP__SCP__SCP11__C_AUTHORIZATIONS

						if((U8_AND_U8(channelInfo.security_level[selected_channel], SCF_AUTHENTICATED_ANY) == SCF_AUTHENTICATED_ANY )
											&& (PX_R_U8(g_current_SCP11SessionPara_sessionType) == SCP11_C))
						{
							ecpMem_Set(BF20CGMBuf, 0, sizeof(BF20CGMBuf));
							if(BF20_GetField(BF20_MAX_VOLATILE_CGM_TAG, BF20CGMBuf, sizeof(BF20CGMBuf), 1) == true)
							{
								if(CgmField_Cache82() > PM_R_U32(&BF20CGMBuf[1]))
								{
									//If the script attempts to perform an operation that is not allowed, the command triggering this operation shall
									//be rejected with a status word of '6985', the SCP11c session shall be terminated, and all objects with a
									//“pending completion” mark shall be deleted

									closeSecuritychannel(SC_TERMINATED);

									// 需要回滚事务
									if(isInProgress())
									{
										abortTransaction();
									}
									return SW_CONDITIONS_NOT_SATISFIED;
								}
							}
						}
#endif

						has82 = 1;
						break;

					case 0x83:	//Cumulative Granted Non Volatile Memory
						CgmField_Cache83() = CgmField_GetValue(APDUBuffer, bOff , tempLen, HAS83_MASK, FALSE);

#if GP__SCP__SCP11__C_AUTHORIZATIONS						
						if((U8_AND_U8(channelInfo.security_level[selected_channel], SCF_AUTHENTICATED_ANY) == SCF_AUTHENTICATED_ANY )
											&& (PX_R_U8(g_current_SCP11SessionPara_sessionType) == SCP11_C))
						{
							ecpMem_Set(BF20CGMBuf, 0, sizeof(BF20CGMBuf));
							if(BF20_GetField(BF20_MAX_NON_VOLATILE_CGM_TAG, BF20CGMBuf, sizeof(BF20CGMBuf), 1) == true)
							{
								if(CgmField_Cache83() > PM_R_U32(&BF20CGMBuf[1]))
								{
									//If the script attempts to perform an operation that is not allowed, the command triggering this operation shall
									//be rejected with a status word of '6985', the SCP11c session shall be terminated, and all objects with a
									//“pending completion” mark shall be deleted
									
									closeSecuritychannel(SC_TERMINATED);

									// 需要回滚事务
									if(isInProgress())
									{
										abortTransaction();
									}
									return SW_CONDITIONS_NOT_SATISFIED;
								}
							}
						}
#endif

						has83 = 1;
						break;
	#endif

	#endif //GP__CGM__FOR_MASK
					default:
						break;
					}

					bOff = U16_A_U8(bOff, tempLen);
					
					//check EFLen valid
					if (U8_A_U8(tempLen, tagLenLen) > EFLen)
					{
						return SW_WRONG_DATA;
					}
					
					EFLen = U8_S_U8(EFLen, U8_A_U8(tempLen, tagLenLen));
//	/*@ -discast @*/if((s8)0 > (s8)EFLen)/*@ +discast @*/
//					{
//						return SW_WRONG_DATA;
//					}
				}
				
	#if GP__CGM__FOR_APPPARACHECK
				
				#if CHINA_MOBILE_DESIGNATED_REQUIREMENT
					//nothing
				#else
				if(has82 && hasD7)
				{
					if(CgmField_CacheD7() > CgmField_Cache82())
					{
						return SW_WRONG_DATA;
					}

				}
				if(hasC7 && hasD7)
				{
					if(CgmField_CacheD7() > CgmField_CacheC7())
					{
						return SW_WRONG_DATA;
					}
				}
				
				if(has83 && hasD8)
				{
					if((u32)CgmField_CacheD8() > CgmField_Cache83())
					{
						return SW_WRONG_DATA;
					}

				}
				if(hasC8 && hasD8)
				{
					if((u32)CgmField_CacheD8() > CgmField_CacheC8())
					{
						return SW_WRONG_DATA;
					}
				}
				#endif //CHINA_MOBILE_DESIGNATED_REQUIREMENT
				
	#endif
				break;
			default: //EA tag 有81XX的情况
				//return SW_WRONG_DATA;
				tmp = APDUBuffer[U16_INC(bOff)];

				if(tmp == 0x81)
				{
					tmp = APDUBuffer[U16_INC(bOff)];
				}

				bOff = U16_A_U16(bOff, tmp);
		}
	}
	
	if(bOff > U16_A_U8(installParaLimit, 1))
	{
		return SW_WRONG_DATA;
	}
	
	if (haveC9 == false)
	{
		return SW_WRONG_DATA;
	}
	
 	if((bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA ) && (hasC7 != hasC8)) //0348标准测试:toolit应用C7C8为必选,加此判断应付ana_ins_inms用例
 		return SW_WRONG_DATA;
#endif

	return SW_NO_ERROR;
}



#define MANDATED_DAP_VERIFICATION 0x01

u8 checkMandatedDAP(void)
{
	u8	 		MDAPnum;

#if GP__DAP__FOR_MASK
	u16			j;
	u16 		bFreeEntryIndex;
	memref	  	theAppTable;
	memref		sdIdBuff;
	u16 		bPrivRef;
	memref		entry;
	memref 		bPrivArray;
	u16 		bAppID;
#endif

	MDAPnum         = 0;

#if GP__DAP__FOR_MASK
	/*@ -discast @*/
	/*@ -disspecopt@*/
	bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable 	= getAppTable();
	/*@ +disspecopt@*/
	/*@ +discast @*/
	for (j = APP_FIRST; j < bFreeEntryIndex; U16_INC(j))
	{

		entry 		= locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(j, SIZE_REF))));
		bAppID 		= U8t16(memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX))));

		if(bAppID == APP_NULL)
		{
			continue;
		}

		bPrivRef	= memReadU16(MEMtPN8(get_object_field_ref(MEMtPN8(entry), bPriv_INDEX)));
		bPrivArray 	= get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));

		//if (bAppID == theSDMap->bSDId)
		if ( (U8_AND_U8(PX_R_U8(bPrivArray), MANDATED_DAP_VERIFICATION) != MANDATED_DAP_VERIFICATION) &&
			 (bAppID != U8t16(bRamBuffer[MANDATE_SD_ID])) )
		{
			continue;
		}

		if (memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bCurrentState_INDEX))) == APPLET_PERSONALIZED)
		{
			U8_INC(MDAPnum);
		}
	}

	sdIdBuff = get_array_data(MEMtPF8(locateObjectInMemory(PN_R_U16(SDIdBuff4DAPVerification_JC))));
	memFillAtomic(MEMtPN8(sdIdBuff), 0x00, MAX_SD_NUM);
#endif
	return MDAPnum;
}

