#ifndef __MHSCPU_CRC_H
#define __MHSCPU_CRC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhcard.h"
#include "mhcard_conf.h"
//#include "driver_rom_card.h"

     
typedef struct
{
    __IO uint32_t CRC_CSR;
    __O  uint32_t CRC_INI;
    union
    {
        __I uint32_t DOUT;
        __O uint32_t  DIN;
    } CRC_DATA;
    __IO uint32_t CRC_MASK;
} CRC_TypeDef;

//#define CRC                                     ((CRC_TypeDef *)CRC_BASE)
#define CRC_BASE                            (MHCARD_AHB_BASE + 0x2000)
#define CRC                                 ((CRC_TypeDef *)CRC_BASE)


#define ALGIN4_UP(a)                        (((uint32_t)(a)+0x03)&~0x03)
#define ALGIN4_DOWN(a)                      (((uint32_t)(a))&~0x03)     

#define XOR_OUT_SEL_SET			            ((uint32_t)0x20)
#define XOR_OUT_SEL_RESET		            (~(uint32_t)0x20)

#define REV_OUT_SEL_SET			            ((uint32_t)0x10)
#define REV_OUT_SEL_RESET		            (~(uint32_t)0x10)

#define REV_IN_SEL_SET			            ((uint32_t)0x04)
#define REV_IN_SEL_RESET		            (~(uint32_t)0x04)

#define TYPE_SEL_SET		                ((uint32_t)0x02)
#define TYPE_SEL_RESET			            (~(uint32_t)0x02)

#define POLY_SEL_SET		                ((uint32_t)0x01)
#define POLY_SEL_RESET			            (~(uint32_t)0x01)

#define CRC_INIT_VALUE_0		            ((uint32_t)0)
#define CRC16_INIT_VALUE_FF	                ((uint32_t)0xffff)
#define CRC32_INIT_VALUE_FF	                ((uint32_t)0xffffffff)

//CRC_CSR
#define CRC_CSR_RSVD                        BITS(31,9)
#define CRC_CSR_CRC_BSY                     BIT(8)
#define CRC_CSR_BYTE_NUM_Pos                (6)
#define CRC_CSR_BYTE_NUM_Mask               (0x3 << CRC_CSR_BYTE_NUM_Pos)
#define CRC_CSR_BYTE_NUM_1                  (0x00)
#define CRC_CSR_BYTE_NUM_2                  (0x01)
#define CRC_CSR_BYTE_NUM_3                  (0x02)
#define CRC_CSR_BYTE_NUM_4                  (0x03)

#define CRC_CSR_XOR_OUT                     BIT(5)
#define CRC_CSR_REV_OUT                     BIT(4)
#define CRC_CSR_BYTE_REV                    BIT(3)
#define CRC_CSR_REV_IN                      BIT(2)
#define CRC_CSR_TYPE_CRC32                  BIT(1)
#define CRC_CSR_POL                         BIT(0)

//CRC INI
#define CRC_INI_INI                         BITS(31,0)

//CRC DATA
#define CRC_MASK_PRE                        BIRS(31,0)


#define CRC_16                           (0x01U)
#define CRC_16_CCITT                     (0x02U)
#define CRC_16_Modbus                    (0x03U)
#define CRC_CCITT_0xffff                 (0x04U)
#define CRC_CCITT_XModem                 (0x05U)
#define CRC_32                           (0x06U)
#define CRC_32_FF                        (0x07U)
#define CRC_16_CUSTOM                    (0x08U)
#define IS_CRC_OPT(OPT)                     ((OPT) == CRC_16 ||\
                                            (OPT) == CRC_16_CCITT ||\
                                            (OPT) == CRC_16_Modbus ||\
                                            (OPT) == CRC_CCITT_0xffff ||\
                                            (OPT) == CRC_CCITT_XModem ||\
                                            (OPT) == CRC_32 ||\
                                            (OPT) == CRC_32_FF ||\
                                            (OPT) == CRC_16_CUSTOM )

typedef enum
{
	CRC_Poly_16_1021 = 0x01,
	CRC_Poly_16_8005 = 0x02,
	CRC_Poly_32_04C11DB7 = 0x03
}CRC_Poly_TypeDef;

typedef enum
{
	CRC_PolyMode_Normal = 0x01,
	CRC_PolyMode_Reversed = 0x02,
}CRC_PolyMode_TypeDef;

typedef struct
{
	CRC_Poly_TypeDef CRC_Poly;
	CRC_PolyMode_TypeDef CRC_PolyMode;
	uint32_t CRC_Init_Value;
	uint32_t CRC_Xor_Value;
}CRC_ConfigTypeDef;


#define CRC_INIT_(csr, ini) {\
            CRC->CRC_CSR = csr;\
            CRC->CRC_INI = ini;\
}

#define CRC_IN(dat) {\
            while(CRC->CRC_CSR & BIT(8));\
            CRC->CRC_DATA.DIN = dat;\
}

#define CRC_WAIT()  {\
            while(CRC->CRC_CSR & BIT(8));\
}

#define CRC_OUT()   (CRC->CRC_DATA.DOUT)


void CRC_DeInit(void);

uint32_t CRC_CalcBlockCRC(uint32_t Opt, void *PvAddr, uint32_t Size);
uint32_t CRC_Calc(CRC_ConfigTypeDef *CRC_ConfigStruct, uint8_t *PvAddr, uint32_t Len);

void CRC_CalcStart(uint32_t Opt);
void CRC_CalcUpdate(void *PvAddr, uint32_t Size);
uint32_t CRC_CalcStop(void);


#ifdef __cplusplus
}
#endif

#endif
