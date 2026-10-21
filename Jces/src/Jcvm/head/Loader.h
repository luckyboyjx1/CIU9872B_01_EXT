#ifndef  __LOADER_H__
#define  __LOADER_H__
#include "typejava.h"

#define  INSTALLER_MAJOR	0
#define  INSTALLER_MINOR	7
#define  INTEGER_MODE		1

#define  WAIT_4_TAG			0
#define  WAIT_4_LEN_H		1
#define  WAIT_4_LEN_L		2
#define  WAIT_4_DATA		3

typedef struct
{
	void (*init)();
	boolean (*process)(void);
	boolean (*postProcess)(void);
} ComponentEntry;

//boolean verifyAPDUOrder(void);
extern u16 resolveE2C4command(void);
extern void load(void);
//void cmpLoad(void);
extern void setLoaderStatus(void);
extern void install(void);

/*
 * CLS for the installer
 */
#define  INSTALLER_CLA  					0x80

/* INS for the installer	*/
#define  INS_CAP_BEGIN        				0xb0
#define  INS_CAP_END          				0xba
#define  INS_COMPONENT_BEGIN  				0xb2
#define  INS_COMPONENT_END    				0xbc
#define  INS_COMPONENT_DATA   				0xb4
#define  INS_APPLET_INSTALL   				0xb8
#define  INS_CAP_ABORT        				0xbe

#define  CAP_COMPONENT_HEADER       		1
#define  CAP_COMPONENT_DIRECTORY    		2
#define  CAP_COMPONENT_APPLET       		3
#define  CAP_COMPONENT_IMPORT       		4
#define  CAP_COMPONENT_CONSTANTPOOL 		5
#define  CAP_COMPONENT_CLASS        		6
#define  CAP_COMPONENT_METHOD       		7
#define  CAP_COMPONENT_STATICFIELD  		8
#define  CAP_COMPONENT_REFERENCELOCATION  	9
#define  CAP_COMPONENT_EXPORT      			10
#define  CAP_COMPONENT_DESCRIPTOR  			11
#define  CAP_COMPONENT_DEBUG       			12

/* number of (required) components + 1	  */
#define  INSTALLER_MAX						13

/* CAP file download order		  */
#define  ORDER_HEADER						1
#define  ORDER_DIRECTORY					2
#define  ORDER_IMPORT						3
#define  ORDER_APPLET						4
#define  ORDER_CLASS						5
#define  ORDER_METHOD						6
#define  ORDER_STATICFIELD					7
#define  ORDER_EXPORT						8
#define  ORDER_CONSTANTPOOL					9
#define  ORDER_REFERENCELOCATION			10
#define  ORDER_DESCRIPTOR					11
#define  ORDER_DEBUG                        12
/*misc.			*/
#define  CAP_MAJOR							2
#define  CAP_MINOR							2

#define  MIN_AID_LENGTH						5
#define  MAX_AID_LENGTH						16
#define  INSTANCE_MAX						255
#define  PACKAGE_METHOD_MAX					128

#define  FLAG_CLASS							1
#define  FLAG_FIELD							2
#define  FLAG_METHOD						3

#define  CAP_MAGIC1							0xDE
#define  CAP_MAGIC2							0xCA
#define  CAP_MAGIC3							0xFF
#define  CAP_MAGIC4							0xED
#define  CAP_MAGIC							0xDECAFFED

#define  TYPE_BOOLEAN						2
#define  TYPE_BYTE							3
#define  TYPE_SHORT							4
#define  TYPE_INT							5

#define  COMP_HEADER_SIZE					3
#define  U1_SIZE							1
#define  U2_SIZE							2
#define  U4_SIZE							4

#define  INSTALLER_STATE_ERROR				0xff
#define  INSTALLER_STATE_READY				0x00
#define  INSTALLER_STATE_LOADING			0x2
#define  INSTALLER_STATE_CREATING			0x4

/* CAP file masks  */
#define  ACC_INT							0x01
#define  ACC_EXPORT							0x02
#define  ACC_APPLET							0x04
#define  ACC_INTERFACE						0x80
#define  ACC_REMOTE							0x20
#define  ACC_SHAREABLE						0x40
#define  ACC_EXTENDED						0x80
#define  ACC_ABSTRACT						0x40
#define  ACC_PUBLIC							0x01
#define  ACC_FINAL							0x10

#define  METHOD_COMP_LIMIT					32767

/* header_component offsets		*/
#define  OFFSET_HEADER_MAGIC				3
#define  OFFSET_HEADER_MAGIC1				3
#define  OFFSET_HEADER_MAGIC2				4
#define  OFFSET_HEADER_MAGIC3				5
#define  OFFSET_HEADER_MAGIC4				6
#define  OFFSET_HEADER_MINOR				7
#define  OFFSET_HEADER_MAJOR				8
#define  OFFSET_HEADER_FLAGS				9
#define  OFFSET_HEADER_PKG					10

/* package_info offsets		 */
#define  OFFSET_PKG_MICRO					0
#define  OFFSET_PKG_MINOR					1
#define  OFFSET_PKG_MAJOR					2
#define  OFFSET_PKG_AID_LEN					3
#define  OFFSET_PKG_AID						4
#define  OFFSET_PACKAGE_INFO_MICRO			0
#define  OFFSET_PACKAGE_INFO_MINOR			1
#define  OFFSET_PACKAGE_INFO_MAJOR			2
#define  OFFSET_PACKAGE_INFO_AID_LENGTH		3
#define  OFFSET_PACKAGE_INFO_AID			4

/* directory_component offsets	 */
#define  OFFSET_DIRECTORY_BASIC_COUNT		3
#define  OFFSET_DIRECTORY_CUSTOM_COUNT		4
#define  OFFSET_DIRECTORY_BASIC_COMPONENTS  5

/* basic_components		  */
#define  OFFSET_BASIC_COMPONENTS_TAG		0
#define  OFFSET_BASIC_COMPONENTS_SIZE		1
#define  BASIC_COMPONENTS_MEMBER_SIZE		3

/* custom_components	   */
#define  OFFSET_CUSTOM_COMPONENTS_TAG		0
#define  OFFSET_CUSTOM_COMPONENTS_SIZE		1
#define  OFFSET_CUSTOM_COMPONENTS_AID_LENGTH	3
#define  OFFSET_CUSTOM_COMPONENTS_AID			4

/* applet_component offsets	 */
#define  OFFSET_APPLET_COUNT					3
#define  OFFSET_APPLET_APPLETS					4
#define  OFFSET_APPLETS_AID_LENGTH				0
#define  OFFSET_APPLETS_AID						1

/* imports_component offsets   */
#define  OFFSET_IMPORT_COUNT					3
#define  OFFSET_IMPORT_PACKAGE_INFO				4

/* constant_pool_component offsets	 */
#define  OFFSET_CP_COUNT						3
#define  OFFSET_CP_INFO							5
#define  CAP_CP_CELL_SIZE						4
#define  CP_CELL_SIZE							2
/*#define  CP_CELL_SIZE  4	 */
#define  TABLE_CELL_SIZE						2

/* class_component offsets	 */
#define  OFFSET_CLASS_INFO						0
#define  OFFSET_CLASS_FLAGS						0
#define  OFFSET_CLASS_SUPER_REF					1
#define  OFFSET_CLASS_DEC_INST_SIZE				3
#define  OFFSET_CLASS_REF_INDEX					4
#define  OFFSET_CLASS_REF_COUNT					5
#define  OFFSET_CLASS_PUB_METH_BASE				6
#define  OFFSET_CLASS_PUB_METH_COUNT			7
#define  OFFSET_CLASS_PKG_METH_BASE				8
#define  OFFSET_CLASS_PKG_METH_COUNT			9
#define  OFFSET_CLASS_PUB_METH_TABLE			10
#define  OFFSET_CLASS_INTERFACE_INFO			0
#define  OFFSET_CLASS_INTERFACE_INFO_CLASS_REF  0
#define  OFFSET_CLASS_INTERFACE_INFO_COUNT		2
#define  OFFSET_CLASS_INTERFACE_INFO_INDEX		3
#define  OFFSET_REMOTE_METHOD_SIGNATURE			2
#define  OFFSET_SIGNATURE_POOL_DATA				2

#define  REMOTE_METHOD_INFO_SIZE				5
#define  INTERFACE_INFO_SIZE					1

/* method_component offsets		  */
/*#define  OFFSET_METHOD_EXCEPTION_HANDLER_COUNT  3
#define  OFFSET_METHOD_EXCEPTION_HANDLER_INFO  4	*/
#define  EXCEPTION_HANDLER_INFO_SIZE				8
#define  OFFSET_EXCEPTION_HANDLER_START_OFFSET		0
#define  OFFSET_EXCEPTION_HANDLER_ACTIVE_LENGTH		2
#define  OFFSET_EXCEPTION_HANDLER_HANDLER_OFFSET	4
#define  OFFSET_EXCEPTION_HANDLER_CATCH_TYPE_INDEX  6

#define  OFFSET_METHOD_NARGS						1
#define  OFFSET_EXTENDED_METHOD_NARGS				2

/* static_field_component offsets	  */
#define  OFFSET_STATICFIELD_BYTE_COUNT				3
#define  OFFSET_STATICFIELD_REFERENCE_COUNT			5
#define  OFFSET_STATICFIELD_ARRAY_INIT_COUNT		7
#define  OFFSET_STATICFIELD_ARRAY_INIT_INFO			9

/* array_init_info offsets	   */
#define  OFFSET_ARRAY_INIT_INFO_TYPE				0
#define  OFFSET_ARRAY_INIT_INFO_COUNT				1
#define  OFFSET_ARRAY_INIT_INFO_VALUES				3

/* export_component offsets	   */
#define  OFFSET_EXPORT_CLASS_COUNT					3
#define  OFFSET_EXPORT_CLASS_EXPORT_INFO			4
#define  OFFSET_EXPORT_COUNT						0
#define  OFFSET_EXPORT_CLASS_INFO					1
#define  OFFSET_EXPORT_INFO_CLASS_OFFSET			0
#define  OFFSET_EXPORT_INFO_STATIC_FIELD_COUNT		2
#define  OFFSET_EXPORT_INFO_STATIC_METHOD_COUNT		3
/*#define  OFFSET_EXPORT_INFO_STATIC_FIELD_OFFSETS  4		 */
#define  EXPORT_INFO_STATIC_FIELD_OFFSETS_SIZE		2
#define  EXPORT_INFO_STATIC_METHOD_OFFSETS_SIZE		2
#define  CLASS_EXPORT_INFO_CELL_SIZE				6

#define  OFFSET_CONSTANT_TAG						0
#define  OFFSET_CONSTANT_COUNT						3
#define  OFFSET_ON_CARD_CONSTANT					2
#define  OFFSET_CONSTANT_CLASSREF					1
#define  OFFSET_CONSTANT_PKG_TOKEN					1
#define  OFFSET_CONSTANT_CLASS_TOKEN				2
#define  OFFSET_CONSTANT_OFFSET						2
#define  OFFSET_CONSTANT_TOKEN						3
#define  OFFSET_CONSTANT_FLD_TOKEN					3
#define  OFFSET_CONSTANT_METH_TOKEN					3

#define  CLASS_REF_SIZE								2

/* constant pool tags	  */
#define  CONSTANT_CLASSREF							1
#define  CONSTANT_INSTANCEFIELDREF					2
#define  CONSTANT_VIRTUALMETHODREF					3
#define  CONSTANT_SUPERMETHODREF					4
#define  CONSTANT_STATICFIELDREF					5
#define  CONSTANT_STATICMETHODREF					6

/* reference_location_component offsets	   */
#define  OFFSET_REFLOC_BYTE_INDEX_COUNT				3
#define  OFFSET_REFLOC_BYTE_INDICES					5

#define  OFFSET_CLA 								0
#define  OFFSET_INS									1
#define  OFFSET_P1									2
#define  OFFSET_P2									3
#define  OFFSET_LC									4
#define  OFFSET_CDATA								5

#define	APDU_HEAD_LENGTH							5

#define ERROR_CAP_MAGIC              				0x6402
#define ERROR_CAP_MINOR              				0x6403
#define ERROR_CAP_MAJOR              				0x6404
#define ERROR_INTEGER_UNSUPPORTED    				0x640b
#define ERROR_DUP_PKG_AID            				0x640c
#define ERROR_DUP_APPLET_AID         				0x640d
#define ERROR_ABORTED                				0x640f
#define ERROR_STATE                  				0x6421
#define ERROR_COMP_ORDER             				0x6A80 //0x6422
#define ERROR_EXCEPTION              				0x6424
#define ERROR_COMMAND_ORDER          				0x6425
#define ERROR_COMP_TAG               				0x6428
#define ERROR_INSTRUCTION            				0x6436
#define ERROR_ON_CARD_PKG_MAX_EXCEEDED  			0x6437
#define ERROR_IMPORT_NOT_FOUND       				0x6438
#define ERROR_PKG_ID                 				0x6439
#define ERROR_PKG_METHOD_MAX_EXCEEDED				0x6442
#define ERROR_APPLET_NOT_FOUND       				0x6443
#define ERROR_APPLET_CREATION        				0x6444
#define ERROR_INSTANCE_MAX_EXCEEDED  				0x6445
#define ERROR_ALLOCATE_FAILURE       				0x6446
#define ERROR_IMPORT_CLASS_NOT_FOUND 				0x6447
#define NO_EXCEPTION                 				0x0000

//handle E2 and C4
#define E2_TAG                   					0
#define E2_SIZE_TAG              					1
#define E2_SIZE_ONEBYTE          					2
#define SD_TAG                   					3
#define SD_LEN                   					4
#define SD_VALUE                 					5
#define SIGDATA_TAG              					6
#define SIGDATA_SIZE_TAG		 					7
#define SIGDATA_SIZE_ONEBYTE     					8
#define SIGDATA_VALUE            					9
#define C4_TAG                   					10
#define D4_TAG                   					11
#define C4_SIZE_TAG              					12
#define D4_SIZE_TAG              					13
#define C4_SIZE_ONEBYTE          					14
#define C4_SIZE_TWOBYTE          					15
#define C4_VALUE                 					16
#define E2E4_FINISH              					17
#define C4_SIZE_THREEBYTE          					18
#define D3_TAG                   					19 //merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP
#define D3_LEN                   					20
#define D3_VALUE                   					21

#define RESOLVE                   					0
#define GET_TAG                   					1
#define GET_SD_LEN                					2
#define GET_SIGDATA_LEN           					3
#define INIT_OFFSET               					4

#define INITRESULT									0
#define GOAHEAD										1
#define NEXTAPDU									2
#define FINISH										3

/* Heard Component Status	  */
#define STATE_READY 								0
#define STATE_MINOR									1
#define STATE_MAJOR									2
#define STATE_FLAGS									3
#define STATE_PKG_MINOR								4
#define STATE_PKG_MAJOR								5
#define STATE_AID_LENGTH							6
#define STATE_AID_DATA								7
#define HC_STATE_PKGNAMELEN 						8
#define HC_STATE_PKGNAME    						9

/* DERECTORY Componet status */
#define HEADER_SIZE									0
#define DERETORY_SIZE								1
#define APPLET_SIZE									2
#define IMPORT_SIZE									3
#define CONSTANT_SIZE								4
#define CLASS_SIZE									5
#define METHOD_SIZE									6
#define STATICFILED_SIZE							7
#define REFLOCATION_SIZE							8
#define EXPORT_SIZE									9
#define DESCRIPTOR_SIZE								10
#define DEBUG_SIZE									11
#define STATIC_IMA_SIZE								12
#define DERETORY_FIN								13
#define IGNORE_SIZE									14


/* Applet Component Status	 */
#define AC_STATE_COUNT   							0
#define AC_STATE_LENGTH  							1
#define AC_STATE_AID     							2
#define AC_STATE_OFFSET  							3

/* process states		   */
#define  IC_STATE_COUNT  							0
#define  IC_STATE_MINOR  							1
#define  IC_STATE_MAJOR  							2
#define  IC_STATE_LENGTH  							3
#define  IC_STATE_AID  								4
#define  IC_STATE_DONE  							5
#define  IC_STATE_NEXT  							6

/*static filed Component  Status	*/
#define SC_STATE_READY            					0
#define SC_STATE_IMAGE_SIZE      					1
#define SC_STATE_REF_COUNT       					2
#define SC_STATE_AI_COUNT        					3
#define SC_STATE_AI_INFO_TYPE     					4
#define SC_STATE_AI_INFO_COUNT    					5
#define SC_STATE_AI_INFO_VALUES   					6
#define SC_STATE_DEFAULT_COUNT   					7
#define SC_STATE_ND_COUNT        					8
#define SC_STATE_ND_VALUES        					9
#define SC_STATE_DONE             					10
#define SC_STATE_NEXT             					11
#define SC_BYTE_AS_TRUE           					0x01

/*  ReferenceLocationComponent  Status	  */
#define  RL_STATE_READY             				0
#define  RL_STATE_BYTE_INDEX_COUNT  				1
#define  RL_STATE_BYTE_INDICES      				2
#define  RL_STATE_BYTE2_INDEX_COUNT					3
#define  RL_STATE_BYTE2_INDICES     				4
#define  RL_STATE_DONE              				6
#define  RL_STATE_NEXT              				7

/*export Component  Status			  */
#define EC_STATE_READY            					0
#define EC_STATE_CLASS_COUNTER    					1
#define EC_STATE_CLASS_OFF        					2
#define EC_STATE_SF_COUNT         					3
#define EC_STATE_SM_COUNT        					4
#define EC_STATE_SF_OFFSETS      					5
#define EC_STATE_SM_OFFSETS       					6

#define isExternal(address)   ((jshort)(address & 0x8000 ) == (jshort)0x8000)
//s16 linkOneConstant(jbyte tag, jbyte b1, jbyte b2, jbyte b3);
//jboolean hasMoreData(void);
extern void setLoaderException(jshort exceptioncode);
extern void resetLinker(void);
//void resetSegmentProcessor(void);
//extern void saveBytes(u16 offset, u16 count);
//extern memref component_allocate(u8 tag);
extern void componentInit(void);
//extern boolean componentLoad(void);
extern boolean postProcess(void);

/* heard component	  */
extern	void headerComponentInit(void);
extern	boolean headerComponentProcess(void);

/* directory Component	*/
extern	void DirectoryComponentInit(void);
extern	boolean DirectoryComponentProcess(void);

/* Applet Component	 */
extern	void AppletComponentInit(void);
extern	boolean AppletComponentProcess(void);

/* import Component	  */
extern	void ImportComponentInit(void);
extern	boolean ImportComponentProcess(void);

/* Constant Component	*/
extern	void ConstantComponentInit(void);
extern	boolean ConstantComponentProcess(void);
extern	boolean ConstantComponentPostProcess(void);

/*class component	  */
extern	void		ClassComponentInit(void);
extern	boolean ClassComponentProcess(void);
extern	boolean ClassComponentpostProcess(void);

/* method component	  */
extern	boolean MethodComponentProcess(void);
extern	boolean MethodComponentpostProcess(void);

/* StaticField Component	*/
extern	void StaticFieldComponentInit(void);
extern	boolean StaticFieldComponentprocess(void);

/* export Component*/
extern	boolean ExportComponentProcess(void);
extern	boolean ExportComponentpostProcess(void);

/* ReferenceLocation Component */
extern	void ReferenceLocationComponentInit(void);
extern	boolean ReferenceLocationComponentprocess(void);

#define isInterface(bitfield)					(U8_AND_U8(bitfield, ACC_INTERFACE) == ACC_INTERFACE)
#define isRemote(bitfield)						(U8_AND_U8(bitfield, ACC_REMOTE) == ACC_REMOTE)

#define isHighbitOn(token)						(U8_AND_U8(token, MASK_EXTERNAL) == MASK_EXTERNAL)
#define isU8External(address)					(U8_AND_U8(address, 0x80) == 0x80)
#define isU16External(address)					(U16_AND_U16(address, 0x8000) == 0x8000)
#define getShort(buffer,offset)					U16_OR_U16(U16_SHL_U8(U8t16(buffer[offset]), 8), buffer[U8_A_U16(1, offset)])
#define setLoaderException(exceptioncode)		PXB_W_U16(MEMtPX8(&bShortBuffer[INSTALLER_EXCEPTION]), exceptioncode)
#define isSplitValue(valueSize)					(U16_A_U8(g_offset, valueSize) > g_max)
#define hasMoreAPDU()							((g_componentSize > g_dataSize) ? 1:0)

//memref getClassCompAddressForClassAddress(memref classAddr);
//jshort getMethodReference(memref classAddr, jbyte methodToken);
//jbyte calcInstanceSize(memref classAddr, jbyte token, memref classCompAddress);
extern	u16 getVirtualMethodReference(PN8 classAddr, u8 methodToken, PN8 classCompAddress)		REENTRANT;
extern	u16 fixupRemoteInfo(u16 offset, u8 classFlag, PF8 classCompAddr)						SMALL_MODE;
extern	u16 resolve(PF8 address, u16 offset)													SMALL_MODE;

/*method component	*/
/*jbyte getNArgs(memref methodAddr);	 */

/*Export Component	   */
extern	boolean ExportComponentpostProcess(void);
extern	boolean resolveAndAddReference(PX8 addr, PN8 componentAddresses)						SMALL_MODE;
extern	u16 getReference(PN8 exportAddr, u16 classToken, u16 token, u8 mode);

#endif
