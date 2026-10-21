/*****************************************************************************
 * File:  chn_manager.c	
 * Version 1.14 03/09/24

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify
 * Original author: David Getreu
 ****************************************************************************/

#ifndef   __CHN_MANAGER_H__
#define   __CHN_MANAGER_H__

#include "profile.h"
#include "typejava.h"
#include "memory.h"
#include "target.h"
#include "bootConfigProfile.h"

/* This structure is used to store logical channel information
 at runtime.  The structure changes constantly, and is allocated
 in the card's RAM memory.
 The information stored inside, per channel:
 - Assigned Clear-On-Deselect space ID
 - Channel state (i.e. Closed, Open, Open Multiselectable)
 - Selected context
 */

typedef struct t_channel_manager
{
	u8 channel_profileIndex[MAX_LOGICAL_CHANNELS]; // initialize to ILLEGAL_PROFILEINDEX
	u8 channel_status[MAX_LOGICAL_CHANNELS];
	u8 security_level[MAX_LOGICAL_CHANNELS];
	u8 security_level_invert[MAX_LOGICAL_CHANNELS];
	u8 requested_security_level[MAX_LOGICAL_CHANNELS];
	u8 sercrity_channel_number[MAX_LOGICAL_CHANNELS];
	u8 selected_context[U8_M_U8(MAX_LOGICAL_CHANNELS, 2)];
	u8 selected_key_appId[MAX_LOGICAL_CHANNELS]; //added by xuxiaojun@20220508 用于记录当前通道上建立安全会话时所使用的key所属的SD
	u8 compulsory_Session_Security_Level[MAX_LOGICAL_CHANNELS] ;
} channel_manager;
// channel Manager Info using mannager the channel
extern channel_manager   channelInfo;

/* Channel status constants */
#define CHANNEL_CLOSED		0x00
#define CHANNEL_DISABLED	0x01
#define CHANNEL_OPEN		0x02
#define CHANNEL_OPEN_MS		0x03

/* Useful constants */
#define CHANNEL_MS_MASK		0x01
#define CHANNEL_OPEN_MASK	0x02

/* Context specific constants */
#define APPLET_MULTISELECTED	0x08
#define APPLET_ACTIVE			0x04
#define PACKAGE_MULTISELECTED	0x02
#define PACKAGE_ACTIVE			0x01

/* Logical Channel operations */
#define OP_CHANNEL_CLOSE            0x00
#define OP_CHANNEL_OPEN             0x01
#define OP_CHANNEL_OPEN_AUTOSELECT  0x02

#define DTR_SPACEID_MASK            0x0003

#define MUL_SEL_RESTRICTION			0x01
#define MUL_SEL_STATE				0x20
#define MUL_APP_SEL_STATE			0x40
#define SIG_SEL_STATE				0x02

#define INVALID_CHANNEL				0xFF

/* Interface functions */
extern  void setSecurityLevel(u8 selected_channel ,u8 securityLeve);
extern  u8 getSecurityLevel(u8 selected_channel);

extern  void channel_init(void);


extern  memref getDTRSpaceAddress(u8 channelId);

extern  u8 getDTRSpaceId(u8 channelId);



extern  u16 closeChannel(u8 channelId);

extern  u16 openChannel(u8 channelId);

extern  u8 getAvailableChannel(void);

extern	u8 getChannelState(u8 channelId);
extern  u8 getChannelProfileIndex(u8 channelId);
extern void setChannelProfileIndex(u8 channelId, u8 profileIndex);
extern void setNextProfileIndex(u8 profileIndex);
extern u8 getNextProfileIndex(void);
extern	u8 isContextSelected(u8 cc);  // change to U8 
extern	u8 MultiSelectState(jref theApplet,u8 channelId)                                   SMALL_MODE;
extern	u16 getChannelContext(u8 channelId);
extern	void setChannelContext(u8 channelId, u16 contextId, jboolean isCloseChannel)	   SMALL_MODE;
extern	void check_SIOaccess(jref objectId, u8 appID)									   SMALL_MODE;
extern	u16 channelManage(u8 channelId, u8 operationId);			
			  			
extern  u8 getContextStatus(u16 contextId, u8 checkApp, u8 profileIndex)                                    SMALL_MODE;

//jboolean checkAppletSelectability(u16 contextId);

#if JCVM__VM__FIRE_WALL
extern  boolean checkShareableObjectAccess(u16 contextId)								   SMALL_MODE;
#endif

/***
 * merged from CIU9872B_01@20220926
 */
extern u8 getIoBaseChannel(void);
extern u8 getIoMaxChannel(void);

#endif

