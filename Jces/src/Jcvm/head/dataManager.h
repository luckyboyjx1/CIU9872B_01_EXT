#ifndef  __DATA_MANAGER_H__
#define  __DATA_MANAGER_H__

#include "typejava.h"
#include "mask.h"
#include "ob_manager.h"

#define STORE_DATA_GET_TAG1									0
#define STORE_DATA_GET_TAG2									1
#define STORE_DATA_GET_DGI									2
#define STORE_DATA_ANALYSE_0070TAG							3
#define STORE_DATA_SELF_HANDLE_TAG							4
#define STORE_DATA_GET_LENGTH								5
#define	STORE_DATA_ANALYSE_TAG								6
#define STORE_DATA_GET_TAG_ARRAY_AFEILD						7
#define STORE_DATA_INIT_STORE_VALUE							8
#define	STORE_DATA_VALUE									9
#define STORE_JAVA_TAG										10 //这个TAG要与JAVA层的对应
#define NEXT_STORE_DATA										11
#define STORE_DATA_FINISH									12
#define JUMP_FUNCTION										13

#define GET_DATA_FINISH										0
#define GET_DATA_THE_JAVA_TAG								1
#define GET_DATA_THE_C_TAG									2
#define GET_SEND_LENGTH_SHIFT								16
#define GET_TAG_DATA_MASK									0x00FF

#define DGI_HANDLE_FINISH									0

#define		DOUBLE_TAG			0x1F
#define 	BIT0_TAG			0x01
#define		TAG1_LENGTH			1
#define		TAG2_LENGTH			1
#define		DGI_LENGTH			2

#define		CUR_STORE_DATA		0x08
#define   	JAVA_STORE_DATA   	0x04

extern  u8			g_handle0070;
extern  u8			g_isLink;
extern  u8			g_BitTagValue[MAX_LOGICAL_CHANNELS];
extern	u16			g_curStoreDataTag;
extern	u8			g_storeDataOffset[MAX_LOGICAL_CHANNELS];
extern	u16			g_curStoreDataTagLength[MAX_LOGICAL_CHANNELS];
extern	u8			g_storeDataSequence[MAX_LOGICAL_CHANNELS];
extern  u16			g_curStoreDataDgiLength[MAX_LOGICAL_CHANNELS];
extern  u8			g_storeDataState[MAX_LOGICAL_CHANNELS];
extern  memref		g_tagArrayContent[MAX_LOGICAL_CHANNELS];
extern	u16			g_curStoreDataTagOffset[MAX_LOGICAL_CHANNELS];
extern	u16			g_storeDataRemaindLen[MAX_LOGICAL_CHANNELS];

extern void dataManager_reset(void);//warning
extern u8 getCardResoure(u16 curGetDataTag, PX8 tempBuff, u8 appNum);
extern u8 dataManager_getData(PN8 sdDataObj);
extern void dataManager_dataManagerFunction(void);
extern u32 dataManager_getDataValue(PX8 buf, jref firstTagNodeRef, u16 tag, u16 curContentOffset, u16 maxLen, u8 is0070group,u8 isSend);
extern void dataManager_checkSequence(u8 p2);
extern u16 dataManager_calcTagAttri(u8 is0070Group);
extern u8 dataManager_storeData(PN8 sdDataObj);
extern jref dataManager_getSdDataRef(u8 sdId);
extern void dataManager_getCardRecognitionData(void);
extern u16 getCardRecognitionData(u8 sdId, PX8 arrayAddr);
extern u16 GetSDGeneraldata(u8 * buf, u16 tag, memref sdDataObj, u16 maxLen, u8 is0070group);
extern void dataManager_abortStoreDataThrowError(u16 error);
extern u16 getSeId(PX8 tempBuff);

#define	getCurStoreDataTag()					g_curStoreDataTag
#define getCurStoreDataOffset()	    			g_storeDataOffset[selected_channel]
#define getCurStoreDataTagLength()				g_curStoreDataTagLength[selected_channel]
#define dataManager_isInStoreDataProcess()		(0 != g_storeDataSequence[selected_channel])
#define dataManager_setBitTag(value)			(g_BitTagValue[selected_channel] = g_BitTagValue[selected_channel] | value)
#define dataManager_clearBitTag(value)			(g_BitTagValue[selected_channel] = g_BitTagValue[selected_channel] & value)

#if GP__DATA_MANAGER__FOR_MASK
	void dataManager_init(void);
	void dataManager_abortStoreData(void);
	#define dataManager_getBitTag(value)			(g_BitTagValue[selected_channel] = g_BitTagValue[selected_channel] & value)
#else
	#define dataManager_init()
	#define dataManager_getBitTag(value)		0
#endif

#endif
