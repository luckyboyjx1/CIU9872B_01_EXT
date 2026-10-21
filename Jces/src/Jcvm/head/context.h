/*****************************************************************************
 * File:  context.h
 * Version 1.6

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,4	.modify
 * Original author: David Getreu
 ****************************************************************************/

#ifndef   __CONTEXT_H__
#define   __CONTEXT_H__

#include "chn_manager.h"
#include "contentManager.h"

#define APPLET_MASK                     0x00FF
#define GROUP_MASK                      0xFF00
#define GROUP_SHIFT_MASK                0x08
#define NUM_GROUPS                      ON_CARD_PKG_MAX  
#define NUM_APPS                      	32
#define GET_PKGID(contentId)			U16t8(U16_SHR_U8(U16_AND_U16(contentId, GROUP_MASK), GROUP_SHIFT_MASK))

/* JCRE System Context definition */
#define JCRE_CONTEXT                    0x00
#define UICC_CONTEXT                    0x0101

/* Null Context definition */
#define NULL_CONTEXT                    0xFFFF

/* Initial PC value for the VM boot */
#define INITIAL_PC                      0
#define INITIAL_PKG                     0
/* Tests if the current context is the JCRE context. */
//#define IS_JCRE_CURRENT_CONTEXT()       (cc < g_bIsdCount)
#define IS_JCRE_CURRENT_CONTEXT()       ((cc < U8t16(g_bIsdCount)) || ((s8)(cc & 0xFF) <= 0x00))
/* Tests if the context of the object referenced by x, is the current context. */
#define IS_OBJECT_CONTEXT_CURRENT(x) 	(U16_AND_U16(get_object_owner(x), OBJ_HDR_GroupContext_MASK) == U16_AND_U16(cc, OBJ_HDR_GroupContext_MASK))
																																		 
#define IS_OBJECT_CONTEXT_SELECTED(x) 	(U16_AND_U16(get_object_owner(x), OBJ_HDR_GroupContext_MASK) == U16_AND_U16(getChannelContext(selected_channel), OBJ_HDR_GroupContext_MASK))
//#define IS_OBJECT_CONTEXT_SELECTED(x) ((get_object_owner(x) & OBJ_HDR_GroupContext_MASK) == ((selected_context) & OBJ_HDR_GroupContext_MASK))

#define IS_SELECTED_CONTEXT() 			(U16_AND_U16(cc, OBJ_HDR_GroupContext_MASK) == U16_AND_U16(getChannelContext(selected_channel), OBJ_HDR_GroupContext_MASK))
//#define IS_SELECTED_CONTEXT() ((cc & OBJ_HDR_GroupContext_MASK) == ((selected_context) & OBJ_HDR_GroupContext_MASK))

extern  u8 fetchByte(void);
extern  u16 fetchShort(void);
//extern  jref fetchReference(void);
#define fetchReference fetchShort

/* Currently selected channel access */	   
#define getCurrentlySelectedContext()	(getChannelContext(selected_channel))

#if VM_INTS_SUPPORTED
extern  u32 fetchInt(void);
#endif

#endif
