#ifndef __UART_HAL_H__
#define	__UART_HAL_H__

#include "DataOperation.h"
#include "SysConfigProfile.h"
#include "MixMode.h"
/*=================================================================*
 * Types:
 *=================================================================*/

/*=================================================================*
 * Defines:
 *=================================================================*/
#define RECEIVE_COMMAND										0x11
#define RECEIVE_DATA										0x12
#define RECEIVE_PPS											0x13
#define SEND_PRC_BYTE										0x60
#define SEND_ACK_BYTE										0x66
#define SEND_ATR                    						0x20
#define SEND_DATA											0x21
#define SEND_STATUS											0x22
#define SEND_60												0x23
#define SEND_PPS											0x24


//T = 0
#define ATR_T0_DIRECT_TS									0x3B
#define ATR_T0_INVERSE_TS									0x3F

#define UART_STATUS_INIT       		0xFF     //
#define UART_STATUS_NONERESET  		0x00     //
#define UART_STATUS_COLDRESET     	0x01     //
#define UART_STATUS_WARMRESET      	0x02     //



#define PHDD_UART_OK                						0x5A5A
#define PHDD_UART_ERROR             						0xA5A5
#define PHDD_UART_NOT_INITIALIZED   						0x5AA5
#define PHDD_UART_BAD_PARAMETER     						0xAA55
#define PHDD_UART_RX_OVERFLOW       						0xBB33
#define PHDD_UART_RX_BUF_FULL       						0x33BB
#define PHDD_UART_BUSY              						0xB33B

#define PHDD_UART_BAUDRATE_BAD_PARAMETER					0
#define PHDD_UART_BAUDRATE_SUPPORT_PARAMETER				1

/** @brief PICC->PCD baudrate setting for 106kbits/s. */
#define PHDD_CIU_TX_BAUDRATE_106K   						0x00
/** @brief PCD->PICC baudrate setting for 106kbits/s. */
#define PHDD_CIU_RX_BAUDRATE_106K							0x00
/** @brief PICC->PCD baudrate setting for 212kbits/s. */
#define PHDD_CIU_TX_BAUDRATE_212K   						0x10
/** @brief PCD->PICC baudrate setting for 212kbits/s. */
#define PHDD_CIU_RX_BAUDRATE_212K   						0x01
/** @brief PICC->PCD baudrate setting for 424kbits/s. */
#define PHDD_CIU_TX_BAUDRATE_424K   						0x20
/** @brief PCD->PICC baudrate setting for 424kbits/s. */
#define PHDD_CIU_RX_BAUDRATE_424K   						0x02
/** @brief PICC->PCD baudrate setting for 848kbits/s. */
#define PHDD_CIU_TX_BAUDRATE_848K   						0x30
/** @brief PCD->PICC baudrate setting for 848kbits/s. */
#define PHDD_CIU_RX_BAUDRATE_848K   						0x03

/** @brief Error function SetBaudrate called with bad parameter, SetBaudrate returns an 8 bit value. */
#define PHDD_CIU_BAUDRATE_BAD_PARAMETER     				(0xD4)
/** @brief Error function SetBaudrate called with unsupported baudrate, SetBaudrate returns an 8 bit value. */
#define PHDD_CIU_BAUDRATE_NOT_SUPPORTED     				(0xD9)
/** @brief Error function SetBaudrate failed because of invalid access conditions for SecRow, SetBaudrate returns an 8 bit value. */
#define PHDD_CIU_BAUDRATE_ACCESS_COND_ERR   				(0xCF)
/** @brief Baudrate was set or read. */
#define PHDD_CIU_BAUDRATE_OK                				(0xA5)

/* TX options */
#define PHDD_UART_TX_OPTION_LCT								0x01

/** @brief recieve rats command when in the mifare activation. */
#define ECP_MIFARE_L3_RATS_RECEIVED							0xC2
/** @brief recieve halt command when in the mifare activation. */
#define ECP_MIFARE_L3_HALT_RECEIVED							0xC1
/** @brief recieve deselect command when in the mifare activation. */
#define ECP_MIFARE_L4_DESLECT_RECEIVED						0xC9

/** @brief success when the mifare read or write. */
#define ECP_MIFARE_READ_WRITE_OK							0xA5
/** @brief error when the mifare read and write. */
#define ECP_MIFARE_READ_WRITE_ERROR							0xB3

extern MixModeStatus X_A	g_mixModeStatus;
extern volatile u8 g_Iso7816Reset;
extern volatile u8 g_Iso7816ReceiveCommandflag; 
extern volatile u8 commandcount; 
#if HAL__IO__ISO7816_T0
	extern void 	ecpUart_Init(void);
	extern void 	ecpUart_T0Init(void);
	extern void 	ecpUart_SendDirectTs(void);
	extern void 	ecpUart_SendinverseTs(void);
	extern void		ecpUart_TsDelay(void);
	extern void 	ecpUart_AtrDelay(void);
	extern void 	ecpUart_SendRemaindAtr(PX8 atrBuf, u8 atrLen, u8 type)    SMALL_MODE;
	extern u8		ecpUart_CheckFiDi(u8 FiDi);
	extern void		ecpUart_SetFiDi(u8 FiDi);
	extern void 	ecpUart_GetRxLength(PX8 buf);
	extern void		ecpUart_RxByte(PX8 buf);
	extern u8 		ecpUart_getRxLen(void);
	extern void		ecpUart_RxMulBytes(PR8 buf, u16 len, u8 flag);
	extern void		ecpUart_TxMulBytes(PR8 buf, u16 len, u8 flag)		SMALL_MODE;
	extern void		ecpUart_EnterSleepMode(void);
	extern void		ecpUart_GuardTime(void);
	extern boolean	ecpUart_IsRisingState(void);
	extern void		ecpUart_StopTx(void);
	extern boolean  ecpUart_IsReset(void);
	extern void		ecpUart_CloseInterrupt(void);
    extern void		ecpUart_OpenInterrupt(void);
    extern u8       ecpUart_DiscoverError(void);
	extern void 	ecpUart_SetRxLength(u16 len);
//	extern void ecpUart_irqRxMulBytes(PR8 buf, PR16 plen);
	extern u8 		ecpUart_getRxLen(void);
	extern void 	ecpUart_ClrRxIrqStatus(void);
	extern void		ecpUart_EnRxIrq(void);
    extern void ecpUart_ClearResetFlag(void);
    extern boolean ecpUart_WarnReset(void);
    extern void ecpUart_SetResetSendAtrFlag(void);
	extern u8 ecpUart_TxRxRunning(void);
	extern void ecpUart_ClearFifo(void);
	extern void Uart_RxIrqHandler(void);
	extern void Uart_RstIrqHandler(void);
	extern void Uart_RstIrqHandlerProcess(void);
	extern void Uart_RxIrqHandlerProcess(void);
#else
	#define	ecpUart_Init(void)
	#define ecpUart_T0Init()
	#define ecpUart_SendDirectTs()
	#define ecpUart_SendinverseTs()
	#define	ecpUart_TsDelay()
	#define ecpUart_AtrDelay()
	#define ecpUart_SendRemaindAtr(atrBuf, atrLen, type)
	#define ecpUart_CheckFiDi(FiDi)									0
	#define ecpUart_SetFiDi(FiDi)
	#define	ecpUart_GetRxLength()
	#define	ecpUart_RxByte(buf)
	#define ecpUart_RxMulBytes(buf, len, flag)
	#define ecpUart_TxMulBytes(buf, len, flag)
	#define ecpUart_EnterSleepMode()
	#define ecpUart_GuardTime()
	#define ecpUart_IsRisingState()
	#define ecpUart_StopTx()
	#define ecpUart_CloseInterrupt()
	#define ecpUart_OpenInterrupt()
	#define ecpUart_DiscoverError()
	#define ecpUart_SetRxLength(len)
    #define ecpUart_ClearResetFlag 
    #define boolean ecpUart_WarnReset(void)
    #define ecpUart_SetResetSendAtrFlag(void)
	#define ecpUart_TxRxRunning(void)     0
	#define ecpUart_ClearFifo(void)
	#define Uart_RxIrqHandler(void)
	#define Uart_RstIrqHandler(void)
#endif

#if HAL__IO__ISO7816_T1

	extern u16  ecpUartCalT1Crc(PX8 buf, u16 length);
	extern void ecpUartT1_SetCrcCal(void);

	/*===========================================================================*/
	/**
	 * \ingroup ecpUart_T0Init
	 *
	 * @brief  The function ecpUart_T1Init() init the T1 uart.
	 *
	 * @param  none
	 *
	 * @return none
	 *
	 * @note   None
	 */
	/*===========================================================================*/
	extern void ecpUart_T1Init(void);

	/*===========================================================================*/
	/**
	 * \ingroup ecpDriverMethod
	 *
	 * @brief  The function ecpUART_RxBuffer(uRxBuffer) receives data via the UART
	 *         and copies it to the given buffer.
	 *
	 * @param  pphBuffer_t uRxBuffer
	 *         pointer to buffer to store the received data
	 *
	 * @return PHDD_UART_OK,                                                     \n
	 *         PHDD_UART_NOT_INITIALIZED,                                        \n
	 *         PHDD_UART_BUSY
	 *
	 * @note   the memeber "fullness" of uRxBuffer should be modifed and it records
	 *		   the number of received bytes.
	 */
	/*===========================================================================*/
	extern u16 ecpUART_RxBuffer(PR8 pData, u16 pDataSize, PX16 fullness, u16 cwt, u8 isEnterSleepMode);

	/*===========================================================================*/
	/**
	 * \ingroup ecpDriverMethod
	 *
	 * @brief  The function ecpUart_TxBuffer(uTxBuffer,uTxOption) transmits the
	 *         content of a buffer via the UART
	 *
	 * @param  pphBuffer_t pTxBuffer
	 *         pointer to buffer containing the data to be transmitted
	 * @param  u8 txOptions
	 *         RFU
	 *
	 * @return PHDD_UART_OK,                                                     \n
	 *         PHDD_UART_NOT_INITIALIZED,                                        \n
	 *         PHDD_UART_BAD_PARAMETER,                                          \n
	 *         PHDD_UART_BUSY
	 *
	 * @note   no members of the uTxBuffer should be modified
	 */
	/*===========================================================================*/
	extern u16 ecpUart_TxBuffer(PR8 pData, u16 pDataSize, u16 fullness, u8 txOptions);

#else

	#define ecpUartCalT1Crc(buf, length)																		0
	#define ecpUartT1_SetCrcCal()
	#define ecpUart_T1Init()
	#define ecpUART_RxBuffer(pData, pDataSize, fullness, cwt, isEnterSleepMode)									0
	#define ecpUart_TxBuffer(pData, pDataSize, fullness, txOptions)												0

#endif

#endif
