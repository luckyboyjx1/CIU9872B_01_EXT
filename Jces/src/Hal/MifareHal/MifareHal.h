#ifndef __MIFARE_HAL_H__
#define	__MIFARE_HAL_H__

#include "DataOperation.h"
#include "SysConfigProfile.h"

#if HAL__IO__MIFARE

#define ATS_TL_OFFSET					0x00
#define ATS_T0_OFFSET					0x01
#define	MIFARE_ATQA0_OFFSET				0x16
#define	MIFARE_ATQA1_OFFSET				0x17
#define MIFARE_DESEL_PARAM_OFFSET		0x18
#define	MIFARE_UID_CONTROL_OFFSET		0x19
#define MIFARE_SAK_INCOMPLETE_OFFSET	0x1A
#define	MIFARE_SAK_COMPLETE_OFFSET		0x1B
#define MIFARE_L4_CONFIG_OFFSET			0x1C
#define MIFARE_L3_CONFIG_OFFSET			0x1D

#define MIFARE_CONFIG_SIZE				0x1E

/******************************************************************************************************************************************************
 ******************************************************************************************************************************************************
 * functions defined for Mifare
 ******************************************************************************************************************************************************
 ******************************************************************************************************************************************************/
 /*===========================================================================*/
/**
 * \ingroup ecpMifare_EnablePer
 *
 * @brief  The function ecpMifare_EnablePer(void) enable the mifare function persistently
 *
 * @param  u8 activeMDF
 *         active or deactive mifare desfire plus
 *
 * @param  u8 activeMFP
 *         active or deactive mifare plus
 *
 * @return ECP_MIFARE_READ_WRITE_OK            						 \n
 *		   ECP_MIFARE_READ_WRITE_ERROR
 * @return void                                   							\n
 */
/*===========================================================================*/
u8 ecpMifare_EnablePer(u8 activeMDF, u8 activeMFP);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_Enable
 *
 * @brief  The function ecpMifare_Enable(void) enable the mifare function
 *
 * @return void                                   							\n
 */
/*===========================================================================*/
extern  void ecpMifare_Enable(void);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_Active
 *
 * @brief  The function ecpMifare_Active(void) actives mifare, the config value
 *		   is stored in mifareConfigBuf
 *
 * @return ECP_MIFARE_L3_RATS_RECEIVED,                                      \n
 *         ECP_MIFARE_L3_HALT_RECEIVED,                                      \n
 *         ECP_MIFARE_L4_DESLECT_RECEIVED,                                   \n
 */
/*===========================================================================*/
extern  u8 ecpMifare_Active(PX8 mifarePara);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_GetRats0
 *
 * @brief  The function ecpMifare_GetRats0(void) get terminal's first rats
 *
 * @return atqa 0                                      						 \n
 *
 * @note   no members of the uTxBuffer should be modified
 */
/*===========================================================================*/
extern  u8 ecpMifare_GetRats0(void);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_GetRats1
 *
 * @brief  The function ecpMifare_GetRats0(void) get terminal's second rats
 *
 * @return atqa 1                                      						 \n
 *
 * @note   no members of the uTxBuffer should be modified
 */
/*===========================================================================*/
extern  u8 ecpMifare_GetRats1(void);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_GetAtqa
 *
 * @brief  The function ecpMifare_GetAtqa(void) get mifare's atqa
 *
 * @return atqa                                      						 \n
 */
/*===========================================================================*/
extern  u16 ecpMifare_GetAtqa(void);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_GetSakIncomplete
 *
 * @brief  The function ecpMifare_GetSakIncomplete(void) get mifare's incomplete sak
 *
 * @return incomplete sak                              						 \n
 */
/*===========================================================================*/
extern  u8 ecpMifare_GetSakIncomplete(void);

 /*===========================================================================*/
/**
 * \ingroup ecpMifare_GetSakcomplete
 *
 * @brief  The function ecpMifare_GetSakcomplete(void) get mifare's complete sak
 *
 * @return complete sak                               						 \n
 */
/*===========================================================================*/
extern  u8 ecpMifare_GetSakcomplete(void);

/*===========================================================================*/
/**
 * \ingroup ecpMifare_GenPw
 *
 * @brief  The function ecpMifare_GenPw(u8 blockNum, XDATA_U8_P EncKey)
 *		   get block's data, EncKey's size must be not smaller than 24
 *
 * @param  u8 blockNum
 *         the input MFkey
 *
 * @param  XDATA_U8_P EncKey
 *         the input encrypted key, and the read data is stored in this buffer
 *
 * @return ECP_MIFARE_READ_WRITE_OK            						 \n
 *		   ECP_MIFARE_READ_WRITE_ERROR
 */
/*===========================================================================*/
extern u8 ecpMifare_Read(u8 blockNum, PM8 EncKey);

/*===========================================================================*/
/**
 * \ingroup ecpMifare_GenPw
 *
 * @brief  The function ecpMifare_GenPw(u8 blockNum, XDATA_U8_P EncKey)
 *		   get block's data, EncKey's size must be not smaller than 24
 *
 * @param  u8 blockNum
 *         the input MFkey
 *
 * @param  XDATA_U8_P EncKey
 *         the overwrite data are stored in this buffer
 *
 * @return ECP_MIFARE_READ_WRITE_OK            						 \n
 *		   ECP_MIFARE_READ_WRITE_ERROR                               \n
 */
/*===========================================================================*/
extern  u8 ecpMifare_Write(u8 blockNum, PM8 EncKey);

#else

#define ecpMifare_EnablePer(activeMDF, activeMFP)						0
#define ecpMifare_Enable()
#define ecpMifare_Active(mifarePara)									0
#define ecpMifare_GetRats0()											0
#define ecpMifare_GetRats1()											0
#define ecpMifare_GetAtqa()												0
#define ecpMifare_GetSakIncomplete()									0
#define ecpMifare_GetSakcomplete()										0
#define ecpMifare_Read(blockNum, EncKey)								0
#define ecpMifare_Write(blockNum, EncKey)								0

#endif

#endif
