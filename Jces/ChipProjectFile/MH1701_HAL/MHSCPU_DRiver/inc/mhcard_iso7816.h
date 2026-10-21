#ifndef __MHCARD_ISO7816_H
#define __MHCARD_ISO7816_H
#include "mhcard.h"

/**************************** hardware defines ************************************/
typedef struct
{
    __IO  uint32_t  MODHW;      //0x00
    __O   uint32_t  RES0;       
    __IO  uint32_t  CTRL;       //0x08
    __O   uint32_t  STAT;      //0x0C
    __IO  uint32_t  INTIO1;     //0x10
    __O   uint32_t  RES1;
    __IO  uint32_t  EDCCTRL;    //0x18
    __IO  uint32_t  BGTCTRL;    //0x1C
    __IO  uint32_t  BUFHW;      //0x20
    __O   uint32_t  RES2;
    __IO  uint32_t  ETUDATA;    //0x28
    __IO  uint32_t  BGTDATA;    //0x2C
    __IO  uint32_t  CWTDATA;    //0x30
    __IO  uint32_t  EDCDATA;    //0x34
    
} SCI_SLAVE_TypeDef;
#define SCI_SLAVE_BASE                          (MHCARD_APB_BASE)
#define SCI                                    ((SCI_SLAVE_TypeDef *)SCI_SLAVE_BASE)
/**************************** hardware defines ************************************/



//SCIMODHW bits

#define SCI_MODHW_SEND_NULL_EN          (BIT(8))          

#define SCI_MODHW_RETRY_SEL_Pos        (5)
#define SCI_MODHW_RETRY_SEL_Msk        (BITS(7,5))
#define SCI_MODHW_RETRY_0              (0 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_1              (1 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_2              (2 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_3              (3 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_4              (4 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_5              (5 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_6              (6 << SCI_MODHW_RETRY_SEL_Pos)
#define SCI_MODHW_RETRY_7              (7 << SCI_MODHW_RETRY_SEL_Pos)

#define SCI_MODHW_EX_ETU_SEL_Pos          (3)
#define SCI_MODHW_EX_ETU_SEL_Msk          (BITS(4,3))
#define SCI_MODHW_EX_ETU_0                (0 << SCI_MODHW_EX_ETU_SEL_Pos)
#define SCI_MODHW_EX_ETU_1                (1 << SCI_MODHW_EX_ETU_SEL_Pos)
#define SCI_MODHW_EX_ETU_2                (2 << SCI_MODHW_EX_ETU_SEL_Pos)
#define SCI_MODHW_EX_ETU_4                (3 << SCI_MODHW_EX_ETU_SEL_Pos)


#define SCI_MODHW_PAD_TYPE_SEL_Pos     (2)
#define SCI_MODHW_PAD_TYPE_SEL_Msk     (BIT(2))
#define SCI_MODHW_PAD_TYPE_OD          (0 << SCI_MODHW_PAD_TYPE_SEL_Pos)
#define SCI_MODHW_PAD_TYPE_PP          (1 << SCI_MODHW_PAD_TYPE_SEL_Pos)

#define SCI_MODHW_DIRC_SEL_Pos         (1)
#define SCI_MODHW_DIRC_SEL_Msk         (BIT(1))
#define SCI_MODHW_DIRC_LSB_FIRST       (0 << SCI_MODHW_DIRC_SEL_Pos)
#define SCI_MODHW_DIRC_MSB_FIRST       (1 << SCI_MODHW_DIRC_SEL_Pos)

#define SCI_MODHW_TMODE_SEL_Pos        (0)
#define SCI_MODHW_TMODE_SEL_Msk        (BIT(0))
#define SCI_MODHW_TMODE_T0             (0 << SCI_MODHW_TMODE_SEL_Pos)
#define SCI_MODHW_TMODE_T1             (1 << SCI_MODHW_TMODE_SEL_Pos)


//SCICTRL bits
#define SCI_CTRL_EXTRST_INT_EDGE_Pos   (15)
#define SCI_CTRL_EXTRST_INT_EDGE_Msk   (BIT(15))
#define SCI_CTRL_EXTRST_INT_RISE_EDGE  (0 << SCI_CTRL_EXTRST_INT_EDGE_Pos)
#define SCI_CTRL_EXTRST_INT_FALL_EDGE  (1 << SCI_CTRL_EXTRST_INT_EDGE_Pos)

#define SCI_CTRL_PD_EN                 (BIT(14))
#define SCI_CTRL_PU_EN                 (BIT(13))
#define SCI_CTRL_EXTRST_INT_EN         (BIT(12))
#define SCI_CTRL_CWT_INT_EN            (BIT(11))
#define SCI_CTRL_BGT_INT_EN            (BIT(10))
#define SCI_CTRL_TX_FIFO_EMPTY_INT_EN  (BIT(9))
#define SCI_CTRL_TX_INT_EN             (BIT(8))
#define SCI_CTRL_RX_INT_EN             (BIT(7))
#define SCI_CTRL_SOI_INT_EN            (BIT(6))
#define SCI_CTRL_TX_FIFO_CLR           (BIT(5))
#define SCI_CTRL_RX_FIFO_CLR           (BIT(4))
#define SCI_CTRL_RETRY_EN              (BIT(3))
#define SCI_CTRL_IO_PAD2_EN            (BIT(2))
#define SCI_CTRL_IOPAD1_EN             (BIT(1))

#define SCI_CTRL_IOMOD_SEL_Pos         (0)
#define SCI_CTRL_IOMOD_SEL_Msk         (BIT(0))
#define SCI_CTRL_IOMOD_HARDIO          (0 << SCI_CTRL_IOMOD_SEL_Pos)
#define SCI_CTRL_IOMOD_SOFTIO          (1 << SCI_CTRL_IOMOD_SEL_Pos)

//SCISTAT
#define SCI_STAT_CWT                   (BIT(7))
#define SCI_STAT_BGT                   (BIT(6))
#define SCI_STAT_TX_ERROR              (BIT(5))
#define SCI_STAT_TX_FIFO_FULL          (BIT(4))
#define SCI_STAT_TX_FIFO_NONEMPTY      (BIT(3))
#define SCI_STAT_RX_ERROR              (BIT(2))
#define SCI_STAT_RX_FIFO_FULL          (BIT(1))
#define SCI_STAT_RX_FIFO_NONEMPTY      (BIT(0))

//SCIINTIO1
#define SCI_INTIO_EXTRST_INT           (BIT(6))
#define SCI_INTIO_CWT_INT              (BIT(5))
#define SCI_INTIO_BGT_INT              (BIT(4))
#define SCI_INTIO_TX_FIFO_EMPTY_INT    (BIT(3))
#define SCI_INTIO_TX_INT               (BIT(2))
#define SCI_INTIO_RX_INT               (BIT(1))
#define SCI_INTIO_PAD_INT              (BIT(0))

#define SCI_INTIO_ALL                  (SCI_INTIO_EXTRST_INT          \
                                        | SCI_INTIO_CWT_INT           \
                                        | SCI_INTIO_BGT_INT           \
                                        | SCI_INTIO_TX_FIFO_EMPTY_INT \
                                        | SCI_INTIO_TX_INT            \
                                        | SCI_INTIO_RX_INT            \
                                        | SCI_INTIO_PAD_INT           \
                                        )

//SCIEDCCTRL
#define SCI_EDCCTRL_EDC_EN             (BIT(0))
    
//SCIBGTCTRL
#define SCI_BGTCTRL_CWT_EN             (BIT(1))
#define SCI_BGTCTRL_BGT_EN             (BIT(0))

//SCIBUFHW
#define SCI_BUFHW_DATA_Pos             (0)
#define SCI_BUFHW_DATA_Msk             (BITS(7,0))

//SCIETUDATA
#define SCI_ETU_DATA_Pos               (0)
#define SCI_ETU_DATA_Msk               (BITS(12,0))
 
//SCIBGTDATA
#define SCI_BGT_DATA_Pos               (0)
#define SCI_BGT_DATA_Msk               (BITS(4,0))
  
//SCICWTDATA
#define SCI_CWT_DATA_Pos               (0)
#define SCI_CWT_DATA_Msk               (BITS(15,0))

//SCIEDCDATA
#define SCI_EDC_DATA_Pos               (0)
#define SCI_EDC_DATA_Msk               (BITS(7,0))

/**************************** hardware defines ************************************/

//#define PBOC_RETRYCNT      4
//#define ISO_RETRYCNT       5
//#define PBOC_GT            16

//#define SCI_MODHW_RETRY(t)          (((t)&0x7) << 5)
//#define SCI_MODHW_EXT_ETU(e)        (((e)&0x3) << 3)





uint32_t SCI_GetCwtData(void);
uint32_t SCI_GetBgtData(void);

#define IS_CWT_DATA(CWT)        (0 == ((CWT) & ~(SCI_CWT_DATA_Msk)))
void SCI_SetCwtData(uint16_t u16CwtData);

#define IS_BGT_DATA(BGT)        (0 == ((BGT) & ~(SCI_BGT_DATA_Msk)))
void SCI_SetBgtData(uint16_t u16BgtData);

#define IS_BGT_CWT(WHOM)        (0 == ((WHOM) & ~(SCI_BGTCTRL_CWT_EN | SCI_BGTCTRL_BGT_EN)))
void SCI_BgtCwtEnable(uint32_t u32Whom, FunctionalState state);
void SCI_AutoSendNULL(FunctionalState state);

#define IS_SCI_T_MODE(MODE)     ((MODE == SCI_MODHW_TMODE_T1) || (MODE == SCI_MODHW_TMODE_T0))
void SCI_SetTMode(uint32_t u32TMode);

#define IS_SCI_RETRY_TIMES(RTIMES)      (0 == ((RTIMES) & ~(SCI_MODHW_RETRY_SEL_Msk)))
void SCI_SetRetryTimes(uint32_t u32ReTimes);

#define IS_SCI_EXTRA_ETU(EXETU)     (0 == ((EXETU) & ~(SCI_MODHW_EX_ETU_SEL_Msk)))
void SCI_SetExtraETU(uint32_t u32ExETU);

uint8_t SCI_GetChar(void);

uint8_t SCI_SendChar(uint8_t u8Char);

#define IS_SCI_IT(IT)   (0 == ((IT) & ~(SCI_CTRL_EXTRST_INT_EN           \
                                        |SCI_CTRL_CWT_INT_EN             \
                                        |SCI_CTRL_BGT_INT_EN             \
                                        |SCI_CTRL_TX_FIFO_EMPTY_INT_EN   \
                                        |SCI_CTRL_TX_INT_EN              \
                                        |SCI_CTRL_RX_INT_EN              \
                                        )))       
void SCI_ITConfig(uint32_t u32Int, FunctionalState state);

#define IS_SCI_FIFO_CLR(FIFO)       (0 == ((FIFO) & ~(SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR)))
void SCI_ClearFifo(uint32_t u32Whom);

#define IS_SCI_FLAG(FLAG)   (0 == ((FLAG) & ~(SCI_STAT_CWT            \
                                             |SCI_STAT_BGT           \
                                             |SCI_STAT_TX_ERROR      \
                                             |SCI_STAT_TX_FIFO_FULL  \
                                             |SCI_STAT_TX_FIFO_NONEMPTY \
                                             |SCI_STAT_RX_ERROR      \
                                             |SCI_STAT_RX_FIFO_FULL  \
                                             |SCI_STAT_RX_FIFO_NONEMPTY \
                                             )))
FlagStatus SCI_GetStatus(uint32_t u32Flag);

#define IS_SCI_IT_FLAG(FLAG)    (0 == ((FLAG) & ~(SCI_INTIO_EXTRST_INT         \
                                                 |SCI_INTIO_CWT_INT           \
                                                 |SCI_INTIO_BGT_INT           \
                                                 |SCI_INTIO_TX_FIFO_EMPTY_INT \
                                                 |SCI_INTIO_TX_INT            \
                                                 |SCI_INTIO_RX_INT            \
                                                 |SCI_INTIO_PAD_INT           \
                                                 )))
ITStatus SCI_GetITStatus(uint32_t u32ITStatus);
void SCI_ClearIT(uint32_t u32ITStatus);
void SCI_RetryEnable(FunctionalState state);

#define IS_SCI_RST_INT_EDGE(EDGE)   (((EDGE) == SCI_CTRL_EXTRST_INT_FALL_EDGE) \
                                      || (((EDGE) == SCI_CTRL_EXTRST_INT_RISE_EDGE))
void SCI_ExRstIntEdge(uint32_t u32Edge);
void SCI_SetEtuData(uint16_t u16Etu);
uint8_t SCI_GetEdcData(void);


#endif
