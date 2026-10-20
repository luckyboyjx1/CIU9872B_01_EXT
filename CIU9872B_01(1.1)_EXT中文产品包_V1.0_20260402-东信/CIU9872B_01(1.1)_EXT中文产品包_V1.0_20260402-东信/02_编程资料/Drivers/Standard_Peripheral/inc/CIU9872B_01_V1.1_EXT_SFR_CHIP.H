#ifndef CIU9872B_01_V1_1_EXT_SFR_CHIP_H__
#define CIU9872B_01_V1_1_EXT_SFR_CHIP_H__
#include <stdint.h>
#define _SFR_STRU
#define M_SFR(module, sfr)            module->sfr
/* (Module Num : 22)|(SFR Num: 242)|(Total Len : 0X0000F900) */
#define	UMASK_BA                      0x4007a000
#define	CKMU_BA                       0x40080000
#define	RSTMU_BA                      0x40080800
#define	EMMU_BA                       0x40081000
#define	NVMCTRL_BA                    0x40081800
#define	CACHE_BA                      0x40085000
#define	SEC_BA                        0x40086000
#define	IOCTRL_BA                     0x40088000
#define	PMUM_BA                       0x40086800
#define	TIMER0_BA                     0x40000000
#define	TIMER1_BA                     0x40000800
#define	TIMER2_BA                     0x40001000
#define	WDT_BA                        0x40004000
#define	S7816_BA                      0x40008000
#define	GPIO_BA                       0x40017000
#define	CRC_BA                        0x40074000
#define	DMA_BA                        0x400a2000
#define	CPU_BA                        0xe0000000
#define	UMASK                         ((volatile UMASK_TypeDef        *)UMASK_BA                      )
#define	CKMU                          ((volatile CKMU_TypeDef         *)CKMU_BA                       )
#define	RSTMU                         ((volatile RSTMU_TypeDef        *)RSTMU_BA                      )
#define	EMMU                          ((volatile EMMU_TypeDef         *)EMMU_BA                       )
#define	NVMCTRL                       ((volatile NVMCTRL_TypeDef      *)NVMCTRL_BA                    )
#define	HED_CACHE                     ((volatile CACHE_TypeDef        *)CACHE_BA                      )
#define	SEC                           ((volatile SEC_TypeDef          *)SEC_BA                        )
#define	IOCTRL                        ((volatile IOCTRL_TypeDef       *)IOCTRL_BA                     )
#define	PMUM                          ((volatile PMUM_TypeDef         *)PMUM_BA                       )
#define	TIMER0                        ((volatile TIMER0_TypeDef       *)TIMER0_BA                     )
#define	TIMER1                        ((volatile TIMER1_TypeDef       *)TIMER1_BA                     )
#define	TIMER2                        ((volatile TIMER2_TypeDef       *)TIMER2_BA                     )
#define	WDT                           ((volatile WDT_TypeDef          *)WDT_BA                        )
#define	S7816                         ((volatile S7816_TypeDef        *)S7816_BA                      )
#define	GPIO                          ((volatile GPIO_TypeDef         *)GPIO_BA                       )
#define	CRC                           ((volatile CRC_TypeDef          *)CRC_BA                        )
#define	DMA                           ((volatile DMA_TypeDef          *)DMA_BA                        )
#define	CPU                           ((volatile CPU_TypeDef          *)CPU_BA                        )
typedef struct
{
    /* length: 16 */
    uint32_t USERMASK0;
    uint32_t USERMASK1;
    uint32_t USERMASK2;
    uint32_t USERMASK3;
} UMASK_TypeDef;
typedef struct
{
    /* length: 168 */
    uint32_t RESERVED0;
    uint32_t SYSCLKCFG1;
    uint32_t RESERVED1[10];
    uint32_t SYSAHB1CLKEN;
    uint32_t SYSAHB2CLKEN;
    uint32_t RESERVED2;
    uint32_t SYSAPBCLKEN;
    uint32_t SYSAHB1CLKCLR;
    uint32_t SYSAHB2CLKCLR;
    uint32_t RESERVED3;
    uint32_t SYSAPBCLKCLR;
    uint32_t RESERVED4[20];
    uint32_t SYSPWRFLAG;
    uint32_t SYSCLKTIM2;
} CKMU_TypeDef;
typedef struct
{
    /* length: 116 */
    uint32_t SYSINFST;
    uint32_t RESERVED0;
    uint32_t SYSRSTFLAG;
    uint32_t SYSRSTCFG;
    uint32_t RESERVED1[8];
    uint32_t SYSINTCLR;
    uint32_t SYSINTSRC;
    uint32_t SYSINTMASK;
    uint32_t RESERVED2;
    uint32_t SYSVDFLAG;
    uint32_t RESERVED3[3];
    uint32_t SYSMODULERST0;
    uint32_t SYSMODULERST1;
    uint32_t RESERVED4[2];
    uint32_t SYSREGRSTREQ;
    uint32_t RESERVED5[3];
    uint32_t SYSSNRCFG;
} RSTMU_TypeDef;
typedef struct
{
    /* length: 128 */
    uint32_t RESERVED0[7];
    uint32_t SYSEMMUCON;
    uint32_t RESERVED1;
    uint32_t SYSEMMUSD;
    uint32_t RESERVED2;
    uint32_t SYSSDSTADDR;
    uint32_t RESERVED3[14];
    uint32_t SYSUSERFUNC1;
    uint32_t SYSUSERFUNC2;
    uint32_t SYSSWITCHTAG1;
    uint32_t SYSSWITCHTAG2;
    uint32_t SYSSWITCHTAG3;
    uint32_t SYSSWITCHTAG4;
} EMMU_TypeDef;
typedef struct
{
    /* length: 260 */
    uint32_t SYSNVMSTATUS;
    uint32_t RESERVED0[20];
    uint32_t SYSNVMDEADDR;
    uint32_t RESERVED1[7];
    uint32_t SYSCFGREG2;
    uint32_t RESERVED2[34];
    uint32_t SYSNVMDE;
} NVMCTRL_TypeDef;
typedef struct
{
    /* length: 40 */
    uint32_t SYSCACON;
    uint32_t SYSCALINV;
    uint32_t SYSCAMASK0;
    uint32_t SYSCAAREA0;
    uint32_t SYSCAMASK1;
    uint32_t SYSCAAREA1;
    uint32_t SYSCAMASK2;
    uint32_t SYSCAAREA2;
    uint32_t SYSCAMASK3;
    uint32_t SYSCAAREA3;
} CACHE_TypeDef;
typedef struct
{
    /* length: 56 */
    uint32_t RESERVED0[6];
    uint32_t SECSWRSTFLG;
    uint32_t SECHWRSTFLG;
    uint32_t RESERVED1[2];
    uint32_t SECREGPRT;
    uint32_t SECSWCES;
    uint32_t SECAUTOCHK;
    uint32_t SECRNDSTOP;
} SEC_TypeDef;
typedef struct
{
    /* length: 48 */
    uint32_t IOCTRLEN;
    uint32_t IOCTRLOD;
    uint32_t IOCTRLDS;
    uint32_t IOCTRLPDEN;
    uint32_t IOCTRLPUEN;
    uint32_t IOCTRLAUTO;
    uint32_t IOCTRLAF;
    uint32_t IODGFEN;
    uint32_t IODGFCFG;
    uint32_t IOCGFEN;
    uint32_t IOCGFCFG;
    uint32_t IOCTRLSEL;
} IOCTRL_TypeDef;
typedef struct
{
    /* length: 80 */
    uint32_t SYSCPUHLDCON;
    uint32_t SYSPMDCON;
    uint32_t SYSSTDWKPCFG;
    uint32_t RESERVED0;
    uint32_t SYSSTDWKPCON;
    uint32_t RESERVED1;
    uint32_t SYSSTDWKPSTATUS1;
    uint32_t SYSSTDWKPSTATUS2;
    uint32_t SYSPDWKPCON;
    uint32_t SYSPDWKPSTATUS;
    uint32_t RESERVED2[9];
    uint32_t SYSMEMPC;
} PMUM_TypeDef;
typedef struct
{
    /* length: 16 */
    uint32_t TIMER0LOAD;
    uint32_t TIMER0VALUE;
    uint32_t TIMER0CONTROL;
    uint32_t TIMER0IS;
} TIMER0_TypeDef;
typedef struct
{
    /* length: 16 */
    uint32_t TIMER1LOAD;
    uint32_t TIMER1VALUE;
    uint32_t TIMER1CONTROL;
    uint32_t TIMER1IS;
} TIMER1_TypeDef;
typedef struct
{
    /* length: 16 */
    uint32_t TIMER2LOAD;
    uint32_t TIMER2VALUE;
    uint32_t TIMER2CONTROL;
    uint32_t TIMER2IS;
} TIMER2_TypeDef;
typedef struct
{
    /* length: 16 */
    uint32_t WDTLoad;
    uint32_t WDTValue;
    uint32_t WDTControl;
    uint32_t WDTIS;
} WDT_TypeDef;
typedef struct
{
    /* length: 36 */
    uint32_t SCISBUF;
    uint32_t SCISCON;
    uint32_t SCISSR;
    uint32_t SCISCNT;
    uint32_t SCITXFCR;
    uint32_t SCIRXFCR;
    uint32_t RESERVED0;
    uint32_t SCIS60CON;
    uint32_t SCIDMACR;
} S7816_TypeDef;
typedef struct
{
    /* length: 36 */
    uint32_t GPIODATA;
    uint32_t GPIODIR;
    uint32_t GPIOIS;
    uint32_t GPIOIBE;
    uint32_t GPIOIEV;
    uint32_t GPIOIE;
    uint32_t GPIORIS;
    uint32_t GPIOMIS;
    uint32_t GPIOIC;
} GPIO_TypeDef;
typedef struct
{
    /* length: 16 */
    uint32_t CRCCTRL;
    uint32_t CRCRESULT;
    uint32_t CRCFLAG;
    uint32_t CRCDATA;
} CRC_TypeDef;
typedef struct
{
    /* length: 80 */
    uint32_t DMAC_CR;
    uint32_t RESERVED0[3];
    uint32_t DMAC_CA0;
    uint32_t DMAC_CB0;
    uint32_t DMAC_SA0;
    uint32_t DMAC_DA0;
    uint32_t DMAC_CA1;
    uint32_t DMAC_CB1;
    uint32_t DMAC_SA1;
    uint32_t DMAC_DA1;
    uint32_t DMAC_CA2;
    uint32_t DMAC_CB2;
    uint32_t DMAC_SA2;
    uint32_t DMAC_DA2;
    uint32_t DMAC_CA3;
    uint32_t DMAC_CB3;
    uint32_t DMAC_SA3;
    uint32_t DMAC_DA3;
} DMA_TypeDef;
typedef struct
{
    /* length: 61464 */
    uint32_t RESERVED0[14340];
    uint32_t CORET_CSR;
    uint32_t CORET_RVR;
    uint32_t CORET_CVR;
    uint32_t CORET_CALIB;
    uint32_t RESERVED1[56];
    uint32_t VIC_ISER;
    uint32_t MBCB;
    uint32_t BABA;
    uint32_t BABB;
    uint32_t RESERVED2[12];
    uint32_t VIC_IWER;
    uint32_t BAMB;
    uint32_t HCR;
    uint32_t HSR;
    uint32_t RESERVED3[12];
    uint32_t VIC_ICER;
    uint32_t BABC;
    uint32_t BAMC;
    uint32_t BABD;
    uint32_t RESERVED4[12];
    uint32_t VIC_IWDR;
    uint32_t BABE;
    uint32_t BAME;
    uint32_t BABF;
    uint32_t RESERVED5[12];
    uint32_t VIC_ISPR;
    uint32_t BABG;
    uint32_t BAMG;
    uint32_t BABH;
    uint32_t RESERVED6[28];
    uint32_t VIC_ICPR;
    uint32_t BABI;
    uint32_t BAMI;
    uint32_t MBIR;
    uint32_t RESERVED7[28];
    uint32_t VIC_IABR;
    uint32_t RESERVED8[63];
    uint32_t VIC_IPR0;
    uint32_t VIC_IPR1;
    uint32_t VIC_IPR2;
    uint32_t VIC_IPR3;
    uint32_t VIC_IPR4;
    uint32_t VIC_IPR5;
    uint32_t RESERVED9[506];
    uint32_t VIC_ISR;
    uint32_t VIC_IPTR;
    uint32_t VIC_TSPEND;
    uint32_t VIC_TSABR;
    uint32_t VIC_TSPR;
    uint32_t RESERVED10[223];
    uint32_t PCR;
    uint32_t RESERVED11[3];
    uint32_t SECR;
    uint32_t RESERVED12[23];
    uint32_t CER;
    uint32_t CIR;
    uint32_t CRCR0;
    uint32_t CRCR1;
    uint32_t CRCR2;
    uint32_t CRCR3;
} CPU_TypeDef;
#endif /* end CIU9872B_01_V1_1_EXT_SFR_CHIP_H__ */
