#include "CgmManager.h"
#include "opFunction.h"
#include "object.h"
#include "ob_manager.h"
#include "global_variable.h"
#include "mask.h"
#include "contentManager.h"
#include "cardManager.h"
#include "SCPInterface.h"
#include "keyManagerOld.h"
#include "keyLogicManager.h"
#include "utilMethod.h"
#include "OPInstaller.h"
#include "MemBackUp.h"
#include "chn_manager.h"
#include "SysConfig.h"
#include "Crypto_API.h"
#include "MemOperation.h"
#include "Apdu.h"
#include "JcvmConfig.h"
#include "JcvmDataConfig.h"
#include "bc_object.h"
#include "SecureCrypto.h"
#include "CVM.h"
#include "OPCMCal.h"
#include "dataManager.h"
#include "SecurityDomainData.h"
#include "keySet.h"
#include "keyGroup.h"
#include "BootOS.h"
#include "SCPAPINativeMethod.h"
#include "nativemethod_impl.h"
#include "dataManager.h"
#include "Mifare.h"
#include "SCPInterface.h"
#include "Interface_LTEFromPlatform.h"
#include "SEEUICCInterface.h"
#if GP__AMENDMENT__H__FOR_MASK
#include "OPELFUpgrade.h" // warning
#endif
#if GP__SCP__SCP11__FOR_MASK
#include "SCP11.h"
#endif
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 
#if !BOOTLOADER_PRO
#include "BootOSImpl.h"		//remove warning,dengyiwen,2022/9/27
#endif

/*****************************************************************************
 * File:  opFunction.c
 * Version 1.13

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 Original author: wang hui
 *****************************************************************************/
/*@ -namechecks @*/

#define FIRST_LOAD_PACKAGE     NPACKAGES
#define MORE_COMMAND_EXIST	   0
//modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11 将此处的宏定义移动到opFunction.h中
#if BOOT__IO__MIFARE
u16 closeMifare(u8 *apdu);
u16 openMifare(u8 *apdu);
#endif

/*
 *store the command in buffer until last command received
 *return: status of store is or not success
*/
u16 StoreReceivedCommandAPDU(void)
{
	//TODO: 先将收到的数据存储起来，涉及到java代码使用apdubuffer的情况需要将apdubuffer替换为该存储地址
	return MORE_COMMAND_EXIST;
}

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
/*
 * add by jxh 20130902
 * update auth try count
 * */
void managerSDLifeCycle()
{	
	PN8 			sdDataObj;
	PN8 			keySetObj;
	memref   		theAppTable;
	memref   		entry;
	jref			keySetRef;
	u16		D_A		j;
	u16    	D_A     bFreeEntryIndex ;
	u8 		D_A		bAppID;
	u8 		D_A		bTryAuth;
	u8    	D_A		bMaxAuth;
	sdDataObj = getCurrentSdObj(IS_NEED_CHECK_SD_LOCKED);
	bMaxAuth = sdData_getAuthFailCount(sdDataObj);
	keySetRef	=	keySet_getKeySet(sdDataObj, currKeySetVer[selected_channel]);
	keySetObj	=	(PN8)locateObjectInMemory(keySetRef);
	bTryAuth = keySet_getTryAuthAttenpt(keySetObj);
	
	if(bMaxAuth != 0xFF)
	{
		if(bTryAuth == bMaxAuth)
		{
			if(g_bSDId[selected_channel] == 0)
			{
				nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetCardStateBase()), CARD_TERMINATED);
			}
			else
			{
				bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
				theAppTable = getAppTable();
				for (j = APP_FIRST; j < bFreeEntryIndex; j++)
				{
					entry = locateObjectInMemory(PN_R_U16((memref)(theAppTable + j * SIZE_REF)));
					bAppID = PX_R_U8((u8 *)get_object_field_ref(entry, bAppID_INDEX));
					if(bAppID == g_bSDId[selected_channel])
					{
						break;
					}
				}
				store_object_B_filed(entry, bCurrentState_INDEX, SSD_LOCKED);
			}
		}
	}
}
#endif

#if GP__SCP
memref getCurentAidAddrss(PX8 aidLength)
{
	memref bCurentAidAdd;
	u8		X_A		appId;

	if( bRamBuffer[PROCESS_LEVEL] == ROCESS_LEVEL_OTA )
	{

		appId = g_bSDId[selected_channel];

		bCurentAidAdd = U16t32(getAppletFiled(getIndexByID((appId)), GET_AID_ARRAY));
	}
	else
	{
		bCurentAidAdd = U16t32(getAppletFiled(getIndexByID(U16t8(U16_AND_U16(getChannelContext(selected_channel), APPLET_MASK))), GET_AID_ARRAY));
	}
	bCurentAidAdd = locateObjectInMemory(U32t16(bCurentAidAdd));

	PX_W_U8(aidLength, U16t8(get_array_length(bCurentAidAdd)));

	bCurentAidAdd = get_array_data(MEMtPF8(bCurentAidAdd));

	return bCurentAidAdd;
}
#endif

#if (!GP__VERSION__FOR_MINI_MASK)
u16 checkOPCommandChain(void)
{
	u8 bIns_ = APDUBuffer[OFFSET_INS];
	u8  privilege[3];

#if UICC__OPERATOR__CHINAUNICOM
	if((bIns_ == INS_OP_GET_DATA)||
			(bIns_ == INS_INITIALIZE_UPDATE)||
			(bIns_ == INS_EXTERNAL_AUTHENTICATION)||
			(bIns_ == INS_OP_GET_STATUS)||
			((bIns_ == INS_OP_INSTALL)&&(U8_AND_U8(APDUBuffer[OFFSET_P1], 0x80) == 0x00))||     //INSTALL and Last (or only) command
			(bIns_ == INS_OP_LOAD) //modify by xuxiaojun@20220707 联通测试转log测试：  F2_TC6_ISDAMSSDTest: @TS1.14 在ISD下创建AM SSD并完成个人化，0通道上选择ISD建立安全通道后,发送Install for load指令和load第一条指令，1通道选择SSD建立安全通道后执行storedata第一条指令，再回到0通道继续发送load指令成功，再到1通道执行storedata最后一条指令成功。
		) // todo
	{
		return SW_NO_ERROR;
	}
#endif

	if (g_moreCommands == CHAINING_PUTKEY) //more put key commands
	{
		if (bIns_ != INS_OP_PUT_KEY)
		{
			g_moreCommands = CHAINING_CLEAR;
			g_CommandOffset = 0;
			cacheBuffer[0] = 0;
			cacheBuffer[1] = 0;
		}
	}
	else if (g_moreCommands == CHAINING_INSTALL)  // more install commands
	{
		if (bIns_ != INS_OP_INSTALL)
		{
			g_moreCommands = CHAINING_CLEAR;
			g_CommandOffset = 0;
		}
	}
	else if (g_moreCommands == CHAINING_DELETE)  // more delete commands
	{
		if (bIns_ != INS_OP_DELETE)
		{
			g_moreCommands = CHAINING_CLEAR;
			g_CommandOffset = 0;
		}
	}
	else if(g_moreCommands == CHAINING_STOREDATA)  //more store data commands
	{
		if(bRamBuffer[INSTALL_FOR_PERSONERLIZE] != 0)//sd 通道进行应用的 installforpersonal  + storedata 操作，来了一条A4指令 ，需要中断storedata操作，其他指令不影响  分发给SD
		{
			getPrivilegeByAppId(bRamBuffer[INSTALL_FOR_PERSONERLIZE], &privilege[0]);

			if(U8_AND_U8(privilege[0] , 0x80) == 0x80)//APPID指向SD  表示SD的storedata ，和普通storedata一样处理，  同一个通道上非storedata指令打断操作；
			{
				if(bIns_ != INS_OP_STORE_DATA)
				{
					bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
					bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = 0;
					g_moreCommands = CHAINING_CLEAR;
					g_CommandOffset = 0;
					dataManager_abortStoreData();//SSD可能会开跨指令事务  终止storedata时取消掉事务
	//				g_storeDataSequence[selected_channel] = 0;
				}
			}
			else //  APPID指向applet可以把非storedata指令分发给该通道上的SSD
			{
				if(ValueBit(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL],selected_channel) == 1)//在personal的通道执行指令
				{
					//gp221  :11.11.1 Definition and Scope :  A personalization session ends when:
					//1  ：The card is reset;
					//2 ：The Security Domain is deselected (i.e. another, or the same, applet is selected on the same logical channel);
					//3 ：The Security Domain is selected on the same or another logical channel;（包含开通道隐式选择的SD？）
					//4 ：The Secure Channel session (if any) established by the Security Domain is reset (另一个应用在该通道被选中，逻辑通道被关闭，卡)
					//5 ：The Security Domain receives an INSTALL [for personalize] command (starting a new personalization session for another application);   在opInstallForPersonalize函数中处理
					//6 ：The Security Domain receives a STORE DATA command indicating P1.b8=1 (last block);   在personerlizeApplication函数处理
					// reset的情况不需要处理   2,3,4 在选择指令里处理
				}
				else//不在personal的通道执行指令，排除storedata指令  其他不影响（由于卡片能力限制 g_moreCommands = CHAINING_STOREDATA不区分安全域和应用   卡上只能有一个storedata操作）
				{
					getPrivilegeByAppId(U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF), &privilege[0]);

					if ((U8_AND_U8(privilege[0] , 0x80) == 0x80)&&(bIns_ == INS_OP_STORE_DATA)&&(U8_AND_U8(APDUBuffer[OFFSET_P1],0X80) == 0X00)) //其他通道又来啦级联storedata 指令 中断personal通道的storedata操作
					{
//						bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
//						bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = 0;
//						g_moreCommands = CHAINING_CLEAR;
//						g_CommandOffset = 0;
	//					g_storeDataSequence[selected_channel] = 0;  这里通道号不对

						return SW_CONDITIONS_NOT_SATISFIED; //卡上同时只能有一个storedata ，保持上一个 （如果是安全域单条的storedata，可以执行）
					}
//					else if ((U8_AND_U8(privilege[0] , 0x80) == 0x80)&&(bIns_ == INS_OP_INSTALL)&&(U8_AND_U8(APDUBuffer[OFFSET_P1],0x7F) == P1_OP_INSTALL_FOR_PERSONALIZE))  //其他通道又来INSTALL_FOR_PERSONALIZE, return 6985
//					{
//						return SW_CONDITIONS_NOT_SATISFIED; //其他通道又来INSTALL_FOR_PERSONALIZE, return 6985
//					}
				}
			}
		}
		else //ssd 按原流程执行
		{
			if (bIns_ != INS_OP_STORE_DATA)//这里并没有判断通道  其他通道的任何一条指令都可以打断当前的storedata操作   ,安全域storedata会开跨指令事务，无法控制其他通道上的应用行为
			{
				g_moreCommands = CHAINING_CLEAR;
				g_CommandOffset = 0;
				g_storeDataSequence[selected_channel] = 0;
				dataManager_abortStoreData();//SSD可能会开跨指令事务  终止storedata时取消掉事务
			}
		}
	}

	return  SW_NO_ERROR;
}
/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 * modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR，g_current_SCP11SessionPara用全局变量，g_current_PerformSecurityOperation复用g_current_SCP_cardChallenge
 */
u16 checkOPCommand() 
{
	u8 			D_A		bCla;
	u16 		D_A		result;
	PN8			D_A		sdDataObj;
	
	if(IF_ProfielChangeOngoing() == TRUE)  //modify by xxj@20230317: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls> profile状态切换过程中，禁止所有的GP指令 
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	bCla = APDUBuffer[OFFSET_CLA];
	
#if GP__CGM__FOR_MASK
	/*CGM flag有可能被修改，比如load被打断，会进入GC中将flag设置成NO_CHECK
	  这里设置成CHECK_RUN，则要保证后面操作不是CHECK_RUN的要另外重新设置*/
	CgmInit_changeFlag(CHECK_RUN);
#endif

	result = checkOPCommandChain();
	
	if(result != SW_NO_ERROR)
	{
		return result;
	}


#if GP__SCP__SCP11__FOR_MASK	
	if(bRamBuffer[PROFILE_APDU_MODE] == FALSE) // modify by xuxiaojun@20221211: profile下载过程中，不需要考虑SCP11功能，且安装MNOSD时，sdDataObj还没有创建
	{
		sdDataObj = MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD));

		sdData_getSCP11SessionPara(sdDataObj, MODE_GET_SCP11_SESSIONPARA);

		if((PX_R_U8(g_current_SCP11SessionPara_sessionType) == SCP11_C))
		{
			apdu_ApduSetIncomingAndReceive(); //modify by xuxiaojun@20220627: SCP11C中执行的指令需要获取数据域信息，所以把收数据提到这里来统一进行。对于SCP11c执行之后还会继续往下执行的指令，其在掉用apdu_ApduSetIncomingAndReceive之前统一增加判断apdu_IsSetIncoming() == 0，避免重复收数据时状态字为6F00
			result = check_SCP11cOPCommand();
			if(result != SW_NO_ERROR)
			{
				if(result == SW_CONDITIONS_NOT_SATISFIED)
				{
					//If the script attempts to perform an operation that is not allowed, the command triggering this operation shall
					//be rejected with a status word of '6985', the SCP11c session shall be terminated, and all objects with a
					//“pending completion” mark shall be deleted

					closeSecuritychannel(SC_TERMINATED);// modify by liweimin

					// 需要回滚事务
					if(isInProgress())
					{
						abortTransaction();
					}

				}
				throw_error(result);
			}
		}
	}

#endif

	if((CLA_OP_ISO7816_ENC == bCla) || (CLA_OP_ISO7816_MAC == bCla))
	{
		sdDataObj = MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD));
		g_current_SCP_Type = sdData_getScpType(sdDataObj);
		if(SCP_10 != g_current_SCP_Type)
		{
			throw_error(SW_CLA_NOT_SUPPORTED);//modify by 20230525 cla检查
			//throw_error(SW_INS_NOT_SUPPORTED);
		}
	}
	
	if (0x04 == bCla)
	{
		//处理CLA=0x04的命令，需要校验MAC，但还需要返回0x6E00
		checkIsAuthenticated(DEMAND_AUTHENTICATE);
		return SW_CLA_NOT_SUPPORTED;
	}

	result = check_Op_Command1();
	if(result)
		return result;
	return check_Op_Command2();
}

/*
 * modify by xuxiaojun@20220519 merger from EPX\v3.0\DEVELOP to support SCP11
 */
u16 check_Op_Command1()
{
	PN8			D_A			sdDataObj;
	u16 		D_A			errorCode;
	u8			D_A			bIns_;
	bIns_ = APDUBuffer[OFFSET_INS];

	errorCode = SW_NO_ERROR;
	
	protocol_type	= apdu_GetProtocol();
	
	//当GET STATUS命令的返回数据超过255字节时，如果中间穿插其他命令，此时终端再发一条GET STATUS
	//（获取应用的信息），卡片会在JAVA层判断shortBuf[Constants.STATUS_FLAG_OFF]不等于0，报6A86
#if !(GP__VERSION__FOR_CHINA_UNICOM_MASK || GP__VERSION__FOR_IRAN)
	if ( (bIns_ != 0xF2) && (apdu_GetProtocol() == APDU_PROTOCOL_T0))
	{
		PXB_W_U16(&bShortBuffer[U8_A_U8(STATUS_FLAG_OFF, selected_channel)], 0x0000);
	}
#endif
	
//	if((dataManager_isInStoreDataProcess()) && (INS_OP_STORE_DATA != bIns_))//当前通道有级联storedata  回滚级联storedata  （根据代码逻辑 checkOPCommandChain已经把g_storeDataSequence清掉了，这里判断不出来），应用是不管
//	{
//		dataManager_abortStoreData();
//	}  这里的操作已经在checkOPCommandChain做了

//	if((dataManager_isInStoreDataProcess()) && (INS_OP_INSTALL == bIns_) && (U8_AND_U8(APDUBuffer[OFFSET_P1],0x7F) == P1_OP_INSTALL_FOR_PERSONALIZE))//当前通道有级联storedata  回滚级联storedata  （根据代码逻辑 checkOPCommandChain已经把g_storeDataSequence清掉了，这里判断不出来），应用是不管
//	{
//		dataManager_abortStoreData();
//	} 
	
	if( g_putKeyCommands && (INS_OP_PUT_KEY != bIns_))
	{
			keyManager_InsertApduManager(1);
	}
	
	switch (bIns_)
	{
#if GP__SCP__SCP11__FOR_MASK
		case INS_OP_PERFORM_SECURITY_OPERATION: // 0x2A
		{
			if(((U8_AND_U8(channelInfo.security_level[selected_channel], 0x80) == 0x80) && (U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) >= SCF_AUTHENTICATED))
				|| ((U8_AND_U8(channelInfo.security_level[selected_channel], 0x40) == 0x40) && (U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) >= SCF_ANY_AUTHENTICATED)))
			{
				errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
			}
			else if((channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED)&&(GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))) //modify by xuxiaojun@20220528:没有考虑通道号的问题 AmendH测试脚本-SCP11c:  MEU_start_error_app_selected_on_basic_lc_00_01_0B
			{
				errorCode = checkIsAuthenticated(IGNORE_AUTHENTICATE);
			}
			else
			{
				return SW_CLA_NOT_SUPPORTED;
			}

			return SCP11PerformSecurityOperation(APDU_DATAFIELD_IS_NOT_RECEIVED);
		}
#endif
	case INS_INITIALIZE_UPDATE:
		{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == 0)//本次指令为标准APDU（本地 or RAM，非profile解析得到的指令）
		{
			/* modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>  (COS内部方案，无规范来源)
			 * profile状态切换(active disable MemoryReset)中，GP指令全部禁掉
			 * profile下载过程中，禁止GP指令列表 auth ，load，install，delete，putkey，storedata
			 */
			if(IF_IsProfileLoadingSession() == TRUE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
#endif

		if (GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))
		{
			errorCode = openSecureChannel(APDU_DATAFIELD_IS_NOT_RECEIVED);
			if (errorCode == SW_NO_ERROR
		#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK	
			|| (errorCode == MAX_AUTHENTICATION_WARNING)
		#endif
			   )
			{
				apdu_ApduSetOutgoingAndSend(0x05, U8t16(U8_AND_U8(APDUBuffer[OFFSET_LC], 0xFF)));
			}
		}
		else
		{
			return SW_CLA_NOT_SUPPORTED;
		}
		return errorCode;
		}
	case INS_EXTERNAL_AUTHENTICATION:
		{
			if (GP_84 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK)) //mask logical channel number
			{

				errorCode = verifyExternalAuthenticate(APDU_DATAFIELD_IS_NOT_RECEIVED);

				if ((SW_NO_ERROR != errorCode)/* && (errorCode != MAX_AUTHENTICATION_WARNING)*/)
				{
					// if verifyExternalAuthenticate error first close the Security channel.
					closeSecuritychannel(SC_TERMINATED);// modify by liweimin

					// second update the try count
				#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					managerSDLifeCycle();
				#endif
				}
				else // modify by xuxiaojun@20220927: SCP02和SCP03鉴权成功之后，cardChallenge和hostChallenge的数据已经无用，可以被复用，所以这里需要将其清除
				{
					memFillNoAtomic(MEMtPM8(g_current_SCP_cardChallenge),0x00,CARD_CHALLENGE_LENGTH);
					memFillNoAtomic(MEMtPM8(g_current_SCP_hostChallenge),0x00,HOST_CHALLENGE_LENGTH);
				}

				return errorCode;
			}
#if GP__SCP__SCP11__FOR_MASK
			else if((GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))&&(PX_R_U8(g_current_PerformSecurityOperation_state) == PERFORMSECURITYOPERATION_STATE_END)) // 如果卡片支持的逻辑通道号大于4个，则这里的判断不正确
			{
				errorCode = SCP11MutualAuthenticate(APDU_DATAFIELD_IS_NOT_RECEIVED); // 执行成功之后，state是否需要清掉？

				ecpMem_Set(g_current_PerformSecurityOperation, 0x00, PerformSecurityOperation_LENGTH); //modify by xuxiaojun@20220525  执行完MUTUAL AUTHENTICATE之后，2A指令相关的变量需要清掉: SCP11测试脚本:  C_sm_PSO_SUCCESS_currentSecureSessionNotTerminated_scp11a_256_a1_30_52

				if(errorCode == SW_NO_ERROR)
				{
					apdu_ApduSetOutgoingAndSend(0x05, U8t16(U8_AND_U8(APDUBuffer[OFFSET_LC], 0xFF)));
				}

				return errorCode;
			}
#endif
			else
			{
				return SW_CONDITIONS_NOT_SATISFIED; //SCP11测试脚本:  C_sm_PSO_SUCCESS_PK_OCE_ECKA_temporarily_stored_256_a1_30_70
			}
		}
	case INS_OP_PIN_VERIFY:
	case INS_OP_PIN_CHANGE:
	case INS_OP_PIN_DISABLE:
	case INS_OP_PIN_ENABLE:
				return checkIsAuthenticated(IGNORE_AUTHENTICATE);
	
	case INS_OP_PIN_UNBLOCK:
	case INS_OP_SET_STATUS:
	case INS_OP_GET_STATUS:
				return checkIsAuthenticated(DEMAND_AUTHENTICATE);
#if GP__SCP__SCP10__FOR_MASK
	case INS_OP_GENERATE_KEYPAIR://SCP10增加产生密钥指令
		sdDataObj = getCurrentSdObj(NOT_NEED_CHECK_SD_LOCKED);	
		//g_current_SCP_Type = sdData_getScpType(sdDataObj);
		if(SCP_10 != sdData_getScpType(sdDataObj))
		{
			if(SCP_10 == sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP10_WITHOUT_IOPTION_MODE, 0x00))
				errorCode = checkIsAuthenticated();
			else
				errorCode = SW_INS_NOT_SUPPORTED;
		}
		if (SW_NO_ERROR == errorCode)
		{
			scp10_generateKeyPair();
		}
		return errorCode;	
#endif
	case INS_OP_GET_DATA:
	
		if (JcvmConfig_GetGetDataScSwitch())// modify by xuxiaojun@20211009 去掉GET_DAT_SC重复宏定义
		{
			if (U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) == SCF_NO_AUTHENTICATED)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
		if(((U8_AND_U8(channelInfo.security_level[selected_channel], 0x80) == 0x80) && (U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) >= SCF_AUTHENTICATED))
			|| ((U8_AND_U8(channelInfo.security_level[selected_channel], 0x40) == 0x40) && (U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) >= SCF_ANY_AUTHENTICATED)))
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		}
		else if((channelInfo.security_level[selected_channel] == SCF_NO_AUTHENTICATED)&&(GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK))	// merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP： GET DATA时可以是80CA收数据, 需要考虑通道号的问题 AmendH测试脚本-SCP11c:  MEU_start_error_app_selected_on_basic_lc_00_01_0B
#if M2M_EUICC
		 &&( G_TARRecIndex[0] != TarType_ISDP)	//isdp的时候的getdata不需要进行安全保护，-jiangqingbin-20220808 (用例：HB_Offline_Test/4管理指令异常数据测试/Tele_eUICCCapabilityAudit_GetData_Errortest_01)
#endif
		)
		{
			errorCode = checkIsAuthenticated(IGNORE_AUTHENTICATE);
		}
		//东信 GET DATA 6.1.2.8.13  CLA遍历时，无效84指令检查
		else if (GP_84 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_MASK)) //考虑通道信息
		{
			return SW_CLA_NOT_SUPPORTED;
		}
	
	/*@ -disspecopt @*/
		if (JcvmConfig_GetCardState() == CARD_LOCKED)//GP 221 11章 APDU Command Reference ，GET DATA  ,GET STATUS指令 在在cardlock时可以执行，条件是该SD是FA SD
		{
			if(check_FinalApplicationPrivi(getCurrentSDId()) == FALSE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}

		if ((U8_AND_U8(APDUBuffer[OFFSET_CLA], 0x04) == 0x04) && (JcvmConfig_GetCardState() == CARD_TERMINATED))//(U8_AND_U8(APDUBuffer[OFFSET_CLA], 0x04) == 0x04)的判断没有找到规范来源
		{
			if(check_FinalApplicationPrivi(getCurrentSDId()) == FALSE)//GP221 5.1.1.5  The state TERMINATED: If a Security Domain has the Final Application privilege only the GET DATAcommand shall be processed, all other commands defined in this specification shall be disabled and shall return an error.
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
		return errorCode;
		/*@ +disspecopt @*/
	#if (GP__SCP__SCP01_RMAC || GP__SCP__SCP02_RMAC || GP__SCP__SCP03_RMAC)

	case INS_OP_BEGIN_RMAC_SESSION:  //0x7A
		sdDataObj = MEMtPN8(getCurrentSdObj(IS_NO_NEED_CHECK_SD));
		if(U8_AND_U8(sdData_getScpOption(sdDataObj), 0x20) == 0x00)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
		if (GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_NO_SECURITY_MASK))
		{
			errorCode = operateBeginRmacSession();
		}
		else
		{
			return SW_CLA_NOT_SUPPORTED;
		}
		return errorCode;

	case INS_OP_END_RMAC_SESSION:    //0x78

		if (GP_80 == U8_AND_U8(APDUBuffer[OFFSET_CLA], GP_NO_LOGIC_NO_SECURITY_MASK))
		{
			errorCode = operateEndRmacSession();
		}
		else
		{
			return SW_CLA_NOT_SUPPORTED;
		}
		return errorCode;
#endif
	case INS_OP_GET_RESPONSE:
		if((U8_AND_U8(channelInfo.security_level[selected_channel], 0x80) == 0x80 && (U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF)) > SCF_AUTHENTICATED) ||
		   (U8_AND_U8(channelInfo.security_level[selected_channel], 0x40) == 0x40 && U8_AND_U8(channelInfo.security_level[selected_channel], 0xFF) > SCF_ANY_AUTHENTICATED))
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		}
		return errorCode;

#if GP__VERSION__FOR_CHINA_UNICOM_MASK
	case INS_OP_EXTEND_OPEN_MIFARE:
		if (GP_84 == (APDUBuffer[OFFSET_CLA] & GP_NO_LOGIC_MASK)) //mask logical channel number
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
			if (errorCode != SW_NO_ERROR)
			{
				return errorCode;
			}
			return openMifare(APDUBuffer);
		}

		return SW_CLA_NOT_SUPPORTED;

	case INS_OP_EXTEND_CLOSE_MIFARE:

		if (GP_84 == (APDUBuffer[OFFSET_CLA] & GP_NO_LOGIC_MASK)) //mask logical channel number
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
			if (errorCode != SW_NO_ERROR)
			{
				return errorCode;
			}
			return closeMifare(APDUBuffer);
		}

		return SW_CLA_NOT_SUPPORTED;
#endif

	default:
		return 0;
//	   break;
	}
}
/*
*merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support GP__AMENDMENT__H__FOR_MASK
*/
u16 check_Op_Command2()
{
	#define   BIT0_TAG_TMP		0x00//warning: incompatible redefinition of macro "BIT0_TAG"

	u8 					p1;
	u8					p2;
	u8					bIns_;
	u16 				errorCode;
	u8 					result;
	//PX8					resBuffer;
	ppApdu_Response_t	response;
	u16 				bPrivRef;
	memref				entry;
	memref 				bPrivArray;

	errorCode 			= SW_NO_ERROR;
	result 				= 0;
	apdu_GetResponse(&response);
	//resBuffer			= response->pData;

	bIns_ = APDUBuffer[OFFSET_INS];
	p1 = APDUBuffer[OFFSET_P1];
	p2 = APDUBuffer[OFFSET_P2];

	switch (bIns_)
	{
	case INS_OP_PUT_KEY:
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == 0)//本次指令为标准APDU（本地 or RAM，非profile解析得到的指令）
		{
			/* modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>  (COS内部方案，无规范来源)
			 * profile状态切换(active disable MemoryReset)中，GP指令全部禁掉
			 * profile下载过程中，禁止GP指令列表 auth ，load，install，delete，putkey，storedata
			 */
			if(IF_IsProfileLoadingSession() == TRUE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}

		if(IF_CapLoaingInCurrnetProfile() == TRUE)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}

//		if(IF_StorDataInCurrnetProfile() == TRUE) 这种情况，会在前面把store data数据回滚
//		{
//			return SW_CONDITIONS_NOT_SATISFIED;
//		}
#endif
		if((JcvmConfig_GetSEState() != CARD_OP_READY) || (DEMAND_AUTHENTICATE_OP_READY)) //modify by xuxiaojun@20220621: gpBFCCompliance208: km_APDU_putKeyDes_OPR_00_02 OPREADY状态执行put key时也需要鉴权
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		}
		else //发卡前
		{
			errorCode = checkIsAuthenticated(IGNORE_AUTHENTICATE);//(DEMAND_AUTHENTICATE);
		}

		if (errorCode != SW_NO_ERROR)
		{
			return errorCode;
		}
		/*@ -disspecopt @*/
		if (JcvmConfig_GetCardState() == CARD_LOCKED)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
		/*@ +disspecopt @*/
		if (g_moreCommands == CHAINING_CLEAR)
		{
			cacheBuffer[0] = APDUBuffer[OFFSET_CDATA];   //save key version
			cacheBuffer[1] = APDUBuffer[U8_A_U8(OFFSET_CDATA, 1)];   //save key type
		}

#if (!GP_COMMAND_CHAINING)
			if (U8_AND_U8(p1, 0x80) == 0x80)
			{
				return SW_INCORRECT_P1P2; //《GPC_Common_Implementation_Configuration_v2.0》5.10.2.1：If chaining is not supported, the value of this bit may either be ignored or be rejected with a response of '6A86' when it is set to 1.
			}
#endif
		if (APDUBuffer[OFFSET_CDATA] == KEY_DAP_VERIF_ID1)
		{
			entry		=	getEntryByAppId(getCurrentSDId());
			bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));

			bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
			if(U8_AND_U8(PM_R_U8(bPrivArray), DAP_VERIFICATION_PRIVILEGES_TAG) != DAP_VERIFICATION_PRIVILEGES_TAG)
			{
		 		//return SW_CONDITIONS_NOT_SATISFIED;
//				return SW_WRONG_DATA;
				return SW_FUNC_NOT_SUPPORTED; // modify by xuxiaojun@20220422 gpBFCCompliance208 km_APDU_getData_SEC_6c_be
		 	}

			//Visa GlobalPlatform 2.1.1 Card Implementation Requirements v2.0.pdf: A Supplementary Security Domain may not use the Secure Channel services of its associated Security Domain (i.e. the Issuer Security Domain) to load a public key i.e. the Secure Channel shall be set up using	the Security Domain’s own Secure Channel keys.
			if((CONFIG_GP_FOR_MAPPING_TEST || CONFIG_VGP)&&(g_bSDId[selected_channel] != channelInfo.selected_key_appId[selected_channel]))//added by xuxiaojun@20220505 gpMGCompliance103: km_APDU_putKeyRsa_01_NoDesKeys
			{
				return SW_CONDITIONS_NOT_SATISFIED; //2[1].1.1_Mapping_guidelines_v1.0.1-Final.pdf:  A Supplementary Security Domain may not use the Secure Channel services of its associated Security Domain (i.e. the Issuer Security Domain) to load a public key i.e. the Secure Channel is set up using the Security Domain’s own Secure Channel keys.
			}
		}

		KM_InitPutKeySequence(p1);
		CgmInit_consumeVar();
//		if(p1 == 0x80)
//		{
			//return StoreReceivedCommandAPDU();	
//		}
		break;
	case INS_OP_STORE_DATA:
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == 0) //add by chentao@20230306 //本次指令为标准APDU（本地 or RAM，非profile解析得到的指令）
		{
			/* modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>  (COS内部方案，无规范来源)
			 * profile状态切换(active disable MemoryReset)中，GP指令全部禁掉
			 * profile下载过程中，禁止GP指令列表 auth ，load，install，delete，putkey，storedata
			 */
			if(IF_IsProfileLoadingSession() == TRUE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
		if(IF_CapLoaingInCurrnetProfile() == TRUE) // 这里需要考虑一种情况：store data如果是给应用个人化的话，是否也被禁止掉？
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
#endif
		if((JcvmConfig_GetSEState() != CARD_OP_READY) || (DEMAND_AUTHENTICATE_OP_READY)) //公司内部发卡前可以不用鉴权,如果需要鉴权的话，可以把配置打开
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		}
		else //发卡前
		{
			errorCode = checkIsAuthenticated(IGNORE_AUTHENTICATE);//(DEMAND_AUTHENTICATE);
		}

		if (errorCode != SW_NO_ERROR)
		{
			return errorCode;
		}
/*@ -disspecopt @*/
		else if (JcvmConfig_GetCardState() == CARD_LOCKED)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
/*@ +disspecopt @*/
		if ((U8_AND_U8(p1, 0x80) == 0x00) && (g_moreCommands != CHAINING_STOREDATA)) //more store data commands
		{
			g_moreCommands = CHAINING_STOREDATA;
			g_storeDataSequence[selected_channel] 			= 0;
		}

		if((U8_AND_U8(p1, 0x80) == 0x80) && (g_moreCommands != CHAINING_STOREDATA)&&(p2 != 0))//单条storedata P2!=0
		{
			return SW_INCORRECT_P1P2;
		}
		if(CHAINING_STOREDATA == g_moreCommands)
		{
			//check sequence counter of chaining store data command before sending to target app.
			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
				cms2ac_dataManager_checkSequence(p1);
			#endif
				dataManager_clearBitTag(BIT0_TAG_TMP);
				dataManager_checkSequence(p2);
				U8_INC(g_storeDataSequence[selected_channel]);
		}

		if((bRamBuffer[INSTALL_FOR_PERSONERLIZE] != 0) && (U8_AND_U8(p1, 0x80) == 0x80))
		{
			dataManager_reset(); //测试用例：中国移动测试工程列表-接触-5G\GP_API_工程集\org_globalplatform_Application_Interface\processData\processData1.1.3
		}

		//配置在SWP关机刷卡时，限制电流的最大值为10mA，临时解决方法，后续需要修正
 		if ( (APDUBuffer[OFFSET_P1] == 0xEC) && (APDUBuffer[OFFSET_P2] == 0xEC) )
 		{
 			nvmWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(NO_CURRENT_LIMIT)), APDUBuffer[OFFSET_CDATA]);
 			return SW_NO_ERROR;
 		}
 		//Bit 3 of the CLA   byte of the command header is cleared.
 		APDUBuffer[OFFSET_CLA] =U8_AND_U8(APDUBuffer[OFFSET_CLA],  GP_NO_SECURITY_MASK);
		break;
	case INS_OP_LOAD:
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == 0)//本次指令为标准APDU（本地 or RAM，非profile解析得到的指令）
		{
			/* modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>  (COS内部方案，无规范来源)
			 * profile状态切换(active disable MemoryReset)中，GP指令全部禁掉
			 * profile下载过程中，禁止GP指令列表 auth ，load，install，delete，putkey，storedata
			 */
			if(IF_IsProfileLoadingSession() == TRUE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
#endif
		errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		if (errorCode != SW_NO_ERROR)
		{
			return errorCode;
		}
		/*@ -disspecopt @*/
		if (JcvmConfig_GetCardState() == CARD_LOCKED)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
		/*@ +disspecopt @*/
//		if (JcvmConfig_GetRegistryUpdate() == REGISTRY_UPDATA_DISABLE)//modify by xuxiaojun@20211009 对registry_updateEnable的重复宏定义导致了调用时读取的配置区地址错误。目前该配置无使用，先注释掉
//		{
//			return SW_INS_NOT_SUPPORTED;
//		}
		
		if ( (U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], (U8_INV(U8_SHL_U8(1, selected_channel)))) != 0) // if other logical channel not this one is processing load
			|| (U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], (U8_SHL_U8(1, selected_channel))) == 0) ) // the load process is broken (maybe select command etc.)
		{
			//RAM的load过程也会被同一个SD的其他RAM命令打断
			//if(bRamBuffer[INSTALL_FOR_LOAD_BY_C2] != 0x01)	//command by linguoming
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}

		if ((p1 != 0) && (p1 != U32t8(0x80)))
		{
			//bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
			return SW_INCORRECT_P1P2;
		}
		if (bRamBuffer[INSTALLER_STATE_INDEX] == U32t8(INSTALLER_STATE_ERROR))
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
		if (p2 != bRamBuffer[BLOCK_NUMBER])
		{
			//bRamBuffer[INSTALL_FOR_LOAD] = U8_AND_U8(bRamBuffer[INSTALL_FOR_LOAD], U8_INV(U8_SHL_U8(1, selected_channel)));
			return SW_INCORRECT_P1P2;
		}
#if GP__CGM__FOR_APPPARACHECK
		CgmInit_changeFlag(CHECK_LOAD);
#endif
		return SW_NO_ERROR;

	case INS_OP_INSTALL:
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == 0)//本次指令为标准APDU（本地 or RAM，非profile解析得到的指令）
		{
			/* modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>  (COS内部方案，无规范来源)
			 * profile状态切换(active disable MemoryReset)中，GP指令全部禁掉
			 * profile下载过程中，禁止GP指令列表 auth ，load，install，delete，putkey，storedata
			 */
			if(IF_IsProfileLoadingSession() == TRUE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
#endif
		errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		if (errorCode != SW_NO_ERROR)
		{
			return errorCode;
		}
		/*@ -disspecopt @*/
		if (JcvmConfig_GetCardState() == CARD_LOCKED)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
		/*@ +disspecopt @*/
		if (p2 != 0 /*&& p2 != 0x01 && p2 != 0x02*/)
		{
			return SW_INCORRECT_P1P2;
		}
#if GP_COMMAND_CHAINING
		if( U8_AND_U8(p1, 0x80) == 0x80)
		{
			//return StoreReceivedCommandAPDU();
			if((U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_LOAD) || (U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_INSTALL) || (U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_MAKE_SELECTABLE) || (U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE)
				|| (U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_EXTRADITION) || (U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_PERSONALIZE) || (U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_REGISTRY_UPDATE))
			{
				//add by wuronghua
				g_moreCommands = CHAINING_INSTALL;
				
				ecpMem_Copy(MEMtPM8(U32_A_U16(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE), g_CommandOffset)), MEMtPX8(&APDUBuffer[OFFSET_CDATA]), APDUBuffer[OFFSET_LC]);
				g_CommandOffset = U16_A_U8(g_CommandOffset, APDUBuffer[OFFSET_LC]);
				
				return 0x6310;
			}
			else
			{
				return SW_INCORRECT_P1P2;
			}
		}
		else if (g_moreCommands == CHAINING_INSTALL)
		{
			g_moreCommands = CHAINING_CLEAR;
			
//			ecpMem_Copy(MEMtPM8(U32_A_U16(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE), g_CommandOffset)), MEMtPX8(&APDUBuffer[OFFSET_CDATA]), APDUBuffer[OFFSET_LC]);
//			g_CommandOffset = U16_A_U8(g_CommandOffset, APDUBuffer[OFFSET_LC]);
//			ecpMem_Copy(MEMtPX8(&APDUBuffer[OFFSET_CDATA]), MEMtPM8(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE)), g_CommandOffset);
//			APDUBuffer[OFFSET_LC] = U16t8(g_CommandOffset);

			ecpMem_Move(MEMtPX8(&APDUBuffer[U16_A_U8(OFFSET_CDATA,g_CommandOffset)]), MEMtPX8(&APDUBuffer[OFFSET_CDATA]), APDUBuffer[OFFSET_LC]);
			ecpMem_Copy(MEMtPX8(&APDUBuffer[OFFSET_CDATA]), MEMtPM8(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE)), g_CommandOffset);

			g_CommandOffset = U16_A_U8(g_CommandOffset, APDUBuffer[OFFSET_LC]);
			APDUBuffer[OFFSET_LC] = U16t8(g_CommandOffset);// 可能会超过0x100 无意义 
			
			g_CommandOffset = 0;
		}
#else
			if (U8_AND_U8(p1, 0x80) == 0x80)
			{
				return SW_INCORRECT_P1P2; //《GPC_Common_Implementation_Configuration_v2.0》5.7.2.1：If bit 8 is present and command chaining is not supported, or if the value of this parameter does not otherwise belong to the above list (i.e. ignoring bit 8), a response of '6A86' shall be returned.
			}
#endif

		switch (U8_AND_U8(p1, 0x7f))
		{
        case P1_OP_INSTALL_FOR_LOAD:
			errorCode = opInstallForLoad();
			if(errorCode != SW_NO_ERROR)
			{
				return errorCode;
			}
			bRamBuffer[INSTALL_FOR_LOAD_TO_SDID] = g_bSDId[selected_channel];

			CgmInit_consumeVar();
		    CgmInit_changeFlag(CHECK_LOAD);
			bRamBuffer[BLOCK_NUMBER] = 0;
			break;
		case P1_OP_INSTALL_FOR_INSTALL:
		case P1_OP_INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE:
			errorCode = opInstallForInstall();
			if(errorCode != SW_NO_ERROR)
			{
				return errorCode;
			}
			//break;
		case P1_OP_INSTALL_FOR_MAKE_SELECTABLE:
		case P1_OP_INSTALL_FOR_EXTRADITION:

			CgmInit_consumeVar();
    		CgmInit_changeFlag(CHECK_INSTALL);
			break;
		
		case P1_OP_INSTALL_FOR_PERSONALIZE:
			if(bRamBuffer[INSTALL_FOR_PERSONERLIZE] != 0)
			{
				if(ValueBit(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL],selected_channel) == 1)//install for personal  in  the same channel 
				{
					bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
					bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = 0;
					g_moreCommands = CHAINING_CLEAR;
					g_CommandOffset = 0;
					g_storeDataSequence[selected_channel] 			= 0;
				}
				else
				{
					return SW_CONDITIONS_NOT_SATISFIED;
				}
			}

//			if(P1_OP_INSTALL_FOR_PERSONALIZE == U8_AND_U8(p1, 0x7f))//JAVA层处理指令会把APPID赋值
			{
				bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = U8_OR_U8(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL], U8_SHL_U8(1, selected_channel));
			}
			CgmInit_UpdateTag();
			break;
		case P1_OP_INSTALL_FOR_REGISTRY_UPDATE:
			if(P1_OP_INSTALL_FOR_PERSONALIZE == U8_AND_U8(p1, 0x7f))
			{
				bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = U8_OR_U8(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL], U8_SHL_U8(1, selected_channel));
			}
			CgmInit_UpdateTag();
    		break;
		default:
			return SW_INCORRECT_P1P2;
		}
		break;
		//for ELF Upgrade
#if GP__AMENDMENT__H__FOR_MASK
		case INS_OP_ELF_UPGRADE:
		{
			errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
			if (errorCode != SW_NO_ERROR)
			{
				return errorCode;
			}
			/*@ -disspecopt @*/
			if (JcvmConfig_GetCardState() == CARD_LOCKED)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}

			if (U8_AND_U8(p1, 0x80) == 0x80)
			{
				return SW_INCORRECT_P1P2; //《GPC_Common_Implementation_Configuration_v2.0》5.7.2.1：If bit 8 is present and command chaining is not supported, or if the value of this parameter does not otherwise belong to the above list (i.e. ignoring bit 8), a response of '6A86' shall be returned.
			}

			switch (U8_AND_U8(p1, 0x7f))
			{
				case P1_OP_ELF_UPGRADE_START:
					errorCode =opElfUpgradeStart();
					return errorCode;

				case P1_OP_ELF_UPGRADE_RESUME:
				case P1_OP_ELF_UPGRADE_RECOVERY:
				case P1_OP_ELF_UPGRADE_ABORT:
				case P1_OP_ELF_UPGRADE_STATUS:
					return errorCode;
				default:
					return SW_INCORRECT_P1P2;
			}
			return SW_NO_ERROR;
		}
#endif
	case INS_OP_DELETE:
#if (EUICC_ONLY||SE_AND_EUICC)
		if(bRamBuffer[PROFILE_APDU_MODE] == 0)//本次指令为标准APDU（本地 or RAM，非profile解析得到的指令）
		{
			/* modify by xuxiaojun@20230307: <新一代ESIM产品质量整改方案-20230301-讨论后new.xls>  (COS内部方案，无规范来源)
			 * profile状态切换(active disable MemoryReset)中，GP指令全部禁掉
			 * profile下载过程中，禁止GP指令列表 auth ，load，install，delete，putkey，storedata
			 */
			if(IF_IsProfileLoadingSession() == TRUE)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
		}
#endif
		errorCode = checkIsAuthenticated(DEMAND_AUTHENTICATE);
		if (errorCode != SW_NO_ERROR)
		{
			return errorCode;
		}
		/*@ -disspecopt @*/
		if (JcvmConfig_GetCardState() == CARD_LOCKED)
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
#if M2M_EUICC
		if(checkDataFieleExercisable() == FALSE)//拦截普通SD对ISDP的操作
		{
			return SW_WRONG_DATA;
		}
#endif
		/*@ +disspecopt @*/
		//p2 = 40 时 东信delete指令测试6.1.2.3.6需要返回6a86
		if ((p2 != 0) && (p2 != U32t8(0x80))
#if GP__AMENDMENT__C__FOR_MASK
				&& p2 != 0x40
#endif
				             )	 //p2 = 40 :Delete a root Security Domain and all associated Applications
		{
			return SW_INCORRECT_P1P2;
		}

		if ( U8_AND_U8(p1, 0x7F) != 0x00)
		{
			return SW_INCORRECT_P1P2;
		}
#if GP_COMMAND_CHAINING
		if( U8_AND_U8(p1, 0x80) == 0x80)
		{
			if(!CONFIG_GP221)
			{
				return SW_INCORRECT_P1P2; //modify by xuxiaojun@20220422 gpBFCCompliance208 ccm_APDU_GP211_delete_SEC_13_WrongP1 GP211的delete指令不支持级联
			}
			//return StoreReceivedCommandAPDU();
			
			//add by wuronghua
			g_moreCommands = CHAINING_DELETE;
			
			ecpMem_Copy(MEMtPM8(U32_A_U16(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE), g_CommandOffset)), MEMtPX8(&APDUBuffer[OFFSET_CDATA]), APDUBuffer[OFFSET_LC]);
			g_CommandOffset = U16_A_U8(g_CommandOffset, APDUBuffer[OFFSET_LC]);
			
			return 0x6310;
		}
		else if (g_moreCommands == CHAINING_DELETE)
		{
			//add by wuronghua
			g_moreCommands = CHAINING_CLEAR;
//			ecpMem_Copy(MEMtPM8(U32_A_U16(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE), g_CommandOffset)), MEMtPX8(&APDUBuffer[OFFSET_CDATA]), APDUBuffer[OFFSET_LC]);
//			g_CommandOffset = U16_A_U8(g_CommandOffset, APDUBuffer[OFFSET_LC]);
//			ecpMem_Copy(MEMtPX8(&APDUBuffer[OFFSET_CDATA]), MEMtPM8(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE)), g_CommandOffset);
//			APDUBuffer[OFFSET_LC] = U16t8(g_CommandOffset);
			//避免 sysConfig_GetTempArrBuffer过短 冲掉其余buffer
			ecpMem_Move(MEMtPX8(&APDUBuffer[U16_A_U8(OFFSET_CDATA,g_CommandOffset)]), MEMtPX8(&APDUBuffer[OFFSET_CDATA]), APDUBuffer[OFFSET_LC]);
			ecpMem_Copy(MEMtPX8(&APDUBuffer[OFFSET_CDATA]), MEMtPM8(sysConfig_GetTempArrBuffer(APDU_BUFFER_SIZE)), g_CommandOffset);

			g_CommandOffset = U16_A_U8(g_CommandOffset, APDUBuffer[OFFSET_LC]);
			APDUBuffer[OFFSET_LC] = U16t8(g_CommandOffset);//可能会超过1字节  无意义
			
			g_CommandOffset = 0;

		}
#else
			if (p1 != 0x00)
			{
				return SW_INCORRECT_P1P2; //《GPC_Common_Implementation_Configuration_v2.0》5.2.2.1：If chaining is not supported and a value other than '00' is present in this parameter, a response of '6A86' shall be returned.
			}
#endif
		if((APDUBuffer[OFFSET_CDATA] == 0xD0) || (APDUBuffer[OFFSET_CDATA] == 0xD2))
		{
			//result = deleteKeySet();
			result	 =	delSetKey();
			if(result == true)
			{
				return SW_NO_ERROR;
			}
			else if (result == 2)
			{
				return SW_CONDITIONS_NOT_SATISFIED;
			}
			else
			{
				return SW_REFERENCED_DATA_NOT_FOUND;
			}
		}
		
		CgmInit_consumeVar();
//		CgmManager_setDeleteFlag(DELETE_PKG_APP);
		break;

	case U32t8(0xE1): //清ISD counter 私有指令
#if UICC__OPERATOR__CHINAMOBILE
		if (JcvmConfig_GetSEState() > CARD_OP_READY)// 移动5G备案  GP功能补充测试脚本_接触_工程集/东信/无效INS遍历    选择ISD后测试  ，期望E1为非法指令
		{
			return SW_INS_NOT_SUPPORTED;
		}
#endif
		break;

	default:
		return SW_INS_NOT_SUPPORTED;
	}
	return SW_NO_ERROR;

}
#endif

#define	CHECK_TOKEN_KEY_STATUS   			0x01
#define	CHECK_RECEIPT_KEY_STATUS  			0x02
#define	CHECK_DAP_KEY_STATUS  				0x03
#if (GP__TOKEN__FOR_MASK | GP__DAP__FOR_MASK | GP__RECEIPT__FOR_MASK)
void OPGetKeySetStatus(void)
{
	boolean	result;
	u8 checkType;
	u8 keyVersion;
	jref	sdRef;
	jref	keySetRef;
	PN8	sdDataObj;
	
	result = true;
	keyVersion = 0;
	
#if (GP__TOKEN__FOR_MASK | GP__DAP__FOR_MASK | GP__RECEIPT__FOR_MASK)
	checkType		= U16t8(popShort());
	sdRef				= popReference();
	switch(checkType)
	{
#if	GP__TOKEN__FOR_MASK
		case CHECK_TOKEN_KEY_STATUS:
			keyVersion = TOKEN_KEY_VERSION;
		break;
#endif
#if	GP__RECEIPT__FOR_MASK
		case CHECK_RECEIPT_KEY_STATUS:
			keyVersion = RECEIPT_KEY_VERSION;
		break;
#endif
#if	GP__DAP__FOR_MASK
		case CHECK_DAP_KEY_STATUS:
			keyVersion = DAP_KEY_VERSION;
		break;
#endif
	}
	
	sdDataObj			= MEMtPN8(locateObjectInMemory(sdData_getSDdataRef(sdRef)));
	
	keySetRef = keySet_getKeySet(sdDataObj, keyVersion);
	if(REF_NULL == keySetRef)
	{
		result = false;
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)
	{
		result = true;
	}
#endif

#endif
	/*@ -typedefparam @*/
	/*@ -discast @*/
	/*@ -type @*/
	pushShort((jshort)result);
	/*@ +type @*/
	/*@ +discast @*/
	/*@ +typedefparam @*/
}
#endif
#if	GP__RECEIPT__FOR_MASK
void OPCMGenData(void)
{
	PD8		X_A		inBuffer;
	u16 	X_A		confirmationCounter;
	u16 	X_A		inOffset;
	u16 	X_A		inLength;
	u8 		X_A		tempResult;

	confirmationCounter = popShort();
	inLength = popShort();
	inOffset = popShort();

	inBuffer = MEMtPX8(getJavaObjectDataArray());

	tempResult = ReceiptGeneration(inBuffer, inOffset, inLength, confirmationCounter);

	pushShort(U8t16(tempResult));
}
#endif

#if (GP__TOKEN__FOR_MASK | GP__DAP__FOR_MASK)
void OPCMVerifyImplement(void)
{

#define			SD_ID_FOR_DAP			byteTagLen
	PD8		X_A		inBuffer;
	u8 		X_A		calType;
#if	GP__TOKEN__FOR_MASK
	u8 		X_A		combined_LoadInstallandMakeSelectable;
#endif
	u8 		X_A		byteTagLen;
	u16 	X_A		inOffset;
	u16 	X_A		inLength;
	u8 		X_A		tempResult;

#if	GP__TOKEN__FOR_MASK
	combined_LoadInstallandMakeSelectable = U16t8(popShort());
#else
	popShort();
#endif
	calType = U16t8(popShort());
	byteTagLen = U16t8(popShort());
	inLength =  popShort();
	inOffset = popShort();

	inBuffer = MEMtPX8(PXtMEM(getJavaObjectDataArray()));

	switch(calType)
	{
#if	GP__DAP__FOR_MASK
		case SD_AGENT_DAP_CAL:
			tempResult = verifyDAP(inBuffer, inOffset, inLength, SD_ID_FOR_DAP);
			break;
#endif
#if	GP__TOKEN__FOR_MASK
		case SD_AGENT_TOKEN_CAL:
			tempResult = verifyToken(inBuffer, inOffset, inLength, combined_LoadInstallandMakeSelectable, byteTagLen);
			break;
#endif
		default:
			tempResult = false;
			break;
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)
	{
		tempResult = true;
	}
#endif
	pushShort(U8t16(tempResult));
}
#endif



#if GP__SCP__API__FOR_MASK

void setNoRmac()
{
	g_SCP_appEnc = 1;
}

/*
*@Function: SCPAPIImplement
*
*@Descrip:  SCPAPIInterfaceImplement中的公共函数
*
*@See:      org.globalplatform.SecureChannel.java / org.globalplatform.SecureChannelx2.java
*           public static native short SCPAPIImplement(byte channel, byte[] inBuffer, short inOffset, short inLength, byte[] key, byte bSecurityLevel, short SCPAPITpye);
*
*@Param:    byte channel
*@Param:    byte[] inBuffer
*@Param:    short inOffset
*@Param:    short inLength
*@Param:    byte[] key
*@Param:    byte bSecurityLevel
*@Param:    short SCPAPITpye
*
*@Return:   the number of bytes to be output 
*
*@History:  V1.0-- 创建该函数
*/
void SCPAPIImplement(void)
{
	u8 		X_A		bchannel;
	u8 		X_A		bSCPAPITpye;
	u8 		X_A		bSecurityLevel;
	PM8		X_A		SrcAddr; // PX8的话会强制转换成物理地址，不会考虑cacheBuffer
	u16 	X_A		boffset;
	u16 	X_A		sLength;
	jref 	X_A		bKey_ref;
	jref 	X_A		srcBuffer;
	u16 	X_A		result;
	memref  X_A		tempAddress;

	bSCPAPITpye = U16t8(popShort());
	bSecurityLevel = U16t8(popShort());

	bKey_ref = popReference(); // 目前JAVA层传入的该参数都为null
	sLength =  popShort();
	boffset = popShort();
	srcBuffer = popReference();
	
	SrcAddr = 0;

	if(REF_NULL == srcBuffer)
	{
		//SrcAddr = APDUBuffer;
		/*modify by zhxx@20221215,GPAPI1.6套件测试：
gpapi_testApplet_92_SC_decryptData_error_nullBuffer_scp02_de_92_cd
gpapi_testApplet_92_SC_encryptData_error_nullBuffer_scp02_de_92_4d
gpapi_testApplet_92_SC_unwrap_error_nullBuffer_de_92_22
gpapi_testApplet_92_SC_wrap_error_nullBuffer_de_92_24*/
		#if UICC__OPERATOR__CHINAUNICOM
			throw_error(NULL_POINTER_EXCEPTION);
		#else
		    throw_error(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION);
		#endif
	}
	else
	{
		tempAddress = locateObjectInMemory(srcBuffer);
		check_bounds_interval(MEMtPF8(tempAddress), U16tS16(boffset), U16tS16(sLength));
		SrcAddr = PM8_A_U16(MEMtPM8(get_array_data(MEMtPF8(tempAddress))), boffset);
	}
	bchannel = U16t8(popShort());

	result = SCPAPI_implementMethod( bSCPAPITpye, MEMtPX8(SrcAddr), sLength, bchannel, bSecurityLevel, bKey_ref);

	pushShort(result);
}
#endif
/**
 * Called from installer (Installer.java) to update the ATR string
 * @param bLen the length of ATR string
 * @param pbBuf the buffer of ATR string
 * @param sOff the offset of ATR string
 */
void setATRHistBytesNM(void) 
{
	
	u8 							bHistLen_;
	PR8 						pbBuffer;
	
	bHistLen_ 					= U16t8(popShort());
	pbBuffer					= getArrayDataWithoutCheck();

	if (bHistLen_ > 15)
	{
		return;
	}
	
#if !BOOTLOADER_PRO
	//修改冷复位ATR的历史字节
	modifyAtrValue(0x00, 0x00, pbBuffer, bHistLen_);
#endif
}

#if GP__GP_API__FOR_V1_7_MASK
/**
 * @description: 获取ATR的历史字节
 *
 * @function: getATRHistBytes
 *
 * @param: buf the buffer where ATRHistByte shall be write
 * @param: sOff the offset where ATRHistByte shall be write
 *
 * @return: len the length of the ATRHistByte
 */
u8 getATRHistBytesNM(void)
{
	u8 							len=0;
	PR8 						buf;

	buf					= getArrayDataWithoutCheck();

	memCopyToRam((PX8)&len,MEMtPM8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_HISTLEN)),1);
	memCopyToRam((PX8)buf,MEMtPM8(JcvmConfig_GetVmConfigAddr(IOCONFIG_COLD_HISTBYTES)),len);

	return len;
}

#if BOOT__IO__SWP
/**
 * @description: 获取卡片UID/PUPI
 *
 * @function: getUIDorPUPIBytes
 *
 * @param: buf the buffer where UID/PUPI shall be write
 * @param: sOff the offset where UID/PUPI shall be write
 *
 * @return: len the length of the UID/PUPI
 */
u8 getUIDorPUPIBytesNM(void)
{
	u8 							len=0;
	PR8 						buf;
	u8							type;
	
	buf					= getArrayDataWithoutCheck();

	type = get_swp_protocol_type();
	
	if(type == 0x81)				//get UID
	{

#if JCVM__IO__SWP

		memCopyAtomic(MEMtPM8(&len), MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_TYPEA_UID_LEN)), 1);
		memCopyAtomic(MEMtPM8(buf), MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_TYPEA_UID)), len);

#endif
	}else if(type == 0x91)		//get PUPI
	{
#if JCVM__IO__SWP
		
		memCopyAtomic(MEMtPM8(&len), MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_TYPEB_PUPI_LEN)), 1);
		memCopyAtomic(MEMtPM8(buf), MEMtPM8(JcvmConfig_GetVmConfigAddr(SWP_TYPEB_PUPI)), len);

#endif
	}
	return len;
}
#endif

#endif
#if  !GP__VERSION__FOR_MINI_MASK
//modify by xuxiaojun@20220621 与java层定义的参数对应，CPLC_DATA_CONF使用缺陷修改为JcvmConfig_GetCplcDataConfBase()
void Get_CPLC_Data(void) 
{
	u16 					length;
	u16 					offset;
	PM8 					pbBuffer;
	u16 					srcOff;

	length 					= popShort();
	offset 					= popShort();
	pbBuffer 				= MEMtPM8(U32_A_U16(getJavaObjectDataArray(), offset));
	srcOff 					= popShort();

	if (srcOff < 42)
	{
		while (length)
		{
			//*(MEMtPM8(pbBuffer) = memReadU8(MEMtPM8(CPLC_DATA_CONF + srcOff));
			memWriteU8NoAtomic(MEMtPM8(pbBuffer), memReadU8(MEMtPM8(U32_A_U16(JcvmConfig_GetCplcDataConfBase(), srcOff))));
			srcOff++;
			pbBuffer += 1L;
			length--;
		}
	}
}
//modify by xuxiaojun@20220621 与java层定义的参数对应，CPLC_DATA_CONF使用缺陷修改为JcvmConfig_GetCplcDataConfBase()
void Put_CPLC_Data(void) 
{
	u16 					length;
	u16 					offset;
	PM8 					pbBuffer;
	u16 					srcOff;
	
	length 					= popShort();
	offset 					= popShort();
	pbBuffer 				= MEMtPM8(U32_A_U16(getJavaObjectDataArray(), offset));
	srcOff 					= popShort();

	if ((srcOff >= 18) && (srcOff < 42))
	{
		memCopyAtomic(MEMtPM8(U32_A_U16(JcvmConfig_GetCplcDataConfBase(), srcOff)), pbBuffer, length);
	}
}
#endif
 
//void manageKeyMapOfSD()
//{
//	u8 type;
//	u8 AppId;
//	u8 sTagOff;
//	u8 bC9Len;
//
//	bC9Len = U16t8(popShort());
//	sTagOff = U16t8(popShort());
//	AppId = U16t8(popShort());
//	type  = U16t8(popShort());
//
//	if(SD_INSTALL == type)
//	{
//		//initSDKeyMap(AppId, sTagOff, bC9Len);
//	}
//	else
//	{
//		//deleteSDKeyMap(AppId);
//	}
//}

//void initSDKeyMap(u8 AppId, u8 sTagOff, u8 bC9Len)
//{
//#if !GP__VERSION__FOR_MINI_MASK
//
//	u8 bDapKeyVersion;
//	u8 i, bTag, bKeySetNum;
//	u8  bSDType, bMaxAuthAttempt, bKeyNum, boffset;
//#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//	u16 bC982Value = 0x2020,bC987Value = 0x2020,bC983Value = 0x20;
//#else
//	u16 bC982Value = 0,bC987Value = 0,bC983Value = 0;
//#endif
//
//	u8  bLen = 0;
//	KeyMap  * m_KeyMaps = (KeyMap  *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
//	u16 sProtocol =  PN_R_U16(&m_KeyMaps[0].sProtocol);
//	u8 value1 = 0,value2 = 0;
//	u16 sRam;
//	u32 iNvm;
//	u32  iKeySetAddr/*, tempAddr*/;
//#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//	u8 CvmFormat, CvmOffset, cvmNum, CvmValueLenth, CvmMaxLenth;
//	u8 SDTypeExtern = 0x15;  //默认接收ISD的请求
//#endif
//
//#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//	u8 has45 = 0;
//	u8 has46 = 0;
//	u8 has4B = 0;
//	u8 has48 = 0;
//	u8 has49 = 0;
//	u8 has4A = 0;
//#endif
//	u8 has81 = 0;
//	u8 hasC982 = 0;
//	u8 hasC983 = 0;
//	u8 hasC987 = 0;
//
//	sRam = 0xFFFF;
//	iNvm = 0xFFFFFFFF;
//	bMaxAuthAttempt = 0xFF;
//	bDapKeyVersion = 0x73;
//	bSDType = 0;
//	bLen = 0;
//	bKeySetNum = 6; //default key set number
//	bKeyNum = 16; //default key number
//	cacheBuffer[(cacheBuffer[0] + 1)] = 0;	//CVM num = 0
//	bRamBuffer[AUTHFAILCNT] = 0xFF;
//	while(bC9Len > 0)
//	{
//		sTagOff += bLen;
//        bTag = APDUBuffer[sTagOff++];
//        bLen = APDUBuffer[sTagOff++];
//		if(bLen > 0x7F)
//		{
//			bLen   = APDUBuffer[sTagOff++];
//			bC9Len --;
//		}
//
//        bC9Len = bC9Len - bLen - 2;
//			switch(bTag)
//			{
//#if GP__VERSION__FOR_IRAN
//			case 0x45:
//				bC982Value = 0xF8F8;
//				bC9Len = 0;
//				break;
//#endif
//       #if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//
//				case 0x45:
//				if(CONFIG_VGP)
//				{
//					//bRamBuffer[INSTALL_APPLET_EXTRADTIONFLAG] = 1;
//					bC9Len = 0;
//				}
//				else
//				{
//					if ((bLen != 1) || (has45 != 0))
//					{
//						pushShort(SW_WRONG_DATA);
//						return;
//					}
//					has45 = 1;
//					if ( (APDUBuffer[sTagOff] & 0x01) == 0x01 )
//					{
//						bSDType |= SD_RECEIPT_GENERATION;
//					}
//					if ( (APDUBuffer[sTagOff] & 0x02) == 0x02 )
//					{
//						bSDType |= SD_NOT_ALLOW_DELETE;
//					}
//					if ( (APDUBuffer[sTagOff] & 0x20) == 0x20 )
//					{
//						bSDType |= SD_NOT_ALLOW_LOCK_FROM_ISD;
//					}
//				}
//
//							break;
//
//        case 0x46:
//				bDapKeyVersion = APDUBuffer[sTagOff];
//				if((bLen != 1) || (has46 == 1)) {
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				has46 = 1;
//				break;
//
//			 case 0x47:
//					//sProtocol = (u16)((APDUBuffer[sTagOff] << 8)) | APDUBuffer[sTagOff+1];
//					//sProtocol = *(u16 *)&APDUBuffer[sTagOff];
//				break;
//
//			 case 0x48:
//				bRamBuffer[AUTHFAILCNT] = APDUBuffer[sTagOff];
//				if((has48 != 0) || (bLen != 1)) {
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				has48 = 1;
//				break;
//
//       case 0x49:
//				//sRam = *(u16 *)&APDUBuffer[sTagOff];
//				sRam = PX_R_U16(&APDUBuffer[sTagOff]);
//				//iNvm = *(u32 *)&APDUBuffer[sTagOff+2];
//				iNvm = PX_R_U32(&APDUBuffer[sTagOff+2]);
//				bSDType |= SD_TYPE_FIX_SIZE;
//				if((has49 != 0) || (bLen != 0x06)) {
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				has49 = 1;
//            	break;
//
//       case 0x4A:
//				bRamBuffer[MAXSYMMETRICKEYCNT] = APDUBuffer[sTagOff];
//
//				if((has4A != 0) || (bLen != 0x01)) {
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//
//				i = (bKeyNum % 3);
//				if (i)
//				{
//					bKeySetNum = bKeyNum / 3 + 1;
//				}
//				else
//				{
//					bKeySetNum = bKeyNum / 3;
//				}
//
//				has4A = 1;
//        break;
//
//
//			case 0x4B:
//			{
//				if(has4B != 0) {
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				CvmOffset = (cacheBuffer[0] + 2);
//				cvmNum = 0;
//				while(bLen != 0)
//				{
//					if((APDUBuffer[sTagOff] != 1)
//						|| (APDUBuffer[(sTagOff+3)] != 1)
//						|| (APDUBuffer[(sTagOff+5)] != 1))
//					{
//						pushShort(SW_WRONG_DATA);
//						return;
//					}
//					cacheBuffer[CvmOffset] = APDUBuffer[(sTagOff+2)];	//CVM ID
//					if(cacheBuffer[CvmOffset] == 0)
//					{
//						pushShort(SW_WRONG_DATA);
//						return;
//					}
//					cacheBuffer[(CvmOffset+1)] = APDUBuffer[(sTagOff+4)];	//CVM most veriry time
//					cacheBuffer[(CvmOffset+2)] = APDUBuffer[(sTagOff+4)];	//CVM remaind veriry time
//					CvmFormat = APDUBuffer[(sTagOff+1)];	//CVM format
//					if((CvmFormat < 1) || (CvmFormat > 3))
//					{
//						pushShort(SW_WRONG_DATA);
//						return;
//					}
//					cacheBuffer[(CvmOffset+3)] = CvmFormat;
//					CvmMaxLenth = APDUBuffer[(sTagOff+6)];							//CVM max length
//					CvmValueLenth = APDUBuffer[(sTagOff+7)];						//CVM length
//					if(CvmValueLenth > CvmMaxLenth)
//					{
//						pushShort(SW_WRONG_DATA);
//						return;
//					}
//					cacheBuffer[(CvmOffset+4)] = CvmValueLenth;
//					for(i = 0; i<CvmValueLenth; i++)
//					{
//						cacheBuffer[(CvmOffset+5+i)] = APDUBuffer[(sTagOff+8+i)];
//					}
//					CvmOffset += GLOBAL_PIN_DATA_SIZE;
//					bLen = bLen - 8 - CvmValueLenth;
//					sTagOff = sTagOff + 8 + CvmValueLenth;
//					cvmNum ++;
//				}
//				CvmOffset = cacheBuffer[0] + 1;
//				cacheBuffer[CvmOffset] = cvmNum;
//				has4B = 1;
//			}
//			break;
//			#endif
//
//			case 0x81:
//			{
//				boffset = sTagOff;
//				if((has81 ==1) && (APDUBuffer[sTagOff] == value1) &&(APDUBuffer[sTagOff+1] == value2))
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//				if(( APDUBuffer[sTagOff] == 0x80) && (APDUBuffer[sTagOff+1]!=0x00))
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//#endif
//				if(( APDUBuffer[sTagOff] == 0x02) && (APDUBuffer[sTagOff+1]!=0x15)
//                                                      &&(APDUBuffer[sTagOff+1]!=0x35)
//			#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//                                                       &&(APDUBuffer[sTagOff+1]!=0x55)
//                                                       &&(APDUBuffer[sTagOff+1]!=0x75)
//			#endif
//																																											)
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				if(( APDUBuffer[sTagOff] == 0x10) && (APDUBuffer[sTagOff+1]!=0x01)
//                                                   &&(APDUBuffer[sTagOff+1]!=0x02) )
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//// 				if((APDUBuffer[sTagOff] == 0x10)&& ((APDUBuffer[sTagOff+1]==0x01)||(APDUBuffer[sTagOff+1]==0x02)))
//// 				{
//// 					sesKeyFlag = 0x80;
//// 				}
//				if(( APDUBuffer[sTagOff] != 0x02) &&(APDUBuffer[sTagOff] != 0x80)
//#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//					&&(APDUBuffer[sTagOff] != 0x03) && (APDUBuffer[sTagOff] != 0x01)&& (APDUBuffer[sTagOff] != 0x10)
//#endif
//#if		GP__AMENDMENT__B__FOR_MASK
//					&& (APDUBuffer[sTagOff] != 0x81)
//#endif
//					)
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				//SDScpProtocolFlag = 0;
//				if ((bLen != 0x02) /*|| (has81 != 0)*/)
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				sProtocol = U16_SHL_U8(U8t16(APDUBuffer[sTagOff]), 8);
//				sProtocol = U16_OR_U16(sProtocol, U8t16(APDUBuffer[U8_A_U8(sTagOff, 1)]));
//
//				value1 = APDUBuffer[U8_INC(boffset)] ;
//				value2 = APDUBuffer[U8_INC(boffset)] ;
//			}
//				has81 = 1;
//            	break;
//
//			case 0x82:
//				boffset = sTagOff;
//				if((hasC982 == 1) || ((bLen != 0x01)
//													#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//															&& (bLen != 0x02)
//													#endif
//																							))
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				if(bLen == 0x01)
//				{
//					bC982Value = U16_OR_U16(U16_SHL_U8(U8t16(APDUBuffer[boffset]),  8), U16_AND_U16(U8t16(APDUBuffer[boffset]),  0x00FF));
//				}else
//				{
//					bC982Value = U16_SHL_U8(U8t16(APDUBuffer[boffset]), 8);
//					U8_INC(boffset);
//					bC982Value = U16_OR_U16(bC982Value, U16_AND_U16(U8t16(APDUBuffer[boffset]), 0x00FF));
//				}
//				hasC982 =1;
//			break;
//			case 0x83:
//				if((hasC983 == 1) || (bLen != 0x01))
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				bC983Value = U8t16(APDUBuffer[sTagOff]);
//				hasC983 =1;
//			break;
//			case 0x84:
//			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//				pushShort(SW_WRONG_DATA);
//				return;
//			#else
//				if(APDUBuffer[U8_S_U8(sTagOff, 1)] != 0x00)
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				else//保存存在Tag84的标志
//					bRamBuffer[LIFECYCLETRANSITION] = 0x01;
//			#endif
//			break;
//#if		GP__AMENDMENT__B__FOR_MASK
//		case 0x85:
//			break;
//#endif
//			case 0x87:
//				boffset = sTagOff;
//				if((hasC987 == 1) || ((bLen != 0x01)
//													#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//														&& (bLen != 0x02)
//													#endif
//																							))
//				{
//					pushShort(SW_WRONG_DATA);
//					return;
//				}
//				if(bLen == 0x01)
//				{
//					bC987Value = U16_OR_U16(U16_SHL_U8(U8t16(APDUBuffer[boffset]), 8), U16_AND_U16(U8t16(APDUBuffer[boffset]), 0x00FF));
//				}else
//				{
//					bC987Value = U16_SHL_U8(U8t16(APDUBuffer[boffset]), 8);
//					U8_INC(boffset);
//					bC987Value = U16_OR_U16(bC987Value, U8t16(APDUBuffer[boffset]));
//				}
//				hasC987 =1;
//			break;
//		case 0x86:
//		{
//			#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
//				pushShort(SW_WRONG_DATA);
//				return;
//			#endif
//		}
//			break;
//
//		default:
//			pushShort(SW_WRONG_DATA);
//			return;
//    }
//	}
//
//	if(has81==0)
//	{
//	  pushShort(SW_WRONG_DATA);
//	}
//
//	if(AppId == 0)
//	{
///*@ -typedefparam @*/
///*@ -type @*/
//		pushShort(U16tS16(SW_NO_ERROR));
///*@ -type @*/
///*@ +typedefparam @*/
//		return;
//	}
//
//	bKeySetNum = 0;
//	iKeySetAddr = 0;
//	//sesKeyFlag	= 0;
//
//	for (i = 0; i < MAX_SD_NUM; U8_INC(i))
//	{
//		if(m_KeyMaps[i].bSDId == 0xFF)
//		{
//			nvmWriteU8Atomic(MEMtPN8(&m_KeyMaps[i].bSDId), AppId);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bDapKeyVersion), bDapKeyVersion);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bSDType), bSDType);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bKeySetNum), bKeySetNum);
//
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bDefaultKeyVersion), 0xFF);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bRsaKeyModuleLen), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bTokenRsaKeyExpLen), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bReceiptRsaKeyExpLen), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bRsaKeySetFlag), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bMaxAuthAttempt), bMaxAuthAttempt);
//
//			nvmWriteU8NoAtomic(MEMtPN8(&(m_KeyMaps[i].bCAPublickeyExpLen)), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&(m_KeyMaps[i].bCASDPriAUTLen)), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&(m_KeyMaps[i].bCASDpriCTLen)), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&(m_KeyMaps[i].bCASDKeyFlag)), 0x00);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bTryAuthAttempt), 0x00);
//			nvmWriteU16NoAtomic(MEMtPN8(&m_KeyMaps[i].sProtocol), sProtocol);
//			//nvmWriteU16Atomic((memref)&m_KeyMaps[i].sRam, sRam);
//			//nvmWriteU32Atomic((memref)&m_KeyMaps[i].iNvm, iNvm);
//
//			nvmWriteU32NoAtomic(MEMtPN8(&m_KeyMaps[i].iKeySetAddr), iKeySetAddr);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bMaxKeyNum), bKeyNum);
//			nvmWriteU8NoAtomic(MEMtPN8(&m_KeyMaps[i].bKeyNum), 0);
//
//			//PN_W_U8((memref)&m_KeyMaps[i].bSDTypeExtern, SDTypeExtern);
//
//			nvmCopyNoAtomic(MEMtPN8(&m_KeyMaps[i].bPriv), MEMtPN8(&bRamBuffer[INSTALL_APPLET_PRIV]),PRIVMAXBYTES);
//			nvmWriteU8NoAtomic(MEMtPN8(&(m_KeyMaps[i].bC983Value)), bC983Value);
//			nvmWriteU16NoAtomic(MEMtPN8(&(m_KeyMaps[i].bC982Value)), bC982Value);
//			nvmWriteU16NoAtomic(MEMtPN8(&(m_KeyMaps[i].bC987Value)), bC987Value);
//
//			pushShort(SW_NO_ERROR);
//			return;
//		}
//	}
//#endif
//
//	pushShort(SW_REFERENCED_DATA_NOT_FOUND);
//	return;
//}

//void deleteSDKeyMap(u8 sdId)
//{
//
//#if !GP__VERSION__FOR_MINI_MASK
//	u8 i;
//	u8 transactionStarted;
//	KeyMap  * m_KeyMaps;
//
//	transactionStarted 		= 0;
//	/*@ -discast @*/
//	m_KeyMaps 				= (KeyMap  *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
//	/*@ +discast @*/
//	if (!isInProgress())
//	{
//    	begin_transaction(SINGLE_APDU_PROCESS);
//   		transactionStarted = 1;
//  	}/*end if*/
//
//	for (i = 0; i < MAX_SD_NUM; U8_INC(i))
//	{
//		if(m_KeyMaps[i].bSDId == sdId)
//			break;
//	}
//
//	nvmWriteU8Atomic(MEMtPN8(&m_KeyMaps[i].bSDId), 0xFF);
//
//	if(transactionStarted)
//	{
//       	commit_transaction();
//  	}/*end if*/
//#endif
//
//	pushShort(SW_NO_ERROR);
//	return;
//}


u32 getInfo(u8 sdId, u8 type,u16 cgmRam , u32 cgmNvm)
{
	
	u32 	result = 0;
	memref  			privArrayAddr;
	
	u8 		i;
	u8 		tempsdId1;
	u8 		tempsdId2;
	u16 	bFreeEntryIndex;
	memref	theAppTable;
	u16 	bPrivRef;
	memref  sdEntryAddr;
	u16 	currentSdId;
	u16 	ancestorAM_sdId;
	u16	 	sdId_temp;
	u16 	objRef;
//	u16     ancestor_sdId1;
//	u8      flag;

#if !GP__VERSION__FOR_MINI_MASK

		u8 					coding;
		u16 				C9Value;
//		KeyMap  			* theSDMap;
//		KeyMap  			* m_KeyMaps;
		PN8  				sdDataObj;

		coding 				= 0;
		ancestorAM_sdId 	= 0;
//		theSDMap 			= MEM_NULL;
/*@ -discast @*/
//		m_KeyMaps 			= (KeyMap *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
/*@ +discast @*/
//		keySet 				= REF_NULL;
//		for (i = 0; i < MAX_SD_NUM; U8_INC(i))
//		{
//			if (m_KeyMaps[i].bSDId == sdId)
//			{/*@ -discast @*/
//				theSDMap = (KeyMap  *)&m_KeyMaps[i];
//				break;
//			}/*@ +discast @*/
//
//		}
//
//		if (theSDMap == MEM_NULL)
//		{/*@ -discast @*/
//			return (u32)(-1);
//		}/*@ +discast @*/
#endif
//		flag = 0;
		privArrayAddr = 0;

		switch (type)
		{
#if !GP__VERSION__FOR_MINI_MASK
		case SD_TYPE:
//			result = U8t32(theSDMap->bSDType);
			break;
		case IS_ALLOWED_EXTRADITON:
	#if GP__AMENDMENT__D
		if(JcvmConfig_GetCmScpTypeIsSCP03)	//modify by jiangqingbin@20211125,修改支持的协议的配置方案
		{}
		else
	#endif	
			{
//				if(CONFIG_VGP)
//				{
//					if( (U8_AND_U8(theSDMap -> bPriv[0], 0xC0) == 0xC0)
//						|| (U8_AND_U8(theSDMap -> bPriv[0], 0x80) == 0x80) )
//					{
//						result = U8t32(manageAppFlag(sdId,CHECK_EXTRA_FLG));
//					}
//				}
			}
			/*if (theSDMap->bSDTypeExtern & SD_ALLOW_APPLET_EXTRADITION)
			{
				result = true;
			}*/
			break;
		case SD_DAP_KEY_VERSION:
//			result = U8t32(theSDMap->bDapKeyVersion);
			//判断dap key是否存在	
//			sdDataObj 	= MEMtPN8(sdData_getsdDataObjByAppId(sdId));
//			keySet 		= keySet_getKeySet(sdDataObj, 0x73);
//			if(keySet == REF_NULL)
//				result = 0xFF;
//根据1月4评审意见修改魏平姣@20230105
			sdDataObj 		= MEMtPN8(sdData_getsdDataObjByAppId(sdId));
			if(keySet_getKeySet(sdDataObj, DAP_KEY_VERSION) == REF_NULL)
			{
				result = 0;
			}
			else
			{
				result = U8t32(DAP_KEY_VERSION);
			}
			break;
//		case SD_SCP:
//			//result = htoj16(theSDMap->sProtocol);
//			//PXB_W_U16(result, theSDMap->sProtocol);
//			result = PXB_R_U16(&(theSDMap->sProtocol));
//			break;
//		case SD_MAX_AUTH_ATTEMPT:
//			result = U8t32(theSDMap->bMaxAuthAttempt);
//			break;
//		case SD_FREE_AUTH_ATTEMPT:
//			result = U8t32(U8_S_U8(theSDMap->bMaxAuthAttempt, theSDMap->bTryAuthAttempt));
//			break;

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
			case SD_TYPE_NOT_ALLOW_DELETE:
	#if GP__AMENDMENT__D
		if(JcvmConfig_GetCmScpTypeIsSCP03)		//modify by jiangqingbin@20211125,修改支持的协议的配置方案
		{}
		else
	#endif

			if(CONFIG_VGP && (U8_AND_U8(theSDMap -> bPriv[0], 0x40) == 0x40))//modify by xuxiaojun@20220621 去掉全局变量isVGPconfig，直接使用CONFIG_VGP
			{
				result = true;
			}

			if (U8_AND_U8(theSDMap->bSDType, SD_NOT_ALLOW_DELETE))
			{
				result = true;
			}
			break;

		case SD_TYPE_NOT_ALLOW_LOCK_FROM_ISD:
			if (theSDMap->bSDType & SD_NOT_ALLOW_LOCK_FROM_ISD)
			{
				result = true;
			}
			break;

		case SD_TYPE_RECEIPT_GENERATION_FOR_SSD:
			if ( (theSDMap->bSDType & SD_RECEIPT_GENERATION) == 1 )
			{
				result = true;
			}
			break;
	#endif

		case SD_ALLOW_PACKAGE_RECEIVE_EXTRADITION_FROM_SD:
		case SD_ALLOW_APPLET_RECEIVE_EXTRADITION_FROM_SD:
		case SD_ALLOW_PACKAGE_EXTRADITION_FROM_SD:
		case SD_ALLOW_APPLET_EXTRADITION_FROM_SD:
		case SD_ALLOW_DELETION_FORM_SD:

			sdEntryAddr = getEntryByAppId(sdId);
			objRef = PX_R_U16(get_object_field_ref(sdEntryAddr, theApplet_INDEX));
			objRef 	= sdData_getSDdataRef(objRef);
			sdDataObj 		= MEMtPN8(locateObjectInMemory(objRef));
			switch (type)
			{
				case SD_ALLOW_PACKAGE_RECEIVE_EXTRADITION_FROM_SD:
					C9Value = sdData_getTag82Value(sdDataObj);
					coding = U16t8(U16_AND_U16(C9Value, 0x00FF));
					break;
				case SD_ALLOW_APPLET_RECEIVE_EXTRADITION_FROM_SD:
					C9Value = sdData_getTag82Value(sdDataObj);
					coding = U16t8(U16_AND_U16(U16_SHR_U8(C9Value, 8), 0x00FF));
					break; 
				case SD_ALLOW_PACKAGE_EXTRADITION_FROM_SD:
					C9Value = sdData_getTag87Value(sdDataObj);
					coding = U16t8(U16_AND_U16(C9Value, 0x00FF));
					break;
				case SD_ALLOW_APPLET_EXTRADITION_FROM_SD:
					C9Value = sdData_getTag87Value(sdDataObj);
					coding = U16t8(U16_AND_U16(U16_SHR_U8(C9Value, 8), 0x00FF));
					break;
				case SD_ALLOW_DELETION_FORM_SD:
					coding = sdData_getTag83Value(sdDataObj);
					break;
				default:
					return 0;
			}	
			currentSdId = cgmRam;
			//sdEntryAddr = getSdEntryObj(sdId);
			if(bRamBuffer[PROFILE_APDU_MODE] == TRUE) // add xuxiaojun@20240118 北京ifree反馈profile中SSD希望自迁移但是C9参数中82 为0
			{
				coding = 0xF8; // 相当于下载profile过程中使用时不做限制，profile激活之后coding才被启用
			}
			if(U8_AND_U8(coding, 0xF8) == 0)
			{
				return  0;
			}
			else
			{
				u8 sdId_temp1 = 0,ancestor_sdId1 =0,ancestor_sdId2 =0;

				if((currentSdId == ISD_APPID) && (U8_AND_U8(coding, SD_IS_ISD_MASK) == SD_IS_ISD))
				{
					return 1;
				}

				sdEntryAddr = getEntryByAppId(currentSdId);
				objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));
				privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));

				/*if((coding & SD_IS_AMSD_MASK) == SD_IS_AMSD)
				{
					if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40)
					{
						return 1;
					}
				}

				if ((U8_AND_U8(coding, SD_IS_DMSD_UNDER_AMSD_MASK) == SD_IS_DMSD_UNDER_AMSD))
				{

					if (U8_AND_U8(memReadU8(MEMtPM8(privArrayAddr)), 0xA0) == 0xA0)
					{
						flag = 1;
					}
					else
					{
						return 0;
					}
				}

				sdId_temp = U8t16(sdId);

				do{
					//ancestor_sdId1 = getAppletFiled(getIndexByID(U16t8(sdId_temp)), GET_SDID);

					//if (ancestor_sdId1 == currentSdId)
					{
						ancestor_sdId1 = getAppletFiled(getIndexByID(U16t8(sdId_temp)), GET_SDID);
						sdEntryAddr = getSdEntryObj(ancestor_sdId1);
						objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));
						privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));

						if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40)
						{
							if ((U8_AND_U8(coding, SD_HAS_AMSD_ANCESTOR_MASK) == SD_HAS_AMSD_ANCESTOR))
							{
								return 1;
							}
							else if ((U8_AND_U8(coding, SD_HAS_AMSD_HIERARCHY_MASK) == SD_HAS_AMSD_HIERARCHY))
							{
								return 1;
							}
							else if (flag == 1)
							{
								return 1;
							}
						}
					}

					if ((U8_AND_U8(coding, SD_HAS_AMSD_HIERARCHY_MASK) != SD_HAS_AMSD_HIERARCHY))
					{
						break;
					}

					if ( (sdId_temp == ancestor_sdId1) || (ancestor_sdId1 == 0x00) )
					{
						break;
					}
					sdId_temp = ancestor_sdId1;

				}while(1);*/
			if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40)
			{
				if((coding & SD_IS_AMSD_MASK) == SD_IS_AMSD)
				{
					return 1;
				}
				sdId_temp1 = sdId;
				do
				{
					ancestor_sdId1 = getAppletFiled((jbyte)getIndexByID(sdId_temp1), GET_SDID);
					if((ancestor_sdId1 == currentSdId) && ((coding & SD_HAS_AMSD_ANCESTOR_MASK) == SD_HAS_AMSD_ANCESTOR))
					{
						return 1;
					}
					if((ancestor_sdId1 == sdId_temp1) || (ancestor_sdId1 == 0) )
					{
						break;
					}else
					{
						sdId_temp1 = ancestor_sdId1;
					}
				}while(1);
				sdId_temp = currentSdId;
				do
				{
					ancestor_sdId2 = getAppletFiled((jbyte)getIndexByID(sdId_temp), GET_SDID);
					if((ancestor_sdId2 == sdId_temp) || (ancestor_sdId2 == 0) )
					{
						break;
					}else
					{
						sdId_temp = ancestor_sdId2;
					}
				}while(1);

				if((ancestor_sdId2 == ancestor_sdId1) && ((coding & SD_HAS_AMSD_HIERARCHY_MASK) == SD_HAS_AMSD_HIERARCHY))
				{
					return 1;
				}
			 }else if(U8_AND_U8(memReadU8(MEMtPM8(privArrayAddr)), 0xA0) == 0xA0)
			 {
				 if((coding & SD_IS_DMSD_UNDER_AMSD_MASK) != SD_IS_DMSD_UNDER_AMSD)
				{
					return 0;
				}
				else
				{
					sdId_temp = currentSdId;
					do
					{
						ancestor_sdId2 = getAppletFiled((jbyte)getIndexByID(sdId_temp), GET_SDID);
						if((ancestor_sdId2 == sdId_temp) || (ancestor_sdId2 == 0) )
						{
							break;
						}else
						{
							sdId_temp = ancestor_sdId2;
						}
					}while(1);

					sdId_temp1 = sdId;
					do
					{
						ancestor_sdId1 = getAppletFiled((jbyte)getIndexByID(sdId_temp1), GET_SDID);
						if(ancestor_sdId1 == ancestor_sdId2)
						{
							return 1;
						}
						if((ancestor_sdId1 == sdId_temp1) || (ancestor_sdId1 == 0) )
						{
							break;
						}else
						{
							sdId_temp1 = ancestor_sdId1;
						}
					}while(1);
				}

			 }
				return 0;
			}


//			currentSdId = cgmRam;
//			sdEntryAddr = getSdEntryObj(sdId);
//
//			if((currentSdId == 0) && (U8_AND_U8(coding, SD_IS_ISD_MASK) == SD_IS_ISD))
//			{
//				return 1;
//			}
//
//
//			sdId_temp = U8t16(sdId);
//
//			do{
//				ancestor_sdId1 = getAppletFiled(getIndexByID(U16t8(sdId_temp)), GET_SDID);
//
//				if (ancestor_sdId1 == currentSdId)
//				{
//					ancestor_sdId1 = getAppletFiled(getIndexByID(U16t8(currentSdId)), GET_SDID);
//					sdEntryAddr = getSdEntryObj(ancestor_sdId1);
//					objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));
//					privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));
//
//					if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40)
//					{
//						if((coding & SD_IS_AMSD_MASK) == SD_IS_AMSD)
//						{
//							return 1;
//						}
//						else if ((U8_AND_U8(coding, SD_HAS_AMSD_ANCESTOR_MASK) == SD_HAS_AMSD_ANCESTOR))
//						{
//							return 1;
//						}
//					}
//					else if (U8_AND_U8(memReadU8(MEMtPM8(privArrayAddr)), 0xA0) == 0xA0)
//					{
//						if ((U8_AND_U8(coding, SD_IS_DMSD_UNDER_AMSD_MASK) == SD_IS_DMSD_UNDER_AMSD))
//						{
//							ancestor_sdId1 = getAppletFiled(getIndexByID(U16t8(ancestor_sdId1)), GET_SDID);
//							sdEntryAddr = getSdEntryObj(ancestor_sdId1);
//							objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));
//							privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));
//
//							if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40)
//							{
//								return 1;
//							}
//						}
//					}
//				}
//
//				if ((U8_AND_U8(coding, SD_HAS_AMSD_HIERARCHY_MASK) == SD_HAS_AMSD_HIERARCHY))
//				{
//					sdEntryAddr = getSdEntryObj(ancestor_sdId1);
//					objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));
//					privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));
//
//					if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40)
//					{
//						return 1;
//					}
//				}
//
//				if ( (sdId_temp == ancestor_sdId1) || (ancestor_sdId1 == 0x00) )
//				{
//					break;
//				}
//
//				sdId_temp = ancestor_sdId1;
//			}while(1);
//
//			return 0;
#endif

#if GP__TOKEN__FOR_MASK
		case SD_SEARCH_UNDER_AMSD_TOKENVERIFICATION_SD:
#endif
#if GP__RECEIPT__FOR_MASK
		case SD_SEARCH_UNDER_AMSD_RECEIPTGENERATION_SD:
#endif

#if (GP__TOKEN__FOR_MASK || GP__RECEIPT__FOR_MASK)
			ancestorAM_sdId = APP_NULL;
			privArrayAddr = MEM_NULL;
			currentSdId = U8t16(sdId);
			currentSdId = getAppletFiled(getIndexByID(U16t8(currentSdId)), GET_SDID);
			//for (i = 0; i < MAX_SD_NUM; U8_INC(i))

			do{
//				u8 sdId_temp1;
/*@ -discast@*/
//			theSDMap = (KeyMap  *)&m_KeyMaps[i];
/*@ +discast@*/
				//sdId_temp1 = theSDMap->bSDId;
				//if (sdId_temp1 == currentSdId)
				{

					sdEntryAddr = getSdEntryObj(currentSdId);

					objRef = PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));

					if (objRef != MEM_NULL) //这种情况会出现？如果出现的话，该语句后面读取privArrayAddr数据的语句也应该放在该判断内部
					{
						privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(objRef)));
					}

					if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x40) == 0x40) // Authorized Management
					{
						if(SD_SEARCH_UNDER_AMSD_TOKENVERIFICATION_SD == type)
						{
							if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x20) == 0x20) // Token Management
							{
								return currentSdId;
							}
						}else
						{
							if(U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,2))), 0x80) == 0x80) // Receipt Generation
							{
								return currentSdId;
							}
						}
						ancestorAM_sdId = currentSdId;
						break;
					}
					else
					{
						sdId_temp = getAppletFiled(getIndexByID(U16t8(currentSdId)), GET_SDID);
						if(sdId_temp == currentSdId)
						{
							return NO_TOKENORRECEIPT_SD;
						}
						else
						{
							//i = -1;
							currentSdId = sdId_temp;
							continue;
						}
					}
				}
			} while(1);

//			if(ancestorAM_sdId != 0) //MNOSD不一定具有收条权限
			{
				bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
				theAppTable 	= getAppTable();

				for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
				{
					sdEntryAddr = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(i, SIZE_REF))));

					if (sdEntryAddr == MEM_NULL) {
						break;
					}

					tempsdId2	= memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(sdEntryAddr), bsdId_INDEX)));

					bPrivRef		= PN_R_U16(get_object_field_ref(sdEntryAddr, bPriv_INDEX));

					if(bPrivRef == MEM_NULL)
					{
						continue;//GP_UICC_Compliance_SCP81/SECURED/CardContentManagement_SECURED/ccm_APDU_delete_7a_22_53_rsaToken
					}

					privArrayAddr = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));

					if( ((SD_SEARCH_UNDER_AMSD_TOKENVERIFICATION_SD == type) && (U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,1))), 0x20) == 0x20))
						|| (U8_AND_U8(memReadU8(MEMtPM8(U32_A_U8(privArrayAddr,2))), 0x80) == 0x80) )
					{
						//if(U8_AND_U8(theSDMap->bPriv[1], 0x20) == 0x20)
						{
							do
							{
								tempsdId1 = getAppletFiled(getIndexByID(tempsdId2), GET_SDID);
								if(tempsdId1 == ancestorAM_sdId)
								{
									return tempsdId2;
								}
								if((tempsdId1 == tempsdId2) || (tempsdId1 == 0))
								{
									break;
								}else
								{
									tempsdId2 = tempsdId1;
								}

							}while(1);
						}
					}
				}
				return NO_TOKENORRECEIPT_SD;
			}
		
			break;
#endif
		default:
/*@ -discast @*/			result = (u32)(-1); /*@ +discast @*/
			break;
		}

		
	return result;
}

/**
 * modify by xuxiaojun@20220427 根据分层代码 第一次代码评审(20220415).xlsx 将GetFileSystemTotalSize()宏定义为0
 */
void getSDInfo(void)
{
	u8 sdId, type;
	PN8 				sdEntryAddr;
	u8 					cgmFlag;
	u8 				sdId_temp;
	u32 cgmNvm = U16t32(popInt());
	u16 cgmRam = popShort();
	type = U16t8(popShort());
	sdId = U16t8(popShort());
	/*@ -typedefparam @*/
	/*@ -discast @*/
	/*@ -type @*/	
#if GP__CGM__FOR_MASK
	if(type < SD_MANAGE_RAM_SIZE)
	{
		pushInt((jint)getInfo(sdId, type,cgmRam,cgmNvm));
	}
	else
	{		
		if(type == SD_MANAGE_NVM_SIZE)
		{
			getAppTable();
			sdEntryAddr 		= MEMtPN8(getSdEntryObj(sdId));
			cgmFlag 			= CgmField_getCGMFlag(sdEntryAddr);
			while(!U8_AND_U8(cgmFlag, HAS83_MASK))
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
			if(sdId == 0)
			{
				pushInt((jint) U32_S_U32(CgmField_getSpaceInfo(sdId, type), GetFileSystemTotalSize()));
			}
			else
			{
				pushInt((jint)CgmField_getSpaceInfo(sdId, type));
			}
		}
		else
		{
			pushInt((jint)CgmField_getSpaceInfo(sdId, type));
		}


		
	}
#else
	pushInt((jint)getInfo(sdId, type,cgmRam,cgmNvm));
#endif
	/*@ +type @*/
	/*@ +discast @*/
	/*@ +typedefparam @*/
}

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
u16 getAppInfoCMS2AC(u8 sdId, u8 * buf, u16 * dataLen)
{	
	u16 offset; //len;
	u16 ramSize, sRam;
	u32 nvmSize;
	jref pkgEntry; //AppEntry;
	u8 pkgId;
	
	u16 bFreeEntryIndex;
	u16	i, j,
	u8  assoSDid, AidLen, appId, context, lenOff, appPagContext, curAppID;
	u8 bPriv[PRIVMAXBYTES];
	u8 sdContext = NULL_CONTEXT;
	u8 flag = 0;
	
	memref EntryAddr, AidAdd, SDAidAdd = 0, entry, theAppTable, assoSDAidAdd;
	u32 iNvm;
	KeyMap  * theSDMap = 0;
	KeyMap  * m_KeyMaps = (KeyMap  *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
	memref packageContextTable;
	u16 bPrivRef;
	u32 bPrivArray;
	u8  n = 1;    //如果是Getdata的第一条命令，则在第一条响应数据中返回后续数据的总长度，所以需要一次搜索全部符合要求的数据
	//u8 sdId = popShort();
	PN8				sdDataObj;
	
	sdDataObj = getCurrentSdObj(IS_NO_NEED_CHECK_SD);
	if (!moreDataFlag[selected_channel])
	{
		g_ExtraPkgOff[selected_channel] = FIRST_LOAD_PACKAGE;
		g_ExtraAppOff[selected_channel] = APP_FIRST;

		g_PkgOff[selected_channel] = FIRST_LOAD_PACKAGE - 3; //20110812 lzm for 6.1.2.8.11 (get data)
		//g_PkgOff = 0;
		g_AppOff[selected_channel] = APP_FIRST;
		
		g_SdOff[selected_channel] = APP_FIRST;
		n = 2;
	}

	for (i = 0; i < MAX_SD_NUM; i++)
	{
		if (m_KeyMaps[i].bSDId == sdId)
		{
			theSDMap = (KeyMap  *)&m_KeyMaps[i];
			break;
		}
	}

	bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
    theAppTable = getAppTable();
    
   for (i = APP_FIRST; i < bFreeEntryIndex; i++)
	{
		entry = locateObjectInMemory(PN_R_U16((memref)(theAppTable + i * SIZE_REF)));
		appId = PX_R_U8((u8 *)get_object_field_ref(entry, bAppID_INDEX));
		if (appId != sdId)
		{
			continue;
		}
		SDAidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(entry, theAID_INDEX)));
        //SDAidAdd = locateObjectInMemory(PX_R_U16((memref)get_object_field_ref(SDAidAdd, AIDARRAY_INDEX_OF_AID)));
        break;
	}
    
	while( n > 0 )
	{
		offset = 0;

		if(!isGPconfig221)
		{
			if (moreDataFlag[selected_channel] == 0)
			{
				buf[offset++] = 0x82;
				lenOff = offset++;
				//cacheBuffer[offset++] = 0x03;
				buf[offset++] = theSDMap->bSDType;
				buf[offset++] = sdData_getScpType(sdDataObj);
				buf[offset++] = sdData_getScpOption(sdDataObj);
				
				if (theSDMap->bSDType & SD_TYPE_FIX_SIZE)
				{
					//获取安全域管理的剩余空间
					sRam = CgmField_getSpaceInfo(sdId, SD_CGM_REMAIN_RAM_SIZE);
					iNvm = CgmField_getSpaceInfo(sdId, SD_CGM_REMAIN_NVM_SIZE);
					buf[offset++] = sRam >> 8;
					buf[offset++] = sRam & 0xFF;
					buf[offset++] = iNvm >> 24;
					buf[offset++] = iNvm >> 16;
					buf[offset++] = iNvm >> 8;
					buf[offset++] = iNvm & 0xFF;

					buf[lenOff] = 0x09;
				}
				else
				{
					buf[lenOff] = 0x03;
				}
			}
		}

		//sd info
		for (j = g_SdOff[selected_channel]; j < bFreeEntryIndex; j++)
		{
			entry = locateObjectInMemory(PN_R_U16((memref)(theAppTable + j * SIZE_REF)));
			bPrivRef = PX_R_U16((u8 *)get_object_field_ref(entry, bPriv_INDEX));
			bPrivArray = (u32)get_array_data((u8 *)locateObjectInMemory(bPrivRef));
			memCopyToRam((memref)bPriv, bPrivArray, PRIVMAXBYTES);
			assoSDid = PX_R_U8((u8 *)get_object_field_ref(entry, bsdId_INDEX));
			curAppID = PX_R_U8((u8 *)get_object_field_ref(entry, bAppID_INDEX));
			
			if ((bPriv[0]&0x80) == 0x80 && assoSDid == sdId && (curAppID != sdId))
			{
				if (sdContext == NULL_CONTEXT)
				{
					sdContext = PX_R_U8((u8 *)get_object_field_ref(entry, theContext_INDEX));
				}
				
				AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(entry, theAID_INDEX)));
				AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(AidAdd, AIDARRAY_INDEX_OF_AID)));
				AidLen = (u8)get_array_length(AidAdd);
				AidAdd = get_array_data((u8 *)AidAdd);
				
				if (n != 2)
				{
					if((offset + AidLen + 7) > (0xFF - 2 - 3))   //if new offset bigger than 255 - tag(2Bytes) - tagLen(3Bytes)
					{
						g_SdOff[selected_channel] = j;
						moreDataFlag[selected_channel] = 1;

						return (offset | 0x8000);
					}
					else
					{
						moreDataFlag[selected_channel] = 0;
					}
					//获取SD自身占用空间
					ramSize = handlRoomOfGetAppRam(entry);
					nvmSize = handlRoomOfGetAppNvm(entry);
					
					if(isGPconfig221)
					{
						buf[offset++] = 0x61;
						buf[offset++] = AidLen + 2;
						buf[offset++] = 0x4F;	
						buf[offset++] = AidLen;
						ecpMem_Copy((u8*)&buf[offset], (u8*)AidAdd, AidLen);
						offset += AidLen;
						//buf[offset++] = ramSize >> 8;
						//buf[offset++] = ramSize & 0xFF;
						//buf[offset++] = (u8)(nvmSize >> 8);
						//buf[offset++] = (u8)(nvmSize & 0xFF);		
					}
					else
					{
						buf[offset++] = 0x62;
						buf[offset++] = AidLen + 5;
						buf[offset++] = AidLen;
						ecpMem_Copy((u8*)&buf[offset], (u8*)AidAdd, AidLen);
						offset += AidLen;
						buf[offset++] = ramSize >> 8;
						buf[offset++] = ramSize & 0xFF;
						buf[offset++] = (u8)(nvmSize >> 8);
						buf[offset++] = (u8)(nvmSize & 0xFF);		
					}
				}
				else
				{
					offset += (3 + AidLen + 4);
				}
			}
		}
		
		if (n != 2)
		{
			g_SdOff[selected_channel] = j;
		}
		
		//sd package
		if ((!moreDataFlag[selected_channel] ) && (sdId == 0) && (sdContext != (u8)NULL_CONTEXT))
		{
			packageContextTable = locateObjectInMemory(PN_R_U16(G_CurExecuEnvironment.packageContextTable));
			packageContextTable = get_array_data((u8 *)packageContextTable);
			
			pkgId = PX_R_U8((u8 *)(packageContextTable + sdContext));
			pkgEntry = getPkgEntry(pkgId);
			
			if(pkgEntry != REF_NULL)
			{
				EntryAddr = locateObjectInMemory(pkgEntry);
				assoSDid = PX_R_U8((u8 *)get_object_field_ref(EntryAddr, sdId_INDEX));

				if(assoSDid == sdId)
				{
					AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(EntryAddr, pkgAID_INDEX)));
					AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(AidAdd, AIDARRAY_INDEX_OF_AID)));
					AidLen = (u8)get_array_length(AidAdd);
					AidAdd = get_array_data((u8 *)AidAdd);
			
					buf[offset++] = 0x60;
					buf[offset++] = AidLen + 7;
					buf[offset++] = AidLen;
					
					ecpMem_Copy((u8*)&buf[offset], (u8*)AidAdd, AidLen);
					offset += AidLen;
					//获取包大小
					//getPagSize(i, (u8*)(buf + offset));
					getLoadFileInfo(pkgId, (u8*)(buf + offset));
					offset += 6;
				}
			}
		}
		
		
		for (i = g_ExtraPkgOff[selected_channel]; i < ON_CARD_PKG_MAX; i++)
		{
			pkgEntry = getPkgEntry(i);
			if (pkgEntry != REF_NULL)
			{
				EntryAddr = locateObjectInMemory(pkgEntry);
				assoSDid = PX_R_U8((u8 *)get_object_field_ref(EntryAddr, sdId_INDEX));

				if (assoSDid != sdId)  //extradited app
				{
					appPagContext = (u8)getPkgContext(i);

					for (j = g_ExtraAppOff[selected_channel]; j < bFreeEntryIndex; j++)
					{
						entry = locateObjectInMemory(PN_R_U16((memref)(theAppTable + j * SIZE_REF)));
						context = PX_R_U8((u8 *)get_object_field_ref(entry, theContext_INDEX));
						
						assoSDAidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(entry, sdAID_INDEX)));
						//assoSDAidAdd = locateObjectInMemory(PX_R_U16((memref)get_object_field_ref(assoSDAidAdd, AIDARRAY_INDEX_OF_AID)));

						if ((context == appPagContext) && (assoSDAidAdd == SDAidAdd))
						{
							AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(entry, theAID_INDEX)));
							AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(AidAdd, AIDARRAY_INDEX_OF_AID)));
							AidLen = (u8)get_array_length(AidAdd);

							if (n != 2)
							{
								if ((offset + AidLen + 7) > (0xFF - 2 - 3))   //if new offset bigger than 255 - tag(2Bytes) - tagLen(3Bytes)
								{
									g_ExtraPkgOff[selected_channel] = i;
									g_ExtraAppOff[selected_channel] = j;
									moreDataFlag[selected_channel] = 1;

									return (offset | 0x8000);
								}
								else
								{
									moreDataFlag[selected_channel] = 0;
								}
								
								AidAdd = get_array_data((u8 *)AidAdd);
								//获取APP占用空间
								ramSize = handlRoomOfGetAppRam(entry);
								nvmSize = handlRoomOfGetAppNvm(entry);

								
								buf[offset++] = 0x61;
								buf[offset++] = AidLen + 5;
								buf[offset++] = AidLen;
								//memCopyToRam((u8*)(cacheBuffer+offset), (u8*)AidAdd, AidLen);
								ecpMem_Copy(&buf[offset], (u8 *)AidAdd, AidLen);
								offset += AidLen;
								buf[offset++] = ramSize >> 8;
								buf[offset++] = ramSize & 0xFF;
								buf[offset++] = (u8)(nvmSize >> 8);
								buf[offset++] = (u8)(nvmSize & 0xFF);
							}
							else
							{
								offset += (3 + AidLen + 4);
							}
						}
					}
				}
			}
		}
		
		if (n != 2)
		{
			g_ExtraPkgOff[selected_channel] = i;
			g_ExtraAppOff[selected_channel] = j;
		}
		
		if (n != 2)
		{
			if ((g_PkgOff[selected_channel] & 0x80) == 0x80)
			{
				flag = 1;
				g_PkgOff[selected_channel] &= 0x7F;
			}
		}
		
		for ( i = g_PkgOff[selected_channel]; i < ON_CARD_PKG_MAX; i++)
		{
			if (n != 2)
			{
				g_PkgOff[selected_channel] = i;
			}

			pkgEntry = getPkgEntry(i);
			if(pkgEntry != REF_NULL)
			{
				EntryAddr = locateObjectInMemory(pkgEntry);
				assoSDid = PX_R_U8((u8 *)get_object_field_ref(EntryAddr, sdId_INDEX));

				if(assoSDid == sdId)
				{
					AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(EntryAddr, pkgAID_INDEX)));
					AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(AidAdd, AIDARRAY_INDEX_OF_AID)));
					AidLen = (u8)get_array_length(AidAdd);
					AidAdd = get_array_data((u8 *)AidAdd);
					
					if (n != 2)
					{
						//如果上一次已经返回了该数据，则不需要再返回
						if (flag == 0)
						{
							if((offset + AidLen + 9) > (0xFF - 2 - 3))   //if new offset bigger than 255 - tag(2Bytes) - tagLen(3Bytes)
							{
								g_PkgOff[selected_channel] = i;
								moreDataFlag[selected_channel] = 1;

								return (offset | 0x8000);
							}
							else
							{
								moreDataFlag[selected_channel] = 0;
							}
							
							buf[offset++] = 0x60;
							buf[offset++] = AidLen + 7;
							buf[offset++] = AidLen;
							//memCopyToRam((u8*)(cacheBuffer+offset), (u8*)AidAdd, AidLen);
							ecpMem_Copy((u8*)&buf[offset], (u8*)AidAdd, AidLen);
							offset += AidLen;
							//获取包大小
							//getPagSize(i, (u8*)(buf + offset));
							getLoadFileInfo(i,(u8*)(buf + offset));
							offset += 6;
						}
						flag = 0;
					}
					else
					{
						offset += (3 + AidLen + 6);
					}
					
					appPagContext = (u8)getPkgContext(i);
				
					for (j = g_AppOff[selected_channel]; j < bFreeEntryIndex; j++)
					{
						entry = locateObjectInMemory(PN_R_U16((memref)(theAppTable + j * SIZE_REF)));
						context = PX_R_U8((u8 *)get_object_field_ref(entry, theContext_INDEX));
						
						assoSDAidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(entry, sdAID_INDEX)));
						//assoSDAidAdd = locateObjectInMemory(PX_R_U16((memref)get_object_field_ref(assoSDAidAdd, AIDARRAY_INDEX_OF_AID)));

						if((context == appPagContext) && (assoSDAidAdd == SDAidAdd))
						{
							AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(entry, theAID_INDEX)));
							AidAdd = locateObjectInMemory(PX_R_U16((u8 *)get_object_field_ref(AidAdd, AIDARRAY_INDEX_OF_AID)));
							AidLen = (u8)get_array_length(AidAdd);
							AidAdd = get_array_data((u8 *)AidAdd);
							
							if (n != 2)
							{
								if((offset + AidLen + 7) > (0xFF - 2 - 3))   //if new offset bigger than 255 - tag(2Bytes) - tagLen(3Bytes)
								{
									//由于当前的包已经放到buffer中，所以指针往后挪, 下一次搜索时，不需要返回该包的信息
									g_PkgOff[selected_channel] = i | 0x80;
									g_AppOff[selected_channel] = j;
									moreDataFlag[selected_channel] = 1;

									return (offset | 0x8000);
								}
								else
								{
									moreDataFlag[selected_channel] = 0;
								}
								//获取自身占用空间
								//ramSize = PX_R_U16((u8 *)get_object_field_ref(entry, sMaxRAM_INDEX));
								//nvmSize = OBJ_read_u32((u8 *)get_object_field_ref(entry, sMaxNVM_INDEX));
								ramSize = handlRoomOfGetAppRam(entry);
								nvmSize = handlRoomOfGetAppNvm(entry);

								buf[offset++] = 0x61;
								buf[offset++] = AidLen + 5;
								buf[offset++] = AidLen;
								//memCopyToRam((u8*)(cacheBuffer+offset), (u8*)AidAdd, AidLen);
								ecpMem_Copy((u8*)&buf[offset], (u8*)AidAdd, AidLen);
								offset += AidLen;
								buf[offset++] = ramSize >> 8;
								buf[offset++] = ramSize & 0xFF;
								buf[offset++] = (u8)(nvmSize >> 8);
								buf[offset++] = (u8)(nvmSize & 0xFF);
							}
							else
							{
								offset += (3 + AidLen + 4);
							}
						}
					}
				}
			}
		}
		
		if (n != 2)
		{
			g_PkgOff[selected_channel] = i;
		}
		
		if (n == 2)
		{
			(*dataLen) += offset;
		}
		
		n--;
	}
	
	return offset;
}
#else
u16 getAppInfo(u8 sdId, u8 * buf, u16 * dataLen)
{	
#if !GP__VERSION__FOR_MINI_MASK
	u16 offset; 
	u8 						PRI;
	u8 						assoSDid;
	u8						AidLen;
	u8						appId;
	u8	 					bPriv[PRIVMAXBYTES];
	u16 					bFreeEntryIndex;
	u16						i;
	u16						j;
	u16 					ref;
	u16 					bPrivRef;
	u32 					bPrivArray;
	memref 					AidAdd;
	memref					entry;
	memref					theAppTable;
//	KeyMap  				* m_KeyMaps;

	PRI						=	0;
	/*@ -discast @*/
//	m_KeyMaps 				= (KeyMap  *)(MEMtPX8(JcvmConfig_GetVmConfigAddr(KEY_MAPS)));
	/*@ +discast @*/
	if (!moreDataFlag[selected_channel])
	{
		g_AppOff[selected_channel] = APP_FIRST;
	}

//	for (i = 0; i < MAX_SD_NUM; U16_INC(i))
//	{
//		if (m_KeyMaps[i].bSDId == sdId)
//		{
//			break;
//		}
//	}
	/*@ -disspecopt @*/
	bFreeEntryIndex = nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
    theAppTable = getAppTable();
    /*@ +disspecopt @*/
    for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
	{
		entry = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(i, SIZE_REF))));
		appId = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX)));
		if (appId != sdId)
		{
			continue;
		}
		bPrivRef 	= memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bPriv_INDEX)));
		bPrivArray  = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
		memCopyToRam(MEMtPX8(bPriv), MEMtPM8(bPrivArray), PRIVMAXBYTES);
		PRI 		= bPriv[1];
        break;
	}
    
	offset = 0;

	//sd info
	for (j = U8t16(g_AppOff[selected_channel]); j < bFreeEntryIndex; U16_INC(j))
    {
		entry = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable, U16_M_U16(j, SIZE_REF))));
		appId = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX)));
		if(appId == APP_NULL) // except LTE system app to avoid to send too much data
		{
			continue;
		}
#if JAVA_CARD_UICC
		if(appId == UICC_APPID) // except LTE system app to avoid to send too much data
		{
			continue;
		}
#endif

#if((EUICC_ONLY) || (SE_AND_EUICC))
		{
			if((appId == ISDR_APPID) || (appId == ECASD_APPID))
			{
				continue;
			}
		}
#endif
		assoSDid = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bsdId_INDEX)));
		bPrivRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bPriv_INDEX)));
		bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
		memCopyToRam(MEMtPX8(bPriv), MEMtPM8(bPrivArray), PRIVMAXBYTES);
		 if(U8_AND_U8(PRI, 0x04) || (assoSDid == sdId))
		{
			ref = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(entry), theAID_INDEX)));
			if(ref == NULL_REF)
				continue;
//			if( bPriv[0]&0x80)
//				continue;
			AidAdd = locateObjectInMemory(ref);
        	AidAdd = locateObjectInMemory(memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(AidAdd), AIDARRAY_INDEX_OF_AID))));
        	AidLen = U16t8(get_array_length(MEMtPF8(AidAdd)));
        	AidAdd = get_array_data(MEMtPF8(AidAdd));

      #if UICC__APP__WHITECARD
        	if(IsWhiteCardApplet(MEMtPX8(AidAdd)) == TRUE)
        	{
        		continue;
        	}
      #endif

      #if UICC__OPERATOR__CHINAMOBILE
			//do not send gba info, for collis test, send response data in one message, ignore gba app info (test:GP304/ds_APDU_getData_12_74_e7)
        	if(IsGBAApplet(MEMtPX8(AidAdd)) == TRUE)
			{
				continue;
			}
      #endif
			
			if(U16_A_U8(offset, U8_A_U8(AidLen, 3)) > U8t16(U8_S_U8(U8_S_U8(0xFF, 2), 3)))   //if new offset bigger than 255 - tag(2Bytes) - tagLen(3Bytes)
			{							
				if (!moreDataFlag[selected_channel])
				{
					g_AppOff[selected_channel] = U16t8(j);
							
					moreDataFlag[selected_channel] = 1;
					dataLen[0] = U16_A_U16(dataLen[0], offset);
				}
				dataLen[0]  = U16_A_U16(dataLen[0], U8t16(U8_A_U8(AidLen, 4)));
				continue;
			}
			else
			{
				moreDataFlag[selected_channel] = 0;
			}

			buf[U16_INC(offset)] = 0x61;
			buf[U16_INC(offset)] = U8_A_U8(AidLen, 2);
			buf[U16_INC(offset)] = 0x4F;
			buf[U16_INC(offset)] = AidLen;
			ecpMem_Copy(MEMtPX8(&buf[offset]), MEMtPX8(AidAdd), AidLen);
			offset = U16_A_U8(offset, AidLen);
		}
	}

	return offset;
#else
	return 0;
#endif
}
#endif

/*
* 取得已注册applet个数
*/
u8 getAppletNum(void)
{
	u8 count;

	count = manageAppFlag(0,GET_APP_COUNT);
	
	return count;
}

memref   getSdDataObjBySdid(u8 bSdid){

		jref			sdRef;
		jref			sdDataRef;
		memref			entry;
		memref			sdDataObj;
	
		entry			=		getEntryByAppId(bSdid);
		sdRef 			= 		nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theApplet_INDEX)));
		sdDataRef 		= 		sdData_getSDdataRef(sdRef);
		sdDataObj 		= 		locateObjectInMemory(sdDataRef);
	
		return 		sdDataObj;
}

/*
*@function   : GPSSDExitCompleteProtolKey
*
*@description: 判断一个ssd支持的所有的协议是否都具有对应的key值；
*				gp2.3-UL测试，当putkey完成后检查ssd的key是否完整，若完整则需要把ssd切换为personal状态	(用例：sm_scp81_script_target_SSD_without_SCP03_16B_keys_19_e9_42)
*
*@param      : u8 sdId, ssd的id
*
*@return     : true表示ssd具有其支持的协议的完整的key值
*
*@history    : V1.0 :created by jiangqingbin@20210615
*
*/
u8 GPSSDExitCompleteProtolKey(u8 sdId)
{
	PN8		sdDataObj;
	u8 			scpArrayLen;
	jref 		scpBufferRef;
	memref		scpBufferFeild;
	memref 		scpBufferAddr;
	u8			tempI;
	u8			tmpScpType;

	sdDataObj = MEMtPN8(getSdDataObjBySdid(sdId));

	scpBufferRef = sdData_getScpBufferRef(sdDataObj);

	if(scpBufferRef == MEM_NULL)//理论上这里是不会成立的
	{
		return FALSE;
	}

	scpBufferAddr = locateObjectInMemory(scpBufferRef);
	scpArrayLen   = U16t8(get_array_length(scpBufferAddr));
	scpBufferFeild = get_array_data(MEMtPF8(scpBufferAddr));
	//scpBufferFeild = sdData_getScpBufferField(sdDataObj);

	//循环查找 SCPBuffer, 检查sd支持的协议的key是否完整
	for(tempI = 0x00; tempI < scpArrayLen ;)
	{
		tmpScpType 	= nvmReadU8(MEMtPN8(scpBufferFeild));

		if((tmpScpType != 0x00)&&(GPSSDExistCompleteSecureKey(sdId,  tmpScpType) == FALSE))
		{
			return FALSE;
		}

		tempI = U16t8(U8_A_U8(tempI, 2));
		scpBufferFeild = U32_A_U8(scpBufferFeild, 2);
	}

	return TRUE;

}

/**
 * modify by https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/xuxiaojun @20210627
 */
u8 iSetSDPersonalizeState(u8 bSdid)
{
#if !GP__VERSION__FOR_MINI_MASK
	u8			bgetEntryPri;
	u16 			bPrivRef;
	memref		entry, bPrivArray;
//	jref 			keyGroupRef;  // remove warning,dengyiwen,2022/9/22
	PN8		sdDataObj;
//	jref 	keySetRef;  // remove warning,dengyiwen,2022/9/22
// 	PN8 	keySetObj;  // remove warning,dengyiwen,2022/9/22
//	u8		scpType;  // remove warning,dengyiwen,2022/9/22

	if(!bSdid)  //ISD
	{
			return false;
	}

	if(bRamBuffer[INSTALL_SD_NOT_AUTO_PERSONALIZED])
	{
		return false;
	}
	
	entry			=	getEntryByAppId(bSdid);
	
	
	sdDataObj	=	MEMtPN8(getSdDataObjBySdid(bSdid));
	bPrivRef	=	memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bPriv_INDEX)));
	bPrivArray 	= 	get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));

	if(U8_AND_U8(PX_R_U8(bPrivArray), DAP_VERIFICATION_PRIVILEGES_TAG) == DAP_VERIFICATION_PRIVILEGES_TAG)
	{
			if(!keySet_getKeySet(sdDataObj, KEY_DAP_VERIF_ID1))
			{
					return false;
			}
	}

	bgetEntryPri	=	PX_R_U8(PN8_A_U8(bPrivArray, 0x02));//第3个权限字节   Receipt 、 Ciphered Load File Data Block

	if( U8_AND_U8(bgetEntryPri, RECEIPT_PRIVILEGES_TAG) == RECEIPT_PRIVILEGES_TAG)
	{
			if(!keySet_getKeySet(sdDataObj, KEY_RECEIPT_ID1))
			{
					return false;
			}
	}

	if( U8_AND_U8(bgetEntryPri, CIPHERED_LOAD_PRIVILEGES_TAG) == CIPHERED_LOAD_PRIVILEGES_TAG)
	{
			if(!keySet_getKeySet(sdDataObj, KEY_CIPHERED_LOAD_ID1))
			{
					return false;
			}
	}

	bgetEntryPri	=	PX_R_U8(PN8_A_U8(bPrivArray, 0x01));
	
	if( U8_AND_U8(bgetEntryPri, TOKEN_PRIVILEGES_TAG) == TOKEN_PRIVILEGES_TAG)
	{
			if(!keySet_getKeySet(sdDataObj, KEY_TOKEN_ID1))
			{
					return false;
			}
	}

#if M2M_EUICC
	if((bSdid == ISDP_APPID)&&(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX))//ISDP - AM  权限 但是关联的是ISDR 
	{

	}
	else
#endif
	{
		if(U8_AND_U8(bgetEntryPri, AUTHORIZED_PRIVILEGES_TAG) == AUTHORIZED_PRIVILEGES_TAG)
		{
			if(APDUBuffer[OFFSET_INS] == INS_OP_INSTALL)
			{
				if(bRamBuffer[INSTALL_APPLET_EXTRADTIONFLAG] != IS_APPLET_EXTRADTION_FLAG ) // only valid for opInstallForExtradition
				{
					return false;
				}
				else
				{
					//GPSSDExitCompleteProtolKey
				}
			}
			else //put key
			{
				if(memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bsdId_INDEX))) == bSdid) //AMSD关联到自己
				{
					//GPSSDExitCompleteProtolKey
				}
				else
				{
					return false;
				}
			}
		}
	 }
	 
	//根据规范gpc2.3,ssd在putkey完成后进行检查其自身支持的协议是否具都有完整的key，如果是则将切换到PERSONALIZED状态	modify by jiangqingbin-20210618
	if(GPSSDExitCompleteProtolKey(bSdid) == FALSE)
	{
		return false;
	}
/*
	if( 0xff != sdData_changeCurrentScpTypeAndOption(sdDataObj, SCP0X_WITHOUT_IOPTION_MODE, 0x00))
	{
		keyGroupRef = sdData_getKeyGroupRef(sdDataObj, SCP_02, FALSE);
		if(REF_NULL != keyGroupRef)
		{
			// check has SCP02 KeySet,is not have return;
			keySetRef = keyGroup_getKeySetRef(MEMtPF8(locateObjectInMemory(keyGroupRef)));
			if (REF_NULL == keySetRef )
			{
				return ;
			}
			keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
			
			while(keySet_getKeyNum(keySetObj) != 0x03)
			{
				keySetRef = keySet_getNextKeySetRef(keySetObj);
				
				if (!keySetRef )
				{
					return ;
				}
				else
				{
					keySetObj = MEMtPN8(locateObjectInMemory(keySetRef));
				}
			}
		}
	 }
*/
	store_object_B_filed(MEMtPF8(entry), bCurrentState_INDEX, SSD_PERSONALIZED);

	return true;
#endif
}

#if !GP__VERSION__FOR_MINI_MASK
/*
 * modified by xuxiaojun@20210414 对于类型确定的数组，_newarrayObject紧接着给数组内容写值时，避免调用get_array_data，可以减少上面一次创建数组时cache的提交
 */
void dealC9Tag(void)
{
//#define 	TAG_LENGTH_OF_81				0x02 代码中未使用
#define 	MAX_SCPTYPE_AND_OPTION_LEN		0x0C // 支持3个81 tag
#define		BYTE_ARRAY_HEADER_SIZE			0x06
#define     MAX_Amend_B_BUFFER_LEN          0x7F //存储附录B的84,85,86,89的单个最大长度
	PM8		X_A		SrcBuf;
	u16 	X_A		boffset;
	u16 	X_A		sLength;
	u8		X_A		tempBuffer[MAX_SCPTYPE_AND_OPTION_LEN];
	u8		X_A		tempBufferlen;
	PX8		X_A		tempPtr;
	
	u8      X_A     bLen;
	u8      X_A     bC9Len;
	u8      X_A     bTag;
	u8		X_A		isSupportSCP0x;
	
	jref	X_A		sdRef;
	PN8		X_A		sdDataObj;

	u8 hasC982 = 0;
	u8 hasC981 = 0;
	u8 hasC983 = 0;
	u8 hasC984 = 0;
	u8 hasC987 = 0;
	u16 bC982Value = 0,bC987Value = 0,bC983Value = 0;
	u8 value1 = 0,value2 = 0;

#if	GP__AMENDMENT__B__FOR_MASK
	PX8		X_A		tempAmendBPtr;
	u8      X_A     bSubTag;
	u8      X_A     bSubLen;
	u8      X_A     hasC9AmendB84;
	u8      X_A     hasC9AmendB85;
	u8      X_A     hasC9AmendB86;
	u8      X_A     hasC9AmendB89;
#endif
#if (EUICC_ONLY||SE_AND_EUICC)//单SE时不做判断
	u8      X_A     offset;
#endif
	sLength 		= popShort();
	SrcBuf 			= getArrayDataWithCheck(sLength);
	
	sdRef 			= popReference();
#if	GP__AMENDMENT__B__FOR_MASK
	hasC9AmendB84 = 0x00;
	hasC9AmendB85 = 0x00;
	hasC9AmendB86 = 0x00;
	hasC9AmendB89 = 0x00;
#endif
	sdRef 			= sdData_getSDdataRef(sdRef);  //初始化一个sdData对象
	sdDataObj 		= MEMtPN8(locateObjectInMemory(sdRef));
	
	// offset to install instance aid len 
	bLen = PM_R_U8(SrcBuf);
	
	// offset to install instance aid 
	SrcBuf = PM8_A_U16(SrcBuf , 0x01);
	
	//offset to privilege len
	SrcBuf = PM8_A_U8(SrcBuf , bLen);
	bLen = PM_R_U8(SrcBuf);
	
	// offset to privilege 
	SrcBuf = PM8_A_U16(SrcBuf , 0x01);
	
	// offset to C9 Tag 
	SrcBuf = PM8_A_U8(SrcBuf , bLen);
	
	// offset to C9 Len
	bC9Len = PM_R_U8(SrcBuf);
	
	// offset to C9 Data
	SrcBuf = PM8_A_U16(SrcBuf , 0x01);
	
	tempPtr = tempBuffer;
//	tempJ = 0x00;
	tempBufferlen = 0x00;
	isSupportSCP0x = 0x00;

	boffset = 0;
	bLen = 0;

	while(bC9Len > 0)
	{
		//boffset = U8_A_U8(boffset, bLen);

		bTag = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));
		bLen = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));
		if(bLen > 0x7F)
		{
			bLen   = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));
			U8_DEC(bC9Len);
		}

		bC9Len = U8_S_U8(bC9Len, 2);
		bC9Len = U8_S_U8(bC9Len, bLen);

		switch(bTag)
		{
			case 0x81:

				if (bLen != 0x02)
				{
					throw_error(SW_WRONG_DATA);
				}

				value1 = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));  // scp ID
				value2 = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));  // Option I

				if((value1 != SCP_02)
						#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
						&& (value1 != SCP_01) && (value1 != SCP_10)
						#endif
						#if JAVA_CARD_UICC
						&& (value1 != SCP_80)
						#endif
						#if GP__AMENDMENT__B__FOR_MASK
						&& (value1 != SCP_81)
						#endif
						#if GP__AMENDMENT__D
						&& (value1 != SCP_03)
						#endif
						#if GP__SCP__SCP11__FOR_MASK
						&& (value1 != SCP_11)
						#endif
					)
				{
					throw_error(SW_WRONG_DATA);
				}

	#if JAVA_CARD_UICC // modify by xuxiaojun@20220609 修改为正确的宏开关
				if((value1 == SCP_80) && (value2 != 0x00))
				{
					throw_error(SW_WRONG_DATA);
				}
	#endif
				if((value1 == SCP_02) && (value2!=0x15) && (value2!=0x35)
	#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
								   && (value2!=0x55)&&(value2!=0x75)
	#endif
					)
				{
					throw_error(SW_WRONG_DATA);
				}
				if((value1 == SCP_10) && (value2 != 0x01) && (value2 != 0x02))
				{
					throw_error(SW_WRONG_DATA);
				}

				#if GP__AMENDMENT__D
				if((value1 == SCP_03) && ((value2&0x0F) != 0x00) )//GP 附录D Table 5-1: Values of Parameter “i”
				{
					throw_error(SW_WRONG_DATA);
				}
				#endif
				#if GP__SCP__SCP11__FOR_MASK
				if((value1 == SCP_11)&&((value2&0xFF) != 0x10)&&((value2&0xFF) != 0x18)&&((value2&0xFF) != 0x38))//modify by xuxiaojun@20220609 GPC_2.3_F_SCP11_v1.3_PublicRelease:Table 6- 2:  Table 4- 1: Values of Parameter “i”
				{
					throw_error(SW_WRONG_DATA);
				}
				#endif
//				PX_W_U8(tempPtr, value1);
//				tempPtr = PX8_A_U8(tempPtr, 1);
//				PX_W_U8(tempPtr, value2);
				tempPtr[tempBufferlen] = value1;
				U8_INC(tempBufferlen);
				tempPtr[tempBufferlen] = value2;
				U8_INC(tempBufferlen);

				if (value1 != SCP_10)
				{
					sdData_getKeyGroupRef(sdDataObj, value1, TRUE);
				}
				if( (SCP_01 == value1) ||  (SCP_02 == value1) ||(SCP_03 == value1) )
				{
					isSupportSCP0x = 0x01;
				}

#if (GP__SCP__SCP11__FOR_MASK&&(!GP__SCP__SCP11__CERT_IN_GLOBAL_RAM)) // modify by xuxiaojun@20221025: SCP11的多通道问题修改，增加GP__SCP__SCP11__CERT_IN_GLOBAL_RAM开关，1表示SCP11C的建立会话过程中临时证书存放在全局变量中；0表示临时证书存放在SSD的DTR中
				if(value1 == SCP_11) // modify by xuxiaojun@20220927： RAM优化，G_TmpCert用DTR
				{
					sdData_getSCP11SessionPara(sdDataObj, MODE_CREATE_SCP11_SESSIONPARA);
				}
#endif
				hasC981 = 1;//modify by chentao@EPX产业化\STM_ST33G1M2\DEVELOP

				break;

			case 0x82:

				if((hasC982 == 1) || ((bLen != 0x01)
		#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
						&& (bLen != 0x02)
		#endif
					))
				{
					throw_error(SW_WRONG_DATA);
				}

				value1 = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));

				if(bLen == 0x01)
				{
					bC982Value = U16_OR_U16(U16_SHL_U8(U8t16(value1),  8), U16_AND_U16(U8t16(value1),  0x00FF));
				}
				else
				{
					value2 = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));
					bC982Value = U16_SHL_U8(U8t16(value1), 8);
					bC982Value = U16_OR_U16(bC982Value, U16_AND_U16(U8t16(value2), 0x00FF));
				}

				sdData_setTag82Value(sdDataObj, bC982Value);

				hasC982 =1;
				break;

			case 0x83:
				if((hasC983 == 1) || (bLen != 0x01))
				{
					throw_error(SW_WRONG_DATA);
				}
				bC983Value = U8t16(PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset))));
				hasC983 =1;

				sdData_setTag83Value(sdDataObj, bC983Value);
				break;

			case 0x84:
				//profile 3.1 --For  an  SD,  none  of  GlobalPlatform's  "Automatic  Transition"  mechanisms  apply  during  Profile  Package 
				//installation,  the  SDs  lifecycle  state  is  always  the  value  of  the  "lifeCycleState"  parameter.  As  a 
				//consequence,  the  eUICC  shall  ignore  tag  '84'  as defined  in  GlobalPlatform  Card  Specification  UICC 
				//Configuration [GP UC] during Profile Package installation.*/
#if (EUICC_ONLY || SE_AND_EUICC)
				if(bRamBuffer[PROFILE_APDU_MODE] == TRUE)
				{
					break;
				}
				else
#endif
				{
					#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
						pushShort(SW_WRONG_DATA);
						return;
					#else
						if(hasC984 == 1)
						{
							throw_error(SW_WRONG_DATA);
						}

				value1 = PM_R_U8(PM8_A_U8(SrcBuf, U8_S_U8(boffset, 1)));
				if(value1 != 0x00)
				{
					throw_error(SW_WRONG_DATA);
				}
				else//保存存在Tag84的标志
					bRamBuffer[LIFECYCLETRANSITION] = 0x01;
			#endif

				hasC984 = 1;

						sdData_setTag84Value(sdDataObj, value1);
				}
				break;

		#if	GP__AMENDMENT__B__FOR_MASK
			case 0x85:
			case 0xA5:
			{
				if(bLen > MAX_Amend_B_BUFFER_LEN)
				{
					throw_error(SW_FILE_FULL);
				}
				else
				{
					while(bLen > 0)
					{
						bSubTag = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));
						bSubLen = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));

						if(bSubTag == 0x84)
						{
							if(hasC9AmendB84 == 1)
							{
								throw_error(SW_WRONG_DATA);
							}

							if(bSubLen != 0x00)
							{
								sdRef	= _newarrayObject(AType_Byte, bSubLen, BIT_NEED_REFRESH);

								tempAmendBPtr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(sdRef))));

								nvmCopyAtomic(MEMtPM8(tempAmendBPtr),PM8_A_U8(SrcBuf, boffset),  U8t16(bSubLen));

								nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_84_buffer_INDEX), sdRef);
							}

							hasC9AmendB84 = 1;

							bLen -= (bSubLen + 2);

							boffset += bSubLen;
						}
						else if(bSubTag == 0x85)
						{
							if(hasC9AmendB85 == 1)
							{
								throw_error(SW_WRONG_DATA);
							}

							if(bSubLen != 0x00)
							{
								sdRef	= _newarrayObject(AType_Byte, bSubLen, BIT_NEED_REFRESH);

								tempAmendBPtr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(sdRef))));

								nvmCopyAtomic(MEMtPM8(tempAmendBPtr),PM8_A_U8(SrcBuf, boffset),  U8t16(bSubLen));

								nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_85_buffer_INDEX), sdRef);
							}

							hasC9AmendB85 = 1;

							bLen -= (bSubLen + 2);

							boffset += bSubLen;

						}
						else if(bSubTag == 0x86)
						{
							if(hasC9AmendB86 == 1)
							{
								throw_error(SW_WRONG_DATA);
							}

							if(bSubLen != 0x00)
							{
								sdRef	= _newarrayObject(AType_Byte, bSubLen, BIT_NEED_REFRESH);

								tempAmendBPtr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(sdRef))));

								nvmCopyAtomic(MEMtPM8(tempAmendBPtr),PM8_A_U8(SrcBuf, boffset),  U8t16(bSubLen));

								nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_86_buffer_INDEX), sdRef);
							}

							hasC9AmendB86 = 1;

							bLen -= (bSubLen + 2);

							boffset += bSubLen;

						}
						else if(bSubTag == 0x89)
						{
							if(hasC9AmendB89 == 1)
							{
								throw_error(SW_WRONG_DATA);
							}

							if(bSubLen != 0x00)
							{
								sdRef	= _newarrayObject(AType_Byte, bSubLen, BIT_NEED_REFRESH);

								tempAmendBPtr	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(sdRef))));

								nvmCopyAtomic(MEMtPM8(tempAmendBPtr),PM8_A_U8(SrcBuf, boffset),  U8t16(bSubLen));

								nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_89_buffer_INDEX), sdRef);
							}

							hasC9AmendB89 = 1;

							bLen -= (bSubLen + 2);

							boffset += bSubLen;
						}
						else
						{
							throw_error(SW_WRONG_DATA);
						}
					}
				}

				break;
			}
		#endif

			case 0x87:

				if((hasC987 == 1) || ((bLen != 0x01)
		#if !GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
						&& (bLen != 0x02)
		#endif
					))
				{
					throw_error(SW_WRONG_DATA);
				}

				value1 = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));

				if(bLen == 0x01)
				{
					bC987Value = U16_OR_U16(U16_SHL_U8(U8t16(value1),  8), U16_AND_U16(U8t16(value1),  0x00FF));
				}
				else
				{
					value2 = PM_R_U8(PM8_A_U8(SrcBuf, U8_INC(boffset)));
					bC987Value = U16_SHL_U8(U8t16(value1), 8);
					bC987Value = U16_OR_U16(bC987Value, U16_AND_U16(U8t16(value2), 0x00FF));
				}
				hasC987 =1;

				sdData_setTag87Value(sdDataObj, bC987Value);
				break;

			case 0x86:
				#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
					throw_error(SW_WRONG_DATA);
				#endif
				break;

			default:
				if(OS_CONF4_C9_45_SUPPORT) // GP CI Compliance 2_0_1
				{
					//冲突，这个要怎么处理还要讨论下，BCTC送检UL-CI测试，ssd安装参数是C90145， 在给ssd写入scp02key后期望ssd的状态切换为0x0F, 所以需要注释以下这行。 jiangqingbin-20220720
					//bRamBuffer[LIFECYCLETRANSITION] = 0x01;//add by xuxiaojun@2022.07.01 工行认证时给SD PUT KEY之后，又执行80F2切换SD到personal状态时返回6985问题。原因是SD安装时不带有84参数，那么put key之后就会自动切换到personal状态，再次执行80f2时就会抛异常6985
					//sdData_setTag84Value(sdDataObj, 0x00);
					if(bTag == 0x45)
					{
						bC982Value = 0xF8F8;
						sdData_setTag82Value(sdDataObj, bC982Value);
						sdData_setTag87Value(sdDataObj, bC982Value);
						bC9Len = 0;
						break;
					}
				}
				else // modify by xuxiaojun@20220621: OS_CONF4_C9_45_SUPPORT配置打开时，如果bTag是其他值也允许安装成功
				{
					throw_error(SW_WRONG_DATA);
				}
		}
	}

	if(JcvmConfig_GetGetRequirement_Default_SCP_Configure() == Requirement_Default_SCP_NO) // 配置所有的SD默认不支持SCP02
	{
		if(hasC981 == 0) //error--不存在81tag
		{
			throw_error(SW_WRONG_DATA);
		}
	}
#if (EUICC_ONLY||SE_AND_EUICC)
	else if(JcvmConfig_GetGetRequirement_Default_SCP_Configure() == Requirement_Default_SCP_SE)
	{
		if((hasC981 == 0) &&(G_CurExecutingProfileIndex != ILLEGAL_PROFILEINDEX) ) //error--不存在81tag
		{
			throw_error(SW_WRONG_DATA);
		}
	}
#endif

	if(JcvmConfig_GetGetRequirement_Default_SCP_Configure() == Requirement_Default_SCP_ALL) //if(JcvmConfig_GetOPerator() == OPERATOR_CHINATELECOM)
	{
		if(0x00 == isSupportSCP0x)//对于profile的应用（SD），不支持SCP0X协议 需要默认增加SCP02
		{
			//tempPtr[tempBufferlen] = JcvmConfig_GetCmScpType(); // modify by xuxiaojun@20211009 去掉SCP重复宏定义
			tempPtr[tempBufferlen] = (JcvmConfig_GetCmScpTypeIsSCP03 ? SCP_03  : SCP_02);	//modify by jiangqingbin@20211125,修改支持的协议的配置方案
			U8_INC(tempBufferlen);
			tempPtr[tempBufferlen] = JcvmConfig_GetScpOption();// modify by xuxiaojun@20211009 去掉OPTION重复宏定义
			U8_INC(tempBufferlen);
		}
	}
	else if(JcvmConfig_GetGetRequirement_Default_SCP_Configure() == Requirement_Default_SCP_SE)// SE cardmanager不调用C9 ， 处理的是SSD
	{
#if (EUICC_ONLY||SE_AND_EUICC)
		if(G_CurExecutingProfileIndex == ILLEGAL_PROFILEINDEX)//对于profile的应用（SD），不支持SCP0X协议 不需要默认增加SCP02
		{
#endif
			if(0x00 == isSupportSCP0x)
			{
				//tempPtr[tempBufferlen] = JcvmConfig_GetCmScpType();// modify by xuxiaojun@20211009 去掉SCP重复宏定义
				tempPtr[tempBufferlen] = (JcvmConfig_GetCmScpTypeIsSCP03 ? SCP_03 : SCP_02);	//modify by  jiangqingbin@20211125,修改支持的协议的配置方案
				U8_INC(tempBufferlen);
				tempPtr[tempBufferlen] = JcvmConfig_GetScpOption();// modify by xuxiaojun@20211009 去掉OPTION重复宏定义
				U8_INC(tempBufferlen);
			}
#if (EUICC_ONLY||SE_AND_EUICC)
		}
#endif
	}

#if (EUICC_ONLY||SE_AND_EUICC)//单SE时不做判断
	if(bRamBuffer[PROFILE_INSTALL_MNOSD_FLAG] == TRUE)//安装MNO-SD,MNO-SD支持SCP80时,需要检查tar,安装普通应用不检查
	{
		if(0x00 != tempBufferlen)
		{
			for(offset = 0;offset < tempBufferlen;offset += 2)
			{
				if(tempPtr[offset] == SCP_80)//HB_Offline_Profile_Test/14-PROFILE PE数据异常测试/Profile_FulSeg0075 ,MNOSD EA参数中不带TAR 但是支持SCP80要求报错 ，collis补充测试也有相关测试
				{
					//bRamBuffer[SD_SUPPORT_SCP80_FLAG] = TRUE;
					//break;
					if(PXB_R_U8(PNtMEM(&bRamBuffer[SD_TAR_INFO_OFF+sdAppId_offset])) == APP_NULL)//支持SCP80 但是EA  CA中没有带TAR
					{
						if(MOBILE_Operator == OPERATOR_CHINATELECOM)//modify by zhxx@20230310,德国商用测试，BugList_23_04，MNOSD EA参数中不带TAR 但是支持SCP80不要求报错
						{
						    throw_error(SW_WRONG_DATA);
						}
					}
				}
			}
		}
	}
#endif

	if(0x00 != tempBufferlen)
	{
		sdRef	= _newarrayObject(AType_Byte, U8_A_U8(tempBufferlen, BYTE_ARRAY_HEADER_SIZE), BIT_NEED_REFRESH);
		SrcBuf	= MEMtPN8(get_array_data(MEMtPF8(locateObjectInMemory(sdRef))));

		nvmCopyAtomic(SrcBuf, MEMtPM8(tempPtr), U8t16(tempBufferlen));

		nvmWriteU16Atomic(get_object_field_ref(sdDataObj, scpBuffer_INDEX), sdRef);

		sdData_setScpType(sdDataObj, tempPtr[0]);
		sdData_setScpOption(sdDataObj, tempPtr[1]);
	}
#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	if(bRamBuffer[AUTHFAILCNT])
		sdData_setAuthFailCount(sdDataObj, bRamBuffer[AUTHFAILCNT]);
	if(bRamBuffer[MAXSYMMETRICKEYCNT])
		sdData_setDESKeyCount(sdDataObj, bRamBuffer[MAXSYMMETRICKEYCNT]);
#endif
	
}
#endif

#define FREE_CARD_SPACE                     (u16)0xFF20    

#if !GP__VERSION__FOR_MINI_MASK
void isActiveApp(u16 appId)
{
	u8  	i_iter;
	u8		tempState;
	u16		conText;

	tempState	=	0;
	conText		=	0;

	//jiangqingbin-20221014，由缺陷编号411和434延伸, 并经过会议讨论(平台中所有涉及到http流程的疑问讨论点-20221008),删除sd/app时需要增加远程服务是否正在操作该对象的判断
	if (IF_IsRAMActiveApp(U16t8(appId)) == true)
	{
		pushShort(true);
		return;
	}


	for (i_iter = 0; i_iter < logicalChannelNums; U8_INC(i_iter))
	{
		tempState = getChannelState(i_iter);
		if (tempState == CHANNEL_OPEN)
		{
		#if (EUICC_ONLY||SE_AND_EUICC)
			if(getChannelProfileIndex(i_iter) == G_CurExecutingProfileIndex)
		#endif
			{
				conText	= PX_R_U16(&channelInfo.selected_context[U8_M_U8((i_iter),2)]);
				if(U16_AND_U16(conText, APPLET_MASK) == appId)
				{
					pushShort(true);
					return;
				}
			}
		}
	}
	pushShort(false);
}
#endif

u8 getCurrentSDId(void)
{
	return g_bSDId[selected_channel];
}

#if (GP__VERSION__FOR_CHINA_UNICOM_MASK && JCVM__IO__SWP)

#define TAG_OPEN_MIFARE_SET_SAK     0x81
#define TAG_OPEN_MIFARE_SET_ATQA    0x82


u16 openMifare(u8 *apdu)
{
	u8 sak;
	u8 backupSak;
	u8 i;
	u8 atqa[2];
	u8 backupAtqa[2];
	
	if ( (apdu[OFFSET_P1] != 0x00) || (apdu[OFFSET_P2] != 0x00) )
	{
		return SW_INCORRECT_P1P2;
	}
	
	if ( (apdu[OFFSET_LC] != 0x07) && (apdu[OFFSET_LC] != 0x00) )
	{
		return SW_WRONG_LENGTH;
	}
	
	if (isMifareActivated() == 0x00)
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}
	
// 	if (memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL))) == 0x01)
// 	{
// 		return SW_CONDITIONS_NOT_SATISFIED;
// 	}
	if ( (nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) & MIFARE_SWITCH_MASK) == 0x00 )
	{  // mifare enable
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	if (apdu[OFFSET_LC] == 0x00)
	{
		sak = memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_SAK)));
		memCopyNoAtomic(MEMtPM8(&atqa[0]), MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_ATQA)), 2);
	}
	else
	{
		//apdu_ApduSetIncomingAndReceive();

		for(i=0; i < apdu[OFFSET_LC];)
		{
			if ( apdu[OFFSET_CDATA + i] == TAG_OPEN_MIFARE_SET_SAK)
			{
				if (apdu[OFFSET_CDATA + i + 1] != 0x01)
				{
					return SW_WRONG_DATA;
				}
				
				sak = apdu[OFFSET_CDATA + i + 2];
				i += 3;
			}
			else if ( apdu[OFFSET_CDATA + i] == TAG_OPEN_MIFARE_SET_ATQA)
			{
				if (apdu[OFFSET_CDATA + i + 1] != 0x02)
				{
					return SW_WRONG_DATA;
				}
				
				memCopyNoAtomic(atqa, &apdu[OFFSET_CDATA + i + 2], 2);
				i += 4;
			}
			else
			{
				return SW_WRONG_DATA;
			}
		}
	}
	
	backupSak = memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_SAK)));
	memCopyNoAtomic(MEMtPM8(&backupAtqa[0]), MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_ATQA)), 2);

	//backup sak
	memWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_SAK)), backupSak);
	//backup atqa
	memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_ATQA)), MEMtPM8(&backupAtqa[0]), 2);

	//set sak
	memWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_SAK)), sak);
	//set atqa
	memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_ATQA)), MEMtPM8(&atqa[0]), 2);

	//enable mifare
	nvmWriteU8Atomic( MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH)), nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) & ~MIFARE_SWITCH_MASK);

	//set control
	memWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL)), 1);
	
	changeMifareUID();
	
	swp_ResetSwpConfigData();
	
	if (isSwpInterfaceEnabled())
	{
		//设置 SAK
		setSwpTypeAPara(HCP_RFTYPEA_GATE_REG_INDEX_SAK, &sak, 1);
		ecpSwpTimeDelay(10);

		//设置 ATQA
		setSwpTypeAPara(HCP_RFTYPEA_GATE_REG_INDEX_ATQA, MEMtPM8(&atqa[0]), 2);
		ecpSwpTimeDelay(10);
	}

	return SW_NO_ERROR;
}


u16 closeMifare(u8 *apdu)
{
	u8 sak;
	u8 atqa[2];

	if ( (apdu[OFFSET_P1] != 0x00) || (apdu[OFFSET_P2] != 0x00) )
	{
		return SW_INCORRECT_P1P2;
	}
	if (apdu[OFFSET_LC] != 0x00)
	{
		return SW_WRONG_LENGTH;
	}
	
	if (isMifareActivated() == 0x00)
	{
		return SW_CONDITIONS_NOT_SATISFIED;
	}

// 	if (memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL))) == 0x00)
// 	{
// 		return SW_CONDITIONS_NOT_SATISFIED;
// 	}
	if ( (nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) & MIFARE_SWITCH_MASK) != 0x00 )
	{  // mifare disable
		return SW_CONDITIONS_NOT_SATISFIED;
	}

	sak = memReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_SAK)));
	memCopyNoAtomic(MEMtPM8(&atqa[0]), MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CURRENT_ATQA)), 2);

	//backup sak
	memWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_SAK)), sak);
	//backup atqa
	memCopyAtomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(SWP_CONFIG_ATQA)), MEMtPM8(&atqa[0]), 2);

	//disable mifare
	nvmWriteU8Atomic( MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH)), nvmReadU8(MEMtPN8(JcvmConfig_GetVmConfigAddr(IFX_SWITCH))) | MIFARE_SWITCH_MASK);
	
	//set control
	memWriteU8Atomic(MEMtPN8(JcvmConfig_GetVmConfigAddr(OPEN_MIFARE_CONTROL)), 0);

	changeMifareUID();

	swp_ResetSwpConfigData();
	
	if (isSwpInterfaceEnabled())
	{
		//设置 SAK
		setSwpTypeAPara(HCP_RFTYPEA_GATE_REG_INDEX_SAK, &sak, 1);
		ecpSwpTimeDelay(10);

		//设置 ATQA
		setSwpTypeAPara(HCP_RFTYPEA_GATE_REG_INDEX_ATQA, MEMtPM8(&atqa[0]), 2);
		ecpSwpTimeDelay(10);
	}
	
	return SW_NO_ERROR;
}
#endif

u8 check_FinalApplicationPrivi(u8  sdId)
{
	u16 	    D_A			bPrivRef;
	memref	    D_A		    entry;
	memref	    D_A		    bPrivArray;

	entry		=	getEntryByAppId(sdId);
	bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));

	bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef))) + 1;
	if(U8_AND_U8(PM_R_U8(bPrivArray), IS_FINAL_APPLICATION_FLAG) != IS_FINAL_APPLICATION_FLAG)
	{
		return FALSE;
	}
	return TRUE;
}
//从指定APP TABLE中查找指定AID所在的entry
u16 SearchEntryFromSpecAppTableByInstAID(u32 AidAdd2, u8 AidLen2,PN8 theAppTable,u16 bFreeEntryIndex)
{
	PN8			D_A			entry;
	u32			D_A			AidAdd1;
	u16			D_A			i;
	u8			D_A			AidLen1;

	if(AidLen2 != 0)
	{

		for (i = APP_FIRST; i < bFreeEntryIndex; U16_INC(i))
		{
				entry	= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)))));
				AidAdd1	= locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(entry), theAID_INDEX))));

				//when aid is 0; check it immediately
				if(AidAdd1 == 0){
					continue;
				}

				AidAdd1 = locateObjectInMemory(nvmReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(AidAdd1), AIDARRAY_INDEX_OF_AID))));
				AidLen1 = U16t8(get_array_length(MEMtPF8(AidAdd1)));
				AidAdd1 = get_array_data(MEMtPF8(AidAdd1));

				if((AidLen2 == AidLen1) && (memCompare(MEMtPM8(AidAdd1),MEMtPM8(AidAdd2),AidLen1) == ECP_CMP_RESULT_EQUAL))
				{
					return nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(i, SIZE_REF)));
				}
		}
	}

	return REF_NULL;
}
/*
*description:在应用列表中通过查找应用AID获取应用的appId
*
*@param     :u8 * aidValue
*@param     :u8 aidLen
*
*@return    :appId
*
*@history   :从platform_api.c文件移动到这里
*/
u8 GetAppIdByAIDFromAIDList(u8 *aidValue,u8 aidLen)
{
	u8   		I_A   		bFreeEntryIndex;
	PN8 		entry, theAppTable;
	
	jref I_A  entryRef;

//	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
//	for (i = 0; i < bFreeEntryIndex; U16_INC(i))
//	{
//		aid = getAppletAid(i);
//		if(aid != APP_INDEX_NULL)
//		{
//			if(AID_equals_array(aid, aidValue, 0x00, aidLen) == true )
//			{
//				theAppTable = getAppTable();
//				entry = locateObjectInMemory(PN_R_U16(U32_A_U16(theAppTable , U8_M_U8(i , SIZE_REF))));
//				return memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(entry), bAppID_INDEX)));
//			}
//		}
//	}


	bFreeEntryIndex		= nvmReadU16(G_CurExecuEnvironment.bFreeEntryIndex);
	theAppTable			= MEMtPN8(getAppTable());
	entryRef = SearchEntryFromSpecAppTableByInstAID(PNtMEM(aidValue),aidLen,theAppTable,bFreeEntryIndex);

	if(entryRef != REF_NULL)
	{
		entry = MEMtPN8(locateObjectInMemory(entryRef)); // modify by xxj@20240718  ??????????,i????
		return memReadU8(MEMtPM8(get_object_field_ref(entry, bAppID_INDEX)));
	}


	return ILLEGAL_APPID;

}
/*
*@Function: check_SCP11cOPCommand
*
*@Descrip : When an SCP11c secure channel session only establishes the ANY_AUTHENTICATED Security Level, the
			usage of Security Domain APDU commands is only allowed if authorized by tag 'BF20' in CERT.OCE.ECKA.
*
*@See     : <GPC_2.3_F_SCP11_v1.2.1.3_Public_Review.pdf>  B.2 Limitations on Commands Received through SCP11c
*
*@Return  : return SW_NO_ERROR if Command is valid, else return others;
*
*@History : V1.0 --- created by qiuping @20210727
*/
#if GP__SCP__SCP11__FOR_MASK
//modify by xuxiaojun@20220627: SCP11C中执行的指令需要获取数据域信息，所以把收数据提到check_SCP11cOPCommand之前。对于SCP11c执行之后还会继续往下执行的指令，其在掉用apdu_ApduSetIncomingAndReceive之前统一增加判断apdu_IsSetIncoming() == 0，避免重复收数据时状态字为6F00
u16 check_SCP11cOPCommand(void)
{
#if GP__SCP__SCP11__C_AUTHORIZATIONS
	u8 					p1;
	u8					occur;
	u8					off;
	PX8					aidBuf;
	PX8					privBuf;
	u16 				bPrivRef;
	memref				entry;
	memref 				bPrivArray;
#endif
	u8					bIns_;
	u8					dataBuf[0x11];

	bIns_ = APDUBuffer[OFFSET_INS];
#if GP__SCP__SCP11__C_AUTHORIZATIONS
	p1 = APDUBuffer[OFFSET_P1];
#endif
	ecpMem_Set(dataBuf, 0x00, sizeof(dataBuf));

	switch (bIns_)
	{
		case INS_OP_DELETE:
		{
			if((APDUBuffer[OFFSET_CDATA] == 0xD0) || (APDUBuffer[OFFSET_CDATA] == 0xD2))	//DELETE (key(s)) command
			{
				return SW_CONDITIONS_NOT_SATISFIED; // The usage of PUT KEY, DELETE [key(s)], and SET STATUS commands shall not be allowed
			}
#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
			if(U8_AND_U8(channelInfo.security_level[selected_channel], SCF_AUTHENTICATED_ANY) == SCF_AUTHENTICATED_ANY )
			{
				aidBuf = &APDUBuffer[OFFSET_CDATA + 1];		//LV

				//see B.6 Cumulative DELETE by Parent SD
				if((APDUBuffer[OFFSET_P2] == 0x40))	//The cumulative DELETE command (P2.b7=1)
				{
					entry		=	getEntryByAppId(getCurrentSDId());
					bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));

					bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
					//The Security Domain that processes the SCP11c session has the Global Delete privilege
					if(U8_AND_U8(PM_R_U8(bPrivArray + 1), GLOBAL_DELETE_PRIVILEGES_TAG) != GLOBAL_DELETE_PRIVILEGES_TAG)
					{

						return SW_CONDITIONS_NOT_SATISFIED;
					}

					// The AID to be deleted refers to a Security Domain.
					entry		=	getEntryByAppId(GetAppIdByAIDFromAIDList(&aidBuf[1], aidBuf[0]));
					bPrivRef	=	memReadU16(MEMtPN8(get_object_field_ref(entry, bPriv_INDEX)));

					bPrivArray 	= 	get_array_data(MEMtPN8(locateObjectInMemory(bPrivRef)));
					if(U8_AND_U8(PM_R_U8(bPrivArray), SECUREDOMAIN_PRIVI_TAG) != SECUREDOMAIN_PRIVI_TAG)
					{
						return SW_CONDITIONS_NOT_SATISFIED;
					}
				}

				while(1)
				{
					occur = 1;
					if(BF20_GetField(BF20_DELETE_APP_ELF_AID_TAG, dataBuf, sizeof(dataBuf), occur) == true)
					{
						if(memCompare(dataBuf, aidBuf, dataBuf[0] + 1) != ECP_CMP_RESULT_EQUAL)
						{
							occur ++;
							continue;
						}
						else
						{
							break;
						}
					}
					else
					{
						return SW_CONDITIONS_NOT_SATISFIED;
					}
				}
			}
#endif
			break;
		}
#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
		case INS_OP_INSTALL:
		{
			if(U8_AND_U8(channelInfo.security_level[selected_channel], SCF_AUTHENTICATED_ANY) == SCF_AUTHENTICATED_ANY )
			{
				switch (U8_AND_U8(p1, 0x7f))
				{
					case P1_OP_INSTALL_FOR_INSTALL:
					case P1_OP_INSTALL_FOR_MAKE_SELECTABLE:
					case P1_OP_INSTALL_FOR_INSTALL_AND_MAKE_SELECTABLE:
					case P1_OP_INSTALL_FOR_REGISTRY_UPDATE:
					{
						off = OFFSET_CDATA;					//偏移指向第一个 Length
						off = off + APDUBuffer[off] + 1;	//偏移指向第二个 Length
						off = off + APDUBuffer[off] + 1;	//偏移指向第三个 Length
						off = off + APDUBuffer[off] + 1;	//偏移指向Length of Privileges
						privBuf = &APDUBuffer[off];		//LV

						if(BF20_GetField(BF20_CREAT_APP_PRIVILEGE_TAG, dataBuf, sizeof(dataBuf), 1) == true)
						{
							if(memCompare(dataBuf, privBuf, dataBuf[0] + 1) != ECP_CMP_RESULT_EQUAL)
							{
								return SW_CONDITIONS_NOT_SATISFIED;
							}
							break;
						}
						return SW_CONDITIONS_NOT_SATISFIED;
					}

					case P1_OP_INSTALL_FOR_LOAD:
					case P1_OP_INSTALL_FOR_PERSONALIZE:
					case P1_OP_INSTALL_FOR_EXTRADITION:
					{
						break;
					}
					default:
						return SW_INCORRECT_P1P2;
				}
				while(1)
				{
					occur = 1;
					if(BF20_GetField(BF20_INSTALL_OR_LOAD_AID_TAG, dataBuf, sizeof(dataBuf), occur) == true)
					{
						if((U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_MAKE_SELECTABLE)
								||(U8_AND_U8(p1, 0x7f) == P1_OP_INSTALL_FOR_PERSONALIZE))
						{
							aidBuf = &APDUBuffer[OFFSET_CDATA + 2];		//LV
						}
						else
						{
							aidBuf = &APDUBuffer[OFFSET_CDATA + 1];		//LV
						}
						if(memCompare(dataBuf, aidBuf, dataBuf[0] + 1) != ECP_CMP_RESULT_EQUAL)
						{
							occur ++;
							continue;
						}
						else
						{
							break;
						}
					}
					else
					{
						return SW_CONDITIONS_NOT_SATISFIED;
					}
				}
			}
			break;
		}

		case INS_OP_LOAD:
		{
			//
			break;
		}
#endif
		case INS_OP_STORE_DATA: // modify by xuxiaojun@20220626 在SCP11C会话中，禁止使用STORE DATA来新增或更新密钥
		{
			if((APDUBuffer[OFFSET_CDATA] == 0x00)&&(APDUBuffer[OFFSET_CDATA + 1] == 0xB9))//Table 11-91:  DGI for Key Information Data
			{
				return SW_CONDITIONS_NOT_SATISFIED; //The STORE DATA command is allowed but should not be used to load keys out of a secure and controlled environment.
			}
			break;
		}
#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
		case INS_OP_GET_DATA:
		case INS_OP_SELECT:
			break;
		case INS_OP_GET_STATUS:
#endif
		case INS_OP_PUT_KEY: // The usage of PUT KEY, DELETE [key(s)], and SET STATUS commands shall not be allowed
		case INS_OP_SET_STATUS:
		{
			return SW_CONDITIONS_NOT_SATISFIED;
		}
#if GP__SCP__SCP11__C_AUTHORIZATIONS // 根据测试部的意见，暂时不实现这一块，大家对这一块的理解可能存在差异
		case INS_OP_PERFORM_TRANSACTION: // 0x12  Table B-4:  PERFORM TRANSACTION OPERATION Command Message
			
			if (APDUBuffer[OFFSET_P2] == 0x80)
			{
				if (!isInProgress())
				{
					begin_transaction(MULTIL_APDU_PROCESS);
				}
			}
			else if (APDUBuffer[OFFSET_P2] == 0x81)
			{

				commit_transaction();
			}
			
			break;
		default:
			break;
#endif

	}
	return SW_NO_ERROR;
}
#endif
/*@ +namechecks @*/

