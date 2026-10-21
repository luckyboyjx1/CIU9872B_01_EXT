/*
 * memHalServe.h
 *
 *  Created on: 2014-2-20
 *      Author: yangfajun
 */

#ifndef MEMHALSERVE_H_
#define MEMHALSERVE_H_

#include "DataOperation.h"
#include "SysConfigProfile.h"

#define		CHECK_WRITE_OK			0x5A
#define		CHECK_WRITE_NOK			0xA5
#if HAL__MEMORY__OPERATION
	/******************************************************************************************************************************************************
	 ******************************************************************************************************************************************************
	 * functions defined for nvm program
	 ******************************************************************************************************************************************************
	 ******************************************************************************************************************************************************/
	/*===========================================================================*/
	/**
	 * @brief  The function ecpEEP_Write(NVM_U8_P eew_Des, MEM_U8_P eew_Src, u16 eew_length)
	 * 		   copy the data from eew_Src to the eew_Des.
	 *
	 * @param  NVM_U8_P eew_Des
	 *         pointer to destination
	 *
	 * @param  MEM_U8_P eew_Src
	 *         pointer to source
	 *
	 * @param  u16 eew_length
	 *         copy length
	 *
	 * @return none
	 *
	 * @note   none
	 */
	/*===========================================================================*/
	extern	void ecpNvm_WriteCache_NoCheck(PN8 eew_Des, PM8 eew_Src, u16 eew_length);
	extern	void ecpNvm_WriteNoCache(PN8 eew_Des, PM8 eew_Src, u16 eew_length);
	extern  void ecpNvm_WriteCache(PN8 eew_Des, PM8 eew_Src, u16 eew_length);
	extern  void ecpNvm_WriteErase(PN8 eew_Des, PM8 eew_Src, u16 eew_length);
	extern	void ecpNvm_ErasePage(PN8 eew_Des);
	extern	void ecpNvm_EraseBlock(PN8 eew_Des);
	extern	void ecpNvm_WriteOnly(PN8 eew_Des, PM8 eew_Src, u16 eew_length);
	
	/*===========================================================================*/
	/**
	 * @brief  The function ecpEEP_Write_Commit() commit the data to eeprom.
	 *
	 * @return none
	 *
	 * @note   none
	 */
	/*===========================================================================*/
	extern  void ecpNvm_WriteCommit(void);

	extern  u8   ecpNvm_CheckWrite(PN8 eew_Des, PM8 eew_Src, u16 checkLength);

	extern  u8   ecpNvm_CheckFill(PN8 eew_Des, u8 value, u16 checkLength);

	extern  u8	 ecpNvm_IsInProgress(void);

	extern  void ecpNvm_WaitForProgress(void);
	#define	ecpNvm_Erase(dest, checkLength)									CHECK_WRITE_OK
#else
	#define  ecpNvm_WriteCache_NoCheck(eew_Des, eew_Src, eew_length)
	#define  ecpNvm_WriteCache(eew_Des, eew_Src, eew_length)
	#define  ecpNvm_Write(eew_Des, eew_Src, eew_length)
	#define  ecpNvm_WriteNoCache(eew_Des, eew_Src, eew_length)
	#define  ecpNvm_WriteCommit()
	#define  ecpNvm_CheckWrite(eew_Des, eew_Src, checkLength)					0
	#define  ecpNvm_CheckFill(eew_Des, value, checkLength)						0
	#define  ecpNvm_IsInProgress()												0
	#define  ecpNvm_WaitForProgress()											0
	#define  ecpNvm_Erase(dest, checkLength)									CHECK_WRITE_OK
#endif

#endif /* MEMHALSERVE_H_ */
