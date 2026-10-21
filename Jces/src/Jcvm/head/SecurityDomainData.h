/*
 * SecurityDomainData.h

 *
 *  Created on: 2013-9-30
 *      Author: linguoming1
 */

#include "SysConfig.h"
#include "object.h"

#ifndef SECURITYDOMAINDATA_H_
#define SECURITYDOMAINDATA_H_

#define	GET_FIRST_TAG_NODE									0
#define GET_FIRST_KEY_GROUP									1




#define KEY_INFO_DATA_OFFSET           2
//***************************************************  sdData instance filed************************************************************//
//**************************************************************************************************************************************//
#define KEY_INFO_LENGTH                         6
#define KEY_INFO_LENGTH_FORMAT2                 12
#define KEY_INFO_LENGTH_OFFSET                  1
#define KEY_INFO_KEYID_OFFSET            	    2
#define KEY_INFO_KEY_VERSION_OFFSET             3
#define KEY_INFO_KEY_TYPE_OFFSET                4
#define KEY_INFO_KEY_TYPE_VALUE_OFFSET_FORMAT2  5
#define KEY_INFO_KEY_LENGTH_OFFSET              5
#define KEY_INFO_KEY_LENGTH_OFFSET_FORMAT2      7

#define KEY_INFO_KEY_USAGE_LENGTH_OFFSET       	8 
#define KEY_INFO_KEY_USAGE_VALUE_OFFSET         9 //U8_A_U8(KEY_INFO_KEY_USAGE_LENGTH_OFFSET, KEY_USAGE_QUALIFIER_LENGTH)
#define KEY_INFO_KEY_ACCESS_LENGTH_OFFSET       U8_A_U8(KEY_INFO_KEY_USAGE_VALUE_OFFSET, 2)
#define KEY_INFO_KEY_ACCESS_VALUE_OFFSET        U8_A_U8(KEY_INFO_KEY_ACCESS_LENGTH_OFFSET, KEY_ACCESS_LENGTH)


#define KEY_INFO_KEY_LENGTH                     8
//***************************************************  sdData instance filed************************************************************//
//**************************************************************************************************************************************//
#define SECURITY_DOMAIN_DATA_PCK_ID							    EASTCOMPEACE_OPENPLATFORM_ID
#define newSecurityDomainDataObject()						    newObject(SECURITY_DOMAIN_DATA_OFFSET,SECURITY_DOMAIN_DATA_PCK_ID, BIT_NEED_REFRESH)

#define sdData_getScpBufferRef(sdDataObj)						nvmReadU16(get_object_field_ref(sdDataObj, scpBuffer_INDEX))
#define sdData_getScpBufferField(sdDataObj)						get_array_data(MEMtPF8(locateObjectInMemory(sdData_getScpBufferRef(sdDataObj))))

#define sdData_getAmend_B_84_bufferRef(sdDataObj)               nvmReadU16(get_object_field_ref(sdDataObj, Amend_B_84_buffer_INDEX))
#define sdData_getAmend_B_84_bufferField(sdDataObj)             get_array_data(MEMtPF8(locateObjectInMemory(sdData_getAmend_B_84_bufferRef(sdDataObj))))
#define sdData_setAmend_B_84_bufferRef(sdObj, ref)				nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_84_buffer_INDEX), ref)

#define sdData_getAmend_B_85_bufferRef(sdDataObj)               nvmReadU16(get_object_field_ref(sdDataObj, Amend_B_85_buffer_INDEX))
#define sdData_getAmend_B_85_bufferField(sdDataObj)             get_array_data(MEMtPF8(locateObjectInMemory(sdData_getAmend_B_85_bufferRef(sdDataObj))))
#define sdData_setAmend_B_85_bufferRef(sdObj, ref)				nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_85_buffer_INDEX), ref)

#define sdData_getAmend_B_86_bufferRef(sdDataObj)               nvmReadU16(get_object_field_ref(sdDataObj, Amend_B_86_buffer_INDEX))
#define sdData_getAmend_B_86_bufferField(sdDataObj)             get_array_data(MEMtPF8(locateObjectInMemory(sdData_getAmend_B_86_bufferRef(sdDataObj))))
#define sdData_setAmend_B_86_bufferRef(sdObj, ref)				nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_86_buffer_INDEX), ref)

#define sdData_getAmend_B_89_bufferRef(sdDataObj)               nvmReadU16(get_object_field_ref(sdDataObj, Amend_B_89_buffer_INDEX))
#define sdData_getAmend_B_89_bufferField(sdDataObj)             get_array_data(MEMtPF8(locateObjectInMemory(sdData_getAmend_B_89_bufferRef(sdDataObj))))
#define sdData_setAmend_B_89_bufferRef(sdObj, ref)				nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_89_buffer_INDEX), ref)

#define sdData_getSD_Tar_bufferRef(sdDataObj)               	nvmReadU16(get_object_field_ref(sdDataObj, SD_Tar_buffer_INDEX))
#define sdData_getSD_Tar_bufferField(sdDataObj)             	get_array_data(MEMtPF8(locateObjectInMemory(sdData_getSD_Tar_bufferRef(sdDataObj))))

//***********************************************Security Domain DNS Resolution Parameters***********************************************//
#define sdData_getAmend_B_A5_bufferRef(sdDataObj)               nvmReadU16(get_object_field_ref(sdDataObj, Amend_B_A5_buffer_INDEX))
#define sdData_getAmend_B_A5_bufferField(sdDataObj)             get_array_data(MEMtPF8(locateObjectInMemory(sdData_getAmend_B_A5_bufferRef(sdDataObj))))
#define sdData_setAmend_B_A5_bufferRef(sdObj, ref)				nvmWriteU16Atomic(get_object_field_ref(sdDataObj, Amend_B_A5_buffer_INDEX), ref)

#define sdData_getDNS_D0_D3_bufferRef(sdDataObj)                nvmReadU16(get_object_field_ref(sdDataObj, DNS_D0_D3_buffer_INDEX))
#define sdData_getDNS_D0_D3_bufferField(sdDataObj)              get_array_data(MEMtPF8(locateObjectInMemory(sdData_getDNS_D0_D3_bufferRef(sdDataObj))))
#define sdData_setDNS_D0_D3_bufferRef(sdObj, ref)				nvmWriteU16Atomic(get_object_field_ref(sdDataObj, DNS_D0_D3_buffer_INDEX), ref)

#define sdData_getDNS_D6_bufferRef(sdDataObj)                   nvmReadU16(get_object_field_ref(sdDataObj, DNS_D6_buffer_INDEX))
#define sdData_getDNS_D6_bufferField(sdDataObj)                 get_array_data(MEMtPF8(locateObjectInMemory(sdData_getDNS_D6_bufferRef(sdDataObj))))
#define sdData_setDNS_D6_bufferRef(sdObj, ref)				    nvmWriteU16Atomic(get_object_field_ref(sdDataObj, DNS_D6_buffer_INDEX), ref)

#define sdData_getDNS_F7_bufferRef(sdDataObj)                   nvmReadU16(get_object_field_ref(sdDataObj, DNS_F7_buffer_INDEX))
#define sdData_getDNS_F7_bufferField(sdDataObj)                 get_array_data(MEMtPF8(locateObjectInMemory(sdData_getDNS_F7_bufferRef(sdDataObj))))
#define sdData_setDNS_F7_bufferRef(sdObj, ref)				    nvmWriteU16Atomic(get_object_field_ref(sdDataObj, DNS_F7_buffer_INDEX), ref)

#define sdData_getDNS_F9_bufferRef(sdDataObj)                   nvmReadU16(get_object_field_ref(sdDataObj, DNS_F9_buffer_INDEX))
#define sdData_getDNS_F9_bufferField(sdDataObj)                 get_array_data(MEMtPF8(locateObjectInMemory(sdData_getDNS_F9_bufferRef(sdDataObj))))
#define sdData_setDNS_F9_bufferRef(sdObj, ref)				    nvmWriteU16Atomic(get_object_field_ref(sdDataObj, DNS_F9_buffer_INDEX), ref)

#define sdData_getDNS_FA_bufferRef(sdDataObj)                   nvmReadU16(get_object_field_ref(sdDataObj, DNS_FA_buffer_INDEX))
#define sdData_getDNS_FA_bufferField(sdDataObj)                 get_array_data(MEMtPF8(locateObjectInMemory(sdData_getDNS_FA_bufferRef(sdDataObj))))
#define sdData_setDNS_FA_bufferRef(sdObj, ref)				    nvmWriteU16Atomic(get_object_field_ref(sdDataObj, DNS_FA_buffer_INDEX), ref)

#define sdData_getDNS_D8_bufferRef(sdDataObj)                   nvmReadU16(get_object_field_ref(sdDataObj, DNS_D8_buffer_INDEX))
#define sdData_getDNS_D8_bufferField(sdDataObj)                 get_array_data(MEMtPF8(locateObjectInMemory(sdData_getDNS_D8_bufferRef(sdDataObj))))
#define sdData_setDNS_D8_bufferRef(sdObj, ref)				    nvmWriteU16Atomic(get_object_field_ref(sdDataObj, DNS_D8_buffer_INDEX), ref)

//***********************************************Security Domain DNS Resolution Parameters***********************************************//
#define sdData_getScpTypeField(sdDataObj)						get_object_field_ref(sdDataObj, curScpType_INDEX)
#define sdData_getScpType(sdDataObj)                      		nvmReadU8(MEMtPN8(sdData_getScpTypeField(sdDataObj)))
#define sdData_setScpType(sdDataObj, scpType)                  	nvmWriteU8NoAtomic(sdData_getScpTypeField(sdDataObj), scpType)

#define sdData_getScpOptionField(sdDataObj)						get_object_field_ref(sdDataObj, curScpOption_INDEX)
#define sdData_getScpOption(sdDataObj)                    		nvmReadU8(sdData_getScpOptionField(sdDataObj))
#define sdData_setScpOption(sdDataObj, scpOption)              	nvmWriteU8NoAtomic(sdData_getScpOptionField(sdDataObj), scpOption)


#define sdData_getAuthFailCountField(sdDATAObj)        			get_object_field_ref(sdDATAObj, AuthFailCount_INDEX)
#define sdData_getAuthFailCount(sdDATAObj)             			nvmReadU8(sdData_getAuthFailCountField(sdDATAObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define sdData_setAuthFailCount(sdDATAObj, counts)		 		nvmWriteU8Atomic(sdData_getAuthFailCountField(sdDATAObj), counts)
#define	sdData_desAuthFailCount(sdDATAObj)						sdData_setAuthFailCount(sdDATAObj, (sdData_getAuthFailCount(sdDATAObj) - 1))

#define	sdData_getDESKeyCountField(sdDATAObj)					get_object_field_ref(sdDATAObj, DESKeyCount_INDEX)
#define	sdData_getDESKeyCount(sdDATAObj)						nvmReadU8(sdData_getDESKeyCountField(sdDATAObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define sdData_setDESKeyCount(sdDATAObj, counts)		 	 	nvmWriteU8Atomic(sdData_getDESKeyCountField(sdDATAObj), counts)
#define	sdData_desDESKeyCount(sdDATAObj)						sdData_setDESKeyCount(sdDATAObj, (sdData_getDESKeyCount(sdDATAObj) - 1))
#define	sdData_incDESKeyCount(sdDATAObj)						sdData_setDESKeyCount(sdDATAObj, (sdData_getDESKeyCount(sdDATAObj) + 1))


#define sdData_getTag82ValueField(sdDataObj)					get_object_field_ref(sdDataObj, tag82Value_INDEX)
#define sdData_getTag82Value(sdDataObj)                      	nvmReadU16(MEMtPN8(sdData_getTag82ValueField(sdDataObj)))
#define sdData_setTag82Value(sdDataObj, tag82Value)             nvmWriteU16NoAtomic(sdData_getTag82ValueField(sdDataObj), tag82Value)

#define sdData_getTag83ValueField(sdDataObj)					get_object_field_ref(sdDataObj, tag83Value_INDEX)
#define sdData_getTag83Value(sdDataObj)                      	nvmReadU8(MEMtPN8(sdData_getTag83ValueField(sdDataObj)))
#define sdData_setTag83Value(sdDataObj, tag83Value)             nvmWriteU8NoAtomic(sdData_getTag83ValueField(sdDataObj), tag83Value)

#define sdData_getTag84ValueField(sdDataObj)					get_object_field_ref(sdDataObj, tag84Value_INDEX)
#define sdData_getTag84Value(sdDataObj)                      	nvmReadU8(MEMtPN8(sdData_getTag84ValueField(sdDataObj)))
#define sdData_setTag84Value(sdDataObj, tag84Value)             nvmWriteU8NoAtomic(sdData_getTag84ValueField(sdDataObj), tag84Value)

#define sdData_getTag87ValueField(sdDataObj)					get_object_field_ref(sdDataObj, tag87Value_INDEX)
#define sdData_getTag87Value(sdDataObj)                      	nvmReadU16(MEMtPN8(sdData_getTag87ValueField(sdDataObj)))
#define sdData_setTag87Value(sdDataObj, tag87Value)             nvmWriteU16NoAtomic(sdData_getTag87ValueField(sdDataObj), tag87Value)

#define	sdData_getSdDataField(sdObj)						    get_object_field_ref(sdObj,sdData_INDEX)
#define sdData_getSdDataRef(sdObj)							    nvmReadU16(sdData_getSdDataField(sdObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define sdData_setSdDataRef(sdObj, ref)						    nvmWriteU16Atomic(sdData_getSdDataField(sdObj), ref)

#define	sdData_getFirstTagNodeField(sdDataObj)				    get_object_field_ref(sdDataObj,firstTagNode_INDEX)
#define sdData_getFirstTagNodeRef(sdDataObj)				    nvmReadU16(sdData_getFirstTagNodeField(sdDataObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define	sdData_setFirstTagNodeRef(sdDataObj, ref)			    nvmWriteU16Atomic(sdData_getFirstTagNodeField(sdDataObj), ref)

#define sdData_getFirstKeyGroupField(sdDataObj)				    get_object_field_ref(sdDataObj,firstKeyGroup_INDEX)
#define sdData_getFirstKeyGroupRef(sdDataObj)				    nvmReadU16(sdData_getFirstKeyGroupField(sdDataObj)) // modify by xuxiaojun@20220625 需要考虑cache
#define sdData_setFirstKeyGroupRef(sdDataObj, ref)			    nvmWriteU16Atomic(sdData_getFirstKeyGroupField(sdDataObj), ref)
#define getFirstKeyGroupHand                                    sdData_getFirstKeyGroupField

// move to SCP11.h
//**************************************************************************************************************************************//
//**************************************************************************************************************************************//
extern u16 writeKeyInfo(PX8 buf, u16 maxSingleLength, PN8 keySetObj, u16 offset);
extern jref sdData_getSDdataRef(jref sdRef);
extern jref sdData_getFirstTagNode(PN8 sdDataHand);
extern void sdData_init(PN8 sdDataObj);
extern void isdScpData_Init(void);
extern jref sdData_getKeyGroupRef(PN8 sdDataAddr, u8 scpType, u8 isNewKeyGroup);
extern memref sdData_getsdDataObjByAppId(u8 appId);
//   X value is 1 , 2 or 3
#define			SCP0X_WITHOUT_IOPTION_MODE			0x01
#define			SCP10_WITHOUT_IOPTION_MODE			0x02
#define			SCP80_WITHOUT_IOPTION_MODE			0x04
#define			SCP81_WITHOUT_IOPTION_MODE			0x08
#define			SCP10_WITH_IOPTION_MODE				0x82

#define			checkSCPTypeIsSCP0X(scpType)		
#define			checkSCPTypeIsSCP10(scpType)
#define			checkSCPTypeIsSCP80(scpType)

/*
for SCP01/SCP02 parameter i
---------------------------------------------
b8 |b7|b6|b5|b4|b3|b2|b1|Description
   |  |	 |  |  |  |  |1 |SCP01 OR SCP02 OR SCP 03
   |  |  |  |  |  |1 |  |SCP10
   |  |  |  |  |1 |  |  |SCP80
   |  |  |  |1 |  |  |  |
   |  |  |1 |  |  |  |  |
   |  |1 |  |  |  |  |  |
   |1 |  |  |  |  |  |  |
 1 |  |  |  |  |  |	 |  |with Ioption
---------------------------------------------
*/

#if GP__KEY_MANAGER
	extern u8 sdData_changeCurrentScpTypeAndOption(PM8 sdDataObj, u8 mode, u8 Ioption);
	extern u16 sdData_getKeyInfo(PX8 buf, u16 maxSingleLength, PN8 sdDataObj, PX16 context);
	extern u8 sdData_getScpOptionByScpType(PX8 sdDataObj, u8 scpType);
#else
	#define sdData_changeCurrentScpTypeAndOption(sdDataObj, mode, Ioption)								0
	#define sdData_getKeyInfo(buf, maxSingleLength, sdDataObj, context)									0
    #define sdData_getScpOptionByScpType(sdDataObj, scpType)                                            0
#endif

extern u32 sdData_getSDTarbufferAddr(PX8 sdDataObj);

#endif /* SECURITYDOMAINDATA_H_ */
