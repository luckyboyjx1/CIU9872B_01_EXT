/*
 * Mifare.h
 *
 *  Created on: 2014Äê4ÔÂ4ÈÕ
 *      Author: linguoming1
 */

#ifndef MIFARE_H_
#define MIFARE_H_

#include "DataOperation.h"
#include "SysConfigProfile.h"

#define 		CLA_MIFARE_AUTH1      0x60
#define 		CLA_MIFARE_AUTH2      0x61
#define 		CLA_MIFARE_READ       0x30
#define 		CLA_MIFARE_WRITE      0xA0
#define 		CLA_MIFARE_INC        0xC1
#define 		CLA_MIFARE_DEC        0xC0
#define 		CLA_MIFARE_RESTORE    0xC2
#define 		CLA_MIFARE_TRANSFER   0xB0


#define 		MIFARE_READ_WRITE_SUCCESS   0x00
#define 		MIFARE_READ_WRITE_FAIL   	0x01

#if BOOT__IO__MIFARE
	#define IsMifareCmd(comCla)		(	(CLA_MIFARE_AUTH1 		== comCla) || 	\
										(CLA_MIFARE_AUTH2 		== comCla) || 	\
										(CLA_MIFARE_READ 		== comCla) ||	\
										(CLA_MIFARE_WRITE 		== comCla) ||	\
										(CLA_MIFARE_INC 		== comCla) ||	\
										(CLA_MIFARE_DEC 		== comCla) ||	\
										(CLA_MIFARE_RESTORE 	== comCla) ||	\
										(CLA_MIFARE_TRANSFER 	== comCla) )
	extern  void mifare_config(u8 isDeselect)	SMALL_MODE;
	extern  u8 	 mifare_Active(void);
	extern  u8   isMifareActivated(void);
	extern  u8   mifare_write(u8 other_block, u8 *dataBuffer, u8 data_length, u8 *key)	SMALL_MODE;
	extern  u8   mifare_Read(u8 other_block, u8 *dataBuffer, u8 data_length, u8 * key)	SMALL_MODE;
	extern  u16  mifare_Function(PX8 apduBuffer);
	extern  u8   mifare_DataConfigCheck(u8 tagH, u8 tagL, u8 thisecurityState, u8 lifeCycle, u8 isStoreData);
	extern  u8 	 mifare_DataConfigGenNewValue(u8 tagH, u8 tagL, u8 oldValue, u8 newValue, u8 isStoreData);
	extern  u16  mifare_Derive4UidFrom7Uid(void);
	extern  void cal_M1_acc_pwd(u8 *pKeyA, u8 *pKeyB, u8 *pAccPWD);
	extern  void changeMifareUID(void);
#else
	#define mifare_config(isDeselect)
	#define mifare_Active()																	0
	#define isMifareActivated()                                                             0
	#define mifare_write(MF_key, Sector, InBuffer, length, isGenerateKey)					0
	#define mifare_Read(MF_key, Sector, OutBuffer, length, isGenerateKey)					0
	#define mifare_Function(apduBuffer)														0
	#define mifare_DataConfigCheck(tagH, tagL, thisecurityState, lifeCycle, isStoreData)	0
	#define mifare_DataConfigGenNewValue(tagH, tagL, oldValue, newValue, isStoreData)		0
	#define mifare_Derive4UidFrom7Uid()														0
	#define Mifare_ReleaseCard()                                                            
	#define openMifare(apdu)                                                                0
	#define closeMifare(apdu)                                                               0
	#define cal_M1_acc_pwd(pKeyA, pKeyB, pAccPWD)                                           0
	#define changeMifareUID()                                                               0
#endif

#endif /* MIFARE_H_ */
