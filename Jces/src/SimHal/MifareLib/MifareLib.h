/*
 * MifareLib.h
 *
 *  Created on: 2015年4月3日
 *      Author: jiangxiaohui
 */

#ifndef MIFARELIB_H_
#define MIFARELIB_H_
 
#define	MIFARE_CLASSIC_ACK				0x0A
#define	MIFARE_CLASSIC_NAK_STATUS_4		0x04
#define	MIFARE_CLASSIC_NAK_STATUS_5		0x05

#define	MIFARE_NVM_PROGRAM_SUCCESS		0x5A
#define	MIFARE_NVM_PROGRAM_FAILED		0xA5


#define	MIFARE_COMMOD_CMD_OFFSET		0x00
#define	MIFARE_COMMOD_ADDR_OFFSET		0x01
#define	MIFARE_COMMOD_CRC_OFFSET		0x02

#define MIFARE_BLOCK_SIZE	   			0x10
#define MIFARE_COMMOND_SIZE	   			0x02
#define MIFARE_BLOCK_CRC_SIZE	   		0x02
#define MIFARE_VALUEBLOCK_VALUE_SIZE	0x0C
#define MIFARE_VALUE_SIZE				0x04
#define MIFARE_VALUEBLOCK_ADDR_SIZE		0x04


#define MIFARECFG_BINDEX_INDEX      	0x0F
#define MIFARECFG_BINDEX_ACTIVE     	0x80
#define MIFARECFG_BMODESIZE_SIZE    	0x05
#define MIFARECFG_BMODESIZE_SIZE_1K 	0x01
#define MIFARECFG_BMODESIZE_SIZE_4K 	0x04
#define MIFARECFG_BMODESIZE_UID_CL1 	0x00
#define MIFARECFG_BMODESIZE_UID_CL2 	0x40
#define MIFARECFG_BMODESIZE_UID_CL3 	0x80
#define MIFARECFG_BMODESIZE_RFU     	0x02
#define MIFARE_CONFIG_SIZE				0x1E
#define UIDSIZE_OFFEST    				0x0F
#define MIF_UID_CL1 					0x00
#define MIF_UID_CL2 					0x40
#define MIF_UID_CL3 					0x80

#define MIFARE_R 						0x00
#define MIFARE_W 						0x01
#define MIFARE_I                        0x02
#define MIFARE_DRT                      0x03

#define CLA_AUTH1 					    0x60
#define CLA_AUTH2 					    0x61
#define	CLA_READ 						0x30
#define	CLA_WRITE 						0xA0
#define	CLA_INC 						0xC1
#define CLA_DEC 						0xC0
#define	CLA_RESTORE 					0xC2
#define	CLA_TRANSFER 					0xB0
#define	CLA_HALT 						0x50
#define CLA_WUPA 						0x52

#define CLA_RATS                 	    0xE0

/*mifare state define*/
#define HW_MIF_HALT_RECEIVED		  	0x0101
#define HW_MIF_HALT_NOT_RECEIVED		0x0202
#define HW_MIF_IDLE               		0x5A5A

/*mifare command error state*/
#define MIFARE_OK                 0x0000
#define MIFARE_CHECK_FAIL         0x1111
#define MIFARE_AUTH_FAIL          0x2222
#define MIFARE_CMD_INVALID        0x3333
#define MIFARE_CMD_UNKNOW         0x4444
#define MIFARE_CMD_RATS           0x5555


#define MIFARE_KEY_TYPE_A				0x00
#define MIFARE_KEY_TYPE_B				0x01
#define MIFARE_KEY_TYPE_AB				0x02
#define MIFARE_KEY_NEVER				0x03


//max(3bit)=8,max(access condition)=4(MIFARE_R/MIFARE_W/MIFARE_I/MIFARE_DRT)
static u8 CONST MifareLb_data[8][4]=
{
	{MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_TYPE_AB},//0 0 0
	{MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_AB},//0 0 1
	{MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER},//0 1 0
	{MIFARE_KEY_TYPE_B, 	MIFARE_KEY_TYPE_B, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER},//0 1 1
	{MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_TYPE_B, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER},//1 0 0
	{MIFARE_KEY_TYPE_B, 	MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER},//1 0 1
	{MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_TYPE_B, 		MIFARE_KEY_TYPE_B, 		MIFARE_KEY_TYPE_AB},//1 1 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER} //1 1 1
};

//max(3bit*r/w)=16,max(access condition)=3(keya/ctrl/keyb)
static u8 CONST MifareLb_key[16][3]=
{
	//r
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_A, 		MIFARE_KEY_TYPE_A},//0 0 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_A, 		MIFARE_KEY_TYPE_A},//0 0 1
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_A, 		MIFARE_KEY_TYPE_A},//0 1 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER},//0 1 1
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER},//1 0 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER},//1 0 1
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER},//1 1 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_AB, 	MIFARE_KEY_NEVER},//1 1 1
	//w
	{MIFARE_KEY_TYPE_A, 	MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_A},//0 0 0
	{MIFARE_KEY_TYPE_A, 	MIFARE_KEY_TYPE_A, 		MIFARE_KEY_TYPE_A},//0 0 1
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER},//0 1 0
	{MIFARE_KEY_TYPE_B, 	MIFARE_KEY_TYPE_B, 		MIFARE_KEY_TYPE_B},//0 1 1
	{MIFARE_KEY_TYPE_B, 	MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_B},//1 0 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_TYPE_B, 		MIFARE_KEY_NEVER},//1 0 1
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER},//1 1 0
	{MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER, 		MIFARE_KEY_NEVER} //1 1 1
};

typedef struct
{
	u8 sector; //当前操作或完成认证的扇区序号.当s50时[0~15],当s50时[0~39]
	u8 key;		//可操作的密钥类型.0-keya,1-keyb,2-keyab,3-never
	u8 crypt; //算法类型.MIFARE_MIFARE-mifare,MIFARE_HUAHONG-huahong
	u8 isAuth;
}mifare_para_t;

typedef struct mifareLibCmdPara
{
	PM8		currentBlockAdd;		/**/
	PM8		accCondBlockAdd;		/**/
	u8		currentSector;			/**/
	u8		relatiBlock;			/**/
}mifareLib_cmd_para_t;

#define	 __MIFARE_OS__SECURE__		    0

#define MIFARE_MIFARE 					0
#define MIFARE_HUAHONG 					1

#define RET_RF_ERR_CRC 0x01
#define RET_RF_ERR_FIFO 0x02
#define RET_RF_ERR_BIT 0x08
#define RET_RF_ERR_PARITY 0x10

#define MIFARE_AUTH_KEY_A_LENGTH		0x06
#define MIFARE_ACCESS_COND_LENGTH		0x04
#define MIFARE_AUTH_KEY_B_LENGTH		0x06

#define	MIFARE_AUTH_KEY_A_OFFSET		0x00
#define	MIFARE_ACCESS_COND_OFFSET		0x06
#define	MIFARE_AUTH_KEY_B_OFFSET		0x0A
#define	MIFARE_AUTH_KEY_M1_OFFSET		0x30
#define	MIFARE_AUTH_KEY_M4_OFFSET		0xF0
#define	MIFARE_M1_MAX_SECTOR_NUMBER		0x0F

#define MIFARE_AUTH_KEY_ADDRESS			0x00
#define MIFARE_ACCESS_COND_ADDRESS		0x01
#define MIFARE_KEY_ADDRESS				0x00
#define MIFARE_KEY_ADDRESS				0x00


extern void MifareLib_changeState();
extern u8 MifareLib_getState();
extern u8 mifareLib_CmdParaInit(mifareLib_cmd_para_t *	myMifareLib_cmdPara, u8 currentBlockNumber);
extern void MifareLib_authProcess(mifare_para_t * myMifarePara, PM8 mifareKeyPoint);
extern u8 MifareLib_NvmProgram(PN8 DestBlockAdd, PX8 inData, u16 dataLen);
extern void MifareLib_SendNAK(u8 NAKCode);
extern void MifareLib_SendACK(void);
extern u8 MifareLib_sendPart1_receivePart2(PX8 mifareDataOut, PX8 mifareCommond, u16 expLen, u8 flags);
extern u8 MifareLib_sendRes_receiveCom(PX8 mifareDataOut, PX8 mifareCommond, u16 len, u8 flags);
extern PX8 MifareLib_getMifReusBuf(void);
extern mifare_para_t * MifareLib_getMifarePara(void);
extern PM8 MifareLib_getMifareAddress(void);
extern void MifareLib_setMifareAddress(u32 addr);
extern void MifareLib_initParam(mifare_para_t * myMifareParm);
extern void MifareLib_auth(PX8 mifareCommond, mifare_para_t * myMifarePara);
extern u8 MifareLib_read(PX8 mifareDataOut, PX8 mifareCommond, mifare_para_t * myMifarePara);
extern void MifareLib_write(PX8 mifareDataOut, PX8 mifareCommond, mifare_para_t * myMifarePara);
extern void MifareLib_MFIdrt(PX8 mifareDataOut, PX8 mifareCommond, mifare_para_t * myMifarePara);
extern void MifareLib_halt(PX8 result, PX8 content);
extern u8 MifareLib_checkAcce(PM8 accCondBlockAdd, u8 currentSector, u8 relatiBlock, u8 key, u8 config);

#define MifareLib_send(pData, fullness, flags)					ecpCiu_TxBuffer(pData, fullness, flags)
#define MifareLib_recv(pData, dataSize, fullness, flags, fsc)	ecpCiu_RxBuffer(pData, dataSize, fullness, flags, fsc)


#endif /* MIFARELIB_H_ */
