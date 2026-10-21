/*
 * chipHead.h
 *
 *  Created on: 2014Äê9ÔÂ29ÈÕ
 *      Author: jiangxiaohui
 *  the head is user for add CMOS chip's own lib head, register head, or other
 */

#ifndef CHIPHEAD_H_
#define CHIPHEAD_H_

#include "ChipFunction.h"

#include "optspace.h"
#include "ChipCoreFunction.h"

#include "CIU9872B_01_V4_SFR_C.h"
#include "CIU9872B_01_V4_Hal.h"
#include "CIU9872B_01_V4_Hal_Sysclk.h"

#include "HED_API_NVM.h"

#define 	BOOTLOADER_CONFIG_BASE		0x000F8000
#define 	BLMFKEY_BASE_ADDR_OFFSET  	0x0000
#define 	BLENKEY_BASE_ADDR_OFFSET 	0x020C
#define   	BLMODE_BASE_ADDR_OFFSET 	0x021C
#define   	BLSPIMODE_BASE_ADDR_OFFSET  0x0211
#define     BLVERSION_BASE_ADDR_OFFSET  0x0212

extern void rtnBackup(u8 resetStatus);
extern void GetChipUID(PX8 uid,u8 uidlevel);

#define SYM_SM4_ENC				0
#define SYM_SM4_DEC				1
#define SYM_SM1_ENC				0
#define SYM_SM1_DEC				1

#define ALG_OK 					0xAAAA
#define ALG_ERROR 				0x5555

#define ALG_SYM_OK 				0xAA
#define ALG_SYM_ERROR 			0x55

#define ALG_RNG_OK				0xBC2679B3
#define ALG_RNG_NOK				0x927C45A5

#define SYM_MK_SIZE				8
#define SYM_TK_SIZE				8

#define     OTP_SWITCH        0x14000200     //      OTP switch address	


/*****************************************************************************************************************/
/********************************************About Register Value define******************************************/
/*****************************************************************************************************************/

//SYSRSTFLAG register value
#define SYSRSTFLAG_7816RstFlag       0x04
#define SYSRSTFLAG_PORRstFlag        0x0200

//SCISSR register value
#define SCISSR_OE                    0x01
#define SCISSR_PE                    0x02
#define SCISSR_RI                    0x04
#define SCISSR_RB                    0x08
#define SCISSR_TI                    0x10
#define SCISSR_TB                    0x20
#define SCISSR_SETU                  0x40

//SCISCON register value
#define SCISCON_SE                   0x40

//SCITXFCR register value
#define SCITXFCR_flush_tx_fifo       0x01
#define SCITXFCR_tx_full             0x02

//SYSINFST register value
#define SYSINFST_SCI_DET             0x01

//SCIRXFCR register value
#define SCIRXFCR_flush_rx_fifo       0x01
#define SCIRXFCR_rx_empty            0x02

//SCIS60CON register value
#define SCIS60CON_SWB                0x01
#define SCIS60CON_WTC                0x02

/*DES DEFINE*/
#define BIG_DES_BLOCKNUM        10
#define DES_SUCCESS 			0xB7C8D9EA
#define DES_LEN_ERR				0x932057CE
#define DES_CHK_ERR				0x48372615
#define ECB_ENCRYPT             0x00
#define ECB_DECRYPT             0x40
#define CBC_ENCRYPT             0x20
#define CBC_DECRYPT             0x60

/*AES DEFINE*/
#define BIG_AES_BLOCKNUM        10
#define AES_SUCCESS 			0xB7C8D9EA
#define AES_LEN_ERR				0x932057CE
#define AES_CHK_ERR				0x48372615
#define ECB_ENCRYPT             0x00
#define ECB_DECRYPT             0x40
#define CBC_ENCRYPT             0x20
#define CBC_DECRYPT             0x60
#define AES_KEYTYPE_128         0x00
#define AES_KEYTYPE_192         0x01
#define AES_KEYTYPE_256         0x02


/*SM4 DEFINE*/
#define BIG_SM4_BLOCKNUM        10
#define SM4_SUCCESS 		     	  0x00
#define SM4_DATA_ERR			      0x01
#define ECB_ENCRYPT             0x00
#define ECB_DECRYPT             0x40
#define CBC_ENCRYPT             0x20
#define CBC_DECRYPT             0x60
#define CFB_ENCRYPT             0x10
#define CFB_DECRYPT             0x50
#define OFB_ENCRYPT             0x30
#define OFB_DECRYPT             0x70
#define CBC_MAC             	  0x80

/*SM1 DEFINE*/
#define BIG_SM1_BLOCKNUM        10
#define SM1_SUCCESS 		     	  0x00
#define SM1_DATA_ERR			      0x01

	
#endif /* CHIPHEAD_H_ */
