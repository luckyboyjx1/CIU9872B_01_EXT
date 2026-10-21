#ifndef  __CONTENT_MANAGER_H__
#define  __CONTENT_MANAGER_H__
#include "typejava.h"

#define APP_FIRST                    	0
#define ISD_APPID                    	APP_FIRST
#define UICC_APPID                       0x01		//USIM  Applet APPID	//去警告，防止重复定义，调整到此处定义

#define ILLEGAL_APPID                      0xFF	//去警告，防止重复定义，调整到此处定义
#define ILLEGAL_PKGID                      0xFF	//去警告，防止重复定义，调整到此处定义

//#define ISDR_APPID                       0x02       //ISD-R Applet APPID
//#define ECASD_APPID						 0x03       //ECASD Applet APPID
//#define ISDP_APPID                       0x04		//ISD-P Applet APPID

#define APP_STATE_NONE         		 	0x00
#define APP_STATE_PRELOAD      		 	0x02
#define APP_STATE_LOADED       		 	0x01
#define APP_STATE_REGISTERED   		 	0x03
#define APP_STATE_SELECTABLE   		 	0x07


#define APP_FLAG_END  					256
#define APP_FLAG_SIZE					32		//32 when iAppIdFlag and iMutiFlag are defined int array
#define APP_NULL  						0xFF	     // invalid applet ID
#define APP_INDEX_NULL					0xFFFF


#define CLR_ID_FLAG						0
#define SET_ID_FLAG						1
#define GET_APP_COUNT					2
#define CLR_MULTI_FLAG					3
#define SET_MULTI_FLAG					4
#define CHECK_MULTI_FLAG				5
#define SET_EXTRA_FLAG					6
#define CLR_EXTRA_FLAG					7
#define CHECK_EXTRA_FLG					8


//AID STRUCT
#define AIDARRAY_INDEX_OF_AID 			0

#define  BAD_ADDRESS  					0xFF
#define  FFFF  							0xffff
#define  ILLEGAL_ADDRESS  				0
#define  NULL_REF  						0
#define  ILLEGAL_ID  					0xFF
#define  ILLEGAL_INDEX  				0xFF
#define  ILLEGAL_TOKEN  				0xFF

#define CLASS_COMPONENT_INDEX  			0
#define METHOD_COMPONENT_INDEX  		1
#define STATIC_FIELD_COMPONENT_INDEX  	2
#define EXPORT_COMPONENT_INDEX  		3

#define GET_AID_ARRAY					(u8)0
#define GET_MEM_MANAGE					(u8)1
#define GET_SDID						(u8)2
#define GET_FirstSDID					(u8)3

#define NEED_VOLATILE_AMENDMENTC_SEARCH	 (u8)0X81
#define VOLATILE_AMENDMENTC_APP_EXIST 	 (u8)0X01

#define SEARCH_NEXT						(s8)1
#define SEARCH_PREVIOUS	    			(s8)-1
#define SEARCH_LAST						(s8)0
#define SEARCH_PASS_TO_CURRENT_APP		(s8)2
#define SEARCH_ERROR					(s8)3


//extern  void appMgrcommit(void);
extern	void PackageMgrCommit(void);
extern  void tempConstantObjFree(void);
extern	jref initAID(PN8 bArray, u16 offset, u8 length)							            SMALL_MODE;
extern	jref newPackageEntry(jref aid, u8 major, u8 minor, u8 capMinor)						SMALL_MODE;
extern	boolean AID_equals_array(jref aid, PF8 bArray, u16 offset, u16 length)				SMALL_MODE;
extern	boolean AID_partialEquals_array(jref aid, PF8 bArray, u16 offset, u16 length)		SMALL_MODE;
extern	u8 findPkgID(PF8 aid, u16 offset, u16 length, u8 isCheckVersion)					SMALL_MODE;
extern	jref getPkgAID(jref PkgEntry);
extern	void releaseStaticFeild(u16 appContext)												SMALL_MODE;
extern	jref getPkgEntry(u8 index)															SMALL_MODE;
extern	u16 getPkgContext(u8 pkgID)															SMALL_MODE;
extern	void addAppletPackage(u8 pkgID)														SMALL_MODE;
extern	u8 manageAppFlag(u8 appID,u8 appFlagManageType)										SMALL_MODE;
extern	u8 getNewID(void);
//extern	jref newAppTableEntry(void);
extern	jref addEntry(jref classaid, boolean zIsInst)										SMALL_MODE;
extern	jref addAppletInfo(jref aid, u8 bContext, u16 installoff)							SMALL_MODE;
extern	void setImportInfo(PF8 bArray, u8 offset, u8 count)									SMALL_MODE;
extern	u16 getIndexByID(u8 AppID)															SMALL_MODE;
extern	u16 getIndexByPageId(u8	PageID)                                                     SMALL_MODE;
extern	jref getSelectedApplet(void);
extern	memref getAppTable(void);
extern  u16 getAppletFiled(u16 index, u8 getType)                                         	SMALL_MODE;
extern	void getManagerAID(void);
extern	void getISDRAID(void);
extern	u8 getPkgIdByContext(u8 pkgContext)                                                 SMALL_MODE;
extern 	u8 getAssociateSDId(u8 AppID);
extern  memref getSdEntryObj(u8 appId);
#define getAppletAid(id)  getAppletFiled(id, GET_AID_ARRAY)
extern  u16 GetAppEntryIdxByPriority(u16 bStartIndex, s8 direction);
u16 getVolatileApplet_AmendmentC(u16 bStartIndex, s8 direction);
extern	u16 GetNextAppEntryIdxByPriority(u16 bStartIndex);
extern 	u16 GetImplicitSelectedAppletId(u8 ioInterface,u8 logicalChannel);
extern	u16 GetImplicitSelectedAppletIdNM(void);
extern u16 getEntryByInstAID(u16 classaid);
extern u16 SearchEntryByInstAID(u16 classaid);
#define getAppAssociatedSdId(appEntryObj)                               		nvmReadU8(MEMtPN8(get_object_field_ref(appEntryObj, bsdId_INDEX)))
//extern void E2P_free(memref address, u32 chunksize);
extern  void setPkgCountLocked(void);
extern	void setAppletCountLocked(void);
extern void GetGBAAppletEntry(void);
extern void SaveICCIDAndAID();
#endif
