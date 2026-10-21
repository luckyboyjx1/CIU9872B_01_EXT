#ifndef __BOOTOSIMPL_H__
#define __BOOTOSIMPL_H__
/***************************************************************************
 External Functions
 ***************************************************************************/

#include "SysConfigProfile.h"
#include "DataOperation.h"
#include "SysConfigData.h"
#include "SysConfigTarget.h"

#if !BOOTLOADER_PRO
extern u16 getPrivateData(PX8 apduBuffer,u8 type);
extern u16 modifyAtrValue(u8 coldOrWarm, u8 hitoryOrWhole, PR8 buf, u8 length);
#endif
#if HAL__CONFIG_AREA__NEW

/***********************************************************************************
 * BOOT INS
 ***********************************************************************************/
#define 		BOOT_INS_EXTERNAL_AUTHENTICATE_PRE_OPERATOR     	0X80
#define 		BOOT_INS_EXTERNAL_AUTHENTICATE             		  	0X82
#define 		BOOT_INS_UPDAKEY            			   	        0XA0
#define 		BOOT_INS_SET_KEY_TRYCTR            			   		0XA1
#define 		BOOT_INS_SET_SYSTEM_STATE            			    0XF0
#define 		BOOT_INS_START_BOOT_OS     						    0X4F
#define 		BOOT_INS_UNLOCK           				            0XF1
#define 		BOOT_INS_SET_BASE_ADDRESS                		    0XAD
#define 		BOOT_INS_READ_EEP                       		    0XCB
#define 		BOOT_INS_WRITE_EEP                      		    0XE3
#define 		BOOT_INS_PATCH_MANAGE                    		    0X51
#define 		BOOT_INS_STORE_DATA					                0XE2
#define 		BOOT_INS_GET_DATA					                0XCA

#define 		INS_GET_CRC_NEW								  		0X30
#define 		INS_VERIFY_CRC_NEW									0X32
#define 		INS_DERIVE4UID_FROM7UID 							0x3F
#define 		INS_GET_CRC              			  				0X50
#define 		INS_NXPP6_MIFAREFUNC								0x52
#define 		INS_VERIFY_CRC           							0X54
#define 		INS_ISSUE								        	0X56

#define 		INS_ATTAIN_MEMORY_CODE  							0X01
#define 		INS_VERIFY_MEMORY_CODE  							0X02

/********************************BOOT 密钥结构（总长度为22 bytes）**************************************
 	 **       密钥结构：          密钥值（KV） || 密钥使用计数器(KEYCTR)  ||密钥尝试计数器(TRYCTR)  || 掩码值索引(MASKCODEINDEX) || 校验值(CHECKVALUE)
	 **         字节数：          16          || 2                      ||1                      || 2                        || 1
**********************************************************************************************************************/
#define    		BOOT_KV_INDEX          		     					0X00
#define    		BOOT_KEYCTR_INDEX          		 					0X10
#define    		BOOT_TRYCTR_INDEX          		 					0X12
#define   		BOOT_MASKCODEINDEX_INDEX           					0X13
#define    		BOOT_CHECKVALUE_INDEX          	 					0X15


/***********************************************************************************
 * BOOT 密钥基址
 ***********************************************************************************/

#define      	BOOT_UL_KEY_BASE			      		    		BOOT_CONFIG_KEY_BASE      //解锁管理认证密钥
#define      	BOOT_CM_KEY_BASE			      					(BOOT_UL_KEY_BASE+BOOT_KEY_STRUCT_LEN)   //配置管理认证密钥
#define      	BOOT_PM_KEY_BASE			      					(BOOT_CM_KEY_BASE+BOOT_KEY_STRUCT_LEN)   //补丁管理认证密钥
#define      	BOOT_NM_KEY_BASE			      					(BOOT_PM_KEY_BASE+BOOT_KEY_STRUCT_LEN)   //内存管理认证密钥
#define         BOOT_KEY_TRY_COUNTER_BASE                       	(BOOT_NM_KEY_BASE+BOOT_KEY_STRUCT_LEN)
#define      	BOOT_LIFECYCLE_BASE			      					(BOOT_KEY_TRY_COUNTER_BASE+1)   //生命周期状态（1 byte）
#define      	BOOT_COMMAND_OLD_COMPONENT_BASE				    	(BOOT_LIFECYCLE_BASE+1)          //option 5A:不兼容；A5：兼容。
#define         BOOT_COMMON_KEY_BASE                            	(BOOT_COMMAND_OLD_COMPONENT_BASE+1)

#define         BOOT_CONFIG_KEY_END									(BOOT_CONFIG_KEY_BASE+BOOT_KEY_DATA_AREA_SIZE)

#define         BOOT_CONFIG_MASKCODE_BASE                      		ROMBase
#define         BOOT_MASKCODE										0XED
#define      	BOOT_SUPPORT_COMPONENT_PRIVATE_COMMAND				0xA5
#define      	BOOT_NOT_SUPPORT_COMPONENT_PRIVATE_COMMAND			0x5A


/***********************************************************************************
 *参数的长度、大小
 ***********************************************************************************/
#define 		BOOT_CONFIG_KEY_SIZE                        		0X70 //NM_key+PM_key+CM_key+UL_key+CMRE_LIFECYCLE
#define    		BOOT_SINGL_KEY_LEN      							0X10
#define    		BOOT_CRYPT_DATA_LEN     							0X13
#define    		BOOT_KEY_STRUCT_LEN     							0X16
#define    		BOOT_MAX_AUTH_NUM  		     						0X0A

/***********************************************************************************
 * BOOT 状态值：
 ***********************************************************************************/
#define 		NVM_DEFAULT_VALUE			                    	0xFF
#define 		NVM_DEFAULT_VALUE_TWO_BYTE	                    	0xFFFF
#define    		BOOT_READY_STATE                                	NVM_DEFAULT_VALUE
#define    		BOOT_INITIALIZED_STATE								0x01
#define    		BOOT_SECURED_STATE									0x02
#define    		BOOT_LOCKED_STATE									0x03
#define    		BOOT_TERMINATED_STATE								0x04

/***********************************************************************************
 * BOOT 特权
 ***********************************************************************************/
#define			BOOT_PRIVILEGE_INIT     					 		0X00
#define   		BOOT_PRIVILEGE_CM       							0X01
#define   		BOOT_PRIVILEGE_PM        					 		0X02
#define   		BOOT_PRIVILEGE_NM       							0X04
#define   		BOOT_PRIVILEGE_UL       					 		0X08
#define   		BOOT_PRIVILEGE_UL_MASK     							0X07
#define   		BOOT_PRIVILEGE_MASK     							0X0F
#define       	BOOT_BOOT_OS_START_MASK             				0X80



/***********************************************************************************
 * BOOT 校验标记
 ***********************************************************************************/
#define   		BOOT_INTERNAL_VERIFY_INIT    						0x00
#define   		BOOT_INTERNAL_VERIFY_OK       						0x10

#define    		BOOT_KEY_VALUE_OK  		         					0X01
#define    		BOOT_KEY_VALUE_FAIL  		         				0X00
/***********************************************************************************
 *BOOT  判断是否已有该权限
 ***********************************************************************************/
#define    		BOOT_IS_HAD_CURRENT_PRIV(p)  						((U8_AND_U8(p,Boot_CurPrivilege) != 0)? true : false)


#define    		BOOT_VERIFY_KEY_CHECK_VALUE   		         		0X01
#define    		BOOT_VERIFY_KEY_INIT_VALUE  		         		0X02

/***********************************************************************************
 *BOOT  scp
 ***********************************************************************************/
#define 		BOOT_SC0102_ICV_FOR_CMAC							0x10
#define 		CONSTANTS_S_ENC   									0x82
#define 		CONSTANTS_C_MAC   									0x01
#define 		CONSTANTS_DEK     									0x81


/***************************************************used for BOOT CONFIG***********************************************************/
extern u8 											         		Boot_CurPrivilege;
extern u8 											         		Boot_internal_verify_flag;

/***********************************************************************************
 * BOOT 接口API
 ***********************************************************************************/
#define SECURITY_STATE_TRANKEY_VERIFIED     						0x01
#define SECURITY_STATE_MFKEY_VERIFIED       						0x02
#define SECURITY_STATE_ECPKEY_VERIFIED								0x03
#define SECURITY_STATE_ATTACK   									0x00
#define KEY_CREATED  												0xA3
#define MAX_TRY_TIME 												0x06
#define PLATFORM_ADM_KEY_LEN  										0x10
#define TRANSPORT_KEY_LEN											0x10
#define MF_DEFAULT_KEY_VERSION										0x16
#define VGP_CONFIG_INDEX											0x17

#define GET_LAST_BASIC_PACKAGE										0
#define CHECK_CARD_CREATED											1
#define GET_RSA_KEY_FLAG											2

#define MAX_CLEAR   						                        0x8000
#define NVM_FILL_CHARACTER 											0x00

u16 boot_ExternalAuthenticatePreOperator(void);
u16  boot_VerifyExternalAuthenticate(void);
u16  boot_UpdateKey(void);
u16  boot_CheckIsAuthenticated (void);
void boot_SetLifeCycleValue(u8 lifeCylcle);
void boot_SetSecuritychannel(u8 privilege);
void boot_SetTryCTR(PM8 bootkeybase);
u16  boot_SetCardState(void);
u8   boot_GetLifeCycle(void);
u8   boot_GetTryCTR (PM8 bootkeybase);
u16  boot_GetKeyCTR (PM8 bootkeybase);
u8   boot_GetMaskCodeFromIndex(u16 index);
memref  boot_GetKeyAddress(u8 privilege);
u8   boot_CalcCheckValue (PX8 src, u16 keyLen);
u8   boot_VerifyCheckValue (PM8  bootkey, u8 mode);
void boot_CloseSecuritychannel (void) ;
void boot_DecrytoKey(PM8 cipherKey, PX8 outbuff);
void boot_GenerateSessionKey(PN8 staticKey, u16 sequenceCounter);
u8   boot_IsSupportPrevission(void);
u16  boot_SetBaseAddress (PX8 apduBuffer);
u16  boot_ReadEEP (PX8 apduBuffer);
u16  boot_WriteEEP (PX8 apduBuffer);
u16  boot_EncryptCommandData(PX8 inBuff, u16 inLen, PM8 keyBuff);
void initBootSecureGlobalVariable(void);
u16  boot_bootOSstart(void);
u16  boot_unwrap(u8 targetPrivilege);
u16  boot_SetKeyTryCounter(void);
void initBootSCPsessionKey(memref keyBuffer);
 
u16 clearAllEeprom(PX8 apduBuffer);
void PrivateSetOutgoingAndSend(u16 bOff, u16 len);
u16  BlockSelect(PX8 apduBuffer);
u16  attainAndVerifyMemoryCode(PX8 apduBuffer);
u16  Write_EEPROM(PX8 apduBuffer);
u16  Read_EEPROM(PX8 apduBuffer);
u16  loadPlatformAdmKey(PX8 apduBuffer);
u16  verifyPlatfromAdmKey(PX8 apduBuffer);
u16  getConfig(u8 configType);
u16  ECP_ManagerMifare(PX8 apduBuffer);
u16  Write_EEPROM_Page(PX8 apduBuffer);
u16  ProcessRemoteComponentCmd(PX8 apduBuffer);
u16  setSeVerInfo(void);
u16  clearCard(void);
u8   secondAtrOperate(PX8 commandBuffer, u8 operateType);
u8 boot_verifyMAC(void);
u8 bootGetKey(u8 keyType, PX8 keyBuffer, u8 keyBufferOffset);
void bootSetKey(u8 keyType, PX8 keyBuffer, u8 keyBufferOffset, u8 keylength);

#endif

#endif
