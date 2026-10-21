/*****************************************************************************
 * File:  chn_manager.c	
 * Version 1.14 03/09/24

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/
#include "chn_manager.h"
#include "memory.h"
#include "context.h"
#include "anomaly.h"
#include "global_variable.h"
#include "object.h"
#include "ob_manager.h"
#include "reference.h"
#include "packageloc.h"
#include "opfunction.h"
#include "contentManager.h"
#include "RamManager.h"
#include "JcvmRam.h"
#include "DataOperation.h"
#include "nativemethod_impl.h"
#include "dtrSpace.h"
#include "cardManager.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if HAL__MEMORY__RAM_MAP_IN_C51
#pragma JCVM_CHANNEL_MANAGER_RAM_START
#endif

channel_manager	X_A		channelInfo;



/* Channel specific access */

u8 getChannelState(u8 channelId) {
	return channelInfo.channel_status[channelId];
}

static void setChannelState(u8 channelId, u8 state) {
	channelInfo.channel_status[channelId] = state;
}

u8 getChannelProfileIndex(u8 channelId) {
	return channelInfo.channel_profileIndex[channelId];
}

void setChannelProfileIndex(u8 channelId, u8 profileIndex) {
	channelInfo.channel_profileIndex[channelId] = profileIndex;
}
#if (EUICC_ONLY||SE_AND_EUICC)
void setNextProfileIndex(u8 profileIndex) {
	G_NextExecutingProfileIndex = profileIndex;
}
u8 getNextProfileIndex(void) {
	return G_NextExecutingProfileIndex;
}
#endif
static void setChannelData(u8 channelId, u16 contextId, boolean isCloseChannel, u8 memSpace)	SMALL_MODE 
{
	PX_W_U16(&channelInfo.selected_context[U8_M_U8((channelId),2)],contextId);
	dtrSpace_setChannelSpaceId(channelId, memSpace);
	if (isCloseChannel == 1) {
		setChannelState(channelId, CHANNEL_CLOSED);
	} else {
		setChannelState(channelId, CHANNEL_OPEN);
	}
}
void setSecurityLevel(u8 channel_selected ,u8 securityLeve)
{
	channelInfo.security_level[channel_selected]		    = securityLeve;
	channelInfo.security_level_invert[channel_selected]		= U8_INV(securityLeve);
}
u8 getSecurityLevel(u8 channel_selected)
{
	if(U8_AND_U8(channelInfo.security_level[channel_selected],channelInfo.security_level_invert[channel_selected]) != 0)
	{
		faultDetected();
	}
	return channelInfo.security_level[channel_selected];
}
u8 isContextSelected(u8 bcc)
{
	u8 i;
		
	for(i = 0; i < logicalChannelNums ; U8_INC(i))
	{
	#if (EUICC_ONLY||SE_AND_EUICC)
		if(getChannelProfileIndex(i) == G_CurExecutingProfileIndex)
	#endif
		{
			if(bcc == U16t8(U16_SHR_U8(getChannelContext(i), GROUP_SHIFT_MASK)))
			{
				return i;
			}
		}
	}
	
	return INVALID_CHANNEL;
}

u8 getContextStatus(u16 contextId, u8 checkApp, u8 profileIndex)    SMALL_MODE
{
	u8		D_A		groupId;
	u8		D_A		i_iter;
	u16		D_A		testContextId;
	u8		D_A		testGroupId;
	u8		D_A		status;

	if (contextId == NULL_CONTEXT) {
		return 0x00;
	}
	status = 0x00;

	/* group context we are looking for */
	groupId	= U16t8(U16_AND_U16(U16_SHR_U8(contextId, GROUP_SHIFT_MASK), LOW_BYTE_OF_SHORT));

	for (i_iter = 0; i_iter < logicalChannelNums; U8_INC(i_iter)) // logicalChannelNums 对应配置区的VM_OS_LOGICAL_CHANNEL_NUMBER
	{
	#if (EUICC_ONLY||SE_AND_EUICC)
		if(getChannelProfileIndex(i_iter) == profileIndex)
	#endif
		{
			testContextId	= getChannelContext(i_iter);

			if (testContextId != NULL_CONTEXT) {
				testGroupId	= U16t8(U16_AND_U16(U16_SHR_U8(testContextId, GROUP_SHIFT_MASK), LOW_BYTE_OF_SHORT));
				/* If groups match */
				if (testGroupId == groupId) {
					if (U8_AND_U8(status, PACKAGE_ACTIVE)) {
						status	= U8_OR_U8(status, PACKAGE_MULTISELECTED);
						if(!checkApp)
						{
							return	status;
						}
					} else {
						status	= U8_OR_U8(status, PACKAGE_ACTIVE);
					}
					/* If in addition applet match */
					if (testContextId == contextId) {
						if (U8_AND_U8(status, APPLET_ACTIVE)) {
							status	= U8_OR_U8(status, APPLET_MULTISELECTED);
							return	status;
						} else {
							status	= U8_OR_U8(status, APPLET_ACTIVE);
						}
					}
				}
			}
		}
	}
	return	status;
}

/**
 * Also, handle DTR space allocation / deallocation accordingly
 * modify by xuxiaojun@20220513 根据分层代码 第一次代码评审(20220415).xlsx 将G_CurExecutingProfileIndex宏定义为0xFF
 */
void setChannelContext(u8 channelId, u16 contextId, boolean isCloseChannel)	SMALL_MODE 
{
	u8		D_A		SpaceId;
	u8		D_A		prevSpace;
	u16		D_A		prevContextId;

	SpaceId	= 0x00;	
	if (contextId == NULL_CONTEXT) {
        /* Deselection scenario */
	    prevSpace		= dtrSpace_getDtrSpaceId(channelId);
		prevContextId	= getChannelContext(channelId);
		setChannelData(channelId, contextId, isCloseChannel, SpaceId);
		if(	(prevContextId != NULL_CONTEXT ) && (U8_AND_U8(getContextStatus(prevContextId, false,getChannelProfileIndex(channelId)), PACKAGE_ACTIVE) == false))
		{
			dtrSpace_closeDtrSpace(prevSpace, prevContextId);
		}
		setChannelProfileIndex(channelId, G_CurExecutingProfileIndex);//modify by xxj@20230404: eUICCMemoryReset的refresh过程中开通道，会获取到通道上原来的profile Index，但是实际上该profile已经不存在了
	} else {
		SpaceId = dtrSpace_assignDtrSpace(contextId, U8_AND_U8(getContextStatus(contextId, false,G_CurExecutingProfileIndex), PACKAGE_ACTIVE));
		if(0xFF == SpaceId)
		{
			throw_error(SW_APPLET_SELECT_FAILED);//throw_error(SYSTEMEXCEPT_NO_TRANSIENT_SPACE);
		}
		setChannelProfileIndex(channelId, G_CurExecutingProfileIndex);
		setChannelData(channelId, contextId, isCloseChannel, SpaceId);
	}
}

u16 getChannelContext(u8 channelId) {
	return (PX_R_U16(&channelInfo.selected_context[U8_M_U8((channelId),2)]));
}

u16 closeChannel(u8 channelId) {
	u8		D_A		tempState;

	channelId	= U8_AND_U8(channelId, DTR_SPACEID_MASK);
	tempState	= getChannelState(channelId);
	if (tempState == CHANNEL_CLOSED) {
		return 0xFFFF;
	} else {
		setChannelState(channelId, CHANNEL_CLOSED);
		return	0;
	}
}

u16 openChannel(u8 channelId) {
	u8		D_A		tempState;

	tempState	= getChannelState(channelId);
	
	if(tempState == CHANNEL_CLOSED){
		setChannelState(channelId, CHANNEL_OPEN);
		return	0;
	}
	return	0xFFFF;
}

/**
*modify by weipingjiao@20210419 EPX产业化\INFENION_SLM97\Develop
*merged from CIU9872B_01 @20220926
*/
u8 getAvailableChannel() 
{
	u8		D_A		i_iter;
	u8		D_A		tempState;
	u8 		maxChannels;


	//call channel function, add by wrh 20221018
	i_iter = getIoBaseChannel();
	maxChannels = i_iter + getIoMaxChannel();

//	for (; i_iter < i_iter + maxChannels; U8_INC(i_iter))
	for (; i_iter < maxChannels; U8_INC(i_iter))
	{
		tempState = getChannelState(i_iter);
		if (tempState == CHANNEL_CLOSED) 
		{
			return i_iter;
		}
	}
	return	0xFF;
}


//This routine checks the selected applet's states.It is called when applet is selected.
//The below explain the states:
//1.MUL_SEL_RESTRICTION:The selected applet have multi-selection restriction on the card.
//2.MUL_SEL_STATE:The applet is being selected while the same applet instance or another 
//                applet instance from the same package is active on another logical channel.
//3.MUL_APP_SEL_STATE:The applet is being selected while the same applet instance is already
//                    active on another logical channel
//4.SIG_SEL_STATE:The selected applet is not in the multi-selection condition. 

u8 MultiSelectState(jref theApplet,u8 channelId)    SMALL_MODE
{
	u16    	D_A		context;
	u16    	D_A		preContext;
	u8     	D_A		MultiSelResult;
	u8	   	D_A		contextStatus;
	u8     	D_A		appID;
	PN8 	D_A		theAppletAddr;
/*	memref classAddr, sFlagMutiselect;		  */

	MultiSelResult	= 0;
	theAppletAddr	= MEMtPN8(locateObjectInMemory(theApplet));
	if(MEM_NULL != theAppletAddr)
	{
		context			= PN_R_U16(PN8_A_CONST(theAppletAddr, OBJ_HDR_Context));
		appID			= U16t8(U16_AND_U16(context, APPLET_MASK));

		preContext		= getChannelContext(channelId);

		contextStatus	= getContextStatus(context, true,G_CurExecutingProfileIndex);

		if((U8_AND_U8(contextStatus, PACKAGE_MULTISELECTED) != 0)
			|| ((U8_AND_U8(contextStatus, PACKAGE_ACTIVE) != 0) && (U16_SHR_U8(preContext, GROUP_SHIFT_MASK) != U16_SHR_U8(context, GROUP_SHIFT_MASK)))
		  )
		{
			if(!manageAppFlag(appID, CHECK_MULTI_FLAG))
			{
				MultiSelResult	= MUL_SEL_RESTRICTION;
			}
			else
			{
				MultiSelResult	= MUL_SEL_STATE;
				if(U8_AND_U8(contextStatus, APPLET_ACTIVE) != 0)
				{
					MultiSelResult	= U8_OR_U8(MultiSelResult, MUL_APP_SEL_STATE);
				}
			}
		}
		else
		{
			MultiSelResult	= SIG_SEL_STATE;
		}
	}
	return	MultiSelResult;
}

#if JCVM__VM__FIRE_WALL
/**
 * modify by xuxiaojun@20220513 根据分层代码 第一次代码评审(20220415).xlsx 将G_CurExecutingProfileIndex宏定义为0xFF
 */
boolean checkShareableObjectAccess(u16 contextId)	SMALL_MODE 
{
	
	u16		D_A		selectedCtx;
	u8		D_A		contextStatus;
	u8		D_A		appID;

	contextStatus = getContextStatus(contextId, false,G_CurExecutingProfileIndex);

	/* If no applets in package active, always allow access */
	if (contextStatus == 0x00) {
		return	1;
	}

	//contextId = (u8)((contextId >> GROUP_SHIFT_MASK) & 0x0F);

	selectedCtx = U16_AND_U16(U16_SHR_U8(getCurrentlySelectedContext(), GROUP_SHIFT_MASK), 0x0F);

	/* If trying to access a shareable interface object belonging to the package
	   where the currently selected applet is defined, allow access. */
	if (U16_AND_U16(U16_SHR_U8(contextId, GROUP_SHIFT_MASK), 0x0F) == selectedCtx) {
		return	1;
	}
	
	appID	= U16t8(U16_AND_U16(contextId, APPLET_MASK));

	/* If context is active and is not multiselectable, disallow access */
	/* For this check to work all applets in package must be either all
	   multiselectable or all non-multiselectable. */
	if(U8_AND_U8(contextStatus, PACKAGE_ACTIVE) != 0)
	{
		if(manageAppFlag(appID, CHECK_MULTI_FLAG))
		{
			return	1;
		}
	}
	/* Shall never get here */
	return	0;
}
#endif

/* Channel initialization structure upon reset */
/**
*modify by weipingjiao@20210419 EPX产业化\INFENION_SLM97\Develop
*merged from CIU9872B_01@20220926
*/
void channel_init(void) {
	u8		D_A		i_iter;

	for (i_iter = 0; i_iter < MAX_LOGICAL_CHANNELS; U8_INC(i_iter)) {
		PX_W_U16(&channelInfo.selected_context[U8_M_U8(i_iter,2)],NULL_CONTEXT);
		channelInfo.selected_key_appId[i_iter] = APP_NULL;
		setSecurityLevel(i_iter ,SCF_NO_AUTHENTICATED);
		channelInfo.compulsory_Session_Security_Level[i_iter]= SCF_NO_AUTHENTICATED;
		channelInfo.channel_status[i_iter]		= CHANNEL_CLOSED;
		channelInfo.sercrity_channel_number[i_iter] = 0x00;
#if (EUICC_ONLY||SE_AND_EUICC)
		setChannelProfileIndex(i_iter, JcvmConfig_GetProfileEnableIndex());
#endif
	}
	// make sure channel 0 is open
	channelInfo.channel_status[ISO_BASIC_CHANNEL_NUM] = CHANNEL_OPEN;
#if JCVM__IO__SWP
	channelInfo.channel_status[NFC_BASIC_CHANNEL_NUM] = CHANNEL_OPEN;
#endif	
#if JCVM__IO__SPI
	channelInfo.channel_status[SPI_BASIC_CHANNEL_NUM] = CHANNEL_OPEN;
#endif	
#if JCVM__IO__I2C
	channelInfo.channel_status[I2C_BASIC_CHANNEL_NUM] = CHANNEL_OPEN;
#endif	

}

u16 channelManage(u8 channelId, u8 operationId) 
{
    u16		D_A		result;

	result	= 0xFFFF;
    switch (operationId) {
      case OP_CHANNEL_CLOSE:
          result		= closeChannel(channelId);
          break;
      case OP_CHANNEL_OPEN:
          result		= openChannel(channelId);
          break;
      case OP_CHANNEL_OPEN_AUTOSELECT:
          channelId		= getAvailableChannel();
          if (channelId == 0xFF) 
		  {
               result = 0xFFFF;
          } else 
		  {
              openChannel(channelId);
              result 	= U8t16(channelId);
          }
          break;
      default:
           result		= 0xFFFF;
           break;
    }
    return result;
}


/*================================================================================
 * FUNCTION:      check_SIOaccess
 * Check if the server applet is not multiselectable and is currently active on another logical channel
 =================================================================================*/
void check_SIOaccess(jref objectId, u8 appID)		SMALL_MODE
{
	u8		D_A		cmdChannel;
	PM8		D_A		selectAppletTable;

	if (objectId == 0)
	{
		throw_error(ASSERTION_FAILED);
	}
		 
	if(!manageAppFlag(appID, CHECK_MULTI_FLAG))
	{
	/*@ -disspecopt @*/	selectAppletTable	= MEMtPN8(get_array_data(MEMtPN8(locateObjectInMemory(nvmReadU16(MEMtPF8(AM_SelectedApplet_JC))))));/*@ +disspecopt @*/
		for(cmdChannel = 0; cmdChannel < logicalChannelNums ; U8_INC(cmdChannel))
		{ 	
			if((selected_channel != cmdChannel) && (U8_AND_U8(getChannelState(cmdChannel), CHANNEL_OPEN) == CHANNEL_OPEN))
			{
			#if (EUICC_ONLY||SE_AND_EUICC)
				if(getChannelProfileIndex(cmdChannel) == G_CurExecutingProfileIndex)
			#endif
				{
					if(objectId == nvmReadU16(PM8_A_U8(selectAppletTable, U8_M_U8(cmdChannel, SIZE_REF))))
					{
						throw_error(SECURITY_EXCEPTION);
					}
				}
			}
		}										
	}
}/*end markAppletRoot*/

/**
  *@brief	get the base channel number of the correspond interface in all logical channels
  *@param	void
  *@retval	base channel number
  *merged from CIU9872B_01@20220926 将获取各个接口的基本逻辑通道封成接口
  */
u8 getIoBaseChannel(void)
{
	u8		D_A		curProtocol;
	u8		D_A		baseChn;
	
	curProtocol = apdu_GetProtocol();
#if JCVM__IO__I2C
	if (curProtocol == PROTOCOL_I2C)
	{
		baseChn = I2C_BASIC_CHANNEL_NUM;
	}
	else
#endif
#if JCVM__IO__SPI
	if (curProtocol == PROTOCOL_SPI)
	{
		baseChn = SPI_BASIC_CHANNEL_NUM;
	}
	else
#endif
#if JCVM__IO__SWP
	if (curProtocol == PROTOCOL_SWP)
	{
		baseChn = NFC_BASIC_CHANNEL_NUM;
	}
	else 
#endif
#if WITH_USB
	if (curProtocol == PROTOCOL_USB)
	{
		baseChn = USB_BASIC_LOGICAL_CHANNEL;
	}
	else 
#endif
#if HAL__IO__ISO14443_TA
	if (curProtocol == PROTOCOL_TA)
	{
		baseChn = NFC_BASIC_CHANNEL_NUM;
	}
	else 
#endif
	{
		baseChn = ISO_BASIC_CHANNEL_NUM;
	}
	
	return baseChn;
}

/**
  *@brief	get the max channel counter of the correspond interface
  *@param	void
  *@retval	max channel counter
  *merged from CIU9872B_01@20220926 将获取各个接口的最大逻辑通道数封成接口
  */
u8 getIoMaxChannel(void)
{
	u8		D_A		curProtocol;
	u8		D_A		maxChn;
	
	curProtocol = apdu_GetProtocol();
#if JCVM__IO__I2C
	if (curProtocol == PROTOCOL_I2C)
	{
		maxChn = I2C_MAX_LOGICAL_CHANNELS;
	}
	else
#endif
#if JCVM__IO__SPI
	if (curProtocol == PROTOCOL_SPI)
	{
		maxChn = SPI_MAX_LOGICAL_CHANNELS;
	}
	else
#endif
#if JCVM__IO__SWP
	if (curProtocol == PROTOCOL_SWP)
	{
		maxChn = NFC_MAX_LOGICAL_CHANNELS;
	}
	else 
#endif
#if WITH_USB
	if (curProtocol == PROTOCOL_USB)
	{
		maxChn = USB_MAX_LOGICAL_CHANNELS;
	}
	else 
#endif
#if HAL__IO__ISO14443_TA
	if (curProtocol == PROTOCOL_TA)
	{
		maxChn = NFC_MAX_LOGICAL_CHANNELS;
	}
	else 
#endif
	{
		maxChn = ISO_MAX_LOGICAL_CHANNELS;
	}

	return maxChn;
}
