#include "mhcard.h"
#include "stdio.h"
#include "mhcard_stdio.h"
#include "mhcard_iso7816.h"
#include "mhcard_spi.h"
#include "mhcard_sysctrl.h"
#ifndef STD_IO_PORT
//    #error "please select the printf port\n(STD_IO_PORT = STD_IO_PORT_SPI)"
    /*
    STD_IO_PORT=0 : NULL
    STD_IO_PORT=1 : SCI(just for output)
    STD_IO_PORT=2 : SPI
    STD_IO_PORT=3 : SCI_T0
    STD_IO_PORT=4 : SCI_UART 
    */
#endif
#if (STD_IO_PORT == STD_IO_PORT_SCI_T0)
void PendSV_Init(void);
void boot_SendT0ATR(uint8_t u8TA1);

#define MSG_PIN_PULL_UP         (GPIO->CON = (GPIO->CON & ~(0x03 << (4 << 1))) | (0x01 << (4 << 1)))
#define MSG_PIN_PULL_DOWN       (GPIO->CON = (GPIO->CON & ~(0x03 << (4 << 1))) | (0x02 << (4 << 1)))
void SCIT0_InitForPrintf(void)
{
    NVIC_DisableIRQ(SCIS_IRQn);
    SYSCTRL_ClockEnable(SYSCTRL_Periph_ISO7816, ENABLE);
    SYSCTRL_SoftReset(SYSCTRL_Periph_ISO7816);
    
    SCI_RetryEnable(ENABLE);
    SCI_SetRetryTimes(SCI_MODHW_RETRY_7);
    SCI_SetExtraETU(SCI_MODHW_EX_ETU_1);
    
    SCI_BgtCwtEnable(SCI_BGTCTRL_BGT_EN | SCI_BGTCTRL_CWT_EN, ENABLE);
    SCI_SetBgtData(0x1F);
    SCI_SetCwtData(0xA14);
    SCI_ExRstIntEdge(SCI_CTRL_EXTRST_INT_RISE_EDGE);
    
    SCI_ClearIT(SCI_INTIO_ALL);
    SCI_ClearFifo(SCI_CTRL_RX_FIFO_CLR | SCI_CTRL_TX_FIFO_CLR);
    SCI_ITConfig(SCI_CTRL_RX_INT_EN | SCI_CTRL_EXTRST_INT_EN, ENABLE);
    
    GPIO->MODE_H = (GPIO->MODE_H & 0xFFFFFF00) | 0x0D;
    NVIC_EnableIRQ(SCIS_IRQn);
    PendSV_Init();
    
    while (0 == (GPIO->IOD & BIT(4 + 8)));//wait sci RST pin to be high
    
    boot_SendT0ATR(0x11);
}
uint8_t u8PendFlag = 0;
void PendSV_Init(void)
{
    NVIC_SetPriority(PendSV_IRQn, 0x01);
}
void PendSV_Pending(void)
{
    u8PendFlag = 1;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}
uint32_t T0_CmdHandler(void);
void PendSV_Handler(void)
{
    u8PendFlag = 0;
    T0_CmdHandler();
    if (u8PendFlag == 0)
    {
        SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;
    }
}

#define IS_QUEUE_C_EMPTY(q)               (((q).head&(sizeof((q).buf) - 1))==((q).tail&(sizeof((q).buf) - 1)))
#define IS_QUEUE_C_FULL(q)                (((q).head&(sizeof((q).buf) - 1))==(((q).tail+1)&(sizeof((q).buf) - 1)))
#define QUEUE_C_LEN(q)                    (((q).tail+(sizeof((q).buf))-(q).head)&(sizeof((q).buf) - 1))
#define QUEUE_C_SPARE(q)                  (((q).head+(sizeof((q).buf) - 1)-(q).tail)&(sizeof((q).buf) - 1))
#define QUEUE_C_HEAD_NEXT(q)              (((q).head+1)&(sizeof((q).buf) - 1))
#define QUEUE_C_TAIL_NEXT(q)              (((q).tail+1)&(sizeof((q).buf) - 1))
#define QUEUE_C_POSI(q, n)                ((n)&(sizeof((q).buf) - 1))


/*S* TPDU Protocol drivers */
#define TPDU_SEND_BUFF_SIZE          (BIT(9))
#define TPDU_RECV_BUFF_SIZE          (BIT(9))

typedef struct
{
    volatile uint16_t       head;
    volatile uint16_t       tail;
    volatile uint16_t       len;
    volatile uint16_t       err_stat;
    volatile uint8_t        buf[TPDU_SEND_BUFF_SIZE];
} TPDU_SEND_BUF;

typedef struct
{
    volatile uint16_t       head;
    volatile uint16_t       tail;
    volatile uint16_t       len;
    volatile uint16_t       err_stat;
    volatile uint8_t        buf[TPDU_RECV_BUFF_SIZE];
} TPDU_RECV_BUF;

TPDU_SEND_BUF tpduSendBuf;
TPDU_RECV_BUF tpduRecvBuf;

uint32_t TPDU_SendChar(uint8_t u8Char)
{
    while (QUEUE_C_SPARE(tpduSendBuf) == 0);
    tpduSendBuf.tail = QUEUE_C_POSI(tpduSendBuf, tpduSendBuf.tail);
    tpduSendBuf.buf[tpduSendBuf.tail++] = u8Char;
    
    SCI_ITConfig(SCI_CTRL_BGT_INT_EN, ENABLE);
    SCI_ITConfig(SCI_CTRL_CWT_INT_EN, DISABLE);
    
    return u8Char;
}

uint32_t TPDU_SendBuff(void *pvBuff, uint32_t u32Len)
{
    uint32_t i;
    for (i = 0; i < u32Len; i++)
    {
        while (QUEUE_C_SPARE(tpduSendBuf) == 0);
        tpduSendBuf.tail = QUEUE_C_POSI(tpduSendBuf, tpduSendBuf.tail);
        tpduSendBuf.buf[tpduSendBuf.tail++] = ((uint8_t *)pvBuff)[i];
    }
    SCI_ITConfig(SCI_CTRL_BGT_INT_EN, ENABLE);
    SCI_ITConfig(SCI_CTRL_CWT_INT_EN, DISABLE);
    return i;
}

uint32_t TPDU_RecvChar(void)
{
    if (QUEUE_C_LEN(tpduRecvBuf))
    {
        tpduRecvBuf.head = QUEUE_C_POSI(tpduRecvBuf, tpduRecvBuf.head);
        return (uint32_t)tpduRecvBuf.buf[tpduRecvBuf.head++];
    }
    return ~0UL;
}

uint32_t TPDU_RecvBuff(void *pvBuff, uint32_t u32Len)
{
    uint32_t i;
    for (i = 0; i < u32Len; i++)
    {
        while (QUEUE_C_LEN(tpduRecvBuf) < 1);
        tpduRecvBuf.head = QUEUE_C_POSI(tpduRecvBuf, tpduRecvBuf.head);
        ((uint8_t *)pvBuff)[i] = tpduRecvBuf.buf[tpduRecvBuf.head++];
    }
    return i;
}


void SCIS_IRQHandler(void)
{
    uint8_t u8Recv;
    static uint32_t su32Count = 0;
    //SCI->INTIO1 |= ~BIT(5);
    if (SCI->CTRL & SCI_CTRL_EXTRST_INT_EN
        && SCI->INTIO1 & SCI_INTIO_EXTRST_INT)
    {
        SYSCFG->RST_EN = 0x0000FFFF;
        while(1);
    }
    if (SCI->STAT & SCI_STAT_TX_ERROR)
    {
        SCI->CTRL |= SCI_CTRL_TX_FIFO_CLR;
    }
    if (SCI->STAT & SCI_STAT_RX_ERROR)
    {
        SCI->CTRL |= SCI_CTRL_RX_FIFO_CLR;
    }
    if (SCI->INTIO1 & SCI_INTIO_RX_INT 
        && SCI->CTRL & SCI_CTRL_RX_INT_EN)
    {
        SCI->INTIO1 |= SCI_INTIO_RX_INT;           //Safe code.
        while (SCI->STAT & (SCI_STAT_RX_FIFO_NONEMPTY | SCI_STAT_RX_FIFO_FULL))
        {
            if (QUEUE_C_SPARE(tpduRecvBuf) > 0)
            {
                u8Recv = SCI->BUFHW;
                tpduRecvBuf.tail = QUEUE_C_POSI(tpduRecvBuf, tpduRecvBuf.tail);
                tpduRecvBuf.buf[tpduRecvBuf.tail++] = u8Recv;
                su32Count++;
            }
            else
            {
                break;
            }
            //Enable CWT
            SCI->CTRL |= SCI_CTRL_CWT_INT_EN;
            
        }
        PendSV_Pending();
        
    }
    else if (SCI->INTIO1 & SCI_INTIO_CWT_INT
             && SCI->CTRL & SCI_CTRL_CWT_INT_EN)
    {
        SCI->INTIO1 |= SCI_INTIO_CWT_INT;
        
        
        if (5 == su32Count)
        {
            SCI->BUFHW = 0x60;        //Recv a new CMD,keep online
        }
    }
    else if (SCI->INTIO1 & SCI_INTIO_TX_FIFO_EMPTY_INT
             && SCI->CTRL & SCI_CTRL_TX_FIFO_EMPTY_INT_EN)
    {
        SCI->INTIO1 |= SCI_INTIO_TX_FIFO_EMPTY_INT;
        //SCI->INTIO1 &= ~SCI_INTIO1_TX_EMPTY_INT;
        while (!(SCI->STAT & SCI_STAT_TX_FIFO_FULL))
        {
            if (!QUEUE_C_LEN(tpduSendBuf))
            {
                //Disable tx int.
                SCI->CTRL &= ~SCI_CTRL_TX_FIFO_EMPTY_INT_EN;

                //Disable BGT
                SCI->CTRL &= ~SCI_CTRL_BGT_INT_EN;

                su32Count = 0;
                break;
            }
            else
            {
                tpduSendBuf.head = QUEUE_C_POSI(tpduSendBuf,tpduSendBuf.head);
                SCI->BUFHW = tpduSendBuf.buf[tpduSendBuf.head++];
            }
        }
        
    }
    

    else if (SCI->INTIO1 & SCI_INTIO_BGT_INT
             && SCI->CTRL & SCI_CTRL_BGT_INT_EN)
    {
        SCI->INTIO1 |= SCI_INTIO_BGT_INT;
        SCI->CTRL   |= SCI_CTRL_TX_FIFO_EMPTY_INT_EN;
    }

    else
    {
        ;
    }
    return;
}
/*E* TPDU Protocol drivers */


/*S* T0 Protocol drivers */
#define T0_SEND_BUFF_SIZE          (BIT(9))
#define T0_RECV_BUFF_SIZE          (BIT(9))
typedef struct
{
    volatile uint16_t       head;
    volatile uint16_t       tail;
    volatile uint16_t       len;
    volatile uint16_t       err_stat;
    volatile uint8_t        buf[T0_SEND_BUFF_SIZE];
} T0_SEND_BUF;

typedef struct
{
    volatile uint16_t       head;
    volatile uint16_t       tail;
    volatile uint16_t       len;
    volatile uint16_t       err_stat;
    volatile uint8_t        buf[T0_RECV_BUFF_SIZE];
} T0_RECV_BUF;

T0_SEND_BUF t0Send;
T0_RECV_BUF t0Recv;

typedef struct
{
    uint8_t u8Rsv[3];//for alighn;
    uint8_t u8Cla;
    uint8_t u8Ins;
    uint8_t u8P1;
    uint8_t u8P2;
    uint8_t u8P3;
    uint8_t         au8Buff[256];
} T0_CMD;

typedef struct
{
    T0_CMD cmdRecv;
    uint8_t au8RespBuff[256];
    uint8_t au8Sw[2];
    uint16_t u16RespLen;
} T0_TRANS;
uint32_t T0_Case2(T0_TRANS *pTrans)
{
    uint32_t u32Len;
    uint32_t i;
    if (pTrans->u16RespLen > 0xFF)
    {
        u32Len = 0xFF;
    }
    else
    {
        u32Len = pTrans->u16RespLen;
    }
    if (u32Len >= pTrans->cmdRecv.u8P3)
    {
        if (pTrans->cmdRecv.u8P3)
        {
            TPDU_SendChar(pTrans->cmdRecv.u8Ins);
            for (i = 0; i < pTrans->cmdRecv.u8P3; i ++)
            {
                t0Send.head = QUEUE_C_POSI(t0Send, t0Send.head);
                pTrans->au8RespBuff[i] = t0Send.buf[t0Send.head ++];
            }
            TPDU_SendBuff(pTrans->au8RespBuff, pTrans->cmdRecv.u8P3);
        }
        pTrans->u16RespLen -= pTrans->cmdRecv.u8P3;
        if (pTrans->cmdRecv.u8P3 == u32Len)
        {
            pTrans->au8Sw[0] = 0x90;pTrans->au8Sw[1] = 0x00;
        }
        else
        {
            pTrans->au8Sw[0] = 0x61;pTrans->au8Sw[1] = u32Len;
        }
    }
    else
    {
        pTrans->au8Sw[0] = 0x6c; pTrans->au8Sw[1] = pTrans->u16RespLen;
    }
    
    TPDU_SendBuff(pTrans->au8Sw, 2);
    return 0;
}

uint32_t T0_Case3(T0_TRANS *pTrans)
{
    uint32_t u32Tmp;
    if (pTrans->cmdRecv.u8P3 > 0)
    {
        TPDU_SendChar(pTrans->cmdRecv.u8Ins);
        u32Tmp = TPDU_RecvBuff(pTrans->cmdRecv.au8Buff , pTrans->cmdRecv.u8P3);
    }
    return u32Tmp;
}

uint32_t T0_Cmd_TRANS_SEND_RECV(T0_TRANS *pTrans)
{
    uint32_t u32Tmp, i;
    u32Tmp = T0_Case3(pTrans);
    if (u32Tmp != pTrans->cmdRecv.u8P3)
    {
        //recv error
    }
    if (u32Tmp > QUEUE_C_SPARE(t0Recv))
    {   
        pTrans->au8Sw[0] = 0x67;pTrans->au8Sw[1] = 0x00;
        TPDU_SendBuff(pTrans->au8Sw, 2);
        return 0;
    }
    
    //copy data to t0Recv
    for (i = 0; i < u32Tmp; i ++)
    {
        t0Recv.tail = QUEUE_C_POSI(t0Recv, t0Recv.tail);
        t0Recv.buf[t0Recv.tail++] = pTrans->cmdRecv.au8Buff[i];
    }
    
    pTrans->u16RespLen = QUEUE_C_LEN(t0Send);
    if (pTrans->u16RespLen > 0xFF)
    {
        pTrans->u16RespLen = 0xFF;
    }
    pTrans->au8Sw[0] = 0x61;pTrans->au8Sw[1] = pTrans->u16RespLen;
    TPDU_SendBuff(pTrans->au8Sw, 2);
    return 0;
}
/*RECV -> card to terminal
  SEND -> terminal to card
*/
#define BOOT_CMD_FETCH_DATA     (0xC0)
#define TRANS_SEND_RECV     0xFF    
#define TRANS_RECV          0xFE
T0_TRANS t0Trans;
uint8_t *pu8Recv = (uint8_t *)&(t0Trans.cmdRecv.u8Cla);;
uint32_t T0_CmdHandler(void)
{
    uint32_t u32Tmp;
    T0_TRANS *pTrans = &t0Trans;
    u32Tmp = TPDU_RecvChar();
    if (~0UL != u32Tmp)
    {
        *pu8Recv++ = (uint8_t)u32Tmp;
    }
    if (QUEUE_C_LEN(t0Send))       
    {
        //GPIO->IOD &= ~(0x01 << 4);  //set io low    
        MSG_PIN_PULL_DOWN;
    }
    else
    {
        //GPIO->IOD |= (0x01 << 4);   //set io high   
        MSG_PIN_PULL_UP;
    }
    if (pu8Recv > &(pTrans->cmdRecv.u8P3))       // recv a packet
    {
        switch (pTrans->cmdRecv.u8Ins)
        {
        case BOOT_CMD_FETCH_DATA:
            //Send response data.
            T0_Case2(pTrans);
            break;
        
        case TRANS_RECV:     //case 2 le=0
            pTrans->u16RespLen = QUEUE_C_LEN(t0Send);
            T0_Case2(pTrans);
            break;
        case TRANS_SEND_RECV:            //case 4
            T0_Cmd_TRANS_SEND_RECV(pTrans);
            
            break;        
        
        default:
            //Unkown INS
            pTrans->au8Sw[0] = 0x6D;pTrans->au8Sw[1] = 0x00;
            TPDU_SendBuff(pTrans->au8Sw, 2);
            break;
        }
        pu8Recv = (uint8_t *)&(pTrans->cmdRecv.u8Cla);
    }
    
}



uint32_t SCIT0_SendBuff(void *pvBuff, uint32_t u32Len)
{
    uint32_t i;
    for (i = 0; i < u32Len; i++)
    {
        if(0 == QUEUE_C_SPARE(t0Send))
        {
            break;
        }
        t0Send.tail = QUEUE_C_POSI(t0Send, t0Send.tail);
        t0Send.buf[t0Send.tail++] = ((uint8_t *)pvBuff)[i];
    }
    return i;
}

uint32_t SCIT0_SendChar(uint8_t ch)
{
    return SCIT0_SendBuff(&ch, 1);
}


uint32_t SCIT0_RecvChar(void)
{
    if (0 == QUEUE_C_LEN(t0Recv))
    {
        return ~0UL;
    }
    t0Recv.head = QUEUE_C_POSI(t0Recv, t0Recv.head);
    return (uint32_t)t0Recv.buf[t0Recv.head++];
}

/*E* T0 Protocol drivers */

SCI_SLAVE_TypeDef bb;
void boot_SendT0ATR(uint8_t u8TA1)
{
    volatile uint32_t i;
    uint8_t au8Atr[32] = {0x3B,0x60,0x00,0x00};
    uint16_t Fi[16] = {372,372,558,744,1116,1488,1860,0,0,512,768,1024,1536,2048,0,0};
    uint16_t Di[16] = {0,1,2,4,8,16,32,64,12,20,0,0,0,0,0,0};
    uint16_t u16Tmp;
    
    if (0 != Fi[ (u8TA1 >> 4) & 0x0F] && 0 != Di[u8TA1 & 0x0F])
    {
        au8Atr[0] = 0x3B;
        au8Atr[1] = 0xF0;
        au8Atr[2] = u8TA1;
        au8Atr[3] = 0x00;
        au8Atr[4] = 0x00;
        au8Atr[5] = 0x10;
        au8Atr[6] = 0x00;
        u16Tmp = 7;
    }
    else//TA1 is invalid,use default
    {
        u8TA1 = 0x11;//372 / 1
        u16Tmp = 4;
    }
    
    TPDU_SendBuff(au8Atr, u16Tmp);
    
    while ( 0 != QUEUE_C_LEN(tpduSendBuf));//wait queue send over
    while ( 0 == SCI_GetITStatus(SCI_INTIO_TX_INT));//wait fifo send over
    
    u16Tmp = Fi[ (u8TA1 >> 4) & 0x0F] / Di[u8TA1 & 0x0F];
    
    SCI_SetEtuData(u16Tmp);
    SCI_ClearFifo(SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR);
}

void SCIT0_RecvFlush(void)
{   
    SCI_ITConfig(SCI_INTIO_RX_INT, DISABLE);
    memset(&t0Recv, 0, sizeof(t0Recv));
    SCI_ITConfig(SCI_INTIO_RX_INT, ENABLE);
}

int32_t SCIT0_IsSendFinish(void)
{
    //return QUEUE_IS_EMPTY(spiSendQueue) && (0 == (SPIS->FSR & 0x7F));
    return (QUEUE_C_LEN(t0Send) == 0) && (GPIO->IOD & (0x01 << (4 + 8))); 
}


int SER_PutChar(int ch)
{
    SCIT0_SendChar(ch);
    while(0 == SCIT0_IsSendFinish());
    return ch;
}
int fputc(int c, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    if (c == '\n')  {
        SER_PutChar('\r');
    }
    return (SER_PutChar(c));
}


int SCI_T0_GetChar(void)
{
    int i32Tmp;
    do
    {
        i32Tmp = SCIT0_RecvChar();
    } while (i32Tmp == -1);
    return i32Tmp;
}
int fgetc(FILE *f)
{
    return SCI_T0_GetChar();
}
#endif

#if (STD_IO_PORT == STD_IO_PORT_SCI)
/*
1, blocking 
2, without interrupt
3, just for output
*/
/*
    SCI CLK = 2.88MHz
    UART BAUD = 115200
    ETUDATA = 25
    EVEN parity
    8N1
    
    or
    
    SCI CLK = 4.5MHz
    UART BAUD = 9600
    ETUDATA = 468.75  = 469
    EVEN parity
    8N1
    
    or 
    
    SCI CLK = 48MHz
    UART BAUD = 9600
    ETUDATA = 5000
    EVEN parity
    8N1
    
    or 
    
    SCI CLK = 2.88MHz
    UART BAUD = 9600
    ETUDATA = 300
    EVEN parity
    8N1
    
    or 
    
    SCI CLK = 200KHz
    UART BAUD = 1200
    ETUDATA = 167
    EVEN parity
    8N1
    
    or 
    
    SCI CLK = 10MHz
    UART BAUD = 9600
    ETUDATA = 1042
    EVEN parity
    8N1
    
    or 
    
    SCI CLK = 10MHz
    UART BAUD = 9600
    ETUDATA = 1042
    EVEN parity
    8N1
*/
void SCI_InitForPrintf(void)
{
    SYSCTRL_ClockEnable(SYSCTRL_Periph_ISO7816, ENABLE);
    SYSCTRL_SoftReset(SYSCTRL_Periph_ISO7816);
    
    NVIC_DisableIRQ(SCIS_IRQn);
    SCI->ETUDATA = 469;
    SCI_SetExtraETU(SCI_MODHW_EX_ETU_4);
    SCI_SetRetryTimes(SCI_MODHW_RETRY_7);
    SCI->INTIO1 = 0xFFFF;
    SCI_ClearFifo(SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR);
    GPIO->MODE_H = (GPIO->MODE_H & 0xFFFFFF00) | 0x0D;
}

int SER_PutChar(int ch)
{
    uint32_t i;
    SCI->INTIO1 = 0xFFFF;
    SCI_SendChar(ch);
    while(RESET == SCI_GetITStatus(SCI_INTIO_TX_INT));
    //for (i = 0; i < 0x1000; i ++);
    return ch;
}

int fputc(int c, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    if (c == '\n')  {
        SER_PutChar('\r');
    }
    return (SER_PutChar(c));
}


int SCI_NoneT0_GetChar(void)
{
    volatile uint32_t i;
    for (i = 0; i < 0x100000; i ++);
    return ' ';
}
int fgetc(FILE *f)
{
    return SCI_NoneT0_GetChar();
}
#endif


#if (STD_IO_PORT == STD_IO_PORT_SPI)
/*
1, blocking 
2, without interrupt
3, input and output
    connect to stm32 use spi.
    stm32 uart baud : 115200
*/
#define SPI_ESC_CHAR         (0xDB)
#define SPI_SR_TX_FULL      (BIT(4))
#define SPI_SR_TX_EMPTY     (BIT(2))
#define SPI_SR_RX_EMPTY     (BIT(10))
#define SPI_SR_RX_FULL      (BIT(12))
#define SPI_CS_PIN          (GPIO_Pin_6)
#define SPI_CLK_PIN         (GPIO_Pin_5)
#define SPI_MOSI_PIN        (GPIO_Pin_1)
#define SPI_MISO_PIN        (GPIO_Pin_0)
#define SPI_MSG_PIN         (GPIO_Pin_7)

#define SPI_MSG_VALID       GPIO_ResetBits(SPI_MSG_PIN)
#define SPI_MSG_UNVALID     GPIO_SetBits(SPI_MSG_PIN)

__STATIC_INLINE Boolean SPI_TxFifoIsFull()
{
    return 0 != (SPIS->SR & SPI_SR_TX_FULL);
}
__STATIC_INLINE Boolean SPI_TxFifoIsEmpty()
{
    return 0 != (SPIS->SR & SPI_SR_TX_EMPTY);
}

__STATIC_INLINE Boolean SPI_RxFifoIsEmpty()
{
    return 0 != (SPIS->SR & SPI_SR_RX_EMPTY);
}

__STATIC_INLINE Boolean SPI_RxFifoIsFull()
{
    return 0 != (SPIS->SR & SPI_SR_RX_FULL);
}

void SPI_GpioInit(void)
{
    volatile uint32_t i;
    
    GPIO_RemapConfig(SPI_CS_PIN, GPIO_Remap_SPI_SLV_CSN);
    GPIO_RemapConfig(SPI_CLK_PIN, GPIO_Remap_SPI_SLV_CLK);
    GPIO_RemapConfig(SPI_MOSI_PIN, GPIO_Remap_SPI_SLV_SDI);
    GPIO_RemapConfig(SPI_MISO_PIN, GPIO_Remap_SPI_SLV_SDO);
    GPIO_RemapConfig(SPI_MSG_PIN, GPIO_Remap_IO);
    GPIO_ModeConfig(SPI_MSG_PIN, GPIO_Mode_OUT);
    SPI_MSG_UNVALID;
}
void SPI_InitForPrintf(void)
{
    SPI_InitTypeDef SPI_InitStruct;
    SYSCTRL_ClockEnable(SYSCTRL_Periph_SPI | SYSCTRL_Periph_GPIO, ENABLE);
    SYSCTRL_SoftReset(SYSCTRL_SPI_RESET | SYSCTRL_GPIO_RESET);
    
    NVIC_DisableIRQ(SPI_IRQn);
    SPI_GpioInit();
    
    SPI_InitStruct.SPI_Sample = SPI_SAMPLE_MODE_0;	
    SPI_InitStruct.SPI_RXFIFOFullThreshold = SPI_RX_FIFO_FULL_THRESHOLD_1;
	SPI_InitStruct.SPI_TXFIFOEmptyThreshold = SPI_TX_FIFO_EMPTY_THRESHOLD_1;
	SPI_InitStruct.SPI_TXEnable = ENABLE;
	SPI_InitStruct.SPI_RXEnable = ENABLE;
	*((uint32_t *)0x50004020) &= ~BIT(24);
	SPI_FIFOReset(SPI_FIFO_TX | SPI_FIFO_RX);
	SPI_Init(&SPI_InitStruct);
    
}

int SPI_GetChar(void)
{
    int ch;
    ch = 0;
    while (0x00 == ch || 0xFF == ch)
    {
        while(SPI_RxFifoIsEmpty());
        //while(!SPI_RxFifoIsFull());
        ch = SPIS->RDR;
    }
    if (SPI_ESC_CHAR == ch)
    {
        while (0x00 == ch || 0xFF == ch)
        {
            while(SPI_RxFifoIsEmpty());
            //while(!SPI_RxFifoIsFull());
            ch = SPIS->RDR;
        }
        return ch ^ 0x01;
    }
    else
    {
        return ch;
    }
}

int SPI_SendChar(int ch)
{
    //while(SPI_TxFifoIsFull());
    SPIS->WDR = 0x00;
    SPI_MSG_VALID;
    while((SPIS->FSR & 0x7F) != 0);
    SPIS->WDR = ch;
    while((SPIS->FSR & 0x7F) != 0);
    SPIS->WDR = 0x00;
    while((SPIS->FSR & 0x7F) != 0);
    SPI_MSG_UNVALID;
    return ch;
}

int SER_PutChar(int ch)
{
    
    if (SPI_ESC_CHAR == ch || 0x00 == ch || 0xFF == ch)
    {
        SPI_SendChar(SPI_ESC_CHAR);
        SPI_SendChar(ch ^ 0x01);
    }
    else
    {
        SPI_SendChar(ch);
    }
    
    return ch;
}

int fputc(int c, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    if (c == '\n')  {
        SER_PutChar('\r');
    }
    return (SER_PutChar(c));
}

int fgetc(FILE *f)
{
    return SPI_GetChar();
}

#endif
#if (STD_IO_PORT == STD_IO_PORT_SCI_UART)
/*
1, blocking 
2, without interrupt
3, just for output
*/
/*
    SCI CLK = 48MHz
    UART BAUD = 9600
    ETUDATA = 5000
    EVEN parity
    8N1
*/
void SCI_UARTInitForPrintf(void)
{
    SYSCTRL_ClocksTypeDef clk;
    SYSCTRL_ClockEnable(SYSCTRL_Periph_ISO7816, ENABLE);
    SYSCTRL_SoftReset(SYSCTRL_Periph_ISO7816);
    
    GPIO->MODE_H = (GPIO->MODE_H & ~GPIO_MODE_H_PA6_Msk) | (GPIO_MODE_RC_2P25M << GPIO_MODE_H_PA6_Pos);
    
    NVIC_DisableIRQ(SCIS_IRQn);
    SYSCTRL_GetClocksFreq(&clk);
    
//    SCI->ETUDATA = clk.PCLK_Frequency / 9600;//332;
    SCI->ETUDATA = 372;//wcq
    SCI_SetExtraETU(SCI_MODHW_EX_ETU_4);
    SCI_SetRetryTimes(SCI_MODHW_RETRY_0);
    SCI->INTIO1 = 0xFFFF;
    SCI_ClearFifo(SCI_CTRL_TX_FIFO_CLR | SCI_CTRL_RX_FIFO_CLR);
    SCI->CTRL &= ~SCI_CTRL_RETRY_EN;//disable retry
#if 1
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA3_Msk) | (GPIO_CON_IN_PULL_UP << GPIO_CON_PA3_Pos);
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA2_Msk) | (GPIO_CON_IN_PULL_UP << GPIO_CON_PA2_Pos);
    GPIO->MODE_L = (GPIO->MODE_L & ~GPIO_MODE_L_PA3_Msk) | (GPIO_MODE_7816_data_in << GPIO_MODE_L_PA3_Pos);//set gpio 3 as sci in
    GPIO->MODE_L = (GPIO->MODE_L & ~GPIO_MODE_L_PA2_Msk) | (GPIO_MODE_7816_data_out << GPIO_MODE_L_PA2_Pos);
#else
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA4_Msk) | (GPIO_CON_IN_PULL_UP << GPIO_CON_PA4_Pos);
    GPIO->MODE_H = (GPIO->MODE_H & ~GPIO_MODE_H_PA4_Msk) | (GPIO_MODE_7816_data_in << GPIO_MODE_H_PA4_Pos);
#endif
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA7_Msk) | (GPIO_CON_IN_PULL_UP << GPIO_CON_PA6_Pos);
    GPIO->MODE_H = (GPIO->MODE_H & ~GPIO_MODE_H_PA7_Msk) | (GPIO_MODE_GPIO << GPIO_MODE_H_PA6_Pos);
}

int SER_PutChar(int ch)
{
    uint32_t i;
    SCI->INTIO1 = 0xFFFF;
    SCI_SendChar(ch);
//    while(RESET == SCI_GetITStatus(SCI_INTIO_TX_INT));
    for (i = 0; i < 0x10000; i ++);
    return ch;
}

int fputc(int c, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    if (c == '\n')  {
        SER_PutChar('\r');
    }
    return (SER_PutChar(c));
}


int SCI_UART_GetChar(void)
{
    volatile uint32_t i;
    while(RESET == SCI_GetStatus(SCI_STAT_RX_FIFO_NONEMPTY));
    SCI->STAT = 0xFFFF;
    return SCI_GetChar();
}
int fgetc(FILE *f)
{
    return SCI_UART_GetChar();
}

#endif

#if (STD_IO_PORT == STD_IO_PORT_NULL)


//int fputc(int c, FILE *f)
//{
//    /* Place your implementation of fputc here */
//    /* e.g. write a character to the USART */
//    return (c);
//}

//int fgetc(FILE *f)
//{
//    return 0;
//}
#endif

#if (STD_IO_PORT == STD_IO_PORT_GPIO)
uint32_t u32HalfLoad = 0;
/*
TX : PA6
RX : PA7
BAUD : 600 ~ 19200
*/
void GPIO_UARTInitForPrintf(uint32_t u32Baud, uint32_t u32Parity)
{
    /* config GPIO6 as output IO mode & default high*/
    GPIO->IOD |= 0x0040;
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA6_Msk) | (GPIO_CON_OUT << GPIO_CON_PA6_Pos);
    GPIO->MODE_H = (GPIO->MODE_H & ~GPIO_MODE_H_PA6_Msk) | (GPIO_MODE_GPIO << GPIO_MODE_H_PA6_Pos);
    
    GPIO->CON = (GPIO->CON & ~GPIO_CON_PA7_Msk) | (GPIO_CON_IN_PULL_UP << GPIO_CON_PA7_Pos);
    GPIO->MODE_H = (GPIO->MODE_H & ~GPIO_MODE_H_PA7_Pos) | (GPIO_MODE_GPIO << GPIO_MODE_H_PA7_Pos);
    GPIO->INT_TYPE = (GPIO->INT_TYPE & ~GPIO_INT_TYPE_PA7_Msk) | (GPIO_INT_TYPE_FALLING << GPIO_INT_TYPE_PA7_Pos);
    GPIO->WAKEUP_EN = (GPIO->WAKEUP_EN | BIT(7));
    
    /* config systick use external clock: RC/64 = 1125000Hz */
    SYSCFG->CLK_CFG &= ~(BIT(26));
    
    SysTick->LOAD =  1125000 / u32Baud; /* 10 bits time */ //750000 1125000
	u32HalfLoad = 375000 / u32Baud;
    
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* start count */
    
    #if(STD_IO_NEED_GETC)
    NVIC_SetPriority(GPIO_IRQn, 0);
    NVIC_EnableIRQ(GPIO_IRQn);
    #endif
}

uint16_t gu16RecvDat = 0;
uint8_t  gu8RecvOver = 0;
uint8_t  gu8RecvBitCount = 0;
uint8_t  gu8RecvEn = 0;
uint8_t  gu8RecvBytes = 0;
uint8_t  gu8RecvReadBytes = 0;
uint8_t  gu8RecvBuff[256];

#if(STD_IO_NEED_GETC)
void GPIO_IRQHandler(void)
{
    uint8_t u8Bits;
    uint32_t u32Remain;
    if (GPIO->INT_STAT & 0x80)//gpio 7 interrupt
    {
        GPIO->INT_STAT = 0x80;//clear interrupt
        GPIO->INT_TYPE = (GPIO->INT_TYPE & ~GPIO_INT_TYPE_PA7_Msk) | (GPIO_INT_TYPE_NONE << GPIO_INT_TYPE_PA7_Pos);//mask interrupt
        gu8RecvEn = 1;
        u32Remain = SysTick->LOAD;
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; /* stop count */
        SysTick->VAL = 0;
        SysTick->LOAD = u32HalfLoad;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* start count */
        while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk ));
        
        gu16RecvDat = 0;
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; /* stop count */
        SysTick->VAL = 0;
        SysTick->LOAD = u32Remain;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* start count */
        for (u8Bits = 0; u8Bits < 10; u8Bits ++)
        {
//            GPIO->IOD ^= 0x02;
            if ((GPIO->IOD) & 0x8000)
            {
                gu16RecvDat |= 0x0400;
            }
            gu16RecvDat >>= 1;
            while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk ));
        }
        gu8RecvBuff[gu8RecvBytes++] = gu16RecvDat >> 1;
        GPIO->INT_TYPE = (GPIO->INT_TYPE & ~GPIO_INT_TYPE_PA7_Msk) | (GPIO_INT_TYPE_FALLING << GPIO_INT_TYPE_PA7_Pos);//enable interrupt
        gu8RecvOver = 1;
    }
}
#endif

void GPIO_PutChar(int c)
{
    uint8_t i;
    uint16_t gu16SendDat = 0;
    volatile uint32_t *pu32Addr = &GPIO->BSR;
    volatile uint32_t *pu32Reg = &SysTick->CTRL;
    volatile uint32_t u32Dat = 0;
    
    gu16SendDat = c | 0xFFFFFF00;
    gu16SendDat = gu16SendDat;// | (i << 8);
    gu16SendDat <<= 2;
    gu16SendDat &= ~(0x03UL);
    gu16SendDat |= 0x01UL;
    
    __ASM("CPSID i");
    SysTick->VAL = 0;
    for (i = 0; i < 11; i ++)
    {
        if (gu16SendDat & 0x0001)
        {
            u32Dat = 0x0040;
        }
        else 
        {
            u32Dat = 0x4000;
        }
        while(!(*pu32Reg & SysTick_CTRL_COUNTFLAG_Msk ));
        *pu32Addr = u32Dat;
        gu16SendDat >>= 1;
    }
    __ASM("CPSIE i");
}

int32_t GPIO_GetChar(void)
{
    if (gu8RecvBytes == gu8RecvReadBytes)
    {
        return -1;
    }
    else
    {
        return gu8RecvBuff[gu8RecvReadBytes++];//(gu16RecvDat >> 1) & 0xFF
    }
}
int fputc(int c, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
	if(c == '\n')
	{
		GPIO_PutChar('\r');
	}
    GPIO_PutChar(c);
    return (c);
}

int fgetc(FILE *f)
{
    return GPIO_GetChar();
}
#endif
/*

*/
#if (STD_IO_PORT == STD_IO_PORT_SCI_MSI)

#define QUEUE_IS_EMPTY(q)               (((q).head&(sizeof((q).buf) - 1))==((q).tail&(sizeof((q).buf) - 1)))
#define QUEUE_IS_FULL(q)                (((q).head&(sizeof((q).buf) - 1))==(((q).tail+1)&(sizeof((q).buf) - 1)))
#define QUEUE_LEN(q)                    (((q).tail+(sizeof((q).buf))-(q).head)&(sizeof((q).buf) - 1))
#define QUEUE_SPARE(q)                  (((q).head+(sizeof((q).buf) - 1)-(q).tail)&(sizeof((q).buf) - 1))
#define QUEUE_HEAD_NEXT(q)              (((q).head+1)&(sizeof((q).buf) - 1))
#define QUEUE_TAIL_NEXT(q)              (((q).tail+1)&(sizeof((q).buf) - 1))
#define QUEUE_POSI(q, n)                ((n)&(sizeof((q).buf) - 1))


/*S* TPDU Protocol drivers */
#define SCIMSI_SEND_BUFF_SIZE          (BIT(7))
#define SCIMSI_RECV_BUFF_SIZE          (BIT(7))

typedef struct
{
    volatile uint16_t       head;
    volatile uint16_t       tail;
    volatile uint16_t       len;
    volatile uint16_t       err_stat;
    volatile uint8_t        buf[SCIMSI_SEND_BUFF_SIZE];
} SCIMSI_SEND_BUF;

typedef struct
{
    volatile uint16_t       head;
    volatile uint16_t       tail;
    volatile uint16_t       len;
    volatile uint16_t       err_stat;
    volatile uint8_t        buf[SCIMSI_RECV_BUFF_SIZE];
} SCIMSI_RECV_BUF;

SCIMSI_SEND_BUF sciMsiSendBuff;
SCIMSI_RECV_BUF sciMsiRecvBuff;


void SCIMSI_InitForPrintf()
{
    NVIC_DisableIRQ(SCIS_IRQn);
    SYSCTRL_ClockEnable(SYSCTRL_Periph_ISO7816, ENABLE);
    SYSCTRL_SoftReset(SYSCTRL_Periph_ISO7816);
    
    SCI_RetryEnable(ENABLE);
    SCI_SetRetryTimes(SCI_MODHW_RETRY_7);
    SCI_SetExtraETU(SCI_MODHW_EX_ETU_1);
    
    SCI_BgtCwtEnable(SCI_BGTCTRL_BGT_EN | SCI_BGTCTRL_CWT_EN, ENABLE);
    SCI_SetBgtData(0x1F);
    SCI_SetCwtData(0x100);
    SCI_ExRstIntEdge(SCI_CTRL_EXTRST_INT_RISE_EDGE);
    
    SCI_ClearIT(SCI_INTIO_ALL);
    SCI_ClearFifo(SCI_CTRL_RX_FIFO_CLR | SCI_CTRL_TX_FIFO_CLR);
    SCI_ITConfig(SCI_CTRL_RX_INT_EN | SCI_CTRL_EXTRST_INT_EN, ENABLE);
    
    GPIO->MODE_H = (GPIO->MODE_H & 0xFFFFFF00) | 0x0D;
    
    memset(&sciMsiRecvBuff, 0, sizeof(sciMsiRecvBuff));
    memset(&sciMsiSendBuff, 0, sizeof(sciMsiSendBuff));
    NVIC_EnableIRQ(SCIS_IRQn);
}

int SER_PutChar(int ch)
{
    uint16_t i, u16Len;
    //wait for send permission
    while (RESET == SCI_GetStatus(SCI_STAT_CWT));
    
    
    if (QUEUE_SPARE(sciMsiSendBuff))
    {
        //put char to send buff
        sciMsiSendBuff.buf[sciMsiSendBuff.tail] = ch;
        sciMsiSendBuff.tail = QUEUE_TAIL_NEXT(sciMsiSendBuff);
    }
    if ((ch == '\n')
        || (QUEUE_IS_FULL(sciMsiSendBuff)))// or buff is full
    {
        u16Len = QUEUE_LEN(sciMsiSendBuff);
        //send len
//        while(SET == SCI_GetStatus(SCI_STAT_TX_FIFO_FULL));
//        SCI_SendChar(u16Len);
        //send package data
        for (i = 0; i < u16Len; i ++)
        {
            while(SET == SCI_GetStatus(SCI_STAT_TX_FIFO_FULL));
            SCI_SendChar(sciMsiSendBuff.buf[sciMsiSendBuff.head]);
            sciMsiSendBuff.head = QUEUE_HEAD_NEXT(sciMsiSendBuff);
        }
    }
    return ch;
}
int fputc(int c, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    if (c == '\n')  {
        SER_PutChar('\r');
    }
    return (SER_PutChar(c));
}

uint32_t SER_GetChar(void)
{
    uint8_t u8Tmp;
    if (0 == QUEUE_LEN(sciMsiRecvBuff))
    {
        return ~0UL;
    }
    u8Tmp = sciMsiRecvBuff.buf[sciMsiRecvBuff.head];
    sciMsiRecvBuff.head = QUEUE_HEAD_NEXT(sciMsiRecvBuff);
    return (uint32_t)u8Tmp;
}
int fgetc(FILE *f)
{
    int i32Tmp;
    do
    {
        i32Tmp = SER_GetChar();
    } while (i32Tmp == -1);
    return i32Tmp;
}

void SCIS_IRQHandler(void)
{
    uint8_t u8Recv;
    static uint32_t su32Count = 0;
    //SCI->INTIO1 |= ~BIT(5);
    if (SCI->CTRL & SCI_CTRL_EXTRST_INT_EN
        && SCI->INTIO1 & SCI_INTIO_EXTRST_INT)
    {
        SYSCFG->RST_EN = 0x0000FFFF;
        while(1);
    }
    if (SCI->STAT & SCI_STAT_TX_ERROR)
    {
        SCI->CTRL |= SCI_CTRL_TX_FIFO_CLR;
    }
    if (SCI->STAT & SCI_STAT_RX_ERROR)
    {
        SCI->CTRL |= SCI_CTRL_RX_FIFO_CLR;
    }
    if (SCI->INTIO1 & SCI_INTIO_RX_INT 
        && SCI->CTRL & SCI_CTRL_RX_INT_EN)
    {
        SCI->INTIO1 |= SCI_INTIO_RX_INT;
        while (SCI->STAT & (SCI_STAT_RX_FIFO_NONEMPTY | SCI_STAT_RX_FIFO_FULL))
        {
            if (QUEUE_SPARE(sciMsiRecvBuff))
            {
                sciMsiRecvBuff.buf[sciMsiRecvBuff.tail] = SCI->BUFHW;
                sciMsiRecvBuff.tail = QUEUE_TAIL_NEXT(sciMsiRecvBuff);
            }
            else 
            {
                break;
            }
        }
    }
    else
    {
        ;
    }
    return;
}
#endif
void mhcard_StdIOInit(uint32_t u32BaudRate, uint32_t u32Parity)
{
#if (STD_IO_PORT == STD_IO_PORT_NULL)
    
#elif (STD_IO_PORT == STD_IO_PORT_SCI)
    SCI_InitForPrintf();
#elif (STD_IO_PORT == STD_IO_PORT_SPI)
    SPI_InitForPrintf();
#elif (STD_IO_PORT == STD_IO_PORT_SCI_T0)
    SCIT0_InitForPrintf();
#elif (STD_IO_PORT == STD_IO_PORT_SCI_UART)
    SCI_UARTInitForPrintf();
#elif (STD_IO_PORT == STD_IO_PORT_GPIO) 
    GPIO_UARTInitForPrintf(u32BaudRate, u32Parity);
#elif (STD_IO_PORT == STD_IO_PORT_SCI_MSI)
    SCIMSI_InitForPrintf();
#endif
}


