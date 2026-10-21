#ifndef _NATIVEMETHOD_IMPL_H
#define _NATIVEMETHOD_IMPL_H

#include "profile.h"
#include "typejava.h"

#define	PROTOCOL_7816_T0				APDU_PROTOCOL_T0
#define PROTOCOL_7816_T1				APDU_PROTOCOL_T1
#define	PROTOCOL_14443_TYPEA			APDU_PROTOCOL_TA
#define	PROTOCOL_14443_TYPEB			APDU_PROTOCOL_TB

#define PRE_CONTEXT_VALUE			0x5A

/* Constants */
#define NOT_A_TRANSIENT_OBJECT          0x00
#define CLEAR_ON_RESET                  0x01
#define CLEAR_ON_DESELECT               0x02
#define INSTALL_METHOD_ADDRESS          0x01
#define PACKAGE_ID                      0x02
#define APPLET_INDEX                    0x03
#define INSTALL_METHOD_ADDRESS          0x01

#define OBJECT_NOT_FOUND 0xffffff01
#define OBJECT_IS_NOT_REMOTE 0XffffFF02
#define METHOD_NOT_FOUND 0xffffff03
#define OTHER_EXCEPTION 0xffffffff

#define MEMORY_TYPE_PERSISTENT			0
#define MEMORY_TYPE_TRANSIENT_RESET		1
#define MEMORY_TYPE_TRANSIENT_DESELECT	2

#define MAKE_TRANSIENT_ARRAY_COMMON_MODE  0
#define MAKE_TRANSIENT_ARRAY_GLOBAL_MODE  1
/* these native methods use for java API*/

extern  void callInstall(void);
extern  void initAIDImpl(void);//added by jiangxiaohui@20210615: 性能优化，EPX产业化\THC_THD89\Branch\EPX_THC_Mobile5G\Develop
extern void isSecurityDomain(void);
extern  void resolveE2C4commandNM(void);

/**memory operation**  these following native method are used for memory operation */

extern	u16 getAvailableMemory(void);
extern	boolean compareRemainedSpace(u16 wLen_, u16 wFlag_)														SMALL_MODE;

/**channel manager && fire wall**  these following native method are used for channel manage and fire wall */
//void channelManageNM();
extern	void getChannelStatusNM(void);
#define getMaxChannelsNM() pushShort(U8t16(logicalChannelNums))
extern	void getContextStatusNM(void);
extern  void manageAppFlagNM(void);
#define setCurrentlySelectedChannelNM() (selected_channel = U16t8( popShort()))
#define getCurrentlySelectedChannelNM() pushShort(U8t16(selected_channel))

/**fire wall**/
extern	void setChannelContextNM(void);
extern	void getChannelContextNM(void);
#define getCurrentContext() pushShort((s16)cc)
#define getPreviousContext() pushShort(getPreviousStackContext())
extern	void setObjectContext(void);
extern  void checkPreviousContextAccessNM(void);
extern	jref checkPreviousContextAccess(PF8 theObject, u16 type);
extern	void setJCREentry(void);
extern	void setPckObject(void);
extern	void checkAccess(void);
extern  void setJCREGlobal(void);

extern	void InitshortBuffer(void);
extern	void InitramBuffer(void);
extern	void initAPDUBuffer(void);
extern	void initCacheBuffer(void);
extern	void InitBuffer(void);

/**Transaction** these following native method are used for Transaction */
extern	void beginTransaction(u8 isMultiApduProcess);
extern	void abortTransaction(void);
extern	void commitTransaction(void);
//#define getTransactionDepth() (pushShort(((s16)isInSingleApduProgress()) || ((s16)isInMultiApduProgress())))
#define getTransactionDepth() (pushShort((s16)isInSingleApduProgress()))
#define getUnusedCommitCapacity() (pushShort((s16)get_unused_commit_capacity()))
#define getMaxCommitCapacity() pushShort(get_max_commit_capacity())

/**Array** these following native method are used for Array operated */
extern	void checkArrayArgs(void);
extern	void arrayCopy(void);
extern	void arrayCopyNonAtomic(void);
extern	void arrayFillNonAtomic(void);
extern  void arrayCompare(void);
extern	void setShort(void);

/*temporary array */
extern  jref make_Transient_Array(u8 type, u8 internalType, jref internalTypeClass, u8 event, u16 count, u8 mode)		SMALL_MODE;
extern  void makeTransientArray(u8 size_type, u8 internalType, jref internalTypeClass)  SMALL_MODE;
extern  void isTransient(void);

/*these following native methods used for GC  */
extern	void initGC(void);
extern	void initAD(void);
extern	void initPackageDeletionNM(void);
extern	void markAppRoots(void);
extern	void checkSIOaccess(void);
extern	void getAPI_VERSION(void);
extern	void markStatics(void);
extern	void startGC(void);
extern	void removePackageNM(void);
extern  void clearGlobalPkgAddrNM(void);
//void cleanupExportedObjectsArray(void);
extern	void releaseStaticRef(void);

/**reference table operation** these following native method are used for reference table operation */
extern	void addReferenceAddressNM(void);
extern	void getReferenceAddressNM(void);
extern	void setComponentInfoForPkg(void);
extern	void getPackageComponentLocationNM(void);
extern	void getMaxPackageIdentifier(void);
extern  void GetContentConfigValue(u8 tag);
extern	void getPackageExportComponent(void);
extern	void getPkgIdForAddress(void);
extern	void getClassNameAddress(void);
extern	void getNewIDNM(void);
extern	void getIndexByIDNM(void);
extern	void getSelectedAppletNM(void);
extern	void addEntryNM(void);
extern  void getEntryByInstAIDNM(void);
extern  void SearchEntryByInstAIDNM(void);
extern	void getPkgContextNM(void);
extern	void getPkgIdByContextNM(void);
extern  void getAvailableMemoryBuf(void);
extern  void makeGlobalArray(void);
/* * end of OS native methods!  add user native methods below this ! ***************************/


#if JCVM__IO__MIFARE
extern  void MF_readWrite_NM(void);
#endif

extern  void privateNativeMethod(u8 methodNumber);

#if JCVM__VM__RMI
extern	void cleanupExportedObjectsArray(void);
extern	void checkAccess(void);
extern	void getAnticollisionString(void);
extern	void getRemoteInterfaceNumber(void);
extern	void getRemoteInterfaceAddress(void);
extern	void copyInterfaceNameIntoBuffer(void);
extern	void copyStringIntoBuffer(void);
extern	void getObjectID(void);
extern	void getClassNameAddress(void);
extern	void getRemoteMethodInfo(void);
extern	void getReturnType(void);
extern	void deleteAllTempArrays(void);
extern	void isAPIException(void);
extern	void rmi_invoker_common(void);
extern	void getRMIErrorCode(void);
extern	void writeArray(void);
#endif

extern  u8	 nativeMethod_GetProtocol(void);
extern	u16  nativeMethod_GetCurrentAPDU(void);
extern	u16  nativeMethod_GetCurrentAPDUBuffer(void);
extern	void nativeMethod_SendBytes(void);
extern	void nativeMethod_SendBytesLong(void);
extern	void nativeMethod_SetOutgoingAndSend(void);
extern	void nativeMehtod_GetCLA(void);

extern  u16 nativeMethods_readSecurityCodeDataNative(void);
extern	void getShort_java(void);
extern  void GetNextAppEntryIdxByPriorityNM(void);

#if GP__HCI_API__FOR_MASK
extern  u8    nativeMethod_HciNoParaProcess(void);
extern	u8    nativeMethod_HciOneParaProcess(void);
extern	void  nativeMethod_HciMulParaProcess(void);
extern	void  nativeMethod_GetReceiveBufferHCI(void);
extern	void  nativeMethod_Swp_Hci_TransactionEvent_6paras(void);
extern	void  nativeMethod_Swp_Hci_TransactionEvent_3paras(void);
#endif

extern  void     setCurrentInvokeApplet(void);
extern	boolean  SetRemoteProcessRes(u16 offset, u16 len);
extern  void     initISDPrivilegeNM(void);
extern  void     setISDPrivilegeNM(void);
extern  void     setSDTarInfoNM(void);

extern  void SeInfoNM(void);
extern 	void GetAlgVersion(void);
extern  void GetChipIDdata(void);
//extern void NM_GetCOSVersionData(void);
extern void NM_GetCrcOrScidInfo(void);
#endif

//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP to support GP__AMENDMENT__H__FOR_MASK
extern  void Upgrade_BoolElementWrite(void);
extern  boolean Upgrade_BoolElementRead(void);

extern void isUsimADF_NM(void);

extern void Image$$ER_SEINFO_START$$Base;
#if BOOTLOADER_PRO
#define  ClearrfFirstSelectCmdFlag()
#else
extern  void ClearrfFirstSelectCmdFlag(void);
#endif


//modify by xuxiaojun@20221020: 根据 合代码时代码评审(20220722).xlsx进行修改： 将JAVA CARD305的本地方法放在nativemethod_impl_JavaCard305中
//extern void sensitiveResult(void);

//extern void arrayFill(void);
//extern void GetSignatureAlgorithm(void);
//extern void GetCipherAlgorithm(void);

//extern void setSensitiveArryObj(void);
//extern void isSensitiveArryObj(void);
//extern void clearArray(void);



