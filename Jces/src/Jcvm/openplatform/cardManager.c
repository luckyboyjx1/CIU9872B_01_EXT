#include "CgmManager.h"
#include "cardManager.h"
#include "global_variable.h"
#include "context.h"
#include "chn_manager.h"
#include "mask.h"
#include "contentManager.h"
#include "stack.h"
#include "opFunction.h"
#include "object.h"
#include "ob_manager.h"
#include "Io.h"
/*#include "string.h"*/
#include "patch.h"
#include "nativemethod_impl.h"
#include "Apdu.h"
#include "MemOperation.h"
#include "JcvmConfig.h"
#include "PbocAPI.h"
#include "AppAPI.h"
#include "Crypto_API.h"
#include "SecureCrypto.h"
#include "dataManager.h"
#include "SCPInterface.h"
#include "SEEUICCInterface.h"
#include "euiccConfig.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

/*****************************************************************************
 * File:  cardManager.c
 * Version 1.0

 wang hui modify
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 Original author: wang hui
 *****************************************************************************/

#define SELECT_COMMAND_FLAG   0xF0            //表示选择命令

#define BASIC_CHANNEL         0x00
#define CLOSE_CHANNEL         0xFE
#define OPEN_CHANNEL          0x80
#define SWP_TRIG_EVENT		  0xAA
#define WARM_RESET            0xA5

#define SELECT_MODE_MASK	  0x03
#define	SELECT_FIRST		  0x00
#define SELECT_LAST			  0x01
#define SELECT_NEXT			  0x02
#define SELECT_PREV			  0x03
#define SELECT_LAST_FLAG	  0Xf1

s8 select_init(PX8 bStartIndex_, PX8 bSearchProfile, PX8 bSwitchProfile);
//u8 SetChannel(memref fileAddr,u16 fileName,u8 option); removing warning

#if GP__VERSION__FOR_CHINA_UNICOM_MASK
boolean isTCKTesting(void);
#endif

/**
 * 参考https://192.168.0.6/svn/cos2/JCES_PRODUCT/V2.0/FINANCE/CIU9872B_01PLUS_306_VG/DEVELOP的实现方案，把该函数复制过来
 */
void initDiversifiedDataArr(void)
{
	PM8 	aidBuffer;
	PM8		diversifiedDataFile;
	PM8		cpclDataArr;
	u8		diverDataArr[10];
	u8 		aidLength;
//	PATCH_JCVM_OPENPLATFORM_CARDMANAGER_003();

	aidBuffer 					= getArrayDataWithoutCheck();

	diversifiedDataFile 		= getJavaObjectDataArray();

	aidLength					= memReadU8(aidBuffer);

	aidLength					= U8_S_U8(aidLength, 1);
	aidBuffer					= PM8_A_U8(aidBuffer, aidLength);

	diverDataArr[0]			= memReadU8(aidBuffer);
	PM8_INC(aidBuffer);
	diverDataArr[1]			= memReadU8(aidBuffer);

	cpclDataArr 				= MEMtPN8(JcvmConfig_GetCplcDataConfBase());

	memCopyAtomic(MEMtPM8(&diverDataArr[2]), MEMtPM8(&cpclDataArr[10]), 0x08);
	memCopyAtomic(diversifiedDataFile, diverDataArr, 0x0A);
}


/**
* modify by lixin@20210816 增加选择、安装、删除应用时对FINAL权限的处理； https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
*/
static void selectApplet()
{
	u8          D_A     bSwitchProfile = false; // true indicate that need switch profile; false indicate that maintain profile
	u8          D_A     bSearchProfile = ILLEGAL_PROFILEINDEX; // the search zone after select_init
#if (EUICC_ONLY||SE_AND_EUICC)
	u8          D_A     bakCurProfile;  // used to backup and restore current profile
#endif
	u8			D_A		loop = 1;
	u8			D_A		loopStep;
	u8			D_A		bState_;
	u8			D_A		bLc_;
	u8			D_A		bP2_;
	u8			D_A		bP2mask;
	u8			D_A		MultiSelResult;
	u8			D_A		bPreAppID_;
//	u8 			D_A		CardState;
//	u8 			D_A		superOwnerId;
	s8			D_A		direction;
	u16			D_A		i;
	u16			D_A		bStartIndex_;
	jref		D_A		aid;
	jref		D_A		theApplet;

	u8 			D_A		CardState;
	u8 			D_A		appIndexWithFinal;
	u8 			D_A		privilege[3];
#if GP__AMENDMENT__C__FOR_MASK
	u8 			D_A		clState;
	u8			D_A		clCommuInterfAccInstance;
	memref		D_A		clAppEntryAddr;
	memref 		D_A		theEntryObjectAddr;

#endif
	bState_			= 0;
	bLc_ 			= PX_R_U8(&APDUBuffer[OFFSET_LC]);
	bStartIndex_ 	= 0;
	direction		= 1;
	MultiSelResult 	= 0;
	bPreAppID_ 		= U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF);

	openChannel(selected_channel);
/*@ -discast @*/
/*@ -type @*/
	if ((AIDLenInvalid(bLc_)) && (bLc_ != 0))
	{
		if (bPreAppID_ == APP_FIRST) //modify by huangzehua@20220819: 解决选择默认应用后，错误选择指令不报错问题。默认应用不报6700. MNOSD_APPID, why does not throw error when pre-applet is other applet?
		{
			//if Pre applet is card manager , throw error
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_WRONG_LENGTH);
		}
		return;
	}
/*@ -type @*/
/*@ +discast @*/

	//移动要求，支持P2 = b1,b2:00,10; b4,b3:00,01,10,11; b7:0,1
	//根据中国移动NFC SIM卡技术规范V2.0.0 第5.9.9.2.2 P2章
		// other select mode ,pass to pre applet process
			// if Pre applet is card manager , throw error

	if(OS_CONF3_TCK_ICC_MODE == OS_TCK_MODE)
	{
		bP2mask 		= 0xE3;
	}
	else
	{
		bP2mask 		= 0xEF;
	}

	bP2_ = U8_AND_U8(PX_R_U8(&APDUBuffer[OFFSET_P2]), bP2mask);

	#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK || GP__VERSION__FOR_CHINA_MOBILE_SWP2_0_MASK

	#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
		if ( (U8_AND_U8(bP2_ , 0xB3) != 0x00) && (U8_AND_U8(bP2_ , 0xB3) != 0x02) )
	#else
		if ( (U8_AND_U8(bP2_ , 0xA3) != 0x00) && (U8_AND_U8(bP2_ , 0xA3) != 0x02) )
	#endif
		{
			if (bPreAppID_ == 0)
			{
				//bShortBuffer[VM_EXCEPTION] = htoj16(SW_INCORRECT_P1P2);
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2);
				
				
			}
			return;
		}
	#endif

	CardState = JcvmConfig_GetCardState();

	direction = select_init(MEMtPX8(&bStartIndex_), MEMtPX8(&bSearchProfile), MEMtPX8(&bSwitchProfile));

	if(SEARCH_PASS_TO_CURRENT_APP == direction) // result of select previous mode, should return
	{
		return;
	}

	if(SW_FILE_NOT_FOUND== PXB_R_U16(&bShortBuffer[VM_EXCEPTION]))// result of select next/previous mode, should return
	{
		return;
	}

#if (EUICC_ONLY||SE_AND_EUICC)
	if(bSwitchProfile == true) //search from SE to enabled profile
	{
		loop = 2;
	}
	else
	{
		loop = 1;
	}

	bakCurProfile = getChannelProfileIndex(selected_channel);// 备份通道上的profile信息与上一条指令一致，无论是否选择成功 都需要还原（deselect之前不能改G_CurExecutingProfileIndex等）
#endif

	for(loopStep = 0; loopStep < loop; loopStep++)
	{
		//go to USIM to get last select app
		if(SEARCH_LAST == direction)
		{
			//bRamBuffer[SELECTING_APPLET_FLAG] = SELECT_LAST_FLAG;
			//return;
		}

		//must read bStartIndex with pointer, because its value is bottom up in simulator

#if (EUICC_ONLY||SE_AND_EUICC)
		if(loopStep == 0)
		{
			G_CurExecutingProfileIndex = bSearchProfile; // maybe current enabled profile or SE（bSearchProfile可能是SE 可能是profile）
		}
		else
		{
			G_CurExecutingProfileIndex = G_CurEnabledProfileIndex;
		}

		ExecuEnvironmentInit(G_CurExecutingProfileIndex); // switch G_CurExecuEnvironment
#endif

		for (i = PX_R_U16(MEMtPX8(&bStartIndex_)); (i != APP_INDEX_NULL); i = GetAppEntryIdxByPriority(i, direction))
		{
			aid	= getAppletAid(i);

			if (aid != APP_INDEX_NULL)
			{
				if (AID_partialEquals_array(aid, APDUBuffer, OFFSET_CDATA, bLc_) == true)
				{
					u8 bAppID = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), bAppID_INDEX)));

	#if GP__AMENDMENT__C__FOR_MASK
					if(bRamBuffer[AMDC_VOLATILE_PRIORITY_TAG] == NEED_VOLATILE_AMENDMENTC_SEARCH)
					{
						theEntryObjectAddr = locateObjectInMemory(PN_R_U16(U32_A_U16(APP_Table_Cash, U16_M_U16(i, SIZE_REF))));
						if(isHaveVolatilePriority(theEntryObjectAddr))
						{
								 continue ;
						}
					}
	#endif
					if (CARD_TERMINATED == CardState)
					{
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INS_NOT_SUPPORTED);
						return;
					}

					getPrivilegeByAppId(bAppID, &privilege[0]);

					if (bAppID == APP_FIRST) // MNOSD是不是也在c层处理？
					{ // if select the isd , process in c level.
						sendSelectResponceAPDU(aid,bAppID);
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_NO_ERROR);
						PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], SELECT_COMMAND_FLAG);
						PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], 0); // 0?

						if (CardState == CARD_LOCKED)
						{
							if((privilege[1] & IS_FINAL_APPLICATION_FLAG) != IS_FINAL_APPLICATION_FLAG)
							{
								appIndexWithFinal = getAppIndexByPrivilege(IS_FINAL_APPLICATION_FLAG, 2); //找到具有FINAL权限的应用的appIndex

								// if select a applet when card is CARD_LOCKED, select the sd with FINAL Privilege .
								PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], appIndexWithFinal);

								PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPORTED);
								return;
							}
							else
							{
								PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), APPLICATION_INVALIDATED);
							}
						}

						//g_bISDId[selected_channel] = PN_R_U8(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), bAppID_INDEX));
						g_bISDId[selected_channel] = 0x00;
						g_bSDId[selected_channel] = g_bISDId[selected_channel];
						closeSecuritychannel(SC_TERMINATED);// modify by liweimin

						//setChannelProfileIndex(selected_channel, G_CurExecutingProfileIndex);

					#if (EUICC_ONLY||SE_AND_EUICC)
						setNextProfileIndex(G_CurExecutingProfileIndex);
						G_CurExecutingProfileIndex = bakCurProfile;
						ExecuEnvironmentInit(G_CurExecutingProfileIndex);
					#endif

						return; // Applet found, the G_CurExecutingProfileIndex need not restore
					}
					// else if other applet not be isd .
					if (CardState == CARD_LOCKED )
					{
						// vgp spc 6.11.2
						//(The Application on the Basic Logical Channel shall first be deselected and the Issuer Security Domain shall
						//become the selected Application on the Basic Logical Channel)
//						if (bPreAppID_ != g_bISDId[selected_channel])
//						{
//							// if select a applet when card is CARD_LOCKED, select the isd .
//							PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], SELECT_COMMAND_FLAG);
//							PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], g_bISDId[selected_channel]);
//
//						}
						if((privilege[1] & IS_FINAL_APPLICATION_FLAG) == IS_FINAL_APPLICATION_FLAG)
						{
							PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], SELECT_COMMAND_FLAG);
							PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], i);

							PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), APPLICATION_INVALIDATED);

							if(U8_AND_U8(privilege[0] , 0x80) == 0x80) //SD
							{
								g_bSDId[selected_channel] = bAppID;
							}
							else
							{
								g_bSDId[selected_channel] = memReadU8(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), bsdId_INDEX));
							}

							return;
						}
						else
						{
							appIndexWithFinal = getAppIndexByPrivilege(IS_FINAL_APPLICATION_FLAG, 2); //找到具有FINAL权限的应用的appIndex

							PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], SELECT_COMMAND_FLAG);
							PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], appIndexWithFinal); // if select a applet when card is CARD_LOCKED, select the sd with FINAL Privilege .
						}

						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPORTED);
						return;
					}

					bState_			= memReadU8(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), bCurrentState_INDEX));
					
					//GP 2.1.1 P57:If a matched applet is found and have multi-selection restriction,continue to search the next applet.
					theApplet		= memReadU16(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), theApplet_INDEX));

					MultiSelResult	= MultiSelectState(theApplet,selected_channel);

					if(MultiSelResult == MUL_SEL_RESTRICTION)
					{
						continue;
					}

					// vgp spec 6.11.2.3
					//If a matching Application is found but the Application is in the Life Cycle State LOCKED, then if no
					//subsequent matching Application exists in the GlobalPlatform Registry, a response of '6A82' shall be
					//returned.
					//if (bState_ == APPLET_SELECTABLE || bState_ == APPLET_PERSONALIZED || bState_ == APPLET_BLOCKED ||)
					if (U8_AND_U8(bState_, 0x87) == APPLET_SELECTABLE)
					{
						u16 bPrivRef;
						u8 bFirstPrivilageByte;
						memref bPrivArray;

						bPrivRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), bPriv_INDEX)));
						bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
						bFirstPrivilageByte = memReadU8(MEMtPM8(bPrivArray));

	#if GP__AMENDMENT__C__FOR_MASK
						//判断当前协议类型，并且判断该非接应用是否为未激活状态
						//解决部分AID选择应用，查找激活的应用
						if (io_GetCurrentProtocol() == PROTOCOL_SWP)
						{
							clAppEntryAddr = locateObjectInMemory(PN_R_U16(get_object_field_ref(MEMtPF8(APP_Entry_Object_Cash), clAppEntry_INDEX)));
							//if((REF_NULL != clAppEntryAddr) && ((memref)REF_ADDRESS_NULL != clAppEntryAddr) )
							if(REF_NULL != clAppEntryAddr)
							{
								clState = memReadU8(MEMtPN8(get_object_field_ref(MEMtPF8(clAppEntryAddr), CLState_INDEX)));
								//is not active
								if (clState != 0x01)
								{
									continue;
								}
								//merged from CIU9872B_01@20220926
								//check Communication Interface Access Parameters is iso14443 support, add by wrh 20221018
								clCommuInterfAccInstance = memReadU16(MEMtPN8(get_object_field_ref(MEMtPF8(clAppEntryAddr), CommuInterfAccInstance_INDEX)));
								if ((clCommuInterfAccInstance&0x40) != 0x40)
								{
									continue;
								}
							}
							else
							{
								continue;
							}
						}
	#endif
						if(U8_AND_U8(bFirstPrivilageByte , 0x80) == 0x80)
						{
							g_bSDId[selected_channel] = memReadU8(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), bAppID_INDEX));	 //wzq
						}
						else if(memReadU8(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), bAppID_INDEX)) != 0)
						{
							g_bSDId[selected_channel] = memReadU8(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), bsdId_INDEX));
						}
						else
						{
							g_bSDId[selected_channel] = 0;	//ISD_ID
						}

						//inform the Java Layer what state is the selected applet in,Java layer will invoke the selecte() or
						//multiselectabel.select() according to the state condition.
						PXB_W_U8(&bRamBuffer[SELECT_STATE], MultiSelResult);

						// set selected applet
						PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], SELECT_COMMAND_FLAG);
						PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], i);

						closeSecuritychannel(SC_TERMINATED);// modify by liweimin

					#if (EUICC_ONLY||SE_AND_EUICC)
						setNextProfileIndex(G_CurExecutingProfileIndex);
						G_CurExecutingProfileIndex = bakCurProfile;
						ExecuEnvironmentInit(G_CurExecutingProfileIndex);
					#endif

						return; // Applet found, if the applet's state is APPLET_SELECTABLE, then the G_CurExecutingProfileIndex need not restore
					}
					else if(U8_AND_U8(bState_, 0x83) == APPLET_LOCKED)
					{
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FILE_NOT_FOUND); //

					#if (EUICC_ONLY||SE_AND_EUICC)
						setNextProfileIndex(bakCurProfile);
						G_CurExecutingProfileIndex = bakCurProfile;
						ExecuEnvironmentInit(G_CurExecutingProfileIndex);
					#endif

						return; // Applet found, if the applet's state isn't APPLET_SELECTABLE, then the G_CurExecutingProfileIndex need restore
					}
					else if(U8_AND_U8(bState_, 0x03) == APPLET_INSTALLED)
					{
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FILE_NOT_FOUND);

					#if (EUICC_ONLY||SE_AND_EUICC)
						setNextProfileIndex(bakCurProfile);
						G_CurExecutingProfileIndex = bakCurProfile;
						ExecuEnvironmentInit(G_CurExecutingProfileIndex);
					#endif

						return; // Applet found, if the applet's state isn't APPLET_SELECTABLE, then the G_CurExecutingProfileIndex need restore
					}
					else
					{
						continue; // continue to search other applet
					}
				}
			}
		}

		//when the select command is fail,the security channel which bulid by the selected SD is also exist
		//setSecurityLevel(selected_channel ,CardSecurityState);
		if(MultiSelResult == MUL_SEL_RESTRICTION)
		{
			if(OS_CONF3_TCK_ICC_MODE == OS_TCK_MODE)
			{
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_CONDITIONS_NOT_SATISFIED);	//// for TCK test
			}
			else
			{
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FILE_NOT_FOUND);		 // for ICC test
			}

		#if (EUICC_ONLY||SE_AND_EUICC)
			setNextProfileIndex(bakCurProfile);
			G_CurExecutingProfileIndex = bakCurProfile;
			ExecuEnvironmentInit(G_CurExecutingProfileIndex);
		#endif

			return; // Applet found, if the applet does not allowed MultiSelectable, then the G_CurExecutingProfileIndex need restore
		}

		if (bPreAppID_ == 0)
		{
			// if Pre applet is card manager , throw error
			if((bSwitchProfile == false) || (loopStep == 1)) //只能循环结束跳出
			{
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FILE_NOT_FOUND);
			}
		}

		if (bP2_ == SELECT_NEXT || bP2_ == SELECT_PREV) //
		{
			// next mode ，
			// If no matching Application is found and the value in P2 indicates next occurrence ('02'), a response of '6A82' shall be returned.		
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FILE_NOT_FOUND);	
			return;
		}
		// otherwise go to pre applet to process.
	}

#if (EUICC_ONLY||SE_AND_EUICC)
	setNextProfileIndex(bakCurProfile);
	G_CurExecutingProfileIndex = bakCurProfile;
	ExecuEnvironmentInit(G_CurExecutingProfileIndex);
#endif

}

/**
 * to get start app index
 * return val: * true: get start, false:not found
 */
u8 selectGet_CurIndex(PX8 bStartIndex_)
{
	u8			D_A			 bPreAppID_;
	u8			D_A			 bLc_;
	jref		D_A			 aid;

	bLc_		= PX_R_U8(&APDUBuffer[OFFSET_LC]);
	bPreAppID_  = U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF);

	PX_W_U16(bStartIndex_, APP_FIRST);
	
    if(bLc_ != 0x00)
    {
		if(bPreAppID_ == 0xFF)
		{
			//PX_W_U16(bStartIndex_, getIndexByID(APP_FIRST));
		}
		else
		{
			PX_W_U16(bStartIndex_, getIndexByID(bPreAppID_));
		}

		aid = getAppletFiled(PX_R_U16(bStartIndex_ ),GET_AID_ARRAY);

		if (!AID_partialEquals_array(aid, MEMtPF8(PXtMEM(APDUBuffer)), OFFSET_CDATA, U8t16(bLc_)))
		{
			return false;// not found
		}
    }
//	else
//	{
//		PX_W_U16(bStartIndex_, APP_FIRST);
//	}

	return true;
}
#if JAVA_CARD_UICC	 // modify by xuxiaojun@20220318 分层
boolean isUSIMApplet(void)
{
	boolean rev;

	u8 curren_appid = U16t8(U16_AND_U16(getChannelContext(selected_channel), APPLET_MASK));

//	if ((U32t8(0xff) != curren_appid) && (curren_appid >= (U32t8(0x80))))//  need to modify
	if (curren_appid == UICC_APPID)
	{
		rev = true;
	}
	else
	{
		rev = false;
	}

	return rev;
}
#endif
/**
 * before loop, get three parameters below： *start *end *direction
 *
 *source:  init value
 *output: bStartIndex_, bEndIndex_, direction
 *return val: direction
 */
s8 select_init(PX8 bStartIndex_, PX8 bSearchProfile, PX8 bSwitchProfile)
{
	u8			D_A			 bP2_,bP3_;
	s8			D_A			 direction;
	u8			D_A			 result;
#if (EUICC_ONLY||SE_AND_EUICC) // modify by xuxiaojun@20220609 warning
	u8          D_A          bPreProfileIndex;
#endif
	bP2_			= PX_R_U8(&APDUBuffer[OFFSET_P2]);
	bP2_ 			= U8_AND_U8(bP2_, SELECT_MODE_MASK);
	result			= false;
	bP3_            = PX_R_U8(&APDUBuffer[OFFSET_LC]);

#if (EUICC_ONLY||SE_AND_EUICC)
	bPreProfileIndex = getChannelProfileIndex(selected_channel);

	PX_W_U8(bSwitchProfile, false);

	PX_W_U8(bSearchProfile, bPreProfileIndex);
#endif

	if (bP2_ 		== SELECT_FIRST) // search from SE to enabled profile
	{ // first mode

#if (EUICC_ONLY||SE_AND_EUICC)
		G_CurExecutingProfileIndex = ILLEGAL_PROFILEINDEX;

		if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)
		{
			if(bP3_ == 0)//AID为空 在当前激活的profile下选择,其余情况优先选择SE区
			{
				G_CurExecutingProfileIndex = G_CurEnabledProfileIndex;
			}
			else
			{
				PX_W_U8(bSwitchProfile, true);
			}
		}

		ExecuEnvironmentInit(G_CurExecutingProfileIndex);
		PX_W_U8(bSearchProfile, G_CurExecutingProfileIndex);
#endif

		PX_W_U16(bStartIndex_, GetAppEntryIdxByPriority(APP_INDEX_NULL, 1));

		result 		= true;

		direction	= SEARCH_NEXT;
	}
	else if (bP2_ 	== SELECT_NEXT) // search within the preSelected Profile
	{ // next mode
		direction	= SEARCH_NEXT;

		result 		= selectGet_CurIndex(bStartIndex_);

		if(result != false )
		{
			if(bP3_ == 0)
			{
				//PX_W_U16(bStartIndex_ , APP_FIRST); // selectGet_CurIndex(bStartIndex_)里已经将bStartIndex_写成APP_FIRST
			}
			else
			{
				PX_W_U16(bStartIndex_ , GetAppEntryIdxByPriority(PN_R_U16(bStartIndex_ ), direction));
			}
		}
		else
		{
			direction = SEARCH_ERROR; // not found
		}

	}
	else if (bP2_ 	== SELECT_PREV) // search with in preSelected Profile
	{ // previous
		direction	= SEARCH_PREVIOUS;

		result 		= selectGet_CurIndex(bStartIndex_);

		if (result != false )
		{
			if(bP3_ == 0)
			{
				//PX_W_U16(bStartIndex_ , APP_FIRST); // selectGet_CurIndex(bStartIndex_)里已经将bStartIndex_写成APP_FIRST
			}
			else
			{
				PX_W_U16(bStartIndex_, GetAppEntryIdxByPriority(PN_R_U16(bStartIndex_ ), direction));
			}
		}

		// jcre/channel/selectfile004/SelectFile004Applet.html; ask cos to pass this command to current app int this channel.
		
		#if JAVA_CARD_UICC // modify by xuxiaojun@20220318 分层
		else if (isUSIMApplet() == false)
		{
			direction = SEARCH_PASS_TO_CURRENT_APP;
		}
		#endif
		else
		{
			direction = SEARCH_ERROR;
		}
	}
	else
	{ //last mode go for usim
		direction 	= SEARCH_LAST;
		result 	 	= true;
	}

	if ((true != result) && (SEARCH_ERROR == direction))//todo isd
	{
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FILE_NOT_FOUND);
	}

    return direction;
}
/*获取UICC APP的APPID, 后续可以考虑写入配置区 优化性能
* modify by xuxiaojun@20220315 从platform_api.c中移动到该文件，原因是该函数只在该文件中被调用
*/

u16 getAppIDByIndex(u8 index)	SMALL_MODE
{

	PN8			D_A			theAppTable;
	PN8			D_A			entry;

	u8			D_A			bAppID;
	theAppTable			= MEMtPN8(getAppTable());
	
	entry			= MEMtPN8(locateObjectInMemory(nvmReadU16(PN8_A_U16(theAppTable, U16_M_U16(index, SIZE_REF)))));
	bAppID			= memReadU8(get_object_field_ref(entry, bAppID_INDEX));
	
	return bAppID;
}
/*
 * modify by lixin@20210816： OPEN_CHANNEL执行成功之后，需要给g_bSDId[selected_channel]赋上正确的值。 https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
 * modify by zhxx@20220517：同步缺陷系统379bug，channelManagerCommand函数中getAppIDByIndex(theAppletIndex)访问地址非法
 */
 void channelManagerCommand(void)
{
	u8		D_A		newChannel;
	u8		D_A		cmdChannel;
	u8		D_A		opType;
	u8		D_A		managedChannel;
	u8		D_A		channelStatus;
	u8		D_A		bCardState;
	u8		D_A		theContext;
	u16		D_A		theAppletIndex;
	u8		D_A		MultiSelResult;
	jref	D_A		theApplet;
	PN8		D_A		selectAppletTable;
	u16		D_A		selectAppletRef;
#if JAVA_CARD_UICC // modify by xuxiaojun@20220609 warning
	u8      D_A     bakChannel;
#endif
	u8      D_A     baseChannel;
#if (EUICC_ONLY||SE_AND_EUICC) // modify by xuxiaojun@20220609 warning
	u8      D_A     profileIndex;
#endif
	u8		D_A		maxChannel;
	u8 		D_A		privilege[3];
	u8		D_A		theAppId;
	
	newChannel		= 0xFF;
	cmdChannel		= selected_channel;
	opType			= PX_R_U8(&APDUBuffer[OFFSET_P1]);
	managedChannel	= PX_R_U8(&APDUBuffer[OFFSET_P2]);

	MultiSelResult	= 0;

	bCardState = JcvmConfig_GetCardState();
	if (bCardState == CARD_TERMINATED)
	{
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INS_NOT_SUPPORTED);
		return ;
	}

	if ((bCardState == CARD_LOCKED)) {
		//if (0x01 == getConfig(GP_CONFIG_FOR_BFC))
		{
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPRORT);
		}
//		else
//		{
//			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INS_NOT_SUPPORTED);//GP功能补充测试脚本_接触_工程集/天喻/通用要求/6.1.1.38.13  CARD_LOCK状态 发关闭通道指令 期望反6A81
//		}
		return ;
	}

	if((opType != P1_CLOSE_CHANNEL) && (opType != P1_OPEN_CHANNEL))
	{
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPORTED);
		return ;
	}

	if(managedChannel <= 3)
	{
		if(U8_AND_U8(PX_R_U8(&APDUBuffer[OFFSET_CLA]), APDU_CLA_B3B4_MASK) != 0)
		{
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_SECURE_MESSAGEING_NOT_SUPPORTED);
			return ;
		}
	}
	else if(managedChannel <= 19)
	{
		if(U8_AND_U8(PX_R_U8(&APDUBuffer[OFFSET_CLA]), APDU_CLA_B6_MASK) != 0)
		{
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_SECURE_MESSAGEING_NOT_SUPPORTED);
			return ;
		}
	}
	else
	{
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPORTED);
		return ;
	}

//	if (logicalChannelNums == 1) {
//		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
//		return;
//	}
	if(getChannelState(cmdChannel) == CHANNEL_CLOSED){
	   PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
	   return;
	}       
	//merged from CIU9872B_01@20220926
	maxChannel  = getIoMaxChannel();
	baseChannel = getIoBaseChannel();
//	if (maxChannel == 1)
//	{
//		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
//		return;
//	}
	
	if ((managedChannel >= maxChannel)) // 如果是要打开的通道号超过了卡片支持的通道个数，抛6A86; 如果是要关闭的通道号超过了卡片支持的通道个数，抛6A81. modify by xuxiaojun@20211208,验证了电信后面注释的用例，测试通过
	{
		if(opType == P1_CLOSE_CHANNEL)
		{
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPRORT);//GP功能补充测试脚本_接触_工程集/东信/apdu命令要求/manage channel指令测试/6.1.2.10.6  关闭未打开和不存在的通道的异常测试
		}
		else
		//电信测试
		{
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2); //JCRE_additional_test_Contacted_工程集\Applet_Selection_Command_Dispatch_Contacted_SECURED\JCRE_Applet_Selection_Command_Dispatch_Contacted08_07_01
		}
		return;
	}
	
	managedChannel += baseChannel;

	channelStatus = getChannelState(managedChannel);
	
	switch(opType){
		  case P1_CLOSE_CHANNEL:
		  {
			// Basic channel cannot be closed
			if (managedChannel == baseChannel)
			{
				  PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPORTED);
				  return;
			}                

			if (APDUBuffer[OFFSET_LC] != 0)
			{
				//bShortBuffer[VM_EXCEPTION] = htoj16(SW_WRONG_LENGTH);
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_WRONG_LENGTH);
				return;
			}

			// If channel is open, close it
			if (U8_AND_U8(channelStatus, CHANNEL_OPEN_MASK) == CHANNEL_OPEN) {
				
				// Deselect applet in channel
				//PrivAccess.deselectOnly(managedChannel);
				PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], CLOSE_CHANNEL);
				selected_channel					= managedChannel;
				// Close channel
				channelManage(managedChannel, OP_CHANNEL_CLOSE);
				 //Close security channel
				 closeSecuritychannel(SC_TERMINATED);// modify by liweimin

			#if (EUICC_ONLY||SE_AND_EUICC)
				#if RSP_EUICC
				 RSP_CloseChannel(managedChannel);
				#else
				 profileIndex = getChannelProfileIndex(managedChannel);

				 if((G_CurExecutingProfileIndex != profileIndex)&&(getChannelContext(managedChannel)!=NULL_CONTEXT))
				 {
					 G_CurExecutingProfileIndex = profileIndex;
					 ExecuEnvironmentInit(G_CurExecutingProfileIndex);// 关通道 需要调用desect方法
				 }
				 setNextProfileIndex(ILLEGAL_PROFILEINDEX); //clear
				 #endif
			 #endif

			} else {
				// If channel is closed, throw a warning
				 PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_WARNING_STATE_UNCHANGED);
				 return;
			}                
			break;
		  }
		case P1_OPEN_CHANNEL:
		{
			//if (managedChannel == P2_AUTOSELECT_CHANNEL) {             
			//由于前面对SWP接口过来的通道号进行特殊处理，此处必须取P2的值
			if (APDUBuffer[OFFSET_P2] == P2_AUTOSELECT_CHANNEL) // 获取一个可用的通道
			{
				// In autoselect, let Java Card runtime environment choose channel automatically
			   
				newChannel = channelManage(0xFF, OP_CHANNEL_OPEN_AUTOSELECT); // todo 返回值是u16的FFFF，newChannel是u8的FF

				if (newChannel == ERR_NO_CHANNEL_AVAILABLE) {
					if(CONFIG_GP_FOR_BFC || CONFIG_GP_FOR_MAPPING_TEST || CONFIG_GP_FOR_CI)//merged from EPX\v2.1\BRANCH_MODULE\FINANCE_TMC_THD89： CI: cm_APDU_manageChannel_12_86_75_8a_61_17
					{
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);//gpBFCCompliance208: cm_APDU_manageChannel_OPR_12_02
					}
					else
					{
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPRORT);
					}
					return;
				}
		
			} else { //打开指定的通道

				//because UICC open channel command P2 must equal 0 ;it corupts with tck/jcre/channel/managechannel002/ManageChannel002Applet.html
				//按照GP标准，0070000100（指定通道号）是可选实现的，移动的脚本是要求支持这种模式的，因此去掉以下限制,by xwh 20200717
				//if ((isUSIMApplet() == true) && (0 == selected_channel))
				//{
				//	PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2);
				//	return;
				//}

				if (APDUBuffer[OFFSET_LC] != 0) //联通EUICC测试\RSP eUICC基本指令测试V2.1.script P2不为0.P3不为0 要抛异常  by chentao20200728
				{
					PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2);
					return;
				}

				// Check channel availability and open it
				if ((channelManage(managedChannel, OP_CHANNEL_OPEN)) != 0xFFFF)
				{
					newChannel = managedChannel;
				}
				else
				{
					PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2);
					return;
				}
			}
			PXB_W_U8(&bRamBuffer[PRE_SEL_CHANNEL], selected_channel);

			selected_channel = newChannel;
			//GetImplicitSelectedAppletId

			if (cmdChannel == baseChannel) { // 在基本逻辑通道上执行的操作
				// If managing channel is the basic channel, select the
				// default applet in the newly open channel.
				// The command channel is passed to allow other implementations
				// to select a different default applet according to the
				// selection channel.
#if (EUICC_ONLY||SE_AND_EUICC)
				G_CurExecutingProfileIndex = G_CurEnabledProfileIndex;//在激活的profile或SE下选默认应用 只选一个区域

				ExecuEnvironmentInit(G_CurExecutingProfileIndex);//0070xx上的执行区域可能是SE ，所以需要切换G_CurExecutingProfileIndex以便后续查找apptable等

				setNextProfileIndex(G_CurEnabledProfileIndex); //initialize with G_CurEnabledProfileIndex
#endif
				theAppletIndex = GetImplicitSelectedAppletId(nativeMethod_GetProtocol(),selected_channel);

				if(theAppletIndex == APP_INDEX_NULL)
				{
				    // modify by xuxiaojun@20221123: 需要兼容se和euicc两种情况，所以这里应该实用函数接口
					theAppletIndex = JcvmConfig_GetdefaultAppID();//modify by xuxiaojun@20221122: 代码还原到modify by xuxiaojun@20220420 GP_CI_Compliance: cm_APDU_manageChannel_84_70_6a_11_9f_fe之前，原因是PrivAccess.defaultAppID存储的是AppTable Index，而不是App ID
				}

#if JAVA_CARD_UICC				//新通道上的应用是UICC APP，才调用SetChannel，否则不需要调用
				if(getAppIDByIndex(theAppletIndex) == GetUICCAppId())
				{	
					if(apdu_GetProtocol() == PROTOCOL_SWP)
					{
						theAppletIndex = MNOSD_APPID;
					}
					else
					{
						bakChannel = G_ChannelNum;

						G_ChannelNum = selected_channel;

						SetChannel(E_FILE_START,0x3F00,FALSE);

						G_ChannelNum = bakChannel;
					}
					// SET SELECTED APPLET
				}
#endif				
			}
			else  // 在补充逻辑通道上执行的操作
			{
				// If managing channel is not the basic channel,
				// select the currently selected applet in the managing
				// channe
#if (EUICC_ONLY||SE_AND_EUICC)
				setNextProfileIndex(getChannelProfileIndex(cmdChannel)); //initial profile correspondence with cmdChannel's profile
#endif
				theContext			= getChannelContext(cmdChannel);
				theAppletIndex		= getIndexByID(theContext);  

#if JAVA_CARD_UICC				
				//新通道上的应用是UICC APP，才调用SetChannel，否则不需要调用
				if(getAppIDByIndex(theAppletIndex) == GetUICCAppId())
				{
				/*if((G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)&&(G_CurEnabledProfileIndex != G_CurExecutingProfileIndex))//在选中了SE应用的通道上发开通道指令 且选中了UICC为默认应用
					{
						G_CurExecutingProfileIndex = G_CurEnabledProfileIndex;//选中UICC应用时 指向激活的profile

						ExecuEnvironmentInit(G_CurExecutingProfileIndex);
					}//此种情况不存在，当前发0170xxxx的通道上应用是UICC时 ，G_CurExecutingProfileIndex一定指向profile
				*/
					bakChannel = G_ChannelNum;
					G_ChannelNum = selected_channel;

					SetChannel(G_CurrentDfAddr[cmdChannel], G_CurrentDfName[cmdChannel], FALSE);	//发出命令的逻辑通道的当前DF应被选为当前DF
					G_CurrentAdfName[G_ChannelNum] = G_CurrentAdfName[cmdChannel];

					G_ChannelNum = bakChannel;
				}
#endif				
				/*@ -disspecopt @*/selectAppletTable	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPN8(AM_SelectedApplet_JC))))));/*@ +disspecopt @*/
				selectAppletRef 	= memReadU16(PN8_A_U16(selectAppletTable, U8_M_U8(cmdChannel, SIZE_REF))); 
				if(!selectAppletRef) // 从理论上来说，selectAppletRef与theAppletIndex是同一个应用
				{
				   PXB_W_U8(&bRamBuffer[PRE_SEL_CHANNEL], selected_channel);

				   selected_channel = newChannel;
				   // abort transaction if applet forgot to commit it
					if (isInProgress()) 
					{
						abortTransaction();
					}
				   // no selected applet if select returns false or throws exception.
					setChannelContext(newChannel, JCRE_CONTEXTID, false);
					memWriteU16Atomic(MEMtPM8(PN8_A_U16(selectAppletTable, U8_M_U8(newChannel, SIZE_REF))), REF_NULL);
					// In case of open auto-select successful, return channel information
					if (APDUBuffer[OFFSET_P2] == P2_AUTOSELECT_CHANNEL)
					{
						//merged from CIU9872B_01@20220926
						PX_W_U8(&APDUBuffer[0], (selected_channel - baseChannel));

						apdu_ApduSetOutgoingAndSend(0,1);
					}
					PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_NO_ERROR);
					
					break;
				}
			}

			getAppTable();
			APP_Entry_Object_Cash	= locateObjectInMemory(nvmReadU16(PN8_A_U16(MEMtPN8(APP_Table_Cash), U16_M_U16(theAppletIndex, SIZE_REF))));
			theApplet				= memReadU16(get_object_field_ref(MEMtPN8(APP_Entry_Object_Cash), theApplet_INDEX));

			MultiSelResult			= MultiSelectState(theApplet, newChannel);
			if(MultiSelResult == MUL_SEL_RESTRICTION)
			{
				channelManage(newChannel, OP_CHANNEL_CLOSE);
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_CONDITIONS_NOT_SATISFIED);
				break;
			}
			//inform the Java Layer what state is the selected applet in,Java layer will invoke the selecte() or 
			//multiselectabel.select() according to the state.
			PXB_W_U8(&bRamBuffer[SELECT_STATE], MultiSelResult);
			PXB_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG], OPEN_CHANNEL);
			PXB_W_U8(&bRamBuffer[SELECTING_APPLET_INDEX], theAppletIndex);

			theAppId = getAppIDByIndex(theAppletIndex); // 如果是通过theAppletIndex		= nvmReadU16(MEMtPN8(PA_DefaultAppID_JC));获取到结果，此时theAppletIndex本身就是theAppId
			getPrivilegeByAppId(theAppId, &privilege[0]);
			if(U8_AND_U8(privilege[0] , 0x80) == 0x80) //SD
			{
				g_bSDId[selected_channel] = theAppId; //测试用例:联通测试的log\2-1-manage channel测试\F2_TC6_ISDAMSSDTest
			}
			else
			{
				g_bSDId[selected_channel] = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(getEntryByAppId(theAppId)), bsdId_INDEX))); //测试用例:联通测试的log\2-1-manage channel测试\F2_TC6_ISDAMSSDTest
			}

			// In case of open auto-select successful, return channel information
			if (APDUBuffer[OFFSET_P2] == P2_AUTOSELECT_CHANNEL) {
				//merged from CIU9872B_01@20220926
				PX_W_U8(&APDUBuffer[0], (selected_channel - baseChannel));
				
				apdu_ApduSetOutgoingAndSend(0,1);
				break;
			}
			break;
		}
			//lint -fallthrough
		default:
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_FUNC_NOT_SUPPORTED);
			return;
	}

}

void Dispatcher1()
{
	u8		D_A		bCardState;
	u8		D_A		receiveCmdData;
	u8				preSelectedChannel;
	u16				status;
#if JAVA_CARD_UICC // modify by xuxiaojun@20220609 warning
	u8      D_A     channelContext;
#endif
	u8      D_A     backFlag;
	u8      X_A     backAID[0x11];
#if (EUICC_ONLY||SE_AND_EUICC) // modify by xuxiaojun@20220609 warning
	u8      X_A     backChannelProfileId;
#endif
	u8      D_A     logicalChannel;
	u8      D_A	    maxChannel;
	u8      D_A	    baseChannel;
	//u8 		D_A		backupChannel;

	PX_W_U8(&bRamBuffer[MAX_LOGICAL_CHANNEL_NUMBER],logicalChannelNums);
	PX_W_U8(&bRamBuffer[SELECTING_APPLET_FLAG],0xFF);
	PX_W_U8(&bRamBuffer[PROCESS_METHOD_FLAG],0x00);
	PX_W_U8(&bRamBuffer[SELECT_STATE],0x00);

	status  	=   popShort();
	EncryRMACResponseData(status);
	apdu_SendStatusAndReceiveCommand(status);
#if JAVA_CARD_UICC	
	G_NextAPDUFlag = FALSE;
#endif	
	receiveCmdData = FALSE;
	preSelectedChannel	= selected_channel;
	selected_channel	= apdu_GetLogicalChannel();
	//merged from CIU9872B_01@20220926
	baseChannel = getIoBaseChannel();
	maxChannel  = getIoMaxChannel();

	logicalChannel	= apdu_GetLogicalChannel();

	if(logicalChannel >= maxChannel)
	{
		//throw_error(SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
		throw_error(SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
		return;
	}
	
	selected_channel = baseChannel + logicalChannel;



	if(OS_CONF3_TCK_ICC_MODE != OS_TCK_MODE)	//for ICC test
	{
		if((U8_AND_U8(APDUBuffer[OFFSET_CLA], 0xF0) == 0xE0) || (U8_AND_U8(APDUBuffer[OFFSET_CLA], 0xF0) == 0xF0))
		{
			selected_channel	= 0;
		}
	}
#if VM_WITH_PBOC_API
	receiveCmdData = PbocAPI_CallPboc(APDUBuffer);
#endif

	PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]),0x00);

#if GP__VERSION__FOR_CHINA_MOBILE_SWP1_0_MASK
	if (moreDataFlag[selected_channel])
	{
		if (!(APDUBuffer[OFFSET_INS] == 0xCA && (APDUBuffer[OFFSET_P1] == 0x2F) && (APDUBuffer[OFFSET_P2] == 0x01)))
		{
			moreDataFlag[selected_channel] = 0;
		}
	}
	else
	{
		handle_2F00Tag[selected_channel] = 0;
	}
#endif

#if (EUICC_ONLY||SE_AND_EUICC)
	G_CurExecutingProfileIndex = getChannelProfileIndex(selected_channel); //selected_channel发生变化时，都需要执行该语句
	ExecuEnvironmentInit(G_CurExecutingProfileIndex);
	backChannelProfileId = G_CurExecutingProfileIndex;//用于还原当前指令的通道执行环境 ，调用deselect之前不能切
#endif

#if GP__DATA_MANAGER__FOR_MASK
	dataManager_getBitTag(0xFE);
	if(U8_AND_U8(handle_2F00Tag[selected_channel] , 0x80) == 0x80)  //dataManager模块get data2F00级联过程插入其他指令处理
	{
		U8_INC(handle_2F00TagLink[selected_channel]);
	}
	else
	{
		handle_2F00TagLink[selected_channel] = 0;
		if(0x04 != dataManager_getBitTag(0x04))
		{
				dataManager_getBitTag(0xfd);
		}
	}
#endif

	scpClr();
	PXB_W_U16(&bShortBuffer[VM_EXCEPTION], 0);

#if GP__CGM__FOR_APPPARACHECK
//linguoming:config C7C8
	CgmInit_changeFlag(CHECK_RUN);
#endif

	bCardState = JcvmConfig_GetCardState();

	if ((bCardState == CARD_TERMINATED) && (APDUBuffer[OFFSET_INS] != INS_OP_GET_DATA))
	{
		//bShortBuffer[VM_EXCEPTION] = htoj16(SW_INS_NOT_SUPPORTED);
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INS_NOT_SUPPORTED);
		return;
	}

	// Filter channel information to detect ISO APDUs
	if (U8_AND_U8(PXB_R_U8(&APDUBuffer[OFFSET_CLA]), APDU_CMD_MASK) == CLA_ISO7816)
	{
		// SM bits are equal to 00
		switch (PXB_R_U8(&APDUBuffer[OFFSET_INS]))
		{
			// ISO 7816-4 SELECT FIlE command
			case INS_SELECT:
			{
				//JCRE 2.2.2 4.5.2: check non-zero secure messaging bits in the CLA
				if(logicalChannel <= 3)
				{
					if(U8_AND_U8(PXB_R_U8(&APDUBuffer[OFFSET_CLA]), APDU_CLA_B3B4_MASK) != 0)
					{
						if (getChannelState(selected_channel) == CHANNEL_CLOSED)
						{
							selected_channel	= preSelectedChannel; //这种情况下是否需要切换当前正在执行的profile?

	#if GP__VERSION__FOR_CHINA_UNICOM_MASK
							if (isUSIMApplet() == true)
							{
								PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_CLA_NOT_SUPPORTED);
							}
							else
	#endif
							{
								PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
							}
						}
						return;
					}
				}
				else if(logicalChannel <= 19)
				{
					if(U8_AND_U8(PXB_R_U8(&APDUBuffer[OFFSET_CLA]), APDU_CLA_B6_MASK) != 0)
					{
						if (getChannelState(selected_channel) == CHANNEL_CLOSED)
						{
							selected_channel	= preSelectedChannel; //这种情况下是否需要切换当前正在执行的profile?
	#if GP__VERSION__FOR_CHINA_UNICOM_MASK
							if (isUSIMApplet() == true)
							{
								PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_CLA_NOT_SUPPORTED);
							}
							else
	#endif
							{
								PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
							}
						}
						return;
					}
				}

				if ((PXB_R_U8(&APDUBuffer[OFFSET_P1]) == P1_SELECT_DFBYNAME) || ((0x00 == APDUBuffer[OFFSET_P1]) && (0x02 == APDUBuffer[OFFSET_LC])))
				{
					if((getChannelState(selected_channel)!= CHANNEL_OPEN) && ((0x00 == APDUBuffer[OFFSET_P1]) && (0x02 == APDUBuffer[OFFSET_LC])) )
					{
						selected_channel	= preSelectedChannel;
						PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
						return;
					}

					//AppAPI里面的AppAPI_SelectByFileId函数AppAPI_SelectByFileId会实现2字节file ID到AID的映射，此功能暂时不需要，因此这里返回,by xwh 20191127
					if((0x00 == APDUBuffer[OFFSET_P1]) && (0x02 == APDUBuffer[OFFSET_LC]))
					{
						return;
					}

					//TODO 因为0x44的情况暂时注释掉的
					if (false == receiveCmdData)
					{
						apdu_ApduSetIncomingAndReceive();
					}
//(UICC__OPERATOR__CHINAMOBILE || UICC__OPERATOR__CHINATELECOM)

#if UICC__APP__WHITECARD
					if(IsWhiteCardApplet(&APDUBuffer[OFFSET_CDATA]) == TRUE)
					{
						if((U8_AND_U8(APDUBuffer[OFFSET_P2],0xF3) != 0x40)&&(U8_AND_U8(APDUBuffer[OFFSET_P2],0xF3) != 0x10)&&(U8_AND_U8(APDUBuffer[OFFSET_P2],0xF3) != 0x00))//白卡入网-APP写卡-空卡不预置KOPC/07parameter
						{
							PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2);
							return ;
						}
					}
#endif

					if((U8_AND_U8(APDUBuffer[OFFSET_P2],0x01) != 0)||(U8_AND_U8(APDUBuffer[OFFSET_P2],0xA0) != 0)) //7816-4 Table 40 — P2,
					{
						if(U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF) == APP_FIRST)
						{
							PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INCORRECT_P1P2);//移动备案  GP功能补充测试脚本_接触_工程集/东信/apdu命令要求/select指令测试/6.1.2.9.3   为什么这么限制没有找到规范来源，限制条件与移动5G备案平台保持一致
						}
						return;//移动 /电信5G    uicc_configuration_additional_test_工程集/SECURED/APDUADDITIONALTEST/apdu_select_test_1_1/步骤21、发送SELECT命令，P1=‘04’，P2=‘01’，指定选择ISD；  期望分发给上一次选中的应用 状态字为应用的状态字9B01
					}


					backFlag = FALSE;

					if(OS_CONF7_TCK_BCTC_MODE)
					{
						if((/*(PXB_R_U8(&APDUBuffer[4]) >= 0) && */(AIDLenInvalid(PXB_R_U8(&APDUBuffer[4])))))
						{
							PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_WRONG_LENGTH);
							return;
						}
					}
					bRamBuffer[SELECTING_APPLET_INDEX] = 0;

					bRamBuffer[SD_REQUESTED_SECURITY_LEVEL] = 0;
					managerRequestedChannelLevel(SCF_NO_AUTHENTICATED) ;
					selectApplet();
					apdu_UndoSetIncomingAndReceive();

#if JAVA_CARD_UICC						//if((PXB_R_U8(&bRamBuffer[SELECTING_APPLET_INDEX]) == 0)&&(PXB_R_U8(MEMtPX8(&bRamBuffer[SELECTING_APPLET_FLAG]) != SELECT_COMMAND_FLAG)))//选应用不成功  尝试选择ADF
					if((bRamBuffer[SELECTING_APPLET_INDEX] == 0)&&(bRamBuffer[SELECTING_APPLET_FLAG] != SELECT_COMMAND_FLAG))
					{					
						if((APDUBuffer[4]!=0)&&(true == AppAPI_SelectByFileId(APDUBuffer, MEMtPX8(&bShortBuffer[VM_EXCEPTION]))))
						{
							channelContext = getChannelContext(selected_channel);

							if(U8_AND_U8(U16t8(channelContext),0xFF) == UICC_APPID)//当前通道上的app是UICC 选中ADF后直接分发给UICC, 这里是否需要区分SE的UICC和profile的UICC?
							{
								return ;
							}
							//others 通道上无应用 或 通道上不是UICC
							backAID[0] = APDUBuffer[4];//中国电信5G卡基本通信功能/5-2_SUCI_Test/3.TC_Error_Not_Select_USIM_6985用例有测到
							ecpMem_Copy(&backAID[1],&APDUBuffer[5],APDUBuffer[4]);//备份收到的ADF 的AID LV，地址需要4字节对齐

							IF_SelectADFSucess(selected_channel);//该函数内部会把UICC AID拷贝到APDUBuffer 用于后续选中UICCAPP

							backFlag = TRUE;

							PXB_W_U16(&bShortBuffer[VM_EXCEPTION], 0);
							selectApplet();//选中ADF时   将UICC选中为通道上的应用
						}
					}
#endif
#if (EUICC_ONLY||SE_AND_EUICC)
					if(PXB_R_U8(&bRamBuffer[SELECTING_APPLET_INDEX]) == UICC_APPID)//UICC 的index和appid都是同一个 这里直接用appid代替index
					{
						if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)//UICC在有profile激活的情况下 仅访问profile下的文件系统 ，文件系统涉及的AID（如Toolkit应用）全部基于当前激活的profile来查找
						{
							G_CurExecutingProfileIndex = G_CurEnabledProfileIndex;

							ExecuEnvironmentInit(G_CurExecutingProfileIndex);//IF_SelectSuccessJavaApp需要在激活的文件系统下查找 这里先切过去

							setNextProfileIndex(G_CurExecutingProfileIndex);
						}
					}
#endif

					if(backFlag == TRUE)
					{
						APDUBuffer[4] = backAID[0];////中国电信5G卡基本通信功能/5-2_SUCI_Test/3.TC_Error_Not_Select_USIM_6985用例有测到，地址需要4字节对齐
						ecpMem_Copy(&APDUBuffer[5],&backAID[1],backAID[0]);
					}

#if UICC__APP__WHITECARD
					if(PXB_R_U16(&bShortBuffer[VM_EXCEPTION]) == 0)//选择成功
					{
						G_AID_tag = 0xFF; //清成FF 如果选中的是白卡写网应用 会在应用里重新赋值
					}
#endif
					if((PXB_R_U16(&bShortBuffer[VM_EXCEPTION]) == 0)&&(bRamBuffer[INSTALL_FOR_PERSONERLIZE] != 0))//选择成功
					{
						//gp221  :11.11.1 Definition and Scope :  A personalization session ends when:
						//1  ：The card is reset;
						//2 ：The Security Domain is deselected (i.e. another, or the same, applet is selected on the same logical channel);
						//3 ：The Security Domain is selected on the same or another logical channel;（包含开通道隐式选择的SD？）
						//4 ：The Secure Channel session (if any) established by the Security Domain is reset (另一个应用在该通道被选中，逻辑通道被关闭，卡reset)
						//5 ：The Security Domain receives an INSTALL [for personalize] command (starting a new personalization session for another application);   在opInstallForPersonalize函数中处理
						//6 ：The Security Domain receives a STORE DATA command indicating P1.b8=1 (last block);   在personerlizeApplication函数处理
						// reset的情况不需要处理   2,3,4 在选择指令里处理

						if(ValueBit(bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL],selected_channel) == 1)//对应2  ，4   任何一个应用在personal的通道被选中 中断应用个人化
						{
							bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
							bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = 0;
							g_storeDataSequence[selected_channel] 			= 0;
						}
//						else if(getAssociateSDId(bRamBuffer[INSTALL_FOR_PERSONERLIZE]) ==  getAppIDByIndex(bRamBuffer[SELECTING_APPLET_INDEX]))// 对应 3 执行installforpersonal的SD在任何一个通道被选中
//						{
//							bRamBuffer[INSTALL_FOR_PERSONERLIZE] = 0;
//							bRamBuffer[INSTALLFORPERSON_SELECT_CHANNEL] = 0;
//							g_storeDataSequence[selected_channel] 			= 0;
//						}
					}
//					if(PXB_R_U16(&bShortBuffer[VM_EXCEPTION]) == 0)//选择成功
//					{
//						//这个判断是因为SE下没有文件系统时会出问题才加的，现在可以去掉了,by xwh 20200602
//						//if(G_CurExecutingProfileIndex == JcvmConfig_GetProfileEnableIndex() )
//						{
//							IF_SelectSuccessJavaApp(selected_channel, &APDUBuffer[5], APDUBuffer[4]); //卡片第一次上电时文件系统还未创建
//						}
//					}
				}

				#if (EUICC_ONLY||SE_AND_EUICC)
				if(backChannelProfileId != G_CurExecutingProfileIndex)//先还原 deselect以后再切到select执行结果指令指定的区域, 执行区域已设置在通道信息中setChannelProfileIndex(selected_channel, G_CurExecutingProfileIndex);
				{
					G_CurExecutingProfileIndex = backChannelProfileId;
					ExecuEnvironmentInit(G_CurExecutingProfileIndex);
				}
				#endif
				
				//if is not iso7816 interface, if select applet on base channel is successful, then set flag to don't select default applet again on java , add by wrh 20221018
				if (bRamBuffer[SELECTING_APPLET_FLAG] == SELECT_COMMAND_FLAG)
				{
					//on base channel, after first select applet successful, then don't select dufault applet or Implicit applet at continue not command
					//merged from CIU9872B_01@20220926
					if ( (apdu_GetProtocol() != PROTOCOL_T0) && (selected_channel == getIoBaseChannel()) )
					{
						bRamBuffer[RF_FIRST_SELECT_CMD_FLAG] = 1;
					}
				}
				return;
			}
				// ISO 7816-4 MANAGE CHANNEL command
			case INS_MANAGECHANNEL:
			{
				channelManagerCommand();
				if(SW_LOGICAL_CHANNEL_NOT_SUPPORTED == PXB_R_U16(&bShortBuffer[VM_EXCEPTION]))
				{
					selected_channel	= preSelectedChannel;
				}
				return;
			}
			default:
				break;
		}
	}
#if JAVA_CARD_UICC // modify by jiangxiaohui@2022606
	else if (APDUBuffer[OFFSET_INS] == INS_MANAGECHANNEL)
	{
#if UICC__OPERATOR__CHINATELECOM
		if (APDUBuffer[OFFSET_CLA] != 0x80) //no exception, 中国电信特殊命令 8070
#endif
		{
			if(U8_AND_U8((APDUBuffer[OFFSET_CLA]), 0x0C) != 0) //b3 b4 :Secure messaging indication
			{
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_SECURE_MESSAGEING_NOT_SUPPORTED);
				return;
			}
			else if(U8_AND_U8(APDUBuffer[OFFSET_CLA], 0xF0) != 0)//7816-4  Table 2 specifies 000x xxxx as the first interindustry values.  Bits 8, 7 and 6 are set to 000. Bit 5 controls command chaining
			{
				PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_CLA_NOT_SUPPORTED);
				return ;
			}
		}

	}
#endif

	if ((bCardState == CARD_TERMINATED) && (PXB_R_U8(&APDUBuffer[OFFSET_INS]) != 0xca))
	{
		if ((0x01 == getConfig(GP_CONFIG_FOR_BFC)) && (PXB_R_U8(&APDUBuffer[OFFSET_INS]) == 0x82))
		{

		}
		else
		{
			PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_INS_NOT_SUPPORTED);
			return;
		}

	}
#if VM_WITH_PBOC_API
	if(true == PbocAPI_IsGetConstructAuthCode(APDUBuffer))
	{
		return;
	}
#endif
	if(getChannelState(selected_channel) == CHANNEL_CLOSED)
	{
		selected_channel	= preSelectedChannel;
		PXB_W_U16(MEMtPX8(&bShortBuffer[VM_EXCEPTION]), SW_LOGICAL_CHANNEL_NOT_SUPPORTED);
	}
	return;
}



/**
 * Requests additional processing time from CAD. The implementation should ensure that this method
 * needs to be invoked only under unusual conditions requiring excessive processing times.
 */
/*
 * To generate a standard select APDU responce for Card Manager
 */
void sendSelectResponceAPDU(jref aid, u8 appId)
{
	u8			offset;
	u8			i;
	PN8			aidArray;
	u16			bLen;

	// File Control Information
	PX_W_U8(&APDUBuffer[0], 0x6F);

	// Card Manager AID
	PX_W_U8(&APDUBuffer[2], 0x84);
	//aidArray				= MEMtPN8(locateObjectInMemory(get_object_reference_field(MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPN8(OP_oCardManagerAID_JC)))), AIDARRAY_INDEX_OF_AID)));
	aidArray				= MEMtPN8(locateObjectInMemory(aid));
	PX_W_U8(&APDUBuffer[3], U16t8(get_array_length(aidArray)));
	aidArray 				= MEMtPN8(get_array_data(aidArray));
	memCopyNoAtomic(MEMtPM8(&APDUBuffer[4]), aidArray, U8t16(PX_R_U8(&APDUBuffer[3])) );
	offset 					= U8_A_U8(PX_R_U8(&APDUBuffer[3]), 4);

	// Proprietary data
	PX_W_U8(&APDUBuffer[offset], 0xA5);
	U8_INC(offset);
	bLen = getCardRecognitionData(appId, &APDUBuffer[offset+1]); //66tag下的73tag
	PX_W_U8(&APDUBuffer[offset], (0x04 + bLen));
	U8_INC(offset);
	offset += bLen;

	// 0x9f65
	for (i = 0; i < 4; U8_INC(i))
	{
		PX_W_U8(&APDUBuffer[offset], rData2[i]);
		U8_INC(offset);
	}

	// Fill in FCI complete length.
	PX_W_U8(&APDUBuffer[1], U8_S_U8(offset, 2));

	apdu_ApduSetOutgoingAndSend(0, offset); // send the output buffer to the terminal
}
// modify by xuxiaojun@20220318 分层,该函数被往上移动
//boolean isUSIMApplet()
/*
*@function    : getPrivilegeByAppId
*
*@description : 通过AppID获取应用的权限
*
*@param       : u8 AppID       --- 应用的AppID
*@param       : PX8 privilege  --- 应用的权限（作为返回值）
*
*@return      : void
*@history     : created by lixin@20210816 增加选择、安装、删除应用时对FINAL权限的处理；https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
*/
void getPrivilegeByAppId(u8 AppID, PX8 privilege)
{
	jref 	X_A		bPrivRef;
	u32 	X_A		appEntryAddr;
	memref 	X_A		bPrivArray;

	appEntryAddr = getEntryByAppId(AppID);
	bPrivRef = PX_R_U16(get_object_field_ref(MEMtPF8(appEntryAddr), bPriv_INDEX));
	bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));

	ecpMem_Copy(privilege, bPrivArray, 0x03);
}

/*
*@function    : getAppIndexByPrivilege
*
*@description : 通过权限值获取AppTable上具有该权限的第一个应用的AppIndex
*
*@param       : u8 privilege  --- 一个字节的权限值
*@param       : u8 number     --- privilege在权限值的第几个字节
*
*@return      : AppIndex
*@history     : created by lixin@20210816 增加选择、安装、删除应用时对FINAL权限的处理；https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
*/
u8 getAppIndexByPrivilege(u8 privilege, u8 number)
{
	u16 		D_A			i;
	u32 		D_A			theEntryObjectAddr;
	u16 		D_A			bPrivRef ;
	memref 		D_A			bPrivArray ;
	u8          D_A         bPrivilege;
	u8          D_A         appId;

	for (i = GetNextAppEntryIdxByPriority(0xFFFF); i != 0xFFFF; i = GetNextAppEntryIdxByPriority(i))
	{
		theEntryObjectAddr = locateObjectInMemory(memReadU16(MEMtPM8(U32_A_U16(APP_Table_Cash, U16_M_U16(i, SIZE_REF)))));
		appId = memReadU8(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), bAppID_INDEX)));
		if(appId == APP_NULL)
		{
			continue;
		}
		bPrivRef = memReadU16(MEMtPM8(get_object_field_ref(MEMtPF8(theEntryObjectAddr), bPriv_INDEX)));

		bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
		switch(number)
		{
			case 1:
				bPrivilege = memReadU8(MEMtPM8(bPrivArray));
				break;
			case 2:
				bPrivilege = memReadU8(MEMtPM8(bPrivArray+1));
				break;
			case 3:
				bPrivilege = memReadU8(MEMtPM8(bPrivArray+2));
				break;
			default:
				break;
		}

		if((bPrivilege & privilege) == privilege)
		{
			return i;
		}

	}

	return APP_NULL;
}
/*
*@function    : SetbSDId
*
*@description : 根据指定应用的权限值设置g_bSDId[selected_channel]
*
*@param       :
*
*@return      : void
*@history     :added by https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin/code @20210816
*@history     :move this function from Interface_PlatformFromLTE.c
*/
void SetbSDId(void)
{
	u16  I_A   appTableEntry;
	PN8	 D_A   entry;
	//u8   I_A   bAppID;
	u16  I_A   bPrivRef;
	u8   I_A   bFirstPrivilageByte;
	memref D_A bPrivArray;
	u8	   D_A bState_;

	appTableEntry = popReference();

	entry			= MEMtPN8(locateObjectInMemory(appTableEntry));
	//bAppID			= memReadU8(get_object_field_ref(entry, bAppID_INDEX));  // remove warning,dengyiwen,2022/9/22
	bState_			= memReadU8(get_object_field_ref(entry, bCurrentState_INDEX));

	bPrivRef = memReadU16(MEMtPM8(get_object_field_ref(entry, bPriv_INDEX)));
	bPrivArray = get_array_data(MEMtPF8(locateObjectInMemory(bPrivRef)));
	bFirstPrivilageByte = memReadU8(MEMtPM8(bPrivArray));

	if (U8_AND_U8(bState_, 0x87) == APPLET_SELECTABLE)
	{
		if(U8_AND_U8(bFirstPrivilageByte , 0x80) == 0x80)
		{
			g_bSDId[selected_channel] = memReadU8(get_object_field_ref(entry, bAppID_INDEX));	 //wzq
		}
		else if(memReadU8(get_object_field_ref(entry, bAppID_INDEX)) != ISD_APPID)
		{
			g_bSDId[selected_channel] = memReadU8(get_object_field_ref(entry, bsdId_INDEX));
		}
		else
		{
			g_bSDId[selected_channel] = ISD_APPID;	//ISD_ID
		}

		pushShort(1);
	}
	else
	{
		pushShort(0);
	}

}
/**
**@history     : created by lixin@20210816 增加选择、安装、删除应用时对FINAL权限的处理；https://192.168.0.6/svn/COS/EPX/EPX/v3.0/BRANCH/lixin
*/
u8 NM_getAppIndexByPrivilege(void)
{
	u8 privilege, number;

	number = popShort();
	privilege = popShort();

	return getAppIndexByPrivilege(privilege, number);
}

#if GP__VERSION__FOR_CHINA_UNICOM_MASK
boolean isTCKTesting()
{
	// total len + aids' LV
	u8 aid_LV_Array[] = {0x10,
						 0x0f,0xa0,0x00,0x00,0x00,0x62,0x03,0x01,0x0a,0x01,0x01,0x02,0x04,0x0b,0x04,0x81
						};
	u8 offset;
	u8 result;

	result = 1;
	for (offset = 1; offset < aid_LV_Array[ 0 ]; )
	{
		result = memCompare(MEMtPM8(&APDUBuffer[ OFFSET_LC ]), MEMtPM8(&aid_LV_Array[ offset ]), (aid_LV_Array[ offset ] + 1));
		offset +=  aid_LV_Array[ offset ] + 1;

		//hit the tck aid
		if (ECP_CMP_RESULT_EQUAL == result)
			break;
	}

	if (ECP_CMP_RESULT_EQUAL == result)
	{
		return true;
	}
	else
	{
		return false;
	}
}
#endif

