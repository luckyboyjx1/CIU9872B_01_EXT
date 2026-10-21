/*
 * SecureOperation.h
 *
 *  Created on: 2014-9-26
 *      Author: ZhangBo
 */
#ifndef _SECURE_OPERATION_H_
#define _SECURE_OPERATION_H_

#include "DataOperation.h"
#include "bootConfigProfile.h"
#include "SecOperationHal.h"
#include "bootConfigProfile.h"

typedef struct
{
	PX8				secCpyDes;
	PM8				secCpySrc;
	u32	          	secCpyLength;
	u8	          	checksum;
} SC_SecCpyMem2RamStruct;
//copy plaintext key from NVM/RAM to RAM buffer(must against DFA/DPA)

typedef struct
{
	PM8				secCpyDes;
	PM8				secCpySrc;
	u32	          	secCpyLength;
	u8	          	checksum;
} SC_SecCpyStruct;

typedef struct
{
	PM8				secCpyDes;
	PM8				secCpySrc;
	u32	          	secCmpLength;
	u8	          	checksum;
} SC_SecCmpStruct;

typedef struct
{
	PX8				secCpyDes;
	PX8				secCpySrc;
	u32	          	secCpyLength;
	u8	          	checksum;
} SC_SecCpyRam2RamStruct;
//copy plaintext key from NVM/RAM to RAM buffer(must against DFA)

typedef struct
{
	PX8				secCmpDes;
	PM8				secCmpSrc;
	u32	          	secCmpLength;
	u8	          	checksum;
} SC_SecCmpRam2RamStruct;


	#define ECP_COPY_TYPE_RAM_2_RAM							0x1A
	#define ECP_COPY_TYPE_MEM_2_RAM							0xA1
	#define ECP_COPY_TYPE_ROM_2_RAM							0x3A
	#define	ECP_COPY_TYPE_ATOMIC							0x5A
	#define	ECP_COPY_TYPE_NOATOMIC							0xA5

#if SEC_MEM_OPERATION

	//°²È«·À»¤

	/*==================================================================================================================================*
		 * Defines for copy, compare and fill
	*==================================================================================================================================*/
	/** @brief signature value of secure copy states. */
	#define ECP_COPY_START									0x0101
	#define ECP_COPY_FINISH									0x0202
	#define ECP_COPY_CHECK									0x0404
	#define ECP_SECURE_COPY_FINISH							(ECP_COPY_START + ECP_COPY_FINISH + ECP_COPY_CHECK)


		/** @brief signature value of secure compare states. */
	#define ECP_FUNC_SIGNATURE_INIT							0x0101
	#define ECP_COMPARE_FINISH								0x0202
	#define ECP_COMPARE_CHECK								0x0404
	#define ECP_RETURN_INVERT_COMPARE_RESULT				0x0808
	#define ECP_RETURN_COMPARE_RESULT						0x1010
	#define ECP_SECURE_COMPARE_PERFORM						(ECP_FUNC_SIGNATURE_INIT + ECP_COMPARE_FINISH + ECP_COMPARE_CHECK + ECP_RETURN_INVERT_COMPARE_RESULT + ECP_RETURN_COMPARE_RESULT)

				/** @brief signature value of secure state check. */

	#define ECP_SECURE_CHECK_INIT							0x0202
	#define ECP_SECURE_CHECK_SIGNATURE						0x1010
	#define ECP_SECURE_CHECK_PERFORM						(ECP_SECURE_CHECK_INIT + ECP_SECURE_CHECK_SIGNATURE) 
	
	#define XOR_INIT_EQU									0xFFFFFFFF
	#define MINUS_INIT_EQU									0x00000000

	#define SECURE_COPY_FLAG								0xA5
	#define COMMON_COPY_FLAG								0x5A

	/** @brief parameter of ecpMem_SecureCopy. */
	typedef struct ecpSecCpy_secParam
	{
		PM8		 		ecpSecCpyDes;
		PM8				ecpSecCpySrc;
		u16	          	ecpSecCpyLength;
		u16	    		ecpSecCpyParamChecksum;
		PM16			ecpSecCpyFuncSignature;
	} ecpSecCpy_secParam_t;

	/** @brief parameter of ecpMem_SecureCompare. */
	typedef struct ecpSecCmp_secParam
	{
		PM8				ecpSecCmpDes;
		PM8				ecpSecCmpSrc;
		u16	          	ecpSecCmpLength;
		u16	          	ecpSecCmpParamChecksum;
		u16				ecpSecCmpFuncSignature;
		PX16			ecpSecCmpInvertResult;
	} ecpSecCmp_secParam_t;

	extern u8		isSecureCopy;
	extern u8		isSecureCopyInv;

	/*===========================================================================*/
	/**
	 * @brief  The secure function ecp_getSecCmpSig(u16 initSigData)
	 *		   return the signature after excecuting the function.
	 *
	 * @param  MEM_U8_P initSigData
	 *         initial signature data
	 *
	 * @return u16
	 *         return function signature
	 */
	/*===========================================================================*/
	#define	ecp_getSecCmpSig(initSigData)	 (initSigData + ECP_SECURE_COMPARE_PERFORM)

	extern void	SC_SecOpeInit(void);
	extern u16	ecp_calSecCmpParamChecksum(ecpSecCmp_secParam_t * cmpSecParam);
	extern void	ecpMem_SecureCompareParamInit(ecpSecCmp_secParam_t * cmpSecParam, PM8 Des, PM8 Src, PX16 secCmpInvRes,u16 length);
	extern u16	ecpMem_SecureCompare(ecpSecCmp_secParam_t * cmpSecParam);
	extern u16	ecp_calSecCpyParamChecksum(ecpSecCpy_secParam_t * cpySecParam);
	extern void ecpMem_SecCpyParamParamInit(ecpSecCpy_secParam_t * cpySecParam, PM8 Des, PM8 Src, u16 length);
	extern u16  SC_MemCopyRam2Ram(SC_SecCpyStruct secCpyPara);
	extern u16  SC_MemCopyRom2Ram(SC_SecCpyStruct secCpyPara);
	extern u16	SC_MemCopy(SC_SecCpyStruct secCpyPara, u8 type);
	extern u8	SC_memCompare(PM8 Des, PM8 Src, u16 length);
	extern u8	SC_StateCheck(u8 state, u8 inverseState, u16 initCtrFlow);
	extern u8	SC_memCompareEqualWithDFA(PM8 Des, PM8 Src, u16 length);
	extern u16	SC_MemCopyWithDFA(SC_SecCpyStruct secCpyPara, u8 type);

	#define SC_MemCopyMem2Ram(des, src, length)								SC_checkReturnCode(SC_MemCopy(secCpyPara, ECP_COPY_TYPE_MEM_2_RAM))
	#define SC_MemCopyAtomic(des, src, length)								SC_checkReturnCode(SC_MemCopyAPI_JAVA(des, src, length, ECP_COPY_TYPE_ATOMIC))
	#define SC_MemCopyNoAtomic(des, src, length)							SC_checkReturnCode(SC_MemCopyAPI_JAVA(des, src, length, ECP_COPY_TYPE_NOATOMIC))

	#define SC_CmpRamVsRom(addr1, addr2, length)							SC_memCompareEqualWithDFA(addr1, addr2, length)
	#define SC_CmpRamVsRam(addr1, addr2, length)							SC_memCompareEqualWithDFA(addr1, addr2, length)
	
	#define SC_CheckSceCopyFlag(fun)										(isSecureCopy == U8_INV(isSecureCopyInv))
	#define SC_IsCopyNotInSec(fun)											(SECURE_COPY_FLAG != isSecureCopy)

#else
	#define SC_SecOpeInit()
	#define SC_StateCheck(state, inverseState, initCtrFlow)					(state)
	#define SC_memCompare(Des, Src, length)									memCompare(Des, Src, length)
	#define	SC_MemCopyMem2Ram(des, src, length)								ecpMem_Copy(des, src, length)
	#define	SC_MemCopyRom2Ram(des, src, length)								ecpMem_Copy(des, src, length)
	#define SC_MemCopyAtomic(des, src, length)								memCopyAtomic(des, src, length)
	#define SC_MemCopyNoAtomic(des, src, length)							memCopyNoAtomic(des, src, length)
	#define SC_CmpRamVsRom(addr1, addr2, length)							memCompareEqual(addr1, addr2, length)
	#define SC_CmpRamVsRam(addr1, addr2, length)							memCompareEqual(addr1, addr2, length)
	#define SC_memCompareEqualWithDFA(Des,  Src, length)                    memCompareEqual(Des,  Src, length)
	#define SC_IsCopyNotInSec()												true
#endif  //SEC_MEM_OPERATION

/** @brief result of comparison. */
#define ECP_SEC_CHECK_STATE_1											0x5A
#define ECP_SEC_CHECK_STATE_2											0xA5
#define ECP_SEC_CHECK_STATE_3											0x5B
#define ECP_SEC_CHECK_STATE_4											0x5E


#define ECP_CMP_RESULT_EQUAL											ECP_SEC_CHECK_STATE_1
#define ECP_CMP_RESULT_NOT_EQUAL										ECP_SEC_CHECK_STATE_2
#define ECP_CMP_RESULT_GREATER											ECP_SEC_CHECK_STATE_3
#define ECP_CMP_RESULT_SMALLER											ECP_SEC_CHECK_STATE_4


#if BOOT__SECURITY__KEY_DATA_SAFE
	#define SEC_DATA_LEN_SIZE									0
	#define SEC_DATA_CHECK_VALUE_SIZE							1
	#define SEC_DATA_HEADER_SIZE								U8_A_U8(SEC_DATA_LEN_SIZE, SEC_DATA_CHECK_VALUE_SIZE)

	extern u8 		SO_getSecDataByRef(PX8 outBuffer, u8 outBufferOffset, u8 dataLength, u16 ref);
	extern u8  		SO_getSecDataByAddr(PX8 outBuffer, u8 outBufferOffset, u8 datalength, PM8 secDataArea);
	extern u16 		SO_storeSecData(PX8 inBuffer, u8 inBufferOffset, u8 datalength, PM8 secDataArea);
	extern u16 		SO_calSecDataAreaLen(u16 dataLen);

#else
	#define SO_storeSecData(inBuffer, inBufferOffset, datalength, secDataArea)
	#define SO_getSecData(outBuffer, outBufferOffset, datalength, secDataArea)					0
	#define SO_calSecDataAreaLen(dataLen)														dataLen
	#define SO_getSecDataByAddr(outBuffer, outBufferOffset, datalength, secDataArea)				0
	#define SO_getSecDataByRef(outBuffer, outBufferOffset, dataLength, ref)						0
#endif

#endif	// _SECURE_OPERATION_H_
